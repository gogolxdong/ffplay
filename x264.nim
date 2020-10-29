import winim, strformat
import common

const slice_type_to_char*: array[3,char] = ['P', 'B', 'I']


template CHROMA_SIZE*(s: untyped): untyped =
  when defined(CHROMA_FORMAT): 
    s shr (CHROMA_H_SHIFT + CHROMA_V_SHIFT) 
  else: 0

template FRAME_SIZE*(s: untyped): untyped = (s + 2 * CHROMA_SIZE(s))

type
  x264_predict_t* = proc (src: uint16)
  x264_predict8x8_t* = proc (src: uint16; edge: array[36, uint16])
  x264_predict_8x8_filter_t* = proc (src: uint16; edge: array[36, uint16];
                                  i_neighbor: cint; i_filters: cint)


when defined(CHROMA_FORMAT):
  const
    CHROMA_H_SHIFT* = (CHROMA_FORMAT == CHROMA_420 or CHROMA_FORMAT == CHROMA_422)
    CHROMA_V_SHIFT* = (CHROMA_FORMAT == CHROMA_420)
    CHROMA444* = (CHROMA_FORMAT == CHROMA_444)
else:
  template CHROMA_FORMAT*(h:untyped) = h.sps.i_chroma_format_idc
  template CHROMA_H_SHIFT*(h:untyped) = h.mb.chroma_h_shift
  template  CHROMA_V_SHIFT*(h:untyped) = h.mb.chroma_v_shift

const
  # X264_PCM_COST* = (FRAME_SIZE(256 * BIT_DEPTH) + 16)
  # QP_BD_OFFSET* = (6 * (BIT_DEPTH - 8))
  # PIXEL_MAX* = ((1 shl BIT_DEPTH) - 1)
  # QP_MAX_SPEC* = (51 + QP_BD_OFFSET)
  # QP_MAX* = (QP_MAX_SPEC + 18)
  # X264_LOOKAHEAD_QP* = (12 + QP_BD_OFFSET)
  NALU_OVERHEAD* = 5
  FILLER_OVERHEAD* = (NALU_OVERHEAD + 1)
  LUMA_DC* = 48
  CHROMA_DC* = 49
  FENC_STRIDE* = 16
  FDEC_STRIDE* = 32
  X264_BFRAME_MAX* = 16
  X264_REF_MAX* = 16
  X264_THREAD_MAX* = 128
  X264_LOOKAHEAD_THREAD_MAX* = 16
  X264_LOOKAHEAD_MAX* = 250
  X264_THREAD_HEIGHT* = 24
  X264_WEIGHTP_FAKE* = (-1)
  X264_SCAN8_LUMA_SIZE* = (5 * 8)
  X264_SCAN8_SIZE* = (X264_SCAN8_LUMA_SIZE * 3)
  X264_SCAN8_0* = (4 + 1 * 8)
  MAX_UINT* = 0xFFFFFFFF
  MAX_INT* = 2147483647
  MAX_INT64* = 0x0000000000000000'i64
  MAX_DOUBLE* = 1.7e+308
  QP_MIN* = 0
  QP_MAX_SPEC* = 51
  QP_MAX_MAX* = 81
  MIN_QPSCALE* = 0.2125
  MAX_MAX_QPSCALE* = 615.465742344771
  MAX_CHROMA_LAMBDA_OFFSET* = 36
  PADH* = 32
  PADV* = 32
  LOWRES_COST_MASK* = ((1 shl 14) - 1)
  LOWRES_COST_SHIFT* = 14

# template SEI_OVERHEAD(h:untyped)* = 
#   NALU_OVERHEAD - (h.param.b_annexb and not h.param.i_avcintra_class and (h.o.i_nal - 1))

template map_col_to_list0*(col: untyped): untyped =
  h.mb.map_col_to_list0[(col) + 2]
template deblock_ref_table*(x: untyped): untyped =
  h.mb.deblock_ref_table[(x) + 2]

template x264_glue3_expand(x,y,z:untyped) = fmt"{x}_{y}_{z}"

template x264_glue3*(x, y, z: untyped): untyped =
  x264_glue3_expand(x, y, z)

template x264_template*(w: untyped): untyped =
  x264_glue3(x264, BIT_DEPTH, w)

template SPEC_QP*(x: untyped): untyped =
  X264_MIN((x), QP_MAX_SPEC)

# const
#   x264_nal_encode* = x264_template(nal_encode)
#   x264_encoder_reconfig* = x264_template(encoder_reconfig)
#   x264_encoder_parameters* = x264_template(encoder_parameters)
#   x264_encoder_headers* = x264_template(encoder_headers)
#   x264_encoder_encode* = x264_template(encoder_encode)
#   x264_encoder_close* = x264_template(encoder_close)
#   x264_encoder_delayed_frames* = x264_template(encoder_delayed_frames)
#   x264_encoder_maximum_delayed_frames* = x264_template(encoder_maximum_delayed_frames)
#   x264_encoder_intra_refresh* = x264_template(encoder_intra_refresh)
#   x264_encoder_invalidate_reference* = x264_template(encoder_invalidate_reference)
#   x264_encoder_open* = x264_template(encoder_open)

type
  profile_e* = enum
    PROFILE_BASELINE = 66, PROFILE_MAIN = 77, PROFILE_HIGH = 100, PROFILE_HIGH10 = 110,
    PROFILE_HIGH422 = 122, PROFILE_HIGH444_PREDICTIVE = 244
  chroma_format_e* = enum
    CHROMA_400 = 0, CHROMA_420 = 1, CHROMA_422 = 2, CHROMA_444 = 3
  slice_type_e* = enum
    SLICE_TYPE_P = 0, SLICE_TYPE_B = 1, SLICE_TYPE_I = 2
  sei_payload_type_e* = enum
    SEI_BUFFERING_PERIOD = 0, SEI_PIC_TIMING = 1, SEI_PAN_SCAN_RECT = 2, SEI_FILLER = 3,
    SEI_USER_DATA_REGISTERED = 4, SEI_USER_DATA_UNREGISTERED = 5,
    SEI_RECOVERY_POINT = 6, SEI_DEC_REF_PIC_MARKING = 7, SEI_FRAME_PACKING = 45,
    SEI_ALTERNATIVE_TRANSFER = 147


##  Scan8 organization:
##     0 1 2 3 4 5 6 7
##  0  DY    y y y y y
##  1        y Y Y Y Y
##  2        y Y Y Y Y
##  3        y Y Y Y Y
##  4        y Y Y Y Y
##  5  DU    u u u u u
##  6        u U U U U
##  7        u U U U U
##  8        u U U U U
##  9        u U U U U
##  10 DV    v v v v v
##  11       v V V V V
##  12       v V V V V
##  13       v V V V V
##  14       v V V V V
##  DY/DU/DV are for luma/chroma DC.
##

type
  x264_level_t* = tuple
    level_idc: int
    mbps: int
    frame_size: int      
    dpb: int              
    bitrate: int          
    cpb: int              
    mv_range: int        
    mvs_per_2mb: int     
    slice_rate: int       
    mincr: int            
    bipred8x8: int        
    direct8x8: int        
    frame_only: int


var x264_levels*: seq[x264_level_t] = @[
    (10, 1485, 99, 396, 64, 175, 64, 64, 0, 2, 0, 0, 1),
    (9, 1485, 99, 396, 128, 350, 64, 64, 0, 2, 0, 0, 1),
    (11, 3000, 396, 900, 192, 500, 128, 64, 0, 2, 0, 0, 1),
    (12, 6000, 396, 2376, 384, 1000, 128, 64, 0, 2, 0, 0, 1),
    (13, 11880, 396, 2376, 768, 2000, 128, 64, 0, 2, 0, 0, 1),
    (20, 11880, 396, 2376, 2000, 2000, 128, 64, 0, 2, 0, 0, 1),
    (21, 19800, 792, 4752, 4000, 4000, 256, 64, 0, 2, 0, 0, 0),
    (22, 20250, 1620, 8100, 4000, 4000, 256, 64, 0, 2, 0, 0, 0),
    (30, 40500, 1620, 8100, 10000, 10000, 256, 32, 22, 2, 0, 1, 0),
    (31, 108000, 3600, 18000, 14000, 14000, 512, 16, 60, 4, 1, 1, 0),
    (32, 216000, 5120, 20480, 20000, 20000, 512, 16, 60, 4, 1, 1, 0),
    (40, 245760, 8192, 32768, 20000, 25000, 512, 16, 60, 4, 1, 1, 0),
    (41, 245760, 8192, 32768, 50000, 62500, 512, 16, 24, 2, 1, 1, 0),
    (42, 522240, 8704, 34816, 50000, 62500, 512, 16, 24, 2, 1, 1, 1),
    (50, 589824, 22080, 110400, 135000, 135000, 512, 16, 24, 2, 1, 1, 1),
    (51, 983040, 36864, 184320, 240000, 240000, 512, 16, 24, 2, 1, 1, 1),
    (52, 2073600, 36864, 184320, 240000, 240000, 512, 16, 24, 2, 1, 1, 1),
    (60, 4177920, 139264, 696320, 240000, 240000, 8192, 16, 24, 2, 1, 1, 1),
    (61, 8355840, 139264, 696320, 480000, 480000, 8192, 16, 24, 2, 1, 1, 1),
    (62, 16711680, 139264, 696320, 800000, 800000, 8192, 16, 24, 2, 1, 1, 1)]

## ****************************************************************************
##  MATH
## ***************************************************************************

var x264_exp2_lut*: array[64, uint8] = [0'u8, 3, 6, 8, 11, 14, 17, 20, 23, 26, 29, 32, 36, 39, 42, 45,
                                   48, 52, 55, 58, 62, 65, 69, 72, 76, 80, 83, 87, 91, 94, 98,
                                   102, 106, 110, 114, 118, 122, 126, 130, 135, 139, 143,
                                   147, 152, 156, 161, 165, 170, 175, 179, 184, 189, 194,
                                   198, 203, 208, 214, 219, 224, 229, 234, 240, 245, 250]

var x264_log2_lut*: array[128, cfloat] = [0.0.cfloat, 0.01123, 0.02237, 0.03342, 0.04439,
                                     0.05528, 0.06609, 0.07682, 0.08746,
                                     0.09803000000000001, 0.10852, 0.11894,
                                     0.12928, 0.13955, 0.14975, 0.15987, 0.16993,
                                     0.17991, 0.18982, 0.19967, 0.20945, 0.21917,
                                     0.22882, 0.2384, 0.24793, 0.25739, 0.26679,
                                     0.27612, 0.2854, 0.29462, 0.30378, 0.31288,
                                     0.32193, 0.33092, 0.33985, 0.34873, 0.35755,
                                     0.36632, 0.37504, 0.3837, 0.39232, 0.40088,
                                     0.40939, 0.41785, 0.42626, 0.43463, 0.44294,
                                     0.45121, 0.45943, 0.46761, 0.47573, 0.48382,
                                     0.49185, 0.49985, 0.50779, 0.5157, 0.52356,
                                     0.53138, 0.53916, 0.54689, 0.55459, 0.56224,
                                     0.56986, 0.57743, 0.58496, 0.59246,
                                     0.5999100000000001, 0.60733, 0.61471, 0.62205,
                                     0.62936, 0.63662, 0.64386, 0.65105, 0.65821,
                                     0.66534, 0.67243, 0.67948, 0.6865,
                                     0.6934900000000001, 0.70044, 0.70736,
                                     0.7142500000000001, 0.7211, 0.72792, 0.73471,
                                     0.74147, 0.74819, 0.7548899999999999,
                                     0.7615499999999999, 0.76818, 0.77479,
                                     0.7813600000000001, 0.7879, 0.79442,
                                     0.8008999999999999, 0.80735,
                                     0.8137799999999999, 0.82018, 0.82655, 0.83289,
                                     0.8391999999999999, 0.84549, 0.85175, 0.85798,
                                     0.86419, 0.87036, 0.87652, 0.88264, 0.88874,
                                     0.8948199999999999, 0.9008699999999999,
                                     0.90689, 0.91289, 0.91886, 0.92481, 0.93074,
                                     0.93664, 0.94251, 0.94837, 0.9542, 0.96,
                                     0.96578, 0.97154, 0.97728, 0.98299, 0.98868,
                                     0.99435]

##  Avoid an int/float conversion.

var x264_log2_lz_lut*: array[32, cfloat] = [31.cfloat, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19,
                                       18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3,
                                       2, 1, 0]

## ****************************************************************************
##  ANALYSE
## ***************************************************************************
##  lambda = pow(2,qp/6-2)

var x264_lambda_tab*: array[QP_MAX_MAX + 1, uint16] = [1'u16, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 2, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 6, 6, 7, 8, 9, 10, 11, 13, 14, 16, 18, 20, 23, 25, 29, 32, 36, 40, 45,
    51, 57, 64, 72, 81, 91, 102, 114, 128, 144, 161, 181, 203, 228, 256, 287, 323, 362, 406, 456,
    512, 575, 645, 724, 813, 912, 1024, 1149, 1290, 1448, 1625, 1825, 2048, 2299, 2580, 2896]

##  lambda2 = pow(lambda,2) * .9 * 256
##  Capped to avoid overflow

