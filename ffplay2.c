#include <inttypes.h>
#include <math.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <gcrypt.h>
#include <inttypes.h>
#include <math.h>
#include <limits.h>
#include <signal.h>
#include <stdint.h>
#include <stdarg.h>
#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <process.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <iconv.h>
#include <fcntl.h>
#include <soxr.h>
#include <fcntl.h>
#if CONFIG_ZLIB
#include <zlib.h>
#endif
#include <bcrypt.h>
#ifdef _WIN32
#undef open
#undef lseek
#undef stat
#undef fstat
#endif
#include <share.h>
#include <errno.h>
#include "config.h"
#ifdef _WIN32
#include <windows.h>
#endif

#define AV_GCC_VERSION_AT_LEAST(x,y) (__GNUC__ > (x) || __GNUC__ == (x) && __GNUC_MINOR__ >= (y))
#define AV_GCC_VERSION_AT_MOST(x,y) (__GNUC__ < (x) || __GNUC__ == (x) && __GNUC_MINOR__ <= (y))
#define AV_HAS_BUILTIN(x) __has_builtin(x)
#define av_always_inline __attribute__((always_inline)) inline
#define av_extern_inline extern inline
#define av_warn_unused_result __attribute__((warn_unused_result))
#define av_noinline __attribute__((noinline))
#define av_pure __attribute__((pure))
#define av_const __attribute__((const))
#define av_cold __attribute__((cold))
#define av_flatten __attribute__((flatten))
#define attribute_deprecated __attribute__((deprecated))
#define AV_NOWARN_DEPRECATED(code) _Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"") code _Pragma("GCC diagnostic pop")
#define av_unused __attribute__((unused))
#define av_used __attribute__((used))
#define av_alias __attribute__((may_alias))
#define av_uninit(x) x=x
#define av_builtin_constant_p __builtin_constant_p
#define av_printf_format(fmtpos,attrpos) __attribute__((__format__(__printf__, fmtpos, attrpos)))
#define av_noreturn __attribute__((noreturn))
#define AVUTIL_VERSION_H 
#define AVUTIL_MACROS_H 
#define AV_STRINGIFY(s) AV_TOSTRING(s)
#define AV_TOSTRING(s) #s
#define AV_GLUE(a,b) a ## b
#define AV_JOIN(a,b) AV_GLUE(a, b)
#define AV_PRAGMA(s) _Pragma(#s)
#define FFALIGN(x,a) (((x)+(a)-1)&~((a)-1))
#define AV_VERSION_INT(a,b,c) ((a)<<16 | (b)<<8 | (c))
#define AV_VERSION_DOT(a,b,c) a ##. ## b ##. ## c
#define AV_VERSION(a,b,c) AV_VERSION_DOT(a, b, c)
#define AV_VERSION_MAJOR(a) ((a) >> 16)
#define AV_VERSION_MINOR(a) (((a) & 0x00FF00) >> 8)
#define AV_VERSION_MICRO(a) ((a) & 0xFF)
#define LIBAVUTIL_VERSION_MAJOR 56
#define LIBAVUTIL_VERSION_MINOR 60
#define LIBAVUTIL_VERSION_MICRO 100
#define LIBAVUTIL_VERSION_INT AV_VERSION_INT(LIBAVUTIL_VERSION_MAJOR, LIBAVUTIL_VERSION_MINOR, LIBAVUTIL_VERSION_MICRO)
#define LIBAVUTIL_VERSION AV_VERSION(LIBAVUTIL_VERSION_MAJOR, LIBAVUTIL_VERSION_MINOR, LIBAVUTIL_VERSION_MICRO)
#define LIBAVUTIL_BUILD LIBAVUTIL_VERSION_INT
#define LIBAVUTIL_IDENT "Lavu" AV_STRINGIFY(LIBAVUTIL_VERSION)
#define FF_API_VAAPI (LIBAVUTIL_VERSION_MAJOR < 57)
#define FF_API_FRAME_QP (LIBAVUTIL_VERSION_MAJOR < 57)
#define FF_API_PLUS1_MINUS1 (LIBAVUTIL_VERSION_MAJOR < 57)
#define FF_API_ERROR_FRAME (LIBAVUTIL_VERSION_MAJOR < 57)
#define FF_API_PKT_PTS (LIBAVUTIL_VERSION_MAJOR < 57)
#define FF_API_CRYPTO_SIZE_T (LIBAVUTIL_VERSION_MAJOR < 57)
#define FF_API_FRAME_GET_SET (LIBAVUTIL_VERSION_MAJOR < 57)
#define FF_API_PSEUDOPAL (LIBAVUTIL_VERSION_MAJOR < 57)
#define FF_API_CHILD_CLASS_NEXT (LIBAVUTIL_VERSION_MAJOR < 57)
#define FF_API_D2STR (LIBAVUTIL_VERSION_MAJOR < 58)

int av_strstart(const char *str, const char *pfx, const char **ptr);
int av_stristart(const char *str, const char *pfx, const char **ptr);
char *av_stristr(const char *haystack, const char *needle);
char *av_strnstr(const char *haystack, const char *needle, size_t hay_length);
size_t av_strlcpy(char *dst, const char *src, size_t size);
size_t av_strlcat(char *dst, const char *src, size_t size);
size_t av_strlcatf(char *dst, size_t size, const char *fmt, ...) ;

static inline size_t av_strnlen(const char *s, size_t len)
{
    size_t i;
    for (i = 0; i < len && s[i]; i++)
        ;
    return i;
}
char *av_asprintf(const char *fmt, ...) ;

char *av_d2str(double d);
char *av_get_token(const char **buf, const char *term);
char *av_strtok(char *s, const char *delim, char **saveptr);

static inline int av_isdigit(int c)
{
    return c >= '0' && c <= '9';
}

static inline int av_isgraph(int c)
{
    return c > 32 && c < 127;
}

static inline int av_isspace(int c)
{
    return c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' ||
           c == '\v';
}

static inline int av_toupper(int c)
{
    if (c >= 'a' && c <= 'z')
        c ^= 0x20;
    return c;
}

static inline int av_tolower(int c)
{
    if (c >= 'A' && c <= 'Z')
        c ^= 0x20;
    return c;
}

static inline int av_isxdigit(int c)
{
    c = av_tolower(c);
    return av_isdigit(c) || (c >= 'a' && c <= 'f');
}

int av_strcasecmp(const char *a, const char *b);

int av_strncasecmp(const char *a, const char *b, size_t n);

char *av_strireplace(const char *str, const char *from, const char *to);
const char *av_basename(const char *path);
const char *av_dirname(char *path);
int av_match_name(const char *name, const char *names);
char *av_append_path_component(const char *path, const char *component);

enum AVEscapeMode {
    AV_ESCAPE_MODE_AUTO,
    AV_ESCAPE_MODE_BACKSLASH,
    AV_ESCAPE_MODE_QUOTE,
};
#define AV_ESCAPE_FLAG_WHITESPACE (1 << 0)
#define AV_ESCAPE_FLAG_STRICT (1 << 1)

int av_escape(char **dst, const char *src, const char *special_chars,
              enum AVEscapeMode mode, int flags);
#define AV_UTF8_FLAG_ACCEPT_INVALID_BIG_CODES 1
#define AV_UTF8_FLAG_ACCEPT_NON_CHARACTERS 2
#define AV_UTF8_FLAG_ACCEPT_SURROGATES 4
#define AV_UTF8_FLAG_EXCLUDE_XML_INVALID_CONTROL_CODES 8
#define AV_UTF8_FLAG_ACCEPT_ALL AV_UTF8_FLAG_ACCEPT_INVALID_BIG_CODES|AV_UTF8_FLAG_ACCEPT_NON_CHARACTERS|AV_UTF8_FLAG_ACCEPT_SURROGATES

int av_utf8_decode(int32_t *codep, const uint8_t **bufp, const uint8_t *buf_end,
                   unsigned int flags);

int av_match_list(const char *name, const char *list, char separator);

int av_sscanf(const char *string, const char *format, ...);
#define AVUTIL_EVAL_H 
#define AVUTIL_AVUTIL_H 
unsigned avutil_version(void);

const char *av_version_info(void);

const char *avutil_configuration(void);

const char *avutil_license(void);
enum AVMediaType {
    AVMEDIA_TYPE_UNKNOWN = -1,
    AVMEDIA_TYPE_VIDEO,
    AVMEDIA_TYPE_AUDIO,
    AVMEDIA_TYPE_DATA,
    AVMEDIA_TYPE_SUBTITLE,
    AVMEDIA_TYPE_ATTACHMENT,
    AVMEDIA_TYPE_NB
};

const char *av_get_media_type_string(enum AVMediaType media_type);
#define FF_LAMBDA_SHIFT 7
#define FF_LAMBDA_SCALE (1<<FF_LAMBDA_SHIFT)
#define FF_QP2LAMBDA 118
#define FF_LAMBDA_MAX (256*128-1)
#define FF_QUALITY_SCALE FF_LAMBDA_SCALE
#define AV_NOPTS_VALUE ((int64_t)UINT64_C(0x8000000000000000))
#define AV_TIME_BASE 1000000
#define AV_TIME_BASE_Q (AVRational){1, AV_TIME_BASE}
enum AVPictureType {
    AV_PICTURE_TYPE_NONE = 0,
    AV_PICTURE_TYPE_I,
    AV_PICTURE_TYPE_P,
    AV_PICTURE_TYPE_B,
    AV_PICTURE_TYPE_S,
    AV_PICTURE_TYPE_SI,
    AV_PICTURE_TYPE_SP,
    AV_PICTURE_TYPE_BI,
};
char av_get_picture_type_char(enum AVPictureType pict_type);
#define AVUTIL_COMMON_H 
#define AVUTIL_AVCONFIG_H 
#define AV_HAVE_BIGENDIAN 0
#define AV_HAVE_FAST_UNALIGNED 1
#define AV_NE(be,le) (le)
#define RSHIFT(a,b) ((a) > 0 ? ((a) + ((1<<(b))>>1))>>(b) : ((a) + ((1<<(b))>>1)-1)>>(b))
#define ROUNDED_DIV(a,b) (((a)>=0 ? (a) + ((b)>>1) : (a) - ((b)>>1))/(b))
#define AV_CEIL_RSHIFT(a,b) (!av_builtin_constant_p(b) ? -((-(a)) >> (b)) : ((a) + (1<<(b)) - 1) >> (b))
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
#define av_ceil_log2 av_ceil_log2_c
#define av_clip av_clip_c
#define av_clip64 av_clip64_c
#define av_clip_uint8 av_clip_uint8_c
#define av_clip_int8 av_clip_int8_c
#define av_clip_uint16 av_clip_uint16_c
#define av_clip_int16 av_clip_int16_c
#define av_clipl_int32 av_clipl_int32_c
#define av_clip_intp2 av_clip_intp2_c
#define av_clip_uintp2 av_clip_uintp2_c
#define av_mod_uintp2 av_mod_uintp2_c
#define av_sat_add32 av_sat_add32_c
#define av_sat_dadd32 av_sat_dadd32_c
#define av_sat_sub32 av_sat_sub32_c
#define av_sat_dsub32 av_sat_dsub32_c
#define av_sat_add64 av_sat_add64_c
#define av_sat_sub64 av_sat_sub64_c
#define av_clipf av_clipf_c
#define av_clipd av_clipd_c
#define av_popcount av_popcount_c
#define av_popcount64 av_popcount64_c
#define av_parity av_parity_c
 
        int av_log2(unsigned v);
 int av_log2_16bit(unsigned v);
static inline int av_clip_c(int a, int amin, int amax)
{
    if (a < amin) return amin;
    else if (a > amax) return amax;
    else return a;
}
static inline int64_t av_clip64_c(int64_t a, int64_t amin, int64_t amax)
{
    if (a < amin) return amin;
    else if (a > amax) return amax;
    else return a;
}

static inline uint8_t av_clip_uint8_c(int a)
{
    if (a&(~0xFF)) return (~a)>>31;
    else return a;
}

static inline int8_t av_clip_int8_c(int a)
{
    if ((a+0x80U) & ~0xFF) return (a>>31) ^ 0x7F;
    else return a;
}

static inline uint16_t av_clip_uint16_c(int a)
{
    if (a&(~0xFFFF)) return (~a)>>31;
    else return a;
}

static inline int16_t av_clip_int16_c(int a)
{
    if ((a+0x8000U) & ~0xFFFF) return (a>>31) ^ 0x7FFF;
    else return a;
}

static inline int32_t av_clipl_int32_c(int64_t a)
{
    if ((a+0x80000000u) & ~0xFFFFFFFFULL) return (int32_t)((a>>63) ^ 0x7FFFFFFF);
    else return (int32_t)a;
}

static inline int av_clip_intp2_c(int a, int p)
{
    if (((unsigned)a + (1 << p)) & ~((2 << p) - 1))
        return (a >> 31) ^ ((1 << p) - 1);
    else
        return a;
}

static inline unsigned av_clip_uintp2_c(int a, int p)
{
    if (a & ~((1<<p) - 1)) return (~a) >> 31 & ((1<<p) - 1);
    else return a;
}

static inline unsigned av_mod_uintp2_c(unsigned a, unsigned p)
{
    return a & ((1U << p) - 1);
}
static inline int av_sat_add32_c(int a, int b)
{
    return av_clipl_int32_c((int64_t)a + b);
}
static inline int av_sat_dadd32_c(int a, int b)
{
    return av_sat_add32_c(a, av_sat_add32_c(b, b));
}
static inline int av_sat_sub32_c(int a, int b)
{
    return av_clipl_int32_c((int64_t)a - b);
}
static inline int av_sat_dsub32_c(int a, int b)
{
    return av_sat_sub32_c(a, av_sat_add32_c(b, b));
}
static inline int64_t av_sat_add64_c(int64_t a, int64_t b) {
    int64_t tmp;
    return !__builtin_add_overflow(a, b, &tmp) ? tmp : (tmp < 0 ? 
                                                                 9223372036854775807LL 
                                                                           : 
                                                                             (-9223372036854775807LL - 1)
                                                                                      );
}
static inline int64_t av_sat_sub64_c(int64_t a, int64_t b) {
    int64_t tmp;
    return !__builtin_sub_overflow(a, b, &tmp) ? tmp : (tmp < 0 ? 
                                                                 9223372036854775807LL 
                                                                           : 
                                                                             (-9223372036854775807LL - 1)
                                                                                      );
}
static inline float av_clipf_c(float a, float amin, float amax)
{
    if (a < amin) return amin;
    else if (a > amax) return amax;
    else return a;
}
static inline double av_clipd_c(double a, double amin, double amax)
{
    if (a < amin) return amin;
    else if (a > amax) return amax;
    else return a;
}

static inline int av_ceil_log2_c(int x)
{
    return av_log2((x - 1U) << 1);
}

static inline int av_popcount_c(uint32_t x)
{
    x -= (x >> 1) & 0x55555555;
    x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
    x = (x + (x >> 4)) & 0x0F0F0F0F;
    x += x >> 8;
    return (x + (x >> 16)) & 0x3F;
}

static inline int av_popcount64_c(uint64_t x)
{
    return av_popcount_c((uint32_t)x) + av_popcount_c((uint32_t)(x >> 32));
}

static inline int av_parity_c(uint32_t v)
{
    return av_popcount_c(v) & 1;
}
#define MKTAG(a,b,c,d) ((a) | ((b) << 8) | ((c) << 16) | ((unsigned)(d) << 24))
#define MKBETAG(a,b,c,d) ((d) | ((c) << 8) | ((b) << 16) | ((unsigned)(a) << 24))
#define GET_UTF8(val,GET_BYTE,ERROR) val= (GET_BYTE); { uint32_t top = (val & 128) >> 1; if ((val & 0xc0) == 0x80 || val >= 0xFE) {ERROR} while (val & top) { unsigned int tmp = (GET_BYTE) - 128; if(tmp>>6) {ERROR} val= (val<<6) + tmp; top <<= 5; } val &= (top << 1) - 1; }
#define GET_UTF16(val,GET_16BIT,ERROR) val = (GET_16BIT); { unsigned int hi = val - 0xD800; if (hi < 0x800) { val = (GET_16BIT) - 0xDC00; if (val > 0x3FFU || hi > 0x3FFU) {ERROR} val += (hi<<10) + 0x10000; } }
#define PUT_UTF8(val,tmp,PUT_BYTE) { int bytes, shift; uint32_t in = val; if (in < 0x80) { tmp = in; PUT_BYTE } else { bytes = (av_log2(in) + 4) / 5; shift = (bytes - 1) * 6; tmp = (256 - (256 >> bytes)) | (in >> shift); PUT_BYTE while (shift >= 6) { shift -= 6; tmp = 0x80 | ((in >> shift) & 0x3f); PUT_BYTE } } }
#define PUT_UTF16(val,tmp,PUT_16BIT) { uint32_t in = val; if (in < 0x10000) { tmp = in; PUT_16BIT } else { tmp = 0xD800 | ((in - 0x10000) >> 10); PUT_16BIT tmp = 0xDC00 | ((in - 0x10000) & 0x3FF); PUT_16BIT } }
#define AVUTIL_MEM_H 
#define AVUTIL_ERROR_H 
#define AVERROR(e) (-(e))
#define AVUNERROR(e) (-(e))
#define FFERRTAG(a,b,c,d) (-(int)MKTAG(a, b, c, d))
#define AVERROR_BSF_NOT_FOUND FFERRTAG(0xF8,'B','S','F')
#define AVERROR_BUG FFERRTAG( 'B','U','G','!')
#define AVERROR_BUFFER_TOO_SMALL FFERRTAG( 'B','U','F','S')
#define AVERROR_DECODER_NOT_FOUND FFERRTAG(0xF8,'D','E','C')
#define AVERROR_DEMUXER_NOT_FOUND FFERRTAG(0xF8,'D','E','M')
#define AVERROR_ENCODER_NOT_FOUND FFERRTAG(0xF8,'E','N','C')
#define AVERROR_EOF FFERRTAG( 'E','O','F',' ')
#define AVERROR_EXIT FFERRTAG( 'E','X','I','T')
#define AVERROR_EXTERNAL FFERRTAG( 'E','X','T',' ')
#define AVERROR_FILTER_NOT_FOUND FFERRTAG(0xF8,'F','I','L')
#define AVERROR_INVALIDDATA FFERRTAG( 'I','N','D','A')
#define AVERROR_MUXER_NOT_FOUND FFERRTAG(0xF8,'M','U','X')
#define AVERROR_OPTION_NOT_FOUND FFERRTAG(0xF8,'O','P','T')
#define AVERROR_PATCHWELCOME FFERRTAG( 'P','A','W','E')
#define AVERROR_PROTOCOL_NOT_FOUND FFERRTAG(0xF8,'P','R','O')
#define AVERROR_STREAM_NOT_FOUND FFERRTAG(0xF8,'S','T','R')
#define AVERROR_BUG2 FFERRTAG( 'B','U','G',' ')
#define AVERROR_UNKNOWN FFERRTAG( 'U','N','K','N')
#define AVERROR_EXPERIMENTAL (-0x2bb2afa8)
#define AVERROR_INPUT_CHANGED (-0x636e6701)
#define AVERROR_OUTPUT_CHANGED (-0x636e6702)
#define AVERROR_HTTP_BAD_REQUEST FFERRTAG(0xF8,'4','0','0')
#define AVERROR_HTTP_UNAUTHORIZED FFERRTAG(0xF8,'4','0','1')
#define AVERROR_HTTP_FORBIDDEN FFERRTAG(0xF8,'4','0','3')
#define AVERROR_HTTP_NOT_FOUND FFERRTAG(0xF8,'4','0','4')
#define AVERROR_HTTP_OTHER_4XX FFERRTAG(0xF8,'4','X','X')
#define AVERROR_HTTP_SERVER_ERROR FFERRTAG(0xF8,'5','X','X')
#define AV_ERROR_MAX_STRING_SIZE 64
int av_strerror(int errnum, char *errbuf, size_t errbuf_size);
static inline char *av_make_error_string(char *errbuf, size_t errbuf_size, int errnum)
{
    av_strerror(errnum, errbuf, errbuf_size);
    return errbuf;
}
#define av_err2str(errnum) av_make_error_string((char[AV_ERROR_MAX_STRING_SIZE]){0}, AV_ERROR_MAX_STRING_SIZE, errnum)
#define DECLARE_ALIGNED(n,t,v) t __attribute__ ((aligned (n))) v
#define DECLARE_ASM_ALIGNED(n,t,v) t av_used __attribute__ ((aligned (n))) v
#define DECLARE_ASM_CONST(n,t,v) static const t av_used __attribute__ ((aligned (n))) v
#define av_malloc_attrib __attribute__((__malloc__))
#define av_alloc_size(...) __attribute__((alloc_size(__VA_ARGS__)))
void *av_malloc(size_t size) ;
void *av_mallocz(size_t size) ;
 void *av_malloc_array(size_t nmemb, size_t size);
 void *av_mallocz_array(size_t nmemb, size_t size);

void *av_calloc(size_t nmemb, size_t size) ;
void *av_realloc(void *ptr, size_t size) ;

int av_reallocp(void *ptr, size_t size);
void *av_realloc_f(void *ptr, size_t nelem, size_t elsize);
 void *av_realloc_array(void *ptr, size_t nmemb, size_t size);
int av_reallocp_array(void *ptr, size_t nmemb, size_t size);
void *av_fast_realloc(void *ptr, unsigned int *size, size_t min_size);
void av_fast_malloc(void *ptr, unsigned int *size, size_t min_size);
void av_fast_mallocz(void *ptr, unsigned int *size, size_t min_size);
void av_free(void *ptr);
void av_freep(void *ptr);
char *av_strdup(const char *s) ;
char *av_strndup(const char *s, size_t len) ;
void *av_memdup(const void *p, size_t size);
void av_memcpy_backptr(uint8_t *dst, int back, int cnt);
void av_dynarray_add(void *tab_ptr, int *nb_ptr, void *elem);

int av_dynarray_add_nofree(void *tab_ptr, int *nb_ptr, void *elem);
void *av_dynarray2_add(void **tab_ptr, int *nb_ptr, size_t elem_size,
                       const uint8_t *elem_data);
static inline int av_size_mult(size_t a, size_t b, size_t *r)
{
    size_t t = a * b;
    if ((a | b) >= ((size_t)1 << (sizeof(size_t) * 4)) && a && t / a != b)
        return (-(
              22
              ));
    *r = t;
    return 0;
}
void av_max_alloc(size_t max);
#define AVUTIL_RATIONAL_H 

typedef struct AVRational{
    int num;
    int den;
} AVRational;
static inline AVRational av_make_q(int num, int den)
{
    AVRational r = { num, den };
    return r;
}
static inline int av_cmp_q(AVRational a, AVRational b){
    const int64_t tmp= a.num * (int64_t)b.den - b.num * (int64_t)a.den;
    if(tmp) return (int)((tmp ^ a.den ^ b.den)>>63)|1;
    else if(b.den && a.den) return 0;
    else if(a.num && b.num) return (a.num>>31) - (b.num>>31);
    else return 
                                  (-0x7fffffff - 1)
                                         ;
}

static inline double av_q2d(AVRational a){
    return a.num / (double) a.den;
}
int av_reduce(int *dst_num, int *dst_den, int64_t num, int64_t den, int64_t max);

AVRational av_mul_q(AVRational b, AVRational c) ;

AVRational av_div_q(AVRational b, AVRational c) ;

AVRational av_add_q(AVRational b, AVRational c) ;

AVRational av_sub_q(AVRational b, AVRational c) ;

static inline AVRational av_inv_q(AVRational q)
{
    AVRational r = { q.den, q.num };
    return r;
}
AVRational av_d2q(double d, int max) ;
int av_nearer_q(AVRational q, AVRational q1, AVRational q2);
int av_find_nearest_q_idx(AVRational q, const AVRational* q_list);
uint32_t av_q2intfloat(AVRational q);

AVRational av_gcd_q(AVRational a, AVRational b, int max_den, AVRational def);
#define AVUTIL_MATHEMATICS_H 
#define AVUTIL_INTFLOAT_H 

union av_intfloat32 {
    uint32_t i;
    float f;
};

union av_intfloat64 {
    uint64_t i;
    double f;
};

static inline float av_int2float(uint32_t i)
{
    union av_intfloat32 v;
    v.i = i;
    return v.f;
}

static inline uint32_t av_float2int(float f)
{
    union av_intfloat32 v;
    v.f = f;
    return v.i;
}

static inline double av_int2double(uint64_t i)
{
    union av_intfloat64 v;
    v.i = i;
    return v.f;
}

static inline uint64_t av_double2int(double f)
{
    union av_intfloat64 v;
    v.f = f;
    return v.i;
}
#define M_LOG2_10 3.32192809488736234787
#define M_PHI 1.61803398874989484820
enum AVRounding {
    AV_ROUND_ZERO = 0,
    AV_ROUND_INF = 1,
    AV_ROUND_DOWN = 2,
    AV_ROUND_UP = 3,
    AV_ROUND_NEAR_INF = 5,
    AV_ROUND_PASS_MINMAX = 8192,
};
int64_t av_gcd(int64_t a, int64_t b);
int64_t av_rescale(int64_t a, int64_t b, int64_t c) ;
int64_t av_rescale_rnd(int64_t a, int64_t b, int64_t c, enum AVRounding rnd) ;
int64_t av_rescale_q(int64_t a, AVRational bq, AVRational cq) ;
int64_t av_rescale_q_rnd(int64_t a, AVRational bq, AVRational cq,
                         enum AVRounding rnd) ;
int av_compare_ts(int64_t ts_a, AVRational tb_a, int64_t ts_b, AVRational tb_b);
int64_t av_compare_mod(uint64_t a, uint64_t b, uint64_t mod);
int64_t av_rescale_delta(AVRational in_tb, int64_t in_ts, AVRational fs_tb, int duration, int64_t *last, AVRational out_tb);
int64_t av_add_stable(AVRational ts_tb, int64_t ts, AVRational inc_tb, int64_t inc);
#define AVUTIL_LOG_H 

typedef enum {
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
    AV_CLASS_CATEGORY_NB
}AVClassCategory;
#define AV_IS_INPUT_DEVICE(category) (((category) == AV_CLASS_CATEGORY_DEVICE_VIDEO_INPUT) || ((category) == AV_CLASS_CATEGORY_DEVICE_AUDIO_INPUT) || ((category) == AV_CLASS_CATEGORY_DEVICE_INPUT))
#define AV_IS_OUTPUT_DEVICE(category) (((category) == AV_CLASS_CATEGORY_DEVICE_VIDEO_OUTPUT) || ((category) == AV_CLASS_CATEGORY_DEVICE_AUDIO_OUTPUT) || ((category) == AV_CLASS_CATEGORY_DEVICE_OUTPUT))

struct AVOptionRanges;

typedef struct AVClass {
    const char* class_name;
    const char* (*item_name)(void* ctx);
    const struct AVOption *option;
    int version;
    int log_level_offset_offset;
    int parent_log_context_offset;
    void* (*child_next)(void *obj, void *prev);
   
    const struct AVClass* (*child_class_next)(const struct AVClass *prev);
    AVClassCategory category;
    AVClassCategory (*get_category)(void* ctx);
    int (*query_ranges)(struct AVOptionRanges **, void *obj, const char *key, int flags);
    const struct AVClass* (*child_class_iterate)(void **iter);
} AVClass;
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
void av_log(void *avcl, int level, const char *fmt, ...) ;
void av_log_once(void* avcl, int initial_level, int subsequent_level, int *state, const char *fmt, ...) ;
void av_vlog(void *avcl, int level, const char *fmt, va_list vl);
int av_log_get_level(void);
void av_log_set_level(int level);
void av_log_set_callback(void (*callback)(void*, int, const char*, va_list));
void av_log_default_callback(void *avcl, int level, const char *fmt,
                             va_list vl);
const char* av_default_item_name(void* ctx);
AVClassCategory av_default_get_category(void *ptr);
void av_log_format_line(void *ptr, int level, const char *fmt, va_list vl,
                        char *line, int line_size, int *print_prefix);
int av_log_format_line2(void *ptr, int level, const char *fmt, va_list vl,
                        char *line, int line_size, int *print_prefix);
#define AV_LOG_SKIP_REPEATED 1
#define AV_LOG_PRINT_LEVEL 2

void av_log_set_flags(int arg);
int av_log_get_flags(void);
#define AVUTIL_PIXFMT_H 
#define AVPALETTE_SIZE 1024
#define AVPALETTE_COUNT 256
enum AVPixelFormat {
    AV_PIX_FMT_NONE = -1,
    AV_PIX_FMT_YUV420P,
    AV_PIX_FMT_YUYV422,
    AV_PIX_FMT_RGB24,
    AV_PIX_FMT_BGR24,
    AV_PIX_FMT_YUV422P,
    AV_PIX_FMT_YUV444P,
    AV_PIX_FMT_YUV410P,
    AV_PIX_FMT_YUV411P,
    AV_PIX_FMT_GRAY8,
    AV_PIX_FMT_MONOWHITE,
    AV_PIX_FMT_MONOBLACK,
    AV_PIX_FMT_PAL8,
    AV_PIX_FMT_YUVJ420P,
    AV_PIX_FMT_YUVJ422P,
    AV_PIX_FMT_YUVJ444P,
    AV_PIX_FMT_UYVY422,
    AV_PIX_FMT_UYYVYY411,
    AV_PIX_FMT_BGR8,
    AV_PIX_FMT_BGR4,
    AV_PIX_FMT_BGR4_BYTE,
    AV_PIX_FMT_RGB8,
    AV_PIX_FMT_RGB4,
    AV_PIX_FMT_RGB4_BYTE,
    AV_PIX_FMT_NV12,
    AV_PIX_FMT_NV21,
    AV_PIX_FMT_ARGB,
    AV_PIX_FMT_RGBA,
    AV_PIX_FMT_ABGR,
    AV_PIX_FMT_BGRA,
    AV_PIX_FMT_GRAY16BE,
    AV_PIX_FMT_GRAY16LE,
    AV_PIX_FMT_YUV440P,
    AV_PIX_FMT_YUVJ440P,
    AV_PIX_FMT_YUVA420P,
    AV_PIX_FMT_RGB48BE,
    AV_PIX_FMT_RGB48LE,
    AV_PIX_FMT_RGB565BE,
    AV_PIX_FMT_RGB565LE,
    AV_PIX_FMT_RGB555BE,
    AV_PIX_FMT_RGB555LE,
    AV_PIX_FMT_BGR565BE,
    AV_PIX_FMT_BGR565LE,
    AV_PIX_FMT_BGR555BE,
    AV_PIX_FMT_BGR555LE,
    AV_PIX_FMT_VAAPI_MOCO,
    AV_PIX_FMT_VAAPI_IDCT,
    AV_PIX_FMT_VAAPI_VLD,
    AV_PIX_FMT_VAAPI = AV_PIX_FMT_VAAPI_VLD,
    AV_PIX_FMT_YUV420P16LE,
    AV_PIX_FMT_YUV420P16BE,
    AV_PIX_FMT_YUV422P16LE,
    AV_PIX_FMT_YUV422P16BE,
    AV_PIX_FMT_YUV444P16LE,
    AV_PIX_FMT_YUV444P16BE,
    AV_PIX_FMT_DXVA2_VLD,
    AV_PIX_FMT_RGB444LE,
    AV_PIX_FMT_RGB444BE,
    AV_PIX_FMT_BGR444LE,
    AV_PIX_FMT_BGR444BE,
    AV_PIX_FMT_YA8,
    AV_PIX_FMT_Y400A = AV_PIX_FMT_YA8,
    AV_PIX_FMT_GRAY8A= AV_PIX_FMT_YA8,
    AV_PIX_FMT_BGR48BE,
    AV_PIX_FMT_BGR48LE,
    AV_PIX_FMT_YUV420P9BE,
    AV_PIX_FMT_YUV420P9LE,
    AV_PIX_FMT_YUV420P10BE,
    AV_PIX_FMT_YUV420P10LE,
    AV_PIX_FMT_YUV422P10BE,
    AV_PIX_FMT_YUV422P10LE,
    AV_PIX_FMT_YUV444P9BE,
    AV_PIX_FMT_YUV444P9LE,
    AV_PIX_FMT_YUV444P10BE,
    AV_PIX_FMT_YUV444P10LE,
    AV_PIX_FMT_YUV422P9BE,
    AV_PIX_FMT_YUV422P9LE,
    AV_PIX_FMT_GBRP,
    AV_PIX_FMT_GBR24P = AV_PIX_FMT_GBRP,
    AV_PIX_FMT_GBRP9BE,
    AV_PIX_FMT_GBRP9LE,
    AV_PIX_FMT_GBRP10BE,
    AV_PIX_FMT_GBRP10LE,
    AV_PIX_FMT_GBRP16BE,
    AV_PIX_FMT_GBRP16LE,
    AV_PIX_FMT_YUVA422P,
    AV_PIX_FMT_YUVA444P,
    AV_PIX_FMT_YUVA420P9BE,
    AV_PIX_FMT_YUVA420P9LE,
    AV_PIX_FMT_YUVA422P9BE,
    AV_PIX_FMT_YUVA422P9LE,
    AV_PIX_FMT_YUVA444P9BE,
    AV_PIX_FMT_YUVA444P9LE,
    AV_PIX_FMT_YUVA420P10BE,
    AV_PIX_FMT_YUVA420P10LE,
    AV_PIX_FMT_YUVA422P10BE,
    AV_PIX_FMT_YUVA422P10LE,
    AV_PIX_FMT_YUVA444P10BE,
    AV_PIX_FMT_YUVA444P10LE,
    AV_PIX_FMT_YUVA420P16BE,
    AV_PIX_FMT_YUVA420P16LE,
    AV_PIX_FMT_YUVA422P16BE,
    AV_PIX_FMT_YUVA422P16LE,
    AV_PIX_FMT_YUVA444P16BE,
    AV_PIX_FMT_YUVA444P16LE,
    AV_PIX_FMT_VDPAU,
    AV_PIX_FMT_XYZ12LE,
    AV_PIX_FMT_XYZ12BE,
    AV_PIX_FMT_NV16,
    AV_PIX_FMT_NV20LE,
    AV_PIX_FMT_NV20BE,
    AV_PIX_FMT_RGBA64BE,
    AV_PIX_FMT_RGBA64LE,
    AV_PIX_FMT_BGRA64BE,
    AV_PIX_FMT_BGRA64LE,
    AV_PIX_FMT_YVYU422,
    AV_PIX_FMT_YA16BE,
    AV_PIX_FMT_YA16LE,
    AV_PIX_FMT_GBRAP,
    AV_PIX_FMT_GBRAP16BE,
    AV_PIX_FMT_GBRAP16LE,
    AV_PIX_FMT_QSV,
    AV_PIX_FMT_MMAL,
    AV_PIX_FMT_D3D11VA_VLD,
    AV_PIX_FMT_CUDA,
    AV_PIX_FMT_0RGB,
    AV_PIX_FMT_RGB0,
    AV_PIX_FMT_0BGR,
    AV_PIX_FMT_BGR0,
    AV_PIX_FMT_YUV420P12BE,
    AV_PIX_FMT_YUV420P12LE,
    AV_PIX_FMT_YUV420P14BE,
    AV_PIX_FMT_YUV420P14LE,
    AV_PIX_FMT_YUV422P12BE,
    AV_PIX_FMT_YUV422P12LE,
    AV_PIX_FMT_YUV422P14BE,
    AV_PIX_FMT_YUV422P14LE,
    AV_PIX_FMT_YUV444P12BE,
    AV_PIX_FMT_YUV444P12LE,
    AV_PIX_FMT_YUV444P14BE,
    AV_PIX_FMT_YUV444P14LE,
    AV_PIX_FMT_GBRP12BE,
    AV_PIX_FMT_GBRP12LE,
    AV_PIX_FMT_GBRP14BE,
    AV_PIX_FMT_GBRP14LE,
    AV_PIX_FMT_YUVJ411P,
    AV_PIX_FMT_BAYER_BGGR8,
    AV_PIX_FMT_BAYER_RGGB8,
    AV_PIX_FMT_BAYER_GBRG8,
    AV_PIX_FMT_BAYER_GRBG8,
    AV_PIX_FMT_BAYER_BGGR16LE,
    AV_PIX_FMT_BAYER_BGGR16BE,
    AV_PIX_FMT_BAYER_RGGB16LE,
    AV_PIX_FMT_BAYER_RGGB16BE,
    AV_PIX_FMT_BAYER_GBRG16LE,
    AV_PIX_FMT_BAYER_GBRG16BE,
    AV_PIX_FMT_BAYER_GRBG16LE,
    AV_PIX_FMT_BAYER_GRBG16BE,
    AV_PIX_FMT_XVMC,
    AV_PIX_FMT_YUV440P10LE,
    AV_PIX_FMT_YUV440P10BE,
    AV_PIX_FMT_YUV440P12LE,
    AV_PIX_FMT_YUV440P12BE,
    AV_PIX_FMT_AYUV64LE,
    AV_PIX_FMT_AYUV64BE,
    AV_PIX_FMT_VIDEOTOOLBOX,
    AV_PIX_FMT_P010LE,
    AV_PIX_FMT_P010BE,
    AV_PIX_FMT_GBRAP12BE,
    AV_PIX_FMT_GBRAP12LE,
    AV_PIX_FMT_GBRAP10BE,
    AV_PIX_FMT_GBRAP10LE,
    AV_PIX_FMT_MEDIACODEC,
    AV_PIX_FMT_GRAY12BE,
    AV_PIX_FMT_GRAY12LE,
    AV_PIX_FMT_GRAY10BE,
    AV_PIX_FMT_GRAY10LE,
    AV_PIX_FMT_P016LE,
    AV_PIX_FMT_P016BE,
    AV_PIX_FMT_D3D11,
    AV_PIX_FMT_GRAY9BE,
    AV_PIX_FMT_GRAY9LE,
    AV_PIX_FMT_GBRPF32BE,
    AV_PIX_FMT_GBRPF32LE,
    AV_PIX_FMT_GBRAPF32BE,
    AV_PIX_FMT_GBRAPF32LE,
    AV_PIX_FMT_DRM_PRIME,
    AV_PIX_FMT_OPENCL,
    AV_PIX_FMT_GRAY14BE,
    AV_PIX_FMT_GRAY14LE,
    AV_PIX_FMT_GRAYF32BE,
    AV_PIX_FMT_GRAYF32LE,
    AV_PIX_FMT_YUVA422P12BE,
    AV_PIX_FMT_YUVA422P12LE,
    AV_PIX_FMT_YUVA444P12BE,
    AV_PIX_FMT_YUVA444P12LE,
    AV_PIX_FMT_NV24,
    AV_PIX_FMT_NV42,
    AV_PIX_FMT_VULKAN,
    AV_PIX_FMT_Y210BE,
    AV_PIX_FMT_Y210LE,
    AV_PIX_FMT_X2RGB10LE,
    AV_PIX_FMT_X2RGB10BE,
    AV_PIX_FMT_NB
};
#define AV_PIX_FMT_NE(be,le) AV_PIX_FMT_ ##le
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
#define AV_PIX_FMT_YUV420P9 AV_PIX_FMT_NE(YUV420P9BE , YUV420P9LE)
#define AV_PIX_FMT_YUV422P9 AV_PIX_FMT_NE(YUV422P9BE , YUV422P9LE)
#define AV_PIX_FMT_YUV444P9 AV_PIX_FMT_NE(YUV444P9BE , YUV444P9LE)
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
#define AV_PIX_FMT_GBRP9 AV_PIX_FMT_NE(GBRP9BE , GBRP9LE)
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
#define AV_PIX_FMT_YUVA420P9 AV_PIX_FMT_NE(YUVA420P9BE , YUVA420P9LE)
#define AV_PIX_FMT_YUVA422P9 AV_PIX_FMT_NE(YUVA422P9BE , YUVA422P9LE)
#define AV_PIX_FMT_YUVA444P9 AV_PIX_FMT_NE(YUVA444P9BE , YUVA444P9LE)
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
#define AV_PIX_FMT_Y210 AV_PIX_FMT_NE(Y210BE, Y210LE)
#define AV_PIX_FMT_X2RGB10 AV_PIX_FMT_NE(X2RGB10BE, X2RGB10LE)

enum AVColorPrimaries {
    AVCOL_PRI_RESERVED0 = 0,
    AVCOL_PRI_BT709 = 1,
    AVCOL_PRI_UNSPECIFIED = 2,
    AVCOL_PRI_RESERVED = 3,
    AVCOL_PRI_BT470M = 4,
    AVCOL_PRI_BT470BG = 5,
    AVCOL_PRI_SMPTE170M = 6,
    AVCOL_PRI_SMPTE240M = 7,
    AVCOL_PRI_FILM = 8,
    AVCOL_PRI_BT2020 = 9,
    AVCOL_PRI_SMPTE428 = 10,
    AVCOL_PRI_SMPTEST428_1 = AVCOL_PRI_SMPTE428,
    AVCOL_PRI_SMPTE431 = 11,
    AVCOL_PRI_SMPTE432 = 12,
    AVCOL_PRI_EBU3213 = 22,
    AVCOL_PRI_JEDEC_P22 = AVCOL_PRI_EBU3213,
    AVCOL_PRI_NB
};

enum AVColorTransferCharacteristic {
    AVCOL_TRC_RESERVED0 = 0,
    AVCOL_TRC_BT709 = 1,
    AVCOL_TRC_UNSPECIFIED = 2,
    AVCOL_TRC_RESERVED = 3,
    AVCOL_TRC_GAMMA22 = 4,
    AVCOL_TRC_GAMMA28 = 5,
    AVCOL_TRC_SMPTE170M = 6,
    AVCOL_TRC_SMPTE240M = 7,
    AVCOL_TRC_LINEAR = 8,
    AVCOL_TRC_LOG = 9,
    AVCOL_TRC_LOG_SQRT = 10,
    AVCOL_TRC_IEC61966_2_4 = 11,
    AVCOL_TRC_BT1361_ECG = 12,
    AVCOL_TRC_IEC61966_2_1 = 13,
    AVCOL_TRC_BT2020_10 = 14,
    AVCOL_TRC_BT2020_12 = 15,
    AVCOL_TRC_SMPTE2084 = 16,
    AVCOL_TRC_SMPTEST2084 = AVCOL_TRC_SMPTE2084,
    AVCOL_TRC_SMPTE428 = 17,
    AVCOL_TRC_SMPTEST428_1 = AVCOL_TRC_SMPTE428,
    AVCOL_TRC_ARIB_STD_B67 = 18,
    AVCOL_TRC_NB
};

enum AVColorSpace {
    AVCOL_SPC_RGB = 0,
    AVCOL_SPC_BT709 = 1,
    AVCOL_SPC_UNSPECIFIED = 2,
    AVCOL_SPC_RESERVED = 3,
    AVCOL_SPC_FCC = 4,
    AVCOL_SPC_BT470BG = 5,
    AVCOL_SPC_SMPTE170M = 6,
    AVCOL_SPC_SMPTE240M = 7,
    AVCOL_SPC_YCGCO = 8,
    AVCOL_SPC_YCOCG = AVCOL_SPC_YCGCO,
    AVCOL_SPC_BT2020_NCL = 9,
    AVCOL_SPC_BT2020_CL = 10,
    AVCOL_SPC_SMPTE2085 = 11,
    AVCOL_SPC_CHROMA_DERIVED_NCL = 12,
    AVCOL_SPC_CHROMA_DERIVED_CL = 13,
    AVCOL_SPC_ICTCP = 14,
    AVCOL_SPC_NB
};
enum AVColorRange {
    AVCOL_RANGE_UNSPECIFIED = 0,
    AVCOL_RANGE_MPEG = 1,
    AVCOL_RANGE_JPEG = 2,
    AVCOL_RANGE_NB
};
enum AVChromaLocation {
    AVCHROMA_LOC_UNSPECIFIED = 0,
    AVCHROMA_LOC_LEFT = 1,
    AVCHROMA_LOC_CENTER = 2,
    AVCHROMA_LOC_TOPLEFT = 3,
    AVCHROMA_LOC_TOP = 4,
    AVCHROMA_LOC_BOTTOMLEFT = 5,
    AVCHROMA_LOC_BOTTOM = 6,
    AVCHROMA_LOC_NB
};

static inline void *av_x_if_null(const void *p, const void *x)
{
    return (void *)(intptr_t)(p ? p : x);
}
unsigned av_int_list_length_for_size(unsigned elsize,
                                     const void *list, uint64_t term) ;
#define av_int_list_length(list,term) av_int_list_length_for_size(sizeof(*(list)), list, term)

FILE *av_fopen_utf8(const char *path, const char *mode);

AVRational av_get_time_base_q(void);
#define AV_FOURCC_MAX_STRING_SIZE 32
#define av_fourcc2str(fourcc) av_fourcc_make_string((char[AV_FOURCC_MAX_STRING_SIZE]){0}, fourcc)
char *av_fourcc_make_string(char *buf, uint32_t fourcc);

typedef struct AVExpr AVExpr;
int av_expr_parse_and_eval(double *res, const char *s,
                           const char * const *const_names, const double *const_values,
                           const char * const *func1_names, double (* const *funcs1)(void *, double),
                           const char * const *func2_names, double (* const *funcs2)(void *, double, double),
                           void *opaque, int log_offset, void *log_ctx);
int av_expr_parse(AVExpr **expr, const char *s,
                  const char * const *const_names,
                  const char * const *func1_names, double (* const *funcs1)(void *, double),
                  const char * const *func2_names, double (* const *funcs2)(void *, double, double),
                  int log_offset, void *log_ctx);
double av_expr_eval(AVExpr *e, const double *const_values, void *opaque);
int av_expr_count_vars(AVExpr *e, unsigned *counter, int size);
int av_expr_count_func(AVExpr *e, unsigned *counter, int size, int arg);

void av_expr_free(AVExpr *e);
double av_strtod(const char *numstr, char **tail);
#define AVUTIL_PIXDESC_H 

typedef struct AVComponentDescriptor {
    int plane;
    int step;
    int offset;
    int shift;
    int depth;
    int step_minus1;
    int depth_minus1;
    int offset_plus1;
} AVComponentDescriptor;
typedef struct AVPixFmtDescriptor {
    const char *name;
    uint8_t nb_components;
    uint8_t log2_chroma_w;
    uint8_t log2_chroma_h;
    uint64_t flags;
    AVComponentDescriptor comp[4];
    const char *alias;
} AVPixFmtDescriptor;
#define AV_PIX_FMT_FLAG_BE (1 << 0)
#define AV_PIX_FMT_FLAG_PAL (1 << 1)
#define AV_PIX_FMT_FLAG_BITSTREAM (1 << 2)
#define AV_PIX_FMT_FLAG_HWACCEL (1 << 3)
#define AV_PIX_FMT_FLAG_PLANAR (1 << 4)
#define AV_PIX_FMT_FLAG_RGB (1 << 5)
#define AV_PIX_FMT_FLAG_PSEUDOPAL (1 << 6)
#define AV_PIX_FMT_FLAG_ALPHA (1 << 7)
#define AV_PIX_FMT_FLAG_BAYER (1 << 8)
#define AV_PIX_FMT_FLAG_FLOAT (1 << 9)
int av_get_bits_per_pixel(const AVPixFmtDescriptor *pixdesc);

int av_get_padded_bits_per_pixel(const AVPixFmtDescriptor *pixdesc);

const AVPixFmtDescriptor *av_pix_fmt_desc_get(enum AVPixelFormat pix_fmt);
const AVPixFmtDescriptor *av_pix_fmt_desc_next(const AVPixFmtDescriptor *prev);

enum AVPixelFormat av_pix_fmt_desc_get_id(const AVPixFmtDescriptor *desc);
int av_pix_fmt_get_chroma_sub_sample(enum AVPixelFormat pix_fmt,
                                     int *h_shift, int *v_shift);

int av_pix_fmt_count_planes(enum AVPixelFormat pix_fmt);

const char *av_color_range_name(enum AVColorRange range);

int av_color_range_from_name(const char *name);

const char *av_color_primaries_name(enum AVColorPrimaries primaries);

int av_color_primaries_from_name(const char *name);

const char *av_color_transfer_name(enum AVColorTransferCharacteristic transfer);

int av_color_transfer_from_name(const char *name);

const char *av_color_space_name(enum AVColorSpace space);

int av_color_space_from_name(const char *name);

const char *av_chroma_location_name(enum AVChromaLocation location);

int av_chroma_location_from_name(const char *name);
enum AVPixelFormat av_get_pix_fmt(const char *name);

const char *av_get_pix_fmt_name(enum AVPixelFormat pix_fmt);
char *av_get_pix_fmt_string(char *buf, int buf_size,
                            enum AVPixelFormat pix_fmt);
void av_read_image_line2(void *dst, const uint8_t *data[4],
                        const int linesize[4], const AVPixFmtDescriptor *desc,
                        int x, int y, int c, int w, int read_pal_component,
                        int dst_element_size);

void av_read_image_line(uint16_t *dst, const uint8_t *data[4],
                        const int linesize[4], const AVPixFmtDescriptor *desc,
                        int x, int y, int c, int w, int read_pal_component);
void av_write_image_line2(const void *src, uint8_t *data[4],
                         const int linesize[4], const AVPixFmtDescriptor *desc,
                         int x, int y, int c, int w, int src_element_size);

void av_write_image_line(const uint16_t *src, uint8_t *data[4],
                         const int linesize[4], const AVPixFmtDescriptor *desc,
                         int x, int y, int c, int w);
enum AVPixelFormat av_pix_fmt_swap_endianness(enum AVPixelFormat pix_fmt);
#define FF_LOSS_RESOLUTION 0x0001
#define FF_LOSS_DEPTH 0x0002
#define FF_LOSS_COLORSPACE 0x0004
#define FF_LOSS_ALPHA 0x0008
#define FF_LOSS_COLORQUANT 0x0010
#define FF_LOSS_CHROMA 0x0020
int av_get_pix_fmt_loss(enum AVPixelFormat dst_pix_fmt,
                        enum AVPixelFormat src_pix_fmt,
                        int has_alpha);
enum AVPixelFormat av_find_best_pix_fmt_of_2(enum AVPixelFormat dst_pix_fmt1, enum AVPixelFormat dst_pix_fmt2,
                                             enum AVPixelFormat src_pix_fmt, int has_alpha, int *loss_ptr);
#define AVUTIL_IMGUTILS_H 
void av_image_fill_max_pixsteps(int max_pixsteps[4], int max_pixstep_comps[4],
                                const AVPixFmtDescriptor *pixdesc);

int av_image_get_linesize(enum AVPixelFormat pix_fmt, int width, int plane);
int av_image_fill_linesizes(int linesizes[4], enum AVPixelFormat pix_fmt, int width);
int av_image_fill_plane_sizes(size_t size[4], enum AVPixelFormat pix_fmt,
                              int height, const ptrdiff_t linesizes[4]);
int av_image_fill_pointers(uint8_t *data[4], enum AVPixelFormat pix_fmt, int height,
                           uint8_t *ptr, const int linesizes[4]);
int av_image_alloc(uint8_t *pointers[4], int linesizes[4],
                   int w, int h, enum AVPixelFormat pix_fmt, int align);
void av_image_copy_plane(uint8_t *dst, int dst_linesize,
                         const uint8_t *src, int src_linesize,
                         int bytewidth, int height);

void av_image_copy(uint8_t *dst_data[4], int dst_linesizes[4],
                   const uint8_t *src_data[4], const int src_linesizes[4],
                   enum AVPixelFormat pix_fmt, int width, int height);
void av_image_copy_uc_from(uint8_t *dst_data[4], const ptrdiff_t dst_linesizes[4],
                           const uint8_t *src_data[4], const ptrdiff_t src_linesizes[4],
                           enum AVPixelFormat pix_fmt, int width, int height);
int av_image_fill_arrays(uint8_t *dst_data[4], int dst_linesize[4],
                         const uint8_t *src,
                         enum AVPixelFormat pix_fmt, int width, int height, int align);
int av_image_get_buffer_size(enum AVPixelFormat pix_fmt, int width, int height, int align);
int av_image_copy_to_buffer(uint8_t *dst, int dst_size,
                            const uint8_t * const src_data[4], const int src_linesize[4],
                            enum AVPixelFormat pix_fmt, int width, int height, int align);
int av_image_check_size(unsigned int w, unsigned int h, int log_offset, void *log_ctx);
int av_image_check_size2(unsigned int w, unsigned int h, int64_t max_pixels, enum AVPixelFormat pix_fmt, int log_offset, void *log_ctx);
int av_image_check_sar(unsigned int w, unsigned int h, AVRational sar);
int av_image_fill_black(uint8_t *dst_data[4], const ptrdiff_t dst_linesize[4],
                        enum AVPixelFormat pix_fmt, enum AVColorRange range,
                        int width, int height);
#define AVUTIL_DICT_H 
#define AV_DICT_MATCH_CASE 1
#define AV_DICT_IGNORE_SUFFIX 2
#define AV_DICT_DONT_STRDUP_KEY 4
#define AV_DICT_DONT_STRDUP_VAL 8
#define AV_DICT_DONT_OVERWRITE 16
#define AV_DICT_APPEND 32
#define AV_DICT_MULTIKEY 64

typedef struct AVDictionaryEntry {
    char *key;
    char *value;
} AVDictionaryEntry;

typedef struct AVDictionary AVDictionary;
AVDictionaryEntry *av_dict_get(const AVDictionary *m, const char *key,
                               const AVDictionaryEntry *prev, int flags);

int av_dict_count(const AVDictionary *m);
int av_dict_set(AVDictionary **pm, const char *key, const char *value, int flags);

int av_dict_set_int(AVDictionary **pm, const char *key, int64_t value, int flags);
int av_dict_parse_string(AVDictionary **pm, const char *str,
                         const char *key_val_sep, const char *pairs_sep,
                         int flags);
int av_dict_copy(AVDictionary **dst, const AVDictionary *src, int flags);

void av_dict_free(AVDictionary **m);
int av_dict_get_string(const AVDictionary *m, char **buffer,
                       const char key_val_sep, const char pairs_sep);
#define AVUTIL_PARSEUTILS_H 

int av_parse_ratio(AVRational *q, const char *str, int max,
                   int log_offset, void *log_ctx);
#define av_parse_ratio_quiet(rate,str,max) av_parse_ratio(rate, str, max, AV_LOG_MAX_OFFSET, NULL)
int av_parse_video_size(int *width_ptr, int *height_ptr, const char *str);
int av_parse_video_rate(AVRational *rate, const char *str);
int av_parse_color(uint8_t *rgba_color, const char *color_string, int slen,
                   void *log_ctx);
const char *av_get_known_color_name(int color_idx, const uint8_t **rgb);
int av_parse_time(int64_t *timeval, const char *timestr, int duration);

int av_find_info_tag(char *arg, int arg_size, const char *tag1, const char *info);
char *av_small_strptime(const char *p, const char *fmt, struct tm *dt);

time_t av_timegm(struct tm *tm);
#define AVUTIL_SAMPLEFMT_H 
enum AVSampleFormat {
    AV_SAMPLE_FMT_NONE = -1,
    AV_SAMPLE_FMT_U8,
    AV_SAMPLE_FMT_S16,
    AV_SAMPLE_FMT_S32,
    AV_SAMPLE_FMT_FLT,
    AV_SAMPLE_FMT_DBL,
    AV_SAMPLE_FMT_U8P,
    AV_SAMPLE_FMT_S16P,
    AV_SAMPLE_FMT_S32P,
    AV_SAMPLE_FMT_FLTP,
    AV_SAMPLE_FMT_DBLP,
    AV_SAMPLE_FMT_S64,
    AV_SAMPLE_FMT_S64P,
    AV_SAMPLE_FMT_NB
};

const char *av_get_sample_fmt_name(enum AVSampleFormat sample_fmt);

enum AVSampleFormat av_get_sample_fmt(const char *name);

enum AVSampleFormat av_get_alt_sample_fmt(enum AVSampleFormat sample_fmt, int planar);
enum AVSampleFormat av_get_packed_sample_fmt(enum AVSampleFormat sample_fmt);
enum AVSampleFormat av_get_planar_sample_fmt(enum AVSampleFormat sample_fmt);
char *av_get_sample_fmt_string(char *buf, int buf_size, enum AVSampleFormat sample_fmt);
int av_get_bytes_per_sample(enum AVSampleFormat sample_fmt);

int av_sample_fmt_is_planar(enum AVSampleFormat sample_fmt);
int av_samples_get_buffer_size(int *linesize, int nb_channels, int nb_samples,
                               enum AVSampleFormat sample_fmt, int align);
int av_samples_fill_arrays(uint8_t **audio_data, int *linesize,
                           const uint8_t *buf,
                           int nb_channels, int nb_samples,
                           enum AVSampleFormat sample_fmt, int align);
int av_samples_alloc(uint8_t **audio_data, int *linesize, int nb_channels,
                     int nb_samples, enum AVSampleFormat sample_fmt, int align);
int av_samples_alloc_array_and_samples(uint8_t ***audio_data, int *linesize, int nb_channels,
                                       int nb_samples, enum AVSampleFormat sample_fmt, int align);
int av_samples_copy(uint8_t **dst, uint8_t * const *src, int dst_offset,
                    int src_offset, int nb_samples, int nb_channels,
                    enum AVSampleFormat sample_fmt);
int av_samples_set_silence(uint8_t **audio_data, int offset, int nb_samples,
                           int nb_channels, enum AVSampleFormat sample_fmt);
#define AVUTIL_AVASSERT_H 
#define av_assert0(cond) do { if (!(cond)) { av_log(NULL, AV_LOG_PANIC, "Assertion %s failed at %s:%d\n", AV_STRINGIFY(cond), __FILE__, __LINE__); abort(); } } while (0)
#define av_assert1(cond) ((void)0)
#define av_assert2(cond) ((void)0)
#define av_assert2_fpu() ((void)0)

void av_assert0_fpu(void);
#define AVUTIL_TIME_H 

int64_t av_gettime(void);
int64_t av_gettime_relative(void);

int av_gettime_relative_is_monotonic(void);
int av_usleep(unsigned usec);
#define AVUTIL_BPRINT_H 
#define FF_PAD_STRUCTURE(name,size,...) struct ff_pad_helper_ ##name { __VA_ARGS__ }; typedef struct name { __VA_ARGS__ char reserved_padding[size - sizeof(struct ff_pad_helper_ ##name)]; } name;
struct ff_pad_helper_AVBPrint { char *str; unsigned len; unsigned size; unsigned size_max; char reserved_internal_buffer[1]; }; typedef struct AVBPrint { char *str; unsigned len; unsigned size; unsigned size_max; char reserved_internal_buffer[1]; char reserved_padding[1024 - sizeof(struct ff_pad_helper_AVBPrint)]; } AVBPrint;
#define AV_BPRINT_SIZE_UNLIMITED ((unsigned)-1)
#define AV_BPRINT_SIZE_AUTOMATIC 1
#define AV_BPRINT_SIZE_COUNT_ONLY 0
void av_bprint_init(AVBPrint *buf, unsigned size_init, unsigned size_max);
void av_bprint_init_for_buffer(AVBPrint *buf, char *buffer, unsigned size);

void av_bprintf(AVBPrint *buf, const char *fmt, ...) ;

void av_vbprintf(AVBPrint *buf, const char *fmt, va_list vl_arg);

void av_bprint_chars(AVBPrint *buf, char c, unsigned n);
void av_bprint_append_data(AVBPrint *buf, const char *data, unsigned size);

struct tm;
void av_bprint_strftime(AVBPrint *buf, const char *fmt, const struct tm *tm);
void av_bprint_get_buffer(AVBPrint *buf, unsigned size,
                          unsigned char **mem, unsigned *actual_size);

void av_bprint_clear(AVBPrint *buf);

static inline int av_bprint_is_complete(const AVBPrint *buf)
{
    return buf->len < buf->size;
}
int av_bprint_finalize(AVBPrint *buf, char **ret_str);
void av_bprint_escape(AVBPrint *dstbuf, const char *src, const char *special_chars,
                      enum AVEscapeMode mode, int flags);
#define AVFORMAT_AVFORMAT_H 
#define AVCODEC_AVCODEC_H 
#define AVUTIL_BUFFER_H 
typedef struct AVBuffer AVBuffer;

typedef struct AVBufferRef {
    AVBuffer *buffer;
    uint8_t *data;
    int size;
} AVBufferRef;

AVBufferRef *av_buffer_alloc(int size);

AVBufferRef *av_buffer_allocz(int size);
#define AV_BUFFER_FLAG_READONLY (1 << 0)
AVBufferRef *av_buffer_create(uint8_t *data, int size,
                              void (*free)(void *opaque, uint8_t *data),
                              void *opaque, int flags);

void av_buffer_default_free(void *opaque, uint8_t *data);

AVBufferRef *av_buffer_ref(AVBufferRef *buf);

void av_buffer_unref(AVBufferRef **buf);

int av_buffer_is_writable(const AVBufferRef *buf);

void *av_buffer_get_opaque(const AVBufferRef *buf);

int av_buffer_get_ref_count(const AVBufferRef *buf);
int av_buffer_make_writable(AVBufferRef **buf);
int av_buffer_realloc(AVBufferRef **buf, int size);
int av_buffer_replace(AVBufferRef **dst, AVBufferRef *src);
typedef struct AVBufferPool AVBufferPool;
AVBufferPool *av_buffer_pool_init(int size, AVBufferRef* (*alloc)(int size));
AVBufferPool *av_buffer_pool_init2(int size, void *opaque,
                                   AVBufferRef* (*alloc)(void *opaque, int size),
                                   void (*pool_free)(void *opaque));
void av_buffer_pool_uninit(AVBufferPool **pool);

AVBufferRef *av_buffer_pool_get(AVBufferPool *pool);
void *av_buffer_pool_buffer_get_opaque(AVBufferRef *ref);
#define AVUTIL_CPU_H 
#define AV_CPU_FLAG_FORCE 0x80000000
#define AV_CPU_FLAG_MMX 0x0001
#define AV_CPU_FLAG_MMXEXT 0x0002
#define AV_CPU_FLAG_MMX2 0x0002
#define AV_CPU_FLAG_3DNOW 0x0004
#define AV_CPU_FLAG_SSE 0x0008
#define AV_CPU_FLAG_SSE2 0x0010
#define AV_CPU_FLAG_SSE2SLOW 0x40000000
#define AV_CPU_FLAG_3DNOWEXT 0x0020
#define AV_CPU_FLAG_SSE3 0x0040
#define AV_CPU_FLAG_SSE3SLOW 0x20000000
#define AV_CPU_FLAG_SSSE3 0x0080
#define AV_CPU_FLAG_SSSE3SLOW 0x4000000
#define AV_CPU_FLAG_ATOM 0x10000000
#define AV_CPU_FLAG_SSE4 0x0100
#define AV_CPU_FLAG_SSE42 0x0200
#define AV_CPU_FLAG_AESNI 0x80000
#define AV_CPU_FLAG_AVX 0x4000
#define AV_CPU_FLAG_AVXSLOW 0x8000000
#define AV_CPU_FLAG_XOP 0x0400
#define AV_CPU_FLAG_FMA4 0x0800
#define AV_CPU_FLAG_CMOV 0x1000
#define AV_CPU_FLAG_AVX2 0x8000
#define AV_CPU_FLAG_FMA3 0x10000
#define AV_CPU_FLAG_BMI1 0x20000
#define AV_CPU_FLAG_BMI2 0x40000
#define AV_CPU_FLAG_AVX512 0x100000
#define AV_CPU_FLAG_ALTIVEC 0x0001
#define AV_CPU_FLAG_VSX 0x0002
#define AV_CPU_FLAG_POWER8 0x0004
#define AV_CPU_FLAG_ARMV5TE (1 << 0)
#define AV_CPU_FLAG_ARMV6 (1 << 1)
#define AV_CPU_FLAG_ARMV6T2 (1 << 2)
#define AV_CPU_FLAG_VFP (1 << 3)
#define AV_CPU_FLAG_VFPV3 (1 << 4)
#define AV_CPU_FLAG_NEON (1 << 5)
#define AV_CPU_FLAG_ARMV8 (1 << 6)
#define AV_CPU_FLAG_VFP_VM (1 << 7)
#define AV_CPU_FLAG_SETEND (1 <<16)
#define AV_CPU_FLAG_MMI (1 << 0)
#define AV_CPU_FLAG_MSA (1 << 1)

int av_get_cpu_flags(void);

void av_force_cpu_flags(int flags);
 void av_set_cpu_flags_mask(int mask);

int av_parse_cpu_flags(const char *s);

int av_parse_cpu_caps(unsigned *flags, const char *s);

int av_cpu_count(void);
size_t av_cpu_max_align(void);
#define AVUTIL_CHANNEL_LAYOUT_H 
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
#define AV_CH_STEREO_LEFT 0x20000000
#define AV_CH_STEREO_RIGHT 0x40000000
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
#define AV_CH_LAYOUT_NATIVE 0x8000000000000000ULL
#define AV_CH_LAYOUT_MONO (AV_CH_FRONT_CENTER)
#define AV_CH_LAYOUT_STEREO (AV_CH_FRONT_LEFT|AV_CH_FRONT_RIGHT)
#define AV_CH_LAYOUT_2POINT1 (AV_CH_LAYOUT_STEREO|AV_CH_LOW_FREQUENCY)
#define AV_CH_LAYOUT_2_1 (AV_CH_LAYOUT_STEREO|AV_CH_BACK_CENTER)
#define AV_CH_LAYOUT_SURROUND (AV_CH_LAYOUT_STEREO|AV_CH_FRONT_CENTER)
#define AV_CH_LAYOUT_3POINT1 (AV_CH_LAYOUT_SURROUND|AV_CH_LOW_FREQUENCY)
#define AV_CH_LAYOUT_4POINT0 (AV_CH_LAYOUT_SURROUND|AV_CH_BACK_CENTER)
#define AV_CH_LAYOUT_4POINT1 (AV_CH_LAYOUT_4POINT0|AV_CH_LOW_FREQUENCY)
#define AV_CH_LAYOUT_2_2 (AV_CH_LAYOUT_STEREO|AV_CH_SIDE_LEFT|AV_CH_SIDE_RIGHT)
#define AV_CH_LAYOUT_QUAD (AV_CH_LAYOUT_STEREO|AV_CH_BACK_LEFT|AV_CH_BACK_RIGHT)
#define AV_CH_LAYOUT_5POINT0 (AV_CH_LAYOUT_SURROUND|AV_CH_SIDE_LEFT|AV_CH_SIDE_RIGHT)
#define AV_CH_LAYOUT_5POINT1 (AV_CH_LAYOUT_5POINT0|AV_CH_LOW_FREQUENCY)
#define AV_CH_LAYOUT_5POINT0_BACK (AV_CH_LAYOUT_SURROUND|AV_CH_BACK_LEFT|AV_CH_BACK_RIGHT)
#define AV_CH_LAYOUT_5POINT1_BACK (AV_CH_LAYOUT_5POINT0_BACK|AV_CH_LOW_FREQUENCY)
#define AV_CH_LAYOUT_6POINT0 (AV_CH_LAYOUT_5POINT0|AV_CH_BACK_CENTER)
#define AV_CH_LAYOUT_6POINT0_FRONT (AV_CH_LAYOUT_2_2|AV_CH_FRONT_LEFT_OF_CENTER|AV_CH_FRONT_RIGHT_OF_CENTER)
#define AV_CH_LAYOUT_HEXAGONAL (AV_CH_LAYOUT_5POINT0_BACK|AV_CH_BACK_CENTER)
#define AV_CH_LAYOUT_6POINT1 (AV_CH_LAYOUT_5POINT1|AV_CH_BACK_CENTER)
#define AV_CH_LAYOUT_6POINT1_BACK (AV_CH_LAYOUT_5POINT1_BACK|AV_CH_BACK_CENTER)
#define AV_CH_LAYOUT_6POINT1_FRONT (AV_CH_LAYOUT_6POINT0_FRONT|AV_CH_LOW_FREQUENCY)
#define AV_CH_LAYOUT_7POINT0 (AV_CH_LAYOUT_5POINT0|AV_CH_BACK_LEFT|AV_CH_BACK_RIGHT)
#define AV_CH_LAYOUT_7POINT0_FRONT (AV_CH_LAYOUT_5POINT0|AV_CH_FRONT_LEFT_OF_CENTER|AV_CH_FRONT_RIGHT_OF_CENTER)
#define AV_CH_LAYOUT_7POINT1 (AV_CH_LAYOUT_5POINT1|AV_CH_BACK_LEFT|AV_CH_BACK_RIGHT)
#define AV_CH_LAYOUT_7POINT1_WIDE (AV_CH_LAYOUT_5POINT1|AV_CH_FRONT_LEFT_OF_CENTER|AV_CH_FRONT_RIGHT_OF_CENTER)
#define AV_CH_LAYOUT_7POINT1_WIDE_BACK (AV_CH_LAYOUT_5POINT1_BACK|AV_CH_FRONT_LEFT_OF_CENTER|AV_CH_FRONT_RIGHT_OF_CENTER)
#define AV_CH_LAYOUT_OCTAGONAL (AV_CH_LAYOUT_5POINT0|AV_CH_BACK_LEFT|AV_CH_BACK_CENTER|AV_CH_BACK_RIGHT)
#define AV_CH_LAYOUT_HEXADECAGONAL (AV_CH_LAYOUT_OCTAGONAL|AV_CH_WIDE_LEFT|AV_CH_WIDE_RIGHT|AV_CH_TOP_BACK_LEFT|AV_CH_TOP_BACK_RIGHT|AV_CH_TOP_BACK_CENTER|AV_CH_TOP_FRONT_CENTER|AV_CH_TOP_FRONT_LEFT|AV_CH_TOP_FRONT_RIGHT)
#define AV_CH_LAYOUT_STEREO_DOWNMIX (AV_CH_STEREO_LEFT|AV_CH_STEREO_RIGHT)
#define AV_CH_LAYOUT_22POINT2 (AV_CH_LAYOUT_5POINT1_BACK|AV_CH_FRONT_LEFT_OF_CENTER|AV_CH_FRONT_RIGHT_OF_CENTER|AV_CH_BACK_CENTER|AV_CH_LOW_FREQUENCY_2|AV_CH_SIDE_LEFT|AV_CH_SIDE_RIGHT|AV_CH_TOP_FRONT_LEFT|AV_CH_TOP_FRONT_RIGHT|AV_CH_TOP_FRONT_CENTER|AV_CH_TOP_CENTER|AV_CH_TOP_BACK_LEFT|AV_CH_TOP_BACK_RIGHT|AV_CH_TOP_SIDE_LEFT|AV_CH_TOP_SIDE_RIGHT|AV_CH_TOP_BACK_CENTER|AV_CH_BOTTOM_FRONT_CENTER|AV_CH_BOTTOM_FRONT_LEFT|AV_CH_BOTTOM_FRONT_RIGHT)

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
uint64_t av_get_channel_layout(const char *name);
int av_get_extended_channel_layout(const char *name, uint64_t* channel_layout, int* nb_channels);
void av_get_channel_layout_string(char *buf, int buf_size, int nb_channels, uint64_t channel_layout);

struct AVBPrint;

void av_bprint_channel_layout(struct AVBPrint *bp, int nb_channels, uint64_t channel_layout);

int av_get_channel_layout_nb_channels(uint64_t channel_layout);

int64_t av_get_default_channel_layout(int nb_channels);
int av_get_channel_layout_channel_index(uint64_t channel_layout,
                                        uint64_t channel);

uint64_t av_channel_layout_extract_channel(uint64_t channel_layout, int index);

const char *av_get_channel_name(uint64_t channel);

const char *av_get_channel_description(uint64_t channel);
int av_get_standard_channel_layout(unsigned index, uint64_t *layout,
                                   const char **name);
#define AVUTIL_FRAME_H 

enum AVFrameSideDataType {
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
    AV_FRAME_DATA_QP_TABLE_PROPERTIES,
    AV_FRAME_DATA_QP_TABLE_DATA,
    AV_FRAME_DATA_S12M_TIMECODE,
    AV_FRAME_DATA_DYNAMIC_HDR_PLUS,
    AV_FRAME_DATA_REGIONS_OF_INTEREST,
    AV_FRAME_DATA_VIDEO_ENC_PARAMS,
    AV_FRAME_DATA_SEI_UNREGISTERED,
};

enum AVActiveFormatDescription {
    AV_AFD_SAME = 8,
    AV_AFD_4_3 = 9,
    AV_AFD_16_9 = 10,
    AV_AFD_14_9 = 11,
    AV_AFD_4_3_SP_14_9 = 13,
    AV_AFD_16_9_SP_14_9 = 14,
    AV_AFD_SP_4_3 = 15,
};
typedef struct AVFrameSideData {
    enum AVFrameSideDataType type;
    uint8_t *data;
    int size;
    AVDictionary *metadata;
    AVBufferRef *buf;
} AVFrameSideData;
typedef struct AVRegionOfInterest {
    uint32_t self_size;
    int top;
    int bottom;
    int left;
    int right;
    AVRational qoffset;
} AVRegionOfInterest;
typedef struct AVFrame {
#define AV_NUM_DATA_POINTERS 8
    uint8_t *data[8];
    int linesize[8];
    uint8_t **extended_data;
    int width, height;
    int nb_samples;
    int format;
    int key_frame;
    enum AVPictureType pict_type;
    AVRational sample_aspect_ratio;
    int64_t pts;
   
    int64_t pkt_pts;
    int64_t pkt_dts;
    int coded_picture_number;
    int display_picture_number;
    int quality;
    void *opaque;
   
    uint64_t error[8];
    int repeat_pict;
    int interlaced_frame;
    int top_field_first;
    int palette_has_changed;
    int64_t reordered_opaque;
    int sample_rate;
    uint64_t channel_layout;
    AVBufferRef *buf[8];
    AVBufferRef **extended_buf;
    int nb_extended_buf;
    AVFrameSideData **side_data;
    int nb_side_data;
#define AV_FRAME_FLAG_CORRUPT (1 << 0)
#define AV_FRAME_FLAG_DISCARD (1 << 2)
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
#define FF_DECODE_ERROR_INVALID_BITSTREAM 1
#define FF_DECODE_ERROR_MISSING_REFERENCE 2
#define FF_DECODE_ERROR_CONCEALMENT_ACTIVE 4
#define FF_DECODE_ERROR_DECODE_SLICES 8
    int channels;
    int pkt_size;
   
    int8_t *qscale_table;
   
    int qstride;
   
    int qscale_type;
   
    AVBufferRef *qp_table_buf;
    AVBufferRef *hw_frames_ctx;
    AVBufferRef *opaque_ref;
    size_t crop_top;
    size_t crop_bottom;
    size_t crop_left;
    size_t crop_right;
    AVBufferRef *private_ref;
} AVFrame;

int64_t av_frame_get_best_effort_timestamp(const AVFrame *frame);

void av_frame_set_best_effort_timestamp(AVFrame *frame, int64_t val);

int64_t av_frame_get_pkt_duration (const AVFrame *frame);

void av_frame_set_pkt_duration (AVFrame *frame, int64_t val);

int64_t av_frame_get_pkt_pos (const AVFrame *frame);

void av_frame_set_pkt_pos (AVFrame *frame, int64_t val);

int64_t av_frame_get_channel_layout (const AVFrame *frame);

void av_frame_set_channel_layout (AVFrame *frame, int64_t val);

int av_frame_get_channels (const AVFrame *frame);

void av_frame_set_channels (AVFrame *frame, int val);

int av_frame_get_sample_rate (const AVFrame *frame);

void av_frame_set_sample_rate (AVFrame *frame, int val);

AVDictionary *av_frame_get_metadata (const AVFrame *frame);

void av_frame_set_metadata (AVFrame *frame, AVDictionary *val);

int av_frame_get_decode_error_flags (const AVFrame *frame);

void av_frame_set_decode_error_flags (AVFrame *frame, int val);

int av_frame_get_pkt_size(const AVFrame *frame);

void av_frame_set_pkt_size(AVFrame *frame, int val);

int8_t *av_frame_get_qp_table(AVFrame *f, int *stride, int *type);

int av_frame_set_qp_table(AVFrame *f, AVBufferRef *buf, int stride, int type);

enum AVColorSpace av_frame_get_colorspace(const AVFrame *frame);

void av_frame_set_colorspace(AVFrame *frame, enum AVColorSpace val);

enum AVColorRange av_frame_get_color_range(const AVFrame *frame);

void av_frame_set_color_range(AVFrame *frame, enum AVColorRange val);

const char *av_get_colorspace_name(enum AVColorSpace val);
AVFrame *av_frame_alloc(void);
void av_frame_free(AVFrame **frame);
int av_frame_ref(AVFrame *dst, const AVFrame *src);
AVFrame *av_frame_clone(const AVFrame *src);

void av_frame_unref(AVFrame *frame);
void av_frame_move_ref(AVFrame *dst, AVFrame *src);
int av_frame_get_buffer(AVFrame *frame, int align);
int av_frame_is_writable(AVFrame *frame);
int av_frame_make_writable(AVFrame *frame);
int av_frame_copy(AVFrame *dst, const AVFrame *src);
int av_frame_copy_props(AVFrame *dst, const AVFrame *src);
AVBufferRef *av_frame_get_plane_buffer(AVFrame *frame, int plane);
AVFrameSideData *av_frame_new_side_data(AVFrame *frame,
                                        enum AVFrameSideDataType type,
                                        int size);
AVFrameSideData *av_frame_new_side_data_from_buf(AVFrame *frame,
                                                 enum AVFrameSideDataType type,
                                                 AVBufferRef *buf);

AVFrameSideData *av_frame_get_side_data(const AVFrame *frame,
                                        enum AVFrameSideDataType type);

void av_frame_remove_side_data(AVFrame *frame, enum AVFrameSideDataType type);

enum {
    AV_FRAME_CROP_UNALIGNED = 1 << 0,
};
int av_frame_apply_cropping(AVFrame *frame, int flags);

const char *av_frame_side_data_name(enum AVFrameSideDataType type);
#define AVUTIL_HWCONTEXT_H 

enum AVHWDeviceType {
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
    AV_HWDEVICE_TYPE_VULKAN,
};

typedef struct AVHWDeviceInternal AVHWDeviceInternal;
typedef struct AVHWDeviceContext {
    const AVClass *av_class;
    AVHWDeviceInternal *internal;
    enum AVHWDeviceType type;
    void *hwctx;
    void (*free)(struct AVHWDeviceContext *ctx);
    void *user_opaque;
} AVHWDeviceContext;

typedef struct AVHWFramesInternal AVHWFramesInternal;
typedef struct AVHWFramesContext {
    const AVClass *av_class;
    AVHWFramesInternal *internal;
    AVBufferRef *device_ref;
    AVHWDeviceContext *device_ctx;
    void *hwctx;
    void (*free)(struct AVHWFramesContext *ctx);
    void *user_opaque;
    AVBufferPool *pool;
    int initial_pool_size;
    enum AVPixelFormat format;
    enum AVPixelFormat sw_format;
    int width, height;
} AVHWFramesContext;
enum AVHWDeviceType av_hwdevice_find_type_by_name(const char *name);

const char *av_hwdevice_get_type_name(enum AVHWDeviceType type);
enum AVHWDeviceType av_hwdevice_iterate_types(enum AVHWDeviceType prev);
AVBufferRef *av_hwdevice_ctx_alloc(enum AVHWDeviceType type);
int av_hwdevice_ctx_init(AVBufferRef *ref);
int av_hwdevice_ctx_create(AVBufferRef **device_ctx, enum AVHWDeviceType type,
                           const char *device, AVDictionary *opts, int flags);
int av_hwdevice_ctx_create_derived(AVBufferRef **dst_ctx,
                                   enum AVHWDeviceType type,
                                   AVBufferRef *src_ctx, int flags);
int av_hwdevice_ctx_create_derived_opts(AVBufferRef **dst_ctx,
                                        enum AVHWDeviceType type,
                                        AVBufferRef *src_ctx,
                                        AVDictionary *options, int flags);
AVBufferRef *av_hwframe_ctx_alloc(AVBufferRef *device_ctx);
int av_hwframe_ctx_init(AVBufferRef *ref);
int av_hwframe_get_buffer(AVBufferRef *hwframe_ctx, AVFrame *frame, int flags);
int av_hwframe_transfer_data(AVFrame *dst, const AVFrame *src, int flags);

enum AVHWFrameTransferDirection {
    AV_HWFRAME_TRANSFER_DIRECTION_FROM,
    AV_HWFRAME_TRANSFER_DIRECTION_TO,
};
int av_hwframe_transfer_get_formats(AVBufferRef *hwframe_ctx,
                                    enum AVHWFrameTransferDirection dir,
                                    enum AVPixelFormat **formats, int flags);
typedef struct AVHWFramesConstraints {
    enum AVPixelFormat *valid_hw_formats;
    enum AVPixelFormat *valid_sw_formats;
    int min_width;
    int min_height;
    int max_width;
    int max_height;
} AVHWFramesConstraints;
void *av_hwdevice_hwconfig_alloc(AVBufferRef *device_ctx);
AVHWFramesConstraints *av_hwdevice_get_hwframe_constraints(AVBufferRef *ref,
                                                           const void *hwconfig);

void av_hwframe_constraints_free(AVHWFramesConstraints **constraints);

enum {
    AV_HWFRAME_MAP_READ = 1 << 0,
    AV_HWFRAME_MAP_WRITE = 1 << 1,
    AV_HWFRAME_MAP_OVERWRITE = 1 << 2,
    AV_HWFRAME_MAP_DIRECT = 1 << 3,
};
int av_hwframe_map(AVFrame *dst, const AVFrame *src, int flags);
int av_hwframe_ctx_create_derived(AVBufferRef **derived_frame_ctx,
                                  enum AVPixelFormat format,
                                  AVBufferRef *derived_device_ctx,
                                  AVBufferRef *source_frame_ctx,
                                  int flags);
#define AVCODEC_BSF_H 
#define AVCODEC_CODEC_ID_H 
enum AVCodecID {
    AV_CODEC_ID_NONE,
    AV_CODEC_ID_MPEG1VIDEO,
    AV_CODEC_ID_MPEG2VIDEO,
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
    AV_CODEC_ID_FIRST_AUDIO = 0x10000,
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
    AV_CODEC_ID_AMR_NB = 0x12000,
    AV_CODEC_ID_AMR_WB,
    AV_CODEC_ID_RA_144 = 0x13000,
    AV_CODEC_ID_RA_288,
    AV_CODEC_ID_ROQ_DPCM = 0x14000,
    AV_CODEC_ID_INTERPLAY_DPCM,
    AV_CODEC_ID_XAN_DPCM,
    AV_CODEC_ID_SOL_DPCM,
    AV_CODEC_ID_SDX2_DPCM = 0x14800,
    AV_CODEC_ID_GREMLIN_DPCM,
    AV_CODEC_ID_DERF_DPCM,
    AV_CODEC_ID_MP2 = 0x15000,
    AV_CODEC_ID_MP3,
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
    AV_CODEC_ID_GSM,
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
    AV_CODEC_ID_GSM_MS,
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
    AV_CODEC_ID_FIRST_SUBTITLE = 0x17000,
    AV_CODEC_ID_DVD_SUBTITLE = 0x17000,
    AV_CODEC_ID_DVB_SUBTITLE,
    AV_CODEC_ID_TEXT,
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
    AV_CODEC_ID_FIRST_UNKNOWN = 0x18000,
    AV_CODEC_ID_TTF = 0x18000,
    AV_CODEC_ID_SCTE_35,
    AV_CODEC_ID_EPG,
    AV_CODEC_ID_BINTEXT = 0x18800,
    AV_CODEC_ID_XBIN,
    AV_CODEC_ID_IDF,
    AV_CODEC_ID_OTF,
    AV_CODEC_ID_SMPTE_KLV,
    AV_CODEC_ID_DVD_NAV,
    AV_CODEC_ID_TIMED_ID3,
    AV_CODEC_ID_BIN_DATA,
    AV_CODEC_ID_PROBE = 0x19000,
    AV_CODEC_ID_MPEG2TS = 0x20000,
    AV_CODEC_ID_MPEG4SYSTEMS = 0x20001,
    AV_CODEC_ID_FFMETADATA = 0x21000,
    AV_CODEC_ID_WRAPPED_AVFRAME = 0x21001,
};

enum AVMediaType avcodec_get_type(enum AVCodecID codec_id);

const char *avcodec_get_name(enum AVCodecID id);
#define AVCODEC_CODEC_PAR_H 
enum AVFieldOrder {
    AV_FIELD_UNKNOWN,
    AV_FIELD_PROGRESSIVE,
    AV_FIELD_TT,
    AV_FIELD_BB,
    AV_FIELD_TB,
    AV_FIELD_BT,
};
typedef struct AVCodecParameters {
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

AVCodecParameters *avcodec_parameters_alloc(void);

void avcodec_parameters_free(AVCodecParameters **par);

int avcodec_parameters_copy(AVCodecParameters *dst, const AVCodecParameters *src);
#define AVCODEC_PACKET_H 
#define AVCODEC_VERSION_H 
#define LIBAVCODEC_VERSION_MAJOR 58
#define LIBAVCODEC_VERSION_MINOR 112
#define LIBAVCODEC_VERSION_MICRO 100
#define LIBAVCODEC_VERSION_INT AV_VERSION_INT(LIBAVCODEC_VERSION_MAJOR, LIBAVCODEC_VERSION_MINOR, LIBAVCODEC_VERSION_MICRO)
#define LIBAVCODEC_VERSION AV_VERSION(LIBAVCODEC_VERSION_MAJOR, LIBAVCODEC_VERSION_MINOR, LIBAVCODEC_VERSION_MICRO)
#define LIBAVCODEC_BUILD LIBAVCODEC_VERSION_INT
#define LIBAVCODEC_IDENT "Lavc" AV_STRINGIFY(LIBAVCODEC_VERSION)
#define FF_API_LOWRES (LIBAVCODEC_VERSION_MAJOR < 59)
#define FF_API_DEBUG_MV (LIBAVCODEC_VERSION_MAJOR < 58)
#define FF_API_AVCTX_TIMEBASE (LIBAVCODEC_VERSION_MAJOR < 59)
#define FF_API_CODED_FRAME (LIBAVCODEC_VERSION_MAJOR < 59)
#define FF_API_SIDEDATA_ONLY_PKT (LIBAVCODEC_VERSION_MAJOR < 59)
#define FF_API_VDPAU_PROFILE (LIBAVCODEC_VERSION_MAJOR < 59)
#define FF_API_CONVERGENCE_DURATION (LIBAVCODEC_VERSION_MAJOR < 59)
#define FF_API_AVPICTURE (LIBAVCODEC_VERSION_MAJOR < 59)
#define FF_API_AVPACKET_OLD_API (LIBAVCODEC_VERSION_MAJOR < 59)
#define FF_API_RTP_CALLBACK (LIBAVCODEC_VERSION_MAJOR < 59)
#define FF_API_VBV_DELAY (LIBAVCODEC_VERSION_MAJOR < 59)
#define FF_API_CODER_TYPE (LIBAVCODEC_VERSION_MAJOR < 59)
#define FF_API_STAT_BITS (LIBAVCODEC_VERSION_MAJOR < 59)
#define FF_API_PRIVATE_OPT (LIBAVCODEC_VERSION_MAJOR < 59)
#define FF_API_ASS_TIMING (LIBAVCODEC_VERSION_MAJOR < 59)
#define FF_API_OLD_BSF (LIBAVCODEC_VERSION_MAJOR < 59)
#define FF_API_COPY_CONTEXT (LIBAVCODEC_VERSION_MAJOR < 59)
#define FF_API_GET_CONTEXT_DEFAULTS (LIBAVCODEC_VERSION_MAJOR < 59)
#define FF_API_NVENC_OLD_NAME (LIBAVCODEC_VERSION_MAJOR < 59)
#define FF_API_STRUCT_VAAPI_CONTEXT (LIBAVCODEC_VERSION_MAJOR < 59)
#define FF_API_MERGE_SD_API (LIBAVCODEC_VERSION_MAJOR < 59)
#define FF_API_TAG_STRING (LIBAVCODEC_VERSION_MAJOR < 59)
#define FF_API_GETCHROMA (LIBAVCODEC_VERSION_MAJOR < 59)
#define FF_API_CODEC_GET_SET (LIBAVCODEC_VERSION_MAJOR < 59)
#define FF_API_USER_VISIBLE_AVHWACCEL (LIBAVCODEC_VERSION_MAJOR < 59)
#define FF_API_LOCKMGR (LIBAVCODEC_VERSION_MAJOR < 59)
#define FF_API_NEXT (LIBAVCODEC_VERSION_MAJOR < 59)
#define FF_API_UNSANITIZED_BITRATES (LIBAVCODEC_VERSION_MAJOR < 59)
#define FF_API_OPENH264_SLICE_MODE (LIBAVCODEC_VERSION_MAJOR < 59)
#define FF_API_OPENH264_CABAC (LIBAVCODEC_VERSION_MAJOR < 59)
#define FF_API_UNUSED_CODEC_CAPS (LIBAVCODEC_VERSION_MAJOR < 59)

enum AVPacketSideDataType {
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
    AV_PKT_DATA_STRINGS_METADATA,
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
    AV_PKT_DATA_PRFT,
    AV_PKT_DATA_ICC_PROFILE,
    AV_PKT_DATA_DOVI_CONF,
    AV_PKT_DATA_S12M_TIMECODE,
    AV_PKT_DATA_NB
};
#define AV_PKT_DATA_QUALITY_FACTOR AV_PKT_DATA_QUALITY_STATS

typedef struct AVPacketSideData {
    uint8_t *data;
    int size;
    enum AVPacketSideDataType type;
} AVPacketSideData;
typedef struct AVPacket {
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
    int64_t pos;
   
    int64_t convergence_duration;
} AVPacket;

typedef struct AVPacketList {
    AVPacket pkt;
    struct AVPacketList *next;
} AVPacketList;
#define AV_PKT_FLAG_KEY 0x0001
#define AV_PKT_FLAG_CORRUPT 0x0002
#define AV_PKT_FLAG_DISCARD 0x0004
#define AV_PKT_FLAG_TRUSTED 0x0008
#define AV_PKT_FLAG_DISPOSABLE 0x0010

enum AVSideDataParamChangeFlags {
    AV_SIDE_DATA_PARAM_CHANGE_CHANNEL_COUNT = 0x0001,
    AV_SIDE_DATA_PARAM_CHANGE_CHANNEL_LAYOUT = 0x0002,
    AV_SIDE_DATA_PARAM_CHANGE_SAMPLE_RATE = 0x0004,
    AV_SIDE_DATA_PARAM_CHANGE_DIMENSIONS = 0x0008,
};
AVPacket *av_packet_alloc(void);
AVPacket *av_packet_clone(const AVPacket *src);
void av_packet_free(AVPacket **pkt);
void av_init_packet(AVPacket *pkt);
int av_new_packet(AVPacket *pkt, int size);

void av_shrink_packet(AVPacket *pkt, int size);

int av_grow_packet(AVPacket *pkt, int grow_by);
int av_packet_from_data(AVPacket *pkt, uint8_t *data, int size);

int av_dup_packet(AVPacket *pkt);

int av_copy_packet(AVPacket *dst, const AVPacket *src);

int av_copy_packet_side_data(AVPacket *dst, const AVPacket *src);

void av_free_packet(AVPacket *pkt);
uint8_t* av_packet_new_side_data(AVPacket *pkt, enum AVPacketSideDataType type,
                                 int size);
int av_packet_add_side_data(AVPacket *pkt, enum AVPacketSideDataType type,
                            uint8_t *data, size_t size);
int av_packet_shrink_side_data(AVPacket *pkt, enum AVPacketSideDataType type,
                               int size);
uint8_t* av_packet_get_side_data(const AVPacket *pkt, enum AVPacketSideDataType type,
                                 int *size);

int av_packet_merge_side_data(AVPacket *pkt);

int av_packet_split_side_data(AVPacket *pkt);

const char *av_packet_side_data_name(enum AVPacketSideDataType type);
uint8_t *av_packet_pack_dictionary(AVDictionary *dict, int *size);
int av_packet_unpack_dictionary(const uint8_t *data, int size, AVDictionary **dict);
void av_packet_free_side_data(AVPacket *pkt);
int av_packet_ref(AVPacket *dst, const AVPacket *src);
void av_packet_unref(AVPacket *pkt);
void av_packet_move_ref(AVPacket *dst, AVPacket *src);
int av_packet_copy_props(AVPacket *dst, const AVPacket *src);
int av_packet_make_refcounted(AVPacket *pkt);
int av_packet_make_writable(AVPacket *pkt);
void av_packet_rescale_ts(AVPacket *pkt, AVRational tb_src, AVRational tb_dst);

typedef struct AVBSFInternal AVBSFInternal;
typedef struct AVBSFContext {
    const AVClass *av_class;
    const struct AVBitStreamFilter *filter;
    AVBSFInternal *internal;
    void *priv_data;
    AVCodecParameters *par_in;
    AVCodecParameters *par_out;
    AVRational time_base_in;
    AVRational time_base_out;
} AVBSFContext;

typedef struct AVBitStreamFilter {
    const char *name;
    const enum AVCodecID *codec_ids;
    const AVClass *priv_class;
    int priv_data_size;
    int (*init)(AVBSFContext *ctx);
    int (*filter)(AVBSFContext *ctx, AVPacket *pkt);
    void (*close)(AVBSFContext *ctx);
    void (*flush)(AVBSFContext *ctx);
} AVBitStreamFilter;

const AVBitStreamFilter *av_bsf_get_by_name(const char *name);
const AVBitStreamFilter *av_bsf_iterate(void **opaque);
int av_bsf_alloc(const AVBitStreamFilter *filter, AVBSFContext **ctx);

int av_bsf_init(AVBSFContext *ctx);
int av_bsf_send_packet(AVBSFContext *ctx, AVPacket *pkt);
int av_bsf_receive_packet(AVBSFContext *ctx, AVPacket *pkt);

void av_bsf_flush(AVBSFContext *ctx);

void av_bsf_free(AVBSFContext **ctx);

const AVClass *av_bsf_get_class(void);

typedef struct AVBSFList AVBSFList;
AVBSFList *av_bsf_list_alloc(void);

void av_bsf_list_free(AVBSFList **lst);
int av_bsf_list_append(AVBSFList *lst, AVBSFContext *bsf);
int av_bsf_list_append2(AVBSFList *lst, const char * bsf_name, AVDictionary **options);
int av_bsf_list_finalize(AVBSFList **lst, AVBSFContext **bsf);
int av_bsf_list_parse_str(const char *str, AVBSFContext **bsf);
int av_bsf_get_null_filter(AVBSFContext **bsf);
#define AVCODEC_CODEC_H 
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
#define AV_CODEC_CAP_INTRA_ONLY 0x40000000
#define AV_CODEC_CAP_LOSSLESS 0x80000000
#define AV_CODEC_CAP_HARDWARE (1 << 18)
#define AV_CODEC_CAP_HYBRID (1 << 19)
#define AV_CODEC_CAP_ENCODER_REORDERED_OPAQUE (1 << 20)
#define AV_CODEC_CAP_ENCODER_FLUSH (1 << 21)

typedef struct AVProfile {
    int profile;
    const char *name;
} AVProfile;

typedef struct AVCodecDefault AVCodecDefault;

struct AVCodecContext;
struct AVSubtitle;
struct AVPacket;

typedef struct AVCodec {
    const char *name;
    const char *long_name;
    enum AVMediaType type;
    enum AVCodecID id;
    int capabilities;
    const AVRational *supported_framerates;
    const enum AVPixelFormat *pix_fmts;
    const int *supported_samplerates;
    const enum AVSampleFormat *sample_fmts;
    const uint64_t *channel_layouts;
    uint8_t max_lowres;
    const AVClass *priv_class;
    const AVProfile *profiles;
    const char *wrapper_name;
    int priv_data_size;
    struct AVCodec *next;
    int (*update_thread_context)(struct AVCodecContext *dst, const struct AVCodecContext *src);
    const AVCodecDefault *defaults;
    void (*init_static_data)(struct AVCodec *codec);
    int (*init)(struct AVCodecContext *);
    int (*encode_sub)(struct AVCodecContext *, uint8_t *buf, int buf_size,
                      const struct AVSubtitle *sub);
    int (*encode2)(struct AVCodecContext *avctx, struct AVPacket *avpkt,
                   const struct AVFrame *frame, int *got_packet_ptr);
    int (*decode)(struct AVCodecContext *, void *outdata, int *outdata_size, struct AVPacket *avpkt);
    int (*close)(struct AVCodecContext *);
    int (*receive_packet)(struct AVCodecContext *avctx, struct AVPacket *avpkt);
    int (*receive_frame)(struct AVCodecContext *avctx, struct AVFrame *frame);
    void (*flush)(struct AVCodecContext *);
    int caps_internal;
    const char *bsfs;
    const struct AVCodecHWConfigInternal **hw_configs;
    const uint32_t *codec_tags;
} AVCodec;
const AVCodec *av_codec_iterate(void **opaque);

AVCodec *avcodec_find_decoder(enum AVCodecID id);

AVCodec *avcodec_find_decoder_by_name(const char *name);

AVCodec *avcodec_find_encoder(enum AVCodecID id);

AVCodec *avcodec_find_encoder_by_name(const char *name);

int av_codec_is_encoder(const AVCodec *codec);

int av_codec_is_decoder(const AVCodec *codec);

enum {
    AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX = 0x01,
    AV_CODEC_HW_CONFIG_METHOD_HW_FRAMES_CTX = 0x02,
    AV_CODEC_HW_CONFIG_METHOD_INTERNAL = 0x04,
    AV_CODEC_HW_CONFIG_METHOD_AD_HOC = 0x08,
};

typedef struct AVCodecHWConfig {
    enum AVPixelFormat pix_fmt;
    int methods;
    enum AVHWDeviceType device_type;
} AVCodecHWConfig;
const AVCodecHWConfig *avcodec_get_hw_config(const AVCodec *codec, int index);
#define AVCODEC_CODEC_DESC_H 
typedef struct AVCodecDescriptor {
    enum AVCodecID id;
    enum AVMediaType type;
    const char *name;
    const char *long_name;
    int props;
    const char *const *mime_types;
    const struct AVProfile *profiles;
} AVCodecDescriptor;
#define AV_CODEC_PROP_INTRA_ONLY (1 << 0)
#define AV_CODEC_PROP_LOSSY (1 << 1)
#define AV_CODEC_PROP_LOSSLESS (1 << 2)
#define AV_CODEC_PROP_REORDER (1 << 3)
#define AV_CODEC_PROP_BITMAP_SUB (1 << 16)
#define AV_CODEC_PROP_TEXT_SUB (1 << 17)

const AVCodecDescriptor *avcodec_descriptor_get(enum AVCodecID id);
const AVCodecDescriptor *avcodec_descriptor_next(const AVCodecDescriptor *prev);

const AVCodecDescriptor *avcodec_descriptor_get_by_name(const char *name);
#define AV_INPUT_BUFFER_PADDING_SIZE 64
#define AV_INPUT_BUFFER_MIN_SIZE 16384

enum AVDiscard{
    AVDISCARD_NONE =-16,
    AVDISCARD_DEFAULT = 0,
    AVDISCARD_NONREF = 8,
    AVDISCARD_BIDIR = 16,
    AVDISCARD_NONINTRA= 24,
    AVDISCARD_NONKEY = 32,
    AVDISCARD_ALL = 48,
};

enum AVAudioServiceType {
    AV_AUDIO_SERVICE_TYPE_MAIN = 0,
    AV_AUDIO_SERVICE_TYPE_EFFECTS = 1,
    AV_AUDIO_SERVICE_TYPE_VISUALLY_IMPAIRED = 2,
    AV_AUDIO_SERVICE_TYPE_HEARING_IMPAIRED = 3,
    AV_AUDIO_SERVICE_TYPE_DIALOGUE = 4,
    AV_AUDIO_SERVICE_TYPE_COMMENTARY = 5,
    AV_AUDIO_SERVICE_TYPE_EMERGENCY = 6,
    AV_AUDIO_SERVICE_TYPE_VOICE_OVER = 7,
    AV_AUDIO_SERVICE_TYPE_KARAOKE = 8,
    AV_AUDIO_SERVICE_TYPE_NB ,
};

typedef struct RcOverride{
    int start_frame;
    int end_frame;
    int qscale;
    float quality_factor;
} RcOverride;
#define AV_CODEC_FLAG_UNALIGNED (1 << 0)
#define AV_CODEC_FLAG_QSCALE (1 << 1)
#define AV_CODEC_FLAG_4MV (1 << 2)
#define AV_CODEC_FLAG_OUTPUT_CORRUPT (1 << 3)
#define AV_CODEC_FLAG_QPEL (1 << 4)
#define AV_CODEC_FLAG_DROPCHANGED (1 << 5)
#define AV_CODEC_FLAG_PASS1 (1 << 9)
#define AV_CODEC_FLAG_PASS2 (1 << 10)
#define AV_CODEC_FLAG_LOOP_FILTER (1 << 11)
#define AV_CODEC_FLAG_GRAY (1 << 13)
#define AV_CODEC_FLAG_PSNR (1 << 15)
#define AV_CODEC_FLAG_TRUNCATED (1 << 16)
#define AV_CODEC_FLAG_INTERLACED_DCT (1 << 18)
#define AV_CODEC_FLAG_LOW_DELAY (1 << 19)
#define AV_CODEC_FLAG_GLOBAL_HEADER (1 << 22)
#define AV_CODEC_FLAG_BITEXACT (1 << 23)
#define AV_CODEC_FLAG_AC_PRED (1 << 24)
#define AV_CODEC_FLAG_INTERLACED_ME (1 << 29)
#define AV_CODEC_FLAG_CLOSED_GOP (1U << 31)
#define AV_CODEC_FLAG2_FAST (1 << 0)
#define AV_CODEC_FLAG2_NO_OUTPUT (1 << 2)
#define AV_CODEC_FLAG2_LOCAL_HEADER (1 << 3)
#define AV_CODEC_FLAG2_DROP_FRAME_TIMECODE (1 << 13)
#define AV_CODEC_FLAG2_CHUNKS (1 << 15)
#define AV_CODEC_FLAG2_IGNORE_CROP (1 << 16)
#define AV_CODEC_FLAG2_SHOW_ALL (1 << 22)
#define AV_CODEC_FLAG2_EXPORT_MVS (1 << 28)
#define AV_CODEC_FLAG2_SKIP_MANUAL (1 << 29)
#define AV_CODEC_FLAG2_RO_FLUSH_NOOP (1 << 30)
#define AV_CODEC_EXPORT_DATA_MVS (1 << 0)
#define AV_CODEC_EXPORT_DATA_PRFT (1 << 1)
#define AV_CODEC_EXPORT_DATA_VIDEO_ENC_PARAMS (1 << 2)

typedef struct AVPanScan {
    int id;
    int width;
    int height;
    int16_t position[3][2];
} AVPanScan;

typedef struct AVCPBProperties {
    int max_bitrate;
    int min_bitrate;
    int avg_bitrate;
    int buffer_size;
    uint64_t vbv_delay;
} AVCPBProperties;

typedef struct AVProducerReferenceTime {
    int64_t wallclock;
    int flags;
} AVProducerReferenceTime;
#define AV_GET_BUFFER_FLAG_REF (1 << 0)

struct AVCodecInternal;
typedef struct AVCodecContext {
    const AVClass *av_class;
    int log_level_offset;
    enum AVMediaType codec_type;
    const struct AVCodec *codec;
    enum AVCodecID codec_id;
    unsigned int codec_tag;
    void *priv_data;
    struct AVCodecInternal *internal;
    void *opaque;
    int64_t bit_rate;
    int bit_rate_tolerance;
    int global_quality;
    int compression_level;
#define FF_COMPRESSION_DEFAULT -1
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
                            const AVFrame *src, int offset[8],
                            int y, int type, int height);
    enum AVPixelFormat (*get_format)(struct AVCodecContext *s, const enum AVPixelFormat * fmt);
    int max_b_frames;
    float b_quant_factor;
   
    int b_frame_strategy;
    float b_quant_offset;
    int has_b_frames;
   
    int mpeg_quant;
    float i_quant_factor;
    float i_quant_offset;
    float lumi_masking;
    float temporal_cplx_masking;
    float spatial_cplx_masking;
    float p_masking;
    float dark_masking;
    int slice_count;
   
     int prediction_method;
#define FF_PRED_LEFT 0
#define FF_PRED_PLANE 1
#define FF_PRED_MEDIAN 2
    int *slice_offset;
    AVRational sample_aspect_ratio;
    int me_cmp;
    int me_sub_cmp;
    int mb_cmp;
    int ildct_cmp;
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
    int dia_size;
    int last_predictor_count;
   
    int pre_me;
    int me_pre_cmp;
    int pre_dia_size;
    int me_subpel_quality;
    int me_range;
    int slice_flags;
#define SLICE_FLAG_CODED_ORDER 0x0001
#define SLICE_FLAG_ALLOW_FIELD 0x0002
#define SLICE_FLAG_ALLOW_PLANE 0x0004
    int mb_decision;
#define FF_MB_DECISION_SIMPLE 0
#define FF_MB_DECISION_BITS 1
#define FF_MB_DECISION_RD 2
    uint16_t *intra_matrix;
    uint16_t *inter_matrix;
   
    int scenechange_threshold;
   
    int noise_reduction;
    int intra_dc_precision;
    int skip_top;
    int skip_bottom;
    int mb_lmin;
    int mb_lmax;
   
    int me_penalty_compensation;
    int bidir_refine;
   
    int brd_scale;
    int keyint_min;
    int refs;
   
    int chromaoffset;
    int mv0_threshold;
   
    int b_sensitivity;
    enum AVColorPrimaries color_primaries;
    enum AVColorTransferCharacteristic color_trc;
    enum AVColorSpace colorspace;
    enum AVColorRange color_range;
    enum AVChromaLocation chroma_sample_location;
    int slices;
    enum AVFieldOrder field_order;
    int sample_rate;
    int channels;
    enum AVSampleFormat sample_fmt;
    int frame_size;
    int frame_number;
    int block_align;
    int cutoff;
    uint64_t channel_layout;
    uint64_t request_channel_layout;
    enum AVAudioServiceType audio_service_type;
    enum AVSampleFormat request_sample_fmt;
    int (*get_buffer2)(struct AVCodecContext *s, AVFrame *frame, int flags);
   
    int refcounted_frames;
    float qcompress;
    float qblur;
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
#define FF_CODER_TYPE_VLC 0
#define FF_CODER_TYPE_AC 1
#define FF_CODER_TYPE_RAW 2
#define FF_CODER_TYPE_RLE 3
   
    int coder_type;
   
    int context_model;
   
    int frame_skip_threshold;
   
    int frame_skip_factor;
   
    int frame_skip_exp;
   
    int frame_skip_cmp;
    int trellis;
   
    int min_prediction_order;
   
    int max_prediction_order;
   
    int64_t timecode_frame_start;
   
    void (*rtp_callback)(struct AVCodecContext *avctx, void *data, int size, int mb_nb);
   
    int rtp_payload_size;
   
    int mv_bits;
   
    int header_bits;
   
    int i_tex_bits;
   
    int p_tex_bits;
   
    int i_count;
   
    int p_count;
   
    int skip_count;
   
    int misc_bits;
   
    int frame_bits;
    char *stats_out;
    char *stats_in;
    int workaround_bugs;
#define FF_BUG_AUTODETECT 1
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
#define FF_BUG_MS 8192
#define FF_BUG_TRUNCATED 16384
#define FF_BUG_IEDGE 32768
    int strict_std_compliance;
#define FF_COMPLIANCE_VERY_STRICT 2
#define FF_COMPLIANCE_STRICT 1
#define FF_COMPLIANCE_NORMAL 0
#define FF_COMPLIANCE_UNOFFICIAL -1
#define FF_COMPLIANCE_EXPERIMENTAL -2
    int error_concealment;
#define FF_EC_GUESS_MVS 1
#define FF_EC_DEBLOCK 2
#define FF_EC_FAVOR_INTER 256
    int debug;
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
    int err_recognition;
#define AV_EF_CRCCHECK (1<<0)
#define AV_EF_BITSTREAM (1<<1)
#define AV_EF_BUFFER (1<<2)
#define AV_EF_EXPLODE (1<<3)
#define AV_EF_IGNORE_ERR (1<<15)
#define AV_EF_CAREFUL (1<<16)
#define AV_EF_COMPLIANT (1<<17)
#define AV_EF_AGGRESSIVE (1<<18)
    int64_t reordered_opaque;
    const struct AVHWAccel *hwaccel;
    void *hwaccel_context;
    uint64_t error[8];
    int dct_algo;
#define FF_DCT_AUTO 0
#define FF_DCT_FASTINT 1
#define FF_DCT_INT 2
#define FF_DCT_MMX 3
#define FF_DCT_ALTIVEC 5
#define FF_DCT_FAAN 6
    int idct_algo;
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
#define FF_IDCT_NONE 24
#define FF_IDCT_SIMPLEAUTO 128
     int bits_per_coded_sample;
    int bits_per_raw_sample;
     int lowres;
    AVFrame *coded_frame;
    int thread_count;
    int thread_type;
#define FF_THREAD_FRAME 1
#define FF_THREAD_SLICE 2
    int active_thread_type;
    int thread_safe_callbacks;
    int (*execute)(struct AVCodecContext *c, int (*func)(struct AVCodecContext *c2, void *arg), void *arg2, int *ret, int count, int size);
    int (*execute2)(struct AVCodecContext *c, int (*func)(struct AVCodecContext *c2, void *arg, int jobnr, int threadnr), void *arg2, int *ret, int count);
     int nsse_weight;
     int profile;
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
#define FF_PROFILE_H264_CONSTRAINED (1<<9)
#define FF_PROFILE_H264_INTRA (1<<11)
#define FF_PROFILE_H264_BASELINE 66
#define FF_PROFILE_H264_CONSTRAINED_BASELINE (66|FF_PROFILE_H264_CONSTRAINED)
#define FF_PROFILE_H264_MAIN 77
#define FF_PROFILE_H264_EXTENDED 88
#define FF_PROFILE_H264_HIGH 100
#define FF_PROFILE_H264_HIGH_10 110
#define FF_PROFILE_H264_HIGH_10_INTRA (110|FF_PROFILE_H264_INTRA)
#define FF_PROFILE_H264_MULTIVIEW_HIGH 118
#define FF_PROFILE_H264_HIGH_422 122
#define FF_PROFILE_H264_HIGH_422_INTRA (122|FF_PROFILE_H264_INTRA)
#define FF_PROFILE_H264_STEREO_HIGH 128
#define FF_PROFILE_H264_HIGH_444 144
#define FF_PROFILE_H264_HIGH_444_PREDICTIVE 244
#define FF_PROFILE_H264_HIGH_444_INTRA (244|FF_PROFILE_H264_INTRA)
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
#define FF_PROFILE_KLVA_SYNC 0
#define FF_PROFILE_KLVA_ASYNC 1
     int level;
#define FF_LEVEL_UNKNOWN -99
    enum AVDiscard skip_loop_filter;
    enum AVDiscard skip_idct;
    enum AVDiscard skip_frame;
    uint8_t *subtitle_header;
    int subtitle_header_size;
   
    uint64_t vbv_delay;
   
    int side_data_only_packets;
    int initial_padding;
    AVRational framerate;
    enum AVPixelFormat sw_pix_fmt;
    AVRational pkt_timebase;
    const AVCodecDescriptor *codec_descriptor;
    int64_t pts_correction_num_faulty_pts;
    int64_t pts_correction_num_faulty_dts;
    int64_t pts_correction_last_pts;
    int64_t pts_correction_last_dts;
    char *sub_charenc;
    int sub_charenc_mode;
#define FF_SUB_CHARENC_MODE_DO_NOTHING -1
#define FF_SUB_CHARENC_MODE_AUTOMATIC 0
#define FF_SUB_CHARENC_MODE_PRE_DECODER 1
#define FF_SUB_CHARENC_MODE_IGNORE 2
    int skip_alpha;
    int seek_preroll;
    int debug_mv;
#define FF_DEBUG_VIS_MV_P_FOR 0x00000001
#define FF_DEBUG_VIS_MV_B_FOR 0x00000002
#define FF_DEBUG_VIS_MV_B_BACK 0x00000004
    uint16_t *chroma_intra_matrix;
    uint8_t *dump_separator;
    char *codec_whitelist;
    unsigned properties;
#define FF_CODEC_PROPERTY_LOSSLESS 0x00000001
#define FF_CODEC_PROPERTY_CLOSED_CAPTIONS 0x00000002
    AVPacketSideData *coded_side_data;
    int nb_coded_side_data;
    AVBufferRef *hw_frames_ctx;
    int sub_text_format;
#define FF_SUB_TEXT_FMT_ASS 0
#define FF_SUB_TEXT_FMT_ASS_WITH_TIMINGS 1
    int trailing_padding;
    int64_t max_pixels;
    AVBufferRef *hw_device_ctx;
    int hwaccel_flags;
    int apply_cropping;
    int extra_hw_frames;
    int discard_damaged_percentage;
    int64_t max_samples;
    int export_side_data;
} AVCodecContext;

AVRational av_codec_get_pkt_timebase (const AVCodecContext *avctx);

void av_codec_set_pkt_timebase (AVCodecContext *avctx, AVRational val);

const AVCodecDescriptor *av_codec_get_codec_descriptor(const AVCodecContext *avctx);

void av_codec_set_codec_descriptor(AVCodecContext *avctx, const AVCodecDescriptor *desc);

unsigned av_codec_get_codec_properties(const AVCodecContext *avctx);

int av_codec_get_lowres(const AVCodecContext *avctx);

void av_codec_set_lowres(AVCodecContext *avctx, int val);

int av_codec_get_seek_preroll(const AVCodecContext *avctx);

void av_codec_set_seek_preroll(AVCodecContext *avctx, int val);

uint16_t *av_codec_get_chroma_intra_matrix(const AVCodecContext *avctx);

void av_codec_set_chroma_intra_matrix(AVCodecContext *avctx, uint16_t *val);

struct AVSubtitle;

int av_codec_get_max_lowres(const AVCodec *codec);

struct MpegEncContext;
typedef struct AVHWAccel {
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
#define AV_HWACCEL_CODEC_CAP_EXPERIMENTAL 0x0200
#define AV_HWACCEL_FLAG_IGNORE_LEVEL (1 << 0)
#define AV_HWACCEL_FLAG_ALLOW_HIGH_DEPTH (1 << 1)
#define AV_HWACCEL_FLAG_ALLOW_PROFILE_MISMATCH (1 << 2)
typedef struct AVPicture {
   
    uint8_t *data[8];
   
    int linesize[8];
} AVPicture;

enum AVSubtitleType {
    SUBTITLE_NONE,
    SUBTITLE_BITMAP,
    SUBTITLE_TEXT,
    SUBTITLE_ASS,
};
#define AV_SUBTITLE_FLAG_FORCED 0x00000001

typedef struct AVSubtitleRect {
    int x;
    int y;
    int w;
    int h;
    int nb_colors;
   
    AVPicture pict;
    uint8_t *data[4];
    int linesize[4];
    enum AVSubtitleType type;
    char *text;
    char *ass;
    int flags;
} AVSubtitleRect;

typedef struct AVSubtitle {
    uint16_t format;
    uint32_t start_display_time;
    uint32_t end_display_time;
    unsigned num_rects;
    AVSubtitleRect **rects;
    int64_t pts;
} AVSubtitle;

AVCodec *av_codec_next(const AVCodec *c);

unsigned avcodec_version(void);

const char *avcodec_configuration(void);

const char *avcodec_license(void);

void avcodec_register(AVCodec *codec);

void avcodec_register_all(void);
AVCodecContext *avcodec_alloc_context3(const AVCodec *codec);

void avcodec_free_context(AVCodecContext **avctx);

int avcodec_get_context_defaults3(AVCodecContext *s, const AVCodec *codec);
const AVClass *avcodec_get_class(void);
const AVClass *avcodec_get_frame_class(void);

const AVClass *avcodec_get_subtitle_rect_class(void);

int avcodec_copy_context(AVCodecContext *dest, const AVCodecContext *src);
int avcodec_parameters_from_context(AVCodecParameters *par,
                                    const AVCodecContext *codec);
int avcodec_parameters_to_context(AVCodecContext *codec,
                                  const AVCodecParameters *par);
int avcodec_open2(AVCodecContext *avctx, const AVCodec *codec, AVDictionary **options);
int avcodec_close(AVCodecContext *avctx);

void avsubtitle_free(AVSubtitle *sub);
int avcodec_default_get_buffer2(AVCodecContext *s, AVFrame *frame, int flags);
void avcodec_align_dimensions(AVCodecContext *s, int *width, int *height);
void avcodec_align_dimensions2(AVCodecContext *s, int *width, int *height,
                               int linesize_align[8]);
int avcodec_enum_to_chroma_pos(int *xpos, int *ypos, enum AVChromaLocation pos);
enum AVChromaLocation avcodec_chroma_pos_to_enum(int xpos, int ypos);

int avcodec_decode_audio4(AVCodecContext *avctx, AVFrame *frame,
                          int *got_frame_ptr, const AVPacket *avpkt);

int avcodec_decode_video2(AVCodecContext *avctx, AVFrame *picture,
                         int *got_picture_ptr,
                         const AVPacket *avpkt);
int avcodec_decode_subtitle2(AVCodecContext *avctx, AVSubtitle *sub,
                            int *got_sub_ptr,
                            AVPacket *avpkt);
int avcodec_send_packet(AVCodecContext *avctx, const AVPacket *avpkt);
int avcodec_receive_frame(AVCodecContext *avctx, AVFrame *frame);
int avcodec_send_frame(AVCodecContext *avctx, const AVFrame *frame);
int avcodec_receive_packet(AVCodecContext *avctx, AVPacket *avpkt);
int avcodec_get_hw_frames_parameters(AVCodecContext *avctx,
                                     AVBufferRef *device_ref,
                                     enum AVPixelFormat hw_pix_fmt,
                                     AVBufferRef **out_frames_ref);
enum AVPictureStructure {
    AV_PICTURE_STRUCTURE_UNKNOWN,
    AV_PICTURE_STRUCTURE_TOP_FIELD,
    AV_PICTURE_STRUCTURE_BOTTOM_FIELD,
    AV_PICTURE_STRUCTURE_FRAME,
};

typedef struct AVCodecParserContext {
    void *priv_data;
    struct AVCodecParser *parser;
    int64_t frame_offset;
    int64_t cur_offset;
    int64_t next_frame_offset;
    int pict_type;
    int repeat_pict;
    int64_t pts;
    int64_t dts;
    int64_t last_pts;
    int64_t last_dts;
    int fetch_timestamp;
#define AV_PARSER_PTS_NB 4
    int cur_frame_start_index;
    int64_t cur_frame_offset[4];
    int64_t cur_frame_pts[4];
    int64_t cur_frame_dts[4];
    int flags;
#define PARSER_FLAG_COMPLETE_FRAMES 0x0001
#define PARSER_FLAG_ONCE 0x0002
#define PARSER_FLAG_FETCHED_OFFSET 0x0004
#define PARSER_FLAG_USE_CODEC_TS 0x1000
    int64_t offset;
    int64_t cur_frame_end[4];
    int key_frame;
   
    int64_t convergence_duration;
    int dts_sync_point;
    int dts_ref_dts_delta;
    int pts_dts_delta;
    int64_t cur_frame_pos[4];
    int64_t pos;
    int64_t last_pos;
    int duration;
    enum AVFieldOrder field_order;
    enum AVPictureStructure picture_structure;
    int output_picture_number;
    int width;
    int height;
    int coded_width;
    int coded_height;
    int format;
} AVCodecParserContext;

typedef struct AVCodecParser {
    int codec_ids[5];
    int priv_data_size;
    int (*parser_init)(AVCodecParserContext *s);
    int (*parser_parse)(AVCodecParserContext *s,
                        AVCodecContext *avctx,
                        const uint8_t **poutbuf, int *poutbuf_size,
                        const uint8_t *buf, int buf_size);
    void (*parser_close)(AVCodecParserContext *s);
    int (*split)(AVCodecContext *avctx, const uint8_t *buf, int buf_size);
    struct AVCodecParser *next;
} AVCodecParser;
const AVCodecParser *av_parser_iterate(void **opaque);

AVCodecParser *av_parser_next(const AVCodecParser *c);

void av_register_codec_parser(AVCodecParser *parser);

AVCodecParserContext *av_parser_init(int codec_id);
int av_parser_parse2(AVCodecParserContext *s,
                     AVCodecContext *avctx,
                     uint8_t **poutbuf, int *poutbuf_size,
                     const uint8_t *buf, int buf_size,
                     int64_t pts, int64_t dts,
                     int64_t pos);

int av_parser_change(AVCodecParserContext *s,
                     AVCodecContext *avctx,
                     uint8_t **poutbuf, int *poutbuf_size,
                     const uint8_t *buf, int buf_size, int keyframe);
void av_parser_close(AVCodecParserContext *s);

int avcodec_encode_audio2(AVCodecContext *avctx, AVPacket *avpkt,
                          const AVFrame *frame, int *got_packet_ptr);

int avcodec_encode_video2(AVCodecContext *avctx, AVPacket *avpkt,
                          const AVFrame *frame, int *got_packet_ptr);

int avcodec_encode_subtitle(AVCodecContext *avctx, uint8_t *buf, int buf_size,
                            const AVSubtitle *sub);

int avpicture_alloc(AVPicture *picture, enum AVPixelFormat pix_fmt, int width, int height);

void avpicture_free(AVPicture *picture);

int avpicture_fill(AVPicture *picture, const uint8_t *ptr,
                   enum AVPixelFormat pix_fmt, int width, int height);

int avpicture_layout(const AVPicture *src, enum AVPixelFormat pix_fmt,
                     int width, int height,
                     unsigned char *dest, int dest_size);

int avpicture_get_size(enum AVPixelFormat pix_fmt, int width, int height);

void av_picture_copy(AVPicture *dst, const AVPicture *src,
                     enum AVPixelFormat pix_fmt, int width, int height);

int av_picture_crop(AVPicture *dst, const AVPicture *src,
                    enum AVPixelFormat pix_fmt, int top_band, int left_band);

int av_picture_pad(AVPicture *dst, const AVPicture *src, int height, int width, enum AVPixelFormat pix_fmt,
            int padtop, int padbottom, int padleft, int padright, int *color);

void avcodec_get_chroma_sub_sample(enum AVPixelFormat pix_fmt, int *h_shift, int *v_shift);

unsigned int avcodec_pix_fmt_to_codec_tag(enum AVPixelFormat pix_fmt);

int avcodec_get_pix_fmt_loss(enum AVPixelFormat dst_pix_fmt, enum AVPixelFormat src_pix_fmt,
                             int has_alpha);
enum AVPixelFormat avcodec_find_best_pix_fmt_of_list(const enum AVPixelFormat *pix_fmt_list,
                                            enum AVPixelFormat src_pix_fmt,
                                            int has_alpha, int *loss_ptr);

enum AVPixelFormat avcodec_find_best_pix_fmt_of_2(enum AVPixelFormat dst_pix_fmt1, enum AVPixelFormat dst_pix_fmt2,
                                            enum AVPixelFormat src_pix_fmt, int has_alpha, int *loss_ptr);

enum AVPixelFormat avcodec_find_best_pix_fmt2(enum AVPixelFormat dst_pix_fmt1, enum AVPixelFormat dst_pix_fmt2,
                                            enum AVPixelFormat src_pix_fmt, int has_alpha, int *loss_ptr);

enum AVPixelFormat avcodec_default_get_format(struct AVCodecContext *s, const enum AVPixelFormat * fmt);

size_t av_get_codec_tag_string(char *buf, size_t buf_size, unsigned int codec_tag);

void avcodec_string(char *buf, int buf_size, AVCodecContext *enc, int encode);
const char *av_get_profile_name(const AVCodec *codec, int profile);
const char *avcodec_profile_name(enum AVCodecID codec_id, int profile);

int avcodec_default_execute(AVCodecContext *c, int (*func)(AVCodecContext *c2, void *arg2),void *arg, int *ret, int count, int size);
int avcodec_default_execute2(AVCodecContext *c, int (*func)(AVCodecContext *c2, void *arg2, int, int),void *arg, int *ret, int count);
int avcodec_fill_audio_frame(AVFrame *frame, int nb_channels,
                             enum AVSampleFormat sample_fmt, const uint8_t *buf,
                             int buf_size, int align);
void avcodec_flush_buffers(AVCodecContext *avctx);

int av_get_bits_per_sample(enum AVCodecID codec_id);

enum AVCodecID av_get_pcm_codec(enum AVSampleFormat fmt, int be);
int av_get_exact_bits_per_sample(enum AVCodecID codec_id);
int av_get_audio_frame_duration(AVCodecContext *avctx, int frame_bytes);

int av_get_audio_frame_duration2(AVCodecParameters *par, int frame_bytes);

typedef struct AVBitStreamFilterContext {
    void *priv_data;
    const struct AVBitStreamFilter *filter;
    AVCodecParserContext *parser;
    struct AVBitStreamFilterContext *next;
    char *args;
} AVBitStreamFilterContext;

void av_register_bitstream_filter(AVBitStreamFilter *bsf);

AVBitStreamFilterContext *av_bitstream_filter_init(const char *name);

int av_bitstream_filter_filter(AVBitStreamFilterContext *bsfc,
                               AVCodecContext *avctx, const char *args,
                               uint8_t **poutbuf, int *poutbuf_size,
                               const uint8_t *buf, int buf_size, int keyframe);

void av_bitstream_filter_close(AVBitStreamFilterContext *bsf);

const AVBitStreamFilter *av_bitstream_filter_next(const AVBitStreamFilter *f);

const AVBitStreamFilter *av_bsf_next(void **opaque);
void av_fast_padded_malloc(void *ptr, unsigned int *size, size_t min_size);

void av_fast_padded_mallocz(void *ptr, unsigned int *size, size_t min_size);
unsigned int av_xiphlacing(unsigned char *s, unsigned int v);

void av_register_hwaccel(AVHWAccel *hwaccel);

AVHWAccel *av_hwaccel_next(const AVHWAccel *hwaccel);
enum AVLockOp {
  AV_LOCK_CREATE,
  AV_LOCK_OBTAIN,
  AV_LOCK_RELEASE,
  AV_LOCK_DESTROY,
};

int av_lockmgr_register(int (*cb)(void **mutex, enum AVLockOp op));

int avcodec_is_open(AVCodecContext *s);
AVCPBProperties *av_cpb_properties_alloc(size_t *size);
#define AVFORMAT_AVIO_H 
#define AVFORMAT_VERSION_H 
#define LIBAVFORMAT_VERSION_MAJOR 58
#define LIBAVFORMAT_VERSION_MINOR 63
#define LIBAVFORMAT_VERSION_MICRO 100
#define LIBAVFORMAT_VERSION_INT AV_VERSION_INT(LIBAVFORMAT_VERSION_MAJOR, LIBAVFORMAT_VERSION_MINOR, LIBAVFORMAT_VERSION_MICRO)
#define LIBAVFORMAT_VERSION AV_VERSION(LIBAVFORMAT_VERSION_MAJOR, LIBAVFORMAT_VERSION_MINOR, LIBAVFORMAT_VERSION_MICRO)
#define LIBAVFORMAT_BUILD LIBAVFORMAT_VERSION_INT
#define LIBAVFORMAT_IDENT "Lavf" AV_STRINGIFY(LIBAVFORMAT_VERSION)
#define FF_API_COMPUTE_PKT_FIELDS2 (LIBAVFORMAT_VERSION_MAJOR < 59)
#define FF_API_OLD_OPEN_CALLBACKS (LIBAVFORMAT_VERSION_MAJOR < 59)
#define FF_API_LAVF_AVCTX (LIBAVFORMAT_VERSION_MAJOR < 59)
#define FF_API_HTTP_USER_AGENT (LIBAVFORMAT_VERSION_MAJOR < 59)
#define FF_API_HLS_WRAP (LIBAVFORMAT_VERSION_MAJOR < 59)
#define FF_API_HLS_USE_LOCALTIME (LIBAVFORMAT_VERSION_MAJOR < 59)
#define FF_API_LAVF_KEEPSIDE_FLAG (LIBAVFORMAT_VERSION_MAJOR < 59)
#define FF_API_OLD_ROTATE_API (LIBAVFORMAT_VERSION_MAJOR < 59)
#define FF_API_FORMAT_GET_SET (LIBAVFORMAT_VERSION_MAJOR < 59)
#define FF_API_OLD_AVIO_EOF_0 (LIBAVFORMAT_VERSION_MAJOR < 59)
#define FF_API_LAVF_FFSERVER (LIBAVFORMAT_VERSION_MAJOR < 59)
#define FF_API_FORMAT_FILENAME (LIBAVFORMAT_VERSION_MAJOR < 59)
#define FF_API_OLD_RTSP_OPTIONS (LIBAVFORMAT_VERSION_MAJOR < 59)
#define FF_API_DASH_MIN_SEG_DURATION (LIBAVFORMAT_VERSION_MAJOR < 59)
#define FF_API_LAVF_MP4A_LATM (LIBAVFORMAT_VERSION_MAJOR < 59)
#define FF_API_AVIOFORMAT (LIBAVFORMAT_VERSION_MAJOR < 59)
#define FF_API_R_FRAME_RATE 1
#define AVIO_SEEKABLE_NORMAL (1 << 0)
#define AVIO_SEEKABLE_TIME (1 << 1)
typedef struct AVIOInterruptCB {
    int (*callback)(void*);
    void *opaque;
} AVIOInterruptCB;

enum AVIODirEntryType {
    AVIO_ENTRY_UNKNOWN,
    AVIO_ENTRY_BLOCK_DEVICE,
    AVIO_ENTRY_CHARACTER_DEVICE,
    AVIO_ENTRY_DIRECTORY,
    AVIO_ENTRY_NAMED_PIPE,
    AVIO_ENTRY_SYMBOLIC_LINK,
    AVIO_ENTRY_SOCKET,
    AVIO_ENTRY_FILE,
    AVIO_ENTRY_SERVER,
    AVIO_ENTRY_SHARE,
    AVIO_ENTRY_WORKGROUP,
};

typedef struct AVIODirEntry {
    char *name;
    int type;
    int utf8;
    int64_t size;
    int64_t modification_timestamp;
    int64_t access_timestamp;
    int64_t status_change_timestamp;
    int64_t user_id;
    int64_t group_id;
    int64_t filemode;
} AVIODirEntry;

typedef struct AVIODirContext {
    struct URLContext *url_context;
} AVIODirContext;

enum AVIODataMarkerType {
    AVIO_DATA_MARKER_HEADER,
    AVIO_DATA_MARKER_SYNC_POINT,
    AVIO_DATA_MARKER_BOUNDARY_POINT,
    AVIO_DATA_MARKER_UNKNOWN,
    AVIO_DATA_MARKER_TRAILER,
    AVIO_DATA_MARKER_FLUSH_POINT,
};
typedef struct AVIOContext {
    const AVClass *av_class;
    unsigned char *buffer;
    int buffer_size;
    unsigned char *buf_ptr;
    unsigned char *buf_end;
    void *opaque;
    int (*read_packet)(void *opaque, uint8_t *buf, int buf_size);
    int (*write_packet)(void *opaque, uint8_t *buf, int buf_size);
    int64_t (*seek)(void *opaque, int64_t offset, int whence);
    int64_t pos;
    int eof_reached;
    int write_flag;
    int max_packet_size;
    unsigned long checksum;
    unsigned char *checksum_ptr;
    unsigned long (*update_checksum)(unsigned long checksum, const uint8_t *buf, unsigned int size);
    int error;
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
const char *avio_find_protocol_name(const char *url);
int avio_check(const char *url, int flags);
int avpriv_io_move(const char *url_src, const char *url_dst);

int avpriv_io_delete(const char *url);
int avio_open_dir(AVIODirContext **s, const char *url, AVDictionary **options);
int avio_read_dir(AVIODirContext *s, AVIODirEntry **next);
int avio_close_dir(AVIODirContext **s);

void avio_free_directory_entry(AVIODirEntry **entry);
AVIOContext *avio_alloc_context(
                  unsigned char *buffer,
                  int buffer_size,
                  int write_flag,
                  void *opaque,
                  int (*read_packet)(void *opaque, uint8_t *buf, int buf_size),
                  int (*write_packet)(void *opaque, uint8_t *buf, int buf_size),
                  int64_t (*seek)(void *opaque, int64_t offset, int whence));

void avio_context_free(AVIOContext **s);

void avio_w8(AVIOContext *s, int b);
void avio_write(AVIOContext *s, const unsigned char *buf, int size);
void avio_wl64(AVIOContext *s, uint64_t val);
void avio_wb64(AVIOContext *s, uint64_t val);
void avio_wl32(AVIOContext *s, unsigned int val);
void avio_wb32(AVIOContext *s, unsigned int val);
void avio_wl24(AVIOContext *s, unsigned int val);
void avio_wb24(AVIOContext *s, unsigned int val);
void avio_wl16(AVIOContext *s, unsigned int val);
void avio_wb16(AVIOContext *s, unsigned int val);

int avio_put_str(AVIOContext *s, const char *str);
int avio_put_str16le(AVIOContext *s, const char *str);
int avio_put_str16be(AVIOContext *s, const char *str);
void avio_write_marker(AVIOContext *s, int64_t time, enum AVIODataMarkerType type);
#define AVSEEK_SIZE 0x10000
#define AVSEEK_FORCE 0x20000

int64_t avio_seek(AVIOContext *s, int64_t offset, int whence);

int64_t avio_skip(AVIOContext *s, int64_t offset);

static inline int64_t avio_tell(AVIOContext *s)
{
    return avio_seek(s, 0, 
                          1
                                  );
}

int64_t avio_size(AVIOContext *s);

int avio_feof(AVIOContext *s);

int avio_printf(AVIOContext *s, const char *fmt, ...) ;

void avio_print_string_array(AVIOContext *s, const char *strings[]);
#define avio_print(s,...) avio_print_string_array(s, (const char*[]){__VA_ARGS__, NULL})
void avio_flush(AVIOContext *s);

int avio_read(AVIOContext *s, unsigned char *buf, int size);
int avio_read_partial(AVIOContext *s, unsigned char *buf, int size);
int avio_r8 (AVIOContext *s);
unsigned int avio_rl16(AVIOContext *s);
unsigned int avio_rl24(AVIOContext *s);
unsigned int avio_rl32(AVIOContext *s);
uint64_t avio_rl64(AVIOContext *s);
unsigned int avio_rb16(AVIOContext *s);
unsigned int avio_rb24(AVIOContext *s);
unsigned int avio_rb32(AVIOContext *s);
uint64_t avio_rb64(AVIOContext *s);
int avio_get_str(AVIOContext *pb, int maxlen, char *buf, int buflen);

int avio_get_str16le(AVIOContext *pb, int maxlen, char *buf, int buflen);
int avio_get_str16be(AVIOContext *pb, int maxlen, char *buf, int buflen);
#define AVIO_FLAG_READ 1
#define AVIO_FLAG_WRITE 2
#define AVIO_FLAG_READ_WRITE (AVIO_FLAG_READ|AVIO_FLAG_WRITE)
#define AVIO_FLAG_NONBLOCK 8
#define AVIO_FLAG_DIRECT 0x8000
int avio_open(AVIOContext **s, const char *url, int flags);
int avio_open2(AVIOContext **s, const char *url, int flags,
               const AVIOInterruptCB *int_cb, AVDictionary **options);
int avio_close(AVIOContext *s);
int avio_closep(AVIOContext **s);
int avio_open_dyn_buf(AVIOContext **s);
int avio_get_dyn_buf(AVIOContext *s, uint8_t **pbuffer);
int avio_close_dyn_buf(AVIOContext *s, uint8_t **pbuffer);
const char *avio_enum_protocols(void **opaque, int output);

const AVClass *avio_protocol_get_class(const char *name);
int avio_pause(AVIOContext *h, int pause);
int64_t avio_seek_time(AVIOContext *h, int stream_index,
                       int64_t timestamp, int flags);

struct AVBPrint;

int avio_read_to_bprint(AVIOContext *h, struct AVBPrint *pb, size_t max_size);
int avio_accept(AVIOContext *s, AVIOContext **c);
int avio_handshake(AVIOContext *c);

struct AVFormatContext;

struct AVDeviceInfoList;
struct AVDeviceCapabilitiesQuery;
int av_get_packet(AVIOContext *s, AVPacket *pkt, int size);
int av_append_packet(AVIOContext *s, AVPacket *pkt, int size);

struct AVCodecTag;

typedef struct AVProbeData {
    const char *filename;
    unsigned char *buf;
    int buf_size;
    const char *mime_type;
} AVProbeData;
#define AVPROBE_SCORE_RETRY (AVPROBE_SCORE_MAX/4)
#define AVPROBE_SCORE_STREAM_RETRY (AVPROBE_SCORE_MAX/4-1)
#define AVPROBE_SCORE_EXTENSION 50
#define AVPROBE_SCORE_MIME 75
#define AVPROBE_SCORE_MAX 100
#define AVPROBE_PADDING_SIZE 32
#define AVFMT_NOFILE 0x0001
#define AVFMT_NEEDNUMBER 0x0002
#define AVFMT_SHOW_IDS 0x0008
#define AVFMT_GLOBALHEADER 0x0040
#define AVFMT_NOTIMESTAMPS 0x0080
#define AVFMT_GENERIC_INDEX 0x0100
#define AVFMT_TS_DISCONT 0x0200
#define AVFMT_VARIABLE_FPS 0x0400
#define AVFMT_NODIMENSIONS 0x0800
#define AVFMT_NOSTREAMS 0x1000
#define AVFMT_NOBINSEARCH 0x2000
#define AVFMT_NOGENSEARCH 0x4000
#define AVFMT_NO_BYTE_SEEK 0x8000
#define AVFMT_ALLOW_FLUSH 0x10000
#define AVFMT_TS_NONSTRICT 0x20000
#define AVFMT_TS_NEGATIVE 0x40000
#define AVFMT_SEEK_TO_PTS 0x4000000

typedef struct AVOutputFormat {
    const char *name;
    const char *long_name;
    const char *mime_type;
    const char *extensions;
    enum AVCodecID audio_codec;
    enum AVCodecID video_codec;
    enum AVCodecID subtitle_codec;
    int flags;
    const struct AVCodecTag * const *codec_tag;
    const AVClass *priv_class;
#define ff_const59 
    struct AVOutputFormat *next;
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
    enum AVCodecID data_codec;
    int (*init)(struct AVFormatContext *);
    void (*deinit)(struct AVFormatContext *);
    int (*check_bitstream)(struct AVFormatContext *, const AVPacket *pkt);
} AVOutputFormat;
typedef struct AVInputFormat {
    const char *name;
    const char *long_name;
    int flags;
    const char *extensions;
    const struct AVCodecTag * const *codec_tag;
    const AVClass *priv_class;
    const char *mime_type;
    struct AVInputFormat *next;
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

enum AVStreamParseType {
    AVSTREAM_PARSE_NONE,
    AVSTREAM_PARSE_FULL,
    AVSTREAM_PARSE_HEADERS,
    AVSTREAM_PARSE_TIMESTAMPS,
    AVSTREAM_PARSE_FULL_ONCE,
    AVSTREAM_PARSE_FULL_RAW,
};

typedef struct AVIndexEntry {
    int64_t pos;
    int64_t timestamp;
#define AVINDEX_KEYFRAME 0x0001
#define AVINDEX_DISCARD_FRAME 0x0002
    int flags:2;
    int size:30;
    int min_distance;
} AVIndexEntry;
#define AV_DISPOSITION_DEFAULT 0x0001
#define AV_DISPOSITION_DUB 0x0002
#define AV_DISPOSITION_ORIGINAL 0x0004
#define AV_DISPOSITION_COMMENT 0x0008
#define AV_DISPOSITION_LYRICS 0x0010
#define AV_DISPOSITION_KARAOKE 0x0020
#define AV_DISPOSITION_FORCED 0x0040
#define AV_DISPOSITION_HEARING_IMPAIRED 0x0080
#define AV_DISPOSITION_VISUAL_IMPAIRED 0x0100
#define AV_DISPOSITION_CLEAN_EFFECTS 0x0200
#define AV_DISPOSITION_ATTACHED_PIC 0x0400
#define AV_DISPOSITION_TIMED_THUMBNAILS 0x0800

typedef struct AVStreamInternal AVStreamInternal;
#define AV_DISPOSITION_CAPTIONS 0x10000
#define AV_DISPOSITION_DESCRIPTIONS 0x20000
#define AV_DISPOSITION_METADATA 0x40000
#define AV_DISPOSITION_DEPENDENT 0x80000
#define AV_DISPOSITION_STILL_IMAGE 0x100000
#define AV_PTS_WRAP_IGNORE 0
#define AV_PTS_WRAP_ADD_OFFSET 1
#define AV_PTS_WRAP_SUB_OFFSET -1
typedef struct AVStream {
    int index;
    int id;
   
    AVCodecContext *codec;
    void *priv_data;
    AVRational time_base;
    int64_t start_time;
    int64_t duration;
    int64_t nb_frames;
    int disposition;
    enum AVDiscard discard;
    AVRational sample_aspect_ratio;
    AVDictionary *metadata;
    AVRational avg_frame_rate;
    AVPacket attached_pic;
    AVPacketSideData *side_data;
    int nb_side_data;
    int event_flags;
#define AVSTREAM_EVENT_FLAG_METADATA_UPDATED 0x0001
    AVRational r_frame_rate;
   
    char *recommended_encoder_configuration;
    AVCodecParameters *codecpar;
#define MAX_STD_TIMEBASES (30*12+30+3+6)
    struct {
        int64_t last_dts;
        int64_t duration_gcd;
        int duration_count;
        int64_t rfps_duration_sum;
        double (*duration_error)[2][(30*12+30+3+6)];
        int64_t codec_info_duration;
        int64_t codec_info_duration_fields;
        int frame_delay_evidence;
        int found_decoder;
        int64_t last_duration;
        int64_t fps_first_dts;
        int fps_first_dts_idx;
        int64_t fps_last_dts;
        int fps_last_dts_idx;
    } *info;
    int pts_wrap_bits;
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
#define MAX_REORDER_DELAY 16
    int64_t pts_buffer[16 +1];
    AVIndexEntry *index_entries;
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
    int64_t pts_reorder_error[16 +1];
    uint8_t pts_reorder_error_count[16 +1];
    int64_t last_dts_for_order_check;
    uint8_t dts_ordered;
    uint8_t dts_misordered;
    int inject_global_side_data;
    AVRational display_aspect_ratio;
    AVStreamInternal *internal;
} AVStream;

AVRational av_stream_get_r_frame_rate(const AVStream *s);

void av_stream_set_r_frame_rate(AVStream *s, AVRational r);

char* av_stream_get_recommended_encoder_configuration(const AVStream *s);

void av_stream_set_recommended_encoder_configuration(AVStream *s, char *configuration);

struct AVCodecParserContext *av_stream_get_parser(const AVStream *s);

int64_t av_stream_get_end_pts(const AVStream *st);
#define AV_PROGRAM_RUNNING 1

typedef struct AVProgram {
    int id;
    int flags;
    enum AVDiscard discard;
    unsigned int *stream_index;
    unsigned int nb_stream_indexes;
    AVDictionary *metadata;
    int program_num;
    int pmt_pid;
    int pcr_pid;
    int pmt_version;
    int64_t start_time;
    int64_t end_time;
    int64_t pts_wrap_reference;
    int pts_wrap_behavior;
} AVProgram;
#define AVFMTCTX_NOHEADER 0x0001
#define AVFMTCTX_UNSEEKABLE 0x0002

typedef struct AVChapter {
    int id;
    AVRational time_base;
    int64_t start, end;
    AVDictionary *metadata;
} AVChapter;

typedef int (*av_format_control_message)(struct AVFormatContext *s, int type,
                                         void *data, size_t data_size);

typedef int (*AVOpenCallback)(struct AVFormatContext *s, AVIOContext **pb, const char *url, int flags,
                              const AVIOInterruptCB *int_cb, AVDictionary **options);

enum AVDurationEstimationMethod {
    AVFMT_DURATION_FROM_PTS,
    AVFMT_DURATION_FROM_STREAM,
    AVFMT_DURATION_FROM_BITRATE
};

typedef struct AVFormatInternal AVFormatInternal;
typedef struct AVFormatContext {
    const AVClass *av_class;
    struct AVInputFormat *iformat;
    struct AVOutputFormat *oformat;
    void *priv_data;
    AVIOContext *pb;
    int ctx_flags;
    unsigned int nb_streams;
    AVStream **streams;
   
    char filename[1024];
    char *url;
    int64_t start_time;
    int64_t duration;
    int64_t bit_rate;
    unsigned int packet_size;
    int max_delay;
    int flags;
#define AVFMT_FLAG_GENPTS 0x0001
#define AVFMT_FLAG_IGNIDX 0x0002
#define AVFMT_FLAG_NONBLOCK 0x0004
#define AVFMT_FLAG_IGNDTS 0x0008
#define AVFMT_FLAG_NOFILLIN 0x0010
#define AVFMT_FLAG_NOPARSE 0x0020
#define AVFMT_FLAG_NOBUFFER 0x0040
#define AVFMT_FLAG_CUSTOM_IO 0x0080
#define AVFMT_FLAG_DISCARD_CORRUPT 0x0100
#define AVFMT_FLAG_FLUSH_PACKETS 0x0200
#define AVFMT_FLAG_BITEXACT 0x0400
#define AVFMT_FLAG_MP4A_LATM 0x8000
#define AVFMT_FLAG_SORT_DTS 0x10000
#define AVFMT_FLAG_PRIV_OPT 0x20000
#define AVFMT_FLAG_KEEP_SIDE_DATA 0x40000
#define AVFMT_FLAG_FAST_SEEK 0x80000
#define AVFMT_FLAG_SHORTEST 0x100000
#define AVFMT_FLAG_AUTO_BSF 0x200000
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
#define FF_FDEBUG_TS 0x0001
    int64_t max_interleave_delta;
    int strict_std_compliance;
    int event_flags;
#define AVFMT_EVENT_FLAG_METADATA_UPDATED 0x0001
    int max_ts_probe;
    int avoid_negative_ts;
#define AVFMT_AVOID_NEG_TS_AUTO -1
#define AVFMT_AVOID_NEG_TS_MAKE_NON_NEGATIVE 1
#define AVFMT_AVOID_NEG_TS_MAKE_ZERO 2
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
   
    int (*open_cb)(struct AVFormatContext *s, AVIOContext **p, const char *url, int flags, const AVIOInterruptCB *int_cb, AVDictionary **options);
    char *protocol_whitelist;
    int (*io_open)(struct AVFormatContext *s, AVIOContext **pb, const char *url,
                   int flags, AVDictionary **options);
    void (*io_close)(struct AVFormatContext *s, AVIOContext *pb);
    char *protocol_blacklist;
    int max_streams;
    int skip_estimate_duration_from_pts;
    int max_probe_packets;
} AVFormatContext;

int av_format_get_probe_score(const AVFormatContext *s);

AVCodec * av_format_get_video_codec(const AVFormatContext *s);

void av_format_set_video_codec(AVFormatContext *s, AVCodec *c);

AVCodec * av_format_get_audio_codec(const AVFormatContext *s);

void av_format_set_audio_codec(AVFormatContext *s, AVCodec *c);

AVCodec * av_format_get_subtitle_codec(const AVFormatContext *s);

void av_format_set_subtitle_codec(AVFormatContext *s, AVCodec *c);

AVCodec * av_format_get_data_codec(const AVFormatContext *s);

void av_format_set_data_codec(AVFormatContext *s, AVCodec *c);

int av_format_get_metadata_header_padding(const AVFormatContext *s);

void av_format_set_metadata_header_padding(AVFormatContext *s, int c);

void * av_format_get_opaque(const AVFormatContext *s);

void av_format_set_opaque(AVFormatContext *s, void *opaque);

av_format_control_message av_format_get_control_message_cb(const AVFormatContext *s);

void av_format_set_control_message_cb(AVFormatContext *s, av_format_control_message callback);
 AVOpenCallback av_format_get_open_cb(const AVFormatContext *s);
 void av_format_set_open_cb(AVFormatContext *s, AVOpenCallback callback);

void av_format_inject_global_side_data(AVFormatContext *s);

enum AVDurationEstimationMethod av_fmt_ctx_get_duration_estimation_method(const AVFormatContext* ctx);
unsigned avformat_version(void);

const char *avformat_configuration(void);

const char *avformat_license(void);

void av_register_all(void);

void av_register_input_format(AVInputFormat *format);

void av_register_output_format(AVOutputFormat *format);
int avformat_network_init(void);

int avformat_network_deinit(void);

AVInputFormat *av_iformat_next(const AVInputFormat *f);

AVOutputFormat *av_oformat_next(const AVOutputFormat *f);
const AVOutputFormat *av_muxer_iterate(void **opaque);
const AVInputFormat *av_demuxer_iterate(void **opaque);

AVFormatContext *avformat_alloc_context(void);

void avformat_free_context(AVFormatContext *s);

const AVClass *avformat_get_class(void);
AVStream *avformat_new_stream(AVFormatContext *s, const AVCodec *c);
int av_stream_add_side_data(AVStream *st, enum AVPacketSideDataType type,
                            uint8_t *data, size_t size);
uint8_t *av_stream_new_side_data(AVStream *stream,
                                 enum AVPacketSideDataType type, int size);
uint8_t *av_stream_get_side_data(const AVStream *stream,
                                 enum AVPacketSideDataType type, int *size);

AVProgram *av_new_program(AVFormatContext *s, int id);
int avformat_alloc_output_context2(AVFormatContext **ctx, AVOutputFormat *oformat,
                                   const char *format_name, const char *filename);
 AVInputFormat *av_find_input_format(const char *short_name);
 AVInputFormat *av_probe_input_format( AVProbeData *pd, int is_opened);
 AVInputFormat *av_probe_input_format2( AVProbeData *pd, int is_opened, int *score_max);
 AVInputFormat *av_probe_input_format3( AVProbeData *pd, int is_opened, int *score_ret);
int av_probe_input_buffer2(AVIOContext *pb, AVInputFormat **fmt,
                           const char *url, void *logctx,
                           unsigned int offset, unsigned int max_probe_size);

int av_probe_input_buffer(AVIOContext *pb, AVInputFormat **fmt,
                          const char *url, void *logctx,
                          unsigned int offset, unsigned int max_probe_size);
int avformat_open_input(AVFormatContext **ps, const char *url, AVInputFormat *fmt, AVDictionary **options);

int av_demuxer_open(AVFormatContext *ic);
int avformat_find_stream_info(AVFormatContext *ic, AVDictionary **options);
AVProgram *av_find_program_from_stream(AVFormatContext *ic, AVProgram *last, int s);

void av_program_add_stream_index(AVFormatContext *ac, int progid, unsigned int idx);
int av_find_best_stream(AVFormatContext *ic,
                        enum AVMediaType type,
                        int wanted_stream_nb,
                        int related_stream,
                        AVCodec **decoder_ret,
                        int flags);
int av_read_frame(AVFormatContext *s, AVPacket *pkt);
int av_seek_frame(AVFormatContext *s, int stream_index, int64_t timestamp,
                  int flags);
int avformat_seek_file(AVFormatContext *s, int stream_index, int64_t min_ts, int64_t ts, int64_t max_ts, int flags);
int avformat_flush(AVFormatContext *s);

int av_read_play(AVFormatContext *s);

int av_read_pause(AVFormatContext *s);

void avformat_close_input(AVFormatContext **s);
#define AVSEEK_FLAG_BACKWARD 1
#define AVSEEK_FLAG_BYTE 2
#define AVSEEK_FLAG_ANY 4
#define AVSEEK_FLAG_FRAME 8
#define AVSTREAM_INIT_IN_WRITE_HEADER 0
#define AVSTREAM_INIT_IN_INIT_OUTPUT 1

int avformat_write_header(AVFormatContext *s, AVDictionary **options);

int avformat_init_output(AVFormatContext *s, AVDictionary **options);
int av_write_frame(AVFormatContext *s, AVPacket *pkt);
int av_interleaved_write_frame(AVFormatContext *s, AVPacket *pkt);
int av_write_uncoded_frame(AVFormatContext *s, int stream_index,
                           AVFrame *frame);
int av_interleaved_write_uncoded_frame(AVFormatContext *s, int stream_index,
                                       AVFrame *frame);

int av_write_uncoded_frame_query(AVFormatContext *s, int stream_index);
int av_write_trailer(AVFormatContext *s);
 AVOutputFormat *av_guess_format(const char *short_name,
                                const char *filename,
                                const char *mime_type);

enum AVCodecID av_guess_codec( AVOutputFormat *fmt, const char *short_name,
                            const char *filename, const char *mime_type,
                            enum AVMediaType type);
int av_get_output_timestamp(struct AVFormatContext *s, int stream,
                            int64_t *dts, int64_t *wall);
void av_hex_dump(FILE *f, const uint8_t *buf, int size);
void av_hex_dump_log(void *avcl, int level, const uint8_t *buf, int size);
void av_pkt_dump2(FILE *f, const AVPacket *pkt, int dump_payload, const AVStream *st);
void av_pkt_dump_log2(void *avcl, int level, const AVPacket *pkt, int dump_payload,
                      const AVStream *st);
enum AVCodecID av_codec_get_id(const struct AVCodecTag * const *tags, unsigned int tag);
unsigned int av_codec_get_tag(const struct AVCodecTag * const *tags, enum AVCodecID id);
int av_codec_get_tag2(const struct AVCodecTag * const *tags, enum AVCodecID id,
                      unsigned int *tag);

int av_find_default_stream_index(AVFormatContext *s);
int av_index_search_timestamp(AVStream *st, int64_t timestamp, int flags);

int av_add_index_entry(AVStream *st, int64_t pos, int64_t timestamp,
                       int size, int distance, int flags);
void av_url_split(char *proto, int proto_size,
                  char *authorization, int authorization_size,
                  char *hostname, int hostname_size,
                  int *port_ptr,
                  char *path, int path_size,
                  const char *url);
void av_dump_format(AVFormatContext *ic,
                    int index,
                    const char *url,
                    int is_output);
#define AV_FRAME_FILENAME_FLAGS_MULTIPLE 1
int av_get_frame_filename2(char *buf, int buf_size,
                          const char *path, int number, int flags);

int av_get_frame_filename(char *buf, int buf_size,
                          const char *path, int number);

int av_filename_number_test(const char *filename);
int av_sdp_create(AVFormatContext *ac[], int n_files, char *buf, int size);
int av_match_ext(const char *filename, const char *extensions);
int avformat_query_codec(const AVOutputFormat *ofmt, enum AVCodecID codec_id,
                         int std_compliance);
const struct AVCodecTag *avformat_get_riff_video_tags(void);

const struct AVCodecTag *avformat_get_riff_audio_tags(void);

const struct AVCodecTag *avformat_get_mov_video_tags(void);

const struct AVCodecTag *avformat_get_mov_audio_tags(void);
AVRational av_guess_sample_aspect_ratio(AVFormatContext *format, AVStream *stream, AVFrame *frame);
AVRational av_guess_frame_rate(AVFormatContext *ctx, AVStream *stream, AVFrame *frame);
int avformat_match_stream_specifier(AVFormatContext *s, AVStream *st,
                                    const char *spec);

int avformat_queue_attached_pictures(AVFormatContext *s);

int av_apply_bitstream_filters(AVCodecContext *codec, AVPacket *pkt,
                               AVBitStreamFilterContext *bsfc);

enum AVTimebaseSource {
    AVFMT_TBCF_AUTO = -1,
    AVFMT_TBCF_DECODER,
    AVFMT_TBCF_DEMUXER,
    AVFMT_TBCF_R_FRAMERATE,
};
int avformat_transfer_internal_stream_timing_info(const AVOutputFormat *ofmt,
                                                  AVStream *ost, const AVStream *ist,
                                                  enum AVTimebaseSource copy_tb);

AVRational av_stream_get_codec_timebase(const AVStream *st);
#define AVDEVICE_AVDEVICE_H 
#define AVDEVICE_VERSION_H 
#define LIBAVDEVICE_VERSION_MAJOR 58
#define LIBAVDEVICE_VERSION_MINOR 11
#define LIBAVDEVICE_VERSION_MICRO 102
#define LIBAVDEVICE_VERSION_INT AV_VERSION_INT(LIBAVDEVICE_VERSION_MAJOR, LIBAVDEVICE_VERSION_MINOR, LIBAVDEVICE_VERSION_MICRO)
#define LIBAVDEVICE_VERSION AV_VERSION(LIBAVDEVICE_VERSION_MAJOR, LIBAVDEVICE_VERSION_MINOR, LIBAVDEVICE_VERSION_MICRO)
#define LIBAVDEVICE_BUILD LIBAVDEVICE_VERSION_INT
#define LIBAVDEVICE_IDENT "Lavd" AV_STRINGIFY(LIBAVDEVICE_VERSION)
#define AVUTIL_OPT_H 
enum AVOptionType{
    AV_OPT_TYPE_FLAGS,
    AV_OPT_TYPE_INT,
    AV_OPT_TYPE_INT64,
    AV_OPT_TYPE_DOUBLE,
    AV_OPT_TYPE_FLOAT,
    AV_OPT_TYPE_STRING,
    AV_OPT_TYPE_RATIONAL,
    AV_OPT_TYPE_BINARY,
    AV_OPT_TYPE_DICT,
    AV_OPT_TYPE_UINT64,
    AV_OPT_TYPE_CONST,
    AV_OPT_TYPE_IMAGE_SIZE,
    AV_OPT_TYPE_PIXEL_FMT,
    AV_OPT_TYPE_SAMPLE_FMT,
    AV_OPT_TYPE_VIDEO_RATE,
    AV_OPT_TYPE_DURATION,
    AV_OPT_TYPE_COLOR,
    AV_OPT_TYPE_CHANNEL_LAYOUT,
    AV_OPT_TYPE_BOOL,
};

typedef struct AVOption {
    const char *name;
    const char *help;
    int offset;
    enum AVOptionType type;
    union {
        int64_t i64;
        double dbl;
        const char *str;
        AVRational q;
    } default_val;
    double min;
    double max;
    int flags;
#define AV_OPT_FLAG_ENCODING_PARAM 1
#define AV_OPT_FLAG_DECODING_PARAM 2
#define AV_OPT_FLAG_AUDIO_PARAM 8
#define AV_OPT_FLAG_VIDEO_PARAM 16
#define AV_OPT_FLAG_SUBTITLE_PARAM 32
#define AV_OPT_FLAG_EXPORT 64
#define AV_OPT_FLAG_READONLY 128
#define AV_OPT_FLAG_BSF_PARAM (1<<8)
#define AV_OPT_FLAG_RUNTIME_PARAM (1<<15)
#define AV_OPT_FLAG_FILTERING_PARAM (1<<16)
#define AV_OPT_FLAG_DEPRECATED (1<<17)
#define AV_OPT_FLAG_CHILD_CONSTS (1<<18)
    const char *unit;
} AVOption;

typedef struct AVOptionRange {
    const char *str;
    double value_min, value_max;
    double component_min, component_max;
    int is_range;
} AVOptionRange;

typedef struct AVOptionRanges {
    AVOptionRange **range;
    int nb_ranges;
    int nb_components;
} AVOptionRanges;
int av_opt_show2(void *obj, void *av_log_obj, int req_flags, int rej_flags);

void av_opt_set_defaults(void *s);
void av_opt_set_defaults2(void *s, int mask, int flags);
int av_set_options_string(void *ctx, const char *opts,
                          const char *key_val_sep, const char *pairs_sep);
int av_opt_set_from_string(void *ctx, const char *opts,
                           const char *const *shorthand,
                           const char *key_val_sep, const char *pairs_sep);

void av_opt_free(void *obj);
int av_opt_flag_is_set(void *obj, const char *field_name, const char *flag_name);
int av_opt_set_dict(void *obj, struct AVDictionary **options);
int av_opt_set_dict2(void *obj, struct AVDictionary **options, int search_flags);
int av_opt_get_key_value(const char **ropts,
                         const char *key_val_sep, const char *pairs_sep,
                         unsigned flags,
                         char **rkey, char **rval);

enum {
    AV_OPT_FLAG_IMPLICIT_KEY = 1,
};
int av_opt_eval_flags (void *obj, const AVOption *o, const char *val, int *flags_out);
int av_opt_eval_int (void *obj, const AVOption *o, const char *val, int *int_out);
int av_opt_eval_int64 (void *obj, const AVOption *o, const char *val, int64_t *int64_out);
int av_opt_eval_float (void *obj, const AVOption *o, const char *val, float *float_out);
int av_opt_eval_double(void *obj, const AVOption *o, const char *val, double *double_out);
int av_opt_eval_q (void *obj, const AVOption *o, const char *val, AVRational *q_out);
#define AV_OPT_SEARCH_CHILDREN (1 << 0)
#define AV_OPT_SEARCH_FAKE_OBJ (1 << 1)
#define AV_OPT_ALLOW_NULL (1 << 2)
#define AV_OPT_MULTI_COMPONENT_RANGE (1 << 12)
const AVOption *av_opt_find(void *obj, const char *name, const char *unit,
                            int opt_flags, int search_flags);
const AVOption *av_opt_find2(void *obj, const char *name, const char *unit,
                             int opt_flags, int search_flags, void **target_obj);
const AVOption *av_opt_next(const void *obj, const AVOption *prev);

void *av_opt_child_next(void *obj, void *prev);

const AVClass *av_opt_child_class_next(const AVClass *parent, const AVClass *prev);
const AVClass *av_opt_child_class_iterate(const AVClass *parent, void **iter);
int av_opt_set (void *obj, const char *name, const char *val, int search_flags);
int av_opt_set_int (void *obj, const char *name, int64_t val, int search_flags);
int av_opt_set_double (void *obj, const char *name, double val, int search_flags);
int av_opt_set_q (void *obj, const char *name, AVRational val, int search_flags);
int av_opt_set_bin (void *obj, const char *name, const uint8_t *val, int size, int search_flags);
int av_opt_set_image_size(void *obj, const char *name, int w, int h, int search_flags);
int av_opt_set_pixel_fmt (void *obj, const char *name, enum AVPixelFormat fmt, int search_flags);
int av_opt_set_sample_fmt(void *obj, const char *name, enum AVSampleFormat fmt, int search_flags);
int av_opt_set_video_rate(void *obj, const char *name, AVRational val, int search_flags);
int av_opt_set_channel_layout(void *obj, const char *name, int64_t ch_layout, int search_flags);

int av_opt_set_dict_val(void *obj, const char *name, const AVDictionary *val, int search_flags);
#define av_opt_set_int_list(obj,name,val,term,flags) (av_int_list_length(val, term) > INT_MAX / sizeof(*(val)) ? AVERROR(EINVAL) : av_opt_set_bin(obj, name, (const uint8_t *)(val), av_int_list_length(val, term) * sizeof(*(val)), flags))
int av_opt_get (void *obj, const char *name, int search_flags, uint8_t **out_val);
int av_opt_get_int (void *obj, const char *name, int search_flags, int64_t *out_val);
int av_opt_get_double (void *obj, const char *name, int search_flags, double *out_val);
int av_opt_get_q (void *obj, const char *name, int search_flags, AVRational *out_val);
int av_opt_get_image_size(void *obj, const char *name, int search_flags, int *w_out, int *h_out);
int av_opt_get_pixel_fmt (void *obj, const char *name, int search_flags, enum AVPixelFormat *out_fmt);
int av_opt_get_sample_fmt(void *obj, const char *name, int search_flags, enum AVSampleFormat *out_fmt);
int av_opt_get_video_rate(void *obj, const char *name, int search_flags, AVRational *out_val);
int av_opt_get_channel_layout(void *obj, const char *name, int search_flags, int64_t *ch_layout);

int av_opt_get_dict_val(void *obj, const char *name, int search_flags, AVDictionary **out_val);
void *av_opt_ptr(const AVClass *avclass, void *obj, const char *name);

void av_opt_freep_ranges(AVOptionRanges **ranges);
int av_opt_query_ranges(AVOptionRanges **, void *obj, const char *key, int flags);
int av_opt_copy(void *dest, const void *src);
int av_opt_query_ranges_default(AVOptionRanges **, void *obj, const char *key, int flags);
int av_opt_is_set_to_default(void *obj, const AVOption *o);
int av_opt_is_set_to_default_by_name(void *obj, const char *name, int search_flags);
#define AV_OPT_SERIALIZE_SKIP_DEFAULTS 0x00000001
#define AV_OPT_SERIALIZE_OPT_FLAGS_EXACT 0x00000002
int av_opt_serialize(void *obj, int opt_flags, int flags, char **buffer,
                     const char key_val_sep, const char pairs_sep);

unsigned avdevice_version(void);

const char *avdevice_configuration(void);

const char *avdevice_license(void);

void avdevice_register_all(void);
AVInputFormat *av_input_audio_device_next(AVInputFormat *d);
AVInputFormat *av_input_video_device_next(AVInputFormat *d);
AVOutputFormat *av_output_audio_device_next(AVOutputFormat *d);
AVOutputFormat *av_output_video_device_next(AVOutputFormat *d);

typedef struct AVDeviceRect {
    int x;
    int y;
    int width;
    int height;
} AVDeviceRect;

enum AVAppToDevMessageType {
    AV_APP_TO_DEV_NONE = (('E') | (('N') << 8) | (('O') << 16) | ((unsigned)('N') << 24)),
    AV_APP_TO_DEV_WINDOW_SIZE = (('M') | (('O') << 8) | (('E') << 16) | ((unsigned)('G') << 24)),
    AV_APP_TO_DEV_WINDOW_REPAINT = (('A') | (('P') << 8) | (('E') << 16) | ((unsigned)('R') << 24)),
    AV_APP_TO_DEV_PAUSE = ((' ') | (('U') << 8) | (('A') << 16) | ((unsigned)('P') << 24)),
    AV_APP_TO_DEV_PLAY = (('Y') | (('A') << 8) | (('L') << 16) | ((unsigned)('P') << 24)),
    AV_APP_TO_DEV_TOGGLE_PAUSE = (('T') | (('U') << 8) | (('A') << 16) | ((unsigned)('P') << 24)),
    AV_APP_TO_DEV_SET_VOLUME = (('L') | (('O') << 8) | (('V') << 16) | ((unsigned)('S') << 24)),
    AV_APP_TO_DEV_MUTE = (('T') | (('U') << 8) | (('M') << 16) | ((unsigned)(' ') << 24)),
    AV_APP_TO_DEV_UNMUTE = (('T') | (('U') << 8) | (('M') << 16) | ((unsigned)('U') << 24)),
    AV_APP_TO_DEV_TOGGLE_MUTE = (('T') | (('U') << 8) | (('M') << 16) | ((unsigned)('T') << 24)),
    AV_APP_TO_DEV_GET_VOLUME = (('L') | (('O') << 8) | (('V') << 16) | ((unsigned)('G') << 24)),
    AV_APP_TO_DEV_GET_MUTE = (('T') | (('U') << 8) | (('M') << 16) | ((unsigned)('G') << 24)),
};

enum AVDevToAppMessageType {
    AV_DEV_TO_APP_NONE = (('E') | (('N') << 8) | (('O') << 16) | ((unsigned)('N') << 24)),
    AV_DEV_TO_APP_CREATE_WINDOW_BUFFER = (('E') | (('R') << 8) | (('C') << 16) | ((unsigned)('B') << 24)),
    AV_DEV_TO_APP_PREPARE_WINDOW_BUFFER = (('E') | (('R') << 8) | (('P') << 16) | ((unsigned)('B') << 24)),
    AV_DEV_TO_APP_DISPLAY_WINDOW_BUFFER = (('S') | (('I') << 8) | (('D') << 16) | ((unsigned)('B') << 24)),
    AV_DEV_TO_APP_DESTROY_WINDOW_BUFFER = (('S') | (('E') << 8) | (('D') << 16) | ((unsigned)('B') << 24)),
    AV_DEV_TO_APP_BUFFER_OVERFLOW = (('L') | (('F') << 8) | (('O') << 16) | ((unsigned)('B') << 24)),
    AV_DEV_TO_APP_BUFFER_UNDERFLOW = (('L') | (('F') << 8) | (('U') << 16) | ((unsigned)('B') << 24)),
    AV_DEV_TO_APP_BUFFER_READABLE = ((' ') | (('D') << 8) | (('R') << 16) | ((unsigned)('B') << 24)),
    AV_DEV_TO_APP_BUFFER_WRITABLE = ((' ') | (('R') << 8) | (('W') << 16) | ((unsigned)('B') << 24)),
    AV_DEV_TO_APP_MUTE_STATE_CHANGED = (('T') | (('U') << 8) | (('M') << 16) | ((unsigned)('C') << 24)),
    AV_DEV_TO_APP_VOLUME_LEVEL_CHANGED = (('L') | (('O') << 8) | (('V') << 16) | ((unsigned)('C') << 24)),
};
int avdevice_app_to_dev_control_message(struct AVFormatContext *s,
                                        enum AVAppToDevMessageType type,
                                        void *data, size_t data_size);
int avdevice_dev_to_app_control_message(struct AVFormatContext *s,
                                        enum AVDevToAppMessageType type,
                                        void *data, size_t data_size);
typedef struct AVDeviceCapabilitiesQuery {
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

extern const AVOption av_device_capabilities[];
int avdevice_capabilities_create(AVDeviceCapabilitiesQuery **caps, AVFormatContext *s,
                                 AVDictionary **device_options);

void avdevice_capabilities_free(AVDeviceCapabilitiesQuery **caps, AVFormatContext *s);

typedef struct AVDeviceInfo {
    char *device_name;
    char *device_description;
} AVDeviceInfo;

typedef struct AVDeviceInfoList {
    AVDeviceInfo **devices;
    int nb_devices;
    int default_device;
} AVDeviceInfoList;
int avdevice_list_devices(struct AVFormatContext *s, AVDeviceInfoList **device_list);

void avdevice_free_list_devices(AVDeviceInfoList **device_list);
int avdevice_list_input_sources(struct AVInputFormat *device, const char *device_name,
                                AVDictionary *device_options, AVDeviceInfoList **device_list);
int avdevice_list_output_sinks(struct AVOutputFormat *device, const char *device_name,
                               AVDictionary *device_options, AVDeviceInfoList **device_list);
#define SWSCALE_SWSCALE_H 
#define SWSCALE_VERSION_H 
#define LIBSWSCALE_VERSION_MAJOR 5
#define LIBSWSCALE_VERSION_MINOR 8
#define LIBSWSCALE_VERSION_MICRO 100
#define LIBSWSCALE_VERSION_INT AV_VERSION_INT(LIBSWSCALE_VERSION_MAJOR, LIBSWSCALE_VERSION_MINOR, LIBSWSCALE_VERSION_MICRO)
#define LIBSWSCALE_VERSION AV_VERSION(LIBSWSCALE_VERSION_MAJOR, LIBSWSCALE_VERSION_MINOR, LIBSWSCALE_VERSION_MICRO)
#define LIBSWSCALE_BUILD LIBSWSCALE_VERSION_INT
#define LIBSWSCALE_IDENT "SwS" AV_STRINGIFY(LIBSWSCALE_VERSION)
#define FF_API_SWS_VECTOR (LIBSWSCALE_VERSION_MAJOR < 6)
unsigned swscale_version(void);

const char *swscale_configuration(void);

const char *swscale_license(void);
#define SWS_FAST_BILINEAR 1
#define SWS_BILINEAR 2
#define SWS_BICUBIC 4
#define SWS_X 8
#define SWS_POINT 0x10
#define SWS_AREA 0x20
#define SWS_BICUBLIN 0x40
#define SWS_GAUSS 0x80
#define SWS_SINC 0x100
#define SWS_LANCZOS 0x200
#define SWS_SPLINE 0x400
#define SWS_SRC_V_CHR_DROP_MASK 0x30000
#define SWS_SRC_V_CHR_DROP_SHIFT 16
#define SWS_PARAM_DEFAULT 123456
#define SWS_PRINT_INFO 0x1000
#define SWS_FULL_CHR_H_INT 0x2000
#define SWS_FULL_CHR_H_INP 0x4000
#define SWS_DIRECT_BGR 0x8000
#define SWS_ACCURATE_RND 0x40000
#define SWS_BITEXACT 0x80000
#define SWS_ERROR_DIFFUSION 0x800000
#define SWS_MAX_REDUCE_CUTOFF 0.002
#define SWS_CS_ITU709 1
#define SWS_CS_FCC 4
#define SWS_CS_ITU601 5
#define SWS_CS_ITU624 5
#define SWS_CS_SMPTE170M 5
#define SWS_CS_SMPTE240M 7
#define SWS_CS_DEFAULT 5
#define SWS_CS_BT2020 9
const int *sws_getCoefficients(int colorspace);

typedef struct SwsVector {
    double *coeff;
    int length;
} SwsVector;

typedef struct SwsFilter {
    SwsVector *lumH;
    SwsVector *lumV;
    SwsVector *chrH;
    SwsVector *chrV;
} SwsFilter;

struct SwsContext;

int sws_isSupportedInput(enum AVPixelFormat pix_fmt);

int sws_isSupportedOutput(enum AVPixelFormat pix_fmt);

int sws_isSupportedEndiannessConversion(enum AVPixelFormat pix_fmt);

struct SwsContext *sws_alloc_context(void);

int sws_init_context(struct SwsContext *sws_context, SwsFilter *srcFilter, SwsFilter *dstFilter);

void sws_freeContext(struct SwsContext *swsContext);
struct SwsContext *sws_getContext(int srcW, int srcH, enum AVPixelFormat srcFormat,
                                  int dstW, int dstH, enum AVPixelFormat dstFormat,
                                  int flags, SwsFilter *srcFilter,
                                  SwsFilter *dstFilter, const double *param);
int sws_scale(struct SwsContext *c, const uint8_t *const srcSlice[],
              const int srcStride[], int srcSliceY, int srcSliceH,
              uint8_t *const dst[], const int dstStride[]);
int sws_setColorspaceDetails(struct SwsContext *c, const int inv_table[4],
                             int srcRange, const int table[4], int dstRange,
                             int brightness, int contrast, int saturation);

int sws_getColorspaceDetails(struct SwsContext *c, int **inv_table,
                             int *srcRange, int **table, int *dstRange,
                             int *brightness, int *contrast, int *saturation);

SwsVector *sws_allocVec(int length);

SwsVector *sws_getGaussianVec(double variance, double quality);

void sws_scaleVec(SwsVector *a, double scalar);

void sws_normalizeVec(SwsVector *a, double height);
 SwsVector *sws_getConstVec(double c, int length);
 SwsVector *sws_getIdentityVec(void);
 void sws_convVec(SwsVector *a, SwsVector *b);
 void sws_addVec(SwsVector *a, SwsVector *b);
 void sws_subVec(SwsVector *a, SwsVector *b);
 void sws_shiftVec(SwsVector *a, int shift);
 SwsVector *sws_cloneVec(SwsVector *a);
 void sws_printVec2(SwsVector *a, AVClass *log_ctx, int log_level);

void sws_freeVec(SwsVector *a);

SwsFilter *sws_getDefaultFilter(float lumaGBlur, float chromaGBlur,
                                float lumaSharpen, float chromaSharpen,
                                float chromaHShift, float chromaVShift,
                                int verbose);
void sws_freeFilter(SwsFilter *filter);
struct SwsContext *sws_getCachedContext(struct SwsContext *context,
                                        int srcW, int srcH, enum AVPixelFormat srcFormat,
                                        int dstW, int dstH, enum AVPixelFormat dstFormat,
                                        int flags, SwsFilter *srcFilter,
                                        SwsFilter *dstFilter, const double *param);
void sws_convertPalette8ToPacked32(const uint8_t *src, uint8_t *dst, int num_pixels, const uint8_t *palette);
void sws_convertPalette8ToPacked24(const uint8_t *src, uint8_t *dst, int num_pixels, const uint8_t *palette);

const AVClass *sws_get_class(void);
#define AVCODEC_AVFFT_H 
typedef float FFTSample;

typedef struct FFTComplex {
    FFTSample re, im;
} FFTComplex;

typedef struct FFTContext FFTContext;

FFTContext *av_fft_init(int nbits, int inverse);

void av_fft_permute(FFTContext *s, FFTComplex *z);

void av_fft_calc(FFTContext *s, FFTComplex *z);

void av_fft_end(FFTContext *s);

FFTContext *av_mdct_init(int nbits, int inverse, double scale);
void av_imdct_calc(FFTContext *s, FFTSample *output, const FFTSample *input);
void av_imdct_half(FFTContext *s, FFTSample *output, const FFTSample *input);
void av_mdct_calc(FFTContext *s, FFTSample *output, const FFTSample *input);
void av_mdct_end(FFTContext *s);

enum RDFTransformType {
    DFT_R2C,
    IDFT_C2R,
    IDFT_R2C,
    DFT_C2R,
};

typedef struct RDFTContext RDFTContext;

RDFTContext *av_rdft_init(int nbits, enum RDFTransformType trans);
void av_rdft_calc(RDFTContext *s, FFTSample *data);
void av_rdft_end(RDFTContext *s);

typedef struct DCTContext DCTContext;

enum DCTTransformType {
    DCT_II = 0,
    DCT_III,
    DCT_I,
    DST_I,
};
DCTContext *av_dct_init(int nbits, enum DCTTransformType type);
void av_dct_calc(DCTContext *s, FFTSample *data);
void av_dct_end (DCTContext *s);
#define SWRESAMPLE_SWRESAMPLE_H 
#define SWRESAMPLE_VERSION_H 
#define LIBSWRESAMPLE_VERSION_MAJOR 3
#define LIBSWRESAMPLE_VERSION_MINOR 8
#define LIBSWRESAMPLE_VERSION_MICRO 100
#define LIBSWRESAMPLE_VERSION_INT AV_VERSION_INT(LIBSWRESAMPLE_VERSION_MAJOR, LIBSWRESAMPLE_VERSION_MINOR, LIBSWRESAMPLE_VERSION_MICRO)
#define LIBSWRESAMPLE_VERSION AV_VERSION(LIBSWRESAMPLE_VERSION_MAJOR, LIBSWRESAMPLE_VERSION_MINOR, LIBSWRESAMPLE_VERSION_MICRO)
#define LIBSWRESAMPLE_BUILD LIBSWRESAMPLE_VERSION_INT
#define LIBSWRESAMPLE_IDENT "SwR" AV_STRINGIFY(LIBSWRESAMPLE_VERSION)
#define SWR_FLAG_RESAMPLE 1

enum SwrDitherType {
    SWR_DITHER_NONE = 0,
    SWR_DITHER_RECTANGULAR,
    SWR_DITHER_TRIANGULAR,
    SWR_DITHER_TRIANGULAR_HIGHPASS,
    SWR_DITHER_NS = 64,
    SWR_DITHER_NS_LIPSHITZ,
    SWR_DITHER_NS_F_WEIGHTED,
    SWR_DITHER_NS_MODIFIED_E_WEIGHTED,
    SWR_DITHER_NS_IMPROVED_E_WEIGHTED,
    SWR_DITHER_NS_SHIBATA,
    SWR_DITHER_NS_LOW_SHIBATA,
    SWR_DITHER_NS_HIGH_SHIBATA,
    SWR_DITHER_NB,
};

enum SwrEngine {
    SWR_ENGINE_SWR,
    SWR_ENGINE_SOXR,
    SWR_ENGINE_NB,
};

enum SwrFilterType {
    SWR_FILTER_TYPE_CUBIC,
    SWR_FILTER_TYPE_BLACKMAN_NUTTALL,
    SWR_FILTER_TYPE_KAISER,
};
typedef struct SwrContext SwrContext;
const AVClass *swr_get_class(void);
struct SwrContext *swr_alloc(void);
int swr_init(struct SwrContext *s);
int swr_is_initialized(struct SwrContext *s);
struct SwrContext *swr_alloc_set_opts(struct SwrContext *s,
                                      int64_t out_ch_layout, enum AVSampleFormat out_sample_fmt, int out_sample_rate,
                                      int64_t in_ch_layout, enum AVSampleFormat in_sample_fmt, int in_sample_rate,
                                      int log_offset, void *log_ctx);
void swr_free(struct SwrContext **s);
void swr_close(struct SwrContext *s);
int swr_convert(struct SwrContext *s, uint8_t **out, int out_count,
                                const uint8_t **in , int in_count);
int64_t swr_next_pts(struct SwrContext *s, int64_t pts);
int swr_set_compensation(struct SwrContext *s, int sample_delta, int compensation_distance);
int swr_set_channel_mapping(struct SwrContext *s, const int *channel_map);
int swr_build_matrix(uint64_t in_layout, uint64_t out_layout,
                     double center_mix_level, double surround_mix_level,
                     double lfe_mix_level, double rematrix_maxval,
                     double rematrix_volume, double *matrix,
                     int stride, enum AVMatrixEncoding matrix_encoding,
                     void *log_ctx);
int swr_set_matrix(struct SwrContext *s, const double *matrix, int stride);
int swr_drop_output(struct SwrContext *s, int count);
int swr_inject_silence(struct SwrContext *s, int count);
int64_t swr_get_delay(struct SwrContext *s, int64_t base);
int swr_get_out_samples(struct SwrContext *s, int in_samples);
unsigned swresample_version(void);

const char *swresample_configuration(void);

const char *swresample_license(void);
int swr_convert_frame(SwrContext *swr,
                      AVFrame *output, const AVFrame *input);
int swr_config_frame(SwrContext *swr, const AVFrame *out, const AVFrame *in);
#define AVFILTER_AVFILTER_H 
#define AVFILTER_VERSION_H 
#define LIBAVFILTER_VERSION_MAJOR 7
#define LIBAVFILTER_VERSION_MINOR 88
#define LIBAVFILTER_VERSION_MICRO 100
#define LIBAVFILTER_VERSION_INT AV_VERSION_INT(LIBAVFILTER_VERSION_MAJOR, LIBAVFILTER_VERSION_MINOR, LIBAVFILTER_VERSION_MICRO)
#define LIBAVFILTER_VERSION AV_VERSION(LIBAVFILTER_VERSION_MAJOR, LIBAVFILTER_VERSION_MINOR, LIBAVFILTER_VERSION_MICRO)
#define LIBAVFILTER_BUILD LIBAVFILTER_VERSION_INT
#define LIBAVFILTER_IDENT "Lavfi" AV_STRINGIFY(LIBAVFILTER_VERSION)
#define FF_API_OLD_FILTER_OPTS_ERROR (LIBAVFILTER_VERSION_MAJOR < 8)
#define FF_API_LAVR_OPTS (LIBAVFILTER_VERSION_MAJOR < 8)
#define FF_API_FILTER_GET_SET (LIBAVFILTER_VERSION_MAJOR < 8)
#define FF_API_SWS_PARAM_OPTION (LIBAVFILTER_VERSION_MAJOR < 8)

unsigned avfilter_version(void);

const char *avfilter_configuration(void);

const char *avfilter_license(void);

typedef struct AVFilterContext AVFilterContext;
typedef struct AVFilterLink AVFilterLink;
typedef struct AVFilterPad AVFilterPad;
typedef struct AVFilterFormats AVFilterFormats;
typedef struct AVFilterChannelLayouts AVFilterChannelLayouts;

int avfilter_pad_count(const AVFilterPad *pads);
const char *avfilter_pad_get_name(const AVFilterPad *pads, int pad_idx);
enum AVMediaType avfilter_pad_get_type(const AVFilterPad *pads, int pad_idx);
#define AVFILTER_FLAG_DYNAMIC_INPUTS (1 << 0)
#define AVFILTER_FLAG_DYNAMIC_OUTPUTS (1 << 1)
#define AVFILTER_FLAG_SLICE_THREADS (1 << 2)
#define AVFILTER_FLAG_SUPPORT_TIMELINE_GENERIC (1 << 16)
#define AVFILTER_FLAG_SUPPORT_TIMELINE_INTERNAL (1 << 17)
#define AVFILTER_FLAG_SUPPORT_TIMELINE (AVFILTER_FLAG_SUPPORT_TIMELINE_GENERIC | AVFILTER_FLAG_SUPPORT_TIMELINE_INTERNAL)

typedef struct AVFilter {
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
    int priv_size;
    int flags_internal;
    struct AVFilter *next;
    int (*process_command)(AVFilterContext *, const char *cmd, const char *arg, char *res, int res_len, int flags);
    int (*init_opaque)(AVFilterContext *ctx, void *opaque);
    int (*activate)(AVFilterContext *ctx);
} AVFilter;
#define AVFILTER_THREAD_SLICE (1 << 0)

typedef struct AVFilterInternal AVFilterInternal;

struct AVFilterContext {
    const AVClass *av_class;
    const AVFilter *filter;
    char *name;
    AVFilterPad *input_pads;
    AVFilterLink **inputs;
    unsigned nb_inputs;
    AVFilterPad *output_pads;
    AVFilterLink **outputs;
    unsigned nb_outputs;
    void *priv;
    struct AVFilterGraph *graph;
    int thread_type;
    AVFilterInternal *internal;
    struct AVFilterCommand *command_queue;
    char *enable_str;
    void *enable;
    double *var_values;
    int is_disabled;
    AVBufferRef *hw_device_ctx;
    int nb_threads;
    unsigned ready;
    int extra_hw_frames;
};
typedef struct AVFilterFormatsConfig {
    AVFilterFormats *formats;
    AVFilterFormats *samplerates;
    AVFilterChannelLayouts *channel_layouts;
} AVFilterFormatsConfig;
struct AVFilterLink {
    AVFilterContext *src;
    AVFilterPad *srcpad;
    AVFilterContext *dst;
    AVFilterPad *dstpad;
    enum AVMediaType type;
    int w;
    int h;
    AVRational sample_aspect_ratio;
    uint64_t channel_layout;
    int sample_rate;
    int format;
    AVRational time_base;
    AVFilterFormatsConfig incfg;
    AVFilterFormatsConfig outcfg;
    enum {
        AVLINK_UNINIT = 0,
        AVLINK_STARTINIT,
        AVLINK_INIT
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
    char reserved[0xF000];
};
int avfilter_link(AVFilterContext *src, unsigned srcpad,
                  AVFilterContext *dst, unsigned dstpad);

void avfilter_link_free(AVFilterLink **link);

int avfilter_link_get_channels(AVFilterLink *link);

void avfilter_link_set_closed(AVFilterLink *link, int closed);

int avfilter_config_links(AVFilterContext *filter);
#define AVFILTER_CMD_FLAG_ONE 1
#define AVFILTER_CMD_FLAG_FAST 2

int avfilter_process_command(AVFilterContext *filter, const char *cmd, const char *arg, char *res, int res_len, int flags);
const AVFilter *av_filter_iterate(void **opaque);

void avfilter_register_all(void);

int avfilter_register(AVFilter *filter);

const AVFilter *avfilter_next(const AVFilter *prev);
const AVFilter *avfilter_get_by_name(const char *name);
int avfilter_init_str(AVFilterContext *ctx, const char *args);
int avfilter_init_dict(AVFilterContext *ctx, AVDictionary **options);

void avfilter_free(AVFilterContext *filter);
int avfilter_insert_filter(AVFilterLink *link, AVFilterContext *filt,
                           unsigned filt_srcpad_idx, unsigned filt_dstpad_idx);

const AVClass *avfilter_get_class(void);

typedef struct AVFilterGraphInternal AVFilterGraphInternal;
typedef int (avfilter_action_func)(AVFilterContext *ctx, void *arg, int jobnr, int nb_jobs);
typedef int (avfilter_execute_func)(AVFilterContext *ctx, avfilter_action_func *func,
                                    void *arg, int *ret, int nb_jobs);

typedef struct AVFilterGraph {
    const AVClass *av_class;
    AVFilterContext **filters;
    unsigned nb_filters;
    char *scale_sws_opts;
    char *resample_lavr_opts;
    int thread_type;
    int nb_threads;
    AVFilterGraphInternal *internal;
    void *opaque;
    avfilter_execute_func *execute;
    char *aresample_swr_opts;
    AVFilterLink **sink_links;
    int sink_links_count;
    unsigned disable_auto_convert;
} AVFilterGraph;

AVFilterGraph *avfilter_graph_alloc(void);
AVFilterContext *avfilter_graph_alloc_filter(AVFilterGraph *graph,
                                             const AVFilter *filter,
                                             const char *name);
AVFilterContext *avfilter_graph_get_filter(AVFilterGraph *graph, const char *name);
int avfilter_graph_create_filter(AVFilterContext **filt_ctx, const AVFilter *filt,
                                 const char *name, const char *args, void *opaque,
                                 AVFilterGraph *graph_ctx);
void avfilter_graph_set_auto_convert(AVFilterGraph *graph, unsigned flags);

enum {
    AVFILTER_AUTO_CONVERT_ALL = 0,
    AVFILTER_AUTO_CONVERT_NONE = -1,
};
int avfilter_graph_config(AVFilterGraph *graphctx, void *log_ctx);

void avfilter_graph_free(AVFilterGraph **graph);
typedef struct AVFilterInOut {
    char *name;
    AVFilterContext *filter_ctx;
    int pad_idx;
    struct AVFilterInOut *next;
} AVFilterInOut;

AVFilterInOut *avfilter_inout_alloc(void);

void avfilter_inout_free(AVFilterInOut **inout);
int avfilter_graph_parse(AVFilterGraph *graph, const char *filters,
                         AVFilterInOut *inputs, AVFilterInOut *outputs,
                         void *log_ctx);
int avfilter_graph_parse_ptr(AVFilterGraph *graph, const char *filters,
                             AVFilterInOut **inputs, AVFilterInOut **outputs,
                             void *log_ctx);
int avfilter_graph_parse2(AVFilterGraph *graph, const char *filters,
                          AVFilterInOut **inputs,
                          AVFilterInOut **outputs);
int avfilter_graph_send_command(AVFilterGraph *graph, const char *target, const char *cmd, const char *arg, char *res, int res_len, int flags);
int avfilter_graph_queue_command(AVFilterGraph *graph, const char *target, const char *cmd, const char *arg, int flags, double ts);
char *avfilter_graph_dump(AVFilterGraph *graph, const char *options);
int avfilter_graph_request_oldest(AVFilterGraph *graph);


int av_buffersink_get_frame_flags(AVFilterContext *ctx, AVFrame *frame, int flags);
#define AV_BUFFERSINK_FLAG_PEEK 1
#define AV_BUFFERSINK_FLAG_NO_REQUEST 2

typedef struct AVBufferSinkParams {
    const enum AVPixelFormat *pixel_fmts;
} AVBufferSinkParams;

AVBufferSinkParams *av_buffersink_params_alloc(void);

typedef struct AVABufferSinkParams {
    const enum AVSampleFormat *sample_fmts;
    const int64_t *channel_layouts;
    const int *channel_counts;
    int all_channel_counts;
    int *sample_rates;
} AVABufferSinkParams;

AVABufferSinkParams *av_abuffersink_params_alloc(void);
void av_buffersink_set_frame_size(AVFilterContext *ctx, unsigned frame_size);

enum AVMediaType av_buffersink_get_type (const AVFilterContext *ctx);
AVRational av_buffersink_get_time_base (const AVFilterContext *ctx);
int av_buffersink_get_format (const AVFilterContext *ctx);

AVRational av_buffersink_get_frame_rate (const AVFilterContext *ctx);
int av_buffersink_get_w (const AVFilterContext *ctx);
int av_buffersink_get_h (const AVFilterContext *ctx);
AVRational av_buffersink_get_sample_aspect_ratio (const AVFilterContext *ctx);

int av_buffersink_get_channels (const AVFilterContext *ctx);
uint64_t av_buffersink_get_channel_layout (const AVFilterContext *ctx);
int av_buffersink_get_sample_rate (const AVFilterContext *ctx);

AVBufferRef * av_buffersink_get_hw_frames_ctx (const AVFilterContext *ctx);
int av_buffersink_get_frame(AVFilterContext *ctx, AVFrame *frame);
int av_buffersink_get_samples(AVFilterContext *ctx, AVFrame *frame, int nb_samples);
enum {
    AV_BUFFERSRC_FLAG_NO_CHECK_FORMAT = 1,
    AV_BUFFERSRC_FLAG_PUSH = 4,
    AV_BUFFERSRC_FLAG_KEEP_REF = 8,
};
unsigned av_buffersrc_get_nb_failed_requests(AVFilterContext *buffer_src);
typedef struct AVBufferSrcParameters {
    int format;
    AVRational time_base;
    int width, height;
    AVRational sample_aspect_ratio;
    AVRational frame_rate;
    AVBufferRef *hw_frames_ctx;
    int sample_rate;
    uint64_t channel_layout;
} AVBufferSrcParameters;

AVBufferSrcParameters *av_buffersrc_parameters_alloc(void);
int av_buffersrc_parameters_set(AVFilterContext *ctx, AVBufferSrcParameters *param);

int av_buffersrc_write_frame(AVFilterContext *ctx, const AVFrame *frame);

int av_buffersrc_add_frame(AVFilterContext *ctx, AVFrame *frame);

int av_buffersrc_add_frame_flags(AVFilterContext *buffer_src,
                                 AVFrame *frame, int flags);
int av_buffersrc_close(AVFilterContext *ctx, int64_t pts, unsigned flags);

#define HAVE_WINAPIFAMILY_H 0
#define WINAPI_FAMILY_WINRT 0
#define __WINDOWS__ 1
#define __WIN32__ 1

#define SDL_DEPRECATED __attribute__((deprecated))
#define SDL_UNUSED __attribute__((unused))
#define DECLSPEC __declspec(dllexport)
#define SDLCALL 
#define SDL_INLINE __inline__
#define SDL_FORCE_INLINE __attribute__((always_inline)) static __inline__
#define SDL_NORETURN __attribute__((noreturn))
extern const char * SDL_GetPlatform (void);
#define HAVE_STDINT_H 1
#define SIZEOF_VOIDP 8
#define HAVE_DDRAW_H 1
#define HAVE_DINPUT_H 1
#define HAVE_DSOUND_H 1
#define HAVE_DXGI_H 1
#define HAVE_XINPUT_H 1
#define HAVE_MMDEVICEAPI_H 1
#define HAVE_AUDIOCLIENT_H 1
#define HAVE_STDARG_H 1
#define HAVE_STDDEF_H 1
#define SDL_AUDIO_DRIVER_WASAPI 1
#define SDL_AUDIO_DRIVER_DSOUND 1
#define SDL_AUDIO_DRIVER_WINMM 1
#define SDL_AUDIO_DRIVER_DISK 1
#define SDL_AUDIO_DRIVER_DUMMY 1
#define SDL_JOYSTICK_DINPUT 1
#define SDL_JOYSTICK_XINPUT 1
#define SDL_JOYSTICK_HIDAPI 1
#define SDL_HAPTIC_DINPUT 1
#define SDL_HAPTIC_XINPUT 1
#define SDL_SENSOR_DUMMY 1
#define SDL_LOADSO_WINDOWS 1
#define SDL_THREAD_WINDOWS 1
#define SDL_TIMER_WINDOWS 1
#define SDL_VIDEO_DRIVER_DUMMY 1
#define SDL_VIDEO_DRIVER_WINDOWS 1
#define SDL_VIDEO_RENDER_D3D 1
#define SDL_VIDEO_RENDER_D3D11 0
#define SDL_VIDEO_OPENGL 1
#define SDL_VIDEO_OPENGL_WGL 1
#define SDL_VIDEO_RENDER_OGL 1
#define SDL_VIDEO_RENDER_OGL_ES2 1
#define SDL_VIDEO_OPENGL_ES2 1
#define SDL_VIDEO_OPENGL_EGL 1
#define SDL_VIDEO_VULKAN 1
#define SDL_POWER_WINDOWS 1
#define SDL_FILESYSTEM_WINDOWS 1
#define SDL_arraysize(array) (sizeof(array)/sizeof(array[0]))
#define SDL_TABLESIZE(table) SDL_arraysize(table)
#define SDL_STRINGIFY_ARG(arg) #arg
#define SDL_reinterpret_cast(type,expression) ((type)(expression))
#define SDL_static_cast(type,expression) ((type)(expression))
#define SDL_const_cast(type,expression) ((type)(expression))
#define SDL_FOURCC(A,B,C,D) ((SDL_static_cast(Uint32, SDL_static_cast(Uint8, (A))) << 0) | (SDL_static_cast(Uint32, SDL_static_cast(Uint8, (B))) << 8) | (SDL_static_cast(Uint32, SDL_static_cast(Uint8, (C))) << 16) | (SDL_static_cast(Uint32, SDL_static_cast(Uint8, (D))) << 24))
typedef enum
{
    SDL_FALSE = 0,
    SDL_TRUE = 1
} SDL_bool;
#define SDL_MAX_SINT8 ((Sint8)0x7F)
#define SDL_MIN_SINT8 ((Sint8)(~0x7F))
typedef int8_t Sint8;
#define SDL_MAX_UINT8 ((Uint8)0xFF)
#define SDL_MIN_UINT8 ((Uint8)0x00)
typedef uint8_t Uint8;
#define SDL_MAX_SINT16 ((Sint16)0x7FFF)
#define SDL_MIN_SINT16 ((Sint16)(~0x7FFF))
typedef int16_t Sint16;
#define SDL_MAX_UINT16 ((Uint16)0xFFFF)
#define SDL_MIN_UINT16 ((Uint16)0x0000)
typedef uint16_t Uint16;
#define SDL_MAX_SINT32 ((Sint32)0x7FFFFFFF)
#define SDL_MIN_SINT32 ((Sint32)(~0x7FFFFFFF))
typedef int32_t Sint32;
#define SDL_MAX_UINT32 ((Uint32)0xFFFFFFFFu)
#define SDL_MIN_UINT32 ((Uint32)0x00000000)
typedef uint32_t Uint32;
#define SDL_MAX_SINT64 ((Sint64)0x7FFFFFFFFFFFFFFFll)
#define SDL_MIN_SINT64 ((Sint64)(~0x7FFFFFFFFFFFFFFFll))
typedef int64_t Sint64;
#define SDL_MAX_UINT64 ((Uint64)0xFFFFFFFFFFFFFFFFull)
#define SDL_MIN_UINT64 ((Uint64)(0x0000000000000000ull))
typedef uint64_t Uint64;
#define SDL_PRIs64 "I64d"
#define SDL_PRIu64 PRIu64
#define SDL_PRIx64 PRIx64
#define SDL_PRIX64 PRIX64
#define SDL_IN_BYTECAP(x) 
#define SDL_INOUT_Z_CAP(x) 
#define SDL_OUT_Z_CAP(x) 
#define SDL_OUT_CAP(x) 
#define SDL_OUT_BYTECAP(x) 
#define SDL_OUT_Z_BYTECAP(x) 
#define SDL_PRINTF_FORMAT_STRING 
#define SDL_SCANF_FORMAT_STRING 
#define SDL_PRINTF_VARARG_FUNC(fmtargnumber) __attribute__ (( format( __printf__, fmtargnumber, fmtargnumber+1 )))
#define SDL_SCANF_VARARG_FUNC(fmtargnumber) __attribute__ (( format( __scanf__, fmtargnumber, fmtargnumber+1 )))
#define SDL_COMPILE_TIME_ASSERT(name,x) typedef int SDL_compile_time_assert_ ## name[(x) * 2 - 1]

typedef int SDL_compile_time_assert_uint8[(sizeof(Uint8) == 1) * 2 - 1];
typedef int SDL_compile_time_assert_sint8[(sizeof(Sint8) == 1) * 2 - 1];
typedef int SDL_compile_time_assert_uint16[(sizeof(Uint16) == 2) * 2 - 1];
typedef int SDL_compile_time_assert_sint16[(sizeof(Sint16) == 2) * 2 - 1];
typedef int SDL_compile_time_assert_uint32[(sizeof(Uint32) == 4) * 2 - 1];
typedef int SDL_compile_time_assert_sint32[(sizeof(Sint32) == 4) * 2 - 1];
typedef int SDL_compile_time_assert_uint64[(sizeof(Uint64) == 8) * 2 - 1];
typedef int SDL_compile_time_assert_sint64[(sizeof(Sint64) == 8) * 2 - 1];
typedef enum
{
    DUMMY_ENUM_VALUE
} SDL_DUMMY_ENUM;

typedef int SDL_compile_time_assert_enum[(sizeof(SDL_DUMMY_ENUM) == sizeof(int)) * 2 - 1];

#define SDL_stack_alloc(type,count) (type*)SDL_malloc(sizeof(type)*(count))
#define SDL_stack_free(data) SDL_free(data)

extern void * SDL_malloc(size_t size);
extern void * SDL_calloc(size_t nmemb, size_t size);
extern void * SDL_realloc(void *mem, size_t size);
extern void SDL_free(void *mem);

typedef void *( *SDL_malloc_func)(size_t size);
typedef void *( *SDL_calloc_func)(size_t nmemb, size_t size);
typedef void *( *SDL_realloc_func)(void *mem, size_t size);
typedef void ( *SDL_free_func)(void *mem);

extern void SDL_GetMemoryFunctions(SDL_malloc_func *malloc_func,
                                                    SDL_calloc_func *calloc_func,
                                                    SDL_realloc_func *realloc_func,
                                                    SDL_free_func *free_func);
extern int SDL_SetMemoryFunctions(SDL_malloc_func malloc_func,
                                                   SDL_calloc_func calloc_func,
                                                   SDL_realloc_func realloc_func,
                                                   SDL_free_func free_func);

extern int SDL_GetNumAllocations(void);

extern char * SDL_getenv(const char *name);
extern int SDL_setenv(const char *name, const char *value, int overwrite);

extern void SDL_qsort(void *base, size_t nmemb, size_t size, int (*compare) (const void *, const void *));

extern int SDL_abs(int x);
#define SDL_min(x,y) (((x) < (y)) ? (x) : (y))
#define SDL_max(x,y) (((x) > (y)) ? (x) : (y))

extern int SDL_isdigit(int x);
extern int SDL_isspace(int x);
extern int SDL_isupper(int x);
extern int SDL_islower(int x);
extern int SDL_toupper(int x);
extern int SDL_tolower(int x);

extern void * SDL_memset( void *dst, int c, size_t len);
#define SDL_zero(x) SDL_memset(&(x), 0, sizeof((x)))
#define SDL_zerop(x) SDL_memset((x), 0, sizeof(*(x)))
#define SDL_zeroa(x) SDL_memset((x), 0, sizeof((x)))
 static inline void SDL_memset4(void *dst, Uint32 val, size_t dwords)
{
    size_t _n = (dwords + 3) / 4;
    Uint32 *_p = ((Uint32 *)(dst));
    Uint32 _val = (val);
    if (dwords == 0)
        return;
    switch (dwords % 4)
    {
        case 0: do { *_p++ = _val;
        case 3: *_p++ = _val;
        case 2: *_p++ = _val;
        case 1: *_p++ = _val;
        } while ( --_n );
    }
}

extern void * SDL_memcpy( void *dst, const void *src, size_t len);

extern void * SDL_memmove( void *dst, const void *src, size_t len);
extern int SDL_memcmp(const void *s1, const void *s2, size_t len);

extern size_t SDL_wcslen(const wchar_t *wstr);
extern size_t SDL_wcslcpy( wchar_t *dst, const wchar_t *src, size_t maxlen);
extern size_t SDL_wcslcat( wchar_t *dst, const wchar_t *src, size_t maxlen);
extern wchar_t * SDL_wcsdup(const wchar_t *wstr);
extern wchar_t * SDL_wcsstr(const wchar_t *haystack, const wchar_t *needle);

extern int SDL_wcscmp(const wchar_t *str1, const wchar_t *str2);
extern int SDL_wcsncmp(const wchar_t *str1, const wchar_t *str2, size_t maxlen);

extern size_t SDL_strlen(const char *str);
extern size_t SDL_strlcpy( char *dst, const char *src, size_t maxlen);
extern size_t SDL_utf8strlcpy( char *dst, const char *src, size_t dst_bytes);
extern size_t SDL_strlcat( char *dst, const char *src, size_t maxlen);
extern char * SDL_strdup(const char *str);
extern char * SDL_strrev(char *str);
extern char * SDL_strupr(char *str);
extern char * SDL_strlwr(char *str);
extern char * SDL_strchr(const char *str, int c);
extern char * SDL_strrchr(const char *str, int c);
extern char * SDL_strstr(const char *haystack, const char *needle);
extern char * SDL_strtokr(char *s1, const char *s2, char **saveptr);
extern size_t SDL_utf8strlen(const char *str);

extern char * SDL_itoa(int value, char *str, int radix);
extern char * SDL_uitoa(unsigned int value, char *str, int radix);
extern char * SDL_ltoa(long value, char *str, int radix);
extern char * SDL_ultoa(unsigned long value, char *str, int radix);
extern char * SDL_lltoa(Sint64 value, char *str, int radix);
extern char * SDL_ulltoa(Uint64 value, char *str, int radix);

extern int SDL_atoi(const char *str);
extern double SDL_atof(const char *str);
extern long SDL_strtol(const char *str, char **endp, int base);
extern unsigned long SDL_strtoul(const char *str, char **endp, int base);
extern Sint64 SDL_strtoll(const char *str, char **endp, int base);
extern Uint64 SDL_strtoull(const char *str, char **endp, int base);
extern double SDL_strtod(const char *str, char **endp);

extern int SDL_strcmp(const char *str1, const char *str2);
extern int SDL_strncmp(const char *str1, const char *str2, size_t maxlen);
extern int SDL_strcasecmp(const char *str1, const char *str2);
extern int SDL_strncasecmp(const char *str1, const char *str2, size_t len);

extern int SDL_sscanf(const char *text, const char *fmt, ...) ;
extern int SDL_vsscanf(const char *text, const char *fmt, va_list ap);
extern int SDL_snprintf( char *text, size_t maxlen, const char *fmt, ... ) ;
extern int SDL_vsnprintf( char *text, size_t maxlen, const char *fmt, va_list ap);

extern double SDL_acos(double x);
extern float SDL_acosf(float x);
extern double SDL_asin(double x);
extern float SDL_asinf(float x);
extern double SDL_atan(double x);
extern float SDL_atanf(float x);
extern double SDL_atan2(double x, double y);
extern float SDL_atan2f(float x, float y);
extern double SDL_ceil(double x);
extern float SDL_ceilf(float x);
extern double SDL_copysign(double x, double y);
extern float SDL_copysignf(float x, float y);
extern double SDL_cos(double x);
extern float SDL_cosf(float x);
extern double SDL_exp(double x);
extern float SDL_expf(float x);
extern double SDL_fabs(double x);
extern float SDL_fabsf(float x);
extern double SDL_floor(double x);
extern float SDL_floorf(float x);
extern double SDL_fmod(double x, double y);
extern float SDL_fmodf(float x, float y);
extern double SDL_log(double x);
extern float SDL_logf(float x);
extern double SDL_log10(double x);
extern float SDL_log10f(float x);
extern double SDL_pow(double x, double y);
extern float SDL_powf(float x, float y);
extern double SDL_scalbn(double x, int n);
extern float SDL_scalbnf(float x, int n);
extern double SDL_sin(double x);
extern float SDL_sinf(float x);
extern double SDL_sqrt(double x);
extern float SDL_sqrtf(float x);
extern double SDL_tan(double x);
extern float SDL_tanf(float x);
#define SDL_ICONV_ERROR (size_t)-1
#define SDL_ICONV_E2BIG (size_t)-2
#define SDL_ICONV_EILSEQ (size_t)-3
#define SDL_ICONV_EINVAL (size_t)-4

typedef struct _SDL_iconv_t *SDL_iconv_t;
extern SDL_iconv_t SDL_iconv_open(const char *tocode,
                                                   const char *fromcode);
extern int SDL_iconv_close(SDL_iconv_t cd);
extern size_t SDL_iconv(SDL_iconv_t cd, const char **inbuf,
                                         size_t * inbytesleft, char **outbuf,
                                         size_t * outbytesleft);

extern char * SDL_iconv_string(const char *tocode,
                                               const char *fromcode,
                                               const char *inbuf,
                                               size_t inbytesleft);
#define SDL_iconv_utf8_locale(S) SDL_iconv_string("", "UTF-8", S, SDL_strlen(S)+1)
#define SDL_iconv_utf8_ucs2(S) (Uint16 *)SDL_iconv_string("UCS-2-INTERNAL", "UTF-8", S, SDL_strlen(S)+1)
#define SDL_iconv_utf8_ucs4(S) (Uint32 *)SDL_iconv_string("UCS-4-INTERNAL", "UTF-8", S, SDL_strlen(S)+1)
 static inline void *SDL_memcpy4( void *dst, const void *src, size_t dwords)
{
    return SDL_memcpy(dst, src, dwords * 4);
}
#define SDL_MAIN_AVAILABLE 
#define SDLMAIN_DECLSPEC 
#define main SDL_main


typedef int (*SDL_main_func)(int argc, char *argv[]);
extern int SDL_main(int argc, char *argv[]);
extern void SDL_SetMainReady(void);

extern int SDL_RegisterApp(char *name, Uint32 style, void *hInst);
extern void SDL_UnregisterApp(void);
#define SDL_assert_h_ 

#define SDL_ASSERT_LEVEL 2
#define SDL_TriggerBreakpoint() __asm__ __volatile__ ( "int $3\n\t" )
#define SDL_FUNCTION __func__
#define SDL_FILE __FILE__
#define SDL_LINE __LINE__
#define SDL_NULL_WHILE_LOOP_CONDITION (0)
#define SDL_disabled_assert(condition) do { (void) sizeof ((condition)); } while (SDL_NULL_WHILE_LOOP_CONDITION)

typedef enum
{
    SDL_ASSERTION_RETRY,
    SDL_ASSERTION_BREAK,
    SDL_ASSERTION_ABORT,
    SDL_ASSERTION_IGNORE,
    SDL_ASSERTION_ALWAYS_IGNORE
} SDL_AssertState;

typedef struct SDL_AssertData
{
    int always_ignore;
    unsigned int trigger_count;
    const char *condition;
    const char *filename;
    int linenum;
    const char *function;
    const struct SDL_AssertData *next;
} SDL_AssertData;

extern SDL_AssertState SDL_ReportAssertion(SDL_AssertData *,
                                                             const char *,
                                                             const char *, int)
;
#define SDL_enabled_assert(condition) do { while ( !(condition) ) { static struct SDL_AssertData sdl_assert_data = { 0, 0, #condition, 0, 0, 0, 0 }; const SDL_AssertState sdl_assert_state = SDL_ReportAssertion(&sdl_assert_data, SDL_FUNCTION, SDL_FILE, SDL_LINE); if (sdl_assert_state == SDL_ASSERTION_RETRY) { continue; } else if (sdl_assert_state == SDL_ASSERTION_BREAK) { SDL_TriggerBreakpoint(); } break; } } while (SDL_NULL_WHILE_LOOP_CONDITION)
#define SDL_assert(condition) SDL_enabled_assert(condition)
#define SDL_assert_release(condition) SDL_enabled_assert(condition)
#define SDL_assert_paranoid(condition) SDL_disabled_assert(condition)
#define SDL_assert_always(condition) SDL_enabled_assert(condition)

typedef SDL_AssertState ( *SDL_AssertionHandler)(
                                 const SDL_AssertData* data, void* userdata);
extern void SDL_SetAssertionHandler(
                                            SDL_AssertionHandler handler,
                                            void *userdata);
extern SDL_AssertionHandler SDL_GetDefaultAssertionHandler(void);
extern SDL_AssertionHandler SDL_GetAssertionHandler(void **puserdata);
extern const SDL_AssertData * SDL_GetAssertionReport(void);
extern void SDL_ResetAssertionReport(void);
#define SDL_assert_state SDL_AssertState
#define SDL_assert_data SDL_AssertData
#define SDL_atomic_h_ 

typedef int SDL_SpinLock;
extern SDL_bool SDL_AtomicTryLock(SDL_SpinLock *lock);

extern void SDL_AtomicLock(SDL_SpinLock *lock);

extern void SDL_AtomicUnlock(SDL_SpinLock *lock);
#define SDL_CompilerBarrier() __asm__ __volatile__ ("" : : : "memory")
extern void SDL_MemoryBarrierReleaseFunction(void);
extern void SDL_MemoryBarrierAcquireFunction(void);
#define SDL_MemoryBarrierRelease() SDL_CompilerBarrier()
#define SDL_MemoryBarrierAcquire() SDL_CompilerBarrier()

typedef struct { int value; } SDL_atomic_t;
extern SDL_bool SDL_AtomicCAS(SDL_atomic_t *a, int oldval, int newval);

extern int SDL_AtomicSet(SDL_atomic_t *a, int v);

extern int SDL_AtomicGet(SDL_atomic_t *a);
extern int SDL_AtomicAdd(SDL_atomic_t *a, int v);
#define SDL_AtomicIncRef(a) SDL_AtomicAdd(a, 1)
#define SDL_AtomicDecRef(a) (SDL_AtomicAdd(a, -1) == 1)
extern SDL_bool SDL_AtomicCASPtr(void **a, void *oldval, void *newval);

extern void* SDL_AtomicSetPtr(void **a, void* v);

extern void* SDL_AtomicGetPtr(void **a);
#define SDL_audio_h_ 
#define SDL_error_h_ 


extern int SDL_SetError( const char *fmt, ...) ;
extern const char * SDL_GetError(void);
extern void SDL_ClearError(void);
#define SDL_OutOfMemory() SDL_Error(SDL_ENOMEM)
#define SDL_Unsupported() SDL_Error(SDL_UNSUPPORTED)
#define SDL_InvalidParamError(param) SDL_SetError("Parameter '%s' is invalid", (param))
typedef enum
{
    SDL_ENOMEM,
    SDL_EFREAD,
    SDL_EFWRITE,
    SDL_EFSEEK,
    SDL_UNSUPPORTED,
    SDL_LASTERROR
} SDL_errorcode;

extern int SDL_Error(SDL_errorcode code);
#define SDL_endian_h_ 
#define SDL_LIL_ENDIAN 1234
#define SDL_BIG_ENDIAN 4321
#define SDL_BYTEORDER SDL_LIL_ENDIAN

 static inline Uint16
SDL_Swap16(Uint16 x)
{
  __asm__("xchgb %b0,%h0": "=Q"(x):"0"(x));
    return x;
}
 static inline Uint32
SDL_Swap32(Uint32 x)
{
  __asm__("bswapl %0": "=r"(x):"0"(x));
    return x;
}
 static inline Uint64
SDL_Swap64(Uint64 x)
{
  __asm__("bswapq %0": "=r"(x):"0"(x));
    return x;
}
 static inline float
SDL_SwapFloat(float x)
{
    union
    {
        float f;
        Uint32 ui32;
    } swapper;
    swapper.f = x;
    swapper.ui32 = SDL_Swap32(swapper.ui32);
    return swapper.f;
}
#define SDL_SwapLE16(X) (X)
#define SDL_SwapLE32(X) (X)
#define SDL_SwapLE64(X) (X)
#define SDL_SwapFloatLE(X) (X)
#define SDL_SwapBE16(X) SDL_Swap16(X)
#define SDL_SwapBE32(X) SDL_Swap32(X)
#define SDL_SwapBE64(X) SDL_Swap64(X)
#define SDL_SwapFloatBE(X) SDL_SwapFloat(X)
#define SDL_mutex_h_ 

#define SDL_MUTEX_TIMEDOUT 1
#define SDL_MUTEX_MAXWAIT (~(Uint32)0)
struct SDL_mutex;
typedef struct SDL_mutex SDL_mutex;

extern SDL_mutex * SDL_CreateMutex(void);
#define SDL_mutexP(m) SDL_LockMutex(m)
extern int SDL_LockMutex(SDL_mutex * mutex);

extern int SDL_TryLockMutex(SDL_mutex * mutex);
#define SDL_mutexV(m) SDL_UnlockMutex(m)
extern int SDL_UnlockMutex(SDL_mutex * mutex);

extern void SDL_DestroyMutex(SDL_mutex * mutex);
struct SDL_semaphore;
typedef struct SDL_semaphore SDL_sem;

extern SDL_sem * SDL_CreateSemaphore(Uint32 initial_value);

extern void SDL_DestroySemaphore(SDL_sem * sem);

extern int SDL_SemWait(SDL_sem * sem);

extern int SDL_SemTryWait(SDL_sem * sem);
extern int SDL_SemWaitTimeout(SDL_sem * sem, Uint32 ms);

extern int SDL_SemPost(SDL_sem * sem);

extern Uint32 SDL_SemValue(SDL_sem * sem);
struct SDL_cond;
typedef struct SDL_cond SDL_cond;
extern SDL_cond * SDL_CreateCond(void);

extern void SDL_DestroyCond(SDL_cond * cond);

extern int SDL_CondSignal(SDL_cond * cond);

extern int SDL_CondBroadcast(SDL_cond * cond);
extern int SDL_CondWait(SDL_cond * cond, SDL_mutex * mutex);
extern int SDL_CondWaitTimeout(SDL_cond * cond,
                                                SDL_mutex * mutex, Uint32 ms);
#define SDL_thread_h_ 


struct SDL_Thread;
typedef struct SDL_Thread SDL_Thread;

typedef unsigned long SDL_threadID;

typedef unsigned int SDL_TLSID;

typedef enum {
    SDL_THREAD_PRIORITY_LOW,
    SDL_THREAD_PRIORITY_NORMAL,
    SDL_THREAD_PRIORITY_HIGH,
    SDL_THREAD_PRIORITY_TIME_CRITICAL
} SDL_ThreadPriority;

typedef int ( * SDL_ThreadFunction) (void *data);
#define SDL_PASSED_BEGINTHREAD_ENDTHREAD 

typedef uintptr_t ( * pfnSDL_CurrentBeginThread)
                   (void *, unsigned, unsigned ( *func)(void *),
                    void * , unsigned, unsigned * );
typedef void ( * pfnSDL_CurrentEndThread) (unsigned code);
#define SDL_beginthread _beginthreadex
#define SDL_endthread _endthreadex

extern SDL_Thread *
SDL_CreateThread(SDL_ThreadFunction fn, const char *name, void *data,
                 pfnSDL_CurrentBeginThread pfnBeginThread,
                 pfnSDL_CurrentEndThread pfnEndThread);

extern SDL_Thread *
SDL_CreateThreadWithStackSize(int ( * fn) (void *),
                 const char *name, const size_t stacksize, void *data,
                 pfnSDL_CurrentBeginThread pfnBeginThread,
                 pfnSDL_CurrentEndThread pfnEndThread);
#define SDL_CreateThread(fn,name,data) SDL_CreateThread(fn, name, data, (pfnSDL_CurrentBeginThread)SDL_beginthread, (pfnSDL_CurrentEndThread)SDL_endthread)
#define SDL_CreateThreadWithStackSize(fn,name,stacksize,data) SDL_CreateThreadWithStackSize(fn, name, data, (pfnSDL_CurrentBeginThread)_beginthreadex, (pfnSDL_CurrentEndThread)SDL_endthread)
extern const char * SDL_GetThreadName(SDL_Thread *thread);

extern SDL_threadID SDL_ThreadID(void);

extern SDL_threadID SDL_GetThreadID(SDL_Thread * thread);

extern int SDL_SetThreadPriority(SDL_ThreadPriority priority);
extern void SDL_WaitThread(SDL_Thread * thread, int *status);
extern void SDL_DetachThread(SDL_Thread * thread);
extern SDL_TLSID SDL_TLSCreate(void);
extern void * SDL_TLSGet(SDL_TLSID id);
extern int SDL_TLSSet(SDL_TLSID id, const void *value, void ( *destructor)(void*));
#define SDL_rwops_h_ 

#define SDL_RWOPS_UNKNOWN 0U
#define SDL_RWOPS_WINFILE 1U
#define SDL_RWOPS_STDFILE 2U
#define SDL_RWOPS_JNIFILE 3U
#define SDL_RWOPS_MEMORY 4U
#define SDL_RWOPS_MEMORY_RO 5U

typedef struct SDL_RWops
{
    Sint64 ( * size) (struct SDL_RWops * context);
    Sint64 ( * seek) (struct SDL_RWops * context, Sint64 offset,
                             int whence);
    size_t ( * read) (struct SDL_RWops * context, void *ptr,
                             size_t size, size_t maxnum);
    size_t ( * write) (struct SDL_RWops * context, const void *ptr,
                              size_t size, size_t num);
    int ( * close) (struct SDL_RWops * context);
    Uint32 type;
    union
    {
        struct
        {
            SDL_bool append;
            void *h;
            struct
            {
                void *data;
                size_t size;
                size_t left;
            } buffer;
        } windowsio;
        struct
        {
            Uint8 *base;
            Uint8 *here;
            Uint8 *stop;
        } mem;
        struct
        {
            void *data1;
            void *data2;
        } unknown;
    } hidden;
} SDL_RWops;
extern SDL_RWops * SDL_RWFromFile(const char *file,
                                                  const char *mode);

extern SDL_RWops * SDL_RWFromFP(void * fp,
                                                SDL_bool autoclose);

extern SDL_RWops * SDL_RWFromMem(void *mem, int size);
extern SDL_RWops * SDL_RWFromConstMem(const void *mem,
                                                      int size);

extern SDL_RWops * SDL_AllocRW(void);
extern void SDL_FreeRW(SDL_RWops * area);
#define RW_SEEK_SET 0
#define RW_SEEK_CUR 1
#define RW_SEEK_END 2

extern Sint64 SDL_RWsize(SDL_RWops *context);

extern Sint64 SDL_RWseek(SDL_RWops *context,
                                          Sint64 offset, int whence);

extern Sint64 SDL_RWtell(SDL_RWops *context);

extern size_t SDL_RWread(SDL_RWops *context,
                                          void *ptr, size_t size, size_t maxnum);

extern size_t SDL_RWwrite(SDL_RWops *context,
                                           const void *ptr, size_t size, size_t num);

extern int SDL_RWclose(SDL_RWops *context);
extern void * SDL_LoadFile_RW(SDL_RWops * src, size_t *datasize,
                                                    int freesrc);
extern void * SDL_LoadFile(const char *file, size_t *datasize);

extern Uint8 SDL_ReadU8(SDL_RWops * src);
extern Uint16 SDL_ReadLE16(SDL_RWops * src);
extern Uint16 SDL_ReadBE16(SDL_RWops * src);
extern Uint32 SDL_ReadLE32(SDL_RWops * src);
extern Uint32 SDL_ReadBE32(SDL_RWops * src);
extern Uint64 SDL_ReadLE64(SDL_RWops * src);
extern Uint64 SDL_ReadBE64(SDL_RWops * src);
extern size_t SDL_WriteU8(SDL_RWops * dst, Uint8 value);
extern size_t SDL_WriteLE16(SDL_RWops * dst, Uint16 value);
extern size_t SDL_WriteBE16(SDL_RWops * dst, Uint16 value);
extern size_t SDL_WriteLE32(SDL_RWops * dst, Uint32 value);
extern size_t SDL_WriteBE32(SDL_RWops * dst, Uint32 value);
extern size_t SDL_WriteLE64(SDL_RWops * dst, Uint64 value);
extern size_t SDL_WriteBE64(SDL_RWops * dst, Uint64 value);

typedef Uint16 SDL_AudioFormat;
#define SDL_AUDIO_MASK_BITSIZE (0xFF)
#define SDL_AUDIO_MASK_DATATYPE (1<<8)
#define SDL_AUDIO_MASK_ENDIAN (1<<12)
#define SDL_AUDIO_MASK_SIGNED (1<<15)
#define SDL_AUDIO_BITSIZE(x) (x & SDL_AUDIO_MASK_BITSIZE)
#define SDL_AUDIO_ISFLOAT(x) (x & SDL_AUDIO_MASK_DATATYPE)
#define SDL_AUDIO_ISBIGENDIAN(x) (x & SDL_AUDIO_MASK_ENDIAN)
#define SDL_AUDIO_ISSIGNED(x) (x & SDL_AUDIO_MASK_SIGNED)
#define SDL_AUDIO_ISINT(x) (!SDL_AUDIO_ISFLOAT(x))
#define SDL_AUDIO_ISLITTLEENDIAN(x) (!SDL_AUDIO_ISBIGENDIAN(x))
#define SDL_AUDIO_ISUNSIGNED(x) (!SDL_AUDIO_ISSIGNED(x))
#define AUDIO_U8 0x0008
#define AUDIO_S8 0x8008
#define AUDIO_U16LSB 0x0010
#define AUDIO_S16LSB 0x8010
#define AUDIO_U16MSB 0x1010
#define AUDIO_S16MSB 0x9010
#define AUDIO_U16 AUDIO_U16LSB
#define AUDIO_S16 AUDIO_S16LSB
#define AUDIO_S32LSB 0x8020
#define AUDIO_S32MSB 0x9020
#define AUDIO_S32 AUDIO_S32LSB
#define AUDIO_F32LSB 0x8120
#define AUDIO_F32MSB 0x9120
#define AUDIO_F32 AUDIO_F32LSB
#define AUDIO_U16SYS AUDIO_U16LSB
#define AUDIO_S16SYS AUDIO_S16LSB
#define AUDIO_S32SYS AUDIO_S32LSB
#define AUDIO_F32SYS AUDIO_F32LSB
#define SDL_AUDIO_ALLOW_FREQUENCY_CHANGE 0x00000001
#define SDL_AUDIO_ALLOW_FORMAT_CHANGE 0x00000002
#define SDL_AUDIO_ALLOW_CHANNELS_CHANGE 0x00000004
#define SDL_AUDIO_ALLOW_SAMPLES_CHANGE 0x00000008
#define SDL_AUDIO_ALLOW_ANY_CHANGE (SDL_AUDIO_ALLOW_FREQUENCY_CHANGE|SDL_AUDIO_ALLOW_FORMAT_CHANGE|SDL_AUDIO_ALLOW_CHANNELS_CHANGE|SDL_AUDIO_ALLOW_SAMPLES_CHANGE)
typedef void ( * SDL_AudioCallback) (void *userdata, Uint8 * stream,
                                            int len);
typedef struct SDL_AudioSpec
{
    int freq;
    SDL_AudioFormat format;
    Uint8 channels;
    Uint8 silence;
    Uint16 samples;
    Uint16 padding;
    Uint32 size;
    SDL_AudioCallback callback;
    void *userdata;
} SDL_AudioSpec;

struct SDL_AudioCVT;
typedef void ( * SDL_AudioFilter) (struct SDL_AudioCVT * cvt,
                                          SDL_AudioFormat format);
#define SDL_AUDIOCVT_MAX_FILTERS 9
#define SDL_AUDIOCVT_PACKED __attribute__((packed))

typedef struct SDL_AudioCVT
{
    int needed;
    SDL_AudioFormat src_format;
    SDL_AudioFormat dst_format;
    double rate_incr;
    Uint8 *buf;
    int len;
    int len_cvt;
    int len_mult;
    double len_ratio;
    SDL_AudioFilter filters[9 + 1];
    int filter_index;
} SDL_AudioCVT;
extern int SDL_GetNumAudioDrivers(void);
extern const char * SDL_GetAudioDriver(int index);
extern int SDL_AudioInit(const char *driver_name);
extern void SDL_AudioQuit(void);

extern const char * SDL_GetCurrentAudioDriver(void);
extern int SDL_OpenAudio(SDL_AudioSpec * desired,
                                          SDL_AudioSpec * obtained);
typedef Uint32 SDL_AudioDeviceID;
extern int SDL_GetNumAudioDevices(int iscapture);
extern const char * SDL_GetAudioDeviceName(int index,
                                                           int iscapture);
extern SDL_AudioDeviceID SDL_OpenAudioDevice(const char
                                                              *device,
                                                              int iscapture,
                                                              const
                                                              SDL_AudioSpec *
                                                              desired,
                                                              SDL_AudioSpec *
                                                              obtained,
                                                              int
                                                              allowed_changes);
typedef enum
{
    SDL_AUDIO_STOPPED = 0,
    SDL_AUDIO_PLAYING,
    SDL_AUDIO_PAUSED
} SDL_AudioStatus;
extern SDL_AudioStatus SDL_GetAudioStatus(void);

extern SDL_AudioStatus
SDL_GetAudioDeviceStatus(SDL_AudioDeviceID dev);
extern void SDL_PauseAudio(int pause_on);
extern void SDL_PauseAudioDevice(SDL_AudioDeviceID dev,
                                                  int pause_on);
extern SDL_AudioSpec * SDL_LoadWAV_RW(SDL_RWops * src,
                                                      int freesrc,
                                                      SDL_AudioSpec * spec,
                                                      Uint8 ** audio_buf,
                                                      Uint32 * audio_len);
#define SDL_LoadWAV(file,spec,audio_buf,audio_len) SDL_LoadWAV_RW(SDL_RWFromFile(file, "rb"),1, spec,audio_buf,audio_len)

extern void SDL_FreeWAV(Uint8 * audio_buf);
extern int SDL_BuildAudioCVT(SDL_AudioCVT * cvt,
                                              SDL_AudioFormat src_format,
                                              Uint8 src_channels,
                                              int src_rate,
                                              SDL_AudioFormat dst_format,
                                              Uint8 dst_channels,
                                              int dst_rate);
extern int SDL_ConvertAudio(SDL_AudioCVT * cvt);
struct _SDL_AudioStream;
typedef struct _SDL_AudioStream SDL_AudioStream;
extern SDL_AudioStream * SDL_NewAudioStream(const SDL_AudioFormat src_format,
                                           const Uint8 src_channels,
                                           const int src_rate,
                                           const SDL_AudioFormat dst_format,
                                           const Uint8 dst_channels,
                                           const int dst_rate);
extern int SDL_AudioStreamPut(SDL_AudioStream *stream, const void *buf, int len);
extern int SDL_AudioStreamGet(SDL_AudioStream *stream, void *buf, int len);
extern int SDL_AudioStreamAvailable(SDL_AudioStream *stream);
extern int SDL_AudioStreamFlush(SDL_AudioStream *stream);
extern void SDL_AudioStreamClear(SDL_AudioStream *stream);
extern void SDL_FreeAudioStream(SDL_AudioStream *stream);
#define SDL_MIX_MAXVOLUME 128

extern void SDL_MixAudio(Uint8 * dst, const Uint8 * src,
                                          Uint32 len, int volume);

extern void SDL_MixAudioFormat(Uint8 * dst,
                                                const Uint8 * src,
                                                SDL_AudioFormat format,
                                                Uint32 len, int volume);
extern int SDL_QueueAudio(SDL_AudioDeviceID dev, const void *data, Uint32 len);
extern Uint32 SDL_DequeueAudio(SDL_AudioDeviceID dev, void *data, Uint32 len);
extern Uint32 SDL_GetQueuedAudioSize(SDL_AudioDeviceID dev);
extern void SDL_ClearQueuedAudio(SDL_AudioDeviceID dev);
extern void SDL_LockAudio(void);
extern void SDL_LockAudioDevice(SDL_AudioDeviceID dev);
extern void SDL_UnlockAudio(void);
extern void SDL_UnlockAudioDevice(SDL_AudioDeviceID dev);

extern void SDL_CloseAudio(void);
extern void SDL_CloseAudioDevice(SDL_AudioDeviceID dev);
#define SDL_clipboard_h_ 

extern int SDL_SetClipboardText(const char *text);

extern char * SDL_GetClipboardText(void);

extern SDL_bool SDL_HasClipboardText(void);
#define SDL_cpuinfo_h_ 

#define SDL_CACHELINE_SIZE 128

extern int SDL_GetCPUCount(void);

extern int SDL_GetCPUCacheLineSize(void);

extern SDL_bool SDL_HasRDTSC(void);

extern SDL_bool SDL_HasAltiVec(void);

extern SDL_bool SDL_HasMMX(void);

extern SDL_bool SDL_Has3DNow(void);

extern SDL_bool SDL_HasSSE(void);

extern SDL_bool SDL_HasSSE2(void);

extern SDL_bool SDL_HasSSE3(void);

extern SDL_bool SDL_HasSSE41(void);

extern SDL_bool SDL_HasSSE42(void);

extern SDL_bool SDL_HasAVX(void);

extern SDL_bool SDL_HasAVX2(void);

extern SDL_bool SDL_HasAVX512F(void);

extern SDL_bool SDL_HasARMSIMD(void);

extern SDL_bool SDL_HasNEON(void);

extern int SDL_GetSystemRAM(void);
extern size_t SDL_SIMDGetAlignment(void);
extern void * SDL_SIMDAlloc(const size_t len);
extern void SDL_SIMDFree(void *ptr);
#define SDL_events_h_ 
#define SDL_video_h_ 
#define SDL_pixels_h_ 

#define SDL_ALPHA_OPAQUE 255
#define SDL_ALPHA_TRANSPARENT 0

typedef enum
{
    SDL_PIXELTYPE_UNKNOWN,
    SDL_PIXELTYPE_INDEX1,
    SDL_PIXELTYPE_INDEX4,
    SDL_PIXELTYPE_INDEX8,
    SDL_PIXELTYPE_PACKED8,
    SDL_PIXELTYPE_PACKED16,
    SDL_PIXELTYPE_PACKED32,
    SDL_PIXELTYPE_ARRAYU8,
    SDL_PIXELTYPE_ARRAYU16,
    SDL_PIXELTYPE_ARRAYU32,
    SDL_PIXELTYPE_ARRAYF16,
    SDL_PIXELTYPE_ARRAYF32
} SDL_PixelType;

typedef enum
{
    SDL_BITMAPORDER_NONE,
    SDL_BITMAPORDER_4321,
    SDL_BITMAPORDER_1234
} SDL_BitmapOrder;

typedef enum
{
    SDL_PACKEDORDER_NONE,
    SDL_PACKEDORDER_XRGB,
    SDL_PACKEDORDER_RGBX,
    SDL_PACKEDORDER_ARGB,
    SDL_PACKEDORDER_RGBA,
    SDL_PACKEDORDER_XBGR,
    SDL_PACKEDORDER_BGRX,
    SDL_PACKEDORDER_ABGR,
    SDL_PACKEDORDER_BGRA
} SDL_PackedOrder;

typedef enum
{
    SDL_ARRAYORDER_NONE,
    SDL_ARRAYORDER_RGB,
    SDL_ARRAYORDER_RGBA,
    SDL_ARRAYORDER_ARGB,
    SDL_ARRAYORDER_BGR,
    SDL_ARRAYORDER_BGRA,
    SDL_ARRAYORDER_ABGR
} SDL_ArrayOrder;

typedef enum
{
    SDL_PACKEDLAYOUT_NONE,
    SDL_PACKEDLAYOUT_332,
    SDL_PACKEDLAYOUT_4444,
    SDL_PACKEDLAYOUT_1555,
    SDL_PACKEDLAYOUT_5551,
    SDL_PACKEDLAYOUT_565,
    SDL_PACKEDLAYOUT_8888,
    SDL_PACKEDLAYOUT_2101010,
    SDL_PACKEDLAYOUT_1010102
} SDL_PackedLayout;
#define SDL_DEFINE_PIXELFOURCC(A,B,C,D) SDL_FOURCC(A, B, C, D)
#define SDL_DEFINE_PIXELFORMAT(type,order,layout,bits,bytes) ((1 << 28) | ((type) << 24) | ((order) << 20) | ((layout) << 16) | ((bits) << 8) | ((bytes) << 0))
#define SDL_PIXELFLAG(X) (((X) >> 28) & 0x0F)
#define SDL_PIXELTYPE(X) (((X) >> 24) & 0x0F)
#define SDL_PIXELORDER(X) (((X) >> 20) & 0x0F)
#define SDL_PIXELLAYOUT(X) (((X) >> 16) & 0x0F)
#define SDL_BITSPERPIXEL(X) (((X) >> 8) & 0xFF)
#define SDL_BYTESPERPIXEL(X) (SDL_ISPIXELFORMAT_FOURCC(X) ? ((((X) == SDL_PIXELFORMAT_YUY2) || ((X) == SDL_PIXELFORMAT_UYVY) || ((X) == SDL_PIXELFORMAT_YVYU)) ? 2 : 1) : (((X) >> 0) & 0xFF))
#define SDL_ISPIXELFORMAT_INDEXED(format) (!SDL_ISPIXELFORMAT_FOURCC(format) && ((SDL_PIXELTYPE(format) == SDL_PIXELTYPE_INDEX1) || (SDL_PIXELTYPE(format) == SDL_PIXELTYPE_INDEX4) || (SDL_PIXELTYPE(format) == SDL_PIXELTYPE_INDEX8)))
#define SDL_ISPIXELFORMAT_PACKED(format) (!SDL_ISPIXELFORMAT_FOURCC(format) && ((SDL_PIXELTYPE(format) == SDL_PIXELTYPE_PACKED8) || (SDL_PIXELTYPE(format) == SDL_PIXELTYPE_PACKED16) || (SDL_PIXELTYPE(format) == SDL_PIXELTYPE_PACKED32)))
#define SDL_ISPIXELFORMAT_ARRAY(format) (!SDL_ISPIXELFORMAT_FOURCC(format) && ((SDL_PIXELTYPE(format) == SDL_PIXELTYPE_ARRAYU8) || (SDL_PIXELTYPE(format) == SDL_PIXELTYPE_ARRAYU16) || (SDL_PIXELTYPE(format) == SDL_PIXELTYPE_ARRAYU32) || (SDL_PIXELTYPE(format) == SDL_PIXELTYPE_ARRAYF16) || (SDL_PIXELTYPE(format) == SDL_PIXELTYPE_ARRAYF32)))
#define SDL_ISPIXELFORMAT_ALPHA(format) ((SDL_ISPIXELFORMAT_PACKED(format) && ((SDL_PIXELORDER(format) == SDL_PACKEDORDER_ARGB) || (SDL_PIXELORDER(format) == SDL_PACKEDORDER_RGBA) || (SDL_PIXELORDER(format) == SDL_PACKEDORDER_ABGR) || (SDL_PIXELORDER(format) == SDL_PACKEDORDER_BGRA))) || (SDL_ISPIXELFORMAT_ARRAY(format) && ((SDL_PIXELORDER(format) == SDL_ARRAYORDER_ARGB) || (SDL_PIXELORDER(format) == SDL_ARRAYORDER_RGBA) || (SDL_PIXELORDER(format) == SDL_ARRAYORDER_ABGR) || (SDL_PIXELORDER(format) == SDL_ARRAYORDER_BGRA))))
#define SDL_ISPIXELFORMAT_FOURCC(format) ((format) && (SDL_PIXELFLAG(format) != 1))

typedef enum
{
    SDL_PIXELFORMAT_UNKNOWN,
    SDL_PIXELFORMAT_INDEX1LSB =
        ((1 << 28) | ((SDL_PIXELTYPE_INDEX1) << 24) | ((SDL_BITMAPORDER_4321) << 20) | ((0) << 16) | ((1) << 8) | ((0) << 0))
                                    ,
    SDL_PIXELFORMAT_INDEX1MSB =
        ((1 << 28) | ((SDL_PIXELTYPE_INDEX1) << 24) | ((SDL_BITMAPORDER_1234) << 20) | ((0) << 16) | ((1) << 8) | ((0) << 0))
                                    ,
    SDL_PIXELFORMAT_INDEX4LSB =
        ((1 << 28) | ((SDL_PIXELTYPE_INDEX4) << 24) | ((SDL_BITMAPORDER_4321) << 20) | ((0) << 16) | ((4) << 8) | ((0) << 0))
                                    ,
    SDL_PIXELFORMAT_INDEX4MSB =
        ((1 << 28) | ((SDL_PIXELTYPE_INDEX4) << 24) | ((SDL_BITMAPORDER_1234) << 20) | ((0) << 16) | ((4) << 8) | ((0) << 0))
                                    ,
    SDL_PIXELFORMAT_INDEX8 =
        ((1 << 28) | ((SDL_PIXELTYPE_INDEX8) << 24) | ((0) << 20) | ((0) << 16) | ((8) << 8) | ((1) << 0)),
    SDL_PIXELFORMAT_RGB332 =
        ((1 << 28) | ((SDL_PIXELTYPE_PACKED8) << 24) | ((SDL_PACKEDORDER_XRGB) << 20) | ((SDL_PACKEDLAYOUT_332) << 16) | ((8) << 8) | ((1) << 0))
                                                          ,
    SDL_PIXELFORMAT_RGB444 =
        ((1 << 28) | ((SDL_PIXELTYPE_PACKED16) << 24) | ((SDL_PACKEDORDER_XRGB) << 20) | ((SDL_PACKEDLAYOUT_4444) << 16) | ((12) << 8) | ((2) << 0))
                                                            ,
    SDL_PIXELFORMAT_BGR444 =
        ((1 << 28) | ((SDL_PIXELTYPE_PACKED16) << 24) | ((SDL_PACKEDORDER_XBGR) << 20) | ((SDL_PACKEDLAYOUT_4444) << 16) | ((12) << 8) | ((2) << 0))
                                                            ,
    SDL_PIXELFORMAT_RGB555 =
        ((1 << 28) | ((SDL_PIXELTYPE_PACKED16) << 24) | ((SDL_PACKEDORDER_XRGB) << 20) | ((SDL_PACKEDLAYOUT_1555) << 16) | ((15) << 8) | ((2) << 0))
                                                            ,
    SDL_PIXELFORMAT_BGR555 =
        ((1 << 28) | ((SDL_PIXELTYPE_PACKED16) << 24) | ((SDL_PACKEDORDER_XBGR) << 20) | ((SDL_PACKEDLAYOUT_1555) << 16) | ((15) << 8) | ((2) << 0))
                                                            ,
    SDL_PIXELFORMAT_ARGB4444 =
        ((1 << 28) | ((SDL_PIXELTYPE_PACKED16) << 24) | ((SDL_PACKEDORDER_ARGB) << 20) | ((SDL_PACKEDLAYOUT_4444) << 16) | ((16) << 8) | ((2) << 0))
                                                            ,
    SDL_PIXELFORMAT_RGBA4444 =
        ((1 << 28) | ((SDL_PIXELTYPE_PACKED16) << 24) | ((SDL_PACKEDORDER_RGBA) << 20) | ((SDL_PACKEDLAYOUT_4444) << 16) | ((16) << 8) | ((2) << 0))
                                                            ,
    SDL_PIXELFORMAT_ABGR4444 =
        ((1 << 28) | ((SDL_PIXELTYPE_PACKED16) << 24) | ((SDL_PACKEDORDER_ABGR) << 20) | ((SDL_PACKEDLAYOUT_4444) << 16) | ((16) << 8) | ((2) << 0))
                                                            ,
    SDL_PIXELFORMAT_BGRA4444 =
        ((1 << 28) | ((SDL_PIXELTYPE_PACKED16) << 24) | ((SDL_PACKEDORDER_BGRA) << 20) | ((SDL_PACKEDLAYOUT_4444) << 16) | ((16) << 8) | ((2) << 0))
                                                            ,
    SDL_PIXELFORMAT_ARGB1555 =
        ((1 << 28) | ((SDL_PIXELTYPE_PACKED16) << 24) | ((SDL_PACKEDORDER_ARGB) << 20) | ((SDL_PACKEDLAYOUT_1555) << 16) | ((16) << 8) | ((2) << 0))
                                                            ,
    SDL_PIXELFORMAT_RGBA5551 =
        ((1 << 28) | ((SDL_PIXELTYPE_PACKED16) << 24) | ((SDL_PACKEDORDER_RGBA) << 20) | ((SDL_PACKEDLAYOUT_5551) << 16) | ((16) << 8) | ((2) << 0))
                                                            ,
    SDL_PIXELFORMAT_ABGR1555 =
        ((1 << 28) | ((SDL_PIXELTYPE_PACKED16) << 24) | ((SDL_PACKEDORDER_ABGR) << 20) | ((SDL_PACKEDLAYOUT_1555) << 16) | ((16) << 8) | ((2) << 0))
                                                            ,
    SDL_PIXELFORMAT_BGRA5551 =
        ((1 << 28) | ((SDL_PIXELTYPE_PACKED16) << 24) | ((SDL_PACKEDORDER_BGRA) << 20) | ((SDL_PACKEDLAYOUT_5551) << 16) | ((16) << 8) | ((2) << 0))
                                                            ,
    SDL_PIXELFORMAT_RGB565 =
        ((1 << 28) | ((SDL_PIXELTYPE_PACKED16) << 24) | ((SDL_PACKEDORDER_XRGB) << 20) | ((SDL_PACKEDLAYOUT_565) << 16) | ((16) << 8) | ((2) << 0))
                                                           ,
    SDL_PIXELFORMAT_BGR565 =
        ((1 << 28) | ((SDL_PIXELTYPE_PACKED16) << 24) | ((SDL_PACKEDORDER_XBGR) << 20) | ((SDL_PACKEDLAYOUT_565) << 16) | ((16) << 8) | ((2) << 0))
                                                           ,
    SDL_PIXELFORMAT_RGB24 =
        ((1 << 28) | ((SDL_PIXELTYPE_ARRAYU8) << 24) | ((SDL_ARRAYORDER_RGB) << 20) | ((0) << 16) | ((24) << 8) | ((3) << 0))
                                     ,
    SDL_PIXELFORMAT_BGR24 =
        ((1 << 28) | ((SDL_PIXELTYPE_ARRAYU8) << 24) | ((SDL_ARRAYORDER_BGR) << 20) | ((0) << 16) | ((24) << 8) | ((3) << 0))
                                     ,
    SDL_PIXELFORMAT_RGB888 =
        ((1 << 28) | ((SDL_PIXELTYPE_PACKED32) << 24) | ((SDL_PACKEDORDER_XRGB) << 20) | ((SDL_PACKEDLAYOUT_8888) << 16) | ((24) << 8) | ((4) << 0))
                                                            ,
    SDL_PIXELFORMAT_RGBX8888 =
        ((1 << 28) | ((SDL_PIXELTYPE_PACKED32) << 24) | ((SDL_PACKEDORDER_RGBX) << 20) | ((SDL_PACKEDLAYOUT_8888) << 16) | ((24) << 8) | ((4) << 0))
                                                            ,
    SDL_PIXELFORMAT_BGR888 =
        ((1 << 28) | ((SDL_PIXELTYPE_PACKED32) << 24) | ((SDL_PACKEDORDER_XBGR) << 20) | ((SDL_PACKEDLAYOUT_8888) << 16) | ((24) << 8) | ((4) << 0))
                                                            ,
    SDL_PIXELFORMAT_BGRX8888 =
        ((1 << 28) | ((SDL_PIXELTYPE_PACKED32) << 24) | ((SDL_PACKEDORDER_BGRX) << 20) | ((SDL_PACKEDLAYOUT_8888) << 16) | ((24) << 8) | ((4) << 0))
                                                            ,
    SDL_PIXELFORMAT_ARGB8888 =
        ((1 << 28) | ((SDL_PIXELTYPE_PACKED32) << 24) | ((SDL_PACKEDORDER_ARGB) << 20) | ((SDL_PACKEDLAYOUT_8888) << 16) | ((32) << 8) | ((4) << 0))
                                                            ,
    SDL_PIXELFORMAT_RGBA8888 =
        ((1 << 28) | ((SDL_PIXELTYPE_PACKED32) << 24) | ((SDL_PACKEDORDER_RGBA) << 20) | ((SDL_PACKEDLAYOUT_8888) << 16) | ((32) << 8) | ((4) << 0))
                                                            ,
    SDL_PIXELFORMAT_ABGR8888 =
        ((1 << 28) | ((SDL_PIXELTYPE_PACKED32) << 24) | ((SDL_PACKEDORDER_ABGR) << 20) | ((SDL_PACKEDLAYOUT_8888) << 16) | ((32) << 8) | ((4) << 0))
                                                            ,
    SDL_PIXELFORMAT_BGRA8888 =
        ((1 << 28) | ((SDL_PIXELTYPE_PACKED32) << 24) | ((SDL_PACKEDORDER_BGRA) << 20) | ((SDL_PACKEDLAYOUT_8888) << 16) | ((32) << 8) | ((4) << 0))
                                                            ,
    SDL_PIXELFORMAT_ARGB2101010 =
        ((1 << 28) | ((SDL_PIXELTYPE_PACKED32) << 24) | ((SDL_PACKEDORDER_ARGB) << 20) | ((SDL_PACKEDLAYOUT_2101010) << 16) | ((32) << 8) | ((4) << 0))
                                                               ,
    SDL_PIXELFORMAT_RGBA32 = SDL_PIXELFORMAT_ABGR8888,
    SDL_PIXELFORMAT_ARGB32 = SDL_PIXELFORMAT_BGRA8888,
    SDL_PIXELFORMAT_BGRA32 = SDL_PIXELFORMAT_ARGB8888,
    SDL_PIXELFORMAT_ABGR32 = SDL_PIXELFORMAT_RGBA8888,
    SDL_PIXELFORMAT_YV12 =
        ((((Uint32)(((Uint8)(('Y'))))) << 0) | (((Uint32)(((Uint8)(('V'))))) << 8) | (((Uint32)(((Uint8)(('1'))))) << 16) | (((Uint32)(((Uint8)(('2'))))) << 24)),
    SDL_PIXELFORMAT_IYUV =
        ((((Uint32)(((Uint8)(('I'))))) << 0) | (((Uint32)(((Uint8)(('Y'))))) << 8) | (((Uint32)(((Uint8)(('U'))))) << 16) | (((Uint32)(((Uint8)(('V'))))) << 24)),
    SDL_PIXELFORMAT_YUY2 =
        ((((Uint32)(((Uint8)(('Y'))))) << 0) | (((Uint32)(((Uint8)(('U'))))) << 8) | (((Uint32)(((Uint8)(('Y'))))) << 16) | (((Uint32)(((Uint8)(('2'))))) << 24)),
    SDL_PIXELFORMAT_UYVY =
        ((((Uint32)(((Uint8)(('U'))))) << 0) | (((Uint32)(((Uint8)(('Y'))))) << 8) | (((Uint32)(((Uint8)(('V'))))) << 16) | (((Uint32)(((Uint8)(('Y'))))) << 24)),
    SDL_PIXELFORMAT_YVYU =
        ((((Uint32)(((Uint8)(('Y'))))) << 0) | (((Uint32)(((Uint8)(('V'))))) << 8) | (((Uint32)(((Uint8)(('Y'))))) << 16) | (((Uint32)(((Uint8)(('U'))))) << 24)),
    SDL_PIXELFORMAT_NV12 =
        ((((Uint32)(((Uint8)(('N'))))) << 0) | (((Uint32)(((Uint8)(('V'))))) << 8) | (((Uint32)(((Uint8)(('1'))))) << 16) | (((Uint32)(((Uint8)(('2'))))) << 24)),
    SDL_PIXELFORMAT_NV21 =
        ((((Uint32)(((Uint8)(('N'))))) << 0) | (((Uint32)(((Uint8)(('V'))))) << 8) | (((Uint32)(((Uint8)(('2'))))) << 16) | (((Uint32)(((Uint8)(('1'))))) << 24)),
    SDL_PIXELFORMAT_EXTERNAL_OES =
        ((((Uint32)(((Uint8)(('O'))))) << 0) | (((Uint32)(((Uint8)(('E'))))) << 8) | (((Uint32)(((Uint8)(('S'))))) << 16) | (((Uint32)(((Uint8)((' '))))) << 24))
} SDL_PixelFormatEnum;

typedef struct SDL_Color
{
    Uint8 r;
    Uint8 g;
    Uint8 b;
    Uint8 a;
} SDL_Color;
#define SDL_Colour SDL_Color

typedef struct SDL_Palette
{
    int ncolors;
    SDL_Color *colors;
    Uint32 version;
    int refcount;
} SDL_Palette;

typedef struct SDL_PixelFormat
{
    Uint32 format;
    SDL_Palette *palette;
    Uint8 BitsPerPixel;
    Uint8 BytesPerPixel;
    Uint8 padding[2];
    Uint32 Rmask;
    Uint32 Gmask;
    Uint32 Bmask;
    Uint32 Amask;
    Uint8 Rloss;
    Uint8 Gloss;
    Uint8 Bloss;
    Uint8 Aloss;
    Uint8 Rshift;
    Uint8 Gshift;
    Uint8 Bshift;
    Uint8 Ashift;
    int refcount;
    struct SDL_PixelFormat *next;
} SDL_PixelFormat;

extern const char* SDL_GetPixelFormatName(Uint32 format);
extern SDL_bool SDL_PixelFormatEnumToMasks(Uint32 format,
                                                            int *bpp,
                                                            Uint32 * Rmask,
                                                            Uint32 * Gmask,
                                                            Uint32 * Bmask,
                                                            Uint32 * Amask);
extern Uint32 SDL_MasksToPixelFormatEnum(int bpp,
                                                          Uint32 Rmask,
                                                          Uint32 Gmask,
                                                          Uint32 Bmask,
                                                          Uint32 Amask);

extern SDL_PixelFormat * SDL_AllocFormat(Uint32 pixel_format);

extern void SDL_FreeFormat(SDL_PixelFormat *format);
extern SDL_Palette * SDL_AllocPalette(int ncolors);

extern int SDL_SetPixelFormatPalette(SDL_PixelFormat * format,
                                                      SDL_Palette *palette);
extern int SDL_SetPaletteColors(SDL_Palette * palette,
                                                 const SDL_Color * colors,
                                                 int firstcolor, int ncolors);

extern void SDL_FreePalette(SDL_Palette * palette);

extern Uint32 SDL_MapRGB(const SDL_PixelFormat * format,
                                          Uint8 r, Uint8 g, Uint8 b);

extern Uint32 SDL_MapRGBA(const SDL_PixelFormat * format,
                                           Uint8 r, Uint8 g, Uint8 b,
                                           Uint8 a);

extern void SDL_GetRGB(Uint32 pixel,
                                        const SDL_PixelFormat * format,
                                        Uint8 * r, Uint8 * g, Uint8 * b);

extern void SDL_GetRGBA(Uint32 pixel,
                                         const SDL_PixelFormat * format,
                                         Uint8 * r, Uint8 * g, Uint8 * b,
                                         Uint8 * a);

extern void SDL_CalculateGammaRamp(float gamma, Uint16 * ramp);
#define SDL_rect_h_ 

typedef struct SDL_Point
{
    int x;
    int y;
} SDL_Point;

typedef struct SDL_FPoint
{
    float x;
    float y;
} SDL_FPoint;
typedef struct SDL_Rect
{
    int x, y;
    int w, h;
} SDL_Rect;

typedef struct SDL_FRect
{
    float x;
    float y;
    float w;
    float h;
} SDL_FRect;
 static inline SDL_bool SDL_PointInRect(const SDL_Point *p, const SDL_Rect *r)
{
    return ( (p->x >= r->x) && (p->x < (r->x + r->w)) &&
             (p->y >= r->y) && (p->y < (r->y + r->h)) ) ? SDL_TRUE : SDL_FALSE;
}
 static inline SDL_bool SDL_RectEmpty(const SDL_Rect *r)
{
    return ((!r) || (r->w <= 0) || (r->h <= 0)) ? SDL_TRUE : SDL_FALSE;
}
 static inline SDL_bool SDL_RectEquals(const SDL_Rect *a, const SDL_Rect *b)
{
    return (a && b && (a->x == b->x) && (a->y == b->y) &&
            (a->w == b->w) && (a->h == b->h)) ? SDL_TRUE : SDL_FALSE;
}

extern SDL_bool SDL_HasIntersection(const SDL_Rect * A,
                                                     const SDL_Rect * B);

extern SDL_bool SDL_IntersectRect(const SDL_Rect * A,
                                                   const SDL_Rect * B,
                                                   SDL_Rect * result);

extern void SDL_UnionRect(const SDL_Rect * A,
                                           const SDL_Rect * B,
                                           SDL_Rect * result);

extern SDL_bool SDL_EnclosePoints(const SDL_Point * points,
                                                   int count,
                                                   const SDL_Rect * clip,
                                                   SDL_Rect * result);

extern SDL_bool SDL_IntersectRectAndLine(const SDL_Rect *
                                                          rect, int *X1,
                                                          int *Y1, int *X2,
                                                          int *Y2);
#define SDL_surface_h_ 
#define SDL_blendmode_h_ 

typedef enum
{
    SDL_BLENDMODE_NONE = 0x00000000,
    SDL_BLENDMODE_BLEND = 0x00000001,
    SDL_BLENDMODE_ADD = 0x00000002,
    SDL_BLENDMODE_MOD = 0x00000004,
    SDL_BLENDMODE_MUL = 0x00000008,
    SDL_BLENDMODE_INVALID = 0x7FFFFFFF
} SDL_BlendMode;

typedef enum
{
    SDL_BLENDOPERATION_ADD = 0x1,
    SDL_BLENDOPERATION_SUBTRACT = 0x2,
    SDL_BLENDOPERATION_REV_SUBTRACT = 0x3,
    SDL_BLENDOPERATION_MINIMUM = 0x4,
    SDL_BLENDOPERATION_MAXIMUM = 0x5
} SDL_BlendOperation;

typedef enum
{
    SDL_BLENDFACTOR_ZERO = 0x1,
    SDL_BLENDFACTOR_ONE = 0x2,
    SDL_BLENDFACTOR_SRC_COLOR = 0x3,
    SDL_BLENDFACTOR_ONE_MINUS_SRC_COLOR = 0x4,
    SDL_BLENDFACTOR_SRC_ALPHA = 0x5,
    SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA = 0x6,
    SDL_BLENDFACTOR_DST_COLOR = 0x7,
    SDL_BLENDFACTOR_ONE_MINUS_DST_COLOR = 0x8,
    SDL_BLENDFACTOR_DST_ALPHA = 0x9,
    SDL_BLENDFACTOR_ONE_MINUS_DST_ALPHA = 0xA
} SDL_BlendFactor;
extern SDL_BlendMode SDL_ComposeCustomBlendMode(SDL_BlendFactor srcColorFactor,
                                                                 SDL_BlendFactor dstColorFactor,
                                                                 SDL_BlendOperation colorOperation,
                                                                 SDL_BlendFactor srcAlphaFactor,
                                                                 SDL_BlendFactor dstAlphaFactor,
                                                                 SDL_BlendOperation alphaOperation);

#define SDL_SWSURFACE 0
#define SDL_PREALLOC 0x00000001
#define SDL_RLEACCEL 0x00000002
#define SDL_DONTFREE 0x00000004
#define SDL_SIMD_ALIGNED 0x00000008
#define SDL_MUSTLOCK(S) (((S)->flags & SDL_RLEACCEL) != 0)

typedef struct SDL_Surface
{
    Uint32 flags;
    SDL_PixelFormat *format;
    int w, h;
    int pitch;
    void *pixels;
    void *userdata;
    int locked;
    void *lock_data;
    SDL_Rect clip_rect;
    struct SDL_BlitMap *map;
    int refcount;
} SDL_Surface;

typedef int ( *SDL_blit) (struct SDL_Surface * src, SDL_Rect * srcrect,
                                 struct SDL_Surface * dst, SDL_Rect * dstrect);

typedef enum
{
    SDL_YUV_CONVERSION_JPEG,
    SDL_YUV_CONVERSION_BT601,
    SDL_YUV_CONVERSION_BT709,
    SDL_YUV_CONVERSION_AUTOMATIC
} SDL_YUV_CONVERSION_MODE;
extern SDL_Surface * SDL_CreateRGBSurface
    (Uint32 flags, int width, int height, int depth,
     Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask);

extern SDL_Surface * SDL_CreateRGBSurfaceWithFormat
    (Uint32 flags, int width, int height, int depth, Uint32 format);

extern SDL_Surface * SDL_CreateRGBSurfaceFrom(void *pixels,
                                                              int width,
                                                              int height,
                                                              int depth,
                                                              int pitch,
                                                              Uint32 Rmask,
                                                              Uint32 Gmask,
                                                              Uint32 Bmask,
                                                              Uint32 Amask);
extern SDL_Surface * SDL_CreateRGBSurfaceWithFormatFrom
    (void *pixels, int width, int height, int depth, int pitch, Uint32 format);
extern void SDL_FreeSurface(SDL_Surface * surface);
extern int SDL_SetSurfacePalette(SDL_Surface * surface,
                                                  SDL_Palette * palette);
extern int SDL_LockSurface(SDL_Surface * surface);

extern void SDL_UnlockSurface(SDL_Surface * surface);
extern SDL_Surface * SDL_LoadBMP_RW(SDL_RWops * src,
                                                    int freesrc);
#define SDL_LoadBMP(file) SDL_LoadBMP_RW(SDL_RWFromFile(file, "rb"), 1)
extern int SDL_SaveBMP_RW
    (SDL_Surface * surface, SDL_RWops * dst, int freedst);
#define SDL_SaveBMP(surface,file) SDL_SaveBMP_RW(surface, SDL_RWFromFile(file, "wb"), 1)
extern int SDL_SetSurfaceRLE(SDL_Surface * surface,
                                              int flag);
extern int SDL_SetColorKey(SDL_Surface * surface,
                                            int flag, Uint32 key);

extern SDL_bool SDL_HasColorKey(SDL_Surface * surface);
extern int SDL_GetColorKey(SDL_Surface * surface,
                                            Uint32 * key);
extern int SDL_SetSurfaceColorMod(SDL_Surface * surface,
                                                   Uint8 r, Uint8 g, Uint8 b);
extern int SDL_GetSurfaceColorMod(SDL_Surface * surface,
                                                   Uint8 * r, Uint8 * g,
                                                   Uint8 * b);
extern int SDL_SetSurfaceAlphaMod(SDL_Surface * surface,
                                                   Uint8 alpha);
extern int SDL_GetSurfaceAlphaMod(SDL_Surface * surface,
                                                   Uint8 * alpha);
extern int SDL_SetSurfaceBlendMode(SDL_Surface * surface,
                                                    SDL_BlendMode blendMode);
extern int SDL_GetSurfaceBlendMode(SDL_Surface * surface,
                                                    SDL_BlendMode *blendMode);
extern SDL_bool SDL_SetClipRect(SDL_Surface * surface,
                                                 const SDL_Rect * rect);

extern void SDL_GetClipRect(SDL_Surface * surface,
                                             SDL_Rect * rect);

extern SDL_Surface * SDL_DuplicateSurface(SDL_Surface * surface);
extern SDL_Surface * SDL_ConvertSurface
    (SDL_Surface * src, const SDL_PixelFormat * fmt, Uint32 flags);
extern SDL_Surface * SDL_ConvertSurfaceFormat
    (SDL_Surface * src, Uint32 pixel_format, Uint32 flags);

extern int SDL_ConvertPixels(int width, int height,
                                              Uint32 src_format,
                                              const void * src, int src_pitch,
                                              Uint32 dst_format,
                                              void * dst, int dst_pitch);
extern int SDL_FillRect
    (SDL_Surface * dst, const SDL_Rect * rect, Uint32 color);
extern int SDL_FillRects
    (SDL_Surface * dst, const SDL_Rect * rects, int count, Uint32 color);
#define SDL_BlitSurface SDL_UpperBlit

extern int SDL_UpperBlit
    (SDL_Surface * src, const SDL_Rect * srcrect,
     SDL_Surface * dst, SDL_Rect * dstrect);

extern int SDL_LowerBlit
    (SDL_Surface * src, SDL_Rect * srcrect,
     SDL_Surface * dst, SDL_Rect * dstrect);

extern int SDL_SoftStretch(SDL_Surface * src,
                                            const SDL_Rect * srcrect,
                                            SDL_Surface * dst,
                                            const SDL_Rect * dstrect);
#define SDL_BlitScaled SDL_UpperBlitScaled

extern int SDL_UpperBlitScaled
    (SDL_Surface * src, const SDL_Rect * srcrect,
    SDL_Surface * dst, SDL_Rect * dstrect);

extern int SDL_LowerBlitScaled
    (SDL_Surface * src, SDL_Rect * srcrect,
    SDL_Surface * dst, SDL_Rect * dstrect);

extern void SDL_SetYUVConversionMode(SDL_YUV_CONVERSION_MODE mode);

extern SDL_YUV_CONVERSION_MODE SDL_GetYUVConversionMode(void);

extern SDL_YUV_CONVERSION_MODE SDL_GetYUVConversionModeForResolution(int width, int height);

typedef struct
{
    Uint32 format;
    int w;
    int h;
    int refresh_rate;
    void *driverdata;
} SDL_DisplayMode;
typedef struct SDL_Window SDL_Window;

typedef enum
{
    SDL_WINDOW_FULLSCREEN = 0x00000001,
    SDL_WINDOW_OPENGL = 0x00000002,
    SDL_WINDOW_SHOWN = 0x00000004,
    SDL_WINDOW_HIDDEN = 0x00000008,
    SDL_WINDOW_BORDERLESS = 0x00000010,
    SDL_WINDOW_RESIZABLE = 0x00000020,
    SDL_WINDOW_MINIMIZED = 0x00000040,
    SDL_WINDOW_MAXIMIZED = 0x00000080,
    SDL_WINDOW_INPUT_GRABBED = 0x00000100,
    SDL_WINDOW_INPUT_FOCUS = 0x00000200,
    SDL_WINDOW_MOUSE_FOCUS = 0x00000400,
    SDL_WINDOW_FULLSCREEN_DESKTOP = ( SDL_WINDOW_FULLSCREEN | 0x00001000 ),
    SDL_WINDOW_FOREIGN = 0x00000800,
    SDL_WINDOW_ALLOW_HIGHDPI = 0x00002000,
    SDL_WINDOW_MOUSE_CAPTURE = 0x00004000,
    SDL_WINDOW_ALWAYS_ON_TOP = 0x00008000,
    SDL_WINDOW_SKIP_TASKBAR = 0x00010000,
    SDL_WINDOW_UTILITY = 0x00020000,
    SDL_WINDOW_TOOLTIP = 0x00040000,
    SDL_WINDOW_POPUP_MENU = 0x00080000,
    SDL_WINDOW_VULKAN = 0x10000000
} SDL_WindowFlags;
#define SDL_WINDOWPOS_UNDEFINED_MASK 0x1FFF0000u
#define SDL_WINDOWPOS_UNDEFINED_DISPLAY(X) (SDL_WINDOWPOS_UNDEFINED_MASK|(X))
#define SDL_WINDOWPOS_UNDEFINED SDL_WINDOWPOS_UNDEFINED_DISPLAY(0)
#define SDL_WINDOWPOS_ISUNDEFINED(X) (((X)&0xFFFF0000) == SDL_WINDOWPOS_UNDEFINED_MASK)
#define SDL_WINDOWPOS_CENTERED_MASK 0x2FFF0000u
#define SDL_WINDOWPOS_CENTERED_DISPLAY(X) (SDL_WINDOWPOS_CENTERED_MASK|(X))
#define SDL_WINDOWPOS_CENTERED SDL_WINDOWPOS_CENTERED_DISPLAY(0)
#define SDL_WINDOWPOS_ISCENTERED(X) (((X)&0xFFFF0000) == SDL_WINDOWPOS_CENTERED_MASK)

typedef enum
{
    SDL_WINDOWEVENT_NONE,
    SDL_WINDOWEVENT_SHOWN,
    SDL_WINDOWEVENT_HIDDEN,
    SDL_WINDOWEVENT_EXPOSED,
    SDL_WINDOWEVENT_MOVED,
    SDL_WINDOWEVENT_RESIZED,
    SDL_WINDOWEVENT_SIZE_CHANGED,
    SDL_WINDOWEVENT_MINIMIZED,
    SDL_WINDOWEVENT_MAXIMIZED,
    SDL_WINDOWEVENT_RESTORED,
    SDL_WINDOWEVENT_ENTER,
    SDL_WINDOWEVENT_LEAVE,
    SDL_WINDOWEVENT_FOCUS_GAINED,
    SDL_WINDOWEVENT_FOCUS_LOST,
    SDL_WINDOWEVENT_CLOSE,
    SDL_WINDOWEVENT_TAKE_FOCUS,
    SDL_WINDOWEVENT_HIT_TEST
} SDL_WindowEventID;

typedef enum
{
    SDL_DISPLAYEVENT_NONE,
    SDL_DISPLAYEVENT_ORIENTATION
} SDL_DisplayEventID;

typedef enum
{
    SDL_ORIENTATION_UNKNOWN,
    SDL_ORIENTATION_LANDSCAPE,
    SDL_ORIENTATION_LANDSCAPE_FLIPPED,
    SDL_ORIENTATION_PORTRAIT,
    SDL_ORIENTATION_PORTRAIT_FLIPPED
} SDL_DisplayOrientation;

typedef void *SDL_GLContext;

typedef enum
{
    SDL_GL_RED_SIZE,
    SDL_GL_GREEN_SIZE,
    SDL_GL_BLUE_SIZE,
    SDL_GL_ALPHA_SIZE,
    SDL_GL_BUFFER_SIZE,
    SDL_GL_DOUBLEBUFFER,
    SDL_GL_DEPTH_SIZE,
    SDL_GL_STENCIL_SIZE,
    SDL_GL_ACCUM_RED_SIZE,
    SDL_GL_ACCUM_GREEN_SIZE,
    SDL_GL_ACCUM_BLUE_SIZE,
    SDL_GL_ACCUM_ALPHA_SIZE,
    SDL_GL_STEREO,
    SDL_GL_MULTISAMPLEBUFFERS,
    SDL_GL_MULTISAMPLESAMPLES,
    SDL_GL_ACCELERATED_VISUAL,
    SDL_GL_RETAINED_BACKING,
    SDL_GL_CONTEXT_MAJOR_VERSION,
    SDL_GL_CONTEXT_MINOR_VERSION,
    SDL_GL_CONTEXT_EGL,
    SDL_GL_CONTEXT_FLAGS,
    SDL_GL_CONTEXT_PROFILE_MASK,
    SDL_GL_SHARE_WITH_CURRENT_CONTEXT,
    SDL_GL_FRAMEBUFFER_SRGB_CAPABLE,
    SDL_GL_CONTEXT_RELEASE_BEHAVIOR,
    SDL_GL_CONTEXT_RESET_NOTIFICATION,
    SDL_GL_CONTEXT_NO_ERROR
} SDL_GLattr;

typedef enum
{
    SDL_GL_CONTEXT_PROFILE_CORE = 0x0001,
    SDL_GL_CONTEXT_PROFILE_COMPATIBILITY = 0x0002,
    SDL_GL_CONTEXT_PROFILE_ES = 0x0004
} SDL_GLprofile;

typedef enum
{
    SDL_GL_CONTEXT_DEBUG_FLAG = 0x0001,
    SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG = 0x0002,
    SDL_GL_CONTEXT_ROBUST_ACCESS_FLAG = 0x0004,
    SDL_GL_CONTEXT_RESET_ISOLATION_FLAG = 0x0008
} SDL_GLcontextFlag;

typedef enum
{
    SDL_GL_CONTEXT_RELEASE_BEHAVIOR_NONE = 0x0000,
    SDL_GL_CONTEXT_RELEASE_BEHAVIOR_FLUSH = 0x0001
} SDL_GLcontextReleaseFlag;

typedef enum
{
    SDL_GL_CONTEXT_RESET_NO_NOTIFICATION = 0x0000,
    SDL_GL_CONTEXT_RESET_LOSE_CONTEXT = 0x0001
} SDL_GLContextResetNotification;
extern int SDL_GetNumVideoDrivers(void);
extern const char * SDL_GetVideoDriver(int index);
extern int SDL_VideoInit(const char *driver_name);
extern void SDL_VideoQuit(void);
extern const char * SDL_GetCurrentVideoDriver(void);

extern int SDL_GetNumVideoDisplays(void);
extern const char * SDL_GetDisplayName(int displayIndex);
extern int SDL_GetDisplayBounds(int displayIndex, SDL_Rect * rect);
extern int SDL_GetDisplayUsableBounds(int displayIndex, SDL_Rect * rect);
extern int SDL_GetDisplayDPI(int displayIndex, float * ddpi, float * hdpi, float * vdpi);
extern SDL_DisplayOrientation SDL_GetDisplayOrientation(int displayIndex);

extern int SDL_GetNumDisplayModes(int displayIndex);
extern int SDL_GetDisplayMode(int displayIndex, int modeIndex,
                                               SDL_DisplayMode * mode);

extern int SDL_GetDesktopDisplayMode(int displayIndex, SDL_DisplayMode * mode);

extern int SDL_GetCurrentDisplayMode(int displayIndex, SDL_DisplayMode * mode);
extern SDL_DisplayMode * SDL_GetClosestDisplayMode(int displayIndex, const SDL_DisplayMode * mode, SDL_DisplayMode * closest);

extern int SDL_GetWindowDisplayIndex(SDL_Window * window);
extern int SDL_SetWindowDisplayMode(SDL_Window * window,
                                                     const SDL_DisplayMode
                                                         * mode);
extern int SDL_GetWindowDisplayMode(SDL_Window * window,
                                                     SDL_DisplayMode * mode);

extern Uint32 SDL_GetWindowPixelFormat(SDL_Window * window);
extern SDL_Window * SDL_CreateWindow(const char *title,
                                                      int x, int y, int w,
                                                      int h, Uint32 flags);
extern SDL_Window * SDL_CreateWindowFrom(const void *data);

extern Uint32 SDL_GetWindowID(SDL_Window * window);

extern SDL_Window * SDL_GetWindowFromID(Uint32 id);

extern Uint32 SDL_GetWindowFlags(SDL_Window * window);

extern void SDL_SetWindowTitle(SDL_Window * window,
                                                const char *title);

extern const char * SDL_GetWindowTitle(SDL_Window * window);

extern void SDL_SetWindowIcon(SDL_Window * window,
                                               SDL_Surface * icon);
extern void* SDL_SetWindowData(SDL_Window * window,
                                                const char *name,
                                                void *userdata);
extern void * SDL_GetWindowData(SDL_Window * window,
                                                const char *name);
extern void SDL_SetWindowPosition(SDL_Window * window,
                                                   int x, int y);
extern void SDL_GetWindowPosition(SDL_Window * window,
                                                   int *x, int *y);
extern void SDL_SetWindowSize(SDL_Window * window, int w,
                                               int h);
extern void SDL_GetWindowSize(SDL_Window * window, int *w,
                                               int *h);
extern int SDL_GetWindowBordersSize(SDL_Window * window,
                                                     int *top, int *left,
                                                     int *bottom, int *right);
extern void SDL_SetWindowMinimumSize(SDL_Window * window,
                                                      int min_w, int min_h);
extern void SDL_GetWindowMinimumSize(SDL_Window * window,
                                                      int *w, int *h);
extern void SDL_SetWindowMaximumSize(SDL_Window * window,
                                                      int max_w, int max_h);
extern void SDL_GetWindowMaximumSize(SDL_Window * window,
                                                      int *w, int *h);
extern void SDL_SetWindowBordered(SDL_Window * window,
                                                   SDL_bool bordered);
extern void SDL_SetWindowResizable(SDL_Window * window,
                                                    SDL_bool resizable);

extern void SDL_ShowWindow(SDL_Window * window);

extern void SDL_HideWindow(SDL_Window * window);

extern void SDL_RaiseWindow(SDL_Window * window);

extern void SDL_MaximizeWindow(SDL_Window * window);

extern void SDL_MinimizeWindow(SDL_Window * window);

extern void SDL_RestoreWindow(SDL_Window * window);
extern int SDL_SetWindowFullscreen(SDL_Window * window,
                                                    Uint32 flags);
extern SDL_Surface * SDL_GetWindowSurface(SDL_Window * window);
extern int SDL_UpdateWindowSurface(SDL_Window * window);
extern int SDL_UpdateWindowSurfaceRects(SDL_Window * window,
                                                         const SDL_Rect * rects,
                                                         int numrects);
extern void SDL_SetWindowGrab(SDL_Window * window,
                                               SDL_bool grabbed);
extern SDL_bool SDL_GetWindowGrab(SDL_Window * window);
extern SDL_Window * SDL_GetGrabbedWindow(void);
extern int SDL_SetWindowBrightness(SDL_Window * window, float brightness);
extern float SDL_GetWindowBrightness(SDL_Window * window);
extern int SDL_SetWindowOpacity(SDL_Window * window, float opacity);
extern int SDL_GetWindowOpacity(SDL_Window * window, float * out_opacity);
extern int SDL_SetWindowModalFor(SDL_Window * modal_window, SDL_Window * parent_window);
extern int SDL_SetWindowInputFocus(SDL_Window * window);
extern int SDL_SetWindowGammaRamp(SDL_Window * window,
                                                   const Uint16 * red,
                                                   const Uint16 * green,
                                                   const Uint16 * blue);
extern int SDL_GetWindowGammaRamp(SDL_Window * window,
                                                   Uint16 * red,
                                                   Uint16 * green,
                                                   Uint16 * blue);

typedef enum
{
    SDL_HITTEST_NORMAL,
    SDL_HITTEST_DRAGGABLE,
    SDL_HITTEST_RESIZE_TOPLEFT,
    SDL_HITTEST_RESIZE_TOP,
    SDL_HITTEST_RESIZE_TOPRIGHT,
    SDL_HITTEST_RESIZE_RIGHT,
    SDL_HITTEST_RESIZE_BOTTOMRIGHT,
    SDL_HITTEST_RESIZE_BOTTOM,
    SDL_HITTEST_RESIZE_BOTTOMLEFT,
    SDL_HITTEST_RESIZE_LEFT
} SDL_HitTestResult;

typedef SDL_HitTestResult ( *SDL_HitTest)(SDL_Window *win,
                                                 const SDL_Point *area,
                                                 void *data);
extern int SDL_SetWindowHitTest(SDL_Window * window,
                                                 SDL_HitTest callback,
                                                 void *callback_data);

extern void SDL_DestroyWindow(SDL_Window * window);
extern SDL_bool SDL_IsScreenSaverEnabled(void);

extern void SDL_EnableScreenSaver(void);

extern void SDL_DisableScreenSaver(void);
extern int SDL_GL_LoadLibrary(const char *path);

extern void * SDL_GL_GetProcAddress(const char *proc);

extern void SDL_GL_UnloadLibrary(void);

extern SDL_bool SDL_GL_ExtensionSupported(const char
                                                           *extension);

extern void SDL_GL_ResetAttributes(void);

extern int SDL_GL_SetAttribute(SDL_GLattr attr, int value);

extern int SDL_GL_GetAttribute(SDL_GLattr attr, int *value);

extern SDL_GLContext SDL_GL_CreateContext(SDL_Window *
                                                           window);

extern int SDL_GL_MakeCurrent(SDL_Window * window,
                                               SDL_GLContext context);

extern SDL_Window* SDL_GL_GetCurrentWindow(void);

extern SDL_GLContext SDL_GL_GetCurrentContext(void);
extern void SDL_GL_GetDrawableSize(SDL_Window * window, int *w,
                                                    int *h);
extern int SDL_GL_SetSwapInterval(int interval);
extern int SDL_GL_GetSwapInterval(void);

extern void SDL_GL_SwapWindow(SDL_Window * window);

extern void SDL_GL_DeleteContext(SDL_GLContext context);
#define SDL_keyboard_h_ 
#define SDL_keycode_h_ 
#define SDL_scancode_h_ 
typedef enum
{
    SDL_SCANCODE_UNKNOWN = 0,
    SDL_SCANCODE_A = 4,
    SDL_SCANCODE_B = 5,
    SDL_SCANCODE_C = 6,
    SDL_SCANCODE_D = 7,
    SDL_SCANCODE_E = 8,
    SDL_SCANCODE_F = 9,
    SDL_SCANCODE_G = 10,
    SDL_SCANCODE_H = 11,
    SDL_SCANCODE_I = 12,
    SDL_SCANCODE_J = 13,
    SDL_SCANCODE_K = 14,
    SDL_SCANCODE_L = 15,
    SDL_SCANCODE_M = 16,
    SDL_SCANCODE_N = 17,
    SDL_SCANCODE_O = 18,
    SDL_SCANCODE_P = 19,
    SDL_SCANCODE_Q = 20,
    SDL_SCANCODE_R = 21,
    SDL_SCANCODE_S = 22,
    SDL_SCANCODE_T = 23,
    SDL_SCANCODE_U = 24,
    SDL_SCANCODE_V = 25,
    SDL_SCANCODE_W = 26,
    SDL_SCANCODE_X = 27,
    SDL_SCANCODE_Y = 28,
    SDL_SCANCODE_Z = 29,
    SDL_SCANCODE_1 = 30,
    SDL_SCANCODE_2 = 31,
    SDL_SCANCODE_3 = 32,
    SDL_SCANCODE_4 = 33,
    SDL_SCANCODE_5 = 34,
    SDL_SCANCODE_6 = 35,
    SDL_SCANCODE_7 = 36,
    SDL_SCANCODE_8 = 37,
    SDL_SCANCODE_9 = 38,
    SDL_SCANCODE_0 = 39,
    SDL_SCANCODE_RETURN = 40,
    SDL_SCANCODE_ESCAPE = 41,
    SDL_SCANCODE_BACKSPACE = 42,
    SDL_SCANCODE_TAB = 43,
    SDL_SCANCODE_SPACE = 44,
    SDL_SCANCODE_MINUS = 45,
    SDL_SCANCODE_EQUALS = 46,
    SDL_SCANCODE_LEFTBRACKET = 47,
    SDL_SCANCODE_RIGHTBRACKET = 48,
    SDL_SCANCODE_BACKSLASH = 49,
    SDL_SCANCODE_NONUSHASH = 50,
    SDL_SCANCODE_SEMICOLON = 51,
    SDL_SCANCODE_APOSTROPHE = 52,
    SDL_SCANCODE_GRAVE = 53,
    SDL_SCANCODE_COMMA = 54,
    SDL_SCANCODE_PERIOD = 55,
    SDL_SCANCODE_SLASH = 56,
    SDL_SCANCODE_CAPSLOCK = 57,
    SDL_SCANCODE_F1 = 58,
    SDL_SCANCODE_F2 = 59,
    SDL_SCANCODE_F3 = 60,
    SDL_SCANCODE_F4 = 61,
    SDL_SCANCODE_F5 = 62,
    SDL_SCANCODE_F6 = 63,
    SDL_SCANCODE_F7 = 64,
    SDL_SCANCODE_F8 = 65,
    SDL_SCANCODE_F9 = 66,
    SDL_SCANCODE_F10 = 67,
    SDL_SCANCODE_F11 = 68,
    SDL_SCANCODE_F12 = 69,
    SDL_SCANCODE_PRINTSCREEN = 70,
    SDL_SCANCODE_SCROLLLOCK = 71,
    SDL_SCANCODE_PAUSE = 72,
    SDL_SCANCODE_INSERT = 73,
    SDL_SCANCODE_HOME = 74,
    SDL_SCANCODE_PAGEUP = 75,
    SDL_SCANCODE_DELETE = 76,
    SDL_SCANCODE_END = 77,
    SDL_SCANCODE_PAGEDOWN = 78,
    SDL_SCANCODE_RIGHT = 79,
    SDL_SCANCODE_LEFT = 80,
    SDL_SCANCODE_DOWN = 81,
    SDL_SCANCODE_UP = 82,
    SDL_SCANCODE_NUMLOCKCLEAR = 83,
    SDL_SCANCODE_KP_DIVIDE = 84,
    SDL_SCANCODE_KP_MULTIPLY = 85,
    SDL_SCANCODE_KP_MINUS = 86,
    SDL_SCANCODE_KP_PLUS = 87,
    SDL_SCANCODE_KP_ENTER = 88,
    SDL_SCANCODE_KP_1 = 89,
    SDL_SCANCODE_KP_2 = 90,
    SDL_SCANCODE_KP_3 = 91,
    SDL_SCANCODE_KP_4 = 92,
    SDL_SCANCODE_KP_5 = 93,
    SDL_SCANCODE_KP_6 = 94,
    SDL_SCANCODE_KP_7 = 95,
    SDL_SCANCODE_KP_8 = 96,
    SDL_SCANCODE_KP_9 = 97,
    SDL_SCANCODE_KP_0 = 98,
    SDL_SCANCODE_KP_PERIOD = 99,
    SDL_SCANCODE_NONUSBACKSLASH = 100,
    SDL_SCANCODE_APPLICATION = 101,
    SDL_SCANCODE_POWER = 102,
    SDL_SCANCODE_KP_EQUALS = 103,
    SDL_SCANCODE_F13 = 104,
    SDL_SCANCODE_F14 = 105,
    SDL_SCANCODE_F15 = 106,
    SDL_SCANCODE_F16 = 107,
    SDL_SCANCODE_F17 = 108,
    SDL_SCANCODE_F18 = 109,
    SDL_SCANCODE_F19 = 110,
    SDL_SCANCODE_F20 = 111,
    SDL_SCANCODE_F21 = 112,
    SDL_SCANCODE_F22 = 113,
    SDL_SCANCODE_F23 = 114,
    SDL_SCANCODE_F24 = 115,
    SDL_SCANCODE_EXECUTE = 116,
    SDL_SCANCODE_HELP = 117,
    SDL_SCANCODE_MENU = 118,
    SDL_SCANCODE_SELECT = 119,
    SDL_SCANCODE_STOP = 120,
    SDL_SCANCODE_AGAIN = 121,
    SDL_SCANCODE_UNDO = 122,
    SDL_SCANCODE_CUT = 123,
    SDL_SCANCODE_COPY = 124,
    SDL_SCANCODE_PASTE = 125,
    SDL_SCANCODE_FIND = 126,
    SDL_SCANCODE_MUTE = 127,
    SDL_SCANCODE_VOLUMEUP = 128,
    SDL_SCANCODE_VOLUMEDOWN = 129,
    SDL_SCANCODE_KP_COMMA = 133,
    SDL_SCANCODE_KP_EQUALSAS400 = 134,
    SDL_SCANCODE_INTERNATIONAL1 = 135,
    SDL_SCANCODE_INTERNATIONAL2 = 136,
    SDL_SCANCODE_INTERNATIONAL3 = 137,
    SDL_SCANCODE_INTERNATIONAL4 = 138,
    SDL_SCANCODE_INTERNATIONAL5 = 139,
    SDL_SCANCODE_INTERNATIONAL6 = 140,
    SDL_SCANCODE_INTERNATIONAL7 = 141,
    SDL_SCANCODE_INTERNATIONAL8 = 142,
    SDL_SCANCODE_INTERNATIONAL9 = 143,
    SDL_SCANCODE_LANG1 = 144,
    SDL_SCANCODE_LANG2 = 145,
    SDL_SCANCODE_LANG3 = 146,
    SDL_SCANCODE_LANG4 = 147,
    SDL_SCANCODE_LANG5 = 148,
    SDL_SCANCODE_LANG6 = 149,
    SDL_SCANCODE_LANG7 = 150,
    SDL_SCANCODE_LANG8 = 151,
    SDL_SCANCODE_LANG9 = 152,
    SDL_SCANCODE_ALTERASE = 153,
    SDL_SCANCODE_SYSREQ = 154,
    SDL_SCANCODE_CANCEL = 155,
    SDL_SCANCODE_CLEAR = 156,
    SDL_SCANCODE_PRIOR = 157,
    SDL_SCANCODE_RETURN2 = 158,
    SDL_SCANCODE_SEPARATOR = 159,
    SDL_SCANCODE_OUT = 160,
    SDL_SCANCODE_OPER = 161,
    SDL_SCANCODE_CLEARAGAIN = 162,
    SDL_SCANCODE_CRSEL = 163,
    SDL_SCANCODE_EXSEL = 164,
    SDL_SCANCODE_KP_00 = 176,
    SDL_SCANCODE_KP_000 = 177,
    SDL_SCANCODE_THOUSANDSSEPARATOR = 178,
    SDL_SCANCODE_DECIMALSEPARATOR = 179,
    SDL_SCANCODE_CURRENCYUNIT = 180,
    SDL_SCANCODE_CURRENCYSUBUNIT = 181,
    SDL_SCANCODE_KP_LEFTPAREN = 182,
    SDL_SCANCODE_KP_RIGHTPAREN = 183,
    SDL_SCANCODE_KP_LEFTBRACE = 184,
    SDL_SCANCODE_KP_RIGHTBRACE = 185,
    SDL_SCANCODE_KP_TAB = 186,
    SDL_SCANCODE_KP_BACKSPACE = 187,
    SDL_SCANCODE_KP_A = 188,
    SDL_SCANCODE_KP_B = 189,
    SDL_SCANCODE_KP_C = 190,
    SDL_SCANCODE_KP_D = 191,
    SDL_SCANCODE_KP_E = 192,
    SDL_SCANCODE_KP_F = 193,
    SDL_SCANCODE_KP_XOR = 194,
    SDL_SCANCODE_KP_POWER = 195,
    SDL_SCANCODE_KP_PERCENT = 196,
    SDL_SCANCODE_KP_LESS = 197,
    SDL_SCANCODE_KP_GREATER = 198,
    SDL_SCANCODE_KP_AMPERSAND = 199,
    SDL_SCANCODE_KP_DBLAMPERSAND = 200,
    SDL_SCANCODE_KP_VERTICALBAR = 201,
    SDL_SCANCODE_KP_DBLVERTICALBAR = 202,
    SDL_SCANCODE_KP_COLON = 203,
    SDL_SCANCODE_KP_HASH = 204,
    SDL_SCANCODE_KP_SPACE = 205,
    SDL_SCANCODE_KP_AT = 206,
    SDL_SCANCODE_KP_EXCLAM = 207,
    SDL_SCANCODE_KP_MEMSTORE = 208,
    SDL_SCANCODE_KP_MEMRECALL = 209,
    SDL_SCANCODE_KP_MEMCLEAR = 210,
    SDL_SCANCODE_KP_MEMADD = 211,
    SDL_SCANCODE_KP_MEMSUBTRACT = 212,
    SDL_SCANCODE_KP_MEMMULTIPLY = 213,
    SDL_SCANCODE_KP_MEMDIVIDE = 214,
    SDL_SCANCODE_KP_PLUSMINUS = 215,
    SDL_SCANCODE_KP_CLEAR = 216,
    SDL_SCANCODE_KP_CLEARENTRY = 217,
    SDL_SCANCODE_KP_BINARY = 218,
    SDL_SCANCODE_KP_OCTAL = 219,
    SDL_SCANCODE_KP_DECIMAL = 220,
    SDL_SCANCODE_KP_HEXADECIMAL = 221,
    SDL_SCANCODE_LCTRL = 224,
    SDL_SCANCODE_LSHIFT = 225,
    SDL_SCANCODE_LALT = 226,
    SDL_SCANCODE_LGUI = 227,
    SDL_SCANCODE_RCTRL = 228,
    SDL_SCANCODE_RSHIFT = 229,
    SDL_SCANCODE_RALT = 230,
    SDL_SCANCODE_RGUI = 231,
    SDL_SCANCODE_MODE = 257,
    SDL_SCANCODE_AUDIONEXT = 258,
    SDL_SCANCODE_AUDIOPREV = 259,
    SDL_SCANCODE_AUDIOSTOP = 260,
    SDL_SCANCODE_AUDIOPLAY = 261,
    SDL_SCANCODE_AUDIOMUTE = 262,
    SDL_SCANCODE_MEDIASELECT = 263,
    SDL_SCANCODE_WWW = 264,
    SDL_SCANCODE_MAIL = 265,
    SDL_SCANCODE_CALCULATOR = 266,
    SDL_SCANCODE_COMPUTER = 267,
    SDL_SCANCODE_AC_SEARCH = 268,
    SDL_SCANCODE_AC_HOME = 269,
    SDL_SCANCODE_AC_BACK = 270,
    SDL_SCANCODE_AC_FORWARD = 271,
    SDL_SCANCODE_AC_STOP = 272,
    SDL_SCANCODE_AC_REFRESH = 273,
    SDL_SCANCODE_AC_BOOKMARKS = 274,
    SDL_SCANCODE_BRIGHTNESSDOWN = 275,
    SDL_SCANCODE_BRIGHTNESSUP = 276,
    SDL_SCANCODE_DISPLAYSWITCH = 277,
    SDL_SCANCODE_KBDILLUMTOGGLE = 278,
    SDL_SCANCODE_KBDILLUMDOWN = 279,
    SDL_SCANCODE_KBDILLUMUP = 280,
    SDL_SCANCODE_EJECT = 281,
    SDL_SCANCODE_SLEEP = 282,
    SDL_SCANCODE_APP1 = 283,
    SDL_SCANCODE_APP2 = 284,
    SDL_SCANCODE_AUDIOREWIND = 285,
    SDL_SCANCODE_AUDIOFASTFORWARD = 286,
    SDL_NUM_SCANCODES = 512
} SDL_Scancode;
typedef Sint32 SDL_Keycode;
#define SDLK_SCANCODE_MASK (1<<30)
#define SDL_SCANCODE_TO_KEYCODE(X) (X | SDLK_SCANCODE_MASK)

typedef enum
{
    SDLK_UNKNOWN = 0,
    SDLK_RETURN = '\r',
    SDLK_ESCAPE = '\033',
    SDLK_BACKSPACE = '\b',
    SDLK_TAB = '\t',
    SDLK_SPACE = ' ',
    SDLK_EXCLAIM = '!',
    SDLK_QUOTEDBL = '"',
    SDLK_HASH = '#',
    SDLK_PERCENT = '%',
    SDLK_DOLLAR = '$',
    SDLK_AMPERSAND = '&',
    SDLK_QUOTE = '\'',
    SDLK_LEFTPAREN = '(',
    SDLK_RIGHTPAREN = ')',
    SDLK_ASTERISK = '*',
    SDLK_PLUS = '+',
    SDLK_COMMA = ',',
    SDLK_MINUS = '-',
    SDLK_PERIOD = '.',
    SDLK_SLASH = '/',
    SDLK_0 = '0',
    SDLK_1 = '1',
    SDLK_2 = '2',
    SDLK_3 = '3',
    SDLK_4 = '4',
    SDLK_5 = '5',
    SDLK_6 = '6',
    SDLK_7 = '7',
    SDLK_8 = '8',
    SDLK_9 = '9',
    SDLK_COLON = ':',
    SDLK_SEMICOLON = ';',
    SDLK_LESS = '<',
    SDLK_EQUALS = '=',
    SDLK_GREATER = '>',
    SDLK_QUESTION = '?',
    SDLK_AT = '@',
    SDLK_LEFTBRACKET = '[',
    SDLK_BACKSLASH = '\\',
    SDLK_RIGHTBRACKET = ']',
    SDLK_CARET = '^',
    SDLK_UNDERSCORE = '_',
    SDLK_BACKQUOTE = '`',
    SDLK_a = 'a',
    SDLK_b = 'b',
    SDLK_c = 'c',
    SDLK_d = 'd',
    SDLK_e = 'e',
    SDLK_f = 'f',
    SDLK_g = 'g',
    SDLK_h = 'h',
    SDLK_i = 'i',
    SDLK_j = 'j',
    SDLK_k = 'k',
    SDLK_l = 'l',
    SDLK_m = 'm',
    SDLK_n = 'n',
    SDLK_o = 'o',
    SDLK_p = 'p',
    SDLK_q = 'q',
    SDLK_r = 'r',
    SDLK_s = 's',
    SDLK_t = 't',
    SDLK_u = 'u',
    SDLK_v = 'v',
    SDLK_w = 'w',
    SDLK_x = 'x',
    SDLK_y = 'y',
    SDLK_z = 'z',
    SDLK_CAPSLOCK = (SDL_SCANCODE_CAPSLOCK | (1<<30)),
    SDLK_F1 = (SDL_SCANCODE_F1 | (1<<30)),
    SDLK_F2 = (SDL_SCANCODE_F2 | (1<<30)),
    SDLK_F3 = (SDL_SCANCODE_F3 | (1<<30)),
    SDLK_F4 = (SDL_SCANCODE_F4 | (1<<30)),
    SDLK_F5 = (SDL_SCANCODE_F5 | (1<<30)),
    SDLK_F6 = (SDL_SCANCODE_F6 | (1<<30)),
    SDLK_F7 = (SDL_SCANCODE_F7 | (1<<30)),
    SDLK_F8 = (SDL_SCANCODE_F8 | (1<<30)),
    SDLK_F9 = (SDL_SCANCODE_F9 | (1<<30)),
    SDLK_F10 = (SDL_SCANCODE_F10 | (1<<30)),
    SDLK_F11 = (SDL_SCANCODE_F11 | (1<<30)),
    SDLK_F12 = (SDL_SCANCODE_F12 | (1<<30)),
    SDLK_PRINTSCREEN = (SDL_SCANCODE_PRINTSCREEN | (1<<30)),
    SDLK_SCROLLLOCK = (SDL_SCANCODE_SCROLLLOCK | (1<<30)),
    SDLK_PAUSE = (SDL_SCANCODE_PAUSE | (1<<30)),
    SDLK_INSERT = (SDL_SCANCODE_INSERT | (1<<30)),
    SDLK_HOME = (SDL_SCANCODE_HOME | (1<<30)),
    SDLK_PAGEUP = (SDL_SCANCODE_PAGEUP | (1<<30)),
    SDLK_DELETE = '\177',
    SDLK_END = (SDL_SCANCODE_END | (1<<30)),
    SDLK_PAGEDOWN = (SDL_SCANCODE_PAGEDOWN | (1<<30)),
    SDLK_RIGHT = (SDL_SCANCODE_RIGHT | (1<<30)),
    SDLK_LEFT = (SDL_SCANCODE_LEFT | (1<<30)),
    SDLK_DOWN = (SDL_SCANCODE_DOWN | (1<<30)),
    SDLK_UP = (SDL_SCANCODE_UP | (1<<30)),
    SDLK_NUMLOCKCLEAR = (SDL_SCANCODE_NUMLOCKCLEAR | (1<<30)),
    SDLK_KP_DIVIDE = (SDL_SCANCODE_KP_DIVIDE | (1<<30)),
    SDLK_KP_MULTIPLY = (SDL_SCANCODE_KP_MULTIPLY | (1<<30)),
    SDLK_KP_MINUS = (SDL_SCANCODE_KP_MINUS | (1<<30)),
    SDLK_KP_PLUS = (SDL_SCANCODE_KP_PLUS | (1<<30)),
    SDLK_KP_ENTER = (SDL_SCANCODE_KP_ENTER | (1<<30)),
    SDLK_KP_1 = (SDL_SCANCODE_KP_1 | (1<<30)),
    SDLK_KP_2 = (SDL_SCANCODE_KP_2 | (1<<30)),
    SDLK_KP_3 = (SDL_SCANCODE_KP_3 | (1<<30)),
    SDLK_KP_4 = (SDL_SCANCODE_KP_4 | (1<<30)),
    SDLK_KP_5 = (SDL_SCANCODE_KP_5 | (1<<30)),
    SDLK_KP_6 = (SDL_SCANCODE_KP_6 | (1<<30)),
    SDLK_KP_7 = (SDL_SCANCODE_KP_7 | (1<<30)),
    SDLK_KP_8 = (SDL_SCANCODE_KP_8 | (1<<30)),
    SDLK_KP_9 = (SDL_SCANCODE_KP_9 | (1<<30)),
    SDLK_KP_0 = (SDL_SCANCODE_KP_0 | (1<<30)),
    SDLK_KP_PERIOD = (SDL_SCANCODE_KP_PERIOD | (1<<30)),
    SDLK_APPLICATION = (SDL_SCANCODE_APPLICATION | (1<<30)),
    SDLK_POWER = (SDL_SCANCODE_POWER | (1<<30)),
    SDLK_KP_EQUALS = (SDL_SCANCODE_KP_EQUALS | (1<<30)),
    SDLK_F13 = (SDL_SCANCODE_F13 | (1<<30)),
    SDLK_F14 = (SDL_SCANCODE_F14 | (1<<30)),
    SDLK_F15 = (SDL_SCANCODE_F15 | (1<<30)),
    SDLK_F16 = (SDL_SCANCODE_F16 | (1<<30)),
    SDLK_F17 = (SDL_SCANCODE_F17 | (1<<30)),
    SDLK_F18 = (SDL_SCANCODE_F18 | (1<<30)),
    SDLK_F19 = (SDL_SCANCODE_F19 | (1<<30)),
    SDLK_F20 = (SDL_SCANCODE_F20 | (1<<30)),
    SDLK_F21 = (SDL_SCANCODE_F21 | (1<<30)),
    SDLK_F22 = (SDL_SCANCODE_F22 | (1<<30)),
    SDLK_F23 = (SDL_SCANCODE_F23 | (1<<30)),
    SDLK_F24 = (SDL_SCANCODE_F24 | (1<<30)),
    SDLK_EXECUTE = (SDL_SCANCODE_EXECUTE | (1<<30)),
    SDLK_HELP = (SDL_SCANCODE_HELP | (1<<30)),
    SDLK_MENU = (SDL_SCANCODE_MENU | (1<<30)),
    SDLK_SELECT = (SDL_SCANCODE_SELECT | (1<<30)),
    SDLK_STOP = (SDL_SCANCODE_STOP | (1<<30)),
    SDLK_AGAIN = (SDL_SCANCODE_AGAIN | (1<<30)),
    SDLK_UNDO = (SDL_SCANCODE_UNDO | (1<<30)),
    SDLK_CUT = (SDL_SCANCODE_CUT | (1<<30)),
    SDLK_COPY = (SDL_SCANCODE_COPY | (1<<30)),
    SDLK_PASTE = (SDL_SCANCODE_PASTE | (1<<30)),
    SDLK_FIND = (SDL_SCANCODE_FIND | (1<<30)),
    SDLK_MUTE = (SDL_SCANCODE_MUTE | (1<<30)),
    SDLK_VOLUMEUP = (SDL_SCANCODE_VOLUMEUP | (1<<30)),
    SDLK_VOLUMEDOWN = (SDL_SCANCODE_VOLUMEDOWN | (1<<30)),
    SDLK_KP_COMMA = (SDL_SCANCODE_KP_COMMA | (1<<30)),
    SDLK_KP_EQUALSAS400 =
        (SDL_SCANCODE_KP_EQUALSAS400 | (1<<30)),
    SDLK_ALTERASE = (SDL_SCANCODE_ALTERASE | (1<<30)),
    SDLK_SYSREQ = (SDL_SCANCODE_SYSREQ | (1<<30)),
    SDLK_CANCEL = (SDL_SCANCODE_CANCEL | (1<<30)),
    SDLK_CLEAR = (SDL_SCANCODE_CLEAR | (1<<30)),
    SDLK_PRIOR = (SDL_SCANCODE_PRIOR | (1<<30)),
    SDLK_RETURN2 = (SDL_SCANCODE_RETURN2 | (1<<30)),
    SDLK_SEPARATOR = (SDL_SCANCODE_SEPARATOR | (1<<30)),
    SDLK_OUT = (SDL_SCANCODE_OUT | (1<<30)),
    SDLK_OPER = (SDL_SCANCODE_OPER | (1<<30)),
    SDLK_CLEARAGAIN = (SDL_SCANCODE_CLEARAGAIN | (1<<30)),
    SDLK_CRSEL = (SDL_SCANCODE_CRSEL | (1<<30)),
    SDLK_EXSEL = (SDL_SCANCODE_EXSEL | (1<<30)),
    SDLK_KP_00 = (SDL_SCANCODE_KP_00 | (1<<30)),
    SDLK_KP_000 = (SDL_SCANCODE_KP_000 | (1<<30)),
    SDLK_THOUSANDSSEPARATOR =
        (SDL_SCANCODE_THOUSANDSSEPARATOR | (1<<30)),
    SDLK_DECIMALSEPARATOR =
        (SDL_SCANCODE_DECIMALSEPARATOR | (1<<30)),
    SDLK_CURRENCYUNIT = (SDL_SCANCODE_CURRENCYUNIT | (1<<30)),
    SDLK_CURRENCYSUBUNIT =
        (SDL_SCANCODE_CURRENCYSUBUNIT | (1<<30)),
    SDLK_KP_LEFTPAREN = (SDL_SCANCODE_KP_LEFTPAREN | (1<<30)),
    SDLK_KP_RIGHTPAREN = (SDL_SCANCODE_KP_RIGHTPAREN | (1<<30)),
    SDLK_KP_LEFTBRACE = (SDL_SCANCODE_KP_LEFTBRACE | (1<<30)),
    SDLK_KP_RIGHTBRACE = (SDL_SCANCODE_KP_RIGHTBRACE | (1<<30)),
    SDLK_KP_TAB = (SDL_SCANCODE_KP_TAB | (1<<30)),
    SDLK_KP_BACKSPACE = (SDL_SCANCODE_KP_BACKSPACE | (1<<30)),
    SDLK_KP_A = (SDL_SCANCODE_KP_A | (1<<30)),
    SDLK_KP_B = (SDL_SCANCODE_KP_B | (1<<30)),
    SDLK_KP_C = (SDL_SCANCODE_KP_C | (1<<30)),
    SDLK_KP_D = (SDL_SCANCODE_KP_D | (1<<30)),
    SDLK_KP_E = (SDL_SCANCODE_KP_E | (1<<30)),
    SDLK_KP_F = (SDL_SCANCODE_KP_F | (1<<30)),
    SDLK_KP_XOR = (SDL_SCANCODE_KP_XOR | (1<<30)),
    SDLK_KP_POWER = (SDL_SCANCODE_KP_POWER | (1<<30)),
    SDLK_KP_PERCENT = (SDL_SCANCODE_KP_PERCENT | (1<<30)),
    SDLK_KP_LESS = (SDL_SCANCODE_KP_LESS | (1<<30)),
    SDLK_KP_GREATER = (SDL_SCANCODE_KP_GREATER | (1<<30)),
    SDLK_KP_AMPERSAND = (SDL_SCANCODE_KP_AMPERSAND | (1<<30)),
    SDLK_KP_DBLAMPERSAND =
        (SDL_SCANCODE_KP_DBLAMPERSAND | (1<<30)),
    SDLK_KP_VERTICALBAR =
        (SDL_SCANCODE_KP_VERTICALBAR | (1<<30)),
    SDLK_KP_DBLVERTICALBAR =
        (SDL_SCANCODE_KP_DBLVERTICALBAR | (1<<30)),
    SDLK_KP_COLON = (SDL_SCANCODE_KP_COLON | (1<<30)),
    SDLK_KP_HASH = (SDL_SCANCODE_KP_HASH | (1<<30)),
    SDLK_KP_SPACE = (SDL_SCANCODE_KP_SPACE | (1<<30)),
    SDLK_KP_AT = (SDL_SCANCODE_KP_AT | (1<<30)),
    SDLK_KP_EXCLAM = (SDL_SCANCODE_KP_EXCLAM | (1<<30)),
    SDLK_KP_MEMSTORE = (SDL_SCANCODE_KP_MEMSTORE | (1<<30)),
    SDLK_KP_MEMRECALL = (SDL_SCANCODE_KP_MEMRECALL | (1<<30)),
    SDLK_KP_MEMCLEAR = (SDL_SCANCODE_KP_MEMCLEAR | (1<<30)),
    SDLK_KP_MEMADD = (SDL_SCANCODE_KP_MEMADD | (1<<30)),
    SDLK_KP_MEMSUBTRACT =
        (SDL_SCANCODE_KP_MEMSUBTRACT | (1<<30)),
    SDLK_KP_MEMMULTIPLY =
        (SDL_SCANCODE_KP_MEMMULTIPLY | (1<<30)),
    SDLK_KP_MEMDIVIDE = (SDL_SCANCODE_KP_MEMDIVIDE | (1<<30)),
    SDLK_KP_PLUSMINUS = (SDL_SCANCODE_KP_PLUSMINUS | (1<<30)),
    SDLK_KP_CLEAR = (SDL_SCANCODE_KP_CLEAR | (1<<30)),
    SDLK_KP_CLEARENTRY = (SDL_SCANCODE_KP_CLEARENTRY | (1<<30)),
    SDLK_KP_BINARY = (SDL_SCANCODE_KP_BINARY | (1<<30)),
    SDLK_KP_OCTAL = (SDL_SCANCODE_KP_OCTAL | (1<<30)),
    SDLK_KP_DECIMAL = (SDL_SCANCODE_KP_DECIMAL | (1<<30)),
    SDLK_KP_HEXADECIMAL =
        (SDL_SCANCODE_KP_HEXADECIMAL | (1<<30)),
    SDLK_LCTRL = (SDL_SCANCODE_LCTRL | (1<<30)),
    SDLK_LSHIFT = (SDL_SCANCODE_LSHIFT | (1<<30)),
    SDLK_LALT = (SDL_SCANCODE_LALT | (1<<30)),
    SDLK_LGUI = (SDL_SCANCODE_LGUI | (1<<30)),
    SDLK_RCTRL = (SDL_SCANCODE_RCTRL | (1<<30)),
    SDLK_RSHIFT = (SDL_SCANCODE_RSHIFT | (1<<30)),
    SDLK_RALT = (SDL_SCANCODE_RALT | (1<<30)),
    SDLK_RGUI = (SDL_SCANCODE_RGUI | (1<<30)),
    SDLK_MODE = (SDL_SCANCODE_MODE | (1<<30)),
    SDLK_AUDIONEXT = (SDL_SCANCODE_AUDIONEXT | (1<<30)),
    SDLK_AUDIOPREV = (SDL_SCANCODE_AUDIOPREV | (1<<30)),
    SDLK_AUDIOSTOP = (SDL_SCANCODE_AUDIOSTOP | (1<<30)),
    SDLK_AUDIOPLAY = (SDL_SCANCODE_AUDIOPLAY | (1<<30)),
    SDLK_AUDIOMUTE = (SDL_SCANCODE_AUDIOMUTE | (1<<30)),
    SDLK_MEDIASELECT = (SDL_SCANCODE_MEDIASELECT | (1<<30)),
    SDLK_WWW = (SDL_SCANCODE_WWW | (1<<30)),
    SDLK_MAIL = (SDL_SCANCODE_MAIL | (1<<30)),
    SDLK_CALCULATOR = (SDL_SCANCODE_CALCULATOR | (1<<30)),
    SDLK_COMPUTER = (SDL_SCANCODE_COMPUTER | (1<<30)),
    SDLK_AC_SEARCH = (SDL_SCANCODE_AC_SEARCH | (1<<30)),
    SDLK_AC_HOME = (SDL_SCANCODE_AC_HOME | (1<<30)),
    SDLK_AC_BACK = (SDL_SCANCODE_AC_BACK | (1<<30)),
    SDLK_AC_FORWARD = (SDL_SCANCODE_AC_FORWARD | (1<<30)),
    SDLK_AC_STOP = (SDL_SCANCODE_AC_STOP | (1<<30)),
    SDLK_AC_REFRESH = (SDL_SCANCODE_AC_REFRESH | (1<<30)),
    SDLK_AC_BOOKMARKS = (SDL_SCANCODE_AC_BOOKMARKS | (1<<30)),
    SDLK_BRIGHTNESSDOWN =
        (SDL_SCANCODE_BRIGHTNESSDOWN | (1<<30)),
    SDLK_BRIGHTNESSUP = (SDL_SCANCODE_BRIGHTNESSUP | (1<<30)),
    SDLK_DISPLAYSWITCH = (SDL_SCANCODE_DISPLAYSWITCH | (1<<30)),
    SDLK_KBDILLUMTOGGLE =
        (SDL_SCANCODE_KBDILLUMTOGGLE | (1<<30)),
    SDLK_KBDILLUMDOWN = (SDL_SCANCODE_KBDILLUMDOWN | (1<<30)),
    SDLK_KBDILLUMUP = (SDL_SCANCODE_KBDILLUMUP | (1<<30)),
    SDLK_EJECT = (SDL_SCANCODE_EJECT | (1<<30)),
    SDLK_SLEEP = (SDL_SCANCODE_SLEEP | (1<<30)),
    SDLK_APP1 = (SDL_SCANCODE_APP1 | (1<<30)),
    SDLK_APP2 = (SDL_SCANCODE_APP2 | (1<<30)),
    SDLK_AUDIOREWIND = (SDL_SCANCODE_AUDIOREWIND | (1<<30)),
    SDLK_AUDIOFASTFORWARD = (SDL_SCANCODE_AUDIOFASTFORWARD | (1<<30))
} SDL_KeyCode;

typedef enum
{
    KMOD_NONE = 0x0000,
    KMOD_LSHIFT = 0x0001,
    KMOD_RSHIFT = 0x0002,
    KMOD_LCTRL = 0x0040,
    KMOD_RCTRL = 0x0080,
    KMOD_LALT = 0x0100,
    KMOD_RALT = 0x0200,
    KMOD_LGUI = 0x0400,
    KMOD_RGUI = 0x0800,
    KMOD_NUM = 0x1000,
    KMOD_CAPS = 0x2000,
    KMOD_MODE = 0x4000,
    KMOD_RESERVED = 0x8000
} SDL_Keymod;
#define KMOD_CTRL (KMOD_LCTRL|KMOD_RCTRL)
#define KMOD_SHIFT (KMOD_LSHIFT|KMOD_RSHIFT)
#define KMOD_ALT (KMOD_LALT|KMOD_RALT)
#define KMOD_GUI (KMOD_LGUI|KMOD_RGUI)

typedef struct SDL_Keysym
{
    SDL_Scancode scancode;
    SDL_Keycode sym;
    Uint16 mod;
    Uint32 unused;
} SDL_Keysym;

extern SDL_Window * SDL_GetKeyboardFocus(void);
extern const Uint8 * SDL_GetKeyboardState(int *numkeys);

extern SDL_Keymod SDL_GetModState(void);

extern void SDL_SetModState(SDL_Keymod modstate);
extern SDL_Keycode SDL_GetKeyFromScancode(SDL_Scancode scancode);
extern SDL_Scancode SDL_GetScancodeFromKey(SDL_Keycode key);
extern const char * SDL_GetScancodeName(SDL_Scancode scancode);
extern SDL_Scancode SDL_GetScancodeFromName(const char *name);
extern const char * SDL_GetKeyName(SDL_Keycode key);
extern SDL_Keycode SDL_GetKeyFromName(const char *name);
extern void SDL_StartTextInput(void);

extern SDL_bool SDL_IsTextInputActive(void);
extern void SDL_StopTextInput(void);

extern void SDL_SetTextInputRect(SDL_Rect *rect);
extern SDL_bool SDL_HasScreenKeyboardSupport(void);
extern SDL_bool SDL_IsScreenKeyboardShown(SDL_Window *window);
#define SDL_mouse_h_ 


typedef struct SDL_Cursor SDL_Cursor;

typedef enum
{
    SDL_SYSTEM_CURSOR_ARROW,
    SDL_SYSTEM_CURSOR_IBEAM,
    SDL_SYSTEM_CURSOR_WAIT,
    SDL_SYSTEM_CURSOR_CROSSHAIR,
    SDL_SYSTEM_CURSOR_WAITARROW,
    SDL_SYSTEM_CURSOR_SIZENWSE,
    SDL_SYSTEM_CURSOR_SIZENESW,
    SDL_SYSTEM_CURSOR_SIZEWE,
    SDL_SYSTEM_CURSOR_SIZENS,
    SDL_SYSTEM_CURSOR_SIZEALL,
    SDL_SYSTEM_CURSOR_NO,
    SDL_SYSTEM_CURSOR_HAND,
    SDL_NUM_SYSTEM_CURSORS
} SDL_SystemCursor;

typedef enum
{
    SDL_MOUSEWHEEL_NORMAL,
    SDL_MOUSEWHEEL_FLIPPED
} SDL_MouseWheelDirection;

extern SDL_Window * SDL_GetMouseFocus(void);
extern Uint32 SDL_GetMouseState(int *x, int *y);
extern Uint32 SDL_GetGlobalMouseState(int *x, int *y);
extern Uint32 SDL_GetRelativeMouseState(int *x, int *y);
extern void SDL_WarpMouseInWindow(SDL_Window * window,
                                                   int x, int y);
extern int SDL_WarpMouseGlobal(int x, int y);
extern int SDL_SetRelativeMouseMode(SDL_bool enabled);
extern int SDL_CaptureMouse(SDL_bool enabled);

extern SDL_bool SDL_GetRelativeMouseMode(void);
extern SDL_Cursor * SDL_CreateCursor(const Uint8 * data,
                                                     const Uint8 * mask,
                                                     int w, int h, int hot_x,
                                                     int hot_y);

extern SDL_Cursor * SDL_CreateColorCursor(SDL_Surface *surface,
                                                          int hot_x,
                                                          int hot_y);

extern SDL_Cursor * SDL_CreateSystemCursor(SDL_SystemCursor id);

extern void SDL_SetCursor(SDL_Cursor * cursor);

extern SDL_Cursor * SDL_GetCursor(void);

extern SDL_Cursor * SDL_GetDefaultCursor(void);
extern void SDL_FreeCursor(SDL_Cursor * cursor);
extern int SDL_ShowCursor(int toggle);
#define SDL_BUTTON(X) (1 << ((X)-1))
#define SDL_BUTTON_LEFT 1
#define SDL_BUTTON_MIDDLE 2
#define SDL_BUTTON_RIGHT 3
#define SDL_BUTTON_X1 4
#define SDL_BUTTON_X2 5
#define SDL_BUTTON_LMASK SDL_BUTTON(SDL_BUTTON_LEFT)
#define SDL_BUTTON_MMASK SDL_BUTTON(SDL_BUTTON_MIDDLE)
#define SDL_BUTTON_RMASK SDL_BUTTON(SDL_BUTTON_RIGHT)
#define SDL_BUTTON_X1MASK SDL_BUTTON(SDL_BUTTON_X1)
#define SDL_BUTTON_X2MASK SDL_BUTTON(SDL_BUTTON_X2)
#define SDL_joystick_h_ 

struct _SDL_Joystick;
typedef struct _SDL_Joystick SDL_Joystick;

typedef struct {
    Uint8 data[16];
} SDL_JoystickGUID;
typedef Sint32 SDL_JoystickID;

typedef enum
{
    SDL_JOYSTICK_TYPE_UNKNOWN,
    SDL_JOYSTICK_TYPE_GAMECONTROLLER,
    SDL_JOYSTICK_TYPE_WHEEL,
    SDL_JOYSTICK_TYPE_ARCADE_STICK,
    SDL_JOYSTICK_TYPE_FLIGHT_STICK,
    SDL_JOYSTICK_TYPE_DANCE_PAD,
    SDL_JOYSTICK_TYPE_GUITAR,
    SDL_JOYSTICK_TYPE_DRUM_KIT,
    SDL_JOYSTICK_TYPE_ARCADE_PAD,
    SDL_JOYSTICK_TYPE_THROTTLE
} SDL_JoystickType;

typedef enum
{
    SDL_JOYSTICK_POWER_UNKNOWN = -1,
    SDL_JOYSTICK_POWER_EMPTY,
    SDL_JOYSTICK_POWER_LOW,
    SDL_JOYSTICK_POWER_MEDIUM,
    SDL_JOYSTICK_POWER_FULL,
    SDL_JOYSTICK_POWER_WIRED,
    SDL_JOYSTICK_POWER_MAX
} SDL_JoystickPowerLevel;
extern void SDL_LockJoysticks(void);
extern void SDL_UnlockJoysticks(void);

extern int SDL_NumJoysticks(void);

extern const char * SDL_JoystickNameForIndex(int device_index);

extern int SDL_JoystickGetDevicePlayerIndex(int device_index);

extern SDL_JoystickGUID SDL_JoystickGetDeviceGUID(int device_index);

extern Uint16 SDL_JoystickGetDeviceVendor(int device_index);

extern Uint16 SDL_JoystickGetDeviceProduct(int device_index);

extern Uint16 SDL_JoystickGetDeviceProductVersion(int device_index);

extern SDL_JoystickType SDL_JoystickGetDeviceType(int device_index);

extern SDL_JoystickID SDL_JoystickGetDeviceInstanceID(int device_index);
extern SDL_Joystick * SDL_JoystickOpen(int device_index);

extern SDL_Joystick * SDL_JoystickFromInstanceID(SDL_JoystickID instance_id);

extern SDL_Joystick * SDL_JoystickFromPlayerIndex(int player_index);

extern const char * SDL_JoystickName(SDL_Joystick * joystick);

extern int SDL_JoystickGetPlayerIndex(SDL_Joystick * joystick);

extern void SDL_JoystickSetPlayerIndex(SDL_Joystick * joystick, int player_index);

extern SDL_JoystickGUID SDL_JoystickGetGUID(SDL_Joystick * joystick);

extern Uint16 SDL_JoystickGetVendor(SDL_Joystick * joystick);

extern Uint16 SDL_JoystickGetProduct(SDL_Joystick * joystick);

extern Uint16 SDL_JoystickGetProductVersion(SDL_Joystick * joystick);

extern SDL_JoystickType SDL_JoystickGetType(SDL_Joystick * joystick);

extern void SDL_JoystickGetGUIDString(SDL_JoystickGUID guid, char *pszGUID, int cbGUID);

extern SDL_JoystickGUID SDL_JoystickGetGUIDFromString(const char *pchGUID);

extern SDL_bool SDL_JoystickGetAttached(SDL_Joystick * joystick);

extern SDL_JoystickID SDL_JoystickInstanceID(SDL_Joystick * joystick);

extern int SDL_JoystickNumAxes(SDL_Joystick * joystick);

extern int SDL_JoystickNumBalls(SDL_Joystick * joystick);

extern int SDL_JoystickNumHats(SDL_Joystick * joystick);

extern int SDL_JoystickNumButtons(SDL_Joystick * joystick);

extern void SDL_JoystickUpdate(void);
extern int SDL_JoystickEventState(int state);
#define SDL_JOYSTICK_AXIS_MAX 32767
#define SDL_JOYSTICK_AXIS_MIN -32768

extern Sint16 SDL_JoystickGetAxis(SDL_Joystick * joystick,
                                                   int axis);
extern SDL_bool SDL_JoystickGetAxisInitialState(SDL_Joystick * joystick,
                                                   int axis, Sint16 *state);
#define SDL_HAT_CENTERED 0x00
#define SDL_HAT_UP 0x01
#define SDL_HAT_RIGHT 0x02
#define SDL_HAT_DOWN 0x04
#define SDL_HAT_LEFT 0x08
#define SDL_HAT_RIGHTUP (SDL_HAT_RIGHT|SDL_HAT_UP)
#define SDL_HAT_RIGHTDOWN (SDL_HAT_RIGHT|SDL_HAT_DOWN)
#define SDL_HAT_LEFTUP (SDL_HAT_LEFT|SDL_HAT_UP)
#define SDL_HAT_LEFTDOWN (SDL_HAT_LEFT|SDL_HAT_DOWN)
extern Uint8 SDL_JoystickGetHat(SDL_Joystick * joystick,
                                                 int hat);
extern int SDL_JoystickGetBall(SDL_Joystick * joystick,
                                                int ball, int *dx, int *dy);

extern Uint8 SDL_JoystickGetButton(SDL_Joystick * joystick,
                                                    int button);
extern int SDL_JoystickRumble(SDL_Joystick * joystick, Uint16 low_frequency_rumble, Uint16 high_frequency_rumble, Uint32 duration_ms);

extern void SDL_JoystickClose(SDL_Joystick * joystick);

extern SDL_JoystickPowerLevel SDL_JoystickCurrentPowerLevel(SDL_Joystick * joystick);
#define SDL_gamecontroller_h_ 

struct _SDL_GameController;
typedef struct _SDL_GameController SDL_GameController;

typedef enum
{
    SDL_CONTROLLER_TYPE_UNKNOWN = 0,
    SDL_CONTROLLER_TYPE_XBOX360,
    SDL_CONTROLLER_TYPE_XBOXONE,
    SDL_CONTROLLER_TYPE_PS3,
    SDL_CONTROLLER_TYPE_PS4,
    SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_PRO
} SDL_GameControllerType;

typedef enum
{
    SDL_CONTROLLER_BINDTYPE_NONE = 0,
    SDL_CONTROLLER_BINDTYPE_BUTTON,
    SDL_CONTROLLER_BINDTYPE_AXIS,
    SDL_CONTROLLER_BINDTYPE_HAT
} SDL_GameControllerBindType;

typedef struct SDL_GameControllerButtonBind
{
    SDL_GameControllerBindType bindType;
    union
    {
        int button;
        int axis;
        struct {
            int hat;
            int hat_mask;
        } hat;
    } value;
} SDL_GameControllerButtonBind;
extern int SDL_GameControllerAddMappingsFromRW(SDL_RWops * rw, int freerw);
#define SDL_GameControllerAddMappingsFromFile(file) SDL_GameControllerAddMappingsFromRW(SDL_RWFromFile(file, "rb"), 1)

extern int SDL_GameControllerAddMapping(const char* mappingString);

extern int SDL_GameControllerNumMappings(void);

extern char * SDL_GameControllerMappingForIndex(int mapping_index);

extern char * SDL_GameControllerMappingForGUID(SDL_JoystickGUID guid);

extern char * SDL_GameControllerMapping(SDL_GameController * gamecontroller);

extern SDL_bool SDL_IsGameController(int joystick_index);

extern const char * SDL_GameControllerNameForIndex(int joystick_index);

extern SDL_GameControllerType SDL_GameControllerTypeForIndex(int joystick_index);

extern char * SDL_GameControllerMappingForDeviceIndex(int joystick_index);
extern SDL_GameController * SDL_GameControllerOpen(int joystick_index);

extern SDL_GameController * SDL_GameControllerFromInstanceID(SDL_JoystickID joyid);

extern SDL_GameController * SDL_GameControllerFromPlayerIndex(int player_index);

extern const char * SDL_GameControllerName(SDL_GameController *gamecontroller);

extern SDL_GameControllerType SDL_GameControllerGetType(SDL_GameController *gamecontroller);

extern int SDL_GameControllerGetPlayerIndex(SDL_GameController *gamecontroller);

extern void SDL_GameControllerSetPlayerIndex(SDL_GameController *gamecontroller, int player_index);

extern Uint16 SDL_GameControllerGetVendor(SDL_GameController * gamecontroller);

extern Uint16 SDL_GameControllerGetProduct(SDL_GameController * gamecontroller);

extern Uint16 SDL_GameControllerGetProductVersion(SDL_GameController * gamecontroller);

extern SDL_bool SDL_GameControllerGetAttached(SDL_GameController *gamecontroller);

extern SDL_Joystick * SDL_GameControllerGetJoystick(SDL_GameController *gamecontroller);
extern int SDL_GameControllerEventState(int state);

extern void SDL_GameControllerUpdate(void);
typedef enum
{
    SDL_CONTROLLER_AXIS_INVALID = -1,
    SDL_CONTROLLER_AXIS_LEFTX,
    SDL_CONTROLLER_AXIS_LEFTY,
    SDL_CONTROLLER_AXIS_RIGHTX,
    SDL_CONTROLLER_AXIS_RIGHTY,
    SDL_CONTROLLER_AXIS_TRIGGERLEFT,
    SDL_CONTROLLER_AXIS_TRIGGERRIGHT,
    SDL_CONTROLLER_AXIS_MAX
} SDL_GameControllerAxis;

extern SDL_GameControllerAxis SDL_GameControllerGetAxisFromString(const char *pchString);

extern const char* SDL_GameControllerGetStringForAxis(SDL_GameControllerAxis axis);

extern SDL_GameControllerButtonBind
SDL_GameControllerGetBindForAxis(SDL_GameController *gamecontroller,
                                 SDL_GameControllerAxis axis);
extern Sint16
SDL_GameControllerGetAxis(SDL_GameController *gamecontroller,
                          SDL_GameControllerAxis axis);

typedef enum
{
    SDL_CONTROLLER_BUTTON_INVALID = -1,
    SDL_CONTROLLER_BUTTON_A,
    SDL_CONTROLLER_BUTTON_B,
    SDL_CONTROLLER_BUTTON_X,
    SDL_CONTROLLER_BUTTON_Y,
    SDL_CONTROLLER_BUTTON_BACK,
    SDL_CONTROLLER_BUTTON_GUIDE,
    SDL_CONTROLLER_BUTTON_START,
    SDL_CONTROLLER_BUTTON_LEFTSTICK,
    SDL_CONTROLLER_BUTTON_RIGHTSTICK,
    SDL_CONTROLLER_BUTTON_LEFTSHOULDER,
    SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
    SDL_CONTROLLER_BUTTON_DPAD_UP,
    SDL_CONTROLLER_BUTTON_DPAD_DOWN,
    SDL_CONTROLLER_BUTTON_DPAD_LEFT,
    SDL_CONTROLLER_BUTTON_DPAD_RIGHT,
    SDL_CONTROLLER_BUTTON_MAX
} SDL_GameControllerButton;

extern SDL_GameControllerButton SDL_GameControllerGetButtonFromString(const char *pchString);

extern const char* SDL_GameControllerGetStringForButton(SDL_GameControllerButton button);

extern SDL_GameControllerButtonBind
SDL_GameControllerGetBindForButton(SDL_GameController *gamecontroller,
                                   SDL_GameControllerButton button);

extern Uint8 SDL_GameControllerGetButton(SDL_GameController *gamecontroller,
                                                          SDL_GameControllerButton button);
extern int SDL_GameControllerRumble(SDL_GameController *gamecontroller, Uint16 low_frequency_rumble, Uint16 high_frequency_rumble, Uint32 duration_ms);

extern void SDL_GameControllerClose(SDL_GameController *gamecontroller);
#define SDL_quit_h_ 
#define SDL_QuitRequested() (SDL_PumpEvents(), (SDL_PeepEvents(NULL,0,SDL_PEEKEVENT,SDL_QUIT,SDL_QUIT) > 0))
#define SDL_gesture_h_ 
#define SDL_touch_h_ 


typedef Sint64 SDL_TouchID;
typedef Sint64 SDL_FingerID;

typedef enum
{
    SDL_TOUCH_DEVICE_INVALID = -1,
    SDL_TOUCH_DEVICE_DIRECT,
    SDL_TOUCH_DEVICE_INDIRECT_ABSOLUTE,
    SDL_TOUCH_DEVICE_INDIRECT_RELATIVE
} SDL_TouchDeviceType;

typedef struct SDL_Finger
{
    SDL_FingerID id;
    float x;
    float y;
    float pressure;
} SDL_Finger;
#define SDL_TOUCH_MOUSEID ((Uint32)-1)
#define SDL_MOUSE_TOUCHID ((Sint64)-1)

extern int SDL_GetNumTouchDevices(void);

extern SDL_TouchID SDL_GetTouchDevice(int index);

extern SDL_TouchDeviceType SDL_GetTouchDeviceType(SDL_TouchID touchID);

extern int SDL_GetNumTouchFingers(SDL_TouchID touchID);

extern SDL_Finger * SDL_GetTouchFinger(SDL_TouchID touchID, int index);


typedef Sint64 SDL_GestureID;
extern int SDL_RecordGesture(SDL_TouchID touchId);

extern int SDL_SaveAllDollarTemplates(SDL_RWops *dst);

extern int SDL_SaveDollarTemplate(SDL_GestureID gestureId,SDL_RWops *dst);

extern int SDL_LoadDollarTemplates(SDL_TouchID touchId, SDL_RWops *src);

#define SDL_RELEASED 0
#define SDL_PRESSED 1

typedef enum
{
    SDL_FIRSTEVENT = 0,
    SDL_QUIT = 0x100,
    SDL_APP_TERMINATING,
    SDL_APP_LOWMEMORY,
    SDL_APP_WILLENTERBACKGROUND,
    SDL_APP_DIDENTERBACKGROUND,
    SDL_APP_WILLENTERFOREGROUND,
    SDL_APP_DIDENTERFOREGROUND,
    SDL_DISPLAYEVENT = 0x150,
    SDL_WINDOWEVENT = 0x200,
    SDL_SYSWMEVENT,
    SDL_KEYDOWN = 0x300,
    SDL_KEYUP,
    SDL_TEXTEDITING,
    SDL_TEXTINPUT,
    SDL_KEYMAPCHANGED,
    SDL_MOUSEMOTION = 0x400,
    SDL_MOUSEBUTTONDOWN,
    SDL_MOUSEBUTTONUP,
    SDL_MOUSEWHEEL,
    SDL_JOYAXISMOTION = 0x600,
    SDL_JOYBALLMOTION,
    SDL_JOYHATMOTION,
    SDL_JOYBUTTONDOWN,
    SDL_JOYBUTTONUP,
    SDL_JOYDEVICEADDED,
    SDL_JOYDEVICEREMOVED,
    SDL_CONTROLLERAXISMOTION = 0x650,
    SDL_CONTROLLERBUTTONDOWN,
    SDL_CONTROLLERBUTTONUP,
    SDL_CONTROLLERDEVICEADDED,
    SDL_CONTROLLERDEVICEREMOVED,
    SDL_CONTROLLERDEVICEREMAPPED,
    SDL_FINGERDOWN = 0x700,
    SDL_FINGERUP,
    SDL_FINGERMOTION,
    SDL_DOLLARGESTURE = 0x800,
    SDL_DOLLARRECORD,
    SDL_MULTIGESTURE,
    SDL_CLIPBOARDUPDATE = 0x900,
    SDL_DROPFILE = 0x1000,
    SDL_DROPTEXT,
    SDL_DROPBEGIN,
    SDL_DROPCOMPLETE,
    SDL_AUDIODEVICEADDED = 0x1100,
    SDL_AUDIODEVICEREMOVED,
    SDL_SENSORUPDATE = 0x1200,
    SDL_RENDER_TARGETS_RESET = 0x2000,
    SDL_RENDER_DEVICE_RESET,
    SDL_USEREVENT = 0x8000,
    SDL_LASTEVENT = 0xFFFF
} SDL_EventType;

typedef struct SDL_CommonEvent
{
    Uint32 type;
    Uint32 timestamp;
} SDL_CommonEvent;

typedef struct SDL_DisplayEvent
{
    Uint32 type;
    Uint32 timestamp;
    Uint32 display;
    Uint8 event;
    Uint8 padding1;
    Uint8 padding2;
    Uint8 padding3;
    Sint32 data1;
} SDL_DisplayEvent;

typedef struct SDL_WindowEvent
{
    Uint32 type;
    Uint32 timestamp;
    Uint32 windowID;
    Uint8 event;
    Uint8 padding1;
    Uint8 padding2;
    Uint8 padding3;
    Sint32 data1;
    Sint32 data2;
} SDL_WindowEvent;

typedef struct SDL_KeyboardEvent
{
    Uint32 type;
    Uint32 timestamp;
    Uint32 windowID;
    Uint8 state;
    Uint8 repeat;
    Uint8 padding2;
    Uint8 padding3;
    SDL_Keysym keysym;
} SDL_KeyboardEvent;
#define SDL_TEXTEDITINGEVENT_TEXT_SIZE (32)

typedef struct SDL_TextEditingEvent
{
    Uint32 type;
    Uint32 timestamp;
    Uint32 windowID;
    char text[(32)];
    Sint32 start;
    Sint32 length;
} SDL_TextEditingEvent;
#define SDL_TEXTINPUTEVENT_TEXT_SIZE (32)

typedef struct SDL_TextInputEvent
{
    Uint32 type;
    Uint32 timestamp;
    Uint32 windowID;
    char text[(32)];
} SDL_TextInputEvent;

typedef struct SDL_MouseMotionEvent
{
    Uint32 type;
    Uint32 timestamp;
    Uint32 windowID;
    Uint32 which;
    Uint32 state;
    Sint32 x;
    Sint32 y;
    Sint32 xrel;
    Sint32 yrel;
} SDL_MouseMotionEvent;

typedef struct SDL_MouseButtonEvent
{
    Uint32 type;
    Uint32 timestamp;
    Uint32 windowID;
    Uint32 which;
    Uint8 button;
    Uint8 state;
    Uint8 clicks;
    Uint8 padding1;
    Sint32 x;
    Sint32 y;
} SDL_MouseButtonEvent;

typedef struct SDL_MouseWheelEvent
{
    Uint32 type;
    Uint32 timestamp;
    Uint32 windowID;
    Uint32 which;
    Sint32 x;
    Sint32 y;
    Uint32 direction;
} SDL_MouseWheelEvent;

typedef struct SDL_JoyAxisEvent
{
    Uint32 type;
    Uint32 timestamp;
    SDL_JoystickID which;
    Uint8 axis;
    Uint8 padding1;
    Uint8 padding2;
    Uint8 padding3;
    Sint16 value;
    Uint16 padding4;
} SDL_JoyAxisEvent;

typedef struct SDL_JoyBallEvent
{
    Uint32 type;
    Uint32 timestamp;
    SDL_JoystickID which;
    Uint8 ball;
    Uint8 padding1;
    Uint8 padding2;
    Uint8 padding3;
    Sint16 xrel;
    Sint16 yrel;
} SDL_JoyBallEvent;

typedef struct SDL_JoyHatEvent
{
    Uint32 type;
    Uint32 timestamp;
    SDL_JoystickID which;
    Uint8 hat;
    Uint8 value;
    Uint8 padding1;
    Uint8 padding2;
} SDL_JoyHatEvent;

typedef struct SDL_JoyButtonEvent
{
    Uint32 type;
    Uint32 timestamp;
    SDL_JoystickID which;
    Uint8 button;
    Uint8 state;
    Uint8 padding1;
    Uint8 padding2;
} SDL_JoyButtonEvent;

typedef struct SDL_JoyDeviceEvent
{
    Uint32 type;
    Uint32 timestamp;
    Sint32 which;
} SDL_JoyDeviceEvent;

typedef struct SDL_ControllerAxisEvent
{
    Uint32 type;
    Uint32 timestamp;
    SDL_JoystickID which;
    Uint8 axis;
    Uint8 padding1;
    Uint8 padding2;
    Uint8 padding3;
    Sint16 value;
    Uint16 padding4;
} SDL_ControllerAxisEvent;

typedef struct SDL_ControllerButtonEvent
{
    Uint32 type;
    Uint32 timestamp;
    SDL_JoystickID which;
    Uint8 button;
    Uint8 state;
    Uint8 padding1;
    Uint8 padding2;
} SDL_ControllerButtonEvent;

typedef struct SDL_ControllerDeviceEvent
{
    Uint32 type;
    Uint32 timestamp;
    Sint32 which;
} SDL_ControllerDeviceEvent;

typedef struct SDL_AudioDeviceEvent
{
    Uint32 type;
    Uint32 timestamp;
    Uint32 which;
    Uint8 iscapture;
    Uint8 padding1;
    Uint8 padding2;
    Uint8 padding3;
} SDL_AudioDeviceEvent;

typedef struct SDL_TouchFingerEvent
{
    Uint32 type;
    Uint32 timestamp;
    SDL_TouchID touchId;
    SDL_FingerID fingerId;
    float x;
    float y;
    float dx;
    float dy;
    float pressure;
    Uint32 windowID;
} SDL_TouchFingerEvent;

typedef struct SDL_MultiGestureEvent
{
    Uint32 type;
    Uint32 timestamp;
    SDL_TouchID touchId;
    float dTheta;
    float dDist;
    float x;
    float y;
    Uint16 numFingers;
    Uint16 padding;
} SDL_MultiGestureEvent;

typedef struct SDL_DollarGestureEvent
{
    Uint32 type;
    Uint32 timestamp;
    SDL_TouchID touchId;
    SDL_GestureID gestureId;
    Uint32 numFingers;
    float error;
    float x;
    float y;
} SDL_DollarGestureEvent;

typedef struct SDL_DropEvent
{
    Uint32 type;
    Uint32 timestamp;
    char *file;
    Uint32 windowID;
} SDL_DropEvent;

typedef struct SDL_SensorEvent
{
    Uint32 type;
    Uint32 timestamp;
    Sint32 which;
    float data[6];
} SDL_SensorEvent;

typedef struct SDL_QuitEvent
{
    Uint32 type;
    Uint32 timestamp;
} SDL_QuitEvent;

typedef struct SDL_OSEvent
{
    Uint32 type;
    Uint32 timestamp;
} SDL_OSEvent;

typedef struct SDL_UserEvent
{
    Uint32 type;
    Uint32 timestamp;
    Uint32 windowID;
    Sint32 code;
    void *data1;
    void *data2;
} SDL_UserEvent;

struct SDL_SysWMmsg;
typedef struct SDL_SysWMmsg SDL_SysWMmsg;

typedef struct SDL_SysWMEvent
{
    Uint32 type;
    Uint32 timestamp;
    SDL_SysWMmsg *msg;
} SDL_SysWMEvent;

typedef union SDL_Event
{
    Uint32 type;
    SDL_CommonEvent common;
    SDL_DisplayEvent display;
    SDL_WindowEvent window;
    SDL_KeyboardEvent key;
    SDL_TextEditingEvent edit;
    SDL_TextInputEvent text;
    SDL_MouseMotionEvent motion;
    SDL_MouseButtonEvent button;
    SDL_MouseWheelEvent wheel;
    SDL_JoyAxisEvent jaxis;
    SDL_JoyBallEvent jball;
    SDL_JoyHatEvent jhat;
    SDL_JoyButtonEvent jbutton;
    SDL_JoyDeviceEvent jdevice;
    SDL_ControllerAxisEvent caxis;
    SDL_ControllerButtonEvent cbutton;
    SDL_ControllerDeviceEvent cdevice;
    SDL_AudioDeviceEvent adevice;
    SDL_SensorEvent sensor;
    SDL_QuitEvent quit;
    SDL_UserEvent user;
    SDL_SysWMEvent syswm;
    SDL_TouchFingerEvent tfinger;
    SDL_MultiGestureEvent mgesture;
    SDL_DollarGestureEvent dgesture;
    SDL_DropEvent drop;
    Uint8 padding[56];
} SDL_Event;

typedef int SDL_compile_time_assert_SDL_Event[(sizeof(SDL_Event) == 56) * 2 - 1];
extern void SDL_PumpEvents(void);

typedef enum
{
    SDL_ADDEVENT,
    SDL_PEEKEVENT,
    SDL_GETEVENT
} SDL_eventaction;
extern int SDL_PeepEvents(SDL_Event * events, int numevents,
                                           SDL_eventaction action,
                                           Uint32 minType, Uint32 maxType);

extern SDL_bool SDL_HasEvent(Uint32 type);
extern SDL_bool SDL_HasEvents(Uint32 minType, Uint32 maxType);

extern void SDL_FlushEvent(Uint32 type);
extern void SDL_FlushEvents(Uint32 minType, Uint32 maxType);
extern int SDL_PollEvent(SDL_Event * event);
extern int SDL_WaitEvent(SDL_Event * event);
extern int SDL_WaitEventTimeout(SDL_Event * event,
                                                 int timeout);

extern int SDL_PushEvent(SDL_Event * event);

typedef int ( * SDL_EventFilter) (void *userdata, SDL_Event * event);
extern void SDL_SetEventFilter(SDL_EventFilter filter,
                                                void *userdata);

extern SDL_bool SDL_GetEventFilter(SDL_EventFilter * filter,
                                                    void **userdata);

extern void SDL_AddEventWatch(SDL_EventFilter filter,
                                               void *userdata);

extern void SDL_DelEventWatch(SDL_EventFilter filter,
                                               void *userdata);

extern void SDL_FilterEvents(SDL_EventFilter filter,
                                              void *userdata);
#define SDL_QUERY -1
#define SDL_IGNORE 0
#define SDL_DISABLE 0
#define SDL_ENABLE 1
extern Uint8 SDL_EventState(Uint32 type, int state);
#define SDL_GetEventState(type) SDL_EventState(type, SDL_QUERY)
extern Uint32 SDL_RegisterEvents(int numevents);
#define SDL_filesystem_h_ 

extern char * SDL_GetBasePath(void);
extern char * SDL_GetPrefPath(const char *org, const char *app);
#define SDL_haptic_h_ 

struct _SDL_Haptic;
typedef struct _SDL_Haptic SDL_Haptic;
#define SDL_HAPTIC_CONSTANT (1u<<0)
#define SDL_HAPTIC_SINE (1u<<1)
#define SDL_HAPTIC_LEFTRIGHT (1u<<2)
#define SDL_HAPTIC_TRIANGLE (1u<<3)
#define SDL_HAPTIC_SAWTOOTHUP (1u<<4)
#define SDL_HAPTIC_SAWTOOTHDOWN (1u<<5)
#define SDL_HAPTIC_RAMP (1u<<6)
#define SDL_HAPTIC_SPRING (1u<<7)
#define SDL_HAPTIC_DAMPER (1u<<8)
#define SDL_HAPTIC_INERTIA (1u<<9)
#define SDL_HAPTIC_FRICTION (1u<<10)
#define SDL_HAPTIC_CUSTOM (1u<<11)
#define SDL_HAPTIC_GAIN (1u<<12)
#define SDL_HAPTIC_AUTOCENTER (1u<<13)
#define SDL_HAPTIC_STATUS (1u<<14)
#define SDL_HAPTIC_PAUSE (1u<<15)
#define SDL_HAPTIC_POLAR 0
#define SDL_HAPTIC_CARTESIAN 1
#define SDL_HAPTIC_SPHERICAL 2
#define SDL_HAPTIC_INFINITY 4294967295U
typedef struct SDL_HapticDirection
{
    Uint8 type;
    Sint32 dir[3];
} SDL_HapticDirection;
typedef struct SDL_HapticConstant
{
    Uint16 type;
    SDL_HapticDirection direction;
    Uint32 length;
    Uint16 delay;
    Uint16 button;
    Uint16 interval;
    Sint16 level;
    Uint16 attack_length;
    Uint16 attack_level;
    Uint16 fade_length;
    Uint16 fade_level;
} SDL_HapticConstant;
typedef struct SDL_HapticPeriodic
{
    Uint16 type;
    SDL_HapticDirection direction;
    Uint32 length;
    Uint16 delay;
    Uint16 button;
    Uint16 interval;
    Uint16 period;
    Sint16 magnitude;
    Sint16 offset;
    Uint16 phase;
    Uint16 attack_length;
    Uint16 attack_level;
    Uint16 fade_length;
    Uint16 fade_level;
} SDL_HapticPeriodic;
typedef struct SDL_HapticCondition
{
    Uint16 type;
    SDL_HapticDirection direction;
    Uint32 length;
    Uint16 delay;
    Uint16 button;
    Uint16 interval;
    Uint16 right_sat[3];
    Uint16 left_sat[3];
    Sint16 right_coeff[3];
    Sint16 left_coeff[3];
    Uint16 deadband[3];
    Sint16 center[3];
} SDL_HapticCondition;
typedef struct SDL_HapticRamp
{
    Uint16 type;
    SDL_HapticDirection direction;
    Uint32 length;
    Uint16 delay;
    Uint16 button;
    Uint16 interval;
    Sint16 start;
    Sint16 end;
    Uint16 attack_length;
    Uint16 attack_level;
    Uint16 fade_length;
    Uint16 fade_level;
} SDL_HapticRamp;
typedef struct SDL_HapticLeftRight
{
    Uint16 type;
    Uint32 length;
    Uint16 large_magnitude;
    Uint16 small_magnitude;
} SDL_HapticLeftRight;
typedef struct SDL_HapticCustom
{
    Uint16 type;
    SDL_HapticDirection direction;
    Uint32 length;
    Uint16 delay;
    Uint16 button;
    Uint16 interval;
    Uint8 channels;
    Uint16 period;
    Uint16 samples;
    Uint16 *data;
    Uint16 attack_length;
    Uint16 attack_level;
    Uint16 fade_length;
    Uint16 fade_level;
} SDL_HapticCustom;
typedef union SDL_HapticEffect
{
    Uint16 type;
    SDL_HapticConstant constant;
    SDL_HapticPeriodic periodic;
    SDL_HapticCondition condition;
    SDL_HapticRamp ramp;
    SDL_HapticLeftRight leftright;
    SDL_HapticCustom custom;
} SDL_HapticEffect;
extern int SDL_NumHaptics(void);
extern const char * SDL_HapticName(int device_index);
extern SDL_Haptic * SDL_HapticOpen(int device_index);
extern int SDL_HapticOpened(int device_index);
extern int SDL_HapticIndex(SDL_Haptic * haptic);
extern int SDL_MouseIsHaptic(void);
extern SDL_Haptic * SDL_HapticOpenFromMouse(void);
extern int SDL_JoystickIsHaptic(SDL_Joystick * joystick);
extern SDL_Haptic * SDL_HapticOpenFromJoystick(SDL_Joystick *
                                                               joystick);

extern void SDL_HapticClose(SDL_Haptic * haptic);
extern int SDL_HapticNumEffects(SDL_Haptic * haptic);
extern int SDL_HapticNumEffectsPlaying(SDL_Haptic * haptic);
extern unsigned int SDL_HapticQuery(SDL_Haptic * haptic);

extern int SDL_HapticNumAxes(SDL_Haptic * haptic);
extern int SDL_HapticEffectSupported(SDL_Haptic * haptic,
                                                      SDL_HapticEffect *
                                                      effect);
extern int SDL_HapticNewEffect(SDL_Haptic * haptic,
                                                SDL_HapticEffect * effect);
extern int SDL_HapticUpdateEffect(SDL_Haptic * haptic,
                                                   int effect,
                                                   SDL_HapticEffect * data);
extern int SDL_HapticRunEffect(SDL_Haptic * haptic,
                                                int effect,
                                                Uint32 iterations);
extern int SDL_HapticStopEffect(SDL_Haptic * haptic,
                                                 int effect);
extern void SDL_HapticDestroyEffect(SDL_Haptic * haptic,
                                                     int effect);
extern int SDL_HapticGetEffectStatus(SDL_Haptic * haptic,
                                                      int effect);
extern int SDL_HapticSetGain(SDL_Haptic * haptic, int gain);
extern int SDL_HapticSetAutocenter(SDL_Haptic * haptic,
                                                    int autocenter);
extern int SDL_HapticPause(SDL_Haptic * haptic);
extern int SDL_HapticUnpause(SDL_Haptic * haptic);

extern int SDL_HapticStopAll(SDL_Haptic * haptic);
extern int SDL_HapticRumbleSupported(SDL_Haptic * haptic);
extern int SDL_HapticRumbleInit(SDL_Haptic * haptic);
extern int SDL_HapticRumblePlay(SDL_Haptic * haptic, float strength, Uint32 length );
extern int SDL_HapticRumbleStop(SDL_Haptic * haptic);
#define SDL_hints_h_ 

#define SDL_HINT_FRAMEBUFFER_ACCELERATION "SDL_FRAMEBUFFER_ACCELERATION"
#define SDL_HINT_RENDER_DRIVER "SDL_RENDER_DRIVER"
#define SDL_HINT_RENDER_OPENGL_SHADERS "SDL_RENDER_OPENGL_SHADERS"
#define SDL_HINT_RENDER_DIRECT3D_THREADSAFE "SDL_RENDER_DIRECT3D_THREADSAFE"
#define SDL_HINT_RENDER_DIRECT3D11_DEBUG "SDL_RENDER_DIRECT3D11_DEBUG"
#define SDL_HINT_RENDER_LOGICAL_SIZE_MODE "SDL_RENDER_LOGICAL_SIZE_MODE"
#define SDL_HINT_RENDER_SCALE_QUALITY "SDL_RENDER_SCALE_QUALITY"
#define SDL_HINT_RENDER_VSYNC "SDL_RENDER_VSYNC"
#define SDL_HINT_VIDEO_ALLOW_SCREENSAVER "SDL_VIDEO_ALLOW_SCREENSAVER"
#define SDL_HINT_VIDEO_EXTERNAL_CONTEXT "SDL_VIDEO_EXTERNAL_CONTEXT"
#define SDL_HINT_VIDEO_X11_XVIDMODE "SDL_VIDEO_X11_XVIDMODE"
#define SDL_HINT_VIDEO_X11_XINERAMA "SDL_VIDEO_X11_XINERAMA"
#define SDL_HINT_VIDEO_X11_XRANDR "SDL_VIDEO_X11_XRANDR"
#define SDL_HINT_VIDEO_X11_WINDOW_VISUALID "SDL_VIDEO_X11_WINDOW_VISUALID"
#define SDL_HINT_VIDEO_X11_NET_WM_PING "SDL_VIDEO_X11_NET_WM_PING"
#define SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR "SDL_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR"
#define SDL_HINT_VIDEO_X11_FORCE_EGL "SDL_VIDEO_X11_FORCE_EGL"
#define SDL_HINT_WINDOW_FRAME_USABLE_WHILE_CURSOR_HIDDEN "SDL_WINDOW_FRAME_USABLE_WHILE_CURSOR_HIDDEN"
#define SDL_HINT_WINDOWS_INTRESOURCE_ICON "SDL_WINDOWS_INTRESOURCE_ICON"
#define SDL_HINT_WINDOWS_INTRESOURCE_ICON_SMALL "SDL_WINDOWS_INTRESOURCE_ICON_SMALL"
#define SDL_HINT_WINDOWS_ENABLE_MESSAGELOOP "SDL_WINDOWS_ENABLE_MESSAGELOOP"
#define SDL_HINT_GRAB_KEYBOARD "SDL_GRAB_KEYBOARD"
#define SDL_HINT_MOUSE_DOUBLE_CLICK_TIME "SDL_MOUSE_DOUBLE_CLICK_TIME"
#define SDL_HINT_MOUSE_DOUBLE_CLICK_RADIUS "SDL_MOUSE_DOUBLE_CLICK_RADIUS"
#define SDL_HINT_MOUSE_NORMAL_SPEED_SCALE "SDL_MOUSE_NORMAL_SPEED_SCALE"
#define SDL_HINT_MOUSE_RELATIVE_SPEED_SCALE "SDL_MOUSE_RELATIVE_SPEED_SCALE"
#define SDL_HINT_MOUSE_RELATIVE_MODE_WARP "SDL_MOUSE_RELATIVE_MODE_WARP"
#define SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH "SDL_MOUSE_FOCUS_CLICKTHROUGH"
#define SDL_HINT_TOUCH_MOUSE_EVENTS "SDL_TOUCH_MOUSE_EVENTS"
#define SDL_HINT_MOUSE_TOUCH_EVENTS "SDL_MOUSE_TOUCH_EVENTS"
#define SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS "SDL_VIDEO_MINIMIZE_ON_FOCUS_LOSS"
#define SDL_HINT_IDLE_TIMER_DISABLED "SDL_IOS_IDLE_TIMER_DISABLED"
#define SDL_HINT_ORIENTATIONS "SDL_IOS_ORIENTATIONS"
#define SDL_HINT_APPLE_TV_CONTROLLER_UI_EVENTS "SDL_APPLE_TV_CONTROLLER_UI_EVENTS"
#define SDL_HINT_APPLE_TV_REMOTE_ALLOW_ROTATION "SDL_APPLE_TV_REMOTE_ALLOW_ROTATION"
#define SDL_HINT_IOS_HIDE_HOME_INDICATOR "SDL_IOS_HIDE_HOME_INDICATOR"
#define SDL_HINT_ACCELEROMETER_AS_JOYSTICK "SDL_ACCELEROMETER_AS_JOYSTICK"
#define SDL_HINT_TV_REMOTE_AS_JOYSTICK "SDL_TV_REMOTE_AS_JOYSTICK"
#define SDL_HINT_XINPUT_ENABLED "SDL_XINPUT_ENABLED"
#define SDL_HINT_XINPUT_USE_OLD_JOYSTICK_MAPPING "SDL_XINPUT_USE_OLD_JOYSTICK_MAPPING"
#define SDL_HINT_GAMECONTROLLERTYPE "SDL_GAMECONTROLLERTYPE"
#define SDL_HINT_GAMECONTROLLERCONFIG "SDL_GAMECONTROLLERCONFIG"
#define SDL_HINT_GAMECONTROLLERCONFIG_FILE "SDL_GAMECONTROLLERCONFIG_FILE"
#define SDL_HINT_GAMECONTROLLER_IGNORE_DEVICES "SDL_GAMECONTROLLER_IGNORE_DEVICES"
#define SDL_HINT_GAMECONTROLLER_IGNORE_DEVICES_EXCEPT "SDL_GAMECONTROLLER_IGNORE_DEVICES_EXCEPT"
#define SDL_HINT_GAMECONTROLLER_USE_BUTTON_LABELS "SDL_GAMECONTROLLER_USE_BUTTON_LABELS"
#define SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS "SDL_JOYSTICK_ALLOW_BACKGROUND_EVENTS"
#define SDL_HINT_JOYSTICK_HIDAPI "SDL_JOYSTICK_HIDAPI"
#define SDL_HINT_JOYSTICK_HIDAPI_PS4 "SDL_JOYSTICK_HIDAPI_PS4"
#define SDL_HINT_JOYSTICK_HIDAPI_PS4_RUMBLE "SDL_JOYSTICK_HIDAPI_PS4_RUMBLE"
#define SDL_HINT_JOYSTICK_HIDAPI_STEAM "SDL_JOYSTICK_HIDAPI_STEAM"
#define SDL_HINT_JOYSTICK_HIDAPI_SWITCH "SDL_JOYSTICK_HIDAPI_SWITCH"
#define SDL_HINT_JOYSTICK_HIDAPI_XBOX "SDL_JOYSTICK_HIDAPI_XBOX"
#define SDL_HINT_JOYSTICK_HIDAPI_GAMECUBE "SDL_JOYSTICK_HIDAPI_GAMECUBE"
#define SDL_HINT_ENABLE_STEAM_CONTROLLERS "SDL_ENABLE_STEAM_CONTROLLERS"
#define SDL_HINT_ALLOW_TOPMOST "SDL_ALLOW_TOPMOST"
#define SDL_HINT_TIMER_RESOLUTION "SDL_TIMER_RESOLUTION"
#define SDL_HINT_QTWAYLAND_CONTENT_ORIENTATION "SDL_QTWAYLAND_CONTENT_ORIENTATION"
#define SDL_HINT_QTWAYLAND_WINDOW_FLAGS "SDL_QTWAYLAND_WINDOW_FLAGS"
#define SDL_HINT_THREAD_STACK_SIZE "SDL_THREAD_STACK_SIZE"
#define SDL_HINT_VIDEO_HIGHDPI_DISABLED "SDL_VIDEO_HIGHDPI_DISABLED"
#define SDL_HINT_MAC_CTRL_CLICK_EMULATE_RIGHT_CLICK "SDL_MAC_CTRL_CLICK_EMULATE_RIGHT_CLICK"
#define SDL_HINT_VIDEO_WIN_D3DCOMPILER "SDL_VIDEO_WIN_D3DCOMPILER"
#define SDL_HINT_VIDEO_WINDOW_SHARE_PIXEL_FORMAT "SDL_VIDEO_WINDOW_SHARE_PIXEL_FORMAT"
#define SDL_HINT_WINRT_PRIVACY_POLICY_URL "SDL_WINRT_PRIVACY_POLICY_URL"
#define SDL_HINT_WINRT_PRIVACY_POLICY_LABEL "SDL_WINRT_PRIVACY_POLICY_LABEL"
#define SDL_HINT_WINRT_HANDLE_BACK_BUTTON "SDL_WINRT_HANDLE_BACK_BUTTON"
#define SDL_HINT_VIDEO_MAC_FULLSCREEN_SPACES "SDL_VIDEO_MAC_FULLSCREEN_SPACES"
#define SDL_HINT_MAC_BACKGROUND_APP "SDL_MAC_BACKGROUND_APP"
#define SDL_HINT_ANDROID_APK_EXPANSION_MAIN_FILE_VERSION "SDL_ANDROID_APK_EXPANSION_MAIN_FILE_VERSION"
#define SDL_HINT_ANDROID_APK_EXPANSION_PATCH_FILE_VERSION "SDL_ANDROID_APK_EXPANSION_PATCH_FILE_VERSION"
#define SDL_HINT_IME_INTERNAL_EDITING "SDL_IME_INTERNAL_EDITING"
#define SDL_HINT_ANDROID_TRAP_BACK_BUTTON "SDL_ANDROID_TRAP_BACK_BUTTON"
#define SDL_HINT_ANDROID_BLOCK_ON_PAUSE "SDL_ANDROID_BLOCK_ON_PAUSE"
#define SDL_HINT_RETURN_KEY_HIDES_IME "SDL_RETURN_KEY_HIDES_IME"
#define SDL_HINT_EMSCRIPTEN_KEYBOARD_ELEMENT "SDL_EMSCRIPTEN_KEYBOARD_ELEMENT"
#define SDL_HINT_NO_SIGNAL_HANDLERS "SDL_NO_SIGNAL_HANDLERS"
#define SDL_HINT_WINDOWS_NO_CLOSE_ON_ALT_F4 "SDL_WINDOWS_NO_CLOSE_ON_ALT_F4"
#define SDL_HINT_BMP_SAVE_LEGACY_FORMAT "SDL_BMP_SAVE_LEGACY_FORMAT"
#define SDL_HINT_WINDOWS_DISABLE_THREAD_NAMING "SDL_WINDOWS_DISABLE_THREAD_NAMING"
#define SDL_HINT_RPI_VIDEO_LAYER "SDL_RPI_VIDEO_LAYER"
#define SDL_HINT_VIDEO_DOUBLE_BUFFER "SDL_VIDEO_DOUBLE_BUFFER"
#define SDL_HINT_OPENGL_ES_DRIVER "SDL_OPENGL_ES_DRIVER"
#define SDL_HINT_AUDIO_RESAMPLING_MODE "SDL_AUDIO_RESAMPLING_MODE"
#define SDL_HINT_AUDIO_CATEGORY "SDL_AUDIO_CATEGORY"
#define SDL_HINT_RENDER_BATCHING "SDL_RENDER_BATCHING"
#define SDL_HINT_EVENT_LOGGING "SDL_EVENT_LOGGING"
#define SDL_HINT_WAVE_RIFF_CHUNK_SIZE "SDL_WAVE_RIFF_CHUNK_SIZE"
#define SDL_HINT_WAVE_TRUNCATION "SDL_WAVE_TRUNCATION"
#define SDL_HINT_WAVE_FACT_CHUNK "SDL_WAVE_FACT_CHUNK"
#define SDL_HINT_DISPLAY_USABLE_BOUNDS "SDL_DISPLAY_USABLE_BOUNDS"

typedef enum
{
    SDL_HINT_DEFAULT,
    SDL_HINT_NORMAL,
    SDL_HINT_OVERRIDE
} SDL_HintPriority;
extern SDL_bool SDL_SetHintWithPriority(const char *name,
                                                         const char *value,
                                                         SDL_HintPriority priority);

extern SDL_bool SDL_SetHint(const char *name,
                                             const char *value);

extern const char * SDL_GetHint(const char *name);

extern SDL_bool SDL_GetHintBoolean(const char *name, SDL_bool default_value);

typedef void ( *SDL_HintCallback)(void *userdata, const char *name, const char *oldValue, const char *newValue);
extern void SDL_AddHintCallback(const char *name,
                                                 SDL_HintCallback callback,
                                                 void *userdata);
extern void SDL_DelHintCallback(const char *name,
                                                 SDL_HintCallback callback,
                                                 void *userdata);

extern void SDL_ClearHints(void);
#define SDL_loadso_h_ 

extern void * SDL_LoadObject(const char *sofile);

extern void * SDL_LoadFunction(void *handle,
                                               const char *name);

extern void SDL_UnloadObject(void *handle);
#define SDL_log_h_ 

#define SDL_MAX_LOG_MESSAGE 4096
typedef enum
{
    SDL_LOG_CATEGORY_APPLICATION,
    SDL_LOG_CATEGORY_ERROR,
    SDL_LOG_CATEGORY_ASSERT,
    SDL_LOG_CATEGORY_SYSTEM,
    SDL_LOG_CATEGORY_AUDIO,
    SDL_LOG_CATEGORY_VIDEO,
    SDL_LOG_CATEGORY_RENDER,
    SDL_LOG_CATEGORY_INPUT,
    SDL_LOG_CATEGORY_TEST,
    SDL_LOG_CATEGORY_RESERVED1,
    SDL_LOG_CATEGORY_RESERVED2,
    SDL_LOG_CATEGORY_RESERVED3,
    SDL_LOG_CATEGORY_RESERVED4,
    SDL_LOG_CATEGORY_RESERVED5,
    SDL_LOG_CATEGORY_RESERVED6,
    SDL_LOG_CATEGORY_RESERVED7,
    SDL_LOG_CATEGORY_RESERVED8,
    SDL_LOG_CATEGORY_RESERVED9,
    SDL_LOG_CATEGORY_RESERVED10,
    SDL_LOG_CATEGORY_CUSTOM
} SDL_LogCategory;

typedef enum
{
    SDL_LOG_PRIORITY_VERBOSE = 1,
    SDL_LOG_PRIORITY_DEBUG,
    SDL_LOG_PRIORITY_INFO,
    SDL_LOG_PRIORITY_WARN,
    SDL_LOG_PRIORITY_ERROR,
    SDL_LOG_PRIORITY_CRITICAL,
    SDL_NUM_LOG_PRIORITIES
} SDL_LogPriority;

extern void SDL_LogSetAllPriority(SDL_LogPriority priority);

extern void SDL_LogSetPriority(int category,
                                                SDL_LogPriority priority);

extern SDL_LogPriority SDL_LogGetPriority(int category);

extern void SDL_LogResetPriorities(void);

extern void SDL_Log( const char *fmt, ...) ;

extern void SDL_LogVerbose(int category, const char *fmt, ...) ;

extern void SDL_LogDebug(int category, const char *fmt, ...) ;

extern void SDL_LogInfo(int category, const char *fmt, ...) ;

extern void SDL_LogWarn(int category, const char *fmt, ...) ;

extern void SDL_LogError(int category, const char *fmt, ...) ;

extern void SDL_LogCritical(int category, const char *fmt, ...) ;

extern void SDL_LogMessage(int category,
                                            SDL_LogPriority priority,
                                            const char *fmt, ...) ;

extern void SDL_LogMessageV(int category,
                                             SDL_LogPriority priority,
                                             const char *fmt, va_list ap);

typedef void ( *SDL_LogOutputFunction)(void *userdata, int category, SDL_LogPriority priority, const char *message);

extern void SDL_LogGetOutputFunction(SDL_LogOutputFunction *callback, void **userdata);

extern void SDL_LogSetOutputFunction(SDL_LogOutputFunction callback, void *userdata);
#define SDL_messagebox_h_ 

typedef enum
{
    SDL_MESSAGEBOX_ERROR = 0x00000010,
    SDL_MESSAGEBOX_WARNING = 0x00000020,
    SDL_MESSAGEBOX_INFORMATION = 0x00000040,
    SDL_MESSAGEBOX_BUTTONS_LEFT_TO_RIGHT = 0x00000080,
    SDL_MESSAGEBOX_BUTTONS_RIGHT_TO_LEFT = 0x00000100
} SDL_MessageBoxFlags;

typedef enum
{
    SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT = 0x00000001,
    SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT = 0x00000002
} SDL_MessageBoxButtonFlags;

typedef struct
{
    Uint32 flags;
    int buttonid;
    const char * text;
} SDL_MessageBoxButtonData;

typedef struct
{
    Uint8 r, g, b;
} SDL_MessageBoxColor;

typedef enum
{
    SDL_MESSAGEBOX_COLOR_BACKGROUND,
    SDL_MESSAGEBOX_COLOR_TEXT,
    SDL_MESSAGEBOX_COLOR_BUTTON_BORDER,
    SDL_MESSAGEBOX_COLOR_BUTTON_BACKGROUND,
    SDL_MESSAGEBOX_COLOR_BUTTON_SELECTED,
    SDL_MESSAGEBOX_COLOR_MAX
} SDL_MessageBoxColorType;

typedef struct
{
    SDL_MessageBoxColor colors[SDL_MESSAGEBOX_COLOR_MAX];
} SDL_MessageBoxColorScheme;

typedef struct
{
    Uint32 flags;
    SDL_Window *window;
    const char *title;
    const char *message;
    int numbuttons;
    const SDL_MessageBoxButtonData *buttons;
    const SDL_MessageBoxColorScheme *colorScheme;
} SDL_MessageBoxData;
extern int SDL_ShowMessageBox(const SDL_MessageBoxData *messageboxdata, int *buttonid);
extern int SDL_ShowSimpleMessageBox(Uint32 flags, const char *title, const char *message, SDL_Window *window);
#define SDL_metal_h_ 

typedef void *SDL_MetalView;
extern SDL_MetalView SDL_Metal_CreateView(SDL_Window * window);
extern void SDL_Metal_DestroyView(SDL_MetalView view);
#define SDL_power_h_ 

typedef enum
{
    SDL_POWERSTATE_UNKNOWN,
    SDL_POWERSTATE_ON_BATTERY,
    SDL_POWERSTATE_NO_BATTERY,
    SDL_POWERSTATE_CHARGING,
    SDL_POWERSTATE_CHARGED
} SDL_PowerState;
extern SDL_PowerState SDL_GetPowerInfo(int *secs, int *pct);
#define SDL_render_h_ 

typedef enum
{
    SDL_RENDERER_SOFTWARE = 0x00000001,
    SDL_RENDERER_ACCELERATED = 0x00000002,
    SDL_RENDERER_PRESENTVSYNC = 0x00000004,
    SDL_RENDERER_TARGETTEXTURE = 0x00000008
} SDL_RendererFlags;

typedef struct SDL_RendererInfo
{
    const char *name;
    Uint32 flags;
    Uint32 num_texture_formats;
    Uint32 texture_formats[16];
    int max_texture_width;
    int max_texture_height;
} SDL_RendererInfo;

typedef enum
{
    SDL_ScaleModeNearest,
    SDL_ScaleModeLinear,
    SDL_ScaleModeBest
} SDL_ScaleMode;

typedef enum
{
    SDL_TEXTUREACCESS_STATIC,
    SDL_TEXTUREACCESS_STREAMING,
    SDL_TEXTUREACCESS_TARGET
} SDL_TextureAccess;

typedef enum
{
    SDL_TEXTUREMODULATE_NONE = 0x00000000,
    SDL_TEXTUREMODULATE_COLOR = 0x00000001,
    SDL_TEXTUREMODULATE_ALPHA = 0x00000002
} SDL_TextureModulate;

typedef enum
{
    SDL_FLIP_NONE = 0x00000000,
    SDL_FLIP_HORIZONTAL = 0x00000001,
    SDL_FLIP_VERTICAL = 0x00000002
} SDL_RendererFlip;

struct SDL_Renderer;
typedef struct SDL_Renderer SDL_Renderer;

struct SDL_Texture;
typedef struct SDL_Texture SDL_Texture;
extern int SDL_GetNumRenderDrivers(void);
extern int SDL_GetRenderDriverInfo(int index,
                                                    SDL_RendererInfo * info);
extern int SDL_CreateWindowAndRenderer(
                                int width, int height, Uint32 window_flags,
                                SDL_Window **window, SDL_Renderer **renderer);
extern SDL_Renderer * SDL_CreateRenderer(SDL_Window * window,
                                               int index, Uint32 flags);
extern SDL_Renderer * SDL_CreateSoftwareRenderer(SDL_Surface * surface);

extern SDL_Renderer * SDL_GetRenderer(SDL_Window * window);

extern int SDL_GetRendererInfo(SDL_Renderer * renderer,
                                                SDL_RendererInfo * info);

extern int SDL_GetRendererOutputSize(SDL_Renderer * renderer,
                                                      int *w, int *h);
extern SDL_Texture * SDL_CreateTexture(SDL_Renderer * renderer,
                                                        Uint32 format,
                                                        int access, int w,
                                                        int h);
extern SDL_Texture * SDL_CreateTextureFromSurface(SDL_Renderer * renderer, SDL_Surface * surface);
extern int SDL_QueryTexture(SDL_Texture * texture,
                                             Uint32 * format, int *access,
                                             int *w, int *h);
extern int SDL_SetTextureColorMod(SDL_Texture * texture,
                                                   Uint8 r, Uint8 g, Uint8 b);
extern int SDL_GetTextureColorMod(SDL_Texture * texture,
                                                   Uint8 * r, Uint8 * g,
                                                   Uint8 * b);
extern int SDL_SetTextureAlphaMod(SDL_Texture * texture,
                                                   Uint8 alpha);
extern int SDL_GetTextureAlphaMod(SDL_Texture * texture,
                                                   Uint8 * alpha);
extern int SDL_SetTextureBlendMode(SDL_Texture * texture,
                                                    SDL_BlendMode blendMode);
extern int SDL_GetTextureBlendMode(SDL_Texture * texture,
                                                    SDL_BlendMode *blendMode);
extern int SDL_SetTextureScaleMode(SDL_Texture * texture,
                                                    SDL_ScaleMode scaleMode);
extern int SDL_GetTextureScaleMode(SDL_Texture * texture,
                                                    SDL_ScaleMode *scaleMode);
extern int SDL_UpdateTexture(SDL_Texture * texture,
                                              const SDL_Rect * rect,
                                              const void *pixels, int pitch);
extern int SDL_UpdateYUVTexture(SDL_Texture * texture,
                                                 const SDL_Rect * rect,
                                                 const Uint8 *Yplane, int Ypitch,
                                                 const Uint8 *Uplane, int Upitch,
                                                 const Uint8 *Vplane, int Vpitch);
extern int SDL_LockTexture(SDL_Texture * texture,
                                            const SDL_Rect * rect,
                                            void **pixels, int *pitch);
extern int SDL_LockTextureToSurface(SDL_Texture *texture,
                                            const SDL_Rect *rect,
                                            SDL_Surface **surface);
extern void SDL_UnlockTexture(SDL_Texture * texture);
extern SDL_bool SDL_RenderTargetSupported(SDL_Renderer *renderer);
extern int SDL_SetRenderTarget(SDL_Renderer *renderer,
                                                SDL_Texture *texture);
extern SDL_Texture * SDL_GetRenderTarget(SDL_Renderer *renderer);
extern int SDL_RenderSetLogicalSize(SDL_Renderer * renderer, int w, int h);
extern void SDL_RenderGetLogicalSize(SDL_Renderer * renderer, int *w, int *h);
extern int SDL_RenderSetIntegerScale(SDL_Renderer * renderer,
                                                      SDL_bool enable);
extern SDL_bool SDL_RenderGetIntegerScale(SDL_Renderer * renderer);
extern int SDL_RenderSetViewport(SDL_Renderer * renderer,
                                                  const SDL_Rect * rect);

extern void SDL_RenderGetViewport(SDL_Renderer * renderer,
                                                   SDL_Rect * rect);
extern int SDL_RenderSetClipRect(SDL_Renderer * renderer,
                                                  const SDL_Rect * rect);
extern void SDL_RenderGetClipRect(SDL_Renderer * renderer,
                                                   SDL_Rect * rect);
extern SDL_bool SDL_RenderIsClipEnabled(SDL_Renderer * renderer);
extern int SDL_RenderSetScale(SDL_Renderer * renderer,
                                               float scaleX, float scaleY);
extern void SDL_RenderGetScale(SDL_Renderer * renderer,
                                               float *scaleX, float *scaleY);
extern int SDL_SetRenderDrawColor(SDL_Renderer * renderer,
                                           Uint8 r, Uint8 g, Uint8 b,
                                           Uint8 a);
extern int SDL_GetRenderDrawColor(SDL_Renderer * renderer,
                                           Uint8 * r, Uint8 * g, Uint8 * b,
                                           Uint8 * a);
extern int SDL_SetRenderDrawBlendMode(SDL_Renderer * renderer,
                                                       SDL_BlendMode blendMode);
extern int SDL_GetRenderDrawBlendMode(SDL_Renderer * renderer,
                                                       SDL_BlendMode *blendMode);
extern int SDL_RenderClear(SDL_Renderer * renderer);
extern int SDL_RenderDrawPoint(SDL_Renderer * renderer,
                                                int x, int y);
extern int SDL_RenderDrawPoints(SDL_Renderer * renderer,
                                                 const SDL_Point * points,
                                                 int count);
extern int SDL_RenderDrawLine(SDL_Renderer * renderer,
                                               int x1, int y1, int x2, int y2);
extern int SDL_RenderDrawLines(SDL_Renderer * renderer,
                                                const SDL_Point * points,
                                                int count);
extern int SDL_RenderDrawRect(SDL_Renderer * renderer,
                                               const SDL_Rect * rect);
extern int SDL_RenderDrawRects(SDL_Renderer * renderer,
                                                const SDL_Rect * rects,
                                                int count);
extern int SDL_RenderFillRect(SDL_Renderer * renderer,
                                               const SDL_Rect * rect);
extern int SDL_RenderFillRects(SDL_Renderer * renderer,
                                                const SDL_Rect * rects,
                                                int count);
extern int SDL_RenderCopy(SDL_Renderer * renderer,
                                           SDL_Texture * texture,
                                           const SDL_Rect * srcrect,
                                           const SDL_Rect * dstrect);
extern int SDL_RenderCopyEx(SDL_Renderer * renderer,
                                           SDL_Texture * texture,
                                           const SDL_Rect * srcrect,
                                           const SDL_Rect * dstrect,
                                           const double angle,
                                           const SDL_Point *center,
                                           const SDL_RendererFlip flip);
extern int SDL_RenderDrawPointF(SDL_Renderer * renderer,
                                                 float x, float y);
extern int SDL_RenderDrawPointsF(SDL_Renderer * renderer,
                                                  const SDL_FPoint * points,
                                                  int count);
extern int SDL_RenderDrawLineF(SDL_Renderer * renderer,
                                                float x1, float y1, float x2, float y2);
extern int SDL_RenderDrawLinesF(SDL_Renderer * renderer,
                                                const SDL_FPoint * points,
                                                int count);
extern int SDL_RenderDrawRectF(SDL_Renderer * renderer,
                                               const SDL_FRect * rect);
extern int SDL_RenderDrawRectsF(SDL_Renderer * renderer,
                                                 const SDL_FRect * rects,
                                                 int count);
extern int SDL_RenderFillRectF(SDL_Renderer * renderer,
                                                const SDL_FRect * rect);
extern int SDL_RenderFillRectsF(SDL_Renderer * renderer,
                                                 const SDL_FRect * rects,
                                                 int count);
extern int SDL_RenderCopyF(SDL_Renderer * renderer,
                                            SDL_Texture * texture,
                                            const SDL_Rect * srcrect,
                                            const SDL_FRect * dstrect);
extern int SDL_RenderCopyExF(SDL_Renderer * renderer,
                                            SDL_Texture * texture,
                                            const SDL_Rect * srcrect,
                                            const SDL_FRect * dstrect,
                                            const double angle,
                                            const SDL_FPoint *center,
                                            const SDL_RendererFlip flip);
extern int SDL_RenderReadPixels(SDL_Renderer * renderer,
                                                 const SDL_Rect * rect,
                                                 Uint32 format,
                                                 void *pixels, int pitch);

extern void SDL_RenderPresent(SDL_Renderer * renderer);

extern void SDL_DestroyTexture(SDL_Texture * texture);

extern void SDL_DestroyRenderer(SDL_Renderer * renderer);
extern int SDL_RenderFlush(SDL_Renderer * renderer);
extern int SDL_GL_BindTexture(SDL_Texture *texture, float *texw, float *texh);
extern int SDL_GL_UnbindTexture(SDL_Texture *texture);
extern void * SDL_RenderGetMetalLayer(SDL_Renderer * renderer);
extern void * SDL_RenderGetMetalCommandEncoder(SDL_Renderer * renderer);
#define SDL_sensor_h_ 

struct _SDL_Sensor;
typedef struct _SDL_Sensor SDL_Sensor;

typedef Sint32 SDL_SensorID;
typedef enum
{
    SDL_SENSOR_INVALID = -1,
    SDL_SENSOR_UNKNOWN,
    SDL_SENSOR_ACCEL,
    SDL_SENSOR_GYRO
} SDL_SensorType;
#define SDL_STANDARD_GRAVITY 9.80665f
extern int SDL_NumSensors(void);
extern const char * SDL_SensorGetDeviceName(int device_index);
extern SDL_SensorType SDL_SensorGetDeviceType(int device_index);
extern int SDL_SensorGetDeviceNonPortableType(int device_index);
extern SDL_SensorID SDL_SensorGetDeviceInstanceID(int device_index);
extern SDL_Sensor * SDL_SensorOpen(int device_index);

extern SDL_Sensor * SDL_SensorFromInstanceID(SDL_SensorID instance_id);

extern const char * SDL_SensorGetName(SDL_Sensor *sensor);
extern SDL_SensorType SDL_SensorGetType(SDL_Sensor *sensor);
extern int SDL_SensorGetNonPortableType(SDL_Sensor *sensor);
extern SDL_SensorID SDL_SensorGetInstanceID(SDL_Sensor *sensor);
extern int SDL_SensorGetData(SDL_Sensor * sensor, float *data, int num_values);

extern void SDL_SensorClose(SDL_Sensor * sensor);
extern void SDL_SensorUpdate(void);
#define SDL_shape_h_ 

#define SDL_NONSHAPEABLE_WINDOW -1
#define SDL_INVALID_SHAPE_ARGUMENT -2
#define SDL_WINDOW_LACKS_SHAPE -3
extern SDL_Window * SDL_CreateShapedWindow(const char *title,unsigned int x,unsigned int y,unsigned int w,unsigned int h,Uint32 flags);
extern SDL_bool SDL_IsShapedWindow(const SDL_Window *window);

typedef enum {
    ShapeModeDefault,
    ShapeModeBinarizeAlpha,
    ShapeModeReverseBinarizeAlpha,
    ShapeModeColorKey
} WindowShapeMode;
#define SDL_SHAPEMODEALPHA(mode) (mode == ShapeModeDefault || mode == ShapeModeBinarizeAlpha || mode == ShapeModeReverseBinarizeAlpha)

typedef union {
    Uint8 binarizationCutoff;
    SDL_Color colorKey;
} SDL_WindowShapeParams;

typedef struct SDL_WindowShapeMode {
    WindowShapeMode mode;
    SDL_WindowShapeParams parameters;
} SDL_WindowShapeMode;
extern int SDL_SetWindowShape(SDL_Window *window,SDL_Surface *shape,SDL_WindowShapeMode *shape_mode);
extern int SDL_GetShapedWindowMode(SDL_Window *window,SDL_WindowShapeMode *shape_mode);
#define SDL_system_h_ 

typedef void ( * SDL_WindowsMessageHook)(void *userdata, void *hWnd, unsigned int message, Uint64 wParam, Sint64 lParam);
extern void SDL_SetWindowsMessageHook(SDL_WindowsMessageHook callback, void *userdata);

extern int SDL_Direct3D9GetAdapterIndex( int displayIndex );

typedef struct IDirect3DDevice9 IDirect3DDevice9;

extern IDirect3DDevice9* SDL_RenderGetD3D9Device(SDL_Renderer * renderer);

extern SDL_bool SDL_DXGIGetOutputInfo( int displayIndex, int *adapterIndex, int *outputIndex );
extern SDL_bool SDL_IsTablet(void);

extern void SDL_OnApplicationWillTerminate(void);
extern void SDL_OnApplicationDidReceiveMemoryWarning(void);
extern void SDL_OnApplicationWillResignActive(void);
extern void SDL_OnApplicationDidEnterBackground(void);
extern void SDL_OnApplicationWillEnterForeground(void);
extern void SDL_OnApplicationDidBecomeActive(void);
#define SDL_timer_h_ 

extern Uint32 SDL_GetTicks(void);
#define SDL_TICKS_PASSED(A,B) ((Sint32)((B) - (A)) <= 0)

extern Uint64 SDL_GetPerformanceCounter(void);

extern Uint64 SDL_GetPerformanceFrequency(void);

extern void SDL_Delay(Uint32 ms);
typedef Uint32 ( * SDL_TimerCallback) (Uint32 interval, void *param);

typedef int SDL_TimerID;

extern SDL_TimerID SDL_AddTimer(Uint32 interval,
                                                 SDL_TimerCallback callback,
                                                 void *param);
extern SDL_bool SDL_RemoveTimer(SDL_TimerID id);
#define SDL_version_h_ 

typedef struct SDL_version
{
    Uint8 major;
    Uint8 minor;
    Uint8 patch;
} SDL_version;
#define SDL_MAJOR_VERSION 2
#define SDL_MINOR_VERSION 0
#define SDL_PATCHLEVEL 12
#define SDL_VERSION(x) { (x)->major = SDL_MAJOR_VERSION; (x)->minor = SDL_MINOR_VERSION; (x)->patch = SDL_PATCHLEVEL; }
#define SDL_VERSIONNUM(X,Y,Z) ((X)*1000 + (Y)*100 + (Z))
#define SDL_COMPILEDVERSION SDL_VERSIONNUM(SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL)
#define SDL_VERSION_ATLEAST(X,Y,Z) (SDL_COMPILEDVERSION >= SDL_VERSIONNUM(X, Y, Z))
extern void SDL_GetVersion(SDL_version * ver);
extern const char * SDL_GetRevision(void);
extern int SDL_GetRevisionNumber(void);

#define SDL_INIT_TIMER 0x00000001u
#define SDL_INIT_AUDIO 0x00000010u
#define SDL_INIT_VIDEO 0x00000020u
#define SDL_INIT_JOYSTICK 0x00000200u
#define SDL_INIT_HAPTIC 0x00001000u
#define SDL_INIT_GAMECONTROLLER 0x00002000u
#define SDL_INIT_EVENTS 0x00004000u
#define SDL_INIT_SENSOR 0x00008000u
#define SDL_INIT_NOPARACHUTE 0x00100000u
#define SDL_INIT_EVERYTHING ( SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC | SDL_INIT_GAMECONTROLLER | SDL_INIT_SENSOR )
extern int SDL_Init(Uint32 flags);
extern int SDL_InitSubSystem(Uint32 flags);

extern void SDL_QuitSubSystem(Uint32 flags);

extern Uint32 SDL_WasInit(Uint32 flags);

extern void SDL_Quit(void);
#define FFTOOLS_CMDUTILS_H 

extern const char program_name[];

extern const int program_birth_year;

extern AVCodecContext *avcodec_opts[AVMEDIA_TYPE_NB];
extern AVFormatContext *avformat_opts;
extern AVDictionary *sws_dict;
extern AVDictionary *swr_opts;
extern AVDictionary *format_opts, *codec_opts, *resample_opts;
extern int hide_banner;

void register_exit(void (*cb)(int ret));

void exit_program(int ret) ;

void init_dynload(void);

void init_opts(void);

void uninit_opts(void);

void log_callback_help(void* ptr, int level, const char* fmt, va_list vl);

int opt_cpuflags(void *optctx, const char *opt, const char *arg);

int opt_default(void *optctx, const char *opt, const char *arg);

int opt_loglevel(void *optctx, const char *opt, const char *arg);

int opt_report(void *optctx, const char *opt, const char *arg);

int opt_max_alloc(void *optctx, const char *opt, const char *arg);

int opt_codec_debug(void *optctx, const char *opt, const char *arg);

int opt_timelimit(void *optctx, const char *opt, const char *arg);
double parse_number_or_die(const char *context, const char *numstr, int type,
                           double min, double max);
int64_t parse_time_or_die(const char *context, const char *timestr,
                          int is_duration);

typedef struct SpecifierOpt {
    char *specifier;
    union {
        uint8_t *str;
        int i;
        int64_t i64;
        uint64_t ui64;
        float f;
        double dbl;
    } u;
} SpecifierOpt;

typedef struct OptionDef {
    const char *name;
    int flags;
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
#define OPT_PERFILE 0x2000
#define OPT_OFFSET 0x4000
#define OPT_SPEC 0x8000
#define OPT_TIME 0x10000
#define OPT_DOUBLE 0x20000
#define OPT_INPUT 0x40000
#define OPT_OUTPUT 0x80000
     union {
        void *dst_ptr;
        int (*func_arg)(void *, const char *, const char *);
        size_t off;
    } u;
    const char *help;
    const char *argname;
} OptionDef;
void show_help_options(const OptionDef *options, const char *msg, int req_flags,
                       int rej_flags, int alt_flags);
#define CMDUTILS_COMMON_OPTIONS_AVDEVICE { "sources" , OPT_EXIT | HAS_ARG, { .func_arg = show_sources }, "list sources of the input device", "device" }, { "sinks" , OPT_EXIT | HAS_ARG, { .func_arg = show_sinks }, "list sinks of the output device", "device" },
#define CMDUTILS_COMMON_OPTIONS { "L", OPT_EXIT, { .func_arg = show_license }, "show license" }, { "h", OPT_EXIT, { .func_arg = show_help }, "show help", "topic" }, { "?", OPT_EXIT, { .func_arg = show_help }, "show help", "topic" }, { "help", OPT_EXIT, { .func_arg = show_help }, "show help", "topic" }, { "-help", OPT_EXIT, { .func_arg = show_help }, "show help", "topic" }, { "version", OPT_EXIT, { .func_arg = show_version }, "show version" }, { "buildconf", OPT_EXIT, { .func_arg = show_buildconf }, "show build configuration" }, { "formats", OPT_EXIT, { .func_arg = show_formats }, "show available formats" }, { "muxers", OPT_EXIT, { .func_arg = show_muxers }, "show available muxers" }, { "demuxers", OPT_EXIT, { .func_arg = show_demuxers }, "show available demuxers" }, { "devices", OPT_EXIT, { .func_arg = show_devices }, "show available devices" }, { "codecs", OPT_EXIT, { .func_arg = show_codecs }, "show available codecs" }, { "decoders", OPT_EXIT, { .func_arg = show_decoders }, "show available decoders" }, { "encoders", OPT_EXIT, { .func_arg = show_encoders }, "show available encoders" }, { "bsfs", OPT_EXIT, { .func_arg = show_bsfs }, "show available bit stream filters" }, { "protocols", OPT_EXIT, { .func_arg = show_protocols }, "show available protocols" }, { "filters", OPT_EXIT, { .func_arg = show_filters }, "show available filters" }, { "pix_fmts", OPT_EXIT, { .func_arg = show_pix_fmts }, "show available pixel formats" }, { "layouts", OPT_EXIT, { .func_arg = show_layouts }, "show standard channel layouts" }, { "sample_fmts", OPT_EXIT, { .func_arg = show_sample_fmts }, "show available audio sample formats" }, { "colors", OPT_EXIT, { .func_arg = show_colors }, "show available color names" }, { "loglevel", HAS_ARG, { .func_arg = opt_loglevel }, "set logging level", "loglevel" }, { "v", HAS_ARG, { .func_arg = opt_loglevel }, "set logging level", "loglevel" }, { "report", 0, { .func_arg = opt_report }, "generate a report" }, { "max_alloc", HAS_ARG, { .func_arg = opt_max_alloc }, "set maximum size of a single allocated block", "bytes" }, { "cpuflags", HAS_ARG | OPT_EXPERT, { .func_arg = opt_cpuflags }, "force specific cpu flags", "flags" }, { "hide_banner", OPT_BOOL | OPT_EXPERT, {&hide_banner}, "do not show program banner", "hide_banner" }, CMDUTILS_COMMON_OPTIONS_AVDEVICE
void show_help_children(const AVClass *class, int flags);

void show_help_default(const char *opt, const char *arg);

int show_help(void *optctx, const char *opt, const char *arg);
void parse_options(void *optctx, int argc, char **argv, const OptionDef *options,
                   void (* parse_arg_function)(void *optctx, const char*));

int parse_option(void *optctx, const char *opt, const char *arg,
                 const OptionDef *options);

typedef struct Option {
    const OptionDef *opt;
    const char *key;
    const char *val;
} Option;

typedef struct OptionGroupDef {
    const char *name;
    const char *sep;
    int flags;
} OptionGroupDef;

typedef struct OptionGroup {
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

typedef struct OptionGroupList {
    const OptionGroupDef *group_def;
    OptionGroup *groups;
    int nb_groups;
} OptionGroupList;

typedef struct OptionParseContext {
    OptionGroup global_opts;
    OptionGroupList *groups;
    int nb_groups;
    OptionGroup cur_group;
} OptionParseContext;

int parse_optgroup(void *optctx, OptionGroup *g);
int split_commandline(OptionParseContext *octx, int argc, char *argv[],
                      const OptionDef *options,
                      const OptionGroupDef *groups, int nb_groups);

void uninit_parse_context(OptionParseContext *octx);

void parse_loglevel(int argc, char **argv, const OptionDef *options);

int locate_option(int argc, char **argv, const OptionDef *options,
                  const char *optname);
int check_stream_specifier(AVFormatContext *s, AVStream *st, const char *spec);
AVDictionary *filter_codec_opts(AVDictionary *opts, enum AVCodecID codec_id,
                                AVFormatContext *s, AVStream *st, AVCodec *codec);
AVDictionary **setup_find_stream_info_opts(AVFormatContext *s,
                                           AVDictionary *codec_opts);
void print_error(const char *filename, int err);

void show_banner(int argc, char **argv, const OptionDef *options);

int show_version(void *optctx, const char *opt, const char *arg);

int show_buildconf(void *optctx, const char *opt, const char *arg);

int show_license(void *optctx, const char *opt, const char *arg);

int show_formats(void *optctx, const char *opt, const char *arg);

int show_muxers(void *optctx, const char *opt, const char *arg);

int show_demuxers(void *optctx, const char *opt, const char *arg);

int show_devices(void *optctx, const char *opt, const char *arg);

int show_sinks(void *optctx, const char *opt, const char *arg);

int show_sources(void *optctx, const char *opt, const char *arg);

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

int read_yesno(void);
FILE *get_preset_file(char *filename, size_t filename_size,
                      const char *preset_name, int is_path, const char *codec_name);
void *grow_array(void *array, int elem_size, int *size, int new_size);
#define media_type_string av_get_media_type_string
#define GROW_ARRAY(array,nb_elems) array = grow_array(array, sizeof(*array), &nb_elems, nb_elems + 1)
#define GET_PIX_FMT_NAME(pix_fmt) const char *name = av_get_pix_fmt_name(pix_fmt);
#define GET_CODEC_NAME(id) const char *name = avcodec_descriptor_get(id)->name;
#define GET_SAMPLE_FMT_NAME(sample_fmt) const char *name = av_get_sample_fmt_name(sample_fmt)
#define GET_SAMPLE_RATE_NAME(rate) char name[16]; snprintf(name, sizeof(name), "%d", rate);
#define GET_CH_LAYOUT_NAME(ch_layout) char name[16]; snprintf(name, sizeof(name), "0x%"PRIx64, ch_layout);
#define GET_CH_LAYOUT_DESC(ch_layout) char name[128]; av_get_channel_layout_string(name, sizeof(name), 0, ch_layout);

double get_rotation(AVStream *st);

const char program_name[] = "ffplay";
const int program_birth_year = 2003;
#define MAX_QUEUE_SIZE (15 * 1024 * 1024)
#define MIN_FRAMES 25
#define EXTERNAL_CLOCK_MIN_FRAMES 2
#define EXTERNAL_CLOCK_MAX_FRAMES 10
#define SDL_AUDIO_MIN_BUFFER_SIZE 512
#define SDL_AUDIO_MAX_CALLBACKS_PER_SEC 30
#define SDL_VOLUME_STEP (0.75)
#define AV_SYNC_THRESHOLD_MIN 0.04
#define AV_SYNC_THRESHOLD_MAX 0.1
#define AV_SYNC_FRAMEDUP_THRESHOLD 0.1
#define AV_NOSYNC_THRESHOLD 10.0
#define SAMPLE_CORRECTION_PERCENT_MAX 10
#define EXTERNAL_CLOCK_SPEED_MIN 0.900
#define EXTERNAL_CLOCK_SPEED_MAX 1.010
#define EXTERNAL_CLOCK_SPEED_STEP 0.001
#define AUDIO_DIFF_AVG_NB 20
#define REFRESH_RATE 0.01
#define SAMPLE_ARRAY_SIZE (8 * 65536)
#define CURSOR_HIDE_DELAY 1000000
#define USE_ONEPASS_SUBTITLE_RENDER 1

static unsigned sws_flags = 4;

typedef struct MyAVPacketList {
    AVPacket pkt;
    struct MyAVPacketList *next;
    int serial;
} MyAVPacketList;

typedef struct PacketQueue {
    MyAVPacketList *first_pkt, *last_pkt;
    int nb_packets;
    int size;
    int64_t duration;
    int abort_request;
    int serial;
    SDL_mutex *mutex;
    SDL_cond *cond;
} PacketQueue;
#define VIDEO_PICTURE_QUEUE_SIZE 3
#define SUBPICTURE_QUEUE_SIZE 16
#define SAMPLE_QUEUE_SIZE 9
#define FRAME_QUEUE_SIZE FFMAX(SAMPLE_QUEUE_SIZE, FFMAX(VIDEO_PICTURE_QUEUE_SIZE, SUBPICTURE_QUEUE_SIZE))

typedef struct AudioParams {
    int freq;
    int channels;
    int64_t channel_layout;
    enum AVSampleFormat fmt;
    int frame_size;
    int bytes_per_sec;
} AudioParams;

typedef struct Clock {
    double pts;
    double pts_drift;
    double last_updated;
    double speed;
    int serial;
    int paused;
    int *queue_serial;
} Clock;

typedef struct Frame {
    AVFrame *frame;
    AVSubtitle sub;
    int serial;
    double pts;
    double duration;
    int64_t pos;
    int width;
    int height;
    int format;
    AVRational sar;
    int uploaded;
    int flip_v;
} Frame;

typedef struct FrameQueue {
    Frame queue[((9) > (((3) > (16) ? (3) : (16))) ? (9) : (((3) > (16) ? (3) : (16))))];
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

enum {
    AV_SYNC_AUDIO_MASTER,
    AV_SYNC_VIDEO_MASTER,
    AV_SYNC_EXTERNAL_CLOCK,
};

typedef struct Decoder {
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

typedef struct VideoState {
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
    double audio_diff_cum;
    double audio_diff_avg_coef;
    double audio_diff_threshold;
    int audio_diff_avg_count;
    AVStream *audio_st;
    PacketQueue audioq;
    int audio_hw_buf_size;
    uint8_t *audio_buf;
    uint8_t *audio_buf1;
    unsigned int audio_buf_size;
    unsigned int audio_buf1_size;
    int audio_buf_index;
    int audio_write_buf_size;
    int audio_volume;
    int muted;
    struct AudioParams audio_src;
    struct AudioParams audio_filter_src;
    struct AudioParams audio_tgt;
    struct SwrContext *swr_ctx;
    int frame_drops_early;
    int frame_drops_late;
    enum ShowMode {
        SHOW_MODE_NONE = -1, SHOW_MODE_VIDEO = 0, SHOW_MODE_WAVES, SHOW_MODE_RDFT, SHOW_MODE_NB
    } show_mode;
    int16_t sample_array[(8 * 65536)];
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
    double max_frame_duration;
    struct SwsContext *img_convert_ctx;
    struct SwsContext *sub_convert_ctx;
    int eof;
    char *filename;
    int width, height, xleft, ytop;
    int step;
    int vfilter_idx;
    AVFilterContext *in_video_filter;
    AVFilterContext *out_video_filter;
    AVFilterContext *in_audio_filter;
    AVFilterContext *out_audio_filter;
    AVFilterGraph *agraph;
    int last_video_stream, last_audio_stream, last_subtitle_stream;
    SDL_cond *continue_read_thread;
} VideoState;

static AVInputFormat *file_iformat;
static const char *input_filename;
static const char *window_title;
static int default_width = 640;
static int default_height = 480;
static int screen_width = 0;
static int screen_height = 0;
static int screen_left = (0x2FFF0000u|(0));
static int screen_top = (0x2FFF0000u|(0));
static int audio_disable;
static int video_disable;
static int subtitle_disable;
static const char* wanted_stream_spec[AVMEDIA_TYPE_NB] = {0};
static int seek_by_bytes = -1;
static float seek_interval = 10;
static int display_disable;
static int borderless;
static int alwaysontop;
static int startup_volume = 100;
static int show_status = -1;
static int av_sync_type = AV_SYNC_AUDIO_MASTER;
static int64_t start_time = ((int64_t)0x8000000000000000ULL);
static int64_t duration = ((int64_t)0x8000000000000000ULL);
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

static const char **vfilters_list = 
                                   ((void *)0)
                                       ;
static int nb_vfilters = 0;
static char *afilters = 
                       ((void *)0)
                           ;

static int autorotate = 1;
static int find_stream_info = 1;
static int filter_nbthreads = 0;

static int is_full_screen;
static int64_t audio_callback_time;

static AVPacket flush_pkt;
#define FF_QUIT_EVENT (SDL_USEREVENT + 2)

static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_RendererInfo renderer_info = {0};
static SDL_AudioDeviceID audio_dev;

static const struct TextureFormatEntry {
    enum AVPixelFormat format;
    int texture_fmt;
} sdl_texture_format_map[] = {
    { AV_PIX_FMT_RGB8, SDL_PIXELFORMAT_RGB332 },
    { AV_PIX_FMT_RGB444LE, SDL_PIXELFORMAT_RGB444 },
    { AV_PIX_FMT_RGB555LE, SDL_PIXELFORMAT_RGB555 },
    { AV_PIX_FMT_BGR555LE, SDL_PIXELFORMAT_BGR555 },
    { AV_PIX_FMT_RGB565LE, SDL_PIXELFORMAT_RGB565 },
    { AV_PIX_FMT_BGR565LE, SDL_PIXELFORMAT_BGR565 },
    { AV_PIX_FMT_RGB24, SDL_PIXELFORMAT_RGB24 },
    { AV_PIX_FMT_BGR24, SDL_PIXELFORMAT_BGR24 },
    { AV_PIX_FMT_BGR0, SDL_PIXELFORMAT_RGB888 },
    { AV_PIX_FMT_RGB0, SDL_PIXELFORMAT_BGR888 },
    { AV_PIX_FMT_0BGR, SDL_PIXELFORMAT_RGBX8888 },
    { AV_PIX_FMT_0RGB, SDL_PIXELFORMAT_BGRX8888 },
    { AV_PIX_FMT_BGRA, SDL_PIXELFORMAT_ARGB8888 },
    { AV_PIX_FMT_ABGR, SDL_PIXELFORMAT_RGBA8888 },
    { AV_PIX_FMT_RGBA, SDL_PIXELFORMAT_ABGR8888 },
    { AV_PIX_FMT_ARGB, SDL_PIXELFORMAT_BGRA8888 },
    { AV_PIX_FMT_YUV420P, SDL_PIXELFORMAT_IYUV },
    { AV_PIX_FMT_YUYV422, SDL_PIXELFORMAT_YUY2 },
    { AV_PIX_FMT_UYVY422, SDL_PIXELFORMAT_UYVY },
    { AV_PIX_FMT_NONE, SDL_PIXELFORMAT_UNKNOWN },
};

static int opt_add_vfilter(void *optctx, const char *opt, const char *arg)
{
    vfilters_list = grow_array(vfilters_list, sizeof(*vfilters_list), &nb_vfilters, nb_vfilters + 1);
    vfilters_list[nb_vfilters - 1] = arg;
    return 0;
}

static inline
int cmp_audio_fmts(enum AVSampleFormat fmt1, int64_t channel_count1,
                   enum AVSampleFormat fmt2, int64_t channel_count2)
{
    if (channel_count1 == 1 && channel_count2 == 1)
        return av_get_packed_sample_fmt(fmt1) != av_get_packed_sample_fmt(fmt2);
    else
        return channel_count1 != channel_count2 || fmt1 != fmt2;
}

static inline
int64_t get_valid_channel_layout(int64_t channel_layout, int channels)
{
    if (channel_layout && av_get_channel_layout_nb_channels(channel_layout) == channels)
        return channel_layout;
    else
        return 0;
}

static int packet_queue_put_private(PacketQueue *q, AVPacket *pkt)
{
    MyAVPacketList *pkt1;
    if (q->abort_request)
       return -1;
    pkt1 = av_malloc(sizeof(MyAVPacketList));
    if (!pkt1)
        return -1;
    pkt1->pkt = *pkt;
    pkt1->next = 
                ((void *)0)
                    ;
    if (pkt == &flush_pkt)
        q->serial++;
    pkt1->serial = q->serial;
    if (!q->last_pkt)
        q->first_pkt = pkt1;
    else
        q->last_pkt->next = pkt1;
    q->last_pkt = pkt1;
    q->nb_packets++;
    q->size += pkt1->pkt.size + sizeof(*pkt1);
    q->duration += pkt1->pkt.duration;
    SDL_CondSignal(q->cond);
    return 0;
}

static int packet_queue_put(PacketQueue *q, AVPacket *pkt)
{
    int ret;
    SDL_LockMutex(q->mutex);
    ret = packet_queue_put_private(q, pkt);
    SDL_UnlockMutex(q->mutex);
    if (pkt != &flush_pkt && ret < 0)
        av_packet_unref(pkt);
    return ret;
}

static int packet_queue_put_nullpacket(PacketQueue *q, int stream_index)
{
    AVPacket pkt1, *pkt = &pkt1;
    av_init_packet(pkt);
    pkt->data = 
               ((void *)0)
                   ;
    pkt->size = 0;
    pkt->stream_index = stream_index;
    return packet_queue_put(q, pkt);
}

static int packet_queue_init(PacketQueue *q)
{
    memset(q, 0, sizeof(PacketQueue));
    q->mutex = SDL_CreateMutex();
    if (!q->mutex) {
        av_log(
              ((void *)0)
                  , 8, "SDL_CreateMutex(): %s\n", SDL_GetError());
        return (-(
              12
              ));
    }
    q->cond = SDL_CreateCond();
    if (!q->cond) {
        av_log(
              ((void *)0)
                  , 8, "SDL_CreateCond(): %s\n", SDL_GetError());
        return (-(
              12
              ));
    }
    q->abort_request = 1;
    return 0;
}

static void packet_queue_flush(PacketQueue *q)
{
    MyAVPacketList *pkt, *pkt1;
    SDL_LockMutex(q->mutex);
    for (pkt = q->first_pkt; pkt; pkt = pkt1) {
        pkt1 = pkt->next;
        av_packet_unref(&pkt->pkt);
        av_freep(&pkt);
    }
    q->last_pkt = 
                 ((void *)0)
                     ;
    q->first_pkt = 
                  ((void *)0)
                      ;
    q->nb_packets = 0;
    q->size = 0;
    q->duration = 0;
    SDL_UnlockMutex(q->mutex);
}

static void packet_queue_destroy(PacketQueue *q)
{
    packet_queue_flush(q);
    SDL_DestroyMutex(q->mutex);
    SDL_DestroyCond(q->cond);
}

static void packet_queue_abort(PacketQueue *q)
{
    SDL_LockMutex(q->mutex);
    q->abort_request = 1;
    SDL_CondSignal(q->cond);
    SDL_UnlockMutex(q->mutex);
}

static void packet_queue_start(PacketQueue *q)
{
    SDL_LockMutex(q->mutex);
    q->abort_request = 0;
    packet_queue_put_private(q, &flush_pkt);
    SDL_UnlockMutex(q->mutex);
}

static int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block, int *serial)
{
    MyAVPacketList *pkt1;
    int ret;
    SDL_LockMutex(q->mutex);
    for (;;) {
        if (q->abort_request) {
            ret = -1;
            break;
        }
        pkt1 = q->first_pkt;
        if (pkt1) {
            q->first_pkt = pkt1->next;
            if (!q->first_pkt)
                q->last_pkt = 
                             ((void *)0)
                                 ;
            q->nb_packets--;
            q->size -= pkt1->pkt.size + sizeof(*pkt1);
            q->duration -= pkt1->pkt.duration;
            *pkt = pkt1->pkt;
            if (serial)
                *serial = pkt1->serial;
            av_free(pkt1);
            ret = 1;
            break;
        } else if (!block) {
            ret = 0;
            break;
        } else {
            SDL_CondWait(q->cond, q->mutex);
        }
    }
    SDL_UnlockMutex(q->mutex);
    return ret;
}

static void decoder_init(Decoder *d, AVCodecContext *avctx, PacketQueue *queue, SDL_cond *empty_queue_cond) {
    memset(d, 0, sizeof(Decoder));
    d->avctx = avctx;
    d->queue = queue;
    d->empty_queue_cond = empty_queue_cond;
    d->start_pts = ((int64_t)0x8000000000000000ULL);
    d->pkt_serial = -1;
}

static int decoder_decode_frame(Decoder *d, AVFrame *frame, AVSubtitle *sub) {
    int ret = (-(
             11
             ));
    for (;;) {
        AVPacket pkt;
        if (d->queue->serial == d->pkt_serial) {
            do {
                if (d->queue->abort_request)
                    return -1;
                switch (d->avctx->codec_type) {
                    case AVMEDIA_TYPE_VIDEO:
                        ret = avcodec_receive_frame(d->avctx, frame);
                        if (ret >= 0) {
                            if (decoder_reorder_pts == -1) {
                                frame->pts = frame->best_effort_timestamp;
                            } else if (!decoder_reorder_pts) {
                                frame->pts = frame->pkt_dts;
                            }
                        }
                        break;
                    case AVMEDIA_TYPE_AUDIO:
                        ret = avcodec_receive_frame(d->avctx, frame);
                        if (ret >= 0) {
                            AVRational tb = (AVRational){1, frame->sample_rate};
                            if (frame->pts != ((int64_t)0x8000000000000000ULL))
                                frame->pts = av_rescale_q(frame->pts, d->avctx->pkt_timebase, tb);
                            else if (d->next_pts != ((int64_t)0x8000000000000000ULL))
                                frame->pts = av_rescale_q(d->next_pts, d->next_pts_tb, tb);
                            if (frame->pts != ((int64_t)0x8000000000000000ULL)) {
                                d->next_pts = frame->pts + frame->nb_samples;
                                d->next_pts_tb = tb;
                            }
                        }
                        break;
                }
                if (ret == (-(int)(('E') | (('O') << 8) | (('F') << 16) | ((unsigned)(' ') << 24)))) {
                    d->finished = d->pkt_serial;
                    avcodec_flush_buffers(d->avctx);
                    return 0;
                }
                if (ret >= 0)
                    return 1;
            } while (ret != (-(
                           11
                           )));
        }
        do {
            if (d->queue->nb_packets == 0)
                SDL_CondSignal(d->empty_queue_cond);
            if (d->packet_pending) {
                av_packet_move_ref(&pkt, &d->pkt);
                d->packet_pending = 0;
            } else {
                if (packet_queue_get(d->queue, &pkt, 1, &d->pkt_serial) < 0)
                    return -1;
            }
            if (d->queue->serial == d->pkt_serial)
                break;
            av_packet_unref(&pkt);
        } while (1);
        if (pkt.data == flush_pkt.data) {
            avcodec_flush_buffers(d->avctx);
            d->finished = 0;
            d->next_pts = d->start_pts;
            d->next_pts_tb = d->start_pts_tb;
        } else {
            if (d->avctx->codec_type == AVMEDIA_TYPE_SUBTITLE) {
                int got_frame = 0;
                ret = avcodec_decode_subtitle2(d->avctx, sub, &got_frame, &pkt);
                if (ret < 0) {
                    ret = (-(
                         11
                         ));
                } else {
                    if (got_frame && !pkt.data) {
                       d->packet_pending = 1;
                       av_packet_move_ref(&d->pkt, &pkt);
                    }
                    ret = got_frame ? 0 : (pkt.data ? (-(
                                                     11
                                                     )) : (-(int)(('E') | (('O') << 8) | (('F') << 16) | ((unsigned)(' ') << 24))));
                }
            } else {
                if (avcodec_send_packet(d->avctx, &pkt) == (-(
                                                          11
                                                          ))) {
                    av_log(d->avctx, 16, "Receive_frame and send_packet both returned EAGAIN, which is an API violation.\n");
                    d->packet_pending = 1;
                    av_packet_move_ref(&d->pkt, &pkt);
                }
            }
            av_packet_unref(&pkt);
        }
    }
}

static void decoder_destroy(Decoder *d) {
    av_packet_unref(&d->pkt);
    avcodec_free_context(&d->avctx);
}

static void frame_queue_unref_item(Frame *vp)
{
    av_frame_unref(vp->frame);
    avsubtitle_free(&vp->sub);
}

static int frame_queue_init(FrameQueue *f, PacketQueue *pktq, int max_size, int keep_last)
{
    int i;
    memset(f, 0, sizeof(FrameQueue));
    if (!(f->mutex = SDL_CreateMutex())) {
        av_log(
              ((void *)0)
                  , 8, "SDL_CreateMutex(): %s\n", SDL_GetError());
        return (-(
              12
              ));
    }
    if (!(f->cond = SDL_CreateCond())) {
        av_log(
              ((void *)0)
                  , 8, "SDL_CreateCond(): %s\n", SDL_GetError());
        return (-(
              12
              ));
    }
    f->pktq = pktq;
    f->max_size = ((max_size) > (((9) > (((3) > (16) ? (3) : (16))) ? (9) : (((3) > (16) ? (3) : (16))))) ? (((9) > (((3) > (16) ? (3) : (16))) ? (9) : (((3) > (16) ? (3) : (16))))) : (max_size));
    f->keep_last = !!keep_last;
    for (i = 0; i < f->max_size; i++)
        if (!(f->queue[i].frame = av_frame_alloc()))
            return (-(
                  12
                  ));
    return 0;
}

static void frame_queue_destory(FrameQueue *f)
{
    int i;
    for (i = 0; i < f->max_size; i++) {
        Frame *vp = &f->queue[i];
        frame_queue_unref_item(vp);
        av_frame_free(&vp->frame);
    }
    SDL_DestroyMutex(f->mutex);
    SDL_DestroyCond(f->cond);
}

static void frame_queue_signal(FrameQueue *f)
{
    SDL_LockMutex(f->mutex);
    SDL_CondSignal(f->cond);
    SDL_UnlockMutex(f->mutex);
}

static Frame *frame_queue_peek(FrameQueue *f)
{
    return &f->queue[(f->rindex + f->rindex_shown) % f->max_size];
}

static Frame *frame_queue_peek_next(FrameQueue *f)
{
    return &f->queue[(f->rindex + f->rindex_shown + 1) % f->max_size];
}

static Frame *frame_queue_peek_last(FrameQueue *f)
{
    return &f->queue[f->rindex];
}

static Frame *frame_queue_peek_writable(FrameQueue *f)
{
    SDL_LockMutex(f->mutex);
    while (f->size >= f->max_size &&
           !f->pktq->abort_request) {
        SDL_CondWait(f->cond, f->mutex);
    }
    SDL_UnlockMutex(f->mutex);
    if (f->pktq->abort_request)
        return 
              ((void *)0)
                  ;
    return &f->queue[f->windex];
}

static Frame *frame_queue_peek_readable(FrameQueue *f)
{
    SDL_LockMutex(f->mutex);
    while (f->size - f->rindex_shown <= 0 &&
           !f->pktq->abort_request) {
        SDL_CondWait(f->cond, f->mutex);
    }
    SDL_UnlockMutex(f->mutex);
    if (f->pktq->abort_request)
        return 
              ((void *)0)
                  ;
    return &f->queue[(f->rindex + f->rindex_shown) % f->max_size];
}

static void frame_queue_push(FrameQueue *f)
{
    if (++f->windex == f->max_size)
        f->windex = 0;
    SDL_LockMutex(f->mutex);
    f->size++;
    SDL_CondSignal(f->cond);
    SDL_UnlockMutex(f->mutex);
}

static void frame_queue_next(FrameQueue *f)
{
    if (f->keep_last && !f->rindex_shown) {
        f->rindex_shown = 1;
        return;
    }
    frame_queue_unref_item(&f->queue[f->rindex]);
    if (++f->rindex == f->max_size)
        f->rindex = 0;
    SDL_LockMutex(f->mutex);
    f->size--;
    SDL_CondSignal(f->cond);
    SDL_UnlockMutex(f->mutex);
}

static int frame_queue_nb_remaining(FrameQueue *f)
{
    return f->size - f->rindex_shown;
}

static int64_t frame_queue_last_pos(FrameQueue *f)
{
    Frame *fp = &f->queue[f->rindex];
    if (f->rindex_shown && fp->serial == f->pktq->serial)
        return fp->pos;
    else
        return -1;
}

static void decoder_abort(Decoder *d, FrameQueue *fq)
{
    packet_queue_abort(d->queue);
    frame_queue_signal(fq);
    SDL_WaitThread(d->decoder_tid, 
                                  ((void *)0)
                                      );
    d->decoder_tid = 
                    ((void *)0)
                        ;
    packet_queue_flush(d->queue);
}

static inline void fill_rectangle(int x, int y, int w, int h)
{
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
    if (w && h)
        SDL_RenderFillRect(renderer, &rect);
}

static int realloc_texture(SDL_Texture **texture, Uint32 new_format, int new_width, int new_height, SDL_BlendMode blendmode, int init_texture)
{
    Uint32 format;
    int access, w, h;
    if (!*texture || SDL_QueryTexture(*texture, &format, &access, &w, &h) < 0 || new_width != w || new_height != h || new_format != format) {
        void *pixels;
        int pitch;
        if (*texture)
            SDL_DestroyTexture(*texture);
        if (!(*texture = SDL_CreateTexture(renderer, new_format, SDL_TEXTUREACCESS_STREAMING, new_width, new_height)))
            return -1;
        if (SDL_SetTextureBlendMode(*texture, blendmode) < 0)
            return -1;
        if (init_texture) {
            if (SDL_LockTexture(*texture, 
                                         ((void *)0)
                                             , &pixels, &pitch) < 0)
                return -1;
            memset(pixels, 0, pitch * new_height);
            SDL_UnlockTexture(*texture);
        }
        av_log(
              ((void *)0)
                  , 40, "Created %dx%d texture with %s.\n", new_width, new_height, SDL_GetPixelFormatName(new_format));
    }
    return 0;
}

static void calculate_display_rect(SDL_Rect *rect,
                                   int scr_xleft, int scr_ytop, int scr_width, int scr_height,
                                   int pic_width, int pic_height, AVRational pic_sar)
{
    AVRational aspect_ratio = pic_sar;
    int64_t width, height, x, y;
    if (av_cmp_q(aspect_ratio, av_make_q(0, 1)) <= 0)
        aspect_ratio = av_make_q(1, 1);
    aspect_ratio = av_mul_q(aspect_ratio, av_make_q(pic_width, pic_height));
    height = scr_height;
    width = av_rescale(height, aspect_ratio.num, aspect_ratio.den) & ~1;
    if (width > scr_width) {
        width = scr_width;
        height = av_rescale(width, aspect_ratio.den, aspect_ratio.num) & ~1;
    }
    x = (scr_width - width) / 2;
    y = (scr_height - height) / 2;
    rect->x = scr_xleft + x;
    rect->y = scr_ytop + y;
    rect->w = (((int)width) > (1) ? ((int)width) : (1));
    rect->h = (((int)height) > (1) ? ((int)height) : (1));
}

static void get_sdl_pix_fmt_and_blendmode(int format, Uint32 *sdl_pix_fmt, SDL_BlendMode *sdl_blendmode)
{
    int i;
    *sdl_blendmode = SDL_BLENDMODE_NONE;
    *sdl_pix_fmt = SDL_PIXELFORMAT_UNKNOWN;
    if (format == AV_PIX_FMT_BGRA ||
        format == AV_PIX_FMT_ABGR ||
        format == AV_PIX_FMT_RGBA ||
        format == AV_PIX_FMT_ARGB)
        *sdl_blendmode = SDL_BLENDMODE_BLEND;
    for (i = 0; i < (sizeof(sdl_texture_format_map) / sizeof((sdl_texture_format_map)[0])) - 1; i++) {
        if (format == sdl_texture_format_map[i].format) {
            *sdl_pix_fmt = sdl_texture_format_map[i].texture_fmt;
            return;
        }
    }
}

static int upload_texture(SDL_Texture **tex, AVFrame *frame, struct SwsContext **img_convert_ctx) {
    int ret = 0;
    Uint32 sdl_pix_fmt;
    SDL_BlendMode sdl_blendmode;
    get_sdl_pix_fmt_and_blendmode(frame->format, &sdl_pix_fmt, &sdl_blendmode);
    if (realloc_texture(tex, sdl_pix_fmt == SDL_PIXELFORMAT_UNKNOWN ? SDL_PIXELFORMAT_ARGB8888 : sdl_pix_fmt, frame->width, frame->height, sdl_blendmode, 0) < 0)
        return -1;
    switch (sdl_pix_fmt) {
        case SDL_PIXELFORMAT_UNKNOWN:
            *img_convert_ctx = sws_getCachedContext(*img_convert_ctx,
                frame->width, frame->height, frame->format, frame->width, frame->height,
                AV_PIX_FMT_BGRA, sws_flags, 
                                           ((void *)0)
                                               , 
                                                 ((void *)0)
                                                     , 
                                                       ((void *)0)
                                                           );
            if (*img_convert_ctx != 
                                   ((void *)0)
                                       ) {
                uint8_t *pixels[4];
                int pitch[4];
                if (!SDL_LockTexture(*tex, 
                                          ((void *)0)
                                              , (void **)pixels, pitch)) {
                    sws_scale(*img_convert_ctx, (const uint8_t * const *)frame->data, frame->linesize,
                              0, frame->height, pixels, pitch);
                    SDL_UnlockTexture(*tex);
                }
            } else {
                av_log(
                      ((void *)0)
                          , 8, "Cannot initialize the conversion context\n");
                ret = -1;
            }
            break;
        case SDL_PIXELFORMAT_IYUV:
            if (frame->linesize[0] > 0 && frame->linesize[1] > 0 && frame->linesize[2] > 0) {
                ret = SDL_UpdateYUVTexture(*tex, 
                                                ((void *)0)
                                                    , frame->data[0], frame->linesize[0],
                                                       frame->data[1], frame->linesize[1],
                                                       frame->data[2], frame->linesize[2]);
            } else if (frame->linesize[0] < 0 && frame->linesize[1] < 0 && frame->linesize[2] < 0) {
                ret = SDL_UpdateYUVTexture(*tex, 
                                                ((void *)0)
                                                    , frame->data[0] + frame->linesize[0] * (frame->height - 1), -frame->linesize[0],
                                                       frame->data[1] + frame->linesize[1] * ((!__builtin_constant_p(1) ? -((-(frame->height)) >> (1)) : ((frame->height) + (1<<(1)) - 1) >> (1)) - 1), -frame->linesize[1],
                                                       frame->data[2] + frame->linesize[2] * ((!__builtin_constant_p(1) ? -((-(frame->height)) >> (1)) : ((frame->height) + (1<<(1)) - 1) >> (1)) - 1), -frame->linesize[2]);
            } else {
                av_log(
                      ((void *)0)
                          , 16, "Mixed negative and positive linesizes are not supported.\n");
                return -1;
            }
            break;
        default:
            if (frame->linesize[0] < 0) {
                ret = SDL_UpdateTexture(*tex, 
                                             ((void *)0)
                                                 , frame->data[0] + frame->linesize[0] * (frame->height - 1), -frame->linesize[0]);
            } else {
                ret = SDL_UpdateTexture(*tex, 
                                             ((void *)0)
                                                 , frame->data[0], frame->linesize[0]);
            }
            break;
    }
    return ret;
}

static void set_sdl_yuv_conversion_mode(AVFrame *frame)
{
    SDL_YUV_CONVERSION_MODE mode = SDL_YUV_CONVERSION_AUTOMATIC;
    if (frame && (frame->format == AV_PIX_FMT_YUV420P || frame->format == AV_PIX_FMT_YUYV422 || frame->format == AV_PIX_FMT_UYVY422)) {
        if (frame->color_range == AVCOL_RANGE_JPEG)
            mode = SDL_YUV_CONVERSION_JPEG;
        else if (frame->colorspace == AVCOL_SPC_BT709)
            mode = SDL_YUV_CONVERSION_BT709;
        else if (frame->colorspace == AVCOL_SPC_BT470BG || frame->colorspace == AVCOL_SPC_SMPTE170M || frame->colorspace == AVCOL_SPC_SMPTE240M)
            mode = SDL_YUV_CONVERSION_BT601;
    }
    SDL_SetYUVConversionMode(mode);
}

static void video_image_display(VideoState *is)
{
    Frame *vp;
    Frame *sp = 
               ((void *)0)
                   ;
    SDL_Rect rect;
    vp = frame_queue_peek_last(&is->pictq);
    if (is->subtitle_st) {
        if (frame_queue_nb_remaining(&is->subpq) > 0) {
            sp = frame_queue_peek(&is->subpq);
            if (vp->pts >= sp->pts + ((float) sp->sub.start_display_time / 1000)) {
                if (!sp->uploaded) {
                    uint8_t* pixels[4];
                    int pitch[4];
                    int i;
                    if (!sp->width || !sp->height) {
                        sp->width = vp->width;
                        sp->height = vp->height;
                    }
                    if (realloc_texture(&is->sub_texture, SDL_PIXELFORMAT_ARGB8888, sp->width, sp->height, SDL_BLENDMODE_BLEND, 1) < 0)
                        return;
                    for (i = 0; i < sp->sub.num_rects; i++) {
                        AVSubtitleRect *sub_rect = sp->sub.rects[i];
                        sub_rect->x = av_clip_c(sub_rect->x, 0, sp->width );
                        sub_rect->y = av_clip_c(sub_rect->y, 0, sp->height);
                        sub_rect->w = av_clip_c(sub_rect->w, 0, sp->width - sub_rect->x);
                        sub_rect->h = av_clip_c(sub_rect->h, 0, sp->height - sub_rect->y);
                        is->sub_convert_ctx = sws_getCachedContext(is->sub_convert_ctx,
                            sub_rect->w, sub_rect->h, AV_PIX_FMT_PAL8,
                            sub_rect->w, sub_rect->h, AV_PIX_FMT_BGRA,
                            0, 
                              ((void *)0)
                                  , 
                                    ((void *)0)
                                        , 
                                          ((void *)0)
                                              );
                        if (!is->sub_convert_ctx) {
                            av_log(
                                  ((void *)0)
                                      , 8, "Cannot initialize the conversion context\n");
                            return;
                        }
                        if (!SDL_LockTexture(is->sub_texture, (SDL_Rect *)sub_rect, (void **)pixels, pitch)) {
                            sws_scale(is->sub_convert_ctx, (const uint8_t * const *)sub_rect->data, sub_rect->linesize,
                                      0, sub_rect->h, pixels, pitch);
                            SDL_UnlockTexture(is->sub_texture);
                        }
                    }
                    sp->uploaded = 1;
                }
            } else
                sp = 
                    ((void *)0)
                        ;
        }
    }
    calculate_display_rect(&rect, is->xleft, is->ytop, is->width, is->height, vp->width, vp->height, vp->sar);
    if (!vp->uploaded) {
        if (upload_texture(&is->vid_texture, vp->frame, &is->img_convert_ctx) < 0)
            return;
        vp->uploaded = 1;
        vp->flip_v = vp->frame->linesize[0] < 0;
    }
    set_sdl_yuv_conversion_mode(vp->frame);
    SDL_RenderCopyEx(renderer, is->vid_texture, 
                                               ((void *)0)
                                                   , &rect, 0, 
                                                               ((void *)0)
                                                                   , vp->flip_v ? SDL_FLIP_VERTICAL : 0);
    set_sdl_yuv_conversion_mode(
                               ((void *)0)
                                   );
    if (sp) {
        SDL_RenderCopy(renderer, is->sub_texture, 
                                                 ((void *)0)
                                                     , &rect);
    }
}

static inline int compute_mod(int a, int b)
{
    return a < 0 ? a%b + b : a%b;
}

static void video_audio_display(VideoState *s)
{
    int i, i_start, x, y1, y, ys, delay, n, nb_display_channels;
    int ch, channels, h, h2;
    int64_t time_diff;
    int rdft_bits, nb_freq;
    for (rdft_bits = 1; (1 << rdft_bits) < 2 * s->height; rdft_bits++)
        ;
    nb_freq = 1 << (rdft_bits - 1);
    channels = s->audio_tgt.channels;
    nb_display_channels = channels;
    if (!s->paused) {
        int data_used= s->show_mode == SHOW_MODE_WAVES ? s->width : (2*nb_freq);
        n = 2 * channels;
        delay = s->audio_write_buf_size;
        delay /= n;
        if (audio_callback_time) {
            time_diff = av_gettime_relative() - audio_callback_time;
            delay -= (time_diff * s->audio_tgt.freq) / 1000000;
        }
        delay += 2 * data_used;
        if (delay < data_used)
            delay = data_used;
        i_start= x = compute_mod(s->sample_array_index - delay * channels, (8 * 65536));
        if (s->show_mode == SHOW_MODE_WAVES) {
            h = 
               (-0x7fffffff - 1)
                      ;
            for (i = 0; i < 1000; i += channels) {
                int idx = ((8 * 65536) + x - i) % (8 * 65536);
                int a = s->sample_array[idx];
                int b = s->sample_array[(idx + 4 * channels) % (8 * 65536)];
                int c = s->sample_array[(idx + 5 * channels) % (8 * 65536)];
                int d = s->sample_array[(idx + 9 * channels) % (8 * 65536)];
                int score = a - d;
                if (h < score && (b ^ c) < 0) {
                    h = score;
                    i_start = idx;
                }
            }
        }
        s->last_i_start = i_start;
    } else {
        i_start = s->last_i_start;
    }
    if (s->show_mode == SHOW_MODE_WAVES) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        h = s->height / nb_display_channels;
        h2 = (h * 9) / 20;
        for (ch = 0; ch < nb_display_channels; ch++) {
            i = i_start + ch;
            y1 = s->ytop + ch * h + (h / 2);
            for (x = 0; x < s->width; x++) {
                y = (s->sample_array[i] * h2) >> 15;
                if (y < 0) {
                    y = -y;
                    ys = y1 - y;
                } else {
                    ys = y1;
                }
                fill_rectangle(s->xleft + x, ys, 1, y);
                i += channels;
                if (i >= (8 * 65536))
                    i -= (8 * 65536);
            }
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        for (ch = 1; ch < nb_display_channels; ch++) {
            y = s->ytop + ch * h;
            fill_rectangle(s->xleft, y, s->width, 1);
        }
    } else {
        if (realloc_texture(&s->vis_texture, SDL_PIXELFORMAT_ARGB8888, s->width, s->height, SDL_BLENDMODE_NONE, 1) < 0)
            return;
        nb_display_channels= ((nb_display_channels) > (2) ? (2) : (nb_display_channels));
        if (rdft_bits != s->rdft_bits) {
            av_rdft_end(s->rdft);
            av_free(s->rdft_data);
            s->rdft = av_rdft_init(rdft_bits, DFT_R2C);
            s->rdft_bits = rdft_bits;
            s->rdft_data = av_malloc_array(nb_freq, 4 *sizeof(*s->rdft_data));
        }
        if (!s->rdft || !s->rdft_data){
            av_log(
                  ((void *)0)
                      , 16, "Failed to allocate buffers for RDFT, switching to waves display\n");
            s->show_mode = SHOW_MODE_WAVES;
        } else {
            FFTSample *data[2];
            SDL_Rect rect = {.x = s->xpos, .y = 0, .w = 1, .h = s->height};
            uint32_t *pixels;
            int pitch;
            for (ch = 0; ch < nb_display_channels; ch++) {
                data[ch] = s->rdft_data + 2 * nb_freq * ch;
                i = i_start + ch;
                for (x = 0; x < 2 * nb_freq; x++) {
                    double w = (x-nb_freq) * (1.0 / nb_freq);
                    data[ch][x] = s->sample_array[i] * (1.0 - w * w);
                    i += channels;
                    if (i >= (8 * 65536))
                        i -= (8 * 65536);
                }
                av_rdft_calc(s->rdft, data[ch]);
            }
            if (!SDL_LockTexture(s->vis_texture, &rect, (void **)&pixels, &pitch)) {
                pitch >>= 2;
                pixels += pitch * s->height;
                for (y = 0; y < s->height; y++) {
                    double w = 1 / sqrt(nb_freq);
                    int a = sqrt(w * sqrt(data[0][2 * y + 0] * data[0][2 * y + 0] + data[0][2 * y + 1] * data[0][2 * y + 1]));
                    int b = (nb_display_channels == 2 ) ? sqrt(w * hypot(data[1][2 * y + 0], data[1][2 * y + 1]))
                                                        : a;
                    a = ((a) > (255) ? (255) : (a));
                    b = ((b) > (255) ? (255) : (b));
                    pixels -= pitch;
                    *pixels = (a << 16) + (b << 8) + ((a+b) >> 1);
                }
                SDL_UnlockTexture(s->vis_texture);
            }
            SDL_RenderCopy(renderer, s->vis_texture, 
                                                    ((void *)0)
                                                        , 
                                                          ((void *)0)
                                                              );
        }
        if (!s->paused)
            s->xpos++;
        if (s->xpos >= s->width)
            s->xpos= s->xleft;
    }
}

static void stream_component_close(VideoState *is, int stream_index)
{
    AVFormatContext *ic = is->ic;
    AVCodecParameters *codecpar;
    if (stream_index < 0 || stream_index >= ic->nb_streams)
        return;
    codecpar = ic->streams[stream_index]->codecpar;
    switch (codecpar->codec_type) {
    case AVMEDIA_TYPE_AUDIO:
        decoder_abort(&is->auddec, &is->sampq);
        SDL_CloseAudioDevice(audio_dev);
        decoder_destroy(&is->auddec);
        swr_free(&is->swr_ctx);
        av_freep(&is->audio_buf1);
        is->audio_buf1_size = 0;
        is->audio_buf = 
                       ((void *)0)
                           ;
        if (is->rdft) {
            av_rdft_end(is->rdft);
            av_freep(&is->rdft_data);
            is->rdft = 
                      ((void *)0)
                          ;
            is->rdft_bits = 0;
        }
        break;
    case AVMEDIA_TYPE_VIDEO:
        decoder_abort(&is->viddec, &is->pictq);
        decoder_destroy(&is->viddec);
        break;
    case AVMEDIA_TYPE_SUBTITLE:
        decoder_abort(&is->subdec, &is->subpq);
        decoder_destroy(&is->subdec);
        break;
    default:
        break;
    }
    ic->streams[stream_index]->discard = AVDISCARD_ALL;
    switch (codecpar->codec_type) {
    case AVMEDIA_TYPE_AUDIO:
        is->audio_st = 
                      ((void *)0)
                          ;
        is->audio_stream = -1;
        break;
    case AVMEDIA_TYPE_VIDEO:
        is->video_st = 
                      ((void *)0)
                          ;
        is->video_stream = -1;
        break;
    case AVMEDIA_TYPE_SUBTITLE:
        is->subtitle_st = 
                         ((void *)0)
                             ;
        is->subtitle_stream = -1;
        break;
    default:
        break;
    }
}

static void stream_close(VideoState *is)
{
    is->abort_request = 1;
    SDL_WaitThread(is->read_tid, 
                                ((void *)0)
                                    );
    if (is->audio_stream >= 0)
        stream_component_close(is, is->audio_stream);
    if (is->video_stream >= 0)
        stream_component_close(is, is->video_stream);
    if (is->subtitle_stream >= 0)
        stream_component_close(is, is->subtitle_stream);
    avformat_close_input(&is->ic);
    packet_queue_destroy(&is->videoq);
    packet_queue_destroy(&is->audioq);
    packet_queue_destroy(&is->subtitleq);
    frame_queue_destory(&is->pictq);
    frame_queue_destory(&is->sampq);
    frame_queue_destory(&is->subpq);
    SDL_DestroyCond(is->continue_read_thread);
    sws_freeContext(is->img_convert_ctx);
    sws_freeContext(is->sub_convert_ctx);
    av_free(is->filename);
    if (is->vis_texture)
        SDL_DestroyTexture(is->vis_texture);
    if (is->vid_texture)
        SDL_DestroyTexture(is->vid_texture);
    if (is->sub_texture)
        SDL_DestroyTexture(is->sub_texture);
    av_free(is);
}

static void do_exit(VideoState *is)
{
    if (is) {
        stream_close(is);
    }
    if (renderer)
        SDL_DestroyRenderer(renderer);
    if (window)
        SDL_DestroyWindow(window);
    uninit_opts();
    av_freep(&vfilters_list);
    avformat_network_deinit();
    if (show_status)
        printf("\n");
    SDL_Quit();
    av_log(
          ((void *)0)
              , -8, "%s", "");
    exit(0);
}

static void sigterm_handler(int sig)
{
    exit(123);
}

static void set_default_window_size(int width, int height, AVRational sar)
{
    SDL_Rect rect;
    int max_width = screen_width ? screen_width : 0x7fffffff;
    int max_height = screen_height ? screen_height : 0x7fffffff;
    if (max_width == 0x7fffffff && max_height == 0x7fffffff)
        max_height = height;
    calculate_display_rect(&rect, 0, 0, max_width, max_height, width, height, sar);
    default_width = rect.w;
    default_height = rect.h;
}

static int video_open(VideoState *is)
{
    int w,h;
    w = screen_width ? screen_width : default_width;
    h = screen_height ? screen_height : default_height;
    if (!window_title)
        window_title = input_filename;
    SDL_SetWindowTitle(window, window_title);
    SDL_SetWindowSize(window, w, h);
    SDL_SetWindowPosition(window, screen_left, screen_top);
    if (is_full_screen)
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    SDL_ShowWindow(window);
    is->width = w;
    is->height = h;
    return 0;
}

static void video_display(VideoState *is)
{
    if (!is->width)
        video_open(is);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    if (is->audio_st && is->show_mode != SHOW_MODE_VIDEO)
        video_audio_display(is);
    else if (is->video_st)
        video_image_display(is);
    SDL_RenderPresent(renderer);
}

static double get_clock(Clock *c)
{
    if (*c->queue_serial != c->serial)
        return 
              __builtin_nanf("")
                 ;
    if (c->paused) {
        return c->pts;
    } else {
        double time = av_gettime_relative() / 1000000.0;
        return c->pts_drift + time - (time - c->last_updated) * (1.0 - c->speed);
    }
}

static void set_clock_at(Clock *c, double pts, int serial, double time)
{
    c->pts = pts;
    c->last_updated = time;
    c->pts_drift = c->pts - time;
    c->serial = serial;
}

static void set_clock(Clock *c, double pts, int serial)
{
    double time = av_gettime_relative() / 1000000.0;
    set_clock_at(c, pts, serial, time);
}

static void set_clock_speed(Clock *c, double speed)
{
    set_clock(c, get_clock(c), c->serial);
    c->speed = speed;
}

static void init_clock(Clock *c, int *queue_serial)
{
    c->speed = 1.0;
    c->paused = 0;
    c->queue_serial = queue_serial;
    set_clock(c, 
                __builtin_nanf("")
                   , -1);
}

static void sync_clock_to_slave(Clock *c, Clock *slave)
{
    double clock = get_clock(c);
    double slave_clock = get_clock(slave);
    if (!
        __builtin_choose_expr ( __builtin_types_compatible_p (__typeof__ (
        slave_clock
        ), double), __isnan(
        slave_clock
        ), __builtin_choose_expr ( __builtin_types_compatible_p (__typeof__ (
        slave_clock
        ), float), __isnanf(
        slave_clock
        ), __builtin_choose_expr ( __builtin_types_compatible_p (__typeof__ (
        slave_clock
        ), long double), __isnanl(
        slave_clock
        ), (__builtin_trap(),
        slave_clock
        )))) 
                           && (
                               __builtin_choose_expr ( __builtin_types_compatible_p (__typeof__ (
                               clock
                               ), double), __isnan(
                               clock
                               ), __builtin_choose_expr ( __builtin_types_compatible_p (__typeof__ (
                               clock
                               ), float), __isnanf(
                               clock
                               ), __builtin_choose_expr ( __builtin_types_compatible_p (__typeof__ (
                               clock
                               ), long double), __isnanl(
                               clock
                               ), (__builtin_trap(),
                               clock
                               )))) 
                                            || fabs(clock - slave_clock) > 10.0))
        set_clock(c, slave_clock, slave->serial);
}

static int get_master_sync_type(VideoState *is) {
    if (is->av_sync_type == AV_SYNC_VIDEO_MASTER) {
        if (is->video_st)
            return AV_SYNC_VIDEO_MASTER;
        else
            return AV_SYNC_AUDIO_MASTER;
    } else if (is->av_sync_type == AV_SYNC_AUDIO_MASTER) {
        if (is->audio_st)
            return AV_SYNC_AUDIO_MASTER;
        else
            return AV_SYNC_EXTERNAL_CLOCK;
    } else {
        return AV_SYNC_EXTERNAL_CLOCK;
    }
}

static double get_master_clock(VideoState *is)
{
    double val;
    switch (get_master_sync_type(is)) {
        case AV_SYNC_VIDEO_MASTER:
            val = get_clock(&is->vidclk);
            break;
        case AV_SYNC_AUDIO_MASTER:
            val = get_clock(&is->audclk);
            break;
        default:
            val = get_clock(&is->extclk);
            break;
    }
    return val;
}

static void check_external_clock_speed(VideoState *is) {
   if (is->video_stream >= 0 && is->videoq.nb_packets <= 2 ||
       is->audio_stream >= 0 && is->audioq.nb_packets <= 2) {
       set_clock_speed(&is->extclk, ((0.900) > (is->extclk.speed - 0.001) ? (0.900) : (is->extclk.speed - 0.001)));
   } else if ((is->video_stream < 0 || is->videoq.nb_packets > 10) &&
              (is->audio_stream < 0 || is->audioq.nb_packets > 10)) {
       set_clock_speed(&is->extclk, ((1.010) > (is->extclk.speed + 0.001) ? (is->extclk.speed + 0.001) : (1.010)));
   } else {
       double speed = is->extclk.speed;
       if (speed != 1.0)
           set_clock_speed(&is->extclk, speed + 0.001 * (1.0 - speed) / fabs(1.0 - speed));
   }
}

static void stream_seek(VideoState *is, int64_t pos, int64_t rel, int seek_by_bytes)
{
    if (!is->seek_req) {
        is->seek_pos = pos;
        is->seek_rel = rel;
        is->seek_flags &= ~2;
        if (seek_by_bytes)
            is->seek_flags |= 2;
        is->seek_req = 1;
        SDL_CondSignal(is->continue_read_thread);
    }
}

static void stream_toggle_pause(VideoState *is)
{
    if (is->paused) {
        is->frame_timer += av_gettime_relative() / 1000000.0 - is->vidclk.last_updated;
        if (is->read_pause_return != (-(
                                    40
                                    ))) {
            is->vidclk.paused = 0;
        }
        set_clock(&is->vidclk, get_clock(&is->vidclk), is->vidclk.serial);
    }
    set_clock(&is->extclk, get_clock(&is->extclk), is->extclk.serial);
    is->paused = is->audclk.paused = is->vidclk.paused = is->extclk.paused = !is->paused;
}

static void toggle_pause(VideoState *is)
{
    stream_toggle_pause(is);
    is->step = 0;
}

static void toggle_mute(VideoState *is)
{
    is->muted = !is->muted;
}

static void update_volume(VideoState *is, int sign, double step)
{
    double volume_level = is->audio_volume ? (20 * log(is->audio_volume / (double)128) / log(10)) : -1000.0;
    int new_volume = lrint(128 * pow(10.0, (volume_level + sign * step) / 20.0));
    is->audio_volume = av_clip_c(is->audio_volume == new_volume ? (is->audio_volume + sign) : new_volume, 0, 128);
}

static void step_to_next_frame(VideoState *is)
{
    if (is->paused)
        stream_toggle_pause(is);
    is->step = 1;
}

static double compute_target_delay(double delay, VideoState *is)
{
    double sync_threshold, diff = 0;
    if (get_master_sync_type(is) != AV_SYNC_VIDEO_MASTER) {
        diff = get_clock(&is->vidclk) - get_master_clock(is);
        sync_threshold = ((0.04) > (((0.1) > (delay) ? (delay) : (0.1))) ? (0.04) : (((0.1) > (delay) ? (delay) : (0.1))));
        if (!
            __builtin_choose_expr ( __builtin_types_compatible_p (__typeof__ (
            diff
            ), double), __isnan(
            diff
            ), __builtin_choose_expr ( __builtin_types_compatible_p (__typeof__ (
            diff
            ), float), __isnanf(
            diff
            ), __builtin_choose_expr ( __builtin_types_compatible_p (__typeof__ (
            diff
            ), long double), __isnanl(
            diff
            ), (__builtin_trap(),
            diff
            )))) 
                        && fabs(diff) < is->max_frame_duration) {
            if (diff <= -sync_threshold)
                delay = ((0) > (delay + diff) ? (0) : (delay + diff));
            else if (diff >= sync_threshold && delay > 0.1)
                delay = delay + diff;
            else if (diff >= sync_threshold)
                delay = 2 * delay;
        }
    }
    av_log(
          ((void *)0)
              , 56, "video: delay=%0.3f A-V=%f\n",
            delay, -diff);
    return delay;
}

static double vp_duration(VideoState *is, Frame *vp, Frame *nextvp) {
    if (vp->serial == nextvp->serial) {
        double duration = nextvp->pts - vp->pts;
        if (
           __builtin_choose_expr ( __builtin_types_compatible_p (__typeof__ (
           duration
           ), double), __isnan(
           duration
           ), __builtin_choose_expr ( __builtin_types_compatible_p (__typeof__ (
           duration
           ), float), __isnanf(
           duration
           ), __builtin_choose_expr ( __builtin_types_compatible_p (__typeof__ (
           duration
           ), long double), __isnanl(
           duration
           ), (__builtin_trap(),
           duration
           )))) 
                           || duration <= 0 || duration > is->max_frame_duration)
            return vp->duration;
        else
            return duration;
    } else {
        return 0.0;
    }
}

static void update_video_pts(VideoState *is, double pts, int64_t pos, int serial) {
    set_clock(&is->vidclk, pts, serial);
    sync_clock_to_slave(&is->extclk, &is->vidclk);
}

static void video_refresh(void *opaque, double *remaining_time)
{
    VideoState *is = opaque;
    double time;
    Frame *sp, *sp2;
    if (!is->paused && get_master_sync_type(is) == AV_SYNC_EXTERNAL_CLOCK && is->realtime)
        check_external_clock_speed(is);
    if (!display_disable && is->show_mode != SHOW_MODE_VIDEO && is->audio_st) {
        time = av_gettime_relative() / 1000000.0;
        if (is->force_refresh || is->last_vis_time + rdftspeed < time) {
            video_display(is);
            is->last_vis_time = time;
        }
        *remaining_time = ((*remaining_time) > (is->last_vis_time + rdftspeed - time) ? (is->last_vis_time + rdftspeed - time) : (*remaining_time));
    }
    if (is->video_st) {
retry:
        if (frame_queue_nb_remaining(&is->pictq) == 0) {
        } else {
            double last_duration, duration, delay;
            Frame *vp, *lastvp;
            lastvp = frame_queue_peek_last(&is->pictq);
            vp = frame_queue_peek(&is->pictq);
            if (vp->serial != is->videoq.serial) {
                frame_queue_next(&is->pictq);
                goto retry;
            }
            if (lastvp->serial != vp->serial)
                is->frame_timer = av_gettime_relative() / 1000000.0;
            if (is->paused)
                goto display;
            last_duration = vp_duration(is, lastvp, vp);
            delay = compute_target_delay(last_duration, is);
            time= av_gettime_relative()/1000000.0;
            if (time < is->frame_timer + delay) {
                *remaining_time = ((is->frame_timer + delay - time) > (*remaining_time) ? (*remaining_time) : (is->frame_timer + delay - time));
                goto display;
            }
            is->frame_timer += delay;
            if (delay > 0 && time - is->frame_timer > 0.1)
                is->frame_timer = time;
            SDL_LockMutex(is->pictq.mutex);
            if (!
                __builtin_choose_expr ( __builtin_types_compatible_p (__typeof__ (
                vp->pts
                ), double), __isnan(
                vp->pts
                ), __builtin_choose_expr ( __builtin_types_compatible_p (__typeof__ (
                vp->pts
                ), float), __isnanf(
                vp->pts
                ), __builtin_choose_expr ( __builtin_types_compatible_p (__typeof__ (
                vp->pts
                ), long double), __isnanl(
                vp->pts
                ), (__builtin_trap(),
                vp->pts
                ))))
                              )
                update_video_pts(is, vp->pts, vp->pos, vp->serial);
            SDL_UnlockMutex(is->pictq.mutex);
            if (frame_queue_nb_remaining(&is->pictq) > 1) {
                Frame *nextvp = frame_queue_peek_next(&is->pictq);
                duration = vp_duration(is, vp, nextvp);
                if(!is->step && (framedrop>0 || (framedrop && get_master_sync_type(is) != AV_SYNC_VIDEO_MASTER)) && time > is->frame_timer + duration){
                    is->frame_drops_late++;
                    frame_queue_next(&is->pictq);
                    goto retry;
                }
            }
            if (is->subtitle_st) {
                    while (frame_queue_nb_remaining(&is->subpq) > 0) {
                        sp = frame_queue_peek(&is->subpq);
                        if (frame_queue_nb_remaining(&is->subpq) > 1)
                            sp2 = frame_queue_peek_next(&is->subpq);
                        else
                            sp2 = 
                                 ((void *)0)
                                     ;
                        if (sp->serial != is->subtitleq.serial
                                || (is->vidclk.pts > (sp->pts + ((float) sp->sub.end_display_time / 1000)))
                                || (sp2 && is->vidclk.pts > (sp2->pts + ((float) sp2->sub.start_display_time / 1000))))
                        {
                            if (sp->uploaded) {
                                int i;
                                for (i = 0; i < sp->sub.num_rects; i++) {
                                    AVSubtitleRect *sub_rect = sp->sub.rects[i];
                                    uint8_t *pixels;
                                    int pitch, j;
                                    if (!SDL_LockTexture(is->sub_texture, (SDL_Rect *)sub_rect, (void **)&pixels, &pitch)) {
                                        for (j = 0; j < sub_rect->h; j++, pixels += pitch)
                                            memset(pixels, 0, sub_rect->w << 2);
                                        SDL_UnlockTexture(is->sub_texture);
                                    }
                                }
                            }
                            frame_queue_next(&is->subpq);
                        } else {
                            break;
                        }
                    }
            }
            frame_queue_next(&is->pictq);
            is->force_refresh = 1;
            if (is->step && !is->paused)
                stream_toggle_pause(is);
        }
display:
        if (!display_disable && is->force_refresh && is->show_mode == SHOW_MODE_VIDEO && is->pictq.rindex_shown)
            video_display(is);
    }
    is->force_refresh = 0;
    if (show_status) {
        AVBPrint buf;
        static int64_t last_time;
        int64_t cur_time;
        int aqsize, vqsize, sqsize;
        double av_diff;
        cur_time = av_gettime_relative();
        if (!last_time || (cur_time - last_time) >= 30000) {
            aqsize = 0;
            vqsize = 0;
            sqsize = 0;
            if (is->audio_st)
                aqsize = is->audioq.size;
            if (is->video_st)
                vqsize = is->videoq.size;
            if (is->subtitle_st)
                sqsize = is->subtitleq.size;
            av_diff = 0;
            if (is->audio_st && is->video_st)
                av_diff = get_clock(&is->audclk) - get_clock(&is->vidclk);
            else if (is->video_st)
                av_diff = get_master_clock(is) - get_clock(&is->vidclk);
            else if (is->audio_st)
                av_diff = get_master_clock(is) - get_clock(&is->audclk);
            av_bprint_init(&buf, 0, 1);
            av_bprintf(&buf,
                      "%7.2f %s:%7.3f fd=%4d aq=%5dKB vq=%5dKB sq=%5dB f=%"
                                                                          "lld"
                                                                                "/%"
                                                                                    "lld"
                                                                                          "   \r",
                      get_master_clock(is),
                      (is->audio_st && is->video_st) ? "A-V" : (is->video_st ? "M-V" : (is->audio_st ? "M-A" : "   ")),
                      av_diff,
                      is->frame_drops_early + is->frame_drops_late,
                      aqsize / 1024,
                      vqsize / 1024,
                      sqsize,
                      is->video_st ? is->viddec.avctx->pts_correction_num_faulty_dts : 0,
                      is->video_st ? is->viddec.avctx->pts_correction_num_faulty_pts : 0);
            if (show_status == 1 && 32 > av_log_get_level())
                fprintf(
                       (__acrt_iob_func(2))
                             , "%s", buf.str);
            else
                av_log(
                      ((void *)0)
                          , 32, "%s", buf.str);
            fflush(
                  (__acrt_iob_func(2))
                        );
            av_bprint_finalize(&buf, 
                                    ((void *)0)
                                        );
            last_time = cur_time;
        }
    }
}

static int queue_picture(VideoState *is, AVFrame *src_frame, double pts, double duration, int64_t pos, int serial)
{
    Frame *vp;
    if (!(vp = frame_queue_peek_writable(&is->pictq)))
        return -1;
    vp->sar = src_frame->sample_aspect_ratio;
    vp->uploaded = 0;
    vp->width = src_frame->width;
    vp->height = src_frame->height;
    vp->format = src_frame->format;
    vp->pts = pts;
    vp->duration = duration;
    vp->pos = pos;
    vp->serial = serial;
    set_default_window_size(vp->width, vp->height, vp->sar);
    av_frame_move_ref(vp->frame, src_frame);
    frame_queue_push(&is->pictq);
    return 0;
}

static int get_video_frame(VideoState *is, AVFrame *frame)
{
    int got_picture;
    if ((got_picture = decoder_decode_frame(&is->viddec, frame, 
                                                               ((void *)0)
                                                                   )) < 0)
        return -1;
    if (got_picture) {
        double dpts = 
                     __builtin_nanf("")
                        ;
        if (frame->pts != ((int64_t)0x8000000000000000ULL))
            dpts = av_q2d(is->video_st->time_base) * frame->pts;
        frame->sample_aspect_ratio = av_guess_sample_aspect_ratio(is->ic, is->video_st, frame);
        if (framedrop>0 || (framedrop && get_master_sync_type(is) != AV_SYNC_VIDEO_MASTER)) {
            if (frame->pts != ((int64_t)0x8000000000000000ULL)) {
                double diff = dpts - get_master_clock(is);
                if (!
                    __builtin_choose_expr ( __builtin_types_compatible_p (__typeof__ (
                    diff
                    ), double), __isnan(
                    diff
                    ), __builtin_choose_expr ( __builtin_types_compatible_p (__typeof__ (
                    diff
                    ), float), __isnanf(
                    diff
                    ), __builtin_choose_expr ( __builtin_types_compatible_p (__typeof__ (
                    diff
                    ), long double), __isnanl(
                    diff
                    ), (__builtin_trap(),
                    diff
                    )))) 
                                && fabs(diff) < 10.0 &&
                    diff - is->frame_last_filter_delay < 0 &&
                    is->viddec.pkt_serial == is->vidclk.serial &&
                    is->videoq.nb_packets) {
                    is->frame_drops_early++;
                    av_frame_unref(frame);
                    got_picture = 0;
                }
            }
        }
    }
    return got_picture;
}

static int configure_filtergraph(AVFilterGraph *graph, const char *filtergraph,
                                 AVFilterContext *source_ctx, AVFilterContext *sink_ctx)
{
    int ret, i;
    int nb_filters = graph->nb_filters;
    AVFilterInOut *outputs = 
                            ((void *)0)
                                , *inputs = 
                                            ((void *)0)
                                                ;
    if (filtergraph) {
        outputs = avfilter_inout_alloc();
        inputs = avfilter_inout_alloc();
        if (!outputs || !inputs) {
            ret = (-(
                 12
                 ));
            goto fail;
        }
        outputs->name = av_strdup("in");
        outputs->filter_ctx = source_ctx;
        outputs->pad_idx = 0;
        outputs->next = 
                             ((void *)0)
                                 ;
        inputs->name = av_strdup("out");
        inputs->filter_ctx = sink_ctx;
        inputs->pad_idx = 0;
        inputs->next = 
                             ((void *)0)
                                 ;
        if ((ret = avfilter_graph_parse_ptr(graph, filtergraph, &inputs, &outputs, 
                                                                                  ((void *)0)
                                                                                      )) < 0)
            goto fail;
    } else {
        if ((ret = avfilter_link(source_ctx, 0, sink_ctx, 0)) < 0)
            goto fail;
    }
    for (i = 0; i < graph->nb_filters - nb_filters; i++)
        do{AVFilterContext* SWAP_tmp= graph->filters[i + nb_filters]; graph->filters[i + nb_filters]= graph->filters[i]; graph->filters[i]= SWAP_tmp;}while(0);
    ret = avfilter_graph_config(graph, 
                                      ((void *)0)
                                          );
fail:
    avfilter_inout_free(&outputs);
    avfilter_inout_free(&inputs);
    return ret;
}

static int configure_video_filters(AVFilterGraph *graph, VideoState *is, const char *vfilters, AVFrame *frame)
{
    enum AVPixelFormat pix_fmts[(sizeof(sdl_texture_format_map) / sizeof((sdl_texture_format_map)[0]))];
    char sws_flags_str[512] = "";
    char buffersrc_args[256];
    int ret;
    AVFilterContext *filt_src = 
                               ((void *)0)
                                   , *filt_out = 
                                                 ((void *)0)
                                                     , *last_filter = 
                                                                      ((void *)0)
                                                                          ;
    AVCodecParameters *codecpar = is->video_st->codecpar;
    AVRational fr = av_guess_frame_rate(is->ic, is->video_st, 
                                                             ((void *)0)
                                                                 );
    AVDictionaryEntry *e = 
                          ((void *)0)
                              ;
    int nb_pix_fmts = 0;
    int i, j;
    for (i = 0; i < renderer_info.num_texture_formats; i++) {
        for (j = 0; j < (sizeof(sdl_texture_format_map) / sizeof((sdl_texture_format_map)[0])) - 1; j++) {
            if (renderer_info.texture_formats[i] == sdl_texture_format_map[j].texture_fmt) {
                pix_fmts[nb_pix_fmts++] = sdl_texture_format_map[j].format;
                break;
            }
        }
    }
    pix_fmts[nb_pix_fmts] = AV_PIX_FMT_NONE;
    while ((e = av_dict_get(sws_dict, "", e, 2))) {
        if (!strcmp(e->key, "sws_flags")) {
            av_strlcatf(sws_flags_str, sizeof(sws_flags_str), "%s=%s:", "flags", e->value);
        } else
            av_strlcatf(sws_flags_str, sizeof(sws_flags_str), "%s=%s:", e->key, e->value);
    }
    if (strlen(sws_flags_str))
        sws_flags_str[strlen(sws_flags_str)-1] = '\0';
    graph->scale_sws_opts = av_strdup(sws_flags_str);
    snprintf(buffersrc_args, sizeof(buffersrc_args),
             "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
             frame->width, frame->height, frame->format,
             is->video_st->time_base.num, is->video_st->time_base.den,
             codecpar->sample_aspect_ratio.num, ((codecpar->sample_aspect_ratio.den) > (1) ? (codecpar->sample_aspect_ratio.den) : (1)));
    if (fr.num && fr.den)
        av_strlcatf(buffersrc_args, sizeof(buffersrc_args), ":frame_rate=%d/%d", fr.num, fr.den);
    if ((ret = avfilter_graph_create_filter(&filt_src,
                                            avfilter_get_by_name("buffer"),
                                            "ffplay_buffer", buffersrc_args, 
                                                                            ((void *)0)
                                                                                ,
                                            graph)) < 0)
        goto fail;
    ret = avfilter_graph_create_filter(&filt_out,
                                       avfilter_get_by_name("buffersink"),
                                       "ffplay_buffersink", 
                                                           ((void *)0)
                                                               , 
                                                                 ((void *)0)
                                                                     , graph);
    if (ret < 0)
        goto fail;
    if ((ret = (av_int_list_length_for_size(sizeof(*(pix_fmts)), pix_fmts, AV_PIX_FMT_NONE) > 0x7fffffff / sizeof(*(pix_fmts)) ? (-(
              22
              )) : av_opt_set_bin(filt_out, "pix_fmts", (const uint8_t *)(pix_fmts), av_int_list_length_for_size(sizeof(*(pix_fmts)), pix_fmts, AV_PIX_FMT_NONE) * sizeof(*(pix_fmts)), (1 << 0)))) < 0)
        goto fail;
    last_filter = filt_out;
#define INSERT_FILT(name,arg) do { AVFilterContext *filt_ctx; ret = avfilter_graph_create_filter(&filt_ctx, avfilter_get_by_name(name), "ffplay_" name, arg, NULL, graph); if (ret < 0) goto fail; ret = avfilter_link(filt_ctx, 0, last_filter, 0); if (ret < 0) goto fail; last_filter = filt_ctx; } while (0)
    if (autorotate) {
        double theta = get_rotation(is->video_st);
        if (fabs(theta - 90) < 1.0) {
            do { AVFilterContext *filt_ctx; ret = avfilter_graph_create_filter(&filt_ctx, avfilter_get_by_name("transpose"), "ffplay_" "transpose", "clock", 
           ((void *)0)
           , graph); if (ret < 0) goto fail; ret = avfilter_link(filt_ctx, 0, last_filter, 0); if (ret < 0) goto fail; last_filter = filt_ctx; } while (0);
        } else if (fabs(theta - 180) < 1.0) {
            do { AVFilterContext *filt_ctx; ret = avfilter_graph_create_filter(&filt_ctx, avfilter_get_by_name("hflip"), "ffplay_" "hflip", 
           ((void *)0)
           , 
           ((void *)0)
           , graph); if (ret < 0) goto fail; ret = avfilter_link(filt_ctx, 0, last_filter, 0); if (ret < 0) goto fail; last_filter = filt_ctx; } while (0);
            do { AVFilterContext *filt_ctx; ret = avfilter_graph_create_filter(&filt_ctx, avfilter_get_by_name("vflip"), "ffplay_" "vflip", 
           ((void *)0)
           , 
           ((void *)0)
           , graph); if (ret < 0) goto fail; ret = avfilter_link(filt_ctx, 0, last_filter, 0); if (ret < 0) goto fail; last_filter = filt_ctx; } while (0);
        } else if (fabs(theta - 270) < 1.0) {
            do { AVFilterContext *filt_ctx; ret = avfilter_graph_create_filter(&filt_ctx, avfilter_get_by_name("transpose"), "ffplay_" "transpose", "cclock", 
           ((void *)0)
           , graph); if (ret < 0) goto fail; ret = avfilter_link(filt_ctx, 0, last_filter, 0); if (ret < 0) goto fail; last_filter = filt_ctx; } while (0);
        } else if (fabs(theta) > 1.0) {
            char rotate_buf[64];
            snprintf(rotate_buf, sizeof(rotate_buf), "%f*PI/180", theta);
            do { AVFilterContext *filt_ctx; ret = avfilter_graph_create_filter(&filt_ctx, avfilter_get_by_name("rotate"), "ffplay_" "rotate", rotate_buf, 
           ((void *)0)
           , graph); if (ret < 0) goto fail; ret = avfilter_link(filt_ctx, 0, last_filter, 0); if (ret < 0) goto fail; last_filter = filt_ctx; } while (0);
        }
    }
    if ((ret = configure_filtergraph(graph, vfilters, filt_src, last_filter)) < 0)
        goto fail;
    is->in_video_filter = filt_src;
    is->out_video_filter = filt_out;

fail:
    return ret;
}

static int configure_audio_filters(VideoState *is, const char *afilters, int force_output_format)
{
    static const enum AVSampleFormat sample_fmts[] = { AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_NONE };
    int sample_rates[2] = { 0, -1 };
    int64_t channel_layouts[2] = { 0, -1 };
    int channels[2] = { 0, -1 };
    AVFilterContext *filt_asrc = 
                                ((void *)0)
                                    , *filt_asink = 
                                                    ((void *)0)
                                                        ;
    char aresample_swr_opts[512] = "";
    AVDictionaryEntry *e = 
                          ((void *)0)
                              ;
    char asrc_args[256];
    int ret;
    avfilter_graph_free(&is->agraph);
    if (!(is->agraph = avfilter_graph_alloc()))
        return (-(
              12
              ));
    is->agraph->nb_threads = filter_nbthreads;
    while ((e = av_dict_get(swr_opts, "", e, 2)))
        av_strlcatf(aresample_swr_opts, sizeof(aresample_swr_opts), "%s=%s:", e->key, e->value);
    if (strlen(aresample_swr_opts))
        aresample_swr_opts[strlen(aresample_swr_opts)-1] = '\0';
    av_opt_set(is->agraph, "aresample_swr_opts", aresample_swr_opts, 0);
    ret = snprintf(asrc_args, sizeof(asrc_args),
                   "sample_rate=%d:sample_fmt=%s:channels=%d:time_base=%d/%d",
                   is->audio_filter_src.freq, av_get_sample_fmt_name(is->audio_filter_src.fmt),
                   is->audio_filter_src.channels,
                   1, is->audio_filter_src.freq);
    if (is->audio_filter_src.channel_layout)
        snprintf(asrc_args + ret, sizeof(asrc_args) - ret,
                 ":channel_layout=0x%"
                                     "llx"
                                           , is->audio_filter_src.channel_layout);
    ret = avfilter_graph_create_filter(&filt_asrc,
                                       avfilter_get_by_name("abuffer"), "ffplay_abuffer",
                                       asrc_args, 
                                                 ((void *)0)
                                                     , is->agraph);
    if (ret < 0)
        goto end;
    ret = avfilter_graph_create_filter(&filt_asink,
                                       avfilter_get_by_name("abuffersink"), "ffplay_abuffersink",
                                       
                                      ((void *)0)
                                          , 
                                            ((void *)0)
                                                , is->agraph);
    if (ret < 0)
        goto end;
    if ((ret = (av_int_list_length_for_size(sizeof(*(sample_fmts)), sample_fmts, AV_SAMPLE_FMT_NONE) > 0x7fffffff / sizeof(*(sample_fmts)) ? (-(
              22
              )) : av_opt_set_bin(filt_asink, "sample_fmts", (const uint8_t *)(sample_fmts), av_int_list_length_for_size(sizeof(*(sample_fmts)), sample_fmts, AV_SAMPLE_FMT_NONE) * sizeof(*(sample_fmts)), (1 << 0)))) < 0)
        goto end;
    if ((ret = av_opt_set_int(filt_asink, "all_channel_counts", 1, (1 << 0))) < 0)
        goto end;
    if (force_output_format) {
        channel_layouts[0] = is->audio_tgt.channel_layout;
        channels [0] = is->audio_tgt.channel_layout ? -1 : is->audio_tgt.channels;
        sample_rates [0] = is->audio_tgt.freq;
        if ((ret = av_opt_set_int(filt_asink, "all_channel_counts", 0, (1 << 0))) < 0)
            goto end;
        if ((ret = (av_int_list_length_for_size(sizeof(*(channel_layouts)), channel_layouts, -1) > 0x7fffffff / sizeof(*(channel_layouts)) ? (-(
                  22
                  )) : av_opt_set_bin(filt_asink, "channel_layouts", (const uint8_t *)(channel_layouts), av_int_list_length_for_size(sizeof(*(channel_layouts)), channel_layouts, -1) * sizeof(*(channel_layouts)), (1 << 0)))) < 0)
            goto end;
        if ((ret = (av_int_list_length_for_size(sizeof(*(channels)), channels, -1) > 0x7fffffff / sizeof(*(channels)) ? (-(
                  22
                  )) : av_opt_set_bin(filt_asink, "channel_counts", (const uint8_t *)(channels), av_int_list_length_for_size(sizeof(*(channels)), channels, -1) * sizeof(*(channels)), (1 << 0)))) < 0)
            goto end;
        if ((ret = (av_int_list_length_for_size(sizeof(*(sample_rates)), sample_rates, -1) > 0x7fffffff / sizeof(*(sample_rates)) ? (-(
                  22
                  )) : av_opt_set_bin(filt_asink, "sample_rates", (const uint8_t *)(sample_rates), av_int_list_length_for_size(sizeof(*(sample_rates)), sample_rates, -1) * sizeof(*(sample_rates)), (1 << 0)))) < 0)
            goto end;
    }
    if ((ret = configure_filtergraph(is->agraph, afilters, filt_asrc, filt_asink)) < 0)
        goto end;
    is->in_audio_filter = filt_asrc;
    is->out_audio_filter = filt_asink;

end:
    if (ret < 0)
        avfilter_graph_free(&is->agraph);
    return ret;
}

static int audio_thread(void *arg)
{
    VideoState *is = arg;
    AVFrame *frame = av_frame_alloc();
    Frame *af;
    int last_serial = -1;
    int64_t dec_channel_layout;
    int reconfigure;
    int got_frame = 0;
    AVRational tb;
    int ret = 0;
    if (!frame)
        return (-(
              12
              ));
    do {
        if ((got_frame = decoder_decode_frame(&is->auddec, frame, 
                                                                 ((void *)0)
                                                                     )) < 0)
            goto the_end;
        if (got_frame) {
                tb = (AVRational){1, frame->sample_rate};
                dec_channel_layout = get_valid_channel_layout(frame->channel_layout, frame->channels);
                reconfigure =
                    cmp_audio_fmts(is->audio_filter_src.fmt, is->audio_filter_src.channels,
                                   frame->format, frame->channels) ||
                    is->audio_filter_src.channel_layout != dec_channel_layout ||
                    is->audio_filter_src.freq != frame->sample_rate ||
                    is->auddec.pkt_serial != last_serial;
                if (reconfigure) {
                    char buf1[1024], buf2[1024];
                    av_get_channel_layout_string(buf1, sizeof(buf1), -1, is->audio_filter_src.channel_layout);
                    av_get_channel_layout_string(buf2, sizeof(buf2), -1, dec_channel_layout);
                    av_log(
                          ((void *)0)
                              , 48,
                           "Audio frame changed from rate:%d ch:%d fmt:%s layout:%s serial:%d to rate:%d ch:%d fmt:%s layout:%s serial:%d\n",
                           is->audio_filter_src.freq, is->audio_filter_src.channels, av_get_sample_fmt_name(is->audio_filter_src.fmt), buf1, last_serial,
                           frame->sample_rate, frame->channels, av_get_sample_fmt_name(frame->format), buf2, is->auddec.pkt_serial);
                    is->audio_filter_src.fmt = frame->format;
                    is->audio_filter_src.channels = frame->channels;
                    is->audio_filter_src.channel_layout = dec_channel_layout;
                    is->audio_filter_src.freq = frame->sample_rate;
                    last_serial = is->auddec.pkt_serial;
                    if ((ret = configure_audio_filters(is, afilters, 1)) < 0)
                        goto the_end;
                }
            if ((ret = av_buffersrc_add_frame(is->in_audio_filter, frame)) < 0)
                goto the_end;
            while ((ret = av_buffersink_get_frame_flags(is->out_audio_filter, frame, 0)) >= 0) {
                tb = av_buffersink_get_time_base(is->out_audio_filter);
                if (!(af = frame_queue_peek_writable(&is->sampq)))
                    goto the_end;
                af->pts = (frame->pts == ((int64_t)0x8000000000000000ULL)) ? 
                                                          __builtin_nanf("") 
                                                              : frame->pts * av_q2d(tb);
                af->pos = frame->pkt_pos;
                af->serial = is->auddec.pkt_serial;
                af->duration = av_q2d((AVRational){frame->nb_samples, frame->sample_rate});
                av_frame_move_ref(af->frame, frame);
                frame_queue_push(&is->sampq);
                if (is->audioq.serial != is->auddec.pkt_serial)
                    break;
            }
            if (ret == (-(int)(('E') | (('O') << 8) | (('F') << 16) | ((unsigned)(' ') << 24))))
                is->auddec.finished = is->auddec.pkt_serial;
        }
    } while (ret >= 0 || ret == (-(
                               11
                               )) || ret == (-(int)(('E') | (('O') << 8) | (('F') << 16) | ((unsigned)(' ') << 24))));
 the_end:
    avfilter_graph_free(&is->agraph);
    av_frame_free(&frame);
    return ret;
}

static int decoder_start(Decoder *d, int (*fn)(void *), const char *thread_name, void* arg)
{
    packet_queue_start(d->queue);
    d->decoder_tid = SDL_CreateThread(fn, thread_name, arg);
    if (!d->decoder_tid) {
        av_log(
              ((void *)0)
                  , 16, "SDL_CreateThread(): %s\n", SDL_GetError());
        return (-(
              12
              ));
    }
    return 0;
}

static int video_thread(void *arg)
{
    VideoState *is = arg;
    AVFrame *frame = av_frame_alloc();
    double pts;
    double duration;
    int ret;
    AVRational tb = is->video_st->time_base;
    AVRational frame_rate = av_guess_frame_rate(is->ic, is->video_st, 
                                                                     ((void *)0)
                                                                         );
    AVFilterGraph *graph = 
                          ((void *)0)
                              ;
    AVFilterContext *filt_out = 
                               ((void *)0)
                                   , *filt_in = 
                                                ((void *)0)
                                                    ;
    int last_w = 0;
    int last_h = 0;
    enum AVPixelFormat last_format = -2;
    int last_serial = -1;
    int last_vfilter_idx = 0;
    if (!frame)
        return (-(
              12
              ));
    for (;;) {
        ret = get_video_frame(is, frame);
        if (ret < 0)
            goto the_end;
        if (!ret)
            continue;
        if ( last_w != frame->width
            || last_h != frame->height
            || last_format != frame->format
            || last_serial != is->viddec.pkt_serial
            || last_vfilter_idx != is->vfilter_idx) {
            av_log(
                  ((void *)0)
                      , 48,
                   "Video frame changed from size:%dx%d format:%s serial:%d to size:%dx%d format:%s serial:%d\n",
                   last_w, last_h,
                   (const char *)av_x_if_null(av_get_pix_fmt_name(last_format), "none"), last_serial,
                   frame->width, frame->height,
                   (const char *)av_x_if_null(av_get_pix_fmt_name(frame->format), "none"), is->viddec.pkt_serial);
            avfilter_graph_free(&graph);
            graph = avfilter_graph_alloc();
            if (!graph) {
                ret = (-(
                     12
                     ));
                goto the_end;
            }
            graph->nb_threads = filter_nbthreads;
            if ((ret = configure_video_filters(graph, is, vfilters_list ? vfilters_list[is->vfilter_idx] : 
                                                                                                          ((void *)0)
                                                                                                              , frame)) < 0) {
                SDL_Event event;
                event.type = (SDL_USEREVENT + 2);
                event.user.data1 = is;
                SDL_PushEvent(&event);
                goto the_end;
            }
            filt_in = is->in_video_filter;
            filt_out = is->out_video_filter;
            last_w = frame->width;
            last_h = frame->height;
            last_format = frame->format;
            last_serial = is->viddec.pkt_serial;
            last_vfilter_idx = is->vfilter_idx;
            frame_rate = av_buffersink_get_frame_rate(filt_out);
        }
        ret = av_buffersrc_add_frame(filt_in, frame);
        if (ret < 0)
            goto the_end;
        while (ret >= 0) {
            is->frame_last_returned_time = av_gettime_relative() / 1000000.0;
            ret = av_buffersink_get_frame_flags(filt_out, frame, 0);
            if (ret < 0) {
                if (ret == (-(int)(('E') | (('O') << 8) | (('F') << 16) | ((unsigned)(' ') << 24))))
                    is->viddec.finished = is->viddec.pkt_serial;
                ret = 0;
                break;
            }
            is->frame_last_filter_delay = av_gettime_relative() / 1000000.0 - is->frame_last_returned_time;
            if (fabs(is->frame_last_filter_delay) > 10.0 / 10.0)
                is->frame_last_filter_delay = 0;
            tb = av_buffersink_get_time_base(filt_out);
            duration = (frame_rate.num && frame_rate.den ? av_q2d((AVRational){frame_rate.den, frame_rate.num}) : 0);
            pts = (frame->pts == ((int64_t)0x8000000000000000ULL)) ? 
                                                  __builtin_nanf("") 
                                                      : frame->pts * av_q2d(tb);
            ret = queue_picture(is, frame, pts, duration, frame->pkt_pos, is->viddec.pkt_serial);
            av_frame_unref(frame);
            if (is->videoq.serial != is->viddec.pkt_serial)
                break;
        }
        if (ret < 0)
            goto the_end;
    }
 the_end:
    avfilter_graph_free(&graph);
    av_frame_free(&frame);
    return 0;
}

static int subtitle_thread(void *arg)
{
    VideoState *is = arg;
    Frame *sp;
    int got_subtitle;
    double pts;
    for (;;) {
        if (!(sp = frame_queue_peek_writable(&is->subpq)))
            return 0;
        if ((got_subtitle = decoder_decode_frame(&is->subdec, 
                                                             ((void *)0)
                                                                 , &sp->sub)) < 0)
            break;
        pts = 0;
        if (got_subtitle && sp->sub.format == 0) {
            if (sp->sub.pts != ((int64_t)0x8000000000000000ULL))
                pts = sp->sub.pts / (double)1000000;
            sp->pts = pts;
            sp->serial = is->subdec.pkt_serial;
            sp->width = is->subdec.avctx->width;
            sp->height = is->subdec.avctx->height;
            sp->uploaded = 0;
            frame_queue_push(&is->subpq);
        } else if (got_subtitle) {
            avsubtitle_free(&sp->sub);
        }
    }
    return 0;
}

static void update_sample_display(VideoState *is, short *samples, int samples_size)
{
    int size, len;
    size = samples_size / sizeof(short);
    while (size > 0) {
        len = (8 * 65536) - is->sample_array_index;
        if (len > size)
            len = size;
        memcpy(is->sample_array + is->sample_array_index, samples, len * sizeof(short));
        samples += len;
        is->sample_array_index += len;
        if (is->sample_array_index >= (8 * 65536))
            is->sample_array_index = 0;
        size -= len;
    }
}

static int synchronize_audio(VideoState *is, int nb_samples)
{
    int wanted_nb_samples = nb_samples;
    if (get_master_sync_type(is) != AV_SYNC_AUDIO_MASTER) {
        double diff, avg_diff;
        int min_nb_samples, max_nb_samples;
        diff = get_clock(&is->audclk) - get_master_clock(is);
        if (!
            __builtin_choose_expr ( __builtin_types_compatible_p (__typeof__ (
            diff
            ), double), __isnan(
            diff
            ), __builtin_choose_expr ( __builtin_types_compatible_p (__typeof__ (
            diff
            ), float), __isnanf(
            diff
            ), __builtin_choose_expr ( __builtin_types_compatible_p (__typeof__ (
            diff
            ), long double), __isnanl(
            diff
            ), (__builtin_trap(),
            diff
            )))) 
                        && fabs(diff) < 10.0) {
            is->audio_diff_cum = diff + is->audio_diff_avg_coef * is->audio_diff_cum;
            if (is->audio_diff_avg_count < 20) {
                is->audio_diff_avg_count++;
            } else {
                avg_diff = is->audio_diff_cum * (1.0 - is->audio_diff_avg_coef);
                if (fabs(avg_diff) >= is->audio_diff_threshold) {
                    wanted_nb_samples = nb_samples + (int)(diff * is->audio_src.freq);
                    min_nb_samples = ((nb_samples * (100 - 10) / 100));
                    max_nb_samples = ((nb_samples * (100 + 10) / 100));
                    wanted_nb_samples = av_clip_c(wanted_nb_samples, min_nb_samples, max_nb_samples);
                }
                av_log(
                      ((void *)0)
                          , 56, "diff=%f adiff=%f sample_diff=%d apts=%0.3f %f\n",
                        diff, avg_diff, wanted_nb_samples - nb_samples,
                        is->audio_clock, is->audio_diff_threshold);
            }
        } else {
            is->audio_diff_avg_count = 0;
            is->audio_diff_cum = 0;
        }
    }
    return wanted_nb_samples;
}
static int audio_decode_frame(VideoState *is)
{
    int data_size, resampled_data_size;
    int64_t dec_channel_layout;
    double audio_clock0;
    int wanted_nb_samples;
    Frame *af;
    if (is->paused)
        return -1;
    do {
        while (frame_queue_nb_remaining(&is->sampq) == 0) {
            if ((av_gettime_relative() - audio_callback_time) > 1000000LL * is->audio_hw_buf_size / is->audio_tgt.bytes_per_sec / 2)
                return -1;
            av_usleep (1000);
        }
        if (!(af = frame_queue_peek_readable(&is->sampq)))
            return -1;
        frame_queue_next(&is->sampq);
    } while (af->serial != is->audioq.serial);
    data_size = av_samples_get_buffer_size(
                                          ((void *)0)
                                              , af->frame->channels,
                                           af->frame->nb_samples,
                                           af->frame->format, 1);
    dec_channel_layout =
        (af->frame->channel_layout && af->frame->channels == av_get_channel_layout_nb_channels(af->frame->channel_layout)) ?
        af->frame->channel_layout : av_get_default_channel_layout(af->frame->channels);
    wanted_nb_samples = synchronize_audio(is, af->frame->nb_samples);
    if (af->frame->format != is->audio_src.fmt ||
        dec_channel_layout != is->audio_src.channel_layout ||
        af->frame->sample_rate != is->audio_src.freq ||
        (wanted_nb_samples != af->frame->nb_samples && !is->swr_ctx)) {
        swr_free(&is->swr_ctx);
        is->swr_ctx = swr_alloc_set_opts(
                                        ((void *)0)
                                            ,
                                         is->audio_tgt.channel_layout, is->audio_tgt.fmt, is->audio_tgt.freq,
                                         dec_channel_layout, af->frame->format, af->frame->sample_rate,
                                         0, 
                                           ((void *)0)
                                               );
        if (!is->swr_ctx || swr_init(is->swr_ctx) < 0) {
            av_log(
                  ((void *)0)
                      , 16,
                   "Cannot create sample rate converter for conversion of %d Hz %s %d channels to %d Hz %s %d channels!\n",
                    af->frame->sample_rate, av_get_sample_fmt_name(af->frame->format), af->frame->channels,
                    is->audio_tgt.freq, av_get_sample_fmt_name(is->audio_tgt.fmt), is->audio_tgt.channels);
            swr_free(&is->swr_ctx);
            return -1;
        }
        is->audio_src.channel_layout = dec_channel_layout;
        is->audio_src.channels = af->frame->channels;
        is->audio_src.freq = af->frame->sample_rate;
        is->audio_src.fmt = af->frame->format;
    }
    if (is->swr_ctx) {
        const uint8_t **in = (const uint8_t **)af->frame->extended_data;
        uint8_t **out = &is->audio_buf1;
        int out_count = (int64_t)wanted_nb_samples * is->audio_tgt.freq / af->frame->sample_rate + 256;
        int out_size = av_samples_get_buffer_size(
                                                  ((void *)0)
                                                      , is->audio_tgt.channels, out_count, is->audio_tgt.fmt, 0);
        int len2;
        if (out_size < 0) {
            av_log(
                  ((void *)0)
                      , 16, "av_samples_get_buffer_size() failed\n");
            return -1;
        }
        if (wanted_nb_samples != af->frame->nb_samples) {
            if (swr_set_compensation(is->swr_ctx, (wanted_nb_samples - af->frame->nb_samples) * is->audio_tgt.freq / af->frame->sample_rate,
                                        wanted_nb_samples * is->audio_tgt.freq / af->frame->sample_rate) < 0) {
                av_log(
                      ((void *)0)
                          , 16, "swr_set_compensation() failed\n");
                return -1;
            }
        }
        av_fast_malloc(&is->audio_buf1, &is->audio_buf1_size, out_size);
        if (!is->audio_buf1)
            return (-(
                  12
                  ));
        len2 = swr_convert(is->swr_ctx, out, out_count, in, af->frame->nb_samples);
        if (len2 < 0) {
            av_log(
                  ((void *)0)
                      , 16, "swr_convert() failed\n");
            return -1;
        }
        if (len2 == out_count) {
            av_log(
                  ((void *)0)
                      , 24, "audio buffer is probably too small\n");
            if (swr_init(is->swr_ctx) < 0)
                swr_free(&is->swr_ctx);
        }
        is->audio_buf = is->audio_buf1;
        resampled_data_size = len2 * is->audio_tgt.channels * av_get_bytes_per_sample(is->audio_tgt.fmt);
    } else {
        is->audio_buf = af->frame->data[0];
        resampled_data_size = data_size;
    }
    audio_clock0 = is->audio_clock;
    if (!
        __builtin_choose_expr ( __builtin_types_compatible_p (__typeof__ (
        af->pts
        ), double), __isnan(
        af->pts
        ), __builtin_choose_expr ( __builtin_types_compatible_p (__typeof__ (
        af->pts
        ), float), __isnanf(
        af->pts
        ), __builtin_choose_expr ( __builtin_types_compatible_p (__typeof__ (
        af->pts
        ), long double), __isnanl(
        af->pts
        ), (__builtin_trap(),
        af->pts
        ))))
                      )
        is->audio_clock = af->pts + (double) af->frame->nb_samples / af->frame->sample_rate;
    else
        is->audio_clock = 
                         __builtin_nanf("")
                            ;
    is->audio_clock_serial = af->serial;
    return resampled_data_size;
}

static void sdl_audio_callback(void *opaque, Uint8 *stream, int len)
{
    VideoState *is = opaque;
    int audio_size, len1;
    audio_callback_time = av_gettime_relative();
    while (len > 0) {
        if (is->audio_buf_index >= is->audio_buf_size) {
           audio_size = audio_decode_frame(is);
           if (audio_size < 0) {
               is->audio_buf = 
                              ((void *)0)
                                  ;
               is->audio_buf_size = 512 / is->audio_tgt.frame_size * is->audio_tgt.frame_size;
           } else {
               if (is->show_mode != SHOW_MODE_VIDEO)
                   update_sample_display(is, (int16_t *)is->audio_buf, audio_size);
               is->audio_buf_size = audio_size;
           }
           is->audio_buf_index = 0;
        }
        len1 = is->audio_buf_size - is->audio_buf_index;
        if (len1 > len)
            len1 = len;
        if (!is->muted && is->audio_buf && is->audio_volume == 128)
            memcpy(stream, (uint8_t *)is->audio_buf + is->audio_buf_index, len1);
        else {
            memset(stream, 0, len1);
            if (!is->muted && is->audio_buf)
                SDL_MixAudioFormat(stream, (uint8_t *)is->audio_buf + is->audio_buf_index, 0x8010, len1, is->audio_volume);
        }
        len -= len1;
        stream += len1;
        is->audio_buf_index += len1;
    }
    is->audio_write_buf_size = is->audio_buf_size - is->audio_buf_index;
    if (!
        __builtin_choose_expr ( __builtin_types_compatible_p (__typeof__ (
        is->audio_clock
        ), double), __isnan(
        is->audio_clock
        ), __builtin_choose_expr ( __builtin_types_compatible_p (__typeof__ (
        is->audio_clock
        ), float), __isnanf(
        is->audio_clock
        ), __builtin_choose_expr ( __builtin_types_compatible_p (__typeof__ (
        is->audio_clock
        ), long double), __isnanl(
        is->audio_clock
        ), (__builtin_trap(),
        is->audio_clock
        ))))
                              ) {
        set_clock_at(&is->audclk, is->audio_clock - (double)(2 * is->audio_hw_buf_size + is->audio_write_buf_size) / is->audio_tgt.bytes_per_sec, is->audio_clock_serial, audio_callback_time / 1000000.0);
        sync_clock_to_slave(&is->extclk, &is->audclk);
    }
}

static int audio_open(void *opaque, int64_t wanted_channel_layout, int wanted_nb_channels, int wanted_sample_rate, struct AudioParams *audio_hw_params)
{
    SDL_AudioSpec wanted_spec, spec;
    const char *env;
    static const int next_nb_channels[] = {0, 0, 1, 6, 2, 6, 4, 6};
    static const int next_sample_rates[] = {0, 44100, 48000, 96000, 192000};
    int next_sample_rate_idx = (sizeof(next_sample_rates) / sizeof((next_sample_rates)[0])) - 1;
    env = SDL_getenv("SDL_AUDIO_CHANNELS");
    if (env) {
        wanted_nb_channels = atoi(env);
        wanted_channel_layout = av_get_default_channel_layout(wanted_nb_channels);
    }
    if (!wanted_channel_layout || wanted_nb_channels != av_get_channel_layout_nb_channels(wanted_channel_layout)) {
        wanted_channel_layout = av_get_default_channel_layout(wanted_nb_channels);
        wanted_channel_layout &= ~(0x20000000|0x40000000);
    }
    wanted_nb_channels = av_get_channel_layout_nb_channels(wanted_channel_layout);
    wanted_spec.channels = wanted_nb_channels;
    wanted_spec.freq = wanted_sample_rate;
    if (wanted_spec.freq <= 0 || wanted_spec.channels <= 0) {
        av_log(
              ((void *)0)
                  , 16, "Invalid sample rate or channel count!\n");
        return -1;
    }
    while (next_sample_rate_idx && next_sample_rates[next_sample_rate_idx] >= wanted_spec.freq)
        next_sample_rate_idx--;
    wanted_spec.format = 0x8010;
    wanted_spec.silence = 0;
    wanted_spec.samples = ((512) > (2 << av_log2(wanted_spec.freq / 30)) ? (512) : (2 << av_log2(wanted_spec.freq / 30)));
    wanted_spec.callback = sdl_audio_callback;
    wanted_spec.userdata = opaque;
    while (!(audio_dev = SDL_OpenAudioDevice(
                                            ((void *)0)
                                                , 0, &wanted_spec, &spec, 0x00000001 | 0x00000004))) {
        av_log(
              ((void *)0)
                  , 24, "SDL_OpenAudio (%d channels, %d Hz): %s\n",
               wanted_spec.channels, wanted_spec.freq, SDL_GetError());
        wanted_spec.channels = next_nb_channels[((7) > (wanted_spec.channels) ? (wanted_spec.channels) : (7))];
        if (!wanted_spec.channels) {
            wanted_spec.freq = next_sample_rates[next_sample_rate_idx--];
            wanted_spec.channels = wanted_nb_channels;
            if (!wanted_spec.freq) {
                av_log(
                      ((void *)0)
                          , 16,
                       "No more combinations to try, audio open failed\n");
                return -1;
            }
        }
        wanted_channel_layout = av_get_default_channel_layout(wanted_spec.channels);
    }
    if (spec.format != 0x8010) {
        av_log(
              ((void *)0)
                  , 16,
               "SDL advised audio format %d is not supported!\n", spec.format);
        return -1;
    }
    if (spec.channels != wanted_spec.channels) {
        wanted_channel_layout = av_get_default_channel_layout(spec.channels);
        if (!wanted_channel_layout) {
            av_log(
                  ((void *)0)
                      , 16,
                   "SDL advised channel count %d is not supported!\n", spec.channels);
            return -1;
        }
    }
    audio_hw_params->fmt = AV_SAMPLE_FMT_S16;
    audio_hw_params->freq = spec.freq;
    audio_hw_params->channel_layout = wanted_channel_layout;
    audio_hw_params->channels = spec.channels;
    audio_hw_params->frame_size = av_samples_get_buffer_size(
                                                            ((void *)0)
                                                                , audio_hw_params->channels, 1, audio_hw_params->fmt, 1);
    audio_hw_params->bytes_per_sec = av_samples_get_buffer_size(
                                                               ((void *)0)
                                                                   , audio_hw_params->channels, audio_hw_params->freq, audio_hw_params->fmt, 1);
    if (audio_hw_params->bytes_per_sec <= 0 || audio_hw_params->frame_size <= 0) {
        av_log(
              ((void *)0)
                  , 16, "av_samples_get_buffer_size failed\n");
        return -1;
    }
    return spec.size;
}

static int stream_component_open(VideoState *is, int stream_index)
{
    AVFormatContext *ic = is->ic;
    AVCodecContext *avctx;
    AVCodec *codec;
    const char *forced_codec_name = 
                                   ((void *)0)
                                       ;
    AVDictionary *opts = 
                        ((void *)0)
                            ;
    AVDictionaryEntry *t = 
                          ((void *)0)
                              ;
    int sample_rate, nb_channels;
    int64_t channel_layout;
    int ret = 0;
    int stream_lowres = lowres;
    if (stream_index < 0 || stream_index >= ic->nb_streams)
        return -1;
    avctx = avcodec_alloc_context3(
                                  ((void *)0)
                                      );
    if (!avctx)
        return (-(
              12
              ));
    ret = avcodec_parameters_to_context(avctx, ic->streams[stream_index]->codecpar);
    if (ret < 0)
        goto fail;
    avctx->pkt_timebase = ic->streams[stream_index]->time_base;
    codec = avcodec_find_decoder(avctx->codec_id);
    switch(avctx->codec_type){
        case AVMEDIA_TYPE_AUDIO : is->last_audio_stream = stream_index; forced_codec_name = audio_codec_name; break;
        case AVMEDIA_TYPE_SUBTITLE: is->last_subtitle_stream = stream_index; forced_codec_name = subtitle_codec_name; break;
        case AVMEDIA_TYPE_VIDEO : is->last_video_stream = stream_index; forced_codec_name = video_codec_name; break;
    }
    if (forced_codec_name)
        codec = avcodec_find_decoder_by_name(forced_codec_name);
    if (!codec) {
        if (forced_codec_name) av_log(
                                     ((void *)0)
                                         , 24,
                                      "No codec could be found with name '%s'\n", forced_codec_name);
        else av_log(
                                     ((void *)0)
                                         , 24,
                                      "No decoder could be found for codec %s\n", avcodec_get_name(avctx->codec_id));
        ret = (-(
             22
             ));
        goto fail;
    }
    avctx->codec_id = codec->id;
    if (stream_lowres > codec->max_lowres) {
        av_log(avctx, 24, "The maximum value for lowres supported by the decoder is %d\n",
                codec->max_lowres);
        stream_lowres = codec->max_lowres;
    }
    avctx->lowres = stream_lowres;
    if (fast)
        avctx->flags2 |= (1 << 0);
    opts = filter_codec_opts(codec_opts, avctx->codec_id, ic, ic->streams[stream_index], codec);
    if (!av_dict_get(opts, "threads", 
                                     ((void *)0)
                                         , 0))
        av_dict_set(&opts, "threads", "auto", 0);
    if (stream_lowres)
        av_dict_set_int(&opts, "lowres", stream_lowres, 0);
    if (avctx->codec_type == AVMEDIA_TYPE_VIDEO || avctx->codec_type == AVMEDIA_TYPE_AUDIO)
        av_dict_set(&opts, "refcounted_frames", "1", 0);
    if ((ret = avcodec_open2(avctx, codec, &opts)) < 0) {
        goto fail;
    }
    if ((t = av_dict_get(opts, "", 
                                  ((void *)0)
                                      , 2))) {
        av_log(
              ((void *)0)
                  , 16, "Option %s not found.\n", t->key);
        ret = (-(int)((0xF8) | (('O') << 8) | (('P') << 16) | ((unsigned)('T') << 24)));
        goto fail;
    }
    is->eof = 0;
    ic->streams[stream_index]->discard = AVDISCARD_DEFAULT;
    switch (avctx->codec_type) {
    case AVMEDIA_TYPE_AUDIO:
        {
            AVFilterContext *sink;
            is->audio_filter_src.freq = avctx->sample_rate;
            is->audio_filter_src.channels = avctx->channels;
            is->audio_filter_src.channel_layout = get_valid_channel_layout(avctx->channel_layout, avctx->channels);
            is->audio_filter_src.fmt = avctx->sample_fmt;
            if ((ret = configure_audio_filters(is, afilters, 0)) < 0)
                goto fail;
            sink = is->out_audio_filter;
            sample_rate = av_buffersink_get_sample_rate(sink);
            nb_channels = av_buffersink_get_channels(sink);
            channel_layout = av_buffersink_get_channel_layout(sink);
        }
        if ((ret = audio_open(is, channel_layout, nb_channels, sample_rate, &is->audio_tgt)) < 0)
            goto fail;
        is->audio_hw_buf_size = ret;
        is->audio_src = is->audio_tgt;
        is->audio_buf_size = 0;
        is->audio_buf_index = 0;
        is->audio_diff_avg_coef = exp(log(0.01) / 20);
        is->audio_diff_avg_count = 0;
        is->audio_diff_threshold = (double)(is->audio_hw_buf_size) / is->audio_tgt.bytes_per_sec;
        is->audio_stream = stream_index;
        is->audio_st = ic->streams[stream_index];
        decoder_init(&is->auddec, avctx, &is->audioq, is->continue_read_thread);
        if ((is->ic->iformat->flags & (0x2000 | 0x4000 | 0x8000)) && !is->ic->iformat->read_seek) {
            is->auddec.start_pts = is->audio_st->start_time;
            is->auddec.start_pts_tb = is->audio_st->time_base;
        }
        if ((ret = decoder_start(&is->auddec, audio_thread, "audio_decoder", is)) < 0)
            goto out;
        SDL_PauseAudioDevice(audio_dev, 0);
        break;
    case AVMEDIA_TYPE_VIDEO:
        is->video_stream = stream_index;
        is->video_st = ic->streams[stream_index];
        decoder_init(&is->viddec, avctx, &is->videoq, is->continue_read_thread);
        if ((ret = decoder_start(&is->viddec, video_thread, "video_decoder", is)) < 0)
            goto out;
        is->queue_attachments_req = 1;
        break;
    case AVMEDIA_TYPE_SUBTITLE:
        is->subtitle_stream = stream_index;
        is->subtitle_st = ic->streams[stream_index];
        decoder_init(&is->subdec, avctx, &is->subtitleq, is->continue_read_thread);
        if ((ret = decoder_start(&is->subdec, subtitle_thread, "subtitle_decoder", is)) < 0)
            goto out;
        break;
    default:
        break;
    }
    goto out;

fail:
    avcodec_free_context(&avctx);
out:
    av_dict_free(&opts);
    return ret;
}

static int decode_interrupt_cb(void *ctx)
{
    VideoState *is = ctx;
    return is->abort_request;
}

static int stream_has_enough_packets(AVStream *st, int stream_id, PacketQueue *queue) {
    return stream_id < 0 ||
           queue->abort_request ||
           (st->disposition & 0x0400) ||
           queue->nb_packets > 25 && (!queue->duration || av_q2d(st->time_base) * queue->duration > 1.0);
}

static int is_realtime(AVFormatContext *s)
{
    if( !strcmp(s->iformat->name, "rtp")
       || !strcmp(s->iformat->name, "rtsp")
       || !strcmp(s->iformat->name, "sdp")
    )
        return 1;
    if(s->pb && ( !strncmp(s->url, "rtp:", 4)
                 || !strncmp(s->url, "udp:", 4)
                )
    )
        return 1;
    return 0;
}

static int read_thread(void *arg)
{
    VideoState *is = arg;
    AVFormatContext *ic = 
                         ((void *)0)
                             ;
    int err, i, ret;
    int st_index[AVMEDIA_TYPE_NB];
    AVPacket pkt1, *pkt = &pkt1;
    int64_t stream_start_time;
    int pkt_in_play_range = 0;
    AVDictionaryEntry *t;
    SDL_mutex *wait_mutex = SDL_CreateMutex();
    int scan_all_pmts_set = 0;
    int64_t pkt_ts;
    if (!wait_mutex) {
        av_log(
              ((void *)0)
                  , 8, "SDL_CreateMutex(): %s\n", SDL_GetError());
        ret = (-(
             12
             ));
        goto fail;
    }
    memset(st_index, -1, sizeof(st_index));
    is->eof = 0;
    ic = avformat_alloc_context();
    if (!ic) {
        av_log(
              ((void *)0)
                  , 8, "Could not allocate context.\n");
        ret = (-(
             12
             ));
        goto fail;
    }
    ic->interrupt_callback.callback = decode_interrupt_cb;
    ic->interrupt_callback.opaque = is;
    if (!av_dict_get(format_opts, "scan_all_pmts", 
                                                  ((void *)0)
                                                      , 1)) {
        av_dict_set(&format_opts, "scan_all_pmts", "1", 16);
        scan_all_pmts_set = 1;
    }
    err = avformat_open_input(&ic, is->filename, is->iformat, &format_opts);
    if (err < 0) {
        print_error(is->filename, err);
        ret = -1;
        goto fail;
    }
    if (scan_all_pmts_set)
        av_dict_set(&format_opts, "scan_all_pmts", 
                                                  ((void *)0)
                                                      , 1);
    if ((t = av_dict_get(format_opts, "", 
                                         ((void *)0)
                                             , 2))) {
        av_log(
              ((void *)0)
                  , 16, "Option %s not found.\n", t->key);
        ret = (-(int)((0xF8) | (('O') << 8) | (('P') << 16) | ((unsigned)('T') << 24)));
        goto fail;
    }
    is->ic = ic;
    if (genpts)
        ic->flags |= 0x0001;
    av_format_inject_global_side_data(ic);
    if (find_stream_info) {
        AVDictionary **opts = setup_find_stream_info_opts(ic, codec_opts);
        int orig_nb_streams = ic->nb_streams;
        err = avformat_find_stream_info(ic, opts);
        for (i = 0; i < orig_nb_streams; i++)
            av_dict_free(&opts[i]);
        av_freep(&opts);
        if (err < 0) {
            av_log(
                  ((void *)0)
                      , 24,
                   "%s: could not find codec parameters\n", is->filename);
            ret = -1;
            goto fail;
        }
    }
    if (ic->pb)
        ic->pb->eof_reached = 0;
    if (seek_by_bytes < 0)
        seek_by_bytes = !!(ic->iformat->flags & 0x0200) && strcmp("ogg", ic->iformat->name);
    is->max_frame_duration = (ic->iformat->flags & 0x0200) ? 10.0 : 3600.0;
    if (!window_title && (t = av_dict_get(ic->metadata, "title", 
                                                                ((void *)0)
                                                                    , 0)))
        window_title = av_asprintf("%s - %s", t->value, input_filename);
    if (start_time != ((int64_t)0x8000000000000000ULL)) {
        int64_t timestamp;
        timestamp = start_time;
        if (ic->start_time != ((int64_t)0x8000000000000000ULL))
            timestamp += ic->start_time;
        ret = avformat_seek_file(ic, -1, 
                                        (-9223372036854775807LL - 1)
                                                 , timestamp, 
                                                              9223372036854775807LL
                                                                       , 0);
        if (ret < 0) {
            av_log(
                  ((void *)0)
                      , 24, "%s: could not seek to position %0.3f\n",
                    is->filename, (double)timestamp / 1000000);
        }
    }
    is->realtime = is_realtime(ic);
    if (show_status)
        av_dump_format(ic, 0, is->filename, 0);
    for (i = 0; i < ic->nb_streams; i++) {
        AVStream *st = ic->streams[i];
        enum AVMediaType type = st->codecpar->codec_type;
        st->discard = AVDISCARD_ALL;
        if (type >= 0 && wanted_stream_spec[type] && st_index[type] == -1)
            if (avformat_match_stream_specifier(ic, st, wanted_stream_spec[type]) > 0)
                st_index[type] = i;
    }
    for (i = 0; i < AVMEDIA_TYPE_NB; i++) {
        if (wanted_stream_spec[i] && st_index[i] == -1) {
            av_log(
                  ((void *)0)
                      , 16, "Stream specifier %s does not match any %s stream\n", wanted_stream_spec[i], av_get_media_type_string(i));
            st_index[i] = 0x7fffffff;
        }
    }
    if (!video_disable)
        st_index[AVMEDIA_TYPE_VIDEO] =
            av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO,
                                st_index[AVMEDIA_TYPE_VIDEO], -1, 
                                                                 ((void *)0)
                                                                     , 0);
    if (!audio_disable)
        st_index[AVMEDIA_TYPE_AUDIO] =
            av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO,
                                st_index[AVMEDIA_TYPE_AUDIO],
                                st_index[AVMEDIA_TYPE_VIDEO],
                                
                               ((void *)0)
                                   , 0);
    if (!video_disable && !subtitle_disable)
        st_index[AVMEDIA_TYPE_SUBTITLE] =
            av_find_best_stream(ic, AVMEDIA_TYPE_SUBTITLE,
                                st_index[AVMEDIA_TYPE_SUBTITLE],
                                (st_index[AVMEDIA_TYPE_AUDIO] >= 0 ?
                                 st_index[AVMEDIA_TYPE_AUDIO] :
                                 st_index[AVMEDIA_TYPE_VIDEO]),
                                
                               ((void *)0)
                                   , 0);
    is->show_mode = show_mode;
    if (st_index[AVMEDIA_TYPE_VIDEO] >= 0) {
        AVStream *st = ic->streams[st_index[AVMEDIA_TYPE_VIDEO]];
        AVCodecParameters *codecpar = st->codecpar;
        AVRational sar = av_guess_sample_aspect_ratio(ic, st, 
                                                             ((void *)0)
                                                                 );
        if (codecpar->width)
            set_default_window_size(codecpar->width, codecpar->height, sar);
    }
    if (st_index[AVMEDIA_TYPE_AUDIO] >= 0) {
        stream_component_open(is, st_index[AVMEDIA_TYPE_AUDIO]);
    }
    ret = -1;
    if (st_index[AVMEDIA_TYPE_VIDEO] >= 0) {
        ret = stream_component_open(is, st_index[AVMEDIA_TYPE_VIDEO]);
    }
    if (is->show_mode == SHOW_MODE_NONE)
        is->show_mode = ret >= 0 ? SHOW_MODE_VIDEO : SHOW_MODE_RDFT;
    if (st_index[AVMEDIA_TYPE_SUBTITLE] >= 0) {
        stream_component_open(is, st_index[AVMEDIA_TYPE_SUBTITLE]);
    }
    if (is->video_stream < 0 && is->audio_stream < 0) {
        av_log(
              ((void *)0)
                  , 8, "Failed to open file '%s' or configure filtergraph\n",
               is->filename);
        ret = -1;
        goto fail;
    }
    if (infinite_buffer < 0 && is->realtime)
        infinite_buffer = 1;
    for (;;) {
        if (is->abort_request)
            break;
        if (is->paused != is->last_paused) {
            is->last_paused = is->paused;
            if (is->paused)
                is->read_pause_return = av_read_pause(ic);
            else
                av_read_play(ic);
        }
        if (is->paused &&
                (!strcmp(ic->iformat->name, "rtsp") ||
                 (ic->pb && !strncmp(input_filename, "mmsh:", 5)))) {
            SDL_Delay(10);
            continue;
        }
        if (is->seek_req) {
            int64_t seek_target = is->seek_pos;
            int64_t seek_min = is->seek_rel > 0 ? seek_target - is->seek_rel + 2: 
                                                                                    (-9223372036854775807LL - 1)
                                                                                             ;
            int64_t seek_max = is->seek_rel < 0 ? seek_target - is->seek_rel - 2: 
                                                                                    9223372036854775807LL
                                                                                             ;
            ret = avformat_seek_file(is->ic, -1, seek_min, seek_target, seek_max, is->seek_flags);
            if (ret < 0) {
                av_log(
                      ((void *)0)
                          , 16,
                       "%s: error while seeking\n", is->ic->url);
            } else {
                if (is->audio_stream >= 0) {
                    packet_queue_flush(&is->audioq);
                    packet_queue_put(&is->audioq, &flush_pkt);
                }
                if (is->subtitle_stream >= 0) {
                    packet_queue_flush(&is->subtitleq);
                    packet_queue_put(&is->subtitleq, &flush_pkt);
                }
                if (is->video_stream >= 0) {
                    packet_queue_flush(&is->videoq);
                    packet_queue_put(&is->videoq, &flush_pkt);
                }
                if (is->seek_flags & 2) {
                   set_clock(&is->extclk, 
                                         __builtin_nanf("")
                                            , 0);
                } else {
                   set_clock(&is->extclk, seek_target / (double)1000000, 0);
                }
            }
            is->seek_req = 0;
            is->queue_attachments_req = 1;
            is->eof = 0;
            if (is->paused)
                step_to_next_frame(is);
        }
        if (is->queue_attachments_req) {
            if (is->video_st && is->video_st->disposition & 0x0400) {
                AVPacket copy;
                if ((ret = av_packet_ref(&copy, &is->video_st->attached_pic)) < 0)
                    goto fail;
                packet_queue_put(&is->videoq, &copy);
                packet_queue_put_nullpacket(&is->videoq, is->video_stream);
            }
            is->queue_attachments_req = 0;
        }
        if (infinite_buffer<1 &&
              (is->audioq.size + is->videoq.size + is->subtitleq.size > (15 * 1024 * 1024)
            || (stream_has_enough_packets(is->audio_st, is->audio_stream, &is->audioq) &&
                stream_has_enough_packets(is->video_st, is->video_stream, &is->videoq) &&
                stream_has_enough_packets(is->subtitle_st, is->subtitle_stream, &is->subtitleq)))) {
            SDL_LockMutex(wait_mutex);
            SDL_CondWaitTimeout(is->continue_read_thread, wait_mutex, 10);
            SDL_UnlockMutex(wait_mutex);
            continue;
        }
        if (!is->paused &&
            (!is->audio_st || (is->auddec.finished == is->audioq.serial && frame_queue_nb_remaining(&is->sampq) == 0)) &&
            (!is->video_st || (is->viddec.finished == is->videoq.serial && frame_queue_nb_remaining(&is->pictq) == 0))) {
            if (loop != 1 && (!loop || --loop)) {
                stream_seek(is, start_time != ((int64_t)0x8000000000000000ULL) ? start_time : 0, 0, 0);
            } else if (autoexit) {
                ret = (-(int)(('E') | (('O') << 8) | (('F') << 16) | ((unsigned)(' ') << 24)));
                goto fail;
            }
        }
        ret = av_read_frame(ic, pkt);
        if (ret < 0) {
            if ((ret == (-(int)(('E') | (('O') << 8) | (('F') << 16) | ((unsigned)(' ') << 24))) || avio_feof(ic->pb)) && !is->eof) {
                if (is->video_stream >= 0)
                    packet_queue_put_nullpacket(&is->videoq, is->video_stream);
                if (is->audio_stream >= 0)
                    packet_queue_put_nullpacket(&is->audioq, is->audio_stream);
                if (is->subtitle_stream >= 0)
                    packet_queue_put_nullpacket(&is->subtitleq, is->subtitle_stream);
                is->eof = 1;
            }
            if (ic->pb && ic->pb->error) {
                if (autoexit)
                    goto fail;
                else
                    break;
            }
            SDL_LockMutex(wait_mutex);
            SDL_CondWaitTimeout(is->continue_read_thread, wait_mutex, 10);
            SDL_UnlockMutex(wait_mutex);
            continue;
        } else {
            is->eof = 0;
        }
        stream_start_time = ic->streams[pkt->stream_index]->start_time;
        pkt_ts = pkt->pts == ((int64_t)0x8000000000000000ULL) ? pkt->dts : pkt->pts;
        pkt_in_play_range = duration == ((int64_t)0x8000000000000000ULL) ||
                (pkt_ts - (stream_start_time != ((int64_t)0x8000000000000000ULL) ? stream_start_time : 0)) *
                av_q2d(ic->streams[pkt->stream_index]->time_base) -
                (double)(start_time != ((int64_t)0x8000000000000000ULL) ? start_time : 0) / 1000000
                <= ((double)duration / 1000000);
        if (pkt->stream_index == is->audio_stream && pkt_in_play_range) {
            packet_queue_put(&is->audioq, pkt);
        } else if (pkt->stream_index == is->video_stream && pkt_in_play_range
                   && !(is->video_st->disposition & 0x0400)) {
            packet_queue_put(&is->videoq, pkt);
        } else if (pkt->stream_index == is->subtitle_stream && pkt_in_play_range) {
            packet_queue_put(&is->subtitleq, pkt);
        } else {
            av_packet_unref(pkt);
        }
    }
    ret = 0;
 fail:
    if (ic && !is->ic)
        avformat_close_input(&ic);
    if (ret != 0) {
        SDL_Event event;
        event.type = (SDL_USEREVENT + 2);
        event.user.data1 = is;
        SDL_PushEvent(&event);
    }
    SDL_DestroyMutex(wait_mutex);
    return 0;
}

static VideoState *stream_open(const char *filename, AVInputFormat *iformat)
{
    VideoState *is;
    is = av_mallocz(sizeof(VideoState));
    if (!is)
        return 
              ((void *)0)
                  ;
    is->last_video_stream = is->video_stream = -1;
    is->last_audio_stream = is->audio_stream = -1;
    is->last_subtitle_stream = is->subtitle_stream = -1;
    is->filename = av_strdup(filename);
    if (!is->filename)
        goto fail;
    is->iformat = iformat;
    is->ytop = 0;
    is->xleft = 0;
    if (frame_queue_init(&is->pictq, &is->videoq, 3, 1) < 0)
        goto fail;
    if (frame_queue_init(&is->subpq, &is->subtitleq, 16, 0) < 0)
        goto fail;
    if (frame_queue_init(&is->sampq, &is->audioq, 9, 1) < 0)
        goto fail;
    if (packet_queue_init(&is->videoq) < 0 ||
        packet_queue_init(&is->audioq) < 0 ||
        packet_queue_init(&is->subtitleq) < 0)
        goto fail;
    if (!(is->continue_read_thread = SDL_CreateCond())) {
        av_log(
              ((void *)0)
                  , 8, "SDL_CreateCond(): %s\n", SDL_GetError());
        goto fail;
    }
    init_clock(&is->vidclk, &is->videoq.serial);
    init_clock(&is->audclk, &is->audioq.serial);
    init_clock(&is->extclk, &is->extclk.serial);
    is->audio_clock_serial = -1;
    if (startup_volume < 0)
        av_log(
              ((void *)0)
                  , 24, "-volume=%d < 0, setting to 0\n", startup_volume);
    if (startup_volume > 100)
        av_log(
              ((void *)0)
                  , 24, "-volume=%d > 100, setting to 100\n", startup_volume);
    startup_volume = av_clip_c(startup_volume, 0, 100);
    startup_volume = av_clip_c(128 * startup_volume / 100, 0, 128);
    is->audio_volume = startup_volume;
    is->muted = 0;
    is->av_sync_type = av_sync_type;
    is->read_tid = SDL_CreateThread(read_thread, "read_thread", is);
    if (!is->read_tid) {
        av_log(
              ((void *)0)
                  , 8, "SDL_CreateThread(): %s\n", SDL_GetError());
fail:
        stream_close(is);
        return 
              ((void *)0)
                  ;
    }
    return is;
}

static void stream_cycle_channel(VideoState *is, int codec_type)
{
    AVFormatContext *ic = is->ic;
    int start_index, stream_index;
    int old_index;
    AVStream *st;
    AVProgram *p = 
                  ((void *)0)
                      ;
    int nb_streams = is->ic->nb_streams;
    if (codec_type == AVMEDIA_TYPE_VIDEO) {
        start_index = is->last_video_stream;
        old_index = is->video_stream;
    } else if (codec_type == AVMEDIA_TYPE_AUDIO) {
        start_index = is->last_audio_stream;
        old_index = is->audio_stream;
    } else {
        start_index = is->last_subtitle_stream;
        old_index = is->subtitle_stream;
    }
    stream_index = start_index;
    if (codec_type != AVMEDIA_TYPE_VIDEO && is->video_stream != -1) {
        p = av_find_program_from_stream(ic, 
                                           ((void *)0)
                                               , is->video_stream);
        if (p) {
            nb_streams = p->nb_stream_indexes;
            for (start_index = 0; start_index < nb_streams; start_index++)
                if (p->stream_index[start_index] == stream_index)
                    break;
            if (start_index == nb_streams)
                start_index = -1;
            stream_index = start_index;
        }
    }
    for (;;) {
        if (++stream_index >= nb_streams)
        {
            if (codec_type == AVMEDIA_TYPE_SUBTITLE)
            {
                stream_index = -1;
                is->last_subtitle_stream = -1;
                goto the_end;
            }
            if (start_index == -1)
                return;
            stream_index = 0;
        }
        if (stream_index == start_index)
            return;
        st = is->ic->streams[p ? p->stream_index[stream_index] : stream_index];
        if (st->codecpar->codec_type == codec_type) {
            switch (codec_type) {
            case AVMEDIA_TYPE_AUDIO:
                if (st->codecpar->sample_rate != 0 &&
                    st->codecpar->channels != 0)
                    goto the_end;
                break;
            case AVMEDIA_TYPE_VIDEO:
            case AVMEDIA_TYPE_SUBTITLE:
                goto the_end;
            default:
                break;
            }
        }
    }
 the_end:
    if (p && stream_index != -1)
        stream_index = p->stream_index[stream_index];
    av_log(
          ((void *)0)
              , 32, "Switch %s stream from #%d to #%d\n",
           av_get_media_type_string(codec_type),
           old_index,
           stream_index);
    stream_component_close(is, old_index);
    stream_component_open(is, stream_index);
}

static void toggle_full_screen(VideoState *is)
{
    is_full_screen = !is_full_screen;
    SDL_SetWindowFullscreen(window, is_full_screen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
}

static void toggle_audio_display(VideoState *is)
{
    int next = is->show_mode;
    do {
        next = (next + 1) % SHOW_MODE_NB;
    } while (next != is->show_mode && (next == SHOW_MODE_VIDEO && !is->video_st || next != SHOW_MODE_VIDEO && !is->audio_st));
    if (is->show_mode != next) {
        is->force_refresh = 1;
        is->show_mode = next;
    }
}

static void refresh_loop_wait_event(VideoState *is, SDL_Event *event) {
    double remaining_time = 0.0;
    SDL_PumpEvents();
    while (!SDL_PeepEvents(event, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT)) {
        if (!cursor_hidden && av_gettime_relative() - cursor_last_shown > 1000000) {
            SDL_ShowCursor(0);
            cursor_hidden = 1;
        }
        if (remaining_time > 0.0)
            av_usleep((int64_t)(remaining_time * 1000000.0));
        remaining_time = 0.01;
        if (is->show_mode != SHOW_MODE_NONE && (!is->paused || is->force_refresh))
            video_refresh(is, &remaining_time);
        SDL_PumpEvents();
    }
}

static void seek_chapter(VideoState *is, int incr)
{
    int64_t pos = get_master_clock(is) * 1000000;
    int i;
    if (!is->ic->nb_chapters)
        return;
    for (i = 0; i < is->ic->nb_chapters; i++) {
        AVChapter *ch = is->ic->chapters[i];
        if (av_compare_ts(pos, (AVRational){1, 1000000}, ch->start, ch->time_base) < 0) {
            i--;
            break;
        }
    }
    i += incr;
    i = ((i) > (0) ? (i) : (0));
    if (i >= is->ic->nb_chapters)
        return;
    av_log(
          ((void *)0)
              , 40, "Seeking to chapter %d.\n", i);
    stream_seek(is, av_rescale_q(is->ic->chapters[i]->start, is->ic->chapters[i]->time_base,
                                 (AVRational){1, 1000000}), 0, 0);
}

static void event_loop(VideoState *cur_stream)
{
    SDL_Event event;
    double incr, pos, frac;
    for (;;) {
        double x;
        refresh_loop_wait_event(cur_stream, &event);
        switch (event.type) {
        case SDL_KEYDOWN:
            if (exit_on_keydown || event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_q) {
                do_exit(cur_stream);
                break;
            }
            if (!cur_stream->width)
                continue;
            switch (event.key.keysym.sym) {
            case SDLK_f:
                toggle_full_screen(cur_stream);
                cur_stream->force_refresh = 1;
                break;
            case SDLK_p:
            case SDLK_SPACE:
                toggle_pause(cur_stream);
                break;
            case SDLK_m:
                toggle_mute(cur_stream);
                break;
            case SDLK_KP_MULTIPLY:
            case SDLK_0:
                update_volume(cur_stream, 1, (0.75));
                break;
            case SDLK_KP_DIVIDE:
            case SDLK_9:
                update_volume(cur_stream, -1, (0.75));
                break;
            case SDLK_s:
                step_to_next_frame(cur_stream);
                break;
            case SDLK_a:
                stream_cycle_channel(cur_stream, AVMEDIA_TYPE_AUDIO);
                break;
            case SDLK_v:
                stream_cycle_channel(cur_stream, AVMEDIA_TYPE_VIDEO);
                break;
            case SDLK_c:
                stream_cycle_channel(cur_stream, AVMEDIA_TYPE_VIDEO);
                stream_cycle_channel(cur_stream, AVMEDIA_TYPE_AUDIO);
                stream_cycle_channel(cur_stream, AVMEDIA_TYPE_SUBTITLE);
                break;
            case SDLK_t:
                stream_cycle_channel(cur_stream, AVMEDIA_TYPE_SUBTITLE);
                break;
            case SDLK_w:
                if (cur_stream->show_mode == SHOW_MODE_VIDEO && cur_stream->vfilter_idx < nb_vfilters - 1) {
                    if (++cur_stream->vfilter_idx >= nb_vfilters)
                        cur_stream->vfilter_idx = 0;
                } else {
                    cur_stream->vfilter_idx = 0;
                    toggle_audio_display(cur_stream);
                }
                break;
            case SDLK_PAGEUP:
                if (cur_stream->ic->nb_chapters <= 1) {
                    incr = 600.0;
                    goto do_seek;
                }
                seek_chapter(cur_stream, 1);
                break;
            case SDLK_PAGEDOWN:
                if (cur_stream->ic->nb_chapters <= 1) {
                    incr = -600.0;
                    goto do_seek;
                }
                seek_chapter(cur_stream, -1);
                break;
            case SDLK_LEFT:
                incr = seek_interval ? -seek_interval : -10.0;
                goto do_seek;
            case SDLK_RIGHT:
                incr = seek_interval ? seek_interval : 10.0;
                goto do_seek;
            case SDLK_UP:
                incr = 60.0;
                goto do_seek;
            case SDLK_DOWN:
                incr = -60.0;
            do_seek:
                    if (seek_by_bytes) {
                        pos = -1;
                        if (pos < 0 && cur_stream->video_stream >= 0)
                            pos = frame_queue_last_pos(&cur_stream->pictq);
                        if (pos < 0 && cur_stream->audio_stream >= 0)
                            pos = frame_queue_last_pos(&cur_stream->sampq);
                        if (pos < 0)
                            pos = avio_tell(cur_stream->ic->pb);
                        if (cur_stream->ic->bit_rate)
                            incr *= cur_stream->ic->bit_rate / 8.0;
                        else
                            incr *= 180000.0;
                        pos += incr;
                        stream_seek(cur_stream, pos, incr, 1);
                    } else {
                        pos = get_master_clock(cur_stream);
                        if (
                           __builtin_choose_expr ( __builtin_types_compatible_p (__typeof__ (
                           pos
                           ), double), __isnan(
                           pos
                           ), __builtin_choose_expr ( __builtin_types_compatible_p (__typeof__ (
                           pos
                           ), float), __isnanf(
                           pos
                           ), __builtin_choose_expr ( __builtin_types_compatible_p (__typeof__ (
                           pos
                           ), long double), __isnanl(
                           pos
                           ), (__builtin_trap(),
                           pos
                           ))))
                                     )
                            pos = (double)cur_stream->seek_pos / 1000000;
                        pos += incr;
                        if (cur_stream->ic->start_time != ((int64_t)0x8000000000000000ULL) && pos < cur_stream->ic->start_time / (double)1000000)
                            pos = cur_stream->ic->start_time / (double)1000000;
                        stream_seek(cur_stream, (int64_t)(pos * 1000000), (int64_t)(incr * 1000000), 0);
                    }
                break;
            default:
                break;
            }
            break;
        case SDL_MOUSEBUTTONDOWN:
            if (exit_on_mousedown) {
                do_exit(cur_stream);
                break;
            }
            if (event.button.button == 1) {
                static int64_t last_mouse_left_click = 0;
                if (av_gettime_relative() - last_mouse_left_click <= 500000) {
                    toggle_full_screen(cur_stream);
                    cur_stream->force_refresh = 1;
                    last_mouse_left_click = 0;
                } else {
                    last_mouse_left_click = av_gettime_relative();
                }
            }
        case SDL_MOUSEMOTION:
            if (cursor_hidden) {
                SDL_ShowCursor(1);
                cursor_hidden = 0;
            }
            cursor_last_shown = av_gettime_relative();
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.button != 3)
                    break;
                x = event.button.x;
            } else {
                if (!(event.motion.state & (1 << ((3)-1))))
                    break;
                x = event.motion.x;
            }
                if (seek_by_bytes || cur_stream->ic->duration <= 0) {
                    uint64_t size = avio_size(cur_stream->ic->pb);
                    stream_seek(cur_stream, size*x/cur_stream->width, 0, 1);
                } else {
                    int64_t ts;
                    int ns, hh, mm, ss;
                    int tns, thh, tmm, tss;
                    tns = cur_stream->ic->duration / 1000000LL;
                    thh = tns / 3600;
                    tmm = (tns % 3600) / 60;
                    tss = (tns % 60);
                    frac = x / cur_stream->width;
                    ns = frac * tns;
                    hh = ns / 3600;
                    mm = (ns % 3600) / 60;
                    ss = (ns % 60);
                    av_log(
                          ((void *)0)
                              , 32,
                           "Seek to %2.0f%% (%2d:%02d:%02d) of total duration (%2d:%02d:%02d)       \n", frac*100,
                            hh, mm, ss, thh, tmm, tss);
                    ts = frac * cur_stream->ic->duration;
                    if (cur_stream->ic->start_time != ((int64_t)0x8000000000000000ULL))
                        ts += cur_stream->ic->start_time;
                    stream_seek(cur_stream, ts, 0, 0);
                }
            break;
        case SDL_WINDOWEVENT:
            switch (event.window.event) {
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    screen_width = cur_stream->width = event.window.data1;
                    screen_height = cur_stream->height = event.window.data2;
                    if (cur_stream->vis_texture) {
                        SDL_DestroyTexture(cur_stream->vis_texture);
                        cur_stream->vis_texture = 
                                                 ((void *)0)
                                                     ;
                    }
                case SDL_WINDOWEVENT_EXPOSED:
                    cur_stream->force_refresh = 1;
            }
            break;
        case SDL_QUIT:
        case (SDL_USEREVENT + 2):
            do_exit(cur_stream);
            break;
        default:
            break;
        }
    }
}

static int opt_frame_size(void *optctx, const char *opt, const char *arg)
{
    av_log(
          ((void *)0)
              , 24, "Option -s is deprecated, use -video_size.\n");
    return opt_default(
                      ((void *)0)
                          , "video_size", arg);
}

static int opt_width(void *optctx, const char *opt, const char *arg)
{
    screen_width = parse_number_or_die(opt, arg, 0x0400, 1, 0x7fffffff);
    return 0;
}

static int opt_height(void *optctx, const char *opt, const char *arg)
{
    screen_height = parse_number_or_die(opt, arg, 0x0400, 1, 0x7fffffff);
    return 0;
}

static int opt_format(void *optctx, const char *opt, const char *arg)
{
    file_iformat = av_find_input_format(arg);
    if (!file_iformat) {
        av_log(
              ((void *)0)
                  , 8, "Unknown input format: %s\n", arg);
        return (-(
              22
              ));
    }
    return 0;
}

static int opt_frame_pix_fmt(void *optctx, const char *opt, const char *arg)
{
    av_log(
          ((void *)0)
              , 24, "Option -pix_fmt is deprecated, use -pixel_format.\n");
    return opt_default(
                      ((void *)0)
                          , "pixel_format", arg);
}

static int opt_sync(void *optctx, const char *opt, const char *arg)
{
    if (!strcmp(arg, "audio"))
        av_sync_type = AV_SYNC_AUDIO_MASTER;
    else if (!strcmp(arg, "video"))
        av_sync_type = AV_SYNC_VIDEO_MASTER;
    else if (!strcmp(arg, "ext"))
        av_sync_type = AV_SYNC_EXTERNAL_CLOCK;
    else {
        av_log(
              ((void *)0)
                  , 16, "Unknown value for %s: %s\n", opt, arg);
        exit(1);
    }
    return 0;
}

static int opt_seek(void *optctx, const char *opt, const char *arg)
{
    start_time = parse_time_or_die(opt, arg, 1);
    return 0;
}

static int opt_duration(void *optctx, const char *opt, const char *arg)
{
    duration = parse_time_or_die(opt, arg, 1);
    return 0;
}

static int opt_show_mode(void *optctx, const char *opt, const char *arg)
{
    show_mode = !strcmp(arg, "video") ? SHOW_MODE_VIDEO :
                !strcmp(arg, "waves") ? SHOW_MODE_WAVES :
                !strcmp(arg, "rdft" ) ? SHOW_MODE_RDFT :
                parse_number_or_die(opt, arg, 0x0080, 0, SHOW_MODE_NB-1);
    return 0;
}

static void opt_input_file(void *optctx, const char *filename)
{
    if (input_filename) {
        av_log(
              ((void *)0)
                  , 8,
               "Argument '%s' provided as input filename, but '%s' was already specified.\n",
                filename, input_filename);
        exit(1);
    }
    if (!strcmp(filename, "-"))
        filename = "pipe:";
    input_filename = filename;
}

static int opt_codec(void *optctx, const char *opt, const char *arg)
{
   const char *spec = strchr(opt, ':');
   if (!spec) {
       av_log(
             ((void *)0)
                 , 16,
              "No media specifier was specified in '%s' in option '%s'\n",
               arg, opt);
       return (-(
             22
             ));
   }
   spec++;
   switch (spec[0]) {
   case 'a' : audio_codec_name = arg; break;
   case 's' : subtitle_codec_name = arg; break;
   case 'v' : video_codec_name = arg; break;
   default:
       av_log(
             ((void *)0)
                 , 16,
              "Invalid media specifier '%s' in option '%s'\n", spec, opt);
       return (-(
             22
             ));
   }
   return 0;
}

static int dummy;

static const OptionDef options[] = {
    { "L", 0x0800, { .func_arg = show_license }, "show license" }, { "h", 0x0800, { .func_arg = show_help }, "show help", "topic" }, { "?", 0x0800, { .func_arg = show_help }, "show help", "topic" }, { "help", 0x0800, { .func_arg = show_help }, "show help", "topic" }, { "-help", 0x0800, { .func_arg = show_help }, "show help", "topic" }, { "version", 0x0800, { .func_arg = show_version }, "show version" }, { "buildconf", 0x0800, { .func_arg = show_buildconf }, "show build configuration" }, { "formats", 0x0800, { .func_arg = show_formats }, "show available formats" }, { "muxers", 0x0800, { .func_arg = show_muxers }, "show available muxers" }, { "demuxers", 0x0800, { .func_arg = show_demuxers }, "show available demuxers" }, { "devices", 0x0800, { .func_arg = show_devices }, "show available devices" }, { "codecs", 0x0800, { .func_arg = show_codecs }, "show available codecs" }, { "decoders", 0x0800, { .func_arg = show_decoders }, "show available decoders" }, { "encoders", 0x0800, { .func_arg = show_encoders }, "show available encoders" }, { "bsfs", 0x0800, { .func_arg = show_bsfs }, "show available bit stream filters" }, { "protocols", 0x0800, { .func_arg = show_protocols }, "show available protocols" }, { "filters", 0x0800, { .func_arg = show_filters }, "show available filters" }, { "pix_fmts", 0x0800, { .func_arg = show_pix_fmts }, "show available pixel formats" }, { "layouts", 0x0800, { .func_arg = show_layouts }, "show standard channel layouts" }, { "sample_fmts", 0x0800, { .func_arg = show_sample_fmts }, "show available audio sample formats" }, { "colors", 0x0800, { .func_arg = show_colors }, "show available color names" }, { "loglevel", 0x0001, { .func_arg = opt_loglevel }, "set logging level", "loglevel" }, { "v", 0x0001, { .func_arg = opt_loglevel }, "set logging level", "loglevel" }, { "report", 0, { .func_arg = opt_report }, "generate a report" }, { "max_alloc", 0x0001, { .func_arg = opt_max_alloc }, "set maximum size of a single allocated block", "bytes" }, { "cpuflags", 0x0001 | 0x0004, { .func_arg = opt_cpuflags }, "force specific cpu flags", "flags" }, { "hide_banner", 0x0002 | 0x0004, {&hide_banner}, "do not show program banner", "hide_banner" }, { "sources" , 0x0800 | 0x0001, { .func_arg = show_sources }, "list sources of the input device", "device" }, { "sinks" , 0x0800 | 0x0001, { .func_arg = show_sinks }, "list sinks of the output device", "device" },
    { "x", 0x0001, { .func_arg = opt_width }, "force displayed width", "width" },
    { "y", 0x0001, { .func_arg = opt_height }, "force displayed height", "height" },
    { "s", 0x0001 | 0x0010, { .func_arg = opt_frame_size }, "set frame size (WxH or abbreviation)", "size" },
    { "fs", 0x0002, { &is_full_screen }, "force full screen" },
    { "an", 0x0002, { &audio_disable }, "disable audio" },
    { "vn", 0x0002, { &video_disable }, "disable video" },
    { "sn", 0x0002, { &subtitle_disable }, "disable subtitling" },
    { "ast", 0x0008 | 0x0001 | 0x0004, { &wanted_stream_spec[AVMEDIA_TYPE_AUDIO] }, "select desired audio stream", "stream_specifier" },
    { "vst", 0x0008 | 0x0001 | 0x0004, { &wanted_stream_spec[AVMEDIA_TYPE_VIDEO] }, "select desired video stream", "stream_specifier" },
    { "sst", 0x0008 | 0x0001 | 0x0004, { &wanted_stream_spec[AVMEDIA_TYPE_SUBTITLE] }, "select desired subtitle stream", "stream_specifier" },
    { "ss", 0x0001, { .func_arg = opt_seek }, "seek to a given position in seconds", "pos" },
    { "t", 0x0001, { .func_arg = opt_duration }, "play  \"duration\" seconds of audio/video", "duration" },
    { "bytes", 0x0080 | 0x0001, { &seek_by_bytes }, "seek by bytes 0=off 1=on -1=auto", "val" },
    { "seek_interval", 0x0100 | 0x0001, { &seek_interval }, "set seek interval for left/right keys, in seconds", "seconds" },
    { "nodisp", 0x0002, { &display_disable }, "disable graphical display" },
    { "noborder", 0x0002, { &borderless }, "borderless window" },
    { "alwaysontop", 0x0002, { &alwaysontop }, "window always on top" },
    { "volume", 0x0080 | 0x0001, { &startup_volume}, "set startup volume 0=min 100=max", "volume" },
    { "f", 0x0001, { .func_arg = opt_format }, "force format", "fmt" },
    { "pix_fmt", 0x0001 | 0x0004 | 0x0010, { .func_arg = opt_frame_pix_fmt }, "set pixel format", "format" },
    { "stats", 0x0002 | 0x0004, { &show_status }, "show status", "" },
    { "fast", 0x0002 | 0x0004, { &fast }, "non spec compliant optimizations", "" },
    { "genpts", 0x0002 | 0x0004, { &genpts }, "generate pts", "" },
    { "drp", 0x0080 | 0x0001 | 0x0004, { &decoder_reorder_pts }, "let decoder reorder pts 0=off 1=on -1=auto", ""},
    { "lowres", 0x0080 | 0x0001 | 0x0004, { &lowres }, "", "" },
    { "sync", 0x0001 | 0x0004, { .func_arg = opt_sync }, "set audio-video sync. type (type=audio/video/ext)", "type" },
    { "autoexit", 0x0002 | 0x0004, { &autoexit }, "exit at the end", "" },
    { "exitonkeydown", 0x0002 | 0x0004, { &exit_on_keydown }, "exit on key down", "" },
    { "exitonmousedown", 0x0002 | 0x0004, { &exit_on_mousedown }, "exit on mouse down", "" },
    { "loop", 0x0080 | 0x0001 | 0x0004, { &loop }, "set number of times the playback shall be looped", "loop count" },
    { "framedrop", 0x0002 | 0x0004, { &framedrop }, "drop frames when cpu is too slow", "" },
    { "infbuf", 0x0002 | 0x0004, { &infinite_buffer }, "don't limit the input buffer size (useful with realtime streams)", "" },
    { "window_title", 0x0008 | 0x0001, { &window_title }, "set window title", "window title" },
    { "left", 0x0080 | 0x0001 | 0x0004, { &screen_left }, "set the x position for the left of the window", "x pos" },
    { "top", 0x0080 | 0x0001 | 0x0004, { &screen_top }, "set the y position for the top of the window", "y pos" },
    { "vf", 0x0004 | 0x0001, { .func_arg = opt_add_vfilter }, "set video filters", "filter_graph" },
    { "af", 0x0008 | 0x0001, { &afilters }, "set audio filters", "filter_graph" },
    { "rdftspeed", 0x0080 | 0x0001| 0x0020 | 0x0004, { &rdftspeed }, "rdft speed", "msecs" },
    { "showmode", 0x0001, { .func_arg = opt_show_mode}, "select show mode (0 = video, 1 = waves, 2 = RDFT)", "mode" },
    { "default", 0x0001 | 0x0020 | 0x0010 | 0x0004, { .func_arg = opt_default }, "generic catch all option", "" },
    { "i", 0x0002, { &dummy}, "read specified file", "input_file"},
    { "codec", 0x0001, { .func_arg = opt_codec}, "force decoder", "decoder_name" },
    { "acodec", 0x0001 | 0x0008 | 0x0004, { &audio_codec_name }, "force audio decoder", "decoder_name" },
    { "scodec", 0x0001 | 0x0008 | 0x0004, { &subtitle_codec_name }, "force subtitle decoder", "decoder_name" },
    { "vcodec", 0x0001 | 0x0008 | 0x0004, { &video_codec_name }, "force video decoder", "decoder_name" },
    { "autorotate", 0x0002, { &autorotate }, "automatically rotate video", "" },
    { "find_stream_info", 0x0002 | 0x40000 | 0x0004, { &find_stream_info },
        "read and decode the streams to fill missing information with heuristics" },
    { "filter_threads", 0x0001 | 0x0080 | 0x0004, { &filter_nbthreads }, "number of filter threads per graph" },
    { 
     ((void *)0)
         , },
};

static void show_usage(void)
{
    av_log(
          ((void *)0)
              , 32, "Simple media player\n");
    av_log(
          ((void *)0)
              , 32, "usage: %s [options] input_file\n", program_name);
    av_log(
          ((void *)0)
              , 32, "\n");
}

void show_help_default(const char *opt, const char *arg)
{
    av_log_set_callback(log_callback_help);
    show_usage();
    show_help_options(options, "Main options:", 0, 0x0004, 0);
    show_help_options(options, "Advanced options:", 0x0004, 0, 0);
    printf("\n");
    show_help_children(avcodec_get_class(), 2);
    show_help_children(avformat_get_class(), 2);
    show_help_children(avfilter_get_class(), (1<<16));
    printf("\nWhile playing:\n"
           "q, ESC              quit\n"
           "f                   toggle full screen\n"
           "p, SPC              pause\n"
           "m                   toggle mute\n"
           "9, 0                decrease and increase volume respectively\n"
           "/, *                decrease and increase volume respectively\n"
           "a                   cycle audio channel in the current program\n"
           "v                   cycle video channel\n"
           "t                   cycle subtitle channel in the current program\n"
           "c                   cycle program\n"
           "w                   cycle video filters or show modes\n"
           "s                   activate frame-step mode\n"
           "left/right          seek backward/forward 10 seconds or to custom interval if -seek_interval is set\n"
           "down/up             seek backward/forward 1 minute\n"
           "page down/page up   seek backward/forward 10 minutes\n"
           "right mouse click   seek to percentage in file corresponding to fraction of width\n"
           "left double-click   toggle full screen\n"
           );
}

int main(int argc, char **argv)
{
    int flags;
    VideoState *is;
    init_dynload();
    av_log_set_flags(1);
    parse_loglevel(argc, argv, options);
    avdevice_register_all();
    avformat_network_init();
    init_opts();
    signal(
          2 
                 , sigterm_handler);
    signal(
          15
                 , sigterm_handler);
    show_banner(argc, argv, options);
    parse_options(
                 ((void *)0)
                     , argc, argv, options, opt_input_file);
    if (!input_filename) {
        show_usage();
        av_log(
              ((void *)0)
                  , 8, "An input file must be specified\n");
        av_log(
              ((void *)0)
                  , 8,
               "Use -h to get full help or, even better, run 'man %s'\n", program_name);
        exit(1);
    }
    if (display_disable) {
        video_disable = 1;
    }
    flags = 0x00000020u | 0x00000010u | 0x00000001u;
    if (audio_disable)
        flags &= ~0x00000010u;
    else {
        if (!SDL_getenv("SDL_AUDIO_ALSA_SET_BUFFER_SIZE"))
            SDL_setenv("SDL_AUDIO_ALSA_SET_BUFFER_SIZE","1", 1);
    }
    if (display_disable)
        flags &= ~0x00000020u;
    if (SDL_Init (flags)) {
        av_log(
              ((void *)0)
                  , 8, "Could not initialize SDL - %s\n", SDL_GetError());
        av_log(
              ((void *)0)
                  , 8, "(Did you set the DISPLAY variable?)\n");
        exit(1);
    }
    SDL_EventState(SDL_SYSWMEVENT, 0);
    SDL_EventState(SDL_USEREVENT, 0);
    av_init_packet(&flush_pkt);
    flush_pkt.data = (uint8_t *)&flush_pkt;
    if (!display_disable) {
        int flags = SDL_WINDOW_HIDDEN;
        if (alwaysontop)
            flags |= SDL_WINDOW_ALWAYS_ON_TOP;
        if (borderless)
            flags |= SDL_WINDOW_BORDERLESS;
        else
            flags |= SDL_WINDOW_RESIZABLE;
        window = SDL_CreateWindow(program_name, (0x1FFF0000u|(0)), (0x1FFF0000u|(0)), default_width, default_height, flags);
        SDL_SetHint("SDL_RENDER_SCALE_QUALITY", "linear");
        if (window) {
            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
            if (!renderer) {
                av_log(
                      ((void *)0)
                          , 24, "Failed to initialize a hardware accelerated renderer: %s\n", SDL_GetError());
                renderer = SDL_CreateRenderer(window, -1, 0);
            }
            if (renderer) {
                if (!SDL_GetRendererInfo(renderer, &renderer_info))
                    av_log(
                          ((void *)0)
                              , 40, "Initialized %s renderer.\n", renderer_info.name);
            }
        }
        if (!window || !renderer || !renderer_info.num_texture_formats) {
            av_log(
                  ((void *)0)
                      , 8, "Failed to create window or renderer: %s", SDL_GetError());
            do_exit(
                   ((void *)0)
                       );
        }
    }
    is = stream_open(input_filename, file_iformat);
    if (!is) {
        av_log(
              ((void *)0)
                  , 8, "Failed to initialize VideoState!\n");
        do_exit(
               ((void *)0)
                   );
    }
    event_loop(is);
    return 0;
}
