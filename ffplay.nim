import posix,winim, sdl2

type
  AVCodecID* = enum
    AV_CODEC_ID_NONE,         ##  video codecs
    AV_CODEC_ID_MPEG1VIDEO, AV_CODEC_ID_MPEG2VIDEO, ## /< preferred ID for MPEG-1/2 video decoding
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
    AV_CODEC_ID_PHOTOCD, AV_CODEC_ID_IPU, AV_CODEC_ID_ARGO, AV_CODEC_ID_CRI, ##  various PCM "codecs"
    AV_CODEC_ID_FIRST_AUDIO = 0x00010000, ## /< A dummy id pointing at the start of audio codecs
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
    AV_CODEC_ID_PCM_S64BE, AV_CODEC_ID_PCM_F16LE, AV_CODEC_ID_PCM_F24LE, AV_CODEC_ID_PCM_VIDC, ##  various ADPCM codecs
    AV_CODEC_ID_ADPCM_IMA_QT = 0x00011000, AV_CODEC_ID_ADPCM_IMA_WAV,
    AV_CODEC_ID_ADPCM_IMA_DK3, AV_CODEC_ID_ADPCM_IMA_DK4,
    AV_CODEC_ID_ADPCM_IMA_WS, AV_CODEC_ID_ADPCM_IMA_SMJPEG, AV_CODEC_ID_ADPCM_MS,
    AV_CODEC_ID_ADPCM_4XM, AV_CODEC_ID_ADPCM_XA, AV_CODEC_ID_ADPCM_ADX,
    AV_CODEC_ID_ADPCM_EA, AV_CODEC_ID_ADPCM_G726, AV_CODEC_ID_ADPCM_CT,
    AV_CODEC_ID_ADPCM_SWF, AV_CODEC_ID_ADPCM_YAMAHA, AV_CODEC_ID_ADPCM_SBPRO_4,
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
    AV_CODEC_ID_ADPCM_IMA_MTF, AV_CODEC_ID_ADPCM_IMA_CUNNING, AV_CODEC_ID_ADPCM_IMA_MOFLEX, ##  AMR
    AV_CODEC_ID_AMR_NB = 0x00012000, AV_CODEC_ID_AMR_WB, ##  RealAudio codecs
    AV_CODEC_ID_RA_144 = 0x00013000, AV_CODEC_ID_RA_288, ##  various DPCM codecs
    AV_CODEC_ID_ROQ_DPCM = 0x00014000, AV_CODEC_ID_INTERPLAY_DPCM,
    AV_CODEC_ID_XAN_DPCM, AV_CODEC_ID_SOL_DPCM,
    AV_CODEC_ID_SDX2_DPCM = 0x00014800, AV_CODEC_ID_GREMLIN_DPCM, AV_CODEC_ID_DERF_DPCM, ##  audio codecs
    AV_CODEC_ID_MP2 = 0x00015000, AV_CODEC_ID_MP3, ## /< preferred ID for decoding MPEG audio layer 1, 2 or 3
    AV_CODEC_ID_AAC, AV_CODEC_ID_AC3, AV_CODEC_ID_DTS, AV_CODEC_ID_VORBIS,
    AV_CODEC_ID_DVAUDIO, AV_CODEC_ID_WMAV1, AV_CODEC_ID_WMAV2, AV_CODEC_ID_MACE3,
    AV_CODEC_ID_MACE6, AV_CODEC_ID_VMDAUDIO, AV_CODEC_ID_FLAC, AV_CODEC_ID_MP3ADU,
    AV_CODEC_ID_MP3ON4, AV_CODEC_ID_SHORTEN, AV_CODEC_ID_ALAC,
    AV_CODEC_ID_WESTWOOD_SND1, AV_CODEC_ID_GSM, ## /< as in Berlin toast format
    AV_CODEC_ID_QDM2, AV_CODEC_ID_COOK, AV_CODEC_ID_TRUESPEECH, AV_CODEC_ID_TTA,
    AV_CODEC_ID_SMACKAUDIO, AV_CODEC_ID_QCELP, AV_CODEC_ID_WAVPACK,
    AV_CODEC_ID_DSICINAUDIO, AV_CODEC_ID_IMC, AV_CODEC_ID_MUSEPACK7,
    AV_CODEC_ID_MLP, AV_CODEC_ID_GSM_MS, ##  as found in WAV
    AV_CODEC_ID_ATRAC3, AV_CODEC_ID_APE, AV_CODEC_ID_NELLYMOSER,
    AV_CODEC_ID_MUSEPACK8, AV_CODEC_ID_SPEEX, AV_CODEC_ID_WMAVOICE,
    AV_CODEC_ID_WMAPRO, AV_CODEC_ID_WMALOSSLESS, AV_CODEC_ID_ATRAC3P,
    AV_CODEC_ID_EAC3, AV_CODEC_ID_SIPR, AV_CODEC_ID_MP1, AV_CODEC_ID_TWINVQ,
    AV_CODEC_ID_TRUEHD, AV_CODEC_ID_MP4ALS, AV_CODEC_ID_ATRAC1,
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
    AV_CODEC_ID_HCA, AV_CODEC_ID_FASTAUDIO, ##  subtitle codecs
    AV_CODEC_ID_FIRST_SUBTITLE = 0x00017000, ## /< A dummy ID pointing at the start of subtitle codecs.
    AV_CODEC_ID_DVB_SUBTITLE, AV_CODEC_ID_TEXT, ## /< raw UTF-8 text
    AV_CODEC_ID_XSUB, AV_CODEC_ID_SSA, AV_CODEC_ID_MOV_TEXT,
    AV_CODEC_ID_HDMV_PGS_SUBTITLE, AV_CODEC_ID_DVB_TELETEXT, AV_CODEC_ID_SRT,
    AV_CODEC_ID_MICRODVD = 0x00017800, AV_CODEC_ID_EIA_608, AV_CODEC_ID_JACOSUB,
    AV_CODEC_ID_SAMI, AV_CODEC_ID_REALTEXT, AV_CODEC_ID_STL,
    AV_CODEC_ID_SUBVIEWER1, AV_CODEC_ID_SUBVIEWER, AV_CODEC_ID_SUBRIP,
    AV_CODEC_ID_WEBVTT, AV_CODEC_ID_MPL2, AV_CODEC_ID_VPLAYER, AV_CODEC_ID_PJS,
    AV_CODEC_ID_ASS, AV_CODEC_ID_HDMV_TEXT_SUBTITLE, AV_CODEC_ID_TTML, AV_CODEC_ID_ARIB_CAPTION, ##  other specific kind of codecs (generally used for attachments)
    AV_CODEC_ID_FIRST_UNKNOWN = 0x00018000, ## /< A dummy ID pointing at the start of various fake codecs.
    AV_CODEC_ID_SCTE_35,      ## /< Contain timestamp estimated through PCR of program stream.
    AV_CODEC_ID_EPG, AV_CODEC_ID_BINTEXT = 0x00018800, AV_CODEC_ID_XBIN,
    AV_CODEC_ID_IDF, AV_CODEC_ID_OTF, AV_CODEC_ID_SMPTE_KLV, AV_CODEC_ID_DVD_NAV,
    AV_CODEC_ID_TIMED_ID3, AV_CODEC_ID_BIN_DATA, AV_CODEC_ID_PROBE = 0x00019000, 
    AV_CODEC_ID_MPEG2TS = 0x00020000, 
    AV_CODEC_ID_MPEG4SYSTEMS = 0x00020001, 
                                        
    AV_CODEC_ID_FFMETADATA = 0x00021000, 
    AV_CODEC_ID_WRAPPED_AVFRAME = 0x00021001 

const
  AV_CODEC_ID_PCM_S16LE = AV_CODEC_ID_FIRST_AUDIO
  AV_CODEC_ID_DVD_SUBTITLE = AV_CODEC_ID_FIRST_SUBTITLE
  AV_CODEC_ID_TTF = AV_CODEC_ID_FIRST_UNKNOWN
  RAW_PACKET_BUFFER_SIZE = 2500000
  SWR_CH_MAX = 64


var program_name*: string = "ffplay"

var program_birth_year*: cint = 2003

var var_names*: array[6,string] = ["t", "n", "pos", "w", "h", ""]
const MAX_REORDER_DELAY = 16

const
  VAR_T* = 0
  VAR_N* = 1
  VAR_POS* = 2
  VAR_W* = 3
  VAR_H* = 4
  VAR_VARS_NB* = 5

type
  AVStereo3DType* = enum
    AV_STEREO3D_2D, AV_STEREO3D_SIDEBYSIDE, AV_STEREO3D_TOPBOTTOM,
    AV_STEREO3D_FRAMESEQUENCE, AV_STEREO3D_CHECKERBOARD,
    AV_STEREO3D_SIDEBYSIDE_QUINCUNX, AV_STEREO3D_LINES, AV_STEREO3D_COLUMNS

  AVPictureStructure* = enum
    AV_PICTURE_STRUCTURE_UNKNOWN, ## < unknown
    AV_PICTURE_STRUCTURE_TOP_FIELD, ## < coded as top field
    AV_PICTURE_STRUCTURE_BOTTOM_FIELD, ## < coded as bottom field
    AV_PICTURE_STRUCTURE_FRAME ## < coded as frame

type
  AVStereo3DView* = enum
    AV_STEREO3D_VIEW_PACKED, AV_STEREO3D_VIEW_LEFT, AV_STEREO3D_VIEW_RIGHT

type
  fft_permutation_type* = enum
    FF_FFT_PERM_DEFAULT, FF_FFT_PERM_SWAP_LSBS, FF_FFT_PERM_AVX


type
  mdct_permutation_type* = enum
    FF_MDCT_PERM_NONE, FF_MDCT_PERM_INTERLEAVE