var x264_lambda2_tab*: array[QP_MAX_MAX + 1, cint] = [14.cint, 18, 22, 28, 36, 45, 57, 72, 91, 115,
    145, 182, 230, 290, 365, 460, 580, 731, 921, 1161, 1462, 1843, 2322, 2925, 3686, 4644, 5851,
    7372, 9289, 11703, 14745, 18578, 23407, 29491, 37156, 46814, 58982, 74313, 93628,
    117964, 148626, 187257, 235929, 297252, 374514, 471859, 594505, 749029, 943718,
    1189010, 1498059, 1887436, 2378021, 2996119, 3774873, 4756042, 5992238, 7549747,
    9512085, 11984476, 15099494, 19024170, 23968953, 30198988, 38048341, 47937906,
    60397977, 76096683, 95875813, 120795955, 134217727, 134217727, 134217727,
    134217727, 134217727, 134217727, 134217727, 134217727, 134217727, 134217727,
    134217727, 134217727]

##  should the intra and inter lambdas be different?
##  I'm just matching the behaviour of deadzone quant.

var x264_trellis_lambda2_tab*: array[2, seq[cint]] = [
  @[46.cint, 58, 73, 92,117, 147, 185, 233, 294, 
  370, 466, 587, 740, 932, 1174, 1480, 1864, 2349, 
  2959, 3728, 4697, 5918, 7457, 9395, 11837, 14914, 18790, 
  23674, 29828, 37581, 47349, 59656, 75163, 94699, 119313, 150326, 
  189399, 238627, 300652, 378798, 477255, 601304, 757596, 954511, 1202608, 
  1515192, 1909022, 2405217, 3030384, 3818045, 4810435, 6060769, 7636091, 9620872, 
  12121539, 15272182, 19241743, 24243077, 30544363, 38483486, 48486154,61088726, 76966972, 
  96972308, 122177453, 134217727, 134217727, 134217727, 134217727, 134217727, 134217727, 134217727, 
  134217727, 134217727, 134217727,134217727], 
    @[27.cint, 34, 43, 54, 68, 86, 108, 136, 172, 
    216, 273, 343, 433, 545, 687, 865, 1090,1374, 
    1731, 2180, 2747, 3461, 4361, 5494, 6922, 8721, 10988, 
    13844, 17442,21976, 27688, 34885, 43953, 55377, 69771, 87906, 
    110755, 139543, 175813, 221511, 279087, 351627, 443023, 558174, 703255, 886046, 
    1116348,1406511, 1772093, 2232697, 2813022, 3544186, 4465396, 5626046, 7088374,
    8930791, 11252092, 14176748, 17861583, 22504184, 28353495, 35723165, 45008368, 56706990, 
    71446330, 90016736, 113413980, 134217727,134217727, 134217727, 134217727, 134217727, 134217727, 
    134217727, 134217727, 134217727, 134217727, 134217727, 134217727, 134217727,134217727, 134217727]]

var x264_chroma_lambda2_offset_tab*: array[MAX_CHROMA_LAMBDA_OFFSET + 1, uint16] = [
    16'u16, 20, 25, 32, 40, 50, 64, 80, 101, 128, 161, 203, 256, 322, 406, 512, 645, 812, 1024, 1290,
    1625, 2048, 2580, 3250, 4096, 5160, 6501, 8192, 10321, 13003, 16384, 20642, 26007, 32768,
    41285, 52015, 65535]

##  inter lambda = .85 * .85 * 2**(qp/3. + 10 - LAMBDA_BITS)
## ****************************************************************************
##  MC
## ***************************************************************************

