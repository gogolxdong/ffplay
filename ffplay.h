#include <inttypes.h>
#include <math.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <gcrypt.h>
#include <pthread.h>
#include <SDL.h>
#ifdef _WIN32
#include <windows.h>
#endif

const char program_name[] = "ffplay";
const int program_birth_year = 2003;

enum BenchAction {
    ACTION_START,
    ACTION_STOP,
    NB_ACTION
};

enum show_muxdemuxers
{
    SHOW_DEFAULT,
    SHOW_DEMUXERS,
    SHOW_MUXERS,
};

typedef enum memory_order
{
    memory_order_relaxed,
    memory_order_consume,
    memory_order_acquire,
    memory_order_release,
    memory_order_acq_rel,
    memory_order_seq_cst
} memory_order;

typedef enum SwsDither
{
    SWS_DITHER_NONE = 0,
    SWS_DITHER_AUTO,
    SWS_DITHER_BAYER,
    SWS_DITHER_ED,
    SWS_DITHER_A_DITHER,
    SWS_DITHER_X_DITHER,
    NB_SWS_DITHER,
} SwsDither;

typedef enum SwsAlphaBlend
{
    SWS_ALPHA_BLEND_NONE = 0,
    SWS_ALPHA_BLEND_UNIFORM,
    SWS_ALPHA_BLEND_CHECKERBOARD,
    SWS_ALPHA_BLEND_NB,
} SwsAlphaBlend;

enum AVRounding
{
    AV_ROUND_ZERO = 0,     ///< Round toward zero.
    AV_ROUND_INF = 1,      ///< Round away from zero.
    AV_ROUND_DOWN = 2,     ///< Round toward -infinity.
    AV_ROUND_UP = 3,       ///< Round toward +infinity.
    AV_ROUND_NEAR_INF = 5, ///< Round to nearest and halfway cases away from zero.
    AV_ROUND_PASS_MINMAX = 8192,
};

enum AVColorRange
{
    AVCOL_RANGE_UNSPECIFIED = 0,
    AVCOL_RANGE_MPEG = 1, ///< the normal 219*2^(n-8) "MPEG" YUV ranges
    AVCOL_RANGE_JPEG = 2, ///< the normal     2^n-1   "JPEG" YUV ranges
    AVCOL_RANGE_NB        ///< Not part of ABI
};

enum AVColorPrimaries
{
    AVCOL_PRI_RESERVED0 = 0,
    AVCOL_PRI_BT709 = 1, ///< also ITU-R BT1361 / IEC 61966-2-4 / SMPTE RP177 Annex B
    AVCOL_PRI_UNSPECIFIED = 2,
    AVCOL_PRI_RESERVED = 3,
    AVCOL_PRI_BT470M = 4, ///< also FCC Title 47 Code of Federal Regulations 73.682 (a)(20)

    AVCOL_PRI_BT470BG = 5,   ///< also ITU-R BT601-6 625 / ITU-R BT1358 625 / ITU-R BT1700 625 PAL & SECAM
    AVCOL_PRI_SMPTE170M = 6, ///< also ITU-R BT601-6 525 / ITU-R BT1358 525 / ITU-R BT1700 NTSC
    AVCOL_PRI_SMPTE240M = 7, ///< functionally identical to above
    AVCOL_PRI_FILM = 8,      ///< colour filters using Illuminant C
    AVCOL_PRI_BT2020 = 9,    ///< ITU-R BT2020
    AVCOL_PRI_SMPTE428 = 10, ///< SMPTE ST 428-1 (CIE 1931 XYZ)
    AVCOL_PRI_SMPTEST428_1 = AVCOL_PRI_SMPTE428,
    AVCOL_PRI_SMPTE431 = 11, ///< SMPTE ST 431-2 (2011) / DCI P3
    AVCOL_PRI_SMPTE432 = 12, ///< SMPTE ST 432-1 (2010) / P3 D65 / Display P3
    AVCOL_PRI_EBU3213 = 22,  ///< EBU Tech. 3213-E / JEDEC P22 phosphors
    AVCOL_PRI_JEDEC_P22 = AVCOL_PRI_EBU3213,
    AVCOL_PRI_NB ///< Not part of ABI
};

enum AVColorTransferCharacteristic
{
    AVCOL_TRC_RESERVED0 = 0,
    AVCOL_TRC_BT709 = 1, ///< also ITU-R BT1361
    AVCOL_TRC_UNSPECIFIED = 2,
    AVCOL_TRC_RESERVED = 3,
    AVCOL_TRC_GAMMA22 = 4,   ///< also ITU-R BT470M / ITU-R BT1700 625 PAL & SECAM
    AVCOL_TRC_GAMMA28 = 5,   ///< also ITU-R BT470BG
    AVCOL_TRC_SMPTE170M = 6, ///< also ITU-R BT601-6 525 or 625 / ITU-R BT1358 525 or 625 / ITU-R BT1700 NTSC
    AVCOL_TRC_SMPTE240M = 7,
    AVCOL_TRC_LINEAR = 8,        ///< "Linear transfer characteristics"
    AVCOL_TRC_LOG = 9,           ///< "Logarithmic transfer characteristic (100:1 range)"
    AVCOL_TRC_LOG_SQRT = 10,     ///< "Logarithmic transfer characteristic (100 * Sqrt(10) : 1 range)"
    AVCOL_TRC_IEC61966_2_4 = 11, ///< IEC 61966-2-4
    AVCOL_TRC_BT1361_ECG = 12,   ///< ITU-R BT1361 Extended Colour Gamut
    AVCOL_TRC_IEC61966_2_1 = 13, ///< IEC 61966-2-1 (sRGB or sYCC)
    AVCOL_TRC_BT2020_10 = 14,    ///< ITU-R BT2020 for 10-bit system
    AVCOL_TRC_BT2020_12 = 15,    ///< ITU-R BT2020 for 12-bit system
    AVCOL_TRC_SMPTE2084 = 16,    ///< SMPTE ST 2084 for 10-, 12-, 14- and 16-bit systems
    AVCOL_TRC_SMPTEST2084 = AVCOL_TRC_SMPTE2084,
    AVCOL_TRC_SMPTE428 = 17, ///< SMPTE ST 428-1
    AVCOL_TRC_SMPTEST428_1 = AVCOL_TRC_SMPTE428,
    AVCOL_TRC_ARIB_STD_B67 = 18, ///< ARIB STD-B67, known as "Hybrid log-gamma"
    AVCOL_TRC_NB                 ///< Not part of ABI
};

enum AVColorSpace
{
    AVCOL_SPC_RGB = 0,   ///< order of coefficients is actually GBR, also IEC 61966-2-1 (sRGB)
    AVCOL_SPC_BT709 = 1, ///< also ITU-R BT1361 / IEC 61966-2-4 xvYCC709 / SMPTE RP177 Annex B
    AVCOL_SPC_UNSPECIFIED = 2,
    AVCOL_SPC_RESERVED = 3,
    AVCOL_SPC_FCC = 4,       ///< FCC Title 47 Code of Federal Regulations 73.682 (a)(20)
    AVCOL_SPC_BT470BG = 5,   ///< also ITU-R BT601-6 625 / ITU-R BT1358 625 / ITU-R BT1700 625 PAL & SECAM / IEC 61966-2-4 xvYCC601
    AVCOL_SPC_SMPTE170M = 6, ///< also ITU-R BT601-6 525 / ITU-R BT1358 525 / ITU-R BT1700 NTSC
    AVCOL_SPC_SMPTE240M = 7, ///< functionally identical to above
    AVCOL_SPC_YCGCO = 8,     ///< Used by Dirac / VC-2 and H.264 FRext, see ITU-T SG16
    AVCOL_SPC_YCOCG = AVCOL_SPC_YCGCO,
    AVCOL_SPC_BT2020_NCL = 9,          ///< ITU-R BT2020 non-constant luminance system
    AVCOL_SPC_BT2020_CL = 10,          ///< ITU-R BT2020 constant luminance system
    AVCOL_SPC_SMPTE2085 = 11,          ///< SMPTE 2085, Y'D'zD'x
    AVCOL_SPC_CHROMA_DERIVED_NCL = 12, ///< Chromaticity-derived non-constant luminance system
    AVCOL_SPC_CHROMA_DERIVED_CL = 13,  ///< Chromaticity-derived constant luminance system
    AVCOL_SPC_ICTCP = 14,              ///< ITU-R BT.2100-0, ICtCp
    AVCOL_SPC_NB                       ///< Not part of ABI
};

enum AVChromaLocation
{
    AVCHROMA_LOC_UNSPECIFIED = 0,
    AVCHROMA_LOC_LEFT = 1,    ///< MPEG-2/4 4:2:0, H.264 default for 4:2:0
    AVCHROMA_LOC_CENTER = 2,  ///< MPEG-1 4:2:0, JPEG 4:2:0, H.263 4:2:0
    AVCHROMA_LOC_TOPLEFT = 3, ///< ITU-R 601, SMPTE 274M 296M S314M(DV 4:1:1), mpeg2 4:2:2
    AVCHROMA_LOC_TOP = 4,
    AVCHROMA_LOC_BOTTOMLEFT = 5,
    AVCHROMA_LOC_BOTTOM = 6,
    AVCHROMA_LOC_NB ///< Not part of ABI
};

enum AVSampleFormat
{
    AV_SAMPLE_FMT_NONE = -1,
    AV_SAMPLE_FMT_U8,  ///< unsigned 8 bits
    AV_SAMPLE_FMT_S16, ///< signed 16 bits
    AV_SAMPLE_FMT_S32, ///< signed 32 bits
    AV_SAMPLE_FMT_FLT, ///< float
    AV_SAMPLE_FMT_DBL, ///< double

    AV_SAMPLE_FMT_U8P,  ///< unsigned 8 bits, planar
    AV_SAMPLE_FMT_S16P, ///< signed 16 bits, planar
    AV_SAMPLE_FMT_S32P, ///< signed 32 bits, planar
    AV_SAMPLE_FMT_FLTP, ///< float, planar
    AV_SAMPLE_FMT_DBLP, ///< double, planar
    AV_SAMPLE_FMT_S64,  ///< signed 64 bits
    AV_SAMPLE_FMT_S64P, ///< signed 64 bits, planar

    AV_SAMPLE_FMT_NB ///< Number of sample formats. DO NOT USE if linking dynamically
};

enum AVDiscard
{
    /* We leave some space between them for extensions (drop some
     * keyframes for intra-only or drop just some bidir frames). */
    AVDISCARD_NONE = -16,    ///< discard nothing
    AVDISCARD_DEFAULT = 0,   ///< discard useless packets like 0 size packets in avi
    AVDISCARD_NONREF = 8,    ///< discard all non reference
    AVDISCARD_BIDIR = 16,    ///< discard all bidirectional frames
    AVDISCARD_NONINTRA = 24, ///< discard all non intra frames
    AVDISCARD_NONKEY = 32,   ///< discard all frames except keyframes
    AVDISCARD_ALL = 48,      ///< discard all
};

enum AVHWDeviceType
{
    AV_HWDEVICE_TYPE_NONE,
    AV_HWDEVICE_TYPE_VDPAU,
    AV_HWDEVICE_TYPE_CUDA,
    AV_HWDEVICE_TYPE_VAAPI,
    AV_HWDEVICE_TYPE_DXVA2,
    AV_HWDEVICE_TYPE_QSV,
    AV_HWDEVICE_TYPE_VIDEOTOOLBOX,
    AV_HWDEVICE_TYPE_D3D11VA,
    AV_HWDEVICE_TYPE_DRM,
    AV_HWDEVICE_TYPE_OPENCL,
    AV_HWDEVICE_TYPE_MEDIACODEC,
};

typedef struct AVDictionaryEntry
{
    char *key;
    char *value;
} AVDictionaryEntry;

struct AVDictionary
{
    int count;
    AVDictionaryEntry *elems;
};

typedef struct AVDictionary AVDictionary;
AVDictionary *sws_dict;
AVDictionary *swr_opts;
AVDictionary *format_opts, *codec_opts, *resample_opts;

typedef struct AVRational
{
    int num; ///< Numerator
    int den; ///< Denominator
} AVRational;

enum AVFrameSideDataType
{
    AV_FRAME_DATA_PANSCAN,
    AV_FRAME_DATA_A53_CC,
    AV_FRAME_DATA_STEREO3D,
    AV_FRAME_DATA_MATRIXENCODING,
    AV_FRAME_DATA_DOWNMIX_INFO,
    AV_FRAME_DATA_REPLAYGAIN,
    AV_FRAME_DATA_DISPLAYMATRIX,
    AV_FRAME_DATA_AFD,
    AV_FRAME_DATA_MOTION_VECTORS,
    AV_FRAME_DATA_SKIP_SAMPLES,
    AV_FRAME_DATA_AUDIO_SERVICE_TYPE,
    AV_FRAME_DATA_MASTERING_DISPLAY_METADATA,
    AV_FRAME_DATA_GOP_TIMECODE,
    AV_FRAME_DATA_SPHERICAL,
    AV_FRAME_DATA_CONTENT_LIGHT_LEVEL,
    AV_FRAME_DATA_ICC_PROFILE,
};

enum AVPictureType
{
    AV_PICTURE_TYPE_NONE = 0, ///< Undefined
    AV_PICTURE_TYPE_I,        ///< Intra
    AV_PICTURE_TYPE_P,        ///< Predicted
    AV_PICTURE_TYPE_B,        ///< Bi-dir predicted
    AV_PICTURE_TYPE_S,        ///< S(GMC)-VOP MPEG-4
    AV_PICTURE_TYPE_SI,       ///< Switching Intra
    AV_PICTURE_TYPE_SP,       ///< Switching Predicted
    AV_PICTURE_TYPE_BI,       ///< BI type
};

enum RDFTransformType
{
    DFT_R2C,
    IDFT_C2R,
    IDFT_R2C,
    DFT_C2R,
};

enum AVPacketSideDataType
{
    AV_PKT_DATA_PALETTE,
    AV_PKT_DATA_NEW_EXTRADATA,
    AV_PKT_DATA_PARAM_CHANGE,
    AV_PKT_DATA_H263_MB_INFO,
    AV_PKT_DATA_REPLAYGAIN,
    AV_PKT_DATA_DISPLAYMATRIX,
    AV_PKT_DATA_STEREO3D,
    AV_PKT_DATA_AUDIO_SERVICE_TYPE,
    AV_PKT_DATA_QUALITY_STATS,
    AV_PKT_DATA_FALLBACK_TRACK,
    AV_PKT_DATA_CPB_PROPERTIES,
    AV_PKT_DATA_SKIP_SAMPLES,
    AV_PKT_DATA_JP_DUALMONO,
    AV_PKT_DATA_SUBTITLE_POSITION,
    AV_PKT_DATA_MATROSKA_BLOCKADDITIONAL,
    AV_PKT_DATA_WEBVTT_IDENTIFIER,
    AV_PKT_DATA_WEBVTT_SETTINGS,
    AV_PKT_DATA_METADATA_UPDATE,
    AV_PKT_DATA_MPEGTS_STREAM_ID,
    AV_PKT_DATA_MASTERING_DISPLAY_METADATA,
    AV_PKT_DATA_SPHERICAL,
    AV_PKT_DATA_CONTENT_LIGHT_LEVEL,
    AV_PKT_DATA_A53_CC,
    AV_PKT_DATA_ENCRYPTION_INIT_INFO,
    AV_PKT_DATA_ENCRYPTION_INFO,
    AV_PKT_DATA_AFD,
    AV_PKT_DATA_NB
};

enum AVSubtitleType
{
    SUBTITLE_NONE,
    SUBTITLE_BITMAP, ///< A bitmap, pict will be set
    SUBTITLE_TEXT,
    SUBTITLE_ASS,
};
enum
{
    AV_SYNC_AUDIO_MASTER, /* default choice */
    AV_SYNC_VIDEO_MASTER,
    AV_SYNC_EXTERNAL_CLOCK, /* synchronize to an external clock */
};

typedef enum
{
    AV_CLASS_CATEGORY_NA = 0,
    AV_CLASS_CATEGORY_INPUT,
    AV_CLASS_CATEGORY_OUTPUT,
    AV_CLASS_CATEGORY_MUXER,
    AV_CLASS_CATEGORY_DEMUXER,
    AV_CLASS_CATEGORY_ENCODER,
    AV_CLASS_CATEGORY_DECODER,
    AV_CLASS_CATEGORY_FILTER,
    AV_CLASS_CATEGORY_BITSTREAM_FILTER,
    AV_CLASS_CATEGORY_SWSCALER,
    AV_CLASS_CATEGORY_SWRESAMPLER,
    AV_CLASS_CATEGORY_DEVICE_VIDEO_OUTPUT = 40,
    AV_CLASS_CATEGORY_DEVICE_VIDEO_INPUT,
    AV_CLASS_CATEGORY_DEVICE_AUDIO_OUTPUT,
    AV_CLASS_CATEGORY_DEVICE_AUDIO_INPUT,
    AV_CLASS_CATEGORY_DEVICE_OUTPUT,
    AV_CLASS_CATEGORY_DEVICE_INPUT,
    AV_CLASS_CATEGORY_NB ///< not part of ABI/API
} AVClassCategory;


enum AVOptionType
{
    AV_OPT_TYPE_FLAGS,
    AV_OPT_TYPE_INT,
    AV_OPT_TYPE_INT64,
    AV_OPT_TYPE_DOUBLE,
    AV_OPT_TYPE_FLOAT,
    AV_OPT_TYPE_STRING,
    AV_OPT_TYPE_RATIONAL,
    AV_OPT_TYPE_BINARY, ///< offset must point to a pointer immediately followed by an int for the length
    AV_OPT_TYPE_DICT,
    AV_OPT_TYPE_UINT64,
    AV_OPT_TYPE_CONST,
    AV_OPT_TYPE_IMAGE_SIZE, ///< offset must point to two consecutive integers
    AV_OPT_TYPE_PIXEL_FMT,
    AV_OPT_TYPE_SAMPLE_FMT,
    AV_OPT_TYPE_VIDEO_RATE, ///< offset must point to AVRational
    AV_OPT_TYPE_DURATION,
    AV_OPT_TYPE_COLOR,
    AV_OPT_TYPE_CHANNEL_LAYOUT,
    AV_OPT_TYPE_BOOL,
};

enum AVMediaType
{
    AVMEDIA_TYPE_UNKNOWN = -1, ///< Usually treated as AVMEDIA_TYPE_DATA
    AVMEDIA_TYPE_VIDEO,
    AVMEDIA_TYPE_AUDIO,
    AVMEDIA_TYPE_DATA, ///< Opaque data information usually continuous
    AVMEDIA_TYPE_SUBTITLE,
    AVMEDIA_TYPE_ATTACHMENT, ///< Opaque data information usually sparse
    AVMEDIA_TYPE_NB
};

enum AVCodecID
{
    AV_CODEC_ID_NONE,

    /* video codecs */
    AV_CODEC_ID_MPEG1VIDEO,
    AV_CODEC_ID_MPEG2VIDEO, ///< preferred ID for MPEG-1/2 video decoding
    AV_CODEC_ID_H261,
    AV_CODEC_ID_H263,
    AV_CODEC_ID_RV10,
    AV_CODEC_ID_RV20,
    AV_CODEC_ID_MJPEG,
    AV_CODEC_ID_MJPEGB,
    AV_CODEC_ID_LJPEG,
    AV_CODEC_ID_SP5X,
    AV_CODEC_ID_JPEGLS,
    AV_CODEC_ID_MPEG4,
    AV_CODEC_ID_RAWVIDEO,
    AV_CODEC_ID_MSMPEG4V1,
    AV_CODEC_ID_MSMPEG4V2,
    AV_CODEC_ID_MSMPEG4V3,
    AV_CODEC_ID_WMV1,
    AV_CODEC_ID_WMV2,
    AV_CODEC_ID_H263P,
    AV_CODEC_ID_H263I,
    AV_CODEC_ID_FLV1,
    AV_CODEC_ID_SVQ1,
    AV_CODEC_ID_SVQ3,
    AV_CODEC_ID_DVVIDEO,
    AV_CODEC_ID_HUFFYUV,
    AV_CODEC_ID_CYUV,
    AV_CODEC_ID_H264,
    AV_CODEC_ID_INDEO3,
    AV_CODEC_ID_VP3,
    AV_CODEC_ID_THEORA,
    AV_CODEC_ID_ASV1,
    AV_CODEC_ID_ASV2,
    AV_CODEC_ID_FFV1,
    AV_CODEC_ID_4XM,
    AV_CODEC_ID_VCR1,
    AV_CODEC_ID_CLJR,
    AV_CODEC_ID_MDEC,
    AV_CODEC_ID_ROQ,
    AV_CODEC_ID_INTERPLAY_VIDEO,
    AV_CODEC_ID_XAN_WC3,
    AV_CODEC_ID_XAN_WC4,
    AV_CODEC_ID_RPZA,
    AV_CODEC_ID_CINEPAK,
    AV_CODEC_ID_WS_VQA,
    AV_CODEC_ID_MSRLE,
    AV_CODEC_ID_MSVIDEO1,
    AV_CODEC_ID_IDCIN,
    AV_CODEC_ID_8BPS,
    AV_CODEC_ID_SMC,
    AV_CODEC_ID_FLIC,
    AV_CODEC_ID_TRUEMOTION1,
    AV_CODEC_ID_VMDVIDEO,
    AV_CODEC_ID_MSZH,
    AV_CODEC_ID_ZLIB,
    AV_CODEC_ID_QTRLE,
    AV_CODEC_ID_TSCC,
    AV_CODEC_ID_ULTI,
    AV_CODEC_ID_QDRAW,
    AV_CODEC_ID_VIXL,
    AV_CODEC_ID_QPEG,
    AV_CODEC_ID_PNG,
    AV_CODEC_ID_PPM,
    AV_CODEC_ID_PBM,
    AV_CODEC_ID_PGM,
    AV_CODEC_ID_PGMYUV,
    AV_CODEC_ID_PAM,
    AV_CODEC_ID_FFVHUFF,
    AV_CODEC_ID_RV30,
    AV_CODEC_ID_RV40,
    AV_CODEC_ID_VC1,
    AV_CODEC_ID_WMV3,
    AV_CODEC_ID_LOCO,
    AV_CODEC_ID_WNV1,
    AV_CODEC_ID_AASC,
    AV_CODEC_ID_INDEO2,
    AV_CODEC_ID_FRAPS,
    AV_CODEC_ID_TRUEMOTION2,
    AV_CODEC_ID_BMP,
    AV_CODEC_ID_CSCD,
    AV_CODEC_ID_MMVIDEO,
    AV_CODEC_ID_ZMBV,
    AV_CODEC_ID_AVS,
    AV_CODEC_ID_SMACKVIDEO,
    AV_CODEC_ID_NUV,
    AV_CODEC_ID_KMVC,
    AV_CODEC_ID_FLASHSV,
    AV_CODEC_ID_CAVS,
    AV_CODEC_ID_JPEG2000,
    AV_CODEC_ID_VMNC,
    AV_CODEC_ID_VP5,
    AV_CODEC_ID_VP6,
    AV_CODEC_ID_VP6F,
    AV_CODEC_ID_TARGA,
    AV_CODEC_ID_DSICINVIDEO,
    AV_CODEC_ID_TIERTEXSEQVIDEO,
    AV_CODEC_ID_TIFF,
    AV_CODEC_ID_GIF,
    AV_CODEC_ID_DXA,
    AV_CODEC_ID_DNXHD,
    AV_CODEC_ID_THP,
    AV_CODEC_ID_SGI,
    AV_CODEC_ID_C93,
    AV_CODEC_ID_BETHSOFTVID,
    AV_CODEC_ID_PTX,
    AV_CODEC_ID_TXD,
    AV_CODEC_ID_VP6A,
    AV_CODEC_ID_AMV,
    AV_CODEC_ID_VB,
    AV_CODEC_ID_PCX,
    AV_CODEC_ID_SUNRAST,
    AV_CODEC_ID_INDEO4,
    AV_CODEC_ID_INDEO5,
    AV_CODEC_ID_MIMIC,
    AV_CODEC_ID_RL2,
    AV_CODEC_ID_ESCAPE124,
    AV_CODEC_ID_DIRAC,
    AV_CODEC_ID_BFI,
    AV_CODEC_ID_CMV,
    AV_CODEC_ID_MOTIONPIXELS,
    AV_CODEC_ID_TGV,
    AV_CODEC_ID_TGQ,
    AV_CODEC_ID_TQI,
    AV_CODEC_ID_AURA,
    AV_CODEC_ID_AURA2,
    AV_CODEC_ID_V210X,
    AV_CODEC_ID_TMV,
    AV_CODEC_ID_V210,
    AV_CODEC_ID_DPX,
    AV_CODEC_ID_MAD,
    AV_CODEC_ID_FRWU,
    AV_CODEC_ID_FLASHSV2,
    AV_CODEC_ID_CDGRAPHICS,
    AV_CODEC_ID_R210,
    AV_CODEC_ID_ANM,
    AV_CODEC_ID_BINKVIDEO,
    AV_CODEC_ID_IFF_ILBM,
#define AV_CODEC_ID_IFF_BYTERUN1 AV_CODEC_ID_IFF_ILBM
    AV_CODEC_ID_KGV1,
    AV_CODEC_ID_YOP,
    AV_CODEC_ID_VP8,
    AV_CODEC_ID_PICTOR,
    AV_CODEC_ID_ANSI,
    AV_CODEC_ID_A64_MULTI,
    AV_CODEC_ID_A64_MULTI5,
    AV_CODEC_ID_R10K,
    AV_CODEC_ID_MXPEG,
    AV_CODEC_ID_LAGARITH,
    AV_CODEC_ID_PRORES,
    AV_CODEC_ID_JV,
    AV_CODEC_ID_DFA,
    AV_CODEC_ID_WMV3IMAGE,
    AV_CODEC_ID_VC1IMAGE,
    AV_CODEC_ID_UTVIDEO,
    AV_CODEC_ID_BMV_VIDEO,
    AV_CODEC_ID_VBLE,
    AV_CODEC_ID_DXTORY,
    AV_CODEC_ID_V410,
    AV_CODEC_ID_XWD,
    AV_CODEC_ID_CDXL,
    AV_CODEC_ID_XBM,
    AV_CODEC_ID_ZEROCODEC,
    AV_CODEC_ID_MSS1,
    AV_CODEC_ID_MSA1,
    AV_CODEC_ID_TSCC2,
    AV_CODEC_ID_MTS2,
    AV_CODEC_ID_CLLC,
    AV_CODEC_ID_MSS2,
    AV_CODEC_ID_VP9,
    AV_CODEC_ID_AIC,
    AV_CODEC_ID_ESCAPE130,
    AV_CODEC_ID_G2M,
    AV_CODEC_ID_WEBP,
    AV_CODEC_ID_HNM4_VIDEO,
    AV_CODEC_ID_HEVC,
#define AV_CODEC_ID_H265 AV_CODEC_ID_HEVC
    AV_CODEC_ID_FIC,
    AV_CODEC_ID_ALIAS_PIX,
    AV_CODEC_ID_BRENDER_PIX,
    AV_CODEC_ID_PAF_VIDEO,
    AV_CODEC_ID_EXR,
    AV_CODEC_ID_VP7,
    AV_CODEC_ID_SANM,
    AV_CODEC_ID_SGIRLE,
    AV_CODEC_ID_MVC1,
    AV_CODEC_ID_MVC2,
    AV_CODEC_ID_HQX,
    AV_CODEC_ID_TDSC,
    AV_CODEC_ID_HQ_HQA,
    AV_CODEC_ID_HAP,
    AV_CODEC_ID_DDS,
    AV_CODEC_ID_DXV,
    AV_CODEC_ID_SCREENPRESSO,
    AV_CODEC_ID_RSCC,
    AV_CODEC_ID_AVS2,
    AV_CODEC_ID_PGX,
    AV_CODEC_ID_AVS3,

    AV_CODEC_ID_Y41P = 0x8000,
    AV_CODEC_ID_AVRP,
    AV_CODEC_ID_012V,
    AV_CODEC_ID_AVUI,
    AV_CODEC_ID_AYUV,
    AV_CODEC_ID_TARGA_Y216,
    AV_CODEC_ID_V308,
    AV_CODEC_ID_V408,
    AV_CODEC_ID_YUV4,
    AV_CODEC_ID_AVRN,
    AV_CODEC_ID_CPIA,
    AV_CODEC_ID_XFACE,
    AV_CODEC_ID_SNOW,
    AV_CODEC_ID_SMVJPEG,
    AV_CODEC_ID_APNG,
    AV_CODEC_ID_DAALA,
    AV_CODEC_ID_CFHD,
    AV_CODEC_ID_TRUEMOTION2RT,
    AV_CODEC_ID_M101,
    AV_CODEC_ID_MAGICYUV,
    AV_CODEC_ID_SHEERVIDEO,
    AV_CODEC_ID_YLC,
    AV_CODEC_ID_PSD,
    AV_CODEC_ID_PIXLET,
    AV_CODEC_ID_SPEEDHQ,
    AV_CODEC_ID_FMVC,
    AV_CODEC_ID_SCPR,
    AV_CODEC_ID_CLEARVIDEO,
    AV_CODEC_ID_XPM,
    AV_CODEC_ID_AV1,
    AV_CODEC_ID_BITPACKED,
    AV_CODEC_ID_MSCC,
    AV_CODEC_ID_SRGC,
    AV_CODEC_ID_SVG,
    AV_CODEC_ID_GDV,
    AV_CODEC_ID_FITS,
    AV_CODEC_ID_IMM4,
    AV_CODEC_ID_PROSUMER,
    AV_CODEC_ID_MWSC,
    AV_CODEC_ID_WCMV,
    AV_CODEC_ID_RASC,
    AV_CODEC_ID_HYMT,
    AV_CODEC_ID_ARBC,
    AV_CODEC_ID_AGM,
    AV_CODEC_ID_LSCR,
    AV_CODEC_ID_VP4,
    AV_CODEC_ID_IMM5,
    AV_CODEC_ID_MVDV,
    AV_CODEC_ID_MVHA,
    AV_CODEC_ID_CDTOONS,
    AV_CODEC_ID_MV30,
    AV_CODEC_ID_NOTCHLC,
    AV_CODEC_ID_PFM,
    AV_CODEC_ID_MOBICLIP,
    AV_CODEC_ID_PHOTOCD,
    AV_CODEC_ID_IPU,
    AV_CODEC_ID_ARGO,
    AV_CODEC_ID_CRI,

    /* various PCM "codecs" */
    AV_CODEC_ID_FIRST_AUDIO = 0x10000, ///< A dummy id pointing at the start of audio codecs
    AV_CODEC_ID_PCM_S16LE = 0x10000,
    AV_CODEC_ID_PCM_S16BE,
    AV_CODEC_ID_PCM_U16LE,
    AV_CODEC_ID_PCM_U16BE,
    AV_CODEC_ID_PCM_S8,
    AV_CODEC_ID_PCM_U8,
    AV_CODEC_ID_PCM_MULAW,
    AV_CODEC_ID_PCM_ALAW,
    AV_CODEC_ID_PCM_S32LE,
    AV_CODEC_ID_PCM_S32BE,
    AV_CODEC_ID_PCM_U32LE,
    AV_CODEC_ID_PCM_U32BE,
    AV_CODEC_ID_PCM_S24LE,
    AV_CODEC_ID_PCM_S24BE,
    AV_CODEC_ID_PCM_U24LE,
    AV_CODEC_ID_PCM_U24BE,
    AV_CODEC_ID_PCM_S24DAUD,
    AV_CODEC_ID_PCM_ZORK,
    AV_CODEC_ID_PCM_S16LE_PLANAR,
    AV_CODEC_ID_PCM_DVD,
    AV_CODEC_ID_PCM_F32BE,
    AV_CODEC_ID_PCM_F32LE,
    AV_CODEC_ID_PCM_F64BE,
    AV_CODEC_ID_PCM_F64LE,
    AV_CODEC_ID_PCM_BLURAY,
    AV_CODEC_ID_PCM_LXF,
    AV_CODEC_ID_S302M,
    AV_CODEC_ID_PCM_S8_PLANAR,
    AV_CODEC_ID_PCM_S24LE_PLANAR,
    AV_CODEC_ID_PCM_S32LE_PLANAR,
    AV_CODEC_ID_PCM_S16BE_PLANAR,

    AV_CODEC_ID_PCM_S64LE = 0x10800,
    AV_CODEC_ID_PCM_S64BE,
    AV_CODEC_ID_PCM_F16LE,
    AV_CODEC_ID_PCM_F24LE,
    AV_CODEC_ID_PCM_VIDC,

    /* various ADPCM codecs */
    AV_CODEC_ID_ADPCM_IMA_QT = 0x11000,
    AV_CODEC_ID_ADPCM_IMA_WAV,
    AV_CODEC_ID_ADPCM_IMA_DK3,
    AV_CODEC_ID_ADPCM_IMA_DK4,
    AV_CODEC_ID_ADPCM_IMA_WS,
    AV_CODEC_ID_ADPCM_IMA_SMJPEG,
    AV_CODEC_ID_ADPCM_MS,
    AV_CODEC_ID_ADPCM_4XM,
    AV_CODEC_ID_ADPCM_XA,
    AV_CODEC_ID_ADPCM_ADX,
    AV_CODEC_ID_ADPCM_EA,
    AV_CODEC_ID_ADPCM_G726,
    AV_CODEC_ID_ADPCM_CT,
    AV_CODEC_ID_ADPCM_SWF,
    AV_CODEC_ID_ADPCM_YAMAHA,
    AV_CODEC_ID_ADPCM_SBPRO_4,
    AV_CODEC_ID_ADPCM_SBPRO_3,
    AV_CODEC_ID_ADPCM_SBPRO_2,
    AV_CODEC_ID_ADPCM_THP,
    AV_CODEC_ID_ADPCM_IMA_AMV,
    AV_CODEC_ID_ADPCM_EA_R1,
    AV_CODEC_ID_ADPCM_EA_R3,
    AV_CODEC_ID_ADPCM_EA_R2,
    AV_CODEC_ID_ADPCM_IMA_EA_SEAD,
    AV_CODEC_ID_ADPCM_IMA_EA_EACS,
    AV_CODEC_ID_ADPCM_EA_XAS,
    AV_CODEC_ID_ADPCM_EA_MAXIS_XA,
    AV_CODEC_ID_ADPCM_IMA_ISS,
    AV_CODEC_ID_ADPCM_G722,
    AV_CODEC_ID_ADPCM_IMA_APC,
    AV_CODEC_ID_ADPCM_VIMA,

    AV_CODEC_ID_ADPCM_AFC = 0x11800,
    AV_CODEC_ID_ADPCM_IMA_OKI,
    AV_CODEC_ID_ADPCM_DTK,
    AV_CODEC_ID_ADPCM_IMA_RAD,
    AV_CODEC_ID_ADPCM_G726LE,
    AV_CODEC_ID_ADPCM_THP_LE,
    AV_CODEC_ID_ADPCM_PSX,
    AV_CODEC_ID_ADPCM_AICA,
    AV_CODEC_ID_ADPCM_IMA_DAT4,
    AV_CODEC_ID_ADPCM_MTAF,
    AV_CODEC_ID_ADPCM_AGM,
    AV_CODEC_ID_ADPCM_ARGO,
    AV_CODEC_ID_ADPCM_IMA_SSI,
    AV_CODEC_ID_ADPCM_ZORK,
    AV_CODEC_ID_ADPCM_IMA_APM,
    AV_CODEC_ID_ADPCM_IMA_ALP,
    AV_CODEC_ID_ADPCM_IMA_MTF,
    AV_CODEC_ID_ADPCM_IMA_CUNNING,
    AV_CODEC_ID_ADPCM_IMA_MOFLEX,

    /* AMR */
    AV_CODEC_ID_AMR_NB = 0x12000,
    AV_CODEC_ID_AMR_WB,

    /* RealAudio codecs*/
    AV_CODEC_ID_RA_144 = 0x13000,
    AV_CODEC_ID_RA_288,

    /* various DPCM codecs */
    AV_CODEC_ID_ROQ_DPCM = 0x14000,
    AV_CODEC_ID_INTERPLAY_DPCM,
    AV_CODEC_ID_XAN_DPCM,
    AV_CODEC_ID_SOL_DPCM,

    AV_CODEC_ID_SDX2_DPCM = 0x14800,
    AV_CODEC_ID_GREMLIN_DPCM,
    AV_CODEC_ID_DERF_DPCM,

    /* audio codecs */
    AV_CODEC_ID_MP2 = 0x15000,
    AV_CODEC_ID_MP3, ///< preferred ID for decoding MPEG audio layer 1, 2 or 3
    AV_CODEC_ID_AAC,
    AV_CODEC_ID_AC3,
    AV_CODEC_ID_DTS,
    AV_CODEC_ID_VORBIS,
    AV_CODEC_ID_DVAUDIO,
    AV_CODEC_ID_WMAV1,
    AV_CODEC_ID_WMAV2,
    AV_CODEC_ID_MACE3,
    AV_CODEC_ID_MACE6,
    AV_CODEC_ID_VMDAUDIO,
    AV_CODEC_ID_FLAC,
    AV_CODEC_ID_MP3ADU,
    AV_CODEC_ID_MP3ON4,
    AV_CODEC_ID_SHORTEN,
    AV_CODEC_ID_ALAC,
    AV_CODEC_ID_WESTWOOD_SND1,
    AV_CODEC_ID_GSM, ///< as in Berlin toast format
    AV_CODEC_ID_QDM2,
    AV_CODEC_ID_COOK,
    AV_CODEC_ID_TRUESPEECH,
    AV_CODEC_ID_TTA,
    AV_CODEC_ID_SMACKAUDIO,
    AV_CODEC_ID_QCELP,
    AV_CODEC_ID_WAVPACK,
    AV_CODEC_ID_DSICINAUDIO,
    AV_CODEC_ID_IMC,
    AV_CODEC_ID_MUSEPACK7,
    AV_CODEC_ID_MLP,
    AV_CODEC_ID_GSM_MS, /* as found in WAV */
    AV_CODEC_ID_ATRAC3,
    AV_CODEC_ID_APE,
    AV_CODEC_ID_NELLYMOSER,
    AV_CODEC_ID_MUSEPACK8,
    AV_CODEC_ID_SPEEX,
    AV_CODEC_ID_WMAVOICE,
    AV_CODEC_ID_WMAPRO,
    AV_CODEC_ID_WMALOSSLESS,
    AV_CODEC_ID_ATRAC3P,
    AV_CODEC_ID_EAC3,
    AV_CODEC_ID_SIPR,
    AV_CODEC_ID_MP1,
    AV_CODEC_ID_TWINVQ,
    AV_CODEC_ID_TRUEHD,
    AV_CODEC_ID_MP4ALS,
    AV_CODEC_ID_ATRAC1,
    AV_CODEC_ID_BINKAUDIO_RDFT,
    AV_CODEC_ID_BINKAUDIO_DCT,
    AV_CODEC_ID_AAC_LATM,
    AV_CODEC_ID_QDMC,
    AV_CODEC_ID_CELT,
    AV_CODEC_ID_G723_1,
    AV_CODEC_ID_G729,
    AV_CODEC_ID_8SVX_EXP,
    AV_CODEC_ID_8SVX_FIB,
    AV_CODEC_ID_BMV_AUDIO,
    AV_CODEC_ID_RALF,
    AV_CODEC_ID_IAC,
    AV_CODEC_ID_ILBC,
    AV_CODEC_ID_OPUS,
    AV_CODEC_ID_COMFORT_NOISE,
    AV_CODEC_ID_TAK,
    AV_CODEC_ID_METASOUND,
    AV_CODEC_ID_PAF_AUDIO,
    AV_CODEC_ID_ON2AVC,
    AV_CODEC_ID_DSS_SP,
    AV_CODEC_ID_CODEC2,

    AV_CODEC_ID_FFWAVESYNTH = 0x15800,
    AV_CODEC_ID_SONIC,
    AV_CODEC_ID_SONIC_LS,
    AV_CODEC_ID_EVRC,
    AV_CODEC_ID_SMV,
    AV_CODEC_ID_DSD_LSBF,
    AV_CODEC_ID_DSD_MSBF,
    AV_CODEC_ID_DSD_LSBF_PLANAR,
    AV_CODEC_ID_DSD_MSBF_PLANAR,
    AV_CODEC_ID_4GV,
    AV_CODEC_ID_INTERPLAY_ACM,
    AV_CODEC_ID_XMA1,
    AV_CODEC_ID_XMA2,
    AV_CODEC_ID_DST,
    AV_CODEC_ID_ATRAC3AL,
    AV_CODEC_ID_ATRAC3PAL,
    AV_CODEC_ID_DOLBY_E,
    AV_CODEC_ID_APTX,
    AV_CODEC_ID_APTX_HD,
    AV_CODEC_ID_SBC,
    AV_CODEC_ID_ATRAC9,
    AV_CODEC_ID_HCOM,
    AV_CODEC_ID_ACELP_KELVIN,
    AV_CODEC_ID_MPEGH_3D_AUDIO,
    AV_CODEC_ID_SIREN,
    AV_CODEC_ID_HCA,
    AV_CODEC_ID_FASTAUDIO,

    /* subtitle codecs */
    AV_CODEC_ID_FIRST_SUBTITLE = 0x17000, ///< A dummy ID pointing at the start of subtitle codecs.
    AV_CODEC_ID_DVD_SUBTITLE = 0x17000,
    AV_CODEC_ID_DVB_SUBTITLE,
    AV_CODEC_ID_TEXT, ///< raw UTF-8 text
    AV_CODEC_ID_XSUB,
    AV_CODEC_ID_SSA,
    AV_CODEC_ID_MOV_TEXT,
    AV_CODEC_ID_HDMV_PGS_SUBTITLE,
    AV_CODEC_ID_DVB_TELETEXT,
    AV_CODEC_ID_SRT,

    AV_CODEC_ID_MICRODVD = 0x17800,
    AV_CODEC_ID_EIA_608,
    AV_CODEC_ID_JACOSUB,
    AV_CODEC_ID_SAMI,
    AV_CODEC_ID_REALTEXT,
    AV_CODEC_ID_STL,
    AV_CODEC_ID_SUBVIEWER1,
    AV_CODEC_ID_SUBVIEWER,
    AV_CODEC_ID_SUBRIP,
    AV_CODEC_ID_WEBVTT,
    AV_CODEC_ID_MPL2,
    AV_CODEC_ID_VPLAYER,
    AV_CODEC_ID_PJS,
    AV_CODEC_ID_ASS,
    AV_CODEC_ID_HDMV_TEXT_SUBTITLE,
    AV_CODEC_ID_TTML,
    AV_CODEC_ID_ARIB_CAPTION,

    /* other specific kind of codecs (generally used for attachments) */
    AV_CODEC_ID_FIRST_UNKNOWN = 0x18000, ///< A dummy ID pointing at the start of various fake codecs.
    AV_CODEC_ID_TTF = 0x18000,

    AV_CODEC_ID_SCTE_35, ///< Contain timestamp estimated through PCR of program stream.
    AV_CODEC_ID_EPG,
    AV_CODEC_ID_BINTEXT = 0x18800,
    AV_CODEC_ID_XBIN,
    AV_CODEC_ID_IDF,
    AV_CODEC_ID_OTF,
    AV_CODEC_ID_SMPTE_KLV,
    AV_CODEC_ID_DVD_NAV,
    AV_CODEC_ID_TIMED_ID3,
    AV_CODEC_ID_BIN_DATA,

    AV_CODEC_ID_PROBE = 0x19000, ///< codec_id is not known (like AV_CODEC_ID_NONE) but lavf should attempt to identify it

    AV_CODEC_ID_MPEG2TS = 0x20000,         /**< _FAKE_ codec to indicate a raw MPEG-2 TS
                                * stream (only used by libavformat) */
    AV_CODEC_ID_MPEG4SYSTEMS = 0x20001,    /**< _FAKE_ codec to indicate a MPEG-4 Systems
                                * stream (only used by libavformat) */
    AV_CODEC_ID_FFMETADATA = 0x21000,      ///< Dummy codec for streams containing only metadata information.
    AV_CODEC_ID_WRAPPED_AVFRAME = 0x21001, ///< Passthrough codec, AVFrames wrapped in AVPacket
};

enum AVPixelFormat
{
    AV_PIX_FMT_NONE = -1,
    AV_PIX_FMT_YUV420P,   ///< planar YUV 4:2:0, 12bpp, (1 Cr & Cb sample per 2x2 Y samples)
    AV_PIX_FMT_YUYV422,   ///< packed YUV 4:2:2, 16bpp, Y0 Cb Y1 Cr
    AV_PIX_FMT_RGB24,     ///< packed RGB 8:8:8, 24bpp, RGBRGB...
    AV_PIX_FMT_BGR24,     ///< packed RGB 8:8:8, 24bpp, BGRBGR...
    AV_PIX_FMT_YUV422P,   ///< planar YUV 4:2:2, 16bpp, (1 Cr & Cb sample per 2x1 Y samples)
    AV_PIX_FMT_YUV444P,   ///< planar YUV 4:4:4, 24bpp, (1 Cr & Cb sample per 1x1 Y samples)
    AV_PIX_FMT_YUV410P,   ///< planar YUV 4:1:0,  9bpp, (1 Cr & Cb sample per 4x4 Y samples)
    AV_PIX_FMT_YUV411P,   ///< planar YUV 4:1:1, 12bpp, (1 Cr & Cb sample per 4x1 Y samples)
    AV_PIX_FMT_GRAY8,     ///<        Y        ,  8bpp
    AV_PIX_FMT_MONOWHITE, ///<        Y        ,  1bpp, 0 is white, 1 is black, in each byte pixels are ordered from the msb to the lsb
    AV_PIX_FMT_MONOBLACK, ///<        Y        ,  1bpp, 0 is black, 1 is white, in each byte pixels are ordered from the msb to the lsb
    AV_PIX_FMT_PAL8,      ///< 8 bits with AV_PIX_FMT_RGB32 palette
    AV_PIX_FMT_YUVJ420P,  ///< planar YUV 4:2:0, 12bpp, full scale (JPEG), deprecated in favor of AV_PIX_FMT_YUV420P and setting color_range
    AV_PIX_FMT_YUVJ422P,  ///< planar YUV 4:2:2, 16bpp, full scale (JPEG), deprecated in favor of AV_PIX_FMT_YUV422P and setting color_range
    AV_PIX_FMT_YUVJ444P,  ///< planar YUV 4:4:4, 24bpp, full scale (JPEG), deprecated in favor of AV_PIX_FMT_YUV444P and setting color_range
    AV_PIX_FMT_UYVY422,   ///< packed YUV 4:2:2, 16bpp, Cb Y0 Cr Y1
    AV_PIX_FMT_UYYVYY411, ///< packed YUV 4:1:1, 12bpp, Cb Y0 Y1 Cr Y2 Y3
    AV_PIX_FMT_BGR8,      ///< packed RGB 3:3:2,  8bpp, (msb)2B 3G 3R(lsb)
    AV_PIX_FMT_BGR4,      ///< packed RGB 1:2:1 bitstream,  4bpp, (msb)1B 2G 1R(lsb), a byte contains two pixels, the first pixel in the byte is the one composed by the 4 msb bits
    AV_PIX_FMT_BGR4_BYTE, ///< packed RGB 1:2:1,  8bpp, (msb)1B 2G 1R(lsb)
    AV_PIX_FMT_RGB8,      ///< packed RGB 3:3:2,  8bpp, (msb)2R 3G 3B(lsb)
    AV_PIX_FMT_RGB4,      ///< packed RGB 1:2:1 bitstream,  4bpp, (msb)1R 2G 1B(lsb), a byte contains two pixels, the first pixel in the byte is the one composed by the 4 msb bits
    AV_PIX_FMT_RGB4_BYTE, ///< packed RGB 1:2:1,  8bpp, (msb)1R 2G 1B(lsb)
    AV_PIX_FMT_NV12,      ///< planar YUV 4:2:0, 12bpp, 1 plane for Y and 1 plane for the UV components, which are interleaved (first byte U and the following byte V)
    AV_PIX_FMT_NV21,      ///< as above, but U and V bytes are swapped

    AV_PIX_FMT_ARGB, ///< packed ARGB 8:8:8:8, 32bpp, ARGBARGB...
    AV_PIX_FMT_RGBA, ///< packed RGBA 8:8:8:8, 32bpp, RGBARGBA...
    AV_PIX_FMT_ABGR, ///< packed ABGR 8:8:8:8, 32bpp, ABGRABGR...
    AV_PIX_FMT_BGRA, ///< packed BGRA 8:8:8:8, 32bpp, BGRABGRA...

    AV_PIX_FMT_GRAY16BE, ///<        Y        , 16bpp, big-endian
    AV_PIX_FMT_GRAY16LE, ///<        Y        , 16bpp, little-endian
    AV_PIX_FMT_YUV440P,  ///< planar YUV 4:4:0 (1 Cr & Cb sample per 1x2 Y samples)
    AV_PIX_FMT_YUVJ440P, ///< planar YUV 4:4:0 full scale (JPEG), deprecated in favor of AV_PIX_FMT_YUV440P and setting color_range
    AV_PIX_FMT_YUVA420P, ///< planar YUV 4:2:0, 20bpp, (1 Cr & Cb sample per 2x2 Y & A samples)
    AV_PIX_FMT_RGB48BE,  ///< packed RGB 16:16:16, 48bpp, 16R, 16G, 16B, the 2-byte value for each R/G/B component is stored as big-endian
    AV_PIX_FMT_RGB48LE,  ///< packed RGB 16:16:16, 48bpp, 16R, 16G, 16B, the 2-byte value for each R/G/B component is stored as little-endian

    AV_PIX_FMT_RGB565BE, ///< packed RGB 5:6:5, 16bpp, (msb)   5R 6G 5B(lsb), big-endian
    AV_PIX_FMT_RGB565LE, ///< packed RGB 5:6:5, 16bpp, (msb)   5R 6G 5B(lsb), little-endian
    AV_PIX_FMT_RGB555BE, ///< packed RGB 5:5:5, 16bpp, (msb)1X 5R 5G 5B(lsb), big-endian   , X=unused/undefined
    AV_PIX_FMT_RGB555LE, ///< packed RGB 5:5:5, 16bpp, (msb)1X 5R 5G 5B(lsb), little-endian, X=unused/undefined

    AV_PIX_FMT_BGR565BE, ///< packed BGR 5:6:5, 16bpp, (msb)   5B 6G 5R(lsb), big-endian
    AV_PIX_FMT_BGR565LE, ///< packed BGR 5:6:5, 16bpp, (msb)   5B 6G 5R(lsb), little-endian
    AV_PIX_FMT_BGR555BE, ///< packed BGR 5:5:5, 16bpp, (msb)1X 5B 5G 5R(lsb), big-endian   , X=unused/undefined
    AV_PIX_FMT_BGR555LE, ///< packed BGR 5:5:5, 16bpp, (msb)1X 5B 5G 5R(lsb), little-endian, X=unused/undefined

#if FF_API_VAAPI
    /** @name Deprecated pixel formats */
    /**@{*/
    AV_PIX_FMT_VAAPI_MOCO, ///< HW acceleration through VA API at motion compensation entry-point, Picture.data[3] contains a vaapi_render_state struct which contains macroblocks as well as various fields extracted from headers
    AV_PIX_FMT_VAAPI_IDCT, ///< HW acceleration through VA API at IDCT entry-point, Picture.data[3] contains a vaapi_render_state struct which contains fields extracted from headers
    AV_PIX_FMT_VAAPI_VLD,  ///< HW decoding through VA API, Picture.data[3] contains a VASurfaceID
    /**@}*/
    AV_PIX_FMT_VAAPI = AV_PIX_FMT_VAAPI_VLD,
#else
    /**
     *  Hardware acceleration through VA-API, data[3] contains a
     *  VASurfaceID.
     */
    AV_PIX_FMT_VAAPI,
#endif
    AV_PIX_FMT_YUV420P16LE,              ///< planar YUV 4:2:0, 24bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
    AV_PIX_FMT_YUV420P16BE,              ///< planar YUV 4:2:0, 24bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
    AV_PIX_FMT_YUV422P16LE,              ///< planar YUV 4:2:2, 32bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
    AV_PIX_FMT_YUV422P16BE,              ///< planar YUV 4:2:2, 32bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
    AV_PIX_FMT_YUV444P16LE,              ///< planar YUV 4:4:4, 48bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
    AV_PIX_FMT_YUV444P16BE,              ///< planar YUV 4:4:4, 48bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
    AV_PIX_FMT_DXVA2_VLD,                ///< HW decoding through DXVA2, Picture.data[3] contains a LPDIRECT3DSURFACE9 pointer
    AV_PIX_FMT_RGB444LE,                 ///< packed RGB 4:4:4, 16bpp, (msb)4X 4R 4G 4B(lsb), little-endian, X=unused/undefined
    AV_PIX_FMT_RGB444BE,                 ///< packed RGB 4:4:4, 16bpp, (msb)4X 4R 4G 4B(lsb), big-endian,    X=unused/undefined
    AV_PIX_FMT_BGR444LE,                 ///< packed BGR 4:4:4, 16bpp, (msb)4X 4B 4G 4R(lsb), little-endian, X=unused/undefined
    AV_PIX_FMT_BGR444BE,                 ///< packed BGR 4:4:4, 16bpp, (msb)4X 4B 4G 4R(lsb), big-endian,    X=unused/undefined
    AV_PIX_FMT_YA8,                      ///< 8 bits gray, 8 bits alpha
    AV_PIX_FMT_Y400A = AV_PIX_FMT_YA8,   ///< alias for AV_PIX_FMT_YA8
    AV_PIX_FMT_GRAY8A = AV_PIX_FMT_YA8,  ///< alias for AV_PIX_FMT_YA8
    AV_PIX_FMT_BGR48BE,                  ///< packed RGB 16:16:16, 48bpp, 16B, 16G, 16R, the 2-byte value for each R/G/B component is stored as big-endian
    AV_PIX_FMT_BGR48LE,                  ///< packed RGB 16:16:16, 48bpp, 16B, 16G, 16R, the 2-byte value for each R/G/B component is stored as little-endian
    AV_PIX_FMT_YUV420P9BE,               ///< planar YUV 4:2:0, 13.5bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
    AV_PIX_FMT_YUV420P9LE,               ///< planar YUV 4:2:0, 13.5bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
    AV_PIX_FMT_YUV420P10BE,              ///< planar YUV 4:2:0, 15bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
    AV_PIX_FMT_YUV420P10LE,              ///< planar YUV 4:2:0, 15bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
    AV_PIX_FMT_YUV422P10BE,              ///< planar YUV 4:2:2, 20bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
    AV_PIX_FMT_YUV422P10LE,              ///< planar YUV 4:2:2, 20bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
    AV_PIX_FMT_YUV444P9BE,               ///< planar YUV 4:4:4, 27bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
    AV_PIX_FMT_YUV444P9LE,               ///< planar YUV 4:4:4, 27bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
    AV_PIX_FMT_YUV444P10BE,              ///< planar YUV 4:4:4, 30bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
    AV_PIX_FMT_YUV444P10LE,              ///< planar YUV 4:4:4, 30bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
    AV_PIX_FMT_YUV422P9BE,               ///< planar YUV 4:2:2, 18bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
    AV_PIX_FMT_YUV422P9LE,               ///< planar YUV 4:2:2, 18bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
    AV_PIX_FMT_GBRP,                     ///< planar GBR 4:4:4 24bpp
    AV_PIX_FMT_GBR24P = AV_PIX_FMT_GBRP, // alias for #AV_PIX_FMT_GBRP
    AV_PIX_FMT_GBRP9BE,                  ///< planar GBR 4:4:4 27bpp, big-endian
    AV_PIX_FMT_GBRP9LE,                  ///< planar GBR 4:4:4 27bpp, little-endian
    AV_PIX_FMT_GBRP10BE,                 ///< planar GBR 4:4:4 30bpp, big-endian
    AV_PIX_FMT_GBRP10LE,                 ///< planar GBR 4:4:4 30bpp, little-endian
    AV_PIX_FMT_GBRP16BE,                 ///< planar GBR 4:4:4 48bpp, big-endian
    AV_PIX_FMT_GBRP16LE,                 ///< planar GBR 4:4:4 48bpp, little-endian
    AV_PIX_FMT_YUVA422P,                 ///< planar YUV 4:2:2 24bpp, (1 Cr & Cb sample per 2x1 Y & A samples)
    AV_PIX_FMT_YUVA444P,                 ///< planar YUV 4:4:4 32bpp, (1 Cr & Cb sample per 1x1 Y & A samples)
    AV_PIX_FMT_YUVA420P9BE,              ///< planar YUV 4:2:0 22.5bpp, (1 Cr & Cb sample per 2x2 Y & A samples), big-endian
    AV_PIX_FMT_YUVA420P9LE,              ///< planar YUV 4:2:0 22.5bpp, (1 Cr & Cb sample per 2x2 Y & A samples), little-endian
    AV_PIX_FMT_YUVA422P9BE,              ///< planar YUV 4:2:2 27bpp, (1 Cr & Cb sample per 2x1 Y & A samples), big-endian
    AV_PIX_FMT_YUVA422P9LE,              ///< planar YUV 4:2:2 27bpp, (1 Cr & Cb sample per 2x1 Y & A samples), little-endian
    AV_PIX_FMT_YUVA444P9BE,              ///< planar YUV 4:4:4 36bpp, (1 Cr & Cb sample per 1x1 Y & A samples), big-endian
    AV_PIX_FMT_YUVA444P9LE,              ///< planar YUV 4:4:4 36bpp, (1 Cr & Cb sample per 1x1 Y & A samples), little-endian
    AV_PIX_FMT_YUVA420P10BE,             ///< planar YUV 4:2:0 25bpp, (1 Cr & Cb sample per 2x2 Y & A samples, big-endian)
    AV_PIX_FMT_YUVA420P10LE,             ///< planar YUV 4:2:0 25bpp, (1 Cr & Cb sample per 2x2 Y & A samples, little-endian)
    AV_PIX_FMT_YUVA422P10BE,             ///< planar YUV 4:2:2 30bpp, (1 Cr & Cb sample per 2x1 Y & A samples, big-endian)
    AV_PIX_FMT_YUVA422P10LE,             ///< planar YUV 4:2:2 30bpp, (1 Cr & Cb sample per 2x1 Y & A samples, little-endian)
    AV_PIX_FMT_YUVA444P10BE,             ///< planar YUV 4:4:4 40bpp, (1 Cr & Cb sample per 1x1 Y & A samples, big-endian)
    AV_PIX_FMT_YUVA444P10LE,             ///< planar YUV 4:4:4 40bpp, (1 Cr & Cb sample per 1x1 Y & A samples, little-endian)
    AV_PIX_FMT_YUVA420P16BE,             ///< planar YUV 4:2:0 40bpp, (1 Cr & Cb sample per 2x2 Y & A samples, big-endian)
    AV_PIX_FMT_YUVA420P16LE,             ///< planar YUV 4:2:0 40bpp, (1 Cr & Cb sample per 2x2 Y & A samples, little-endian)
    AV_PIX_FMT_YUVA422P16BE,             ///< planar YUV 4:2:2 48bpp, (1 Cr & Cb sample per 2x1 Y & A samples, big-endian)
    AV_PIX_FMT_YUVA422P16LE,             ///< planar YUV 4:2:2 48bpp, (1 Cr & Cb sample per 2x1 Y & A samples, little-endian)
    AV_PIX_FMT_YUVA444P16BE,             ///< planar YUV 4:4:4 64bpp, (1 Cr & Cb sample per 1x1 Y & A samples, big-endian)
    AV_PIX_FMT_YUVA444P16LE,             ///< planar YUV 4:4:4 64bpp, (1 Cr & Cb sample per 1x1 Y & A samples, little-endian)
    AV_PIX_FMT_VDPAU,                    ///< HW acceleration through VDPAU, Picture.data[3] contains a VdpVideoSurface
    AV_PIX_FMT_XYZ12LE,                  ///< packed XYZ 4:4:4, 36 bpp, (msb) 12X, 12Y, 12Z (lsb), the 2-byte value for each X/Y/Z is stored as little-endian, the 4 lower bits are set to 0
    AV_PIX_FMT_XYZ12BE,                  ///< packed XYZ 4:4:4, 36 bpp, (msb) 12X, 12Y, 12Z (lsb), the 2-byte value for each X/Y/Z is stored as big-endian, the 4 lower bits are set to 0
    AV_PIX_FMT_NV16,                     ///< interleaved chroma YUV 4:2:2, 16bpp, (1 Cr & Cb sample per 2x1 Y samples)
    AV_PIX_FMT_NV20LE,                   ///< interleaved chroma YUV 4:2:2, 20bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
    AV_PIX_FMT_NV20BE,                   ///< interleaved chroma YUV 4:2:2, 20bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
    AV_PIX_FMT_RGBA64BE,                 ///< packed RGBA 16:16:16:16, 64bpp, 16R, 16G, 16B, 16A, the 2-byte value for each R/G/B/A component is stored as big-endian
    AV_PIX_FMT_RGBA64LE,                 ///< packed RGBA 16:16:16:16, 64bpp, 16R, 16G, 16B, 16A, the 2-byte value for each R/G/B/A component is stored as little-endian
    AV_PIX_FMT_BGRA64BE,                 ///< packed RGBA 16:16:16:16, 64bpp, 16B, 16G, 16R, 16A, the 2-byte value for each R/G/B/A component is stored as big-endian
    AV_PIX_FMT_BGRA64LE,                 ///< packed RGBA 16:16:16:16, 64bpp, 16B, 16G, 16R, 16A, the 2-byte value for each R/G/B/A component is stored as little-endian
    AV_PIX_FMT_YVYU422,                  ///< packed YUV 4:2:2, 16bpp, Y0 Cr Y1 Cb
    AV_PIX_FMT_YA16BE,                   ///< 16 bits gray, 16 bits alpha (big-endian)
    AV_PIX_FMT_YA16LE,                   ///< 16 bits gray, 16 bits alpha (little-endian)
    AV_PIX_FMT_GBRAP,                    ///< planar GBRA 4:4:4:4 32bpp
    AV_PIX_FMT_GBRAP16BE,                ///< planar GBRA 4:4:4:4 64bpp, big-endian
    AV_PIX_FMT_GBRAP16LE,                ///< planar GBRA 4:4:4:4 64bpp, little-endian
    AV_PIX_FMT_QSV,
    AV_PIX_FMT_MMAL,
    AV_PIX_FMT_D3D11VA_VLD, ///< HW decoding through Direct3D11 via old API, Picture.data[3] contains a ID3D11VideoDecoderOutputView pointer
    AV_PIX_FMT_CUDA,
    AV_PIX_FMT_0RGB,           ///< packed RGB 8:8:8, 32bpp, XRGBXRGB...   X=unused/undefined
    AV_PIX_FMT_RGB0,           ///< packed RGB 8:8:8, 32bpp, RGBXRGBX...   X=unused/undefined
    AV_PIX_FMT_0BGR,           ///< packed BGR 8:8:8, 32bpp, XBGRXBGR...   X=unused/undefined
    AV_PIX_FMT_BGR0,           ///< packed BGR 8:8:8, 32bpp, BGRXBGRX...   X=unused/undefined
    AV_PIX_FMT_YUV420P12BE,    ///< planar YUV 4:2:0,18bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
    AV_PIX_FMT_YUV420P12LE,    ///< planar YUV 4:2:0,18bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
    AV_PIX_FMT_YUV420P14BE,    ///< planar YUV 4:2:0,21bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
    AV_PIX_FMT_YUV420P14LE,    ///< planar YUV 4:2:0,21bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
    AV_PIX_FMT_YUV422P12BE,    ///< planar YUV 4:2:2,24bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
    AV_PIX_FMT_YUV422P12LE,    ///< planar YUV 4:2:2,24bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
    AV_PIX_FMT_YUV422P14BE,    ///< planar YUV 4:2:2,28bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
    AV_PIX_FMT_YUV422P14LE,    ///< planar YUV 4:2:2,28bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
    AV_PIX_FMT_YUV444P12BE,    ///< planar YUV 4:4:4,36bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
    AV_PIX_FMT_YUV444P12LE,    ///< planar YUV 4:4:4,36bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
    AV_PIX_FMT_YUV444P14BE,    ///< planar YUV 4:4:4,42bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
    AV_PIX_FMT_YUV444P14LE,    ///< planar YUV 4:4:4,42bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
    AV_PIX_FMT_GBRP12BE,       ///< planar GBR 4:4:4 36bpp, big-endian
    AV_PIX_FMT_GBRP12LE,       ///< planar GBR 4:4:4 36bpp, little-endian
    AV_PIX_FMT_GBRP14BE,       ///< planar GBR 4:4:4 42bpp, big-endian
    AV_PIX_FMT_GBRP14LE,       ///< planar GBR 4:4:4 42bpp, little-endian
    AV_PIX_FMT_YUVJ411P,       ///< planar YUV 4:1:1, 12bpp, (1 Cr & Cb sample per 4x1 Y samples) full scale (JPEG), deprecated in favor of AV_PIX_FMT_YUV411P and setting color_range
    AV_PIX_FMT_BAYER_BGGR8,    ///< bayer, BGBG..(odd line), GRGR..(even line), 8-bit samples */
    AV_PIX_FMT_BAYER_RGGB8,    ///< bayer, RGRG..(odd line), GBGB..(even line), 8-bit samples */
    AV_PIX_FMT_BAYER_GBRG8,    ///< bayer, GBGB..(odd line), RGRG..(even line), 8-bit samples */
    AV_PIX_FMT_BAYER_GRBG8,    ///< bayer, GRGR..(odd line), BGBG..(even line), 8-bit samples */
    AV_PIX_FMT_BAYER_BGGR16LE, ///< bayer, BGBG..(odd line), GRGR..(even line), 16-bit samples, little-endian */
    AV_PIX_FMT_BAYER_BGGR16BE, ///< bayer, BGBG..(odd line), GRGR..(even line), 16-bit samples, big-endian */
    AV_PIX_FMT_BAYER_RGGB16LE, ///< bayer, RGRG..(odd line), GBGB..(even line), 16-bit samples, little-endian */
    AV_PIX_FMT_BAYER_RGGB16BE, ///< bayer, RGRG..(odd line), GBGB..(even line), 16-bit samples, big-endian */
    AV_PIX_FMT_BAYER_GBRG16LE, ///< bayer, GBGB..(odd line), RGRG..(even line), 16-bit samples, little-endian */
    AV_PIX_FMT_BAYER_GBRG16BE, ///< bayer, GBGB..(odd line), RGRG..(even line), 16-bit samples, big-endian */
    AV_PIX_FMT_BAYER_GRBG16LE, ///< bayer, GRGR..(odd line), BGBG..(even line), 16-bit samples, little-endian */
    AV_PIX_FMT_BAYER_GRBG16BE, ///< bayer, GRGR..(odd line), BGBG..(even line), 16-bit samples, big-endian */
    AV_PIX_FMT_XVMC,           ///< XVideo Motion Acceleration via common packet passing
    AV_PIX_FMT_YUV440P10LE,    ///< planar YUV 4:4:0,20bpp, (1 Cr & Cb sample per 1x2 Y samples), little-endian
    AV_PIX_FMT_YUV440P10BE,    ///< planar YUV 4:4:0,20bpp, (1 Cr & Cb sample per 1x2 Y samples), big-endian
    AV_PIX_FMT_YUV440P12LE,    ///< planar YUV 4:4:0,24bpp, (1 Cr & Cb sample per 1x2 Y samples), little-endian
    AV_PIX_FMT_YUV440P12BE,    ///< planar YUV 4:4:0,24bpp, (1 Cr & Cb sample per 1x2 Y samples), big-endian
    AV_PIX_FMT_AYUV64LE,       ///< packed AYUV 4:4:4,64bpp (1 Cr & Cb sample per 1x1 Y & A samples), little-endian
    AV_PIX_FMT_AYUV64BE,       ///< packed AYUV 4:4:4,64bpp (1 Cr & Cb sample per 1x1 Y & A samples), big-endian
    AV_PIX_FMT_VIDEOTOOLBOX,   ///< hardware decoding through Videotoolbox
    AV_PIX_FMT_P010LE,         ///< like NV12, with 10bpp per component, data in the high bits, zeros in the low bits, little-endian
    AV_PIX_FMT_P010BE,         ///< like NV12, with 10bpp per component, data in the high bits, zeros in the low bits, big-endian
    AV_PIX_FMT_GBRAP12BE,      ///< planar GBR 4:4:4:4 48bpp, big-endian
    AV_PIX_FMT_GBRAP12LE,      ///< planar GBR 4:4:4:4 48bpp, little-endian
    AV_PIX_FMT_GBRAP10BE,      ///< planar GBR 4:4:4:4 40bpp, big-endian
    AV_PIX_FMT_GBRAP10LE,      ///< planar GBR 4:4:4:4 40bpp, little-endian
    AV_PIX_FMT_MEDIACODEC,     ///< hardware decoding through MediaCodec
    AV_PIX_FMT_GRAY12BE,       ///<        Y        , 12bpp, big-endian
    AV_PIX_FMT_GRAY12LE,       ///<        Y        , 12bpp, little-endian
    AV_PIX_FMT_GRAY10BE,       ///<        Y        , 10bpp, big-endian
    AV_PIX_FMT_GRAY10LE,       ///<        Y        , 10bpp, little-endian
    AV_PIX_FMT_P016LE,         ///< like NV12, with 16bpp per component, little-endian
    AV_PIX_FMT_P016BE,         ///< like NV12, with 16bpp per component, big-endian
    AV_PIX_FMT_D3D11,
    AV_PIX_FMT_GRAY9BE,    ///<        Y        , 9bpp, big-endian
    AV_PIX_FMT_GRAY9LE,    ///<        Y        , 9bpp, little-endian
    AV_PIX_FMT_GBRPF32BE,  ///< IEEE-754 single precision planar GBR 4:4:4,     96bpp, big-endian
    AV_PIX_FMT_GBRPF32LE,  ///< IEEE-754 single precision planar GBR 4:4:4,     96bpp, little-endian
    AV_PIX_FMT_GBRAPF32BE, ///< IEEE-754 single precision planar GBRA 4:4:4:4, 128bpp, big-endian
    AV_PIX_FMT_GBRAPF32LE, ///< IEEE-754 single precision planar GBRA 4:4:4:4, 128bpp, little-endian
    AV_PIX_FMT_DRM_PRIME,
    AV_PIX_FMT_OPENCL,
    AV_PIX_FMT_GRAY14BE,     ///<        Y        , 14bpp, big-endian
    AV_PIX_FMT_GRAY14LE,     ///<        Y        , 14bpp, little-endian
    AV_PIX_FMT_GRAYF32BE,    ///< IEEE-754 single precision Y, 32bpp, big-endian
    AV_PIX_FMT_GRAYF32LE,    ///< IEEE-754 single precision Y, 32bpp, little-endian
    AV_PIX_FMT_YUVA422P12BE, ///< planar YUV 4:2:2,24bpp, (1 Cr & Cb sample per 2x1 Y samples), 12b alpha, big-endian
    AV_PIX_FMT_YUVA422P12LE, ///< planar YUV 4:2:2,24bpp, (1 Cr & Cb sample per 2x1 Y samples), 12b alpha, little-endian
    AV_PIX_FMT_YUVA444P12BE, ///< planar YUV 4:4:4,36bpp, (1 Cr & Cb sample per 1x1 Y samples), 12b alpha, big-endian
    AV_PIX_FMT_YUVA444P12LE, ///< planar YUV 4:4:4,36bpp, (1 Cr & Cb sample per 1x1 Y samples), 12b alpha, little-endian
    AV_PIX_FMT_NV24,         ///< planar YUV 4:4:4, 24bpp, 1 plane for Y and 1 plane for the UV components, which are interleaved (first byte U and the following byte V)
    AV_PIX_FMT_NV42,         ///< as above, but U and V bytes are swapped
        AV_PIX_FMT_VULKAN,

    AV_PIX_FMT_Y210BE,    ///< packed YUV 4:2:2 like YUYV422, 20bpp, data in the high bits, big-endian
    AV_PIX_FMT_Y210LE,    ///< packed YUV 4:2:2 like YUYV422, 20bpp, data in the high bits, little-endian

    AV_PIX_FMT_X2RGB10LE, ///< packed RGB 10:10:10, 30bpp, (msb)2X 10R 10G 10B(lsb), little-endian, X=unused/undefined
    AV_PIX_FMT_X2RGB10BE, ///< packed RGB 10:10:10, 30bpp, (msb)2X 10R 10G 10B(lsb), big-endian, X=unused/undefined
    AV_PIX_FMT_NB   
};

enum OutputFormat
{
    FMT_MPEG1,
    FMT_H261,
    FMT_H263,
    FMT_MJPEG,
};

enum AVDurationEstimationMethod
{
    AVFMT_DURATION_FROM_PTS,    ///< Duration accurately estimated from PTSes
    AVFMT_DURATION_FROM_STREAM, ///< Duration estimated from a stream with a known duration
    AVFMT_DURATION_FROM_BITRATE ///< Duration estimated from bitrate (less accurate)
};

enum AVFieldOrder
{
    AV_FIELD_UNKNOWN,
    AV_FIELD_PROGRESSIVE,
    AV_FIELD_TT, //< Top coded_first, top displayed first
    AV_FIELD_BB, //< Bottom coded first, bottom displayed first
    AV_FIELD_TB, //< Top coded first, bottom displayed first
    AV_FIELD_BT, //< Bottom coded first, top displayed first
};

enum AVAudioServiceType
{
    AV_AUDIO_SERVICE_TYPE_MAIN = 0,
    AV_AUDIO_SERVICE_TYPE_EFFECTS = 1,
    AV_AUDIO_SERVICE_TYPE_VISUALLY_IMPAIRED = 2,
    AV_AUDIO_SERVICE_TYPE_HEARING_IMPAIRED = 3,
    AV_AUDIO_SERVICE_TYPE_DIALOGUE = 4,
    AV_AUDIO_SERVICE_TYPE_COMMENTARY = 5,
    AV_AUDIO_SERVICE_TYPE_EMERGENCY = 6,
    AV_AUDIO_SERVICE_TYPE_VOICE_OVER = 7,
    AV_AUDIO_SERVICE_TYPE_KARAOKE = 8,
    AV_AUDIO_SERVICE_TYPE_NB, ///< Not part of ABI
};

enum ShowMode
{
    SHOW_MODE_NONE = -1,
    SHOW_MODE_VIDEO = 0,
    SHOW_MODE_WAVES,
    SHOW_MODE_RDFT,
    SHOW_MODE_NB
};

enum AVIODataMarkerType
{
    AVIO_DATA_MARKER_HEADER,
    AVIO_DATA_MARKER_SYNC_POINT,
    AVIO_DATA_MARKER_BOUNDARY_POINT,
    AVIO_DATA_MARKER_UNKNOWN,
    AVIO_DATA_MARKER_TRAILER,
    AVIO_DATA_MARKER_FLUSH_POINT,
};

enum AVStreamParseType
{
    AVSTREAM_PARSE_NONE,
    AVSTREAM_PARSE_FULL,       /**< full parsing and repack */
    AVSTREAM_PARSE_HEADERS,    /**< Only parse headers, do not repack. */
    AVSTREAM_PARSE_TIMESTAMPS, /**< full parsing and interpolation of timestamps for frames not starting on a packet boundary */
    AVSTREAM_PARSE_FULL_ONCE,  /**< full parsing and repack of the first frame only, only implemented for H.264 currently */
    AVSTREAM_PARSE_FULL_RAW,   /**< full parsing and repack with timestamp and position generation by parser for raw
                                    this assumes that each packet in the file contains no demuxer level headers and
                                    just codec level data, otherwise position generation would fail */
};

enum SwrDitherType {
    SWR_DITHER_NONE = 0,
    SWR_DITHER_RECTANGULAR,
    SWR_DITHER_TRIANGULAR,
    SWR_DITHER_TRIANGULAR_HIGHPASS,

    SWR_DITHER_NS = 64,         ///< not part of API/ABI
    SWR_DITHER_NS_LIPSHITZ,
    SWR_DITHER_NS_F_WEIGHTED,
    SWR_DITHER_NS_MODIFIED_E_WEIGHTED,
    SWR_DITHER_NS_IMPROVED_E_WEIGHTED,
    SWR_DITHER_NS_SHIBATA,
    SWR_DITHER_NS_LOW_SHIBATA,
    SWR_DITHER_NS_HIGH_SHIBATA,
    SWR_DITHER_NB,              ///< not part of API/ABI
};

/** Resampling Engines */
enum SwrEngine {
    SWR_ENGINE_SWR,             /**< SW Resampler */
    SWR_ENGINE_SOXR,            /**< SoX Resampler */
    SWR_ENGINE_NB,              ///< not part of API/ABI
};

/** Resampling Filter Types */
enum SwrFilterType {
    SWR_FILTER_TYPE_CUBIC,              /**< Cubic */
    SWR_FILTER_TYPE_BLACKMAN_NUTTALL,   /**< Blackman Nuttall windowed sinc */
    SWR_FILTER_TYPE_KAISER,             /**< Kaiser windowed sinc */
};

enum {
    AV_FRAME_CROP_UNALIGNED     = 1 << 0,
};

enum {
    ///< Set when the thread is awaiting a packet.
    STATE_INPUT_READY,
    ///< Set before the codec has called ff_thread_finish_setup().
    STATE_SETTING_UP,
    /**
     * Set when the codec calls get_buffer().
     * State is returned to STATE_SETTING_UP afterwards.
     */
    STATE_GET_BUFFER,
     /**
      * Set when the codec calls get_format().
      * State is returned to STATE_SETTING_UP afterwards.
      */
    STATE_GET_FORMAT,
    ///< Set after the codec has called ff_thread_finish_setup().
    STATE_SETUP_FINISHED,
};

enum {
    AV_OPT_FLAG_IMPLICIT_KEY = 1,
};

enum {
    AV_BUFFERSRC_FLAG_NO_CHECK_FORMAT = 1,
    AV_BUFFERSRC_FLAG_PUSH = 4,
    AV_BUFFERSRC_FLAG_KEEP_REF = 8,
};


#if (defined(__GNUC__) || defined(__clang__)) && !defined(__INTEL_COMPILER)
#    define av_uninit(x) x=x
#else
#    define av_uninit(x) x
#endif


#if defined _MSC_VER && !defined _CRT_USE_BUILTIN_OFFSETOF
    #ifdef __cplusplus
        #define offsetof(s,m) ((::size_t)&reinterpret_cast<char const volatile&>((((s*)0)->m)))
    #else
        #define offsetof(s,m) ((size_t)&(((s*)0)->m))
    #endif
#else
    #define offsetof(s,m) __builtin_offsetof(s,m)
#endif

#ifdef __GNUC__
#define AV_GCC_VERSION_AT_LEAST(x, y) (__GNUC__ > (x) || __GNUC__ == (x) && __GNUC_MINOR__ >= (y))
#define AV_GCC_VERSION_AT_MOST(x, y) (__GNUC__ < (x) || __GNUC__ == (x) && __GNUC_MINOR__ <= (y))
#else
#define AV_GCC_VERSION_AT_LEAST(x, y) 0
#define AV_GCC_VERSION_AT_MOST(x, y) 0
#endif

#if AV_GCC_VERSION_AT_LEAST(3, 1)
#define attribute_deprecated __attribute__((deprecated))
#elif defined(_MSC_VER)
#define attribute_deprecated __declspec(deprecated)
#else
#define attribute_deprecated
#endif

#if defined(__GNUC__) || defined(__clang__)
#define av_unused __attribute__((unused))
#else
#define av_unused
#endif

#ifndef av_always_inline
#if AV_GCC_VERSION_AT_LEAST(3, 1)
#define av_always_inline __attribute__((always_inline)) inline
#elif defined(_MSC_VER)
#define av_always_inline __forceinline
#else
#define av_always_inline inline
#endif
#endif

#if defined(__INTEL_COMPILER) && __INTEL_COMPILER < 1110 || defined(__SUNPRO_C)
#define DECLARE_ALIGNED(n, t, v) t __attribute__((aligned(n))) v
#define DECLARE_ASM_ALIGNED(n, t, v) t __attribute__((aligned(n))) v
#define DECLARE_ASM_CONST(n, t, v) const t __attribute__((aligned(n))) v
#elif defined(__DJGPP__)
#define DECLARE_ALIGNED(n, t, v) t __attribute__((aligned(FFMIN(n, 16)))) v
#define DECLARE_ASM_ALIGNED(n, t, v) t av_used __attribute__((aligned(FFMIN(n, 16)))) v
#define DECLARE_ASM_CONST(n, t, v) static const t av_used __attribute__((aligned(FFMIN(n, 16)))) v
#elif defined(__GNUC__) || defined(__clang__)
#define DECLARE_ALIGNED(n, t, v) t __attribute__((aligned(n))) v
#define DECLARE_ASM_ALIGNED(n, t, v) t av_used __attribute__((aligned(n))) v
#define DECLARE_ASM_CONST(n, t, v) static const t av_used __attribute__((aligned(n))) v
#elif defined(_MSC_VER)
#define DECLARE_ALIGNED(n, t, v) __declspec(align(n)) t v
#define DECLARE_ASM_ALIGNED(n, t, v) __declspec(align(n)) t v
#define DECLARE_ASM_CONST(n, t, v) __declspec(align(n)) static const t v
#else
#define DECLARE_ALIGNED(n, t, v) t v
#define DECLARE_ASM_ALIGNED(n, t, v) t v
#define DECLARE_ASM_CONST(n, t, v) static const t v
#endif

#if AV_GCC_VERSION_AT_LEAST(2, 6) || defined(__clang__)
#define av_const __attribute__((const))
#else
#define av_const
#endif

#define YUVRGB_TABLE_HEADROOM 512
#define YUVRGB_TABLE_LUMA_HEADROOM 512
#define SWS_MAX_FILTER_SIZE 256

#define MAX_FILTER_SIZE SWS_MAX_FILTER_SIZE

static av_always_inline av_const int av_popcount_c(uint32_t x)
{
    x -= (x >> 1) & 0x55555555;
    x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
    x = (x + (x >> 4)) & 0x0F0F0F0F;
    x += x >> 8;
    return (x + (x >> 16)) & 0x3F;
}

#if _INTEGRAL_MAX_BITS >= 128
// minimum signed 128 bit value
#define _I128_MIN (-170141183460469231731687303715884105727i128 - 1)
// maximum signed 128 bit value
#define _I128_MAX 170141183460469231731687303715884105727i128
// maximum unsigned 128 bit value
#define _UI128_MAX 0xffffffffffffffffffffffffffffffffui128
#endif

#ifndef SIZE_MAX
#ifdef _WIN64
#define SIZE_MAX _UI64_MAX
#else
#define SIZE_MAX UINT_MAX
#endif
#endif

#if __STDC_WANT_SECURE_LIB__
#ifndef RSIZE_MAX
#define RSIZE_MAX (SIZE_MAX >> 1)
#endif
#endif

#ifndef av_ceil_log2
#define av_ceil_log2 av_ceil_log2_c
#endif

#ifndef av_clip64
#define av_clip64 av_clip64_c
#endif
#ifndef av_clip_uint8
#define av_clip_uint8 av_clip_uint8_c
#endif
#ifndef av_clip_int8
#define av_clip_int8 av_clip_int8_c
#endif
#ifndef av_clip_uint16
#define av_clip_uint16 av_clip_uint16_c
#endif
#ifndef av_clip_int16
#define av_clip_int16 av_clip_int16_c
#endif
#ifndef av_clipl_int32
#define av_clipl_int32 av_clipl_int32_c
#endif
#ifndef av_clip_intp2
#define av_clip_intp2 av_clip_intp2_c
#endif
#ifndef av_clip_uintp2
#define av_clip_uintp2 av_clip_uintp2_c
#endif
#ifndef av_mod_uintp2
#define av_mod_uintp2 av_mod_uintp2_c
#endif
#ifndef av_sat_add32
#define av_sat_add32 av_sat_add32_c
#endif
#ifndef av_sat_dadd32
#define av_sat_dadd32 av_sat_dadd32_c
#endif
#ifndef av_sat_sub32
#define av_sat_sub32 av_sat_sub32_c
#endif
#ifndef av_sat_dsub32
#define av_sat_dsub32 av_sat_dsub32_c
#endif
#ifndef av_sat_add64
#define av_sat_add64 av_sat_add64_c
#endif
#ifndef av_sat_sub64
#define av_sat_sub64 av_sat_sub64_c
#endif
#ifndef av_clipf
#define av_clipf av_clipf_c
#endif
#ifndef av_clipd
#define av_clipd av_clipd_c
#endif
#ifndef av_popcount
#define av_popcount av_popcount_c
#endif
#ifndef av_popcount64
#define av_popcount64 av_popcount64_c
#endif
#ifndef av_parity
#define av_parity av_parity_c
#endif

#ifndef FFMPEG_CONFIG_H
#define FFMPEG_CONFIG_H
#define FFMPEG_CONFIGURATION "--target-os=mingw32 --arch=x86_64 --disable-debug --disable-stripping --disable-doc --disable-w32threads --disable-static --enable-shared --enable-sdl2 --enable-vulkan --enable-dxva2 --enable-d3d11va --enable-gpl --enable-fontconfig --enable-iconv --enable-gnutls --enable-libxml2 --enable-lzma --enable-libsnappy --enable-zlib --enable-libsrt --enable-libssh --enable-libzmq --enable-libbluray --enable-libcaca --enable-libdav1d --enable-libwebp --enable-libx264 --enable-libx265 --enable-libxvid --enable-libaom --enable-libopenjpeg --enable-libvpx --enable-libass --enable-libfreetype --enable-libfribidi --enable-libvidstab --enable-libzimg --enable-libmfx --enable-libcdio --enable-libgme --enable-libmodplug --enable-libmp3lame --enable-libshine --enable-libtheora --enable-libtwolame --enable-libwavpack --enable-libilbc --enable-libgsm --enable-libopus --enable-libspeex --enable-libvorbis --enable-libbs2b --enable-libmysofa --enable-librubberband --enable-libsoxr --enable-chromaprint"
#define FFMPEG_LICENSE "GPL version 2 or later"
#define CONFIG_THIS_YEAR 2020
#define FFMPEG_DATADIR "/usr/local/share/ffmpeg"
#define AVCONV_DATADIR "/usr/local/share/ffmpeg"
#define CC_IDENT "gcc 10.2.0 (Rev3, Built by MSYS2 project)"
#define av_restrict restrict
#define EXTERN_PREFIX ""
#define EXTERN_ASM
#define BUILDSUF ""
#define SLIBSUF ".dll"
#define HAVE_MMX2 HAVE_MMXEXT
#define ARCH_AARCH64 0
#define ARCH_ALPHA 0
#define ARCH_ARM 0
#define ARCH_AVR32 0
#define ARCH_AVR32_AP 0
#define ARCH_AVR32_UC 0
#define ARCH_BFIN 0
#define ARCH_IA64 0
#define ARCH_M68K 0
#define ARCH_MIPS 0
#define ARCH_MIPS64 0
#define ARCH_PARISC 0
#define ARCH_PPC 0
#define ARCH_PPC64 0
#define ARCH_S390 0
#define ARCH_SH4 0
#define ARCH_SPARC 0
#define ARCH_SPARC64 0
#define ARCH_TILEGX 0
#define ARCH_TILEPRO 0
#define ARCH_TOMI 0
#define ARCH_X86 1
#define ARCH_X86_32 0
#define ARCH_X86_64 1
#define HAVE_ARMV5TE 0
#define HAVE_ARMV6 0
#define HAVE_ARMV6T2 0
#define HAVE_ARMV8 0
#define HAVE_NEON 0
#define HAVE_VFP 0
#define HAVE_VFPV3 0
#define HAVE_SETEND 0
#define HAVE_ALTIVEC 0
#define HAVE_DCBZL 0
#define HAVE_LDBRX 0
#define HAVE_POWER8 0
#define HAVE_PPC4XX 0
#define HAVE_VSX 0
#define HAVE_AESNI 1
#define HAVE_AMD3DNOW 1
#define HAVE_AMD3DNOWEXT 1
#define HAVE_AVX 1
#define HAVE_AVX2 1
#define HAVE_AVX512 1
#define HAVE_FMA3 1
#define HAVE_FMA4 1
#define HAVE_MMX 1
#define HAVE_MMXEXT 1
#define HAVE_SSE 1
#define HAVE_SSE2 1
#define HAVE_SSE3 1
#define HAVE_SSE4 1
#define HAVE_SSE42 1
#define HAVE_SSSE3 1
#define HAVE_XOP 1
#define HAVE_CPUNOP 0
#define HAVE_I686 1
#define HAVE_MIPSFPU 0
#define HAVE_MIPS32R2 0
#define HAVE_MIPS32R5 0
#define HAVE_MIPS64R2 0
#define HAVE_MIPS32R6 0
#define HAVE_MIPS64R6 0
#define HAVE_MIPSDSP 0
#define HAVE_MIPSDSPR2 0
#define HAVE_MSA 0
#define HAVE_MSA2 0
#define HAVE_LOONGSON2 0
#define HAVE_LOONGSON3 0
#define HAVE_MMI 0
#define HAVE_ARMV5TE_EXTERNAL 0
#define HAVE_ARMV6_EXTERNAL 0
#define HAVE_ARMV6T2_EXTERNAL 0
#define HAVE_ARMV8_EXTERNAL 0
#define HAVE_NEON_EXTERNAL 0
#define HAVE_VFP_EXTERNAL 0
#define HAVE_VFPV3_EXTERNAL 0
#define HAVE_SETEND_EXTERNAL 0
#define HAVE_ALTIVEC_EXTERNAL 0
#define HAVE_DCBZL_EXTERNAL 0
#define HAVE_LDBRX_EXTERNAL 0
#define HAVE_POWER8_EXTERNAL 0
#define HAVE_PPC4XX_EXTERNAL 0
#define HAVE_VSX_EXTERNAL 0
#define HAVE_AESNI_EXTERNAL 1
#define HAVE_AMD3DNOW_EXTERNAL 1
#define HAVE_AMD3DNOWEXT_EXTERNAL 1
#define HAVE_AVX_EXTERNAL 1
#define HAVE_AVX2_EXTERNAL 1
#define HAVE_AVX512_EXTERNAL 1
#define HAVE_FMA3_EXTERNAL 1
#define HAVE_FMA4_EXTERNAL 1
#define HAVE_MMX_EXTERNAL 1
#define HAVE_MMXEXT_EXTERNAL 1
#define HAVE_SSE_EXTERNAL 1
#define HAVE_SSE2_EXTERNAL 1
#define HAVE_SSE3_EXTERNAL 1
#define HAVE_SSE4_EXTERNAL 1
#define HAVE_SSE42_EXTERNAL 1
#define HAVE_SSSE3_EXTERNAL 1
#define HAVE_XOP_EXTERNAL 1
#define HAVE_CPUNOP_EXTERNAL 0
#define HAVE_I686_EXTERNAL 0
#define HAVE_MIPSFPU_EXTERNAL 0
#define HAVE_MIPS32R2_EXTERNAL 0
#define HAVE_MIPS32R5_EXTERNAL 0
#define HAVE_MIPS64R2_EXTERNAL 0
#define HAVE_MIPS32R6_EXTERNAL 0
#define HAVE_MIPS64R6_EXTERNAL 0
#define HAVE_MIPSDSP_EXTERNAL 0
#define HAVE_MIPSDSPR2_EXTERNAL 0
#define HAVE_MSA_EXTERNAL 0
#define HAVE_MSA2_EXTERNAL 0
#define HAVE_LOONGSON2_EXTERNAL 0
#define HAVE_LOONGSON3_EXTERNAL 0
#define HAVE_MMI_EXTERNAL 0
#define HAVE_ARMV5TE_INLINE 0
#define HAVE_ARMV6_INLINE 0
#define HAVE_ARMV6T2_INLINE 0
#define HAVE_ARMV8_INLINE 0
#define HAVE_NEON_INLINE 0
#define HAVE_VFP_INLINE 0
#define HAVE_VFPV3_INLINE 0
#define HAVE_SETEND_INLINE 0
#define HAVE_ALTIVEC_INLINE 0
#define HAVE_DCBZL_INLINE 0
#define HAVE_LDBRX_INLINE 0
#define HAVE_POWER8_INLINE 0
#define HAVE_PPC4XX_INLINE 0
#define HAVE_VSX_INLINE 0
#define HAVE_AESNI_INLINE 1
#define HAVE_AMD3DNOW_INLINE 1
#define HAVE_AMD3DNOWEXT_INLINE 1
#define HAVE_AVX_INLINE 1
#define HAVE_AVX2_INLINE 1
#define HAVE_AVX512_INLINE 1
#define HAVE_FMA3_INLINE 1
#define HAVE_FMA4_INLINE 1
#define HAVE_MMX_INLINE 1
#define HAVE_MMXEXT_INLINE 1
#define HAVE_SSE_INLINE 1
#define HAVE_SSE2_INLINE 1
#define HAVE_SSE3_INLINE 1
#define HAVE_SSE4_INLINE 1
#define HAVE_SSE42_INLINE 1
#define HAVE_SSSE3_INLINE 1
#define HAVE_XOP_INLINE 1
#define HAVE_CPUNOP_INLINE 0
#define HAVE_I686_INLINE 0
#define HAVE_MIPSFPU_INLINE 0
#define HAVE_MIPS32R2_INLINE 0
#define HAVE_MIPS32R5_INLINE 0
#define HAVE_MIPS64R2_INLINE 0
#define HAVE_MIPS32R6_INLINE 0
#define HAVE_MIPS64R6_INLINE 0
#define HAVE_MIPSDSP_INLINE 0
#define HAVE_MIPSDSPR2_INLINE 0
#define HAVE_MSA_INLINE 0
#define HAVE_MSA2_INLINE 0
#define HAVE_LOONGSON2_INLINE 0
#define HAVE_LOONGSON3_INLINE 0
#define HAVE_MMI_INLINE 0
#define HAVE_ALIGNED_STACK 1
#define HAVE_FAST_64BIT 1
#define HAVE_FAST_CLZ 1
#define HAVE_FAST_CMOV 1
#define HAVE_LOCAL_ALIGNED 1
#define HAVE_SIMD_ALIGN_16 1
#define HAVE_SIMD_ALIGN_32 1
#define HAVE_SIMD_ALIGN_64 1
#define HAVE_ATOMIC_CAS_PTR 0
#define HAVE_MACHINE_RW_BARRIER 0
#define HAVE_MEMORYBARRIER 1
#define HAVE_MM_EMPTY 1
#define HAVE_RDTSC 1
#define HAVE_SEM_TIMEDWAIT 1
#define HAVE_SYNC_VAL_COMPARE_AND_SWAP 1
#define HAVE_CABS 1
#define HAVE_CEXP 1
#define HAVE_INLINE_ASM 1
#define HAVE_SYMVER 1
#define HAVE_X86ASM 1
#define HAVE_BIGENDIAN 0
#define HAVE_FAST_UNALIGNED 1
#define HAVE_ARPA_INET_H 0
#define HAVE_ASM_TYPES_H 0
#define HAVE_CDIO_PARANOIA_H 0
#define HAVE_CDIO_PARANOIA_PARANOIA_H 1
#define HAVE_CUDA_H 0
#define HAVE_DISPATCH_DISPATCH_H 0
#define HAVE_DEV_BKTR_IOCTL_BT848_H 0
#define HAVE_DEV_BKTR_IOCTL_METEOR_H 0
#define HAVE_DEV_IC_BT8XX_H 0
#define HAVE_DEV_VIDEO_BKTR_IOCTL_BT848_H 0
#define HAVE_DEV_VIDEO_METEOR_IOCTL_METEOR_H 0
#define HAVE_DIRECT_H 1
#define HAVE_DIRENT_H 1
#define HAVE_DXGIDEBUG_H 1
#define HAVE_DXVA_H 1
#define HAVE_ES2_GL_H 0
#define HAVE_GSM_H 1
#define HAVE_IO_H 1
#define HAVE_LINUX_PERF_EVENT_H 0
#define HAVE_MACHINE_IOCTL_BT848_H 0
#define HAVE_MACHINE_IOCTL_METEOR_H 0
#define HAVE_MALLOC_H 1
#define HAVE_OPENCV2_CORE_CORE_C_H 0
#define HAVE_OPENGL_GL3_H 0
#define HAVE_POLL_H 0
#define HAVE_SYS_PARAM_H 1
#define HAVE_SYS_RESOURCE_H 0
#define HAVE_SYS_SELECT_H 0
#define HAVE_SYS_SOUNDCARD_H 0
#define HAVE_SYS_TIME_H 1
#define HAVE_SYS_UN_H 0
#define HAVE_SYS_VIDEOIO_H 0
#define HAVE_TERMIOS_H 0
#define HAVE_UDPLITE_H 0
#define HAVE_UNISTD_H 1
#define HAVE_VALGRIND_VALGRIND_H 0
#define HAVE_WINDOWS_H 1
#define HAVE_WINSOCK2_H 1
#define HAVE_INTRINSICS_NEON 0
#define HAVE_ATANF 1
#define HAVE_ATAN2F 1
#define HAVE_CBRT 1
#define HAVE_CBRTF 1
#define HAVE_COPYSIGN 1
#define HAVE_COSF 1
#define HAVE_ERF 1
#define HAVE_EXP2 1
#define HAVE_EXP2F 1
#define HAVE_EXPF 1
#define HAVE_HYPOT 1
#define HAVE_ISFINITE 1
#define HAVE_ISINF 1
#define HAVE_ISNAN 1
#define HAVE_LDEXPF 1
#define HAVE_LLRINT 1
#define HAVE_LLRINTF 1
#define HAVE_LOG2 1
#define HAVE_LOG2F 1
#define HAVE_LOG10F 1
#define HAVE_LRINT 1
#define HAVE_LRINTF 1
#define HAVE_POWF 1
#define HAVE_RINT 1
#define HAVE_ROUND 1
#define HAVE_ROUNDF 1
#define HAVE_SINF 1
#define HAVE_TRUNC 1
#define HAVE_TRUNCF 1
#define HAVE_DOS_PATHS 1
#define HAVE_LIBC_MSVCRT 0
#define HAVE_MMAL_PARAMETER_VIDEO_MAX_NUM_CALLBACKS 0
#define HAVE_SECTION_DATA_REL_RO 0
#define HAVE_THREADS 1
#define HAVE_UWP 0
#define HAVE_WINRT 0
#define HAVE_ACCESS 1
#define HAVE_ALIGNED_MALLOC 1
#define HAVE_ARC4RANDOM 0
#define HAVE_CLOCK_GETTIME 1
#define HAVE_CLOSESOCKET 1
#define HAVE_COMMANDLINETOARGVW 1
#define HAVE_FCNTL 0
#define HAVE_GETADDRINFO 1
#define HAVE_GETHRTIME 0
#define HAVE_GETOPT 1
#define HAVE_GETMODULEHANDLE 1
#define HAVE_GETPROCESSAFFINITYMASK 1
#define HAVE_GETPROCESSMEMORYINFO 1
#define HAVE_GETPROCESSTIMES 1
#define HAVE_GETRUSAGE 0
#define HAVE_GETSTDHANDLE 1
#define HAVE_GETSYSTEMTIMEASFILETIME 1
#define HAVE_GETTIMEOFDAY 1
#define HAVE_GLOB 0
#define HAVE_GLXGETPROCADDRESS 0
#define HAVE_GMTIME_R 1
#define HAVE_INET_ATON 0
#define HAVE_ISATTY 1
#define HAVE_KBHIT 1
#define HAVE_LOCALTIME_R 1
#define HAVE_LSTAT 0
#define HAVE_LZO1X_999_COMPRESS 0
#define HAVE_MACH_ABSOLUTE_TIME 0
#define HAVE_MAPVIEWOFFILE 1
#define HAVE_MEMALIGN 0
#define HAVE_MKSTEMP 1
#define HAVE_MMAP 0
#define HAVE_MPROTECT 1
#define HAVE_NANOSLEEP 1
#define HAVE_PEEKNAMEDPIPE 1
#define HAVE_POSIX_MEMALIGN 0
#define HAVE_PTHREAD_CANCEL 1
#define HAVE_SCHED_GETAFFINITY 0
#define HAVE_SECITEMIMPORT 0
#define HAVE_SETCONSOLETEXTATTRIBUTE 1
#define HAVE_SETCONSOLECTRLHANDLER 1
#define HAVE_SETDLLDIRECTORY 1
#define HAVE_SETMODE 1
#define HAVE_SETRLIMIT 0
#define HAVE_SLEEP 1
#define HAVE_STRERROR_R 0
#define HAVE_SYSCONF 0
#define HAVE_SYSCTL 0
#define HAVE_USLEEP 1
#define HAVE_UTGETOSTYPEFROMSTRING 0
#define HAVE_VIRTUALALLOC 1
#define HAVE_WGLGETPROCADDRESS 0
#define HAVE_BCRYPT 1
#define HAVE_VAAPI_DRM 0
#define HAVE_VAAPI_X11 0
#define HAVE_VDPAU_X11 0
#define HAVE_PTHREADS 1
#define HAVE_OS2THREADS 0
#define HAVE_W32THREADS 0
#define HAVE_AS_ARCH_DIRECTIVE 0
#define HAVE_AS_DN_DIRECTIVE 0
#define HAVE_AS_FPU_DIRECTIVE 0
#define HAVE_AS_FUNC 0
#define HAVE_AS_OBJECT_ARCH 0
#define HAVE_ASM_MOD_Q 0
#define HAVE_BLOCKS_EXTENSION 0
#define HAVE_EBP_AVAILABLE 1
#define HAVE_EBX_AVAILABLE 1
#define HAVE_GNU_AS 0
#define HAVE_GNU_WINDRES 1
#define HAVE_IBM_ASM 0
#define HAVE_INLINE_ASM_DIRECT_SYMBOL_REFS 1
#define HAVE_INLINE_ASM_LABELS 1
#define HAVE_INLINE_ASM_NONLOCAL_LABELS 1
#define HAVE_PRAGMA_DEPRECATED 1
#define HAVE_RSYNC_CONTIMEOUT 0
#define HAVE_SYMVER_ASM_LABEL 1
#define HAVE_SYMVER_GNU_ASM 0
#define HAVE_VFP_ARGS 0
#define HAVE_XFORM_ASM 0
#define HAVE_XMM_CLOBBERS 1
#define HAVE_KCMVIDEOCODECTYPE_HEVC 0
#define HAVE_KCVPIXELFORMATTYPE_420YPCBCR10BIPLANARVIDEORANGE 0
#define HAVE_KCVIMAGEBUFFERTRANSFERFUNCTION_SMPTE_ST_2084_PQ 0
#define HAVE_KCVIMAGEBUFFERTRANSFERFUNCTION_ITU_R_2100_HLG 0
#define HAVE_KCVIMAGEBUFFERTRANSFERFUNCTION_LINEAR 0
#define HAVE_SOCKLEN_T 1
#define HAVE_STRUCT_ADDRINFO 1
#define HAVE_STRUCT_GROUP_SOURCE_REQ 1
#define HAVE_STRUCT_IP_MREQ_SOURCE 1
#define HAVE_STRUCT_IPV6_MREQ 1
#define HAVE_STRUCT_MSGHDR_MSG_FLAGS 0
#define HAVE_STRUCT_POLLFD 1
#define HAVE_STRUCT_RUSAGE_RU_MAXRSS 0
#define HAVE_STRUCT_SCTP_EVENT_SUBSCRIBE 0
#define HAVE_STRUCT_SOCKADDR_IN6 1
#define HAVE_STRUCT_SOCKADDR_SA_LEN 0
#define HAVE_STRUCT_SOCKADDR_STORAGE 1
#define HAVE_STRUCT_STAT_ST_MTIM_TV_NSEC 0
#define HAVE_STRUCT_V4L2_FRMIVALENUM_DISCRETE 0
#define HAVE_LIBDRM_GETFB2 0
#define HAVE_MAKEINFO 1
#define HAVE_MAKEINFO_HTML 1
#define HAVE_OPENCL_D3D11 0
#define HAVE_OPENCL_DRM_ARM 0
#define HAVE_OPENCL_DRM_BEIGNET 0
#define HAVE_OPENCL_DXVA2 0
#define HAVE_OPENCL_VAAPI_BEIGNET 0
#define HAVE_OPENCL_VAAPI_INTEL_MEDIA 0
#define HAVE_PERL 1
#define HAVE_POD2MAN 1
#define HAVE_TEXI2HTML 0
#define CONFIG_DOC 0
#define CONFIG_HTMLPAGES 1
#define CONFIG_MANPAGES 1
#define CONFIG_PODPAGES 1
#define CONFIG_TXTPAGES 1
#define CONFIG_AVIO_LIST_DIR_EXAMPLE 1
#define CONFIG_AVIO_READING_EXAMPLE 1
#define CONFIG_DECODE_AUDIO_EXAMPLE 1
#define CONFIG_DECODE_VIDEO_EXAMPLE 1
#define CONFIG_DEMUXING_DECODING_EXAMPLE 1
#define CONFIG_ENCODE_AUDIO_EXAMPLE 1
#define CONFIG_ENCODE_VIDEO_EXAMPLE 1
#define CONFIG_EXTRACT_MVS_EXAMPLE 1
#define CONFIG_FILTER_AUDIO_EXAMPLE 1
#define CONFIG_FILTERING_AUDIO_EXAMPLE 1
#define CONFIG_FILTERING_VIDEO_EXAMPLE 1
#define CONFIG_HTTP_MULTICLIENT_EXAMPLE 0
#define CONFIG_HW_DECODE_EXAMPLE 1
#define CONFIG_METADATA_EXAMPLE 1
#define CONFIG_MUXING_EXAMPLE 1
#define CONFIG_QSVDEC_EXAMPLE 1
#define CONFIG_REMUXING_EXAMPLE 1
#define CONFIG_RESAMPLING_AUDIO_EXAMPLE 1
#define CONFIG_SCALING_VIDEO_EXAMPLE 1
#define CONFIG_TRANSCODE_AAC_EXAMPLE 1
#define CONFIG_TRANSCODING_EXAMPLE 1
#define CONFIG_VAAPI_ENCODE_EXAMPLE 0
#define CONFIG_VAAPI_TRANSCODE_EXAMPLE 0
#define CONFIG_AVISYNTH 0
#define CONFIG_FREI0R 0
#define CONFIG_LIBCDIO 1
#define CONFIG_LIBDAVS2 0
#define CONFIG_LIBRUBBERBAND 1
#define CONFIG_LIBVIDSTAB 1
#define CONFIG_LIBX264 1
#define CONFIG_LIBX265 1
#define CONFIG_LIBXAVS 0
#define CONFIG_LIBXAVS2 0
#define CONFIG_LIBXVID 1
#define CONFIG_DECKLINK 0
#define CONFIG_LIBFDK_AAC 0
#define CONFIG_OPENSSL 0
#define CONFIG_LIBTLS 0
#define CONFIG_GMP 0
#define CONFIG_LIBARIBB24 0
#define CONFIG_LIBLENSFUN 0
#define CONFIG_LIBOPENCORE_AMRNB 0
#define CONFIG_LIBOPENCORE_AMRWB 0
#define CONFIG_LIBVO_AMRWBENC 0
#define CONFIG_MBEDTLS 0
#define CONFIG_RKMPP 0
#define CONFIG_LIBSMBCLIENT 0
#define CONFIG_CHROMAPRINT 1
#define CONFIG_GCRYPT 0
#define CONFIG_GNUTLS 1
#define CONFIG_JNI 0
#define CONFIG_LADSPA 0
#define CONFIG_LIBAOM 1
#define CONFIG_LIBASS 1
#define CONFIG_LIBBLURAY 1
#define CONFIG_LIBBS2B 1
#define CONFIG_LIBCACA 1
#define CONFIG_LIBCELT 0
#define CONFIG_LIBCODEC2 0
#define CONFIG_LIBDAV1D 1
#define CONFIG_LIBDC1394 0
#define CONFIG_LIBDRM 0
#define CONFIG_LIBFLITE 0
#define CONFIG_LIBFONTCONFIG 1
#define CONFIG_LIBFREETYPE 1
#define CONFIG_LIBFRIBIDI 1
#define CONFIG_LIBGLSLANG 0
#define CONFIG_LIBGME 1
#define CONFIG_LIBGSM 1
#define CONFIG_LIBIEC61883 0
#define CONFIG_LIBILBC 1
#define CONFIG_LIBJACK 0
#define CONFIG_LIBKLVANC 0
#define CONFIG_LIBKVAZAAR 0
#define CONFIG_LIBMODPLUG 1
#define CONFIG_LIBMP3LAME 1
#define CONFIG_LIBMYSOFA 1
#define CONFIG_LIBOPENCV 0
#define CONFIG_LIBOPENH264 0
#define CONFIG_LIBOPENJPEG 1
#define CONFIG_LIBOPENMPT 0
#define CONFIG_LIBOPENVINO 0
#define CONFIG_LIBOPUS 1
#define CONFIG_LIBPULSE 0
#define CONFIG_LIBRABBITMQ 0
#define CONFIG_LIBRAV1E 0
#define CONFIG_LIBRSVG 0
#define CONFIG_LIBRTMP 0
#define CONFIG_LIBSHINE 1
#define CONFIG_LIBSMBCLIENT 0
#define CONFIG_LIBSNAPPY 1
#define CONFIG_LIBSOXR 1
#define CONFIG_LIBSPEEX 1
#define CONFIG_LIBSRT 1
#define CONFIG_LIBSSH 1
#define CONFIG_LIBSVTAV1 0
#define CONFIG_LIBTENSORFLOW 0
#define CONFIG_LIBTESSERACT 0
#define CONFIG_LIBTHEORA 1
#define CONFIG_LIBTWOLAME 1
#define CONFIG_LIBV4L2 0
#define CONFIG_LIBVMAF 0
#define CONFIG_LIBVORBIS 1
#define CONFIG_LIBVPX 1
#define CONFIG_LIBWAVPACK 1
#define CONFIG_LIBWEBP 1
#define CONFIG_LIBXML2 1
#define CONFIG_LIBZIMG 1
#define CONFIG_LIBZMQ 1
#define CONFIG_LIBZVBI 0
#define CONFIG_LV2 0
#define CONFIG_MEDIACODEC 0
#define CONFIG_OPENAL 0
#define CONFIG_OPENGL 0
#define CONFIG_POCKETSPHINX 0
#define CONFIG_VAPOURSYNTH 0
#define CONFIG_ALSA 0
#define CONFIG_APPKIT 0
#define CONFIG_AVFOUNDATION 0
#define CONFIG_BZLIB 1
#define CONFIG_COREIMAGE 0
#define CONFIG_ICONV 1
#define CONFIG_LIBXCB 0
#define CONFIG_LIBXCB_SHM 0
#define CONFIG_LIBXCB_SHAPE 0
#define CONFIG_LIBXCB_XFIXES 0
#define CONFIG_LZMA 1
#define CONFIG_MEDIAFOUNDATION 1
#define CONFIG_SCHANNEL 0
#define CONFIG_SDL2 1
#define CONFIG_SECURETRANSPORT 0
#define CONFIG_SNDIO 0
#define CONFIG_XLIB 0
#define CONFIG_ZLIB 1
#define CONFIG_CUDA_NVCC 0
#define CONFIG_CUDA_SDK 0
#define CONFIG_LIBNPP 0
#define CONFIG_LIBMFX 1
#define CONFIG_MMAL 0
#define CONFIG_OMX 0
#define CONFIG_OPENCL 0
#define CONFIG_VULKAN 1
#define CONFIG_AMF 0
#define CONFIG_AUDIOTOOLBOX 0
#define CONFIG_CRYSTALHD 0
#define CONFIG_CUDA 0
#define CONFIG_CUDA_LLVM 0
#define CONFIG_CUVID 0
#define CONFIG_D3D11VA 1
#define CONFIG_DXVA2 1
#define CONFIG_FFNVCODEC 0
#define CONFIG_NVDEC 0
#define CONFIG_NVENC 0
#define CONFIG_VAAPI 0
#define CONFIG_VDPAU 0
#define CONFIG_VIDEOTOOLBOX 0
#define CONFIG_V4L2_M2M 0
#define CONFIG_XVMC 0
#define CONFIG_FTRAPV 0
#define CONFIG_GRAY 0
#define CONFIG_HARDCODED_TABLES 0
#define CONFIG_OMX_RPI 0
#define CONFIG_RUNTIME_CPUDETECT 1
#define CONFIG_SAFE_BITSTREAM_READER 1
#define CONFIG_SHARED 1
#define CONFIG_SMALL 0
#define CONFIG_STATIC 0
#define CONFIG_SWSCALE_ALPHA 1
#define CONFIG_GPL 1
#define CONFIG_NONFREE 0
#define CONFIG_VERSION3 0
#define CONFIG_AVDEVICE 1
#define CONFIG_AVFILTER 1
#define CONFIG_SWSCALE 1
#define CONFIG_POSTPROC 1
#define CONFIG_AVFORMAT 1
#define CONFIG_AVCODEC 1
#define CONFIG_SWRESAMPLE 1
#define CONFIG_AVRESAMPLE 0
#define CONFIG_AVUTIL 1
#define CONFIG_FFPLAY 1
#define CONFIG_FFPROBE 1
#define CONFIG_FFMPEG 1
#define CONFIG_DCT 1
#define CONFIG_DWT 1
#define CONFIG_ERROR_RESILIENCE 1
#define CONFIG_FAAN 1
#define CONFIG_FAST_UNALIGNED 1
#define CONFIG_FFT 1
#define CONFIG_LSP 1
#define CONFIG_LZO 1
#define CONFIG_MDCT 1
#define CONFIG_PIXELUTILS 1
#define CONFIG_NETWORK 1
#define CONFIG_RDFT 1
#define CONFIG_AUTODETECT 0
#define CONFIG_FONTCONFIG 1
#define CONFIG_LARGE_TESTS 1
#define CONFIG_LINUX_PERF 0
#define CONFIG_MEMORY_POISONING 0
#define CONFIG_NEON_CLOBBER_TEST 0
#define CONFIG_OSSFUZZ 0
#define CONFIG_PIC 1
#define CONFIG_THUMB 0
#define CONFIG_VALGRIND_BACKTRACE 0
#define CONFIG_XMM_CLOBBER_TEST 0
#define CONFIG_BSFS 1
#define CONFIG_DECODERS 1
#define CONFIG_ENCODERS 1
#define CONFIG_HWACCELS 1
#define CONFIG_PARSERS 1
#define CONFIG_INDEVS 1
#define CONFIG_OUTDEVS 1
#define CONFIG_FILTERS 1
#define CONFIG_DEMUXERS 1
#define CONFIG_MUXERS 1
#define CONFIG_PROTOCOLS 1
#define CONFIG_AANDCTTABLES 1
#define CONFIG_AC3DSP 1
#define CONFIG_ADTS_HEADER 1
#define CONFIG_ATSC_A53 1
#define CONFIG_AUDIO_FRAME_QUEUE 1
#define CONFIG_AUDIODSP 1
#define CONFIG_BLOCKDSP 1
#define CONFIG_BSWAPDSP 1
#define CONFIG_CABAC 1
#define CONFIG_CBS 1
#define CONFIG_CBS_AV1 1
#define CONFIG_CBS_H264 1
#define CONFIG_CBS_H265 1
#define CONFIG_CBS_JPEG 0
#define CONFIG_CBS_MPEG2 1
#define CONFIG_CBS_VP9 1
#define CONFIG_DIRAC_PARSE 1
#define CONFIG_DNN 1
#define CONFIG_DVPROFILE 1
#define CONFIG_EXIF 1
#define CONFIG_FAANDCT 1
#define CONFIG_FAANIDCT 1
#define CONFIG_FDCTDSP 1
#define CONFIG_FLACDSP 1
#define CONFIG_FMTCONVERT 1
#define CONFIG_FRAME_THREAD_ENCODER 1
#define CONFIG_G722DSP 1
#define CONFIG_GOLOMB 1
#define CONFIG_GPLV3 0
#define CONFIG_H263DSP 1
#define CONFIG_H264CHROMA 1
#define CONFIG_H264DSP 1
#define CONFIG_H264PARSE 1
#define CONFIG_H264PRED 1
#define CONFIG_H264QPEL 1
#define CONFIG_HEVCPARSE 1
#define CONFIG_HPELDSP 1
#define CONFIG_HUFFMAN 1
#define CONFIG_HUFFYUVDSP 1
#define CONFIG_HUFFYUVENCDSP 1
#define CONFIG_IDCTDSP 1
#define CONFIG_IIRFILTER 1
#define CONFIG_MDCT15 1
#define CONFIG_INTRAX8 1
#define CONFIG_ISO_MEDIA 1
#define CONFIG_IVIDSP 1
#define CONFIG_JPEGTABLES 1
#define CONFIG_LGPLV3 0
#define CONFIG_LIBX262 0
#define CONFIG_LLAUDDSP 1
#define CONFIG_LLVIDDSP 1
#define CONFIG_LLVIDENCDSP 1
#define CONFIG_LPC 1
#define CONFIG_LZF 1
#define CONFIG_ME_CMP 1
#define CONFIG_MPEG_ER 1
#define CONFIG_MPEGAUDIO 1
#define CONFIG_MPEGAUDIODSP 1
#define CONFIG_MPEGAUDIOHEADER 1
#define CONFIG_MPEGVIDEO 1
#define CONFIG_MPEGVIDEOENC 1
#define CONFIG_MSS34DSP 1
#define CONFIG_PIXBLOCKDSP 1
#define CONFIG_QPELDSP 1
#define CONFIG_QSV 1
#define CONFIG_QSVDEC 1
#define CONFIG_QSVENC 1
#define CONFIG_QSVVPP 1
#define CONFIG_RANGECODER 1
#define CONFIG_RIFFDEC 1
#define CONFIG_RIFFENC 1
#define CONFIG_RTPDEC 1
#define CONFIG_RTPENC_CHAIN 1
#define CONFIG_RV34DSP 1
#define CONFIG_SCENE_SAD 1
#define CONFIG_SINEWIN 1
#define CONFIG_SNAPPY 1
#define CONFIG_SRTP 1
#define CONFIG_STARTCODE 1
#define CONFIG_TEXTUREDSP 1
#define CONFIG_TEXTUREDSPENC 1
#define CONFIG_TPELDSP 1
#define CONFIG_VAAPI_1 0
#define CONFIG_VAAPI_ENCODE 0
#define CONFIG_VC1DSP 1
#define CONFIG_VIDEODSP 1
#define CONFIG_VP3DSP 1
#define CONFIG_VP56DSP 1
#define CONFIG_VP8DSP 1
#define CONFIG_WMA_FREQS 1
#define CONFIG_WMV2DSP 1
#define CONFIG_AAC_ADTSTOASC_BSF 1
#define CONFIG_AV1_FRAME_MERGE_BSF 1
#define CONFIG_AV1_FRAME_SPLIT_BSF 1
#define CONFIG_AV1_METADATA_BSF 1
#define CONFIG_CHOMP_BSF 1
#define CONFIG_DUMP_EXTRADATA_BSF 1
#define CONFIG_DCA_CORE_BSF 1
#define CONFIG_EAC3_CORE_BSF 1
#define CONFIG_EXTRACT_EXTRADATA_BSF 1
#define CONFIG_FILTER_UNITS_BSF 1
#define CONFIG_H264_METADATA_BSF 1
#define CONFIG_H264_MP4TOANNEXB_BSF 1
#define CONFIG_H264_REDUNDANT_PPS_BSF 1
#define CONFIG_HAPQA_EXTRACT_BSF 1
#define CONFIG_HEVC_METADATA_BSF 1
#define CONFIG_HEVC_MP4TOANNEXB_BSF 1
#define CONFIG_IMX_DUMP_HEADER_BSF 1
#define CONFIG_MJPEG2JPEG_BSF 1
#define CONFIG_MJPEGA_DUMP_HEADER_BSF 1
#define CONFIG_MP3_HEADER_DECOMPRESS_BSF 1
#define CONFIG_MPEG2_METADATA_BSF 1
#define CONFIG_MPEG4_UNPACK_BFRAMES_BSF 1
#define CONFIG_MOV2TEXTSUB_BSF 1
#define CONFIG_NOISE_BSF 1
#define CONFIG_NULL_BSF 1
#define CONFIG_OPUS_METADATA_BSF 1
#define CONFIG_PCM_RECHUNK_BSF 1
#define CONFIG_PRORES_METADATA_BSF 1
#define CONFIG_REMOVE_EXTRADATA_BSF 1
#define CONFIG_TEXT2MOVSUB_BSF 1
#define CONFIG_TRACE_HEADERS_BSF 1
#define CONFIG_TRUEHD_CORE_BSF 1
#define CONFIG_VP9_METADATA_BSF 1
#define CONFIG_VP9_RAW_REORDER_BSF 1
#define CONFIG_VP9_SUPERFRAME_BSF 1
#define CONFIG_VP9_SUPERFRAME_SPLIT_BSF 1
#define CONFIG_AASC_DECODER 1
#define CONFIG_AIC_DECODER 1
#define CONFIG_ALIAS_PIX_DECODER 1
#define CONFIG_AGM_DECODER 1
#define CONFIG_AMV_DECODER 1
#define CONFIG_ANM_DECODER 1
#define CONFIG_ANSI_DECODER 1
#define CONFIG_APNG_DECODER 1
#define CONFIG_ARBC_DECODER 1
#define CONFIG_ASV1_DECODER 1
#define CONFIG_ASV2_DECODER 1
#define CONFIG_AURA_DECODER 1
#define CONFIG_AURA2_DECODER 1
#define CONFIG_AVRP_DECODER 1
#define CONFIG_AVRN_DECODER 1
#define CONFIG_AVS_DECODER 1
#define CONFIG_AVUI_DECODER 1
#define CONFIG_AYUV_DECODER 1
#define CONFIG_BETHSOFTVID_DECODER 1
#define CONFIG_BFI_DECODER 1
#define CONFIG_BINK_DECODER 1
#define CONFIG_BITPACKED_DECODER 1
#define CONFIG_BMP_DECODER 1
#define CONFIG_BMV_VIDEO_DECODER 1
#define CONFIG_BRENDER_PIX_DECODER 1
#define CONFIG_C93_DECODER 1
#define CONFIG_CAVS_DECODER 1
#define CONFIG_CDGRAPHICS_DECODER 1
#define CONFIG_CDTOONS_DECODER 1
#define CONFIG_CDXL_DECODER 1
#define CONFIG_CFHD_DECODER 1
#define CONFIG_CINEPAK_DECODER 1
#define CONFIG_CLEARVIDEO_DECODER 1
#define CONFIG_CLJR_DECODER 1
#define CONFIG_CLLC_DECODER 1
#define CONFIG_COMFORTNOISE_DECODER 1
#define CONFIG_CPIA_DECODER 1
#define CONFIG_CSCD_DECODER 1
#define CONFIG_CYUV_DECODER 1
#define CONFIG_DDS_DECODER 1
#define CONFIG_DFA_DECODER 1
#define CONFIG_DIRAC_DECODER 1
#define CONFIG_DNXHD_DECODER 1
#define CONFIG_DPX_DECODER 1
#define CONFIG_DSICINVIDEO_DECODER 1
#define CONFIG_DVAUDIO_DECODER 1
#define CONFIG_DVVIDEO_DECODER 1
#define CONFIG_DXA_DECODER 1
#define CONFIG_DXTORY_DECODER 1
#define CONFIG_DXV_DECODER 1
#define CONFIG_EACMV_DECODER 1
#define CONFIG_EAMAD_DECODER 1
#define CONFIG_EATGQ_DECODER 1
#define CONFIG_EATGV_DECODER 1
#define CONFIG_EATQI_DECODER 1
#define CONFIG_EIGHTBPS_DECODER 1
#define CONFIG_EIGHTSVX_EXP_DECODER 1
#define CONFIG_EIGHTSVX_FIB_DECODER 1
#define CONFIG_ESCAPE124_DECODER 1
#define CONFIG_ESCAPE130_DECODER 1
#define CONFIG_EXR_DECODER 1
#define CONFIG_FFV1_DECODER 1
#define CONFIG_FFVHUFF_DECODER 1
#define CONFIG_FIC_DECODER 1
#define CONFIG_FITS_DECODER 1
#define CONFIG_FLASHSV_DECODER 1
#define CONFIG_FLASHSV2_DECODER 1
#define CONFIG_FLIC_DECODER 1
#define CONFIG_FLV_DECODER 1
#define CONFIG_FMVC_DECODER 1
#define CONFIG_FOURXM_DECODER 1
#define CONFIG_FRAPS_DECODER 1
#define CONFIG_FRWU_DECODER 1
#define CONFIG_G2M_DECODER 1
#define CONFIG_GDV_DECODER 1
#define CONFIG_GIF_DECODER 1
#define CONFIG_H261_DECODER 1
#define CONFIG_H263_DECODER 1
#define CONFIG_H263I_DECODER 1
#define CONFIG_H263P_DECODER 1
#define CONFIG_H263_V4L2M2M_DECODER 0
#define CONFIG_H264_DECODER 1
#define CONFIG_H264_CRYSTALHD_DECODER 0
#define CONFIG_H264_V4L2M2M_DECODER 0
#define CONFIG_H264_MEDIACODEC_DECODER 0
#define CONFIG_H264_MMAL_DECODER 0
#define CONFIG_H264_QSV_DECODER 1
#define CONFIG_H264_RKMPP_DECODER 0
#define CONFIG_HAP_DECODER 1
#define CONFIG_HEVC_DECODER 1
#define CONFIG_HEVC_QSV_DECODER 1
#define CONFIG_HEVC_RKMPP_DECODER 0
#define CONFIG_HEVC_V4L2M2M_DECODER 0
#define CONFIG_HNM4_VIDEO_DECODER 1
#define CONFIG_HQ_HQA_DECODER 1
#define CONFIG_HQX_DECODER 1
#define CONFIG_HUFFYUV_DECODER 1
#define CONFIG_HYMT_DECODER 1
#define CONFIG_IDCIN_DECODER 1
#define CONFIG_IFF_ILBM_DECODER 1
#define CONFIG_IMM4_DECODER 1
#define CONFIG_IMM5_DECODER 1
#define CONFIG_INDEO2_DECODER 1
#define CONFIG_INDEO3_DECODER 1
#define CONFIG_INDEO4_DECODER 1
#define CONFIG_INDEO5_DECODER 1
#define CONFIG_INTERPLAY_VIDEO_DECODER 1
#define CONFIG_IPU_DECODER 1
#define CONFIG_JPEG2000_DECODER 1
#define CONFIG_JPEGLS_DECODER 1
#define CONFIG_JV_DECODER 1
#define CONFIG_KGV1_DECODER 1
#define CONFIG_KMVC_DECODER 1
#define CONFIG_LAGARITH_DECODER 1
#define CONFIG_LOCO_DECODER 1
#define CONFIG_LSCR_DECODER 1
#define CONFIG_M101_DECODER 1
#define CONFIG_MAGICYUV_DECODER 1
#define CONFIG_MDEC_DECODER 1
#define CONFIG_MIMIC_DECODER 1
#define CONFIG_MJPEG_DECODER 1
#define CONFIG_MJPEGB_DECODER 1
#define CONFIG_MMVIDEO_DECODER 1
#define CONFIG_MOBICLIP_DECODER 1
#define CONFIG_MOTIONPIXELS_DECODER 1
#define CONFIG_MPEG1VIDEO_DECODER 1
#define CONFIG_MPEG2VIDEO_DECODER 1
#define CONFIG_MPEG4_DECODER 1
#define CONFIG_MPEG4_CRYSTALHD_DECODER 0
#define CONFIG_MPEG4_V4L2M2M_DECODER 0
#define CONFIG_MPEG4_MMAL_DECODER 0
#define CONFIG_MPEGVIDEO_DECODER 1
#define CONFIG_MPEG1_V4L2M2M_DECODER 0
#define CONFIG_MPEG2_MMAL_DECODER 0
#define CONFIG_MPEG2_CRYSTALHD_DECODER 0
#define CONFIG_MPEG2_V4L2M2M_DECODER 0
#define CONFIG_MPEG2_QSV_DECODER 1
#define CONFIG_MPEG2_MEDIACODEC_DECODER 0
#define CONFIG_MSA1_DECODER 1
#define CONFIG_MSCC_DECODER 1
#define CONFIG_MSMPEG4V1_DECODER 1
#define CONFIG_MSMPEG4V2_DECODER 1
#define CONFIG_MSMPEG4V3_DECODER 1
#define CONFIG_MSMPEG4_CRYSTALHD_DECODER 0
#define CONFIG_MSRLE_DECODER 1
#define CONFIG_MSS1_DECODER 1
#define CONFIG_MSS2_DECODER 1
#define CONFIG_MSVIDEO1_DECODER 1
#define CONFIG_MSZH_DECODER 1
#define CONFIG_MTS2_DECODER 1
#define CONFIG_MV30_DECODER 1
#define CONFIG_MVC1_DECODER 1
#define CONFIG_MVC2_DECODER 1
#define CONFIG_MVDV_DECODER 1
#define CONFIG_MVHA_DECODER 1
#define CONFIG_MWSC_DECODER 1
#define CONFIG_MXPEG_DECODER 1
#define CONFIG_NOTCHLC_DECODER 1
#define CONFIG_NUV_DECODER 1
#define CONFIG_PAF_VIDEO_DECODER 1
#define CONFIG_PAM_DECODER 1
#define CONFIG_PBM_DECODER 1
#define CONFIG_PCX_DECODER 1
#define CONFIG_PFM_DECODER 1
#define CONFIG_PGM_DECODER 1
#define CONFIG_PGMYUV_DECODER 1
#define CONFIG_PGX_DECODER 1
#define CONFIG_PHOTOCD_DECODER 1
#define CONFIG_PICTOR_DECODER 1
#define CONFIG_PIXLET_DECODER 1
#define CONFIG_PNG_DECODER 1
#define CONFIG_PPM_DECODER 1
#define CONFIG_PRORES_DECODER 1
#define CONFIG_PROSUMER_DECODER 1
#define CONFIG_PSD_DECODER 1
#define CONFIG_PTX_DECODER 1
#define CONFIG_QDRAW_DECODER 1
#define CONFIG_QPEG_DECODER 1
#define CONFIG_QTRLE_DECODER 1
#define CONFIG_R10K_DECODER 1
#define CONFIG_R210_DECODER 1
#define CONFIG_RASC_DECODER 1
#define CONFIG_RAWVIDEO_DECODER 1
#define CONFIG_RL2_DECODER 1
#define CONFIG_ROQ_DECODER 1
#define CONFIG_RPZA_DECODER 1
#define CONFIG_RSCC_DECODER 1
#define CONFIG_RV10_DECODER 1
#define CONFIG_RV20_DECODER 1
#define CONFIG_RV30_DECODER 1
#define CONFIG_RV40_DECODER 1
#define CONFIG_S302M_DECODER 1
#define CONFIG_SANM_DECODER 1
#define CONFIG_SCPR_DECODER 1
#define CONFIG_SCREENPRESSO_DECODER 1
#define CONFIG_SGI_DECODER 1
#define CONFIG_SGIRLE_DECODER 1
#define CONFIG_SHEERVIDEO_DECODER 1
#define CONFIG_SMACKER_DECODER 1
#define CONFIG_SMC_DECODER 1
#define CONFIG_SMVJPEG_DECODER 1
#define CONFIG_SNOW_DECODER 1
#define CONFIG_SP5X_DECODER 1
#define CONFIG_SPEEDHQ_DECODER 1
#define CONFIG_SRGC_DECODER 1
#define CONFIG_SUNRAST_DECODER 1
#define CONFIG_SVQ1_DECODER 1
#define CONFIG_SVQ3_DECODER 1
#define CONFIG_TARGA_DECODER 1
#define CONFIG_TARGA_Y216_DECODER 1
#define CONFIG_TDSC_DECODER 1
#define CONFIG_THEORA_DECODER 1
#define CONFIG_THP_DECODER 1
#define CONFIG_TIERTEXSEQVIDEO_DECODER 1
#define CONFIG_TIFF_DECODER 1
#define CONFIG_TMV_DECODER 1
#define CONFIG_TRUEMOTION1_DECODER 1
#define CONFIG_TRUEMOTION2_DECODER 1
#define CONFIG_TRUEMOTION2RT_DECODER 1
#define CONFIG_TSCC_DECODER 1
#define CONFIG_TSCC2_DECODER 1
#define CONFIG_TXD_DECODER 1
#define CONFIG_ULTI_DECODER 1
#define CONFIG_UTVIDEO_DECODER 1
#define CONFIG_V210_DECODER 1
#define CONFIG_V210X_DECODER 1
#define CONFIG_V308_DECODER 1
#define CONFIG_V408_DECODER 1
#define CONFIG_V410_DECODER 1
#define CONFIG_VB_DECODER 1
#define CONFIG_VBLE_DECODER 1
#define CONFIG_VC1_DECODER 1
#define CONFIG_VC1_CRYSTALHD_DECODER 0
#define CONFIG_VC1IMAGE_DECODER 1
#define CONFIG_VC1_MMAL_DECODER 0
#define CONFIG_VC1_QSV_DECODER 1
#define CONFIG_VC1_V4L2M2M_DECODER 0
#define CONFIG_VCR1_DECODER 1
#define CONFIG_VMDVIDEO_DECODER 1
#define CONFIG_VMNC_DECODER 1
#define CONFIG_VP3_DECODER 1
#define CONFIG_VP4_DECODER 1
#define CONFIG_VP5_DECODER 1
#define CONFIG_VP6_DECODER 1
#define CONFIG_VP6A_DECODER 1
#define CONFIG_VP6F_DECODER 1
#define CONFIG_VP7_DECODER 1
#define CONFIG_VP8_DECODER 1
#define CONFIG_VP8_RKMPP_DECODER 0
#define CONFIG_VP8_V4L2M2M_DECODER 0
#define CONFIG_VP9_DECODER 1
#define CONFIG_VP9_RKMPP_DECODER 0
#define CONFIG_VP9_V4L2M2M_DECODER 0
#define CONFIG_VQA_DECODER 1
#define CONFIG_WEBP_DECODER 1
#define CONFIG_WCMV_DECODER 1
#define CONFIG_WRAPPED_AVFRAME_DECODER 1
#define CONFIG_WMV1_DECODER 1
#define CONFIG_WMV2_DECODER 1
#define CONFIG_WMV3_DECODER 1
#define CONFIG_WMV3_CRYSTALHD_DECODER 0
#define CONFIG_WMV3IMAGE_DECODER 1
#define CONFIG_WNV1_DECODER 1
#define CONFIG_XAN_WC3_DECODER 1
#define CONFIG_XAN_WC4_DECODER 1
#define CONFIG_XBM_DECODER 1
#define CONFIG_XFACE_DECODER 1
#define CONFIG_XL_DECODER 1
#define CONFIG_XPM_DECODER 1
#define CONFIG_XWD_DECODER 1
#define CONFIG_Y41P_DECODER 1
#define CONFIG_YLC_DECODER 1
#define CONFIG_YOP_DECODER 1
#define CONFIG_YUV4_DECODER 1
#define CONFIG_ZERO12V_DECODER 1
#define CONFIG_ZEROCODEC_DECODER 1
#define CONFIG_ZLIB_DECODER 1
#define CONFIG_ZMBV_DECODER 1
#define CONFIG_AAC_DECODER 1
#define CONFIG_AAC_FIXED_DECODER 1
#define CONFIG_AAC_LATM_DECODER 1
#define CONFIG_AC3_DECODER 1
#define CONFIG_AC3_FIXED_DECODER 1
#define CONFIG_ACELP_KELVIN_DECODER 1
#define CONFIG_ALAC_DECODER 1
#define CONFIG_ALS_DECODER 1
#define CONFIG_AMRNB_DECODER 1
#define CONFIG_AMRWB_DECODER 1
#define CONFIG_APE_DECODER 1
#define CONFIG_APTX_DECODER 1
#define CONFIG_APTX_HD_DECODER 1
#define CONFIG_ATRAC1_DECODER 1
#define CONFIG_ATRAC3_DECODER 1
#define CONFIG_ATRAC3AL_DECODER 1
#define CONFIG_ATRAC3P_DECODER 1
#define CONFIG_ATRAC3PAL_DECODER 1
#define CONFIG_ATRAC9_DECODER 1
#define CONFIG_BINKAUDIO_DCT_DECODER 1
#define CONFIG_BINKAUDIO_RDFT_DECODER 1
#define CONFIG_BMV_AUDIO_DECODER 1
#define CONFIG_COOK_DECODER 1
#define CONFIG_DCA_DECODER 1
#define CONFIG_DOLBY_E_DECODER 1
#define CONFIG_DSD_LSBF_DECODER 1
#define CONFIG_DSD_MSBF_DECODER 1
#define CONFIG_DSD_LSBF_PLANAR_DECODER 1
#define CONFIG_DSD_MSBF_PLANAR_DECODER 1
#define CONFIG_DSICINAUDIO_DECODER 1
#define CONFIG_DSS_SP_DECODER 1
#define CONFIG_DST_DECODER 1
#define CONFIG_EAC3_DECODER 1
#define CONFIG_EVRC_DECODER 1
#define CONFIG_FASTAUDIO_DECODER 1
#define CONFIG_FFWAVESYNTH_DECODER 1
#define CONFIG_FLAC_DECODER 1
#define CONFIG_G723_1_DECODER 1
#define CONFIG_G729_DECODER 1
#define CONFIG_GSM_DECODER 1
#define CONFIG_GSM_MS_DECODER 1
#define CONFIG_HCA_DECODER 1
#define CONFIG_HCOM_DECODER 1
#define CONFIG_IAC_DECODER 1
#define CONFIG_ILBC_DECODER 1
#define CONFIG_IMC_DECODER 1
#define CONFIG_INTERPLAY_ACM_DECODER 1
#define CONFIG_MACE3_DECODER 1
#define CONFIG_MACE6_DECODER 1
#define CONFIG_METASOUND_DECODER 1
#define CONFIG_MLP_DECODER 1
#define CONFIG_MP1_DECODER 1
#define CONFIG_MP1FLOAT_DECODER 1
#define CONFIG_MP2_DECODER 1
#define CONFIG_MP2FLOAT_DECODER 1
#define CONFIG_MP3FLOAT_DECODER 1
#define CONFIG_MP3_DECODER 1
#define CONFIG_MP3ADUFLOAT_DECODER 1
#define CONFIG_MP3ADU_DECODER 1
#define CONFIG_MP3ON4FLOAT_DECODER 1
#define CONFIG_MP3ON4_DECODER 1
#define CONFIG_MPC7_DECODER 1
#define CONFIG_MPC8_DECODER 1
#define CONFIG_NELLYMOSER_DECODER 1
#define CONFIG_ON2AVC_DECODER 1
#define CONFIG_OPUS_DECODER 1
#define CONFIG_PAF_AUDIO_DECODER 1
#define CONFIG_QCELP_DECODER 1
#define CONFIG_QDM2_DECODER 1
#define CONFIG_QDMC_DECODER 1
#define CONFIG_RA_144_DECODER 1
#define CONFIG_RA_288_DECODER 1
#define CONFIG_RALF_DECODER 1
#define CONFIG_SBC_DECODER 1
#define CONFIG_SHORTEN_DECODER 1
#define CONFIG_SIPR_DECODER 1
#define CONFIG_SIREN_DECODER 1
#define CONFIG_SMACKAUD_DECODER 1
#define CONFIG_SONIC_DECODER 1
#define CONFIG_TAK_DECODER 1
#define CONFIG_TRUEHD_DECODER 1
#define CONFIG_TRUESPEECH_DECODER 1
#define CONFIG_TTA_DECODER 1
#define CONFIG_TWINVQ_DECODER 1
#define CONFIG_VMDAUDIO_DECODER 1
#define CONFIG_VORBIS_DECODER 1
#define CONFIG_WAVPACK_DECODER 1
#define CONFIG_WMALOSSLESS_DECODER 1
#define CONFIG_WMAPRO_DECODER 1
#define CONFIG_WMAV1_DECODER 1
#define CONFIG_WMAV2_DECODER 1
#define CONFIG_WMAVOICE_DECODER 1
#define CONFIG_WS_SND1_DECODER 1
#define CONFIG_XMA1_DECODER 1
#define CONFIG_XMA2_DECODER 1
#define CONFIG_PCM_ALAW_DECODER 1
#define CONFIG_PCM_BLURAY_DECODER 1
#define CONFIG_PCM_DVD_DECODER 1
#define CONFIG_PCM_F16LE_DECODER 1
#define CONFIG_PCM_F24LE_DECODER 1
#define CONFIG_PCM_F32BE_DECODER 1
#define CONFIG_PCM_F32LE_DECODER 1
#define CONFIG_PCM_F64BE_DECODER 1
#define CONFIG_PCM_F64LE_DECODER 1
#define CONFIG_PCM_LXF_DECODER 1
#define CONFIG_PCM_MULAW_DECODER 1
#define CONFIG_PCM_S8_DECODER 1
#define CONFIG_PCM_S8_PLANAR_DECODER 1
#define CONFIG_PCM_S16BE_DECODER 1
#define CONFIG_PCM_S16BE_PLANAR_DECODER 1
#define CONFIG_PCM_S16LE_DECODER 1
#define CONFIG_PCM_S16LE_PLANAR_DECODER 1
#define CONFIG_PCM_S24BE_DECODER 1
#define CONFIG_PCM_S24DAUD_DECODER 1
#define CONFIG_PCM_S24LE_DECODER 1
#define CONFIG_PCM_S24LE_PLANAR_DECODER 1
#define CONFIG_PCM_S32BE_DECODER 1
#define CONFIG_PCM_S32LE_DECODER 1
#define CONFIG_PCM_S32LE_PLANAR_DECODER 1
#define CONFIG_PCM_S64BE_DECODER 1
#define CONFIG_PCM_S64LE_DECODER 1
#define CONFIG_PCM_U8_DECODER 1
#define CONFIG_PCM_U16BE_DECODER 1
#define CONFIG_PCM_U16LE_DECODER 1
#define CONFIG_PCM_U24BE_DECODER 1
#define CONFIG_PCM_U24LE_DECODER 1
#define CONFIG_PCM_U32BE_DECODER 1
#define CONFIG_PCM_U32LE_DECODER 1
#define CONFIG_PCM_VIDC_DECODER 1
#define CONFIG_DERF_DPCM_DECODER 1
#define CONFIG_GREMLIN_DPCM_DECODER 1
#define CONFIG_INTERPLAY_DPCM_DECODER 1
#define CONFIG_ROQ_DPCM_DECODER 1
#define CONFIG_SDX2_DPCM_DECODER 1
#define CONFIG_SOL_DPCM_DECODER 1
#define CONFIG_XAN_DPCM_DECODER 1
#define CONFIG_ADPCM_4XM_DECODER 1
#define CONFIG_ADPCM_ADX_DECODER 1
#define CONFIG_ADPCM_AFC_DECODER 1
#define CONFIG_ADPCM_AGM_DECODER 1
#define CONFIG_ADPCM_AICA_DECODER 1
#define CONFIG_ADPCM_ARGO_DECODER 1
#define CONFIG_ADPCM_CT_DECODER 1
#define CONFIG_ADPCM_DTK_DECODER 1
#define CONFIG_ADPCM_EA_DECODER 1
#define CONFIG_ADPCM_EA_MAXIS_XA_DECODER 1
#define CONFIG_ADPCM_EA_R1_DECODER 1
#define CONFIG_ADPCM_EA_R2_DECODER 1
#define CONFIG_ADPCM_EA_R3_DECODER 1
#define CONFIG_ADPCM_EA_XAS_DECODER 1
#define CONFIG_ADPCM_G722_DECODER 1
#define CONFIG_ADPCM_G726_DECODER 1
#define CONFIG_ADPCM_G726LE_DECODER 1
#define CONFIG_ADPCM_IMA_AMV_DECODER 1
#define CONFIG_ADPCM_IMA_ALP_DECODER 1
#define CONFIG_ADPCM_IMA_APC_DECODER 1
#define CONFIG_ADPCM_IMA_APM_DECODER 1
#define CONFIG_ADPCM_IMA_CUNNING_DECODER 1
#define CONFIG_ADPCM_IMA_DAT4_DECODER 1
#define CONFIG_ADPCM_IMA_DK3_DECODER 1
#define CONFIG_ADPCM_IMA_DK4_DECODER 1
#define CONFIG_ADPCM_IMA_EA_EACS_DECODER 1
#define CONFIG_ADPCM_IMA_EA_SEAD_DECODER 1
#define CONFIG_ADPCM_IMA_ISS_DECODER 1
#define CONFIG_ADPCM_IMA_MOFLEX_DECODER 1
#define CONFIG_ADPCM_IMA_MTF_DECODER 1
#define CONFIG_ADPCM_IMA_OKI_DECODER 1
#define CONFIG_ADPCM_IMA_QT_DECODER 1
#define CONFIG_ADPCM_IMA_RAD_DECODER 1
#define CONFIG_ADPCM_IMA_SSI_DECODER 1
#define CONFIG_ADPCM_IMA_SMJPEG_DECODER 1
#define CONFIG_ADPCM_IMA_WAV_DECODER 1
#define CONFIG_ADPCM_IMA_WS_DECODER 1
#define CONFIG_ADPCM_MS_DECODER 1
#define CONFIG_ADPCM_MTAF_DECODER 1
#define CONFIG_ADPCM_PSX_DECODER 1
#define CONFIG_ADPCM_SBPRO_2_DECODER 1
#define CONFIG_ADPCM_SBPRO_3_DECODER 1
#define CONFIG_ADPCM_SBPRO_4_DECODER 1
#define CONFIG_ADPCM_SWF_DECODER 1
#define CONFIG_ADPCM_THP_DECODER 1
#define CONFIG_ADPCM_THP_LE_DECODER 1
#define CONFIG_ADPCM_VIMA_DECODER 1
#define CONFIG_ADPCM_XA_DECODER 1
#define CONFIG_ADPCM_YAMAHA_DECODER 1
#define CONFIG_ADPCM_ZORK_DECODER 1
#define CONFIG_SSA_DECODER 1
#define CONFIG_ASS_DECODER 1
#define CONFIG_CCAPTION_DECODER 1
#define CONFIG_DVBSUB_DECODER 1
#define CONFIG_DVDSUB_DECODER 1
#define CONFIG_JACOSUB_DECODER 1
#define CONFIG_MICRODVD_DECODER 1
#define CONFIG_MOVTEXT_DECODER 1
#define CONFIG_MPL2_DECODER 1
#define CONFIG_PGSSUB_DECODER 1
#define CONFIG_PJS_DECODER 1
#define CONFIG_REALTEXT_DECODER 1
#define CONFIG_SAMI_DECODER 1
#define CONFIG_SRT_DECODER 1
#define CONFIG_STL_DECODER 1
#define CONFIG_SUBRIP_DECODER 1
#define CONFIG_SUBVIEWER_DECODER 1
#define CONFIG_SUBVIEWER1_DECODER 1
#define CONFIG_TEXT_DECODER 1
#define CONFIG_VPLAYER_DECODER 1
#define CONFIG_WEBVTT_DECODER 1
#define CONFIG_XSUB_DECODER 1
#define CONFIG_AAC_AT_DECODER 0
#define CONFIG_AC3_AT_DECODER 0
#define CONFIG_ADPCM_IMA_QT_AT_DECODER 0
#define CONFIG_ALAC_AT_DECODER 0
#define CONFIG_AMR_NB_AT_DECODER 0
#define CONFIG_EAC3_AT_DECODER 0
#define CONFIG_GSM_MS_AT_DECODER 0
#define CONFIG_ILBC_AT_DECODER 0
#define CONFIG_MP1_AT_DECODER 0
#define CONFIG_MP2_AT_DECODER 0
#define CONFIG_MP3_AT_DECODER 0
#define CONFIG_PCM_ALAW_AT_DECODER 0
#define CONFIG_PCM_MULAW_AT_DECODER 0
#define CONFIG_QDMC_AT_DECODER 0
#define CONFIG_QDM2_AT_DECODER 0
#define CONFIG_LIBARIBB24_DECODER 0
#define CONFIG_LIBCELT_DECODER 0
#define CONFIG_LIBCODEC2_DECODER 0
#define CONFIG_LIBDAV1D_DECODER 1
#define CONFIG_LIBDAVS2_DECODER 0
#define CONFIG_LIBFDK_AAC_DECODER 0
#define CONFIG_LIBGSM_DECODER 1
#define CONFIG_LIBGSM_MS_DECODER 1
#define CONFIG_LIBILBC_DECODER 1
#define CONFIG_LIBOPENCORE_AMRNB_DECODER 0
#define CONFIG_LIBOPENCORE_AMRWB_DECODER 0
#define CONFIG_LIBOPENJPEG_DECODER 1
#define CONFIG_LIBOPUS_DECODER 1
#define CONFIG_LIBRSVG_DECODER 0
#define CONFIG_LIBSPEEX_DECODER 1
#define CONFIG_LIBVORBIS_DECODER 1
#define CONFIG_LIBVPX_VP8_DECODER 1
#define CONFIG_LIBVPX_VP9_DECODER 1
#define CONFIG_LIBZVBI_TELETEXT_DECODER 0
#define CONFIG_BINTEXT_DECODER 1
#define CONFIG_XBIN_DECODER 1
#define CONFIG_IDF_DECODER 1
#define CONFIG_LIBAOM_AV1_DECODER 1
#define CONFIG_AV1_DECODER 1
#define CONFIG_LIBOPENH264_DECODER 0
#define CONFIG_H264_CUVID_DECODER 0
#define CONFIG_HEVC_CUVID_DECODER 0
#define CONFIG_HEVC_MEDIACODEC_DECODER 0
#define CONFIG_MJPEG_CUVID_DECODER 0
#define CONFIG_MJPEG_QSV_DECODER 1
#define CONFIG_MPEG1_CUVID_DECODER 0
#define CONFIG_MPEG2_CUVID_DECODER 0
#define CONFIG_MPEG4_CUVID_DECODER 0
#define CONFIG_MPEG4_MEDIACODEC_DECODER 0
#define CONFIG_VC1_CUVID_DECODER 0
#define CONFIG_VP8_CUVID_DECODER 0
#define CONFIG_VP8_MEDIACODEC_DECODER 0
#define CONFIG_VP8_QSV_DECODER 1
#define CONFIG_VP9_CUVID_DECODER 0
#define CONFIG_VP9_MEDIACODEC_DECODER 0
#define CONFIG_VP9_QSV_DECODER 1
#define CONFIG_A64MULTI_ENCODER 1
#define CONFIG_A64MULTI5_ENCODER 1
#define CONFIG_ALIAS_PIX_ENCODER 1
#define CONFIG_AMV_ENCODER 1
#define CONFIG_APNG_ENCODER 1
#define CONFIG_ASV1_ENCODER 1
#define CONFIG_ASV2_ENCODER 1
#define CONFIG_AVRP_ENCODER 1
#define CONFIG_AVUI_ENCODER 1
#define CONFIG_AYUV_ENCODER 1
#define CONFIG_BMP_ENCODER 1
#define CONFIG_CFHD_ENCODER 1
#define CONFIG_CINEPAK_ENCODER 1
#define CONFIG_CLJR_ENCODER 1
#define CONFIG_COMFORTNOISE_ENCODER 1
#define CONFIG_DNXHD_ENCODER 1
#define CONFIG_DPX_ENCODER 1
#define CONFIG_DVVIDEO_ENCODER 1
#define CONFIG_FFV1_ENCODER 1
#define CONFIG_FFVHUFF_ENCODER 1
#define CONFIG_FITS_ENCODER 1
#define CONFIG_FLASHSV_ENCODER 1
#define CONFIG_FLASHSV2_ENCODER 1
#define CONFIG_FLV_ENCODER 1
#define CONFIG_GIF_ENCODER 1
#define CONFIG_H261_ENCODER 1
#define CONFIG_H263_ENCODER 1
#define CONFIG_H263P_ENCODER 1
#define CONFIG_HAP_ENCODER 1
#define CONFIG_HUFFYUV_ENCODER 1
#define CONFIG_JPEG2000_ENCODER 1
#define CONFIG_JPEGLS_ENCODER 1
#define CONFIG_LJPEG_ENCODER 1
#define CONFIG_MAGICYUV_ENCODER 1
#define CONFIG_MJPEG_ENCODER 1
#define CONFIG_MPEG1VIDEO_ENCODER 1
#define CONFIG_MPEG2VIDEO_ENCODER 1
#define CONFIG_MPEG4_ENCODER 1
#define CONFIG_MSMPEG4V2_ENCODER 1
#define CONFIG_MSMPEG4V3_ENCODER 1
#define CONFIG_MSVIDEO1_ENCODER 1
#define CONFIG_PAM_ENCODER 1
#define CONFIG_PBM_ENCODER 1
#define CONFIG_PCX_ENCODER 1
#define CONFIG_PGM_ENCODER 1
#define CONFIG_PGMYUV_ENCODER 1
#define CONFIG_PNG_ENCODER 1
#define CONFIG_PPM_ENCODER 1
#define CONFIG_PRORES_ENCODER 1
#define CONFIG_PRORES_AW_ENCODER 1
#define CONFIG_PRORES_KS_ENCODER 1
#define CONFIG_QTRLE_ENCODER 1
#define CONFIG_R10K_ENCODER 1
#define CONFIG_R210_ENCODER 1
#define CONFIG_RAWVIDEO_ENCODER 1
#define CONFIG_ROQ_ENCODER 1
#define CONFIG_RPZA_ENCODER 1
#define CONFIG_RV10_ENCODER 1
#define CONFIG_RV20_ENCODER 1
#define CONFIG_S302M_ENCODER 1
#define CONFIG_SGI_ENCODER 1
#define CONFIG_SNOW_ENCODER 1
#define CONFIG_SUNRAST_ENCODER 1
#define CONFIG_SVQ1_ENCODER 1
#define CONFIG_TARGA_ENCODER 1
#define CONFIG_TIFF_ENCODER 1
#define CONFIG_UTVIDEO_ENCODER 1
#define CONFIG_V210_ENCODER 1
#define CONFIG_V308_ENCODER 1
#define CONFIG_V408_ENCODER 1
#define CONFIG_V410_ENCODER 1
#define CONFIG_VC2_ENCODER 1
#define CONFIG_WRAPPED_AVFRAME_ENCODER 1
#define CONFIG_WMV1_ENCODER 1
#define CONFIG_WMV2_ENCODER 1
#define CONFIG_XBM_ENCODER 1
#define CONFIG_XFACE_ENCODER 1
#define CONFIG_XWD_ENCODER 1
#define CONFIG_Y41P_ENCODER 1
#define CONFIG_YUV4_ENCODER 1
#define CONFIG_ZLIB_ENCODER 1
#define CONFIG_ZMBV_ENCODER 1
#define CONFIG_AAC_ENCODER 1
#define CONFIG_AC3_ENCODER 1
#define CONFIG_AC3_FIXED_ENCODER 1
#define CONFIG_ALAC_ENCODER 1
#define CONFIG_APTX_ENCODER 1
#define CONFIG_APTX_HD_ENCODER 1
#define CONFIG_DCA_ENCODER 1
#define CONFIG_EAC3_ENCODER 1
#define CONFIG_FLAC_ENCODER 1
#define CONFIG_G723_1_ENCODER 1
#define CONFIG_MLP_ENCODER 1
#define CONFIG_MP2_ENCODER 1
#define CONFIG_MP2FIXED_ENCODER 1
#define CONFIG_NELLYMOSER_ENCODER 1
#define CONFIG_OPUS_ENCODER 1
#define CONFIG_RA_144_ENCODER 1
#define CONFIG_SBC_ENCODER 1
#define CONFIG_SONIC_ENCODER 1
#define CONFIG_SONIC_LS_ENCODER 1
#define CONFIG_TRUEHD_ENCODER 1
#define CONFIG_TTA_ENCODER 1
#define CONFIG_VORBIS_ENCODER 1
#define CONFIG_WAVPACK_ENCODER 1
#define CONFIG_WMAV1_ENCODER 1
#define CONFIG_WMAV2_ENCODER 1
#define CONFIG_PCM_ALAW_ENCODER 1
#define CONFIG_PCM_DVD_ENCODER 1
#define CONFIG_PCM_F32BE_ENCODER 1
#define CONFIG_PCM_F32LE_ENCODER 1
#define CONFIG_PCM_F64BE_ENCODER 1
#define CONFIG_PCM_F64LE_ENCODER 1
#define CONFIG_PCM_MULAW_ENCODER 1
#define CONFIG_PCM_S8_ENCODER 1
#define CONFIG_PCM_S8_PLANAR_ENCODER 1
#define CONFIG_PCM_S16BE_ENCODER 1
#define CONFIG_PCM_S16BE_PLANAR_ENCODER 1
#define CONFIG_PCM_S16LE_ENCODER 1
#define CONFIG_PCM_S16LE_PLANAR_ENCODER 1
#define CONFIG_PCM_S24BE_ENCODER 1
#define CONFIG_PCM_S24DAUD_ENCODER 1
#define CONFIG_PCM_S24LE_ENCODER 1
#define CONFIG_PCM_S24LE_PLANAR_ENCODER 1
#define CONFIG_PCM_S32BE_ENCODER 1
#define CONFIG_PCM_S32LE_ENCODER 1
#define CONFIG_PCM_S32LE_PLANAR_ENCODER 1
#define CONFIG_PCM_S64BE_ENCODER 1
#define CONFIG_PCM_S64LE_ENCODER 1
#define CONFIG_PCM_U8_ENCODER 1
#define CONFIG_PCM_U16BE_ENCODER 1
#define CONFIG_PCM_U16LE_ENCODER 1
#define CONFIG_PCM_U24BE_ENCODER 1
#define CONFIG_PCM_U24LE_ENCODER 1
#define CONFIG_PCM_U32BE_ENCODER 1
#define CONFIG_PCM_U32LE_ENCODER 1
#define CONFIG_PCM_VIDC_ENCODER 1
#define CONFIG_ROQ_DPCM_ENCODER 1
#define CONFIG_ADPCM_ADX_ENCODER 1
#define CONFIG_ADPCM_ARGO_ENCODER 1
#define CONFIG_ADPCM_G722_ENCODER 1
#define CONFIG_ADPCM_G726_ENCODER 1
#define CONFIG_ADPCM_G726LE_ENCODER 1
#define CONFIG_ADPCM_IMA_APM_ENCODER 1
#define CONFIG_ADPCM_IMA_QT_ENCODER 1
#define CONFIG_ADPCM_IMA_SSI_ENCODER 1
#define CONFIG_ADPCM_IMA_WAV_ENCODER 1
#define CONFIG_ADPCM_MS_ENCODER 1
#define CONFIG_ADPCM_SWF_ENCODER 1
#define CONFIG_ADPCM_YAMAHA_ENCODER 1
#define CONFIG_SSA_ENCODER 1
#define CONFIG_ASS_ENCODER 1
#define CONFIG_DVBSUB_ENCODER 1
#define CONFIG_DVDSUB_ENCODER 1
#define CONFIG_MOVTEXT_ENCODER 1
#define CONFIG_SRT_ENCODER 1
#define CONFIG_SUBRIP_ENCODER 1
#define CONFIG_TEXT_ENCODER 1
#define CONFIG_WEBVTT_ENCODER 1
#define CONFIG_XSUB_ENCODER 1
#define CONFIG_AAC_AT_ENCODER 0
#define CONFIG_ALAC_AT_ENCODER 0
#define CONFIG_ILBC_AT_ENCODER 0
#define CONFIG_PCM_ALAW_AT_ENCODER 0
#define CONFIG_PCM_MULAW_AT_ENCODER 0
#define CONFIG_LIBAOM_AV1_ENCODER 1
#define CONFIG_LIBCODEC2_ENCODER 0
#define CONFIG_LIBFDK_AAC_ENCODER 0
#define CONFIG_LIBGSM_ENCODER 1
#define CONFIG_LIBGSM_MS_ENCODER 1
#define CONFIG_LIBILBC_ENCODER 1
#define CONFIG_LIBMP3LAME_ENCODER 1
#define CONFIG_LIBOPENCORE_AMRNB_ENCODER 0
#define CONFIG_LIBOPENJPEG_ENCODER 1
#define CONFIG_LIBOPUS_ENCODER 1
#define CONFIG_LIBRAV1E_ENCODER 0
#define CONFIG_LIBSHINE_ENCODER 1
#define CONFIG_LIBSPEEX_ENCODER 1
#define CONFIG_LIBSVTAV1_ENCODER 0
#define CONFIG_LIBTHEORA_ENCODER 1
#define CONFIG_LIBTWOLAME_ENCODER 1
#define CONFIG_LIBVO_AMRWBENC_ENCODER 0
#define CONFIG_LIBVORBIS_ENCODER 1
#define CONFIG_LIBVPX_VP8_ENCODER 1
#define CONFIG_LIBVPX_VP9_ENCODER 1
#define CONFIG_LIBWAVPACK_ENCODER 1
#define CONFIG_LIBWEBP_ANIM_ENCODER 1
#define CONFIG_LIBWEBP_ENCODER 1
#define CONFIG_LIBX262_ENCODER 0
#define CONFIG_LIBX264_ENCODER 1
#define CONFIG_LIBX264RGB_ENCODER 1
#define CONFIG_LIBX265_ENCODER 1
#define CONFIG_LIBXAVS_ENCODER 0
#define CONFIG_LIBXAVS2_ENCODER 0
#define CONFIG_LIBXVID_ENCODER 1
#define CONFIG_AAC_MF_ENCODER 1
#define CONFIG_AC3_MF_ENCODER 1
#define CONFIG_H263_V4L2M2M_ENCODER 0
#define CONFIG_LIBOPENH264_ENCODER 0
#define CONFIG_H264_AMF_ENCODER 0
#define CONFIG_H264_MF_ENCODER 1
#define CONFIG_H264_NVENC_ENCODER 0
#define CONFIG_H264_OMX_ENCODER 0
#define CONFIG_H264_QSV_ENCODER 1
#define CONFIG_H264_V4L2M2M_ENCODER 0
#define CONFIG_H264_VAAPI_ENCODER 0
#define CONFIG_H264_VIDEOTOOLBOX_ENCODER 0
#define CONFIG_NVENC_ENCODER 0
#define CONFIG_NVENC_H264_ENCODER 0
#define CONFIG_NVENC_HEVC_ENCODER 0
#define CONFIG_HEVC_AMF_ENCODER 0
#define CONFIG_HEVC_MF_ENCODER 1
#define CONFIG_HEVC_NVENC_ENCODER 0
#define CONFIG_HEVC_QSV_ENCODER 1
#define CONFIG_HEVC_V4L2M2M_ENCODER 0
#define CONFIG_HEVC_VAAPI_ENCODER 0
#define CONFIG_HEVC_VIDEOTOOLBOX_ENCODER 0
#define CONFIG_LIBKVAZAAR_ENCODER 0
#define CONFIG_MJPEG_QSV_ENCODER 1
#define CONFIG_MJPEG_VAAPI_ENCODER 0
#define CONFIG_MP3_MF_ENCODER 1
#define CONFIG_MPEG2_QSV_ENCODER 1
#define CONFIG_MPEG2_VAAPI_ENCODER 0
#define CONFIG_MPEG4_OMX_ENCODER 0
#define CONFIG_MPEG4_V4L2M2M_ENCODER 0
#define CONFIG_VP8_V4L2M2M_ENCODER 0
#define CONFIG_VP8_VAAPI_ENCODER 0
#define CONFIG_VP9_VAAPI_ENCODER 0
#define CONFIG_VP9_QSV_ENCODER 1
#define CONFIG_H263_VAAPI_HWACCEL 0
#define CONFIG_H263_VIDEOTOOLBOX_HWACCEL 0
#define CONFIG_H264_D3D11VA_HWACCEL 1
#define CONFIG_H264_D3D11VA2_HWACCEL 1
#define CONFIG_H264_DXVA2_HWACCEL 1
#define CONFIG_H264_NVDEC_HWACCEL 0
#define CONFIG_H264_VAAPI_HWACCEL 0
#define CONFIG_H264_VDPAU_HWACCEL 0
#define CONFIG_H264_VIDEOTOOLBOX_HWACCEL 0
#define CONFIG_HEVC_D3D11VA_HWACCEL 1
#define CONFIG_HEVC_D3D11VA2_HWACCEL 1
#define CONFIG_HEVC_DXVA2_HWACCEL 1
#define CONFIG_HEVC_NVDEC_HWACCEL 0
#define CONFIG_HEVC_VAAPI_HWACCEL 0
#define CONFIG_HEVC_VDPAU_HWACCEL 0
#define CONFIG_HEVC_VIDEOTOOLBOX_HWACCEL 0
#define CONFIG_MJPEG_NVDEC_HWACCEL 0
#define CONFIG_MJPEG_VAAPI_HWACCEL 0
#define CONFIG_MPEG1_NVDEC_HWACCEL 0
#define CONFIG_MPEG1_VDPAU_HWACCEL 0
#define CONFIG_MPEG1_VIDEOTOOLBOX_HWACCEL 0
#define CONFIG_MPEG1_XVMC_HWACCEL 0
#define CONFIG_MPEG2_D3D11VA_HWACCEL 1
#define CONFIG_MPEG2_D3D11VA2_HWACCEL 1
#define CONFIG_MPEG2_NVDEC_HWACCEL 0
#define CONFIG_MPEG2_DXVA2_HWACCEL 1
#define CONFIG_MPEG2_VAAPI_HWACCEL 0
#define CONFIG_MPEG2_VDPAU_HWACCEL 0
#define CONFIG_MPEG2_VIDEOTOOLBOX_HWACCEL 0
#define CONFIG_MPEG2_XVMC_HWACCEL 0
#define CONFIG_MPEG4_NVDEC_HWACCEL 0
#define CONFIG_MPEG4_VAAPI_HWACCEL 0
#define CONFIG_MPEG4_VDPAU_HWACCEL 0
#define CONFIG_MPEG4_VIDEOTOOLBOX_HWACCEL 0
#define CONFIG_VC1_D3D11VA_HWACCEL 1
#define CONFIG_VC1_D3D11VA2_HWACCEL 1
#define CONFIG_VC1_DXVA2_HWACCEL 1
#define CONFIG_VC1_NVDEC_HWACCEL 0
#define CONFIG_VC1_VAAPI_HWACCEL 0
#define CONFIG_VC1_VDPAU_HWACCEL 0
#define CONFIG_VP8_NVDEC_HWACCEL 0
#define CONFIG_VP8_VAAPI_HWACCEL 0
#define CONFIG_VP9_D3D11VA_HWACCEL 1
#define CONFIG_VP9_D3D11VA2_HWACCEL 1
#define CONFIG_VP9_DXVA2_HWACCEL 1
#define CONFIG_VP9_NVDEC_HWACCEL 0
#define CONFIG_VP9_VAAPI_HWACCEL 0
#define CONFIG_VP9_VDPAU_HWACCEL 0
#define CONFIG_WMV3_D3D11VA_HWACCEL 1
#define CONFIG_WMV3_D3D11VA2_HWACCEL 1
#define CONFIG_WMV3_DXVA2_HWACCEL 1
#define CONFIG_WMV3_NVDEC_HWACCEL 0
#define CONFIG_WMV3_VAAPI_HWACCEL 0
#define CONFIG_WMV3_VDPAU_HWACCEL 0
#define CONFIG_AAC_PARSER 1
#define CONFIG_AAC_LATM_PARSER 1
#define CONFIG_AC3_PARSER 1
#define CONFIG_ADX_PARSER 1
#define CONFIG_AV1_PARSER 1
#define CONFIG_AVS2_PARSER 1
#define CONFIG_BMP_PARSER 1
#define CONFIG_CAVSVIDEO_PARSER 1
#define CONFIG_COOK_PARSER 1
#define CONFIG_DCA_PARSER 1
#define CONFIG_DIRAC_PARSER 1
#define CONFIG_DNXHD_PARSER 1
#define CONFIG_DPX_PARSER 1
#define CONFIG_DVAUDIO_PARSER 1
#define CONFIG_DVBSUB_PARSER 1
#define CONFIG_DVDSUB_PARSER 1
#define CONFIG_DVD_NAV_PARSER 1
#define CONFIG_FLAC_PARSER 1
#define CONFIG_G723_1_PARSER 1
#define CONFIG_G729_PARSER 1
#define CONFIG_GIF_PARSER 1
#define CONFIG_GSM_PARSER 1
#define CONFIG_H261_PARSER 1
#define CONFIG_H263_PARSER 1
#define CONFIG_H264_PARSER 1
#define CONFIG_HEVC_PARSER 1
#define CONFIG_IPU_PARSER 1
#define CONFIG_JPEG2000_PARSER 1
#define CONFIG_MJPEG_PARSER 1
#define CONFIG_MLP_PARSER 1
#define CONFIG_MPEG4VIDEO_PARSER 1
#define CONFIG_MPEGAUDIO_PARSER 1
#define CONFIG_MPEGVIDEO_PARSER 1
#define CONFIG_OPUS_PARSER 1
#define CONFIG_PNG_PARSER 1
#define CONFIG_PNM_PARSER 1
#define CONFIG_RV30_PARSER 1
#define CONFIG_RV40_PARSER 1
#define CONFIG_SBC_PARSER 1
#define CONFIG_SIPR_PARSER 1
#define CONFIG_TAK_PARSER 1
#define CONFIG_VC1_PARSER 1
#define CONFIG_VORBIS_PARSER 1
#define CONFIG_VP3_PARSER 1
#define CONFIG_VP8_PARSER 1
#define CONFIG_VP9_PARSER 1
#define CONFIG_WEBP_PARSER 1
#define CONFIG_XMA_PARSER 1
#define CONFIG_ALSA_INDEV 0
#define CONFIG_ANDROID_CAMERA_INDEV 0
#define CONFIG_AVFOUNDATION_INDEV 0
#define CONFIG_BKTR_INDEV 0
#define CONFIG_DECKLINK_INDEV 0
#define CONFIG_DSHOW_INDEV 1
#define CONFIG_FBDEV_INDEV 0
#define CONFIG_GDIGRAB_INDEV 1
#define CONFIG_IEC61883_INDEV 0
#define CONFIG_JACK_INDEV 0
#define CONFIG_KMSGRAB_INDEV 0
#define CONFIG_LAVFI_INDEV 1
#define CONFIG_OPENAL_INDEV 0
#define CONFIG_OSS_INDEV 0
#define CONFIG_PULSE_INDEV 0
#define CONFIG_SNDIO_INDEV 0
#define CONFIG_V4L2_INDEV 0
#define CONFIG_VFWCAP_INDEV 1
#define CONFIG_XCBGRAB_INDEV 0
#define CONFIG_LIBCDIO_INDEV 1
#define CONFIG_LIBDC1394_INDEV 0
#define CONFIG_ALSA_OUTDEV 0
#define CONFIG_AUDIOTOOLBOX_OUTDEV 0
#define CONFIG_CACA_OUTDEV 1
#define CONFIG_DECKLINK_OUTDEV 0
#define CONFIG_FBDEV_OUTDEV 0
#define CONFIG_OPENGL_OUTDEV 0
#define CONFIG_OSS_OUTDEV 0
#define CONFIG_PULSE_OUTDEV 0
#define CONFIG_SDL2_OUTDEV 1
#define CONFIG_SNDIO_OUTDEV 0
#define CONFIG_V4L2_OUTDEV 0
#define CONFIG_XV_OUTDEV 0
#define CONFIG_ABENCH_FILTER 1
#define CONFIG_ACOMPRESSOR_FILTER 1
#define CONFIG_ACONTRAST_FILTER 1
#define CONFIG_ACOPY_FILTER 1
#define CONFIG_ACUE_FILTER 1
#define CONFIG_ACROSSFADE_FILTER 1
#define CONFIG_ACROSSOVER_FILTER 1
#define CONFIG_ACRUSHER_FILTER 1
#define CONFIG_ADECLICK_FILTER 1
#define CONFIG_ADECLIP_FILTER 1
#define CONFIG_ADELAY_FILTER 1
#define CONFIG_ADERIVATIVE_FILTER 1
#define CONFIG_AECHO_FILTER 1
#define CONFIG_AEMPHASIS_FILTER 1
#define CONFIG_AEVAL_FILTER 1
#define CONFIG_AFADE_FILTER 1
#define CONFIG_AFFTDN_FILTER 1
#define CONFIG_AFFTFILT_FILTER 1
#define CONFIG_AFIR_FILTER 1
#define CONFIG_AFORMAT_FILTER 1
#define CONFIG_AGATE_FILTER 1
#define CONFIG_AIIR_FILTER 1
#define CONFIG_AINTEGRAL_FILTER 1
#define CONFIG_AINTERLEAVE_FILTER 1
#define CONFIG_ALIMITER_FILTER 1
#define CONFIG_ALLPASS_FILTER 1
#define CONFIG_ALOOP_FILTER 1
#define CONFIG_AMERGE_FILTER 1
#define CONFIG_AMETADATA_FILTER 1
#define CONFIG_AMIX_FILTER 1
#define CONFIG_AMULTIPLY_FILTER 1
#define CONFIG_ANEQUALIZER_FILTER 1
#define CONFIG_ANLMDN_FILTER 1
#define CONFIG_ANLMS_FILTER 1
#define CONFIG_ANULL_FILTER 1
#define CONFIG_APAD_FILTER 1
#define CONFIG_APERMS_FILTER 1
#define CONFIG_APHASER_FILTER 1
#define CONFIG_APULSATOR_FILTER 1
#define CONFIG_AREALTIME_FILTER 1
#define CONFIG_ARESAMPLE_FILTER 1
#define CONFIG_AREVERSE_FILTER 1
#define CONFIG_ARNNDN_FILTER 1
#define CONFIG_ASELECT_FILTER 1
#define CONFIG_ASENDCMD_FILTER 1
#define CONFIG_ASETNSAMPLES_FILTER 1
#define CONFIG_ASETPTS_FILTER 1
#define CONFIG_ASETRATE_FILTER 1
#define CONFIG_ASETTB_FILTER 1
#define CONFIG_ASHOWINFO_FILTER 1
#define CONFIG_ASIDEDATA_FILTER 1
#define CONFIG_ASOFTCLIP_FILTER 1
#define CONFIG_ASPLIT_FILTER 1
#define CONFIG_ASR_FILTER 0
#define CONFIG_ASTATS_FILTER 1
#define CONFIG_ASTREAMSELECT_FILTER 1
#define CONFIG_ASUBBOOST_FILTER 1
#define CONFIG_ATEMPO_FILTER 1
#define CONFIG_ATRIM_FILTER 1
#define CONFIG_AXCORRELATE_FILTER 1
#define CONFIG_AZMQ_FILTER 1
#define CONFIG_BANDPASS_FILTER 1
#define CONFIG_BANDREJECT_FILTER 1
#define CONFIG_BASS_FILTER 1
#define CONFIG_BIQUAD_FILTER 1
#define CONFIG_BS2B_FILTER 1
#define CONFIG_CHROMABER_VULKAN_FILTER 0
#define CONFIG_CHANNELMAP_FILTER 1
#define CONFIG_CHANNELSPLIT_FILTER 1
#define CONFIG_CHORUS_FILTER 1
#define CONFIG_COMPAND_FILTER 1
#define CONFIG_COMPENSATIONDELAY_FILTER 1
#define CONFIG_CROSSFEED_FILTER 1
#define CONFIG_CRYSTALIZER_FILTER 1
#define CONFIG_DCSHIFT_FILTER 1
#define CONFIG_DEESSER_FILTER 1
#define CONFIG_DRMETER_FILTER 1
#define CONFIG_DYNAUDNORM_FILTER 1
#define CONFIG_EARWAX_FILTER 1
#define CONFIG_EBUR128_FILTER 1
#define CONFIG_EQUALIZER_FILTER 1
#define CONFIG_EXTRASTEREO_FILTER 1
#define CONFIG_FIREQUALIZER_FILTER 1
#define CONFIG_FLANGER_FILTER 1
#define CONFIG_HAAS_FILTER 1
#define CONFIG_HDCD_FILTER 1
#define CONFIG_HEADPHONE_FILTER 1
#define CONFIG_HIGHPASS_FILTER 1
#define CONFIG_HIGHSHELF_FILTER 1
#define CONFIG_JOIN_FILTER 1
#define CONFIG_LADSPA_FILTER 0
#define CONFIG_LOUDNORM_FILTER 1
#define CONFIG_LOWPASS_FILTER 1
#define CONFIG_LOWSHELF_FILTER 1
#define CONFIG_LV2_FILTER 0
#define CONFIG_MCOMPAND_FILTER 1
#define CONFIG_PAN_FILTER 1
#define CONFIG_REPLAYGAIN_FILTER 1
#define CONFIG_RESAMPLE_FILTER 0
#define CONFIG_RUBBERBAND_FILTER 1
#define CONFIG_SIDECHAINCOMPRESS_FILTER 1
#define CONFIG_SIDECHAINGATE_FILTER 1
#define CONFIG_SILENCEDETECT_FILTER 1
#define CONFIG_SILENCEREMOVE_FILTER 1
#define CONFIG_SOFALIZER_FILTER 1
#define CONFIG_STEREOTOOLS_FILTER 1
#define CONFIG_STEREOWIDEN_FILTER 1
#define CONFIG_SUPEREQUALIZER_FILTER 1
#define CONFIG_SURROUND_FILTER 1
#define CONFIG_TREBLE_FILTER 1
#define CONFIG_TREMOLO_FILTER 1
#define CONFIG_VIBRATO_FILTER 1
#define CONFIG_VOLUME_FILTER 1
#define CONFIG_VOLUMEDETECT_FILTER 1
#define CONFIG_AEVALSRC_FILTER 1
#define CONFIG_AFIRSRC_FILTER 1
#define CONFIG_ANOISESRC_FILTER 1
#define CONFIG_ANULLSRC_FILTER 1
#define CONFIG_FLITE_FILTER 0
#define CONFIG_HILBERT_FILTER 1
#define CONFIG_SINC_FILTER 1
#define CONFIG_SINE_FILTER 1
#define CONFIG_ANULLSINK_FILTER 1
#define CONFIG_ADDROI_FILTER 1
#define CONFIG_ALPHAEXTRACT_FILTER 1
#define CONFIG_ALPHAMERGE_FILTER 1
#define CONFIG_AMPLIFY_FILTER 1
#define CONFIG_ASS_FILTER 1
#define CONFIG_ATADENOISE_FILTER 1
#define CONFIG_AVGBLUR_FILTER 1
#define CONFIG_AVGBLUR_OPENCL_FILTER 0
#define CONFIG_AVGBLUR_VULKAN_FILTER 0
#define CONFIG_BBOX_FILTER 1
#define CONFIG_BENCH_FILTER 1
#define CONFIG_BILATERAL_FILTER 1
#define CONFIG_BITPLANENOISE_FILTER 1
#define CONFIG_BLACKDETECT_FILTER 1
#define CONFIG_BLACKFRAME_FILTER 1
#define CONFIG_BLEND_FILTER 1
#define CONFIG_BM3D_FILTER 1
#define CONFIG_BOXBLUR_FILTER 1
#define CONFIG_BOXBLUR_OPENCL_FILTER 0
#define CONFIG_BWDIF_FILTER 1
#define CONFIG_CAS_FILTER 1
#define CONFIG_CHROMAHOLD_FILTER 1
#define CONFIG_CHROMAKEY_FILTER 1
#define CONFIG_CHROMANR_FILTER 1
#define CONFIG_CHROMASHIFT_FILTER 1
#define CONFIG_CIESCOPE_FILTER 1
#define CONFIG_CODECVIEW_FILTER 1
#define CONFIG_COLORBALANCE_FILTER 1
#define CONFIG_COLORCHANNELMIXER_FILTER 1
#define CONFIG_COLORKEY_FILTER 1
#define CONFIG_COLORKEY_OPENCL_FILTER 0
#define CONFIG_COLORHOLD_FILTER 1
#define CONFIG_COLORLEVELS_FILTER 1
#define CONFIG_COLORMATRIX_FILTER 1
#define CONFIG_COLORSPACE_FILTER 1
#define CONFIG_CONVOLUTION_FILTER 1
#define CONFIG_CONVOLUTION_OPENCL_FILTER 0
#define CONFIG_CONVOLVE_FILTER 1
#define CONFIG_COPY_FILTER 1
#define CONFIG_COREIMAGE_FILTER 0
#define CONFIG_COVER_RECT_FILTER 1
#define CONFIG_CROP_FILTER 1
#define CONFIG_CROPDETECT_FILTER 1
#define CONFIG_CUE_FILTER 1
#define CONFIG_CURVES_FILTER 1
#define CONFIG_DATASCOPE_FILTER 1
#define CONFIG_DBLUR_FILTER 1
#define CONFIG_DCTDNOIZ_FILTER 1
#define CONFIG_DEBAND_FILTER 1
#define CONFIG_DEBLOCK_FILTER 1
#define CONFIG_DECIMATE_FILTER 1
#define CONFIG_DECONVOLVE_FILTER 1
#define CONFIG_DEDOT_FILTER 1
#define CONFIG_DEFLATE_FILTER 1
#define CONFIG_DEFLICKER_FILTER 1
#define CONFIG_DEINTERLACE_QSV_FILTER 1
#define CONFIG_DEINTERLACE_VAAPI_FILTER 0
#define CONFIG_DEJUDDER_FILTER 1
#define CONFIG_DELOGO_FILTER 1
#define CONFIG_DENOISE_VAAPI_FILTER 0
#define CONFIG_DERAIN_FILTER 1
#define CONFIG_DESHAKE_FILTER 1
#define CONFIG_DESHAKE_OPENCL_FILTER 0
#define CONFIG_DESPILL_FILTER 1
#define CONFIG_DETELECINE_FILTER 1
#define CONFIG_DILATION_FILTER 1
#define CONFIG_DILATION_OPENCL_FILTER 0
#define CONFIG_DISPLACE_FILTER 1
#define CONFIG_DNN_PROCESSING_FILTER 1
#define CONFIG_DOUBLEWEAVE_FILTER 1
#define CONFIG_DRAWBOX_FILTER 1
#define CONFIG_DRAWGRAPH_FILTER 1
#define CONFIG_DRAWGRID_FILTER 1
#define CONFIG_DRAWTEXT_FILTER 1
#define CONFIG_EDGEDETECT_FILTER 1
#define CONFIG_ELBG_FILTER 1
#define CONFIG_ENTROPY_FILTER 1
#define CONFIG_EQ_FILTER 1
#define CONFIG_EROSION_FILTER 1
#define CONFIG_EROSION_OPENCL_FILTER 0
#define CONFIG_EXTRACTPLANES_FILTER 1
#define CONFIG_FADE_FILTER 1
#define CONFIG_FFTDNOIZ_FILTER 1
#define CONFIG_FFTFILT_FILTER 1
#define CONFIG_FIELD_FILTER 1
#define CONFIG_FIELDHINT_FILTER 1
#define CONFIG_FIELDMATCH_FILTER 1
#define CONFIG_FIELDORDER_FILTER 1
#define CONFIG_FILLBORDERS_FILTER 1
#define CONFIG_FIND_RECT_FILTER 1
#define CONFIG_FLOODFILL_FILTER 1
#define CONFIG_FORMAT_FILTER 1
#define CONFIG_FPS_FILTER 1
#define CONFIG_FRAMEPACK_FILTER 1
#define CONFIG_FRAMERATE_FILTER 1
#define CONFIG_FRAMESTEP_FILTER 1
#define CONFIG_FREEZEDETECT_FILTER 1
#define CONFIG_FREEZEFRAMES_FILTER 1
#define CONFIG_FREI0R_FILTER 0
#define CONFIG_FSPP_FILTER 1
#define CONFIG_GBLUR_FILTER 1
#define CONFIG_GEQ_FILTER 1
#define CONFIG_GRADFUN_FILTER 1
#define CONFIG_GRAPHMONITOR_FILTER 1
#define CONFIG_GREYEDGE_FILTER 1
#define CONFIG_HALDCLUT_FILTER 1
#define CONFIG_HFLIP_FILTER 1
#define CONFIG_HISTEQ_FILTER 1
#define CONFIG_HISTOGRAM_FILTER 1
#define CONFIG_HQDN3D_FILTER 1
#define CONFIG_HQX_FILTER 1
#define CONFIG_HSTACK_FILTER 1
#define CONFIG_HUE_FILTER 1
#define CONFIG_HWDOWNLOAD_FILTER 1
#define CONFIG_HWMAP_FILTER 1
#define CONFIG_HWUPLOAD_FILTER 1
#define CONFIG_HWUPLOAD_CUDA_FILTER 0
#define CONFIG_HYSTERESIS_FILTER 1
#define CONFIG_IDET_FILTER 1
#define CONFIG_IL_FILTER 1
#define CONFIG_INFLATE_FILTER 1
#define CONFIG_INTERLACE_FILTER 1
#define CONFIG_INTERLEAVE_FILTER 1
#define CONFIG_KERNDEINT_FILTER 1
#define CONFIG_LAGFUN_FILTER 1
#define CONFIG_LENSCORRECTION_FILTER 1
#define CONFIG_LENSFUN_FILTER 0
#define CONFIG_LIBVMAF_FILTER 0
#define CONFIG_LIMITER_FILTER 1
#define CONFIG_LOOP_FILTER 1
#define CONFIG_LUMAKEY_FILTER 1
#define CONFIG_LUT_FILTER 1
#define CONFIG_LUT1D_FILTER 1
#define CONFIG_LUT2_FILTER 1
#define CONFIG_LUT3D_FILTER 1
#define CONFIG_LUTRGB_FILTER 1
#define CONFIG_LUTYUV_FILTER 1
#define CONFIG_MASKEDCLAMP_FILTER 1
#define CONFIG_MASKEDMAX_FILTER 1
#define CONFIG_MASKEDMERGE_FILTER 1
#define CONFIG_MASKEDMIN_FILTER 1
#define CONFIG_MASKEDTHRESHOLD_FILTER 1
#define CONFIG_MASKFUN_FILTER 1
#define CONFIG_MCDEINT_FILTER 1
#define CONFIG_MEDIAN_FILTER 1
#define CONFIG_MERGEPLANES_FILTER 1
#define CONFIG_MESTIMATE_FILTER 1
#define CONFIG_METADATA_FILTER 1
#define CONFIG_MIDEQUALIZER_FILTER 1
#define CONFIG_MINTERPOLATE_FILTER 1
#define CONFIG_MIX_FILTER 1
#define CONFIG_MPDECIMATE_FILTER 1
#define CONFIG_NEGATE_FILTER 1
#define CONFIG_NLMEANS_FILTER 1
#define CONFIG_NLMEANS_OPENCL_FILTER 0
#define CONFIG_NNEDI_FILTER 1
#define CONFIG_NOFORMAT_FILTER 1
#define CONFIG_NOISE_FILTER 1
#define CONFIG_NORMALIZE_FILTER 1
#define CONFIG_NULL_FILTER 1
#define CONFIG_OCR_FILTER 0
#define CONFIG_OCV_FILTER 0
#define CONFIG_OSCILLOSCOPE_FILTER 1
#define CONFIG_OVERLAY_FILTER 1
#define CONFIG_OVERLAY_OPENCL_FILTER 0
#define CONFIG_OVERLAY_QSV_FILTER 1
#define CONFIG_OVERLAY_VULKAN_FILTER 0
#define CONFIG_OVERLAY_CUDA_FILTER 0
#define CONFIG_OWDENOISE_FILTER 1
#define CONFIG_PAD_FILTER 1
#define CONFIG_PAD_OPENCL_FILTER 0
#define CONFIG_PALETTEGEN_FILTER 1
#define CONFIG_PALETTEUSE_FILTER 1
#define CONFIG_PERMS_FILTER 1
#define CONFIG_PERSPECTIVE_FILTER 1
#define CONFIG_PHASE_FILTER 1
#define CONFIG_PHOTOSENSITIVITY_FILTER 1
#define CONFIG_PIXDESCTEST_FILTER 1
#define CONFIG_PIXSCOPE_FILTER 1
#define CONFIG_PP_FILTER 1
#define CONFIG_PP7_FILTER 1
#define CONFIG_PREMULTIPLY_FILTER 1
#define CONFIG_PREWITT_FILTER 1
#define CONFIG_PREWITT_OPENCL_FILTER 0
#define CONFIG_PROCAMP_VAAPI_FILTER 0
#define CONFIG_PROGRAM_OPENCL_FILTER 0
#define CONFIG_PSEUDOCOLOR_FILTER 1
#define CONFIG_PSNR_FILTER 1
#define CONFIG_PULLUP_FILTER 1
#define CONFIG_QP_FILTER 1
#define CONFIG_RANDOM_FILTER 1
#define CONFIG_READEIA608_FILTER 1
#define CONFIG_READVITC_FILTER 1
#define CONFIG_REALTIME_FILTER 1
#define CONFIG_REMAP_FILTER 1
#define CONFIG_REMOVEGRAIN_FILTER 1
#define CONFIG_REMOVELOGO_FILTER 1
#define CONFIG_REPEATFIELDS_FILTER 1
#define CONFIG_REVERSE_FILTER 1
#define CONFIG_RGBASHIFT_FILTER 1
#define CONFIG_ROBERTS_FILTER 1
#define CONFIG_ROBERTS_OPENCL_FILTER 0
#define CONFIG_ROTATE_FILTER 1
#define CONFIG_SAB_FILTER 1
#define CONFIG_SCALE_FILTER 1
#define CONFIG_SCALE_CUDA_FILTER 0
#define CONFIG_SCALE_NPP_FILTER 0
#define CONFIG_SCALE_QSV_FILTER 1
#define CONFIG_SCALE_VAAPI_FILTER 0
#define CONFIG_SCALE_VULKAN_FILTER 0
#define CONFIG_SCALE2REF_FILTER 1
#define CONFIG_SCDET_FILTER 1
#define CONFIG_SCROLL_FILTER 1
#define CONFIG_SELECT_FILTER 1
#define CONFIG_SELECTIVECOLOR_FILTER 1
#define CONFIG_SENDCMD_FILTER 1
#define CONFIG_SEPARATEFIELDS_FILTER 1
#define CONFIG_SETDAR_FILTER 1
#define CONFIG_SETFIELD_FILTER 1
#define CONFIG_SETPARAMS_FILTER 1
#define CONFIG_SETPTS_FILTER 1
#define CONFIG_SETRANGE_FILTER 1
#define CONFIG_SETSAR_FILTER 1
#define CONFIG_SETTB_FILTER 1
#define CONFIG_SHARPNESS_VAAPI_FILTER 0
#define CONFIG_SHOWINFO_FILTER 1
#define CONFIG_SHOWPALETTE_FILTER 1
#define CONFIG_SHUFFLEFRAMES_FILTER 1
#define CONFIG_SHUFFLEPLANES_FILTER 1
#define CONFIG_SIDEDATA_FILTER 1
#define CONFIG_SIGNALSTATS_FILTER 1
#define CONFIG_SIGNATURE_FILTER 1
#define CONFIG_SMARTBLUR_FILTER 1
#define CONFIG_SOBEL_FILTER 1
#define CONFIG_SOBEL_OPENCL_FILTER 0
#define CONFIG_SPLIT_FILTER 1
#define CONFIG_SPP_FILTER 1
#define CONFIG_SR_FILTER 1
#define CONFIG_SSIM_FILTER 1
#define CONFIG_STEREO3D_FILTER 1
#define CONFIG_STREAMSELECT_FILTER 1
#define CONFIG_SUBTITLES_FILTER 1
#define CONFIG_SUPER2XSAI_FILTER 1
#define CONFIG_SWAPRECT_FILTER 1
#define CONFIG_SWAPUV_FILTER 1
#define CONFIG_TBLEND_FILTER 1
#define CONFIG_TELECINE_FILTER 1
#define CONFIG_THISTOGRAM_FILTER 1
#define CONFIG_THRESHOLD_FILTER 1
#define CONFIG_THUMBNAIL_FILTER 1
#define CONFIG_THUMBNAIL_CUDA_FILTER 0
#define CONFIG_TILE_FILTER 1
#define CONFIG_TINTERLACE_FILTER 1
#define CONFIG_TLUT2_FILTER 1
#define CONFIG_TMEDIAN_FILTER 1
#define CONFIG_TMIX_FILTER 1
#define CONFIG_TONEMAP_FILTER 1
#define CONFIG_TONEMAP_OPENCL_FILTER 0
#define CONFIG_TONEMAP_VAAPI_FILTER 0
#define CONFIG_TPAD_FILTER 1
#define CONFIG_TRANSPOSE_FILTER 1
#define CONFIG_TRANSPOSE_NPP_FILTER 0
#define CONFIG_TRANSPOSE_OPENCL_FILTER 0
#define CONFIG_TRANSPOSE_VAAPI_FILTER 0
#define CONFIG_TRIM_FILTER 1
#define CONFIG_UNPREMULTIPLY_FILTER 1
#define CONFIG_UNSHARP_FILTER 1
#define CONFIG_UNSHARP_OPENCL_FILTER 0
#define CONFIG_UNTILE_FILTER 1
#define CONFIG_USPP_FILTER 1
#define CONFIG_V360_FILTER 1
#define CONFIG_VAGUEDENOISER_FILTER 1
#define CONFIG_VECTORSCOPE_FILTER 1
#define CONFIG_VFLIP_FILTER 1
#define CONFIG_VFRDET_FILTER 1
#define CONFIG_VIBRANCE_FILTER 1
#define CONFIG_VIDSTABDETECT_FILTER 1
#define CONFIG_VIDSTABTRANSFORM_FILTER 1
#define CONFIG_VIGNETTE_FILTER 1
#define CONFIG_VMAFMOTION_FILTER 1
#define CONFIG_VPP_QSV_FILTER 1
#define CONFIG_VSTACK_FILTER 1
#define CONFIG_W3FDIF_FILTER 1
#define CONFIG_WAVEFORM_FILTER 1
#define CONFIG_WEAVE_FILTER 1
#define CONFIG_XBR_FILTER 1
#define CONFIG_XFADE_FILTER 1
#define CONFIG_XFADE_OPENCL_FILTER 0
#define CONFIG_XMEDIAN_FILTER 1
#define CONFIG_XSTACK_FILTER 1
#define CONFIG_YADIF_FILTER 1
#define CONFIG_YADIF_CUDA_FILTER 0
#define CONFIG_YAEPBLUR_FILTER 1
#define CONFIG_ZMQ_FILTER 1
#define CONFIG_ZOOMPAN_FILTER 1
#define CONFIG_ZSCALE_FILTER 1
#define CONFIG_ALLRGB_FILTER 1
#define CONFIG_ALLYUV_FILTER 1
#define CONFIG_CELLAUTO_FILTER 1
#define CONFIG_COLOR_FILTER 1
#define CONFIG_COREIMAGESRC_FILTER 0
#define CONFIG_FREI0R_SRC_FILTER 0
#define CONFIG_GRADIENTS_FILTER 1
#define CONFIG_HALDCLUTSRC_FILTER 1
#define CONFIG_LIFE_FILTER 1
#define CONFIG_MANDELBROT_FILTER 1
#define CONFIG_MPTESTSRC_FILTER 1
#define CONFIG_NULLSRC_FILTER 1
#define CONFIG_OPENCLSRC_FILTER 0
#define CONFIG_PAL75BARS_FILTER 1
#define CONFIG_PAL100BARS_FILTER 1
#define CONFIG_RGBTESTSRC_FILTER 1
#define CONFIG_SIERPINSKI_FILTER 1
#define CONFIG_SMPTEBARS_FILTER 1
#define CONFIG_SMPTEHDBARS_FILTER 1
#define CONFIG_TESTSRC_FILTER 1
#define CONFIG_TESTSRC2_FILTER 1
#define CONFIG_YUVTESTSRC_FILTER 1
#define CONFIG_NULLSINK_FILTER 1
#define CONFIG_ABITSCOPE_FILTER 1
#define CONFIG_ADRAWGRAPH_FILTER 1
#define CONFIG_AGRAPHMONITOR_FILTER 1
#define CONFIG_AHISTOGRAM_FILTER 1
#define CONFIG_APHASEMETER_FILTER 1
#define CONFIG_AVECTORSCOPE_FILTER 1
#define CONFIG_CONCAT_FILTER 1
#define CONFIG_SHOWCQT_FILTER 1
#define CONFIG_SHOWFREQS_FILTER 1
#define CONFIG_SHOWSPATIAL_FILTER 1
#define CONFIG_SHOWSPECTRUM_FILTER 1
#define CONFIG_SHOWSPECTRUMPIC_FILTER 1
#define CONFIG_SHOWVOLUME_FILTER 1
#define CONFIG_SHOWWAVES_FILTER 1
#define CONFIG_SHOWWAVESPIC_FILTER 1
#define CONFIG_SPECTRUMSYNTH_FILTER 1
#define CONFIG_AMOVIE_FILTER 1
#define CONFIG_MOVIE_FILTER 1
#define CONFIG_AFIFO_FILTER 1
#define CONFIG_FIFO_FILTER 1
#define CONFIG_AA_DEMUXER 1
#define CONFIG_AAC_DEMUXER 1
#define CONFIG_AAX_DEMUXER 1
#define CONFIG_AC3_DEMUXER 1
#define CONFIG_ACM_DEMUXER 1
#define CONFIG_ACT_DEMUXER 1
#define CONFIG_ADF_DEMUXER 1
#define CONFIG_ADP_DEMUXER 1
#define CONFIG_ADS_DEMUXER 1
#define CONFIG_ADX_DEMUXER 1
#define CONFIG_AEA_DEMUXER 1
#define CONFIG_AFC_DEMUXER 1
#define CONFIG_AIFF_DEMUXER 1
#define CONFIG_AIX_DEMUXER 1
#define CONFIG_ALP_DEMUXER 1
#define CONFIG_AMR_DEMUXER 1
#define CONFIG_AMRNB_DEMUXER 1
#define CONFIG_AMRWB_DEMUXER 1
#define CONFIG_ANM_DEMUXER 1
#define CONFIG_APC_DEMUXER 1
#define CONFIG_APE_DEMUXER 1
#define CONFIG_APM_DEMUXER 1
#define CONFIG_APNG_DEMUXER 1
#define CONFIG_APTX_DEMUXER 1
#define CONFIG_APTX_HD_DEMUXER 1
#define CONFIG_AQTITLE_DEMUXER 1
#define CONFIG_ARGO_ASF_DEMUXER 1
#define CONFIG_ARGO_BRP_DEMUXER 1
#define CONFIG_ASF_DEMUXER 1
#define CONFIG_ASF_O_DEMUXER 1
#define CONFIG_ASS_DEMUXER 1
#define CONFIG_AST_DEMUXER 1
#define CONFIG_AU_DEMUXER 1
#define CONFIG_AV1_DEMUXER 1
#define CONFIG_AVI_DEMUXER 1
#define CONFIG_AVISYNTH_DEMUXER 0
#define CONFIG_AVR_DEMUXER 1
#define CONFIG_AVS_DEMUXER 1
#define CONFIG_AVS2_DEMUXER 1
#define CONFIG_BETHSOFTVID_DEMUXER 1
#define CONFIG_BFI_DEMUXER 1
#define CONFIG_BINTEXT_DEMUXER 1
#define CONFIG_BINK_DEMUXER 1
#define CONFIG_BIT_DEMUXER 1
#define CONFIG_BMV_DEMUXER 1
#define CONFIG_BFSTM_DEMUXER 1
#define CONFIG_BRSTM_DEMUXER 1
#define CONFIG_BOA_DEMUXER 1
#define CONFIG_C93_DEMUXER 1
#define CONFIG_CAF_DEMUXER 1
#define CONFIG_CAVSVIDEO_DEMUXER 1
#define CONFIG_CDG_DEMUXER 1
#define CONFIG_CDXL_DEMUXER 1
#define CONFIG_CINE_DEMUXER 1
#define CONFIG_CODEC2_DEMUXER 1
#define CONFIG_CODEC2RAW_DEMUXER 1
#define CONFIG_CONCAT_DEMUXER 1
#define CONFIG_DASH_DEMUXER 1
#define CONFIG_DATA_DEMUXER 1
#define CONFIG_DAUD_DEMUXER 1
#define CONFIG_DCSTR_DEMUXER 1
#define CONFIG_DERF_DEMUXER 1
#define CONFIG_DFA_DEMUXER 1
#define CONFIG_DHAV_DEMUXER 1
#define CONFIG_DIRAC_DEMUXER 1
#define CONFIG_DNXHD_DEMUXER 1
#define CONFIG_DSF_DEMUXER 1
#define CONFIG_DSICIN_DEMUXER 1
#define CONFIG_DSS_DEMUXER 1
#define CONFIG_DTS_DEMUXER 1
#define CONFIG_DTSHD_DEMUXER 1
#define CONFIG_DV_DEMUXER 1
#define CONFIG_DVBSUB_DEMUXER 1
#define CONFIG_DVBTXT_DEMUXER 1
#define CONFIG_DXA_DEMUXER 1
#define CONFIG_EA_DEMUXER 1
#define CONFIG_EA_CDATA_DEMUXER 1
#define CONFIG_EAC3_DEMUXER 1
#define CONFIG_EPAF_DEMUXER 1
#define CONFIG_FFMETADATA_DEMUXER 1
#define CONFIG_FILMSTRIP_DEMUXER 1
#define CONFIG_FITS_DEMUXER 1
#define CONFIG_FLAC_DEMUXER 1
#define CONFIG_FLIC_DEMUXER 1
#define CONFIG_FLV_DEMUXER 1
#define CONFIG_LIVE_FLV_DEMUXER 1
#define CONFIG_FOURXM_DEMUXER 1
#define CONFIG_FRM_DEMUXER 1
#define CONFIG_FSB_DEMUXER 1
#define CONFIG_FWSE_DEMUXER 1
#define CONFIG_G722_DEMUXER 1
#define CONFIG_G723_1_DEMUXER 1
#define CONFIG_G726_DEMUXER 1
#define CONFIG_G726LE_DEMUXER 1
#define CONFIG_G729_DEMUXER 1
#define CONFIG_GDV_DEMUXER 1
#define CONFIG_GENH_DEMUXER 1
#define CONFIG_GIF_DEMUXER 1
#define CONFIG_GSM_DEMUXER 1
#define CONFIG_GXF_DEMUXER 1
#define CONFIG_H261_DEMUXER 1
#define CONFIG_H263_DEMUXER 1
#define CONFIG_H264_DEMUXER 1
#define CONFIG_HCA_DEMUXER 1
#define CONFIG_HCOM_DEMUXER 1
#define CONFIG_HEVC_DEMUXER 1
#define CONFIG_HLS_DEMUXER 1
#define CONFIG_HNM_DEMUXER 1
#define CONFIG_ICO_DEMUXER 1
#define CONFIG_IDCIN_DEMUXER 1
#define CONFIG_IDF_DEMUXER 1
#define CONFIG_IFF_DEMUXER 1
#define CONFIG_IFV_DEMUXER 1
#define CONFIG_ILBC_DEMUXER 1
#define CONFIG_IMAGE2_DEMUXER 1
#define CONFIG_IMAGE2PIPE_DEMUXER 1
#define CONFIG_IMAGE2_ALIAS_PIX_DEMUXER 1
#define CONFIG_IMAGE2_BRENDER_PIX_DEMUXER 1
#define CONFIG_INGENIENT_DEMUXER 1
#define CONFIG_IPMOVIE_DEMUXER 1
#define CONFIG_IPU_DEMUXER 1
#define CONFIG_IRCAM_DEMUXER 1
#define CONFIG_ISS_DEMUXER 1
#define CONFIG_IV8_DEMUXER 1
#define CONFIG_IVF_DEMUXER 1
#define CONFIG_IVR_DEMUXER 1
#define CONFIG_JACOSUB_DEMUXER 1
#define CONFIG_JV_DEMUXER 1
#define CONFIG_KUX_DEMUXER 1
#define CONFIG_KVAG_DEMUXER 1
#define CONFIG_LMLM4_DEMUXER 1
#define CONFIG_LOAS_DEMUXER 1
#define CONFIG_LUODAT_DEMUXER 1
#define CONFIG_LRC_DEMUXER 1
#define CONFIG_LVF_DEMUXER 1
#define CONFIG_LXF_DEMUXER 1
#define CONFIG_M4V_DEMUXER 1
#define CONFIG_MCA_DEMUXER 1
#define CONFIG_MCC_DEMUXER 1
#define CONFIG_MATROSKA_DEMUXER 1
#define CONFIG_MGSTS_DEMUXER 1
#define CONFIG_MICRODVD_DEMUXER 1
#define CONFIG_MJPEG_DEMUXER 1
#define CONFIG_MJPEG_2000_DEMUXER 1
#define CONFIG_MLP_DEMUXER 1
#define CONFIG_MLV_DEMUXER 1
#define CONFIG_MM_DEMUXER 1
#define CONFIG_MMF_DEMUXER 1
#define CONFIG_MODS_DEMUXER 1
#define CONFIG_MOFLEX_DEMUXER 1
#define CONFIG_MOV_DEMUXER 1
#define CONFIG_MP3_DEMUXER 1
#define CONFIG_MPC_DEMUXER 1
#define CONFIG_MPC8_DEMUXER 1
#define CONFIG_MPEGPS_DEMUXER 1
#define CONFIG_MPEGTS_DEMUXER 1
#define CONFIG_MPEGTSRAW_DEMUXER 1
#define CONFIG_MPEGVIDEO_DEMUXER 1
#define CONFIG_MPJPEG_DEMUXER 1
#define CONFIG_MPL2_DEMUXER 1
#define CONFIG_MPSUB_DEMUXER 1
#define CONFIG_MSF_DEMUXER 1
#define CONFIG_MSNWC_TCP_DEMUXER 1
#define CONFIG_MTAF_DEMUXER 1
#define CONFIG_MTV_DEMUXER 1
#define CONFIG_MUSX_DEMUXER 1
#define CONFIG_MV_DEMUXER 1
#define CONFIG_MVI_DEMUXER 1
#define CONFIG_MXF_DEMUXER 1
#define CONFIG_MXG_DEMUXER 1
#define CONFIG_NC_DEMUXER 1
#define CONFIG_NISTSPHERE_DEMUXER 1
#define CONFIG_NSP_DEMUXER 1
#define CONFIG_NSV_DEMUXER 1
#define CONFIG_NUT_DEMUXER 1
#define CONFIG_NUV_DEMUXER 1
#define CONFIG_OBU_DEMUXER 1
#define CONFIG_OGG_DEMUXER 1
#define CONFIG_OMA_DEMUXER 1
#define CONFIG_PAF_DEMUXER 1
#define CONFIG_PCM_ALAW_DEMUXER 1
#define CONFIG_PCM_MULAW_DEMUXER 1
#define CONFIG_PCM_VIDC_DEMUXER 1
#define CONFIG_PCM_F64BE_DEMUXER 1
#define CONFIG_PCM_F64LE_DEMUXER 1
#define CONFIG_PCM_F32BE_DEMUXER 1
#define CONFIG_PCM_F32LE_DEMUXER 1
#define CONFIG_PCM_S32BE_DEMUXER 1
#define CONFIG_PCM_S32LE_DEMUXER 1
#define CONFIG_PCM_S24BE_DEMUXER 1
#define CONFIG_PCM_S24LE_DEMUXER 1
#define CONFIG_PCM_S16BE_DEMUXER 1
#define CONFIG_PCM_S16LE_DEMUXER 1
#define CONFIG_PCM_S8_DEMUXER 1
#define CONFIG_PCM_U32BE_DEMUXER 1
#define CONFIG_PCM_U32LE_DEMUXER 1
#define CONFIG_PCM_U24BE_DEMUXER 1
#define CONFIG_PCM_U24LE_DEMUXER 1
#define CONFIG_PCM_U16BE_DEMUXER 1
#define CONFIG_PCM_U16LE_DEMUXER 1
#define CONFIG_PCM_U8_DEMUXER 1
#define CONFIG_PJS_DEMUXER 1
#define CONFIG_PMP_DEMUXER 1
#define CONFIG_PP_BNK_DEMUXER 1
#define CONFIG_PVA_DEMUXER 1
#define CONFIG_PVF_DEMUXER 1
#define CONFIG_QCP_DEMUXER 1
#define CONFIG_R3D_DEMUXER 1
#define CONFIG_RAWVIDEO_DEMUXER 1
#define CONFIG_REALTEXT_DEMUXER 1
#define CONFIG_REDSPARK_DEMUXER 1
#define CONFIG_RL2_DEMUXER 1
#define CONFIG_RM_DEMUXER 1
#define CONFIG_ROQ_DEMUXER 1
#define CONFIG_RPL_DEMUXER 1
#define CONFIG_RSD_DEMUXER 1
#define CONFIG_RSO_DEMUXER 1
#define CONFIG_RTP_DEMUXER 1
#define CONFIG_RTSP_DEMUXER 1
#define CONFIG_S337M_DEMUXER 1
#define CONFIG_SAMI_DEMUXER 1
#define CONFIG_SAP_DEMUXER 1
#define CONFIG_SBC_DEMUXER 1
#define CONFIG_SBG_DEMUXER 1
#define CONFIG_SCC_DEMUXER 1
#define CONFIG_SDP_DEMUXER 1
#define CONFIG_SDR2_DEMUXER 1
#define CONFIG_SDS_DEMUXER 1
#define CONFIG_SDX_DEMUXER 1
#define CONFIG_SEGAFILM_DEMUXER 1
#define CONFIG_SER_DEMUXER 1
#define CONFIG_SHORTEN_DEMUXER 1
#define CONFIG_SIFF_DEMUXER 1
#define CONFIG_SLN_DEMUXER 1
#define CONFIG_SMACKER_DEMUXER 1
#define CONFIG_SMJPEG_DEMUXER 1
#define CONFIG_SMUSH_DEMUXER 1
#define CONFIG_SOL_DEMUXER 1
#define CONFIG_SOX_DEMUXER 1
#define CONFIG_SPDIF_DEMUXER 1
#define CONFIG_SRT_DEMUXER 1
#define CONFIG_STR_DEMUXER 1
#define CONFIG_STL_DEMUXER 1
#define CONFIG_SUBVIEWER1_DEMUXER 1
#define CONFIG_SUBVIEWER_DEMUXER 1
#define CONFIG_SUP_DEMUXER 1
#define CONFIG_SVAG_DEMUXER 1
#define CONFIG_SVS_DEMUXER 1
#define CONFIG_SWF_DEMUXER 1
#define CONFIG_TAK_DEMUXER 1
#define CONFIG_TEDCAPTIONS_DEMUXER 1
#define CONFIG_THP_DEMUXER 1
#define CONFIG_THREEDOSTR_DEMUXER 1
#define CONFIG_TIERTEXSEQ_DEMUXER 1
#define CONFIG_TMV_DEMUXER 1
#define CONFIG_TRUEHD_DEMUXER 1
#define CONFIG_TTA_DEMUXER 1
#define CONFIG_TXD_DEMUXER 1
#define CONFIG_TTY_DEMUXER 1
#define CONFIG_TY_DEMUXER 1
#define CONFIG_V210_DEMUXER 1
#define CONFIG_V210X_DEMUXER 1
#define CONFIG_VAG_DEMUXER 1
#define CONFIG_VC1_DEMUXER 1
#define CONFIG_VC1T_DEMUXER 1
#define CONFIG_VIVIDAS_DEMUXER 1
#define CONFIG_VIVO_DEMUXER 1
#define CONFIG_VMD_DEMUXER 1
#define CONFIG_VOBSUB_DEMUXER 1
#define CONFIG_VOC_DEMUXER 1
#define CONFIG_VPK_DEMUXER 1
#define CONFIG_VPLAYER_DEMUXER 1
#define CONFIG_VQF_DEMUXER 1
#define CONFIG_W64_DEMUXER 1
#define CONFIG_WAV_DEMUXER 1
#define CONFIG_WC3_DEMUXER 1
#define CONFIG_WEBM_DASH_MANIFEST_DEMUXER 1
#define CONFIG_WEBVTT_DEMUXER 1
#define CONFIG_WSAUD_DEMUXER 1
#define CONFIG_WSD_DEMUXER 1
#define CONFIG_WSVQA_DEMUXER 1
#define CONFIG_WTV_DEMUXER 1
#define CONFIG_WVE_DEMUXER 1
#define CONFIG_WV_DEMUXER 1
#define CONFIG_XA_DEMUXER 1
#define CONFIG_XBIN_DEMUXER 1
#define CONFIG_XMV_DEMUXER 1
#define CONFIG_XVAG_DEMUXER 1
#define CONFIG_XWMA_DEMUXER 1
#define CONFIG_YOP_DEMUXER 1
#define CONFIG_YUV4MPEGPIPE_DEMUXER 1
#define CONFIG_IMAGE_BMP_PIPE_DEMUXER 1
#define CONFIG_IMAGE_DDS_PIPE_DEMUXER 1
#define CONFIG_IMAGE_DPX_PIPE_DEMUXER 1
#define CONFIG_IMAGE_EXR_PIPE_DEMUXER 1
#define CONFIG_IMAGE_GIF_PIPE_DEMUXER 1
#define CONFIG_IMAGE_J2K_PIPE_DEMUXER 1
#define CONFIG_IMAGE_JPEG_PIPE_DEMUXER 1
#define CONFIG_IMAGE_JPEGLS_PIPE_DEMUXER 1
#define CONFIG_IMAGE_PAM_PIPE_DEMUXER 1
#define CONFIG_IMAGE_PBM_PIPE_DEMUXER 1
#define CONFIG_IMAGE_PCX_PIPE_DEMUXER 1
#define CONFIG_IMAGE_PGMYUV_PIPE_DEMUXER 1
#define CONFIG_IMAGE_PGM_PIPE_DEMUXER 1
#define CONFIG_IMAGE_PGX_PIPE_DEMUXER 1
#define CONFIG_IMAGE_PHOTOCD_PIPE_DEMUXER 1
#define CONFIG_IMAGE_PICTOR_PIPE_DEMUXER 1
#define CONFIG_IMAGE_PNG_PIPE_DEMUXER 1
#define CONFIG_IMAGE_PPM_PIPE_DEMUXER 1
#define CONFIG_IMAGE_PSD_PIPE_DEMUXER 1
#define CONFIG_IMAGE_QDRAW_PIPE_DEMUXER 1
#define CONFIG_IMAGE_SGI_PIPE_DEMUXER 1
#define CONFIG_IMAGE_SVG_PIPE_DEMUXER 1
#define CONFIG_IMAGE_SUNRAST_PIPE_DEMUXER 1
#define CONFIG_IMAGE_TIFF_PIPE_DEMUXER 1
#define CONFIG_IMAGE_WEBP_PIPE_DEMUXER 1
#define CONFIG_IMAGE_XPM_PIPE_DEMUXER 1
#define CONFIG_IMAGE_XWD_PIPE_DEMUXER 1
#define CONFIG_LIBGME_DEMUXER 1
#define CONFIG_LIBMODPLUG_DEMUXER 1
#define CONFIG_LIBOPENMPT_DEMUXER 0
#define CONFIG_VAPOURSYNTH_DEMUXER 0
#define CONFIG_A64_MUXER 1
#define CONFIG_AC3_MUXER 1
#define CONFIG_ADTS_MUXER 1
#define CONFIG_ADX_MUXER 1
#define CONFIG_AIFF_MUXER 1
#define CONFIG_AMR_MUXER 1
#define CONFIG_APM_MUXER 1
#define CONFIG_APNG_MUXER 1
#define CONFIG_APTX_MUXER 1
#define CONFIG_APTX_HD_MUXER 1
#define CONFIG_ARGO_ASF_MUXER 1
#define CONFIG_ASF_MUXER 1
#define CONFIG_ASS_MUXER 1
#define CONFIG_AST_MUXER 1
#define CONFIG_ASF_STREAM_MUXER 1
#define CONFIG_AU_MUXER 1
#define CONFIG_AVI_MUXER 1
#define CONFIG_AVM2_MUXER 1
#define CONFIG_AVS2_MUXER 1
#define CONFIG_BIT_MUXER 1
#define CONFIG_CAF_MUXER 1
#define CONFIG_CAVSVIDEO_MUXER 1
#define CONFIG_CODEC2_MUXER 1
#define CONFIG_CODEC2RAW_MUXER 1
#define CONFIG_CRC_MUXER 1
#define CONFIG_DASH_MUXER 1
#define CONFIG_DATA_MUXER 1
#define CONFIG_DAUD_MUXER 1
#define CONFIG_DIRAC_MUXER 1
#define CONFIG_DNXHD_MUXER 1
#define CONFIG_DTS_MUXER 1
#define CONFIG_DV_MUXER 1
#define CONFIG_EAC3_MUXER 1
#define CONFIG_F4V_MUXER 1
#define CONFIG_FFMETADATA_MUXER 1
#define CONFIG_FIFO_MUXER 1
#define CONFIG_FIFO_TEST_MUXER 1
#define CONFIG_FILMSTRIP_MUXER 1
#define CONFIG_FITS_MUXER 1
#define CONFIG_FLAC_MUXER 1
#define CONFIG_FLV_MUXER 1
#define CONFIG_FRAMECRC_MUXER 1
#define CONFIG_FRAMEHASH_MUXER 1
#define CONFIG_FRAMEMD5_MUXER 1
#define CONFIG_G722_MUXER 1
#define CONFIG_G723_1_MUXER 1
#define CONFIG_G726_MUXER 1
#define CONFIG_G726LE_MUXER 1
#define CONFIG_GIF_MUXER 1
#define CONFIG_GSM_MUXER 1
#define CONFIG_GXF_MUXER 1
#define CONFIG_H261_MUXER 1
#define CONFIG_H263_MUXER 1
#define CONFIG_H264_MUXER 1
#define CONFIG_HASH_MUXER 1
#define CONFIG_HDS_MUXER 1
#define CONFIG_HEVC_MUXER 1
#define CONFIG_HLS_MUXER 1
#define CONFIG_ICO_MUXER 1
#define CONFIG_ILBC_MUXER 1
#define CONFIG_IMAGE2_MUXER 1
#define CONFIG_IMAGE2PIPE_MUXER 1
#define CONFIG_IPOD_MUXER 1
#define CONFIG_IRCAM_MUXER 1
#define CONFIG_ISMV_MUXER 1
#define CONFIG_IVF_MUXER 1
#define CONFIG_JACOSUB_MUXER 1
#define CONFIG_KVAG_MUXER 1
#define CONFIG_LATM_MUXER 1
#define CONFIG_LRC_MUXER 1
#define CONFIG_M4V_MUXER 1
#define CONFIG_MD5_MUXER 1
#define CONFIG_MATROSKA_MUXER 1
#define CONFIG_MATROSKA_AUDIO_MUXER 1
#define CONFIG_MICRODVD_MUXER 1
#define CONFIG_MJPEG_MUXER 1
#define CONFIG_MLP_MUXER 1
#define CONFIG_MMF_MUXER 1
#define CONFIG_MOV_MUXER 1
#define CONFIG_MP2_MUXER 1
#define CONFIG_MP3_MUXER 1
#define CONFIG_MP4_MUXER 1
#define CONFIG_MPEG1SYSTEM_MUXER 1
#define CONFIG_MPEG1VCD_MUXER 1
#define CONFIG_MPEG1VIDEO_MUXER 1
#define CONFIG_MPEG2DVD_MUXER 1
#define CONFIG_MPEG2SVCD_MUXER 1
#define CONFIG_MPEG2VIDEO_MUXER 1
#define CONFIG_MPEG2VOB_MUXER 1
#define CONFIG_MPEGTS_MUXER 1
#define CONFIG_MPJPEG_MUXER 1
#define CONFIG_MXF_MUXER 1
#define CONFIG_MXF_D10_MUXER 1
#define CONFIG_MXF_OPATOM_MUXER 1
#define CONFIG_NULL_MUXER 1
#define CONFIG_NUT_MUXER 1
#define CONFIG_OGA_MUXER 1
#define CONFIG_OGG_MUXER 1
#define CONFIG_OGV_MUXER 1
#define CONFIG_OMA_MUXER 1
#define CONFIG_OPUS_MUXER 1
#define CONFIG_PCM_ALAW_MUXER 1
#define CONFIG_PCM_MULAW_MUXER 1
#define CONFIG_PCM_VIDC_MUXER 1
#define CONFIG_PCM_F64BE_MUXER 1
#define CONFIG_PCM_F64LE_MUXER 1
#define CONFIG_PCM_F32BE_MUXER 1
#define CONFIG_PCM_F32LE_MUXER 1
#define CONFIG_PCM_S32BE_MUXER 1
#define CONFIG_PCM_S32LE_MUXER 1
#define CONFIG_PCM_S24BE_MUXER 1
#define CONFIG_PCM_S24LE_MUXER 1
#define CONFIG_PCM_S16BE_MUXER 1
#define CONFIG_PCM_S16LE_MUXER 1
#define CONFIG_PCM_S8_MUXER 1
#define CONFIG_PCM_U32BE_MUXER 1
#define CONFIG_PCM_U32LE_MUXER 1
#define CONFIG_PCM_U24BE_MUXER 1
#define CONFIG_PCM_U24LE_MUXER 1
#define CONFIG_PCM_U16BE_MUXER 1
#define CONFIG_PCM_U16LE_MUXER 1
#define CONFIG_PCM_U8_MUXER 1
#define CONFIG_PSP_MUXER 1
#define CONFIG_RAWVIDEO_MUXER 1
#define CONFIG_RM_MUXER 1
#define CONFIG_ROQ_MUXER 1
#define CONFIG_RSO_MUXER 1
#define CONFIG_RTP_MUXER 1
#define CONFIG_RTP_MPEGTS_MUXER 1
#define CONFIG_RTSP_MUXER 1
#define CONFIG_SAP_MUXER 1
#define CONFIG_SBC_MUXER 1
#define CONFIG_SCC_MUXER 1
#define CONFIG_SEGAFILM_MUXER 1
#define CONFIG_SEGMENT_MUXER 1
#define CONFIG_STREAM_SEGMENT_MUXER 1
#define CONFIG_SINGLEJPEG_MUXER 1
#define CONFIG_SMJPEG_MUXER 1
#define CONFIG_SMOOTHSTREAMING_MUXER 1
#define CONFIG_SOX_MUXER 1
#define CONFIG_SPX_MUXER 1
#define CONFIG_SPDIF_MUXER 1
#define CONFIG_SRT_MUXER 1
#define CONFIG_STREAMHASH_MUXER 1
#define CONFIG_SUP_MUXER 1
#define CONFIG_SWF_MUXER 1
#define CONFIG_TEE_MUXER 1
#define CONFIG_TG2_MUXER 1
#define CONFIG_TGP_MUXER 1
#define CONFIG_MKVTIMESTAMP_V2_MUXER 1
#define CONFIG_TRUEHD_MUXER 1
#define CONFIG_TTA_MUXER 1
#define CONFIG_UNCODEDFRAMECRC_MUXER 1
#define CONFIG_VC1_MUXER 1
#define CONFIG_VC1T_MUXER 1
#define CONFIG_VOC_MUXER 1
#define CONFIG_W64_MUXER 1
#define CONFIG_WAV_MUXER 1
#define CONFIG_WEBM_MUXER 1
#define CONFIG_WEBM_DASH_MANIFEST_MUXER 1
#define CONFIG_WEBM_CHUNK_MUXER 1
#define CONFIG_WEBP_MUXER 1
#define CONFIG_WEBVTT_MUXER 1
#define CONFIG_WTV_MUXER 1
#define CONFIG_WV_MUXER 1
#define CONFIG_YUV4MPEGPIPE_MUXER 1
#define CONFIG_CHROMAPRINT_MUXER 1
#define CONFIG_ASYNC_PROTOCOL 1
#define CONFIG_BLURAY_PROTOCOL 1
#define CONFIG_CACHE_PROTOCOL 1
#define CONFIG_CONCAT_PROTOCOL 1
#define CONFIG_CRYPTO_PROTOCOL 1
#define CONFIG_DATA_PROTOCOL 1
#define CONFIG_FFRTMPCRYPT_PROTOCOL 0
#define CONFIG_FFRTMPHTTP_PROTOCOL 1
#define CONFIG_FILE_PROTOCOL 1
#define CONFIG_FTP_PROTOCOL 1
#define CONFIG_GOPHER_PROTOCOL 1
#define CONFIG_HLS_PROTOCOL 1
#define CONFIG_HTTP_PROTOCOL 1
#define CONFIG_HTTPPROXY_PROTOCOL 1
#define CONFIG_HTTPS_PROTOCOL 1
#define CONFIG_ICECAST_PROTOCOL 1
#define CONFIG_MMSH_PROTOCOL 1
#define CONFIG_MMST_PROTOCOL 1
#define CONFIG_MD5_PROTOCOL 1
#define CONFIG_PIPE_PROTOCOL 1
#define CONFIG_PROMPEG_PROTOCOL 1
#define CONFIG_RTMP_PROTOCOL 1
#define CONFIG_RTMPE_PROTOCOL 0
#define CONFIG_RTMPS_PROTOCOL 1
#define CONFIG_RTMPT_PROTOCOL 1
#define CONFIG_RTMPTE_PROTOCOL 0
#define CONFIG_RTMPTS_PROTOCOL 1
#define CONFIG_RTP_PROTOCOL 1
#define CONFIG_SCTP_PROTOCOL 0
#define CONFIG_SRTP_PROTOCOL 1
#define CONFIG_SUBFILE_PROTOCOL 1
#define CONFIG_TEE_PROTOCOL 1
#define CONFIG_TCP_PROTOCOL 1
#define CONFIG_TLS_PROTOCOL 1
#define CONFIG_UDP_PROTOCOL 1
#define CONFIG_UDPLITE_PROTOCOL 1
#define CONFIG_UNIX_PROTOCOL 0
#define CONFIG_LIBAMQP_PROTOCOL 0
#define CONFIG_LIBRTMP_PROTOCOL 0
#define CONFIG_LIBRTMPE_PROTOCOL 0
#define CONFIG_LIBRTMPS_PROTOCOL 0
#define CONFIG_LIBRTMPT_PROTOCOL 0
#define CONFIG_LIBRTMPTE_PROTOCOL 0
#define CONFIG_LIBSRT_PROTOCOL 1
#define CONFIG_LIBSSH_PROTOCOL 1
#define CONFIG_LIBSMBCLIENT_PROTOCOL 0
#define CONFIG_LIBZMQ_PROTOCOL 1
#endif /* FFMPEG_CONFIG_H */

#define MAX_QUEUE_SIZE (15 * 1024 * 1024)
#define MIN_FRAMES 25
#define EXTERNAL_CLOCK_MIN_FRAMES 2
#define EXTERNAL_CLOCK_MAX_FRAMES 10

/* Minimum SDL audio buffer size, in samples. */
#define SDL_AUDIO_MIN_BUFFER_SIZE 512
/* Calculate actual buffer size keeping in mind not cause too frequent audio callbacks */
#define SDL_AUDIO_MAX_CALLBACKS_PER_SEC 30

/* Step size for volume control in dB */
#define SDL_VOLUME_STEP (0.75)

/* no AV sync correction is done if below the minimum AV sync threshold */
#define AV_SYNC_THRESHOLD_MIN 0.04
/* AV sync correction is done if above the maximum AV sync threshold */
#define AV_SYNC_THRESHOLD_MAX 0.1
/* If a frame duration is longer than this, it will not be duplicated to compensate AV sync */
#define AV_SYNC_FRAMEDUP_THRESHOLD 0.1
/* no AV correction is done if too big error */
#define AV_NOSYNC_THRESHOLD 10.0

/* maximum audio speed change to get correct sync */
#define SAMPLE_CORRECTION_PERCENT_MAX 10

/* external clock speed adjustment constants for realtime sources based on buffer fullness */
#define EXTERNAL_CLOCK_SPEED_MIN 0.900
#define EXTERNAL_CLOCK_SPEED_MAX 1.010
#define EXTERNAL_CLOCK_SPEED_STEP 0.001

/* we use about AUDIO_DIFF_AVG_NB A-V differences to make the average */
#define AUDIO_DIFF_AVG_NB 20

/* polls for possible required screen refresh at least this often, should be less than 1/fps */
#define REFRESH_RATE 0.01

/* NOTE: the size must be big enough to compensate the hardware audio buffersize size */
/* TODO: We assume that a decoded and resampled frame fits into this buffer */
#define SAMPLE_ARRAY_SIZE (8 * 65536)

#define CURSOR_HIDE_DELAY 1000000

#define USE_ONEPASS_SUBTITLE_RENDER 1

#define SWS_BICUBIC 4
static unsigned sws_flags = SWS_BICUBIC;

#define AV_FRAME_FLAG_CORRUPT (1 << 0)
#define AV_FRAME_FLAG_DISCARD (1 << 2)
#define FF_DECODE_ERROR_INVALID_BITSTREAM 1
#define FF_DECODE_ERROR_MISSING_REFERENCE 2
#define FF_DECODE_ERROR_CONCEALMENT_ACTIVE 4
#define FF_DECODE_ERROR_DECODE_SLICES 8
#define AV_NUM_DATA_POINTERS 8

#define VIDEO_PICTURE_QUEUE_SIZE 3
#define SUBPICTURE_QUEUE_SIZE 16
#define SAMPLE_QUEUE_SIZE 9
#define FFMAX(a, b) ((a) > (b) ? (a) : (b))
#define FRAME_QUEUE_SIZE FFMAX(SAMPLE_QUEUE_SIZE, FFMAX(VIDEO_PICTURE_QUEUE_SIZE, SUBPICTURE_QUEUE_SIZE))

#define FF_PROFILE_UNKNOWN -99
#define FF_PROFILE_RESERVED -100

#define FF_PROFILE_AAC_MAIN 0
#define FF_PROFILE_AAC_LOW 1
#define FF_PROFILE_AAC_SSR 2
#define FF_PROFILE_AAC_LTP 3
#define FF_PROFILE_AAC_HE 4
#define FF_PROFILE_AAC_HE_V2 28
#define FF_PROFILE_AAC_LD 22
#define FF_PROFILE_AAC_ELD 38
#define FF_PROFILE_MPEG2_AAC_LOW 128
#define FF_PROFILE_MPEG2_AAC_HE 131

#define FF_PROFILE_DNXHD 0
#define FF_PROFILE_DNXHR_LB 1
#define FF_PROFILE_DNXHR_SQ 2
#define FF_PROFILE_DNXHR_HQ 3
#define FF_PROFILE_DNXHR_HQX 4
#define FF_PROFILE_DNXHR_444 5

#define FF_PROFILE_DTS 20
#define FF_PROFILE_DTS_ES 30
#define FF_PROFILE_DTS_96_24 40
#define FF_PROFILE_DTS_HD_HRA 50
#define FF_PROFILE_DTS_HD_MA 60
#define FF_PROFILE_DTS_EXPRESS 70

#define FF_PROFILE_MPEG2_422 0
#define FF_PROFILE_MPEG2_HIGH 1
#define FF_PROFILE_MPEG2_SS 2
#define FF_PROFILE_MPEG2_SNR_SCALABLE 3
#define FF_PROFILE_MPEG2_MAIN 4
#define FF_PROFILE_MPEG2_SIMPLE 5

#define FF_PROFILE_H264_CONSTRAINED (1 << 9) // 8+1; constraint_set1_flag
#define FF_PROFILE_H264_INTRA (1 << 11)      // 8+3; constraint_set3_flag

#define FF_PROFILE_H264_BASELINE 66
#define FF_PROFILE_H264_CONSTRAINED_BASELINE (66 | FF_PROFILE_H264_CONSTRAINED)
#define FF_PROFILE_H264_MAIN 77
#define FF_PROFILE_H264_EXTENDED 88
#define FF_PROFILE_H264_HIGH 100
#define FF_PROFILE_H264_HIGH_10 110
#define FF_PROFILE_H264_HIGH_10_INTRA (110 | FF_PROFILE_H264_INTRA)
#define FF_PROFILE_H264_MULTIVIEW_HIGH 118
#define FF_PROFILE_H264_HIGH_422 122
#define FF_PROFILE_H264_HIGH_422_INTRA (122 | FF_PROFILE_H264_INTRA)
#define FF_PROFILE_H264_STEREO_HIGH 128
#define FF_PROFILE_H264_HIGH_444 144
#define FF_PROFILE_H264_HIGH_444_PREDICTIVE 244
#define FF_PROFILE_H264_HIGH_444_INTRA (244 | FF_PROFILE_H264_INTRA)
#define FF_PROFILE_H264_CAVLC_444 44

#define FF_PROFILE_VC1_SIMPLE 0
#define FF_PROFILE_VC1_MAIN 1
#define FF_PROFILE_VC1_COMPLEX 2
#define FF_PROFILE_VC1_ADVANCED 3

#define FF_PROFILE_MPEG4_SIMPLE 0
#define FF_PROFILE_MPEG4_SIMPLE_SCALABLE 1
#define FF_PROFILE_MPEG4_CORE 2
#define FF_PROFILE_MPEG4_MAIN 3
#define FF_PROFILE_MPEG4_N_BIT 4
#define FF_PROFILE_MPEG4_SCALABLE_TEXTURE 5
#define FF_PROFILE_MPEG4_SIMPLE_FACE_ANIMATION 6
#define FF_PROFILE_MPEG4_BASIC_ANIMATED_TEXTURE 7
#define FF_PROFILE_MPEG4_HYBRID 8
#define FF_PROFILE_MPEG4_ADVANCED_REAL_TIME 9
#define FF_PROFILE_MPEG4_CORE_SCALABLE 10
#define FF_PROFILE_MPEG4_ADVANCED_CODING 11
#define FF_PROFILE_MPEG4_ADVANCED_CORE 12
#define FF_PROFILE_MPEG4_ADVANCED_SCALABLE_TEXTURE 13
#define FF_PROFILE_MPEG4_SIMPLE_STUDIO 14
#define FF_PROFILE_MPEG4_ADVANCED_SIMPLE 15

#define FF_PROFILE_JPEG2000_CSTREAM_RESTRICTION_0 1
#define FF_PROFILE_JPEG2000_CSTREAM_RESTRICTION_1 2
#define FF_PROFILE_JPEG2000_CSTREAM_NO_RESTRICTION 32768
#define FF_PROFILE_JPEG2000_DCINEMA_2K 3
#define FF_PROFILE_JPEG2000_DCINEMA_4K 4

#define FF_PROFILE_VP9_0 0
#define FF_PROFILE_VP9_1 1
#define FF_PROFILE_VP9_2 2
#define FF_PROFILE_VP9_3 3

#define FF_PROFILE_HEVC_MAIN 1
#define FF_PROFILE_HEVC_MAIN_10 2
#define FF_PROFILE_HEVC_MAIN_STILL_PICTURE 3
#define FF_PROFILE_HEVC_REXT 4

#define FF_PROFILE_AV1_MAIN 0
#define FF_PROFILE_AV1_HIGH 1
#define FF_PROFILE_AV1_PROFESSIONAL 2

#define FF_PROFILE_MJPEG_HUFFMAN_BASELINE_DCT 0xc0
#define FF_PROFILE_MJPEG_HUFFMAN_EXTENDED_SEQUENTIAL_DCT 0xc1
#define FF_PROFILE_MJPEG_HUFFMAN_PROGRESSIVE_DCT 0xc2
#define FF_PROFILE_MJPEG_HUFFMAN_LOSSLESS 0xc3
#define FF_PROFILE_MJPEG_JPEG_LS 0xf7

#define FF_PROFILE_SBC_MSBC 1

#define FF_PROFILE_PRORES_PROXY 0
#define FF_PROFILE_PRORES_LT 1
#define FF_PROFILE_PRORES_STANDARD 2
#define FF_PROFILE_PRORES_HQ 3
#define FF_PROFILE_PRORES_4444 4
#define FF_PROFILE_PRORES_XQ 5

#define FF_PROFILE_ARIB_PROFILE_A 0
#define FF_PROFILE_ARIB_PROFILE_C 1

#define FF_CMP_SAD 0
#define FF_CMP_SSE 1
#define FF_CMP_SATD 2
#define FF_CMP_DCT 3
#define FF_CMP_PSNR 4
#define FF_CMP_BIT 5
#define FF_CMP_RD 6
#define FF_CMP_ZERO 7
#define FF_CMP_VSAD 8
#define FF_CMP_VSSE 9
#define FF_CMP_NSSE 10
#define FF_CMP_W53 11
#define FF_CMP_W97 12
#define FF_CMP_DCTMAX 13
#define FF_CMP_DCT264 14
#define FF_CMP_MEDIAN_SAD 15
#define FF_CMP_CHROMA 256

#define FF_PRED_LEFT 0
#define FF_PRED_PLANE 1
#define FF_PRED_MEDIAN 2

#define SLICE_FLAG_CODED_ORDER 0x0001 ///< draw_horiz_band() is called in coded order instead of display
#define SLICE_FLAG_ALLOW_FIELD 0x0002 ///< allow draw_horiz_band() with field slices (MPEG-2 field pics)
#define SLICE_FLAG_ALLOW_PLANE 0x0004 ///< allow draw_horiz_band() with 1 component at a time (SVQ1)

#define FF_MB_DECISION_SIMPLE 0 ///< uses mb_cmp
#define FF_MB_DECISION_BITS 1   ///< chooses the one which needs the fewest bits
#define FF_MB_DECISION_RD 2     ///< rate distortion
#define FF_COMPRESSION_DEFAULT -1

#define FF_CODER_TYPE_VLC 0
#define FF_CODER_TYPE_AC 1
#define FF_CODER_TYPE_RAW 2
#define FF_CODER_TYPE_RLE 3

#define FF_BUG_AUTODETECT 1 ///< autodetection
#define FF_BUG_XVID_ILACE 4
#define FF_BUG_UMP4 8
#define FF_BUG_NO_PADDING 16
#define FF_BUG_AMV 32
#define FF_BUG_QPEL_CHROMA 64
#define FF_BUG_STD_QPEL 128
#define FF_BUG_QPEL_CHROMA2 256
#define FF_BUG_DIRECT_BLOCKSIZE 512
#define FF_BUG_EDGE 1024
#define FF_BUG_HPEL_CHROMA 2048
#define FF_BUG_DC_CLIP 4096
#define FF_BUG_MS 8192 ///< Work around various bugs in Microsoft's broken decoders.
#define FF_BUG_TRUNCATED 16384
#define FF_BUG_IEDGE 32768

#define FF_COMPLIANCE_VERY_STRICT 2 ///< Strictly conform to an older more strict version of the spec or reference software.
#define FF_COMPLIANCE_STRICT 1      ///< Strictly conform to all the things in the spec no matter what consequences.
#define FF_COMPLIANCE_NORMAL 0
#define FF_COMPLIANCE_UNOFFICIAL -1   ///< Allow unofficial extensions
#define FF_COMPLIANCE_EXPERIMENTAL -2 ///< Allow nonstandardized experimental things.

#define FF_EC_GUESS_MVS 1
#define FF_EC_DEBLOCK 2
#define FF_EC_FAVOR_INTER 256

#define FF_DEBUG_PICT_INFO 1
#define FF_DEBUG_RC 2
#define FF_DEBUG_BITSTREAM 4
#define FF_DEBUG_MB_TYPE 8
#define FF_DEBUG_QP 16
#define FF_DEBUG_DCT_COEFF 0x00000040
#define FF_DEBUG_SKIP 0x00000080
#define FF_DEBUG_STARTCODE 0x00000100
#define FF_DEBUG_ER 0x00000400
#define FF_DEBUG_MMCO 0x00000800
#define FF_DEBUG_BUGS 0x00001000
#define FF_DEBUG_BUFFERS 0x00008000
#define FF_DEBUG_THREADS 0x00010000
#define FF_DEBUG_GREEN_MD 0x00800000
#define FF_DEBUG_NOMC 0x01000000

#define AV_EF_CRCCHECK (1 << 0)
#define AV_EF_BITSTREAM (1 << 1)   ///< detect bitstream specification deviations
#define AV_EF_BUFFER (1 << 2)      ///< detect improper bitstream length
#define AV_EF_EXPLODE (1 << 3)     ///< abort decoding on minor error detection
#define AV_EF_IGNORE_ERR (1 << 15) ///< ignore errors and continue
#define AV_EF_CAREFUL (1 << 16)    ///< consider things that violate the spec, are fast to calculate and have not been seen in the wild as errors
#define AV_EF_COMPLIANT (1 << 17)  ///< consider all spec non compliances as errors
#define AV_EF_AGGRESSIVE (1 << 18) ///< consider things that a sane encoder should not do as an error

#define FF_DCT_AUTO 0
#define FF_DCT_FASTINT 1
#define FF_DCT_INT 2
#define FF_DCT_MMX 3
#define FF_DCT_ALTIVEC 5
#define FF_DCT_FAAN 6

#define FF_IDCT_AUTO 0
#define FF_IDCT_INT 1
#define FF_IDCT_SIMPLE 2
#define FF_IDCT_SIMPLEMMX 3
#define FF_IDCT_ARM 7
#define FF_IDCT_ALTIVEC 8
#define FF_IDCT_SIMPLEARM 10
#define FF_IDCT_XVID 14
#define FF_IDCT_SIMPLEARMV5TE 16
#define FF_IDCT_SIMPLEARMV6 17
#define FF_IDCT_FAAN 20
#define FF_IDCT_SIMPLENEON 22
#define FF_IDCT_NONE 24 /* Used by XvMC to extract IDCT coefficients with FF_IDCT_PERM_NONE */
#define FF_IDCT_SIMPLEAUTO 128

#define FF_THREAD_FRAME 1 ///< Decode more than one frame at once
#define FF_THREAD_SLICE 2 ///< Decode more than one part of a single frame at once
#define FF_LEVEL_UNKNOWN -99
#define FF_SUB_CHARENC_MODE_DO_NOTHING -1 ///< do nothing (demuxer outputs a stream supposed to be already in UTF-8, or the codec is bitmap for instance)
#define FF_SUB_CHARENC_MODE_AUTOMATIC 0   ///< libavcodec will select the mode itself
#define FF_SUB_CHARENC_MODE_PRE_DECODER 1 ///< the AVPacket data needs to be recoded to UTF-8 before being fed to the decoder, requires iconv
#define FF_SUB_CHARENC_MODE_IGNORE 2      ///< neither convert the subtitles, nor check them for valid UTF-8
#define FF_CODEC_PROPERTY_LOSSLESS 0x00000001
#define FF_CODEC_PROPERTY_CLOSED_CAPTIONS 0x00000002
#define FF_SUB_TEXT_FMT_ASS 0

#define INDENT 1
#define SHOW_VERSION 2
#define SHOW_CONFIG 4
#define SHOW_COPYRIGHT 8

#define FFMPEG_VERSION "N-99357-g14d6838638"

#define AV_CODEC_CAP_DRAW_HORIZ_BAND (1 << 0)
#define AV_CODEC_CAP_DR1 (1 << 1)
#define AV_CODEC_CAP_TRUNCATED (1 << 3)
#define AV_CODEC_CAP_DELAY (1 << 5)
#define AV_CODEC_CAP_SMALL_LAST_FRAME (1 << 6)
#define AV_CODEC_CAP_SUBFRAMES (1 << 8)
#define AV_CODEC_CAP_EXPERIMENTAL (1 << 9)
#define AV_CODEC_CAP_CHANNEL_CONF (1 << 10)
#define AV_CODEC_CAP_FRAME_THREADS (1 << 12)
#define AV_CODEC_CAP_SLICE_THREADS (1 << 13)
#define AV_CODEC_CAP_PARAM_CHANGE (1 << 14)
#define AV_CODEC_CAP_AUTO_THREADS (1 << 15)
#define AV_CODEC_CAP_VARIABLE_FRAME_SIZE (1 << 16)
#define AV_CODEC_CAP_AVOID_PROBING (1 << 17)
#define AV_CODEC_CAP_HARDWARE (1 << 18)
#define AV_CODEC_CAP_HYBRID (1 << 19)
#define AV_CODEC_CAP_ENCODER_REORDERED_OPAQUE (1 << 20)
#define AV_CODEC_CAP_ENCODER_FLUSH (1 << 21)

void *grow_array(void *array, int elem_size, int *size, int new_size);

#define GROW_ARRAY(array, nb_elems) \
    array = grow_array(array, sizeof(*array), &nb_elems, nb_elems + 1)

#define GET_PIX_FMT_NAME(pix_fmt) \
    const char *name = av_get_pix_fmt_name(pix_fmt);

#define GET_CODEC_NAME(id) \
    const char *name = avcodec_descriptor_get(id)->name;

#define GET_SAMPLE_FMT_NAME(sample_fmt) \
    const char *name = av_get_sample_fmt_name(sample_fmt)

#define GET_SAMPLE_RATE_NAME(rate) \
    char name[16];                 \
    snprintf(name, sizeof(name), "%d", rate);

#define GET_CH_LAYOUT_NAME(ch_layout) \
    char name[16];                    \
    snprintf(name, sizeof(name), "0x%" PRIx64, ch_layout);

#define GET_CH_LAYOUT_DESC(ch_layout) \
    char name[128];                   \
    av_get_channel_layout_string(name, sizeof(name), 0, ch_layout);

#define AV_LOG_SKIP_REPEATED 1

#define AV_LOG_PRINT_LEVEL 2

#define AV_VERSION_INT(a, b, c) ((a) << 16 | (b) << 8 | (c))
#define AV_VERSION_DOT(a, b, c) a##.##b##.##c
#define AV_VERSION(a, b, c) AV_VERSION_DOT(a, b, c)
#define AV_VERSION_MAJOR(a) ((a) >> 16)
#define AV_VERSION_MINOR(a) (((a)&0x00FF00) >> 8)
#define AV_VERSION_MICRO(a) ((a)&0xFF)

#define LIBAVUTIL_VERSION_MAJOR 56
#define LIBAVUTIL_VERSION_MINOR 60
#define LIBAVUTIL_VERSION_MICRO 100

#define LIBAVUTIL_VERSION_INT AV_VERSION_INT(LIBAVUTIL_VERSION_MAJOR, \
                                             LIBAVUTIL_VERSION_MINOR, \
                                             LIBAVUTIL_VERSION_MICRO)
#define LIBAVUTIL_VERSION AV_VERSION(LIBAVUTIL_VERSION_MAJOR, \
                                     LIBAVUTIL_VERSION_MINOR, \
                                     LIBAVUTIL_VERSION_MICRO)
#define LIBAVUTIL_BUILD LIBAVUTIL_VERSION_INT

#define LIBAVUTIL_IDENT "Lavu" AV_STRINGIFY(LIBAVUTIL_VERSION)

#define LIBAVCODEC_VERSION_MAJOR 58
#define LIBAVCODEC_VERSION_MINOR 111
#define LIBAVCODEC_VERSION_MICRO 101

#define LIBAVCODEC_VERSION_INT AV_VERSION_INT(LIBAVCODEC_VERSION_MAJOR, \
                                              LIBAVCODEC_VERSION_MINOR, \
                                              LIBAVCODEC_VERSION_MICRO)
#define LIBAVCODEC_VERSION AV_VERSION(LIBAVCODEC_VERSION_MAJOR, \
                                      LIBAVCODEC_VERSION_MINOR, \
                                      LIBAVCODEC_VERSION_MICRO)
#define LIBAVCODEC_BUILD LIBAVCODEC_VERSION_INT

#define LIBAVCODEC_IDENT "Lavc" AV_STRINGIFY(LIBAVCODEC_VERSION)

#define LIBAVFORMAT_VERSION_MAJOR 58
#define LIBAVFORMAT_VERSION_MINOR 62
#define LIBAVFORMAT_VERSION_MICRO 100

#define LIBAVFORMAT_VERSION_INT AV_VERSION_INT(LIBAVFORMAT_VERSION_MAJOR, \
                                               LIBAVFORMAT_VERSION_MINOR, \
                                               LIBAVFORMAT_VERSION_MICRO)
#define LIBAVFORMAT_VERSION AV_VERSION(LIBAVFORMAT_VERSION_MAJOR, \
                                       LIBAVFORMAT_VERSION_MINOR, \
                                       LIBAVFORMAT_VERSION_MICRO)
#define LIBAVFORMAT_BUILD LIBAVFORMAT_VERSION_INT

#define LIBAVFORMAT_IDENT "Lavf" AV_STRINGIFY(LIBAVFORMAT_VERSION)

#define LIBAVDEVICE_VERSION_MAJOR 58
#define LIBAVDEVICE_VERSION_MINOR 11
#define LIBAVDEVICE_VERSION_MICRO 102

#define LIBAVDEVICE_VERSION_INT AV_VERSION_INT(LIBAVDEVICE_VERSION_MAJOR, \
                                               LIBAVDEVICE_VERSION_MINOR, \
                                               LIBAVDEVICE_VERSION_MICRO)
#define LIBAVDEVICE_VERSION AV_VERSION(LIBAVDEVICE_VERSION_MAJOR, \
                                       LIBAVDEVICE_VERSION_MINOR, \
                                       LIBAVDEVICE_VERSION_MICRO)
#define LIBAVDEVICE_BUILD LIBAVDEVICE_VERSION_INT

#define LIBAVDEVICE_IDENT "Lavd" AV_STRINGIFY(LIBAVDEVICE_VERSION)

#define LIBAVFILTER_VERSION_MAJOR 7
#define LIBAVFILTER_VERSION_MINOR 87
#define LIBAVFILTER_VERSION_MICRO 100

#define LIBAVFILTER_VERSION_INT AV_VERSION_INT(LIBAVFILTER_VERSION_MAJOR, \
                                               LIBAVFILTER_VERSION_MINOR, \
                                               LIBAVFILTER_VERSION_MICRO)
#define LIBAVFILTER_VERSION AV_VERSION(LIBAVFILTER_VERSION_MAJOR, \
                                       LIBAVFILTER_VERSION_MINOR, \
                                       LIBAVFILTER_VERSION_MICRO)
#define LIBAVFILTER_BUILD LIBAVFILTER_VERSION_INT

#define LIBAVFILTER_IDENT "Lavfi" AV_STRINGIFY(LIBAVFILTER_VERSION)

#define LIBAVRESAMPLE_VERSION_MAJOR 4
#define LIBAVRESAMPLE_VERSION_MINOR 0
#define LIBAVRESAMPLE_VERSION_MICRO 0

#define LIBAVRESAMPLE_VERSION_INT AV_VERSION_INT(LIBAVRESAMPLE_VERSION_MAJOR, \
                                                 LIBAVRESAMPLE_VERSION_MINOR, \
                                                 LIBAVRESAMPLE_VERSION_MICRO)
#define LIBAVRESAMPLE_VERSION AV_VERSION(LIBAVRESAMPLE_VERSION_MAJOR, \
                                         LIBAVRESAMPLE_VERSION_MINOR, \
                                         LIBAVRESAMPLE_VERSION_MICRO)
#define LIBAVRESAMPLE_BUILD LIBAVRESAMPLE_VERSION_INT

#define LIBAVRESAMPLE_IDENT "Lavr" AV_STRINGIFY(LIBAVRESAMPLE_VERSION)

#define LIBSWSCALE_VERSION_MAJOR 5
#define LIBSWSCALE_VERSION_MINOR 8
#define LIBSWSCALE_VERSION_MICRO 100

#define LIBSWSCALE_VERSION_INT AV_VERSION_INT(LIBSWSCALE_VERSION_MAJOR, \
                                              LIBSWSCALE_VERSION_MINOR, \
                                              LIBSWSCALE_VERSION_MICRO)
#define LIBSWSCALE_VERSION AV_VERSION(LIBSWSCALE_VERSION_MAJOR, \
                                      LIBSWSCALE_VERSION_MINOR, \
                                      LIBSWSCALE_VERSION_MICRO)
#define LIBSWSCALE_BUILD LIBSWSCALE_VERSION_INT

#define LIBSWSCALE_IDENT "SwS" AV_STRINGIFY(LIBSWSCALE_VERSION)

#define LIBSWRESAMPLE_VERSION_MAJOR 3
#define LIBSWRESAMPLE_VERSION_MINOR 8
#define LIBSWRESAMPLE_VERSION_MICRO 100

#define LIBSWRESAMPLE_VERSION_INT AV_VERSION_INT(LIBSWRESAMPLE_VERSION_MAJOR, \
                                                 LIBSWRESAMPLE_VERSION_MINOR, \
                                                 LIBSWRESAMPLE_VERSION_MICRO)
#define LIBSWRESAMPLE_VERSION AV_VERSION(LIBSWRESAMPLE_VERSION_MAJOR, \
                                         LIBSWRESAMPLE_VERSION_MINOR, \
                                         LIBSWRESAMPLE_VERSION_MICRO)
#define LIBSWRESAMPLE_BUILD LIBSWRESAMPLE_VERSION_INT

#define LIBSWRESAMPLE_IDENT "SwR" AV_STRINGIFY(LIBSWRESAMPLE_VERSION)

#define LIBPOSTPROC_VERSION_MAJOR 55
#define LIBPOSTPROC_VERSION_MINOR 8
#define LIBPOSTPROC_VERSION_MICRO 100

#define LIBPOSTPROC_VERSION_INT AV_VERSION_INT(LIBPOSTPROC_VERSION_MAJOR, \
                                               LIBPOSTPROC_VERSION_MINOR, \
                                               LIBPOSTPROC_VERSION_MICRO)
#define LIBPOSTPROC_VERSION AV_VERSION(LIBPOSTPROC_VERSION_MAJOR, \
                                       LIBPOSTPROC_VERSION_MINOR, \
                                       LIBPOSTPROC_VERSION_MICRO)
#define LIBPOSTPROC_BUILD LIBPOSTPROC_VERSION_INT

#define LIBPOSTPROC_IDENT "postproc" AV_STRINGIFY(LIBPOSTPROC_VERSION)

#define AV_CODEC_PROP_INTRA_ONLY (1 << 0)
#define AV_CODEC_PROP_LOSSY (1 << 1)
#define AV_CODEC_PROP_LOSSLESS (1 << 2)
#define AV_CODEC_PROP_REORDER (1 << 3)
#define AV_CODEC_PROP_BITMAP_SUB (1 << 16)
#define AV_CODEC_PROP_TEXT_SUB (1 << 17)

#define AVFILTER_FLAG_DYNAMIC_INPUTS (1 << 0)
#define AVFILTER_FLAG_DYNAMIC_OUTPUTS (1 << 1)
#define AVFILTER_FLAG_SLICE_THREADS (1 << 2)
#define AVFILTER_FLAG_SUPPORT_TIMELINE_GENERIC (1 << 16)
#define AVFILTER_FLAG_SUPPORT_TIMELINE_INTERNAL (1 << 17)
#define AVFILTER_FLAG_SUPPORT_TIMELINE (AVFILTER_FLAG_SUPPORT_TIMELINE_GENERIC | AVFILTER_FLAG_SUPPORT_TIMELINE_INTERNAL)

#define AV_PIX_FMT_FLAG_BE (1 << 0)
#define AV_PIX_FMT_FLAG_PAL (1 << 1)
#define AV_PIX_FMT_FLAG_BITSTREAM (1 << 2)
#define AV_PIX_FMT_FLAG_HWACCEL (1 << 3)
#define AV_PIX_FMT_FLAG_PLANAR (1 << 4)
#define AV_PIX_FMT_FLAG_RGB (1 << 5)

#define AV_LOG_SKIP_REPEATED 1

#define AV_OPT_FLAG_ENCODING_PARAM 1 ///< a generic parameter which can be set by the user for muxing or encoding
#define AV_OPT_FLAG_DECODING_PARAM 2 ///< a generic parameter which can be set by the user for demuxing or decoding
#define AV_OPT_FLAG_AUDIO_PARAM 8
#define AV_OPT_FLAG_VIDEO_PARAM 16
#define AV_OPT_FLAG_SUBTITLE_PARAM 32
#define AV_OPT_FLAG_EXPORT 64
#define AV_OPT_FLAG_READONLY 128
#define AV_OPT_FLAG_BSF_PARAM (1 << 8)        ///< a generic parameter which can be set by the user for bit stream filtering
#define AV_OPT_FLAG_RUNTIME_PARAM (1 << 15)   ///< a generic parameter which can be set by the user at runtime
#define AV_OPT_FLAG_FILTERING_PARAM (1 << 16) ///< a generic parameter which can be set by the user for filtering
#define AV_OPT_FLAG_DEPRECATED (1 << 17)      ///< set if option is deprecated, users should refer to AVOption.help text for more information
#define AV_OPT_FLAG_CHILD_CONSTS (1 << 18)    ///< set if option constants can also reside in child objects



#define HAS_ARG 0x0001
#define OPT_BOOL 0x0002
#define OPT_EXPERT 0x0004
#define OPT_STRING 0x0008
#define OPT_VIDEO 0x0010
#define OPT_AUDIO 0x0020
#define OPT_INT 0x0080
#define OPT_FLOAT 0x0100
#define OPT_SUBTITLE 0x0200
#define OPT_INT64 0x0400
#define OPT_EXIT 0x0800
#define OPT_DATA 0x1000
#define OPT_PERFILE 0x2000 /* the option is per-file (currently ffmpeg-only). \
                              implied by OPT_OFFSET or OPT_SPEC */
#define OPT_OFFSET 0x4000  /* option is specified as an offset in a passed optctx */
#define OPT_SPEC 0x8000    /* option is to be stored in an array of SpecifierOpt.  \
                              Implies OPT_OFFSET. Next element after the offset is \
                              an int containing element count in the array. */
#define OPT_TIME 0x10000
#define OPT_DOUBLE 0x20000
#define OPT_INPUT 0x40000
#define OPT_OUTPUT 0x80000

#define AV_LOG_QUIET -8
#define AV_LOG_PANIC 0
#define AV_LOG_FATAL 8
#define AV_LOG_ERROR 16
#define AV_LOG_WARNING 24
#define AV_LOG_INFO 32
#define AV_LOG_VERBOSE 40
#define AV_LOG_DEBUG 48
#define AV_LOG_TRACE 56
#define AV_LOG_MAX_OFFSET (AV_LOG_TRACE - AV_LOG_QUIET)
#define AV_LOG_C(x) ((x) << 8)

static int report_file_level = AV_LOG_DEBUG;
int hide_banner = 0;


#define MV_DIR_FORWARD 1
#define MV_DIR_BACKWARD 2
#define MV_DIRECT 4     ///< bidirectional mode where the difference equals the MV of the last P/S/I-Frame (MPEG-4)
#define MV_TYPE_16X16 0 ///< 1 vector for the whole mb
#define MV_TYPE_8X8 1   ///< 4 vectors (H.263, MPEG-4 4MV)
#define MV_TYPE_16X8 2  ///< 2 vectors, one per 16x8 block
#define MV_TYPE_FIELD 3 ///< 2 vectors, one per field
#define MV_TYPE_DMV 4   ///< 2 vectors, special mpeg2 Dual Prime Vectors
#define UNI_AC_ENC_INDEX(run, level) ((run)*128 + (level))
#define VIDEO_FORMAT_COMPONENT 0
#define VIDEO_FORMAT_PAL 1
#define VIDEO_FORMAT_NTSC 2
#define VIDEO_FORMAT_SECAM 3
#define VIDEO_FORMAT_MAC 4
#define VIDEO_FORMAT_UNSPECIFIED 5
#define CHROMA_420 1
#define CHROMA_422 2
#define CHROMA_444 3
#define SLICE_OK 0
#define SLICE_ERROR -1
#define SLICE_END -2   ///<end marker found
#define SLICE_NOEND -3 ///<no end marker or error found but mb count exceeded

#define MAX_RUN 64
#define MAX_LEVEL 64
#define MAX_THREADS 32
#define MAX_B_FRAMES 16

#define AVFMT_FLAG_GENPTS 0x0001          ///< Generate missing pts even if it requires parsing future frames.
#define AVFMT_FLAG_IGNIDX 0x0002          ///< Ignore index.
#define AVFMT_FLAG_NONBLOCK 0x0004        ///< Do not block when reading packets from input.
#define AVFMT_FLAG_IGNDTS 0x0008          ///< Ignore DTS on frames that contain both DTS & PTS
#define AVFMT_FLAG_NOFILLIN 0x0010        ///< Do not infer any values from other values, just return what is stored in the container
#define AVFMT_FLAG_NOPARSE 0x0020         ///< Do not use AVParsers, you also must set AVFMT_FLAG_NOFILLIN as the fillin code works on frames and no parsing -> no frames. Also seeking to frames can not work if parsing to find frame boundaries has been disabled
#define AVFMT_FLAG_NOBUFFER 0x0040        ///< Do not buffer frames when possible
#define AVFMT_FLAG_CUSTOM_IO 0x0080       ///< The caller has supplied a custom AVIOContext, don't avio_close() it.
#define AVFMT_FLAG_DISCARD_CORRUPT 0x0100 ///< Discard frames marked corrupted
#define AVFMT_FLAG_FLUSH_PACKETS 0x0200   ///< Flush the AVIOContext every packet.
#define AVFMT_FLAG_BITEXACT 0x0400
#if FF_API_LAVF_MP4A_LATM
#define AVFMT_FLAG_MP4A_LATM 0x8000 ///< Deprecated, does nothing.
#endif
#define AVFMT_FLAG_SORT_DTS 0x10000 ///< try to interleave outputted packets by dts (using this flag can slow demuxing down)
#define AVFMT_FLAG_PRIV_OPT 0x20000 ///< Enable use of private options by delaying codec open (this could be made default once all code is converted)
#if FF_API_LAVF_KEEPSIDE_FLAG
#define AVFMT_FLAG_KEEP_SIDE_DATA 0x40000 ///< Deprecated, does nothing.
#endif
#define AVFMT_FLAG_FAST_SEEK 0x80000 ///< Enable fast, but inaccurate seeks for some formats
#define AVFMT_FLAG_SHORTEST 0x100000 ///< Stop muxing when the shortest stream stops.
#define AVFMT_FLAG_AUTO_BSF 0x200000 ///< Add bitstream filters as requested by the muxer

#define AVSTREAM_EVENT_FLAG_METADATA_UPDATED 0x0001 ///< The call resulted in updated metadata.
#define MAX_STD_TIMEBASES (30 * 12 + 30 + 3 + 6)
#define MAX_REORDER_DELAY 16

#if ARCH_IA64 // Limit static arrays to avoid gcc failing "short data segment overflowed"
#define MAX_MV 1024
#else
#define MAX_MV 4096
#endif
#define MAX_DMV (2 * MAX_MV)
#define ME_MAP_SIZE 64

#if AV_HAVE_BIGENDIAN
#define AV_PIX_FMT_NE(be, le) AV_PIX_FMT_##be
#else
#define AV_PIX_FMT_NE(be, le) AV_PIX_FMT_##le
#endif

#define AV_PIX_FMT_RGB32 AV_PIX_FMT_NE(ARGB, BGRA)
#define AV_PIX_FMT_RGB32_1 AV_PIX_FMT_NE(RGBA, ABGR)
#define AV_PIX_FMT_BGR32 AV_PIX_FMT_NE(ABGR, RGBA)
#define AV_PIX_FMT_BGR32_1 AV_PIX_FMT_NE(BGRA, ARGB)
#define AV_PIX_FMT_0RGB32 AV_PIX_FMT_NE(0RGB, BGR0)
#define AV_PIX_FMT_0BGR32 AV_PIX_FMT_NE(0BGR, RGB0)

#define AV_PIX_FMT_GRAY9 AV_PIX_FMT_NE(GRAY9BE, GRAY9LE)
#define AV_PIX_FMT_GRAY10 AV_PIX_FMT_NE(GRAY10BE, GRAY10LE)
#define AV_PIX_FMT_GRAY12 AV_PIX_FMT_NE(GRAY12BE, GRAY12LE)
#define AV_PIX_FMT_GRAY14 AV_PIX_FMT_NE(GRAY14BE, GRAY14LE)
#define AV_PIX_FMT_GRAY16 AV_PIX_FMT_NE(GRAY16BE, GRAY16LE)
#define AV_PIX_FMT_YA16 AV_PIX_FMT_NE(YA16BE, YA16LE)
#define AV_PIX_FMT_RGB48 AV_PIX_FMT_NE(RGB48BE, RGB48LE)
#define AV_PIX_FMT_RGB565 AV_PIX_FMT_NE(RGB565BE, RGB565LE)
#define AV_PIX_FMT_RGB555 AV_PIX_FMT_NE(RGB555BE, RGB555LE)
#define AV_PIX_FMT_RGB444 AV_PIX_FMT_NE(RGB444BE, RGB444LE)
#define AV_PIX_FMT_RGBA64 AV_PIX_FMT_NE(RGBA64BE, RGBA64LE)
#define AV_PIX_FMT_BGR48 AV_PIX_FMT_NE(BGR48BE, BGR48LE)
#define AV_PIX_FMT_BGR565 AV_PIX_FMT_NE(BGR565BE, BGR565LE)
#define AV_PIX_FMT_BGR555 AV_PIX_FMT_NE(BGR555BE, BGR555LE)
#define AV_PIX_FMT_BGR444 AV_PIX_FMT_NE(BGR444BE, BGR444LE)
#define AV_PIX_FMT_BGRA64 AV_PIX_FMT_NE(BGRA64BE, BGRA64LE)

#define AV_PIX_FMT_YUV420P9 AV_PIX_FMT_NE(YUV420P9BE, YUV420P9LE)
#define AV_PIX_FMT_YUV422P9 AV_PIX_FMT_NE(YUV422P9BE, YUV422P9LE)
#define AV_PIX_FMT_YUV444P9 AV_PIX_FMT_NE(YUV444P9BE, YUV444P9LE)
#define AV_PIX_FMT_YUV420P10 AV_PIX_FMT_NE(YUV420P10BE, YUV420P10LE)
#define AV_PIX_FMT_YUV422P10 AV_PIX_FMT_NE(YUV422P10BE, YUV422P10LE)
#define AV_PIX_FMT_YUV440P10 AV_PIX_FMT_NE(YUV440P10BE, YUV440P10LE)
#define AV_PIX_FMT_YUV444P10 AV_PIX_FMT_NE(YUV444P10BE, YUV444P10LE)
#define AV_PIX_FMT_YUV420P12 AV_PIX_FMT_NE(YUV420P12BE, YUV420P12LE)
#define AV_PIX_FMT_YUV422P12 AV_PIX_FMT_NE(YUV422P12BE, YUV422P12LE)
#define AV_PIX_FMT_YUV440P12 AV_PIX_FMT_NE(YUV440P12BE, YUV440P12LE)
#define AV_PIX_FMT_YUV444P12 AV_PIX_FMT_NE(YUV444P12BE, YUV444P12LE)
#define AV_PIX_FMT_YUV420P14 AV_PIX_FMT_NE(YUV420P14BE, YUV420P14LE)
#define AV_PIX_FMT_YUV422P14 AV_PIX_FMT_NE(YUV422P14BE, YUV422P14LE)
#define AV_PIX_FMT_YUV444P14 AV_PIX_FMT_NE(YUV444P14BE, YUV444P14LE)
#define AV_PIX_FMT_YUV420P16 AV_PIX_FMT_NE(YUV420P16BE, YUV420P16LE)
#define AV_PIX_FMT_YUV422P16 AV_PIX_FMT_NE(YUV422P16BE, YUV422P16LE)
#define AV_PIX_FMT_YUV444P16 AV_PIX_FMT_NE(YUV444P16BE, YUV444P16LE)

#define AV_PIX_FMT_GBRP9 AV_PIX_FMT_NE(GBRP9BE, GBRP9LE)
#define AV_PIX_FMT_GBRP10 AV_PIX_FMT_NE(GBRP10BE, GBRP10LE)
#define AV_PIX_FMT_GBRP12 AV_PIX_FMT_NE(GBRP12BE, GBRP12LE)
#define AV_PIX_FMT_GBRP14 AV_PIX_FMT_NE(GBRP14BE, GBRP14LE)
#define AV_PIX_FMT_GBRP16 AV_PIX_FMT_NE(GBRP16BE, GBRP16LE)
#define AV_PIX_FMT_GBRAP10 AV_PIX_FMT_NE(GBRAP10BE, GBRAP10LE)
#define AV_PIX_FMT_GBRAP12 AV_PIX_FMT_NE(GBRAP12BE, GBRAP12LE)
#define AV_PIX_FMT_GBRAP16 AV_PIX_FMT_NE(GBRAP16BE, GBRAP16LE)

#define AV_PIX_FMT_BAYER_BGGR16 AV_PIX_FMT_NE(BAYER_BGGR16BE, BAYER_BGGR16LE)
#define AV_PIX_FMT_BAYER_RGGB16 AV_PIX_FMT_NE(BAYER_RGGB16BE, BAYER_RGGB16LE)
#define AV_PIX_FMT_BAYER_GBRG16 AV_PIX_FMT_NE(BAYER_GBRG16BE, BAYER_GBRG16LE)
#define AV_PIX_FMT_BAYER_GRBG16 AV_PIX_FMT_NE(BAYER_GRBG16BE, BAYER_GRBG16LE)

#define AV_PIX_FMT_GBRPF32 AV_PIX_FMT_NE(GBRPF32BE, GBRPF32LE)
#define AV_PIX_FMT_GBRAPF32 AV_PIX_FMT_NE(GBRAPF32BE, GBRAPF32LE)

#define AV_PIX_FMT_GRAYF32 AV_PIX_FMT_NE(GRAYF32BE, GRAYF32LE)

#define AV_PIX_FMT_YUVA420P9 AV_PIX_FMT_NE(YUVA420P9BE, YUVA420P9LE)
#define AV_PIX_FMT_YUVA422P9 AV_PIX_FMT_NE(YUVA422P9BE, YUVA422P9LE)
#define AV_PIX_FMT_YUVA444P9 AV_PIX_FMT_NE(YUVA444P9BE, YUVA444P9LE)
#define AV_PIX_FMT_YUVA420P10 AV_PIX_FMT_NE(YUVA420P10BE, YUVA420P10LE)
#define AV_PIX_FMT_YUVA422P10 AV_PIX_FMT_NE(YUVA422P10BE, YUVA422P10LE)
#define AV_PIX_FMT_YUVA444P10 AV_PIX_FMT_NE(YUVA444P10BE, YUVA444P10LE)
#define AV_PIX_FMT_YUVA422P12 AV_PIX_FMT_NE(YUVA422P12BE, YUVA422P12LE)
#define AV_PIX_FMT_YUVA444P12 AV_PIX_FMT_NE(YUVA444P12BE, YUVA444P12LE)
#define AV_PIX_FMT_YUVA420P16 AV_PIX_FMT_NE(YUVA420P16BE, YUVA420P16LE)
#define AV_PIX_FMT_YUVA422P16 AV_PIX_FMT_NE(YUVA422P16BE, YUVA422P16LE)
#define AV_PIX_FMT_YUVA444P16 AV_PIX_FMT_NE(YUVA444P16BE, YUVA444P16LE)

#define AV_PIX_FMT_XYZ12 AV_PIX_FMT_NE(XYZ12BE, XYZ12LE)
#define AV_PIX_FMT_NV20 AV_PIX_FMT_NE(NV20BE, NV20LE)
#define AV_PIX_FMT_AYUV64 AV_PIX_FMT_NE(AYUV64BE, AYUV64LE)
#define AV_PIX_FMT_P010 AV_PIX_FMT_NE(P010BE, P010LE)
#define AV_PIX_FMT_P016 AV_PIX_FMT_NE(P016BE, P016LE)

#define AV_NOPTS_VALUE ((int64_t)UINT64_C(0x8000000000000000))
#define AV_TIME_BASE 1000000
#define AV_TIME_BASE_Q \
    (AVRational) { 1, AV_TIME_BASE }

#define AV_HAVE_BIGENDIAN 0
#define AV_HAVE_FAST_UNALIGNED 1

#define FF_FDEBUG_TS 0x0001
#define AVFMT_EVENT_FLAG_METADATA_UPDATED 0x0001 ///< The call resulted in updated metadata.
#define AVFMT_AVOID_NEG_TS_AUTO -1               ///< Enabled when required by target format
#define AVFMT_AVOID_NEG_TS_MAKE_NON_NEGATIVE 1   ///< Shift timestamps so they are non negative
#define AVFMT_AVOID_NEG_TS_MAKE_ZERO 2           ///< Shift timestamps so that they start at 0

#ifdef _WIN64
typedef unsigned __int64 size_t;
typedef __int64 ptrdiff_t;
typedef __int64 intptr_t;
#else
typedef unsigned int size_t;
typedef int ptrdiff_t;
typedef int intptr_t;
#endif

#define atomic_store(object, desired) \
    do                                \
    {                                 \
        *(object) = (desired);        \
    } while (0)

#define atomic_store_explicit(object, desired, order) \
    atomic_store(object, desired)

#define atomic_load(object) \
    (*(object))

#define atomic_load_explicit(object, order) \
    atomic_load(object)

#define ATOMIC_FLAG_INIT 0

#define ATOMIC_VAR_INIT(value) (value)

#define atomic_init(obj, value) \
    do                          \
    {                           \
        *(obj) = (value);       \
    } while (0)

#define kill_dependency(y) ((void)0)

#define atomic_thread_fence(order) \
    ((void)0)

#define atomic_signal_fence(order) \
    ((void)0)

#define atomic_is_lock_free(obj) 0

#define FFMIN(a, b) ((a) > (b) ? (b) : (a))
#define AV_BUFFER_FLAG_READONLY (1 << 0)

#define SWS_PARAM_DEFAULT 123456

#define RED_DITHER "0*8"
#define GREEN_DITHER "1*8"
#define BLUE_DITHER "2*8"
#define Y_COEFF "3*8"
#define VR_COEFF "4*8"
#define UB_COEFF "5*8"
#define VG_COEFF "6*8"
#define UG_COEFF "7*8"
#define Y_OFFSET "8*8"
#define U_OFFSET "9*8"
#define V_OFFSET "10*8"
#define LUM_MMX_FILTER_OFFSET "11*8"
#define CHR_MMX_FILTER_OFFSET "11*8+4*4*" AV_STRINGIFY(MAX_FILTER_SIZE)
#define DSTW_OFFSET "11*8+4*4*" AV_STRINGIFY(MAX_FILTER_SIZE) "*2"
#define ESP_OFFSET "11*8+4*4*" AV_STRINGIFY(MAX_FILTER_SIZE) "*2+8"
#define VROUNDER_OFFSET "11*8+4*4*" AV_STRINGIFY(MAX_FILTER_SIZE) "*2+16"
#define U_TEMP "11*8+4*4*" AV_STRINGIFY(MAX_FILTER_SIZE) "*2+24"
#define V_TEMP "11*8+4*4*" AV_STRINGIFY(MAX_FILTER_SIZE) "*2+32"
#define Y_TEMP "11*8+4*4*" AV_STRINGIFY(MAX_FILTER_SIZE) "*2+40"
#define ALP_MMX_FILTER_OFFSET "11*8+4*4*" AV_STRINGIFY(MAX_FILTER_SIZE) "*2+48"
#define UV_OFF_PX "11*8+4*4*" AV_STRINGIFY(MAX_FILTER_SIZE) "*3+48"
#define UV_OFF_BYTE "11*8+4*4*" AV_STRINGIFY(MAX_FILTER_SIZE) "*3+56"
#define DITHER16 "11*8+4*4*" AV_STRINGIFY(MAX_FILTER_SIZE) "*3+64"
#define DITHER32 "11*8+4*4*" AV_STRINGIFY(MAX_FILTER_SIZE) "*3+80"
#define DITHER32_INT (11 * 8 + 4 * 4 * MAX_FILTER_SIZE * 3 + 80) // value equal to above, used for checking that the struct hasn't been changed by mistake
#define XYZ_GAMMA (2.6f)
#define RGB_GAMMA (2.2f)

#define AVPROBE_SCORE_RETRY (AVPROBE_SCORE_MAX / 4)
#define AVPROBE_SCORE_STREAM_RETRY (AVPROBE_SCORE_MAX / 4 - 1)

#define AVPROBE_SCORE_EXTENSION 50 ///< score for file extension
#define AVPROBE_SCORE_MIME 75      ///< score for file mime type
#define AVPROBE_SCORE_MAX 100      ///< maximum score

#if CACHED_BITSTREAM_READER
#define MIN_CACHE_BITS 64
#elif defined LONG_BITSTREAM_READER
#define MIN_CACHE_BITS 32
#else
#define MIN_CACHE_BITS 25
#endif

#if !CACHED_BITSTREAM_READER

#define OPEN_READER_NOSIZE(name, gb)         \
    unsigned int name##_index = (gb)->index; \
    unsigned int av_unused name##_cache

#if UNCHECKED_BITSTREAM_READER
#define OPEN_READER(name, gb) OPEN_READER_NOSIZE(name, gb)

#define BITS_AVAILABLE(name, gb) 1
#else
#define OPEN_READER(name, gb)     \
    OPEN_READER_NOSIZE(name, gb); \
    unsigned int name##_size_plus8 = (gb)->size_in_bits_plus8

#define BITS_AVAILABLE(name, gb) name##_index < name##_size_plus8
#endif

#define CLOSE_READER(name, gb) (gb)->index = name##_index

#ifdef LONG_BITSTREAM_READER

#define UPDATE_CACHE_LE(name, gb) name##_cache = \
                                      AV_RL64((gb)->buffer + (name##_index >> 3)) >> (name##_index & 7)

#define UPDATE_CACHE_BE(name, gb) name##_cache = \
                                      AV_RB64((gb)->buffer + (name##_index >> 3)) >> (32 - (name##_index & 7))

#else

#define UPDATE_CACHE_LE(name, gb) name##_cache = \
                                      AV_RL32((gb)->buffer + (name##_index >> 3)) >> (name##_index & 7)

#define UPDATE_CACHE_BE(name, gb) name##_cache = \
                                      AV_RB32((gb)->buffer + (name##_index >> 3)) << (name##_index & 7)

#endif

#ifdef BITSTREAM_READER_LE

#define UPDATE_CACHE(name, gb) UPDATE_CACHE_LE(name, gb)

#define SKIP_CACHE(name, gb, num) name##_cache >>= (num)

#else

#define UPDATE_CACHE(name, gb) UPDATE_CACHE_BE(name, gb)

#define SKIP_CACHE(name, gb, num) name##_cache <<= (num)

#endif

#if UNCHECKED_BITSTREAM_READER
#define SKIP_COUNTER(name, gb, num) name##_index += (num)
#else
#define SKIP_COUNTER(name, gb, num) \
    name##_index = FFMIN(name##_size_plus8, name##_index + (num))
#endif

#define BITS_LEFT(name, gb) ((int)((gb)->size_in_bits - name##_index))

#define SKIP_BITS(name, gb, num)     \
    do                               \
    {                                \
        SKIP_CACHE(name, gb, num);   \
        SKIP_COUNTER(name, gb, num); \
    } while (0)

#define LAST_SKIP_BITS(name, gb, num) SKIP_COUNTER(name, gb, num)

#define SHOW_UBITS_LE(name, gb, num) zero_extend(name##_cache, num)
#define SHOW_SBITS_LE(name, gb, num) sign_extend(name##_cache, num)

#define SHOW_UBITS_BE(name, gb, num) NEG_USR32(name##_cache, num)
#define SHOW_SBITS_BE(name, gb, num) NEG_SSR32(name##_cache, num)

#ifdef BITSTREAM_READER_LE
#define SHOW_UBITS(name, gb, num) SHOW_UBITS_LE(name, gb, num)
#define SHOW_SBITS(name, gb, num) SHOW_SBITS_LE(name, gb, num)
#else
#define SHOW_UBITS(name, gb, num) SHOW_UBITS_BE(name, gb, num)
#define SHOW_SBITS(name, gb, num) SHOW_SBITS_BE(name, gb, num)
#endif

#define GET_CACHE(name, gb) ((uint32_t)name##_cache)

#endif

#define OFFSET(x) offsetof(AADemuxContext, x)
#define ADTS_HEADER_SIZE 7

#define ID3v2_HEADER_SIZE 10
#define ID3v2_DEFAULT_MAGIC "ID3"
#define ID3v2_FLAG_DATALEN 0x0001
#define ID3v2_FLAG_UNSYNCH 0x0002
#define ID3v2_FLAG_ENCRYPTION 0x0004
#define ID3v2_FLAG_COMPRESSION 0x0008

#define FF_QUIT_EVENT (SDL_USEREVENT + 2)


#define AV_CODEC_FLAG2_FAST (1 << 0)
#define AVFMT_NOFILE 0x0001
#define AVFMT_NEEDNUMBER 0x0002    /**< Needs '%d' in filename. */
#define AVFMT_SHOW_IDS 0x0008      /**< Show format stream IDs numbers. */
#define AVFMT_GLOBALHEADER 0x0040  /**< Format wants global header. */
#define AVFMT_NOTIMESTAMPS 0x0080  /**< Format does not need / have any timestamps. */
#define AVFMT_GENERIC_INDEX 0x0100 /**< Use generic index building code. */
#define AVFMT_TS_DISCONT 0x0200    /**< Format allows timestamp discontinuities. Note, muxers always require valid (monotone) timestamps */
#define AVFMT_VARIABLE_FPS 0x0400  /**< Format allows variable fps. */
#define AVFMT_NODIMENSIONS 0x0800  /**< Format does not need width/height */
#define AVFMT_NOSTREAMS 0x1000     /**< Format does not require any streams */
#define AVFMT_NOBINSEARCH 0x2000   /**< Format does not allow to fall back on binary search via read_timestamp */
#define AVFMT_NOGENSEARCH 0x4000   /**< Format does not allow to fall back on generic search */
#define AVFMT_NO_BYTE_SEEK 0x8000  /**< Format does not allow seeking by bytes */
#define AVFMT_ALLOW_FLUSH 0x10000  /**< Format allows flushing. If not set, the muxer will not receive a NULL packet in the write_packet function. */
#define AVFMT_TS_NONSTRICT 0x20000 /**< Format does not require strictly     \
                                        increasing timestamps, but they must \
                                        still be monotonic */
#define AVFMT_TS_NEGATIVE 0x40000  /**< Format allows muxing negative               \
                                        timestamps. If not set the timestamp        \
                                        will be shifted in av_write_frame and       \
                                        av_interleaved_write_frame so they          \
                                        start from 0.                               \
                                        The user or muxer can override this through \
                                        AVFormatContext.avoid_negative_ts           \
                                        */

#define AVFMT_SEEK_TO_PTS 0x4000000 /**< Seeking is based on PTS */

#define AV_DISPOSITION_DEFAULT 0x0001
#define AV_DISPOSITION_DUB 0x0002
#define AV_DISPOSITION_ORIGINAL 0x0004
#define AV_DISPOSITION_COMMENT 0x0008
#define AV_DISPOSITION_LYRICS 0x0010
#define AV_DISPOSITION_KARAOKE 0x0020

#define AV_DISPOSITION_FORCED 0x0040
#define AV_DISPOSITION_HEARING_IMPAIRED 0x0080 /**< stream for hearing impaired audiences */
#define AV_DISPOSITION_VISUAL_IMPAIRED 0x0100  /**< stream for visual impaired audiences */
#define AV_DISPOSITION_CLEAN_EFFECTS 0x0200    /**< stream without voice */

#define AV_DISPOSITION_ATTACHED_PIC 0x0400

#define AV_DISPOSITION_TIMED_THUMBNAILS 0x0800

#define AV_CH_FRONT_LEFT 0x00000001
#define AV_CH_FRONT_RIGHT 0x00000002
#define AV_CH_FRONT_CENTER 0x00000004
#define AV_CH_LOW_FREQUENCY 0x00000008
#define AV_CH_BACK_LEFT 0x00000010
#define AV_CH_BACK_RIGHT 0x00000020
#define AV_CH_FRONT_LEFT_OF_CENTER 0x00000040
#define AV_CH_FRONT_RIGHT_OF_CENTER 0x00000080
#define AV_CH_BACK_CENTER 0x00000100
#define AV_CH_SIDE_LEFT 0x00000200
#define AV_CH_SIDE_RIGHT 0x00000400
#define AV_CH_TOP_CENTER 0x00000800
#define AV_CH_TOP_FRONT_LEFT 0x00001000
#define AV_CH_TOP_FRONT_CENTER 0x00002000
#define AV_CH_TOP_FRONT_RIGHT 0x00004000
#define AV_CH_TOP_BACK_LEFT 0x00008000
#define AV_CH_TOP_BACK_CENTER 0x00010000
#define AV_CH_TOP_BACK_RIGHT 0x00020000
#define AV_CH_STEREO_LEFT 0x20000000  ///< Stereo downmix.
#define AV_CH_STEREO_RIGHT 0x40000000 ///< See AV_CH_STEREO_LEFT.
#define AV_CH_WIDE_LEFT 0x0000000080000000ULL
#define AV_CH_WIDE_RIGHT 0x0000000100000000ULL
#define AV_CH_SURROUND_DIRECT_LEFT 0x0000000200000000ULL
#define AV_CH_SURROUND_DIRECT_RIGHT 0x0000000400000000ULL
#define AV_CH_LOW_FREQUENCY_2 0x0000000800000000ULL
#define AV_CH_TOP_SIDE_LEFT 0x0000001000000000ULL
#define AV_CH_TOP_SIDE_RIGHT 0x0000002000000000ULL
#define AV_CH_BOTTOM_FRONT_CENTER 0x0000004000000000ULL
#define AV_CH_BOTTOM_FRONT_LEFT 0x0000008000000000ULL
#define AV_CH_BOTTOM_FRONT_RIGHT 0x0000010000000000ULL

/** Channel mask value used for AVCodecContext.request_channel_layout
    to indicate that the user requests the channel order of the decoder output
    to be the native codec channel order. */
#define AV_CH_LAYOUT_NATIVE 0x8000000000000000ULL
#define AV_CH_LAYOUT_MONO (AV_CH_FRONT_CENTER)
#define AV_CH_LAYOUT_STEREO (AV_CH_FRONT_LEFT | AV_CH_FRONT_RIGHT)
#define AV_CH_LAYOUT_2POINT1 (AV_CH_LAYOUT_STEREO | AV_CH_LOW_FREQUENCY)
#define AV_CH_LAYOUT_2_1 (AV_CH_LAYOUT_STEREO | AV_CH_BACK_CENTER)
#define AV_CH_LAYOUT_SURROUND (AV_CH_LAYOUT_STEREO | AV_CH_FRONT_CENTER)
#define AV_CH_LAYOUT_3POINT1 (AV_CH_LAYOUT_SURROUND | AV_CH_LOW_FREQUENCY)
#define AV_CH_LAYOUT_4POINT0 (AV_CH_LAYOUT_SURROUND | AV_CH_BACK_CENTER)
#define AV_CH_LAYOUT_4POINT1 (AV_CH_LAYOUT_4POINT0 | AV_CH_LOW_FREQUENCY)
#define AV_CH_LAYOUT_2_2 (AV_CH_LAYOUT_STEREO | AV_CH_SIDE_LEFT | AV_CH_SIDE_RIGHT)
#define AV_CH_LAYOUT_QUAD (AV_CH_LAYOUT_STEREO | AV_CH_BACK_LEFT | AV_CH_BACK_RIGHT)
#define AV_CH_LAYOUT_5POINT0 (AV_CH_LAYOUT_SURROUND | AV_CH_SIDE_LEFT | AV_CH_SIDE_RIGHT)
#define AV_CH_LAYOUT_5POINT1 (AV_CH_LAYOUT_5POINT0 | AV_CH_LOW_FREQUENCY)
#define AV_CH_LAYOUT_5POINT0_BACK (AV_CH_LAYOUT_SURROUND | AV_CH_BACK_LEFT | AV_CH_BACK_RIGHT)
#define AV_CH_LAYOUT_5POINT1_BACK (AV_CH_LAYOUT_5POINT0_BACK | AV_CH_LOW_FREQUENCY)
#define AV_CH_LAYOUT_6POINT0 (AV_CH_LAYOUT_5POINT0 | AV_CH_BACK_CENTER)
#define AV_CH_LAYOUT_6POINT0_FRONT (AV_CH_LAYOUT_2_2 | AV_CH_FRONT_LEFT_OF_CENTER | AV_CH_FRONT_RIGHT_OF_CENTER)
#define AV_CH_LAYOUT_HEXAGONAL (AV_CH_LAYOUT_5POINT0_BACK | AV_CH_BACK_CENTER)
#define AV_CH_LAYOUT_6POINT1 (AV_CH_LAYOUT_5POINT1 | AV_CH_BACK_CENTER)
#define AV_CH_LAYOUT_6POINT1_BACK (AV_CH_LAYOUT_5POINT1_BACK | AV_CH_BACK_CENTER)
#define AV_CH_LAYOUT_6POINT1_FRONT (AV_CH_LAYOUT_6POINT0_FRONT | AV_CH_LOW_FREQUENCY)
#define AV_CH_LAYOUT_7POINT0 (AV_CH_LAYOUT_5POINT0 | AV_CH_BACK_LEFT | AV_CH_BACK_RIGHT)
#define AV_CH_LAYOUT_7POINT0_FRONT (AV_CH_LAYOUT_5POINT0 | AV_CH_FRONT_LEFT_OF_CENTER | AV_CH_FRONT_RIGHT_OF_CENTER)
#define AV_CH_LAYOUT_7POINT1 (AV_CH_LAYOUT_5POINT1 | AV_CH_BACK_LEFT | AV_CH_BACK_RIGHT)
#define AV_CH_LAYOUT_7POINT1_WIDE (AV_CH_LAYOUT_5POINT1 | AV_CH_FRONT_LEFT_OF_CENTER | AV_CH_FRONT_RIGHT_OF_CENTER)
#define AV_CH_LAYOUT_7POINT1_WIDE_BACK (AV_CH_LAYOUT_5POINT1_BACK | AV_CH_FRONT_LEFT_OF_CENTER | AV_CH_FRONT_RIGHT_OF_CENTER)
#define AV_CH_LAYOUT_OCTAGONAL (AV_CH_LAYOUT_5POINT0 | AV_CH_BACK_LEFT | AV_CH_BACK_CENTER | AV_CH_BACK_RIGHT)
#define AV_CH_LAYOUT_HEXADECAGONAL (AV_CH_LAYOUT_OCTAGONAL | AV_CH_WIDE_LEFT | AV_CH_WIDE_RIGHT | AV_CH_TOP_BACK_LEFT | AV_CH_TOP_BACK_RIGHT | AV_CH_TOP_BACK_CENTER | AV_CH_TOP_FRONT_CENTER | AV_CH_TOP_FRONT_LEFT | AV_CH_TOP_FRONT_RIGHT)
#define AV_CH_LAYOUT_STEREO_DOWNMIX (AV_CH_STEREO_LEFT | AV_CH_STEREO_RIGHT)
#define AV_CH_LAYOUT_22POINT2 (AV_CH_LAYOUT_5POINT1_BACK | AV_CH_FRONT_LEFT_OF_CENTER | AV_CH_FRONT_RIGHT_OF_CENTER | AV_CH_BACK_CENTER | AV_CH_LOW_FREQUENCY_2 | AV_CH_SIDE_LEFT | AV_CH_SIDE_RIGHT | AV_CH_TOP_FRONT_LEFT | AV_CH_TOP_FRONT_RIGHT | AV_CH_TOP_FRONT_CENTER | AV_CH_TOP_CENTER | AV_CH_TOP_BACK_LEFT | AV_CH_TOP_BACK_RIGHT | AV_CH_TOP_SIDE_LEFT | AV_CH_TOP_SIDE_RIGHT | AV_CH_TOP_BACK_CENTER | AV_CH_BOTTOM_FRONT_CENTER | AV_CH_BOTTOM_FRONT_LEFT | AV_CH_BOTTOM_FRONT_RIGHT)


#define LINE_SZ 1024
#define NB_LEVELS 8

#ifndef attribute_align_arg
#if ARCH_X86_32 && AV_GCC_VERSION_AT_LEAST(4, 2)
#define attribute_align_arg __attribute__((force_align_arg_pointer))
#else
#define attribute_align_arg
#endif
#endif

#define AV_CODEC_FLAG_DROPCHANGED (1 << 5)
#define AVERROR_INPUT_CHANGED (-0x636e6701) ///< Input changed between calls. Reconfiguration is required. (can be OR-ed with AVERROR_OUTPUT_CHANGED)
#define MKTAG(a, b, c, d) ((a) | ((b) << 8) | ((c) << 16) | ((unsigned)(d) << 24))

#if EDOM > 0
#define AVERROR(e) (-(e))   ///< Returns a negative error code from a POSIX error code, to return from library functions.
#define AVUNERROR(e) (-(e)) ///< Returns a POSIX error code from a library function error return value.
#else
#define AVERROR(e) (e)
#define AVUNERROR(e) (e)
#endif

#define AV_INPUT_BUFFER_PADDING_SIZE 64
#define AV_INPUT_BUFFER_MIN_SIZE 16384

#define av_assert0(cond)                                                 \
    do                                                                   \
    {                                                                    \
        if (!(cond))                                                     \
        {                                                                \
            av_log(NULL, AV_LOG_PANIC, "Assertion %s failed at %s:%d\n", \
                   AV_STRINGIFY(cond), __FILE__, __LINE__);              \
            abort();                                                     \
        }                                                                \
    } while (0)

#if defined(ASSERT_LEVEL) && ASSERT_LEVEL > 0
#define av_assert1(cond) av_assert0(cond)
#else
#define av_assert1(cond) ((void)0)
#endif

#if defined(ASSERT_LEVEL) && ASSERT_LEVEL > 1
#define av_assert2(cond) av_assert0(cond)
#define av_assert2_fpu() av_assert0_fpu()
#else
#define av_assert2(cond) ((void)0)
#define av_assert2_fpu() ((void)0)
#endif

#define SRWLOCK_INIT RTL_SRWLOCK_INIT
#define AV_MUTEX_INITIALIZER PTHREAD_MUTEX_INITIALIZER
// #define PTHREAD_MUTEX_INITIALIZER SRWLOCK_INIT
#define AVMutex pthread_mutex_t

#define ff_mutex_init pthread_mutex_init
#define ff_mutex_lock pthread_mutex_lock
#define ff_mutex_unlock pthread_mutex_unlock
#define ff_mutex_destroy pthread_mutex_destroy
#define pthread_mutex_lock strict_pthread_mutex_lock

#define BUFFER_FLAG_REALLOCATABLE (1 << 0)
#define ALIGN (HAVE_AVX512 ? 64 : (HAVE_AVX ? 32 : 16))

#define WHITESPACES " \n\t\r"

#define AV_STRINGIFY(s) AV_TOSTRING(s)
#define AV_TOSTRING(s) #s

#ifndef M_PI
#define M_PI 3.14159265358979323846 /* pi */
#endif

#define CONV_FP(x) ((double)(x)) / (1 << 16)

// double to fixed point
#define CONV_DB(x) (int32_t)((x) * (1 << 16))

#define AV_IS_INPUT_DEVICE(category)                         \
    (((category) == AV_CLASS_CATEGORY_DEVICE_VIDEO_INPUT) || \
     ((category) == AV_CLASS_CATEGORY_DEVICE_AUDIO_INPUT) || \
     ((category) == AV_CLASS_CATEGORY_DEVICE_INPUT))

#define AV_IS_OUTPUT_DEVICE(category)                         \
    (((category) == AV_CLASS_CATEGORY_DEVICE_VIDEO_OUTPUT) || \
     ((category) == AV_CLASS_CATEGORY_DEVICE_AUDIO_OUTPUT) || \
     ((category) == AV_CLASS_CATEGORY_DEVICE_OUTPUT))


#define EPERM 1
#define ENOENT 2
#define ESRCH 3
#define EINTR 4
#define EIO 5
#define ENXIO 6
#define E2BIG 7
#define ENOEXEC 8
#define EBADF 9
#define ECHILD 10
#define EAGAIN 11
#define ENOMEM 12
#define EACCES 13
#define EFAULT 14
#define EBUSY 16
#define EEXIST 17
#define EXDEV 18
#define ENODEV 19
#define ENOTDIR 20
#define EISDIR 21
#define ENFILE 23
#define EMFILE 24
#define ENOTTY 25
#define EFBIG 27
#define ENOSPC 28
#define ESPIPE 29
#define EROFS 30
#define EMLINK 31
#define EPIPE 32
#define EDOM 33
#define EDEADLK 36
#define ENAMETOOLONG 38
#define ENOLCK 39
#define ENOSYS 40
#define ENOTEMPTY 41

#define RY_IDX 0
#define GY_IDX 1
#define BY_IDX 2
#define RU_IDX 3
#define GU_IDX 4
#define BU_IDX 5
#define RV_IDX 6
#define GV_IDX 7
#define BV_IDX 8
#define RGB2YUV_SHIFT 15

#define FFERRTAG(a, b, c, d) (-(int)MKTAG(a, b, c, d))

#define AVERROR_BSF_NOT_FOUND FFERRTAG(0xF8, 'B', 'S', 'F')      ///< Bitstream filter not found
#define AVERROR_BUG FFERRTAG('B', 'U', 'G', '!')                 ///< Internal bug, also see AVERROR_BUG2
#define AVERROR_BUFFER_TOO_SMALL FFERRTAG('B', 'U', 'F', 'S')    ///< Buffer too small
#define AVERROR_DECODER_NOT_FOUND FFERRTAG(0xF8, 'D', 'E', 'C')  ///< Decoder not found
#define AVERROR_DEMUXER_NOT_FOUND FFERRTAG(0xF8, 'D', 'E', 'M')  ///< Demuxer not found
#define AVERROR_ENCODER_NOT_FOUND FFERRTAG(0xF8, 'E', 'N', 'C')  ///< Encoder not found
#define AVERROR_EOF FFERRTAG('E', 'O', 'F', ' ')                 ///< End of file
#define AVERROR_EXIT FFERRTAG('E', 'X', 'I', 'T')                ///< Immediate exit was requested; the called function should not be restarted
#define AVERROR_EXTERNAL FFERRTAG('E', 'X', 'T', ' ')            ///< Generic error in an external library
#define AVERROR_FILTER_NOT_FOUND FFERRTAG(0xF8, 'F', 'I', 'L')   ///< Filter not found
#define AVERROR_INVALIDDATA FFERRTAG('I', 'N', 'D', 'A')         ///< Invalid data found when processing input
#define AVERROR_MUXER_NOT_FOUND FFERRTAG(0xF8, 'M', 'U', 'X')    ///< Muxer not found
#define AVERROR_OPTION_NOT_FOUND FFERRTAG(0xF8, 'O', 'P', 'T')   ///< Option not found
#define AVERROR_PATCHWELCOME FFERRTAG('P', 'A', 'W', 'E')        ///< Not yet implemented in FFmpeg, patches welcome
#define AVERROR_PROTOCOL_NOT_FOUND FFERRTAG(0xF8, 'P', 'R', 'O') ///< Protocol not found
#define AVERROR_STREAM_NOT_FOUND FFERRTAG(0xF8, 'S', 'T', 'R')   ///< Stream not found

#define AV_DICT_MATCH_CASE 1      /**< Only get an entry with exact-case key match. Only relevant in av_dict_get(). */
#define AV_DICT_IGNORE_SUFFIX 2   /**< Return first entry in a dictionary whose first part corresponds to the search key, \
                                       ignoring the suffix of the found key string. Only relevant in av_dict_get(). */
#define AV_DICT_DONT_STRDUP_KEY 4 /**< Take ownership of a key that's been \
                                       allocated with av_malloc() or another memory allocation function. */
#define AV_DICT_DONT_STRDUP_VAL 8 /**< Take ownership of a value that's been \
                                       allocated with av_malloc() or another memory allocation function. */
#define AV_DICT_DONT_OVERWRITE 16 ///< Don't overwrite existing entries.
#define AV_DICT_APPEND 32         /**< If the entry already exists, append to it.  Note that no \
                                    delimiter is added, the strings are simply concatenated. */
#define AV_DICT_MULTIKEY 64       /**< Allow to store several equal keys in the dictionary */

#define AV_OPT_SEARCH_CHILDREN (1 << 0) /**< Search in possible children of the given object first. */




#define AUDIO_U8 0x0008     /**< Unsigned 8-bit samples */
#define AUDIO_S8 0x8008     /**< Signed 8-bit samples */
#define AUDIO_U16LSB 0x0010 /**< Unsigned 16-bit samples */
#define AUDIO_S16LSB 0x8010 /**< Signed 16-bit samples */
#define AUDIO_U16MSB 0x1010 /**< As above, but big-endian byte order */
#define AUDIO_S16MSB 0x9010 /**< As above, but big-endian byte order */
#define AUDIO_U16 AUDIO_U16LSB
#define AUDIO_S16 AUDIO_S16LSB

#define AUDIO_S32LSB 0x8020 /**< 32-bit integer samples */
#define AUDIO_S32MSB 0x9020 /**< As above, but big-endian byte order */
#define AUDIO_S32 AUDIO_S32LSB

#define AUDIO_F32LSB 0x8120 /**< 32-bit floating point samples */
#define AUDIO_F32MSB 0x9120 /**< As above, but big-endian byte order */
#define AUDIO_F32 AUDIO_F32LSB

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#define AUDIO_U16SYS AUDIO_U16LSB
#define AUDIO_S16SYS AUDIO_S16LSB
#define AUDIO_S32SYS AUDIO_S32LSB
#define AUDIO_F32SYS AUDIO_F32LSB
#else
#define AUDIO_U16SYS AUDIO_U16MSB
#define AUDIO_S16SYS AUDIO_S16MSB
#define AUDIO_S32SYS AUDIO_S32MSB
#define AUDIO_F32SYS AUDIO_F32MSB
#endif

#if CONFIG_SMALL
#define NULL_IF_CONFIG_SMALL(x) NULL
#else
#define NULL_IF_CONFIG_SMALL(x) x
#endif

#define FFABS(a) ((a) >= 0 ? (a) : (-(a)))
#define FFSIGN(a) ((a) > 0 ? 1 : -1)

#define MAX_PCE_SIZE 320

#if defined(_MSC_VER) || defined(__MWERKS__) || defined(__BORLANDC__)
#ifdef __BORLANDC__
#pragma nopackwarning
#endif
#pragma pack(pop)
#endif /* Compiler needs structure packing set */

#define AVSEEK_FLAG_BACKWARD 1 ///< seek backward
#define AVSEEK_FLAG_BYTE 2     ///< seeking based on position in bytes
#define AVSEEK_FLAG_ANY 4      ///< seek to any frame, even non-keyframes
#define AVSEEK_FLAG_FRAME 8    ///< seeking based on frame number

#define FF_PAD_STRUCTURE(name, size, ...)                                  \
    struct ff_pad_helper_##name                                            \
    {                                                                      \
        __VA_ARGS__                                                        \
    };                                                                     \
    typedef struct name                                                    \
    {                                                                      \
        __VA_ARGS__                                                        \
        char reserved_padding[size - sizeof(struct ff_pad_helper_##name)]; \
    } name;

FF_PAD_STRUCTURE(AVBPrint, 1024,
                 char *str;         /**< string so far */
                 unsigned len;      /**< length so far */
                 unsigned size;     /**< allocated memory */
                 unsigned size_max; /**< maximum allocated memory */
                 char reserved_internal_buffer[1];)

#define AV_BPRINT_SIZE_UNLIMITED ((unsigned)-1)
#define AV_BPRINT_SIZE_AUTOMATIC 1
#define AV_BPRINT_SIZE_COUNT_ONLY 0

#define FFSWAP(type, a, b) \
    do                     \
    {                      \
        type SWAP_tmp = b; \
        b = a;             \
        a = SWAP_tmp;      \
    } while (0)
#define FF_ARRAY_ELEMS(a) (sizeof(a) / sizeof((a)[0]))

#define AV_OPT_SEARCH_CHILDREN (1 << 0) /**< Search in possible children of the given object first. */
#define AV_OPT_SEARCH_FAKE_OBJ (1 << 1)
#define AV_OPT_ALLOW_NULL (1 << 2)
#define AV_OPT_MULTI_COMPONENT_RANGE (1 << 12)

#define atomic_fetch_sub_explicit(object, operand, order) \
    atomic_fetch_sub(object, operand)


#define av_bprint_room(buf) ((buf)->size - FFMIN((buf)->len, (buf)->size))
#define av_bprint_is_allocated(buf) ((buf)->str != (buf)->reserved_internal_buffer)

typedef Uint16 SDL_AudioFormat;

#if SDL_DYNAMIC_API
#include "dynapi/SDL_dynapi_overrides.h"
/* force DECLSPEC and SDLCALL off...it's all internal symbols now.
   These will have actual #defines during SDL_dynapi.c only */
#define DECLSPEC
#define SDLCALL
#endif

#define ID3v2_PRIV_METADATA_PREFIX "id3v2_priv."

#define ADTS_HEADER_SIZE 7

#define AV_ONCE_INIT PTHREAD_ONCE_INIT

#define AVOnce pthread_once_t
#define ENC AV_OPT_FLAG_ENCODING_PARAM

// #define OFFSET(obj) offsetof(ADTSContext, obj)

#define AVIO_SEEKABLE_NORMAL (1 << 0)

#define AA_MAGIC 1469084982 /* this identifies an audible .aa file */
#define MAX_CODEC_SECOND_SIZE 3982
#define MAX_TOC_ENTRIES 16
#define MAX_DICTIONARY_ENTRIES 128
#define TEA_BLOCK_SIZE 8
#define CHAPTER_HEADER_SIZE 8
#define TIMEPREC 1000
#define MP3_FRAME_SIZE 104

#define AVINDEX_KEYFRAME 0x0001
#define AVINDEX_DISCARD_FRAME 0x0002

#define AC3_MAX_CODED_FRAME_SIZE 3840 /* in bytes */
#define EAC3_MAX_CHANNELS 16          /**< maximum number of channels in EAC3 */
#define AC3_MAX_CHANNELS 7            /**< maximum number of channels, including coupling channel */
#define CPL_CH 0                      /**< coupling channel index */

#define AC3_MAX_COEFS   256
#define AC3_BLOCK_SIZE  256
#define AC3_MAX_BLOCKS    6
#define AC3_FRAME_SIZE (AC3_MAX_BLOCKS * 256)
#define AC3_WINDOW_SIZE (AC3_BLOCK_SIZE * 2)
#define AC3_CRITICAL_BANDS 50
#define AC3_MAX_CPL_BANDS  18

#define CONV_FP(x) ((double) (x)) / (1 << 16)

// double to fixed point
#define CONV_DB(x) (int32_t) ((x) * (1 << 16))

#define AVSEEK_SIZE 0x10000

#define AVSEEK_FORCE 0x20000

#define FETCH_MODIFY(opname, op)                                            \
static inline intptr_t atomic_fetch_ ## opname(intptr_t *object, intptr_t operand) \
{                                                                    \
    intptr_t ret;                                                    \
    ret = *object;                                                   \
    *object = *object op operand;                                    \
    return ret;                                                      \
}

FETCH_MODIFY(add, +)
FETCH_MODIFY(sub, -)
FETCH_MODIFY(or,  |)
FETCH_MODIFY(xor, ^)
FETCH_MODIFY(and, &)
#undef FETCH_MODIFY

#define atomic_fetch_add_explicit(object, operand, order) \
    atomic_fetch_add(object, operand)

#define RSHIFT(a,b) ((a) > 0 ? ((a) + ((1<<(b))>>1))>>(b) : ((a) + ((1<<(b))>>1)-1)>>(b))
/* assume b>0 */
#define ROUNDED_DIV(a,b) (((a)>=0 ? (a) + ((b)>>1) : (a) - ((b)>>1))/(b))
/* Fast a/(1<<b) rounded toward +inf. Assume a>=0 and b>=0 */
#define AV_CEIL_RSHIFT(a,b) (!av_builtin_constant_p(b) ? -((-(a)) >> (b)) \
                                                       : ((a) + (1<<(b)) - 1) >> (b))
/* Backwards compat. */
#define FF_CEIL_RSHIFT AV_CEIL_RSHIFT

#define FFUDIV(a,b) (((a)>0 ?(a):(a)-(b)+1) / (b))
#define FFUMOD(a,b) ((a)-(b)*FFUDIV(a,b))

#define FFABS(a) ((a) >= 0 ? (a) : (-(a)))
#define FFSIGN(a) ((a) > 0 ? 1 : -1)

#define FFNABS(a) ((a) <= 0 ? (a) : (-(a)))

#define FFDIFFSIGN(x,y) (((x)>(y)) - ((x)<(y)))

#define FFMAX(a,b) ((a) > (b) ? (a) : (b))
#define FFMAX3(a,b,c) FFMAX(FFMAX(a,b),c)
#define FFMIN(a,b) ((a) > (b) ? (b) : (a))
#define FFMIN3(a,b,c) FFMIN(FFMIN(a,b),c)

#define FFSWAP(type,a,b) do{type SWAP_tmp= b; b= a; a= SWAP_tmp;}while(0)
#define FF_ARRAY_ELEMS(a) (sizeof(a) / sizeof((a)[0]))

#if AV_GCC_VERSION_AT_LEAST(4,3) || defined(__clang__)
#    define av_cold __attribute__((cold))
#else
#    define av_cold
#endif


#   if ARCH_X86_64
#       define ff_ctzll(v) _tzcnt_u64(v)
#   else
#       define ff_ctzll ff_ctzll_x86
static av_always_inline av_const int ff_ctzll_x86(long long v)
{
    return ((uint32_t)v == 0) ? _tzcnt_u32((uint32_t)(v >> 32)) + 32 : _tzcnt_u32((uint32_t)v);
}
#   endif

#define av_opt_set_int_list(obj, name, val, term, flags) \
    (av_int_list_length(val, term) > INT_MAX / sizeof(*(val)) ? \
     AVERROR(EINVAL) : \
     av_opt_set_bin(obj, name, (const uint8_t *)(val), \
                    av_int_list_length(val, term) * sizeof(*(val)), flags))

#define FORMATS_UNREF(ref, list)                                   \
do {                                                               \
    int idx = -1;                                                  \
                                                                   \
    if (!*ref)                                                     \
        return;                                                    \
                                                                   \
    FIND_REF_INDEX(ref, idx);                                      \
                                                                   \
    if (idx >= 0) {                                                \
        memmove((*ref)->refs + idx, (*ref)->refs + idx + 1,        \
            sizeof(*(*ref)->refs) * ((*ref)->refcount - idx - 1)); \
        --(*ref)->refcount;                                        \
    }                                                              \
    if (!(*ref)->refcount) {                                       \
        av_free((*ref)->list);                                     \
        av_free((*ref)->refs);                                     \
        av_free(*ref);                                             \
    }                                                              \
    *ref = NULL;                                                   \
} while (0)

#define CHECK_VIDEO_PARAM_CHANGE(s, c, width, height, format, pts)\
    if (c->w != width || c->h != height || c->pix_fmt != format) {\
        av_log(s, AV_LOG_INFO, "filter context - w: %d h: %d fmt: %d, incoming frame - w: %d h: %d fmt: %d pts_time: %s\n",\
               c->w, c->h, c->pix_fmt, width, height, format, av_ts2timestr(pts, &s->outputs[0]->time_base));\
        av_log(s, AV_LOG_WARNING, "Changing video frame properties on the fly is not supported by all filters.\n");\
    }

#define CHECK_AUDIO_PARAM_CHANGE(s, c, srate, ch_layout, ch_count, format, pts)\
    if (c->sample_fmt != format || c->sample_rate != srate ||\
        c->channel_layout != ch_layout || c->channels != ch_count) {\
        av_log(s, AV_LOG_INFO, "filter context - fmt: %s r: %d layout: %"PRIX64" ch: %d, incoming frame - fmt: %s r: %d layout: %"PRIX64" ch: %d pts_time: %s\n",\
               av_get_sample_fmt_name(c->sample_fmt), c->sample_rate, c->channel_layout, c->channels,\
               av_get_sample_fmt_name(format), srate, ch_layout, ch_count, av_ts2timestr(pts, &s->outputs[0]->time_base));\
        av_log(s, AV_LOG_ERROR, "Changing audio frame properties on the fly is not supported.\n");\
        return AVERROR(EINVAL);\
    }

#define AV_CODEC_FLAG_UNALIGNED       (1 <<  0)
#define AV_CODEC_FLAG_QSCALE          (1 <<  1)
#define AV_CODEC_FLAG_4MV             (1 <<  2)
#define AV_CODEC_FLAG_OUTPUT_CORRUPT  (1 <<  3)
#define AV_CODEC_FLAG_QPEL            (1 <<  4)
#define AV_CODEC_FLAG_DROPCHANGED     (1 <<  5)
#define AV_CODEC_FLAG_PASS1           (1 <<  9)
#define AV_CODEC_FLAG_PASS2           (1 << 10)
#define AV_CODEC_FLAG_LOOP_FILTER     (1 << 11)
#define AV_CODEC_FLAG_GRAY            (1 << 13)
#define AV_CODEC_FLAG_PSNR            (1 << 15)
#define AV_CODEC_FLAG_TRUNCATED       (1 << 16)
#define AV_CODEC_FLAG_INTERLACED_DCT  (1 << 18)
#define AV_CODEC_FLAG_LOW_DELAY       (1 << 19)
#define AV_CODEC_FLAG_GLOBAL_HEADER   (1 << 22)

#define AV_CODEC_FLAG_BITEXACT        (1 << 23)
#define AV_CODEC_FLAG_AC_PRED         (1 << 24)
#define AV_CODEC_FLAG_INTERLACED_ME   (1 << 29)
#define AV_CODEC_FLAG_CLOSED_GOP      (1U << 31)
#define AV_CODEC_FLAG2_FAST           (1 <<  0)
#define AV_CODEC_FLAG2_NO_OUTPUT      (1 <<  2)
#define AV_CODEC_FLAG2_LOCAL_HEADER   (1 <<  3)
#define AV_CODEC_FLAG2_DROP_FRAME_TIMECODE (1 << 13)
#define AV_CODEC_FLAG2_CHUNKS         (1 << 15)
#define AV_CODEC_FLAG2_IGNORE_CROP    (1 << 16)
#define AV_CODEC_FLAG2_SHOW_ALL       (1 << 22)
#define AV_CODEC_FLAG2_EXPORT_MVS     (1 << 28)
#define AV_CODEC_FLAG2_SKIP_MANUAL    (1 << 29)
#define AV_CODEC_FLAG2_RO_FLUSH_NOOP  (1 << 30)

#define media_type_string av_get_media_type_string
#define AV_ERROR_MAX_STRING_SIZE 64
#define av_err2str(errnum) \
    av_make_error_string((char[AV_ERROR_MAX_STRING_SIZE]){0}, AV_ERROR_MAX_STRING_SIZE, errnum)

#define ff_thread_once(control, routine) pthread_once(control, routine)

#define GET_UTF8(val, GET_BYTE, ERROR)\
    val= (GET_BYTE);\
    {\
        uint32_t top = (val & 128) >> 1;\
        if ((val & 0xc0) == 0x80 || val >= 0xFE)\
            {ERROR}\
        while (val & top) {\
            unsigned int tmp = (GET_BYTE) - 128;\
            if(tmp>>6)\
                {ERROR}\
            val= (val<<6) + tmp;\
            top <<= 5;\
        }\
        val &= (top << 1) - 1;\
    }

    #define PUT_UTF16(val, tmp, PUT_16BIT)\
    {\
        uint32_t in = val;\
        if (in < 0x10000) {\
            tmp = in;\
            PUT_16BIT\
        } else {\
            tmp = 0xD800 | ((in - 0x10000) >> 10);\
            PUT_16BIT\
            tmp = 0xDC00 | ((in - 0x10000) & 0x3FF);\
            PUT_16BIT\
        }\
    }\

#if HAVE_LIBC_MSVCRT
#include <crtversion.h>
#if defined(_VC_CRT_MAJOR_VERSION) && _VC_CRT_MAJOR_VERSION < 14
#pragma comment(linker, "/include:" EXTERN_PREFIX "avpriv_strtod")
#pragma comment(linker, "/include:" EXTERN_PREFIX "avpriv_snprintf")
#endif

#define avpriv_open ff_open
#define avpriv_tempfile ff_tempfile
#define PTRDIFF_SPECIFIER "Id"
#define SIZE_SPECIFIER "Iu"
#else
#define PTRDIFF_SPECIFIER "td"
#define SIZE_SPECIFIER "zu"
#endif

#define SWS_FAST_BILINEAR     1
#define SWS_BILINEAR          2
#define SWS_BICUBIC           4
#define SWS_X                 8
#define SWS_POINT          0x10
#define SWS_AREA           0x20
#define SWS_BICUBLIN       0x40
#define SWS_GAUSS          0x80
#define SWS_SINC          0x100
#define SWS_LANCZOS       0x200
#define SWS_SPLINE        0x400

#define SWS_SRC_V_CHR_DROP_MASK     0x30000
#define SWS_SRC_V_CHR_DROP_SHIFT    16

#define SWS_PARAM_DEFAULT           123456

#define SWS_PRINT_INFO              0x1000

//the following 3 flags are not completely implemented
//internal chrominance subsampling info
#define SWS_FULL_CHR_H_INT    0x2000
//input subsampling info
#define SWS_FULL_CHR_H_INP    0x4000
#define SWS_DIRECT_BGR        0x8000
#define SWS_ACCURATE_RND      0x40000
#define SWS_BITEXACT          0x80000
#define SWS_ERROR_DIFFUSION  0x800000

#define SWS_MAX_REDUCE_CUTOFF 0.002

#define SWS_CS_ITU709         1
#define SWS_CS_FCC            4
#define SWS_CS_ITU601         5
#define SWS_CS_ITU624         5
#define SWS_CS_SMPTE170M      5
#define SWS_CS_SMPTE240M      7
#define SWS_CS_DEFAULT        5
#define SWS_CS_BT2020         9

#if FF_API_PSEUDOPAL
#define FF_PSEUDOPAL AV_PIX_FMT_FLAG_PSEUDOPAL
#else
#define FF_PSEUDOPAL 0
#endif

#define AV_PIX_FMT_FLAG_BE           (1 << 0)
#define AV_PIX_FMT_FLAG_PAL          (1 << 1)
#define AV_PIX_FMT_FLAG_BITSTREAM    (1 << 2)
#define AV_PIX_FMT_FLAG_HWACCEL      (1 << 3)
#define AV_PIX_FMT_FLAG_PLANAR       (1 << 4)
#define AV_PIX_FMT_FLAG_RGB          (1 << 5)

#if FF_API_PSEUDOPAL
#define AV_PIX_FMT_FLAG_PSEUDOPAL    (1 << 6)
#endif

#define AV_PIX_FMT_FLAG_ALPHA        (1 << 7)

#define AV_PIX_FMT_FLAG_BAYER        (1 << 8)

#define AV_PIX_FMT_FLAG_FLOAT        (1 << 9)

#define RETCODE_USE_CASCADE -12345

#define AVFILTER_DEFINE_CLASS(fname)            \
    static const AVClass fname##_class = {      \
        .class_name = #fname,                   \
        .item_name  = av_default_item_name,     \
        .option     = fname##_options,          \
        .version    = LIBAVUTIL_VERSION_INT,    \
        .category   = AV_CLASS_CATEGORY_FILTER, \
    }

#define DEFINE_OPTIONS(filt_name, FLAGS)                                                                                \
static const AVOption filt_name##_options[] = {                                                                         \
    { "action", "set action", OFFSET(action), AV_OPT_TYPE_INT, {.i64=ACTION_START}, 0, NB_ACTION-1, FLAGS, "action" },  \
        { "start", "start timer",  0, AV_OPT_TYPE_CONST, {.i64=ACTION_START}, INT_MIN, INT_MAX, FLAGS, "action" },      \
        { "stop",  "stop timer",   0, AV_OPT_TYPE_CONST, {.i64=ACTION_STOP},  INT_MIN, INT_MAX, FLAGS, "action" },      \
    { NULL }                                                                                                            \
}
#define AVFILTER_THREAD_SLICE (1 << 0)

#define AV_BUFFERSINK_FLAG_PEEK 1
#define AV_BUFFERSINK_FLAG_NO_REQUEST 2

#if CONFIG_AVDEVICE
#define CMDUTILS_COMMON_OPTIONS_AVDEVICE                                                                       \
    {"sources", OPT_EXIT | HAS_ARG, {.func_arg = show_sources}, "list sources of the input device", "device"}, \
        {"sinks", OPT_EXIT | HAS_ARG, {.func_arg = show_sinks}, "list sinks of the output device", "device"},
#else
#define CMDUTILS_COMMON_OPTIONS_AVDEVICE
#endif
#define CMDUTILS_COMMON_OPTIONS                                                                                       \
    {"buildconf", OPT_EXIT, {.func_arg = show_buildconf}, "show build configuration"},                                \
        {"formats", OPT_EXIT, {.func_arg = show_formats}, "show available formats"},                                  \
        {"muxers", OPT_EXIT, {.func_arg = show_muxers}, "show available muxers"},                                     \
        {"demuxers", OPT_EXIT, {.func_arg = show_demuxers}, "show available demuxers"},                               \
        {"devices", OPT_EXIT, {.func_arg = show_devices}, "show available devices"},                                  \
        {"codecs", OPT_EXIT, {.func_arg = show_codecs}, "show available codecs"},                                     \
        {"decoders", OPT_EXIT, {.func_arg = show_decoders}, "show available decoders"},                               \
        {"encoders", OPT_EXIT, {.func_arg = show_encoders}, "show available encoders"},                               \
        {"bsfs", OPT_EXIT, {.func_arg = show_bsfs}, "show available bit stream filters"},                             \
        {"protocols", OPT_EXIT, {.func_arg = show_protocols}, "show available protocols"},                            \
        {"filters", OPT_EXIT, {.func_arg = show_filters}, "show available filters"},                                  \
        {"pix_fmts", OPT_EXIT, {.func_arg = show_pix_fmts}, "show available pixel formats"},                          \
        {"layouts", OPT_EXIT, {.func_arg = show_layouts}, "show standard channel layouts"},                           \
        {"sample_fmts", OPT_EXIT, {.func_arg = show_sample_fmts}, "show available audio sample formats"},             \
        {"colors", OPT_EXIT, {.func_arg = show_colors}, "show available color names"},                                \
        {"loglevel", HAS_ARG, {.func_arg = opt_loglevel}, "set logging level", "loglevel"},                           \
        {"v", HAS_ARG, {.func_arg = opt_loglevel}, "set logging level", "loglevel"},                                  \
        {"report", 0, {.func_arg = opt_report}, "generate a report"},                                                 \
        {"max_alloc", HAS_ARG, {.func_arg = opt_max_alloc}, "set maximum size of a single allocated block", "bytes"}, \
        {"cpuflags", HAS_ARG | OPT_EXPERT, {.func_arg = opt_cpuflags}, "force specific cpu flags", "flags"},          \
        {"hide_banner", OPT_BOOL | OPT_EXPERT, {&hide_banner}, "do not show program banner", "hide_banner"},          \
        CMDUTILS_COMMON_OPTIONS_AVDEVICE


typedef struct AVComponentDescriptor
{
    int plane;
    int step;
    int offset;
    int shift;
    int depth;
} AVComponentDescriptor;

typedef struct AVPixFmtDescriptor
{
    const char *name;
    uint8_t nb_components; ///< The number of components each pixel has, (1-4)
    uint8_t log2_chroma_w;
    uint8_t log2_chroma_h;
    uint64_t flags;
    AVComponentDescriptor comp[4];
    const char *alias;
} AVPixFmtDescriptor;

typedef struct SpecifierOpt
{
    char *specifier; /**< stream/chapter/program/... specifier */
    union
    {
        uint8_t *str;
        int i;
        int64_t i64;
        uint64_t ui64;
        float f;
        double dbl;
    } u;
} SpecifierOpt;

typedef struct OptionDef
{
    const char *name;
    int flags;

    union
    {
        void *dst_ptr;
        int (*func_arg)(void *, const char *, const char *);
        size_t off;
    } u;
    const char *help;
    const char *argname;
} OptionDef;

typedef struct Option
{
    const OptionDef *opt;
    const char *key;
    const char *val;
} Option;

typedef struct OptionGroupDef
{
    const char *name;
    const char *sep;
    int flags;
} OptionGroupDef;

typedef struct AVDictionary AVDictionary;

typedef struct OptionGroup
{
    const OptionGroupDef *group_def;
    const char *arg;

    Option *opts;
    int nb_opts;

    AVDictionary *codec_opts;
    AVDictionary *format_opts;
    AVDictionary *resample_opts;
    AVDictionary *sws_dict;
    AVDictionary *swr_opts;
} OptionGroup;

typedef struct OptionGroupList
{
    const OptionGroupDef *group_def;
    OptionGroup *groups;
    int nb_groups;
} OptionGroupList;

typedef struct OptionParseContext
{
    OptionGroup global_opts;
    OptionGroupList *groups;
    int nb_groups;
    OptionGroup cur_group;
} OptionParseContext;

void show_banner(int argc, char **argv, const OptionDef *options);

int show_version(void *optctx, const char *opt, const char *arg);

int show_buildconf(void *optctx, const char *opt, const char *arg);
int show_formats(void *optctx, const char *opt, const char *arg);
int show_muxers(void *optctx, const char *opt, const char *arg);
int show_demuxers(void *optctx, const char *opt, const char *arg);
int show_devices(void *optctx, const char *opt, const char *arg);
#if CONFIG_AVDEVICE
int show_sinks(void *optctx, const char *opt, const char *arg);
int show_sources(void *optctx, const char *opt, const char *arg);
#endif
int show_codecs(void *optctx, const char *opt, const char *arg);
int show_decoders(void *optctx, const char *opt, const char *arg);
int show_encoders(void *optctx, const char *opt, const char *arg);
int show_filters(void *optctx, const char *opt, const char *arg);
int show_bsfs(void *optctx, const char *opt, const char *arg);
int show_protocols(void *optctx, const char *opt, const char *arg);
int show_pix_fmts(void *optctx, const char *opt, const char *arg);
int show_layouts(void *optctx, const char *opt, const char *arg);
int show_sample_fmts(void *optctx, const char *opt, const char *arg);
int show_colors(void *optctx, const char *opt, const char *arg);

void log_callback_help(void *ptr, int level, const char *fmt, va_list vl);
int opt_cpuflags(void *optctx, const char *opt, const char *arg);
int opt_default(void *optctx, const char *opt, const char *arg);
int opt_loglevel(void *optctx, const char *opt, const char *arg);
int opt_report(void *optctx, const char *opt, const char *arg);
int opt_max_alloc(void *optctx, const char *opt, const char *arg);
int opt_codec_debug(void *optctx, const char *opt, const char *arg);
int opt_timelimit(void *optctx, const char *opt, const char *arg);


typedef void(__stdcall *SDL_AudioCallback)(void *userdata, Uint8 *stream,
                                           int len);



typedef struct SDL_cond SDL_cond;

struct SDL_mutex;
typedef struct SDL_mutex SDL_mutex;

struct SDL_Thread;
typedef struct SDL_Thread SDL_Thread;

struct SDL_Texture;
typedef struct SDL_Texture SDL_Texture;

typedef intptr_t atomic_uint;

struct AVBuffer
{
    uint8_t *data; /**< data described by this buffer */
    int size;      /**< size of data in bytes */
    atomic_uint refcount;
    void (*free)(void *opaque, uint8_t *data);
    void *opaque;
    int flags;
    int flags_internal;
};

typedef struct AVBuffer AVBuffer;

typedef struct AVBufferRef
{
    AVBuffer *buffer;
    uint8_t *data;
    int size;
} AVBufferRef;

typedef struct AVFrameSideData
{
    enum AVFrameSideDataType type;
    uint8_t *data;
    int size;
    AVDictionary *metadata;
    AVBufferRef *buf;
} AVFrameSideData;

typedef struct AVFrame
{
    uint8_t *data[AV_NUM_DATA_POINTERS];
    int linesize[AV_NUM_DATA_POINTERS];
    uint8_t **extended_data;
    int width, height;
    int nb_samples;
    int format;
    int key_frame;
    enum AVPictureType pict_type;
    AVRational sample_aspect_ratio;
    int64_t pts;
    int64_t pkt_dts;
    int coded_picture_number;
    int display_picture_number;
    int quality;
    void *opaque;
    int repeat_pict;
    int interlaced_frame;
    int top_field_first;
    int palette_has_changed;
    int64_t reordered_opaque;
    int sample_rate;
    uint64_t channel_layout;
    AVBufferRef *buf[AV_NUM_DATA_POINTERS];
    AVBufferRef **extended_buf;
    int nb_extended_buf;
    AVFrameSideData **side_data;
    int nb_side_data;
    int flags;
    enum AVColorRange color_range;
    enum AVColorPrimaries color_primaries;
    enum AVColorTransferCharacteristic color_trc;
    enum AVColorSpace colorspace;
    enum AVChromaLocation chroma_location;
    int64_t best_effort_timestamp;
    int64_t pkt_pos;
    int64_t pkt_duration;
    AVDictionary *metadata;
    int decode_error_flags;
    int channels;
    int pkt_size;
    AVBufferRef *hw_frames_ctx;
    AVBufferRef *opaque_ref;
    size_t crop_top;
    size_t crop_bottom;
    size_t crop_left;
    size_t crop_right;
    AVBufferRef *private_ref;
} AVFrame;

typedef enum Predictor
{
    LEFT = 0,
    PLANE,
    MEDIAN,
} Predictor;

typedef struct AVPacketSideData
{
    uint8_t *data;
    int size;
    enum AVPacketSideDataType type;
} AVPacketSideData;

typedef struct AVPacket
{
    AVBufferRef *buf;
    int64_t pts;
    int64_t dts;
    uint8_t *data;
    int size;
    int stream_index;
    int flags;
    AVPacketSideData *side_data;
    int side_data_elems;
    int64_t duration;
    int64_t pos; ///< byte position in stream, -1 if unknown
} AVPacket;

typedef struct MyAVPacketList
{
    AVPacket pkt;
    struct MyAVPacketList *next;
    int serial;
} MyAVPacketList;

typedef struct PacketQueue
{
    MyAVPacketList *first_pkt, *last_pkt;
    int nb_packets;
    int size;
    int64_t duration;
    int abort_request;
    int serial;
    SDL_mutex *mutex;
    SDL_cond *cond;
} PacketQueue;

typedef struct AudioParams
{
    int freq;
    int channels;
    int64_t channel_layout;
    enum AVSampleFormat fmt;
    int frame_size;
    int bytes_per_sec;
} AudioParams;

typedef struct Clock
{
    double pts;       /* clock base */
    double pts_drift; /* clock base minus time at which we updated the clock */
    double last_updated;
    double speed;
    int serial; /* clock is based on a packet with this serial */
    int paused;
    int *queue_serial; /* pointer to the current packet queue serial, used for obsolete clock detection */
} Clock;

typedef struct AVSubtitleRect
{
    int x;         ///< top left corner  of pict, undefined when pict is not set
    int y;         ///< top left corner  of pict, undefined when pict is not set
    int w;         ///< width            of pict, undefined when pict is not set
    int h;         ///< height           of pict, undefined when pict is not set
    int nb_colors; ///< number of colors in pict, undefined when pict is not set
    uint8_t *data[4];
    int linesize[4];
    enum AVSubtitleType type;
    char *text; ///< 0 terminated plain UTF-8 text
    char *ass;
    int flags;
} AVSubtitleRect;

typedef struct AVSubtitle
{
    uint16_t format;             /* 0 = graphics */
    uint32_t start_display_time; /* relative to packet pts, in ms */
    uint32_t end_display_time;   /* relative to packet pts, in ms */
    unsigned num_rects;
    AVSubtitleRect **rects;
    int64_t pts; ///< Same as packet pts, in AV_TIME_BASE
} AVSubtitle;

typedef struct Frame
{
    AVFrame *frame;
    AVSubtitle sub;
    int serial;
    double pts;      /* presentation timestamp for the frame */
    double duration; /* estimated duration of the frame */
    int64_t pos;     /* byte position of the frame in the input file */
    int width;
    int height;
    int format;
    AVRational sar;
    int uploaded;
    int flip_v;
} Frame;

typedef struct FrameQueue
{
    Frame queue[FRAME_QUEUE_SIZE];
    int rindex;
    int windex;
    int size;
    int max_size;
    int keep_last;
    int rindex_shown;
    SDL_mutex *mutex;
    SDL_cond *cond;
    PacketQueue *pktq;
} FrameQueue;

typedef struct AVOptionRange
{
    const char *str;
    double value_min, value_max;
    double component_min, component_max;
    int is_range;
} AVOptionRange;

typedef struct AVOptionRanges
{
    AVOptionRange **range;
    int nb_ranges;
    int nb_components;
} AVOptionRanges;

typedef struct AVClass
{
    const char *class_name;
    const char *(*item_name)(void *ctx);
    const struct AVOption *option;
    int version;
    int log_level_offset_offset;
    int parent_log_context_offset;
    void *(*child_next)(void *obj, void *prev);
    const struct AVClass *(*child_class_next)(const struct AVClass *prev);
    AVClassCategory category;
    AVClassCategory (*get_category)(void *ctx);
    int (*query_ranges)(struct AVOptionRanges **, void *obj, const char *key, int flags);
    const struct AVClass* (*child_class_iterate)(void **iter);

} AVClass;

typedef struct AVOption
{
    const char *name;
    const char *help;
    int offset;
    enum AVOptionType type;
    union
    {
        int64_t i64;
        double dbl;
        const char *str;
        /* TODO those are unused now */
        AVRational q;
    } default_val;
    double min; ///< minimum valid value for the option
    double max; ///< maximum valid value for the option

    int flags;
    const char *unit;
} AVOption;



#define AV_CODEC_ID_IFF_BYTERUN1 AV_CODEC_ID_IFF_ILBM
#define AV_CODEC_ID_H265 AV_CODEC_ID_HEVC

typedef struct AVProfile
{
    int profile;
    const char *name; ///< short name for the profile
} AVProfile;

struct AVCodecDefault
{
    const uint8_t *key;
    const uint8_t *value;
};

typedef struct AVCodecDefault AVCodecDefault;

typedef struct RcOverride
{
    int start_frame;
    int end_frame;
    int qscale; // If this is 0 then quality_factor will be used instead.
    float quality_factor;
} RcOverride;

typedef struct AVCodecDescriptor
{
    enum AVCodecID id;
    enum AVMediaType type;
    const char *name;
    const char *long_name;
    int props;
    const char *const *mime_types;
    const struct AVProfile *profiles;
} AVCodecDescriptor;

typedef struct AVCodecContext AVCodecContext;

typedef struct ScanTable
{
    const uint8_t *scantable;
    uint8_t permutated[64];
    uint8_t raster_end[64];
} ScanTable;

typedef struct ThreadFrame
{
    AVFrame *f;
    AVCodecContext *owner[2];
    // progress->data is an array of 2 ints holding progress for top/bottom
    // fields
    AVBufferRef *progress;
} ThreadFrame;

typedef struct Picture
{
    struct AVFrame *f;
    ThreadFrame tf;
    AVBufferRef *qscale_table_buf;
    int8_t *qscale_table;
    AVBufferRef *motion_val_buf[2];
    // int16_t (*motion_val[2])[2];
    AVBufferRef *mb_type_buf;
    uint32_t *mb_type; ///< types and macros are defined in mpegutils.h
    AVBufferRef *mbskip_table_buf;
    uint8_t *mbskip_table;
    AVBufferRef *ref_index_buf[2];
    int8_t *ref_index[2];
    AVBufferRef *mb_var_buf;
    uint16_t *mb_var; ///< Table for MB variances
    AVBufferRef *mc_mb_var_buf;
    uint16_t *mc_mb_var; ///< Table for motion compensated MB variances
    int alloc_mb_width;  ///< mb_width used to allocate tables
    int alloc_mb_height; ///< mb_height used to allocate tables
    AVBufferRef *mb_mean_buf;
    uint8_t *mb_mean; ///< Table for MB luminance
    AVBufferRef *hwaccel_priv_buf;
    void *hwaccel_picture_private; ///< Hardware accelerator private data
    int field_picture;             ///< whether or not the picture was encoded in separate fields
    int64_t mb_var_sum;            ///< sum of MB variance for current frame
    int64_t mc_mb_var_sum;         ///< motion compensated MB variance for current frame
    int b_frame_score;
    int needs_realloc; ///< Picture needs to be reallocated (eg due to a frame size change)
    int reference;
    int shared;
    uint64_t encoding_error[AV_NUM_DATA_POINTERS];
} Picture;

typedef uint64_t BitBuf;

typedef struct PutBitContext
{
    BitBuf bit_buf;
    int bit_left;
    uint8_t *buf, *buf_ptr, *buf_end;
    int size_in_bits;
} PutBitContext;

typedef void (*op_fill_func)(uint8_t *block /* align width (8 or 16) */,
                             uint8_t value, ptrdiff_t line_size, int h);

typedef struct BlockDSPContext
{
    void (*clear_block)(int16_t *block /* align 32 */);
    void (*clear_blocks)(int16_t *blocks /* align 32 */);

    op_fill_func fill_block_tab[2];
} BlockDSPContext;

typedef struct FDCTDSPContext
{
    void (*fdct)(int16_t *block );
    void (*fdct248)(int16_t *block );
} FDCTDSPContext;

typedef void (*h264_chroma_mc_func)(uint8_t *dst , uint8_t *src , ptrdiff_t srcStride, int h, int x, int y);

typedef struct H264ChromaContext
{
    h264_chroma_mc_func put_h264_chroma_pixels_tab[4];
    h264_chroma_mc_func avg_h264_chroma_pixels_tab[4];
} H264ChromaContext;

typedef void (*op_pixels_func)(uint8_t *block ,
                               const uint8_t *pixels ,
                               ptrdiff_t line_size, int h);

typedef struct HpelDSPContext
{
    op_pixels_func put_pixels_tab[4][4];
    op_pixels_func avg_pixels_tab[4][4];
    op_pixels_func put_no_rnd_pixels_tab[4][4];
    op_pixels_func avg_no_rnd_pixels_tab[4];
} HpelDSPContext;

enum idct_permutation_type
{
    FF_IDCT_PERM_NONE,
    FF_IDCT_PERM_LIBMPEG2,
    FF_IDCT_PERM_SIMPLE,
    FF_IDCT_PERM_TRANSPOSE,
    FF_IDCT_PERM_PARTTRANS,
    FF_IDCT_PERM_SSE2,
};

typedef struct IDCTDSPContext
{
    void (*put_pixels_clamped)(const int16_t *block ,uint8_t *av_restrict pixels,ptrdiff_t line_size);
    void (*put_signed_pixels_clamped)(const int16_t *block,uint8_t *av_restrict pixels,ptrdiff_t line_size);
    void (*add_pixels_clamped)(const int16_t *block,uint8_t *av_restrict pixels,ptrdiff_t line_size);
    void (*idct)(int16_t *block);
    void (*idct_put)(uint8_t *dest, ptrdiff_t line_size, int16_t *block);
    void (*idct_add)(uint8_t *dest, ptrdiff_t line_size, int16_t *block);
    uint8_t idct_permutation[64];
    enum idct_permutation_type perm_type;
    int mpeg4_studio_profile;
} IDCTDSPContext;

struct MpegEncContext;
typedef int (*me_cmp_func)(struct MpegEncContext *c,
                           uint8_t *blk1 /* align width (8 or 16) */,
                           uint8_t *blk2 /* align 1 */, ptrdiff_t stride,
                           int h);
typedef struct MECmpContext
{
    int (*sum_abs_dctelem)(int16_t *block /* align 16 */);

    me_cmp_func sad[6]; /* identical to pix_absAxA except additional void * */
    me_cmp_func sse[6];
    me_cmp_func hadamard8_diff[6];
    me_cmp_func dct_sad[6];
    me_cmp_func quant_psnr[6];
    me_cmp_func bit[6];
    me_cmp_func rd[6];
    me_cmp_func vsad[6];
    me_cmp_func vsse[6];
    me_cmp_func nsse[6];
    me_cmp_func w53[6];
    me_cmp_func w97[6];
    me_cmp_func dct_max[6];
    me_cmp_func dct264_sad[6];

    me_cmp_func me_pre_cmp[6];
    me_cmp_func me_cmp[6];
    me_cmp_func me_sub_cmp[6];
    me_cmp_func mb_cmp[6];
    me_cmp_func ildct_cmp[6];      // only width 16 used
    me_cmp_func frame_skip_cmp[6]; // only width 8 used

    me_cmp_func pix_abs[2][4];
    me_cmp_func median_sad[6];
} MECmpContext;

typedef int atomic_int;

typedef struct ERPicture
{
    AVFrame *f;
    ThreadFrame *tf;
    // int16_t (*motion_val[2])[2];
    int8_t *ref_index[2];

    uint32_t *mb_type;
    int field_picture;
} ERPicture;

typedef struct ERContext
{
    AVCodecContext *avctx;
    MECmpContext mecc;
    int mecc_inited;

    int *mb_index2xy;
    int mb_num;
    int mb_width, mb_height;
    ptrdiff_t mb_stride;
    ptrdiff_t b8_stride;

    atomic_int error_count;
    int error_occurred;
    uint8_t *error_status_table;
    uint8_t *er_temp_buffer;
    int16_t *dc_val[3];
    uint8_t *mbskip_table;
    uint8_t *mbintra_table;
    int mv[2][4][2];

    ERPicture cur_pic;
    ERPicture last_pic;
    ERPicture next_pic;

    AVBufferRef *ref_index_buf[2];
    AVBufferRef *motion_val_buf[2];

    uint16_t pp_time;
    uint16_t pb_time;
    int quarter_sample;
    int partitioned_frame;
    int ref_count;

    void (*decode_mb)(void *opaque, int ref, int mv_dir, int mv_type,
                      int (*mv)[2][4][2],
                      int mb_x, int mb_y, int mb_intra, int mb_skipped);
    void *opaque;
} ERContext;

typedef struct
{
    int start;       ///< timecode frame start (first base frame number)
    uint32_t flags;  ///< flags such as drop frame, +24 hours support, ...
    AVRational rate; ///< frame rate in rational form
    unsigned fps;    ///< frame per second; must be consistent with the rate field
} AVTimecode;

typedef struct MpegVideoDSPContext
{
    void (*gmc1)(uint8_t *dst /* align 8 */, uint8_t *src /* align 1 */,
                 int srcStride, int h, int x16, int y16, int rounder);
    void (*gmc)(uint8_t *dst /* align 8 */, uint8_t *src /* align 1 */,
                int stride, int h, int ox, int oy,
                int dxx, int dxy, int dyx, int dyy,
                int shift, int r, int width, int height);
} MpegVideoDSPContext;

typedef struct MpegvideoEncDSPContext
{
    int (*try_8x8basis)(int16_t rem[64], int16_t weight[64],
                        int16_t basis[64], int scale);
    void (*add_8x8basis)(int16_t rem[64], int16_t basis[64], int scale);

    int (*pix_sum)(uint8_t *pix, int line_size);
    int (*pix_norm1)(uint8_t *pix, int line_size);

    // void (*shrink[4])(uint8_t *dst, int dst_wrap, const uint8_t *src,int src_wrap, int width, int height);

    void (*draw_edges)(uint8_t *buf, int wrap, int width, int height, int w, int h, int sides);
} MpegvideoEncDSPContext;

typedef struct PixblockDSPContext
{
    // void (*get_pixels)(int16_t *av_restrict block,const uint8_t *pixels,ptrdiff_t stride);
    // void (*get_pixels_unaligned)(int16_t *av_restrict block ,const uint8_t *pixels,ptrdiff_t stride);
    // void (*diff_pixels)(int16_t *av_restrict block ,const uint8_t *s1 ,const uint8_t *s2 ,ptrdiff_t stride);
    // void (*diff_pixels_unaligned)(int16_t *av_restrict block ,const uint8_t *s1,const uint8_t *s2,ptrdiff_t stride);

} PixblockDSPContext;

typedef void (*qpel_mc_func)(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);

typedef struct QpelDSPContext
{
    qpel_mc_func put_qpel_pixels_tab[2][16];
    qpel_mc_func avg_qpel_pixels_tab[2][16];
    qpel_mc_func put_no_rnd_qpel_pixels_tab[2][16];
} QpelDSPContext;

typedef struct VideoDSPContext
{
    void (*emulated_edge_mc)(uint8_t *dst, const uint8_t *src, ptrdiff_t dst_linesize, ptrdiff_t src_linesize, int block_w, int block_h, int src_x, int src_y, int w, int h);

    void (*prefetch)(uint8_t *buf, ptrdiff_t stride, int h);
} VideoDSPContext;

typedef struct H263DSPContext
{
    void (*h263_h_loop_filter)(uint8_t *src, int stride, int qscale);
    void (*h263_v_loop_filter)(uint8_t *src, int stride, int qscale);
} H263DSPContext;

typedef struct MotionEstContext
{
    AVCodecContext *avctx;
    int skip;                ///< set if ME is skipped for the current MB
    int co_located_mv[4][2]; ///< mv from last P-frame for direct mode ME
    int direct_basis_mv[4][2];
    uint8_t *scratchpad; /**< data area for the ME algo, so that
                                     * the ME does not need to malloc/free. */
    uint8_t *best_mb;
    uint8_t *temp_mb[2];
    uint8_t *temp;
    int best_bits;
    uint32_t *map;       ///< map to avoid duplicate evaluations
    uint32_t *score_map; ///< map to store the scores
    unsigned map_generation;
    int pre_penalty_factor;
    int penalty_factor;
    int sub_penalty_factor;
    int mb_penalty_factor;
    int flags;
    int sub_flags;
    int mb_flags;
    int pre_pass; ///< = 1 for the pre pass
    int dia_size;
    int xmin;
    int xmax;
    int ymin;
    int ymax;
    int pred_x;
    int pred_y;
    uint8_t *src[4][4];
    uint8_t *ref[4][4];
    int stride;
    int uvstride;
    /* temp variables for picture complexity calculation */
    int64_t mc_mb_var_sum_temp;
    int64_t mb_var_sum_temp;
    int scene_change_score;

    op_pixels_func (*hpel_put)[4];
    op_pixels_func (*hpel_avg)[4];
    qpel_mc_func (*qpel_put)[16];
    qpel_mc_func (*qpel_avg)[16];
    uint8_t (*mv_penalty)[MAX_DMV * 2 + 1]; ///< bit amount needed to encode a MV
    uint8_t *current_mv_penalty;
    int (*sub_motion_search)(struct MpegEncContext *s,
                             int *mx_ptr, int *my_ptr, int dmin,
                             int src_index, int ref_index,
                             int size, int h);
} MotionEstContext;

typedef struct ScratchpadContext
{
    uint8_t *edge_emu_buffer; ///< temporary buffer for if MVs point to out-of-frame data
    uint8_t *rd_scratchpad;   ///< scratchpad for rate distortion mb decision
    uint8_t *obmc_scratchpad;
    uint8_t *b_scratchpad; ///< scratchpad used for writing into write only buffers
} ScratchpadContext;

typedef struct AVExpr AVExpr;

typedef struct RateControlEntry
{
    int pict_type;
    float qscale;
    int mv_bits;
    int i_tex_bits;
    int p_tex_bits;
    int misc_bits;
    int header_bits;
    uint64_t expected_bits;
    int new_pict_type;
    float new_qscale;
    int64_t mc_mb_var_sum;
    int64_t mb_var_sum;
    int i_count;
    int skip_count;
    int f_code;
    int b_code;
} RateControlEntry;



typedef struct RateControlContext
{
    int num_entries; ///< number of RateControlEntries
    RateControlEntry *entry;
    double buffer_index; ///< amount of bits in the video/audio buffer
    Predictor pred[5];
    double short_term_qsum;        ///< sum of recent qscales
    double short_term_qcount;      ///< count of recent qscales
    double pass1_rc_eq_output_sum; ///< sum of the output of the rc equation, this is used for normalization
    double pass1_wanted_bits;      ///< bits which should have been output by the pass1 code (including complexity init)
    double last_qscale;
    double last_qscale_for[5]; ///< last qscale for a specific pict type, used for max_diff & ipb factor stuff
    int64_t last_mc_mb_var_sum;
    int64_t last_mb_var_sum;
    uint64_t i_cplx_sum[5];
    uint64_t p_cplx_sum[5];
    uint64_t mv_bits_sum[5];
    uint64_t qscale_sum[5];
    int frame_count[5];
    int last_non_b_pict_type;

    void *non_lavc_opaque;   ///< context for non lavc rc code (for example xvid)
    float dry_run_qscale;    ///< for xvid rc
    int last_picture_number; ///< for xvid rc
    AVExpr *rc_eq_eval;
} RateControlContext;

typedef struct GetBitContext
{
    const uint8_t *buffer, *buffer_end;
#if CACHED_BITSTREAM_READER
    uint64_t cache;
    unsigned bits_left;
#endif
    int index;
    int size_in_bits;
    int size_in_bits_plus8;
} GetBitContext;

typedef struct ParseContext
{
    uint8_t *buffer;
    int index;
    int last_index;
    unsigned int buffer_size;
    uint32_t state; ///< contains the last few bytes in MSB order
    int frame_start_found;
    int overread;       ///< the number of bytes which where irreversibly read from the next frame
    int overread_index; ///< the index into ParseContext.buffer of the overread bytes
    uint64_t state64;   ///< contains the last 8 bytes in MSB order
} ParseContext;

typedef struct MpegEncContext
{
    AVClass *class;
    int y_dc_scale, c_dc_scale;
    int ac_pred;
    int block_last_index[12];  ///< last non zero coefficient in block
    int h263_aic;              ///< Advanced INTRA Coding (AIC)
    ScanTable inter_scantable; ///< if inter == intra then intra should be used to reduce the cache usage
    ScanTable intra_scantable;
    ScanTable intra_h_scantable;
    ScanTable intra_v_scantable;
    struct AVCodecContext *avctx;
    int width, height; ///< picture size. must be a multiple of 16
    int gop_size;
    int intra_only;               ///< if true, only intra pictures are generated
    int64_t bit_rate;             ///< wanted bit rate
    enum OutputFormat out_format; ///< output format
    int h263_pred;                ///< use MPEG-4/H.263 ac/dc predictions
    int pb_frame;                 ///< PB-frame mode (0 = none, 1 = base, 2 = improved)
    int h263_plus;                ///< H.263+ headers
    int h263_flv;                 ///< use flv H.263 header
    enum AVCodecID codec_id;      /* see AV_CODEC_ID_xxx */
    int fixed_qscale;             ///< fixed qscale if non zero
    int encoding;                 ///< true if we are encoding (vs decoding)
    int max_b_frames;             ///< max number of B-frames for encoding
    int luma_elim_threshold;
    int chroma_elim_threshold;
    int strict_std_compliance; ///< strictly follow the std (MPEG-4, ...)
    int workaround_bugs;       ///< workaround bugs in encoders which cannot be detected automatically
    int codec_tag;             ///< internal codec_tag upper case converted from avctx codec_tag
    int context_initialized;
    int input_picture_number;          ///< used to set pic->display_picture_number, should not be used for/by anything else
    int coded_picture_number;          ///< used to set pic->coded_picture_number, should not be used for/by anything else
    int picture_number;                //FIXME remove, unclear definition
    int picture_in_gop_number;         ///< 0-> first pic in gop, ...
    int mb_width, mb_height;           ///< number of MBs horizontally & vertically
    int mb_stride;                     ///< mb_width+1 used for some arrays to allow simple addressing of left & top MBs without sig11
    int b8_stride;                     ///< 2*mb_width+1 used for some 8x8 block arrays to allow simple addressing
    int h_edge_pos, v_edge_pos;        ///< horizontal / vertical position of the right/bottom edge (pixel replication)
    int mb_num;                        ///< number of MBs of a picture
    ptrdiff_t linesize;                ///< line size, in bytes, may be different from width
    ptrdiff_t uvlinesize;              ///< line size, for chroma in bytes, may be different from width
    Picture *picture;                  ///< main picture buffer
    Picture **input_picture;           ///< next pictures on display order for encoding
    Picture **reordered_input_picture; ///< pointer to the next pictures in coded order for encoding
    int64_t user_specified_pts;        ///< last non-zero pts from AVFrame which was passed into avcodec_encode_video2()
    int64_t dts_delta;
    int64_t reordered_pts;
    PutBitContext pb;
    int start_mb_y; ///< start mb_y of this thread (so current thread should process start_mb_y <= row < end_mb_y)
    int end_mb_y;   ///< end   mb_y of this thread (so current thread should process start_mb_y <= row < end_mb_y)
    struct MpegEncContext *thread_context[MAX_THREADS];
    int slice_context_count; ///< number of used thread_contexts
    Picture last_picture;
    Picture next_picture;
    Picture new_picture;
    Picture current_picture;      ///< buffer to store the decompressed current picture
    Picture *last_picture_ptr;    ///< pointer to the previous picture.
    Picture *next_picture_ptr;    ///< pointer to the next picture (for bidir pred)
    Picture *current_picture_ptr; ///< pointer to the current picture
    int last_dc[3];               ///< last DC values for MPEG-1
    int16_t *dc_val_base;
    int16_t *dc_val[3];                 ///< used for MPEG-4 DC prediction, all 3 arrays must be continuous
    const uint8_t *y_dc_scale_table;    ///< qscale -> y_dc_scale table
    const uint8_t *c_dc_scale_table;    ///< qscale -> c_dc_scale table
    const uint8_t *chroma_qscale_table; ///< qscale -> chroma_qscale (H.263)
    uint8_t *coded_block_base;
    uint8_t *coded_block; ///< used for coded block pattern prediction (msmpeg4v3, wmv1)
    int16_t (*ac_val_base)[16];
    // int16_t (*ac_val[3])[16];      ///< used for MPEG-4 AC prediction, all 3 arrays must be continuous
    int mb_skipped;          ///< MUST BE SET only during DECODING
    uint8_t *mbskip_table;   /**< used to avoid copy if macroblock skipped (for black regions for example) and used for B-frame encoding & decoding (contains skip table of next P-frame) */
    uint8_t *mbintra_table;  ///< used to avoid setting {ac, dc, cbp}-pred stuff to zero on inter MB decoding
    uint8_t *cbp_table;      ///< used to store cbp, ac_pred for partitioned decoding
    uint8_t *pred_dir_table; ///< used to store pred_dir for partitioned decoding
    ScratchpadContext sc;
    int qscale;           ///< QP
    int chroma_qscale;    ///< chroma QP
    unsigned int lambda;  ///< Lagrange multiplier used in rate distortion
    unsigned int lambda2; ///< (lambda*lambda) >> FF_LAMBDA_SHIFT
    int *lambda_table;
    int adaptive_quant; ///< use adaptive quantization
    int dquant;         ///< qscale difference to prev qscale
    int closed_gop;     ///< MPEG1/2 GOP is closed
    int pict_type;      ///< AV_PICTURE_TYPE_I, AV_PICTURE_TYPE_P, AV_PICTURE_TYPE_B, ...
    int vbv_delay;
    int last_pict_type;       //FIXME removes
    int last_non_b_pict_type; ///< used for MPEG-4 gmc B-frames & ratecontrol
    int droppable;
    int frame_rate_index;
    AVRational mpeg2_frame_rate_ext;
    int last_lambda_for[5]; ///< last lambda for a specific pict type
    int skipdct;            ///< skip dct and code zero residual
    /* motion compensation */
    int unrestricted_mv;   ///< mv can point outside of the coded picture
    int h263_long_vectors; ///< use horrible H.263v1 long vector mode
    BlockDSPContext bdsp;
    FDCTDSPContext fdsp;
    H264ChromaContext h264chroma;
    HpelDSPContext hdsp;
    IDCTDSPContext idsp;
    MECmpContext mecc;
    MpegVideoDSPContext mdsp;
    MpegvideoEncDSPContext mpvencdsp;
    PixblockDSPContext pdsp;
    QpelDSPContext qdsp;
    VideoDSPContext vdsp;
    H263DSPContext h263dsp;
    int f_code; ///< forward MV resolution
    int b_code; ///< backward MV resolution for B-frames (MPEG-4)
    int16_t (*p_mv_table_base)[2];
    int16_t (*b_forw_mv_table_base)[2];
    int16_t (*b_back_mv_table_base)[2];
    int16_t (*b_bidir_forw_mv_table_base)[2];
    int16_t (*b_bidir_back_mv_table_base)[2];
    int16_t (*b_direct_mv_table_base)[2];
    // int16_t (*p_field_mv_table_base[2][2])[2];
    // int16_t (*b_field_mv_table_base[2][2][2])[2];
    int16_t (*p_mv_table)[2];            ///< MV table (1MV per MB) P-frame encoding
    int16_t (*b_forw_mv_table)[2];       ///< MV table (1MV per MB) forward mode B-frame encoding
    int16_t (*b_back_mv_table)[2];       ///< MV table (1MV per MB) backward mode B-frame encoding
    int16_t (*b_bidir_forw_mv_table)[2]; ///< MV table (1MV per MB) bidir mode B-frame encoding
    int16_t (*b_bidir_back_mv_table)[2]; ///< MV table (1MV per MB) bidir mode B-frame encoding
    int16_t (*b_direct_mv_table)[2];     ///< MV table (1MV per MB) direct mode B-frame encoding
    // int16_t (*p_field_mv_table[2][2])[2];   ///< MV table (2MV per MB) interlaced P-frame encoding
    // int16_t (*b_field_mv_table[2][2][2])[2];///< MV table (4MV per MB) interlaced B-frame encoding
    // uint8_t (*p_field_select_table[2]);
    // uint8_t (*b_field_select_table[2][2]);
    int motion_est; ///< ME algorithm
    int me_penalty_compensation;
    int me_pre; ///< prepass for motion estimation
    int mv_dir;
    int mv_type;
    int mv[2][4][2];
    int field_select[2][2];
    int last_mv[2][2][2];           ///< last MV, used for MV prediction in MPEG-1 & B-frame MPEG-4
    uint8_t *fcode_tab;             ///< smallest fcode needed for each MV
    int16_t direct_scale_mv[2][64]; ///< precomputed to avoid divisions in ff_mpeg4_set_direct_mv
    MotionEstContext me;
    int no_rounding; /**< apply no rounding to motion compensation (MPEG-4, msmpeg4, ...)
                        for B-frames rounding mode is always 0 */
    /* macroblock layer */
    int mb_x, mb_y;
    int mb_skip_run;
    int mb_intra;
    uint16_t *mb_type;  ///< Table for candidate MB types for encoding (defines in mpegutils.h)
    int block_index[6]; ///< index to current MB in block based arrays with edges
    int block_wrap[6];
    uint8_t *dest[3];
    int *mb_index2xy; ///< mb_index -> mb_x + mb_y*mb_stride
    /** matrix transmitted in the bitstream */
    uint16_t intra_matrix[64];
    uint16_t chroma_intra_matrix[64];
    uint16_t inter_matrix[64];
    uint16_t chroma_inter_matrix[64];
    int force_duplicated_matrix; ///< Force duplication of mjpeg matrices, useful for rtp streaming
    int intra_quant_bias;        ///< bias for the quantizer
    int inter_quant_bias;        ///< bias for the quantizer
    int min_qcoeff;              ///< minimum encodable coefficient
    int max_qcoeff;              ///< maximum encodable coefficient
    int ac_esc_length;           ///< num of bits needed to encode the longest esc
    uint8_t *intra_ac_vlc_length;
    uint8_t *intra_ac_vlc_last_length;
    uint8_t *intra_chroma_ac_vlc_length;
    uint8_t *intra_chroma_ac_vlc_last_length;
    uint8_t *inter_ac_vlc_length;
    uint8_t *inter_ac_vlc_last_length;
    uint8_t *luma_dc_vlc_length;
    int coded_score[12];
    /** precomputed matrix (combine qscale and DCT renorm) */
    int (*q_intra_matrix)[64];
    int (*q_chroma_intra_matrix)[64];
    int (*q_inter_matrix)[64];
    /** identical to the above but for MMX & these are not permutated, second 64 entries are bias*/
    uint16_t (*q_intra_matrix16)[2][64];
    uint16_t (*q_chroma_intra_matrix16)[2][64];
    uint16_t (*q_inter_matrix16)[2][64];
    /* noise reduction */
    int (*dct_error_sum)[64];
    int dct_count[2];
    uint16_t (*dct_offset)[64];
    /* bit rate control */
    int64_t total_bits;
    int frame_bits;                ///< bits used for the current frame
    int stuffing_bits;             ///< bits used for stuffing
    int next_lambda;               ///< next lambda used for retrying to encode a frame
    RateControlContext rc_context; ///< contains stuff only accessed in ratecontrol.c
    /* statistics, used for 2-pass encoding */
    int mv_bits;
    int header_bits;
    int i_tex_bits;
    int p_tex_bits;
    int i_count;
    int f_count;
    int b_count;
    int skip_count;
    int misc_bits; ///< cbp, mb_type
    int last_bits; ///< temp var used for calculating the above vars
    /* error concealment / resync */
    int resync_mb_x;              ///< x position of last resync marker
    int resync_mb_y;              ///< y position of last resync marker
    GetBitContext last_resync_gb; ///< used to search for the next resync marker
    int mb_num_left;              ///< number of MBs left in this video packet (for partitioned Slices only)
    int next_p_frame_damaged;     ///< set if the next p frame is damaged, to avoid showing trashed B-frames
    ParseContext parse_context;
    /* H.263 specific */
    int gob_index;
    int obmc;    ///< overlapped block motion compensation
    int mb_info; ///< interval for outputting info about mb offsets as side data
    int prev_mb_info, last_mb_info;
    uint8_t *mb_info_ptr;
    int mb_info_size;
    int ehc_mode;
    int rc_strategy; ///< deprecated

    /* H.263+ specific */
    int umvplus;      ///< == H.263+ && unrestricted_mv
    int h263_aic_dir; ///< AIC direction: 0 = left, 1 = top
    int h263_slice_structured;
    int alt_inter_vlc; ///< alternative inter vlc
    int modified_quant;
    int loop_filter;
    int custom_pcf;

    /* MPEG-4 specific */
    int studio_profile;
    int dct_precision;
    ///< number of bits to represent the fractional part of time (encoder only)
    int time_increment_bits;
    int last_time_base;
    int time_base; ///< time in seconds of last I,P,S Frame
    int64_t time;  ///< time of current frame
    int64_t last_non_b_time;
    uint16_t pp_time; ///< time distance between the last 2 p,s,i frames
    uint16_t pb_time; ///< time distance between the last b and p,s,i frame
    uint16_t pp_field_time;
    uint16_t pb_field_time; ///< like above, just for interlaced
    int real_sprite_warping_points;
    int sprite_offset[2][2]; ///< sprite offset[isChroma][isMVY]
    int sprite_delta[2][2];  ///< sprite_delta [isY][isMVY]
    int mcsel;
    int quant_precision;
    int quarter_sample;    ///< 1->qpel, 0->half pel ME/MC
    int aspect_ratio_info; //FIXME remove
    int sprite_warping_accuracy;
    int data_partitioning; ///< data partitioning flag from header
    int partitioned_frame; ///< is current frame partitioned
    int low_delay;         ///< no reordering needed / has no B-frames
    int vo_type;
    PutBitContext tex_pb; ///< used for data partitioned VOPs
    PutBitContext pb2;    ///< used for data partitioned VOPs
    int mpeg_quant;
    int padding_bug_score; ///< used to detect the VERY common padding bug in MPEG-4

    /* divx specific, used to workaround (many) bugs in divx5 */
    int divx_packed;
    uint8_t *bitstream_buffer; //Divx 5.01 puts several frames in a single one, this is used to reorder them
    int bitstream_buffer_size;
    unsigned int allocated_bitstream_buffer_size;

    /* RV10 specific */
    int rv10_version; ///< RV10 version: 0 or 3
    int rv10_first_dc_coded[3];
    /* MJPEG specific */
    struct MJpegContext *mjpeg_ctx;
    int esc_pos;
    int pred;
    int huffman;
    /* MSMPEG4 specific */
    int mv_table_index;
    int rl_table_index;
    int rl_chroma_table_index;
    int dc_table_index;
    int use_skip_mb_code;
    int slice_height;     ///< in macroblocks
    int first_slice_line; ///< used in MPEG-4 too to handle resync markers
    int flipflop_rounding;
    int msmpeg4_version; ///< 0=not msmpeg4, 1=mp41, 2=mp42, 3=mp43/divx3 4=wmv1/7 5=wmv2/8
    int per_mb_rl_table;
    int esc3_level_length;
    int esc3_run_length;
    /** [mb_intra][isChroma][level][run][last] */
    int (*ac_stats)[2][MAX_LEVEL + 1][MAX_RUN + 1][2];
    int inter_intra_pred;
    int mspel;
    /* decompression specific */
    GetBitContext gb;
    /* MPEG-1 specific */
    int gop_picture_number; ///< index of the first picture of a GOP based on fake_pic_num & MPEG-1 specific
    int last_mv_dir;        ///< last mv_dir, used for B-frame encoding
    uint8_t *vbv_delay_ptr; ///< pointer to vbv_delay in the bitstream
    /* MPEG-2-specific - I wished not to have to support this mess. */
    int progressive_sequence;
    int mpeg_f_code[2][2];
    int a53_cc;
    // picture structure defines are loaded from mpegutils.h
    int picture_structure;
    int64_t timecode_frame_start; ///< GOP timecode frame start number, in non drop frame format
    int intra_dc_precision;
    int frame_pred_frame_dct;
    int top_field_first;
    int concealment_motion_vectors;
    int q_scale_type;
    int brd_scale;
    int intra_vlc_format;
    int alternate_scan;
    int seq_disp_ext;
    int video_format;
    int repeat_first_field;
    int chroma_420_type;
    int chroma_format;
    int chroma_x_shift; //depend on pix_format, that depend on chroma_format
    int chroma_y_shift;
    int progressive_frame;
    int full_pel[2];
    int interlaced_dct;
    int first_field;         ///< is 1 for the first field of a field picture 0 otherwise
    int drop_frame_timecode; ///< timecode is in drop frame format.
    int scan_offset;         ///< reserve space for SVCD scan offset user data.
    /* RTP specific */
    int rtp_mode;
    int rtp_payload_size;
    char *tc_opt_str; ///< timecode option string
    AVTimecode tc;    ///< timecode context
    uint8_t *ptr_lastgob;
    int swap_uv;      //vcr2 codec is an MPEG-2 variant with U and V swapped
    int pack_pblocks; //xvmc needs to keep blocks without gaps.
    // int16_t (*pblocks[12])[64];
    int16_t (*block)[64];                                              ///< points to one of the following blocks
    int16_t (*blocks)[12][64];                                         // for HQ mode we need to keep the best block
    int (*decode_mb)(struct MpegEncContext *s, int16_t block[12][64]); // used by some codecs to avoid a switch()
    int32_t (*block32)[12][64];
    int dpcm_direction; // 0 = DCT, 1 = DPCM top to bottom scan, -1 = DPCM bottom to top scan
    int16_t (*dpcm_macroblock)[3][256];

    void (*dct_unquantize_mpeg1_intra)(struct MpegEncContext *s,
                                       int16_t *block /*align 16*/, int n, int qscale);
    void (*dct_unquantize_mpeg1_inter)(struct MpegEncContext *s,
                                       int16_t *block /*align 16*/, int n, int qscale);
    void (*dct_unquantize_mpeg2_intra)(struct MpegEncContext *s,
                                       int16_t *block /*align 16*/, int n, int qscale);
    void (*dct_unquantize_mpeg2_inter)(struct MpegEncContext *s,
                                       int16_t *block /*align 16*/, int n, int qscale);
    void (*dct_unquantize_h263_intra)(struct MpegEncContext *s,
                                      int16_t *block /*align 16*/, int n, int qscale);
    void (*dct_unquantize_h263_inter)(struct MpegEncContext *s,
                                      int16_t *block /*align 16*/, int n, int qscale);
    void (*dct_unquantize_intra)(struct MpegEncContext *s, // unquantizer to use (MPEG-4 can use both)
                                 int16_t *block /*align 16*/, int n, int qscale);
    void (*dct_unquantize_inter)(struct MpegEncContext *s, // unquantizer to use (MPEG-4 can use both)
                                 int16_t *block /*align 16*/, int n, int qscale);
    int (*dct_quantize)(struct MpegEncContext *s, int16_t *block /*align 16*/, int n, int qscale, int *overflow);
    int (*fast_dct_quantize)(struct MpegEncContext *s, int16_t *block /*align 16*/, int n, int qscale, int *overflow);
    void (*denoise_dct)(struct MpegEncContext *s, int16_t *block);
    int mpv_flags; ///< flags set by private options
    int quantizer_noise_shaping;
    float rc_qsquish;
    float rc_qmod_amp;
    int rc_qmod_freq;
    float rc_initial_cplx;
    float rc_buffer_aggressivity;
    float border_masking;
    int lmin, lmax;
    int vbv_ignore_qmax;
    char *rc_eq;
    /* temp buffers for rate control */
    float *cplx_tab, *bits_tab;
    /* flag to indicate a reinitialization is required, e.g. after
     * a frame size change */
    int context_reinit;
    ERContext er;
    int error_rate;
    AVFrame *tmp_frames[MAX_B_FRAMES + 2];
    int b_frame_strategy;
    int b_sensitivity;
    int frame_skip_threshold;
    int frame_skip_factor;
    int frame_skip_exp;
    int frame_skip_cmp;
    int scenechange_threshold;
    int noise_reduction;
    int intra_penalty;
} MpegEncContext;

typedef struct AVHWAccel
{
    const char *name;
    enum AVMediaType type;
    enum AVCodecID id;
    enum AVPixelFormat pix_fmt;
    int capabilities;
    int (*alloc_frame)(AVCodecContext *avctx, AVFrame *frame);
    int (*start_frame)(AVCodecContext *avctx, const uint8_t *buf, uint32_t buf_size);
    int (*decode_params)(AVCodecContext *avctx, int type, const uint8_t *buf, uint32_t buf_size);
    int (*decode_slice)(AVCodecContext *avctx, const uint8_t *buf, uint32_t buf_size);
    int (*end_frame)(AVCodecContext *avctx);
    int frame_priv_data_size;
    void (*decode_mb)(struct MpegEncContext *s);
    int (*init)(AVCodecContext *avctx);
    int (*uninit)(AVCodecContext *avctx);
    int priv_data_size;
    int caps_internal;
    int (*frame_params)(AVCodecContext *avctx, AVBufferRef *hw_frames_ctx);
} AVHWAccel;

typedef struct AVCodecContext
{
    const AVClass *av_class;
    int log_level_offset;
    enum AVMediaType codec_type; /* see AVMEDIA_TYPE_xxx */
    const struct AVCodec *codec;
    enum AVCodecID codec_id; /* see AV_CODEC_ID_xxx */
    unsigned int codec_tag;
    void *priv_data;
    struct AVCodecInternal *internal;
    void *opaque;
    int64_t bit_rate;
    int bit_rate_tolerance;
    int global_quality;
    int compression_level;
    int flags;
    int flags2;
    uint8_t *extradata;
    int extradata_size;
    AVRational time_base;
    int ticks_per_frame;
    int delay;
    int width, height;
    int coded_width, coded_height;
    int gop_size;
    enum AVPixelFormat pix_fmt;
    void (*draw_horiz_band)(struct AVCodecContext *s,
                            const AVFrame *src, int offset[AV_NUM_DATA_POINTERS],
                            int y, int type, int height);
    enum AVPixelFormat (*get_format)(struct AVCodecContext *s, const enum AVPixelFormat *fmt);
    int max_b_frames;
    float b_quant_factor;
    float b_quant_offset;
    int has_b_frames;
    float i_quant_factor;
    float i_quant_offset;
    float lumi_masking;
    float temporal_cplx_masking;
    float spatial_cplx_masking;
    float p_masking;
    float dark_masking;
    int slice_count;
    int *slice_offset;
    AVRational sample_aspect_ratio;
    int me_cmp;
    int me_sub_cmp;
    int mb_cmp;
    int ildct_cmp;
    int dia_size;
    int last_predictor_count;
    int me_pre_cmp;
    int pre_dia_size;
    int me_subpel_quality;
    int me_range;
    int slice_flags;
    int mb_decision;
    uint16_t *intra_matrix;
    uint16_t *inter_matrix;
    int intra_dc_precision;
    int skip_top;
    int skip_bottom;
    int mb_lmin;
    int mb_lmax;
    int bidir_refine;
    int keyint_min;
    int refs;
    int mv0_threshold;
    enum AVColorPrimaries color_primaries;
    enum AVColorTransferCharacteristic color_trc;
    enum AVColorSpace colorspace;
    enum AVColorRange color_range;
    enum AVChromaLocation chroma_sample_location;
    int slices;
    enum AVFieldOrder field_order;
    int sample_rate;                ///< samples per second
    int channels;                   ///< number of audio channels
    enum AVSampleFormat sample_fmt; ///< sample format
    int frame_size;
    int frame_number;
    int block_align;
    int cutoff;
    uint64_t channel_layout;
    uint64_t request_channel_layout;
    enum AVAudioServiceType audio_service_type;
    enum AVSampleFormat request_sample_fmt;
    int (*get_buffer2)(struct AVCodecContext *s, AVFrame *frame, int flags);
    attribute_deprecated int refcounted_frames;
    float qcompress; ///< amount of qscale change between easy & hard scenes (0.0-1.0)
    float qblur;     ///< amount of qscale smoothing over time (0.0-1.0)
    int qmin;
    int qmax;
    int max_qdiff;
    int rc_buffer_size;
    int rc_override_count;
    RcOverride *rc_override;
    int64_t rc_max_rate;
    int64_t rc_min_rate;
    float rc_max_available_vbv_use;
    float rc_min_vbv_overflow_use;
    int rc_initial_buffer_occupancy;
    int trellis;
    char *stats_out;
    char *stats_in;
    int workaround_bugs;
    int strict_std_compliance;
    int error_concealment;
    int debug;
    int err_recognition;
    int64_t reordered_opaque;
    const struct AVHWAccel *hwaccel;
    void *hwaccel_context;
    uint64_t error[AV_NUM_DATA_POINTERS];
    int dct_algo;
    int idct_algo;
    int bits_per_coded_sample;
    int bits_per_raw_sample;
#if FF_API_LOWRES
    int lowres;
#endif
    int thread_count;
    int thread_type;
    int active_thread_type;
    int thread_safe_callbacks;
    int (*execute)(struct AVCodecContext *c, int (*func)(struct AVCodecContext *c2, void *arg), void *arg2, int *ret, int count, int size);
    int (*execute2)(struct AVCodecContext *c, int (*func)(struct AVCodecContext *c2, void *arg, int jobnr, int threadnr), void *arg2, int *ret, int count);
    int nsse_weight;
    int profile;
    int level;
    enum AVDiscard skip_loop_filter;
    enum AVDiscard skip_idct;
    enum AVDiscard skip_frame;
    uint8_t *subtitle_header;
    int subtitle_header_size;
    int initial_padding;
    AVRational framerate;
    enum AVPixelFormat sw_pix_fmt;
    AVRational pkt_timebase;
    const AVCodecDescriptor *codec_descriptor;
#if !FF_API_LOWRES
    int lowres;
#endif
    int64_t pts_correction_num_faulty_pts; /// Number of incorrect PTS values so far
    int64_t pts_correction_num_faulty_dts; /// Number of incorrect DTS values so far
    int64_t pts_correction_last_pts;       /// PTS of the last frame
    int64_t pts_correction_last_dts;       /// DTS of the last frame
    char *sub_charenc;
    int sub_charenc_mode;

    int skip_alpha;
    int seek_preroll;
    uint16_t *chroma_intra_matrix;
    uint8_t *dump_separator;
    char *codec_whitelist;
    unsigned properties;

    AVPacketSideData *coded_side_data;
    int nb_coded_side_data;
    AVBufferRef *hw_frames_ctx;
    int sub_text_format;
#if FF_API_ASS_TIMING
#define FF_SUB_TEXT_FMT_ASS_WITH_TIMINGS 1
#endif
    int trailing_padding;
    int64_t max_pixels;
    AVBufferRef *hw_device_ctx;
    int hwaccel_flags;
    int apply_cropping;
    int extra_hw_frames;
    int discard_damaged_percentage;
    int64_t max_samples;
} AVCodecContext;

struct AVExpr
{
    // enum {
    //     e_value, e_const, e_func0, e_func1, e_func2,
    //     e_squish, e_gauss, e_ld, e_isnan, e_isinf,
    //     e_mod, e_max, e_min, e_eq, e_gt, e_gte, e_lte, e_lt,
    //     e_pow, e_mul, e_div, e_add,
    //     e_last, e_st, e_while, e_taylor, e_root, e_floor, e_ceil, e_trunc, e_round,
    //     e_sqrt, e_not, e_random, e_hypot, e_gcd,
    //     e_if, e_ifnot, e_print, e_bitand, e_bitor, e_between, e_clip, e_atan2, e_lerp,
    //     e_sgn,
    // } type;
    double value; // is sign in other types
    int const_index;
    union
    {
        double (*func0)(double);
        double (*func1)(void *, double);
        double (*func2)(void *, double, double);
    } a;
    struct AVExpr *param[3];
    double *var;
};

typedef struct AVCodecHWConfig
{
    enum AVPixelFormat pix_fmt;
    int methods;
    enum AVHWDeviceType device_type;
} AVCodecHWConfig;

typedef struct AVCodecHWConfigInternal
{
    AVCodecHWConfig public;
    const AVHWAccel *hwaccel;
} AVCodecHWConfigInternal;

typedef struct AVCodec
{
    const char *name;
    const char *long_name;
    enum AVMediaType type;
    enum AVCodecID id;
    int capabilities;
    const AVRational *supported_framerates; ///< array of supported framerates, or NULL if any, array is terminated by {0,0}
    const enum AVPixelFormat *pix_fmts;     ///< array of supported pixel formats, or NULL if unknown, array is terminated by -1
    const int *supported_samplerates;       ///< array of supported audio samplerates, or NULL if unknown, array is terminated by 0
    const enum AVSampleFormat *sample_fmts; ///< array of supported sample formats, or NULL if unknown, array is terminated by -1
    const uint64_t *channel_layouts;        ///< array of support channel layouts, or NULL if unknown. array is terminated by 0
    uint8_t max_lowres;                     ///< maximum value for lowres supported by the decoder
    const AVClass *priv_class;              ///< AVClass for the private context
    const AVProfile *profiles;              ///< array of recognized profiles, or NULL if unknown, array is terminated by {FF_PROFILE_UNKNOWN}
    const char *wrapper_name;
    int priv_data_size;
    struct AVCodec *next;
    int (*init_thread_copy)(AVCodecContext *);
    int (*update_thread_context)(AVCodecContext *dst, const AVCodecContext *src);
    const AVCodecDefault *defaults;
    void (*init_static_data)(struct AVCodec *codec);
    int (*init)(AVCodecContext *);
    int (*encode_sub)(AVCodecContext *, uint8_t *buf, int buf_size,
                      const struct AVSubtitle *sub);
    int (*encode2)(AVCodecContext *avctx, AVPacket *avpkt, const AVFrame *frame,
                   int *got_packet_ptr);
    int (*decode)(AVCodecContext *, void *outdata, int *outdata_size, AVPacket *avpkt);
    int (*close)(AVCodecContext *);
    int (*send_frame)(AVCodecContext *avctx, const AVFrame *frame);
    int (*receive_packet)(AVCodecContext *avctx, AVPacket *avpkt);
    int (*receive_frame)(AVCodecContext *avctx, AVFrame *frame);
    void (*flush)(AVCodecContext *);
    int caps_internal;
    const char *bsfs;
    const struct AVCodecHWConfigInternal **hw_configs;
} AVCodec;

typedef struct AVCodecParameters
{
    enum AVMediaType codec_type;
    enum AVCodecID codec_id;
    uint32_t codec_tag;
    uint8_t *extradata;
    int extradata_size;
    int format;
    int64_t bit_rate;
    int bits_per_coded_sample;
    int bits_per_raw_sample;
    int profile;
    int level;
    int width;
    int height;
    AVRational sample_aspect_ratio;
    enum AVFieldOrder field_order;
    enum AVColorRange color_range;
    enum AVColorPrimaries color_primaries;
    enum AVColorTransferCharacteristic color_trc;
    enum AVColorSpace color_space;
    enum AVChromaLocation chroma_location;
    int video_delay;
    uint64_t channel_layout;
    int channels;
    int sample_rate;
    int block_align;
    int frame_size;
    int initial_padding;
    int trailing_padding;
    int seek_preroll;
} AVCodecParameters;

typedef struct AVBSFInternal AVBSFInternal;

typedef struct AVBSFContext
{
    const AVClass *av_class;
    const struct AVBitStreamFilter *filter;
    AVBSFInternal *internal;
    void *priv_data;
    AVCodecParameters *par_in;
    AVCodecParameters *par_out;
    AVRational time_base_in;
    AVRational time_base_out;
} AVBSFContext;

typedef struct AVBitStreamFilter
{
    const char *name;
    const enum AVCodecID *codec_ids;
    const AVClass *priv_class;
    int priv_data_size;
    int (*init)(AVBSFContext *ctx);
    int (*filter)(AVBSFContext *ctx, AVPacket *pkt);
    void (*close)(AVBSFContext *ctx);
    void (*flush)(AVBSFContext *ctx);
} AVBitStreamFilter;

typedef struct DecodeSimpleContext
{
    AVPacket *in_pkt;
} DecodeSimpleContext;

typedef struct AVPacketList
{
    AVPacket pkt;
    struct AVPacketList *next;
} AVPacketList;

typedef struct EncodeSimpleContext
{
    AVFrame *in_frame;
} EncodeSimpleContext;

typedef struct AVCodecInternal
{
    int is_copy;
    int last_audio_frame;
    AVFrame *to_free;
    AVBufferRef *pool;
    void *thread_ctx;
    DecodeSimpleContext ds;
    AVBSFContext *bsf;
    AVPacket *last_pkt_props;
    AVPacketList *pkt_props;
    AVPacketList *pkt_props_tail;
    uint8_t *byte_buffer;
    unsigned int byte_buffer_size;
    void *frame_thread_encoder;
    EncodeSimpleContext es;
    int skip_samples;
    void *hwaccel_priv_data;
    int draining;
    AVPacket *buffer_pkt;
    AVFrame *buffer_frame;
    int draining_done;
    int compat_decode_warned;
    size_t compat_decode_consumed;
    size_t compat_decode_partial_size;
    AVFrame *compat_decode_frame;
    AVPacket *compat_encode_packet;
    int showed_multi_packet_warning;
    int skip_samples_multiplier;
    int nb_draining_errors;
    int changed_frames_dropped;
    int initial_format;
    int initial_width, initial_height;
    int initial_sample_rate;
    int initial_channels;
    uint64_t initial_channel_layout;
} AVCodecInternal;

typedef struct Decoder
{
    AVPacket pkt;
    PacketQueue *queue;
    AVCodecContext *avctx;
    int pkt_serial;
    int finished;
    int packet_pending;
    SDL_cond *empty_queue_cond;
    int64_t start_pts;
    AVRational start_pts_tb;
    int64_t next_pts;
    AVRational next_pts_tb;
    SDL_Thread *decoder_tid;
} Decoder;

typedef struct AVIOContext
{
    const AVClass *av_class;
    unsigned char *buffer;  /**< Start of the buffer. */
    int buffer_size;        /**< Maximum buffer size */
    unsigned char *buf_ptr; /**< Current position in the buffer */
    unsigned char *buf_end; /**< End of the data, may be less than
                                 buffer+buffer_size if the read function returned
                                 less data than requested, e.g. for streams where
                                 no more data has been received yet. */
    void *opaque;           /**< A private pointer, passed to the read/write/seek/...
                                 functions. */
    int (*read_packet)(void *opaque, uint8_t *buf, int buf_size);
    int (*write_packet)(void *opaque, uint8_t *buf, int buf_size);
    int64_t (*seek)(void *opaque, int64_t offset, int whence);
    int64_t pos;     /**< position in the file of the current buffer */
    int eof_reached; /**< true if was unable to read due to error or eof */
    int write_flag;  /**< true if open for writing */
    int max_packet_size;
    unsigned long checksum;
    unsigned char *checksum_ptr;
    unsigned long (*update_checksum)(unsigned long checksum, const uint8_t *buf, unsigned int size);
    int error; /**< contains the error code or 0 if no error happened */
    int (*read_pause)(void *opaque, int pause);
    int64_t (*read_seek)(void *opaque, int stream_index,
                         int64_t timestamp, int flags);
    int seekable;
    int64_t maxsize;
    int direct;
    int64_t bytes_read;
    int seek_count;
    int writeout_count;
    int orig_buffer_size;
    int short_seek_threshold;
    const char *protocol_whitelist;
    const char *protocol_blacklist;
    int (*write_data_type)(void *opaque, uint8_t *buf, int buf_size,
                           enum AVIODataMarkerType type, int64_t time);
    int ignore_boundary_point;
    enum AVIODataMarkerType current_type;
    int64_t last_time;
    int (*short_seek_get)(void *opaque);
    int64_t written;
    unsigned char *buf_ptr_max;
    int min_packet_size;
} AVIOContext;

typedef struct AVProgram
{
    int id;
    int flags;
    enum AVDiscard discard; ///< selects which program to discard and which to feed to the caller
    unsigned int *stream_index;
    unsigned int nb_stream_indexes;
    AVDictionary *metadata;
    int program_num;
    int pmt_pid;
    int pcr_pid;
    int pmt_version;
    int64_t start_time;
    int64_t end_time;
    int64_t pts_wrap_reference; ///< reference dts for wrap detection
    int pts_wrap_behavior;      ///< behavior on wrap detection
} AVProgram;

typedef struct AVChapter
{
    int id;               ///< unique ID to identify the chapter
    AVRational time_base; ///< time base in which the start/end timestamps are specified
    int64_t start, end;   ///< chapter start/end time in time_base units
    AVDictionary *metadata;
} AVChapter;

struct AVFormatContext;
typedef int (*av_format_control_message)(struct AVFormatContext *s, int type,
                                         void *data, size_t data_size);

typedef struct AVIOInterruptCB
{
    int (*callback)(void *);
    void *opaque;
} AVIOInterruptCB;

struct AVFormatInternal {
    int nb_interleaved_streams;
    struct AVPacketList *packet_buffer;
    struct AVPacketList *packet_buffer_end;
    int64_t data_offset; /**< offset of the first packet */
    struct AVPacketList *raw_packet_buffer;
    struct AVPacketList *raw_packet_buffer_end;
    struct AVPacketList *parse_queue;
    struct AVPacketList *parse_queue_end;
#define RAW_PACKET_BUFFER_SIZE 2500000
    int raw_packet_buffer_remaining_size;
    int64_t offset;
    AVRational offset_timebase;
#if FF_API_COMPUTE_PKT_FIELDS2
    int missing_ts_warning;
#endif
    int inject_global_side_data;
    int avoid_negative_ts_use_pts;
    int64_t shortest_end;
    int initialized;
    int streams_initialized;
    AVDictionary *id3v2_meta;
    int prefer_codec_framerate;
};

typedef struct AVFormatInternal AVFormatInternal;

typedef struct AVDeviceInfo
{
    char *device_name;        /**< device name, format depends on device */
    char *device_description; /**< human friendly name */
} AVDeviceInfo;

/**
 * List of devices.
 */
typedef struct AVDeviceInfoList
{
    AVDeviceInfo **devices; /**< list of autodetected devices */
    int nb_devices;         /**< number of autodetected devices */
    int default_device;     /**< index of default device or -1 if no default */
} AVDeviceInfoList;

struct AVFormatContext;

struct AVDeviceCapabilitiesQuery;

typedef struct AVOutputFormat
{
    const char *name;
    const char *long_name;
    const char *mime_type;
    const char *extensions;        /**< comma-separated filename extensions */
    enum AVCodecID audio_codec;    /**< default audio codec */
    enum AVCodecID video_codec;    /**< default video codec */
    enum AVCodecID subtitle_codec; /**< default subtitle codec */
    int flags;
    const struct AVCodecTag *const *codec_tag;
    const AVClass *priv_class; ///< AVClass for the private context
#if FF_API_AVIOFORMAT
#define ff_const59
#else
#define ff_const59 const
#endif
    const struct AVOutputFormat *next;
    int priv_data_size;
    int (*write_header)(struct AVFormatContext *);
    int (*write_packet)(struct AVFormatContext *, AVPacket *pkt);
    int (*write_trailer)(struct AVFormatContext *);
    int (*interleave_packet)(struct AVFormatContext *, AVPacket *out,
                             AVPacket *in, int flush);
    int (*query_codec)(enum AVCodecID id, int std_compliance);

    void (*get_output_timestamp)(struct AVFormatContext *s, int stream,
                                 int64_t *dts, int64_t *wall);
    int (*control_message)(struct AVFormatContext *s, int type,
                           void *data, size_t data_size);

    int (*write_uncoded_frame)(struct AVFormatContext *, int stream_index,
                               AVFrame **frame, unsigned flags);
    int (*get_device_list)(struct AVFormatContext *s, struct AVDeviceInfoList *device_list);
    int (*create_device_capabilities)(struct AVFormatContext *s, struct AVDeviceCapabilitiesQuery *caps);
    int (*free_device_capabilities)(struct AVFormatContext *s, struct AVDeviceCapabilitiesQuery *caps);
    enum AVCodecID data_codec; /**< default data codec */
    int (*init)(struct AVFormatContext *);
    void (*deinit)(struct AVFormatContext *);
    int (*check_bitstream)(struct AVFormatContext *, const AVPacket *pkt);
} AVOutputFormat;

typedef float FFTSample;
typedef float FFTDouble;

typedef struct FFTComplex
{
    FFTSample re, im;
} FFTComplex;

enum fft_permutation_type
{
    FF_FFT_PERM_DEFAULT,
    FF_FFT_PERM_SWAP_LSBS,
    FF_FFT_PERM_AVX,
};

enum mdct_permutation_type
{
    FF_MDCT_PERM_NONE,
    FF_MDCT_PERM_INTERLEAVE,
};

struct FFTContext
{
    int nbits;
    int inverse;
    uint16_t *revtab;
    FFTComplex *tmp_buf;
    int mdct_size; /* size of MDCT (i.e. number of input data * 2) */
    int mdct_bits; /* n = 2^nbits */
    /* pre/post rotation tables */
    FFTSample *tcos;
    FFTSample *tsin;
    /**
     * Do the permutation needed BEFORE calling fft_calc().
     */
    void (*fft_permute)(struct FFTContext *s, FFTComplex *z);
    /**
     * Do a complex FFT with the parameters defined in ff_fft_init(). The
     * input data must be permuted before. No 1.0/sqrt(n) normalization is done.
     */
    void (*fft_calc)(struct FFTContext *s, FFTComplex *z);
    void (*imdct_calc)(struct FFTContext *s, FFTSample *output, const FFTSample *input);
    void (*imdct_half)(struct FFTContext *s, FFTSample *output, const FFTSample *input);
    void (*mdct_calc)(struct FFTContext *s, FFTSample *output, const FFTSample *input);
    void (*mdct_calcw)(struct FFTContext *s, FFTDouble *output, const FFTSample *input);
    enum fft_permutation_type fft_permutation;
    enum mdct_permutation_type mdct_permutation;
    uint32_t *revtab32;
};
typedef struct FFTContext FFTContext;
struct RDFTContext
{
    int nbits;
    int inverse;
    int sign_convention;
    /* pre/post rotation tables */
    const FFTSample *tcos;
    const FFTSample *tsin;
    int negative_sin;
    FFTContext fft;
    void (*rdft_calc)(struct RDFTContext *s, FFTSample *z);
};

typedef struct RDFTContext RDFTContext;

struct SwsContext;

typedef struct DitherDSPContext {
    /**
     * Convert samples from flt to s16 with added dither noise.
     *
     * @param dst    destination float array, range -0.5 to 0.5
     * @param src    source int array, range INT_MIN to INT_MAX.
     * @param dither float dither noise array
     * @param len    number of samples
     */
    void (*quantize)(int16_t *dst, const float *src, float *dither, int len);

    int ptr_align;      ///< src and dst constraints for quantize()
    int samples_align;  ///< len constraints for quantize()

    /**
     * Convert dither noise from int to float with triangular distribution.
     *
     * @param dst  destination float array, range -0.5 to 0.5
     *             constraints: 32-byte aligned
     * @param src0 source int array, range INT_MIN to INT_MAX.
     *             the array size is len * 2
     *             constraints: 32-byte aligned
     * @param len  number of output noise samples
     *             constraints: multiple of 16
     */
    void (*dither_int_to_float)(float *dst, int *src0, int len);
} DitherDSPContext;

#define AVRESAMPLE_MAX_CHANNELS 32

typedef struct ChannelMapInfo {
    int channel_map[AVRESAMPLE_MAX_CHANNELS];   /**< source index of each output channel, -1 if not remapped */
    int do_remap;                               /**< remap needed */
    int channel_copy[AVRESAMPLE_MAX_CHANNELS];  /**< dest index to copy from */
    int do_copy;                                /**< copy needed */
    int channel_zero[AVRESAMPLE_MAX_CHANNELS];  /**< dest index to zero */
    int do_zero;                                /**< zeroing needed */
    int input_map[AVRESAMPLE_MAX_CHANNELS];     /**< dest index of each input channel */
} ChannelMapInfo;

typedef struct AVLFG {
    unsigned int state[64];
    int index;
} AVLFG;

typedef struct DitherState {
    int mute;
    unsigned int seed;
    AVLFG lfg;
    float *noise_buf;
    int noise_buf_size;
    int noise_buf_ptr;
    float dither_a[4];
    float dither_b[4];
} DitherState;

struct AudioData {
    const AVClass *class;               /**< AVClass for logging            */
    uint8_t *data[AVRESAMPLE_MAX_CHANNELS]; /**< data plane pointers        */
    uint8_t *buffer;                    /**< data buffer                    */
    unsigned int buffer_size;           /**< allocated buffer size          */
    int allocated_samples;              /**< number of samples the buffer can hold */
    int nb_samples;                     /**< current number of samples      */
    enum AVSampleFormat sample_fmt;     /**< sample format                  */
    int channels;                       /**< channel count                  */
    int allocated_channels;             /**< allocated channel count        */
    int is_planar;                      /**< sample format is planar        */
    int planes;                         /**< number of data planes          */
    int sample_size;                    /**< bytes per sample               */
    int stride;                         /**< sample byte offset within a plane */
    int read_only;                      /**< data is read-only              */
    int allow_realloc;                  /**< realloc is allowed             */
    int ptr_align;                      /**< minimum data pointer alignment */
    int samples_align;                  /**< allocated samples alignment    */
    const char *name;                   /**< name for debug logging         */
};

typedef struct AudioData AudioData;

typedef struct AVAudioResampleContext AVAudioResampleContext;

enum attribute_deprecated AVResampleDitherMethod {
    AV_RESAMPLE_DITHER_NONE,            /**< Do not use dithering */
    AV_RESAMPLE_DITHER_RECTANGULAR,     /**< Rectangular Dither */
    AV_RESAMPLE_DITHER_TRIANGULAR,      /**< Triangular Dither*/
    AV_RESAMPLE_DITHER_TRIANGULAR_HP,   /**< Triangular Dither with High Pass */
    AV_RESAMPLE_DITHER_TRIANGULAR_NS,   /**< Triangular Dither with Noise Shaping */
    AV_RESAMPLE_DITHER_NB,              /**< Number of dither types. Not part of ABI. */
};

typedef struct DitherContext DitherContext;

enum ConvFuncType {
    CONV_FUNC_TYPE_FLAT,
    CONV_FUNC_TYPE_INTERLEAVE,
    CONV_FUNC_TYPE_DEINTERLEAVE,
};

typedef void (conv_func_flat)(uint8_t *out, const uint8_t *in, int len);

typedef void (conv_func_interleave)(uint8_t *out, uint8_t *const *in,
                                    int len, int channels);

typedef void (conv_func_deinterleave)(uint8_t **out, const uint8_t *in, int len,
                                      int channels);
struct AudioConvert {
    AVAudioResampleContext *avr;
    DitherContext *dc;
    enum AVSampleFormat in_fmt;
    enum AVSampleFormat out_fmt;
    int apply_map;
    int channels;
    int planes;
    int ptr_align;
    int samples_align;
    int has_optimized_func;
    const char *func_descr;
    const char *func_descr_generic;
    enum ConvFuncType func_type;
    conv_func_flat         *conv_flat;
    conv_func_flat         *conv_flat_generic;
    conv_func_interleave   *conv_interleave;
    conv_func_interleave   *conv_interleave_generic;
    conv_func_deinterleave *conv_deinterleave;
    conv_func_deinterleave *conv_deinterleave_generic;
};

typedef struct AudioConvert AudioConvert;

struct DitherContext {
    DitherDSPContext  ddsp;
    enum AVResampleDitherMethod method;
    int apply_map;
    ChannelMapInfo *ch_map_info;

    int mute_dither_threshold;  // threshold for disabling dither
    int mute_reset_threshold;   // threshold for resetting noise shaping
    const float *ns_coef_b;     // noise shaping coeffs
    const float *ns_coef_a;     // noise shaping coeffs

    int channels;
    DitherState *state;         // dither states for each channel

    AudioData *flt_data;        // input data in fltp
    AudioData *s16_data;        // dithered output in s16p
    AudioConvert *ac_in;        // converter for input to fltp
    AudioConvert *ac_out;       // converter for s16p to s16 (if needed)

    void (*quantize)(int16_t *dst, const float *src, float *dither, int len);
    int samples_align;
};

enum attribute_deprecated AVMixCoeffType {
    AV_MIX_COEFF_TYPE_Q8,   /** 16-bit 8.8 fixed-point                      */
    AV_MIX_COEFF_TYPE_Q15,  /** 32-bit 17.15 fixed-point                    */
    AV_MIX_COEFF_TYPE_FLT,  /** floating-point                              */
    AV_MIX_COEFF_TYPE_NB,   /** Number of coeff types. Not part of ABI      */
};

typedef void (mix_func)(uint8_t **src, void **matrix, int len, int out_ch,
                        int in_ch);

struct AudioMix {
    AVAudioResampleContext *avr;
    enum AVSampleFormat fmt;
    enum AVMixCoeffType coeff_type;
    uint64_t in_layout;
    uint64_t out_layout;
    int in_channels;
    int out_channels;

    int ptr_align;
    int samples_align;
    int has_optimized_func;
    const char *func_descr;
    const char *func_descr_generic;
    mix_func *mix;
    mix_func *mix_generic;

    int in_matrix_channels;
    int out_matrix_channels;
    int output_zero[AVRESAMPLE_MAX_CHANNELS];
    int input_skip[AVRESAMPLE_MAX_CHANNELS];
    int output_skip[AVRESAMPLE_MAX_CHANNELS];
    int16_t *matrix_q8[AVRESAMPLE_MAX_CHANNELS];
    int32_t *matrix_q15[AVRESAMPLE_MAX_CHANNELS];
    float   *matrix_flt[AVRESAMPLE_MAX_CHANNELS];
    void   **matrix;
};

typedef struct AudioMix AudioMix;

enum attribute_deprecated AVResampleFilterType {
    AV_RESAMPLE_FILTER_TYPE_CUBIC,              /**< Cubic */
    AV_RESAMPLE_FILTER_TYPE_BLACKMAN_NUTTALL,   /**< Blackman Nuttall Windowed Sinc */
    AV_RESAMPLE_FILTER_TYPE_KAISER,             /**< Kaiser Windowed Sinc */
};


typedef struct AVFifoBuffer {
    uint8_t *buffer;
    uint8_t *rptr, *wptr, *end;
    uint32_t rndx, wndx;
} AVFifoBuffer;

struct AVAudioFifo {
    AVFifoBuffer **buf;             /**< single buffer for interleaved, per-channel buffers for planar */
    int nb_buffers;                 /**< number of buffers */
    int nb_samples;                 /**< number of samples currently in the FIFO */
    int allocated_samples;          /**< current allocated size, in samples */

    int channels;                   /**< number of channels */
    enum AVSampleFormat sample_fmt; /**< sample format */
    int sample_size;                /**< size, in bytes, of one sample in a buffer */
};

typedef struct AVAudioFifo AVAudioFifo;

typedef struct ResampleContext {
    const AVClass *class;
    AVAudioResampleContext *avr;
    AVDictionary *options;

    int resampling;
    int64_t next_pts;
    int64_t next_in_pts;

    /* set by filter_frame() to signal an output frame to request_frame() */
    int got_output;
} ResampleContext;

enum AVMatrixEncoding {
    AV_MATRIX_ENCODING_NONE,
    AV_MATRIX_ENCODING_DOLBY,
    AV_MATRIX_ENCODING_DPLII,
    AV_MATRIX_ENCODING_DPLIIX,
    AV_MATRIX_ENCODING_DPLIIZ,
    AV_MATRIX_ENCODING_DOLBYEX,
    AV_MATRIX_ENCODING_DOLBYHEADPHONE,
    AV_MATRIX_ENCODING_NB
};

enum RemapPoint {
    REMAP_NONE,
    REMAP_IN_COPY,
    REMAP_IN_CONVERT,
    REMAP_OUT_COPY,
    REMAP_OUT_CONVERT,
};

struct AVAudioResampleContext {
    const AVClass *av_class;        /**< AVClass for logging and AVOptions  */

    uint64_t in_channel_layout;                 /**< input channel layout   */
    enum AVSampleFormat in_sample_fmt;          /**< input sample format    */
    int in_sample_rate;                         /**< input sample rate      */
    uint64_t out_channel_layout;                /**< output channel layout  */
    enum AVSampleFormat out_sample_fmt;         /**< output sample format   */
    int out_sample_rate;                        /**< output sample rate     */
    enum AVSampleFormat internal_sample_fmt;    /**< internal sample format */
    enum AVMixCoeffType mix_coeff_type;         /**< mixing coefficient type */
    double center_mix_level;                    /**< center mix level       */
    double surround_mix_level;                  /**< surround mix level     */
    double lfe_mix_level;                       /**< lfe mix level          */
    int normalize_mix_level;                    /**< enable mix level normalization */
    int force_resampling;                       /**< force resampling       */
    int filter_size;                            /**< length of each FIR filter in the resampling filterbank relative to the cutoff frequency */
    int phase_shift;                            /**< log2 of the number of entries in the resampling polyphase filterbank */
    int linear_interp;                          /**< if 1 then the resampling FIR filter will be linearly interpolated */
    double cutoff;                              /**< resampling cutoff frequency. 1.0 corresponds to half the output sample rate */
    enum AVResampleFilterType filter_type;      /**< resampling filter type */
    int kaiser_beta;                            /**< beta value for Kaiser window (only applicable if filter_type == AV_FILTER_TYPE_KAISER) */
    enum AVResampleDitherMethod dither_method;  /**< dither method          */

    int in_channels;        /**< number of input channels                   */
    int out_channels;       /**< number of output channels                  */
    int resample_channels;  /**< number of channels used for resampling     */
    int downmix_needed;     /**< downmixing is needed                       */
    int upmix_needed;       /**< upmixing is needed                         */
    int mixing_needed;      /**< either upmixing or downmixing is needed    */
    int resample_needed;    /**< resampling is needed                       */
    int in_convert_needed;  /**< input sample format conversion is needed   */
    int out_convert_needed; /**< output sample format conversion is needed  */
    int in_copy_needed;     /**< input data copy is needed                  */

    AudioData *in_buffer;           /**< buffer for converted input         */
    AudioData *resample_out_buffer; /**< buffer for output from resampler   */
    AudioData *out_buffer;          /**< buffer for converted output        */
    AVAudioFifo *out_fifo;          /**< FIFO for output samples            */

    AudioConvert *ac_in;        /**< input sample format conversion context  */
    AudioConvert *ac_out;       /**< output sample format conversion context */
    ResampleContext *resample;  /**< resampling context                      */
    AudioMix *am;               /**< channel mixing context                  */
    enum AVMatrixEncoding matrix_encoding;      /**< matrixed stereo encoding */

    /**
     * mix matrix
     * only used if avresample_set_matrix() is called before avresample_open()
     */
    double *mix_matrix;

    int use_channel_map;
    enum RemapPoint remap_point;
    ChannelMapInfo ch_map_info;
};

typedef struct ResampleContext * (* resample_init_func)(struct ResampleContext *c, int out_rate, int in_rate, int filter_size, int phase_shift, int linear,
                                    double cutoff, enum AVSampleFormat format, enum SwrFilterType filter_type, double kaiser_beta, double precision, int cheby, int exact_rational);
typedef void    (* resample_free_func)(struct ResampleContext **c);
typedef int     (* multiple_resample_func)(struct ResampleContext *c, AudioData *dst, int dst_size, AudioData *src, int src_size, int *consumed);
typedef int     (* resample_flush_func)(struct SwrContext *c);
typedef int     (* set_compensation_func)(struct ResampleContext *c, int sample_delta, int compensation_distance);
typedef int64_t (* get_delay_func)(struct SwrContext *s, int64_t base);
typedef int     (* invert_initial_buffer_func)(struct ResampleContext *c, AudioData *dst, const AudioData *src, int src_size, int *dst_idx, int *dst_count);
typedef int64_t (* get_out_samples_func)(struct SwrContext *s, int in_samples);
struct Resampler {
  resample_init_func            init;
  resample_free_func            free;
  multiple_resample_func        multiple_resample;
  resample_flush_func           flush;
  set_compensation_func         set_compensation;
  get_delay_func                get_delay;
  invert_initial_buffer_func    invert_initial_buffer;
  get_out_samples_func          get_out_samples;
};
#if ARCH_X86_64
typedef int64_t integer;
#else
typedef int integer;
#endif
typedef void (mix_1_1_func_type)(void *out, const void *in, void *coeffp, integer index, integer len);
typedef void (mix_2_1_func_type)(void *out, const void *in1, const void *in2, void *coeffp, integer index1, integer index2, integer len);
typedef void (mix_any_func_type)(uint8_t **out, const uint8_t **in1, void *coeffp, integer len);
#define SWR_CH_MAX 64
struct SwrContext {
    const AVClass *av_class;                        ///< AVClass used for AVOption and av_log()
    int log_level_offset;                           ///< logging level offset
    void *log_ctx;                                  ///< parent logging context
    enum AVSampleFormat  in_sample_fmt;             ///< input sample format
    enum AVSampleFormat int_sample_fmt;             ///< internal sample format (AV_SAMPLE_FMT_FLTP or AV_SAMPLE_FMT_S16P)
    enum AVSampleFormat out_sample_fmt;             ///< output sample format
    int64_t  in_ch_layout;                          ///< input channel layout
    int64_t out_ch_layout;                          ///< output channel layout
    int      in_sample_rate;                        ///< input sample rate
    int     out_sample_rate;                        ///< output sample rate
    int flags;                                      ///< miscellaneous flags such as SWR_FLAG_RESAMPLE
    float slev;                                     ///< surround mixing level
    float clev;                                     ///< center mixing level
    float lfe_mix_level;                            ///< LFE mixing level
    float rematrix_volume;                          ///< rematrixing volume coefficient
    float rematrix_maxval;                          ///< maximum value for rematrixing output
    int matrix_encoding;                            /**< matrixed stereo encoding */
    const int *channel_map;                         ///< channel index (or -1 if muted channel) map
    int used_ch_count;                              ///< number of used input channels (mapped channel count if channel_map, otherwise in.ch_count)
    int engine;
    int user_in_ch_count;                           ///< User set input channel count
    int user_out_ch_count;                          ///< User set output channel count
    int user_used_ch_count;                         ///< User set used channel count
    int64_t user_in_ch_layout;                      ///< User set input channel layout
    int64_t user_out_ch_layout;                     ///< User set output channel layout
    enum AVSampleFormat user_int_sample_fmt;        ///< User set internal sample format
    int user_dither_method;                         ///< User set dither method
    struct DitherContext dither;
    int filter_size;                                /**< length of each FIR filter in the resampling filterbank relative to the cutoff frequency */
    int phase_shift;                                /**< log2 of the number of entries in the resampling polyphase filterbank */
    int linear_interp;                              /**< if 1 then the resampling FIR filter will be linearly interpolated */
    int exact_rational;                             /**< if 1 then enable non power of 2 phase_count */
    double cutoff;                                  /**< resampling cutoff frequency (swr: 6dB point; soxr: 0dB point). 1.0 corresponds to half the output sample rate */
    int filter_type;                                /**< swr resampling filter type */
    double kaiser_beta;                                /**< swr beta value for Kaiser window (only applicable if filter_type == AV_FILTER_TYPE_KAISER) */
    double precision;                               /**< soxr resampling precision (in bits) */
    int cheby;                                      /**< soxr: if 1 then passband rolloff will be none (Chebyshev) & irrational ratio approximation precision will be higher */
    float min_compensation;                         ///< swr minimum below which no compensation will happen
    float min_hard_compensation;                    ///< swr minimum below which no silence inject / sample drop will happen
    float soft_compensation_duration;               ///< swr duration over which soft compensation is applied
    float max_soft_compensation;                    ///< swr maximum soft compensation in seconds over soft_compensation_duration
    float async;                                    ///< swr simple 1 parameter async, similar to ffmpegs -async
    int64_t firstpts_in_samples;                    ///< swr first pts in samples
    int resample_first;                             ///< 1 if resampling must come first, 0 if rematrixing
    int rematrix;                                   ///< flag to indicate if rematrixing is needed (basically if input and output layouts mismatch)
    int rematrix_custom;                            ///< flag to indicate that a custom matrix has been defined
    AudioData in;                                   ///< input audio data
    AudioData postin;                               ///< post-input audio data: used for rematrix/resample
    AudioData midbuf;                               ///< intermediate audio data (postin/preout)
    AudioData preout;                               ///< pre-output audio data: used for rematrix/resample
    AudioData out;                                  ///< converted output audio data
    AudioData in_buffer;                            ///< cached audio data (convert and resample purpose)
    AudioData silence;                              ///< temporary with silence
    AudioData drop_temp;                            ///< temporary used to discard output
    int in_buffer_index;                            ///< cached buffer position
    int in_buffer_count;                            ///< cached buffer length
    int resample_in_constraint;                     ///< 1 if the input end was reach before the output end, 0 otherwise
    int flushed;                                    ///< 1 if data is to be flushed and no further input is expected
    int64_t outpts;                                 ///< output PTS
    int64_t firstpts;                               ///< first PTS
    int drop_output;                                ///< number of output samples to drop
    double delayed_samples_fixup;                   ///< soxr 0.1.1: needed to fixup delayed_samples after flush has been called.
    struct AudioConvert *in_convert;                ///< input conversion context
    struct AudioConvert *out_convert;               ///< output conversion context
    struct AudioConvert *full_convert;              ///< full conversion context (single conversion for input and output)
    struct ResampleContext *resample;               ///< resampling context
    struct Resampler const *resampler;              ///< resampler virtual function table
    double matrix[SWR_CH_MAX][SWR_CH_MAX];          ///< floating point rematrixing coefficients
    float matrix_flt[SWR_CH_MAX][SWR_CH_MAX];       ///< single precision floating point rematrixing coefficients
    uint8_t *native_matrix;
    uint8_t *native_one;
    uint8_t *native_simd_one;
    uint8_t *native_simd_matrix;
    int32_t matrix32[SWR_CH_MAX][SWR_CH_MAX];       ///< 17.15 fixed point rematrixing coefficients
    uint8_t matrix_ch[SWR_CH_MAX][SWR_CH_MAX+1];    ///< Lists of input channels per output channel that have non zero rematrixing coefficients
    mix_1_1_func_type *mix_1_1_f;
    mix_1_1_func_type *mix_1_1_simd;
    mix_2_1_func_type *mix_2_1_f;
    mix_2_1_func_type *mix_2_1_simd;
    mix_any_func_type *mix_any_f;

    /* TODO: callbacks for ASM optimizations */
};
typedef struct SwrContext SwrContext;

typedef struct AVFilterContext AVFilterContext;

struct AVFilterChannelLayouts {
    uint64_t *channel_layouts;  ///< list of channel layouts
    int    nb_channel_layouts;  ///< number of channel layouts
    char all_layouts;           ///< accept any known channel layout
    char all_counts;            ///< accept any channel layout or count

    unsigned refcount;          ///< number of references to this list
    struct AVFilterChannelLayouts ***refs; ///< references to this list
};
typedef struct AVFilterChannelLayouts AVFilterChannelLayouts;

struct AVFilterFormats {
    unsigned nb_formats;        ///< number of formats
    int *formats;               ///< list of media formats

    unsigned refcount;          ///< number of references to this list
    struct AVFilterFormats ***refs; ///< references to this list
};
typedef struct AVFilterFormats AVFilterFormats;


#define FF_INTERNAL_FIELDS 1
typedef struct FFFrameQueueGlobal {
    char dummy; /* C does not allow empty structs */
} FFFrameQueueGlobal;
typedef struct FFFrameBucket {
    AVFrame *frame;
} FFFrameBucket;
typedef struct FFFrameQueue {
    FFFrameBucket *queue;
    size_t allocated;
    size_t tail;
    size_t queued;
    FFFrameBucket first_bucket;
    uint64_t total_frames_head;
    uint64_t total_frames_tail;
    uint64_t total_samples_head;
    uint64_t total_samples_tail;
    int samples_skipped;
} FFFrameQueue;
typedef struct AVFilterFormatsConfig {
    AVFilterFormats *formats;
    AVFilterFormats  *samplerates;
    AVFilterChannelLayouts  *channel_layouts;
} AVFilterFormatsConfig;
typedef struct AVFilterPad AVFilterPad;
struct AVFilterLink {
    AVFilterContext *src;       ///< source filter
    AVFilterPad *srcpad;        ///< output pad on the source filter
    AVFilterContext *dst;       ///< dest filter
    AVFilterPad *dstpad;        ///< input pad on the dest filter
    enum AVMediaType type;      ///< filter media type
    /* These parameters apply only to video */
    int w;                      ///< agreed upon image width
    int h;                      ///< agreed upon image height
    AVRational sample_aspect_ratio; ///< agreed upon sample aspect ratio
    /* These parameters apply only to audio */
    uint64_t channel_layout;    ///< channel layout of current buffer (see libavutil/channel_layout.h)
    int sample_rate;            ///< samples per second
    int format;                 ///< agreed upon media format
    AVRational time_base;
    AVFilterFormatsConfig incfg;
    AVFilterFormatsConfig outcfg;
    /** stage of the initialization of the link properties (dimensions, etc) */
    enum {
        AVLINK_UNINIT = 0,      ///< not started
        AVLINK_STARTINIT,       ///< started, but incomplete
        AVLINK_INIT             ///< complete
    } init_state;
    struct AVFilterGraph *graph;
    int64_t current_pts;
    int64_t current_pts_us;
    int age_index;
    AVRational frame_rate;
    AVFrame *partial_buf;
    int partial_buf_size;
    int min_samples;
    int max_samples;
    int channels;
    unsigned flags;
    int64_t frame_count_in, frame_count_out;
    void *frame_pool;
    int frame_wanted_out;
    AVBufferRef *hw_frames_ctx;

#ifndef FF_INTERNAL_FIELDS
    char reserved[0xF000];
#else /* FF_INTERNAL_FIELDS */
    FFFrameQueue fifo;
    int frame_blocked_in;
    int status_in;
    int64_t status_in_pts;
    int status_out;
#endif /* FF_INTERNAL_FIELDS */

};
typedef struct AVFilterLink AVFilterLink;
typedef struct AVFilterFormats AVFilterFormats;
typedef struct AVFilterInternal AVFilterInternal;
typedef struct AVFilterGraphInternal AVFilterGraphInternal;

enum AVMediaType av_buffersink_get_type(const AVFilterContext *ctx);
AVRational av_buffersink_get_time_base(const AVFilterContext *ctx);
int av_buffersink_get_format(const AVFilterContext *ctx);

AVRational av_buffersink_get_frame_rate(const AVFilterContext *ctx);
int av_buffersink_get_w(const AVFilterContext *ctx);
int av_buffersink_get_h(const AVFilterContext *ctx);
AVRational av_buffersink_get_sample_aspect_ratio(const AVFilterContext *ctx);

int av_buffersink_get_channels(const AVFilterContext *ctx);
uint64_t av_buffersink_get_channel_layout(const AVFilterContext *ctx);
int av_buffersink_get_sample_rate(const AVFilterContext *ctx);

AVBufferRef *av_buffersink_get_hw_frames_ctx(const AVFilterContext *ctx);

typedef struct AVFilterInOut
{
    char *name;
    AVFilterContext *filter_ctx;
    int pad_idx;
    struct AVFilterInOut *next;
} AVFilterInOut;

typedef struct AVFilter
{
    const char *name;
    const char *description;
    const AVFilterPad *inputs;
    const AVFilterPad *outputs;
    const AVClass *priv_class;
    int flags;
    int (*preinit)(AVFilterContext *ctx);
    int (*init)(AVFilterContext *ctx);
    int (*init_dict)(AVFilterContext *ctx, AVDictionary **options);
    void (*uninit)(AVFilterContext *ctx);
    int (*query_formats)(AVFilterContext *);
    int priv_size;      ///< size of private data to allocate for the filter
    int flags_internal; ///< Additional flags for avfilter internal use only.
    struct AVFilter *next;
    int (*process_command)(AVFilterContext *, const char *cmd, const char *arg, char *res, int res_len, int flags);
    int (*init_opaque)(AVFilterContext *ctx, void *opaque);
    int (*activate)(AVFilterContext *ctx);
} AVFilter;

struct AVFilterContext
{
    const AVClass *av_class;     ///< needed for av_log() and filters common options
    const AVFilter *filter;      ///< the AVFilter of which this is an instance
    char *name;                  ///< name of this filter instance
    AVFilterPad *input_pads;     ///< array of input pads
    AVFilterLink **inputs;       ///< array of pointers to input links
    unsigned nb_inputs;          ///< number of input pads
    AVFilterPad *output_pads;    ///< array of output pads
    AVFilterLink **outputs;      ///< array of pointers to output links
    unsigned nb_outputs;         ///< number of output pads
    void *priv;                  ///< private data for use by the filter
    struct AVFilterGraph *graph; ///< filtergraph this filter belongs to
    int thread_type;
    AVFilterInternal *internal;
    struct AVFilterCommand *command_queue;
    char *enable_str;   ///< enable expression string
    void *enable;       ///< parsed expression (AVExpr*)
    double *var_values; ///< variable values for the enable expression
    int is_disabled;    ///< the enabled state from the last expression evaluation

    AVBufferRef *hw_device_ctx;
    int nb_threads;
    unsigned ready;
    int extra_hw_frames;
};

typedef int(avfilter_action_func)(AVFilterContext *ctx, void *arg, int jobnr, int nb_jobs);

typedef int(avfilter_execute_func)(AVFilterContext *ctx, avfilter_action_func *func,
                                   void *arg, int *ret, int nb_jobs);

struct AVFilterGraphInternal {
    void *thread;
    avfilter_execute_func *thread_execute;
    FFFrameQueueGlobal frame_queues;
};
typedef struct AVFilterGraph
{
    const AVClass *av_class;
    AVFilterContext **filters;
    unsigned nb_filters;
    char *scale_sws_opts; ///< sws options to use for the auto-inserted scale filters
    int thread_type;
    int nb_threads;
    AVFilterGraphInternal *internal;
    void *opaque;
    avfilter_execute_func *execute;
    char *aresample_swr_opts; ///< swr options to use for the auto-inserted aresample filters, Access ONLY through AVOptions
    AVFilterLink **sink_links;
    int sink_links_count;
    unsigned disable_auto_convert;
} AVFilterGraph;

typedef struct AVProbeData
{
    const char *filename;
    unsigned char *buf;    /**< Buffer must have AVPROBE_PADDING_SIZE of extra allocated bytes filled with zero. */
    int buf_size;          /**< Size of buf except extra allocated bytes */
    const char *mime_type; /**< mime_type, when known. */
} AVProbeData;

typedef struct AVIndexEntry
{
    int64_t pos;
    int64_t timestamp;

    int flags : 2;
    int size : 30;    //Yeah, trying to keep the size of this small to reduce memory requirements (it is 24 vs. 32 bytes due to possible 8-byte alignment).
    int min_distance; /**< Minimum distance between this and the previous keyframe, used to avoid unneeded searching. */
} AVIndexEntry;

typedef struct FFFrac
{
    int64_t val, num, den;
} FFFrac;

struct AVStreamInternal
{
    int reorder;
    AVBSFContext *bsfc;
    int bitstream_checked;
    AVCodecContext *avctx;
    int avctx_inited;
    enum AVCodecID orig_codec_id;
    struct
    {
        AVBSFContext *bsf;
        AVPacket *pkt;
        int inited;
    } extract_extradata;
    int need_context_update;
    int is_intra_only;
    FFFrac *priv_pts;
};

typedef struct AVStreamInternal AVStreamInternal;

typedef struct AVStream
{
    int index; /**< stream index in AVFormatContext */
    int id;
    void *priv_data;
    AVRational time_base;
    int64_t start_time;
    int64_t duration;
    int64_t nb_frames;      ///< number of frames in this stream if known or 0
    int disposition;        /**< AV_DISPOSITION_* bit field */
    enum AVDiscard discard; ///< Selects which packets can be discarded at will and do not need to be demuxed.
    AVRational sample_aspect_ratio;
    AVDictionary *metadata;
    AVRational avg_frame_rate;
    AVPacket attached_pic;
    AVPacketSideData *side_data;
    int nb_side_data;
    int event_flags;

    AVRational r_frame_rate;
    AVCodecParameters *codecpar;
    struct
    {
        int64_t last_dts;
        int64_t duration_gcd;
        int duration_count;
        int64_t rfps_duration_sum;
        double (*duration_error)[2][MAX_STD_TIMEBASES];
        int64_t codec_info_duration;
        int64_t codec_info_duration_fields;
        int frame_delay_evidence;
        int found_decoder;
        int64_t last_duration;
        int64_t fps_first_dts;
        int fps_first_dts_idx;
        int64_t fps_last_dts;
        int fps_last_dts_idx;
    } * info;
    int pts_wrap_bits; /**< number of bits in pts (used for wrapping control) */
    int64_t first_dts;
    int64_t cur_dts;
    int64_t last_IP_pts;
    int last_IP_duration;
    int probe_packets;
    int codec_info_nb_frames;
    enum AVStreamParseType need_parsing;
    struct AVCodecParserContext *parser;
    struct AVPacketList *last_in_packet_buffer;
    AVProbeData probe_data;
    int64_t pts_buffer[MAX_REORDER_DELAY + 1];
    AVIndexEntry *index_entries; /**< Only used if the format does not
                                    support seeking natively. */
    int nb_index_entries;
    unsigned int index_entries_allocated_size;
    int stream_identifier;
    int program_num;
    int pmt_version;
    int pmt_stream_idx;
    int64_t interleaver_chunk_size;
    int64_t interleaver_chunk_duration;
    int request_probe;
    int skip_to_keyframe;
    int skip_samples;
    int64_t start_skip_samples;
    int64_t first_discard_sample;
    int64_t last_discard_sample;
    int nb_decoded_frames;
    int64_t mux_ts_offset;
    int64_t pts_wrap_reference;
    int pts_wrap_behavior;
    int update_initial_durations_done;
    int64_t pts_reorder_error[MAX_REORDER_DELAY + 1];
    uint8_t pts_reorder_error_count[MAX_REORDER_DELAY + 1];
    int64_t last_dts_for_order_check;
    uint8_t dts_ordered;
    uint8_t dts_misordered;
    int inject_global_side_data;
    AVRational display_aspect_ratio;
    AVStreamInternal *internal;
} AVStream;

typedef struct AVFormatContext
{
    const AVClass *av_class;
    const struct AVInputFormat *iformat;
    const struct AVOutputFormat *oformat;
    void *priv_data;
    AVIOContext *pb;
    int ctx_flags;
    unsigned int nb_streams;
    AVStream **streams;
    char *url;
    int64_t start_time;
    int64_t duration;
    int64_t bit_rate;
    unsigned int packet_size;
    int max_delay;
    int flags;
    int64_t probesize;
    int64_t max_analyze_duration;
    const uint8_t *key;
    int keylen;
    unsigned int nb_programs;
    AVProgram **programs;
    enum AVCodecID video_codec_id;
    enum AVCodecID audio_codec_id;
    enum AVCodecID subtitle_codec_id;
    unsigned int max_index_size;
    unsigned int max_picture_buffer;
    unsigned int nb_chapters;
    AVChapter **chapters;
    AVDictionary *metadata;
    int64_t start_time_realtime;
    int fps_probe_size;
    int error_recognition;
    AVIOInterruptCB interrupt_callback;
    int debug;
    int64_t max_interleave_delta;
    int strict_std_compliance;
    int event_flags;
    int max_ts_probe;
    int avoid_negative_ts;
    int ts_id;
    int audio_preload;
    int max_chunk_duration;
    int max_chunk_size;
    int use_wallclock_as_timestamps;
    int avio_flags;
    enum AVDurationEstimationMethod duration_estimation_method;
    int64_t skip_initial_bytes;
    unsigned int correct_ts_overflow;
    int seek2any;
    int flush_packets;
    int probe_score;
    int format_probesize;
    char *codec_whitelist;
    char *format_whitelist;
    AVFormatInternal *internal;
    int io_repositioned;
    AVCodec *video_codec;
    AVCodec *audio_codec;
    AVCodec *subtitle_codec;
    AVCodec *data_codec;
    int metadata_header_padding;
    void *opaque;
    av_format_control_message control_message_cb;
    int64_t output_ts_offset;
    uint8_t *dump_separator;
    enum AVCodecID data_codec_id;
    char *protocol_whitelist;
    int (*io_open)(struct AVFormatContext *s, AVIOContext **pb, const char *url, int flags, AVDictionary **options);
    void (*io_close)(struct AVFormatContext *s, AVIOContext *pb);
    char *protocol_blacklist;
    int max_streams;
    int skip_estimate_duration_from_pts;
    int max_probe_packets;
} AVFormatContext;

typedef struct AVDeviceCapabilitiesQuery
{
    const AVClass *av_class;
    AVFormatContext *device_context;
    enum AVCodecID codec;
    enum AVSampleFormat sample_format;
    enum AVPixelFormat pixel_format;
    int sample_rate;
    int channels;
    int64_t channel_layout;
    int window_width;
    int window_height;
    int frame_width;
    int frame_height;
    AVRational fps;
} AVDeviceCapabilitiesQuery;

typedef struct AVInputFormat
{
    const char *name;
    const char *long_name;
    int flags;
    const char *extensions;
    const struct AVCodecTag *const *codec_tag;
    const AVClass *priv_class; ///< AVClass for the private context
    const char *mime_type;
    const struct AVInputFormat *next;
    int raw_codec_id;
    int priv_data_size;
    int (*read_probe)(const AVProbeData *);
    int (*read_header)(struct AVFormatContext *);
    int (*read_packet)(struct AVFormatContext *, AVPacket *pkt);
    int (*read_close)(struct AVFormatContext *);
    int (*read_seek)(struct AVFormatContext *,
                     int stream_index, int64_t timestamp, int flags);
    int64_t (*read_timestamp)(struct AVFormatContext *s, int stream_index,
                              int64_t *pos, int64_t pos_limit);
    int (*read_play)(struct AVFormatContext *);
    int (*read_pause)(struct AVFormatContext *);
    int (*read_seek2)(struct AVFormatContext *s, int stream_index, int64_t min_ts, int64_t ts, int64_t max_ts, int flags);
    int (*get_device_list)(struct AVFormatContext *s, struct AVDeviceInfoList *device_list);
    int (*create_device_capabilities)(struct AVFormatContext *s, struct AVDeviceCapabilitiesQuery *caps);
    int (*free_device_capabilities)(struct AVFormatContext *s, struct AVDeviceCapabilitiesQuery *caps);
} AVInputFormat;

typedef struct VideoState
{
    SDL_Thread *read_tid;
    AVInputFormat *iformat;
    int abort_request;
    int force_refresh;
    int paused;
    int last_paused;
    int queue_attachments_req;
    int seek_req;
    int seek_flags;
    int64_t seek_pos;
    int64_t seek_rel;
    int read_pause_return;
    AVFormatContext *ic;
    int realtime;
    Clock audclk;
    Clock vidclk;
    Clock extclk;
    FrameQueue pictq;
    FrameQueue subpq;
    FrameQueue sampq;
    Decoder auddec;
    Decoder viddec;
    Decoder subdec;
    int audio_stream;
    int av_sync_type;
    double audio_clock;
    int audio_clock_serial;
    double audio_diff_cum; /* used for AV difference average computation */
    double audio_diff_avg_coef;
    double audio_diff_threshold;
    int audio_diff_avg_count;
    AVStream *audio_st;
    PacketQueue audioq;
    int audio_hw_buf_size;
    uint8_t *audio_buf;
    uint8_t *audio_buf1;
    unsigned int audio_buf_size; /* in bytes */
    unsigned int audio_buf1_size;
    int audio_buf_index; /* in bytes */
    int audio_write_buf_size;
    int audio_volume;
    int muted;
    struct AudioParams audio_src;
    struct AudioParams audio_filter_src;
    struct AudioParams audio_tgt;
    struct SwrContext *swr_ctx;
    int frame_drops_early;
    int frame_drops_late;
    enum ShowMode show_mode;
    int16_t sample_array[SAMPLE_ARRAY_SIZE];
    int sample_array_index;
    int last_i_start;
    RDFTContext *rdft;
    int rdft_bits;
    FFTSample *rdft_data;
    int xpos;
    double last_vis_time;
    SDL_Texture *vis_texture;
    SDL_Texture *sub_texture;
    SDL_Texture *vid_texture;
    int subtitle_stream;
    AVStream *subtitle_st;
    PacketQueue subtitleq;
    double frame_timer;
    double frame_last_returned_time;
    double frame_last_filter_delay;
    int video_stream;
    AVStream *video_st;
    PacketQueue videoq;
    double max_frame_duration; // maximum duration of a frame - above this, we consider the jump a timestamp discontinuity
    struct SwsContext *img_convert_ctx;
    struct SwsContext *sub_convert_ctx;
    int eof;
    char *filename;
    int width, height, xleft, ytop;
    int step;
    int vfilter_idx;
    AVFilterContext *in_video_filter;  // the first filter in the video chain
    AVFilterContext *out_video_filter; // the last filter in the video chain
    AVFilterContext *in_audio_filter;  // the first filter in the audio chain
    AVFilterContext *out_audio_filter; // the last filter in the audio chain
    AVFilterGraph *agraph;             // audio filter graph
    int last_video_stream, last_audio_stream, last_subtitle_stream;
    SDL_cond *continue_read_thread;
} VideoState;

typedef struct DynBuffer
{
    int pos, size, allocated_size;
    uint8_t *buffer;
    int io_buffer_size;
    uint8_t io_buffer[1];
} DynBuffer;

typedef struct PerThreadContext
{
    struct FrameThreadContext *parent;

    pthread_t thread;
    int thread_init;
    pthread_cond_t input_cond;    ///< Used to wait for a new packet from the main thread.
    pthread_cond_t progress_cond; ///< Used by child threads to wait for progress to change.
    pthread_cond_t output_cond;   ///< Used by the main thread to wait for frames to finish.

    pthread_mutex_t mutex;          ///< Mutex used to protect the contents of the PerThreadContext.
    pthread_mutex_t progress_mutex; ///< Mutex used to protect frame progress values and progress_cond.

    AVCodecContext *avctx; ///< Context used to decode packets passed to this thread.

    AVPacket avpkt; ///< Input packet (for decoding) or output (for encoding).

    AVFrame *frame; ///< Output frame (for decoding) or input (for encoding).
    int got_frame;  ///< The output of got_picture_ptr from the last avcodec_decode_video() call.
    int result;     ///< The result of the last codec decode/encode() call.

    atomic_int state;

    /**
     * Array of frames passed to ff_thread_release_buffer().
     * Frames are released after all threads referencing them are finished.
     */
    AVFrame **released_buffers;
    int num_released_buffers;
    int released_buffers_allocated;

    AVFrame *requested_frame; ///< AVFrame the codec passed to get_buffer()
    int requested_flags;      ///< flags passed to get_buffer() for requested_frame

    const enum AVPixelFormat *available_formats; ///< Format array for get_format()
    enum AVPixelFormat result_format;            ///< get_format() result

    int die; ///< Set when the thread should exit.

    int hwaccel_serializing;
    int async_serializing;

    atomic_int debug_threads; ///< Set if the FF_DEBUG_THREADS option is set.
} PerThreadContext;

typedef struct FrameThreadContext
{
    PerThreadContext *threads;     ///< The contexts for each thread.
    PerThreadContext *prev_thread; ///< The last thread submit_packet() was called on.

    pthread_mutex_t buffer_mutex; ///< Mutex used to protect get/release_buffer().
    /**
     * This lock is used for ensuring threads run in serial when hwaccel
     * is used.
     */
    pthread_mutex_t hwaccel_mutex;
    pthread_mutex_t async_mutex;
    pthread_cond_t async_cond;
    int async_lock;

    int next_decoding; ///< The next context to submit a packet to.
    int next_finished; ///< The next context to return output from.

    int delaying; /**<
                                    * Set for the first N packets, where N is the number of threads.
                                    * While it is set, ff_thread_en/decode_frame won't return any results.
                                    */
} FrameThreadContext;

typedef struct SDL_Window SDL_Window;
struct SDL_Renderer;
typedef struct SDL_Renderer SDL_Renderer;
/* options specified by the user */
static AVInputFormat *file_iformat;
static const char *input_filename;
static const char *window_title;
static int default_width = 640;
static int default_height = 480;
static int screen_width = 0;
static int screen_height = 0;
static int screen_left = SDL_WINDOWPOS_CENTERED;
static int screen_top = SDL_WINDOWPOS_CENTERED;
static int audio_disable;
static int video_disable;
static int subtitle_disable;
static const char *wanted_stream_spec[AVMEDIA_TYPE_NB] = {0};
static int seek_by_bytes = -1;
static float seek_interval = 10;
static int display_disable;
static int borderless;
static int alwaysontop;
static int startup_volume = 100;
static int show_status = -1;
static int av_sync_type = AV_SYNC_AUDIO_MASTER;
static int64_t start_time = AV_NOPTS_VALUE;
static int64_t duration = AV_NOPTS_VALUE;
static int fast = 0;
static int genpts = 0;
static int lowres = 0;
static int decoder_reorder_pts = -1;
static int autoexit;
static int exit_on_keydown;
static int exit_on_mousedown;
static int loop = 1;
static int framedrop = -1;
static int infinite_buffer = -1;
static enum ShowMode show_mode = SHOW_MODE_NONE;
static const char *audio_codec_name;
static const char *subtitle_codec_name;
static const char *video_codec_name;
double rdftspeed = 0.02;
static int64_t cursor_last_shown;
static int cursor_hidden = 0;
static const char **vfilters_list = NULL;
static int nb_vfilters = 0;
static char *afilters = NULL;
static int autorotate = 1;
static int find_stream_info = 1;
static int filter_nbthreads = 0;
static int is_full_screen;
static int64_t audio_callback_time;
static AVPacket flush_pkt;
static SDL_Window *window;
static SDL_Renderer *renderer;

static SDL_RendererInfo renderer_info = {0};

typedef uint32_t Uint32;
typedef Uint32 SDL_AudioDeviceID;
static SDL_AudioDeviceID audio_dev;

struct TextureFormatEntry
{
    enum AVPixelFormat format;
    int texture_fmt;
};

struct AVFilterInternal {
    avfilter_execute_func *execute;
};

static const struct TextureFormatEntry sdl_texture_format_map[] = {
    {AV_PIX_FMT_RGB8, SDL_PIXELFORMAT_RGB332},
    {AV_PIX_FMT_RGB444, SDL_PIXELFORMAT_RGB444},
    {AV_PIX_FMT_RGB555, SDL_PIXELFORMAT_RGB555},
    {AV_PIX_FMT_BGR555, SDL_PIXELFORMAT_BGR555},
    {AV_PIX_FMT_RGB565, SDL_PIXELFORMAT_RGB565},
    {AV_PIX_FMT_BGR565, SDL_PIXELFORMAT_BGR565},
    {AV_PIX_FMT_RGB24, SDL_PIXELFORMAT_RGB24},
    {AV_PIX_FMT_BGR24, SDL_PIXELFORMAT_BGR24},
    {AV_PIX_FMT_0RGB32, SDL_PIXELFORMAT_RGB888},
    {AV_PIX_FMT_0BGR32, SDL_PIXELFORMAT_BGR888},
    {AV_PIX_FMT_NE(RGB0, 0BGR), SDL_PIXELFORMAT_RGBX8888},
    {AV_PIX_FMT_NE(BGR0, 0RGB), SDL_PIXELFORMAT_BGRX8888},
    {AV_PIX_FMT_RGB32, SDL_PIXELFORMAT_ARGB8888},
    {AV_PIX_FMT_RGB32_1, SDL_PIXELFORMAT_RGBA8888},
    {AV_PIX_FMT_BGR32, SDL_PIXELFORMAT_ABGR8888},
    {AV_PIX_FMT_BGR32_1, SDL_PIXELFORMAT_BGRA8888},
    {AV_PIX_FMT_YUV420P, SDL_PIXELFORMAT_IYUV},
    {AV_PIX_FMT_YUYV422, SDL_PIXELFORMAT_YUY2},
    {AV_PIX_FMT_UYVY422, SDL_PIXELFORMAT_UYVY},
    {AV_PIX_FMT_NONE, SDL_PIXELFORMAT_UNKNOWN},
};

typedef int (*SwsFunc)(struct SwsContext *context, const uint8_t *src[],
                       int srcStride[], int srcSliceY, int srcSliceH,
                       uint8_t *dst[], int dstStride[]);

typedef void (*yuv2planar1_fn)(const int16_t *src, uint8_t *dest, int dstW,
                               const uint8_t *dither, int offset);

typedef void (*yuv2planarX_fn)(const int16_t *filter, int filterSize,
                               const int16_t **src, uint8_t *dest, int dstW,
                               const uint8_t *dither, int offset);

typedef void (*yuv2interleavedX_fn)(enum AVPixelFormat dstFormat,
                                    const uint8_t *chrDither,
                                    const int16_t *chrFilter,
                                    int chrFilterSize,
                                    const int16_t **chrUSrc,
                                    const int16_t **chrVSrc,
                                    uint8_t *dest, int dstW);

typedef void (*yuv2packed1_fn)(struct SwsContext *c, const int16_t *lumSrc,
                               const int16_t *chrUSrc[2],
                               const int16_t *chrVSrc[2],
                               const int16_t *alpSrc, uint8_t *dest,
                               int dstW, int uvalpha, int y);

typedef void (*yuv2packed2_fn)(struct SwsContext *c, const int16_t *lumSrc[2],
                               const int16_t *chrUSrc[2],
                               const int16_t *chrVSrc[2],
                               const int16_t *alpSrc[2],
                               uint8_t *dest,
                               int dstW, int yalpha, int uvalpha, int y);

typedef void (*yuv2packedX_fn)(struct SwsContext *c, const int16_t *lumFilter,
                               const int16_t **lumSrc, int lumFilterSize,
                               const int16_t *chrFilter,
                               const int16_t **chrUSrc,
                               const int16_t **chrVSrc, int chrFilterSize,
                               const int16_t **alpSrc, uint8_t *dest,
                               int dstW, int y);

typedef void (*yuv2anyX_fn)(struct SwsContext *c, const int16_t *lumFilter,
                            const int16_t **lumSrc, int lumFilterSize,
                            const int16_t *chrFilter,
                            const int16_t **chrUSrc,
                            const int16_t **chrVSrc, int chrFilterSize,
                            const int16_t **alpSrc, uint8_t **dest,
                            int dstW, int y);

typedef struct SwsContext
{
    const AVClass *av_class;
    SwsFunc swscale;
    int srcW;    ///< Width  of source      luma/alpha planes.
    int srcH;    ///< Height of source      luma/alpha planes.
    int dstH;    ///< Height of destination luma/alpha planes.
    int chrSrcW; ///< Width  of source      chroma     planes.
    int chrSrcH; ///< Height of source      chroma     planes.
    int chrDstW; ///< Width  of destination chroma     planes.
    int chrDstH; ///< Height of destination chroma     planes.
    int lumXInc, chrXInc;
    int lumYInc, chrYInc;
    enum AVPixelFormat dstFormat; ///< Destination pixel format.
    enum AVPixelFormat srcFormat; ///< Source      pixel format.
    int dstFormatBpp;             ///< Number of bits per pixel of the destination pixel format.
    int srcFormatBpp;             ///< Number of bits per pixel of the source      pixel format.
    int dstBpc, srcBpc;
    int chrSrcHSubSample; ///< Binary logarithm of horizontal subsampling factor between luma/alpha and chroma planes in source      image.
    int chrSrcVSubSample; ///< Binary logarithm of vertical   subsampling factor between luma/alpha and chroma planes in source      image.
    int chrDstHSubSample; ///< Binary logarithm of horizontal subsampling factor between luma/alpha and chroma planes in destination image.
    int chrDstVSubSample; ///< Binary logarithm of vertical   subsampling factor between luma/alpha and chroma planes in destination image.
    int vChrDrop;         ///< Binary logarithm of extra vertical subsampling factor in source image chroma planes specified by user.
    int sliceDir;         ///< Direction that slices are fed to the scaler (1 = top-to-bottom, -1 = bottom-to-top).
    double param[2];      ///< Input parameters for scaling algorithms that need them.

    struct SwsContext *cascaded_context[3];
    int cascaded_tmpStride[4];
    uint8_t *cascaded_tmp[4];
    int cascaded1_tmpStride[4];
    uint8_t *cascaded1_tmp[4];
    int cascaded_mainindex;
    double gamma_value;
    int gamma_flag;
    int is_internal_gamma;
    uint16_t *gamma;
    uint16_t *inv_gamma;
    int numDesc;
    int descIndex[2];
    int numSlice;
    struct SwsSlice *slice;
    struct SwsFilterDescriptor *desc;
    uint32_t pal_yuv[256];
    uint32_t pal_rgb[256];
    float uint2float_lut[256];
    int lastInLumBuf; ///< Last scaled horizontal luma/alpha line from source in the ring buffer.
    int lastInChrBuf; ///< Last scaled horizontal chroma     line from source in the ring buffer.
    //@}
    uint8_t *formatConvBuffer;
    int needAlpha;
    int16_t *hLumFilter;    ///< Array of horizontal filter coefficients for luma/alpha planes.
    int16_t *hChrFilter;    ///< Array of horizontal filter coefficients for chroma     planes.
    int16_t *vLumFilter;    ///< Array of vertical   filter coefficients for luma/alpha planes.
    int16_t *vChrFilter;    ///< Array of vertical   filter coefficients for chroma     planes.
    int32_t *hLumFilterPos; ///< Array of horizontal filter starting positions for each dst[i] for luma/alpha planes.
    int32_t *hChrFilterPos; ///< Array of horizontal filter starting positions for each dst[i] for chroma     planes.
    int32_t *vLumFilterPos; ///< Array of vertical   filter starting positions for each dst[i] for luma/alpha planes.
    int32_t *vChrFilterPos; ///< Array of vertical   filter starting positions for each dst[i] for chroma     planes.
    int hLumFilterSize;     ///< Horizontal filter size for luma/alpha pixels.
    int hChrFilterSize;     ///< Horizontal filter size for chroma     pixels.
    int vLumFilterSize;     ///< Vertical   filter size for luma/alpha pixels.
    int vChrFilterSize;     ///< Vertical   filter size for chroma     pixels.
    //@}
    int lumMmxextFilterCodeSize;  ///< Runtime-generated MMXEXT horizontal fast bilinear scaler code size for luma/alpha planes.
    int chrMmxextFilterCodeSize;  ///< Runtime-generated MMXEXT horizontal fast bilinear scaler code size for chroma planes.
    uint8_t *lumMmxextFilterCode; ///< Runtime-generated MMXEXT horizontal fast bilinear scaler code for luma/alpha planes.
    uint8_t *chrMmxextFilterCode; ///< Runtime-generated MMXEXT horizontal fast bilinear scaler code for chroma planes.
    int canMMXEXTBeUsed;
    int warned_unuseable_bilinear;
    int dstY;       ///< Last destination vertical line output from last slice.
    int flags;      ///< Flags passed by the user to select scaler algorithm, optimizations, subsampling, etc...
    void *yuvTable; // pointer to the yuv->rgb table start so it can be freed()
    // alignment ensures the offset can be added in a single
    // instruction on e.g. ARM
    DECLARE_ALIGNED(16, int, table_gV)
    [256 + 2 * YUVRGB_TABLE_HEADROOM];
    uint8_t *table_rV[256 + 2 * YUVRGB_TABLE_HEADROOM];
    uint8_t *table_gU[256 + 2 * YUVRGB_TABLE_HEADROOM];
    uint8_t *table_bU[256 + 2 * YUVRGB_TABLE_HEADROOM];
    DECLARE_ALIGNED(16, int32_t, input_rgb2yuv_table)
    [16 + 40 * 4]; // This table can contain both C and SIMD formatted values, the C vales are always at the XY_IDX points
    int *dither_error[4];
    int contrast, brightness, saturation; // for sws_getColorspaceDetails
    int srcColorspaceTable[4];
    int dstColorspaceTable[4];
    int srcRange; ///< 0 = MPG YUV range, 1 = JPG YUV range (source      image).
    int dstRange; ///< 0 = MPG YUV range, 1 = JPG YUV range (destination image).
    int src0Alpha;
    int dst0Alpha;
    int srcXYZ;
    int dstXYZ;
    int src_h_chr_pos;
    int dst_h_chr_pos;
    int src_v_chr_pos;
    int dst_v_chr_pos;
    int yuv2rgb_y_offset;
    int yuv2rgb_y_coeff;
    int yuv2rgb_v2r_coeff;
    int yuv2rgb_v2g_coeff;
    int yuv2rgb_u2g_coeff;
    int yuv2rgb_u2b_coeff;

    DECLARE_ALIGNED(8, uint64_t, redDither);
    DECLARE_ALIGNED(8, uint64_t, greenDither);
    DECLARE_ALIGNED(8, uint64_t, blueDither);

    DECLARE_ALIGNED(8, uint64_t, yCoeff);
    DECLARE_ALIGNED(8, uint64_t, vrCoeff);
    DECLARE_ALIGNED(8, uint64_t, ubCoeff);
    DECLARE_ALIGNED(8, uint64_t, vgCoeff);
    DECLARE_ALIGNED(8, uint64_t, ugCoeff);
    DECLARE_ALIGNED(8, uint64_t, yOffset);
    DECLARE_ALIGNED(8, uint64_t, uOffset);
    DECLARE_ALIGNED(8, uint64_t, vOffset);
    int32_t lumMmxFilter[4 * MAX_FILTER_SIZE];
    int32_t chrMmxFilter[4 * MAX_FILTER_SIZE];
    int dstW; ///< Width  of destination luma/alpha planes.
    DECLARE_ALIGNED(8, uint64_t, esp);
    DECLARE_ALIGNED(8, uint64_t, vRounder);
    DECLARE_ALIGNED(8, uint64_t, u_temp);
    DECLARE_ALIGNED(8, uint64_t, v_temp);
    DECLARE_ALIGNED(8, uint64_t, y_temp);
    int32_t alpMmxFilter[4 * MAX_FILTER_SIZE];
    // alignment of these values is not necessary, but merely here
    // to maintain the same offset across x8632 and x86-64. Once we
    // use proper offset macros in the asm, they can be removed.
    DECLARE_ALIGNED(8, ptrdiff_t, uv_off);   ///< offset (in pixels) between u and v planes
    DECLARE_ALIGNED(8, ptrdiff_t, uv_offx2); ///< offset (in bytes) between u and v planes
    DECLARE_ALIGNED(8, uint16_t, dither16)
    [8];
    DECLARE_ALIGNED(8, uint32_t, dither32)
    [8];

    const uint8_t *chrDither8, *lumDither8;

#if HAVE_ALTIVEC
    vector signed short CY;
    vector signed short CRV;
    vector signed short CBU;
    vector signed short CGU;
    vector signed short CGV;
    vector signed short OY;
    vector unsigned short CSHIFT;
    vector signed short *vYCoeffsBank, *vCCoeffsBank;
#endif

    int use_mmx_vfilter;

    int16_t *xyzgamma;
    int16_t *rgbgamma;
    int16_t *xyzgammainv;
    int16_t *rgbgammainv;
    int16_t xyz2rgb_matrix[3][4];
    int16_t rgb2xyz_matrix[3][4];
    yuv2planar1_fn yuv2plane1;
    yuv2planarX_fn yuv2planeX;
    yuv2interleavedX_fn yuv2nv12cX;
    yuv2packed1_fn yuv2packed1;
    yuv2packed2_fn yuv2packed2;
    yuv2packedX_fn yuv2packedX;
    yuv2anyX_fn yuv2anyX;

    void (*lumToYV12)(uint8_t *dst, const uint8_t *src, const uint8_t *src2, const uint8_t *src3,
                      int width, uint32_t *pal);
    void (*alpToYV12)(uint8_t *dst, const uint8_t *src, const uint8_t *src2, const uint8_t *src3,
                      int width, uint32_t *pal);
    void (*chrToYV12)(uint8_t *dstU, uint8_t *dstV,
                      const uint8_t *src1, const uint8_t *src2, const uint8_t *src3,
                      int width, uint32_t *pal);

    void (*readLumPlanar)(uint8_t *dst, const uint8_t *src[4], int width, int32_t *rgb2yuv);
    void (*readChrPlanar)(uint8_t *dstU, uint8_t *dstV, const uint8_t *src[4],
                          int width, int32_t *rgb2yuv);
    void (*readAlpPlanar)(uint8_t *dst, const uint8_t *src[4], int width, int32_t *rgb2yuv);
    void (*hyscale_fast)(struct SwsContext *c,
                         int16_t *dst, int dstWidth,
                         const uint8_t *src, int srcW, int xInc);
    void (*hcscale_fast)(struct SwsContext *c,
                         int16_t *dst1, int16_t *dst2, int dstWidth,
                         const uint8_t *src1, const uint8_t *src2,
                         int srcW, int xInc);
    void (*hyScale)(struct SwsContext *c, int16_t *dst, int dstW,
                    const uint8_t *src, const int16_t *filter,
                    const int32_t *filterPos, int filterSize);
    void (*hcScale)(struct SwsContext *c, int16_t *dst, int dstW,
                    const uint8_t *src, const int16_t *filter,
                    const int32_t *filterPos, int filterSize);
    void (*lumConvertRange)(int16_t *dst, int width);
    void (*chrConvertRange)(int16_t *dst1, int16_t *dst2, int width);
    int needs_hcscale; ///< Set if there are chroma planes to be converted.
    SwsDither dither;

    SwsAlphaBlend alphablend;
} SwsContext;

typedef struct SwsVector
{
    double *coeff; ///< pointer to the list of coefficients
    int length;    ///< number of coefficients in the vector
} SwsVector;

typedef struct SwsFilter
{
    SwsVector *lumH;
    SwsVector *lumV;
    SwsVector *chrH;
    SwsVector *chrV;
} SwsFilter;



typedef struct ADTSContext
{
    AVClass *class;
    int write_adts;
    int objecttype;
    int sample_rate_index;
    int channel_conf;
    int pce_size;
    int apetag;
    int id3v2tag;
    int mpeg_id;
    uint8_t pce_data[MAX_PCE_SIZE];
} ADTSContext;

typedef struct MPEG4AudioConfig
{
    int object_type;
    int sampling_index;
    int sample_rate;
    int chan_config;
    int sbr; ///< -1 implicit, 1 presence
    int ext_object_type;
    int ext_sampling_index;
    int ext_sample_rate;
    int ext_chan_config;
    int channels;
    int ps; ///< -1 implicit, 1 presence
    int frame_length_short;
} MPEG4AudioConfig;

typedef struct AC3BitAllocParameters {
    int sr_code;
    int sr_shift;
    int slow_gain, slow_decay, fast_decay, db_per_bit, floor;
    int cpl_fast_leak, cpl_slow_leak;
} AC3BitAllocParameters;



typedef struct BswapDSPContext {
    void (*bswap_buf)(uint32_t *dst, const uint32_t *src, int w);
    void (*bswap16_buf)(uint16_t *dst, const uint16_t *src, int len);
} BswapDSPContext;

typedef struct AVFixedDSPContext {
    void (*vector_fmul_window_scaled)(int16_t *dst, const int32_t *src0, const int32_t *src1, const int32_t *win, int len, uint8_t bits);
    void (*vector_fmul_window)(int32_t *dst, const int32_t *src0, const int32_t *src1, const int32_t *win, int len);
    void (*vector_fmul)(int *dst, const int *src0, const int *src1,int len);
    void (*vector_fmul_reverse)(int *dst, const int *src0, const int *src1, int len);
    void (*vector_fmul_add)(int *dst, const int *src0, const int *src1, const int *src2, int len);
    int (*scalarproduct_fixed)(const int *v1, const int *v2, int len);
    void (*butterflies_fixed)(int *av_restrict v1, int *av_restrict v2, int len);
} AVFixedDSPContext;


typedef struct AVFloatDSPContext {    
    void (*vector_fmul)(float *dst, const float *src0, const float *src1,
                        int len);
    void (*vector_fmac_scalar)(float *dst, const float *src, float mul,
                               int len);
    void (*vector_dmac_scalar)(double *dst, const double *src, double mul,
                               int len);
    void (*vector_fmul_scalar)(float *dst, const float *src, float mul,
                               int len);
    void (*vector_dmul_scalar)(double *dst, const double *src, double mul,
                               int len);
    void (*vector_fmul_window)(float *dst, const float *src0,
                               const float *src1, const float *win, int len);
    void (*vector_fmul_add)(float *dst, const float *src0, const float *src1,
                            const float *src2, int len);
    void (*vector_fmul_reverse)(float *dst, const float *src0,
                                const float *src1, int len);
    void (*butterflies_float)(float *av_restrict v1, float *av_restrict v2, int len);
    float (*scalarproduct_float)(const float *v1, const float *v2, int len);
    void (*vector_dmul)(double *dst, const double *src0, const double *src1,
                        int len);
} AVFloatDSPContext;

typedef struct AC3DSPContext {
    void (*ac3_exponent_min)(uint8_t *exp, int num_reuse_blocks, int nb_coefs);
    int (*ac3_max_msb_abs_int16)(const int16_t *src, int len);
    void (*ac3_lshift_int16)(int16_t *src, unsigned int len, unsigned int shift);
    void (*ac3_rshift_int32)(int32_t *src, unsigned int len, unsigned int shift);
    void (*float_to_fixed24)(int32_t *dst, const float *src, unsigned int len);
    void (*bit_alloc_calc_bap)(int16_t *mask, int16_t *psd, int start, int end,
                               int snr_offset, int floor,
                               const uint8_t *bap_tab, uint8_t *bap);
    void (*update_bap_counts)(uint16_t mant_cnt[16], uint8_t *bap, int len);
    int (*compute_mantissa_size)(uint16_t mant_cnt[6][16]);
    void (*extract_exponents)(uint8_t *exp, int32_t *coef, int nb_coefs);
    void (*sum_square_butterfly_int32)(int64_t sum[4], const int32_t *coef0,
                                       const int32_t *coef1, int len);
    void (*sum_square_butterfly_float)(float sum[4], const float *coef0,
                                       const float *coef1, int len);
    int out_channels;
    int in_channels;
    void (*downmix)(float **samples, float **matrix, int len);
    void (*downmix_fixed)(int32_t **samples, int16_t **matrix, int len);
    void (*apply_window_int16)(int16_t *output, const int16_t *input,
                               const int16_t *window, unsigned int len);
} AC3DSPContext;

#define AC3_OUTPUT_LFEON  8
#define SPX_MAX_BANDS    17
#define AC3_FRAME_BUFFER_SIZE 32768

typedef int                     INTFLOAT;
typedef int16_t                 SHORTFLOAT;

typedef struct FmtConvertContext {
    void (*int32_to_float_fmul_scalar)(float *dst, const int32_t *src,
                                       float mul, int len);
    void (*int32_to_float)(float *dst, const int32_t *src, intptr_t len);
    void (*int32_to_float_fmul_array8)(struct FmtConvertContext *c,
                                       float *dst, const int32_t *src,
                                       const float *mul, int len);

} FmtConvertContext;

typedef struct AC3DecodeContext
{
    AVClass *class;        ///< class for AVOptions
    AVCodecContext *avctx; ///< parent context
    GetBitContext gbc;     ///< bitstream reader
    int frame_type;                 ///< frame type                             (strmtyp)
    int substreamid;                ///< substream identification
    int superframe_size;            ///< current superframe size, in bytes
    int frame_size;                 ///< current frame size, in bytes
    int bit_rate;                   ///< stream bit rate, in bits-per-second
    int sample_rate;                ///< sample frequency, in Hz
    int num_blocks;                 ///< number of audio blocks
    int bitstream_id;               ///< bitstream id                           (bsid)
    int bitstream_mode;             ///< bitstream mode                         (bsmod)
    int channel_mode;               ///< channel mode                           (acmod)
    int lfe_on;                     ///< lfe channel in use
    int dialog_normalization[2];    ///< dialog level in dBFS                   (dialnorm)
    int compression_exists[2];      ///< compression field is valid for frame   (compre)
    int compression_gain[2];        ///< gain to apply for heavy compression    (compr)
    int channel_map;                ///< custom channel map                     (chanmap)
    int preferred_downmix;          ///< Preferred 2-channel downmix mode       (dmixmod)
    int center_mix_level;           ///< Center mix level index
    int center_mix_level_ltrt;      ///< Center mix level index for Lt/Rt       (ltrtcmixlev)
    int surround_mix_level;         ///< Surround mix level index
    int surround_mix_level_ltrt;    ///< Surround mix level index for Lt/Rt     (ltrtsurmixlev)
    int lfe_mix_level_exists;       ///< indicates if lfemixlevcod is specified (lfemixlevcode)
    int lfe_mix_level;              ///< LFE mix level index                    (lfemixlevcod)
    int eac3;                       ///< indicates if current frame is E-AC-3
    int eac3_frame_dependent_found; ///< bitstream has E-AC-3 dependent frame(s)
    int eac3_subsbtreamid_found;    ///< bitstream has E-AC-3 additional substream(s)
    int dolby_surround_mode;        ///< dolby surround mode                    (dsurmod)
    int dolby_surround_ex_mode;     ///< dolby surround ex mode                 (dsurexmod)
    int dolby_headphone_mode;       ///< dolby headphone mode                   (dheadphonmod)
                                    ///@}

    int preferred_stereo_downmix;
    float ltrt_center_mix_level;
    float ltrt_surround_mix_level;
    float loro_center_mix_level;
    float loro_surround_mix_level;
    int target_level; ///< target level in dBFS
    float level_gain[2];

    int snr_offset_strategy;   ///< SNR offset strategy                    (snroffststr)
    int block_switch_syntax;   ///< block switch syntax enabled            (blkswe)
    int dither_flag_syntax;    ///< dither flag syntax enabled             (dithflage)
    int bit_allocation_syntax; ///< bit allocation model syntax enabled    (bamode)
    int fast_gain_syntax;      ///< fast gain codes enabled                (frmfgaincode)
    int dba_syntax;            ///< delta bit allocation syntax enabled    (dbaflde)
    int skip_syntax;           ///< skip field syntax enabled              (skipflde)
                               ///@}

    int cpl_in_use[AC3_MAX_BLOCKS];          ///< coupling in use                        (cplinu)
    int cpl_strategy_exists[AC3_MAX_BLOCKS]; ///< coupling strategy exists               (cplstre)
    int channel_in_cpl[AC3_MAX_CHANNELS];    ///< channel in coupling                    (chincpl)
    int phase_flags_in_use;                  ///< phase flags in use                     (phsflginu)
    int phase_flags[AC3_MAX_CPL_BANDS];      ///< phase flags                            (phsflg)
    int num_cpl_bands;                       ///< number of coupling bands               (ncplbnd)
    uint8_t cpl_band_struct[AC3_MAX_CPL_BANDS];
    uint8_t cpl_band_sizes[AC3_MAX_CPL_BANDS];           ///< number of coeffs in each coupling band
    int firstchincpl;                                    ///< first channel in coupling
    int first_cpl_coords[AC3_MAX_CHANNELS];              ///< first coupling coordinates states      (firstcplcos)
    int cpl_coords[AC3_MAX_CHANNELS][AC3_MAX_CPL_BANDS]; ///< coupling coordinates      (cplco)
                                                         ///@}
    int spx_in_use;                             ///< spectral extension in use              (spxinu)
    uint8_t channel_uses_spx[AC3_MAX_CHANNELS]; ///< channel uses spectral extension        (chinspx)
    int8_t spx_atten_code[AC3_MAX_CHANNELS];    ///< spx attenuation code                   (spxattencod)
    int spx_src_start_freq;                     ///< spx start frequency bin
    int spx_dst_end_freq;                       ///< spx end frequency bin
    int spx_dst_start_freq;                     ///< spx starting frequency bin for copying (copystartmant)
                                                ///< the copy region ends at the start of the spx region.
    int num_spx_bands;                          ///< number of spx bands                    (nspxbnds)
    uint8_t spx_band_struct[SPX_MAX_BANDS];
    uint8_t spx_band_sizes[SPX_MAX_BANDS];                      ///< number of bins in each spx band
    uint8_t first_spx_coords[AC3_MAX_CHANNELS];                 ///< first spx coordinates states           (firstspxcos)
    INTFLOAT spx_noise_blend[AC3_MAX_CHANNELS][SPX_MAX_BANDS];  ///< spx noise blending factor  (nblendfact)
    INTFLOAT spx_signal_blend[AC3_MAX_CHANNELS][SPX_MAX_BANDS]; ///< spx signal blending factor (sblendfact)
                                                                ///@}
    int channel_uses_aht[AC3_MAX_CHANNELS];                            ///< channel AHT in use (chahtinu)
    int pre_mantissa[AC3_MAX_CHANNELS][AC3_MAX_COEFS][AC3_MAX_BLOCKS]; ///< pre-IDCT mantissas
                                                                       ///@}
    int fbw_channels;              ///< number of full-bandwidth channels
    int channels;                  ///< number of total channels
    int lfe_ch;                    ///< index of LFE channel
    SHORTFLOAT *downmix_coeffs[2]; ///< stereo downmix coefficients
    int downmixed;                 ///< indicates if coeffs are currently downmixed
    int output_mode;               ///< output channel configuration
    int prev_output_mode;          ///< output channel configuration for previous frame
    int out_channels;              ///< number of output channels
    int prev_bit_rate;             ///< stream bit rate, in bits-per-second for previous frame
                                   ///@}
    INTFLOAT dynamic_range[2];       ///< dynamic range
    INTFLOAT drc_scale;              ///< percentage of dynamic range compression to be applied
    int heavy_compression;           ///< apply heavy compression
    INTFLOAT heavy_dynamic_range[2]; ///< heavy dynamic range compression
                                     ///@}
    int start_freq[AC3_MAX_CHANNELS]; ///< start frequency bin                    (strtmant)
    int end_freq[AC3_MAX_CHANNELS];   ///< end frequency bin                      (endmant)
                                      ///@}
    int consistent_noise_generation; ///< seed noise generation with AC-3 frame on decode
                                     ///@}
    int num_rematrixing_bands; ///< number of rematrixing bands            (nrematbnd)
    int rematrixing_flags[4];  ///< rematrixing flags                      (rematflg)
                               ///@}

    int num_exp_groups[AC3_MAX_CHANNELS];               ///< Number of exponent groups      (nexpgrp)
    int8_t dexps[AC3_MAX_CHANNELS][AC3_MAX_COEFS];      ///< decoded exponents
    int exp_strategy[AC3_MAX_BLOCKS][AC3_MAX_CHANNELS]; ///< exponent strategies        (expstr)

    AC3BitAllocParameters bit_alloc_params;                 ///< bit allocation parameters
    int first_cpl_leak;                                     ///< first coupling leak state      (firstcplleak)
    int snr_offset[AC3_MAX_CHANNELS];                       ///< signal-to-noise ratio offsets  (snroffst)
    int fast_gain[AC3_MAX_CHANNELS];                        ///< fast gain values/SMR's         (fgain)
    uint8_t bap[AC3_MAX_CHANNELS][AC3_MAX_COEFS];           ///< bit allocation pointers
    int16_t psd[AC3_MAX_CHANNELS][AC3_MAX_COEFS];           ///< scaled exponents
    int16_t band_psd[AC3_MAX_CHANNELS][AC3_CRITICAL_BANDS]; ///< interpolated exponents
    int16_t mask[AC3_MAX_CHANNELS][AC3_CRITICAL_BANDS];     ///< masking curve values
    int dba_mode[AC3_MAX_CHANNELS];                         ///< delta bit allocation mode
    int dba_nsegs[AC3_MAX_CHANNELS];                        ///< number of delta segments
    uint8_t dba_offsets[AC3_MAX_CHANNELS][8];               ///< delta segment offsets
    uint8_t dba_lengths[AC3_MAX_CHANNELS][8];               ///< delta segment lengths
    uint8_t dba_values[AC3_MAX_CHANNELS][8];                ///< delta values for each segment

    int dither_flag[AC3_MAX_CHANNELS]; ///< dither flags                           (dithflg)
    AVLFG dith_state;                  ///< for dither generation

    int block_switch[AC3_MAX_CHANNELS]; ///< block switch flags                     (blksw)
    FFTContext imdct_512;               ///< for 512 sample IMDCT
    FFTContext imdct_256;               ///< for 256 sample IMDCT
    BswapDSPContext bdsp;
#if USE_FIXED
    AVFixedDSPContext *fdsp;
#else
    AVFloatDSPContext *fdsp;
#endif
    AC3DSPContext ac3dsp;
    FmtConvertContext fmt_conv; ///< optimized conversion functions

    SHORTFLOAT *outptr[AC3_MAX_CHANNELS];
    INTFLOAT *xcfptr[AC3_MAX_CHANNELS];
    INTFLOAT *dlyptr[AC3_MAX_CHANNELS];

    DECLARE_ALIGNED(16, int, fixed_coeffs)
    [AC3_MAX_CHANNELS][AC3_MAX_COEFS]; ///< fixed-point transform coefficients
    DECLARE_ALIGNED(32, INTFLOAT, transform_coeffs)
    [AC3_MAX_CHANNELS][AC3_MAX_COEFS]; ///< transform coefficients
    DECLARE_ALIGNED(32, INTFLOAT, delay)
    [EAC3_MAX_CHANNELS][AC3_BLOCK_SIZE]; ///< delay - added to the next block
    DECLARE_ALIGNED(32, INTFLOAT, window)[AC3_BLOCK_SIZE]; ///< window coefficients
    DECLARE_ALIGNED(32, INTFLOAT, tmp_output)[AC3_BLOCK_SIZE]; ///< temporary storage for output before windowing
    DECLARE_ALIGNED(32, SHORTFLOAT, output)[EAC3_MAX_CHANNELS][AC3_BLOCK_SIZE]; ///< output after imdct transform and windowing
    DECLARE_ALIGNED(32, uint8_t, input_buffer)[AC3_FRAME_BUFFER_SIZE + AV_INPUT_BUFFER_PADDING_SIZE]; ///< temp buffer to prevent overread
    DECLARE_ALIGNED(32, SHORTFLOAT, output_buffer)
    [EAC3_MAX_CHANNELS][AC3_BLOCK_SIZE * 6]; ///< final output buffer
} AC3DecodeContext;

typedef struct AADemuxContext
{
    AVClass *class;
    uint8_t *aa_fixed_key;
    int aa_fixed_key_len;
    int codec_second_size;
    int current_codec_second_size;
    int chapter_idx;
    struct AVTEA *tea_ctx;
    uint8_t file_key[16];
    int64_t current_chapter_size;
    int64_t content_start;
    int64_t content_end;
    int seek_offset;
} AADemuxContext;

typedef struct ID3v2ExtraMetaGEOB
{
    uint32_t datasize;
    uint8_t *mime_type;
    uint8_t *file_name;
    uint8_t *description;
    uint8_t *data;
} ID3v2ExtraMetaGEOB;

typedef struct ID3v2ExtraMetaAPIC
{
    AVBufferRef *buf;
    const char *type;
    uint8_t *description;
    enum AVCodecID id;
} ID3v2ExtraMetaAPIC;

typedef struct ID3v2ExtraMetaPRIV
{
    uint8_t *owner;
    uint8_t *data;
    uint32_t datasize;
} ID3v2ExtraMetaPRIV;

typedef struct ID3v2ExtraMetaCHAP
{
    uint8_t *element_id;
    uint32_t start, end;
    AVDictionary *meta;
} ID3v2ExtraMetaCHAP;

typedef struct ID3v2ExtraMeta
{
    const char *tag;
    struct ID3v2ExtraMeta *next;
    union
    {
        ID3v2ExtraMetaAPIC apic;
        ID3v2ExtraMetaCHAP chap;
        ID3v2ExtraMetaGEOB geob;
        ID3v2ExtraMetaPRIV priv;
    } data;
} ID3v2ExtraMeta;

typedef struct SampleFmtInfo {
    char name[8];
    int bits;
    int planar;
    enum AVSampleFormat altform; ///< planar<->packed alternative form
} SampleFmtInfo;

typedef struct BufferSourceContext {
    const AVClass    *class;
    AVRational        time_base;     ///< time_base to set in the output link
    AVRational        frame_rate;    ///< frame_rate to set in the output link
    unsigned          nb_failed_requests;
    int               w, h;
    enum AVPixelFormat  pix_fmt;
    AVRational        pixel_aspect;
#if FF_API_SWS_PARAM_OPTION
    char              *sws_param;
#endif

    AVBufferRef *hw_frames_ctx;
    int sample_rate;
    enum AVSampleFormat sample_fmt;
    int channels;
    uint64_t channel_layout;
    char    *channel_layout_str;

    int eof;
} BufferSourceContext;



typedef struct FrameDecodeData {
    int (*post_process)(void *logctx, AVFrame *frame);
    void *post_process_opaque;
    void (*post_process_opaque_free)(void *opaque);
    void *hwaccel_priv;
    void (*hwaccel_priv_free)(void *priv);
} FrameDecodeData;


struct AVBSFInternal {
    AVPacket *buffer_pkt;
    int eof;
};


struct AVFilterPad {
    const char *name;
    enum AVMediaType type;
    AVFrame *(*get_video_buffer)(AVFilterLink *link, int w, int h);
    AVFrame *(*get_audio_buffer)(AVFilterLink *link, int nb_samples);
    int (*filter_frame)(AVFilterLink *link, AVFrame *frame);
    int (*request_frame)(AVFilterLink *link);
    int (*config_props)(AVFilterLink *link);
    int needs_writable;
};

typedef struct ID3v2EncContext {
    int      version;       ///< ID3v2 minor version, either 3 or 4
    int64_t size_pos;       ///< offset of the tag total size
    int          len;       ///< size of the tag written so far
} ID3v2EncContext;

typedef struct BenchContext {
    const AVClass *class;
    int action;
    int64_t max, min;
    int64_t sum;
    int n;
} BenchContext;

typedef struct AVSliceThread AVSliceThread;
typedef struct WorkerContext {
    AVSliceThread   *ctx;
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
    pthread_t       thread;
    int             done;
} WorkerContext;


struct AVSliceThread {
    WorkerContext   *workers;
    int             nb_threads;
    int             nb_active_threads;
    int             nb_jobs;
    atomic_uint     first_job;
    atomic_uint     current_job;
    pthread_mutex_t done_mutex;
    pthread_cond_t  done_cond;
    int             done;
    int             finished;

    void            *priv;
    void            (*worker_func)(void *priv, int jobnr, int threadnr, int nb_jobs, int nb_threads);
    void            (*main_func)(void *priv);
};

typedef struct ThreadContext {
    AVFilterGraph *graph;
    AVSliceThread *thread;
    avfilter_action_func *func;
    AVFilterContext *ctx;
    void *arg;
    int   *rets;
} ThreadContext;

typedef struct BufferSinkContext {
    const AVClass *class;
    unsigned warning_limit;

    /* only used for video */
    enum AVPixelFormat *pixel_fmts;           ///< list of accepted pixel formats, must be terminated with -1
    int pixel_fmts_size;

    /* only used for audio */
    enum AVSampleFormat *sample_fmts;       ///< list of accepted sample formats, terminated by AV_SAMPLE_FMT_NONE
    int sample_fmts_size;
    int64_t *channel_layouts;               ///< list of accepted channel layouts, terminated by -1
    int channel_layouts_size;
    int *channel_counts;                    ///< list of accepted channel counts, terminated by -1
    int channel_counts_size;
    int all_channel_counts;
    int *sample_rates;                      ///< list of accepted sample rates, terminated by -1
    int sample_rates_size;

    AVFrame *peeked_frame;
} BufferSinkContext;

static const SampleFmtInfo sample_fmt_info[AV_SAMPLE_FMT_NB];

const char *av_default_item_name(void *ptr);
static const AVClass ac3_decoder_class;
AVCodec ff_ac3_fixed_decoder;
static int adts_write_packet(AVFormatContext *s, AVPacket *pkt);
static int adts_write_trailer(AVFormatContext *s);
static int adts_write_header(AVFormatContext *s);
static int adts_decode_extradata(AVFormatContext *s, ADTSContext *adts, const uint8_t *buf, int size);

int avio_get_dyn_buf(AVIOContext *s, uint8_t **pbuffer);
static int url_open_dyn_buf_internal(AVIOContext **s, int max_packet_size);
int avio_open_dyn_buf(AVIOContext **s);
void avpriv_align_put_bits(PutBitContext *s);

const AVProfile ff_aac_profiles[];
const AVProfile ff_dca_profiles[];
const AVProfile ff_dnxhd_profiles[];
const AVProfile ff_h264_profiles[];
const AVProfile ff_hevc_profiles[];
const AVProfile ff_jpeg2000_profiles[];
const AVProfile ff_mpeg2_video_profiles[];
const AVProfile ff_mpeg4_video_profiles[];
const AVProfile ff_vc1_profiles[];
const AVProfile ff_vp9_profiles[];
const AVProfile ff_av1_profiles[];
const AVProfile ff_sbc_profiles[];
const AVProfile ff_prores_profiles[];
const AVProfile ff_mjpeg_profiles[];
const AVProfile ff_arib_caption_profiles[];

AVInputFormat ff_aa_demuxer;
AVInputFormat ff_aac_demuxer;
AVInputFormat ff_aax_demuxer;
AVInputFormat ff_ac3_demuxer;
AVInputFormat ff_acm_demuxer;
AVInputFormat ff_act_demuxer;
AVInputFormat ff_adf_demuxer;
AVInputFormat ff_adp_demuxer;
AVInputFormat ff_ads_demuxer;
AVInputFormat ff_adx_demuxer;
AVInputFormat ff_aea_demuxer;
AVInputFormat ff_afc_demuxer;
AVInputFormat ff_aiff_demuxer;
AVInputFormat ff_aix_demuxer;
AVInputFormat ff_alp_demuxer;
AVInputFormat ff_amr_demuxer;
AVInputFormat ff_amrnb_demuxer;
AVInputFormat ff_amrwb_demuxer;
AVInputFormat ff_anm_demuxer;
AVInputFormat ff_apc_demuxer;
AVInputFormat ff_ape_demuxer;
AVInputFormat ff_apm_demuxer;
AVInputFormat ff_apng_demuxer;
AVInputFormat ff_aptx_demuxer;
AVInputFormat ff_aptx_hd_demuxer;
AVInputFormat ff_aqtitle_demuxer;
AVInputFormat ff_argo_asf_demuxer;
AVInputFormat ff_argo_brp_demuxer;
AVInputFormat ff_asf_demuxer;
AVInputFormat ff_asf_o_demuxer;
AVInputFormat ff_ass_demuxer;
AVInputFormat ff_ast_demuxer;
AVInputFormat ff_au_demuxer;
AVInputFormat ff_av1_demuxer;
AVInputFormat ff_avi_demuxer;
AVInputFormat ff_avr_demuxer;
AVInputFormat ff_avs_demuxer;
AVInputFormat ff_avs2_demuxer;
AVInputFormat ff_bethsoftvid_demuxer;
AVInputFormat ff_bfi_demuxer;
AVInputFormat ff_bintext_demuxer;
AVInputFormat ff_bink_demuxer;
AVInputFormat ff_bit_demuxer;
AVInputFormat ff_bmv_demuxer;
AVInputFormat ff_bfstm_demuxer;
AVInputFormat ff_brstm_demuxer;
AVInputFormat ff_boa_demuxer;
AVInputFormat ff_c93_demuxer;
AVInputFormat ff_caf_demuxer;
AVInputFormat ff_cavsvideo_demuxer;
AVInputFormat ff_cdg_demuxer;
AVInputFormat ff_cdxl_demuxer;
AVInputFormat ff_cine_demuxer;
AVInputFormat ff_codec2_demuxer;
AVInputFormat ff_codec2raw_demuxer;
AVInputFormat ff_concat_demuxer;
AVInputFormat ff_dash_demuxer;
AVInputFormat ff_data_demuxer;
AVInputFormat ff_daud_demuxer;
AVInputFormat ff_dcstr_demuxer;
AVInputFormat ff_derf_demuxer;
AVInputFormat ff_dfa_demuxer;
AVInputFormat ff_dhav_demuxer;
AVInputFormat ff_dirac_demuxer;
AVInputFormat ff_dnxhd_demuxer;
AVInputFormat ff_dsf_demuxer;
AVInputFormat ff_dsicin_demuxer;
AVInputFormat ff_dss_demuxer;
AVInputFormat ff_dts_demuxer;
AVInputFormat ff_dtshd_demuxer;
AVInputFormat ff_dv_demuxer;
AVInputFormat ff_dvbsub_demuxer;
AVInputFormat ff_dvbtxt_demuxer;
AVInputFormat ff_dxa_demuxer;
AVInputFormat ff_ea_demuxer;
AVInputFormat ff_ea_cdata_demuxer;
AVInputFormat ff_eac3_demuxer;
AVInputFormat ff_epaf_demuxer;
AVInputFormat ff_ffmetadata_demuxer;
AVInputFormat ff_filmstrip_demuxer;
AVInputFormat ff_fits_demuxer;
AVInputFormat ff_flac_demuxer;
AVInputFormat ff_flic_demuxer;
AVInputFormat ff_flv_demuxer;
AVInputFormat ff_live_flv_demuxer;
AVInputFormat ff_fourxm_demuxer;
AVInputFormat ff_frm_demuxer;
AVInputFormat ff_fsb_demuxer;
AVInputFormat ff_fwse_demuxer;
AVInputFormat ff_g722_demuxer;
AVInputFormat ff_g723_1_demuxer;
AVInputFormat ff_g726_demuxer;
AVInputFormat ff_g726le_demuxer;
AVInputFormat ff_g729_demuxer;
AVInputFormat ff_gdv_demuxer;
AVInputFormat ff_genh_demuxer;
AVInputFormat ff_gif_demuxer;
AVInputFormat ff_gsm_demuxer;
AVInputFormat ff_gxf_demuxer;
AVInputFormat ff_h261_demuxer;
AVInputFormat ff_h263_demuxer;
AVInputFormat ff_h264_demuxer;
AVInputFormat ff_hca_demuxer;
AVInputFormat ff_hcom_demuxer;
AVInputFormat ff_hevc_demuxer;
AVInputFormat ff_hls_demuxer;
AVInputFormat ff_hnm_demuxer;
AVInputFormat ff_ico_demuxer;
AVInputFormat ff_idcin_demuxer;
AVInputFormat ff_idf_demuxer;
AVInputFormat ff_iff_demuxer;
AVInputFormat ff_ifv_demuxer;
AVInputFormat ff_ilbc_demuxer;
AVInputFormat ff_image2_demuxer;
AVInputFormat ff_image2pipe_demuxer;
AVInputFormat ff_image2_alias_pix_demuxer;
AVInputFormat ff_image2_brender_pix_demuxer;
AVInputFormat ff_ingenient_demuxer;
AVInputFormat ff_ipmovie_demuxer;
AVInputFormat ff_ipu_demuxer;
AVInputFormat ff_ircam_demuxer;
AVInputFormat ff_iss_demuxer;
AVInputFormat ff_iv8_demuxer;
AVInputFormat ff_ivf_demuxer;
AVInputFormat ff_ivr_demuxer;
AVInputFormat ff_jacosub_demuxer;
AVInputFormat ff_jv_demuxer;
AVInputFormat ff_kux_demuxer;
AVInputFormat ff_kvag_demuxer;
AVInputFormat ff_lmlm4_demuxer;
AVInputFormat ff_loas_demuxer;
AVInputFormat ff_luodat_demuxer;
AVInputFormat ff_lrc_demuxer;
AVInputFormat ff_lvf_demuxer;
AVInputFormat ff_lxf_demuxer;
AVInputFormat ff_m4v_demuxer;
AVInputFormat ff_mca_demuxer;
AVInputFormat ff_mcc_demuxer;
AVInputFormat ff_matroska_demuxer;
AVInputFormat ff_mgsts_demuxer;
AVInputFormat ff_microdvd_demuxer;
AVInputFormat ff_mjpeg_demuxer;
AVInputFormat ff_mjpeg_2000_demuxer;
AVInputFormat ff_mlp_demuxer;
AVInputFormat ff_mlv_demuxer;
AVInputFormat ff_mm_demuxer;
AVInputFormat ff_mmf_demuxer;
AVInputFormat ff_mods_demuxer;
AVInputFormat ff_moflex_demuxer;
AVInputFormat ff_mov_demuxer;
AVInputFormat ff_mp3_demuxer;
AVInputFormat ff_mpc_demuxer;
AVInputFormat ff_mpc8_demuxer;
AVInputFormat ff_mpegps_demuxer;
AVInputFormat ff_mpegts_demuxer;
AVInputFormat ff_mpegtsraw_demuxer;
AVInputFormat ff_mpegvideo_demuxer;
AVInputFormat ff_mpjpeg_demuxer;
AVInputFormat ff_mpl2_demuxer;
AVInputFormat ff_mpsub_demuxer;
AVInputFormat ff_msf_demuxer;
AVInputFormat ff_msnwc_tcp_demuxer;
AVInputFormat ff_mtaf_demuxer;
AVInputFormat ff_mtv_demuxer;
AVInputFormat ff_musx_demuxer;
AVInputFormat ff_mv_demuxer;
AVInputFormat ff_mvi_demuxer;
AVInputFormat ff_mxf_demuxer;
AVInputFormat ff_mxg_demuxer;
AVInputFormat ff_nc_demuxer;
AVInputFormat ff_nistsphere_demuxer;
AVInputFormat ff_nsp_demuxer;
AVInputFormat ff_nsv_demuxer;
AVInputFormat ff_nut_demuxer;
AVInputFormat ff_nuv_demuxer;
AVInputFormat ff_obu_demuxer;
AVInputFormat ff_ogg_demuxer;
AVInputFormat ff_oma_demuxer;
AVInputFormat ff_paf_demuxer;
AVInputFormat ff_pcm_alaw_demuxer;
AVInputFormat ff_pcm_mulaw_demuxer;
AVInputFormat ff_pcm_vidc_demuxer;
AVInputFormat ff_pcm_f64be_demuxer;
AVInputFormat ff_pcm_f64le_demuxer;
AVInputFormat ff_pcm_f32be_demuxer;
AVInputFormat ff_pcm_f32le_demuxer;
AVInputFormat ff_pcm_s32be_demuxer;
AVInputFormat ff_pcm_s32le_demuxer;
AVInputFormat ff_pcm_s24be_demuxer;
AVInputFormat ff_pcm_s24le_demuxer;
AVInputFormat ff_pcm_s16be_demuxer;
AVInputFormat ff_pcm_s16le_demuxer;
AVInputFormat ff_pcm_s8_demuxer;
AVInputFormat ff_pcm_u32be_demuxer;
AVInputFormat ff_pcm_u32le_demuxer;
AVInputFormat ff_pcm_u24be_demuxer;
AVInputFormat ff_pcm_u24le_demuxer;
AVInputFormat ff_pcm_u16be_demuxer;
AVInputFormat ff_pcm_u16le_demuxer;
AVInputFormat ff_pcm_u8_demuxer;
AVInputFormat ff_pjs_demuxer;
AVInputFormat ff_pmp_demuxer;
AVInputFormat ff_pp_bnk_demuxer;
AVInputFormat ff_pva_demuxer;
AVInputFormat ff_pvf_demuxer;
AVInputFormat ff_qcp_demuxer;
AVInputFormat ff_r3d_demuxer;
AVInputFormat ff_rawvideo_demuxer;
AVInputFormat ff_realtext_demuxer;
AVInputFormat ff_redspark_demuxer;
AVInputFormat ff_rl2_demuxer;
AVInputFormat ff_rm_demuxer;
AVInputFormat ff_roq_demuxer;
AVInputFormat ff_rpl_demuxer;
AVInputFormat ff_rsd_demuxer;
AVInputFormat ff_rso_demuxer;
AVInputFormat ff_rtp_demuxer;
AVInputFormat ff_rtsp_demuxer;
AVInputFormat ff_s337m_demuxer;
AVInputFormat ff_sami_demuxer;
AVInputFormat ff_sap_demuxer;
AVInputFormat ff_sbc_demuxer;
AVInputFormat ff_sbg_demuxer;
AVInputFormat ff_scc_demuxer;
AVInputFormat ff_sdp_demuxer;
AVInputFormat ff_sdr2_demuxer;
AVInputFormat ff_sds_demuxer;
AVInputFormat ff_sdx_demuxer;
AVInputFormat ff_segafilm_demuxer;
AVInputFormat ff_ser_demuxer;
AVInputFormat ff_shorten_demuxer;
AVInputFormat ff_siff_demuxer;
AVInputFormat ff_sln_demuxer;
AVInputFormat ff_smacker_demuxer;
AVInputFormat ff_smjpeg_demuxer;
AVInputFormat ff_smush_demuxer;
AVInputFormat ff_sol_demuxer;
AVInputFormat ff_sox_demuxer;
AVInputFormat ff_spdif_demuxer;
AVInputFormat ff_srt_demuxer;
AVInputFormat ff_str_demuxer;
AVInputFormat ff_stl_demuxer;
AVInputFormat ff_subviewer1_demuxer;
AVInputFormat ff_subviewer_demuxer;
AVInputFormat ff_sup_demuxer;
AVInputFormat ff_svag_demuxer;
AVInputFormat ff_svs_demuxer;
AVInputFormat ff_swf_demuxer;
AVInputFormat ff_tak_demuxer;
AVInputFormat ff_tedcaptions_demuxer;
AVInputFormat ff_thp_demuxer;
AVInputFormat ff_threedostr_demuxer;
AVInputFormat ff_tiertexseq_demuxer;
AVInputFormat ff_tmv_demuxer;
AVInputFormat ff_truehd_demuxer;
AVInputFormat ff_tta_demuxer;
AVInputFormat ff_txd_demuxer;
AVInputFormat ff_tty_demuxer;
AVInputFormat ff_ty_demuxer;
AVInputFormat ff_v210_demuxer;
AVInputFormat ff_v210x_demuxer;
AVInputFormat ff_vag_demuxer;
AVInputFormat ff_vc1_demuxer;
AVInputFormat ff_vc1t_demuxer;
AVInputFormat ff_vividas_demuxer;
AVInputFormat ff_vivo_demuxer;
AVInputFormat ff_vmd_demuxer;
AVInputFormat ff_vobsub_demuxer;
AVInputFormat ff_voc_demuxer;
AVInputFormat ff_vpk_demuxer;
AVInputFormat ff_vplayer_demuxer;
AVInputFormat ff_vqf_demuxer;
AVInputFormat ff_w64_demuxer;
AVInputFormat ff_wav_demuxer;
AVInputFormat ff_wc3_demuxer;
AVInputFormat ff_webm_dash_manifest_demuxer;
AVInputFormat ff_webvtt_demuxer;
AVInputFormat ff_wsaud_demuxer;
AVInputFormat ff_wsd_demuxer;
AVInputFormat ff_wsvqa_demuxer;
AVInputFormat ff_wtv_demuxer;
AVInputFormat ff_wve_demuxer;
AVInputFormat ff_wv_demuxer;
AVInputFormat ff_xa_demuxer;
AVInputFormat ff_xbin_demuxer;
AVInputFormat ff_xmv_demuxer;
AVInputFormat ff_xvag_demuxer;
AVInputFormat ff_xwma_demuxer;
AVInputFormat ff_yop_demuxer;
AVInputFormat ff_yuv4mpegpipe_demuxer;
AVInputFormat ff_image_bmp_pipe_demuxer;
AVInputFormat ff_image_dds_pipe_demuxer;
AVInputFormat ff_image_dpx_pipe_demuxer;
AVInputFormat ff_image_exr_pipe_demuxer;
AVInputFormat ff_image_gif_pipe_demuxer;
AVInputFormat ff_image_j2k_pipe_demuxer;
AVInputFormat ff_image_jpeg_pipe_demuxer;
AVInputFormat ff_image_jpegls_pipe_demuxer;
AVInputFormat ff_image_pam_pipe_demuxer;
AVInputFormat ff_image_pbm_pipe_demuxer;
AVInputFormat ff_image_pcx_pipe_demuxer;
AVInputFormat ff_image_pgmyuv_pipe_demuxer;
AVInputFormat ff_image_pgm_pipe_demuxer;
AVInputFormat ff_image_pgx_pipe_demuxer;
AVInputFormat ff_image_photocd_pipe_demuxer;
AVInputFormat ff_image_pictor_pipe_demuxer;
AVInputFormat ff_image_png_pipe_demuxer;
AVInputFormat ff_image_ppm_pipe_demuxer;
AVInputFormat ff_image_psd_pipe_demuxer;
AVInputFormat ff_image_qdraw_pipe_demuxer;
AVInputFormat ff_image_sgi_pipe_demuxer;
AVInputFormat ff_image_svg_pipe_demuxer;
AVInputFormat ff_image_sunrast_pipe_demuxer;
AVInputFormat ff_image_tiff_pipe_demuxer;
AVInputFormat ff_image_webp_pipe_demuxer;
AVInputFormat ff_image_xpm_pipe_demuxer;
AVInputFormat ff_image_xwd_pipe_demuxer;
AVInputFormat ff_libgme_demuxer;
AVInputFormat ff_libmodplug_demuxer;
AVOutputFormat ff_a64_muxer;
AVOutputFormat ff_ac3_muxer;
AVOutputFormat ff_adts_muxer;
AVOutputFormat ff_adx_muxer;
AVOutputFormat ff_aiff_muxer;
AVOutputFormat ff_amr_muxer;
AVOutputFormat ff_apm_muxer;
AVOutputFormat ff_apng_muxer;
AVOutputFormat ff_aptx_muxer;
AVOutputFormat ff_aptx_hd_muxer;
AVOutputFormat ff_argo_asf_muxer;
AVOutputFormat ff_asf_muxer;
AVOutputFormat ff_ass_muxer;
AVOutputFormat ff_ast_muxer;
AVOutputFormat ff_asf_stream_muxer;
AVOutputFormat ff_au_muxer;
AVOutputFormat ff_avi_muxer;
AVOutputFormat ff_avm2_muxer;
AVOutputFormat ff_avs2_muxer;
AVOutputFormat ff_bit_muxer;
AVOutputFormat ff_caf_muxer;
AVOutputFormat ff_cavsvideo_muxer;
AVOutputFormat ff_codec2_muxer;
AVOutputFormat ff_codec2raw_muxer;
AVOutputFormat ff_crc_muxer;
AVOutputFormat ff_dash_muxer;
AVOutputFormat ff_data_muxer;
AVOutputFormat ff_daud_muxer;
AVOutputFormat ff_dirac_muxer;
AVOutputFormat ff_dnxhd_muxer;
AVOutputFormat ff_dts_muxer;
AVOutputFormat ff_dv_muxer;
AVOutputFormat ff_eac3_muxer;
AVOutputFormat ff_f4v_muxer;
AVOutputFormat ff_ffmetadata_muxer;
AVOutputFormat ff_fifo_muxer;
AVOutputFormat ff_fifo_test_muxer;
AVOutputFormat ff_filmstrip_muxer;
AVOutputFormat ff_fits_muxer;
AVOutputFormat ff_flac_muxer;
AVOutputFormat ff_flv_muxer;
AVOutputFormat ff_framecrc_muxer;
AVOutputFormat ff_framehash_muxer;
AVOutputFormat ff_framemd5_muxer;
AVOutputFormat ff_g722_muxer;
AVOutputFormat ff_g723_1_muxer;
AVOutputFormat ff_g726_muxer;
AVOutputFormat ff_g726le_muxer;
AVOutputFormat ff_gif_muxer;
AVOutputFormat ff_gsm_muxer;
AVOutputFormat ff_gxf_muxer;
AVOutputFormat ff_h261_muxer;
AVOutputFormat ff_h263_muxer;
AVOutputFormat ff_h264_muxer;
AVOutputFormat ff_hash_muxer;
AVOutputFormat ff_hds_muxer;
AVOutputFormat ff_hevc_muxer;
AVOutputFormat ff_hls_muxer;
AVOutputFormat ff_ico_muxer;
AVOutputFormat ff_ilbc_muxer;
AVOutputFormat ff_image2_muxer;
AVOutputFormat ff_image2pipe_muxer;
AVOutputFormat ff_ipod_muxer;
AVOutputFormat ff_ircam_muxer;
AVOutputFormat ff_ismv_muxer;
AVOutputFormat ff_ivf_muxer;
AVOutputFormat ff_jacosub_muxer;
AVOutputFormat ff_kvag_muxer;
AVOutputFormat ff_latm_muxer;
AVOutputFormat ff_lrc_muxer;
AVOutputFormat ff_m4v_muxer;
AVOutputFormat ff_md5_muxer;
AVOutputFormat ff_matroska_muxer;
AVOutputFormat ff_matroska_audio_muxer;
AVOutputFormat ff_microdvd_muxer;
AVOutputFormat ff_mjpeg_muxer;
AVOutputFormat ff_mlp_muxer;
AVOutputFormat ff_mmf_muxer;
AVOutputFormat ff_mov_muxer;
AVOutputFormat ff_mp2_muxer;
AVOutputFormat ff_mp3_muxer;
AVOutputFormat ff_mp4_muxer;
AVOutputFormat ff_mpeg1system_muxer;
AVOutputFormat ff_mpeg1vcd_muxer;
AVOutputFormat ff_mpeg1video_muxer;
AVOutputFormat ff_mpeg2dvd_muxer;
AVOutputFormat ff_mpeg2svcd_muxer;
AVOutputFormat ff_mpeg2video_muxer;
AVOutputFormat ff_mpeg2vob_muxer;
AVOutputFormat ff_mpegts_muxer;
AVOutputFormat ff_mpjpeg_muxer;
AVOutputFormat ff_mxf_muxer;
AVOutputFormat ff_mxf_d10_muxer;
AVOutputFormat ff_mxf_opatom_muxer;
AVOutputFormat ff_null_muxer;
AVOutputFormat ff_nut_muxer;
AVOutputFormat ff_oga_muxer;
AVOutputFormat ff_ogg_muxer;
AVOutputFormat ff_ogv_muxer;
AVOutputFormat ff_oma_muxer;
AVOutputFormat ff_opus_muxer;
AVOutputFormat ff_pcm_alaw_muxer;
AVOutputFormat ff_pcm_mulaw_muxer;
AVOutputFormat ff_pcm_vidc_muxer;
AVOutputFormat ff_pcm_f64be_muxer;
AVOutputFormat ff_pcm_f64le_muxer;
AVOutputFormat ff_pcm_f32be_muxer;
AVOutputFormat ff_pcm_f32le_muxer;
AVOutputFormat ff_pcm_s32be_muxer;
AVOutputFormat ff_pcm_s32le_muxer;
AVOutputFormat ff_pcm_s24be_muxer;
AVOutputFormat ff_pcm_s24le_muxer;
AVOutputFormat ff_pcm_s16be_muxer;
AVOutputFormat ff_pcm_s16le_muxer;
AVOutputFormat ff_pcm_s8_muxer;
AVOutputFormat ff_pcm_u32be_muxer;
AVOutputFormat ff_pcm_u32le_muxer;
AVOutputFormat ff_pcm_u24be_muxer;
AVOutputFormat ff_pcm_u24le_muxer;
AVOutputFormat ff_pcm_u16be_muxer;
AVOutputFormat ff_pcm_u16le_muxer;
AVOutputFormat ff_pcm_u8_muxer;
AVOutputFormat ff_psp_muxer;
AVOutputFormat ff_rawvideo_muxer;
AVOutputFormat ff_rm_muxer;
AVOutputFormat ff_roq_muxer;
AVOutputFormat ff_rso_muxer;
AVOutputFormat ff_rtp_muxer;
AVOutputFormat ff_rtp_mpegts_muxer;
AVOutputFormat ff_rtsp_muxer;
AVOutputFormat ff_sap_muxer;
AVOutputFormat ff_sbc_muxer;
AVOutputFormat ff_scc_muxer;
AVOutputFormat ff_segafilm_muxer;
AVOutputFormat ff_segment_muxer;
AVOutputFormat ff_stream_segment_muxer;
AVOutputFormat ff_singlejpeg_muxer;
AVOutputFormat ff_smjpeg_muxer;
AVOutputFormat ff_smoothstreaming_muxer;
AVOutputFormat ff_sox_muxer;
AVOutputFormat ff_spx_muxer;
AVOutputFormat ff_spdif_muxer;
AVOutputFormat ff_srt_muxer;
AVOutputFormat ff_streamhash_muxer;
AVOutputFormat ff_sup_muxer;
AVOutputFormat ff_swf_muxer;
AVOutputFormat ff_tee_muxer;
AVOutputFormat ff_tg2_muxer;
AVOutputFormat ff_tgp_muxer;
AVOutputFormat ff_mkvtimestamp_v2_muxer;
AVOutputFormat ff_truehd_muxer;
AVOutputFormat ff_tta_muxer;
AVOutputFormat ff_uncodedframecrc_muxer;
AVOutputFormat ff_vc1_muxer;
AVOutputFormat ff_vc1t_muxer;
AVOutputFormat ff_voc_muxer;
AVOutputFormat ff_w64_muxer;
AVOutputFormat ff_wav_muxer;
AVOutputFormat ff_webm_muxer;
AVOutputFormat ff_webm_dash_manifest_muxer;
AVOutputFormat ff_webm_chunk_muxer;
AVOutputFormat ff_webp_muxer;
AVOutputFormat ff_webvtt_muxer;
AVOutputFormat ff_wtv_muxer;
AVOutputFormat ff_wv_muxer;
AVOutputFormat ff_yuv4mpegpipe_muxer;
AVOutputFormat ff_chromaprint_muxer;
AVFilter ff_af_acompressor;
AVFilter ff_af_acontrast;
AVFilter ff_af_acopy;
AVFilter ff_af_acue;
AVFilter ff_af_acrossfade;
AVFilter ff_af_acrossover;
AVFilter ff_af_acrusher;
AVFilter ff_af_adeclick;
AVFilter ff_af_adeclip;
AVFilter ff_af_adelay;
AVFilter ff_af_aderivative;
AVFilter ff_af_aecho;
AVFilter ff_af_aemphasis;
AVFilter ff_af_aeval;
AVFilter ff_af_afade;
AVFilter ff_af_afftdn;
AVFilter ff_af_afftfilt;
AVFilter ff_af_afir;
AVFilter ff_af_aformat;
AVFilter ff_af_agate;
AVFilter ff_af_aiir;
AVFilter ff_af_aintegral;
AVFilter ff_af_ainterleave;
AVFilter ff_af_alimiter;
AVFilter ff_af_allpass;
AVFilter ff_af_aloop;
AVFilter ff_af_amerge;
AVFilter ff_af_ametadata;
AVFilter ff_af_amix;
AVFilter ff_af_amultiply;
AVFilter ff_af_anequalizer;
AVFilter ff_af_anlmdn;
AVFilter ff_af_anlms;
AVFilter ff_af_anull;
AVFilter ff_af_apad;
AVFilter ff_af_aperms;
AVFilter ff_af_aphaser;
AVFilter ff_af_apulsator;
AVFilter ff_af_arealtime;
AVFilter ff_af_aresample;
AVFilter ff_af_areverse;
AVFilter ff_af_arnndn;
AVFilter ff_af_aselect;
AVFilter ff_af_asendcmd;
AVFilter ff_af_asetnsamples;
AVFilter ff_af_asetpts;
AVFilter ff_af_asetrate;
AVFilter ff_af_asettb;
AVFilter ff_af_ashowinfo;
AVFilter ff_af_asidedata;
AVFilter ff_af_asoftclip;
AVFilter ff_af_asplit;
AVFilter ff_af_astats;
AVFilter ff_af_astreamselect;
AVFilter ff_af_asubboost;
AVFilter ff_af_atempo;
AVFilter ff_af_atrim;
AVFilter ff_af_axcorrelate;
AVFilter ff_af_azmq;
AVFilter ff_af_bandpass;
AVFilter ff_af_bandreject;
AVFilter ff_af_bass;
AVFilter ff_af_biquad;
AVFilter ff_af_bs2b;
AVFilter ff_af_channelmap;
AVFilter ff_af_channelsplit;
AVFilter ff_af_chorus;
AVFilter ff_af_compand;
AVFilter ff_af_compensationdelay;
AVFilter ff_af_crossfeed;
AVFilter ff_af_crystalizer;
AVFilter ff_af_dcshift;
AVFilter ff_af_deesser;
AVFilter ff_af_drmeter;
AVFilter ff_af_dynaudnorm;
AVFilter ff_af_earwax;
AVFilter ff_af_ebur128;
AVFilter ff_af_equalizer;
AVFilter ff_af_extrastereo;
AVFilter ff_af_firequalizer;
AVFilter ff_af_flanger;
AVFilter ff_af_haas;
AVFilter ff_af_hdcd;
AVFilter ff_af_headphone;
AVFilter ff_af_highpass;
AVFilter ff_af_highshelf;
AVFilter ff_af_join;
AVFilter ff_af_loudnorm;
AVFilter ff_af_lowpass;
AVFilter ff_af_lowshelf;
AVFilter ff_af_mcompand;
AVFilter ff_af_pan;
AVFilter ff_af_replaygain;
AVFilter ff_af_rubberband;
AVFilter ff_af_sidechaincompress;
AVFilter ff_af_sidechaingate;
AVFilter ff_af_silencedetect;
AVFilter ff_af_silenceremove;
AVFilter ff_af_sofalizer;
AVFilter ff_af_stereotools;
AVFilter ff_af_stereowiden;
AVFilter ff_af_superequalizer;
AVFilter ff_af_surround;
AVFilter ff_af_treble;
AVFilter ff_af_tremolo;
AVFilter ff_af_vibrato;
AVFilter ff_af_volume;
AVFilter ff_af_volumedetect;
AVFilter ff_asrc_aevalsrc;
AVFilter ff_asrc_afirsrc;
AVFilter ff_asrc_anoisesrc;
AVFilter ff_asrc_anullsrc;
AVFilter ff_asrc_hilbert;
AVFilter ff_asrc_sinc;
AVFilter ff_asrc_sine;
AVFilter ff_asink_anullsink;
AVFilter ff_vf_addroi;
AVFilter ff_vf_alphaextract;
AVFilter ff_vf_alphamerge;
AVFilter ff_vf_amplify;
AVFilter ff_vf_ass;
AVFilter ff_vf_atadenoise;
AVFilter ff_vf_avgblur;
AVFilter ff_vf_bbox;
AVFilter ff_vf_bench;
AVFilter ff_vf_bilateral;
AVFilter ff_vf_bitplanenoise;
AVFilter ff_vf_blackdetect;
AVFilter ff_vf_blackframe;
AVFilter ff_vf_blend;
AVFilter ff_vf_bm3d;
AVFilter ff_vf_boxblur;
AVFilter ff_vf_bwdif;
AVFilter ff_vf_cas;
AVFilter ff_vf_chromahold;
AVFilter ff_vf_chromakey;
AVFilter ff_vf_chromanr;
AVFilter ff_vf_chromashift;
AVFilter ff_vf_ciescope;
AVFilter ff_vf_codecview;
AVFilter ff_vf_colorbalance;
AVFilter ff_vf_colorchannelmixer;
AVFilter ff_vf_colorkey;
AVFilter ff_vf_colorhold;
AVFilter ff_vf_colorlevels;
AVFilter ff_vf_colormatrix;
AVFilter ff_vf_colorspace;
AVFilter ff_vf_convolution;
AVFilter ff_vf_convolve;
AVFilter ff_vf_copy;
AVFilter ff_vf_cover_rect;
AVFilter ff_vf_crop;
AVFilter ff_vf_cropdetect;
AVFilter ff_vf_cue;
AVFilter ff_vf_curves;
AVFilter ff_vf_datascope;
AVFilter ff_vf_dblur;
AVFilter ff_vf_dctdnoiz;
AVFilter ff_vf_deband;
AVFilter ff_vf_deblock;
AVFilter ff_vf_decimate;
AVFilter ff_vf_deconvolve;
AVFilter ff_vf_dedot;
AVFilter ff_vf_deflate;
AVFilter ff_vf_deflicker;
AVFilter ff_vf_deinterlace_qsv;
AVFilter ff_vf_dejudder;
AVFilter ff_vf_delogo;
AVFilter ff_vf_derain;
AVFilter ff_vf_deshake;
AVFilter ff_vf_despill;
AVFilter ff_vf_detelecine;
AVFilter ff_vf_dilation;
AVFilter ff_vf_displace;
AVFilter ff_vf_dnn_processing;
AVFilter ff_vf_doubleweave;
AVFilter ff_vf_drawbox;
AVFilter ff_vf_drawgraph;
AVFilter ff_vf_drawgrid;
AVFilter ff_vf_drawtext;
AVFilter ff_vf_edgedetect;
AVFilter ff_vf_elbg;
AVFilter ff_vf_entropy;
AVFilter ff_vf_eq;
AVFilter ff_vf_erosion;
AVFilter ff_vf_extractplanes;
AVFilter ff_vf_fade;
AVFilter ff_vf_fftdnoiz;
AVFilter ff_vf_fftfilt;
AVFilter ff_vf_field;
AVFilter ff_vf_fieldhint;
AVFilter ff_vf_fieldmatch;
AVFilter ff_vf_fieldorder;
AVFilter ff_vf_fillborders;
AVFilter ff_vf_find_rect;
AVFilter ff_vf_floodfill;
AVFilter ff_vf_format;
AVFilter ff_vf_fps;
AVFilter ff_vf_framepack;
AVFilter ff_vf_framerate;
AVFilter ff_vf_framestep;
AVFilter ff_vf_freezedetect;
AVFilter ff_vf_freezeframes;
AVFilter ff_vf_fspp;
AVFilter ff_vf_gblur;
AVFilter ff_vf_geq;
AVFilter ff_vf_gradfun;
AVFilter ff_vf_graphmonitor;
AVFilter ff_vf_greyedge;
AVFilter ff_vf_haldclut;
AVFilter ff_vf_hflip;
AVFilter ff_vf_histeq;
AVFilter ff_vf_histogram;
AVFilter ff_vf_hqdn3d;
AVFilter ff_vf_hqx;
AVFilter ff_vf_hstack;
AVFilter ff_vf_hue;
AVFilter ff_vf_hwdownload;
AVFilter ff_vf_hwmap;
AVFilter ff_vf_hwupload;
AVFilter ff_vf_hysteresis;
AVFilter ff_vf_idet;
AVFilter ff_vf_il;
AVFilter ff_vf_inflate;
AVFilter ff_vf_interlace;
AVFilter ff_vf_interleave;
AVFilter ff_vf_kerndeint;
AVFilter ff_vf_lagfun;
AVFilter ff_vf_lenscorrection;
AVFilter ff_vf_limiter;
AVFilter ff_vf_loop;
AVFilter ff_vf_lumakey;
AVFilter ff_vf_lut;
AVFilter ff_vf_lut1d;
AVFilter ff_vf_lut2;
AVFilter ff_vf_lut3d;
AVFilter ff_vf_lutrgb;
AVFilter ff_vf_lutyuv;
AVFilter ff_vf_maskedclamp;
AVFilter ff_vf_maskedmax;
AVFilter ff_vf_maskedmerge;
AVFilter ff_vf_maskedmin;
AVFilter ff_vf_maskedthreshold;
AVFilter ff_vf_maskfun;
AVFilter ff_vf_mcdeint;
AVFilter ff_vf_median;
AVFilter ff_vf_mergeplanes;
AVFilter ff_vf_mestimate;
AVFilter ff_vf_metadata;
AVFilter ff_vf_midequalizer;
AVFilter ff_vf_minterpolate;
AVFilter ff_vf_mix;
AVFilter ff_vf_mpdecimate;
AVFilter ff_vf_negate;
AVFilter ff_vf_nlmeans;
AVFilter ff_vf_nnedi;
AVFilter ff_vf_noformat;
AVFilter ff_vf_noise;
AVFilter ff_vf_normalize;
AVFilter ff_vf_null;
AVFilter ff_vf_oscilloscope;
AVFilter ff_vf_overlay;
AVFilter ff_vf_overlay_qsv;
AVFilter ff_vf_owdenoise;
AVFilter ff_vf_pad;
AVFilter ff_vf_palettegen;
AVFilter ff_vf_paletteuse;
AVFilter ff_vf_perms;
AVFilter ff_vf_perspective;
AVFilter ff_vf_phase;
AVFilter ff_vf_photosensitivity;
AVFilter ff_vf_pixdesctest;
AVFilter ff_vf_pixscope;
AVFilter ff_vf_pp;
AVFilter ff_vf_pp7;
AVFilter ff_vf_premultiply;
AVFilter ff_vf_prewitt;
AVFilter ff_vf_pseudocolor;
AVFilter ff_vf_psnr;
AVFilter ff_vf_pullup;
AVFilter ff_vf_qp;
AVFilter ff_vf_random;
AVFilter ff_vf_readeia608;
AVFilter ff_vf_readvitc;
AVFilter ff_vf_realtime;
AVFilter ff_vf_remap;
AVFilter ff_vf_removegrain;
AVFilter ff_vf_removelogo;
AVFilter ff_vf_repeatfields;
AVFilter ff_vf_reverse;
AVFilter ff_vf_rgbashift;
AVFilter ff_vf_roberts;
AVFilter ff_vf_rotate;
AVFilter ff_vf_sab;
AVFilter ff_vf_scale;
AVFilter ff_vf_scale_qsv;
AVFilter ff_vf_scale2ref;
AVFilter ff_vf_scdet;
AVFilter ff_vf_scroll;
AVFilter ff_vf_select;
AVFilter ff_vf_selectivecolor;
AVFilter ff_vf_sendcmd;
AVFilter ff_vf_separatefields;
AVFilter ff_vf_setdar;
AVFilter ff_vf_setfield;
AVFilter ff_vf_setparams;
AVFilter ff_vf_setpts;
AVFilter ff_vf_setrange;
AVFilter ff_vf_setsar;
AVFilter ff_vf_settb;
AVFilter ff_vf_showinfo;
AVFilter ff_vf_showpalette;
AVFilter ff_vf_shuffleframes;
AVFilter ff_vf_shuffleplanes;
AVFilter ff_vf_sidedata;
AVFilter ff_vf_signalstats;
AVFilter ff_vf_signature;
AVFilter ff_vf_smartblur;
AVFilter ff_vf_sobel;
AVFilter ff_vf_split;
AVFilter ff_vf_spp;
AVFilter ff_vf_sr;
AVFilter ff_vf_ssim;
AVFilter ff_vf_stereo3d;
AVFilter ff_vf_streamselect;
AVFilter ff_vf_subtitles;
AVFilter ff_vf_super2xsai;
AVFilter ff_vf_swaprect;
AVFilter ff_vf_swapuv;
AVFilter ff_vf_tblend;
AVFilter ff_vf_telecine;
AVFilter ff_vf_thistogram;
AVFilter ff_vf_threshold;
AVFilter ff_vf_thumbnail;
AVFilter ff_vf_tile;
AVFilter ff_vf_tinterlace;
AVFilter ff_vf_tlut2;
AVFilter ff_vf_tmedian;
AVFilter ff_vf_tmix;
AVFilter ff_vf_tonemap;
AVFilter ff_vf_tpad;
AVFilter ff_vf_transpose;
AVFilter ff_vf_trim;
AVFilter ff_vf_unpremultiply;
AVFilter ff_vf_unsharp;
AVFilter ff_vf_untile;
AVFilter ff_vf_uspp;
AVFilter ff_vf_v360;
AVFilter ff_vf_vaguedenoiser;
AVFilter ff_vf_vectorscope;
AVFilter ff_vf_vflip;
AVFilter ff_vf_vfrdet;
AVFilter ff_vf_vibrance;
AVFilter ff_vf_vidstabdetect;
AVFilter ff_vf_vidstabtransform;
AVFilter ff_vf_vignette;
AVFilter ff_vf_vmafmotion;
AVFilter ff_vf_vpp_qsv;
AVFilter ff_vf_vstack;
AVFilter ff_vf_w3fdif;
AVFilter ff_vf_waveform;
AVFilter ff_vf_weave;
AVFilter ff_vf_xbr;
AVFilter ff_vf_xfade;
AVFilter ff_vf_xmedian;
AVFilter ff_vf_xstack;
AVFilter ff_vf_yadif;
AVFilter ff_vf_yaepblur;
AVFilter ff_vf_zmq;
AVFilter ff_vf_zoompan;
AVFilter ff_vf_zscale;
AVFilter ff_vsrc_allrgb;
AVFilter ff_vsrc_allyuv;
AVFilter ff_vsrc_cellauto;
AVFilter ff_vsrc_color;
AVFilter ff_vsrc_gradients;
AVFilter ff_vsrc_haldclutsrc;
AVFilter ff_vsrc_life;
AVFilter ff_vsrc_mandelbrot;
AVFilter ff_vsrc_mptestsrc;
AVFilter ff_vsrc_nullsrc;
AVFilter ff_vsrc_pal75bars;
AVFilter ff_vsrc_pal100bars;
AVFilter ff_vsrc_rgbtestsrc;
AVFilter ff_vsrc_sierpinski;
AVFilter ff_vsrc_smptebars;
AVFilter ff_vsrc_smptehdbars;
AVFilter ff_vsrc_testsrc;
AVFilter ff_vsrc_testsrc2;
AVFilter ff_vsrc_yuvtestsrc;
AVFilter ff_vsink_nullsink;
AVFilter ff_avf_abitscope;
AVFilter ff_avf_adrawgraph;
AVFilter ff_avf_agraphmonitor;
AVFilter ff_avf_ahistogram;
AVFilter ff_avf_aphasemeter;
AVFilter ff_avf_avectorscope;
AVFilter ff_avf_concat;
AVFilter ff_avf_showcqt;
AVFilter ff_avf_showfreqs;
AVFilter ff_avf_showspatial;
AVFilter ff_avf_showspectrum;
AVFilter ff_avf_showspectrumpic;
AVFilter ff_avf_showvolume;
AVFilter ff_avf_showwaves;
AVFilter ff_avf_showwavespic;
AVFilter ff_vaf_spectrumsynth;
AVFilter ff_avsrc_amovie;
AVFilter ff_avsrc_movie;
AVFilter ff_af_afifo;
AVFilter ff_vf_fifo;
AVFilter ff_asrc_abuffer;
AVFilter ff_vsrc_buffer;
AVFilter ff_asink_abuffer;
AVFilter ff_vsink_buffer;

static const AVOutputFormat * const muxer_list[] = {
    &ff_a64_muxer,
    &ff_ac3_muxer,
    &ff_adts_muxer,
    &ff_adx_muxer,
    &ff_aiff_muxer,
    &ff_amr_muxer,
    &ff_apm_muxer,
    &ff_apng_muxer,
    &ff_aptx_muxer,
    &ff_aptx_hd_muxer,
    &ff_argo_asf_muxer,
    &ff_asf_muxer,
    &ff_ass_muxer,
    &ff_ast_muxer,
    &ff_asf_stream_muxer,
    &ff_au_muxer,
    &ff_avi_muxer,
    &ff_avm2_muxer,
    &ff_avs2_muxer,
    &ff_bit_muxer,
    &ff_caf_muxer,
    &ff_cavsvideo_muxer,
    &ff_codec2_muxer,
    &ff_codec2raw_muxer,
    &ff_crc_muxer,
    &ff_dash_muxer,
    &ff_data_muxer,
    &ff_daud_muxer,
    &ff_dirac_muxer,
    &ff_dnxhd_muxer,
    &ff_dts_muxer,
    &ff_dv_muxer,
    &ff_eac3_muxer,
    &ff_f4v_muxer,
    &ff_ffmetadata_muxer,
    &ff_fifo_muxer,
    &ff_fifo_test_muxer,
    &ff_filmstrip_muxer,
    &ff_fits_muxer,
    &ff_flac_muxer,
    &ff_flv_muxer,
    &ff_framecrc_muxer,
    &ff_framehash_muxer,
    &ff_framemd5_muxer,
    &ff_g722_muxer,
    &ff_g723_1_muxer,
    &ff_g726_muxer,
    &ff_g726le_muxer,
    &ff_gif_muxer,
    &ff_gsm_muxer,
    &ff_gxf_muxer,
    &ff_h261_muxer,
    &ff_h263_muxer,
    &ff_h264_muxer,
    &ff_hash_muxer,
    &ff_hds_muxer,
    &ff_hevc_muxer,
    &ff_hls_muxer,
    &ff_ico_muxer,
    &ff_ilbc_muxer,
    &ff_image2_muxer,
    &ff_image2pipe_muxer,
    &ff_ipod_muxer,
    &ff_ircam_muxer,
    &ff_ismv_muxer,
    &ff_ivf_muxer,
    &ff_jacosub_muxer,
    &ff_kvag_muxer,
    &ff_latm_muxer,
    &ff_lrc_muxer,
    &ff_m4v_muxer,
    &ff_md5_muxer,
    &ff_matroska_muxer,
    &ff_matroska_audio_muxer,
    &ff_microdvd_muxer,
    &ff_mjpeg_muxer,
    &ff_mlp_muxer,
    &ff_mmf_muxer,
    &ff_mov_muxer,
    &ff_mp2_muxer,
    &ff_mp3_muxer,
    &ff_mp4_muxer,
    &ff_mpeg1system_muxer,
    &ff_mpeg1vcd_muxer,
    &ff_mpeg1video_muxer,
    &ff_mpeg2dvd_muxer,
    &ff_mpeg2svcd_muxer,
    &ff_mpeg2video_muxer,
    &ff_mpeg2vob_muxer,
    &ff_mpegts_muxer,
    &ff_mpjpeg_muxer,
    &ff_mxf_muxer,
    &ff_mxf_d10_muxer,
    &ff_mxf_opatom_muxer,
    &ff_null_muxer,
    &ff_nut_muxer,
    &ff_oga_muxer,
    &ff_ogg_muxer,
    &ff_ogv_muxer,
    &ff_oma_muxer,
    &ff_opus_muxer,
    &ff_pcm_alaw_muxer,
    &ff_pcm_mulaw_muxer,
    &ff_pcm_vidc_muxer,
    &ff_pcm_f64be_muxer,
    &ff_pcm_f64le_muxer,
    &ff_pcm_f32be_muxer,
    &ff_pcm_f32le_muxer,
    &ff_pcm_s32be_muxer,
    &ff_pcm_s32le_muxer,
    &ff_pcm_s24be_muxer,
    &ff_pcm_s24le_muxer,
    &ff_pcm_s16be_muxer,
    &ff_pcm_s16le_muxer,
    &ff_pcm_s8_muxer,
    &ff_pcm_u32be_muxer,
    &ff_pcm_u32le_muxer,
    &ff_pcm_u24be_muxer,
    &ff_pcm_u24le_muxer,
    &ff_pcm_u16be_muxer,
    &ff_pcm_u16le_muxer,
    &ff_pcm_u8_muxer,
    &ff_psp_muxer,
    &ff_rawvideo_muxer,
    &ff_rm_muxer,
    &ff_roq_muxer,
    &ff_rso_muxer,
    &ff_rtp_muxer,
    &ff_rtp_mpegts_muxer,
    &ff_rtsp_muxer,
    &ff_sap_muxer,
    &ff_sbc_muxer,
    &ff_scc_muxer,
    &ff_segafilm_muxer,
    &ff_segment_muxer,
    &ff_stream_segment_muxer,
    &ff_singlejpeg_muxer,
    &ff_smjpeg_muxer,
    &ff_smoothstreaming_muxer,
    &ff_sox_muxer,
    &ff_spx_muxer,
    &ff_spdif_muxer,
    &ff_srt_muxer,
    &ff_streamhash_muxer,
    &ff_sup_muxer,
    &ff_swf_muxer,
    &ff_tee_muxer,
    &ff_tg2_muxer,
    &ff_tgp_muxer,
    &ff_mkvtimestamp_v2_muxer,
    &ff_truehd_muxer,
    &ff_tta_muxer,
    &ff_uncodedframecrc_muxer,
    &ff_vc1_muxer,
    &ff_vc1t_muxer,
    &ff_voc_muxer,
    &ff_w64_muxer,
    &ff_wav_muxer,
    &ff_webm_muxer,
    &ff_webm_dash_manifest_muxer,
    &ff_webm_chunk_muxer,
    &ff_webp_muxer,
    &ff_webvtt_muxer,
    &ff_wtv_muxer,
    &ff_wv_muxer,
    &ff_yuv4mpegpipe_muxer,
    &ff_chromaprint_muxer,
    NULL };

    static const AVInputFormat * const demuxer_list[] = {
    &ff_aa_demuxer,
    &ff_aac_demuxer,
    &ff_aax_demuxer,
    &ff_ac3_demuxer,
    &ff_acm_demuxer,
    &ff_act_demuxer,
    &ff_adf_demuxer,
    &ff_adp_demuxer,
    &ff_ads_demuxer,
    &ff_adx_demuxer,
    &ff_aea_demuxer,
    &ff_afc_demuxer,
    &ff_aiff_demuxer,
    &ff_aix_demuxer,
    &ff_alp_demuxer,
    &ff_amr_demuxer,
    &ff_amrnb_demuxer,
    &ff_amrwb_demuxer,
    &ff_anm_demuxer,
    &ff_apc_demuxer,
    &ff_ape_demuxer,
    &ff_apm_demuxer,
    &ff_apng_demuxer,
    &ff_aptx_demuxer,
    &ff_aptx_hd_demuxer,
    &ff_aqtitle_demuxer,
    &ff_argo_asf_demuxer,
    &ff_argo_brp_demuxer,
    &ff_asf_demuxer,
    &ff_asf_o_demuxer,
    &ff_ass_demuxer,
    &ff_ast_demuxer,
    &ff_au_demuxer,
    &ff_av1_demuxer,
    &ff_avi_demuxer,
    &ff_avr_demuxer,
    &ff_avs_demuxer,
    &ff_avs2_demuxer,
    &ff_bethsoftvid_demuxer,
    &ff_bfi_demuxer,
    &ff_bintext_demuxer,
    &ff_bink_demuxer,
    &ff_bit_demuxer,
    &ff_bmv_demuxer,
    &ff_bfstm_demuxer,
    &ff_brstm_demuxer,
    &ff_boa_demuxer,
    &ff_c93_demuxer,
    &ff_caf_demuxer,
    &ff_cavsvideo_demuxer,
    &ff_cdg_demuxer,
    &ff_cdxl_demuxer,
    &ff_cine_demuxer,
    &ff_codec2_demuxer,
    &ff_codec2raw_demuxer,
    &ff_concat_demuxer,
    &ff_dash_demuxer,
    &ff_data_demuxer,
    &ff_daud_demuxer,
    &ff_dcstr_demuxer,
    &ff_derf_demuxer,
    &ff_dfa_demuxer,
    &ff_dhav_demuxer,
    &ff_dirac_demuxer,
    &ff_dnxhd_demuxer,
    &ff_dsf_demuxer,
    &ff_dsicin_demuxer,
    &ff_dss_demuxer,
    &ff_dts_demuxer,
    &ff_dtshd_demuxer,
    &ff_dv_demuxer,
    &ff_dvbsub_demuxer,
    &ff_dvbtxt_demuxer,
    &ff_dxa_demuxer,
    &ff_ea_demuxer,
    &ff_ea_cdata_demuxer,
    &ff_eac3_demuxer,
    &ff_epaf_demuxer,
    &ff_ffmetadata_demuxer,
    &ff_filmstrip_demuxer,
    &ff_fits_demuxer,
    &ff_flac_demuxer,
    &ff_flic_demuxer,
    &ff_flv_demuxer,
    &ff_live_flv_demuxer,
    &ff_fourxm_demuxer,
    &ff_frm_demuxer,
    &ff_fsb_demuxer,
    &ff_fwse_demuxer,
    &ff_g722_demuxer,
    &ff_g723_1_demuxer,
    &ff_g726_demuxer,
    &ff_g726le_demuxer,
    &ff_g729_demuxer,
    &ff_gdv_demuxer,
    &ff_genh_demuxer,
    &ff_gif_demuxer,
    &ff_gsm_demuxer,
    &ff_gxf_demuxer,
    &ff_h261_demuxer,
    &ff_h263_demuxer,
    &ff_h264_demuxer,
    &ff_hca_demuxer,
    &ff_hcom_demuxer,
    &ff_hevc_demuxer,
    &ff_hls_demuxer,
    &ff_hnm_demuxer,
    &ff_ico_demuxer,
    &ff_idcin_demuxer,
    &ff_idf_demuxer,
    &ff_iff_demuxer,
    &ff_ifv_demuxer,
    &ff_ilbc_demuxer,
    &ff_image2_demuxer,
    &ff_image2pipe_demuxer,
    &ff_image2_alias_pix_demuxer,
    &ff_image2_brender_pix_demuxer,
    &ff_ingenient_demuxer,
    &ff_ipmovie_demuxer,
    &ff_ipu_demuxer,
    &ff_ircam_demuxer,
    &ff_iss_demuxer,
    &ff_iv8_demuxer,
    &ff_ivf_demuxer,
    &ff_ivr_demuxer,
    &ff_jacosub_demuxer,
    &ff_jv_demuxer,
    &ff_kux_demuxer,
    &ff_kvag_demuxer,
    &ff_lmlm4_demuxer,
    &ff_loas_demuxer,
    &ff_luodat_demuxer,
    &ff_lrc_demuxer,
    &ff_lvf_demuxer,
    &ff_lxf_demuxer,
    &ff_m4v_demuxer,
    &ff_mca_demuxer,
    &ff_mcc_demuxer,
    &ff_matroska_demuxer,
    &ff_mgsts_demuxer,
    &ff_microdvd_demuxer,
    &ff_mjpeg_demuxer,
    &ff_mjpeg_2000_demuxer,
    &ff_mlp_demuxer,
    &ff_mlv_demuxer,
    &ff_mm_demuxer,
    &ff_mmf_demuxer,
    &ff_mods_demuxer,
    &ff_moflex_demuxer,
    &ff_mov_demuxer,
    &ff_mp3_demuxer,
    &ff_mpc_demuxer,
    &ff_mpc8_demuxer,
    &ff_mpegps_demuxer,
    &ff_mpegts_demuxer,
    &ff_mpegtsraw_demuxer,
    &ff_mpegvideo_demuxer,
    &ff_mpjpeg_demuxer,
    &ff_mpl2_demuxer,
    &ff_mpsub_demuxer,
    &ff_msf_demuxer,
    &ff_msnwc_tcp_demuxer,
    &ff_mtaf_demuxer,
    &ff_mtv_demuxer,
    &ff_musx_demuxer,
    &ff_mv_demuxer,
    &ff_mvi_demuxer,
    &ff_mxf_demuxer,
    &ff_mxg_demuxer,
    &ff_nc_demuxer,
    &ff_nistsphere_demuxer,
    &ff_nsp_demuxer,
    &ff_nsv_demuxer,
    &ff_nut_demuxer,
    &ff_nuv_demuxer,
    &ff_obu_demuxer,
    &ff_ogg_demuxer,
    &ff_oma_demuxer,
    &ff_paf_demuxer,
    &ff_pcm_alaw_demuxer,
    &ff_pcm_mulaw_demuxer,
    &ff_pcm_vidc_demuxer,
    &ff_pcm_f64be_demuxer,
    &ff_pcm_f64le_demuxer,
    &ff_pcm_f32be_demuxer,
    &ff_pcm_f32le_demuxer,
    &ff_pcm_s32be_demuxer,
    &ff_pcm_s32le_demuxer,
    &ff_pcm_s24be_demuxer,
    &ff_pcm_s24le_demuxer,
    &ff_pcm_s16be_demuxer,
    &ff_pcm_s16le_demuxer,
    &ff_pcm_s8_demuxer,
    &ff_pcm_u32be_demuxer,
    &ff_pcm_u32le_demuxer,
    &ff_pcm_u24be_demuxer,
    &ff_pcm_u24le_demuxer,
    &ff_pcm_u16be_demuxer,
    &ff_pcm_u16le_demuxer,
    &ff_pcm_u8_demuxer,
    &ff_pjs_demuxer,
    &ff_pmp_demuxer,
    &ff_pp_bnk_demuxer,
    &ff_pva_demuxer,
    &ff_pvf_demuxer,
    &ff_qcp_demuxer,
    &ff_r3d_demuxer,
    &ff_rawvideo_demuxer,
    &ff_realtext_demuxer,
    &ff_redspark_demuxer,
    &ff_rl2_demuxer,
    &ff_rm_demuxer,
    &ff_roq_demuxer,
    &ff_rpl_demuxer,
    &ff_rsd_demuxer,
    &ff_rso_demuxer,
    &ff_rtp_demuxer,
    &ff_rtsp_demuxer,
    &ff_s337m_demuxer,
    &ff_sami_demuxer,
    &ff_sap_demuxer,
    &ff_sbc_demuxer,
    &ff_sbg_demuxer,
    &ff_scc_demuxer,
    &ff_sdp_demuxer,
    &ff_sdr2_demuxer,
    &ff_sds_demuxer,
    &ff_sdx_demuxer,
    &ff_segafilm_demuxer,
    &ff_ser_demuxer,
    &ff_shorten_demuxer,
    &ff_siff_demuxer,
    &ff_sln_demuxer,
    &ff_smacker_demuxer,
    &ff_smjpeg_demuxer,
    &ff_smush_demuxer,
    &ff_sol_demuxer,
    &ff_sox_demuxer,
    &ff_spdif_demuxer,
    &ff_srt_demuxer,
    &ff_str_demuxer,
    &ff_stl_demuxer,
    &ff_subviewer1_demuxer,
    &ff_subviewer_demuxer,
    &ff_sup_demuxer,
    &ff_svag_demuxer,
    &ff_svs_demuxer,
    &ff_swf_demuxer,
    &ff_tak_demuxer,
    &ff_tedcaptions_demuxer,
    &ff_thp_demuxer,
    &ff_threedostr_demuxer,
    &ff_tiertexseq_demuxer,
    &ff_tmv_demuxer,
    &ff_truehd_demuxer,
    &ff_tta_demuxer,
    &ff_txd_demuxer,
    &ff_tty_demuxer,
    &ff_ty_demuxer,
    &ff_v210_demuxer,
    &ff_v210x_demuxer,
    &ff_vag_demuxer,
    &ff_vc1_demuxer,
    &ff_vc1t_demuxer,
    &ff_vividas_demuxer,
    &ff_vivo_demuxer,
    &ff_vmd_demuxer,
    &ff_vobsub_demuxer,
    &ff_voc_demuxer,
    &ff_vpk_demuxer,
    &ff_vplayer_demuxer,
    &ff_vqf_demuxer,
    &ff_w64_demuxer,
    &ff_wav_demuxer,
    &ff_wc3_demuxer,
    &ff_webm_dash_manifest_demuxer,
    &ff_webvtt_demuxer,
    &ff_wsaud_demuxer,
    &ff_wsd_demuxer,
    &ff_wsvqa_demuxer,
    &ff_wtv_demuxer,
    &ff_wve_demuxer,
    &ff_wv_demuxer,
    &ff_xa_demuxer,
    &ff_xbin_demuxer,
    &ff_xmv_demuxer,
    &ff_xvag_demuxer,
    &ff_xwma_demuxer,
    &ff_yop_demuxer,
    &ff_yuv4mpegpipe_demuxer,
    &ff_image_bmp_pipe_demuxer,
    &ff_image_dds_pipe_demuxer,
    &ff_image_dpx_pipe_demuxer,
    &ff_image_exr_pipe_demuxer,
    &ff_image_gif_pipe_demuxer,
    &ff_image_j2k_pipe_demuxer,
    &ff_image_jpeg_pipe_demuxer,
    &ff_image_jpegls_pipe_demuxer,
    &ff_image_pam_pipe_demuxer,
    &ff_image_pbm_pipe_demuxer,
    &ff_image_pcx_pipe_demuxer,
    &ff_image_pgmyuv_pipe_demuxer,
    &ff_image_pgm_pipe_demuxer,
    &ff_image_pgx_pipe_demuxer,
    &ff_image_photocd_pipe_demuxer,
    &ff_image_pictor_pipe_demuxer,
    &ff_image_png_pipe_demuxer,
    &ff_image_ppm_pipe_demuxer,
    &ff_image_psd_pipe_demuxer,
    &ff_image_qdraw_pipe_demuxer,
    &ff_image_sgi_pipe_demuxer,
    &ff_image_svg_pipe_demuxer,
    &ff_image_sunrast_pipe_demuxer,
    &ff_image_tiff_pipe_demuxer,
    &ff_image_webp_pipe_demuxer,
    &ff_image_xpm_pipe_demuxer,
    &ff_image_xwd_pipe_demuxer,
    &ff_libgme_demuxer,
    &ff_libmodplug_demuxer,
    NULL };
#define MT(...) (const char *const[]){ __VA_ARGS__, NULL }

static const AVCodecDescriptor codec_descriptors[] = {
    /* video codecs */
    {
        .id        = AV_CODEC_ID_MPEG1VIDEO,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "mpeg1video",
        .long_name = NULL_IF_CONFIG_SMALL("MPEG-1 video"),
        .props     = AV_CODEC_PROP_LOSSY | AV_CODEC_PROP_REORDER,
    },
    {
        .id        = AV_CODEC_ID_MPEG2VIDEO,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "mpeg2video",
        .long_name = NULL_IF_CONFIG_SMALL("MPEG-2 video"),
        .props     = AV_CODEC_PROP_LOSSY | AV_CODEC_PROP_REORDER,
        .profiles  = NULL_IF_CONFIG_SMALL(ff_mpeg2_video_profiles),
    },
    {
        .id        = AV_CODEC_ID_H261,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "h261",
        .long_name = NULL_IF_CONFIG_SMALL("H.261"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_H263,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "h263",
        .long_name = NULL_IF_CONFIG_SMALL("H.263 / H.263-1996, H.263+ / H.263-1998 / H.263 version 2"),
        .props     = AV_CODEC_PROP_LOSSY | AV_CODEC_PROP_REORDER,
    },
    {
        .id        = AV_CODEC_ID_RV10,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "rv10",
        .long_name = NULL_IF_CONFIG_SMALL("RealVideo 1.0"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_RV20,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "rv20",
        .long_name = NULL_IF_CONFIG_SMALL("RealVideo 2.0"),
        .props     = AV_CODEC_PROP_LOSSY | AV_CODEC_PROP_REORDER,
    },
    {
        .id        = AV_CODEC_ID_MJPEG,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "mjpeg",
        .long_name = NULL_IF_CONFIG_SMALL("Motion JPEG"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
        .mime_types= MT("image/jpeg"),
        .profiles  = NULL_IF_CONFIG_SMALL(ff_mjpeg_profiles),
    },
    {
        .id        = AV_CODEC_ID_MJPEGB,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "mjpegb",
        .long_name = NULL_IF_CONFIG_SMALL("Apple MJPEG-B"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_LJPEG,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "ljpeg",
        .long_name = NULL_IF_CONFIG_SMALL("Lossless JPEG"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_SP5X,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "sp5x",
        .long_name = NULL_IF_CONFIG_SMALL("Sunplus JPEG (SP5X)"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_JPEGLS,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "jpegls",
        .long_name = NULL_IF_CONFIG_SMALL("JPEG-LS"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY |
                     AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_MPEG4,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "mpeg4",
        .long_name = NULL_IF_CONFIG_SMALL("MPEG-4 part 2"),
        .props     = AV_CODEC_PROP_LOSSY | AV_CODEC_PROP_REORDER,
        .profiles  = NULL_IF_CONFIG_SMALL(ff_mpeg4_video_profiles),
    },
    {
        .id        = AV_CODEC_ID_RAWVIDEO,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "rawvideo",
        .long_name = NULL_IF_CONFIG_SMALL("raw video"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_MSMPEG4V1,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "msmpeg4v1",
        .long_name = NULL_IF_CONFIG_SMALL("MPEG-4 part 2 Microsoft variant version 1"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_MSMPEG4V2,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "msmpeg4v2",
        .long_name = NULL_IF_CONFIG_SMALL("MPEG-4 part 2 Microsoft variant version 2"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_MSMPEG4V3,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "msmpeg4v3",
        .long_name = NULL_IF_CONFIG_SMALL("MPEG-4 part 2 Microsoft variant version 3"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_WMV1,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "wmv1",
        .long_name = NULL_IF_CONFIG_SMALL("Windows Media Video 7"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_WMV2,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "wmv2",
        .long_name = NULL_IF_CONFIG_SMALL("Windows Media Video 8"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_H263P,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "h263p",
        .long_name = NULL_IF_CONFIG_SMALL("H.263+ / H.263-1998 / H.263 version 2"),
        .props     = AV_CODEC_PROP_LOSSY | AV_CODEC_PROP_REORDER,
    },
    {
        .id        = AV_CODEC_ID_H263I,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "h263i",
        .long_name = NULL_IF_CONFIG_SMALL("Intel H.263"),
        .props     = AV_CODEC_PROP_LOSSY | AV_CODEC_PROP_REORDER,
    },
    {
        .id        = AV_CODEC_ID_FLV1,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "flv1",
        .long_name = NULL_IF_CONFIG_SMALL("FLV / Sorenson Spark / Sorenson H.263 (Flash Video)"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_SVQ1,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "svq1",
        .long_name = NULL_IF_CONFIG_SMALL("Sorenson Vector Quantizer 1 / Sorenson Video 1 / SVQ1"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_SVQ3,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "svq3",
        .long_name = NULL_IF_CONFIG_SMALL("Sorenson Vector Quantizer 3 / Sorenson Video 3 / SVQ3"),
        .props     = AV_CODEC_PROP_LOSSY | AV_CODEC_PROP_REORDER,
    },
    {
        .id        = AV_CODEC_ID_DVVIDEO,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "dvvideo",
        .long_name = NULL_IF_CONFIG_SMALL("DV (Digital Video)"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_HUFFYUV,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "huffyuv",
        .long_name = NULL_IF_CONFIG_SMALL("HuffYUV"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_CYUV,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "cyuv",
        .long_name = NULL_IF_CONFIG_SMALL("Creative YUV (CYUV)"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_H264,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "h264",
        .long_name = NULL_IF_CONFIG_SMALL("H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10"),
        .props     = AV_CODEC_PROP_LOSSY | AV_CODEC_PROP_LOSSLESS | AV_CODEC_PROP_REORDER,
        .profiles  = NULL_IF_CONFIG_SMALL(ff_h264_profiles),
    },
    {
        .id        = AV_CODEC_ID_INDEO3,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "indeo3",
        .long_name = NULL_IF_CONFIG_SMALL("Intel Indeo 3"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_VP3,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "vp3",
        .long_name = NULL_IF_CONFIG_SMALL("On2 VP3"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_THEORA,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "theora",
        .long_name = NULL_IF_CONFIG_SMALL("Theora"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ASV1,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "asv1",
        .long_name = NULL_IF_CONFIG_SMALL("ASUS V1"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ASV2,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "asv2",
        .long_name = NULL_IF_CONFIG_SMALL("ASUS V2"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_FFV1,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "ffv1",
        .long_name = NULL_IF_CONFIG_SMALL("FFmpeg video codec #1"),
        .props     = AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_4XM,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "4xm",
        .long_name = NULL_IF_CONFIG_SMALL("4X Movie"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_VCR1,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "vcr1",
        .long_name = NULL_IF_CONFIG_SMALL("ATI VCR1"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_CLJR,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "cljr",
        .long_name = NULL_IF_CONFIG_SMALL("Cirrus Logic AccuPak"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_MDEC,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "mdec",
        .long_name = NULL_IF_CONFIG_SMALL("Sony PlayStation MDEC (Motion DECoder)"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ROQ,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "roq",
        .long_name = NULL_IF_CONFIG_SMALL("id RoQ video"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_INTERPLAY_VIDEO,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "interplayvideo",
        .long_name = NULL_IF_CONFIG_SMALL("Interplay MVE video"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_XAN_WC3,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "xan_wc3",
        .long_name = NULL_IF_CONFIG_SMALL("Wing Commander III / Xan"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_XAN_WC4,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "xan_wc4",
        .long_name = NULL_IF_CONFIG_SMALL("Wing Commander IV / Xxan"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_RPZA,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "rpza",
        .long_name = NULL_IF_CONFIG_SMALL("QuickTime video (RPZA)"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_CINEPAK,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "cinepak",
        .long_name = NULL_IF_CONFIG_SMALL("Cinepak"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_WS_VQA,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "ws_vqa",
        .long_name = NULL_IF_CONFIG_SMALL("Westwood Studios VQA (Vector Quantized Animation) video"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_MSRLE,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "msrle",
        .long_name = NULL_IF_CONFIG_SMALL("Microsoft RLE"),
        .props     = AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_MSVIDEO1,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "msvideo1",
        .long_name = NULL_IF_CONFIG_SMALL("Microsoft Video 1"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_IDCIN,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "idcin",
        .long_name = NULL_IF_CONFIG_SMALL("id Quake II CIN video"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_8BPS,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "8bps",
        .long_name = NULL_IF_CONFIG_SMALL("QuickTime 8BPS video"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_SMC,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "smc",
        .long_name = NULL_IF_CONFIG_SMALL("QuickTime Graphics (SMC)"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_FLIC,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "flic",
        .long_name = NULL_IF_CONFIG_SMALL("Autodesk Animator Flic video"),
        .props     = AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_TRUEMOTION1,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "truemotion1",
        .long_name = NULL_IF_CONFIG_SMALL("Duck TrueMotion 1.0"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_VMDVIDEO,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "vmdvideo",
        .long_name = NULL_IF_CONFIG_SMALL("Sierra VMD video"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_MSZH,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "mszh",
        .long_name = NULL_IF_CONFIG_SMALL("LCL (LossLess Codec Library) MSZH"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_ZLIB,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "zlib",
        .long_name = NULL_IF_CONFIG_SMALL("LCL (LossLess Codec Library) ZLIB"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_QTRLE,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "qtrle",
        .long_name = NULL_IF_CONFIG_SMALL("QuickTime Animation (RLE) video"),
        .props     = AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_TSCC,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "tscc",
        .long_name = NULL_IF_CONFIG_SMALL("TechSmith Screen Capture Codec"),
        .props     = AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_ULTI,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "ulti",
        .long_name = NULL_IF_CONFIG_SMALL("IBM UltiMotion"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_QDRAW,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "qdraw",
        .long_name = NULL_IF_CONFIG_SMALL("Apple QuickDraw"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_VIXL,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "vixl",
        .long_name = NULL_IF_CONFIG_SMALL("Miro VideoXL"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_QPEG,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "qpeg",
        .long_name = NULL_IF_CONFIG_SMALL("Q-team QPEG"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_PNG,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "png",
        .long_name = NULL_IF_CONFIG_SMALL("PNG (Portable Network Graphics) image"),
        .props     = AV_CODEC_PROP_LOSSLESS,
        .mime_types= MT("image/png"),
    },
    {
        .id        = AV_CODEC_ID_PPM,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "ppm",
        .long_name = NULL_IF_CONFIG_SMALL("PPM (Portable PixelMap) image"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_PBM,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "pbm",
        .long_name = NULL_IF_CONFIG_SMALL("PBM (Portable BitMap) image"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_PGM,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "pgm",
        .long_name = NULL_IF_CONFIG_SMALL("PGM (Portable GrayMap) image"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_PGMYUV,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "pgmyuv",
        .long_name = NULL_IF_CONFIG_SMALL("PGMYUV (Portable GrayMap YUV) image"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_PAM,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "pam",
        .long_name = NULL_IF_CONFIG_SMALL("PAM (Portable AnyMap) image"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
        .mime_types= MT("image/x-portable-pixmap"),
    },
    {
        .id        = AV_CODEC_ID_FFVHUFF,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "ffvhuff",
        .long_name = NULL_IF_CONFIG_SMALL("Huffyuv FFmpeg variant"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_RV30,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "rv30",
        .long_name = NULL_IF_CONFIG_SMALL("RealVideo 3.0"),
        .props     = AV_CODEC_PROP_LOSSY | AV_CODEC_PROP_REORDER,
    },
    {
        .id        = AV_CODEC_ID_RV40,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "rv40",
        .long_name = NULL_IF_CONFIG_SMALL("RealVideo 4.0"),
        .props     = AV_CODEC_PROP_LOSSY | AV_CODEC_PROP_REORDER,
    },
    {
        .id        = AV_CODEC_ID_VC1,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "vc1",
        .long_name = NULL_IF_CONFIG_SMALL("SMPTE VC-1"),
        .props     = AV_CODEC_PROP_LOSSY | AV_CODEC_PROP_REORDER,
        .profiles  = NULL_IF_CONFIG_SMALL(ff_vc1_profiles),
    },
    {
        .id        = AV_CODEC_ID_WMV3,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "wmv3",
        .long_name = NULL_IF_CONFIG_SMALL("Windows Media Video 9"),
        .props     = AV_CODEC_PROP_LOSSY | AV_CODEC_PROP_REORDER,
        .profiles  = NULL_IF_CONFIG_SMALL(ff_vc1_profiles),
    },
    {
        .id        = AV_CODEC_ID_LOCO,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "loco",
        .long_name = NULL_IF_CONFIG_SMALL("LOCO"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_WNV1,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "wnv1",
        .long_name = NULL_IF_CONFIG_SMALL("Winnov WNV1"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_AASC,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "aasc",
        .long_name = NULL_IF_CONFIG_SMALL("Autodesk RLE"),
        .props     = AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_INDEO2,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "indeo2",
        .long_name = NULL_IF_CONFIG_SMALL("Intel Indeo 2"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_FRAPS,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "fraps",
        .long_name = NULL_IF_CONFIG_SMALL("Fraps"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_TRUEMOTION2,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "truemotion2",
        .long_name = NULL_IF_CONFIG_SMALL("Duck TrueMotion 2.0"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_BMP,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "bmp",
        .long_name = NULL_IF_CONFIG_SMALL("BMP (Windows and OS/2 bitmap)"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
        .mime_types= MT("image/x-ms-bmp"),
    },
    {
        .id        = AV_CODEC_ID_CSCD,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "cscd",
        .long_name = NULL_IF_CONFIG_SMALL("CamStudio"),
        .props     = AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_MMVIDEO,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "mmvideo",
        .long_name = NULL_IF_CONFIG_SMALL("American Laser Games MM Video"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ZMBV,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "zmbv",
        .long_name = NULL_IF_CONFIG_SMALL("Zip Motion Blocks Video"),
        .props     = AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_AVS,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "avs",
        .long_name = NULL_IF_CONFIG_SMALL("AVS (Audio Video Standard) video"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_SMACKVIDEO,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "smackvideo",
        .long_name = NULL_IF_CONFIG_SMALL("Smacker video"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_NUV,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "nuv",
        .long_name = NULL_IF_CONFIG_SMALL("NuppelVideo/RTJPEG"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_KMVC,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "kmvc",
        .long_name = NULL_IF_CONFIG_SMALL("Karl Morton's video codec"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_FLASHSV,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "flashsv",
        .long_name = NULL_IF_CONFIG_SMALL("Flash Screen Video v1"),
        .props     = AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_CAVS,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "cavs",
        .long_name = NULL_IF_CONFIG_SMALL("Chinese AVS (Audio Video Standard) (AVS1-P2, JiZhun profile)"),
        .props     = AV_CODEC_PROP_LOSSY | AV_CODEC_PROP_REORDER,
    },
    {
        .id        = AV_CODEC_ID_JPEG2000,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "jpeg2000",
        .long_name = NULL_IF_CONFIG_SMALL("JPEG 2000"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY |
                     AV_CODEC_PROP_LOSSLESS,
        .mime_types= MT("image/jp2"),
        .profiles  = NULL_IF_CONFIG_SMALL(ff_jpeg2000_profiles),
    },
    {
        .id        = AV_CODEC_ID_VMNC,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "vmnc",
        .long_name = NULL_IF_CONFIG_SMALL("VMware Screen Codec / VMware Video"),
        .props     = AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_VP5,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "vp5",
        .long_name = NULL_IF_CONFIG_SMALL("On2 VP5"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_VP6,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "vp6",
        .long_name = NULL_IF_CONFIG_SMALL("On2 VP6"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_VP6F,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "vp6f",
        .long_name = NULL_IF_CONFIG_SMALL("On2 VP6 (Flash version)"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_TARGA,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "targa",
        .long_name = NULL_IF_CONFIG_SMALL("Truevision Targa image"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
        .mime_types= MT("image/x-targa", "image/x-tga"),
    },
    {
        .id        = AV_CODEC_ID_DSICINVIDEO,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "dsicinvideo",
        .long_name = NULL_IF_CONFIG_SMALL("Delphine Software International CIN video"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_TIERTEXSEQVIDEO,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "tiertexseqvideo",
        .long_name = NULL_IF_CONFIG_SMALL("Tiertex Limited SEQ video"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_TIFF,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "tiff",
        .long_name = NULL_IF_CONFIG_SMALL("TIFF image"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
        .mime_types= MT("image/tiff"),
    },
    {
        .id        = AV_CODEC_ID_GIF,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "gif",
        .long_name = NULL_IF_CONFIG_SMALL("CompuServe GIF (Graphics Interchange Format)"),
        .props     = AV_CODEC_PROP_LOSSLESS,
        .mime_types= MT("image/gif"),
    },
    {
        .id        = AV_CODEC_ID_DXA,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "dxa",
        .long_name = NULL_IF_CONFIG_SMALL("Feeble Files/ScummVM DXA"),
        .props     = AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_DNXHD,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "dnxhd",
        .long_name = NULL_IF_CONFIG_SMALL("VC3/DNxHD"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
        .profiles  = NULL_IF_CONFIG_SMALL(ff_dnxhd_profiles),
    },
    {
        .id        = AV_CODEC_ID_THP,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "thp",
        .long_name = NULL_IF_CONFIG_SMALL("Nintendo Gamecube THP video"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_SGI,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "sgi",
        .long_name = NULL_IF_CONFIG_SMALL("SGI image"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_C93,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "c93",
        .long_name = NULL_IF_CONFIG_SMALL("Interplay C93"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_BETHSOFTVID,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "bethsoftvid",
        .long_name = NULL_IF_CONFIG_SMALL("Bethesda VID video"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_PTX,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "ptx",
        .long_name = NULL_IF_CONFIG_SMALL("V.Flash PTX image"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_TXD,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "txd",
        .long_name = NULL_IF_CONFIG_SMALL("Renderware TXD (TeXture Dictionary) image"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_VP6A,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "vp6a",
        .long_name = NULL_IF_CONFIG_SMALL("On2 VP6 (Flash version, with alpha channel)"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_AMV,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "amv",
        .long_name = NULL_IF_CONFIG_SMALL("AMV Video"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_VB,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "vb",
        .long_name = NULL_IF_CONFIG_SMALL("Beam Software VB"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_PCX,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "pcx",
        .long_name = NULL_IF_CONFIG_SMALL("PC Paintbrush PCX image"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
        .mime_types= MT("image/x-pcx"),
    },
    {
        .id        = AV_CODEC_ID_SUNRAST,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "sunrast",
        .long_name = NULL_IF_CONFIG_SMALL("Sun Rasterfile image"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_INDEO4,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "indeo4",
        .long_name = NULL_IF_CONFIG_SMALL("Intel Indeo Video Interactive 4"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_INDEO5,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "indeo5",
        .long_name = NULL_IF_CONFIG_SMALL("Intel Indeo Video Interactive 5"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_MIMIC,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "mimic",
        .long_name = NULL_IF_CONFIG_SMALL("Mimic"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_RL2,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "rl2",
        .long_name = NULL_IF_CONFIG_SMALL("RL2 video"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ESCAPE124,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "escape124",
        .long_name = NULL_IF_CONFIG_SMALL("Escape 124"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_DIRAC,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "dirac",
        .long_name = NULL_IF_CONFIG_SMALL("Dirac"),
        .props     = AV_CODEC_PROP_LOSSY | AV_CODEC_PROP_LOSSLESS | AV_CODEC_PROP_REORDER,
    },
    {
        .id        = AV_CODEC_ID_BFI,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "bfi",
        .long_name = NULL_IF_CONFIG_SMALL("Brute Force & Ignorance"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_CMV,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "cmv",
        .long_name = NULL_IF_CONFIG_SMALL("Electronic Arts CMV video"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_MOTIONPIXELS,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "motionpixels",
        .long_name = NULL_IF_CONFIG_SMALL("Motion Pixels video"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_TGV,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "tgv",
        .long_name = NULL_IF_CONFIG_SMALL("Electronic Arts TGV video"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_TGQ,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "tgq",
        .long_name = NULL_IF_CONFIG_SMALL("Electronic Arts TGQ video"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_TQI,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "tqi",
        .long_name = NULL_IF_CONFIG_SMALL("Electronic Arts TQI video"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_AURA,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "aura",
        .long_name = NULL_IF_CONFIG_SMALL("Auravision AURA"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_AURA2,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "aura2",
        .long_name = NULL_IF_CONFIG_SMALL("Auravision Aura 2"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_V210X,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "v210x",
        .long_name = NULL_IF_CONFIG_SMALL("Uncompressed 4:2:2 10-bit"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_TMV,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "tmv",
        .long_name = NULL_IF_CONFIG_SMALL("8088flex TMV"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_V210,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "v210",
        .long_name = NULL_IF_CONFIG_SMALL("Uncompressed 4:2:2 10-bit"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_DPX,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "dpx",
        .long_name = NULL_IF_CONFIG_SMALL("DPX (Digital Picture Exchange) image"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_MAD,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "mad",
        .long_name = NULL_IF_CONFIG_SMALL("Electronic Arts Madcow Video"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_FRWU,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "frwu",
        .long_name = NULL_IF_CONFIG_SMALL("Forward Uncompressed"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_FLASHSV2,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "flashsv2",
        .long_name = NULL_IF_CONFIG_SMALL("Flash Screen Video v2"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_CDGRAPHICS,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "cdgraphics",
        .long_name = NULL_IF_CONFIG_SMALL("CD Graphics video"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_R210,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "r210",
        .long_name = NULL_IF_CONFIG_SMALL("Uncompressed RGB 10-bit"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_ANM,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "anm",
        .long_name = NULL_IF_CONFIG_SMALL("Deluxe Paint Animation"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_BINKVIDEO,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "binkvideo",
        .long_name = NULL_IF_CONFIG_SMALL("Bink video"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_IFF_ILBM,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "iff_ilbm",
        .long_name = NULL_IF_CONFIG_SMALL("IFF ACBM/ANIM/DEEP/ILBM/PBM/RGB8/RGBN"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_KGV1,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "kgv1",
        .long_name = NULL_IF_CONFIG_SMALL("Kega Game Video"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_YOP,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "yop",
        .long_name = NULL_IF_CONFIG_SMALL("Psygnosis YOP Video"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_VP8,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "vp8",
        .long_name = NULL_IF_CONFIG_SMALL("On2 VP8"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_PICTOR,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "pictor",
        .long_name = NULL_IF_CONFIG_SMALL("Pictor/PC Paint"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ANSI,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "ansi",
        .long_name = NULL_IF_CONFIG_SMALL("ASCII/ANSI art"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_A64_MULTI,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "a64_multi",
        .long_name = NULL_IF_CONFIG_SMALL("Multicolor charset for Commodore 64"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_A64_MULTI5,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "a64_multi5",
        .long_name = NULL_IF_CONFIG_SMALL("Multicolor charset for Commodore 64, extended with 5th color (colram)"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_R10K,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "r10k",
        .long_name = NULL_IF_CONFIG_SMALL("AJA Kona 10-bit RGB Codec"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_MXPEG,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "mxpeg",
        .long_name = NULL_IF_CONFIG_SMALL("Mobotix MxPEG video"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_LAGARITH,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "lagarith",
        .long_name = NULL_IF_CONFIG_SMALL("Lagarith lossless"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_PRORES,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "prores",
        .long_name = NULL_IF_CONFIG_SMALL("Apple ProRes (iCodec Pro)"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
        .profiles  = NULL_IF_CONFIG_SMALL(ff_prores_profiles),
    },
    {
        .id        = AV_CODEC_ID_JV,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "jv",
        .long_name = NULL_IF_CONFIG_SMALL("Bitmap Brothers JV video"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_DFA,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "dfa",
        .long_name = NULL_IF_CONFIG_SMALL("Chronomaster DFA"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_WMV3IMAGE,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "wmv3image",
        .long_name = NULL_IF_CONFIG_SMALL("Windows Media Video 9 Image"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_VC1IMAGE,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "vc1image",
        .long_name = NULL_IF_CONFIG_SMALL("Windows Media Video 9 Image v2"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_UTVIDEO,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "utvideo",
        .long_name = NULL_IF_CONFIG_SMALL("Ut Video"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_BMV_VIDEO,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "bmv_video",
        .long_name = NULL_IF_CONFIG_SMALL("Discworld II BMV video"),
        .props     = AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_VBLE,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "vble",
        .long_name = NULL_IF_CONFIG_SMALL("VBLE Lossless Codec"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_DXTORY,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "dxtory",
        .long_name = NULL_IF_CONFIG_SMALL("Dxtory"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_V410,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "v410",
        .long_name = NULL_IF_CONFIG_SMALL("Uncompressed 4:4:4 10-bit"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_XWD,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "xwd",
        .long_name = NULL_IF_CONFIG_SMALL("XWD (X Window Dump) image"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
        .mime_types= MT("image/x-xwindowdump"),
    },
    {
        .id        = AV_CODEC_ID_CDXL,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "cdxl",
        .long_name = NULL_IF_CONFIG_SMALL("Commodore CDXL video"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_XBM,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "xbm",
        .long_name = NULL_IF_CONFIG_SMALL("XBM (X BitMap) image"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
        .mime_types= MT("image/x-xbitmap"),
    },
    {
        .id        = AV_CODEC_ID_ZEROCODEC,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "zerocodec",
        .long_name = NULL_IF_CONFIG_SMALL("ZeroCodec Lossless Video"),
        .props     = AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_MSS1,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "mss1",
        .long_name = NULL_IF_CONFIG_SMALL("MS Screen 1"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_MSA1,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "msa1",
        .long_name = NULL_IF_CONFIG_SMALL("MS ATC Screen"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_TSCC2,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "tscc2",
        .long_name = NULL_IF_CONFIG_SMALL("TechSmith Screen Codec 2"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_MTS2,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "mts2",
        .long_name = NULL_IF_CONFIG_SMALL("MS Expression Encoder Screen"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_CLLC,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "cllc",
        .long_name = NULL_IF_CONFIG_SMALL("Canopus Lossless Codec"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_MSS2,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "mss2",
        .long_name = NULL_IF_CONFIG_SMALL("MS Windows Media Video V9 Screen"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_VP9,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "vp9",
        .long_name = NULL_IF_CONFIG_SMALL("Google VP9"),
        .props     = AV_CODEC_PROP_LOSSY,
        .profiles  = NULL_IF_CONFIG_SMALL(ff_vp9_profiles),
    },
    {
        .id        = AV_CODEC_ID_AIC,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "aic",
        .long_name = NULL_IF_CONFIG_SMALL("Apple Intermediate Codec"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ESCAPE130,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "escape130",
        .long_name = NULL_IF_CONFIG_SMALL("Escape 130"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_G2M,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "g2m",
        .long_name = NULL_IF_CONFIG_SMALL("Go2Meeting"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_WEBP,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "webp",
        .long_name = NULL_IF_CONFIG_SMALL("WebP"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY |
                     AV_CODEC_PROP_LOSSLESS,
        .mime_types= MT("image/webp"),
    },
    {
        .id        = AV_CODEC_ID_HNM4_VIDEO,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "hnm4video",
        .long_name = NULL_IF_CONFIG_SMALL("HNM 4 video"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_HEVC,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "hevc",
        .long_name = NULL_IF_CONFIG_SMALL("H.265 / HEVC (High Efficiency Video Coding)"),
        .props     = AV_CODEC_PROP_LOSSY | AV_CODEC_PROP_REORDER,
        .profiles  = NULL_IF_CONFIG_SMALL(ff_hevc_profiles),
    },
    {
        .id        = AV_CODEC_ID_FIC,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "fic",
        .long_name = NULL_IF_CONFIG_SMALL("Mirillis FIC"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ALIAS_PIX,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "alias_pix",
        .long_name = NULL_IF_CONFIG_SMALL("Alias/Wavefront PIX image"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_BRENDER_PIX,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "brender_pix",
        .long_name = NULL_IF_CONFIG_SMALL("BRender PIX image"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_PAF_VIDEO,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "paf_video",
        .long_name = NULL_IF_CONFIG_SMALL("Amazing Studio Packed Animation File Video"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_EXR,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "exr",
        .long_name = NULL_IF_CONFIG_SMALL("OpenEXR image"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY |
                     AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_VP7,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "vp7",
        .long_name = NULL_IF_CONFIG_SMALL("On2 VP7"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_SANM,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "sanm",
        .long_name = NULL_IF_CONFIG_SMALL("LucasArts SANM/SMUSH video"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_SGIRLE,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "sgirle",
        .long_name = NULL_IF_CONFIG_SMALL("SGI RLE 8-bit"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_MVC1,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "mvc1",
        .long_name = NULL_IF_CONFIG_SMALL("Silicon Graphics Motion Video Compressor 1"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_MVC2,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "mvc2",
        .long_name = NULL_IF_CONFIG_SMALL("Silicon Graphics Motion Video Compressor 2"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_HQX,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "hqx",
        .long_name = NULL_IF_CONFIG_SMALL("Canopus HQX"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_TDSC,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "tdsc",
        .long_name = NULL_IF_CONFIG_SMALL("TDSC"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_HQ_HQA,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "hq_hqa",
        .long_name = NULL_IF_CONFIG_SMALL("Canopus HQ/HQA"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_HAP,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "hap",
        .long_name = NULL_IF_CONFIG_SMALL("Vidvox Hap"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_DDS,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "dds",
        .long_name = NULL_IF_CONFIG_SMALL("DirectDraw Surface image decoder"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY |
                     AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_DXV,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "dxv",
        .long_name = NULL_IF_CONFIG_SMALL("Resolume DXV"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_SCREENPRESSO,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "screenpresso",
        .long_name = NULL_IF_CONFIG_SMALL("Screenpresso"),
        .props     = AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_RSCC,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "rscc",
        .long_name = NULL_IF_CONFIG_SMALL("innoHeim/Rsupport Screen Capture Codec"),
        .props     = AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_AVS2,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "avs2",
        .long_name = NULL_IF_CONFIG_SMALL("AVS2-P2/IEEE1857.4"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_PGX,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "pgx",
        .long_name = NULL_IF_CONFIG_SMALL("PGX (JPEG2000 Test Format)"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_AVS3,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "avs3",
        .long_name = NULL_IF_CONFIG_SMALL("AVS3-P2/IEEE1857.10"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_Y41P,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "y41p",
        .long_name = NULL_IF_CONFIG_SMALL("Uncompressed YUV 4:1:1 12-bit"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_AVRP,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "avrp",
        .long_name = NULL_IF_CONFIG_SMALL("Avid 1:1 10-bit RGB Packer"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_012V,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "012v",
        .long_name = NULL_IF_CONFIG_SMALL("Uncompressed 4:2:2 10-bit"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_AVUI,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "avui",
        .long_name = NULL_IF_CONFIG_SMALL("Avid Meridien Uncompressed"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_AYUV,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "ayuv",
        .long_name = NULL_IF_CONFIG_SMALL("Uncompressed packed MS 4:4:4:4"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_TARGA_Y216,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "targa_y216",
        .long_name = NULL_IF_CONFIG_SMALL("Pinnacle TARGA CineWave YUV16"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_V308,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "v308",
        .long_name = NULL_IF_CONFIG_SMALL("Uncompressed packed 4:4:4"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_V408,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "v408",
        .long_name = NULL_IF_CONFIG_SMALL("Uncompressed packed QT 4:4:4:4"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_YUV4,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "yuv4",
        .long_name = NULL_IF_CONFIG_SMALL("Uncompressed packed 4:2:0"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_AVRN,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "avrn",
        .long_name = NULL_IF_CONFIG_SMALL("Avid AVI Codec"),
    },
    {
        .id        = AV_CODEC_ID_CPIA,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "cpia",
        .long_name = NULL_IF_CONFIG_SMALL("CPiA video format"),
    },
    {
        .id        = AV_CODEC_ID_XFACE,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "xface",
        .long_name = NULL_IF_CONFIG_SMALL("X-face image"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_SNOW,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "snow",
        .long_name = NULL_IF_CONFIG_SMALL("Snow"),
        .props     = AV_CODEC_PROP_LOSSY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_SMVJPEG,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "smvjpeg",
        .long_name = NULL_IF_CONFIG_SMALL("Sigmatel Motion Video"),
    },
    {
        .id        = AV_CODEC_ID_APNG,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "apng",
        .long_name = NULL_IF_CONFIG_SMALL("APNG (Animated Portable Network Graphics) image"),
        .props     = AV_CODEC_PROP_LOSSLESS,
        .mime_types= MT("image/png"),
    },
    {
        .id        = AV_CODEC_ID_DAALA,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "daala",
        .long_name = NULL_IF_CONFIG_SMALL("Daala"),
        .props     = AV_CODEC_PROP_LOSSY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_CFHD,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "cfhd",
        .long_name = NULL_IF_CONFIG_SMALL("GoPro CineForm HD"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_TRUEMOTION2RT,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "truemotion2rt",
        .long_name = NULL_IF_CONFIG_SMALL("Duck TrueMotion 2.0 Real Time"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_M101,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "m101",
        .long_name = NULL_IF_CONFIG_SMALL("Matrox Uncompressed SD"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_MAGICYUV,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "magicyuv",
        .long_name = NULL_IF_CONFIG_SMALL("MagicYUV video"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_SHEERVIDEO,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "sheervideo",
        .long_name = NULL_IF_CONFIG_SMALL("BitJazz SheerVideo"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_YLC,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "ylc",
        .long_name = NULL_IF_CONFIG_SMALL("YUY2 Lossless Codec"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_PSD,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "psd",
        .long_name = NULL_IF_CONFIG_SMALL("Photoshop PSD file"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_PIXLET,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "pixlet",
        .long_name = NULL_IF_CONFIG_SMALL("Apple Pixlet"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_SPEEDHQ,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "speedhq",
        .long_name = NULL_IF_CONFIG_SMALL("NewTek SpeedHQ"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_FMVC,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "fmvc",
        .long_name = NULL_IF_CONFIG_SMALL("FM Screen Capture Codec"),
        .props     = AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_SCPR,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "scpr",
        .long_name = NULL_IF_CONFIG_SMALL("ScreenPressor"),
        .props     = AV_CODEC_PROP_LOSSLESS | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_CLEARVIDEO,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "clearvideo",
        .long_name = NULL_IF_CONFIG_SMALL("Iterated Systems ClearVideo"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_XPM,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "xpm",
        .long_name = NULL_IF_CONFIG_SMALL("XPM (X PixMap) image"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
        .mime_types= MT("image/x-xpixmap"),
    },
    {
        .id        = AV_CODEC_ID_AV1,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "av1",
        .long_name = NULL_IF_CONFIG_SMALL("Alliance for Open Media AV1"),
        .props     = AV_CODEC_PROP_LOSSY,
        .profiles  = NULL_IF_CONFIG_SMALL(ff_av1_profiles),
    },
    {
        .id        = AV_CODEC_ID_BITPACKED,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "bitpacked",
        .long_name = NULL_IF_CONFIG_SMALL("Bitpacked"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_MSCC,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "mscc",
        .long_name = NULL_IF_CONFIG_SMALL("Mandsoft Screen Capture Codec"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_SRGC,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "srgc",
        .long_name = NULL_IF_CONFIG_SMALL("Screen Recorder Gold Codec"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_SVG,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "svg",
        .long_name = NULL_IF_CONFIG_SMALL("Scalable Vector Graphics"),
        .props     = AV_CODEC_PROP_LOSSLESS,
        .mime_types= MT("image/svg+xml"),
    },
    {
        .id        = AV_CODEC_ID_GDV,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "gdv",
        .long_name = NULL_IF_CONFIG_SMALL("Gremlin Digital Video"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_FITS,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "fits",
        .long_name = NULL_IF_CONFIG_SMALL("FITS (Flexible Image Transport System)"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_IMM4,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "imm4",
        .long_name = NULL_IF_CONFIG_SMALL("Infinity IMM4"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_PROSUMER,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "prosumer",
        .long_name = NULL_IF_CONFIG_SMALL("Brooktree ProSumer Video"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_MWSC,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "mwsc",
        .long_name = NULL_IF_CONFIG_SMALL("MatchWare Screen Capture Codec"),
        .props     = AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_WCMV,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "wcmv",
        .long_name = NULL_IF_CONFIG_SMALL("WinCAM Motion Video"),
        .props     = AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_RASC,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "rasc",
        .long_name = NULL_IF_CONFIG_SMALL("RemotelyAnywhere Screen Capture"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_HYMT,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "hymt",
        .long_name = NULL_IF_CONFIG_SMALL("HuffYUV MT"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_ARBC,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "arbc",
        .long_name = NULL_IF_CONFIG_SMALL("Gryphon's Anim Compressor"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_AGM,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "agm",
        .long_name = NULL_IF_CONFIG_SMALL("Amuse Graphics Movie"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_LSCR,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "lscr",
        .long_name = NULL_IF_CONFIG_SMALL("LEAD Screen Capture"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_VP4,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "vp4",
        .long_name = NULL_IF_CONFIG_SMALL("On2 VP4"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_IMM5,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "imm5",
        .long_name = NULL_IF_CONFIG_SMALL("Infinity IMM5"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_MVDV,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "mvdv",
        .long_name = NULL_IF_CONFIG_SMALL("MidiVid VQ"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_MVHA,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "mvha",
        .long_name = NULL_IF_CONFIG_SMALL("MidiVid Archive Codec"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_CDTOONS,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "cdtoons",
        .long_name = NULL_IF_CONFIG_SMALL("CDToons video"),
        .props     = AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_MV30,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "mv30",
        .long_name = NULL_IF_CONFIG_SMALL("MidiVid 3.0"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_NOTCHLC,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "notchlc",
        .long_name = NULL_IF_CONFIG_SMALL("NotchLC"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_PFM,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "pfm",
        .long_name = NULL_IF_CONFIG_SMALL("PFM (Portable FloatMap) image"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_MOBICLIP,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "mobiclip",
        .long_name = NULL_IF_CONFIG_SMALL("MobiClip Video"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_PHOTOCD,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "photocd",
        .long_name = NULL_IF_CONFIG_SMALL("Kodak Photo CD"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_IPU,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "ipu",
        .long_name = NULL_IF_CONFIG_SMALL("IPU Video"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ARGO,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "argo",
        .long_name = NULL_IF_CONFIG_SMALL("Argonaut Games Video"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_CRI,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "cri",
        .long_name = NULL_IF_CONFIG_SMALL("Cintel RAW"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY | AV_CODEC_PROP_LOSSLESS,
    },

    /* various PCM "codecs" */
    {
        .id        = AV_CODEC_ID_PCM_S16LE,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "pcm_s16le",
        .long_name = NULL_IF_CONFIG_SMALL("PCM signed 16-bit little-endian"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_PCM_S16BE,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "pcm_s16be",
        .long_name = NULL_IF_CONFIG_SMALL("PCM signed 16-bit big-endian"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_PCM_U16LE,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "pcm_u16le",
        .long_name = NULL_IF_CONFIG_SMALL("PCM unsigned 16-bit little-endian"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_PCM_U16BE,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "pcm_u16be",
        .long_name = NULL_IF_CONFIG_SMALL("PCM unsigned 16-bit big-endian"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_PCM_S8,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "pcm_s8",
        .long_name = NULL_IF_CONFIG_SMALL("PCM signed 8-bit"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_PCM_U8,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "pcm_u8",
        .long_name = NULL_IF_CONFIG_SMALL("PCM unsigned 8-bit"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_PCM_MULAW,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "pcm_mulaw",
        .long_name = NULL_IF_CONFIG_SMALL("PCM mu-law / G.711 mu-law"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_PCM_ALAW,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "pcm_alaw",
        .long_name = NULL_IF_CONFIG_SMALL("PCM A-law / G.711 A-law"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_PCM_S32LE,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "pcm_s32le",
        .long_name = NULL_IF_CONFIG_SMALL("PCM signed 32-bit little-endian"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_PCM_S32BE,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "pcm_s32be",
        .long_name = NULL_IF_CONFIG_SMALL("PCM signed 32-bit big-endian"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_PCM_U32LE,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "pcm_u32le",
        .long_name = NULL_IF_CONFIG_SMALL("PCM unsigned 32-bit little-endian"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_PCM_U32BE,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "pcm_u32be",
        .long_name = NULL_IF_CONFIG_SMALL("PCM unsigned 32-bit big-endian"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_PCM_S24LE,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "pcm_s24le",
        .long_name = NULL_IF_CONFIG_SMALL("PCM signed 24-bit little-endian"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_PCM_S24BE,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "pcm_s24be",
        .long_name = NULL_IF_CONFIG_SMALL("PCM signed 24-bit big-endian"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_PCM_U24LE,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "pcm_u24le",
        .long_name = NULL_IF_CONFIG_SMALL("PCM unsigned 24-bit little-endian"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_PCM_U24BE,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "pcm_u24be",
        .long_name = NULL_IF_CONFIG_SMALL("PCM unsigned 24-bit big-endian"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_PCM_S24DAUD,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "pcm_s24daud",
        .long_name = NULL_IF_CONFIG_SMALL("PCM D-Cinema audio signed 24-bit"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_PCM_S16LE_PLANAR,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "pcm_s16le_planar",
        .long_name = NULL_IF_CONFIG_SMALL("PCM signed 16-bit little-endian planar"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_PCM_DVD,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "pcm_dvd",
        .long_name = NULL_IF_CONFIG_SMALL("PCM signed 20|24-bit big-endian"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_PCM_F32BE,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "pcm_f32be",
        .long_name = NULL_IF_CONFIG_SMALL("PCM 32-bit floating point big-endian"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_PCM_F32LE,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "pcm_f32le",
        .long_name = NULL_IF_CONFIG_SMALL("PCM 32-bit floating point little-endian"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_PCM_F64BE,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "pcm_f64be",
        .long_name = NULL_IF_CONFIG_SMALL("PCM 64-bit floating point big-endian"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_PCM_F64LE,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "pcm_f64le",
        .long_name = NULL_IF_CONFIG_SMALL("PCM 64-bit floating point little-endian"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_PCM_BLURAY,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "pcm_bluray",
        .long_name = NULL_IF_CONFIG_SMALL("PCM signed 16|20|24-bit big-endian for Blu-ray media"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_PCM_LXF,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "pcm_lxf",
        .long_name = NULL_IF_CONFIG_SMALL("PCM signed 20-bit little-endian planar"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_S302M,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "s302m",
        .long_name = NULL_IF_CONFIG_SMALL("SMPTE 302M"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_PCM_S8_PLANAR,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "pcm_s8_planar",
        .long_name = NULL_IF_CONFIG_SMALL("PCM signed 8-bit planar"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_PCM_S24LE_PLANAR,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "pcm_s24le_planar",
        .long_name = NULL_IF_CONFIG_SMALL("PCM signed 24-bit little-endian planar"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_PCM_S32LE_PLANAR,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "pcm_s32le_planar",
        .long_name = NULL_IF_CONFIG_SMALL("PCM signed 32-bit little-endian planar"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_PCM_S16BE_PLANAR,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "pcm_s16be_planar",
        .long_name = NULL_IF_CONFIG_SMALL("PCM signed 16-bit big-endian planar"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_PCM_S64LE,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "pcm_s64le",
        .long_name = NULL_IF_CONFIG_SMALL("PCM signed 64-bit little-endian"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_PCM_S64BE,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "pcm_s64be",
        .long_name = NULL_IF_CONFIG_SMALL("PCM signed 64-bit big-endian"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_PCM_F16LE,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "pcm_f16le",
        .long_name = NULL_IF_CONFIG_SMALL("PCM 16.8 floating point little-endian"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_PCM_F24LE,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "pcm_f24le",
        .long_name = NULL_IF_CONFIG_SMALL("PCM 24.0 floating point little-endian"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_PCM_VIDC,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "pcm_vidc",
        .long_name = NULL_IF_CONFIG_SMALL("PCM Archimedes VIDC"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },

    /* various ADPCM codecs */
    {
        .id        = AV_CODEC_ID_ADPCM_IMA_QT,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "adpcm_ima_qt",
        .long_name = NULL_IF_CONFIG_SMALL("ADPCM IMA QuickTime"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ADPCM_IMA_WAV,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "adpcm_ima_wav",
        .long_name = NULL_IF_CONFIG_SMALL("ADPCM IMA WAV"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ADPCM_IMA_DK3,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "adpcm_ima_dk3",
        .long_name = NULL_IF_CONFIG_SMALL("ADPCM IMA Duck DK3"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ADPCM_IMA_DK4,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "adpcm_ima_dk4",
        .long_name = NULL_IF_CONFIG_SMALL("ADPCM IMA Duck DK4"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ADPCM_IMA_WS,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "adpcm_ima_ws",
        .long_name = NULL_IF_CONFIG_SMALL("ADPCM IMA Westwood"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ADPCM_IMA_SMJPEG,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "adpcm_ima_smjpeg",
        .long_name = NULL_IF_CONFIG_SMALL("ADPCM IMA Loki SDL MJPEG"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ADPCM_MS,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "adpcm_ms",
        .long_name = NULL_IF_CONFIG_SMALL("ADPCM Microsoft"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ADPCM_4XM,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "adpcm_4xm",
        .long_name = NULL_IF_CONFIG_SMALL("ADPCM 4X Movie"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ADPCM_XA,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "adpcm_xa",
        .long_name = NULL_IF_CONFIG_SMALL("ADPCM CDROM XA"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ADPCM_ADX,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "adpcm_adx",
        .long_name = NULL_IF_CONFIG_SMALL("SEGA CRI ADX ADPCM"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ADPCM_EA,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "adpcm_ea",
        .long_name = NULL_IF_CONFIG_SMALL("ADPCM Electronic Arts"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ADPCM_G726,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "adpcm_g726",
        .long_name = NULL_IF_CONFIG_SMALL("G.726 ADPCM"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ADPCM_CT,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "adpcm_ct",
        .long_name = NULL_IF_CONFIG_SMALL("ADPCM Creative Technology"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ADPCM_SWF,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "adpcm_swf",
        .long_name = NULL_IF_CONFIG_SMALL("ADPCM Shockwave Flash"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ADPCM_YAMAHA,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "adpcm_yamaha",
        .long_name = NULL_IF_CONFIG_SMALL("ADPCM Yamaha"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ADPCM_SBPRO_4,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "adpcm_sbpro_4",
        .long_name = NULL_IF_CONFIG_SMALL("ADPCM Sound Blaster Pro 4-bit"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ADPCM_SBPRO_3,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "adpcm_sbpro_3",
        .long_name = NULL_IF_CONFIG_SMALL("ADPCM Sound Blaster Pro 2.6-bit"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ADPCM_SBPRO_2,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "adpcm_sbpro_2",
        .long_name = NULL_IF_CONFIG_SMALL("ADPCM Sound Blaster Pro 2-bit"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ADPCM_THP,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "adpcm_thp",
        .long_name = NULL_IF_CONFIG_SMALL("ADPCM Nintendo THP"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ADPCM_IMA_AMV,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "adpcm_ima_amv",
        .long_name = NULL_IF_CONFIG_SMALL("ADPCM IMA AMV"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ADPCM_EA_R1,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "adpcm_ea_r1",
        .long_name = NULL_IF_CONFIG_SMALL("ADPCM Electronic Arts R1"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ADPCM_EA_R3,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "adpcm_ea_r3",
        .long_name = NULL_IF_CONFIG_SMALL("ADPCM Electronic Arts R3"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ADPCM_EA_R2,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "adpcm_ea_r2",
        .long_name = NULL_IF_CONFIG_SMALL("ADPCM Electronic Arts R2"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ADPCM_IMA_EA_SEAD,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "adpcm_ima_ea_sead",
        .long_name = NULL_IF_CONFIG_SMALL("ADPCM IMA Electronic Arts SEAD"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ADPCM_IMA_EA_EACS,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "adpcm_ima_ea_eacs",
        .long_name = NULL_IF_CONFIG_SMALL("ADPCM IMA Electronic Arts EACS"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ADPCM_EA_XAS,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "adpcm_ea_xas",
        .long_name = NULL_IF_CONFIG_SMALL("ADPCM Electronic Arts XAS"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ADPCM_EA_MAXIS_XA,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "adpcm_ea_maxis_xa",
        .long_name = NULL_IF_CONFIG_SMALL("ADPCM Electronic Arts Maxis CDROM XA"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ADPCM_IMA_ISS,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "adpcm_ima_iss",
        .long_name = NULL_IF_CONFIG_SMALL("ADPCM IMA Funcom ISS"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ADPCM_G722,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "adpcm_g722",
        .long_name = NULL_IF_CONFIG_SMALL("G.722 ADPCM"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ADPCM_IMA_APC,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "adpcm_ima_apc",
        .long_name = NULL_IF_CONFIG_SMALL("ADPCM IMA CRYO APC"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ADPCM_VIMA,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "adpcm_vima",
        .long_name = NULL_IF_CONFIG_SMALL("LucasArts VIMA audio"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ADPCM_AFC,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "adpcm_afc",
        .long_name = NULL_IF_CONFIG_SMALL("ADPCM Nintendo Gamecube AFC"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ADPCM_IMA_OKI,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "adpcm_ima_oki",
        .long_name = NULL_IF_CONFIG_SMALL("ADPCM IMA Dialogic OKI"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ADPCM_DTK,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "adpcm_dtk",
        .long_name = NULL_IF_CONFIG_SMALL("ADPCM Nintendo Gamecube DTK"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ADPCM_IMA_RAD,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "adpcm_ima_rad",
        .long_name = NULL_IF_CONFIG_SMALL("ADPCM IMA Radical"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ADPCM_G726LE,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "adpcm_g726le",
        .long_name = NULL_IF_CONFIG_SMALL("G.726 ADPCM little-endian"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ADPCM_THP_LE,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "adpcm_thp_le",
        .long_name = NULL_IF_CONFIG_SMALL("ADPCM Nintendo THP (Little-Endian)"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ADPCM_PSX,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "adpcm_psx",
        .long_name = NULL_IF_CONFIG_SMALL("ADPCM Playstation"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ADPCM_AICA,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "adpcm_aica",
        .long_name = NULL_IF_CONFIG_SMALL("ADPCM Yamaha AICA"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ADPCM_IMA_DAT4,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "adpcm_ima_dat4",
        .long_name = NULL_IF_CONFIG_SMALL("ADPCM IMA Eurocom DAT4"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ADPCM_MTAF,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "adpcm_mtaf",
        .long_name = NULL_IF_CONFIG_SMALL("ADPCM MTAF"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ADPCM_AGM,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "adpcm_agm",
        .long_name = NULL_IF_CONFIG_SMALL("ADPCM AmuseGraphics Movie AGM"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ADPCM_ARGO,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "adpcm_argo",
        .long_name = NULL_IF_CONFIG_SMALL("ADPCM Argonaut Games"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ADPCM_IMA_SSI,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "adpcm_ima_ssi",
        .long_name = NULL_IF_CONFIG_SMALL("ADPCM IMA Simon & Schuster Interactive"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ADPCM_ZORK,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "adpcm_zork",
        .long_name = NULL_IF_CONFIG_SMALL("ADPCM Zork"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ADPCM_IMA_APM,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "adpcm_ima_apm",
        .long_name = NULL_IF_CONFIG_SMALL("ADPCM IMA Ubisoft APM"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ADPCM_IMA_ALP,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "adpcm_ima_alp",
        .long_name = NULL_IF_CONFIG_SMALL("ADPCM IMA High Voltage Software ALP"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ADPCM_IMA_MTF,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "adpcm_ima_mtf",
        .long_name = NULL_IF_CONFIG_SMALL("ADPCM IMA Capcom's MT Framework"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ADPCM_IMA_CUNNING,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "adpcm_ima_cunning",
        .long_name = NULL_IF_CONFIG_SMALL("ADPCM IMA Cunning Developments"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ADPCM_IMA_MOFLEX,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "adpcm_ima_moflex",
        .long_name = NULL_IF_CONFIG_SMALL("ADPCM IMA MobiClip MOFLEX"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },

    /* AMR */
    {
        .id        = AV_CODEC_ID_AMR_NB,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "amr_nb",
        .long_name = NULL_IF_CONFIG_SMALL("AMR-NB (Adaptive Multi-Rate NarrowBand)"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_AMR_WB,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "amr_wb",
        .long_name = NULL_IF_CONFIG_SMALL("AMR-WB (Adaptive Multi-Rate WideBand)"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },

    /* RealAudio codecs*/
    {
        .id        = AV_CODEC_ID_RA_144,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "ra_144",
        .long_name = NULL_IF_CONFIG_SMALL("RealAudio 1.0 (14.4K)"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_RA_288,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "ra_288",
        .long_name = NULL_IF_CONFIG_SMALL("RealAudio 2.0 (28.8K)"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },

    /* various DPCM codecs */
    {
        .id        = AV_CODEC_ID_ROQ_DPCM,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "roq_dpcm",
        .long_name = NULL_IF_CONFIG_SMALL("DPCM id RoQ"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_INTERPLAY_DPCM,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "interplay_dpcm",
        .long_name = NULL_IF_CONFIG_SMALL("DPCM Interplay"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_XAN_DPCM,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "xan_dpcm",
        .long_name = NULL_IF_CONFIG_SMALL("DPCM Xan"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_SOL_DPCM,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "sol_dpcm",
        .long_name = NULL_IF_CONFIG_SMALL("DPCM Sol"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_SDX2_DPCM,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "sdx2_dpcm",
        .long_name = NULL_IF_CONFIG_SMALL("DPCM Squareroot-Delta-Exact"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_GREMLIN_DPCM,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "gremlin_dpcm",
        .long_name = NULL_IF_CONFIG_SMALL("DPCM Gremlin"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_DERF_DPCM,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "derf_dpcm",
        .long_name = NULL_IF_CONFIG_SMALL("DPCM Xilam DERF"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },

    /* audio codecs */
    {
        .id        = AV_CODEC_ID_MP2,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "mp2",
        .long_name = NULL_IF_CONFIG_SMALL("MP2 (MPEG audio layer 2)"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_MP3,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "mp3",
        .long_name = NULL_IF_CONFIG_SMALL("MP3 (MPEG audio layer 3)"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_AAC,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "aac",
        .long_name = NULL_IF_CONFIG_SMALL("AAC (Advanced Audio Coding)"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
        .profiles  = NULL_IF_CONFIG_SMALL(ff_aac_profiles),
    },
    {
        .id        = AV_CODEC_ID_AC3,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "ac3",
        .long_name = NULL_IF_CONFIG_SMALL("ATSC A/52A (AC-3)"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_DTS,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "dts",
        .long_name = NULL_IF_CONFIG_SMALL("DCA (DTS Coherent Acoustics)"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY | AV_CODEC_PROP_LOSSLESS,
        .profiles  = NULL_IF_CONFIG_SMALL(ff_dca_profiles),
    },
    {
        .id        = AV_CODEC_ID_VORBIS,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "vorbis",
        .long_name = NULL_IF_CONFIG_SMALL("Vorbis"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_DVAUDIO,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "dvaudio",
        .long_name = NULL_IF_CONFIG_SMALL("DV audio"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_WMAV1,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "wmav1",
        .long_name = NULL_IF_CONFIG_SMALL("Windows Media Audio 1"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_WMAV2,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "wmav2",
        .long_name = NULL_IF_CONFIG_SMALL("Windows Media Audio 2"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_MACE3,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "mace3",
        .long_name = NULL_IF_CONFIG_SMALL("MACE (Macintosh Audio Compression/Expansion) 3:1"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_MACE6,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "mace6",
        .long_name = NULL_IF_CONFIG_SMALL("MACE (Macintosh Audio Compression/Expansion) 6:1"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_VMDAUDIO,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "vmdaudio",
        .long_name = NULL_IF_CONFIG_SMALL("Sierra VMD audio"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_FLAC,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "flac",
        .long_name = NULL_IF_CONFIG_SMALL("FLAC (Free Lossless Audio Codec)"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_MP3ADU,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "mp3adu",
        .long_name = NULL_IF_CONFIG_SMALL("ADU (Application Data Unit) MP3 (MPEG audio layer 3)"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_MP3ON4,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "mp3on4",
        .long_name = NULL_IF_CONFIG_SMALL("MP3onMP4"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_SHORTEN,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "shorten",
        .long_name = NULL_IF_CONFIG_SMALL("Shorten"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_ALAC,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "alac",
        .long_name = NULL_IF_CONFIG_SMALL("ALAC (Apple Lossless Audio Codec)"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_WESTWOOD_SND1,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "westwood_snd1",
        .long_name = NULL_IF_CONFIG_SMALL("Westwood Audio (SND1)"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_GSM,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "gsm",
        .long_name = NULL_IF_CONFIG_SMALL("GSM"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_QDM2,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "qdm2",
        .long_name = NULL_IF_CONFIG_SMALL("QDesign Music Codec 2"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_COOK,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "cook",
        .long_name = NULL_IF_CONFIG_SMALL("Cook / Cooker / Gecko (RealAudio G2)"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_TRUESPEECH,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "truespeech",
        .long_name = NULL_IF_CONFIG_SMALL("DSP Group TrueSpeech"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_TTA,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "tta",
        .long_name = NULL_IF_CONFIG_SMALL("TTA (True Audio)"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_SMACKAUDIO,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "smackaudio",
        .long_name = NULL_IF_CONFIG_SMALL("Smacker audio"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_QCELP,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "qcelp",
        .long_name = NULL_IF_CONFIG_SMALL("QCELP / PureVoice"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_WAVPACK,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "wavpack",
        .long_name = NULL_IF_CONFIG_SMALL("WavPack"),
        .props     = AV_CODEC_PROP_INTRA_ONLY |
                     AV_CODEC_PROP_LOSSY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_DSICINAUDIO,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "dsicinaudio",
        .long_name = NULL_IF_CONFIG_SMALL("Delphine Software International CIN audio"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_IMC,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "imc",
        .long_name = NULL_IF_CONFIG_SMALL("IMC (Intel Music Coder)"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_MUSEPACK7,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "musepack7",
        .long_name = NULL_IF_CONFIG_SMALL("Musepack SV7"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_MLP,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "mlp",
        .long_name = NULL_IF_CONFIG_SMALL("MLP (Meridian Lossless Packing)"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_GSM_MS,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "gsm_ms",
        .long_name = NULL_IF_CONFIG_SMALL("GSM Microsoft variant"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ATRAC3,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "atrac3",
        .long_name = NULL_IF_CONFIG_SMALL("ATRAC3 (Adaptive TRansform Acoustic Coding 3)"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_APE,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "ape",
        .long_name = NULL_IF_CONFIG_SMALL("Monkey's Audio"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_NELLYMOSER,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "nellymoser",
        .long_name = NULL_IF_CONFIG_SMALL("Nellymoser Asao"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_MUSEPACK8,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "musepack8",
        .long_name = NULL_IF_CONFIG_SMALL("Musepack SV8"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_SPEEX,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "speex",
        .long_name = NULL_IF_CONFIG_SMALL("Speex"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_WMAVOICE,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "wmavoice",
        .long_name = NULL_IF_CONFIG_SMALL("Windows Media Audio Voice"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_WMAPRO,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "wmapro",
        .long_name = NULL_IF_CONFIG_SMALL("Windows Media Audio 9 Professional"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_WMALOSSLESS,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "wmalossless",
        .long_name = NULL_IF_CONFIG_SMALL("Windows Media Audio Lossless"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_ATRAC3P,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "atrac3p",
        .long_name = NULL_IF_CONFIG_SMALL("ATRAC3+ (Adaptive TRansform Acoustic Coding 3+)"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_EAC3,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "eac3",
        .long_name = NULL_IF_CONFIG_SMALL("ATSC A/52B (AC-3, E-AC-3)"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_SIPR,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "sipr",
        .long_name = NULL_IF_CONFIG_SMALL("RealAudio SIPR / ACELP.NET"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_MP1,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "mp1",
        .long_name = NULL_IF_CONFIG_SMALL("MP1 (MPEG audio layer 1)"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_TWINVQ,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "twinvq",
        .long_name = NULL_IF_CONFIG_SMALL("VQF TwinVQ"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_TRUEHD,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "truehd",
        .long_name = NULL_IF_CONFIG_SMALL("TrueHD"),
        .props     = AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_MP4ALS,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "mp4als",
        .long_name = NULL_IF_CONFIG_SMALL("MPEG-4 Audio Lossless Coding (ALS)"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_ATRAC1,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "atrac1",
        .long_name = NULL_IF_CONFIG_SMALL("ATRAC1 (Adaptive TRansform Acoustic Coding)"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_BINKAUDIO_RDFT,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "binkaudio_rdft",
        .long_name = NULL_IF_CONFIG_SMALL("Bink Audio (RDFT)"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_BINKAUDIO_DCT,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "binkaudio_dct",
        .long_name = NULL_IF_CONFIG_SMALL("Bink Audio (DCT)"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_AAC_LATM,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "aac_latm",
        .long_name = NULL_IF_CONFIG_SMALL("AAC LATM (Advanced Audio Coding LATM syntax)"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
        .profiles  = NULL_IF_CONFIG_SMALL(ff_aac_profiles),
    },
    {
        .id        = AV_CODEC_ID_QDMC,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "qdmc",
        .long_name = NULL_IF_CONFIG_SMALL("QDesign Music"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_CELT,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "celt",
        .long_name = NULL_IF_CONFIG_SMALL("Constrained Energy Lapped Transform (CELT)"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_G723_1,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "g723_1",
        .long_name = NULL_IF_CONFIG_SMALL("G.723.1"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_G729,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "g729",
        .long_name = NULL_IF_CONFIG_SMALL("G.729"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_8SVX_EXP,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "8svx_exp",
        .long_name = NULL_IF_CONFIG_SMALL("8SVX exponential"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_8SVX_FIB,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "8svx_fib",
        .long_name = NULL_IF_CONFIG_SMALL("8SVX fibonacci"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_BMV_AUDIO,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "bmv_audio",
        .long_name = NULL_IF_CONFIG_SMALL("Discworld II BMV audio"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_RALF,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "ralf",
        .long_name = NULL_IF_CONFIG_SMALL("RealAudio Lossless"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_IAC,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "iac",
        .long_name = NULL_IF_CONFIG_SMALL("IAC (Indeo Audio Coder)"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ILBC,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "ilbc",
        .long_name = NULL_IF_CONFIG_SMALL("iLBC (Internet Low Bitrate Codec)"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_OPUS,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "opus",
        .long_name = NULL_IF_CONFIG_SMALL("Opus (Opus Interactive Audio Codec)"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_COMFORT_NOISE,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "comfortnoise",
        .long_name = NULL_IF_CONFIG_SMALL("RFC 3389 Comfort Noise"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_TAK,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "tak",
        .long_name = NULL_IF_CONFIG_SMALL("TAK (Tom's lossless Audio Kompressor)"),
        .props     = AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_METASOUND,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "metasound",
        .long_name = NULL_IF_CONFIG_SMALL("Voxware MetaSound"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_PAF_AUDIO,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "paf_audio",
        .long_name = NULL_IF_CONFIG_SMALL("Amazing Studio Packed Animation File Audio"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ON2AVC,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "avc",
        .long_name = NULL_IF_CONFIG_SMALL("On2 Audio for Video Codec"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_DSS_SP,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "dss_sp",
        .long_name = NULL_IF_CONFIG_SMALL("Digital Speech Standard - Standard Play mode (DSS SP)"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_CODEC2,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "codec2",
        .long_name = NULL_IF_CONFIG_SMALL("codec2 (very low bitrate speech codec)"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_FFWAVESYNTH,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "wavesynth",
        .long_name = NULL_IF_CONFIG_SMALL("Wave synthesis pseudo-codec"),
        .props     = AV_CODEC_PROP_INTRA_ONLY,
    },
    {
        .id        = AV_CODEC_ID_SONIC,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "sonic",
        .long_name = NULL_IF_CONFIG_SMALL("Sonic"),
        .props     = AV_CODEC_PROP_INTRA_ONLY,
    },
    {
        .id        = AV_CODEC_ID_SONIC_LS,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "sonicls",
        .long_name = NULL_IF_CONFIG_SMALL("Sonic lossless"),
        .props     = AV_CODEC_PROP_INTRA_ONLY,
    },
    {
        .id        = AV_CODEC_ID_EVRC,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "evrc",
        .long_name = NULL_IF_CONFIG_SMALL("EVRC (Enhanced Variable Rate Codec)"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_SMV,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "smv",
        .long_name = NULL_IF_CONFIG_SMALL("SMV (Selectable Mode Vocoder)"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_DSD_LSBF,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "dsd_lsbf",
        .long_name = NULL_IF_CONFIG_SMALL("DSD (Direct Stream Digital), least significant bit first"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_DSD_MSBF,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "dsd_msbf",
        .long_name = NULL_IF_CONFIG_SMALL("DSD (Direct Stream Digital), most significant bit first"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_DSD_LSBF_PLANAR,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "dsd_lsbf_planar",
        .long_name = NULL_IF_CONFIG_SMALL("DSD (Direct Stream Digital), least significant bit first, planar"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_DSD_MSBF_PLANAR,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "dsd_msbf_planar",
        .long_name = NULL_IF_CONFIG_SMALL("DSD (Direct Stream Digital), most significant bit first, planar"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_4GV,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "4gv",
        .long_name = NULL_IF_CONFIG_SMALL("4GV (Fourth Generation Vocoder)"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_INTERPLAY_ACM,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "interplayacm",
        .long_name = NULL_IF_CONFIG_SMALL("Interplay ACM"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_XMA1,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "xma1",
        .long_name = NULL_IF_CONFIG_SMALL("Xbox Media Audio 1"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_XMA2,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "xma2",
        .long_name = NULL_IF_CONFIG_SMALL("Xbox Media Audio 2"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_DST,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "dst",
        .long_name = NULL_IF_CONFIG_SMALL("DST (Direct Stream Transfer)"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_ATRAC3AL,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "atrac3al",
        .long_name = NULL_IF_CONFIG_SMALL("ATRAC3 AL (Adaptive TRansform Acoustic Coding 3 Advanced Lossless)"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_ATRAC3PAL,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "atrac3pal",
        .long_name = NULL_IF_CONFIG_SMALL("ATRAC3+ AL (Adaptive TRansform Acoustic Coding 3+ Advanced Lossless)"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS,
    },
    {
        .id        = AV_CODEC_ID_DOLBY_E,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "dolby_e",
        .long_name = NULL_IF_CONFIG_SMALL("Dolby E"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_APTX,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "aptx",
        .long_name = NULL_IF_CONFIG_SMALL("aptX (Audio Processing Technology for Bluetooth)"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_APTX_HD,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "aptx_hd",
        .long_name = NULL_IF_CONFIG_SMALL("aptX HD (Audio Processing Technology for Bluetooth)"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_SBC,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "sbc",
        .long_name = NULL_IF_CONFIG_SMALL("SBC (low-complexity subband codec)"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ATRAC9,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "atrac9",
        .long_name = NULL_IF_CONFIG_SMALL("ATRAC9 (Adaptive TRansform Acoustic Coding 9)"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_HCOM,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "hcom",
        .long_name = NULL_IF_CONFIG_SMALL("HCOM Audio"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_ACELP_KELVIN,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "acelp.kelvin",
        .long_name = NULL_IF_CONFIG_SMALL("Sipro ACELP.KELVIN"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_MPEGH_3D_AUDIO,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "mpegh_3d_audio",
        .long_name = NULL_IF_CONFIG_SMALL("MPEG-H 3D Audio"),
        .props     = AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_SIREN,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "siren",
        .long_name = NULL_IF_CONFIG_SMALL("Siren"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_HCA,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "hca",
        .long_name = NULL_IF_CONFIG_SMALL("CRI HCA"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },
    {
        .id        = AV_CODEC_ID_FASTAUDIO,
        .type      = AVMEDIA_TYPE_AUDIO,
        .name      = "fastaudio",
        .long_name = NULL_IF_CONFIG_SMALL("MobiClip FastAudio"),
        .props     = AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY,
    },

    /* subtitle codecs */
    {
        .id        = AV_CODEC_ID_DVD_SUBTITLE,
        .type      = AVMEDIA_TYPE_SUBTITLE,
        .name      = "dvd_subtitle",
        .long_name = NULL_IF_CONFIG_SMALL("DVD subtitles"),
        .props     = AV_CODEC_PROP_BITMAP_SUB,
    },
    {
        .id        = AV_CODEC_ID_DVB_SUBTITLE,
        .type      = AVMEDIA_TYPE_SUBTITLE,
        .name      = "dvb_subtitle",
        .long_name = NULL_IF_CONFIG_SMALL("DVB subtitles"),
        .props     = AV_CODEC_PROP_BITMAP_SUB,
    },
    {
        .id        = AV_CODEC_ID_TEXT,
        .type      = AVMEDIA_TYPE_SUBTITLE,
        .name      = "text",
        .long_name = NULL_IF_CONFIG_SMALL("raw UTF-8 text"),
        .props     = AV_CODEC_PROP_TEXT_SUB,
    },
    {
        .id        = AV_CODEC_ID_XSUB,
        .type      = AVMEDIA_TYPE_SUBTITLE,
        .name      = "xsub",
        .long_name = NULL_IF_CONFIG_SMALL("XSUB"),
        .props     = AV_CODEC_PROP_BITMAP_SUB,
    },
    {
        .id        = AV_CODEC_ID_SSA,
        .type      = AVMEDIA_TYPE_SUBTITLE,
        .name      = "ssa",
        .long_name = NULL_IF_CONFIG_SMALL("SSA (SubStation Alpha) subtitle"),
        .props     = AV_CODEC_PROP_TEXT_SUB,
    },
    {
        .id        = AV_CODEC_ID_MOV_TEXT,
        .type      = AVMEDIA_TYPE_SUBTITLE,
        .name      = "mov_text",
        .long_name = NULL_IF_CONFIG_SMALL("MOV text"),
        .props     = AV_CODEC_PROP_TEXT_SUB,
    },
    {
        .id        = AV_CODEC_ID_HDMV_PGS_SUBTITLE,
        .type      = AVMEDIA_TYPE_SUBTITLE,
        .name      = "hdmv_pgs_subtitle",
        .long_name = NULL_IF_CONFIG_SMALL("HDMV Presentation Graphic Stream subtitles"),
        .props     = AV_CODEC_PROP_BITMAP_SUB,
    },
    {
        .id        = AV_CODEC_ID_DVB_TELETEXT,
        .type      = AVMEDIA_TYPE_SUBTITLE,
        .name      = "dvb_teletext",
        .long_name = NULL_IF_CONFIG_SMALL("DVB teletext"),
    },
    {
        .id        = AV_CODEC_ID_SRT,
        .type      = AVMEDIA_TYPE_SUBTITLE,
        .name      = "srt",
        .long_name = NULL_IF_CONFIG_SMALL("SubRip subtitle with embedded timing"),
        .props     = AV_CODEC_PROP_TEXT_SUB,
    },
    {
        .id        = AV_CODEC_ID_MICRODVD,
        .type      = AVMEDIA_TYPE_SUBTITLE,
        .name      = "microdvd",
        .long_name = NULL_IF_CONFIG_SMALL("MicroDVD subtitle"),
        .props     = AV_CODEC_PROP_TEXT_SUB,
    },
    {
        .id        = AV_CODEC_ID_EIA_608,
        .type      = AVMEDIA_TYPE_SUBTITLE,
        .name      = "eia_608",
        .long_name = NULL_IF_CONFIG_SMALL("EIA-608 closed captions"),
        .props     = AV_CODEC_PROP_TEXT_SUB,
    },
    {
        .id        = AV_CODEC_ID_JACOSUB,
        .type      = AVMEDIA_TYPE_SUBTITLE,
        .name      = "jacosub",
        .long_name = NULL_IF_CONFIG_SMALL("JACOsub subtitle"),
        .props     = AV_CODEC_PROP_TEXT_SUB,
    },
    {
        .id        = AV_CODEC_ID_SAMI,
        .type      = AVMEDIA_TYPE_SUBTITLE,
        .name      = "sami",
        .long_name = NULL_IF_CONFIG_SMALL("SAMI subtitle"),
        .props     = AV_CODEC_PROP_TEXT_SUB,
    },
    {
        .id        = AV_CODEC_ID_REALTEXT,
        .type      = AVMEDIA_TYPE_SUBTITLE,
        .name      = "realtext",
        .long_name = NULL_IF_CONFIG_SMALL("RealText subtitle"),
        .props     = AV_CODEC_PROP_TEXT_SUB,
    },
    {
        .id        = AV_CODEC_ID_STL,
        .type      = AVMEDIA_TYPE_SUBTITLE,
        .name      = "stl",
        .long_name = NULL_IF_CONFIG_SMALL("Spruce subtitle format"),
        .props     = AV_CODEC_PROP_TEXT_SUB,
    },
    {
        .id        = AV_CODEC_ID_SUBVIEWER1,
        .type      = AVMEDIA_TYPE_SUBTITLE,
        .name      = "subviewer1",
        .long_name = NULL_IF_CONFIG_SMALL("SubViewer v1 subtitle"),
        .props     = AV_CODEC_PROP_TEXT_SUB,
    },
    {
        .id        = AV_CODEC_ID_SUBVIEWER,
        .type      = AVMEDIA_TYPE_SUBTITLE,
        .name      = "subviewer",
        .long_name = NULL_IF_CONFIG_SMALL("SubViewer subtitle"),
        .props     = AV_CODEC_PROP_TEXT_SUB,
    },
    {
        .id        = AV_CODEC_ID_SUBRIP,
        .type      = AVMEDIA_TYPE_SUBTITLE,
        .name      = "subrip",
        .long_name = NULL_IF_CONFIG_SMALL("SubRip subtitle"),
        .props     = AV_CODEC_PROP_TEXT_SUB,
    },
    {
        .id        = AV_CODEC_ID_WEBVTT,
        .type      = AVMEDIA_TYPE_SUBTITLE,
        .name      = "webvtt",
        .long_name = NULL_IF_CONFIG_SMALL("WebVTT subtitle"),
        .props     = AV_CODEC_PROP_TEXT_SUB,
    },
    {
        .id        = AV_CODEC_ID_MPL2,
        .type      = AVMEDIA_TYPE_SUBTITLE,
        .name      = "mpl2",
        .long_name = NULL_IF_CONFIG_SMALL("MPL2 subtitle"),
        .props     = AV_CODEC_PROP_TEXT_SUB,
    },
    {
        .id        = AV_CODEC_ID_VPLAYER,
        .type      = AVMEDIA_TYPE_SUBTITLE,
        .name      = "vplayer",
        .long_name = NULL_IF_CONFIG_SMALL("VPlayer subtitle"),
        .props     = AV_CODEC_PROP_TEXT_SUB,
    },
    {
        .id        = AV_CODEC_ID_PJS,
        .type      = AVMEDIA_TYPE_SUBTITLE,
        .name      = "pjs",
        .long_name = NULL_IF_CONFIG_SMALL("PJS (Phoenix Japanimation Society) subtitle"),
        .props     = AV_CODEC_PROP_TEXT_SUB,
    },
    {
        .id        = AV_CODEC_ID_ASS,
        .type      = AVMEDIA_TYPE_SUBTITLE,
        .name      = "ass",
        .long_name = NULL_IF_CONFIG_SMALL("ASS (Advanced SSA) subtitle"),
        .props     = AV_CODEC_PROP_TEXT_SUB,
    },
    {
        .id        = AV_CODEC_ID_HDMV_TEXT_SUBTITLE,
        .type      = AVMEDIA_TYPE_SUBTITLE,
        .name      = "hdmv_text_subtitle",
        .long_name = NULL_IF_CONFIG_SMALL("HDMV Text subtitle"),
        .props     = AV_CODEC_PROP_TEXT_SUB,
    },
    {
        .id        = AV_CODEC_ID_TTML,
        .type      = AVMEDIA_TYPE_SUBTITLE,
        .name      = "ttml",
        .long_name = NULL_IF_CONFIG_SMALL("Timed Text Markup Language"),
        .props     = AV_CODEC_PROP_TEXT_SUB,
    },
    {
        .id        = AV_CODEC_ID_ARIB_CAPTION,
        .type      = AVMEDIA_TYPE_SUBTITLE,
        .name      = "arib_caption",
        .long_name = NULL_IF_CONFIG_SMALL("ARIB STD-B24 caption"),
        .props     = AV_CODEC_PROP_TEXT_SUB,
        .profiles  = NULL_IF_CONFIG_SMALL(ff_arib_caption_profiles),
    },

    /* other kind of codecs and pseudo-codecs */
    {
        .id        = AV_CODEC_ID_TTF,
        .type      = AVMEDIA_TYPE_DATA,
        .name      = "ttf",
        .long_name = NULL_IF_CONFIG_SMALL("TrueType font"),
        .mime_types= MT("application/x-truetype-font", "application/x-font"),
    },
    {
        .id        = AV_CODEC_ID_SCTE_35,
        .type      = AVMEDIA_TYPE_DATA,
        .name      = "scte_35",
        .long_name = NULL_IF_CONFIG_SMALL("SCTE 35 Message Queue"),
    },
    {
        .id        = AV_CODEC_ID_EPG,
        .type      = AVMEDIA_TYPE_DATA,
        .name      = "epg",
        .long_name = NULL_IF_CONFIG_SMALL("Electronic Program Guide"),
    },
    {
        .id        = AV_CODEC_ID_BINTEXT,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "bintext",
        .long_name = NULL_IF_CONFIG_SMALL("Binary text"),
        .props     = AV_CODEC_PROP_INTRA_ONLY,
    },
    {
        .id        = AV_CODEC_ID_XBIN,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "xbin",
        .long_name = NULL_IF_CONFIG_SMALL("eXtended BINary text"),
        .props     = AV_CODEC_PROP_INTRA_ONLY,
    },
    {
        .id        = AV_CODEC_ID_IDF,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "idf",
        .long_name = NULL_IF_CONFIG_SMALL("iCEDraw text"),
        .props     = AV_CODEC_PROP_INTRA_ONLY,
    },
    {
        .id        = AV_CODEC_ID_OTF,
        .type      = AVMEDIA_TYPE_DATA,
        .name      = "otf",
        .long_name = NULL_IF_CONFIG_SMALL("OpenType font"),
        .mime_types= MT("application/vnd.ms-opentype"),
    },
    {
        .id        = AV_CODEC_ID_SMPTE_KLV,
        .type      = AVMEDIA_TYPE_DATA,
        .name      = "klv",
        .long_name = NULL_IF_CONFIG_SMALL("SMPTE 336M Key-Length-Value (KLV) metadata"),
    },
    {
        .id        = AV_CODEC_ID_DVD_NAV,
        .type      = AVMEDIA_TYPE_DATA,
        .name      = "dvd_nav_packet",
        .long_name = NULL_IF_CONFIG_SMALL("DVD Nav packet"),
    },
    {
        .id        = AV_CODEC_ID_TIMED_ID3,
        .type      = AVMEDIA_TYPE_DATA,
        .name      = "timed_id3",
        .long_name = NULL_IF_CONFIG_SMALL("timed ID3 metadata"),
    },
    {
        .id        = AV_CODEC_ID_BIN_DATA,
        .type      = AVMEDIA_TYPE_DATA,
        .name      = "bin_data",
        .long_name = NULL_IF_CONFIG_SMALL("binary data"),
        .mime_types= MT("application/octet-stream"),
    },
    {
        .id        = AV_CODEC_ID_WRAPPED_AVFRAME,
        .type      = AVMEDIA_TYPE_VIDEO,
        .name      = "wrapped_avframe",
        .long_name = NULL_IF_CONFIG_SMALL("AVFrame to AVPacket passthrough"),
        .props     = AV_CODEC_PROP_LOSSLESS,
    },
};


void (*rgb32tobgr24)(const uint8_t *src, uint8_t *dst, int src_size);
void (*rgb32tobgr16)(const uint8_t *src, uint8_t *dst, int src_size);
void (*rgb32tobgr15)(const uint8_t *src, uint8_t *dst, int src_size);
void (*rgb24tobgr32)(const uint8_t *src, uint8_t *dst, int src_size);
void (*rgb24tobgr24)(const uint8_t *src, uint8_t *dst, int src_size);
void (*rgb24tobgr16)(const uint8_t *src, uint8_t *dst, int src_size);
void (*rgb24tobgr15)(const uint8_t *src, uint8_t *dst, int src_size);
void (*rgb16tobgr24)(const uint8_t *src, uint8_t *dst, int src_size);
void (*rgb15tobgr24)(const uint8_t *src, uint8_t *dst, int src_size);

void (*rgb32to16)(const uint8_t *src, uint8_t *dst, int src_size);
void (*rgb32to15)(const uint8_t *src, uint8_t *dst, int src_size);
void (*rgb24to16)(const uint8_t *src, uint8_t *dst, int src_size);
void (*rgb24to15)(const uint8_t *src, uint8_t *dst, int src_size);
void (*rgb16to32)(const uint8_t *src, uint8_t *dst, int src_size);
void (*rgb16to15)(const uint8_t *src, uint8_t *dst, int src_size);
void (*rgb15to16)(const uint8_t *src, uint8_t *dst, int src_size);
void (*rgb15to32)(const uint8_t *src, uint8_t *dst, int src_size);

void (*shuffle_bytes_0321)(const uint8_t *src, uint8_t *dst, int src_size);
void (*shuffle_bytes_2103)(const uint8_t *src, uint8_t *dst, int src_size);
void (*shuffle_bytes_1230)(const uint8_t *src, uint8_t *dst, int src_size);
void (*shuffle_bytes_3012)(const uint8_t *src, uint8_t *dst, int src_size);
void (*shuffle_bytes_3210)(const uint8_t *src, uint8_t *dst, int src_size);


void (*yv12toyuy2)(const uint8_t *ysrc, const uint8_t *usrc,
                   const uint8_t *vsrc, uint8_t *dst,
                   int width, int height,
                   int lumStride, int chromStride, int dstStride);
void (*yv12touyvy)(const uint8_t *ysrc, const uint8_t *usrc,
                   const uint8_t *vsrc, uint8_t *dst,
                   int width, int height,
                   int lumStride, int chromStride, int dstStride);
void (*yuv422ptoyuy2)(const uint8_t *ysrc, const uint8_t *usrc,
                      const uint8_t *vsrc, uint8_t *dst,
                      int width, int height,
                      int lumStride, int chromStride, int dstStride);
void (*yuv422ptouyvy)(const uint8_t *ysrc, const uint8_t *usrc,
                      const uint8_t *vsrc, uint8_t *dst,
                      int width, int height,
                      int lumStride, int chromStride, int dstStride);
void (*yuy2toyv12)(const uint8_t *src, uint8_t *ydst,
                   uint8_t *udst, uint8_t *vdst,
                   int width, int height,
                   int lumStride, int chromStride, int srcStride);
void (*ff_rgb24toyv12)(const uint8_t *src, uint8_t *ydst,
                       uint8_t *udst, uint8_t *vdst,
                       int width, int height,
                       int lumStride, int chromStride, int srcStride,
                       int32_t *rgb2yuv);
void (*planar2x)(const uint8_t *src, uint8_t *dst, int width, int height,
                 int srcStride, int dstStride);
void (*interleaveBytes)(const uint8_t *src1, const uint8_t *src2, uint8_t *dst,
                        int width, int height, int src1Stride,
                        int src2Stride, int dstStride);
void (*deinterleaveBytes)(const uint8_t *src, uint8_t *dst1, uint8_t *dst2,
                          int width, int height, int srcStride,
                          int dst1Stride, int dst2Stride);
void (*vu9_to_vu12)(const uint8_t *src1, const uint8_t *src2,
                    uint8_t *dst1, uint8_t *dst2,
                    int width, int height,
                    int srcStride1, int srcStride2,
                    int dstStride1, int dstStride2);
void (*yvu9_to_yuy2)(const uint8_t *src1, const uint8_t *src2,
                     const uint8_t *src3, uint8_t *dst,
                     int width, int height,
                     int srcStride1, int srcStride2,
                     int srcStride3, int dstStride);
void (*uyvytoyuv420)(uint8_t *ydst, uint8_t *udst, uint8_t *vdst,
                     const uint8_t *src, int width, int height,
                     int lumStride, int chromStride, int srcStride);
void (*uyvytoyuv422)(uint8_t *ydst, uint8_t *udst, uint8_t *vdst,
                     const uint8_t *src, int width, int height,
                     int lumStride, int chromStride, int srcStride);
void (*yuyvtoyuv420)(uint8_t *ydst, uint8_t *udst, uint8_t *vdst,
                     const uint8_t *src, int width, int height,
                     int lumStride, int chromStride, int srcStride);
void (*yuyvtoyuv422)(uint8_t *ydst, uint8_t *udst, uint8_t *vdst,
                     const uint8_t *src, int width, int height,
                     int lumStride, int chromStride, int srcStride);

static int get_second_size(char *codec_name);
static int aa_read_header(AVFormatContext *s);
static int aa_read_packet(AVFormatContext *s, AVPacket *pkt);
static int aa_read_seek(AVFormatContext *s,int stream_index, int64_t timestamp, int flags);
static int aa_probe(const AVProbeData *p);
static int aa_read_close(AVFormatContext *s);
static const AVOption aa_options[];
static const AVClass aa_class;
static int adts_aac_probe(const AVProbeData *p);
static int adts_aac_resync(AVFormatContext *s);
static int adts_aac_read_header(AVFormatContext *s);
static int handle_id3(AVFormatContext *s, AVPacket *pkt);
static int adts_aac_read_packet(AVFormatContext *s, AVPacket *pkt);
static int aax_probe(const AVProbeData *p);
static int64_t get_pts(AVFormatContext *s, int64_t pos, int size);
static int aax_read_header(AVFormatContext *s);
static int aax_read_packet(AVFormatContext *s, AVPacket *pkt);
static int aax_read_close(AVFormatContext *s);



static int write_ctoc(AVFormatContext *s, ID3v2EncContext *id3, int enc);
static int write_chapter(AVFormatContext *s, ID3v2EncContext *id3, int id, int enc);
static int write_metadata(AVIOContext *pb, AVDictionary **metadata,ID3v2EncContext *id3, int enc);
void ff_id3v2_start(ID3v2EncContext *id3, AVIOContext *pb, int id3v2_version,const char *magic);
void ff_id3v2_finish(ID3v2EncContext *id3, AVIOContext *pb,int padding_bytes);
int ff_id3v2_write_simple(struct AVFormatContext *s, int id3v2_version,const char *magic);
int ff_id3v2_write_metadata(AVFormatContext *s, ID3v2EncContext *id3);
int avpriv_mpeg4audio_get_config2(MPEG4AudioConfig *c, const uint8_t *buf,int size, int sync_extension, void *logctx);

static int a64_write_header(AVFormatContext *s);
int ff_raw_write_packet(AVFormatContext *s, AVPacket *pkt);
static int force_one_stream(AVFormatContext *s);



int ff_standardize_creation_time(AVFormatContext *s);

static inline int init_get_bits_xe(GetBitContext *s, const uint8_t *buffer,int bit_size, int is_le)
{
    int buffer_size;
    int ret = 0;

    if (bit_size >= INT_MAX - FFMAX(7, AV_INPUT_BUFFER_PADDING_SIZE * 8) || bit_size < 0 || !buffer)
    {
        bit_size = 0;
        buffer = NULL;
        ret = AVERROR_INVALIDDATA;
    }

    buffer_size = (bit_size + 7) >> 3;

    s->buffer = buffer;
    s->size_in_bits = bit_size;
    s->size_in_bits_plus8 = bit_size + 8;
    s->buffer_end = buffer + buffer_size;
    s->index = 0;

#if CACHED_BITSTREAM_READER
    s->cache = 0;
    s->bits_left = 0;
    refill_64(s, is_le);
#endif

    return ret;
}

static inline int init_get_bits(GetBitContext *s, const uint8_t *buffer,
                                int bit_size)
{
#ifdef BITSTREAM_READER_LE
    return init_get_bits_xe(s, buffer, bit_size, 1);
#else
    return init_get_bits_xe(s, buffer, bit_size, 0);
#endif
}

static inline void skip_bits(GetBitContext *s, int n)
{
#if CACHED_BITSTREAM_READER
    if (n < s->bits_left)
        skip_remaining(s, n);
    else
    {
        n -= s->bits_left;
        s->cache = 0;
        s->bits_left = 0;

        if (n >= 64)
        {
            unsigned skip = (n / 8) * 8;

            n -= skip;
            s->index += skip;
        }
#ifdef BITSTREAM_READER_LE
        refill_64(s, 1);
#else
        refill_64(s, 0);
#endif
        if (n)
            skip_remaining(s, n);
    }
#else
    OPEN_READER(re, s);
    LAST_SKIP_BITS(re, s, n);
    CLOSE_READER(re, s);
#endif
}

static inline void skip_bits_long(GetBitContext *s, int n)
{
#if CACHED_BITSTREAM_READER
    skip_bits(s, n);
#else
#if UNCHECKED_BITSTREAM_READER
    s->index += n;
#else
    s->index += av_clip(n, -s->index, s->size_in_bits_plus8 - s->index);
#endif
#endif
}

static av_always_inline av_const int av_clip_c(int a, int amin, int amax)
{
#if defined(HAVE_AV_CONFIG_H) && defined(ASSERT_LEVEL) && ASSERT_LEVEL >= 2
    if (amin > amax)
        abort();
#endif
    if (a < amin)
        return amin;
    else if (a > amax)
        return amax;
    else
        return a;
}

#ifndef av_clip
#define av_clip av_clip_c
#endif

static inline unsigned int get_bits(GetBitContext *s, int n)
{
    register unsigned int tmp;
#if CACHED_BITSTREAM_READER

    av_assert2(n > 0 && n <= 32);
    if (n > s->bits_left)
    {
#ifdef BITSTREAM_READER_LE
        refill_32(s, 1);
#else
        refill_32(s, 0);
#endif
        if (s->bits_left < 32)
            s->bits_left = n;
    }

#ifdef BITSTREAM_READER_LE
    tmp = get_val(s, n, 1);
#else
    tmp = get_val(s, n, 0);
#endif
#else
    OPEN_READER(re, s);
    av_assert2(n > 0 && n <= 25);
    UPDATE_CACHE(re, s);
    tmp = SHOW_UBITS(re, s, n);
    LAST_SKIP_BITS(re, s, n);
    CLOSE_READER(re, s);
#endif
    av_assert2(tmp < UINT64_C(1) << n);
    return tmp;
}

static const int BUF_BITS = 8 * sizeof(BitBuf);


static inline void init_put_bits(PutBitContext *s, uint8_t *buffer,
                                 int buffer_size)
{
    if (buffer_size < 0)
    {
        buffer_size = 0;
        buffer = NULL;
    }

    s->size_in_bits = 8 * buffer_size;
    s->buf = buffer;
    s->buf_end = s->buf + buffer_size;
    s->buf_ptr = s->buf;
    s->bit_left = BUF_BITS;
    s->bit_buf = 0;
}

static inline void put_bits_no_assert(PutBitContext *s, int n, BitBuf value)
{
    BitBuf bit_buf;
    int bit_left;

    bit_buf = s->bit_buf;
    bit_left = s->bit_left;

    /* XXX: optimize */
#ifdef BITSTREAM_WRITER_LE
    bit_buf |= value << (BUF_BITS - bit_left);
    if (n >= bit_left)
    {
        if (s->buf_end - s->buf_ptr >= sizeof(BitBuf))
        {
            AV_WLBUF(s->buf_ptr, bit_buf);
            s->buf_ptr += sizeof(BitBuf);
        }
        else
        {
            av_log(NULL, AV_LOG_ERROR, "Internal error, put_bits buffer too small\n");
            av_assert2(0);
        }
        bit_buf = value >> bit_left;
        bit_left += BUF_BITS;
    }
    bit_left -= n;
#else
    if (n < bit_left)
    {
        bit_buf = (bit_buf << n) | value;
        bit_left -= n;
    }
    else
    {
        bit_buf <<= bit_left;
        bit_buf |= value >> (n - bit_left);
        if (s->buf_end - s->buf_ptr >= sizeof(BitBuf))
        {
            AV_WBBUF(s->buf_ptr, bit_buf);
            s->buf_ptr += sizeof(BitBuf);
        }
        else
        {
            av_log(NULL, AV_LOG_ERROR, "Internal error, put_bits buffer too small\n");
            av_assert2(0);
        }
        bit_left += BUF_BITS - n;
        bit_buf = value;
    }
#endif

    s->bit_buf = bit_buf;
    s->bit_left = bit_left;
}

static inline void put_bits(PutBitContext *s, int n, BitBuf value)
{
    av_assert2(n <= 31 && value < (1UL << n));
    put_bits_no_assert(s, n, value);
}

static inline int put_bits_count(PutBitContext *s)
{
    return (s->buf_ptr - s->buf) * 8 + BUF_BITS - s->bit_left;
}

static av_always_inline unsigned int ff_pce_copy_bits(PutBitContext *pb,
                                                      GetBitContext *gb,
                                                      int bits)
{
    unsigned int el = get_bits(gb, bits);
    put_bits(pb, bits, el);
    return el;
}


static inline int get_bits_count(const GetBitContext *s)
{
#if CACHED_BITSTREAM_READER
    return s->index - s->bits_left;
#else
    return s->index;
#endif
}

static inline const uint8_t *align_get_bits(GetBitContext *s)
{
    int n = -get_bits_count(s) & 7;
    if (n)
        skip_bits(s, n);
    return s->buffer + (s->index >> 3);
}

static inline int ff_copy_pce_data(PutBitContext *pb, GetBitContext *gb)
{
    int five_bit_ch, four_bit_ch, comment_size, bits;
    int offset = put_bits_count(pb);

    ff_pce_copy_bits(pb, gb, 10);               // Tag, Object Type, Frequency
    five_bit_ch = ff_pce_copy_bits(pb, gb, 4);  // Front
    five_bit_ch += ff_pce_copy_bits(pb, gb, 4); // Side
    five_bit_ch += ff_pce_copy_bits(pb, gb, 4); // Back
    four_bit_ch = ff_pce_copy_bits(pb, gb, 2);  // LFE
    four_bit_ch += ff_pce_copy_bits(pb, gb, 3); // Data
    five_bit_ch += ff_pce_copy_bits(pb, gb, 4); // Coupling
    if (ff_pce_copy_bits(pb, gb, 1))            // Mono Mixdown
        ff_pce_copy_bits(pb, gb, 4);
    if (ff_pce_copy_bits(pb, gb, 1)) // Stereo Mixdown
        ff_pce_copy_bits(pb, gb, 4);
    if (ff_pce_copy_bits(pb, gb, 1)) // Matrix Mixdown
        ff_pce_copy_bits(pb, gb, 3);
    for (bits = five_bit_ch * 5 + four_bit_ch * 4; bits > 16; bits -= 16)
        ff_pce_copy_bits(pb, gb, 16);
    if (bits)
        ff_pce_copy_bits(pb, gb, bits);
    avpriv_align_put_bits(pb);
    align_get_bits(gb);
    comment_size = ff_pce_copy_bits(pb, gb, 8);
    for (; comment_size > 0; comment_size--)
        ff_pce_copy_bits(pb, gb, 8);

    return put_bits_count(pb) - offset;
}

static inline void flush_put_bits(PutBitContext *s)
{
#ifndef BITSTREAM_WRITER_LE
    if (s->bit_left < BUF_BITS)
        s->bit_buf <<= s->bit_left;
#endif
    while (s->bit_left < BUF_BITS)
    {
        av_assert0(s->buf_ptr < s->buf_end);
#ifdef BITSTREAM_WRITER_LE
        *s->buf_ptr++ = s->bit_buf;
        s->bit_buf >>= 8;
#else
        *s->buf_ptr++ = s->bit_buf >> (BUF_BITS - 8);
        s->bit_buf <<= 8;
#endif
        s->bit_left += 8;
    }
    s->bit_left = BUF_BITS;
    s->bit_buf = 0;
}