type
  H264_SEI_Type* = enum
    H264_SEI_TYPE_BUFFERING_PERIOD = 0, ## /< buffering period (H.264, D.1.1)
    H264_SEI_TYPE_PIC_TIMING = 1, ## /< picture timing
    H264_SEI_TYPE_PAN_SCAN_RECT = 2, ## /< pan-scan rectangle
    H264_SEI_TYPE_FILLER_PAYLOAD = 3, ## /< filler data
    H264_SEI_TYPE_USER_DATA_REGISTERED = 4, ## /< registered user data as specified by Rec. ITU-T T.35
    H264_SEI_TYPE_USER_DATA_UNREGISTERED = 5, ## /< unregistered user data
    H264_SEI_TYPE_RECOVERY_POINT = 6, ## /< recovery point (frame # to decoder sync)
    H264_SEI_TYPE_FRAME_PACKING = 45, ## /< frame packing arrangement
    H264_SEI_TYPE_DISPLAY_ORIENTATION = 47, ## /< display orientation
    H264_SEI_TYPE_GREEN_METADATA = 56, ## /< GreenMPEG information
    H264_SEI_TYPE_MASTERING_DISPLAY_COLOUR_VOLUME = 137, ## /< mastering display properties
    H264_SEI_TYPE_ALTERNATIVE_TRANSFER = 147 ## /< alternative transfer
  H264_SEI_PicStructType* = enum
    H264_SEI_PIC_STRUCT_FRAME = 0, ## /<  0: %frame
    H264_SEI_PIC_STRUCT_TOP_FIELD = 1, ## /<  1: top field
    H264_SEI_PIC_STRUCT_BOTTOM_FIELD = 2, ## /<  2: bottom field
    H264_SEI_PIC_STRUCT_TOP_BOTTOM = 3, ## /<  3: top field, bottom field, in that order
    H264_SEI_PIC_STRUCT_BOTTOM_TOP = 4, ## /<  4: bottom field, top field, in that order
    H264_SEI_PIC_STRUCT_TOP_BOTTOM_TOP = 5, ## /<  5: top field, bottom field, top field repeated, in that order
    H264_SEI_PIC_STRUCT_BOTTOM_TOP_BOTTOM = 6, ## /<  6: bottom field, top field, bottom field repeated, in that order
    H264_SEI_PIC_STRUCT_FRAME_DOUBLING = 7, ## /<  7: %frame doubling
    H264_SEI_PIC_STRUCT_FRAME_TRIPLING = 8
  H264_SEI_FpaType* = enum
    H264_SEI_FPA_TYPE_CHECKERBOARD = 0, H264_SEI_FPA_TYPE_INTERLEAVE_COLUMN = 1,
    H264_SEI_FPA_TYPE_INTERLEAVE_ROW = 2, H264_SEI_FPA_TYPE_SIDE_BY_SIDE = 3,
    H264_SEI_FPA_TYPE_TOP_BOTTOM = 4, H264_SEI_FPA_TYPE_INTERLEAVE_TEMPORAL = 5,
    H264_SEI_FPA_TYPE_2D = 6




type
  ID3v2Encoding* = enum
    ID3v2_ENCODING_ISO8859 = 0, ID3v2_ENCODING_UTF16BOM = 1,
    ID3v2_ENCODING_UTF16BE = 2, ID3v2_ENCODING_UTF8 = 3


type
  AVEscapeMode* = enum
    AV_ESCAPE_MODE_AUTO,      ## /< Use auto-selected escaping mode.
    AV_ESCAPE_MODE_BACKSLASH, ## /< Use backslash escaping.
    AV_ESCAPE_MODE_QUOTE      ## /< Use single-quote escaping.


type
  AVPacketSideDataType* = enum
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


type
  AVSideDataParamChangeFlags* = enum
    AV_SIDE_DATA_PARAM_CHANGE_CHANNEL_COUNT = 0x00000001,
    AV_SIDE_DATA_PARAM_CHANGE_CHANNEL_LAYOUT = 0x00000002,
    AV_SIDE_DATA_PARAM_CHANGE_SAMPLE_RATE = 0x00000004,
    AV_SIDE_DATA_PARAM_CHANGE_DIMENSIONS = 0x00000008


type
  AudioObjectType* = enum
    AOT_NULL,                 ##  Support?                Name
    AOT_AAC_MAIN,             ## /< Y                       Main
    AOT_AAC_LC,               ## /< Y                       Low Complexity
    AOT_AAC_SSR,              ## /< N (code in SoC repo)    Scalable Sample Rate
    AOT_AAC_LTP,              ## /< Y                       Long Term Prediction
    AOT_SBR,                  ## /< Y                       Spectral Band Replication
    AOT_AAC_SCALABLE,         ## /< N                       Scalable
    AOT_TWINVQ,               ## /< N                       Twin Vector Quantizer
    AOT_CELP,                 ## /< N                       Code Excited Linear Prediction
    AOT_HVXC,                 ## /< N                       Harmonic Vector eXcitation Coding
    AOT_TTSI = 12,              ## /< N                       Text-To-Speech Interface
    AOT_MAINSYNTH,            ## /< N                       Main Synthesis
    AOT_WAVESYNTH,            ## /< N                       Wavetable Synthesis
    AOT_MIDI,                 ## /< N                       General MIDI
    AOT_SAFX,                 ## /< N                       Algorithmic Synthesis and Audio Effects
    AOT_ER_AAC_LC,            ## /< N                       Error Resilient Low Complexity
    AOT_ER_AAC_LTP = 19,        ## /< N                       Error Resilient Long Term Prediction
    AOT_ER_AAC_SCALABLE,      ## /< N                       Error Resilient Scalable
    AOT_ER_TWINVQ,            ## /< N                       Error Resilient Twin Vector Quantizer
    AOT_ER_BSAC,              ## /< N                       Error Resilient Bit-Sliced Arithmetic Coding
    AOT_ER_AAC_LD,            ## /< N                       Error Resilient Low Delay
    AOT_ER_CELP,              ## /< N                       Error Resilient Code Excited Linear Prediction
    AOT_ER_HVXC,              ## /< N                       Error Resilient Harmonic Vector eXcitation Coding
    AOT_ER_HILN,              ## /< N                       Error Resilient Harmonic and Individual Lines plus Noise
    AOT_ER_PARAM,             ## /< N                       Error Resilient Parametric
    AOT_SSC,                  ## /< N                       SinuSoidal Coding
    AOT_PS,                   ## /< N                       Parametric Stereo
    AOT_SURROUND,             ## /< N                       MPEG Surround
    AOT_ESCAPE,               ## /< Y                       Escape Value
    AOT_L1,                   ## /< Y                       Layer 1
    AOT_L2,                   ## /< Y                       Layer 2
    AOT_L3,                   ## /< Y                       Layer 3
    AOT_DST,                  ## /< N                       Direct Stream Transfer
    AOT_ALS,                  ## /< Y                       Audio LosslesS
    AOT_SLS,                  ## /< N                       Scalable LosslesS
    AOT_SLS_NON_CORE,         ## /< N                       Scalable LosslesS (non core)
    AOT_ER_AAC_ELD,           ## /< N                       Error Resilient Enhanced Low Delay
    AOT_SMR_SIMPLE,           ## /< N                       Symbolic Music Representation Simple
    AOT_SMR_MAIN,             ## /< N                       Symbolic Music Representation Main
    AOT_USAC_NOSBR,           ## /< N                       Unified Speech and Audio Coding (no SBR)
    AOT_SAOC,                 ## /< N                       Spatial Audio Object Coding
    AOT_LD_SURROUND,          ## /< N                       Low Delay MPEG Surround
    AOT_USAC                  ## /< N                       Unified Speech and Audio Coding


type
  BenchAction* = enum
    ACTION_START, ACTION_STOP, NB_ACTION


type
  show_muxdemuxers* = enum
    SHOW_DEFAULT, SHOW_DEMUXERS, SHOW_MUXERS


type
  memory_order* = enum
    memory_order_relaxed, memory_order_consume, memory_order_acquire,
    memory_order_release, memory_order_acq_rel, memory_order_seq_cst
  SwsDither* = enum
    SWS_DITHER_NONE = 0, SWS_DITHER_AUTO, SWS_DITHER_BAYER, SWS_DITHER_ED,
    SWS_DITHER_A_DITHER, SWS_DITHER_X_DITHER, NB_SWS_DITHER
  SwsAlphaBlend* = enum
    SWS_ALPHA_BLEND_NONE = 0, SWS_ALPHA_BLEND_UNIFORM, SWS_ALPHA_BLEND_CHECKERBOARD,
    SWS_ALPHA_BLEND_NB


type
  AVRounding* = enum
    AV_ROUND_ZERO = 0,          ## /< Round toward zero.
    AV_ROUND_INF = 1,           ## /< Round away from zero.
    AV_ROUND_DOWN = 2,          ## /< Round toward -infinity.
    AV_ROUND_UP = 3,            ## /< Round toward +infinity.
    AV_ROUND_NEAR_INF = 5,      ## /< Round to nearest and halfway cases away from zero.
    AV_ROUND_PASS_MINMAX = 8192


type
  AVColorRange* = enum
    AVCOL_RANGE_UNSPECIFIED = 0, AVCOL_RANGE_MPEG = 1, ## /< the normal 219*2^(n-8) "MPEG" YUV ranges
    AVCOL_RANGE_JPEG = 2,       ## /< the normal     2^n-1   "JPEG" YUV ranges
    AVCOL_RANGE_NB            ## /< Not part of ABI


type
  AVColorPrimaries* = enum
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

const
  AVCOL_PRI_SMPTEST428_1 = AVCOL_PRI_SMPTE428
  AVCOL_PRI_JEDEC_P22 = AVCOL_PRI_EBU3213

type
  AVColorTransferCharacteristic* = enum
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

const
  AVCOL_TRC_SMPTEST2084 = AVCOL_TRC_SMPTE2084
  AVCOL_TRC_SMPTEST428_1 = AVCOL_TRC_SMPTE428

type
  AVColorSpace* = enum
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

const
  AVCOL_SPC_YCOCG = AVCOL_SPC_YCGCO

type
  AVChromaLocation* = enum
    AVCHROMA_LOC_UNSPECIFIED = 0, AVCHROMA_LOC_LEFT = 1, ## /< MPEG-2/4 4:2:0, H.264 default for 4:2:0
    AVCHROMA_LOC_CENTER = 2,    ## /< MPEG-1 4:2:0, JPEG 4:2:0, H.263 4:2:0
    AVCHROMA_LOC_TOPLEFT = 3,   ## /< ITU-R 601, SMPTE 274M 296M S314M(DV 4:1:1), mpeg2 4:2:2
    AVCHROMA_LOC_TOP = 4, AVCHROMA_LOC_BOTTOMLEFT = 5, AVCHROMA_LOC_BOTTOM = 6, AVCHROMA_LOC_NB ## /< Not part of ABI


type
  AVSampleFormat* = enum
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


type
  AVDiscard* = enum ##  We leave some space between them for extensions (drop some
                 ##  keyframes for intra-only or drop just some bidir frames).
    AVDISCARD_NONE = -16,       ## /< discard nothing
    AVDISCARD_DEFAULT = 0,      ## /< discard useless packets like 0 size packets in avi
    AVDISCARD_NONREF = 8,       ## /< discard all non reference
    AVDISCARD_BIDIR = 16,       ## /< discard all bidirectional frames
    AVDISCARD_NONINTRA = 24,    ## /< discard all non intra frames
    AVDISCARD_NONKEY = 32,      ## /< discard all frames except keyframes
    AVDISCARD_ALL = 48          ## /< discard all