var x264_hpel_ref0*: array[16, uint8] = [0'u8, 1, 1, 1, 0, 1, 1, 1, 2, 3, 3, 3, 0, 1, 1, 1]

var x264_hpel_ref1*: array[16, uint8] = [0'u8, 0, 1, 0, 2, 2, 3, 2, 2, 2, 3, 2, 2, 2, 3, 2]

## ****************************************************************************
##  CQM
## ***************************************************************************
##  default quant matrices

var x264_cqm_jvt4i*: seq[uint8] = @[6'u8, 13, 20, 28, 13, 20, 28, 32, 20, 28, 32, 37, 28, 32, 37,
                                    42]

var x264_cqm_jvt4p*: seq[uint8] = @[10'u8, 14, 20, 24, 14, 20, 24, 27, 20, 24, 27, 30, 24, 27,
                                    30, 34]

var x264_cqm_jvt8i*: seq[uint8] = @[6'u8, 10, 13, 16, 18, 23, 25, 27, 10, 11, 16, 18, 23, 25, 27,
                                    29, 13, 16, 18, 23, 25, 27, 29, 31, 16, 18, 23, 25, 27,
                                    29, 31, 33, 18, 23, 25, 27, 29, 31, 33, 36, 23, 25, 27,
                                    29, 31, 33, 36, 38, 25, 27, 29, 31, 33, 36, 38, 40, 27,
                                    29, 31, 33, 36, 38, 40, 42]

var x264_cqm_jvt8p*: seq[uint8] = @[9'u8, 13, 15, 17, 19, 21, 22, 24, 13, 13, 17, 19, 21, 22, 24,
                                    25, 15, 17, 19, 21, 22, 24, 25, 27, 17, 19, 21, 22, 24,
                                    25, 27, 28, 19, 21, 22, 24, 25, 27, 28, 30, 21, 22, 24,
                                    25, 27, 28, 30, 32, 22, 24, 25, 27, 28, 30, 32, 33, 24,
                                    25, 27, 28, 30, 32, 33, 35]

var x264_cqm_flat16*: seq[uint8] = @[16'u8, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
                                     16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
                                     16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
                                     16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
                                     16, 16, 16, 16, 16, 16, 16, 16]

var x264_cqm_jvt*: array[8, seq[uint8]] = [x264_cqm_jvt4i, x264_cqm_jvt4p,
                                    x264_cqm_jvt4i, x264_cqm_jvt4p,
                                    x264_cqm_jvt8i, x264_cqm_jvt8p,
                                    x264_cqm_jvt8i, x264_cqm_jvt8p]

var x264_cqm_avci50_4ic*: array[16, uint8] = [16'u8, 22, 28, 40, 22, 28, 40, 44, 28, 40, 44, 48, 40,
    44, 48, 60]

var x264_cqm_avci50_p_8iy*: array[64, uint8] = [16'u8, 18, 19, 21, 24, 27, 30, 33, 18, 19, 21, 24,
    27, 30, 33, 78, 19, 21, 24, 27, 30, 33, 78, 81, 21, 24, 27, 30, 33, 78, 81, 84, 24, 27, 30, 33, 78,
    81, 84, 87, 27, 30, 33, 78, 81, 84, 87, 90, 30, 33, 78, 81, 84, 87, 90, 93, 33, 78, 81, 84, 87, 90,
    93, 96]

var x264_cqm_avci50_1080i_8iy*: array[64, uint8] = [16'u8, 18, 19, 21, 27, 33, 81, 87, 18, 19, 21,
    24, 30, 33, 81, 87, 19, 21, 24, 27, 30, 78, 84, 90, 21, 24, 27, 30, 33, 78, 84, 90, 24, 27, 30, 33,
    78, 81, 84, 90, 24, 27, 30, 33, 78, 81, 84, 93, 27, 30, 33, 78, 78, 81, 87, 93, 30, 33, 33, 78, 81,
    84, 87, 96]

var x264_cqm_avci100_720p_4ic*: array[16, uint8] = [16'u8, 21, 27, 34, 21, 27, 34, 41, 27, 34, 41,
    46, 34, 41, 46, 54]

var x264_cqm_avci100_720p_8iy*: array[64, uint8] = [16'u8, 18, 19, 21, 22, 24, 26, 32, 18, 19, 19,
    21, 22, 24, 26, 32, 19, 19, 21, 22, 22, 24, 26, 32, 21, 21, 22, 22, 23, 24, 26, 34, 22, 22, 22, 23,
    24, 25, 26, 34, 24, 24, 24, 24, 25, 26, 34, 36, 26, 26, 26, 26, 26, 34, 36, 38, 32, 32, 32, 34, 34,
    36, 38, 42]

var x264_cqm_avci100_1080_4ic*: array[16, uint8] = [16'u8, 20, 26, 32, 20, 26, 32, 38, 26, 32, 38,
    44, 32, 38, 44, 50]

var x264_cqm_avci100_1080i_8iy*: array[64, uint8] = [16'u8, 19, 20, 23, 24, 26, 32, 42, 18, 19,
    22, 24, 26, 32, 36, 42, 18, 20, 23, 24, 26, 32, 36, 63, 19, 20, 23, 26, 32, 36, 42, 63, 20, 22, 24,
    26, 32, 36, 59, 63, 22, 23, 24, 26, 32, 36, 59, 68, 22, 23, 24, 26, 32, 42, 59, 68, 22, 23, 24, 26,
    36, 42, 59, 72]

var x264_cqm_avci100_1080p_8iy*: array[64, uint8] = [16'u8, 18, 19, 20, 22, 23, 24, 26, 18, 19,
    20, 22, 23, 24, 26, 32, 19, 20, 22, 23, 24, 26, 32, 36, 20, 22, 23, 24, 26, 32, 36, 42, 22, 23, 24,
    26, 32, 36, 42, 59, 23, 24, 26, 32, 36, 42, 59, 63, 24, 26, 32, 36, 42, 59, 63, 68, 26, 32, 36, 42,
    59, 63, 68, 72]


var x264_decimate_table4*: array[16, uint8] = [3'u8, 2, 2, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]

var x264_decimate_table8*: array[64, uint8] = [3'u8, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0]

var x264_scan8*: array[16 * 3 + 3, uint8] = [4'u8 + 1 * 8, 5 + 1 * 8, 4 + 2 * 8, 5 + 2 * 8, 6 + 1 * 8, 7 + 1 * 8,
                                    6 + 2 * 8, 7 + 2 * 8, 4 + 3 * 8, 5 + 3 * 8, 4 + 4 * 8, 5 + 4 * 8,
                                    6 + 3 * 8, 7 + 3 * 8, 6 + 4 * 8, 7 + 4 * 8, 4 + 6 * 8, 5 + 6 * 8,
                                    4 + 7 * 8, 5 + 7 * 8, 6 + 6 * 8, 7 + 6 * 8, 6 + 7 * 8, 7 + 7 * 8,
                                    4 + 8 * 8, 5 + 8 * 8, 4 + 9 * 8, 5 + 9 * 8, 6 + 8 * 8, 7 + 8 * 8,
                                    6 + 9 * 8, 7 + 9 * 8, 4 + 11 * 8, 5 + 11 * 8, 4 + 12 * 8, 5 + 12 * 8,
                                    6 + 11 * 8, 7 + 11 * 8, 6 + 12 * 8, 7 + 12 * 8, 4 + 13 * 8,
                                    5 + 13 * 8, 4 + 14 * 8, 5 + 14 * 8, 6 + 13 * 8, 7 + 13 * 8,
                                    6 + 14 * 8, 7 + 14 * 8, 0 + 0 * 8, 0 + 5 * 8, 0 + 10 * 8]

type
  weight_fn_t* = proc (a1: uint16; a2: uint64; a3: uint16; a4: uint64; a5: x264_weight_t; a6: cint)

  x264_weight_t* = ref object
    cachea*: array[8, int16]
    cacheb*: array[8, int16]
    i_denom*: int32
    i_scale*: int32
    i_offset*: int32
    weightfn*: weight_fn_t

  x264_me_t* = ref object
    i_pixel*: cint             ##  PIXEL_WxH
    p_cost_mv*: uint16      ##  lambda * nbits for each possible mv
    i_ref_cost*: cint
    i_ref*: cint
    weight*:x264_weight_t
    p_fref*: array[12, uint16]
    p_fref_w*: uint16
    p_fenc*: array[3, uint16]
    integral*: uint16
    i_stride*: array[3, cint]
    mvp*: array[2, int16]
    cost_mv*: cint             ##  lambda * nbits for the chosen mv
    cost*: cint                ##  satd + lambda * nbits
    mv*: array[2, int16]

  x264_mc_functions_t* = ref object
    mc_luma*: proc (dst: uint16; i_dst: uint64; src: ptr uint16; i_src: uint64;
                  mvx: cint; mvy: cint; i_width: cint; i_height: cint;
                  weight:x264_weight_t) ##  may round up the dimensions if they're not a power of 2
    get_ref*: proc (dst: uint16; i_dst: ptr uint64; src: ptr uint16; i_src: uint64;
                  mvx: cint; mvy: cint; i_width: cint; i_height: cint;
                  weight:x264_weight_t): uint16
    mc_chroma*: proc (dstu: uint16; dstv: uint16; i_dst: uint64; src: uint16;
                    i_src: uint64; mvx: cint; mvy: cint; i_width: cint; i_height: cint) ##  void (*avg[12])( uint16 *dst,  uint64 dst_stride, uint16 *src1, uint64 src1_stride, uint16 *src2, uint64 src2_stride, int i_weight );
                                                                              ##  void (*copy[7])( uint16 *dst, uint64 dst_stride, uint16 *src, uint64 src_stride, int i_height );
    copy_16x16_unaligned*: proc (dst: uint16; dst_stride: uint64; src: uint16;
                               src_stride: uint64; i_height: cint)
    store_interleave_chroma*: proc (dst: uint16; i_dst: uint64; srcu: uint16;
                                  srcv: uint16; height: cint)
    load_deinterleave_chroma_fenc*: proc (dst: uint16; src: uint16;
                                        i_src: uint64; height: cint)
    load_deinterleave_chroma_fdec*: proc (dst: uint16; src: uint16;
                                        i_src: uint64; height: cint)
    plane_copy*: proc (dst: uint16; i_dst: uint64; src: uint16; i_src: uint64;
                     w: cint; h: cint)
    plane_copy_swap*: proc (dst: uint16; i_dst: uint64; src: uint16;
                          i_src: uint64; w: cint; h: cint)
    plane_copy_interleave*: proc (dst: uint16; i_dst: uint64; srcu: uint16;
                                i_srcu: uint64; srcv: uint16; i_srcv: uint64;
                                w: cint; h: cint) ##  may write up to 15 pixels off the end of each plane
    plane_copy_deinterleave*: proc (dstu: uint16; i_dstu: uint64; dstv: uint16;
                                  i_dstv: uint64; src: uint16; i_src: uint64;
                                  w: cint; h: cint)
    plane_copy_deinterleave_yuyv*: proc (dsta: uint16; i_dsta: uint64;
                                       dstb: uint16; i_dstb: uint64;
                                       src: uint16; i_src: uint64; w: cint; h: cint)
    plane_copy_deinterleave_rgb*: proc (dsta: uint16; i_dsta: uint64;
                                      dstb: uint16; i_dstb: uint64;
                                      dstc: uint16; i_dstc: uint64;
                                      src: uint16; i_src: uint64; pw: cint;
                                      w: cint; h: cint)
    plane_copy_deinterleave_v210*: proc (dsty: uint16; i_dsty: uint64;
                                       dstc: uint16; i_dstc: uint64;
                                       src: uint32; i_src: uint64; w: cint;
                                       h: cint)
    hpel_filter*: proc (dsth: uint16; dstv: uint16; dstc: uint16;
                      src: uint16; i_stride: uint64; i_width: cint; i_height: cint;
                      buf: int16) ##  prefetch the next few macroblocks of fenc or fdec
    prefetch_fenc*: proc (pix_y: uint16; stride_y: uint64; pix_uv: uint16;
                        stride_uv: uint64; mb_x: cint)
    prefetch_fenc_400*: proc (pix_y: uint16; stride_y: uint64; pix_uv: uint16;
                            stride_uv: uint64; mb_x: cint)
    prefetch_fenc_420*: proc (pix_y: uint16; stride_y: uint64; pix_uv: uint16;
                            stride_uv: uint64; mb_x: cint)
    prefetch_fenc_422*: proc (pix_y: uint16; stride_y: uint64; pix_uv: uint16;
                            stride_uv: uint64; mb_x: cint) ##  prefetch the next few macroblocks of a hpel reference frame
    prefetch_ref*: proc (pix: uint16; stride: uint64; parity: cint)
    memcpy_aligned*: proc (dst: pointer; src: pointer; n: csize_t): pointer
    memzero_aligned*: proc (dst: pointer; n: csize_t) ##  successive elimination prefilter
    integral_init4h*: proc (sum: uint16; pix: uint16; stride: uint64)
    integral_init8h*: proc (sum: uint16; pix: uint16; stride: uint64)
    integral_init4v*: proc (sum8: uint16; sum4: uint16; stride: uint64)
    integral_init8v*: proc (sum8: uint16; stride: uint64)
    frame_init_lowres_core*: proc (src0: uint16; dst0: uint16; dsth: uint16;
                                 dstv: uint16; dstc: uint16;
                                 src_stride: uint64; dst_stride: uint64;
                                 width: cint; height: cint)
    weight*: weight_fn_t
    offsetadd*: weight_fn_t
    offsetsub*: weight_fn_t
    weight_cache*: proc (a1: x264_t; a2:x264_weight_t)
    mbtree_propagate_cost*: proc (dst: int16; propagate_in: uint16;
                                intra_costs: uint16; inter_costs: uint16;
                                inv_qscales: uint16; fps_factor: cfloat;
                                len: cint)
    mbtree_propagate_list*: proc (h: x264_t; ref_costs: uint16;
                                mvs: array[2, int16]; propagate_amount: int16;
                                lowres_costs: uint16; bipred_weight: cint;
                                mb_y: cint; len: cint; list: cint)
    mbtree_fix8_pack*: proc (dst: uint16; src: cfloat; count: cint)
    mbtree_fix8_unpack*: proc (dst: cfloat; src: uint16; count: cint)
    
  X264Opaque* = ref object
    reordered_opaque*: int64
    wallclock*: int64

  X264ParamVui* = ref object
    i_sar_height*: cint
    i_sar_width*: cint
    i_overscan*: cint          ##  0=undef, 1=no overscan, 2=overscan
    i_vidformat*: cint
    b_fullrange*: cint
    i_colorprim*: cint
    i_transfer*: cint
    i_colmatrix*: cint
    i_chroma_loc*: cint

  X264ParamAnalyse* = ref object
    intra*: cuint              ##  intra partitions
    inter*: cuint              ##  inter partitions
    b_transform_8x8*: cint
    i_weighted_pred*: cint     ##  weighting for P-frames
    b_weighted_bipred*: cint   ##  implicit weighting for B-frames
    i_direct_mv_pred*: cint    ##  spatial vs temporal mv prediction
    i_chroma_qp_offset*: cint
    i_me_method*: cint         ##  motion estimation algorithm to use (X264_ME_*)
    i_me_range*: cint          ##  integer uint16 motion estimation search range (from predicted mv)
    i_mv_range*: cint          ##  maximum length of a mv (in pixels). -1 = auto, based on level
    i_mv_range_thread*: cint   ##  minimum space between threads. -1 = auto, based on number of threads.
    i_subpel_refine*: cint     ##  subpixel motion estimation quality
    b_chroma_me*: cint         ##  chroma ME for subpel and mode decision in P-frames
    b_mixed_references*: cint  ##  allow each mb partition to have its own reference number
    i_trellis*: cint           ##  trellis RD quantization
    b_fast_pskip*: cint        ##  early SKIP detection on P-frames
    b_dct_decimate*: cint      ##  transform coefficient thresholding on P-frames
    i_noise_reduction*: cint   ##  adaptive pseudo-deadzone
    f_psy_rd*: cfloat          ##  Psy RD strength
    f_psy_trellis*: cfloat     ##  Psy trellis strength
    b_psy*: cint               ##  Toggle all psy optimizations
    b_mb_info*: cint           ##  Use input mb_info data in x264_picture_t
    b_mb_info_update*: cint ##  Update the values in mb_info according to the results of encoding.
                          ##  the deadzone size that will be used in luma quantization
    i_luma_deadzone*: array[2, cint] ##  {inter, intra}
    b_psnr*: cint              ##  compute and print PSNR stats
    b_ssim*: cint              ##  compute and print SSIM stats

  X264ParamRC* = ref object
    i_rc_method*: cint         ##  X264_RC_*
    i_qp_constant*: cint       ##  0=lossless
    i_qp_min*: cint            ##  min allowed QP value
    i_qp_max*: cint            ##  max allowed QP value
    i_qp_step*: cint           ##  max QP step between frames
    i_bitrate*: cint
    f_rf_constant*: cfloat     ##  1pass VBR, nominal QP
    f_rf_constant_max*: cfloat ##  In CRF mode, maximum CRF as caused by VBV
    f_rate_tolerance*: cfloat
    i_vbv_max_bitrate*: cint
    i_vbv_buffer_size*: cint
    f_vbv_buffer_init*: cfloat ##  <=1: fraction of buffer_size. >1: kbit
    f_ip_factor*: cfloat
    f_pb_factor*: cfloat
    b_filler*: cint
    i_aq_mode*: cint           ##  psy adaptive QP. (X264_AQ_*)
    f_aq_strength*: cfloat
    b_mb_tree*: cint           ##  Macroblock-tree ratecontrol.
    i_lookahead*: cint
    b_stat_write*: cint        ##  Enable stat writing in psz_stat_out
    psz_stat_out*: cstring     ##  output filename (in UTF-8) of the 2pass stats file
    b_stat_read*: cint         ##  Read stat from psz_stat_in and use it
    psz_stat_in*: cstring      ##  input filename (in UTF-8) of the 2pass stats file
    f_qcompress*: cfloat       ##  0.0 => cbr, 1.0 => constant qp
    f_qblur*: cfloat           ##  temporally blur quants
    f_complexity_blur*: cfloat ##  temporally blur complexity
    zones*: ptr x264_zone_t     ##  ratecontrol overrides
    i_zones*: cint             ##  number of zone_t's
    psz_zones*: cstring        ##  alternate method of specifying zones

  X264ParamCropRect* = ref object
    i_left*: cint
    i_top*: cint
    i_right*: cint
    i_bottom*: cint

  x264_zone_t* = ref object
    i_start*: cint
    i_end*: cint               ##  range of frame numbers
    b_force_qp*: cint          ##  whether to use qp vs bitrate factor
    i_qp*: cint
    f_bitrate_factor*: cfloat
    param*: x264_param_t

  X264SpsCrop* = ref object
    i_left*: cint
    i_right*: cint
    i_top*: cint
    i_bottom*: cint

  X264SpsVuiHrd* = ref object
    i_cpb_cnt*: cint
    i_bit_rate_scale*: cint
    i_cpb_size_scale*: cint
    i_bit_rate_value*: cint
    i_cpb_size_value*: cint
    i_bit_rate_unscaled*: cint
    i_cpb_size_unscaled*: cint
    b_cbr_hrd*: cint
    i_initial_cpb_removal_delay_length*: cint
    i_cpb_removal_delay_length*: cint
    i_dpb_output_delay_length*: cint
    i_time_offset_length*: cint

  X264SpsVui* = ref object
    b_aspect_ratio_info_present*: cint
    i_sar_width*: cint
    i_sar_height*: cint
    b_overscan_info_present*: cint
    b_overscan_info*: cint
    b_signal_type_present*: cint
    i_vidformat*: cint
    b_fullrange*: cint
    b_color_description_present*: cint
    i_colorprim*: cint
    i_transfer*: cint
    i_colmatrix*: cint
    b_chroma_loc_info_present*: cint
    i_chroma_loc_top*: cint
    i_chroma_loc_bottom*: cint
    b_timing_info_present*: cint
    i_num_units_in_tick*: uint32
    i_time_scale*: uint32
    b_fixed_frame_rate*: cint
    b_nal_hrd_parameters_present*: cint
    b_vcl_hrd_parameters_present*: cint
    hrd*: X264SpsVuiHrd
    b_pic_struct_present*: cint
    b_bitstream_restriction*: cint
    b_motion_vectors_over_pic_boundaries*: cint
    i_max_bytes_per_pic_denom*: cint
    i_max_bits_per_mb_denom*: cint
    i_log2_max_mv_length_horizontal*: cint
    i_log2_max_mv_length_vertical*: cint
    i_num_reorder_frames*: cint
    i_max_dec_frame_buffering*: cint ##  FIXME to complete

  X264SliceHeaderRefPicListOrder* = ref object
    idc*: cint
    arg*: cint

  X264SliceHeadermmco* = ref object
    i_difference_of_pic_nums*: cint
    i_poc*: cint

  x264_sps_t* = ref object
    i_id*: cint
    i_profile_idc*: cint
    i_level_idc*: cint
    b_constraint_set0*: cint
    b_constraint_set1*: cint
    b_constraint_set2*: cint
    b_constraint_set3*: cint
    i_log2_max_frame_num*: cint
    i_poc_type*: cint          ##  poc 0
    i_log2_max_poc_lsb*: cint
    i_num_ref_frames*: cint
    b_gaps_in_frame_num_value_allowed*: cint
    i_mb_width*: cint
    i_mb_height*: cint
    b_frame_mbs_only*: cint
    b_mb_adaptive_frame_field*: cint
    b_direct8x8_inference*: cint
    b_crop*: cint
    crop*: X264SpsCrop
    b_vui*: cint
    vui*: X264SpsVui
    b_qpprime_y_zero_transform_bypass*: cint
    i_chroma_format_idc*: cint
    b_avcintra*: cint
    i_cqm_preset*: cint
    scaling_list*: array[8, uint8] ##  could be 12, but we don't allow separate Cb/Cr lists

  x264_pps_t* = ref object
    i_id*: cint
    i_sps_id*: cint
    b_cabac*: cint
    b_pic_order*: cint
    i_num_slice_groups*: cint
    i_num_ref_idx_l0_default_active*: cint
    i_num_ref_idx_l1_default_active*: cint
    b_weighted_pred*: cint
    b_weighted_bipred*: cint
    i_pic_init_qp*: cint
    i_pic_init_qs*: cint
    i_chroma_qp_index_offset*: cint
    b_deblocking_filter_control*: cint
    b_constrained_intra_pred*: cint
    b_redundant_pic_cnt*: cint
    b_transform_8x8_mode*: cint

  x264_slice_header_t* = ref object
    sps*: ptr x264_sps_t
    pps*: ptr x264_pps_t
    i_type*: cint
    i_first_mb*: cint
    i_last_mb*: cint
    i_pps_id*: cint
    i_frame_num*: cint
    b_mbaff*: cint
    b_field_pic*: cint
    b_bottom_field*: cint
    i_idr_pic_id*: cint        ##  -1 if nal_type != 5
    i_poc*: cint
    i_delta_poc_bottom*: cint
    i_delta_poc*: array[2, cint]
    i_redundant_pic_cnt*: cint
    b_direct_spatial_mv_pred*: cint
    b_num_ref_idx_override*: cint
    i_num_ref_idx_l0_active*: cint
    i_num_ref_idx_l1_active*: cint
    b_ref_pic_list_reordering*: array[2, cint]
    ref_pic_list_order*: array[2, array[X264_REF_MAX, X264SliceHeaderRefPicListOrder]] ##  P-frame weighting
    b_weighted_pred*: cint
    weight*: array[X264_REF_MAX * 2, array[3, x264_weight_t]]
    i_mmco_remove_from_end*: cint
    i_mmco_command_count*: cint
    mmco*: array[X264_REF_MAX, X264SliceHeadermmco]
    i_cabac_init_idc*: cint
    i_qp*: cint
    i_qp_delta*: cint
    b_sp_for_swidth*: cint
    i_qs_delta*: cint          ##  deblocking filter
    i_disable_deblocking_filter_idc*: cint
    i_alpha_c0_offset*: cint
    i_beta_offset*: cint
  x264_pthread_mutex_t* = CRITICAL_SECTION
  x264_pthread_t* = ref object
    handle*: pointer
    `func`*: proc (arg: pointer): pointer
    arg*: pointer
    p_ret*: ptr pointer
    ret*: pointer

  x264_pthread_cond_t* = ref object
    Ptr*: pointer

  x264_hrd_t* = ref object
    cpb_initial_arrival_time*: cdouble
    cpb_final_arrival_time*: cdouble
    cpb_removal_time*: cdouble
    dpb_output_time*: cdouble

  x264_sei_payload_t* = ref object
    payload_size*: cint
    payload_type*: cint
    payload*: uint8

  x264_sei_t* = ref object
    num_payloads*: cint
    payloads*: ptr x264_sei_payload_t ##  In: optional callback to free each payload AND x264_sei_payload_t when used.
    sei_free*: proc (a1: pointer)

  x264_image_t* = ref object
    i_csp*: cint               ##  Colorspace
    i_plane*: cint             ##  Number of image planes
    i_stride*: array[4, cint]   ##  Strides for each plane
    plane*: array[4, uint8] ##  Pointers to each plane

  x264_image_properties_t* = ref object
    quant_offsets*: cfloat ##  All arrays of data here are ordered as follows:
                            ##  each array contains one offset per macroblock, in raster scan order.  In interlaced
                            ##  mode, top-field MBs and bottom-field MBs are interleaved at the row level.
                            ##  Macroblocks are 16x16 blocks of pixels (with respect to the luma plane).  For the
                            ##  purposes of calculating the number of macroblocks, width and height are rounded up to
                            ##  the nearest 16.  If in interlaced mode, height is rounded up to the nearest 32 instead.
                            ##  In: an array of quantizer offsets to be applied to this image during encoding.
                            ##      These are added on top of the decisions made by x264.
                            ##      Offsets can be fractional; they are added before QPs are rounded to integer.
                            ##      Adaptive quantization must be enabled to use this feature.  Behavior if quant
                            ##      offsets differ between encoding passes is undefined.
    ##  In: optional callback to free quant_offsets when used.
    ##      Useful if one wants to use a different quant_offset array for each frame.
    quant_offsets_free*: proc (a1: pointer) ##  In: optional array of flags for each macroblock.
                                        ##      Allows specifying additional information for the encoder such as which macroblocks
                                        ##      remain unchanged.  Usable flags are listed below.
                                        ##      x264_param_t.analyse.b_mb_info must be set to use this, since x264 needs to track
                                        ##      extra data internally to make full use of this information.
                                        ##
                                        ##  Out: if b_mb_info_update is set, x264 will update this array as a result of encoding.
                                        ##
                                        ##       For "MBINFO_CONSTANT", it will remove this flag on any macroblock whose decoded
                                        ##       pixels have changed.  This can be useful for e.g. noting which areas of the
                                        ##       frame need to actually be blitted. Note: this intentionally ignores the effects
                                        ##       of deblocking for the current frame, which should be fine unless one needs exact
                                        ##       uint16-perfect accuracy.
                                        ##
                                        ##       Results for MBINFO_CONSTANT are currently only set for P-frames, and are not
                                        ##       guaranteed to enumerate all blocks which haven't changed.  (There may be false
                                        ##       negatives, but no false positives.)
                                        ##
    mb_info*: uint8       ##  In: optional callback to free mb_info when used.
    mb_info_free*: proc (a1: pointer) ##  The macroblock is constant and remains unchanged from the previous frame.
                                  ##  More flags may be added in the future.
                                  ##  Out: SSIM of the the frame luma (if x264_param_t.b_ssim is set)
    f_ssim*: cdouble           ##  Out: Average PSNR of the frame (if x264_param_t.b_psnr is set)
    f_psnr_avg*: cdouble       ##  Out: PSNR of Y, U, and V (if x264_param_t.b_psnr is set)
    f_psnr*: array[3, cdouble]  ##  Out: Average effective CRF of the encoded frame
    f_crf_avg*: cdouble

  x264_picture_t* = ref object
    i_type*: cint ##  In: force picture type (if not auto)
                ##      If x264 encoding parameters are violated in the forcing of picture types,
                ##      x264 will correct the input picture type and log a warning.
                ##  Out: type of the picture encoded
    ##  In: force quantizer for != X264_QP_AUTO
    i_qpplus1*: cint ##  In: pic_struct, for pulldown/doubling/etc...used only if b_pic_struct=1.
                   ##      use pic_struct_e for pic_struct inputs
                   ##  Out: pic_struct element associated with frame
    i_pic_struct*: cint ##  Out: whether this frame is a keyframe.  Important when using modes that result in
                      ##  SEI recovery points being used instead of IDR frames.
    b_keyframe*: cint          ##  In: user pts, Out: pts of encoded picture (user)
    i_pts*: int64 ##  Out: frame dts. When the pts of the first frame is close to zero,
                  ##       initial frames may have a negative dts which must be dealt with by any muxer
    i_dts*: int64 ##  In: custom encoding parameters to be set from this frame forwards
                  ##            (in coded order, not display order). If NULL, continue using
                  ##            parameters from the previous frame.  Some parameters, such as
                  ##            aspect ratio, can only be changed per-GOP due to the limitations
                  ##            of H.264 itself; in this case, the caller must force an IDR frame
                  ##            if it needs the changed parameter to apply immediately.
    param*: x264_param_t ##  In: raw image data
                          ##  Out: reconstructed image data.  x264 may skip part of the reconstruction process,
                          ##             e.g. deblocking, in frames where it isn't necessary.  To force complete
                          ##             reconstruction, at a small speed cost, set b_full_recon.
    img*: x264_image_t ##  In: optional information to modify encoder decisions for this frame
                     ##  Out: information about the encoded frame
    prop*: x264_image_properties_t ##  Out: HRD timing information. Output only when i_nal_hrd is set.
    hrd_timing*: x264_hrd_t    ##  In: arbitrary user SEI (e.g subtitles, AFDs)
    extra_sei*: x264_sei_t     ##  private user data. copied from input to output frames.
    opaque*: pointer

  x264_frame_t* = ref object
    base*: uint8            ##  Base pointer for all malloced data in this frame.
    i_poc*: cint
    i_delta_poc*: array[2, cint]
    i_type*: cint
    i_forced_type*: cint
    i_qpplus1*: cint
    i_pts*: int64
    i_dts*: int64
    i_reordered_pts*: int64
    i_duration*: int64         ##  in SPS time_scale units (i.e 2 * timebase units) used for vfr
    f_duration*: cfloat        ##  in seconds
    i_cpb_duration*: int64
    i_cpb_delay*: int64        ##  in SPS time_scale units (i.e 2 * timebase units)
    i_dpb_output_delay*: int64
    param*: x264_param_t
    i_frame*: cint             ##  Presentation frame number
    i_coded*: cint             ##  Coded frame number
    i_field_cnt*: int64        ##  Presentation field count
    i_frame_num*: cint         ##  7.4.3 frame_num
    b_kept_as_ref*: cint
    i_pic_struct*: cint
    b_keyframe*: cint
    b_fdec*: uint8
    b_last_minigop_bframe*: uint8 ##  this frame is the last b in a sequence of bframes
    i_bframes*: uint8          ##  number of bframes following this nonb in coded order
    f_qp_avg_rc*: cfloat       ##  QPs as decided by ratecontrol
    f_qp_avg_aq*: cfloat       ##  QPs as decided by AQ in addition to ratecontrol
    f_crf_avg*: cfloat         ##  Average effective CRF for this frame
    i_poc_l0ref0*: cint ##  poc of first refframe in L0, used to check if direct temporal is possible
                      ##  YUV buffer
    i_csp*: cint               ##  Internal csp
    i_plane*: cint
    i_stride*: array[3, cint]
    i_width*: array[3, cint]
    i_lines*: array[3, cint]
    i_stride_lowres*: cint
    i_width_lowres*: cint
    i_lines_lowres*: cint
    plane*: array[3, int16]
    plane_fld*: array[3, int16]
    filtered*: array[3, array[4, int16]] ##  plane[0], H, V, HV
    filtered_fld*: array[3, array[4, int16]]
    lowres*: array[4, int16] ##  half-size copy of input frame: Orig, H, V, HV
    integral*: uint16 ##  for unrestricted mv we allocate more data than needed
                         ##  allocated data are stored in buffer
    buffer*: array[4, int16]
    buffer_fld*: array[4, int16]
    buffer_lowres*: int16
    weight*: array[X264_REF_MAX, array[3, x264_weight_t]] ##  [ref_index][plane]
    weighted*: array[X264_REF_MAX, int16] ##  plane[0] weighted of the reference frames
    b_duplicate*: cint
    orig*: x264_frame_t       ##  motion data
    mb_type*: int8
    mb_partition*: uint8    ##  int16 (*mv[2])[2];
                          ##  int16 (*mv16x16)[2];
                          ##  int16 (*lowres_mvs[2][X264_BFRAME_MAX+1])[2];
    field*: uint8
    effective_qp*: uint8    ##  uint16 (*lowres_costs[X264_BFRAME_MAX+2][X264_BFRAME_MAX+2]);
    lowres_mv_costs*: array[2, array[X264_BFRAME_MAX + 1, ptr cint]]
    r*: array[2, int8]
    i_ref*: array[2, cint]
    ref_poc*: array[2, array[X264_REF_MAX, cint]]
    inv_ref_poc*: array[2, int16] ##  inverse values of ref0 poc to avoid divisions in temporal MV prediction
                                 ##  for adaptive B-frame decision.
                                 ##  contains the SATD cost of the lowres frame encoded in various modes
                                 ##  FIXME: how big an array do we need?
    i_cost_est*: array[X264_BFRAME_MAX + 2, array[X264_BFRAME_MAX + 2, cint]]
    i_cost_est_aq*: array[X264_BFRAME_MAX + 2, array[X264_BFRAME_MAX + 2, cint]]
    i_satd*: cint              ##  the i_cost_est of the selected frametype
    i_intra_mbs*: array[X264_BFRAME_MAX + 2, cint]
    i_row_satds*: array[X264_BFRAME_MAX + 2, array[X264_BFRAME_MAX + 2, ptr cint]]
    i_row_satd*: ptr cint
    i_row_bits*: ptr cint
    f_row_qp*: cfloat
    f_row_qscale*: cfloat
    f_qp_offset*: cfloat
    f_qp_offset_aq*: cfloat
    b_intra_calculated*: cint
    i_intra_cost*: uint16
    i_propagate_cost*: uint16
    i_inv_qscale_factor*: uint16
    b_scenecut*: cint          ##  Set to zero if the frame cannot possibly be part of a real scenecut.
    f_weighted_cost_delta*: array[X264_BFRAME_MAX + 2, cfloat]
    i_pixel_sum*: array[3, uint32]
    i_pixel_ssd*: array[3, uint64] ##  hrd
    hrd_timing*: x264_hrd_t    ##  vbv
    i_planned_type*: array[X264_LOOKAHEAD_MAX + 1, uint8]
    i_planned_satd*: array[X264_LOOKAHEAD_MAX + 1, cint]
    f_planned_cpb_duration*: array[X264_LOOKAHEAD_MAX + 1, cdouble]
    i_coded_fields_lookahead*: int64
    i_cpb_delay_lookahead*: int64 ##  threading
    i_lines_completed*: cint   ##  in pixels
    i_lines_weighted*: cint    ##  FIXME: this only supports weighting of one reference frame
    i_reference_count*: cint   ##  number of threads using this frame (not necessarily the number of pointers)
    mutex*: x264_pthread_mutex_t
    cv*: x264_pthread_cond_t
    i_slice_count*: cint ##  Atomically written to/read from with slice threads
                       ##  periodic intra refresh
    f_pir_position*: cfloat
    i_pir_start_col*: cint
    i_pir_end_col*: cint
    i_frames_since_pir*: cint  ##  interactive encoder control
    b_corrupt*: cint           ##  user sei
    extra_sei*: x264_sei_t     ##  user data
    opaque*: pointer           ##  user frame properties
    mb_info*: uint8
    mb_info_free*: proc (a1: pointer)
    # opencl*: x264_frame_opencl_t

  x264_sync_frame_list_t* = ref object
    list*: x264_frame_t
    i_max_size*: cint
    i_size*: cint
    mutex*: x264_pthread_mutex_t
    cv_fill*: x264_pthread_cond_t ##  event signaling that the list became fuller
    cv_empty*: x264_pthread_cond_t ##  event signaling that the list became emptier

  x264_deblock_inter_t* = proc (pix: int16; stride: uint64; alpha: cint; beta: cint;tc0: int8)
  x264_deblock_intra_t* = proc (pix: int16; stride: uint64; alpha: cint; beta: cint)
  x264_deblock_function_t* = ref object
    deblock_luma*: array[2, x264_deblock_inter_t]
    deblock_chroma*: array[2, x264_deblock_inter_t]
    deblock_h_chroma_420*: x264_deblock_inter_t
    deblock_h_chroma_422*: x264_deblock_inter_t
    deblock_luma_intra*: array[2, x264_deblock_intra_t]
    deblock_chroma_intra*: array[2, x264_deblock_intra_t]
    deblock_h_chroma_420_intra*: x264_deblock_intra_t
    deblock_h_chroma_422_intra*: x264_deblock_intra_t
    deblock_luma_mbaff*: x264_deblock_inter_t
    deblock_chroma_mbaff*: x264_deblock_inter_t
    deblock_chroma_420_mbaff*: x264_deblock_inter_t
    deblock_chroma_422_mbaff*: x264_deblock_inter_t
    deblock_luma_intra_mbaff*: x264_deblock_intra_t
    deblock_chroma_intra_mbaff*: x264_deblock_intra_t
    deblock_chroma_420_intra_mbaff*: x264_deblock_intra_t
    deblock_chroma_422_intra_mbaff*: x264_deblock_intra_t
    deblock_strength*: proc (nnz: array[X264_SCAN8_SIZE, uint8];r: array[2, array[X264_SCAN8_LUMA_SIZE, int8]]; mv: array[2, array[X264_SCAN8_LUMA_SIZE, array[2, int16]]];bs: array[2, array[8, array[4, uint8]]]; mvy_limit: cint;bframe: cint)


  x264_lookahead_t* = ref object
    b_exit_thread*: uint8
    b_thread_active*: uint8
    b_analyse_keyframe*: uint8
    i_last_keyframe*: cint
    i_slicetype_length*: cint
    last_nonb*: x264_frame_t
    thread_handle*: x264_pthread_t
    ifbuf*: x264_sync_frame_list_t
    next*: x264_sync_frame_list_t
    ofbuf*: x264_sync_frame_list_t

  x264_left_table_t* = ref object
    intra*: array[4, uint8]
    nnz*: array[4, uint8]
    nnz_chroma*: array[4, uint8]
    mv*: array[4, uint8]
    r*: array[4, uint8]

  x264_frame_stat_t* = ref object
    i_mv_bits*: cint
    i_tex_bits*: cint
    i_misc_bits*: cint
    i_mb_count*: array[19, cint]
    i_mb_count_i*: cint
    i_mb_count_p*: cint
    i_mb_count_skip*: cint
    i_mb_count_8x8dct*: array[2, cint]
    i_mb_count_ref*: array[2, array[X264_REF_MAX * 2, cint]]
    i_mb_partition*: array[17, cint]
    i_mb_cbp*: array[6, cint]
    i_mb_pred_mode*: array[4, array[13, cint]]
    i_mb_field*: array[3, cint]
    i_direct_score*: array[2, cint]
    i_ssd*: array[3, int64]
    f_ssim*: cdouble
    i_ssim_cnt*: cint



  bs_t* = ref object
    p_start*: uint8
    p*: uint8
    p_end*: uint8
    cur_bits*: uint64
    i_left*: cint              ##  i_count number of available bits
    i_bits_encoded*: cint      ##  RD only




  X264CostTable* = ref object
    # r*: array[QP_MAX + 1, array[3, array[33, uint16]]]
    # i4x4_mode*: array[QP_MAX + 1, array[17, uint16]]

  INNER_C_STRUCT_playground_818* = ref object
    current*: x264_frame_t
    unused*: array[2, x264_frame_t]
    blank_unused*: x264_frame_t
    reference*: array[X264_REF_MAX + 2, x264_frame_t]
    i_last_keyframe*: cint     ##  Frame number of the last keyframe
    i_last_idr*: cint          ##  Frame number of the last IDR (not RP)
    i_poc_last_open_gop*: cint ##  Poc of the I frame of the last open-gop. The value
                             ##  is only assigned during the period between that
                             ##  I frame and the next P or I frame, else -1
    i_input*: cint             ##  Number of input frames already accepted
    i_max_dpb*: cint           ##  Number of frames allocated in the decoded picture buffer
    i_max_ref0*: cint
    i_max_ref1*: cint
    i_delay*: cint             ##  Number of frames buffered for B reordering
    i_bframe_delay*: cint
    i_bframe_delay_time*: int64
    i_first_pts*: int64
    i_prev_reordered_pts*: array[2, int64]
    i_largest_pts*: int64
    i_second_largest_pts*: int64
    b_have_lowres*: cint       ##  Whether 1/2 resolution luma planes are being used
    b_have_sub8x8_esa*: cint

  INNER_C_STRUCT_playground_864* = ref object
    luma16x16_dc*: array[3, array[16, int32]]
    chroma_dc*: array[2, array[8, int32]]
    luma8x8*: array[12, array[64, int32]]
    luma4x4*: array[16 * 3, array[16, int32]]

  INNER_C_STRUCT_playground_1004* = ref object
    fenc_buf*: array[48 * FENC_STRIDE, uint16] ##  space for p_fenc and p_fdec
    fdec_buf*: array[54 * FDEC_STRIDE, uint16] ##  i4x4 and i8x8 backup data, for skipping the encode stage when possible
    i4x4_fdec_buf*: array[16 * 16, uint16]
    i8x8_fdec_buf*: array[16 * 16, uint16]
    i8x8_dct_buf*: array[3, array[64, int32]]
    i4x4_dct_buf*: array[15, array[16, int32]]
    i4x4_nnz_buf*: array[4, uint32]
    i8x8_nnz_buf*: array[4, uint32] ##  Psy trellis DCT data
    fenc_dct8*: array[4, array[64, int32]]
    fenc_dct4*: array[16, array[16, int32]] ##  Psy RD SATD/SA8D scores cache
    fenc_satd_cache*: array[32, uint32]
    fenc_hadamard_cache*: array[9, uint64]
    i4x4_cbp*: cint
    i8x8_cbp*: cint            ##  pointer over mb of the frame to be compressed
    p_fenc*: array[3, int16] ##  y,u,v
                             ##  pointer to the actual source frame, not a block copy
    p_fenc_plane*: array[3, int16] ##  pointer over mb of the frame to be reconstructed
    p_fdec*: array[3, int16] ##  pointer over mb of the references
    i_fref*: array[2, cint]     ##  [12]: yN, yH, yV, yHV, (NV12 ? uv : I444 ? (uN, uH, uV, uHV, vN, ...))
    p_fref*: array[2, array[X264_REF_MAX * 2, array[12, int16]]]
    p_fref_w*: array[X264_REF_MAX * 2, int16] ##  weighted fullpel luma
    p_integral*: array[2, array[X264_REF_MAX, uint16]] ##  fref stride
    i_stride*: array[3, cint]

  INNER_C_STRUCT_playground_1049* = ref object
    intra4x4_pred_mode*: array[X264_SCAN8_LUMA_SIZE, int8] ##  real intra4x4_pred_mode if I_4X4 or I_8X8, I_PRED_4x4_DC if mb available, -1 if not
    ##  i_non_zero_count if available else 0x80. intentionally misaligned by 8 for asm
    non_zero_count*: array[X264_SCAN8_SIZE, uint8] ##  -1 if unused, -2 if unavailable
    r*: array[2, array[X264_SCAN8_LUMA_SIZE, int8]] ##  0 if not available
    mv*: array[2, array[X264_SCAN8_LUMA_SIZE, array[2, int16]]]
    mvd*: array[2, array[X264_SCAN8_LUMA_SIZE, array[2, uint8]]] ##  1 if SKIP or DIRECT. set only for B-frames + CABAC
    skip*: array[X264_SCAN8_LUMA_SIZE, int8]
    direct_mv*: array[2, array[4, array[2, int16]]]
    direct_ref*: array[2, array[4, int8]]
    direct_partition*: cint
    pskip_mv*: array[2, int16] ##  number of neighbors (top and left) that used 8x8 dct
    i_neighbour_transform_size*: cint
    i_neighbour_skip*: cint    ##  neighbor CBPs
    i_cbp_top*: cint
    i_cbp_left*: cint          ##  extra data required for mbaff in mv prediction
    topright_mv*: array[2, array[3, array[2, int16]]]
    topright_ref*: array[2, array[3, int8]] ##  current mb deblock strength
    deblock_strength*: array[8, array[4, uint8]]

  INNER_C_STRUCT_playground_873* = ref object
    i_mb_width*: cint
    i_mb_height*: cint
    i_mb_count*: cint          ##  number of mbs in a frame
                    ##  Chroma subsampling
    chroma_h_shift*: cint
    chroma_v_shift*: cint      ##  Strides
    i_mb_stride*: cint
    i_b8_stride*: cint
    i_b4_stride*: cint
    left_b8*: array[2, cint]
    left_b4*: array[2, cint]    ##  Current index
    i_mb_x*: cint
    i_mb_y*: cint
    i_mb_xy*: cint
    i_b8_xy*: cint
    i_b4_xy*: cint             ##  Search parameters
    i_me_method*: cint
    i_subpel_refine*: cint
    b_chroma_me*: cint
    b_trellis*: cint
    b_noise_reduction*: cint
    b_dct_decimate*: cint
    i_psy_rd*: cint            ##  Psy RD strength--fixed point value
    i_psy_trellis*: cint       ##  Psy trellis strength--fixed point value
    b_interlaced*: cint
    b_adaptive_mbaff*: cint ##  MBAFF+subme 0 requires non-adaptive MBAFF i.e. all field mbs
                          ##  Allowed qpel MV range to stay within the picture + emulated edge pixels
    mv_min*: array[2, cint]
    mv_max*: array[2, cint]
    mv_miny_row*: array[3, cint] ##  0 == top progressive, 1 == bot progressive, 2 == interlaced
    mv_maxy_row*: array[3, cint] ##  Subpel MV range for motion search.
                              ##  same mv_min/max but includes levels' i_mv_range.
    mv_min_spel*: array[2, cint]
    mv_max_spel*: array[2, cint]
    mv_miny_spel_row*: array[3, cint]
    mv_maxy_spel_row*: array[3, cint] ##  Fullpel MV range for motion search
    mv_limit_fpel*: array[2, array[2, int16]] ##  min_x, min_y, max_x, max_y
    mv_miny_fpel_row*: array[3, cint]
    mv_maxy_fpel_row*: array[3, cint] ##  neighboring MBs
    i_neighbour*: cuint
    i_neighbour8*: array[4, cuint] ##  neighbours of each 8x8 or 4x4 block that are available
    i_neighbour4*: array[16, cuint] ##  at the time the block is coded
    i_neighbour_intra*: cuint  ##  for constrained intra pred
    i_neighbour_frame*: cuint  ##  ignoring slice boundaries
    i_mb_type_top*: cint
    i_mb_type_left*: array[2, cint]
    i_mb_type_topleft*: cint
    i_mb_type_topright*: cint
    i_mb_prev_xy*: cint
    i_mb_left_xy*: array[2, cint]
    i_mb_top_xy*: cint
    i_mb_topleft_xy*: cint
    i_mb_topright_xy*: cint
    i_mb_top_y*: cint
    i_mb_topleft_y*: cint
    i_mb_topright_y*: cint
    left_index_table*: ptr x264_left_table_t
    i_mb_top_mbpair_xy*: cint
    topleft_partition*: cint
    b_allow_skip*: cint
    field_decoding_flag*: cint ## *** thread synchronization ends here ***
                             ##  subsequent variables are either thread-local or constant,
                             ##  and won't be copied from one thread to another
                             ##  mb table
    base*: uint8            ##  base pointer for all malloced data in this mb
    t*: int8         ##  mb type
    partition*: uint8       ##  mb partition
    qp*: int8             ##  mb qp
    cbp*: int16           ##  mb cbp: 0x0?: luma, 0x?0: chroma, 0x100: luma dc, 0x200 and 0x400: chroma dc, 0x1000 PCM (all set for PCM)
    intra4x4_pred_mode*: array[8, int8] ##  intra4x4 pred mode. for non I4x4 set to I_PRED_4x4_DC(2)
                                       ##  actually has only 7 entries; set to 8 for write-combining optimizations
    non_zero_count*: array[16 * 3, uint8] ##  nzc. for I_PCM set to 16
    chroma_pred_mode*: int8 ##  chroma_pred_mode. cabac only. for non intra I_PRED_CHROMA_DC(0)
                               ##  int16 (*mv[2])[2];                /* mb mv. set to 0 for intra mb */
                               ##  uint8 (*mvd[2])[8][2];            /* absolute value of mb mv difference with predict, clipped to [0,33]. set to 0 if intra. cabac only */
    r*: array[2, int8] ##  mb ref. set to -1 if non used (intra or Lx only)
                             ##  int16 (*mvr[2][X264_REF_MAX*2])[2];/* 16x16 mv for each possible ref */
    skipbp*: int8         ##  block pattern for SKIP or DIRECT (sub)mbs. B-frames + cabac only
    mb_transform_size*: int8 ##  transform_size_8x8_flag of each mb
    slice_table*: int32   ##  sh->first_mb of the slice that the indexed mb is part of
    field*: uint8           ##  buffer for weighted versions of the reference frames
    p_weight_buf*: array[X264_REF_MAX, int16] ##  current value
    i_type*: cint
    i_partition*: cint
    i_sub_partition*: array[4, uint8]
    b_transform_8x8*: cint
    i_cbp_luma*: cint
    i_cbp_chroma*: cint
    i_intra16x16_pred_mode*: cint
    i_chroma_pred_mode*: cint ##  skip flags for i4x4 and i8x8
                            ##  0 = encode as normal.
                            ##  1 (non-RD only) = the DCT is still in h->dct, restore fdec and skip reconstruction.
                            ##  2 (RD only) = the DCT has since been overwritten by RD; restore that too.
    i_skip_intra*: cint        ##  skip flag for motion compensation
                      ##  if we've already done MC, we don't need to do it again
    b_skip_mc*: cint           ##  set to true if we are re-encoding a macroblock.
    b_reencode_mb*: cint
    ip_offset*: cint           ##  Used by PIR to offset the quantizer of intra-refresh blocks.
    b_deblock_rdo*: cint
    b_overflow*: cint          ##  If CAVLC had a level code overflow during bitstream writing.
    pic*: INNER_C_STRUCT_playground_1004 ##  cache
    cache*: INNER_C_STRUCT_playground_1049
    i_qp*: cint                ##  current qp
    i_chroma_qp*: cint
    i_last_qp*: cint           ##  last qp
    i_last_dqp*: cint          ##  last delta qp
    b_variable_qp*: cint       ##  whether qp is allowed to vary per macroblock
    b_lossless*: cint
    b_direct_auto_read*: cint  ##  take stats for --direct auto from the 2pass log
    b_direct_auto_write*: cint ##  analyse direct modes, to use and/or save
                             ##  lambda values
    i_trellis_lambda2*: array[2, array[2, cint]] ##  [luma,chroma][inter,intra]
    i_psy_rd_lambda*: cint
    i_chroma_lambda2_offset*: cint ##  B_direct and weighted prediction
    dist_scale_factor_buf*: array[2, array[2,
        array[X264_REF_MAX * 2, array[4, int16]]]]
    dist_scale_factor*: array[4, int16]
    bipred_weight_buf*: array[2, array[2, array[X264_REF_MAX * 2, array[4, int8]]]]
    bipred_weight*: array[4, int8] ##  maps fref1[0]'s ref indices into the current list0

    map_col_to_list0*: array[X264_REF_MAX + 2, int8]
    ref_blind_dupe*: cint      ##  The index of the blind reference frame duplicate.
    deblock_ref_table*: array[X264_REF_MAX * 2 + 2, int8]



  INNER_C_STRUCT_playground_1122* = ref object
    i_frame_count*: array[3, cint] ##  Cumulated stats per slice info
    i_frame_size*: array[3, int64]
    f_frame_qp*: array[3, cdouble]
    i_consecutive_bframes*: array[X264_BFRAME_MAX + 1, cint]
    f_ssd_global*: array[3, cdouble]
    f_psnr_average*: array[3, cdouble]
    f_psnr_mean_y*: array[3, cdouble]
    f_psnr_mean_u*: array[3, cdouble]
    f_psnr_mean_v*: array[3, cdouble]
    f_ssim_mean_y*: array[3, cdouble]
    f_frame_duration*: array[3, cdouble]
    i_mb_count*: array[3, array[19, int64]]
    i_mb_partition*: array[2, array[17, int64]]
    i_mb_count_8x8dct*: array[2, int64]
    i_mb_count_ref*: array[2, array[2, array[X264_REF_MAX * 2, int64]]]
    i_mb_cbp*: array[6, int64]
    i_mb_pred_mode*: array[4, array[13, int64]]
    i_mb_field*: array[3, int64]
    i_direct_score*: array[2, cint]
    i_direct_frames*: array[2, cint] ##  num p-frames weighted
    i_wpred*: array[2, cint]    ##  Current frame stats
    frame*: x264_frame_stat_t

  x264_param_t* = ref object
    cpu*: uint32               ##  CPU flags
    i_threads*: cint           ##  encode multiple frames in parallel
    i_lookahead_threads*: cint ##  multiple threads for lookahead analysis
    b_sliced_threads*: cint    ##  Whether to use slice-based threading.
    b_deterministic*: cint     ##  whether to allow non-deterministic optimizations when threaded
    b_cpu_independent*: cint   ##  force canonical behavior rather than cpu-dependent optimal algorithms
    i_sync_lookahead*: cint    ##  threaded lookahead buffer
    i_width*: cint
    i_height*: cint
    i_csp*: cint               ##  CSP of encoded bitstream
    i_bitdepth*: cint
    i_level_idc*: cint
    i_frame_total*: cint       ##  number of frames to encode if known, else 0
    i_nal_hrd*: cint
    vui*: X264ParamVui
    i_frame_reference*: cint   ##  Maximum number of reference frames
    i_dpb_size*: cint ##  Force a DPB size larger than that implied by B-frames and reference frames.
                    ##  Useful in combination with interactive error resilience.
    i_keyint_max*: cint        ##  Force an IDR keyframe at this interval
    i_keyint_min*: cint        ##  Scenecuts closer together than this are coded as I, not IDR.
    i_scenecut_threshold*: cint ##  how aggressively to insert extra I frames
    b_intra_refresh*: cint     ##  Whether or not to use periodic intra refresh instead of IDR frames.
    i_bframe*: cint            ##  how many b-frame between 2 references pictures
    i_bframe_adaptive*: cint
    i_bframe_bias*: cint
    i_bframe_pyramid*: cint    ##  Keep some B-frames as references: 0=off, 1=strict hierarchical, 2=normal
    b_open_gop*: cint
    b_bluray_compat*: cint
    i_avcintra_class*: cint
    i_avcintra_flavor*: cint
    b_deblocking_filter*: cint
    i_deblocking_filter_alphac0*: cint ##  [-6, 6] -6 light filter, 6 strong
    i_deblocking_filter_beta*: cint ##  [-6, 6]  idem
    b_cabac*: cint
    i_cabac_init_idc*: cint
    b_interlaced*: cint
    b_constrained_intra*: cint
    i_cqm_preset*: cint
    psz_cqm_file*: cstring     ##  filename (in UTF-8) of CQM file, JM format
    cqm_4iy*: array[16, uint8]  ##  used only if i_cqm_preset == X264_CQM_CUSTOM
    cqm_4py*: array[16, uint8]
    cqm_4ic*: array[16, uint8]
    cqm_4pc*: array[16, uint8]
    cqm_8iy*: array[64, uint8]
    cqm_8py*: array[64, uint8]
    cqm_8ic*: array[64, uint8]
    cqm_8pc*: array[64, uint8]
    pf_log*: proc (a1: pointer; i_level: cint; psz: cstring; a4: va_list)
    p_log_private*: pointer
    i_log_level*: cint
    b_full_recon*: cint        ##  fully reconstruct frames, even when not necessary for encoding.  Implied by psz_dump_yuv
    psz_dump_yuv*: cstring     ##  filename (in UTF-8) for reconstructed frames
                         ##  Encoder analyser parameters
    analyse*: X264ParamAnalyse ##  Rate control parameters
    rc*: X264ParamRC
    crop_rect*: X264ParamCropRect
    i_frame_packing*: cint
    i_alternative_transfer*: cint ##  Muxing parameters
    b_aud*: cint               ##  generate access unit delimiters
    b_repeat_headers*: cint    ##  put SPS/PPS before each keyframe
    b_annexb*: cint ##  if set, place start codes (4 bytes) before NAL units,
                  ##  otherwise place size (4 bytes) before NAL units.
    i_sps_id*: cint            ##  SPS and PPS id number
    b_vfr_input*: cint ##  VFR input.  If 1, use timebase and timestamps for ratecontrol purposes.
                     ##  If 0, use fps only.
    b_pulldown*: cint          ##  use explicity set timebase for CFR
    i_fps_num*: uint32
    i_fps_den*: uint32
    i_timebase_num*: uint32    ##  Timebase numerator
    i_timebase_den*: uint32    ##  Timebase denominator
    b_tff*: cint
    b_pic_struct*: cint
    b_fake_interlaced*: cint
    b_stitchable*: cint
    b_opencl*: cint            ##  use OpenCL when available
    i_opencl_device*: cint     ##  specify count of GPU devices to skip, for CLI users
    opencl_device_id*: pointer ##  pass explicit cl_device_id as void*, for API users
    psz_clbin_file*: cstring ##  filename (in UTF-8) of the compiled OpenCL kernel cache file
                           ##  Slicing parameters
    i_slice_max_size*: cint    ##  Max size per slice in bytes; includes estimated NAL overhead.
    i_slice_max_mbs*: cint     ##  Max number of MBs per slice; overrides i_slice_count.
    i_slice_min_mbs*: cint     ##  Min number of MBs per slice
    i_slice_count*: cint       ##  Number of slices per frame: forces rectangular slices.
    i_slice_count_max*: cint ##  Absolute cap on slices per frame; stops applying slice-max-size
                           ##  and slice-max-mbs if this is reached.
    param_free*: proc (a1: pointer)
    nalu_process*: proc (h: x264_t; nal: x264_nal_t; opaque: pointer)
    opaque*: pointer

  x264_threadpool_t* = ref object
    exit*: cint
    threads*: cint
    thread_handle*: x264_pthread_t
    init_func*: proc (a1: pointer)
    init_arg*: pointer ##  requires a synchronized list structure and associated methods,
                     ##        so use what is already implemented for frames
    uninit*: x264_sync_frame_list_t ##  list of jobs that are awaiting use
    run*: x264_sync_frame_list_t ##  list of jobs that are queued for processing by the pool
    done*: x264_sync_frame_list_t ##  list of jobs that have finished processing

  x264_cabac_t* = ref object
    i_low*: cint               ##  state
    i_range*: cint             ##  bit stream
    i_queue*: cint             ## stored with an offset of -8 for faster asm
    i_bytes_outstanding*: cint
    p_start*: uint8
    p*: uint8
    p_end*: uint8         ##  aligned for memcpy_aligned starting here
    f8_bits_encoded*: cint     ##  only if using x264_cabac_size_decision()
    state*: array[1024, uint8]
    padding*: array[12, uint8]
  ratecontrol_entry_t* = ref object
    pict_type*: cint
    frame_type*: cint
    kept_as_ref*: cint
    qscale*: cdouble
    mv_bits*: cint
    tex_bits*: cint
    misc_bits*: cint
    expected_bits*: cdouble    ##  total expected bits up to the current frame (current one excluded)
    expected_vbv*: cdouble
    new_qscale*: cdouble
    new_qp*: cfloat
    i_count*: cint
    p_count*: cint
    s_count*: cint
    blurred_complexity*: cfloat
    direct_mode*: char
    weight*: array[3, array[2, int16]]
    i_weight_denom*: array[2, int16]
    refcount*: array[16, cint]
    refs*: cint
    i_duration*: int64
    i_cpb_duration*: int64
    out_num*: cint

  predictor_t* = ref object
    coeff_min*: cfloat
    coeff*: cfloat
    count*: cfloat
    decay*: cfloat
    offset*: cfloat

  INNER_C_STRUCT_playground_107* = ref object
    qp_buffer*: array[2, ptr uint16] ##  Global buffers for converting MB-tree quantizer data.
    qpbuf_pos*: cint ##  In order to handle pyramid reordering, QP buffer acts as a stack.
                   ##  This value is the current position (0 or 1).
    src_mb_count*: cint        ##  For rescaling
    rescale_enabled*: cint
    scale_buffer*: array[2, ptr cfloat] ##  Intermediate buffers
    filtersize*: array[2, cint] ##  filter size (H/V)
    coeffs*: array[2, ptr cfloat]
    pos*: array[2, ptr cint]
    srcdim*: array[2, cint]     ##  Source dimensions (W/H)

  x264_ratecontrol_t* = ref object
    b_abr*: cint               ##  constants
    b_2pass*: cint
    b_vbv*: cint
    b_vbv_min_rate*: cint
    fps*: cdouble
    bitrate*: cdouble
    rate_tolerance*: cdouble
    qcompress*: cdouble
    nmb*: cint                 ##  number of macroblocks in a frame
    qp_constant*: array[3, cint] ##  current frame
    rce*: ptr ratecontrol_entry_t
    qpm*: cfloat               ##  qp for current macroblock: precise float for AQ
    qpa_rc*: cfloat            ##  average of macroblocks' qp before aq
    qpa_rc_prev*: cfloat
    qpa_aq*: cint              ##  average of macroblocks' qp after aq
    qpa_aq_prev*: cint
    qp_novbv*: cfloat ##  QP for the current frame if 1-pass VBV was disabled.
                    ##  VBV stuff
    buffer_size*: cdouble
    buffer_fill_final*: int64
    buffer_fill_final_min*: int64
    buffer_fill*: cdouble      ##  planned buffer, if all in-progress frames hit their bit budget
    buffer_rate*: cdouble      ##  # of bits added to buffer_fill after each frame
    vbv_max_rate*: cdouble     ##  # of bits added to buffer_fill per second
    pred*: ptr predictor_t      ##  predict frame size from satd
    single_frame_vbv*: cint
    rate_factor_max_increment*: cfloat ##  Don't allow RF above (CRF + this value).
                                     ##  ABR stuff
    last_satd*: cint
    last_rceq*: cdouble
    cplxr_sum*: cdouble        ##  sum of bits*qscale/rceq
    expected_bits_sum*: cdouble ##  sum of qscale2bits after rceq, ratefactor, and overflow, only includes finished frames
    filler_bits_sum*: int64  ##  sum in bits of finished frames' filler data
    wanted_bits_window*: cdouble ##  target bitrate * window
    cbr_decay*: cdouble
    short_term_cplxsum*: cdouble
    short_term_cplxcount*: cdouble
    rate_factor_constant*: cdouble
    ip_offset*: cdouble
    pb_offset*: cdouble        ##  2pass stuff
    p_stat_file_out*: ptr FILE
    psz_stat_file_tmpname*: cstring
    p_mbtree_stat_file_out*: ptr FILE
    psz_mbtree_stat_file_tmpname*: cstring
    psz_mbtree_stat_file_name*: cstring
    p_mbtree_stat_file_in*: ptr FILE
    num_entries*: cint         ##  number of ratecontrol_entry_ts
    entry*: ptr ratecontrol_entry_t ##  FIXME: copy needed data and free this once init is done
    entry_out*: ptr ptr ratecontrol_entry_t
    last_qscale*: cdouble
    last_qscale_for*: array[3, cdouble] ##  last qscale for a specific pict type, used for max_diff & ipb factor stuff
    last_non_b_pict_type*: cint
    accum_p_qp*: cdouble       ##  for determining I-frame quant
    accum_p_norm*: cdouble
    last_accum_p_norm*: cdouble
    lmin*: array[3, cdouble]    ##  min qscale by frame type
    lmax*: array[3, cdouble]
    lstep*: cdouble            ##  max change (multiply) in qscale per frame
    mbtree*: INNER_C_STRUCT_playground_107 ##  MBRC stuff
    frame_size_estimated*: cfloat ##  Access to this variable must be atomic: double is
                                ##  not atomic on all arches we care about
    frame_size_maximum*: cdouble ##  Maximum frame size due to MinCR
    frame_size_planned*: cdouble
    slice_size_planned*: cdouble
    row_pred*: ptr predictor_t
    row_preds*: array[3, array[2, predictor_t]]
    pred_b_from_p*: ptr predictor_t ##  predict B-frame size from P-frame satd
    bframes*: cint             ##  # consecutive B-frames before this P-frame
    bframe_bits*: cint         ##  total cost of those frames
    i_zones*: cint
    zones*: ptr x264_zone_t
    prev_zone*: ptr x264_zone_t ##  hrd stuff
    initial_cpb_removal_delay*: cint
    initial_cpb_removal_delay_offset*: cint
    nrt_first_access_unit*: cdouble ##  nominal removal time
    previous_cpb_final_arrival_time*: cdouble
    hrd_multiply_denom*: uint64

  x264_nal_t* = ref object
    i_ref_idc*: cint           ##  nal_priority_e
    i_type*: cint              ##  nal_unit_type_e
    b_long_startcode*: cint
    i_first_mb*: cint          ##  If this NAL is a slice, the index of the first MB in the slice.
    i_last_mb*: cint ##  If this NAL is a slice, the index of the last MB in the slice.
                   ##  Size of payload (including any padding) in bytes.
    i_payload*: cint ##  If param->b_annexb is set, Annex-B bytestream with startcode. Otherwise, startcode is replaced with a 4-byte size.
                   ##  This size is the size used in mp4/similar muxing; it is equal to i_payload-4
    p_payload*: uint8     ##  Size of padding in bytes.
    i_padding*: cint
    
  X264Out* = ref object
    i_nal*: cint
    i_nals_allocated*: cint
    nal*: seq[x264_nal_t]
    i_bitstream*: cint         ##  size of p_bitstream
    p_bitstream*: uint8     ##  will hold data for all nal
    bs*: bs_t

  x264_t* = ref object
    param*: x264_param_t
    thread*: array[X264_THREAD_MAX + 1, x264_t]
    lookahead_thread*: array[X264_LOOKAHEAD_THREAD_MAX, x264_t]
    b_thread_active*: cint
    i_thread_phase*: cint      
    i_thread_idx*: cint        
    i_threadslice_start*: cint 
    i_threadslice_end*: cint   
    i_threadslice_pass*: cint  ##  which pass of encoding we are on
    threadpool*: x264_threadpool_t
    lookaheadpool*: x264_threadpool_t
    mutex*: x264_pthread_mutex_t
    cv*: x264_pthread_cond_t
    o*: X264Out
    nal_buffer*: ptr uint8
    nal_buffer_size*: cint
    reconfig_h*: x264_t
    reconfig*: cint
    i_frame*: cint
    i_frame_num*: cint
    i_thread_frames*: cint     ##  Number of different frames being encoded by threads;* 1 when sliced-threads is on.
    i_nal_type*: cint
    i_nal_ref_idc*: cint
    i_disp_fields*: int64      ##  Number of displayed fields (both coded and implied via pic_struct)
    i_disp_fields_last_frame*: cint
    i_prev_duration*: int64    ##  Duration of previous frame
    i_coded_fields*: int64     ##  Number of coded fields (both coded and implied via pic_struct)
    i_cpb_delay*: int64        ##  Equal to number of fields preceding this field
                      ##  since last buffering_period SEI
    i_coded_fields_lookahead*: int64 ##  Use separate counters for lookahead
    i_cpb_delay_lookahead*: int64
    i_cpb_delay_pir_offset*: int64
    i_cpb_delay_pir_offset_next*: int64
    b_queued_intra_refresh*: cint
    i_last_idr_pts*: int64
    i_idr_pic_id*: cint ##  int             (*dequant4_mf[4])[16];   /* [4][6][16] */
                      ##  int             (*dequant8_mf[4])[64];   /* [4][6][64] */
                      ##  int             (*unquant4_mf[4])[16];   /* [4][QP_MAX_SPEC+1][16] */
                      ##  int             (*unquant8_mf[4])[64];   /* [4][QP_MAX_SPEC+1][64] */
                      ##  uint32        (*quant4_mf[4])[16];     /* [4][QP_MAX_SPEC+1][16] */
                      ##  uint32        (*quant8_mf[4])[64];     /* [4][QP_MAX_SPEC+1][64] */
                      ##  uint32        (*quant4_bias[4])[16];   /* [4][QP_MAX_SPEC+1][16] */
                      ##  uint32        (*quant8_bias[4])[64];   /* [4][QP_MAX_SPEC+1][64] */
                      ##  uint32        (*quant4_bias0[4])[16];  /* [4][QP_MAX_SPEC+1][16] */
                      ##  uint32        (*quant8_bias0[4])[64];  /* [4][QP_MAX_SPEC+1][64] */
                      ##  uint32        (*nr_offset_emergency)[4][64];
    # cost_mv*: array[QP_MAX + 1, uint16]
    # cost_mv_fpel*: array[QP_MAX + 1, array[4, uint16]]
    cost_table*: ptr X264CostTable
    chroma_qp_table*: uint8
    sh*: x264_slice_header_t
    sps*: array[1, x264_sps_t]
    pps*: array[1, x264_pps_t]
    b_sh_backup*: cint
    sh_backup*: x264_slice_header_t
    cabac*: x264_cabac_t
    frames*: INNER_C_STRUCT_playground_818 ##  current frame being encoded
    fenc*: x264_frame_t     ##  frame being reconstructed
    fdec*: x264_frame_t     ##  references lists
    i_ref*: array[2, cint]
    fref*: array[2, array[X264_REF_MAX + 3, x264_frame_t]]
    fref_nearest*: array[2, x264_frame_t]
    b_ref_reorder*: array[2, cint] ##  hrd
    initial_cpb_removal_delay*: cint
    initial_cpb_removal_delay_offset*: cint
    i_reordered_pts_delay*: int64 ##  Current MB DCT coeffs
    dct*: INNER_C_STRUCT_playground_864 ##  MB table and cache for current frame/mb
    mb*: INNER_C_STRUCT_playground_873 ##  rate control encoding only
    rc*: x264_ratecontrol_t ##  stats
    stat*: INNER_C_STRUCT_playground_1122 ##  0 = luma 4x4, 1 = luma 8x8, 2 = chroma 4x4, 3 = chroma 8x8
    nr_offset*: array[64, uint32]
    nr_residual_sum*: array[64, uint32]
    nr_count*: ptr uint32
    nr_offset_denoise*: array[4, array[64, uint32]]
    nr_residual_sum_buf*: array[2, array[4, array[64, uint32]]]
    nr_count_buf*: array[2, array[4, uint32]]
    luma2chroma_pixel*: array[7, uint8] ##  Subsampled uint16 size Buffers that are allocated per-thread even in sliced threads.
    scratch_buffer*: pointer   ##  for any temporary storage that doesn't want repeated malloc
    scratch_buffer2*: pointer  ##  if the first one's already in use
    intra_border_backup*: array[5, array[3, int16]] ##  bottom pixels of the previous mb row, used for intra prediction after the framebuffer has been deblocked
                                                   ##  uint8 (*deblock_strength[2])[2][8][4];
                                                   ##  CPU functions dependents
    predict_16x16*: array[4 + 3, x264_predict_t]
    predict_8x8*: array[9 + 3, x264_predict8x8_t]
    predict_4x4*: array[9 + 3, x264_predict_t]
    predict_chroma*: array[4 + 3, x264_predict_t]
    predict_8x8c*: array[4 + 3, x264_predict_t]
    predict_8x16c*: array[4 + 3, x264_predict_t]
    predict_8x8_filter*: x264_predict_8x8_filter_t
    # pixf*: x264_pixel_function_t
    # mc*: x264_mc_functions_t
    # dctf*: x264_dct_function_t
    # zigzagf*: x264_zigzag_function_t
    # zigzagf_interlaced*: x264_zigzag_function_t
    # zigzagf_progressive*: x264_zigzag_function_t
    # quantf*: x264_quant_function_t
    # loopf*: x264_deblock_function_t
    # bsf*: x264_bitstream_function_t
    # lookahead*: ptr x264_lookahead_t
    # opencl*: x264_opencl_t

  x264_api_t* = ref object
    x264*: x264_t           ##  Internal reference to x264_t data
    nal_encode*: proc (h: x264_t; dst: uint8; nal: x264_nal_t)
    encoder_reconfig*: proc (a1: x264_t; a2: ptr x264_param_t): cint
    encoder_parameters*: proc (a1: x264_t; a2: ptr x264_param_t)
    encoder_headers*: proc (a1: x264_t; pp_nal: x264_nal_t; pi_nal: cint): cint
    encoder_encode*: proc (a1: x264_t; pp_nal: x264_nal_t; pi_nal: ptr cint; pic_in: ptr x264_picture_t; pic_out: ptr x264_picture_t): cint
    encoder_close*: proc (a1: x264_t)
    encoder_delayed_frames*: proc (a1: x264_t): cint
    encoder_maximum_delayed_frames*: proc (a1: x264_t): cint
    encoder_intra_refresh*: proc (a1: x264_t)
    encoder_invalidate_reference*: proc (a1: x264_t; pts: int64): cint

# proc check_encapsulated_buffer*(h: ptr x264_t; h0: ptr x264_t; start: cint; previous_nal_size: int64; necessary_size: int64): cint =
#   if h0.nal_buffer_size < necessary_size:
#     necessary_size = necessary_size * 2
#     if necessary_size > INT_MAX:
#       return -1
#     var buf: ptr uint8 = x264_malloc(necessary_size)
#     if not buf:
#       return -1
#     if previous_nal_size:
#       memcpy(buf, h0.nal_buffer, previous_nal_size)
#     var delta: intptr_t = buf - h0.nal_buffer
#     for i in 0..start-1:
#       inc(h.o.nal[i].p_payload, delta)
#     h0.nal_buffer = buf
#     h0.nal_buffer_size = necessary_size
#   return 0


# proc encoder_encapsulate_nals*(h: x264_t; start: cint): cint =
#   var h0: x264_t = h.thread[0]
#   var
#     nal_size: cint = 0
#     previous_nal_size: int64 = 0
#   for i in 0..h.o.i_nal.high:
#     nal_size += h.o.nal[i].i_payload
#   return nal_size
#   for i in 0..start-1:
#     previous_nal_size += h.o.nal[i].i_payload
#   for i in 0..h.o.i_nal.high:
#     nal_size += h.o.nal[i].i_payload
#   var necessary_size: int64 = previous_nal_size + nal_size * 3 div 2 + h.o.i_nal * 4 + 4 + 64
#   for i in 0..h.o.i_nal.high:
#     necessary_size += h.o.nal[i].i_padding
#   if check_encapsulated_buffer(h, h0, start, previous_nal_size, necessary_size) == -1:
#     return -1
#   var nal_buffer: uint8 = h0.nal_buffer + previous_nal_size
#   for i in 0..h.o.i_nal.high:
#     h.o.nal[i].b_long_startcode = not i or h.o.nal[i].i_type == NAL_SPS or h.o.nal[i].i_type == NAL_PPS or h.param.i_avcintra_class 
#     x264_nal_encode(h, nal_buffer, addr(h.o.nal[i]))
#     nal_buffer += h.o.nal[i].i_payload
#   x264_emms()
#   return nal_buffer - (h0.nal_buffer + previous_nal_size)

# proc bs_init*(s: bs_t; p_data: pointer; i_data: cint) {.inline.} =
#   var offset: cint = (cast[uint64](p_data) and 3)
#   s.p = s.p_start = cast[uint8](p_data) - offset
#   s.p_end = cast[uint8](p_data) + i_data
#   s.i_left = (WORD_SIZE - offset) * 8
#   if offset:
#     s.cur_bits = endian_fix32(M32(s.p))
#     s.cur_bits = s.cur_bits shr ((4 - offset) * 8)
#   else:
#     s.cur_bits = 0

# proc bs_pos*(s: ptr bs_t): cint {.inline.} =
#   return 8 * (s.p - s.p_start) + (WORD_SIZE * 8) - s.i_left

# proc bs_flush*(s: ptr bs_t) {.inline.} =
#   M32(s.p) = endian_fix32(s.cur_bits shl (s.i_left and 31))
#   inc(s.p, WORD_SIZE - (s.i_left shr 3))
#   s.i_left = WORD_SIZE * 8


# proc bs_realign*(s: ptr bs_t) {.inline.} =
#   var offset: cint = (cast[uint64](s.p) and 3)
#   if offset:
#     s.p = cast[uint8](s.p) - offset
#     s.i_left = (WORD_SIZE - offset) * 8
#     s.cur_bits = endian_fix32(M32(s.p))
#     s.cur_bits = s.cur_bits shr ((4 - offset) * 8)

# proc bs_write*(s: ptr bs_t; i_count: cint; i_bits: uint32_t) {.inline.} =
#   if WORD_SIZE == 8:
#     s.cur_bits = (s.cur_bits shl i_count) or i_bits
#     dec(s.i_left, i_count)
#     if s.i_left <= 32:
#       when WORDS_BIGENDIAN:
#         M32(s.p) = s.cur_bits shr (32 - s.i_left)
#       else:
#         M32(s.p) = endian_fix(s.cur_bits shl s.i_left)
#       inc(s.i_left, 32)
#       inc(s.p, 4)
#   else:
#     if i_count < s.i_left:
#       s.cur_bits = (s.cur_bits shl i_count) or i_bits
#       dec(s.i_left, i_count)
#     else:
#       dec(i_count, s.i_left)
#       s.cur_bits = (s.cur_bits shl s.i_left) or (i_bits shr i_count)
#       M32(s.p) = endian_fix(s.cur_bits)
#       inc(s.p, 4)
#       s.cur_bits = i_bits
#       s.i_left = 32 - i_count


# proc bs_write32*(s: ptr bs_t; i_bits: uint32_t) {.inline.} =
#   bs_write(s, 16, i_bits shr 16)
#   bs_write(s, 16, i_bits)

# proc bs_write1*(s: ptr bs_t; i_bit: uint32_t) {.inline.} =
#   s.cur_bits = s.cur_bits shl 1
#   s.cur_bits = s.cur_bits or i_bit
#   dec(s.i_left)
#   if s.i_left == WORD_SIZE * 8 - 32:
#     M32(s.p) = endian_fix32(s.cur_bits)
#     inc(s.p, 4)
#     s.i_left = WORD_SIZE * 8

# proc bs_align_0*(s: ptr bs_t) {.inline.} =
#   bs_write(s, s.i_left and 7, 0)
#   bs_flush(s)

# proc bs_align_1*(s: ptr bs_t) {.inline.} =
#   bs_write(s, s.i_left and 7, (1 shl (s.i_left and 7)) - 1)
#   bs_flush(s)

# proc bs_align_10*(s: ptr bs_t) {.inline.} =
#   if s.i_left and 7:
#     bs_write(s, s.i_left and 7, 1 shl ((s.i_left and 7) - 1))
#   bs_flush(s)

# proc nal_end*(h: ptr x264_t): cint =
#   var nal: x264_nal_t = addr(h.o.nal[h.o.i_nal])
#   var e: uint8 = addr(h.o.p_bitstream[bs_pos(addr(h.o.bs)) div 8])
#   nal.i_payload = e - nal.p_payload
#   h.param.nalu_process(h, nal, h.fenc.opaque)
#   inc(h.o.i_nal)
#   return nal_check_buffer(h)

# proc x264_encoder_headers*(h: ptr x264_t; pp_nal: x264_nal_t; pi_nal: ptr cint): cint =
#   var frame_size: cint = 0
#   ##  init bitstream context
#   h.o.i_nal = 0
#   bs_init(addr(h.o.bs), h.o.p_bitstream, h.o.i_bitstream)
#   nal_start(h, NAL_SPS, NAL_PRIORITY_HIGHEST)
#   x264_sps_write(addr(h.o.bs), h.sps)
#   if nal_end(h):
#     return -1
#   nal_start(h, NAL_PPS, NAL_PRIORITY_HIGHEST)
#   x264_pps_write(addr(h.o.bs), h.sps, h.pps)
#   if nal_end(h):
#     return -1
#   nal_start(h, NAL_SEI, NAL_PRIORITY_DISPOSABLE)
#   if x264_sei_version_write(h, addr(h.o.bs)):
#     return -1
#   if nal_end(h):
#     return -1
#   frame_size = encoder_encapsulate_nals(h, 0)
#   if frame_size < 0:
#     return -1
#   pi_nal[] = h.o.i_nal
#   pp_nal[] = addr(h.o.nal[0])
#   h.o.i_nal = 0
#   return frame_size

# proc x264_encoder_open*(param: x264_param_t): x264_t =
#   var api: x264_api_t
#   if param.i_bitdepth == 8:
#     api.nal_encode = x264_8_nal_encode
#     api.encoder_reconfig = x264_8_encoder_reconfig
#     api.encoder_parameters = x264_8_encoder_parameters
#     api.encoder_headers = x264_8_encoder_headers
#     api.encoder_encode = x264_8_encoder_encode
#     api.encoder_close = x264_8_encoder_close
#     api.encoder_delayed_frames = x264_8_encoder_delayed_frames
#     api.encoder_maximum_delayed_frames = x264_8_encoder_maximum_delayed_frames
#     api.encoder_intra_refresh = x264_8_encoder_intra_refresh
#     api.encoder_invalidate_reference = x264_8_encoder_invalidate_reference
#     api.x264 = x264_8_encoder_open(param)
#   elif param.i_bitdepth == 10:
#     api.nal_encode = x264_10_nal_encode
#     api.encoder_reconfig = x264_10_encoder_reconfig
#     api.encoder_parameters = x264_10_encoder_parameters
#     api.encoder_headers = x264_10_encoder_headers
#     api.encoder_encode = x264_10_encoder_encode
#     api.encoder_close = x264_10_encoder_close
#     api.encoder_delayed_frames = x264_10_encoder_delayed_frames
#     api.encoder_maximum_delayed_frames = x264_10_encoder_maximum_delayed_frames
#     api.encoder_intra_refresh = x264_10_encoder_intra_refresh
#     api.encoder_invalidate_reference = x264_10_encoder_invalidate_reference
#     api.x264 = x264_10_encoder_open(param)
#   return api.x264

# proc x264_encoder_close*(h: x264_t) =
#   var api: x264_api_t = cast[x264_api_t](h)
#   api.encoder_close(api.x264)

# proc x264_nal_encode*(h: x264_t; dst: uint8; nal: x264_nal_t) =
#   var api: x264_api_t = cast[x264_api_t](h)
#   api.nal_encode(api.x264, dst, nal)

# proc x264_encoder_reconfig*(h: x264_t; param: ptr x264_param_t): cint =
#   var api: x264_api_t = cast[x264_api_t](h)
#   return api.encoder_reconfig(api.x264, param)

# proc x264_encoder_parameters*(h: x264_t; param: ptr x264_param_t) =
#   var api: x264_api_t = cast[x264_api_t](h)
#   api.encoder_parameters(api.x264, param)

# proc x264_encoder_headers*(h: x264_t; pp_nal: x264_nal_t; pi_nal: cint): cint =
#   var api: x264_api_t = cast[x264_api_t](h)
#   return api.encoder_headers(api.x264, pp_nal, pi_nal)

# proc x264_encoder_encode*(h: x264_t; pp_nal: x264_nal_t; pi_nal: ptr cint;
#                          pic_in: ptr x264_picture_t; pic_out: ptr x264_picture_t): cint =
#   var api: x264_api_t = cast[x264_api_t](h)
#   return api.encoder_encode(api.x264, pp_nal, pi_nal, pic_in, pic_out)

# proc x264_encoder_delayed_frames*(h: x264_t): cint =
#   var api: x264_api_t = cast[x264_api_t](h)
#   return api.encoder_delayed_frames(api.x264)

# proc x264_encoder_maximum_delayed_frames*(h: x264_t): cint =
#   var api: x264_api_t = cast[x264_api_t](h)
#   return api.encoder_maximum_delayed_frames(api.x264)

# proc x264_encoder_intra_refresh*(h: x264_t) =
#   var api: x264_api_t = cast[x264_api_t](h)
#   api.encoder_intra_refresh(api.x264)

# proc x264_encoder_invalidate_reference*(h: x264_t; pts: int64): cint =
#   var api: x264_api_t = cast[x264_api_t](h)
#   return api.encoder_invalidate_reference(api.x264, pts)


# proc x264_clip3*(v: cint; i_min: cint; i_max: cint): cint {.inline.} =
#   if (v < i_min): i_min 
#   else: 
#     if (v > i_max): i_max 
#     else: v

# proc x264_clip3f*(v: cdouble; f_min: cdouble; f_max: cdouble): cdouble {.inline.} =
#   if (v < f_min): f_min 
#   else: 
#     if (v > f_max): f_max 
#     else: v

# ##  Not a general-purpose function; multiplies input by -1/6 to convert
# ##  qp to qscale.

# proc x264_exp2fix8*(x: cfloat): cint {.inline.} =
#   var i: cint = cint(x * (- 64.0 / 6.0) + 512.5)
#   if i < 0:
#     return 0
#   if i > 1023:
#     return 0x0000FFFF
#   return cint(x264_exp2_lut[i and 63].int + 256) shl (i shr 6) shr 8

# proc x264_clz*(x: var uint32): cint {.inline.} =
#   var lut: array[16, uint8] = [4'u8, 3, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0]
#   var
#     y: cint
#     z: cint = cint(((x shr 16) - 1) shr 27) and 16
#   x = x shr (z xor 16)
#   inc(z, y = ((x.int - 0x00000100) shr 28) and 8)
#   x = x shr (y xor 8)
#   inc(z, y = ((x.int - 0x00000010) shr 29) and 4)
#   x = x shr (y xor 4)
#   return z + lut[x].cint


# proc x264_log2*(x: var uint32): cfloat {.inline.} =
#   var lz: cint = x264_clz(x)
#   return x264_log2_lut[(x shl lz shr 24) and 0x0000007F] + x264_log2_lz_lut[lz]

# proc x264_median*(a: var cint; b:var cint; c: cint): cint {.inline.} =
#   var t: cint = (a - b) and ((a - b) shr 31)
#   dec(a, t)
#   inc(b, t)
#   dec(b, (b - c) and ((b - c) shr 31))
#   inc(b, (a - b) and ((a - b) shr 31))
#   return b

# proc x264_median_mv*(dst: var seq[int16]; a: seq[int16]; b: seq[int16]; c: seq[int16]) {.inline.} = discard
#   # dst[0] = x264_median(a[0].cint, b[0].cint, c[0].cint)
#   # dst[1] = x264_median(a[1], b[1], c[1])

# # proc x264_predictor_difference*(mvc: array[2, int]; i_mvc: uint64): cint {.inline.} =
# #   var sum: cint = 0
# #   for i in 0.. i_mvc - 1:
# #     sum += abs(mvc[i][0] - mvc[i + 1][0]) + abs(mvc[i][1] - mvc[i + 1][1]))
# #   return sum

# proc x264_cabac_mvd_sum*(mvdleft: seq[int]; mvdtop: seq[int]): uint16 {.inline.} =
#   var amvd0: cint = cint mvdleft[0] + mvdtop[0]
#   var amvd1: cint = cint mvdleft[1] + mvdtop[1]
#   amvd0 = (amvd0 > 2) + (amvd0 > 32)
#   amvd1 = (amvd1 > 2) + (amvd1 > 32)
#   return uint16 amvd0 + (amvd1 shl 8)

