#include "ffplay.h"

#if HAVE_MALLOC_H
#include <malloc.h>
#endif

#ifdef MALLOC_PREFIX

#define malloc         AV_JOIN(MALLOC_PREFIX, malloc)
#define memalign       AV_JOIN(MALLOC_PREFIX, memalign)
#define posix_memalign AV_JOIN(MALLOC_PREFIX, posix_memalign)
#define realloc        AV_JOIN(MALLOC_PREFIX, realloc)
#define free           AV_JOIN(MALLOC_PREFIX, free)

void *malloc(size_t size);
void *memalign(size_t align, size_t size);
int   posix_memalign(void **ptr, size_t align, size_t size);
void *realloc(void *ptr, size_t size);
void  free(void *ptr);

#endif /* MALLOC_PREFIX */



#define ALIGN (HAVE_AVX512 ? 64 : (HAVE_AVX ? 32 : 16))

/* NOTE: if you want to override these functions with your own
 * implementations (not recommended) you have to link libav* as
 * dynamic libraries and remove -Wl,-Bsymbolic from the linker flags.
 * Note that this will cost performance. */

static size_t max_alloc_size= INT_MAX;

void av_max_alloc(size_t max){
    max_alloc_size = max;
}

void *av_malloc(size_t size)
{
    void *ptr = NULL;

    if (size > max_alloc_size)
        return NULL;

#if HAVE_POSIX_MEMALIGN
    if (size) //OS X on SDK 10.6 has a broken posix_memalign implementation
    if (posix_memalign(&ptr, ALIGN, size))
        ptr = NULL;
#elif HAVE_ALIGNED_MALLOC
    ptr = _aligned_malloc(size, ALIGN);
#elif HAVE_MEMALIGN
#ifndef __DJGPP__
    ptr = memalign(ALIGN, size);
#else
    ptr = memalign(size, ALIGN);
#endif
    /* Why 64?
     * Indeed, we should align it:
     *   on  4 for 386
     *   on 16 for 486
     *   on 32 for 586, PPro - K6-III
     *   on 64 for K7 (maybe for P3 too).
     * Because L1 and L2 caches are aligned on those values.
     * But I don't want to code such logic here!
     */
    /* Why 32?
     * For AVX ASM. SSE / NEON needs only 16.
     * Why not larger? Because I did not see a difference in benchmarks ...
     */
    /* benchmarks with P3
     * memalign(64) + 1          3071, 3051, 3032
     * memalign(64) + 2          3051, 3032, 3041
     * memalign(64) + 4          2911, 2896, 2915
     * memalign(64) + 8          2545, 2554, 2550
     * memalign(64) + 16         2543, 2572, 2563
     * memalign(64) + 32         2546, 2545, 2571
     * memalign(64) + 64         2570, 2533, 2558
     *
     * BTW, malloc seems to do 8-byte alignment by default here.
     */
#else
    ptr = malloc(size);
#endif
    if(!ptr && !size) {
        size = 1;
        ptr= av_malloc(1);
    }
#if CONFIG_MEMORY_POISONING
    if (ptr)
        memset(ptr, FF_MEMORY_POISON, size);
#endif
    return ptr;
}

void *av_realloc(void *ptr, size_t size)
{
    if (size > max_alloc_size)
        return NULL;

#if HAVE_ALIGNED_MALLOC
    return _aligned_realloc(ptr, size + !size, ALIGN);
#else
    return realloc(ptr, size + !size);
#endif
}

static inline int av_size_mult(size_t a, size_t b, size_t *r)
{
    size_t t = a * b;
    /* Hack inspired from glibc: don't try the division if nelem and elsize
     * are both less than sqrt(SIZE_MAX). */
    if ((a | b) >= ((size_t)1 << (sizeof(size_t) * 4)) && a && t / a != b)
        return AVERROR(EINVAL);
    *r = t;
    return 0;
}

void av_free(void *ptr)
{
#if HAVE_ALIGNED_MALLOC
    _aligned_free(ptr);
#else
    free(ptr);
#endif
}

void *av_realloc_f(void *ptr, size_t nelem, size_t elsize)
{
    size_t size;
    void *r;

    if (av_size_mult(elsize, nelem, &size)) {
        av_free(ptr);
        return NULL;
    }
    r = av_realloc(ptr, size);
    if (!r)
        av_free(ptr);
    return r;
}

void av_freep(void *arg)
{
    void *val;

    memcpy(&val, arg, sizeof(val));
    memcpy(arg, &(void *){ NULL }, sizeof(val));
    av_free(val);
}