type
  AVHWDeviceType* = enum
    AV_HWDEVICE_TYPE_NONE, AV_HWDEVICE_TYPE_VDPAU, AV_HWDEVICE_TYPE_CUDA,
    AV_HWDEVICE_TYPE_VAAPI, AV_HWDEVICE_TYPE_DXVA2, AV_HWDEVICE_TYPE_QSV,
    AV_HWDEVICE_TYPE_VIDEOTOOLBOX, AV_HWDEVICE_TYPE_D3D11VA, AV_HWDEVICE_TYPE_DRM,
    AV_HWDEVICE_TYPE_OPENCL, AV_HWDEVICE_TYPE_MEDIACODEC


  AVFrameSideDataType* = enum
    AV_FRAME_DATA_PANSCAN, AV_FRAME_DATA_A53_CC, AV_FRAME_DATA_STEREO3D,
    AV_FRAME_DATA_MATRIXENCODING, AV_FRAME_DATA_DOWNMIX_INFO,
    AV_FRAME_DATA_REPLAYGAIN, AV_FRAME_DATA_DISPLAYMATRIX, AV_FRAME_DATA_AFD,
    AV_FRAME_DATA_MOTION_VECTORS, AV_FRAME_DATA_SKIP_SAMPLES,
    AV_FRAME_DATA_AUDIO_SERVICE_TYPE, AV_FRAME_DATA_MASTERING_DISPLAY_METADATA,
    AV_FRAME_DATA_GOP_TIMECODE, AV_FRAME_DATA_SPHERICAL,
    AV_FRAME_DATA_CONTENT_LIGHT_LEVEL, AV_FRAME_DATA_ICC_PROFILE, ##  #if FF_API_FRAME_QP
    AV_FRAME_DATA_QP_TABLE_PROPERTIES, AV_FRAME_DATA_QP_TABLE_DATA, ##  #endif
    AV_FRAME_DATA_S12M_TIMECODE, AV_FRAME_DATA_DYNAMIC_HDR_PLUS,
    AV_FRAME_DATA_REGIONS_OF_INTEREST, AV_FRAME_DATA_VIDEO_ENC_PARAMS,
    AV_FRAME_DATA_SEI_UNREGISTERED

  AVPictureType* = enum
    AV_PICTURE_TYPE_NONE = 0,   ## /< Undefined
    AV_PICTURE_TYPE_I,        ## /< Intra
    AV_PICTURE_TYPE_P,        ## /< Predicted
    AV_PICTURE_TYPE_B,        ## /< Bi-dir predicted
    AV_PICTURE_TYPE_S,        ## /< S(GMC)-VOP MPEG-4
    AV_PICTURE_TYPE_SI,       ## /< Switching Intra
    AV_PICTURE_TYPE_SP,       ## /< Switching Predicted
    AV_PICTURE_TYPE_BI        ## /< BI type

  RDFTransformType* = enum
    DFT_R2C, IDFT_C2R, IDFT_R2C, DFT_C2R

  AVSubtitleType* = enum
    SUBTITLE_NONE, SUBTITLE_BITMAP, 
    SUBTITLE_TEXT, SUBTITLE_ASS

  AVClassCategory* = enum
    AV_CLASS_CATEGORY_NA = 0, AV_CLASS_CATEGORY_INPUT, AV_CLASS_CATEGORY_OUTPUT,
    AV_CLASS_CATEGORY_MUXER, AV_CLASS_CATEGORY_DEMUXER, AV_CLASS_CATEGORY_ENCODER,
    AV_CLASS_CATEGORY_DECODER, AV_CLASS_CATEGORY_FILTER,
    AV_CLASS_CATEGORY_BITSTREAM_FILTER, AV_CLASS_CATEGORY_SWSCALER,
    AV_CLASS_CATEGORY_SWRESAMPLER, AV_CLASS_CATEGORY_DEVICE_VIDEO_OUTPUT = 40,
    AV_CLASS_CATEGORY_DEVICE_VIDEO_INPUT, AV_CLASS_CATEGORY_DEVICE_AUDIO_OUTPUT,
    AV_CLASS_CATEGORY_DEVICE_AUDIO_INPUT, AV_CLASS_CATEGORY_DEVICE_OUTPUT,
    AV_CLASS_CATEGORY_DEVICE_INPUT, AV_CLASS_CATEGORY_NB ## /< not part of ABI/API


  AVOptionType* = enum
    AV_OPT_TYPE_FLAGS, AV_OPT_TYPE_INT, AV_OPT_TYPE_INT64, AV_OPT_TYPE_DOUBLE,
    AV_OPT_TYPE_FLOAT, AV_OPT_TYPE_STRING, AV_OPT_TYPE_RATIONAL, AV_OPT_TYPE_BINARY, ## /< offset must point to a pointer immediately followed by an int for the length
    AV_OPT_TYPE_DICT, AV_OPT_TYPE_UINT64, AV_OPT_TYPE_CONST, AV_OPT_TYPE_IMAGE_SIZE, ## /< offset must point to two consecutive integers
    AV_OPT_TYPE_PIXEL_FMT, AV_OPT_TYPE_SAMPLE_FMT, AV_OPT_TYPE_VIDEO_RATE, ## /< offset must point to AVRational
    AV_OPT_TYPE_DURATION, AV_OPT_TYPE_COLOR, AV_OPT_TYPE_CHANNEL_LAYOUT,
    AV_OPT_TYPE_BOOL


  AVMediaType* = enum
    AVMEDIA_TYPE_UNKNOWN = -1,  ## /< Usually treated as AVMEDIA_TYPE_DATA
    AVMEDIA_TYPE_VIDEO, AVMEDIA_TYPE_AUDIO, AVMEDIA_TYPE_DATA, ## /< Opaque data information usually continuous
    AVMEDIA_TYPE_SUBTITLE, AVMEDIA_TYPE_ATTACHMENT, ## /< Opaque data information usually sparse
    AVMEDIA_TYPE_NB

const
  MAX_STD_TIMEBASES* = (30 * 12 + 30 + 3 + 6)

type
  AVPixelFormat* = enum
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
    AV_PIX_FMT_BGR555LE, ## /< packed BGR 5:5:5, 16bpp, (msb)1X 5B 5G 5R(lsb), little-endian, X=unused/undefined
                        ##  #if FF_API_VAAPI
    AV_PIX_FMT_VAAPI_MOCO,    ## /< HW acceleration through VA API at motion compensation entry-point, Picture.data[3] contains a vaapi_render_state struct which contains macroblocks as well as various fields extracted from headers
    AV_PIX_FMT_VAAPI_IDCT,    ## /< HW acceleration through VA API at IDCT entry-point, Picture.data[3] contains a vaapi_render_state struct which contains fields extracted from headers
    AV_PIX_FMT_VAAPI_VLD,     ## /< HW decoding through VA API, Picture.data[3] contains a VASurfaceID
    AV_PIX_FMT_VAAPI,         ##  #endif
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
    AV_PIX_FMT_BAYER_BGGR8,   ## /< bayer, BGBG..(odd line), GRGR..(even line), 8-bit samples */
    AV_PIX_FMT_BAYER_RGGB8,   ## /< bayer, RGRG..(odd line), GBGB..(even line), 8-bit samples */
    AV_PIX_FMT_BAYER_GBRG8,   ## /< bayer, GBGB..(odd line), RGRG..(even line), 8-bit samples */
    AV_PIX_FMT_BAYER_GRBG8,   ## /< bayer, GRGR..(odd line), BGBG..(even line), 8-bit samples */
    AV_PIX_FMT_BAYER_BGGR16LE, ## /< bayer, BGBG..(odd line), GRGR..(even line), 16-bit samples, little-endian */
    AV_PIX_FMT_BAYER_BGGR16BE, ## /< bayer, BGBG..(odd line), GRGR..(even line), 16-bit samples, big-endian */
    AV_PIX_FMT_BAYER_RGGB16LE, ## /< bayer, RGRG..(odd line), GBGB..(even line), 16-bit samples, little-endian */
    AV_PIX_FMT_BAYER_RGGB16BE, ## /< bayer, RGRG..(odd line), GBGB..(even line), 16-bit samples, big-endian */
    AV_PIX_FMT_BAYER_GBRG16LE, ## /< bayer, GBGB..(odd line), RGRG..(even line), 16-bit samples, little-endian */
    AV_PIX_FMT_BAYER_GBRG16BE, ## /< bayer, GBGB..(odd line), RGRG..(even line), 16-bit samples, big-endian */
    AV_PIX_FMT_BAYER_GRBG16LE, ## /< bayer, GRGR..(odd line), BGBG..(even line), 16-bit samples, little-endian */
    AV_PIX_FMT_BAYER_GRBG16BE, ## /< bayer, GRGR..(odd line), BGBG..(even line), 16-bit samples, big-endian */
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
    AV_PIX_FMT_NB

type
  OutputFormat* = enum
    FMT_MPEG1, FMT_H261, FMT_H263, FMT_MJPEG

const MAX_THREADS = 32



  
const
  AV_SYNC_AUDIO_MASTER* = 0     ##  default choice
  AV_SYNC_VIDEO_MASTER* = 1
  AV_SYNC_EXTERNAL_CLOCK* = 2   ##  synchronize to an external clock
  AV_PIX_FMT_Y400A = AV_PIX_FMT_YA8
  AV_PIX_FMT_GRAY8A = AV_PIX_FMT_YA8
  AV_PIX_FMT_GBR24P = AV_PIX_FMT_GBRP
  AV_FRAME_CROP_UNALIGNED* = 1 shl 0
  STATE_INPUT_READY* = 0
  STATE_SETTING_UP* = 1
  STATE_GET_BUFFER* = 2
  STATE_GET_FORMAT* = 3
  STATE_SETUP_FINISHED* = 4
  AV_OPT_FLAG_IMPLICIT_KEY* = 1
  AV_BUFFERSRC_FLAG_NO_CHECK_FORMAT* = 1
  AV_BUFFERSRC_FLAG_PUSH* = 4
  AV_BUFFERSRC_FLAG_KEEP_REF* = 8
  AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX* = 0x00000001
  AV_CODEC_HW_CONFIG_METHOD_HW_FRAMES_CTX* = 0x00000002
  AV_CODEC_HW_CONFIG_METHOD_INTERNAL* = 0x00000004
  AV_CODEC_HW_CONFIG_METHOD_AD_HOC* = 0x00000008

type
  AVDictionaryEntry* {.bycopy.} = object
    key*: cstring
    value*: cstring

  AVDictionary* {.bycopy.} = object
    count*: cint
    elems*: ptr AVDictionaryEntry

  AVRational* {.bycopy.} = object
    num*: cint                 ## /< Numerator
    den*: cint                 ## /< Denominator

