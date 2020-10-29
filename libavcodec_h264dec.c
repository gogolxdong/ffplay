
// enum AVMediaType
// {
//     AVMEDIA_TYPE_UNKNOWN = -1,
//     AVMEDIA_TYPE_VIDEO,
//     AVMEDIA_TYPE_AUDIO,
//     AVMEDIA_TYPE_DATA,
//     AVMEDIA_TYPE_SUBTITLE,
//     AVMEDIA_TYPE_ATTACHMENT,
//     AVMEDIA_TYPE_NB
// };

// enum AVPictureType
// {
//     AV_PICTURE_TYPE_NONE = 0,
//     AV_PICTURE_TYPE_I,
//     AV_PICTURE_TYPE_P,
//     AV_PICTURE_TYPE_B,
//     AV_PICTURE_TYPE_S,
//     AV_PICTURE_TYPE_SI,
//     AV_PICTURE_TYPE_SP,
//     AV_PICTURE_TYPE_BI,
// };

// static inline int av_clip_c(int a, int amin, int amax)
// {
//     if (a < amin)
//         return amin;
//     else if (a > amax)
//         return amax;
//     else
//         return a;
// }
// static inline int64_t av_clip64_c(int64_t a, int64_t amin, int64_t amax)
// {
//     if (a < amin)
//         return amin;
//     else if (a > amax)
//         return amax;
//     else
//         return a;
// }

// static inline uint8_t av_clip_uint8_c(int a)
// {
//     if (a & (~0xFF))
//         return (~a) >> 31;
//     else
//         return a;
// }

// static inline int8_t av_clip_int8_c(int a)
// {
//     if ((a + 0x80U) & ~0xFF)
//         return (a >> 31) ^ 0x7F;
//     else
//         return a;
// }

// static inline uint16_t av_clip_uint16_c(int a)
// {
//     if (a & (~0xFFFF))
//         return (~a) >> 31;
//     else
//         return a;
// }

// static inline int16_t av_clip_int16_c(int a)
// {
//     if ((a + 0x8000U) & ~0xFFFF)
//         return (a >> 31) ^ 0x7FFF;
//     else
//         return a;
// }

// static inline int32_t av_clipl_int32_c(int64_t a)
// {
//     if ((a + 0x80000000u) & ~0xFFFFFFFFULL)
//         return (int32_t)((a >> 63) ^ 0x7FFFFFFF);
//     else
//         return (int32_t)a;
// }

// static inline int av_clip_intp2_c(int a, int p)
// {
//     if (((unsigned)a + (1 << p)) & ~((2 << p) - 1))
//         return (a >> 31) ^ ((1 << p) - 1);
//     else
//         return a;
// }

// static inline unsigned av_clip_uintp2_c(int a, int p)
// {
//     if (a & ~((1 << p) - 1))
//         return (~a) >> 31 & ((1 << p) - 1);
//     else
//         return a;
// }

// static inline unsigned av_mod_uintp2_c(unsigned a, unsigned p)
// {
//     return a & ((1U << p) - 1);
// }
// static inline int av_sat_add32_c(int a, int b)
// {
//     return av_clipl_int32_c((int64_t)a + b);
// }
// static inline int av_sat_dadd32_c(int a, int b)
// {
//     return av_sat_add32_c(a, av_sat_add32_c(b, b));
// }
// static inline int av_sat_sub32_c(int a, int b)
// {
//     return av_clipl_int32_c((int64_t)a - b);
// }
// static inline int av_sat_dsub32_c(int a, int b)
// {
//     return av_sat_sub32_c(a, av_sat_add32_c(b, b));
// }
// static inline int64_t av_sat_add64_c(int64_t a, int64_t b)
// {
//     int64_t tmp;
//     return !__builtin_add_overflow(a, b, &tmp) ? tmp : (tmp < 0 ? 9223372036854775807LL : (-9223372036854775807LL - 1));
// }
// static inline int64_t av_sat_sub64_c(int64_t a, int64_t b)
// {
//     int64_t tmp;
//     return !__builtin_sub_overflow(a, b, &tmp) ? tmp : (tmp < 0 ? 9223372036854775807LL : (-9223372036854775807LL - 1));
// }
// static inline float av_clipf_c(float a, float amin, float amax)
// {
//     if (a < amin)
//         return amin;
//     else if (a > amax)
//         return amax;
//     else
//         return a;
// }
// static inline double av_clipd_c(double a, double amin, double amax)
// {
//     if (a < amin)
//         return amin;
//     else if (a > amax)
//         return amax;
//     else
//         return a;
// }

// static inline int av_ceil_log2_c(int x)
// {
//     return av_log2((x - 1U) << 1);
// }

// static inline int av_popcount_c(uint32_t x)
// {
//     x -= (x >> 1) & 0x55555555;
//     x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
//     x = (x + (x >> 4)) & 0x0F0F0F0F;
//     x += x >> 8;
//     return (x + (x >> 16)) & 0x3F;
// }

// static inline int av_popcount64_c(uint64_t x)
// {
//     return av_popcount_c((uint32_t)x) + av_popcount_c((uint32_t)(x >> 32));
// }

// static inline int av_parity_c(uint32_t v)
// {
//     return av_popcount_c(v) & 1;
// }

// int av_strerror(int errnum, char *errbuf, size_t errbuf_size);
// static inline char *av_make_error_string(char *errbuf, size_t errbuf_size, int errnum)
// {
//     av_strerror(errnum, errbuf, errbuf_size);
//     return errbuf;
// }

// static inline int av_size_mult(size_t a, size_t b, size_t *r)
// {
//     size_t t = a * b;
//     if ((a | b) >= ((size_t)1 << (sizeof(size_t) * 4)) && a && t / a != b)
//         return (-(
//             22));
//     *r = t;
//     return 0;
// }

// typedef struct AVRational
// {
//     int num;
//     int den;
// } AVRational;
// static inline AVRational av_make_q(int num, int den)
// {
//     AVRational r = {num, den};
//     return r;
// }
// static inline int av_cmp_q(AVRational a, AVRational b)
// {
//     const int64_t tmp = a.num * (int64_t)b.den - b.num * (int64_t)a.den;
//     if (tmp)
//         return (int)((tmp ^ a.den ^ b.den) >> 63) | 1;
//     else if (b.den && a.den)
//         return 0;
//     else if (a.num && b.num)
//         return (a.num >> 31) - (b.num >> 31);
//     else
//         return (-0x7fffffff - 1);
// }

// static inline double av_q2d(AVRational a)
// {
//     return a.num / (double)a.den;
// }

// static inline AVRational av_inv_q(AVRational q)
// {
//     AVRational r = {q.den, q.num};
//     return r;
// }

// union av_intfloat32
// {
//     uint32_t i;
//     float f;
// };

// union av_intfloat64
// {
//     uint64_t i;
//     double f;
// };

// static inline float av_int2float(uint32_t i)
// {
//     union av_intfloat32 v;
//     v.i = i;
//     return v.f;
// }

// static inline uint32_t av_float2int(float f)
// {
//     union av_intfloat32 v;
//     v.f = f;
//     return v.i;
// }

// static inline double av_int2double(uint64_t i)
// {
//     union av_intfloat64 v;
//     v.i = i;
//     return v.f;
// }

// static inline uint64_t av_double2int(double f)
// {
//     union av_intfloat64 v;
//     v.f = f;
//     return v.i;
// }

// enum AVRounding
// {
//     AV_ROUND_ZERO = 0,
//     AV_ROUND_INF = 1,
//     AV_ROUND_DOWN = 2,
//     AV_ROUND_UP = 3,
//     AV_ROUND_NEAR_INF = 5,
//     AV_ROUND_PASS_MINMAX = 8192,
// };
// typedef enum
// {
//     AV_CLASS_CATEGORY_NA = 0,
//     AV_CLASS_CATEGORY_INPUT,
//     AV_CLASS_CATEGORY_OUTPUT,
//     AV_CLASS_CATEGORY_MUXER,
//     AV_CLASS_CATEGORY_DEMUXER,
//     AV_CLASS_CATEGORY_ENCODER,
//     AV_CLASS_CATEGORY_DECODER,
//     AV_CLASS_CATEGORY_FILTER,
//     AV_CLASS_CATEGORY_BITSTREAM_FILTER,
//     AV_CLASS_CATEGORY_SWSCALER,
//     AV_CLASS_CATEGORY_SWRESAMPLER,
//     AV_CLASS_CATEGORY_DEVICE_VIDEO_OUTPUT = 40,
//     AV_CLASS_CATEGORY_DEVICE_VIDEO_INPUT,
//     AV_CLASS_CATEGORY_DEVICE_AUDIO_OUTPUT,
//     AV_CLASS_CATEGORY_DEVICE_AUDIO_INPUT,
//     AV_CLASS_CATEGORY_DEVICE_OUTPUT,
//     AV_CLASS_CATEGORY_DEVICE_INPUT,
//     AV_CLASS_CATEGORY_NB
// } AVClassCategory;

// struct AVOptionRanges;

// typedef struct AVClass
// {
//     const char *class_name;
//     const char *(*item_name)(void *ctx);
//     const struct AVOption *option;
//     int version;
//     int log_level_offset_offset;
//     int parent_log_context_offset;
//     void *(*child_next)(void *obj, void *prev);

//     const struct AVClass *(*child_class_next)(const struct AVClass *prev);
//     AVClassCategory category;
//     AVClassCategory (*get_category)(void *ctx);
//     int (*query_ranges)(struct AVOptionRanges **, void *obj, const char *key, int flags);
//     const struct AVClass *(*child_class_iterate)(void **iter);
// } AVClass;

// enum AVPixelFormat
// {
//     AV_PIX_FMT_NONE = -1,
//     AV_PIX_FMT_YUV420P,
//     AV_PIX_FMT_YUYV422,
//     AV_PIX_FMT_RGB24,
//     AV_PIX_FMT_BGR24,
//     AV_PIX_FMT_YUV422P,
//     AV_PIX_FMT_YUV444P,
//     AV_PIX_FMT_YUV410P,
//     AV_PIX_FMT_YUV411P,
//     AV_PIX_FMT_GRAY8,
//     AV_PIX_FMT_MONOWHITE,
//     AV_PIX_FMT_MONOBLACK,
//     AV_PIX_FMT_PAL8,
//     AV_PIX_FMT_YUVJ420P,
//     AV_PIX_FMT_YUVJ422P,
//     AV_PIX_FMT_YUVJ444P,
//     AV_PIX_FMT_UYVY422,
//     AV_PIX_FMT_UYYVYY411,
//     AV_PIX_FMT_BGR8,
//     AV_PIX_FMT_BGR4,
//     AV_PIX_FMT_BGR4_BYTE,
//     AV_PIX_FMT_RGB8,
//     AV_PIX_FMT_RGB4,
//     AV_PIX_FMT_RGB4_BYTE,
//     AV_PIX_FMT_NV12,
//     AV_PIX_FMT_NV21,
//     AV_PIX_FMT_ARGB,
//     AV_PIX_FMT_RGBA,
//     AV_PIX_FMT_ABGR,
//     AV_PIX_FMT_BGRA,
//     AV_PIX_FMT_GRAY16BE,
//     AV_PIX_FMT_GRAY16LE,
//     AV_PIX_FMT_YUV440P,
//     AV_PIX_FMT_YUVJ440P,
//     AV_PIX_FMT_YUVA420P,
//     AV_PIX_FMT_RGB48BE,
//     AV_PIX_FMT_RGB48LE,
//     AV_PIX_FMT_RGB565BE,
//     AV_PIX_FMT_RGB565LE,
//     AV_PIX_FMT_RGB555BE,
//     AV_PIX_FMT_RGB555LE,
//     AV_PIX_FMT_BGR565BE,
//     AV_PIX_FMT_BGR565LE,
//     AV_PIX_FMT_BGR555BE,
//     AV_PIX_FMT_BGR555LE,
//     AV_PIX_FMT_VAAPI_MOCO,
//     AV_PIX_FMT_VAAPI_IDCT,
//     AV_PIX_FMT_VAAPI_VLD,
//     AV_PIX_FMT_VAAPI = AV_PIX_FMT_VAAPI_VLD,
//     AV_PIX_FMT_YUV420P16LE,
//     AV_PIX_FMT_YUV420P16BE,
//     AV_PIX_FMT_YUV422P16LE,
//     AV_PIX_FMT_YUV422P16BE,
//     AV_PIX_FMT_YUV444P16LE,
//     AV_PIX_FMT_YUV444P16BE,
//     AV_PIX_FMT_DXVA2_VLD,
//     AV_PIX_FMT_RGB444LE,
//     AV_PIX_FMT_RGB444BE,
//     AV_PIX_FMT_BGR444LE,
//     AV_PIX_FMT_BGR444BE,
//     AV_PIX_FMT_YA8,
//     AV_PIX_FMT_Y400A = AV_PIX_FMT_YA8,
//     AV_PIX_FMT_GRAY8A = AV_PIX_FMT_YA8,
//     AV_PIX_FMT_BGR48BE,
//     AV_PIX_FMT_BGR48LE,
//     AV_PIX_FMT_YUV420P9BE,
//     AV_PIX_FMT_YUV420P9LE,
//     AV_PIX_FMT_YUV420P10BE,
//     AV_PIX_FMT_YUV420P10LE,
//     AV_PIX_FMT_YUV422P10BE,
//     AV_PIX_FMT_YUV422P10LE,
//     AV_PIX_FMT_YUV444P9BE,
//     AV_PIX_FMT_YUV444P9LE,
//     AV_PIX_FMT_YUV444P10BE,
//     AV_PIX_FMT_YUV444P10LE,
//     AV_PIX_FMT_YUV422P9BE,
//     AV_PIX_FMT_YUV422P9LE,
//     AV_PIX_FMT_GBRP,
//     AV_PIX_FMT_GBR24P = AV_PIX_FMT_GBRP,
//     AV_PIX_FMT_GBRP9BE,
//     AV_PIX_FMT_GBRP9LE,
//     AV_PIX_FMT_GBRP10BE,
//     AV_PIX_FMT_GBRP10LE,
//     AV_PIX_FMT_GBRP16BE,
//     AV_PIX_FMT_GBRP16LE,
//     AV_PIX_FMT_YUVA422P,
//     AV_PIX_FMT_YUVA444P,
//     AV_PIX_FMT_YUVA420P9BE,
//     AV_PIX_FMT_YUVA420P9LE,
//     AV_PIX_FMT_YUVA422P9BE,
//     AV_PIX_FMT_YUVA422P9LE,
//     AV_PIX_FMT_YUVA444P9BE,
//     AV_PIX_FMT_YUVA444P9LE,
//     AV_PIX_FMT_YUVA420P10BE,
//     AV_PIX_FMT_YUVA420P10LE,
//     AV_PIX_FMT_YUVA422P10BE,
//     AV_PIX_FMT_YUVA422P10LE,
//     AV_PIX_FMT_YUVA444P10BE,
//     AV_PIX_FMT_YUVA444P10LE,
//     AV_PIX_FMT_YUVA420P16BE,
//     AV_PIX_FMT_YUVA420P16LE,
//     AV_PIX_FMT_YUVA422P16BE,
//     AV_PIX_FMT_YUVA422P16LE,
//     AV_PIX_FMT_YUVA444P16BE,
//     AV_PIX_FMT_YUVA444P16LE,
//     AV_PIX_FMT_VDPAU,
//     AV_PIX_FMT_XYZ12LE,
//     AV_PIX_FMT_XYZ12BE,
//     AV_PIX_FMT_NV16,
//     AV_PIX_FMT_NV20LE,
//     AV_PIX_FMT_NV20BE,
//     AV_PIX_FMT_RGBA64BE,
//     AV_PIX_FMT_RGBA64LE,
//     AV_PIX_FMT_BGRA64BE,
//     AV_PIX_FMT_BGRA64LE,
//     AV_PIX_FMT_YVYU422,
//     AV_PIX_FMT_YA16BE,
//     AV_PIX_FMT_YA16LE,
//     AV_PIX_FMT_GBRAP,
//     AV_PIX_FMT_GBRAP16BE,
//     AV_PIX_FMT_GBRAP16LE,
//     AV_PIX_FMT_QSV,
//     AV_PIX_FMT_MMAL,
//     AV_PIX_FMT_D3D11VA_VLD,
//     AV_PIX_FMT_CUDA,
//     AV_PIX_FMT_0RGB,
//     AV_PIX_FMT_RGB0,
//     AV_PIX_FMT_0BGR,
//     AV_PIX_FMT_BGR0,
//     AV_PIX_FMT_YUV420P12BE,
//     AV_PIX_FMT_YUV420P12LE,
//     AV_PIX_FMT_YUV420P14BE,
//     AV_PIX_FMT_YUV420P14LE,
//     AV_PIX_FMT_YUV422P12BE,
//     AV_PIX_FMT_YUV422P12LE,
//     AV_PIX_FMT_YUV422P14BE,
//     AV_PIX_FMT_YUV422P14LE,
//     AV_PIX_FMT_YUV444P12BE,
//     AV_PIX_FMT_YUV444P12LE,
//     AV_PIX_FMT_YUV444P14BE,
//     AV_PIX_FMT_YUV444P14LE,
//     AV_PIX_FMT_GBRP12BE,
//     AV_PIX_FMT_GBRP12LE,
//     AV_PIX_FMT_GBRP14BE,
//     AV_PIX_FMT_GBRP14LE,
//     AV_PIX_FMT_YUVJ411P,
//     AV_PIX_FMT_BAYER_BGGR8,
//     AV_PIX_FMT_BAYER_RGGB8,
//     AV_PIX_FMT_BAYER_GBRG8,
//     AV_PIX_FMT_BAYER_GRBG8,
//     AV_PIX_FMT_BAYER_BGGR16LE,
//     AV_PIX_FMT_BAYER_BGGR16BE,
//     AV_PIX_FMT_BAYER_RGGB16LE,
//     AV_PIX_FMT_BAYER_RGGB16BE,
//     AV_PIX_FMT_BAYER_GBRG16LE,
//     AV_PIX_FMT_BAYER_GBRG16BE,
//     AV_PIX_FMT_BAYER_GRBG16LE,
//     AV_PIX_FMT_BAYER_GRBG16BE,
//     AV_PIX_FMT_XVMC,
//     AV_PIX_FMT_YUV440P10LE,
//     AV_PIX_FMT_YUV440P10BE,
//     AV_PIX_FMT_YUV440P12LE,
//     AV_PIX_FMT_YUV440P12BE,
//     AV_PIX_FMT_AYUV64LE,
//     AV_PIX_FMT_AYUV64BE,
//     AV_PIX_FMT_VIDEOTOOLBOX,
//     AV_PIX_FMT_P010LE,
//     AV_PIX_FMT_P010BE,
//     AV_PIX_FMT_GBRAP12BE,
//     AV_PIX_FMT_GBRAP12LE,
//     AV_PIX_FMT_GBRAP10BE,
//     AV_PIX_FMT_GBRAP10LE,
//     AV_PIX_FMT_MEDIACODEC,
//     AV_PIX_FMT_GRAY12BE,
//     AV_PIX_FMT_GRAY12LE,
//     AV_PIX_FMT_GRAY10BE,
//     AV_PIX_FMT_GRAY10LE,
//     AV_PIX_FMT_P016LE,
//     AV_PIX_FMT_P016BE,
//     AV_PIX_FMT_D3D11,
//     AV_PIX_FMT_GRAY9BE,
//     AV_PIX_FMT_GRAY9LE,
//     AV_PIX_FMT_GBRPF32BE,
//     AV_PIX_FMT_GBRPF32LE,
//     AV_PIX_FMT_GBRAPF32BE,
//     AV_PIX_FMT_GBRAPF32LE,
//     AV_PIX_FMT_DRM_PRIME,
//     AV_PIX_FMT_OPENCL,
//     AV_PIX_FMT_GRAY14BE,
//     AV_PIX_FMT_GRAY14LE,
//     AV_PIX_FMT_GRAYF32BE,
//     AV_PIX_FMT_GRAYF32LE,
//     AV_PIX_FMT_YUVA422P12BE,
//     AV_PIX_FMT_YUVA422P12LE,
//     AV_PIX_FMT_YUVA444P12BE,
//     AV_PIX_FMT_YUVA444P12LE,
//     AV_PIX_FMT_NV24,
//     AV_PIX_FMT_NV42,
//     AV_PIX_FMT_VULKAN,
//     AV_PIX_FMT_Y210BE,
//     AV_PIX_FMT_Y210LE,
//     AV_PIX_FMT_X2RGB10LE,
//     AV_PIX_FMT_X2RGB10BE,
//     AV_PIX_FMT_NB
// };

// enum AVColorPrimaries
// {
//     AVCOL_PRI_RESERVED0 = 0,
//     AVCOL_PRI_BT709 = 1,
//     AVCOL_PRI_UNSPECIFIED = 2,
//     AVCOL_PRI_RESERVED = 3,
//     AVCOL_PRI_BT470M = 4,
//     AVCOL_PRI_BT470BG = 5,
//     AVCOL_PRI_SMPTE170M = 6,
//     AVCOL_PRI_SMPTE240M = 7,
//     AVCOL_PRI_FILM = 8,
//     AVCOL_PRI_BT2020 = 9,
//     AVCOL_PRI_SMPTE428 = 10,
//     AVCOL_PRI_SMPTEST428_1 = AVCOL_PRI_SMPTE428,
//     AVCOL_PRI_SMPTE431 = 11,
//     AVCOL_PRI_SMPTE432 = 12,
//     AVCOL_PRI_EBU3213 = 22,
//     AVCOL_PRI_JEDEC_P22 = AVCOL_PRI_EBU3213,
//     AVCOL_PRI_NB
// };

// enum AVColorTransferCharacteristic
// {
//     AVCOL_TRC_RESERVED0 = 0,
//     AVCOL_TRC_BT709 = 1,
//     AVCOL_TRC_UNSPECIFIED = 2,
//     AVCOL_TRC_RESERVED = 3,
//     AVCOL_TRC_GAMMA22 = 4,
//     AVCOL_TRC_GAMMA28 = 5,
//     AVCOL_TRC_SMPTE170M = 6,
//     AVCOL_TRC_SMPTE240M = 7,
//     AVCOL_TRC_LINEAR = 8,
//     AVCOL_TRC_LOG = 9,
//     AVCOL_TRC_LOG_SQRT = 10,
//     AVCOL_TRC_IEC61966_2_4 = 11,
//     AVCOL_TRC_BT1361_ECG = 12,
//     AVCOL_TRC_IEC61966_2_1 = 13,
//     AVCOL_TRC_BT2020_10 = 14,
//     AVCOL_TRC_BT2020_12 = 15,
//     AVCOL_TRC_SMPTE2084 = 16,
//     AVCOL_TRC_SMPTEST2084 = AVCOL_TRC_SMPTE2084,
//     AVCOL_TRC_SMPTE428 = 17,
//     AVCOL_TRC_SMPTEST428_1 = AVCOL_TRC_SMPTE428,
//     AVCOL_TRC_ARIB_STD_B67 = 18,
//     AVCOL_TRC_NB
// };

// enum AVColorSpace
// {
//     AVCOL_SPC_RGB = 0,
//     AVCOL_SPC_BT709 = 1,
//     AVCOL_SPC_UNSPECIFIED = 2,
//     AVCOL_SPC_RESERVED = 3,
//     AVCOL_SPC_FCC = 4,
//     AVCOL_SPC_BT470BG = 5,
//     AVCOL_SPC_SMPTE170M = 6,
//     AVCOL_SPC_SMPTE240M = 7,
//     AVCOL_SPC_YCGCO = 8,
//     AVCOL_SPC_YCOCG = AVCOL_SPC_YCGCO,
//     AVCOL_SPC_BT2020_NCL = 9,
//     AVCOL_SPC_BT2020_CL = 10,
//     AVCOL_SPC_SMPTE2085 = 11,
//     AVCOL_SPC_CHROMA_DERIVED_NCL = 12,
//     AVCOL_SPC_CHROMA_DERIVED_CL = 13,
//     AVCOL_SPC_ICTCP = 14,
//     AVCOL_SPC_NB
// };
// enum AVColorRange
// {
//     AVCOL_RANGE_UNSPECIFIED = 0,
//     AVCOL_RANGE_MPEG = 1,
//     AVCOL_RANGE_JPEG = 2,
//     AVCOL_RANGE_NB
// };
// enum AVChromaLocation
// {
//     AVCHROMA_LOC_UNSPECIFIED = 0,
//     AVCHROMA_LOC_LEFT = 1,
//     AVCHROMA_LOC_CENTER = 2,
//     AVCHROMA_LOC_TOPLEFT = 3,
//     AVCHROMA_LOC_TOP = 4,
//     AVCHROMA_LOC_BOTTOMLEFT = 5,
//     AVCHROMA_LOC_BOTTOM = 6,
//     AVCHROMA_LOC_NB
// };

// static inline void *av_x_if_null(const void *p, const void *x)
// {
//     return (void *)(intptr_t)(p ? p : x);
// }

// typedef struct AVComponentDescriptor
// {
//     int plane;
//     int step;
//     int offset;
//     int shift;
//     int depth;
//     int step_minus1;
//     int depth_minus1;
//     int offset_plus1;
// } AVComponentDescriptor;
// typedef struct AVPixFmtDescriptor
// {
//     const char *name;
//     uint8_t nb_components;
//     uint8_t log2_chroma_w;
//     uint8_t log2_chroma_h;
//     uint64_t flags;
//     AVComponentDescriptor comp[4];
//     const char *alias;
// } AVPixFmtDescriptor;

// typedef struct AVDictionaryEntry
// {
//     char *key;
//     char *value;
// } AVDictionaryEntry;

// typedef struct AVDictionary AVDictionary;

// enum AVSampleFormat
// {
//     AV_SAMPLE_FMT_NONE = -1,
//     AV_SAMPLE_FMT_U8,
//     AV_SAMPLE_FMT_S16,
//     AV_SAMPLE_FMT_S32,
//     AV_SAMPLE_FMT_FLT,
//     AV_SAMPLE_FMT_DBL,
//     AV_SAMPLE_FMT_U8P,
//     AV_SAMPLE_FMT_S16P,
//     AV_SAMPLE_FMT_S32P,
//     AV_SAMPLE_FMT_FLTP,
//     AV_SAMPLE_FMT_DBLP,
//     AV_SAMPLE_FMT_S64,
//     AV_SAMPLE_FMT_S64P,
//     AV_SAMPLE_FMT_NB
// };

// enum AVOptionType
// {
//     AV_OPT_TYPE_FLAGS,
//     AV_OPT_TYPE_INT,
//     AV_OPT_TYPE_INT64,
//     AV_OPT_TYPE_DOUBLE,
//     AV_OPT_TYPE_FLOAT,
//     AV_OPT_TYPE_STRING,
//     AV_OPT_TYPE_RATIONAL,
//     AV_OPT_TYPE_BINARY,
//     AV_OPT_TYPE_DICT,
//     AV_OPT_TYPE_UINT64,
//     AV_OPT_TYPE_CONST,
//     AV_OPT_TYPE_IMAGE_SIZE,
//     AV_OPT_TYPE_PIXEL_FMT,
//     AV_OPT_TYPE_SAMPLE_FMT,
//     AV_OPT_TYPE_VIDEO_RATE,
//     AV_OPT_TYPE_DURATION,
//     AV_OPT_TYPE_COLOR,
//     AV_OPT_TYPE_CHANNEL_LAYOUT,
//     AV_OPT_TYPE_BOOL,
// };

// typedef struct AVOption
// {
//     const char *name;
//     const char *help;
//     int offset;
//     enum AVOptionType type;
//     union
//     {
//         int64_t i64;
//         double dbl;
//         const char *str;
//         AVRational q;
//     } default_val;
//     double min;
//     double max;
//     int flags;
//     const char *unit;
// } AVOption;

// typedef struct AVOptionRange
// {
//     const char *str;
//     double value_min, value_max;
//     double component_min, component_max;
//     int is_range;
// } AVOptionRange;

// typedef struct AVOptionRanges
// {
//     AVOptionRange **range;
//     int nb_ranges;
//     int nb_components;
// } AVOptionRanges;

// enum
// {
//     AV_OPT_FLAG_IMPLICIT_KEY = 1,
// };

// typedef struct AVBuffer AVBuffer;

// typedef struct AVBufferRef
// {
//     AVBuffer *buffer;
//     uint8_t *data;
//     int size;
// } AVBufferRef;

// enum AVFrameSideDataType
// {
//     AV_FRAME_DATA_PANSCAN,
//     AV_FRAME_DATA_A53_CC,
//     AV_FRAME_DATA_STEREO3D,
//     AV_FRAME_DATA_MATRIXENCODING,
//     AV_FRAME_DATA_DOWNMIX_INFO,
//     AV_FRAME_DATA_REPLAYGAIN,
//     AV_FRAME_DATA_DISPLAYMATRIX,
//     AV_FRAME_DATA_AFD,
//     AV_FRAME_DATA_MOTION_VECTORS,
//     AV_FRAME_DATA_SKIP_SAMPLES,
//     AV_FRAME_DATA_AUDIO_SERVICE_TYPE,
//     AV_FRAME_DATA_MASTERING_DISPLAY_METADATA,
//     AV_FRAME_DATA_GOP_TIMECODE,
//     AV_FRAME_DATA_SPHERICAL,
//     AV_FRAME_DATA_CONTENT_LIGHT_LEVEL,
//     AV_FRAME_DATA_ICC_PROFILE,
//     AV_FRAME_DATA_QP_TABLE_PROPERTIES,
//     AV_FRAME_DATA_QP_TABLE_DATA,
//     AV_FRAME_DATA_S12M_TIMECODE,
//     AV_FRAME_DATA_DYNAMIC_HDR_PLUS,
//     AV_FRAME_DATA_REGIONS_OF_INTEREST,
//     AV_FRAME_DATA_VIDEO_ENC_PARAMS,
//     AV_FRAME_DATA_SEI_UNREGISTERED,
// };

// enum AVActiveFormatDescription
// {
//     AV_AFD_SAME = 8,
//     AV_AFD_4_3 = 9,
//     AV_AFD_16_9 = 10,
//     AV_AFD_14_9 = 11,
//     AV_AFD_4_3_SP_14_9 = 13,
//     AV_AFD_16_9_SP_14_9 = 14,
//     AV_AFD_SP_4_3 = 15,
// };
// typedef struct AVFrameSideData
// {
//     enum AVFrameSideDataType type;
//     uint8_t *data;
//     int size;
//     AVDictionary *metadata;
//     AVBufferRef *buf;
// } AVFrameSideData;
// typedef struct AVRegionOfInterest
// {
//     uint32_t self_size;
//     int top;
//     int bottom;
//     int left;
//     int right;
//     AVRational qoffset;
// } AVRegionOfInterest;
// typedef struct AVFrame
// {
//     uint8_t *data[8];
//     int linesize[8];
//     uint8_t **extended_data;
//     int width, height;
//     int nb_samples;
//     int format;
//     int key_frame;
//     enum AVPictureType pict_type;
//     AVRational sample_aspect_ratio;
//     int64_t pts;

//     int64_t pkt_pts;
//     int64_t pkt_dts;
//     int coded_picture_number;
//     int display_picture_number;
//     int quality;
//     void *opaque;

//     uint64_t error[8];
//     int repeat_pict;
//     int interlaced_frame;
//     int top_field_first;
//     int palette_has_changed;
//     int64_t reordered_opaque;
//     int sample_rate;
//     uint64_t channel_layout;
//     AVBufferRef *buf[8];
//     AVBufferRef **extended_buf;
//     int nb_extended_buf;
//     AVFrameSideData **side_data;
//     int nb_side_data;
//     int flags;
//     enum AVColorRange color_range;
//     enum AVColorPrimaries color_primaries;
//     enum AVColorTransferCharacteristic color_trc;
//     enum AVColorSpace colorspace;
//     enum AVChromaLocation chroma_location;
//     int64_t best_effort_timestamp;
//     int64_t pkt_pos;
//     int64_t pkt_duration;
//     AVDictionary *metadata;
//     int decode_error_flags;
//     int channels;
//     int pkt_size;

//     int8_t *qscale_table;

//     int qstride;

//     int qscale_type;

//     AVBufferRef *qp_table_buf;
//     AVBufferRef *hw_frames_ctx;
//     AVBufferRef *opaque_ref;
//     size_t crop_top;
//     size_t crop_bottom;
//     size_t crop_left;
//     size_t crop_right;
//     AVBufferRef *private_ref;
// } AVFrame;

// enum
// {
//     AV_FRAME_CROP_UNALIGNED = 1 << 0,
// };

// enum AVStereo3DType
// {
//     AV_STEREO3D_2D,
//     AV_STEREO3D_SIDEBYSIDE,
//     AV_STEREO3D_TOPBOTTOM,
//     AV_STEREO3D_FRAMESEQUENCE,
//     AV_STEREO3D_CHECKERBOARD,
//     AV_STEREO3D_SIDEBYSIDE_QUINCUNX,
//     AV_STEREO3D_LINES,
//     AV_STEREO3D_COLUMNS,
// };

// enum AVStereo3DView
// {
//     AV_STEREO3D_VIEW_PACKED,
//     AV_STEREO3D_VIEW_LEFT,
//     AV_STEREO3D_VIEW_RIGHT,
// };

// typedef struct AVStereo3D
// {
//     enum AVStereo3DType type;
//     int flags;
//     enum AVStereo3DView view;
// } AVStereo3D;

// enum AVVideoEncParamsType
// {
//     AV_VIDEO_ENC_PARAMS_NONE = -1,
//     AV_VIDEO_ENC_PARAMS_VP9,
//     AV_VIDEO_ENC_PARAMS_H264,
// };

// typedef struct AVVideoEncParams
// {
//     unsigned int nb_blocks;
//     size_t blocks_offset;
//     size_t block_size;
//     enum AVVideoEncParamsType type;
//     int32_t qp;
//     int32_t delta_qp[4][2];
// } AVVideoEncParams;
// typedef struct AVVideoBlockParams
// {
//     int src_x, src_y;
//     int w, h;
//     int32_t delta_qp;
// } AVVideoBlockParams;

// enum AVMatrixEncoding
// {
//     AV_MATRIX_ENCODING_NONE,
//     AV_MATRIX_ENCODING_DOLBY,
//     AV_MATRIX_ENCODING_DPLII,
//     AV_MATRIX_ENCODING_DPLIIX,
//     AV_MATRIX_ENCODING_DPLIIZ,
//     AV_MATRIX_ENCODING_DOLBYEX,
//     AV_MATRIX_ENCODING_DOLBYHEADPHONE,
//     AV_MATRIX_ENCODING_NB
// };

// enum AVHWDeviceType
// {
//     AV_HWDEVICE_TYPE_NONE,
//     AV_HWDEVICE_TYPE_VDPAU,
//     AV_HWDEVICE_TYPE_CUDA,
//     AV_HWDEVICE_TYPE_VAAPI,
//     AV_HWDEVICE_TYPE_DXVA2,
//     AV_HWDEVICE_TYPE_QSV,
//     AV_HWDEVICE_TYPE_VIDEOTOOLBOX,
//     AV_HWDEVICE_TYPE_D3D11VA,
//     AV_HWDEVICE_TYPE_DRM,
//     AV_HWDEVICE_TYPE_OPENCL,
//     AV_HWDEVICE_TYPE_MEDIACODEC,
//     AV_HWDEVICE_TYPE_VULKAN,
// };

// typedef struct AVHWDeviceInternal AVHWDeviceInternal;
// typedef struct AVHWDeviceContext
// {
//     const AVClass *av_class;
//     AVHWDeviceInternal *internal;
//     enum AVHWDeviceType type;
//     void *hwctx;
//     void (*free)(struct AVHWDeviceContext *ctx);
//     void *user_opaque;
// } AVHWDeviceContext;

// typedef struct AVHWFramesInternal AVHWFramesInternal;
// typedef struct AVHWFramesContext
// {
//     const AVClass *av_class;
//     AVHWFramesInternal *internal;
//     AVBufferRef *device_ref;
//     AVHWDeviceContext *device_ctx;
//     void *hwctx;
//     void (*free)(struct AVHWFramesContext *ctx);
//     void *user_opaque;
//     AVBufferPool *pool;
//     int initial_pool_size;
//     enum AVPixelFormat format;
//     enum AVPixelFormat sw_format;
//     int width, height;
// } AVHWFramesContext;

// enum AVCodecID
// {
//     AV_CODEC_ID_NONE,
//     AV_CODEC_ID_MPEG1VIDEO,
//     AV_CODEC_ID_MPEG2VIDEO,
//     AV_CODEC_ID_H261,
//     AV_CODEC_ID_H263,
//     AV_CODEC_ID_RV10,
//     AV_CODEC_ID_RV20,
//     AV_CODEC_ID_MJPEG,
//     AV_CODEC_ID_MJPEGB,
//     AV_CODEC_ID_LJPEG,
//     AV_CODEC_ID_SP5X,
//     AV_CODEC_ID_JPEGLS,
//     AV_CODEC_ID_MPEG4,
//     AV_CODEC_ID_RAWVIDEO,
//     AV_CODEC_ID_MSMPEG4V1,
//     AV_CODEC_ID_MSMPEG4V2,
//     AV_CODEC_ID_MSMPEG4V3,
//     AV_CODEC_ID_WMV1,
//     AV_CODEC_ID_WMV2,
//     AV_CODEC_ID_H263P,
//     AV_CODEC_ID_H263I,
//     AV_CODEC_ID_FLV1,
//     AV_CODEC_ID_SVQ1,
//     AV_CODEC_ID_SVQ3,
//     AV_CODEC_ID_DVVIDEO,
//     AV_CODEC_ID_HUFFYUV,
//     AV_CODEC_ID_CYUV,
//     AV_CODEC_ID_H264,
//     AV_CODEC_ID_INDEO3,
//     AV_CODEC_ID_VP3,
//     AV_CODEC_ID_THEORA,
//     AV_CODEC_ID_ASV1,
//     AV_CODEC_ID_ASV2,
//     AV_CODEC_ID_FFV1,
//     AV_CODEC_ID_4XM,
//     AV_CODEC_ID_VCR1,
//     AV_CODEC_ID_CLJR,
//     AV_CODEC_ID_MDEC,
//     AV_CODEC_ID_ROQ,
//     AV_CODEC_ID_INTERPLAY_VIDEO,
//     AV_CODEC_ID_XAN_WC3,
//     AV_CODEC_ID_XAN_WC4,
//     AV_CODEC_ID_RPZA,
//     AV_CODEC_ID_CINEPAK,
//     AV_CODEC_ID_WS_VQA,
//     AV_CODEC_ID_MSRLE,
//     AV_CODEC_ID_MSVIDEO1,
//     AV_CODEC_ID_IDCIN,
//     AV_CODEC_ID_8BPS,
//     AV_CODEC_ID_SMC,
//     AV_CODEC_ID_FLIC,
//     AV_CODEC_ID_TRUEMOTION1,
//     AV_CODEC_ID_VMDVIDEO,
//     AV_CODEC_ID_MSZH,
//     AV_CODEC_ID_ZLIB,
//     AV_CODEC_ID_QTRLE,
//     AV_CODEC_ID_TSCC,
//     AV_CODEC_ID_ULTI,
//     AV_CODEC_ID_QDRAW,
//     AV_CODEC_ID_VIXL,
//     AV_CODEC_ID_QPEG,
//     AV_CODEC_ID_PNG,
//     AV_CODEC_ID_PPM,
//     AV_CODEC_ID_PBM,
//     AV_CODEC_ID_PGM,
//     AV_CODEC_ID_PGMYUV,
//     AV_CODEC_ID_PAM,
//     AV_CODEC_ID_FFVHUFF,
//     AV_CODEC_ID_RV30,
//     AV_CODEC_ID_RV40,
//     AV_CODEC_ID_VC1,
//     AV_CODEC_ID_WMV3,
//     AV_CODEC_ID_LOCO,
//     AV_CODEC_ID_WNV1,
//     AV_CODEC_ID_AASC,
//     AV_CODEC_ID_INDEO2,
//     AV_CODEC_ID_FRAPS,
//     AV_CODEC_ID_TRUEMOTION2,
//     AV_CODEC_ID_BMP,
//     AV_CODEC_ID_CSCD,
//     AV_CODEC_ID_MMVIDEO,
//     AV_CODEC_ID_ZMBV,
//     AV_CODEC_ID_AVS,
//     AV_CODEC_ID_SMACKVIDEO,
//     AV_CODEC_ID_NUV,
//     AV_CODEC_ID_KMVC,
//     AV_CODEC_ID_FLASHSV,
//     AV_CODEC_ID_CAVS,
//     AV_CODEC_ID_JPEG2000,
//     AV_CODEC_ID_VMNC,
//     AV_CODEC_ID_VP5,
//     AV_CODEC_ID_VP6,
//     AV_CODEC_ID_VP6F,
//     AV_CODEC_ID_TARGA,
//     AV_CODEC_ID_DSICINVIDEO,
//     AV_CODEC_ID_TIERTEXSEQVIDEO,
//     AV_CODEC_ID_TIFF,
//     AV_CODEC_ID_GIF,
//     AV_CODEC_ID_DXA,
//     AV_CODEC_ID_DNXHD,
//     AV_CODEC_ID_THP,
//     AV_CODEC_ID_SGI,
//     AV_CODEC_ID_C93,
//     AV_CODEC_ID_BETHSOFTVID,
//     AV_CODEC_ID_PTX,
//     AV_CODEC_ID_TXD,
//     AV_CODEC_ID_VP6A,
//     AV_CODEC_ID_AMV,
//     AV_CODEC_ID_VB,
//     AV_CODEC_ID_PCX,
//     AV_CODEC_ID_SUNRAST,
//     AV_CODEC_ID_INDEO4,
//     AV_CODEC_ID_INDEO5,
//     AV_CODEC_ID_MIMIC,
//     AV_CODEC_ID_RL2,
//     AV_CODEC_ID_ESCAPE124,
//     AV_CODEC_ID_DIRAC,
//     AV_CODEC_ID_BFI,
//     AV_CODEC_ID_CMV,
//     AV_CODEC_ID_MOTIONPIXELS,
//     AV_CODEC_ID_TGV,
//     AV_CODEC_ID_TGQ,
//     AV_CODEC_ID_TQI,
//     AV_CODEC_ID_AURA,
//     AV_CODEC_ID_AURA2,
//     AV_CODEC_ID_V210X,
//     AV_CODEC_ID_TMV,
//     AV_CODEC_ID_V210,
//     AV_CODEC_ID_DPX,
//     AV_CODEC_ID_MAD,
//     AV_CODEC_ID_FRWU,
//     AV_CODEC_ID_FLASHSV2,
//     AV_CODEC_ID_CDGRAPHICS,
//     AV_CODEC_ID_R210,
//     AV_CODEC_ID_ANM,
//     AV_CODEC_ID_BINKVIDEO,
//     AV_CODEC_ID_IFF_ILBM,
//     AV_CODEC_ID_KGV1,
//     AV_CODEC_ID_YOP,
//     AV_CODEC_ID_VP8,
//     AV_CODEC_ID_PICTOR,
//     AV_CODEC_ID_ANSI,
//     AV_CODEC_ID_A64_MULTI,
//     AV_CODEC_ID_A64_MULTI5,
//     AV_CODEC_ID_R10K,
//     AV_CODEC_ID_MXPEG,
//     AV_CODEC_ID_LAGARITH,
//     AV_CODEC_ID_PRORES,
//     AV_CODEC_ID_JV,
//     AV_CODEC_ID_DFA,
//     AV_CODEC_ID_WMV3IMAGE,
//     AV_CODEC_ID_VC1IMAGE,
//     AV_CODEC_ID_UTVIDEO,
//     AV_CODEC_ID_BMV_VIDEO,
//     AV_CODEC_ID_VBLE,
//     AV_CODEC_ID_DXTORY,
//     AV_CODEC_ID_V410,
//     AV_CODEC_ID_XWD,
//     AV_CODEC_ID_CDXL,
//     AV_CODEC_ID_XBM,
//     AV_CODEC_ID_ZEROCODEC,
//     AV_CODEC_ID_MSS1,
//     AV_CODEC_ID_MSA1,
//     AV_CODEC_ID_TSCC2,
//     AV_CODEC_ID_MTS2,
//     AV_CODEC_ID_CLLC,
//     AV_CODEC_ID_MSS2,
//     AV_CODEC_ID_VP9,
//     AV_CODEC_ID_AIC,
//     AV_CODEC_ID_ESCAPE130,
//     AV_CODEC_ID_G2M,
//     AV_CODEC_ID_WEBP,
//     AV_CODEC_ID_HNM4_VIDEO,
//     AV_CODEC_ID_HEVC,
//     AV_CODEC_ID_FIC,
//     AV_CODEC_ID_ALIAS_PIX,
//     AV_CODEC_ID_BRENDER_PIX,
//     AV_CODEC_ID_PAF_VIDEO,
//     AV_CODEC_ID_EXR,
//     AV_CODEC_ID_VP7,
//     AV_CODEC_ID_SANM,
//     AV_CODEC_ID_SGIRLE,
//     AV_CODEC_ID_MVC1,
//     AV_CODEC_ID_MVC2,
//     AV_CODEC_ID_HQX,
//     AV_CODEC_ID_TDSC,
//     AV_CODEC_ID_HQ_HQA,
//     AV_CODEC_ID_HAP,
//     AV_CODEC_ID_DDS,
//     AV_CODEC_ID_DXV,
//     AV_CODEC_ID_SCREENPRESSO,
//     AV_CODEC_ID_RSCC,
//     AV_CODEC_ID_AVS2,
//     AV_CODEC_ID_PGX,
//     AV_CODEC_ID_AVS3,
//     AV_CODEC_ID_Y41P = 0x8000,
//     AV_CODEC_ID_AVRP,
//     AV_CODEC_ID_012V,
//     AV_CODEC_ID_AVUI,
//     AV_CODEC_ID_AYUV,
//     AV_CODEC_ID_TARGA_Y216,
//     AV_CODEC_ID_V308,
//     AV_CODEC_ID_V408,
//     AV_CODEC_ID_YUV4,
//     AV_CODEC_ID_AVRN,
//     AV_CODEC_ID_CPIA,
//     AV_CODEC_ID_XFACE,
//     AV_CODEC_ID_SNOW,
//     AV_CODEC_ID_SMVJPEG,
//     AV_CODEC_ID_APNG,
//     AV_CODEC_ID_DAALA,
//     AV_CODEC_ID_CFHD,
//     AV_CODEC_ID_TRUEMOTION2RT,
//     AV_CODEC_ID_M101,
//     AV_CODEC_ID_MAGICYUV,
//     AV_CODEC_ID_SHEERVIDEO,
//     AV_CODEC_ID_YLC,
//     AV_CODEC_ID_PSD,
//     AV_CODEC_ID_PIXLET,
//     AV_CODEC_ID_SPEEDHQ,
//     AV_CODEC_ID_FMVC,
//     AV_CODEC_ID_SCPR,
//     AV_CODEC_ID_CLEARVIDEO,
//     AV_CODEC_ID_XPM,
//     AV_CODEC_ID_AV1,
//     AV_CODEC_ID_BITPACKED,
//     AV_CODEC_ID_MSCC,
//     AV_CODEC_ID_SRGC,
//     AV_CODEC_ID_SVG,
//     AV_CODEC_ID_GDV,
//     AV_CODEC_ID_FITS,
//     AV_CODEC_ID_IMM4,
//     AV_CODEC_ID_PROSUMER,
//     AV_CODEC_ID_MWSC,
//     AV_CODEC_ID_WCMV,
//     AV_CODEC_ID_RASC,
//     AV_CODEC_ID_HYMT,
//     AV_CODEC_ID_ARBC,
//     AV_CODEC_ID_AGM,
//     AV_CODEC_ID_LSCR,
//     AV_CODEC_ID_VP4,
//     AV_CODEC_ID_IMM5,
//     AV_CODEC_ID_MVDV,
//     AV_CODEC_ID_MVHA,
//     AV_CODEC_ID_CDTOONS,
//     AV_CODEC_ID_MV30,
//     AV_CODEC_ID_NOTCHLC,
//     AV_CODEC_ID_PFM,
//     AV_CODEC_ID_MOBICLIP,
//     AV_CODEC_ID_PHOTOCD,
//     AV_CODEC_ID_IPU,
//     AV_CODEC_ID_ARGO,
//     AV_CODEC_ID_CRI,
//     AV_CODEC_ID_FIRST_AUDIO = 0x10000,
//     AV_CODEC_ID_PCM_S16LE = 0x10000,
//     AV_CODEC_ID_PCM_S16BE,
//     AV_CODEC_ID_PCM_U16LE,
//     AV_CODEC_ID_PCM_U16BE,
//     AV_CODEC_ID_PCM_S8,
//     AV_CODEC_ID_PCM_U8,
//     AV_CODEC_ID_PCM_MULAW,
//     AV_CODEC_ID_PCM_ALAW,
//     AV_CODEC_ID_PCM_S32LE,
//     AV_CODEC_ID_PCM_S32BE,
//     AV_CODEC_ID_PCM_U32LE,
//     AV_CODEC_ID_PCM_U32BE,
//     AV_CODEC_ID_PCM_S24LE,
//     AV_CODEC_ID_PCM_S24BE,
//     AV_CODEC_ID_PCM_U24LE,
//     AV_CODEC_ID_PCM_U24BE,
//     AV_CODEC_ID_PCM_S24DAUD,
//     AV_CODEC_ID_PCM_ZORK,
//     AV_CODEC_ID_PCM_S16LE_PLANAR,
//     AV_CODEC_ID_PCM_DVD,
//     AV_CODEC_ID_PCM_F32BE,
//     AV_CODEC_ID_PCM_F32LE,
//     AV_CODEC_ID_PCM_F64BE,
//     AV_CODEC_ID_PCM_F64LE,
//     AV_CODEC_ID_PCM_BLURAY,
//     AV_CODEC_ID_PCM_LXF,
//     AV_CODEC_ID_S302M,
//     AV_CODEC_ID_PCM_S8_PLANAR,
//     AV_CODEC_ID_PCM_S24LE_PLANAR,
//     AV_CODEC_ID_PCM_S32LE_PLANAR,
//     AV_CODEC_ID_PCM_S16BE_PLANAR,
//     AV_CODEC_ID_PCM_S64LE = 0x10800,
//     AV_CODEC_ID_PCM_S64BE,
//     AV_CODEC_ID_PCM_F16LE,
//     AV_CODEC_ID_PCM_F24LE,
//     AV_CODEC_ID_PCM_VIDC,
//     AV_CODEC_ID_ADPCM_IMA_QT = 0x11000,
//     AV_CODEC_ID_ADPCM_IMA_WAV,
//     AV_CODEC_ID_ADPCM_IMA_DK3,
//     AV_CODEC_ID_ADPCM_IMA_DK4,
//     AV_CODEC_ID_ADPCM_IMA_WS,
//     AV_CODEC_ID_ADPCM_IMA_SMJPEG,
//     AV_CODEC_ID_ADPCM_MS,
//     AV_CODEC_ID_ADPCM_4XM,
//     AV_CODEC_ID_ADPCM_XA,
//     AV_CODEC_ID_ADPCM_ADX,
//     AV_CODEC_ID_ADPCM_EA,
//     AV_CODEC_ID_ADPCM_G726,
//     AV_CODEC_ID_ADPCM_CT,
//     AV_CODEC_ID_ADPCM_SWF,
//     AV_CODEC_ID_ADPCM_YAMAHA,
//     AV_CODEC_ID_ADPCM_SBPRO_4,
//     AV_CODEC_ID_ADPCM_SBPRO_3,
//     AV_CODEC_ID_ADPCM_SBPRO_2,
//     AV_CODEC_ID_ADPCM_THP,
//     AV_CODEC_ID_ADPCM_IMA_AMV,
//     AV_CODEC_ID_ADPCM_EA_R1,
//     AV_CODEC_ID_ADPCM_EA_R3,
//     AV_CODEC_ID_ADPCM_EA_R2,
//     AV_CODEC_ID_ADPCM_IMA_EA_SEAD,
//     AV_CODEC_ID_ADPCM_IMA_EA_EACS,
//     AV_CODEC_ID_ADPCM_EA_XAS,
//     AV_CODEC_ID_ADPCM_EA_MAXIS_XA,
//     AV_CODEC_ID_ADPCM_IMA_ISS,
//     AV_CODEC_ID_ADPCM_G722,
//     AV_CODEC_ID_ADPCM_IMA_APC,
//     AV_CODEC_ID_ADPCM_VIMA,
//     AV_CODEC_ID_ADPCM_AFC = 0x11800,
//     AV_CODEC_ID_ADPCM_IMA_OKI,
//     AV_CODEC_ID_ADPCM_DTK,
//     AV_CODEC_ID_ADPCM_IMA_RAD,
//     AV_CODEC_ID_ADPCM_G726LE,
//     AV_CODEC_ID_ADPCM_THP_LE,
//     AV_CODEC_ID_ADPCM_PSX,
//     AV_CODEC_ID_ADPCM_AICA,
//     AV_CODEC_ID_ADPCM_IMA_DAT4,
//     AV_CODEC_ID_ADPCM_MTAF,
//     AV_CODEC_ID_ADPCM_AGM,
//     AV_CODEC_ID_ADPCM_ARGO,
//     AV_CODEC_ID_ADPCM_IMA_SSI,
//     AV_CODEC_ID_ADPCM_ZORK,
//     AV_CODEC_ID_ADPCM_IMA_APM,
//     AV_CODEC_ID_ADPCM_IMA_ALP,
//     AV_CODEC_ID_ADPCM_IMA_MTF,
//     AV_CODEC_ID_ADPCM_IMA_CUNNING,
//     AV_CODEC_ID_ADPCM_IMA_MOFLEX,
//     AV_CODEC_ID_AMR_NB = 0x12000,
//     AV_CODEC_ID_AMR_WB,
//     AV_CODEC_ID_RA_144 = 0x13000,
//     AV_CODEC_ID_RA_288,
//     AV_CODEC_ID_ROQ_DPCM = 0x14000,
//     AV_CODEC_ID_INTERPLAY_DPCM,
//     AV_CODEC_ID_XAN_DPCM,
//     AV_CODEC_ID_SOL_DPCM,
//     AV_CODEC_ID_SDX2_DPCM = 0x14800,
//     AV_CODEC_ID_GREMLIN_DPCM,
//     AV_CODEC_ID_DERF_DPCM,
//     AV_CODEC_ID_MP2 = 0x15000,
//     AV_CODEC_ID_MP3,
//     AV_CODEC_ID_AAC,
//     AV_CODEC_ID_AC3,
//     AV_CODEC_ID_DTS,
//     AV_CODEC_ID_VORBIS,
//     AV_CODEC_ID_DVAUDIO,
//     AV_CODEC_ID_WMAV1,
//     AV_CODEC_ID_WMAV2,
//     AV_CODEC_ID_MACE3,
//     AV_CODEC_ID_MACE6,
//     AV_CODEC_ID_VMDAUDIO,
//     AV_CODEC_ID_FLAC,
//     AV_CODEC_ID_MP3ADU,
//     AV_CODEC_ID_MP3ON4,
//     AV_CODEC_ID_SHORTEN,
//     AV_CODEC_ID_ALAC,
//     AV_CODEC_ID_WESTWOOD_SND1,
//     AV_CODEC_ID_GSM,
//     AV_CODEC_ID_QDM2,
//     AV_CODEC_ID_COOK,
//     AV_CODEC_ID_TRUESPEECH,
//     AV_CODEC_ID_TTA,
//     AV_CODEC_ID_SMACKAUDIO,
//     AV_CODEC_ID_QCELP,
//     AV_CODEC_ID_WAVPACK,
//     AV_CODEC_ID_DSICINAUDIO,
//     AV_CODEC_ID_IMC,
//     AV_CODEC_ID_MUSEPACK7,
//     AV_CODEC_ID_MLP,
//     AV_CODEC_ID_GSM_MS,
//     AV_CODEC_ID_ATRAC3,
//     AV_CODEC_ID_APE,
//     AV_CODEC_ID_NELLYMOSER,
//     AV_CODEC_ID_MUSEPACK8,
//     AV_CODEC_ID_SPEEX,
//     AV_CODEC_ID_WMAVOICE,
//     AV_CODEC_ID_WMAPRO,
//     AV_CODEC_ID_WMALOSSLESS,
//     AV_CODEC_ID_ATRAC3P,
//     AV_CODEC_ID_EAC3,
//     AV_CODEC_ID_SIPR,
//     AV_CODEC_ID_MP1,
//     AV_CODEC_ID_TWINVQ,
//     AV_CODEC_ID_TRUEHD,
//     AV_CODEC_ID_MP4ALS,
//     AV_CODEC_ID_ATRAC1,
//     AV_CODEC_ID_BINKAUDIO_RDFT,
//     AV_CODEC_ID_BINKAUDIO_DCT,
//     AV_CODEC_ID_AAC_LATM,
//     AV_CODEC_ID_QDMC,
//     AV_CODEC_ID_CELT,
//     AV_CODEC_ID_G723_1,
//     AV_CODEC_ID_G729,
//     AV_CODEC_ID_8SVX_EXP,
//     AV_CODEC_ID_8SVX_FIB,
//     AV_CODEC_ID_BMV_AUDIO,
//     AV_CODEC_ID_RALF,
//     AV_CODEC_ID_IAC,
//     AV_CODEC_ID_ILBC,
//     AV_CODEC_ID_OPUS,
//     AV_CODEC_ID_COMFORT_NOISE,
//     AV_CODEC_ID_TAK,
//     AV_CODEC_ID_METASOUND,
//     AV_CODEC_ID_PAF_AUDIO,
//     AV_CODEC_ID_ON2AVC,
//     AV_CODEC_ID_DSS_SP,
//     AV_CODEC_ID_CODEC2,
//     AV_CODEC_ID_FFWAVESYNTH = 0x15800,
//     AV_CODEC_ID_SONIC,
//     AV_CODEC_ID_SONIC_LS,
//     AV_CODEC_ID_EVRC,
//     AV_CODEC_ID_SMV,
//     AV_CODEC_ID_DSD_LSBF,
//     AV_CODEC_ID_DSD_MSBF,
//     AV_CODEC_ID_DSD_LSBF_PLANAR,
//     AV_CODEC_ID_DSD_MSBF_PLANAR,
//     AV_CODEC_ID_4GV,
//     AV_CODEC_ID_INTERPLAY_ACM,
//     AV_CODEC_ID_XMA1,
//     AV_CODEC_ID_XMA2,
//     AV_CODEC_ID_DST,
//     AV_CODEC_ID_ATRAC3AL,
//     AV_CODEC_ID_ATRAC3PAL,
//     AV_CODEC_ID_DOLBY_E,
//     AV_CODEC_ID_APTX,
//     AV_CODEC_ID_APTX_HD,
//     AV_CODEC_ID_SBC,
//     AV_CODEC_ID_ATRAC9,
//     AV_CODEC_ID_HCOM,
//     AV_CODEC_ID_ACELP_KELVIN,
//     AV_CODEC_ID_MPEGH_3D_AUDIO,
//     AV_CODEC_ID_SIREN,
//     AV_CODEC_ID_HCA,
//     AV_CODEC_ID_FASTAUDIO,
//     AV_CODEC_ID_FIRST_SUBTITLE = 0x17000,
//     AV_CODEC_ID_DVD_SUBTITLE = 0x17000,
//     AV_CODEC_ID_DVB_SUBTITLE,
//     AV_CODEC_ID_TEXT,
//     AV_CODEC_ID_XSUB,
//     AV_CODEC_ID_SSA,
//     AV_CODEC_ID_MOV_TEXT,
//     AV_CODEC_ID_HDMV_PGS_SUBTITLE,
//     AV_CODEC_ID_DVB_TELETEXT,
//     AV_CODEC_ID_SRT,
//     AV_CODEC_ID_MICRODVD = 0x17800,
//     AV_CODEC_ID_EIA_608,
//     AV_CODEC_ID_JACOSUB,
//     AV_CODEC_ID_SAMI,
//     AV_CODEC_ID_REALTEXT,
//     AV_CODEC_ID_STL,
//     AV_CODEC_ID_SUBVIEWER1,
//     AV_CODEC_ID_SUBVIEWER,
//     AV_CODEC_ID_SUBRIP,
//     AV_CODEC_ID_WEBVTT,
//     AV_CODEC_ID_MPL2,
//     AV_CODEC_ID_VPLAYER,
//     AV_CODEC_ID_PJS,
//     AV_CODEC_ID_ASS,
//     AV_CODEC_ID_HDMV_TEXT_SUBTITLE,
//     AV_CODEC_ID_TTML,
//     AV_CODEC_ID_ARIB_CAPTION,
//     AV_CODEC_ID_FIRST_UNKNOWN = 0x18000,
//     AV_CODEC_ID_TTF = 0x18000,
//     AV_CODEC_ID_SCTE_35,
//     AV_CODEC_ID_EPG,
//     AV_CODEC_ID_BINTEXT = 0x18800,
//     AV_CODEC_ID_XBIN,
//     AV_CODEC_ID_IDF,
//     AV_CODEC_ID_OTF,
//     AV_CODEC_ID_SMPTE_KLV,
//     AV_CODEC_ID_DVD_NAV,
//     AV_CODEC_ID_TIMED_ID3,
//     AV_CODEC_ID_BIN_DATA,
//     AV_CODEC_ID_PROBE = 0x19000,
//     AV_CODEC_ID_MPEG2TS = 0x20000,
//     AV_CODEC_ID_MPEG4SYSTEMS = 0x20001,
//     AV_CODEC_ID_FFMETADATA = 0x21000,
//     AV_CODEC_ID_WRAPPED_AVFRAME = 0x21001,
// };

// enum AVFieldOrder
// {
//     AV_FIELD_UNKNOWN,
//     AV_FIELD_PROGRESSIVE,
//     AV_FIELD_TT,
//     AV_FIELD_BB,
//     AV_FIELD_TB,
//     AV_FIELD_BT,
// };
// typedef struct AVCodecParameters
// {
//     enum AVMediaType codec_type;
//     enum AVCodecID codec_id;
//     uint32_t codec_tag;
//     uint8_t *extradata;
//     int extradata_size;
//     int format;
//     int64_t bit_rate;
//     int bits_per_coded_sample;
//     int bits_per_raw_sample;
//     int profile;
//     int level;
//     int width;
//     int height;
//     AVRational sample_aspect_ratio;
//     enum AVFieldOrder field_order;
//     enum AVColorRange color_range;
//     enum AVColorPrimaries color_primaries;
//     enum AVColorTransferCharacteristic color_trc;
//     enum AVColorSpace color_space;
//     enum AVChromaLocation chroma_location;
//     int video_delay;
//     uint64_t channel_layout;
//     int channels;
//     int sample_rate;
//     int block_align;
//     int frame_size;
//     int initial_padding;
//     int trailing_padding;
//     int seek_preroll;
// } AVCodecParameters;

// enum AVPacketSideDataType
// {
//     AV_PKT_DATA_PALETTE,
//     AV_PKT_DATA_NEW_EXTRADATA,
//     AV_PKT_DATA_PARAM_CHANGE,
//     AV_PKT_DATA_H263_MB_INFO,
//     AV_PKT_DATA_REPLAYGAIN,
//     AV_PKT_DATA_DISPLAYMATRIX,
//     AV_PKT_DATA_STEREO3D,
//     AV_PKT_DATA_AUDIO_SERVICE_TYPE,
//     AV_PKT_DATA_QUALITY_STATS,
//     AV_PKT_DATA_FALLBACK_TRACK,
//     AV_PKT_DATA_CPB_PROPERTIES,
//     AV_PKT_DATA_SKIP_SAMPLES,
//     AV_PKT_DATA_JP_DUALMONO,
//     AV_PKT_DATA_STRINGS_METADATA,
//     AV_PKT_DATA_SUBTITLE_POSITION,
//     AV_PKT_DATA_MATROSKA_BLOCKADDITIONAL,
//     AV_PKT_DATA_WEBVTT_IDENTIFIER,
//     AV_PKT_DATA_WEBVTT_SETTINGS,
//     AV_PKT_DATA_METADATA_UPDATE,
//     AV_PKT_DATA_MPEGTS_STREAM_ID,
//     AV_PKT_DATA_MASTERING_DISPLAY_METADATA,
//     AV_PKT_DATA_SPHERICAL,
//     AV_PKT_DATA_CONTENT_LIGHT_LEVEL,
//     AV_PKT_DATA_A53_CC,
//     AV_PKT_DATA_ENCRYPTION_INIT_INFO,
//     AV_PKT_DATA_ENCRYPTION_INFO,
//     AV_PKT_DATA_AFD,
//     AV_PKT_DATA_PRFT,
//     AV_PKT_DATA_ICC_PROFILE,
//     AV_PKT_DATA_DOVI_CONF,
//     AV_PKT_DATA_S12M_TIMECODE,
//     AV_PKT_DATA_NB
// };

// typedef struct AVPacketSideData
// {
//     uint8_t *data;
//     int size;
//     enum AVPacketSideDataType type;
// } AVPacketSideData;
// typedef struct AVPacket
// {
//     AVBufferRef *buf;
//     int64_t pts;
//     int64_t dts;
//     uint8_t *data;
//     int size;
//     int stream_index;
//     int flags;
//     AVPacketSideData *side_data;
//     int side_data_elems;
//     int64_t duration;
//     int64_t pos;

//     int64_t convergence_duration;
// } AVPacket;

// typedef struct AVPacketList
// {
//     AVPacket pkt;
//     struct AVPacketList *next;
// } AVPacketList;

// enum AVSideDataParamChangeFlags
// {
//     AV_SIDE_DATA_PARAM_CHANGE_CHANNEL_COUNT = 0x0001,
//     AV_SIDE_DATA_PARAM_CHANGE_CHANNEL_LAYOUT = 0x0002,
//     AV_SIDE_DATA_PARAM_CHANGE_SAMPLE_RATE = 0x0004,
//     AV_SIDE_DATA_PARAM_CHANGE_DIMENSIONS = 0x0008,
// };

// typedef struct AVBSFInternal AVBSFInternal;
// typedef struct AVBSFContext
// {
//     const AVClass *av_class;
//     const struct AVBitStreamFilter *filter;
//     AVBSFInternal *internal;
//     void *priv_data;
//     AVCodecParameters *par_in;
//     AVCodecParameters *par_out;
//     AVRational time_base_in;
//     AVRational time_base_out;
// } AVBSFContext;

// typedef struct AVBitStreamFilter
// {
//     const char *name;
//     const enum AVCodecID *codec_ids;
//     const AVClass *priv_class;
//     int priv_data_size;
//     int (*init)(AVBSFContext *ctx);
//     int (*filter)(AVBSFContext *ctx, AVPacket *pkt);
//     void (*close)(AVBSFContext *ctx);
//     void (*flush)(AVBSFContext *ctx);
// } AVBitStreamFilter;

// typedef struct AVProfile
// {
//     int profile;
//     const char *name;
// } AVProfile;

// typedef struct AVCodecDefault AVCodecDefault;

// struct AVCodecContext;
// struct AVSubtitle;
// struct AVPacket;

// typedef struct AVCodec
// {
//     const char *name;
//     const char *long_name;
//     enum AVMediaType type;
//     enum AVCodecID id;
//     int capabilities;
//     const AVRational *supported_framerates;
//     const enum AVPixelFormat *pix_fmts;
//     const int *supported_samplerates;
//     const enum AVSampleFormat *sample_fmts;
//     const uint64_t *channel_layouts;
//     uint8_t max_lowres;
//     const AVClass *priv_class;
//     const AVProfile *profiles;
//     const char *wrapper_name;
//     int priv_data_size;
//     struct AVCodec *next;
//     int (*update_thread_context)(struct AVCodecContext *dst, const struct AVCodecContext *src);
//     const AVCodecDefault *defaults;
//     void (*init_static_data)(struct AVCodec *codec);
//     int (*init)(struct AVCodecContext *);
//     int (*encode_sub)(struct AVCodecContext *, uint8_t *buf, int buf_size,
//                       const struct AVSubtitle *sub);
//     int (*encode2)(struct AVCodecContext *avctx, struct AVPacket *avpkt,
//                    const struct AVFrame *frame, int *got_packet_ptr);
//     int (*decode)(struct AVCodecContext *, void *outdata, int *outdata_size, struct AVPacket *avpkt);
//     int (*close)(struct AVCodecContext *);
//     int (*receive_packet)(struct AVCodecContext *avctx, struct AVPacket *avpkt);
//     int (*receive_frame)(struct AVCodecContext *avctx, struct AVFrame *frame);
//     void (*flush)(struct AVCodecContext *);
//     int caps_internal;
//     const char *bsfs;
//     const struct AVCodecHWConfigInternal **hw_configs;
//     const uint32_t *codec_tags;
// } AVCodec;

// enum
// {
//     AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX = 0x01,
//     AV_CODEC_HW_CONFIG_METHOD_HW_FRAMES_CTX = 0x02,
//     AV_CODEC_HW_CONFIG_METHOD_INTERNAL = 0x04,
//     AV_CODEC_HW_CONFIG_METHOD_AD_HOC = 0x08,
// };

// typedef struct AVCodecHWConfig
// {
//     enum AVPixelFormat pix_fmt;
//     int methods;
//     enum AVHWDeviceType device_type;
// } AVCodecHWConfig;
// const AVCodecHWConfig *avcodec_get_hw_config(const AVCodec *codec, int index);

// typedef struct AVCodecDescriptor
// {
//     enum AVCodecID id;
//     enum AVMediaType type;
//     const char *name;
//     const char *long_name;
//     int props;
//     const char *const *mime_types;
//     const struct AVProfile *profiles;
// } AVCodecDescriptor;

// enum AVDiscard
// {
//     AVDISCARD_NONE = -16,
//     AVDISCARD_DEFAULT = 0,
//     AVDISCARD_NONREF = 8,
//     AVDISCARD_BIDIR = 16,
//     AVDISCARD_NONINTRA = 24,
//     AVDISCARD_NONKEY = 32,
//     AVDISCARD_ALL = 48,
// };

// enum AVAudioServiceType
// {
//     AV_AUDIO_SERVICE_TYPE_MAIN = 0,
//     AV_AUDIO_SERVICE_TYPE_EFFECTS = 1,
//     AV_AUDIO_SERVICE_TYPE_VISUALLY_IMPAIRED = 2,
//     AV_AUDIO_SERVICE_TYPE_HEARING_IMPAIRED = 3,
//     AV_AUDIO_SERVICE_TYPE_DIALOGUE = 4,
//     AV_AUDIO_SERVICE_TYPE_COMMENTARY = 5,
//     AV_AUDIO_SERVICE_TYPE_EMERGENCY = 6,
//     AV_AUDIO_SERVICE_TYPE_VOICE_OVER = 7,
//     AV_AUDIO_SERVICE_TYPE_KARAOKE = 8,
//     AV_AUDIO_SERVICE_TYPE_NB,
// };

// typedef struct RcOverride
// {
//     int start_frame;
//     int end_frame;
//     int qscale;
//     float quality_factor;
// } RcOverride;

// typedef struct AVPanScan
// {
//     int id;
//     int width;
//     int height;
//     int16_t position[3][2];
// } AVPanScan;

// typedef struct AVCPBProperties
// {
//     int max_bitrate;
//     int min_bitrate;
//     int avg_bitrate;
//     int buffer_size;
//     uint64_t vbv_delay;
// } AVCPBProperties;

// typedef struct AVProducerReferenceTime
// {
//     int64_t wallclock;
//     int flags;
// } AVProducerReferenceTime;

// struct AVCodecInternal;
// typedef struct AVCodecContext
// {
//     const AVClass *av_class;
//     int log_level_offset;
//     enum AVMediaType codec_type;
//     const struct AVCodec *codec;
//     enum AVCodecID codec_id;
//     unsigned int codec_tag;
//     void *priv_data;
//     struct AVCodecInternal *internal;
//     void *opaque;
//     int64_t bit_rate;
//     int bit_rate_tolerance;
//     int global_quality;
//     int compression_level;
//     int flags;
//     int flags2;
//     uint8_t *extradata;
//     int extradata_size;
//     AVRational time_base;
//     int ticks_per_frame;
//     int delay;
//     int width, height;
//     int coded_width, coded_height;
//     int gop_size;
//     enum AVPixelFormat pix_fmt;
//     void (*draw_horiz_band)(struct AVCodecContext *s,
//                             const AVFrame *src, int offset[8],
//                             int y, int type, int height);
//     enum AVPixelFormat (*get_format)(struct AVCodecContext *s, const enum AVPixelFormat *fmt);
//     int max_b_frames;
//     float b_quant_factor;

//     int b_frame_strategy;
//     float b_quant_offset;
//     int has_b_frames;

//     int mpeg_quant;
//     float i_quant_factor;
//     float i_quant_offset;
//     float lumi_masking;
//     float temporal_cplx_masking;
//     float spatial_cplx_masking;
//     float p_masking;
//     float dark_masking;
//     int slice_count;

//     int prediction_method;
//     int *slice_offset;
//     AVRational sample_aspect_ratio;
//     int me_cmp;
//     int me_sub_cmp;
//     int mb_cmp;
//     int ildct_cmp;
//     int dia_size;
//     int last_predictor_count;

//     int pre_me;
//     int me_pre_cmp;
//     int pre_dia_size;
//     int me_subpel_quality;
//     int me_range;
//     int slice_flags;
//     int mb_decision;
//     uint16_t *intra_matrix;
//     uint16_t *inter_matrix;

//     int scenechange_threshold;

//     int noise_reduction;
//     int intra_dc_precision;
//     int skip_top;
//     int skip_bottom;
//     int mb_lmin;
//     int mb_lmax;

//     int me_penalty_compensation;
//     int bidir_refine;

//     int brd_scale;
//     int keyint_min;
//     int refs;

//     int chromaoffset;
//     int mv0_threshold;

//     int b_sensitivity;
//     enum AVColorPrimaries color_primaries;
//     enum AVColorTransferCharacteristic color_trc;
//     enum AVColorSpace colorspace;
//     enum AVColorRange color_range;
//     enum AVChromaLocation chroma_sample_location;
//     int slices;
//     enum AVFieldOrder field_order;
//     int sample_rate;
//     int channels;
//     enum AVSampleFormat sample_fmt;
//     int frame_size;
//     int frame_number;
//     int block_align;
//     int cutoff;
//     uint64_t channel_layout;
//     uint64_t request_channel_layout;
//     enum AVAudioServiceType audio_service_type;
//     enum AVSampleFormat request_sample_fmt;
//     int (*get_buffer2)(struct AVCodecContext *s, AVFrame *frame, int flags);

//     int refcounted_frames;
//     float qcompress;
//     float qblur;
//     int qmin;
//     int qmax;
//     int max_qdiff;
//     int rc_buffer_size;
//     int rc_override_count;
//     RcOverride *rc_override;
//     int64_t rc_max_rate;
//     int64_t rc_min_rate;
//     float rc_max_available_vbv_use;
//     float rc_min_vbv_overflow_use;
//     int rc_initial_buffer_occupancy;

//     int coder_type;

//     int context_model;

//     int frame_skip_threshold;

//     int frame_skip_factor;

//     int frame_skip_exp;

//     int frame_skip_cmp;
//     int trellis;

//     int min_prediction_order;

//     int max_prediction_order;

//     int64_t timecode_frame_start;

//     void (*rtp_callback)(struct AVCodecContext *avctx, void *data, int size, int mb_nb);

//     int rtp_payload_size;

//     int mv_bits;

//     int header_bits;

//     int i_tex_bits;

//     int p_tex_bits;

//     int i_count;

//     int p_count;

//     int skip_count;

//     int misc_bits;

//     int frame_bits;
//     char *stats_out;
//     char *stats_in;
//     int workaround_bugs;
//     int strict_std_compliance;
//     int error_concealment;
//     int debug;
//     int err_recognition;
//     int64_t reordered_opaque;
//     const struct AVHWAccel *hwaccel;
//     void *hwaccel_context;
//     uint64_t error[8];
//     int dct_algo;
//     int idct_algo;
//     int bits_per_coded_sample;
//     int bits_per_raw_sample;
//     int lowres;
//     AVFrame *coded_frame;
//     int thread_count;
//     int thread_type;
//     int active_thread_type;
//     int thread_safe_callbacks;
//     int (*execute)(struct AVCodecContext *c, int (*func)(struct AVCodecContext *c2, void *arg), void *arg2, int *ret, int count, int size);
//     int (*execute2)(struct AVCodecContext *c, int (*func)(struct AVCodecContext *c2, void *arg, int jobnr, int threadnr), void *arg2, int *ret, int count);
//     int nsse_weight;
//     int profile;
//     int level;
//     enum AVDiscard skip_loop_filter;
//     enum AVDiscard skip_idct;
//     enum AVDiscard skip_frame;
//     uint8_t *subtitle_header;
//     int subtitle_header_size;

//     uint64_t vbv_delay;

//     int side_data_only_packets;
//     int initial_padding;
//     AVRational framerate;
//     enum AVPixelFormat sw_pix_fmt;
//     AVRational pkt_timebase;
//     const AVCodecDescriptor *codec_descriptor;
//     int64_t pts_correction_num_faulty_pts;
//     int64_t pts_correction_num_faulty_dts;
//     int64_t pts_correction_last_pts;
//     int64_t pts_correction_last_dts;
//     char *sub_charenc;
//     int sub_charenc_mode;
//     int skip_alpha;
//     int seek_preroll;
//     int debug_mv;
//     uint16_t *chroma_intra_matrix;
//     uint8_t *dump_separator;
//     char *codec_whitelist;
//     unsigned properties;
//     AVPacketSideData *coded_side_data;
//     int nb_coded_side_data;
//     AVBufferRef *hw_frames_ctx;
//     int sub_text_format;
//     int trailing_padding;
//     int64_t max_pixels;
//     AVBufferRef *hw_device_ctx;
//     int hwaccel_flags;
//     int apply_cropping;
//     int extra_hw_frames;
//     int discard_damaged_percentage;
//     int64_t max_samples;
//     int export_side_data;
// } AVCodecContext;

// struct AVSubtitle;

// struct MpegEncContext;
// typedef struct AVHWAccel
// {
//     const char *name;
//     enum AVMediaType type;
//     enum AVCodecID id;
//     enum AVPixelFormat pix_fmt;
//     int capabilities;
//     int (*alloc_frame)(AVCodecContext *avctx, AVFrame *frame);
//     int (*start_frame)(AVCodecContext *avctx, const uint8_t *buf, uint32_t buf_size);
//     int (*decode_params)(AVCodecContext *avctx, int type, const uint8_t *buf, uint32_t buf_size);
//     int (*decode_slice)(AVCodecContext *avctx, const uint8_t *buf, uint32_t buf_size);
//     int (*end_frame)(AVCodecContext *avctx);
//     int frame_priv_data_size;
//     void (*decode_mb)(struct MpegEncContext *s);
//     int (*init)(AVCodecContext *avctx);
//     int (*uninit)(AVCodecContext *avctx);
//     int priv_data_size;
//     int caps_internal;
//     int (*frame_params)(AVCodecContext *avctx, AVBufferRef *hw_frames_ctx);
// } AVHWAccel;

// typedef struct AVPicture
// {

//     uint8_t *data[8];

//     int linesize[8];
// } AVPicture;

// enum AVSubtitleType
// {
//     SUBTITLE_NONE,
//     SUBTITLE_BITMAP,
//     SUBTITLE_TEXT,
//     SUBTITLE_ASS,
// };

// typedef struct AVSubtitleRect
// {
//     int x;
//     int y;
//     int w;
//     int h;
//     int nb_colors;

//     AVPicture pict;
//     uint8_t *data[4];
//     int linesize[4];
//     enum AVSubtitleType type;
//     char *text;
//     char *ass;
//     int flags;
// } AVSubtitleRect;

// typedef struct AVSubtitle
// {
//     uint16_t format;
//     uint32_t start_display_time;
//     uint32_t end_display_time;
//     unsigned num_rects;
//     AVSubtitleRect **rects;
//     int64_t pts;
// } AVSubtitle;

// enum AVPictureStructure
// {
//     AV_PICTURE_STRUCTURE_UNKNOWN,
//     AV_PICTURE_STRUCTURE_TOP_FIELD,
//     AV_PICTURE_STRUCTURE_BOTTOM_FIELD,
//     AV_PICTURE_STRUCTURE_FRAME,
// };

// typedef struct AVCodecParserContext
// {
//     void *priv_data;
//     struct AVCodecParser *parser;
//     int64_t frame_offset;
//     int64_t cur_offset;
//     int64_t next_frame_offset;
//     int pict_type;
//     int repeat_pict;
//     int64_t pts;
//     int64_t dts;
//     int64_t last_pts;
//     int64_t last_dts;
//     int fetch_timestamp;
//     int cur_frame_start_index;
//     int64_t cur_frame_offset[4];
//     int64_t cur_frame_pts[4];
//     int64_t cur_frame_dts[4];
//     int flags;
//     int64_t offset;
//     int64_t cur_frame_end[4];
//     int key_frame;

//     int64_t convergence_duration;
//     int dts_sync_point;
//     int dts_ref_dts_delta;
//     int pts_dts_delta;
//     int64_t cur_frame_pos[4];
//     int64_t pos;
//     int64_t last_pos;
//     int duration;
//     enum AVFieldOrder field_order;
//     enum AVPictureStructure picture_structure;
//     int output_picture_number;
//     int width;
//     int height;
//     int coded_width;
//     int coded_height;
//     int format;
// } AVCodecParserContext;

// typedef struct AVCodecParser
// {
//     int codec_ids[5];
//     int priv_data_size;
//     int (*parser_init)(AVCodecParserContext *s);
//     int (*parser_parse)(AVCodecParserContext *s,
//                         AVCodecContext *avctx,
//                         const uint8_t **poutbuf, int *poutbuf_size,
//                         const uint8_t *buf, int buf_size);
//     void (*parser_close)(AVCodecParserContext *s);
//     int (*split)(AVCodecContext *avctx, const uint8_t *buf, int buf_size);
//     struct AVCodecParser *next;
// } AVCodecParser;

// typedef struct AVBitStreamFilterContext
// {
//     void *priv_data;
//     const struct AVBitStreamFilter *filter;
//     AVCodecParserContext *parser;
//     struct AVBitStreamFilterContext *next;
//     char *args;
// } AVBitStreamFilterContext;

// AVHWAccel *av_hwaccel_next(const AVHWAccel *hwaccel);
// enum AVLockOp
// {
//     AV_LOCK_CREATE,
//     AV_LOCK_OBTAIN,
//     AV_LOCK_RELEASE,
//     AV_LOCK_DESTROY,
// };

// int av_lockmgr_register(int (*cb)(void **mutex, enum AVLockOp op));

// int avcodec_is_open(AVCodecContext *s);
// AVCPBProperties *av_cpb_properties_alloc(size_t *size);

// typedef struct DecodeSimpleContext
// {
//     AVPacket *in_pkt;
// } DecodeSimpleContext;

// typedef struct EncodeSimpleContext
// {
//     AVFrame *in_frame;
// } EncodeSimpleContext;

// typedef struct AVCodecInternal
// {
//     int is_copy;
//     int last_audio_frame;
//     AVFrame *to_free;
//     AVBufferRef *pool;
//     void *thread_ctx;
//     DecodeSimpleContext ds;
//     AVBSFContext *bsf;
//     AVPacket *last_pkt_props;
//     AVPacketList *pkt_props;
//     AVPacketList *pkt_props_tail;
//     uint8_t *byte_buffer;
//     unsigned int byte_buffer_size;
//     void *frame_thread_encoder;
//     EncodeSimpleContext es;
//     int skip_samples;
//     void *hwaccel_priv_data;
//     int draining;
//     AVPacket *buffer_pkt;
//     AVFrame *buffer_frame;
//     int draining_done;
//     int compat_decode_warned;
//     size_t compat_decode_consumed;
//     size_t compat_decode_partial_size;
//     AVFrame *compat_decode_frame;
//     AVPacket *compat_encode_packet;
//     int showed_multi_packet_warning;
//     int skip_samples_multiplier;
//     int nb_draining_errors;
//     int changed_frames_dropped;
//     int initial_format;
//     int initial_width, initial_height;
//     int initial_sample_rate;
//     int initial_channels;
//     uint64_t initial_channel_layout;
// } AVCodecInternal;

// struct AVCodecDefault
// {
//     const uint8_t *key;
//     const uint8_t *value;
// };

// typedef union
// {
//     uint64_t u64;
//     uint32_t u32[2];
//     uint16_t u16[4];
//     uint8_t u8[8];
//     double f64;
//     float f32[2];
// } av_alias64;

// typedef union
// {
//     uint32_t u32;
//     uint16_t u16[2];
//     uint8_t u8[4];
//     float f32;
// } av_alias32;

// typedef union
// {
//     uint16_t u16;
//     uint8_t u8[2];
// } av_alias16;
// union unaligned_64
// {
//     uint64_t l;
// };
// union unaligned_32
// {
//     uint32_t l;
// };
// union unaligned_16
// {
//     uint16_t l;
// };

// typedef struct GetByteContext
// {
//     const uint8_t *buffer, *buffer_end, *buffer_start;
// } GetByteContext;

// typedef struct PutByteContext
// {
//     uint8_t *buffer, *buffer_end, *buffer_start;
//     int eof;
// } PutByteContext;

// extern const uint8_t ff_log2_run[41];

// int ff_match_2uint16(const uint16_t (*tab)[2], int size, int a, int b);

// unsigned int avpriv_toupper4(unsigned int x);

// void ff_color_frame(AVFrame *frame, const int color[4]);

// int ff_alloc_packet2(AVCodecContext *avctx, AVPacket *avpkt, int64_t size, int64_t min_size);

// static inline int64_t ff_samples_to_time_base(AVCodecContext *avctx,
//                                               int64_t samples)
// {
//     if (samples == ((int64_t)0x8000000000000000ULL))
//         return ((int64_t)0x8000000000000000ULL);
//     return av_rescale_q(samples, (AVRational){1, avctx->sample_rate},
//                         avctx->time_base);
// }

// static inline float ff_exp2fi(int x)
// {
//     if (-126 <= x && x <= 128)
//         return av_int2float((x + 127) << 23);
//     else if (x > 128)
//         return __builtin_inff();
//     else if (x > -150)
//         return av_int2float(1 << (x + 149));
//     else
//         return 0;
// }

// static inline uint16_t av_bswap16(uint16_t x)
// {
//     x = (x >> 8) | (x << 8);
//     return x;
// }

// static inline uint32_t av_bswap32(uint32_t x)
// {
//     return ((((x) << 8 & 0xff00) | ((x) >> 8 & 0x00ff)) << 16 | ((((x) >> 16) << 8 & 0xff00) | (((x) >> 16) >> 8 & 0x00ff)));
// }

// static inline uint64_t av_bswap64(uint64_t x)
// {
//     return (uint64_t)av_bswap32(x) << 32 | av_bswap32(x >> 32);
// }

// static inline uint64_t bytestream_get_le64(const uint8_t **b)
// {
//     (*b) += 8;
//     return (((const union unaligned_64 *)(*b - 8))->l);
// }
// static inline void bytestream_put_le64(uint8_t **b, const uint64_t value)
// {
//     ((((union unaligned_64 *)(*b))->l) = (value));
//     (*b) += 8;
// }
// static inline void bytestream2_put_le64u(PutByteContext *p, const uint64_t value) { bytestream_put_le64(&p->buffer, value); }
// static inline void bytestream2_put_le64(PutByteContext *p, const uint64_t value)
// {
//     if (!p->eof && (p->buffer_end - p->buffer >= 8))
//     {
//         ((((union unaligned_64 *)(p->buffer))->l) = (value));
//         p->buffer += 8;
//     }
//     else
//         p->eof = 1;
// }
// static inline uint64_t bytestream2_get_le64u(GetByteContext *g) { return bytestream_get_le64(&g->buffer); }
// static inline uint64_t bytestream2_get_le64(GetByteContext *g)
// {
//     if (g->buffer_end - g->buffer < 8)
//     {
//         g->buffer = g->buffer_end;
//         return 0;
//     }
//     return bytestream2_get_le64u(g);
// }
// static inline uint64_t bytestream2_peek_le64u(GetByteContext *g) { return (((const union unaligned_64 *)(g->buffer))->l); }
// static inline uint64_t bytestream2_peek_le64(GetByteContext *g)
// {
//     if (g->buffer_end - g->buffer < 8)
//         return 0;
//     return bytestream2_peek_le64u(g);
// }
// static inline unsigned int bytestream_get_le32(const uint8_t **b)
// {
//     (*b) += 4;
//     return (((const union unaligned_32 *)(*b - 4))->l);
// }
// static inline void bytestream_put_le32(uint8_t **b, const unsigned int value)
// {
//     ((((union unaligned_32 *)(*b))->l) = (value));
//     (*b) += 4;
// }
// static inline void bytestream2_put_le32u(PutByteContext *p, const unsigned int value) { bytestream_put_le32(&p->buffer, value); }
// static inline void bytestream2_put_le32(PutByteContext *p, const unsigned int value)
// {
//     if (!p->eof && (p->buffer_end - p->buffer >= 4))
//     {
//         ((((union unaligned_32 *)(p->buffer))->l) = (value));
//         p->buffer += 4;
//     }
//     else
//         p->eof = 1;
// }
// static inline unsigned int bytestream2_get_le32u(GetByteContext *g) { return bytestream_get_le32(&g->buffer); }
// static inline unsigned int bytestream2_get_le32(GetByteContext *g)
// {
//     if (g->buffer_end - g->buffer < 4)
//     {
//         g->buffer = g->buffer_end;
//         return 0;
//     }
//     return bytestream2_get_le32u(g);
// }
// static inline unsigned int bytestream2_peek_le32u(GetByteContext *g) { return (((const union unaligned_32 *)(g->buffer))->l); }
// static inline unsigned int bytestream2_peek_le32(GetByteContext *g)
// {
//     if (g->buffer_end - g->buffer < 4)
//         return 0;
//     return bytestream2_peek_le32u(g);
// }
// static inline unsigned int bytestream_get_le24(const uint8_t **b)
// {
//     (*b) += 3;
//     return ((((const uint8_t *)(*b - 3))[2] << 16) | (((const uint8_t *)(*b - 3))[1] << 8) | ((const uint8_t *)(*b - 3))[0]);
// }
// static inline void bytestream_put_le24(uint8_t **b, const unsigned int value)
// {
//     do
//     {
//         ((uint8_t *)(*b))[0] = (value);
//         ((uint8_t *)(*b))[1] = (value) >> 8;
//         ((uint8_t *)(*b))[2] = (value) >> 16;
//     } while (0);
//     (*b) += 3;
// }
// static inline void bytestream2_put_le24u(PutByteContext *p, const unsigned int value) { bytestream_put_le24(&p->buffer, value); }
// static inline void bytestream2_put_le24(PutByteContext *p, const unsigned int value)
// {
//     if (!p->eof && (p->buffer_end - p->buffer >= 3))
//     {
//         do
//         {
//             ((uint8_t *)(p->buffer))[0] = (value);
//             ((uint8_t *)(p->buffer))[1] = (value) >> 8;
//             ((uint8_t *)(p->buffer))[2] = (value) >> 16;
//         } while (0);
//         p->buffer += 3;
//     }
//     else
//         p->eof = 1;
// }
// static inline unsigned int bytestream2_get_le24u(GetByteContext *g) { return bytestream_get_le24(&g->buffer); }
// static inline unsigned int bytestream2_get_le24(GetByteContext *g)
// {
//     if (g->buffer_end - g->buffer < 3)
//     {
//         g->buffer = g->buffer_end;
//         return 0;
//     }
//     return bytestream2_get_le24u(g);
// }
// static inline unsigned int bytestream2_peek_le24u(GetByteContext *g) { return ((((const uint8_t *)(g->buffer))[2] << 16) | (((const uint8_t *)(g->buffer))[1] << 8) | ((const uint8_t *)(g->buffer))[0]); }
// static inline unsigned int bytestream2_peek_le24(GetByteContext *g)
// {
//     if (g->buffer_end - g->buffer < 3)
//         return 0;
//     return bytestream2_peek_le24u(g);
// }
// static inline unsigned int bytestream_get_le16(const uint8_t **b)
// {
//     (*b) += 2;
//     return (((const union unaligned_16 *)(*b - 2))->l);
// }
// static inline void bytestream_put_le16(uint8_t **b, const unsigned int value)
// {
//     ((((union unaligned_16 *)(*b))->l) = (value));
//     (*b) += 2;
// }
// static inline void bytestream2_put_le16u(PutByteContext *p, const unsigned int value) { bytestream_put_le16(&p->buffer, value); }
// static inline void bytestream2_put_le16(PutByteContext *p, const unsigned int value)
// {
//     if (!p->eof && (p->buffer_end - p->buffer >= 2))
//     {
//         ((((union unaligned_16 *)(p->buffer))->l) = (value));
//         p->buffer += 2;
//     }
//     else
//         p->eof = 1;
// }
// static inline unsigned int bytestream2_get_le16u(GetByteContext *g) { return bytestream_get_le16(&g->buffer); }
// static inline unsigned int bytestream2_get_le16(GetByteContext *g)
// {
//     if (g->buffer_end - g->buffer < 2)
//     {
//         g->buffer = g->buffer_end;
//         return 0;
//     }
//     return bytestream2_get_le16u(g);
// }
// static inline unsigned int bytestream2_peek_le16u(GetByteContext *g) { return (((const union unaligned_16 *)(g->buffer))->l); }
// static inline unsigned int bytestream2_peek_le16(GetByteContext *g)
// {
//     if (g->buffer_end - g->buffer < 2)
//         return 0;
//     return bytestream2_peek_le16u(g);
// }
// static inline uint64_t bytestream_get_be64(const uint8_t **b)
// {
//     (*b) += 8;
//     return av_bswap64((((const union unaligned_64 *)(*b - 8))->l));
// }
// static inline void bytestream_put_be64(uint8_t **b, const uint64_t value)
// {
//     ((((union unaligned_64 *)(*b))->l) = (av_bswap64(value)));
//     (*b) += 8;
// }
// static inline void bytestream2_put_be64u(PutByteContext *p, const uint64_t value) { bytestream_put_be64(&p->buffer, value); }
// static inline void bytestream2_put_be64(PutByteContext *p, const uint64_t value)
// {
//     if (!p->eof && (p->buffer_end - p->buffer >= 8))
//     {
//         ((((union unaligned_64 *)(p->buffer))->l) = (av_bswap64(value)));
//         p->buffer += 8;
//     }
//     else
//         p->eof = 1;
// }
// static inline uint64_t bytestream2_get_be64u(GetByteContext *g) { return bytestream_get_be64(&g->buffer); }
// static inline uint64_t bytestream2_get_be64(GetByteContext *g)
// {
//     if (g->buffer_end - g->buffer < 8)
//     {
//         g->buffer = g->buffer_end;
//         return 0;
//     }
//     return bytestream2_get_be64u(g);
// }
// static inline uint64_t bytestream2_peek_be64u(GetByteContext *g) { return av_bswap64((((const union unaligned_64 *)(g->buffer))->l)); }
// static inline uint64_t bytestream2_peek_be64(GetByteContext *g)
// {
//     if (g->buffer_end - g->buffer < 8)
//         return 0;
//     return bytestream2_peek_be64u(g);
// }
// static inline unsigned int bytestream_get_be32(const uint8_t **b)
// {
//     (*b) += 4;
//     return av_bswap32((((const union unaligned_32 *)(*b - 4))->l));
// }
// static inline void bytestream_put_be32(uint8_t **b, const unsigned int value)
// {
//     ((((union unaligned_32 *)(*b))->l) = (av_bswap32(value)));
//     (*b) += 4;
// }
// static inline void bytestream2_put_be32u(PutByteContext *p, const unsigned int value) { bytestream_put_be32(&p->buffer, value); }
// static inline void bytestream2_put_be32(PutByteContext *p, const unsigned int value)
// {
//     if (!p->eof && (p->buffer_end - p->buffer >= 4))
//     {
//         ((((union unaligned_32 *)(p->buffer))->l) = (av_bswap32(value)));
//         p->buffer += 4;
//     }
//     else
//         p->eof = 1;
// }
// static inline unsigned int bytestream2_get_be32u(GetByteContext *g) { return bytestream_get_be32(&g->buffer); }
// static inline unsigned int bytestream2_get_be32(GetByteContext *g)
// {
//     if (g->buffer_end - g->buffer < 4)
//     {
//         g->buffer = g->buffer_end;
//         return 0;
//     }
//     return bytestream2_get_be32u(g);
// }
// static inline unsigned int bytestream2_peek_be32u(GetByteContext *g) { return av_bswap32((((const union unaligned_32 *)(g->buffer))->l)); }
// static inline unsigned int bytestream2_peek_be32(GetByteContext *g)
// {
//     if (g->buffer_end - g->buffer < 4)
//         return 0;
//     return bytestream2_peek_be32u(g);
// }
// static inline unsigned int bytestream_get_be24(const uint8_t **b)
// {
//     (*b) += 3;
//     return ((((const uint8_t *)(*b - 3))[0] << 16) | (((const uint8_t *)(*b - 3))[1] << 8) | ((const uint8_t *)(*b - 3))[2]);
// }
// static inline void bytestream_put_be24(uint8_t **b, const unsigned int value)
// {
//     do
//     {
//         ((uint8_t *)(*b))[2] = (value);
//         ((uint8_t *)(*b))[1] = (value) >> 8;
//         ((uint8_t *)(*b))[0] = (value) >> 16;
//     } while (0);
//     (*b) += 3;
// }
// static inline void bytestream2_put_be24u(PutByteContext *p, const unsigned int value) { bytestream_put_be24(&p->buffer, value); }
// static inline void bytestream2_put_be24(PutByteContext *p, const unsigned int value)
// {
//     if (!p->eof && (p->buffer_end - p->buffer >= 3))
//     {
//         do
//         {
//             ((uint8_t *)(p->buffer))[2] = (value);
//             ((uint8_t *)(p->buffer))[1] = (value) >> 8;
//             ((uint8_t *)(p->buffer))[0] = (value) >> 16;
//         } while (0);
//         p->buffer += 3;
//     }
//     else
//         p->eof = 1;
// }
// static inline unsigned int bytestream2_get_be24u(GetByteContext *g) { return bytestream_get_be24(&g->buffer); }
// static inline unsigned int bytestream2_get_be24(GetByteContext *g)
// {
//     if (g->buffer_end - g->buffer < 3)
//     {
//         g->buffer = g->buffer_end;
//         return 0;
//     }
//     return bytestream2_get_be24u(g);
// }
// static inline unsigned int bytestream2_peek_be24u(GetByteContext *g) { return ((((const uint8_t *)(g->buffer))[0] << 16) | (((const uint8_t *)(g->buffer))[1] << 8) | ((const uint8_t *)(g->buffer))[2]); }
// static inline unsigned int bytestream2_peek_be24(GetByteContext *g)
// {
//     if (g->buffer_end - g->buffer < 3)
//         return 0;
//     return bytestream2_peek_be24u(g);
// }
// static inline unsigned int bytestream_get_be16(const uint8_t **b)
// {
//     (*b) += 2;
//     return av_bswap16((((const union unaligned_16 *)(*b - 2))->l));
// }
// static inline void bytestream_put_be16(uint8_t **b, const unsigned int value)
// {
//     ((((union unaligned_16 *)(*b))->l) = (av_bswap16(value)));
//     (*b) += 2;
// }
// static inline void bytestream2_put_be16u(PutByteContext *p, const unsigned int value) { bytestream_put_be16(&p->buffer, value); }
// static inline void bytestream2_put_be16(PutByteContext *p, const unsigned int value)
// {
//     if (!p->eof && (p->buffer_end - p->buffer >= 2))
//     {
//         ((((union unaligned_16 *)(p->buffer))->l) = (av_bswap16(value)));
//         p->buffer += 2;
//     }
//     else
//         p->eof = 1;
// }
// static inline unsigned int bytestream2_get_be16u(GetByteContext *g) { return bytestream_get_be16(&g->buffer); }
// static inline unsigned int bytestream2_get_be16(GetByteContext *g)
// {
//     if (g->buffer_end - g->buffer < 2)
//     {
//         g->buffer = g->buffer_end;
//         return 0;
//     }
//     return bytestream2_get_be16u(g);
// }
// static inline unsigned int bytestream2_peek_be16u(GetByteContext *g) { return av_bswap16((((const union unaligned_16 *)(g->buffer))->l)); }
// static inline unsigned int bytestream2_peek_be16(GetByteContext *g)
// {
//     if (g->buffer_end - g->buffer < 2)
//         return 0;
//     return bytestream2_peek_be16u(g);
// }
// static inline unsigned int bytestream_get_byte(const uint8_t **b)
// {
//     (*b) += 1;
//     return (((const uint8_t *)(*b - 1))[0]);
// }
// static inline void bytestream_put_byte(uint8_t **b, const unsigned int value)
// {
//     do
//     {
//         ((uint8_t *)(*b))[0] = (value);
//     } while (0);
//     (*b) += 1;
// }
// static inline void bytestream2_put_byteu(PutByteContext *p, const unsigned int value) { bytestream_put_byte(&p->buffer, value); }
// static inline void bytestream2_put_byte(PutByteContext *p, const unsigned int value)
// {
//     if (!p->eof && (p->buffer_end - p->buffer >= 1))
//     {
//         do
//         {
//             ((uint8_t *)(p->buffer))[0] = (value);
//         } while (0);
//         p->buffer += 1;
//     }
//     else
//         p->eof = 1;
// }
// static inline unsigned int bytestream2_get_byteu(GetByteContext *g) { return bytestream_get_byte(&g->buffer); }
// static inline unsigned int bytestream2_get_byte(GetByteContext *g)
// {
//     if (g->buffer_end - g->buffer < 1)
//     {
//         g->buffer = g->buffer_end;
//         return 0;
//     }
//     return bytestream2_get_byteu(g);
// }
// static inline unsigned int bytestream2_peek_byteu(GetByteContext *g) { return (((const uint8_t *)(g->buffer))[0]); }
// static inline unsigned int bytestream2_peek_byte(GetByteContext *g)
// {
//     if (g->buffer_end - g->buffer < 1)
//         return 0;
//     return bytestream2_peek_byteu(g);
// }

// static inline void bytestream2_init(GetByteContext *g,
//                                     const uint8_t *buf,
//                                     int buf_size)
// {
//     do
//     {
//         if (!(buf_size >= 0))
//         {
//             av_log(
//                 ((void *)0), 0, "Assertion %s failed at %s:%d\n", "buf_size >= 0", "C:\\FFmpeg\\libavcodec\\bytestream.h", 141);
//             abort();
//         }
//     } while (0);
//     g->buffer = buf;
//     g->buffer_start = buf;
//     g->buffer_end = buf + buf_size;
// }

// static inline void bytestream2_init_writer(PutByteContext *p,
//                                            uint8_t *buf,
//                                            int buf_size)
// {
//     do
//     {
//         if (!(buf_size >= 0))
//         {
//             av_log(
//                 ((void *)0), 0, "Assertion %s failed at %s:%d\n", "buf_size >= 0", "C:\\FFmpeg\\libavcodec\\bytestream.h", 151);
//             abort();
//         }
//     } while (0);
//     p->buffer = buf;
//     p->buffer_start = buf;
//     p->buffer_end = buf + buf_size;
//     p->eof = 0;
// }

// static inline int bytestream2_get_bytes_left(GetByteContext *g)
// {
//     return g->buffer_end - g->buffer;
// }

// static inline int bytestream2_get_bytes_left_p(PutByteContext *p)
// {
//     return p->buffer_end - p->buffer;
// }

// static inline void bytestream2_skip(GetByteContext *g,
//                                     unsigned int size)
// {
//     g->buffer += ((g->buffer_end - g->buffer) > (size) ? (size) : (g->buffer_end - g->buffer));
// }

// static inline void bytestream2_skipu(GetByteContext *g,
//                                      unsigned int size)
// {
//     g->buffer += size;
// }

// static inline void bytestream2_skip_p(PutByteContext *p,
//                                       unsigned int size)
// {
//     int size2;
//     if (p->eof)
//         return;
//     size2 = ((p->buffer_end - p->buffer) > (size) ? (size) : (p->buffer_end - p->buffer));
//     if (size2 != size)
//         p->eof = 1;
//     p->buffer += size2;
// }

// static inline int bytestream2_tell(GetByteContext *g)
// {
//     return (int)(g->buffer - g->buffer_start);
// }

// static inline int bytestream2_tell_p(PutByteContext *p)
// {
//     return (int)(p->buffer - p->buffer_start);
// }

// static inline int bytestream2_size(GetByteContext *g)
// {
//     return (int)(g->buffer_end - g->buffer_start);
// }

// static inline int bytestream2_size_p(PutByteContext *p)
// {
//     return (int)(p->buffer_end - p->buffer_start);
// }

// static inline int bytestream2_seek(GetByteContext *g,
//                                    int offset,
//                                    int whence)
// {
//     switch (whence)
//     {
//     case 1:
//         offset = av_clip_c(offset, -(g->buffer - g->buffer_start),
//                            g->buffer_end - g->buffer);
//         g->buffer += offset;
//         break;
//     case 2:
//         offset = av_clip_c(offset, -(g->buffer_end - g->buffer_start), 0);
//         g->buffer = g->buffer_end + offset;
//         break;
//     case 0:
//         offset = av_clip_c(offset, 0, g->buffer_end - g->buffer_start);
//         g->buffer = g->buffer_start + offset;
//         break;
//     default:
//         return (-(
//             22));
//     }
//     return bytestream2_tell(g);
// }

// static inline int bytestream2_seek_p(PutByteContext *p,
//                                      int offset,
//                                      int whence)
// {
//     p->eof = 0;
//     switch (whence)
//     {
//     case 1:
//         if (p->buffer_end - p->buffer < offset)
//             p->eof = 1;
//         offset = av_clip_c(offset, -(p->buffer - p->buffer_start),
//                            p->buffer_end - p->buffer);
//         p->buffer += offset;
//         break;
//     case 2:
//         if (offset > 0)
//             p->eof = 1;
//         offset = av_clip_c(offset, -(p->buffer_end - p->buffer_start), 0);
//         p->buffer = p->buffer_end + offset;
//         break;
//     case 0:
//         if (p->buffer_end - p->buffer_start < offset)
//             p->eof = 1;
//         offset = av_clip_c(offset, 0, p->buffer_end - p->buffer_start);
//         p->buffer = p->buffer_start + offset;
//         break;
//     default:
//         return (-(
//             22));
//     }
//     return bytestream2_tell_p(p);
// }

// static inline unsigned int bytestream2_get_buffer(GetByteContext *g,
//                                                   uint8_t *dst,
//                                                   unsigned int size)
// {
//     int size2 = ((g->buffer_end - g->buffer) > (size) ? (size) : (g->buffer_end - g->buffer));
//     memcpy(dst, g->buffer, size2);
//     g->buffer += size2;
//     return size2;
// }

// static inline unsigned int bytestream2_get_bufferu(GetByteContext *g,
//                                                    uint8_t *dst,
//                                                    unsigned int size)
// {
//     memcpy(dst, g->buffer, size);
//     g->buffer += size;
//     return size;
// }

// static inline unsigned int bytestream2_put_buffer(PutByteContext *p,
//                                                   const uint8_t *src,
//                                                   unsigned int size)
// {
//     int size2;
//     if (p->eof)
//         return 0;
//     size2 = ((p->buffer_end - p->buffer) > (size) ? (size) : (p->buffer_end - p->buffer));
//     if (size2 != size)
//         p->eof = 1;
//     memcpy(p->buffer, src, size2);
//     p->buffer += size2;
//     return size2;
// }

// static inline unsigned int bytestream2_put_bufferu(PutByteContext *p,
//                                                    const uint8_t *src,
//                                                    unsigned int size)
// {
//     memcpy(p->buffer, src, size);
//     p->buffer += size;
//     return size;
// }

// static inline void bytestream2_set_buffer(PutByteContext *p,
//                                           const uint8_t c,
//                                           unsigned int size)
// {
//     int size2;
//     if (p->eof)
//         return;
//     size2 = ((p->buffer_end - p->buffer) > (size) ? (size) : (p->buffer_end - p->buffer));
//     if (size2 != size)
//         p->eof = 1;
//     memset(p->buffer, c, size2);
//     p->buffer += size2;
// }

// static inline void bytestream2_set_bufferu(PutByteContext *p,
//                                            const uint8_t c,
//                                            unsigned int size)
// {
//     memset(p->buffer, c, size);
//     p->buffer += size;
// }

// static inline unsigned int bytestream2_get_eof(PutByteContext *p)
// {
//     return p->eof;
// }

// static inline unsigned int bytestream2_copy_bufferu(PutByteContext *p,
//                                                     GetByteContext *g,
//                                                     unsigned int size)
// {
//     memcpy(p->buffer, g->buffer, size);
//     p->buffer += size;
//     g->buffer += size;
//     return size;
// }

// static inline unsigned int bytestream2_copy_buffer(PutByteContext *p,
//                                                    GetByteContext *g,
//                                                    unsigned int size)
// {
//     int size2;
//     if (p->eof)
//         return 0;
//     size = ((g->buffer_end - g->buffer) > (size) ? (size) : (g->buffer_end - g->buffer));
//     size2 = ((p->buffer_end - p->buffer) > (size) ? (size) : (p->buffer_end - p->buffer));
//     if (size2 != size)
//         p->eof = 1;
//     return bytestream2_copy_bufferu(p, g, size2);
// }

// static inline unsigned int bytestream_get_buffer(const uint8_t **b,
//                                                  uint8_t *dst,
//                                                  unsigned int size)
// {
//     memcpy(dst, *b, size);
//     (*b) += size;
//     return size;
// }

// static inline void bytestream_put_buffer(uint8_t **b,
//                                          const uint8_t *src,
//                                          unsigned int size)
// {
//     memcpy(*b, src, size);
//     (*b) += size;
// }

// typedef uint64_t BitBuf;

// static const int BUF_BITS = 8 * sizeof(BitBuf);

// typedef struct PutBitContext
// {
//     BitBuf bit_buf;
//     int bit_left;
//     uint8_t *buf, *buf_ptr, *buf_end;
//     int size_in_bits;
// } PutBitContext;

// static inline void init_put_bits(PutBitContext *s, uint8_t *buffer,
//                                  int buffer_size)
// {
//     if (buffer_size < 0)
//     {
//         buffer_size = 0;
//         buffer =
//             ((void *)0);
//     }
//     s->size_in_bits = 8 * buffer_size;
//     s->buf = buffer;
//     s->buf_end = s->buf + buffer_size;
//     s->buf_ptr = s->buf;
//     s->bit_left = BUF_BITS;
//     s->bit_buf = 0;
// }

// static inline int put_bits_count(PutBitContext *s)
// {
//     return (s->buf_ptr - s->buf) * 8 + BUF_BITS - s->bit_left;
// }
// static inline void rebase_put_bits(PutBitContext *s, uint8_t *buffer,
//                                    int buffer_size)
// {
//     do
//     {
//         if (!(8 * buffer_size >= put_bits_count(s)))
//         {
//             av_log(
//                 ((void *)0), 0, "Assertion %s failed at %s:%d\n", "8*buffer_size >= put_bits_count(s)", "C:\\FFmpeg\\libavcodec\\put_bits.h", 96);
//             abort();
//         }
//     } while (0);
//     s->buf_end = buffer + buffer_size;
//     s->buf_ptr = buffer + (s->buf_ptr - s->buf);
//     s->buf = buffer;
//     s->size_in_bits = 8 * buffer_size;
// }

// static inline int put_bits_left(PutBitContext *s)
// {
//     return (s->buf_end - s->buf_ptr) * 8 - BUF_BITS + s->bit_left;
// }

// static inline void flush_put_bits(PutBitContext *s)
// {
//     if (s->bit_left < BUF_BITS)
//         s->bit_buf <<= s->bit_left;
//     while (s->bit_left < BUF_BITS)
//     {
//         do
//         {
//             if (!(s->buf_ptr < s->buf_end))
//             {
//                 av_log(
//                     ((void *)0), 0, "Assertion %s failed at %s:%d\n", "s->buf_ptr < s->buf_end", "C:\\FFmpeg\\libavcodec\\put_bits.h", 122);
//                 abort();
//             }
//         } while (0);
//         *s->buf_ptr++ = s->bit_buf >> (BUF_BITS - 8);
//         s->bit_buf <<= 8;
//         s->bit_left += 8;
//     }
//     s->bit_left = BUF_BITS;
//     s->bit_buf = 0;
// }

// static inline void flush_put_bits_le(PutBitContext *s)
// {
//     while (s->bit_left < BUF_BITS)
//     {
//         do
//         {
//             if (!(s->buf_ptr < s->buf_end))
//             {
//                 av_log(
//                     ((void *)0), 0, "Assertion %s failed at %s:%d\n", "s->buf_ptr < s->buf_end", "C:\\FFmpeg\\libavcodec\\put_bits.h", 139);
//                 abort();
//             }
//         } while (0);
//         *s->buf_ptr++ = s->bit_buf;
//         s->bit_buf >>= 8;
//         s->bit_left += 8;
//     }
//     s->bit_left = BUF_BITS;
//     s->bit_buf = 0;
// }

// static inline void put_bits_no_assert(PutBitContext *s, int n, BitBuf value)
// {
//     BitBuf bit_buf;
//     int bit_left;
//     bit_buf = s->bit_buf;
//     bit_left = s->bit_left;
//     if (n < bit_left)
//     {
//         bit_buf = (bit_buf << n) | value;
//         bit_left -= n;
//     }
//     else
//     {
//         bit_buf <<= bit_left;
//         bit_buf |= value >> (n - bit_left);
//         if (s->buf_end - s->buf_ptr >= sizeof(BitBuf))
//         {
//             ((((union unaligned_64 *)(s->buf_ptr))->l) = (av_bswap64(bit_buf)));
//             s->buf_ptr += sizeof(BitBuf);
//         }
//         else
//         {
//             av_log(
//                 ((void *)0), 16, "Internal error, put_bits buffer too small\n");
//             ((void)0);
//         }
//         bit_left += BUF_BITS - n;
//         bit_buf = value;
//     }
//     s->bit_buf = bit_buf;
//     s->bit_left = bit_left;
// }

// static inline void put_bits(PutBitContext *s, int n, BitBuf value)
// {
//     ((void)0);
//     put_bits_no_assert(s, n, value);
// }

// static inline void put_bits_le(PutBitContext *s, int n, BitBuf value)
// {
//     BitBuf bit_buf;
//     int bit_left;
//     ((void)0);
//     bit_buf = s->bit_buf;
//     bit_left = s->bit_left;
//     bit_buf |= value << (BUF_BITS - bit_left);
//     if (n >= bit_left)
//     {
//         if (s->buf_end - s->buf_ptr >= sizeof(BitBuf))
//         {
//             ((((union unaligned_64 *)(s->buf_ptr))->l) = (bit_buf));
//             s->buf_ptr += sizeof(BitBuf);
//         }
//         else
//         {
//             av_log(
//                 ((void *)0), 16, "Internal error, put_bits buffer too small\n");
//             ((void)0);
//         }
//         bit_buf = value >> bit_left;
//         bit_left += BUF_BITS;
//     }
//     bit_left -= n;
//     s->bit_buf = bit_buf;
//     s->bit_left = bit_left;
// }

// static inline void put_sbits(PutBitContext *pb, int n, int32_t value)
// {
//     ((void)0);
//     put_bits(pb, n, av_mod_uintp2_c(value, n));
// }

// static void put_bits32(PutBitContext *s, uint32_t value)
// {
//     BitBuf bit_buf;
//     int bit_left;
//     if (BUF_BITS > 32)
//     {
//         put_bits_no_assert(s, 32, value);
//         return;
//     }
//     bit_buf = s->bit_buf;
//     bit_left = s->bit_left;
//     bit_buf = (uint64_t)bit_buf << bit_left;
//     bit_buf |= (BitBuf)value >> (BUF_BITS - bit_left);
//     if (s->buf_end - s->buf_ptr >= sizeof(BitBuf))
//     {
//         ((((union unaligned_64 *)(s->buf_ptr))->l) = (av_bswap64(bit_buf)));
//         s->buf_ptr += sizeof(BitBuf);
//     }
//     else
//     {
//         av_log(
//             ((void *)0), 16, "Internal error, put_bits buffer too small\n");
//         ((void)0);
//     }
//     bit_buf = value;
//     s->bit_buf = bit_buf;
//     s->bit_left = bit_left;
// }

// static inline void put_bits64(PutBitContext *s, int n, uint64_t value)
// {
//     ((void)0);
//     if (n < 32)
//         put_bits(s, n, value);
//     else if (n == 32)
//         put_bits32(s, value);
//     else if (n < 64)
//     {
//         uint32_t lo = value & 0xffffffff;
//         uint32_t hi = value >> 32;
//         put_bits(s, n - 32, hi);
//         put_bits32(s, lo);
//     }
//     else
//     {
//         uint32_t lo = value & 0xffffffff;
//         uint32_t hi = value >> 32;
//         put_bits32(s, hi);
//         put_bits32(s, lo);
//     }
// }

// static inline uint8_t *put_bits_ptr(PutBitContext *s)
// {
//     return s->buf_ptr;
// }

// static inline void skip_put_bytes(PutBitContext *s, int n)
// {
//     ((void)0);
//     ((void)0);
//     do
//     {
//         if (!(n <= s->buf_end - s->buf_ptr))
//         {
//             av_log(
//                 ((void *)0), 0, "Assertion %s failed at %s:%d\n", "n <= s->buf_end - s->buf_ptr", "C:\\FFmpeg\\libavcodec\\put_bits.h", 360);
//             abort();
//         }
//     } while (0);
//     s->buf_ptr += n;
// }

// static inline void skip_put_bits(PutBitContext *s, int n)
// {
//     unsigned bits = BUF_BITS - s->bit_left + n;
//     s->buf_ptr += sizeof(BitBuf) * (bits / BUF_BITS);
//     s->bit_left = BUF_BITS - (bits & (BUF_BITS - 1));
// }

// static inline void set_put_bits_buffer_size(PutBitContext *s, int size)
// {
//     do
//     {
//         if (!(size <= 0x7fffffff / 8 - BUF_BITS))
//         {
//             av_log(
//                 ((void *)0), 0, "Assertion %s failed at %s:%d\n", "size <= 0x7fffffff/8 - BUF_BITS", "C:\\FFmpeg\\libavcodec\\put_bits.h", 383);
//             abort();
//         }
//     } while (0);
//     s->buf_end = s->buf + size;
//     s->size_in_bits = 8 * size;
// }

// extern const uint8_t ff_h264_cabac_tables[512 + 4 * 2 * 64 + 4 * 64 + 63];

// typedef struct CABACContext
// {
//     int low;
//     int range;
//     int outstanding_count;
//     const uint8_t *bytestream_start;
//     const uint8_t *bytestream;
//     const uint8_t *bytestream_end;
//     PutBitContext pb;
// } CABACContext;

// typedef struct xmm_reg
// {
//     uint64_t a, b;
// } xmm_reg;
// typedef struct ymm_reg
// {
//     uint64_t a, b, c, d;
// } ymm_reg;

// typedef int64_t x86_reg;

// static inline int get_cabac_inline_x86(CABACContext *c,
//                                        uint8_t *const state)
// {
//     int bit, tmp;
//     __asm__ volatile(
//         "movzbl "
//         "(%4)"
//         "    , "
//         "%0"
//         "                                     \n\t"
//         "mov    "
//         "%2"
//         "     , "
//         "%3"
//         "                                     \n\t"
//         "and    $0xC0       , "
//         "%2"
//         "                                   \n\t"
//         "movzbl "
//         ""
//         "ff_h264_cabac_tables"
//         "+"
//         "512"
//         "("
//         "%0"
//         ", "
//         "%2"
//         ", 2), "
//         "%2"
//         " \n\t"
//         "sub    "
//         "%2"
//         "     , "
//         "%3"
//         "                                     \n\t"
//         "mov    "
//         "%3"
//         "       , %%ecx     \n\t"
//         "shl    $17         , "
//         "%3"
//         "     \n\t"
//         "cmp    "
//         "%1"
//         "       , "
//         "%3"
//         "     \n\t"
//         "cmova  %%ecx       , "
//         "%2"
//         "   \n\t"
//         "sbb    %%ecx       , %%ecx     \n\t"
//         "and    %%ecx       , "
//         "%3"
//         "     \n\t"
//         "xor    %%ecx       , "
//         "%0"
//         "     \n\t"
//         "sub    "
//         "%3"
//         "       , "
//         "%1"
//         "     \n\t"
//         "movzbl "
//         ""
//         "ff_h264_cabac_tables"
//         "+"
//         "0"
//         "("
//         "%2"
//         "), %%ecx    \n\t"
//         "shl    %%cl        , "
//         "%2"
//         "                                   \n\t"
//         "movzbl "
//         ""
//         "ff_h264_cabac_tables"
//         "+"
//         "1024"
//         "+128("
//         "%0"
//         "), "
//         "%3"
//         "  \n\t"
//         "shl    %%cl        , "
//         "%1"
//         "                                     \n\t"
//         "mov    "
//         "%b3"
//         "   , "
//         "(%4)"
//         "                                  \n\t"
//         "test   "
//         "%w1"
//         "   , "
//         "%w1"
//         "                                 \n\t"
//         " jnz   2f                                                      \n\t"
//         "mov    "
//         "%c6(%5)"
//         "      , %%"
//         "rcx"
//         "                              \n\t"
//         ""
//         "add"
//         "q"
//         " $2  , "
//         "%c6(%5)"
//         "                                    \n\t"
//         "1:                                                             \n\t"
//         "movzwl (%%"
//         "rcx"
//         ") , "
//         "%3"
//         "                                  \n\t"
//         "lea    -1("
//         "%1"
//         ")   , %%ecx                                     \n\t"
//         "xor    "
//         "%1"
//         "       , %%ecx                                     \n\t"
//         "shr    $15         , %%ecx                                     \n\t"
//         "bswap  "
//         "%3"
//         "                                                   \n\t"
//         "shr    $15         , "
//         "%3"
//         "                                     \n\t"
//         "movzbl "
//         ""
//         "ff_h264_cabac_tables"
//         "+"
//         "0"
//         "(%%ecx), %%ecx \n\t"
//         "sub    $0xFFFF     , "
//         "%3"
//         "                                     \n\t"
//         "neg    %%ecx                                                   \n\t"
//         "add    $7          , %%ecx                                     \n\t"
//         "shl    %%cl        , "
//         "%3"
//         "                                     \n\t"
//         "add    "
//         "%3"
//         "       , "
//         "%1"
//         "                                     \n\t"
//         "2:                                                             \n\t"
//         : "=&r"(bit), "=&r"(c->low), "=&r"(c->range), "=&q"(tmp)
//         : "r"(state), "r"(c),
//           "i"(
//               __builtin_offsetof(
//                   CABACContext,
//                   bytestream)),
//           "i"(
//               __builtin_offsetof(
//                   CABACContext,
//                   bytestream_end))

//               ,
//           "1"(c->low), "2"(c->range)
//         : "%"
//           "rcx",
//           "memory");
//     return bit & 1;
// }

// static inline int get_cabac_bypass_sign_x86(CABACContext *c, int val)
// {
//     x86_reg tmp;
//     __asm__ volatile(
//         "movl        %c6(%2), %k1       \n\t"
//         "movl        %c3(%2), %%eax     \n\t"
//         "shl             $17, %k1       \n\t"
//         "add           %%eax, %%eax     \n\t"
//         "sub             %k1, %%eax     \n\t"
//         "cdq                            \n\t"
//         "and           %%edx, %k1       \n\t"
//         "add             %k1, %%eax     \n\t"
//         "xor           %%edx, %%ecx     \n\t"
//         "sub           %%edx, %%ecx     \n\t"
//         "test           %%ax, %%ax      \n\t"
//         "jnz              1f            \n\t"
//         "mov         %c4(%2), %1        \n\t"
//         "subl        $0xFFFF, %%eax     \n\t"
//         "movzwl         (%1), %%edx     \n\t"
//         "bswap         %%edx            \n\t"
//         "shrl            $15, %%edx     \n\t"
//         "add              $2, %1        \n\t"
//         "addl          %%edx, %%eax     \n\t"
//         "mov              %1, %c4(%2)   \n\t"
//         "1:                             \n\t"
//         "movl          %%eax, %c3(%2)   \n\t"
//         : "+c"(val), "=&r"(tmp)
//         : "r"(c),
//           "i"(
//               __builtin_offsetof(
//                   CABACContext,
//                   low)),
//           "i"(
//               __builtin_offsetof(
//                   CABACContext,
//                   bytestream)),
//           "i"(
//               __builtin_offsetof(
//                   CABACContext,
//                   bytestream_end)),
//           "i"(
//               __builtin_offsetof(
//                   CABACContext,
//                   range))
//         : "%eax", "%edx", "memory");
//     return val;
// }

// static inline int get_cabac_bypass_x86(CABACContext *c)
// {
//     x86_reg tmp;
//     int res;
//     __asm__ volatile(
//         "movl        %c6(%2), %k1       \n\t"
//         "movl        %c3(%2), %%eax     \n\t"
//         "shl             $17, %k1       \n\t"
//         "add           %%eax, %%eax     \n\t"
//         "sub             %k1, %%eax     \n\t"
//         "cdq                            \n\t"
//         "and           %%edx, %k1       \n\t"
//         "add             %k1, %%eax     \n\t"
//         "inc           %%edx            \n\t"
//         "test           %%ax, %%ax      \n\t"
//         "jnz              1f            \n\t"
//         "mov         %c4(%2), %1        \n\t"
//         "subl        $0xFFFF, %%eax     \n\t"
//         "movzwl         (%1), %%ecx     \n\t"
//         "bswap         %%ecx            \n\t"
//         "shrl            $15, %%ecx     \n\t"
//         "addl          %%ecx, %%eax     \n\t"
//         "cmp         %c5(%2), %1        \n\t"
//         "jge              1f            \n\t"
//         "add"
//         "q"
//         "   $2, %c4(%2)   \n\t"
//         "1:                             \n\t"
//         "movl          %%eax, %c3(%2)   \n\t"
//         : "=&d"(res), "=&r"(tmp)
//         : "r"(c),
//           "i"(
//               __builtin_offsetof(
//                   CABACContext,
//                   low)),
//           "i"(
//               __builtin_offsetof(
//                   CABACContext,
//                   bytestream)),
//           "i"(
//               __builtin_offsetof(
//                   CABACContext,
//                   bytestream_end)),
//           "i"(
//               __builtin_offsetof(
//                   CABACContext,
//                   range))
//         : "%eax", "%ecx", "memory");
//     return res;
// }

// static void refill(CABACContext *c)
// {
//     c->low += (c->bytestream[0] << 9) + (c->bytestream[1] << 1);
//     c->low -= ((1 << 16) - 1);
//     c->bytestream += 16 / 8;
// }

// static inline void renorm_cabac_decoder_once(CABACContext *c)
// {
//     int shift = (uint32_t)(c->range - 0x100) >> 31;
//     c->range <<= shift;
//     c->low <<= shift;
//     if (!(c->low & ((1 << 16) - 1)))
//         refill(c);
// }
// static int get_cabac_noinline(CABACContext *c, uint8_t *const state)
// {
//     return get_cabac_inline_x86(c, state);
// }

// static int get_cabac(CABACContext *c, uint8_t *const state)
// {
//     return get_cabac_inline_x86(c, state);
// }
// static int get_cabac_terminate(CABACContext *c)
// {
//     c->range -= 2;
//     if (c->low < c->range << (16 + 1))
//     {
//         renorm_cabac_decoder_once(c);
//         return 0;
//     }
//     else
//     {
//         return c->bytestream - c->bytestream_start;
//     }
// }

// static const uint8_t *skip_bytes(CABACContext *c, int n)
// {
//     const uint8_t *ptr = c->bytestream;
//     if (c->low & 0x1)
//         ptr--;
//     if (c->low & 0x1FF)
//         ptr--;
//     if ((int)(c->bytestream_end - ptr) < n)
//         return ((void *)0);
//     if (ff_init_cabac_decoder(c, ptr + n, c->bytestream_end - ptr - n) < 0)
//         return ((void *)0);
//     return ptr;
// }

// typedef struct MECmpContext
// {
//     int (*sum_abs_dctelem)(int16_t *block);
//     me_cmp_func sad[6];
//     me_cmp_func sse[6];
//     me_cmp_func hadamard8_diff[6];
//     me_cmp_func dct_sad[6];
//     me_cmp_func quant_psnr[6];
//     me_cmp_func bit[6];
//     me_cmp_func rd[6];
//     me_cmp_func vsad[6];
//     me_cmp_func vsse[6];
//     me_cmp_func nsse[6];
//     me_cmp_func w53[6];
//     me_cmp_func w97[6];
//     me_cmp_func dct_max[6];
//     me_cmp_func dct264_sad[6];
//     me_cmp_func me_pre_cmp[6];
//     me_cmp_func me_cmp[6];
//     me_cmp_func me_sub_cmp[6];
//     me_cmp_func mb_cmp[6];
//     me_cmp_func ildct_cmp[6];
//     me_cmp_func frame_skip_cmp[6];
//     me_cmp_func pix_abs[2][4];
//     me_cmp_func median_sad[6];
// } MECmpContext;

// typedef struct ThreadFrame
// {
//     AVFrame *f;
//     AVCodecContext *owner[2];
//     AVBufferRef *progress;
// } ThreadFrame;

// typedef struct ERPicture
// {
//     AVFrame *f;
//     ThreadFrame *tf;
//     int16_t (*motion_val[2])[2];
//     int8_t *ref_index[2];
//     uint32_t *mb_type;
//     int field_picture;
// } ERPicture;

// typedef struct ERContext
// {
//     AVCodecContext *avctx;
//     MECmpContext mecc;
//     int mecc_inited;
//     int *mb_index2xy;
//     int mb_num;
//     int mb_width, mb_height;
//     ptrdiff_t mb_stride;
//     ptrdiff_t b8_stride;
//     atomic_int error_count;
//     int error_occurred;
//     uint8_t *error_status_table;
//     uint8_t *er_temp_buffer;
//     int16_t *dc_val[3];
//     uint8_t *mbskip_table;
//     uint8_t *mbintra_table;
//     int mv[2][4][2];
//     ERPicture cur_pic;
//     ERPicture last_pic;
//     ERPicture next_pic;
//     AVBufferRef *ref_index_buf[2];
//     AVBufferRef *motion_val_buf[2];
//     uint16_t pp_time;
//     uint16_t pb_time;
//     int quarter_sample;
//     int partitioned_frame;
//     int ref_count;
//     void (*decode_mb)(void *opaque, int ref, int mv_dir, int mv_type,
//                       int (*mv)[2][4][2],
//                       int mb_x, int mb_y, int mb_intra, int mb_skipped);
//     void *opaque;
// } ERContext;

// void ff_er_frame_start(ERContext *s);
// void ff_er_frame_end(ERContext *s);
// void ff_er_add_slice(ERContext *s, int startx, int starty, int endx, int endy,
//                      int status);

// enum
// {
//     H264_NAL_UNSPECIFIED = 0,
//     H264_NAL_SLICE = 1,
//     H264_NAL_DPA = 2,
//     H264_NAL_DPB = 3,
//     H264_NAL_DPC = 4,
//     H264_NAL_IDR_SLICE = 5,
//     H264_NAL_SEI = 6,
//     H264_NAL_SPS = 7,
//     H264_NAL_PPS = 8,
//     H264_NAL_AUD = 9,
//     H264_NAL_END_SEQUENCE = 10,
//     H264_NAL_END_STREAM = 11,
//     H264_NAL_FILLER_DATA = 12,
//     H264_NAL_SPS_EXT = 13,
//     H264_NAL_PREFIX = 14,
//     H264_NAL_SUB_SPS = 15,
//     H264_NAL_DPS = 16,
//     H264_NAL_RESERVED17 = 17,
//     H264_NAL_RESERVED18 = 18,
//     H264_NAL_AUXILIARY_SLICE = 19,
//     H264_NAL_EXTEN_SLICE = 20,
//     H264_NAL_DEPTH_EXTEN_SLICE = 21,
//     H264_NAL_RESERVED22 = 22,
//     H264_NAL_RESERVED23 = 23,
//     H264_NAL_UNSPECIFIED24 = 24,
//     H264_NAL_UNSPECIFIED25 = 25,
//     H264_NAL_UNSPECIFIED26 = 26,
//     H264_NAL_UNSPECIFIED27 = 27,
//     H264_NAL_UNSPECIFIED28 = 28,
//     H264_NAL_UNSPECIFIED29 = 29,
//     H264_NAL_UNSPECIFIED30 = 30,
//     H264_NAL_UNSPECIFIED31 = 31,
// };

// enum
// {
//     H264_MAX_SPS_COUNT = 32,
//     H264_MAX_PPS_COUNT = 256,
//     H264_MAX_DPB_FRAMES = 16,
//     H264_MAX_REFS = 2 * H264_MAX_DPB_FRAMES,
//     H264_MAX_RPLM_COUNT = H264_MAX_REFS + 1,
//     H264_MAX_MMCO_COUNT = H264_MAX_REFS * 2 + 3,
//     H264_MAX_SLICE_GROUPS = 8,
//     H264_MAX_CPB_CNT = 32,
//     H264_MAX_MB_PIC_SIZE = 139264,
//     H264_MAX_MB_WIDTH = 1055,
//     H264_MAX_MB_HEIGHT = 1055,
//     H264_MAX_WIDTH = H264_MAX_MB_WIDTH * 16,
//     H264_MAX_HEIGHT = H264_MAX_MB_HEIGHT * 16,
// };

// extern const uint8_t ff_reverse[256];

// extern const uint32_t ff_inverse[257];
// extern const uint8_t ff_sqrt_tab[256];
// extern const uint8_t ff_crop_tab[256 + 2 * 1024];
// extern const uint8_t ff_zigzag_direct[64];
// extern const uint8_t ff_zigzag_scan[16 + 1];

// static inline int mid_pred(int a, int b, int c)
// {
//     int i = b;
//     __asm__(
//         "cmp    %2, %1 \n\t"
//         "cmovg  %1, %0 \n\t"
//         "cmovg  %2, %1 \n\t"
//         "cmp    %3, %1 \n\t"
//         "cmovl  %3, %1 \n\t"
//         "cmp    %1, %0 \n\t"
//         "cmovg  %1, %0 \n\t"
//         : "+&r"(i), "+&r"(a)
//         : "r"(b), "r"(c));
//     return i;
// }

// static inline int32_t NEG_SSR32(int32_t a, int8_t s)
// {
//     __asm__("sarl %1, %0\n\t"
//             : "+r"(a)
//             : "ic"((uint8_t)(-s)));
//     return a;
// }

// static inline uint32_t NEG_USR32(uint32_t a, int8_t s)
// {
//     __asm__("shrl %1, %0\n\t"
//             : "+r"(a)
//             : "ic"((uint8_t)(-s)));
//     return a;
// }

// static inline int MULH(int a, int b)
// {
//     return ((int64_t)(a) * (int64_t)(b)) >> 32;
// }

// static inline unsigned UMULH(unsigned a, unsigned b)
// {
//     return ((uint64_t)(a) * (uint64_t)(b)) >> 32;
// }

// static inline int median4(int a, int b, int c, int d)
// {
//     if (a < b)
//     {
//         if (c < d)
//             return (((b) > (d) ? (d) : (b)) + ((a) > (c) ? (a) : (c))) / 2;
//         else
//             return (((b) > (c) ? (c) : (b)) + ((a) > (d) ? (a) : (d))) / 2;
//     }
//     else
//     {
//         if (c < d)
//             return (((a) > (d) ? (d) : (a)) + ((b) > (c) ? (b) : (c))) / 2;
//         else
//             return (((a) > (c) ? (c) : (a)) + ((b) > (d) ? (b) : (d))) / 2;
//     }
// }

// static inline int sign_extend(int val, unsigned bits)
// {
//     unsigned shift = 8 * sizeof(int) - bits;
//     union
//     {
//         unsigned u;
//         int s;
//     } v = {(unsigned)val << shift};
//     return v.s >> shift;
// }

// static inline unsigned zero_extend(unsigned val, unsigned bits)
// {
//     return (val << ((8 * sizeof(int)) - bits)) >> ((8 * sizeof(int)) - bits);
// }

// static inline unsigned int ff_sqrt(unsigned int a)
// {
//     unsigned int b;
//     if (a < 255)
//         return (ff_sqrt_tab[a + 1] - 1) >> 4;
//     else if (a < (1 << 12))
//         b = ff_sqrt_tab[a >> 4] >> 2;
//     else if (a < (1 << 14))
//         b = ff_sqrt_tab[a >> 6] >> 1;
//     else if (a < (1 << 16))
//         b = ff_sqrt_tab[a >> 8];
//     else
//     {
//         int s = av_log2_16bit(a >> 16) >> 1;
//         unsigned int c = a >> (s + 2);
//         b = ff_sqrt_tab[c >> (s + 8)];
//         b = ((uint32_t)((((uint64_t)c) * ff_inverse[b]) >> 32)) + (b << s);
//     }
//     return b - (a < b * b);
// }

// static inline float ff_sqrf(float a)
// {
//     return a * a;
// }

// static inline int8_t ff_u8_to_s8(uint8_t a)
// {
//     union
//     {
//         uint8_t u8;
//         int8_t s8;
//     } b;
//     b.u8 = a;
//     return b.s8;
// }

// static inline uint32_t bitswap_32(uint32_t x)
// {
//     return (uint32_t)ff_reverse[x & 0xFF] << 24 |
//            (uint32_t)ff_reverse[(x >> 8) & 0xFF] << 16 |
//            (uint32_t)ff_reverse[(x >> 16) & 0xFF] << 8 |
//            (uint32_t)ff_reverse[x >> 24];
// }

// typedef struct VLC
// {
//     int bits;
//     int16_t (*table)[2];
//     int table_size, table_allocated;
// } VLC;

// typedef struct RL_VLC_ELEM
// {
//     int16_t level;
//     int8_t len;
//     uint8_t run;
// } RL_VLC_ELEM;

// int ff_init_vlc_sparse(VLC *vlc, int nb_bits, int nb_codes,
//                        const void *bits, int bits_wrap, int bits_size,
//                        const void *codes, int codes_wrap, int codes_size,
//                        const void *symbols, int symbols_wrap, int symbols_size,
//                        int flags);
// void ff_free_vlc(VLC *vlc);

// typedef struct GetBitContext
// {
//     const uint8_t *buffer, *buffer_end;
//     int index;
//     int size_in_bits;
//     int size_in_bits_plus8;
// } GetBitContext;

// static inline unsigned int get_bits(GetBitContext *s, int n);
// static inline void skip_bits(GetBitContext *s, int n);
// static inline unsigned int show_bits(GetBitContext *s, int n);

// static inline int get_bits_count(const GetBitContext *s)
// {
//     return s->index;
// }
// static inline void skip_bits_long(GetBitContext *s, int n)
// {
//     s->index += n;
// }
// static inline int get_xbits(GetBitContext *s, int n)
// {
//     register int sign;
//     register int32_t cache;
//     unsigned int re_index = (s)->index;
//     unsigned int re_cache;
//     ((void)0);
//     re_cache = av_bswap32((((const union unaligned_32 *)((s)->buffer + (re_index >> 3)))->l)) << (re_index & 7);
//     cache = ((uint32_t)re_cache);
//     sign = ~cache >> 31;
//     re_index += (n);
//     (s)->index = re_index;
//     return (NEG_USR32(sign ^ cache, n) ^ sign) - sign;
// }

// static inline int get_xbits_le(GetBitContext *s, int n)
// {
//     register int sign;
//     register int32_t cache;
//     unsigned int re_index = (s)->index;
//     unsigned int re_cache;
//     ((void)0);
//     re_cache = (((const union unaligned_32 *)((s)->buffer + (re_index >> 3)))->l) >> (re_index & 7);
//     cache = ((uint32_t)re_cache);
//     sign = sign_extend(~cache, n) >> 31;
//     re_index += (n);
//     (s)->index = re_index;
//     return (zero_extend(sign ^ cache, n) ^ sign) - sign;
// }

// static inline int get_sbits(GetBitContext *s, int n)
// {
//     register int tmp;
//     unsigned int re_index = (s)->index;
//     unsigned int re_cache;
//     ((void)0);
//     re_cache = av_bswap32((((const union unaligned_32 *)((s)->buffer + (re_index >> 3)))->l)) << (re_index & 7);
//     tmp = NEG_SSR32(re_cache, n);
//     re_index += (n);
//     (s)->index = re_index;
//     return tmp;
// }

// static inline unsigned int get_bits(GetBitContext *s, int n)
// {
//     register unsigned int tmp;
//     unsigned int re_index = (s)->index;
//     unsigned int re_cache;
//     ((void)0);
//     re_cache = av_bswap32((((const union unaligned_32 *)((s)->buffer + (re_index >> 3)))->l)) << (re_index & 7);
//     tmp = NEG_USR32(re_cache, n);
//     re_index += (n);
//     (s)->index = re_index;
//     ((void)0);
//     return tmp;
// }

// static inline int get_bitsz(GetBitContext *s, int n)
// {
//     return n ? get_bits(s, n) : 0;
// }

// static inline unsigned int get_bits_le(GetBitContext *s, int n)
// {
//     register int tmp;
//     unsigned int re_index = (s)->index;
//     unsigned int re_cache;
//     ((void)0);
//     re_cache = (((const union unaligned_32 *)((s)->buffer + (re_index >> 3)))->l) >> (re_index & 7);
//     tmp = zero_extend(re_cache, n);
//     re_index += (n);
//     (s)->index = re_index;
//     return tmp;
// }

// static inline unsigned int show_bits(GetBitContext *s, int n)
// {
//     register unsigned int tmp;
//     unsigned int re_index = (s)->index;
//     unsigned int re_cache;
//     ((void)0);
//     re_cache = av_bswap32((((const union unaligned_32 *)((s)->buffer + (re_index >> 3)))->l)) << (re_index & 7);
//     tmp = NEG_USR32(re_cache, n);
//     return tmp;
// }

// static inline void skip_bits(GetBitContext *s, int n)
// {
//     unsigned int re_index = (s)->index;
//     unsigned int re_cache;
//     re_index += (n);
//     (s)->index = re_index;
// }

// static inline unsigned int get_bits1(GetBitContext *s)
// {
//     unsigned int index = s->index;
//     uint8_t result = s->buffer[index >> 3];
//     result <<= index & 7;
//     result >>= 8 - 1;
//     index++;
//     s->index = index;
//     return result;
// }

// static inline unsigned int show_bits1(GetBitContext *s)
// {
//     return show_bits(s, 1);
// }

// static inline void skip_bits1(GetBitContext *s)
// {
//     skip_bits(s, 1);
// }

// static inline unsigned int get_bits_long(GetBitContext *s, int n)
// {
//     ((void)0);
//     if (!n)
//     {
//         return 0;
//     }
//     else if (n <= 25)
//     {
//         return get_bits(s, n);
//     }
//     else
//     {
//         unsigned ret = get_bits(s, 16) << (n - 16);
//         return ret | get_bits(s, n - 16);
//     }
// }

// static inline uint64_t get_bits64(GetBitContext *s, int n)
// {
//     if (n <= 32)
//     {
//         return get_bits_long(s, n);
//     }
//     else
//     {
//         uint64_t ret = (uint64_t)get_bits_long(s, n - 32) << 32;
//         return ret | get_bits_long(s, 32);
//     }
// }

// static inline int get_sbits_long(GetBitContext *s, int n)
// {
//     if (!n)
//         return 0;
//     return sign_extend(get_bits_long(s, n), n);
// }

// static inline unsigned int show_bits_long(GetBitContext *s, int n)
// {
//     if (n <= 25)
//     {
//         return show_bits(s, n);
//     }
//     else
//     {
//         GetBitContext gb = *s;
//         return get_bits_long(&gb, n);
//     }
// }

// static inline int check_marker(void *logctx, GetBitContext *s, const char *msg)
// {
//     int bit = get_bits1(s);
//     if (!bit)
//         av_log(logctx, 32, "Marker bit missing at %d of %d %s\n",
//                get_bits_count(s) - 1, s->size_in_bits, msg);
//     return bit;
// }

// static inline int init_get_bits_xe(GetBitContext *s, const uint8_t *buffer,
//                                    int bit_size, int is_le)
// {
//     int buffer_size;
//     int ret = 0;
//     if (bit_size >= 0x7fffffff - ((7) > (64 * 8) ? (7) : (64 * 8)) || bit_size < 0 || !buffer)
//     {
//         bit_size = 0;
//         buffer =
//             ((void *)0);
//         ret = (-(int)(('I') | (('N') << 8) | (('D') << 16) | ((unsigned)('A') << 24)));
//     }
//     buffer_size = (bit_size + 7) >> 3;
//     s->buffer = buffer;
//     s->size_in_bits = bit_size;
//     s->size_in_bits_plus8 = bit_size + 8;
//     s->buffer_end = buffer + buffer_size;
//     s->index = 0;
//     return ret;
// }
// static inline int init_get_bits(GetBitContext *s, const uint8_t *buffer,
//                                 int bit_size)
// {
//     return init_get_bits_xe(s, buffer, bit_size, 0);
// }
// static inline int init_get_bits8(GetBitContext *s, const uint8_t *buffer,
//                                  int byte_size)
// {
//     if (byte_size > 0x7fffffff / 8 || byte_size < 0)
//         byte_size = -1;
//     return init_get_bits(s, buffer, byte_size * 8);
// }

// static inline int init_get_bits8_le(GetBitContext *s, const uint8_t *buffer,
//                                     int byte_size)
// {
//     if (byte_size > 0x7fffffff / 8 || byte_size < 0)
//         byte_size = -1;
//     return init_get_bits_xe(s, buffer, byte_size * 8, 1);
// }

// static inline const uint8_t *align_get_bits(GetBitContext *s)
// {
//     int n = -get_bits_count(s) & 7;
//     if (n)
//         skip_bits(s, n);
//     return s->buffer + (s->index >> 3);
// }

// static inline int set_idx(GetBitContext *s, int code, int *n, int *nb_bits,
//                           int16_t (*table)[2])
// {
//     unsigned idx;
//     *nb_bits = -*n;
//     idx = show_bits(s, *nb_bits) + code;
//     *n = table[idx][1];
//     return table[idx][0];
// }
// static inline int get_vlc2(GetBitContext *s, int16_t (*table)[2],
//                            int bits, int max_depth)
// {
//     int code;
//     unsigned int re_index = (s)->index;
//     unsigned int re_cache;
//     re_cache = av_bswap32((((const union unaligned_32 *)((s)->buffer + (re_index >> 3)))->l)) << (re_index & 7);
//     do
//     {
//         int n, nb_bits;
//         unsigned int index;
//         index = NEG_USR32(re_cache, bits);
//         code = table[index][0];
//         n = table[index][1];
//         if (max_depth > 1 && n < 0)
//         {
//             re_index += (bits);
//             re_cache = av_bswap32((((const union unaligned_32 *)((s)->buffer + (re_index >> 3)))->l)) << (re_index & 7);
//             nb_bits = -n;
//             index = NEG_USR32(re_cache, nb_bits) + code;
//             code = table[index][0];
//             n = table[index][1];
//             if (max_depth > 2 && n < 0)
//             {
//                 re_index += (nb_bits);
//                 re_cache = av_bswap32((((const union unaligned_32 *)((s)->buffer + (re_index >> 3)))->l)) << (re_index & 7);
//                 nb_bits = -n;
//                 index = NEG_USR32(re_cache, nb_bits) + code;
//                 code = table[index][0];
//                 n = table[index][1];
//             }
//         }
//         do
//         {
//             re_cache <<= (n);
//             re_index += (n);
//         } while (0);
//     } while (0);
//     (s)->index = re_index;
//     return code;
// }

// static inline int decode012(GetBitContext *gb)
// {
//     int n;
//     n = get_bits1(gb);
//     if (n == 0)
//         return 0;
//     else
//         return get_bits1(gb) + 1;
// }

// static inline int decode210(GetBitContext *gb)
// {
//     if (get_bits1(gb))
//         return 0;
//     else
//         return 2 - get_bits1(gb);
// }

// static inline int get_bits_left(GetBitContext *gb)
// {
//     return gb->size_in_bits - get_bits_count(gb);
// }

// static inline int skip_1stop_8data_bits(GetBitContext *gb)
// {
//     if (get_bits_left(gb) <= 0)
//         return (-(int)(('I') | (('N') << 8) | (('D') << 16) | ((unsigned)('A') << 24)));
//     while (get_bits1(gb))
//     {
//         skip_bits(gb, 8);
//         if (get_bits_left(gb) <= 0)
//             return (-(int)(('I') | (('N') << 8) | (('D') << 16) | ((unsigned)('A') << 24)));
//     }
//     return 0;
// }

// typedef struct SPS
// {
//     unsigned int sps_id;
//     int profile_idc;
//     int level_idc;
//     int chroma_format_idc;
//     int transform_bypass;
//     int log2_max_frame_num;
//     int poc_type;
//     int log2_max_poc_lsb;
//     int delta_pic_order_always_zero_flag;
//     int offset_for_non_ref_pic;
//     int offset_for_top_to_bottom_field;
//     int poc_cycle_length;
//     int ref_frame_count;
//     int gaps_in_frame_num_allowed_flag;
//     int mb_width;
//     int mb_height;
//     int frame_mbs_only_flag;
//     int mb_aff;
//     int direct_8x8_inference_flag;
//     int crop;
//     unsigned int crop_left;
//     unsigned int crop_right;
//     unsigned int crop_top;
//     unsigned int crop_bottom;
//     int vui_parameters_present_flag;
//     AVRational sar;
//     int video_signal_type_present_flag;
//     int full_range;
//     int colour_description_present_flag;
//     enum AVColorPrimaries color_primaries;
//     enum AVColorTransferCharacteristic color_trc;
//     enum AVColorSpace colorspace;
//     enum AVChromaLocation chroma_location;
//     int timing_info_present_flag;
//     uint32_t num_units_in_tick;
//     uint32_t time_scale;
//     int fixed_frame_rate_flag;
//     int32_t offset_for_ref_frame[256];
//     int bitstream_restriction_flag;
//     int num_reorder_frames;
//     int scaling_matrix_present;
//     uint8_t scaling_matrix4[6][16];
//     uint8_t scaling_matrix8[6][64];
//     int nal_hrd_parameters_present_flag;
//     int vcl_hrd_parameters_present_flag;
//     int pic_struct_present_flag;
//     int time_offset_length;
//     int cpb_cnt;
//     int initial_cpb_removal_delay_length;
//     int cpb_removal_delay_length;
//     int dpb_output_delay_length;
//     int bit_depth_luma;
//     int bit_depth_chroma;
//     int residual_color_transform_flag;
//     int constraint_set_flags;
//     uint8_t data[4096];
//     size_t data_size;
// } SPS;

// typedef struct PPS
// {
//     unsigned int sps_id;
//     int cabac;
//     int pic_order_present;
//     int slice_group_count;
//     int mb_slice_group_map_type;
//     unsigned int ref_count[2];
//     int weighted_pred;
//     int weighted_bipred_idc;
//     int init_qp;
//     int init_qs;
//     int chroma_qp_index_offset[2];
//     int deblocking_filter_parameters_present;
//     int constrained_intra_pred;
//     int redundant_pic_cnt_present;
//     int transform_8x8_mode;
//     uint8_t scaling_matrix4[6][16];
//     uint8_t scaling_matrix8[6][64];
//     uint8_t chroma_qp_table[2][(51 + 6 * 6) + 1];
//     int chroma_qp_diff;
//     uint8_t data[4096];
//     size_t data_size;
//     uint32_t dequant4_buffer[6][(51 + 6 * 6) + 1][16];
//     uint32_t dequant8_buffer[6][(51 + 6 * 6) + 1][64];
//     uint32_t (*dequant4_coeff[6])[16];
//     uint32_t (*dequant8_coeff[6])[64];
//     AVBufferRef *sps_ref;
//     const SPS *sps;
// } PPS;

// typedef struct H264ParamSets
// {
//     AVBufferRef *sps_list[32];
//     AVBufferRef *pps_list[256];
//     AVBufferRef *pps_ref;
//     const PPS *pps;
//     const SPS *sps;
//     int overread_warning_printed[2];
// } H264ParamSets;

// int ff_h264_decode_seq_parameter_set(GetBitContext *gb, AVCodecContext *avctx,
//                                      H264ParamSets *ps, int ignore_truncation);

// int ff_h264_decode_picture_parameter_set(GetBitContext *gb, AVCodecContext *avctx,
//                                          H264ParamSets *ps, int bit_length);

// void ff_h264_ps_uninit(H264ParamSets *ps);

// typedef struct H264PredWeightTable
// {
//     int use_weight;
//     int use_weight_chroma;
//     int luma_log2_weight_denom;
//     int chroma_log2_weight_denom;
//     int luma_weight_flag[2];
//     int chroma_weight_flag[2];
//     int luma_weight[48][2][2];
//     int chroma_weight[48][2][2][2];
//     int implicit_weight[48][48][2];
// } H264PredWeightTable;

// typedef struct H264POCContext
// {
//     int poc_lsb;
//     int poc_msb;
//     int delta_poc_bottom;
//     int delta_poc[2];
//     int frame_num;
//     int prev_poc_msb;
//     int prev_poc_lsb;
//     int frame_num_offset;
//     int prev_frame_num_offset;
//     int prev_frame_num;
// } H264POCContext;

// int ff_h264_pred_weight_table(GetBitContext *gb, const SPS *sps,
//                               const int *ref_count, int slice_type_nos,
//                               H264PredWeightTable *pwt,
//                               int picture_structure, void *logctx);

// int ff_h264_check_intra4x4_pred_mode(int8_t *pred_mode_cache, void *logctx,
//                                      int top_samples_available, int left_samples_available);

// int ff_h264_check_intra_pred_mode(void *logctx, int top_samples_available,
//                                   int left_samples_available,
//                                   int mode, int is_chroma);

// int ff_h264_parse_ref_count(int *plist_count, int ref_count[2],
//                             GetBitContext *gb, const PPS *pps,
//                             int slice_type_nos, int picture_structure, void *logctx);

// int ff_h264_init_poc(int pic_field_poc[2], int *pic_poc,
//                      const SPS *sps, H264POCContext *poc,
//                      int picture_structure, int nal_ref_idc);

// int ff_h264_decode_extradata(const uint8_t *data, int size, H264ParamSets *ps,
//                              int *is_avc, int *nal_length_size,
//                              int err_recognition, void *logctx);

// int ff_h264_get_profile(const SPS *sps);

// typedef enum
// {
//     H264_SEI_TYPE_BUFFERING_PERIOD = 0,
//     H264_SEI_TYPE_PIC_TIMING = 1,
//     H264_SEI_TYPE_PAN_SCAN_RECT = 2,
//     H264_SEI_TYPE_FILLER_PAYLOAD = 3,
//     H264_SEI_TYPE_USER_DATA_REGISTERED = 4,
//     H264_SEI_TYPE_USER_DATA_UNREGISTERED = 5,
//     H264_SEI_TYPE_RECOVERY_POINT = 6,
//     H264_SEI_TYPE_FRAME_PACKING = 45,
//     H264_SEI_TYPE_DISPLAY_ORIENTATION = 47,
//     H264_SEI_TYPE_GREEN_METADATA = 56,
//     H264_SEI_TYPE_MASTERING_DISPLAY_COLOUR_VOLUME = 137,
//     H264_SEI_TYPE_ALTERNATIVE_TRANSFER = 147,
// } H264_SEI_Type;

// typedef enum
// {
//     H264_SEI_PIC_STRUCT_FRAME = 0,
//     H264_SEI_PIC_STRUCT_TOP_FIELD = 1,
//     H264_SEI_PIC_STRUCT_BOTTOM_FIELD = 2,
//     H264_SEI_PIC_STRUCT_TOP_BOTTOM = 3,
//     H264_SEI_PIC_STRUCT_BOTTOM_TOP = 4,
//     H264_SEI_PIC_STRUCT_TOP_BOTTOM_TOP = 5,
//     H264_SEI_PIC_STRUCT_BOTTOM_TOP_BOTTOM = 6,
//     H264_SEI_PIC_STRUCT_FRAME_DOUBLING = 7,
//     H264_SEI_PIC_STRUCT_FRAME_TRIPLING = 8
// } H264_SEI_PicStructType;

// typedef enum
// {
//     H264_SEI_FPA_TYPE_CHECKERBOARD = 0,
//     H264_SEI_FPA_TYPE_INTERLEAVE_COLUMN = 1,
//     H264_SEI_FPA_TYPE_INTERLEAVE_ROW = 2,
//     H264_SEI_FPA_TYPE_SIDE_BY_SIDE = 3,
//     H264_SEI_FPA_TYPE_TOP_BOTTOM = 4,
//     H264_SEI_FPA_TYPE_INTERLEAVE_TEMPORAL = 5,
//     H264_SEI_FPA_TYPE_2D = 6,
// } H264_SEI_FpaType;

// typedef struct H264SEITimeCode
// {
//     int full;
//     int frame;
//     int seconds;
//     int minutes;
//     int hours;
//     int dropframe;
// } H264SEITimeCode;

// typedef struct H264SEIPictureTiming
// {
//     uint8_t payload[40];
//     int payload_size_bits;
//     int present;
//     H264_SEI_PicStructType pic_struct;
//     int ct_type;
//     int dpb_output_delay;
//     int cpb_removal_delay;
//     H264SEITimeCode timecode[3];
//     int timecode_cnt;
// } H264SEIPictureTiming;

// typedef struct H264SEIAFD
// {
//     int present;
//     uint8_t active_format_description;
// } H264SEIAFD;

// typedef struct H264SEIA53Caption
// {
//     AVBufferRef *buf_ref;
// } H264SEIA53Caption;

// typedef struct H264SEIUnregistered
// {
//     int x264_build;
//     AVBufferRef **buf_ref;
//     int nb_buf_ref;
// } H264SEIUnregistered;

// typedef struct H264SEIRecoveryPoint
// {
//     int recovery_frame_cnt;
// } H264SEIRecoveryPoint;

// typedef struct H264SEIBufferingPeriod
// {
//     int present;
//     int initial_cpb_removal_delay[32];
// } H264SEIBufferingPeriod;

// typedef struct H264SEIFramePacking
// {
//     int present;
//     int arrangement_id;
//     int arrangement_cancel_flag;
//     H264_SEI_FpaType arrangement_type;
//     int arrangement_repetition_period;
//     int content_interpretation_type;
//     int quincunx_sampling_flag;
//     int current_frame_is_frame0_flag;
// } H264SEIFramePacking;

// typedef struct H264SEIDisplayOrientation
// {
//     int present;
//     int anticlockwise_rotation;
//     int hflip, vflip;
// } H264SEIDisplayOrientation;

// typedef struct H264SEIGreenMetaData
// {
//     uint8_t green_metadata_type;
//     uint8_t period_type;
//     uint16_t num_seconds;
//     uint16_t num_pictures;
//     uint8_t percent_non_zero_macroblocks;
//     uint8_t percent_intra_coded_macroblocks;
//     uint8_t percent_six_tap_filtering;
//     uint8_t percent_alpha_point_deblocking_instance;
//     uint8_t xsd_metric_type;
//     uint16_t xsd_metric_value;
// } H264SEIGreenMetaData;

// typedef struct H264SEIAlternativeTransfer
// {
//     int present;
//     int preferred_transfer_characteristics;
// } H264SEIAlternativeTransfer;

// typedef struct H264SEIContext
// {
//     H264SEIPictureTiming picture_timing;
//     H264SEIAFD afd;
//     H264SEIA53Caption a53_caption;
//     H264SEIUnregistered unregistered;
//     H264SEIRecoveryPoint recovery_point;
//     H264SEIBufferingPeriod buffering_period;
//     H264SEIFramePacking frame_packing;
//     H264SEIDisplayOrientation display_orientation;
//     H264SEIGreenMetaData green_metadata;
//     H264SEIAlternativeTransfer alternative_transfer;
// } H264SEIContext;

// struct H264ParamSets;

// int ff_h264_sei_decode(H264SEIContext *h, GetBitContext *gb,
//                        const struct H264ParamSets *ps, void *logctx);

// void ff_h264_sei_uninit(H264SEIContext *h);

// const char *ff_h264_sei_stereo_mode(const H264SEIFramePacking *h);

// int ff_h264_sei_process_picture_timing(H264SEIPictureTiming *h, const SPS *sps,
//                                        void *logctx);

// typedef struct H2645NAL
// {
//     uint8_t *rbsp_buffer;
//     int size;
//     const uint8_t *data;
//     int size_bits;
//     int raw_size;
//     const uint8_t *raw_data;
//     GetBitContext gb;
//     int type;
//     int temporal_id;
//     int nuh_layer_id;
//     int skipped_bytes;
//     int skipped_bytes_pos_size;
//     int *skipped_bytes_pos;
//     int ref_idc;
// } H2645NAL;

// typedef struct H2645RBSP
// {
//     uint8_t *rbsp_buffer;
//     AVBufferRef *rbsp_buffer_ref;
//     int rbsp_buffer_alloc_size;
//     int rbsp_buffer_size;
// } H2645RBSP;

// typedef struct H2645Packet
// {
//     H2645NAL *nals;
//     H2645RBSP rbsp;
//     int nb_nals;
//     int nals_allocated;
//     unsigned nal_buffer_size;
// } H2645Packet;

// int ff_h2645_extract_rbsp(const uint8_t *src, int length, H2645RBSP *rbsp,
//                           H2645NAL *nal, int small_padding);
// int ff_h2645_packet_split(H2645Packet *pkt, const uint8_t *buf, int length,
//                           void *logctx, int is_nalff, int nal_length_size,
//                           enum AVCodecID codec_id, int small_padding, int use_ref);

// void ff_h2645_packet_uninit(H2645Packet *pkt);

// static inline int get_nalsize(int nal_length_size, const uint8_t *buf,
//                               int buf_size, int *buf_index, void *logctx)
// {
//     int i, nalsize = 0;
//     if (*buf_index >= buf_size - nal_length_size)
//     {
//         return (-(
//             11));
//     }
//     for (i = 0; i < nal_length_size; i++)
//         nalsize = ((unsigned)nalsize << 8) | buf[(*buf_index)++];
//     if (nalsize <= 0 || nalsize > buf_size - *buf_index)
//     {
//         av_log(logctx, 16,
//                "Invalid NAL unit size (%d > %d).\n", nalsize, buf_size - *buf_index);
//         return (-(int)(('I') | (('N') << 8) | (('D') << 16) | ((unsigned)('A') << 24)));
//     }
//     return nalsize;
// }

// typedef void (*h264_chroma_mc_func)(uint8_t *dst, uint8_t *src, ptrdiff_t srcStride, int h, int x, int y);

// typedef struct H264ChromaContext
// {
//     h264_chroma_mc_func put_h264_chroma_pixels_tab[4];
//     h264_chroma_mc_func avg_h264_chroma_pixels_tab[4];
// } H264ChromaContext;

// void ff_h264chroma_init(H264ChromaContext *c, int bit_depth);

// void ff_h264chroma_init_aarch64(H264ChromaContext *c, int bit_depth);
// void ff_h264chroma_init_arm(H264ChromaContext *c, int bit_depth);
// void ff_h264chroma_init_ppc(H264ChromaContext *c, int bit_depth);
// void ff_h264chroma_init_x86(H264ChromaContext *c, int bit_depth);
// void ff_h264chroma_init_mips(H264ChromaContext *c, int bit_depth);

// typedef void (*h264_weight_func)(uint8_t *block, ptrdiff_t stride, int height,
//                                  int log2_denom, int weight, int offset);
// typedef void (*h264_biweight_func)(uint8_t *dst, uint8_t *src,
//                                    ptrdiff_t stride, int height, int log2_denom,
//                                    int weightd, int weights, int offset);

// typedef struct H264DSPContext
// {
//     h264_weight_func weight_h264_pixels_tab[4];
//     h264_biweight_func biweight_h264_pixels_tab[4];
//     void (*h264_v_loop_filter_luma)(uint8_t *pix, ptrdiff_t stride,
//                                     int alpha, int beta, int8_t *tc0);
//     void (*h264_h_loop_filter_luma)(uint8_t *pix, ptrdiff_t stride,
//                                     int alpha, int beta, int8_t *tc0);
//     void (*h264_h_loop_filter_luma_mbaff)(uint8_t *pix, ptrdiff_t stride,
//                                           int alpha, int beta, int8_t *tc0);
//     void (*h264_v_loop_filter_luma_intra)(uint8_t *pix, ptrdiff_t stride,
//                                           int alpha, int beta);
//     void (*h264_h_loop_filter_luma_intra)(uint8_t *pix, ptrdiff_t stride,
//                                           int alpha, int beta);
//     void (*h264_h_loop_filter_luma_mbaff_intra)(uint8_t *pix,
//                                                 ptrdiff_t stride, int alpha, int beta);
//     void (*h264_v_loop_filter_chroma)(uint8_t *pix, ptrdiff_t stride,
//                                       int alpha, int beta, int8_t *tc0);
//     void (*h264_h_loop_filter_chroma)(uint8_t *pix, ptrdiff_t stride,
//                                       int alpha, int beta, int8_t *tc0);
//     void (*h264_h_loop_filter_chroma_mbaff)(uint8_t *pix,
//                                             ptrdiff_t stride, int alpha, int beta,
//                                             int8_t *tc0);
//     void (*h264_v_loop_filter_chroma_intra)(uint8_t *pix,
//                                             ptrdiff_t stride, int alpha, int beta);
//     void (*h264_h_loop_filter_chroma_intra)(uint8_t *pix,
//                                             ptrdiff_t stride, int alpha, int beta);
//     void (*h264_h_loop_filter_chroma_mbaff_intra)(uint8_t *pix,
//                                                   ptrdiff_t stride, int alpha, int beta);
//     void (*h264_loop_filter_strength)(int16_t bS[2][4][4], uint8_t nnz[40],
//                                       int8_t ref[2][40], int16_t mv[2][40][2],
//                                       int bidir, int edges, int step,
//                                       int mask_mv0, int mask_mv1, int field);
//     void (*h264_idct_add)(uint8_t *dst,
//                           int16_t *block, int stride);
//     void (*h264_idct8_add)(uint8_t *dst,
//                            int16_t *block, int stride);
//     void (*h264_idct_dc_add)(uint8_t *dst,
//                              int16_t *block, int stride);
//     void (*h264_idct8_dc_add)(uint8_t *dst,
//                               int16_t *block, int stride);
//     void (*h264_idct_add16)(uint8_t *dst, const int *blockoffset,
//                             int16_t *block, int stride,
//                             const uint8_t nnzc[15 * 8]);
//     void (*h264_idct8_add4)(uint8_t *dst, const int *blockoffset,
//                             int16_t *block, int stride,
//                             const uint8_t nnzc[15 * 8]);
//     void (*h264_idct_add8)(uint8_t **dst, const int *blockoffset,
//                            int16_t *block, int stride,
//                            const uint8_t nnzc[15 * 8]);
//     void (*h264_idct_add16intra)(uint8_t *dst, const int *blockoffset,
//                                  int16_t *block,
//                                  int stride, const uint8_t nnzc[15 * 8]);
//     void (*h264_luma_dc_dequant_idct)(int16_t *output,
//                                       int16_t *input, int qmul);
//     void (*h264_chroma_dc_dequant_idct)(int16_t *block, int qmul);
//     void (*h264_add_pixels8_clear)(uint8_t *dst, int16_t *block, int stride);
//     void (*h264_add_pixels4_clear)(uint8_t *dst, int16_t *block, int stride);
//     int (*startcode_find_candidate)(const uint8_t *buf, int size);
// } H264DSPContext;

// void ff_h264dsp_init(H264DSPContext *c, const int bit_depth,
//                      const int chroma_format_idc);
// void ff_h264dsp_init_aarch64(H264DSPContext *c, const int bit_depth,
//                              const int chroma_format_idc);
// void ff_h264dsp_init_arm(H264DSPContext *c, const int bit_depth,
//                          const int chroma_format_idc);
// void ff_h264dsp_init_ppc(H264DSPContext *c, const int bit_depth,
//                          const int chroma_format_idc);
// void ff_h264dsp_init_x86(H264DSPContext *c, const int bit_depth,
//                          const int chroma_format_idc);
// void ff_h264dsp_init_mips(H264DSPContext *c, const int bit_depth,
//                           const int chroma_format_idc);

// typedef struct H264PredContext
// {
//     void (*pred4x4[9 + 3 + 3])(uint8_t *src, const uint8_t *topright,
//                                ptrdiff_t stride);
//     void (*pred8x8l[9 + 3])(uint8_t *src, int topleft, int topright,
//                             ptrdiff_t stride);
//     void (*pred8x8[4 + 3 + 4])(uint8_t *src, ptrdiff_t stride);
//     void (*pred16x16[4 + 3 + 2])(uint8_t *src, ptrdiff_t stride);
//     void (*pred4x4_add[2])(uint8_t *pix,
//                            int16_t *block, ptrdiff_t stride);
//     void (*pred8x8l_add[2])(uint8_t *pix,
//                             int16_t *block, ptrdiff_t stride);
//     void (*pred8x8l_filter_add[2])(uint8_t *pix,
//                                    int16_t *block, int topleft, int topright, ptrdiff_t stride);
//     void (*pred8x8_add[3])(uint8_t *pix,
//                            const int *block_offset,
//                            int16_t *block, ptrdiff_t stride);
//     void (*pred16x16_add[3])(uint8_t *pix,
//                              const int *block_offset,
//                              int16_t *block, ptrdiff_t stride);
// } H264PredContext;

// void ff_h264_pred_init(H264PredContext *h, int codec_id,
//                        const int bit_depth, const int chroma_format_idc);
// void ff_h264_pred_init_aarch64(H264PredContext *h, int codec_id,
//                                const int bit_depth,
//                                const int chroma_format_idc);
// void ff_h264_pred_init_arm(H264PredContext *h, int codec_id,
//                            const int bit_depth, const int chroma_format_idc);
// void ff_h264_pred_init_x86(H264PredContext *h, int codec_id,
//                            const int bit_depth, const int chroma_format_idc);
// void ff_h264_pred_init_mips(H264PredContext *h, int codec_id,
//                             const int bit_depth, const int chroma_format_idc);

// void ff_put_pixels8x8_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
// void ff_avg_pixels8x8_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
// void ff_put_pixels16x16_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
// void ff_avg_pixels16x16_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);

// void ff_put_pixels8_l2_8(uint8_t *dst, const uint8_t *src1, const uint8_t *src2,
//                          int dst_stride, int src_stride1, int src_stride2,
//                          int h);

// void ff_put_qpel16_mc11_old_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
// void ff_put_no_rnd_qpel16_mc11_old_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
// void ff_avg_qpel16_mc11_old_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
// void ff_put_qpel16_mc31_old_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
// void ff_put_no_rnd_qpel16_mc31_old_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
// void ff_avg_qpel16_mc31_old_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
// void ff_put_qpel16_mc12_old_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
// void ff_put_no_rnd_qpel16_mc12_old_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
// void ff_avg_qpel16_mc12_old_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
// void ff_put_qpel16_mc32_old_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
// void ff_put_no_rnd_qpel16_mc32_old_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
// void ff_avg_qpel16_mc32_old_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
// void ff_put_qpel16_mc13_old_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
// void ff_put_no_rnd_qpel16_mc13_old_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
// void ff_avg_qpel16_mc13_old_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
// void ff_put_qpel16_mc33_old_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
// void ff_put_no_rnd_qpel16_mc33_old_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
// void ff_avg_qpel16_mc33_old_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
// void ff_put_qpel8_mc11_old_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
// void ff_put_no_rnd_qpel8_mc11_old_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
// void ff_avg_qpel8_mc11_old_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
// void ff_put_qpel8_mc31_old_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
// void ff_put_no_rnd_qpel8_mc31_old_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
// void ff_avg_qpel8_mc31_old_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
// void ff_put_qpel8_mc12_old_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
// void ff_put_no_rnd_qpel8_mc12_old_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
// void ff_avg_qpel8_mc12_old_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
// void ff_put_qpel8_mc32_old_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
// void ff_put_no_rnd_qpel8_mc32_old_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
// void ff_avg_qpel8_mc32_old_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
// void ff_put_qpel8_mc13_old_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
// void ff_put_no_rnd_qpel8_mc13_old_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
// void ff_avg_qpel8_mc13_old_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
// void ff_put_qpel8_mc33_old_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
// void ff_put_no_rnd_qpel8_mc33_old_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
// void ff_avg_qpel8_mc33_old_c(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);

// typedef void (*qpel_mc_func)(uint8_t *dst,
//                              const uint8_t *src,
//                              ptrdiff_t stride);

// typedef struct QpelDSPContext
// {
//     qpel_mc_func put_qpel_pixels_tab[2][16];
//     qpel_mc_func avg_qpel_pixels_tab[2][16];
//     qpel_mc_func put_no_rnd_qpel_pixels_tab[2][16];
// } QpelDSPContext;

// void ff_qpeldsp_init(QpelDSPContext *c);

// void ff_qpeldsp_init_x86(QpelDSPContext *c);
// void ff_qpeldsp_init_mips(QpelDSPContext *c);

// typedef struct H264QpelContext
// {
//     qpel_mc_func put_h264_qpel_pixels_tab[4][16];
//     qpel_mc_func avg_h264_qpel_pixels_tab[4][16];
// } H264QpelContext;

// void ff_h264qpel_init(H264QpelContext *c, int bit_depth);

// void ff_h264qpel_init_aarch64(H264QpelContext *c, int bit_depth);
// void ff_h264qpel_init_arm(H264QpelContext *c, int bit_depth);
// void ff_h264qpel_init_ppc(H264QpelContext *c, int bit_depth);
// void ff_h264qpel_init_x86(H264QpelContext *c, int bit_depth);
// void ff_h264qpel_init_mips(H264QpelContext *c, int bit_depth);

// enum OutputFormat
// {
//     FMT_MPEG1,
//     FMT_H261,
//     FMT_H263,
//     FMT_MJPEG,
// };

// void ff_draw_horiz_band(AVCodecContext *avctx, AVFrame *cur, AVFrame *last,
//                         int y, int h, int picture_structure, int first_field,
//                         int low_delay);

// void ff_print_debug_info2(AVCodecContext *avctx, AVFrame *pict, uint8_t *mbskip_table,
//                           uint32_t *mbtype_table, int8_t *qscale_table, int16_t (*motion_val[2])[2],
//                           int *low_delay,
//                           int mb_width, int mb_height, int mb_stride, int quarter_sample);

// typedef struct ParseContext
// {
//     uint8_t *buffer;
//     int index;
//     int last_index;
//     unsigned int buffer_size;
//     uint32_t state;
//     int frame_start_found;
//     int overread;
//     int overread_index;
//     uint64_t state64;
// } ParseContext;

// int ff_combine_frame(ParseContext *pc, int next, const uint8_t **buf, int *buf_size);
// int ff_mpeg4video_split(AVCodecContext *avctx, const uint8_t *buf,
//                         int buf_size);
// void ff_parse_close(AVCodecParserContext *s);

// void ff_fetch_timestamp(AVCodecParserContext *s, int off, int remove, int fuzzy);



// typedef struct VideoDSPContext
// {
//     void (*emulated_edge_mc)(uint8_t *dst, const uint8_t *src,
//                              ptrdiff_t dst_linesize,
//                              ptrdiff_t src_linesize,
//                              int block_w, int block_h,
//                              int src_x, int src_y, int w, int h);
//     void (*prefetch)(uint8_t *buf, ptrdiff_t stride, int h);
// } VideoDSPContext;

// typedef enum MMCOOpcode
// {
//     MMCO_END = 0,
//     MMCO_SHORT2UNUSED,
//     MMCO_LONG2UNUSED,
//     MMCO_SHORT2LONG,
//     MMCO_SET_MAX_LONG,
//     MMCO_RESET,
//     MMCO_LONG,
// } MMCOOpcode;

// typedef struct MMCO
// {
//     MMCOOpcode opcode;
//     int short_pic_num;
//     int long_arg;
// } MMCO;

// typedef struct H264Picture
// {
//     AVFrame *f;
//     ThreadFrame tf;
//     AVBufferRef *qscale_table_buf;
//     int8_t *qscale_table;
//     AVBufferRef *motion_val_buf[2];
//     int16_t (*motion_val[2])[2];
//     AVBufferRef *mb_type_buf;
//     uint32_t *mb_type;
//     AVBufferRef *hwaccel_priv_buf;
//     void *hwaccel_picture_private;
//     AVBufferRef *ref_index_buf[2];
//     int8_t *ref_index[2];
//     int field_poc[2];
//     int poc;
//     int frame_num;
//     int mmco_reset;
//     int pic_id;
//     int long_ref;
//     int ref_poc[2][2][32];
//     int ref_count[2][2];
//     int mbaff;
//     int field_picture;
//     int reference;
//     int recovered;
//     int invalid_gap;
//     int sei_recovery_frame_cnt;
//     AVBufferRef *pps_buf;
//     const PPS *pps;
//     int mb_width, mb_height;
//     int mb_stride;
// } H264Picture;

// typedef struct H264Ref
// {
//     uint8_t *data[3];
//     int linesize[3];
//     int reference;
//     int poc;
//     int pic_id;
//     H264Picture *parent;
// } H264Ref;

// typedef struct H264SliceContext
// {
//     struct H264Context *h264;
//     GetBitContext gb;
//     ERContext er;
//     int slice_num;
//     int slice_type;
//     int slice_type_nos;
//     int slice_type_fixed;
//     int qscale;
//     int chroma_qp[2];
//     int qp_thresh;
//     int last_qscale_diff;
//     int deblocking_filter;
//     int slice_alpha_c0_offset;
//     int slice_beta_offset;
//     H264PredWeightTable pwt;
//     int prev_mb_skipped;
//     int next_mb_skipped;
//     int chroma_pred_mode;
//     int intra16x16_pred_mode;
//     int8_t intra4x4_pred_mode_cache[5 * 8];
//     int8_t(*intra4x4_pred_mode);
//     int topleft_mb_xy;
//     int top_mb_xy;
//     int topright_mb_xy;
//     int left_mb_xy[2];
//     int topleft_type;
//     int top_type;
//     int topright_type;
//     int left_type[2];
//     const uint8_t *left_block;
//     int topleft_partition;
//     unsigned int topleft_samples_available;
//     unsigned int top_samples_available;
//     unsigned int topright_samples_available;
//     unsigned int left_samples_available;
//     ptrdiff_t linesize, uvlinesize;
//     ptrdiff_t mb_linesize;
//     ptrdiff_t mb_uvlinesize;
//     int mb_x, mb_y;
//     int mb_xy;
//     int resync_mb_x;
//     int resync_mb_y;
//     unsigned int first_mb_addr;
//     int next_slice_idx;
//     int mb_skip_run;
//     int is_complex;
//     int picture_structure;
//     int mb_field_decoding_flag;
//     int mb_mbaff;
//     int redundant_pic_count;
//     int neighbor_transform_size;
//     int direct_spatial_mv_pred;
//     int col_parity;
//     int col_fieldoff;
//     int cbp;
//     int top_cbp;
//     int left_cbp;
//     int dist_scale_factor[32];
//     int dist_scale_factor_field[2][32];
//     int map_col_to_list0[2][16 + 32];
//     int map_col_to_list0_field[2][2][16 + 32];
//     unsigned int ref_count[2];
//     unsigned int list_count;
//     H264Ref ref_list[2][48];
//     struct
//     {
//         uint8_t op;
//         uint32_t val;
//     } ref_modifications[2][32];
//     int nb_ref_modifications[2];
//     unsigned int pps_id;
//     const uint8_t *intra_pcm_ptr;
//     int16_t *dc_val_base;
//     uint8_t *bipred_scratchpad;
//     uint8_t *edge_emu_buffer;
//     uint8_t (*top_borders[2])[(16 * 3) * 2];
//     int bipred_scratchpad_allocated;
//     int edge_emu_buffer_allocated;
//     int top_borders_allocated[2];
//     uint8_t non_zero_count_cache[15 * 8];
//     int16_t mv_cache[2][5 * 8][2];
//     int8_t ref_cache[2][5 * 8];
//     uint8_t mvd_cache[2][5 * 8][2];
//     uint8_t direct_cache[5 * 8];
//     uint16_t sub_mb_type[4];
//     int16_t mb[16 * 48 * 2];
//     int16_t mb_luma_dc[3][16 * 2];
//     int16_t mb_padding[256 * 2];
//     uint8_t (*mvd_table[2])[2];
//     CABACContext cabac;
//     uint8_t cabac_state[1024];
//     int cabac_init_idc;
//     MMCO mmco[66];
//     int nb_mmco;
//     int explicit_ref_marking;
//     int frame_num;
//     int poc_lsb;
//     int delta_poc_bottom;
//     int delta_poc[2];
//     int curr_pic_num;
//     int max_pic_num;
// } H264SliceContext;

// typedef struct H264Context
// {
//     const AVClass *class;
//     AVCodecContext *avctx;
//     VideoDSPContext vdsp;
//     H264DSPContext h264dsp;
//     H264ChromaContext h264chroma;
//     H264QpelContext h264qpel;
//     H264Picture DPB[36];
//     H264Picture *cur_pic_ptr;
//     H264Picture cur_pic;
//     H264Picture last_pic_for_ec;
//     H264SliceContext *slice_ctx;
//     int nb_slice_ctx;
//     int nb_slice_ctx_queued;
//     H2645Packet pkt;
//     int pixel_shift;
//     int width, height;
//     int chroma_x_shift, chroma_y_shift;
//     int droppable;
//     int coded_picture_number;
//     int context_initialized;
//     int flags;
//     int workaround_bugs;
//     int x264_build;
//     int postpone_filter;
//     int picture_idr;
//     int crop_left;
//     int crop_right;
//     int crop_top;
//     int crop_bottom;
//     int8_t(*intra4x4_pred_mode);
//     H264PredContext hpc;
//     uint8_t (*non_zero_count)[48];
//     int block_offset[2 * (16 * 3)];
//     uint32_t *mb2b_xy;
//     uint32_t *mb2br_xy;
//     int b_stride;
//     uint16_t *slice_table;
//     int mb_aff_frame;
//     int picture_structure;
//     int first_field;
//     uint8_t *list_counts;
//     uint16_t *cbp_table;
//     uint8_t *chroma_pred_mode_table;
//     uint8_t (*mvd_table[2])[2];
//     uint8_t *direct_table;
//     uint8_t scan_padding[16];
//     uint8_t zigzag_scan[16];
//     uint8_t zigzag_scan8x8[64];
//     uint8_t zigzag_scan8x8_cavlc[64];
//     uint8_t field_scan[16];
//     uint8_t field_scan8x8[64];
//     uint8_t field_scan8x8_cavlc[64];
//     uint8_t zigzag_scan_q0[16];
//     uint8_t zigzag_scan8x8_q0[64];
//     uint8_t zigzag_scan8x8_cavlc_q0[64];
//     uint8_t field_scan_q0[16];
//     uint8_t field_scan8x8_q0[64];
//     uint8_t field_scan8x8_cavlc_q0[64];
//     int mb_y;
//     int mb_height, mb_width;
//     int mb_stride;
//     int mb_num;
//     int nal_ref_idc;
//     int nal_unit_type;
//     int has_slice;
//     int is_avc;
//     int nal_length_size;
//     int bit_depth_luma;
//     int chroma_format_idc;
//     H264ParamSets ps;
//     uint16_t *slice_table_base;
//     H264POCContext poc;
//     H264Ref default_ref[2];
//     H264Picture *short_ref[32];
//     H264Picture *long_ref[32];
//     H264Picture *delayed_pic[16 + 2];
//     int last_pocs[16];
//     H264Picture *next_output_pic;
//     int next_outputed_poc;
//     MMCO mmco[66];
//     int nb_mmco;
//     int mmco_reset;
//     int explicit_ref_marking;
//     int long_ref_count;
//     int short_ref_count;
//     int current_slice;
//     int prev_interlaced_frame;
//     int valid_recovery_point;
//     int recovery_frame;
//     int frame_recovered;
//     int has_recovery_point;
//     int missing_fields;
//     int setup_finished;
//     int cur_chroma_format_idc;
//     int cur_bit_depth_luma;
//     int16_t slice_row[32];
//     int width_from_caller;
//     int height_from_caller;
//     int enable_er;
//     H264SEIContext sei;
//     AVBufferPool *qscale_table_pool;
//     AVBufferPool *mb_type_pool;
//     AVBufferPool *motion_val_pool;
//     AVBufferPool *ref_index_pool;
//     int ref2frm[32][2][64];
// } H264Context;

extern const uint16_t ff_h264_mb_sizes[4];

static const uint8_t scan8[16 * 3 + 3] = {
    4 + 1 * 8, 5 + 1 * 8, 4 + 2 * 8, 5 + 2 * 8,
    6 + 1 * 8, 7 + 1 * 8, 6 + 2 * 8, 7 + 2 * 8,
    4 + 3 * 8, 5 + 3 * 8, 4 + 4 * 8, 5 + 4 * 8,
    6 + 3 * 8, 7 + 3 * 8, 6 + 4 * 8, 7 + 4 * 8,
    4 + 6 * 8, 5 + 6 * 8, 4 + 7 * 8, 5 + 7 * 8,
    6 + 6 * 8, 7 + 6 * 8, 6 + 7 * 8, 7 + 7 * 8,
    4 + 8 * 8, 5 + 8 * 8, 4 + 9 * 8, 5 + 9 * 8,
    6 + 8 * 8, 7 + 8 * 8, 6 + 9 * 8, 7 + 9 * 8,
    4 + 11 * 8, 5 + 11 * 8, 4 + 12 * 8, 5 + 12 * 8,
    6 + 11 * 8, 7 + 11 * 8, 6 + 12 * 8, 7 + 12 * 8,
    4 + 13 * 8, 5 + 13 * 8, 4 + 14 * 8, 5 + 14 * 8,
    6 + 13 * 8, 7 + 13 * 8, 6 + 14 * 8, 7 + 14 * 8,
    0 + 0 * 8, 0 + 5 * 8, 0 + 10 * 8};

static inline uint32_t pack16to32(unsigned a, unsigned b)
{
    return (a & 0xFFFF) + (b << 16);
}

static inline uint16_t pack8to16(unsigned a, unsigned b)
{
    return (a & 0xFF) + (b << 8);
}

static inline int get_chroma_qp(const PPS *pps, int t, int qscale)
{
    return pps->chroma_qp_table[t][qscale];
}

static inline int pred_intra_mode(const H264Context *h,
                                  H264SliceContext *sl, int n)
{
    const int index8 = scan8[n];
    const int left = sl->intra4x4_pred_mode_cache[index8 - 1];
    const int top = sl->intra4x4_pred_mode_cache[index8 - 8];
    const int min = ((left) > (top) ? (top) : (left));
    do
    {
    } while (0);
    if (min < 0)
        return 2;
    else
        return min;
}

static inline void write_back_intra_pred_mode(const H264Context *h,
                                              H264SliceContext *sl)
{
    int8_t *i4x4 = sl->intra4x4_pred_mode + h->mb2br_xy[sl->mb_xy];
    int8_t *i4x4_cache = sl->intra4x4_pred_mode_cache;
    (((av_alias32 *)(i4x4))->u32 = ((const av_alias32 *)(i4x4_cache + 4 + 8 * 4))->u32);
    i4x4[4] = i4x4_cache[7 + 8 * 3];
    i4x4[5] = i4x4_cache[7 + 8 * 2];
    i4x4[6] = i4x4_cache[7 + 8 * 1];
}

static inline void write_back_non_zero_count(const H264Context *h,
                                             H264SliceContext *sl)
{
    const int mb_xy = sl->mb_xy;
    uint8_t *nnz = h->non_zero_count[mb_xy];
    uint8_t *nnz_cache = sl->non_zero_count_cache;
    (((av_alias32 *)(&nnz[0]))->u32 = ((const av_alias32 *)(&nnz_cache[4 + 8 * 1]))->u32);
    (((av_alias32 *)(&nnz[4]))->u32 = ((const av_alias32 *)(&nnz_cache[4 + 8 * 2]))->u32);
    (((av_alias32 *)(&nnz[8]))->u32 = ((const av_alias32 *)(&nnz_cache[4 + 8 * 3]))->u32);
    (((av_alias32 *)(&nnz[12]))->u32 = ((const av_alias32 *)(&nnz_cache[4 + 8 * 4]))->u32);
    (((av_alias32 *)(&nnz[16]))->u32 = ((const av_alias32 *)(&nnz_cache[4 + 8 * 6]))->u32);
    (((av_alias32 *)(&nnz[20]))->u32 = ((const av_alias32 *)(&nnz_cache[4 + 8 * 7]))->u32);
    (((av_alias32 *)(&nnz[32]))->u32 = ((const av_alias32 *)(&nnz_cache[4 + 8 * 11]))->u32);
    (((av_alias32 *)(&nnz[36]))->u32 = ((const av_alias32 *)(&nnz_cache[4 + 8 * 12]))->u32);
    if (!h->chroma_y_shift)
    {
        (((av_alias32 *)(&nnz[24]))->u32 = ((const av_alias32 *)(&nnz_cache[4 + 8 * 8]))->u32);
        (((av_alias32 *)(&nnz[28]))->u32 = ((const av_alias32 *)(&nnz_cache[4 + 8 * 9]))->u32);
        (((av_alias32 *)(&nnz[40]))->u32 = ((const av_alias32 *)(&nnz_cache[4 + 8 * 13]))->u32);
        (((av_alias32 *)(&nnz[44]))->u32 = ((const av_alias32 *)(&nnz_cache[4 + 8 * 14]))->u32);
    }
}

static inline void write_back_motion_list(const H264Context *h,
                                          H264SliceContext *sl,
                                          int b_stride,
                                          int b_xy, int b8_xy,
                                          int mb_type, int list)
{
    int16_t(*mv_dst)[2] = &h->cur_pic.motion_val[list][b_xy];
    int16_t(*mv_src)[2] = &sl->mv_cache[list][scan8[0]];
    do
    {
        (((av_alias64 *)(mv_dst + 0 * b_stride))->u64 = ((const av_alias64 *)(mv_src + 8 * 0))->u64);
        (((av_alias64 *)((char *)(mv_dst + 0 * b_stride) + 8))->u64 = ((const av_alias64 *)((char *)(mv_src + 8 * 0) + 8))->u64);
    } while (0);
    do
    {
        (((av_alias64 *)(mv_dst + 1 * b_stride))->u64 = ((const av_alias64 *)(mv_src + 8 * 1))->u64);
        (((av_alias64 *)((char *)(mv_dst + 1 * b_stride) + 8))->u64 = ((const av_alias64 *)((char *)(mv_src + 8 * 1) + 8))->u64);
    } while (0);
    do
    {
        (((av_alias64 *)(mv_dst + 2 * b_stride))->u64 = ((const av_alias64 *)(mv_src + 8 * 2))->u64);
        (((av_alias64 *)((char *)(mv_dst + 2 * b_stride) + 8))->u64 = ((const av_alias64 *)((char *)(mv_src + 8 * 2) + 8))->u64);
    } while (0);
    do
    {
        (((av_alias64 *)(mv_dst + 3 * b_stride))->u64 = ((const av_alias64 *)(mv_src + 8 * 3))->u64);
        (((av_alias64 *)((char *)(mv_dst + 3 * b_stride) + 8))->u64 = ((const av_alias64 *)((char *)(mv_src + 8 * 3) + 8))->u64);
    } while (0);
    if ((h)->ps.pps->cabac)
    {
        uint8_t(*mvd_dst)[2] = &sl->mvd_table[list][0 ? 8 * sl->mb_xy
                                                      : h->mb2br_xy[sl->mb_xy]];
        uint8_t(*mvd_src)[2] = &sl->mvd_cache[list][scan8[0]];
        if (((mb_type) & (1 << 11)))
        {
            do
            {
                (((av_alias64 *)(mvd_dst))->u64 = 0);
                (((av_alias64 *)((char *)(mvd_dst) + 8))->u64 = 0);
            } while (0);
        }
        else
        {
            (((av_alias64 *)(mvd_dst))->u64 = ((const av_alias64 *)(mvd_src + 8 * 3))->u64);
            (((av_alias16 *)(mvd_dst + 3 + 3))->u16 = ((const av_alias16 *)(mvd_src + 3 + 8 * 0))->u16);
            (((av_alias16 *)(mvd_dst + 3 + 2))->u16 = ((const av_alias16 *)(mvd_src + 3 + 8 * 1))->u16);
            (((av_alias16 *)(mvd_dst + 3 + 1))->u16 = ((const av_alias16 *)(mvd_src + 3 + 8 * 2))->u16);
        }
    }
    {
        int8_t *ref_index = &h->cur_pic.ref_index[list][b8_xy];
        int8_t *ref_cache = sl->ref_cache[list];
        ref_index[0 + 0 * 2] = ref_cache[scan8[0]];
        ref_index[1 + 0 * 2] = ref_cache[scan8[4]];
        ref_index[0 + 1 * 2] = ref_cache[scan8[8]];
        ref_index[1 + 1 * 2] = ref_cache[scan8[12]];
    }
}

static inline void h264_fill_rectangle(void *vp, int w, int h, int stride, uint32_t val, int size)
{
    uint8_t *p = (uint8_t *)vp;
    ((void)0);
    ((void)0);
    w *= size;
    stride *= size;
    ((void)0);
    ((void)0);
    if (w == 2)
    {
        const uint16_t v = size == 4 ? val : val * 0x0101;
        *(uint16_t *)(p + 0 * stride) = v;
        if (h == 1)
            return;
        *(uint16_t *)(p + 1 * stride) = v;
        if (h == 2)
            return;
        *(uint16_t *)(p + 2 * stride) = v;
        *(uint16_t *)(p + 3 * stride) = v;
    }
    else if (w == 4)
    {
        const uint32_t v = size == 4 ? val : size == 2 ? val * 0x00010001 : val * 0x01010101;
        *(uint32_t *)(p + 0 * stride) = v;
        if (h == 1)
            return;
        *(uint32_t *)(p + 1 * stride) = v;
        if (h == 2)
            return;
        *(uint32_t *)(p + 2 * stride) = v;
        *(uint32_t *)(p + 3 * stride) = v;
    }
    else if (w == 8)
    {
        const uint64_t v = size == 2 ? val * 0x0001000100010001ULL : val * 0x0100000001ULL;
        *(uint64_t *)(p + 0 * stride) = v;
        if (h == 1)
            return;
        *(uint64_t *)(p + 1 * stride) = v;
        if (h == 2)
            return;
        *(uint64_t *)(p + 2 * stride) = v;
        *(uint64_t *)(p + 3 * stride) = v;
    }
    else if (w == 16)
    {
        const uint64_t v = val * 0x0100000001ULL;
        *(uint64_t *)(p + 0 + 0 * stride) = v;
        *(uint64_t *)(p + 8 + 0 * stride) = v;
        *(uint64_t *)(p + 0 + 1 * stride) = v;
        *(uint64_t *)(p + 8 + 1 * stride) = v;
        if (h == 2)
            return;
        *(uint64_t *)(p + 0 + 2 * stride) = v;
        *(uint64_t *)(p + 8 + 2 * stride) = v;
        *(uint64_t *)(p + 0 + 3 * stride) = v;
        *(uint64_t *)(p + 8 + 3 * stride) = v;
    }
    else
        ((void)0);
    ((void)0);
}
static inline void write_back_motion(const H264Context *h,
                                     H264SliceContext *sl,
                                     int mb_type)
{
    const int b_stride = h->b_stride;
    const int b_xy = 4 * sl->mb_x + 4 * sl->mb_y * h->b_stride;
    const int b8_xy = 4 * sl->mb_xy;
    if (((mb_type) & (((1 << 12) | (1 << 13)) << (2 * (0)))))
    {
        write_back_motion_list(h, sl, b_stride, b_xy, b8_xy, mb_type, 0);
    }
    else
    {
        h264_fill_rectangle(&h->cur_pic.ref_index[0][b8_xy],
                       2, 2, 2, (uint8_t)-1, 1);
    }
    if (((mb_type) & (((1 << 12) | (1 << 13)) << (2 * (1)))))
        write_back_motion_list(h, sl, b_stride, b_xy, b8_xy, mb_type, 1);
    if (sl->slice_type_nos == AV_PICTURE_TYPE_B && (h)->ps.pps->cabac)
    {
        if (((mb_type) & (1 << 6)))
        {
            uint8_t *direct_table = &h->direct_table[4 * sl->mb_xy];
            direct_table[1] = sl->sub_mb_type[1] >> 1;
            direct_table[2] = sl->sub_mb_type[2] >> 1;
            direct_table[3] = sl->sub_mb_type[3] >> 1;
        }
    }
}

static inline int get_dct8x8_allowed(const H264Context *h, H264SliceContext *sl)
{
    if (h->ps.sps->direct_8x8_inference_flag)
        return !((((const av_alias64 *)(sl->sub_mb_type))->u64) &
                 (((1 << 4) | (1 << 5) | (1 << 6)) *
                  0x0001000100010001ULL));
    else
        return !((((const av_alias64 *)(sl->sub_mb_type))->u64) &
                 (((1 << 4) | (1 << 5) | (1 << 6) | (1 << 8)) *
                  0x0001000100010001ULL));
}

const uint8_t *avpriv_find_start_code(const uint8_t *av_restrict p,
                                      const uint8_t *end,
                                      uint32_t *av_restrict state)
{
    int i;

    av_assert0(p <= end);
    if (p >= end)
        return end;

    for (i = 0; i < 3; i++) {
        uint32_t tmp = *state << 8;
        *state = tmp + *(p++);
        if (tmp == 0x100 || p == end)
            return p;
    }

    while (p < end) {
        if      (p[-1] > 1      ) p += 3;
        else if (p[-2]          ) p += 2;
        else if (p[-3]|(p[-1]-1)) p++;
        else {
            p++;
            break;
        }
    }

    p = FFMIN(p, end) - 4;
    *state = AV_RB32(p);

    return p + 4;
}

static inline int find_start_code(const uint8_t *buf, int buf_size,
                                  int buf_index, int next_avc)
{
    uint32_t state = -1;
    buf_index = avpriv_find_start_code(buf + buf_index, buf + next_avc + 1, &state) - buf - 1;
    return ((buf_index) > (buf_size) ? (buf_size) : (buf_index));
}


typedef struct IMbInfo
{
    uint16_t type;
    uint8_t pred_mode;
    uint8_t cbp;
} IMbInfo;

extern const IMbInfo ff_h264_i_mb_type_info[26];

typedef struct PMbInfo
{
    uint16_t type;
    uint8_t partition_count;
} PMbInfo;



static const AVRational ff_h264_pixel_aspect[17] = {
    {0, 1},
    {1, 1},
    {12, 11},
    {10, 11},
    {16, 11},
    {40, 33},
    {24, 11},
    {20, 11},
    {32, 11},
    {80, 33},
    {18, 11},
    {15, 11},
    {64, 33},
    {160, 99},
    {4, 3},
    {3, 2},
    {2, 1},
};

extern const uint8_t ff_h264_dequant4_coeff_init[6][3];
extern const uint8_t ff_h264_dequant8_coeff_init_scan[16];
extern const uint8_t ff_h264_dequant8_coeff_init[6][6];
extern const uint8_t ff_h264_quant_rem6[(51 + 6 * 6) + 1];
extern const uint8_t ff_h264_quant_div6[(51 + 6 * 6) + 1];

extern const uint8_t ff_h264_chroma_qp[7][(51 + 6 * 6) + 1];
static inline int fetch_diagonal_mv(const H264Context *h, H264SliceContext *sl,
                                    const int16_t **C,
                                    int i, int list, int part_width)
{
    const int topright_ref = sl->ref_cache[list][i - 8 + part_width];
    if ((h)->mb_aff_frame)
    {
        if (topright_ref == -2 && i >= scan8[0] + 8 && (i & 7) == 4 && sl->ref_cache[list][scan8[0] - 1] != -2)
        {
            const uint32_t *mb_types = h->cur_pic_ptr->mb_type;
            const int16_t *mv;
            (((av_alias32 *)(sl->mv_cache[list][scan8[0] - 2]))->u32 = 0);
            *C = sl->mv_cache[list][scan8[0] - 2];
            if (!(sl)->mb_field_decoding_flag && ((sl->left_type[0]) & (1 << 7)))
            {
                const int xy = sl->left_mb_xy[0] + h->mb_stride, y4 = (sl->mb_y & 1) * 2 + (i >> 5);
                const int mb_type = mb_types[xy + (y4 >> 2) * h->mb_stride];
                if (!((mb_type) & (((1 << 12) | (1 << 13)) << (2 * (list)))))
                    return -1;
                mv = h->cur_pic_ptr->motion_val[list][h->mb2b_xy[xy] + 3 + y4 * h->b_stride];
                sl->mv_cache[list][scan8[0] - 2][0] = mv[0];
                sl->mv_cache[list][scan8[0] - 2][1] = mv[1] * 2;
                return h->cur_pic_ptr->ref_index[list][4 * xy + 1 + (y4 & ~1)] >> 1;
                ;
            }
            if ((sl)->mb_field_decoding_flag && !((sl->left_type[0]) & (1 << 7)))
            {
                const int xy = sl->left_mb_xy[i >= 36], y4 = ((i >> 2)) & 3;
                const int mb_type = mb_types[xy + (y4 >> 2) * h->mb_stride];
                if (!((mb_type) & (((1 << 12) | (1 << 13)) << (2 * (list)))))
                    return -1;
                mv = h->cur_pic_ptr->motion_val[list][h->mb2b_xy[xy] + 3 + y4 * h->b_stride];
                sl->mv_cache[list][scan8[0] - 2][0] = mv[0];
                sl->mv_cache[list][scan8[0] - 2][1] = mv[1] / 2;
                return h->cur_pic_ptr->ref_index[list][4 * xy + 1 + (y4 & ~1)] * 2;
                ;
            }
        }
    }
    if (topright_ref != -2)
    {
        *C = sl->mv_cache[list][i - 8 + part_width];
        return topright_ref;
    }
    else
    {
        do
        {
        } while (0);
        *C = sl->mv_cache[list][i - 8 - 1];
        return sl->ref_cache[list][i - 8 - 1];
    }
}


static inline void pred_motion(const H264Context *const h,
                               H264SliceContext *sl,
                               int n,
                               int part_width, int list, int ref,
                               int *const mx, int *const my)
{
    const int index8 = scan8[n];
    const int top_ref = sl->ref_cache[list][index8 - 8];
    const int left_ref = sl->ref_cache[list][index8 - 1];
    const int16_t *const A = sl->mv_cache[list][index8 - 1];
    const int16_t *const B = sl->mv_cache[list][index8 - 8];
    const int16_t *C;
    int diagonal_ref, match_count;
    ((void)0);
    diagonal_ref = fetch_diagonal_mv(h, sl, &C, index8, list, part_width);
    match_count = (diagonal_ref == ref) + (top_ref == ref) + (left_ref == ref);
    do
    {
    } while (0);
    if (match_count > 1)
    {
        *mx = mid_pred(A[0], B[0], C[0]);
        *my = mid_pred(A[1], B[1], C[1]);
    }
    else if (match_count == 1)
    {
        if (left_ref == ref)
        {
            *mx = A[0];
            *my = A[1];
        }
        else if (top_ref == ref)
        {
            *mx = B[0];
            *my = B[1];
        }
        else
        {
            *mx = C[0];
            *my = C[1];
        }
    }
    else
    {
        if (top_ref == -2 &&
            diagonal_ref == -2 &&
            left_ref != -2)
        {
            *mx = A[0];
            *my = A[1];
        }
        else
        {
            *mx = mid_pred(A[0], B[0], C[0]);
            *my = mid_pred(A[1], B[1], C[1]);
        }
    }
    do
    {
    } while (0);
}

static inline void pred_16x8_motion(const H264Context *const h,
                                    H264SliceContext *sl,
                                    int n, int list, int ref,
                                    int *const mx, int *const my)
{
    if (n == 0)
    {
        const int top_ref = sl->ref_cache[list][scan8[0] - 8];
        const int16_t *const B = sl->mv_cache[list][scan8[0] - 8];
        do
        {
        } while (0);
        if (top_ref == ref)
        {
            *mx = B[0];
            *my = B[1];
            return;
        }
    }
    else
    {
        const int left_ref = sl->ref_cache[list][scan8[8] - 1];
        const int16_t *const A = sl->mv_cache[list][scan8[8] - 1];
        do
        {
        } while (0);
        if (left_ref == ref)
        {
            *mx = A[0];
            *my = A[1];
            return;
        }
    }
    pred_motion(h, sl, n, 4, list, ref, mx, my);
}

static inline void pred_8x16_motion(const H264Context *const h,
                                    H264SliceContext *sl,
                                    int n, int list, int ref,
                                    int *const mx, int *const my)
{
    if (n == 0)
    {
        const int left_ref = sl->ref_cache[list][scan8[0] - 1];
        const int16_t *const A = sl->mv_cache[list][scan8[0] - 1];
        do
        {
        } while (0);
        if (left_ref == ref)
        {
            *mx = A[0];
            *my = A[1];
            return;
        }
    }
    else
    {
        const int16_t *C;
        int diagonal_ref;
        diagonal_ref = fetch_diagonal_mv(h, sl, &C, scan8[4], list, 2);
        do
        {
        } while (0);
        if (diagonal_ref == ref)
        {
            *mx = C[0];
            *my = C[1];
            return;
        }
    }
    pred_motion(h, sl, n, 2, list, ref, mx, my);
}
static inline void pred_pskip_motion(const H264Context *const h,
                                     H264SliceContext *sl)
{
    static const int16_t zeromv[2] = {0};
    int16_t mvbuf[3][2];
    int8_t *ref = h->cur_pic.ref_index[0];
    int16_t(*mv)[2] = h->cur_pic.motion_val[0];
    int top_ref, left_ref, diagonal_ref, match_count, mx, my;
    const int16_t *A, *B, *C;
    int b_stride = h->b_stride;
    h264_fill_rectangle(&sl->ref_cache[0][scan8[0]], 4, 4, 8, 0, 1);
    if (((sl->left_type[0]) & (((1 << 12) | (1 << 13)) << (2 * (0)))))
    {
        left_ref = ref[4 * sl->left_mb_xy[0] + 1 + (sl->left_block[0] & ~1)];
        A = mv[h->mb2b_xy[sl->left_mb_xy[0]] + 3 + b_stride * sl->left_block[0]];
        if ((h)->mb_aff_frame)
        {
            if ((sl)->mb_field_decoding_flag)
            {
                if (!((sl->left_type[0]) & (1 << 7)))
                {
                    left_ref <<= 1;
                    (((av_alias32 *)(mvbuf[0]))->u32 = ((const av_alias32 *)(A))->u32);
                    mvbuf[0][1] /= 2;
                    A = mvbuf[0];
                }
            }
            else
            {
                if (((sl->left_type[0]) & (1 << 7)))
                {
                    left_ref >>= 1;
                    (((av_alias32 *)(mvbuf[0]))->u32 = ((const av_alias32 *)(A))->u32);
                    mvbuf[0][1] *= 2;
                    A = mvbuf[0];
                }
            }
        };
        if (!(left_ref | (((const av_alias32 *)(A))->u32)))
            goto zeromv;
    }
    else if (sl->left_type[0])
    {
        left_ref = -1;
        A = zeromv;
    }
    else
    {
        goto zeromv;
    }
    if (((sl->top_type) & (((1 << 12) | (1 << 13)) << (2 * (0)))))
    {
        top_ref = ref[4 * sl->top_mb_xy + 2];
        B = mv[h->mb2b_xy[sl->top_mb_xy] + 3 * b_stride];
        if ((h)->mb_aff_frame)
        {
            if ((sl)->mb_field_decoding_flag)
            {
                if (!((sl->top_type) & (1 << 7)))
                {
                    top_ref <<= 1;
                    (((av_alias32 *)(mvbuf[1]))->u32 = ((const av_alias32 *)(B))->u32);
                    mvbuf[1][1] /= 2;
                    B = mvbuf[1];
                }
            }
            else
            {
                if (((sl->top_type) & (1 << 7)))
                {
                    top_ref >>= 1;
                    (((av_alias32 *)(mvbuf[1]))->u32 = ((const av_alias32 *)(B))->u32);
                    mvbuf[1][1] *= 2;
                    B = mvbuf[1];
                }
            }
        };
        if (!(top_ref | (((const av_alias32 *)(B))->u32)))
            goto zeromv;
    }
    else if (sl->top_type)
    {
        top_ref = -1;
        B = zeromv;
    }
    else
    {
        goto zeromv;
    }
    do
    {
    } while (0);
    if (((sl->topright_type) & (((1 << 12) | (1 << 13)) << (2 * (0)))))
    {
        diagonal_ref = ref[4 * sl->topright_mb_xy + 2];
        C = mv[h->mb2b_xy[sl->topright_mb_xy] + 3 * b_stride];
        if ((h)->mb_aff_frame)
        {
            if ((sl)->mb_field_decoding_flag)
            {
                if (!((sl->topright_type) & (1 << 7)))
                {
                    diagonal_ref <<= 1;
                    (((av_alias32 *)(mvbuf[2]))->u32 = ((const av_alias32 *)(C))->u32);
                    mvbuf[2][1] /= 2;
                    C = mvbuf[2];
                }
            }
            else
            {
                if (((sl->topright_type) & (1 << 7)))
                {
                    diagonal_ref >>= 1;
                    (((av_alias32 *)(mvbuf[2]))->u32 = ((const av_alias32 *)(C))->u32);
                    mvbuf[2][1] *= 2;
                    C = mvbuf[2];
                }
            }
        };
    }
    else if (sl->topright_type)
    {
        diagonal_ref = -1;
        C = zeromv;
    }
    else
    {
        if (((sl->topleft_type) & (((1 << 12) | (1 << 13)) << (2 * (0)))))
        {
            diagonal_ref = ref[4 * sl->topleft_mb_xy + 1 +
                               (sl->topleft_partition & 2)];
            C = mv[h->mb2b_xy[sl->topleft_mb_xy] + 3 + b_stride +
                   (sl->topleft_partition & 2 * b_stride)];
            if ((h)->mb_aff_frame)
            {
                if ((sl)->mb_field_decoding_flag)
                {
                    if (!((sl->topleft_type) & (1 << 7)))
                    {
                        diagonal_ref <<= 1;
                        (((av_alias32 *)(mvbuf[2]))->u32 = ((const av_alias32 *)(C))->u32);
                        mvbuf[2][1] /= 2;
                        C = mvbuf[2];
                    }
                }
                else
                {
                    if (((sl->topleft_type) & (1 << 7)))
                    {
                        diagonal_ref >>= 1;
                        (((av_alias32 *)(mvbuf[2]))->u32 = ((const av_alias32 *)(C))->u32);
                        mvbuf[2][1] *= 2;
                        C = mvbuf[2];
                    }
                }
            };
        }
        else if (sl->topleft_type)
        {
            diagonal_ref = -1;
            C = zeromv;
        }
        else
        {
            diagonal_ref = -2;
            C = zeromv;
        }
    }
    match_count = !diagonal_ref + !top_ref + !left_ref;
    do
    {
    } while (0);
    if (match_count > 1)
    {
        mx = mid_pred(A[0], B[0], C[0]);
        my = mid_pred(A[1], B[1], C[1]);
    }
    else if (match_count == 1)
    {
        if (!left_ref)
        {
            mx = A[0];
            my = A[1];
        }
        else if (!top_ref)
        {
            mx = B[0];
            my = B[1];
        }
        else
        {
            mx = C[0];
            my = C[1];
        }
    }
    else
    {
        mx = mid_pred(A[0], B[0], C[0]);
        my = mid_pred(A[1], B[1], C[1]);
    }
    h264_fill_rectangle(sl->mv_cache[0][scan8[0]], 4, 4, 8, pack16to32(mx, my), 4);
    return;

zeromv:
    h264_fill_rectangle(sl->mv_cache[0][scan8[0]], 4, 4, 8, 0, 4);
    return;
}

static void fill_decode_neighbors(const H264Context *h, H264SliceContext *sl, int mb_type)
{
    const int mb_xy = sl->mb_xy;
    int topleft_xy, top_xy, topright_xy, left_xy[2];
    static const uint8_t left_block_options[4][32] = {
        {0, 1, 2, 3, 7, 10, 8, 11, 3 + 0 * 4, 3 + 1 * 4, 3 + 2 * 4, 3 + 3 * 4, 1 + 4 * 4, 1 + 8 * 4, 1 + 5 * 4, 1 + 9 * 4},
        {2, 2, 3, 3, 8, 11, 8, 11, 3 + 2 * 4, 3 + 2 * 4, 3 + 3 * 4, 3 + 3 * 4, 1 + 5 * 4, 1 + 9 * 4, 1 + 5 * 4, 1 + 9 * 4},
        {0, 0, 1, 1, 7, 10, 7, 10, 3 + 0 * 4, 3 + 0 * 4, 3 + 1 * 4, 3 + 1 * 4, 1 + 4 * 4, 1 + 8 * 4, 1 + 4 * 4, 1 + 8 * 4},
        {0, 2, 0, 2, 7, 10, 7, 10, 3 + 0 * 4, 3 + 2 * 4, 3 + 0 * 4, 3 + 2 * 4, 1 + 4 * 4, 1 + 8 * 4, 1 + 4 * 4, 1 + 8 * 4}};
    sl->topleft_partition = -1;
    top_xy = mb_xy - (h->mb_stride << (sl)->mb_field_decoding_flag);
    topleft_xy = top_xy - 1;
    topright_xy = top_xy + 1;
    left_xy[1] = left_xy[0] = mb_xy - 1;
    sl->left_block = left_block_options[0];
    if ((h)->mb_aff_frame)
    {
        const int left_mb_field_flag = ((h->cur_pic.mb_type[mb_xy - 1]) & (1 << 7));
        const int curr_mb_field_flag = ((mb_type) & (1 << 7));
        if (sl->mb_y & 1)
        {
            if (left_mb_field_flag != curr_mb_field_flag)
            {
                left_xy[1] = left_xy[0] = mb_xy - h->mb_stride - 1;
                if (curr_mb_field_flag)
                {
                    left_xy[1] += h->mb_stride;
                    sl->left_block = left_block_options[3];
                }
                else
                {
                    topleft_xy += h->mb_stride;
                    sl->topleft_partition = 0;
                    sl->left_block = left_block_options[1];
                }
            }
        }
        else
        {
            if (curr_mb_field_flag)
            {
                topleft_xy += h->mb_stride & (((h->cur_pic.mb_type[top_xy - 1] >> 7) & 1) - 1);
                topright_xy += h->mb_stride & (((h->cur_pic.mb_type[top_xy + 1] >> 7) & 1) - 1);
                top_xy += h->mb_stride & (((h->cur_pic.mb_type[top_xy] >> 7) & 1) - 1);
            }
            if (left_mb_field_flag != curr_mb_field_flag)
            {
                if (curr_mb_field_flag)
                {
                    left_xy[1] += h->mb_stride;
                    sl->left_block = left_block_options[3];
                }
                else
                {
                    sl->left_block = left_block_options[2];
                }
            }
        }
    }
    sl->topleft_mb_xy = topleft_xy;
    sl->top_mb_xy = top_xy;
    sl->topright_mb_xy = topright_xy;
    sl->left_mb_xy[0] = left_xy[0];
    sl->left_mb_xy[1] = left_xy[1];
    sl->topleft_type = h->cur_pic.mb_type[topleft_xy];
    sl->top_type = h->cur_pic.mb_type[top_xy];
    sl->topright_type = h->cur_pic.mb_type[topright_xy];
    sl->left_type[0] = h->cur_pic.mb_type[left_xy[0]];
    sl->left_type[1] = h->cur_pic.mb_type[left_xy[1]];
    if (0)
    {
        if (h->slice_table[topleft_xy] != sl->slice_num)
            sl->topleft_type = 0;
        if (h->slice_table[top_xy] != sl->slice_num)
            sl->top_type = 0;
        if (h->slice_table[left_xy[0]] != sl->slice_num)
            sl->left_type[0] = sl->left_type[1] = 0;
    }
    else
    {
        if (h->slice_table[topleft_xy] != sl->slice_num)
        {
            sl->topleft_type = 0;
            if (h->slice_table[top_xy] != sl->slice_num)
                sl->top_type = 0;
            if (h->slice_table[left_xy[0]] != sl->slice_num)
                sl->left_type[0] = sl->left_type[1] = 0;
        }
    }
    if (h->slice_table[topright_xy] != sl->slice_num)
        sl->topright_type = 0;
}

static void fill_decode_caches(const H264Context *h, H264SliceContext *sl, int mb_type)
{
    int topleft_xy, top_xy, topright_xy, left_xy[2];
    int topleft_type, top_type, topright_type, left_type[2];
    const uint8_t *left_block = sl->left_block;
    int i;
    uint8_t *nnz;
    uint8_t *nnz_cache;
    topleft_xy = sl->topleft_mb_xy;
    top_xy = sl->top_mb_xy;
    topright_xy = sl->topright_mb_xy;
    left_xy[0] = sl->left_mb_xy[0];
    left_xy[1] = sl->left_mb_xy[1];
    topleft_type = sl->topleft_type;
    top_type = sl->top_type;
    topright_type = sl->topright_type;
    left_type[0] = sl->left_type[0];
    left_type[1] = sl->left_type[1];
    if (!((mb_type) & (1 << 11)))
    {
        if (((mb_type)&7))
        {
            int type_mask = h->ps.pps->constrained_intra_pred ? ((-1) & 7) : -1;
            sl->topleft_samples_available =
                sl->top_samples_available =
                    sl->left_samples_available = 0xFFFF;
            sl->topright_samples_available = 0xEEEA;
            if (!(top_type & type_mask))
            {
                sl->topleft_samples_available = 0xB3FF;
                sl->top_samples_available = 0x33FF;
                sl->topright_samples_available = 0x26EA;
            }
            if (((mb_type) & (1 << 7)) != ((left_type[0]) & (1 << 7)))
            {
                if (((mb_type) & (1 << 7)))
                {
                    if (!(left_type[0] & type_mask))
                    {
                        sl->topleft_samples_available &= 0xDFFF;
                        sl->left_samples_available &= 0x5FFF;
                    }
                    if (!(left_type[1] & type_mask))
                    {
                        sl->topleft_samples_available &= 0xFF5F;
                        sl->left_samples_available &= 0xFF5F;
                    }
                }
                else
                {
                    int left_typei = h->cur_pic.mb_type[left_xy[0] + h->mb_stride];
                    ((void)0);
                    if (!((left_typei & type_mask) && (left_type[0] & type_mask)))
                    {
                        sl->topleft_samples_available &= 0xDF5F;
                        sl->left_samples_available &= 0x5F5F;
                    }
                }
            }
            else
            {
                if (!(left_type[0] & type_mask))
                {
                    sl->topleft_samples_available &= 0xDF5F;
                    sl->left_samples_available &= 0x5F5F;
                }
            }
            if (!(topleft_type & type_mask))
                sl->topleft_samples_available &= 0x7FFF;
            if (!(topright_type & type_mask))
                sl->topright_samples_available &= 0xFBFF;
            if (((mb_type) & (1 << 0)))
            {
                if (((top_type) & (1 << 0)))
                {
                    (((av_alias32 *)(sl->intra4x4_pred_mode_cache + 4 + 8 * 0))->u32 = ((const av_alias32 *)(sl->intra4x4_pred_mode + h->mb2br_xy[top_xy]))->u32);
                }
                else
                {
                    sl->intra4x4_pred_mode_cache[4 + 8 * 0] =
                        sl->intra4x4_pred_mode_cache[5 + 8 * 0] =
                            sl->intra4x4_pred_mode_cache[6 + 8 * 0] =
                                sl->intra4x4_pred_mode_cache[7 + 8 * 0] = 2 - 3 * !(top_type & type_mask);
                }
                for (i = 0; i < 2; i++)
                {
                    if (((left_type[(i)]) & (1 << 0)))
                    {
                        int8_t *mode = sl->intra4x4_pred_mode + h->mb2br_xy[left_xy[(i)]];
                        sl->intra4x4_pred_mode_cache[3 + 8 * 1 + 2 * 8 * i] = mode[6 - left_block[0 + 2 * i]];
                        sl->intra4x4_pred_mode_cache[3 + 8 * 2 + 2 * 8 * i] = mode[6 - left_block[1 + 2 * i]];
                    }
                    else
                    {
                        sl->intra4x4_pred_mode_cache[3 + 8 * 1 + 2 * 8 * i] =
                            sl->intra4x4_pred_mode_cache[3 + 8 * 2 + 2 * 8 * i] = 2 - 3 * !(left_type[(i)] & type_mask);
                    }
                }
            }
        }
        nnz_cache = sl->non_zero_count_cache;
        if (top_type)
        {
            nnz = h->non_zero_count[top_xy];
            (((av_alias32 *)(&nnz_cache[4 + 8 * 0]))->u32 = ((const av_alias32 *)(&nnz[4 * 3]))->u32);
            if (!h->chroma_y_shift)
            {
                (((av_alias32 *)(&nnz_cache[4 + 8 * 5]))->u32 = ((const av_alias32 *)(&nnz[4 * 7]))->u32);
                (((av_alias32 *)(&nnz_cache[4 + 8 * 10]))->u32 = ((const av_alias32 *)(&nnz[4 * 11]))->u32);
            }
            else
            {
                (((av_alias32 *)(&nnz_cache[4 + 8 * 5]))->u32 = ((const av_alias32 *)(&nnz[4 * 5]))->u32);
                (((av_alias32 *)(&nnz_cache[4 + 8 * 10]))->u32 = ((const av_alias32 *)(&nnz[4 * 9]))->u32);
            }
        }
        else
        {
            uint32_t top_empty = (h)->ps.pps->cabac && !((mb_type)&7) ? 0 : 0x40404040;
            (((av_alias32 *)(&nnz_cache[4 + 8 * 0]))->u32 = (top_empty));
            (((av_alias32 *)(&nnz_cache[4 + 8 * 5]))->u32 = (top_empty));
            (((av_alias32 *)(&nnz_cache[4 + 8 * 10]))->u32 = (top_empty));
        }
        for (i = 0; i < 2; i++)
        {
            if (left_type[(i)])
            {
                nnz = h->non_zero_count[left_xy[(i)]];
                nnz_cache[3 + 8 * 1 + 2 * 8 * i] = nnz[left_block[8 + 0 + 2 * i]];
                nnz_cache[3 + 8 * 2 + 2 * 8 * i] = nnz[left_block[8 + 1 + 2 * i]];
                if (((h)->ps.sps->chroma_format_idc == 3))
                {
                    nnz_cache[3 + 8 * 6 + 2 * 8 * i] = nnz[left_block[8 + 0 + 2 * i] + 4 * 4];
                    nnz_cache[3 + 8 * 7 + 2 * 8 * i] = nnz[left_block[8 + 1 + 2 * i] + 4 * 4];
                    nnz_cache[3 + 8 * 11 + 2 * 8 * i] = nnz[left_block[8 + 0 + 2 * i] + 8 * 4];
                    nnz_cache[3 + 8 * 12 + 2 * 8 * i] = nnz[left_block[8 + 1 + 2 * i] + 8 * 4];
                }
                else if (((h)->ps.sps->chroma_format_idc == 2))
                {
                    nnz_cache[3 + 8 * 6 + 2 * 8 * i] = nnz[left_block[8 + 0 + 2 * i] - 2 + 4 * 4];
                    nnz_cache[3 + 8 * 7 + 2 * 8 * i] = nnz[left_block[8 + 1 + 2 * i] - 2 + 4 * 4];
                    nnz_cache[3 + 8 * 11 + 2 * 8 * i] = nnz[left_block[8 + 0 + 2 * i] - 2 + 8 * 4];
                    nnz_cache[3 + 8 * 12 + 2 * 8 * i] = nnz[left_block[8 + 1 + 2 * i] - 2 + 8 * 4];
                }
                else
                {
                    nnz_cache[3 + 8 * 6 + 8 * i] = nnz[left_block[8 + 4 + 2 * i]];
                    nnz_cache[3 + 8 * 11 + 8 * i] = nnz[left_block[8 + 5 + 2 * i]];
                }
            }
            else
            {
                nnz_cache[3 + 8 * 1 + 2 * 8 * i] =
                    nnz_cache[3 + 8 * 2 + 2 * 8 * i] =
                        nnz_cache[3 + 8 * 6 + 2 * 8 * i] =
                            nnz_cache[3 + 8 * 7 + 2 * 8 * i] =
                                nnz_cache[3 + 8 * 11 + 2 * 8 * i] =
                                    nnz_cache[3 + 8 * 12 + 2 * 8 * i] = (h)->ps.pps->cabac && !((mb_type)&7) ? 0 : 64;
            }
        }
        if ((h)->ps.pps->cabac)
        {
            if (top_type)
                sl->top_cbp = h->cbp_table[top_xy];
            else
                sl->top_cbp = ((mb_type)&7) ? 0x7CF : 0x00F;
            if (left_type[0])
            {
                sl->left_cbp = (h->cbp_table[left_xy[0]] & 0x7F0) |
                               ((h->cbp_table[left_xy[0]] >> (left_block[0] & (~1))) & 2) |
                               (((h->cbp_table[left_xy[1]] >> (left_block[2] & (~1))) & 2) << 2);
            }
            else
            {
                sl->left_cbp = ((mb_type)&7) ? 0x7CF : 0x00F;
            }
        }
    }
    if (((mb_type) & ((1 << 3) | (1 << 4) | (1 << 5) | (1 << 6))) || (((mb_type) & (1 << 8)) && sl->direct_spatial_mv_pred))
    {
        int list;
        int b_stride = h->b_stride;
        for (list = 0; list < sl->list_count; list++)
        {
            int8_t *ref_cache = &sl->ref_cache[list][scan8[0]];
            int8_t *ref = h->cur_pic.ref_index[list];
            int16_t(*mv_cache)[2] = &sl->mv_cache[list][scan8[0]];
            int16_t(*mv)[2] = h->cur_pic.motion_val[list];
            if (!((mb_type) & (((1 << 12) | (1 << 13)) << (2 * (list)))))
                continue;
            ((void)0);
            if (((top_type) & (((1 << 12) | (1 << 13)) << (2 * (list)))))
            {
                const int b_xy = h->mb2b_xy[top_xy] + 3 * b_stride;
                do
                {
                    (((av_alias64 *)(mv_cache[0 - 1 * 8]))->u64 = ((const av_alias64 *)(mv[b_xy + 0]))->u64);
                    (((av_alias64 *)((char *)(mv_cache[0 - 1 * 8]) + 8))->u64 = ((const av_alias64 *)((char *)(mv[b_xy + 0]) + 8))->u64);
                } while (0);
                ref_cache[0 - 1 * 8] =
                    ref_cache[1 - 1 * 8] = ref[4 * top_xy + 2];
                ref_cache[2 - 1 * 8] =
                    ref_cache[3 - 1 * 8] = ref[4 * top_xy + 3];
            }
            else
            {
                do
                {
                    (((av_alias64 *)(mv_cache[0 - 1 * 8]))->u64 = 0);
                    (((av_alias64 *)((char *)(mv_cache[0 - 1 * 8]) + 8))->u64 = 0);
                } while (0);
                (((av_alias32 *)(&ref_cache[0 - 1 * 8]))->u32 = (((top_type ? -1 : -2) & 0xFF) * 0x01010101u));
            }
            if (mb_type & ((1 << 4) | (1 << 6)))
            {
                for (i = 0; i < 2; i++)
                {
                    int cache_idx = -1 + i * 2 * 8;
                    if (((left_type[(i)]) & (((1 << 12) | (1 << 13)) << (2 * (list)))))
                    {
                        const int b_xy = h->mb2b_xy[left_xy[(i)]] + 3;
                        const int b8_xy = 4 * left_xy[(i)] + 1;
                        (((av_alias32 *)(mv_cache[cache_idx]))->u32 = ((const av_alias32 *)(mv[b_xy + b_stride * left_block[0 + i * 2]]))->u32);
                        (((av_alias32 *)(mv_cache[cache_idx + 8]))->u32 = ((const av_alias32 *)(mv[b_xy + b_stride * left_block[1 + i * 2]]))->u32);
                        ref_cache[cache_idx] = ref[b8_xy + (left_block[0 + i * 2] & ~1)];
                        ref_cache[cache_idx + 8] = ref[b8_xy + (left_block[1 + i * 2] & ~1)];
                    }
                    else
                    {
                        (((av_alias32 *)(mv_cache[cache_idx]))->u32 = 0);
                        (((av_alias32 *)(mv_cache[cache_idx + 8]))->u32 = 0);
                        ref_cache[cache_idx] =
                            ref_cache[cache_idx + 8] = (left_type[(i)]) ? -1
                                                                        : -2;
                    }
                }
            }
            else
            {
                if (((left_type[0]) & (((1 << 12) | (1 << 13)) << (2 * (list)))))
                {
                    const int b_xy = h->mb2b_xy[left_xy[0]] + 3;
                    const int b8_xy = 4 * left_xy[0] + 1;
                    (((av_alias32 *)(mv_cache[-1]))->u32 = ((const av_alias32 *)(mv[b_xy + b_stride * left_block[0]]))->u32);
                    ref_cache[-1] = ref[b8_xy + (left_block[0] & ~1)];
                }
                else
                {
                    (((av_alias32 *)(mv_cache[-1]))->u32 = 0);
                    ref_cache[-1] = left_type[0] ? -1
                                                 : -2;
                }
            }
            if (((topright_type) & (((1 << 12) | (1 << 13)) << (2 * (list)))))
            {
                const int b_xy = h->mb2b_xy[topright_xy] + 3 * b_stride;
                (((av_alias32 *)(mv_cache[4 - 1 * 8]))->u32 = ((const av_alias32 *)(mv[b_xy]))->u32);
                ref_cache[4 - 1 * 8] = ref[4 * topright_xy + 2];
            }
            else
            {
                (((av_alias32 *)(mv_cache[4 - 1 * 8]))->u32 = 0);
                ref_cache[4 - 1 * 8] = topright_type ? -1
                                                     : -2;
            }
            if (ref_cache[2 - 1 * 8] < 0 || ref_cache[4 - 1 * 8] < 0)
            {
                if (((topleft_type) & (((1 << 12) | (1 << 13)) << (2 * (list)))))
                {
                    const int b_xy = h->mb2b_xy[topleft_xy] + 3 + b_stride +
                                     (sl->topleft_partition & 2 * b_stride);
                    const int b8_xy = 4 * topleft_xy + 1 + (sl->topleft_partition & 2);
                    (((av_alias32 *)(mv_cache[-1 - 1 * 8]))->u32 = ((const av_alias32 *)(mv[b_xy]))->u32);
                    ref_cache[-1 - 1 * 8] = ref[b8_xy];
                }
                else
                {
                    (((av_alias32 *)(mv_cache[-1 - 1 * 8]))->u32 = 0);
                    ref_cache[-1 - 1 * 8] = topleft_type ? -1
                                                         : -2;
                }
            }
            if ((mb_type & ((1 << 11) | (1 << 8))) && !(h)->mb_aff_frame)
                continue;
            if (!(mb_type & ((1 << 11) | (1 << 8))))
            {
                uint8_t(*mvd_cache)[2] = &sl->mvd_cache[list][scan8[0]];
                uint8_t(*mvd)[2] = sl->mvd_table[list];
                ref_cache[2 + 8 * 0] =
                    ref_cache[2 + 8 * 2] = -2;
                (((av_alias32 *)(mv_cache[2 + 8 * 0]))->u32 = 0);
                (((av_alias32 *)(mv_cache[2 + 8 * 2]))->u32 = 0);
                if ((h)->ps.pps->cabac)
                {
                    if (((top_type) & (((1 << 12) | (1 << 13)) << (2 * (list)))))
                    {
                        const int b_xy = h->mb2br_xy[top_xy];
                        (((av_alias64 *)(mvd_cache[0 - 1 * 8]))->u64 = ((const av_alias64 *)(mvd[b_xy + 0]))->u64);
                    }
                    else
                    {
                        (((av_alias64 *)(mvd_cache[0 - 1 * 8]))->u64 = 0);
                    }
                    if (((left_type[0]) & (((1 << 12) | (1 << 13)) << (2 * (list)))))
                    {
                        const int b_xy = h->mb2br_xy[left_xy[0]] + 6;
                        (((av_alias16 *)(mvd_cache[-1 + 0 * 8]))->u16 = ((const av_alias16 *)(mvd[b_xy - left_block[0]]))->u16);
                        (((av_alias16 *)(mvd_cache[-1 + 1 * 8]))->u16 = ((const av_alias16 *)(mvd[b_xy - left_block[1]]))->u16);
                    }
                    else
                    {
                        (((av_alias16 *)(mvd_cache[-1 + 0 * 8]))->u16 = 0);
                        (((av_alias16 *)(mvd_cache[-1 + 1 * 8]))->u16 = 0);
                    }
                    if (((left_type[1]) & (((1 << 12) | (1 << 13)) << (2 * (list)))))
                    {
                        const int b_xy = h->mb2br_xy[left_xy[1]] + 6;
                        (((av_alias16 *)(mvd_cache[-1 + 2 * 8]))->u16 = ((const av_alias16 *)(mvd[b_xy - left_block[2]]))->u16);
                        (((av_alias16 *)(mvd_cache[-1 + 3 * 8]))->u16 = ((const av_alias16 *)(mvd[b_xy - left_block[3]]))->u16);
                    }
                    else
                    {
                        (((av_alias16 *)(mvd_cache[-1 + 2 * 8]))->u16 = 0);
                        (((av_alias16 *)(mvd_cache[-1 + 3 * 8]))->u16 = 0);
                    }
                    (((av_alias16 *)(mvd_cache[2 + 8 * 0]))->u16 = 0);
                    (((av_alias16 *)(mvd_cache[2 + 8 * 2]))->u16 = 0);
                    if (sl->slice_type_nos == AV_PICTURE_TYPE_B)
                    {
                        uint8_t *direct_cache = &sl->direct_cache[scan8[0]];
                        uint8_t *direct_table = h->direct_table;
                        h264_fill_rectangle(direct_cache, 4, 4, 8, (1 << 3) >> 1, 1);
                        if (((top_type) & (1 << 8)))
                        {
                            (((av_alias32 *)(&direct_cache[-1 * 8]))->u32 = (0x01010101u * ((1 << 8) >> 1)));
                        }
                        else if (((top_type) & (1 << 6)))
                        {
                            int b8_xy = 4 * top_xy;
                            direct_cache[0 - 1 * 8] = direct_table[b8_xy + 2];
                            direct_cache[2 - 1 * 8] = direct_table[b8_xy + 3];
                        }
                        else
                        {
                            (((av_alias32 *)(&direct_cache[-1 * 8]))->u32 = (0x01010101 * ((1 << 3) >> 1)));
                        }
                        if (((left_type[0]) & (1 << 8)))
                            direct_cache[-1 + 0 * 8] = (1 << 8) >> 1;
                        else if (((left_type[0]) & (1 << 6)))
                            direct_cache[-1 + 0 * 8] = direct_table[4 * left_xy[0] + 1 + (left_block[0] & ~1)];
                        else
                            direct_cache[-1 + 0 * 8] = (1 << 3) >> 1;
                        if (((left_type[1]) & (1 << 8)))
                            direct_cache[-1 + 2 * 8] = (1 << 8) >> 1;
                        else if (((left_type[1]) & (1 << 6)))
                            direct_cache[-1 + 2 * 8] = direct_table[4 * left_xy[1] + 1 + (left_block[2] & ~1)];
                        else
                            direct_cache[-1 + 2 * 8] = (1 << 3) >> 1;
                    }
                }
            }
            if ((h)->mb_aff_frame)
            {
                if ((sl)->mb_field_decoding_flag)
                {
                    if (!((topleft_type) & (1 << 7)) && sl->ref_cache[list][scan8[0] - 1 - 1 * 8] >= 0)
                    {
                        sl->ref_cache[list][scan8[0] - 1 - 1 * 8] *= 2;
                        sl->mv_cache[list][scan8[0] - 1 - 1 * 8][1] /= 2;
                        sl->mvd_cache[list][scan8[0] - 1 - 1 * 8][1] >>= 1;
                    }
                    if (!((top_type) & (1 << 7)) && sl->ref_cache[list][scan8[0] + 0 - 1 * 8] >= 0)
                    {
                        sl->ref_cache[list][scan8[0] + 0 - 1 * 8] *= 2;
                        sl->mv_cache[list][scan8[0] + 0 - 1 * 8][1] /= 2;
                        sl->mvd_cache[list][scan8[0] + 0 - 1 * 8][1] >>= 1;
                    }
                    if (!((top_type) & (1 << 7)) && sl->ref_cache[list][scan8[0] + 1 - 1 * 8] >= 0)
                    {
                        sl->ref_cache[list][scan8[0] + 1 - 1 * 8] *= 2;
                        sl->mv_cache[list][scan8[0] + 1 - 1 * 8][1] /= 2;
                        sl->mvd_cache[list][scan8[0] + 1 - 1 * 8][1] >>= 1;
                    }
                    if (!((top_type) & (1 << 7)) && sl->ref_cache[list][scan8[0] + 2 - 1 * 8] >= 0)
                    {
                        sl->ref_cache[list][scan8[0] + 2 - 1 * 8] *= 2;
                        sl->mv_cache[list][scan8[0] + 2 - 1 * 8][1] /= 2;
                        sl->mvd_cache[list][scan8[0] + 2 - 1 * 8][1] >>= 1;
                    }
                    if (!((top_type) & (1 << 7)) && sl->ref_cache[list][scan8[0] + 3 - 1 * 8] >= 0)
                    {
                        sl->ref_cache[list][scan8[0] + 3 - 1 * 8] *= 2;
                        sl->mv_cache[list][scan8[0] + 3 - 1 * 8][1] /= 2;
                        sl->mvd_cache[list][scan8[0] + 3 - 1 * 8][1] >>= 1;
                    }
                    if (!((topright_type) & (1 << 7)) && sl->ref_cache[list][scan8[0] + 4 - 1 * 8] >= 0)
                    {
                        sl->ref_cache[list][scan8[0] + 4 - 1 * 8] *= 2;
                        sl->mv_cache[list][scan8[0] + 4 - 1 * 8][1] /= 2;
                        sl->mvd_cache[list][scan8[0] + 4 - 1 * 8][1] >>= 1;
                    }
                    if (!((left_type[0]) & (1 << 7)) && sl->ref_cache[list][scan8[0] - 1 + 0 * 8] >= 0)
                    {
                        sl->ref_cache[list][scan8[0] - 1 + 0 * 8] *= 2;
                        sl->mv_cache[list][scan8[0] - 1 + 0 * 8][1] /= 2;
                        sl->mvd_cache[list][scan8[0] - 1 + 0 * 8][1] >>= 1;
                    }
                    if (!((left_type[0]) & (1 << 7)) && sl->ref_cache[list][scan8[0] - 1 + 1 * 8] >= 0)
                    {
                        sl->ref_cache[list][scan8[0] - 1 + 1 * 8] *= 2;
                        sl->mv_cache[list][scan8[0] - 1 + 1 * 8][1] /= 2;
                        sl->mvd_cache[list][scan8[0] - 1 + 1 * 8][1] >>= 1;
                    }
                    if (!((left_type[1]) & (1 << 7)) && sl->ref_cache[list][scan8[0] - 1 + 2 * 8] >= 0)
                    {
                        sl->ref_cache[list][scan8[0] - 1 + 2 * 8] *= 2;
                        sl->mv_cache[list][scan8[0] - 1 + 2 * 8][1] /= 2;
                        sl->mvd_cache[list][scan8[0] - 1 + 2 * 8][1] >>= 1;
                    }
                    if (!((left_type[1]) & (1 << 7)) && sl->ref_cache[list][scan8[0] - 1 + 3 * 8] >= 0)
                    {
                        sl->ref_cache[list][scan8[0] - 1 + 3 * 8] *= 2;
                        sl->mv_cache[list][scan8[0] - 1 + 3 * 8][1] /= 2;
                        sl->mvd_cache[list][scan8[0] - 1 + 3 * 8][1] >>= 1;
                    }
                }
                else
                {
                    if (((topleft_type) & (1 << 7)) && sl->ref_cache[list][scan8[0] - 1 - 1 * 8] >= 0)
                    {
                        sl->ref_cache[list][scan8[0] - 1 - 1 * 8] >>= 1;
                        sl->mv_cache[list][scan8[0] - 1 - 1 * 8][1] *= 2;
                        sl->mvd_cache[list][scan8[0] - 1 - 1 * 8][1] <<= 1;
                    }
                    if (((top_type) & (1 << 7)) && sl->ref_cache[list][scan8[0] + 0 - 1 * 8] >= 0)
                    {
                        sl->ref_cache[list][scan8[0] + 0 - 1 * 8] >>= 1;
                        sl->mv_cache[list][scan8[0] + 0 - 1 * 8][1] *= 2;
                        sl->mvd_cache[list][scan8[0] + 0 - 1 * 8][1] <<= 1;
                    }
                    if (((top_type) & (1 << 7)) && sl->ref_cache[list][scan8[0] + 1 - 1 * 8] >= 0)
                    {
                        sl->ref_cache[list][scan8[0] + 1 - 1 * 8] >>= 1;
                        sl->mv_cache[list][scan8[0] + 1 - 1 * 8][1] *= 2;
                        sl->mvd_cache[list][scan8[0] + 1 - 1 * 8][1] <<= 1;
                    }
                    if (((top_type) & (1 << 7)) && sl->ref_cache[list][scan8[0] + 2 - 1 * 8] >= 0)
                    {
                        sl->ref_cache[list][scan8[0] + 2 - 1 * 8] >>= 1;
                        sl->mv_cache[list][scan8[0] + 2 - 1 * 8][1] *= 2;
                        sl->mvd_cache[list][scan8[0] + 2 - 1 * 8][1] <<= 1;
                    }
                    if (((top_type) & (1 << 7)) && sl->ref_cache[list][scan8[0] + 3 - 1 * 8] >= 0)
                    {
                        sl->ref_cache[list][scan8[0] + 3 - 1 * 8] >>= 1;
                        sl->mv_cache[list][scan8[0] + 3 - 1 * 8][1] *= 2;
                        sl->mvd_cache[list][scan8[0] + 3 - 1 * 8][1] <<= 1;
                    }
                    if (((topright_type) & (1 << 7)) && sl->ref_cache[list][scan8[0] + 4 - 1 * 8] >= 0)
                    {
                        sl->ref_cache[list][scan8[0] + 4 - 1 * 8] >>= 1;
                        sl->mv_cache[list][scan8[0] + 4 - 1 * 8][1] *= 2;
                        sl->mvd_cache[list][scan8[0] + 4 - 1 * 8][1] <<= 1;
                    }
                    if (((left_type[0]) & (1 << 7)) && sl->ref_cache[list][scan8[0] - 1 + 0 * 8] >= 0)
                    {
                        sl->ref_cache[list][scan8[0] - 1 + 0 * 8] >>= 1;
                        sl->mv_cache[list][scan8[0] - 1 + 0 * 8][1] *= 2;
                        sl->mvd_cache[list][scan8[0] - 1 + 0 * 8][1] <<= 1;
                    }
                    if (((left_type[0]) & (1 << 7)) && sl->ref_cache[list][scan8[0] - 1 + 1 * 8] >= 0)
                    {
                        sl->ref_cache[list][scan8[0] - 1 + 1 * 8] >>= 1;
                        sl->mv_cache[list][scan8[0] - 1 + 1 * 8][1] *= 2;
                        sl->mvd_cache[list][scan8[0] - 1 + 1 * 8][1] <<= 1;
                    }
                    if (((left_type[1]) & (1 << 7)) && sl->ref_cache[list][scan8[0] - 1 + 2 * 8] >= 0)
                    {
                        sl->ref_cache[list][scan8[0] - 1 + 2 * 8] >>= 1;
                        sl->mv_cache[list][scan8[0] - 1 + 2 * 8][1] *= 2;
                        sl->mvd_cache[list][scan8[0] - 1 + 2 * 8][1] <<= 1;
                    }
                    if (((left_type[1]) & (1 << 7)) && sl->ref_cache[list][scan8[0] - 1 + 3 * 8] >= 0)
                    {
                        sl->ref_cache[list][scan8[0] - 1 + 3 * 8] >>= 1;
                        sl->mv_cache[list][scan8[0] - 1 + 3 * 8][1] *= 2;
                        sl->mvd_cache[list][scan8[0] - 1 + 3 * 8][1] <<= 1;
                    }
                }
            }
        }
    }
    sl->neighbor_transform_size = !!((top_type)&0x01000000) + !!((left_type[0]) & 0x01000000);
}


static void missing_feature_sample(int sample, void *avc, const char *msg,
                                   va_list argument_list)
{
    av_vlog(avc, AV_LOG_WARNING, msg, argument_list);
    av_log(avc, AV_LOG_WARNING, " is not implemented. Update your FFmpeg "
           "version to the newest one from Git. If the problem still "
           "occurs, it means that your file has a feature which has not "
           "been implemented.\n");
    if (sample)
        av_log(avc, AV_LOG_WARNING, "If you want to help, upload a sample "
               "of this file to https://streams.videolan.org/upload/ "
               "and contact the ffmpeg-devel mailing list. (ffmpeg-devel@ffmpeg.org)\n");
}
void avpriv_request_sample(void *avc, const char *msg, ...)
{
    va_list argument_list;

    va_start(argument_list, msg);
    missing_feature_sample(1, avc, msg, argument_list);
    va_end(argument_list);
}

static int get_scale_factor(H264SliceContext *sl,
                            int poc, int poc1, int i)
{
    int poc0 = sl->ref_list[0][i].poc;
    int64_t pocdiff = poc1 - (int64_t)poc0;
    int td = av_clip_int8(pocdiff);

    if (pocdiff != (int)pocdiff)
        avpriv_request_sample(sl->h264->avctx, "pocdiff overflow");

    if (td == 0 || sl->ref_list[0][i].parent->long_ref) {
        return 256;
    } else {
        int64_t pocdiff0 = poc - (int64_t)poc0;
        int tb = av_clip_int8(pocdiff0);
        int tx = (16384 + (FFABS(td) >> 1)) / td;

        if (pocdiff0 != (int)pocdiff0)
            av_log(sl->h264->avctx, AV_LOG_DEBUG, "pocdiff0 overflow\n");

        return av_clip_intp2((tb * tx + 32) >> 6, 10);
    }
}

void ff_h264_direct_dist_scale_factor(const H264Context *const h,
                                      H264SliceContext *sl)
{
    const int poc  = FIELD_PICTURE(h) ? h->cur_pic_ptr->field_poc[h->picture_structure == PICT_BOTTOM_FIELD]
                                      : h->cur_pic_ptr->poc;
    const int poc1 = sl->ref_list[1][0].poc;
    int i, field;

    if (FRAME_MBAFF(h))
        for (field = 0; field < 2; field++) {
            const int poc  = h->cur_pic_ptr->field_poc[field];
            const int poc1 = sl->ref_list[1][0].parent->field_poc[field];
            for (i = 0; i < 2 * sl->ref_count[0]; i++)
                sl->dist_scale_factor_field[field][i ^ field] =
                    get_scale_factor(sl, poc, poc1, i + 16);
        }

    for (i = 0; i < sl->ref_count[0]; i++)
        sl->dist_scale_factor[i] = get_scale_factor(sl, poc, poc1, i);
}

static void fill_colmap(const H264Context *h, H264SliceContext *sl,
                        int map[2][16 + 32], int list,
                        int field, int colfield, int mbafi)
{
    H264Picture *const ref1 = sl->ref_list[1][0].parent;
    int j, old_ref, rfield;
    int start  = mbafi ? 16                       : 0;
    int end    = mbafi ? 16 + 2 * sl->ref_count[0] : sl->ref_count[0];
    int interl = mbafi || h->picture_structure != PICT_FRAME;

    /* bogus; fills in for missing frames */
    memset(map[list], 0, sizeof(map[list]));

    for (rfield = 0; rfield < 2; rfield++) {
        for (old_ref = 0; old_ref < ref1->ref_count[colfield][list]; old_ref++) {
            int poc = ref1->ref_poc[colfield][list][old_ref];

            if (!interl)
                poc |= 3;
            // FIXME: store all MBAFF references so this is not needed
            else if (interl && (poc & 3) == 3)
                poc = (poc & ~3) + rfield + 1;

            for (j = start; j < end; j++) {
                if (4 * sl->ref_list[0][j].parent->frame_num +
                    (sl->ref_list[0][j].reference & 3) == poc) {
                    int cur_ref = mbafi ? (j - 16) ^ field : j;
                    if (ref1->mbaff)
                        map[list][2 * old_ref + (rfield ^ field) + 16] = cur_ref;
                    if (rfield == field || !interl)
                        map[list][old_ref] = cur_ref;
                    break;
                }
            }
        }
    }
}

void ff_h264_direct_ref_list_init(const H264Context *const h, H264SliceContext *sl)
{
    H264Ref *const ref1 = &sl->ref_list[1][0];
    H264Picture *const cur = h->cur_pic_ptr;
    int list, j, field;
    int sidx     = (h->picture_structure & 1) ^ 1;
    int ref1sidx = (ref1->reference      & 1) ^ 1;

    for (list = 0; list < sl->list_count; list++) {
        cur->ref_count[sidx][list] = sl->ref_count[list];
        for (j = 0; j < sl->ref_count[list]; j++)
            cur->ref_poc[sidx][list][j] = 4 * sl->ref_list[list][j].parent->frame_num +
                                          (sl->ref_list[list][j].reference & 3);
    }

    if (h->picture_structure == PICT_FRAME) {
        memcpy(cur->ref_count[1], cur->ref_count[0], sizeof(cur->ref_count[0]));
        memcpy(cur->ref_poc[1],   cur->ref_poc[0],   sizeof(cur->ref_poc[0]));
    }

    if (h->current_slice == 0) {
        cur->mbaff = FRAME_MBAFF(h);
    } else {
        av_assert0(cur->mbaff == FRAME_MBAFF(h));
    }

    sl->col_fieldoff = 0;

    if (sl->list_count != 2 || !sl->ref_count[1])
        return;

    if (h->picture_structure == PICT_FRAME) {
        int cur_poc  = h->cur_pic_ptr->poc;
        int *col_poc = sl->ref_list[1][0].parent->field_poc;
        if (col_poc[0] == INT_MAX && col_poc[1] == INT_MAX) {
            av_log(h->avctx, AV_LOG_ERROR, "co located POCs unavailable\n");
            sl->col_parity = 1;
        } else
            sl->col_parity = (FFABS(col_poc[0] - (int64_t)cur_poc) >=
                              FFABS(col_poc[1] - (int64_t)cur_poc));
        ref1sidx =
        sidx     = sl->col_parity;
    // FL -> FL & differ parity
    } else if (!(h->picture_structure & sl->ref_list[1][0].reference) &&
               !sl->ref_list[1][0].parent->mbaff) {
        sl->col_fieldoff = 2 * sl->ref_list[1][0].reference - 3;
    }

    if (sl->slice_type_nos != AV_PICTURE_TYPE_B || sl->direct_spatial_mv_pred)
        return;

    for (list = 0; list < 2; list++) {
        fill_colmap(h, sl, sl->map_col_to_list0, list, sidx, ref1sidx, 0);
        if (FRAME_MBAFF(h))
            for (field = 0; field < 2; field++)
                fill_colmap(h, sl, sl->map_col_to_list0_field[field], list, field,
                            field, 1);
    }
}

void ff_thread_await_progress(ThreadFrame *f, int n, int field)
{
    PerThreadContext *p;
    atomic_int *progress = f->progress ? (atomic_int*)f->progress->data : NULL;

    if (!progress ||
        atomic_load_explicit(&progress[field], memory_order_acquire) >= n)
        return;

    p = f->owner[field]->internal->thread_ctx;

    if (atomic_load_explicit(&p->debug_threads, memory_order_relaxed))
        av_log(f->owner[field], AV_LOG_DEBUG,
               "thread awaiting %d field %d from %p\n", n, field, progress);

    pthread_mutex_lock(&p->progress_mutex);
    while (atomic_load_explicit(&progress[field], memory_order_relaxed) < n)
        pthread_cond_wait(&p->progress_cond, &p->progress_mutex);
    pthread_mutex_unlock(&p->progress_mutex);
}


static void await_reference_mb_row(const H264Context *const h, H264Ref *ref,
                                   int mb_y)
{
    int ref_field         = ref->reference - 1;
    int ref_field_picture = ref->parent->field_picture;
    int ref_height        = 16 * h->mb_height >> ref_field_picture;

    if (!HAVE_THREADS || !(h->avctx->active_thread_type & FF_THREAD_FRAME))
        return;

    /* FIXME: It can be safe to access mb stuff
     * even if pixels aren't deblocked yet. */

    ff_thread_await_progress(&ref->parent->tf,
                             FFMIN(16 * mb_y >> ref_field_picture,
                                   ref_height - 1),
                             ref_field_picture && ref_field);
}

static void pred_spatial_direct_motion(const H264Context *const h, H264SliceContext *sl,
                                       int *mb_type)
{
    int b8_stride = 2;
    int b4_stride = h->b_stride;
    int mb_xy = sl->mb_xy, mb_y = sl->mb_y;
    int mb_type_col[2];
    const int16_t (*l1mv0)[2], (*l1mv1)[2];
    const int8_t *l1ref0, *l1ref1;
    const int is_b8x8 = IS_8X8(*mb_type);
    unsigned int sub_mb_type = MB_TYPE_L0L1;
    int i8, i4;
    int ref[2];
    int mv[2];
    int list;

    assert(sl->ref_list[1][0].reference & 3);

    await_reference_mb_row(h, &sl->ref_list[1][0],
                           sl->mb_y + !!IS_INTERLACED(*mb_type));

#define MB_TYPE_16x16_OR_INTRA (MB_TYPE_16x16 | MB_TYPE_INTRA4x4 | \
                                MB_TYPE_INTRA16x16 | MB_TYPE_INTRA_PCM)

    /* ref = min(neighbors) */
    for (list = 0; list < 2; list++) {
        int left_ref     = sl->ref_cache[list][scan8[0] - 1];
        int top_ref      = sl->ref_cache[list][scan8[0] - 8];
        int refc         = sl->ref_cache[list][scan8[0] - 8 + 4];
        const int16_t *C = sl->mv_cache[list][scan8[0]  - 8 + 4];
        if (refc == PART_NOT_AVAILABLE) {
            refc = sl->ref_cache[list][scan8[0] - 8 - 1];
            C    = sl->mv_cache[list][scan8[0]  - 8 - 1];
        }
        ref[list] = FFMIN3((unsigned)left_ref,
                           (unsigned)top_ref,
                           (unsigned)refc);
        if (ref[list] >= 0) {
            /* This is just pred_motion() but with the cases removed that
             * cannot happen for direct blocks. */
            const int16_t *const A = sl->mv_cache[list][scan8[0] - 1];
            const int16_t *const B = sl->mv_cache[list][scan8[0] - 8];

            int match_count = (left_ref == ref[list]) +
                              (top_ref  == ref[list]) +
                              (refc     == ref[list]);

            if (match_count > 1) { // most common
                mv[list] = pack16to32(mid_pred(A[0], B[0], C[0]),
                                      mid_pred(A[1], B[1], C[1]));
            } else {
                assert(match_count == 1);
                if (left_ref == ref[list])
                    mv[list] = AV_RN32A(A);
                else if (top_ref == ref[list])
                    mv[list] = AV_RN32A(B);
                else
                    mv[list] = AV_RN32A(C);
            }
            av_assert2(ref[list] < (sl->ref_count[list] << !!FRAME_MBAFF(h)));
        } else {
            int mask = ~(MB_TYPE_L0 << (2 * list));
            mv[list]  = 0;
            ref[list] = -1;
            if (!is_b8x8)
                *mb_type &= mask;
            sub_mb_type &= mask;
        }
    }
    if (ref[0] < 0 && ref[1] < 0) {
        ref[0] = ref[1] = 0;
        if (!is_b8x8)
            *mb_type |= MB_TYPE_L0L1;
        sub_mb_type |= MB_TYPE_L0L1;
    }

    if (!(is_b8x8 | mv[0] | mv[1])) {
        fill_rectangle(&sl->ref_cache[0][scan8[0]], 4, 4, 8, (uint8_t)ref[0], 1);
        fill_rectangle(&sl->ref_cache[1][scan8[0]], 4, 4, 8, (uint8_t)ref[1], 1);
        fill_rectangle(&sl->mv_cache[0][scan8[0]], 4, 4, 8, 0, 4);
        fill_rectangle(&sl->mv_cache[1][scan8[0]], 4, 4, 8, 0, 4);
        *mb_type = (*mb_type & ~(MB_TYPE_8x8 | MB_TYPE_16x8 | MB_TYPE_8x16 |
                                 MB_TYPE_P1L0 | MB_TYPE_P1L1)) |
                   MB_TYPE_16x16 | MB_TYPE_DIRECT2;
        return;
    }

    if (IS_INTERLACED(sl->ref_list[1][0].parent->mb_type[mb_xy])) { // AFL/AFR/FR/FL -> AFL/FL
        if (!IS_INTERLACED(*mb_type)) {                    //     AFR/FR    -> AFL/FL
            mb_y  = (sl->mb_y & ~1) + sl->col_parity;
            mb_xy = sl->mb_x +
                    ((sl->mb_y & ~1) + sl->col_parity) * h->mb_stride;
            b8_stride = 0;
        } else {
            mb_y  += sl->col_fieldoff;
            mb_xy += h->mb_stride * sl->col_fieldoff; // non-zero for FL -> FL & differ parity
        }
        goto single_col;
    } else {                                             // AFL/AFR/FR/FL -> AFR/FR
        if (IS_INTERLACED(*mb_type)) {                   // AFL       /FL -> AFR/FR
            mb_y           =  sl->mb_y & ~1;
            mb_xy          = (sl->mb_y & ~1) * h->mb_stride + sl->mb_x;
            mb_type_col[0] = sl->ref_list[1][0].parent->mb_type[mb_xy];
            mb_type_col[1] = sl->ref_list[1][0].parent->mb_type[mb_xy + h->mb_stride];
            b8_stride      = 2 + 4 * h->mb_stride;
            b4_stride     *= 6;
            if (IS_INTERLACED(mb_type_col[0]) !=
                IS_INTERLACED(mb_type_col[1])) {
                mb_type_col[0] &= ~MB_TYPE_INTERLACED;
                mb_type_col[1] &= ~MB_TYPE_INTERLACED;
            }

            sub_mb_type |= MB_TYPE_16x16 | MB_TYPE_DIRECT2; /* B_SUB_8x8 */
            if ((mb_type_col[0] & MB_TYPE_16x16_OR_INTRA) &&
                (mb_type_col[1] & MB_TYPE_16x16_OR_INTRA) &&
                !is_b8x8) {
                *mb_type |= MB_TYPE_16x8 | MB_TYPE_DIRECT2;  /* B_16x8 */
            } else {
                *mb_type |= MB_TYPE_8x8;
            }
        } else {                                         //     AFR/FR    -> AFR/FR
single_col:
            mb_type_col[0] =
            mb_type_col[1] = sl->ref_list[1][0].parent->mb_type[mb_xy];

            sub_mb_type |= MB_TYPE_16x16 | MB_TYPE_DIRECT2; /* B_SUB_8x8 */
            if (!is_b8x8 && (mb_type_col[0] & MB_TYPE_16x16_OR_INTRA)) {
                *mb_type |= MB_TYPE_16x16 | MB_TYPE_DIRECT2; /* B_16x16 */
            } else if (!is_b8x8 &&
                       (mb_type_col[0] & (MB_TYPE_16x8 | MB_TYPE_8x16))) {
                *mb_type |= MB_TYPE_DIRECT2 |
                            (mb_type_col[0] & (MB_TYPE_16x8 | MB_TYPE_8x16));
            } else {
                if (!h->ps.sps->direct_8x8_inference_flag) {
                    /* FIXME: Save sub mb types from previous frames (or derive
                     * from MVs) so we know exactly what block size to use. */
                    sub_mb_type += (MB_TYPE_8x8 - MB_TYPE_16x16); /* B_SUB_4x4 */
                }
                *mb_type |= MB_TYPE_8x8;
            }
        }
    }

    await_reference_mb_row(h, &sl->ref_list[1][0], mb_y);

    l1mv0  = (void*)&sl->ref_list[1][0].parent->motion_val[0][h->mb2b_xy[mb_xy]];
    l1mv1  = (void*)&sl->ref_list[1][0].parent->motion_val[1][h->mb2b_xy[mb_xy]];
    l1ref0 = &sl->ref_list[1][0].parent->ref_index[0][4 * mb_xy];
    l1ref1 = &sl->ref_list[1][0].parent->ref_index[1][4 * mb_xy];
    if (!b8_stride) {
        if (sl->mb_y & 1) {
            l1ref0 += 2;
            l1ref1 += 2;
            l1mv0  += 2 * b4_stride;
            l1mv1  += 2 * b4_stride;
        }
    }

    if (IS_INTERLACED(*mb_type) != IS_INTERLACED(mb_type_col[0])) {
        int n = 0;
        for (i8 = 0; i8 < 4; i8++) {
            int x8  = i8 & 1;
            int y8  = i8 >> 1;
            int xy8 = x8     + y8 * b8_stride;
            int xy4 = x8 * 3 + y8 * b4_stride;
            int a, b;

            if (is_b8x8 && !IS_DIRECT(sl->sub_mb_type[i8]))
                continue;
            sl->sub_mb_type[i8] = sub_mb_type;

            fill_rectangle(&sl->ref_cache[0][scan8[i8 * 4]], 2, 2, 8,
                           (uint8_t)ref[0], 1);
            fill_rectangle(&sl->ref_cache[1][scan8[i8 * 4]], 2, 2, 8,
                           (uint8_t)ref[1], 1);
            if (!IS_INTRA(mb_type_col[y8]) && !sl->ref_list[1][0].parent->long_ref &&
                ((l1ref0[xy8] == 0 &&
                  FFABS(l1mv0[xy4][0]) <= 1 &&
                  FFABS(l1mv0[xy4][1]) <= 1) ||
                 (l1ref0[xy8] < 0 &&
                  l1ref1[xy8] == 0 &&
                  FFABS(l1mv1[xy4][0]) <= 1 &&
                  FFABS(l1mv1[xy4][1]) <= 1))) {
                a =
                b = 0;
                if (ref[0] > 0)
                    a = mv[0];
                if (ref[1] > 0)
                    b = mv[1];
                n++;
            } else {
                a = mv[0];
                b = mv[1];
            }
            fill_rectangle(&sl->mv_cache[0][scan8[i8 * 4]], 2, 2, 8, a, 4);
            fill_rectangle(&sl->mv_cache[1][scan8[i8 * 4]], 2, 2, 8, b, 4);
        }
        if (!is_b8x8 && !(n & 3))
            *mb_type = (*mb_type & ~(MB_TYPE_8x8 | MB_TYPE_16x8 | MB_TYPE_8x16 |
                                     MB_TYPE_P1L0 | MB_TYPE_P1L1)) |
                       MB_TYPE_16x16 | MB_TYPE_DIRECT2;
    } else if (IS_16X16(*mb_type)) {
        int a, b;

        fill_rectangle(&sl->ref_cache[0][scan8[0]], 4, 4, 8, (uint8_t)ref[0], 1);
        fill_rectangle(&sl->ref_cache[1][scan8[0]], 4, 4, 8, (uint8_t)ref[1], 1);
        if (!IS_INTRA(mb_type_col[0]) && !sl->ref_list[1][0].parent->long_ref &&
            ((l1ref0[0] == 0 &&
              FFABS(l1mv0[0][0]) <= 1 &&
              FFABS(l1mv0[0][1]) <= 1) ||
             (l1ref0[0] < 0 && !l1ref1[0] &&
              FFABS(l1mv1[0][0]) <= 1 &&
              FFABS(l1mv1[0][1]) <= 1 &&
              h->x264_build > 33U))) {
            a = b = 0;
            if (ref[0] > 0)
                a = mv[0];
            if (ref[1] > 0)
                b = mv[1];
        } else {
            a = mv[0];
            b = mv[1];
        }
        fill_rectangle(&sl->mv_cache[0][scan8[0]], 4, 4, 8, a, 4);
        fill_rectangle(&sl->mv_cache[1][scan8[0]], 4, 4, 8, b, 4);
    } else {
        int n = 0;
        for (i8 = 0; i8 < 4; i8++) {
            const int x8 = i8 & 1;
            const int y8 = i8 >> 1;

            if (is_b8x8 && !IS_DIRECT(sl->sub_mb_type[i8]))
                continue;
            sl->sub_mb_type[i8] = sub_mb_type;

            fill_rectangle(&sl->mv_cache[0][scan8[i8 * 4]], 2, 2, 8, mv[0], 4);
            fill_rectangle(&sl->mv_cache[1][scan8[i8 * 4]], 2, 2, 8, mv[1], 4);
            fill_rectangle(&sl->ref_cache[0][scan8[i8 * 4]], 2, 2, 8,
                           (uint8_t)ref[0], 1);
            fill_rectangle(&sl->ref_cache[1][scan8[i8 * 4]], 2, 2, 8,
                           (uint8_t)ref[1], 1);

            assert(b8_stride == 2);
            /* col_zero_flag */
            if (!IS_INTRA(mb_type_col[0]) && !sl->ref_list[1][0].parent->long_ref &&
                (l1ref0[i8] == 0 ||
                 (l1ref0[i8] < 0 &&
                  l1ref1[i8] == 0 &&
                  h->x264_build > 33U))) {
                const int16_t (*l1mv)[2] = l1ref0[i8] == 0 ? l1mv0 : l1mv1;
                if (IS_SUB_8X8(sub_mb_type)) {
                    const int16_t *mv_col = l1mv[x8 * 3 + y8 * 3 * b4_stride];
                    if (FFABS(mv_col[0]) <= 1 && FFABS(mv_col[1]) <= 1) {
                        if (ref[0] == 0)
                            fill_rectangle(&sl->mv_cache[0][scan8[i8 * 4]], 2, 2,
                                           8, 0, 4);
                        if (ref[1] == 0)
                            fill_rectangle(&sl->mv_cache[1][scan8[i8 * 4]], 2, 2,
                                           8, 0, 4);
                        n += 4;
                    }
                } else {
                    int m = 0;
                    for (i4 = 0; i4 < 4; i4++) {
                        const int16_t *mv_col = l1mv[x8 * 2 + (i4 & 1) +
                                                     (y8 * 2 + (i4 >> 1)) * b4_stride];
                        if (FFABS(mv_col[0]) <= 1 && FFABS(mv_col[1]) <= 1) {
                            if (ref[0] == 0)
                                AV_ZERO32(sl->mv_cache[0][scan8[i8 * 4 + i4]]);
                            if (ref[1] == 0)
                                AV_ZERO32(sl->mv_cache[1][scan8[i8 * 4 + i4]]);
                            m++;
                        }
                    }
                    if (!(m & 3))
                        sl->sub_mb_type[i8] += MB_TYPE_16x16 - MB_TYPE_8x8;
                    n += m;
                }
            }
        }
        if (!is_b8x8 && !(n & 15))
            *mb_type = (*mb_type & ~(MB_TYPE_8x8 | MB_TYPE_16x8 | MB_TYPE_8x16 |
                                     MB_TYPE_P1L0 | MB_TYPE_P1L1)) |
                       MB_TYPE_16x16 | MB_TYPE_DIRECT2;
    }
}

static void pred_temp_direct_motion(const H264Context *const h, H264SliceContext *sl,
                                    int *mb_type)
{
    int b8_stride = 2;
    int b4_stride = h->b_stride;
    int mb_xy = sl->mb_xy, mb_y = sl->mb_y;
    int mb_type_col[2];
    const int16_t (*l1mv0)[2], (*l1mv1)[2];
    const int8_t *l1ref0, *l1ref1;
    const int is_b8x8 = IS_8X8(*mb_type);
    unsigned int sub_mb_type;
    int i8, i4;

    assert(sl->ref_list[1][0].reference & 3);

    await_reference_mb_row(h, &sl->ref_list[1][0],
                           sl->mb_y + !!IS_INTERLACED(*mb_type));

    if (IS_INTERLACED(sl->ref_list[1][0].parent->mb_type[mb_xy])) { // AFL/AFR/FR/FL -> AFL/FL
        if (!IS_INTERLACED(*mb_type)) {                    //     AFR/FR    -> AFL/FL
            mb_y  = (sl->mb_y & ~1) + sl->col_parity;
            mb_xy = sl->mb_x +
                    ((sl->mb_y & ~1) + sl->col_parity) * h->mb_stride;
            b8_stride = 0;
        } else {
            mb_y  += sl->col_fieldoff;
            mb_xy += h->mb_stride * sl->col_fieldoff; // non-zero for FL -> FL & differ parity
        }
        goto single_col;
    } else {                                        // AFL/AFR/FR/FL -> AFR/FR
        if (IS_INTERLACED(*mb_type)) {              // AFL       /FL -> AFR/FR
            mb_y           = sl->mb_y & ~1;
            mb_xy          = sl->mb_x + (sl->mb_y & ~1) * h->mb_stride;
            mb_type_col[0] = sl->ref_list[1][0].parent->mb_type[mb_xy];
            mb_type_col[1] = sl->ref_list[1][0].parent->mb_type[mb_xy + h->mb_stride];
            b8_stride      = 2 + 4 * h->mb_stride;
            b4_stride     *= 6;
            if (IS_INTERLACED(mb_type_col[0]) !=
                IS_INTERLACED(mb_type_col[1])) {
                mb_type_col[0] &= ~MB_TYPE_INTERLACED;
                mb_type_col[1] &= ~MB_TYPE_INTERLACED;
            }

            sub_mb_type = MB_TYPE_16x16 | MB_TYPE_P0L0 | MB_TYPE_P0L1 |
                          MB_TYPE_DIRECT2;                  /* B_SUB_8x8 */

            if ((mb_type_col[0] & MB_TYPE_16x16_OR_INTRA) &&
                (mb_type_col[1] & MB_TYPE_16x16_OR_INTRA) &&
                !is_b8x8) {
                *mb_type |= MB_TYPE_16x8 | MB_TYPE_L0L1 |
                            MB_TYPE_DIRECT2;                /* B_16x8 */
            } else {
                *mb_type |= MB_TYPE_8x8 | MB_TYPE_L0L1;
            }
        } else {                                    //     AFR/FR    -> AFR/FR
single_col:
            mb_type_col[0]     =
                mb_type_col[1] = sl->ref_list[1][0].parent->mb_type[mb_xy];

            sub_mb_type = MB_TYPE_16x16 | MB_TYPE_P0L0 | MB_TYPE_P0L1 |
                          MB_TYPE_DIRECT2;                  /* B_SUB_8x8 */
            if (!is_b8x8 && (mb_type_col[0] & MB_TYPE_16x16_OR_INTRA)) {
                *mb_type |= MB_TYPE_16x16 | MB_TYPE_P0L0 | MB_TYPE_P0L1 |
                            MB_TYPE_DIRECT2;                /* B_16x16 */
            } else if (!is_b8x8 &&
                       (mb_type_col[0] & (MB_TYPE_16x8 | MB_TYPE_8x16))) {
                *mb_type |= MB_TYPE_L0L1 | MB_TYPE_DIRECT2 |
                            (mb_type_col[0] & (MB_TYPE_16x8 | MB_TYPE_8x16));
            } else {
                if (!h->ps.sps->direct_8x8_inference_flag) {
                    /* FIXME: save sub mb types from previous frames (or derive
                     * from MVs) so we know exactly what block size to use */
                    sub_mb_type = MB_TYPE_8x8 | MB_TYPE_P0L0 | MB_TYPE_P0L1 |
                                  MB_TYPE_DIRECT2;          /* B_SUB_4x4 */
                }
                *mb_type |= MB_TYPE_8x8 | MB_TYPE_L0L1;
            }
        }
    }

    await_reference_mb_row(h, &sl->ref_list[1][0], mb_y);

    l1mv0  = (void*)&sl->ref_list[1][0].parent->motion_val[0][h->mb2b_xy[mb_xy]];
    l1mv1  = (void*)&sl->ref_list[1][0].parent->motion_val[1][h->mb2b_xy[mb_xy]];
    l1ref0 = &sl->ref_list[1][0].parent->ref_index[0][4 * mb_xy];
    l1ref1 = &sl->ref_list[1][0].parent->ref_index[1][4 * mb_xy];
    if (!b8_stride) {
        if (sl->mb_y & 1) {
            l1ref0 += 2;
            l1ref1 += 2;
            l1mv0  += 2 * b4_stride;
            l1mv1  += 2 * b4_stride;
        }
    }

    {
        const int *map_col_to_list0[2] = { sl->map_col_to_list0[0],
                                           sl->map_col_to_list0[1] };
        const int *dist_scale_factor = sl->dist_scale_factor;
        int ref_offset;

        if (FRAME_MBAFF(h) && IS_INTERLACED(*mb_type)) {
            map_col_to_list0[0] = sl->map_col_to_list0_field[sl->mb_y & 1][0];
            map_col_to_list0[1] = sl->map_col_to_list0_field[sl->mb_y & 1][1];
            dist_scale_factor   = sl->dist_scale_factor_field[sl->mb_y & 1];
        }
        ref_offset = (sl->ref_list[1][0].parent->mbaff << 4) & (mb_type_col[0] >> 3);

        if (IS_INTERLACED(*mb_type) != IS_INTERLACED(mb_type_col[0])) {
            int y_shift = 2 * !IS_INTERLACED(*mb_type);
            assert(h->ps.sps->direct_8x8_inference_flag);

            for (i8 = 0; i8 < 4; i8++) {
                const int x8 = i8 & 1;
                const int y8 = i8 >> 1;
                int ref0, scale;
                const int16_t (*l1mv)[2] = l1mv0;

                if (is_b8x8 && !IS_DIRECT(sl->sub_mb_type[i8]))
                    continue;
                sl->sub_mb_type[i8] = sub_mb_type;

                fill_rectangle(&sl->ref_cache[1][scan8[i8 * 4]], 2, 2, 8, 0, 1);
                if (IS_INTRA(mb_type_col[y8])) {
                    fill_rectangle(&sl->ref_cache[0][scan8[i8 * 4]], 2, 2, 8, 0, 1);
                    fill_rectangle(&sl->mv_cache[0][scan8[i8 * 4]], 2, 2, 8, 0, 4);
                    fill_rectangle(&sl->mv_cache[1][scan8[i8 * 4]], 2, 2, 8, 0, 4);
                    continue;
                }

                ref0 = l1ref0[x8 + y8 * b8_stride];
                if (ref0 >= 0)
                    ref0 = map_col_to_list0[0][ref0 + ref_offset];
                else {
                    ref0 = map_col_to_list0[1][l1ref1[x8 + y8 * b8_stride] +
                                               ref_offset];
                    l1mv = l1mv1;
                }
                scale = dist_scale_factor[ref0];
                fill_rectangle(&sl->ref_cache[0][scan8[i8 * 4]], 2, 2, 8,
                               ref0, 1);

                {
                    const int16_t *mv_col = l1mv[x8 * 3 + y8 * b4_stride];
                    int my_col            = (mv_col[1] * (1 << y_shift)) / 2;
                    int mx                = (scale * mv_col[0] + 128) >> 8;
                    int my                = (scale * my_col    + 128) >> 8;
                    fill_rectangle(&sl->mv_cache[0][scan8[i8 * 4]], 2, 2, 8,
                                   pack16to32(mx, my), 4);
                    fill_rectangle(&sl->mv_cache[1][scan8[i8 * 4]], 2, 2, 8,
                                   pack16to32(mx - mv_col[0], my - my_col), 4);
                }
            }
            return;
        }

        /* one-to-one mv scaling */

        if (IS_16X16(*mb_type)) {
            int ref, mv0, mv1;

            fill_rectangle(&sl->ref_cache[1][scan8[0]], 4, 4, 8, 0, 1);
            if (IS_INTRA(mb_type_col[0])) {
                ref = mv0 = mv1 = 0;
            } else {
                const int ref0 = l1ref0[0] >= 0 ? map_col_to_list0[0][l1ref0[0] + ref_offset]
                                                : map_col_to_list0[1][l1ref1[0] + ref_offset];
                const int scale = dist_scale_factor[ref0];
                const int16_t *mv_col = l1ref0[0] >= 0 ? l1mv0[0] : l1mv1[0];
                int mv_l0[2];
                mv_l0[0] = (scale * mv_col[0] + 128) >> 8;
                mv_l0[1] = (scale * mv_col[1] + 128) >> 8;
                ref      = ref0;
                mv0      = pack16to32(mv_l0[0], mv_l0[1]);
                mv1      = pack16to32(mv_l0[0] - mv_col[0], mv_l0[1] - mv_col[1]);
            }
            fill_rectangle(&sl->ref_cache[0][scan8[0]], 4, 4, 8, ref, 1);
            fill_rectangle(&sl->mv_cache[0][scan8[0]], 4, 4, 8, mv0, 4);
            fill_rectangle(&sl->mv_cache[1][scan8[0]], 4, 4, 8, mv1, 4);
        } else {
            for (i8 = 0; i8 < 4; i8++) {
                const int x8 = i8 & 1;
                const int y8 = i8 >> 1;
                int ref0, scale;
                const int16_t (*l1mv)[2] = l1mv0;

                if (is_b8x8 && !IS_DIRECT(sl->sub_mb_type[i8]))
                    continue;
                sl->sub_mb_type[i8] = sub_mb_type;
                fill_rectangle(&sl->ref_cache[1][scan8[i8 * 4]], 2, 2, 8, 0, 1);
                if (IS_INTRA(mb_type_col[0])) {
                    fill_rectangle(&sl->ref_cache[0][scan8[i8 * 4]], 2, 2, 8, 0, 1);
                    fill_rectangle(&sl->mv_cache[0][scan8[i8 * 4]], 2, 2, 8, 0, 4);
                    fill_rectangle(&sl->mv_cache[1][scan8[i8 * 4]], 2, 2, 8, 0, 4);
                    continue;
                }

                assert(b8_stride == 2);
                ref0 = l1ref0[i8];
                if (ref0 >= 0)
                    ref0 = map_col_to_list0[0][ref0 + ref_offset];
                else {
                    ref0 = map_col_to_list0[1][l1ref1[i8] + ref_offset];
                    l1mv = l1mv1;
                }
                scale = dist_scale_factor[ref0];

                fill_rectangle(&sl->ref_cache[0][scan8[i8 * 4]], 2, 2, 8,
                               ref0, 1);
                if (IS_SUB_8X8(sub_mb_type)) {
                    const int16_t *mv_col = l1mv[x8 * 3 + y8 * 3 * b4_stride];
                    int mx                = (scale * mv_col[0] + 128) >> 8;
                    int my                = (scale * mv_col[1] + 128) >> 8;
                    fill_rectangle(&sl->mv_cache[0][scan8[i8 * 4]], 2, 2, 8,
                                   pack16to32(mx, my), 4);
                    fill_rectangle(&sl->mv_cache[1][scan8[i8 * 4]], 2, 2, 8,
                                   pack16to32(mx - mv_col[0], my - mv_col[1]), 4);
                } else {
                    for (i4 = 0; i4 < 4; i4++) {
                        const int16_t *mv_col = l1mv[x8 * 2 + (i4 & 1) +
                                                     (y8 * 2 + (i4 >> 1)) * b4_stride];
                        int16_t *mv_l0 = sl->mv_cache[0][scan8[i8 * 4 + i4]];
                        mv_l0[0] = (scale * mv_col[0] + 128) >> 8;
                        mv_l0[1] = (scale * mv_col[1] + 128) >> 8;
                        AV_WN32A(sl->mv_cache[1][scan8[i8 * 4 + i4]],
                                 pack16to32(mv_l0[0] - mv_col[0],
                                            mv_l0[1] - mv_col[1]));
                    }
                }
            }
        }
    }
}

void ff_h264_pred_direct_motion(const H264Context *const h, H264SliceContext *sl,
                                int *mb_type)
{
    if (sl->direct_spatial_mv_pred)
        pred_spatial_direct_motion(h, sl, mb_type);
    else
        pred_temp_direct_motion(h, sl, mb_type);
}
static void decode_mb_skip(const H264Context *h, H264SliceContext *sl)
{
    const int mb_xy = sl->mb_xy;
    int mb_type = 0;
    memset(h->non_zero_count[mb_xy], 0, 48);
    if ((sl)->mb_field_decoding_flag)
        mb_type |= (1 << 7);
    if (sl->slice_type_nos == AV_PICTURE_TYPE_B)
    {
        mb_type |= (((1 << 12) | (1 << 13)) | ((1 << 14) | (1 << 15))) | (1 << 8) | (1 << 11);
        if (sl->direct_spatial_mv_pred)
        {
            fill_decode_neighbors(h, sl, mb_type);
            fill_decode_caches(h, sl, mb_type);
        }
        ff_h264_pred_direct_motion(h, sl, &mb_type);
        mb_type |= (1 << 11);
    }
    else
    {
        mb_type |= (1 << 3) | (1 << 12) | (1 << 13) | (1 << 11);
        fill_decode_neighbors(h, sl, mb_type);
        pred_pskip_motion(h, sl);
    }
    write_back_motion(h, sl, mb_type);
    h->cur_pic.mb_type[mb_xy] = mb_type;
    h->cur_pic.qscale_table[mb_xy] = sl->qscale;
    h->slice_table[mb_xy] = sl->slice_num;
    sl->prev_mb_skipped = 1;
}

static inline int get_ue_golomb(GetBitContext *gb)
{
    unsigned int buf;
    unsigned int re_index = (gb)->index;
    unsigned int re_cache;
    re_cache = av_bswap32((((const union unaligned_32 *)((gb)->buffer + (re_index >> 3)))->l)) << (re_index & 7);
    buf = ((uint32_t)re_cache);
    if (buf >= (1 << 27))
    {
        buf >>= 32 - 9;
        re_index += (ff_golomb_vlc_len[buf]);
        (gb)->index = re_index;
        return ff_ue_golomb_vlc_code[buf];
    }
    else
    {
        int log = 2 * av_log2(buf) - 31;
        re_index += (32 - log);
        (gb)->index = re_index;
        if (log < 7)
            return (-(int)(('I') | (('N') << 8) | (('D') << 16) | ((unsigned)('A') << 24)));
        buf >>= log;
        buf--;
        return buf;
    }
}

static inline unsigned get_ue_golomb_long(GetBitContext *gb)
{
    unsigned buf, log;
    buf = show_bits_long(gb, 32);
    log = 31 - av_log2(buf);
    skip_bits_long(gb, log);
    return get_bits_long(gb, log + 1) - 1;
}

static inline int get_ue_golomb_31(GetBitContext *gb)
{
    unsigned int buf;
    unsigned int re_index = (gb)->index;
    unsigned int re_cache;
    re_cache = av_bswap32((((const union unaligned_32 *)((gb)->buffer + (re_index >> 3)))->l)) << (re_index & 7);
    buf = ((uint32_t)re_cache);
    buf >>= 32 - 9;
    re_index += (ff_golomb_vlc_len[buf]);
    (gb)->index = re_index;
    return ff_ue_golomb_vlc_code[buf];
}

static inline unsigned get_interleaved_ue_golomb(GetBitContext *gb)
{
    uint32_t buf;
    unsigned int re_index = (gb)->index;
    unsigned int re_cache;
    re_cache = av_bswap32((((const union unaligned_32 *)((gb)->buffer + (re_index >> 3)))->l)) << (re_index & 7);
    buf = ((uint32_t)re_cache);
    if (buf & 0xAA800000)
    {
        buf >>= 32 - 8;
        re_index += (ff_interleaved_golomb_vlc_len[buf]);
        (gb)->index = re_index;
        return ff_interleaved_ue_golomb_vlc_code[buf];
    }
    else
    {
        unsigned ret = 1;
        do
        {
            buf >>= 32 - 8;
            re_index += (((ff_interleaved_golomb_vlc_len[buf]) > (8) ? (8) : (ff_interleaved_golomb_vlc_len[buf])));
            if (ff_interleaved_golomb_vlc_len[buf] != 9)
            {
                ret <<= (ff_interleaved_golomb_vlc_len[buf] - 1) >> 1;
                ret |= ff_interleaved_dirac_golomb_vlc_code[buf];
                break;
            }
            ret = (ret << 4) | ff_interleaved_dirac_golomb_vlc_code[buf];
            re_cache = av_bswap32((((const union unaligned_32 *)((gb)->buffer + (re_index >> 3)))->l)) << (re_index & 7);
            buf = ((uint32_t)re_cache);
        } while (ret < 0x8000000U && 1);
        (gb)->index = re_index;
        return ret - 1;
    }
}

static inline int get_te0_golomb(GetBitContext *gb, int range)
{
    ((void)0);
    if (range == 1)
        return 0;
    else if (range == 2)
        return get_bits1(gb) ^ 1;
    else
        return get_ue_golomb(gb);
}

static inline int get_te_golomb(GetBitContext *gb, int range)
{
    ((void)0);
    if (range == 2)
        return get_bits1(gb) ^ 1;
    else
        return get_ue_golomb(gb);
}

static inline int get_se_golomb(GetBitContext *gb)
{
    unsigned int buf;
    unsigned int re_index = (gb)->index;
    unsigned int re_cache;
    re_cache = av_bswap32((((const union unaligned_32 *)((gb)->buffer + (re_index >> 3)))->l)) << (re_index & 7);
    buf = ((uint32_t)re_cache);
    if (buf >= (1 << 27))
    {
        buf >>= 32 - 9;
        re_index += (ff_golomb_vlc_len[buf]);
        (gb)->index = re_index;
        return ff_se_golomb_vlc_code[buf];
    }
    else
    {
        int log = av_log2(buf), sign;
        re_index += (31 - log);
        re_cache = av_bswap32((((const union unaligned_32 *)((gb)->buffer + (re_index >> 3)))->l)) << (re_index & 7);
        buf = ((uint32_t)re_cache);
        buf >>= log;
        re_index += (32 - log);
        (gb)->index = re_index;
        sign = -(buf & 1);
        buf = ((buf >> 1) ^ sign) - sign;
        return buf;
    }
}

static inline int get_se_golomb_long(GetBitContext *gb)
{
    unsigned int buf = get_ue_golomb_long(gb);
    int sign = (buf & 1) - 1;
    return ((buf >> 1) ^ sign) + 1;
}

static inline int get_interleaved_se_golomb(GetBitContext *gb)
{
    unsigned int buf;
    unsigned int re_index = (gb)->index;
    unsigned int re_cache;
    re_cache = av_bswap32((((const union unaligned_32 *)((gb)->buffer + (re_index >> 3)))->l)) << (re_index & 7);
    buf = ((uint32_t)re_cache);
    if (buf & 0xAA800000)
    {
        buf >>= 32 - 8;
        re_index += (ff_interleaved_golomb_vlc_len[buf]);
        (gb)->index = re_index;
        return ff_interleaved_se_golomb_vlc_code[buf];
    }
    else
    {
        int log;
        re_index += (8);
        re_cache = av_bswap32((((const union unaligned_32 *)((gb)->buffer + (re_index >> 3)))->l)) << (re_index & 7);
        buf |= 1 | (((uint32_t)re_cache) >> 8);
        if ((buf & 0xAAAAAAAA) == 0)
            return 0x80000000;
        for (log = 31; (buf & 0x80000000) == 0; log--)
            buf = (buf << 2) - ((buf << log) >> (log - 1)) + (buf >> 30);
        re_index += (63 - 2 * log - 8);
        (gb)->index = re_index;
        return (signed)(((((buf << log) >> log) - 1) ^ -(buf & 0x1)) + 1) >> 1;
    }
}

static inline int dirac_get_se_golomb(GetBitContext *gb)
{
    uint32_t ret = get_interleaved_ue_golomb(gb);
    if (ret)
    {
        int sign = -get_bits1(gb);
        ret = (ret ^ sign) - sign;
    }
    return ret;
}

static inline int get_ur_golomb(GetBitContext *gb, int k, int limit,
                                int esc_len)
{
    unsigned int buf;
    int log;
    unsigned int re_index = (gb)->index;
    unsigned int re_cache;
    re_cache = av_bswap32((((const union unaligned_32 *)((gb)->buffer + (re_index >> 3)))->l)) << (re_index & 7);
    buf = ((uint32_t)re_cache);
    log = av_log2(buf);
    if (log > 31 - limit)
    {
        buf >>= log - k;
        buf += (30U - log) << k;
        re_index += (32 + k - log);
        (gb)->index = re_index;
        return buf;
    }
    else
    {
        re_index += (limit);
        re_cache = av_bswap32((((const union unaligned_32 *)((gb)->buffer + (re_index >> 3)))->l)) << (re_index & 7);
        buf = NEG_USR32(re_cache, esc_len);
        re_index += (esc_len);
        (gb)->index = re_index;
        return buf + limit - 1;
    }
}

static inline int get_ur_golomb_jpegls(GetBitContext *gb, int k, int limit,
                                       int esc_len)
{
    unsigned int buf;
    int log;
    unsigned int re_index = (gb)->index;
    unsigned int re_cache;
    re_cache = av_bswap32((((const union unaligned_32 *)((gb)->buffer + (re_index >> 3)))->l)) << (re_index & 7);
    buf = ((uint32_t)re_cache);
    log = av_log2(buf);
    ((void)0);
    if (log - k >= 32 - 25 + (25 == 32) &&
        32 - log < limit)
    {
        buf >>= log - k;
        buf += (30U - log) << k;
        re_index += (32 + k - log);
        (gb)->index = re_index;
        return buf;
    }
    else
    {
        int i;
        for (i = 0; i + 25 <= limit && NEG_USR32(re_cache, 25) == 0; i += 25)
        {
            if (gb->size_in_bits <= re_index)
            {
                (gb)->index = re_index;
                return -1;
            }
            re_index += (25);
            re_cache = av_bswap32((((const union unaligned_32 *)((gb)->buffer + (re_index >> 3)))->l)) << (re_index & 7);
        }
        for (; i < limit && NEG_USR32(re_cache, 1) == 0; i++)
        {
            do
            {
                re_cache <<= (1);
                re_index += (1);
            } while (0);
        }
        re_index += (1);
        re_cache = av_bswap32((((const union unaligned_32 *)((gb)->buffer + (re_index >> 3)))->l)) << (re_index & 7);
        if (i < limit - 1)
        {
            if (k)
            {
                if (k > 25 - 1)
                {
                    buf = NEG_USR32(re_cache, 16) << (k - 16);
                    re_index += (16);
                    re_cache = av_bswap32((((const union unaligned_32 *)((gb)->buffer + (re_index >> 3)))->l)) << (re_index & 7);
                    buf |= NEG_USR32(re_cache, k - 16);
                    re_index += (k - 16);
                }
                else
                {
                    buf = NEG_USR32(re_cache, k);
                    re_index += (k);
                }
            }
            else
            {
                buf = 0;
            }
            buf += ((SUINT)i << k);
        }
        else if (i == limit - 1)
        {
            buf = NEG_USR32(re_cache, esc_len);
            re_index += (esc_len);
            buf++;
        }
        else
        {
            buf = -1;
        }
        (gb)->index = re_index;
        return buf;
    }
}

static inline int get_sr_golomb(GetBitContext *gb, int k, int limit,
                                int esc_len)
{
    unsigned v = get_ur_golomb(gb, k, limit, esc_len);
    return (v >> 1) ^ -(v & 1);
}

static inline int get_sr_golomb_flac(GetBitContext *gb, int k, int limit,
                                     int esc_len)
{
    unsigned v = get_ur_golomb_jpegls(gb, k, limit, esc_len);
    return (v >> 1) ^ -(v & 1);
}

static inline unsigned int get_ur_golomb_shorten(GetBitContext *gb, int k)
{
    return get_ur_golomb_jpegls(gb, k, 0x7fffffff, 0);
}

static inline int get_sr_golomb_shorten(GetBitContext *gb, int k)
{
    int uvar = get_ur_golomb_jpegls(gb, k + 1, 0x7fffffff, 0);
    return (uvar >> 1) ^ -(uvar & 1);
}
static inline void set_ue_golomb(PutBitContext *pb, int i)
{
    ((void)0);
    ((void)0);
    if (i < 256)
        put_bits(pb, ff_ue_golomb_len[i], i + 1);
    else
    {
        int e = av_log2(i + 1);
        put_bits(pb, 2 * e + 1, i + 1);
    }
}

static inline void set_ue_golomb_long(PutBitContext *pb, uint32_t i)
{
    ((void)0);
    if (i < 256)
        put_bits(pb, ff_ue_golomb_len[i], i + 1);
    else
    {
        int e = av_log2(i + 1);
        put_bits64(pb, 2 * e + 1, i + 1);
    }
}

static inline void set_te_golomb(PutBitContext *pb, int i, int range)
{
    ((void)0);
    ((void)0);
    if (range == 2)
        put_bits(pb, 1, i ^ 1);
    else
        set_ue_golomb(pb, i);
}

static inline void set_se_golomb(PutBitContext *pb, int i)
{
    i = 2 * i - 1;
    if (i < 0)
        i ^= -1;
    set_ue_golomb(pb, i);
}

static inline void set_ur_golomb(PutBitContext *pb, int i, int k, int limit,
                                 int esc_len)
{
    int e;
    ((void)0);
    e = i >> k;
    if (e < limit)
        put_bits(pb, e + k + 1, (1 << k) + av_mod_uintp2_c(i, k));
    else
        put_bits(pb, limit + esc_len, i - limit + 1);
}

static inline void set_ur_golomb_jpegls(PutBitContext *pb, int i, int k,
                                        int limit, int esc_len)
{
    int e;
    ((void)0);
    e = (i >> k) + 1;
    if (e < limit)
    {
        while (e > 31)
        {
            put_bits(pb, 31, 0);
            e -= 31;
        }
        put_bits(pb, e, 1);
        if (k)
            put_sbits(pb, k, i);
    }
    else
    {
        while (limit > 31)
        {
            put_bits(pb, 31, 0);
            limit -= 31;
        }
        put_bits(pb, limit, 1);
        put_bits(pb, esc_len, i - 1);
    }
}

static inline void set_sr_golomb(PutBitContext *pb, int i, int k, int limit,
                                 int esc_len)
{
    int v;
    v = -2 * i - 1;
    v ^= (v >> 31);
    set_ur_golomb(pb, v, k, limit, esc_len);
}

static inline void set_sr_golomb_flac(PutBitContext *pb, int i, int k,
                                      int limit, int esc_len)
{
    int v;
    v = -2 * i - 1;
    v ^= (v >> 31);
    set_ur_golomb_jpegls(pb, v, k, limit, esc_len);
}

const uint16_t ff_h264_mb_sizes[4] = {256, 384, 512, 768};

int avpriv_h264_has_num_reorder_frames(AVCodecContext *avctx)
{
    H264Context *h = avctx->priv_data;
    return h && h->ps.sps ? h->ps.sps->num_reorder_frames : 0;
}

static void h264_er_decode_mb(void *opaque, int ref, int mv_dir, int mv_type,
                              int (*mv)[2][4][2],
                              int mb_x, int mb_y, int mb_intra, int mb_skipped)
{
    H264Context *h = opaque;
    H264SliceContext *sl = &h->slice_ctx[0];
    sl->mb_x = mb_x;
    sl->mb_y = mb_y;
    sl->mb_xy = mb_x + mb_y * h->mb_stride;
    memset(sl->non_zero_count_cache, 0, sizeof(sl->non_zero_count_cache));
    ((void)0);
    if (ref >= sl->ref_count[0])
        ref = 0;
    if (!sl->ref_list[0][ref].data[0])
    {
        av_log(h->avctx, 48, "Reference not available for error concealing\n");
        ref = 0;
    }
    if ((sl->ref_list[0][ref].reference & 3) != 3)
    {
        av_log(h->avctx, 48, "Reference invalid\n");
        return;
    }
    h264_fill_rectangle(&h->cur_pic.ref_index[0][4 * sl->mb_xy],
                   2, 2, 2, ref, 1);
    h264_fill_rectangle(&sl->ref_cache[0][scan8[0]], 4, 4, 8, ref, 1);
    h264_fill_rectangle(sl->mv_cache[0][scan8[0]], 4, 4, 8,
                   pack16to32((*mv)[0][0][0], (*mv)[0][0][1]), 4);
    sl->mb_mbaff =
        sl->mb_field_decoding_flag = 0;
    ff_h264_hl_decode_mb(h, &h->slice_ctx[0]);
}

void ff_h264_draw_horiz_band(const H264Context *h, H264SliceContext *sl,
                             int y, int height)
{
    AVCodecContext *avctx = h->avctx;
    const AVFrame *src = h->cur_pic.f;
    const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(avctx->pix_fmt);
    int vshift = desc->log2_chroma_h;
    const int field_pic = h->picture_structure != 3;
    if (field_pic)
    {
        height <<= 1;
        y <<= 1;
    }
    height = ((height) > (avctx->height - y) ? (avctx->height - y) : (height));
    if (field_pic && h->first_field && !(avctx->slice_flags & 0x0002))
        return;
    if (avctx->draw_horiz_band)
    {
        int offset[8];
        int i;
        offset[0] = y * src->linesize[0];
        offset[1] =
            offset[2] = (y >> vshift) * src->linesize[1];
        for (i = 3; i < 8; i++)
            offset[i] = 0;
        emms_c();
        avctx->draw_horiz_band(avctx, src, offset,
                               y, h->picture_structure, height);
    }
}

void ff_h264_free_tables(H264Context *h)
{
    int i;
    av_freep(&h->intra4x4_pred_mode);
    av_freep(&h->chroma_pred_mode_table);
    av_freep(&h->cbp_table);
    av_freep(&h->mvd_table[0]);
    av_freep(&h->mvd_table[1]);
    av_freep(&h->direct_table);
    av_freep(&h->non_zero_count);
    av_freep(&h->slice_table_base);
    h->slice_table =
        ((void *)0);
    av_freep(&h->list_counts);
    av_freep(&h->mb2b_xy);
    av_freep(&h->mb2br_xy);
    av_buffer_pool_uninit(&h->qscale_table_pool);
    av_buffer_pool_uninit(&h->mb_type_pool);
    av_buffer_pool_uninit(&h->motion_val_pool);
    av_buffer_pool_uninit(&h->ref_index_pool);
    for (i = 0; i < h->nb_slice_ctx; i++)
    {
        H264SliceContext *sl = &h->slice_ctx[i];
        av_freep(&sl->dc_val_base);
        av_freep(&sl->er.mb_index2xy);
        av_freep(&sl->er.error_status_table);
        av_freep(&sl->er.er_temp_buffer);
        av_freep(&sl->bipred_scratchpad);
        av_freep(&sl->edge_emu_buffer);
        av_freep(&sl->top_borders[0]);
        av_freep(&sl->top_borders[1]);
        sl->bipred_scratchpad_allocated = 0;
        sl->edge_emu_buffer_allocated = 0;
        sl->top_borders_allocated[0] = 0;
        sl->top_borders_allocated[1] = 0;
    }
}

int ff_h264_alloc_tables(H264Context *h)
{
    const int big_mb_num = h->mb_stride * (h->mb_height + 1);
    const int row_mb_num = 2 * h->mb_stride * ((h->nb_slice_ctx) > (1) ? (h->nb_slice_ctx) : (1));
    const int st_size = big_mb_num + h->mb_stride;
    int x, y;
    if (!FF_ALLOCZ_TYPED_ARRAY(h->intra4x4_pred_mode, row_mb_num * 8) ||
        !FF_ALLOCZ_TYPED_ARRAY(h->non_zero_count, big_mb_num) ||
        !FF_ALLOCZ_TYPED_ARRAY(h->slice_table_base, st_size) ||
        !FF_ALLOCZ_TYPED_ARRAY(h->cbp_table, big_mb_num) ||
        !FF_ALLOCZ_TYPED_ARRAY(h->chroma_pred_mode_table, big_mb_num) ||
        !FF_ALLOCZ_TYPED_ARRAY(h->mvd_table[0], row_mb_num * 8) ||
        !FF_ALLOCZ_TYPED_ARRAY(h->mvd_table[1], row_mb_num * 8) ||
        !FF_ALLOCZ_TYPED_ARRAY(h->direct_table, big_mb_num * 4) ||
        !FF_ALLOCZ_TYPED_ARRAY(h->list_counts, big_mb_num) ||
        !FF_ALLOCZ_TYPED_ARRAY(h->mb2b_xy, big_mb_num) ||
        !FF_ALLOCZ_TYPED_ARRAY(h->mb2br_xy, big_mb_num))
        return (-(
            12));
    h->slice_ctx[0].intra4x4_pred_mode = h->intra4x4_pred_mode;
    h->slice_ctx[0].mvd_table[0] = h->mvd_table[0];
    h->slice_ctx[0].mvd_table[1] = h->mvd_table[1];
    memset(h->slice_table_base, -1,
           st_size * sizeof(*h->slice_table_base));
    h->slice_table = h->slice_table_base + h->mb_stride * 2 + 1;
    for (y = 0; y < h->mb_height; y++)
        for (x = 0; x < h->mb_width; x++)
        {
            const int mb_xy = x + y * h->mb_stride;
            const int b_xy = 4 * x + 4 * y * h->b_stride;
            h->mb2b_xy[mb_xy] = b_xy;
            h->mb2br_xy[mb_xy] = 8 * (0 ? mb_xy : (mb_xy % (2 * h->mb_stride)));
        }
    return 0;
}

int ff_h264_slice_context_init(H264Context *h, H264SliceContext *sl)
{
    ERContext *er = &sl->er;
    int mb_array_size = h->mb_height * h->mb_stride;
    int y_size = (2 * h->mb_width + 1) * (2 * h->mb_height + 1);
    int c_size = h->mb_stride * (h->mb_height + 1);
    int yc_size = y_size + 2 * c_size;
    int x, y, i;
    sl->ref_cache[0][scan8[5] + 1] =
        sl->ref_cache[0][scan8[7] + 1] =
            sl->ref_cache[0][scan8[13] + 1] =
                sl->ref_cache[1][scan8[5] + 1] =
                    sl->ref_cache[1][scan8[7] + 1] =
                        sl->ref_cache[1][scan8[13] + 1] = -2;
    if (sl != h->slice_ctx)
    {
        memset(er, 0, sizeof(*er));
    }
    else if (1)
    {
        const int er_size = h->mb_height * h->mb_stride * (4 * sizeof(int) + 1);
        er->avctx = h->avctx;
        er->decode_mb = h264_er_decode_mb;
        er->opaque = h;
        er->quarter_sample = 1;
        er->mb_num = h->mb_num;
        er->mb_width = h->mb_width;
        er->mb_height = h->mb_height;
        er->mb_stride = h->mb_stride;
        er->b8_stride = h->mb_width * 2 + 1;
        if (!FF_ALLOCZ_TYPED_ARRAY(er->mb_index2xy, h->mb_num + 1) ||
            !FF_ALLOCZ_TYPED_ARRAY(er->error_status_table, mb_array_size) ||
            !FF_ALLOCZ_TYPED_ARRAY(er->er_temp_buffer, er_size) ||
            !FF_ALLOCZ_TYPED_ARRAY(sl->dc_val_base, yc_size))
            return (-(
                12));
        for (y = 0; y < h->mb_height; y++)
            for (x = 0; x < h->mb_width; x++)
                er->mb_index2xy[x + y * h->mb_width] = x + y * h->mb_stride;
        er->mb_index2xy[h->mb_height * h->mb_width] = (h->mb_height - 1) *
                                                          h->mb_stride +
                                                      h->mb_width;
        er->dc_val[0] = sl->dc_val_base + h->mb_width * 2 + 2;
        er->dc_val[1] = sl->dc_val_base + y_size + h->mb_stride + 1;
        er->dc_val[2] = er->dc_val[1] + c_size;
        for (i = 0; i < yc_size; i++)
            sl->dc_val_base[i] = 1024;
    }
    return 0;
}

static int h264_init_context(AVCodecContext *avctx, H264Context *h)
{
    int i;
    h->avctx = avctx;
    h->cur_chroma_format_idc = -1;
    h->width_from_caller = avctx->width;
    h->height_from_caller = avctx->height;
    h->workaround_bugs = avctx->workaround_bugs;
    h->flags = avctx->flags;
    h->poc.prev_poc_msb = 1 << 16;
    h->recovery_frame = -1;
    h->frame_recovered = 0;
    h->poc.prev_frame_num = -1;
    h->sei.frame_packing.arrangement_cancel_flag = -1;
    h->sei.unregistered.x264_build = -1;
    h->next_outputed_poc =
        (-0x7fffffff - 1);
    for (i = 0; i < 16; i++)
        h->last_pocs[i] =
            (-0x7fffffff - 1);
    ff_h264_sei_uninit(&h->sei);
    h->nb_slice_ctx = (avctx->active_thread_type & 2) ? avctx->thread_count : 1;
    h->slice_ctx = av_mallocz_array(h->nb_slice_ctx, sizeof(*h->slice_ctx));
    if (!h->slice_ctx)
    {
        h->nb_slice_ctx = 0;
        return (-(
            12));
    }
    for (i = 0; i < 36; i++)
    {
        h->DPB[i].f = av_frame_alloc();
        if (!h->DPB[i].f)
            return (-(
                12));
    }
    h->cur_pic.f = av_frame_alloc();
    if (!h->cur_pic.f)
        return (-(
            12));
    h->last_pic_for_ec.f = av_frame_alloc();
    if (!h->last_pic_for_ec.f)
        return (-(
            12));
    for (i = 0; i < h->nb_slice_ctx; i++)
        h->slice_ctx[i].h264 = h;
    return 0;
}

static int h264_decode_end(AVCodecContext *avctx)
{
    H264Context *h = avctx->priv_data;
    int i;
    ff_h264_remove_all_refs(h);
    ff_h264_free_tables(h);
    for (i = 0; i < 36; i++)
    {
        ff_h264_unref_picture(h, &h->DPB[i]);
        av_frame_free(&h->DPB[i].f);
    }
    memset(h->delayed_pic, 0, sizeof(h->delayed_pic));
    h->cur_pic_ptr =
        ((void *)0);
    av_freep(&h->slice_ctx);
    h->nb_slice_ctx = 0;
    ff_h264_sei_uninit(&h->sei);
    ff_h264_ps_uninit(&h->ps);
    ff_h2645_packet_uninit(&h->pkt);
    ff_h264_unref_picture(h, &h->cur_pic);
    av_frame_free(&h->cur_pic.f);
    ff_h264_unref_picture(h, &h->last_pic_for_ec);
    av_frame_free(&h->last_pic_for_ec.f);
    return 0;
}

static pthread_once_t h264_vlc_init = 0;

static int h264_decode_init(AVCodecContext *avctx)
{
    H264Context *h = avctx->priv_data;
    int ret;
    ret = h264_init_context(avctx, h);
    if (ret < 0)
        return ret;
    ret = pthread_once(&h264_vlc_init, ff_h264_decode_init_vlc);
    if (ret != 0)
    {
        av_log(avctx, 16, "pthread_once has failed.");
        return (-(int)(('U') | (('N') << 8) | (('K') << 16) | ((unsigned)('N') << 24)));
    }
    if (avctx->ticks_per_frame == 1)
    {
        if (h->avctx->time_base.den < 0x7fffffff / 2)
        {
            h->avctx->time_base.den *= 2;
        }
        else
            h->avctx->time_base.num /= 2;
    }
    avctx->ticks_per_frame = 2;
    if (!avctx->internal->is_copy)
    {
        if (avctx->extradata_size > 0 && avctx->extradata)
        {
            ret = ff_h264_decode_extradata(avctx->extradata, avctx->extradata_size,
                                           &h->ps, &h->is_avc, &h->nal_length_size,
                                           avctx->err_recognition, avctx);
            if (ret < 0)
            {
                int explode = avctx->err_recognition & (1 << 3);
                av_log(avctx, explode ? 16 : 24,
                       "Error decoding the extradata\n");
                if (explode)
                {
                    return ret;
                }
                ret = 0;
            }
        }
    }
    if (h->ps.sps && h->ps.sps->bitstream_restriction_flag &&
        h->avctx->has_b_frames < h->ps.sps->num_reorder_frames)
    {
        h->avctx->has_b_frames = h->ps.sps->num_reorder_frames;
    }
    ff_h264_flush_change(h);
    if (h->enable_er < 0 && (avctx->active_thread_type & 2))
        h->enable_er = 0;
    if (h->enable_er && (avctx->active_thread_type & 2))
    {
        av_log(avctx, 24,
               "Error resilience with slice threads is enabled. It is unsafe and unsupported and may crash. "
               "Use it at your own risk\n");
    }
    return 0;
}

static void idr(H264Context *h)
{
    int i;
    ff_h264_remove_all_refs(h);
    h->poc.prev_frame_num =
        h->poc.prev_frame_num_offset = 0;
    h->poc.prev_poc_msb = 1 << 16;
    h->poc.prev_poc_lsb = -1;
    for (i = 0; i < 16; i++)
        h->last_pocs[i] =
            (-0x7fffffff - 1);
}

void ff_h264_flush_change(H264Context *h)
{
    int i, j;
    h->next_outputed_poc =
        (-0x7fffffff - 1);
    h->prev_interlaced_frame = 1;
    idr(h);
    h->poc.prev_frame_num = -1;
    if (h->cur_pic_ptr)
    {
        h->cur_pic_ptr->reference = 0;
        for (j = i = 0; h->delayed_pic[i]; i++)
            if (h->delayed_pic[i] != h->cur_pic_ptr)
                h->delayed_pic[j++] = h->delayed_pic[i];
        h->delayed_pic[j] =
            ((void *)0);
    }
    ff_h264_unref_picture(h, &h->last_pic_for_ec);
    h->first_field = 0;
    h->recovery_frame = -1;
    h->frame_recovered = 0;
    h->current_slice = 0;
    h->mmco_reset = 1;
}

static void h264_decode_flush(AVCodecContext *avctx)
{
    H264Context *h = avctx->priv_data;
    int i;
    memset(h->delayed_pic, 0, sizeof(h->delayed_pic));
    ff_h264_flush_change(h);
    ff_h264_sei_uninit(&h->sei);
    for (i = 0; i < 36; i++)
        ff_h264_unref_picture(h, &h->DPB[i]);
    h->cur_pic_ptr =
        ((void *)0);
    ff_h264_unref_picture(h, &h->cur_pic);
    h->mb_y = 0;
    ff_h264_free_tables(h);
    h->context_initialized = 0;
}

static int get_last_needed_nal(H264Context *h)
{
    int nals_needed = 0;
    int first_slice = 0;
    int i, ret;
    for (i = 0; i < h->pkt.nb_nals; i++)
    {
        H2645NAL *nal = &h->pkt.nals[i];
        GetBitContext gb;
        switch (nal->type)
        {
        case H264_NAL_SPS:
        case H264_NAL_PPS:
            nals_needed = i;
            break;
        case H264_NAL_DPA:
        case H264_NAL_IDR_SLICE:
        case H264_NAL_SLICE:
            ret = init_get_bits8(&gb, nal->data + 1, nal->size - 1);
            if (ret < 0)
            {
                av_log(h->avctx, 16, "Invalid zero-sized VCL NAL unit\n");
                if (h->avctx->err_recognition & (1 << 3))
                    return ret;
                break;
            }
            if (!get_ue_golomb_long(&gb) ||
                !first_slice ||
                first_slice != nal->type)
                nals_needed = i;
            if (!first_slice)
                first_slice = nal->type;
        }
    }
    return nals_needed;
}

static void debug_green_metadata(const H264SEIGreenMetaData *gm, void *logctx)
{
    av_log(logctx, 48, "Green Metadata Info SEI message\n");
    av_log(logctx, 48, "  green_metadata_type: %d\n", gm->green_metadata_type);
    if (gm->green_metadata_type == 0)
    {
        av_log(logctx, 48, "  green_metadata_period_type: %d\n", gm->period_type);
        if (gm->period_type == 2)
            av_log(logctx, 48, "  green_metadata_num_seconds: %d\n", gm->num_seconds);
        else if (gm->period_type == 3)
            av_log(logctx, 48, "  green_metadata_num_pictures: %d\n", gm->num_pictures);
        av_log(logctx, 48, "  SEI GREEN Complexity Metrics: %f %f %f %f\n",
               (float)gm->percent_non_zero_macroblocks / 255,
               (float)gm->percent_intra_coded_macroblocks / 255,
               (float)gm->percent_six_tap_filtering / 255,
               (float)gm->percent_alpha_point_deblocking_instance / 255);
    }
    else if (gm->green_metadata_type == 1)
    {
        av_log(logctx, 48, "  xsd_metric_type: %d\n", gm->xsd_metric_type);
        if (gm->xsd_metric_type == 0)
            av_log(logctx, 48, "  xsd_metric_value: %f\n",
                   (float)gm->xsd_metric_value / 100);
    }
}

static int decode_nal_units(H264Context *h, const uint8_t *buf, int buf_size)
{
    AVCodecContext *const avctx = h->avctx;
    int nals_needed = 0;
    int idr_cleared = 0;
    int i, ret = 0;
    h->has_slice = 0;
    h->nal_unit_type = 0;
    if (!(avctx->flags2 & (1 << 15)))
    {
        h->current_slice = 0;
        if (!h->first_field)
        {
            h->cur_pic_ptr =
                ((void *)0);
            ff_h264_sei_uninit(&h->sei);
        }
    }
    if (h->nal_length_size == 4)
    {
        if (buf_size > 8 && av_bswap32((((const union unaligned_32 *)(buf))->l)) == 1 && av_bswap32((((const union unaligned_32 *)(buf + 5))->l)) > (unsigned)buf_size)
        {
            h->is_avc = 0;
        }
        else if (buf_size > 3 && av_bswap32((((const union unaligned_32 *)(buf))->l)) > 1 && av_bswap32((((const union unaligned_32 *)(buf))->l)) <= (unsigned)buf_size)
            h->is_avc = 1;
    }
    ret = ff_h2645_packet_split(&h->pkt, buf, buf_size, avctx, h->is_avc, h->nal_length_size,
                                avctx->codec_id, 0, 0);
    if (ret < 0)
    {
        av_log(avctx, 16,
               "Error splitting the input into NAL units.\n");
        return ret;
    }
    if (avctx->active_thread_type & 1)
        nals_needed = get_last_needed_nal(h);
    if (nals_needed < 0)
        return nals_needed;
    for (i = 0; i < h->pkt.nb_nals; i++)
    {
        H2645NAL *nal = &h->pkt.nals[i];
        int max_slice_ctx, err;
        if (avctx->skip_frame >= AVDISCARD_NONREF &&
            nal->ref_idc == 0 && nal->type != H264_NAL_SEI)
            continue;
        h->nal_ref_idc = nal->ref_idc;
        h->nal_unit_type = nal->type;
        err = 0;
        switch (nal->type)
        {
        case H264_NAL_IDR_SLICE:
            if ((nal->data[1] & 0xFC) == 0x98)
            {
                av_log(h->avctx, 16, "Invalid inter IDR frame\n");
                h->next_outputed_poc =
                    (-0x7fffffff - 1);
                ret = -1;
                goto end;
            }
            if (!idr_cleared)
            {
                idr(h);
            }
            idr_cleared = 1;
            h->has_recovery_point = 1;
        case H264_NAL_SLICE:
            h->has_slice = 1;
            if ((err = ff_h264_queue_decode_slice(h, nal)))
            {
                H264SliceContext *sl = h->slice_ctx + h->nb_slice_ctx_queued;
                sl->ref_count[0] = sl->ref_count[1] = 0;
                break;
            }
            if (h->current_slice == 1)
            {
                if (avctx->active_thread_type & 1 &&
                    i >= nals_needed && !h->setup_finished && h->cur_pic_ptr)
                {
                    ff_thread_finish_setup(avctx);
                    h->setup_finished = 1;
                }
                if (h->avctx->hwaccel &&
                    (ret = h->avctx->hwaccel->start_frame(h->avctx, buf, buf_size)) < 0)
                    goto end;
            }
            max_slice_ctx = avctx->hwaccel ? 1 : h->nb_slice_ctx;
            if (h->nb_slice_ctx_queued == max_slice_ctx)
            {
                if (h->avctx->hwaccel)
                {
                    ret = avctx->hwaccel->decode_slice(avctx, nal->raw_data, nal->raw_size);
                    h->nb_slice_ctx_queued = 0;
                }
                else
                    ret = ff_h264_execute_decode_slices(h);
                if (ret < 0 && (h->avctx->err_recognition & (1 << 3)))
                    goto end;
            }
            break;
        case H264_NAL_DPA:
        case H264_NAL_DPB:
        case H264_NAL_DPC:
            avpriv_request_sample(avctx, "data partitioning");
            break;
        case H264_NAL_SEI:
            ret = ff_h264_sei_decode(&h->sei, &nal->gb, &h->ps, avctx);
            h->has_recovery_point = h->has_recovery_point || h->sei.recovery_point.recovery_frame_cnt != -1;
            if (avctx->debug & 0x00800000)
                debug_green_metadata(&h->sei.green_metadata, h->avctx);
            if (ret < 0 && (h->avctx->err_recognition & (1 << 3)))
                goto end;
            break;
        case H264_NAL_SPS:
        {
            GetBitContext tmp_gb = nal->gb;
            if (avctx->hwaccel && avctx->hwaccel->decode_params)
            {
                ret = avctx->hwaccel->decode_params(avctx,
                                                    nal->type,
                                                    nal->raw_data,
                                                    nal->raw_size);
                if (ret < 0)
                    goto end;
            }
            if (ff_h264_decode_seq_parameter_set(&tmp_gb, avctx, &h->ps, 0) >= 0)
                break;
            av_log(h->avctx, 48,
                   "SPS decoding failure, trying again with the complete NAL\n");
            init_get_bits8(&tmp_gb, nal->raw_data + 1, nal->raw_size - 1);
            if (ff_h264_decode_seq_parameter_set(&tmp_gb, avctx, &h->ps, 0) >= 0)
                break;
            ff_h264_decode_seq_parameter_set(&nal->gb, avctx, &h->ps, 1);
            break;
        }
        case H264_NAL_PPS:
            if (avctx->hwaccel && avctx->hwaccel->decode_params)
            {
                ret = avctx->hwaccel->decode_params(avctx,
                                                    nal->type,
                                                    nal->raw_data,
                                                    nal->raw_size);
                if (ret < 0)
                    goto end;
            }
            ret = ff_h264_decode_picture_parameter_set(&nal->gb, avctx, &h->ps,
                                                       nal->size_bits);
            if (ret < 0 && (h->avctx->err_recognition & (1 << 3)))
                goto end;
            break;
        case H264_NAL_AUD:
        case H264_NAL_END_SEQUENCE:
        case H264_NAL_END_STREAM:
        case H264_NAL_FILLER_DATA:
        case H264_NAL_SPS_EXT:
        case H264_NAL_AUXILIARY_SLICE:
            break;
        default:
            av_log(avctx, 48, "Unknown NAL code: %d (%d bits)\n",
                   nal->type, nal->size_bits);
        }
        if (err < 0)
        {
            av_log(h->avctx, 16, "decode_slice_header error\n");
        }
    }
    ret = ff_h264_execute_decode_slices(h);
    if (ret < 0 && (h->avctx->err_recognition & (1 << 3)))
        goto end;
    if ((ret < 0 || h->slice_ctx->er.error_occurred) && h->cur_pic_ptr)
    {
        h->cur_pic_ptr->f->decode_error_flags |= 8;
    }
    ret = 0;
end:
    if (!((h)->picture_structure != 3) && h->current_slice && h->enable_er)
    {
        H264SliceContext *sl = h->slice_ctx;
        int use_last_pic = h->last_pic_for_ec.f->buf[0] && !sl->ref_count[0];
        ff_h264_set_erpic(&sl->er.cur_pic, h->cur_pic_ptr);
        if (use_last_pic)
        {
            ff_h264_set_erpic(&sl->er.last_pic, &h->last_pic_for_ec);
            sl->ref_list[0][0].parent = &h->last_pic_for_ec;
            memcpy(sl->ref_list[0][0].data, h->last_pic_for_ec.f->data, sizeof(sl->ref_list[0][0].data));
            memcpy(sl->ref_list[0][0].linesize, h->last_pic_for_ec.f->linesize, sizeof(sl->ref_list[0][0].linesize));
            sl->ref_list[0][0].reference = h->last_pic_for_ec.reference;
        }
        else if (sl->ref_count[0])
        {
            ff_h264_set_erpic(&sl->er.last_pic, sl->ref_list[0][0].parent);
        }
        else
            ff_h264_set_erpic(&sl->er.last_pic,
                              ((void *)0));
        if (sl->ref_count[1])
            ff_h264_set_erpic(&sl->er.next_pic, sl->ref_list[1][0].parent);
        sl->er.ref_count = sl->ref_count[0];
        ff_er_frame_end(&sl->er);
        if (use_last_pic)
            memset(&sl->ref_list[0][0], 0, sizeof(sl->ref_list[0][0]));
    }
    if (h->cur_pic_ptr && !h->droppable && h->has_slice)
    {
        ff_thread_report_progress(&h->cur_pic_ptr->tf, 0x7fffffff,
                                  h->picture_structure == 2);
    }
    return (ret < 0) ? ret : buf_size;
}

static int get_consumed_bytes(int pos, int buf_size)
{
    if (pos == 0)
        pos = 1;
    if (pos + 10 > buf_size)
        pos = buf_size;
    return pos;
}

static int h264_export_enc_params(AVFrame *f, H264Picture *p)
{
    AVVideoEncParams *par;
    unsigned int nb_mb = p->mb_height * p->mb_width;
    unsigned int x, y;
    par = av_video_enc_params_create_side_data(f, AV_VIDEO_ENC_PARAMS_H264, nb_mb);
    if (!par)
        return (-(
            12));
    par->qp = p->pps->init_qp;
    par->delta_qp[1][0] = p->pps->chroma_qp_index_offset[0];
    par->delta_qp[1][1] = p->pps->chroma_qp_index_offset[0];
    par->delta_qp[2][0] = p->pps->chroma_qp_index_offset[1];
    par->delta_qp[2][1] = p->pps->chroma_qp_index_offset[1];
    for (y = 0; y < p->mb_height; y++)
        for (x = 0; x < p->mb_width; x++)
        {
            const unsigned int block_idx = y * p->mb_width + x;
            const unsigned int mb_xy = y * p->mb_stride + x;
            AVVideoBlockParams *b = av_video_enc_params_block(par, block_idx);
            b->src_x = x * 16;
            b->src_y = y * 16;
            b->w = 16;
            b->h = 16;
            b->delta_qp = p->qscale_table[mb_xy] - par->qp;
        }
    return 0;
}

static int output_frame(H264Context *h, AVFrame *dst, H264Picture *srcp)
{
    AVFrame *src = srcp->f;
    int ret;
    ret = av_frame_ref(dst, src);
    if (ret < 0)
        return ret;
    av_dict_set(&dst->metadata, "stereo_mode", ff_h264_sei_stereo_mode(&h->sei.frame_packing), 0);
    if (srcp->sei_recovery_frame_cnt == 0)
        dst->key_frame = 1;
    if (h->avctx->export_side_data & (1 << 2))
    {
        ret = h264_export_enc_params(dst, srcp);
        if (ret < 0)
            goto fail;
    }
    return 0;
fail:
    av_frame_unref(dst);
    return ret;
}

static int is_avcc_extradata(const uint8_t *buf, int buf_size)
{
    int cnt = buf[5] & 0x1f;
    const uint8_t *p = buf + 6;
    if (!cnt)
        return 0;
    while (cnt--)
    {
        int nalsize = av_bswap16((((const union unaligned_16 *)(p))->l)) + 2;
        if (nalsize > buf_size - (p - buf) || (p[2] & 0x9F) != 7)
            return 0;
        p += nalsize;
    }
    cnt = *(p++);
    if (!cnt)
        return 0;
    while (cnt--)
    {
        int nalsize = av_bswap16((((const union unaligned_16 *)(p))->l)) + 2;
        if (nalsize > buf_size - (p - buf) || (p[2] & 0x9F) != 8)
            return 0;
        p += nalsize;
    }
    return 1;
}

static int finalize_frame(H264Context *h, AVFrame *dst, H264Picture *out, int *got_frame)
{
    int ret;
    if (((h->avctx->flags & (1 << 3)) ||
         (h->avctx->flags2 & (1 << 22)) ||
         out->recovered))
    {
        if (!h->avctx->hwaccel &&
            (out->field_poc[0] == 0x7fffffff ||
             out->field_poc[1] == 0x7fffffff))
        {
            int p;
            AVFrame *f = out->f;
            int field = out->field_poc[0] == 0x7fffffff;
            uint8_t *dst_data[4];
            int linesizes[4];
            const uint8_t *src_data[4];
            av_log(h->avctx, 48, "Duplicating field %d to fill missing\n", field);
            for (p = 0; p < 4; p++)
            {
                dst_data[p] = f->data[p] + (field ^ 1) * f->linesize[p];
                src_data[p] = f->data[p] + field * f->linesize[p];
                linesizes[p] = 2 * f->linesize[p];
            }
            av_image_copy(dst_data, linesizes, src_data, linesizes,
                          f->format, f->width, f->height >> 1);
        }
        ret = output_frame(h, dst, out);
        if (ret < 0)
            return ret;
        *got_frame = 1;
        if (1)
        {
            ff_print_debug_info2(h->avctx, dst,
                                 ((void *)0),
                                 out->mb_type,
                                 out->qscale_table,
                                 out->motion_val,

                                 ((void *)0),
                                 h->mb_width, h->mb_height, h->mb_stride, 1);
        }
    }
    return 0;
}

static int send_next_delayed_frame(H264Context *h, AVFrame *dst_frame,
                                   int *got_frame, int buf_index)
{
    int ret, i, out_idx;
    H264Picture *out = h->delayed_pic[0];
    h->cur_pic_ptr =
        ((void *)0);
    h->first_field = 0;
    out_idx = 0;
    for (i = 1;
         h->delayed_pic[i] &&
         !h->delayed_pic[i]->f->key_frame &&
         !h->delayed_pic[i]->mmco_reset;
         i++)
        if (h->delayed_pic[i]->poc < out->poc)
        {
            out = h->delayed_pic[i];
            out_idx = i;
        }
    for (i = out_idx; h->delayed_pic[i]; i++)
        h->delayed_pic[i] = h->delayed_pic[i + 1];
    if (out)
    {
        out->reference &= ~4;
        ret = finalize_frame(h, dst_frame, out, got_frame);
        if (ret < 0)
            return ret;
    }
    return buf_index;
}

static int h264_decode_frame(AVCodecContext *avctx, void *data,
                             int *got_frame, AVPacket *avpkt)
{
    const uint8_t *buf = avpkt->data;
    int buf_size = avpkt->size;
    H264Context *h = avctx->priv_data;
    AVFrame *pict = data;
    int buf_index;
    int ret;
    h->flags = avctx->flags;
    h->setup_finished = 0;
    h->nb_slice_ctx_queued = 0;
    ff_h264_unref_picture(h, &h->last_pic_for_ec);
    if (buf_size == 0)
        return send_next_delayed_frame(h, pict, got_frame, 0);
    if (av_packet_get_side_data(avpkt, AV_PKT_DATA_NEW_EXTRADATA,
                                ((void *)0)))
    {
        int side_size;
        uint8_t *side = av_packet_get_side_data(avpkt, AV_PKT_DATA_NEW_EXTRADATA, &side_size);
        ff_h264_decode_extradata(side, side_size,
                                 &h->ps, &h->is_avc, &h->nal_length_size,
                                 avctx->err_recognition, avctx);
    }
    if (h->is_avc && buf_size >= 9 && buf[0] == 1 && buf[2] == 0 && (buf[4] & 0xFC) == 0xFC)
    {
        if (is_avcc_extradata(buf, buf_size))
            return ff_h264_decode_extradata(buf, buf_size,
                                            &h->ps, &h->is_avc, &h->nal_length_size,
                                            avctx->err_recognition, avctx);
    }
    buf_index = decode_nal_units(h, buf, buf_size);
    if (buf_index < 0)
        return (-(int)(('I') | (('N') << 8) | (('D') << 16) | ((unsigned)('A') << 24)));
    if (!h->cur_pic_ptr && h->nal_unit_type == H264_NAL_END_SEQUENCE)
    {
        do
        {
            if (!(buf_index <= buf_size))
            {
                av_log(
                    ((void *)0), 0, "Assertion %s failed at %s:%d\n", "buf_index <= buf_size", "C:\\FFmpeg\\libavcodec\\h264dec.c", 996);
                abort();
            }
        } while (0);
        return send_next_delayed_frame(h, pict, got_frame, buf_index);
    }
    if (!(avctx->flags2 & (1 << 15)) && (!h->cur_pic_ptr || !h->has_slice))
    {
        if (avctx->skip_frame >= AVDISCARD_NONREF ||
            buf_size >= 4 && !memcmp("Q264", buf, 4))
            return buf_size;
        av_log(avctx, 16, "no frame!\n");
        return (-(int)(('I') | (('N') << 8) | (('D') << 16) | ((unsigned)('A') << 24)));
    }
    if (!(avctx->flags2 & (1 << 15)) ||
        (h->mb_y >= h->mb_height && h->mb_height))
    {
        if ((ret = ff_h264_field_end(h, &h->slice_ctx[0], 0)) < 0)
            return ret;
        if (h->next_output_pic)
        {
            ret = finalize_frame(h, pict, h->next_output_pic, got_frame);
            if (ret < 0)
                return ret;
        }
    }
    do
    {
        if (!(pict->buf[0] || !*got_frame))
        {
            av_log(
                ((void *)0), 0, "Assertion %s failed at %s:%d\n", "pict->buf[0] || !*got_frame", "C:\\FFmpeg\\libavcodec\\h264dec.c", 1021);
            abort();
        }
    } while (0);
    ff_h264_unref_picture(h, &h->last_pic_for_ec);
    return get_consumed_bytes(buf_index, buf_size);
}
static const AVOption h264_options[] = {
    {"is_avc", "is avc", __builtin_offsetof(H264Context, is_avc), AV_OPT_TYPE_BOOL, {.i64 = 0}, 0, 1, 0},
    {"nal_length_size", "nal_length_size", __builtin_offsetof(H264Context, nal_length_size), AV_OPT_TYPE_INT, {.i64 = 0}, 0, 4, 0},
    {"enable_er", "Enable error resilience on damaged frames (unsafe)", __builtin_offsetof(H264Context, enable_er), AV_OPT_TYPE_BOOL, {.i64 = -1}, -1, 1, 16 | 2},
    {"x264_build", "Assume this x264 version if no x264 version found in any SEI", __builtin_offsetof(H264Context, x264_build), AV_OPT_TYPE_INT, {.i64 = -1}, -1, 0x7fffffff, 16 | 2},
    {((void *)0)},
};

static const AVClass h264_class = {
    .class_name = "H264 Decoder",
    .item_name = av_default_item_name,
    .option = h264_options,
    .version = ((56) << 16 | (60) << 8 | (100)),
};

AVCodec ff_h264_decoder = {
    .name = "h264",
    .long_name = NULL_IF_CONFIG_SMALL("H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10"),
    .type = AVMEDIA_TYPE_VIDEO,
    .id = AV_CODEC_ID_H264,
    .priv_data_size = sizeof(H264Context),
    .init = h264_decode_init,
    .close = h264_decode_end,
    .decode = h264_decode_frame,
    .capabilities = (1 << 1) |
                    (1 << 5) | (1 << 13) |
                    (1 << 12),
    .hw_configs = (const AVCodecHWConfigInternal *[]){
        &(const AVCodecHWConfigInternal){
            .public = {
                .pix_fmt = AV_PIX_FMT_DXVA2_VLD,
                .methods = (1 ? AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX : 0) | (1 ? AV_CODEC_HW_CONFIG_METHOD_HW_FRAMES_CTX : 0) | (1 ? AV_CODEC_HW_CONFIG_METHOD_AD_HOC : 0),
                .device_type = AV_HWDEVICE_TYPE_DXVA2,
            },
            .hwaccel = &ff_h264_dxva2_hwaccel,
        },
        &(const AVCodecHWConfigInternal){
            .public = {
                .pix_fmt = AV_PIX_FMT_D3D11VA_VLD,
                .methods = (0 ? AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX : 0) | (0 ? AV_CODEC_HW_CONFIG_METHOD_HW_FRAMES_CTX : 0) | (1 ? AV_CODEC_HW_CONFIG_METHOD_AD_HOC : 0),
                .device_type = AV_HWDEVICE_TYPE_NONE,
            },
            .hwaccel = &ff_h264_d3d11va_hwaccel,
        },
        &(const AVCodecHWConfigInternal){
            .public = {
                .pix_fmt = AV_PIX_FMT_D3D11,
                .methods = (1 ? AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX : 0) | (1 ? AV_CODEC_HW_CONFIG_METHOD_HW_FRAMES_CTX : 0) | (0 ? AV_CODEC_HW_CONFIG_METHOD_AD_HOC : 0),
                .device_type = AV_HWDEVICE_TYPE_D3D11VA,
            },
            .hwaccel = &ff_h264_d3d11va2_hwaccel,
        },

        ((void *)0)

    },
    .caps_internal = (1 << 0) | (1 << 4) | (1 << 6) | (1 << 1),
    .flush = h264_decode_flush,
    .update_thread_context = ONLY_IF_THREADS_ENABLED(ff_h264_update_thread_context),
    .profiles = NULL_IF_CONFIG_SMALL(ff_h264_profiles),
    .priv_class = &h264_class,
};