int av_reallocp(void *ptr, size_t size)
{
    void *val;

    if (!size) {
        av_freep(ptr);
        return 0;
    }

    memcpy(&val, ptr, sizeof(val));
    val = av_realloc(val, size);

    if (!val) {
        av_freep(ptr);
        return AVERROR(ENOMEM);
    }

    memcpy(ptr, &val, sizeof(val));
    return 0;
}

void *av_malloc_array(size_t nmemb, size_t size)
{
    size_t result;
    if (av_size_mult(nmemb, size, &result) < 0)
        return NULL;
    return av_malloc(result);
}

void *av_mallocz(size_t size)
{
    void *ptr = av_malloc(size);
    if (ptr)
        memset(ptr, 0, size);
    return ptr;
}

void *av_mallocz_array(size_t nmemb, size_t size)
{
    size_t result;
    if (av_size_mult(nmemb, size, &result) < 0)
        return NULL;
    return av_mallocz(result);
}

void *av_realloc_array(void *ptr, size_t nmemb, size_t size)
{
    size_t result;
    if (av_size_mult(nmemb, size, &result) < 0)
        return NULL;
    return av_realloc(ptr, result);
}

int av_reallocp_array(void *ptr, size_t nmemb, size_t size)
{
    void *val;

    memcpy(&val, ptr, sizeof(val));
    val = av_realloc_f(val, nmemb, size);
    memcpy(ptr, &val, sizeof(val));
    if (!val && nmemb && size)
        return AVERROR(ENOMEM);

    return 0;
}







void *av_calloc(size_t nmemb, size_t size)
{
    size_t result;
    if (av_size_mult(nmemb, size, &result) < 0)
        return NULL;
    return av_mallocz(result);
}

char *av_strdup(const char *s)
{
    char *ptr = NULL;
    if (s) {
        size_t len = strlen(s) + 1;
        ptr = av_realloc(NULL, len);
        if (ptr)
            memcpy(ptr, s, len);
    }
    return ptr;
}

char *av_strndup(const char *s, size_t len)
{
    char *ret = NULL, *end;

    if (!s)
        return NULL;

    end = memchr(s, 0, len);
    if (end)
        len = end - s;

    ret = av_realloc(NULL, len + 1);
    if (!ret)
        return NULL;

    memcpy(ret, s, len);
    ret[len] = 0;
    return ret;
}

void *av_memdup(const void *p, size_t size)
{
    void *ptr = NULL;
    if (p) {
        ptr = av_malloc(size);
        if (ptr)
            memcpy(ptr, p, size);
    }
    return ptr;
}

int av_dynarray_add_nofree(void *tab_ptr, int *nb_ptr, void *elem)
{
    void **tab;
    memcpy(&tab, tab_ptr, sizeof(tab));

    FF_DYNARRAY_ADD(INT_MAX, sizeof(*tab), tab, *nb_ptr, {
        tab[*nb_ptr] = elem;
        memcpy(tab_ptr, &tab, sizeof(tab));
    }, {
        return AVERROR(ENOMEM);
    });
    return 0;
}

void av_dynarray_add(void *tab_ptr, int *nb_ptr, void *elem)
{
    void **tab;
    memcpy(&tab, tab_ptr, sizeof(tab));

    FF_DYNARRAY_ADD(INT_MAX, sizeof(*tab), tab, *nb_ptr, {
        tab[*nb_ptr] = elem;
        memcpy(tab_ptr, &tab, sizeof(tab));
    }, {
        *nb_ptr = 0;
        av_freep(tab_ptr);
    });
}

void *av_dynarray2_add(void **tab_ptr, int *nb_ptr, size_t elem_size,
                       const uint8_t *elem_data)
{
    uint8_t *tab_elem_data = NULL;

    FF_DYNARRAY_ADD(INT_MAX, elem_size, *tab_ptr, *nb_ptr, {
        tab_elem_data = (uint8_t *)*tab_ptr + (*nb_ptr) * elem_size;
        if (elem_data)
            memcpy(tab_elem_data, elem_data, elem_size);
        else if (CONFIG_MEMORY_POISONING)
            memset(tab_elem_data, FF_MEMORY_POISON, elem_size);
    }, {
        av_freep(tab_ptr);
        *nb_ptr = 0;
    });
    return tab_elem_data;
}