var sws_dict*: ptr AVDictionary
var swr_opts*: ptr AVDictionary
var
  format_opts*: ptr AVDictionary
  codec_opts*: ptr AVDictionary
  resample_opts*: ptr AVDictionary

type
  AVDurationEstimationMethod* = enum
    AVFMT_DURATION_FROM_PTS,  ## /< Duration accurately estimated from PTSes
    AVFMT_DURATION_FROM_STREAM, ## /< Duration estimated from a stream with a known duration
    AVFMT_DURATION_FROM_BITRATE ## /< Duration estimated from bitrate (less accurate)


type
  AVFieldOrder* = enum
    AV_FIELD_UNKNOWN, AV_FIELD_PROGRESSIVE, AV_FIELD_TT, ## < Top coded_first, top displayed first
    AV_FIELD_BB,              ## < Bottom coded first, bottom displayed first
    AV_FIELD_TB,              ## < Top coded first, bottom displayed first
    AV_FIELD_BT               ## < Bottom coded first, top displayed first


type
  AVAudioServiceType* = enum
    AV_AUDIO_SERVICE_TYPE_MAIN = 0, AV_AUDIO_SERVICE_TYPE_EFFECTS = 1,
    AV_AUDIO_SERVICE_TYPE_VISUALLY_IMPAIRED = 2,
    AV_AUDIO_SERVICE_TYPE_HEARING_IMPAIRED = 3, AV_AUDIO_SERVICE_TYPE_DIALOGUE = 4,
    AV_AUDIO_SERVICE_TYPE_COMMENTARY = 5, AV_AUDIO_SERVICE_TYPE_EMERGENCY = 6,
    AV_AUDIO_SERVICE_TYPE_VOICE_OVER = 7, AV_AUDIO_SERVICE_TYPE_KARAOKE = 8, AV_AUDIO_SERVICE_TYPE_NB ## /< Not part of ABI


type
  ShowMode* = enum
    SHOW_MODE_NONE = -1, SHOW_MODE_VIDEO = 0, SHOW_MODE_WAVES, SHOW_MODE_RDFT,
    SHOW_MODE_NB


type
  AVIODataMarkerType* = enum
    AVIO_DATA_MARKER_HEADER, AVIO_DATA_MARKER_SYNC_POINT,
    AVIO_DATA_MARKER_BOUNDARY_POINT, AVIO_DATA_MARKER_UNKNOWN,
    AVIO_DATA_MARKER_TRAILER, AVIO_DATA_MARKER_FLUSH_POINT

type
  AVStreamParseType* = enum
    AVSTREAM_PARSE_NONE, AVSTREAM_PARSE_FULL, 
    AVSTREAM_PARSE_HEADERS,  
    AVSTREAM_PARSE_TIMESTAMPS, 
    AVSTREAM_PARSE_FULL_ONCE, 
    AVSTREAM_PARSE_FULL_RAW 
                                                  
type
  SwrDitherType* = enum
    SWR_DITHER_NONE = 0, SWR_DITHER_RECTANGULAR, SWR_DITHER_TRIANGULAR,
    SWR_DITHER_TRIANGULAR_HIGHPASS, SWR_DITHER_NS = 64, 
    SWR_DITHER_NS_LIPSHITZ, SWR_DITHER_NS_F_WEIGHTED,
    SWR_DITHER_NS_MODIFIED_E_WEIGHTED, SWR_DITHER_NS_IMPROVED_E_WEIGHTED,
    SWR_DITHER_NS_SHIBATA, SWR_DITHER_NS_LOW_SHIBATA, SWR_DITHER_NS_HIGH_SHIBATA, SWR_DITHER_NB


type
  SwrEngine* = enum
    SWR_ENGINE_SWR,          
    SWR_ENGINE_SOXR,      
    SWR_ENGINE_NB        

type
  SwrFilterType* = enum
    SWR_FILTER_TYPE_CUBIC,    ## *< Cubic
    SWR_FILTER_TYPE_BLACKMAN_NUTTALL, ## *< Blackman Nuttall windowed sinc
    SWR_FILTER_TYPE_KAISER    ## *< Kaiser windowed sinc

const
  AV_PARSER_PTS_NB* = 4
  PARSER_FLAG_COMPLETE_FRAMES* = 0x00000001
  PARSER_FLAG_ONCE* = 0x00000002
  PARSER_FLAG_FETCHED_OFFSET* = 0x00000004
  PARSER_FLAG_USE_CODEC_TS* = 0x00001000

const AV_NUM_DATA_POINTERS = 8


type
  AVCodecTag* {.bycopy.} = object
    id*: AVCodecID
    tag*: cuint
  AVOptionUnion* {.bycopy.} = object {.union.}
    i64*: int64
    dbl*: cdouble
    str*: cstring             
    q*: AVRational
  AVOption* {.bycopy.} = object
    name*: cstring
    help*: cstring
    offset*: cint
    `type`*: AVOptionType
    default_val*: AVOptionUnion
    min*: cdouble              
    max*: cdouble           
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
    range*: ref AVOptionRange
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
  AVProbeData* {.bycopy.} = object
    filename*: cstring
    buf*: ptr cuchar            
    buf_size*: cint            
    mime_type*: cstring        
  AVIOContext* {.bycopy.} = object
    av_class*: ptr AVClass
    buffer*: ptr cuchar         ## *< Start of the buffer.
    buffer_size*: cint         ## *< Maximum buffer size
    buf_ptr*: ptr cuchar        ## *< Current position in the buffer
    buf_end*: ptr cuchar 
    opaque*: pointer                            
    read_packet*: proc (opaque: pointer; buf: ptr uint8; buf_size: cint): cint
    write_packet*: proc (opaque: pointer; buf: ptr uint8; buf_size: cint): cint
    seek*: proc (opaque: pointer; offset: int64; whence: cint): int64
    pos*: int64              ## *< position in the file of the current buffer
    eof_reached*: cint         ## *< true if was unable to read due to error or eof
    write_flag*: cint          ## *< true if open for writing
    max_packet_size*: cint
    checksum*: culong
    checksum_ptr*: ptr cuchar
    update_checksum*: proc (checksum: culong; buf: ptr uint8; size: cuint): culong
    error*: cint               
    read_pause*: proc (opaque: pointer; pause: cint): cint
    read_seek*: proc (opaque: pointer; stream_index: cint; timestamp: int64;flags: cint): int64
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
    write_data_type*: proc (opaque: pointer; buf: ptr uint8; buf_size: cint;`type`: AVIODataMarkerType; time: int64): cint
    ignore_boundary_point*: cint
    current_type*: AVIODataMarkerType
    last_time*: int64
    short_seek_get*: proc (opaque: pointer): cint
    written*: int64
    buf_ptr_max*: ptr cuchar
    min_packet_size*: cint
  AVBufferRef = ref object
  AVBuffer* = object
    buffer*: ptr AVBuffer
    data*: ptr uint8
    size*: cint
  AVPacketSideData* {.bycopy.} = object
    data*: ptr uint8
    size*: cint
    `type`*: AVPacketSideDataType
  AVPacket* {.bycopy.} = object
    buf*: ptr AVBufferRef
    pts*: int64
    dts*: int64
    data*: uint8
    size*: cint
    stream_index*: cint
    flags*: cint
    side_data*: ptr AVPacketSideData
    side_data_elems*: cint
    duration*: int64
    pos*: int64   
  AVCodecParameters* {.bycopy.} = object
    codec_type*: AVMediaType
    codec_id*: AVCodecID
    codec_tag*: uint32
    extradata*: uint8
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
  AVStreamInnerStruct* {.bycopy.} = object
    last_dts*: int64
    duration_gcd*: int64
    duration_count*: cint
    rfps_duration_sum*: int64
    duration_error*: array[2, array[MAX_STD_TIMEBASES, cdouble]]
    codec_info_duration*: int64
    codec_info_duration_fields*: int64
    frame_delay_evidence*: cint
    found_decoder*: cint
    last_duration*: int64
    fps_first_dts*: int64
    fps_first_dts_idx*: cint
    fps_last_dts*: int64
    fps_last_dts_idx*: cint
  AVProfile* {.bycopy.} = object
    profile*: cint
    name*: cstring             
  AVCodecDefault* {.bycopy.} = object
    key*: ptr uint8
    value*: ptr uint8
  AVSubtitleRect* = ref object
    x*: cint                   ## /< top left corner  of pict, undefined when pict is not set
    y*: cint                   ## /< top left corner  of pict, undefined when pict is not set
    w*: cint                   ## /< width            of pict, undefined when pict is not set
    h*: cint                   ## /< height           of pict, undefined when pict is not set
    nb_colors*: cint           ## /< number of colors in pict, undefined when pict is not set
    data*: array[4, ptr uint8]
    linesize*: array[4, cint]
    `type`*: AVSubtitleType
    text*: cstring             ## /< 0 terminated plain UTF-8 text
    ass*: cstring
    flags*: cint
  AVSubtitle* {.bycopy.} = object
    format*: uint16         ##  0 = graphics
    start_display_time*: uint32 ##  relative to packet pts, in ms
    end_display_time*: uint32 ##  relative to packet pts, in ms
    num_rects*: cuint
    rects*: AVSubtitleRect
    pts*: int64              
  AVFrameSideData* {.bycopy.} = object
    `type`*: AVFrameSideDataType
    data*: ptr uint8
    size*: cint
    metadata*: ptr AVDictionary
    buf*: ptr AVBufferRef
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
    pkt_dts*: int64
    coded_picture_number*: cint
    display_picture_number*: cint
    quality*: cint
    opaque*: pointer
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
    hw_frames_ctx*: ptr AVBufferRef
    opaque_ref*: ptr AVBufferRef
    crop_top*: csize_t
    crop_bottom*: csize_t
    crop_left*: csize_t
    crop_right*: csize_t
    private_ref*: ptr AVBufferRef
  ScanTable* {.bycopy.} = object
    scantable*: ptr uint8
    permutated*: array[64, uint8]
    raster_end*: array[64, uint8]

