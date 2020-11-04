{.passL: "-lmingw32   -lshell32   -lavutil -lavcodec -lavfilter -lswscale -lavformat -lswresample -lavdevice  -lSDL2 -lSDL2main  -lpostproc".}

import strutils
import macros
import math
# when defined(windows):
#   import winim
# else:
import posix
import sequtils

when defined(SDL_Static):
  static: echo "SDL_Static option is deprecated and will soon be removed. Instead please use --dynlibOverride:SDL2."

else:
  when defined(windows):
    const LibName* = "SDL2.dll"
  elif defined(macosx):
    const LibName* = "libSDL2.dylib"
  elif defined(openbsd):
    const LibName* = "libSDL2.so.0.6"
  else:
    const LibName* = "libSDL2.so"


template `+`*[T](p: ptr T, off: int): ptr T =
  cast[ptr type(p[])](cast[ByteAddress](p) +% off * sizeof(p[]))

template `+=`*[T](p: ptr T, off: int) =
  p = p + off

template `-`*[T](p: ptr T, off: int): ptr T =
  cast[ptr type(p[])](cast[ByteAddress](p) -% off * sizeof(p[]))

template `-=`*[T](p: ptr T, off: int) =
  p = p - off

template `[]`*[T](p: ptr T, off: int): T =
  (p + off)[]

template `[]=`*[T](p: ptr T, off: int, val: T) =
  (p + off)[] = val

template AV_STRINGIFY*(s: untyped): untyped =
  AV_TOSTRING(s)


template AV_JOIN*(a, b: untyped): untyped =
  AV_GLUE(a, b)

template FFALIGN*(x, a: untyped): untyped =
  (((x) + (a) - 1) and not ((a) - 1))

template AV_VERSION_INT*(a, b, c: untyped): untyped =
  ((a) shl 16 or (b) shl 8 or (c))

template AV_VERSION_DOT(a, b, c) = `a. b . c`

template AV_VERSION*(a, b, c: untyped): untyped =
  AV_VERSION_DOT(a, b, c)

template AV_VERSION_MAJOR*(a: untyped): untyped =
  ((a) shr 16)

template AV_VERSION_MINOR*(a: untyped): untyped =
  (((a) and 0x0000FF00) shr 8)

template AV_VERSION_MICRO*(a: untyped): untyped =
  ((a) and 0x000000FF)

template MKTAG*(a, b, c, d: untyped): untyped =
  (a.int or (b.int shl 8) or (c.int shl 16) or (d.int shl 24))

template MKBETAG*(a, b, c, d: untyped): untyped =
  (d.int or (c.int shl 8) or (b.int shl 16) or (a.int shl 24))

template AVERROR*(e: untyped): untyped =
  (-(e))

template AVUNERROR*(e: untyped): untyped =
  (-(e))

template FFERRTAG*(a, b, c, d: untyped): untyped =
  (-cast[cint](MKTAG(a, b, c, d)))



template FF_ARRAY_ELEMS*(a: untyped): untyped =
  (sizeof((a) div sizeof(((a)[0]))))


template KMOD_CTRL*: untyped = (KMOD_LCTRL or KMOD_RCTRL)
template KMOD_SHIFT*:untyped = (KMOD_LSHIFT or KMOD_RSHIFT)
template KMOD_ALT*: untyped = (KMOD_LALT or KMOD_RALT)
template KMOD_GUI*: untyped = (KMOD_LGUI or KMOD_RGUI)


proc av_isdigit*(c: cint): cint {.inline.} =
  return cint isDigit c.char

proc av_isgraph*(c: cint): cint {.inline.} =
  return cint c > 32 and c < 127

proc av_isspace*(c: cint): cint {.inline.} =
  return isSpaceAscii(c.char).cint

proc av_toupper*(c: cint): cint {.inline.} =
  if c >= 'a'.cint and c <= 'z'.cint:
    result = c xor 0x00000020

proc av_tolower*(c: cint): cint {.inline.} =
  if c >= 'A'.cint and c <= 'Z'.cint:
    result = c xor 0x00000020

proc av_isxdigit*(c: cint): cint {.inline.} =
  result = av_tolower(c)
  result = av_isdigit(result) or cint(result >= 'a'.cint and result <= 'f'.cint)


type
  AVEscapeMode* {.size: sizeof(cint).} = enum
    AV_ESCAPE_MODE_AUTO,      ## /< Use auto-selected escaping mode.
    AV_ESCAPE_MODE_BACKSLASH, ## /< Use backslash escaping.
    AV_ESCAPE_MODE_QUOTE      ## /< Use single-quote escaping.
  AVExprEnum* {.size: sizeof(cint).} = enum
    e_value, e_const, e_func0, e_func1, e_func2, e_squish, e_gauss, e_ld, e_isnan,
    e_isinf, e_mod, e_max, e_min, e_eq, e_gt, e_gte, e_lte, e_lt, e_pow, e_mul, e_div,
    e_add, e_last, e_st, e_while, e_taylor, e_root, e_floor, e_ceil, e_trunc, e_round,
    e_sqrt, e_not, e_random, e_hypot, e_gcd, e_if, e_ifnot, e_print, e_bitand, e_bitor,
    e_between, e_clip, e_atan2, e_lerp, e_sgn
  AVMediaType* {.size: sizeof(cint).} = enum
    AVMEDIA_TYPE_UNKNOWN = -1,  ## /< Usually treated as AVMEDIA_TYPE_DATA
    AVMEDIA_TYPE_VIDEO, AVMEDIA_TYPE_AUDIO, AVMEDIA_TYPE_DATA, ## /< Opaque data information usually continuous
    AVMEDIA_TYPE_SUBTITLE, AVMEDIA_TYPE_ATTACHMENT, ## /< Opaque data information usually sparse
    AVMEDIA_TYPE_NB

type
  AVPictureType* {.size: sizeof(cint).} = enum
    AV_PICTURE_TYPE_NONE = 0,   ## /< Undefined
    AV_PICTURE_TYPE_I,        ## /< Intra
    AV_PICTURE_TYPE_P,        ## /< Predicted
    AV_PICTURE_TYPE_B,        ## /< Bi-dir predicted
    AV_PICTURE_TYPE_S,        ## /< S(GMC)-VOP MPEG-4
    AV_PICTURE_TYPE_SI,       ## /< Switching Intra
    AV_PICTURE_TYPE_SP,       ## /< Switching Predicted
    AV_PICTURE_TYPE_BI        ## /< BI type
  AVRounding* {.size: sizeof(cint).} = enum
    AV_ROUND_ZERO = 0,          ## /< Round toward zero.
    AV_ROUND_INF = 1,           ## /< Round away from zero.
    AV_ROUND_DOWN = 2,          ## /< Round toward -infinity.
    AV_ROUND_UP = 3,            ## /< Round toward +infinity.
    AV_ROUND_NEAR_INF = 5,      ## /< Round to nearest and halfway cases away from zero.
    AV_ROUND_PASS_MINMAX = 8192
  AVTimebaseSource* {.size: sizeof(cint).} = enum
    AVFMT_TBCF_AUTO = -1, AVFMT_TBCF_DECODER, AVFMT_TBCF_DEMUXER,
    AVFMT_TBCF_R_FRAMERATE
  AVOptionType* {.size: sizeof(cint).} = enum
    AV_OPT_TYPE_FLAGS, AV_OPT_TYPE_INT, AV_OPT_TYPE_INT64, AV_OPT_TYPE_DOUBLE,
    AV_OPT_TYPE_FLOAT, AV_OPT_TYPE_STRING, AV_OPT_TYPE_RATIONAL, AV_OPT_TYPE_BINARY, ## /< offset must point to a pointer immediately followed by an int for the length
    AV_OPT_TYPE_DICT, AV_OPT_TYPE_UINT64, AV_OPT_TYPE_CONST, AV_OPT_TYPE_IMAGE_SIZE, ## /< offset must point to two consecutive integers
    AV_OPT_TYPE_PIXEL_FMT, AV_OPT_TYPE_SAMPLE_FMT, AV_OPT_TYPE_VIDEO_RATE, ## /< offset must point to AVRational
    AV_OPT_TYPE_DURATION, AV_OPT_TYPE_COLOR, AV_OPT_TYPE_CHANNEL_LAYOUT,
    AV_OPT_TYPE_BOOL
  AVCodecID* {.size: sizeof(cint).} = enum
    AV_CODEC_ID_NONE, AV_CODEC_ID_MPEG1VIDEO, AV_CODEC_ID_MPEG2VIDEO, ## /< preferred ID for MPEG-1/2 video decoding
    AV_CODEC_ID_H261, AV_CODEC_ID_H263, AV_CODEC_ID_RV10, AV_CODEC_ID_RV20,
    AV_CODEC_ID_MJPEG, AV_CODEC_ID_MJPEGB, AV_CODEC_ID_LJPEG, AV_CODEC_ID_SP5X,
    AV_CODEC_ID_JPEGLS, AV_CODEC_ID_MPEG4, AV_CODEC_ID_RAWVIDEO,
    AV_CODEC_ID_MSMPEG4V1, AV_CODEC_ID_MSMPEG4V2, AV_CODEC_ID_MSMPEG4V3,
    AV_CODEC_ID_WMV1, AV_CODEC_ID_WMV2, AV_CODEC_ID_H263P, AV_CODEC_ID_H263I,
    AV_CODEC_ID_FLV1, AV_CODEC_ID_SVQ1, AV_CODEC_ID_SVQ3, AV_CODEC_ID_DVVIDEO,
    AV_CODEC_ID_HUFFYUV, AV_CODEC_ID_CYUV, AV_CODEC_ID_H264, AV_CODEC_ID_INDEO3,
    AV_CODEC_ID_VP3, AV_CODEC_ID_THEORA, AV_CODEC_ID_ASV1, AV_CODEC_ID_ASV2,
    AV_CODEC_ID_FFV1, AV_CODEC_ID_4XM, AV_CODEC_ID_VCR1, AV_CODEC_ID_CLJR,
    AV_CODEC_ID_MDEC, AV_CODEC_ID_ROQ, AV_CODEC_ID_INTERPLAY_VIDEO,
    AV_CODEC_ID_XAN_WC3, AV_CODEC_ID_XAN_WC4, AV_CODEC_ID_RPZA,
    AV_CODEC_ID_CINEPAK, AV_CODEC_ID_WS_VQA, AV_CODEC_ID_MSRLE,
    AV_CODEC_ID_MSVIDEO1, AV_CODEC_ID_IDCIN, AV_CODEC_ID_8BPS, AV_CODEC_ID_SMC,
    AV_CODEC_ID_FLIC, AV_CODEC_ID_TRUEMOTION1, AV_CODEC_ID_VMDVIDEO,
    AV_CODEC_ID_MSZH, AV_CODEC_ID_ZLIB, AV_CODEC_ID_QTRLE, AV_CODEC_ID_TSCC,
    AV_CODEC_ID_ULTI, AV_CODEC_ID_QDRAW, AV_CODEC_ID_VIXL, AV_CODEC_ID_QPEG,
    AV_CODEC_ID_PNG, AV_CODEC_ID_PPM, AV_CODEC_ID_PBM, AV_CODEC_ID_PGM,
    AV_CODEC_ID_PGMYUV, AV_CODEC_ID_PAM, AV_CODEC_ID_FFVHUFF, AV_CODEC_ID_RV30,
    AV_CODEC_ID_RV40, AV_CODEC_ID_VC1, AV_CODEC_ID_WMV3, AV_CODEC_ID_LOCO,
    AV_CODEC_ID_WNV1, AV_CODEC_ID_AASC, AV_CODEC_ID_INDEO2, AV_CODEC_ID_FRAPS,
    AV_CODEC_ID_TRUEMOTION2, AV_CODEC_ID_BMP, AV_CODEC_ID_CSCD,
    AV_CODEC_ID_MMVIDEO, AV_CODEC_ID_ZMBV, AV_CODEC_ID_AVS, AV_CODEC_ID_SMACKVIDEO,
    AV_CODEC_ID_NUV, AV_CODEC_ID_KMVC, AV_CODEC_ID_FLASHSV, AV_CODEC_ID_CAVS,
    AV_CODEC_ID_JPEG2000, AV_CODEC_ID_VMNC, AV_CODEC_ID_VP5, AV_CODEC_ID_VP6,
    AV_CODEC_ID_VP6F, AV_CODEC_ID_TARGA, AV_CODEC_ID_DSICINVIDEO,
    AV_CODEC_ID_TIERTEXSEQVIDEO, AV_CODEC_ID_TIFF, AV_CODEC_ID_GIF,
    AV_CODEC_ID_DXA, AV_CODEC_ID_DNXHD, AV_CODEC_ID_THP, AV_CODEC_ID_SGI,
    AV_CODEC_ID_C93, AV_CODEC_ID_BETHSOFTVID, AV_CODEC_ID_PTX, AV_CODEC_ID_TXD,
    AV_CODEC_ID_VP6A, AV_CODEC_ID_AMV, AV_CODEC_ID_VB, AV_CODEC_ID_PCX,
    AV_CODEC_ID_SUNRAST, AV_CODEC_ID_INDEO4, AV_CODEC_ID_INDEO5, AV_CODEC_ID_MIMIC,
    AV_CODEC_ID_RL2, AV_CODEC_ID_ESCAPE124, AV_CODEC_ID_DIRAC, AV_CODEC_ID_BFI,
    AV_CODEC_ID_CMV, AV_CODEC_ID_MOTIONPIXELS, AV_CODEC_ID_TGV, AV_CODEC_ID_TGQ,
    AV_CODEC_ID_TQI, AV_CODEC_ID_AURA, AV_CODEC_ID_AURA2, AV_CODEC_ID_V210X,
    AV_CODEC_ID_TMV, AV_CODEC_ID_V210, AV_CODEC_ID_DPX, AV_CODEC_ID_MAD,
    AV_CODEC_ID_FRWU, AV_CODEC_ID_FLASHSV2, AV_CODEC_ID_CDGRAPHICS,
    AV_CODEC_ID_R210, AV_CODEC_ID_ANM, AV_CODEC_ID_BINKVIDEO, AV_CODEC_ID_IFF_ILBM,
    AV_CODEC_ID_KGV1, AV_CODEC_ID_YOP, AV_CODEC_ID_VP8, AV_CODEC_ID_PICTOR,
    AV_CODEC_ID_ANSI, AV_CODEC_ID_A64_MULTI, AV_CODEC_ID_A64_MULTI5,
    AV_CODEC_ID_R10K, AV_CODEC_ID_MXPEG, AV_CODEC_ID_LAGARITH, AV_CODEC_ID_PRORES,
    AV_CODEC_ID_JV, AV_CODEC_ID_DFA, AV_CODEC_ID_WMV3IMAGE, AV_CODEC_ID_VC1IMAGE,
    AV_CODEC_ID_UTVIDEO, AV_CODEC_ID_BMV_VIDEO, AV_CODEC_ID_VBLE,
    AV_CODEC_ID_DXTORY, AV_CODEC_ID_V410, AV_CODEC_ID_XWD, AV_CODEC_ID_CDXL,
    AV_CODEC_ID_XBM, AV_CODEC_ID_ZEROCODEC, AV_CODEC_ID_MSS1, AV_CODEC_ID_MSA1,
    AV_CODEC_ID_TSCC2, AV_CODEC_ID_MTS2, AV_CODEC_ID_CLLC, AV_CODEC_ID_MSS2,
    AV_CODEC_ID_VP9, AV_CODEC_ID_AIC, AV_CODEC_ID_ESCAPE130, AV_CODEC_ID_G2M,
    AV_CODEC_ID_WEBP, AV_CODEC_ID_HNM4_VIDEO, AV_CODEC_ID_HEVC, AV_CODEC_ID_FIC,
    AV_CODEC_ID_ALIAS_PIX, AV_CODEC_ID_BRENDER_PIX, AV_CODEC_ID_PAF_VIDEO,
    AV_CODEC_ID_EXR, AV_CODEC_ID_VP7, AV_CODEC_ID_SANM, AV_CODEC_ID_SGIRLE,
    AV_CODEC_ID_MVC1, AV_CODEC_ID_MVC2, AV_CODEC_ID_HQX, AV_CODEC_ID_TDSC,
    AV_CODEC_ID_HQ_HQA, AV_CODEC_ID_HAP, AV_CODEC_ID_DDS, AV_CODEC_ID_DXV,
    AV_CODEC_ID_SCREENPRESSO, AV_CODEC_ID_RSCC, AV_CODEC_ID_AVS2, AV_CODEC_ID_PGX,
    AV_CODEC_ID_AVS3, AV_CODEC_ID_Y41P = 0x00008000, AV_CODEC_ID_AVRP,
    AV_CODEC_ID_012V, AV_CODEC_ID_AVUI, AV_CODEC_ID_AYUV, AV_CODEC_ID_TARGA_Y216,
    AV_CODEC_ID_V308, AV_CODEC_ID_V408, AV_CODEC_ID_YUV4, AV_CODEC_ID_AVRN,
    AV_CODEC_ID_CPIA, AV_CODEC_ID_XFACE, AV_CODEC_ID_SNOW, AV_CODEC_ID_SMVJPEG,
    AV_CODEC_ID_APNG, AV_CODEC_ID_DAALA, AV_CODEC_ID_CFHD,
    AV_CODEC_ID_TRUEMOTION2RT, AV_CODEC_ID_M101, AV_CODEC_ID_MAGICYUV,
    AV_CODEC_ID_SHEERVIDEO, AV_CODEC_ID_YLC, AV_CODEC_ID_PSD, AV_CODEC_ID_PIXLET,
    AV_CODEC_ID_SPEEDHQ, AV_CODEC_ID_FMVC, AV_CODEC_ID_SCPR,
    AV_CODEC_ID_CLEARVIDEO, AV_CODEC_ID_XPM, AV_CODEC_ID_AV1,
    AV_CODEC_ID_BITPACKED, AV_CODEC_ID_MSCC, AV_CODEC_ID_SRGC, AV_CODEC_ID_SVG,
    AV_CODEC_ID_GDV, AV_CODEC_ID_FITS, AV_CODEC_ID_IMM4, AV_CODEC_ID_PROSUMER,
    AV_CODEC_ID_MWSC, AV_CODEC_ID_WCMV, AV_CODEC_ID_RASC, AV_CODEC_ID_HYMT,
    AV_CODEC_ID_ARBC, AV_CODEC_ID_AGM, AV_CODEC_ID_LSCR, AV_CODEC_ID_VP4,
    AV_CODEC_ID_IMM5, AV_CODEC_ID_MVDV, AV_CODEC_ID_MVHA, AV_CODEC_ID_CDTOONS,
    AV_CODEC_ID_MV30, AV_CODEC_ID_NOTCHLC, AV_CODEC_ID_PFM, AV_CODEC_ID_MOBICLIP,
    AV_CODEC_ID_PHOTOCD, AV_CODEC_ID_IPU, AV_CODEC_ID_ARGO, AV_CODEC_ID_CRI, AV_CODEC_ID_FIRST_AUDIO = 0x00010000, ## /< A dummy id pointing at the start of audio codecs
    AV_CODEC_ID_PCM_S16BE, AV_CODEC_ID_PCM_U16LE, AV_CODEC_ID_PCM_U16BE,
    AV_CODEC_ID_PCM_S8, AV_CODEC_ID_PCM_U8, AV_CODEC_ID_PCM_MULAW,
    AV_CODEC_ID_PCM_ALAW, AV_CODEC_ID_PCM_S32LE, AV_CODEC_ID_PCM_S32BE,
    AV_CODEC_ID_PCM_U32LE, AV_CODEC_ID_PCM_U32BE, AV_CODEC_ID_PCM_S24LE,
    AV_CODEC_ID_PCM_S24BE, AV_CODEC_ID_PCM_U24LE, AV_CODEC_ID_PCM_U24BE,
    AV_CODEC_ID_PCM_S24DAUD, AV_CODEC_ID_PCM_ZORK, AV_CODEC_ID_PCM_S16LE_PLANAR,
    AV_CODEC_ID_PCM_DVD, AV_CODEC_ID_PCM_F32BE, AV_CODEC_ID_PCM_F32LE,
    AV_CODEC_ID_PCM_F64BE, AV_CODEC_ID_PCM_F64LE, AV_CODEC_ID_PCM_BLURAY,
    AV_CODEC_ID_PCM_LXF, AV_CODEC_ID_S302M, AV_CODEC_ID_PCM_S8_PLANAR,
    AV_CODEC_ID_PCM_S24LE_PLANAR, AV_CODEC_ID_PCM_S32LE_PLANAR,
    AV_CODEC_ID_PCM_S16BE_PLANAR, AV_CODEC_ID_PCM_S64LE = 0x00010800,
    AV_CODEC_ID_PCM_S64BE, AV_CODEC_ID_PCM_F16LE, AV_CODEC_ID_PCM_F24LE,
    AV_CODEC_ID_PCM_VIDC, AV_CODEC_ID_ADPCM_IMA_QT = 0x00011000,
    AV_CODEC_ID_ADPCM_IMA_WAV, AV_CODEC_ID_ADPCM_IMA_DK3,
    AV_CODEC_ID_ADPCM_IMA_DK4, AV_CODEC_ID_ADPCM_IMA_WS,
    AV_CODEC_ID_ADPCM_IMA_SMJPEG, AV_CODEC_ID_ADPCM_MS, AV_CODEC_ID_ADPCM_4XM,
    AV_CODEC_ID_ADPCM_XA, AV_CODEC_ID_ADPCM_ADX, AV_CODEC_ID_ADPCM_EA,
    AV_CODEC_ID_ADPCM_G726, AV_CODEC_ID_ADPCM_CT, AV_CODEC_ID_ADPCM_SWF,
    AV_CODEC_ID_ADPCM_YAMAHA, AV_CODEC_ID_ADPCM_SBPRO_4,
    AV_CODEC_ID_ADPCM_SBPRO_3, AV_CODEC_ID_ADPCM_SBPRO_2, AV_CODEC_ID_ADPCM_THP,
    AV_CODEC_ID_ADPCM_IMA_AMV, AV_CODEC_ID_ADPCM_EA_R1, AV_CODEC_ID_ADPCM_EA_R3,
    AV_CODEC_ID_ADPCM_EA_R2, AV_CODEC_ID_ADPCM_IMA_EA_SEAD,
    AV_CODEC_ID_ADPCM_IMA_EA_EACS, AV_CODEC_ID_ADPCM_EA_XAS,
    AV_CODEC_ID_ADPCM_EA_MAXIS_XA, AV_CODEC_ID_ADPCM_IMA_ISS,
    AV_CODEC_ID_ADPCM_G722, AV_CODEC_ID_ADPCM_IMA_APC, AV_CODEC_ID_ADPCM_VIMA,
    AV_CODEC_ID_ADPCM_AFC = 0x00011800, AV_CODEC_ID_ADPCM_IMA_OKI,
    AV_CODEC_ID_ADPCM_DTK, AV_CODEC_ID_ADPCM_IMA_RAD, AV_CODEC_ID_ADPCM_G726LE,
    AV_CODEC_ID_ADPCM_THP_LE, AV_CODEC_ID_ADPCM_PSX, AV_CODEC_ID_ADPCM_AICA,
    AV_CODEC_ID_ADPCM_IMA_DAT4, AV_CODEC_ID_ADPCM_MTAF, AV_CODEC_ID_ADPCM_AGM,
    AV_CODEC_ID_ADPCM_ARGO, AV_CODEC_ID_ADPCM_IMA_SSI, AV_CODEC_ID_ADPCM_ZORK,
    AV_CODEC_ID_ADPCM_IMA_APM, AV_CODEC_ID_ADPCM_IMA_ALP,
    AV_CODEC_ID_ADPCM_IMA_MTF, AV_CODEC_ID_ADPCM_IMA_CUNNING,
    AV_CODEC_ID_ADPCM_IMA_MOFLEX, AV_CODEC_ID_AMR_NB = 0x00012000,
    AV_CODEC_ID_AMR_WB, AV_CODEC_ID_RA_144 = 0x00013000, AV_CODEC_ID_RA_288,
    AV_CODEC_ID_ROQ_DPCM = 0x00014000, AV_CODEC_ID_INTERPLAY_DPCM,
    AV_CODEC_ID_XAN_DPCM, AV_CODEC_ID_SOL_DPCM,
    AV_CODEC_ID_SDX2_DPCM = 0x00014800, AV_CODEC_ID_GREMLIN_DPCM,
    AV_CODEC_ID_DERF_DPCM, AV_CODEC_ID_MP2 = 0x00015000, AV_CODEC_ID_MP3, ## /< preferred ID for decoding MPEG audio layer 1, 2 or 3
    AV_CODEC_ID_AAC, AV_CODEC_ID_AC3, AV_CODEC_ID_DTS, AV_CODEC_ID_VORBIS,
    AV_CODEC_ID_DVAUDIO, AV_CODEC_ID_WMAV1, AV_CODEC_ID_WMAV2, AV_CODEC_ID_MACE3,
    AV_CODEC_ID_MACE6, AV_CODEC_ID_VMDAUDIO, AV_CODEC_ID_FLAC, AV_CODEC_ID_MP3ADU,
    AV_CODEC_ID_MP3ON4, AV_CODEC_ID_SHORTEN, AV_CODEC_ID_ALAC,
    AV_CODEC_ID_WESTWOOD_SND1, AV_CODEC_ID_GSM, ## /< as in Berlin toast format
    AV_CODEC_ID_QDM2, AV_CODEC_ID_COOK, AV_CODEC_ID_TRUESPEECH, AV_CODEC_ID_TTA,
    AV_CODEC_ID_SMACKAUDIO, AV_CODEC_ID_QCELP, AV_CODEC_ID_WAVPACK,
    AV_CODEC_ID_DSICINAUDIO, AV_CODEC_ID_IMC, AV_CODEC_ID_MUSEPACK7,
    AV_CODEC_ID_MLP, AV_CODEC_ID_GSM_MS, AV_CODEC_ID_ATRAC3, AV_CODEC_ID_APE,
    AV_CODEC_ID_NELLYMOSER, AV_CODEC_ID_MUSEPACK8, AV_CODEC_ID_SPEEX,
    AV_CODEC_ID_WMAVOICE, AV_CODEC_ID_WMAPRO, AV_CODEC_ID_WMALOSSLESS,
    AV_CODEC_ID_ATRAC3P, AV_CODEC_ID_EAC3, AV_CODEC_ID_SIPR, AV_CODEC_ID_MP1,
    AV_CODEC_ID_TWINVQ, AV_CODEC_ID_TRUEHD, AV_CODEC_ID_MP4ALS, AV_CODEC_ID_ATRAC1,
    AV_CODEC_ID_BINKAUDIO_RDFT, AV_CODEC_ID_BINKAUDIO_DCT, AV_CODEC_ID_AAC_LATM,
    AV_CODEC_ID_QDMC, AV_CODEC_ID_CELT, AV_CODEC_ID_G723_1, AV_CODEC_ID_G729,
    AV_CODEC_ID_8SVX_EXP, AV_CODEC_ID_8SVX_FIB, AV_CODEC_ID_BMV_AUDIO,
    AV_CODEC_ID_RALF, AV_CODEC_ID_IAC, AV_CODEC_ID_ILBC, AV_CODEC_ID_OPUS,
    AV_CODEC_ID_COMFORT_NOISE, AV_CODEC_ID_TAK, AV_CODEC_ID_METASOUND,
    AV_CODEC_ID_PAF_AUDIO, AV_CODEC_ID_ON2AVC, AV_CODEC_ID_DSS_SP,
    AV_CODEC_ID_CODEC2, AV_CODEC_ID_FFWAVESYNTH = 0x00015800, AV_CODEC_ID_SONIC,
    AV_CODEC_ID_SONIC_LS, AV_CODEC_ID_EVRC, AV_CODEC_ID_SMV, AV_CODEC_ID_DSD_LSBF,
    AV_CODEC_ID_DSD_MSBF, AV_CODEC_ID_DSD_LSBF_PLANAR,
    AV_CODEC_ID_DSD_MSBF_PLANAR, AV_CODEC_ID_4GV, AV_CODEC_ID_INTERPLAY_ACM,
    AV_CODEC_ID_XMA1, AV_CODEC_ID_XMA2, AV_CODEC_ID_DST, AV_CODEC_ID_ATRAC3AL,
    AV_CODEC_ID_ATRAC3PAL, AV_CODEC_ID_DOLBY_E, AV_CODEC_ID_APTX,
    AV_CODEC_ID_APTX_HD, AV_CODEC_ID_SBC, AV_CODEC_ID_ATRAC9, AV_CODEC_ID_HCOM,
    AV_CODEC_ID_ACELP_KELVIN, AV_CODEC_ID_MPEGH_3D_AUDIO, AV_CODEC_ID_SIREN,
    AV_CODEC_ID_HCA, AV_CODEC_ID_FASTAUDIO, AV_CODEC_ID_FIRST_SUBTITLE = 0x00017000, ## /< A dummy ID pointing at the start of subtitle codecs.
    AV_CODEC_ID_DVB_SUBTITLE, AV_CODEC_ID_TEXT, ## /< raw UTF-8 text
    AV_CODEC_ID_XSUB, AV_CODEC_ID_SSA, AV_CODEC_ID_MOV_TEXT,
    AV_CODEC_ID_HDMV_PGS_SUBTITLE, AV_CODEC_ID_DVB_TELETEXT, AV_CODEC_ID_SRT,
    AV_CODEC_ID_MICRODVD = 0x00017800, AV_CODEC_ID_EIA_608, AV_CODEC_ID_JACOSUB,
    AV_CODEC_ID_SAMI, AV_CODEC_ID_REALTEXT, AV_CODEC_ID_STL,
    AV_CODEC_ID_SUBVIEWER1, AV_CODEC_ID_SUBVIEWER, AV_CODEC_ID_SUBRIP,
    AV_CODEC_ID_WEBVTT, AV_CODEC_ID_MPL2, AV_CODEC_ID_VPLAYER, AV_CODEC_ID_PJS,
    AV_CODEC_ID_ASS, AV_CODEC_ID_HDMV_TEXT_SUBTITLE, AV_CODEC_ID_TTML,
    AV_CODEC_ID_ARIB_CAPTION, AV_CODEC_ID_FIRST_UNKNOWN = 0x00018000, ## /< A dummy ID pointing at the start of various fake codecs.
    AV_CODEC_ID_SCTE_35,      ## /< Contain timestamp estimated through PCR of program stream.
    AV_CODEC_ID_EPG, AV_CODEC_ID_BINTEXT = 0x00018800, AV_CODEC_ID_XBIN,
    AV_CODEC_ID_IDF, AV_CODEC_ID_OTF, AV_CODEC_ID_SMPTE_KLV, AV_CODEC_ID_DVD_NAV,
    AV_CODEC_ID_TIMED_ID3, AV_CODEC_ID_BIN_DATA, AV_CODEC_ID_PROBE = 0x00019000, ## /< codec_id is not known (like AV_CODEC_ID_NONE) but lavf should attempt to identify it
    AV_CODEC_ID_MPEG2TS = 0x00020000, AV_CODEC_ID_MPEG4SYSTEMS = 0x00020001, AV_CODEC_ID_FFMETADATA = 0x00021000, ## /< Dummy codec for streams containing only metadata information.
    AV_CODEC_ID_WRAPPED_AVFRAME = 0x00021001 ## /< Passthrough codec, AVFrames wrapped in AVPacket
  AVPacketSideDataType* {.size: sizeof(cint).} = enum
    AV_PKT_DATA_PALETTE, AV_PKT_DATA_NEW_EXTRADATA, AV_PKT_DATA_PARAM_CHANGE,
    AV_PKT_DATA_H263_MB_INFO, AV_PKT_DATA_REPLAYGAIN, AV_PKT_DATA_DISPLAYMATRIX,
    AV_PKT_DATA_STEREO3D, AV_PKT_DATA_AUDIO_SERVICE_TYPE,
    AV_PKT_DATA_QUALITY_STATS, AV_PKT_DATA_FALLBACK_TRACK,
    AV_PKT_DATA_CPB_PROPERTIES, AV_PKT_DATA_SKIP_SAMPLES, AV_PKT_DATA_JP_DUALMONO,
    AV_PKT_DATA_STRINGS_METADATA, AV_PKT_DATA_SUBTITLE_POSITION,
    AV_PKT_DATA_MATROSKA_BLOCKADDITIONAL, AV_PKT_DATA_WEBVTT_IDENTIFIER,
    AV_PKT_DATA_WEBVTT_SETTINGS, AV_PKT_DATA_METADATA_UPDATE,
    AV_PKT_DATA_MPEGTS_STREAM_ID, AV_PKT_DATA_MASTERING_DISPLAY_METADATA,
    AV_PKT_DATA_SPHERICAL, AV_PKT_DATA_CONTENT_LIGHT_LEVEL, AV_PKT_DATA_A53_CC,
    AV_PKT_DATA_ENCRYPTION_INIT_INFO, AV_PKT_DATA_ENCRYPTION_INFO,
    AV_PKT_DATA_AFD, AV_PKT_DATA_PRFT, AV_PKT_DATA_ICC_PROFILE,
    AV_PKT_DATA_DOVI_CONF, AV_PKT_DATA_S12M_TIMECODE, AV_PKT_DATA_NB
  SDL_AssertState* {.size: sizeof(cint).} = enum
    SDL_ASSERTION_RETRY, SDL_ASSERTION_BREAK, SDL_ASSERTION_ABORT,
    SDL_ASSERTION_IGNORE, SDL_ASSERTION_ALWAYS_IGNORE
  sDL_PixelType* {.size: sizeof(cint).} = enum
    SDL_PIXELTYPE_UNKNOWN, SDL_PIXELTYPE_INDEX1, SDL_PIXELTYPE_INDEX4,
    SDL_PIXELTYPE_INDEX8, SDL_PIXELTYPE_PACKED8, SDL_PIXELTYPE_PACKED16,
    SDL_PIXELTYPE_PACKED32, SDL_PIXELTYPE_ARRAYU8, SDL_PIXELTYPE_ARRAYU16,
    SDL_PIXELTYPE_ARRAYU32, SDL_PIXELTYPE_ARRAYF16, SDL_PIXELTYPE_ARRAYF32
  SDL_BitmapOrder* {.size: sizeof(cint).} = enum
    SDL_BITMAPORDER_NONE, SDL_BITMAPORDER_4321, SDL_BITMAPORDER_1234
  SDL_PackedOrder* {.size: sizeof(cint).} = enum
    SDL_PACKEDORDER_NONE, SDL_PACKEDORDER_XRGB, SDL_PACKEDORDER_RGBX,
    SDL_PACKEDORDER_ARGB, SDL_PACKEDORDER_RGBA, SDL_PACKEDORDER_XBGR,
    SDL_PACKEDORDER_BGRX, SDL_PACKEDORDER_ABGR, SDL_PACKEDORDER_BGRA
  SDL_ArrayOrder* {.size: sizeof(cint).} = enum
    SDL_ARRAYORDER_NONE, SDL_ARRAYORDER_RGB, SDL_ARRAYORDER_RGBA,
    SDL_ARRAYORDER_ARGB, SDL_ARRAYORDER_BGR, SDL_ARRAYORDER_BGRA,
    SDL_ARRAYORDER_ABGR
  SDL_PackedLayout* {.size: sizeof(cint).} = enum
    SDL_PACKEDLAYOUT_NONE, SDL_PACKEDLAYOUT_332, SDL_PACKEDLAYOUT_4444,
    SDL_PACKEDLAYOUT_1555, SDL_PACKEDLAYOUT_5551, SDL_PACKEDLAYOUT_565,
    SDL_PACKEDLAYOUT_8888, SDL_PACKEDLAYOUT_2101010, SDL_PACKEDLAYOUT_1010102
  SDL_ScanCode* {.size: sizeof(cint).} = enum
    SDL_SCANCODE_UNKNOWN = 0,
    SDL_SCANCODE_A = 4, SDL_SCANCODE_B = 5, SDL_SCANCODE_C = 6,
    SDL_SCANCODE_D = 7, SDL_SCANCODE_E = 8, SDL_SCANCODE_F = 9,
    SDL_SCANCODE_G = 10, SDL_SCANCODE_H = 11, SDL_SCANCODE_I = 12,
    SDL_SCANCODE_J = 13, SDL_SCANCODE_K = 14, SDL_SCANCODE_L = 15,
    SDL_SCANCODE_M = 16, SDL_SCANCODE_N = 17, SDL_SCANCODE_O = 18,
    SDL_SCANCODE_P = 19, SDL_SCANCODE_Q = 20, SDL_SCANCODE_R = 21,
    SDL_SCANCODE_S = 22, SDL_SCANCODE_T = 23, SDL_SCANCODE_U = 24,
    SDL_SCANCODE_V = 25, SDL_SCANCODE_W = 26, SDL_SCANCODE_X = 27,
    SDL_SCANCODE_Y = 28, SDL_SCANCODE_Z = 29, SDL_SCANCODE_1 = 30,
    SDL_SCANCODE_2 = 31, SDL_SCANCODE_3 = 32, SDL_SCANCODE_4 = 33,
    SDL_SCANCODE_5 = 34, SDL_SCANCODE_6 = 35, SDL_SCANCODE_7 = 36,
    SDL_SCANCODE_8 = 37, SDL_SCANCODE_9 = 38, SDL_SCANCODE_0 = 39,
    SDL_SCANCODE_RETURN = 40, SDL_SCANCODE_ESCAPE = 41,
    SDL_SCANCODE_BACKSPACE = 42, SDL_SCANCODE_TAB = 43,
    SDL_SCANCODE_SPACE = 44, SDL_SCANCODE_MINUS = 45,
    SDL_SCANCODE_EQUALS = 46, SDL_SCANCODE_LEFTBRACKET = 47,
    SDL_SCANCODE_RIGHTBRACKET = 48, SDL_SCANCODE_BACKSLASH = 49,
    SDL_SCANCODE_NONUSHASH = 50, #
    SDL_SCANCODE_SEMICOLON = 51, SDL_SCANCODE_APOSTROPHE = 52, SDL_SCANCODE_GRAVE = 53,
    SDL_SCANCODE_COMMA = 54, SDL_SCANCODE_PERIOD = 55,
    SDL_SCANCODE_SLASH = 56, SDL_SCANCODE_CAPSLOCK = 57, SDL_SCANCODE_F1 = 58,
    SDL_SCANCODE_F2 = 59, SDL_SCANCODE_F3 = 60, SDL_SCANCODE_F4 = 61,
    SDL_SCANCODE_F5 = 62, SDL_SCANCODE_F6 = 63, SDL_SCANCODE_F7 = 64,
    SDL_SCANCODE_F8 = 65, SDL_SCANCODE_F9 = 66, SDL_SCANCODE_F10 = 67,
    SDL_SCANCODE_F11 = 68, SDL_SCANCODE_F12 = 69,
    SDL_SCANCODE_PRINTSCREEN = 70, SDL_SCANCODE_SCROLLLOCK = 71,
    SDL_SCANCODE_PAUSE = 72, SDL_SCANCODE_INSERT = 73,
    SDL_SCANCODE_HOME = 74, SDL_SCANCODE_PAGEUP = 75,
    SDL_SCANCODE_DELETE = 76, SDL_SCANCODE_END = 77,
    SDL_SCANCODE_PAGEDOWN = 78, SDL_SCANCODE_RIGHT = 79,
    SDL_SCANCODE_LEFT = 80, SDL_SCANCODE_DOWN = 81, SDL_SCANCODE_UP = 82, SDL_SCANCODE_NUMLOCKCLEAR = 83,
    SDL_SCANCODE_KP_DIVIDE = 84, SDL_SCANCODE_KP_MULTIPLY = 85,
    SDL_SCANCODE_KP_MINUS = 86, SDL_SCANCODE_KP_PLUS = 87,
    SDL_SCANCODE_KP_ENTER = 88, SDL_SCANCODE_KP_1 = 89,
    SDL_SCANCODE_KP_2 = 90, SDL_SCANCODE_KP_3 = 91, SDL_SCANCODE_KP_4 = 92,
    SDL_SCANCODE_KP_5 = 93, SDL_SCANCODE_KP_6 = 94, SDL_SCANCODE_KP_7 = 95,
    SDL_SCANCODE_KP_8 = 96, SDL_SCANCODE_KP_9 = 97, SDL_SCANCODE_KP_0 = 98,
    SDL_SCANCODE_KP_PERIOD = 99, SDL_SCANCODE_NONUSBACKSLASH = 100,
    SDL_SCANCODE_APPLICATION = 101, #*< windows contextual menu, compose
    SDL_SCANCODE_POWER = 102,
    SDL_SCANCODE_KP_EQUALS = 103, SDL_SCANCODE_F13 = 104,
    SDL_SCANCODE_F14 = 105, SDL_SCANCODE_F15 = 106, SDL_SCANCODE_F16 = 107,
    SDL_SCANCODE_F17 = 108, SDL_SCANCODE_F18 = 109, SDL_SCANCODE_F19 = 110,
    SDL_SCANCODE_F20 = 111, SDL_SCANCODE_F21 = 112, SDL_SCANCODE_F22 = 113,
    SDL_SCANCODE_F23 = 114, SDL_SCANCODE_F24 = 115,
    SDL_SCANCODE_EXECUTE = 116, SDL_SCANCODE_HELP = 117,
    SDL_SCANCODE_MENU = 118, SDL_SCANCODE_SELECT = 119,
    SDL_SCANCODE_STOP = 120, SDL_SCANCODE_AGAIN = 121, #*< redo
    SDL_SCANCODE_UNDO = 122, SDL_SCANCODE_CUT = 123, SDL_SCANCODE_COPY = 124,
    SDL_SCANCODE_PASTE = 125, SDL_SCANCODE_FIND = 126,
    SDL_SCANCODE_MUTE = 127, SDL_SCANCODE_VOLUMEUP = 128, SDL_SCANCODE_VOLUMEDOWN = 129,
    SDL_SCANCODE_KP_COMMA = 133, SDL_SCANCODE_KP_EQUALSAS400 = 134, SDL_SCANCODE_INTERNATIONAL1 = 135,
    SDL_SCANCODE_INTERNATIONAL2 = 136, SDL_SCANCODE_INTERNATIONAL3 = 137, #*< Yen
    SDL_SCANCODE_INTERNATIONAL4 = 138, SDL_SCANCODE_INTERNATIONAL5 = 139,
    SDL_SCANCODE_INTERNATIONAL6 = 140, SDL_SCANCODE_INTERNATIONAL7 = 141,
    SDL_SCANCODE_INTERNATIONAL8 = 142, SDL_SCANCODE_INTERNATIONAL9 = 143, SDL_SCANCODE_LANG1 = 144, #*< Hangul/English toggle
    SDL_SCANCODE_LANG2 = 145, #*< Hanja conversion
    SDL_SCANCODE_LANG3 = 146, #*< Katakana
    SDL_SCANCODE_LANG4 = 147, #*< Hiragana
    SDL_SCANCODE_LANG5 = 148, #*< Zenkaku/Hankaku
    SDL_SCANCODE_LANG6 = 149, #*< reserved
    SDL_SCANCODE_LANG7 = 150, #*< reserved
    SDL_SCANCODE_LANG8 = 151, #*< reserved
    SDL_SCANCODE_LANG9 = 152, #*< reserved
    SDL_SCANCODE_ALTERASE = 153, #*< Erase-Eaze
    SDL_SCANCODE_SYSREQ = 154, SDL_SCANCODE_CANCEL = 155,
    SDL_SCANCODE_CLEAR = 156, SDL_SCANCODE_PRIOR = 157,
    SDL_SCANCODE_RETURN2 = 158, SDL_SCANCODE_SEPARATOR = 159,
    SDL_SCANCODE_OUT = 160, SDL_SCANCODE_OPER = 161,
    SDL_SCANCODE_CLEARAGAIN = 162, SDL_SCANCODE_CRSEL = 163,
    SDL_SCANCODE_EXSEL = 164, SDL_SCANCODE_KP_00 = 176,
    SDL_SCANCODE_KP_000 = 177, SDL_SCANCODE_THOUSANDSSEPARATOR = 178,
    SDL_SCANCODE_DECIMALSEPARATOR = 179, SDL_SCANCODE_CURRENCYUNIT = 180,
    SDL_SCANCODE_CURRENCYSUBUNIT = 181, SDL_SCANCODE_KP_LEFTPAREN = 182,
    SDL_SCANCODE_KP_RIGHTPAREN = 183, SDL_SCANCODE_KP_LEFTBRACE = 184,
    SDL_SCANCODE_KP_RIGHTBRACE = 185, SDL_SCANCODE_KP_TAB = 186,
    SDL_SCANCODE_KP_BACKSPACE = 187, SDL_SCANCODE_KP_A = 188,
    SDL_SCANCODE_KP_B = 189, SDL_SCANCODE_KP_C = 190, SDL_SCANCODE_KP_D = 191,
    SDL_SCANCODE_KP_E = 192, SDL_SCANCODE_KP_F = 193,
    SDL_SCANCODE_KP_XOR = 194, SDL_SCANCODE_KP_POWER = 195,
    SDL_SCANCODE_KP_PERCENT = 196, SDL_SCANCODE_KP_LESS = 197,
    SDL_SCANCODE_KP_GREATER = 198, SDL_SCANCODE_KP_AMPERSAND = 199,
    SDL_SCANCODE_KP_DBLAMPERSAND = 200, SDL_SCANCODE_KP_VERTICALBAR = 201,
    SDL_SCANCODE_KP_DBLVERTICALBAR = 202, SDL_SCANCODE_KP_COLON = 203,
    SDL_SCANCODE_KP_HASH = 204, SDL_SCANCODE_KP_SPACE = 205,
    SDL_SCANCODE_KP_AT = 206, SDL_SCANCODE_KP_EXCLAM = 207,
    SDL_SCANCODE_KP_MEMSTORE = 208, SDL_SCANCODE_KP_MEMRECALL = 209,
    SDL_SCANCODE_KP_MEMCLEAR = 210, SDL_SCANCODE_KP_MEMADD = 211,
    SDL_SCANCODE_KP_MEMSUBTRACT = 212, SDL_SCANCODE_KP_MEMMULTIPLY = 213,
    SDL_SCANCODE_KP_MEMDIVIDE = 214, SDL_SCANCODE_KP_PLUSMINUS = 215,
    SDL_SCANCODE_KP_CLEAR = 216, SDL_SCANCODE_KP_CLEARENTRY = 217,
    SDL_SCANCODE_KP_BINARY = 218, SDL_SCANCODE_KP_OCTAL = 219,
    SDL_SCANCODE_KP_DECIMAL = 220, SDL_SCANCODE_KP_HEXADECIMAL = 221,
    SDL_SCANCODE_LCTRL = 224, SDL_SCANCODE_LSHIFT = 225, SDL_SCANCODE_LALT = 226, #*< alt, option
    SDL_SCANCODE_LGUI = 227, #*< windows, command (apple), meta
    SDL_SCANCODE_RCTRL = 228, SDL_SCANCODE_RSHIFT = 229, SDL_SCANCODE_RALT = 230, #*< alt gr, option
    SDL_SCANCODE_RGUI = 231, #*< windows, command (apple), meta
    SDL_SCANCODE_MODE = 257,
    SDL_SCANCODE_AUDIONEXT = 258, SDL_SCANCODE_AUDIOPREV = 259,
    SDL_SCANCODE_AUDIOSTOP = 260, SDL_SCANCODE_AUDIOPLAY = 261,
    SDL_SCANCODE_AUDIOMUTE = 262, SDL_SCANCODE_MEDIASELECT = 263,
    SDL_SCANCODE_WWW = 264, SDL_SCANCODE_MAIL = 265,
    SDL_SCANCODE_CALCULATOR = 266, SDL_SCANCODE_COMPUTER = 267,
    SDL_SCANCODE_AC_SEARCH = 268, SDL_SCANCODE_AC_HOME = 269,
    SDL_SCANCODE_AC_BACK = 270, SDL_SCANCODE_AC_FORWARD = 271,
    SDL_SCANCODE_AC_STOP = 272, SDL_SCANCODE_AC_REFRESH = 273, SDL_SCANCODE_AC_BOOKMARKS = 274,
    SDL_SCANCODE_BRIGHTNESSDOWN = 275, SDL_SCANCODE_BRIGHTNESSUP = 276, SDL_SCANCODE_DISPLAYSWITCH = 277,
    SDL_SCANCODE_KBDILLUMTOGGLE = 278, SDL_SCANCODE_KBDILLUMDOWN = 279,
    SDL_SCANCODE_KBDILLUMUP = 280, SDL_SCANCODE_EJECT = 281, SDL_SCANCODE_SLEEP = 282,
    SDL_NUM_SCANCODES = 512 

  AVAppToDevMessageType* {.size: sizeof(cint).} = enum
    AV_APP_TO_DEV_NONE = MKBETAG('N', 'O', 'N', 'E'),
    AV_APP_TO_DEV_WINDOW_SIZE #= MKBETAG('G', 'E', 'O', 'M'),
    AV_APP_TO_DEV_WINDOW_REPAINT #= MKBETAG('R', 'E', 'P', 'A'),
    AV_APP_TO_DEV_PAUSE #= MKBETAG('P', 'A', 'U', ' '),
    AV_APP_TO_DEV_PLAY #= MKBETAG('P', 'L', 'A', 'Y'),
    AV_APP_TO_DEV_TOGGLE_PAUSE #= MKBETAG('P', 'A', 'U', 'T'),
    AV_APP_TO_DEV_SET_VOLUME #= MKBETAG('S', 'V', 'O', 'L'),
    AV_APP_TO_DEV_MUTE #= MKBETAG(' ', 'M', 'U', 'T'),
    AV_APP_TO_DEV_UNMUTE #= MKBETAG('U', 'M', 'U', 'T'),
    AV_APP_TO_DEV_TOGGLE_MUTE #= MKBETAG('T', 'M', 'U', 'T'),
    AV_APP_TO_DEV_GET_VOLUME #= MKBETAG('G', 'V', 'O', 'L'),
    AV_APP_TO_DEV_GET_MUTE #= MKBETAG('G', 'M', 'U', 'T')

  AVDevToAppMessageType* {.size: sizeof(cint).} = enum
    AV_DEV_TO_APP_NONE #= MKBETAG('N', 'O', 'N', 'E'),
    AV_DEV_TO_APP_CREATE_WINDOW_BUFFER #= MKBETAG('B', 'C', 'R', 'E'),
    AV_DEV_TO_APP_PREPARE_WINDOW_BUFFER# = MKBETAG('B', 'P', 'R', 'E'),
    AV_DEV_TO_APP_DISPLAY_WINDOW_BUFFER #= MKBETAG('B', 'D', 'I', 'S'),
    AV_DEV_TO_APP_DESTROY_WINDOW_BUFFER #= MKBETAG('B', 'D', 'E', 'S'),
    AV_DEV_TO_APP_BUFFER_OVERFLOW #= MKBETAG('B', 'O', 'F', 'L'),
    AV_DEV_TO_APP_BUFFER_UNDERFLOW #= MKBETAG('B', 'U', 'F', 'L'),
    AV_DEV_TO_APP_BUFFER_READABLE #= MKBETAG('B', 'R', 'D', ' '),
    AV_DEV_TO_APP_BUFFER_WRITABLE #= MKBETAG('B', 'W', 'R', ' '),
    AV_DEV_TO_APP_MUTE_STATE_CHANGED #= MKBETAG('C', 'M', 'U', 'T'),
    AV_DEV_TO_APP_VOLUME_LEVEL_CHANGED #= MKBETAG('C', 'V', 'O', 'L')

  SDL_Keymod* {.size: sizeof(cint).} = enum
    KMOD_NONE = 0x0000, KMOD_LSHIFT = 0x0001, KMOD_RSHIFT = 0x0002,
    KMOD_LCTRL= 0x0040, KMOD_RCTRL  = 0x0080, KMOD_LALT   = 0x0100,
    KMOD_RALT = 0x0200, KMOD_LGUI   = 0x0400, KMOD_RGUI   = 0x0800,
    KMOD_NUM  = 0x1000, KMOD_CAPS   = 0x2000, KMOD_MODE   = 0x4000,
    KMOD_RESERVED=0x8000

  AVPixelFormat* {.size: sizeof(cint).} = enum
    AV_PIX_FMT_NONE = -1, AV_PIX_FMT_YUV420P, ## /< planar YUV 4:2:0, 12bpp, (1 Cr & Cb sample per 2x2 Y samples)
    AV_PIX_FMT_YUYV422,       ## /< packed YUV 4:2:2, 16bpp, Y0 Cb Y1 Cr
    AV_PIX_FMT_RGB24,         ## /< packed RGB 8:8:8, 24bpp, RGBRGB...
    AV_PIX_FMT_BGR24,         ## /< packed RGB 8:8:8, 24bpp, BGRBGR...
    AV_PIX_FMT_YUV422P,       ## /< planar YUV 4:2:2, 16bpp, (1 Cr & Cb sample per 2x1 Y samples)
    AV_PIX_FMT_YUV444P,       ## /< planar YUV 4:4:4, 24bpp, (1 Cr & Cb sample per 1x1 Y samples)
    AV_PIX_FMT_YUV410P,       ## /< planar YUV 4:1:0,  9bpp, (1 Cr & Cb sample per 4x4 Y samples)
    AV_PIX_FMT_YUV411P,       ## /< planar YUV 4:1:1, 12bpp, (1 Cr & Cb sample per 4x1 Y samples)
    AV_PIX_FMT_GRAY8,         ## /<        Y        ,  8bpp
    AV_PIX_FMT_MONOWHITE,     ## /<        Y        ,  1bpp, 0 is white, 1 is black, in each byte pixels are ordered from the msb to the lsb
    AV_PIX_FMT_MONOBLACK,     ## /<        Y        ,  1bpp, 0 is black, 1 is white, in each byte pixels are ordered from the msb to the lsb
    AV_PIX_FMT_PAL8,          ## /< 8 bits with AV_PIX_FMT_RGB32 palette
    AV_PIX_FMT_YUVJ420P,      ## /< planar YUV 4:2:0, 12bpp, full scale (JPEG), deprecated in favor of AV_PIX_FMT_YUV420P and setting color_range
    AV_PIX_FMT_YUVJ422P,      ## /< planar YUV 4:2:2, 16bpp, full scale (JPEG), deprecated in favor of AV_PIX_FMT_YUV422P and setting color_range
    AV_PIX_FMT_YUVJ444P,      ## /< planar YUV 4:4:4, 24bpp, full scale (JPEG), deprecated in favor of AV_PIX_FMT_YUV444P and setting color_range
    AV_PIX_FMT_UYVY422,       ## /< packed YUV 4:2:2, 16bpp, Cb Y0 Cr Y1
    AV_PIX_FMT_UYYVYY411,     ## /< packed YUV 4:1:1, 12bpp, Cb Y0 Y1 Cr Y2 Y3
    AV_PIX_FMT_BGR8,          ## /< packed RGB 3:3:2,  8bpp, (msb)2B 3G 3R(lsb)
    AV_PIX_FMT_BGR4,          ## /< packed RGB 1:2:1 bitstream,  4bpp, (msb)1B 2G 1R(lsb), a byte contains two pixels, the first pixel in the byte is the one composed by the 4 msb bits
    AV_PIX_FMT_BGR4_BYTE,     ## /< packed RGB 1:2:1,  8bpp, (msb)1B 2G 1R(lsb)
    AV_PIX_FMT_RGB8,          ## /< packed RGB 3:3:2,  8bpp, (msb)2R 3G 3B(lsb)
    AV_PIX_FMT_RGB4,          ## /< packed RGB 1:2:1 bitstream,  4bpp, (msb)1R 2G 1B(lsb), a byte contains two pixels, the first pixel in the byte is the one composed by the 4 msb bits
    AV_PIX_FMT_RGB4_BYTE,     ## /< packed RGB 1:2:1,  8bpp, (msb)1R 2G 1B(lsb)
    AV_PIX_FMT_NV12,          ## /< planar YUV 4:2:0, 12bpp, 1 plane for Y and 1 plane for the UV components, which are interleaved (first byte U and the following byte V)
    AV_PIX_FMT_NV21,          ## /< as above, but U and V bytes are swapped
    AV_PIX_FMT_ARGB,          ## /< packed ARGB 8:8:8:8, 32bpp, ARGBARGB...
    AV_PIX_FMT_RGBA,          ## /< packed RGBA 8:8:8:8, 32bpp, RGBARGBA...
    AV_PIX_FMT_ABGR,          ## /< packed ABGR 8:8:8:8, 32bpp, ABGRABGR...
    AV_PIX_FMT_BGRA,          ## /< packed BGRA 8:8:8:8, 32bpp, BGRABGRA...
    AV_PIX_FMT_GRAY16BE,      ## /<        Y        , 16bpp, big-endian
    AV_PIX_FMT_GRAY16LE,      ## /<        Y        , 16bpp, little-endian
    AV_PIX_FMT_YUV440P,       ## /< planar YUV 4:4:0 (1 Cr & Cb sample per 1x2 Y samples)
    AV_PIX_FMT_YUVJ440P,      ## /< planar YUV 4:4:0 full scale (JPEG), deprecated in favor of AV_PIX_FMT_YUV440P and setting color_range
    AV_PIX_FMT_YUVA420P,      ## /< planar YUV 4:2:0, 20bpp, (1 Cr & Cb sample per 2x2 Y & A samples)
    AV_PIX_FMT_RGB48BE,       ## /< packed RGB 16:16:16, 48bpp, 16R, 16G, 16B, the 2-byte value for each R/G/B component is stored as big-endian
    AV_PIX_FMT_RGB48LE,       ## /< packed RGB 16:16:16, 48bpp, 16R, 16G, 16B, the 2-byte value for each R/G/B component is stored as little-endian
    AV_PIX_FMT_RGB565BE,      ## /< packed RGB 5:6:5, 16bpp, (msb)   5R 6G 5B(lsb), big-endian
    AV_PIX_FMT_RGB565LE,      ## /< packed RGB 5:6:5, 16bpp, (msb)   5R 6G 5B(lsb), little-endian
    AV_PIX_FMT_RGB555BE,      ## /< packed RGB 5:5:5, 16bpp, (msb)1X 5R 5G 5B(lsb), big-endian   , X=unused/undefined
    AV_PIX_FMT_RGB555LE,      ## /< packed RGB 5:5:5, 16bpp, (msb)1X 5R 5G 5B(lsb), little-endian, X=unused/undefined
    AV_PIX_FMT_BGR565BE,      ## /< packed BGR 5:6:5, 16bpp, (msb)   5B 6G 5R(lsb), big-endian
    AV_PIX_FMT_BGR565LE,      ## /< packed BGR 5:6:5, 16bpp, (msb)   5B 6G 5R(lsb), little-endian
    AV_PIX_FMT_BGR555BE,      ## /< packed BGR 5:5:5, 16bpp, (msb)1X 5B 5G 5R(lsb), big-endian   , X=unused/undefined
    AV_PIX_FMT_BGR555LE,      ## /< packed BGR 5:5:5, 16bpp, (msb)1X 5B 5G 5R(lsb), little-endian, X=unused/undefined
    AV_PIX_FMT_VAAPI_MOCO,    ## /< HW acceleration through VA API at motion compensation entry-point, Picture.data[3] contains a vaapi_render_state struct which contains macroblocks as well as various fields extracted from headers
    AV_PIX_FMT_VAAPI_IDCT,    ## /< HW acceleration through VA API at IDCT entry-point, Picture.data[3] contains a vaapi_render_state struct which contains fields extracted from headers
    AV_PIX_FMT_VAAPI_VLD,     ## /< HW decoding through VA API, Picture.data[3] contains a VASurfaceID
    AV_PIX_FMT_YUV420P16LE,   ## /< planar YUV 4:2:0, 24bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
    AV_PIX_FMT_YUV420P16BE,   ## /< planar YUV 4:2:0, 24bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
    AV_PIX_FMT_YUV422P16LE,   ## /< planar YUV 4:2:2, 32bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
    AV_PIX_FMT_YUV422P16BE,   ## /< planar YUV 4:2:2, 32bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
    AV_PIX_FMT_YUV444P16LE,   ## /< planar YUV 4:4:4, 48bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
    AV_PIX_FMT_YUV444P16BE,   ## /< planar YUV 4:4:4, 48bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
    AV_PIX_FMT_DXVA2_VLD,     ## /< HW decoding through DXVA2, Picture.data[3] contains a LPDIRECT3DSURFACE9 pointer
    AV_PIX_FMT_RGB444LE,      ## /< packed RGB 4:4:4, 16bpp, (msb)4X 4R 4G 4B(lsb), little-endian, X=unused/undefined
    AV_PIX_FMT_RGB444BE,      ## /< packed RGB 4:4:4, 16bpp, (msb)4X 4R 4G 4B(lsb), big-endian,    X=unused/undefined
    AV_PIX_FMT_BGR444LE,      ## /< packed BGR 4:4:4, 16bpp, (msb)4X 4B 4G 4R(lsb), little-endian, X=unused/undefined
    AV_PIX_FMT_BGR444BE,      ## /< packed BGR 4:4:4, 16bpp, (msb)4X 4B 4G 4R(lsb), big-endian,    X=unused/undefined
    AV_PIX_FMT_YA8,           ## /< 8 bits gray, 8 bits alpha
    AV_PIX_FMT_BGR48BE,       ## /< packed RGB 16:16:16, 48bpp, 16B, 16G, 16R, the 2-byte value for each R/G/B component is stored as big-endian
    AV_PIX_FMT_BGR48LE,       ## /< packed RGB 16:16:16, 48bpp, 16B, 16G, 16R, the 2-byte value for each R/G/B component is stored as little-endian
    AV_PIX_FMT_YUV420P9BE,    ## /< planar YUV 4:2:0, 13.5bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
    AV_PIX_FMT_YUV420P9LE,    ## /< planar YUV 4:2:0, 13.5bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
    AV_PIX_FMT_YUV420P10BE,   ## /< planar YUV 4:2:0, 15bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
    AV_PIX_FMT_YUV420P10LE,   ## /< planar YUV 4:2:0, 15bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
    AV_PIX_FMT_YUV422P10BE,   ## /< planar YUV 4:2:2, 20bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
    AV_PIX_FMT_YUV422P10LE,   ## /< planar YUV 4:2:2, 20bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
    AV_PIX_FMT_YUV444P9BE,    ## /< planar YUV 4:4:4, 27bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
    AV_PIX_FMT_YUV444P9LE,    ## /< planar YUV 4:4:4, 27bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
    AV_PIX_FMT_YUV444P10BE,   ## /< planar YUV 4:4:4, 30bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
    AV_PIX_FMT_YUV444P10LE,   ## /< planar YUV 4:4:4, 30bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
    AV_PIX_FMT_YUV422P9BE,    ## /< planar YUV 4:2:2, 18bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
    AV_PIX_FMT_YUV422P9LE,    ## /< planar YUV 4:2:2, 18bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
    AV_PIX_FMT_GBRP,          ## /< planar GBR 4:4:4 24bpp
    AV_PIX_FMT_GBRP9BE,       ## /< planar GBR 4:4:4 27bpp, big-endian
    AV_PIX_FMT_GBRP9LE,       ## /< planar GBR 4:4:4 27bpp, little-endian
    AV_PIX_FMT_GBRP10BE,      ## /< planar GBR 4:4:4 30bpp, big-endian
    AV_PIX_FMT_GBRP10LE,      ## /< planar GBR 4:4:4 30bpp, little-endian
    AV_PIX_FMT_GBRP16BE,      ## /< planar GBR 4:4:4 48bpp, big-endian
    AV_PIX_FMT_GBRP16LE,      ## /< planar GBR 4:4:4 48bpp, little-endian
    AV_PIX_FMT_YUVA422P,      ## /< planar YUV 4:2:2 24bpp, (1 Cr & Cb sample per 2x1 Y & A samples)
    AV_PIX_FMT_YUVA444P,      ## /< planar YUV 4:4:4 32bpp, (1 Cr & Cb sample per 1x1 Y & A samples)
    AV_PIX_FMT_YUVA420P9BE,   ## /< planar YUV 4:2:0 22.5bpp, (1 Cr & Cb sample per 2x2 Y & A samples), big-endian
    AV_PIX_FMT_YUVA420P9LE,   ## /< planar YUV 4:2:0 22.5bpp, (1 Cr & Cb sample per 2x2 Y & A samples), little-endian
    AV_PIX_FMT_YUVA422P9BE,   ## /< planar YUV 4:2:2 27bpp, (1 Cr & Cb sample per 2x1 Y & A samples), big-endian
    AV_PIX_FMT_YUVA422P9LE,   ## /< planar YUV 4:2:2 27bpp, (1 Cr & Cb sample per 2x1 Y & A samples), little-endian
    AV_PIX_FMT_YUVA444P9BE,   ## /< planar YUV 4:4:4 36bpp, (1 Cr & Cb sample per 1x1 Y & A samples), big-endian
    AV_PIX_FMT_YUVA444P9LE,   ## /< planar YUV 4:4:4 36bpp, (1 Cr & Cb sample per 1x1 Y & A samples), little-endian
    AV_PIX_FMT_YUVA420P10BE,  ## /< planar YUV 4:2:0 25bpp, (1 Cr & Cb sample per 2x2 Y & A samples, big-endian)
    AV_PIX_FMT_YUVA420P10LE,  ## /< planar YUV 4:2:0 25bpp, (1 Cr & Cb sample per 2x2 Y & A samples, little-endian)
    AV_PIX_FMT_YUVA422P10BE,  ## /< planar YUV 4:2:2 30bpp, (1 Cr & Cb sample per 2x1 Y & A samples, big-endian)
    AV_PIX_FMT_YUVA422P10LE,  ## /< planar YUV 4:2:2 30bpp, (1 Cr & Cb sample per 2x1 Y & A samples, little-endian)
    AV_PIX_FMT_YUVA444P10BE,  ## /< planar YUV 4:4:4 40bpp, (1 Cr & Cb sample per 1x1 Y & A samples, big-endian)
    AV_PIX_FMT_YUVA444P10LE,  ## /< planar YUV 4:4:4 40bpp, (1 Cr & Cb sample per 1x1 Y & A samples, little-endian)
    AV_PIX_FMT_YUVA420P16BE,  ## /< planar YUV 4:2:0 40bpp, (1 Cr & Cb sample per 2x2 Y & A samples, big-endian)
    AV_PIX_FMT_YUVA420P16LE,  ## /< planar YUV 4:2:0 40bpp, (1 Cr & Cb sample per 2x2 Y & A samples, little-endian)
    AV_PIX_FMT_YUVA422P16BE,  ## /< planar YUV 4:2:2 48bpp, (1 Cr & Cb sample per 2x1 Y & A samples, big-endian)
    AV_PIX_FMT_YUVA422P16LE,  ## /< planar YUV 4:2:2 48bpp, (1 Cr & Cb sample per 2x1 Y & A samples, little-endian)
    AV_PIX_FMT_YUVA444P16BE,  ## /< planar YUV 4:4:4 64bpp, (1 Cr & Cb sample per 1x1 Y & A samples, big-endian)
    AV_PIX_FMT_YUVA444P16LE,  ## /< planar YUV 4:4:4 64bpp, (1 Cr & Cb sample per 1x1 Y & A samples, little-endian)
    AV_PIX_FMT_VDPAU,         ## /< HW acceleration through VDPAU, Picture.data[3] contains a VdpVideoSurface
    AV_PIX_FMT_XYZ12LE,       ## /< packed XYZ 4:4:4, 36 bpp, (msb) 12X, 12Y, 12Z (lsb), the 2-byte value for each X/Y/Z is stored as little-endian, the 4 lower bits are set to 0
    AV_PIX_FMT_XYZ12BE,       ## /< packed XYZ 4:4:4, 36 bpp, (msb) 12X, 12Y, 12Z (lsb), the 2-byte value for each X/Y/Z is stored as big-endian, the 4 lower bits are set to 0
    AV_PIX_FMT_NV16,          ## /< interleaved chroma YUV 4:2:2, 16bpp, (1 Cr & Cb sample per 2x1 Y samples)
    AV_PIX_FMT_NV20LE,        ## /< interleaved chroma YUV 4:2:2, 20bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
    AV_PIX_FMT_NV20BE,        ## /< interleaved chroma YUV 4:2:2, 20bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
    AV_PIX_FMT_RGBA64BE,      ## /< packed RGBA 16:16:16:16, 64bpp, 16R, 16G, 16B, 16A, the 2-byte value for each R/G/B/A component is stored as big-endian
    AV_PIX_FMT_RGBA64LE,      ## /< packed RGBA 16:16:16:16, 64bpp, 16R, 16G, 16B, 16A, the 2-byte value for each R/G/B/A component is stored as little-endian
    AV_PIX_FMT_BGRA64BE,      ## /< packed RGBA 16:16:16:16, 64bpp, 16B, 16G, 16R, 16A, the 2-byte value for each R/G/B/A component is stored as big-endian
    AV_PIX_FMT_BGRA64LE,      ## /< packed RGBA 16:16:16:16, 64bpp, 16B, 16G, 16R, 16A, the 2-byte value for each R/G/B/A component is stored as little-endian
    AV_PIX_FMT_YVYU422,       ## /< packed YUV 4:2:2, 16bpp, Y0 Cr Y1 Cb
    AV_PIX_FMT_YA16BE,        ## /< 16 bits gray, 16 bits alpha (big-endian)
    AV_PIX_FMT_YA16LE,        ## /< 16 bits gray, 16 bits alpha (little-endian)
    AV_PIX_FMT_GBRAP,         ## /< planar GBRA 4:4:4:4 32bpp
    AV_PIX_FMT_GBRAP16BE,     ## /< planar GBRA 4:4:4:4 64bpp, big-endian
    AV_PIX_FMT_GBRAP16LE,     ## /< planar GBRA 4:4:4:4 64bpp, little-endian
    AV_PIX_FMT_QSV, AV_PIX_FMT_MMAL, AV_PIX_FMT_D3D11VA_VLD, ## /< HW decoding through Direct3D11 via old API, Picture.data[3] contains a ID3D11VideoDecoderOutputView pointer
    AV_PIX_FMT_CUDA, AV_PIX_FMT_0RGB, ## /< packed RGB 8:8:8, 32bpp, XRGBXRGB...   X=unused/undefined
    AV_PIX_FMT_RGB0,          ## /< packed RGB 8:8:8, 32bpp, RGBXRGBX...   X=unused/undefined
    AV_PIX_FMT_0BGR,          ## /< packed BGR 8:8:8, 32bpp, XBGRXBGR...   X=unused/undefined
    AV_PIX_FMT_BGR0,          ## /< packed BGR 8:8:8, 32bpp, BGRXBGRX...   X=unused/undefined
    AV_PIX_FMT_YUV420P12BE,   ## /< planar YUV 4:2:0,18bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
    AV_PIX_FMT_YUV420P12LE,   ## /< planar YUV 4:2:0,18bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
    AV_PIX_FMT_YUV420P14BE,   ## /< planar YUV 4:2:0,21bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
    AV_PIX_FMT_YUV420P14LE,   ## /< planar YUV 4:2:0,21bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
    AV_PIX_FMT_YUV422P12BE,   ## /< planar YUV 4:2:2,24bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
    AV_PIX_FMT_YUV422P12LE,   ## /< planar YUV 4:2:2,24bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
    AV_PIX_FMT_YUV422P14BE,   ## /< planar YUV 4:2:2,28bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
    AV_PIX_FMT_YUV422P14LE,   ## /< planar YUV 4:2:2,28bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
    AV_PIX_FMT_YUV444P12BE,   ## /< planar YUV 4:4:4,36bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
    AV_PIX_FMT_YUV444P12LE,   ## /< planar YUV 4:4:4,36bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
    AV_PIX_FMT_YUV444P14BE,   ## /< planar YUV 4:4:4,42bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
    AV_PIX_FMT_YUV444P14LE,   ## /< planar YUV 4:4:4,42bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
    AV_PIX_FMT_GBRP12BE,      ## /< planar GBR 4:4:4 36bpp, big-endian
    AV_PIX_FMT_GBRP12LE,      ## /< planar GBR 4:4:4 36bpp, little-endian
    AV_PIX_FMT_GBRP14BE,      ## /< planar GBR 4:4:4 42bpp, big-endian
    AV_PIX_FMT_GBRP14LE,      ## /< planar GBR 4:4:4 42bpp, little-endian
    AV_PIX_FMT_YUVJ411P,      ## /< planar YUV 4:1:1, 12bpp, (1 Cr & Cb sample per 4x1 Y samples) full scale (JPEG), deprecated in favor of AV_PIX_FMT_YUV411P and setting color_range
    AV_PIX_FMT_BAYER_BGGR8,   ## /< bayer, BGBG..(odd line), GRGR..(even line), 8-bit samples
    AV_PIX_FMT_BAYER_RGGB8,   ## /< bayer, RGRG..(odd line), GBGB..(even line), 8-bit samples
    AV_PIX_FMT_BAYER_GBRG8,   ## /< bayer, GBGB..(odd line), RGRG..(even line), 8-bit samples
    AV_PIX_FMT_BAYER_GRBG8,   ## /< bayer, GRGR..(odd line), BGBG..(even line), 8-bit samples
    AV_PIX_FMT_BAYER_BGGR16LE, ## /< bayer, BGBG..(odd line), GRGR..(even line), 16-bit samples, little-endian
    AV_PIX_FMT_BAYER_BGGR16BE, ## /< bayer, BGBG..(odd line), GRGR..(even line), 16-bit samples, big-endian
    AV_PIX_FMT_BAYER_RGGB16LE, ## /< bayer, RGRG..(odd line), GBGB..(even line), 16-bit samples, little-endian
    AV_PIX_FMT_BAYER_RGGB16BE, ## /< bayer, RGRG..(odd line), GBGB..(even line), 16-bit samples, big-endian
    AV_PIX_FMT_BAYER_GBRG16LE, ## /< bayer, GBGB..(odd line), RGRG..(even line), 16-bit samples, little-endian
    AV_PIX_FMT_BAYER_GBRG16BE, ## /< bayer, GBGB..(odd line), RGRG..(even line), 16-bit samples, big-endian
    AV_PIX_FMT_BAYER_GRBG16LE, ## /< bayer, GRGR..(odd line), BGBG..(even line), 16-bit samples, little-endian
    AV_PIX_FMT_BAYER_GRBG16BE, ## /< bayer, GRGR..(odd line), BGBG..(even line), 16-bit samples, big-endian
    AV_PIX_FMT_XVMC,          ## /< XVideo Motion Acceleration via common packet passing
    AV_PIX_FMT_YUV440P10LE,   ## /< planar YUV 4:4:0,20bpp, (1 Cr & Cb sample per 1x2 Y samples), little-endian
    AV_PIX_FMT_YUV440P10BE,   ## /< planar YUV 4:4:0,20bpp, (1 Cr & Cb sample per 1x2 Y samples), big-endian
    AV_PIX_FMT_YUV440P12LE,   ## /< planar YUV 4:4:0,24bpp, (1 Cr & Cb sample per 1x2 Y samples), little-endian
    AV_PIX_FMT_YUV440P12BE,   ## /< planar YUV 4:4:0,24bpp, (1 Cr & Cb sample per 1x2 Y samples), big-endian
    AV_PIX_FMT_AYUV64LE,      ## /< packed AYUV 4:4:4,64bpp (1 Cr & Cb sample per 1x1 Y & A samples), little-endian
    AV_PIX_FMT_AYUV64BE,      ## /< packed AYUV 4:4:4,64bpp (1 Cr & Cb sample per 1x1 Y & A samples), big-endian
    AV_PIX_FMT_VIDEOTOOLBOX,  ## /< hardware decoding through Videotoolbox
    AV_PIX_FMT_P010LE,        ## /< like NV12, with 10bpp per component, data in the high bits, zeros in the low bits, little-endian
    AV_PIX_FMT_P010BE,        ## /< like NV12, with 10bpp per component, data in the high bits, zeros in the low bits, big-endian
    AV_PIX_FMT_GBRAP12BE,     ## /< planar GBR 4:4:4:4 48bpp, big-endian
    AV_PIX_FMT_GBRAP12LE,     ## /< planar GBR 4:4:4:4 48bpp, little-endian
    AV_PIX_FMT_GBRAP10BE,     ## /< planar GBR 4:4:4:4 40bpp, big-endian
    AV_PIX_FMT_GBRAP10LE,     ## /< planar GBR 4:4:4:4 40bpp, little-endian
    AV_PIX_FMT_MEDIACODEC,    ## /< hardware decoding through MediaCodec
    AV_PIX_FMT_GRAY12BE,      ## /<        Y        , 12bpp, big-endian
    AV_PIX_FMT_GRAY12LE,      ## /<        Y        , 12bpp, little-endian
    AV_PIX_FMT_GRAY10BE,      ## /<        Y        , 10bpp, big-endian
    AV_PIX_FMT_GRAY10LE,      ## /<        Y        , 10bpp, little-endian
    AV_PIX_FMT_P016LE,        ## /< like NV12, with 16bpp per component, little-endian
    AV_PIX_FMT_P016BE,        ## /< like NV12, with 16bpp per component, big-endian
    AV_PIX_FMT_D3D11, AV_PIX_FMT_GRAY9BE, ## /<        Y        , 9bpp, big-endian
    AV_PIX_FMT_GRAY9LE,       ## /<        Y        , 9bpp, little-endian
    AV_PIX_FMT_GBRPF32BE,     ## /< IEEE-754 single precision planar GBR 4:4:4,     96bpp, big-endian
    AV_PIX_FMT_GBRPF32LE,     ## /< IEEE-754 single precision planar GBR 4:4:4,     96bpp, little-endian
    AV_PIX_FMT_GBRAPF32BE,    ## /< IEEE-754 single precision planar GBRA 4:4:4:4, 128bpp, big-endian
    AV_PIX_FMT_GBRAPF32LE,    ## /< IEEE-754 single precision planar GBRA 4:4:4:4, 128bpp, little-endian
    AV_PIX_FMT_DRM_PRIME, AV_PIX_FMT_OPENCL, AV_PIX_FMT_GRAY14BE, ## /<        Y        , 14bpp, big-endian
    AV_PIX_FMT_GRAY14LE,      ## /<        Y        , 14bpp, little-endian
    AV_PIX_FMT_GRAYF32BE,     ## /< IEEE-754 single precision Y, 32bpp, big-endian
    AV_PIX_FMT_GRAYF32LE,     ## /< IEEE-754 single precision Y, 32bpp, little-endian
    AV_PIX_FMT_YUVA422P12BE,  ## /< planar YUV 4:2:2,24bpp, (1 Cr & Cb sample per 2x1 Y samples), 12b alpha, big-endian
    AV_PIX_FMT_YUVA422P12LE,  ## /< planar YUV 4:2:2,24bpp, (1 Cr & Cb sample per 2x1 Y samples), 12b alpha, little-endian
    AV_PIX_FMT_YUVA444P12BE,  ## /< planar YUV 4:4:4,36bpp, (1 Cr & Cb sample per 1x1 Y samples), 12b alpha, big-endian
    AV_PIX_FMT_YUVA444P12LE,  ## /< planar YUV 4:4:4,36bpp, (1 Cr & Cb sample per 1x1 Y samples), 12b alpha, little-endian
    AV_PIX_FMT_NV24,          ## /< planar YUV 4:4:4, 24bpp, 1 plane for Y and 1 plane for the UV components, which are interleaved (first byte U and the following byte V)
    AV_PIX_FMT_NV42,          ## /< as above, but U and V bytes are swapped
    AV_PIX_FMT_VULKAN, AV_PIX_FMT_Y210BE, ## /< packed YUV 4:2:2 like YUYV422, 20bpp, data in the high bits, big-endian
    AV_PIX_FMT_Y210LE,        ## /< packed YUV 4:2:2 like YUYV422, 20bpp, data in the high bits, little-endian
    AV_PIX_FMT_X2RGB10LE,     ## /< packed RGB 10:10:10, 30bpp, (msb)2X 10R 10G 10B(lsb), little-endian, X=unused/undefined
    AV_PIX_FMT_X2RGB10BE,     ## /< packed RGB 10:10:10, 30bpp, (msb)2X 10R 10G 10B(lsb), big-endian, X=unused/undefined
    AV_PIX_FMT_NB             ## /< number of pixel formats, DO NOT USE THIS if you want to link with shared libav* because the number of formats might differ between versions

  AVColorPrimaries* {.size: sizeof(cint).} = enum
    AVCOL_PRI_RESERVED0 = 0, AVCOL_PRI_BT709 = 1, ## /< also ITU-R BT1361 / IEC 61966-2-4 / SMPTE RP177 Annex B
    AVCOL_PRI_UNSPECIFIED = 2, AVCOL_PRI_RESERVED = 3, AVCOL_PRI_BT470M = 4, ## /< also FCC Title 47 Code of Federal Regulations 73.682 (a)(20)
    AVCOL_PRI_BT470BG = 5,      ## /< also ITU-R BT601-6 625 / ITU-R BT1358 625 / ITU-R BT1700 625 PAL & SECAM
    AVCOL_PRI_SMPTE170M = 6,    ## /< also ITU-R BT601-6 525 / ITU-R BT1358 525 / ITU-R BT1700 NTSC
    AVCOL_PRI_SMPTE240M = 7,    ## /< functionally identical to above
    AVCOL_PRI_FILM = 8,         ## /< colour filters using Illuminant C
    AVCOL_PRI_BT2020 = 9,       ## /< ITU-R BT2020
    AVCOL_PRI_SMPTE428 = 10,    ## /< SMPTE ST 428-1 (CIE 1931 XYZ)
    AVCOL_PRI_SMPTE431 = 11,    ## /< SMPTE ST 431-2 (2011) / DCI P3
    AVCOL_PRI_SMPTE432 = 12,    ## /< SMPTE ST 432-1 (2010) / P3 D65 / Display P3
    AVCOL_PRI_EBU3213 = 22,     ## /< EBU Tech. 3213-E / JEDEC P22 phosphors
    AVCOL_PRI_NB              ## /< Not part of ABI
  AVColorTransferCharacteristic* {.size: sizeof(cint).} = enum
    AVCOL_TRC_RESERVED0 = 0, AVCOL_TRC_BT709 = 1, ## /< also ITU-R BT1361
    AVCOL_TRC_UNSPECIFIED = 2, AVCOL_TRC_RESERVED = 3, AVCOL_TRC_GAMMA22 = 4, ## /< also ITU-R BT470M / ITU-R BT1700 625 PAL & SECAM
    AVCOL_TRC_GAMMA28 = 5,      ## /< also ITU-R BT470BG
    AVCOL_TRC_SMPTE170M = 6,    ## /< also ITU-R BT601-6 525 or 625 / ITU-R BT1358 525 or 625 / ITU-R BT1700 NTSC
    AVCOL_TRC_SMPTE240M = 7, AVCOL_TRC_LINEAR = 8, ## /< "Linear transfer characteristics"
    AVCOL_TRC_LOG = 9,          ## /< "Logarithmic transfer characteristic (100:1 range)"
    AVCOL_TRC_LOG_SQRT = 10,    ## /< "Logarithmic transfer characteristic (100 * Sqrt(10) : 1 range)"
    AVCOL_TRC_IEC61966_2_4 = 11, ## /< IEC 61966-2-4
    AVCOL_TRC_BT1361_ECG = 12,  ## /< ITU-R BT1361 Extended Colour Gamut
    AVCOL_TRC_IEC61966_2_1 = 13, ## /< IEC 61966-2-1 (sRGB or sYCC)
    AVCOL_TRC_BT2020_10 = 14,   ## /< ITU-R BT2020 for 10-bit system
    AVCOL_TRC_BT2020_12 = 15,   ## /< ITU-R BT2020 for 12-bit system
    AVCOL_TRC_SMPTE2084 = 16,   ## /< SMPTE ST 2084 for 10-, 12-, 14- and 16-bit systems
    AVCOL_TRC_SMPTE428 = 17,    ## /< SMPTE ST 428-1
    AVCOL_TRC_ARIB_STD_B67 = 18, ## /< ARIB STD-B67, known as "Hybrid log-gamma"
    AVCOL_TRC_NB              ## /< Not part of ABI
  AVColorSpace* {.size: sizeof(cint).} = enum
    AVCOL_SPC_RGB = 0,          ## /< order of coefficients is actually GBR, also IEC 61966-2-1 (sRGB)
    AVCOL_SPC_BT709 = 1,        ## /< also ITU-R BT1361 / IEC 61966-2-4 xvYCC709 / SMPTE RP177 Annex B
    AVCOL_SPC_UNSPECIFIED = 2, AVCOL_SPC_RESERVED = 3, AVCOL_SPC_FCC = 4, ## /< FCC Title 47 Code of Federal Regulations 73.682 (a)(20)
    AVCOL_SPC_BT470BG = 5,      ## /< also ITU-R BT601-6 625 / ITU-R BT1358 625 / ITU-R BT1700 625 PAL & SECAM / IEC 61966-2-4 xvYCC601
    AVCOL_SPC_SMPTE170M = 6,    ## /< also ITU-R BT601-6 525 / ITU-R BT1358 525 / ITU-R BT1700 NTSC
    AVCOL_SPC_SMPTE240M = 7,    ## /< functionally identical to above
    AVCOL_SPC_YCGCO = 8,        ## /< Used by Dirac / VC-2 and H.264 FRext, see ITU-T SG16
    AVCOL_SPC_BT2020_NCL = 9,   ## /< ITU-R BT2020 non-constant luminance system
    AVCOL_SPC_BT2020_CL = 10,   ## /< ITU-R BT2020 constant luminance system
    AVCOL_SPC_SMPTE2085 = 11,   ## /< SMPTE 2085, Y'D'zD'x
    AVCOL_SPC_CHROMA_DERIVED_NCL = 12, ## /< Chromaticity-derived non-constant luminance system
    AVCOL_SPC_CHROMA_DERIVED_CL = 13, ## /< Chromaticity-derived constant luminance system
    AVCOL_SPC_ICTCP = 14,       ## /< ITU-R BT.2100-0, ICtCp
    AVCOL_SPC_NB              ## /< Not part of ABI
  AVColorRange* {.size: sizeof(cint).} = enum
    AVCOL_RANGE_UNSPECIFIED = 0, AVCOL_RANGE_MPEG = 1, AVCOL_RANGE_JPEG = 2, AVCOL_RANGE_NB ## /< Not part of ABI


  AVChromaLocation* {.size: sizeof(cint).} = enum
    AVCHROMA_LOC_UNSPECIFIED = 0, AVCHROMA_LOC_LEFT = 1, ## /< MPEG-2/4 4:2:0, H.264 default for 4:2:0
    AVCHROMA_LOC_CENTER = 2,    ## /< MPEG-1 4:2:0, JPEG 4:2:0, H.263 4:2:0
    AVCHROMA_LOC_TOPLEFT = 3,   ## /< ITU-R 601, SMPTE 274M 296M S314M(DV 4:1:1), mpeg2 4:2:2
    AVCHROMA_LOC_TOP = 4, AVCHROMA_LOC_BOTTOMLEFT = 5, AVCHROMA_LOC_BOTTOM = 6, AVCHROMA_LOC_NB ## /< Not part of ABI

  AVClassCategory* {.size: sizeof(cint).} = enum
    AV_CLASS_CATEGORY_NA = 0, AV_CLASS_CATEGORY_INPUT, AV_CLASS_CATEGORY_OUTPUT,
    AV_CLASS_CATEGORY_MUXER, AV_CLASS_CATEGORY_DEMUXER, AV_CLASS_CATEGORY_ENCODER,
    AV_CLASS_CATEGORY_DECODER, AV_CLASS_CATEGORY_FILTER,
    AV_CLASS_CATEGORY_BITSTREAM_FILTER, AV_CLASS_CATEGORY_SWSCALER,
    AV_CLASS_CATEGORY_SWRESAMPLER, AV_CLASS_CATEGORY_DEVICE_VIDEO_OUTPUT = 40,
    AV_CLASS_CATEGORY_DEVICE_VIDEO_INPUT, AV_CLASS_CATEGORY_DEVICE_AUDIO_OUTPUT,
    AV_CLASS_CATEGORY_DEVICE_AUDIO_INPUT, AV_CLASS_CATEGORY_DEVICE_OUTPUT,
    AV_CLASS_CATEGORY_DEVICE_INPUT, AV_CLASS_CATEGORY_NB ## /< not part of ABI/API

const
    SDL_PIXELFORMAT_UNKNOWN = 0
    SDL_PIXELFORMAT_INDEX1LSB = ((1 shl 28) or ((SDL_PIXELTYPE_INDEX1).int shl 24) or ((SDL_BITMAPORDER_4321).int shl 20) or ((0) shl 16) or ((1) shl 8) or ((0) shl 0))
    SDL_PIXELFORMAT_INDEX1MSB = ((1 shl 28) or ((SDL_PIXELTYPE_INDEX1).int shl 24) or ((SDL_BITMAPORDER_1234).int shl 20) or ((0) shl 16) or ((1) shl 8) or ((0) shl 0))
    SDL_PIXELFORMAT_INDEX4LSB = ((1 shl 28) or ((SDL_PIXELTYPE_INDEX4).int shl 24) or ((SDL_BITMAPORDER_4321).int shl 20) or ((0) shl 16) or ((4) shl 8) or ((0) shl 0)) 
    SDL_PIXELFORMAT_INDEX4MSB = ((1 shl 28) or ((SDL_PIXELTYPE_INDEX4).int shl 24) or ((SDL_BITMAPORDER_1234).int shl 20) or ((0) shl 16) or ((4) shl 8) or ((0) shl 0)) 
    SDL_PIXELFORMAT_INDEX8 = ((1 shl 28) or ((SDL_PIXELTYPE_INDEX8).int shl 24) or ((0) shl 20) or ((0) shl 16) or ((8) shl 8) or ((1) shl 0)) 
    SDL_PIXELFORMAT_RGB332 = ((1 shl 28) or ((SDL_PIXELTYPE_PACKED8).int shl 24) or ((SDL_PACKEDORDER_XRGB).int shl 20) or ((SDL_PACKEDLAYOUT_332).int shl 16) or ((8) shl 8) or ((1) shl 0))
    SDL_PIXELFORMAT_RGB444 = ((1 shl 28) or ((SDL_PIXELTYPE_PACKED16).int shl 24) or ((SDL_PACKEDORDER_XRGB).int shl 20) or ((SDL_PACKEDLAYOUT_4444).int shl 16) or ((12) shl 8) or ((2) shl 0)) 
    SDL_PIXELFORMAT_BGR444 = ((1 shl 28) or ((SDL_PIXELTYPE_PACKED16).int shl 24) or ((SDL_PACKEDORDER_XBGR).int shl 20) or ((SDL_PACKEDLAYOUT_4444).int shl 16) or ((12) shl 8) or ((2) shl 0)) 
    SDL_PIXELFORMAT_RGB555 = ((1 shl 28) or ((SDL_PIXELTYPE_PACKED16).int shl 24) or ((SDL_PACKEDORDER_XRGB).int shl 20) or ((SDL_PACKEDLAYOUT_1555).int shl 16) or ((15) shl 8) or ((2) shl 0)) 
    SDL_PIXELFORMAT_BGR555 = ((1 shl 28) or ((SDL_PIXELTYPE_PACKED16).int shl 24) or ((SDL_PACKEDORDER_XBGR).int shl 20) or ((SDL_PACKEDLAYOUT_1555).int shl 16) or ((15) shl 8) or ((2) shl 0)) 
    SDL_PIXELFORMAT_ARGB4444 = ((1 shl 28) or ((SDL_PIXELTYPE_PACKED16).int shl 24) or ((SDL_PACKEDORDER_ARGB).int shl 20) or ((SDL_PACKEDLAYOUT_4444).int shl 16) or ((16) shl 8) or ((2) shl 0))
    SDL_PIXELFORMAT_RGBA4444 = ((1 shl 28) or ((SDL_PIXELTYPE_PACKED16).int shl 24) or ((SDL_PACKEDORDER_RGBA).int shl 20) or ((SDL_PACKEDLAYOUT_4444).int shl 16) or ((16) shl 8) or ((2) shl 0)) 
    SDL_PIXELFORMAT_ABGR4444 = ((1 shl 28) or ((SDL_PIXELTYPE_PACKED16).int shl 24) or ((SDL_PACKEDORDER_ABGR).int shl 20) or ((SDL_PACKEDLAYOUT_4444).int shl 16) or ((16) shl 8) or ((2) shl 0)) 
    SDL_PIXELFORMAT_BGRA4444 = ((1 shl 28) or ((SDL_PIXELTYPE_PACKED16).int shl 24) or ((SDL_PACKEDORDER_BGRA).int shl 20) or ((SDL_PACKEDLAYOUT_4444).int shl 16) or ((16) shl 8) or ((2) shl 0)) 
    SDL_PIXELFORMAT_ARGB1555 = ((1 shl 28) or ((SDL_PIXELTYPE_PACKED16).int shl 24) or ((SDL_PACKEDORDER_ARGB).int shl 20) or ((SDL_PACKEDLAYOUT_1555).int shl 16) or ((16) shl 8) or ((2) shl 0)) 
    SDL_PIXELFORMAT_RGBA5551 = ((1 shl 28) or ((SDL_PIXELTYPE_PACKED16).int shl 24) or ((SDL_PACKEDORDER_RGBA).int shl 20) or ((SDL_PACKEDLAYOUT_5551).int shl 16) or ((16) shl 8) or ((2) shl 0)) 
    SDL_PIXELFORMAT_ABGR1555 = ((1 shl 28) or ((SDL_PIXELTYPE_PACKED16).int shl 24) or ((SDL_PACKEDORDER_ABGR).int shl 20) or ((SDL_PACKEDLAYOUT_1555).int shl 16) or ((16) shl 8) or ((2) shl 0)) 
    SDL_PIXELFORMAT_BGRA5551 = ((1 shl 28) or ((SDL_PIXELTYPE_PACKED16).int shl 24) or ((SDL_PACKEDORDER_BGRA).int shl 20) or ((SDL_PACKEDLAYOUT_5551).int shl 16) or ((16) shl 8) or ((2) shl 0)) 
    SDL_PIXELFORMAT_RGB565 = ((1 shl 28) or ((SDL_PIXELTYPE_PACKED16).int shl 24) or ((SDL_PACKEDORDER_XRGB).int shl 20) or ((SDL_PACKEDLAYOUT_565).int shl 16) or ((16) shl 8) or ((2) shl 0)) 
    SDL_PIXELFORMAT_BGR565 = ((1 shl 28) or ((SDL_PIXELTYPE_PACKED16).int shl 24) or ((SDL_PACKEDORDER_XBGR).int shl 20) or ((SDL_PACKEDLAYOUT_565).int shl 16) or ((16) shl 8) or ((2) shl 0))
    SDL_PIXELFORMAT_RGB24 = ((1 shl 28) or ((SDL_PIXELTYPE_ARRAYU8).int shl 24) or ((SDL_ARRAYORDER_RGB).int shl 20) or ((0) shl 16) or ((24) shl 8) or ((3) shl 0))
    SDL_PIXELFORMAT_BGR24 = ((1 shl 28) or ((SDL_PIXELTYPE_ARRAYU8).int shl 24) or ((SDL_ARRAYORDER_BGR).int shl 20) or ((0) shl 16) or ((24) shl 8) or ((3) shl 0))
    SDL_PIXELFORMAT_RGB888 = (( 1 shl 28) or ((SDL_PIXELTYPE_PACKED32).int shl 24) or ((SDL_PACKEDORDER_XRGB).int shl 20) or ((SDL_PACKEDLAYOUT_8888).int shl 16) or ((24) shl 8) or ((4) shl 0))
    SDL_PIXELFORMAT_RGBX8888 = ((1 shl 28) or ((SDL_PIXELTYPE_PACKED32).int shl 24) or ((SDL_PACKEDORDER_RGBX).int shl 20) or ((SDL_PACKEDLAYOUT_8888).int shl 16) or ((24) shl 8) or ((4) shl 0))
    SDL_PIXELFORMAT_BGR888 = ((1 shl 28) or ((SDL_PIXELTYPE_PACKED32).int shl 24) or ((SDL_PACKEDORDER_XBGR).int shl 20) or ((SDL_PACKEDLAYOUT_8888).int shl 16) or ((24) shl 8) or ((4) shl 0)) 
    SDL_PIXELFORMAT_BGRX8888 = ((1 shl 28) or ((SDL_PIXELTYPE_PACKED32).int shl 24) or ((SDL_PACKEDORDER_BGRX).int shl 20) or ((SDL_PACKEDLAYOUT_8888).int shl 16) or ((24) shl 8) or ((4) shl 0)) 
    SDL_PIXELFORMAT_ARGB8888 = ((1 shl 28) or ((SDL_PIXELTYPE_PACKED32).int shl 24) or ((SDL_PACKEDORDER_ARGB).int shl 20) or ((SDL_PACKEDLAYOUT_8888).int shl 16) or ((32) shl 8) or ((4) shl 0)) 
    SDL_PIXELFORMAT_RGBA8888 = ((1 shl 28) or ((SDL_PIXELTYPE_PACKED32).int shl 24) or ((SDL_PACKEDORDER_RGBA).int shl 20) or ((SDL_PACKEDLAYOUT_8888).int shl 16) or ((32) shl 8) or ((4) shl 0)) 
    SDL_PIXELFORMAT_ABGR8888 = ((1 shl 28) or ((SDL_PIXELTYPE_PACKED32).int shl 24) or ((SDL_PACKEDORDER_ABGR).int shl 20) or ((SDL_PACKEDLAYOUT_8888).int shl 16) or ((32) shl 8) or ((4) shl 0)) 
    SDL_PIXELFORMAT_BGRA8888 = ((1 shl 28) or ((SDL_PIXELTYPE_PACKED32).int shl 24) or ((SDL_PACKEDORDER_BGRA).int shl 20) or ((SDL_PACKEDLAYOUT_8888).int shl 16) or ((32) shl 8) or ((4) shl 0)) 
    SDL_PIXELFORMAT_ARGB2101010 = (( 1 shl 28) or ((SDL_PIXELTYPE_PACKED32).int shl 24) or ((SDL_PACKEDORDER_ARGB).int shl 20) or ((SDL_PACKEDLAYOUT_2101010).int shl 16) or ((32) shl 8) or ((4) shl 0))
    SDL_PIXELFORMAT_YV12 = ((((uint32)(((uint8)(('Y'))))) shl 0) or (((uint32)(((uint8)(('V'))))) shl 8) or (((uint32)(((uint8)(('1'))))) shl 16) or (((uint32)(((uint8)(('2'))))) shl 24))
    SDL_PIXELFORMAT_IYUV = ((((uint32)(((uint8)(('I'))))) shl 0) or (((uint32)(((uint8)(('Y'))))) shl 8) or (((uint32)(((uint8)(('U'))))) shl 16) or (((uint32)(((uint8)(('V'))))) shl 24))
    SDL_PIXELFORMAT_YUY2 = ((((uint32)(((uint8)(('Y'))))) shl 0) or (((uint32)(((uint8)(('U'))))) shl 8) or (((uint32)(((uint8)(('Y'))))) shl 16) or (((uint32)(((uint8)(('2'))))) shl 24))
    SDL_PIXELFORMAT_UYVY = ((((uint32)(((uint8)(('U'))))) shl 0) or (((uint32)(((uint8)(('Y'))))) shl 8) or (((uint32)(((uint8)(('V'))))) shl 16) or (((uint32)(((uint8)(('Y'))))) shl 24)) 
    SDL_PIXELFORMAT_YVYU = ((((uint32)(((uint8)(('Y'))))) shl 0) or (((uint32)(((uint8)(('V'))))) shl 8) or (((uint32)(((uint8)(('Y'))))) shl 16) or (((uint32)(((uint8)(('U'))))) shl 24)) 
    SDL_PIXELFORMAT_NV12 = ((((uint32)(((uint8)(('N'))))) shl 0) or (((uint32)(((uint8)(('V'))))) shl 8) or (((uint32)(((uint8)(('1'))))) shl 16) or (((uint32)(((uint8)(('2'))))) shl 24)) 
    SDL_PIXELFORMAT_NV21 = ((((uint32)(((uint8)(('N'))))) shl 0) or (((uint32)(((uint8)(('V'))))) shl 8) or (((uint32)(((uint8)(('2'))))) shl 16) or (((uint32)(((uint8)(('1'))))) shl 24)) 
    SDL_PIXELFORMAT_EXTERNAL_OES = ((((uint32)(((uint8)(('O'))))) shl 0) or (((uint32)(((uint8)(('E'))))) shl 8) or (((uint32)(((uint8)(('S'))))) shl 16) or (((uint32)(((uint8)((' '))))) shl 24))

template SDL_SCANCODE_TO_KEYCODE(some: SDL_ScanCode): untyped = (some.cint or K_SCANCODE_MASK)

proc av_get_picture_type_char*(pict_type: AVPictureType): char


template AV_NE*(be, le: untyped): untyped =
  (le)

template RSHIFT*(a, b: untyped): untyped =
  (if (a) > 0: ((a) + ((1 shl (b)) shr 1)) shr (b) else: ((a) + ((1 shl (b)) shr 1) - 1) shr (b))

template ROUNDED_DIV*(a, b: untyped): untyped =
  ((if (a) >= 0: (a) + ((b) shr 1) else: (a) - ((b) shr 1)) div (b))

template AV_CEIL_RSHIFT*(a, b: untyped): untyped =
  (if not av_builtin_constant_p(b): -((-(a)) shr (b)) else: ((a) + (1 shl (b)) - 1) shr
      (b))


template FFUDIV*(a, b: untyped): untyped =
  ((if (a) > 0: (a) else: (a) - (b) + 1) div (b))

template FFUMOD*(a, b: untyped): untyped =
  ((a) - (b) * FFUDIV(a, b))

template FFABS*(a: untyped): untyped =
  (if (a) >= 0: (a) else: (-(a)))

template FFSIGN*(a: untyped): untyped =
  (if (a) > 0: 1 else: -1)

template FFNABS*(a: untyped): untyped =
  (if (a) <= 0: (a) else: (-(a)))

template FFDIFFSIGN*(x, y: untyped): untyped =
  (((x) > (y)) - ((x) < (y)))

template FFMAX*(a, b: untyped): untyped =
  (if (a) > (b): (a) else: (b))

template FFMAX3*(a, b, c: untyped): untyped =
  FFMAX(FFMAX(a, b), c)

template FFMIN*(a, b: untyped): untyped =
  (if (a) > (b): (b) else: (a))

template FFMIN3*(a, b, c: untyped): untyped =
  FFMIN(FFMIN(a, b), c)

template FFSWAP*(`type`, a, b: untyped): void =
  while true:
    var temp: `type` = b
    b = a
    a = temp
    if not 0:
      break


const
  AV_HAVE_BIGENDIAN* = 0
  AV_HAVE_FAST_UNALIGNED* = 1
  AVERROR_BSF_NOT_FOUND* = FFERRTAG(0x000000F8, 'B', 'S', 'F')
  AVERROR_BUG* = FFERRTAG('B', 'U', 'G', '!')
  AVERROR_BUFFER_TOO_SMALL* = FFERRTAG('B', 'U', 'F', 'S')
  AVERROR_DECODER_NOT_FOUND* = FFERRTAG(0x000000F8, 'D', 'E', 'C')
  AVERROR_DEMUXER_NOT_FOUND* = FFERRTAG(0x000000F8, 'D', 'E', 'M')
  AVERROR_ENCODER_NOT_FOUND* = FFERRTAG(0x000000F8, 'E', 'N', 'C')
  AVERROR_EOF* = FFERRTAG('E', 'O', 'F', ' ')
  AVERROR_EXIT* = FFERRTAG('E', 'X', 'I', 'T')
  AVERROR_EXTERNAL* = FFERRTAG('E', 'X', 'T', ' ')
  AVERROR_FILTER_NOT_FOUND* = FFERRTAG(0x000000F8, 'F', 'I', 'L')
  AVERROR_INVALIDDATA* = FFERRTAG('I', 'N', 'D', 'A')
  AVERROR_MUXER_NOT_FOUND* = FFERRTAG(0x000000F8, 'M', 'U', 'X')
  AVERROR_OPTION_NOT_FOUND* = FFERRTAG(0x000000F8, 'O', 'P', 'T')
  AVERROR_PATCHWELCOME* = FFERRTAG('P', 'A', 'W', 'E')
  AVERROR_PROTOCOL_NOT_FOUND* = FFERRTAG(0x000000F8, 'P', 'R', 'O')
  AVERROR_STREAM_NOT_FOUND* = FFERRTAG(0x000000F8, 'S', 'T', 'R')
  AVERROR_BUG2* = FFERRTAG('B', 'U', 'G', ' ')
  AVERROR_UNKNOWN* = FFERRTAG('U', 'N', 'K', 'N')
  AVERROR_EXPERIMENTAL* = (-0x2BB2AFA8)
  AVERROR_INPUT_CHANGED* = (-0x636E6701)
  AVERROR_OUTPUT_CHANGED* = (-0x636E6702)
  AVERROR_HTTP_BAD_REQUEST* = FFERRTAG(0x000000F8, '4', '0', '0')
  AVERROR_HTTP_UNAUTHORIZED* = FFERRTAG(0x000000F8, '4', '0', '1')
  AVERROR_HTTP_FORBIDDEN* = FFERRTAG(0x000000F8, '4', '0', '3')
  AVERROR_HTTP_NOT_FOUND* = FFERRTAG(0x000000F8, '4', '0', '4')
  AVERROR_HTTP_OTHER_4XX* = FFERRTAG(0x000000F8, '4', 'X', 'X')
  AVERROR_HTTP_SERVER_ERROR* = FFERRTAG(0x000000F8, '5', 'X', 'X')
  AV_ERROR_MAX_STRING_SIZE* = 64
  AVFORMAT_AVFORMAT_H* = true
  AV_BUFFER_FLAG_READONLY* = (1 shl 0)
  AV_CPU_FLAG_FORCE* = 0x80000000
  AV_CPU_FLAG_MMX* = 0x00000001
  AV_CPU_FLAG_MMXEXT* = 0x00000002
  AV_CPU_FLAG_MMX2* = 0x00000002
  AV_CPU_FLAG_3DNOW* = 0x00000004
  AV_CPU_FLAG_SSE* = 0x00000008
  AV_CPU_FLAG_SSE2* = 0x00000010
  AV_CPU_FLAG_SSE2SLOW* = 0x40000000
  AV_CPU_FLAG_3DNOWEXT* = 0x00000020
  AV_CPU_FLAG_SSE3* = 0x00000040
  AV_CPU_FLAG_SSE3SLOW* = 0x20000000
  AV_CPU_FLAG_SSSE3* = 0x00000080
  AV_CPU_FLAG_SSSE3SLOW* = 0x04000000
  AV_CPU_FLAG_ATOM* = 0x10000000
  AV_CPU_FLAG_SSE4* = 0x00000100
  AV_CPU_FLAG_SSE42* = 0x00000200
  AV_CPU_FLAG_AESNI* = 0x00080000
  AV_CPU_FLAG_AVX* = 0x00004000
  AV_CPU_FLAG_AVXSLOW* = 0x08000000
  AV_CPU_FLAG_XOP* = 0x00000400
  AV_CPU_FLAG_FMA4* = 0x00000800
  AV_CPU_FLAG_CMOV* = 0x00001000
  AV_CPU_FLAG_AVX2* = 0x00008000
  AV_CPU_FLAG_FMA3* = 0x00010000
  AV_CPU_FLAG_BMI1* = 0x00020000
  AV_CPU_FLAG_BMI2* = 0x00040000
  AV_CPU_FLAG_AVX512* = 0x00100000
  AV_CPU_FLAG_ALTIVEC* = 0x00000001
  AV_CPU_FLAG_VSX* = 0x00000002
  AV_CPU_FLAG_POWER8* = 0x00000004
  AV_CPU_FLAG_ARMV5TE* = (1 shl 0)
  AV_CPU_FLAG_ARMV6* = (1 shl 1)
  AV_CPU_FLAG_ARMV6T2* = (1 shl 2)
  AV_CPU_FLAG_VFP* = (1 shl 3)
  AV_CPU_FLAG_VFPV3* = (1 shl 4)
  AV_CPU_FLAG_NEON* = (1 shl 5)
  AV_CPU_FLAG_ARMV8* = (1 shl 6)
  AV_CPU_FLAG_VFP_VM* = (1 shl 7)
  AV_CPU_FLAG_SETEND* = (1 shl 16)
  AV_CPU_FLAG_MMI* = (1 shl 0)
  AV_CPU_FLAG_MSA* = (1 shl 1)
  AVUTIL_CHANNEL_LAYOUT_H* = true
  AV_CH_FRONT_LEFT* = 0x00000001
  AV_CH_FRONT_RIGHT* = 0x00000002
  AV_CH_FRONT_CENTER* = 0x00000004
  AV_CH_LOW_FREQUENCY* = 0x00000008
  AV_CH_BACK_LEFT* = 0x00000010
  AV_CH_BACK_RIGHT* = 0x00000020
  AV_CH_FRONT_LEFT_OF_CENTER* = 0x00000040
  AV_CH_FRONT_RIGHT_OF_CENTER* = 0x00000080
  AV_CH_BACK_CENTER* = 0x00000100
  AV_CH_SIDE_LEFT* = 0x00000200
  AV_CH_SIDE_RIGHT* = 0x00000400
  AV_CH_TOP_CENTER* = 0x00000800
  AV_CH_TOP_FRONT_LEFT* = 0x00001000
  AV_CH_TOP_FRONT_CENTER* = 0x00002000
  AV_CH_TOP_FRONT_RIGHT* = 0x00004000
  AV_CH_TOP_BACK_LEFT* = 0x00008000
  AV_CH_TOP_BACK_CENTER* = 0x00010000
  AV_CH_TOP_BACK_RIGHT* = 0x00020000
  AV_CH_STEREO_LEFT* = 0x20000000
  AV_CH_STEREO_RIGHT* = 0x40000000
  AV_CH_WIDE_LEFT* = 0x0000000000000000'i64
  AV_CH_WIDE_RIGHT* = 0x0000000000000000'i64
  AV_CH_SURROUND_DIRECT_LEFT* = 0x0000000000000000'i64
  AV_CH_SURROUND_DIRECT_RIGHT* = 0x0000000000000000'i64
  AV_CH_LOW_FREQUENCY_2* = 0x0000000000000000'i64
  AV_CH_TOP_SIDE_LEFT* = 0x0000000000000000'i64
  AV_CH_TOP_SIDE_RIGHT* = 0x0000000000000000'i64
  AV_CH_BOTTOM_FRONT_CENTER* = 0x0000000000000000'i64
  AV_CH_BOTTOM_FRONT_LEFT* = 0x0000000000000000'i64
  AV_CH_BOTTOM_FRONT_RIGHT* = 0x0000000000000000'i64
  AV_CH_LAYOUT_NATIVE* = 0x0000000000000000'i64
  AV_CH_LAYOUT_MONO* = (AV_CH_FRONT_CENTER)
  AV_CH_LAYOUT_STEREO* = (AV_CH_FRONT_LEFT or AV_CH_FRONT_RIGHT)
  AV_CH_LAYOUT_2POINT1* = (AV_CH_LAYOUT_STEREO or AV_CH_LOW_FREQUENCY)
  AV_CH_LAYOUT_2_1* = (AV_CH_LAYOUT_STEREO or AV_CH_BACK_CENTER)
  AV_CH_LAYOUT_SURROUND* = (AV_CH_LAYOUT_STEREO or AV_CH_FRONT_CENTER)
  AV_CH_LAYOUT_3POINT1* = (AV_CH_LAYOUT_SURROUND or AV_CH_LOW_FREQUENCY)
  AV_CH_LAYOUT_4POINT0* = (AV_CH_LAYOUT_SURROUND or AV_CH_BACK_CENTER)
  AV_CH_LAYOUT_4POINT1* = (AV_CH_LAYOUT_4POINT0 or AV_CH_LOW_FREQUENCY)
  AV_CH_LAYOUT_2_2* = (AV_CH_LAYOUT_STEREO or AV_CH_SIDE_LEFT or AV_CH_SIDE_RIGHT)
  AV_CH_LAYOUT_QUAD* = (AV_CH_LAYOUT_STEREO or AV_CH_BACK_LEFT or AV_CH_BACK_RIGHT)
  AV_CH_LAYOUT_5POINT0* = (AV_CH_LAYOUT_SURROUND or AV_CH_SIDE_LEFT or AV_CH_SIDE_RIGHT)
  AV_CH_LAYOUT_5POINT1* = (AV_CH_LAYOUT_5POINT0 or AV_CH_LOW_FREQUENCY)
  AV_CH_LAYOUT_5POINT0_BACK* = (AV_CH_LAYOUT_SURROUND or AV_CH_BACK_LEFT or AV_CH_BACK_RIGHT)
  AV_CH_LAYOUT_5POINT1_BACK* = (AV_CH_LAYOUT_5POINT0_BACK or AV_CH_LOW_FREQUENCY)
  AV_CH_LAYOUT_6POINT0* = (AV_CH_LAYOUT_5POINT0 or AV_CH_BACK_CENTER)
  AV_CH_LAYOUT_6POINT0_FRONT* = (AV_CH_LAYOUT_2_2 or AV_CH_FRONT_LEFT_OF_CENTER or AV_CH_FRONT_RIGHT_OF_CENTER)
  AV_CH_LAYOUT_HEXAGONAL* = (AV_CH_LAYOUT_5POINT0_BACK or AV_CH_BACK_CENTER)
  AV_CH_LAYOUT_6POINT1* = (AV_CH_LAYOUT_5POINT1 or AV_CH_BACK_CENTER)
  AV_CH_LAYOUT_6POINT1_BACK* = (AV_CH_LAYOUT_5POINT1_BACK or AV_CH_BACK_CENTER)
  AV_CH_LAYOUT_6POINT1_FRONT* = (AV_CH_LAYOUT_6POINT0_FRONT or AV_CH_LOW_FREQUENCY)
  AV_CH_LAYOUT_7POINT0* = (AV_CH_LAYOUT_5POINT0 or AV_CH_BACK_LEFT or AV_CH_BACK_RIGHT)
  AV_CH_LAYOUT_7POINT0_FRONT* = (AV_CH_LAYOUT_5POINT0 or AV_CH_FRONT_LEFT_OF_CENTER or AV_CH_FRONT_RIGHT_OF_CENTER)
  AV_CH_LAYOUT_7POINT1* = (AV_CH_LAYOUT_5POINT1 or AV_CH_BACK_LEFT or AV_CH_BACK_RIGHT)
  AV_CH_LAYOUT_7POINT1_WIDE* = (AV_CH_LAYOUT_5POINT1 or AV_CH_FRONT_LEFT_OF_CENTER or AV_CH_FRONT_RIGHT_OF_CENTER)
  AV_CH_LAYOUT_7POINT1_WIDE_BACK* = (AV_CH_LAYOUT_5POINT1_BACK or AV_CH_FRONT_LEFT_OF_CENTER or AV_CH_FRONT_RIGHT_OF_CENTER)
  AV_CH_LAYOUT_OCTAGONAL* = (AV_CH_LAYOUT_5POINT0 or AV_CH_BACK_LEFT or AV_CH_BACK_CENTER or AV_CH_BACK_RIGHT)
  AV_CH_LAYOUT_HEXADECAGONAL* = (AV_CH_LAYOUT_OCTAGONAL or AV_CH_WIDE_LEFT or AV_CH_WIDE_RIGHT or AV_CH_TOP_BACK_LEFT or AV_CH_TOP_BACK_RIGHT or AV_CH_TOP_BACK_CENTER or AV_CH_TOP_FRONT_CENTER or AV_CH_TOP_FRONT_LEFT or AV_CH_TOP_FRONT_RIGHT)
  AV_CH_LAYOUT_STEREO_DOWNMIX* = (AV_CH_STEREO_LEFT or AV_CH_STEREO_RIGHT)
  AV_CH_LAYOUT_22POINT2* = (AV_CH_LAYOUT_5POINT1_BACK or AV_CH_FRONT_LEFT_OF_CENTER or AV_CH_FRONT_RIGHT_OF_CENTER or AV_CH_BACK_CENTER or AV_CH_LOW_FREQUENCY_2 or AV_CH_SIDE_LEFT or AV_CH_SIDE_RIGHT or AV_CH_TOP_FRONT_LEFT or AV_CH_TOP_FRONT_RIGHT or
      AV_CH_TOP_FRONT_CENTER or AV_CH_TOP_CENTER or AV_CH_TOP_BACK_LEFT or
      AV_CH_TOP_BACK_RIGHT or AV_CH_TOP_SIDE_LEFT or AV_CH_TOP_SIDE_RIGHT or
      AV_CH_TOP_BACK_CENTER or AV_CH_BOTTOM_FRONT_CENTER or
      AV_CH_BOTTOM_FRONT_LEFT or AV_CH_BOTTOM_FRONT_RIGHT)
  AV_NUM_DATA_POINTERS* = 8
  AV_FRAME_FLAG_CORRUPT* = (1 shl 0)
  AV_FRAME_FLAG_DISCARD* = (1 shl 2)
  FF_DECODE_ERROR_INVALID_BITSTREAM* = 1
  FF_DECODE_ERROR_MISSING_REFERENCE* = 2
  FF_DECODE_ERROR_CONCEALMENT_ACTIVE* = 4
  FF_DECODE_ERROR_DECODE_SLICES* = 8
  AVCODEC_BSF_H* = true
  AVCODEC_CODEC_ID_H* = true
  AV_CODEC_ID_IFF_BYTERUN1* = AV_CODEC_ID_IFF_ILBM
  AV_CODEC_ID_H265* = AV_CODEC_ID_HEVC
  AVCODEC_CODEC_PAR_H* = true
  AVCODEC_PACKET_H* = true
  AVCODEC_VERSION_H* = true
  LIBAVCODEC_VERSION_MAJOR* = 58
  LIBAVCODEC_VERSION_MINOR* = 112
  LIBAVCODEC_VERSION_MICRO* = 100
  LIBAVCODEC_VERSION_INT* = AV_VERSION_INT(LIBAVCODEC_VERSION_MAJOR, LIBAVCODEC_VERSION_MINOR, LIBAVCODEC_VERSION_MICRO)
  LIBAVCODEC_BUILD* = LIBAVCODEC_VERSION_INT
  FF_API_LOWRES* = (LIBAVCODEC_VERSION_MAJOR < 59)
  FF_API_DEBUG_MV* = (LIBAVCODEC_VERSION_MAJOR < 58)
  FF_API_AVCTX_TIMEBASE* = (LIBAVCODEC_VERSION_MAJOR < 59)
  FF_API_CODED_FRAME* = (LIBAVCODEC_VERSION_MAJOR < 59)
  FF_API_SIDEDATA_ONLY_PKT* = (LIBAVCODEC_VERSION_MAJOR < 59)
  FF_API_VDPAU_PROFILE* = (LIBAVCODEC_VERSION_MAJOR < 59)
  FF_API_CONVERGENCE_DURATION* = (LIBAVCODEC_VERSION_MAJOR < 59)
  FF_API_AVPICTURE* = (LIBAVCODEC_VERSION_MAJOR < 59)
  FF_API_AVPACKET_OLD_API* = (LIBAVCODEC_VERSION_MAJOR < 59)
  FF_API_RTP_CALLBACK* = (LIBAVCODEC_VERSION_MAJOR < 59)
  FF_API_VBV_DELAY* = (LIBAVCODEC_VERSION_MAJOR < 59)
  FF_API_CODER_TYPE* = (LIBAVCODEC_VERSION_MAJOR < 59)
  FF_API_STAT_BITS* = (LIBAVCODEC_VERSION_MAJOR < 59)
  FF_API_PRIVATE_OPT* = (LIBAVCODEC_VERSION_MAJOR < 59)
  FF_API_ASS_TIMING* = (LIBAVCODEC_VERSION_MAJOR < 59)
  FF_API_OLD_BSF* = (LIBAVCODEC_VERSION_MAJOR < 59)
  FF_API_COPY_CONTEXT* = (LIBAVCODEC_VERSION_MAJOR < 59)
  FF_API_GET_CONTEXT_DEFAULTS* = (LIBAVCODEC_VERSION_MAJOR < 59)
  FF_API_NVENC_OLD_NAME* = (LIBAVCODEC_VERSION_MAJOR < 59)
  FF_API_STRUCT_VAAPI_CONTEXT* = (LIBAVCODEC_VERSION_MAJOR < 59)
  FF_API_MERGE_SD_API* = (LIBAVCODEC_VERSION_MAJOR < 59)
  FF_API_TAG_STRING* = (LIBAVCODEC_VERSION_MAJOR < 59)
  FF_API_GETCHROMA* = (LIBAVCODEC_VERSION_MAJOR < 59)
  FF_API_CODEC_GET_SET* = (LIBAVCODEC_VERSION_MAJOR < 59)
  FF_API_USER_VISIBLE_AVHWACCEL* = (LIBAVCODEC_VERSION_MAJOR < 59)
  FF_API_LOCKMGR* = (LIBAVCODEC_VERSION_MAJOR < 59)
  FF_API_NEXT* = (LIBAVCODEC_VERSION_MAJOR < 59)
  FF_API_UNSANITIZED_BITRATES* = (LIBAVCODEC_VERSION_MAJOR < 59)
  FF_API_OPENH264_SLICE_MODE* = (LIBAVCODEC_VERSION_MAJOR < 59)
  FF_API_OPENH264_CABAC* = (LIBAVCODEC_VERSION_MAJOR < 59)
  FF_API_UNUSED_CODEC_CAPS* = (LIBAVCODEC_VERSION_MAJOR < 59)
  AV_PKT_DATA_QUALITY_FACTOR* = AV_PKT_DATA_QUALITY_STATS
  AV_PKT_FLAG_KEY* = 0x00000001
  AV_PKT_FLAG_CORRUPT* = 0x00000002
  AV_PKT_FLAG_DISCARD* = 0x00000004
  AV_PKT_FLAG_TRUSTED* = 0x00000008
  AV_PKT_FLAG_DISPOSABLE* = 0x00000010
  AVCODEC_CODEC_H* = true
  AV_CODEC_CAP_DRAW_HORIZ_BAND* = (1 shl 0)
  AV_CODEC_CAP_DR1* = (1 shl 1)
  AV_CODEC_CAP_TRUNCATED* = (1 shl 3)
  AV_CODEC_CAP_DELAY* = (1 shl 5)
  AV_CODEC_CAP_SMALL_LAST_FRAME* = (1 shl 6)
  AV_CODEC_CAP_SUBFRAMES* = (1 shl 8)
  AV_CODEC_CAP_EXPERIMENTAL* = (1 shl 9)
  AV_CODEC_CAP_CHANNEL_CONF* = (1 shl 10)
  AV_CODEC_CAP_FRAME_THREADS* = (1 shl 12)
  AV_CODEC_CAP_SLICE_THREADS* = (1 shl 13)
  AV_CODEC_CAP_PARAM_CHANGE* = (1 shl 14)
  AV_CODEC_CAP_AUTO_THREADS* = (1 shl 15)
  AV_CODEC_CAP_VARIABLE_FRAME_SIZE* = (1 shl 16)
  AV_CODEC_CAP_AVOID_PROBING* = (1 shl 17)
  AV_CODEC_CAP_INTRA_ONLY* = 0x40000000
  AV_CODEC_CAP_LOSSLESS* = 0x80000000
  AV_CODEC_CAP_HARDWARE* = (1 shl 18)
  AV_CODEC_CAP_HYBRID* = (1 shl 19)
  AV_CODEC_CAP_ENCODER_REORDERED_OPAQUE* = (1 shl 20)
  AV_CODEC_CAP_ENCODER_FLUSH* = (1 shl 21)
  AVCODEC_CODEC_DESC_H* = true
  AV_CODEC_PROP_INTRA_ONLY* = (1 shl 0)
  AV_CODEC_PROP_LOSSY* = (1 shl 1)
  AV_CODEC_PROP_LOSSLESS* = (1 shl 2)
  AV_CODEC_PROP_REORDER* = (1 shl 3)
  AV_CODEC_PROP_BITMAP_SUB* = (1 shl 16)
  AV_CODEC_PROP_TEXT_SUB* = (1 shl 17)
  AV_INPUT_BUFFER_PADDING_SIZE* = 64
  AV_INPUT_BUFFER_MIN_SIZE* = 16384
  AV_CODEC_FLAG_UNALIGNED* = (1 shl 0)
  AV_CODEC_FLAG_QSCALE* = (1 shl 1)
  AV_CODEC_FLAG_4MV* = (1 shl 2)
  AV_CODEC_FLAG_OUTPUT_CORRUPT* = (1 shl 3)
  AV_CODEC_FLAG_QPEL* = (1 shl 4)
  AV_CODEC_FLAG_DROPCHANGED* = (1 shl 5)
  AV_CODEC_FLAG_PASS1* = (1 shl 9)
  AV_CODEC_FLAG_PASS2* = (1 shl 10)
  AV_CODEC_FLAG_LOOP_FILTER* = (1 shl 11)
  AV_CODEC_FLAG_GRAY* = (1 shl 13)
  AV_CODEC_FLAG_PSNR* = (1 shl 15)
  AV_CODEC_FLAG_TRUNCATED* = (1 shl 16)
  AV_CODEC_FLAG_INTERLACED_DCT* = (1 shl 18)
  AV_CODEC_FLAG_LOW_DELAY* = (1 shl 19)
  AV_CODEC_FLAG_GLOBAL_HEADER* = (1 shl 22)
  AV_CODEC_FLAG_BITEXACT* = (1 shl 23)
  AV_CODEC_FLAG_AC_PRED* = (1 shl 24)
  AV_CODEC_FLAG_INTERLACED_ME* = (1 shl 29)
  AV_CODEC_FLAG_CLOSED_GOP* = (1 shl 31)
  AV_CODEC_FLAG2_FAST* = (1 shl 0)
  AV_CODEC_FLAG2_NO_OUTPUT* = (1 shl 2)
  AV_CODEC_FLAG2_LOCAL_HEADER* = (1 shl 3)
  AV_CODEC_FLAG2_DROP_FRAME_TIMECODE* = (1 shl 13)
  AV_CODEC_FLAG2_CHUNKS* = (1 shl 15)
  AV_CODEC_FLAG2_IGNORE_CROP* = (1 shl 16)
  AV_CODEC_FLAG2_SHOW_ALL* = (1 shl 22)
  AV_CODEC_FLAG2_EXPORT_MVS* = (1 shl 28)
  AV_CODEC_FLAG2_SKIP_MANUAL* = (1 shl 29)
  AV_CODEC_FLAG2_RO_FLUSH_NOOP* = (1 shl 30)
  AV_CODEC_EXPORT_DATA_MVS* = (1 shl 0)
  AV_CODEC_EXPORT_DATA_PRFT* = (1 shl 1)
  AV_CODEC_EXPORT_DATA_VIDEO_ENC_PARAMS* = (1 shl 2)
  AV_GET_BUFFER_FLAG_REF* = (1 shl 0)
  FF_COMPRESSION_DEFAULT* = -1
  FF_PRED_LEFT* = 0
  FF_PRED_PLANE* = 1
  FF_PRED_MEDIAN* = 2
  FF_CMP_SAD* = 0
  FF_CMP_SSE* = 1
  FF_CMP_SATD* = 2
  FF_CMP_DCT* = 3
  FF_CMP_PSNR* = 4
  FF_CMP_BIT* = 5
  FF_CMP_RD* = 6
  FF_CMP_ZERO* = 7
  FF_CMP_VSAD* = 8
  FF_CMP_VSSE* = 9
  FF_CMP_NSSE* = 10
  FF_CMP_W53* = 11
  FF_CMP_W97* = 12
  FF_CMP_DCTMAX* = 13
  FF_CMP_DCT264* = 14
  FF_CMP_MEDIAN_SAD* = 15
  FF_CMP_CHROMA* = 256
  SLICE_FLAG_CODED_ORDER* = 0x00000001
  SLICE_FLAG_ALLOW_FIELD* = 0x00000002
  SLICE_FLAG_ALLOW_PLANE* = 0x00000004
  FF_MB_DECISION_SIMPLE* = 0
  FF_MB_DECISION_BITS* = 1
  FF_MB_DECISION_RD* = 2
  FF_CODER_TYPE_VLC* = 0
  FF_CODER_TYPE_AC* = 1
  FF_CODER_TYPE_RAW* = 2
  FF_CODER_TYPE_RLE* = 3
  FF_BUG_AUTODETECT* = 1
  FF_BUG_XVID_ILACE* = 4
  FF_BUG_UMP4* = 8
  FF_BUG_NO_PADDING* = 16
  FF_BUG_AMV* = 32
  FF_BUG_QPEL_CHROMA* = 64
  FF_BUG_STD_QPEL* = 128
  FF_BUG_QPEL_CHROMA2* = 256
  FF_BUG_DIRECT_BLOCKSIZE* = 512
  FF_BUG_EDGE* = 1024
  FF_BUG_HPEL_CHROMA* = 2048
  FF_BUG_DC_CLIP* = 4096
  FF_BUG_MS* = 8192
  FF_BUG_TRUNCATED* = 16384
  FF_BUG_IEDGE* = 32768
  FF_COMPLIANCE_VERY_STRICT* = 2
  FF_COMPLIANCE_STRICT* = 1
  FF_COMPLIANCE_NORMAL* = 0
  FF_COMPLIANCE_UNOFFICIAL* = -1
  FF_COMPLIANCE_EXPERIMENTAL* = -2
  FF_EC_GUESS_MVS* = 1
  FF_EC_DEBLOCK* = 2
  FF_EC_FAVOR_INTER* = 256
  FF_DEBUG_PICT_INFO* = 1
  FF_DEBUG_RC* = 2
  FF_DEBUG_BITSTREAM* = 4
  FF_DEBUG_MB_TYPE* = 8
  FF_DEBUG_QP* = 16
  FF_DEBUG_DCT_COEFF* = 0x00000040
  FF_DEBUG_SKIP* = 0x00000080
  FF_DEBUG_STARTCODE* = 0x00000100
  FF_DEBUG_ER* = 0x00000400
  FF_DEBUG_MMCO* = 0x00000800
  FF_DEBUG_BUGS* = 0x00001000
  FF_DEBUG_BUFFERS* = 0x00008000
  FF_DEBUG_THREADS* = 0x00010000
  FF_DEBUG_GREEN_MD* = 0x00800000
  FF_DEBUG_NOMC* = 0x01000000
  AV_EF_CRCCHECK* = (1 shl 0)
  AV_EF_BITSTREAM* = (1 shl 1)
  AV_EF_BUFFER* = (1 shl 2)
  AV_EF_EXPLODE* = (1 shl 3)
  AV_EF_IGNORE_ERR* = (1 shl 15)
  AV_EF_CAREFUL* = (1 shl 16)
  AV_EF_COMPLIANT* = (1 shl 17)
  AV_EF_AGGRESSIVE* = (1 shl 18)
  FF_DCT_AUTO* = 0
  FF_DCT_FASTINT* = 1
  FF_DCT_INT* = 2
  FF_DCT_MMX* = 3
  FF_DCT_ALTIVEC* = 5
  FF_DCT_FAAN* = 6
  FF_IDCT_AUTO* = 0
  FF_IDCT_INT* = 1
  FF_IDCT_SIMPLE* = 2
  FF_IDCT_SIMPLEMMX* = 3
  FF_IDCT_ARM* = 7
  FF_IDCT_ALTIVEC* = 8
  FF_IDCT_SIMPLEARM* = 10
  FF_IDCT_XVID* = 14
  FF_IDCT_SIMPLEARMV5TE* = 16
  FF_IDCT_SIMPLEARMV6* = 17
  FF_IDCT_FAAN* = 20
  FF_IDCT_SIMPLENEON* = 22
  FF_IDCT_NONE* = 24
  FF_IDCT_SIMPLEAUTO* = 128
  FF_THREAD_FRAME* = 1
  FF_THREAD_SLICE* = 2
  FF_PROFILE_UNKNOWN* = -99
  FF_PROFILE_RESERVED* = -100
  FF_PROFILE_AAC_MAIN* = 0
  FF_PROFILE_AAC_LOW* = 1
  FF_PROFILE_AAC_SSR* = 2
  FF_PROFILE_AAC_LTP* = 3
  FF_PROFILE_AAC_HE* = 4
  FF_PROFILE_AAC_HE_V2* = 28
  FF_PROFILE_AAC_LD* = 22
  FF_PROFILE_AAC_ELD* = 38
  FF_PROFILE_MPEG2_AAC_LOW* = 128
  FF_PROFILE_MPEG2_AAC_HE* = 131
  FF_PROFILE_DNXHD* = 0
  FF_PROFILE_DNXHR_LB* = 1
  FF_PROFILE_DNXHR_SQ* = 2
  FF_PROFILE_DNXHR_HQ* = 3
  FF_PROFILE_DNXHR_HQX* = 4
  FF_PROFILE_DNXHR_444* = 5
  FF_PROFILE_DTS* = 20
  FF_PROFILE_DTS_ES* = 30
  FF_PROFILE_DTS_96_24* = 40
  FF_PROFILE_DTS_HD_HRA* = 50
  FF_PROFILE_DTS_HD_MA* = 60
  FF_PROFILE_DTS_EXPRESS* = 70
  FF_PROFILE_MPEG2_422* = 0
  FF_PROFILE_MPEG2_HIGH* = 1
  FF_PROFILE_MPEG2_SS* = 2
  FF_PROFILE_MPEG2_SNR_SCALABLE* = 3
  FF_PROFILE_MPEG2_MAIN* = 4
  FF_PROFILE_MPEG2_SIMPLE* = 5
  FF_PROFILE_H264_CONSTRAINED* = (1 shl 9)
  FF_PROFILE_H264_INTRA* = (1 shl 11)
  FF_PROFILE_H264_BASELINE* = 66
  FF_PROFILE_H264_CONSTRAINED_BASELINE* = (66 or FF_PROFILE_H264_CONSTRAINED)
  FF_PROFILE_H264_MAIN* = 77
  FF_PROFILE_H264_EXTENDED* = 88
  FF_PROFILE_H264_HIGH* = 100
  FF_PROFILE_H264_HIGH_10* = 110
  FF_PROFILE_H264_HIGH_10_INTRA* = (110 or FF_PROFILE_H264_INTRA)
  FF_PROFILE_H264_MULTIVIEW_HIGH* = 118
  FF_PROFILE_H264_HIGH_422* = 122
  FF_PROFILE_H264_HIGH_422_INTRA* = (122 or FF_PROFILE_H264_INTRA)
  FF_PROFILE_H264_STEREO_HIGH* = 128
  FF_PROFILE_H264_HIGH_444* = 144
  FF_PROFILE_H264_HIGH_444_PREDICTIVE* = 244
  FF_PROFILE_H264_HIGH_444_INTRA* = (244 or FF_PROFILE_H264_INTRA)
  FF_PROFILE_H264_CAVLC_444* = 44
  FF_PROFILE_VC1_SIMPLE* = 0
  FF_PROFILE_VC1_MAIN* = 1
  FF_PROFILE_VC1_COMPLEX* = 2
  FF_PROFILE_VC1_ADVANCED* = 3
  FF_PROFILE_MPEG4_SIMPLE* = 0
  FF_PROFILE_MPEG4_SIMPLE_SCALABLE* = 1
  FF_PROFILE_MPEG4_CORE* = 2
  FF_PROFILE_MPEG4_MAIN* = 3
  FF_PROFILE_MPEG4_N_BIT* = 4
  FF_PROFILE_MPEG4_SCALABLE_TEXTURE* = 5
  FF_PROFILE_MPEG4_SIMPLE_FACE_ANIMATION* = 6
  FF_PROFILE_MPEG4_BASIC_ANIMATED_TEXTURE* = 7
  FF_PROFILE_MPEG4_HYBRID* = 8
  FF_PROFILE_MPEG4_ADVANCED_REAL_TIME* = 9
  FF_PROFILE_MPEG4_CORE_SCALABLE* = 10
  FF_PROFILE_MPEG4_ADVANCED_CODING* = 11
  FF_PROFILE_MPEG4_ADVANCED_CORE* = 12
  FF_PROFILE_MPEG4_ADVANCED_SCALABLE_TEXTURE* = 13
  FF_PROFILE_MPEG4_SIMPLE_STUDIO* = 14
  FF_PROFILE_MPEG4_ADVANCED_SIMPLE* = 15
  FF_PROFILE_JPEG2000_CSTREAM_RESTRICTION_0* = 1
  FF_PROFILE_JPEG2000_CSTREAM_RESTRICTION_1* = 2
  FF_PROFILE_JPEG2000_CSTREAM_NO_RESTRICTION* = 32768
  FF_PROFILE_JPEG2000_DCINEMA_2K* = 3
  FF_PROFILE_JPEG2000_DCINEMA_4K* = 4
  FF_PROFILE_VP9_0* = 0
  FF_PROFILE_VP9_1* = 1
  FF_PROFILE_VP9_2* = 2
  FF_PROFILE_VP9_3* = 3
  FF_PROFILE_HEVC_MAIN* = 1
  FF_PROFILE_HEVC_MAIN_10* = 2
  FF_PROFILE_HEVC_MAIN_STILL_PICTURE* = 3
  FF_PROFILE_HEVC_REXT* = 4
  FF_PROFILE_AV1_MAIN* = 0
  FF_PROFILE_AV1_HIGH* = 1
  FF_PROFILE_AV1_PROFESSIONAL* = 2
  FF_PROFILE_MJPEG_HUFFMAN_BASELINE_DCT* = 0x000000C0
  FF_PROFILE_MJPEG_HUFFMAN_EXTENDED_SEQUENTIAL_DCT* = 0x000000C1
  FF_PROFILE_MJPEG_HUFFMAN_PROGRESSIVE_DCT* = 0x000000C2
  FF_PROFILE_MJPEG_HUFFMAN_LOSSLESS* = 0x000000C3
  FF_PROFILE_MJPEG_JPEG_LS* = 0x000000F7
  FF_PROFILE_SBC_MSBC* = 1
  FF_PROFILE_PRORES_PROXY* = 0
  FF_PROFILE_PRORES_LT* = 1
  FF_PROFILE_PRORES_STANDARD* = 2
  FF_PROFILE_PRORES_HQ* = 3
  FF_PROFILE_PRORES_4444* = 4
  FF_PROFILE_PRORES_XQ* = 5
  FF_PROFILE_ARIB_PROFILE_A* = 0
  FF_PROFILE_ARIB_PROFILE_C* = 1
  FF_PROFILE_KLVA_SYNC* = 0
  FF_PROFILE_KLVA_ASYNC* = 1
  FF_LEVEL_UNKNOWN* = -99
  FF_SUB_CHARENC_MODE_DO_NOTHING* = -1
  FF_SUB_CHARENC_MODE_AUTOMATIC* = 0
  FF_SUB_CHARENC_MODE_PRE_DECODER* = 1
  FF_SUB_CHARENC_MODE_IGNORE* = 2
  FF_DEBUG_VIS_MV_P_FOR* = 0x00000001
  FF_DEBUG_VIS_MV_B_FOR* = 0x00000002
  FF_DEBUG_VIS_MV_B_BACK* = 0x00000004
  FF_CODEC_PROPERTY_LOSSLESS* = 0x00000001
  FF_CODEC_PROPERTY_CLOSED_CAPTIONS* = 0x00000002
  FF_SUB_TEXT_FMT_ASS* = 0
  FF_SUB_TEXT_FMT_ASS_WITH_TIMINGS* = 1
  AV_HWACCEL_CODEC_CAP_EXPERIMENTAL* = 0x00000200
  AV_HWACCEL_FLAG_IGNORE_LEVEL* = (1 shl 0)
  AV_HWACCEL_FLAG_ALLOW_HIGH_DEPTH* = (1 shl 1)
  AV_HWACCEL_FLAG_ALLOW_PROFILE_MISMATCH* = (1 shl 2)
  AV_SUBTITLE_FLAG_FORCED* = 0x00000001
  AV_PARSER_PTS_NB* = 4
  PARSER_FLAG_COMPLETE_FRAMES* = 0x00000001
  PARSER_FLAG_ONCE* = 0x00000002
  PARSER_FLAG_FETCHED_OFFSET* = 0x00000004
  PARSER_FLAG_USE_CODEC_TS* = 0x00001000
  LIBAVFORMAT_VERSION_MAJOR* = 58
  LIBAVFORMAT_VERSION_MINOR* = 63
  LIBAVFORMAT_VERSION_MICRO* = 100
  LIBAVFORMAT_VERSION_INT* = AV_VERSION_INT(LIBAVFORMAT_VERSION_MAJOR,LIBAVFORMAT_VERSION_MINOR, LIBAVFORMAT_VERSION_MICRO)
  # LIBAVFORMAT_VERSION* = AV_VERSION(LIBAVFORMAT_VERSION_MAJOR, LIBAVFORMAT_VERSION_MINOR, LIBAVFORMAT_VERSION_MICRO)
  LIBAVFORMAT_BUILD* = LIBAVFORMAT_VERSION_INT
  #LIBAVFORMAT_IDENT "Lavf" AV_STRINGIFY(LIBAVFORMAT_VERSION)
  FF_API_COMPUTE_PKT_FIELDS2* = (LIBAVFORMAT_VERSION_MAJOR < 59)
  FF_API_OLD_OPEN_CALLBACKS* = (LIBAVFORMAT_VERSION_MAJOR < 59)
  FF_API_LAVF_AVCTX* = (LIBAVFORMAT_VERSION_MAJOR < 59)
  FF_API_HTTP_USER_AGENT* = (LIBAVFORMAT_VERSION_MAJOR < 59)
  FF_API_HLS_WRAP* = (LIBAVFORMAT_VERSION_MAJOR < 59)
  FF_API_HLS_USE_LOCALTIME* = (LIBAVFORMAT_VERSION_MAJOR < 59)
  FF_API_LAVF_KEEPSIDE_FLAG* = (LIBAVFORMAT_VERSION_MAJOR < 59)
  FF_API_OLD_ROTATE_API* = (LIBAVFORMAT_VERSION_MAJOR < 59)
  FF_API_FORMAT_GET_SET* = (LIBAVFORMAT_VERSION_MAJOR < 59)
  FF_API_OLD_AVIO_EOF_0* = (LIBAVFORMAT_VERSION_MAJOR < 59)
  FF_API_LAVF_FFSERVER* = (LIBAVFORMAT_VERSION_MAJOR < 59)
  FF_API_FORMAT_FILENAME* = (LIBAVFORMAT_VERSION_MAJOR < 59)
  FF_API_OLD_RTSP_OPTIONS* = (LIBAVFORMAT_VERSION_MAJOR < 59)
  FF_API_DASH_MIN_SEG_DURATION* = (LIBAVFORMAT_VERSION_MAJOR < 59)
  FF_API_LAVF_MP4A_LATM* = (LIBAVFORMAT_VERSION_MAJOR < 59)
  FF_API_AVIOFORMAT* = (LIBAVFORMAT_VERSION_MAJOR < 59)
  FF_API_R_FRAME_RATE* = 1
  AVIO_SEEKABLE_NORMAL* = (1 shl 0)
  AVIO_SEEKABLE_TIME* = (1 shl 1)
  AVSEEK_SIZE* = 0x00010000
  AVSEEK_FORCE* = 0x00020000
  AVIO_FLAG_READ* = 1
  AVIO_FLAG_WRITE* = 2
  AVIO_FLAG_READ_WRITE* = (AVIO_FLAG_READ or AVIO_FLAG_WRITE)
  AVIO_FLAG_NONBLOCK* = 8
  AVIO_FLAG_DIRECT* = 0x00008000
  AVPROBE_SCORE_MAX* = 100
  AVPROBE_SCORE_RETRY* = (AVPROBE_SCORE_MAX div 4)
  AVPROBE_SCORE_STREAM_RETRY* = (AVPROBE_SCORE_MAX div 4 - 1)
  AVPROBE_SCORE_EXTENSION* = 50
  AVPROBE_SCORE_MIME* = 75
  AVPROBE_PADDING_SIZE* = 32
  AVFMT_NOFILE* = 0x00000001
  AVFMT_NEEDNUMBER* = 0x00000002
  AVFMT_SHOW_IDS* = 0x00000008
  AVFMT_GLOBALHEADER* = 0x00000040
  AVFMT_NOTIMESTAMPS* = 0x00000080
  AVFMT_GENERIC_INDEX* = 0x00000100
  AVFMT_TS_DISCONT* = 0x00000200
  AVFMT_VARIABLE_FPS* = 0x00000400
  AVFMT_NODIMENSIONS* = 0x00000800
  AVFMT_NOSTREAMS* = 0x00001000
  AVFMT_NOBINSEARCH* = 0x00002000
  AVFMT_NOGENSEARCH* = 0x00004000
  AVFMT_NO_BYTE_SEEK* = 0x00008000
  AVFMT_ALLOW_FLUSH* = 0x00010000
  AVFMT_TS_NONSTRICT* = 0x00020000
  AVFMT_TS_NEGATIVE* = 0x00040000
  AVFMT_SEEK_TO_PTS* = 0x04000000
  ff_const59* = true
  AVINDEX_KEYFRAME* = 0x00000001
  AVINDEX_DISCARD_FRAME* = 0x00000002
  AV_DISPOSITION_DEFAULT* = 0x00000001
  AV_DISPOSITION_DUB* = 0x00000002
  AV_DISPOSITION_ORIGINAL* = 0x00000004
  AV_DISPOSITION_COMMENT* = 0x00000008
  AV_DISPOSITION_LYRICS* = 0x00000010
  AV_DISPOSITION_KARAOKE* = 0x00000020
  AV_DISPOSITION_FORCED* = 0x00000040
  AV_DISPOSITION_HEARING_IMPAIRED* = 0x00000080
  AV_DISPOSITION_VISUAL_IMPAIRED* = 0x00000100
  AV_DISPOSITION_CLEAN_EFFECTS* = 0x00000200
  AV_DISPOSITION_ATTACHED_PIC* = 0x00000400
  AV_DISPOSITION_TIMED_THUMBNAILS* = 0x00000800
  AV_DISPOSITION_CAPTIONS* = 0x00010000
  AV_DISPOSITION_DESCRIPTIONS* = 0x00020000
  AV_DISPOSITION_METADATA* = 0x00040000
  AV_DISPOSITION_DEPENDENT* = 0x00080000
  AV_DISPOSITION_STILL_IMAGE* = 0x00100000
  AV_PTS_WRAP_IGNORE* = 0
  AV_PTS_WRAP_ADD_OFFSET* = 1
  AV_PTS_WRAP_SUB_OFFSET* = -1
  AVSTREAM_EVENT_FLAG_METADATA_UPDATED* = 0x00000001
  MAX_STD_TIMEBASES* = (30 * 12 + 30 + 3 + 6)
  MAX_REORDER_DELAY* = 16
  AV_PROGRAM_RUNNING* = 1
  AVFMTCTX_NOHEADER* = 0x00000001
  AVFMTCTX_UNSEEKABLE* = 0x00000002
  AVFMT_FLAG_GENPTS* = 0x00000001
  AVFMT_FLAG_IGNIDX* = 0x00000002
  AVFMT_FLAG_NONBLOCK* = 0x00000004
  AVFMT_FLAG_IGNDTS* = 0x00000008
  AVFMT_FLAG_NOFILLIN* = 0x00000010
  AVFMT_FLAG_NOPARSE* = 0x00000020
  AVFMT_FLAG_NOBUFFER* = 0x00000040
  AVFMT_FLAG_CUSTOM_IO* = 0x00000080
  AVFMT_FLAG_DISCARD_CORRUPT* = 0x00000100
  AVFMT_FLAG_FLUSH_PACKETS* = 0x00000200
  AVFMT_FLAG_BITEXACT* = 0x00000400
  AVFMT_FLAG_MP4A_LATM* = 0x00008000
  AVFMT_FLAG_SORT_DTS* = 0x00010000
  AVFMT_FLAG_PRIV_OPT* = 0x00020000
  AVFMT_FLAG_KEEP_SIDE_DATA* = 0x00040000
  AVFMT_FLAG_FAST_SEEK* = 0x00080000
  AVFMT_FLAG_SHORTEST* = 0x00100000
  AVFMT_FLAG_AUTO_BSF* = 0x00200000
  FF_FDEBUG_TS* = 0x00000001
  AVFMT_EVENT_FLAG_METADATA_UPDATED* = 0x00000001
  AVFMT_AVOID_NEG_TS_AUTO* = -1
  AVFMT_AVOID_NEG_TS_MAKE_NON_NEGATIVE* = 1
  AVFMT_AVOID_NEG_TS_MAKE_ZERO* = 2
  AVSEEK_FLAG_BACKWARD* = 1
  AVSEEK_FLAG_BYTE* = 2
  AVSEEK_FLAG_ANY* = 4
  AVSEEK_FLAG_FRAME* = 8
  AVSTREAM_INIT_IN_WRITE_HEADER* = 0
  AVSTREAM_INIT_IN_INIT_OUTPUT* = 1
  AV_FRAME_FILENAME_FLAGS_MULTIPLE* = 1
  AVDEVICE_AVDEVICE_H* = true
  AVDEVICE_VERSION_H* = true
  LIBAVDEVICE_VERSION_MAJOR* = 58
  LIBAVDEVICE_VERSION_MINOR* = 11
  LIBAVDEVICE_VERSION_MICRO* = 102
  LIBAVDEVICE_VERSION_INT* = AV_VERSION_INT(LIBAVDEVICE_VERSION_MAJOR,LIBAVDEVICE_VERSION_MINOR, LIBAVDEVICE_VERSION_MICRO)
  # LIBAVDEVICE_VERSION* = AV_VERSION(LIBAVDEVICE_VERSION_MAJOR,LIBAVDEVICE_VERSION_MINOR,LIBAVDEVICE_VERSION_MICRO)
  LIBAVDEVICE_BUILD* = LIBAVDEVICE_VERSION_INT
  #LIBAVDEVICE_IDENT = "Lavd" AV_STRINGIFY(LIBAVDEVICE_VERSION)
  AVUTIL_OPT_H* = true
  AV_OPT_FLAG_ENCODING_PARAM* = 1
  AV_OPT_FLAG_DECODING_PARAM* = 2
  AV_OPT_FLAG_AUDIO_PARAM* = 8
  AV_OPT_FLAG_VIDEO_PARAM* = 16
  AV_OPT_FLAG_SUBTITLE_PARAM* = 32
  AV_OPT_FLAG_EXPORT* = 64
  AV_OPT_FLAG_READONLY* = 128
  AV_OPT_FLAG_BSF_PARAM* = (1 shl 8)
  AV_OPT_FLAG_RUNTIME_PARAM* = (1 shl 15)
  AV_OPT_FLAG_FILTERING_PARAM* = (1 shl 16)
  AV_OPT_FLAG_DEPRECATED* = (1 shl 17)
  AV_OPT_FLAG_CHILD_CONSTS* = (1 shl 18)
  AV_OPT_SEARCH_CHILDREN* = (1 shl 0)
  AV_OPT_SEARCH_FAKE_OBJ* = (1 shl 1)
  AV_OPT_ALLOW_NULL* = (1 shl 2)
  AV_OPT_MULTI_COMPONENT_RANGE* = (1 shl 12)
  #define av_err2str(errnum) av_make_error_string((char[AV_ERROR_MAX_STRING_SIZE]){0}, AV_ERROR_MAX_STRING_SIZE, errnum)
  #define DECLARE_ALIGNED(n, t, v) t __attribute__((aligned(n))) v
  #define DECLARE_ASM_ALIGNED(n, t, v) t av_used __attribute__((aligned(n))) v
  #define DECLARE_ASM_CONST(n, t, v) static const t av_used __attribute__((aligned(n))) v
  #define av_malloc_attrib __attribute__((__malloc__))
  #define av_alloc_size(...) __attribute__((alloc_size(__VA_ARGS__)))
  K_SCANCODE_MASK = 1 shl 30
  K_UNKNOWN*: cint = 0
  K_BACKSPACE*: cint = '\b'.cint
  K_TAB*: cint = '\t'.cint
  K_RETURN*: cint = '\r'.cint
  K_ESCAPE*: cint = '\27'.cint
  K_SPACE*: cint = ' '.cint
  K_EXCLAIM*: cint = '!'.cint
  K_QUOTEDBL*: cint = '"'.cint
  K_HASH*: cint = '#'.cint
  K_DOLLAR*: cint = '$'.cint
  K_PERCENT*: cint = '%'.cint
  K_AMPERSAND*: cint = '&'.cint
  K_QUOTE*: cint = '\''.cint
  K_LEFTPAREN*: cint = '('.cint
  K_RIGHTPAREN*: cint = ')'.cint
  K_ASTERISK*: cint = '*'.cint
  K_PLUS*: cint = '+'.cint
  K_COMMA*: cint = ','.cint
  K_MINUS*: cint = '-'.cint
  K_PERIOD*: cint = '.'.cint
  K_SLASH*: cint = '/'.cint
  K_0*: cint = '0'.cint
  K_1*: cint = '1'.cint
  K_2*: cint = '2'.cint
  K_3*: cint = '3'.cint
  K_4*: cint = '4'.cint
  K_5*: cint = '5'.cint
  K_6*: cint = '6'.cint
  K_7*: cint = '7'.cint
  K_8*: cint = '8'.cint
  K_9*: cint = '9'.cint
  K_COLON*: cint = ':'.cint
  K_SEMICOLON*: cint = ';'.cint
  K_LESS*: cint = '<'.cint
  K_EQUALS*: cint = '='.cint
  K_GREATER*: cint = '>'.cint
  K_QUESTION*: cint = '?'.cint
  K_AT*: cint = '@'.cint # Skip uppercase letters
  K_LEFTBRACKET*: cint = '['.cint
  K_BACKSLASH*: cint = '\\'.cint
  K_RIGHTBRACKET*: cint = ']'.cint
  K_CARET*: cint = '^'.cint
  K_UNDERSCORE*: cint = '_'.cint
  K_BACKQUOTE*: cint = '`'.cint
  K_a*: cint = 'a'.cint
  K_b*: cint = 'b'.cint
  K_c*: cint = 'c'.cint
  K_d*: cint = 'd'.cint
  K_e*: cint = 'e'.cint
  K_f*: cint = 'f'.cint
  K_g*: cint = 'g'.cint
  K_h*: cint = 'h'.cint
  K_i*: cint = 'i'.cint
  K_j*: cint = 'j'.cint
  K_k*: cint = 'k'.cint
  K_l*: cint = 'l'.cint
  K_m*: cint = 'm'.cint
  K_n*: cint = 'n'.cint
  K_o*: cint = 'o'.cint
  K_p*: cint = 'p'.cint
  K_q*: cint = 'q'.cint
  K_r*: cint =  'r'.cint
  K_s*: cint =  's'.cint
  K_t*: cint =  't'.cint
  K_u*: cint =  'u'.cint
  K_v*: cint =  'v'.cint
  K_w*: cint =  'w'.cint
  K_x*: cint =  'x'.cint
  K_y*: cint =  'y'.cint
  K_z*: cint =  'z'.cint

  K_DELETE*: cint =  '\127'.cint
  K_CAPSLOCK*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CAPSLOCK)

  K_F1*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F1)
  K_F2*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F2)
  K_F3*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F3)
  K_F4*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F4)
  K_F5*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F5)
  K_F6*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F6)
  K_F7*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F7)
  K_F8*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F8)
  K_F9*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F9)
  K_F10*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F10)
  K_F11*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F11)
  K_F12*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F12)

  K_PRINTSCREEN*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PRINTSCREEN)
  K_SCROLLLOCK*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SCROLLLOCK)
  K_PAUSE*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PAUSE)
  K_INSERT*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_INSERT)
  K_HOME*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_HOME)
  K_PAGEUP*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PAGEUP)
  K_END*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_END)
  K_PAGEDOWN*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PAGEDOWN)
  K_RIGHT*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RIGHT)
  K_LEFT*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LEFT)
  K_DOWN*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_DOWN)
  K_UP*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_UP)

  K_NUMLOCKCLEAR*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_NUMLOCKCLEAR)
  K_KP_DIVIDE*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_DIVIDE)
  K_KP_MULTIPLY*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MULTIPLY)
  K_KP_MINUS*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MINUS)
  K_KP_PLUS*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PLUS)
  K_KP_ENTER*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_ENTER)
  K_KP_1*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_1)
  K_KP_2*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_2)
  K_KP_3*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_3)
  K_KP_4*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_4)
  K_KP_5*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_5)
  K_KP_6*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_6)
  K_KP_7*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_7)
  K_KP_8*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_8)
  K_KP_9*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_9)
  K_KP_0*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_0)
  K_KP_PERIOD*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PERIOD)

  K_APPLICATION*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_APPLICATION)
  K_POWER*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_POWER)
  K_KP_EQUALS*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_EQUALS)
  K_F13*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F13)
  K_F14*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F14)
  K_F15*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F15)
  K_F16*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F16)
  K_F17*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F17)
  K_F18*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F18)
  K_F19*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F19)
  K_F20*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F20)
  K_F21*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F21)
  K_F22*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F22)
  K_F23*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F23)
  K_F24*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F24)
  K_EXECUTE*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_EXECUTE)
  K_HELP*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_HELP)
  K_MENU*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MENU)
  K_SELECT*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SELECT)
  K_STOP*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_STOP)
  K_AGAIN*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AGAIN)
  K_UNDO*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_UNDO)
  K_CUT*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CUT)
  K_COPY*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_COPY)
  K_PASTE*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PASTE)
  K_FIND*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_FIND)
  K_MUTE*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MUTE)
  K_VOLUMEUP*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_VOLUMEUP)
  K_VOLUMEDOWN*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_VOLUMEDOWN)
  K_KP_COMMA*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_COMMA)
  K_KP_EQUALSAS400*: cint =
    SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_EQUALSAS400)

  K_ALTERASE*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_ALTERASE)
  K_SYSREQ*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SYSREQ)
  K_CANCEL*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CANCEL)
  K_CLEAR*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CLEAR)
  K_PRIOR*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PRIOR)
  K_RETURN2*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RETURN2)
  K_SEPARATOR*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SEPARATOR)
  K_OUT*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_OUT)
  K_OPER*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_OPER)
  K_CLEARAGAIN*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CLEARAGAIN)
  K_CRSEL*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CRSEL)
  K_EXSEL*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_EXSEL)

  K_KP_00*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_00)
  K_KP_000*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_000)
  K_THOUSANDSSEPARATOR*: cint =
    SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_THOUSANDSSEPARATOR)
  K_DECIMALSEPARATOR*: cint =
    SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_DECIMALSEPARATOR)
  K_CURRENCYUNIT*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CURRENCYUNIT)
  K_CURRENCYSUBUNIT*: cint =
    SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CURRENCYSUBUNIT)
  K_KP_LEFTPAREN*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_LEFTPAREN)
  K_KP_RIGHTPAREN*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_RIGHTPAREN)
  K_KP_LEFTBRACE*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_LEFTBRACE)
  K_KP_RIGHTBRACE*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_RIGHTBRACE)
  K_KP_TAB*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_TAB)
  K_KP_BACKSPACE*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_BACKSPACE)
  K_KP_A*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_A)
  K_KP_B*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_B)
  K_KP_C*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_C)
  K_KP_D*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_D)
  K_KP_E*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_E)
  K_KP_F*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_F)
  K_KP_XOR*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_XOR)
  K_KP_POWER*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_POWER)
  K_KP_PERCENT*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PERCENT)
  K_KP_LESS*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_LESS)
  K_KP_GREATER*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_GREATER)
  K_KP_AMPERSAND*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_AMPERSAND)
  K_KP_DBLAMPERSAND*: cint =
    SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_DBLAMPERSAND)
  K_KP_VERTICALBAR*: cint =
    SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_VERTICALBAR)
  K_KP_DBLVERTICALBAR*: cint =
    SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_DBLVERTICALBAR)
  K_KP_COLON*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_COLON)
  K_KP_HASH*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_HASH)
  K_KP_SPACE*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_SPACE)
  K_KP_AT*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_AT)
  K_KP_EXCLAM*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_EXCLAM)
  K_KP_MEMSTORE*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMSTORE)
  K_KP_MEMRECALL*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMRECALL)
  K_KP_MEMCLEAR*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMCLEAR)
  K_KP_MEMADD*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMADD)
  K_KP_MEMSUBTRACT*: cint =
    SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMSUBTRACT)
  K_KP_MEMMULTIPLY*: cint =
    SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMMULTIPLY)
  K_KP_MEMDIVIDE*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMDIVIDE)
  K_KP_PLUSMINUS*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PLUSMINUS)
  K_KP_CLEAR*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_CLEAR)
  K_KP_CLEARENTRY*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_CLEARENTRY)
  K_KP_BINARY*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_BINARY)
  K_KP_OCTAL*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_OCTAL)
  K_KP_DECIMAL*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_DECIMAL)
  K_KP_HEXADECIMAL*: cint =
    SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_HEXADECIMAL)

  K_LCTRL*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LCTRL)
  K_LSHIFT*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LSHIFT)
  K_LALT*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LALT)
  K_LGUI*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LGUI)
  K_RCTRL*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RCTRL)
  K_RSHIFT*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RSHIFT)
  K_RALT*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RALT)
  K_RGUI*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RGUI)

  K_MODE*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MODE)

  K_AUDIONEXT*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AUDIONEXT)
  K_AUDIOPREV*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AUDIOPREV)
  K_AUDIOSTOP*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AUDIOSTOP)
  K_AUDIOPLAY*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AUDIOPLAY)
  K_AUDIOMUTE*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AUDIOMUTE)
  K_MEDIASELECT*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIASELECT)
  K_WWW*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_WWW)
  K_MAIL*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MAIL)
  K_CALCULATOR*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CALCULATOR)
  K_COMPUTER*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_COMPUTER)
  K_AC_SEARCH*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_SEARCH)
  K_AC_HOME*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_HOME)
  K_AC_BACK*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_BACK)
  K_AC_FORWARD*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_FORWARD)
  K_AC_STOP*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_STOP)
  K_AC_REFRESH*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_REFRESH)
  K_AC_BOOKMARKS*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_BOOKMARKS)

  K_BRIGHTNESSDOWN*: cint =
    SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_BRIGHTNESSDOWN)
  K_BRIGHTNESSUP*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_BRIGHTNESSUP)
  K_DISPLAYSWITCH*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_DISPLAYSWITCH)
  K_KBDILLUMTOGGLE*: cint =
    SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KBDILLUMTOGGLE)
  K_KBDILLUMDOWN*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KBDILLUMDOWN)
  K_KBDILLUMUP*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KBDILLUMUP)
  K_EJECT*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_EJECT)
  K_SLEEP*: cint =  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SLEEP)
  LIBAVUTIL_VERSION_MAJOR* = 56
  LIBAVUTIL_VERSION_MINOR* = 60
  LIBAVUTIL_VERSION_MICRO* = 100
  LIBAVUTIL_VERSION_INT* = AV_VERSION_INT(LIBAVUTIL_VERSION_MAJOR,LIBAVUTIL_VERSION_MINOR,LIBAVUTIL_VERSION_MICRO)
  # LIBAVUTIL_VERSION* = AV_VERSION(LIBAVUTIL_VERSION_MAJOR, LIBAVUTIL_VERSION_MINOR, LIBAVUTIL_VERSION_MICRO)
  LIBAVUTIL_BUILD* = LIBAVUTIL_VERSION_INT
  #LIBAVUTIL_IDENT  = "Lavu" AV_STRINGIFY(LIBAVUTIL_VERSION)
  FF_API_VAAPI* = (LIBAVUTIL_VERSION_MAJOR < 57)
  FF_API_FRAME_QP* = (LIBAVUTIL_VERSION_MAJOR < 57)
  FF_API_PLUS1_MINUS1* = (LIBAVUTIL_VERSION_MAJOR < 57)
  FF_API_ERROR_FRAME* = (LIBAVUTIL_VERSION_MAJOR < 57)
  FF_API_PKT_PTS* = (LIBAVUTIL_VERSION_MAJOR < 57)
  FF_API_CRYPTO_SIZE_T* = (LIBAVUTIL_VERSION_MAJOR < 57)
  FF_API_FRAME_GET_SET* = (LIBAVUTIL_VERSION_MAJOR < 57)
  FF_API_PSEUDOPAL* = (LIBAVUTIL_VERSION_MAJOR < 57)
  FF_API_CHILD_CLASS_NEXT* = (LIBAVUTIL_VERSION_MAJOR < 57)
  FF_API_D2STR* = (LIBAVUTIL_VERSION_MAJOR < 58)
  AV_ESCAPE_FLAG_WHITESPACE* = (1 shl 0)
  AV_ESCAPE_FLAG_STRICT* = (1 shl 1)
  AV_UTF8_FLAG_ACCEPT_INVALID_BIG_CODES* = 1
  AV_UTF8_FLAG_ACCEPT_NON_CHARACTERS* = 2
  AV_UTF8_FLAG_ACCEPT_SURROGATES* = 4
  AV_UTF8_FLAG_EXCLUDE_XML_INVALID_CONTROL_CODES* = 8
  AV_UTF8_FLAG_ACCEPT_ALL* = AV_UTF8_FLAG_ACCEPT_INVALID_BIG_CODES or AV_UTF8_FLAG_ACCEPT_NON_CHARACTERS or AV_UTF8_FLAG_ACCEPT_SURROGATES
  FF_LAMBDA_SHIFT* = 7
  FF_LAMBDA_SCALE* = (1 shl FF_LAMBDA_SHIFT)
  FF_QP2LAMBDA* = 118
  FF_LAMBDA_MAX* = (256 * 128 - 1)
  FF_QUALITY_SCALE* = FF_LAMBDA_SCALE
  AV_NOPTS_VALUE* = 0
  AV_TIME_BASE* = 1000000
  M_LOG2_10* = 3.321928094887362
  M_PHI* = 1.618033988749895



type
  AVRational* {.bycopy.} = object
    num*: cint                 ## /< Numerator
    den*: cint                 ## /< Denominator
  AVDeviceRect* {.bycopy.} = object
    x*: cint
    y*: cint
    width*: cint
    height*: cint
  AVDictionaryEntry* {.bycopy.} = object
    key*: cstring
    value*: cstring

  AVDictionary* {.bycopy.} = object
    count*: cint
    elems*: ptr AVDictionaryEntry

  SDL_AssertData* {.bycopy.} = object
    always_ignore*: cint
    trigger_count*: cuint
    condition*: cstring
    filename*: cstring
    linenum*: cint
    function*: cstring
    next*: ptr SDL_AssertData

  SDL_Color* {.bycopy.} = object
    r*: uint8
    g*: uint8
    b*: uint8
    a*: uint8

  SDL_Palette* {.bycopy.} = object
    ncolors*: cint
    colors*: ptr SDL_Color
    version*: uint32
    refcount*: cint

  SDL_PixelFormat* {.bycopy.} = object
    format*: uint32
    palette*: ptr SDL_Palette
    BitsPerPixel*: uint8
    BytesPerPixel*: uint8
    padding*: array[2, uint8]
    Rmask*: uint32
    Gmask*: uint32
    Bmask*: uint32
    Amask*: uint32
    Rloss*: uint8
    Gloss*: uint8
    Bloss*: uint8
    Aloss*: uint8
    Rshift*: uint8
    Gshift*: uint8
    Bshift*: uint8
    Ashift*: uint8
    refcount*: cint
    next*: ptr SDL_PixelFormat

  SwsVector* {.bycopy.} = object
    coeff*: ptr cdouble         ## /< pointer to the list of coefficients
    length*: cint              ## /< number of coefficients in the vector

  AVDeviceInfo* {.bycopy.} = object
    device_name*: cstring
    device_description*: cstring

  AVDeviceInfoList* {.bycopy.} = object
    devices*: ptr ptr AVDeviceInfo
    nb_devices*: cint
    default_device*: cint
    
  SwsFilter* {.bycopy.} = object
    lumH*: ptr SwsVector
    lumV*: ptr SwsVector
    chrH*: ptr SwsVector
    chrV*: ptr SwsVector

  SwsContext* {.bycopy.} = object

  av_intfloat32* {.union.} = object 
    i*: uint32
    f*: cfloat

  av_intfloat64* {.union.} = object 
    i*: uint64
    f*: cdouble

  INNER_C_UNION_playground_5456* {.union.} = object
    i64*: int64
    dbl*: cdouble
    str*: cstring
    q*: AVRational

  AVOption* {.bycopy.} = object
    name*: cstring
    help*: cstring
    offset*: cint
    `type`*: AVOptionType
    default_val*: INNER_C_UNION_playground_5456
    min*: cdouble              ## /< minimum valid value for the option
    max*: cdouble              ## /< maximum valid value for the option
    flags*: cint
    unit*: cstring

  AVOptionRange* {.bycopy.} = object
    str*: cstring
    value_min*: cdouble
    value_max*: cdouble
    component_min*: cdouble
    component_max*: cdouble
    is_range*: cint

  AVOptionRanges* {.bycopy.} = object
    range*: ptr ptr AVOptionRange
    nb_ranges*: cint
    nb_components*: cint

  AVClass* {.bycopy.} = object
    class_name*: cstring
    item_name*: proc (ctx: pointer): cstring
    option*: ptr AVOption
    version*: cint
    log_level_offset_offset*: cint
    parent_log_context_offset*: cint
    child_next*: proc (obj: pointer; prev: pointer): pointer
    child_class_next*: proc (prev: ptr AVClass): ptr AVClass
    category*: AVClassCategory
    get_category*: proc (ctx: pointer): AVClassCategory
    query_ranges*: proc (a1: ptr ptr AVOptionRanges; obj: pointer; key: cstring;flags: cint): cint
    child_class_iterate*: proc (iter: ptr pointer): ptr AVClass

  INNER_C_UNION_playground_57* {.union.} = object
    func0*: proc (a1: cdouble): cdouble
    func1*: proc (a1: pointer; a2: cdouble): cdouble
    func2*: proc (a1: pointer; a2: cdouble; a3: cdouble): cdouble


  AVExpr* {.bycopy.} = object
    `type`*: AVExprEnum
    value*: cdouble            ##  is sign in other types
    const_index*: cint
    a*: INNER_C_UNION_playground_57
    param*: array[3, ptr AVExpr]
    `var`*: ptr cdouble

proc av_clip_c*(a: cint; amin: cint; amax: cint): cint {.inline.} =
  if a < amin:
    return amin
  elif a > amax:
    return amax
  else:
    return a

proc av_clip64_c*(a: auto; amin: auto; amax: auto): int64 {.inline.} =
  if a < amin:
    return amin
  elif a > amax:
    return amax
  else:
    return a

proc av_clip_uint8_c*(a: auto): auto {.inline.} =
  if (a and (not 0x000000FF)) != 0:
    return (not a) shr 31
  else:
    return a

proc av_clip_int8_c*(a: auto): auto {.inline.} =
  if (a + 0x00000080) and not 0x000000FF:
    return (a shr 31) xor 0x0000007F
  else:
    return a

proc av_clip_uint16_c*(a: auto): auto {.inline.} =
  if a and (not 0x0000FFFF):
    return (not a) shr 31
  else:
    return a

proc av_clip_int16_c*(a: auto): auto {.inline.} =
  if (a + 0x00008000) and not 0x0000FFFF:
    return (a shr 31) xor 0x00007FFF
  else:
    return a

proc av_clipl_int32_c*(a: auto): auto {.inline.} =
  if ((a + 0x80000000) and not 0xFFFFFFFF) != 0:
    return (int32)((a shr 63) xor 0x7FFFFFFF)
  else:
    return cast[int32](a)

proc av_clip_intp2_c*(a: cint; p: cint): auto {.inline.} =
  if ((a + (1 shl p)).cint and not ((2 shl p) - 1).cint) != 0:
    return (a shr 31) xor ((1 shl p) - 1)
  else:
    return a

# proc av_clip_uintp2_c*(a: cint; p: cint): auto {.inline.} =
#   if a and not ((1 shl p) - 1):
#     return (not a) shr 31 and ((1 shl p) - 1)
#   else:
#     return a

# proc av_mod_uintp2_c*(a: cuint; p: cuint): auto {.inline.} =
#   return a and ((1 shl p) - 1)

proc av_sat_add32_c*(a: cint; b: cint): auto {.inline.} =
  return av_clipl_int32_c(cast[int64](a) + b)

proc av_sat_dadd32_c*(a: cint; b: cint): auto {.inline.} =
  return av_sat_add32_c(a, av_sat_add32_c(b, b))

proc av_sat_sub32_c*(a: cint; b: cint): auto {.inline.} =
  return av_clipl_int32_c(cast[int64](a) - b)

proc av_sat_dsub32_c*(a: cint; b: cint): auto {.inline.} =
  return av_sat_sub32_c(a, av_sat_add32_c(b, b))

# proc av_sat_add64_c*(a: int64; b: int64): auto {.inline.} =
#   var tmp: int64
#   return if not __builtin_add_overflow(a, b, addr(tmp)): tmp else: (
#       if tmp < 0: 9223372036854775807'i64 else: (-9223372036854775807'i64 - 1))

# proc av_sat_sub64_c*(a: int64; b: int64): auto {.inline.} =
#   var tmp: int64
#   return if not __builtin_sub_overflow(a, b, addr(tmp)): tmp else: (
#       if tmp < 0: 9223372036854775807'i64 else: (-9223372036854775807'i64 - 1))

proc av_clipf_c*(a: cfloat; amin: cfloat; amax: cfloat): auto {.inline.} =
  if a < amin:
    return amin
  elif a > amax:
    return amax
  else:
    return a

proc av_clipd_c*(a: cdouble; amin: cdouble; amax: cdouble): auto {.inline.} =
  if a < amin:
    return amin
  elif a > amax:
    return amax
  else:
    return a

# proc av_ceil_log2_c*(x: cint): auto {.inline.} =
#   return av_log2((x - 1) shl 1)

# proc av_popcount_c*(x: auto): auto {.inline.} =
#   dec(x, (x shr 1) and 0x55555555)
#   x = (x and 0x33333333) + ((x shr 2) and 0x33333333)
#   x = (x + (x shr 4)) and 0x0F0F0F0F
#   inc(x, x shr 8)
#   return (x + (x shr 16)) and 0x0000003F

# proc av_popcount64_c*(x: uint64): cint {.inline.} =
#   return av_popcount_c(cast[uint32](x)) + av_popcount_c((uint32)(x shr 32))

# proc av_parity_c*(v: uint32): cint {.inline.} =
#   return av_popcount_c(v) and 1
# proc av_size_mult*(a: csize_t; b: csize_t; r: ptr csize_t): cint {.inline.} =
#   var t: csize_t = a * b
#   if (a or b) >= (cast[csize_t](1) shl (sizeof(csize_t) * 4))) and a and t div a != b:
#     return -(22)
#   r[] = t
#   return 0

proc av_make_q*(num: cint; den: cint): AVRational {.inline.} = AVRational(num:num, den:den)

proc av_cmp_q*(a: AVRational; b: AVRational): auto {.inline.} =
  var tmp: int64 = a.num * cast[int64](b.den) - b.num * cast[int64](a.den)
  if tmp != 0:
    return (int)((tmp xor a.den xor b.den) shr 63) or 1
  elif b.den != 0 and a.den != 0:
    return 0
  elif a.num != 0 and b.num != 0:
    return (a.num shr 31) - (b.num shr 31)
  else:
    return -0x7FFFFFFF - 1



proc av_strerror*(errnum: cint; errbuf: cstring; errbuf_size: csize_t): cint {.discardable.}
proc av_make_error_string*(errbuf: cstring; errbuf_size: csize_t; errnum: cint): cstring {.
    inline.} =
  av_strerror(errnum, errbuf, errbuf_size)
  return errbuf

proc av_inv_q*(q: AVRational): AVRational {.inline.} = AVRational(num: q.den, den:q.num)



proc av_int2float*(i: uint32): cfloat {.inline.} =
  var v: av_intfloat32
  v.i = i
  return v.f

proc av_float2int*(f: cfloat): uint32 {.inline.} =
  var v: av_intfloat32
  v.f = f
  return v.i

proc av_int2double*(i: uint64): cdouble {.inline.} =
  var v: av_intfloat64
  v.i = i
  return v.f

proc av_double2int*(f: cdouble): uint64 {.inline.} =
  var v: av_intfloat64
  v.f = f
  return v.i








template AV_IS_INPUT_DEVICE*(category: untyped): untyped =
  (((category) == AV_CLASS_CATEGORY_DEVICE_VIDEO_INPUT) or
      ((category) == AV_CLASS_CATEGORY_DEVICE_AUDIO_INPUT) or
      ((category) == AV_CLASS_CATEGORY_DEVICE_INPUT))

template AV_IS_OUTPUT_DEVICE*(category: untyped): untyped =
  (((category) == AV_CLASS_CATEGORY_DEVICE_VIDEO_OUTPUT) or
      ((category) == AV_CLASS_CATEGORY_DEVICE_AUDIO_OUTPUT) or
      ((category) == AV_CLASS_CATEGORY_DEVICE_OUTPUT))



const
  AV_LOG_QUIET* = -8
  AV_LOG_PANIC* = 0
  AV_LOG_FATAL* = 8
  AV_LOG_ERROR* = 16
  AV_LOG_WARNING* = 24
  AV_LOG_INFO* = 32
  AV_LOG_VERBOSE* = 40
  AV_LOG_DEBUG* = 48
  AV_LOG_TRACE* = 56
  AV_LOG_MAX_OFFSET* = (AV_LOG_TRACE - AV_LOG_QUIET)

template AV_LOG_C*(x: untyped): untyped =
  ((x) shl 8)


const
  AV_LOG_SKIP_REPEATED* = 1
  AV_LOG_PRINT_LEVEL* = 2
  AVPALETTE_SIZE* = 1024
  AVPALETTE_COUNT* = 256












macro AV_PIX_FMT_NE(be, le:untyped):untyped = ident "AV_PIX_FMT_" & le.strVal

const
  AV_PIX_FMT_VAAPI = AV_PIX_FMT_VAAPI_VLD
  AV_PIX_FMT_Y400A = AV_PIX_FMT_YA8
  AV_PIX_FMT_GRAY8A = AV_PIX_FMT_YA8
  AV_PIX_FMT_GBR24P = AV_PIX_FMT_GBRP
  AV_PIX_FMT_RGB32* = AV_PIX_FMT_NE(ARGB, BGRA)
  AV_PIX_FMT_RGB32_1* = AV_PIX_FMT_NE(RGBA, ABGR)
  AV_PIX_FMT_BGR32* = AV_PIX_FMT_NE(ABGR, RGBA)
  AV_PIX_FMT_BGR32_1* = AV_PIX_FMT_NE(BGRA, ARGB)
  AV_PIX_FMT_0RGB32* = AV_PIX_FMT_NE(0, BGR0)
  AV_PIX_FMT_0BGR32* = AV_PIX_FMT_NE(0b00000000000000000000000000000000, RGB0)
  AV_PIX_FMT_GRAY9* = AV_PIX_FMT_NE(GRAY9BE, GRAY9LE)
  AV_PIX_FMT_GRAY10* = AV_PIX_FMT_NE(GRAY10BE, GRAY10LE)
  AV_PIX_FMT_GRAY12* = AV_PIX_FMT_NE(GRAY12BE, GRAY12LE)
  AV_PIX_FMT_GRAY14* = AV_PIX_FMT_NE(GRAY14BE, GRAY14LE)
  AV_PIX_FMT_GRAY16* = AV_PIX_FMT_NE(GRAY16BE, GRAY16LE)
  AV_PIX_FMT_YA16* = AV_PIX_FMT_NE(YA16BE, YA16LE)
  AV_PIX_FMT_RGB48* = AV_PIX_FMT_NE(RGB48BE, RGB48LE)
  AV_PIX_FMT_RGB565* = AV_PIX_FMT_NE(RGB565BE, RGB565LE)
  AV_PIX_FMT_RGB555* = AV_PIX_FMT_NE(RGB555BE, RGB555LE)
  AV_PIX_FMT_RGB444* = AV_PIX_FMT_NE(RGB444BE, RGB444LE)
  AV_PIX_FMT_RGBA64* = AV_PIX_FMT_NE(RGBA64BE, RGBA64LE)
  AV_PIX_FMT_BGR48* = AV_PIX_FMT_NE(BGR48BE, BGR48LE)
  AV_PIX_FMT_BGR565* = AV_PIX_FMT_NE(BGR565BE, BGR565LE)
  AV_PIX_FMT_BGR555* = AV_PIX_FMT_NE(BGR555BE, BGR555LE)
  AV_PIX_FMT_BGR444* = AV_PIX_FMT_NE(BGR444BE, BGR444LE)
  AV_PIX_FMT_BGRA64* = AV_PIX_FMT_NE(BGRA64BE, BGRA64LE)
  AV_PIX_FMT_YUV420P9* = AV_PIX_FMT_NE(YUV420P9BE, YUV420P9LE)
  AV_PIX_FMT_YUV422P9* = AV_PIX_FMT_NE(YUV422P9BE, YUV422P9LE)
  AV_PIX_FMT_YUV444P9* = AV_PIX_FMT_NE(YUV444P9BE, YUV444P9LE)
  AV_PIX_FMT_YUV420P10* = AV_PIX_FMT_NE(YUV420P10BE, YUV420P10LE)
  AV_PIX_FMT_YUV422P10* = AV_PIX_FMT_NE(YUV422P10BE, YUV422P10LE)
  AV_PIX_FMT_YUV440P10* = AV_PIX_FMT_NE(YUV440P10BE, YUV440P10LE)
  AV_PIX_FMT_YUV444P10* = AV_PIX_FMT_NE(YUV444P10BE, YUV444P10LE)
  AV_PIX_FMT_YUV420P12* = AV_PIX_FMT_NE(YUV420P12BE, YUV420P12LE)
  AV_PIX_FMT_YUV422P12* = AV_PIX_FMT_NE(YUV422P12BE, YUV422P12LE)
  AV_PIX_FMT_YUV440P12* = AV_PIX_FMT_NE(YUV440P12BE, YUV440P12LE)
  AV_PIX_FMT_YUV444P12* = AV_PIX_FMT_NE(YUV444P12BE, YUV444P12LE)
  AV_PIX_FMT_YUV420P14* = AV_PIX_FMT_NE(YUV420P14BE, YUV420P14LE)
  AV_PIX_FMT_YUV422P14* = AV_PIX_FMT_NE(YUV422P14BE, YUV422P14LE)
  AV_PIX_FMT_YUV444P14* = AV_PIX_FMT_NE(YUV444P14BE, YUV444P14LE)
  AV_PIX_FMT_YUV420P16* = AV_PIX_FMT_NE(YUV420P16BE, YUV420P16LE)
  AV_PIX_FMT_YUV422P16* = AV_PIX_FMT_NE(YUV422P16BE, YUV422P16LE)
  AV_PIX_FMT_YUV444P16* = AV_PIX_FMT_NE(YUV444P16BE, YUV444P16LE)
  AV_PIX_FMT_GBRP9* = AV_PIX_FMT_NE(GBRP9BE, GBRP9LE)
  AV_PIX_FMT_GBRP10* = AV_PIX_FMT_NE(GBRP10BE, GBRP10LE)
  AV_PIX_FMT_GBRP12* = AV_PIX_FMT_NE(GBRP12BE, GBRP12LE)
  AV_PIX_FMT_GBRP14* = AV_PIX_FMT_NE(GBRP14BE, GBRP14LE)
  AV_PIX_FMT_GBRP16* = AV_PIX_FMT_NE(GBRP16BE, GBRP16LE)
  AV_PIX_FMT_GBRAP10* = AV_PIX_FMT_NE(GBRAP10BE, GBRAP10LE)
  AV_PIX_FMT_GBRAP12* = AV_PIX_FMT_NE(GBRAP12BE, GBRAP12LE)
  AV_PIX_FMT_GBRAP16* = AV_PIX_FMT_NE(GBRAP16BE, GBRAP16LE)
  AV_PIX_FMT_BAYER_BGGR16* = AV_PIX_FMT_NE(BAYER_BGGR16BE, BAYER_BGGR16LE)
  AV_PIX_FMT_BAYER_RGGB16* = AV_PIX_FMT_NE(BAYER_RGGB16BE, BAYER_RGGB16LE)
  AV_PIX_FMT_BAYER_GBRG16* = AV_PIX_FMT_NE(BAYER_GBRG16BE, BAYER_GBRG16LE)
  AV_PIX_FMT_BAYER_GRBG16* = AV_PIX_FMT_NE(BAYER_GRBG16BE, BAYER_GRBG16LE)
  AV_PIX_FMT_GBRPF32* = AV_PIX_FMT_NE(GBRPF32BE, GBRPF32LE)
  AV_PIX_FMT_GBRAPF32* = AV_PIX_FMT_NE(GBRAPF32BE, GBRAPF32LE)
  AV_PIX_FMT_GRAYF32* = AV_PIX_FMT_NE(GRAYF32BE, GRAYF32LE)
  AV_PIX_FMT_YUVA420P9* = AV_PIX_FMT_NE(YUVA420P9BE, YUVA420P9LE)
  AV_PIX_FMT_YUVA422P9* = AV_PIX_FMT_NE(YUVA422P9BE, YUVA422P9LE)
  AV_PIX_FMT_YUVA444P9* = AV_PIX_FMT_NE(YUVA444P9BE, YUVA444P9LE)
  AV_PIX_FMT_YUVA420P10* = AV_PIX_FMT_NE(YUVA420P10BE, YUVA420P10LE)
  AV_PIX_FMT_YUVA422P10* = AV_PIX_FMT_NE(YUVA422P10BE, YUVA422P10LE)
  AV_PIX_FMT_YUVA444P10* = AV_PIX_FMT_NE(YUVA444P10BE, YUVA444P10LE)
  AV_PIX_FMT_YUVA422P12* = AV_PIX_FMT_NE(YUVA422P12BE, YUVA422P12LE)
  AV_PIX_FMT_YUVA444P12* = AV_PIX_FMT_NE(YUVA444P12BE, YUVA444P12LE)
  AV_PIX_FMT_YUVA420P16* = AV_PIX_FMT_NE(YUVA420P16BE, YUVA420P16LE)
  AV_PIX_FMT_YUVA422P16* = AV_PIX_FMT_NE(YUVA422P16BE, YUVA422P16LE)
  AV_PIX_FMT_YUVA444P16* = AV_PIX_FMT_NE(YUVA444P16BE, YUVA444P16LE)
  AV_PIX_FMT_XYZ12* = AV_PIX_FMT_NE(XYZ12BE, XYZ12LE)
  AV_PIX_FMT_NV20* = AV_PIX_FMT_NE(NV20BE, NV20LE)
  AV_PIX_FMT_AYUV64* = AV_PIX_FMT_NE(AYUV64BE, AYUV64LE)
  AV_PIX_FMT_P010* = AV_PIX_FMT_NE(P010BE, P010LE)
  AV_PIX_FMT_P016* = AV_PIX_FMT_NE(P016BE, P016LE)
  AV_PIX_FMT_Y210* = AV_PIX_FMT_NE(Y210BE, Y210LE)
  AV_PIX_FMT_X2RGB10* = AV_PIX_FMT_NE(X2RGB10BE, X2RGB10LE)
  AVCOL_PRI_SMPTEST428_1 = AVCOL_PRI_SMPTE428
  AVCOL_PRI_JEDEC_P22 = AVCOL_PRI_EBU3213
  AVCOL_TRC_SMPTEST2084 = AVCOL_TRC_SMPTE2084
  AVCOL_TRC_SMPTEST428_1 = AVCOL_TRC_SMPTE428
  AVCOL_SPC_YCOCG = AVCOL_SPC_YCGCO



proc av_x_if_null*(p: pointer; x: pointer): pointer {.inline.} = 
  if p != nil: p else: x

template av_int_list_length*(list, term: untyped): untyped =
  av_int_list_length_for_size(sizeof(((list)[])), list, term)



const
  AV_FOURCC_MAX_STRING_SIZE* = 32






const
  AVUTIL_PIXDESC_H* = true
  AV_PIX_FMT_FLAG_BE* = (1 shl 0)
  AV_PIX_FMT_FLAG_PAL* = (1 shl 1)
  AV_PIX_FMT_FLAG_BITSTREAM* = (1 shl 2)
  AV_PIX_FMT_FLAG_HWACCEL* = (1 shl 3)
  AV_PIX_FMT_FLAG_PLANAR* = (1 shl 4)
  AV_PIX_FMT_FLAG_RGB* = (1 shl 5)
  AV_PIX_FMT_FLAG_PSEUDOPAL* = (1 shl 6)
  AV_PIX_FMT_FLAG_ALPHA* = (1 shl 7)
  AV_PIX_FMT_FLAG_BAYER* = (1 shl 8)
  AV_PIX_FMT_FLAG_FLOAT* = (1 shl 9)

type
  AVComponentDescriptor* {.bycopy.} = object
    plane*: cint
    step*: cint
    offset*: cint
    shift*: cint
    depth*: cint
    step_minus1*: cint
    depth_minus1*: cint
    offset_plus1*: cint

  AVPixFmtDescriptor* {.bycopy.} = object
    name*: cstring
    nb_components*: uint8    ## /< The number of components each pixel has, (1-4)
    log2_chroma_w*: uint8
    log2_chroma_h*: uint8
    flags*: uint64
    comp*: array[4, AVComponentDescriptor]
    alias*: cstring


const
  FF_LOSS_RESOLUTION* = 0x00000001
  FF_LOSS_DEPTH* = 0x00000002
  FF_LOSS_COLORSPACE* = 0x00000004
  FF_LOSS_ALPHA* = 0x00000008
  FF_LOSS_COLORQUANT* = 0x00000010
  FF_LOSS_CHROMA* = 0x00000020



const
  AV_DICT_MATCH_CASE* = 1
  AV_DICT_IGNORE_SUFFIX* = 2
  AV_DICT_DONT_STRDUP_KEY* = 4
  AV_DICT_DONT_STRDUP_VAL* = 8
  AV_DICT_DONT_OVERWRITE* = 16
  AV_DICT_APPEND* = 32
  AV_DICT_MULTIKEY* = 64


type
  AVSampleFormat* {.size: sizeof(cint).} = enum
    AV_SAMPLE_FMT_NONE = -1, AV_SAMPLE_FMT_U8, ## /< unsigned 8 bits
    AV_SAMPLE_FMT_S16,        ## /< signed 16 bits
    AV_SAMPLE_FMT_S32,        ## /< signed 32 bits
    AV_SAMPLE_FMT_FLT,        ## /< float
    AV_SAMPLE_FMT_DBL,        ## /< double
    AV_SAMPLE_FMT_U8P,        ## /< unsigned 8 bits, planar
    AV_SAMPLE_FMT_S16P,       ## /< signed 16 bits, planar
    AV_SAMPLE_FMT_S32P,       ## /< signed 32 bits, planar
    AV_SAMPLE_FMT_FLTP,       ## /< float, planar
    AV_SAMPLE_FMT_DBLP,       ## /< double, planar
    AV_SAMPLE_FMT_S64,        ## /< signed 64 bits
    AV_SAMPLE_FMT_S64P,       ## /< signed 64 bits, planar
    AV_SAMPLE_FMT_NB          ## /< Number of sample formats. DO NOT USE if linking dynamically



const
  AVUTIL_AVASSERT_H* = true

# template av_assert0*(cond: untyped): void =
#   while true:
#     if not (cond):
#       av_log(nil, AV_LOG_PANIC, "Assertion %s failed at %s:%d",
#              AV_STRINGIFY(cond), __FILE__, __LINE__)
#       abort()
#     if not 0:
#       break

template av_assert1*(cond: untyped): untyped =
  (cast[nil](0))

template av_assert2*(cond: untyped): untyped =
  (cast[nil](0))

template av_assert2_fpu*(): untyped =
  (cast[nil](0))



const
  AVUTIL_TIME_H* = true
  AVUTIL_BPRINT_H* = true
  AV_BPRINT_SIZE_UNLIMITED* = uint64.high
  AV_BPRINT_SIZE_AUTOMATIC* = 1
  AV_BPRINT_SIZE_COUNT_ONLY* = 0
##  #define FF_PAD_STRUCTURE(name, size, ...)                                              \
##      struct ff_pad_helper_##name                                                        \
##      {                                                                                  \
##          __VA_ARGS__                                                                    \
##      };                                                                                 \
##      typedef struct name                                                                \
##      {                                                                                  \
##          __VA_ARGS__ char reserved_padding[size - sizeof(struct ff_pad_helper_##name)]; \
##      } name;

type
  AVBuffer* {.bycopy.} = object
    data*: ptr uint8          ## *< data described by this buffer
    size*: cint                ## *< size of data in bytes
    refcount*: int64
    free*: proc (opaque: pointer; data: ptr uint8)
    opaque*: pointer
    flags*: cint
    flags_internal*: cint


  ff_pad_helper_AVBPrint* {.bycopy.} = object
    str*: cstring
    len*: cuint
    size*: cuint
    size_max*: cuint
    reserved_internal_buffer*: array[1, char]

  AVBPrint* {.bycopy.} = object
    str*: cstring
    len*: cuint
    size*: cuint
    size_max*: cuint
    reserved_internal_buffer*: array[1, char]
    reserved_padding*: array[1024 - sizeof(ff_pad_helper_AVBPrint), char]

  tm* {.bycopy.} = object
  AVBufferPool* {.bycopy.} = object
  AVBufferRef* {.bycopy.} = object
    buffer*: ptr AVBuffer
    data*: ptr uint8
    size*: cint




proc av_strstart*(str: cstring; pfx: cstring; `ptr`: cstringArray): cint
proc av_stristart*(str: cstring; pfx: cstring; `ptr`: cstringArray): cint
proc av_stristr*(haystack: cstring; needle: cstring): cstring
proc av_strnstr*(haystack: cstring; needle: cstring; hay_length: csize_t): cstring
proc av_strlcpy*(dst: cstring; src: cstring; size: csize_t): csize_t
proc av_strlcat*(dst: cstring; src: cstring; size: csize_t): csize_t
proc av_strlcatf*(dst: cstring; size: csize_t; fmt: cstring): csize_t {.varargs.}
proc av_asprintf*(fmt: cstring): cstring {.varargs.}
proc av_d2str*(d: cdouble): cstring
proc av_get_token*(buf: cstringArray; term: cstring): cstring
proc av_strtok*(s: cstring; delim: cstring; saveptr: cstringArray): cstring
proc av_strcasecmp*(a: cstring; b: cstring): cint
proc av_strncasecmp*(a: cstring; b: cstring; n: csize_t): cint
proc av_strireplace*(str: cstring; `from`: cstring; to: cstring): cstring
proc av_basename*(path: cstring): cstring
proc av_dirname*(path: cstring): cstring
proc av_match_name*(name: cstring; names: cstring): cint
proc av_append_path_component*(path: cstring; component: cstring): cstring
proc av_escape*(dst: cstringArray; src: cstring; special_chars: cstring;
               mode: AVEscapeMode; flags: cint): cint
               
proc av_utf8_decode*(codep: ptr int32; bufp: ptr ptr uint8; buf_end: ptr uint8;
                    flags: cuint): cint
proc av_match_list*(name: cstring; list: cstring; separator: char): cint
proc av_sscanf*(string: cstring; format: cstring): cint {.varargs.}
proc avutil_version*(): cuint
proc av_version_info*(): cstring
proc avutil_configuration*(): cstring
proc avutil_license*(): cstring
proc av_get_media_type_string*(media_type: auto): cstring
proc av_bprint_init*(buf: ptr AVBPrint; size_init: cuint; size_max: cuint)
proc av_bprint_init_for_buffer*(buf: ptr AVBPrint; buffer: cstring; size: cuint)
proc av_bprintf*(buf: ptr AVBPrint; fmt: cstring) {.varargs.}
proc av_vbprintf*(buf: ptr AVBPrint; fmt: cstring; vl_arg: varargs[untyped])
proc av_bprint_chars*(buf: ptr AVBPrint; c: char; n: cuint)
proc av_bprint_append_data*(buf: ptr AVBPrint; data: cstring; size: cuint)
## define av_fourcc2str(fourcc) av_fourcc_make_string((char[AV_FOURCC_MAX_STRING_SIZE]){0}, fourcc)
proc av_log_set_flags*(arg: cint)
proc av_log_get_flags*(): cint
proc av_int_list_length_for_size*(elsize: cuint; list: pointer; term: uint64): cuint
proc av_fopen_utf8*(path: cstring; mode: cstring): ptr FILE
proc av_get_time_base_q*(): AVRational
proc av_log2*(v: auto): cint
proc av_log2_16bit*(v: auto): cint

proc av_malloc*(size: auto): pointer {.importc, dynlib:"avutil(-56).dll".}
proc av_mallocz*(size: auto): pointer {.importc.}
proc av_malloc_array*(nmemb: auto; size: auto): pointer
proc av_mallocz_array*(nmemb: auto; size: auto): pointer
proc av_calloc*(nmemb: auto; size: auto): pointer
proc av_realloc*(`ptr`: pointer; size: auto): pointer
proc av_reallocp*(`ptr`: pointer; size: auto): cint
proc av_realloc_f*(`ptr`: pointer; nelem: auto; elsize: auto): pointer
proc av_realloc_array*(`ptr`: pointer; nmemb: auto; size: auto): pointer
proc av_reallocp_array*(`ptr`: pointer; nmemb: auto; size: auto): cint
proc av_fast_realloc*(`ptr`: pointer; size: ptr cuint; min_size: auto): pointer
proc av_fast_malloc*(`ptr`: pointer; size: ptr cuint; min_size: auto)
proc av_fast_mallocz*(`ptr`: pointer; size: ptr cuint; min_size: auto)
proc av_free*(`ptr`: pointer)
proc av_freep*(`ptr`: pointer)
proc av_strdup*(s: cstring): cstring
proc av_strndup*(s: cstring; len: auto): cstring
proc av_memdup*(p: pointer; size: auto): pointer
proc av_memcpy_backptr*(dst: ptr uint8; back: auto; cnt: auto)
proc av_dynarray_add*(tab_ptr: pointer; nb_ptr: ptr cint; elem: pointer)
proc av_dynarray_add_nofree*(tab_ptr: pointer; nb_ptr: ptr cint; elem: pointer): cint
proc av_dynarray2_add*(tab_ptr: ptr pointer; nb_ptr: ptr cint; elem_size: csize_t; elem_data: ptr uint8): pointer
proc av_max_alloc*(max: auto)
proc av_q2d*(a: AVRational): auto {.inline.} = a.num div a.den
proc av_reduce*(dst_num: ptr cint; dst_den: ptr cint; num: int64; den: int64;max: int64): cint
proc av_mul_q*(b: AVRational; c: AVRational): AVRational
proc av_div_q*(b: AVRational; c: AVRational): AVRational
proc av_add_q*(b: AVRational; c: AVRational): AVRational
proc av_sub_q*(b: AVRational; c: AVRational): AVRational
proc av_d2q*(d: cdouble; max: cint): AVRational
proc av_nearer_q*(q: AVRational; q1: AVRational; q2: AVRational): cint
proc av_find_nearest_q_idx*(q: AVRational; q_list: ptr AVRational): cint
proc av_q2intfloat*(q: AVRational): uint32
proc av_gcd_q*(a: AVRational; b: AVRational; max_den: cint; def: AVRational): AVRational
proc av_gcd*(a: int64; b: int64): int64
proc av_rescale*(a: int64; b: int64; c: int64): int64
proc av_rescale_rnd*(a: int64; b: int64; c: int64; rnd: AVRounding): int64
proc av_rescale_q*(a: int64; bq: AVRational; cq: AVRational): int64
proc av_rescale_q_rnd*(a: int64; bq: AVRational; cq: AVRational; rnd: AVRounding): int64
proc av_compare_ts*(ts_a: int64; tb_a: AVRational; ts_b: int64; tb_b: AVRational): cint
proc av_compare_mod*(a: uint64; b: uint64; `mod`: uint64): int64
proc av_rescale_delta*(in_tb: AVRational; in_ts: int64; fs_tb: AVRational; duration: cint; last: ptr int64; out_tb: AVRational): int64
proc av_add_stable*(ts_tb: AVRational; ts: int64; inc_tb: AVRational; inc: int64): int64
proc av_log*(avcl: pointer; level: cint; fmt: cstring) {.varargs.}
proc av_log_once*(avcl: pointer; initial_level: cint; subsequent_level: cint; state: ptr cint; fmt: cstring) {.varargs.}
proc av_vlog*(avcl: pointer; level: cint; fmt: cstring; vl: varargs[untyped])
proc av_log_get_level*(): cint
proc av_log_set_level*(level: cint)
proc av_log_set_callback*(callback: proc (a1: pointer; a2: cint; a3: cstring; a4: varargs[untyped]))
proc av_log_default_callback*(avcl: pointer; level: cint; fmt: cstring; vl: varargs[untyped])
proc av_default_item_name*(ctx: pointer): cstring
proc av_default_get_category*(`ptr`: pointer): AVClassCategory
proc av_log_format_line*(`ptr`: pointer; level: cint; fmt: cstring; vl: varargs[untyped]; line: cstring; line_size: cint; print_prefix: ptr cint)
proc av_log_format_line2*(`ptr`: pointer; level: cint; fmt: cstring; vl: varargs[untyped];line: cstring; line_size: cint; print_prefix: ptr cint): cint
proc av_fourcc_make_string*(buf: cstring; fourcc: uint32): cstring
proc av_expr_parse_and_eval*(res: ptr cdouble; s: cstring; const_names: cstringArray;const_values: ptr cdouble; func1_names: cstringArray; funcs1: proc (a1: pointer; a2: cdouble): cdouble; 
          func2_names: cstringArray; funcs2: proc (a1: pointer; a2: cdouble; a3: cdouble): cdouble; opaque: pointer; log_offset: cint; log_ctx: pointer): cint
proc av_expr_parse*(expr: ptr ptr AVExpr; s: cstring; const_names: cstringArray;
                   func1_names: cstringArray;
                   funcs1: proc (a1: pointer; a2: cdouble): cdouble;
                   func2_names: cstringArray; funcs2: proc (a1: pointer; a2: cdouble;a3: cdouble): cdouble; 
                   log_offset: cint; log_ctx: pointer): cint
proc av_expr_eval*(e: ptr AVExpr; const_values: ptr cdouble; opaque: pointer): cdouble
proc av_expr_count_vars*(e: ptr AVExpr; counter: ptr cuint; size: cint): cint
proc av_expr_count_func*(e: ptr AVExpr; counter: ptr cuint; size: cint; arg: cint): cint
proc av_expr_free*(e: ptr AVExpr)
proc av_strtod*(numstr: cstring; tail: cstringArray): cdouble
proc av_get_bits_per_pixel*(pixdesc: ptr AVPixFmtDescriptor): cint
proc av_get_padded_bits_per_pixel*(pixdesc: ptr AVPixFmtDescriptor): cint
proc av_pix_fmt_desc_get*(pix_fmt: AVPixelFormat): ptr AVPixFmtDescriptor
proc av_pix_fmt_desc_next*(prev: ptr AVPixFmtDescriptor): ptr AVPixFmtDescriptor
proc av_pix_fmt_desc_get_id*(desc: ptr AVPixFmtDescriptor): AVPixelFormat
proc av_pix_fmt_get_chroma_sub_sample*(pix_fmt: AVPixelFormat; h_shift: ptr cint;
                                      v_shift: ptr cint): cint
proc av_pix_fmt_count_planes*(pix_fmt: AVPixelFormat): cint
proc av_color_range_name*(range: AVColorRange): cstring
proc av_color_range_from_name*(name: cstring): cint
proc av_color_primaries_name*(primaries: AVColorPrimaries): cstring
proc av_color_primaries_from_name*(name: cstring): cint
proc av_color_transfer_name*(transfer: AVColorTransferCharacteristic): cstring
proc av_color_transfer_from_name*(name: cstring): cint
proc av_color_space_name*(space: AVColorSpace): cstring
proc av_color_space_from_name*(name: cstring): cint
proc av_chroma_location_name*(location: AVChromaLocation): cstring
proc av_chroma_location_from_name*(name: cstring): cint
proc av_get_pix_fmt*(name: cstring): AVPixelFormat
proc av_get_pix_fmt_name*(pix_fmt: AVPixelFormat): cstring
proc av_get_pix_fmt_string*(buf: cstring; buf_size: cint; pix_fmt: AVPixelFormat): cstring
proc av_read_image_line2*(dst: pointer; data: array[4, ptr uint8];
                         linesize: array[4, cint]; desc: ptr AVPixFmtDescriptor;
                         x: cint; y: cint; c: cint; w: cint; read_pal_component: cint;
                         dst_element_size: cint)
proc av_read_image_line*(dst: ptr uint16; data: array[4, ptr uint8];
                        linesize: array[4, cint]; desc: ptr AVPixFmtDescriptor;
                        x: cint; y: cint; c: cint; w: cint; read_pal_component: cint)
proc av_write_image_line2*(src: pointer; data: array[4, ptr uint8];
                          linesize: array[4, cint]; desc: ptr AVPixFmtDescriptor;
                          x: cint; y: cint; c: cint; w: cint; src_element_size: cint)
proc av_write_image_line*(src: ptr uint16; data: array[4, ptr uint8];
                         linesize: array[4, cint]; desc: ptr AVPixFmtDescriptor;
                         x: cint; y: cint; c: cint; w: cint)
proc av_pix_fmt_swap_endianness*(pix_fmt: AVPixelFormat): AVPixelFormat
proc av_get_pix_fmt_loss*(dst_pix_fmt: AVPixelFormat; src_pix_fmt: AVPixelFormat;
                         has_alpha: cint): cint
proc av_find_best_pix_fmt_of_2*(dst_pix_fmt1: AVPixelFormat;
                               dst_pix_fmt2: AVPixelFormat;
                               src_pix_fmt: AVPixelFormat; has_alpha: cint;
                               loss_ptr: ptr cint): AVPixelFormat
proc av_image_fill_max_pixsteps*(max_pixsteps: array[4, cint];
                                max_pixstep_comps: array[4, cint];
                                pixdesc: ptr AVPixFmtDescriptor)
proc av_image_get_linesize*(pix_fmt: AVPixelFormat; width: cint; plane: cint): cint
proc av_image_fill_linesizes*(linesizes: array[4, cint]; pix_fmt: AVPixelFormat;
                             width: cint): cint
proc av_image_fill_plane_sizes*(size: array[4, csize_t]; pix_fmt: AVPixelFormat;
                               height: cint; linesizes: array[4, int64]): cint
proc av_image_fill_pointers*(data: array[4, ptr uint8]; pix_fmt: AVPixelFormat;
                            height: cint; `ptr`: ptr uint8;
                            linesizes: array[4, cint]): cint
proc av_image_alloc*(pointers: array[4, ptr uint8]; linesizes: array[4, cint];
                    w: cint; h: cint; pix_fmt: AVPixelFormat; align: cint): cint
proc av_image_copy_plane*(dst: ptr uint8; dst_linesize: cint; src: ptr uint8;
                         src_linesize: cint; bytewidth: cint; height: cint)
proc av_image_copy*(dst_data: array[4, ptr uint8]; dst_linesizes: array[4, cint];
                   src_data: array[4, ptr uint8]; src_linesizes: array[4, cint];
                   pix_fmt: AVPixelFormat; width: cint; height: cint)
proc av_image_copy_uc_from*(dst_data: array[4, ptr uint8];
                           dst_linesizes: array[4, int64];
                           src_data: array[4, ptr uint8];
                           src_linesizes: array[4, int64];
                           pix_fmt: AVPixelFormat; width: cint; height: cint)
proc av_image_fill_arrays*(dst_data: array[4, ptr uint8];
                          dst_linesize: array[4, cint]; src: ptr uint8;
                          pix_fmt: AVPixelFormat; width: cint; height: cint;
                          align: cint): cint
proc av_image_get_buffer_size*(pix_fmt: AVPixelFormat; width: cint; height: cint;
                              align: cint): cint
proc av_image_copy_to_buffer*(dst: ptr uint8; dst_size: cint;
                             src_data: array[4, ptr uint8];
                             src_linesize: array[4, cint]; pix_fmt: AVPixelFormat;
                             width: cint; height: cint; align: cint): cint
proc av_image_check_size*(w: cuint; h: cuint; log_offset: cint; log_ctx: pointer): cint
proc av_image_check_size2*(w: cuint; h: cuint; max_pixels: int64;pix_fmt: AVPixelFormat; log_offset: cint; log_ctx: pointer): cint
proc av_image_check_sar*(w: cuint; h: cuint; sar: AVRational): cint
proc av_image_fill_black*(dst_data: array[4, ptr uint8];dst_linesize: array[4, int64]; pix_fmt: AVPixelFormat; range: AVColorRange; width: cint; height: cint): cint
proc av_dict_get*(m: ptr AVDictionary; key: cstring; prev: ptr AVDictionaryEntry;flags: cint): ptr AVDictionaryEntry
proc av_dict_count*(m: ptr AVDictionary): cint {.discardable, importc, dynlib:"avutil(-56).dll".}

proc av_dict_set*(pm: ptr ptr AVDictionary; key: cstring; value: cstring; flags: cint): cint {.discardable, importc, dynlib:"avutil(-56).dll".}

proc av_dict_set_int*(pm: ptr ptr AVDictionary; key: cstring; value: int64; flags: cint): cint {.discardable.}
proc av_dict_parse_string*(pm: ptr ptr AVDictionary; str: cstring; key_val_sep: cstring; pairs_sep: cstring; flags: cint): cint {.discardable.}
proc av_dict_copy*(dst: ptr ptr AVDictionary; src: ptr AVDictionary; flags: cint): cint {.discardable.}
proc av_dict_free*(m: ptr ptr AVDictionary) 
proc av_dict_get_string*(m: ptr AVDictionary; buffer: cstringArray; key_val_sep: char;pairs_sep: char): cint {.discardable.}
proc av_parse_ratio*(q: ptr AVRational; str: cstring; max: cint; log_offset: cint; log_ctx: pointer): cint
template av_parse_ratio_quiet*(rate, str, max: untyped): untyped =
  av_parse_ratio(rate, str, max, AV_LOG_MAX_OFFSET, nil)

proc av_parse_video_size*(width_ptr: ptr cint; height_ptr: ptr cint; str: cstring): cint
proc av_parse_video_rate*(rate: ptr AVRational; str: cstring): cint
proc av_parse_color*(rgba_color: ptr uint8; color_string: cstring; slen: cint;log_ctx: pointer): cint
proc av_get_known_color_name*(color_idx: cint; rgb: ptr ptr uint8): cstring
proc av_parse_time*(timeval: ptr int64; timestr: cstring; duration: cint): cint
proc av_find_info_tag*(arg: cstring; arg_size: cint; tag1: cstring; info: cstring): cint
proc av_small_strptime*(p: cstring; fmt: cstring; dt: ptr tm): cstring
proc av_timegm*(tm: ptr tm): int64
proc av_get_sample_fmt_name*(sample_fmt: AVSampleFormat): cstring
proc av_get_sample_fmt*(name: cstring): AVSampleFormat
proc av_get_alt_sample_fmt*(sample_fmt: AVSampleFormat; planar: cint): AVSampleFormat
proc av_get_packed_sample_fmt*(sample_fmt: AVSampleFormat): AVSampleFormat
proc av_get_planar_sample_fmt*(sample_fmt: AVSampleFormat): AVSampleFormat
proc av_get_sample_fmt_string*(buf: cstring; buf_size: cint; sample_fmt: AVSampleFormat): cstring
proc av_get_bytes_per_sample*(sample_fmt: AVSampleFormat): cint
proc av_sample_fmt_is_planar*(sample_fmt: AVSampleFormat): cint
proc av_samples_get_buffer_size*(linesize: ptr cint; nb_channels: cint;
                                nb_samples: cint; sample_fmt: AVSampleFormat;
                                align: cint): cint
proc av_samples_fill_arrays*(audio_data: ptr ptr uint8; linesize: ptr cint;
                            buf: ptr uint8; nb_channels: cint; nb_samples: cint;
                            sample_fmt: AVSampleFormat; align: cint): cint
proc av_samples_alloc*(audio_data: ptr ptr uint8; linesize: ptr cint;
                      nb_channels: cint; nb_samples: cint;
                      sample_fmt: AVSampleFormat; align: cint): cint
proc av_samples_alloc_array_and_samples*(audio_data: ptr ptr ptr uint8;
                                        linesize: ptr cint; nb_channels: cint;
                                        nb_samples: cint;
                                        sample_fmt: AVSampleFormat; align: cint): cint
proc av_samples_copy*(dst: ptr ptr uint8; src: ptr ptr uint8; dst_offset: cint;
                     src_offset: cint; nb_samples: cint; nb_channels: cint;
                     sample_fmt: AVSampleFormat): cint
proc av_samples_set_silence*(audio_data: ptr ptr uint8; offset: cint;
                            nb_samples: cint; nb_channels: cint;
                            sample_fmt: AVSampleFormat): cint
proc av_assert0_fpu*()
proc av_gettime*(): int64
proc av_gettime_relative*(): int64
proc av_gettime_relative_is_monotonic*(): cint
proc av_usleep*(usec: auto): cint {.discardable.}
proc av_buffer_alloc*(size: cint): ptr AVBufferRef
proc av_buffer_allocz*(size: cint): ptr AVBufferRef
proc av_bprint_strftime*(buf: ptr AVBPrint; fmt: cstring; tm: ptr tm)
proc av_bprint_get_buffer*(buf: ptr AVBPrint; size: cuint; mem: ptr ptr cuchar;
                          actual_size: ptr cuint)
proc av_bprint_clear*(buf: ptr AVBPrint)

proc av_bprint_finalize*(buf: ptr AVBPrint; ret_str: cstringArray): cint
proc av_bprint_escape*(dstbuf: ptr AVBPrint; src: cstring; special_chars: cstring;
                      mode: AVEscapeMode; flags: cint)
proc av_buffer_create*(data: ptr uint8; size: cint;
                      free: proc (opaque: pointer; data: ptr uint8); opaque: pointer;
                      flags: cint): ptr AVBufferRef
proc av_buffer_default_free*(opaque: pointer; data: ptr uint8)
proc av_buffer_ref*(buf: ptr AVBufferRef): ptr AVBufferRef
proc av_buffer_unref*(buf: ptr ptr AVBufferRef)
proc av_buffer_is_writable*(buf: ptr AVBufferRef): cint
proc av_buffer_get_opaque*(buf: ptr AVBufferRef): pointer
proc av_buffer_get_ref_count*(buf: ptr AVBufferRef): cint
proc av_buffer_make_writable*(buf: ptr ptr AVBufferRef): cint
proc av_buffer_realloc*(buf: ptr ptr AVBufferRef; size: cint): cint
proc av_buffer_replace*(dst: ptr ptr AVBufferRef; src: ptr AVBufferRef): cint

proc av_buffer_pool_init*(size: cint; alloc: proc (size: cint): ptr AVBufferRef): ptr AVBufferPool
proc av_buffer_pool_init2*(size: cint; opaque: pointer; alloc: proc (opaque: pointer;
    size: cint): ptr AVBufferRef; pool_free: proc (opaque: pointer)): ptr AVBufferPool
proc av_buffer_pool_uninit*(pool: ptr ptr AVBufferPool)
proc av_buffer_pool_get*(pool: ptr AVBufferPool): ptr AVBufferRef
proc av_buffer_pool_buffer_get_opaque*(`ref`: ptr AVBufferRef): pointer
proc av_get_cpu_flags*(): cint
proc av_force_cpu_flags*(flags: cint)
proc av_set_cpu_flags_mask*(mask: cint)
proc av_parse_cpu_flags*(s: cstring): cint
proc av_parse_cpu_caps*(flags: ptr cuint; s: cstring): cint
proc av_cpu_count*(): cint
proc av_cpu_max_align*(): csize_t


proc av_bprint_is_complete*(buf: ptr AVBPrint): auto {.inline.} =
  return buf.len < buf.size

template av_opt_set_int_list*(obj, name, val, term, flags: untyped): untyped =
  (if av_int_list_length(val, term) > INT_MAX div sizeof(((val)[])): AVERROR(EINVAL) else: av_opt_set_bin(
      obj, name, cast[ptr uint8]((val)),
      av_int_list_length(val, term) * sizeof(((val)[])), flags))

const
  AV_OPT_SERIALIZE_SKIP_DEFAULTS* = 0x00000001
  AV_OPT_SERIALIZE_OPT_FLAGS_EXACT* = 0x00000002
  LIBSWSCALE_VERSION_MAJOR* = 5
  LIBSWSCALE_VERSION_MINOR* = 8
  LIBSWSCALE_VERSION_MICRO* = 100
  LIBSWSCALE_VERSION_INT* = AV_VERSION_INT(LIBSWSCALE_VERSION_MAJOR,LIBSWSCALE_VERSION_MINOR, LIBSWSCALE_VERSION_MICRO)
  # LIBSWSCALE_VERSION* = AV_VERSION(LIBSWSCALE_VERSION_MAJOR,LIBSWSCALE_VERSION_MINOR,LIBSWSCALE_VERSION_MICRO)
  LIBSWSCALE_BUILD* = LIBSWSCALE_VERSION_INT
  #LIBSWSCALE_IDENT = "SwS" AV_STRINGIFY(LIBSWSCALE_VERSION)
  FF_API_SWS_VECTOR* = (LIBSWSCALE_VERSION_MAJOR < 6)
  SWS_FAST_BILINEAR* = 1
  SWS_BILINEAR* = 2
  SWS_BICUBIC* = 4
  SWS_X* = 8
  SWS_POINT* = 0x00000010
  SWS_AREA* = 0x00000020
  SWS_BICUBLIN* = 0x00000040
  SWS_GAUSS* = 0x00000080
  SWS_SINC* = 0x00000100
  SWS_LANCZOS* = 0x00000200
  SWS_SPLINE* = 0x00000400
  SWS_SRC_V_CHR_DROP_MASK* = 0x00030000
  SWS_SRC_V_CHR_DROP_SHIFT* = 16
  SWS_PARAM_DEFAULT* = 123456
  SWS_PRINT_INFO* = 0x00001000
  SWS_FULL_CHR_H_INT* = 0x00002000
  SWS_FULL_CHR_H_INP* = 0x00004000
  SWS_DIRECT_BGR* = 0x00008000
  SWS_ACCURATE_RND* = 0x00040000
  SWS_BITEXACT* = 0x00080000
  SWS_ERROR_DIFFUSION* = 0x00800000
  SWS_MAX_REDUCE_CUTOFF* = 0.002
  SWS_CS_ITU709* = 1
  SWS_CS_FCC* = 4
  SWS_CS_ITU601* = 5
  SWS_CS_ITU624* = 5
  SWS_CS_SMPTE170M* = 5
  SWS_CS_SMPTE240M* = 7
  SWS_CS_DEFAULT* = 5
  SWS_CS_BT2020* = 9
  AVCODEC_AVFFT_H* = true
  LIBSWRESAMPLE_VERSION_MAJOR* = 3
  LIBSWRESAMPLE_VERSION_MINOR* = 8
  LIBSWRESAMPLE_VERSION_MICRO* = 100
  LIBSWRESAMPLE_VERSION_INT* = AV_VERSION_INT(LIBSWRESAMPLE_VERSION_MAJOR,LIBSWRESAMPLE_VERSION_MINOR, LIBSWRESAMPLE_VERSION_MICRO)
  # LIBSWRESAMPLE_VERSION* = AV_VERSION(LIBSWRESAMPLE_VERSION_MAJOR,LIBSWRESAMPLE_VERSION_MINOR,LIBSWRESAMPLE_VERSION_MICRO)
  LIBSWRESAMPLE_BUILD* = LIBSWRESAMPLE_VERSION_INT
  #LIBSWRESAMPLE_IDENT = "SwR" AV_STRINGIFY(LIBSWRESAMPLE_VERSION)
  SWR_FLAG_RESAMPLE* = 1
  LIBAVFILTER_VERSION_MAJOR* = 7
  LIBAVFILTER_VERSION_MINOR* = 88
  LIBAVFILTER_VERSION_MICRO* = 100
  LIBAVFILTER_VERSION_INT* = AV_VERSION_INT(LIBAVFILTER_VERSION_MAJOR,LIBAVFILTER_VERSION_MINOR, LIBAVFILTER_VERSION_MICRO)
  # LIBAVFILTER_VERSION* = AV_VERSION(LIBAVFILTER_VERSION_MAJOR,LIBAVFILTER_VERSION_MINOR,LIBAVFILTER_VERSION_MICRO)
  LIBAVFILTER_BUILD* = LIBAVFILTER_VERSION_INT
  #LIBAVFILTER_IDENT = "Lavfi" AV_STRINGIFY(LIBAVFILTER_VERSION)
  FF_API_OLD_FILTER_OPTS_ERROR* = (LIBAVFILTER_VERSION_MAJOR < 8)
  FF_API_LAVR_OPTS* = (LIBAVFILTER_VERSION_MAJOR < 8)
  FF_API_FILTER_GET_SET* = (LIBAVFILTER_VERSION_MAJOR < 8)
  FF_API_SWS_PARAM_OPTION* = (LIBAVFILTER_VERSION_MAJOR < 8)
  AVFILTER_FLAG_DYNAMIC_INPUTS* = (1 shl 0)
  AVFILTER_FLAG_DYNAMIC_OUTPUTS* = (1 shl 1)
  AVFILTER_FLAG_SLICE_THREADS* = (1 shl 2)
  AVFILTER_FLAG_SUPPORT_TIMELINE_GENERIC* = (1 shl 16)
  AVFILTER_FLAG_SUPPORT_TIMELINE_INTERNAL* = (1 shl 17)
  AVFILTER_FLAG_SUPPORT_TIMELINE* = (AVFILTER_FLAG_SUPPORT_TIMELINE_GENERIC or AVFILTER_FLAG_SUPPORT_TIMELINE_INTERNAL)
  AVFILTER_THREAD_SLICE* = (1 shl 0)
  AVFILTER_CMD_FLAG_ONE* = 1
  AVFILTER_CMD_FLAG_FAST* = 2
  AVFILTER_BUFFERSINK_H* = true
  AV_BUFFERSINK_FLAG_PEEK* = 1
  AV_BUFFERSINK_FLAG_NO_REQUEST* = 2
  AVFILTER_BUFFERSRC_H* = true
  HAVE_WINAPIFAMILY_H* = 0
  WINAPI_FAMILY_WINRT* = 0
  # SDL_DEPRECATED* = __attribute__((deprecated))
  # SDL_UNUSED* = __attribute__((unused))
  # DECLSPEC* = __declspec(dllexport)
  # SDLCALL* = true
  # SDL_INLINE* = __inline__

##  #define SDL_FORCE_INLINE __attribute__((always_inline)) static __inline__
##  #define SDL_NORETURN __attribute__((noreturn))

const
  HAVE_STDINT_H* = 1
  SIZEOF_VOIDP* = 8
  HAVE_DDRAW_H* = 1
  HAVE_DINPUT_H* = 1
  HAVE_DSOUND_H* = 1
  HAVE_DXGI_H* = 1
  HAVE_XINPUT_H* = 1
  HAVE_MMDEVICEAPI_H* = 1
  HAVE_AUDIOCLIENT_H* = 1
  HAVE_STDARG_H* = 1
  HAVE_STDDEF_H* = 1
  SDL_AUDIO_DRIVER_WASAPI* = 1
  SDL_AUDIO_DRIVER_DSOUND* = 1
  SDL_AUDIO_DRIVER_WINMM* = 1
  SDL_AUDIO_DRIVER_DISK* = 1
  SDL_AUDIO_DRIVER_DUMMY* = 1
  SDL_JOYSTICK_DINPUT* = 1
  SDL_JOYSTICK_XINPUT* = 1
  SDL_JOYSTICK_HIDAPI* = 1
  SDL_HAPTIC_DINPUT* = 1
  SDL_HAPTIC_XINPUT* = 1
  SDL_SENSOR_DUMMY* = 1
  SDL_LOADSO_WINDOWS* = 1
  SDL_THREAD_WINDOWS* = 1
  SDL_TIMER_WINDOWS* = 1
  SDL_VIDEO_DRIVER_DUMMY* = 1
  SDL_VIDEO_DRIVER_WINDOWS* = 1
  SDL_VIDEO_RENDER_D3D* = 1
  SDL_VIDEO_RENDER_D3D11* = 0
  SDL_VIDEO_OPENGL* = 1
  SDL_VIDEO_OPENGL_WGL* = 1
  SDL_VIDEO_RENDER_OGL* = 1
  SDL_VIDEO_RENDER_OGL_ES2* = 1
  SDL_VIDEO_OPENGL_ES2* = 1
  SDL_VIDEO_OPENGL_EGL* = 1
  SDL_VIDEO_VULKAN* = 1
  SDL_POWER_WINDOWS* = 1
  SDL_FILESYSTEM_WINDOWS* = 1

template SDL_arraysize*(array: untyped): untyped =
  (sizeof((array) div sizeof((array[0]))))

template SDL_TABLESIZE*(table: untyped): untyped =
  SDL_arraysize(table)

template SDL_STRINGIFY_ARG(arg)  = $arg

template SDL_reinterpret_cast*(`type`, expression: untyped): untyped =
  ((`type`)(expression))

template SDL_static_cast*(`type`, expression: untyped): untyped =
  ((`type`)(expression))

template SDL_const_cast*(`type`, expression: untyped): untyped =
  ((`type`)(expression))

template SDL_FOURCC*(A, B, C, D: untyped): untyped =
  ((SDL_static_cast(uint32, SDL_static_cast(uint8, (A))) shl 0) or
      (SDL_static_cast(uint32, SDL_static_cast(uint8, (B))) shl 8) or
      (SDL_static_cast(uint32, SDL_static_cast(uint8, (C))) shl 16) or
      (SDL_static_cast(uint32, SDL_static_cast(uint8, (D))) shl 24))

const
  SDL_MAX_SINT8* = (cast[int8](0x0000007F))
  SDL_MIN_SINT8* = ((int8)(not 0x0000007F))
  SDL_MAX_UINT8* = (cast[uint8](0x000000FF))
  SDL_MIN_UINT8* = (cast[uint8](0x00000000))
  SDL_MAX_SINT16* = (cast[int16](0x00007FFF))
  SDL_MIN_SINT16* = ((int16)(not 0x00007FFF))
  SDL_MAX_UINT16* = (cast[uint16](0x0000FFFF))
  SDL_MIN_UINT16* = (cast[uint16](0x00000000))
  SDL_MAX_SINT32* = (cast[int32](0x7FFFFFFF))
  SDL_MIN_SINT32* = ((int32)(not 0x7FFFFFFF))
  SDL_MAX_UINT32* = (cast[uint32](0xFFFFFFFF))
  SDL_MIN_UINT32* = (cast[uint32](0x00000000))
  SDL_MAX_SINT64* = int64.high
  SDL_MIN_SINT64* = int64.low
  SDL_MAX_UINT64 = uint64.high
  SDL_MIN_UINT64* = uint64.low
  # SDL_PRIs64* = "I64d"
  # SDL_PRIu64* = PRIu64
  # SDL_PRIx64* = PRIx64
  # SDL_PRIX64* = PRIX64

template SDL_IN_BYTECAP*(x: untyped): void =
  nil

template SDL_INOUT_Z_CAP*(x: untyped): void =
  nil

template SDL_OUT_Z_CAP*(x: untyped): void =
  nil

template SDL_OUT_CAP*(x: untyped): void =
  nil

template SDL_OUT_BYTECAP*(x: untyped): void =
  nil

template SDL_OUT_Z_BYTECAP*(x: untyped): void =
  nil

const
  SDL_PRINTF_FORMAT_STRING* = true
  SDL_SCANF_FORMAT_STRING* = true

# template SDL_PRINTF_VARARG_FUNC*(fmtargnumber: untyped): untyped =
#   __attribute__((format(__printf__, fmtargnumber, fmtargnumber + 1)))

# template SDL_SCANF_VARARG_FUNC*(fmtargnumber: untyped): untyped =
#   __attribute__((format(__scanf__, fmtargnumber, fmtargnumber + 1)))

##  #define SDL_COMPILE_TIME_ASSERT(name, x) typedef int SDL_compile_time_assert_##name[(x)*2 - 1]

template SDL_stack_alloc*(`type`, count: untyped): untyped =
  cast[ptr `type`](SDL_malloc(sizeof((`type`) * (count))))

template SDL_stack_free*(data: untyped): untyped =
  SDL_free(data)

template SDL_min*(x, y: untyped): untyped =
  (if ((x) < (y)): (x) else: (y))

template SDL_max*(x, y: untyped): untyped =
  (if ((x) > (y)): (x) else: (y))

template SDL_zero*(x: untyped): untyped =
  SDL_memset(addr((x)), 0, sizeof(((x))))

template SDL_zerop*(x: untyped): untyped =
  SDL_memset((x), 0, sizeof(((x)[])))

template SDL_zeroa*(x: untyped): untyped =
  SDL_memset((x), 0, sizeof(((x))))

# const
#   SDL_ICONV_ERROR* = (csize_t) - 1
#   SDL_ICONV_E2BIG* = (csize_t) - 2
#   SDL_ICONV_EILSEQ* = (csize_t) - 3
#   SDL_ICONV_EINVAL* = (csize_t) - 4

template SDL_iconv_utf8_locale*(S: untyped): untyped =
  SDL_iconv_string("", "UTF-8", S, SDL_strlen(S) + 1)

template SDL_iconv_utf8_ucs2*(S: untyped): untyped =
  cast[ptr uint16](SDL_iconv_string("UCS-2-INTERNAL", "UTF-8", S, SDL_strlen(S) + 1))

template SDL_iconv_utf8_ucs4*(S: untyped): untyped =
  cast[ptr uint32](SDL_iconv_string("UCS-4-INTERNAL", "UTF-8", S, SDL_strlen(S) + 1))

const
  SDL_MAIN_AVAILABLE* = true
  SDLMAIN_DECLSPEC* = true
  SDL_ASSERT_LEVEL* = 2
  #SDL_TriggerBreakpoint() __asm__ __volatile__("int $3\t")
  # SDL_FUNCTION* = __func__
  # SDL_FILE* = __FILE__
  # SDL_LINE* = __LINE__
  SDL_NULL_WHILE_LOOP_CONDITION* = (0)

template SDL_disabled_assert*(condition: untyped): void =
  while true:
    cast[nil](sizeof(((condition))))
    if not SDL_NULL_WHILE_LOOP_CONDITION:
      break

template SDL_assert*(condition: untyped): untyped =
  SDL_enabled_assert(condition)

template SDL_assert_release*(condition: untyped): untyped =
  SDL_enabled_assert(condition)

template SDL_assert_paranoid*(condition: untyped): untyped =
  SDL_disabled_assert(condition)

template SDL_assert_always*(condition: untyped): untyped =
  SDL_enabled_assert(condition)



template SDL_MemoryBarrierRelease*(): untyped =
  SDL_CompilerBarrier()

template SDL_MemoryBarrierAcquire*(): untyped =
  SDL_CompilerBarrier()

template SDL_AtomicIncRef*(a: untyped): untyped =
  SDL_AtomicAdd(a, 1)

template SDL_AtomicDecRef*(a: untyped): untyped =
  (SDL_AtomicAdd(a, -1) == 1)

template SDL_OutOfMemory*(): untyped =
  SDL_Error(SDL_ENOMEM)

template SDL_Unsupported*(): untyped =
  SDL_Error(SDL_UNSUPPORTED)

template SDL_InvalidParamError*(param: untyped): untyped =
  SDL_SetError("Parameter \'%s\' is invalid", (param))

const
  SDL_LIL_ENDIAN* = 1234
  SDL_BIG_ENDIAN* = 4321
  SDL_BYTEORDER* = SDL_LIL_ENDIAN

template SDL_SwapLE16*(X: untyped): untyped =
  (X)

template SDL_SwapLE32*(X: untyped): untyped =
  (X)

template SDL_SwapLE64*(X: untyped): untyped =
  (X)

template SDL_SwapFloatLE*(X: untyped): untyped =
  (X)

template SDL_SwapBE16*(X: untyped): untyped =
  SDL_Swap16(X)

template SDL_SwapBE32*(X: untyped): untyped =
  SDL_Swap32(X)

template SDL_SwapBE64*(X: untyped): untyped =
  SDL_Swap64(X)

template SDL_SwapFloatBE*(X: untyped): untyped =
  SDL_SwapFloat(X)

const
  SDL_MUTEX_TIMEDOUT* = 1
  SDL_MUTEX_MAXWAIT* = (not cast[uint32](0))

template SDL_mutexP*(m: untyped): untyped =
  SDL_LockMutex(m)

const
  SDL_PASSED_BEGINTHREAD_ENDTHREAD* = true

 
type 
  SDL_beginthread* {.importc:"_beginthreadex"} = object
  SDL_endthread* {.importc:"_endthreadex"} = object

type
  SDL_Thread* {.bycopy.} = object

  SDL_threadID* = culong
  SDL_TLSID* = cuint
  SDL_ThreadPriority* {.size: sizeof(cint).} = enum
    SDL_THREAD_PRIORITY_LOW, SDL_THREAD_PRIORITY_NORMAL, SDL_THREAD_PRIORITY_HIGH,
    SDL_THREAD_PRIORITY_TIME_CRITICAL
  SDL_ThreadFunction* = proc (data: pointer): cint
  pfnSDL_CurrentBeginThread* = proc (a1: pointer; a2: cuint; `func`: proc (a1: pointer): cuint; a4: pointer; a5: cuint; a6: ptr cuint): uint64
  pfnSDL_CurrentEndThread* = proc (code: cuint)


const
  SDL_RWOPS_UNKNOWN* = 0
  SDL_RWOPS_WINFILE* = 1
  SDL_RWOPS_STDFILE* = 2
  SDL_RWOPS_JNIFILE* = 3
  SDL_RWOPS_MEMORY* = 4
  SDL_RWOPS_MEMORY_RO* = 5
  RW_SEEK_SET* = 0
  RW_SEEK_CUR* = 1
  RW_SEEK_END* = 2
  SDL_AUDIO_MASK_BITSIZE* = (0x000000FF)
  SDL_AUDIO_MASK_DATATYPE* = (1 shl 8)
  SDL_AUDIO_MASK_ENDIAN* = (1 shl 12)
  SDL_AUDIO_MASK_SIGNED* = (1 shl 15)

template SDL_AUDIO_BITSIZE*(x: untyped): untyped =
  (x and SDL_AUDIO_MASK_BITSIZE)

template SDL_AUDIO_ISFLOAT*(x: untyped): untyped =
  (x and SDL_AUDIO_MASK_DATATYPE)

template SDL_AUDIO_ISBIGENDIAN*(x: untyped): untyped =
  (x and SDL_AUDIO_MASK_ENDIAN)

template SDL_AUDIO_ISSIGNED*(x: untyped): untyped =
  (x and SDL_AUDIO_MASK_SIGNED)

template SDL_AUDIO_ISINT*(x: untyped): untyped =
  (not SDL_AUDIO_ISFLOAT(x))

template SDL_AUDIO_ISLITTLEENDIAN*(x: untyped): untyped =
  (not SDL_AUDIO_ISBIGENDIAN(x))

template SDL_AUDIO_ISUNSIGNED*(x: untyped): untyped =
  (not SDL_AUDIO_ISSIGNED(x))

const
  AUDIO_U8* = 0x00000008
  AUDIO_S8* = 0x00008008
  AUDIO_U16LSB* = 0x00000010
  AUDIO_S16LSB* = 0x00008010
  AUDIO_U16MSB* = 0x00001010
  AUDIO_S16MSB* = 0x00009010
  AUDIO_U16* = AUDIO_U16LSB
  AUDIO_S16* = AUDIO_S16LSB
  AUDIO_S32LSB* = 0x00008020
  AUDIO_S32MSB* = 0x00009020
  AUDIO_S32* = AUDIO_S32LSB
  AUDIO_F32LSB* = 0x00008120
  AUDIO_F32MSB* = 0x00009120
  AUDIO_F32* = AUDIO_F32LSB
  AUDIO_U16SYS* = AUDIO_U16LSB
  AUDIO_S16SYS* = AUDIO_S16LSB
  AUDIO_S32SYS* = AUDIO_S32LSB
  AUDIO_F32SYS* = AUDIO_F32LSB
  SDL_AUDIO_ALLOW_FREQUENCY_CHANGE* = 0x00000001
  SDL_AUDIO_ALLOW_FORMAT_CHANGE* = 0x00000002
  SDL_AUDIO_ALLOW_CHANNELS_CHANGE* = 0x00000004
  SDL_AUDIO_ALLOW_SAMPLES_CHANGE* = 0x00000008
  SDL_AUDIO_ALLOW_ANY_CHANGE* = (SDL_AUDIO_ALLOW_FREQUENCY_CHANGE or
      SDL_AUDIO_ALLOW_FORMAT_CHANGE or SDL_AUDIO_ALLOW_CHANNELS_CHANGE or
      SDL_AUDIO_ALLOW_SAMPLES_CHANGE)
  SDL_AUDIOCVT_MAX_FILTERS* = 9
  # SDL_AUDIOCVT_PACKED* = __attribute__((packed))

template SDL_LoadWAV*(file, spec, audio_buf, audio_len: untyped): untyped =
  SDL_LoadWAV_RW(SDL_RWFromFile(file, "rb"), 1, spec, audio_buf, audio_len)

const
  SDL_MIX_MAXVOLUME* = 128
  SDL_CACHELINE_SIZE* = 128
  SDL_ALPHA_OPAQUE* = 255
  SDL_ALPHA_TRANSPARENT* = 0

template SDL_DEFINE_PIXELFOURCC*(A, B, C, D: untyped): untyped =
  SDL_FOURCC(A, B, C, D)

template SDL_DEFINE_PIXELFORMAT*(`type`, order, layout, bits, bytes: untyped): untyped =
  ((1 shl 28) or ((`type`) shl 24) or ((order) shl 20) or ((layout) shl 16) or ((bits) shl 8) or
      ((bytes) shl 0))

template SDL_PIXELFLAG*(X: untyped): untyped =
  (((X) shr 28) and 0x0000000F)

template SDL_PIXELTYPE*(X: untyped): untyped =
  (((X) shr 24) and 0x0000000F)

template SDL_PIXELORDER*(X: untyped): untyped =
  (((X) shr 20) and 0x0000000F)

template SDL_PIXELLAYOUT*(X: untyped): untyped =
  (((X) shr 16) and 0x0000000F)

template SDL_BITSPERPIXEL*(X: untyped): untyped =
  (((X) shr 8) and 0x000000FF)

template SDL_BYTESPERPIXEL*(X: untyped): untyped =
  (if SDL_ISPIXELFORMAT_FOURCC(X): (if (((X) == SDL_PIXELFORMAT_YUY2) or
      ((X) == SDL_PIXELFORMAT_UYVY) or ((X) == SDL_PIXELFORMAT_YVYU)): 2 else: 1) else: (
      ((X) shr 0) and 0x000000FF))

template SDL_ISPIXELFORMAT_INDEXED*(format: untyped): untyped =
  (not SDL_ISPIXELFORMAT_FOURCC(format) and
      ((SDL_PIXELTYPE(format) == SDL_PIXELTYPE_INDEX1) or
      (SDL_PIXELTYPE(format) == SDL_PIXELTYPE_INDEX4) or
      (SDL_PIXELTYPE(format) == SDL_PIXELTYPE_INDEX8)))

template SDL_ISPIXELFORMAT_PACKED*(format: untyped): untyped =
  (not SDL_ISPIXELFORMAT_FOURCC(format) and
      ((SDL_PIXELTYPE(format) == SDL_PIXELTYPE_PACKED8) or
      (SDL_PIXELTYPE(format) == SDL_PIXELTYPE_PACKED16) or
      (SDL_PIXELTYPE(format) == SDL_PIXELTYPE_PACKED32)))

template SDL_ISPIXELFORMAT_ARRAY*(format: untyped): untyped =
  (not SDL_ISPIXELFORMAT_FOURCC(format) and
      ((SDL_PIXELTYPE(format) == SDL_PIXELTYPE_ARRAYU8) or
      (SDL_PIXELTYPE(format) == SDL_PIXELTYPE_ARRAYU16) or
      (SDL_PIXELTYPE(format) == SDL_PIXELTYPE_ARRAYU32) or
      (SDL_PIXELTYPE(format) == SDL_PIXELTYPE_ARRAYF16) or
      (SDL_PIXELTYPE(format) == SDL_PIXELTYPE_ARRAYF32)))

template SDL_ISPIXELFORMAT_ALPHA*(format: untyped): untyped =
  ((SDL_ISPIXELFORMAT_PACKED(format) and
      ((SDL_PIXELORDER(format) == SDL_PACKEDORDER_ARGB) or
      (SDL_PIXELORDER(format) == SDL_PACKEDORDER_RGBA) or
      (SDL_PIXELORDER(format) == SDL_PACKEDORDER_ABGR) or
      (SDL_PIXELORDER(format) == SDL_PACKEDORDER_BGRA))) or
      (SDL_ISPIXELFORMAT_ARRAY(format) and
      ((SDL_PIXELORDER(format) == SDL_ARRAYORDER_ARGB) or
      (SDL_PIXELORDER(format) == SDL_ARRAYORDER_RGBA) or
      (SDL_PIXELORDER(format) == SDL_ARRAYORDER_ABGR) or
      (SDL_PIXELORDER(format) == SDL_ARRAYORDER_BGRA))))

template SDL_ISPIXELFORMAT_FOURCC*(format: untyped): untyped =
  ((format) and (SDL_PIXELFLAG(format) != 1))

const
  SDL_SWSURFACE* = 0
  SDL_PREALLOC* = 0x00000001
  SDL_RLEACCEL* = 0x00000002
  SDL_DONTFREE* = 0x00000004
  SDL_SIMD_ALIGNED* = 0x00000008

template SDL_MUSTLOCK*(S: untyped): untyped =
  (((S).flags and SDL_RLEACCEL) != 0)

template SDL_LoadBMP*(file: untyped): untyped =
  SDL_LoadBMP_RW(SDL_RWFromFile(file, "rb"), 1)

template SDL_SaveBMP*(surface, file: untyped): untyped =
  SDL_SaveBMP_RW(surface, SDL_RWFromFile(file, "wb"), 1)

const
  # SDL_BlitSurface* = SDL_UpperBlit
  # SDL_BlitScaled* = SDL_UpperBlitScaled
  SDL_WINDOWPOS_UNDEFINED_MASK* = 0x1FFF0000

template SDL_WINDOWPOS_UNDEFINED_DISPLAY*(X: untyped): untyped =
  (SDL_WINDOWPOS_UNDEFINED_MASK or (X))

const
  SDL_WINDOWPOS_UNDEFINED* = SDL_WINDOWPOS_UNDEFINED_DISPLAY(0)

template SDL_WINDOWPOS_ISUNDEFINED*(X: untyped): untyped =
  (((X) and 0xFFFF0000) == SDL_WINDOWPOS_UNDEFINED_MASK)

const
  SDL_WINDOWPOS_CENTERED_MASK* = 0x2FFF0000

template SDL_WINDOWPOS_CENTERED_DISPLAY*(X: untyped): untyped =
  (SDL_WINDOWPOS_CENTERED_MASK or (X))

const
  SDL_WINDOWPOS_CENTERED* = SDL_WINDOWPOS_CENTERED_DISPLAY(0)

template SDL_WINDOWPOS_ISCENTERED*(X: untyped): untyped =
  (((X) and 0xFFFF0000) == SDL_WINDOWPOS_CENTERED_MASK)

template SDL_BUTTON*(X: untyped): untyped =
  (1 shl ((X) - 1))

const
  SDL_BUTTON_LEFT* = 1
  SDL_BUTTON_MIDDLE* = 2
  SDL_BUTTON_RIGHT* = 3
  SDL_BUTTON_X1* = 4
  SDL_BUTTON_X2* = 5
  SDL_BUTTON_LMASK* = SDL_BUTTON(SDL_BUTTON_LEFT)
  SDL_BUTTON_MMASK* = SDL_BUTTON(SDL_BUTTON_MIDDLE)
  SDL_BUTTON_RMASK* = SDL_BUTTON(SDL_BUTTON_RIGHT)
  SDL_BUTTON_X1MASK* = SDL_BUTTON(SDL_BUTTON_X1)
  SDL_BUTTON_X2MASK* = SDL_BUTTON(SDL_BUTTON_X2)
  SDL_JOYSTICK_AXIS_MAX* = 32767
  SDL_JOYSTICK_AXIS_MIN* = -32768
  SDL_HAT_CENTERED* = 0x00000000
  SDL_HAT_UP* = 0x00000001
  SDL_HAT_RIGHT* = 0x00000002
  SDL_HAT_DOWN* = 0x00000004
  SDL_HAT_LEFT* = 0x00000008
  SDL_HAT_RIGHTUP* = (SDL_HAT_RIGHT or SDL_HAT_UP)
  SDL_HAT_RIGHTDOWN* = (SDL_HAT_RIGHT or SDL_HAT_DOWN)
  SDL_HAT_LEFTUP* = (SDL_HAT_LEFT or SDL_HAT_UP)
  SDL_HAT_LEFTDOWN* = (SDL_HAT_LEFT or SDL_HAT_DOWN)

template SDL_GameControllerAddMappingsFromFile*(file: untyped): untyped =
  SDL_GameControllerAddMappingsFromRW(SDL_RWFromFile(file, "rb"), 1)

# template SDL_QuitRequested*(): untyped =
#   (
#     SDL_PumpEvents()
#     (SDL_PeepEvents(nil, 0, SDL_PEEKEVENT, SDL_QUIT, SDL_QUIT) > 0))

const
  SDL_TOUCH_MOUSEID* = - 1
  SDL_MOUSE_TOUCHID* = - 1
  SDL_RELEASED* = 0
  SDL_PRESSED* = 1
  SDL_TEXTEDITINGEVENT_TEXT_SIZE* = (32)
  SDL_TEXTINPUTEVENT_TEXT_SIZE* = (32)
  SDL_QUERY* = -1
  SDL_IGNORE* = 0
  SDL_DISABLE* = 0
  SDL_ENABLE* = 1

template SDL_GetEventState*(`type`: untyped): untyped =
  SDL_EventState(`type`, SDL_QUERY)

const
  SDL_HAPTIC_CONSTANT_VALUE* = (1 shl 0)
  SDL_HAPTIC_SINE* = (1 shl 1)
  SDL_HAPTIC_LEFTRIGHT_VALUE* = (1 shl 2)
  SDL_HAPTIC_TRIANGLE* = (1 shl 3)
  SDL_HAPTIC_SAWTOOTHUP* = (1 shl 4)
  SDL_HAPTIC_SAWTOOTHDOWN* = (1 shl 5)
  SDL_HAPTIC_RAMP_VALUE* = (1 shl 6)
  SDL_HAPTIC_SPRING* = (1 shl 7)
  SDL_HAPTIC_DAMPER* = (1 shl 8)
  SDL_HAPTIC_INERTIA* = (1 shl 9)
  SDL_HAPTIC_FRICTION* = (1 shl 10)
  SDL_HAPTIC_CUSTOM_VALUE* = (1 shl 11)
  SDL_HAPTIC_GAIN* = (1 shl 12)
  SDL_HAPTIC_AUTOCENTER* = (1 shl 13)
  SDL_HAPTIC_STATUS* = (1 shl 14)
  SDL_HAPTIC_PAUSE_VALUE* = (1 shl 15)
  SDL_HAPTIC_POLAR* = 0
  SDL_HAPTIC_CARTESIAN* = 1
  SDL_HAPTIC_SPHERICAL* = 2
  SDL_HAPTIC_INFINITY* = 4294967295'i64
  SDL_HINT_FRAMEBUFFER_ACCELERATION* = "SDL_FRAMEBUFFER_ACCELERATION"
  SDL_HINT_RENDER_DRIVER* = "SDL_RENDER_DRIVER"
  SDL_HINT_RENDER_OPENGL_SHADERS* = "SDL_RENDER_OPENGL_SHADERS"
  SDL_HINT_RENDER_DIRECT3D_THREADSAFE* = "SDL_RENDER_DIRECT3D_THREADSAFE"
  SDL_HINT_RENDER_DIRECT3D11_DEBUG* = "SDL_RENDER_DIRECT3D11_DEBUG"
  SDL_HINT_RENDER_LOGICAL_SIZE_MODE* = "SDL_RENDER_LOGICAL_SIZE_MODE"
  SDL_HINT_RENDER_SCALE_QUALITY* = "SDL_RENDER_SCALE_QUALITY"
  SDL_HINT_RENDER_VSYNC* = "SDL_RENDER_VSYNC"
  SDL_HINT_VIDEO_ALLOW_SCREENSAVER* = "SDL_VIDEO_ALLOW_SCREENSAVER"
  SDL_HINT_VIDEO_EXTERNAL_CONTEXT* = "SDL_VIDEO_EXTERNAL_CONTEXT"
  SDL_HINT_VIDEO_X11_XVIDMODE* = "SDL_VIDEO_X11_XVIDMODE"
  SDL_HINT_VIDEO_X11_XINERAMA* = "SDL_VIDEO_X11_XINERAMA"
  SDL_HINT_VIDEO_X11_XRANDR* = "SDL_VIDEO_X11_XRANDR"
  SDL_HINT_VIDEO_X11_WINDOW_VISUALID* = "SDL_VIDEO_X11_WINDOW_VISUALID"
  SDL_HINT_VIDEO_X11_NET_WM_PING* = "SDL_VIDEO_X11_NET_WM_PING"
  SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR* = "SDL_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR"
  SDL_HINT_VIDEO_X11_FORCE_EGL* = "SDL_VIDEO_X11_FORCE_EGL"
  SDL_HINT_WINDOW_FRAME_USABLE_WHILE_CURSOR_HIDDEN* = "SDL_WINDOW_FRAME_USABLE_WHILE_CURSOR_HIDDEN"
  SDL_HINT_WINDOWS_INTRESOURCE_ICON* = "SDL_WINDOWS_INTRESOURCE_ICON"
  SDL_HINT_WINDOWS_INTRESOURCE_ICON_SMALL* = "SDL_WINDOWS_INTRESOURCE_ICON_SMALL"
  SDL_HINT_WINDOWS_ENABLE_MESSAGELOOP* = "SDL_WINDOWS_ENABLE_MESSAGELOOP"
  SDL_HINT_GRAB_KEYBOARD* = "SDL_GRAB_KEYBOARD"
  SDL_HINT_MOUSE_DOUBLE_CLICK_TIME* = "SDL_MOUSE_DOUBLE_CLICK_TIME"
  SDL_HINT_MOUSE_DOUBLE_CLICK_RADIUS* = "SDL_MOUSE_DOUBLE_CLICK_RADIUS"
  SDL_HINT_MOUSE_NORMAL_SPEED_SCALE* = "SDL_MOUSE_NORMAL_SPEED_SCALE"
  SDL_HINT_MOUSE_RELATIVE_SPEED_SCALE* = "SDL_MOUSE_RELATIVE_SPEED_SCALE"
  SDL_HINT_MOUSE_RELATIVE_MODE_WARP* = "SDL_MOUSE_RELATIVE_MODE_WARP"
  SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH* = "SDL_MOUSE_FOCUS_CLICKTHROUGH"
  SDL_HINT_TOUCH_MOUSE_EVENTS* = "SDL_TOUCH_MOUSE_EVENTS"
  SDL_HINT_MOUSE_TOUCH_EVENTS* = "SDL_MOUSE_TOUCH_EVENTS"
  SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS* = "SDL_VIDEO_MINIMIZE_ON_FOCUS_LOSS"
  SDL_HINT_IDLE_TIMER_DISABLED* = "SDL_IOS_IDLE_TIMER_DISABLED"
  SDL_HINT_ORIENTATIONS* = "SDL_IOS_ORIENTATIONS"
  SDL_HINT_APPLE_TV_CONTROLLER_UI_EVENTS* = "SDL_APPLE_TV_CONTROLLER_UI_EVENTS"
  SDL_HINT_APPLE_TV_REMOTE_ALLOW_ROTATION* = "SDL_APPLE_TV_REMOTE_ALLOW_ROTATION"
  SDL_HINT_IOS_HIDE_HOME_INDICATOR* = "SDL_IOS_HIDE_HOME_INDICATOR"
  SDL_HINT_ACCELEROMETER_AS_JOYSTICK* = "SDL_ACCELEROMETER_AS_JOYSTICK"
  SDL_HINT_TV_REMOTE_AS_JOYSTICK* = "SDL_TV_REMOTE_AS_JOYSTICK"
  SDL_HINT_XINPUT_ENABLED* = "SDL_XINPUT_ENABLED"
  SDL_HINT_XINPUT_USE_OLD_JOYSTICK_MAPPING* = "SDL_XINPUT_USE_OLD_JOYSTICK_MAPPING"
  SDL_HINT_GAMECONTROLLERTYPE* = "SDL_GAMECONTROLLERTYPE"
  SDL_HINT_GAMECONTROLLERCONFIG* = "SDL_GAMECONTROLLERCONFIG"
  SDL_HINT_GAMECONTROLLERCONFIG_FILE* = "SDL_GAMECONTROLLERCONFIG_FILE"
  SDL_HINT_GAMECONTROLLER_IGNORE_DEVICES* = "SDL_GAMECONTROLLER_IGNORE_DEVICES"
  SDL_HINT_GAMECONTROLLER_IGNORE_DEVICES_EXCEPT* = "SDL_GAMECONTROLLER_IGNORE_DEVICES_EXCEPT"
  SDL_HINT_GAMECONTROLLER_USE_BUTTON_LABELS* = "SDL_GAMECONTROLLER_USE_BUTTON_LABELS"
  SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS* = "SDL_JOYSTICK_ALLOW_BACKGROUND_EVENTS"
  SDL_HINT_JOYSTICK_HIDAPI* = "SDL_JOYSTICK_HIDAPI"
  SDL_HINT_JOYSTICK_HIDAPI_PS4* = "SDL_JOYSTICK_HIDAPI_PS4"
  SDL_HINT_JOYSTICK_HIDAPI_PS4_RUMBLE* = "SDL_JOYSTICK_HIDAPI_PS4_RUMBLE"
  SDL_HINT_JOYSTICK_HIDAPI_STEAM* = "SDL_JOYSTICK_HIDAPI_STEAM"
  SDL_HINT_JOYSTICK_HIDAPI_SWITCH* = "SDL_JOYSTICK_HIDAPI_SWITCH"
  SDL_HINT_JOYSTICK_HIDAPI_XBOX* = "SDL_JOYSTICK_HIDAPI_XBOX"
  SDL_HINT_JOYSTICK_HIDAPI_GAMECUBE* = "SDL_JOYSTICK_HIDAPI_GAMECUBE"
  SDL_HINT_ENABLE_STEAM_CONTROLLERS* = "SDL_ENABLE_STEAM_CONTROLLERS"
  SDL_HINT_ALLOW_TOPMOST* = "SDL_ALLOW_TOPMOST"
  SDL_HINT_TIMER_RESOLUTION* = "SDL_TIMER_RESOLUTION"
  SDL_HINT_QTWAYLAND_CONTENT_ORIENTATION* = "SDL_QTWAYLAND_CONTENT_ORIENTATION"
  SDL_HINT_QTWAYLAND_WINDOW_FLAGS* = "SDL_QTWAYLAND_WINDOW_FLAGS"
  SDL_HINT_THREAD_STACK_SIZE* = "SDL_THREAD_STACK_SIZE"
  SDL_HINT_VIDEO_HIGHDPI_DISABLED* = "SDL_VIDEO_HIGHDPI_DISABLED"
  SDL_HINT_MAC_CTRL_CLICK_EMULATE_RIGHT_CLICK* = "SDL_MAC_CTRL_CLICK_EMULATE_RIGHT_CLICK"
  SDL_HINT_VIDEO_WIN_D3DCOMPILER* = "SDL_VIDEO_WIN_D3DCOMPILER"
  SDL_HINT_VIDEO_WINDOW_SHARE_PIXEL_FORMAT* = "SDL_VIDEO_WINDOW_SHARE_PIXEL_FORMAT"
  SDL_HINT_WINRT_PRIVACY_POLICY_URL* = "SDL_WINRT_PRIVACY_POLICY_URL"
  SDL_HINT_WINRT_PRIVACY_POLICY_LABEL* = "SDL_WINRT_PRIVACY_POLICY_LABEL"
  SDL_HINT_WINRT_HANDLE_BACK_BUTTON* = "SDL_WINRT_HANDLE_BACK_BUTTON"
  SDL_HINT_VIDEO_MAC_FULLSCREEN_SPACES* = "SDL_VIDEO_MAC_FULLSCREEN_SPACES"
  SDL_HINT_MAC_BACKGROUND_APP* = "SDL_MAC_BACKGROUND_APP"
  SDL_HINT_ANDROID_APK_EXPANSION_MAIN_FILE_VERSION* = "SDL_ANDROID_APK_EXPANSION_MAIN_FILE_VERSION"
  SDL_HINT_ANDROID_APK_EXPANSION_PATCH_FILE_VERSION* = "SDL_ANDROID_APK_EXPANSION_PATCH_FILE_VERSION"
  SDL_HINT_IME_INTERNAL_EDITING* = "SDL_IME_INTERNAL_EDITING"
  SDL_HINT_ANDROID_TRAP_BACK_BUTTON* = "SDL_ANDROID_TRAP_BACK_BUTTON"
  SDL_HINT_ANDROID_BLOCK_ON_PAUSE* = "SDL_ANDROID_BLOCK_ON_PAUSE"
  SDL_HINT_RETURN_KEY_HIDES_IME* = "SDL_RETURN_KEY_HIDES_IME"
  SDL_HINT_EMSCRIPTEN_KEYBOARD_ELEMENT* = "SDL_EMSCRIPTEN_KEYBOARD_ELEMENT"
  SDL_HINT_NO_SIGNAL_HANDLERS* = "SDL_NO_SIGNAL_HANDLERS"
  SDL_HINT_WINDOWS_NO_CLOSE_ON_ALT_F4* = "SDL_WINDOWS_NO_CLOSE_ON_ALT_F4"
  SDL_HINT_BMP_SAVE_LEGACY_FORMAT* = "SDL_BMP_SAVE_LEGACY_FORMAT"
  SDL_HINT_WINDOWS_DISABLE_THREAD_NAMING* = "SDL_WINDOWS_DISABLE_THREAD_NAMING"
  SDL_HINT_RPI_VIDEO_LAYER* = "SDL_RPI_VIDEO_LAYER"
  SDL_HINT_VIDEO_DOUBLE_BUFFER* = "SDL_VIDEO_DOUBLE_BUFFER"
  SDL_HINT_OPENGL_ES_DRIVER* = "SDL_OPENGL_ES_DRIVER"
  SDL_HINT_AUDIO_RESAMPLING_MODE* = "SDL_AUDIO_RESAMPLING_MODE"
  SDL_HINT_AUDIO_CATEGORY* = "SDL_AUDIO_CATEGORY"
  SDL_HINT_RENDER_BATCHING* = "SDL_RENDER_BATCHING"
  SDL_HINT_EVENT_LOGGING* = "SDL_EVENT_LOGGING"
  SDL_HINT_WAVE_RIFF_CHUNK_SIZE* = "SDL_WAVE_RIFF_CHUNK_SIZE"
  SDL_HINT_WAVE_TRUNCATION* = "SDL_WAVE_TRUNCATION"
  SDL_HINT_WAVE_FACT_CHUNK* = "SDL_WAVE_FACT_CHUNK"
  SDL_HINT_DISPLAY_USABLE_BOUNDS* = "SDL_DISPLAY_USABLE_BOUNDS"
  SDL_MAX_LOG_MESSAGE* = 4096
  SDL_STANDARD_GRAVITY* = 9.806649999999999
  SDL_NONSHAPEABLE_WINDOW* = -1
  SDL_INVALID_SHAPE_ARGUMENT* = -2
  SDL_WINDOW_LACKS_SHAPE* = -3

template SDL_SHAPEMODEALPHA*(mode: untyped): untyped =
  (mode == ShapeModeDefault or mode == ShapeModeBinarizeAlpha or
      mode == ShapeModeReverseBinarizeAlpha)

template SDL_TICKS_PASSED*(A, B: untyped): untyped =
  ((int32)((B) - (A)) <= 0)

const
  SDL_MAJOR_VERSION* = 2
  SDL_MINOR_VERSION* = 0
  SDL_PATCHLEVEL* = 12

template SDL_VERSIONNUM*(X, Y, Z: untyped): untyped =
  ((X) * 1000 + (Y) * 100 + (Z))

const
  SDL_COMPILEDVERSION* = SDL_VERSIONNUM(SDL_MAJOR_VERSION, SDL_MINOR_VERSION,
                                      SDL_PATCHLEVEL)

template SDL_VERSION_ATLEAST*(X, Y, Z: untyped): untyped =
  (SDL_COMPILEDVERSION >= SDL_VERSIONNUM(X, Y, Z))

const
  SDL_INIT_TIMER* = 0x00000001
  SDL_INIT_AUDIO* = 0x00000010
  SDL_INIT_VIDEO* = 0x00000020
  SDL_INIT_JOYSTICK* = 0x00000200
  SDL_INIT_HAPTIC* = 0x00001000
  SDL_INIT_GAMECONTROLLER* = 0x00002000
  SDL_INIT_EVENTS* = 0x00004000
  SDL_INIT_SENSOR* = 0x00008000
  SDL_INIT_NOPARACHUTE* = 0x00100000
  SDL_INIT_EVERYTHING* = (SDL_INIT_TIMER or SDL_INIT_AUDIO or SDL_INIT_VIDEO or SDL_INIT_EVENTS or SDL_INIT_JOYSTICK or SDL_INIT_HAPTIC or SDL_INIT_GAMECONTROLLER or SDL_INIT_SENSOR)
  HAS_ARG* = 0x00000001
  OPT_BOOL* = 0x00000002
  OPT_EXPERT* = 0x00000004
  OPT_STRING* = 0x00000008
  OPT_VIDEO* = 0x00000010
  OPT_AUDIO* = 0x00000020
  OPT_INT* = 0x00000080
  OPT_FLOAT* = 0x00000100
  OPT_SUBTITLE* = 0x00000200
  OPT_INT64* = 0x00000400
  OPT_EXIT* = 0x00000800
  OPT_DATA* = 0x00001000
  OPT_PERFILE* = 0x00002000
  OPT_OFFSET* = 0x00004000
  OPT_SPEC* = 0x00008000
  OPT_TIME* = 0x00010000
  OPT_DOUBLE* = 0x00020000
  OPT_INPUT* = 0x00040000
  OPT_OUTPUT* = 0x00080000

template GROW_ARRAY*(arr, nb_elems: untyped): untyped =
  arr = grow_array(arr, sizeof((array[])), addr(nb_elems), nb_elems + 1)

template GET_PIX_FMT_NAME*(pix_fmt: untyped): void =
  var name*: cstring = av_get_pix_fmt_name(pix_fmt)

template GET_CODEC_NAME*(id: untyped): void =
  var name*: cstring = avcodec_descriptor_get(id).name

const
  MAX_QUEUE_SIZE* = (15 * 1024 * 1024)
  MIN_FRAMES* = 25
  EXTERNAL_CLOCK_MIN_FRAMES* = 2
  EXTERNAL_CLOCK_MAX_FRAMES* = 10
  SDL_AUDIO_MIN_BUFFER_SIZE* = 512
  SDL_AUDIO_MAX_CALLBACKS_PER_SEC* = 30
  SDL_VOLUME_STEP* = (0.75)
  AV_SYNC_THRESHOLD_MIN* = 0.04
  AV_SYNC_THRESHOLD_MAX* = 0.1
  AV_SYNC_FRAMEDUP_THRESHOLD* = 0.1
  AV_NOSYNC_THRESHOLD* = 10.0
  SAMPLE_CORRECTION_PERCENT_MAX* = 10
  EXTERNAL_CLOCK_SPEED_MIN* = 0.9
  EXTERNAL_CLOCK_SPEED_MAX* = 1.01
  EXTERNAL_CLOCK_SPEED_STEP* = 0.001
  AUDIO_DIFF_AVG_NB* = 20
  REFRESH_RATE* = 0.01
  SAMPLE_ARRAY_SIZE* = (8 * 65536)
  CURSOR_HIDE_DELAY* = 1000000
  USE_ONEPASS_SUBTITLE_RENDER* = 1
  VIDEO_PICTURE_QUEUE_SIZE* = 3
  SUBPICTURE_QUEUE_SIZE* = 16
  SAMPLE_QUEUE_SIZE* = 9
  FRAME_QUEUE_SIZE* = FFMAX(SAMPLE_QUEUE_SIZE, FFMAX(VIDEO_PICTURE_QUEUE_SIZE,
      SUBPICTURE_QUEUE_SIZE))

type
  AVMatrixEncoding* {.size: sizeof(cint).} = enum
    AV_MATRIX_ENCODING_NONE, AV_MATRIX_ENCODING_DOLBY, AV_MATRIX_ENCODING_DPLII,
    AV_MATRIX_ENCODING_DPLIIX, AV_MATRIX_ENCODING_DPLIIZ,
    AV_MATRIX_ENCODING_DOLBYEX, AV_MATRIX_ENCODING_DOLBYHEADPHONE,
    AV_MATRIX_ENCODING_NB



type
  AVFrameSideDataType* {.size: sizeof(cint).} = enum
    AV_FRAME_DATA_PANSCAN, AV_FRAME_DATA_A53_CC, AV_FRAME_DATA_STEREO3D,
    AV_FRAME_DATA_MATRIXENCODING, AV_FRAME_DATA_DOWNMIX_INFO,
    AV_FRAME_DATA_REPLAYGAIN, AV_FRAME_DATA_DISPLAYMATRIX, AV_FRAME_DATA_AFD,
    AV_FRAME_DATA_MOTION_VECTORS, AV_FRAME_DATA_SKIP_SAMPLES,
    AV_FRAME_DATA_AUDIO_SERVICE_TYPE, AV_FRAME_DATA_MASTERING_DISPLAY_METADATA,
    AV_FRAME_DATA_GOP_TIMECODE, AV_FRAME_DATA_SPHERICAL,
    AV_FRAME_DATA_CONTENT_LIGHT_LEVEL, AV_FRAME_DATA_ICC_PROFILE,
    AV_FRAME_DATA_QP_TABLE_PROPERTIES, AV_FRAME_DATA_QP_TABLE_DATA,
    AV_FRAME_DATA_S12M_TIMECODE, AV_FRAME_DATA_DYNAMIC_HDR_PLUS,
    AV_FRAME_DATA_REGIONS_OF_INTEREST, AV_FRAME_DATA_VIDEO_ENC_PARAMS,
    AV_FRAME_DATA_SEI_UNREGISTERED


type
  AVActiveFormatDescription* {.size: sizeof(cint).} = enum
    AV_AFD_SAME = 8, AV_AFD_4_3 = 9, AV_AFD_16_9 = 10, AV_AFD_14_9 = 11,
    AV_AFD_4_3_SP_14_9 = 13, AV_AFD_16_9_SP_14_9 = 14, AV_AFD_SP_4_3 = 15


type
  AVFrameSideData* {.bycopy.} = object
    `type`*: AVFrameSideDataType
    data*: ptr uint8
    size*: cint
    metadata*: ptr AVDictionary
    buf*: ptr AVBufferRef

  AVRegionOfInterest* {.bycopy.} = object
    self_size*: uint32
    top*: cint
    bottom*: cint
    left*: cint
    right*: cint
    qoffset*: AVRational

  AVFrame* {.bycopy.} = object
    data*: array[8, ptr uint8]
    linesize*: array[8, cint]
    extended_data*: ptr ptr uint8
    width*: cint
    height*: cint
    nb_samples*: cint
    format*: cint
    key_frame*: cint
    pict_type*: AVPictureType
    sample_aspect_ratio*: AVRational
    pts*: int64
    pkt_pts*: int64
    pkt_dts*: int64
    coded_picture_number*: cint
    display_picture_number*: cint
    quality*: cint
    opaque*: pointer
    error*: array[8, uint64]
    repeat_pict*: cint
    interlaced_frame*: cint
    top_field_first*: cint
    palette_has_changed*: cint
    reordered_opaque*: int64
    sample_rate*: cint
    channel_layout*: uint64
    buf*: array[8, ptr AVBufferRef]
    extended_buf*: ptr ptr AVBufferRef
    nb_extended_buf*: cint
    side_data*: ptr ptr AVFrameSideData
    nb_side_data*: cint
    flags*: cint
    color_range*: AVColorRange
    color_primaries*: AVColorPrimaries
    color_trc*: AVColorTransferCharacteristic
    colorspace*: AVColorSpace
    chroma_location*: AVChromaLocation
    best_effort_timestamp*: int64
    pkt_pos*: int64
    pkt_duration*: int64
    metadata*: ptr AVDictionary
    decode_error_flags*: cint
    channels*: cint
    pkt_size*: cint
    qscale_table*: ptr int8
    qstride*: cint
    qscale_type*: cint
    qp_table_buf*: ptr AVBufferRef
    hw_frames_ctx*: ptr AVBufferRef
    opaque_ref*: ptr AVBufferRef
    crop_top*: csize_t
    crop_bottom*: csize_t
    crop_left*: csize_t
    crop_right*: csize_t
    private_ref*: ptr AVBufferRef




const
  AV_FRAME_CROP_UNALIGNED* = 1 shl 0

type
  AVHWDeviceType* {.size: sizeof(cint).} = enum
    AV_HWDEVICE_TYPE_NONE, AV_HWDEVICE_TYPE_VDPAU, AV_HWDEVICE_TYPE_CUDA,
    AV_HWDEVICE_TYPE_VAAPI, AV_HWDEVICE_TYPE_DXVA2, AV_HWDEVICE_TYPE_QSV,
    AV_HWDEVICE_TYPE_VIDEOTOOLBOX, AV_HWDEVICE_TYPE_D3D11VA, AV_HWDEVICE_TYPE_DRM,
    AV_HWDEVICE_TYPE_OPENCL, AV_HWDEVICE_TYPE_MEDIACODEC, AV_HWDEVICE_TYPE_VULKAN
type
  AVHWDeviceInternal {.importc.} = object
  AVHWFramesInternal {.importc.} = object
  AVBSFInternal {.importc.} = object

  AVHWDeviceContext* {.bycopy.} = object
    av_class*: ptr AVClass
    internal*: ptr AVHWDeviceInternal
    `type`*: AVHWDeviceType
    hwctx*: pointer
    free*: proc (ctx: ptr AVHWDeviceContext)
    user_opaque*: pointer

  AVHWFramesContext* {.bycopy.} = object
    av_class*: ptr AVClass
    internal*: ptr AVHWFramesInternal
    device_ref*: ptr AVBufferRef
    device_ctx*: ptr AVHWDeviceContext
    hwctx*: pointer
    free*: proc (ctx: ptr AVHWFramesContext)
    user_opaque*: pointer
    pool*: ptr AVBufferPool
    initial_pool_size*: cint
    format*: AVPixelFormat
    sw_format*: AVPixelFormat
    width*: cint
    height*: cint
  AVHWFramesConstraints* {.bycopy.} = object
    valid_hw_formats*: ptr AVPixelFormat
    valid_sw_formats*: ptr AVPixelFormat
    min_width*: cint
    min_height*: cint
    max_width*: cint
    max_height*: cint


const
  AV_HWFRAME_MAP_READ* = 1 shl 0
  AV_HWFRAME_MAP_WRITE* = 1 shl 1
  AV_HWFRAME_MAP_OVERWRITE* = 1 shl 2
  AV_HWFRAME_MAP_DIRECT* = 1 shl 3




const
  AV_CODEC_ID_PCM_S16LE = AV_CODEC_ID_FIRST_AUDIO
  AV_CODEC_ID_DVD_SUBTITLE = AV_CODEC_ID_FIRST_SUBTITLE
  AV_CODEC_ID_TTF = AV_CODEC_ID_FIRST_UNKNOWN


type
  AVFieldOrder* {.size: sizeof(cint).} = enum
    AV_FIELD_UNKNOWN, AV_FIELD_PROGRESSIVE, AV_FIELD_TT, ## < Top coded_first, top displayed first
    AV_FIELD_BB,              ## < Bottom coded first, bottom displayed first
    AV_FIELD_TB,              ## < Top coded first, bottom displayed first
    AV_FIELD_BT               ## < Bottom coded first, top displayed first

  AVSideDataParamChangeFlags* {.size: sizeof(cint).} = enum
    AV_SIDE_DATA_PARAM_CHANGE_CHANNEL_COUNT = 0x00000001,
    AV_SIDE_DATA_PARAM_CHANGE_CHANNEL_LAYOUT = 0x00000002,
    AV_SIDE_DATA_PARAM_CHANGE_SAMPLE_RATE = 0x00000004,
    AV_SIDE_DATA_PARAM_CHANGE_DIMENSIONS = 0x00000008

  AVDiscard* {.size: sizeof(cint).} = enum
    AVDISCARD_NONE = -16,       ## /< discard nothing
    AVDISCARD_DEFAULT = 0,      ## /< discard useless packets like 0 size packets in avi
    AVDISCARD_NONREF = 8,       ## /< discard all non reference
    AVDISCARD_BIDIR = 16,       ## /< discard all bidirectional frames
    AVDISCARD_NONINTRA = 24,    ## /< discard all non intra frames
    AVDISCARD_NONKEY = 32,      ## /< discard all frames except keyframes
    AVDISCARD_ALL = 48          ## /< discard all

  AVAudioServiceType* {.size: sizeof(cint).} = enum
    AV_AUDIO_SERVICE_TYPE_MAIN = 0, AV_AUDIO_SERVICE_TYPE_EFFECTS = 1,
    AV_AUDIO_SERVICE_TYPE_VISUALLY_IMPAIRED = 2,
    AV_AUDIO_SERVICE_TYPE_HEARING_IMPAIRED = 3, AV_AUDIO_SERVICE_TYPE_DIALOGUE = 4,
    AV_AUDIO_SERVICE_TYPE_COMMENTARY = 5, AV_AUDIO_SERVICE_TYPE_EMERGENCY = 6,
    AV_AUDIO_SERVICE_TYPE_VOICE_OVER = 7, AV_AUDIO_SERVICE_TYPE_KARAOKE = 8, AV_AUDIO_SERVICE_TYPE_NB ## /< Not part of ABI

  AVSubtitleType* {.size: sizeof(cint).} = enum
    SUBTITLE_NONE, SUBTITLE_BITMAP, ## /< A bitmap, pict will be set
    SUBTITLE_TEXT, SUBTITLE_ASS

  AVStreamParseType* {.size: sizeof(cint).} = enum
    AVSTREAM_PARSE_NONE, AVSTREAM_PARSE_FULL, AVSTREAM_PARSE_HEADERS,
    AVSTREAM_PARSE_TIMESTAMPS, AVSTREAM_PARSE_FULL_ONCE, AVSTREAM_PARSE_FULL_RAW

  AVIODataMarkerType* {.size: sizeof(cint).} = enum
    AVIO_DATA_MARKER_HEADER, AVIO_DATA_MARKER_SYNC_POINT,
    AVIO_DATA_MARKER_BOUNDARY_POINT, AVIO_DATA_MARKER_UNKNOWN,
    AVIO_DATA_MARKER_TRAILER, AVIO_DATA_MARKER_FLUSH_POINT

  AVPictureStructure* {.size: sizeof(cint).} = enum
    AV_PICTURE_STRUCTURE_UNKNOWN, ## < unknown
    AV_PICTURE_STRUCTURE_TOP_FIELD, ## < coded as top field
    AV_PICTURE_STRUCTURE_BOTTOM_FIELD, ## < coded as bottom field
    AV_PICTURE_STRUCTURE_FRAME ## < coded as frame

  AVLockOp* {.size: sizeof(cint).} = enum
    AV_LOCK_CREATE,           ## /< Create a mutex
    AV_LOCK_OBTAIN,           ## /< Lock the mutex
    AV_LOCK_RELEASE,          ## /< Unlock the mutex
    AV_LOCK_DESTROY           ## /< Free mutex resources

  AVIODirEntryType* {.size: sizeof(cint).} = enum
    AVIO_ENTRY_UNKNOWN, AVIO_ENTRY_BLOCK_DEVICE, AVIO_ENTRY_CHARACTER_DEVICE,
    AVIO_ENTRY_DIRECTORY, AVIO_ENTRY_NAMED_PIPE, AVIO_ENTRY_SYMBOLIC_LINK,
    AVIO_ENTRY_SOCKET, AVIO_ENTRY_FILE, AVIO_ENTRY_SERVER, AVIO_ENTRY_SHARE,
    AVIO_ENTRY_WORKGROUP

  AVDurationEstimationMethod* {.size: sizeof(cint).} = enum
    AVFMT_DURATION_FROM_PTS,  ## /< Duration accurately estimated from PTSes
    AVFMT_DURATION_FROM_STREAM, ## /< Duration estimated from a stream with a known duration
    AVFMT_DURATION_FROM_BITRATE ## /< Duration estimated from bitrate (less accurate)

  SDL_BlendMode* {.size: sizeof(cint).} = enum
    SDL_BLENDMODE_NONE = 0x00000000, SDL_BLENDMODE_BLEND = 0x00000001,
    SDL_BLENDMODE_ADD = 0x00000002, SDL_BLENDMODE_MOD = 0x00000004,
    SDL_BLENDMODE_MUL = 0x00000008, SDL_BLENDMODE_INVALID = 0x7FFFFFFF
  SDL_BlendOperation* {.size: sizeof(cint).} = enum
    SDL_BLENDOPERATION_ADD = 0x00000001, SDL_BLENDOPERATION_SUBTRACT = 0x00000002,
    SDL_BLENDOPERATION_REV_SUBTRACT = 0x00000003,
    SDL_BLENDOPERATION_MINIMUM = 0x00000004,
    SDL_BLENDOPERATION_MAXIMUM = 0x00000005
  SDL_BlendFactor* {.size: sizeof(cint).} = enum
    SDL_BLENDFACTOR_ZERO = 0x00000001, SDL_BLENDFACTOR_ONE = 0x00000002,
    SDL_BLENDFACTOR_SRC_COLOR = 0x00000003,
    SDL_BLENDFACTOR_ONE_MINUS_SRC_COLOR = 0x00000004,
    SDL_BLENDFACTOR_SRC_ALPHA = 0x00000005,
    SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA = 0x00000006,
    SDL_BLENDFACTOR_DST_COLOR = 0x00000007,
    SDL_BLENDFACTOR_ONE_MINUS_DST_COLOR = 0x00000008,
    SDL_BLENDFACTOR_DST_ALPHA = 0x00000009,
    SDL_BLENDFACTOR_ONE_MINUS_DST_ALPHA = 0x0000000A

type
  AVCodecParameters* {.bycopy.} = object
    codec_type*: AVMediaType
    codec_id*: AVCodecID
    codec_tag*: uint32
    extradata*: ptr uint8
    extradata_size*: cint
    format*: cint
    bit_rate*: int64
    bits_per_coded_sample*: cint
    bits_per_raw_sample*: cint
    profile*: cint
    level*: cint
    width*: cint
    height*: cint
    sample_aspect_ratio*: AVRational
    field_order*: AVFieldOrder
    color_range*: AVColorRange
    color_primaries*: AVColorPrimaries
    color_trc*: AVColorTransferCharacteristic
    color_space*: AVColorSpace
    chroma_location*: AVChromaLocation
    video_delay*: cint
    channel_layout*: uint64
    channels*: cint
    sample_rate*: cint
    block_align*: cint
    frame_size*: cint
    initial_padding*: cint
    trailing_padding*: cint
    seek_preroll*: cint

  AVPacketSideData* {.bycopy.} = object
    data*: ptr uint8
    size*: cint
    `type`*: AVPacketSideDataType

  AVPacket* {.bycopy.} = object
    buf*: ptr AVBufferRef
    pts*: int64
    dts*: int64
    data*: ptr uint8
    size*: cint
    stream_index*: cint
    flags*: cint
    side_data*: ptr AVPacketSideData
    side_data_elems*: cint
    duration*: int64
    pos*: int64              
    convergence_duration*: int64

  AVPacketList* {.bycopy.} = object
    pkt*: AVPacket
    next*: ptr AVPacketList


  AVBSFContext* {.bycopy.} = object
    av_class*: ptr AVClass
    filter*: ptr AVBitStreamFilter
    internal*: ptr AVBSFInternal
    priv_data*: pointer
    par_in*: ptr AVCodecParameters
    par_out*: ptr AVCodecParameters
    time_base_in*: AVRational
    time_base_out*: AVRational

  AVBitStreamFilter* {.bycopy.} = object
    name*: cstring
    codec_ids*: ptr AVCodecID
    priv_class*: ptr AVClass
    priv_data_size*: cint
    init*: proc (ctx: ptr AVBSFContext): cint
    filter*: proc (ctx: ptr AVBSFContext; pkt: ptr AVPacket): cint
    close*: proc (ctx: ptr AVBSFContext)
    flush*: proc (ctx: ptr AVBSFContext)
  AVProfile* {.bycopy.} = object
    profile*: cint
    name*: cstring             ## /< short name for the profile

  RcOverride* {.bycopy.} = object
    start_frame*: cint
    end_frame*: cint
    qscale*: cint              ##  If this is 0 then quality_factor will be used instead.
    quality_factor*: cfloat

  AVPanScan* {.bycopy.} = object
    id*: cint
    width*: cint
    height*: cint
    position*: array[3, array[2, int16]]

  AVCPBProperties* {.bycopy.} = object
    max_bitrate*: cint
    min_bitrate*: cint
    avg_bitrate*: cint
    buffer_size*: cint
    vbv_delay*: uint64

  AVProducerReferenceTime* {.bycopy.} = object
    wallclock*: int64
    flags*: cint

  AVCodecInternal* {.bycopy.} = object

  AVSubtitle* {.bycopy.} = object
    format*: uint16
    start_display_time*: uint32
    end_display_time*: uint32
    num_rects*: cuint
    rects*: ptr ptr AVSubtitleRect
    pts*: int64              ## /< Same as packet pts, in AV_TIME_BASE

  MpegEncContext* {.bycopy.} = object


  AVHWAccel* {.bycopy.} = object
    name*: cstring
    `type`*: AVMediaType
    id*: AVCodecID
    pix_fmt*: AVPixelFormat
    capabilities*: cint
    alloc_frame*: proc (avctx: ptr AVCodecContext; frame: ptr AVFrame): cint
    start_frame*: proc (avctx: ptr AVCodecContext; buf: ptr uint8; buf_size: uint32): cint
    decode_params*: proc (avctx: ptr AVCodecContext; `type`: cint; buf: ptr uint8;
                        buf_size: uint32): cint
    decode_slice*: proc (avctx: ptr AVCodecContext; buf: ptr uint8; buf_size: uint32): cint
    end_frame*: proc (avctx: ptr AVCodecContext): cint
    frame_priv_data_size*: cint
    decode_mb*: proc (s: ptr MpegEncContext)
    init*: proc (avctx: ptr AVCodecContext): cint
    uninit*: proc (avctx: ptr AVCodecContext): cint
    priv_data_size*: cint
    caps_internal*: cint
    frame_params*: proc (avctx: ptr AVCodecContext; hw_frames_ctx: ptr AVBufferRef): cint

  AVPicture* {.bycopy.} = object
    data*: array[8, ptr uint8] ## /< pointers to the image data planes
    linesize*: array[8, cint]   ## /< number of bytes per line


  AVSubtitleRect* {.bycopy.} = object
    x*: cint                   ## /< top left corner  of pict, undefined when pict is not set
    y*: cint                   ## /< top left corner  of pict, undefined when pict is not set
    w*: cint                   ## /< width            of pict, undefined when pict is not set
    h*: cint                   ## /< height           of pict, undefined when pict is not set
    nb_colors*: cint           ## /< number of colors in pict, undefined when pict is not set
    pict*: AVPicture
    data*: array[4, ptr uint8]
    linesize*: array[4, cint]
    `type`*: AVSubtitleType
    text*: cstring             ## /< 0 terminated plain UTF-8 text
    ass*: cstring
    flags*: cint

  AVCodecHWConfig* {.bycopy.} = object
    pix_fmt*: AVPixelFormat
    methods*: cint
    device_type*: AVHWDeviceType

  AVCodecDescriptor* {.bycopy.} = object
    id*: AVCodecID
    `type`*: AVMediaType
    name*: cstring
    long_name*: cstring
    props*: cint
    mime_types*: cstringArray
    profiles*: ptr AVProfile

  AVCodecContext* {.bycopy.} = object
    av_class*: ptr AVClass
    log_level_offset*: cint
    codec_type*: AVMediaType
    codec*: ptr AVCodec
    codec_id*: AVCodecID
    codec_tag*: cuint
    priv_data*: pointer
    internal*: ptr AVCodecInternal
    opaque*: pointer
    bit_rate*: int64
    bit_rate_tolerance*: cint
    global_quality*: cint
    compression_level*: cint
    flags*: cint
    flags2*: cint
    extradata*: ptr uint8
    extradata_size*: cint
    time_base*: AVRational
    ticks_per_frame*: cint
    delay*: cint
    width*: cint
    height*: cint
    coded_width*: cint
    coded_height*: cint
    gop_size*: cint
    pix_fmt*: AVPixelFormat
    draw_horiz_band*: proc (s: ptr AVCodecContext; src: ptr AVFrame;
                          offset: array[8, cint]; y: cint; `type`: cint; height: cint)
    get_format*: proc (s: ptr AVCodecContext; fmt: ptr AVPixelFormat): AVPixelFormat
    max_b_frames*: cint
    b_quant_factor*: cfloat
    b_frame_strategy*: cint
    b_quant_offset*: cfloat
    has_b_frames*: cint
    mpeg_quant*: cint
    i_quant_factor*: cfloat
    i_quant_offset*: cfloat
    lumi_masking*: cfloat
    temporal_cplx_masking*: cfloat
    spatial_cplx_masking*: cfloat
    p_masking*: cfloat
    dark_masking*: cfloat
    slice_count*: cint
    prediction_method*: cint
    slice_offset*: ptr cint
    sample_aspect_ratio*: AVRational
    me_cmp*: cint
    me_sub_cmp*: cint
    mb_cmp*: cint
    ildct_cmp*: cint
    dia_size*: cint
    last_predictor_count*: cint
    pre_me*: cint
    me_pre_cmp*: cint
    pre_dia_size*: cint
    me_subpel_quality*: cint
    me_range*: cint
    slice_flags*: cint
    mb_decision*: cint
    intra_matrix*: ptr uint16
    inter_matrix*: ptr uint16
    scenechange_threshold*: cint
    noise_reduction*: cint
    intra_dc_precision*: cint
    skip_top*: cint
    skip_bottom*: cint
    mb_lmin*: cint
    mb_lmax*: cint
    me_penalty_compensation*: cint
    bidir_refine*: cint
    brd_scale*: cint
    keyint_min*: cint
    refs*: cint
    chromaoffset*: cint
    mv0_threshold*: cint
    b_sensitivity*: cint
    color_primaries*: AVColorPrimaries
    color_trc*: AVColorTransferCharacteristic
    colorspace*: AVColorSpace
    color_range*: AVColorRange
    chroma_sample_location*: AVChromaLocation
    slices*: cint
    field_order*: AVFieldOrder
    sample_rate*: cint         ## /< samples per second
    channels*: cint            ## /< number of audio channels
    sample_fmt*: AVSampleFormat ## /< sample format
    frame_size*: cint
    frame_number*: cint
    block_align*: cint
    cutoff*: cint
    channel_layout*: uint64
    request_channel_layout*: uint64
    audio_service_type*: AVAudioServiceType
    request_sample_fmt*: AVSampleFormat
    get_buffer2*: proc (s: ptr AVCodecContext; frame: ptr AVFrame; flags: cint): cint
    refcounted_frames*: cint
    qcompress*: cfloat         ## /< amount of qscale change between easy & hard scenes (0.0-1.0)
    qblur*: cfloat             ## /< amount of qscale smoothing over time (0.0-1.0)
    qmin*: cint
    qmax*: cint
    max_qdiff*: cint
    rc_buffer_size*: cint
    rc_override_count*: cint
    rc_override*: ptr RcOverride
    rc_max_rate*: int64
    rc_min_rate*: int64
    rc_max_available_vbv_use*: cfloat
    rc_min_vbv_overflow_use*: cfloat
    rc_initial_buffer_occupancy*: cint
    coder_type*: cint
    context_model*: cint
    frame_skip_threshold*: cint
    frame_skip_factor*: cint
    frame_skip_exp*: cint
    frame_skip_cmp*: cint
    trellis*: cint
    min_prediction_order*: cint
    max_prediction_order*: cint
    timecode_frame_start*: int64
    rtp_callback*: proc (avctx: ptr AVCodecContext; data: pointer; size: cint;
                       mb_nb: cint)
    rtp_payload_size*: cint
    mv_bits*: cint
    header_bits*: cint
    i_tex_bits*: cint
    p_tex_bits*: cint
    i_count*: cint
    p_count*: cint
    skip_count*: cint
    misc_bits*: cint
    frame_bits*: cint
    stats_out*: cstring
    stats_in*: cstring
    workaround_bugs*: cint
    strict_std_compliance*: cint
    error_concealment*: cint
    debug*: cint
    err_recognition*: cint
    reordered_opaque*: int64
    hwaccel*: ptr AVHWAccel
    hwaccel_context*: pointer
    error*: array[8, uint64]
    dct_algo*: cint
    idct_algo*: cint
    bits_per_coded_sample*: cint
    bits_per_raw_sample*: cint
    lowres*: cint
    coded_frame*: ptr AVFrame
    thread_count*: cint
    thread_type*: cint
    active_thread_type*: cint
    thread_safe_callbacks*: cint
    execute*: proc (c: ptr AVCodecContext;
                  `func`: proc (c2: ptr AVCodecContext; arg: pointer): cint;
                  arg2: pointer; result: ptr cint; count: cint; size: cint): cint
    execute2*: proc (c: ptr AVCodecContext; `func`: proc (c2: ptr AVCodecContext;
        arg: pointer; jobnr: cint; threadnr: cint): cint; arg2: pointer; result: ptr cint;
                   count: cint): cint
    nsse_weight*: cint
    profile*: cint
    level*: cint
    skip_loop_filter*: AVDiscard
    skip_idct*: AVDiscard
    skip_frame*: AVDiscard
    subtitle_header*: ptr uint8
    subtitle_header_size*: cint
    vbv_delay*: uint64
    side_data_only_packets*: cint
    initial_padding*: cint
    framerate*: AVRational
    sw_pix_fmt*: AVPixelFormat
    pkt_timebase*: AVRational
    codec_descriptor*: ptr AVCodecDescriptor
    pts_correction_num_faulty_pts*: int64 ## / Number of incorrect PTS values so far
    pts_correction_num_faulty_dts*: int64 ## / Number of incorrect DTS values so far
    pts_correction_last_pts*: int64 ## / PTS of the last frame
    pts_correction_last_dts*: int64 ## / DTS of the last frame
    sub_charenc*: cstring
    sub_charenc_mode*: cint
    skip_alpha*: cint
    seek_preroll*: cint
    debug_mv*: cint
    chroma_intra_matrix*: ptr uint16
    dump_separator*: ptr uint8
    codec_whitelist*: cstring
    properties*: cuint
    coded_side_data*: ptr AVPacketSideData
    nb_coded_side_data*: cint
    hw_frames_ctx*: ptr AVBufferRef
    sub_text_format*: cint
    trailing_padding*: cint
    max_pixels*: int64
    hw_device_ctx*: ptr AVBufferRef
    hwaccel_flags*: cint
    apply_cropping*: cint
    extra_hw_frames*: cint
    discard_damaged_percentage*: cint
    max_samples*: int64
    export_side_data*: cint

  AVCodecDefault* {.bycopy.} = object

  AVCodecHWConfigInternal* = object
  AVBSFList* = object
  URLContext* = object
  AVCodec* {.bycopy.} = object
    name*: cstring
    long_name*: cstring
    `type`*: AVMediaType
    id*: AVCodecID
    capabilities*: cint
    supported_framerates*: ptr AVRational ## /< array of supported framerates, or NULL if any, array is terminated by {0,0}
    pix_fmts*: ptr AVPixelFormat ## /< array of supported pixel formats, or NULL if unknown, array is terminated by -1
    supported_samplerates*: ptr cint ## /< array of supported audio samplerates, or NULL if unknown, array is terminated by 0
    sample_fmts*: ptr AVSampleFormat ## /< array of supported sample formats, or NULL if unknown, array is terminated by -1
    channel_layouts*: ptr uint64 ## /< array of support channel layouts, or NULL if unknown. array is terminated by 0
    max_lowres*: uint8       ## /< maximum value for lowres supported by the decoder
    priv_class*: ptr AVClass    ## /< AVClass for the private context
    profiles*: ptr AVProfile    ## /< array of recognized profiles, or NULL if unknown, array is terminated by {FF_PROFILE_UNKNOWN}
    wrapper_name*: cstring
    priv_data_size*: cint
    next*: ptr AVCodec
    update_thread_context*: proc (dst: ptr AVCodecContext; src: ptr AVCodecContext): cint
    defaults*: ptr AVCodecDefault
    init_static_data*: proc (codec: ptr AVCodec)
    init*: proc (a1: ptr AVCodecContext): cint
    encode_sub*: proc (a1: ptr AVCodecContext; buf: ptr uint8; buf_size: cint;
                     sub: ptr AVSubtitle): cint
    encode2*: proc (avctx: ptr AVCodecContext; avpkt: ptr AVPacket; frame: ptr AVFrame;
                  got_packet_ptr: ptr cint): cint
    decode*: proc (a1: ptr AVCodecContext; outdata: pointer; outdata_size: ptr cint;
                 avpkt: ptr AVPacket): cint
    close*: proc (a1: ptr AVCodecContext): cint
    receive_packet*: proc (avctx: ptr AVCodecContext; avpkt: ptr AVPacket): cint
    receive_frame*: proc (avctx: ptr AVCodecContext; frame: ptr AVFrame): cint
    flush*: proc (a1: ptr AVCodecContext)
    caps_internal*: cint
    bsfs*: cstring
    hw_configs*: ptr ptr AVCodecHWConfigInternal
    codec_tags*: ptr uint32

  AVIODirEntry* {.bycopy.} = object
    name*: cstring
    `type`*: cint
    utf8*: cint
    size*: int64
    modification_timestamp*: int64
    access_timestamp*: int64
    status_change_timestamp*: int64
    user_id*: int64
    group_id*: int64
    filemode*: int64

  AVIODirContext* {.bycopy.} = object
    url_context*: ptr URLContext



  AVIOContext* {.bycopy.} = object
    av_class*: ptr AVClass
    buffer*: ptr cuchar
    buffer_size*: cint
    buf_ptr*: ptr cuchar
    buf_end*: ptr cuchar
    opaque*: pointer
    read_packet*: proc (opaque: pointer; buf: ptr uint8; buf_size: cint): cint
    write_packet*: proc (opaque: pointer; buf: ptr uint8; buf_size: cint): cint
    seek*: proc (opaque: pointer; offset: int64; whence: cint): int64
    pos*: int64
    eof_reached*: cint
    write_flag*: cint
    max_packet_size*: cint
    checksum*: culong
    checksum_ptr*: ptr cuchar
    update_checksum*: proc (checksum: culong; buf: ptr uint8; size: cuint): culong
    error*: cint
    read_pause*: proc (opaque: pointer; pause: cint): cint
    read_seek*: proc (opaque: pointer; stream_index: cint; timestamp: int64;
                    flags: cint): int64
    seekable*: cint
    maxsize*: int64
    direct*: cint
    bytes_read*: int64
    seek_count*: cint
    writeout_count*: cint
    orig_buffer_size*: cint
    short_seek_threshold*: cint
    protocol_whitelist*: cstring
    protocol_blacklist*: cstring
    write_data_type*: proc (opaque: pointer; buf: ptr uint8; buf_size: cint;
                          `type`: AVIODataMarkerType; time: int64): cint
    ignore_boundary_point*: cint
    current_type*: AVIODataMarkerType
    last_time*: int64
    short_seek_get*: proc (opaque: pointer): cint
    written*: int64
    buf_ptr_max*: ptr cuchar
    min_packet_size*: cint

  AVCodecParserContext* {.bycopy.} = object
    priv_data*: pointer
    parser*: ptr AVCodecParser
    frame_offset*: int64
    cur_offset*: int64
    next_frame_offset*: int64
    pict_type*: cint
    repeat_pict*: cint
    pts*: int64
    dts*: int64
    last_pts*: int64
    last_dts*: int64
    fetch_timestamp*: cint
    cur_frame_start_index*: cint
    cur_frame_offset*: array[4, int64]
    cur_frame_pts*: array[4, int64]
    cur_frame_dts*: array[4, int64]
    flags*: cint               ##  Set if the parser has a valid file offset
    offset*: int64           ## /< byte offset from starting packet start
    cur_frame_end*: array[4, int64]
    key_frame*: cint
    convergence_duration*: int64
    dts_sync_point*: cint
    dts_ref_dts_delta*: cint
    pts_dts_delta*: cint
    cur_frame_pos*: array[4, int64]
    pos*: int64
    last_pos*: int64
    duration*: cint
    field_order*: AVFieldOrder
    picture_structure*: AVPictureStructure
    output_picture_number*: cint
    width*: cint
    height*: cint
    coded_width*: cint
    coded_height*: cint
    format*: cint

  AVStreamInternal = object

  AVStream* {.bycopy.} = object
    index*: cint
    id*: cint
    codec*: ptr AVCodecContext
    priv_data*: pointer
    time_base*: AVRational
    start_time*: int64
    duration*: int64
    nb_frames*: int64        ## /< number of frames in this stream if known or 0
    disposition*: cint
    `discard`*: AVDiscard      ## /< Selects which packets can be discarded at will and do not need to be demuxed.
    sample_aspect_ratio*: AVRational
    metadata*: ptr AVDictionary
    avg_frame_rate*: AVRational
    attached_pic*: AVPacket
    side_data*: ptr AVPacketSideData
    nb_side_data*: cint
    event_flags*: cint
    r_frame_rate*: AVRational
    recommended_encoder_configuration*: cstring
    codecpar*: ptr AVCodecParameters
    info*: ptr AVStreamInfo
    pts_wrap_bits*: cint       ##  Timestamp generation support:
    first_dts*: int64
    cur_dts*: int64
    last_IP_pts*: int64
    last_IP_duration*: cint
    probe_packets*: cint
    codec_info_nb_frames*: cint
    need_parsing*: AVStreamParseType
    parser*: ptr AVCodecParserContext
    last_in_packet_buffer*: ptr AVPacketList
    probe_data*: AVProbeData
    pts_buffer*: array[16 + 1, int64]
    index_entries*: ptr AVIndexEntry
    nb_index_entries*: cint
    index_entries_allocated_size*: cuint
    stream_identifier*: cint
    program_num*: cint
    pmt_version*: cint
    pmt_stream_idx*: cint
    interleaver_chunk_size*: int64
    interleaver_chunk_duration*: int64
    request_probe*: cint
    skip_to_keyframe*: cint
    skip_samples*: cint
    start_skip_samples*: int64
    first_discard_sample*: int64
    last_discard_sample*: int64
    nb_decoded_frames*: cint
    mux_ts_offset*: int64
    pts_wrap_reference*: int64
    pts_wrap_behavior*: cint
    update_initial_durations_done*: cint
    pts_reorder_error*: array[16 + 1, int64]
    pts_reorder_error_count*: array[16 + 1, uint8]
    last_dts_for_order_check*: int64
    dts_ordered*: uint8
    dts_misordered*: uint8
    inject_global_side_data*: cint
    display_aspect_ratio*: AVRational
    internal*: ptr AVStreamInternal

  AVProgram* {.bycopy.} = object
    id*: cint
    flags*: cint
    `discard`*: AVDiscard      ## /< selects which program to discard and which to feed to the caller
    stream_index*: ptr cuint
    nb_stream_indexes*: cuint
    metadata*: ptr AVDictionary
    program_num*: cint
    pmt_pid*: cint
    pcr_pid*: cint
    pmt_version*: cint
    start_time*: int64
    end_time*: int64
    pts_wrap_reference*: int64 ## /< reference dts for wrap detection
    pts_wrap_behavior*: cint   ## /< behavior on wrap detection

  AVChapter* {.bycopy.} = object
    id*: cint                  ## /< unique ID to identify the chapter
    time_base*: AVRational     ## /< time base in which the start/end timestamps are specified
    start*: int64
    `end`*: int64            ## /< chapter start/end time in time_base units
    metadata*: ptr AVDictionary

  AVBitStreamFilterContext* {.bycopy.} = object
    priv_data*: pointer
    filter*: ptr AVBitStreamFilter
    parser*: ptr AVCodecParserContext
    next*: ptr AVBitStreamFilterContext
    args*: cstring


  AVIOInterruptCB* {.bycopy.} = object
    callback*: proc (a1: pointer): cint
    opaque*: pointer


  av_format_control_message* = proc (s: ptr AVFormatContext; `type`: cint;
                                  data: pointer; data_size: csize_t): cint
  AVOpenCallback* = proc (s: ptr AVFormatContext; pb: ptr ptr AVIOContext; url: cstring;
                       flags: cint; int_cb: ptr AVIOInterruptCB;
                       options: ptr ptr AVDictionary): cint
  AVFormatInternal = object
  AVFormatContext* {.bycopy.} = object
    av_class*: ptr AVClass
    iformat*: ptr AVInputFormat
    oformat*: ptr AVOutputFormat
    priv_data*: pointer
    pb*: ptr AVIOContext
    ctx_flags*: cint
    nb_streams*: cuint
    streams*: ptr ptr AVStream
    filename*: array[1024, char]
    url*: cstring
    start_time*: int64
    duration*: int64
    bit_rate*: int64
    packet_size*: cuint
    max_delay*: cint
    flags*: cint
    probesize*: int64
    max_analyze_duration*: int64
    key*: ptr uint8
    keylen*: cint
    nb_programs*: cuint
    programs*: ptr ptr AVProgram
    video_codec_id*: AVCodecID
    audio_codec_id*: AVCodecID
    subtitle_codec_id*: AVCodecID
    max_index_size*: cuint
    max_picture_buffer*: cuint
    nb_chapters*: cuint
    chapters*: ptr ptr AVChapter
    metadata*: ptr AVDictionary
    start_time_realtime*: int64
    fps_probe_size*: cint
    error_recognition*: cint
    interrupt_callback*: AVIOInterruptCB
    debug*: cint
    max_interleave_delta*: int64
    strict_std_compliance*: cint
    event_flags*: cint
    max_ts_probe*: cint
    avoid_negative_ts*: cint
    ts_id*: cint
    audio_preload*: cint
    max_chunk_duration*: cint
    max_chunk_size*: cint
    use_wallclock_as_timestamps*: cint
    avio_flags*: cint
    duration_estimation_method*: AVDurationEstimationMethod
    skip_initial_bytes*: int64
    correct_ts_overflow*: cuint
    seek2any*: cint
    flush_packets*: cint
    probe_score*: cint
    format_probesize*: cint
    codec_whitelist*: cstring
    format_whitelist*: cstring
    internal*: ptr AVFormatInternal
    io_repositioned*: cint
    video_codec*: ptr AVCodec
    audio_codec*: ptr AVCodec
    subtitle_codec*: ptr AVCodec
    data_codec*: ptr AVCodec
    metadata_header_padding*: cint
    opaque*: pointer
    control_message_cb*: av_format_control_message
    output_ts_offset*: int64
    dump_separator*: ptr uint8
    data_codec_id*: AVCodecID
    open_cb*: proc (s: ptr AVFormatContext; p: ptr ptr AVIOContext; url: cstring;
                  flags: cint; int_cb: ptr AVIOInterruptCB;
                  options: ptr ptr AVDictionary): cint
    protocol_whitelist*: cstring
    io_open*: proc (s: ptr AVFormatContext; pb: ptr ptr AVIOContext; url: cstring;
                  flags: cint; options: ptr ptr AVDictionary): cint
    io_close*: proc (s: ptr AVFormatContext; pb: ptr AVIOContext)
    protocol_blacklist*: cstring
    max_streams*: cint
    skip_estimate_duration_from_pts*: cint
    max_probe_packets*: cint

  AVDeviceCapabilitiesQuery* {.bycopy.} = object
    av_class*: ptr AVClass
    device_context*: ptr AVFormatContext
    codec*: AVCodecID
    sample_format*: AVSampleFormat
    pixel_format*: AVPixelFormat
    sample_rate*: cint
    channels*: cint
    channel_layout*: int64
    window_width*: cint
    window_height*: cint
    frame_width*: cint
    frame_height*: cint
    fps*: AVRational
  AVCodecTag* {.bycopy.} = object

  AVProbeData* {.bycopy.} = object
    filename*: cstring
    buf*: ptr cuchar
    buf_size*: cint
    mime_type*: cstring

  AVOutputFormat* {.bycopy.} = object
    name*: cstring
    long_name*: cstring
    mime_type*: cstring
    extensions*: cstring
    audio_codec*: AVCodecID
    video_codec*: AVCodecID
    subtitle_codec*: AVCodecID
    flags*: cint
    codec_tag*: ptr ptr AVCodecTag
    priv_class*: ptr AVClass    ## /< AVClass for the private context
    next*: ptr AVOutputFormat
    priv_data_size*: cint
    write_header*: proc (a1: ptr AVFormatContext): cint
    write_packet*: proc (a1: ptr AVFormatContext; pkt: ptr AVPacket): cint
    write_trailer*: proc (a1: ptr AVFormatContext): cint
    interleave_packet*: proc (a1: ptr AVFormatContext; `out`: ptr AVPacket;
                            `in`: ptr AVPacket; flush: cint): cint
    query_codec*: proc (id: AVCodecID; std_compliance: cint): cint
    get_output_timestamp*: proc (s: ptr AVFormatContext; stream: cint;
                               dts: ptr int64; wall: ptr int64)
    control_message*: proc (s: ptr AVFormatContext; `type`: cint; data: pointer;
                          data_size: csize_t): cint
    write_uncoded_frame*: proc (a1: ptr AVFormatContext; stream_index: cint;
                              frame: ptr ptr AVFrame; flags: cuint): cint
    get_device_list*: proc (s: ptr AVFormatContext; device_list: ptr AVDeviceInfoList): cint
    create_device_capabilities*: proc (s: ptr AVFormatContext;
                                     caps: ptr AVDeviceCapabilitiesQuery): cint
    free_device_capabilities*: proc (s: ptr AVFormatContext;
                                   caps: ptr AVDeviceCapabilitiesQuery): cint
    data_codec*: AVCodecID
    init*: proc (a1: ptr AVFormatContext): cint
    deinit*: proc (a1: ptr AVFormatContext)
    check_bitstream*: proc (a1: ptr AVFormatContext; pkt: ptr AVPacket): cint

  AVInputFormat* {.bycopy.} = object
    name*: cstring
    long_name*: cstring
    flags*: cint
    extensions*: cstring
    codec_tag*: ptr ptr AVCodecTag
    priv_class*: ptr AVClass    ## /< AVClass for the private context
    mime_type*: cstring
    next*: ptr AVInputFormat
    raw_codec_id*: cint
    priv_data_size*: cint
    read_probe*: proc (a1: ptr AVProbeData): cint
    read_header*: proc (a1: ptr AVFormatContext): cint
    read_packet*: proc (a1: ptr AVFormatContext; pkt: ptr AVPacket): cint
    read_close*: proc (a1: ptr AVFormatContext): cint
    read_seek*: proc (a1: ptr AVFormatContext; stream_index: cint; timestamp: int64;
                    flags: cint): cint
    read_timestamp*: proc (s: ptr AVFormatContext; stream_index: cint;
                         pos: ptr int64; pos_limit: int64): int64
    read_play*: proc (a1: ptr AVFormatContext): cint
    read_pause*: proc (a1: ptr AVFormatContext): cint
    read_seek2*: proc (s: ptr AVFormatContext; stream_index: cint; min_ts: int64;
                     ts: int64; max_ts: int64; flags: cint): cint
    get_device_list*: proc (s: ptr AVFormatContext; device_list: ptr AVDeviceInfoList): cint
    create_device_capabilities*: proc (s: ptr AVFormatContext;
                                     caps: ptr AVDeviceCapabilitiesQuery): cint
    free_device_capabilities*: proc (s: ptr AVFormatContext;
                                   caps: ptr AVDeviceCapabilitiesQuery): cint


  AVIndexEntry* {.bycopy.} = object
    pos*: int64
    timestamp*: int64
    flags* {.bitsize: 2.}: cint
    size* {.bitsize: 30.}: cint  ## Yeah, trying to keep the size of this small to reduce memory requirements (it is 24 vs. 32 bytes due to possible 8-byte alignment).
    min_distance*: cint

  AVStreamInfo* {.bycopy.} = object
    last_dts*: int64
    duration_gcd*: int64
    duration_count*: cint
    rfps_duration_sum*: int64
    duration_error*: array[2, array[(30 * 12 + 30 + 3 + 6), cdouble]]
    codec_info_duration*: int64
    codec_info_duration_fields*: int64
    frame_delay_evidence*: cint
    found_decoder*: cint
    last_duration*: int64
    fps_first_dts*: int64
    fps_first_dts_idx*: cint
    fps_last_dts*: int64
    fps_last_dts_idx*: cint

  AVCodecParser* {.bycopy.} = object
    codec_ids*: array[5, cint]
    priv_data_size*: cint
    parser_init*: proc (s: ptr AVCodecParserContext): cint
    parser_parse*: proc (s: ptr AVCodecParserContext; avctx: ptr AVCodecContext;
                       poutbuf: ptr ptr uint8; poutbuf_size: ptr cint;
                       buf: ptr uint8; buf_size: cint): cint
    parser_close*: proc (s: ptr AVCodecParserContext)
    split*: proc (avctx: ptr AVCodecContext; buf: ptr uint8; buf_size: cint): cint
    next*: ptr AVCodecParser

  avfilter_action_func* = proc (ctx: ptr AVFilterContext; arg: pointer; jobnr: cint;
                             nb_jobs: cint): cint
  avfilter_execute_func* = proc (ctx: ptr AVFilterContext;
                              `func`: ptr avfilter_action_func; arg: pointer;
                              result: ptr cint; nb_jobs: cint): cint
  AVFilterGraphInternal = object
  AVFilterGraph* {.bycopy.} = object
    av_class*: ptr AVClass
    filters*: ptr ptr AVFilterContext
    nb_filters*: cuint
    scale_sws_opts*: cstring   ## /< sws options to use for the auto-inserted scale filters
    resample_lavr_opts*: cstring ## /< libavresample options to use for the auto-inserted resample filters
    thread_type*: cint
    nb_threads*: cint
    internal*: ptr AVFilterGraphInternal
    opaque*: pointer
    execute*: ptr avfilter_execute_func
    aresample_swr_opts*: cstring ## /< swr options to use for the auto-inserted aresample filters, Access ONLY through AVOptions
    sink_links*: ptr ptr AVFilterLink
    sink_links_count*: cint
    disable_auto_convert*: cuint

  AVFilterPad = object
  AVFilterInternal = object
  AVFilterCommand = object
  AVFilterFormats = object
  AVFilterChannelLayouts = object
  AVFilter* {.bycopy.} = object
    name*: cstring
    description*: cstring
    inputs*: ptr AVFilterPad
    outputs*: ptr AVFilterPad
    priv_class*: ptr AVClass
    flags*: cint
    preinit*: proc (ctx: ptr AVFilterContext): cint
    init*: proc (ctx: ptr AVFilterContext): cint
    init_dict*: proc (ctx: ptr AVFilterContext; options: ptr ptr AVDictionary): cint
    uninit*: proc (ctx: ptr AVFilterContext)
    query_formats*: proc (a1: ptr AVFilterContext): cint
    priv_size*: cint           ## /< size of private data to allocate for the filter
    flags_internal*: cint      ## /< Additional flags for avfilter internal use only.
    next*: ptr AVFilter
    process_command*: proc (a1: ptr AVFilterContext; cmd: cstring; arg: cstring;
                          res: cstring; res_len: cint; flags: cint): cint
    init_opaque*: proc (ctx: ptr AVFilterContext; opaque: pointer): cint
    activate*: proc (ctx: ptr AVFilterContext): cint

  AVFilterContext* {.bycopy.} = object
    av_class*: ptr AVClass      ## /< needed for av_log() and filters common options
    filter*: ptr AVFilter       ## /< the AVFilter of which this is an instance
    name*: cstring             ## /< name of this filter instance
    input_pads*: ptr AVFilterPad ## /< array of input pads
    inputs*: ptr ptr AVFilterLink ## /< array of pointers to input links
    nb_inputs*: cuint          ## /< number of input pads
    output_pads*: ptr AVFilterPad ## /< array of output pads
    outputs*: ptr ptr AVFilterLink ## /< array of pointers to output links
    nb_outputs*: cuint         ## /< number of output pads
    priv*: pointer             ## /< private data for use by the filter
    graph*: ptr AVFilterGraph   ## /< filtergraph this filter belongs to
    thread_type*: cint
    internal*: ptr AVFilterInternal
    command_queue*: ptr AVFilterCommand
    enable_str*: cstring       ## /< enable expression string
    enable*: pointer           ## /< parsed expression (AVExpr*)
    var_values*: ptr cdouble    ## /< variable values for the enable expression
    is_disabled*: cint         ## /< the enabled state from the last expression evaluation
    hw_device_ctx*: ptr AVBufferRef
    nb_threads*: cint
    ready*: cuint
    extra_hw_frames*: cint

  AVFilterFormatsConfig* {.bycopy.} = object
    formats*: ptr AVFilterFormats
    samplerates*: ptr AVFilterFormats
    channel_layouts*: ptr AVFilterChannelLayouts

  AVLINKEnum* {.size: sizeof(cint).} = enum
    AVLINK_UNINIT = 0,          ## /< not started
    AVLINK_STARTINIT,         ## /< started, but incomplete
    AVLINK_INIT               ## /< complete


  AVFilterLink* {.bycopy.} = object
    src*: ptr AVFilterContext   ## /< source filter
    srcpad*: ptr AVFilterPad    ## /< output pad on the source filter
    dst*: ptr AVFilterContext   ## /< dest filter
    dstpad*: ptr AVFilterPad    ## /< input pad on the dest filter
    `type`*: AVMediaType       ## /< filter media type
    w*: cint                   ## /< agreed upon image width
    h*: cint                   ## /< agreed upon image height
    sample_aspect_ratio*: AVRational ## /< agreed upon sample aspect ratio
    channel_layout*: uint64  ## /< channel layout of current buffer (see libavutil/channel_layout.h)
    sample_rate*: cint         ## /< samples per second
    format*: cint              ## /< agreed upon media format
    time_base*: AVRational
    incfg*: AVFilterFormatsConfig
    outcfg*: AVFilterFormatsConfig
    init_state*: AVLINKEnum
    graph*: ptr AVFilterGraph
    current_pts*: int64
    current_pts_us*: int64
    age_index*: cint
    frame_rate*: AVRational
    partial_buf*: ptr AVFrame
    partial_buf_size*: cint
    min_samples*: cint
    max_samples*: cint
    channels*: cint
    flags*: cuint
    frame_count_in*: int64
    frame_count_out*: int64
    frame_pool*: pointer
    frame_wanted_out*: cint
    hw_frames_ctx*: ptr AVBufferRef
    reserved*: array[0x0000F000, char]

type
  SDL_TouchID* = int64
  SDL_FingerID* = int64
  SDL_TouchDeviceType* {.size: sizeof(cint).} = enum
    SDL_TOUCH_DEVICE_INVALID = -1, SDL_TOUCH_DEVICE_DIRECT,
    SDL_TOUCH_DEVICE_INDIRECT_ABSOLUTE, SDL_TOUCH_DEVICE_INDIRECT_RELATIVE
  SDL_Finger* {.bycopy.} = object
    id*: SDL_FingerID
    x*: cfloat
    y*: cfloat
    pressure*: cfloat
type
  SDL_GestureID* = int64
type
  SDL_EventType* {.size: sizeof(cint).} = enum
    SDL_FIRSTEVENT = 0, SDL_QUIT_ENUM = 0x00000100, SDL_APP_TERMINATING, SDL_APP_LOWMEMORY,
    SDL_APP_WILLENTERBACKGROUND, SDL_APP_DIDENTERBACKGROUND,
    SDL_APP_WILLENTERFOREGROUND, SDL_APP_DIDENTERFOREGROUND,
    SDL_DISPLAYEVENT_ENUM = 0x00000150, SDL_WINDOWEVENT_ENUM = 0x00000200, SDL_SYSWMEVENT_ENUM,
    SDL_KEYDOWN = 0x00000300, SDL_KEYUP, SDL_TEXTEDITING, SDL_TEXTINPUT,
    SDL_KEYMAPCHANGED, SDL_MOUSEMOTION = 0x00000400, SDL_MOUSEBUTTONDOWN,
    SDL_MOUSEBUTTONUP, SDL_MOUSEWHEEL, SDL_JOYAXISMOTION = 0x00000600,
    SDL_JOYBALLMOTION, SDL_JOYHATMOTION, SDL_JOYBUTTONDOWN, SDL_JOYBUTTONUP,
    SDL_JOYDEVICEADDED, SDL_JOYDEVICEREMOVED,
    SDL_CONTROLLERAXISMOTION = 0x00000650, SDL_CONTROLLERBUTTONDOWN,
    SDL_CONTROLLERBUTTONUP, SDL_CONTROLLERDEVICEADDED,
    SDL_CONTROLLERDEVICEREMOVED, SDL_CONTROLLERDEVICEREMAPPED,
    SDL_FINGERDOWN = 0x00000700, SDL_FINGERUP, SDL_FINGERMOTION,
    SDL_DOLLARGESTURE = 0x00000800, SDL_DOLLARRECORD, SDL_MULTIGESTURE,
    SDL_CLIPBOARDUPDATE = 0x00000900, SDL_DROPFILE = 0x00001000, SDL_DROPTEXT,
    SDL_DROPBEGIN, SDL_DROPCOMPLETE, SDL_AUDIODEVICEADDED = 0x00001100,
    SDL_AUDIODEVICEREMOVED, SDL_SENSORUPDATE_ENUM = 0x00001200,
    SDL_RENDER_TARGETS_RESET = 0x00002000, SDL_RENDER_DEVICE_RESET,
    SDL_USEREVENT_ENUM = 0x00008000, SDL_LASTEVENT = 0x0000FFFF

  

  SDL_CommonEvent* {.bycopy.} = object
    `type`*: uint32
    timestamp*: uint32

  SDL_DisplayEvent* {.bycopy.} = object
    `type`*: uint32
    timestamp*: uint32
    display*: uint32
    event*: uint8
    padding1*: uint8
    padding2*: uint8
    padding3*: uint8
    data1*: int32

  SDL_WindowEvent* {.bycopy.} = object
    `type`*: uint32
    timestamp*: uint32
    windowID*: uint32
    event*: uint8
    padding1*: uint8
    padding2*: uint8
    padding3*: uint8
    data1*: int32
    data2*: int32

  SDL_KeyCode = cint

  SDL_Keysym* {.bycopy.} = object
    scancode*: SDL_ScanCode
    sym*: SDL_KeyCode
    `mod`*: uint16
    unused*: uint32

  SDL_KeyboardEvent* {.bycopy.} = object
    `type`*: uint32
    timestamp*: uint32
    windowID*: uint32
    state*: uint8
    repeat*: uint8
    padding2*: uint8
    padding3*: uint8
    keysym*: SDL_Keysym

  SDL_TextEditingEvent* {.bycopy.} = object
    `type`*: uint32
    timestamp*: uint32
    windowID*: uint32
    text*: array[(32), char]
    start*: int32
    length*: int32

  SDL_TextInputEvent* {.bycopy.} = object
    `type`*: uint32
    timestamp*: uint32
    windowID*: uint32
    text*: array[32, char]

  SDL_MouseMotionEvent* {.bycopy.} = object
    `type`*: uint32
    timestamp*: uint32
    windowID*: uint32
    which*: uint32
    state*: uint32
    x*: int32
    y*: int32
    xrel*: int32
    yrel*: int32

  SDL_MouseButtonEvent* {.bycopy.} = object
    `type`*: uint32
    timestamp*: uint32
    windowID*: uint32
    which*: uint32
    button*: uint8
    state*: uint8
    clicks*: uint8
    padding1*: uint8
    x*: int32
    y*: int32

  SDL_MouseWheelEvent* {.bycopy.} = object
    `type`*: uint32
    timestamp*: uint32
    windowID*: uint32
    which*: uint32
    x*: int32
    y*: int32
    direction*: uint32

  SDL_JoyAxisEvent* {.bycopy.} = object
    `type`*: uint32
    timestamp*: uint32
    which*: int32
    axis*: uint8
    padding1*: uint8
    padding2*: uint8
    padding3*: uint8
    value*: int16
    padding4*: uint16

  SDL_JoyBallEvent* {.bycopy.} = object
    `type`*: uint32
    timestamp*: uint32
    which*: int32
    ball*: uint8
    padding1*: uint8
    padding2*: uint8
    padding3*: uint8
    xrel*: int16
    yrel*: int16

  SDL_JoyHatEvent* {.bycopy.} = object
    `type`*: uint32
    timestamp*: uint32
    which*: int32
    hat*: uint8
    value*: uint8
    padding1*: uint8
    padding2*: uint8

  SDL_JoyButtonEvent* {.bycopy.} = object
    `type`*: uint32
    timestamp*: uint32
    which*: int32
    button*: uint8
    state*: uint8
    padding1*: uint8
    padding2*: uint8

  SDL_JoyDeviceEvent* {.bycopy.} = object
    `type`*: uint32
    timestamp*: uint32
    which*: int32

  SDL_ControllerAxisEvent* {.bycopy.} = object
    `type`*: uint32
    timestamp*: uint32
    which*: int32
    axis*: uint8
    padding1*: uint8
    padding2*: uint8
    padding3*: uint8
    value*: int16
    padding4*: uint16

  SDL_ControllerButtonEvent* {.bycopy.} = object
    `type`*: uint32
    timestamp*: uint32
    which*: int32
    button*: uint8
    state*: uint8
    padding1*: uint8
    padding2*: uint8

  SDL_ControllerDeviceEvent* {.bycopy.} = object
    `type`*: uint32
    timestamp*: uint32
    which*: int32

  SDL_AudioDeviceEvent* {.bycopy.} = object
    `type`*: uint32
    timestamp*: uint32
    which*: uint32
    iscapture*: uint8
    padding1*: uint8
    padding2*: uint8
    padding3*: uint8

  SDL_TouchFingerEvent* {.bycopy.} = object
    `type`*: uint32
    timestamp*: uint32
    touchId*: SDL_TouchID
    fingerId*: SDL_FingerID
    x*: cfloat
    y*: cfloat
    dx*: cfloat
    dy*: cfloat
    pressure*: cfloat
    windowID*: uint32

  SDL_MultiGestureEvent* {.bycopy.} = object
    `type`*: uint32
    timestamp*: uint32
    touchId*: SDL_TouchID
    dTheta*: cfloat
    dDist*: cfloat
    x*: cfloat
    y*: cfloat
    numFingers*: uint16
    padding*: uint16

  SDL_DollarGestureEvent* {.bycopy.} = object
    `type`*: uint32
    timestamp*: uint32
    touchId*: SDL_TouchID
    gestureId*: SDL_GestureID
    numFingers*: uint32
    error*: cfloat
    x*: cfloat
    y*: cfloat

  SDL_DropEvent* {.bycopy.} = object
    `type`*: uint32
    timestamp*: uint32
    file*: cstring
    windowID*: uint32

  SDL_SensorEvent* {.bycopy.} = object
    `type`*: uint32
    timestamp*: uint32
    which*: int32
    data*: array[6, cfloat]

  SDL_QuitEvent* {.bycopy.} = object
    `type`*: uint32
    timestamp*: uint32

  SDL_OSEvent* {.bycopy.} = object
    `type`*: uint32
    timestamp*: uint32

  SDL_UserEvent* {.bycopy.} = object
    `type`*: uint32
    timestamp*: uint32
    windowID*: uint32
    code*: int32
    data1*: pointer
    data2*: pointer

  SDL_SysWMmsg* {.bycopy.} = object

  SDL_SysWMEvent* {.bycopy.} = object
    `type`*: uint32
    timestamp*: uint32
    msg*: ptr SDL_SysWMmsg

  SDL_Event* {.union.} = object
    `type`*: uint32
    common*: SDL_CommonEvent
    display*: SDL_DisplayEvent
    window*: SDL_WindowEvent
    key*: SDL_KeyboardEvent
    edit*: SDL_TextEditingEvent
    text*: SDL_TextInputEvent
    motion*: SDL_MouseMotionEvent
    button*: SDL_MouseButtonEvent
    wheel*: SDL_MouseWheelEvent
    jaxis*: SDL_JoyAxisEvent
    jball*: SDL_JoyBallEvent
    jhat*: SDL_JoyHatEvent
    jbutton*: SDL_JoyButtonEvent
    jdevice*: SDL_JoyDeviceEvent
    caxis*: SDL_ControllerAxisEvent
    cbutton*: SDL_ControllerButtonEvent
    cdevice*: SDL_ControllerDeviceEvent
    adevice*: SDL_AudioDeviceEvent
    sensor*: SDL_SensorEvent
    quit*: SDL_QuitEvent
    user*: SDL_UserEvent
    syswm*: SDL_SysWMEvent
    tfinger*: SDL_TouchFingerEvent
    mgesture*: SDL_MultiGestureEvent
    dgesture*: SDL_DollarGestureEvent
    drop*: SDL_DropEvent
    padding*: array[56, uint8]

  SDL_compile_time_assert_SDL_Event* = array[(sizeof((SDL_Event)) == 56).int * 2 - 1, cint]
type
  AVHWFrameTransferDirection* {.size: sizeof(byte).} = enum
    AV_HWFRAME_TRANSFER_DIRECTION_FROM, AV_HWFRAME_TRANSFER_DIRECTION_TO

const
  AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX* = 0x00000001
  AV_CODEC_HW_CONFIG_METHOD_HW_FRAMES_CTX* = 0x00000002
  AV_CODEC_HW_CONFIG_METHOD_INTERNAL* = 0x00000004
  AV_CODEC_HW_CONFIG_METHOD_AD_HOC* = 0x00000008
  AV_OPT_FLAG_IMPLICIT_KEY* = 1

proc av_get_channel_layout*(name: cstring): uint64
proc av_get_extended_channel_layout*(name: cstring; channel_layout: ptr uint64;
                                    nb_channels: ptr cint): cint
proc av_get_channel_layout_string*(buf: cstring; buf_size: cint; nb_channels: cint;
                                  channel_layout: uint64)

proc av_bprint_channel_layout*(bp: ptr AVBPrint; nb_channels: cint;
                              channel_layout: uint64)
proc av_get_default_channel_layout*(nb_channels: cint): int64
proc av_get_channel_layout_channel_index*(channel_layout: uint64;
    channel: uint64): cint
proc av_channel_layout_extract_channel*(channel_layout: uint64; index: cint): uint64
proc av_get_channel_name*(channel: uint64): cstring
proc av_get_channel_description*(channel: uint64): cstring
proc av_get_standard_channel_layout*(index: cuint; layout: ptr uint64;name: cstringArray): cint
proc av_frame_get_best_effort_timestamp*(frame: ptr AVFrame): int64
proc av_frame_set_best_effort_timestamp*(frame: ptr AVFrame; val: int64)
proc av_frame_get_pkt_duration*(frame: ptr AVFrame): int64
proc av_frame_set_pkt_duration*(frame: ptr AVFrame; val: int64)
proc av_frame_get_pkt_pos*(frame: ptr AVFrame): int64
proc av_frame_set_pkt_pos*(frame: ptr AVFrame; val: int64)
proc av_frame_get_channel_layout*(frame: ptr AVFrame): int64
proc av_frame_set_channel_layout*(frame: ptr AVFrame; val: int64)
proc av_frame_get_channels*(frame: ptr AVFrame): cint
proc av_frame_set_channels*(frame: ptr AVFrame; val: cint)
proc av_frame_get_sample_rate*(frame: ptr AVFrame): cint
proc av_frame_set_sample_rate*(frame: ptr AVFrame; val: cint)
proc av_frame_get_metadata*(frame: ptr AVFrame): ptr AVDictionary
proc av_frame_set_metadata*(frame: ptr AVFrame; val: ptr AVDictionary)
proc av_frame_get_decode_error_flags*(frame: ptr AVFrame): cint
proc av_frame_set_decode_error_flags*(frame: ptr AVFrame; val: cint)
proc av_frame_get_pkt_size*(frame: ptr AVFrame): cint
proc av_frame_set_pkt_size*(frame: ptr AVFrame; val: cint)
proc av_frame_get_qp_table*(f: ptr AVFrame; stride: ptr cint; `type`: ptr cint): ptr int8
proc av_frame_set_qp_table*(f: ptr AVFrame; buf: ptr AVBufferRef; stride: cint;
                           `type`: cint): cint
proc av_frame_get_colorspace*(frame: ptr AVFrame): AVColorSpace
proc av_frame_set_colorspace*(frame: ptr AVFrame; val: AVColorSpace)
proc av_frame_get_color_range*(frame: ptr AVFrame): AVColorRange
proc av_frame_set_color_range*(frame: ptr AVFrame; val: AVColorRange)
proc av_get_colorspace_name*(val: AVColorSpace): cstring
proc av_frame_alloc*(): ptr AVFrame
proc av_frame_free*(frame: ptr ptr AVFrame)
proc av_frame_ref*(dst: ptr AVFrame; src: ptr AVFrame): cint
proc av_frame_clone*(src: ptr AVFrame): ptr AVFrame
proc av_frame_unref*(frame: ptr AVFrame)
proc av_frame_move_ref*(dst: ptr AVFrame; src: ptr AVFrame)
proc av_frame_get_buffer*(frame: ptr AVFrame; align: cint): cint
proc av_frame_is_writable*(frame: ptr AVFrame): cint
proc av_frame_make_writable*(frame: ptr AVFrame): cint
proc av_frame_copy*(dst: ptr AVFrame; src: ptr AVFrame): cint
proc av_frame_copy_props*(dst: ptr AVFrame; src: ptr AVFrame): cint
proc av_frame_get_plane_buffer*(frame: ptr AVFrame; plane: cint): ptr AVBufferRef
proc av_frame_new_side_data*(frame: ptr AVFrame; `type`: AVFrameSideDataType;
                            size: cint): ptr AVFrameSideData
proc av_frame_new_side_data_from_buf*(frame: ptr AVFrame;
                                     `type`: AVFrameSideDataType;
                                     buf: ptr AVBufferRef): ptr AVFrameSideData
proc av_frame_get_side_data*(frame: ptr AVFrame; `type`: AVFrameSideDataType): ptr AVFrameSideData
proc av_frame_remove_side_data*(frame: ptr AVFrame; `type`: AVFrameSideDataType)


proc av_frame_apply_cropping*(frame: ptr AVFrame; flags: cint): cint
proc av_frame_side_data_name*(`type`: AVFrameSideDataType): cstring
proc av_hwframe_map*(dst: ptr AVFrame; src: ptr AVFrame; flags: cint): cint
proc av_hwframe_ctx_create_derived*(derived_frame_ctx: ptr ptr AVBufferRef;
                                   format: AVPixelFormat;
                                   derived_device_ctx: ptr AVBufferRef;
                                   source_frame_ctx: ptr AVBufferRef; flags: cint): cint

proc av_hwdevice_find_type_by_name*(name: cstring): AVHWDeviceType
proc av_hwdevice_get_type_name*(`type`: AVHWDeviceType): cstring
proc av_hwdevice_iterate_types*(prev: AVHWDeviceType): AVHWDeviceType
proc av_hwdevice_ctx_alloc*(`type`: AVHWDeviceType): ptr AVBufferRef
proc av_hwdevice_ctx_init*(`ref`: ptr AVBufferRef): cint
proc av_hwdevice_ctx_create*(device_ctx: ptr ptr AVBufferRef; `type`: AVHWDeviceType;device: cstring; opts: ptr AVDictionary; flags: cint): cint
proc av_hwdevice_ctx_create_derived*(dst_ctx: ptr ptr AVBufferRef;`type`: AVHWDeviceType;src_ctx: ptr AVBufferRef; flags: cint): cint
proc av_hwdevice_ctx_create_derived_opts*(dst_ctx: ptr ptr AVBufferRef;`type`: AVHWDeviceType; src_ctx: ptr AVBufferRef; options: ptr AVDictionary; flags: cint): cint
proc av_hwframe_ctx_alloc*(device_ctx: ptr AVBufferRef): ptr AVBufferRef
proc av_hwframe_ctx_init*(`ref`: ptr AVBufferRef): cint
proc av_hwframe_get_buffer*(hwframe_ctx: ptr AVBufferRef; frame: ptr AVFrame; flags: cint): cint
proc av_hwframe_transfer_data*(dst: ptr AVFrame; src: ptr AVFrame; flags: cint): cint


proc avcodec_get_hw_config*(codec: ptr AVCodec; index: cint): ptr AVCodecHWConfig


proc av_hwframe_transfer_get_formats*(hwframe_ctx: ptr AVBufferRef; dir: AVHWFrameTransferDirection; formats: ptr ptr AVPixelFormat; flags: cint): cint
proc av_hwdevice_hwconfig_alloc*(device_ctx: ptr AVBufferRef): pointer
proc av_hwdevice_get_hwframe_constraints*(`ref`: ptr AVBufferRef; hwconfig: pointer): ptr AVHWFramesConstraints
proc av_hwframe_constraints_free*(constraints: ptr ptr AVHWFramesConstraints)
proc avcodec_get_type*(codec_id: AVCodecID): AVMediaType
proc avcodec_get_name*(id: AVCodecID): cstring
proc avcodec_parameters_alloc*(): ptr AVCodecParameters
proc avcodec_parameters_free*(par: ptr ptr AVCodecParameters)
proc avcodec_parameters_copy*(dst: ptr AVCodecParameters; src: ptr AVCodecParameters): cint
proc av_packet_alloc*(): ptr AVPacket
proc av_packet_clone*(src: ptr AVPacket): ptr AVPacket
proc av_packet_free*(pkt: ptr ptr AVPacket)
proc av_init_packet*(pkt: ptr AVPacket)
proc av_new_packet*(pkt: ptr AVPacket; size: cint): cint
proc av_shrink_packet*(pkt: ptr AVPacket; size: cint)
proc av_grow_packet*(pkt: ptr AVPacket; grow_by: cint): cint
proc av_packet_from_data*(pkt: ptr AVPacket; data: ptr uint8; size: cint): cint
proc av_dup_packet*(pkt: ptr AVPacket): cint
proc av_copy_packet*(dst: ptr AVPacket; src: ptr AVPacket): cint
proc av_copy_packet_side_data*(dst: ptr AVPacket; src: ptr AVPacket): cint
proc av_free_packet*(pkt: ptr AVPacket)
proc av_packet_new_side_data*(pkt: ptr AVPacket; `type`: AVPacketSideDataType;
                             size: cint): ptr uint8
proc av_packet_add_side_data*(pkt: ptr AVPacket; `type`: AVPacketSideDataType;
                             data: ptr uint8; size: csize_t): cint
proc av_packet_shrink_side_data*(pkt: ptr AVPacket; `type`: AVPacketSideDataType;
                                size: cint): cint
proc av_packet_get_side_data*(pkt: ptr AVPacket; `type`: AVPacketSideDataType;
                             size: ptr cint): ptr uint8
proc av_packet_merge_side_data*(pkt: ptr AVPacket): cint
proc av_packet_split_side_data*(pkt: ptr AVPacket): cint
proc av_packet_side_data_name*(`type`: AVPacketSideDataType): cstring
proc av_packet_pack_dictionary*(dict: ptr AVDictionary; size: ptr cint): ptr uint8
proc av_packet_unpack_dictionary*(data: ptr uint8; size: cint;
                                 dict: ptr ptr AVDictionary): cint
proc av_packet_free_side_data*(pkt: ptr AVPacket)
proc av_packet_ref*(dst: ptr AVPacket; src: ptr AVPacket): cint
proc av_packet_unref*(pkt: ptr AVPacket)
proc av_packet_move_ref*(dst: ptr AVPacket; src: ptr AVPacket)
proc av_packet_copy_props*(dst: ptr AVPacket; src: ptr AVPacket): cint
proc av_packet_make_refcounted*(pkt: ptr AVPacket): cint
proc av_packet_make_writable*(pkt: ptr AVPacket): cint
proc av_packet_rescale_ts*(pkt: ptr AVPacket; tb_src: AVRational; tb_dst: AVRational)
proc av_bsf_get_by_name*(name: cstring): ptr AVBitStreamFilter
proc av_bsf_iterate*(opaque: ptr pointer): ptr AVBitStreamFilter
proc av_bsf_alloc*(filter: ptr AVBitStreamFilter; ctx: ptr ptr AVBSFContext): cint
proc av_bsf_init*(ctx: ptr AVBSFContext): cint
proc av_bsf_send_packet*(ctx: ptr AVBSFContext; pkt: ptr AVPacket): cint
proc av_bsf_receive_packet*(ctx: ptr AVBSFContext; pkt: ptr AVPacket): cint
proc av_bsf_flush*(ctx: ptr AVBSFContext)
proc av_bsf_free*(ctx: ptr ptr AVBSFContext)
proc av_bsf_get_class*(): ptr AVClass

proc av_bsf_list_alloc*(): ptr AVBSFList
proc av_bsf_list_free*(lst: ptr ptr AVBSFList)
proc av_bsf_list_append*(lst: ptr AVBSFList; bsf: ptr AVBSFContext): cint
proc av_bsf_list_append2*(lst: ptr AVBSFList; bsf_name: cstring;
                         options: ptr ptr AVDictionary): cint
proc av_bsf_list_finalize*(lst: ptr ptr AVBSFList; bsf: ptr ptr AVBSFContext): cint
proc av_bsf_list_parse_str*(str: cstring; bsf: ptr ptr AVBSFContext): cint
proc av_bsf_get_null_filter*(bsf: ptr ptr AVBSFContext): cint
proc av_codec_iterate*(opaque: ptr pointer): ptr AVCodec
proc avcodec_find_decoder*(id: AVCodecID): ptr AVCodec
proc avcodec_find_decoder_by_name*(name: cstring): ptr AVCodec
proc avcodec_find_encoder*(id: AVCodecID): ptr AVCodec
proc avcodec_find_encoder_by_name*(name: cstring): ptr AVCodec
proc av_codec_is_encoder*(codec: ptr AVCodec): cint
proc av_codec_is_decoder*(codec: ptr AVCodec): cint
proc avcodec_descriptor_get*(id: AVCodecID): ptr AVCodecDescriptor
proc avcodec_descriptor_next*(prev: ptr AVCodecDescriptor): ptr AVCodecDescriptor
proc avcodec_descriptor_get_by_name*(name: cstring): ptr AVCodecDescriptor
proc av_codec_get_pkt_timebase*(avctx: ptr AVCodecContext): AVRational
proc av_codec_set_pkt_timebase*(avctx: ptr AVCodecContext; val: AVRational)
proc av_codec_get_codec_descriptor*(avctx: ptr AVCodecContext): ptr AVCodecDescriptor
proc av_codec_set_codec_descriptor*(avctx: ptr AVCodecContext;
                                   desc: ptr AVCodecDescriptor)
proc av_codec_get_codec_properties*(avctx: ptr AVCodecContext): cuint
proc av_codec_get_lowres*(avctx: ptr AVCodecContext): cint
proc av_codec_set_lowres*(avctx: ptr AVCodecContext; val: cint)
proc av_codec_get_seek_preroll*(avctx: ptr AVCodecContext): cint
proc av_codec_set_seek_preroll*(avctx: ptr AVCodecContext; val: cint)
proc av_codec_get_chroma_intra_matrix*(avctx: ptr AVCodecContext): ptr uint16
proc av_codec_set_chroma_intra_matrix*(avctx: ptr AVCodecContext; val: ptr uint16)


proc av_codec_get_max_lowres*(codec: ptr AVCodec): cint


proc av_codec_next*(c: ptr AVCodec): ptr AVCodec
proc avcodec_version*(): cuint
proc avcodec_configuration*(): cstring
proc avcodec_license*(): cstring
proc avcodec_register*(codec: ptr AVCodec)
proc avcodec_register_all*()
proc avcodec_alloc_context3*(codec: ptr AVCodec): ptr AVCodecContext
proc avcodec_free_context*(avctx: ptr ptr AVCodecContext)
proc avcodec_get_context_defaults3*(s: ptr AVCodecContext; codec: ptr AVCodec): cint
proc avcodec_get_class*(): ptr AVClass
proc avcodec_get_frame_class*(): ptr AVClass
proc avcodec_get_subtitle_rect_class*(): ptr AVClass
proc avcodec_copy_context*(dest: ptr AVCodecContext; src: ptr AVCodecContext): cint
proc avcodec_parameters_from_context*(par: ptr AVCodecParameters;
                                     codec: ptr AVCodecContext): cint
proc avcodec_parameters_to_context*(codec: ptr AVCodecContext;
                                   par: ptr AVCodecParameters): cint
proc avcodec_open2*(avctx: ptr AVCodecContext; codec: ptr AVCodec;
                   options: ptr ptr AVDictionary): cint
proc avcodec_close*(avctx: ptr AVCodecContext): cint
proc avsubtitle_free*(sub: ptr AVSubtitle)
proc avcodec_default_get_buffer2*(s: ptr AVCodecContext; frame: ptr AVFrame;
                                 flags: cint): cint
proc avcodec_align_dimensions*(s: ptr AVCodecContext; width: ptr cint; height: ptr cint)
proc avcodec_align_dimensions2*(s: ptr AVCodecContext; width: ptr cint;
                               height: ptr cint; linesize_align: array[8, cint])
proc avcodec_enum_to_chroma_pos*(xpos: ptr cint; ypos: ptr cint; pos: AVChromaLocation): cint
proc avcodec_chroma_pos_to_enum*(xpos: cint; ypos: cint): AVChromaLocation
proc avcodec_decode_audio4*(avctx: ptr AVCodecContext; frame: ptr AVFrame;
                           got_frame_ptr: ptr cint; avpkt: ptr AVPacket): cint
proc avcodec_decode_video2*(avctx: ptr AVCodecContext; picture: ptr AVFrame;
                           got_picture_ptr: ptr cint; avpkt: ptr AVPacket): cint
proc avcodec_decode_subtitle2*(avctx: ptr AVCodecContext; sub: ptr AVSubtitle;
                              got_sub_ptr: ptr cint; avpkt: ptr AVPacket): cint
proc avcodec_send_packet*(avctx: ptr AVCodecContext; avpkt: ptr AVPacket): cint
proc avcodec_receive_frame*(avctx: ptr AVCodecContext; frame: ptr AVFrame): cint
proc avcodec_send_frame*(avctx: ptr AVCodecContext; frame: ptr AVFrame): cint
proc avcodec_receive_packet*(avctx: ptr AVCodecContext; avpkt: ptr AVPacket): cint
proc avcodec_get_hw_frames_parameters*(avctx: ptr AVCodecContext;
                                      device_ref: ptr AVBufferRef;
                                      hw_pix_fmt: AVPixelFormat;
                                      out_frames_ref: ptr ptr AVBufferRef): cint



proc av_parser_iterate*(opaque: ptr pointer): ptr AVCodecParser
proc av_parser_next*(c: ptr AVCodecParser): ptr AVCodecParser
proc av_register_codec_parser*(parser: ptr AVCodecParser)
proc av_parser_init*(codec_id: cint): ptr AVCodecParserContext
proc av_parser_parse2*(s: ptr AVCodecParserContext; avctx: ptr AVCodecContext;
                      poutbuf: ptr ptr uint8; poutbuf_size: ptr cint;
                      buf: ptr uint8; buf_size: cint; pts: int64; dts: int64;
                      pos: int64): cint
proc av_parser_change*(s: ptr AVCodecParserContext; avctx: ptr AVCodecContext;
                      poutbuf: ptr ptr uint8; poutbuf_size: ptr cint;
                      buf: ptr uint8; buf_size: cint; keyframe: cint): cint
proc av_parser_close*(s: ptr AVCodecParserContext)
proc avcodec_encode_audio2*(avctx: ptr AVCodecContext; avpkt: ptr AVPacket;
                           frame: ptr AVFrame; got_packet_ptr: ptr cint): cint
proc avcodec_encode_video2*(avctx: ptr AVCodecContext; avpkt: ptr AVPacket;
                           frame: ptr AVFrame; got_packet_ptr: ptr cint): cint
proc avcodec_encode_subtitle*(avctx: ptr AVCodecContext; buf: ptr uint8;
                             buf_size: cint; sub: ptr AVSubtitle): cint
proc avpicture_alloc*(picture: ptr AVPicture; pix_fmt: AVPixelFormat; width: cint;
                     height: cint): cint
proc avpicture_free*(picture: ptr AVPicture)
proc avpicture_fill*(picture: ptr AVPicture; `ptr`: ptr uint8;
                    pix_fmt: AVPixelFormat; width: cint; height: cint): cint
proc avpicture_layout*(src: ptr AVPicture; pix_fmt: AVPixelFormat; width: cint;
                      height: cint; dest: ptr cuchar; dest_size: cint): cint
proc avpicture_get_size*(pix_fmt: AVPixelFormat; width: cint; height: cint): cint
proc av_picture_copy*(dst: ptr AVPicture; src: ptr AVPicture; pix_fmt: AVPixelFormat;
                     width: cint; height: cint)
proc av_picture_crop*(dst: ptr AVPicture; src: ptr AVPicture; pix_fmt: AVPixelFormat;
                     top_band: cint; left_band: cint): cint
proc av_picture_pad*(dst: ptr AVPicture; src: ptr AVPicture; height: cint; width: cint;
                    pix_fmt: AVPixelFormat; padtop: cint; padbottom: cint;
                    padleft: cint; padright: cint; color: ptr cint): cint
proc avcodec_get_chroma_sub_sample*(pix_fmt: AVPixelFormat; h_shift: ptr cint;
                                   v_shift: ptr cint)
proc avcodec_pix_fmt_to_codec_tag*(pix_fmt: AVPixelFormat): cuint
proc avcodec_get_pix_fmt_loss*(dst_pix_fmt: AVPixelFormat;
                              src_pix_fmt: AVPixelFormat; has_alpha: cint): cint
proc avcodec_find_best_pix_fmt_of_list*(pix_fmt_list: ptr AVPixelFormat;
                                       src_pix_fmt: AVPixelFormat;
                                       has_alpha: cint; loss_ptr: ptr cint): AVPixelFormat
proc avcodec_find_best_pix_fmt_of_2*(dst_pix_fmt1: AVPixelFormat;
                                    dst_pix_fmt2: AVPixelFormat;
                                    src_pix_fmt: AVPixelFormat; has_alpha: cint;
                                    loss_ptr: ptr cint): AVPixelFormat
proc avcodec_find_best_pix_fmt2*(dst_pix_fmt1: AVPixelFormat;
                                dst_pix_fmt2: AVPixelFormat;
                                src_pix_fmt: AVPixelFormat; has_alpha: cint;
                                loss_ptr: ptr cint): AVPixelFormat
proc avcodec_default_get_format*(s: ptr AVCodecContext; fmt: ptr AVPixelFormat): AVPixelFormat
proc av_get_codec_tag_string*(buf: cstring; buf_size: csize_t; codec_tag: cuint): csize_t
proc avcodec_string*(buf: cstring; buf_size: cint; enc: ptr AVCodecContext; encode: cint)
proc av_get_profile_name*(codec: ptr AVCodec; profile: cint): cstring
proc avcodec_profile_name*(codec_id: AVCodecID; profile: cint): cstring
proc avcodec_default_execute*(c: ptr AVCodecContext; `func`: proc (
    c2: ptr AVCodecContext; arg2: pointer): cint; arg: pointer; result: ptr cint; count: cint;
                             size: cint): cint
proc avcodec_default_execute2*(c: ptr AVCodecContext; `func`: proc (
    c2: ptr AVCodecContext; arg2: pointer; a3: cint; a4: cint): cint; arg: pointer;
                              result: ptr cint; count: cint): cint
proc avcodec_fill_audio_frame*(frame: ptr AVFrame; nb_channels: cint;
                              sample_fmt: AVSampleFormat; buf: ptr uint8;
                              buf_size: cint; align: cint): cint
proc avcodec_flush_buffers*(avctx: ptr AVCodecContext)
proc av_get_bits_per_sample*(codec_id: AVCodecID): cint
proc av_get_pcm_codec*(fmt: AVSampleFormat; be: cint): AVCodecID
proc av_get_exact_bits_per_sample*(codec_id: AVCodecID): cint
proc av_get_audio_frame_duration*(avctx: ptr AVCodecContext; frame_bytes: cint): cint
proc av_get_audio_frame_duration2*(par: ptr AVCodecParameters; frame_bytes: cint): cint

proc av_register_bitstream_filter*(bsf: ptr AVBitStreamFilter)
proc av_bitstream_filter_init*(name: cstring): ptr AVBitStreamFilterContext
proc av_bitstream_filter_filter*(bsfc: ptr AVBitStreamFilterContext;
                                avctx: ptr AVCodecContext; args: cstring;
                                poutbuf: ptr ptr uint8; poutbuf_size: ptr cint;
                                buf: ptr uint8; buf_size: cint; keyframe: cint): cint
proc av_bitstream_filter_close*(bsf: ptr AVBitStreamFilterContext)
proc av_bitstream_filter_next*(f: ptr AVBitStreamFilter): ptr AVBitStreamFilter
proc av_bsf_next*(opaque: ptr pointer): ptr AVBitStreamFilter
proc av_fast_padded_malloc*(`ptr`: pointer; size: ptr cuint; min_size: csize_t)
proc av_fast_padded_mallocz*(`ptr`: pointer; size: ptr cuint; min_size: csize_t)
proc av_xiphlacing*(s: ptr cuchar; v: cuint): cuint
proc av_register_hwaccel*(hwaccel: ptr AVHWAccel)
proc av_hwaccel_next*(hwaccel: ptr AVHWAccel): ptr AVHWAccel



proc av_lockmgr_register*(cb: proc (mutex: ptr pointer; op: AVLockOp): cint): cint
proc avcodec_is_open*(s: ptr AVCodecContext): cint
proc av_cpb_properties_alloc*(size: ptr csize_t): ptr AVCPBProperties




proc avio_find_protocol_name*(url: cstring): cstring
proc avio_check*(url: cstring; flags: cint): cint
proc avpriv_io_move*(url_src: cstring; url_dst: cstring): cint
proc avpriv_io_delete*(url: cstring): cint
proc avio_open_dir*(s: ptr ptr AVIODirContext; url: cstring;
                   options: ptr ptr AVDictionary): cint
proc avio_read_dir*(s: ptr AVIODirContext; next: ptr ptr AVIODirEntry): cint
proc avio_close_dir*(s: ptr ptr AVIODirContext): cint
proc avio_free_directory_entry*(entry: ptr ptr AVIODirEntry)
proc avio_alloc_context*(buffer: ptr cuchar; buffer_size: cint; write_flag: cint;
                        opaque: pointer; read_packet: proc (opaque: pointer;
    buf: ptr uint8; buf_size: cint): cint; write_packet: proc (opaque: pointer;
    buf: ptr uint8; buf_size: cint): cint; seek: proc (opaque: pointer; offset: int64;
    whence: cint): int64): ptr AVIOContext
proc avio_context_free*(s: ptr ptr AVIOContext)
proc avio_w8*(s: ptr AVIOContext; b: cint)
proc avio_write*(s: ptr AVIOContext; buf: ptr cuchar; size: cint)
proc avio_wl64*(s: ptr AVIOContext; val: uint64)
proc avio_wb64*(s: ptr AVIOContext; val: uint64)
proc avio_wl32*(s: ptr AVIOContext; val: cuint)
proc avio_wb32*(s: ptr AVIOContext; val: cuint)
proc avio_wl24*(s: ptr AVIOContext; val: cuint)
proc avio_wb24*(s: ptr AVIOContext; val: cuint)
proc avio_wl16*(s: ptr AVIOContext; val: cuint)
proc avio_wb16*(s: ptr AVIOContext; val: cuint)
proc avio_put_str*(s: ptr AVIOContext; str: cstring): cint
proc avio_put_str16le*(s: ptr AVIOContext; str: cstring): cint
proc avio_put_str16be*(s: ptr AVIOContext; str: cstring): cint
proc avio_write_marker*(s: ptr AVIOContext; time: int64; `type`: AVIODataMarkerType)
proc avio_seek*(s: ptr AVIOContext; offset: int64; whence: cint): int64
proc avio_skip*(s: ptr AVIOContext; offset: int64): int64
proc avio_tell*(s: ptr AVIOContext): int64 {.inline.} =
  return avio_seek(s, 0, 1)

proc avio_size*(s: ptr AVIOContext): int64
proc avio_feof*(s: ptr AVIOContext): cint
proc avio_printf*(s: ptr AVIOContext; fmt: cstring): cint {.varargs.}
proc avio_print_string_array*(s: ptr AVIOContext; strings: ptr cstring)
##  #define avio_print(s, ...) avio_print_string_array(s, (const char *[]){__VA_ARGS__, NULL})

proc avio_flush*(s: ptr AVIOContext)
proc avio_read*(s: ptr AVIOContext; buf: ptr cuchar; size: cint): cint
proc avio_read_partial*(s: ptr AVIOContext; buf: ptr cuchar; size: cint): cint
proc avio_r8*(s: ptr AVIOContext): cint
proc avio_rl16*(s: ptr AVIOContext): cuint
proc avio_rl24*(s: ptr AVIOContext): cuint
proc avio_rl32*(s: ptr AVIOContext): cuint
proc avio_rl64*(s: ptr AVIOContext): uint64
proc avio_rb16*(s: ptr AVIOContext): cuint
proc avio_rb24*(s: ptr AVIOContext): cuint
proc avio_rb32*(s: ptr AVIOContext): cuint
proc avio_rb64*(s: ptr AVIOContext): uint64
proc avio_get_str*(pb: ptr AVIOContext; maxlen: cint; buf: cstring; buflen: cint): cint
proc avio_get_str16le*(pb: ptr AVIOContext; maxlen: cint; buf: cstring; buflen: cint): cint
proc avio_get_str16be*(pb: ptr AVIOContext; maxlen: cint; buf: cstring; buflen: cint): cint
proc avio_open*(s: ptr ptr AVIOContext; url: cstring; flags: cint): cint
proc avio_open2*(s: ptr ptr AVIOContext; url: cstring; flags: cint;
                int_cb: ptr AVIOInterruptCB; options: ptr ptr AVDictionary): cint
proc avio_close*(s: ptr AVIOContext): cint
proc avio_closep*(s: ptr ptr AVIOContext): cint
proc avio_open_dyn_buf*(s: ptr ptr AVIOContext): cint
proc avio_get_dyn_buf*(s: ptr AVIOContext; pbuffer: ptr ptr uint8): cint
proc avio_close_dyn_buf*(s: ptr AVIOContext; pbuffer: ptr ptr uint8): cint
proc avio_enum_protocols*(opaque: ptr pointer; output: cint): cstring
proc avio_protocol_get_class*(name: cstring): ptr AVClass
proc avio_pause*(h: ptr AVIOContext; pause: cint): cint
proc avio_seek_time*(h: ptr AVIOContext; stream_index: cint; timestamp: int64;
                    flags: cint): int64


proc avio_read_to_bprint*(h: ptr AVIOContext; pb: ptr AVBPrint; max_size: csize_t): cint
proc avio_accept*(s: ptr AVIOContext; c: ptr ptr AVIOContext): cint
proc avio_handshake*(c: ptr AVIOContext): cint



proc av_get_packet*(s: ptr AVIOContext; pkt: ptr AVPacket; size: cint): cint
proc av_append_packet*(s: ptr AVIOContext; pkt: ptr AVPacket; size: cint): cint


proc av_stream_get_r_frame_rate*(s: ptr AVStream): AVRational
proc av_stream_set_r_frame_rate*(s: ptr AVStream; r: AVRational)
proc av_stream_get_recommended_encoder_configuration*(s: ptr AVStream): cstring
proc av_stream_set_recommended_encoder_configuration*(s: ptr AVStream;
    configuration: cstring)
proc av_stream_get_parser*(s: ptr AVStream): ptr AVCodecParserContext
proc av_stream_get_end_pts*(st: ptr AVStream): int64





proc av_format_get_probe_score*(s: ptr AVFormatContext): cint
proc av_format_get_video_codec*(s: ptr AVFormatContext): ptr AVCodec
proc av_format_set_video_codec*(s: ptr AVFormatContext; c: ptr AVCodec)
proc av_format_get_audio_codec*(s: ptr AVFormatContext): ptr AVCodec
proc av_format_set_audio_codec*(s: ptr AVFormatContext; c: ptr AVCodec)
proc av_format_get_subtitle_codec*(s: ptr AVFormatContext): ptr AVCodec
proc av_format_set_subtitle_codec*(s: ptr AVFormatContext; c: ptr AVCodec)
proc av_format_get_data_codec*(s: ptr AVFormatContext): ptr AVCodec
proc av_format_set_data_codec*(s: ptr AVFormatContext; c: ptr AVCodec)
proc av_format_get_metadata_header_padding*(s: ptr AVFormatContext): cint
proc av_format_set_metadata_header_padding*(s: ptr AVFormatContext; c: cint)
proc av_format_get_opaque*(s: ptr AVFormatContext): pointer
proc av_format_set_opaque*(s: ptr AVFormatContext; opaque: pointer)


proc av_format_get_control_message_cb*(s: ptr AVFormatContext): av_format_control_message
proc av_format_set_control_message_cb*(s: ptr AVFormatContext; callback: av_format_control_message)
proc av_format_get_open_cb*(s: ptr AVFormatContext): AVOpenCallback
proc av_format_set_open_cb*(s: ptr AVFormatContext; callback: AVOpenCallback)
proc av_format_inject_global_side_data*(s: ptr AVFormatContext)
proc av_fmt_ctx_get_duration_estimation_method*(ctx: ptr AVFormatContext): AVDurationEstimationMethod
proc avformat_version*(): cuint
proc avformat_configuration*(): cstring
proc avformat_license*(): cstring
proc av_register_all*()
proc av_register_input_format*(format: ptr AVInputFormat)
proc av_register_output_format*(format: ptr AVOutputFormat)
proc avformat_network_init*(): cint
proc avformat_network_deinit*(): cint
proc av_iformat_next*(f: ptr AVInputFormat): ptr AVInputFormat
proc av_oformat_next*(f: ptr AVOutputFormat): ptr AVOutputFormat
proc av_muxer_iterate*(opaque: ptr pointer): ptr AVOutputFormat
proc av_demuxer_iterate*(opaque: ptr pointer): ptr AVInputFormat
proc avformat_alloc_context*(): ptr AVFormatContext
proc avformat_free_context*(s: ptr AVFormatContext)
proc avformat_get_class*(): ptr AVClass
proc avformat_new_stream*(s: ptr AVFormatContext; c: ptr AVCodec): ptr AVStream
proc av_stream_add_side_data*(st: ptr AVStream; `type`: AVPacketSideDataType;
                             data: ptr uint8; size: csize_t): cint
proc av_stream_new_side_data*(stream: ptr AVStream; `type`: AVPacketSideDataType;
                             size: cint): ptr uint8
proc av_stream_get_side_data*(stream: ptr AVStream; `type`: AVPacketSideDataType;
                             size: ptr cint): ptr uint8
proc av_new_program*(s: ptr AVFormatContext; id: cint): ptr AVProgram
proc avformat_alloc_output_context2*(ctx: ptr ptr AVFormatContext;
                                    oformat: ptr AVOutputFormat;
                                    format_name: cstring; filename: cstring): cint
proc av_find_input_format*(short_name: cstring): ptr AVInputFormat
proc av_probe_input_format*(pd: ptr AVProbeData; is_opened: cint): ptr AVInputFormat
proc av_probe_input_format2*(pd: ptr AVProbeData; is_opened: cint; score_max: ptr cint): ptr AVInputFormat
proc av_probe_input_format3*(pd: ptr AVProbeData; is_opened: cint; score_ret: ptr cint): ptr AVInputFormat
proc av_probe_input_buffer2*(pb: ptr AVIOContext; fmt: ptr ptr AVInputFormat;
                            url: cstring; logctx: pointer; offset: cuint;
                            max_probe_size: cuint): cint
proc av_probe_input_buffer*(pb: ptr AVIOContext; fmt: ptr ptr AVInputFormat;
                           url: cstring; logctx: pointer; offset: cuint;
                           max_probe_size: cuint): cint
proc avformat_open_input*(ps: ptr ptr AVFormatContext; url: cstring;
                         fmt: ptr AVInputFormat; options: ptr ptr AVDictionary): cint
proc av_demuxer_open*(ic: ptr AVFormatContext): cint
proc avformat_find_stream_info*(ic: ptr AVFormatContext;
                               options: ptr ptr AVDictionary): cint
proc av_find_program_from_stream*(ic: ptr AVFormatContext; last: ptr AVProgram; s: cint): ptr AVProgram
proc av_program_add_stream_index*(ac: ptr AVFormatContext; progid: cint; idx: cuint)
proc av_find_best_stream*(ic: ptr AVFormatContext; `type`: AVMediaType;
                         wanted_stream_nb: cint; related_stream: cint;
                         decoder_ret: ptr ptr AVCodec; flags: cint): cint
proc av_read_frame*(s: ptr AVFormatContext; pkt: ptr AVPacket): cint
proc av_seek_frame*(s: ptr AVFormatContext; stream_index: cint; timestamp: int64;
                   flags: cint): cint
proc avformat_seek_file*(s: ptr AVFormatContext; stream_index: cint; min_ts: int64;
                        ts: int64; max_ts: int64; flags: cint): cint
proc avformat_flush*(s: ptr AVFormatContext): cint
proc av_read_play*(s: ptr AVFormatContext): cint {.discardable.}
proc av_read_pause*(s: ptr AVFormatContext): cint
proc avformat_close_input*(s: ptr ptr AVFormatContext)
proc avformat_write_header*(s: ptr AVFormatContext; options: ptr ptr AVDictionary): cint
proc avformat_init_output*(s: ptr AVFormatContext; options: ptr ptr AVDictionary): cint
proc av_write_frame*(s: ptr AVFormatContext; pkt: ptr AVPacket): cint
proc av_interleaved_write_frame*(s: ptr AVFormatContext; pkt: ptr AVPacket): cint
proc av_write_uncoded_frame*(s: ptr AVFormatContext; stream_index: cint;
                            frame: ptr AVFrame): cint
proc av_interleaved_write_uncoded_frame*(s: ptr AVFormatContext; stream_index: cint;
                                        frame: ptr AVFrame): cint
proc av_write_uncoded_frame_query*(s: ptr AVFormatContext; stream_index: cint): cint
proc av_write_trailer*(s: ptr AVFormatContext): cint
proc av_guess_format*(short_name: cstring; filename: cstring; mime_type: cstring): ptr AVOutputFormat
proc av_guess_codec*(fmt: ptr AVOutputFormat; short_name: cstring; filename: cstring;
                    mime_type: cstring; `type`: AVMediaType): AVCodecID
proc av_get_output_timestamp*(s: ptr AVFormatContext; stream: cint; dts: ptr int64;
                             wall: ptr int64): cint
proc av_hex_dump*(f: ptr FILE; buf: ptr uint8; size: cint)
proc av_hex_dump_log*(avcl: pointer; level: cint; buf: ptr uint8; size: cint)
proc av_pkt_dump2*(f: ptr FILE; pkt: ptr AVPacket; dump_payload: cint; st: ptr AVStream)
proc av_pkt_dump_log2*(avcl: pointer; level: cint; pkt: ptr AVPacket;
                      dump_payload: cint; st: ptr AVStream)
proc av_codec_get_id*(tags: ptr ptr AVCodecTag; tag: cuint): AVCodecID
proc av_codec_get_tag*(tags: ptr ptr AVCodecTag; id: AVCodecID): cuint
proc av_codec_get_tag2*(tags: ptr ptr AVCodecTag; id: AVCodecID; tag: ptr cuint): cint
proc av_find_default_stream_index*(s: ptr AVFormatContext): cint
proc av_index_search_timestamp*(st: ptr AVStream; timestamp: int64; flags: cint): cint
proc av_add_index_entry*(st: ptr AVStream; pos: int64; timestamp: int64; size: cint;
                        distance: cint; flags: cint): cint
proc av_url_split*(proto: cstring; proto_size: cint; authorization: cstring;
                  authorization_size: cint; hostname: cstring; hostname_size: cint;
                  port_ptr: ptr cint; path: cstring; path_size: cint; url: cstring)
proc av_dump_format*(ic: ptr AVFormatContext; index: cint; url: cstring; is_output: cint)
proc av_get_frame_filename2*(buf: cstring; buf_size: cint; path: cstring; number: cint;
                            flags: cint): cint
proc av_get_frame_filename*(buf: cstring; buf_size: cint; path: cstring; number: cint): cint
proc av_filename_number_test*(filename: cstring): cint
proc av_sdp_create*(ac: ptr ptr AVFormatContext; n_files: cint; buf: cstring; size: cint): cint
proc av_match_ext*(filename: cstring; extensions: cstring): cint
proc avformat_query_codec*(ofmt: ptr AVOutputFormat; codec_id: AVCodecID;
                          std_compliance: cint): cint
proc avformat_get_riff_video_tags*(): ptr AVCodecTag
proc avformat_get_riff_audio_tags*(): ptr AVCodecTag
proc avformat_get_mov_video_tags*(): ptr AVCodecTag
proc avformat_get_mov_audio_tags*(): ptr AVCodecTag
proc av_guess_sample_aspect_ratio*(format: ptr AVFormatContext;
                                  stream: ptr AVStream; frame: ptr AVFrame): AVRational
proc av_guess_frame_rate*(ctx: ptr AVFormatContext; stream: ptr AVStream;
                         frame: ptr AVFrame): AVRational
proc avformat_match_stream_specifier*(s: ptr AVFormatContext; st: ptr AVStream;
                                     spec: cstring): cint
proc avformat_queue_attached_pictures*(s: ptr AVFormatContext): cint
proc av_apply_bitstream_filters*(codec: ptr AVCodecContext; pkt: ptr AVPacket;
                                bsfc: ptr AVBitStreamFilterContext): cint

proc avformat_transfer_internal_stream_timing_info*(ofmt: ptr AVOutputFormat;ost: ptr AVStream; ist: ptr AVStream; copy_tb: AVTimebaseSource): cint
proc av_stream_get_codec_timebase*(st: ptr AVStream): AVRational

proc av_opt_show2*(obj: pointer; av_log_obj: pointer; req_flags: cint; rej_flags: cint): cint
proc av_opt_set_defaults*(s: pointer)
proc av_opt_set_defaults2*(s: pointer; mask: cint; flags: cint)
proc av_set_options_string*(ctx: pointer; opts: cstring; key_val_sep: cstring;
                           pairs_sep: cstring): cint
proc av_opt_set_from_string*(ctx: pointer; opts: cstring; shorthand: cstringArray;
                            key_val_sep: cstring; pairs_sep: cstring): cint
proc av_opt_free*(obj: pointer)
proc av_opt_flag_is_set*(obj: pointer; field_name: cstring; flag_name: cstring): cint
proc av_opt_set_dict*(obj: pointer; options: ptr ptr AVDictionary): cint
proc av_opt_set_dict2*(obj: pointer; options: ptr ptr AVDictionary; search_flags: cint): cint
proc av_opt_get_key_value*(ropts: cstringArray; key_val_sep: cstring; pairs_sep: cstring; flags: cuint; rkey: cstringArray; rval: cstringArray): cint


proc av_opt_eval_flags*(obj: pointer; o: ptr AVOption; val: cstring; flags_out: ptr cint): cint
proc av_opt_eval_int*(obj: pointer; o: ptr AVOption; val: cstring; int_out: ptr cint): cint
proc av_opt_eval_int64*(obj: pointer; o: ptr AVOption; val: cstring; int64_out: ptr int64): cint
proc av_opt_eval_float*(obj: pointer; o: ptr AVOption; val: cstring;float_out: ptr cfloat): cint
proc av_opt_eval_double*(obj: pointer; o: ptr AVOption; val: cstring;double_out: ptr cdouble): cint
proc av_opt_eval_q*(obj: pointer; o: ptr AVOption; val: cstring; q_out: ptr AVRational): cint
proc av_opt_find*(obj: pointer; name: cstring; unit: cstring; opt_flags: cint; search_flags: cint): ptr AVOption
proc av_opt_find2*(obj: pointer; name: cstring; unit: cstring; opt_flags: cint; search_flags: cint; target_obj: ptr pointer): ptr AVOption
proc av_opt_next*(obj: pointer; prev: ptr AVOption): ptr AVOption
proc av_opt_child_next*(obj: pointer; prev: pointer): pointer
proc av_opt_child_class_next*(parent: ptr AVClass; prev: ptr AVClass): ptr AVClass
proc av_opt_child_class_iterate*(parent: ptr AVClass; iter: ptr pointer): ptr AVClass
proc av_opt_set*(obj: pointer; name: cstring; val: cstring; search_flags: cint): cint
proc av_opt_set_int*(obj: pointer; name: cstring; val: int64; search_flags: cint): cint
proc av_opt_set_double*(obj: pointer; name: cstring; val: cdouble; search_flags: cint): cint
proc av_opt_set_q*(obj: pointer; name: cstring; val: AVRational; search_flags: cint): cint
proc av_opt_set_bin*(obj: pointer; name: cstring; val: ptr uint8; size: cint; search_flags: cint): cint
proc av_opt_set_image_size*(obj: pointer; name: cstring; w: cint; h: cint; search_flags: cint): cint
proc av_opt_set_pixel_fmt*(obj: pointer; name: cstring; fmt: AVPixelFormat; search_flags: cint): cint
proc av_opt_set_sample_fmt*(obj: pointer; name: cstring; fmt: AVSampleFormat; search_flags: cint): cint
proc av_opt_set_video_rate*(obj: pointer; name: cstring; val: AVRational; search_flags: cint): cint
proc av_opt_set_channel_layout*(obj: pointer; name: cstring; ch_layout: int64; search_flags: cint): cint
proc av_opt_set_dict_val*(obj: pointer; name: cstring; val: ptr AVDictionary; search_flags: cint): cint
proc av_opt_get*(obj: pointer; name: cstring; search_flags: cint; out_val: ptr ptr uint8): cint
proc av_opt_get_int*(obj: pointer; name: cstring; search_flags: cint; out_val: ptr int64): cint
proc av_opt_get_double*(obj: pointer; name: cstring; search_flags: cint; out_val: ptr cdouble): cint
proc av_opt_get_q*(obj: pointer; name: cstring; search_flags: cint; out_val: ptr AVRational): cint
proc av_opt_get_image_size*(obj: pointer; name: cstring; search_flags: cint; w_out: ptr cint; h_out: ptr cint): cint
proc av_opt_get_pixel_fmt*(obj: pointer; name: cstring; search_flags: cint; out_fmt: ptr AVPixelFormat): cint
proc av_opt_get_sample_fmt*(obj: pointer; name: cstring; search_flags: cint; out_fmt: ptr AVSampleFormat): cint
proc av_opt_get_video_rate*(obj: pointer; name: cstring; search_flags: cint; out_val: ptr AVRational): cint
proc av_opt_get_channel_layout*(obj: pointer; name: cstring; search_flags: cint; ch_layout: ptr int64): cint
proc av_opt_get_dict_val*(obj: pointer; name: cstring; search_flags: cint; out_val: ptr ptr AVDictionary): cint
proc av_opt_ptr*(avclass: ptr AVClass; obj: pointer; name: cstring): pointer
proc av_opt_freep_ranges*(ranges: ptr ptr AVOptionRanges)
proc av_opt_query_ranges*(a1: ptr ptr AVOptionRanges; obj: pointer; key: cstring; flags: cint): cint
proc av_opt_copy*(dest: pointer; src: pointer): cint
proc av_opt_query_ranges_default*(a1: ptr ptr AVOptionRanges; obj: pointer;key: cstring; flags: cint): cint
proc av_opt_is_set_to_default*(obj: pointer; o: ptr AVOption): cint
proc av_opt_is_set_to_default_by_name*(obj: pointer; name: cstring; search_flags: cint): cint
proc av_opt_serialize*(obj: pointer; opt_flags: cint; flags: cint;buffer: cstringArray; key_val_sep: char; pairs_sep: char): cint
proc avdevice_version*(): cuint
proc avdevice_configuration*(): cstring
proc avdevice_license*(): cstring
proc avdevice_register_all*()
proc av_input_audio_device_next*(d: ptr AVInputFormat): ptr AVInputFormat
proc av_input_video_device_next*(d: ptr AVInputFormat): ptr AVInputFormat
proc av_output_audio_device_next*(d: ptr AVOutputFormat): ptr AVOutputFormat
proc av_output_video_device_next*(d: ptr AVOutputFormat): ptr AVOutputFormat

proc avdevice_app_to_dev_control_message*(s: ptr AVFormatContext;`type`: AVAppToDevMessageType; data: pointer; data_size: csize_t): cint
proc avdevice_dev_to_app_control_message*(s: ptr AVFormatContext; `type`: AVDevToAppMessageType; data: pointer; data_size: csize_t): cint


proc avdevice_capabilities_create*(caps: ptr ptr AVDeviceCapabilitiesQuery;s: ptr AVFormatContext; device_options: ptr ptr AVDictionary): cint
proc avdevice_capabilities_free*(caps: ptr ptr AVDeviceCapabilitiesQuery;s: ptr AVFormatContext)

proc avdevice_list_devices*(s: ptr AVFormatContext;
                           device_list: ptr ptr AVDeviceInfoList): cint
proc avdevice_free_list_devices*(device_list: ptr ptr AVDeviceInfoList)
proc avdevice_list_input_sources*(device: ptr AVInputFormat; device_name: cstring;
                                 device_options: ptr AVDictionary;
                                 device_list: ptr ptr AVDeviceInfoList): cint
proc avdevice_list_output_sinks*(device: ptr AVOutputFormat; device_name: cstring;
                                device_options: ptr AVDictionary;
                                device_list: ptr ptr AVDeviceInfoList): cint
proc swscale_version*(): cuint
proc swscale_configuration*(): cstring
proc swscale_license*(): cstring
proc sws_getCoefficients*(colorspace: cint): ptr cint

proc sws_isSupportedInput*(pix_fmt: AVPixelFormat): cint
proc sws_isSupportedOutput*(pix_fmt: AVPixelFormat): cint
proc sws_isSupportedEndiannessConversion*(pix_fmt: AVPixelFormat): cint
proc sws_alloc_context*(): ptr SwsContext
proc sws_init_context*(sws_context: ptr SwsContext; srcFilter: ptr SwsFilter;
                      dstFilter: ptr SwsFilter): cint
proc sws_freeContext*(swsContext: ptr SwsContext)
proc sws_getContext*(srcW: cint; srcH: cint; srcFormat: AVPixelFormat; dstW: cint;
                    dstH: cint; dstFormat: AVPixelFormat; flags: cint;
                    srcFilter: ptr SwsFilter; dstFilter: ptr SwsFilter;
                    param: ptr cdouble): ptr SwsContext
proc sws_scale*(c: ptr SwsContext; srcSlice: ptr ptr uint8; srcStride: ptr cint;
               srcSliceY: cint; srcSliceH: cint; dst: ptr ptr uint8;
               dstStride: ptr cint): cint {.discardable.}
proc sws_setColorspaceDetails*(c: ptr SwsContext; inv_table: array[4, cint];
                              srcRange: cint; table: array[4, cint]; dstRange: cint;
                              brightness: cint; contrast: cint; saturation: cint): cint
proc sws_getColorspaceDetails*(c: ptr SwsContext; inv_table: ptr ptr cint;
                              srcRange: ptr cint; table: ptr ptr cint;
                              dstRange: ptr cint; brightness: ptr cint;
                              contrast: ptr cint; saturation: ptr cint): cint
proc sws_allocVec*(length: cint): ptr SwsVector
proc sws_getGaussianVec*(variance: cdouble; quality: cdouble): ptr SwsVector
proc sws_scaleVec*(a: ptr SwsVector; scalar: cdouble)
proc sws_normalizeVec*(a: ptr SwsVector; height: cdouble)
proc sws_getConstVec*(c: cdouble; length: cint): ptr SwsVector
proc sws_getIdentityVec*(): ptr SwsVector
proc sws_convVec*(a: ptr SwsVector; b: ptr SwsVector)
proc sws_addVec*(a: ptr SwsVector; b: ptr SwsVector)
proc sws_subVec*(a: ptr SwsVector; b: ptr SwsVector)
proc sws_shiftVec*(a: ptr SwsVector; shift: cint)
proc sws_cloneVec*(a: ptr SwsVector): ptr SwsVector
proc sws_printVec2*(a: ptr SwsVector; log_ctx: ptr AVClass; log_level: cint)
proc sws_freeVec*(a: ptr SwsVector)
proc sws_getDefaultFilter*(lumaGBlur: cfloat; chromaGBlur: cfloat;
                          lumaSharpen: cfloat; chromaSharpen: cfloat;
                          chromaHShift: cfloat; chromaVShift: cfloat; verbose: cint): ptr SwsFilter
proc sws_freeFilter*(filter: ptr SwsFilter)
proc sws_getCachedContext*(context: ptr SwsContext; srcW: auto; srcH: auto;
                          srcFormat: auto; dstW: auto; dstH: auto;
                          dstFormat: auto; flags: auto;
                          srcFilter: ptr SwsFilter; dstFilter: ptr SwsFilter;
                          param: ptr cdouble): ptr SwsContext
proc sws_convertPalette8ToPacked32*(src: ptr uint8; dst: ptr uint8;num_pixels: cint; palette: ptr uint8)
proc sws_convertPalette8ToPacked24*(src: ptr uint8; dst: ptr uint8; num_pixels: cint; palette: ptr uint8)
proc sws_get_class*(): ptr AVClass
type
  FFTSample* = cfloat
  FFTComplex* {.bycopy.} = object
    re*: FFTSample
    im*: FFTSample

type FFTContext = object
type RDFTContext = object
type DCTContext = object
type SwrContext = object




type
  RDFTransformType* {.size: sizeof(cint).} = enum
    DFT_R2C, IDFT_C2R, IDFT_R2C, DFT_C2R
  DCTTransformType* {.size: sizeof(cint).} = enum
    DCT_II = 0, DCT_III, DCT_I, DST_I
  SwrDitherType* {.size: sizeof(cint).} = enum
    SWR_DITHER_NONE = 0, SWR_DITHER_RECTANGULAR, SWR_DITHER_TRIANGULAR,
    SWR_DITHER_TRIANGULAR_HIGHPASS, SWR_DITHER_NS = 64, ## /< not part of API/ABI
    SWR_DITHER_NS_LIPSHITZ, SWR_DITHER_NS_F_WEIGHTED,
    SWR_DITHER_NS_MODIFIED_E_WEIGHTED, SWR_DITHER_NS_IMPROVED_E_WEIGHTED,
    SWR_DITHER_NS_SHIBATA, SWR_DITHER_NS_LOW_SHIBATA, SWR_DITHER_NS_HIGH_SHIBATA, SWR_DITHER_NB ## /< not part of API/ABI
  SwrEngine* {.size: sizeof(cint).} = enum
    SWR_ENGINE_SWR, SWR_ENGINE_SOXR, SWR_ENGINE_NB ## /< not part of API/ABI
  SwrFilterType* {.size: sizeof(cint).} = enum
    SWR_FILTER_TYPE_CUBIC, SWR_FILTER_TYPE_BLACKMAN_NUTTALL,
    SWR_FILTER_TYPE_KAISER

proc av_fft_init*(nbits: cint; inverse: cint): ptr FFTContext
proc av_fft_permute*(s: ptr FFTContext; z: ptr FFTComplex)
proc av_fft_calc*(s: ptr FFTContext; z: ptr FFTComplex)
proc av_fft_end*(s: ptr FFTContext)
proc av_mdct_init*(nbits: cint; inverse: cint; scale: cdouble): ptr FFTContext
proc av_imdct_calc*(s: ptr FFTContext; output: ptr FFTSample; input: ptr FFTSample)
proc av_imdct_half*(s: ptr FFTContext; output: ptr FFTSample; input: ptr FFTSample)
proc av_mdct_calc*(s: ptr FFTContext; output: ptr FFTSample; input: ptr FFTSample)
proc av_mdct_end*(s: ptr FFTContext)
proc av_rdft_init*(nbits: cint; trans: RDFTransformType): ptr RDFTContext
proc av_rdft_calc*(s: ptr RDFTContext; data: ptr FFTSample)
proc av_rdft_end*(s: ptr RDFTContext)

proc av_dct_init*(nbits: cint; `type`: DCTTransformType): ptr DCTContext
proc av_dct_calc*(s: ptr DCTContext; data: ptr FFTSample)
proc av_dct_end*(s: ptr DCTContext)

proc swr_get_class*(): ptr AVClass
proc swr_alloc*(): ptr SwrContext
proc swr_init*(s: ptr SwrContext): cint
proc swr_is_initialized*(s: ptr SwrContext): cint
proc swr_alloc_set_opts*(s: ptr SwrContext; out_ch_layout: int64;
                        out_sample_fmt: AVSampleFormat; out_sample_rate: cint;
                        in_ch_layout: int64; in_sample_fmt: AVSampleFormat;
                        in_sample_rate: cint; log_offset: cint; log_ctx: pointer): ptr SwrContext
proc swr_free*(s: ptr ptr SwrContext)
proc swr_close*(s: ptr SwrContext)
proc swr_convert*(s: ptr SwrContext; `out`: ptr ptr uint8; out_count: cint;
                 `in`: ptr ptr uint8; in_count: cint): cint
proc swr_next_pts*(s: ptr SwrContext; pts: int64): int64
proc swr_set_compensation*(s: ptr SwrContext; sample_delta: cint;
                          compensation_distance: cint): cint
proc swr_set_channel_mapping*(s: ptr SwrContext; channel_map: ptr cint): cint
proc swr_build_matrix*(in_layout: uint64; out_layout: uint64;
                      center_mix_level: cdouble; surround_mix_level: cdouble;
                      lfe_mix_level: cdouble; rematrix_maxval: cdouble;
                      rematrix_volume: cdouble; matrix: ptr cdouble; stride: cint;
                      matrix_encoding: AVMatrixEncoding; log_ctx: pointer): cint
proc swr_set_matrix*(s: ptr SwrContext; matrix: ptr cdouble; stride: cint): cint
proc swr_drop_output*(s: ptr SwrContext; count: cint): cint
proc swr_inject_silence*(s: ptr SwrContext; count: cint): cint
proc swr_get_delay*(s: ptr SwrContext; base: int64): int64
proc swr_get_out_samples*(s: ptr SwrContext; in_samples: cint): cint
proc swresample_version*(): cuint
proc swresample_configuration*(): cstring
proc swresample_license*(): cstring
proc swr_convert_frame*(swr: ptr SwrContext; output: ptr AVFrame; input: ptr AVFrame): cint
proc swr_config_frame*(swr: ptr SwrContext; `out`: ptr AVFrame; `in`: ptr AVFrame): cint
proc avfilter_version*(): cuint
proc avfilter_configuration*(): cstring
proc avfilter_license*(): cstring

proc avfilter_pad_count*(pads: ptr AVFilterPad): cint
proc avfilter_pad_get_name*(pads: ptr AVFilterPad; pad_idx: cint): cstring
proc avfilter_pad_get_type*(pads: ptr AVFilterPad; pad_idx: cint): AVMediaType


proc avfilter_link*(src: ptr AVFilterContext; srcpad: cuint; dst: ptr AVFilterContext;dstpad: cuint): cint {.discardable.}
proc avfilter_link_free*(link: ptr ptr AVFilterLink)
proc avfilter_link_get_channels*(link: ptr AVFilterLink): cint
proc avfilter_link_set_closed*(link: ptr AVFilterLink; closed: cint)
proc avfilter_config_links*(filter: ptr AVFilterContext): cint
proc avfilter_process_command*(filter: ptr AVFilterContext; cmd: cstring;
                              arg: cstring; res: cstring; res_len: cint; flags: cint): cint
proc av_filter_iterate*(opaque: ptr pointer): ptr AVFilter
proc avfilter_register_all*()
proc avfilter_register*(filter: ptr AVFilter): cint
proc avfilter_next*(prev: ptr AVFilter): ptr AVFilter
proc avfilter_get_by_name*(name: cstring): ptr AVFilter
proc avfilter_init_str*(ctx: ptr AVFilterContext; args: cstring): cint
proc avfilter_init_dict*(ctx: ptr AVFilterContext; options: ptr ptr AVDictionary): cint
proc avfilter_free*(filter: ptr AVFilterContext)
proc avfilter_insert_filter*(link: ptr AVFilterLink; filt: ptr AVFilterContext;
                            filt_srcpad_idx: cuint; filt_dstpad_idx: cuint): cint
proc avfilter_get_class*(): ptr AVClass



proc avfilter_graph_alloc*(): ptr AVFilterGraph
proc avfilter_graph_alloc_filter*(graph: ptr AVFilterGraph; filter: ptr AVFilter;
                                 name: cstring): ptr AVFilterContext
proc avfilter_graph_get_filter*(graph: ptr AVFilterGraph; name: cstring): ptr AVFilterContext
proc avfilter_graph_create_filter*(filt_ctx: ptr ptr AVFilterContext; filt: ptr AVFilter; name: cstring; args: cstring; opaque: pointer; graph_ctx: ptr AVFilterGraph): cint {.discardable.}
proc avfilter_graph_set_auto_convert*(graph: ptr AVFilterGraph; flags: cuint)
const
  AVFILTER_AUTO_CONVERT_ALL* = 0
  AVFILTER_AUTO_CONVERT_NONE* = -1

proc avfilter_graph_config*(graphctx: ptr AVFilterGraph; log_ctx: pointer): cint
proc avfilter_graph_free*(graph: ptr ptr AVFilterGraph)
type
  AVFilterInOut* {.bycopy.} = object
    name*: cstring
    filter_ctx*: ptr AVFilterContext
    pad_idx*: cint
    next*: ptr AVFilterInOut


proc avfilter_inout_alloc*(): ptr AVFilterInOut
proc avfilter_inout_free*(inout: ptr ptr AVFilterInOut)
proc avfilter_graph_parse*(graph: ptr AVFilterGraph; filters: cstring;
                          inputs: ptr AVFilterInOut; outputs: ptr AVFilterInOut;
                          log_ctx: pointer): cint
proc avfilter_graph_parse_ptr*(graph: ptr AVFilterGraph; filters: cstring;
                              inputs: ptr ptr AVFilterInOut;
                              outputs: ptr ptr AVFilterInOut; log_ctx: pointer): cint
proc avfilter_graph_parse2*(graph: ptr AVFilterGraph; filters: cstring;
                           inputs: ptr ptr AVFilterInOut;
                           outputs: ptr ptr AVFilterInOut): cint
proc avfilter_graph_send_command*(graph: ptr AVFilterGraph; target: cstring;
                                 cmd: cstring; arg: cstring; res: cstring;
                                 res_len: cint; flags: cint): cint
proc avfilter_graph_queue_command*(graph: ptr AVFilterGraph; target: cstring;
                                  cmd: cstring; arg: cstring; flags: cint; ts: cdouble): cint
proc avfilter_graph_dump*(graph: ptr AVFilterGraph; options: cstring): cstring
proc avfilter_graph_request_oldest*(graph: ptr AVFilterGraph): cint
proc av_buffersink_get_frame_flags*(ctx: ptr AVFilterContext; frame: ptr AVFrame;
                                   flags: cint): cint
type
  AVBufferSinkParams* {.bycopy.} = object
    pixel_fmts*: ptr AVPixelFormat ## /< list of allowed pixel formats, terminated by AV_PIX_FMT_NONE


proc av_buffersink_params_alloc*(): ptr AVBufferSinkParams
type
  AVABufferSinkParams* {.bycopy.} = object
    sample_fmts*: ptr AVSampleFormat ## /< list of allowed sample formats, terminated by AV_SAMPLE_FMT_NONE
    channel_layouts*: ptr int64 ## /< list of allowed channel layouts, terminated by -1
    channel_counts*: ptr cint   ## /< list of allowed channel counts, terminated by -1
    all_channel_counts*: cint  ## /< if not 0, accept any channel count or layout
    sample_rates*: ptr cint     ## /< list of allowed sample rates, terminated by -1


proc av_abuffersink_params_alloc*(): ptr AVABufferSinkParams
proc av_buffersink_set_frame_size*(ctx: ptr AVFilterContext; frame_size: cuint)
proc av_buffersink_get_type*(ctx: ptr AVFilterContext): AVMediaType
proc av_buffersink_get_time_base*(ctx: ptr AVFilterContext): AVRational
proc av_buffersink_get_format*(ctx: ptr AVFilterContext): cint
proc av_buffersink_get_frame_rate*(ctx: ptr AVFilterContext): AVRational
proc av_buffersink_get_w*(ctx: ptr AVFilterContext): cint
proc av_buffersink_get_h*(ctx: ptr AVFilterContext): cint
proc av_buffersink_get_sample_aspect_ratio*(ctx: ptr AVFilterContext): AVRational
proc av_buffersink_get_channels*(ctx: ptr AVFilterContext): cint
proc av_buffersink_get_channel_layout*(ctx: ptr AVFilterContext): uint64
proc av_buffersink_get_sample_rate*(ctx: ptr AVFilterContext): cint
proc av_buffersink_get_hw_frames_ctx*(ctx: ptr AVFilterContext): ptr AVBufferRef
proc av_buffersink_get_frame*(ctx: ptr AVFilterContext; frame: ptr AVFrame): cint
proc av_buffersink_get_samples*(ctx: ptr AVFilterContext; frame: ptr AVFrame;
                               nb_samples: cint): cint
const
  AV_BUFFERSRC_FLAG_NO_CHECK_FORMAT* = 1
  AV_BUFFERSRC_FLAG_PUSH* = 4
  AV_BUFFERSRC_FLAG_KEEP_REF* = 8

proc av_buffersrc_get_nb_failed_requests*(buffer_src: ptr AVFilterContext): cuint
type
  AVBufferSrcParameters* {.bycopy.} = object
    format*: cint
    time_base*: AVRational
    width*: cint
    height*: cint
    sample_aspect_ratio*: AVRational
    frame_rate*: AVRational
    hw_frames_ctx*: ptr AVBufferRef
    sample_rate*: cint
    channel_layout*: uint64


proc av_buffersrc_parameters_alloc*(): ptr AVBufferSrcParameters
proc av_buffersrc_parameters_set*(ctx: ptr AVFilterContext;
                                 param: ptr AVBufferSrcParameters): cint
proc av_buffersrc_write_frame*(ctx: ptr AVFilterContext; frame: ptr AVFrame): cint
proc av_buffersrc_add_frame*(ctx: ptr AVFilterContext; frame: ptr AVFrame): cint
proc av_buffersrc_add_frame_flags*(buffer_src: ptr AVFilterContext;
                                  frame: ptr AVFrame; flags: cint): cint
proc av_buffersrc_close*(ctx: ptr AVFilterContext; pts: int64; flags: cuint): cint

type
  SDL_bool* {.size: sizeof(cint).} = enum
    SDL_FALSE = 0, SDL_TRUE = 1
  SDL_compile_time_assert_uint8* = array[(sizeof(uint8) == 1).int * 2 - 1, cint]
  SDL_compile_time_assert_sint8* = array[(sizeof((int8)) == 1).int * 2 - 1, cint]
  SDL_compile_time_assert_uint16* = array[(sizeof((uint16)) == 2).int * 2 - 1, cint]
  SDL_compile_time_assert_sint16* = array[(sizeof((int16)) == 2).int * 2 - 1, cint]
  SDL_compile_time_assert_uint32* = array[(sizeof((uint32)) == 4).int * 2 - 1, cint]
  SDL_compile_time_assert_sint32* = array[(sizeof((int32)) == 4).int * 2 - 1, cint]
  SDL_compile_time_assert_uint64* = array[(sizeof((uint64)) == 8).int * 2 - 1, cint]
  SDL_compile_time_assert_sint64* = array[(sizeof((int64)) == 8).int * 2 - 1, cint]
  SDL_DUMMY_ENUM* {.size: sizeof(cint).} = enum
    DUMMY_ENUM_VALUE
  # SDL_compile_time_assert_enum* = array[(sizeof(SDL_DUMMY_ENUM) == sizeof(int)).int * 2 - 1, cint]


##  static inline void SDL_memset4(void *dst, uint32 val, csize_t dwords)
##  {
##      csize_t _n = (dwords + 3) / 4;
##      uint32 *_p = ((uint32 *)(dst));
##      uint32 _val = (val);
##      if (dwords == 0)
##          return;
##      switch (dwords % 4)
##      {
##      case 0:
##          do
##          {
##              *_p++ = _val;
##          case 3:
##              *_p++ = _val;
##          case 2:
##              *_p++ = _val;
##          case 1:
##              *_p++ = _val;
##          } while (--_n);
##      }
##  }

type
  SDL_malloc_func* = proc (size: csize_t): pointer
  SDL_calloc_func* = proc (nmemb: csize_t; size: csize_t): pointer
  SDL_realloc_func* = proc (mem: pointer; size: csize_t): pointer
  SDL_free_func* = proc (mem: pointer)



# type
#   SDL_iconv_t* = ptr _SDL_iconv_t

# proc SDL_iconv_open*(tocode: cstring; fromcode: cstring): SDL_iconv_t
# proc SDL_iconv_close*(cd: SDL_iconv_t): cint
# proc SDL_iconv*(cd: SDL_iconv_t; inbuf: cstringArray; inbytesleft: ptr csize_t;
#                outbuf: cstringArray; outbytesleft: ptr csize_t): csize_t

type
  SDL_main_func* = proc (argc: cint; argv: ptr cstring): cint



##  #define SDL_enabled_assert(condition)                                                                                         \
##      do                                                                                                                        \
##      {                                                                                                                         \
##          while (!(condition))                                                                                                  \
##          {                                                                                                                     \
##              static struct SDL_AssertData sdl_assert_data = {0, 0, #condition, 0, 0, 0, 0};                                    \
##              const SDL_AssertState sdl_assert_state = SDL_ReportAssertion(&sdl_assert_data, SDL_FUNCTION, SDL_FILE, SDL_LINE); \
##              if (sdl_assert_state == SDL_ASSERTION_RETRY)                                                                      \
##              {                                                                                                                 \
##                  continue;                                                                                                     \
##              }                                                                                                                 \
##              else if (sdl_assert_state == SDL_ASSERTION_BREAK)                                                                 \
##              {                                                                                                                 \
##                  SDL_TriggerBreakpoint();                                                                                      \
##              }                                                                                                                 \
##              break;                                                                                                            \
##          }                                                                                                                     \
##      } while (SDL_NULL_WHILE_LOOP_CONDITION)

type
  SDL_AssertionHandler* = proc (data: ptr SDL_AssertData; userdata: pointer): SDL_AssertState
type
  SDL_SpinLock* = cint
type
  SDL_atomic_t* {.bycopy.} = object
    value*: cint
type
  SDL_errorcode* {.size: sizeof(cint).} = enum
    SDL_ENOMEM, SDL_EFREAD, SDL_EFWRITE, SDL_EFSEEK, SDL_UNSUPPORTED_ERROR, SDL_LASTERROR



##  #define SDL_CompilerBarrier() __asm__ __volatile__("" \
##                                                     :  \
##                                                     :  \
##                                                     : "memory")


##  static inline uint16
##  SDL_Swap16(uint16 x)
##  {
##      __asm__("xchgb %b0,%h0"
##              : "=Q"(x)
##              : "0"(x));
##      return x;
##  }
##  static inline uint32
##  SDL_Swap32(uint32 x)
##  {
##      __asm__("bswapl %0"
##              : "=r"(x)
##              : "0"(x));
##      return x;
##  }
##  static inline uint64
##  SDL_Swap64(uint64 x)
##  {
##      __asm__("bswapq %0"
##              : "=r"(x)
##              : "0"(x));
##      return x;
##  }
##  static inline float
##  SDL_SwapFloat(float x)
##  {
##      union
##      {
##          float f;
##          uint32 ui32;
##      } swapper;
##      swapper.f = x;
##      swapper.ui32 = SDL_Swap32(swapper.ui32);
##      return swapper.f;
##  }
type 
  SDL_semaphore* {.bycopy.} = object

  SDL_sem* = SDL_semaphore


type
  SDL_cond* {.bycopy.} = object

type
  SDL_mutex* {.bycopy.} = object


type
  SDL_RWopsHiddenUnionWindowioBuffer* {.bycopy.} = object
    data*: pointer
    size*: csize_t
    left*: csize_t

  SDL_RWopsHiddenUnionWindowio* {.bycopy.} = object
    append*: SDL_bool
    h*: pointer
    buffer*: SDL_RWopsHiddenUnionWindowioBuffer

  SDL_RWopsHiddenUnionMem* {.bycopy.} = object
    base*: ptr uint8
    here*: ptr uint8
    stop*: ptr uint8

  SDL_RWopsHiddenUnionUnknown* {.bycopy.} = object
    data1*: pointer
    data2*: pointer

  SDL_RWopsHiddenUnion* {.bycopy.} = object {.union.}
    windowsio*: SDL_RWopsHiddenUnionWindowio
    mem*: SDL_RWopsHiddenUnionMem
    unknown*: SDL_RWopsHiddenUnionUnknown

  SDL_RWops* {.bycopy.} = object
    size*: proc (context: ptr SDL_RWops): int64
    seek*: proc (context: ptr SDL_RWops; offset: int64; whence: cint): int64
    read*: proc (context: ptr SDL_RWops; `ptr`: pointer; size: csize_t; maxnum: csize_t): csize_t
    write*: proc (context: ptr SDL_RWops; `ptr`: pointer; size: csize_t; num: csize_t): csize_t
    close*: proc (context: ptr SDL_RWops): cint
    `type`*: uint32
    hidden*: SDL_RWopsHiddenUnion

type SDL_AudioCallback* = proc (userdata: pointer; stream: ptr uint8; len: cint) {.cdecl.}


type
  SDL_AudioFormat* = uint16
  SDL_AudioSpec* {.bycopy.} = object
    freq*: cint
    format*: SDL_AudioFormat
    channels*: uint8
    silence*: uint8
    samples*: uint16
    padding*: uint16
    size*: uint32
    callback*: SDL_AudioCallback
    userdata*: pointer

  SDL_AudioCVT* {.bycopy.} = object
    needed*: cint
    src_format*: SDL_AudioFormat
    dst_format*: SDL_AudioFormat
    rate_incr*: cdouble
    buf*: ptr uint8
    len*: cint
    len_cvt*: cint
    len_mult*: cint
    len_ratio*: cdouble
    filters*: array[9 + 1, SDL_AudioFilter]
    filter_index*: cint

  SDL_AudioFilter* = proc (cvt: ptr SDL_AudioCVT; format: SDL_AudioFormat)
  SDL_AudioDeviceID* = uint32

  SDL_AudioStatus* {.size: sizeof(cint).} = enum
    SDL_AUDIO_STOPPED = 0, SDL_AUDIO_PLAYING, SDL_AUDIO_PAUSED
  SDL_AudioStream* {.bycopy.} = object

  SDL_Point*  = tuple
    x: cint
    y: cint

  SDL_FPoint*  = tuple
    x: cfloat
    y: cfloat

  SDL_Rect*  = tuple
    x: cint
    y: cint
    w: cint
    h: cint

  SDL_FRect* = tuple
    x: cfloat
    y: cfloat
    w: cfloat
    h: cfloat

  SDL_BlitMap* {.bycopy.} = object

  SDL_Surface* {.bycopy.} = object
    flags*: uint32
    format*: ptr SDL_PixelFormat
    w*: cint
    h*: cint
    pitch*: cint
    pixels*: pointer
    userdata*: pointer
    locked*: cint
    lock_data*: pointer
    clip_rect*: SDL_Rect
    map*: ptr SDL_BlitMap
    refcount*: cint

  SDL_blit* = proc (src: ptr SDL_Surface; srcrect: ptr SDL_Rect; dst: ptr SDL_Surface; dstrect: ptr SDL_Rect): cint

  SDL_YUV_CONVERSION_MODE* {.size: sizeof(cint).} = enum
    SDL_YUV_CONVERSION_JPEG, SDL_YUV_CONVERSION_BT601, SDL_YUV_CONVERSION_BT709,
    SDL_YUV_CONVERSION_AUTOMATIC

type
  SDL_DisplayMode* {.bycopy.} = object
    format*: uint32
    w*: cint
    h*: cint
    refresh_rate*: cint
    driverdata*: pointer
type
  SDL_SystemCursor* {.size: sizeof(cint).} = enum
    SDL_SYSTEM_CURSOR_ARROW, SDL_SYSTEM_CURSOR_IBEAM, SDL_SYSTEM_CURSOR_WAIT,
    SDL_SYSTEM_CURSOR_CROSSHAIR, SDL_SYSTEM_CURSOR_WAITARROW,
    SDL_SYSTEM_CURSOR_SIZENWSE, SDL_SYSTEM_CURSOR_SIZENESW,
    SDL_SYSTEM_CURSOR_SIZEWE, SDL_SYSTEM_CURSOR_SIZENS, SDL_SYSTEM_CURSOR_SIZEALL,
    SDL_SYSTEM_CURSOR_NO, SDL_SYSTEM_CURSOR_HAND, SDL_NUM_SYSTEM_CURSORS
  SDL_MouseWheelDirection* {.size: sizeof(cint).} = enum
    SDL_MOUSEWHEEL_NORMAL, SDL_MOUSEWHEEL_FLIPPED

  SDL_Joystick* {.bycopy.} = object

  SDL_JoystickGUID* {.bycopy.} = object
    data*: array[16, uint8]

  SDL_JoystickType* {.size: sizeof(cint).} = enum
    SDL_JOYSTICK_TYPE_UNKNOWN, SDL_JOYSTICK_TYPE_GAMECONTROLLER,
    SDL_JOYSTICK_TYPE_WHEEL, SDL_JOYSTICK_TYPE_ARCADE_STICK,
    SDL_JOYSTICK_TYPE_FLIGHT_STICK, SDL_JOYSTICK_TYPE_DANCE_PAD,
    SDL_JOYSTICK_TYPE_GUITAR, SDL_JOYSTICK_TYPE_DRUM_KIT,
    SDL_JOYSTICK_TYPE_ARCADE_PAD, SDL_JOYSTICK_TYPE_THROTTLE
  SDL_JoystickPowerLevel* {.size: sizeof(cint).} = enum
    SDL_JOYSTICK_POWER_UNKNOWN = -1, SDL_JOYSTICK_POWER_EMPTY,
    SDL_JOYSTICK_POWER_LOW, SDL_JOYSTICK_POWER_MEDIUM, SDL_JOYSTICK_POWER_FULL,
    SDL_JOYSTICK_POWER_WIRED, SDL_JOYSTICK_POWER_MAX
  SDL_GameController* {.bycopy.} = object

  INNER_C_STRUCT_playground_8495* {.bycopy.} = object
    hat*: cint
    hat_mask*: cint

  INNER_C_UNION_playground_8491* {.bycopy.} = object {.union.}
    button*: cint
    axis*: cint
    hat*: INNER_C_STRUCT_playground_8495

  SDL_GameControllerType* {.size: sizeof(cint).} = enum
    SDL_CONTROLLER_TYPE_UNKNOWN = 0, SDL_CONTROLLER_TYPE_XBOX360,
    SDL_CONTROLLER_TYPE_XBOXONE, SDL_CONTROLLER_TYPE_PS3, SDL_CONTROLLER_TYPE_PS4,
    SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_PRO
  SDL_GameControllerBindType* {.size: sizeof(cint).} = enum
    SDL_CONTROLLER_BINDTYPE_NONE = 0, SDL_CONTROLLER_BINDTYPE_BUTTON,
    SDL_CONTROLLER_BINDTYPE_AXIS, SDL_CONTROLLER_BINDTYPE_HAT
  SDL_GameControllerButtonBind* {.bycopy.} = object
    bindType*: SDL_GameControllerBindType
    value*: INNER_C_UNION_playground_8491
  SDL_Cursor = object
  SDL_Window = object
  SDL_WindowFlags* {.size: sizeof(cint).} = enum
    SDL_WINDOW_FULLSCREEN = 0x00000001, SDL_WINDOW_OPENGL = 0x00000002,
    SDL_WINDOW_SHOWN = 0x00000004, SDL_WINDOW_HIDDEN = 0x00000008,
    SDL_WINDOW_BORDERLESS = 0x00000010, SDL_WINDOW_RESIZABLE = 0x00000020,
    SDL_WINDOW_MINIMIZED = 0x00000040, SDL_WINDOW_MAXIMIZED = 0x00000080,
    SDL_WINDOW_INPUT_GRABBED = 0x00000100, SDL_WINDOW_INPUT_FOCUS = 0x00000200,
    SDL_WINDOW_MOUSE_FOCUS = 0x00000400,
    SDL_WINDOW_FOREIGN = 0x00000800,
    SDL_WINDOW_FULLSCREEN_DESKTOP = (SDL_WINDOW_FULLSCREEN.int or 0x00001000),
    SDL_WINDOW_ALLOW_HIGHDPI = 0x00002000,
    SDL_WINDOW_MOUSE_CAPTURE = 0x00004000, SDL_WINDOW_ALWAYS_ON_TOP = 0x00008000,
    SDL_WINDOW_SKIP_TASKBAR = 0x00010000, SDL_WINDOW_UTILITY = 0x00020000,
    SDL_WINDOW_TOOLTIP = 0x00040000, SDL_WINDOW_POPUP_MENU = 0x00080000,
    SDL_WINDOW_VULKAN = 0x10000000
  SDL_WindowEventID* {.size: sizeof(cint).} = enum
    SDL_WINDOWEVENT_NONE, SDL_WINDOWEVENT_SHOWN, SDL_WINDOWEVENT_HIDDEN,
    SDL_WINDOWEVENT_EXPOSED, SDL_WINDOWEVENT_MOVED, SDL_WINDOWEVENT_RESIZED,
    SDL_WINDOWEVENT_SIZE_CHANGED, SDL_WINDOWEVENT_MINIMIZED,
    SDL_WINDOWEVENT_MAXIMIZED, SDL_WINDOWEVENT_RESTORED, SDL_WINDOWEVENT_ENTER,
    SDL_WINDOWEVENT_LEAVE, SDL_WINDOWEVENT_FOCUS_GAINED,
    SDL_WINDOWEVENT_FOCUS_LOST, SDL_WINDOWEVENT_CLOSE, SDL_WINDOWEVENT_TAKE_FOCUS,
    SDL_WINDOWEVENT_HIT_TEST
  SDL_DisplayEventID* {.size: sizeof(cint).} = enum
    SDL_DISPLAYEVENT_NONE, SDL_DISPLAYEVENT_ORIENTATION
  SDL_DisplayOrientation* {.size: sizeof(cint).} = enum
    SDL_ORIENTATION_UNKNOWN, SDL_ORIENTATION_LANDSCAPE,
    SDL_ORIENTATION_LANDSCAPE_FLIPPED, SDL_ORIENTATION_PORTRAIT,
    SDL_ORIENTATION_PORTRAIT_FLIPPED
  SDL_GLContext* = pointer
  SDL_GLattr* {.size: sizeof(cint).} = enum
    SDL_GL_RED_SIZE, SDL_GL_GREEN_SIZE, SDL_GL_BLUE_SIZE, SDL_GL_ALPHA_SIZE,
    SDL_GL_BUFFER_SIZE, SDL_GL_DOUBLEBUFFER, SDL_GL_DEPTH_SIZE,
    SDL_GL_STENCIL_SIZE, SDL_GL_ACCUM_RED_SIZE, SDL_GL_ACCUM_GREEN_SIZE,
    SDL_GL_ACCUM_BLUE_SIZE, SDL_GL_ACCUM_ALPHA_SIZE, SDL_GL_STEREO,
    SDL_GL_MULTISAMPLEBUFFERS, SDL_GL_MULTISAMPLESAMPLES,
    SDL_GL_ACCELERATED_VISUAL, SDL_GL_RETAINED_BACKING,
    SDL_GL_CONTEXT_MAJOR_VERSION, SDL_GL_CONTEXT_MINOR_VERSION,
    SDL_GL_CONTEXT_EGL, SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_PROFILE_MASK,
    SDL_GL_SHARE_WITH_CURRENT_CONTEXT, SDL_GL_FRAMEBUFFER_SRGB_CAPABLE,
    SDL_GL_CONTEXT_RELEASE_BEHAVIOR, SDL_GL_CONTEXT_RESET_NOTIFICATION,
    SDL_GL_CONTEXT_NO_ERROR
  SDL_GLprofile* {.size: sizeof(cint).} = enum
    SDL_GL_CONTEXT_PROFILE_CORE = 0x00000001,
    SDL_GL_CONTEXT_PROFILE_COMPATIBILITY = 0x00000002,
    SDL_GL_CONTEXT_PROFILE_ES = 0x00000004
  SDL_GLcontextFlag* {.size: sizeof(cint).} = enum
    SDL_GL_CONTEXT_DEBUG_FLAG = 0x00000001,
    SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG = 0x00000002,
    SDL_GL_CONTEXT_ROBUST_ACCESS_FLAG = 0x00000004,
    SDL_GL_CONTEXT_RESET_ISOLATION_FLAG = 0x00000008
  SDL_GLcontextReleaseFlag* {.size: sizeof(cint).} = enum
    SDL_GL_CONTEXT_RELEASE_BEHAVIOR_NONE = 0x00000000,
    SDL_GL_CONTEXT_RELEASE_BEHAVIOR_FLUSH = 0x00000001
  # SDL_GLContextResetNotification* {.size: sizeof(cint).} = enum
  #   SDL_GL_CONTEXT_RESET_NO_NOTIFICATION = 0x00000000,
  #   SDL_GL_CONTEXT_RESET_LOSE_CONTEXT = 0x00000001
type
  SDL_GameControllerAxis* {.size: sizeof(cint).} = enum
    SDL_CONTROLLER_AXIS_INVALID = -1, SDL_CONTROLLER_AXIS_LEFTX,
    SDL_CONTROLLER_AXIS_LEFTY, SDL_CONTROLLER_AXIS_RIGHTX,
    SDL_CONTROLLER_AXIS_RIGHTY, SDL_CONTROLLER_AXIS_TRIGGERLEFT,
    SDL_CONTROLLER_AXIS_TRIGGERRIGHT, SDL_CONTROLLER_AXIS_MAX
type
  SDL_GameControllerButton* {.size: sizeof(cint).} = enum
    SDL_CONTROLLER_BUTTON_INVALID = -1, SDL_CONTROLLER_BUTTON_A,
    SDL_CONTROLLER_BUTTON_B, SDL_CONTROLLER_BUTTON_X, SDL_CONTROLLER_BUTTON_Y,
    SDL_CONTROLLER_BUTTON_BACK, SDL_CONTROLLER_BUTTON_GUIDE,
    SDL_CONTROLLER_BUTTON_START, SDL_CONTROLLER_BUTTON_LEFTSTICK,
    SDL_CONTROLLER_BUTTON_RIGHTSTICK, SDL_CONTROLLER_BUTTON_LEFTSHOULDER,
    SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, SDL_CONTROLLER_BUTTON_DPAD_UP,
    SDL_CONTROLLER_BUTTON_DPAD_DOWN, SDL_CONTROLLER_BUTTON_DPAD_LEFT,
    SDL_CONTROLLER_BUTTON_DPAD_RIGHT, SDL_CONTROLLER_BUTTON_MAX


type
  SDL_HitTestResult* {.size: sizeof(cint).} = enum
    SDL_HITTEST_NORMAL, SDL_HITTEST_DRAGGABLE, SDL_HITTEST_RESIZE_TOPLEFT,
    SDL_HITTEST_RESIZE_TOP, SDL_HITTEST_RESIZE_TOPRIGHT, SDL_HITTEST_RESIZE_RIGHT,
    SDL_HITTEST_RESIZE_BOTTOMRIGHT, SDL_HITTEST_RESIZE_BOTTOM,
    SDL_HITTEST_RESIZE_BOTTOMLEFT, SDL_HITTEST_RESIZE_LEFT
  SDL_HitTest* = proc (win: ptr SDL_Window; area: ptr SDL_Point; data: pointer): SDL_HitTestResult 




# const
#   SDL_PIXELFORMAT_RGBA32 = SDL_PIXELFORMAT_ABGR8888
#   SDL_PIXELFORMAT_ARGB32 = SDL_PIXELFORMAT_BGRA8888
#   SDL_PIXELFORMAT_BGRA32 = SDL_PIXELFORMAT_ARGB8888
#   SDL_PIXELFORMAT_ABGR32 = SDL_PIXELFORMAT_RGBA8888

proc SDL_PointInRect*(p: ptr SDL_Point; r: ptr SDL_Rect): SDL_bool {.inline.} =
  return if ((p.x >= r.x) and (p.x < (r.x + r.w)) and (p.y >= r.y) and (p.y < (r.y + r.h))): SDL_TRUE else: SDL_FALSE

proc SDL_RectEmpty*(r: ptr SDL_Rect): SDL_bool {.inline.} =
  return if ( r == nil or (r.w <= 0) or (r.h <= 0)): SDL_TRUE else: SDL_FALSE

proc SDL_RectEquals*(a: ptr SDL_Rect; b: ptr SDL_Rect): SDL_bool {.inline.} =
  return if (a != nil and b != nil and (a.x == b.x) and (a.y == b.y) and (a.w == b.w) and (a.h == b.h)): SDL_TRUE else: SDL_FALSE



type
  SDL_eventaction* {.size: sizeof(cint).} = enum
    SDL_ADDEVENT, SDL_PEEKEVENT, SDL_GETEVENT



type
  SDL_EventFilter* = proc (userdata: pointer; event: ptr SDL_Event): cint


type
  SDL_Haptic* {.bycopy.} = object

  SDL_HapticDirection* {.bycopy.} = object
    `type`*: uint8
    dir*: array[3, int32]

  SDL_HapticConstant* {.bycopy.} = object
    `type`*: uint16
    direction*: SDL_HapticDirection
    length*: uint32
    delay*: uint16
    button*: uint16
    interval*: uint16
    level*: int16
    attack_length*: uint16
    attack_level*: uint16
    fade_length*: uint16
    fade_level*: uint16

  SDL_HapticPeriodic* {.bycopy.} = object
    `type`*: uint16
    direction*: SDL_HapticDirection
    length*: uint32
    delay*: uint16
    button*: uint16
    interval*: uint16
    period*: uint16
    magnitude*: int16
    offset*: int16
    phase*: uint16
    attack_length*: uint16
    attack_level*: uint16
    fade_length*: uint16
    fade_level*: uint16

  SDL_HapticCondition* {.bycopy.} = object
    `type`*: uint16
    direction*: SDL_HapticDirection
    length*: uint32
    delay*: uint16
    button*: uint16
    interval*: uint16
    right_sat*: array[3, uint16]
    left_sat*: array[3, uint16]
    right_coeff*: array[3, int16]
    left_coeff*: array[3, int16]
    deadband*: array[3, uint16]
    center*: array[3, int16]

  SDL_HapticRamp* {.bycopy.} = object
    `type`*: uint16
    direction*: SDL_HapticDirection
    length*: uint32
    delay*: uint16
    button*: uint16
    interval*: uint16
    start*: int16
    `end`*: int16
    attack_length*: uint16
    attack_level*: uint16
    fade_length*: uint16
    fade_level*: uint16

  SDL_HapticLeftRight* {.bycopy.} = object
    `type`*: uint16
    length*: uint32
    large_magnitude*: uint16
    small_magnitude*: uint16

  SDL_HapticCustom* {.bycopy.} = object
    `type`*: uint16
    direction*: SDL_HapticDirection
    length*: uint32
    delay*: uint16
    button*: uint16
    interval*: uint16
    channels*: uint8
    period*: uint16
    samples*: uint16
    data*: ptr uint16
    attack_length*: uint16
    attack_level*: uint16
    fade_length*: uint16
    fade_level*: uint16

  SDL_HapticEffect* {.bycopy.} = object {.union.}
    `type`*: uint16
    constant*: SDL_HapticConstant
    periodic*: SDL_HapticPeriodic
    condition*: SDL_HapticCondition
    ramp*: SDL_HapticRamp
    leftright*: SDL_HapticLeftRight
    custom*: SDL_HapticCustom



type
  SDL_HintPriority* {.size: sizeof(cint).} = enum
    SDL_HINT_DEFAULT, SDL_HINT_NORMAL, SDL_HINT_OVERRIDE



type
  SDL_LogCategory* {.size: sizeof(cint).} = enum
    SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_CATEGORY_ERROR, SDL_LOG_CATEGORY_ASSERT,
    SDL_LOG_CATEGORY_SYSTEM, SDL_LOG_CATEGORY_AUDIO, SDL_LOG_CATEGORY_VIDEO,
    SDL_LOG_CATEGORY_RENDER, SDL_LOG_CATEGORY_INPUT, SDL_LOG_CATEGORY_TEST,
    SDL_LOG_CATEGORY_RESERVED1, SDL_LOG_CATEGORY_RESERVED2,
    SDL_LOG_CATEGORY_RESERVED3, SDL_LOG_CATEGORY_RESERVED4,
    SDL_LOG_CATEGORY_RESERVED5, SDL_LOG_CATEGORY_RESERVED6,
    SDL_LOG_CATEGORY_RESERVED7, SDL_LOG_CATEGORY_RESERVED8,
    SDL_LOG_CATEGORY_RESERVED9, SDL_LOG_CATEGORY_RESERVED10,
    SDL_LOG_CATEGORY_CUSTOM
  SDL_LogPriority* {.size: sizeof(cint).} = enum
    SDL_LOG_PRIORITY_VERBOSE = 1, SDL_LOG_PRIORITY_DEBUG, SDL_LOG_PRIORITY_INFO,
    SDL_LOG_PRIORITY_WARN, SDL_LOG_PRIORITY_ERROR, SDL_LOG_PRIORITY_CRITICAL,
    SDL_NUM_LOG_PRIORITIES
type
  SDL_MessageBoxFlags* {.size: sizeof(cint).} = enum
    SDL_MESSAGEBOX_ERROR = 0x00000010, SDL_MESSAGEBOX_WARNING = 0x00000020,
    SDL_MESSAGEBOX_INFORMATION = 0x00000040,
    SDL_MESSAGEBOX_BUTTONS_LEFT_TO_RIGHT = 0x00000080,
    SDL_MESSAGEBOX_BUTTONS_RIGHT_TO_LEFT = 0x00000100
  SDL_MessageBoxButtonFlags* {.size: sizeof(cint).} = enum
    SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT = 0x00000001,
    SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT = 0x00000002
  SDL_MessageBoxButtonData* {.bycopy.} = object
    flags*: uint32
    buttonid*: cint
    text*: cstring

  SDL_MessageBoxColor* {.bycopy.} = object
    r*: uint8
    g*: uint8
    b*: uint8

  SDL_MessageBoxColorType* {.size: sizeof(cint).} = enum
    SDL_MESSAGEBOX_COLOR_BACKGROUND, SDL_MESSAGEBOX_COLOR_TEXT,
    SDL_MESSAGEBOX_COLOR_BUTTON_BORDER, SDL_MESSAGEBOX_COLOR_BUTTON_BACKGROUND,
    SDL_MESSAGEBOX_COLOR_BUTTON_SELECTED, SDL_MESSAGEBOX_COLOR_MAX
  SDL_MessageBoxColorScheme* {.bycopy.} = object
    colors*: array[SDL_MESSAGEBOX_COLOR_MAX, SDL_MessageBoxColor]

  SDL_MessageBoxData* {.bycopy.} = object
    flags*: uint32
    window*: ptr SDL_Window
    title*: cstring
    message*: cstring
    numbuttons*: cint
    buttons*: ptr SDL_MessageBoxButtonData
    colorScheme*: ptr SDL_MessageBoxColorScheme

type
  SDL_MetalView* = pointer
type
  SDL_PowerState* {.size: sizeof(cint).} = enum
    SDL_POWERSTATE_UNKNOWN, SDL_POWERSTATE_ON_BATTERY, SDL_POWERSTATE_NO_BATTERY,
    SDL_POWERSTATE_CHARGING, SDL_POWERSTATE_CHARGED


type
  SDL_RendererFlags* {.size: sizeof(cint).} = enum
    SDL_RENDERER_SOFTWARE = 0x00000001, SDL_RENDERER_ACCELERATED = 0x00000002,
    SDL_RENDERER_PRESENTVSYNC = 0x00000004, SDL_RENDERER_TARGETTEXTURE = 0x00000008
    
  SDL_RendererInfo* {.bycopy.} = object
    name*: cstring
    flags*: uint32
    num_texture_formats*: uint32
    texture_formats*: array[16, uint32]
    max_texture_width*: cint
    max_texture_height*: cint

  SDL_ScaleMode* {.size: sizeof(cint).} = enum
    SDL_ScaleModeNearest, SDL_ScaleModeLinear, SDL_ScaleModeBest
  SDL_TextureAccess* {.size: sizeof(cint).} = enum
    SDL_TEXTUREACCESS_STATIC, SDL_TEXTUREACCESS_STREAMING,
    SDL_TEXTUREACCESS_TARGET
  SDL_TextureModulate* {.size: sizeof(cint).} = enum
    SDL_TEXTUREMODULATE_NONE = 0x00000000, SDL_TEXTUREMODULATE_COLOR = 0x00000001,
    SDL_TEXTUREMODULATE_ALPHA = 0x00000002
  SDL_RendererFlip* {.size: sizeof(cint).} = enum
    SDL_FLIP_NONE = 0x00000000, SDL_FLIP_HORIZONTAL = 0x00000001,
    SDL_FLIP_VERTICAL = 0x00000002


type
  SDL_Renderer* {.bycopy,importc.} = object

  SDL_Texture* {.bycopy,importc.} = object

type
  SDL_SensorID* = int32
  SDL_SensorType* {.size: sizeof(cint).} = enum
    SDL_SENSOR_INVALID = -1, SDL_SENSOR_UNKNOWN, SDL_SENSOR_ACCEL, SDL_SENSOR_GYRO
  ShowMode* {.size: sizeof(cint).} = enum
    SHOW_MODE_NONE = -1, SHOW_MODE_VIDEO = 0, SHOW_MODE_WAVES, SHOW_MODE_RDFT,
    SHOW_MODE_NB
  WindowShapeMode* {.size: sizeof(cint).} = enum
    ShapeModeDefault, ShapeModeBinarizeAlpha, ShapeModeReverseBinarizeAlpha,
    ShapeModeColorKey

  SDL_WindowShapeParams* {.union.} = object
    binarizationCutoff*: uint8
    colorKey*: SDL_Color

  SDL_WindowShapeMode* {.bycopy.} = object
    mode*: WindowShapeMode
    parameters*: SDL_WindowShapeParams

  SDL_version* {.bycopy.} = object
    major*: uint8
    minor*: uint8
    patch*: uint8

  SpecifierOptUnion* {.union.} = object 
    str*: ptr uint8
    i*: cint
    i64*: int64
    ui64*: uint64
    f*: cfloat
    dbl*: cdouble

  SpecifierOpt* {.bycopy.} = object
    specifier*: cstring
    u*: SpecifierOptUnion

  OptionDefUnion* {.union.} = object 
    dst_ptr*: pointer
    func_arg*: proc (a1: pointer; a2: cstring; a3: cstring): cint
    off*: csize_t

  OptionDef* {.bycopy.} = object
    name*: cstring
    flags*: cint
    u*: OptionDefUnion
    help*: cstring
    argname*: cstring
    
  Option* {.bycopy.} = object
    opt*: ptr OptionDef
    key*: cstring
    val*: cstring

  OptionGroupDef* {.bycopy.} = object
    name*: cstring
    sep*: cstring
    flags*: cint

  OptionGroup* {.bycopy.} = object
    group_def*: ptr OptionGroupDef
    arg*: cstring
    opts*: ptr Option
    nb_opts*: cint
    codec_opts*: ptr AVDictionary
    format_opts*: ptr AVDictionary
    resample_opts*: ptr AVDictionary
    sws_dict*: ptr AVDictionary
    swr_opts*: ptr AVDictionary

  OptionGroupList* {.bycopy.} = object
    group_def*: ptr OptionGroupDef
    groups*: ptr OptionGroup
    nb_groups*: cint

  OptionParseContext* {.bycopy.} = object
    global_opts*: OptionGroup
    groups*: ptr OptionGroupList
    nb_groups*: cint
    cur_group*: OptionGroup
  
  SDL_Sensor = object
  IDirect3DDevice9 = object
  SDL_TimerCallback* = proc (interval: uint32; param: pointer): uint32
  SDL_TimerID* = cint
type
  SDL_WindowsMessageHook* = proc (userdata: pointer; hWnd: pointer; message: cuint;
                               wParam: uint64; lParam: int64)
proc SDL_NumSensors*(): cint

type
  SDL_LogOutputFunction* = proc (userdata: pointer; category: cint;
                              priority: SDL_LogPriority; message: cstring)

type
  SDL_HintCallback* = proc (userdata: pointer; name: cstring; oldValue: cstring; newValue: cstring)

{.push importc, dynlib: LibName, discardable.}
proc SDL_GetPlatform*(): cstring
proc SDL_malloc*(size: csize_t): pointer
proc SDL_calloc*(nmemb: csize_t; size: csize_t): pointer
proc SDL_realloc*(mem: pointer; size: csize_t): pointer
proc SDL_free*(mem: pointer)
proc SDL_GetMemoryFunctions*(malloc_func: ptr SDL_malloc_func;
                            calloc_func: ptr SDL_calloc_func;
                            realloc_func: ptr SDL_realloc_func;
                            free_func: ptr SDL_free_func)
proc SDL_SetMemoryFunctions*(malloc_func: SDL_malloc_func;
                            calloc_func: SDL_calloc_func;
                            realloc_func: SDL_realloc_func;
                            free_func: SDL_free_func): cint
proc SDL_GetNumAllocations*(): cint
proc SDL_getenv*(name: cstring): cstring
proc SDL_setenv*(name: cstring; value: cstring; overwrite: cint): cint
proc SDL_qsort*(base: pointer; nmemb: csize_t; size: csize_t;
               compare: proc (a1: pointer; a2: pointer): cint)
proc SDL_abs*(x: cint): cint
proc SDL_isdigit*(x: cint): cint
proc SDL_isspace*(x: cint): cint
proc SDL_isupper*(x: cint): cint
proc SDL_islower*(x: cint): cint
proc SDL_toupper*(x: cint): cint
proc SDL_tolower*(x: cint): cint
proc SDL_memset*(dst: pointer; c: cint; len: csize_t): pointer
proc SDL_memcpy*(dst: pointer; src: pointer; len: csize_t): pointer
proc SDL_memmove*(dst: pointer; src: pointer; len: csize_t): pointer
proc SDL_memcmp*(s1: pointer; s2: pointer; len: csize_t): cint
proc SDL_wcslen*(wstr: ptr int32): csize_t
proc SDL_wcslcpy*(dst: ptr int32; src: ptr int32; maxlen: csize_t): csize_t
proc SDL_wcslcat*(dst: ptr int32; src: ptr int32; maxlen: csize_t): csize_t
proc SDL_wcsdup*(wstr: ptr int32): ptr int32
proc SDL_wcsstr*(haystack: ptr int32; needle: ptr int32): ptr int32
proc SDL_wcscmp*(str1: ptr int32; str2: ptr int32): cint
proc SDL_wcsncmp*(str1: ptr int32; str2: ptr int32; maxlen: csize_t): cint
proc SDL_strlen*(str: cstring): csize_t
proc SDL_strlcpy*(dst: cstring; src: cstring; maxlen: csize_t): csize_t
proc SDL_utf8strlcpy*(dst: cstring; src: cstring; dst_bytes: csize_t): csize_t
proc SDL_strlcat*(dst: cstring; src: cstring; maxlen: csize_t): csize_t
proc SDL_strdup*(str: cstring): cstring
proc SDL_strrev*(str: cstring): cstring
proc SDL_strupr*(str: cstring): cstring
proc SDL_strlwr*(str: cstring): cstring
proc SDL_strchr*(str: cstring; c: cint): cstring
proc SDL_strrchr*(str: cstring; c: cint): cstring
proc SDL_strstr*(haystack: cstring; needle: cstring): cstring
proc SDL_strtokr*(s1: cstring; s2: cstring; saveptr: cstringArray): cstring
proc SDL_utf8strlen*(str: cstring): csize_t
proc SDL_itoa*(value: cint; str: cstring; radix: cint): cstring
proc SDL_uitoa*(value: cuint; str: cstring; radix: cint): cstring
proc SDL_ltoa*(value: clong; str: cstring; radix: cint): cstring
proc SDL_ultoa*(value: culong; str: cstring; radix: cint): cstring
proc SDL_lltoa*(value: int64; str: cstring; radix: cint): cstring
proc SDL_ulltoa*(value: uint64; str: cstring; radix: cint): cstring
proc SDL_atoi*(str: cstring): cint
proc SDL_atof*(str: cstring): cdouble
proc SDL_strtol*(str: cstring; endp: cstringArray; base: cint): clong
proc SDL_strtoul*(str: cstring; endp: cstringArray; base: cint): culong
proc SDL_strtoll*(str: cstring; endp: cstringArray; base: cint): int64
proc SDL_strtoull*(str: cstring; endp: cstringArray; base: cint): uint64
proc SDL_strtod*(str: cstring; endp: cstringArray): cdouble
proc SDL_strcmp*(str1: cstring; str2: cstring): cint
proc SDL_strncmp*(str1: cstring; str2: cstring; maxlen: csize_t): cint
proc SDL_strcasecmp*(str1: cstring; str2: cstring): cint
proc SDL_strncasecmp*(str1: cstring; str2: cstring; len: csize_t): cint
proc SDL_sscanf*(text: cstring; fmt: cstring): cint {.varargs.}
proc SDL_vsscanf*(text: cstring; fmt: cstring; ap: varargs[untyped]): cint
proc SDL_snprintf*(text: cstring; maxlen: csize_t; fmt: cstring): cint {.varargs.}
proc SDL_vsnprintf*(text: cstring; maxlen: csize_t; fmt: cstring; ap: varargs[untyped]): cint
proc SDL_acos*(x: cdouble): cdouble
proc SDL_acosf*(x: cfloat): cfloat
proc SDL_asin*(x: cdouble): cdouble
proc SDL_asinf*(x: cfloat): cfloat
proc SDL_atan*(x: cdouble): cdouble
proc SDL_atanf*(x: cfloat): cfloat
proc SDL_atan2*(x: cdouble; y: cdouble): cdouble
proc SDL_atan2f*(x: cfloat; y: cfloat): cfloat
proc SDL_ceil*(x: cdouble): cdouble
proc SDL_ceilf*(x: cfloat): cfloat
proc SDL_copysign*(x: cdouble; y: cdouble): cdouble
proc SDL_copysignf*(x: cfloat; y: cfloat): cfloat
proc SDL_cos*(x: cdouble): cdouble
proc SDL_cosf*(x: cfloat): cfloat
proc SDL_exp*(x: cdouble): cdouble
proc SDL_expf*(x: cfloat): cfloat
proc SDL_fabs*(x: cdouble): cdouble
proc SDL_fabsf*(x: cfloat): cfloat
proc SDL_floor*(x: cdouble): cdouble
proc SDL_floorf*(x: cfloat): cfloat
proc SDL_fmod*(x: cdouble; y: cdouble): cdouble
proc SDL_fmodf*(x: cfloat; y: cfloat): cfloat
proc SDL_log*(x: cdouble): cdouble
proc SDL_logf*(x: cfloat): cfloat
proc SDL_log10*(x: cdouble): cdouble
proc SDL_log10f*(x: cfloat): cfloat
proc SDL_pow*(x: cdouble; y: cdouble): cdouble
proc SDL_powf*(x: cfloat; y: cfloat): cfloat
proc SDL_scalbn*(x: cdouble; n: cint): cdouble
proc SDL_scalbnf*(x: cfloat; n: cint): cfloat
proc SDL_sin*(x: cdouble): cdouble
proc SDL_sinf*(x: cfloat): cfloat
proc SDL_sqrt*(x: cdouble): cdouble
proc SDL_sqrtf*(x: cfloat): cfloat
proc SDL_tan*(x: cdouble): cdouble
proc SDL_tanf*(x: cfloat): cfloat
proc SDL_iconv_string*(tocode: cstring; fromcode: cstring; inbuf: cstring;inbytesleft: csize_t): cstring
proc SDL_main*(argc: cint; argv: ptr cstring): cint
proc SDL_SetMainReady*()
proc SDL_RegisterApp*(name: cstring; style: uint32; hInst: pointer): cint
proc SDL_UnregisterApp*()
proc SDL_ReportAssertion*(a1: ptr SDL_AssertData; a2: cstring; a3: cstring; a4: cint): SDL_AssertState
proc SDL_AtomicTryLock*(lock: ptr SDL_SpinLock): SDL_bool
proc SDL_SetAssertionHandler*(handler: SDL_AssertionHandler; userdata: pointer)
proc SDL_GetDefaultAssertionHandler*(): SDL_AssertionHandler
proc SDL_GetAssertionHandler*(puserdata: ptr pointer): SDL_AssertionHandler
proc SDL_GetAssertionReport*(): ptr SDL_AssertData
proc SDL_ResetAssertionReport*()
proc SDL_AtomicLock*(lock: ptr SDL_SpinLock)
proc SDL_AtomicUnlock*(lock: ptr SDL_SpinLock)
proc SDL_MemoryBarrierReleaseFunction*()
proc SDL_MemoryBarrierAcquireFunction*()
proc SDL_AtomicCAS*(a: ptr SDL_atomic_t; oldval: cint; newval: cint): SDL_bool
proc SDL_AtomicSet*(a: ptr SDL_atomic_t; v: cint): cint
proc SDL_AtomicGet*(a: ptr SDL_atomic_t): cint
proc SDL_AtomicAdd*(a: ptr SDL_atomic_t; v: cint): cint
proc SDL_AtomicCASPtr*(a: ptr pointer; oldval: pointer; newval: pointer): SDL_bool
proc SDL_AtomicSetPtr*(a: ptr pointer; v: pointer): pointer
proc SDL_AtomicGetPtr*(a: ptr pointer): pointer
proc SDL_SetError*(fmt: cstring): cint {.varargs.}
proc SDL_GetError*(): cstring
proc SDL_ClearError*()
proc SDL_Error*(code: SDL_errorcode): cint
proc SDL_CreateMutex*(): ptr SDL_mutex 
proc SDL_LockMutex*(mutex: ptr SDL_mutex): cint 
proc SDL_TryLockMutex*(mutex: ptr SDL_mutex): cint 
template SDL_mutexV*(m: untyped): untyped = SDL_UnlockMutex(m)
proc SDL_UnlockMutex*(mutex: ptr SDL_mutex): cint 
proc SDL_DestroyMutex*(mutex: ptr SDL_mutex) 
proc SDL_CreateSemaphore*(initial_value: uint32): ptr SDL_sem
proc SDL_DestroySemaphore*(sem: ptr SDL_sem)
proc SDL_SemWait*(sem: ptr SDL_sem): cint
proc SDL_SemTryWait*(sem: ptr SDL_sem): cint
proc SDL_SemWaitTimeout*(sem: ptr SDL_sem; ms: uint32): cint
proc SDL_SemPost*(sem: ptr SDL_sem): cint
proc SDL_SemValue*(sem: ptr SDL_sem): uint32

proc SDL_CreateCond*(): ptr SDL_cond
proc SDL_DestroyCond*(cond: ptr SDL_cond)
proc SDL_CondSignal*(cond: ptr SDL_cond): cint 
proc SDL_CondBroadcast*(cond: ptr SDL_cond): cint 
proc SDL_CondWait*(cond: ptr SDL_cond; mutex: ptr SDL_mutex): cint 
proc SDL_CondWaitTimeout*(cond: ptr SDL_cond; mutex: ptr SDL_mutex; ms: uint32): cint 
proc SDL_CreateThread*(fn: SDL_ThreadFunction; name: cstring; data: pointer;): ptr SDL_Thread
proc SDL_CreateThreadWithStackSize*(fn: proc (a1: pointer): cint; name: cstring;
                                   stacksize: csize_t; data: pointer;
                                   pfnBeginThread: pfnSDL_CurrentBeginThread;
                                   pfnEndThread: pfnSDL_CurrentEndThread): ptr SDL_Thread
proc SDL_GetThreadName*(thread: ptr SDL_Thread): cstring
# proc SDL_ThreadID*(): SDL_threadID
proc SDL_GetThreadID*(thread: ptr SDL_Thread): SDL_threadID
proc SDL_SetThreadPriority*(priority: SDL_ThreadPriority): cint
proc SDL_WaitThread*(thread: ptr SDL_Thread; status: ptr cint)
proc SDL_DetachThread*(thread: ptr SDL_Thread)
proc SDL_TLSCreate*(): SDL_TLSID
proc SDL_TLSGet*(id: SDL_TLSID): pointer
proc SDL_TLSSet*(id: SDL_TLSID; value: pointer; destructor: proc (a1: pointer)): cint
proc SDL_RWFromFile*(file: cstring; mode: cstring): ptr SDL_RWops
proc SDL_RWFromFP*(fp: pointer; autoclose: SDL_bool): ptr SDL_RWops
proc SDL_RWFromMem*(mem: pointer; size: cint): ptr SDL_RWops
proc SDL_RWFromConstMem*(mem: pointer; size: cint): ptr SDL_RWops
proc SDL_AllocRW*(): ptr SDL_RWops
proc SDL_FreeRW*(area: ptr SDL_RWops)
proc SDL_RWsize*(context: ptr SDL_RWops): int64
proc SDL_RWseek*(context: ptr SDL_RWops; offset: int64; whence: cint): int64
proc SDL_RWtell*(context: ptr SDL_RWops): int64
proc SDL_RWread*(context: ptr SDL_RWops; `ptr`: pointer; size: csize_t; maxnum: csize_t): csize_t
proc SDL_RWwrite*(context: ptr SDL_RWops; `ptr`: pointer; size: csize_t; num: csize_t): csize_t
proc SDL_RWclose*(context: ptr SDL_RWops): cint
proc SDL_LoadFile_RW*(src: ptr SDL_RWops; datasize: ptr csize_t; freesrc: cint): pointer
proc SDL_LoadFile*(file: cstring; datasize: ptr csize_t): pointer
proc SDL_ReadU8*(src: ptr SDL_RWops): uint8
proc SDL_ReadLE16*(src: ptr SDL_RWops): uint16
proc SDL_ReadBE16*(src: ptr SDL_RWops): uint16
proc SDL_ReadLE32*(src: ptr SDL_RWops): uint32
proc SDL_ReadBE32*(src: ptr SDL_RWops): uint32
proc SDL_ReadLE64*(src: ptr SDL_RWops): uint64
proc SDL_ReadBE64*(src: ptr SDL_RWops): uint64
proc SDL_WriteU8*(dst: ptr SDL_RWops; value: uint8): csize_t
proc SDL_WriteLE16*(dst: ptr SDL_RWops; value: uint16): csize_t
proc SDL_WriteBE16*(dst: ptr SDL_RWops; value: uint16): csize_t
proc SDL_WriteLE32*(dst: ptr SDL_RWops; value: uint32): csize_t
proc SDL_WriteBE32*(dst: ptr SDL_RWops; value: uint32): csize_t
proc SDL_WriteLE64*(dst: ptr SDL_RWops; value: uint64): csize_t
proc SDL_WriteBE64*(dst: ptr SDL_RWops; value: uint64): csize_t
proc SDL_GetNumAudioDrivers*(): cint
proc SDL_GetAudioDriver*(index: cint): cstring
proc SDL_AudioInit*(driver_name: cstring): cint
proc SDL_AudioQuit*()
proc SDL_GetCurrentAudioDriver*(): cstring
proc SDL_OpenAudio*(desired: ptr SDL_AudioSpec; obtained: ptr SDL_AudioSpec): cint
proc SDL_GetNumAudioDevices*(iscapture: cint): cint
proc SDL_GetAudioDeviceName*(index: cint; iscapture: cint): cstring
proc SDL_OpenAudioDevice*(device: cstring; iscapture: cint;
                         desired: ptr SDL_AudioSpec; obtained: ptr SDL_AudioSpec;
                         allowed_changes: cint): SDL_AudioDeviceID
proc SDL_GetAudioStatus*(): SDL_AudioStatus
proc SDL_GetAudioDeviceStatus*(dev: SDL_AudioDeviceID): SDL_AudioStatus
proc SDL_PauseAudio*(pause_on: cint)
proc SDL_PauseAudioDevice*(dev: SDL_AudioDeviceID; pause_on: cint)
proc SDL_LoadWAV_RW*(src: ptr SDL_RWops; freesrc: cint; spec: ptr SDL_AudioSpec;
                    audio_buf: ptr ptr uint8; audio_len: ptr uint32): ptr SDL_AudioSpec
proc SDL_FreeWAV*(audio_buf: ptr uint8)
proc SDL_BuildAudioCVT*(cvt: ptr SDL_AudioCVT; src_format: SDL_AudioFormat;
                       src_channels: uint8; src_rate: cint;
                       dst_format: SDL_AudioFormat; dst_channels: uint8;
                       dst_rate: cint): cint
proc SDL_ConvertAudio*(cvt: ptr SDL_AudioCVT): cint
proc SDL_NewAudioStream*(src_format: SDL_AudioFormat; src_channels: uint8;
                        src_rate: cint; dst_format: SDL_AudioFormat;
                        dst_channels: uint8; dst_rate: cint): ptr SDL_AudioStream
proc SDL_AudioStreamPut*(stream: ptr SDL_AudioStream; buf: pointer; len: cint): cint
proc SDL_AudioStreamGet*(stream: ptr SDL_AudioStream; buf: pointer; len: cint): cint
proc SDL_AudioStreamAvailable*(stream: ptr SDL_AudioStream): cint
proc SDL_AudioStreamFlush*(stream: ptr SDL_AudioStream): cint
proc SDL_AudioStreamClear*(stream: ptr SDL_AudioStream)
proc SDL_FreeAudioStream*(stream: ptr SDL_AudioStream)
proc SDL_MixAudio*(dst: ptr uint8; src: ptr uint8; len: uint32; volume: cint)
proc SDL_MixAudioFormat*(dst: ptr uint8; src: ptr uint8; format: SDL_AudioFormat;
                        len: uint32; volume: cint)
proc SDL_QueueAudio*(dev: SDL_AudioDeviceID; data: pointer; len: uint32): cint
proc SDL_DequeueAudio*(dev: SDL_AudioDeviceID; data: pointer; len: uint32): uint32
proc SDL_GetQueuedAudioSize*(dev: SDL_AudioDeviceID): uint32
proc SDL_ClearQueuedAudio*(dev: SDL_AudioDeviceID)
proc SDL_LockAudio*()
proc SDL_LockAudioDevice*(dev: SDL_AudioDeviceID)
proc SDL_UnlockAudio*()
proc SDL_UnlockAudioDevice*(dev: SDL_AudioDeviceID)
proc SDL_CloseAudio*()
proc SDL_CloseAudioDevice*(dev: SDL_AudioDeviceID)
proc SDL_SetClipboardText*(text: cstring): cint
proc SDL_GetClipboardText*(): cstring
proc SDL_HasClipboardText*(): SDL_bool
proc SDL_GetCPUCount*(): cint
proc SDL_GetCPUCacheLineSize*(): cint
proc SDL_HasRDTSC*(): SDL_bool
proc SDL_HasAltiVec*(): SDL_bool
proc SDL_HasMMX*(): SDL_bool
proc SDL_Has3DNow*(): SDL_bool
proc SDL_HasSSE*(): SDL_bool
proc SDL_HasSSE2*(): SDL_bool
proc SDL_HasSSE3*(): SDL_bool
proc SDL_HasSSE41*(): SDL_bool
proc SDL_HasSSE42*(): SDL_bool
proc SDL_HasAVX*(): SDL_bool
proc SDL_HasAVX2*(): SDL_bool
proc SDL_HasAVX512F*(): SDL_bool
proc SDL_HasARMSIMD*(): SDL_bool
proc SDL_HasNEON*(): SDL_bool
proc SDL_GetSystemRAM*(): cint
proc SDL_SIMDGetAlignment*(): csize_t
proc SDL_SIMDAlloc*(len: csize_t): pointer
proc SDL_SIMDFree*(`ptr`: pointer)
proc SDL_GetPixelFormatName*(format: uint32): cstring
proc SDL_PixelFormatEnumToMasks*(format: uint32; bpp: ptr cint; Rmask: ptr uint32;
                                Gmask: ptr uint32; Bmask: ptr uint32;
                                Amask: ptr uint32): SDL_bool
proc SDL_MasksToPixelFormatEnum*(bpp: cint; Rmask: uint32; Gmask: uint32;
                                Bmask: uint32; Amask: uint32): uint32
proc SDL_AllocFormat*(pixel_format: uint32): ptr SDL_PixelFormat
proc SDL_FreeFormat*(format: ptr SDL_PixelFormat)
proc SDL_AllocPalette*(ncolors: cint): ptr SDL_Palette
proc SDL_SetPixelFormatPalette*(format: ptr SDL_PixelFormat;
                               palette: ptr SDL_Palette): cint
proc SDL_SetPaletteColors*(palette: ptr SDL_Palette; colors: ptr SDL_Color;
                          firstcolor: cint; ncolors: cint): cint
proc SDL_FreePalette*(palette: ptr SDL_Palette)
proc SDL_MapRGB*(format: ptr SDL_PixelFormat; r: uint8; g: uint8; b: uint8): uint32
proc SDL_MapRGBA*(format: ptr SDL_PixelFormat; r: uint8; g: uint8; b: uint8; a: uint8): uint32
proc SDL_GetRGB*(pixel: uint32; format: ptr SDL_PixelFormat; r: ptr uint8; g: ptr uint8;
                b: ptr uint8)
proc SDL_GetRGBA*(pixel: uint32; format: ptr SDL_PixelFormat; r: ptr uint8; g: ptr uint8;
                 b: ptr uint8; a: ptr uint8)
proc SDL_CalculateGammaRamp*(gamma: cfloat; ramp: ptr uint16)
proc SDL_HasIntersection*(A: ptr SDL_Rect; B: ptr SDL_Rect): SDL_bool
proc SDL_IntersectRect*(A: ptr SDL_Rect; B: ptr SDL_Rect; result: ptr SDL_Rect): SDL_bool
proc SDL_UnionRect*(A: ptr SDL_Rect; B: ptr SDL_Rect; result: ptr SDL_Rect)
proc SDL_EnclosePoints*(points: ptr SDL_Point; count: cint; clip: ptr SDL_Rect;result: ptr SDL_Rect): SDL_bool
proc SDL_IntersectRectAndLine*(rect: ptr SDL_Rect; X1: ptr cint; Y1: ptr cint;X2: ptr cint; Y2: ptr cint): SDL_bool
proc SDL_ComposeCustomBlendMode*(srcColorFactor: SDL_BlendFactor;
                                dstColorFactor: SDL_BlendFactor;
                                colorOperation: SDL_BlendOperation;
                                srcAlphaFactor: SDL_BlendFactor;
                                dstAlphaFactor: SDL_BlendFactor;
                                alphaOperation: SDL_BlendOperation): SDL_BlendMode
proc SDL_CreateRGBSurface*(flags: uint32; width: cint; height: cint; depth: cint;
                          Rmask: uint32; Gmask: uint32; Bmask: uint32; Amask: uint32): ptr SDL_Surface
proc SDL_CreateRGBSurfaceWithFormat*(flags: uint32; width: cint; height: cint;
                                    depth: cint; format: uint32): ptr SDL_Surface
proc SDL_CreateRGBSurfaceFrom*(pixels: pointer; width: cint; height: cint; depth: cint;
                              pitch: cint; Rmask: uint32; Gmask: uint32;
                              Bmask: uint32; Amask: uint32): ptr SDL_Surface
proc SDL_CreateRGBSurfaceWithFormatFrom*(pixels: pointer; width: cint; height: cint;
                                        depth: cint; pitch: cint; format: uint32): ptr SDL_Surface
proc SDL_FreeSurface*(surface: ptr SDL_Surface)
proc SDL_SetSurfacePalette*(surface: ptr SDL_Surface; palette: ptr SDL_Palette): cint
proc SDL_LockSurface*(surface: ptr SDL_Surface): cint
proc SDL_UnlockSurface*(surface: ptr SDL_Surface)
proc SDL_LoadBMP_RW*(src: ptr SDL_RWops; freesrc: cint): ptr SDL_Surface
proc SDL_SaveBMP_RW*(surface: ptr SDL_Surface; dst: ptr SDL_RWops; freedst: cint): cint
proc SDL_SetSurfaceRLE*(surface: ptr SDL_Surface; flag: cint): cint
proc SDL_SetColorKey*(surface: ptr SDL_Surface; flag: cint; key: uint32): cint
proc SDL_HasColorKey*(surface: ptr SDL_Surface): SDL_bool
proc SDL_GetColorKey*(surface: ptr SDL_Surface; key: ptr uint32): cint
proc SDL_SetSurfaceColorMod*(surface: ptr SDL_Surface; r: uint8; g: uint8; b: uint8): cint
proc SDL_GetSurfaceColorMod*(surface: ptr SDL_Surface; r: ptr uint8; g: ptr uint8;
                            b: ptr uint8): cint
proc SDL_SetSurfaceAlphaMod*(surface: ptr SDL_Surface; alpha: uint8): cint
proc SDL_GetSurfaceAlphaMod*(surface: ptr SDL_Surface; alpha: ptr uint8): cint
proc SDL_SetSurfaceBlendMode*(surface: ptr SDL_Surface; blendMode: SDL_BlendMode): cint
proc SDL_GetSurfaceBlendMode*(surface: ptr SDL_Surface; blendMode: ptr SDL_BlendMode): cint
proc SDL_SetClipRect*(surface: ptr SDL_Surface; rect: ptr SDL_Rect): SDL_bool
proc SDL_GetClipRect*(surface: ptr SDL_Surface; rect: ptr SDL_Rect)
proc SDL_DuplicateSurface*(surface: ptr SDL_Surface): ptr SDL_Surface
proc SDL_ConvertSurface*(src: ptr SDL_Surface; fmt: ptr SDL_PixelFormat; flags: uint32): ptr SDL_Surface
proc SDL_ConvertSurfaceFormat*(src: ptr SDL_Surface; pixel_format: uint32;
                              flags: uint32): ptr SDL_Surface
proc SDL_ConvertPixels*(width: cint; height: cint; src_format: uint32; src: pointer;
                       src_pitch: cint; dst_format: uint32; dst: pointer;
                       dst_pitch: cint): cint
proc SDL_FillRect*(dst: ptr SDL_Surface; rect: ptr SDL_Rect; color: uint32): cint
proc SDL_FillRects*(dst: ptr SDL_Surface; rects: ptr SDL_Rect; count: cint; color: uint32): cint
proc SDL_UpperBlit*(src: ptr SDL_Surface; srcrect: ptr SDL_Rect; dst: ptr SDL_Surface;
                   dstrect: ptr SDL_Rect): cint
proc SDL_LowerBlit*(src: ptr SDL_Surface; srcrect: ptr SDL_Rect; dst: ptr SDL_Surface;
                   dstrect: ptr SDL_Rect): cint
proc SDL_SoftStretch*(src: ptr SDL_Surface; srcrect: ptr SDL_Rect;
                     dst: ptr SDL_Surface; dstrect: ptr SDL_Rect): cint
proc SDL_UpperBlitScaled*(src: ptr SDL_Surface; srcrect: ptr SDL_Rect;
                         dst: ptr SDL_Surface; dstrect: ptr SDL_Rect): cint
proc SDL_LowerBlitScaled*(src: ptr SDL_Surface; srcrect: ptr SDL_Rect;
                         dst: ptr SDL_Surface; dstrect: ptr SDL_Rect): cint
proc SDL_SetYUVConversionMode*(mode: SDL_YUV_CONVERSION_MODE)
proc SDL_GetYUVConversionMode*(): SDL_YUV_CONVERSION_MODE
proc SDL_GetYUVConversionModeForResolution*(width: cint; height: cint): SDL_YUV_CONVERSION_MODE
proc SDL_GetNumVideoDrivers*(): cint
proc SDL_GetVideoDriver*(index: cint): cstring
proc SDL_VideoInit*(driver_name: cstring): cint
proc SDL_VideoQuit*()
proc SDL_GetCurrentVideoDriver*(): cstring
proc SDL_GetNumVideoDisplays*(): cint
proc SDL_GetDisplayName*(displayIndex: cint): cstring
proc SDL_GetDisplayBounds*(displayIndex: cint; rect: ptr SDL_Rect): cint
proc SDL_GetDisplayUsableBounds*(displayIndex: cint; rect: ptr SDL_Rect): cint
proc SDL_GetDisplayDPI*(displayIndex: cint; ddpi: ptr cfloat; hdpi: ptr cfloat;
                       vdpi: ptr cfloat): cint
proc SDL_GetDisplayOrientation*(displayIndex: cint): SDL_DisplayOrientation
proc SDL_GetNumDisplayModes*(displayIndex: cint): cint
proc SDL_GetDisplayMode*(displayIndex: cint; modeIndex: cint;
                        mode: ptr SDL_DisplayMode): cint
proc SDL_GetDesktopDisplayMode*(displayIndex: cint; mode: ptr SDL_DisplayMode): cint
proc SDL_GetCurrentDisplayMode*(displayIndex: cint; mode: ptr SDL_DisplayMode): cint
proc SDL_GetClosestDisplayMode*(displayIndex: cint; mode: ptr SDL_DisplayMode;
                               closest: ptr SDL_DisplayMode): ptr SDL_DisplayMode
proc SDL_GetWindowDisplayIndex*(window: ptr SDL_Window): cint
proc SDL_SetWindowDisplayMode*(window: ptr SDL_Window; mode: ptr SDL_DisplayMode): cint
proc SDL_GetWindowDisplayMode*(window: ptr SDL_Window; mode: ptr SDL_DisplayMode): cint
proc SDL_GetWindowPixelFormat*(window: ptr SDL_Window): uint32
proc SDL_CreateWindow*(title: cstring; x: cint; y: cint; w: cint; h: cint; flags: uint32): ptr SDL_Window
proc SDL_CreateWindowFrom*(data: pointer): ptr SDL_Window
proc SDL_GetWindowID*(window: ptr SDL_Window): uint32
proc SDL_GetWindowFromID*(id: uint32): ptr SDL_Window
proc SDL_GetWindowFlags*(window: ptr SDL_Window): uint32
proc SDL_SetWindowTitle*(window: ptr SDL_Window; title: cstring)
proc SDL_GetWindowTitle*(window: ptr SDL_Window): cstring
proc SDL_SetWindowIcon*(window: ptr SDL_Window; icon: ptr SDL_Surface)
proc SDL_SetWindowData*(window: ptr SDL_Window; name: cstring; userdata: pointer): pointer
proc SDL_GetWindowData*(window: ptr SDL_Window; name: cstring): pointer
proc SDL_SetWindowPosition*(window: ptr SDL_Window; x: cint; y: cint)
proc SDL_GetWindowPosition*(window: ptr SDL_Window; x: ptr cint; y: ptr cint)
proc SDL_SetWindowSize*(window: ptr SDL_Window; w: cint; h: cint)
proc SDL_GetWindowSize*(window: ptr SDL_Window; w: ptr cint; h: ptr cint)
proc SDL_GetWindowBordersSize*(window: ptr SDL_Window; top: ptr cint; left: ptr cint; bottom: ptr cint; right: ptr cint): cint {.discardable.}
proc SDL_SetWindowMinimumSize*(window: ptr SDL_Window; min_w: cint; min_h: cint)
proc SDL_GetWindowMinimumSize*(window: ptr SDL_Window; w: ptr cint; h: ptr cint)
proc SDL_SetWindowMaximumSize*(window: ptr SDL_Window; max_w: cint; max_h: cint)
proc SDL_GetWindowMaximumSize*(window: ptr SDL_Window; w: ptr cint; h: ptr cint)
proc SDL_SetWindowBordered*(window: ptr SDL_Window; bordered: SDL_bool)
proc SDL_SetWindowResizable*(window: ptr SDL_Window; resizable: SDL_bool)
proc SDL_ShowWindow*(window: ptr SDL_Window)
proc SDL_HideWindow*(window: ptr SDL_Window)
proc SDL_RaiseWindow*(window: ptr SDL_Window)
proc SDL_MaximizeWindow*(window: ptr SDL_Window)
proc SDL_MinimizeWindow*(window: ptr SDL_Window)
proc SDL_RestoreWindow*(window: ptr SDL_Window)
proc SDL_SetWindowFullscreen*(window: ptr SDL_Window; flags: uint32): cint {.discardable.}
proc SDL_GetWindowSurface*(window: ptr SDL_Window): ptr SDL_Surface
proc SDL_UpdateWindowSurface*(window: ptr SDL_Window): cint
proc SDL_UpdateWindowSurfaceRects*(window: ptr SDL_Window; rects: ptr SDL_Rect;
                                  numrects: cint): cint
proc SDL_SetWindowGrab*(window: ptr SDL_Window; grabbed: SDL_bool)
proc SDL_GetWindowGrab*(window: ptr SDL_Window): SDL_bool
proc SDL_GetGrabbedWindow*(): ptr SDL_Window
proc SDL_SetWindowBrightness*(window: ptr SDL_Window; brightness: cfloat): cint
proc SDL_GetWindowBrightness*(window: ptr SDL_Window): cfloat
proc SDL_SetWindowOpacity*(window: ptr SDL_Window; opacity: cfloat): cint
proc SDL_GetWindowOpacity*(window: ptr SDL_Window; out_opacity: ptr cfloat): cint
proc SDL_SetWindowModalFor*(modal_window: ptr SDL_Window;
                           parent_window: ptr SDL_Window): cint
proc SDL_SetWindowInputFocus*(window: ptr SDL_Window): cint
proc SDL_SetWindowGammaRamp*(window: ptr SDL_Window; red: ptr uint16;
                            green: ptr uint16; blue: ptr uint16): cint
proc SDL_GetWindowGammaRamp*(window: ptr SDL_Window; red: ptr uint16;
                            green: ptr uint16; blue: ptr uint16): cint
proc SDL_SetWindowHitTest*(window: ptr SDL_Window; callback: SDL_HitTest; callback_data: pointer): cint {.discardable.}
proc SDL_DestroyWindow*(window: ptr SDL_Window)
proc SDL_IsScreenSaverEnabled*(): SDL_bool
proc SDL_EnableScreenSaver*()
proc SDL_DisableScreenSaver*()
proc SDL_GL_LoadLibrary*(path: cstring): cint
proc SDL_GL_GetProcAddress*(`proc`: cstring): pointer
proc SDL_GL_UnloadLibrary*()
proc SDL_GL_ExtensionSupported*(extension: cstring): SDL_bool
proc SDL_GL_ResetAttributes*()
proc SDL_GL_SetAttribute*(attr: SDL_GLattr; value: cint): cint
proc SDL_GL_GetAttribute*(attr: SDL_GLattr; value: ptr cint): cint
proc SDL_GL_CreateContext*(window: ptr SDL_Window): SDL_GLContext
proc SDL_GL_MakeCurrent*(window: ptr SDL_Window; context: SDL_GLContext): cint
proc SDL_GL_GetCurrentWindow*(): ptr SDL_Window
proc SDL_GL_GetCurrentContext*(): SDL_GLContext
proc SDL_GL_GetDrawableSize*(window: ptr SDL_Window; w: ptr cint; h: ptr cint)
proc SDL_GL_SetSwapInterval*(interval: cint): cint
proc SDL_GL_GetSwapInterval*(): cint
proc SDL_GL_SwapWindow*(window: ptr SDL_Window)
proc SDL_GL_DeleteContext*(context: SDL_GLContext)
proc SDL_GetKeyboardFocus*(): ptr SDL_Window
proc SDL_GetKeyboardState*(numkeys: ptr cint): ptr uint8
proc SDL_GetModState*(): SDL_Keymod
proc SDL_SetModState*(modstate: SDL_Keymod)
proc SDL_GetKeyFromScancode*(scancode: SDL_ScanCode): SDL_KeyCode
proc SDL_GetScancodeFromKey*(key: SDL_KeyCode): SDL_ScanCode
proc SDL_GetScancodeName*(scancode: SDL_ScanCode): cstring
proc SDL_GetScancodeFromName*(name: cstring): SDL_ScanCode
proc SDL_GetKeyName*(key: SDL_KeyCode): cstring
proc SDL_GetKeyFromName*(name: cstring): SDL_KeyCode
proc SDL_StartTextInput*()
proc SDL_IsTextInputActive*(): SDL_bool
proc SDL_StopTextInput*()
proc SDL_SetTextInputRect*(rect: ptr SDL_Rect)
proc SDL_HasScreenKeyboardSupport*(): SDL_bool
proc SDL_IsScreenKeyboardShown*(window: ptr SDL_Window): SDL_bool
proc SDL_GetMouseFocus*(): ptr SDL_Window
proc SDL_GetMouseState*(x: ptr cint; y: ptr cint): uint32
proc SDL_GetGlobalMouseState*(x: ptr cint; y: ptr cint): uint32
proc SDL_GetRelativeMouseState*(x: ptr cint; y: ptr cint): uint32
proc SDL_WarpMouseInWindow*(window: ptr SDL_Window; x: cint; y: cint)
proc SDL_WarpMouseGlobal*(x: cint; y: cint): cint
proc SDL_SetRelativeMouseMode*(enabled: SDL_bool): cint
proc SDL_CaptureMouse*(enabled: SDL_bool): cint
proc SDL_GetRelativeMouseMode*(): SDL_bool
proc SDL_CreateCursor*(data: ptr uint8; mask: ptr uint8; w: cint; h: cint; hot_x: cint; hot_y: cint): ptr SDL_Cursor
proc SDL_CreateColorCursor*(surface: ptr SDL_Surface; hot_x: cint; hot_y: cint): ptr SDL_Cursor
proc SDL_CreateSystemCursor*(id: SDL_SystemCursor): ptr SDL_Cursor
proc SDL_SetCursor*(cursor: ptr SDL_Cursor)
proc SDL_GetCursor*(): ptr SDL_Cursor
proc SDL_GetDefaultCursor*(): ptr SDL_Cursor
proc SDL_FreeCursor*(cursor: ptr SDL_Cursor)
proc SDL_ShowCursor*(toggle: cint): cint {.discardable.}
proc SDL_LockJoysticks*()
proc SDL_UnlockJoysticks*()
proc SDL_NumJoysticks*(): cint
proc SDL_JoystickNameForIndex*(device_index: cint): cstring
proc SDL_JoystickGetDevicePlayerIndex*(device_index: cint): cint
proc SDL_JoystickGetDeviceGUID*(device_index: cint): SDL_JoystickGUID
proc SDL_JoystickGetDeviceVendor*(device_index: cint): uint16
proc SDL_JoystickGetDeviceProduct*(device_index: cint): uint16
proc SDL_JoystickGetDeviceProductVersion*(device_index: cint): uint16
proc SDL_JoystickGetDeviceType*(device_index: cint): SDL_JoystickType
proc SDL_JoystickGetDeviceInstanceID*(device_index: cint): int32
proc SDL_JoystickOpen*(device_index: cint): ptr SDL_Joystick
proc SDL_JoystickFromInstanceID*(instance_id: int32): ptr SDL_Joystick
proc SDL_JoystickFromPlayerIndex*(player_index: cint): ptr SDL_Joystick
proc SDL_JoystickName*(joystick: ptr SDL_Joystick): cstring
proc SDL_JoystickGetPlayerIndex*(joystick: ptr SDL_Joystick): cint
proc SDL_JoystickSetPlayerIndex*(joystick: ptr SDL_Joystick; player_index: cint)
proc SDL_JoystickGetGUID*(joystick: ptr SDL_Joystick): SDL_JoystickGUID
proc SDL_JoystickGetVendor*(joystick: ptr SDL_Joystick): uint16
proc SDL_JoystickGetProduct*(joystick: ptr SDL_Joystick): uint16
proc SDL_JoystickGetProductVersion*(joystick: ptr SDL_Joystick): uint16
proc SDL_JoystickGetType*(joystick: ptr SDL_Joystick): SDL_JoystickType
proc SDL_JoystickGetGUIDString*(guid: SDL_JoystickGUID; pszGUID: cstring; cbGUID: cint)
proc SDL_JoystickGetGUIDFromString*(pchGUID: cstring): SDL_JoystickGUID
proc SDL_JoystickGetAttached*(joystick: ptr SDL_Joystick): SDL_bool
proc SDL_JoystickInstanceID*(joystick: ptr SDL_Joystick): int32
proc SDL_JoystickNumAxes*(joystick: ptr SDL_Joystick): cint
proc SDL_JoystickNumBalls*(joystick: ptr SDL_Joystick): cint
proc SDL_JoystickNumHats*(joystick: ptr SDL_Joystick): cint
proc SDL_JoystickNumButtons*(joystick: ptr SDL_Joystick): cint
proc SDL_JoystickUpdate*()
proc SDL_JoystickEventState*(state: cint): cint
proc SDL_JoystickGetAxis*(joystick: ptr SDL_Joystick; axis: cint): int16
proc SDL_JoystickGetAxisInitialState*(joystick: ptr SDL_Joystick; axis: cint;
                                     state: ptr int16): SDL_bool
proc SDL_JoystickGetHat*(joystick: ptr SDL_Joystick; hat: cint): uint8
proc SDL_JoystickGetBall*(joystick: ptr SDL_Joystick; ball: cint; dx: ptr cint;
                         dy: ptr cint): cint
proc SDL_JoystickGetButton*(joystick: ptr SDL_Joystick; button: cint): uint8
proc SDL_JoystickRumble*(joystick: ptr SDL_Joystick; low_frequency_rumble: uint16;
                        high_frequency_rumble: uint16; duration_ms: uint32): cint
proc SDL_JoystickClose*(joystick: ptr SDL_Joystick)
proc SDL_JoystickCurrentPowerLevel*(joystick: ptr SDL_Joystick): SDL_JoystickPowerLevel
proc SDL_GameControllerAddMappingsFromRW*(rw: ptr SDL_RWops; freerw: cint): cint
proc SDL_GameControllerAddMapping*(mappingString: cstring): cint
proc SDL_GameControllerNumMappings*(): cint
proc SDL_GameControllerMappingForIndex*(mapping_index: cint): cstring
proc SDL_GameControllerMappingForGUID*(guid: SDL_JoystickGUID): cstring
proc SDL_GameControllerMapping*(gamecontroller: ptr SDL_GameController): cstring
proc SDL_IsGameController*(joystick_index: cint): SDL_bool
proc SDL_GameControllerNameForIndex*(joystick_index: cint): cstring
proc SDL_GameControllerTypeForIndex*(joystick_index: cint): SDL_GameControllerType
proc SDL_GameControllerMappingForDeviceIndex*(joystick_index: cint): cstring
proc SDL_GameControllerOpen*(joystick_index: cint): ptr SDL_GameController
proc SDL_GameControllerFromInstanceID*(joyid: int32): ptr SDL_GameController
proc SDL_GameControllerFromPlayerIndex*(player_index: cint): ptr SDL_GameController
proc SDL_GameControllerName*(gamecontroller: ptr SDL_GameController): cstring
proc SDL_GameControllerGetType*(gamecontroller: ptr SDL_GameController): SDL_GameControllerType
proc SDL_GameControllerGetPlayerIndex*(gamecontroller: ptr SDL_GameController): cint
proc SDL_GameControllerSetPlayerIndex*(gamecontroller: ptr SDL_GameController;
                                      player_index: cint)
proc SDL_GameControllerGetVendor*(gamecontroller: ptr SDL_GameController): uint16
proc SDL_GameControllerGetProduct*(gamecontroller: ptr SDL_GameController): uint16
proc SDL_GameControllerGetProductVersion*(gamecontroller: ptr SDL_GameController): uint16
proc SDL_GameControllerGetAttached*(gamecontroller: ptr SDL_GameController): SDL_bool
proc SDL_GameControllerGetJoystick*(gamecontroller: ptr SDL_GameController): ptr SDL_Joystick
proc SDL_GameControllerEventState*(state: cint): cint
proc SDL_GameControllerUpdate*()


proc SDL_GameControllerGetAxisFromString*(pchString: cstring): SDL_GameControllerAxis
proc SDL_GameControllerGetStringForAxis*(axis: SDL_GameControllerAxis): cstring
proc SDL_GameControllerGetBindForAxis*(gamecontroller: ptr SDL_GameController;
                                      axis: SDL_GameControllerAxis): SDL_GameControllerButtonBind
proc SDL_GameControllerGetAxis*(gamecontroller: ptr SDL_GameController;
                               axis: SDL_GameControllerAxis): int16



proc SDL_GameControllerGetButtonFromString*(pchString: cstring): SDL_GameControllerButton
proc SDL_GameControllerGetStringForButton*(button: SDL_GameControllerButton): cstring
proc SDL_GameControllerGetBindForButton*(gamecontroller: ptr SDL_GameController;
                                        button: SDL_GameControllerButton): SDL_GameControllerButtonBind
proc SDL_GameControllerGetButton*(gamecontroller: ptr SDL_GameController;
                                 button: SDL_GameControllerButton): uint8
proc SDL_GameControllerRumble*(gamecontroller: ptr SDL_GameController;
                              low_frequency_rumble: uint16;
                              high_frequency_rumble: uint16; duration_ms: uint32): cint
proc SDL_GameControllerClose*(gamecontroller: ptr SDL_GameController)


proc SDL_GetNumTouchDevices*(): cint
proc SDL_GetTouchDevice*(index: cint): SDL_TouchID
proc SDL_GetTouchDeviceType*(touchID: SDL_TouchID): SDL_TouchDeviceType
proc SDL_GetNumTouchFingers*(touchID: SDL_TouchID): cint
proc SDL_GetTouchFinger*(touchID: SDL_TouchID; index: cint): ptr SDL_Finger


proc SDL_RecordGesture*(touchId: SDL_TouchID): cint
proc SDL_SaveAllDollarTemplates*(dst: ptr SDL_RWops): cint
proc SDL_SaveDollarTemplate*(gestureId: SDL_GestureID; dst: ptr SDL_RWops): cint
proc SDL_LoadDollarTemplates*(touchId: SDL_TouchID; src: ptr SDL_RWops): cint


proc SDL_PumpEvents*()

proc SDL_PeepEvents*(events: ptr SDL_Event; numevents: cint; action: SDL_eventaction;
                    minType: uint32; maxType: uint32): cint
proc SDL_HasEvent*(`type`: uint32): SDL_bool
proc SDL_HasEvents*(minType: uint32; maxType: uint32): SDL_bool
proc SDL_FlushEvent*(`type`: uint32)
proc SDL_FlushEvents*(minType: uint32; maxType: uint32)
proc SDL_PollEvent*(event: ptr SDL_Event): cint
proc SDL_WaitEvent*(event: ptr SDL_Event): cint
proc SDL_WaitEventTimeout*(event: ptr SDL_Event; timeout: cint): cint
proc SDL_PushEvent*(event: ptr SDL_Event): cint {.discardable.}
proc SDL_SetEventFilter*(filter: SDL_EventFilter; userdata: pointer)
proc SDL_GetEventFilter*(filter: ptr SDL_EventFilter; userdata: ptr pointer): SDL_bool
proc SDL_AddEventWatch*(filter: SDL_EventFilter; userdata: pointer)
proc SDL_DelEventWatch*(filter: SDL_EventFilter; userdata: pointer)
proc SDL_FilterEvents*(filter: SDL_EventFilter; userdata: pointer)
proc SDL_EventState*(`type`: uint32; state: cint): uint8
proc SDL_RegisterEvents*(numevents: cint): uint32
proc SDL_GetBasePath*(): cstring
proc SDL_GetPrefPath*(org: cstring; app: cstring): cstring
proc SDL_NumHaptics*(): cint
proc SDL_HapticName*(device_index: cint): cstring
proc SDL_HapticOpen*(device_index: cint): ptr SDL_Haptic
proc SDL_HapticOpened*(device_index: cint): cint
proc SDL_HapticIndex*(haptic: ptr SDL_Haptic): cint
proc SDL_MouseIsHaptic*(): cint
proc SDL_HapticOpenFromMouse*(): ptr SDL_Haptic
proc SDL_JoystickIsHaptic*(joystick: ptr SDL_Joystick): cint
proc SDL_HapticOpenFromJoystick*(joystick: ptr SDL_Joystick): ptr SDL_Haptic
proc SDL_HapticClose*(haptic: ptr SDL_Haptic)
proc SDL_HapticNumEffects*(haptic: ptr SDL_Haptic): cint
proc SDL_HapticNumEffectsPlaying*(haptic: ptr SDL_Haptic): cint
proc SDL_HapticQuery*(haptic: ptr SDL_Haptic): cuint
proc SDL_HapticNumAxes*(haptic: ptr SDL_Haptic): cint
proc SDL_HapticEffectSupported*(haptic: ptr SDL_Haptic; effect: ptr SDL_HapticEffect): cint
proc SDL_HapticNewEffect*(haptic: ptr SDL_Haptic; effect: ptr SDL_HapticEffect): cint
proc SDL_HapticUpdateEffect*(haptic: ptr SDL_Haptic; effect: cint;
                            data: ptr SDL_HapticEffect): cint
proc SDL_HapticRunEffect*(haptic: ptr SDL_Haptic; effect: cint; iterations: uint32): cint
proc SDL_HapticStopEffect*(haptic: ptr SDL_Haptic; effect: cint): cint
proc SDL_HapticDestroyEffect*(haptic: ptr SDL_Haptic; effect: cint)
proc SDL_HapticGetEffectStatus*(haptic: ptr SDL_Haptic; effect: cint): cint
proc SDL_HapticSetGain*(haptic: ptr SDL_Haptic; gain: cint): cint
proc SDL_HapticSetAutocenter*(haptic: ptr SDL_Haptic; autocenter: cint): cint
proc SDL_HapticPause*(haptic: ptr SDL_Haptic): cint
proc SDL_HapticUnpause*(haptic: ptr SDL_Haptic): cint
proc SDL_HapticStopAll*(haptic: ptr SDL_Haptic): cint
proc SDL_HapticRumbleSupported*(haptic: ptr SDL_Haptic): cint
proc SDL_HapticRumbleInit*(haptic: ptr SDL_Haptic): cint
proc SDL_HapticRumblePlay*(haptic: ptr SDL_Haptic; strength: cfloat; length: uint32): cint
proc SDL_HapticRumbleStop*(haptic: ptr SDL_Haptic): cint
proc SDL_SetHintWithPriority*(name: cstring; value: cstring;
                             priority: SDL_HintPriority): SDL_bool
proc SDL_SetHint*(name: cstring; value: cstring): SDL_bool
proc SDL_GetHint*(name: cstring): cstring
proc SDL_GetHintBoolean*(name: cstring; default_value: SDL_bool): SDL_bool


proc SDL_AddHintCallback*(name: cstring; callback: SDL_HintCallback;
                         userdata: pointer)
proc SDL_DelHintCallback*(name: cstring; callback: SDL_HintCallback;
                         userdata: pointer)
proc SDL_ClearHints*()
proc SDL_LoadObject*(sofile: cstring): pointer
proc SDL_LoadFunction*(handle: pointer; name: cstring): pointer
proc SDL_UnloadObject*(handle: pointer)
proc SDL_LogSetAllPriority*(priority: SDL_LogPriority)
proc SDL_LogSetPriority*(category: cint; priority: SDL_LogPriority)
proc SDL_LogGetPriority*(category: cint): SDL_LogPriority
proc SDL_LogResetPriorities*()
proc SDL_Log*(fmt: cstring) {.varargs.}
proc SDL_LogVerbose*(category: cint; fmt: cstring) {.varargs.}
proc SDL_LogDebug*(category: cint; fmt: cstring) {.varargs.}
proc SDL_LogInfo*(category: cint; fmt: cstring) {.varargs.}
proc SDL_LogWarn*(category: cint; fmt: cstring) {.varargs.}
proc SDL_LogError*(category: cint; fmt: cstring) {.varargs.}
proc SDL_LogCritical*(category: cint; fmt: cstring) {.varargs.}
proc SDL_LogMessage*(category: cint; priority: SDL_LogPriority; fmt: cstring) {.varargs.}
proc SDL_LogMessageV*(category: cint; priority: SDL_LogPriority; fmt: cstring; ap: varargs[untyped])
proc SDL_LogGetOutputFunction*(callback: ptr SDL_LogOutputFunction; userdata: ptr pointer)
proc SDL_LogSetOutputFunction*(callback: SDL_LogOutputFunction; userdata: pointer)

proc SDL_ShowMessageBox*(messageboxdata: ptr SDL_MessageBoxData; buttonid: ptr cint): cint
proc SDL_ShowSimpleMessageBox*(flags: uint32; title: cstring; message: cstring; window: ptr SDL_Window): cint
proc SDL_Metal_CreateView*(window: ptr SDL_Window): SDL_MetalView
proc SDL_Metal_DestroyView*(view: SDL_MetalView)
proc SDL_GetPowerInfo*(secs: ptr cint; pct: ptr cint): SDL_PowerState

proc SDL_GetNumRenderDrivers*(): cint
proc SDL_GetRenderDriverInfo*(index: cint; info: ptr SDL_RendererInfo): cint
proc SDL_CreateWindowAndRenderer*(width: cint; height: cint; window_flags: uint32;window: ptr ptr SDL_Window;renderer: ptr ptr SDL_Renderer): cint
proc SDL_CreateRenderer*(window: ptr SDL_Window; index: cint; flags: uint32): ptr SDL_Renderer
proc SDL_CreateSoftwareRenderer*(surface: ptr SDL_Surface): ptr SDL_Renderer
proc SDL_GetRenderer*(window: ptr SDL_Window): ptr SDL_Renderer
proc SDL_GetRendererInfo*(renderer: ptr SDL_Renderer; info: ptr SDL_RendererInfo): cint
proc SDL_GetRendererOutputSize*(renderer: ptr SDL_Renderer; w: ptr cint; h: ptr cint): cint
proc SDL_CreateTexture*(renderer: ptr SDL_Renderer; format: auto; access: auto; w: auto; h: auto): ptr SDL_Texture
proc SDL_CreateTextureFromSurface*(renderer: ptr SDL_Renderer;surface: ptr SDL_Surface): ptr SDL_Texture
proc SDL_QueryTexture*(texture: ptr SDL_Texture; format: ptr uint32; access: ptr cint; w: ptr cint; h: ptr cint): cint
proc SDL_SetTextureColorMod*(texture: ptr SDL_Texture; r: uint8; g: uint8; b: uint8): cint
proc SDL_GetTextureColorMod*(texture: ptr SDL_Texture; r: ptr uint8; g: ptr uint8;b: ptr uint8): cint
proc SDL_SetTextureAlphaMod*(texture: ptr SDL_Texture; alpha: uint8): cint
proc SDL_GetTextureAlphaMod*(texture: ptr SDL_Texture; alpha: ptr uint8): cint
proc SDL_SetTextureBlendMode*(texture: ptr SDL_Texture; blendMode: SDL_BlendMode): cint
proc SDL_GetTextureBlendMode*(texture: ptr SDL_Texture; blendMode: ptr SDL_BlendMode): cint
proc SDL_SetTextureScaleMode*(texture: ptr SDL_Texture; scaleMode: SDL_ScaleMode): cint
proc SDL_GetTextureScaleMode*(texture: ptr SDL_Texture; scaleMode: ptr SDL_ScaleMode): cint
proc SDL_UpdateTexture*(texture: ptr SDL_Texture; rect: ptr SDL_Rect; pixels: pointer; pitch: cint): cint
proc SDL_UpdateYUVTexture*(texture: ptr SDL_Texture; rect: ptr SDL_Rect;
                          Yplane: ptr uint8; Ypitch: cint; Uplane: ptr uint8;
                          Upitch: cint; Vplane: ptr uint8; Vpitch: cint): cint
proc SDL_LockTexture*(texture: ptr SDL_Texture; rect: ptr SDL_Rect;
                     pixels: ptr pointer; pitch: ptr cint): cint {.discardable.}
proc SDL_LockTextureToSurface*(texture: ptr SDL_Texture; rect: ptr SDL_Rect;
                              surface: ptr ptr SDL_Surface): cint {.discardable.}
proc SDL_UnlockTexture*(texture: ptr SDL_Texture)
proc SDL_RenderTargetSupported*(renderer: ptr SDL_Renderer): SDL_bool
proc SDL_SetRenderTarget*(renderer: ptr SDL_Renderer; texture: ptr SDL_Texture): cint {.discardable.}
proc SDL_GetRenderTarget*(renderer: ptr SDL_Renderer): ptr SDL_Texture
proc SDL_RenderSetLogicalSize*(renderer: ptr SDL_Renderer; w: cint; h: cint): cint {.discardable.}
proc SDL_RenderGetLogicalSize*(renderer: ptr SDL_Renderer; w: ptr cint; h: ptr cint)
proc SDL_RenderSetIntegerScale*(renderer: ptr SDL_Renderer; enable: SDL_bool): cint {.discardable.}
proc SDL_RenderGetIntegerScale*(renderer: ptr SDL_Renderer): SDL_bool
proc SDL_RenderSetViewport*(renderer: ptr SDL_Renderer; rect: ptr SDL_Rect): cint {.discardable.}
proc SDL_RenderGetViewport*(renderer: ptr SDL_Renderer; rect: ptr SDL_Rect)
proc SDL_RenderSetClipRect*(renderer: ptr SDL_Renderer; rect: ptr SDL_Rect): cint {.discardable.}
proc SDL_RenderGetClipRect*(renderer: ptr SDL_Renderer; rect: ptr SDL_Rect)
proc SDL_RenderIsClipEnabled*(renderer: ptr SDL_Renderer): SDL_bool
proc SDL_RenderSetScale*(renderer: ptr SDL_Renderer; scaleX: cfloat; scaleY: cfloat): cint {.discardable.}
proc SDL_RenderGetScale*(renderer: ptr SDL_Renderer; scaleX: ptr cfloat; scaleY: ptr cfloat)
proc SDL_SetRenderDrawColor*(renderer: ptr SDL_Renderer; r: uint8; g: uint8; b: uint8; a: uint8): cint {.discardable.}
proc SDL_GetRenderDrawColor*(renderer: ptr SDL_Renderer; r: ptr uint8; g: ptr uint8; b: ptr uint8; a: ptr uint8): cint {.discardable.}
proc SDL_SetRenderDrawBlendMode*(renderer: ptr SDL_Renderer; blendMode: SDL_BlendMode): cint {.discardable.}
proc SDL_GetRenderDrawBlendMode*(renderer: ptr SDL_Renderer; blendMode: ptr SDL_BlendMode): cint {.discardable.}
proc SDL_RenderClear*(renderer: ptr SDL_Renderer): cint {.discardable.}
proc SDL_RenderDrawPoint*(renderer: ptr SDL_Renderer; x: cint; y: cint): cint {.discardable.}
proc SDL_RenderDrawPoints*(renderer: ptr SDL_Renderer; points: ptr SDL_Point; count: cint): cint {.discardable.}
proc SDL_RenderDrawLine*(renderer: ptr SDL_Renderer; x1: cint; y1: cint; x2: cint; y2: cint): cint {.discardable.}
proc SDL_RenderDrawLines*(renderer: ptr SDL_Renderer; points: ptr SDL_Point; count: cint): cint {.discardable.}
proc SDL_RenderDrawRect*(renderer: ptr SDL_Renderer; rect: ptr SDL_Rect): cint {.discardable.}
proc SDL_RenderDrawRects*(renderer: ptr SDL_Renderer; rects: ptr SDL_Rect; count: cint): cint {.discardable.}
proc SDL_RenderFillRect*(renderer: ptr SDL_Renderer; rect: ptr SDL_Rect): cint {.discardable.}
proc SDL_RenderFillRects*(renderer: ptr SDL_Renderer; rects: ptr SDL_Rect; count: cint): cint {.discardable.}
proc SDL_RenderCopy*(renderer: ptr SDL_Renderer; texture: ptr SDL_Texture; srcrect: ptr SDL_Rect; dstrect: ptr SDL_Rect): cint {.discardable.}
proc SDL_RenderCopyEx*(renderer: ptr SDL_Renderer; texture: ptr SDL_Texture;srcrect: ptr SDL_Rect; dstrect: ptr SDL_Rect; angle: cdouble; center: ptr SDL_Point; flip: SDL_RendererFlip): cint  {.discardable.}
proc SDL_RenderDrawPointF*(renderer: ptr SDL_Renderer; x: cfloat; y: cfloat): cint {.discardable.}
proc SDL_RenderDrawPointsF*(renderer: ptr SDL_Renderer; points: ptr SDL_FPoint;count: cint): cint {.discardable.}
proc SDL_RenderDrawLineF*(renderer: ptr SDL_Renderer; x1: cfloat; y1: cfloat; x2: cfloat; y2: cfloat): cint {.discardable.}
proc SDL_RenderDrawLinesF*(renderer: ptr SDL_Renderer; points: ptr SDL_FPoint; count: cint): cint {.discardable.}
proc SDL_RenderDrawRectF*(renderer: ptr SDL_Renderer; rect: ptr SDL_FRect): cint {.discardable.}
proc SDL_RenderDrawRectsF*(renderer: ptr SDL_Renderer; rects: ptr SDL_FRect;count: cint): cint {.discardable.}
proc SDL_RenderFillRectF*(renderer: ptr SDL_Renderer; rect: ptr SDL_FRect): cint {.discardable.}
proc SDL_RenderFillRectsF*(renderer: ptr SDL_Renderer; rects: ptr SDL_FRect;count: cint): cint {.discardable.}
proc SDL_RenderCopyF*(renderer: ptr SDL_Renderer; texture: ptr SDL_Texture;srcrect: ptr SDL_Rect; dstrect: ptr SDL_FRect): cint {.discardable.}
proc SDL_RenderCopyExF*(renderer: ptr SDL_Renderer; texture: ptr SDL_Texture;srcrect: ptr SDL_Rect; dstrect: ptr SDL_FRect; angle: cdouble;center: ptr SDL_FPoint; flip: SDL_RendererFlip): cint {.discardable.}
proc SDL_RenderReadPixels*(renderer: ptr SDL_Renderer; rect: ptr SDL_Rect; format: uint32; pixels: pointer; pitch: cint): cint {.discardable.}
proc SDL_RenderPresent*(renderer: ptr SDL_Renderer)
proc SDL_DestroyTexture*(texture: ptr SDL_Texture)
proc SDL_DestroyRenderer*(renderer: ptr SDL_Renderer)
proc SDL_RenderFlush*(renderer: ptr SDL_Renderer): cint
proc SDL_GL_BindTexture*(texture: ptr SDL_Texture; texw: ptr cfloat; texh: ptr cfloat): cint
proc SDL_GL_UnbindTexture*(texture: ptr SDL_Texture): cint
proc SDL_RenderGetMetalLayer*(renderer: ptr SDL_Renderer): pointer
proc SDL_RenderGetMetalCommandEncoder*(renderer: ptr SDL_Renderer): pointer

proc SDL_SensorGetDeviceName*(device_index: cint): cstring
proc SDL_SensorGetDeviceType*(device_index: cint): SDL_SensorType
proc SDL_SensorGetDeviceNonPortableType*(device_index: cint): cint
proc SDL_SensorGetDeviceInstanceID*(device_index: cint): SDL_SensorID
proc SDL_SensorOpen*(device_index: cint): ptr SDL_Sensor
proc SDL_SensorFromInstanceID*(instance_id: SDL_SensorID): ptr SDL_Sensor
proc SDL_SensorGetName*(sensor: ptr SDL_Sensor): cstring
proc SDL_SensorGetType*(sensor: ptr SDL_Sensor): SDL_SensorType
proc SDL_SensorGetNonPortableType*(sensor: ptr SDL_Sensor): cint
proc SDL_SensorGetInstanceID*(sensor: ptr SDL_Sensor): SDL_SensorID
proc SDL_SensorGetData*(sensor: ptr SDL_Sensor; data: ptr cfloat; num_values: cint): cint
proc SDL_SensorClose*(sensor: ptr SDL_Sensor)
proc SDL_SensorUpdate*()
proc SDL_CreateShapedWindow*(title: cstring; x: cuint; y: cuint; w: cuint; h: cuint;
                            flags: uint32): ptr SDL_Window
proc SDL_IsShapedWindow*(window: ptr SDL_Window): SDL_bool
proc SDL_SetWindowShape*(window: ptr SDL_Window; shape: ptr SDL_Surface;
                        shape_mode: ptr SDL_WindowShapeMode): cint
proc SDL_GetShapedWindowMode*(window: ptr SDL_Window;
                             shape_mode: ptr SDL_WindowShapeMode): cint

proc SDL_SetWindowsMessageHook*(callback: SDL_WindowsMessageHook; userdata: pointer)
proc SDL_Direct3D9GetAdapterIndex*(displayIndex: cint): cint

proc SDL_RenderGetD3D9Device*(renderer: ptr SDL_Renderer): ptr IDirect3DDevice9
proc SDL_DXGIGetOutputInfo*(displayIndex: cint; adapterIndex: ptr cint; outputIndex: ptr cint): SDL_bool
proc SDL_IsTablet*(): SDL_bool
proc SDL_OnApplicationWillTerminate*()
proc SDL_OnApplicationDidReceiveMemoryWarning*()
proc SDL_OnApplicationWillResignActive*()
proc SDL_OnApplicationDidEnterBackground*()
proc SDL_OnApplicationWillEnterForeground*()
proc SDL_OnApplicationDidBecomeActive*()
proc SDL_GetTicks*(): uint32
proc SDL_GetPerformanceCounter*(): uint64
proc SDL_GetPerformanceFrequency*(): uint64
proc SDL_Delay*(ms: uint32)

proc SDL_AddTimer*(interval: uint32; callback: SDL_TimerCallback; param: pointer): SDL_TimerID
proc SDL_RemoveTimer*(id: SDL_TimerID): SDL_bool
proc SDL_GetVersion*(ver: ptr SDL_version)
proc SDL_GetRevision*(): cstring
proc SDL_GetRevisionNumber*(): cint
proc SDL_Init*(flags: uint32): cint
proc SDL_InitSubSystem*(flags: uint32): cint
proc SDL_QuitSubSystem*(flags: uint32)
proc SDL_WasInit*(flags: uint32): uint32
proc SDL_Quit*()
{.pop.}

proc SDL_memcpy4*(dst: pointer; src: pointer; dwords: csize_t): pointer {.inline.} =
  return SDL_memcpy(dst, src, dwords * 4)

##  #define SDL_VERSION(x)                  \
##      {                                   \
##          (x).major = SDL_MAJOR_VERSION; \
##          (x).minor = SDL_MINOR_VERSION; \
##          (x).patch = SDL_PATCHLEVEL;    \
##      }





var avcodec_opts*: array[AVMEDIA_TYPE_NB.int, ptr AVCodecContext]

var avformat_opts*: ptr AVFormatContext

var sws_dict*: ptr AVDictionary

var swr_opts*: ptr AVDictionary

var
  format_opts*: ptr AVDictionary
  codec_opts*: ptr AVDictionary
  resample_opts*: ptr AVDictionary

var hide_banner*: cint

proc register_exit*(cb: proc (result: cint))
proc exit_program*(result: cint)
proc init_dynload*()

proc init_opts*() =
  av_dict_set(addr(sws_dict), "flags", "bicubic", 0)


proc uninit_opts*()
proc log_callback_help*(`ptr`: pointer; level: cint; fmt: cstring; vl: varargs[untyped])
proc opt_cpuflags*(optctx: pointer; opt: cstring; arg: cstring): cint
proc opt_default*(optctx: pointer; opt: cstring; arg: cstring): cint
proc opt_loglevel*(optctx: pointer; opt: cstring; arg: cstring): cint
proc opt_report*(optctx: pointer; opt: cstring; arg: cstring): cint
proc opt_max_alloc*(optctx: pointer; opt: cstring; arg: cstring): cint
proc opt_codec_debug*(optctx: pointer; opt: cstring; arg: cstring): cint
proc opt_timelimit*(optctx: pointer; opt: cstring; arg: cstring): cint
proc parse_number_or_die*(context: cstring; numstr: cstring; `type`: cint;min: cdouble; max: cdouble): cdouble
proc parse_time_or_die*(context: cstring; timestr: cstring; is_duration: cint): int64
proc show_help_options*(options: ptr OptionDef; msg: cstring; req_flags: cint;rej_flags: cint; alt_flags: cint)
proc show_banner*(argc: cint; argv: cstringArray; options: ptr OptionDef)
proc show_version*(optctx: pointer; opt: cstring; arg: cstring): cint
proc show_buildconf*(optctx: pointer; opt: cstring; arg: cstring): cint
proc show_license*(optctx: pointer; opt: cstring; arg: cstring): cint
proc show_formats*(optctx: pointer; opt: cstring; arg: cstring): cint
proc show_muxers*(optctx: pointer; opt: cstring; arg: cstring): cint
proc show_demuxers*(optctx: pointer; opt: cstring; arg: cstring): cint
proc show_devices*(optctx: pointer; opt: cstring; arg: cstring): cint
proc show_sinks*(optctx: pointer; opt: cstring; arg: cstring): cint
proc show_sources*(optctx: pointer; opt: cstring; arg: cstring): cint
proc show_codecs*(optctx: pointer; opt: cstring; arg: cstring): cint
proc show_decoders*(optctx: pointer; opt: cstring; arg: cstring): cint
proc show_encoders*(optctx: pointer; opt: cstring; arg: cstring): cint
proc show_filters*(optctx: pointer; opt: cstring; arg: cstring): cint
proc show_bsfs*(optctx: pointer; opt: cstring; arg: cstring): cint
proc show_protocols*(optctx: pointer; opt: cstring; arg: cstring): cint
proc show_pix_fmts*(optctx: pointer; opt: cstring; arg: cstring): cint
proc show_layouts*(optctx: pointer; opt: cstring; arg: cstring): cint
proc show_sample_fmts*(optctx: pointer; opt: cstring; arg: cstring): cint
proc show_colors*(optctx: pointer; opt: cstring; arg: cstring): cint
# proc show_help*(optctx: pointer; opt: cstring; arg: cstring): cint
# proc read_yesno*(): cint

# var CMDUTILS_COMMON_OPTIONS_AVDEVICE:seq[OptionDef] = @[
#     OptionDef(name:"sources"    , flags:OPT_EXIT or HAS_ARG, u:OptionDefUnion(func_arg: show_sources),  help:"list sources of the input device", argname:"device"),                                                                   
#     OptionDef(name:"sinks", flags:OPT_EXIT or HAS_ARG, u:OptionDefUnion(func_arg:show_sinks),    help:"list sinks of the output device", argname:"device")]

# var CMDUTILS_COMMON_OPTIONS: seq[OptionDef] = @[
#     OptionDef(name:"L",           flags:OPT_EXIT,             u:OptionDefUnion(func_arg : show_license ),    help:"show license" ),                          
#     OptionDef(name:"h",           flags:OPT_EXIT,             u:OptionDefUnion(func_arg : show_help ),       help:"show help", argname:"topic" ),                    
#     OptionDef(name:"?",           flags:OPT_EXIT,             u:OptionDefUnion(func_arg : show_help ),       help:"show help", argname:"topic" ),                    
#     OptionDef(name:"help",        flags:OPT_EXIT,             u:OptionDefUnion(func_arg : show_help ),       help:"show help", argname:"topic" ),                    
#     OptionDef(name:"-help",       flags:OPT_EXIT,             u:OptionDefUnion(func_arg : show_help ),       help:"show help", argname:"topic" ),                    
#     OptionDef(name:"version",     flags:OPT_EXIT,             u:OptionDefUnion(func_arg : show_version ),    help:"show version" ),                          
#     OptionDef(name:"buildconf",   flags:OPT_EXIT,             u:OptionDefUnion(func_arg : show_buildconf ),  help:"show build configuration" ),              
#     OptionDef(name:"formats",     flags:OPT_EXIT,             u:OptionDefUnion(func_arg : show_formats ),    help:"show available formats" ),                
#     OptionDef(name:"muxers",      flags:OPT_EXIT,             u:OptionDefUnion(func_arg : show_muxers ),     help:"show available muxers" ),                 
#     OptionDef(name:"demuxers",    flags:OPT_EXIT,             u:OptionDefUnion(func_arg : show_demuxers ),   help:"show available demuxers" ),               
#     OptionDef(name:"devices",     flags:OPT_EXIT,             u:OptionDefUnion(func_arg : show_devices ),    help:"show available devices" ),                
#     OptionDef(name:"codecs",      flags:OPT_EXIT,             u:OptionDefUnion(func_arg : show_codecs ),     help:"show available codecs" ),                 
#     OptionDef(name:"decoders",    flags:OPT_EXIT,             u:OptionDefUnion(func_arg : show_decoders ),   help:"show available decoders" ),               
#     OptionDef(name:"encoders",    flags:OPT_EXIT,             u:OptionDefUnion(func_arg : show_encoders ),   help:"show available encoders" ),               
#     OptionDef(name:"bsfs",        flags:OPT_EXIT,             u:OptionDefUnion(func_arg : show_bsfs ),       help:"show available bit stream filters" ),     
#     OptionDef(name:"protocols",   flags:OPT_EXIT,             u:OptionDefUnion(func_arg : show_protocols ),  help:"show available protocols" ),              
#     OptionDef(name:"filters",     flags:OPT_EXIT,             u:OptionDefUnion(func_arg : show_filters ),    help:"show available filters" ),                
#     OptionDef(name:"pix_fmts",    flags:OPT_EXIT,             u:OptionDefUnion(func_arg : show_pix_fmts ),   help:"show available pixel formats" ),          
#     OptionDef(name:"layouts",     flags:OPT_EXIT,             u:OptionDefUnion(func_arg : show_layouts ),    help:"show standard channel layouts" ),         
#     OptionDef(name:"sample_fmts", flags:OPT_EXIT,             u:OptionDefUnion(func_arg : show_sample_fmts ),help:"show available audio sample formats" ),   
#     OptionDef(name:"colors",      flags:OPT_EXIT,             u:OptionDefUnion(func_arg : show_colors ),     help:"show available color names" ),            
#     OptionDef(name:"loglevel",    flags:HAS_ARG,              u:OptionDefUnion(func_arg : opt_loglevel ),    help:"set logging level", argname:"loglevel" ),         
#     OptionDef(name:"v",           flags:HAS_ARG,              u:OptionDefUnion(func_arg : opt_loglevel ),    help:"set logging level", argname:"loglevel" ),         
#     OptionDef(name:"report",      flags:0,                    u:OptionDefUnion(func_arg : opt_report ),      help:"generate a report" ),                     
#     OptionDef(name:"max_alloc",   flags:HAS_ARG,              u:OptionDefUnion(func_arg : opt_max_alloc ),   help:"set maximum size of a single allocated block", argname:"bytes" ), 
#     OptionDef(name:"cpuflags",    flags:HAS_ARG or OPT_EXPERT, u:OptionDefUnion(func_arg : opt_cpuflags ),     help:"force specific cpu flags", argname:"flags" ),     
#     OptionDef(name:"hide_banner", flags:OPT_BOOL or OPT_EXPERT, u:OptionDefUnion(dst_ptr:addr hide_banner),     help:"do not show program banner", argname:"hide_banner" ),          
# ]  & CMDUTILS_COMMON_OPTIONS_AVDEVICE

proc show_help_children*(class: ptr AVClass; flags: cint)
proc parse_options*(optctx: pointer; argc: cint; argv: cstringArray; options: ptr OptionDef; parse_arg_function: proc (optctx: pointer; a2: cstring))
proc parse_option*(optctx: pointer; opt: cstring; arg: cstring; options: ptr OptionDef): cint



proc parse_optgroup*(optctx: pointer; g: ptr OptionGroup): cint
proc split_commandline*(octx: ptr OptionParseContext; argc: cint; argv: ptr cstring;
                       options: ptr OptionDef; groups: ptr OptionGroupDef;nb_groups: cint): cint
proc uninit_parse_context*(octx: ptr OptionParseContext)
proc parse_loglevel*(argc: cint; argv: cstringArray; options: ptr OptionDef)
proc locate_option*(argc: cint; argv: cstringArray; options: ptr OptionDef;optname: cstring): cint
proc check_stream_specifier*(s: ptr AVFormatContext; st: ptr AVStream; spec: cstring): cint
proc filter_codec_opts*(opts: ptr AVDictionary; codec_id: AVCodecID;s: ptr AVFormatContext; st: ptr AVStream; codec: ptr AVCodec): ptr AVDictionary
proc setup_find_stream_info_opts*(s: ptr AVFormatContext;codec_opts: ptr AVDictionary): ptr ptr AVDictionary
proc print_error*(filename: cstring; err: cint)


proc get_preset_file*(filename: cstring; filename_size: csize_t;preset_name: cstring; is_path: cint; codec_name: cstring): ptr FILE
proc grow_array*(array: pointer; elem_size: cint; size: ptr cint; new_size: cint): pointer
##  #define GET_SAMPLE_FMT_NAME(sample_fmt) const char *name = av_get_sample_fmt_name(sample_fmt)
##  #define GET_SAMPLE_RATE_NAME(rate) \
##      char name[16];                 \
##      snprintf(name, sizeof(name), "%d", rate);
##  #define GET_CH_LAYOUT_NAME(ch_layout) \
##      char name[16];                    \
##      snprintf(name, sizeof(name), "0x%" PRIx64, ch_layout);
##  #define GET_CH_LAYOUT_DESC(ch_layout) \
##      char name[128];                   \
##      av_get_channel_layout_string(name, sizeof(name), 0, ch_layout);

var program_name*: string = "ffplay"

var program_birth_year*: cint = 2003

var sws_flags*: cuint = 4
const
  AV_SYNC_AUDIO_MASTER* = 0
  AV_SYNC_VIDEO_MASTER* = 1
  AV_SYNC_EXTERNAL_CLOCK* = 2

type
  MyAVPacketList* {.bycopy.} = object
    pkt*: AVPacket
    next*: ptr MyAVPacketList
    serial*: cint

  PacketQueue* {.bycopy.} = object
    first_pkt*: ptr MyAVPacketList
    last_pkt*: ptr MyAVPacketList
    nb_packets*: cint
    size*: cint
    duration*: int64
    abort_request*: cint
    serial*: cint
    mutex*: ptr SDL_mutex
    cond*: ptr SDL_cond

  AudioParams* {.bycopy.} = object
    freq*: cint
    channels*: cint
    channel_layout*: int64
    fmt*: AVSampleFormat
    frame_size*: cint
    bytes_per_sec*: cint

  Clock* {.bycopy.} = object
    pts*: cdouble
    pts_drift*: cdouble
    last_updated*: cdouble
    speed*: cdouble
    serial*: cint
    paused*: cint
    queue_serial*: ptr cint

  Frame* {.bycopy.} = object
    frame*: ptr AVFrame
    sub*: AVSubtitle
    serial*: cint
    pts*: cdouble
    duration*: cdouble
    pos*: int64
    width*: cint
    height*: cint
    format*: cint
    sar*: AVRational
    uploaded*: cint
    flip_v*: cint

  FrameQueue* {.bycopy.} = object
    queue*: array[(if (9) > ((if (3) > (16): (3) else: (16))): (9) else: ((if (3) > (16): (3) else: (16)))), Frame]
    rindex*: cint
    windex*: cint
    size*: cint
    max_size*: cint
    keep_last*: cint
    rindex_shown*: cint
    mutex*: ptr SDL_mutex
    cond*: ptr SDL_cond
    pktq*: ptr PacketQueue

  Decoder* {.bycopy.} = object
    pkt*: AVPacket
    queue*: ptr PacketQueue
    avctx*: ptr AVCodecContext
    pkt_serial*: cint
    finished*: cint
    packet_pending*: cint
    empty_queue_cond*: ptr SDL_cond
    start_pts*: int64
    start_pts_tb*: AVRational
    next_pts*: int64
    next_pts_tb*: AVRational
    decoder_tid*: ptr SDL_Thread



  VideoState* {.bycopy.} = object
    read_tid*: ptr SDL_Thread
    iformat*: ptr AVInputFormat
    abort_request*: cint
    force_refresh*: cint
    paused*: cint
    last_paused*: cint
    queue_attachments_req*: cint
    seek_req*: cint
    seek_flags*: cint
    seek_pos*: int64
    seek_rel*: int64
    read_pause_return*: cint
    ic*: ptr AVFormatContext
    realtime*: cint
    audclk*: Clock
    vidclk*: Clock
    extclk*: Clock
    pictq*: FrameQueue
    subpq*: FrameQueue
    sampq*: FrameQueue
    auddec*: Decoder
    viddec*: Decoder
    subdec*: Decoder
    audio_stream*: cint
    av_sync_type*: cint
    audio_clock*: cdouble
    audio_clock_serial*: cint
    audio_diff_cum*: cdouble
    audio_diff_avg_coef*: cdouble
    audio_diff_threshold*: cdouble
    audio_diff_avg_count*: cint
    audio_st*: ptr AVStream
    audioq*: PacketQueue
    audio_hw_buf_size*: cint
    audio_buf*: ptr uint8
    audio_buf1*: ptr uint8
    audio_buf_size*: cuint
    audio_buf1_size*: cuint
    audio_buf_index*: cint
    audio_write_buf_size*: cint
    audio_volume*: cint
    muted*: cint
    audio_src*: AudioParams
    audio_filter_src*: AudioParams
    audio_tgt*: AudioParams
    swr_ctx*: ptr SwrContext
    frame_drops_early*: cint
    frame_drops_late*: cint
    show_mode*: ShowMode
    sample_array*: array[(8 * 65536), int16]
    sample_array_index*: cint
    last_i_start*: cint
    rdft*: ptr RDFTContext
    rdft_bits*: cint
    rdft_data*: ptr FFTSample
    xpos*: cint
    last_vis_time*: cdouble
    vis_texture*: ptr SDL_Texture
    sub_texture*: ptr SDL_Texture
    vid_texture*: ptr SDL_Texture
    subtitle_stream*: cint
    subtitle_st*: ptr AVStream
    subtitleq*: PacketQueue
    frame_timer*: cdouble
    frame_last_returned_time*: cdouble
    frame_last_filter_delay*: cdouble
    video_stream*: cint
    video_st*: ptr AVStream
    videoq*: PacketQueue
    max_frame_duration*: cdouble ##  maximum duration of a frame - above this, we consider the jump a timestamp discontinuity
    img_convert_ctx*: ptr SwsContext
    sub_convert_ctx*: ptr SwsContext
    eof*: cint
    filename*: cstring
    width*: cint
    height*: cint
    xleft*: cint
    ytop*: cint
    step*: cint
    vfilter_idx*: cint
    in_video_filter*: ptr AVFilterContext ##  the first filter in the video chain
    out_video_filter*: ptr AVFilterContext ##  the last filter in the video chain
    in_audio_filter*: ptr AVFilterContext ##  the first filter in the audio chain
    out_audio_filter*: ptr AVFilterContext ##  the last filter in the audio chain
    agraph*: ptr AVFilterGraph  ##  audio filter graph
    last_video_stream*: cint
    last_audio_stream*: cint
    last_subtitle_stream*: cint
    continue_read_thread*: ptr SDL_cond


var file_iformat*: ptr AVInputFormat

var input_filename*: cstring

var window_title*: cstring

var default_width*: cint = 640

var default_height*: cint = 480

var screen_width*: cint = 0

var screen_height*: cint = 0

var screen_left*: cint = (0x2FFF0000 or (0))

var screen_top*: cint = (0x2FFF0000 or (0))

var audio_disable*: cint

var video_disable*: cint

var subtitle_disable*: cint

var wanted_stream_spec*: array[AVMEDIA_TYPE_NB.int, cstring]

var seek_by_bytes*: cint = -1

var seek_interval*: cfloat = 10

var display_disable*: cint

var borderless*: cint

var alwaysontop*: cint

var startup_volume*: cint = 100

var show_status*: cint = -1

var av_sync_type*: cint = AV_SYNC_AUDIO_MASTER

var start_time*: int64 = AV_NOPTS_VALUE

var duration*: int64 = AV_NOPTS_VALUE

var fast*: cint = 0

var genpts*: cint = 0

var lowres*: cint = 0

var decoder_reorder_pts*: cint = -1

var autoexit*: cint

var exit_on_keydown*: cint

var exit_on_mousedown*: cint

var loop*: cint = 1

var framedrop*: cint = -1

var infinite_buffer*: cint = -1

var show_mode*: ShowMode = SHOW_MODE_NONE

var audio_codec_name*: cstring

var subtitle_codec_name*: cstring

var video_codec_name*: cstring

var rdftspeed*: cdouble = 0.02

var cursor_last_shown*: int64

var cursor_hidden*: cint = 0

var vfilters_list*: cstringArray

var nb_vfilters*: cint = 0

var afilters*: cstring

var autorotate*: cint = 1

var find_stream_info*: cint = 1

var filter_nbthreads*: cint = 0

var is_full_screen*: cint

var audio_callback_time*: int64

var flush_pkt*: AVPacket

const
  FF_QUIT_EVENT* = (SDL_USEREVENT_ENUM.int + 2)

var window*: ptr SDL_Window

var renderer*: ptr SDL_Renderer

var renderer_info:SDL_RendererInfo

var audio_dev*: SDL_AudioDeviceID

type
  TextureFormatEntry* = tuple
    format: AVPixelFormat
    texture_fmt: cint


var sdl_texture_format_map*: seq[TextureFormatEntry] = @[
    (AV_PIX_FMT_RGB8, SDL_PIXELFORMAT_RGB332.cint),
    (AV_PIX_FMT_RGB444LE, SDL_PIXELFORMAT_RGB444.cint),
    (AV_PIX_FMT_RGB555LE, SDL_PIXELFORMAT_RGB555.cint),
    (AV_PIX_FMT_BGR555LE, SDL_PIXELFORMAT_BGR555.cint),
    (AV_PIX_FMT_RGB565LE, SDL_PIXELFORMAT_RGB565.cint),
    (AV_PIX_FMT_BGR565LE, SDL_PIXELFORMAT_BGR565.cint),
    (AV_PIX_FMT_RGB24, SDL_PIXELFORMAT_RGB24.cint),
    (AV_PIX_FMT_BGR24, SDL_PIXELFORMAT_BGR24.cint),
    (AV_PIX_FMT_BGR0, SDL_PIXELFORMAT_RGB888.cint),
    (AV_PIX_FMT_RGB0, SDL_PIXELFORMAT_BGR888.cint),
    (AV_PIX_FMT_0BGR, SDL_PIXELFORMAT_RGBX8888.cint),
    (AV_PIX_FMT_0RGB, SDL_PIXELFORMAT_BGRX8888.cint),
    (AV_PIX_FMT_BGRA, SDL_PIXELFORMAT_ARGB8888.cint),
    (AV_PIX_FMT_ABGR, SDL_PIXELFORMAT_RGBA8888.cint),
    (AV_PIX_FMT_RGBA, SDL_PIXELFORMAT_ABGR8888.cint),
    (AV_PIX_FMT_ARGB, SDL_PIXELFORMAT_BGRA8888.cint),
    (AV_PIX_FMT_YUV420P, SDL_PIXELFORMAT_IYUV.cint),
    (AV_PIX_FMT_YUYV422, SDL_PIXELFORMAT_YUY2.cint),
    (AV_PIX_FMT_UYVY422, SDL_PIXELFORMAT_UYVY.cint),
    (AV_PIX_FMT_NONE, SDL_PIXELFORMAT_UNKNOWN.cint)]

proc opt_add_vfilter*(optctx: pointer; opt: cstring; arg: cstring): cint =
  vfilters_list = cast[cstringArray](grow_array(vfilters_list, sizeof((vfilters_list[])).cint, addr(nb_vfilters), nb_vfilters + 1))
  vfilters_list[nb_vfilters - 1] = arg
  return 0.cint

proc cmp_audio_fmts*(fmt1: AVSampleFormat; channel_count1: int64; fmt2: AVSampleFormat; channel_count2: int64): auto {.inline.} =
  if channel_count1 == 1 and channel_count2 == 1:
    return av_get_packed_sample_fmt(fmt1) != av_get_packed_sample_fmt(fmt2)
  else:
    return channel_count1 != channel_count2 or fmt1 != fmt2

proc av_get_channel_layout_nb_channels*(channel_layout: auto): cint

proc get_valid_channel_layout*(channel_layout: auto; channels: auto): auto {.inline.} =
  if channel_layout != 0 and av_get_channel_layout_nb_channels(channel_layout) == channels:
    return channel_layout
  else:
    return 0

proc packet_queue_put_private*(q: ptr PacketQueue; pkt: ptr AVPacket): auto {.discardable.} =
  var pkt1: ptr MyAVPacketList
  if q.abort_request != 0:
    return -1
  pkt1 = cast[ptr MyAVPacketList](av_malloc(sizeof(MyAVPacketList).csize_t))
  if pkt1 == nil:
    return -1
  pkt1.pkt = pkt[]
  pkt1.next = nil
  if pkt == addr(flush_pkt):
    inc(q.serial)
  pkt1.serial = q.serial
  if q.last_pkt != nil:
    q.first_pkt = pkt1
  else:
    q.last_pkt.next = pkt1
  q.last_pkt = pkt1
  inc(q.nb_packets)
  inc(q.size, pkt1.pkt.size + sizeof((pkt1[])))
  inc(q.duration, pkt1.pkt.duration)
  discard SDL_CondSignal(q.cond)
  return 0

proc packet_queue_put*(q: ptr PacketQueue; pkt: ptr AVPacket): auto {.discardable.} =
  var result: cint
  SDL_LockMutex(q.mutex)
  result = packet_queue_put_private(q, pkt)
  SDL_UnlockMutex(q.mutex)
  if pkt != addr(flush_pkt) and result < 0:
    av_packet_unref(pkt)
  return result

proc packet_queue_put_nullpacket*(q: ptr PacketQueue; stream_index: cint): auto {.discardable.} =
  var
    pkt1: AVPacket
    pkt: ptr AVPacket = addr(pkt1)
  av_init_packet(pkt)
  pkt.data = nil
  pkt.size = 0
  pkt.stream_index = stream_index
  return packet_queue_put(q, pkt)

proc packet_queue_init*(q: ptr PacketQueue): auto {.discardable.} =
  q.mutex = SDL_CreateMutex()
  q.cond = SDL_CreateCond()
  q.abort_request = 1
  return 0

proc packet_queue_flush*(q: ptr PacketQueue) =
  var
    pkt: ptr MyAVPacketList
    pkt1: ptr MyAVPacketList
  SDL_LockMutex(q.mutex)
  pkt = q.first_pkt
  while pkt != nil:
    pkt1 = pkt.next
    av_packet_unref(addr(pkt.pkt))
    av_freep(addr(pkt))
    pkt = pkt1
  q.last_pkt = nil
  q.first_pkt = nil
  q.nb_packets = 0
  q.size = 0
  q.duration = 0
  SDL_UnlockMutex(q.mutex)

proc packet_queue_destroy*(q: ptr PacketQueue) =
  packet_queue_flush(q)
  SDL_DestroyMutex(q.mutex)
  SDL_DestroyCond(q.cond)

proc packet_queue_abort*(q: ptr PacketQueue) =
  SDL_LockMutex(q.mutex)
  q.abort_request = 1
  SDL_CondSignal(q.cond)
  SDL_UnlockMutex(q.mutex)

proc packet_queue_start*(q: ptr PacketQueue) =
  SDL_LockMutex(q.mutex)
  q.abort_request = 0
  packet_queue_put_private(q, addr(flush_pkt))
  SDL_UnlockMutex(q.mutex)

proc packet_queue_get*(q: ptr PacketQueue; pkt: ptr AVPacket; `block`: cint; serial: ptr cint): auto =
  var pkt1: ptr MyAVPacketList
  var result: cint
  SDL_LockMutex(q.mutex)
  while true:
    if q.abort_request != 0:
      result = -1
      break
    pkt1 = q.first_pkt
    if pkt1 != nil:
      q.first_pkt = pkt1.next
      if q.first_pkt == nil:
        q.last_pkt = nil
      dec(q.nb_packets)
      dec(q.size, pkt1.pkt.size + sizeof((pkt1[])))
      dec(q.duration, pkt1.pkt.duration)
      pkt[] = pkt1.pkt
      if serial != nil:
        serial[] = pkt1.serial
      av_free(pkt1)
      result = 1
      break
    elif `block` == 0:
      result = 0
      break
    else:
      SDL_CondWait(q.cond, q.mutex)
  SDL_UnlockMutex(q.mutex)
  return result

proc decoder_init*(d: ptr Decoder; avctx: ptr AVCodecContext; queue: ptr PacketQueue;empty_queue_cond: ptr SDL_cond) =
  d.avctx = avctx
  d.queue = queue
  d.empty_queue_cond = empty_queue_cond
  d.start_pts = AV_NOPTS_VALUE
  d.pkt_serial = -1

proc decoder_decode_frame*(d: ptr Decoder; frame: ptr AVFrame; sub: ptr AVSubtitle): cint =
  var result: cint = (-(11))
  while true:
    var pkt: AVPacket
    if d.queue.serial == d.pkt_serial:
      while true:
        if d.queue.abort_request != 0:
          return -1
        case d.avctx.codec_type
        of AVMEDIA_TYPE_VIDEO:
          result = avcodec_receive_frame(d.avctx, frame)
          if result >= 0:
            if decoder_reorder_pts == -1:
              frame.pts = frame.best_effort_timestamp
            elif decoder_reorder_pts == 0:
              frame.pts = frame.pkt_dts
        of AVMEDIA_TYPE_AUDIO:
          result = avcodec_receive_frame(d.avctx, frame)
          if result >= 0:
             var tb:AVRational = AVRational(num:1, den:frame.sample_rate)
             if (frame.pts != 0):
                 frame.pts = av_rescale_q(frame.pts, d.avctx.pkt_timebase, tb)
             elif (d.next_pts != 0):
                 frame.pts = av_rescale_q(d.next_pts, d.next_pts_tb, tb)
             if (frame.pts != 0):
                 d.next_pts = frame.pts + frame.nb_samples
                 d.next_pts_tb = tb;
        else:discard
        if result ==
            (-(int)(('E').int or (('O').int shl 8) or (('F').int shl 16) or ((' ').int shl 24))):
          d.finished = d.pkt_serial
          avcodec_flush_buffers(d.avctx)
          return 0
        if result >= 0:
          return 1
        if not (result != (-(11))):
          break
    while true:
      if d.queue.nb_packets == 0:
        SDL_CondSignal(d.empty_queue_cond)
      if d.packet_pending != 0:
        av_packet_move_ref(addr(pkt), addr(d.pkt))
        d.packet_pending = 0
      else:
        if packet_queue_get(d.queue, addr(pkt), 1, addr(d.pkt_serial)) < 0:
          return -1
      if d.queue.serial == d.pkt_serial:
        break
      av_packet_unref(addr(pkt))
    if pkt.data == flush_pkt.data:
      avcodec_flush_buffers(d.avctx)
      d.finished = 0
      d.next_pts = d.start_pts
      d.next_pts_tb = d.start_pts_tb
    else:
      if d.avctx.codec_type == AVMEDIA_TYPE_SUBTITLE:
        var got_frame: cint = 0
        result = avcodec_decode_subtitle2(d.avctx, sub, addr(got_frame), addr(pkt))
        if result < 0:
          result = (-(11))
        else:
          if got_frame != 0 and pkt.data == nil:
            d.packet_pending = 1
            av_packet_move_ref(addr(d.pkt), addr(pkt))
          result = if got_frame != 0: 0 
                else: 
                  if pkt.data != nil: -11
                  else: (-(int)(('E').int or (('O').int shl 8) or (('F').int shl 16) or ((' ').int shl 24)))
      else:
        if avcodec_send_packet(d.avctx, addr(pkt)) == (-(11)):
          av_log(d.avctx, 16, "Receive_frame and send_packet both returned EAGAIN, which is an API violation.")
          d.packet_pending = 1
          av_packet_move_ref(addr(d.pkt), addr(pkt))
      av_packet_unref(addr(pkt))

proc decoder_destroy*(d: ptr Decoder) =
  av_packet_unref(addr(d.pkt))
  avcodec_free_context(addr(d.avctx))

proc frame_queue_unref_item*(vp: ptr Frame) =
  av_frame_unref(vp.frame)
  avsubtitle_free(addr(vp.sub))

proc frame_queue_init*(f: ptr FrameQueue; pktq: ptr PacketQueue; max_size: cint;
                      keep_last: cint): cint =
  f.mutex = SDL_CreateMutex()
  f.cond = SDL_CreateCond()
  f.pktq = pktq
  f.max_size = (if (max_size) >
      ((if (9) > ((if (3) > (16): (3) else: (16))): (9) else: (
      (if (3) > (16): (3) else: (16))))): ((if (9) > ((if (3) > (16): (3) else: (16))): (9) else: (
      (if (3) > (16): (3) else: (16))))) else: (max_size))
  f.keep_last = not not keep_last
  for i in 0 .. f.max_size - 1:
    f.queue[i].frame = av_frame_alloc()
    if f.queue[i].frame == nil:
      return -(12)
  return 0

proc frame_queue_destory*(f: ptr FrameQueue) =
  var i: cint
  i = 0
  while i < f.max_size:
    var vp: ptr Frame = addr(f.queue[i])
    frame_queue_unref_item(vp)
    av_frame_free(addr(vp.frame))
    inc(i)
  SDL_DestroyMutex(f.mutex)
  SDL_DestroyCond(f.cond)

proc frame_queue_signal*(f: ptr FrameQueue) =
  SDL_LockMutex(f.mutex)
  SDL_CondSignal(f.cond)
  SDL_UnlockMutex(f.mutex)

proc frame_queue_peek*(f: ptr FrameQueue): ptr Frame =
  return addr(f.queue[(f.rindex + f.rindex_shown) mod f.max_size])

proc frame_queue_peek_next*(f: ptr FrameQueue): ptr Frame =
  return addr(f.queue[(f.rindex + f.rindex_shown + 1) mod f.max_size])

proc frame_queue_peek_last*(f: ptr FrameQueue): ptr Frame =
  return addr(f.queue[f.rindex])

proc frame_queue_peek_writable*(f: ptr FrameQueue): ptr Frame =
  SDL_LockMutex(f.mutex)
  while f.size >= f.max_size and f.pktq.abort_request == 0:
    SDL_CondWait(f.cond, f.mutex)
  SDL_UnlockMutex(f.mutex)
  if f.pktq.abort_request != 0:
    return nil
  return addr(f.queue[f.windex])

proc frame_queue_peek_readable*(f: ptr FrameQueue): ptr Frame =
  SDL_LockMutex(f.mutex)
  while f.size - f.rindex_shown <= 0 and f.pktq.abort_request == 0:
    SDL_CondWait(f.cond, f.mutex)
  SDL_UnlockMutex(f.mutex)
  if f.pktq.abort_request != 0:
    return nil
  return addr(f.queue[(f.rindex + f.rindex_shown) mod f.max_size])

proc frame_queue_push*(f: ptr FrameQueue) =
  inc(f.windex)
  if f.windex == f.max_size:
    f.windex = 0
  SDL_LockMutex(f.mutex)
  inc(f.size)
  SDL_CondSignal(f.cond)
  SDL_UnlockMutex(f.mutex)

proc frame_queue_next*(f: ptr FrameQueue) =
  if f.keep_last != 0 and f.rindex_shown != 0:
    f.rindex_shown = 1
    return
  frame_queue_unref_item(addr(f.queue[f.rindex]))
  inc(f.rindex)
  if f.rindex == f.max_size:
    f.rindex = 0
  SDL_LockMutex(f.mutex)
  dec(f.size)
  SDL_CondSignal(f.cond)
  SDL_UnlockMutex(f.mutex)

proc frame_queue_nb_remaining*(f: ptr FrameQueue): cint =
  return f.size - f.rindex_shown

proc frame_queue_last_pos*(f: ptr FrameQueue): auto =
  var fp: ptr Frame = addr(f.queue[f.rindex])
  if f.rindex_shown != 0 and fp.serial == f.pktq.serial:
    return fp.pos
  else:
    return -1

proc decoder_abort*(d: ptr Decoder; fq: ptr FrameQueue) =
  packet_queue_abort(d.queue)
  frame_queue_signal(fq)
  SDL_WaitThread(d.decoder_tid, nil)
  d.decoder_tid = nil
  packet_queue_flush(d.queue)

proc fill_rectangle*(x: cint; y: cint; w: cint; h: cint) {.inline.} =
  var rect: SDL_Rect
  rect.x = x
  rect.y = y
  rect.w = w
  rect.h = h
  if w != 0 and h != 0:
    SDL_RenderFillRect(renderer, addr(rect))

proc realloc_texture*(texture: ptr ptr SDL_Texture; new_format: uint32;
                     new_width: cint; new_height: cint; blendmode: SDL_BlendMode;
                     init_texture: cint): cint {.discardable.} =
  var format: uint32
  var
    access: cint
    w: cint
    h: cint
  if texture[] != nil or
      SDL_QueryTexture(texture[], addr(format), addr(access), addr(w), addr(h)) < 0 or
      new_width != w or new_height != h or new_format != format:
    var pixels: pointer
    var pitch: cint
    if texture[] != nil:
      SDL_DestroyTexture(texture[])
    texture[] = SDL_CreateTexture(renderer, new_format, SDL_TEXTUREACCESS_STREAMING, new_width, new_height)
    if SDL_SetTextureBlendMode(texture[], blendmode) < 0:
      return -1
    if init_texture != 0:
      if SDL_LockTexture(texture[], nil, addr(pixels), addr(pitch)) < 0:
        return -1
      SDL_UnlockTexture(texture[])
    av_log(nil, 40, "Created %dx%d texture with %s.", new_width, new_height, SDL_GetPixelFormatName(new_format))
  return 0

proc calculate_display_rect*(rect: ptr SDL_Rect; scr_xleft: cint; scr_ytop: cint;
                            scr_width: cint; scr_height: cint; pic_width: cint;
                            pic_height: cint; pic_sar: AVRational) =
  var aspect_ratio: AVRational = pic_sar
  var
    width: int64
    height: int64
    x: int64
    y: int64
  if av_cmp_q(aspect_ratio, av_make_q(0, 1)) <= 0:
    aspect_ratio = av_make_q(1, 1)
  aspect_ratio = av_mul_q(aspect_ratio, av_make_q(pic_width, pic_height))
  height = scr_height
  width = av_rescale(height, aspect_ratio.num, aspect_ratio.den) and not 1
  if width > scr_width:
    width = scr_width
    height = av_rescale(width, aspect_ratio.den, aspect_ratio.num) and not 1
  x = (scr_width - width) div 2
  y = (scr_height - height) div 2
  rect.x = cint scr_xleft + x
  rect.y = cint scr_ytop + y
  rect.w = (if (cast[cint](width)) > (1): (cast[cint](width)) else: (1))
  rect.h = (if (cast[cint](height)) > (1): (cast[cint](height)) else: (1))

proc get_sdl_pix_fmt_and_blendmode*(format: cint; sdl_pix_fmt: ptr uint32;sdl_blendmode: ptr SDL_BlendMode) =
  sdl_blendmode[] = SDL_BLENDMODE_NONE
  sdl_pix_fmt[] = SDL_PIXELFORMAT_UNKNOWN
  if format == AV_PIX_FMT_BGRA.cint or format == AV_PIX_FMT_ABGR.cint or format == AV_PIX_FMT_RGBA.cint or format == AV_PIX_FMT_ARGB.cint:
    sdl_blendmode[] = SDL_BLENDMODE_BLEND
  for i in 0.. sdl_texture_format_map.high:
    if format == sdl_texture_format_map[i].format.cint:
      sdl_pix_fmt[] = sdl_texture_format_map[i].texture_fmt.uint32
      return

proc upload_texture*(tex: ptr ptr SDL_Texture; frame: ptr AVFrame;img_convert_ctx: ptr ptr SwsContext): cint =
  var result: cint = 0
  var sdl_pix_fmt: uint32
  var sdl_blendmode: SDL_BlendMode
  get_sdl_pix_fmt_and_blendmode(frame.format, addr(sdl_pix_fmt),addr(sdl_blendmode))
  var tmp = if sdl_pix_fmt == SDL_PIXELFORMAT_UNKNOWN: SDL_PIXELFORMAT_ARGB8888.uint32 else: sdl_pix_fmt
  discard realloc_texture(tex, tmp,frame.width,frame.height, sdl_blendmode, 0)
  case sdl_pix_fmt
  of SDL_PIXELFORMAT_UNKNOWN:
    img_convert_ctx[] = sws_getCachedContext(img_convert_ctx[], frame.width,
        frame.height, frame.format, frame.width, frame.height, AV_PIX_FMT_BGRA,
        sws_flags, nil, nil, nil)
    if img_convert_ctx[] != nil:
      var pixels: array[4, ptr uint8]
      var pitch: array[4, cint]
      if SDL_LockTexture(tex[], nil, cast[ptr pointer](pixels.addr),pitch[0].addr) == 0:
        sws_scale(img_convert_ctx[], cast[ptr ptr uint8](frame.data.addr), frame.linesize[0].addr, 0, frame.height, pixels[0].addr, pitch[0].addr)
        SDL_UnlockTexture(tex[])
    else:
      av_log(nil, 8, "Cannot initialize the conversion context")
      result = -1
  of SDL_PIXELFORMAT_IYUV:
    if frame.linesize[0] > 0 and frame.linesize[1] > 0 and frame.linesize[2] > 0:
      result = SDL_UpdateYUVTexture(tex[], nil, frame.data[0],frame.linesize[0], frame.data[1],frame.linesize[1], frame.data[2], frame.linesize[2])
    elif frame.linesize[0] < 0 and frame.linesize[1] < 0 and frame.linesize[2] < 0:
      result = SDL_UpdateYUVTexture(tex[], nil, frame.data[0] + frame.linesize[0] * (frame.height - 1), -frame.linesize[0], frame.data[1] + frame.linesize[1] *
          ((if false: -((-(frame.height)) shr (1)) else: (
          (frame.height) + (1 shl (1)) - 1) shr (1)) - 1), -frame.linesize[1], frame.data[2] +
          frame.linesize[2] *
          ((if false: -((-(frame.height)) shr (1)) else: (
          (frame.height) + (1 shl (1)) - 1) shr (1)) - 1), -frame.linesize[2])
    else:
      av_log(nil, 16,
             "Mixed negative and positive linesizes are not supported.")
      return -1
  else:
    if frame.linesize[0] < 0:
      result = SDL_UpdateTexture(tex[], nil, frame.data[0] +
          frame.linesize[0] * (frame.height - 1), -frame.linesize[0])
    else:
      result = SDL_UpdateTexture(tex[], nil, frame.data[0],
                            frame.linesize[0])
  return result

proc set_sdl_yuv_conversion_mode*(frame: ptr AVFrame) =
  var mode: SDL_YUV_CONVERSION_MODE = SDL_YUV_CONVERSION_AUTOMATIC
  if frame != nil and (frame.format == AV_PIX_FMT_YUV420P.cint or frame.format == AV_PIX_FMT_YUYV422.cint or frame.format == AV_PIX_FMT_UYVY422.cint):
    if frame.color_range == AVCOL_RANGE_JPEG:
      mode = SDL_YUV_CONVERSION_JPEG
    elif frame.colorspace == AVCOL_SPC_BT709:
      mode = SDL_YUV_CONVERSION_BT709
    elif frame.colorspace == AVCOL_SPC_BT470BG or
        frame.colorspace == AVCOL_SPC_SMPTE170M or
        frame.colorspace == AVCOL_SPC_SMPTE240M:
      mode = SDL_YUV_CONVERSION_BT601
  SDL_SetYUVConversionMode(mode)

proc video_image_display*(`is`: ptr VideoState) =
  var vp: ptr Frame
  var sp: ptr Frame
  var rect: SDL_Rect
  vp = frame_queue_peek_last(addr(`is`.pictq))
  if `is`.subtitle_st != nil:
    if frame_queue_nb_remaining(addr(`is`.subpq)) > 0:
      sp = frame_queue_peek(addr(`is`.subpq))
      if vp.pts >= sp.pts + (cast[cfloat](sp.sub.start_display_time div 1000)):
        if sp.uploaded == 0:
          var pixels: array[4, ptr uint8]
          var pitch: array[4, cint]
          if sp.width == 0 or sp.height == 0:
            sp.width = vp.width
            sp.height = vp.height
          if realloc_texture(addr(`is`.sub_texture), SDL_PIXELFORMAT_ARGB8888, sp.width, sp.height, SDL_BLENDMODE_BLEND, 1) < 0:
            return
          for i in 0.. sp.sub.num_rects-1:
            var sub_rect: ptr AVSubtitleRect = sp.sub.rects[i.int]
            sub_rect.x = av_clip_c(sub_rect.x, 0, sp.width)
            sub_rect.y = av_clip_c(sub_rect.y, 0, sp.height)
            sub_rect.w = av_clip_c(sub_rect.w, 0, sp.width - sub_rect.x)
            sub_rect.h = av_clip_c(sub_rect.h, 0, sp.height - sub_rect.y)
            `is`.sub_convert_ctx = sws_getCachedContext(`is`.sub_convert_ctx,
                sub_rect.w, sub_rect.h, AV_PIX_FMT_PAL8, sub_rect.w, sub_rect.h,
                AV_PIX_FMT_BGRA, 0, nil, nil,
                nil)
            if `is`.sub_convert_ctx == nil:
              av_log(nil, 8, "Cannot initialize the conversion context")
              return
            if SDL_LockTexture(`is`.sub_texture, cast[ptr SDL_Rect](sub_rect),cast[ptr pointer](pixels.addr), pitch[0].addr) == 0:
              sws_scale(`is`.sub_convert_ctx, cast[ptr ptr uint8](sub_rect.data.addr), sub_rect.linesize[0].addr, 0, sub_rect.h, pixels[0].addr, pitch[0].addr)
              SDL_UnlockTexture(`is`.sub_texture)
          sp.uploaded = 1
      else:
        sp = nil
  calculate_display_rect(addr(rect), `is`.xleft, `is`.ytop, `is`.width, `is`.height, vp.width, vp.height, vp.sar)
  if vp.uploaded == 0:
    if upload_texture(addr(`is`.vid_texture), vp.frame, addr(`is`.img_convert_ctx)) < 0:
      return
    vp.uploaded = 1
    vp.flip_v = cint vp.frame.linesize[0] < 0
  set_sdl_yuv_conversion_mode(vp.frame)
  SDL_RenderCopyEx(renderer, `is`.vid_texture, nil, addr(rect), 0, nil, if vp.flip_v != 0: SDL_FLIP_VERTICAL else: 0.SDL_RendererFlip)
  set_sdl_yuv_conversion_mode(nil)
  if sp != nil:
    SDL_RenderCopy(renderer, `is`.sub_texture, nil, addr(rect))

proc compute_mod*(a: cint; b: cint): cint {.inline.} =
  return if a < 0: a mod b + b else: a mod b

proc video_audio_display*(s: ptr VideoState) =
  var
    i: cint
    i_start: cint
    x: cint
    y1: cint
    y: cint
    ys: cint
    delay: cint
    n: cint
    nb_display_channels: cint
  var
    ch: cint
    channels: cint
    h: cint
    h2: cint
  var time_diff: int64
  var
    rdft_bits: cint
    nb_freq: cint
  rdft_bits = 1
  while (1 shl rdft_bits) < 2 * s.height:
    inc(rdft_bits)
  nb_freq = cint 1 shl (rdft_bits - 1)
  ##  compute display index : center on currently output samples
  channels = s.audio_tgt.channels
  nb_display_channels = channels
  if s.paused == 0:
    var data_used: cint = if s.show_mode == SHOW_MODE_WAVES: s.width else: (2 * nb_freq)
    n = 2 * channels
    delay = s.audio_write_buf_size
    delay = cint delay / n
    ##  to be more precise, we take into account the time spent since
    ## the last buffer computation
    if audio_callback_time != 0:
      time_diff = av_gettime_relative() - audio_callback_time
      dec(delay, (time_diff * s.audio_tgt.freq) div 1000000)
    inc(delay, 2 * data_used)
    if delay < data_used:
      delay = data_used
    x = compute_mod(s.sample_array_index - delay * channels, SAMPLE_ARRAY_SIZE)
    i_start = x
    if s.show_mode == SHOW_MODE_WAVES:
      h = int.low.cint
      i = 0
      while i < 1000:
        var idx: cint = (SAMPLE_ARRAY_SIZE + x - i) mod SAMPLE_ARRAY_SIZE
        var a: cint = s.sample_array[idx]
        var b: cint = s.sample_array[(idx + 4 * channels) mod SAMPLE_ARRAY_SIZE]
        var c: cint = s.sample_array[(idx + 5 * channels) mod SAMPLE_ARRAY_SIZE]
        var d: cint = s.sample_array[(idx + 9 * channels) mod SAMPLE_ARRAY_SIZE]
        var score: cint = a - d
        if h < score and (b xor c) < 0:
          h = score
          i_start = idx
        inc(i, channels)
    s.last_i_start = i_start
  else:
    i_start = s.last_i_start
  if s.show_mode == SHOW_MODE_WAVES:
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255)
    h = s.height div nb_display_channels
    h2 = (h * 9) div 20
    ch = 0
    while ch < nb_display_channels:
      i = i_start + ch
      y1 = s.ytop + ch * h + (h div 2)
      ##  position of center line
      x = 0
      while x < s.width:
        y = (s.sample_array[i] * h2) shr 15
        if y < 0:
          y = -y
          ys = y1 - y
        else:
          ys = y1
        fill_rectangle(s.xleft + x, ys, 1, y)
        inc(i, channels)
        if i >= SAMPLE_ARRAY_SIZE:
          dec(i, SAMPLE_ARRAY_SIZE)
        inc(x)
      inc(ch)
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255)
    ch = 1
    while ch < nb_display_channels:
      y = s.ytop + ch * h
      fill_rectangle(s.xleft, y, s.width, 1)
      inc(ch)
  else:
    if realloc_texture(s.vis_texture.addr, SDL_PIXELFORMAT_ARGB8888, s.width, s.height, SDL_BLENDMODE_NONE, 1) < 0:
      return
    nb_display_channels = min(nb_display_channels, 2)
    if rdft_bits != s.rdft_bits:
      av_rdft_end(s.rdft)
      av_free(s.rdft_data)
      s.rdft = av_rdft_init(rdft_bits, DFT_R2C)
      s.rdft_bits = rdft_bits
      s.rdft_data = cast[ptr FFTSample](av_malloc_array(nb_freq.csize_t, 4 * sizeof((s.rdft_data[]))))
    if s.rdft == nil or  s.rdft_data == nil:
      echo("Failed to allocate buffers for RDFT, switching to waves display")
      s.show_mode = SHOW_MODE_WAVES
    else:
      var data: array[2, ptr FFTSample]
      var rect: SDL_Rect = (x:s.xpos, y:0.cint,w:1.cint,h:s.height)
      var pixels: ptr uint32
      var pitch: cint
      ch = 0
      while ch < nb_display_channels:
        data[ch] = s.rdft_data + 2 * nb_freq * ch
        i = i_start + ch
        x = 0
        while x < 2 * nb_freq:
          var w: cdouble = cdouble (x - nb_freq) * (1 div nb_freq)
          data[ch][x] = s.sample_array[i].float * (1 - w * w)
          inc(i, channels)
          if i >= SAMPLE_ARRAY_SIZE:
            dec(i, SAMPLE_ARRAY_SIZE)
          inc(x)
        av_rdft_calc(s.rdft, data[ch])
        inc(ch)
      if SDL_LockTexture(s.vis_texture, addr(rect), cast[ptr pointer](addr(pixels)), addr(pitch)) == 0:
        pitch = pitch shr 2
        pixels += pitch * s.height
        y = 0
        while y < s.height:
          var w: cdouble = cdouble 1 div sqrt(nb_freq.float).int
          var a: uint32 = uint32 sqrt(w * sqrt(data[0][2 * y + 0] * data[0][2 * y + 0] + data[0][2 * y + 1] * data[0][2 * y + 1]))
          var b: uint32 = if nb_display_channels == 2: sqrt(w * hypot(data[1][2 * y + 0], data[1][2 * y + 1])).uint32 else: a
          a = min(a, 255)
          b = min(b, 255)
          pixels -= pitch
          pixels[] =  (a shl 16) + (b shl 8) + ((a + b) shr 1)
          inc(y)
        SDL_UnlockTexture(s.vis_texture)
      SDL_RenderCopy(renderer, s.vis_texture, nil, nil)
    if s.paused == 0:
      inc(s.xpos)
    if s.xpos >= s.width:
      s.xpos = s.xleft

proc stream_component_close*(vs: ptr VideoState; stream_index: cint) =
  var ic: ptr AVFormatContext = vs.ic
  var codecpar: ptr AVCodecParameters
  if stream_index < 0 or stream_index >= ic.nb_streams.cint:
    return
  codecpar = ic.streams[stream_index].codecpar
  case codecpar.codec_type
  of AVMEDIA_TYPE_AUDIO:
    decoder_abort(addr(vs.auddec), addr(vs.sampq))
    SDL_CloseAudioDevice(audio_dev)
    decoder_destroy(addr(vs.auddec))
    swr_free(addr(vs.swr_ctx))
    av_freep(addr(vs.audio_buf1))
    vs.audio_buf1_size = 0
    vs.audio_buf = nil
    if vs.rdft != nil:
      av_rdft_end(vs.rdft)
      av_freep(addr(vs.rdft_data))
      vs.rdft = nil
      vs.rdft_bits = 0
  of AVMEDIA_TYPE_VIDEO:
    decoder_abort(addr(vs.viddec), addr(vs.pictq))
    decoder_destroy(addr(vs.viddec))
  of AVMEDIA_TYPE_SUBTITLE:
    decoder_abort(addr(vs.subdec), addr(vs.subpq))
    decoder_destroy(addr(vs.subdec))
  else:
    discard
  ic.streams[stream_index].`discard` = AVDISCARD_ALL
  case codecpar.codec_type
  of AVMEDIA_TYPE_AUDIO:
    vs.audio_st = nil
    vs.audio_stream = -1
  of AVMEDIA_TYPE_VIDEO:
    vs.video_st = nil
    vs.video_stream = -1
  of AVMEDIA_TYPE_SUBTITLE:
    vs.subtitle_st = nil
    vs.subtitle_stream = -1
  else:
    discard

proc stream_close*(vs: ptr VideoState) =
  ##  XXX: use a special url_shutdown call to abort parse cleanly
  vs.abort_request = 1
  # SDL_WaitThread(vs.read_tid, nil)
  ##  close each stream
  if vs.audio_stream >= 0:
    stream_component_close(vs, vs.audio_stream)
  if vs.video_stream >= 0:
    stream_component_close(vs, vs.video_stream)
  if vs.subtitle_stream >= 0:
    stream_component_close(vs, vs.subtitle_stream)
  avformat_close_input(addr(vs.ic))
  packet_queue_destroy(addr(vs.videoq))
  packet_queue_destroy(addr(vs.audioq))
  packet_queue_destroy(addr(vs.subtitleq))
  ##  free all pictures
  frame_queue_destory(addr(vs.pictq))
  frame_queue_destory(addr(vs.sampq))
  frame_queue_destory(addr(vs.subpq))
  # SDL_DestroyCond(vs.continue_read_thread)
  sws_freeContext(vs.img_convert_ctx)
  sws_freeContext(vs.sub_convert_ctx)
  av_free(vs.filename)
  if vs.vis_texture != nil:
    SDL_DestroyTexture(vs.vis_texture)
  if vs.vid_texture != nil:
    SDL_DestroyTexture(vs.vid_texture)
  if vs.sub_texture != nil:
    SDL_DestroyTexture(vs.sub_texture)
  av_free(vs)


proc uninit_opts*() =
  av_dict_free(addr(swr_opts))
  av_dict_free(addr(sws_dict))
  av_dict_free(addr(format_opts))
  av_dict_free(addr(codec_opts))
  av_dict_free(addr(resample_opts))


proc do_exit*(vs: ptr VideoState) =
  if vs != nil:
    stream_close(vs)
  if renderer != nil:
    SDL_DestroyRenderer(renderer)
  if window != nil:
    SDL_DestroyWindow(window)
  uninit_opts()
  av_freep(addr(vfilters_list))
  discard avformat_network_deinit()
  SDL_Quit()
  quit(0)

proc sigterm_handler*(sig: cint) =
  quit(123)

proc set_default_window_size*(width: cint; height: cint; sar: AVRational) =
  var rect: SDL_Rect
  var max_width: cint =  if screen_width != 0: screen_width else: int.high.cint
  var max_height: cint = if screen_height != 0: screen_height else: int.high.cint
  if max_width == int.high and max_height == int.high:
    max_height = height
  calculate_display_rect(addr(rect), 0, 0, max_width, max_height, width, height, sar)
  default_width = rect.w
  default_height = rect.h

proc video_open*(vs: ptr VideoState): cint =
  var
    w: cint
    h: cint
  w = if screen_width != 0: screen_width else: default_width
  h = if screen_height != 0: screen_height else: default_height
  if window_title == nil:
    window_title = input_filename
  SDL_SetWindowTitle(window, window_title)
  SDL_SetWindowSize(window, w, h)
  SDL_SetWindowPosition(window, screen_left, screen_top)
  if is_full_screen != 0:
    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP.uint32)
  SDL_ShowWindow(window)
  vs.width = w
  vs.height = h
  return 0

##  display the current picture, if any

proc video_display*(vs: ptr VideoState) =
  if vs.width == 0:
    discard video_open(vs)
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255)
  SDL_RenderClear(renderer)
  if vs.audio_st != nil and vs.show_mode != SHOW_MODE_VIDEO:
    video_audio_display(vs)
  elif vs.video_st != nil:
    video_image_display(vs)
  SDL_RenderPresent(renderer)



proc get_clock*(c: ptr Clock): cdouble =
  if c.queue_serial[] != c.serial:
    return Nan
  if c.paused != 0:
    return c.pts
  else:
    var time: cdouble = cdouble av_gettime_relative() div 1000000
    return c.pts_drift + time - (time - c.last_updated) * (1.0 - c.speed)

proc set_clock_at*(c: ptr Clock; pts: cdouble; serial: cint; time: cdouble) =
  c.pts = pts
  c.last_updated = time
  c.pts_drift = c.pts - time
  c.serial = serial

proc set_clock*(c: ptr Clock; pts: cdouble; serial: cint) =
  var time: cdouble = cdouble av_gettime_relative() div 1000000
  set_clock_at(c, pts, serial, time)

proc set_clock_speed*(c: ptr Clock; speed: cdouble) =
  set_clock(c, get_clock(c), c.serial)
  c.speed = speed

proc init_clock*(c: ptr Clock; queue_serial: ptr cint) =
  c.speed = 1.0
  c.paused = 0
  c.queue_serial = queue_serial
  set_clock(c, Nan, -1)



proc sync_clock_to_slave*(c: ptr Clock; slave: ptr Clock) =
  var clock: cdouble = get_clock(c)
  var slave_clock: cdouble = get_clock(slave)
  if slave_clock != Nan and (clock == Nan or abs(clock - slave_clock) > AV_NOSYNC_THRESHOLD):
    set_clock(c, slave_clock, slave.serial)

proc get_master_sync_type*(vs: ptr VideoState): cint =
  if vs.av_sync_type == AV_SYNC_VIDEO_MASTER:
    if vs.video_st != nil:
      return AV_SYNC_VIDEO_MASTER
    else:
      return AV_SYNC_AUDIO_MASTER
  elif vs.av_sync_type == AV_SYNC_AUDIO_MASTER:
    if vs.audio_st != nil:
      return AV_SYNC_AUDIO_MASTER
    else:
      return AV_SYNC_EXTERNAL_CLOCK
  else:
    return AV_SYNC_EXTERNAL_CLOCK

##  get the current master clock value

proc get_master_clock*(vs: ptr VideoState): cdouble =
  var val: cdouble
  case get_master_sync_type(vs)
  of AV_SYNC_VIDEO_MASTER:
    val = get_clock(addr(vs.vidclk))
  of AV_SYNC_AUDIO_MASTER:
    val = get_clock(addr(vs.audclk))
  else:
    val = get_clock(addr(vs.extclk))
  return val

proc check_external_clock_speed*(vs: ptr VideoState) =
  if vs.video_stream >= 0 and
      vs.videoq.nb_packets <= EXTERNAL_CLOCK_MIN_FRAMES or
      vs.audio_stream >= 0 and
      vs.audioq.nb_packets <= EXTERNAL_CLOCK_MIN_FRAMES:
    set_clock_speed(addr(vs.extclk), max(EXTERNAL_CLOCK_SPEED_MIN,
        vs.extclk.speed - EXTERNAL_CLOCK_SPEED_STEP))
  elif (vs.video_stream < 0 or
      vs.videoq.nb_packets > EXTERNAL_CLOCK_MAX_FRAMES) and
      (vs.audio_stream < 0 or 
      vs.audioq.nb_packets > EXTERNAL_CLOCK_MAX_FRAMES):
    set_clock_speed(addr(vs.extclk), min(EXTERNAL_CLOCK_SPEED_MAX,
        vs.extclk.speed + EXTERNAL_CLOCK_SPEED_STEP))
  else:
    var speed: cdouble = vs.extclk.speed
    if speed != 1.0:
      set_clock_speed(addr(vs.extclk), speed + EXTERNAL_CLOCK_SPEED_STEP * (1.0 - speed) / abs(1.0 - speed))

##  seek in the stream

proc stream_seek*(vs: ptr VideoState; pos: int64; rel: int64; seek_by_bytes: cint) =
  if vs.seek_req != 0:
    vs.seek_pos = pos
    vs.seek_rel = rel
    vs.seek_flags = vs.seek_flags and not AVSEEK_FLAG_BYTE
    if seek_by_bytes != 0:
      vs.seek_flags = vs.seek_flags or AVSEEK_FLAG_BYTE
    vs.seek_req = 1
    SDL_CondSignal(vs.continue_read_thread)

const
  EPERM* = 1
  ENOENT* = 2
  ESRCH* = 3
  EINTR* = 4
  EIO* = 5
  ENXIO* = 6
  E2BIG* = 7
  ENOEXEC* = 8
  EBADF* = 9
  ECHILD* = 10
  EAGAIN* = 11
  ENOMEM* = 12
  EACCES* = 13
  EFAULT* = 14
  EBUSY* = 16
  EEXIST* = 17
  EXDEV* = 18
  ENODEV* = 19
  ENOTDIR* = 20
  EISDIR* = 21
  ENFILE* = 23
  EMFILE* = 24
  ENOTTY* = 25
  EFBIG* = 27
  ENOSPC* = 28
  ESPIPE* = 29
  EROFS* = 30
  EMLINK* = 31
  EPIPE* = 32
  EDOM* = 33
  EDEADLK* = 36
  ENAMETOOLONG* = 38
  ENOLCK* = 39
  ENOSYS* = 40
  ENOTEMPTY* = 41

proc stream_toggle_pause*(vs: ptr VideoState) =
  if vs.paused != 0:
    vs.frame_timer += cdouble av_gettime_relative() div 1000000 - vs.vidclk.last_updated.int64
    if vs.read_pause_return != AVERROR(ENOSYS):
      vs.vidclk.paused = 0
    set_clock(addr(vs.vidclk), get_clock(addr(vs.vidclk)), vs.vidclk.serial)
  set_clock(addr(vs.extclk), get_clock(addr(vs.extclk)), vs.extclk.serial)
  vs.paused= not vs.paused
  vs.audclk.paused= not vs.paused
  vs.vidclk.paused = not vs.paused
  vs.extclk.paused = not vs.paused

proc toggle_pause*(vs: ptr VideoState) =
  stream_toggle_pause(vs)
  vs.step = 0

proc toggle_mute*(vs: ptr VideoState) =
  vs.muted = not vs.muted

proc update_volume*(vs: ptr VideoState; sign: cint; step: cdouble) =
  var volume_level: cdouble = if vs.audio_volume != 0: (20 * ln(float64 vs.audio_volume div SDL_MIX_MAXVOLUME) / ln(10.0)) else: -1000.0
  var new_volume: cint = cint round(SDL_MIX_MAXVOLUME * pow(10.0, (volume_level + sign.cdouble * step) / 20.0))
  vs.audio_volume = av_clip_c(if vs.audio_volume == new_volume: (vs.audio_volume + sign) else: new_volume, 0, SDL_MIX_MAXVOLUME)

proc step_to_next_frame*(vs: ptr VideoState) =
  if vs.paused != 0:
    stream_toggle_pause(vs)
  vs.step = 1

proc compute_target_delay*(delay: cdouble; vs: ptr VideoState): cdouble =
  var
    sync_threshold: cdouble
    diff: cdouble = 0
  ##  update delay to follow master synchronisation source
  if get_master_sync_type(vs) != AV_SYNC_VIDEO_MASTER:
    ##  if video is slave, we try to correct big delays by duplicating or deleting a frame
    diff = get_clock(addr(vs.vidclk)) - get_master_clock(vs)
    ##  skip or repeat frame. We take into account the delay to compute the threshold. I still don't know if it is the best guess
    sync_threshold = max(AV_SYNC_THRESHOLD_MIN, min(AV_SYNC_THRESHOLD_MAX, delay))
    if diff != Nan and abs(diff) < vs.max_frame_duration:
      if diff <= -sync_threshold:
        result = max(0, delay + diff)
      elif diff >= sync_threshold and delay > AV_SYNC_FRAMEDUP_THRESHOLD:
        result = delay + diff
      elif diff >= sync_threshold:
        result = 2 * delay
  echo("video: delay=%0.3f A-V=%f", delay, -diff)
  return delay

proc vp_duration*(vs: ptr VideoState; vp: ptr Frame; nextvp: ptr Frame): cdouble =
  if vp.serial == nextvp.serial:
    var duration: cdouble = nextvp.pts - vp.pts
    if duration == Nan or duration <= 0 or duration > vs.max_frame_duration:
      return vp.duration
    else:
      return duration
  else:
    return 0.0

proc update_video_pts*(vs: ptr VideoState; pts: cdouble; pos: int64; serial: cint) =
  ##  update current video pts
  set_clock(addr(vs.vidclk), pts, serial)
  sync_clock_to_slave(addr(vs.extclk), addr(vs.vidclk))


proc video_refresh*(opaque: pointer; remaining_time: ptr cdouble) =
  var vs: ptr VideoState = cast[ptr VideoState](opaque)
  var time: cdouble
  var
    sp: ptr Frame
    sp2: ptr Frame
  if vs.paused == 0 and get_master_sync_type(vs) == AV_SYNC_EXTERNAL_CLOCK and vs.realtime != 0:
    check_external_clock_speed(vs)
  if display_disable == 0 and vs.show_mode != SHOW_MODE_VIDEO and vs.audio_st != nil:
    time = cdouble av_gettime_relative().int / 1000000
    if vs.force_refresh != 0 or vs.last_vis_time + rdftspeed < time:
      video_display(vs)
      vs.last_vis_time = time
    remaining_time[] = min(remaining_time[],vs.last_vis_time + rdftspeed - time)
  if vs.video_st != nil:
    if frame_queue_nb_remaining(addr(vs.pictq)) == 0:
      ##  nothing to do, no picture to display in the queue
    else:
      var
        last_duration: cdouble
        duration: cdouble
        delay: cdouble
      var
        vp: ptr Frame
        lastvp: ptr Frame
      ##  dequeue the picture
      lastvp = frame_queue_peek_last(addr(vs.pictq))
      vp = frame_queue_peek(addr(vs.pictq))
      if vp.serial != vs.videoq.serial:
        frame_queue_next(addr(vs.pictq))
        echo "retry"
      if lastvp.serial != vp.serial:
        vs.frame_timer = cdouble av_gettime_relative() div 1000000
      if vs.paused != 0:
        echo "display"
      last_duration = vp_duration(vs, lastvp, vp)
      delay = compute_target_delay(last_duration, vs)
      time = cdouble av_gettime_relative() div 1000000
      if time < vs.frame_timer + delay:
        remaining_time[] = min(vs.frame_timer + delay - time, remaining_time[])
        echo "display"
      vs.frame_timer += delay
      if delay > 0 and time - vs.frame_timer > AV_SYNC_THRESHOLD_MAX:
        vs.frame_timer = time
      # SDL_LockMutex(vs.pictq.mutex)
      if vp.pts != Nan:
        update_video_pts(vs, vp.pts, vp.pos, vp.serial)
      # SDL_UnlockMutex(vs.pictq.mutex)
      if frame_queue_nb_remaining(addr(vs.pictq)) > 1:
        var nextvp: ptr Frame = frame_queue_peek_next(addr(vs.pictq))
        duration = vp_duration(vs, vp, nextvp)
        if vs.step == 0 and
            (framedrop > 0 or
            (framedrop != 0 and get_master_sync_type(vs) != AV_SYNC_VIDEO_MASTER)) and
            time > vs.frame_timer + duration:
          inc(vs.frame_drops_late)
          frame_queue_next(addr(vs.pictq))
          echo "retry"
      if vs.subtitle_st != nil:
        while frame_queue_nb_remaining(addr(vs.subpq)) > 0:
          sp = frame_queue_peek(addr(vs.subpq))
          if frame_queue_nb_remaining(addr(vs.subpq)) > 1:
            sp2 = frame_queue_peek_next(addr(vs.subpq))
          else:
            sp2 = nil
          if sp.serial != vs.subtitleq.serial or
              (vs.vidclk.pts >
              (sp.pts + (cast[cfloat](sp.sub.end_display_time div 1000)))) or
              (sp2 != nil and
              vs.vidclk.pts >
              (sp2.pts + (cast[cfloat](sp2.sub.start_display_time div 1000)))):
            if sp.uploaded != 0:
              for i in 0.. sp.sub.num_rects-1:
                var sub_rect: ptr AVSubtitleRect = sp.sub.rects[i.int]
                var pixels: ptr uint8
                var
                  pitch: cint
                if SDL_LockTexture(vs.sub_texture, cast[ptr SDL_Rect](sub_rect), cast[ptr pointer](addr(pixels)), addr(pitch)) == 0:
                  for j in 0.. sub_rect.h-1:
                    pixels += pitch
                  SDL_UnlockTexture(vs.sub_texture)
            frame_queue_next(addr(vs.subpq))
          else:
            break
      frame_queue_next(addr(vs.pictq))
      vs.force_refresh = 1
      if vs.step != 0 and  vs.paused == 0:
        stream_toggle_pause(vs)
    if display_disable == 0 and vs.force_refresh != 0 and vs.show_mode == SHOW_MODE_VIDEO and vs.pictq.rindex_shown != 0:
      video_display(vs)
  vs.force_refresh = 0
  if show_status != 0:
    var buf: AVBPrint
    var last_time: int64
    var cur_time: int64
    var
      aqsize: cint
      vqsize: cint
      sqsize: cint
    var av_diff: cdouble
    cur_time = av_gettime_relative()
    if last_time == 0 or (cur_time - last_time) >= 30000:
      aqsize = 0
      vqsize = 0
      sqsize = 0
      if vs.audio_st != nil:
        aqsize = vs.audioq.size
      if vs.video_st != nil:
        vqsize = vs.videoq.size
      if vs.subtitle_st != nil:
        sqsize = vs.subtitleq.size
      av_diff = 0
      if vs.audio_st != nil and vs.video_st != nil:
        av_diff = get_clock(addr(vs.audclk)) - get_clock(addr(vs.vidclk))
      elif vs.video_st != nil:
        av_diff = get_master_clock(vs) - get_clock(addr(vs.vidclk))
      elif vs.audio_st != nil:
        av_diff = get_master_clock(vs) - get_clock(addr(vs.audclk))
      av_bprint_init(addr(buf), 0, AV_BPRINT_SIZE_AUTOMATIC)
      av_bprintf(addr(buf),
                 "%7.2f %s:%7.3f fd=%4d aq=%5dKB vq=%5dKB sq=%5dB f=%", "lld",
                 "/%", "lld", "   \c", get_master_clock(vs), if (
          vs.audio_st != nil and vs.video_st != nil): "A-V" else: (
          if vs.video_st != nil: "M-V" else: (if vs.audio_st != nil: "M-A" else: "   ")),
                 av_diff, vs.frame_drops_early + vs.frame_drops_late,
                 aqsize div 1024, vqsize div 1024, sqsize, if vs.video_st != nil: vs.viddec.avctx.pts_correction_num_faulty_dts else: 0, if vs.video_st != nil: vs.viddec.avctx.pts_correction_num_faulty_pts else: 0)
      if show_status == 1 and AV_LOG_INFO > av_log_get_level():
        echo buf.str
      else:
        echo(buf.str)
      discard av_bprint_finalize(addr(buf), nil)
      last_time = cur_time

proc queue_picture*(vs: ptr VideoState; src_frame: ptr AVFrame; pts: cdouble;duration: cdouble; pos: int64; serial: cint): cint =
  var vp: ptr Frame = frame_queue_peek_writable(addr(vs.pictq))
  if vp == nil:
    return -1
  vp.sar = src_frame.sample_aspect_ratio
  vp.uploaded = 0
  vp.width = src_frame.width
  vp.height = src_frame.height
  vp.format = src_frame.format
  vp.pts = pts
  vp.duration = duration
  vp.pos = pos
  vp.serial = serial
  set_default_window_size(vp.width, vp.height, vp.sar)
  av_frame_move_ref(vp.frame, src_frame)
  frame_queue_push(addr(vs.pictq))
  return 0

proc get_video_frame*(vs: ptr VideoState; frame: ptr AVFrame): cint =
  var got_picture: cint = decoder_decode_frame(addr(vs.viddec), frame, nil)
  if got_picture < 0:
    return -1
  if got_picture != 0:
    var dpts: cdouble = Nan
    if frame.pts != AV_NOPTS_VALUE:
      dpts = av_q2d(vs.video_st.time_base).cdouble * frame.pts.cdouble
    frame.sample_aspect_ratio = av_guess_sample_aspect_ratio(vs.ic, vs.video_st, frame)
    if framedrop > 0 or (framedrop != 0 and get_master_sync_type(vs) != AV_SYNC_VIDEO_MASTER):
      if frame.pts != AV_NOPTS_VALUE:
        var diff: cdouble = dpts - get_master_clock(vs)
        if diff != Nan and abs(diff) < AV_NOSYNC_THRESHOLD and
            diff - vs.frame_last_filter_delay < 0 and
            vs.viddec.pkt_serial == vs.vidclk.serial and
            vs.videoq.nb_packets != 0:
          inc(vs.frame_drops_early)
          av_frame_unref(frame)
          got_picture = 0
  return got_picture

proc configure_filtergraph*(graph: ptr AVFilterGraph; filtergraph: cstring;
                           source_ctx: ptr AVFilterContext;
                           sink_ctx: ptr AVFilterContext): cint =
  var
    result: cint
  var nb_filters: cint = cint graph.nb_filters
  var
    outputs: ptr AVFilterInOut = nil
    inputs: ptr AVFilterInOut = nil
  if filtergraph != nil:
    outputs = avfilter_inout_alloc()
    inputs = avfilter_inout_alloc()
    if outputs == nil or inputs == nil:
      return AVERROR(ENOMEM)
    outputs.name = av_strdup("in")
    outputs.filter_ctx = source_ctx
    outputs.pad_idx = 0
    outputs.next = nil
    inputs.name = av_strdup("out")
    inputs.filter_ctx = sink_ctx
    inputs.pad_idx = 0
    inputs.next = nil
    if avfilter_graph_parse_ptr(graph, filtergraph, addr(inputs),addr(outputs), nil) < 0:
      echo  "fail"
  else:
    if avfilter_link(source_ctx, 0, sink_ctx, 0) < 0:
       echo  "fail"
  for i in 0 .. graph.nb_filters.cint - nb_filters - 1:
      swap(graph.filters[i], graph.filters[i + nb_filters])
  result = avfilter_graph_config(graph, nil)
  avfilter_inout_free(addr(outputs))
  avfilter_inout_free(addr(inputs))
  return result

proc av_strlcatf*(dst: cstring; size: csize_t; fmt: cstring, args:varargs[auto]): csize_t 

proc snprintf(dst: cstring; size: csize_t; fmt: cstring, args:varargs[untyped]) 

proc get_rotation(st: ptr AVStream): cdouble 

template INSERT_FILT*(name, arg: untyped): void =
    var filt_ctx: ptr AVFilterContext
    avfilter_graph_create_filter(addr(filt_ctx), avfilter_get_by_name(name), name, arg, nil, graph)
    avfilter_link(filt_ctx, 0, last_filter, 0)
    last_filter = filt_ctx


proc configure_video_filters*(graph: ptr AVFilterGraph; vs: ptr VideoState;vfilters: cstring; frame: ptr AVFrame): cint =
  var pix_fmts: array[sizeof(sdl_texture_format_map), AVPixelFormat]
  var sws_flags_str: array[512, char]
  var buffersrc_args: array[256, char]
  var
    filt_src: ptr AVFilterContext = nil
    filt_out: ptr AVFilterContext = nil
    last_filter: ptr AVFilterContext = nil
  var codecpar: ptr AVCodecParameters = vs.video_st.codecpar
  var fr: AVRational = av_guess_frame_rate(vs.ic, vs.video_st, nil)
  var e: ptr AVDictionaryEntry = nil
  var nb_pix_fmts: cint = 0
  for i in 0.. renderer_info.num_texture_formats-1:
    for j in 0.. sdl_texture_format_map.high:
      if renderer_info.texture_formats[i] == sdl_texture_format_map[j].texture_fmt.uint32:
        pix_fmts[nb_pix_fmts] = sdl_texture_format_map[j].format
        nb_pix_fmts.inc
        break
  pix_fmts[nb_pix_fmts] = AV_PIX_FMT_NONE
  e = av_dict_get(sws_dict, "", e, AV_DICT_IGNORE_SUFFIX)
  while e != nil:
    if e.key == "sws_flags":
      discard av_strlcatf(sws_flags_str[0].addr, sizeof(sws_flags_str).csize_t, "%s=%s:".cstring, "flags".cstring, e.value)
    else:
      discard av_strlcatf(sws_flags_str[0].addr, sizeof(sws_flags_str).csize_t, "%s=%s:".cstring, e.key, e.value)
  if len(sws_flags_str) != 0:
    sws_flags_str[len(sws_flags_str) - 1] = '\x00'
  graph.scale_sws_opts = av_strdup(sws_flags_str[0].addr)
  snprintf(buffersrc_args[0].addr, sizeof((buffersrc_args)).csize_t,
           "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
           frame.width, frame.height, frame.format, vs.video_st.time_base.num,
           vs.video_st.time_base.den, codecpar.sample_aspect_ratio.num,
           max(codecpar.sample_aspect_ratio.den, 1.cint))
  if fr.num != 0 and fr.den != 0:
    discard av_strlcatf(buffersrc_args[0].addr, sizeof((buffersrc_args)).csize_t, ":frame_rate=%d/%d", fr.num, fr.den)
  if avfilter_graph_create_filter(addr(filt_src),avfilter_get_by_name("buffer"),"ffplay_buffer", buffersrc_args[0].addr, nil, graph) < 0:
    echo "fail"
  result = avfilter_graph_create_filter(addr(filt_out),avfilter_get_by_name("buffersink"),"ffplay_buffersink", nil, nil, graph)
  if result < 0:
    echo "fail"
  if av_opt_set_bin(filt_out, "pix_fmts", cast[ptr uint8](pix_fmts.addr), AV_PIX_FMT_NONE.cint,AV_OPT_SEARCH_CHILDREN) < 0:
    echo "fail"
  last_filter = filt_out
  ##  Note: this macro adds a filter before the lastly added filter, so the processing order of the filters is in reverse
  if autorotate != 0:
    var theta: cdouble = get_rotation(vs.video_st)
    if abs(theta - 90) < 1.0:
      INSERT_FILT("transpose", "clock".cstring)
    elif abs(theta - 180) < 1.0:
      INSERT_FILT("hflip", nil)
      INSERT_FILT("vflip", nil)
    elif abs(theta - 270) < 1.0:
      INSERT_FILT("transpose", "cclock".cstring)
    elif abs(theta) > 1.0:
      var rotate_buf: array[64, char]
      snprintf(rotate_buf[0].addr, sizeof(rotate_buf).csize_t, "%f*PI/180", theta)
      INSERT_FILT("rotate", rotate_buf[0].addr)
  result = configure_filtergraph(graph, vfilters, filt_src, last_filter) 
  vs.in_video_filter = filt_src
  vs.out_video_filter = filt_out
  return result

proc configure_audio_filters*(vs: ptr VideoState; afilters: cstring; force_output_format: cint): cint =
  var sample_fmts: seq[AVSampleFormat] = @[AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_NONE]
  var sample_rates: array[2, cint] = [0.cint, -1]
  var channel_layouts: array[2, int64] = [0'i64, -1]
  var channels: array[2, cint] = [0.cint, -1]
  var
    filt_asrc: ptr AVFilterContext = nil
    filt_asink: ptr AVFilterContext = nil
  var aresample_swr_opts: array[512, char]
  var e: ptr AVDictionaryEntry = nil
  var asrc_args: array[256, char]
  avfilter_graph_free(addr(vs.agraph))
  vs.agraph = avfilter_graph_alloc()
  if vs.agraph == nil:
    return AVERROR(ENOMEM)
  vs.agraph.nb_threads = filter_nbthreads
  e = av_dict_get(swr_opts, "", e, AV_DICT_IGNORE_SUFFIX)
  while e != nil:
    discard av_strlcatf(aresample_swr_opts[0].addr, sizeof(aresample_swr_opts).csize_t, "%s=%s:", e.key, e.value)
  if len(aresample_swr_opts) != 0:
    aresample_swr_opts[len(aresample_swr_opts) - 1] = '\x00'
  discard av_opt_set(vs.agraph, "aresample_swr_opts", aresample_swr_opts[0].addr, 0)
  snprintf(asrc_args[0].addr, sizeof(asrc_args).csize_t, "sample_rate=%d:sample_fmt=%s:channels=%d:time_base=%d/%d", vs.audio_filter_src.freq, av_get_sample_fmt_name(vs.audio_filter_src.fmt), vs.audio_filter_src.channels, 1, vs.audio_filter_src.freq)
  if vs.audio_filter_src.channel_layout != 0:
    snprintf(asrc_args[0].addr + result, sizeof(asrc_args).csize_t - result.csize_t, ":channel_layout=0x%", "llx", vs.audio_filter_src.channel_layout)
  result = avfilter_graph_create_filter(addr(filt_asrc),avfilter_get_by_name("abuffer"), "ffplay_abuffer", asrc_args[0].addr, nil, vs.agraph)
  if result < 0:
    echo "avfilter_graph_create_filter error"
  result = avfilter_graph_create_filter(addr(filt_asink), avfilter_get_by_name("abuffersink"), "ffplay_abuffersink", nil, nil, vs.agraph)
  if result < 0:
    echo "avfilter_graph_create_filter error"
  result = av_opt_set_bin(filt_asink, "sample_fmts", cast[ptr uint8](sample_fmts.addr), AV_SAMPLE_FMT_NONE.cint, AV_OPT_SEARCH_CHILDREN.cint)
  if result < 0:
    echo "avfilter_graph_create_filter error"
  result = av_opt_set_int(filt_asink, "all_channel_counts", 1, AV_OPT_SEARCH_CHILDREN.cint)
  if result < 0:
    echo "avfilter_graph_create_filter error"
  if force_output_format != 0:
    channel_layouts[0] = vs.audio_tgt.channel_layout
    channels[0] = if vs.audio_tgt.channel_layout != 0: -1 else: vs.audio_tgt.channels
    sample_rates[0] = vs.audio_tgt.freq
    result = av_opt_set_int(filt_asink, "all_channel_counts", 0, AV_OPT_SEARCH_CHILDREN.cint)
    if result < 0:
      echo "avfilter_graph_create_filter error"
    result = av_opt_set_bin(filt_asink, "channel_layouts", cast[ptr uint8](channel_layouts.addr), -1,AV_OPT_SEARCH_CHILDREN)
    if result < 0:
      echo "avfilter_graph_create_filter error"
    result = av_opt_set_bin(filt_asink, "channel_counts", cast[ptr uint8](channels.addr), -1, AV_OPT_SEARCH_CHILDREN)
    if result < 0:
      echo "avfilter_graph_create_filter error"
    result = av_opt_set_bin(filt_asink, "sample_rates", cast[ptr uint8](sample_rates.addr), -1, AV_OPT_SEARCH_CHILDREN)
    if result < 0:
      echo "avfilter_graph_create_filter error"
  result = configure_filtergraph(vs.agraph, afilters, filt_asrc, filt_asink)
  if result < 0:
    echo "avfilter_graph_create_filter error"
  vs.in_audio_filter = filt_asrc
  vs.out_audio_filter = filt_asink
  if result < 0:
    avfilter_graph_free(addr(vs.agraph))
  return result

proc audio_thread*(arg: pointer): cint =
  var vs: ptr VideoState = cast[ptr VideoState](arg)
  var frame: ptr AVFrame = av_frame_alloc()
  var af: ptr Frame
  var last_serial: cint = -1
  var dec_channel_layout: int64
  var reconfigure: bool
  var got_frame: cint = 0
  var tb: AVRational
  var result: cint = 0
  if frame == nil:
    return AVERROR(ENOMEM)
  while true:
    got_frame = decoder_decode_frame(addr(vs.auddec), frame, nil)
    if got_frame < 0:
      echo "the end"
    if got_frame != 0:
      tb = AVRational(num:1, den:frame.sample_rate)
      dec_channel_layout = int64 get_valid_channel_layout(frame.channel_layout.uint64,frame.channels)
      reconfigure = cmp_audio_fmts(vs.audio_filter_src.fmt, vs.audio_filter_src.channels, frame.format.AVSampleFormat,frame.channels)
      reconfigure = reconfigure != false or vs.audio_filter_src.channel_layout != dec_channel_layout or vs.audio_filter_src.freq != frame.sample_rate or vs.auddec.pkt_serial != last_serial
      if reconfigure:
        var
          buf1: array[1024, char]
          buf2: array[1024, char]
        av_get_channel_layout_string(buf1[0].addr, sizeof(buf1).cint, -1,vs.audio_filter_src.channel_layout.uint64)
        av_get_channel_layout_string(buf2[0].addr, sizeof(buf2).cint, -1, dec_channel_layout.uint64)
        echo("Audio frame changed from rate:%d ch:%d fmt:%s layout:%s serial:%d to rate:%d ch:%d fmt:%s layout:%s serial:%d",
               vs.audio_filter_src.freq, vs.audio_filter_src.channels,
               av_get_sample_fmt_name(vs.audio_filter_src.fmt), buf1,
               last_serial, frame.sample_rate, frame.channels,
               av_get_sample_fmt_name(frame.format.AVSampleFormat), buf2, vs.auddec.pkt_serial)
        vs.audio_filter_src.fmt = frame.format.AVSampleFormat
        vs.audio_filter_src.channels = frame.channels
        vs.audio_filter_src.channel_layout = dec_channel_layout
        vs.audio_filter_src.freq = frame.sample_rate
        last_serial = vs.auddec.pkt_serial
        result = configure_audio_filters(vs, afilters, 1)
        if result < 0:
          echo "the_end"
      result = av_buffersrc_add_frame(vs.in_audio_filter, frame)
      if result < 0:
        echo "the_end"
      result = av_buffersink_get_frame_flags(vs.out_audio_filter, frame, 0)
      while result >= 0:
        tb = av_buffersink_get_time_base(vs.out_audio_filter)
        af = frame_queue_peek_writable(addr(vs.sampq))
        if af == nil:
          echo "the_end"
        af.pts =  if frame.pts == AV_NOPTS_VALUE: Nan else: cdouble frame.pts * av_q2d(tb)
        af.pos = frame.pkt_pos
        af.serial = vs.auddec.pkt_serial
        af.duration = cdouble av_q2d(AVRational(num:frame.nb_samples, den:frame.sample_rate));
        av_frame_move_ref(af.frame, frame)
        frame_queue_push(addr(vs.sampq))
        if vs.audioq.serial != vs.auddec.pkt_serial:
          break
      if result == AVERROR_EOF:
        vs.auddec.finished = vs.auddec.pkt_serial
    if not (result >= 0 or result == AVERROR(EAGAIN) or result == AVERROR_EOF):
      break
  avfilter_graph_free(addr(vs.agraph))
  av_frame_free(addr(frame))
  return result

proc decoder_start*(d: ptr Decoder; fn: proc (a1: pointer):cint; thread_name: cstring; arg: pointer): cint =
  packet_queue_start(d.queue)
  d.decoder_tid = SDL_CreateThread(fn, thread_name, arg)

proc video_thread*(arg: pointer): cint =
  var vs: ptr VideoState = cast[ptr VideoState](arg)
  var frame: ptr AVFrame = av_frame_alloc()
  var pts: cdouble
  var duration: cdouble
  var result: cint
  var tb: AVRational = vs.video_st.time_base
  var frame_rate: AVRational = av_guess_frame_rate(vs.ic, vs.video_st, nil)
  var graph: ptr AVFilterGraph
  var
    filt_out: ptr AVFilterContext
    filt_in: ptr AVFilterContext
  var last_w: cint = 0
  var last_h: cint = 0
  var last_format = -2
  var last_serial: cint = -1
  var last_vfilter_idx: cint = 0
  if frame == nil:
    return AVERROR(ENOMEM)
  while true:
    result = get_video_frame(vs, frame)
    if result < 0:
      echo "the_end"
    if result == 0:
      continue
    if last_w != frame.width or last_h != frame.height or last_format.cint != frame.format or
        last_serial != vs.viddec.pkt_serial or
        last_vfilter_idx != vs.vfilter_idx:
      echo("Video frame changed from size:%dx%d format:%s serial:%d to size:%dx%d format:%s serial:%d",
             last_w, last_h, cast[cstring](av_x_if_null(av_get_pix_fmt_name(last_format.AVPixelFormat), "none".cstring)), last_serial, frame.width,
             frame.height, cast[cstring](av_x_if_null(av_get_pix_fmt_name(frame.format.AVPixelFormat), "none".cstring)), vs.viddec.pkt_serial)
      avfilter_graph_free(addr(graph))
      graph = avfilter_graph_alloc()
      if graph == nil:
        result = AVERROR(ENOMEM)
        echo "the_end"
      graph.nb_threads = filter_nbthreads
      result = configure_video_filters(graph, vs, if vfilters_list != nil: vfilters_list[vs.vfilter_idx] else: nil, frame)
      if result < 0:
        var event: SDL_Event
        event.type = FF_QUIT_EVENT
        event.user.data1 = vs
        discard SDL_PushEvent(addr(event))
        echo "the_end"
      filt_in = vs.in_video_filter
      filt_out = vs.out_video_filter
      last_w = frame.width
      last_h = frame.height
      last_format = frame.format
      last_serial = vs.viddec.pkt_serial
      last_vfilter_idx = vs.vfilter_idx
      frame_rate = av_buffersink_get_frame_rate(filt_out)
    result = av_buffersrc_add_frame(filt_in, frame)
    if result < 0:
      echo "the_end"
    while result >= 0:
      vs.frame_last_returned_time = av_gettime_relative().float64 / 1000000.0
      result = av_buffersink_get_frame_flags(filt_out, frame, 0)
      if result < 0:
        if result == AVERROR_EOF:
          vs.viddec.finished = vs.viddec.pkt_serial
        result = 0
        break
      vs.frame_last_filter_delay = av_gettime_relative().float64 / 1000000.0 -
          vs.frame_last_returned_time
      if abs(vs.frame_last_filter_delay) > AV_NOSYNC_THRESHOLD / 10.0:
        vs.frame_last_filter_delay = 0
      tb = av_buffersink_get_time_base(filt_out)
      duration =  if frame_rate.num != 0 and frame_rate.den != 0 : cdouble av_q2d(AVRational(num:frame_rate.den, den:frame_rate.num)) else: 0
      pts =  if (frame.pts == AV_NOPTS_VALUE): Nan else: cdouble frame.pts * av_q2d(tb)
      result = queue_picture(vs, frame, pts, duration, frame.pkt_pos, vs.viddec.pkt_serial)
      av_frame_unref(frame)
      if vs.videoq.serial != vs.viddec.pkt_serial:
        break
    if result < 0:
      echo "the_end"
  avfilter_graph_free(addr(graph))
  av_frame_free(addr(frame))
  return 0

proc subtitle_thread*(arg: pointer): cint =
  var vs: ptr VideoState = cast[ptr VideoState](arg)
  var sp: ptr Frame
  var got_subtitle: cint
  var pts: cdouble
  while true:
    sp = frame_queue_peek_writable(addr(vs.subpq))
    if sp == nil:
      return 0
    got_subtitle = decoder_decode_frame(addr(vs.subdec), nil, addr(sp.sub))
    if got_subtitle < 0:
      break
    pts = 0
    if got_subtitle != 0 and sp.sub.format == 0:
      if sp.sub.pts != AV_NOPTS_VALUE:
        pts = sp.sub.pts.cdouble / cast[cdouble](AV_TIME_BASE)
      sp.pts = pts
      sp.serial = vs.subdec.pkt_serial
      sp.width = vs.subdec.avctx.width
      sp.height = vs.subdec.avctx.height
      sp.uploaded = 0
      frame_queue_push(addr(vs.subpq))
    elif got_subtitle != 0:
      avsubtitle_free(addr(sp.sub))
  return 0



##  return the wanted number of samples to get better sync if sync_type is video
##  or external master clock

proc synchronize_audio*(vs: ptr VideoState; nb_samples: cint): cint =
  var wanted_nb_samples: cint = nb_samples
  ##  if not master, then we try to remove or add samples to correct the clock
  if get_master_sync_type(vs) != AV_SYNC_AUDIO_MASTER:
    var
      diff: cdouble
      avg_diff: cdouble
    var
      min_nb_samples: cint
      max_nb_samples: cint
    diff = get_clock(addr(vs.audclk)) - get_master_clock(vs)
    if diff != Nan and abs(diff) < AV_NOSYNC_THRESHOLD:
      vs.audio_diff_cum = diff + vs.audio_diff_avg_coef * vs.audio_diff_cum
      if vs.audio_diff_avg_count < AUDIO_DIFF_AVG_NB:
        ##  not enough measures to have a correct estimate
        inc(vs.audio_diff_avg_count)
      else:
        ##  estimate the A-V difference
        avg_diff = vs.audio_diff_cum * (1.0 - vs.audio_diff_avg_coef)
        if abs(avg_diff) >= vs.audio_diff_threshold:
          wanted_nb_samples = cint nb_samples + (int)(diff * vs.audio_src.freq.cdouble)
          min_nb_samples = ((nb_samples * (100 - SAMPLE_CORRECTION_PERCENT_MAX) div 100))
          max_nb_samples = ((nb_samples * (100 + SAMPLE_CORRECTION_PERCENT_MAX) div 100))
          wanted_nb_samples = av_clip_c(wanted_nb_samples, min_nb_samples,
                                    max_nb_samples)
        echo("diff=%f adiff=%f sample_diff=%d apts=%0.3f %f", diff, avg_diff,
               wanted_nb_samples - nb_samples, vs.audio_clock,
               vs.audio_diff_threshold)
    else:
      ##  too big difference : may be initial PTS errors, so reset A-V filter
      vs.audio_diff_avg_count = 0
      vs.audio_diff_cum = 0
  return wanted_nb_samples

## *
##  Decode one audio frame and return its uncompressed size.
##
##  The processed audio frame is decoded, converted if required, and
##  stored in is.audio_buf, with size in bytes given by the return
##  value.
##


proc audio_decode_frame*(vs: ptr VideoState): cint =
  var
    data_size: cint
    resampled_data_size: cint
  var dec_channel_layout: int64
  var audio_clock0: cdouble
  var wanted_nb_samples: cint
  var af: ptr Frame
  if vs.paused != 0:
    return -1
  while true:
    while frame_queue_nb_remaining(addr(vs.sampq)) == 0:
      if (av_gettime_relative() - audio_callback_time) >
          1000000 * vs.audio_hw_buf_size div vs.audio_tgt.bytes_per_sec div 2:
        return -1
      discard av_usleep(1000)
    af = frame_queue_peek_readable(addr(vs.sampq))
    if af == nil:
      return -1
    frame_queue_next(addr(vs.sampq))
    if af.serial == vs.audioq.serial:
      break
  data_size = av_samples_get_buffer_size(nil, af.frame.channels,
                                       af.frame.nb_samples, af.frame.format.AVSampleFormat, 1)
  dec_channel_layout = if (af.frame.channel_layout != 0 and af.frame.channels == av_get_channel_layout_nb_channels(af.frame.channel_layout)): af.frame.channel_layout.int64 else: av_get_default_channel_layout(af.frame.channels)
  wanted_nb_samples = synchronize_audio(vs, af.frame.nb_samples)
  if af.frame.format != vs.audio_src.fmt.cint or dec_channel_layout != vs.audio_src.channel_layout or af.frame.sample_rate != vs.audio_src.freq or (wanted_nb_samples != af.frame.nb_samples and vs.swr_ctx == nil):
    swr_free(addr(vs.swr_ctx))
    vs.swr_ctx = swr_alloc_set_opts(nil, vs.audio_tgt.channel_layout,
                                    vs.audio_tgt.fmt, vs.audio_tgt.freq,
                                    dec_channel_layout, af.frame.format.AVSampleFormat,
                                    af.frame.sample_rate, 0, nil)
    if vs.swr_ctx == nil or swr_init(vs.swr_ctx) < 0:
      echo("Cannot create sample rate converter for conversion of %d Hz %s %d channels to %d Hz %s %d channels!",
             af.frame.sample_rate, av_get_sample_fmt_name(af.frame.format.AVSampleFormat),
             af.frame.channels, vs.audio_tgt.freq,
             av_get_sample_fmt_name(vs.audio_tgt.fmt), vs.audio_tgt.channels)
      swr_free(addr(vs.swr_ctx))
      return -1
    vs.audio_src.channel_layout = dec_channel_layout
    vs.audio_src.channels = af.frame.channels
    vs.audio_src.freq = af.frame.sample_rate
    vs.audio_src.fmt = af.frame.format.AVSampleFormat
  if vs.swr_ctx != nil:
    var `in`: ptr ptr uint8 = cast[ptr ptr uint8](af.frame.extended_data)
    var `out`: ptr ptr uint8 = addr(vs.audio_buf1)
    var out_count: cint = cast[cint](wanted_nb_samples * vs.audio_tgt.freq div af.frame.sample_rate) + 256
    var out_size: cint = av_samples_get_buffer_size(nil, vs.audio_tgt.channels,
        out_count, vs.audio_tgt.fmt, 0)
    var len2: cint
    if out_size < 0:
      echo("av_samples_get_buffer_size() failed")
      return -1
    if wanted_nb_samples != af.frame.nb_samples:
      if swr_set_compensation(vs.swr_ctx, (
          wanted_nb_samples - af.frame.nb_samples) * vs.audio_tgt.freq div
          af.frame.sample_rate, wanted_nb_samples * vs.audio_tgt.freq div
          af.frame.sample_rate) < 0:
        echo("swr_set_compensation() failed")
        return -1
    av_fast_malloc(addr(vs.audio_buf1), addr(vs.audio_buf1_size), out_size.csize_t)
    if vs.audio_buf1 == nil:
      return AVERROR(ENOMEM)
    len2 = swr_convert(vs.swr_ctx, `out`, out_count, `in`, af.frame.nb_samples)
    if len2 < 0:
      echo("swr_convert() failed")
      return -1
    if len2 == out_count:
      echo("audio buffer is probably too small")
      if swr_init(vs.swr_ctx) < 0:
        swr_free(addr(vs.swr_ctx))
    vs.audio_buf = vs.audio_buf1
    resampled_data_size = len2 * vs.audio_tgt.channels * av_get_bytes_per_sample(vs.audio_tgt.fmt)
  else:
    vs.audio_buf = af.frame.data[0]
    resampled_data_size = data_size
  audio_clock0 = vs.audio_clock
  ##  update the audio clock with the pts
  if af.pts != Nan:
    vs.audio_clock = af.pts + cast[cdouble](af.frame.nb_samples div af.frame.sample_rate)
  else:
    vs.audio_clock = Nan
  vs.audio_clock_serial = af.serial
  return resampled_data_size



proc sdl_audio_callback*(opaque: pointer; stream:var ptr uint8; len: var cint) =
  var vs: ptr VideoState = cast[ptr VideoState](opaque)
  var
    audio_size: cint
    len1: cint
  audio_callback_time = av_gettime_relative()
  while len > 0:
    if vs.audio_buf_index.cuint >= vs.audio_buf_size:
      audio_size = audio_decode_frame(vs)
      if audio_size < 0:
        vs.audio_buf = nil
        vs.audio_buf_size = cuint SDL_AUDIO_MIN_BUFFER_SIZE div vs.audio_tgt.frame_size * vs.audio_tgt.frame_size
      else:
        if vs.show_mode != SHOW_MODE_VIDEO:
          var
            size: cint
            len: cint
          size = cint audio_size div sizeof(cshort)
          while size > 0:
            len = SAMPLE_ARRAY_SIZE - vs.sample_array_index
            if len > size:
              len = size
            copyMem(vs.sample_array[vs.sample_array_index].addr, cast[ptr int16](vs.audio_buf),len * sizeof((cshort)))
            vs.audio_buf += len * 2
            vs.sample_array_index += len
            if vs.sample_array_index >= SAMPLE_ARRAY_SIZE:
              vs.sample_array_index = 0
            size -= len
        vs.audio_buf_size = cuint audio_size
      vs.audio_buf_index = 0
    len1 = vs.audio_buf_size.cint - vs.audio_buf_index
    if len1 > len:
      len1 = len
    if vs.muted == 0 and vs.audio_buf != nil and vs.audio_volume == SDL_MIX_MAXVOLUME:
      copyMem(stream, vs.audio_buf + vs.audio_buf_index, len1)
    else:
      if vs.muted == 0 and vs.audio_buf != nil:
        SDL_MixAudioFormat(stream, vs.audio_buf + vs.audio_buf_index, AUDIO_S16SYS, len1.uint32, vs.audio_volume)
    len = len - len1
    stream += len1
    vs.audio_buf_index += len1
  vs.audio_write_buf_size = vs.audio_buf_size.cint - vs.audio_buf_index
  ##  Let's assume the audio driver that is used by SDL has two periods.
  if vs.audio_clock != Nan:
    set_clock_at(addr(vs.audclk), vs.audio_clock - (cdouble)(2 * vs.audio_hw_buf_size + vs.audio_write_buf_size) / vs.audio_tgt.bytes_per_sec.cdouble, vs.audio_clock_serial, audio_callback_time.float64 / 1000000.0)
    sync_clock_to_slave(addr(vs.extclk), addr(vs.audclk))



proc audio_open*(opaque: pointer; wanted_channel_layout: var int64;
                wanted_nb_channels: var cint; wanted_sample_rate: cint;
                audio_hw_params: ptr AudioParams): cint =
  var
    wanted_spec: SDL_AudioSpec
    spec: SDL_AudioSpec
  var env: cstring
  var next_nb_channels: seq[cint] = @[0.cint, 0, 1, 6, 2, 6, 4, 6]
  var next_sample_rates: seq[cint] = @[0.cint, 44100, 48000, 96000, 192000]
  var next_sample_rate_idx: cint = sizeof(next_sample_rates) - 1
  env = SDL_getenv("SDL_AUDIO_CHANNELS")
  if env != nil:
    wanted_nb_channels = cint parseInt($env)
    wanted_channel_layout = av_get_default_channel_layout(wanted_nb_channels)
  if wanted_channel_layout == 0 or wanted_nb_channels != av_get_channel_layout_nb_channels(wanted_channel_layout.uint64):
    wanted_channel_layout = av_get_default_channel_layout(wanted_nb_channels)
    wanted_channel_layout = wanted_channel_layout and not AV_CH_LAYOUT_STEREO_DOWNMIX
  wanted_nb_channels = av_get_channel_layout_nb_channels(wanted_channel_layout.uint64)
  wanted_spec.channels = uint8 wanted_nb_channels
  wanted_spec.freq = wanted_sample_rate
  if wanted_spec.freq <= 0 or wanted_spec.channels <= 0:
    echo("Invalid sample rate or channel count!")
    return -1
  while next_sample_rate_idx != 0 and next_sample_rates[next_sample_rate_idx] >= wanted_spec.freq:
    next_sample_rate_idx.dec
  wanted_spec.format = AUDIO_S16SYS
  wanted_spec.silence = 0
  wanted_spec.samples = uint16 max(SDL_AUDIO_MIN_BUFFER_SIZE, 2 shl av_log2(cuint wanted_spec.freq div SDL_AUDIO_MAX_CALLBACKS_PER_SEC))
  # wanted_spec.callback = sdl_audio_callback
  wanted_spec.userdata = opaque
  audio_dev = SDL_OpenAudioDevice(nil, 0, addr(wanted_spec), addr(spec), SDL_AUDIO_ALLOW_FREQUENCY_CHANGE or SDL_AUDIO_ALLOW_CHANNELS_CHANGE)
  while audio_dev == 0:
    # echo("SDL_OpenAudio (%d channels, %d Hz): %s", wanted_spec.channels, wanted_spec.freq, SDL_GetError())
    wanted_spec.channels = uint8 next_nb_channels[min(7.uint8, wanted_spec.channels)]
    if wanted_spec.channels == 0:
      wanted_spec.freq = next_sample_rates[next_sample_rate_idx]
      next_sample_rate_idx.dec
      wanted_spec.channels = uint8 wanted_nb_channels
      if wanted_spec.freq == 0:
        echo("No more combinations to try, audio open failed")
        return -1
    wanted_channel_layout = av_get_default_channel_layout(wanted_spec.channels.cint)
  if spec.format != AUDIO_S16SYS:
    echo("SDL advised audio format %d is not supported!", spec.format)
    return -1
  if spec.channels != wanted_spec.channels:
    wanted_channel_layout = av_get_default_channel_layout(spec.channels.cint)
    if wanted_channel_layout != 0:
      echo("SDL advised channel count %d is not supported!", spec.channels)
      return -1
  audio_hw_params.fmt = AV_SAMPLE_FMT_S16
  audio_hw_params.freq = spec.freq
  audio_hw_params.channel_layout = wanted_channel_layout
  audio_hw_params.channels = cint spec.channels
  audio_hw_params.frame_size = av_samples_get_buffer_size(nil, audio_hw_params.channels, 1, audio_hw_params.fmt, 1)
  audio_hw_params.bytes_per_sec = av_samples_get_buffer_size(nil, audio_hw_params.channels, audio_hw_params.freq, audio_hw_params.fmt, 1)
  if audio_hw_params.bytes_per_sec <= 0 or audio_hw_params.frame_size <= 0:
    echo("av_samples_get_buffer_size failed")
    return -1
  return spec.size.cint


proc stream_component_open*(vs: ptr VideoState; stream_index: cint): cint {.discardable.} =
  var ic: ptr AVFormatContext = vs.ic
  var avctx: ptr AVCodecContext
  var codec: ptr AVCodec
  var forced_codec_name: cstring
  var opts: ptr AVDictionary
  var t: ptr AVDictionaryEntry
  var
    sample_rate: cint
    nb_channels: cint
  var channel_layout: int64
  var result: cint = 0
  var stream_lowres: cint = lowres
  if stream_index < 0.cint or stream_index >= ic.nb_streams.cint:
    return -1
  avctx = avcodec_alloc_context3(nil)
  result = avcodec_parameters_to_context(avctx, ic.streams[stream_index].codecpar)
  if result < 0:
    echo "fail avcodec_parameters_to_context"
  avctx.pkt_timebase = ic.streams[stream_index].time_base
  codec = avcodec_find_decoder(avctx.codec_id)
  case avctx.codec_type
  of AVMEDIA_TYPE_AUDIO:
    vs.last_audio_stream = stream_index
    forced_codec_name = audio_codec_name
  of AVMEDIA_TYPE_SUBTITLE:
    vs.last_subtitle_stream = stream_index
    forced_codec_name = subtitle_codec_name
  of AVMEDIA_TYPE_VIDEO:
    vs.last_video_stream = stream_index
    forced_codec_name = video_codec_name
  else:discard
  if forced_codec_name != nil:
    codec = avcodec_find_decoder_by_name(forced_codec_name)
  if codec == nil:
    if forced_codec_name != nil:
      echo("No codec could be found with name \'%s\'",forced_codec_name)
    else:
      echo("No decoder could be found for codec %s", avcodec_get_name(avctx.codec_id))
    result = AVERROR(EINVAL)
    echo "fail"
  avctx.codec_id = codec.id
  if stream_lowres > codec.max_lowres.cint:
    echo("The maximum value for lowres supported by the decoder is %d",codec.max_lowres)
    stream_lowres = cint codec.max_lowres
  avctx.lowres = stream_lowres
  if fast != 0:
    avctx.flags2 = avctx.flags2 or AV_CODEC_FLAG2_FAST
  opts = filter_codec_opts(codec_opts, avctx.codec_id, ic, ic.streams[stream_index],codec)
  if av_dict_get(opts, "threads", nil, 0) == nil:
    av_dict_set(addr(opts), "threads", "auto", 0)
  if stream_lowres != 0:
    av_dict_set_int(addr(opts), "lowres", stream_lowres, 0)
  if avctx.codec_type == AVMEDIA_TYPE_VIDEO or avctx.codec_type == AVMEDIA_TYPE_AUDIO:
    av_dict_set(addr(opts), "refcounted_frames", "1", 0)
  result = avcodec_open2(avctx, codec, addr(opts))
  t = av_dict_get(opts, "", nil, AV_DICT_IGNORE_SUFFIX)

  vs.eof = 0
  ic.streams[stream_index].`discard` = AVDISCARD_DEFAULT
  case avctx.codec_type
  of AVMEDIA_TYPE_AUDIO:
    var sink: ptr AVFilterContext
    vs.audio_filter_src.freq = avctx.sample_rate
    vs.audio_filter_src.channels = avctx.channels
    vs.audio_filter_src.channel_layout = int64 get_valid_channel_layout(avctx.channel_layout, avctx.channels)
    vs.audio_filter_src.fmt = avctx.sample_fmt
    result = configure_audio_filters(vs, afilters, 0)
    sink = vs.out_audio_filter
    sample_rate = av_buffersink_get_sample_rate(sink)
    nb_channels = av_buffersink_get_channels(sink)
    channel_layout = int64 av_buffersink_get_channel_layout(sink)
    ##  prepare audio output
    result = audio_open(vs, channel_layout, nb_channels, sample_rate,addr(vs.audio_tgt))
    vs.audio_hw_buf_size = result
    vs.audio_src = vs.audio_tgt
    vs.audio_buf_size = 0
    vs.audio_buf_index = 0
    ##  init averaging filter
    vs.audio_diff_avg_coef = exp(ln(0.01) / AUDIO_DIFF_AVG_NB.float64)
    vs.audio_diff_avg_count = 0
 
    vs.audio_diff_threshold = (cdouble)(vs.audio_hw_buf_size) / vs.audio_tgt.bytes_per_sec.cdouble
    vs.audio_stream = stream_index
    vs.audio_st = ic.streams[stream_index]
    decoder_init(addr(vs.auddec), avctx, addr(vs.audioq),vs.continue_read_thread)
    if (vs.ic.iformat.flags != 0 and (AVFMT_NOBINSEARCH or AVFMT_NOGENSEARCH or AVFMT_NO_BYTE_SEEK) != 0) and vs.ic.iformat.read_seek == nil:
      vs.auddec.start_pts = vs.audio_st.start_time
      vs.auddec.start_pts_tb = vs.audio_st.time_base
    result = decoder_start(addr(vs.auddec), audio_thread, "audio_decoder", vs)
    SDL_PauseAudioDevice(audio_dev, 0)
  of AVMEDIA_TYPE_VIDEO:
    vs.video_stream = stream_index
    vs.video_st = ic.streams[stream_index]
    decoder_init(addr(vs.viddec), avctx, addr(vs.videoq),vs.continue_read_thread)
    result = decoder_start(addr(vs.viddec), video_thread, "video_decoder", vs)
    vs.queue_attachments_req = 1
  of AVMEDIA_TYPE_SUBTITLE:
    vs.subtitle_stream = stream_index
    vs.subtitle_st = ic.streams[stream_index]
    decoder_init(addr(vs.subdec), avctx, addr(vs.subtitleq),vs.continue_read_thread)
    result = decoder_start(addr(vs.subdec), subtitle_thread, "subtitle_decoder",vs)
  else:
    discard
  avcodec_free_context(addr(avctx))
  av_dict_free(addr(opts))
  return result

proc decode_interrupt_cb*(ctx: pointer): cint =
  var vs: ptr VideoState = cast[ptr VideoState](ctx)
  return vs.abort_request

proc stream_has_enough_packets*(st: ptr AVStream; stream_id: cint;queue: ptr PacketQueue): auto =
  return stream_id < 0 or queue.abort_request != 0 or (st.disposition and AV_DISPOSITION_ATTACHED_PIC) != 0 or queue.nb_packets > MIN_FRAMES and ( queue.duration == 0 or float64(av_q2d(st.time_base) * queue.duration) > 1.0)


proc strcmp(a:cstring,b:cstring): int {.importc.}
proc strncmp(a:cstring,b:cstring,l: cint): int {.importc.}

proc is_realtime*(s: ptr AVFormatContext): cint =
  if strcmp(s.iformat.name, "rtp") == 0 or strcmp(s.iformat.name, "rtsp") == 0 or strcmp(s.iformat.name, "sdp") == 0:
    return 1
  if s.pb != nil and (strncmp(s.url, "rtp:", 4) == 0 or strncmp(s.url, "udp:", 4) == 0):
    return 1
  return 0

##  this thread gets the stream from the disk or the network
proc read_thread*(arg: pointer): cint =
  var vs: ptr VideoState = cast[ptr VideoState](arg)
  var ic: ptr AVFormatContext 
  var
    err: cint
  var st_index: array[AVMEDIA_TYPE_NB.int, cint]
  var
    pkt1: AVPacket
    pkt: ptr AVPacket = addr(pkt1)
  var stream_start_time: int64
  var pkt_in_play_range: cint = 0
  var t: ptr AVDictionaryEntry
  var wait_mutex: ptr SDL_mutex = SDL_CreateMutex()
  var scan_all_pmts_set: cint = 0
  var pkt_ts: int64
  vs.eof = 0
  ic = avformat_alloc_context()
  ic.interrupt_callback.callback = decode_interrupt_cb
  ic.interrupt_callback.opaque = vs
  if av_dict_get(format_opts, "scan_all_pmts", nil, AV_DICT_MATCH_CASE) == nil:
    av_dict_set(addr(format_opts), "scan_all_pmts", "1", AV_DICT_DONT_OVERWRITE)
    scan_all_pmts_set = 1
  err = avformat_open_input(addr(ic), vs.filename, vs.iformat, addr(format_opts))
  if err < 0:
    print_error(vs.filename, err)
    result = -1
    echo "fail"
  if scan_all_pmts_set != 0:
    av_dict_set(addr(format_opts), "scan_all_pmts", nil, AV_DICT_MATCH_CASE)
  t = av_dict_get(format_opts, "", nil, AV_DICT_IGNORE_SUFFIX)
  if t != nil:
    echo("Option %s not found.", t.key)
    result = AVERROR_OPTION_NOT_FOUND
  vs.ic = ic
  if genpts != 0:
    ic.flags = ic.flags or AVFMT_FLAG_GENPTS
  av_format_inject_global_side_data(ic)
  if find_stream_info != 0:
    var opts: ptr ptr AVDictionary = setup_find_stream_info_opts(ic, codec_opts)
    var orig_nb_streams: cint = cint ic.nb_streams
    err = avformat_find_stream_info(ic, opts)
    for i in 0..orig_nb_streams.int-1:
      av_dict_free(addr(opts[i]))
    av_freep(addr(opts))
    if err < 0:
      echo("%s: could not find codec parameters",vs.filename)
      result = -1
  if ic.pb != nil:
    ic.pb.eof_reached = 0
  if seek_by_bytes < 0:
    var temp = not not(ic.iformat.flags and AVFMT_TS_DISCONT)
    var temp2 = cint temp != 0 and strcmp("ogg", ic.iformat.name) == 0
    seek_by_bytes = temp2
  vs.max_frame_duration = if (ic.iformat.flags and AVFMT_TS_DISCONT) != 0: 10.0 else: 3600.0
  t = av_dict_get(ic.metadata, "title", nil, 0)
  if window_title == nil and t != nil:
    window_title = av_asprintf("%s - %s", t.value, input_filename)
  if start_time != AV_NOPTS_VALUE:
    var timestamp: int64
    timestamp = start_time
    ##  add the stream start time
    if ic.start_time != AV_NOPTS_VALUE:
      inc(timestamp, ic.start_time)
    result = avformat_seek_file(ic, -1, int64.low, timestamp, int64.high, 0)
    if result < 0:
      echo("%s: could not seek to position %0.3f",vs.filename, cast[cdouble](timestamp div AV_TIME_BASE))
  vs.realtime = is_realtime(ic)
  if show_status != 0:
    av_dump_format(ic, 0, vs.filename, 0)
  for i in 0..ic.nb_streams.int-1:
    var st: ptr AVStream = ic.streams[i]
    var t: int = int st.codecpar.codec_type
    st.`discard` = AVDISCARD_ALL
    if t >= 0 and wanted_stream_spec[t] != nil and st_index[t] == -1:
      if avformat_match_stream_specifier(ic, st, wanted_stream_spec[t]) > 0:
        st_index[t] = cint i

  for i in 0..AVMEDIA_TYPE_NB.int-1:
    if wanted_stream_spec[i] != nil and st_index[i] == -1:
      echo("Stream specifier %s does not match any %s stream",wanted_stream_spec[i], av_get_media_type_string(i))
      st_index[i] = cint int.high
  if video_disable == 0:
    st_index[AVMEDIA_TYPE_VIDEO.int] = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO,st_index[AVMEDIA_TYPE_VIDEO.int], -1, nil, 0)
  if audio_disable == 0:
    st_index[AVMEDIA_TYPE_AUDIO.int] = av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO,st_index[AVMEDIA_TYPE_AUDIO.int], st_index[AVMEDIA_TYPE_VIDEO.int], nil, 0)
  if video_disable == 0 and subtitle_disable == 0:
    st_index[AVMEDIA_TYPE_SUBTITLE.int] = av_find_best_stream(ic, AVMEDIA_TYPE_SUBTITLE, st_index[AVMEDIA_TYPE_SUBTITLE.int], (if st_index[AVMEDIA_TYPE_AUDIO.int] >= 0: st_index[AVMEDIA_TYPE_AUDIO.int] else: st_index[AVMEDIA_TYPE_VIDEO.int]), nil, 0)
  vs.show_mode = show_mode
  if st_index[AVMEDIA_TYPE_VIDEO.int] >= 0:
    var st: ptr AVStream = ic.streams[st_index[AVMEDIA_TYPE_VIDEO.int]]
    var codecpar: ptr AVCodecParameters = st.codecpar
    var sar: AVRational = av_guess_sample_aspect_ratio(ic, st, nil)
    if codecpar.width != 0:
      set_default_window_size(codecpar.width, codecpar.height, sar)
  if st_index[AVMEDIA_TYPE_AUDIO.int] >= 0:
    stream_component_open(vs, st_index[AVMEDIA_TYPE_AUDIO.int])
  result = -1
  if st_index[AVMEDIA_TYPE_VIDEO.int] >= 0:
    result = stream_component_open(vs, st_index[AVMEDIA_TYPE_VIDEO.int])
  if vs.show_mode == SHOW_MODE_NONE:
    vs.show_mode = if result >= 0: SHOW_MODE_VIDEO else: SHOW_MODE_RDFT
  if st_index[AVMEDIA_TYPE_SUBTITLE.int] >= 0:
    stream_component_open(vs, st_index[AVMEDIA_TYPE_SUBTITLE.int])
  if vs.video_stream < 0 and vs.audio_stream < 0:
    echo("Failed to open file \'%s\' or configure filtergraph", vs.filename)
    result = -1
  if infinite_buffer < 0 and vs.realtime != 0:
    infinite_buffer = 1
  while true:
    if vs.abort_request != 0:
      break
    if vs.paused != vs.last_paused:
      vs.last_paused = vs.paused
      if vs.paused != 0:
        vs.read_pause_return = av_read_pause(ic)
      else:
        av_read_play(ic)
    if vs.paused != 0 and
        (strcmp(ic.iformat.name, "rtsp") == 0 or
        (ic.pb != nil and strncmp(input_filename, "mmsh:", 5) == 0)):
      SDL_Delay(10)
      continue
    if vs.seek_req != 0:
      var seek_target: int64 = vs.seek_pos
      var seek_min: int64 = if vs.seek_rel > 0: seek_target - vs.seek_rel + 2 else: int64.low
      var seek_max: int64 = if vs.seek_rel < 0: seek_target - vs.seek_rel - 2 else: int64.high
      ##  FIXME the +-2 is due to rounding being not done in the correct direction in generation
      ##       of the seek_pos/seek_rel variables
      result = avformat_seek_file(vs.ic, -1, seek_min, seek_target, seek_max, vs.seek_flags)
      if result < 0:
        echo("%s: error while seeking", vs.ic.url)
      else:
        if vs.audio_stream >= 0:
          packet_queue_flush(addr(vs.audioq))
          packet_queue_put(addr(vs.audioq), addr(flush_pkt))
        if vs.subtitle_stream >= 0:
          packet_queue_flush(addr(vs.subtitleq))
          packet_queue_put(addr(vs.subtitleq), addr(flush_pkt))
        if vs.video_stream >= 0:
          packet_queue_flush(addr(vs.videoq))
          packet_queue_put(addr(vs.videoq), addr(flush_pkt))
        if (vs.seek_flags and AVSEEK_FLAG_BYTE) != 0 :
          set_clock(addr(vs.extclk), Nan, 0)
        else:
          set_clock(addr(vs.extclk), cdouble seek_target div AV_TIME_BASE, 0)
      vs.seek_req = 0
      vs.queue_attachments_req = 1
      vs.eof = 0
      if vs.paused != 0:
        step_to_next_frame(vs)
    if vs.queue_attachments_req != 0:
      if vs.video_st != nil and
          (vs.video_st.disposition and AV_DISPOSITION_ATTACHED_PIC) != 0:
        var copy: AVPacket
        result = av_packet_ref(addr(copy), addr(vs.video_st.attached_pic))
        packet_queue_put(addr(vs.videoq), addr(copy))
        packet_queue_put_nullpacket(addr(vs.videoq), vs.video_stream)
      vs.queue_attachments_req = 0
    if infinite_buffer < 1 and
        (vs.audioq.size + vs.videoq.size + vs.subtitleq.size > MAX_QUEUE_SIZE or
        (stream_has_enough_packets(vs.audio_st, vs.audio_stream, addr(vs.audioq)) and
        stream_has_enough_packets(vs.video_st, vs.video_stream, addr(vs.videoq)) and
        stream_has_enough_packets(vs.subtitle_st, vs.subtitle_stream, addr(vs.subtitleq)))):
      ##  wait 10 ms
      SDL_LockMutex(wait_mutex)
      SDL_CondWaitTimeout(vs.continue_read_thread, wait_mutex, 10)
      SDL_UnlockMutex(wait_mutex)
      continue
    if vs.paused != 0 and
        (vs.audio_st != nil or
        (vs.auddec.finished == vs.audioq.serial and
        frame_queue_nb_remaining(addr(vs.sampq)) == 0)) and
        (vs.video_st != nil or 
        (vs.viddec.finished == vs.videoq.serial and
        frame_queue_nb_remaining(addr(vs.pictq)) == 0)):
      if loop != 1 and (loop == 0 or loop - 1 != 0):
        loop.dec
        stream_seek(vs, if start_time != AV_NOPTS_VALUE: start_time else: 0, 0, 0)
      elif autoexit != 0:
        result = AVERROR_EOF
        echo "fail"
    result = av_read_frame(ic, pkt)
    if result < 0:
      if (result == AVERROR_EOF or avio_feof(ic.pb) != 0) and vs.eof == 0:
        if vs.video_stream >= 0:
          packet_queue_put_nullpacket(addr(vs.videoq), vs.video_stream)
        if vs.audio_stream >= 0:
          packet_queue_put_nullpacket(addr(vs.audioq), vs.audio_stream)
        if vs.subtitle_stream >= 0:
          packet_queue_put_nullpacket(addr(vs.subtitleq), vs.subtitle_stream)
        vs.eof = 1
      if ic.pb != nil and ic.pb.error != 0:
        if autoexit != 0:
          echo "fail"
        else:
          break
      SDL_LockMutex(wait_mutex)
      SDL_CondWaitTimeout(vs.continue_read_thread, wait_mutex, 10)
      SDL_UnlockMutex(wait_mutex)
      continue
    else:
      vs.eof = 0
    ##  check if packet is in play range specified by user, then queue, otherwise discard
    stream_start_time = ic.streams[pkt.stream_index].start_time
    pkt_ts = if pkt.pts == AV_NOPTS_VALUE: pkt.dts else: pkt.pts
    pkt_in_play_range = cint duration == AV_NOPTS_VALUE or 
        (pkt_ts - (if stream_start_time != AV_NOPTS_VALUE: stream_start_time else: 0)) * av_q2d(ic.streams[pkt.stream_index].time_base) - (if start_time != AV_NOPTS_VALUE: start_time else: 0) div 1000000 <= (duration div 1000000)
    if pkt.stream_index == vs.audio_stream and pkt_in_play_range != 0:
      packet_queue_put(addr(vs.audioq), pkt)
    elif pkt.stream_index == vs.video_stream and pkt_in_play_range != 0 and (vs.video_st.disposition and AV_DISPOSITION_ATTACHED_PIC) == 0:
      packet_queue_put(addr(vs.videoq), pkt)
    elif pkt.stream_index == vs.subtitle_stream and pkt_in_play_range != 0:
      packet_queue_put(addr(vs.subtitleq), pkt)
    else:
      av_packet_unref(pkt)
  result = 0
  if ic != nil and vs.ic == nil:
    avformat_close_input(addr(ic))
  if result != 0:
    var event: SDL_Event
    event.type = FF_QUIT_EVENT
    event.user.data1 = vs
    SDL_PushEvent(addr(event))
  SDL_DestroyMutex(wait_mutex)
  return 0

proc stream_open*(filename: cstring; iformat: ptr AVInputFormat): ptr VideoState =
  var vs: ptr VideoState
  vs = cast[ptr VideoState](av_mallocz(sizeof((VideoState)).csize_t))
  vs.video_stream = -1
  vs.last_video_stream = -1
  vs.audio_stream = -1
  vs.last_audio_stream = -1
  vs.subtitle_stream = -1
  vs.last_subtitle_stream = -1
  vs.filename = av_strdup(filename)
  if vs.filename == nil:
    echo "fail"
  vs.iformat = iformat
  vs.ytop = 0
  vs.xleft = 0
  ##  start video display
  if frame_queue_init(addr(vs.pictq), addr(vs.videoq), VIDEO_PICTURE_QUEUE_SIZE, 1) < 0:
    echo "fail"
  if frame_queue_init(addr(vs.subpq), addr(vs.subtitleq),SUBPICTURE_QUEUE_SIZE, 0) < 0:
    echo "fail"
  if frame_queue_init(addr(vs.sampq), addr(vs.audioq), SAMPLE_QUEUE_SIZE, 1) < 0:
    echo "fail"
  if packet_queue_init(addr(vs.videoq)) < 0 or
      packet_queue_init(addr(vs.audioq)) < 0 or
      packet_queue_init(addr(vs.subtitleq)) < 0:
    echo "fail"
  vs.continue_read_thread = SDL_CreateCond()
  if vs.continue_read_thread == nil:
    echo("SDL_CreateCond(): %s", SDL_GetError())
  init_clock(addr(vs.vidclk), addr(vs.videoq.serial))
  init_clock(addr(vs.audclk), addr(vs.audioq.serial))
  init_clock(addr(vs.extclk), addr(vs.extclk.serial))
  vs.audio_clock_serial = -1
  if startup_volume < 0:
    echo("-volume=%d < 0, setting to 0", startup_volume)
  if startup_volume > 100:
    echo("-volume=%d > 100, setting to 100",startup_volume)
  startup_volume = av_clip_c(startup_volume, 0, 100)
  startup_volume = av_clip_c(SDL_MIX_MAXVOLUME * startup_volume div 100, 0, SDL_MIX_MAXVOLUME)
  vs.audio_volume = startup_volume
  vs.muted = 0
  vs.av_sync_type = av_sync_type
  vs.read_tid = SDL_CreateThread(read_thread,"read thread", vs)
  if vs.read_tid == nil:
    echo("SDL_CreateThread(): %s", SDL_GetError())
    stream_close(vs)
    return nil
  return vs

proc stream_cycle_channel*(vs: ptr VideoState; codec_type: cint) =
  var ic: ptr AVFormatContext = vs.ic
  var
    start_index: cint
    stream_index: cint
  var old_index: cint
  var st: ptr AVStream
  var p: ptr AVProgram
  var nb_streams: cint = cint vs.ic.nb_streams
  if codec_type == AVMEDIA_TYPE_VIDEO.int:
    start_index = vs.last_video_stream
    old_index = vs.video_stream
  elif codec_type == AVMEDIA_TYPE_AUDIO.int:
    start_index = vs.last_audio_stream
    old_index = vs.audio_stream
  else:
    start_index = vs.last_subtitle_stream
    old_index = vs.subtitle_stream
  stream_index = start_index
  if codec_type != AVMEDIA_TYPE_VIDEO.int and vs.video_stream != -1:
    p = av_find_program_from_stream(ic, nil, vs.video_stream)
    if p != nil:
      nb_streams = cint p.nb_stream_indexes
      start_index = 0
      while start_index < nb_streams:
        if p.stream_index[start_index] == stream_index.cuint:
          break
        inc(start_index)
      if start_index == nb_streams:
        start_index = -1
      stream_index = start_index
  while true:
    inc(stream_index)
    if stream_index >= nb_streams:
      if codec_type == AVMEDIA_TYPE_SUBTITLE.int:
        stream_index = -1
        vs.last_subtitle_stream = -1
        echo "the_end"
      if start_index == -1:
        return
      stream_index = 0
    if stream_index == start_index:
      return
    st = vs.ic.streams[if p != nil: p.stream_index[stream_index].int else: stream_index]
    if st.codecpar.codec_type.int == codec_type:
      ##  check that parameters are OK
      case codec_type
      of AVMEDIA_TYPE_AUDIO.cint:
        if st.codecpar.sample_rate != 0 and st.codecpar.channels != 0:
          echo "the_end"
      of AVMEDIA_TYPE_VIDEO.cint, AVMEDIA_TYPE_SUBTITLE.cint:
        echo "the_end"
      else:
        discard
  if p != nil and stream_index != -1:
    stream_index = cint p.stream_index[stream_index]
  echo("Switch %s stream from #%d to #%d",av_get_media_type_string(codec_type), old_index, stream_index)
  stream_component_close(vs, old_index)
  stream_component_open(vs, stream_index)

proc toggle_full_screen*(vs: ptr VideoState) =
  is_full_screen = not is_full_screen
  SDL_SetWindowFullscreen(window, if is_full_screen != 0: SDL_WINDOW_FULLSCREEN_DESKTOP.uint32 else: 0)

proc toggle_audio_display*(vs: ptr VideoState) =
  var next: cint = cint vs.show_mode
  while true:
    next = (next + 1) mod SHOW_MODE_NB.int32
    if  next != vs.show_mode.cint and (next == SHOW_MODE_VIDEO.cint and vs.video_st == nil or next != SHOW_MODE_VIDEO.cint and vs.audio_st == nil) :
      break
  if vs.show_mode.cint != next:
    vs.force_refresh = 1
    vs.show_mode = ShowMode next

proc refresh_loop_wait_event*(vs: ptr VideoState; event: ptr SDL_Event) =
  var remaining_time: cdouble = 0.0
  SDL_PumpEvents()
  while SDL_PeepEvents(event, 1, SDL_GETEVENT, SDL_FIRSTEVENT.uint32, SDL_LASTEVENT.uint32) == 0:
    if cursor_hidden == 0 and av_gettime_relative() - cursor_last_shown > CURSOR_HIDE_DELAY:
      SDL_ShowCursor(0)
      cursor_hidden = 1
    if remaining_time > 0.0:
      av_usleep(remaining_time * 1000000.0)
    remaining_time = REFRESH_RATE
    if vs.show_mode != SHOW_MODE_NONE and (vs.paused == 0 or vs.force_refresh != 0):
      video_refresh(vs, addr(remaining_time))
    SDL_PumpEvents()

const AV_TIME_BASE_Q = AVRational(num:1, den:AV_TIME_BASE)

proc seek_chapter*(vs: ptr VideoState; incr: cint) =
  var pos: int64 = int64 get_master_clock(vs) * AV_TIME_BASE
  var i: cint
  if vs.ic.nb_chapters == 0:
    return
  i = 0
  while i < vs.ic.nb_chapters.cint:
    var ch: ptr AVChapter = vs.ic.chapters[i]
    if av_compare_ts(pos, AV_TIME_BASE_Q, ch.start, ch.time_base) < 0:
      dec(i)
      break
    inc(i)
  inc(i, incr)
  i = max(i, 0)
  if i >= vs.ic.nb_chapters.cint:
    return
  echo("Seeking to chapter %d.", i)
  stream_seek(vs, av_rescale_q(vs.ic.chapters[i].start, vs.ic.chapters[i].time_base, AV_TIME_BASE_Q), 0,0)

##  handle an event sent by the GUI




proc event_loop*(cur_stream: ptr VideoState) =
  var event: SDL_Event
  var
    incr: cdouble
    pos: cdouble
    frac: cdouble
  while true:
    var x: cdouble
    refresh_loop_wait_event(cur_stream, addr(event))
    case event.type
    of SDL_KEYDOWN.uint32:
      if exit_on_keydown != 0 or event.key.keysym.sym == K_ESCAPE or event.key.keysym.sym == K_q:
        do_exit(cur_stream)
        break
      if cur_stream.width == 0:
        continue
      case event.key.keysym.sym
      of K_f:
        toggle_full_screen(cur_stream)
        cur_stream.force_refresh = 1
      of K_p, K_SPACE:
        toggle_pause(cur_stream)
      of K_m:
        toggle_mute(cur_stream)
      of K_KP_MULTIPLY, K_0:
        update_volume(cur_stream, 1, SDL_VOLUME_STEP)
      of K_KP_DIVIDE, K_9:
        update_volume(cur_stream, -1, SDL_VOLUME_STEP)
      of K_s:               ##  S: Step to next frame
        step_to_next_frame(cur_stream)
      of K_a:
        stream_cycle_channel(cur_stream, AVMEDIA_TYPE_AUDIO.cint)
      of K_v:
        stream_cycle_channel(cur_stream, AVMEDIA_TYPE_VIDEO.cint)
      of K_c:
        stream_cycle_channel(cur_stream, AVMEDIA_TYPE_VIDEO.cint)
        stream_cycle_channel(cur_stream, AVMEDIA_TYPE_AUDIO.cint)
        stream_cycle_channel(cur_stream, AVMEDIA_TYPE_SUBTITLE.cint)
      of K_t:
        stream_cycle_channel(cur_stream, AVMEDIA_TYPE_SUBTITLE.cint)
      of K_w:
        if cur_stream.show_mode == SHOW_MODE_VIDEO and
            cur_stream.vfilter_idx < nb_vfilters - 1:
          inc(cur_stream.vfilter_idx)
          if  cur_stream.vfilter_idx >= nb_vfilters:
            cur_stream.vfilter_idx = 0
        else:
          cur_stream.vfilter_idx = 0
          toggle_audio_display(cur_stream)
      of K_PAGEUP:
        if cur_stream.ic.nb_chapters <= 1:
          incr = 600.0
          echo "do_seek"
        seek_chapter(cur_stream, 1)
      of K_PAGEDOWN:
        if cur_stream.ic.nb_chapters <= 1:
          incr = -600.0
          echo "do_seek"
        seek_chapter(cur_stream, -1)
      of K_LEFT:
        incr = if seek_interval != 0.0: -seek_interval else: -10.0
        echo "do_seek"
      of K_RIGHT:
        incr = if seek_interval != 0.0: seek_interval else: 10.0
        echo "do_seek"
      of K_UP:
        incr = 60.0
        echo "do_seek"
      of K_DOWN:
        incr = -60.0
        if seek_by_bytes != 0:
          pos = -1
          if pos < 0 and cur_stream.video_stream >= 0:
            pos = cdouble frame_queue_last_pos(addr(cur_stream.pictq))
          if pos < 0 and cur_stream.audio_stream >= 0:
            pos = cdouble frame_queue_last_pos(addr(cur_stream.sampq))
          if pos < 0:
            pos = cdouble avio_tell(cur_stream.ic.pb)
          if cur_stream.ic.bit_rate != 0:
            incr = incr * (cur_stream.ic.bit_rate.float64 / 8.0)
          else:
            incr = incr * 180000.0
          pos += incr
          stream_seek(cur_stream, pos.int64, incr.int64, 1)
        else:
          pos = get_master_clock(cur_stream)
          if pos == Nan:
            pos = cur_stream.seek_pos.cdouble / AV_TIME_BASE
          pos += incr
          if cur_stream.ic.start_time != AV_NOPTS_VALUE and
              pos < cur_stream.ic.start_time.cdouble / AV_TIME_BASE:
            pos = cur_stream.ic.start_time.cdouble / AV_TIME_BASE
          stream_seek(cur_stream, pos.int64 * AV_TIME_BASE,int64(incr * AV_TIME_BASE), 0)
      else:
        discard
    of SDL_MOUSEBUTTONDOWN.uint32:
      if exit_on_mousedown != 0:
        do_exit(cur_stream)
        break
      if event.button.button == SDL_BUTTON_LEFT:
        var last_mouse_left_click: int64 = 0
        if av_gettime_relative() - last_mouse_left_click <= 500000:
          toggle_full_screen(cur_stream)
          cur_stream.force_refresh = 1
          last_mouse_left_click = 0
        else:
          last_mouse_left_click = av_gettime_relative()
    of SDL_MOUSEMOTION.uint32:
      if cursor_hidden != 0:
        SDL_ShowCursor(1)
        cursor_hidden = 0
      cursor_last_shown = av_gettime_relative()
      if event.type == SDL_MOUSEBUTTONDOWN.uint32:
        if event.button.button != SDL_BUTTON_RIGHT:
          break
        x = cdouble event.button.x
      else:
        if (event.motion.state and SDL_BUTTON_RMASK) == 0:
          break
        x = cdouble event.motion.x
      if seek_by_bytes != 0 or cur_stream.ic.duration <= 0:
        var size: uint64 = uint64 avio_size(cur_stream.ic.pb)
        stream_seek(cur_stream, size.int64 * x.int64 div cur_stream.width, 0, 1)
      else:
        var ts: int64
        var
          ns: cint
          hh: cint
          mm: cint
          ss: cint
        var
          tns: cint
          thh: cint
          tmm: cint
          tss: cint
        tns = cint cur_stream.ic.duration div 1000000
        thh = tns div 3600
        tmm = (tns mod 3600) div 60
        tss = (tns mod 60)
        frac = x / cur_stream.width.cdouble
        ns = frac.cint * tns
        hh = ns div 3600
        mm = (ns mod 3600) div 60
        ss = (ns mod 60)
        echo("Seek to %2.0f%% (%2d:%02d:%02d) of total duration (%2d:%02d:%02d)       ",frac * 100, hh, mm, ss, thh, tmm, tss)
        ts = frac.int64 * cur_stream.ic.duration
        if cur_stream.ic.start_time != AV_NOPTS_VALUE:
          inc(ts, cur_stream.ic.start_time)
        stream_seek(cur_stream, ts, 0, 0)
    of SDL_WINDOWEVENT_ENUM.uint32:
      if event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED.uint8:
        cur_stream.width = event.window.data1
        screen_width = event.window.data1
        cur_stream.height = event.window.data2
        screen_height = event.window.data2
        if cur_stream.vis_texture != nil:
          SDL_DestroyTexture(cur_stream.vis_texture)
          cur_stream.vis_texture = nil
      elif event.window.event ==  SDL_WINDOWEVENT_EXPOSED.uint8:
        cur_stream.force_refresh = 1
    of SDL_QUIT_ENUM.uint32, FF_QUIT_EVENT.uint32:
      do_exit(cur_stream)
    else:
      discard

proc opt_frame_size*(optctx: pointer; opt: cstring; arg: cstring): cint =
  echo("Option -s is deprecated, use -video_size.")
  return opt_default(nil, "video_size", arg)

proc opt_width*(optctx: pointer; opt: cstring; arg: cstring): cint =
  screen_width = cint parse_number_or_die(opt, arg, OPT_INT64, 1, int.high.cdouble)
  return 0

proc opt_height*(optctx: pointer; opt: cstring; arg: cstring): cint =
  screen_height = cint parse_number_or_die(opt, arg, OPT_INT64, 1, int.high.cdouble)
  return 0

proc opt_format*(optctx: pointer; opt: cstring; arg: cstring): cint =
  file_iformat = av_find_input_format(arg)
  if file_iformat == nil:
    echo("Unknown input format: %s", arg)
    return AVERROR(EINVAL)
  return 0

proc opt_frame_pix_fmt*(optctx: pointer; opt: cstring; arg: cstring): cint =
  echo("Option -pix_fmt is deprecated, use -pixel_format.")
  return opt_default(nil, "pixel_format", arg)

proc opt_sync*(optctx: pointer; opt: cstring; arg: cstring): cint =
  if strcmp(arg, "audio") == 0:
    av_sync_type = AV_SYNC_AUDIO_MASTER
  elif strcmp(arg, "video") == 0:
    av_sync_type = AV_SYNC_VIDEO_MASTER
  elif strcmp(arg, "ext") == 0:
    av_sync_type = AV_SYNC_EXTERNAL_CLOCK
  else:
    echo("Unknown value for %s: %s", opt, arg)
    quit(1)
  return 0

proc opt_seek*(optctx: pointer; opt: cstring; arg: cstring): cint =
  start_time = parse_time_or_die(opt, arg, 1)
  return 0

proc opt_duration*(optctx: pointer; opt: cstring; arg: cstring): cint =
  duration = parse_time_or_die(opt, arg, 1)
  return 0

proc opt_show_mode*(optctx: pointer; opt: cstring; arg: cstring): cint =
  show_mode = if strcmp(arg, "video") == 0: SHOW_MODE_VIDEO 
              else: 
                if strcmp(arg, "waves") == 0: SHOW_MODE_WAVES 
                else: 
                  if strcmp(arg, "rdft") == 0: SHOW_MODE_RDFT 
                  else: ShowMode parse_number_or_die(opt, arg, OPT_INT, 0, SHOW_MODE_NB.int - 1)
  return 0

proc opt_input_file*(optctx: pointer; filename: var cstring) =
  if input_filename != nil:
    echo("Argument \'%s\' provided as input filename, but \'%s\' was already specified.",filename, input_filename)
    quit(1)
  if strcmp(filename, "-") == 0:
    filename = "pipe:"
  input_filename = filename

# proc strchr(s:cstring, c: int): cstring {.importc.}

proc opt_codec*(optctx: pointer; opt: cstring; arg: cstring): cint 
# proc opt_codec*(optctx: pointer; opt: cstring; arg: cstring): cint = 
#   var spec: cstring = strchr(opt, ':'.int)
#   if spec == nil:
#     echo("No media specifier was specified in \'%s\' in option \'%s\'", arg,opt)
#     return AVERROR(EINVAL)
#   # spec += 1
#   case spec[0]
#   of 'a':
#     audio_codec_name = arg
#   of 's':
#     subtitle_codec_name = arg
#   of 'v':
#     video_codec_name = arg
#   else:
#     echo("Invalid media specifier \'%s\' in option \'%s\'",spec, opt)
#     return AVERROR(EINVAL)
#   return 0

var dummy*: cint

var  options : seq[OptionDef] #= CMDUTILS_COMMON_OPTIONS  & 
#     @[OptionDef(name:"x", flags:HAS_ARG, u:OptionDefUnion(func_arg : opt_width), help:"force displayed width", argname:"width"),
#     OptionDef(name:"y", flags:HAS_ARG, u:OptionDefUnion(func_arg : opt_height), help:"force displayed height", argname:"height"),
#     OptionDef(name:"s", flags:HAS_ARG or  OPT_VIDEO, u:OptionDefUnion(func_arg:opt_frame_size), help:"set frame size (WxH or abbreviation)", argname:"size"),
#     OptionDef(name:"fs", flags:OPT_BOOL, u:OptionDefUnion(dst_ptr:addr is_full_screen), help:"force full screen"),
#     OptionDef(name:"an", flags:OPT_BOOL, u:OptionDefUnion(dst_ptr:addr audio_disable), help:"disable audio"),
#     OptionDef(name:"vn", flags:OPT_BOOL, u:OptionDefUnion(dst_ptr:addr video_disable), help:"disable video"),
#     OptionDef(name:"sn", flags:OPT_BOOL, u:OptionDefUnion(dst_ptr:addr subtitle_disable), help:"disable subtitling"),
#     OptionDef(name:"ast", flags:OPT_STRING or HAS_ARG or OPT_EXPERT, u:OptionDefUnion(dst_ptr: wanted_stream_spec[AVMEDIA_TYPE_AUDIO.int]), help:"select desired audio stream", argname:"stream_specifier"),
#     OptionDef(name:"vst", flags:OPT_STRING or HAS_ARG or OPT_EXPERT, u:OptionDefUnion(dst_ptr: wanted_stream_spec[AVMEDIA_TYPE_VIDEO.int]), help:"select desired video stream", argname:"stream_specifier"),
#     OptionDef(name:"sst", flags:OPT_STRING or HAS_ARG or OPT_EXPERT, u:OptionDefUnion(dst_ptr: wanted_stream_spec[AVMEDIA_TYPE_SUBTITLE.int]), help:"select desired subtitle stream", argname:"stream_specifier"),
#     OptionDef(name:"ss", flags:HAS_ARG, u:OptionDefUnion(func_arg: opt_seek), help:"seek to a given position in seconds", argname:"pos"),
#     OptionDef(name:"t", flags:HAS_ARG, u:OptionDefUnion(func_arg: opt_duration), help:"play  \"duration\" seconds of audio/video", argname:"duration"),
#     OptionDef(name:"bytes", flags:OPT_INT or  HAS_ARG, u:OptionDefUnion(dst_ptr:addr seek_by_bytes), help:"seek by bytes 0=off 1=on -1=auto", argname:"val"),
#     OptionDef(name:"seek_interval", flags:OPT_FLOAT or  HAS_ARG, u:OptionDefUnion(dst_ptr:addr seek_interval), help:"set seek interval for left/right keys, in seconds", argname:"seconds"),
#     OptionDef(name:"nodisp", flags:OPT_BOOL, u:OptionDefUnion(dst_ptr:addr display_disable), help:"disable graphical display"),
#     OptionDef(name:"noborder", flags:OPT_BOOL, u:OptionDefUnion(dst_ptr:addr borderless), help:"borderless window"),
#     OptionDef(name:"alwaysontop", flags:OPT_BOOL, u:OptionDefUnion(dst_ptr:addr alwaysontop), help:"window always on top"),
#     OptionDef(name:"volume", flags:OPT_INT or  HAS_ARG, u:OptionDefUnion(dst_ptr:addr startup_volume), help:"set startup volume 0=min 100=max", argname:"volume"),
#     OptionDef(name:"f", flags:HAS_ARG, u:OptionDefUnion(func_arg: opt_format), help:"force format", argname:"fmt"),
#     OptionDef(name:"pix_fmt", flags:HAS_ARG or  OPT_EXPERT or  OPT_VIDEO, u:OptionDefUnion(func_arg: opt_frame_pix_fmt), help:"set pixel format", argname:"format"),
#     OptionDef(name:"stats", flags:OPT_BOOL or  OPT_EXPERT, u:OptionDefUnion(dst_ptr:addr show_status), help:"show status", argname:""),
#     OptionDef(name:"fast", flags:OPT_BOOL or  OPT_EXPERT, u:OptionDefUnion(dst_ptr:addr fast), help:"non spec compliant optimizations", argname:""),
#     OptionDef(name:"genpts", flags:OPT_BOOL or  OPT_EXPERT, u:OptionDefUnion(dst_ptr:addr genpts), help:"generate pts", argname:""),
#     OptionDef(name:"drp", flags:OPT_INT or  HAS_ARG or  OPT_EXPERT, u:OptionDefUnion(dst_ptr:addr decoder_reorder_pts), help:"let decoder reorder pts 0=off 1=on -1=auto", argname:""),
#     OptionDef(name:"lowres", flags:OPT_INT or  HAS_ARG or  OPT_EXPERT, u:OptionDefUnion(dst_ptr:addr lowres), help:"", argname:""),
#     OptionDef(name:"sync", flags:HAS_ARG or  OPT_EXPERT, u:OptionDefUnion(func_arg: opt_sync), help:"set audio-video sync. type (type=audio/video/ext)", argname:"type"),
#     OptionDef(name:"autoexit", flags:OPT_BOOL or  OPT_EXPERT, u:OptionDefUnion(dst_ptr:addr autoexit), help:"exit at the end", argname:""),
#     OptionDef(name:"exitonkeydown", flags:OPT_BOOL or  OPT_EXPERT, u:OptionDefUnion(dst_ptr:addr exit_on_keydown), help:"exit on key down", argname:""),
#     OptionDef(name:"exitonmousedown", flags:OPT_BOOL or  OPT_EXPERT, u:OptionDefUnion(dst_ptr:addr exit_on_mousedown), help:"exit on mouse down", argname:""),
#     OptionDef(name:"loop", flags:OPT_INT or  HAS_ARG or  OPT_EXPERT, u:OptionDefUnion(dst_ptr:addr loop), help:"set number of times the playback shall be looped", argname:"loop count"),
#     OptionDef(name:"framedrop", flags:OPT_BOOL or  OPT_EXPERT, u:OptionDefUnion(dst_ptr:addr framedrop), help:"drop frames when cpu is too slow", argname:""),
#     OptionDef(name:"infbuf", flags:OPT_BOOL or  OPT_EXPERT, u:OptionDefUnion(dst_ptr:addr infinite_buffer), help:"don't limit the input buffer size (useful with realtime streams)", argname:""),
#     OptionDef(name:"window_title", flags:OPT_STRING or  HAS_ARG, u:OptionDefUnion(dst_ptr:addr window_title), help:"set window title", argname:"window title"),
#     OptionDef(name:"left", flags:OPT_INT or  HAS_ARG or  OPT_EXPERT, u:OptionDefUnion(dst_ptr:addr screen_left), help:"set the x position for the left of the window", argname:"x pos"),
#     OptionDef(name:"top", flags:OPT_INT or  HAS_ARG or  OPT_EXPERT, u:OptionDefUnion(dst_ptr:addr screen_top), help:"set the y position for the top of the window", argname:"y pos"),
#     OptionDef(name:"vf", flags:OPT_EXPERT or  HAS_ARG, u:OptionDefUnion(func_arg: opt_add_vfilter), help:"set video filters", argname:"filter_graph"),
#     OptionDef(name:"af", flags:OPT_STRING or  HAS_ARG, u:OptionDefUnion(dst_ptr:addr afilters), help:"set audio filters", argname:"filter_graph"),
#     OptionDef(name:"rdftspeed", flags:OPT_INT or  HAS_ARG or  OPT_AUDIO or  OPT_EXPERT, u:OptionDefUnion(dst_ptr:addr rdftspeed), help:"rdft speed", argname:"msecs"),
#     OptionDef(name:"showmode", flags:HAS_ARG, u:OptionDefUnion(func_arg: opt_show_mode), help:"select show mode (0 = video, 1 = waves, 2 = RDFT)", argname:"mode"),
#     OptionDef(name:"default", flags:HAS_ARG or  OPT_AUDIO or  OPT_VIDEO or  OPT_EXPERT, u:OptionDefUnion(func_arg: opt_default), help:"generic catch all option", argname:""),
#     OptionDef(name:"i", flags:OPT_BOOL, u:OptionDefUnion(dst_ptr:addr dummy), help:"read specified file", argname:"input_file"),
#     OptionDef(name:"codec", flags:HAS_ARG, u:OptionDefUnion(func_arg: opt_codec), help:"force decoder", argname:"decoder_name"),
#     OptionDef(name:"acodec", flags:HAS_ARG or  OPT_STRING or  OPT_EXPERT, u:OptionDefUnion(dst_ptr:addr audio_codec_name), help:"force audio decoder", argname:"decoder_name"),
#     OptionDef(name:"scodec", flags:HAS_ARG or  OPT_STRING or  OPT_EXPERT, u:OptionDefUnion(dst_ptr:addr subtitle_codec_name), help:"force subtitle decoder", argname:"decoder_name"),
#     OptionDef(name:"vcodec", flags:HAS_ARG or  OPT_STRING or  OPT_EXPERT, u:OptionDefUnion(dst_ptr:addr video_codec_name), help:"force video decoder", argname:"decoder_name"),
#     OptionDef(name:"autorotate", flags:OPT_BOOL, u:OptionDefUnion(dst_ptr:addr autorotate), help:"automatically rotate video", argname:""),
#     OptionDef(name:"find_stream_info", flags:OPT_BOOL or  OPT_INPUT or  OPT_EXPERT, u:OptionDefUnion(dst_ptr:addr find_stream_info), help:"read and decode the streams to fill missing information with heuristics"),
#     OptionDef(name:"filter_threads", flags:HAS_ARG or  OPT_INT or  OPT_EXPERT, u:OptionDefUnion(dst_ptr:addr filter_nbthreads), help:"number of filter threads per graph"),
    
# ]


proc show_help_default*(opt: cstring; arg: cstring) =
  av_log_set_callback(log_callback_help)
  # show_help_options(options, "Main options:", 0, OPT_EXPERT, 0)
  # show_help_options(options, "Advanced options:", OPT_EXPERT, 0, 0)
  show_help_children(avcodec_get_class(), AV_OPT_FLAG_DECODING_PARAM)
  show_help_children(avformat_get_class(), AV_OPT_FLAG_DECODING_PARAM)
  show_help_children(avfilter_get_class(), AV_OPT_FLAG_FILTERING_PARAM)
  echo("\nWhile playing:\nq, ESC quit\nf toggle full screen\np, SPC pause\nm toggle mute\n9, 0 decrease and increase volume respectively/, * decrease and increase volume respectively\na cycle audio channel in the current program\nv cycle video channel\nt                   cycle subtitle channel in the current program\nc                   cycle program\nw                   cycle video filters or show modes\ns                   activate frame-step mode\nleft/right          seek backward/forward 10 seconds or to custom interval if -seek_interval is set\ndown/up             seek backward/forward 1 minute\npage down/page up   seek backward/forward 10 minutes\nright mouse click   seek to percentage in file corresponding to fraction of width\nleft cdouble-click   toggle full screen")


proc main*(argc: cint; argv: cstringArray): cint =
  var flags: cint
  var vs: ptr VideoState
  init_dynload()
  av_log_set_flags(AV_LOG_SKIP_REPEATED)
  # parse_loglevel(argc, argv, options)
  ##  register all codecs, demux and protocols
  avdevice_register_all()
  avformat_network_init()
  init_opts()
  signal(SIGINT, sigterm_handler)
  ##  Interrupt (ANSI).
  signal(SIGTERM, sigterm_handler)
  ##  Termination (ANSI).
  # show_banner(argc, argv, options)
  # parse_options(nil, argc, argv, options, opt_input_file)
  if input_filename == nil:
    echo("Use -h to get full help or, even better, run \'man %s\'",program_name)
    quit(1)
  if display_disable != 0:
    video_disable = 1
  flags = SDL_INIT_VIDEO or SDL_INIT_AUDIO or SDL_INIT_TIMER
  if audio_disable != 0:
    flags = flags and not SDL_INIT_AUDIO
  else:
    if SDL_getenv("SDL_AUDIO_ALSA_SET_BUFFER_SIZE") == nil:
      SDL_setenv("SDL_AUDIO_ALSA_SET_BUFFER_SIZE", "1", 1)
  if display_disable != 0:
    flags = flags and not SDL_INIT_VIDEO
  if SDL_Init(flags) != 0:
    echo("Could not initialize SDL - %s")
    echo("(Did you set the DISPLAY variable?)")
    quit(1)
  SDL_EventState(SDL_SYSWMEVENT, SDL_IGNORE)
  SDL_EventState(SDL_USEREVENT, SDL_IGNORE)
  av_init_packet(addr(flush_pkt))
  flush_pkt.data = cast[ptr uint8](addr(flush_pkt))
  if display_disable == 0:
    var flags: cint = SDL_WINDOW_HIDDEN
    if alwaysontop:
      flags = flags or SDL_WINDOW_ALWAYS_ON_TOP
    if borderless:
      flags = flags or SDL_WINDOW_BORDERLESS
    else:
      flags = flags or SDL_WINDOW_RESIZABLE
    window = SDL_CreateWindow(program_name, SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED, default_width,default_height, flags)
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear")
    if window != nil:
      renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED or SDL_RENDERER_PRESENTVSYNC)
      if renderer == nil:
        echo("Failed to initialize a hardware accelerated renderer: %s")
        renderer = SDL_CreateRenderer(window, -1, 0)
      if renderer:
        if SDL_GetRendererInfo(renderer, addr(renderer_info)) == 0:
          echo("Initialized %s renderer.",renderer_info.name)
    if window == nil or renderer == nil or  renderer_info.num_texture_formats == 0:
      echo("Failed to create window or renderer: %s")
      do_exit(nil)
  vs = stream_open(input_filename, file_iformat)
  if vs == nil:
    echo("Failed to initialize VideoState!")
    do_exit(nil)
  event_loop(vs)
  return 0