static void fill16(uint8_t *dst, int len)
{
    uint32_t v = AV_RN16(dst - 2);

    v |= v << 16;

    while (len >= 4) {
        AV_WN32(dst, v);
        dst += 4;
        len -= 4;
    }

    while (len--) {
        *dst = dst[-2];
        dst++;
    }
}

static void fill24(uint8_t *dst, int len)
{
#if HAVE_BIGENDIAN
    uint32_t v = AV_RB24(dst - 3);
    uint32_t a = v << 8  | v >> 16;
    uint32_t b = v << 16 | v >> 8;
    uint32_t c = v << 24 | v;
#else
    uint32_t v = AV_RL24(dst - 3);
    uint32_t a = v       | v << 24;
    uint32_t b = v >> 8  | v << 16;
    uint32_t c = v >> 16 | v << 8;
#endif

    while (len >= 12) {
        AV_WN32(dst,     a);
        AV_WN32(dst + 4, b);
        AV_WN32(dst + 8, c);
        dst += 12;
        len -= 12;
    }

    if (len >= 4) {
        AV_WN32(dst, a);
        dst += 4;
        len -= 4;
    }

    if (len >= 4) {
        AV_WN32(dst, b);
        dst += 4;
        len -= 4;
    }

    while (len--) {
        *dst = dst[-3];
        dst++;
    }
}

static void fill32(uint8_t *dst, int len)
{
    uint32_t v = AV_RN32(dst - 4);

#if HAVE_FAST_64BIT
    uint64_t v2= v + ((uint64_t)v<<32);
    while (len >= 32) {
        AV_WN64(dst   , v2);
        AV_WN64(dst+ 8, v2);
        AV_WN64(dst+16, v2);
        AV_WN64(dst+24, v2);
        dst += 32;
        len -= 32;
    }
#endif

    while (len >= 4) {
        AV_WN32(dst, v);
        dst += 4;
        len -= 4;
    }

    while (len--) {
        *dst = dst[-4];
        dst++;
    }
}

#define AV_COPYU(n, d, s) AV_WN##n(d, AV_RN##n(s));
#define AV_COPY16U(d, s) AV_COPYU(16, d, s)
#define AV_COPY32U(d, s) AV_COPYU(32, d, s)
#define AV_COPY64U(d, s) AV_COPYU(64, d, s)
#   define AV_COPY128U(d, s)                                    \
    do {                                                        \
        AV_COPY64U(d, s);                                       \
        AV_COPY64U((char *)(d) + 8, (const char *)(s) + 8);     \
    } while(0)

void av_memcpy_backptr(uint8_t *dst, int back, int cnt)
{
    const uint8_t *src = &dst[-back];
    if (!back)
        return;

    if (back == 1) {
        memset(dst, *src, cnt);
    } else if (back == 2) {
        fill16(dst, cnt);
    } else if (back == 3) {
        fill24(dst, cnt);
    } else if (back == 4) {
        fill32(dst, cnt);
    } else {
        if (cnt >= 16) {
            int blocklen = back;
            while (cnt > blocklen) {
                memcpy(dst, src, blocklen);
                dst       += blocklen;
                cnt       -= blocklen;
                blocklen <<= 1;
            }
            memcpy(dst, src, cnt);
            return;
        }
        if (cnt >= 8) {
            AV_COPY32U(dst,     src);
            AV_COPY32U(dst + 4, src + 4);
            src += 8;
            dst += 8;
            cnt -= 8;
        }
        if (cnt >= 4) {
            AV_COPY32U(dst, src);
            src += 4;
            dst += 4;
            cnt -= 4;
        }
        if (cnt >= 2) {
            AV_COPY16U(dst, src);
            src += 2;
            dst += 2;
            cnt -= 2;
        }
        if (cnt)
            *dst = *src;
    }
}

void *av_fast_realloc(void *ptr, unsigned int *size, size_t min_size)
{
    if (min_size <= *size)
        return ptr;

    if (min_size > max_alloc_size) {
        *size = 0;
        return NULL;
    }

    min_size = FFMIN(max_alloc_size, FFMAX(min_size + min_size / 16 + 32, min_size));

    ptr = av_realloc(ptr, min_size);
    /* we could set this to the unmodified min_size but this is safer
     * if the user lost the ptr and uses NULL now
     */
    if (!ptr)
        min_size = 0;

    *size = min_size;

    return ptr;
}

void av_fast_malloc(void *ptr, unsigned int *size, size_t min_size)
{
    ff_fast_malloc(ptr, size, min_size, 0);
}