type
  ThreadFrame* {.bycopy.} = object
    f*: ptr AVFrame
    owner*: array[2, ptr AVCodecContext]
    progress*: ptr AVBufferRef

  Picture* {.bycopy.} = object
    f*: ptr AVFrame
    tf*: ThreadFrame
    qscale_table_buf*: ptr AVBufferRef
    qscale_table*: ptr int8
    motion_val_buf*: array[2, ptr AVBufferRef] ##  int16 (*motion_val[2])[2];
    mb_type_buf*: ptr AVBufferRef
    mb_type*: ptr uint32      ## /< types and macros are defined in mpegutils.h
    mbskip_table_buf*: ptr AVBufferRef
    mbskip_table*: ptr uint8
    ref_index_buf*: array[2, ptr AVBufferRef]
    ref_index*: array[2, ptr int8]
    mb_var_buf*: ptr AVBufferRef
    mb_var*: ptr uint16       ## /< Table for MB variances
    mc_mb_var_buf*: ptr AVBufferRef
    mc_mb_var*: ptr uint16    ## /< Table for motion compensated MB variances
    alloc_mb_width*: cint      ## /< mb_width used to allocate tables
    alloc_mb_height*: cint     ## /< mb_height used to allocate tables
    mb_mean_buf*: ptr AVBufferRef
    mb_mean*: ptr uint8       ## /< Table for MB luminance
    hwaccel_priv_buf*: ptr AVBufferRef
    hwaccel_picture_private*: pointer ## /< Hardware accelerator private data
    field_picture*: cint       ## /< whether or not the picture was encoded in separate fields
    mb_var_sum*: int64       ## /< sum of MB variance for current frame
    mc_mb_var_sum*: int64    ## /< motion compensated MB variance for current frame
    b_frame_score*: cint
    needs_realloc*: cint       ## /< Picture needs to be reallocated (eg due to a frame size change)
    reference*: cint
    shared*: cint
    encoding_error*: array[AV_NUM_DATA_POINTERS, uint64]

  PutBitContext* {.bycopy.} = object
    bit_buf*: uint64
    bit_left*: cint
    buf*: ptr int8
    buf_ptr*: ptr int8
    buf_end*: ptr int8
    size_in_bits*: cint


  ScratchpadContext* {.bycopy.} = object
    edge_emu_buffer*: ptr uint8 
    rd_scratchpad*: ptr uint8 
    obmc_scratchpad*: ptr uint8
    b_scratchpad*: ptr uint8  


  op_fill_func* = proc (`block`: ptr uint8; ##  align width (8 or 16)
                     value: uint8; line_size: int64; h: cint)
  BlockDSPContext* {.bycopy.} = object
    clear_block*: proc (`block`: ptr int16) ##  align 32
    clear_blocks*: proc (blocks: ptr int16) ##  align 32
    fill_block_tab*: array[2, op_fill_func]

  FDCTDSPContext* {.bycopy.} = object
    fdct*: proc (`block`: ptr int16)
    fdct248*: proc (`block`: ptr int16)

  h264_chroma_mc_func* = proc (dst: ptr uint8; src: ptr uint8; srcStride: int64; h: cint;
                            x: cint; y: cint)
  H264ChromaContext* {.bycopy.} = object
    put_h264_chroma_pixels_tab*: array[4, h264_chroma_mc_func]
    avg_h264_chroma_pixels_tab*: array[4, h264_chroma_mc_func]

  op_pixels_func* = proc (`block`: ptr uint8; pixels: ptr uint8; line_size: int64; h: cint)
  HpelDSPContext* {.bycopy.} = object
    put_pixels_tab*: array[4, array[4, op_pixels_func]]
    avg_pixels_tab*: array[4, array[4, op_pixels_func]]
    put_no_rnd_pixels_tab*: array[4, array[4, op_pixels_func]]
    avg_no_rnd_pixels_tab*: array[4, op_pixels_func]

  idct_permutation_type* = enum
    FF_IDCT_PERM_NONE, FF_IDCT_PERM_LIBMPEG2, FF_IDCT_PERM_SIMPLE,
    FF_IDCT_PERM_TRANSPOSE, FF_IDCT_PERM_PARTTRANS, FF_IDCT_PERM_SSE2

  IDCTDSPContext* {.bycopy.} = object
    put_pixels_clamped*: proc (`block`: ptr int16; pixels: ptr uint8; line_size: int64)
    put_signed_pixels_clamped*: proc (`block`: ptr int16; pixels: ptr uint8;
                                    line_size: int64)
    add_pixels_clamped*: proc (`block`: ptr int16; pixels: ptr uint8; line_size: int64)
    idct*: proc (`block`: ptr int16)
    idct_put*: proc (dest: ptr uint8; line_size: int64; `block`: ptr int16)
    idct_add*: proc (dest: ptr uint8; line_size: int64; `block`: ptr int16)
    idct_permutation*: array[64, uint8]
    perm_type*: idct_permutation_type
    mpeg4_studio_profile*: cint

  MpegEncContext* {.bycopy.} = object

  me_cmp_func* = proc (c: ptr MpegEncContext; blk1: ptr uint8; ##  align width (8 or 16)
                    blk2: ptr uint8; ##  align 1
                    stride: int64; h: cint): cint
  MECmpContext* {.bycopy.} = object
    sum_abs_dctelem*: proc (`block`: ptr int16): cint ##  align 16
    sad*: array[6, me_cmp_func] ##  identical to pix_absAxA except additional void *
    sse*: array[6, me_cmp_func]
    hadamard8_diff*: array[6, me_cmp_func]
    dct_sad*: array[6, me_cmp_func]
    quant_psnr*: array[6, me_cmp_func]
    bit*: array[6, me_cmp_func]
    rd*: array[6, me_cmp_func]
    vsad*: array[6, me_cmp_func]
    vsse*: array[6, me_cmp_func]
    nsse*: array[6, me_cmp_func]
    w53*: array[6, me_cmp_func]
    w97*: array[6, me_cmp_func]
    dct_max*: array[6, me_cmp_func]
    dct264_sad*: array[6, me_cmp_func]
    me_pre_cmp*: array[6, me_cmp_func]
    me_cmp*: array[6, me_cmp_func]
    me_sub_cmp*: array[6, me_cmp_func]
    mb_cmp*: array[6, me_cmp_func]
    ildct_cmp*: array[6, me_cmp_func] ##  only width 16 used
    frame_skip_cmp*: array[6, me_cmp_func] ##  only width 8 used
    pix_abs*: array[2, array[4, me_cmp_func]]
    median_sad*: array[6, me_cmp_func]

  atomic_int* = cint
  ERPicture* {.bycopy.} = object
    f*: ptr AVFrame
    tf*: ptr ThreadFrame        ##  int16 (*motion_val[2])[2];
    ref_index*: array[2, ptr int8]
    mb_type*: ptr uint32
    field_picture*: cint

  ERContext* {.bycopy.} = object
    avctx*: ptr AVCodecContext
    mecc*: MECmpContext
    mecc_inited*: cint
    mb_index2xy*: ptr cint
    mb_num*: cint
    mb_width*: cint
    mb_height*: cint
    mb_stride*: int64
    b8_stride*: int64
    error_count*: atomic_int
    error_occurred*: cint
    error_status_table*: ptr uint8
    er_temp_buffer*: ptr uint8
    dc_val*: array[3, ptr int16]
    mbskip_table*: ptr uint8
    mbintra_table*: ptr uint8
    mv*: array[2, array[4, array[2, cint]]]
    cur_pic*: ERPicture
    last_pic*: ERPicture
    next_pic*: ERPicture
    ref_index_buf*: array[2, ptr AVBufferRef]
    motion_val_buf*: array[2, ptr AVBufferRef]
    pp_time*: uint16
    pb_time*: uint16
    quarter_sample*: cint
    partitioned_frame*: cint
    ref_count*: cint
    decode_mb*: proc (opaque: pointer; `ref`: cint; mv_dir: cint; mv_type: cint;
                    mv: array[2, array[4, array[2, cint]]]; mb_x: cint; mb_y: cint;
                    mb_intra: cint; mb_skipped: cint)
    opaque*: pointer

  AVTimecode* {.bycopy.} = object
    start*: cint               ## /< timecode frame start (first base frame number)
    flags*: uint32           ## /< flags such as drop frame, +24 hours support, ...
    rate*: AVRational          ## /< frame rate in rational form
    fps*: cuint                ## /< frame per second; must be consistent with the rate field

  MpegVideoDSPContext* {.bycopy.} = object
    gmc1*: proc (dst: ptr uint8;  src: ptr uint8; srcStride: cint; h: cint; x16: cint; y16: cint; rounder: cint)
    gmc*: proc (dst: ptr uint8;src: ptr uint8;stride: cint; h: cint; ox: cint; oy: cint; dxx: cint; dxy: cint; dyx: cint;dyy: cint; shift: cint; r: cint; width: cint; height: cint)

  MpegvideoEncDSPContext* {.bycopy.} = object
    try_8x8basis*: proc (rem: array[64, int16]; weight: array[64, int16];
                       basis: array[64, int16]; scale: cint): cint
    add_8x8basis*: proc (rem: array[64, int16]; basis: array[64, int16]; scale: cint)
    pix_sum*: proc (pix: ptr uint8; line_size: cint): cint
    pix_norm1*: proc (pix: ptr uint8; line_size: cint): cint
    # void (*shrink[4])(uint8 *dst, int dst_wrap, const uint8 *src,int src_wrap, int width, int height);
    draw_edges*: proc (buf: ptr uint8; wrap: cint; width: cint; height: cint; w: cint;h: cint; sides: cint)

  PixblockDSPContext* {.bycopy.} = object
    get_pixels*: proc (`block`: ptr int16; pixels: ptr uint8; stride: int64)
    get_pixels_unaligned*: proc (`block`: ptr int16; pixels: ptr uint8; stride: int64)
    diff_pixels*: proc (`block`: ptr int16; s1: ptr uint8; s2: ptr uint8; stride: int64)
    diff_pixels_unaligned*: proc (`block`: ptr int16; s1: ptr uint8; s2: ptr uint8;stride: int64)

  qpel_mc_func* = proc (dst: ptr uint8; src: ptr uint8; stride: int64)
  QpelDSPContext* {.bycopy.} = object
    put_qpel_pixels_tab*: array[2, array[16, qpel_mc_func]]
    avg_qpel_pixels_tab*: array[2, array[16, qpel_mc_func]]
    put_no_rnd_qpel_pixels_tab*: array[2, array[16, qpel_mc_func]]

  VideoDSPContext* {.bycopy.} = object
    emulated_edge_mc*: proc (dst: ptr uint8; src: ptr uint8; dst_linesize: int64;src_linesize: int64; block_w: cint; block_h: cint;src_x: cint; src_y: cint; w: cint; h: cint)
    prefetch*: proc (buf: ptr uint8; stride: int64; h: cint)

  H263DSPContext* {.bycopy.} = object
    h263_h_loop_filter*: proc (src: ptr uint8; stride: cint; qscale: cint)
    h263_v_loop_filter*: proc (src: ptr uint8; stride: cint; qscale: cint)
  AVCodecHWConfig* {.bycopy.} = object
    pix_fmt*: AVPixelFormat
    methods*: cint
    device_type*: AVHWDeviceType
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

  AVCodecHWConfigInternal* {.bycopy.} = object
    public*: AVCodecHWConfig
    hwaccel*: ptr AVHWAccel
  
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
    max_lowres*: uint8
    priv_class*: ptr AVClass    ## /< AVClass for the private context
    profiles*: ptr AVProfile    ## /< array of recognized profiles, or NULL if unknown, array is terminated by {FF_PROFILE_UNKNOWN}
    wrapper_name*: cstring
    priv_data_size*: cint
    next*: ptr AVCodec
    init_thread_copy*: proc (a1: ptr AVCodecContext): cint
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
    send_frame*: proc (avctx: ptr AVCodecContext; frame: ptr AVFrame): cint
    receive_packet*: proc (avctx: ptr AVCodecContext; avpkt: ptr AVPacket): cint
    receive_frame*: proc (avctx: ptr AVCodecContext; frame: ptr AVFrame): cint
    flush*: proc (a1: ptr AVCodecContext)
    caps_internal*: cint
    bsfs*: cstring
    hw_configs*: ptr ptr AVCodecHWConfigInternal

  DecodeSimpleContext* {.bycopy.} = object
    in_pkt*: ptr AVPacket

  AVPacketList* {.bycopy.} = object
    pkt*: AVPacket
    next*: ptr AVPacketList

  EncodeSimpleContext* {.bycopy.} = object
    in_frame*: ptr AVFrame

  AVBitStreamFilter* {.bycopy.} = object
    name*: cstring
    codec_ids*: ptr AVCodecID
    priv_class*: ptr AVClass
    priv_data_size*: cint
    init*: proc (ctx: ptr AVBSFContext): cint
    filter*: proc (ctx: ptr AVBSFContext; pkt: ptr AVPacket): cint
    close*: proc (ctx: ptr AVBSFContext)
    flush*: proc (ctx: ptr AVBSFContext)

  AVBSFInternal* {.bycopy.} = object
    buffer_pkt*: ptr AVPacket
    eof*: cint

  AVBSFContext* {.bycopy.} = object
    av_class*: ptr AVClass
    filter*: ptr AVBitStreamFilter
    internal*: ptr AVBSFInternal
    priv_data*: pointer
    par_in*: ptr AVCodecParameters
    par_out*: ptr AVCodecParameters
    time_base_in*: AVRational
    time_base_out*: AVRational


  AVCodecInternal* {.bycopy.} = object
    is_copy*: cint
    last_audio_frame*: cint
    to_free*: ptr AVFrame
    pool*: ptr AVBufferRef
    thread_ctx*: pointer
    ds*: DecodeSimpleContext
    bsf*: ptr AVBSFContext
    last_pkt_props*: ptr AVPacket
    pkt_props*: ptr AVPacketList
    pkt_props_tail*: ptr AVPacketList
    byte_buffer*: ptr uint8
    byte_buffer_size*: cuint
    frame_thread_encoder*: pointer
    es*: EncodeSimpleContext
    skip_samples*: cint
    hwaccel_priv_data*: pointer
    draining*: cint
    buffer_pkt*: ptr AVPacket
    buffer_frame*: ptr AVFrame
    draining_done*: cint
    compat_decode_warned*: cint
    compat_decode_consumed*: csize_t
    compat_decode_partial_size*: csize_t
    compat_decode_frame*: ptr AVFrame
    compat_encode_packet*: ptr AVPacket
    showed_multi_packet_warning*: cint
    skip_samples_multiplier*: cint
    nb_draining_errors*: cint
    changed_frames_dropped*: cint
    initial_format*: cint
    initial_width*: cint
    initial_height*: cint
    initial_sample_rate*: cint
    initial_channels*: cint
    initial_channel_layout*: uint64

  RcOverride* {.bycopy.} = object
    start_frame*: cint
    end_frame*: cint
    qscale*: cint              ##  If this is 0 then quality_factor will be used instead.
    quality_factor*: cfloat

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
    codec_type*: AVMediaType   ##  see AVMEDIA_TYPE_xxx
    codec*: ptr AVCodec
    codec_id*: AVCodecID       ##  see AV_CODEC_ID_xxx
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
    draw_horiz_band*: proc (s: ptr AVCodecContext; src: ptr AVFrame;offset: array[8, cint]; y: cint;`type`: cint; height: cint)
    get_format*: proc (s: ptr AVCodecContext; fmt: ptr AVPixelFormat): AVPixelFormat
    max_b_frames*: cint
    b_quant_factor*: cfloat
    b_quant_offset*: cfloat
    has_b_frames*: cint
    i_quant_factor*: cfloat
    i_quant_offset*: cfloat
    lumi_masking*: cfloat
    temporal_cplx_masking*: cfloat
    spatial_cplx_masking*: cfloat
    p_masking*: cfloat
    dark_masking*: cfloat
    slice_count*: cint
    slice_offset*: ptr cint
    sample_aspect_ratio*: AVRational
    me_cmp*: cint
    me_sub_cmp*: cint
    mb_cmp*: cint
    ildct_cmp*: cint
    dia_size*: cint
    last_predictor_count*: cint
    me_pre_cmp*: cint
    pre_dia_size*: cint
    me_subpel_quality*: cint
    me_range*: cint
    slice_flags*: cint
    mb_decision*: cint
    intra_matrix*: ptr uint16
    inter_matrix*: ptr uint16
    intra_dc_precision*: cint
    skip_top*: cint
    skip_bottom*: cint
    mb_lmin*: cint
    mb_lmax*: cint
    bidir_refine*: cint
    keyint_min*: cint
    refs*: cint
    mv0_threshold*: cint
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
    trellis*: cint
    stats_out*: cstring
    stats_in*: cstring
    workaround_bugs*: cint
    strict_std_compliance*: cint
    error_concealment*: cint
    debug*: cint
    debug_mv*: cint
    err_recognition*: cint
    reordered_opaque*: int64
    hwaccel*: ptr AVHWAccel
    hwaccel_context*: pointer
    error*: array[8, uint64]
    dct_algo*: cint
    idct_algo*: cint
    bits_per_coded_sample*: cint
    bits_per_raw_sample*: cint
    thread_count*: cint
    thread_type*: cint
    active_thread_type*: cint
    thread_safe_callbacks*: cint
    execute*: proc (c: ptr AVCodecContext;
                  `func`: proc (c2: ptr AVCodecContext; arg: pointer): cint;
                  arg2: pointer; ret: ptr cint; count: cint; size: cint): cint
    execute2*: proc (c: ptr AVCodecContext; `func`: proc (c2: ptr AVCodecContext;
        arg: pointer; jobnr: cint; threadnr: cint): cint; arg2: pointer; ret: ptr cint;
                   count: cint): cint
    nsse_weight*: cint
    profile*: cint
    level*: cint
    skip_loop_filter*: AVDiscard
    skip_idct*: AVDiscard
    skip_frame*: AVDiscard
    subtitle_header*: ptr uint8
    subtitle_header_size*: cint
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

  AVCodecParser* = ref object
    codec_ids*: array[5, cint]  ##  several codec IDs are permitted
    priv_data_size*: cint
    parser_init*: proc (s: ptr AVCodecParserContext): cint
    parser_parse*: proc (s: ptr AVCodecParserContext; avctx: ptr AVCodecContext;
                       poutbuf: ptr ptr uint8; poutbuf_size: ptr cint;
                       buf: ptr uint8; buf_size: cint): cint
    parser_close*: proc (s: ptr AVCodecParserContext)
    split*: proc (avctx: ptr AVCodecContext; buf: ptr uint8; buf_size: cint): cint
    next*: AVCodecParser




  AVCodecParserContext* {.bycopy.} = object
    priv_data*: pointer
    parser*: AVCodecParser
    frame_offset*: int64     ##  offset of the current frame
    cur_offset*: int64       ##  current offset (incremented by each av_parser_parse())
    next_frame_offset*: int64 ##  offset of the next frame
    pict_type*: cint          
    repeat_pict*: cint         
    pts*: int64              ##  pts of the current frame
    dts*: int64              ##  dts of the current frame
    last_pts*: int64
    last_dts*: int64
    fetch_timestamp*: cint
    cur_frame_start_index*: cint
    cur_frame_offset*: array[AV_PARSER_PTS_NB, int64]
    cur_frame_pts*: array[AV_PARSER_PTS_NB, int64]
    cur_frame_dts*: array[AV_PARSER_PTS_NB, int64]
    flags*: cint
    offset*: int64           ## /< byte offset from starting packet start
    cur_frame_end*: array[AV_PARSER_PTS_NB, int64]
    key_frame*: cint
    convergence_duration*: int64
    dts_sync_point*: cint
    dts_ref_dts_delta*: cint
    pts_dts_delta*: cint
    cur_frame_pos*: array[AV_PARSER_PTS_NB, int64]
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
  AVIndexEntry* {.bycopy.} = object
    pos*: int64
    timestamp*: int64
    flags* {.bitsize: 2.}: cint
    size* {.bitsize: 30.}: cint  ## Yeah, trying to keep the size of this small to reduce memory requirements (it is 24 vs. 32 bytes due to possible 8-byte alignment).
    min_distance*: cint  

  AVStreamInternalInner* {.bycopy.} = object
    bsf*: ptr AVBSFContext
    pkt*: ptr AVPacket
    inited*: cint

  FFFrac* {.bycopy.} = object
    val*: int64
    num*: int64
    den*: int64


  AVStreamInternal* {.bycopy.} = object
    reorder*: cint
    bsfc*: ptr AVBSFContext
    bitstream_checked*: cint
    avctx*: ptr AVCodecContext
    avctx_inited*: cint
    orig_codec_id*: AVCodecID
    extract_extradata*: AVStreamInternalInner
    need_context_update*: cint
    is_intra_only*: cint
    priv_pts*: ptr FFFrac


  AVStream* {.bycopy.} = object
    index*: cint               ## *< stream index in AVFormatContext
    id*: cint
    priv_data*: pointer
    time_base*: AVRational
    start_time*: int64
    duration*: int64
    nb_frames*: int64        ## /< number of frames in this stream if known or 0
    disposition*: cint         ## *< AV_DISPOSITION_* bit field
    `discard`*: AVDiscard      ## /< Selects which packets can be discarded at will and do not need to be demuxed.
    sample_aspect_ratio*: AVRational
    metadata*: ptr AVDictionary
    avg_frame_rate*: AVRational
    attached_pic*: AVPacket
    side_data*: ptr AVPacketSideData
    nb_side_data*: cint
    event_flags*: cint
    r_frame_rate*: AVRational
    codecpar*: ptr AVCodecParameters
    info*: ptr AVStreamInnerStruct
    pts_wrap_bits*: cint       
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
    pts_buffer*: array[MAX_REORDER_DELAY + 1, int64]
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
    pts_reorder_error*: array[MAX_REORDER_DELAY + 1, int64]
    pts_reorder_error_count*: array[MAX_REORDER_DELAY + 1, uint8]
    last_dts_for_order_check*: int64
    dts_ordered*: uint8
    dts_misordered*: uint8
    inject_global_side_data*: cint
    display_aspect_ratio*: AVRational
    internal*: ptr AVStreamInternal
  AVProgram* {.bycopy.} = object
    id*: cint
    flags*: cint
    `discard`*: AVDiscard      
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

  AVIOInterruptCB* {.bycopy.} = object
    callback*: proc (a1: pointer): cint
    opaque*: pointer



  AVFormatInternal* {.bycopy.} = object
    nb_interleaved_streams*: cint
    packet_buffer*: ptr AVPacketList
    packet_buffer_end*: ptr AVPacketList
    data_offset*: int64      ## *< offset of the first packet
    raw_packet_buffer*: ptr AVPacketList
    raw_packet_buffer_end*: ptr AVPacketList
    parse_queue*: ptr AVPacketList
    parse_queue_end*: ptr AVPacketList
    raw_packet_buffer_remaining_size*: cint
    offset*: int64
    offset_timebase*: AVRational
    inject_global_side_data*: cint
    avoid_negative_ts_use_pts*: cint
    shortest_end*: int64
    initialized*: cint
    streams_initialized*: cint
    id3v2_meta*: ptr AVDictionary
    prefer_codec_framerate*: cint

  av_format_control_message* = proc (s: ptr AVFormatContext; `type`: cint;data: pointer; data_size: csize_t): cint
  AVFormatContext* {.bycopy.} = object
    av_class*: ptr AVClass
    iformat*: ptr AVInputFormat
    oformat*: ptr AVOutputFormat
    priv_data*: pointer
    pb*: ptr AVIOContext
    ctx_flags*: cint
    nb_streams*: cuint
    streams*: ptr ptr AVStream
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
    protocol_whitelist*: cstring
    io_open*: proc (s: ptr AVFormatContext; pb: ptr ptr AVIOContext; url: cstring;
                  flags: cint; options: ptr ptr AVDictionary): cint
    io_close*: proc (s: ptr AVFormatContext; pb: ptr AVIOContext)
    protocol_blacklist*: cstring
    max_streams*: cint
    skip_estimate_duration_from_pts*: cint
    max_probe_packets*: cint
  AVDeviceInfo* {.bycopy.} = object
    device_name*: cstring      
    device_description*: cstring 
  AVDeviceInfoList* {.bycopy.} = object
    devices*: ptr ptr AVDeviceInfo ## *< list of autodetected devices
    nb_devices*: cint          ## *< number of autodetected devices
    default_device*: cint      ## *< index of default device or -1 if no default

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


  AVOutputFormat* {.bycopy.} = object
    name*: cstring
    long_name*: cstring
    mime_type*: cstring
    extensions*: cstring       ## *< comma-separated filename extensions
    audio_codec*: AVCodecID    ## *< default audio codec
    video_codec*: AVCodecID    ## *< default video codec
    subtitle_codec*: AVCodecID ## *< default subtitle codec
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
    data_codec*: AVCodecID     ## *< default data codec
    init*: proc (a1: ptr AVFormatContext): cint
    deinit*: proc (a1: ptr AVFormatContext)
    check_bitstream*: proc (a1: ptr AVFormatContext; pkt: ptr AVPacket): cint
  AVInputFormat* {.bycopy.} = object
    name*: cstring
    long_name*: cstring
    flags*: cint
    extensions*: cstring
    codec_tag*: ptr ptr AVCodecTag
    priv_class*: ptr AVClass    
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

  Frame* {.bycopy.} = object
    frame*: ptr AVFrame
    sub*: AVSubtitle
    serial*: cint
    pts*: cdouble              ##  presentation timestamp for the frame
    duration*: cdouble         ##  estimated duration of the frame
    pos*: int64             ##  byte position of the frame in the input file
    width*: cint
    height*: cint
    format*: cint
    sar*: AVRational
    uploaded*: cint
    flip_v*: cint

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
    # mutex*: ptr SDL_mutex
    # cond*: ptr SDL_cond


  FrameQueue* {.bycopy.} = object
    queue*: array[16, Frame]
    rindex*: cint
    windex*: cint
    size*: cint
    max_size*: cint
    keep_last*: cint
    rindex_shown*: cint
    # mutex*: ptr SDL_mutex
    # cond*: ptr SDL_cond
    pktq*: ptr PacketQueue

  Decoder* {.bycopy.} = object
    pkt*: AVPacket
    queue*: ptr PacketQueue
    avctx*: ptr AVCodecContext
    pkt_serial*: cint
    finished*: cint
    packet_pending*: cint
    # empty_queue_cond*: ptr SDL_cond
    start_pts*: int64
    start_pts_tb*: AVRational
    next_pts*: int64
    next_pts_tb*: AVRational
    # decoder_tid*: ptr SDL_Thread

  AudioParams* {.bycopy.} = object
    freq*: cint
    channels*: cint
    channel_layout*: int64
    fmt*: AVSampleFormat
    frame_size*: cint
    bytes_per_sec*: cint

  AudioData* {.bycopy.} = object
    ch*: array[64, ptr uint8] ## /< samples buffer per channel
    data*: ptr uint8          ## /< samples buffer
    ch_count*: cint            ## /< number of channels
    bps*: cint                 ## /< bytes per sample
    count*: cint               ## /< number of samples
    planar*: cint              ## /< 1 if planar audio, 0 otherwise
    fmt*: AVSampleFormat       ## /< sample format


  DitherContext* {.bycopy.} = object
    `method`*: cint
    noise_pos*: cint
    scale*: cfloat
    noise_scale*: cfloat       ## /< Noise scale
    ns_taps*: cint             ## /< Noise shaping dither taps
    ns_scale*: cfloat          ## /< Noise shaping dither scale
    ns_scale_1*: cfloat        ## /< Noise shaping dither scale^-1
    ns_pos*: cint              ## /< Noise shaping dither position
    ns_coeffs*: array[20, cfloat] ## /< Noise shaping filter coefficients
    ns_errors*: array[64, array[2 * 20, cfloat]]
    noise*: AudioData          ## /< noise used for dithering
    temp*: AudioData           ## /< temporary storage when writing into the input buffer isn't possible
    output_sample_bits*: cint  ## /< the number of used output bits, needed to scale dither correctly

  conv_func_type* = proc (po: ptr uint8; pi: ptr uint8; `is`: cint; os: cint;`end`: ptr uint8)
  simd_func_type* = proc (dst: ptr ptr uint8; src: ptr ptr uint8; len: cint)

  AudioConvert* {.bycopy.} = object
    channels*: cint
    in_simd_align_mask*: cint
    out_simd_align_mask*: cint
    conv_f*: conv_func_type
    simd_f*: simd_func_type
    ch_map*: ptr cint
    silence*: array[8, uint8] ## /< silence input sample

type
  ResampleInner* {.bycopy.} = object
    resample_one*: proc (dst: pointer; src: pointer; n: cint; index: int64;incr: int64)
    resample_common*: proc (c: ptr ResampleContext; dst: pointer; src: pointer; n: cint;update_ctx: cint): cint
    resample_linear*: proc (c: ptr ResampleContext; dst: pointer; src: pointer; n: cint;update_ctx: cint): cint

  ResampleContext* {.bycopy.} = object
    av_class*: ptr AVClass
    filter_bank*: ptr uint8
    filter_length*: cint
    filter_alloc*: cint
    ideal_dst_incr*: cint
    dst_incr*: cint
    dst_incr_div*: cint
    dst_incr_mod*: cint
    index*: cint
    frac*: cint
    src_incr*: cint
    compensation_distance*: cint
    phase_count*: cint
    linear*: cint
    filter_type*: SwrFilterType
    kaiser_beta*: cdouble
    factor*: cdouble
    format*: AVSampleFormat
    felem_size*: cint
    filter_shift*: cint
    phase_count_compensation*: cint 
    dsp*: ResampleInner

  resample_init_func* = proc (c: ptr ResampleContext; out_rate: cint; in_rate: cint;
                           filter_size: cint; phase_shift: cint; linear: cint;
                           cutoff: cdouble; format: AVSampleFormat;
                           filter_type: SwrFilterType; kaiser_beta: cdouble;
                           precision: cdouble; cheby: cint; exact_rational: cint): ptr ResampleContext

  resample_free_func* = proc (c: ptr ptr ResampleContext)
  multiple_resample_func* = proc (c: ptr ResampleContext; dst: ptr AudioData;
                               dst_size: cint; src: ptr AudioData; src_size: cint;
                               consumed: ptr cint): cint
  resample_flush_func* = proc (c: ptr SwrContext): cint
  set_compensation_func* = proc (c: ptr ResampleContext; sample_delta: cint;
                              compensation_distance: cint): cint
  get_delay_func* = proc (s: ptr SwrContext; base: int64): int64
  invert_initial_buffer_func* = proc (c: ptr ResampleContext; dst: ptr AudioData;
                                   src: ptr AudioData; src_size: cint;
                                   dst_idx: ptr cint; dst_count: ptr cint): cint
  get_out_samples_func* = proc (s: ptr SwrContext; in_samples: cint): int64

  Resampler* {.bycopy.} = object
    init*: resample_init_func
    free*: resample_free_func
    multiple_resample*: multiple_resample_func
    flush*: resample_flush_func
    set_compensation*: set_compensation_func
    get_delay*: get_delay_func
    invert_initial_buffer*: invert_initial_buffer_func
    get_out_samples*: get_out_samples_func

  mix_1_1_func_type* = proc (`out`: pointer; `in`: pointer; coeffp: pointer;
                          index: int64; len: int64)
  mix_2_1_func_type* = proc (`out`: pointer; in1: pointer; in2: pointer; coeffp: pointer;
                          index1: int64; index2: int64; len: int64)
  mix_any_func_type* = proc (`out`: ptr ptr int8; in1: ptr ptr int8; coeffp: pointer;
                          len: int64)


  SwrContext* {.bycopy.} = object
    av_class*: ptr AVClass      ## /< AVClass used
    log_level_offset*: cint    ## /< logging level offset
    log_ctx*: pointer          ## /< parent logging context
    in_sample_fmt*: AVSampleFormat ## /< input sample format
    int_sample_fmt*: AVSampleFormat ## /< internal sample format (AV_SAMPLE_FMT_FLTP or AV_SAMPLE_FMT_S16P)
    out_sample_fmt*: AVSampleFormat ## /< output sample format
    in_ch_layout*: int64     ## /< input channel layout
    out_ch_layout*: int64    ## /< output channel layout
    in_sample_rate*: cint      ## /< input sample rate
    out_sample_rate*: cint     ## /< output sample rate
    flags*: cint               ## /< miscellaneous flags such as SWR_FLAG_RESAMPLE
    slev*: cfloat              ## /< surround mixing level
    clev*: cfloat              ## /< center mixing level
    lfe_mix_level*: cfloat     ## /< LFE mixing level
    rematrix_volume*: cfloat   ## /< rematrixing volume coefficient
    rematrix_maxval*: cfloat   ## /< maximum value for rematrixing output
    matrix_encoding*: cint     ## *< matrixed stereo encoding
    channel_map*: ptr cint      ## /< channel index (or -1 if muted channel) map
    used_ch_count*: cint       ## /< number of used input channels (mapped channel count if channel_map, otherwise in.ch_count)
    engine*: cint
    user_in_ch_count*: cint    ## /< User set input channel count
    user_out_ch_count*: cint   ## /< User set output channel count
    user_used_ch_count*: cint  ## /< User set used channel count
    user_in_ch_layout*: int64 ## /< User set input channel layout
    user_out_ch_layout*: int64 ## /< User set output channel layout
    user_int_sample_fmt*: AVSampleFormat ## /< User set internal sample format
    user_dither_method*: cint  ## /< User set dither method
    dither*: DitherContext
    filter_size*: cint         ## *< length of each FIR filter in the resampling filterbank relative to the cutoff frequency
    phase_shift*: cint         ## *< log2 of the number of entries in the resampling polyphase filterbank
    linear_interp*: cint       ## *< if 1 then the resampling FIR filter will be linearly interpolated
    exact_rational*: cint      ## *< if 1 then enable non power of 2 phase_count
    cutoff*: cdouble           ## *< resampling cutoff frequency (swr: 6dB point; soxr: 0dB point). 1.0 corresponds to half the output sample rate
    filter_type*: cint         ## *< swr resampling filter type
    kaiser_beta*: cdouble      ## *< swr beta value for Kaiser window (only applicable if filter_type == AV_FILTER_TYPE_KAISER)
    precision*: cdouble        ## *< soxr resampling precision (in bits)
    cheby*: cint               ## *< soxr: if 1 then passband rolloff will be none (Chebyshev) & irrational ratio approximation precision will be higher
    min_compensation*: cfloat  ## /< swr minimum below which no compensation will happen
    min_hard_compensation*: cfloat ## /< swr minimum below which no silence inject / sample drop will happen
    soft_compensation_duration*: cfloat ## /< swr duration over which soft compensation is applied
    max_soft_compensation*: cfloat ## /< swr maximum soft compensation in seconds over soft_compensation_duration
    async*: cfloat             ## /< swr simple 1 parameter async, similar to ffmpegs -async
    firstpts_in_samples*: int64 ## /< swr first pts in samples
    resample_first*: cint      ## /< 1 if resampling must come first, 0 if rematrixing
    rematrix*: cint            ## /< flag to indicate if rematrixing is needed (basically if input and output layouts mismatch)
    rematrix_custom*: cint     ## /< flag to indicate that a custom matrix has been defined
    `in`*: AudioData           ## /< input audio data
    postin*: AudioData         ## /< post-input audio data: used for rematrix/resample
    midbuf*: AudioData         ## /< intermediate audio data (postin/preout)
    preout*: AudioData         ## /< pre-output audio data: used for rematrix/resample
    `out`*: AudioData          ## /< converted output audio data
    in_buffer*: AudioData      ## /< cached audio data (convert and resample purpose)
    silence*: AudioData        ## /< temporary with silence
    drop_temp*: AudioData      ## /< temporary used to discard output
    in_buffer_index*: cint     ## /< cached buffer position
    in_buffer_count*: cint     ## /< cached buffer length
    resample_in_constraint*: cint ## /< 1 if the input end was reach before the output end, 0 otherwise
    flushed*: cint             ## /< 1 if data is to be flushed and no further input is expected
    outpts*: int64           ## /< output PTS
    firstpts*: int64         ## /< first PTS
    drop_output*: cint         ## /< number of output samples to drop
    delayed_samples_fixup*: cdouble ## /< soxr 0.1.1: needed to fixup delayed_samples after flush has been called.
    in_convert*: ptr AudioConvert ## /< input conversion context
    out_convert*: ptr AudioConvert ## /< output conversion context
    full_convert*: ptr AudioConvert ## /< full conversion context (single conversion for input and output)
    resample*: ptr ResampleContext ## /< resampling context
    resampler*: ptr Resampler   ## /< resampler virtual function table
    matrix*: array[SWR_CH_MAX, array[SWR_CH_MAX, cdouble]] ## /< floating point rematrixing coefficients
    matrix_flt*: array[SWR_CH_MAX, array[SWR_CH_MAX, cfloat]] ## /< single precision floating point rematrixing coefficients
    native_matrix*: ptr uint8
    native_one*: ptr uint8
    native_simd_one*: ptr uint8
    native_simd_matrix*: ptr uint8
    matrix32*: array[SWR_CH_MAX, array[SWR_CH_MAX, int32]] ## /< 17.15 fixed point rematrixing coefficients
    matrix_ch*: array[SWR_CH_MAX, array[SWR_CH_MAX + 1, uint8]] ## /< Lists of input channels per output channel that have non zero rematrixing coefficients
    mix_1_1_f*: ptr mix_1_1_func_type
    mix_1_1_simd*: ptr mix_1_1_func_type
    mix_2_1_f*: ptr mix_2_1_func_type
    mix_2_1_simd*: ptr mix_2_1_func_type
    mix_any_f*: ptr mix_any_func_type

  FFTComplex* {.bycopy.} = object
    re*: float
    im*: float

  FFTContext* {.bycopy.} = object
    nbits*: cint
    inverse*: cint
    revtab*: ptr uint16
    tmp_buf*: ptr FFTComplex
    mdct_size*: cint           ##  size of MDCT (i.e. number of input data * 2)
    mdct_bits*: cint           ##  n = 2^nbits
    tcos*: ptr float
    tsin*: ptr float
    fft_permute*: proc (s: ptr FFTContext; z: ptr FFTComplex)
    fft_calc*: proc (s: ptr FFTContext; z: ptr FFTComplex)
    imdct_calc*: proc (s: ptr FFTContext; output: ptr float; input: ptr float)
    imdct_half*: proc (s: ptr FFTContext; output: ptr float; input: ptr float)
    mdct_calc*: proc (s: ptr FFTContext; output: ptr float; input: ptr float)
    mdct_calcw*: proc (s: ptr FFTContext; output: ptr float; input: ptr float)
    fft_permutation*: fft_permutation_type
    mdct_permutation*: mdct_permutation_type
    revtab32*: ptr uint32



  RDFTContext* {.bycopy.} = object
    nbits*: cint
    inverse*: cint
    sign_convention*: cint     ##  pre/post rotation tables
    tcos*: ptr float
    tsin*: ptr float
    negative_sin*: cint
    fft*: FFTContext
    rdft_calc*: proc (s: ptr RDFTContext; z: ptr float)

  VideoState* {.bycopy.} = object
    # read_tid*: ptr SDL_Thread
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
    audio_diff_cum*: cdouble   ##  used for AV difference average computation
    audio_diff_avg_coef*: cdouble
    audio_diff_threshold*: cdouble
    audio_diff_avg_count*: cint
    audio_st*: ptr AVStream
    audioq*: PacketQueue
    audio_hw_buf_size*: cint
    audio_buf*: ptr uint8
    audio_buf1*: ptr uint8
    audio_buf_size*: cuint     ##  in bytes
    audio_buf1_size*: cuint
    audio_buf_index*: cint     ##  in bytes
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
    sample_array*: array[8 * 65536, int16]
    sample_array_index*: cint
    last_i_start*: cint
    rdft*: ptr RDFTContext
    rdft_bits*: cint
    rdft_data*: ptr float
    xpos*: cint
    last_vis_time*: cdouble
    vis_texture*: TexturePtr
    sub_texture*: TexturePtr
    vid_texture*: TexturePtr
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
    # continue_read_thread*: ptr SDL_cond
  

  AVFilterPad* {.bycopy.} = object
    name*: cstring
    `type`*: AVMediaType
    get_video_buffer*: proc (link: ptr AVFilterLink; w: cint; h: cint): ptr AVFrame
    get_audio_buffer*: proc (link: ptr AVFilterLink; nb_samples: cint): ptr AVFrame
    filter_frame*: proc (link: ptr AVFilterLink; frame: ptr AVFrame): cint
    request_frame*: proc (link: ptr AVFilterLink): cint
    config_props*: proc (link: ptr AVFilterLink): cint
    needs_writable*: cint

  AVFilterContext* {.bycopy.} = object
    av_class*: ptr AVClass      ## /< needed for  and filters common options
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