void av_fast_mallocz(void *ptr, unsigned int *size, size_t min_size)
{
    ff_fast_malloc(ptr, size, min_size, 1);
}


// #include "checkasm.c"
#include "libavutil_samplefmt.c"
#include "libavutil_log.c"

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



void show_help_options(const OptionDef *options, const char *msg, int req_flags,int rej_flags, int alt_flags);
void show_help_children(const AVClass *class, int flags);

void show_help_default(const char *opt, const char *arg);

int show_help(void *optctx, const char *opt, const char *arg);
void parse_options(void *optctx, int argc, char **argv, const OptionDef *options,
                   void (* parse_arg_function)(void *optctx, const char*));

int parse_option(void *optctx, const char *opt, const char *arg,
                 const OptionDef *options);


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
#define GROW_ARRAY(array,nb_elems) array = grow_array(array, sizeof(*array), &nb_elems, nb_elems + 1)
#define GET_PIX_FMT_NAME(pix_fmt) const char *name = av_get_pix_fmt_name(pix_fmt);
#define GET_CODEC_NAME(id) const char *name = avcodec_descriptor_get(id)->name;
#define GET_SAMPLE_FMT_NAME(sample_fmt) const char *name = av_get_sample_fmt_name(sample_fmt)
#define GET_SAMPLE_RATE_NAME(rate) char name[16]; snprintf(name, sizeof(name), "%d", rate);
#define GET_CH_LAYOUT_DESC(ch_layout) char name[128]; av_get_channel_layout_string(name, sizeof(name), 0, ch_layout);

double get_rotation(AVStream *st);



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


void *grow_array(void *array, int elem_size, int *size, int new_size)
{
    if (new_size >= 0x7fffffff / elem_size) {
        av_log(
              ((void *)0)
                  , 16, "Array too big.\n");
        exit_program(1);
    }
    if (*size < new_size) {
        uint8_t *tmp = av_realloc_array(array, new_size, elem_size);
        if (!tmp) {
            av_log(
                  ((void *)0)
                      , 16, "Could not alloc buffer.\n");
            exit_program(1);
        }
        memset(tmp + *size*elem_size, 0, (new_size-*size) * elem_size);
        *size = new_size;
        return tmp;
    }
    return array;
}
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


int av_get_channel_layout_nb_channels(uint64_t channel_layout)
{
    return av_popcount64_c(channel_layout);
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

void av_packet_free_side_data(AVPacket *pkt)
{
    int i;
    for (i = 0; i < pkt->side_data_elems; i++)
        av_freep(&pkt->side_data[i].data);
    av_freep(&pkt->side_data);
    pkt->side_data_elems = 0;
}


static void buffer_replace(AVBufferRef **dst, AVBufferRef **src)
{
    AVBuffer *b;

    b = (*dst)->buffer;

    if (src) {
        **dst = **src;
        av_freep(src);
    } else
        av_freep(dst);

    if (atomic_fetch_sub_explicit(&b->refcount, 1, memory_order_acq_rel) == 1) {
        b->free(b->opaque, b->data);
        av_freep(&b);
    }
}
void av_buffer_unref(AVBufferRef **buf)
{
    if (!buf || !*buf)
        return;

    buffer_replace(buf, NULL);
}

void av_init_packet(AVPacket *pkt)
{
    pkt->pts                  = AV_NOPTS_VALUE;
    pkt->dts                  = AV_NOPTS_VALUE;
    pkt->pos                  = -1;
    pkt->duration             = 0;
#if FF_API_CONVERGENCE_DURATION
FF_DISABLE_DEPRECATION_WARNINGS
    pkt->convergence_duration = 0;
FF_ENABLE_DEPRECATION_WARNINGS
#endif
    pkt->flags                = 0;
    pkt->stream_index         = 0;
    pkt->buf                  = NULL;
    pkt->side_data            = NULL;
    pkt->side_data_elems      = 0;
}


void av_packet_unref(AVPacket *pkt)
{
    av_packet_free_side_data(pkt);
    av_buffer_unref(&pkt->buf);
    av_init_packet(pkt);
    pkt->data = NULL;
    pkt->size = 0;
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
    d->decoder_tid = SDL_CreateThread(fn, thread_name, arg, (pfnSDL_CurrentBeginThread)_beginthreadex, (pfnSDL_CurrentEndThread)_endthreadex);
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
    is->read_tid = SDL_CreateThread(read_thread, "read_thread", is, (pfnSDL_CurrentBeginThread)_beginthreadex, (pfnSDL_CurrentEndThread)_endthreadex);
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