proc init_dynload() = 
    echo SetDllDirectory("")


var  flush_pkt:AVPacket

proc main(): cint =
    var flags: cint
    var `is`: ptr VideoState
    init_dynload()
    avdevice_register_all()
    # avformat_network_init()
    init_opts()
    signal(SIGINT, sigterm_handler)
    signal(SIGTERM, sigterm_handler)
    #   show_banner(argc, argv, options)
    parse_options(nil, argc, argv, options, opt_input_file)
    # if not input_filename:
    #     show_usage()
    #     exit(1)
    flags = INIT_VIDEO or INIT_AUDIO or INIT_TIMER
    if init(flags):
        exit(1)
    eventState(SDL_SYSWMEVENT, SDL_IGNORE)
    eventState(SDL_USEREVENT, SDL_IGNORE)
    av_init_packet(addr(flush_pkt))
    flush_pkt.data = cast[ptr uint8](addr(flush_pkt))
    if not display_disable:
        var flags: cint = SDL_WINDOW_HIDDEN
        if alwaysontop:
        flags = flags or SDL_WINDOW_ALWAYS_ON_TOP
        if borderless:
        flags = flags or SDL_WINDOW_BORDERLESS
        else:
        flags = flags or SDL_WINDOW_RESIZABLE
        window = createWindow(program_name, SDL_WINDOWPOS_UNDEFINED,
                                SDL_WINDOWPOS_UNDEFINED, default_width,
                                default_height, flags)
        setHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear")
        if window:
        renderer = createRenderer(window, -1, SDL_RENDERER_ACCELERATED or
            SDL_RENDERER_PRESENTVSYNC)
        if not renderer:
            renderer = createRenderer(window, -1, 0)
        if renderer:
            if not getRendererInfo(renderer, addr(renderer_info)):
            av_log(nil, AV_LOG_VERBOSE, "Initialized %s renderer.\n",
                    renderer_info.name)
        if not window or not renderer or not renderer_info.num_texture_formats:
        do_exit(nil)
    `is` = stream_open(input_filename, file_iformat)
    if not `is`:
        do_exit(nil)
    event_loop(`is`)
    return 0
