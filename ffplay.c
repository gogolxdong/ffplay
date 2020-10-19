#include "ffplay.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// #include "demuxer_list.c"
// #include "muxer_list.c"
// #include "ac3dec_fixed.c"
// #include "adtsenc.c"
// #include "aviobuf.c"
// #include "bitstream.c"
// #include "codec_desc.c"
// #include "id3v2enc.c"
// #include "log.c"
// #include "mpeg4audio.c"
// #include "profiles.c"
// #include "samplefmt.c"
// #include "utils.c"
static size_t max_alloc_size = INT_MAX;

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

void av_dict_free(AVDictionary **pm)
{
    AVDictionary *m = *pm;

    if (m)
    {
        while (m->count--)
        {
            av_freep(&m->elems[m->count].key);
            av_freep(&m->elems[m->count].value);
        }
        av_freep(&m->elems);
    }
    av_freep(pm);
}

int av_buffer_is_writable(const AVBufferRef *buf)
{
    if (buf->buffer->flags & AV_BUFFER_FLAG_READONLY)
        return 0;

    return atomic_load(&buf->buffer->refcount) == 1;
}

int av_packet_add_side_data(AVPacket *pkt, enum AVPacketSideDataType type,
                            uint8_t *data, size_t size)
{
    AVPacketSideData *tmp;
    int i, elems = pkt->side_data_elems;

    for (i = 0; i < elems; i++)
    {
        AVPacketSideData *sd = &pkt->side_data[i];

        if (sd->type == type)
        {
            av_free(sd->data);
            sd->data = data;
            sd->size = size;
            return 0;
        }
    }

    if ((unsigned)elems + 1 > AV_PKT_DATA_NB)
        return AVERROR(ERANGE);

    tmp = av_realloc(pkt->side_data, (elems + 1) * sizeof(*tmp));
    if (!tmp)
        return AVERROR(ENOMEM);

    pkt->side_data = tmp;
    pkt->side_data[elems].data = data;
    pkt->side_data[elems].size = size;
    pkt->side_data[elems].type = type;
    pkt->side_data_elems++;

    return 0;
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

#else
    ptr = malloc(size);
#endif
    if (!ptr && !size)
    {
        size = 1;
        ptr = av_malloc(1);
    }
#if CONFIG_MEMORY_POISONING
    if (ptr)
        memset(ptr, FF_MEMORY_POISON, size);
#endif
    return ptr;
}

void *av_mallocz(size_t size)
{
    void *ptr = av_malloc(size);
    if (ptr)
        memset(ptr, 0, size);
    return ptr;
}

uint8_t *av_packet_new_side_data(AVPacket *pkt, enum AVPacketSideDataType type,
                                 int size)
{
    int ret;
    uint8_t *data;

    if ((unsigned)size > INT_MAX - AV_INPUT_BUFFER_PADDING_SIZE)
        return NULL;
    data = av_mallocz(size + AV_INPUT_BUFFER_PADDING_SIZE);
    if (!data)
        return NULL;

    ret = av_packet_add_side_data(pkt, type, data, size);
    if (ret < 0)
    {
        av_freep(&data);
        return NULL;
    }

    return data;
}

void av_init_packet(AVPacket *pkt)
{
    pkt->pts = AV_NOPTS_VALUE;
    pkt->dts = AV_NOPTS_VALUE;
    pkt->pos = -1;
    pkt->duration = 0;
#if FF_API_CONVERGENCE_DURATION
    FF_DISABLE_DEPRECATION_WARNINGS
    pkt->convergence_duration = 0;
    FF_ENABLE_DEPRECATION_WARNINGS
#endif
    pkt->flags = 0;
    pkt->stream_index = 0;
    pkt->buf = NULL;
    pkt->side_data = NULL;
    pkt->side_data_elems = 0;
}

int av_packet_copy_props(AVPacket *dst, const AVPacket *src)
{
    int i;

    dst->pts = src->pts;
    dst->dts = src->dts;
    dst->pos = src->pos;
    dst->duration = src->duration;
#if FF_API_CONVERGENCE_DURATION
    FF_DISABLE_DEPRECATION_WARNINGS
    dst->convergence_duration = src->convergence_duration;
    FF_ENABLE_DEPRECATION_WARNINGS
#endif
    dst->flags = src->flags;
    dst->stream_index = src->stream_index;

    dst->side_data = NULL;
    dst->side_data_elems = 0;
    for (i = 0; i < src->side_data_elems; i++)
    {
        enum AVPacketSideDataType type = src->side_data[i].type;
        int size = src->side_data[i].size;
        uint8_t *src_data = src->side_data[i].data;
        uint8_t *dst_data = av_packet_new_side_data(dst, type, size);

        if (!dst_data)
        {
            av_packet_free_side_data(dst);
            return AVERROR(ENOMEM);
        }
        memcpy(dst_data, src_data, size);
    }

    return 0;
}

void av_buffer_default_free(void *opaque, uint8_t *data)
{
    av_free(data);
}

AVBufferRef *av_buffer_create(uint8_t *data, int size,
                              void (*free)(void *opaque, uint8_t *data),
                              void *opaque, int flags)
{
    AVBufferRef *ref = NULL;
    AVBuffer *buf = NULL;

    buf = av_mallocz(sizeof(*buf));
    if (!buf)
        return NULL;

    buf->data = data;
    buf->size = size;
    buf->free = free ? free : av_buffer_default_free;
    buf->opaque = opaque;

    atomic_init(&buf->refcount, 1);

    buf->flags = flags;

    ref = av_mallocz(sizeof(*ref));
    if (!ref)
    {
        av_freep(&buf);
        return NULL;
    }

    ref->buffer = buf;
    ref->data = data;
    ref->size = size;

    return ref;
}

AVBufferRef *av_buffer_ref(AVBufferRef *buf)
{
    AVBufferRef *ret = av_mallocz(sizeof(*ret));

    if (!ret)
        return NULL;

    *ret = *buf;

    atomic_fetch_add_explicit(&buf->buffer->refcount, 1, memory_order_relaxed);

    return ret;
}

static void buffer_replace(AVBufferRef **dst, AVBufferRef **src)
{
    AVBuffer *b;

    b = (*dst)->buffer;

    if (src)
    {
        **dst = **src;
        av_freep(src);
    }
    else
        av_freep(dst);

    if (atomic_fetch_sub_explicit(&b->refcount, 1, memory_order_acq_rel) == 1)
    {
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

int av_buffer_realloc(AVBufferRef **pbuf, int size)
{
    AVBufferRef *buf = *pbuf;
    uint8_t *tmp;
    int ret;

    if (!buf)
    {
        /* allocate a new buffer with av_realloc(), so it will be reallocatable
         * later */
        uint8_t *data = av_realloc(NULL, size);
        if (!data)
            return AVERROR(ENOMEM);

        buf = av_buffer_create(data, size, av_buffer_default_free, NULL, 0);
        if (!buf)
        {
            av_freep(&data);
            return AVERROR(ENOMEM);
        }

        buf->buffer->flags_internal |= BUFFER_FLAG_REALLOCATABLE;
        *pbuf = buf;

        return 0;
    }
    else if (buf->size == size)
        return 0;

    if (!(buf->buffer->flags_internal & BUFFER_FLAG_REALLOCATABLE) ||
        !av_buffer_is_writable(buf) || buf->data != buf->buffer->data)
    {
        /* cannot realloc, allocate a new reallocable buffer and copy data */
        AVBufferRef *new = NULL;

        ret = av_buffer_realloc(&new, size);
        if (ret < 0)
            return ret;

        memcpy(new->data, buf->data, FFMIN(size, buf->size));

        buffer_replace(pbuf, &new);
        return 0;
    }

    tmp = av_realloc(buf->buffer->data, size);
    if (!tmp)
        return AVERROR(ENOMEM);

    buf->buffer->data = buf->data = tmp;
    buf->buffer->size = buf->size = size;
    return 0;
}

static inline int av_bprint_is_complete(const AVBPrint *buf)
{
    return buf->len < buf->size;
}


static int av_bprint_alloc(AVBPrint *buf, unsigned room)
{
    char *old_str, *new_str;
    unsigned min_size, new_size;

    if (buf->size == buf->size_max)
        return AVERROR(EIO);
    if (!av_bprint_is_complete(buf))
        return AVERROR_INVALIDDATA; /* it is already truncated anyway */
    min_size = buf->len + 1 + FFMIN(UINT_MAX - buf->len - 1, room);
    new_size = buf->size > buf->size_max / 2 ? buf->size_max : buf->size * 2;
    if (new_size < min_size)
        new_size = FFMIN(buf->size_max, min_size);
    old_str = av_bprint_is_allocated(buf) ? buf->str : NULL;
    new_str = av_realloc(old_str, new_size);
    if (!new_str)
        return AVERROR(ENOMEM);
    if (!old_str)
        memcpy(new_str, buf->str, buf->len + 1);
    buf->str = new_str;
    buf->size = new_size;
    return 0;
}

static void av_bprint_grow(AVBPrint *buf, unsigned extra_len)
{
    /* arbitrary margin to avoid small overflows */
    extra_len = FFMIN(extra_len, UINT_MAX - 5 - buf->len);
    buf->len += extra_len;
    if (buf->size)
        buf->str[FFMIN(buf->len, buf->size - 1)] = 0;
}

void av_bprint_init(AVBPrint *buf, unsigned size_init, unsigned size_max)
{
    unsigned size_auto = (char *)buf + sizeof(*buf) -
                         buf->reserved_internal_buffer;

    if (size_max == 1)
        size_max = size_auto;
    buf->str = buf->reserved_internal_buffer;
    buf->len = 0;
    buf->size = FFMIN(size_auto, size_max);
    buf->size_max = size_max;
    *buf->str = 0;
    if (size_init > buf->size)
        av_bprint_alloc(buf, size_init - 1);
}

void av_bprintf(AVBPrint *buf, const char *fmt, ...)
{
    unsigned room;
    char *dst;
    va_list vl;
    int extra_len;

    while (1)
    {
        room = av_bprint_room(buf);
        dst = room ? buf->str + buf->len : NULL;
        va_start(vl, fmt);
        extra_len = vsnprintf(dst, room, fmt, vl);
        va_end(vl);
        if (extra_len <= 0)
            return;
        if (extra_len < room)
            break;
        if (av_bprint_alloc(buf, extra_len))
            break;
    }
    av_bprint_grow(buf, extra_len);
}

void av_vbprintf(AVBPrint *buf, const char *fmt, va_list vl_arg)
{
    unsigned room;
    char *dst;
    int extra_len;
    va_list vl;

    while (1)
    {
        room = av_bprint_room(buf);
        dst = room ? buf->str + buf->len : NULL;
        va_copy(vl, vl_arg);
        extra_len = vsnprintf(dst, room, fmt, vl);
        va_end(vl);
        if (extra_len <= 0)
            return;
        if (extra_len < room)
            break;
        if (av_bprint_alloc(buf, extra_len))
            break;
    }
    av_bprint_grow(buf, extra_len);
}

static int get_category(void *ptr)
{
    AVClass *avc = *(AVClass **)ptr;
    if (!avc || (avc->version & 0xFF) < 100 || avc->version < (51 << 16 | 59 << 8) || avc->category >= AV_CLASS_CATEGORY_NB)
        return AV_CLASS_CATEGORY_NA + 16;

    if (avc->get_category)
        return avc->get_category(ptr) + 16;

    return avc->category + 16;
}

static const char *get_level_str(int level)
{
    switch (level)
    {
    case AV_LOG_QUIET:
        return "quiet";
    case AV_LOG_DEBUG:
        return "debug";
    case AV_LOG_TRACE:
        return "trace";
    case AV_LOG_VERBOSE:
        return "verbose";
    case AV_LOG_INFO:
        return "info";
    case AV_LOG_WARNING:
        return "warning";
    case AV_LOG_ERROR:
        return "error";
    case AV_LOG_FATAL:
        return "fatal";
    case AV_LOG_PANIC:
        return "panic";
    default:
        return "";
    }
}

static int av_log_level = AV_LOG_INFO;
static int flags;

static void format_line(void *avcl, int level, const char *fmt, va_list vl,
                        AVBPrint part[4], int *print_prefix, int type[2])
{
    AVClass *avc = avcl ? *(AVClass **)avcl : NULL;
    av_bprint_init(part + 0, 0, AV_BPRINT_SIZE_AUTOMATIC);
    av_bprint_init(part + 1, 0, AV_BPRINT_SIZE_AUTOMATIC);
    av_bprint_init(part + 2, 0, AV_BPRINT_SIZE_AUTOMATIC);
    av_bprint_init(part + 3, 0, 65536);

    if (type)
        type[0] = type[1] = AV_CLASS_CATEGORY_NA + 16;
    if (*print_prefix && avc)
    {
        if (avc->parent_log_context_offset)
        {
            AVClass **parent = *(AVClass ***)(((uint8_t *)avcl) +
                                              avc->parent_log_context_offset);
            if (parent && *parent)
            {
                av_bprintf(part + 0, "[%s @ %p] ",
                           (*parent)->item_name(parent), parent);
                if (type)
                    type[0] = get_category(parent);
            }
        }
        av_bprintf(part + 1, "[%s @ %p] ",
                   avc->item_name(avcl), avcl);
        if (type)
            type[1] = get_category(avcl);
    }

    if (*print_prefix && (level > AV_LOG_QUIET) && (flags & AV_LOG_PRINT_LEVEL))
        av_bprintf(part + 2, "[%s] ", get_level_str(level));

    av_vbprintf(part + 3, fmt, vl);

    if (*part[0].str || *part[1].str || *part[2].str || *part[3].str)
    {
        char lastc = part[3].len && part[3].len <= part[3].size ? part[3].str[part[3].len - 1] : 0;
        *print_prefix = lastc == '\n' || lastc == '\r';
    }
}

static int packet_alloc(AVBufferRef **buf, int size)
{
    int ret;
    if (size < 0 || size >= INT_MAX - AV_INPUT_BUFFER_PADDING_SIZE)
        return AVERROR(EINVAL);

    ret = av_buffer_realloc(buf, size + AV_INPUT_BUFFER_PADDING_SIZE);
    if (ret < 0)
        return ret;

    memset((*buf)->data + size, 0, AV_INPUT_BUFFER_PADDING_SIZE);
    return 0;
}

static inline int strict_pthread_mutex_lock(pthread_mutex_t *mutex)
{
    ASSERT_PTHREAD(pthread_mutex_lock, mutex);
}

void av_packet_unref(AVPacket *pkt)
{
    av_packet_free_side_data(pkt);
    av_buffer_unref(&pkt->buf);
    av_init_packet(pkt);
    pkt->data = NULL;
    pkt->size = 0;
}

int av_packet_ref(AVPacket *dst, const AVPacket *src)
{
    int ret;

    dst->buf = NULL;

    ret = av_packet_copy_props(dst, src);
    if (ret < 0)
        goto fail;

    if (!src->buf)
    {
        ret = packet_alloc(&dst->buf, src->size);
        if (ret < 0)
            goto fail;
        av_assert1(!src->size || src->data);
        if (src->size)
            memcpy(dst->buf->data, src->data, src->size);

        dst->data = dst->buf->data;
    }
    else
    {
        dst->buf = av_buffer_ref(src->buf);
        if (!dst->buf)
        {
            ret = AVERROR(ENOMEM);
            goto fail;
        }
        dst->data = src->data;
    }

    dst->size = src->size;

    return 0;
fail:
    av_packet_unref(dst);
    return ret;
}

void av_packet_move_ref(AVPacket *dst, AVPacket *src)
{
    *dst = *src;
    av_init_packet(src);
    src->data = NULL;
    src->size = 0;
}

static AVMutex mutex = AV_MUTEX_INITIALIZER;

static void sanitize(uint8_t *line)
{
    while (*line)
    {
        if (*line < 0x08 || (*line > 0x0D && *line < 0x20))
            *line = '?';
        line++;
    }
}

static int use_color = -1;
static int16_t background, attr_orig;
static HANDLE con;

static void check_color_terminal(void)
{
    char *term = getenv("TERM");

#if defined(_WIN32) && HAVE_SETCONSOLETEXTATTRIBUTE && HAVE_GETSTDHANDLE
    CONSOLE_SCREEN_BUFFER_INFO con_info;
    DWORD dummy;
    con = GetStdHandle(STD_ERROR_HANDLE);
    if (con != INVALID_HANDLE_VALUE && !GetConsoleMode(con, &dummy))
        con = INVALID_HANDLE_VALUE;
    if (con != INVALID_HANDLE_VALUE)
    {
        GetConsoleScreenBufferInfo(con, &con_info);
        attr_orig = con_info.wAttributes;
        background = attr_orig & 0xF0;
    }
#endif

    if (getenv("AV_LOG_FORCE_NOCOLOR"))
    {
        use_color = 0;
    }
    else if (getenv("AV_LOG_FORCE_COLOR"))
    {
        use_color = 1;
    }
    else
    {
#if defined(_WIN32) && HAVE_SETCONSOLETEXTATTRIBUTE && HAVE_GETSTDHANDLE
        use_color = (con != INVALID_HANDLE_VALUE);
#elif HAVE_ISATTY
        use_color = (term && isatty(2));
#else
        use_color = 0;
#endif
    }

    if (getenv("AV_LOG_FORCE_256COLOR") || term && strstr(term, "256color"))
        use_color *= 256;
}

static const uint8_t color[16 + AV_CLASS_CATEGORY_NB] = {
    [AV_LOG_PANIC  /8] = 12,
    [AV_LOG_FATAL  /8] = 12,
    [AV_LOG_ERROR  /8] = 12,
    [AV_LOG_WARNING/8] = 14,
    [AV_LOG_INFO   /8] =  7,
    [AV_LOG_VERBOSE/8] = 10,
    [AV_LOG_DEBUG  /8] = 10,
    [AV_LOG_TRACE  /8] = 8,
    [16+AV_CLASS_CATEGORY_NA              ] =  7,
    [16+AV_CLASS_CATEGORY_INPUT           ] = 13,
    [16+AV_CLASS_CATEGORY_OUTPUT          ] =  5,
    [16+AV_CLASS_CATEGORY_MUXER           ] = 13,
    [16+AV_CLASS_CATEGORY_DEMUXER         ] =  5,
    [16+AV_CLASS_CATEGORY_ENCODER         ] = 11,
    [16+AV_CLASS_CATEGORY_DECODER         ] =  3,
    [16+AV_CLASS_CATEGORY_FILTER          ] = 10,
    [16+AV_CLASS_CATEGORY_BITSTREAM_FILTER] =  9,
    [16+AV_CLASS_CATEGORY_SWSCALER        ] =  7,
    [16+AV_CLASS_CATEGORY_SWRESAMPLER     ] =  7,
    [16+AV_CLASS_CATEGORY_DEVICE_VIDEO_OUTPUT ] = 13,
    [16+AV_CLASS_CATEGORY_DEVICE_VIDEO_INPUT  ] = 5,
    [16+AV_CLASS_CATEGORY_DEVICE_AUDIO_OUTPUT ] = 13,
    [16+AV_CLASS_CATEGORY_DEVICE_AUDIO_INPUT  ] = 5,
    [16+AV_CLASS_CATEGORY_DEVICE_OUTPUT       ] = 13,
    [16+AV_CLASS_CATEGORY_DEVICE_INPUT        ] = 5,
};


#if defined(_WIN32) && HAVE_SETCONSOLETEXTATTRIBUTE && HAVE_GETSTDHANDLE
static void win_console_puts(const char *str)
{
    const uint8_t *q = str;
    uint16_t line[LINE_SZ];

    while (*q) {
        uint16_t *buf = line;
        DWORD nb_chars = 0;
        DWORD written;

        while (*q && nb_chars < LINE_SZ - 1) {
            uint32_t ch;
            uint16_t tmp;

            GET_UTF8(ch, *q ? *q++ : 0, ch = 0xfffd; goto continue_on_invalid;)
continue_on_invalid:
            PUT_UTF16(ch, tmp, *buf++ = tmp; nb_chars++;)
        }

        WriteConsoleW(con, line, nb_chars, &written, NULL);
    }
}
#endif

static void ansi_fputs(int level, int tint, const char *str, int local_use_color)
{
    if (local_use_color == 1) {
        fprintf(stderr,
                "\033[%"PRIu32";3%"PRIu32"m%s\033[0m",
                (color[level] >> 4) & 15,
                color[level] & 15,
                str);
    } else if (tint && use_color == 256) {
        fprintf(stderr,
                "\033[48;5;%"PRIu32"m\033[38;5;%dm%s\033[0m",
                (color[level] >> 16) & 0xff,
                tint,
                str);
    } else if (local_use_color == 256) {
        fprintf(stderr,
                "\033[48;5;%"PRIu32"m\033[38;5;%"PRIu32"m%s\033[0m",
                (color[level] >> 16) & 0xff,
                (color[level] >> 8) & 0xff,
                str);
    } else
        fputs(str, stderr);
}

static void colored_fputs(int level, int tint, const char *str)
{
    int local_use_color;
    if (!*str)
        return;

    if (use_color < 0)
        check_color_terminal();

    if (level == AV_LOG_INFO / 8)
        local_use_color = 0;
    else
        local_use_color = use_color;

#if defined(_WIN32) && HAVE_SETCONSOLETEXTATTRIBUTE && HAVE_GETSTDHANDLE
    if (con != INVALID_HANDLE_VALUE)
    {
        if (local_use_color)
            SetConsoleTextAttribute(con, background | color[level]);
        win_console_puts(str);
        if (local_use_color)
            SetConsoleTextAttribute(con, attr_orig);
    }
    else
    {
        ansi_fputs(level, tint, str, local_use_color);
    }
#else
    ansi_fputs(level, tint, str, local_use_color);
#endif
}

int av_bprint_finalize(AVBPrint *buf, char **ret_str)
{
    unsigned real_size = FFMIN(buf->len + 1, buf->size);
    char *str;
    int ret = 0;

    if (ret_str)
    {
        if (av_bprint_is_allocated(buf))
        {
            str = av_realloc(buf->str, real_size);
            if (!str)
                str = buf->str;
            buf->str = NULL;
        }
        else
        {
            str = av_malloc(real_size);
            if (str)
                memcpy(str, buf->str, real_size);
            else
                ret = AVERROR(ENOMEM);
        }
        *ret_str = str;
    }
    else
    {
        if (av_bprint_is_allocated(buf))
            av_freep(&buf->str);
    }
    buf->size = real_size;
    return ret;
}

void av_log_default_callback(void *ptr, int level, const char *fmt, va_list vl)
{
    static int print_prefix = 1;
    static int count;
    static char prev[LINE_SZ];
    AVBPrint part[4];
    char line[LINE_SZ];
    static int is_atty;
    int type[2];
    unsigned tint = 0;

    if (level >= 0)
    {
        tint = level & 0xff00;
        level &= 0xff;
    }

    if (level > av_log_level)
        return;
    ff_mutex_lock(&mutex);

    format_line(ptr, level, fmt, vl, part, &print_prefix, type);
    snprintf(line, sizeof(line), "%s%s%s%s", part[0].str, part[1].str, part[2].str, part[3].str);

#if HAVE_ISATTY
    if (!is_atty)
        is_atty = isatty(2) ? 1 : -1;
#endif

    if (print_prefix && (flags & AV_LOG_SKIP_REPEATED) && !strcmp(line, prev) &&
        *line && line[strlen(line) - 1] != '\r')
    {
        count++;
        if (is_atty == 1)
            fprintf(stderr, "    Last message repeated %d times\r", count);
        goto end;
    }
    if (count > 0)
    {
        fprintf(stderr, "    Last message repeated %d times\n", count);
        count = 0;
    }
    strcpy(prev, line);
    sanitize(part[0].str);
    colored_fputs(type[0], 0, part[0].str);
    sanitize(part[1].str);
    colored_fputs(type[1], 0, part[1].str);
    sanitize(part[2].str);
    colored_fputs(av_clip(level >> 3, 0, NB_LEVELS - 1), tint >> 8, part[2].str);
    sanitize(part[3].str);
    colored_fputs(av_clip(level >> 3, 0, NB_LEVELS - 1), tint >> 8, part[3].str);

#if CONFIG_VALGRIND_BACKTRACE
    if (level <= BACKTRACE_LOGLEVEL)
        VALGRIND_PRINTF_BACKTRACE("%s", "");
#endif
end:
    av_bprint_finalize(part + 3, NULL);
    ff_mutex_unlock(&mutex);
}

static void (*av_log_callback)(void *, int, const char *, va_list) =
    av_log_default_callback;

void av_vlog(void *avcl, int level, const char *fmt, va_list vl)
{
    AVClass *avc = avcl ? *(AVClass **)avcl : NULL;
    void (*log_callback)(void *, int, const char *, va_list) = av_log_callback;
    if (avc && avc->version >= (50 << 16 | 15 << 8 | 2) &&
        avc->log_level_offset_offset && level >= AV_LOG_FATAL)
        level += *(int *)(((uint8_t *)avcl) + avc->log_level_offset_offset);
    if (log_callback)
        log_callback(avcl, level, fmt, vl);
}

int av_log_get_level(void)
{
    return av_log_level;
}

void av_log_set_level(int level)
{
    av_log_level = level;
}

void av_log_set_flags(int arg)
{
    flags = arg;
}

int av_log_get_flags(void)
{
    return flags;
}

void av_log(void *avcl, int level, const char *fmt, ...)
{
    va_list vl;
    va_start(vl, fmt);
    av_vlog(avcl, level, fmt, vl);
    va_end(vl);
}

void av_free(void *ptr)
{
#if HAVE_ALIGNED_MALLOC
    _aligned_free(ptr);
#else
    free(ptr);
#endif
}

void av_freep(void *arg)
{
    void *val;

    memcpy(&val, arg, sizeof(val));
    memcpy(arg, &(void *){NULL}, sizeof(val));
    av_free(val);
}

void av_packet_free_side_data(AVPacket *pkt)
{
    int i;
    for (i = 0; i < pkt->side_data_elems; i++)
        av_freep(&pkt->side_data[i].data);
    av_freep(&pkt->side_data);
    pkt->side_data_elems = 0;
}

static int opt_add_vfilter(void *optctx, const char *opt, const char *arg)
{
    GROW_ARRAY(vfilters_list, nb_vfilters);
    vfilters_list[nb_vfilters - 1] = arg;
    return 0;
}

enum AVSampleFormat av_get_packed_sample_fmt(enum AVSampleFormat sample_fmt)
{
    if (sample_fmt < 0 || sample_fmt >= AV_SAMPLE_FMT_NB)
        return AV_SAMPLE_FMT_NONE;
    if (sample_fmt_info[sample_fmt].planar)
        return sample_fmt_info[sample_fmt].altform;
    return sample_fmt;
}

static inline int cmp_audio_fmts(enum AVSampleFormat fmt1, int64_t channel_count1,
                                 enum AVSampleFormat fmt2, int64_t channel_count2)
{
    /* If channel count == 1, planar and non-planar formats are the same */
    if (channel_count1 == 1 && channel_count2 == 1)
        return av_get_packed_sample_fmt(fmt1) != av_get_packed_sample_fmt(fmt2);
    else
        return channel_count1 != channel_count2 || fmt1 != fmt2;
}

static av_always_inline av_const int av_popcount64_c(uint64_t x)
{
    return av_popcount((uint32_t)x) + av_popcount((uint32_t)(x >> 32));
}

int av_get_channel_layout_nb_channels(uint64_t channel_layout)
{
    return av_popcount64(channel_layout);
}

static inline int64_t get_valid_channel_layout(int64_t channel_layout, int channels)
{
    if (channel_layout && av_get_channel_layout_nb_channels(channel_layout) == channels)
        return channel_layout;
    else
        return 0;
}

void av_max_alloc(size_t max)
{
    max_alloc_size = max;
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
    pkt1->next = NULL;
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
    pkt->data = NULL;
    pkt->size = 0;
    pkt->stream_index = stream_index;
    return packet_queue_put(q, pkt);
}

/* packet queue handling */
static int packet_queue_init(PacketQueue *q)
{
    memset(q, 0, sizeof(PacketQueue));
    q->mutex = SDL_CreateMutex();
    if (!q->mutex)
    {
        av_log(NULL, AV_LOG_FATAL, "SDL_CreateMutex(): %s\n", SDL_GetError());
        return AVERROR(ENOMEM);
    }
    q->cond = SDL_CreateCond();
    if (!q->cond)
    {
        av_log(NULL, AV_LOG_FATAL, "SDL_CreateCond(): %s\n", SDL_GetError());
        return AVERROR(ENOMEM);
    }
    q->abort_request = 1;
    return 0;
}

static void packet_queue_flush(PacketQueue *q)
{
    MyAVPacketList *pkt, *pkt1;

    SDL_LockMutex(q->mutex);
    for (pkt = q->first_pkt; pkt; pkt = pkt1)
    {
        pkt1 = pkt->next;
        av_packet_unref(&pkt->pkt);
        av_freep(&pkt);
    }
    q->last_pkt = NULL;
    q->first_pkt = NULL;
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

/* return < 0 if aborted, 0 if no packet and > 0 if packet.  */
static int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block, int *serial)
{
    MyAVPacketList *pkt1;
    int ret;

    SDL_LockMutex(q->mutex);

    for (;;)
    {
        if (q->abort_request)
        {
            ret = -1;
            break;
        }

        pkt1 = q->first_pkt;
        if (pkt1)
        {
            q->first_pkt = pkt1->next;
            if (!q->first_pkt)
                q->last_pkt = NULL;
            q->nb_packets--;
            q->size -= pkt1->pkt.size + sizeof(*pkt1);
            q->duration -= pkt1->pkt.duration;
            *pkt = pkt1->pkt;
            if (serial)
                *serial = pkt1->serial;
            av_free(pkt1);
            ret = 1;
            break;
        }
        else if (!block)
        {
            ret = 0;
            break;
        }
        else
        {
            SDL_CondWait(q->cond, q->mutex);
        }
    }
    SDL_UnlockMutex(q->mutex);
    return ret;
}

static void decoder_init(Decoder *d, AVCodecContext *avctx, PacketQueue *queue, SDL_cond *empty_queue_cond)
{
    memset(d, 0, sizeof(Decoder));
    d->avctx = avctx;
    d->queue = queue;
    d->empty_queue_cond = empty_queue_cond;
    d->start_pts = AV_NOPTS_VALUE;
    d->pkt_serial = -1;
}

static void free_side_data(AVFrameSideData **ptr_sd)
{
    AVFrameSideData *sd = *ptr_sd;

    av_buffer_unref(&sd->buf);
    av_dict_free(&sd->metadata);
    av_freep(ptr_sd);
}

static void wipe_side_data(AVFrame *frame)
{
    int i;

    for (i = 0; i < frame->nb_side_data; i++)
    {
        free_side_data(&frame->side_data[i]);
    }
    frame->nb_side_data = 0;

    av_freep(&frame->side_data);
}

static void get_frame_defaults(AVFrame *frame)
{
    if (frame->extended_data != frame->data)
        av_freep(&frame->extended_data);

    memset(frame, 0, sizeof(*frame));

    frame->pts =
        frame->pkt_dts = AV_NOPTS_VALUE;
#if FF_API_PKT_PTS
    FF_DISABLE_DEPRECATION_WARNINGS
    frame->pkt_pts = AV_NOPTS_VALUE;
    FF_ENABLE_DEPRECATION_WARNINGS
#endif
    frame->best_effort_timestamp = AV_NOPTS_VALUE;
    frame->pkt_duration = 0;
    frame->pkt_pos = -1;
    frame->pkt_size = -1;
    frame->key_frame = 1;
    frame->sample_aspect_ratio = (AVRational){0, 1};
    frame->format = -1; /* unknown */
    frame->extended_data = frame->data;
    frame->color_primaries = AVCOL_PRI_UNSPECIFIED;
    frame->color_trc = AVCOL_TRC_UNSPECIFIED;
    frame->colorspace = AVCOL_SPC_UNSPECIFIED;
    frame->color_range = AVCOL_RANGE_UNSPECIFIED;
    frame->chroma_location = AVCHROMA_LOC_UNSPECIFIED;
    frame->flags = 0;
}

void av_frame_unref(AVFrame *frame)
{
    int i;

    if (!frame)
        return;

    wipe_side_data(frame);

    for (i = 0; i < FF_ARRAY_ELEMS(frame->buf); i++)
        av_buffer_unref(&frame->buf[i]);
    for (i = 0; i < frame->nb_extended_buf; i++)
        av_buffer_unref(&frame->extended_buf[i]);
    av_freep(&frame->extended_buf);
    av_dict_free(&frame->metadata);
#if FF_API_FRAME_QP
    FF_DISABLE_DEPRECATION_WARNINGS
    av_buffer_unref(&frame->qp_table_buf);
    FF_ENABLE_DEPRECATION_WARNINGS
#endif

    av_buffer_unref(&frame->hw_frames_ctx);

    av_buffer_unref(&frame->opaque_ref);
    av_buffer_unref(&frame->private_ref);

    get_frame_defaults(frame);
}

int avcodec_is_open(AVCodecContext *s)
{
    return !!s->internal;
}

void av_frame_move_ref(AVFrame *dst, AVFrame *src)
{
    av_assert1(dst->width == 0 && dst->height == 0);
    av_assert1(dst->channels == 0);

    *dst = *src;
    if (src->extended_data == src->data)
        dst->extended_data = dst->data;
    memset(src, 0, sizeof(*src));
    get_frame_defaults(src);
}

static int decode_receive_frame_internal(AVCodecContext *avctx, AVFrame *frame)
{
    AVCodecInternal *avci = avctx->internal;
    int ret;

    av_assert0(!frame->buf[0]);

    if (avctx->codec->receive_frame) {
        ret = avctx->codec->receive_frame(avctx, frame);
        if (ret != AVERROR(EAGAIN))
            av_packet_unref(avci->last_pkt_props);
    } else
        ret = decode_simple_receive_frame(avctx, frame);

    if (ret == AVERROR_EOF)
        avci->draining_done = 1;

    if (!ret) {
        /* the only case where decode data is not set should be decoders
         * that do not call ff_get_buffer() */
        av_assert0((frame->private_ref && frame->private_ref->size == sizeof(FrameDecodeData)) ||
                   !(avctx->codec->capabilities & AV_CODEC_CAP_DR1));

        if (frame->private_ref) {
            FrameDecodeData *fdd = (FrameDecodeData*)frame->private_ref->data;

            if (fdd->post_process) {
                ret = fdd->post_process(avctx, frame);
                if (ret < 0) {
                    av_frame_unref(frame);
                    return ret;
                }
            }
        }
    }

    /* free the per-frame decode data */
    av_buffer_unref(&frame->private_ref);

    return ret;
}

static int apply_cropping(AVCodecContext *avctx, AVFrame *frame)
{
    /* make sure we are noisy about decoders returning invalid cropping data */
    if (frame->crop_left >= INT_MAX - frame->crop_right        ||
        frame->crop_top  >= INT_MAX - frame->crop_bottom       ||
        (frame->crop_left + frame->crop_right) >= frame->width ||
        (frame->crop_top + frame->crop_bottom) >= frame->height) {
        av_log(avctx, AV_LOG_WARNING,
               "Invalid cropping information set by a decoder: "
               "%"SIZE_SPECIFIER"/%"SIZE_SPECIFIER"/%"SIZE_SPECIFIER"/%"SIZE_SPECIFIER" "
               "(frame size %dx%d). This is a bug, please report it\n",
               frame->crop_left, frame->crop_right, frame->crop_top, frame->crop_bottom,
               frame->width, frame->height);
        frame->crop_left   = 0;
        frame->crop_right  = 0;
        frame->crop_top    = 0;
        frame->crop_bottom = 0;
        return 0;
    }

    if (!avctx->apply_cropping)
        return 0;

    return av_frame_apply_cropping(frame, avctx->flags & AV_CODEC_FLAG_UNALIGNED ?
                                          AV_FRAME_CROP_UNALIGNED : 0);
}

int attribute_align_arg avcodec_receive_frame(AVCodecContext *avctx, AVFrame *frame)
{
    AVCodecInternal *avci = avctx->internal;
    int ret, changed;

    av_frame_unref(frame);

    if (!avcodec_is_open(avctx) || !av_codec_is_decoder(avctx->codec))
        return AVERROR(EINVAL);

    if (avci->buffer_frame->buf[0])
    {
        av_frame_move_ref(frame, avci->buffer_frame);
    }
    else
    {
        ret = decode_receive_frame_internal(avctx, frame);
        if (ret < 0)
            return ret;
    }

    if (avctx->codec_type == AVMEDIA_TYPE_VIDEO)
    {
        ret = apply_cropping(avctx, frame);
        if (ret < 0)
        {
            av_frame_unref(frame);
            return ret;
        }
    }

    avctx->frame_number++;

    if (avctx->flags & AV_CODEC_FLAG_DROPCHANGED)
    {

        if (avctx->frame_number == 1)
        {
            avci->initial_format = frame->format;
            switch (avctx->codec_type)
            {
            case AVMEDIA_TYPE_VIDEO:
                avci->initial_width = frame->width;
                avci->initial_height = frame->height;
                break;
            case AVMEDIA_TYPE_AUDIO:
                avci->initial_sample_rate = frame->sample_rate ? frame->sample_rate : avctx->sample_rate;
                avci->initial_channels = frame->channels;
                avci->initial_channel_layout = frame->channel_layout;
                break;
            }
        }

        if (avctx->frame_number > 1)
        {
            changed = avci->initial_format != frame->format;

            switch (avctx->codec_type)
            {
            case AVMEDIA_TYPE_VIDEO:
                changed |= avci->initial_width != frame->width ||
                           avci->initial_height != frame->height;
                break;
            case AVMEDIA_TYPE_AUDIO:
                changed |= avci->initial_sample_rate != frame->sample_rate ||
                           avci->initial_sample_rate != avctx->sample_rate ||
                           avci->initial_channels != frame->channels ||
                           avci->initial_channel_layout != frame->channel_layout;
                break;
            }

            if (changed)
            {
                avci->changed_frames_dropped++;
                av_log(avctx, AV_LOG_INFO, "dropped changed frame #%d pts %" PRId64 " drop count: %d \n",
                       avctx->frame_number, frame->pts,
                       avci->changed_frames_dropped);
                av_frame_unref(frame);
                return AVERROR_INPUT_CHANGED;
            }
        }
    }
    return 0;
}

int64_t av_rescale_rnd(int64_t a, int64_t b, int64_t c, enum AVRounding rnd)
{
    int64_t r = 0;
    av_assert2(c > 0);
    av_assert2(b >= 0);
    av_assert2((unsigned)(rnd & ~AV_ROUND_PASS_MINMAX) <= 5 && (rnd & ~AV_ROUND_PASS_MINMAX) != 4);

    if (c <= 0 || b < 0 || !((unsigned)(rnd & ~AV_ROUND_PASS_MINMAX) <= 5 && (rnd & ~AV_ROUND_PASS_MINMAX) != 4))
        return INT64_MIN;

    if (rnd & AV_ROUND_PASS_MINMAX)
    {
        if (a == INT64_MIN || a == INT64_MAX)
            return a;
        rnd -= AV_ROUND_PASS_MINMAX;
    }

    if (a < 0)
        return -(uint64_t)av_rescale_rnd(-FFMAX(a, -INT64_MAX), b, c, rnd ^ ((rnd >> 1) & 1));

    if (rnd == AV_ROUND_NEAR_INF)
        r = c / 2;
    else if (rnd & 1)
        r = c - 1;

    if (b <= INT_MAX && c <= INT_MAX)
    {
        if (a <= INT_MAX)
            return (a * b + r) / c;
        else
        {
            int64_t ad = a / c;
            int64_t a2 = (a % c * b + r) / c;
            if (ad >= INT32_MAX && b && ad > (INT64_MAX - a2) / b)
                return INT64_MIN;
            return ad * b + a2;
        }
    }
    else
    {
        uint64_t a0 = a & 0xFFFFFFFF;
        uint64_t a1 = a >> 32;
        uint64_t b0 = b & 0xFFFFFFFF;
        uint64_t b1 = b >> 32;
        uint64_t t1 = a0 * b1 + a1 * b0;
        uint64_t t1a = t1 << 32;
        int i;

        a0 = a0 * b0 + t1a;
        a1 = a1 * b1 + (t1 >> 32) + (a0 < t1a);
        a0 += r;
        a1 += a0 < r;

        for (i = 63; i >= 0; i--)
        {
            a1 += a1 + ((a0 >> i) & 1);
            t1 += t1;
            if (c <= a1)
            {
                a1 -= c;
                t1++;
            }
        }
        if (t1 > INT64_MAX)
            return INT64_MIN;
        return t1;
    }
}

int64_t av_rescale(int64_t a, int64_t b, int64_t c)
{
    return av_rescale_rnd(a, b, c, AV_ROUND_NEAR_INF);
}

int64_t av_rescale_q_rnd(int64_t a, AVRational bq, AVRational cq,
                         enum AVRounding rnd)
{
    int64_t b = bq.num * (int64_t)cq.den;
    int64_t c = cq.num * (int64_t)bq.den;
    return av_rescale_rnd(a, b, c, rnd);
}

int64_t av_rescale_q(int64_t a, AVRational bq, AVRational cq)
{
    return av_rescale_q_rnd(a, bq, cq, AV_ROUND_NEAR_INF);
}

int av_codec_is_encoder(const AVCodec *codec)
{
    return codec && (codec->encode_sub || codec->encode2 || codec->receive_packet);
}

int av_codec_is_decoder(const AVCodec *codec)
{
    return codec && (codec->decode || codec->receive_frame);
}

static void park_frame_worker_threads(FrameThreadContext *fctx, int thread_count)
{
    int i;

    async_unlock(fctx);

    for (i = 0; i < thread_count; i++) {
        PerThreadContext *p = &fctx->threads[i];

        if (atomic_load(&p->state) != STATE_INPUT_READY) {
            pthread_mutex_lock(&p->progress_mutex);
            while (atomic_load(&p->state) != STATE_INPUT_READY)
                pthread_cond_wait(&p->output_cond, &p->progress_mutex);
            pthread_mutex_unlock(&p->progress_mutex);
        }
        p->got_frame = 0;
    }

    async_lock(fctx);
}

static int update_context_from_thread(AVCodecContext *dst, AVCodecContext *src, int for_user)
{
    int err = 0;

    if (dst != src && (for_user || src->codec->update_thread_context)) {
        dst->time_base = src->time_base;
        dst->framerate = src->framerate;
        dst->width     = src->width;
        dst->height    = src->height;
        dst->pix_fmt   = src->pix_fmt;
        dst->sw_pix_fmt = src->sw_pix_fmt;

        dst->coded_width  = src->coded_width;
        dst->coded_height = src->coded_height;

        dst->has_b_frames = src->has_b_frames;
        dst->idct_algo    = src->idct_algo;

        dst->bits_per_coded_sample = src->bits_per_coded_sample;
        dst->sample_aspect_ratio   = src->sample_aspect_ratio;

        dst->profile = src->profile;
        dst->level   = src->level;

        dst->bits_per_raw_sample = src->bits_per_raw_sample;
        dst->ticks_per_frame     = src->ticks_per_frame;
        dst->color_primaries     = src->color_primaries;

        dst->color_trc   = src->color_trc;
        dst->colorspace  = src->colorspace;
        dst->color_range = src->color_range;
        dst->chroma_sample_location = src->chroma_sample_location;

        dst->hwaccel = src->hwaccel;
        dst->hwaccel_context = src->hwaccel_context;

        dst->channels       = src->channels;
        dst->sample_rate    = src->sample_rate;
        dst->sample_fmt     = src->sample_fmt;
        dst->channel_layout = src->channel_layout;
        dst->internal->hwaccel_priv_data = src->internal->hwaccel_priv_data;

        if (!!dst->hw_frames_ctx != !!src->hw_frames_ctx ||
            (dst->hw_frames_ctx && dst->hw_frames_ctx->data != src->hw_frames_ctx->data)) {
            av_buffer_unref(&dst->hw_frames_ctx);

            if (src->hw_frames_ctx) {
                dst->hw_frames_ctx = av_buffer_ref(src->hw_frames_ctx);
                if (!dst->hw_frames_ctx)
                    return AVERROR(ENOMEM);
            }
        }

        dst->hwaccel_flags = src->hwaccel_flags;

        err = av_buffer_replace(&dst->internal->pool, src->internal->pool);
        if (err < 0)
            return err;
    }

    if (for_user) {
#if FF_API_CODED_FRAME
FF_DISABLE_DEPRECATION_WARNINGS
        dst->coded_frame = src->coded_frame;
FF_ENABLE_DEPRECATION_WARNINGS
#endif
    } else {
        if (dst->codec->update_thread_context)
            err = dst->codec->update_thread_context(dst, src);
    }

    return err;
}

static void release_delayed_buffers(PerThreadContext *p)
{
    FrameThreadContext *fctx = p->parent;

    while (p->num_released_buffers > 0) {
        AVFrame *f;

        pthread_mutex_lock(&fctx->buffer_mutex);

        // fix extended data in case the caller screwed it up
        av_assert0(p->avctx->codec_type == AVMEDIA_TYPE_VIDEO ||
                   p->avctx->codec_type == AVMEDIA_TYPE_AUDIO);
        f = p->released_buffers[--p->num_released_buffers];
        f->extended_data = f->data;
        av_frame_unref(f);

        pthread_mutex_unlock(&fctx->buffer_mutex);
    }
}

void ff_thread_flush(AVCodecContext *avctx)
{
    int i;
    FrameThreadContext *fctx = avctx->internal->thread_ctx;

    if (!fctx)
        return;

    park_frame_worker_threads(fctx, avctx->thread_count);
    if (fctx->prev_thread)
    {
        if (fctx->prev_thread != &fctx->threads[0])
            update_context_from_thread(fctx->threads[0].avctx, fctx->prev_thread->avctx, 0);
    }

    fctx->next_decoding = fctx->next_finished = 0;
    fctx->delaying = 1;
    fctx->prev_thread = NULL;
    for (i = 0; i < avctx->thread_count; i++)
    {
        PerThreadContext *p = &fctx->threads[i];
        // Make sure decode flush calls with size=0 won't return old frames
        p->got_frame = 0;
        av_frame_unref(p->frame);
        p->result = 0;

        release_delayed_buffers(p);

        if (avctx->codec->flush)
            avctx->codec->flush(p->avctx);
    }
}

void av_bsf_flush(AVBSFContext *ctx)
{
    AVBSFInternal *bsfi = ctx->internal;

    bsfi->eof = 0;

    av_packet_unref(bsfi->buffer_pkt);

    if (ctx->filter->flush)
        ctx->filter->flush(ctx);
}

void avcodec_flush_buffers(AVCodecContext *avctx)
{
    AVCodecInternal *avci = avctx->internal;

    if (av_codec_is_encoder(avctx->codec))
    {
        int caps = avctx->codec->capabilities;

        if (!(caps & AV_CODEC_CAP_ENCODER_FLUSH))
        {
            // Only encoders that explicitly declare support for it can be
            // flushed. Otherwise, this is a no-op.
            av_log(avctx, AV_LOG_WARNING, "Ignoring attempt to flush encoder "
                                          "that doesn't support it\n");
            return;
        }

        // We haven't implemented flushing for frame-threaded encoders.
        av_assert0(!(caps & AV_CODEC_CAP_FRAME_THREADS));
    }

    avci->draining = 0;
    avci->draining_done = 0;
    avci->nb_draining_errors = 0;
    av_frame_unref(avci->buffer_frame);
    av_frame_unref(avci->compat_decode_frame);
    av_packet_unref(avci->compat_encode_packet);
    av_packet_unref(avci->buffer_pkt);

    av_frame_unref(avci->es.in_frame);
    av_packet_unref(avci->ds.in_pkt);

    if (HAVE_THREADS && avctx->active_thread_type & FF_THREAD_FRAME)
        ff_thread_flush(avctx);
    else if (avctx->codec->flush)
        avctx->codec->flush(avctx);

    avctx->pts_correction_last_pts =
        avctx->pts_correction_last_dts = INT64_MIN;

    if (av_codec_is_decoder(avctx->codec))
        av_bsf_flush(avci->bsf);

    if (!avctx->refcounted_frames)
        av_frame_unref(avci->to_free);
}

int avcodec_decode_subtitle2(AVCodecContext *avctx, AVSubtitle *sub,
                             int *got_sub_ptr,
                             AVPacket *avpkt)
{
    int i, ret = 0;

    if (!avpkt->data && avpkt->size) {
        av_log(avctx, AV_LOG_ERROR, "invalid packet: NULL data, size != 0\n");
        return AVERROR(EINVAL);
    }
    if (!avctx->codec)
        return AVERROR(EINVAL);
    if (avctx->codec->type != AVMEDIA_TYPE_SUBTITLE) {
        av_log(avctx, AV_LOG_ERROR, "Invalid media type for subtitles\n");
        return AVERROR(EINVAL);
    }

    *got_sub_ptr = 0;
    get_subtitle_defaults(sub);

    if ((avctx->codec->capabilities & AV_CODEC_CAP_DELAY) || avpkt->size) {
        AVPacket pkt_recoded = *avpkt;

        ret = recode_subtitle(avctx, &pkt_recoded, avpkt);
        if (ret < 0) {
            *got_sub_ptr = 0;
        } else {
             ret = extract_packet_props(avctx->internal, &pkt_recoded);
             if (ret < 0)
                return ret;

            if (avctx->pkt_timebase.num && avpkt->pts != AV_NOPTS_VALUE)
                sub->pts = av_rescale_q(avpkt->pts,
                                        avctx->pkt_timebase, AV_TIME_BASE_Q);
            ret = avctx->codec->decode(avctx, sub, got_sub_ptr, &pkt_recoded);
            av_assert1((ret >= 0) >= !!*got_sub_ptr &&
                       !!*got_sub_ptr >= !!sub->num_rects);

#if FF_API_ASS_TIMING
            if (avctx->sub_text_format == FF_SUB_TEXT_FMT_ASS_WITH_TIMINGS
                && *got_sub_ptr && sub->num_rects) {
                const AVRational tb = avctx->pkt_timebase.num ? avctx->pkt_timebase
                                                              : avctx->time_base;
                int err = convert_sub_to_old_ass_form(sub, avpkt, tb);
                if (err < 0)
                    ret = err;
            }
#endif

            if (sub->num_rects && !sub->end_display_time && avpkt->duration &&
                avctx->pkt_timebase.num) {
                AVRational ms = { 1, 1000 };
                sub->end_display_time = av_rescale_q(avpkt->duration,
                                                     avctx->pkt_timebase, ms);
            }

            if (avctx->codec_descriptor->props & AV_CODEC_PROP_BITMAP_SUB)
                sub->format = 0;
            else if (avctx->codec_descriptor->props & AV_CODEC_PROP_TEXT_SUB)
                sub->format = 1;

            for (i = 0; i < sub->num_rects; i++) {
                if (avctx->sub_charenc_mode != FF_SUB_CHARENC_MODE_IGNORE &&
                    sub->rects[i]->ass && !utf8_check(sub->rects[i]->ass)) {
                    av_log(avctx, AV_LOG_ERROR,
                           "Invalid UTF-8 in decoded subtitles text; "
                           "maybe missing -sub_charenc option\n");
                    avsubtitle_free(sub);
                    ret = AVERROR_INVALIDDATA;
                    break;
                }
            }

            if (avpkt->data != pkt_recoded.data) { // did we recode?
                /* prevent from destroying side data from original packet */
                pkt_recoded.side_data = NULL;
                pkt_recoded.side_data_elems = 0;

                av_packet_unref(&pkt_recoded);
            }
        }

        if (*got_sub_ptr)
            avctx->frame_number++;
    }

    return ret;
}


int attribute_align_arg avcodec_send_packet(AVCodecContext *avctx, const AVPacket *avpkt)
{
    AVCodecInternal *avci = avctx->internal;
    int ret;

    if (!avcodec_is_open(avctx) || !av_codec_is_decoder(avctx->codec))
        return AVERROR(EINVAL);

    if (avctx->internal->draining)
        return AVERROR_EOF;

    if (avpkt && !avpkt->size && avpkt->data)
        return AVERROR(EINVAL);

    av_packet_unref(avci->buffer_pkt);
    if (avpkt && (avpkt->data || avpkt->side_data_elems)) {
        ret = av_packet_ref(avci->buffer_pkt, avpkt);
        if (ret < 0)
            return ret;
    }

    ret = av_bsf_send_packet(avci->bsf, avci->buffer_pkt);
    if (ret < 0) {
        av_packet_unref(avci->buffer_pkt);
        return ret;
    }

    if (!avci->buffer_frame->buf[0]) {
        ret = decode_receive_frame_internal(avctx, avci->buffer_frame);
        if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
            return ret;
    }

    return 0;
}

static int decoder_decode_frame(Decoder *d, AVFrame *frame, AVSubtitle *sub)
{
    int ret = AVERROR(EAGAIN);
    for (;;)
    {
        AVPacket pkt;
        if (d->queue->serial == d->pkt_serial)
        {
            do
            {
                if (d->queue->abort_request)
                    return -1;
                switch (d->avctx->codec_type)
                {
                case AVMEDIA_TYPE_VIDEO:
                    ret = avcodec_receive_frame(d->avctx, frame);
                    if (ret >= 0)
                    {
                        if (decoder_reorder_pts == -1)
                        {
                            frame->pts = frame->best_effort_timestamp;
                        }
                        else if (!decoder_reorder_pts)
                        {
                            frame->pts = frame->pkt_dts;
                        }
                    }
                    break;
                case AVMEDIA_TYPE_AUDIO:
                    ret = avcodec_receive_frame(d->avctx, frame);
                    if (ret >= 0)
                    {
                        AVRational tb = {1, frame->sample_rate}; //(AVRational){1, frame->sample_rate};
                        if (frame->pts != AV_NOPTS_VALUE)
                            frame->pts = av_rescale_q(frame->pts, d->avctx->pkt_timebase, tb);
                        else if (d->next_pts != AV_NOPTS_VALUE)
                            frame->pts = av_rescale_q(d->next_pts, d->next_pts_tb, tb);
                        if (frame->pts != AV_NOPTS_VALUE)
                        {
                            d->next_pts = frame->pts + frame->nb_samples;
                            d->next_pts_tb = tb;
                        }
                    }
                    break;
                }
                if (ret == AVERROR_EOF)
                {
                    d->finished = d->pkt_serial;
                    avcodec_flush_buffers(d->avctx);
                    return 0;
                }
                if (ret >= 0)
                    return 1;
            } while (ret != AVERROR(EAGAIN));
        }

        do
        {
            if (d->queue->nb_packets == 0)
                SDL_CondSignal(d->empty_queue_cond);
            if (d->packet_pending)
            {
                av_packet_move_ref(&pkt, &d->pkt);
                d->packet_pending = 0;
            }
            else
            {
                if (packet_queue_get(d->queue, &pkt, 1, &d->pkt_serial) < 0)
                    return -1;
            }
            if (d->queue->serial == d->pkt_serial)
                break;
            av_packet_unref(&pkt);
        } while (1);

        if (pkt.data == flush_pkt.data)
        {
            avcodec_flush_buffers(d->avctx);
            d->finished = 0;
            d->next_pts = d->start_pts;
            d->next_pts_tb = d->start_pts_tb;
        }
        else
        {
            if (d->avctx->codec_type == AVMEDIA_TYPE_SUBTITLE)
            {
                int got_frame = 0;
                ret = avcodec_decode_subtitle2(d->avctx, sub, &got_frame, &pkt);
                if (ret < 0)
                {
                    ret = AVERROR(EAGAIN);
                }
                else
                {
                    if (got_frame && !pkt.data)
                    {
                        d->packet_pending = 1;
                        av_packet_move_ref(&d->pkt, &pkt);
                    }
                    ret = got_frame ? 0 : (pkt.data ? AVERROR(EAGAIN) : AVERROR_EOF);
                }
            }
            else
            {
                if (avcodec_send_packet(d->avctx, &pkt) == AVERROR(EAGAIN))
                {
                    av_log(d->avctx, AV_LOG_ERROR, "Receive_frame and send_packet both returned EAGAIN, which is an API violation.\n");
                    d->packet_pending = 1;
                    av_packet_move_ref(&d->pkt, &pkt);
                }
            }
            av_packet_unref(&pkt);
        }
    }
}

void avcodec_free_context(AVCodecContext **pavctx)
{
    AVCodecContext *avctx = *pavctx;

    if (!avctx)
        return;

    avcodec_close(avctx);

    av_freep(&avctx->extradata);
    av_freep(&avctx->subtitle_header);
    av_freep(&avctx->intra_matrix);
    av_freep(&avctx->inter_matrix);
    av_freep(&avctx->rc_override);

    av_freep(pavctx);
}

static void decoder_destroy(Decoder *d)
{
    av_packet_unref(&d->pkt);
    avcodec_free_context(&d->avctx);
}

void avsubtitle_free(AVSubtitle *sub)
{
    int i;
    for (i = 0; i < sub->num_rects; i++)
    {
        av_freep(&sub->rects[i]->data[0]);
        av_freep(&sub->rects[i]->data[1]);
        av_freep(&sub->rects[i]->data[2]);
        av_freep(&sub->rects[i]->data[3]);
        av_freep(&sub->rects[i]->text);
        av_freep(&sub->rects[i]->ass);
        av_freep(&sub->rects[i]);
    }
    av_freep(&sub->rects);
    memset(sub, 0, sizeof(*sub));
}

static void frame_queue_unref_item(Frame *vp)
{
    av_frame_unref(vp->frame);
    avsubtitle_free(&vp->sub);
}

AVFrame *av_frame_alloc(void)
{
    AVFrame *frame = av_mallocz(sizeof(*frame));

    if (!frame)
        return NULL;

    frame->extended_data = NULL;
    get_frame_defaults(frame);

    return frame;
}


void av_frame_free(AVFrame **frame)
{
    if (!frame || !*frame)
        return;

    av_frame_unref(*frame);
    av_freep(frame);
}

static int frame_queue_init(FrameQueue *f, PacketQueue *pktq, int max_size, int keep_last)
{
    int i;
    memset(f, 0, sizeof(FrameQueue));
    if (!(f->mutex = SDL_CreateMutex()))
    {
        av_log(NULL, AV_LOG_FATAL, "SDL_CreateMutex(): %s\n", SDL_GetError());
        return AVERROR(ENOMEM);
    }
    if (!(f->cond = SDL_CreateCond()))
    {
        av_log(NULL, AV_LOG_FATAL, "SDL_CreateCond(): %s\n", SDL_GetError());
        return AVERROR(ENOMEM);
    }
    f->pktq = pktq;
    f->max_size = FFMIN(max_size, FRAME_QUEUE_SIZE);
    f->keep_last = !!keep_last;
    for (i = 0; i < f->max_size; i++)
        if (!(f->queue[i].frame = av_frame_alloc()))
            return AVERROR(ENOMEM);
    return 0;
}

static void frame_queue_destory(FrameQueue *f)
{
    int i;
    for (i = 0; i < f->max_size; i++)
    {
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
    /* wait until we have space to put a new frame */
    SDL_LockMutex(f->mutex);
    while (f->size >= f->max_size &&
           !f->pktq->abort_request)
    {
        SDL_CondWait(f->cond, f->mutex);
    }
    SDL_UnlockMutex(f->mutex);

    if (f->pktq->abort_request)
        return NULL;

    return &f->queue[f->windex];
}

static Frame *frame_queue_peek_readable(FrameQueue *f)
{
    /* wait until we have a readable a new frame */
    SDL_LockMutex(f->mutex);
    while (f->size - f->rindex_shown <= 0 &&
           !f->pktq->abort_request)
    {
        SDL_CondWait(f->cond, f->mutex);
    }
    SDL_UnlockMutex(f->mutex);

    if (f->pktq->abort_request)
        return NULL;

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
    if (f->keep_last && !f->rindex_shown)
    {
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

/* return the number of undisplayed frames in the queue */
static int frame_queue_nb_remaining(FrameQueue *f)
{
    return f->size - f->rindex_shown;
}

/* return last shown position */
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
    SDL_WaitThread(d->decoder_tid, NULL);
    d->decoder_tid = NULL;
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
    if (!*texture || SDL_QueryTexture(*texture, &format, &access, &w, &h) < 0 || new_width != w || new_height != h || new_format != format)
    {
        void *pixels;
        int pitch;
        if (*texture)
            SDL_DestroyTexture(*texture);
        if (!(*texture = SDL_CreateTexture(renderer, new_format, SDL_TEXTUREACCESS_STREAMING, new_width, new_height)))
            return -1;
        if (SDL_SetTextureBlendMode(*texture, blendmode) < 0)
            return -1;
        if (init_texture)
        {
            if (SDL_LockTexture(*texture, NULL, &pixels, &pitch) < 0)
                return -1;
            memset(pixels, 0, pitch * new_height);
            SDL_UnlockTexture(*texture);
        }
        av_log(NULL, AV_LOG_VERBOSE, "Created %dx%d texture with %s.\n", new_width, new_height, SDL_GetPixelFormatName(new_format));
    }
    return 0;
}

static inline AVRational av_make_q(int num, int den)
{
    AVRational r = {num, den};
    return r;
}

int64_t av_gcd(int64_t a, int64_t b)
{
    int za, zb, k;
    int64_t u, v;
    if (a == 0)
        return b;
    if (b == 0)
        return a;
    za = ff_ctzll(a);
    zb = ff_ctzll(b);
    k = FFMIN(za, zb);
    u = llabs(a >> za);
    v = llabs(b >> zb);
    while (u != v)
    {
        if (u > v)
            FFSWAP(int64_t, v, u);
        v -= u;
        v >>= ff_ctzll(v);
    }
    return (uint64_t)u << k;
}

int av_reduce(int *dst_num, int *dst_den,
              int64_t num, int64_t den, int64_t max)
{
    AVRational a0 = {0, 1}, a1 = {1, 0};
    int sign = (num < 0) ^ (den < 0);
    int64_t gcd = av_gcd(FFABS(num), FFABS(den));

    if (gcd)
    {
        num = FFABS(num) / gcd;
        den = FFABS(den) / gcd;
    }
    if (num <= max && den <= max)
    {
        a1 = (AVRational){num, den};
        den = 0;
    }

    while (den)
    {
        uint64_t x = num / den;
        int64_t next_den = num - den * x;
        int64_t a2n = x * a1.num + a0.num;
        int64_t a2d = x * a1.den + a0.den;

        if (a2n > max || a2d > max)
        {
            if (a1.num)
                x = (max - a0.num) / a1.num;
            if (a1.den)
                x = FFMIN(x, (max - a0.den) / a1.den);

            if (den * (2 * x * a1.den + a0.den) > num * a1.den)
                a1 = (AVRational){x * a1.num + a0.num, x * a1.den + a0.den};
            break;
        }

        a0 = a1;
        a1 = (AVRational){a2n, a2d};
        num = den;
        den = next_den;
    }
    av_assert2(av_gcd(a1.num, a1.den) <= 1U);
    av_assert2(a1.num <= max && a1.den <= max);

    *dst_num = sign ? -a1.num : a1.num;
    *dst_den = a1.den;

    return den == 0;
}

AVRational av_mul_q(AVRational b, AVRational c)
{
    av_reduce(&b.num, &b.den,
              b.num * (int64_t)c.num,
              b.den * (int64_t)c.den, INT_MAX);
    return b;
}

AVRational av_div_q(AVRational b, AVRational c)
{
    return av_mul_q(b, (AVRational){c.den, c.num});
}

static inline int av_cmp_q(AVRational a, AVRational b)
{
    const int64_t tmp = a.num * (int64_t)b.den - b.num * (int64_t)a.den;

    if (tmp)
        return (int)((tmp ^ a.den ^ b.den) >> 63) | 1;
    else if (b.den && a.den)
        return 0;
    else if (a.num && b.num)
        return (a.num >> 31) - (b.num >> 31);
    else
        return INT_MIN;
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

    /* XXX: we suppose the screen has a 1.0 pixel ratio */
    height = scr_height;
    width = av_rescale(height, aspect_ratio.num, aspect_ratio.den) & ~1;
    if (width > scr_width)
    {
        width = scr_width;
        height = av_rescale(width, aspect_ratio.den, aspect_ratio.num) & ~1;
    }
    x = (scr_width - width) / 2;
    y = (scr_height - height) / 2;
    rect->x = scr_xleft + x;
    rect->y = scr_ytop + y;
    rect->w = FFMAX((int)width, 1);
    rect->h = FFMAX((int)height, 1);
}

static void get_sdl_pix_fmt_and_blendmode(int format, Uint32 *sdl_pix_fmt, SDL_BlendMode *sdl_blendmode)
{
    int i;
    *sdl_blendmode = SDL_BLENDMODE_NONE;
    *sdl_pix_fmt = SDL_PIXELFORMAT_UNKNOWN;
    if (format == AV_PIX_FMT_RGB32 ||
        format == AV_PIX_FMT_RGB32_1 ||
        format == AV_PIX_FMT_BGR32 ||
        format == AV_PIX_FMT_BGR32_1)
        *sdl_blendmode = SDL_BLENDMODE_BLEND;
    for (i = 0; i < FF_ARRAY_ELEMS(sdl_texture_format_map) - 1; i++)
    {
        if (format == sdl_texture_format_map[i].format)
        {
            *sdl_pix_fmt = sdl_texture_format_map[i].texture_fmt;
            return;
        }
    }
}

int av_opt_get_int(void *obj, const char *name, int search_flags, int64_t *out_val)
{
    int64_t intnum = 1;
    double num = 1;
    int ret, den = 1;

    if ((ret = get_number(obj, name, NULL, &num, &den, &intnum, search_flags)) < 0)
        return ret;
    *out_val = num * intnum / den;
    return 0;
}



void sws_freeContext(SwsContext *c)
{
    int i;
    if (!c)
        return;

    for (i = 0; i < 4; i++)
        av_freep(&c->dither_error[i]);

    av_freep(&c->vLumFilter);
    av_freep(&c->vChrFilter);
    av_freep(&c->hLumFilter);
    av_freep(&c->hChrFilter);
#if HAVE_ALTIVEC
    av_freep(&c->vYCoeffsBank);
    av_freep(&c->vCCoeffsBank);
#endif

    av_freep(&c->vLumFilterPos);
    av_freep(&c->vChrFilterPos);
    av_freep(&c->hLumFilterPos);
    av_freep(&c->hChrFilterPos);

#if HAVE_MMX_INLINE
#if USE_MMAP
    if (c->lumMmxextFilterCode)
        munmap(c->lumMmxextFilterCode, c->lumMmxextFilterCodeSize);
    if (c->chrMmxextFilterCode)
        munmap(c->chrMmxextFilterCode, c->chrMmxextFilterCodeSize);
#elif HAVE_VIRTUALALLOC
    if (c->lumMmxextFilterCode)
        VirtualFree(c->lumMmxextFilterCode, 0, MEM_RELEASE);
    if (c->chrMmxextFilterCode)
        VirtualFree(c->chrMmxextFilterCode, 0, MEM_RELEASE);
#else
    av_free(c->lumMmxextFilterCode);
    av_free(c->chrMmxextFilterCode);
#endif
    c->lumMmxextFilterCode = NULL;
    c->chrMmxextFilterCode = NULL;
#endif /* HAVE_MMX_INLINE */

    av_freep(&c->yuvTable);
    av_freep(&c->formatConvBuffer);

    sws_freeContext(c->cascaded_context[0]);
    sws_freeContext(c->cascaded_context[1]);
    sws_freeContext(c->cascaded_context[2]);
    memset(c->cascaded_context, 0, sizeof(c->cascaded_context));
    av_freep(&c->cascaded_tmp[0]);
    av_freep(&c->cascaded1_tmp[0]);

    av_freep(&c->gamma);
    av_freep(&c->inv_gamma);

    ff_free_filters(c);

    av_free(c);
}

static const char *sws_context_to_name(void *ptr)
{
    return "swscaler";
}

#define OFFSET(x) offsetof(SwsContext, x)
#define DEFAULT 0
#define VE AV_OPT_FLAG_VIDEO_PARAM | AV_OPT_FLAG_ENCODING_PARAM

static const AVOption swscale_options[] = {
    { "sws_flags",       "scaler flags",                  OFFSET(flags),     AV_OPT_TYPE_FLAGS,  { .i64  = SWS_BICUBIC        }, 0,      UINT_MAX,        VE, "sws_flags" },
    { "fast_bilinear",   "fast bilinear",                 0,                 AV_OPT_TYPE_CONST,  { .i64  = SWS_FAST_BILINEAR  }, INT_MIN, INT_MAX,        VE, "sws_flags" },
    { "bilinear",        "bilinear",                      0,                 AV_OPT_TYPE_CONST,  { .i64  = SWS_BILINEAR       }, INT_MIN, INT_MAX,        VE, "sws_flags" },
    { "bicubic",         "bicubic",                       0,                 AV_OPT_TYPE_CONST,  { .i64  = SWS_BICUBIC        }, INT_MIN, INT_MAX,        VE, "sws_flags" },
    { "experimental",    "experimental",                  0,                 AV_OPT_TYPE_CONST,  { .i64  = SWS_X              }, INT_MIN, INT_MAX,        VE, "sws_flags" },
    { "neighbor",        "nearest neighbor",              0,                 AV_OPT_TYPE_CONST,  { .i64  = SWS_POINT          }, INT_MIN, INT_MAX,        VE, "sws_flags" },
    { "area",            "averaging area",                0,                 AV_OPT_TYPE_CONST,  { .i64  = SWS_AREA           }, INT_MIN, INT_MAX,        VE, "sws_flags" },
    { "bicublin",        "luma bicubic, chroma bilinear", 0,                 AV_OPT_TYPE_CONST,  { .i64  = SWS_BICUBLIN       }, INT_MIN, INT_MAX,        VE, "sws_flags" },
    { "gauss",           "Gaussian",                      0,                 AV_OPT_TYPE_CONST,  { .i64  = SWS_GAUSS          }, INT_MIN, INT_MAX,        VE, "sws_flags" },
    { "sinc",            "sinc",                          0,                 AV_OPT_TYPE_CONST,  { .i64  = SWS_SINC           }, INT_MIN, INT_MAX,        VE, "sws_flags" },
    { "lanczos",         "Lanczos",                       0,                 AV_OPT_TYPE_CONST,  { .i64  = SWS_LANCZOS        }, INT_MIN, INT_MAX,        VE, "sws_flags" },
    { "spline",          "natural bicubic spline",        0,                 AV_OPT_TYPE_CONST,  { .i64  = SWS_SPLINE         }, INT_MIN, INT_MAX,        VE, "sws_flags" },
    { "print_info",      "print info",                    0,                 AV_OPT_TYPE_CONST,  { .i64  = SWS_PRINT_INFO     }, INT_MIN, INT_MAX,        VE, "sws_flags" },
    { "accurate_rnd",    "accurate rounding",             0,                 AV_OPT_TYPE_CONST,  { .i64  = SWS_ACCURATE_RND   }, INT_MIN, INT_MAX,        VE, "sws_flags" },
    { "full_chroma_int", "full chroma interpolation",     0,                 AV_OPT_TYPE_CONST,  { .i64  = SWS_FULL_CHR_H_INT }, INT_MIN, INT_MAX,        VE, "sws_flags" },
    { "full_chroma_inp", "full chroma input",             0,                 AV_OPT_TYPE_CONST,  { .i64  = SWS_FULL_CHR_H_INP }, INT_MIN, INT_MAX,        VE, "sws_flags" },
    { "bitexact",        "",                              0,                 AV_OPT_TYPE_CONST,  { .i64  = SWS_BITEXACT       }, INT_MIN, INT_MAX,        VE, "sws_flags" },
    { "error_diffusion", "error diffusion dither",        0,                 AV_OPT_TYPE_CONST,  { .i64  = SWS_ERROR_DIFFUSION}, INT_MIN, INT_MAX,        VE, "sws_flags" },

    { "srcw",            "source width",                  OFFSET(srcW),      AV_OPT_TYPE_INT,    { .i64 = 16                 }, 1,       INT_MAX,        VE },
    { "srch",            "source height",                 OFFSET(srcH),      AV_OPT_TYPE_INT,    { .i64 = 16                 }, 1,       INT_MAX,        VE },
    { "dstw",            "destination width",             OFFSET(dstW),      AV_OPT_TYPE_INT,    { .i64 = 16                 }, 1,       INT_MAX,        VE },
    { "dsth",            "destination height",            OFFSET(dstH),      AV_OPT_TYPE_INT,    { .i64 = 16                 }, 1,       INT_MAX,        VE },
    { "src_format",      "source format",                 OFFSET(srcFormat), AV_OPT_TYPE_PIXEL_FMT,{ .i64 = DEFAULT          }, 0,       INT_MAX, VE },
    { "dst_format",      "destination format",            OFFSET(dstFormat), AV_OPT_TYPE_PIXEL_FMT,{ .i64 = DEFAULT          }, 0,       INT_MAX, VE },
    { "src_range",       "source is full range",          OFFSET(srcRange),  AV_OPT_TYPE_BOOL,   { .i64 = DEFAULT            }, 0,       1,              VE },
    { "dst_range",       "destination is full range",     OFFSET(dstRange),  AV_OPT_TYPE_BOOL,   { .i64 = DEFAULT            }, 0,       1,              VE },
    { "param0",          "scaler param 0",                OFFSET(param[0]),  AV_OPT_TYPE_DOUBLE, { .dbl = SWS_PARAM_DEFAULT  }, INT_MIN, INT_MAX,        VE },
    { "param1",          "scaler param 1",                OFFSET(param[1]),  AV_OPT_TYPE_DOUBLE, { .dbl = SWS_PARAM_DEFAULT  }, INT_MIN, INT_MAX,        VE },

    { "src_v_chr_pos",   "source vertical chroma position in luma grid/256"  ,      OFFSET(src_v_chr_pos), AV_OPT_TYPE_INT, { .i64 = -513 }, -513,      512,             VE },
    { "src_h_chr_pos",   "source horizontal chroma position in luma grid/256",      OFFSET(src_h_chr_pos), AV_OPT_TYPE_INT, { .i64 = -513 }, -513,      512,             VE },
    { "dst_v_chr_pos",   "destination vertical chroma position in luma grid/256"  , OFFSET(dst_v_chr_pos), AV_OPT_TYPE_INT, { .i64 = -513 }, -513,      512,             VE },
    { "dst_h_chr_pos",   "destination horizontal chroma position in luma grid/256", OFFSET(dst_h_chr_pos), AV_OPT_TYPE_INT, { .i64 = -513 }, -513,      512,             VE },

    { "sws_dither",      "set dithering algorithm",       OFFSET(dither),    AV_OPT_TYPE_INT,    { .i64  = SWS_DITHER_AUTO    }, 0,       NB_SWS_DITHER,  VE, "sws_dither" },
    { "auto",            "leave choice to sws",           0,                 AV_OPT_TYPE_CONST,  { .i64  = SWS_DITHER_AUTO    }, INT_MIN, INT_MAX,        VE, "sws_dither" },
    { "bayer",           "bayer dither",                  0,                 AV_OPT_TYPE_CONST,  { .i64  = SWS_DITHER_BAYER   }, INT_MIN, INT_MAX,        VE, "sws_dither" },
    { "ed",              "error diffusion",               0,                 AV_OPT_TYPE_CONST,  { .i64  = SWS_DITHER_ED      }, INT_MIN, INT_MAX,        VE, "sws_dither" },
    { "a_dither",        "arithmetic addition dither",    0,                 AV_OPT_TYPE_CONST,  { .i64  = SWS_DITHER_A_DITHER}, INT_MIN, INT_MAX,        VE, "sws_dither" },
    { "x_dither",        "arithmetic xor dither",         0,                 AV_OPT_TYPE_CONST,  { .i64  = SWS_DITHER_X_DITHER}, INT_MIN, INT_MAX,        VE, "sws_dither" },
    { "gamma",           "gamma correct scaling",         OFFSET(gamma_flag),AV_OPT_TYPE_BOOL,   { .i64  = 0                  }, 0,       1,              VE },
    { "alphablend",      "mode for alpha -> non alpha",   OFFSET(alphablend),AV_OPT_TYPE_INT,    { .i64  = SWS_ALPHA_BLEND_NONE}, 0,       SWS_ALPHA_BLEND_NB-1, VE, "alphablend" },
    { "none",            "ignore alpha",                  0,                 AV_OPT_TYPE_CONST,  { .i64  = SWS_ALPHA_BLEND_NONE}, INT_MIN, INT_MAX,       VE, "alphablend" },
    { "uniform_color",   "blend onto a uniform color",    0,                 AV_OPT_TYPE_CONST,  { .i64  = SWS_ALPHA_BLEND_UNIFORM},INT_MIN, INT_MAX,     VE, "alphablend" },
    { "checkerboard",    "blend onto a checkerboard",     0,                 AV_OPT_TYPE_CONST,  { .i64  = SWS_ALPHA_BLEND_CHECKERBOARD},INT_MIN, INT_MAX,     VE, "alphablend" },

    { NULL }
};

const AVClass ff_sws_context_class = {
    .class_name = "SWScaler",
    .item_name  = sws_context_to_name,
    .option     = swscale_options,
    .category   = AV_CLASS_CATEGORY_SWSCALER,
    .version    = LIBAVUTIL_VERSION_INT,
};

SwsContext *sws_alloc_context(void)
{
    SwsContext *c = av_mallocz(sizeof(SwsContext));

    av_assert0(offsetof(SwsContext, redDither) + DITHER32_INT == offsetof(SwsContext, dither32));

    if (c) {
        c->av_class = &ff_sws_context_class;
        av_opt_set_defaults(c);
    }

    return c;
}

int av_opt_set_int(void *obj, const char *name, int64_t val, int search_flags)
{
    return set_number(obj, name, 1, 1, val, search_flags);
}

static const AVPixFmtDescriptor av_pix_fmt_descriptors[AV_PIX_FMT_NB] = {
    [AV_PIX_FMT_YUV420P] = {
        .name = "yuv420p",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 1, 0, 0, 8, 0, 7, 1 },        /* Y */
            { 1, 1, 0, 0, 8, 0, 7, 1 },        /* U */
            { 2, 1, 0, 0, 8, 0, 7, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUYV422] = {
        .name = "yuyv422",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 8, 1, 7, 1 },        /* Y */
            { 0, 4, 1, 0, 8, 3, 7, 2 },        /* U */
            { 0, 4, 3, 0, 8, 3, 7, 4 },        /* V */
        },
    },
    [AV_PIX_FMT_YVYU422] = {
        .name = "yvyu422",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 8, 1, 7, 1 },        /* Y */
            { 0, 4, 3, 0, 8, 3, 7, 4 },        /* U */
            { 0, 4, 1, 0, 8, 3, 7, 2 },        /* V */
        },
    },
    [AV_PIX_FMT_Y210LE] = {
        .name = "y210le",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 4, 0, 6, 10, 3, 9, 1 },        /* Y */
            { 0, 8, 2, 6, 10, 7, 9, 3 },        /* U */
            { 0, 8, 6, 6, 10, 7, 9, 7 },        /* V */
        },
    },
    [AV_PIX_FMT_Y210BE] = {
        .name = "y210be",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 4, 0, 6, 10, 3, 9, 1 },        /* Y */
            { 0, 8, 2, 6, 10, 7, 9, 3 },        /* U */
            { 0, 8, 6, 6, 10, 7, 9, 7 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_BE,
    },
    [AV_PIX_FMT_RGB24] = {
        .name = "rgb24",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 3, 0, 0, 8, 2, 7, 1 },        /* R */
            { 0, 3, 1, 0, 8, 2, 7, 2 },        /* G */
            { 0, 3, 2, 0, 8, 2, 7, 3 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_BGR24] = {
        .name = "bgr24",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 3, 2, 0, 8, 2, 7, 3 },        /* R */
            { 0, 3, 1, 0, 8, 2, 7, 2 },        /* G */
            { 0, 3, 0, 0, 8, 2, 7, 1 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_X2RGB10LE] = {
        .name = "x2rgb10le",
        .nb_components= 3,
        .log2_chroma_w= 0,
        .log2_chroma_h= 0,
        .comp = {
            { 0, 4, 2, 4, 10, 3, 9, 2 },       /* R */
            { 0, 4, 1, 2, 10, 3, 9, 3 },       /* G */
            { 0, 4, 0, 0, 10, 3, 9, 4 },       /* B */
        },
        .flags = AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_X2RGB10BE] = {
        .name = "x2rgb10be",
        .nb_components= 3,
        .log2_chroma_w= 0,
        .log2_chroma_h= 0,
        .comp = {
            { 0, 4, 0, 4, 10, 3, 9, 2 },       /* R */
            { 0, 4, 1, 2, 10, 3, 9, 3 },       /* G */
            { 0, 4, 2, 0, 10, 3, 9, 4 },       /* B */
        },
        .flags = AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_BE,
    },
    [AV_PIX_FMT_YUV422P] = {
        .name = "yuv422p",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 1, 0, 0, 8, 0, 7, 1 },        /* Y */
            { 1, 1, 0, 0, 8, 0, 7, 1 },        /* U */
            { 2, 1, 0, 0, 8, 0, 7, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV444P] = {
        .name = "yuv444p",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 1, 0, 0, 8, 0, 7, 1 },        /* Y */
            { 1, 1, 0, 0, 8, 0, 7, 1 },        /* U */
            { 2, 1, 0, 0, 8, 0, 7, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV410P] = {
        .name = "yuv410p",
        .nb_components = 3,
        .log2_chroma_w = 2,
        .log2_chroma_h = 2,
        .comp = {
            { 0, 1, 0, 0, 8, 0, 7, 1 },        /* Y */
            { 1, 1, 0, 0, 8, 0, 7, 1 },        /* U */
            { 2, 1, 0, 0, 8, 0, 7, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV411P] = {
        .name = "yuv411p",
        .nb_components = 3,
        .log2_chroma_w = 2,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 1, 0, 0, 8, 0, 7, 1 },        /* Y */
            { 1, 1, 0, 0, 8, 0, 7, 1 },        /* U */
            { 2, 1, 0, 0, 8, 0, 7, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUVJ411P] = {
        .name = "yuvj411p",
        .nb_components = 3,
        .log2_chroma_w = 2,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 1, 0, 0, 8, 0, 7, 1 },        /* Y */
            { 1, 1, 0, 0, 8, 0, 7, 1 },        /* U */
            { 2, 1, 0, 0, 8, 0, 7, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_GRAY8] = {
        .name = "gray",
        .nb_components = 1,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 1, 0, 0, 8, 0, 7, 1 },        /* Y */
        },
        .flags = FF_PSEUDOPAL,
        .alias = "gray8,y8",
    },
    [AV_PIX_FMT_MONOWHITE] = {
        .name = "monow",
        .nb_components = 1,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 1, 0, 0, 1, 0, 0, 1 },        /* Y */
        },
        .flags = AV_PIX_FMT_FLAG_BITSTREAM,
    },
    [AV_PIX_FMT_MONOBLACK] = {
        .name = "monob",
        .nb_components = 1,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 1, 0, 7, 1, 0, 0, 1 },        /* Y */
        },
        .flags = AV_PIX_FMT_FLAG_BITSTREAM,
    },
    [AV_PIX_FMT_PAL8] = {
        .name = "pal8",
        .nb_components = 1,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 1, 0, 0, 8, 0, 7, 1 },
        },
        .flags = AV_PIX_FMT_FLAG_PAL | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_YUVJ420P] = {
        .name = "yuvj420p",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 1, 0, 0, 8, 0, 7, 1 },        /* Y */
            { 1, 1, 0, 0, 8, 0, 7, 1 },        /* U */
            { 2, 1, 0, 0, 8, 0, 7, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUVJ422P] = {
        .name = "yuvj422p",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 1, 0, 0, 8, 0, 7, 1 },        /* Y */
            { 1, 1, 0, 0, 8, 0, 7, 1 },        /* U */
            { 2, 1, 0, 0, 8, 0, 7, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUVJ444P] = {
        .name = "yuvj444p",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 1, 0, 0, 8, 0, 7, 1 },        /* Y */
            { 1, 1, 0, 0, 8, 0, 7, 1 },        /* U */
            { 2, 1, 0, 0, 8, 0, 7, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_XVMC] = {
        .name = "xvmc",
        .flags = AV_PIX_FMT_FLAG_HWACCEL,
    },
    [AV_PIX_FMT_UYVY422] = {
        .name = "uyvy422",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 1, 0, 8, 1, 7, 2 },        /* Y */
            { 0, 4, 0, 0, 8, 3, 7, 1 },        /* U */
            { 0, 4, 2, 0, 8, 3, 7, 3 },        /* V */
        },
    },
    [AV_PIX_FMT_UYYVYY411] = {
        .name = "uyyvyy411",
        .nb_components = 3,
        .log2_chroma_w = 2,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 4, 1, 0, 8, 3, 7, 2 },        /* Y */
            { 0, 6, 0, 0, 8, 5, 7, 1 },        /* U */
            { 0, 6, 3, 0, 8, 5, 7, 4 },        /* V */
        },
    },
    [AV_PIX_FMT_BGR8] = {
        .name = "bgr8",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 1, 0, 0, 3, 0, 2, 1 },        /* R */
            { 0, 1, 0, 3, 3, 0, 2, 1 },        /* G */
            { 0, 1, 0, 6, 2, 0, 1, 1 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_RGB | FF_PSEUDOPAL,
    },
    [AV_PIX_FMT_BGR4] = {
        .name = "bgr4",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 4, 3, 0, 1, 3, 0, 4 },        /* R */
            { 0, 4, 1, 0, 2, 3, 1, 2 },        /* G */
            { 0, 4, 0, 0, 1, 3, 0, 1 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_BITSTREAM | AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_BGR4_BYTE] = {
        .name = "bgr4_byte",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 1, 0, 0, 1, 0, 0, 1 },        /* R */
            { 0, 1, 0, 1, 2, 0, 1, 1 },        /* G */
            { 0, 1, 0, 3, 1, 0, 0, 1 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_RGB | FF_PSEUDOPAL,
    },
    [AV_PIX_FMT_RGB8] = {
        .name = "rgb8",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 1, 0, 6, 2, 0, 1, 1 },        /* R */
            { 0, 1, 0, 3, 3, 0, 2, 1 },        /* G */
            { 0, 1, 0, 0, 3, 0, 2, 1 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_RGB | FF_PSEUDOPAL,
    },
    [AV_PIX_FMT_RGB4] = {
        .name = "rgb4",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 4, 0, 0, 1, 3, 0, 1 },        /* R */
            { 0, 4, 1, 0, 2, 3, 1, 2 },        /* G */
            { 0, 4, 3, 0, 1, 3, 0, 4 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_BITSTREAM | AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_RGB4_BYTE] = {
        .name = "rgb4_byte",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 1, 0, 3, 1, 0, 0, 1 },        /* R */
            { 0, 1, 0, 1, 2, 0, 1, 1 },        /* G */
            { 0, 1, 0, 0, 1, 0, 0, 1 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_RGB | FF_PSEUDOPAL,
    },
    [AV_PIX_FMT_NV12] = {
        .name = "nv12",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 1, 0, 0, 8, 0, 7, 1 },        /* Y */
            { 1, 2, 0, 0, 8, 1, 7, 1 },        /* U */
            { 1, 2, 1, 0, 8, 1, 7, 2 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_NV21] = {
        .name = "nv21",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 1, 0, 0, 8, 0, 7, 1 },        /* Y */
            { 1, 2, 1, 0, 8, 1, 7, 2 },        /* U */
            { 1, 2, 0, 0, 8, 1, 7, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_ARGB] = {
        .name = "argb",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 4, 1, 0, 8, 3, 7, 2 },        /* R */
            { 0, 4, 2, 0, 8, 3, 7, 3 },        /* G */
            { 0, 4, 3, 0, 8, 3, 7, 4 },        /* B */
            { 0, 4, 0, 0, 8, 3, 7, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_RGBA] = {
        .name = "rgba",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 4, 0, 0, 8, 3, 7, 1 },        /* R */
            { 0, 4, 1, 0, 8, 3, 7, 2 },        /* G */
            { 0, 4, 2, 0, 8, 3, 7, 3 },        /* B */
            { 0, 4, 3, 0, 8, 3, 7, 4 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_ABGR] = {
        .name = "abgr",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 4, 3, 0, 8, 3, 7, 4 },        /* R */
            { 0, 4, 2, 0, 8, 3, 7, 3 },        /* G */
            { 0, 4, 1, 0, 8, 3, 7, 2 },        /* B */
            { 0, 4, 0, 0, 8, 3, 7, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_BGRA] = {
        .name = "bgra",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 4, 2, 0, 8, 3, 7, 3 },        /* R */
            { 0, 4, 1, 0, 8, 3, 7, 2 },        /* G */
            { 0, 4, 0, 0, 8, 3, 7, 1 },        /* B */
            { 0, 4, 3, 0, 8, 3, 7, 4 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_0RGB] = {
        .name = "0rgb",
        .nb_components= 3,
        .log2_chroma_w= 0,
        .log2_chroma_h= 0,
        .comp = {
            { 0, 4, 1, 0, 8, 3, 7, 2 },        /* R */
            { 0, 4, 2, 0, 8, 3, 7, 3 },        /* G */
            { 0, 4, 3, 0, 8, 3, 7, 4 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_RGB0] = {
        .name = "rgb0",
        .nb_components= 3,
        .log2_chroma_w= 0,
        .log2_chroma_h= 0,
        .comp = {
            { 0, 4, 0, 0, 8, 3, 7, 1 },        /* R */
            { 0, 4, 1, 0, 8, 3, 7, 2 },        /* G */
            { 0, 4, 2, 0, 8, 3, 7, 3 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_0BGR] = {
        .name = "0bgr",
        .nb_components= 3,
        .log2_chroma_w= 0,
        .log2_chroma_h= 0,
        .comp = {
            { 0, 4, 3, 0, 8, 3, 7, 4 },        /* R */
            { 0, 4, 2, 0, 8, 3, 7, 3 },        /* G */
            { 0, 4, 1, 0, 8, 3, 7, 2 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_BGR0] = {
        .name = "bgr0",
        .nb_components= 3,
        .log2_chroma_w= 0,
        .log2_chroma_h= 0,
        .comp = {
            { 0, 4, 2, 0, 8, 3, 7, 3 },        /* R */
            { 0, 4, 1, 0, 8, 3, 7, 2 },        /* G */
            { 0, 4, 0, 0, 8, 3, 7, 1 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_GRAY9BE] = {
        .name = "gray9be",
        .nb_components = 1,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 9, 1, 8, 1 },       /* Y */
        },
        .flags = AV_PIX_FMT_FLAG_BE,
        .alias = "y9be",
    },
    [AV_PIX_FMT_GRAY9LE] = {
        .name = "gray9le",
        .nb_components = 1,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 9, 1, 8, 1 },       /* Y */
        },
        .alias = "y9le",
    },
    [AV_PIX_FMT_GRAY10BE] = {
        .name = "gray10be",
        .nb_components = 1,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 10, 1, 9, 1 },       /* Y */
        },
        .flags = AV_PIX_FMT_FLAG_BE,
        .alias = "y10be",
    },
    [AV_PIX_FMT_GRAY10LE] = {
        .name = "gray10le",
        .nb_components = 1,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 10, 1, 9, 1 },       /* Y */
        },
        .alias = "y10le",
    },
    [AV_PIX_FMT_GRAY12BE] = {
        .name = "gray12be",
        .nb_components = 1,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 12, 1, 11, 1 },       /* Y */
        },
        .flags = AV_PIX_FMT_FLAG_BE,
        .alias = "y12be",
    },
    [AV_PIX_FMT_GRAY12LE] = {
        .name = "gray12le",
        .nb_components = 1,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 12, 1, 11, 1 },       /* Y */
        },
        .alias = "y12le",
    },
    [AV_PIX_FMT_GRAY14BE] = {
        .name = "gray14be",
        .nb_components = 1,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 14, 1, 13, 1 },       /* Y */
        },
        .flags = AV_PIX_FMT_FLAG_BE,
        .alias = "y14be",
    },
    [AV_PIX_FMT_GRAY14LE] = {
        .name = "gray14le",
        .nb_components = 1,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 14, 1, 13, 1 },       /* Y */
        },
        .alias = "y14le",
    },
    [AV_PIX_FMT_GRAY16BE] = {
        .name = "gray16be",
        .nb_components = 1,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 16, 1, 15, 1 },       /* Y */
        },
        .flags = AV_PIX_FMT_FLAG_BE,
        .alias = "y16be",
    },
    [AV_PIX_FMT_GRAY16LE] = {
        .name = "gray16le",
        .nb_components = 1,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 16, 1, 15, 1 },       /* Y */
        },
        .alias = "y16le",
    },
    [AV_PIX_FMT_YUV440P] = {
        .name = "yuv440p",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 1, 0, 0, 8, 0, 7, 1 },        /* Y */
            { 1, 1, 0, 0, 8, 0, 7, 1 },        /* U */
            { 2, 1, 0, 0, 8, 0, 7, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUVJ440P] = {
        .name = "yuvj440p",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 1, 0, 0, 8, 0, 7, 1 },        /* Y */
            { 1, 1, 0, 0, 8, 0, 7, 1 },        /* U */
            { 2, 1, 0, 0, 8, 0, 7, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV440P10LE] = {
        .name = "yuv440p10le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 2, 0, 0, 10, 1, 9, 1 },        /* Y */
            { 1, 2, 0, 0, 10, 1, 9, 1 },        /* U */
            { 2, 2, 0, 0, 10, 1, 9, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV440P10BE] = {
        .name = "yuv440p10be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 2, 0, 0, 10, 1, 9, 1 },        /* Y */
            { 1, 2, 0, 0, 10, 1, 9, 1 },        /* U */
            { 2, 2, 0, 0, 10, 1, 9, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV440P12LE] = {
        .name = "yuv440p12le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 2, 0, 0, 12, 1, 11, 1 },        /* Y */
            { 1, 2, 0, 0, 12, 1, 11, 1 },        /* U */
            { 2, 2, 0, 0, 12, 1, 11, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV440P12BE] = {
        .name = "yuv440p12be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 2, 0, 0, 12, 1, 11, 1 },        /* Y */
            { 1, 2, 0, 0, 12, 1, 11, 1 },        /* U */
            { 2, 2, 0, 0, 12, 1, 11, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUVA420P] = {
        .name = "yuva420p",
        .nb_components = 4,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 1, 0, 0, 8, 0, 7, 1 },        /* Y */
            { 1, 1, 0, 0, 8, 0, 7, 1 },        /* U */
            { 2, 1, 0, 0, 8, 0, 7, 1 },        /* V */
            { 3, 1, 0, 0, 8, 0, 7, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_YUVA422P] = {
        .name = "yuva422p",
        .nb_components = 4,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 1, 0, 0, 8, 0, 7, 1 },        /* Y */
            { 1, 1, 0, 0, 8, 0, 7, 1 },        /* U */
            { 2, 1, 0, 0, 8, 0, 7, 1 },        /* V */
            { 3, 1, 0, 0, 8, 0, 7, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_YUVA444P] = {
        .name = "yuva444p",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 1, 0, 0, 8, 0, 7, 1 },        /* Y */
            { 1, 1, 0, 0, 8, 0, 7, 1 },        /* U */
            { 2, 1, 0, 0, 8, 0, 7, 1 },        /* V */
            { 3, 1, 0, 0, 8, 0, 7, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_YUVA420P9BE] = {
        .name = "yuva420p9be",
        .nb_components = 4,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 2, 0, 0, 9, 1, 8, 1 },        /* Y */
            { 1, 2, 0, 0, 9, 1, 8, 1 },        /* U */
            { 2, 2, 0, 0, 9, 1, 8, 1 },        /* V */
            { 3, 2, 0, 0, 9, 1, 8, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_YUVA420P9LE] = {
        .name = "yuva420p9le",
        .nb_components = 4,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 2, 0, 0, 9, 1, 8, 1 },        /* Y */
            { 1, 2, 0, 0, 9, 1, 8, 1 },        /* U */
            { 2, 2, 0, 0, 9, 1, 8, 1 },        /* V */
            { 3, 2, 0, 0, 9, 1, 8, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_YUVA422P9BE] = {
        .name = "yuva422p9be",
        .nb_components = 4,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 9, 1, 8, 1 },        /* Y */
            { 1, 2, 0, 0, 9, 1, 8, 1 },        /* U */
            { 2, 2, 0, 0, 9, 1, 8, 1 },        /* V */
            { 3, 2, 0, 0, 9, 1, 8, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_YUVA422P9LE] = {
        .name = "yuva422p9le",
        .nb_components = 4,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 9, 1, 8, 1 },        /* Y */
            { 1, 2, 0, 0, 9, 1, 8, 1 },        /* U */
            { 2, 2, 0, 0, 9, 1, 8, 1 },        /* V */
            { 3, 2, 0, 0, 9, 1, 8, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_YUVA444P9BE] = {
        .name = "yuva444p9be",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 9, 1, 8, 1 },        /* Y */
            { 1, 2, 0, 0, 9, 1, 8, 1 },        /* U */
            { 2, 2, 0, 0, 9, 1, 8, 1 },        /* V */
            { 3, 2, 0, 0, 9, 1, 8, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_YUVA444P9LE] = {
        .name = "yuva444p9le",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 9, 1, 8, 1 },        /* Y */
            { 1, 2, 0, 0, 9, 1, 8, 1 },        /* U */
            { 2, 2, 0, 0, 9, 1, 8, 1 },        /* V */
            { 3, 2, 0, 0, 9, 1, 8, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_YUVA420P10BE] = {
        .name = "yuva420p10be",
        .nb_components = 4,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 2, 0, 0, 10, 1, 9, 1 },        /* Y */
            { 1, 2, 0, 0, 10, 1, 9, 1 },        /* U */
            { 2, 2, 0, 0, 10, 1, 9, 1 },        /* V */
            { 3, 2, 0, 0, 10, 1, 9, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_YUVA420P10LE] = {
        .name = "yuva420p10le",
        .nb_components = 4,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 2, 0, 0, 10, 1, 9, 1 },        /* Y */
            { 1, 2, 0, 0, 10, 1, 9, 1 },        /* U */
            { 2, 2, 0, 0, 10, 1, 9, 1 },        /* V */
            { 3, 2, 0, 0, 10, 1, 9, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_YUVA422P10BE] = {
        .name = "yuva422p10be",
        .nb_components = 4,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 10, 1, 9, 1 },        /* Y */
            { 1, 2, 0, 0, 10, 1, 9, 1 },        /* U */
            { 2, 2, 0, 0, 10, 1, 9, 1 },        /* V */
            { 3, 2, 0, 0, 10, 1, 9, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_YUVA422P10LE] = {
        .name = "yuva422p10le",
        .nb_components = 4,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 10, 1, 9, 1 },        /* Y */
            { 1, 2, 0, 0, 10, 1, 9, 1 },        /* U */
            { 2, 2, 0, 0, 10, 1, 9, 1 },        /* V */
            { 3, 2, 0, 0, 10, 1, 9, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_YUVA444P10BE] = {
        .name = "yuva444p10be",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 10, 1, 9, 1 },        /* Y */
            { 1, 2, 0, 0, 10, 1, 9, 1 },        /* U */
            { 2, 2, 0, 0, 10, 1, 9, 1 },        /* V */
            { 3, 2, 0, 0, 10, 1, 9, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_YUVA444P10LE] = {
        .name = "yuva444p10le",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 10, 1, 9, 1 },        /* Y */
            { 1, 2, 0, 0, 10, 1, 9, 1 },        /* U */
            { 2, 2, 0, 0, 10, 1, 9, 1 },        /* V */
            { 3, 2, 0, 0, 10, 1, 9, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_YUVA420P16BE] = {
        .name = "yuva420p16be",
        .nb_components = 4,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 2, 0, 0, 16, 1, 15, 1 },        /* Y */
            { 1, 2, 0, 0, 16, 1, 15, 1 },        /* U */
            { 2, 2, 0, 0, 16, 1, 15, 1 },        /* V */
            { 3, 2, 0, 0, 16, 1, 15, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_YUVA420P16LE] = {
        .name = "yuva420p16le",
        .nb_components = 4,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 2, 0, 0, 16, 1, 15, 1 },        /* Y */
            { 1, 2, 0, 0, 16, 1, 15, 1 },        /* U */
            { 2, 2, 0, 0, 16, 1, 15, 1 },        /* V */
            { 3, 2, 0, 0, 16, 1, 15, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_YUVA422P16BE] = {
        .name = "yuva422p16be",
        .nb_components = 4,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 16, 1, 15, 1 },        /* Y */
            { 1, 2, 0, 0, 16, 1, 15, 1 },        /* U */
            { 2, 2, 0, 0, 16, 1, 15, 1 },        /* V */
            { 3, 2, 0, 0, 16, 1, 15, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_YUVA422P16LE] = {
        .name = "yuva422p16le",
        .nb_components = 4,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 16, 1, 15, 1 },        /* Y */
            { 1, 2, 0, 0, 16, 1, 15, 1 },        /* U */
            { 2, 2, 0, 0, 16, 1, 15, 1 },        /* V */
            { 3, 2, 0, 0, 16, 1, 15, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_YUVA444P16BE] = {
        .name = "yuva444p16be",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 16, 1, 15, 1 },        /* Y */
            { 1, 2, 0, 0, 16, 1, 15, 1 },        /* U */
            { 2, 2, 0, 0, 16, 1, 15, 1 },        /* V */
            { 3, 2, 0, 0, 16, 1, 15, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_YUVA444P16LE] = {
        .name = "yuva444p16le",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 16, 1, 15, 1 },        /* Y */
            { 1, 2, 0, 0, 16, 1, 15, 1 },        /* U */
            { 2, 2, 0, 0, 16, 1, 15, 1 },        /* V */
            { 3, 2, 0, 0, 16, 1, 15, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_RGB48BE] = {
        .name = "rgb48be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 6, 0, 0, 16, 5, 15, 1 },       /* R */
            { 0, 6, 2, 0, 16, 5, 15, 3 },       /* G */
            { 0, 6, 4, 0, 16, 5, 15, 5 },       /* B */
        },
        .flags = AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_BE,
    },
    [AV_PIX_FMT_RGB48LE] = {
        .name = "rgb48le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 6, 0, 0, 16, 5, 15, 1 },       /* R */
            { 0, 6, 2, 0, 16, 5, 15, 3 },       /* G */
            { 0, 6, 4, 0, 16, 5, 15, 5 },       /* B */
        },
        .flags = AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_RGBA64BE] = {
        .name = "rgba64be",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 8, 0, 0, 16, 7, 15, 1 },       /* R */
            { 0, 8, 2, 0, 16, 7, 15, 3 },       /* G */
            { 0, 8, 4, 0, 16, 7, 15, 5 },       /* B */
            { 0, 8, 6, 0, 16, 7, 15, 7 },       /* A */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_RGBA64LE] = {
        .name = "rgba64le",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 8, 0, 0, 16, 7, 15, 1 },       /* R */
            { 0, 8, 2, 0, 16, 7, 15, 3 },       /* G */
            { 0, 8, 4, 0, 16, 7, 15, 5 },       /* B */
            { 0, 8, 6, 0, 16, 7, 15, 7 },       /* A */
        },
        .flags = AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_RGB565BE] = {
        .name = "rgb565be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, -1, 3, 5, 1, 4, 0 },        /* R */
            { 0, 2,  0, 5, 6, 1, 5, 1 },        /* G */
            { 0, 2,  0, 0, 5, 1, 4, 1 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_RGB565LE] = {
        .name = "rgb565le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 1, 3, 5, 1, 4, 2 },        /* R */
            { 0, 2, 0, 5, 6, 1, 5, 1 },        /* G */
            { 0, 2, 0, 0, 5, 1, 4, 1 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_RGB555BE] = {
        .name = "rgb555be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, -1, 2, 5, 1, 4, 0 },        /* R */
            { 0, 2,  0, 5, 5, 1, 4, 1 },        /* G */
            { 0, 2,  0, 0, 5, 1, 4, 1 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_RGB555LE] = {
        .name = "rgb555le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 1, 2, 5, 1, 4, 2 },        /* R */
            { 0, 2, 0, 5, 5, 1, 4, 1 },        /* G */
            { 0, 2, 0, 0, 5, 1, 4, 1 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_RGB444BE] = {
        .name = "rgb444be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, -1, 0, 4, 1, 3, 0 },        /* R */
            { 0, 2,  0, 4, 4, 1, 3, 1 },        /* G */
            { 0, 2,  0, 0, 4, 1, 3, 1 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_RGB444LE] = {
        .name = "rgb444le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 1, 0, 4, 1, 3, 2 },        /* R */
            { 0, 2, 0, 4, 4, 1, 3, 1 },        /* G */
            { 0, 2, 0, 0, 4, 1, 3, 1 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_BGR48BE] = {
        .name = "bgr48be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 6, 4, 0, 16, 5, 15, 5 },       /* R */
            { 0, 6, 2, 0, 16, 5, 15, 3 },       /* G */
            { 0, 6, 0, 0, 16, 5, 15, 1 },       /* B */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_BGR48LE] = {
        .name = "bgr48le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 6, 4, 0, 16, 5, 15, 5 },       /* R */
            { 0, 6, 2, 0, 16, 5, 15, 3 },       /* G */
            { 0, 6, 0, 0, 16, 5, 15, 1 },       /* B */
        },
        .flags = AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_BGRA64BE] = {
        .name = "bgra64be",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 8, 4, 0, 16, 7, 15, 5 },       /* R */
            { 0, 8, 2, 0, 16, 7, 15, 3 },       /* G */
            { 0, 8, 0, 0, 16, 7, 15, 1 },       /* B */
            { 0, 8, 6, 0, 16, 7, 15, 7 },       /* A */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_BGRA64LE] = {
        .name = "bgra64le",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 8, 4, 0, 16, 7, 15, 5 },       /* R */
            { 0, 8, 2, 0, 16, 7, 15, 3 },       /* G */
            { 0, 8, 0, 0, 16, 7, 15, 1 },       /* B */
            { 0, 8, 6, 0, 16, 7, 15, 7 },       /* A */
        },
        .flags = AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_BGR565BE] = {
        .name = "bgr565be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2,  0, 0, 5, 1, 4, 1 },        /* R */
            { 0, 2,  0, 5, 6, 1, 5, 1 },        /* G */
            { 0, 2, -1, 3, 5, 1, 4, 0 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_BGR565LE] = {
        .name = "bgr565le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 5, 1, 4, 1 },        /* R */
            { 0, 2, 0, 5, 6, 1, 5, 1 },        /* G */
            { 0, 2, 1, 3, 5, 1, 4, 2 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_BGR555BE] = {
        .name = "bgr555be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2,  0, 0, 5, 1, 4, 1 },       /* R */
            { 0, 2,  0, 5, 5, 1, 4, 1 },       /* G */
            { 0, 2, -1, 2, 5, 1, 4, 0 },       /* B */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_RGB,
     },
    [AV_PIX_FMT_BGR555LE] = {
        .name = "bgr555le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 5, 1, 4, 1 },        /* R */
            { 0, 2, 0, 5, 5, 1, 4, 1 },        /* G */
            { 0, 2, 1, 2, 5, 1, 4, 2 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_BGR444BE] = {
        .name = "bgr444be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2,  0, 0, 4, 1, 3, 1 },       /* R */
            { 0, 2,  0, 4, 4, 1, 3, 1 },       /* G */
            { 0, 2, -1, 0, 4, 1, 3, 0 },       /* B */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_RGB,
     },
    [AV_PIX_FMT_BGR444LE] = {
        .name = "bgr444le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 4, 1, 3, 1 },        /* R */
            { 0, 2, 0, 4, 4, 1, 3, 1 },        /* G */
            { 0, 2, 1, 0, 4, 1, 3, 2 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_RGB,
    },
#if FF_API_VAAPI
    [AV_PIX_FMT_VAAPI_MOCO] = {
        .name = "vaapi_moco",
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .flags = AV_PIX_FMT_FLAG_HWACCEL,
    },
    [AV_PIX_FMT_VAAPI_IDCT] = {
        .name = "vaapi_idct",
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .flags = AV_PIX_FMT_FLAG_HWACCEL,
    },
    [AV_PIX_FMT_VAAPI_VLD] = {
        .name = "vaapi_vld",
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .flags = AV_PIX_FMT_FLAG_HWACCEL,
    },
#else
    [AV_PIX_FMT_VAAPI] = {
        .name = "vaapi",
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .flags = AV_PIX_FMT_FLAG_HWACCEL,
    },
#endif
    [AV_PIX_FMT_YUV420P9LE] = {
        .name = "yuv420p9le",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 2, 0, 0, 9, 1, 8, 1 },        /* Y */
            { 1, 2, 0, 0, 9, 1, 8, 1 },        /* U */
            { 2, 2, 0, 0, 9, 1, 8, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV420P9BE] = {
        .name = "yuv420p9be",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 2, 0, 0, 9, 1, 8, 1 },        /* Y */
            { 1, 2, 0, 0, 9, 1, 8, 1 },        /* U */
            { 2, 2, 0, 0, 9, 1, 8, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV420P10LE] = {
        .name = "yuv420p10le",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 2, 0, 0, 10, 1, 9, 1 },        /* Y */
            { 1, 2, 0, 0, 10, 1, 9, 1 },        /* U */
            { 2, 2, 0, 0, 10, 1, 9, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV420P10BE] = {
        .name = "yuv420p10be",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 2, 0, 0, 10, 1, 9, 1 },        /* Y */
            { 1, 2, 0, 0, 10, 1, 9, 1 },        /* U */
            { 2, 2, 0, 0, 10, 1, 9, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV420P12LE] = {
        .name = "yuv420p12le",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 2, 0, 0, 12, 1, 11, 1 },        /* Y */
            { 1, 2, 0, 0, 12, 1, 11, 1 },        /* U */
            { 2, 2, 0, 0, 12, 1, 11, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV420P12BE] = {
        .name = "yuv420p12be",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 2, 0, 0, 12, 1, 11, 1 },        /* Y */
            { 1, 2, 0, 0, 12, 1, 11, 1 },        /* U */
            { 2, 2, 0, 0, 12, 1, 11, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV420P14LE] = {
        .name = "yuv420p14le",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 2, 0, 0, 14, 1, 13, 1 },        /* Y */
            { 1, 2, 0, 0, 14, 1, 13, 1 },        /* U */
            { 2, 2, 0, 0, 14, 1, 13, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV420P14BE] = {
        .name = "yuv420p14be",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 2, 0, 0, 14, 1, 13, 1 },        /* Y */
            { 1, 2, 0, 0, 14, 1, 13, 1 },        /* U */
            { 2, 2, 0, 0, 14, 1, 13, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV420P16LE] = {
        .name = "yuv420p16le",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 2, 0, 0, 16, 1, 15, 1 },        /* Y */
            { 1, 2, 0, 0, 16, 1, 15, 1 },        /* U */
            { 2, 2, 0, 0, 16, 1, 15, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV420P16BE] = {
        .name = "yuv420p16be",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 2, 0, 0, 16, 1, 15, 1 },        /* Y */
            { 1, 2, 0, 0, 16, 1, 15, 1 },        /* U */
            { 2, 2, 0, 0, 16, 1, 15, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV422P9LE] = {
        .name = "yuv422p9le",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 9, 1, 8, 1 },        /* Y */
            { 1, 2, 0, 0, 9, 1, 8, 1 },        /* U */
            { 2, 2, 0, 0, 9, 1, 8, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV422P9BE] = {
        .name = "yuv422p9be",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 9, 1, 8, 1 },        /* Y */
            { 1, 2, 0, 0, 9, 1, 8, 1 },        /* U */
            { 2, 2, 0, 0, 9, 1, 8, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV422P10LE] = {
        .name = "yuv422p10le",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 10, 1, 9, 1 },        /* Y */
            { 1, 2, 0, 0, 10, 1, 9, 1 },        /* U */
            { 2, 2, 0, 0, 10, 1, 9, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV422P10BE] = {
        .name = "yuv422p10be",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 10, 1, 9, 1 },        /* Y */
            { 1, 2, 0, 0, 10, 1, 9, 1 },        /* U */
            { 2, 2, 0, 0, 10, 1, 9, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV422P12LE] = {
        .name = "yuv422p12le",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 12, 1, 11, 1 },        /* Y */
            { 1, 2, 0, 0, 12, 1, 11, 1 },        /* U */
            { 2, 2, 0, 0, 12, 1, 11, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV422P12BE] = {
        .name = "yuv422p12be",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 12, 1, 11, 1 },        /* Y */
            { 1, 2, 0, 0, 12, 1, 11, 1 },        /* U */
            { 2, 2, 0, 0, 12, 1, 11, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV422P14LE] = {
        .name = "yuv422p14le",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 14, 1, 13, 1 },        /* Y */
            { 1, 2, 0, 0, 14, 1, 13, 1 },        /* U */
            { 2, 2, 0, 0, 14, 1, 13, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV422P14BE] = {
        .name = "yuv422p14be",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 14, 1, 13, 1 },        /* Y */
            { 1, 2, 0, 0, 14, 1, 13, 1 },        /* U */
            { 2, 2, 0, 0, 14, 1, 13, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV422P16LE] = {
        .name = "yuv422p16le",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 16, 1, 15, 1 },        /* Y */
            { 1, 2, 0, 0, 16, 1, 15, 1 },        /* U */
            { 2, 2, 0, 0, 16, 1, 15, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV422P16BE] = {
        .name = "yuv422p16be",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 16, 1, 15, 1 },        /* Y */
            { 1, 2, 0, 0, 16, 1, 15, 1 },        /* U */
            { 2, 2, 0, 0, 16, 1, 15, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV444P16LE] = {
        .name = "yuv444p16le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 16, 1, 15, 1 },        /* Y */
            { 1, 2, 0, 0, 16, 1, 15, 1 },        /* U */
            { 2, 2, 0, 0, 16, 1, 15, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV444P16BE] = {
        .name = "yuv444p16be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 16, 1, 15, 1 },        /* Y */
            { 1, 2, 0, 0, 16, 1, 15, 1 },        /* U */
            { 2, 2, 0, 0, 16, 1, 15, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV444P10LE] = {
        .name = "yuv444p10le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 10, 1, 9, 1 },        /* Y */
            { 1, 2, 0, 0, 10, 1, 9, 1 },        /* U */
            { 2, 2, 0, 0, 10, 1, 9, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV444P10BE] = {
        .name = "yuv444p10be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 10, 1, 9, 1 },        /* Y */
            { 1, 2, 0, 0, 10, 1, 9, 1 },        /* U */
            { 2, 2, 0, 0, 10, 1, 9, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV444P9LE] = {
        .name = "yuv444p9le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 9, 1, 8, 1 },        /* Y */
            { 1, 2, 0, 0, 9, 1, 8, 1 },        /* U */
            { 2, 2, 0, 0, 9, 1, 8, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV444P9BE] = {
        .name = "yuv444p9be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 9, 1, 8, 1 },        /* Y */
            { 1, 2, 0, 0, 9, 1, 8, 1 },        /* U */
            { 2, 2, 0, 0, 9, 1, 8, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV444P12LE] = {
        .name = "yuv444p12le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 12, 1, 11, 1 },        /* Y */
            { 1, 2, 0, 0, 12, 1, 11, 1 },        /* U */
            { 2, 2, 0, 0, 12, 1, 11, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV444P12BE] = {
        .name = "yuv444p12be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 12, 1, 11, 1 },        /* Y */
            { 1, 2, 0, 0, 12, 1, 11, 1 },        /* U */
            { 2, 2, 0, 0, 12, 1, 11, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV444P14LE] = {
        .name = "yuv444p14le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 14, 1, 13, 1 },        /* Y */
            { 1, 2, 0, 0, 14, 1, 13, 1 },        /* U */
            { 2, 2, 0, 0, 14, 1, 13, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_YUV444P14BE] = {
        .name = "yuv444p14be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 14, 1, 13, 1 },        /* Y */
            { 1, 2, 0, 0, 14, 1, 13, 1 },        /* U */
            { 2, 2, 0, 0, 14, 1, 13, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_D3D11VA_VLD] = {
        .name = "d3d11va_vld",
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .flags = AV_PIX_FMT_FLAG_HWACCEL,
    },
    [AV_PIX_FMT_DXVA2_VLD] = {
        .name = "dxva2_vld",
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .flags = AV_PIX_FMT_FLAG_HWACCEL,
    },
    [AV_PIX_FMT_YA8] = {
        .name = "ya8",
        .nb_components = 2,
        .comp = {
            { 0, 2, 0, 0, 8, 1, 7, 1 },        /* Y */
            { 0, 2, 1, 0, 8, 1, 7, 2 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_ALPHA,
        .alias = "gray8a",
    },
    [AV_PIX_FMT_YA16LE] = {
        .name = "ya16le",
        .nb_components = 2,
        .comp = {
            { 0, 4, 0, 0, 16, 3, 15, 1 },        /* Y */
            { 0, 4, 2, 0, 16, 3, 15, 3 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_YA16BE] = {
        .name = "ya16be",
        .nb_components = 2,
        .comp = {
            { 0, 4, 0, 0, 16, 3, 15, 1 },        /* Y */
            { 0, 4, 2, 0, 16, 3, 15, 3 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_VIDEOTOOLBOX] = {
        .name = "videotoolbox_vld",
        .flags = AV_PIX_FMT_FLAG_HWACCEL,
    },
    [AV_PIX_FMT_GBRP] = {
        .name = "gbrp",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 2, 1, 0, 0, 8, 0, 7, 1 },        /* R */
            { 0, 1, 0, 0, 8, 0, 7, 1 },        /* G */
            { 1, 1, 0, 0, 8, 0, 7, 1 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_GBRP9LE] = {
        .name = "gbrp9le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 2, 2, 0, 0, 9, 1, 8, 1 },        /* R */
            { 0, 2, 0, 0, 9, 1, 8, 1 },        /* G */
            { 1, 2, 0, 0, 9, 1, 8, 1 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_GBRP9BE] = {
        .name = "gbrp9be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 2, 2, 0, 0, 9, 1, 8, 1 },        /* R */
            { 0, 2, 0, 0, 9, 1, 8, 1 },        /* G */
            { 1, 2, 0, 0, 9, 1, 8, 1 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_GBRP10LE] = {
        .name = "gbrp10le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 2, 2, 0, 0, 10, 1, 9, 1 },        /* R */
            { 0, 2, 0, 0, 10, 1, 9, 1 },        /* G */
            { 1, 2, 0, 0, 10, 1, 9, 1 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_GBRP10BE] = {
        .name = "gbrp10be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 2, 2, 0, 0, 10, 1, 9, 1 },        /* R */
            { 0, 2, 0, 0, 10, 1, 9, 1 },        /* G */
            { 1, 2, 0, 0, 10, 1, 9, 1 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_GBRP12LE] = {
        .name = "gbrp12le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 2, 2, 0, 0, 12, 1, 11, 1 },        /* R */
            { 0, 2, 0, 0, 12, 1, 11, 1 },        /* G */
            { 1, 2, 0, 0, 12, 1, 11, 1 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_GBRP12BE] = {
        .name = "gbrp12be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 2, 2, 0, 0, 12, 1, 11, 1 },        /* R */
            { 0, 2, 0, 0, 12, 1, 11, 1 },        /* G */
            { 1, 2, 0, 0, 12, 1, 11, 1 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_GBRP14LE] = {
        .name = "gbrp14le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 2, 2, 0, 0, 14, 1, 13, 1 },        /* R */
            { 0, 2, 0, 0, 14, 1, 13, 1 },        /* G */
            { 1, 2, 0, 0, 14, 1, 13, 1 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_GBRP14BE] = {
        .name = "gbrp14be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 2, 2, 0, 0, 14, 1, 13, 1 },        /* R */
            { 0, 2, 0, 0, 14, 1, 13, 1 },        /* G */
            { 1, 2, 0, 0, 14, 1, 13, 1 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_GBRP16LE] = {
        .name = "gbrp16le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 2, 2, 0, 0, 16, 1, 15, 1 },       /* R */
            { 0, 2, 0, 0, 16, 1, 15, 1 },       /* G */
            { 1, 2, 0, 0, 16, 1, 15, 1 },       /* B */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_GBRP16BE] = {
        .name = "gbrp16be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 2, 2, 0, 0, 16, 1, 15, 1 },       /* R */
            { 0, 2, 0, 0, 16, 1, 15, 1 },       /* G */
            { 1, 2, 0, 0, 16, 1, 15, 1 },       /* B */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_GBRAP] = {
        .name = "gbrap",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 2, 1, 0, 0, 8, 0, 7, 1 },        /* R */
            { 0, 1, 0, 0, 8, 0, 7, 1 },        /* G */
            { 1, 1, 0, 0, 8, 0, 7, 1 },        /* B */
            { 3, 1, 0, 0, 8, 0, 7, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_RGB |
                 AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_GBRAP16LE] = {
        .name = "gbrap16le",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 2, 2, 0, 0, 16, 1, 15, 1 },       /* R */
            { 0, 2, 0, 0, 16, 1, 15, 1 },       /* G */
            { 1, 2, 0, 0, 16, 1, 15, 1 },       /* B */
            { 3, 2, 0, 0, 16, 1, 15, 1 },       /* A */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_RGB |
                 AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_GBRAP16BE] = {
        .name = "gbrap16be",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 2, 2, 0, 0, 16, 1, 15, 1 },       /* R */
            { 0, 2, 0, 0, 16, 1, 15, 1 },       /* G */
            { 1, 2, 0, 0, 16, 1, 15, 1 },       /* B */
            { 3, 2, 0, 0, 16, 1, 15, 1 },       /* A */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR |
                 AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_VDPAU] = {
        .name = "vdpau",
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .flags = AV_PIX_FMT_FLAG_HWACCEL,
    },
    [AV_PIX_FMT_XYZ12LE] = {
        .name = "xyz12le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 6, 0, 4, 12, 5, 11, 1 },       /* X */
            { 0, 6, 2, 4, 12, 5, 11, 3 },       /* Y */
            { 0, 6, 4, 4, 12, 5, 11, 5 },       /* Z */
      },
      /*.flags = -- not used*/
    },
    [AV_PIX_FMT_XYZ12BE] = {
        .name = "xyz12be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 6, 0, 4, 12, 5, 11, 1 },       /* X */
            { 0, 6, 2, 4, 12, 5, 11, 3 },       /* Y */
            { 0, 6, 4, 4, 12, 5, 11, 5 },       /* Z */
       },
        .flags = AV_PIX_FMT_FLAG_BE,
    },

#define BAYER8_DESC_COMMON \
        .nb_components= 3, \
        .log2_chroma_w= 0, \
        .log2_chroma_h= 0, \
        .comp = {          \
            {0,1,0,0,2,0,1,1},\
            {0,1,0,0,4,0,3,1},\
            {0,1,0,0,2,0,1,1},\
        },                 \

#define BAYER16_DESC_COMMON \
        .nb_components= 3, \
        .log2_chroma_w= 0, \
        .log2_chroma_h= 0, \
        .comp = {          \
            {0,2,0,0,4,1,3,1},\
            {0,2,0,0,8,1,7,1},\
            {0,2,0,0,4,1,3,1},\
        },                 \

    [AV_PIX_FMT_BAYER_BGGR8] = {
        .name = "bayer_bggr8",
        BAYER8_DESC_COMMON
        .flags = AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_BAYER,
    },
    [AV_PIX_FMT_BAYER_BGGR16LE] = {
        .name = "bayer_bggr16le",
        BAYER16_DESC_COMMON
        .flags = AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_BAYER,
    },
    [AV_PIX_FMT_BAYER_BGGR16BE] = {
        .name = "bayer_bggr16be",
        BAYER16_DESC_COMMON
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_BAYER,
    },
    [AV_PIX_FMT_BAYER_RGGB8] = {
        .name = "bayer_rggb8",
        BAYER8_DESC_COMMON
        .flags = AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_BAYER,
    },
    [AV_PIX_FMT_BAYER_RGGB16LE] = {
        .name = "bayer_rggb16le",
        BAYER16_DESC_COMMON
        .flags = AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_BAYER,
    },
    [AV_PIX_FMT_BAYER_RGGB16BE] = {
        .name = "bayer_rggb16be",
        BAYER16_DESC_COMMON
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_BAYER,
    },
    [AV_PIX_FMT_BAYER_GBRG8] = {
        .name = "bayer_gbrg8",
        BAYER8_DESC_COMMON
        .flags = AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_BAYER,
    },
    [AV_PIX_FMT_BAYER_GBRG16LE] = {
        .name = "bayer_gbrg16le",
        BAYER16_DESC_COMMON
        .flags = AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_BAYER,
    },
    [AV_PIX_FMT_BAYER_GBRG16BE] = {
        .name = "bayer_gbrg16be",
        BAYER16_DESC_COMMON
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_BAYER,
    },
    [AV_PIX_FMT_BAYER_GRBG8] = {
        .name = "bayer_grbg8",
        BAYER8_DESC_COMMON
        .flags = AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_BAYER,
    },
    [AV_PIX_FMT_BAYER_GRBG16LE] = {
        .name = "bayer_grbg16le",
        BAYER16_DESC_COMMON
        .flags = AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_BAYER,
    },
    [AV_PIX_FMT_BAYER_GRBG16BE] = {
        .name = "bayer_grbg16be",
        BAYER16_DESC_COMMON
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_BAYER,
    },
    [AV_PIX_FMT_NV16] = {
        .name = "nv16",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 1, 0, 0, 8, 0, 7, 1 },        /* Y */
            { 1, 2, 0, 0, 8, 1, 7, 1 },        /* U */
            { 1, 2, 1, 0, 8, 1, 7, 2 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_NV20LE] = {
        .name = "nv20le",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 10, 1, 9, 1 },        /* Y */
            { 1, 4, 0, 0, 10, 3, 9, 1 },        /* U */
            { 1, 4, 2, 0, 10, 3, 9, 3 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_NV20BE] = {
        .name = "nv20be",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 10, 1, 9, 1 },        /* Y */
            { 1, 4, 0, 0, 10, 3, 9, 1 },        /* U */
            { 1, 4, 2, 0, 10, 3, 9, 3 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_BE,
    },
    [AV_PIX_FMT_QSV] = {
        .name = "qsv",
        .flags = AV_PIX_FMT_FLAG_HWACCEL,
    },
    [AV_PIX_FMT_MEDIACODEC] = {
        .name = "mediacodec",
        .flags = AV_PIX_FMT_FLAG_HWACCEL,
    },
    [AV_PIX_FMT_MMAL] = {
        .name = "mmal",
        .flags = AV_PIX_FMT_FLAG_HWACCEL,
    },
    [AV_PIX_FMT_CUDA] = {
        .name = "cuda",
        .flags = AV_PIX_FMT_FLAG_HWACCEL,
    },
    [AV_PIX_FMT_AYUV64LE] = {
        .name = "ayuv64le",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 8, 2, 0, 16, 7, 15, 3 },        /* Y */
            { 0, 8, 4, 0, 16, 7, 15, 5 },        /* U */
            { 0, 8, 6, 0, 16, 7, 15, 7 },        /* V */
            { 0, 8, 0, 0, 16, 7, 15, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_AYUV64BE] = {
        .name = "ayuv64be",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 8, 2, 0, 16, 7, 15, 3 },        /* Y */
            { 0, 8, 4, 0, 16, 7, 15, 5 },        /* U */
            { 0, 8, 6, 0, 16, 7, 15, 7 },        /* V */
            { 0, 8, 0, 0, 16, 7, 15, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_P010LE] = {
        .name = "p010le",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 2, 0, 6, 10, 1, 9, 1 },        /* Y */
            { 1, 4, 0, 6, 10, 3, 9, 1 },        /* U */
            { 1, 4, 2, 6, 10, 3, 9, 3 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_P010BE] = {
        .name = "p010be",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 2, 0, 6, 10, 1, 9, 1 },        /* Y */
            { 1, 4, 0, 6, 10, 3, 9, 1 },        /* U */
            { 1, 4, 2, 6, 10, 3, 9, 3 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_BE,
    },
    [AV_PIX_FMT_P016LE] = {
        .name = "p016le",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 2, 0, 0, 16, 1, 15, 1 },       /* Y */
            { 1, 4, 0, 0, 16, 3, 15, 1 },       /* U */
            { 1, 4, 2, 0, 16, 3, 15, 3 },       /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_P016BE] = {
        .name = "p016be",
        .nb_components = 3,
        .log2_chroma_w = 1,
        .log2_chroma_h = 1,
        .comp = {
            { 0, 2, 0, 0, 16, 1, 15, 1 },       /* Y */
            { 1, 4, 0, 0, 16, 3, 15, 1 },       /* U */
            { 1, 4, 2, 0, 16, 3, 15, 3 },       /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_BE,
    },
    [AV_PIX_FMT_GBRAP12LE] = {
        .name = "gbrap12le",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 2, 2, 0, 0, 12, 1, 11, 1 },       /* R */
            { 0, 2, 0, 0, 12, 1, 11, 1 },       /* G */
            { 1, 2, 0, 0, 12, 1, 11, 1 },       /* B */
            { 3, 2, 0, 0, 12, 1, 11, 1 },       /* A */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_RGB |
                 AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_GBRAP12BE] = {
        .name = "gbrap12be",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 2, 2, 0, 0, 12, 1, 11, 1 },       /* R */
            { 0, 2, 0, 0, 12, 1, 11, 1 },       /* G */
            { 1, 2, 0, 0, 12, 1, 11, 1 },       /* B */
            { 3, 2, 0, 0, 12, 1, 11, 1 },       /* A */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR |
                 AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_GBRAP10LE] = {
        .name = "gbrap10le",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 2, 2, 0, 0, 10, 1, 9, 1 },       /* R */
            { 0, 2, 0, 0, 10, 1, 9, 1 },       /* G */
            { 1, 2, 0, 0, 10, 1, 9, 1 },       /* B */
            { 3, 2, 0, 0, 10, 1, 9, 1 },       /* A */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_RGB |
                 AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_GBRAP10BE] = {
        .name = "gbrap10be",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 2, 2, 0, 0, 10, 1, 9, 1 },       /* R */
            { 0, 2, 0, 0, 10, 1, 9, 1 },       /* G */
            { 1, 2, 0, 0, 10, 1, 9, 1 },       /* B */
            { 3, 2, 0, 0, 10, 1, 9, 1 },       /* A */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR |
                 AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_D3D11] = {
        .name = "d3d11",
        .flags = AV_PIX_FMT_FLAG_HWACCEL,
    },
    [AV_PIX_FMT_GBRPF32BE] = {
        .name = "gbrpf32be",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 2, 4, 0, 0, 32, 3, 31, 1 },        /* R */
            { 0, 4, 0, 0, 32, 3, 31, 1 },        /* G */
            { 1, 4, 0, 0, 32, 3, 31, 1 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR |
                 AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_FLOAT,
    },
    [AV_PIX_FMT_GBRPF32LE] = {
        .name = "gbrpf32le",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 2, 4, 0, 0, 32, 3, 31, 1 },        /* R */
            { 0, 4, 0, 0, 32, 3, 31, 1 },        /* G */
            { 1, 4, 0, 0, 32, 3, 31, 1 },        /* B */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_FLOAT | AV_PIX_FMT_FLAG_RGB,
    },
    [AV_PIX_FMT_GBRAPF32BE] = {
        .name = "gbrapf32be",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 2, 4, 0, 0, 32, 3, 31, 1 },        /* R */
            { 0, 4, 0, 0, 32, 3, 31, 1 },        /* G */
            { 1, 4, 0, 0, 32, 3, 31, 1 },        /* B */
            { 3, 4, 0, 0, 32, 3, 31, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR |
                 AV_PIX_FMT_FLAG_ALPHA | AV_PIX_FMT_FLAG_RGB |
                 AV_PIX_FMT_FLAG_FLOAT,
    },
    [AV_PIX_FMT_GBRAPF32LE] = {
        .name = "gbrapf32le",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 2, 4, 0, 0, 32, 3, 31, 1 },        /* R */
            { 0, 4, 0, 0, 32, 3, 31, 1 },        /* G */
            { 1, 4, 0, 0, 32, 3, 31, 1 },        /* B */
            { 3, 4, 0, 0, 32, 3, 31, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA |
                 AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_FLOAT,
    },
    [AV_PIX_FMT_DRM_PRIME] = {
        .name = "drm_prime",
        .flags = AV_PIX_FMT_FLAG_HWACCEL,
    },
    [AV_PIX_FMT_OPENCL] = {
        .name  = "opencl",
        .flags = AV_PIX_FMT_FLAG_HWACCEL,
    },
    [AV_PIX_FMT_GRAYF32BE] = {
        .name = "grayf32be",
        .nb_components = 1,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 4, 0, 0, 32, 3, 31, 1 },       /* Y */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_FLOAT,
        .alias = "yf32be",
    },
    [AV_PIX_FMT_GRAYF32LE] = {
        .name = "grayf32le",
        .nb_components = 1,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 4, 0, 0, 32, 3, 31, 1 },       /* Y */
        },
        .flags = AV_PIX_FMT_FLAG_FLOAT,
        .alias = "yf32le",
    },
    [AV_PIX_FMT_YUVA422P12BE] = {
        .name = "yuva422p12be",
        .nb_components = 4,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 12, 1, 11, 1 },        /* Y */
            { 1, 2, 0, 0, 12, 1, 11, 1 },        /* U */
            { 2, 2, 0, 0, 12, 1, 11, 1 },        /* V */
            { 3, 2, 0, 0, 12, 1, 11, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_YUVA422P12LE] = {
        .name = "yuva422p12le",
        .nb_components = 4,
        .log2_chroma_w = 1,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 12, 1, 11, 1 },        /* Y */
            { 1, 2, 0, 0, 12, 1, 11, 1 },        /* U */
            { 2, 2, 0, 0, 12, 1, 11, 1 },        /* V */
            { 3, 2, 0, 0, 12, 1, 11, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_YUVA444P12BE] = {
        .name = "yuva444p12be",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 12, 1, 11, 1 },        /* Y */
            { 1, 2, 0, 0, 12, 1, 11, 1 },        /* U */
            { 2, 2, 0, 0, 12, 1, 11, 1 },        /* V */
            { 3, 2, 0, 0, 12, 1, 11, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_BE | AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_YUVA444P12LE] = {
        .name = "yuva444p12le",
        .nb_components = 4,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 2, 0, 0, 12, 1, 11, 1 },        /* Y */
            { 1, 2, 0, 0, 12, 1, 11, 1 },        /* U */
            { 2, 2, 0, 0, 12, 1, 11, 1 },        /* V */
            { 3, 2, 0, 0, 12, 1, 11, 1 },        /* A */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_ALPHA,
    },
    [AV_PIX_FMT_NV24] = {
        .name = "nv24",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 1, 0, 0, 8, 0, 7, 1 },        /* Y */
            { 1, 2, 0, 0, 8, 1, 7, 1 },        /* U */
            { 1, 2, 1, 0, 8, 1, 7, 2 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_NV42] = {
        .name = "nv42",
        .nb_components = 3,
        .log2_chroma_w = 0,
        .log2_chroma_h = 0,
        .comp = {
            { 0, 1, 0, 0, 8, 0, 7, 1 },        /* Y */
            { 1, 2, 1, 0, 8, 1, 7, 2 },        /* U */
            { 1, 2, 0, 0, 8, 1, 7, 1 },        /* V */
        },
        .flags = AV_PIX_FMT_FLAG_PLANAR,
    },
    [AV_PIX_FMT_VULKAN] = {
        .name = "vulkan",
        .flags = AV_PIX_FMT_FLAG_HWACCEL,
    },
};


const char *av_get_pix_fmt_name(enum AVPixelFormat pix_fmt)
{
    return (unsigned)pix_fmt < AV_PIX_FMT_NB ?
        av_pix_fmt_descriptors[pix_fmt].name : NULL;
}
static atomic_int cpu_flags = ATOMIC_VAR_INIT(-1);
int av_get_cpu_flags(void)
{
    int flags = atomic_load_explicit(&cpu_flags, memory_order_relaxed);
    if (flags == -1) {
        flags = get_cpu_flags();
        atomic_store_explicit(&cpu_flags, flags, memory_order_relaxed);
    }
    return flags;
}

int av_get_bits_per_pixel(const AVPixFmtDescriptor *pixdesc)
{
    int c, bits = 0;
    int log2_pixels = pixdesc->log2_chroma_w + pixdesc->log2_chroma_h;

    for (c = 0; c < pixdesc->nb_components; c++) {
        int s = c == 1 || c == 2 ? 0 : log2_pixels;
        bits += pixdesc->comp[c].depth << s;
    }

    return bits >> log2_pixels;
}

typedef struct FormatEntry {
    uint8_t is_supported_in         :1;
    uint8_t is_supported_out        :1;
    uint8_t is_supported_endianness :1;
} FormatEntry;

static const FormatEntry format_entries[] = {
    [AV_PIX_FMT_YUV420P]     = { 1, 1 },
    [AV_PIX_FMT_YUYV422]     = { 1, 1 },
    [AV_PIX_FMT_RGB24]       = { 1, 1 },
    [AV_PIX_FMT_BGR24]       = { 1, 1 },
    [AV_PIX_FMT_YUV422P]     = { 1, 1 },
    [AV_PIX_FMT_YUV444P]     = { 1, 1 },
    [AV_PIX_FMT_YUV410P]     = { 1, 1 },
    [AV_PIX_FMT_YUV411P]     = { 1, 1 },
    [AV_PIX_FMT_GRAY8]       = { 1, 1 },
    [AV_PIX_FMT_MONOWHITE]   = { 1, 1 },
    [AV_PIX_FMT_MONOBLACK]   = { 1, 1 },
    [AV_PIX_FMT_PAL8]        = { 1, 0 },
    [AV_PIX_FMT_YUVJ420P]    = { 1, 1 },
    [AV_PIX_FMT_YUVJ411P]    = { 1, 1 },
    [AV_PIX_FMT_YUVJ422P]    = { 1, 1 },
    [AV_PIX_FMT_YUVJ444P]    = { 1, 1 },
    [AV_PIX_FMT_YVYU422]     = { 1, 1 },
    [AV_PIX_FMT_UYVY422]     = { 1, 1 },
    [AV_PIX_FMT_UYYVYY411]   = { 0, 0 },
    [AV_PIX_FMT_BGR8]        = { 1, 1 },
    [AV_PIX_FMT_BGR4]        = { 0, 1 },
    [AV_PIX_FMT_BGR4_BYTE]   = { 1, 1 },
    [AV_PIX_FMT_RGB8]        = { 1, 1 },
    [AV_PIX_FMT_RGB4]        = { 0, 1 },
    [AV_PIX_FMT_RGB4_BYTE]   = { 1, 1 },
    [AV_PIX_FMT_NV12]        = { 1, 1 },
    [AV_PIX_FMT_NV21]        = { 1, 1 },
    [AV_PIX_FMT_ARGB]        = { 1, 1 },
    [AV_PIX_FMT_RGBA]        = { 1, 1 },
    [AV_PIX_FMT_ABGR]        = { 1, 1 },
    [AV_PIX_FMT_BGRA]        = { 1, 1 },
    [AV_PIX_FMT_0RGB]        = { 1, 1 },
    [AV_PIX_FMT_RGB0]        = { 1, 1 },
    [AV_PIX_FMT_0BGR]        = { 1, 1 },
    [AV_PIX_FMT_BGR0]        = { 1, 1 },
    [AV_PIX_FMT_GRAY9BE]     = { 1, 1 },
    [AV_PIX_FMT_GRAY9LE]     = { 1, 1 },
    [AV_PIX_FMT_GRAY10BE]    = { 1, 1 },
    [AV_PIX_FMT_GRAY10LE]    = { 1, 1 },
    [AV_PIX_FMT_GRAY12BE]    = { 1, 1 },
    [AV_PIX_FMT_GRAY12LE]    = { 1, 1 },
    [AV_PIX_FMT_GRAY14BE]    = { 1, 1 },
    [AV_PIX_FMT_GRAY14LE]    = { 1, 1 },
    [AV_PIX_FMT_GRAY16BE]    = { 1, 1 },
    [AV_PIX_FMT_GRAY16LE]    = { 1, 1 },
    [AV_PIX_FMT_YUV440P]     = { 1, 1 },
    [AV_PIX_FMT_YUVJ440P]    = { 1, 1 },
    [AV_PIX_FMT_YUV440P10LE] = { 1, 1 },
    [AV_PIX_FMT_YUV440P10BE] = { 1, 1 },
    [AV_PIX_FMT_YUV440P12LE] = { 1, 1 },
    [AV_PIX_FMT_YUV440P12BE] = { 1, 1 },
    [AV_PIX_FMT_YUVA420P]    = { 1, 1 },
    [AV_PIX_FMT_YUVA422P]    = { 1, 1 },
    [AV_PIX_FMT_YUVA444P]    = { 1, 1 },
    [AV_PIX_FMT_YUVA420P9BE] = { 1, 1 },
    [AV_PIX_FMT_YUVA420P9LE] = { 1, 1 },
    [AV_PIX_FMT_YUVA422P9BE] = { 1, 1 },
    [AV_PIX_FMT_YUVA422P9LE] = { 1, 1 },
    [AV_PIX_FMT_YUVA444P9BE] = { 1, 1 },
    [AV_PIX_FMT_YUVA444P9LE] = { 1, 1 },
    [AV_PIX_FMT_YUVA420P10BE]= { 1, 1 },
    [AV_PIX_FMT_YUVA420P10LE]= { 1, 1 },
    [AV_PIX_FMT_YUVA422P10BE]= { 1, 1 },
    [AV_PIX_FMT_YUVA422P10LE]= { 1, 1 },
    [AV_PIX_FMT_YUVA444P10BE]= { 1, 1 },
    [AV_PIX_FMT_YUVA444P10LE]= { 1, 1 },
    [AV_PIX_FMT_YUVA420P16BE]= { 1, 1 },
    [AV_PIX_FMT_YUVA420P16LE]= { 1, 1 },
    [AV_PIX_FMT_YUVA422P16BE]= { 1, 1 },
    [AV_PIX_FMT_YUVA422P16LE]= { 1, 1 },
    [AV_PIX_FMT_YUVA444P16BE]= { 1, 1 },
    [AV_PIX_FMT_YUVA444P16LE]= { 1, 1 },
    [AV_PIX_FMT_RGB48BE]     = { 1, 1 },
    [AV_PIX_FMT_RGB48LE]     = { 1, 1 },
    [AV_PIX_FMT_RGBA64BE]    = { 1, 1, 1 },
    [AV_PIX_FMT_RGBA64LE]    = { 1, 1, 1 },
    [AV_PIX_FMT_RGB565BE]    = { 1, 1 },
    [AV_PIX_FMT_RGB565LE]    = { 1, 1 },
    [AV_PIX_FMT_RGB555BE]    = { 1, 1 },
    [AV_PIX_FMT_RGB555LE]    = { 1, 1 },
    [AV_PIX_FMT_BGR565BE]    = { 1, 1 },
    [AV_PIX_FMT_BGR565LE]    = { 1, 1 },
    [AV_PIX_FMT_BGR555BE]    = { 1, 1 },
    [AV_PIX_FMT_BGR555LE]    = { 1, 1 },
    [AV_PIX_FMT_YUV420P16LE] = { 1, 1 },
    [AV_PIX_FMT_YUV420P16BE] = { 1, 1 },
    [AV_PIX_FMT_YUV422P16LE] = { 1, 1 },
    [AV_PIX_FMT_YUV422P16BE] = { 1, 1 },
    [AV_PIX_FMT_YUV444P16LE] = { 1, 1 },
    [AV_PIX_FMT_YUV444P16BE] = { 1, 1 },
    [AV_PIX_FMT_RGB444LE]    = { 1, 1 },
    [AV_PIX_FMT_RGB444BE]    = { 1, 1 },
    [AV_PIX_FMT_BGR444LE]    = { 1, 1 },
    [AV_PIX_FMT_BGR444BE]    = { 1, 1 },
    [AV_PIX_FMT_YA8]         = { 1, 1 },
    [AV_PIX_FMT_YA16BE]      = { 1, 1 },
    [AV_PIX_FMT_YA16LE]      = { 1, 1 },
    [AV_PIX_FMT_BGR48BE]     = { 1, 1 },
    [AV_PIX_FMT_BGR48LE]     = { 1, 1 },
    [AV_PIX_FMT_BGRA64BE]    = { 1, 1, 1 },
    [AV_PIX_FMT_BGRA64LE]    = { 1, 1, 1 },
    [AV_PIX_FMT_YUV420P9BE]  = { 1, 1 },
    [AV_PIX_FMT_YUV420P9LE]  = { 1, 1 },
    [AV_PIX_FMT_YUV420P10BE] = { 1, 1 },
    [AV_PIX_FMT_YUV420P10LE] = { 1, 1 },
    [AV_PIX_FMT_YUV420P12BE] = { 1, 1 },
    [AV_PIX_FMT_YUV420P12LE] = { 1, 1 },
    [AV_PIX_FMT_YUV420P14BE] = { 1, 1 },
    [AV_PIX_FMT_YUV420P14LE] = { 1, 1 },
    [AV_PIX_FMT_YUV422P9BE]  = { 1, 1 },
    [AV_PIX_FMT_YUV422P9LE]  = { 1, 1 },
    [AV_PIX_FMT_YUV422P10BE] = { 1, 1 },
    [AV_PIX_FMT_YUV422P10LE] = { 1, 1 },
    [AV_PIX_FMT_YUV422P12BE] = { 1, 1 },
    [AV_PIX_FMT_YUV422P12LE] = { 1, 1 },
    [AV_PIX_FMT_YUV422P14BE] = { 1, 1 },
    [AV_PIX_FMT_YUV422P14LE] = { 1, 1 },
    [AV_PIX_FMT_YUV444P9BE]  = { 1, 1 },
    [AV_PIX_FMT_YUV444P9LE]  = { 1, 1 },
    [AV_PIX_FMT_YUV444P10BE] = { 1, 1 },
    [AV_PIX_FMT_YUV444P10LE] = { 1, 1 },
    [AV_PIX_FMT_YUV444P12BE] = { 1, 1 },
    [AV_PIX_FMT_YUV444P12LE] = { 1, 1 },
    [AV_PIX_FMT_YUV444P14BE] = { 1, 1 },
    [AV_PIX_FMT_YUV444P14LE] = { 1, 1 },
    [AV_PIX_FMT_GBRP]        = { 1, 1 },
    [AV_PIX_FMT_GBRP9LE]     = { 1, 1 },
    [AV_PIX_FMT_GBRP9BE]     = { 1, 1 },
    [AV_PIX_FMT_GBRP10LE]    = { 1, 1 },
    [AV_PIX_FMT_GBRP10BE]    = { 1, 1 },
    [AV_PIX_FMT_GBRAP10LE]   = { 1, 1 },
    [AV_PIX_FMT_GBRAP10BE]   = { 1, 1 },
    [AV_PIX_FMT_GBRP12LE]    = { 1, 1 },
    [AV_PIX_FMT_GBRP12BE]    = { 1, 1 },
    [AV_PIX_FMT_GBRAP12LE]   = { 1, 1 },
    [AV_PIX_FMT_GBRAP12BE]   = { 1, 1 },
    [AV_PIX_FMT_GBRP14LE]    = { 1, 1 },
    [AV_PIX_FMT_GBRP14BE]    = { 1, 1 },
    [AV_PIX_FMT_GBRP16LE]    = { 1, 1 },
    [AV_PIX_FMT_GBRP16BE]    = { 1, 1 },
    [AV_PIX_FMT_GBRPF32LE]   = { 1, 1 },
    [AV_PIX_FMT_GBRPF32BE]   = { 1, 1 },
    [AV_PIX_FMT_GBRAPF32LE]  = { 1, 1 },
    [AV_PIX_FMT_GBRAPF32BE]  = { 1, 1 },
    [AV_PIX_FMT_GBRAP]       = { 1, 1 },
    [AV_PIX_FMT_GBRAP16LE]   = { 1, 1 },
    [AV_PIX_FMT_GBRAP16BE]   = { 1, 1 },
    [AV_PIX_FMT_BAYER_BGGR8] = { 1, 0 },
    [AV_PIX_FMT_BAYER_RGGB8] = { 1, 0 },
    [AV_PIX_FMT_BAYER_GBRG8] = { 1, 0 },
    [AV_PIX_FMT_BAYER_GRBG8] = { 1, 0 },
    [AV_PIX_FMT_BAYER_BGGR16LE] = { 1, 0 },
    [AV_PIX_FMT_BAYER_BGGR16BE] = { 1, 0 },
    [AV_PIX_FMT_BAYER_RGGB16LE] = { 1, 0 },
    [AV_PIX_FMT_BAYER_RGGB16BE] = { 1, 0 },
    [AV_PIX_FMT_BAYER_GBRG16LE] = { 1, 0 },
    [AV_PIX_FMT_BAYER_GBRG16BE] = { 1, 0 },
    [AV_PIX_FMT_BAYER_GRBG16LE] = { 1, 0 },
    [AV_PIX_FMT_BAYER_GRBG16BE] = { 1, 0 },
    [AV_PIX_FMT_XYZ12BE]     = { 1, 1, 1 },
    [AV_PIX_FMT_XYZ12LE]     = { 1, 1, 1 },
    [AV_PIX_FMT_AYUV64LE]    = { 1, 1},
    [AV_PIX_FMT_P010LE]      = { 1, 1 },
    [AV_PIX_FMT_P010BE]      = { 1, 1 },
    [AV_PIX_FMT_P016LE]      = { 1, 1 },
    [AV_PIX_FMT_P016BE]      = { 1, 1 },
    [AV_PIX_FMT_GRAYF32LE]   = { 1, 1 },
    [AV_PIX_FMT_GRAYF32BE]   = { 1, 1 },
    [AV_PIX_FMT_YUVA422P12BE] = { 1, 1 },
    [AV_PIX_FMT_YUVA422P12LE] = { 1, 1 },
    [AV_PIX_FMT_YUVA444P12BE] = { 1, 1 },
    [AV_PIX_FMT_YUVA444P12LE] = { 1, 1 },
    [AV_PIX_FMT_NV24]        = { 1, 1 },
    [AV_PIX_FMT_NV42]        = { 1, 1 },
    [AV_PIX_FMT_Y210LE]      = { 1, 0 },
    [AV_PIX_FMT_X2RGB10LE]   = { 1, 1 },
};


int sws_isSupportedInput(enum AVPixelFormat pix_fmt)
{
    return (unsigned)pix_fmt < FF_ARRAY_ELEMS(format_entries) ?
           format_entries[pix_fmt].is_supported_in : 0;
}


int sws_isSupportedOutput(enum AVPixelFormat pix_fmt)
{
    return (unsigned)pix_fmt < FF_ARRAY_ELEMS(format_entries) ?
           format_entries[pix_fmt].is_supported_out : 0;
}

const int32_t ff_yuv2rgb_coeffs[11][4] = {
    { 117489, 138438, 13975, 34925 }, /* no sequence_display_extension */
    { 117489, 138438, 13975, 34925 }, /* ITU-R Rec. 709 (1990) */
    { 104597, 132201, 25675, 53279 }, /* unspecified */
    { 104597, 132201, 25675, 53279 }, /* reserved */
    { 104448, 132798, 24759, 53109 }, /* FCC */
    { 104597, 132201, 25675, 53279 }, /* ITU-R Rec. 624-4 System B, G */
    { 104597, 132201, 25675, 53279 }, /* SMPTE 170M */
    { 117579, 136230, 16907, 35559 }, /* SMPTE 240M (1987) */
    {      0                       }, /* YCgCo */
    { 110013, 140363, 12277, 42626 }, /* Bt-2020-NCL */
    { 110013, 140363, 12277, 42626 }, /* Bt-2020-CL */
};

typedef struct {
    int flag;                   ///< flag associated to the algorithm
    const char *description;    ///< human-readable description
    int size_factor;            ///< size factor used when initing the filters
} ScaleAlgorithm;

static const ScaleAlgorithm scale_algorithms[] = {
    { SWS_AREA,          "area averaging",                  1 /* downscale only, for upscale it is bilinear */ },
    { SWS_BICUBIC,       "bicubic",                         4 },
    { SWS_BICUBLIN,      "luma bicubic / chroma bilinear", -1 },
    { SWS_BILINEAR,      "bilinear",                        2 },
    { SWS_FAST_BILINEAR, "fast bilinear",                  -1 },
    { SWS_GAUSS,         "Gaussian",                        8 /* infinite ;) */ },
    { SWS_LANCZOS,       "Lanczos",                        -1 /* custom */ },
    { SWS_POINT,         "nearest neighbor / point",       -1 },
    { SWS_SINC,          "sinc",                           20 /* infinite ;) */ },
    { SWS_SPLINE,        "bicubic spline",                 20 /* infinite :)*/ },
    { SWS_X,             "experimental",                    8 },
};

int ff_sws_alphablendaway(SwsContext *c, const uint8_t *src[],
                          int srcStride[], int srcSliceY, int srcSliceH,
                          uint8_t *dst[], int dstStride[])
{
    const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(c->srcFormat);
    int nb_components = desc->nb_components;
    int plane, x, y;
    int plane_count = isGray(c->srcFormat) ? 1 : 3;
    int sixteen_bits = desc->comp[0].depth >= 9;
    unsigned off    = 1<<(desc->comp[0].depth - 1);
    unsigned shift  = desc->comp[0].depth;
    unsigned max    = (1<<shift) - 1;
    int target_table[2][3];

    for (plane = 0; plane < plane_count; plane++) {
        int a = 0, b = 0;
        if (c->alphablend == SWS_ALPHA_BLEND_CHECKERBOARD) {
            a = (1<<(desc->comp[0].depth - 1))/2;
            b = 3*(1<<(desc->comp[0].depth-1))/2;
        }
        target_table[0][plane] = plane && !(desc->flags & AV_PIX_FMT_FLAG_RGB) ? 1<<(desc->comp[0].depth - 1) : a;
        target_table[1][plane] = plane && !(desc->flags & AV_PIX_FMT_FLAG_RGB) ? 1<<(desc->comp[0].depth - 1) : b;
    }

    av_assert0(plane_count == nb_components - 1);
    if (desc->flags & AV_PIX_FMT_FLAG_PLANAR) {
        for (plane = 0; plane < plane_count; plane++) {
            int w = plane ? c->chrSrcW : c->srcW;
            int x_subsample = plane ? desc->log2_chroma_w: 0;
            int y_subsample = plane ? desc->log2_chroma_h: 0;
            for (y = srcSliceY >> y_subsample; y < AV_CEIL_RSHIFT(srcSliceH, y_subsample); y++) {
                if (x_subsample || y_subsample) {
                    int alpha;
                    unsigned u;
                    if (sixteen_bits) {
                        ptrdiff_t alpha_step = srcStride[plane_count] >> 1;
                        const uint16_t *s = (const uint16_t *)(src[plane      ] +  srcStride[plane      ] * y);
                        const uint16_t *a = (const uint16_t *)(src[plane_count] + (srcStride[plane_count] * y << y_subsample));
                              uint16_t *d = (      uint16_t *)(dst[plane      ] +  dstStride[plane      ] * y);
                        if ((!isBE(c->srcFormat)) == !HAVE_BIGENDIAN) {
                            for (x = 0; x < w; x++) {
                                if (y_subsample) {
                                    alpha = (a[2*x]              + a[2*x + 1] + 2 +
                                             a[2*x + alpha_step] + a[2*x + alpha_step + 1]) >> 2;
                                } else
                                    alpha = (a[2*x] + a[2*x + 1]) >> 1;
                                u = s[x]*alpha + target_table[((x^y)>>5)&1][plane]*(max-alpha) + off;
                                d[x] = av_clip((u + (u >> shift)) >> shift, 0, max);
                            }
                        } else {
                            for (x = 0; x < w; x++) {
                                if (y_subsample) {
                                    alpha = (av_bswap16(a[2*x])              + av_bswap16(a[2*x + 1]) + 2 +
                                             av_bswap16(a[2*x + alpha_step]) + av_bswap16(a[2*x + alpha_step + 1])) >> 2;
                                } else
                                    alpha = (av_bswap16(a[2*x]) + av_bswap16(a[2*x + 1])) >> 1;
                                u = av_bswap16(s[x])*alpha + target_table[((x^y)>>5)&1][plane]*(max-alpha) + off;
                                d[x] = av_clip((u + (u >> shift)) >> shift, 0, max);
                            }
                        }
                    } else {
                        ptrdiff_t alpha_step = srcStride[plane_count];
                        const uint8_t *s = src[plane      ] + srcStride[plane] * y;
                        const uint8_t *a = src[plane_count] + (srcStride[plane_count] * y << y_subsample);
                              uint8_t *d = dst[plane      ] + dstStride[plane] * y;
                        for (x = 0; x < w; x++) {
                            if (y_subsample) {
                                alpha = (a[2*x]              + a[2*x + 1] + 2 +
                                         a[2*x + alpha_step] + a[2*x + alpha_step + 1]) >> 2;
                            } else
                                alpha = (a[2*x] + a[2*x + 1]) >> 1;
                            u = s[x]*alpha + target_table[((x^y)>>5)&1][plane]*(255-alpha) + 128;
                            d[x] = (257*u) >> 16;
                        }
                    }
                } else {
                if (sixteen_bits) {
                    const uint16_t *s = (const uint16_t *)(src[plane      ] + srcStride[plane      ] * y);
                    const uint16_t *a = (const uint16_t *)(src[plane_count] + srcStride[plane_count] * y);
                          uint16_t *d = (      uint16_t *)(dst[plane      ] + dstStride[plane      ] * y);
                    if ((!isBE(c->srcFormat)) == !HAVE_BIGENDIAN) {
                        for (x = 0; x < w; x++) {
                            unsigned u = s[x]*a[x] + target_table[((x^y)>>5)&1][plane]*(max-a[x]) + off;
                            d[x] = av_clip((u + (u >> shift)) >> shift, 0, max);
                        }
                    } else {
                        for (x = 0; x < w; x++) {
                            unsigned aswap =av_bswap16(a[x]);
                            unsigned u = av_bswap16(s[x])*aswap + target_table[((x^y)>>5)&1][plane]*(max-aswap) + off;
                            d[x] = av_clip((u + (u >> shift)) >> shift, 0, max);
                        }
                    }
                } else {
                    const uint8_t *s = src[plane      ] + srcStride[plane] * y;
                    const uint8_t *a = src[plane_count] + srcStride[plane_count] * y;
                          uint8_t *d = dst[plane      ] + dstStride[plane] * y;
                    for (x = 0; x < w; x++) {
                        unsigned u = s[x]*a[x] + target_table[((x^y)>>5)&1][plane]*(255-a[x]) + 128;
                        d[x] = (257*u) >> 16;
                    }
                }
                }
            }
        }
    } else {
        int alpha_pos = desc->comp[plane_count].offset;
        int w = c->srcW;
        for (y = srcSliceY; y < srcSliceH; y++) {
            if (sixteen_bits) {
                const uint16_t *s = (const uint16_t *)(src[0] + srcStride[0] * y + 2*!alpha_pos);
                const uint16_t *a = (const uint16_t *)(src[0] + srcStride[0] * y +    alpha_pos);
                      uint16_t *d = (      uint16_t *)(dst[0] + dstStride[0] * y);
                if ((!isBE(c->srcFormat)) == !HAVE_BIGENDIAN) {
                    for (x = 0; x < w; x++) {
                        for (plane = 0; plane < plane_count; plane++) {
                            int x_index = (plane_count + 1) * x;
                            unsigned u = s[x_index + plane]*a[x_index] + target_table[((x^y)>>5)&1][plane]*(max-a[x_index]) + off;
                            d[plane_count*x + plane] = av_clip((u + (u >> shift)) >> shift, 0, max);
                        }
                    }
                } else {
                    for (x = 0; x < w; x++) {
                        for (plane = 0; plane < plane_count; plane++) {
                            int x_index = (plane_count + 1) * x;
                            unsigned aswap =av_bswap16(a[x_index]);
                            unsigned u = av_bswap16(s[x_index + plane])*aswap + target_table[((x^y)>>5)&1][plane]*(max-aswap) + off;
                            d[plane_count*x + plane] = av_clip((u + (u >> shift)) >> shift, 0, max);
                        }
                    }
                }
            } else {
                const uint8_t *s = src[0] + srcStride[0] * y + !alpha_pos;
                const uint8_t *a = src[0] + srcStride[0] * y + alpha_pos;
                      uint8_t *d = dst[0] + dstStride[0] * y;
                for (x = 0; x < w; x++) {
                    for (plane = 0; plane < plane_count; plane++) {
                        int x_index = (plane_count + 1) * x;
                        unsigned u = s[x_index + plane]*a[x_index] + target_table[((x^y)>>5)&1][plane]*(255-a[x_index]) + 128;
                        d[plane_count*x + plane] = (257*u) >> 16;
                    }
                }
            }
        }
    }

    return 0;
}


av_cold int sws_init_context(SwsContext *c, SwsFilter *srcFilter,
                             SwsFilter *dstFilter)
{
    int i;
    int usesVFilter, usesHFilter;
    int unscaled;
    SwsFilter dummyFilter = { NULL, NULL, NULL, NULL };
    int srcW              = c->srcW;
    int srcH              = c->srcH;
    int dstW              = c->dstW;
    int dstH              = c->dstH;
    int dst_stride        = FFALIGN(dstW * sizeof(int16_t) + 66, 16);
    int flags, cpu_flags;
    enum AVPixelFormat srcFormat = c->srcFormat;
    enum AVPixelFormat dstFormat = c->dstFormat;
    const AVPixFmtDescriptor *desc_src;
    const AVPixFmtDescriptor *desc_dst;
    int ret = 0;
    enum AVPixelFormat tmpFmt;
    static const float float_mult = 1.0f / 255.0f;

    cpu_flags = av_get_cpu_flags();
    flags     = c->flags;
    emms_c();
    if (!rgb15to16)
        ff_sws_rgb2rgb_init();

    unscaled = (srcW == dstW && srcH == dstH);

    c->srcRange |= handle_jpeg(&c->srcFormat);
    c->dstRange |= handle_jpeg(&c->dstFormat);

    if(srcFormat!=c->srcFormat || dstFormat!=c->dstFormat)
        av_log(c, AV_LOG_WARNING, "deprecated pixel format used, make sure you did set range correctly\n");

    if (!c->contrast && !c->saturation && !c->dstFormatBpp)
        sws_setColorspaceDetails(c, ff_yuv2rgb_coeffs[SWS_CS_DEFAULT], c->srcRange,
                                 ff_yuv2rgb_coeffs[SWS_CS_DEFAULT],
                                 c->dstRange, 0, 1 << 16, 1 << 16);

    handle_formats(c);
    srcFormat = c->srcFormat;
    dstFormat = c->dstFormat;
    desc_src = av_pix_fmt_desc_get(srcFormat);
    desc_dst = av_pix_fmt_desc_get(dstFormat);

    // If the source has no alpha then disable alpha blendaway
    if (c->src0Alpha)
        c->alphablend = SWS_ALPHA_BLEND_NONE;

    if (!(unscaled && sws_isSupportedEndiannessConversion(srcFormat) &&
          av_pix_fmt_swap_endianness(srcFormat) == dstFormat)) {
    if (!sws_isSupportedInput(srcFormat)) {
        av_log(c, AV_LOG_ERROR, "%s is not supported as input pixel format\n",
               av_get_pix_fmt_name(srcFormat));
        return AVERROR(EINVAL);
    }
    if (!sws_isSupportedOutput(dstFormat)) {
        av_log(c, AV_LOG_ERROR, "%s is not supported as output pixel format\n",
               av_get_pix_fmt_name(dstFormat));
        return AVERROR(EINVAL);
    }
    }
    av_assert2(desc_src && desc_dst);

    i = flags & (SWS_POINT         |
                 SWS_AREA          |
                 SWS_BILINEAR      |
                 SWS_FAST_BILINEAR |
                 SWS_BICUBIC       |
                 SWS_X             |
                 SWS_GAUSS         |
                 SWS_LANCZOS       |
                 SWS_SINC          |
                 SWS_SPLINE        |
                 SWS_BICUBLIN);

    /* provide a default scaler if not set by caller */
    if (!i) {
        if (dstW < srcW && dstH < srcH)
            flags |= SWS_BICUBIC;
        else if (dstW > srcW && dstH > srcH)
            flags |= SWS_BICUBIC;
        else
            flags |= SWS_BICUBIC;
        c->flags = flags;
    } else if (i & (i - 1)) {
        av_log(c, AV_LOG_ERROR,
               "Exactly one scaler algorithm must be chosen, got %X\n", i);
        return AVERROR(EINVAL);
    }
    /* sanity check */
    if (srcW < 1 || srcH < 1 || dstW < 1 || dstH < 1) {
        /* FIXME check if these are enough and try to lower them after
         * fixing the relevant parts of the code */
        av_log(c, AV_LOG_ERROR, "%dx%d -> %dx%d is invalid scaling dimension\n",
               srcW, srcH, dstW, dstH);
        return AVERROR(EINVAL);
    }
    if (flags & SWS_FAST_BILINEAR) {
        if (srcW < 8 || dstW < 8) {
            flags ^= SWS_FAST_BILINEAR | SWS_BILINEAR;
            c->flags = flags;
        }
    }

    if (!dstFilter)
        dstFilter = &dummyFilter;
    if (!srcFilter)
        srcFilter = &dummyFilter;

    c->lumXInc      = (((int64_t)srcW << 16) + (dstW >> 1)) / dstW;
    c->lumYInc      = (((int64_t)srcH << 16) + (dstH >> 1)) / dstH;
    c->dstFormatBpp = av_get_bits_per_pixel(desc_dst);
    c->srcFormatBpp = av_get_bits_per_pixel(desc_src);
    c->vRounder     = 4 * 0x0001000100010001ULL;

    usesVFilter = (srcFilter->lumV && srcFilter->lumV->length > 1) ||
                  (srcFilter->chrV && srcFilter->chrV->length > 1) ||
                  (dstFilter->lumV && dstFilter->lumV->length > 1) ||
                  (dstFilter->chrV && dstFilter->chrV->length > 1);
    usesHFilter = (srcFilter->lumH && srcFilter->lumH->length > 1) ||
                  (srcFilter->chrH && srcFilter->chrH->length > 1) ||
                  (dstFilter->lumH && dstFilter->lumH->length > 1) ||
                  (dstFilter->chrH && dstFilter->chrH->length > 1);

    av_pix_fmt_get_chroma_sub_sample(srcFormat, &c->chrSrcHSubSample, &c->chrSrcVSubSample);
    av_pix_fmt_get_chroma_sub_sample(dstFormat, &c->chrDstHSubSample, &c->chrDstVSubSample);

    if (isAnyRGB(dstFormat) && !(flags&SWS_FULL_CHR_H_INT)) {
        if (dstW&1) {
            av_log(c, AV_LOG_DEBUG, "Forcing full internal H chroma due to odd output size\n");
            flags |= SWS_FULL_CHR_H_INT;
            c->flags = flags;
        }

        if (   c->chrSrcHSubSample == 0
            && c->chrSrcVSubSample == 0
            && c->dither != SWS_DITHER_BAYER //SWS_FULL_CHR_H_INT is currently not supported with SWS_DITHER_BAYER
            && !(c->flags & SWS_FAST_BILINEAR)
        ) {
            av_log(c, AV_LOG_DEBUG, "Forcing full internal H chroma due to input having non subsampled chroma\n");
            flags |= SWS_FULL_CHR_H_INT;
            c->flags = flags;
        }
    }

    if (c->dither == SWS_DITHER_AUTO) {
        if (flags & SWS_ERROR_DIFFUSION)
            c->dither = SWS_DITHER_ED;
    }

    if(dstFormat == AV_PIX_FMT_BGR4_BYTE ||
       dstFormat == AV_PIX_FMT_RGB4_BYTE ||
       dstFormat == AV_PIX_FMT_BGR8 ||
       dstFormat == AV_PIX_FMT_RGB8) {
        if (c->dither == SWS_DITHER_AUTO)
            c->dither = (flags & SWS_FULL_CHR_H_INT) ? SWS_DITHER_ED : SWS_DITHER_BAYER;
        if (!(flags & SWS_FULL_CHR_H_INT)) {
            if (c->dither == SWS_DITHER_ED || c->dither == SWS_DITHER_A_DITHER || c->dither == SWS_DITHER_X_DITHER) {
                av_log(c, AV_LOG_DEBUG,
                    "Desired dithering only supported in full chroma interpolation for destination format '%s'\n",
                    av_get_pix_fmt_name(dstFormat));
                flags   |= SWS_FULL_CHR_H_INT;
                c->flags = flags;
            }
        }
        if (flags & SWS_FULL_CHR_H_INT) {
            if (c->dither == SWS_DITHER_BAYER) {
                av_log(c, AV_LOG_DEBUG,
                    "Ordered dither is not supported in full chroma interpolation for destination format '%s'\n",
                    av_get_pix_fmt_name(dstFormat));
                c->dither = SWS_DITHER_ED;
            }
        }
    }
    if (isPlanarRGB(dstFormat)) {
        if (!(flags & SWS_FULL_CHR_H_INT)) {
            av_log(c, AV_LOG_DEBUG,
                   "%s output is not supported with half chroma resolution, switching to full\n",
                   av_get_pix_fmt_name(dstFormat));
            flags   |= SWS_FULL_CHR_H_INT;
            c->flags = flags;
        }
    }

    /* reuse chroma for 2 pixels RGB/BGR unless user wants full
     * chroma interpolation */
    if (flags & SWS_FULL_CHR_H_INT &&
        isAnyRGB(dstFormat)        &&
        !isPlanarRGB(dstFormat)    &&
        dstFormat != AV_PIX_FMT_RGBA64LE &&
        dstFormat != AV_PIX_FMT_RGBA64BE &&
        dstFormat != AV_PIX_FMT_BGRA64LE &&
        dstFormat != AV_PIX_FMT_BGRA64BE &&
        dstFormat != AV_PIX_FMT_RGB48LE &&
        dstFormat != AV_PIX_FMT_RGB48BE &&
        dstFormat != AV_PIX_FMT_BGR48LE &&
        dstFormat != AV_PIX_FMT_BGR48BE &&
        dstFormat != AV_PIX_FMT_RGBA  &&
        dstFormat != AV_PIX_FMT_ARGB  &&
        dstFormat != AV_PIX_FMT_BGRA  &&
        dstFormat != AV_PIX_FMT_ABGR  &&
        dstFormat != AV_PIX_FMT_RGB24 &&
        dstFormat != AV_PIX_FMT_BGR24 &&
        dstFormat != AV_PIX_FMT_BGR4_BYTE &&
        dstFormat != AV_PIX_FMT_RGB4_BYTE &&
        dstFormat != AV_PIX_FMT_BGR8 &&
        dstFormat != AV_PIX_FMT_RGB8
    ) {
        av_log(c, AV_LOG_WARNING,
               "full chroma interpolation for destination format '%s' not yet implemented\n",
               av_get_pix_fmt_name(dstFormat));
        flags   &= ~SWS_FULL_CHR_H_INT;
        c->flags = flags;
    }
    if (isAnyRGB(dstFormat) && !(flags & SWS_FULL_CHR_H_INT))
        c->chrDstHSubSample = 1;

    // drop some chroma lines if the user wants it
    c->vChrDrop          = (flags & SWS_SRC_V_CHR_DROP_MASK) >>
                           SWS_SRC_V_CHR_DROP_SHIFT;
    c->chrSrcVSubSample += c->vChrDrop;

    /* drop every other pixel for chroma calculation unless user
     * wants full chroma */
    if (isAnyRGB(srcFormat) && !(flags & SWS_FULL_CHR_H_INP)   &&
        srcFormat != AV_PIX_FMT_RGB8 && srcFormat != AV_PIX_FMT_BGR8 &&
        srcFormat != AV_PIX_FMT_RGB4 && srcFormat != AV_PIX_FMT_BGR4 &&
        srcFormat != AV_PIX_FMT_RGB4_BYTE && srcFormat != AV_PIX_FMT_BGR4_BYTE &&
        srcFormat != AV_PIX_FMT_GBRP9BE   && srcFormat != AV_PIX_FMT_GBRP9LE  &&
        srcFormat != AV_PIX_FMT_GBRP10BE  && srcFormat != AV_PIX_FMT_GBRP10LE &&
        srcFormat != AV_PIX_FMT_GBRAP10BE && srcFormat != AV_PIX_FMT_GBRAP10LE &&
        srcFormat != AV_PIX_FMT_GBRP12BE  && srcFormat != AV_PIX_FMT_GBRP12LE &&
        srcFormat != AV_PIX_FMT_GBRAP12BE && srcFormat != AV_PIX_FMT_GBRAP12LE &&
        srcFormat != AV_PIX_FMT_GBRP14BE  && srcFormat != AV_PIX_FMT_GBRP14LE &&
        srcFormat != AV_PIX_FMT_GBRP16BE  && srcFormat != AV_PIX_FMT_GBRP16LE &&
        srcFormat != AV_PIX_FMT_GBRAP16BE  && srcFormat != AV_PIX_FMT_GBRAP16LE &&
        srcFormat != AV_PIX_FMT_GBRPF32BE  && srcFormat != AV_PIX_FMT_GBRPF32LE &&
        srcFormat != AV_PIX_FMT_GBRAPF32BE && srcFormat != AV_PIX_FMT_GBRAPF32LE &&
        ((dstW >> c->chrDstHSubSample) <= (srcW >> 1) ||
         (flags & SWS_FAST_BILINEAR)))
        c->chrSrcHSubSample = 1;

    // Note the AV_CEIL_RSHIFT is so that we always round toward +inf.
    c->chrSrcW = AV_CEIL_RSHIFT(srcW, c->chrSrcHSubSample);
    c->chrSrcH = AV_CEIL_RSHIFT(srcH, c->chrSrcVSubSample);
    c->chrDstW = AV_CEIL_RSHIFT(dstW, c->chrDstHSubSample);
    c->chrDstH = AV_CEIL_RSHIFT(dstH, c->chrDstVSubSample);

    if (!FF_ALLOCZ_TYPED_ARRAY(c->formatConvBuffer, FFALIGN(srcW * 2 + 78, 16) * 2))
        goto nomem;

    c->srcBpc = desc_src->comp[0].depth;
    if (c->srcBpc < 8)
        c->srcBpc = 8;
    c->dstBpc = desc_dst->comp[0].depth;
    if (c->dstBpc < 8)
        c->dstBpc = 8;
    if (isAnyRGB(srcFormat) || srcFormat == AV_PIX_FMT_PAL8)
        c->srcBpc = 16;
    if (c->dstBpc == 16)
        dst_stride <<= 1;

    if (INLINE_MMXEXT(cpu_flags) && c->srcBpc == 8 && c->dstBpc <= 14) {
        c->canMMXEXTBeUsed = dstW >= srcW && (dstW & 31) == 0 &&
                             c->chrDstW >= c->chrSrcW &&
                             (srcW & 15) == 0;
        if (!c->canMMXEXTBeUsed && dstW >= srcW && c->chrDstW >= c->chrSrcW && (srcW & 15) == 0

            && (flags & SWS_FAST_BILINEAR)) {
            if (flags & SWS_PRINT_INFO)
                av_log(c, AV_LOG_INFO,
                       "output width is not a multiple of 32 -> no MMXEXT scaler\n");
        }
        if (usesHFilter || isNBPS(c->srcFormat) || is16BPS(c->srcFormat) || isAnyRGB(c->srcFormat))
            c->canMMXEXTBeUsed = 0;
    } else
        c->canMMXEXTBeUsed = 0;

    c->chrXInc = (((int64_t)c->chrSrcW << 16) + (c->chrDstW >> 1)) / c->chrDstW;
    c->chrYInc = (((int64_t)c->chrSrcH << 16) + (c->chrDstH >> 1)) / c->chrDstH;

    /* Match pixel 0 of the src to pixel 0 of dst and match pixel n-2 of src
     * to pixel n-2 of dst, but only for the FAST_BILINEAR mode otherwise do
     * correct scaling.
     * n-2 is the last chrominance sample available.
     * This is not perfect, but no one should notice the difference, the more
     * correct variant would be like the vertical one, but that would require
     * some special code for the first and last pixel */
    if (flags & SWS_FAST_BILINEAR) {
        if (c->canMMXEXTBeUsed) {
            c->lumXInc += 20;
            c->chrXInc += 20;
        }
        // we don't use the x86 asm scaler if MMX is available
        else if (INLINE_MMX(cpu_flags) && c->dstBpc <= 14) {
            c->lumXInc = ((int64_t)(srcW       - 2) << 16) / (dstW       - 2) - 20;
            c->chrXInc = ((int64_t)(c->chrSrcW - 2) << 16) / (c->chrDstW - 2) - 20;
        }
    }

    // hardcoded for now
    c->gamma_value = 2.2;
    tmpFmt = AV_PIX_FMT_RGBA64LE;


    if (!unscaled && c->gamma_flag && (srcFormat != tmpFmt || dstFormat != tmpFmt)) {
        SwsContext *c2;
        c->cascaded_context[0] = NULL;

        ret = av_image_alloc(c->cascaded_tmp, c->cascaded_tmpStride,
                            srcW, srcH, tmpFmt, 64);
        if (ret < 0)
            return ret;

        c->cascaded_context[0] = sws_getContext(srcW, srcH, srcFormat,
                                                srcW, srcH, tmpFmt,
                                                flags, NULL, NULL, c->param);
        if (!c->cascaded_context[0]) {
            return AVERROR(ENOMEM);
        }

        c->cascaded_context[1] = sws_getContext(srcW, srcH, tmpFmt,
                                                dstW, dstH, tmpFmt,
                                                flags, srcFilter, dstFilter, c->param);

        if (!c->cascaded_context[1])
            return AVERROR(ENOMEM);

        c2 = c->cascaded_context[1];
        c2->is_internal_gamma = 1;
        c2->gamma     = alloc_gamma_tbl(    c->gamma_value);
        c2->inv_gamma = alloc_gamma_tbl(1.f/c->gamma_value);
        if (!c2->gamma || !c2->inv_gamma)
            return AVERROR(ENOMEM);

        // is_internal_flag is set after creating the context
        // to properly create the gamma convert FilterDescriptor
        // we have to re-initialize it
        ff_free_filters(c2);
        if ((ret = ff_init_filters(c2)) < 0) {
            sws_freeContext(c2);
            c->cascaded_context[1] = NULL;
            return ret;
        }

        c->cascaded_context[2] = NULL;
        if (dstFormat != tmpFmt) {
            ret = av_image_alloc(c->cascaded1_tmp, c->cascaded1_tmpStride,
                                dstW, dstH, tmpFmt, 64);
            if (ret < 0)
                return ret;

            c->cascaded_context[2] = sws_getContext(dstW, dstH, tmpFmt,
                                                dstW, dstH, dstFormat,
                                                flags, NULL, NULL, c->param);
            if (!c->cascaded_context[2])
                return AVERROR(ENOMEM);
        }
        return 0;
    }

    if (isBayer(srcFormat)) {
        if (!unscaled ||
            (dstFormat != AV_PIX_FMT_RGB24 && dstFormat != AV_PIX_FMT_YUV420P &&
             dstFormat != AV_PIX_FMT_RGB48)) {
            enum AVPixelFormat tmpFormat = isBayer16BPS(srcFormat) ? AV_PIX_FMT_RGB48 : AV_PIX_FMT_RGB24;

            ret = av_image_alloc(c->cascaded_tmp, c->cascaded_tmpStride,
                                srcW, srcH, tmpFormat, 64);
            if (ret < 0)
                return ret;

            c->cascaded_context[0] = sws_getContext(srcW, srcH, srcFormat,
                                                    srcW, srcH, tmpFormat,
                                                    flags, srcFilter, NULL, c->param);
            if (!c->cascaded_context[0])
                return AVERROR(ENOMEM);

            c->cascaded_context[1] = sws_getContext(srcW, srcH, tmpFormat,
                                                    dstW, dstH, dstFormat,
                                                    flags, NULL, dstFilter, c->param);
            if (!c->cascaded_context[1])
                return AVERROR(ENOMEM);
            return 0;
        }
    }

    if (unscaled && c->srcBpc == 8 && dstFormat == AV_PIX_FMT_GRAYF32){
        for (i = 0; i < 256; ++i){
            c->uint2float_lut[i] = (float)i * float_mult;
        }
    }

    // float will be converted to uint16_t
    if ((srcFormat == AV_PIX_FMT_GRAYF32BE || srcFormat == AV_PIX_FMT_GRAYF32LE) &&
        (!unscaled || unscaled && dstFormat != srcFormat && (srcFormat != AV_PIX_FMT_GRAYF32 ||
        dstFormat != AV_PIX_FMT_GRAY8))){
        c->srcBpc = 16;
    }

    if (CONFIG_SWSCALE_ALPHA && isALPHA(srcFormat) && !isALPHA(dstFormat)) {
        enum AVPixelFormat tmpFormat = alphaless_fmt(srcFormat);

        if (tmpFormat != AV_PIX_FMT_NONE && c->alphablend != SWS_ALPHA_BLEND_NONE) {
            if (!unscaled ||
                dstFormat != tmpFormat ||
                usesHFilter || usesVFilter ||
                c->srcRange != c->dstRange
            ) {
                c->cascaded_mainindex = 1;
                ret = av_image_alloc(c->cascaded_tmp, c->cascaded_tmpStride,
                                     srcW, srcH, tmpFormat, 64);
                if (ret < 0)
                    return ret;

                c->cascaded_context[0] = sws_alloc_set_opts(srcW, srcH, srcFormat,
                                                            srcW, srcH, tmpFormat,
                                                            flags, c->param);
                if (!c->cascaded_context[0])
                    return AVERROR(EINVAL);
                c->cascaded_context[0]->alphablend = c->alphablend;
                ret = sws_init_context(c->cascaded_context[0], NULL , NULL);
                if (ret < 0)
                    return ret;

                c->cascaded_context[1] = sws_alloc_set_opts(srcW, srcH, tmpFormat,
                                                            dstW, dstH, dstFormat,
                                                            flags, c->param);
                if (!c->cascaded_context[1])
                    return AVERROR(EINVAL);

                c->cascaded_context[1]->srcRange = c->srcRange;
                c->cascaded_context[1]->dstRange = c->dstRange;
                ret = sws_init_context(c->cascaded_context[1], srcFilter , dstFilter);
                if (ret < 0)
                    return ret;

                return 0;
            }
        }
    }

#if HAVE_MMAP && HAVE_MPROTECT && defined(MAP_ANONYMOUS)
#define USE_MMAP 1
#else
#define USE_MMAP 0
#endif

    /* precalculate horizontal scaler filter coefficients */
    {
#if HAVE_MMXEXT_INLINE
// can't downscale !!!
        if (c->canMMXEXTBeUsed && (flags & SWS_FAST_BILINEAR)) {
            c->lumMmxextFilterCodeSize = ff_init_hscaler_mmxext(dstW, c->lumXInc, NULL,
                                                             NULL, NULL, 8);
            c->chrMmxextFilterCodeSize = ff_init_hscaler_mmxext(c->chrDstW, c->chrXInc,
                                                             NULL, NULL, NULL, 4);

#if USE_MMAP
            c->lumMmxextFilterCode = mmap(NULL, c->lumMmxextFilterCodeSize,
                                          PROT_READ | PROT_WRITE,
                                          MAP_PRIVATE | MAP_ANONYMOUS,
                                          -1, 0);
            c->chrMmxextFilterCode = mmap(NULL, c->chrMmxextFilterCodeSize,
                                          PROT_READ | PROT_WRITE,
                                          MAP_PRIVATE | MAP_ANONYMOUS,
                                          -1, 0);
#elif HAVE_VIRTUALALLOC
            c->lumMmxextFilterCode = VirtualAlloc(NULL,
                                                  c->lumMmxextFilterCodeSize,
                                                  MEM_COMMIT,
                                                  PAGE_EXECUTE_READWRITE);
            c->chrMmxextFilterCode = VirtualAlloc(NULL,
                                                  c->chrMmxextFilterCodeSize,
                                                  MEM_COMMIT,
                                                  PAGE_EXECUTE_READWRITE);
#else
            c->lumMmxextFilterCode = av_malloc(c->lumMmxextFilterCodeSize);
            c->chrMmxextFilterCode = av_malloc(c->chrMmxextFilterCodeSize);
#endif

#ifdef MAP_ANONYMOUS
            if (c->lumMmxextFilterCode == MAP_FAILED || c->chrMmxextFilterCode == MAP_FAILED)
#else
            if (!c->lumMmxextFilterCode || !c->chrMmxextFilterCode)
#endif
            {
                av_log(c, AV_LOG_ERROR, "Failed to allocate MMX2FilterCode\n");
                return AVERROR(ENOMEM);
            }

            if (!FF_ALLOCZ_TYPED_ARRAY(c->hLumFilter,    dstW           / 8 + 8) ||
                !FF_ALLOCZ_TYPED_ARRAY(c->hChrFilter,    c->chrDstW     / 4 + 8) ||
                !FF_ALLOCZ_TYPED_ARRAY(c->hLumFilterPos, dstW       / 2 / 8 + 8) ||
                !FF_ALLOCZ_TYPED_ARRAY(c->hChrFilterPos, c->chrDstW / 2 / 4 + 8))
                goto nomem;

            ff_init_hscaler_mmxext(      dstW, c->lumXInc, c->lumMmxextFilterCode,
                                c->hLumFilter, (uint32_t*)c->hLumFilterPos, 8);
            ff_init_hscaler_mmxext(c->chrDstW, c->chrXInc, c->chrMmxextFilterCode,
                                c->hChrFilter, (uint32_t*)c->hChrFilterPos, 4);

#if USE_MMAP
            if (   mprotect(c->lumMmxextFilterCode, c->lumMmxextFilterCodeSize, PROT_EXEC | PROT_READ) == -1
                || mprotect(c->chrMmxextFilterCode, c->chrMmxextFilterCodeSize, PROT_EXEC | PROT_READ) == -1) {
                av_log(c, AV_LOG_ERROR, "mprotect failed, cannot use fast bilinear scaler\n");
                ret = AVERROR(EINVAL);
                goto fail;
            }
#endif
        } else
#endif /* HAVE_MMXEXT_INLINE */
        {
            const int filterAlign = X86_MMX(cpu_flags)     ? 4 :
                                    PPC_ALTIVEC(cpu_flags) ? 8 :
                                    have_neon(cpu_flags)   ? 8 : 1;

            if ((ret = initFilter(&c->hLumFilter, &c->hLumFilterPos,
                           &c->hLumFilterSize, c->lumXInc,
                           srcW, dstW, filterAlign, 1 << 14,
                           (flags & SWS_BICUBLIN) ? (flags | SWS_BICUBIC) : flags,
                           cpu_flags, srcFilter->lumH, dstFilter->lumH,
                           c->param,
                           get_local_pos(c, 0, 0, 0),
                           get_local_pos(c, 0, 0, 0))) < 0)
                goto fail;
            if ((ret = initFilter(&c->hChrFilter, &c->hChrFilterPos,
                           &c->hChrFilterSize, c->chrXInc,
                           c->chrSrcW, c->chrDstW, filterAlign, 1 << 14,
                           (flags & SWS_BICUBLIN) ? (flags | SWS_BILINEAR) : flags,
                           cpu_flags, srcFilter->chrH, dstFilter->chrH,
                           c->param,
                           get_local_pos(c, c->chrSrcHSubSample, c->src_h_chr_pos, 0),
                           get_local_pos(c, c->chrDstHSubSample, c->dst_h_chr_pos, 0))) < 0)
                goto fail;
        }
    } // initialize horizontal stuff

    /* precalculate vertical scaler filter coefficients */
    {
        const int filterAlign = X86_MMX(cpu_flags)     ? 2 :
                                PPC_ALTIVEC(cpu_flags) ? 8 :
                                have_neon(cpu_flags)   ? 2 : 1;

        if ((ret = initFilter(&c->vLumFilter, &c->vLumFilterPos, &c->vLumFilterSize,
                       c->lumYInc, srcH, dstH, filterAlign, (1 << 12),
                       (flags & SWS_BICUBLIN) ? (flags | SWS_BICUBIC) : flags,
                       cpu_flags, srcFilter->lumV, dstFilter->lumV,
                       c->param,
                       get_local_pos(c, 0, 0, 1),
                       get_local_pos(c, 0, 0, 1))) < 0)
            goto fail;
        if ((ret = initFilter(&c->vChrFilter, &c->vChrFilterPos, &c->vChrFilterSize,
                       c->chrYInc, c->chrSrcH, c->chrDstH,
                       filterAlign, (1 << 12),
                       (flags & SWS_BICUBLIN) ? (flags | SWS_BILINEAR) : flags,
                       cpu_flags, srcFilter->chrV, dstFilter->chrV,
                       c->param,
                       get_local_pos(c, c->chrSrcVSubSample, c->src_v_chr_pos, 1),
                       get_local_pos(c, c->chrDstVSubSample, c->dst_v_chr_pos, 1))) < 0)

            goto fail;

#if HAVE_ALTIVEC
        if (!FF_ALLOC_TYPED_ARRAY(c->vYCoeffsBank, c->vLumFilterSize * c->dstH) ||
            !FF_ALLOC_TYPED_ARRAY(c->vCCoeffsBank, c->vChrFilterSize * c->chrDstH))
            goto nomem;

        for (i = 0; i < c->vLumFilterSize * c->dstH; i++) {
            int j;
            short *p = (short *)&c->vYCoeffsBank[i];
            for (j = 0; j < 8; j++)
                p[j] = c->vLumFilter[i];
        }

        for (i = 0; i < c->vChrFilterSize * c->chrDstH; i++) {
            int j;
            short *p = (short *)&c->vCCoeffsBank[i];
            for (j = 0; j < 8; j++)
                p[j] = c->vChrFilter[i];
        }
#endif
    }

    for (i = 0; i < 4; i++)
        if (!FF_ALLOCZ_TYPED_ARRAY(c->dither_error[i], c->dstW + 2))
            goto nomem;

    c->needAlpha = (CONFIG_SWSCALE_ALPHA && isALPHA(c->srcFormat) && isALPHA(c->dstFormat)) ? 1 : 0;

    // 64 / c->scalingBpp is the same as 16 / sizeof(scaling_intermediate)
    c->uv_off   = (dst_stride>>1) + 64 / (c->dstBpc &~ 7);
    c->uv_offx2 = dst_stride + 16;

    av_assert0(c->chrDstH <= dstH);

    if (flags & SWS_PRINT_INFO) {
        const char *scaler = NULL, *cpucaps;

        for (i = 0; i < FF_ARRAY_ELEMS(scale_algorithms); i++) {
            if (flags & scale_algorithms[i].flag) {
                scaler = scale_algorithms[i].description;
                break;
            }
        }
        if (!scaler)
            scaler =  "ehh flags invalid?!";
        av_log(c, AV_LOG_INFO, "%s scaler, from %s to %s%s ",
               scaler,
               av_get_pix_fmt_name(srcFormat),
#ifdef DITHER1XBPP
               dstFormat == AV_PIX_FMT_BGR555   || dstFormat == AV_PIX_FMT_BGR565   ||
               dstFormat == AV_PIX_FMT_RGB444BE || dstFormat == AV_PIX_FMT_RGB444LE ||
               dstFormat == AV_PIX_FMT_BGR444BE || dstFormat == AV_PIX_FMT_BGR444LE ?
                                                             "dithered " : "",
#else
               "",
#endif
               av_get_pix_fmt_name(dstFormat));

        if (INLINE_MMXEXT(cpu_flags))
            cpucaps = "MMXEXT";
        else if (INLINE_AMD3DNOW(cpu_flags))
            cpucaps = "3DNOW";
        else if (INLINE_MMX(cpu_flags))
            cpucaps = "MMX";
        else if (PPC_ALTIVEC(cpu_flags))
            cpucaps = "AltiVec";
        else
            cpucaps = "C";

        av_log(c, AV_LOG_INFO, "using %s\n", cpucaps);

        av_log(c, AV_LOG_VERBOSE, "%dx%d -> %dx%d\n", srcW, srcH, dstW, dstH);
        av_log(c, AV_LOG_DEBUG,
               "lum srcW=%d srcH=%d dstW=%d dstH=%d xInc=%d yInc=%d\n",
               c->srcW, c->srcH, c->dstW, c->dstH, c->lumXInc, c->lumYInc);
        av_log(c, AV_LOG_DEBUG,
               "chr srcW=%d srcH=%d dstW=%d dstH=%d xInc=%d yInc=%d\n",
               c->chrSrcW, c->chrSrcH, c->chrDstW, c->chrDstH,
               c->chrXInc, c->chrYInc);
    }

    /* alpha blend special case, note this has been split via cascaded contexts if its scaled */
    if (unscaled && !usesHFilter && !usesVFilter &&
        c->alphablend != SWS_ALPHA_BLEND_NONE &&
        isALPHA(srcFormat) &&
        (c->srcRange == c->dstRange || isAnyRGB(dstFormat)) &&
        alphaless_fmt(srcFormat) == dstFormat
    ) {
        c->swscale = ff_sws_alphablendaway;

        if (flags & SWS_PRINT_INFO)
            av_log(c, AV_LOG_INFO,
                    "using alpha blendaway %s -> %s special converter\n",
                    av_get_pix_fmt_name(srcFormat), av_get_pix_fmt_name(dstFormat));
        return 0;
    }

    /* unscaled special cases */
    if (unscaled && !usesHFilter && !usesVFilter &&
        (c->srcRange == c->dstRange || isAnyRGB(dstFormat) ||
         isFloat(srcFormat) || isFloat(dstFormat))){
        ff_get_unscaled_swscale(c);

        if (c->swscale) {
            if (flags & SWS_PRINT_INFO)
                av_log(c, AV_LOG_INFO,
                       "using unscaled %s -> %s special converter\n",
                       av_get_pix_fmt_name(srcFormat), av_get_pix_fmt_name(dstFormat));
            return 0;
        }
    }

    c->swscale = ff_getSwsFunc(c);
    return ff_init_filters(c);
nomem:
    ret = AVERROR(ENOMEM);
fail: // FIXME replace things by appropriate error codes
    if (ret == RETCODE_USE_CASCADE)  {
        int tmpW = sqrt(srcW * (int64_t)dstW);
        int tmpH = sqrt(srcH * (int64_t)dstH);
        enum AVPixelFormat tmpFormat = AV_PIX_FMT_YUV420P;

        if (isALPHA(srcFormat))
            tmpFormat = AV_PIX_FMT_YUVA420P;

        if (srcW*(int64_t)srcH <= 4LL*dstW*dstH)
            return AVERROR(EINVAL);

        ret = av_image_alloc(c->cascaded_tmp, c->cascaded_tmpStride,
                             tmpW, tmpH, tmpFormat, 64);
        if (ret < 0)
            return ret;

        c->cascaded_context[0] = sws_getContext(srcW, srcH, srcFormat,
                                                tmpW, tmpH, tmpFormat,
                                                flags, srcFilter, NULL, c->param);
        if (!c->cascaded_context[0])
            return AVERROR(ENOMEM);

        c->cascaded_context[1] = sws_getContext(tmpW, tmpH, tmpFormat,
                                                dstW, dstH, dstFormat,
                                                flags, NULL, dstFilter, c->param);
        if (!c->cascaded_context[1])
            return AVERROR(ENOMEM);
        return 0;
    }
    return ret;
}


struct SwsContext *sws_getCachedContext(struct SwsContext *context, int srcW,
                                        int srcH, enum AVPixelFormat srcFormat,
                                        int dstW, int dstH,
                                        enum AVPixelFormat dstFormat, int flags,
                                        SwsFilter *srcFilter,
                                        SwsFilter *dstFilter,
                                        const double *param)
{
    static const double default_param[2] = {SWS_PARAM_DEFAULT,
                                            SWS_PARAM_DEFAULT};
    int64_t src_h_chr_pos = -513, dst_h_chr_pos = -513,
            src_v_chr_pos = -513, dst_v_chr_pos = -513;

    if (!param)
        param = default_param;

    if (context &&
        (context->srcW != srcW ||
         context->srcH != srcH ||
         context->srcFormat != srcFormat ||
         context->dstW != dstW ||
         context->dstH != dstH ||
         context->dstFormat != dstFormat ||
         context->flags != flags ||
         context->param[0] != param[0] ||
         context->param[1] != param[1]))
    {

        av_opt_get_int(context, "src_h_chr_pos", 0, &src_h_chr_pos);
        av_opt_get_int(context, "src_v_chr_pos", 0, &src_v_chr_pos);
        av_opt_get_int(context, "dst_h_chr_pos", 0, &dst_h_chr_pos);
        av_opt_get_int(context, "dst_v_chr_pos", 0, &dst_v_chr_pos);
        sws_freeContext(context);
        context = NULL;
    }

    if (!context)
    {
        if (!(context = sws_alloc_context()))
            return NULL;
        context->srcW = srcW;
        context->srcH = srcH;
        context->srcFormat = srcFormat;
        context->dstW = dstW;
        context->dstH = dstH;
        context->dstFormat = dstFormat;
        context->flags = flags;
        context->param[0] = param[0];
        context->param[1] = param[1];

        av_opt_set_int(context, "src_h_chr_pos", src_h_chr_pos, 0);
        av_opt_set_int(context, "src_v_chr_pos", src_v_chr_pos, 0);
        av_opt_set_int(context, "dst_h_chr_pos", dst_h_chr_pos, 0);
        av_opt_set_int(context, "dst_v_chr_pos", dst_v_chr_pos, 0);

        if (sws_init_context(context, srcFilter, dstFilter) < 0)
        {
            sws_freeContext(context);
            return NULL;
        }
    }
    return context;
}
int attribute_align_arg sws_scale(struct SwsContext *c,
                                  const uint8_t * const srcSlice[],
                                  const int srcStride[], int srcSliceY,
                                  int srcSliceH, uint8_t *const dst[],
                                  const int dstStride[])
{
    int i, ret;
    const uint8_t *src2[4];
    uint8_t *dst2[4];
    uint8_t *rgb0_tmp = NULL;
    int macro_height = isBayer(c->srcFormat) ? 2 : (1 << c->chrSrcVSubSample);
    // copy strides, so they can safely be modified
    int srcStride2[4];
    int dstStride2[4];
    int srcSliceY_internal = srcSliceY;

    if (!srcStride || !dstStride || !dst || !srcSlice) {
        av_log(c, AV_LOG_ERROR, "One of the input parameters to sws_scale() is NULL, please check the calling code\n");
        return 0;
    }

    for (i=0; i<4; i++) {
        srcStride2[i] = srcStride[i];
        dstStride2[i] = dstStride[i];
    }

    if ((srcSliceY & (macro_height-1)) ||
        ((srcSliceH& (macro_height-1)) && srcSliceY + srcSliceH != c->srcH) ||
        srcSliceY + srcSliceH > c->srcH) {
        av_log(c, AV_LOG_ERROR, "Slice parameters %d, %d are invalid\n", srcSliceY, srcSliceH);
        return AVERROR(EINVAL);
    }

    if (c->gamma_flag && c->cascaded_context[0]) {
        ret = sws_scale(c->cascaded_context[0],
                    srcSlice, srcStride, srcSliceY, srcSliceH,
                    c->cascaded_tmp, c->cascaded_tmpStride);

        if (ret < 0)
            return ret;

        if (c->cascaded_context[2])
            ret = sws_scale(c->cascaded_context[1], (const uint8_t * const *)c->cascaded_tmp, c->cascaded_tmpStride, srcSliceY, srcSliceH, c->cascaded1_tmp, c->cascaded1_tmpStride);
        else
            ret = sws_scale(c->cascaded_context[1], (const uint8_t * const *)c->cascaded_tmp, c->cascaded_tmpStride, srcSliceY, srcSliceH, dst, dstStride);

        if (ret < 0)
            return ret;

        if (c->cascaded_context[2]) {
            ret = sws_scale(c->cascaded_context[2],
                        (const uint8_t * const *)c->cascaded1_tmp, c->cascaded1_tmpStride, c->cascaded_context[1]->dstY - ret, c->cascaded_context[1]->dstY,
                        dst, dstStride);
        }
        return ret;
    }

    if (c->cascaded_context[0] && srcSliceY == 0 && srcSliceH == c->cascaded_context[0]->srcH) {
        ret = sws_scale(c->cascaded_context[0],
                        srcSlice, srcStride, srcSliceY, srcSliceH,
                        c->cascaded_tmp, c->cascaded_tmpStride);
        if (ret < 0)
            return ret;
        ret = sws_scale(c->cascaded_context[1],
                        (const uint8_t * const * )c->cascaded_tmp, c->cascaded_tmpStride, 0, c->cascaded_context[0]->dstH,
                        dst, dstStride);
        return ret;
    }

    memcpy(src2, srcSlice, sizeof(src2));
    memcpy(dst2, dst, sizeof(dst2));

    // do not mess up sliceDir if we have a "trailing" 0-size slice
    if (srcSliceH == 0)
        return 0;

    if (!check_image_pointers(srcSlice, c->srcFormat, srcStride)) {
        av_log(c, AV_LOG_ERROR, "bad src image pointers\n");
        return 0;
    }
    if (!check_image_pointers((const uint8_t* const*)dst, c->dstFormat, dstStride)) {
        av_log(c, AV_LOG_ERROR, "bad dst image pointers\n");
        return 0;
    }

    if (c->sliceDir == 0 && srcSliceY != 0 && srcSliceY + srcSliceH != c->srcH) {
        av_log(c, AV_LOG_ERROR, "Slices start in the middle!\n");
        return 0;
    }
    if (c->sliceDir == 0) {
        if (srcSliceY == 0) c->sliceDir = 1; else c->sliceDir = -1;
    }

    if (usePal(c->srcFormat)) {
        for (i = 0; i < 256; i++) {
            int r, g, b, y, u, v, a = 0xff;
            if (c->srcFormat == AV_PIX_FMT_PAL8) {
                uint32_t p = ((const uint32_t *)(srcSlice[1]))[i];
                a = (p >> 24) & 0xFF;
                r = (p >> 16) & 0xFF;
                g = (p >>  8) & 0xFF;
                b =  p        & 0xFF;
            } else if (c->srcFormat == AV_PIX_FMT_RGB8) {
                r = ( i >> 5     ) * 36;
                g = ((i >> 2) & 7) * 36;
                b = ( i       & 3) * 85;
            } else if (c->srcFormat == AV_PIX_FMT_BGR8) {
                b = ( i >> 6     ) * 85;
                g = ((i >> 3) & 7) * 36;
                r = ( i       & 7) * 36;
            } else if (c->srcFormat == AV_PIX_FMT_RGB4_BYTE) {
                r = ( i >> 3     ) * 255;
                g = ((i >> 1) & 3) * 85;
                b = ( i       & 1) * 255;
            } else if (c->srcFormat == AV_PIX_FMT_GRAY8 || c->srcFormat == AV_PIX_FMT_GRAY8A) {
                r = g = b = i;
            } else {
                av_assert1(c->srcFormat == AV_PIX_FMT_BGR4_BYTE);
                b = ( i >> 3     ) * 255;
                g = ((i >> 1) & 3) * 85;
                r = ( i       & 1) * 255;
            }
#define RGB2YUV_SHIFT 15
#define BY ( (int) (0.114 * 219 / 255 * (1 << RGB2YUV_SHIFT) + 0.5))
#define BV (-(int) (0.081 * 224 / 255 * (1 << RGB2YUV_SHIFT) + 0.5))
#define BU ( (int) (0.500 * 224 / 255 * (1 << RGB2YUV_SHIFT) + 0.5))
#define GY ( (int) (0.587 * 219 / 255 * (1 << RGB2YUV_SHIFT) + 0.5))
#define GV (-(int) (0.419 * 224 / 255 * (1 << RGB2YUV_SHIFT) + 0.5))
#define GU (-(int) (0.331 * 224 / 255 * (1 << RGB2YUV_SHIFT) + 0.5))
#define RY ( (int) (0.299 * 219 / 255 * (1 << RGB2YUV_SHIFT) + 0.5))
#define RV ( (int) (0.500 * 224 / 255 * (1 << RGB2YUV_SHIFT) + 0.5))
#define RU (-(int) (0.169 * 224 / 255 * (1 << RGB2YUV_SHIFT) + 0.5))

            y = av_clip_uint8((RY * r + GY * g + BY * b + ( 33 << (RGB2YUV_SHIFT - 1))) >> RGB2YUV_SHIFT);
            u = av_clip_uint8((RU * r + GU * g + BU * b + (257 << (RGB2YUV_SHIFT - 1))) >> RGB2YUV_SHIFT);
            v = av_clip_uint8((RV * r + GV * g + BV * b + (257 << (RGB2YUV_SHIFT - 1))) >> RGB2YUV_SHIFT);
            c->pal_yuv[i]= y + (u<<8) + (v<<16) + ((unsigned)a<<24);

            switch (c->dstFormat) {
            case AV_PIX_FMT_BGR32:
#if !HAVE_BIGENDIAN
            case AV_PIX_FMT_RGB24:
#endif
                c->pal_rgb[i]=  r + (g<<8) + (b<<16) + ((unsigned)a<<24);
                break;
            case AV_PIX_FMT_BGR32_1:
#if HAVE_BIGENDIAN
            case AV_PIX_FMT_BGR24:
#endif
                c->pal_rgb[i]= a + (r<<8) + (g<<16) + ((unsigned)b<<24);
                break;
            case AV_PIX_FMT_RGB32_1:
#if HAVE_BIGENDIAN
            case AV_PIX_FMT_RGB24:
#endif
                c->pal_rgb[i]= a + (b<<8) + (g<<16) + ((unsigned)r<<24);
                break;
            case AV_PIX_FMT_RGB32:
#if !HAVE_BIGENDIAN
            case AV_PIX_FMT_BGR24:
#endif
            default:
                c->pal_rgb[i]=  b + (g<<8) + (r<<16) + ((unsigned)a<<24);
            }
        }
    }

    if (c->src0Alpha && !c->dst0Alpha && isALPHA(c->dstFormat)) {
        uint8_t *base;
        int x,y;
        rgb0_tmp = av_malloc(FFABS(srcStride[0]) * srcSliceH + 32);
        if (!rgb0_tmp)
            return AVERROR(ENOMEM);

        base = srcStride[0] < 0 ? rgb0_tmp - srcStride[0] * (srcSliceH-1) : rgb0_tmp;
        for (y=0; y<srcSliceH; y++){
            memcpy(base + srcStride[0]*y, src2[0] + srcStride[0]*y, 4*c->srcW);
            for (x=c->src0Alpha-1; x<4*c->srcW; x+=4) {
                base[ srcStride[0]*y + x] = 0xFF;
            }
        }
        src2[0] = base;
    }

    if (c->srcXYZ && !(c->dstXYZ && c->srcW==c->dstW && c->srcH==c->dstH)) {
        uint8_t *base;
        rgb0_tmp = av_malloc(FFABS(srcStride[0]) * srcSliceH + 32);
        if (!rgb0_tmp)
            return AVERROR(ENOMEM);

        base = srcStride[0] < 0 ? rgb0_tmp - srcStride[0] * (srcSliceH-1) : rgb0_tmp;

        xyz12Torgb48(c, (uint16_t*)base, (const uint16_t*)src2[0], srcStride[0]/2, srcSliceH);
        src2[0] = base;
    }

    if (!srcSliceY && (c->flags & SWS_BITEXACT) && c->dither == SWS_DITHER_ED && c->dither_error[0])
        for (i = 0; i < 4; i++)
            memset(c->dither_error[i], 0, sizeof(c->dither_error[0][0]) * (c->dstW+2));

    if (c->sliceDir != 1) {
        // slices go from bottom to top => we flip the image internally
        for (i=0; i<4; i++) {
            srcStride2[i] *= -1;
            dstStride2[i] *= -1;
        }

        src2[0] += (srcSliceH - 1) * srcStride[0];
        if (!usePal(c->srcFormat))
            src2[1] += ((srcSliceH >> c->chrSrcVSubSample) - 1) * srcStride[1];
        src2[2] += ((srcSliceH >> c->chrSrcVSubSample) - 1) * srcStride[2];
        src2[3] += (srcSliceH - 1) * srcStride[3];
        dst2[0] += ( c->dstH                         - 1) * dstStride[0];
        dst2[1] += ((c->dstH >> c->chrDstVSubSample) - 1) * dstStride[1];
        dst2[2] += ((c->dstH >> c->chrDstVSubSample) - 1) * dstStride[2];
        dst2[3] += ( c->dstH                         - 1) * dstStride[3];

        srcSliceY_internal = c->srcH-srcSliceY-srcSliceH;
    }
    reset_ptr(src2, c->srcFormat);
    reset_ptr((void*)dst2, c->dstFormat);

    /* reset slice direction at end of frame */
    if (srcSliceY_internal + srcSliceH == c->srcH)
        c->sliceDir = 0;
    ret = c->swscale(c, src2, srcStride2, srcSliceY_internal, srcSliceH, dst2, dstStride2);

    if (c->dstXYZ && !(c->srcXYZ && c->srcW==c->dstW && c->srcH==c->dstH)) {
        int dstY = c->dstY ? c->dstY : srcSliceY + srcSliceH;
        uint16_t *dst16 = (uint16_t*)(dst2[0] + (dstY - ret) * dstStride2[0]);
        av_assert0(dstY >= ret);
        av_assert0(ret >= 0);
        av_assert0(c->dstH >= dstY);

        /* replace on the same data */
        rgb48Toxyz12(c, dst16, dst16, dstStride2[0]/2, ret);
    }

    av_free(rgb0_tmp);
    return ret;
}


static int upload_texture(SDL_Texture **tex, AVFrame *frame, struct SwsContext **img_convert_ctx)
{
    int ret = 0;
    Uint32 sdl_pix_fmt;
    SDL_BlendMode sdl_blendmode;
    get_sdl_pix_fmt_and_blendmode(frame->format, &sdl_pix_fmt, &sdl_blendmode);
    if (realloc_texture(tex, sdl_pix_fmt == SDL_PIXELFORMAT_UNKNOWN ? SDL_PIXELFORMAT_ARGB8888 : sdl_pix_fmt, frame->width, frame->height, sdl_blendmode, 0) < 0)
        return -1;
    switch (sdl_pix_fmt)
    {
    case SDL_PIXELFORMAT_UNKNOWN:
        /* This should only happen if we are not using avfilter... */
        *img_convert_ctx = sws_getCachedContext(*img_convert_ctx,
                                                frame->width, frame->height, frame->format, frame->width, frame->height,
                                                AV_PIX_FMT_BGRA, sws_flags, NULL, NULL, NULL);
        if (*img_convert_ctx != NULL)
        {
            uint8_t *pixels[4];
            int pitch[4];
            if (!SDL_LockTexture(*tex, NULL, (void **)pixels, pitch))
            {
                sws_scale(*img_convert_ctx, (const uint8_t *const *)frame->data, frame->linesize,
                          0, frame->height, pixels, pitch);
                SDL_UnlockTexture(*tex);
            }
        }
        else
        {
            av_log(NULL, AV_LOG_FATAL, "Cannot initialize the conversion context\n");
            ret = -1;
        }
        break;
    case SDL_PIXELFORMAT_IYUV:
        if (frame->linesize[0] > 0 && frame->linesize[1] > 0 && frame->linesize[2] > 0)
        {
            ret = SDL_UpdateYUVTexture(*tex, NULL, frame->data[0], frame->linesize[0],
                                       frame->data[1], frame->linesize[1],
                                       frame->data[2], frame->linesize[2]);
        }
        else if (frame->linesize[0] < 0 && frame->linesize[1] < 0 && frame->linesize[2] < 0)
        {
            ret = SDL_UpdateYUVTexture(*tex, NULL, frame->data[0] + frame->linesize[0] * (frame->height - 1), -frame->linesize[0],
                                       frame->data[1] + frame->linesize[1] * (AV_CEIL_RSHIFT(frame->height, 1) - 1), -frame->linesize[1],
                                       frame->data[2] + frame->linesize[2] * (AV_CEIL_RSHIFT(frame->height, 1) - 1), -frame->linesize[2]);
        }
        else
        {
            av_log(NULL, AV_LOG_ERROR, "Mixed negative and positive linesizes are not supported.\n");
            return -1;
        }
        break;
    default:
        if (frame->linesize[0] < 0)
        {
            ret = SDL_UpdateTexture(*tex, NULL, frame->data[0] + frame->linesize[0] * (frame->height - 1), -frame->linesize[0]);
        }
        else
        {
            ret = SDL_UpdateTexture(*tex, NULL, frame->data[0], frame->linesize[0]);
        }
        break;
    }
    return ret;
}

static void set_sdl_yuv_conversion_mode(AVFrame *frame)
{
    SDL_YUV_CONVERSION_MODE mode = SDL_YUV_CONVERSION_AUTOMATIC;
    if (frame && (frame->format == AV_PIX_FMT_YUV420P || frame->format == AV_PIX_FMT_YUYV422 || frame->format == AV_PIX_FMT_UYVY422))
    {
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
    Frame *sp = NULL;
    SDL_Rect rect;

    vp = frame_queue_peek_last(&is->pictq);
    if (is->subtitle_st)
    {
        if (frame_queue_nb_remaining(&is->subpq) > 0)
        {
            sp = frame_queue_peek(&is->subpq);

            if (vp->pts >= sp->pts + ((float)sp->sub.start_display_time / 1000))
            {
                if (!sp->uploaded)
                {
                    uint8_t *pixels[4];
                    int pitch[4];
                    int i;
                    if (!sp->width || !sp->height)
                    {
                        sp->width = vp->width;
                        sp->height = vp->height;
                    }
                    if (realloc_texture(&is->sub_texture, SDL_PIXELFORMAT_ARGB8888, sp->width, sp->height, SDL_BLENDMODE_BLEND, 1) < 0)
                        return;

                    for (i = 0; i < sp->sub.num_rects; i++)
                    {
                        AVSubtitleRect *sub_rect = sp->sub.rects[i];

                        sub_rect->x = av_clip(sub_rect->x, 0, sp->width);
                        sub_rect->y = av_clip(sub_rect->y, 0, sp->height);
                        sub_rect->w = av_clip(sub_rect->w, 0, sp->width - sub_rect->x);
                        sub_rect->h = av_clip(sub_rect->h, 0, sp->height - sub_rect->y);

                        is->sub_convert_ctx = sws_getCachedContext(is->sub_convert_ctx,
                                                                   sub_rect->w, sub_rect->h, AV_PIX_FMT_PAL8,
                                                                   sub_rect->w, sub_rect->h, AV_PIX_FMT_BGRA,
                                                                   0, NULL, NULL, NULL);
                        if (!is->sub_convert_ctx)
                        {
                            av_log(NULL, AV_LOG_FATAL, "Cannot initialize the conversion context\n");
                            return;
                        }
                        if (!SDL_LockTexture(is->sub_texture, (SDL_Rect *)sub_rect, (void **)pixels, pitch))
                        {
                            sws_scale(is->sub_convert_ctx, (const uint8_t *const *)sub_rect->data, sub_rect->linesize,
                                      0, sub_rect->h, pixels, pitch);
                            SDL_UnlockTexture(is->sub_texture);
                        }
                    }
                    sp->uploaded = 1;
                }
            }
            else
                sp = NULL;
        }
    }

    calculate_display_rect(&rect, is->xleft, is->ytop, is->width, is->height, vp->width, vp->height, vp->sar);

    if (!vp->uploaded)
    {
        if (upload_texture(&is->vid_texture, vp->frame, &is->img_convert_ctx) < 0)
            return;
        vp->uploaded = 1;
        vp->flip_v = vp->frame->linesize[0] < 0;
    }

    set_sdl_yuv_conversion_mode(vp->frame);
    SDL_RenderCopyEx(renderer, is->vid_texture, NULL, &rect, 0, NULL, vp->flip_v ? SDL_FLIP_VERTICAL : 0);
    set_sdl_yuv_conversion_mode(NULL);
    if (sp)
    {
        // #if USE_ONEPASS_SUBTITLE_RENDER
        //         SDL_RenderCopy(renderer, is->sub_texture, NULL, &rect);
        // #else
        int i;
        double xratio = (double)rect.w / (double)sp->width;
        double yratio = (double)rect.h / (double)sp->height;
        for (i = 0; i < sp->sub.num_rects; i++)
        {
            SDL_Rect *sub_rect = (SDL_Rect *)sp->sub.rects[i];
            SDL_Rect target = {.x = rect.x + sub_rect->x * xratio,
                               .y = rect.y + sub_rect->y * yratio,
                               .w = sub_rect->w * xratio,
                               .h = sub_rect->h * yratio};
            SDL_RenderCopy(renderer, is->sub_texture, sub_rect, &target);
        }
        // #endif
    }
}

static inline int compute_mod(int a, int b)
{
    return a < 0 ? a % b + b : a % b;
}

int64_t av_gettime(void)
{
#if HAVE_GETTIMEOFDAY
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (int64_t)tv.tv_sec * 1000000 + tv.tv_usec;
#elif HAVE_GETSYSTEMTIMEASFILETIME
    FILETIME ft;
    int64_t t;
    GetSystemTimeAsFileTime(&ft);
    t = (int64_t)ft.dwHighDateTime << 32 | ft.dwLowDateTime;
    return t / 10 - 11644473600000000; /* Jan 1, 1601 */
#else
    return -1;
#endif
}

int64_t av_gettime_relative(void)
{
#if HAVE_CLOCK_GETTIME && defined(CLOCK_MONOTONIC)
#ifdef __APPLE__
    if (clock_gettime)
#endif
    {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return (int64_t)ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
    }
#endif
    return av_gettime() + 42 * 60 * 60 * INT64_C(1000000);
}

av_cold void av_rdft_end(RDFTContext *s)
{
    if (s) {
        ff_rdft_end(s);
        av_free(s);
    }
}


RDFTContext *av_rdft_init(int nbits, enum RDFTransformType trans)
{
    RDFTContext *s = av_malloc(sizeof(*s));

    if (s && ff_rdft_init(s, nbits, trans))
        av_freep(&s);

    return s;
}

void *av_malloc_array(size_t nmemb, size_t size)
{
    size_t result;
    if (av_size_mult(nmemb, size, &result) < 0)
        return NULL;
    return av_malloc(result);
}

void *av_mallocz_array(size_t nmemb, size_t size)
{
    size_t result;
    if (av_size_mult(nmemb, size, &result) < 0)
        return NULL;
    return av_mallocz(result);
}


void av_rdft_calc(RDFTContext *s, FFTSample *data)
{
    s->rdft_calc(s, data);
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

    /* compute display index : center on currently output samples */
    channels = s->audio_tgt.channels;
    nb_display_channels = channels;
    if (!s->paused)
    {
        int data_used = s->show_mode == SHOW_MODE_WAVES ? s->width : (2 * nb_freq);
        n = 2 * channels;
        delay = s->audio_write_buf_size;
        delay /= n;

        /* to be more precise, we take into account the time spent since
           the last buffer computation */
        if (audio_callback_time)
        {
            time_diff = av_gettime_relative() - audio_callback_time;
            delay -= (time_diff * s->audio_tgt.freq) / 1000000;
        }

        delay += 2 * data_used;
        if (delay < data_used)
            delay = data_used;

        i_start = x = compute_mod(s->sample_array_index - delay * channels, SAMPLE_ARRAY_SIZE);
        if (s->show_mode == SHOW_MODE_WAVES)
        {
            h = INT_MIN;
            for (i = 0; i < 1000; i += channels)
            {
                int idx = (SAMPLE_ARRAY_SIZE + x - i) % SAMPLE_ARRAY_SIZE;
                int a = s->sample_array[idx];
                int b = s->sample_array[(idx + 4 * channels) % SAMPLE_ARRAY_SIZE];
                int c = s->sample_array[(idx + 5 * channels) % SAMPLE_ARRAY_SIZE];
                int d = s->sample_array[(idx + 9 * channels) % SAMPLE_ARRAY_SIZE];
                int score = a - d;
                if (h < score && (b ^ c) < 0)
                {
                    h = score;
                    i_start = idx;
                }
            }
        }

        s->last_i_start = i_start;
    }
    else
    {
        i_start = s->last_i_start;
    }

    if (s->show_mode == SHOW_MODE_WAVES)
    {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

        /* total height for one channel */
        h = s->height / nb_display_channels;
        /* graph height / 2 */
        h2 = (h * 9) / 20;
        for (ch = 0; ch < nb_display_channels; ch++)
        {
            i = i_start + ch;
            y1 = s->ytop + ch * h + (h / 2); /* position of center line */
            for (x = 0; x < s->width; x++)
            {
                y = (s->sample_array[i] * h2) >> 15;
                if (y < 0)
                {
                    y = -y;
                    ys = y1 - y;
                }
                else
                {
                    ys = y1;
                }
                fill_rectangle(s->xleft + x, ys, 1, y);
                i += channels;
                if (i >= SAMPLE_ARRAY_SIZE)
                    i -= SAMPLE_ARRAY_SIZE;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);

        for (ch = 1; ch < nb_display_channels; ch++)
        {
            y = s->ytop + ch * h;
            fill_rectangle(s->xleft, y, s->width, 1);
        }
    }
    else
    {
        if (realloc_texture(&s->vis_texture, SDL_PIXELFORMAT_ARGB8888, s->width, s->height, SDL_BLENDMODE_NONE, 1) < 0)
            return;

        nb_display_channels = FFMIN(nb_display_channels, 2);
        if (rdft_bits != s->rdft_bits)
        {
            av_rdft_end(s->rdft);
            av_free(s->rdft_data);
            s->rdft = av_rdft_init(rdft_bits, DFT_R2C);
            s->rdft_bits = rdft_bits;
            s->rdft_data = av_malloc_array(nb_freq, 4 * sizeof(*s->rdft_data));
        }
        if (!s->rdft || !s->rdft_data)
        {
            av_log(NULL, AV_LOG_ERROR, "Failed to allocate buffers for RDFT, switching to waves display\n");
            s->show_mode = SHOW_MODE_WAVES;
        }
        else
        {
            FFTSample *data[2];
            SDL_Rect rect = {.x = s->xpos, .y = 0, .w = 1, .h = s->height};
            uint32_t *pixels;
            int pitch;
            for (ch = 0; ch < nb_display_channels; ch++)
            {
                data[ch] = s->rdft_data + 2 * nb_freq * ch;
                i = i_start + ch;
                for (x = 0; x < 2 * nb_freq; x++)
                {
                    double w = (x - nb_freq) * (1.0 / nb_freq);
                    data[ch][x] = s->sample_array[i] * (1.0 - w * w);
                    i += channels;
                    if (i >= SAMPLE_ARRAY_SIZE)
                        i -= SAMPLE_ARRAY_SIZE;
                }
                av_rdft_calc(s->rdft, data[ch]);
            }
            /* Least efficient way to do this, we should of course
             * directly access it but it is more than fast enough. */
            if (!SDL_LockTexture(s->vis_texture, &rect, (void **)&pixels, &pitch))
            {
                pitch >>= 2;
                pixels += pitch * s->height;
                for (y = 0; y < s->height; y++)
                {
                    double w = 1 / sqrt(nb_freq);
                    int a = sqrt(w * sqrt(data[0][2 * y + 0] * data[0][2 * y + 0] + data[0][2 * y + 1] * data[0][2 * y + 1]));
                    int b = (nb_display_channels == 2) ? sqrt(w * hypot(data[1][2 * y + 0], data[1][2 * y + 1]))
                                                       : a;
                    a = FFMIN(a, 255);
                    b = FFMIN(b, 255);
                    pixels -= pitch;
                    *pixels = (a << 16) + (b << 8) + ((a + b) >> 1);
                }
                SDL_UnlockTexture(s->vis_texture);
            }
            SDL_RenderCopy(renderer, s->vis_texture, NULL, NULL);
        }
        if (!s->paused)
            s->xpos++;
        if (s->xpos >= s->width)
            s->xpos = s->xleft;
    }
}

av_cold void swr_free(SwrContext **ss){
    SwrContext *s= *ss;
    if(s){
        clear_context(s);
        if (s->resampler)
            s->resampler->free(&s->resample);
    }

    av_freep(ss);
}

static void stream_component_close(VideoState *is, int stream_index)
{
    AVFormatContext *ic = is->ic;
    AVCodecParameters *codecpar;

    if (stream_index < 0 || stream_index >= ic->nb_streams)
        return;
    codecpar = ic->streams[stream_index]->codecpar;

    switch (codecpar->codec_type)
    {
    case AVMEDIA_TYPE_AUDIO:
        decoder_abort(&is->auddec, &is->sampq);
        SDL_CloseAudioDevice(audio_dev);
        decoder_destroy(&is->auddec);
        swr_free(&is->swr_ctx);
        av_freep(&is->audio_buf1);
        is->audio_buf1_size = 0;
        is->audio_buf = NULL;

        if (is->rdft)
        {
            av_rdft_end(is->rdft);
            av_freep(&is->rdft_data);
            is->rdft = NULL;
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
    switch (codecpar->codec_type)
    {
    case AVMEDIA_TYPE_AUDIO:
        is->audio_st = NULL;
        is->audio_stream = -1;
        break;
    case AVMEDIA_TYPE_VIDEO:
        is->video_st = NULL;
        is->video_stream = -1;
        break;
    case AVMEDIA_TYPE_SUBTITLE:
        is->subtitle_st = NULL;
        is->subtitle_stream = -1;
        break;
    default:
        break;
    }
}

void avpriv_packet_list_free(AVPacketList **pkt_buf, AVPacketList **pkt_buf_end)
{
    AVPacketList *tmp = *pkt_buf;

    while (tmp) {
        AVPacketList *pktl = tmp;
        tmp = pktl->next;
        av_packet_unref(&pktl->pkt);
        av_freep(&pktl);
    }
    *pkt_buf     = NULL;
    *pkt_buf_end = NULL;
}

static void flush_packet_queue(AVFormatContext *s)
{
    if (!s->internal)
        return;
    avpriv_packet_list_free(&s->internal->parse_queue,       &s->internal->parse_queue_end);
    avpriv_packet_list_free(&s->internal->packet_buffer,     &s->internal->packet_buffer_end);
    avpriv_packet_list_free(&s->internal->raw_packet_buffer, &s->internal->raw_packet_buffer_end);

    s->internal->raw_packet_buffer_remaining_size = RAW_PACKET_BUFFER_SIZE;
}

void avformat_close_input(AVFormatContext **ps)
{
    AVFormatContext *s;
    AVIOContext *pb;

    if (!ps || !*ps)
        return;

    s  = *ps;
    pb = s->pb;

    if ((s->iformat && strcmp(s->iformat->name, "image2") && s->iformat->flags & AVFMT_NOFILE) ||
        (s->flags & AVFMT_FLAG_CUSTOM_IO))
        pb = NULL;

    flush_packet_queue(s);

    if (s->iformat)
        if (s->iformat->read_close)
            s->iformat->read_close(s);

    avformat_free_context(s);

    *ps = NULL;

    avio_close(pb);
}

static void stream_close(VideoState *is)
{
    /* XXX: use a special url_shutdown call to abort parse cleanly */
    is->abort_request = 1;
    SDL_WaitThread(is->read_tid, NULL);

    /* close each stream */
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

    /* free all pictures */
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

void ff_network_close(void)
{
#if HAVE_WINSOCK2_H
    WSACleanup();
#endif
}

int avformat_network_deinit(void)
{
    ff_network_close();
    return 0;
}

static void do_exit(VideoState *is)
{
    if (is)
    {
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
    av_log(NULL, AV_LOG_QUIET, "%s", "");
    exit(0);
}

static void sigterm_handler(int sig)
{
    exit(123);
}

static void set_default_window_size(int width, int height, AVRational sar)
{
    SDL_Rect rect;
    int max_width = screen_width ? screen_width : INT_MAX;
    int max_height = screen_height ? screen_height : INT_MAX;
    if (max_width == INT_MAX && max_height == INT_MAX)
        max_height = height;
    calculate_display_rect(&rect, 0, 0, max_width, max_height, width, height, sar);
    default_width = rect.w;
    default_height = rect.h;
}

static int video_open(VideoState *is)
{
    int w, h;

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

/* display the current picture, if any */
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
        return NAN;
    if (c->paused)
    {
        return c->pts;
    }
    else
    {
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
    set_clock(c, NAN, -1);
}

static void sync_clock_to_slave(Clock *c, Clock *slave)
{
    double clock = get_clock(c);
    double slave_clock = get_clock(slave);
    if (!isnan(slave_clock) && (isnan(clock) || fabs(clock - slave_clock) > AV_NOSYNC_THRESHOLD))
        set_clock(c, slave_clock, slave->serial);
}

static int get_master_sync_type(VideoState *is)
{
    if (is->av_sync_type == AV_SYNC_VIDEO_MASTER)
    {
        if (is->video_st)
            return AV_SYNC_VIDEO_MASTER;
        else
            return AV_SYNC_AUDIO_MASTER;
    }
    else if (is->av_sync_type == AV_SYNC_AUDIO_MASTER)
    {
        if (is->audio_st)
            return AV_SYNC_AUDIO_MASTER;
        else
            return AV_SYNC_EXTERNAL_CLOCK;
    }
    else
    {
        return AV_SYNC_EXTERNAL_CLOCK;
    }
}

/* get the current master clock value */
static double get_master_clock(VideoState *is)
{
    double val;

    switch (get_master_sync_type(is))
    {
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

static void check_external_clock_speed(VideoState *is)
{
    if (is->video_stream >= 0 && is->videoq.nb_packets <= EXTERNAL_CLOCK_MIN_FRAMES ||
        is->audio_stream >= 0 && is->audioq.nb_packets <= EXTERNAL_CLOCK_MIN_FRAMES)
    {
        set_clock_speed(&is->extclk, FFMAX(EXTERNAL_CLOCK_SPEED_MIN, is->extclk.speed - EXTERNAL_CLOCK_SPEED_STEP));
    }
    else if ((is->video_stream < 0 || is->videoq.nb_packets > EXTERNAL_CLOCK_MAX_FRAMES) &&
             (is->audio_stream < 0 || is->audioq.nb_packets > EXTERNAL_CLOCK_MAX_FRAMES))
    {
        set_clock_speed(&is->extclk, FFMIN(EXTERNAL_CLOCK_SPEED_MAX, is->extclk.speed + EXTERNAL_CLOCK_SPEED_STEP));
    }
    else
    {
        double speed = is->extclk.speed;
        if (speed != 1.0)
            set_clock_speed(&is->extclk, speed + EXTERNAL_CLOCK_SPEED_STEP * (1.0 - speed) / fabs(1.0 - speed));
    }
}

/* seek in the stream */
static void stream_seek(VideoState *is, int64_t pos, int64_t rel, int seek_by_bytes)
{
    if (!is->seek_req)
    {
        is->seek_pos = pos;
        is->seek_rel = rel;
        is->seek_flags &= ~AVSEEK_FLAG_BYTE;
        if (seek_by_bytes)
            is->seek_flags |= AVSEEK_FLAG_BYTE;
        is->seek_req = 1;
        SDL_CondSignal(is->continue_read_thread);
    }
}

/* pause or resume the video */
static void stream_toggle_pause(VideoState *is)
{
    if (is->paused)
    {
        is->frame_timer += av_gettime_relative() / 1000000.0 - is->vidclk.last_updated;
        if (is->read_pause_return != AVERROR(ENOSYS))
        {
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
    double volume_level = is->audio_volume ? (20 * log(is->audio_volume / (double)SDL_MIX_MAXVOLUME) / log(10)) : -1000.0;
    int new_volume = lrint(SDL_MIX_MAXVOLUME * pow(10.0, (volume_level + sign * step) / 20.0));
    is->audio_volume = av_clip(is->audio_volume == new_volume ? (is->audio_volume + sign) : new_volume, 0, SDL_MIX_MAXVOLUME);
}

static void step_to_next_frame(VideoState *is)
{
    /* if the stream is paused unpause it, then step */
    if (is->paused)
        stream_toggle_pause(is);
    is->step = 1;
}

static double compute_target_delay(double delay, VideoState *is)
{
    double sync_threshold, diff = 0;

    /* update delay to follow master synchronisation source */
    if (get_master_sync_type(is) != AV_SYNC_VIDEO_MASTER)
    {
        /* if video is slave, we try to correct big delays by
           duplicating or deleting a frame */
        diff = get_clock(&is->vidclk) - get_master_clock(is);

        /* skip or repeat frame. We take into account the
           delay to compute the threshold. I still don't know
           if it is the best guess */
        sync_threshold = FFMAX(AV_SYNC_THRESHOLD_MIN, FFMIN(AV_SYNC_THRESHOLD_MAX, delay));
        if (!isnan(diff) && fabs(diff) < is->max_frame_duration)
        {
            if (diff <= -sync_threshold)
                delay = FFMAX(0, delay + diff);
            else if (diff >= sync_threshold && delay > AV_SYNC_FRAMEDUP_THRESHOLD)
                delay = delay + diff;
            else if (diff >= sync_threshold)
                delay = 2 * delay;
        }
    }

    av_log(NULL, AV_LOG_TRACE, "video: delay=%0.3f A-V=%f\n",
           delay, -diff);

    return delay;
}

static double vp_duration(VideoState *is, Frame *vp, Frame *nextvp)
{
    if (vp->serial == nextvp->serial)
    {
        double duration = nextvp->pts - vp->pts;
        if (isnan(duration) || duration <= 0 || duration > is->max_frame_duration)
            return vp->duration;
        else
            return duration;
    }
    else
    {
        return 0.0;
    }
}

static void update_video_pts(VideoState *is, double pts, int64_t pos, int serial)
{
    /* update current video pts */
    set_clock(&is->vidclk, pts, serial);
    sync_clock_to_slave(&is->extclk, &is->vidclk);
}

/* called to display each frame */
static void video_refresh(void *opaque, double *remaining_time)
{
    VideoState *is = opaque;
    double time;

    Frame *sp, *sp2;

    if (!is->paused && get_master_sync_type(is) == AV_SYNC_EXTERNAL_CLOCK && is->realtime)
        check_external_clock_speed(is);

    if (!display_disable && is->show_mode != SHOW_MODE_VIDEO && is->audio_st)
    {
        time = av_gettime_relative() / 1000000.0;
        if (is->force_refresh || is->last_vis_time + rdftspeed < time)
        {
            video_display(is);
            is->last_vis_time = time;
        }
        *remaining_time = FFMIN(*remaining_time, is->last_vis_time + rdftspeed - time);
    }

    if (is->video_st)
    {
    retry:
        if (frame_queue_nb_remaining(&is->pictq) == 0)
        {
            // nothing to do, no picture to display in the queue
        }
        else
        {
            double last_duration, duration, delay;
            Frame *vp, *lastvp;

            /* dequeue the picture */
            lastvp = frame_queue_peek_last(&is->pictq);
            vp = frame_queue_peek(&is->pictq);

            if (vp->serial != is->videoq.serial)
            {
                frame_queue_next(&is->pictq);
                goto retry;
            }

            if (lastvp->serial != vp->serial)
                is->frame_timer = av_gettime_relative() / 1000000.0;

            if (is->paused)
                goto display;

            /* compute nominal last_duration */
            last_duration = vp_duration(is, lastvp, vp);
            delay = compute_target_delay(last_duration, is);

            time = av_gettime_relative() / 1000000.0;
            if (time < is->frame_timer + delay)
            {
                *remaining_time = FFMIN(is->frame_timer + delay - time, *remaining_time);
                goto display;
            }

            is->frame_timer += delay;
            if (delay > 0 && time - is->frame_timer > AV_SYNC_THRESHOLD_MAX)
                is->frame_timer = time;

            SDL_LockMutex(is->pictq.mutex);
            if (!isnan(vp->pts))
                update_video_pts(is, vp->pts, vp->pos, vp->serial);
            SDL_UnlockMutex(is->pictq.mutex);

            if (frame_queue_nb_remaining(&is->pictq) > 1)
            {
                Frame *nextvp = frame_queue_peek_next(&is->pictq);
                duration = vp_duration(is, vp, nextvp);
                if (!is->step && (framedrop > 0 || (framedrop && get_master_sync_type(is) != AV_SYNC_VIDEO_MASTER)) && time > is->frame_timer + duration)
                {
                    is->frame_drops_late++;
                    frame_queue_next(&is->pictq);
                    goto retry;
                }
            }

            if (is->subtitle_st)
            {
                while (frame_queue_nb_remaining(&is->subpq) > 0)
                {
                    sp = frame_queue_peek(&is->subpq);

                    if (frame_queue_nb_remaining(&is->subpq) > 1)
                        sp2 = frame_queue_peek_next(&is->subpq);
                    else
                        sp2 = NULL;

                    if (sp->serial != is->subtitleq.serial || (is->vidclk.pts > (sp->pts + ((float)sp->sub.end_display_time / 1000))) || (sp2 && is->vidclk.pts > (sp2->pts + ((float)sp2->sub.start_display_time / 1000))))
                    {
                        if (sp->uploaded)
                        {
                            int i;
                            for (i = 0; i < sp->sub.num_rects; i++)
                            {
                                AVSubtitleRect *sub_rect = sp->sub.rects[i];
                                uint8_t *pixels;
                                int pitch, j;

                                if (!SDL_LockTexture(is->sub_texture, (SDL_Rect *)sub_rect, (void **)&pixels, &pitch))
                                {
                                    for (j = 0; j < sub_rect->h; j++, pixels += pitch)
                                        memset(pixels, 0, sub_rect->w << 2);
                                    SDL_UnlockTexture(is->sub_texture);
                                }
                            }
                        }
                        frame_queue_next(&is->subpq);
                    }
                    else
                    {
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
        /* display picture */
        if (!display_disable && is->force_refresh && is->show_mode == SHOW_MODE_VIDEO && is->pictq.rindex_shown)
            video_display(is);
    }
    is->force_refresh = 0;
    if (show_status)
    {
        AVBPrint buf;
        static int64_t last_time;
        int64_t cur_time;
        int aqsize, vqsize, sqsize;
        double av_diff;

        cur_time = av_gettime_relative();
        if (!last_time || (cur_time - last_time) >= 30000)
        {
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

            av_bprint_init(&buf, 0, AV_BPRINT_SIZE_AUTOMATIC);
            av_bprintf(&buf,
                       "%7.2f %s:%7.3f fd=%4d aq=%5dKB vq=%5dKB sq=%5dB f=%" PRId64 "/%" PRId64 "   \r",
                       get_master_clock(is),
                       (is->audio_st && is->video_st) ? "A-V" : (is->video_st ? "M-V" : (is->audio_st ? "M-A" : "   ")),
                       av_diff,
                       is->frame_drops_early + is->frame_drops_late,
                       aqsize / 1024,
                       vqsize / 1024,
                       sqsize,
                       is->video_st ? is->viddec.avctx->pts_correction_num_faulty_dts : 0,
                       is->video_st ? is->viddec.avctx->pts_correction_num_faulty_pts : 0);

            if (show_status == 1 && AV_LOG_INFO > av_log_get_level())
                fprintf(stderr, "%s", buf.str);
            else
                av_log(NULL, AV_LOG_INFO, "%s", buf.str);

            fflush(stderr);
            av_bprint_finalize(&buf, NULL);

            last_time = cur_time;
        }
    }
}

static int queue_picture(VideoState *is, AVFrame *src_frame, double pts, double duration, int64_t pos, int serial)
{
    Frame *vp;

    // #if defined(DEBUG_SYNC)
    //     printf("frame_type=%c pts=%0.3f\n",
    //            av_get_picture_type_char(src_frame->pict_type), pts);
    // #endif

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

AVRational av_guess_sample_aspect_ratio(AVFormatContext *format, AVStream *stream, AVFrame *frame)
{
    AVRational undef = {0, 1};
    AVRational stream_sample_aspect_ratio = stream ? stream->sample_aspect_ratio : undef;
    AVRational codec_sample_aspect_ratio = stream && stream->codecpar ? stream->codecpar->sample_aspect_ratio : undef;
    AVRational frame_sample_aspect_ratio = frame ? frame->sample_aspect_ratio : codec_sample_aspect_ratio;

    av_reduce(&stream_sample_aspect_ratio.num, &stream_sample_aspect_ratio.den,
              stream_sample_aspect_ratio.num, stream_sample_aspect_ratio.den, INT_MAX);
    if (stream_sample_aspect_ratio.num <= 0 || stream_sample_aspect_ratio.den <= 0)
        stream_sample_aspect_ratio = undef;

    av_reduce(&frame_sample_aspect_ratio.num, &frame_sample_aspect_ratio.den,
              frame_sample_aspect_ratio.num, frame_sample_aspect_ratio.den, INT_MAX);
    if (frame_sample_aspect_ratio.num <= 0 || frame_sample_aspect_ratio.den <= 0)
        frame_sample_aspect_ratio = undef;

    if (stream_sample_aspect_ratio.num)
        return stream_sample_aspect_ratio;
    else
        return frame_sample_aspect_ratio;
}

static inline double av_q2d(AVRational a)
{
    return a.num / (double)a.den;
}

static int get_video_frame(VideoState *is, AVFrame *frame)
{
    int got_picture;

    if ((got_picture = decoder_decode_frame(&is->viddec, frame, NULL)) < 0)
        return -1;

    if (got_picture)
    {
        double dpts = NAN;

        if (frame->pts != AV_NOPTS_VALUE)
            dpts = av_q2d(is->video_st->time_base) * frame->pts;

        frame->sample_aspect_ratio = av_guess_sample_aspect_ratio(is->ic, is->video_st, frame);

        if (framedrop > 0 || (framedrop && get_master_sync_type(is) != AV_SYNC_VIDEO_MASTER))
        {
            if (frame->pts != AV_NOPTS_VALUE)
            {
                double diff = dpts - get_master_clock(is);
                if (!isnan(diff) && fabs(diff) < AV_NOSYNC_THRESHOLD &&
                    diff - is->frame_last_filter_delay < 0 &&
                    is->viddec.pkt_serial == is->vidclk.serial &&
                    is->videoq.nb_packets)
                {
                    is->frame_drops_early++;
                    av_frame_unref(frame);
                    got_picture = 0;
                }
            }
        }
    }

    return got_picture;
}

char *av_strdup(const char *s)
{
    char *ptr = NULL;
    if (s)
    {
        size_t len = strlen(s) + 1;
        ptr = av_realloc(NULL, len);
        if (ptr)
            memcpy(ptr, s, len);
    }
    return ptr;
}

AVFilterInOut *avfilter_inout_alloc(void)
{
    return av_mallocz(sizeof(AVFilterInOut));
}

void avfilter_inout_free(AVFilterInOut **inout)
{
    while (*inout) {
        AVFilterInOut *next = (*inout)->next;
        av_freep(&(*inout)->name);
        av_freep(inout);
        *inout = next;
    }
}

int avfilter_graph_parse_ptr(AVFilterGraph *graph, const char *filters,
                         AVFilterInOut **open_inputs_ptr, AVFilterInOut **open_outputs_ptr,
                         void *log_ctx)
{
    int index = 0, ret = 0;
    char chr = 0;

    AVFilterInOut *curr_inputs = NULL;
    AVFilterInOut *open_inputs  = open_inputs_ptr  ? *open_inputs_ptr  : NULL;
    AVFilterInOut *open_outputs = open_outputs_ptr ? *open_outputs_ptr : NULL;

    if ((ret = parse_sws_flags(&filters, graph)) < 0)
        goto end;

    do {
        AVFilterContext *filter;
        const char *filterchain = filters;
        filters += strspn(filters, WHITESPACES);

        if ((ret = parse_inputs(&filters, &curr_inputs, &open_outputs, log_ctx)) < 0)
            goto end;

        if ((ret = parse_filter(&filter, &filters, graph, index, log_ctx)) < 0)
            goto end;

        if (filter->nb_inputs == 1 && !curr_inputs && !index) {
            /* First input pad, assume it is "[in]" if not specified */
            const char *tmp = "[in]";
            if ((ret = parse_inputs(&tmp, &curr_inputs, &open_outputs, log_ctx)) < 0)
                goto end;
        }

        if ((ret = link_filter_inouts(filter, &curr_inputs, &open_inputs, log_ctx)) < 0)
            goto end;

        if ((ret = parse_outputs(&filters, &curr_inputs, &open_inputs, &open_outputs,
                                 log_ctx)) < 0)
            goto end;

        filters += strspn(filters, WHITESPACES);
        chr = *filters++;

        if (chr == ';' && curr_inputs) {
            av_log(log_ctx, AV_LOG_ERROR,
                   "Invalid filterchain containing an unlabelled output pad: \"%s\"\n",
                   filterchain);
            ret = AVERROR(EINVAL);
            goto end;
        }
        index++;
    } while (chr == ',' || chr == ';');

    if (chr) {
        av_log(log_ctx, AV_LOG_ERROR,
               "Unable to parse graph description substring: \"%s\"\n",
               filters - 1);
        ret = AVERROR(EINVAL);
        goto end;
    }

    if (curr_inputs) {
        /* Last output pad, assume it is "[out]" if not specified */
        const char *tmp = "[out]";
        if ((ret = parse_outputs(&tmp, &curr_inputs, &open_inputs, &open_outputs,
                                 log_ctx)) < 0)
            goto end;
    }

end:
    /* clear open_in/outputs only if not passed as parameters */
    if (open_inputs_ptr) *open_inputs_ptr = open_inputs;
    else avfilter_inout_free(&open_inputs);
    if (open_outputs_ptr) *open_outputs_ptr = open_outputs;
    else avfilter_inout_free(&open_outputs);
    avfilter_inout_free(&curr_inputs);

    if (ret < 0) {
        while (graph->nb_filters)
            avfilter_free(graph->filters[0]);
        av_freep(&graph->filters);
    }
    return ret;
}

void ff_framequeue_init(FFFrameQueue *fq, FFFrameQueueGlobal *fqg)
{
    fq->queue = &fq->first_bucket;
    fq->allocated = 1;
}

static inline void *av_x_if_null(const void *p, const void *x)
{
    return (void *)(intptr_t)(p ? p : x);
}

const char *av_get_media_type_string(enum AVMediaType media_type)
{
    switch (media_type) {
    case AVMEDIA_TYPE_VIDEO:      return "video";
    case AVMEDIA_TYPE_AUDIO:      return "audio";
    case AVMEDIA_TYPE_DATA:       return "data";
    case AVMEDIA_TYPE_SUBTITLE:   return "subtitle";
    case AVMEDIA_TYPE_ATTACHMENT: return "attachment";
    default:                      return NULL;
    }
}

int avfilter_link(AVFilterContext *src, unsigned srcpad,
                  AVFilterContext *dst, unsigned dstpad)
{
    AVFilterLink *link;

    av_assert0(src->graph);
    av_assert0(dst->graph);
    av_assert0(src->graph == dst->graph);

    if (src->nb_outputs <= srcpad || dst->nb_inputs <= dstpad ||
        src->outputs[srcpad]      || dst->inputs[dstpad])
        return AVERROR(EINVAL);

    if (src->output_pads[srcpad].type != dst->input_pads[dstpad].type) {
        av_log(src, AV_LOG_ERROR,
               "Media type mismatch between the '%s' filter output pad %d (%s) and the '%s' filter input pad %d (%s)\n",
               src->name, srcpad, (char *)av_x_if_null(av_get_media_type_string(src->output_pads[srcpad].type), "?"),
               dst->name, dstpad, (char *)av_x_if_null(av_get_media_type_string(dst-> input_pads[dstpad].type), "?"));
        return AVERROR(EINVAL);
    }

    link = av_mallocz(sizeof(*link));
    if (!link)
        return AVERROR(ENOMEM);

    src->outputs[srcpad] = dst->inputs[dstpad] = link;

    link->src     = src;
    link->dst     = dst;
    link->srcpad  = &src->output_pads[srcpad];
    link->dstpad  = &dst->input_pads[dstpad];
    link->type    = src->output_pads[srcpad].type;
    av_assert0(AV_PIX_FMT_NONE == -1 && AV_SAMPLE_FMT_NONE == -1);
    link->format  = -1;
    ff_framequeue_init(&link->fifo, &src->graph->internal->frame_queues);

    return 0;
}

static int graph_check_validity(AVFilterGraph *graph, AVClass *log_ctx)
{
    AVFilterContext *filt;
    int i, j;

    for (i = 0; i < graph->nb_filters; i++) {
        const AVFilterPad *pad;
        filt = graph->filters[i];

        for (j = 0; j < filt->nb_inputs; j++) {
            if (!filt->inputs[j] || !filt->inputs[j]->src) {
                pad = &filt->input_pads[j];
                av_log(log_ctx, AV_LOG_ERROR,
                       "Input pad \"%s\" with type %s of the filter instance \"%s\" of %s not connected to any source\n",
                       pad->name, av_get_media_type_string(pad->type), filt->name, filt->filter->name);
                return AVERROR(EINVAL);
            }
        }

        for (j = 0; j < filt->nb_outputs; j++) {
            if (!filt->outputs[j] || !filt->outputs[j]->dst) {
                pad = &filt->output_pads[j];
                av_log(log_ctx, AV_LOG_ERROR,
                       "Output pad \"%s\" with type %s of the filter instance \"%s\" of %s not connected to any destination\n",
                       pad->name, av_get_media_type_string(pad->type), filt->name, filt->filter->name);
                return AVERROR(EINVAL);
            }
        }
    }

    return 0;
}

/**
 * Configure all the links of graphctx.
 *
 * @return >= 0 in case of success, a negative value otherwise
 */
static int graph_config_links(AVFilterGraph *graph, AVClass *log_ctx)
{
    AVFilterContext *filt;
    int i, ret;

    for (i = 0; i < graph->nb_filters; i++) {
        filt = graph->filters[i];

        if (!filt->nb_outputs) {
            if ((ret = avfilter_config_links(filt)))
                return ret;
        }
    }

    return 0;
}

static int graph_check_links(AVFilterGraph *graph, AVClass *log_ctx)
{
    AVFilterContext *f;
    AVFilterLink *l;
    unsigned i, j;
    int ret;

    for (i = 0; i < graph->nb_filters; i++) {
        f = graph->filters[i];
        for (j = 0; j < f->nb_outputs; j++) {
            l = f->outputs[j];
            if (l->type == AVMEDIA_TYPE_VIDEO) {
                ret = av_image_check_size2(l->w, l->h, INT64_MAX, l->format, 0, f);
                if (ret < 0)
                    return ret;
            }
        }
    }
    return 0;
}

static int graph_config_formats(AVFilterGraph *graph, AVClass *log_ctx)
{
    int ret;

    /* find supported formats from sub-filters, and merge along links */
    while ((ret = query_formats(graph, log_ctx)) == AVERROR(EAGAIN))
        av_log(graph, AV_LOG_DEBUG, "query_formats not finished\n");
    if (ret < 0)
        return ret;

    /* Once everything is merged, it's possible that we'll still have
     * multiple valid media format choices. We try to minimize the amount
     * of format conversion inside filters */
    if ((ret = reduce_formats(graph)) < 0)
        return ret;

    /* for audio filters, ensure the best format, sample rate and channel layout
     * is selected */
    swap_sample_fmts(graph);
    swap_samplerates(graph);
    swap_channel_layouts(graph);

    if ((ret = pick_formats(graph)) < 0)
        return ret;

    return 0;
}

void *av_calloc(size_t nmemb, size_t size)
{
    size_t result;
    if (av_size_mult(nmemb, size, &result) < 0)
        return NULL;
    return av_mallocz(result);
}

static int graph_config_pointers(AVFilterGraph *graph,
                                             AVClass *log_ctx)
{
    unsigned i, j;
    int sink_links_count = 0, n = 0;
    AVFilterContext *f;
    AVFilterLink **sinks;

    for (i = 0; i < graph->nb_filters; i++) {
        f = graph->filters[i];
        for (j = 0; j < f->nb_inputs; j++) {
            f->inputs[j]->graph     = graph;
            f->inputs[j]->age_index = -1;
        }
        for (j = 0; j < f->nb_outputs; j++) {
            f->outputs[j]->graph    = graph;
            f->outputs[j]->age_index= -1;
        }
        if (!f->nb_outputs) {
            if (f->nb_inputs > INT_MAX - sink_links_count)
                return AVERROR(EINVAL);
            sink_links_count += f->nb_inputs;
        }
    }
    sinks = av_calloc(sink_links_count, sizeof(*sinks));
    if (!sinks)
        return AVERROR(ENOMEM);
    for (i = 0; i < graph->nb_filters; i++) {
        f = graph->filters[i];
        if (!f->nb_outputs) {
            for (j = 0; j < f->nb_inputs; j++) {
                sinks[n] = f->inputs[j];
                f->inputs[j]->age_index = n++;
            }
        }
    }
    av_assert0(n == sink_links_count);
    graph->sink_links       = sinks;
    graph->sink_links_count = sink_links_count;
    return 0;
}

int avfilter_graph_config(AVFilterGraph *graphctx, void *log_ctx)
{
    int ret;

    if ((ret = graph_check_validity(graphctx, log_ctx)))
        return ret;
    if ((ret = graph_config_formats(graphctx, log_ctx)))
        return ret;
    if ((ret = graph_config_links(graphctx, log_ctx)))
        return ret;
    if ((ret = graph_check_links(graphctx, log_ctx)))
        return ret;
    if ((ret = graph_config_pointers(graphctx, log_ctx)))
        return ret;

    return 0;
}


static int configure_filtergraph(AVFilterGraph *graph, const char *filtergraph,
                                 AVFilterContext *source_ctx, AVFilterContext *sink_ctx)
{
    int ret, i;
    int nb_filters = graph->nb_filters;
    AVFilterInOut *outputs = NULL, *inputs = NULL;

    if (filtergraph)
    {
        outputs = avfilter_inout_alloc();
        inputs = avfilter_inout_alloc();
        if (!outputs || !inputs)
        {
            ret = AVERROR(ENOMEM);
            goto fail;
        }

        outputs->name = av_strdup("in");
        outputs->filter_ctx = source_ctx;
        outputs->pad_idx = 0;
        outputs->next = NULL;

        inputs->name = av_strdup("out");
        inputs->filter_ctx = sink_ctx;
        inputs->pad_idx = 0;
        inputs->next = NULL;

        if ((ret = avfilter_graph_parse_ptr(graph, filtergraph, &inputs, &outputs, NULL)) < 0)
            goto fail;
    }
    else
    {
        if ((ret = avfilter_link(source_ctx, 0, sink_ctx, 0)) < 0)
            goto fail;
    }

    /* Reorder the filters to ensure that inputs of the custom filters are merged first */
    for (i = 0; i < graph->nb_filters - nb_filters; i++)
        FFSWAP(AVFilterContext *, graph->filters[i], graph->filters[i + nb_filters]);

    ret = avfilter_graph_config(graph, NULL);
fail:
    avfilter_inout_free(&outputs);
    avfilter_inout_free(&inputs);
    return ret;
}

AVRational av_guess_frame_rate(AVFormatContext *format, AVStream *st, AVFrame *frame)
{
    AVRational fr = st->r_frame_rate;
    AVRational codec_fr = st->internal->avctx->framerate;
    AVRational avg_fr = st->avg_frame_rate;

    if (avg_fr.num > 0 && avg_fr.den > 0 && fr.num > 0 && fr.den > 0 &&
        av_q2d(avg_fr) < 70 && av_q2d(fr) > 210)
    {
        fr = avg_fr;
    }

    if (st->internal->avctx->ticks_per_frame > 1)
    {
        if (codec_fr.num > 0 && codec_fr.den > 0 &&
            (fr.num == 0 || av_q2d(codec_fr) < av_q2d(fr) * 0.7 && fabs(1.0 - av_q2d(av_div_q(avg_fr, fr))) > 0.1))
            fr = codec_fr;
    }

    return fr;
}



static inline av_const int av_toupper(int c)
{
    if (c >= 'a' && c <= 'z')
        c ^= 0x20;
    return c;
}

AVDictionaryEntry *av_dict_get(const AVDictionary *m, const char *key,
                               const AVDictionaryEntry *prev, int flags)
{
    unsigned int i, j;

    if (!m)
        return NULL;

    if (prev)
        i = prev - m->elems + 1;
    else
        i = 0;

    for (; i < m->count; i++)
    {
        const char *s = m->elems[i].key;
        if (flags & AV_DICT_MATCH_CASE)
            for (j = 0; s[j] == key[j] && key[j]; j++)
                ;
        else
            for (j = 0; av_toupper(s[j]) == av_toupper(key[j]) && key[j]; j++)
                ;
        if (key[j])
            continue;
        if (s[j] && !(flags & AV_DICT_IGNORE_SUFFIX))
            continue;
        return &m->elems[i];
    }
    return NULL;
}

double av_display_rotation_get(const int32_t matrix[9])
{
    double rotation, scale[2];

    scale[0] = hypot(CONV_FP(matrix[0]), CONV_FP(matrix[3]));
    scale[1] = hypot(CONV_FP(matrix[1]), CONV_FP(matrix[4]));

    if (scale[0] == 0.0 || scale[1] == 0.0)
        return NAN;

    rotation = atan2(CONV_FP(matrix[1]) / scale[1],
                     CONV_FP(matrix[0]) / scale[0]) *
               180 / M_PI;

    return -rotation;
}


uint8_t *av_stream_get_side_data(const AVStream *st, enum AVPacketSideDataType type, int *size)
{
    int i;

    for (i = 0; i < st->nb_side_data; i++)
    {
        if (st->side_data[i].type == type)
        {
            if (size)
                *size = st->side_data[i].size;
            return st->side_data[i].data;
        }
    }
    if (size)
        *size = 0;
    return NULL;
}


double get_rotation(AVStream *st)
{
    uint8_t *displaymatrix = av_stream_get_side_data(st,
                                                     AV_PKT_DATA_DISPLAYMATRIX, NULL);
    double theta = 0;
    if (displaymatrix)
        theta = -av_display_rotation_get((int32_t *)displaymatrix);

    theta -= 360 * floor(theta / 360 + 0.9 / 360);

    if (fabs(theta - 90 * round(theta / 90)) > 2)
        av_log(NULL, AV_LOG_WARNING, "Odd rotation angle.\n"
                                     "If you want to help, upload a sample "
                                     "of this file to https://streams.videolan.org/upload/ "
                                     "and contact the ffmpeg-devel mailing list. (ffmpeg-devel@ffmpeg.org)");

    return theta;
}

size_t av_strlcatf(char *dst, size_t size, const char *fmt, ...)
{
    size_t len = strlen(dst);
    va_list vl;

    va_start(vl, fmt);
    len += vsnprintf(dst + len, size > len ? size - len : 0, fmt, vl);
    va_end(vl);

    return len;
}

static const char *default_filter_name(void *filter_ctx)
{
    AVFilterContext *ctx = filter_ctx;
    return ctx->name ? ctx->name : ctx->filter->name;
}
static void *filter_child_next(void *obj, void *prev)
{
    AVFilterContext *ctx = obj;
    if (!prev && ctx->filter && ctx->filter->priv_class && ctx->priv)
        return ctx->priv;
    return NULL;
}
#define OFFSET(x) offsetof(BenchContext, x)
#if CONFIG_ABENCH_FILTER
DEFINE_OPTIONS(abench, AV_OPT_FLAG_FILTERING_PARAM|AV_OPT_FLAG_AUDIO_PARAM);
AVFILTER_DEFINE_CLASS(abench);
#define START_TIME_KEY "lavfi.bench.start_time"
#define T2F(v) ((v) / 1000000.)

static av_cold int init(AVFilterContext *ctx)
{
    BenchContext *s = ctx->priv;
    s->min = INT64_MAX;
    s->max = INT64_MIN;
    return 0;
}
static int filter_frame(AVFilterLink *inlink, AVFrame *in)
{
    AVFilterContext *ctx = inlink->dst;
    BenchContext *s = ctx->priv;
    AVFilterLink *outlink = ctx->outputs[0];
    const int64_t t = av_gettime();

    if (t < 0)
        return ff_filter_frame(outlink, in);

    if (s->action == ACTION_START) {
        av_dict_set_int(&in->metadata, START_TIME_KEY, t, 0);
    } else if (s->action == ACTION_STOP) {
        AVDictionaryEntry *e = av_dict_get(in->metadata, START_TIME_KEY, NULL, 0);
        if (e) {
            const int64_t start = strtoll(e->value, NULL, 0);
            const int64_t diff = t - start;
            s->sum += diff;
            s->n++;
            s->min = FFMIN(s->min, diff);
            s->max = FFMAX(s->max, diff);
            av_log(s, AV_LOG_INFO, "t:%f avg:%f max:%f min:%f\n",
                   T2F(diff), T2F(s->sum / s->n), T2F(s->max), T2F(s->min));
        }
        av_dict_set(&in->metadata, START_TIME_KEY, NULL, 0);
    }

    return ff_filter_frame(outlink, in);
}
static const AVFilterPad abench_inputs[] = {
    {
        .name         = "default",
        .type         = AVMEDIA_TYPE_AUDIO,
        .filter_frame = filter_frame,
    },
    { NULL }
};

static const AVFilterPad abench_outputs[] = {
    {
        .name = "default",
        .type = AVMEDIA_TYPE_AUDIO,
    },
    { NULL }
};
AVFilter ff_af_abench = {
    .name          = "abench",
    .description   = NULL_IF_CONFIG_SMALL("Benchmark part of a filtergraph."),
    .priv_size     = sizeof(BenchContext),
    .init          = init,
    .inputs        = abench_inputs,
    .outputs       = abench_outputs,
    .priv_class    = &abench_class,
};
#endif
static const AVFilter * const filter_list[] = {
    &ff_af_abench,
    &ff_af_acompressor,
    &ff_af_acontrast,
    &ff_af_acopy,
    &ff_af_acue,
    &ff_af_acrossfade,
    &ff_af_acrossover,
    &ff_af_acrusher,
    &ff_af_adeclick,
    &ff_af_adeclip,
    &ff_af_adelay,
    &ff_af_aderivative,
    &ff_af_aecho,
    &ff_af_aemphasis,
    &ff_af_aeval,
    &ff_af_afade,
    &ff_af_afftdn,
    &ff_af_afftfilt,
    &ff_af_afir,
    &ff_af_aformat,
    &ff_af_agate,
    &ff_af_aiir,
    &ff_af_aintegral,
    &ff_af_ainterleave,
    &ff_af_alimiter,
    &ff_af_allpass,
    &ff_af_aloop,
    &ff_af_amerge,
    &ff_af_ametadata,
    &ff_af_amix,
    &ff_af_amultiply,
    &ff_af_anequalizer,
    &ff_af_anlmdn,
    &ff_af_anlms,
    &ff_af_anull,
    &ff_af_apad,
    &ff_af_aperms,
    &ff_af_aphaser,
    &ff_af_apulsator,
    &ff_af_arealtime,
    &ff_af_aresample,
    &ff_af_areverse,
    &ff_af_arnndn,
    &ff_af_aselect,
    &ff_af_asendcmd,
    &ff_af_asetnsamples,
    &ff_af_asetpts,
    &ff_af_asetrate,
    &ff_af_asettb,
    &ff_af_ashowinfo,
    &ff_af_asidedata,
    &ff_af_asoftclip,
    &ff_af_asplit,
    &ff_af_astats,
    &ff_af_astreamselect,
    &ff_af_asubboost,
    &ff_af_atempo,
    &ff_af_atrim,
    &ff_af_axcorrelate,
    &ff_af_azmq,
    &ff_af_bandpass,
    &ff_af_bandreject,
    &ff_af_bass,
    &ff_af_biquad,
    &ff_af_bs2b,
    &ff_af_channelmap,
    &ff_af_channelsplit,
    &ff_af_chorus,
    &ff_af_compand,
    &ff_af_compensationdelay,
    &ff_af_crossfeed,
    &ff_af_crystalizer,
    &ff_af_dcshift,
    &ff_af_deesser,
    &ff_af_drmeter,
    &ff_af_dynaudnorm,
    &ff_af_earwax,
    &ff_af_ebur128,
    &ff_af_equalizer,
    &ff_af_extrastereo,
    &ff_af_firequalizer,
    &ff_af_flanger,
    &ff_af_haas,
    &ff_af_hdcd,
    &ff_af_headphone,
    &ff_af_highpass,
    &ff_af_highshelf,
    &ff_af_join,
    &ff_af_loudnorm,
    &ff_af_lowpass,
    &ff_af_lowshelf,
    &ff_af_mcompand,
    &ff_af_pan,
    &ff_af_replaygain,
    &ff_af_rubberband,
    &ff_af_sidechaincompress,
    &ff_af_sidechaingate,
    &ff_af_silencedetect,
    &ff_af_silenceremove,
    &ff_af_sofalizer,
    &ff_af_stereotools,
    &ff_af_stereowiden,
    &ff_af_superequalizer,
    &ff_af_surround,
    &ff_af_treble,
    &ff_af_tremolo,
    &ff_af_vibrato,
    &ff_af_volume,
    &ff_af_volumedetect,
    &ff_asrc_aevalsrc,
    &ff_asrc_afirsrc,
    &ff_asrc_anoisesrc,
    &ff_asrc_anullsrc,
    &ff_asrc_hilbert,
    &ff_asrc_sinc,
    &ff_asrc_sine,
    &ff_asink_anullsink,
    &ff_vf_addroi,
    &ff_vf_alphaextract,
    &ff_vf_alphamerge,
    &ff_vf_amplify,
    &ff_vf_ass,
    &ff_vf_atadenoise,
    &ff_vf_avgblur,
    &ff_vf_bbox,
    &ff_vf_bench,
    &ff_vf_bilateral,
    &ff_vf_bitplanenoise,
    &ff_vf_blackdetect,
    &ff_vf_blackframe,
    &ff_vf_blend,
    &ff_vf_bm3d,
    &ff_vf_boxblur,
    &ff_vf_bwdif,
    &ff_vf_cas,
    &ff_vf_chromahold,
    &ff_vf_chromakey,
    &ff_vf_chromanr,
    &ff_vf_chromashift,
    &ff_vf_ciescope,
    &ff_vf_codecview,
    &ff_vf_colorbalance,
    &ff_vf_colorchannelmixer,
    &ff_vf_colorkey,
    &ff_vf_colorhold,
    &ff_vf_colorlevels,
    &ff_vf_colormatrix,
    &ff_vf_colorspace,
    &ff_vf_convolution,
    &ff_vf_convolve,
    &ff_vf_copy,
    &ff_vf_cover_rect,
    &ff_vf_crop,
    &ff_vf_cropdetect,
    &ff_vf_cue,
    &ff_vf_curves,
    &ff_vf_datascope,
    &ff_vf_dblur,
    &ff_vf_dctdnoiz,
    &ff_vf_deband,
    &ff_vf_deblock,
    &ff_vf_decimate,
    &ff_vf_deconvolve,
    &ff_vf_dedot,
    &ff_vf_deflate,
    &ff_vf_deflicker,
    &ff_vf_deinterlace_qsv,
    &ff_vf_dejudder,
    &ff_vf_delogo,
    &ff_vf_derain,
    &ff_vf_deshake,
    &ff_vf_despill,
    &ff_vf_detelecine,
    &ff_vf_dilation,
    &ff_vf_displace,
    &ff_vf_dnn_processing,
    &ff_vf_doubleweave,
    &ff_vf_drawbox,
    &ff_vf_drawgraph,
    &ff_vf_drawgrid,
    &ff_vf_drawtext,
    &ff_vf_edgedetect,
    &ff_vf_elbg,
    &ff_vf_entropy,
    &ff_vf_eq,
    &ff_vf_erosion,
    &ff_vf_extractplanes,
    &ff_vf_fade,
    &ff_vf_fftdnoiz,
    &ff_vf_fftfilt,
    &ff_vf_field,
    &ff_vf_fieldhint,
    &ff_vf_fieldmatch,
    &ff_vf_fieldorder,
    &ff_vf_fillborders,
    &ff_vf_find_rect,
    &ff_vf_floodfill,
    &ff_vf_format,
    &ff_vf_fps,
    &ff_vf_framepack,
    &ff_vf_framerate,
    &ff_vf_framestep,
    &ff_vf_freezedetect,
    &ff_vf_freezeframes,
    &ff_vf_fspp,
    &ff_vf_gblur,
    &ff_vf_geq,
    &ff_vf_gradfun,
    &ff_vf_graphmonitor,
    &ff_vf_greyedge,
    &ff_vf_haldclut,
    &ff_vf_hflip,
    &ff_vf_histeq,
    &ff_vf_histogram,
    &ff_vf_hqdn3d,
    &ff_vf_hqx,
    &ff_vf_hstack,
    &ff_vf_hue,
    &ff_vf_hwdownload,
    &ff_vf_hwmap,
    &ff_vf_hwupload,
    &ff_vf_hysteresis,
    &ff_vf_idet,
    &ff_vf_il,
    &ff_vf_inflate,
    &ff_vf_interlace,
    &ff_vf_interleave,
    &ff_vf_kerndeint,
    &ff_vf_lagfun,
    &ff_vf_lenscorrection,
    &ff_vf_limiter,
    &ff_vf_loop,
    &ff_vf_lumakey,
    &ff_vf_lut,
    &ff_vf_lut1d,
    &ff_vf_lut2,
    &ff_vf_lut3d,
    &ff_vf_lutrgb,
    &ff_vf_lutyuv,
    &ff_vf_maskedclamp,
    &ff_vf_maskedmax,
    &ff_vf_maskedmerge,
    &ff_vf_maskedmin,
    &ff_vf_maskedthreshold,
    &ff_vf_maskfun,
    &ff_vf_mcdeint,
    &ff_vf_median,
    &ff_vf_mergeplanes,
    &ff_vf_mestimate,
    &ff_vf_metadata,
    &ff_vf_midequalizer,
    &ff_vf_minterpolate,
    &ff_vf_mix,
    &ff_vf_mpdecimate,
    &ff_vf_negate,
    &ff_vf_nlmeans,
    &ff_vf_nnedi,
    &ff_vf_noformat,
    &ff_vf_noise,
    &ff_vf_normalize,
    &ff_vf_null,
    &ff_vf_oscilloscope,
    &ff_vf_overlay,
    &ff_vf_overlay_qsv,
    &ff_vf_owdenoise,
    &ff_vf_pad,
    &ff_vf_palettegen,
    &ff_vf_paletteuse,
    &ff_vf_perms,
    &ff_vf_perspective,
    &ff_vf_phase,
    &ff_vf_photosensitivity,
    &ff_vf_pixdesctest,
    &ff_vf_pixscope,
    &ff_vf_pp,
    &ff_vf_pp7,
    &ff_vf_premultiply,
    &ff_vf_prewitt,
    &ff_vf_pseudocolor,
    &ff_vf_psnr,
    &ff_vf_pullup,
    &ff_vf_qp,
    &ff_vf_random,
    &ff_vf_readeia608,
    &ff_vf_readvitc,
    &ff_vf_realtime,
    &ff_vf_remap,
    &ff_vf_removegrain,
    &ff_vf_removelogo,
    &ff_vf_repeatfields,
    &ff_vf_reverse,
    &ff_vf_rgbashift,
    &ff_vf_roberts,
    &ff_vf_rotate,
    &ff_vf_sab,
    &ff_vf_scale,
    &ff_vf_scale_qsv,
    &ff_vf_scale2ref,
    &ff_vf_scdet,
    &ff_vf_scroll,
    &ff_vf_select,
    &ff_vf_selectivecolor,
    &ff_vf_sendcmd,
    &ff_vf_separatefields,
    &ff_vf_setdar,
    &ff_vf_setfield,
    &ff_vf_setparams,
    &ff_vf_setpts,
    &ff_vf_setrange,
    &ff_vf_setsar,
    &ff_vf_settb,
    &ff_vf_showinfo,
    &ff_vf_showpalette,
    &ff_vf_shuffleframes,
    &ff_vf_shuffleplanes,
    &ff_vf_sidedata,
    &ff_vf_signalstats,
    &ff_vf_signature,
    &ff_vf_smartblur,
    &ff_vf_sobel,
    &ff_vf_split,
    &ff_vf_spp,
    &ff_vf_sr,
    &ff_vf_ssim,
    &ff_vf_stereo3d,
    &ff_vf_streamselect,
    &ff_vf_subtitles,
    &ff_vf_super2xsai,
    &ff_vf_swaprect,
    &ff_vf_swapuv,
    &ff_vf_tblend,
    &ff_vf_telecine,
    &ff_vf_thistogram,
    &ff_vf_threshold,
    &ff_vf_thumbnail,
    &ff_vf_tile,
    &ff_vf_tinterlace,
    &ff_vf_tlut2,
    &ff_vf_tmedian,
    &ff_vf_tmix,
    &ff_vf_tonemap,
    &ff_vf_tpad,
    &ff_vf_transpose,
    &ff_vf_trim,
    &ff_vf_unpremultiply,
    &ff_vf_unsharp,
    &ff_vf_untile,
    &ff_vf_uspp,
    &ff_vf_v360,
    &ff_vf_vaguedenoiser,
    &ff_vf_vectorscope,
    &ff_vf_vflip,
    &ff_vf_vfrdet,
    &ff_vf_vibrance,
    &ff_vf_vidstabdetect,
    &ff_vf_vidstabtransform,
    &ff_vf_vignette,
    &ff_vf_vmafmotion,
    &ff_vf_vpp_qsv,
    &ff_vf_vstack,
    &ff_vf_w3fdif,
    &ff_vf_waveform,
    &ff_vf_weave,
    &ff_vf_xbr,
    &ff_vf_xfade,
    &ff_vf_xmedian,
    &ff_vf_xstack,
    &ff_vf_yadif,
    &ff_vf_yaepblur,
    &ff_vf_zmq,
    &ff_vf_zoompan,
    &ff_vf_zscale,
    &ff_vsrc_allrgb,
    &ff_vsrc_allyuv,
    &ff_vsrc_cellauto,
    &ff_vsrc_color,
    &ff_vsrc_gradients,
    &ff_vsrc_haldclutsrc,
    &ff_vsrc_life,
    &ff_vsrc_mandelbrot,
    &ff_vsrc_mptestsrc,
    &ff_vsrc_nullsrc,
    &ff_vsrc_pal75bars,
    &ff_vsrc_pal100bars,
    &ff_vsrc_rgbtestsrc,
    &ff_vsrc_sierpinski,
    &ff_vsrc_smptebars,
    &ff_vsrc_smptehdbars,
    &ff_vsrc_testsrc,
    &ff_vsrc_testsrc2,
    &ff_vsrc_yuvtestsrc,
    &ff_vsink_nullsink,
    &ff_avf_abitscope,
    &ff_avf_adrawgraph,
    &ff_avf_agraphmonitor,
    &ff_avf_ahistogram,
    &ff_avf_aphasemeter,
    &ff_avf_avectorscope,
    &ff_avf_concat,
    &ff_avf_showcqt,
    &ff_avf_showfreqs,
    &ff_avf_showspatial,
    &ff_avf_showspectrum,
    &ff_avf_showspectrumpic,
    &ff_avf_showvolume,
    &ff_avf_showwaves,
    &ff_avf_showwavespic,
    &ff_vaf_spectrumsynth,
    &ff_avsrc_amovie,
    &ff_avsrc_movie,
    &ff_af_afifo,
    &ff_vf_fifo,
    &ff_asrc_abuffer,
    &ff_vsrc_buffer,
    &ff_asink_abuffer,
    &ff_vsink_buffer,
    NULL };
const AVFilter *av_filter_iterate(void **opaque)
{
    uintptr_t i = (uintptr_t)*opaque;
    const AVFilter *f = filter_list[i];

    if (f)
        *opaque = (void*)(i + 1);

    return f;
}
static const AVClass *filter_child_class_iterate(void **iter)
{
    const AVFilter *f;

    while ((f = av_filter_iterate(iter)))
        if (f->priv_class)
            return f->priv_class;

    return NULL;
}
#define OFFSET(x) offsetof(AVFilterContext, x)
#define FLAGS AV_OPT_FLAG_FILTERING_PARAM
static const AVOption avfilter_options[] = {
    { "thread_type", "Allowed thread types", OFFSET(thread_type), AV_OPT_TYPE_FLAGS,
        { .i64 = AVFILTER_THREAD_SLICE }, 0, INT_MAX, FLAGS, "thread_type" },
        { "slice", NULL, 0, AV_OPT_TYPE_CONST, { .i64 = AVFILTER_THREAD_SLICE }, .flags = FLAGS, .unit = "thread_type" },
    { "enable", "set enable expression", OFFSET(enable_str), AV_OPT_TYPE_STRING, {.str=NULL}, .flags = FLAGS },
    { "threads", "Allowed number of threads", OFFSET(nb_threads), AV_OPT_TYPE_INT,
        { .i64 = 0 }, 0, INT_MAX, FLAGS },
    { "extra_hw_frames", "Number of extra hardware frames to allocate for the user",
        OFFSET(extra_hw_frames), AV_OPT_TYPE_INT, { .i64 = -1 }, -1, INT_MAX, FLAGS },
    { NULL },
};
static const AVClass avfilter_class = {
    .class_name = "AVFilter",
    .item_name  = default_filter_name,
    .version    = LIBAVUTIL_VERSION_INT,
    .category   = AV_CLASS_CATEGORY_FILTER,
    .child_next = filter_child_next,
#if FF_API_CHILD_CLASS_NEXT
    .child_class_next = filter_child_class_next,
#endif
    .child_class_iterate = filter_child_class_iterate,
    .option           = avfilter_options,
};

static int default_execute(AVFilterContext *ctx, avfilter_action_func *func, void *arg,
                           int *ret, int nb_jobs)
{
    int i;

    for (i = 0; i < nb_jobs; i++) {
        int r = func(ctx, arg, i, nb_jobs);
        if (ret)
            ret[i] = r;
    }
    return 0;
}
AVFilterContext *ff_filter_alloc(const AVFilter *filter, const char *inst_name)
{
    AVFilterContext *ret;
    int preinited = 0;

    if (!filter)
        return NULL;

    ret = av_mallocz(sizeof(AVFilterContext));
    if (!ret)
        return NULL;

    ret->av_class = &avfilter_class;
    ret->filter   = filter;
    ret->name     = inst_name ? av_strdup(inst_name) : NULL;
    if (filter->priv_size) {
        ret->priv     = av_mallocz(filter->priv_size);
        if (!ret->priv)
            goto err;
    }
    if (filter->preinit) {
        if (filter->preinit(ret) < 0)
            goto err;
        preinited = 1;
    }

    av_opt_set_defaults(ret);
    if (filter->priv_class) {
        *(const AVClass**)ret->priv = filter->priv_class;
        av_opt_set_defaults(ret->priv);
    }

    ret->internal = av_mallocz(sizeof(*ret->internal));
    if (!ret->internal)
        goto err;
    ret->internal->execute = default_execute;

    ret->nb_inputs = avfilter_pad_count(filter->inputs);
    if (ret->nb_inputs ) {
        ret->input_pads   = av_malloc_array(ret->nb_inputs, sizeof(AVFilterPad));
        if (!ret->input_pads)
            goto err;
        memcpy(ret->input_pads, filter->inputs, sizeof(AVFilterPad) * ret->nb_inputs);
        ret->inputs       = av_mallocz_array(ret->nb_inputs, sizeof(AVFilterLink*));
        if (!ret->inputs)
            goto err;
    }

    ret->nb_outputs = avfilter_pad_count(filter->outputs);
    if (ret->nb_outputs) {
        ret->output_pads  = av_malloc_array(ret->nb_outputs, sizeof(AVFilterPad));
        if (!ret->output_pads)
            goto err;
        memcpy(ret->output_pads, filter->outputs, sizeof(AVFilterPad) * ret->nb_outputs);
        ret->outputs      = av_mallocz_array(ret->nb_outputs, sizeof(AVFilterLink*));
        if (!ret->outputs)
            goto err;
    }

    return ret;

err:
    if (preinited)
        filter->uninit(ret);
    av_freep(&ret->inputs);
    av_freep(&ret->input_pads);
    ret->nb_inputs = 0;
    av_freep(&ret->outputs);
    av_freep(&ret->output_pads);
    ret->nb_outputs = 0;
    av_freep(&ret->priv);
    av_freep(&ret->internal);
    av_free(ret);
    return NULL;
}

AVFilterContext *avfilter_graph_alloc_filter(AVFilterGraph *graph,
                                             const AVFilter *filter,
                                             const char *name)
{
    AVFilterContext **filters, *s;

    if (graph->thread_type && !graph->internal->thread_execute) {
        if (graph->execute) {
            graph->internal->thread_execute = graph->execute;
        } else {
            int ret = ff_graph_thread_init(graph);
            if (ret < 0) {
                av_log(graph, AV_LOG_ERROR, "Error initializing threading: %s.\n", av_err2str(ret));
                return NULL;
            }
        }
    }

    s = ff_filter_alloc(filter, name);
    if (!s)
        return NULL;

    filters = av_realloc(graph->filters, sizeof(*filters) * (graph->nb_filters + 1));
    if (!filters) {
        avfilter_free(s);
        return NULL;
    }

    graph->filters = filters;
    graph->filters[graph->nb_filters++] = s;

    s->graph = graph;

    return s;
}


int avfilter_graph_create_filter(AVFilterContext **filt_ctx, const AVFilter *filt,
                                 const char *name, const char *args, void *opaque,
                                 AVFilterGraph *graph_ctx)
{
    int ret;

    *filt_ctx = avfilter_graph_alloc_filter(graph_ctx, filt, name);
    if (!*filt_ctx)
        return AVERROR(ENOMEM);

    ret = avfilter_init_str(*filt_ctx, args);
    if (ret < 0)
        goto fail;

    return 0;

fail:
    if (*filt_ctx)
        avfilter_free(*filt_ctx);
    *filt_ctx = NULL;
    return ret;
}






static const AVFilterPad bench_inputs[] = {
    {
        .name         = "default",
        .type         = AVMEDIA_TYPE_VIDEO,
        .filter_frame = filter_frame,
    },
    { NULL }
};

static const AVFilterPad bench_outputs[] = {
    {
        .name = "default",
        .type = AVMEDIA_TYPE_VIDEO,
    },
    { NULL }
};









const AVFilter *avfilter_get_by_name(const char *name)
{
    const AVFilter *f = NULL;
    void *opaque = 0;

    if (!name)
        return NULL;

    while ((f = av_filter_iterate(&opaque)))
        if (!strcmp(f->name, name))
            return f;

    return NULL;
}

static int configure_video_filters(AVFilterGraph *graph, VideoState *is, const char *vfilters, AVFrame *frame)
{
    enum AVPixelFormat pix_fmts[FF_ARRAY_ELEMS(sdl_texture_format_map)];
    char sws_flags_str[512] = "";
    char buffersrc_args[256];
    int ret;
    AVFilterContext *filt_src = NULL, *filt_out = NULL, *last_filter = NULL;
    AVCodecParameters *codecpar = is->video_st->codecpar;
    AVRational fr = av_guess_frame_rate(is->ic, is->video_st, NULL);
    AVDictionaryEntry *e = NULL;
    int nb_pix_fmts = 0;
    int i, j;

    for (i = 0; i < renderer_info.num_texture_formats; i++)
    {
        for (j = 0; j < FF_ARRAY_ELEMS(sdl_texture_format_map) - 1; j++)
        {
            if (renderer_info.texture_formats[i] == sdl_texture_format_map[j].texture_fmt)
            {
                pix_fmts[nb_pix_fmts++] = sdl_texture_format_map[j].format;
                break;
            }
        }
    }
    pix_fmts[nb_pix_fmts] = AV_PIX_FMT_NONE;

    while ((e = av_dict_get(sws_dict, "", e, AV_DICT_IGNORE_SUFFIX)))
    {
        if (!strcmp(e->key, "sws_flags"))
        {
            av_strlcatf(sws_flags_str, sizeof(sws_flags_str), "%s=%s:", "flags", e->value);
        }
        else
            av_strlcatf(sws_flags_str, sizeof(sws_flags_str), "%s=%s:", e->key, e->value);
    }
    if (strlen(sws_flags_str))
        sws_flags_str[strlen(sws_flags_str) - 1] = '\0';

    graph->scale_sws_opts = av_strdup(sws_flags_str);

    snprintf(buffersrc_args, sizeof(buffersrc_args),
             "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
             frame->width, frame->height, frame->format,
             is->video_st->time_base.num, is->video_st->time_base.den,
             codecpar->sample_aspect_ratio.num, FFMAX(codecpar->sample_aspect_ratio.den, 1));
    if (fr.num && fr.den)
        av_strlcatf(buffersrc_args, sizeof(buffersrc_args), ":frame_rate=%d/%d", fr.num, fr.den);

    if ((ret = avfilter_graph_create_filter(&filt_src,
                                            avfilter_get_by_name("buffer"),
                                            "ffplay_buffer", buffersrc_args, NULL,
                                            graph)) < 0)
        goto fail;

    ret = avfilter_graph_create_filter(&filt_out,
                                       avfilter_get_by_name("buffersink"),
                                       "ffplay_buffersink", NULL, NULL, graph);
    if (ret < 0)
        goto fail;

    if ((ret = av_opt_set_int_list(filt_out, "pix_fmts", pix_fmts, AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN)) < 0)
        goto fail;

    last_filter = filt_out;

/* Note: this macro adds a filter before the lastly added filter, so the
 * processing order of the filters is in reverse */
#define INSERT_FILT(name, arg)                                                \
    do                                                                        \
    {                                                                         \
        AVFilterContext *filt_ctx;                                            \
                                                                              \
        ret = avfilter_graph_create_filter(&filt_ctx,                         \
                                           avfilter_get_by_name(name),        \
                                           "ffplay_" name, arg, NULL, graph); \
        if (ret < 0)                                                          \
            goto fail;                                                        \
                                                                              \
        ret = avfilter_link(filt_ctx, 0, last_filter, 0);                     \
        if (ret < 0)                                                          \
            goto fail;                                                        \
                                                                              \
        last_filter = filt_ctx;                                               \
    } while (0)

    if (autorotate)
    {
        double theta = get_rotation(is->video_st);

        if (fabs(theta - 90) < 1.0)
        {
            INSERT_FILT("transpose", "clock");
        }
        else if (fabs(theta - 180) < 1.0)
        {
            INSERT_FILT("hflip", NULL);
            INSERT_FILT("vflip", NULL);
        }
        else if (fabs(theta - 270) < 1.0)
        {
            INSERT_FILT("transpose", "cclock");
        }
        else if (fabs(theta) > 1.0)
        {
            char rotate_buf[64];
            snprintf(rotate_buf, sizeof(rotate_buf), "%f*PI/180", theta);
            INSERT_FILT("rotate", rotate_buf);
        }
    }

    if ((ret = configure_filtergraph(graph, vfilters, filt_src, last_filter)) < 0)
        goto fail;

    is->in_video_filter = filt_src;
    is->out_video_filter = filt_out;

fail:
    return ret;
}



const AVClass *av_opt_child_class_iterate(const AVClass *parent, void **iter)
{
    if (parent->child_class_iterate)
        return parent->child_class_iterate(iter);
#if FF_API_CHILD_CLASS_NEXT
FF_DISABLE_DEPRECATION_WARNINGS
    if (parent->child_class_next) {
        *iter = parent->child_class_next(*iter);
        return *iter;
    }
FF_ENABLE_DEPRECATION_WARNINGS
#endif
    return NULL;
}


const AVOption *av_opt_find2(void *obj, const char *name, const char *unit,
                             int opt_flags, int search_flags, void **target_obj)
{
    const AVClass  *c;
    const AVOption *o = NULL;

    if(!obj)
        return NULL;

    c= *(AVClass**)obj;

    if (!c)
        return NULL;

    if (search_flags & AV_OPT_SEARCH_CHILDREN) {
        if (search_flags & AV_OPT_SEARCH_FAKE_OBJ) {
            void *iter = NULL;
            const AVClass *child;
            while (child = av_opt_child_class_iterate(c, &iter))
                if (o = av_opt_find2(&child, name, unit, opt_flags, search_flags, NULL))
                    return o;
        } else {
            void *child = NULL;
            while (child = av_opt_child_next(obj, child))
                if (o = av_opt_find2(child, name, unit, opt_flags, search_flags, target_obj))
                    return o;
        }
    }

    while (o = av_opt_next(obj, o)) {
        if (!strcmp(o->name, name) && (o->flags & opt_flags) == opt_flags &&
            ((!unit && o->type != AV_OPT_TYPE_CONST) ||
             (unit  && o->type == AV_OPT_TYPE_CONST && o->unit && !strcmp(o->unit, unit)))) {
            if (target_obj) {
                if (!(search_flags & AV_OPT_SEARCH_FAKE_OBJ))
                    *target_obj = obj;
                else
                    *target_obj = NULL;
            }
            return o;
        }
    }
    return NULL;
}

static int set_string_bool(void *obj, const AVOption *o, const char *val, int *dst)
{
    int n;

    if (!val)
        return 0;

    if (!strcmp(val, "auto")) {
        n = -1;
    } else if (av_match_name(val, "true,y,yes,enable,enabled,on")) {
        n = 1;
    } else if (av_match_name(val, "false,n,no,disable,disabled,off")) {
        n = 0;
    } else {
        char *end = NULL;
        n = strtol(val, &end, 10);
        if (val + strlen(val) != end)
            goto fail;
    }

    if (n < o->min || n > o->max)
        goto fail;

    *dst = n;
    return 0;

fail:
    av_log(obj, AV_LOG_ERROR, "Unable to parse option value \"%s\" as boolean\n", val);
    return AVERROR(EINVAL);
}

static int set_string(void *obj, const AVOption *o, const char *val, uint8_t **dst)
{
    av_freep(dst);
    *dst = av_strdup(val);
    return *dst ? 0 : AVERROR(ENOMEM);
}

static int set_string_binary(void *obj, const AVOption *o, const char *val, uint8_t **dst)
{
    int *lendst = (int *)(dst + 1);
    uint8_t *bin, *ptr;
    int len;

    av_freep(dst);
    *lendst = 0;

    if (!val || !(len = strlen(val)))
        return 0;

    if (len & 1)
        return AVERROR(EINVAL);
    len /= 2;

    ptr = bin = av_malloc(len);
    if (!ptr)
        return AVERROR(ENOMEM);
    while (*val) {
        int a = hexchar2int(*val++);
        int b = hexchar2int(*val++);
        if (a < 0 || b < 0) {
            av_free(bin);
            return AVERROR(EINVAL);
        }
        *ptr++ = (a << 4) | b;
    }
    *dst    = bin;
    *lendst = len;

    return 0;
}

int av_opt_set(void *obj, const char *name, const char *val, int search_flags)
{
    int ret = 0;
    void *dst, *target_obj;
    const AVOption *o = av_opt_find2(obj, name, NULL, 0, search_flags, &target_obj);
    if (!o || !target_obj)
        return AVERROR_OPTION_NOT_FOUND;
    if (!val && (o->type != AV_OPT_TYPE_STRING &&
                 o->type != AV_OPT_TYPE_PIXEL_FMT && o->type != AV_OPT_TYPE_SAMPLE_FMT &&
                 o->type != AV_OPT_TYPE_IMAGE_SIZE &&
                 o->type != AV_OPT_TYPE_DURATION && o->type != AV_OPT_TYPE_COLOR &&
                 o->type != AV_OPT_TYPE_CHANNEL_LAYOUT && o->type != AV_OPT_TYPE_BOOL))
        return AVERROR(EINVAL);

    if (o->flags & AV_OPT_FLAG_READONLY)
        return AVERROR(EINVAL);

    if (o->flags & AV_OPT_FLAG_DEPRECATED)
        av_log(obj, AV_LOG_WARNING, "The \"%s\" option is deprecated: %s\n", name, o->help);

    dst = ((uint8_t *)target_obj) + o->offset;
    switch (o->type) {
    case AV_OPT_TYPE_BOOL:
        return set_string_bool(obj, o, val, dst);
    case AV_OPT_TYPE_STRING:
        return set_string(obj, o, val, dst);
    case AV_OPT_TYPE_BINARY:
        return set_string_binary(obj, o, val, dst);
    case AV_OPT_TYPE_FLAGS:
    case AV_OPT_TYPE_INT:
    case AV_OPT_TYPE_INT64:
    case AV_OPT_TYPE_UINT64:
    case AV_OPT_TYPE_FLOAT:
    case AV_OPT_TYPE_DOUBLE:
    case AV_OPT_TYPE_RATIONAL:
        return set_string_number(obj, target_obj, o, val, dst);
    case AV_OPT_TYPE_IMAGE_SIZE:
        return set_string_image_size(obj, o, val, dst);
    case AV_OPT_TYPE_VIDEO_RATE: {
        AVRational tmp;
        ret = set_string_video_rate(obj, o, val, &tmp);
        if (ret < 0)
            return ret;
        return write_number(obj, o, dst, 1, tmp.den, tmp.num);
    }
    case AV_OPT_TYPE_PIXEL_FMT:
        return set_string_pixel_fmt(obj, o, val, dst);
    case AV_OPT_TYPE_SAMPLE_FMT:
        return set_string_sample_fmt(obj, o, val, dst);
    case AV_OPT_TYPE_DURATION:
        {
            int64_t usecs = 0;
            if (val) {
                if ((ret = av_parse_time(&usecs, val, 1)) < 0) {
                    av_log(obj, AV_LOG_ERROR, "Unable to parse option value \"%s\" as duration\n", val);
                    return ret;
                }
            }
            if (usecs < o->min || usecs > o->max) {
                av_log(obj, AV_LOG_ERROR, "Value %f for parameter '%s' out of range [%g - %g]\n",
                       usecs / 1000000.0, o->name, o->min / 1000000.0, o->max / 1000000.0);
                return AVERROR(ERANGE);
            }
            *(int64_t *)dst = usecs;
            return 0;
        }
    case AV_OPT_TYPE_COLOR:
        return set_string_color(obj, o, val, dst);
    case AV_OPT_TYPE_CHANNEL_LAYOUT:
        if (!val || !strcmp(val, "none")) {
            *(int64_t *)dst = 0;
        } else {
            int64_t cl = av_get_channel_layout(val);
            if (!cl) {
                av_log(obj, AV_LOG_ERROR, "Unable to parse option value \"%s\" as channel layout\n", val);
                ret = AVERROR(EINVAL);
            }
            *(int64_t *)dst = cl;
            return ret;
        }
        break;
    case AV_OPT_TYPE_DICT:
        return set_string_dict(obj, o, val, dst);
    }

    av_log(obj, AV_LOG_ERROR, "Invalid option type.\n");
    return AVERROR(EINVAL);
}

char *av_get_token(const char **buf, const char *term)
{
    char *out = av_malloc(strlen(*buf) + 1);
    char *ret = out, *end = out;
    const char *p = *buf;
    if (!out)
        return NULL;
    p += strspn(p, WHITESPACES);

    while (*p && !strspn(p, term))
    {
        char c = *p++;
        if (c == '\\' && *p)
        {
            *out++ = *p++;
            end = out;
        }
        else if (c == '\'')
        {
            while (*p && *p != '\'')
                *out++ = *p++;
            if (*p)
            {
                p++;
                end = out;
            }
        }
        else
        {
            *out++ = c;
        }
    }

    do
        *out-- = 0;
    while (out >= end && strspn(out, WHITESPACES));

    *buf = p;

    return ret;
}


int av_opt_get_key_value(const char **ropts,
                         const char *key_val_sep, const char *pairs_sep,
                         unsigned flags,
                         char **rkey, char **rval)
{
    int ret;
    char *key = NULL, *val;
    const char *opts = *ropts;

    if ((ret = get_key(&opts, key_val_sep, &key)) < 0 &&
        !(flags & AV_OPT_FLAG_IMPLICIT_KEY))
        return AVERROR(EINVAL);
    if (!(val = av_get_token(&opts, pairs_sep))) {
        av_free(key);
        return AVERROR(ENOMEM);
    }
    *ropts = opts;
    *rkey  = key;
    *rval  = val;
    return 0;
}

const AVOption *av_opt_find(void *obj, const char *name, const char *unit,
                            int opt_flags, int search_flags)
{
    return av_opt_find2(obj, name, unit, opt_flags, search_flags, NULL);
}

static int process_options(AVFilterContext *ctx, AVDictionary **options,
                           const char *args)
{
    const AVOption *o = NULL;
    int ret, count = 0;
    char *av_uninit(parsed_key), *av_uninit(value);
    const char *key;
    int offset= -1;

    if (!args)
        return 0;

    while (*args) {
        const char *shorthand = NULL;

        o = av_opt_next(ctx->priv, o);
        if (o) {
            if (o->type == AV_OPT_TYPE_CONST || o->offset == offset)
                continue;
            offset = o->offset;
            shorthand = o->name;
        }

        ret = av_opt_get_key_value(&args, "=", ":",
                                   shorthand ? AV_OPT_FLAG_IMPLICIT_KEY : 0,
                                   &parsed_key, &value);
        if (ret < 0) {
            if (ret == AVERROR(EINVAL))
                av_log(ctx, AV_LOG_ERROR, "No option name near '%s'\n", args);
            else
                av_log(ctx, AV_LOG_ERROR, "Unable to parse '%s': %s\n", args,
                       av_err2str(ret));
            return ret;
        }
        if (*args)
            args++;
        if (parsed_key) {
            key = parsed_key;
            while ((o = av_opt_next(ctx->priv, o))); /* discard all remaining shorthand */
        } else {
            key = shorthand;
        }

        av_log(ctx, AV_LOG_DEBUG, "Setting '%s' to value '%s'\n", key, value);

        if (av_opt_find(ctx, key, NULL, 0, 0)) {
            ret = av_opt_set(ctx, key, value, 0);
            if (ret < 0) {
                av_free(value);
                av_free(parsed_key);
                return ret;
            }
        } else {
            av_dict_set(options, key, value, 0);
            if ((ret = av_opt_set(ctx->priv, key, value, AV_OPT_SEARCH_CHILDREN)) < 0) {
                if (!av_opt_find(ctx->priv, key, NULL, 0, AV_OPT_SEARCH_CHILDREN | AV_OPT_SEARCH_FAKE_OBJ)) {
                    if (ret == AVERROR_OPTION_NOT_FOUND)
                        av_log(ctx, AV_LOG_ERROR, "Option '%s' not found\n", key);
                    av_free(value);
                    av_free(parsed_key);
                    return ret;
                }
            }
        }

        av_free(value);
        av_free(parsed_key);
        count++;
    }

    if (ctx->enable_str) {
        ret = set_enable_expr(ctx, ctx->enable_str);
        if (ret < 0)
            return ret;
    }
    return count;
}

int avfilter_init_str(AVFilterContext *filter, const char *args)
{
    AVDictionary *options = NULL;
    AVDictionaryEntry *e;
    int ret = 0;

    if (args && *args) {
        if (!filter->filter->priv_class) {
            av_log(filter, AV_LOG_ERROR, "This filter does not take any "
                   "options, but options were provided: %s.\n", args);
            return AVERROR(EINVAL);
        }

#if FF_API_OLD_FILTER_OPTS_ERROR
            if (   !strcmp(filter->filter->name, "format")     ||
                   !strcmp(filter->filter->name, "noformat")   ||
                   !strcmp(filter->filter->name, "frei0r")     ||
                   !strcmp(filter->filter->name, "frei0r_src") ||
                   !strcmp(filter->filter->name, "ocv")        ||
                   !strcmp(filter->filter->name, "pan")        ||
                   !strcmp(filter->filter->name, "pp")         ||
                   !strcmp(filter->filter->name, "aevalsrc")) {
            /* a hack for compatibility with the old syntax
             * replace colons with |s */
            char *copy = av_strdup(args);
            char *p    = copy;
            int nb_leading = 0; // number of leading colons to skip
            int deprecated = 0;

            if (!copy) {
                ret = AVERROR(ENOMEM);
                goto fail;
            }

            if (!strcmp(filter->filter->name, "frei0r") ||
                !strcmp(filter->filter->name, "ocv"))
                nb_leading = 1;
            else if (!strcmp(filter->filter->name, "frei0r_src"))
                nb_leading = 3;

            while (nb_leading--) {
                p = strchr(p, ':');
                if (!p) {
                    p = copy + strlen(copy);
                    break;
                }
                p++;
            }

            deprecated = strchr(p, ':') != NULL;

            if (!strcmp(filter->filter->name, "aevalsrc")) {
                deprecated = 0;
                while ((p = strchr(p, ':')) && p[1] != ':') {
                    const char *epos = strchr(p + 1, '=');
                    const char *spos = strchr(p + 1, ':');
                    const int next_token_is_opt = epos && (!spos || epos < spos);
                    if (next_token_is_opt) {
                        p++;
                        break;
                    }
                    /* next token does not contain a '=', assume a channel expression */
                    deprecated = 1;
                    *p++ = '|';
                }
                if (p && *p == ':') { // double sep '::' found
                    deprecated = 1;
                    memmove(p, p + 1, strlen(p));
                }
            } else
            while ((p = strchr(p, ':')))
                *p++ = '|';

            if (deprecated) {
                av_log(filter, AV_LOG_ERROR, "This syntax is deprecated. Use "
                       "'|' to separate the list items ('%s' instead of '%s')\n",
                       copy, args);
                ret = AVERROR(EINVAL);
            } else {
                ret = process_options(filter, &options, copy);
            }
            av_freep(&copy);

            if (ret < 0)
                goto fail;
        } else
#endif
        {
            ret = process_options(filter, &options, args);
            if (ret < 0)
                goto fail;
        }
    }

    ret = avfilter_init_dict(filter, &options);
    if (ret < 0)
        goto fail;

    if ((e = av_dict_get(options, "", NULL, AV_DICT_IGNORE_SUFFIX))) {
        av_log(filter, AV_LOG_ERROR, "No such option: %s.\n", e->key);
        ret = AVERROR_OPTION_NOT_FOUND;
        goto fail;
    }

fail:
    av_dict_free(&options);

    return ret;
}

void avpriv_slicethread_execute(AVSliceThread *ctx, int nb_jobs, int execute_main)
{
    int nb_workers, i, is_last = 0;

    av_assert0(nb_jobs > 0);
    ctx->nb_jobs           = nb_jobs;
    ctx->nb_active_threads = FFMIN(nb_jobs, ctx->nb_threads);
    atomic_store_explicit(&ctx->first_job, 0, memory_order_relaxed);
    atomic_store_explicit(&ctx->current_job, ctx->nb_active_threads, memory_order_relaxed);
    nb_workers             = ctx->nb_active_threads;
    if (!ctx->main_func || !execute_main)
        nb_workers--;

    for (i = 0; i < nb_workers; i++) {
        WorkerContext *w = &ctx->workers[i];
        pthread_mutex_lock(&w->mutex);
        w->done = 0;
        pthread_cond_signal(&w->cond);
        pthread_mutex_unlock(&w->mutex);
    }

    if (ctx->main_func && execute_main)
        ctx->main_func(ctx->priv);
    else
        is_last = run_jobs(ctx);

    if (!is_last) {
        pthread_mutex_lock(&ctx->done_mutex);
        while (!ctx->done)
            pthread_cond_wait(&ctx->done_cond, &ctx->done_mutex);
        ctx->done = 0;
        pthread_mutex_unlock(&ctx->done_mutex);
    }
}

void avpriv_slicethread_free(AVSliceThread **pctx)
{
    AVSliceThread *ctx;
    int nb_workers, i;

    if (!pctx || !*pctx)
        return;

    ctx = *pctx;
    nb_workers = ctx->nb_threads;
    if (!ctx->main_func)
        nb_workers--;

    ctx->finished = 1;
    for (i = 0; i < nb_workers; i++) {
        WorkerContext *w = &ctx->workers[i];
        pthread_mutex_lock(&w->mutex);
        w->done = 0;
        pthread_cond_signal(&w->cond);
        pthread_mutex_unlock(&w->mutex);
    }

    for (i = 0; i < nb_workers; i++) {
        WorkerContext *w = &ctx->workers[i];
        pthread_join(w->thread, NULL);
        pthread_cond_destroy(&w->cond);
        pthread_mutex_destroy(&w->mutex);
    }

    pthread_cond_destroy(&ctx->done_cond);
    pthread_mutex_destroy(&ctx->done_mutex);
    av_freep(&ctx->workers);
    av_freep(pctx);
}

static void worker_func(void *priv, int jobnr, int threadnr, int nb_jobs, int nb_threads)
{
    ThreadContext *c = priv;
    int ret = c->func(c->ctx, c->arg, jobnr, nb_jobs);
    if (c->rets)
        c->rets[jobnr] = ret;
}
static int thread_init_internal(ThreadContext *c, int nb_threads)
{
    nb_threads = avpriv_slicethread_create(&c->thread, c, worker_func, NULL, nb_threads);
    if (nb_threads <= 1)
        avpriv_slicethread_free(&c->thread);
    return FFMAX(nb_threads, 1);
}


static int thread_execute(AVFilterContext *ctx, avfilter_action_func *func,
                          void *arg, int *ret, int nb_jobs)
{
    ThreadContext *c = ctx->graph->internal->thread;

    if (nb_jobs <= 0)
        return 0;
    c->ctx         = ctx;
    c->arg         = arg;
    c->func        = func;
    c->rets        = ret;

    avpriv_slicethread_execute(c->thread, nb_jobs, 0);
    return 0;
}

int ff_graph_thread_init(AVFilterGraph *graph)
{
    int ret;

    if (graph->nb_threads == 1) {
        graph->thread_type = 0;
        return 0;
    }

    graph->internal->thread = av_mallocz(sizeof(ThreadContext));
    if (!graph->internal->thread)
        return AVERROR(ENOMEM);

    ret = thread_init_internal(graph->internal->thread, graph->nb_threads);
    if (ret <= 1) {
        av_freep(&graph->internal->thread);
        graph->thread_type = 0;
        graph->nb_threads  = 1;
        return (ret < 0) ? ret : 0;
    }
    graph->nb_threads = ret;

    graph->internal->thread_execute = thread_execute;

    return 0;
}

int avfilter_pad_count(const AVFilterPad *pads)
{
    int count;

    if (!pads)
        return 0;

    for (count = 0; pads->name; count++)
        pads++;
    return count;
}


#if FF_API_CHILD_CLASS_NEXT
static const AVClass *filter_child_class_next(const AVClass *prev)
{
    void *opaque = NULL;
    const AVFilter *f = NULL;

    /* find the filter that corresponds to prev */
    while (prev && (f = av_filter_iterate(&opaque)))
        if (f->priv_class == prev)
            break;

    /* could not find filter corresponding to prev */
    if (prev && !f)
        return NULL;

    /* find next filter with specific options */
    while ((f = av_filter_iterate(&opaque)))
        if (f->priv_class)
            return f->priv_class;

    return NULL;
}
#endif










void ff_filter_graph_remove_filter(AVFilterGraph *graph, AVFilterContext *filter)
{
    int i, j;
    for (i = 0; i < graph->nb_filters; i++) {
        if (graph->filters[i] == filter) {
            FFSWAP(AVFilterContext*, graph->filters[i],
                   graph->filters[graph->nb_filters - 1]);
            graph->nb_filters--;
            filter->graph = NULL;
            for (j = 0; j<filter->nb_outputs; j++)
                if (filter->outputs[j])
                    filter->outputs[j]->graph = NULL;

            return;
        }
    }
}




void ff_formats_unref(AVFilterFormats **ref)
{
    FORMATS_UNREF(ref, formats);
}

static void free_link(AVFilterLink *link)
{
    if (!link)
        return;

    if (link->src)
        link->src->outputs[link->srcpad - link->src->output_pads] = NULL;
    if (link->dst)
        link->dst->inputs[link->dstpad - link->dst->input_pads] = NULL;

    av_buffer_unref(&link->hw_frames_ctx);

    ff_formats_unref(&link->incfg.formats);
    ff_formats_unref(&link->outcfg.formats);
    ff_formats_unref(&link->incfg.samplerates);
    ff_formats_unref(&link->outcfg.samplerates);
    ff_channel_layouts_unref(&link->incfg.channel_layouts);
    ff_channel_layouts_unref(&link->outcfg.channel_layouts);
    avfilter_link_free(&link);
}

void avfilter_free(AVFilterContext *filter)
{
    int i;

    if (!filter)
        return;

    if (filter->graph)
        ff_filter_graph_remove_filter(filter->graph, filter);

    if (filter->filter->uninit)
        filter->filter->uninit(filter);

    for (i = 0; i < filter->nb_inputs; i++) {
        free_link(filter->inputs[i]);
    }
    for (i = 0; i < filter->nb_outputs; i++) {
        free_link(filter->outputs[i]);
    }

    if (filter->filter->priv_class)
        av_opt_free(filter->priv);

    av_buffer_unref(&filter->hw_device_ctx);

    av_freep(&filter->name);
    av_freep(&filter->input_pads);
    av_freep(&filter->output_pads);
    av_freep(&filter->inputs);
    av_freep(&filter->outputs);
    av_freep(&filter->priv);
    while(filter->command_queue){
        ff_command_queue_pop(filter);
    }
    av_opt_free(filter);
    av_expr_free(filter->enable);
    filter->enable = NULL;
    av_freep(&filter->var_values);
    av_freep(&filter->internal);
    av_free(filter);
}

static void slice_thread_uninit(ThreadContext *c)
{
    avpriv_slicethread_free(&c->thread);
}

void ff_graph_thread_free(AVFilterGraph *graph)
{
    if (graph->internal->thread)
        slice_thread_uninit(graph->internal->thread);
    av_freep(&graph->internal->thread);
}

void avfilter_graph_free(AVFilterGraph **graph)
{
    if (!*graph)
        return;

    while ((*graph)->nb_filters)
        avfilter_free((*graph)->filters[0]);

    ff_graph_thread_free(*graph);

    av_freep(&(*graph)->sink_links);

    av_freep(&(*graph)->scale_sws_opts);
    av_freep(&(*graph)->aresample_swr_opts);
#if FF_API_LAVR_OPTS
    av_freep(&(*graph)->resample_lavr_opts);
#endif
    av_freep(&(*graph)->filters);
    av_freep(&(*graph)->internal);
    av_freep(graph);
}

void ff_framequeue_global_init(FFFrameQueueGlobal *fqg)
{
}

#define OFFSET(x) offsetof(AVFilterGraph, x)
#define F AV_OPT_FLAG_FILTERING_PARAM
#define V AV_OPT_FLAG_VIDEO_PARAM
#define A AV_OPT_FLAG_AUDIO_PARAM
static const AVOption filtergraph_options[] = {
    { "thread_type", "Allowed thread types", OFFSET(thread_type), AV_OPT_TYPE_FLAGS,
        { .i64 = AVFILTER_THREAD_SLICE }, 0, INT_MAX, F|V|A, "thread_type" },
        { "slice", NULL, 0, AV_OPT_TYPE_CONST, { .i64 = AVFILTER_THREAD_SLICE }, .flags = F|V|A, .unit = "thread_type" },
    { "threads",     "Maximum number of threads", OFFSET(nb_threads),
        AV_OPT_TYPE_INT,   { .i64 = 0 }, 0, INT_MAX, F|V|A },
    {"scale_sws_opts"       , "default scale filter options"        , OFFSET(scale_sws_opts)        ,
        AV_OPT_TYPE_STRING, {.str = NULL}, 0, 0, F|V },
    {"aresample_swr_opts"   , "default aresample filter options"    , OFFSET(aresample_swr_opts)    ,
        AV_OPT_TYPE_STRING, {.str = NULL}, 0, 0, F|A },
    { NULL },
};

static const AVClass filtergraph_class = {
    .class_name = "AVFilterGraph",
    .item_name  = av_default_item_name,
    .version    = LIBAVUTIL_VERSION_INT,
    .option     = filtergraph_options,
    .category   = AV_CLASS_CATEGORY_FILTER,
};
AVFilterGraph *avfilter_graph_alloc(void)
{
    AVFilterGraph *ret = av_mallocz(sizeof(*ret));
    if (!ret)
        return NULL;

    ret->internal = av_mallocz(sizeof(*ret->internal));
    if (!ret->internal) {
        av_freep(&ret);
        return NULL;
    }

    ret->av_class = &filtergraph_class;
    av_opt_set_defaults(ret);
    ff_framequeue_global_init(&ret->internal->frame_queues);

    return ret;
}

const char *av_get_sample_fmt_name(enum AVSampleFormat sample_fmt)
{
    if (sample_fmt < 0 || sample_fmt >= AV_SAMPLE_FMT_NB)
        return NULL;
    return sample_fmt_info[sample_fmt].name;
}

static int configure_audio_filters(VideoState *is, const char *afilters, int force_output_format)
{
    static const enum AVSampleFormat sample_fmts[] = {AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_NONE};
    int sample_rates[2] = {0, -1};
    int64_t channel_layouts[2] = {0, -1};
    int channels[2] = {0, -1};
    AVFilterContext *filt_asrc = NULL, *filt_asink = NULL;
    char aresample_swr_opts[512] = "";
    AVDictionaryEntry *e = NULL;
    char asrc_args[256];
    int ret;

    avfilter_graph_free(&is->agraph);
    if (!(is->agraph = avfilter_graph_alloc()))
        return AVERROR(ENOMEM);
    is->agraph->nb_threads = filter_nbthreads;

    while ((e = av_dict_get(swr_opts, "", e, AV_DICT_IGNORE_SUFFIX)))
        av_strlcatf(aresample_swr_opts, sizeof(aresample_swr_opts), "%s=%s:", e->key, e->value);
    if (strlen(aresample_swr_opts))
        aresample_swr_opts[strlen(aresample_swr_opts) - 1] = '\0';
    av_opt_set(is->agraph, "aresample_swr_opts", aresample_swr_opts, 0);

    ret = snprintf(asrc_args, sizeof(asrc_args),
                   "sample_rate=%d:sample_fmt=%s:channels=%d:time_base=%d/%d",
                   is->audio_filter_src.freq, av_get_sample_fmt_name(is->audio_filter_src.fmt),
                   is->audio_filter_src.channels,
                   1, is->audio_filter_src.freq);
    if (is->audio_filter_src.channel_layout)
        snprintf(asrc_args + ret, sizeof(asrc_args) - ret,
                 ":channel_layout=0x%" PRIx64, is->audio_filter_src.channel_layout);

    ret = avfilter_graph_create_filter(&filt_asrc,
                                       avfilter_get_by_name("abuffer"), "ffplay_abuffer",
                                       asrc_args, NULL, is->agraph);
    if (ret < 0)
        goto end;

    ret = avfilter_graph_create_filter(&filt_asink,
                                       avfilter_get_by_name("abuffersink"), "ffplay_abuffersink",
                                       NULL, NULL, is->agraph);
    if (ret < 0)
        goto end;

    if ((ret = av_opt_set_int_list(filt_asink, "sample_fmts", sample_fmts, AV_SAMPLE_FMT_NONE, AV_OPT_SEARCH_CHILDREN)) < 0)
        goto end;
    if ((ret = av_opt_set_int(filt_asink, "all_channel_counts", 1, AV_OPT_SEARCH_CHILDREN)) < 0)
        goto end;

    if (force_output_format)
    {
        channel_layouts[0] = is->audio_tgt.channel_layout;
        channels[0] = is->audio_tgt.channel_layout ? -1 : is->audio_tgt.channels;
        sample_rates[0] = is->audio_tgt.freq;
        if ((ret = av_opt_set_int(filt_asink, "all_channel_counts", 0, AV_OPT_SEARCH_CHILDREN)) < 0)
            goto end;
        if ((ret = av_opt_set_int_list(filt_asink, "channel_layouts", channel_layouts, -1, AV_OPT_SEARCH_CHILDREN)) < 0)
            goto end;
        if ((ret = av_opt_set_int_list(filt_asink, "channel_counts", channels, -1, AV_OPT_SEARCH_CHILDREN)) < 0)
            goto end;
        if ((ret = av_opt_set_int_list(filt_asink, "sample_rates", sample_rates, -1, AV_OPT_SEARCH_CHILDREN)) < 0)
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

void av_bprint_init_for_buffer(AVBPrint *buf, char *buffer, unsigned size)
{
    buf->str      = buffer;
    buf->len      = 0;
    buf->size     = size;
    buf->size_max = size;
    *buf->str = 0;
}

static const struct {
    const char *name;
    int         nb_channels;
    uint64_t     layout;
} channel_layout_map[] = {
    { "mono",        1,  AV_CH_LAYOUT_MONO },
    { "stereo",      2,  AV_CH_LAYOUT_STEREO },
    { "2.1",         3,  AV_CH_LAYOUT_2POINT1 },
    { "3.0",         3,  AV_CH_LAYOUT_SURROUND },
    { "3.0(back)",   3,  AV_CH_LAYOUT_2_1 },
    { "4.0",         4,  AV_CH_LAYOUT_4POINT0 },
    { "quad",        4,  AV_CH_LAYOUT_QUAD },
    { "quad(side)",  4,  AV_CH_LAYOUT_2_2 },
    { "3.1",         4,  AV_CH_LAYOUT_3POINT1 },
    { "5.0",         5,  AV_CH_LAYOUT_5POINT0_BACK },
    { "5.0(side)",   5,  AV_CH_LAYOUT_5POINT0 },
    { "4.1",         5,  AV_CH_LAYOUT_4POINT1 },
    { "5.1",         6,  AV_CH_LAYOUT_5POINT1_BACK },
    { "5.1(side)",   6,  AV_CH_LAYOUT_5POINT1 },
    { "6.0",         6,  AV_CH_LAYOUT_6POINT0 },
    { "6.0(front)",  6,  AV_CH_LAYOUT_6POINT0_FRONT },
    { "hexagonal",   6,  AV_CH_LAYOUT_HEXAGONAL },
    { "6.1",         7,  AV_CH_LAYOUT_6POINT1 },
    { "6.1(back)",   7,  AV_CH_LAYOUT_6POINT1_BACK },
    { "6.1(front)",  7,  AV_CH_LAYOUT_6POINT1_FRONT },
    { "7.0",         7,  AV_CH_LAYOUT_7POINT0 },
    { "7.0(front)",  7,  AV_CH_LAYOUT_7POINT0_FRONT },
    { "7.1",         8,  AV_CH_LAYOUT_7POINT1 },
    { "7.1(wide)",   8,  AV_CH_LAYOUT_7POINT1_WIDE_BACK },
    { "7.1(wide-side)",   8,  AV_CH_LAYOUT_7POINT1_WIDE },
    { "octagonal",   8,  AV_CH_LAYOUT_OCTAGONAL },
    { "hexadecagonal", 16, AV_CH_LAYOUT_HEXADECAGONAL },
    { "downmix",     2,  AV_CH_LAYOUT_STEREO_DOWNMIX, },
    { "22.2",          24, AV_CH_LAYOUT_22POINT2, },
};
void av_bprint_channel_layout(struct AVBPrint *bp,
                              int nb_channels, uint64_t channel_layout)
{
    int i;

    if (nb_channels <= 0)
        nb_channels = av_get_channel_layout_nb_channels(channel_layout);

    for (i = 0; i < FF_ARRAY_ELEMS(channel_layout_map); i++)
        if (nb_channels    == channel_layout_map[i].nb_channels &&
            channel_layout == channel_layout_map[i].layout) {
            av_bprintf(bp, "%s", channel_layout_map[i].name);
            return;
        }

    av_bprintf(bp, "%d channels", nb_channels);
    if (channel_layout) {
        int i, ch;
        av_bprintf(bp, " (");
        for (i = 0, ch = 0; i < 64; i++) {
            if ((channel_layout & (UINT64_C(1) << i))) {
                const char *name = get_channel_name(i);
                if (name) {
                    if (ch > 0)
                        av_bprintf(bp, "+");
                    av_bprintf(bp, "%s", name);
                }
                ch++;
            }
        }
        av_bprintf(bp, ")");
    }
}

void av_get_channel_layout_string(char *buf, int buf_size,
                                  int nb_channels, uint64_t channel_layout)
{
    AVBPrint bp;

    av_bprint_init_for_buffer(&bp, buf, buf_size);
    av_bprint_channel_layout(&bp, nb_channels, channel_layout);
}

static int av_buffersrc_add_frame_internal(AVFilterContext *ctx,
                                           AVFrame *frame, int flags)
{
    BufferSourceContext *s = ctx->priv;
    AVFrame *copy;
    int refcounted, ret;

    s->nb_failed_requests = 0;

    if (!frame)
        return av_buffersrc_close(ctx, AV_NOPTS_VALUE, flags);
    if (s->eof)
        return AVERROR(EINVAL);

    refcounted = !!frame->buf[0];

    if (!(flags & AV_BUFFERSRC_FLAG_NO_CHECK_FORMAT)) {

        switch (ctx->outputs[0]->type) {
        case AVMEDIA_TYPE_VIDEO:
            CHECK_VIDEO_PARAM_CHANGE(ctx, s, frame->width, frame->height,
                                     frame->format, frame->pts);
            break;
        case AVMEDIA_TYPE_AUDIO:
            /* For layouts unknown on input but known on link after negotiation. */
            if (!frame->channel_layout)
                frame->channel_layout = s->channel_layout;
            CHECK_AUDIO_PARAM_CHANGE(ctx, s, frame->sample_rate, frame->channel_layout,
                                     frame->channels, frame->format, frame->pts);
            break;
        default:
            return AVERROR(EINVAL);
        }

    }

    if (!(copy = av_frame_alloc()))
        return AVERROR(ENOMEM);

    if (refcounted) {
        av_frame_move_ref(copy, frame);
    } else {
        ret = av_frame_ref(copy, frame);
        if (ret < 0) {
            av_frame_free(&copy);
            return ret;
        }
    }

    ret = ff_filter_frame(ctx->outputs[0], copy);
    if (ret < 0)
        return ret;

    if ((flags & AV_BUFFERSRC_FLAG_PUSH)) {
        ret = push_frame(ctx->graph);
        if (ret < 0)
            return ret;
    }

    return 0;
}

int attribute_align_arg av_buffersrc_add_frame_flags(AVFilterContext *ctx, AVFrame *frame, int flags)
{
    AVFrame *copy = NULL;
    int ret = 0;

    if (frame && frame->channel_layout &&
        av_get_channel_layout_nb_channels(frame->channel_layout) != frame->channels) {
        av_log(ctx, AV_LOG_ERROR, "Layout indicates a different number of channels than actually present\n");
        return AVERROR(EINVAL);
    }

    if (!(flags & AV_BUFFERSRC_FLAG_KEEP_REF) || !frame)
        return av_buffersrc_add_frame_internal(ctx, frame, flags);

    if (!(copy = av_frame_alloc()))
        return AVERROR(ENOMEM);
    ret = av_frame_ref(copy, frame);
    if (ret >= 0)
        ret = av_buffersrc_add_frame_internal(ctx, copy, flags);

    av_frame_free(&copy);
    return ret;
}

int attribute_align_arg av_buffersrc_add_frame(AVFilterContext *ctx, AVFrame *frame)
{
    return av_buffersrc_add_frame_flags(ctx, frame, 0);
}

static int return_or_keep_frame(BufferSinkContext *buf, AVFrame *out, AVFrame *in, int flags)
{
    if ((flags & AV_BUFFERSINK_FLAG_PEEK)) {
        buf->peeked_frame = in;
        return out ? av_frame_ref(out, in) : 0;
    } else {
        av_assert1(out);
        buf->peeked_frame = NULL;
        av_frame_move_ref(out, in);
        av_frame_free(&in);
        return 0;
    }
}

static inline uint64_t ff_framequeue_queued_samples(const FFFrameQueue *fq)
{
    return fq->total_samples_head - fq->total_samples_tail;
}


int ff_inlink_check_available_samples(AVFilterLink *link, unsigned min)
{
    uint64_t samples = ff_framequeue_queued_samples(&link->fifo);
    av_assert1(min);
    return samples >= min || (link->status_in && samples);
}

static inline size_t ff_framequeue_queued_frames(const FFFrameQueue *fq)
{
    return fq->queued;
}

static int samples_ready(AVFilterLink *link, unsigned min)
{
    return ff_framequeue_queued_frames(&link->fifo) &&
           (ff_framequeue_queued_samples(&link->fifo) >= min ||
            link->status_in);
}

static inline FFFrameBucket *bucket(FFFrameQueue *fq, size_t idx)
{
    return &fq->queue[(fq->tail + idx) & (fq->allocated - 1)];
}

static void check_consistency(FFFrameQueue *fq)
{
#if defined(ASSERT_LEVEL) && ASSERT_LEVEL >= 2
    uint64_t nb_samples = 0;
    size_t i;

    av_assert0(fq->queued == fq->total_frames_head - fq->total_frames_tail);
    for (i = 0; i < fq->queued; i++)
        nb_samples += bucket(fq, i)->frame->nb_samples;
    av_assert0(nb_samples == fq->total_samples_head - fq->total_samples_tail);
#endif
}

AVFrame *ff_framequeue_peek(FFFrameQueue *fq, size_t idx)
{
    FFFrameBucket *b;

    check_consistency(fq);
    av_assert1(idx < fq->queued);
    b = bucket(fq, idx);
    check_consistency(fq);
    return b->frame;
}

AVFrame *ff_framequeue_take(FFFrameQueue *fq)
{
    FFFrameBucket *b;

    check_consistency(fq);
    av_assert1(fq->queued);
    b = bucket(fq, 0);
    fq->queued--;
    fq->tail++;
    fq->tail &= fq->allocated - 1;
    fq->total_frames_tail++;
    fq->total_samples_tail += b->frame->nb_samples;
    fq->samples_skipped = 0;
    check_consistency(fq);
    return b->frame;
}

int av_sample_fmt_is_planar(enum AVSampleFormat sample_fmt)
{
     if (sample_fmt < 0 || sample_fmt >= AV_SAMPLE_FMT_NB)
         return 0;
     return sample_fmt_info[sample_fmt].planar;
}

int av_get_bytes_per_sample(enum AVSampleFormat sample_fmt)
{
     return sample_fmt < 0 || sample_fmt >= AV_SAMPLE_FMT_NB ?
        0 : sample_fmt_info[sample_fmt].bits >> 3;
}

static inline void ff_framequeue_update_peeked(FFFrameQueue *fq, size_t idx)
{
}

void ff_framequeue_skip_samples(FFFrameQueue *fq, size_t samples, AVRational time_base)
{
    FFFrameBucket *b;
    size_t bytes;
    int planar, planes, i;

    check_consistency(fq);
    av_assert1(fq->queued);
    b = bucket(fq, 0);
    av_assert1(samples < b->frame->nb_samples);
    planar = av_sample_fmt_is_planar(b->frame->format);
    planes = planar ? b->frame->channels : 1;
    bytes = samples * av_get_bytes_per_sample(b->frame->format);
    if (!planar)
        bytes *= b->frame->channels;
    if (b->frame->pts != AV_NOPTS_VALUE)
        b->frame->pts += av_rescale_q(samples, av_make_q(1, b->frame->sample_rate), time_base);
    b->frame->nb_samples -= samples;
    b->frame->linesize[0] -= bytes;
    for (i = 0; i < planes; i++)
        b->frame->extended_data[i] += bytes;
    for (i = 0; i < planes && i < AV_NUM_DATA_POINTERS; i++)
        b->frame->data[i] = b->frame->extended_data[i];
    fq->total_samples_tail += samples;
    fq->samples_skipped = 1;
    ff_framequeue_update_peeked(fq, 0);
}


static int take_samples(AVFilterLink *link, unsigned min, unsigned max,
                        AVFrame **rframe)
{
    AVFrame *frame0, *frame, *buf;
    unsigned nb_samples, nb_frames, i, p;
    int ret;
    av_assert1(samples_ready(link, link->min_samples));
    frame0 = frame = ff_framequeue_peek(&link->fifo, 0);
    if (!link->fifo.samples_skipped && frame->nb_samples >= min && frame->nb_samples <= max) {
        *rframe = ff_framequeue_take(&link->fifo);
        return 0;
    }
    nb_frames = 0;
    nb_samples = 0;
    while (1) {
        if (nb_samples + frame->nb_samples > max) {
            if (nb_samples < min)
                nb_samples = max;
            break;
        }
        nb_samples += frame->nb_samples;
        nb_frames++;
        if (nb_frames == ff_framequeue_queued_frames(&link->fifo))
            break;
        frame = ff_framequeue_peek(&link->fifo, nb_frames);
    }

    buf = ff_get_audio_buffer(link, nb_samples);
    if (!buf)
        return AVERROR(ENOMEM);
    ret = av_frame_copy_props(buf, frame0);
    if (ret < 0) {
        av_frame_free(&buf);
        return ret;
    }
    buf->pts = frame0->pts;

    p = 0;
    for (i = 0; i < nb_frames; i++) {
        frame = ff_framequeue_take(&link->fifo);
        av_samples_copy(buf->extended_data, frame->extended_data, p, 0,
                        frame->nb_samples, link->channels, link->format);
        p += frame->nb_samples;
        av_frame_free(&frame);
    }
    if (p < nb_samples) {
        unsigned n = nb_samples - p;
        frame = ff_framequeue_peek(&link->fifo, 0);
        av_samples_copy(buf->extended_data, frame->extended_data, p, 0, n,
                        link->channels, link->format);
        ff_framequeue_skip_samples(&link->fifo, n, link->time_base);
    }

    *rframe = buf;
    return 0;
}


int ff_inlink_consume_samples(AVFilterLink *link, unsigned min, unsigned max,
                            AVFrame **rframe)
{
    AVFrame *frame;
    int ret;

    av_assert1(min);
    *rframe = NULL;
    if (!ff_inlink_check_available_samples(link, min))
        return 0;
    if (link->status_in)
        min = FFMIN(min, ff_framequeue_queued_samples(&link->fifo));
    ret = take_samples(link, min, max, &frame);
    if (ret < 0)
        return ret;
    consume_update(link, frame);
    *rframe = frame;
    return 1;
}

static int get_frame_internal(AVFilterContext *ctx, AVFrame *frame, int flags, int samples)
{
    BufferSinkContext *buf = ctx->priv;
    AVFilterLink *inlink = ctx->inputs[0];
    int status, ret;
    AVFrame *cur_frame;
    int64_t pts;

    if (buf->peeked_frame)
        return return_or_keep_frame(buf, frame, buf->peeked_frame, flags);

    while (1) {
        ret = samples ? ff_inlink_consume_samples(inlink, samples, samples, &cur_frame) :
                        ff_inlink_consume_frame(inlink, &cur_frame);
        if (ret < 0) {
            return ret;
        } else if (ret) {
            /* TODO return the frame instead of copying it */
            return return_or_keep_frame(buf, frame, cur_frame, flags);
        } else if (ff_inlink_acknowledge_status(inlink, &status, &pts)) {
            return status;
        } else if ((flags & AV_BUFFERSINK_FLAG_NO_REQUEST)) {
            return AVERROR(EAGAIN);
        } else if (inlink->frame_wanted_out) {
            ret = ff_filter_graph_run_once(ctx->graph);
            if (ret < 0)
                return ret;
        } else {
            ff_inlink_request_frame(inlink);
        }
    }
}

int attribute_align_arg av_buffersink_get_frame_flags(AVFilterContext *ctx, AVFrame *frame, int flags)
{
    return get_frame_internal(ctx, frame, flags, ctx->inputs[0]->min_samples);
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
        return AVERROR(ENOMEM);

    do
    {
        if ((got_frame = decoder_decode_frame(&is->auddec, frame, NULL)) < 0)
            goto the_end;

        if (got_frame)
        {
            tb = (AVRational){1, frame->sample_rate};

            dec_channel_layout = get_valid_channel_layout(frame->channel_layout, frame->channels);

            reconfigure =
                cmp_audio_fmts(is->audio_filter_src.fmt, is->audio_filter_src.channels,
                               frame->format, frame->channels) ||
                is->audio_filter_src.channel_layout != dec_channel_layout ||
                is->audio_filter_src.freq != frame->sample_rate ||
                is->auddec.pkt_serial != last_serial;

            if (reconfigure)
            {
                char buf1[1024], buf2[1024];
                av_get_channel_layout_string(buf1, sizeof(buf1), -1, is->audio_filter_src.channel_layout);
                av_get_channel_layout_string(buf2, sizeof(buf2), -1, dec_channel_layout);
                av_log(NULL, AV_LOG_DEBUG,
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

            while ((ret = av_buffersink_get_frame_flags(is->out_audio_filter, frame, 0)) >= 0)
            {
                tb = av_buffersink_get_time_base(is->out_audio_filter);
                if (!(af = frame_queue_peek_writable(&is->sampq)))
                    goto the_end;

                af->pts = (frame->pts == AV_NOPTS_VALUE) ? NAN : frame->pts * av_q2d(tb);
                af->pos = frame->pkt_pos;
                af->serial = is->auddec.pkt_serial;
                af->duration = av_q2d((AVRational){frame->nb_samples, frame->sample_rate});

                av_frame_move_ref(af->frame, frame);
                frame_queue_push(&is->sampq);

                if (is->audioq.serial != is->auddec.pkt_serial)
                    break;
            }
            if (ret == AVERROR_EOF)
                is->auddec.finished = is->auddec.pkt_serial;
        }
    } while (ret >= 0 || ret == AVERROR(EAGAIN) || ret == AVERROR_EOF);
the_end:
    avfilter_graph_free(&is->agraph);
    av_frame_free(&frame);
    return ret;
}

static int decoder_start(Decoder *d, int (*fn)(void *), const char *thread_name, void *arg)
{
    packet_queue_start(d->queue);
    d->decoder_tid = SDL_CreateThread(fn, thread_name, arg);
    if (!d->decoder_tid)
    {
        av_log(NULL, AV_LOG_ERROR, "SDL_CreateThread(): %s\n", SDL_GetError());
        return AVERROR(ENOMEM);
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
    AVRational frame_rate = av_guess_frame_rate(is->ic, is->video_st, NULL);

    AVFilterGraph *graph = NULL;
    AVFilterContext *filt_out = NULL, *filt_in = NULL;
    int last_w = 0;
    int last_h = 0;
    enum AVPixelFormat last_format = -2;
    int last_serial = -1;
    int last_vfilter_idx = 0;

    if (!frame)
        return AVERROR(ENOMEM);

    for (;;)
    {
        ret = get_video_frame(is, frame);
        if (ret < 0)
            goto the_end;
        if (!ret)
            continue;

        if (last_w != frame->width || last_h != frame->height || last_format != frame->format || last_serial != is->viddec.pkt_serial || last_vfilter_idx != is->vfilter_idx)
        {
            av_log(NULL, AV_LOG_DEBUG,
                   "Video frame changed from size:%dx%d format:%s serial:%d to size:%dx%d format:%s serial:%d\n",
                   last_w, last_h,
                   (const char *)av_x_if_null(av_get_pix_fmt_name(last_format), "none"), last_serial,
                   frame->width, frame->height,
                   (const char *)av_x_if_null(av_get_pix_fmt_name(frame->format), "none"), is->viddec.pkt_serial);
            avfilter_graph_free(&graph);
            graph = avfilter_graph_alloc();
            if (!graph)
            {
                ret = AVERROR(ENOMEM);
                goto the_end;
            }
            graph->nb_threads = filter_nbthreads;
            if ((ret = configure_video_filters(graph, is, vfilters_list ? vfilters_list[is->vfilter_idx] : NULL, frame)) < 0)
            {
                SDL_Event event;
                event.type = FF_QUIT_EVENT;
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

        while (ret >= 0)
        {
            is->frame_last_returned_time = av_gettime_relative() / 1000000.0;

            ret = av_buffersink_get_frame_flags(filt_out, frame, 0);
            if (ret < 0)
            {
                if (ret == AVERROR_EOF)
                    is->viddec.finished = is->viddec.pkt_serial;
                ret = 0;
                break;
            }

            is->frame_last_filter_delay = av_gettime_relative() / 1000000.0 - is->frame_last_returned_time;
            if (fabs(is->frame_last_filter_delay) > AV_NOSYNC_THRESHOLD / 10.0)
                is->frame_last_filter_delay = 0;
            tb = av_buffersink_get_time_base(filt_out);
            duration = (frame_rate.num && frame_rate.den ? av_q2d((AVRational){frame_rate.den, frame_rate.num}) : 0);
            pts = (frame->pts == AV_NOPTS_VALUE) ? NAN : frame->pts * av_q2d(tb);
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

    for (;;)
    {
        if (!(sp = frame_queue_peek_writable(&is->subpq)))
            return 0;

        if ((got_subtitle = decoder_decode_frame(&is->subdec, NULL, &sp->sub)) < 0)
            break;

        pts = 0;

        if (got_subtitle && sp->sub.format == 0)
        {
            if (sp->sub.pts != AV_NOPTS_VALUE)
                pts = sp->sub.pts / (double)AV_TIME_BASE;
            sp->pts = pts;
            sp->serial = is->subdec.pkt_serial;
            sp->width = is->subdec.avctx->width;
            sp->height = is->subdec.avctx->height;
            sp->uploaded = 0;

            /* now we can update the picture count */
            frame_queue_push(&is->subpq);
        }
        else if (got_subtitle)
        {
            avsubtitle_free(&sp->sub);
        }
    }
    return 0;
}

/* copy samples for viewing in editor window */
static void update_sample_display(VideoState *is, short *samples, int samples_size)
{
    int size, len;

    size = samples_size / sizeof(short);
    while (size > 0)
    {
        len = SAMPLE_ARRAY_SIZE - is->sample_array_index;
        if (len > size)
            len = size;
        memcpy(is->sample_array + is->sample_array_index, samples, len * sizeof(short));
        samples += len;
        is->sample_array_index += len;
        if (is->sample_array_index >= SAMPLE_ARRAY_SIZE)
            is->sample_array_index = 0;
        size -= len;
    }
}

/* return the wanted number of samples to get better sync if sync_type is video
 * or external master clock */
static int synchronize_audio(VideoState *is, int nb_samples)
{
    int wanted_nb_samples = nb_samples;

    /* if not master, then we try to remove or add samples to correct the clock */
    if (get_master_sync_type(is) != AV_SYNC_AUDIO_MASTER)
    {
        double diff, avg_diff;
        int min_nb_samples, max_nb_samples;

        diff = get_clock(&is->audclk) - get_master_clock(is);

        if (!isnan(diff) && fabs(diff) < AV_NOSYNC_THRESHOLD)
        {
            is->audio_diff_cum = diff + is->audio_diff_avg_coef * is->audio_diff_cum;
            if (is->audio_diff_avg_count < AUDIO_DIFF_AVG_NB)
            {
                /* not enough measures to have a correct estimate */
                is->audio_diff_avg_count++;
            }
            else
            {
                /* estimate the A-V difference */
                avg_diff = is->audio_diff_cum * (1.0 - is->audio_diff_avg_coef);

                if (fabs(avg_diff) >= is->audio_diff_threshold)
                {
                    wanted_nb_samples = nb_samples + (int)(diff * is->audio_src.freq);
                    min_nb_samples = ((nb_samples * (100 - SAMPLE_CORRECTION_PERCENT_MAX) / 100));
                    max_nb_samples = ((nb_samples * (100 + SAMPLE_CORRECTION_PERCENT_MAX) / 100));
                    wanted_nb_samples = av_clip(wanted_nb_samples, min_nb_samples, max_nb_samples);
                }
                av_log(NULL, AV_LOG_TRACE, "diff=%f adiff=%f sample_diff=%d apts=%0.3f %f\n",
                       diff, avg_diff, wanted_nb_samples - nb_samples,
                       is->audio_clock, is->audio_diff_threshold);
            }
        }
        else
        {
            /* too big difference : may be initial PTS errors, so
               reset A-V filter */
            is->audio_diff_avg_count = 0;
            is->audio_diff_cum = 0;
        }
    }

    return wanted_nb_samples;
}

int av_usleep(unsigned usec)
{
#if HAVE_NANOSLEEP
    struct timespec ts = { usec / 1000000, usec % 1000000 * 1000 };
    while (nanosleep(&ts, &ts) < 0 && errno == EINTR);
    return 0;
#elif HAVE_USLEEP
    return usleep(usec);
#elif HAVE_SLEEP
    Sleep(usec / 1000);
    return 0;
#else
    return AVERROR(ENOSYS);
#endif
}


const AVClass *sws_get_class(void)
{
    return &ff_sws_context_class;
}

static const char* context_to_name(void* ptr) {
    return "SWR";
}
static const AVOption options[]={
{"ich"                  , "set input channel count"     , OFFSET(user_in_ch_count  ), AV_OPT_TYPE_INT, {.i64=0                    }, 0      , SWR_CH_MAX, PARAM},
{"in_channel_count"     , "set input channel count"     , OFFSET(user_in_ch_count  ), AV_OPT_TYPE_INT, {.i64=0                    }, 0      , SWR_CH_MAX, PARAM},
{"och"                  , "set output channel count"    , OFFSET(user_out_ch_count ), AV_OPT_TYPE_INT, {.i64=0                    }, 0      , SWR_CH_MAX, PARAM},
{"out_channel_count"    , "set output channel count"    , OFFSET(user_out_ch_count ), AV_OPT_TYPE_INT, {.i64=0                    }, 0      , SWR_CH_MAX, PARAM},
{"uch"                  , "set used channel count"      , OFFSET(user_used_ch_count), AV_OPT_TYPE_INT, {.i64=0                    }, 0      , SWR_CH_MAX, PARAM},
{"used_channel_count"   , "set used channel count"      , OFFSET(user_used_ch_count), AV_OPT_TYPE_INT, {.i64=0                    }, 0      , SWR_CH_MAX, PARAM},
{"isr"                  , "set input sample rate"       , OFFSET( in_sample_rate), AV_OPT_TYPE_INT  , {.i64=0                     }, 0      , INT_MAX   , PARAM},
{"in_sample_rate"       , "set input sample rate"       , OFFSET( in_sample_rate), AV_OPT_TYPE_INT  , {.i64=0                     }, 0      , INT_MAX   , PARAM},
{"osr"                  , "set output sample rate"      , OFFSET(out_sample_rate), AV_OPT_TYPE_INT  , {.i64=0                     }, 0      , INT_MAX   , PARAM},
{"out_sample_rate"      , "set output sample rate"      , OFFSET(out_sample_rate), AV_OPT_TYPE_INT  , {.i64=0                     }, 0      , INT_MAX   , PARAM},
{"isf"                  , "set input sample format"     , OFFSET( in_sample_fmt ), AV_OPT_TYPE_SAMPLE_FMT , {.i64=AV_SAMPLE_FMT_NONE}, -1   , INT_MAX, PARAM},
{"in_sample_fmt"        , "set input sample format"     , OFFSET( in_sample_fmt ), AV_OPT_TYPE_SAMPLE_FMT , {.i64=AV_SAMPLE_FMT_NONE}, -1   , INT_MAX, PARAM},
{"osf"                  , "set output sample format"    , OFFSET(out_sample_fmt ), AV_OPT_TYPE_SAMPLE_FMT , {.i64=AV_SAMPLE_FMT_NONE}, -1   , INT_MAX, PARAM},
{"out_sample_fmt"       , "set output sample format"    , OFFSET(out_sample_fmt ), AV_OPT_TYPE_SAMPLE_FMT , {.i64=AV_SAMPLE_FMT_NONE}, -1   , INT_MAX, PARAM},
{"tsf"                  , "set internal sample format"  , OFFSET(user_int_sample_fmt), AV_OPT_TYPE_SAMPLE_FMT , {.i64=AV_SAMPLE_FMT_NONE}, -1   , INT_MAX, PARAM},
{"internal_sample_fmt"  , "set internal sample format"  , OFFSET(user_int_sample_fmt), AV_OPT_TYPE_SAMPLE_FMT , {.i64=AV_SAMPLE_FMT_NONE}, -1   , INT_MAX, PARAM},
{"icl"                  , "set input channel layout"    , OFFSET(user_in_ch_layout ), AV_OPT_TYPE_CHANNEL_LAYOUT, {.i64=0           }, INT64_MIN, INT64_MAX , PARAM, "channel_layout"},
{"in_channel_layout"    , "set input channel layout"    , OFFSET(user_in_ch_layout ), AV_OPT_TYPE_CHANNEL_LAYOUT, {.i64=0           }, INT64_MIN, INT64_MAX , PARAM, "channel_layout"},
{"ocl"                  , "set output channel layout"   , OFFSET(user_out_ch_layout), AV_OPT_TYPE_CHANNEL_LAYOUT, {.i64=0           }, INT64_MIN, INT64_MAX , PARAM, "channel_layout"},
{"out_channel_layout"   , "set output channel layout"   , OFFSET(user_out_ch_layout), AV_OPT_TYPE_CHANNEL_LAYOUT, {.i64=0           }, INT64_MIN, INT64_MAX , PARAM, "channel_layout"},
{"clev"                 , "set center mix level"        , OFFSET(clev           ), AV_OPT_TYPE_FLOAT, {.dbl=C_30DB                }, -32    , 32        , PARAM},
{"center_mix_level"     , "set center mix level"        , OFFSET(clev           ), AV_OPT_TYPE_FLOAT, {.dbl=C_30DB                }, -32    , 32        , PARAM},
{"slev"                 , "set surround mix level"      , OFFSET(slev           ), AV_OPT_TYPE_FLOAT, {.dbl=C_30DB                }, -32    , 32        , PARAM},
{"surround_mix_level"   , "set surround mix Level"      , OFFSET(slev           ), AV_OPT_TYPE_FLOAT, {.dbl=C_30DB                }, -32    , 32        , PARAM},
{"lfe_mix_level"        , "set LFE mix level"           , OFFSET(lfe_mix_level  ), AV_OPT_TYPE_FLOAT, {.dbl=0                     }, -32    , 32        , PARAM},
{"rmvol"                , "set rematrix volume"         , OFFSET(rematrix_volume), AV_OPT_TYPE_FLOAT, {.dbl=1.0                   }, -1000  , 1000      , PARAM},
{"rematrix_volume"      , "set rematrix volume"         , OFFSET(rematrix_volume), AV_OPT_TYPE_FLOAT, {.dbl=1.0                   }, -1000  , 1000      , PARAM},
{"rematrix_maxval"      , "set rematrix maxval"         , OFFSET(rematrix_maxval), AV_OPT_TYPE_FLOAT, {.dbl=0.0                   }, 0      , 1000      , PARAM},

{"flags"                , "set flags"                   , OFFSET(flags          ), AV_OPT_TYPE_FLAGS, {.i64=0                     }, 0      , UINT_MAX  , PARAM, "flags"},
{"swr_flags"            , "set flags"                   , OFFSET(flags          ), AV_OPT_TYPE_FLAGS, {.i64=0                     }, 0      , UINT_MAX  , PARAM, "flags"},
{"res"                  , "force resampling"            , 0                      , AV_OPT_TYPE_CONST, {.i64=SWR_FLAG_RESAMPLE     }, INT_MIN, INT_MAX   , PARAM, "flags"},

{"dither_scale"         , "set dither scale"            , OFFSET(dither.scale   ), AV_OPT_TYPE_FLOAT, {.dbl=1                     }, 0      , INT_MAX   , PARAM},

{"dither_method"        , "set dither method"           , OFFSET(user_dither_method),AV_OPT_TYPE_INT, {.i64=0                     }, 0      , SWR_DITHER_NB-1, PARAM, "dither_method"},
{"rectangular"          , "select rectangular dither"   , 0                      , AV_OPT_TYPE_CONST, {.i64=SWR_DITHER_RECTANGULAR}, INT_MIN, INT_MAX   , PARAM, "dither_method"},
{"triangular"           , "select triangular dither"    , 0                      , AV_OPT_TYPE_CONST, {.i64=SWR_DITHER_TRIANGULAR }, INT_MIN, INT_MAX   , PARAM, "dither_method"},
{"triangular_hp"        , "select triangular dither with high pass" , 0          , AV_OPT_TYPE_CONST, {.i64=SWR_DITHER_TRIANGULAR_HIGHPASS }, INT_MIN, INT_MAX, PARAM, "dither_method"},
{"lipshitz"             , "select Lipshitz noise shaping dither" , 0             , AV_OPT_TYPE_CONST, {.i64=SWR_DITHER_NS_LIPSHITZ}, INT_MIN, INT_MAX, PARAM, "dither_method"},
{"shibata"              , "select Shibata noise shaping dither" , 0              , AV_OPT_TYPE_CONST, {.i64=SWR_DITHER_NS_SHIBATA }, INT_MIN, INT_MAX, PARAM, "dither_method"},
{"low_shibata"          , "select low Shibata noise shaping dither" , 0          , AV_OPT_TYPE_CONST, {.i64=SWR_DITHER_NS_LOW_SHIBATA }, INT_MIN, INT_MAX, PARAM, "dither_method"},
{"high_shibata"         , "select high Shibata noise shaping dither" , 0         , AV_OPT_TYPE_CONST, {.i64=SWR_DITHER_NS_HIGH_SHIBATA }, INT_MIN, INT_MAX, PARAM, "dither_method"},
{"f_weighted"           , "select f-weighted noise shaping dither" , 0           , AV_OPT_TYPE_CONST, {.i64=SWR_DITHER_NS_F_WEIGHTED }, INT_MIN, INT_MAX, PARAM, "dither_method"},
{"modified_e_weighted"  , "select modified-e-weighted noise shaping dither" , 0  , AV_OPT_TYPE_CONST, {.i64=SWR_DITHER_NS_MODIFIED_E_WEIGHTED }, INT_MIN, INT_MAX, PARAM, "dither_method"},
{"improved_e_weighted"  , "select improved-e-weighted noise shaping dither" , 0  , AV_OPT_TYPE_CONST, {.i64=SWR_DITHER_NS_IMPROVED_E_WEIGHTED }, INT_MIN, INT_MAX, PARAM, "dither_method"},

{"filter_size"          , "set swr resampling filter size", OFFSET(filter_size)  , AV_OPT_TYPE_INT  , {.i64=32                    }, 0      , INT_MAX   , PARAM },
{"phase_shift"          , "set swr resampling phase shift", OFFSET(phase_shift)  , AV_OPT_TYPE_INT  , {.i64=10                    }, 0      , 24        , PARAM },
{"linear_interp"        , "enable linear interpolation" , OFFSET(linear_interp)  , AV_OPT_TYPE_BOOL , {.i64=1                     }, 0      , 1         , PARAM },
{"exact_rational"       , "enable exact rational"       , OFFSET(exact_rational) , AV_OPT_TYPE_BOOL , {.i64=1                     }, 0      , 1         , PARAM },
{"cutoff"               , "set cutoff frequency ratio"  , OFFSET(cutoff)         , AV_OPT_TYPE_DOUBLE,{.dbl=0.                    }, 0      , 1         , PARAM },

/* duplicate option in order to work with avconv */
{"resample_cutoff"      , "set cutoff frequency ratio"  , OFFSET(cutoff)         , AV_OPT_TYPE_DOUBLE,{.dbl=0.                    }, 0      , 1         , PARAM },

{"resampler"            , "set resampling Engine"       , OFFSET(engine)         , AV_OPT_TYPE_INT  , {.i64=0                     }, 0      , SWR_ENGINE_NB-1, PARAM, "resampler"},
{"swr"                  , "select SW Resampler"         , 0                      , AV_OPT_TYPE_CONST, {.i64=SWR_ENGINE_SWR        }, INT_MIN, INT_MAX   , PARAM, "resampler"},
{"soxr"                 , "select SoX Resampler"        , 0                      , AV_OPT_TYPE_CONST, {.i64=SWR_ENGINE_SOXR       }, INT_MIN, INT_MAX   , PARAM, "resampler"},
{"precision"            , "set soxr resampling precision (in bits)"
                                                        , OFFSET(precision)      , AV_OPT_TYPE_DOUBLE,{.dbl=20.0                  }, 15.0   , 33.0      , PARAM },
{"cheby"                , "enable soxr Chebyshev passband & higher-precision irrational ratio approximation"
                                                        , OFFSET(cheby)          , AV_OPT_TYPE_BOOL , {.i64=0                     }, 0      , 1         , PARAM },
{"min_comp"             , "set minimum difference between timestamps and audio data (in seconds) below which no timestamp compensation of either kind is applied"
                                                        , OFFSET(min_compensation),AV_OPT_TYPE_FLOAT ,{.dbl=FLT_MAX               }, 0      , FLT_MAX   , PARAM },
{"min_hard_comp"        , "set minimum difference between timestamps and audio data (in seconds) to trigger padding/trimming the data."
                                                        , OFFSET(min_hard_compensation),AV_OPT_TYPE_FLOAT ,{.dbl=0.1                   }, 0      , INT_MAX   , PARAM },
{"comp_duration"        , "set duration (in seconds) over which data is stretched/squeezed to make it match the timestamps."
                                                        , OFFSET(soft_compensation_duration),AV_OPT_TYPE_FLOAT ,{.dbl=1                     }, 0      , INT_MAX   , PARAM },
{"max_soft_comp"        , "set maximum factor by which data is stretched/squeezed to make it match the timestamps."
                                                        , OFFSET(max_soft_compensation),AV_OPT_TYPE_FLOAT ,{.dbl=0                     }, INT_MIN, INT_MAX   , PARAM },
{"async"                , "simplified 1 parameter audio timestamp matching, 0(disabled), 1(filling and trimming), >1(maximum stretch/squeeze in samples per second)"
                                                        , OFFSET(async)          , AV_OPT_TYPE_FLOAT ,{.dbl=0                     }, INT_MIN, INT_MAX   , PARAM },
{"first_pts"            , "Assume the first pts should be this value (in samples)."
                                                        , OFFSET(firstpts_in_samples), AV_OPT_TYPE_INT64 ,{.i64=AV_NOPTS_VALUE    }, INT64_MIN,INT64_MAX, PARAM },

{ "matrix_encoding"     , "set matrixed stereo encoding" , OFFSET(matrix_encoding), AV_OPT_TYPE_INT   ,{.i64 = AV_MATRIX_ENCODING_NONE}, AV_MATRIX_ENCODING_NONE,     AV_MATRIX_ENCODING_NB-1, PARAM, "matrix_encoding" },
    { "none",  "select none",               0, AV_OPT_TYPE_CONST, { .i64 = AV_MATRIX_ENCODING_NONE  }, INT_MIN, INT_MAX, PARAM, "matrix_encoding" },
    { "dolby", "select Dolby",              0, AV_OPT_TYPE_CONST, { .i64 = AV_MATRIX_ENCODING_DOLBY }, INT_MIN, INT_MAX, PARAM, "matrix_encoding" },
    { "dplii", "select Dolby Pro Logic II", 0, AV_OPT_TYPE_CONST, { .i64 = AV_MATRIX_ENCODING_DPLII }, INT_MIN, INT_MAX, PARAM, "matrix_encoding" },

{ "filter_type"         , "select swr filter type"      , OFFSET(filter_type)    , AV_OPT_TYPE_INT  , { .i64 = SWR_FILTER_TYPE_KAISER }, SWR_FILTER_TYPE_CUBIC, SWR_FILTER_TYPE_KAISER, PARAM, "filter_type" },
    { "cubic"           , "select cubic"                , 0                      , AV_OPT_TYPE_CONST, { .i64 = SWR_FILTER_TYPE_CUBIC            }, INT_MIN, INT_MAX, PARAM, "filter_type" },
    { "blackman_nuttall", "select Blackman Nuttall windowed sinc", 0             , AV_OPT_TYPE_CONST, { .i64 = SWR_FILTER_TYPE_BLACKMAN_NUTTALL }, INT_MIN, INT_MAX, PARAM, "filter_type" },
    { "kaiser"          , "select Kaiser windowed sinc" , 0                      , AV_OPT_TYPE_CONST, { .i64 = SWR_FILTER_TYPE_KAISER           }, INT_MIN, INT_MAX, PARAM, "filter_type" },

{ "kaiser_beta"         , "set swr Kaiser window beta"  , OFFSET(kaiser_beta)    , AV_OPT_TYPE_DOUBLE  , {.dbl=9                     }, 2      , 16        , PARAM },

{ "output_sample_bits"  , "set swr number of output sample bits", OFFSET(dither.output_sample_bits), AV_OPT_TYPE_INT  , {.i64=0   }, 0      , 64        , PARAM },
{0}
};
static const AVClass av_class = {
    .class_name                = "SWResampler",
    .item_name                 = context_to_name,
    .option                    = options,
    .version                   = LIBAVUTIL_VERSION_INT,
    .log_level_offset_offset   = OFFSET(log_level_offset),
    .parent_log_context_offset = OFFSET(log_ctx),
    .category                  = AV_CLASS_CATEGORY_SWRESAMPLER,
};
const AVClass *swr_get_class(void)
{
    return &av_class;
}

av_cold struct SwrContext *swr_alloc(void){
    SwrContext *s= av_mallocz(sizeof(SwrContext));
    if(s){
        s->av_class= &av_class;
        av_opt_set_defaults(s);
    }
    return s;
}

struct SwrContext *swr_alloc_set_opts(struct SwrContext *s,
                                      int64_t out_ch_layout, enum AVSampleFormat out_sample_fmt, int out_sample_rate,
                                      int64_t  in_ch_layout, enum AVSampleFormat  in_sample_fmt, int  in_sample_rate,
                                      int log_offset, void *log_ctx){
    if(!s) s= swr_alloc();
    if(!s) return NULL;

    s->log_level_offset= log_offset;
    s->log_ctx= log_ctx;

    if (av_opt_set_int(s, "ocl", out_ch_layout,   0) < 0)
        goto fail;

    if (av_opt_set_int(s, "osf", out_sample_fmt,  0) < 0)
        goto fail;

    if (av_opt_set_int(s, "osr", out_sample_rate, 0) < 0)
        goto fail;

    if (av_opt_set_int(s, "icl", in_ch_layout,    0) < 0)
        goto fail;

    if (av_opt_set_int(s, "isf", in_sample_fmt,   0) < 0)
        goto fail;

    if (av_opt_set_int(s, "isr", in_sample_rate,  0) < 0)
        goto fail;

    if (av_opt_set_int(s, "ich", av_get_channel_layout_nb_channels(s-> user_in_ch_layout), 0) < 0)
        goto fail;

    if (av_opt_set_int(s, "och", av_get_channel_layout_nb_channels(s->user_out_ch_layout), 0) < 0)
        goto fail;

    av_opt_set_int(s, "uch", 0, 0);
    return s;
fail:
    av_log(s, AV_LOG_ERROR, "Failed to set option\n");
    swr_free(&s);
    return NULL;
}

static void set_audiodata_fmt(AudioData *a, enum AVSampleFormat fmt){
    a->fmt   = fmt;
    a->bps   = av_get_bytes_per_sample(fmt);
    a->planar= av_sample_fmt_is_planar(fmt);
    if (a->ch_count == 1)
        a->planar = 1;
}

static void clear_context(SwrContext *s){
    s->in_buffer_index= 0;
    s->in_buffer_count= 0;
    s->resample_in_constraint= 0;
    memset(s->in.ch, 0, sizeof(s->in.ch));
    memset(s->out.ch, 0, sizeof(s->out.ch));
    free_temp(&s->postin);
    free_temp(&s->midbuf);
    free_temp(&s->preout);
    free_temp(&s->in_buffer);
    free_temp(&s->silence);
    free_temp(&s->drop_temp);
    free_temp(&s->dither.noise);
    free_temp(&s->dither.temp);
    swri_audio_convert_free(&s-> in_convert);
    swri_audio_convert_free(&s->out_convert);
    swri_audio_convert_free(&s->full_convert);
    swri_rematrix_free(s);

    s->delayed_samples_fixup = 0;
    s->flushed = 0;
}

av_cold int swr_init(struct SwrContext *s){
    int ret;
    char l1[1024], l2[1024];

    clear_context(s);

    if(s-> in_sample_fmt >= AV_SAMPLE_FMT_NB){
        av_log(s, AV_LOG_ERROR, "Requested input sample format %d is invalid\n", s->in_sample_fmt);
        return AVERROR(EINVAL);
    }
    if(s->out_sample_fmt >= AV_SAMPLE_FMT_NB){
        av_log(s, AV_LOG_ERROR, "Requested output sample format %d is invalid\n", s->out_sample_fmt);
        return AVERROR(EINVAL);
    }

    if(s-> in_sample_rate <= 0){
        av_log(s, AV_LOG_ERROR, "Requested input sample rate %d is invalid\n", s->in_sample_rate);
        return AVERROR(EINVAL);
    }
    if(s->out_sample_rate <= 0){
        av_log(s, AV_LOG_ERROR, "Requested output sample rate %d is invalid\n", s->out_sample_rate);
        return AVERROR(EINVAL);
    }
    s->out.ch_count  = s-> user_out_ch_count;
    s-> in.ch_count  = s->  user_in_ch_count;
    s->used_ch_count = s->user_used_ch_count;

    s-> in_ch_layout = s-> user_in_ch_layout;
    s->out_ch_layout = s->user_out_ch_layout;

    s->int_sample_fmt= s->user_int_sample_fmt;

    s->dither.method = s->user_dither_method;

    if(av_get_channel_layout_nb_channels(s-> in_ch_layout) > SWR_CH_MAX) {
        av_log(s, AV_LOG_WARNING, "Input channel layout 0x%"PRIx64" is invalid or unsupported.\n", s-> in_ch_layout);
        s->in_ch_layout = 0;
    }

    if(av_get_channel_layout_nb_channels(s->out_ch_layout) > SWR_CH_MAX) {
        av_log(s, AV_LOG_WARNING, "Output channel layout 0x%"PRIx64" is invalid or unsupported.\n", s->out_ch_layout);
        s->out_ch_layout = 0;
    }

    switch(s->engine){
#if CONFIG_LIBSOXR
        case SWR_ENGINE_SOXR: s->resampler = &swri_soxr_resampler; break;
#endif
        case SWR_ENGINE_SWR : s->resampler = &swri_resampler; break;
        default:
            av_log(s, AV_LOG_ERROR, "Requested resampling engine is unavailable\n");
            return AVERROR(EINVAL);
    }

    if(!s->used_ch_count)
        s->used_ch_count= s->in.ch_count;

    if(s->used_ch_count && s-> in_ch_layout && s->used_ch_count != av_get_channel_layout_nb_channels(s-> in_ch_layout)){
        av_log(s, AV_LOG_WARNING, "Input channel layout has a different number of channels than the number of used channels, ignoring layout\n");
        s-> in_ch_layout= 0;
    }

    if(!s-> in_ch_layout)
        s-> in_ch_layout= av_get_default_channel_layout(s->used_ch_count);
    if(!s->out_ch_layout)
        s->out_ch_layout= av_get_default_channel_layout(s->out.ch_count);

    s->rematrix= s->out_ch_layout  !=s->in_ch_layout || s->rematrix_volume!=1.0 ||
                 s->rematrix_custom;

    if(s->int_sample_fmt == AV_SAMPLE_FMT_NONE){
        if(   av_get_bytes_per_sample(s-> in_sample_fmt) <= 2
           && av_get_bytes_per_sample(s->out_sample_fmt) <= 2){
            s->int_sample_fmt= AV_SAMPLE_FMT_S16P;
        }else if(   av_get_bytes_per_sample(s-> in_sample_fmt) <= 2
           && !s->rematrix
           && s->out_sample_rate==s->in_sample_rate
           && !(s->flags & SWR_FLAG_RESAMPLE)){
            s->int_sample_fmt= AV_SAMPLE_FMT_S16P;
        }else if(   av_get_planar_sample_fmt(s-> in_sample_fmt) == AV_SAMPLE_FMT_S32P
                 && av_get_planar_sample_fmt(s->out_sample_fmt) == AV_SAMPLE_FMT_S32P
                 && !s->rematrix
                 && s->out_sample_rate == s->in_sample_rate
                 && !(s->flags & SWR_FLAG_RESAMPLE)
                 && s->engine != SWR_ENGINE_SOXR){
            s->int_sample_fmt= AV_SAMPLE_FMT_S32P;
        }else if(av_get_bytes_per_sample(s->in_sample_fmt) <= 4){
            s->int_sample_fmt= AV_SAMPLE_FMT_FLTP;
        }else{
            s->int_sample_fmt= AV_SAMPLE_FMT_DBLP;
        }
    }
    av_log(s, AV_LOG_DEBUG, "Using %s internally between filters\n", av_get_sample_fmt_name(s->int_sample_fmt));

    if(   s->int_sample_fmt != AV_SAMPLE_FMT_S16P
        &&s->int_sample_fmt != AV_SAMPLE_FMT_S32P
        &&s->int_sample_fmt != AV_SAMPLE_FMT_S64P
        &&s->int_sample_fmt != AV_SAMPLE_FMT_FLTP
        &&s->int_sample_fmt != AV_SAMPLE_FMT_DBLP){
        av_log(s, AV_LOG_ERROR, "Requested sample format %s is not supported internally, s16p/s32p/s64p/fltp/dblp are supported\n", av_get_sample_fmt_name(s->int_sample_fmt));
        return AVERROR(EINVAL);
    }

    set_audiodata_fmt(&s-> in, s-> in_sample_fmt);
    set_audiodata_fmt(&s->out, s->out_sample_fmt);

    if (s->firstpts_in_samples != AV_NOPTS_VALUE) {
        if (!s->async && s->min_compensation >= FLT_MAX/2)
            s->async = 1;
        s->firstpts =
        s->outpts   = s->firstpts_in_samples * s->out_sample_rate;
    } else
        s->firstpts = AV_NOPTS_VALUE;

    if (s->async) {
        if (s->min_compensation >= FLT_MAX/2)
            s->min_compensation = 0.001;
        if (s->async > 1.0001) {
            s->max_soft_compensation = s->async / (double) s->in_sample_rate;
        }
    }

    if (s->out_sample_rate!=s->in_sample_rate || (s->flags & SWR_FLAG_RESAMPLE)){
        s->resample = s->resampler->init(s->resample, s->out_sample_rate, s->in_sample_rate, s->filter_size, s->phase_shift, s->linear_interp, s->cutoff, s->int_sample_fmt, s->filter_type, s->kaiser_beta, s->precision, s->cheby, s->exact_rational);
        if (!s->resample) {
            av_log(s, AV_LOG_ERROR, "Failed to initialize resampler\n");
            return AVERROR(ENOMEM);
        }
    }else
        s->resampler->free(&s->resample);
    if(    s->int_sample_fmt != AV_SAMPLE_FMT_S16P
        && s->int_sample_fmt != AV_SAMPLE_FMT_S32P
        && s->int_sample_fmt != AV_SAMPLE_FMT_FLTP
        && s->int_sample_fmt != AV_SAMPLE_FMT_DBLP
        && s->resample){
        av_log(s, AV_LOG_ERROR, "Resampling only supported with internal s16p/s32p/fltp/dblp\n");
        ret = AVERROR(EINVAL);
        goto fail;
    }

#define RSC 1 //FIXME finetune
    if(!s-> in.ch_count)
        s-> in.ch_count= av_get_channel_layout_nb_channels(s-> in_ch_layout);
    if(!s->used_ch_count)
        s->used_ch_count= s->in.ch_count;
    if(!s->out.ch_count)
        s->out.ch_count= av_get_channel_layout_nb_channels(s->out_ch_layout);

    if(!s-> in.ch_count){
        av_assert0(!s->in_ch_layout);
        av_log(s, AV_LOG_ERROR, "Input channel count and layout are unset\n");
        ret = AVERROR(EINVAL);
        goto fail;
    }

    av_get_channel_layout_string(l1, sizeof(l1), s-> in.ch_count, s-> in_ch_layout);
    av_get_channel_layout_string(l2, sizeof(l2), s->out.ch_count, s->out_ch_layout);
    if (s->out_ch_layout && s->out.ch_count != av_get_channel_layout_nb_channels(s->out_ch_layout)) {
        av_log(s, AV_LOG_ERROR, "Output channel layout %s mismatches specified channel count %d\n", l2, s->out.ch_count);
        ret = AVERROR(EINVAL);
        goto fail;
    }
    if (s->in_ch_layout && s->used_ch_count != av_get_channel_layout_nb_channels(s->in_ch_layout)) {
        av_log(s, AV_LOG_ERROR, "Input channel layout %s mismatches specified channel count %d\n", l1, s->used_ch_count);
        ret = AVERROR(EINVAL);
        goto fail;
    }

    if ((!s->out_ch_layout || !s->in_ch_layout) && s->used_ch_count != s->out.ch_count && !s->rematrix_custom) {
        av_log(s, AV_LOG_ERROR, "Rematrix is needed between %s and %s "
               "but there is not enough information to do it\n", l1, l2);
        ret = AVERROR(EINVAL);
        goto fail;
    }

av_assert0(s->used_ch_count);
av_assert0(s->out.ch_count);
    s->resample_first= RSC*s->out.ch_count/s->used_ch_count - RSC < s->out_sample_rate/(float)s-> in_sample_rate - 1.0;

    s->in_buffer= s->in;
    s->silence  = s->in;
    s->drop_temp= s->out;

    if ((ret = swri_dither_init(s, s->out_sample_fmt, s->int_sample_fmt)) < 0)
        goto fail;

    if(!s->resample && !s->rematrix && !s->channel_map && !s->dither.method){
        s->full_convert = swri_audio_convert_alloc(s->out_sample_fmt,
                                                   s-> in_sample_fmt, s-> in.ch_count, NULL, 0);
        return 0;
    }

    s->in_convert = swri_audio_convert_alloc(s->int_sample_fmt,
                                             s-> in_sample_fmt, s->used_ch_count, s->channel_map, 0);
    s->out_convert= swri_audio_convert_alloc(s->out_sample_fmt,
                                             s->int_sample_fmt, s->out.ch_count, NULL, 0);

    if (!s->in_convert || !s->out_convert) {
        ret = AVERROR(ENOMEM);
        goto fail;
    }

    s->postin= s->in;
    s->preout= s->out;
    s->midbuf= s->in;

    if(s->channel_map){
        s->postin.ch_count=
        s->midbuf.ch_count= s->used_ch_count;
        if(s->resample)
            s->in_buffer.ch_count= s->used_ch_count;
    }
    if(!s->resample_first){
        s->midbuf.ch_count= s->out.ch_count;
        if(s->resample)
            s->in_buffer.ch_count = s->out.ch_count;
    }

    set_audiodata_fmt(&s->postin, s->int_sample_fmt);
    set_audiodata_fmt(&s->midbuf, s->int_sample_fmt);
    set_audiodata_fmt(&s->preout, s->int_sample_fmt);

    if(s->resample){
        set_audiodata_fmt(&s->in_buffer, s->int_sample_fmt);
    }

    av_assert0(!s->preout.count);
    s->dither.noise = s->preout;
    s->dither.temp  = s->preout;
    if (s->dither.method > SWR_DITHER_NS) {
        s->dither.noise.bps = 4;
        s->dither.noise.fmt = AV_SAMPLE_FMT_FLTP;
        s->dither.noise_scale = 1;
    }

    if(s->rematrix || s->dither.method) {
        ret = swri_rematrix_init(s);
        if (ret < 0)
            goto fail;
    }

    return 0;
fail:
    swr_close(s);
    return ret;

}

int swr_set_compensation(struct SwrContext *s, int sample_delta, int compensation_distance){
    int ret;

    if (!s || compensation_distance < 0)
        return AVERROR(EINVAL);
    if (!compensation_distance && sample_delta)
        return AVERROR(EINVAL);
    if (!s->resample) {
        s->flags |= SWR_FLAG_RESAMPLE;
        ret = swr_init(s);
        if (ret < 0)
            return ret;
    }
    if (!s->resampler->set_compensation){
        return AVERROR(EINVAL);
    }else{
        return s->resampler->set_compensation(s->resample, sample_delta, compensation_distance);
    }
}

void av_fast_malloc(void *ptr, unsigned int *size, size_t min_size)
{
    ff_fast_malloc(ptr, size, min_size, 0);
}

int attribute_align_arg swr_convert(struct SwrContext *s, uint8_t *out_arg[SWR_CH_MAX], int out_count,
                                                    const uint8_t *in_arg [SWR_CH_MAX], int  in_count){
    AudioData * in= &s->in;
    AudioData *out= &s->out;
    int av_unused max_output;

    if (!swr_is_initialized(s)) {
        av_log(s, AV_LOG_ERROR, "Context has not been initialized\n");
        return AVERROR(EINVAL);
    }
#if defined(ASSERT_LEVEL) && ASSERT_LEVEL >1
    max_output = swr_get_out_samples(s, in_count);
#endif

    while(s->drop_output > 0){
        int ret;
        uint8_t *tmp_arg[SWR_CH_MAX];
#define MAX_DROP_STEP 16384
        if((ret=swri_realloc_audio(&s->drop_temp, FFMIN(s->drop_output, MAX_DROP_STEP)))<0)
            return ret;

        reversefill_audiodata(&s->drop_temp, tmp_arg);
        s->drop_output *= -1; //FIXME find a less hackish solution
        ret = swr_convert(s, tmp_arg, FFMIN(-s->drop_output, MAX_DROP_STEP), in_arg, in_count); //FIXME optimize but this is as good as never called so maybe it doesn't matter
        s->drop_output *= -1;
        in_count = 0;
        if(ret>0) {
            s->drop_output -= ret;
            if (!s->drop_output && !out_arg)
                return 0;
            continue;
        }

        av_assert0(s->drop_output);
        return 0;
    }

    if(!in_arg){
        if(s->resample){
            if (!s->flushed)
                s->resampler->flush(s);
            s->resample_in_constraint = 0;
            s->flushed = 1;
        }else if(!s->in_buffer_count){
            return 0;
        }
    }else
        fill_audiodata(in ,  (void*)in_arg);

    fill_audiodata(out, out_arg);

    if(s->resample){
        int ret = swr_convert_internal(s, out, out_count, in, in_count);
        if(ret>0 && !s->drop_output)
            s->outpts += ret * (int64_t)s->in_sample_rate;

        av_assert2(max_output < 0 || ret < 0 || ret <= max_output);

        return ret;
    }else{
        AudioData tmp= *in;
        int ret2=0;
        int ret, size;
        size = FFMIN(out_count, s->in_buffer_count);
        if(size){
            buf_set(&tmp, &s->in_buffer, s->in_buffer_index);
            ret= swr_convert_internal(s, out, size, &tmp, size);
            if(ret<0)
                return ret;
            ret2= ret;
            s->in_buffer_count -= ret;
            s->in_buffer_index += ret;
            buf_set(out, out, ret);
            out_count -= ret;
            if(!s->in_buffer_count)
                s->in_buffer_index = 0;
        }

        if(in_count){
            size= s->in_buffer_index + s->in_buffer_count + in_count - out_count;

            if(in_count > out_count) { //FIXME move after swr_convert_internal
                if(   size > s->in_buffer.count
                && s->in_buffer_count + in_count - out_count <= s->in_buffer_index){
                    buf_set(&tmp, &s->in_buffer, s->in_buffer_index);
                    copy(&s->in_buffer, &tmp, s->in_buffer_count);
                    s->in_buffer_index=0;
                }else
                    if((ret=swri_realloc_audio(&s->in_buffer, size)) < 0)
                        return ret;
            }

            if(out_count){
                size = FFMIN(in_count, out_count);
                ret= swr_convert_internal(s, out, size, in, size);
                if(ret<0)
                    return ret;
                buf_set(in, in, ret);
                in_count -= ret;
                ret2 += ret;
            }
            if(in_count){
                buf_set(&tmp, &s->in_buffer, s->in_buffer_index + s->in_buffer_count);
                copy(&tmp, in, in_count);
                s->in_buffer_count += in_count;
            }
        }
        if(ret2>0 && !s->drop_output)
            s->outpts += ret2 * (int64_t)s->in_sample_rate;
        av_assert2(max_output < 0 || ret2 < 0 || ret2 <= max_output);
        return ret2;
    }
}




static int audio_decode_frame(VideoState *is)
{
    int data_size, resampled_data_size;
    int64_t dec_channel_layout;
    av_unused double audio_clock0;
    int wanted_nb_samples;
    Frame *af;

    if (is->paused)
        return -1;

    do
    {
        // #if defined(_WIN32)
        while (frame_queue_nb_remaining(&is->sampq) == 0)
        {
            if ((av_gettime_relative() - audio_callback_time) > 1000000LL * is->audio_hw_buf_size / is->audio_tgt.bytes_per_sec / 2)
                return -1;
            av_usleep(1000);
        }
        // #endif
        if (!(af = frame_queue_peek_readable(&is->sampq)))
            return -1;
        frame_queue_next(&is->sampq);
    } while (af->serial != is->audioq.serial);

    data_size = av_samples_get_buffer_size(NULL, af->frame->channels,
                                           af->frame->nb_samples,
                                           af->frame->format, 1);

    dec_channel_layout =
        (af->frame->channel_layout && af->frame->channels == av_get_channel_layout_nb_channels(af->frame->channel_layout)) ? af->frame->channel_layout : av_get_default_channel_layout(af->frame->channels);
    wanted_nb_samples = synchronize_audio(is, af->frame->nb_samples);

    if (af->frame->format != is->audio_src.fmt ||
        dec_channel_layout != is->audio_src.channel_layout ||
        af->frame->sample_rate != is->audio_src.freq ||
        (wanted_nb_samples != af->frame->nb_samples && !is->swr_ctx))
    {
        swr_free(&is->swr_ctx);
        is->swr_ctx = swr_alloc_set_opts(NULL,
                                         is->audio_tgt.channel_layout, is->audio_tgt.fmt, is->audio_tgt.freq,
                                         dec_channel_layout, af->frame->format, af->frame->sample_rate,
                                         0, NULL);
        if (!is->swr_ctx || swr_init(is->swr_ctx) < 0)
        {
            av_log(NULL, AV_LOG_ERROR,
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

    if (is->swr_ctx)
    {
        const uint8_t **in = (const uint8_t **)af->frame->extended_data;
        uint8_t **out = &is->audio_buf1;
        int out_count = (int64_t)wanted_nb_samples * is->audio_tgt.freq / af->frame->sample_rate + 256;
        int out_size = av_samples_get_buffer_size(NULL, is->audio_tgt.channels, out_count, is->audio_tgt.fmt, 0);
        int len2;
        if (out_size < 0)
        {
            av_log(NULL, AV_LOG_ERROR, "av_samples_get_buffer_size() failed\n");
            return -1;
        }
        if (wanted_nb_samples != af->frame->nb_samples)
        {
            if (swr_set_compensation(is->swr_ctx, (wanted_nb_samples - af->frame->nb_samples) * is->audio_tgt.freq / af->frame->sample_rate,
                                     wanted_nb_samples * is->audio_tgt.freq / af->frame->sample_rate) < 0)
            {
                av_log(NULL, AV_LOG_ERROR, "swr_set_compensation() failed\n");
                return -1;
            }
        }
        av_fast_malloc(&is->audio_buf1, &is->audio_buf1_size, out_size);
        if (!is->audio_buf1)
            return AVERROR(ENOMEM);
        len2 = swr_convert(is->swr_ctx, out, out_count, in, af->frame->nb_samples);
        if (len2 < 0)
        {
            av_log(NULL, AV_LOG_ERROR, "swr_convert() failed\n");
            return -1;
        }
        if (len2 == out_count)
        {
            av_log(NULL, AV_LOG_WARNING, "audio buffer is probably too small\n");
            if (swr_init(is->swr_ctx) < 0)
                swr_free(&is->swr_ctx);
        }
        is->audio_buf = is->audio_buf1;
        resampled_data_size = len2 * is->audio_tgt.channels * av_get_bytes_per_sample(is->audio_tgt.fmt);
    }
    else
    {
        is->audio_buf = af->frame->data[0];
        resampled_data_size = data_size;
    }

    audio_clock0 = is->audio_clock;
    /* update the audio clock with the pts */
    if (!isnan(af->pts))
        is->audio_clock = af->pts + (double)af->frame->nb_samples / af->frame->sample_rate;
    else
        is->audio_clock = NAN;
    is->audio_clock_serial = af->serial;
    // #ifdef DEBUG
    //     {
    //         static double last_clock;
    //         printf("audio: delay=%0.3f clock=%0.3f clock0=%0.3f\n",
    //                is->audio_clock - last_clock,
    //                is->audio_clock, audio_clock0);
    //         last_clock = is->audio_clock;
    //     }
    // #endif
    return resampled_data_size;
}

/* prepare a new audio buffer */
static void sdl_audio_callback(void *opaque, Uint8 *stream, int len)
{
    VideoState *is = opaque;
    int audio_size, len1;

    audio_callback_time = av_gettime_relative();

    while (len > 0)
    {
        if (is->audio_buf_index >= is->audio_buf_size)
        {
            audio_size = audio_decode_frame(is);
            if (audio_size < 0)
            {
                /* if error, just output silence */
                is->audio_buf = NULL;
                is->audio_buf_size = SDL_AUDIO_MIN_BUFFER_SIZE / is->audio_tgt.frame_size * is->audio_tgt.frame_size;
            }
            else
            {
                if (is->show_mode != SHOW_MODE_VIDEO)
                    update_sample_display(is, (int16_t *)is->audio_buf, audio_size);
                is->audio_buf_size = audio_size;
            }
            is->audio_buf_index = 0;
        }
        len1 = is->audio_buf_size - is->audio_buf_index;
        if (len1 > len)
            len1 = len;
        if (!is->muted && is->audio_buf && is->audio_volume == SDL_MIX_MAXVOLUME)
            memcpy(stream, (uint8_t *)is->audio_buf + is->audio_buf_index, len1);
        else
        {
            memset(stream, 0, len1);
            if (!is->muted && is->audio_buf)
                SDL_MixAudioFormat(stream, (uint8_t *)is->audio_buf + is->audio_buf_index, AUDIO_S16SYS, len1, is->audio_volume);
        }
        len -= len1;
        stream += len1;
        is->audio_buf_index += len1;
    }
    is->audio_write_buf_size = is->audio_buf_size - is->audio_buf_index;
    /* Let's assume the audio driver that is used by SDL has two periods. */
    if (!isnan(is->audio_clock))
    {
        set_clock_at(&is->audclk, is->audio_clock - (double)(2 * is->audio_hw_buf_size + is->audio_write_buf_size) / is->audio_tgt.bytes_per_sec, is->audio_clock_serial, audio_callback_time / 1000000.0);
        sync_clock_to_slave(&is->extclk, &is->audclk);
    }
}

int av_log2(unsigned v)
{
    return ff_log2(v);
}

int64_t av_get_default_channel_layout(int nb_channels) {
    int i;
    for (i = 0; i < FF_ARRAY_ELEMS(channel_layout_map); i++)
        if (nb_channels == channel_layout_map[i].nb_channels)
            return channel_layout_map[i].layout;
    return 0;
}

int av_samples_get_buffer_size(int *linesize, int nb_channels, int nb_samples,
                               enum AVSampleFormat sample_fmt, int align)
{
    int line_size;
    int sample_size = av_get_bytes_per_sample(sample_fmt);
    int planar      = av_sample_fmt_is_planar(sample_fmt);

    /* validate parameter ranges */
    if (!sample_size || nb_samples <= 0 || nb_channels <= 0)
        return AVERROR(EINVAL);

    /* auto-select alignment if not specified */
    if (!align) {
        if (nb_samples > INT_MAX - 31)
            return AVERROR(EINVAL);
        align = 1;
        nb_samples = FFALIGN(nb_samples, 32);
    }

    /* check for integer overflow */
    if (nb_channels > INT_MAX / align ||
        (int64_t)nb_channels * nb_samples > (INT_MAX - (align * nb_channels)) / sample_size)
        return AVERROR(EINVAL);

    line_size = planar ? FFALIGN(nb_samples * sample_size,               align) :
                         FFALIGN(nb_samples * sample_size * nb_channels, align);
    if (linesize)
        *linesize = line_size;

    return planar ? line_size * nb_channels : line_size;
}

static int audio_open(void *opaque, int64_t wanted_channel_layout, int wanted_nb_channels, int wanted_sample_rate, struct AudioParams *audio_hw_params)
{
    SDL_AudioSpec wanted_spec, spec;
    const char *env;
    static const int next_nb_channels[] = {0, 0, 1, 6, 2, 6, 4, 6};
    static const int next_sample_rates[] = {0, 44100, 48000, 96000, 192000};
    int next_sample_rate_idx = FF_ARRAY_ELEMS(next_sample_rates) - 1;

    env = SDL_getenv("SDL_AUDIO_CHANNELS");
    if (env)
    {
        wanted_nb_channels = atoi(env);
        wanted_channel_layout = av_get_default_channel_layout(wanted_nb_channels);
    }
    if (!wanted_channel_layout || wanted_nb_channels != av_get_channel_layout_nb_channels(wanted_channel_layout))
    {
        wanted_channel_layout = av_get_default_channel_layout(wanted_nb_channels);
        wanted_channel_layout &= ~AV_CH_LAYOUT_STEREO_DOWNMIX;
    }
    wanted_nb_channels = av_get_channel_layout_nb_channels(wanted_channel_layout);
    wanted_spec.channels = wanted_nb_channels;
    wanted_spec.freq = wanted_sample_rate;
    if (wanted_spec.freq <= 0 || wanted_spec.channels <= 0)
    {
        av_log(NULL, AV_LOG_ERROR, "Invalid sample rate or channel count!\n");
        return -1;
    }
    while (next_sample_rate_idx && next_sample_rates[next_sample_rate_idx] >= wanted_spec.freq)
        next_sample_rate_idx--;
    wanted_spec.format = AUDIO_S16SYS;
    wanted_spec.silence = 0;
    wanted_spec.samples = FFMAX(SDL_AUDIO_MIN_BUFFER_SIZE, 2 << av_log2(wanted_spec.freq / SDL_AUDIO_MAX_CALLBACKS_PER_SEC));
    wanted_spec.callback = sdl_audio_callback;
    wanted_spec.userdata = opaque;
    while (!(audio_dev = SDL_OpenAudioDevice(NULL, 0, &wanted_spec, &spec, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_CHANNELS_CHANGE)))
    {
        av_log(NULL, AV_LOG_WARNING, "SDL_OpenAudio (%d channels, %d Hz): %s\n",
               wanted_spec.channels, wanted_spec.freq, SDL_GetError());
        wanted_spec.channels = next_nb_channels[FFMIN(7, wanted_spec.channels)];
        if (!wanted_spec.channels)
        {
            wanted_spec.freq = next_sample_rates[next_sample_rate_idx--];
            wanted_spec.channels = wanted_nb_channels;
            if (!wanted_spec.freq)
            {
                av_log(NULL, AV_LOG_ERROR,
                       "No more combinations to try, audio open failed\n");
                return -1;
            }
        }
        wanted_channel_layout = av_get_default_channel_layout(wanted_spec.channels);
    }
    if (spec.format != AUDIO_S16SYS)
    {
        av_log(NULL, AV_LOG_ERROR,
               "SDL advised audio format %d is not supported!\n", spec.format);
        return -1;
    }
    if (spec.channels != wanted_spec.channels)
    {
        wanted_channel_layout = av_get_default_channel_layout(spec.channels);
        if (!wanted_channel_layout)
        {
            av_log(NULL, AV_LOG_ERROR,
                   "SDL advised channel count %d is not supported!\n", spec.channels);
            return -1;
        }
    }

    audio_hw_params->fmt = AV_SAMPLE_FMT_S16;
    audio_hw_params->freq = spec.freq;
    audio_hw_params->channel_layout = wanted_channel_layout;
    audio_hw_params->channels = spec.channels;
    audio_hw_params->frame_size = av_samples_get_buffer_size(NULL, audio_hw_params->channels, 1, audio_hw_params->fmt, 1);
    audio_hw_params->bytes_per_sec = av_samples_get_buffer_size(NULL, audio_hw_params->channels, audio_hw_params->freq, audio_hw_params->fmt, 1);
    if (audio_hw_params->bytes_per_sec <= 0 || audio_hw_params->frame_size <= 0)
    {
        av_log(NULL, AV_LOG_ERROR, "av_samples_get_buffer_size failed\n");
        return -1;
    }
    return spec.size;
}

const AVCodec *av_codec_iterate(void **opaque)
{
    uintptr_t i = (uintptr_t)*opaque;
    const AVCodec *c = codec_list[i];

    ff_thread_once(&av_codec_static_init, av_codec_init_static);

    if (c)
        *opaque = (void*)(i + 1);

    return c;
}

static AVOnce av_codec_static_init = AV_ONCE_INIT;

static void av_codec_init_static(void)
{
    for (int i = 0; codec_list[i]; i++) {
        if (codec_list[i]->init_static_data)
            codec_list[i]->init_static_data((AVCodec*)codec_list[i]);
    }
}
static AVCodec *find_codec_by_name(const char *name, int (*x)(const AVCodec *))
{
    void *i = 0;
    const AVCodec *p;

    if (!name)
        return NULL;

    while ((p = av_codec_iterate(&i))) {
        if (!x(p))
            continue;
        if (strcmp(name, p->name) == 0)
            return (AVCodec*)p;
    }

    return NULL;
}

AVCodec *avcodec_find_encoder_by_name(const char *name)
{
    return find_codec_by_name(name, av_codec_is_encoder);
}

AVCodec *avcodec_find_decoder_by_name(const char *name)
{
    return find_codec_by_name(name, av_codec_is_decoder);
}

const AVClass *avcodec_get_class(void)
{
    return &av_codec_context_class;
}

static AVCodec *find_codec(enum AVCodecID id, int (*x)(const AVCodec *))
{
    const AVCodec *p, *experimental = NULL;
    void *i = 0;

    id = remap_deprecated_codec_id(id);

    while ((p = av_codec_iterate(&i))) {
        if (!x(p))
            continue;
        if (p->id == id) {
            if (p->capabilities & AV_CODEC_CAP_EXPERIMENTAL && !experimental) {
                experimental = p;
            } else
                return (AVCodec*)p;
        }
    }

    return (AVCodec*)experimental;
}

static enum AVCodecID remap_deprecated_codec_id(enum AVCodecID id)
{
    switch(id){
        default                                         : return id;
    }
}

AVCodec *avcodec_find_encoder(enum AVCodecID id)
{
    return find_codec(id, av_codec_is_encoder);
}

AVCodec *avcodec_find_decoder(enum AVCodecID id)
{
    return find_codec(id, av_codec_is_decoder);
}

AVDictionary *filter_codec_opts(AVDictionary *opts, enum AVCodecID codec_id,
                                AVFormatContext *s, AVStream *st, AVCodec *codec)
{
    AVDictionary *ret = NULL;
    AVDictionaryEntry *t = NULL;
    int flags = s->oformat ? AV_OPT_FLAG_ENCODING_PARAM
                           : AV_OPT_FLAG_DECODING_PARAM;
    char prefix = 0;
    const AVClass *cc = avcodec_get_class();

    if (!codec)
        codec = s->oformat ? avcodec_find_encoder(codec_id)
                           : avcodec_find_decoder(codec_id);

    switch (st->codecpar->codec_type)
    {
    case AVMEDIA_TYPE_VIDEO:
        prefix = 'v';
        flags |= AV_OPT_FLAG_VIDEO_PARAM;
        break;
    case AVMEDIA_TYPE_AUDIO:
        prefix = 'a';
        flags |= AV_OPT_FLAG_AUDIO_PARAM;
        break;
    case AVMEDIA_TYPE_SUBTITLE:
        prefix = 's';
        flags |= AV_OPT_FLAG_SUBTITLE_PARAM;
        break;
    }

    while (t = av_dict_get(opts, "", t, AV_DICT_IGNORE_SUFFIX))
    {
        char *p = strchr(t->key, ':');

        /* check stream specification in opt name */
        if (p)
            switch (check_stream_specifier(s, st, p + 1))
            {
            case 1:
                *p = 0;
                break;
            case 0:
                continue;
            default:
                exit_program(1);
            }

        if (av_opt_find(&cc, t->key, NULL, flags, AV_OPT_SEARCH_FAKE_OBJ) ||
            !codec ||
            (codec->priv_class &&
             av_opt_find(&codec->priv_class, t->key, NULL, flags,
                         AV_OPT_SEARCH_FAKE_OBJ)))
            av_dict_set(&ret, t->key, t->value, 0);
        else if (t->key[0] == prefix &&
                 av_opt_find(&cc, t->key + 1, NULL, flags,
                             AV_OPT_SEARCH_FAKE_OBJ))
            av_dict_set(&ret, t->key + 1, t->value, 0);

        if (p)
            *p = ':';
    }
    return ret;
}

static int init_context_defaults(AVCodecContext *s, const AVCodec *codec)
{
    int flags=0;
    memset(s, 0, sizeof(AVCodecContext));

    s->av_class = &av_codec_context_class;

    s->codec_type = codec ? codec->type : AVMEDIA_TYPE_UNKNOWN;
    if (codec) {
        s->codec = codec;
        s->codec_id = codec->id;
    }

    if(s->codec_type == AVMEDIA_TYPE_AUDIO)
        flags= AV_OPT_FLAG_AUDIO_PARAM;
    else if(s->codec_type == AVMEDIA_TYPE_VIDEO)
        flags= AV_OPT_FLAG_VIDEO_PARAM;
    else if(s->codec_type == AVMEDIA_TYPE_SUBTITLE)
        flags= AV_OPT_FLAG_SUBTITLE_PARAM;
    av_opt_set_defaults2(s, flags, flags);

    s->time_base           = (AVRational){0,1};
    s->framerate           = (AVRational){ 0, 1 };
    s->pkt_timebase        = (AVRational){ 0, 1 };
    s->get_buffer2         = avcodec_default_get_buffer2;
    s->get_format          = avcodec_default_get_format;
    s->execute             = avcodec_default_execute;
    s->execute2            = avcodec_default_execute2;
    s->sample_aspect_ratio = (AVRational){0,1};
    s->pix_fmt             = AV_PIX_FMT_NONE;
    s->sw_pix_fmt          = AV_PIX_FMT_NONE;
    s->sample_fmt          = AV_SAMPLE_FMT_NONE;

    s->reordered_opaque    = AV_NOPTS_VALUE;
    if(codec && codec->priv_data_size){
        if(!s->priv_data){
            s->priv_data= av_mallocz(codec->priv_data_size);
            if (!s->priv_data) {
                return AVERROR(ENOMEM);
            }
        }
        if(codec->priv_class){
            *(const AVClass**)s->priv_data = codec->priv_class;
            av_opt_set_defaults(s->priv_data);
        }
    }
    if (codec && codec->defaults) {
        int ret;
        const AVCodecDefault *d = codec->defaults;
        while (d->key) {
            ret = av_opt_set(s, d->key, d->value, 0);
            av_assert0(ret >= 0);
            d++;
        }
    }
    return 0;
}

AVCodecContext *avcodec_alloc_context3(const AVCodec *codec)
{
    AVCodecContext *avctx= av_malloc(sizeof(AVCodecContext));

    if (!avctx)
        return NULL;

    if (init_context_defaults(avctx, codec) < 0) {
        av_free(avctx);
        return NULL;
    }

    return avctx;
}

int avcodec_parameters_to_context(AVCodecContext *codec,
                                  const AVCodecParameters *par)
{
    codec->codec_type = par->codec_type;
    codec->codec_id   = par->codec_id;
    codec->codec_tag  = par->codec_tag;

    codec->bit_rate              = par->bit_rate;
    codec->bits_per_coded_sample = par->bits_per_coded_sample;
    codec->bits_per_raw_sample   = par->bits_per_raw_sample;
    codec->profile               = par->profile;
    codec->level                 = par->level;

    switch (par->codec_type) {
    case AVMEDIA_TYPE_VIDEO:
        codec->pix_fmt                = par->format;
        codec->width                  = par->width;
        codec->height                 = par->height;
        codec->field_order            = par->field_order;
        codec->color_range            = par->color_range;
        codec->color_primaries        = par->color_primaries;
        codec->color_trc              = par->color_trc;
        codec->colorspace             = par->color_space;
        codec->chroma_sample_location = par->chroma_location;
        codec->sample_aspect_ratio    = par->sample_aspect_ratio;
        codec->has_b_frames           = par->video_delay;
        break;
    case AVMEDIA_TYPE_AUDIO:
        codec->sample_fmt       = par->format;
        codec->channel_layout   = par->channel_layout;
        codec->channels         = par->channels;
        codec->sample_rate      = par->sample_rate;
        codec->block_align      = par->block_align;
        codec->frame_size       = par->frame_size;
        codec->delay            =
        codec->initial_padding  = par->initial_padding;
        codec->trailing_padding = par->trailing_padding;
        codec->seek_preroll     = par->seek_preroll;
        break;
    case AVMEDIA_TYPE_SUBTITLE:
        codec->width  = par->width;
        codec->height = par->height;
        break;
    }

    if (par->extradata) {
        av_freep(&codec->extradata);
        codec->extradata = av_mallocz(par->extradata_size + AV_INPUT_BUFFER_PADDING_SIZE);
        if (!codec->extradata)
            return AVERROR(ENOMEM);
        memcpy(codec->extradata, par->extradata, par->extradata_size);
        codec->extradata_size = par->extradata_size;
    }

    return 0;
}

static int descriptor_compare(const void *key, const void *member)
{
    enum AVCodecID id = *(const enum AVCodecID *) key;
    const AVCodecDescriptor *desc = member;

    return id - desc->id;
}


const AVCodecDescriptor *avcodec_descriptor_get(enum AVCodecID id)
{
    return bsearch(&id, codec_descriptors, FF_ARRAY_ELEMS(codec_descriptors),
                   sizeof(codec_descriptors[0]), descriptor_compare);
}

const char *avcodec_get_name(enum AVCodecID id)
{
    const AVCodecDescriptor *cd;
    const AVCodec *codec;

    if (id == AV_CODEC_ID_NONE)
        return "none";
    cd = avcodec_descriptor_get(id);
    if (cd)
        return cd->name;
    av_log(NULL, AV_LOG_WARNING, "Codec 0x%x is not in the full list.\n", id);
    codec = avcodec_find_decoder(id);
    if (codec)
        return codec->name;
    codec = avcodec_find_encoder(id);
    if (codec)
        return codec->name;
    return "unknown_codec";
}

int av_dict_set_int(AVDictionary **pm, const char *key, int64_t value,
                int flags)
{
    char valuestr[22];
    snprintf(valuestr, sizeof(valuestr), "%"PRId64, value);
    flags &= ~AV_DICT_DONT_STRDUP_VAL;
    return av_dict_set(pm, key, valuestr, flags);
}

int attribute_align_arg avcodec_open2(AVCodecContext *avctx, const AVCodec *codec, AVDictionary **options)
{
    int ret = 0;
    int codec_init_ok = 0;
    AVDictionary *tmp = NULL;
    const AVPixFmtDescriptor *pixdesc;
    AVCodecInternal *avci;

    if (avcodec_is_open(avctx))
        return 0;

    if (!codec && !avctx->codec) {
        av_log(avctx, AV_LOG_ERROR, "No codec provided to avcodec_open2()\n");
        return AVERROR(EINVAL);
    }
    if (codec && avctx->codec && codec != avctx->codec) {
        av_log(avctx, AV_LOG_ERROR, "This AVCodecContext was allocated for %s, "
                                    "but %s passed to avcodec_open2()\n", avctx->codec->name, codec->name);
        return AVERROR(EINVAL);
    }
    if (!codec)
        codec = avctx->codec;

    if (avctx->extradata_size < 0 || avctx->extradata_size >= FF_MAX_EXTRADATA_SIZE)
        return AVERROR(EINVAL);

    if (options)
        av_dict_copy(&tmp, *options, 0);

    ff_lock_avcodec(avctx, codec);

    avci = av_mallocz(sizeof(*avci));
    if (!avci) {
        ret = AVERROR(ENOMEM);
        goto end;
    }
    avctx->internal = avci;

    avci->to_free = av_frame_alloc();
    avci->compat_decode_frame = av_frame_alloc();
    avci->compat_encode_packet = av_packet_alloc();
    avci->buffer_frame = av_frame_alloc();
    avci->buffer_pkt = av_packet_alloc();
    avci->es.in_frame = av_frame_alloc();
    avci->ds.in_pkt = av_packet_alloc();
    avci->last_pkt_props = av_packet_alloc();
    if (!avci->compat_decode_frame || !avci->compat_encode_packet ||
        !avci->buffer_frame || !avci->buffer_pkt          ||
        !avci->es.in_frame  || !avci->ds.in_pkt           ||
        !avci->to_free      || !avci->last_pkt_props) {
        ret = AVERROR(ENOMEM);
        goto free_and_end;
    }

    avci->skip_samples_multiplier = 1;

    if (codec->priv_data_size > 0) {
        if (!avctx->priv_data) {
            avctx->priv_data = av_mallocz(codec->priv_data_size);
            if (!avctx->priv_data) {
                ret = AVERROR(ENOMEM);
                goto free_and_end;
            }
            if (codec->priv_class) {
                *(const AVClass **)avctx->priv_data = codec->priv_class;
                av_opt_set_defaults(avctx->priv_data);
            }
        }
        if (codec->priv_class && (ret = av_opt_set_dict(avctx->priv_data, &tmp)) < 0)
            goto free_and_end;
    } else {
        avctx->priv_data = NULL;
    }
    if ((ret = av_opt_set_dict(avctx, &tmp)) < 0)
        goto free_and_end;

    if (avctx->codec_whitelist && av_match_list(codec->name, avctx->codec_whitelist, ',') <= 0) {
        av_log(avctx, AV_LOG_ERROR, "Codec (%s) not on whitelist \'%s\'\n", codec->name, avctx->codec_whitelist);
        ret = AVERROR(EINVAL);
        goto free_and_end;
    }

    // only call ff_set_dimensions() for non H.264/VP6F/DXV codecs so as not to overwrite previously setup dimensions
    if (!(avctx->coded_width && avctx->coded_height && avctx->width && avctx->height &&
          (avctx->codec_id == AV_CODEC_ID_H264 || avctx->codec_id == AV_CODEC_ID_VP6F || avctx->codec_id == AV_CODEC_ID_DXV))) {
        if (avctx->coded_width && avctx->coded_height)
            ret = ff_set_dimensions(avctx, avctx->coded_width, avctx->coded_height);
        else if (avctx->width && avctx->height)
            ret = ff_set_dimensions(avctx, avctx->width, avctx->height);
        if (ret < 0)
            goto free_and_end;
    }

    if ((avctx->coded_width || avctx->coded_height || avctx->width || avctx->height)
        && (  av_image_check_size2(avctx->coded_width, avctx->coded_height, avctx->max_pixels, AV_PIX_FMT_NONE, 0, avctx) < 0
           || av_image_check_size2(avctx->width,       avctx->height,       avctx->max_pixels, AV_PIX_FMT_NONE, 0, avctx) < 0)) {
        av_log(avctx, AV_LOG_WARNING, "Ignoring invalid width/height values\n");
        ff_set_dimensions(avctx, 0, 0);
    }

    if (avctx->width > 0 && avctx->height > 0) {
        if (av_image_check_sar(avctx->width, avctx->height,
                               avctx->sample_aspect_ratio) < 0) {
            av_log(avctx, AV_LOG_WARNING, "ignoring invalid SAR: %u/%u\n",
                   avctx->sample_aspect_ratio.num,
                   avctx->sample_aspect_ratio.den);
            avctx->sample_aspect_ratio = (AVRational){ 0, 1 };
        }
    }

    /* if the decoder init function was already called previously,
     * free the already allocated subtitle_header before overwriting it */
    if (av_codec_is_decoder(codec))
        av_freep(&avctx->subtitle_header);

    if (avctx->channels > FF_SANE_NB_CHANNELS || avctx->channels < 0) {
        av_log(avctx, AV_LOG_ERROR, "Too many or invalid channels: %d\n", avctx->channels);
        ret = AVERROR(EINVAL);
        goto free_and_end;
    }
    if (avctx->sample_rate < 0) {
        av_log(avctx, AV_LOG_ERROR, "Invalid sample rate: %d\n", avctx->sample_rate);
        ret = AVERROR(EINVAL);
        goto free_and_end;
    }
    if (avctx->block_align < 0) {
        av_log(avctx, AV_LOG_ERROR, "Invalid block align: %d\n", avctx->block_align);
        ret = AVERROR(EINVAL);
        goto free_and_end;
    }

    avctx->codec = codec;
    if ((avctx->codec_type == AVMEDIA_TYPE_UNKNOWN || avctx->codec_type == codec->type) &&
        avctx->codec_id == AV_CODEC_ID_NONE) {
        avctx->codec_type = codec->type;
        avctx->codec_id   = codec->id;
    }
    if (avctx->codec_id != codec->id || (avctx->codec_type != codec->type
                                         && avctx->codec_type != AVMEDIA_TYPE_ATTACHMENT)) {
        av_log(avctx, AV_LOG_ERROR, "Codec type or id mismatches\n");
        ret = AVERROR(EINVAL);
        goto free_and_end;
    }
    avctx->frame_number = 0;
    avctx->codec_descriptor = avcodec_descriptor_get(avctx->codec_id);

    if ((avctx->codec->capabilities & AV_CODEC_CAP_EXPERIMENTAL) &&
        avctx->strict_std_compliance > FF_COMPLIANCE_EXPERIMENTAL) {
        const char *codec_string = av_codec_is_encoder(codec) ? "encoder" : "decoder";
        const AVCodec *codec2;
        av_log(avctx, AV_LOG_ERROR,
               "The %s '%s' is experimental but experimental codecs are not enabled, "
               "add '-strict %d' if you want to use it.\n",
               codec_string, codec->name, FF_COMPLIANCE_EXPERIMENTAL);
        codec2 = av_codec_is_encoder(codec) ? avcodec_find_encoder(codec->id) : avcodec_find_decoder(codec->id);
        if (!(codec2->capabilities & AV_CODEC_CAP_EXPERIMENTAL))
            av_log(avctx, AV_LOG_ERROR, "Alternatively use the non experimental %s '%s'.\n",
                codec_string, codec2->name);
        ret = AVERROR_EXPERIMENTAL;
        goto free_and_end;
    }

    if (avctx->codec_type == AVMEDIA_TYPE_AUDIO &&
        (!avctx->time_base.num || !avctx->time_base.den)) {
        avctx->time_base.num = 1;
        avctx->time_base.den = avctx->sample_rate;
    }

    if (!HAVE_THREADS)
        av_log(avctx, AV_LOG_WARNING, "Warning: not compiled with thread support, using thread emulation\n");

    if (CONFIG_FRAME_THREAD_ENCODER && av_codec_is_encoder(avctx->codec)) {
        ff_unlock_avcodec(codec); //we will instantiate a few encoders thus kick the counter to prevent false detection of a problem
        ret = ff_frame_thread_encoder_init(avctx, options ? *options : NULL);
        ff_lock_avcodec(avctx, codec);
        if (ret < 0)
            goto free_and_end;
    }

    if (av_codec_is_decoder(avctx->codec)) {
        ret = ff_decode_bsfs_init(avctx);
        if (ret < 0)
            goto free_and_end;
    }

    if (HAVE_THREADS
        && !(avci->frame_thread_encoder && (avctx->active_thread_type&FF_THREAD_FRAME))) {
        ret = ff_thread_init(avctx);
        if (ret < 0) {
            goto free_and_end;
        }
    }
    if (!HAVE_THREADS && !(codec->capabilities & AV_CODEC_CAP_AUTO_THREADS))
        avctx->thread_count = 1;

    if (avctx->codec->max_lowres < avctx->lowres || avctx->lowres < 0) {
        av_log(avctx, AV_LOG_WARNING, "The maximum value for lowres supported by the decoder is %d\n",
               avctx->codec->max_lowres);
        avctx->lowres = avctx->codec->max_lowres;
    }

    if (av_codec_is_encoder(avctx->codec)) {
        int i;
#if FF_API_CODED_FRAME
FF_DISABLE_DEPRECATION_WARNINGS
        avctx->coded_frame = av_frame_alloc();
        if (!avctx->coded_frame) {
            ret = AVERROR(ENOMEM);
            goto free_and_end;
        }
FF_ENABLE_DEPRECATION_WARNINGS
#endif

        if (avctx->time_base.num <= 0 || avctx->time_base.den <= 0) {
            av_log(avctx, AV_LOG_ERROR, "The encoder timebase is not set.\n");
            ret = AVERROR(EINVAL);
            goto free_and_end;
        }

        if (avctx->codec->sample_fmts) {
            for (i = 0; avctx->codec->sample_fmts[i] != AV_SAMPLE_FMT_NONE; i++) {
                if (avctx->sample_fmt == avctx->codec->sample_fmts[i])
                    break;
                if (avctx->channels == 1 &&
                    av_get_planar_sample_fmt(avctx->sample_fmt) ==
                    av_get_planar_sample_fmt(avctx->codec->sample_fmts[i])) {
                    avctx->sample_fmt = avctx->codec->sample_fmts[i];
                    break;
                }
            }
            if (avctx->codec->sample_fmts[i] == AV_SAMPLE_FMT_NONE) {
                char buf[128];
                snprintf(buf, sizeof(buf), "%d", avctx->sample_fmt);
                av_log(avctx, AV_LOG_ERROR, "Specified sample format %s is invalid or not supported\n",
                       (char *)av_x_if_null(av_get_sample_fmt_name(avctx->sample_fmt), buf));
                ret = AVERROR(EINVAL);
                goto free_and_end;
            }
        }
        if (avctx->codec->pix_fmts) {
            for (i = 0; avctx->codec->pix_fmts[i] != AV_PIX_FMT_NONE; i++)
                if (avctx->pix_fmt == avctx->codec->pix_fmts[i])
                    break;
            if (avctx->codec->pix_fmts[i] == AV_PIX_FMT_NONE
                && !((avctx->codec_id == AV_CODEC_ID_MJPEG || avctx->codec_id == AV_CODEC_ID_LJPEG)
                     && avctx->strict_std_compliance <= FF_COMPLIANCE_UNOFFICIAL)) {
                char buf[128];
                snprintf(buf, sizeof(buf), "%d", avctx->pix_fmt);
                av_log(avctx, AV_LOG_ERROR, "Specified pixel format %s is invalid or not supported\n",
                       (char *)av_x_if_null(av_get_pix_fmt_name(avctx->pix_fmt), buf));
                ret = AVERROR(EINVAL);
                goto free_and_end;
            }
            if (avctx->codec->pix_fmts[i] == AV_PIX_FMT_YUVJ420P ||
                avctx->codec->pix_fmts[i] == AV_PIX_FMT_YUVJ411P ||
                avctx->codec->pix_fmts[i] == AV_PIX_FMT_YUVJ422P ||
                avctx->codec->pix_fmts[i] == AV_PIX_FMT_YUVJ440P ||
                avctx->codec->pix_fmts[i] == AV_PIX_FMT_YUVJ444P)
                avctx->color_range = AVCOL_RANGE_JPEG;
        }
        if (avctx->codec->supported_samplerates) {
            for (i = 0; avctx->codec->supported_samplerates[i] != 0; i++)
                if (avctx->sample_rate == avctx->codec->supported_samplerates[i])
                    break;
            if (avctx->codec->supported_samplerates[i] == 0) {
                av_log(avctx, AV_LOG_ERROR, "Specified sample rate %d is not supported\n",
                       avctx->sample_rate);
                ret = AVERROR(EINVAL);
                goto free_and_end;
            }
        }
        if (avctx->sample_rate < 0) {
            av_log(avctx, AV_LOG_ERROR, "Specified sample rate %d is not supported\n",
                    avctx->sample_rate);
            ret = AVERROR(EINVAL);
            goto free_and_end;
        }
        if (avctx->codec->channel_layouts) {
            if (!avctx->channel_layout) {
                av_log(avctx, AV_LOG_WARNING, "Channel layout not specified\n");
            } else {
                for (i = 0; avctx->codec->channel_layouts[i] != 0; i++)
                    if (avctx->channel_layout == avctx->codec->channel_layouts[i])
                        break;
                if (avctx->codec->channel_layouts[i] == 0) {
                    char buf[512];
                    av_get_channel_layout_string(buf, sizeof(buf), -1, avctx->channel_layout);
                    av_log(avctx, AV_LOG_ERROR, "Specified channel layout '%s' is not supported\n", buf);
                    ret = AVERROR(EINVAL);
                    goto free_and_end;
                }
            }
        }
        if (avctx->channel_layout && avctx->channels) {
            int channels = av_get_channel_layout_nb_channels(avctx->channel_layout);
            if (channels != avctx->channels) {
                char buf[512];
                av_get_channel_layout_string(buf, sizeof(buf), -1, avctx->channel_layout);
                av_log(avctx, AV_LOG_ERROR,
                       "Channel layout '%s' with %d channels does not match number of specified channels %d\n",
                       buf, channels, avctx->channels);
                ret = AVERROR(EINVAL);
                goto free_and_end;
            }
        } else if (avctx->channel_layout) {
            avctx->channels = av_get_channel_layout_nb_channels(avctx->channel_layout);
        }
        if (avctx->channels < 0) {
            av_log(avctx, AV_LOG_ERROR, "Specified number of channels %d is not supported\n",
                    avctx->channels);
            ret = AVERROR(EINVAL);
            goto free_and_end;
        }
        if(avctx->codec_type == AVMEDIA_TYPE_VIDEO) {
            pixdesc = av_pix_fmt_desc_get(avctx->pix_fmt);
            if (    avctx->bits_per_raw_sample < 0
                || (avctx->bits_per_raw_sample > 8 && pixdesc->comp[0].depth <= 8)) {
                av_log(avctx, AV_LOG_WARNING, "Specified bit depth %d not possible with the specified pixel formats depth %d\n",
                    avctx->bits_per_raw_sample, pixdesc->comp[0].depth);
                avctx->bits_per_raw_sample = pixdesc->comp[0].depth;
            }
            if (avctx->width <= 0 || avctx->height <= 0) {
                av_log(avctx, AV_LOG_ERROR, "dimensions not set\n");
                ret = AVERROR(EINVAL);
                goto free_and_end;
            }
        }
        if (   (avctx->codec_type == AVMEDIA_TYPE_VIDEO || avctx->codec_type == AVMEDIA_TYPE_AUDIO)
            && avctx->bit_rate>0 && avctx->bit_rate<1000) {
            av_log(avctx, AV_LOG_WARNING, "Bitrate %"PRId64" is extremely low, maybe you mean %"PRId64"k\n", avctx->bit_rate, avctx->bit_rate);
        }

        if (!avctx->rc_initial_buffer_occupancy)
            avctx->rc_initial_buffer_occupancy = avctx->rc_buffer_size * 3LL / 4;

        if (avctx->ticks_per_frame && avctx->time_base.num &&
            avctx->ticks_per_frame > INT_MAX / avctx->time_base.num) {
            av_log(avctx, AV_LOG_ERROR,
                   "ticks_per_frame %d too large for the timebase %d/%d.",
                   avctx->ticks_per_frame,
                   avctx->time_base.num,
                   avctx->time_base.den);
            goto free_and_end;
        }

        if (avctx->hw_frames_ctx) {
            AVHWFramesContext *frames_ctx = (AVHWFramesContext*)avctx->hw_frames_ctx->data;
            if (frames_ctx->format != avctx->pix_fmt) {
                av_log(avctx, AV_LOG_ERROR,
                       "Mismatching AVCodecContext.pix_fmt and AVHWFramesContext.format\n");
                ret = AVERROR(EINVAL);
                goto free_and_end;
            }
            if (avctx->sw_pix_fmt != AV_PIX_FMT_NONE &&
                avctx->sw_pix_fmt != frames_ctx->sw_format) {
                av_log(avctx, AV_LOG_ERROR,
                       "Mismatching AVCodecContext.sw_pix_fmt (%s) "
                       "and AVHWFramesContext.sw_format (%s)\n",
                       av_get_pix_fmt_name(avctx->sw_pix_fmt),
                       av_get_pix_fmt_name(frames_ctx->sw_format));
                ret = AVERROR(EINVAL);
                goto free_and_end;
            }
            avctx->sw_pix_fmt = frames_ctx->sw_format;
        }
    }

    avctx->pts_correction_num_faulty_pts =
    avctx->pts_correction_num_faulty_dts = 0;
    avctx->pts_correction_last_pts =
    avctx->pts_correction_last_dts = INT64_MIN;

    if (   !CONFIG_GRAY && avctx->flags & AV_CODEC_FLAG_GRAY
        && avctx->codec_descriptor->type == AVMEDIA_TYPE_VIDEO)
        av_log(avctx, AV_LOG_WARNING,
               "gray decoding requested but not enabled at configuration time\n");
    if (avctx->flags2 & AV_CODEC_FLAG2_EXPORT_MVS) {
        avctx->export_side_data |= AV_CODEC_EXPORT_DATA_MVS;
    }

    if (   avctx->codec->init && (!(avctx->active_thread_type&FF_THREAD_FRAME)
        || avci->frame_thread_encoder)) {
        ret = avctx->codec->init(avctx);
        if (ret < 0) {
            codec_init_ok = -1;
            goto free_and_end;
        }
        codec_init_ok = 1;
    }

    ret=0;

    if (av_codec_is_decoder(avctx->codec)) {
        if (!avctx->bit_rate)
            avctx->bit_rate = get_bit_rate(avctx);
        /* validate channel layout from the decoder */
        if (avctx->channel_layout) {
            int channels = av_get_channel_layout_nb_channels(avctx->channel_layout);
            if (!avctx->channels)
                avctx->channels = channels;
            else if (channels != avctx->channels) {
                char buf[512];
                av_get_channel_layout_string(buf, sizeof(buf), -1, avctx->channel_layout);
                av_log(avctx, AV_LOG_WARNING,
                       "Channel layout '%s' with %d channels does not match specified number of channels %d: "
                       "ignoring specified channel layout\n",
                       buf, channels, avctx->channels);
                avctx->channel_layout = 0;
            }
        }
        if (avctx->channels && avctx->channels < 0 ||
            avctx->channels > FF_SANE_NB_CHANNELS) {
            ret = AVERROR(EINVAL);
            goto free_and_end;
        }
        if (avctx->bits_per_coded_sample < 0) {
            ret = AVERROR(EINVAL);
            goto free_and_end;
        }
        if (avctx->sub_charenc) {
            if (avctx->codec_type != AVMEDIA_TYPE_SUBTITLE) {
                av_log(avctx, AV_LOG_ERROR, "Character encoding is only "
                       "supported with subtitles codecs\n");
                ret = AVERROR(EINVAL);
                goto free_and_end;
            } else if (avctx->codec_descriptor->props & AV_CODEC_PROP_BITMAP_SUB) {
                av_log(avctx, AV_LOG_WARNING, "Codec '%s' is bitmap-based, "
                       "subtitles character encoding will be ignored\n",
                       avctx->codec_descriptor->name);
                avctx->sub_charenc_mode = FF_SUB_CHARENC_MODE_DO_NOTHING;
            } else {
                /* input character encoding is set for a text based subtitle
                 * codec at this point */
                if (avctx->sub_charenc_mode == FF_SUB_CHARENC_MODE_AUTOMATIC)
                    avctx->sub_charenc_mode = FF_SUB_CHARENC_MODE_PRE_DECODER;

                if (avctx->sub_charenc_mode == FF_SUB_CHARENC_MODE_PRE_DECODER) {
#if CONFIG_ICONV
                    iconv_t cd = iconv_open("UTF-8", avctx->sub_charenc);
                    if (cd == (iconv_t)-1) {
                        ret = AVERROR(errno);
                        av_log(avctx, AV_LOG_ERROR, "Unable to open iconv context "
                               "with input character encoding \"%s\"\n", avctx->sub_charenc);
                        goto free_and_end;
                    }
                    iconv_close(cd);
#else
                    av_log(avctx, AV_LOG_ERROR, "Character encoding subtitles "
                           "conversion needs a libavcodec built with iconv support "
                           "for this codec\n");
                    ret = AVERROR(ENOSYS);
                    goto free_and_end;
#endif
                }
            }
        }

#if FF_API_AVCTX_TIMEBASE
        if (avctx->framerate.num > 0 && avctx->framerate.den > 0)
            avctx->time_base = av_inv_q(av_mul_q(avctx->framerate, (AVRational){avctx->ticks_per_frame, 1}));
#endif
    }
    if (codec->priv_data_size > 0 && avctx->priv_data && codec->priv_class) {
        av_assert0(*(const AVClass **)avctx->priv_data == codec->priv_class);
    }

end:
    ff_unlock_avcodec(codec);
    if (options) {
        av_dict_free(options);
        *options = tmp;
    }

    return ret;
free_and_end:
    if (avctx->codec && avctx->codec->close &&
        (codec_init_ok > 0 || (codec_init_ok < 0 &&
         avctx->codec->caps_internal & FF_CODEC_CAP_INIT_CLEANUP)))
        avctx->codec->close(avctx);

    if (HAVE_THREADS && avci->thread_ctx)
        ff_thread_free(avctx);

    if (codec->priv_class && avctx->priv_data)
        av_opt_free(avctx->priv_data);
    av_opt_free(avctx);

    if (av_codec_is_encoder(avctx->codec)) {
#if FF_API_CODED_FRAME
FF_DISABLE_DEPRECATION_WARNINGS
        av_frame_free(&avctx->coded_frame);
FF_ENABLE_DEPRECATION_WARNINGS
#endif
        av_freep(&avctx->extradata);
        avctx->extradata_size = 0;
    }

    av_dict_free(&tmp);
    av_freep(&avctx->priv_data);
    av_freep(&avctx->subtitle_header);

    av_frame_free(&avci->to_free);
    av_frame_free(&avci->compat_decode_frame);
    av_frame_free(&avci->buffer_frame);
    av_packet_free(&avci->compat_encode_packet);
    av_packet_free(&avci->buffer_pkt);
    av_packet_free(&avci->last_pkt_props);

    av_packet_free(&avci->ds.in_pkt);
    av_frame_free(&avci->es.in_frame);
    av_bsf_free(&avci->bsf);

    av_buffer_unref(&avci->pool);
    av_freep(&avci);
    avctx->internal = NULL;
    avctx->codec = NULL;
    goto end;
}

static int stream_component_open(VideoState *is, int stream_index)
{
    AVFormatContext *ic = is->ic;
    AVCodecContext *avctx;
    AVCodec *codec;
    const char *forced_codec_name = NULL;
    AVDictionary *opts = NULL;
    AVDictionaryEntry *t = NULL;
    int sample_rate, nb_channels;
    int64_t channel_layout;
    int ret = 0;
    int stream_lowres = lowres;

    if (stream_index < 0 || stream_index >= ic->nb_streams)
        return -1;

    avctx = avcodec_alloc_context3(NULL);
    if (!avctx)
        return AVERROR(ENOMEM);

    ret = avcodec_parameters_to_context(avctx, ic->streams[stream_index]->codecpar);
    if (ret < 0)
        goto fail;
    avctx->pkt_timebase = ic->streams[stream_index]->time_base;

    codec = avcodec_find_decoder(avctx->codec_id);

    switch (avctx->codec_type)
    {
    case AVMEDIA_TYPE_AUDIO:
        is->last_audio_stream = stream_index;
        forced_codec_name = audio_codec_name;
        break;
    case AVMEDIA_TYPE_SUBTITLE:
        is->last_subtitle_stream = stream_index;
        forced_codec_name = subtitle_codec_name;
        break;
    case AVMEDIA_TYPE_VIDEO:
        is->last_video_stream = stream_index;
        forced_codec_name = video_codec_name;
        break;
    }
    if (forced_codec_name)
        codec = avcodec_find_decoder_by_name(forced_codec_name);
    if (!codec)
    {
        if (forced_codec_name)
            av_log(NULL, AV_LOG_WARNING,
                   "No codec could be found with name '%s'\n", forced_codec_name);
        else
            av_log(NULL, AV_LOG_WARNING,
                   "No decoder could be found for codec %s\n", avcodec_get_name(avctx->codec_id));
        ret = AVERROR(EINVAL);
        goto fail;
    }

    avctx->codec_id = codec->id;
    if (stream_lowres > codec->max_lowres)
    {
        av_log(avctx, AV_LOG_WARNING, "The maximum value for lowres supported by the decoder is %d\n",
               codec->max_lowres);
        stream_lowres = codec->max_lowres;
    }
    avctx->lowres = stream_lowres;

    if (fast)
        avctx->flags2 |= AV_CODEC_FLAG2_FAST;

    opts = filter_codec_opts(codec_opts, avctx->codec_id, ic, ic->streams[stream_index], codec);
    if (!av_dict_get(opts, "threads", NULL, 0))
        av_dict_set(&opts, "threads", "auto", 0);
    if (stream_lowres)
        av_dict_set_int(&opts, "lowres", stream_lowres, 0);
    if (avctx->codec_type == AVMEDIA_TYPE_VIDEO || avctx->codec_type == AVMEDIA_TYPE_AUDIO)
        av_dict_set(&opts, "refcounted_frames", "1", 0);
    if ((ret = avcodec_open2(avctx, codec, &opts)) < 0)
    {
        goto fail;
    }
    if ((t = av_dict_get(opts, "", NULL, AV_DICT_IGNORE_SUFFIX)))
    {
        av_log(NULL, AV_LOG_ERROR, "Option %s not found.\n", t->key);
        ret = AVERROR_OPTION_NOT_FOUND;
        goto fail;
    }

    is->eof = 0;
    ic->streams[stream_index]->discard = AVDISCARD_DEFAULT;
    switch (avctx->codec_type)
    {
    case AVMEDIA_TYPE_AUDIO:
        // #if CONFIG_AVFILTER
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
        // #else
        // sample_rate = avctx->sample_rate;
        // nb_channels = avctx->channels;
        // channel_layout = avctx->channel_layout;
        // #endif

        /* prepare audio output */
        if ((ret = audio_open(is, channel_layout, nb_channels, sample_rate, &is->audio_tgt)) < 0)
            goto fail;
        is->audio_hw_buf_size = ret;
        is->audio_src = is->audio_tgt;
        is->audio_buf_size = 0;
        is->audio_buf_index = 0;

        /* init averaging filter */
        is->audio_diff_avg_coef = exp(log(0.01) / AUDIO_DIFF_AVG_NB);
        is->audio_diff_avg_count = 0;
        /* since we do not have a precise anough audio FIFO fullness,
           we correct audio sync only if larger than this threshold */
        is->audio_diff_threshold = (double)(is->audio_hw_buf_size) / is->audio_tgt.bytes_per_sec;

        is->audio_stream = stream_index;
        is->audio_st = ic->streams[stream_index];

        decoder_init(&is->auddec, avctx, &is->audioq, is->continue_read_thread);
        if ((is->ic->iformat->flags & (AVFMT_NOBINSEARCH | AVFMT_NOGENSEARCH | AVFMT_NO_BYTE_SEEK)) && !is->ic->iformat->read_seek)
        {
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

static int stream_has_enough_packets(AVStream *st, int stream_id, PacketQueue *queue)
{
    return stream_id < 0 ||
           queue->abort_request ||
           (st->disposition & AV_DISPOSITION_ATTACHED_PIC) ||
           queue->nb_packets > MIN_FRAMES && (!queue->duration || av_q2d(st->time_base) * queue->duration > 1.0);
}

static int is_realtime(AVFormatContext *s)
{
    if (!strcmp(s->iformat->name, "rtp") || !strcmp(s->iformat->name, "rtsp") || !strcmp(s->iformat->name, "sdp"))
        return 1;

    if (s->pb && (!strncmp(s->url, "rtp:", 4) || !strncmp(s->url, "udp:", 4)))
        return 1;
    return 0;
}

AVDictionary **setup_find_stream_info_opts(AVFormatContext *s,
                                           AVDictionary *codec_opts)
{
    int i;
    AVDictionary **opts;

    if (!s->nb_streams)
        return NULL;
    opts = av_mallocz_array(s->nb_streams, sizeof(*opts));
    if (!opts)
    {
        av_log(NULL, AV_LOG_ERROR,
               "Could not alloc memory for stream options.\n");
        return NULL;
    }
    for (i = 0; i < s->nb_streams; i++)
        opts[i] = filter_codec_opts(codec_opts, s->streams[i]->codecpar->codec_id,
                                    s, s->streams[i], NULL);
    return opts;
}

AVProgram *av_find_program_from_stream(AVFormatContext *ic, AVProgram *last, int s)
{
    int i, j;

    for (i = 0; i < ic->nb_programs; i++) {
        if (ic->programs[i] == last) {
            last = NULL;
        } else {
            if (!last)
                for (j = 0; j < ic->programs[i]->nb_stream_indexes; j++)
                    if (ic->programs[i]->stream_index[j] == s)
                        return ic->programs[i];
        }
    }
    return NULL;
}

static const AVCodec *find_decoder(AVFormatContext *s, const AVStream *st, enum AVCodecID codec_id)
{
#if FF_API_LAVF_AVCTX
FF_DISABLE_DEPRECATION_WARNINGS
    if (st->codec->codec)
        return st->codec->codec;
FF_ENABLE_DEPRECATION_WARNINGS
#endif

    switch (st->codecpar->codec_type) {
    case AVMEDIA_TYPE_VIDEO:
        if (s->video_codec)    return s->video_codec;
        break;
    case AVMEDIA_TYPE_AUDIO:
        if (s->audio_codec)    return s->audio_codec;
        break;
    case AVMEDIA_TYPE_SUBTITLE:
        if (s->subtitle_codec) return s->subtitle_codec;
        break;
    }

    return avcodec_find_decoder(codec_id);
}

int av_find_best_stream(AVFormatContext *ic, enum AVMediaType type,
                        int wanted_stream_nb, int related_stream,
                        AVCodec **decoder_ret, int flags)
{
    int i, nb_streams = ic->nb_streams;
    int ret = AVERROR_STREAM_NOT_FOUND;
    int best_count = -1, best_multiframe = -1, best_disposition = -1;
    int count, multiframe, disposition;
    int64_t best_bitrate = -1;
    int64_t bitrate;
    unsigned *program = NULL;
    const AVCodec *decoder = NULL, *best_decoder = NULL;

    if (related_stream >= 0 && wanted_stream_nb < 0)
    {
        AVProgram *p = av_find_program_from_stream(ic, NULL, related_stream);
        if (p)
        {
            program = p->stream_index;
            nb_streams = p->nb_stream_indexes;
        }
    }
    for (i = 0; i < nb_streams; i++)
    {
        int real_stream_index = program ? program[i] : i;
        AVStream *st = ic->streams[real_stream_index];
        AVCodecParameters *par = st->codecpar;
        if (par->codec_type != type)
            continue;
        if (wanted_stream_nb >= 0 && real_stream_index != wanted_stream_nb)
            continue;
        if (type == AVMEDIA_TYPE_AUDIO && !(par->channels && par->sample_rate))
            continue;
        if (decoder_ret)
        {
            decoder = find_decoder(ic, st, par->codec_id);
            if (!decoder)
            {
                if (ret < 0)
                    ret = AVERROR_DECODER_NOT_FOUND;
                continue;
            }
        }
        disposition = !(st->disposition & (AV_DISPOSITION_HEARING_IMPAIRED | AV_DISPOSITION_VISUAL_IMPAIRED)) + !!(st->disposition & AV_DISPOSITION_DEFAULT);
        count = st->codec_info_nb_frames;
        bitrate = par->bit_rate;
        multiframe = FFMIN(5, count);
        if ((best_disposition > disposition) ||
            (best_disposition == disposition && best_multiframe > multiframe) ||
            (best_disposition == disposition && best_multiframe == multiframe && best_bitrate > bitrate) ||
            (best_disposition == disposition && best_multiframe == multiframe && best_bitrate == bitrate && best_count >= count))
            continue;
        best_disposition = disposition;
        best_count = count;
        best_bitrate = bitrate;
        best_multiframe = multiframe;
        ret = real_stream_index;
        best_decoder = decoder;
        if (program && i == nb_streams - 1 && ret < 0)
        {
            program = NULL;
            nb_streams = ic->nb_streams;
            /* no related stream found, try again with everything */
            i = 0;
        }
    }
    if (decoder_ret)
        *decoder_ret = (AVCodec *)best_decoder;
    return ret;
}

const URLProtocol **ffurl_get_protocols(const char *whitelist,
                                        const char *blacklist)
{
    const URLProtocol **ret;
    int i, ret_idx = 0;

    ret = av_mallocz_array(FF_ARRAY_ELEMS(url_protocols), sizeof(*ret));
    if (!ret)
        return NULL;

    for (i = 0; url_protocols[i]; i++) {
        const URLProtocol *up = url_protocols[i];

        if (whitelist && *whitelist && !av_match_name(up->name, whitelist))
            continue;
        if (blacklist && *blacklist && av_match_name(up->name, blacklist))
            continue;

        ret[ret_idx++] = up;
    }

    return ret;
}

size_t av_strlcpy(char *dst, const char *src, size_t size)
{
    size_t len = 0;
    while (++len < size && *src)
        *dst++ = *src++;
    if (len <= size)
        *dst = 0;
    return len + strlen(src) - 1;
}

static const struct URLProtocol *url_find_protocol(const char *filename)
{
    const URLProtocol **protocols;
    char proto_str[128], proto_nested[128], *ptr;
    size_t proto_len = strspn(filename, URL_SCHEME_CHARS);
    int i;

    if (filename[proto_len] != ':' &&
        (strncmp(filename, "subfile,", 8) || !strchr(filename + proto_len + 1, ':')) ||
        is_dos_path(filename))
        strcpy(proto_str, "file");
    else
        av_strlcpy(proto_str, filename,
                   FFMIN(proto_len + 1, sizeof(proto_str)));

    av_strlcpy(proto_nested, proto_str, sizeof(proto_nested));
    if ((ptr = strchr(proto_nested, '+')))
        *ptr = '\0';

    protocols = ffurl_get_protocols(NULL, NULL);
    if (!protocols)
        return NULL;
    for (i = 0; protocols[i]; i++) {
            const URLProtocol *up = protocols[i];
        if (!strcmp(proto_str, up->name)) {
            av_freep(&protocols);
            return up;
        }
        if (up->flags & URL_PROTOCOL_FLAG_NESTED_SCHEME &&
            !strcmp(proto_nested, up->name)) {
            av_freep(&protocols);
            return up;
        }
    }
    av_freep(&protocols);
    if (av_strstart(filename, "https:", NULL) || av_strstart(filename, "tls:", NULL))
        av_log(NULL, AV_LOG_WARNING, "https protocol not found, recompile FFmpeg with "
                                     "openssl, gnutls or securetransport enabled.\n");

    return NULL;
}

int ffurl_alloc(URLContext **puc, const char *filename, int flags,
                const AVIOInterruptCB *int_cb)
{
    const URLProtocol *p = NULL;

    p = url_find_protocol(filename);
    if (p)
       return url_alloc_for_protocol(puc, p, filename, flags, int_cb);

    *puc = NULL;
    return AVERROR_PROTOCOL_NOT_FOUND;
}

int ffurl_open_whitelist(URLContext **puc, const char *filename, int flags,
                         const AVIOInterruptCB *int_cb, AVDictionary **options,
                         const char *whitelist, const char* blacklist,
                         URLContext *parent)
{
    AVDictionary *tmp_opts = NULL;
    AVDictionaryEntry *e;
    int ret = ffurl_alloc(puc, filename, flags, int_cb);
    if (ret < 0)
        return ret;
    if (parent)
        av_opt_copy(*puc, parent);
    if (options &&
        (ret = av_opt_set_dict(*puc, options)) < 0)
        goto fail;
    if (options && (*puc)->prot->priv_data_class &&
        (ret = av_opt_set_dict((*puc)->priv_data, options)) < 0)
        goto fail;

    if (!options)
        options = &tmp_opts;

    av_assert0(!whitelist ||
               !(e=av_dict_get(*options, "protocol_whitelist", NULL, 0)) ||
               !strcmp(whitelist, e->value));
    av_assert0(!blacklist ||
               !(e=av_dict_get(*options, "protocol_blacklist", NULL, 0)) ||
               !strcmp(blacklist, e->value));

    if ((ret = av_dict_set(options, "protocol_whitelist", whitelist, 0)) < 0)
        goto fail;

    if ((ret = av_dict_set(options, "protocol_blacklist", blacklist, 0)) < 0)
        goto fail;

    if ((ret = av_opt_set_dict(*puc, options)) < 0)
        goto fail;

    ret = ffurl_connect(*puc, options);

    if (!ret)
        return 0;
fail:
    ffurl_closep(puc);
    return ret;
}

int ffio_open_whitelist(AVIOContext **s, const char *filename, int flags,
                         const AVIOInterruptCB *int_cb, AVDictionary **options,
                         const char *whitelist, const char *blacklist
                        )
{
    URLContext *h;
    int err;

    *s = NULL;

    err = ffurl_open_whitelist(&h, filename, flags, int_cb, options, whitelist, blacklist, NULL);
    if (err < 0)
        return err;
    err = ffio_fdopen(s, h);
    if (err < 0) {
        ffurl_close(h);
        return err;
    }
    return 0;
}

static int io_open_default(AVFormatContext *s, AVIOContext **pb,
                           const char *url, int flags, AVDictionary **options)
{
    int loglevel;

    if (!strcmp(url, s->url) ||
        s->iformat && !strcmp(s->iformat->name, "image2") ||
        s->oformat && !strcmp(s->oformat->name, "image2")
    ) {
        loglevel = AV_LOG_DEBUG;
    } else
        loglevel = AV_LOG_INFO;

    av_log(s, loglevel, "Opening \'%s\' for %s\n", url, flags & AVIO_FLAG_WRITE ? "writing" : "reading");

#if FF_API_OLD_OPEN_CALLBACKS
FF_DISABLE_DEPRECATION_WARNINGS
    if (s->open_cb)
        return s->open_cb(s, pb, url, flags, &s->interrupt_callback, options);
FF_ENABLE_DEPRECATION_WARNINGS
#endif

    return ffio_open_whitelist(pb, url, flags, &s->interrupt_callback, options, s->protocol_whitelist, s->protocol_blacklist);
}

static void io_close_default(AVFormatContext *s, AVIOContext *pb)
{
    avio_close(pb);
}

static void avformat_get_context_defaults(AVFormatContext *s)
{
    memset(s, 0, sizeof(AVFormatContext));

    s->av_class = &av_format_context_class;

    s->io_open  = io_open_default;
    s->io_close = io_close_default;

    av_opt_set_defaults(s);
}


AVFormatContext *avformat_alloc_context(void)
{
    AVFormatContext *ic;
    AVFormatInternal *internal;
    ic = av_malloc(sizeof(AVFormatContext));
    if (!ic) return ic;

    internal = av_mallocz(sizeof(*internal));
    if (!internal) {
        av_free(ic);
        return NULL;
    }
    avformat_get_context_defaults(ic);
    ic->internal = internal;
    ic->internal->offset = AV_NOPTS_VALUE;
    ic->internal->raw_packet_buffer_remaining_size = RAW_PACKET_BUFFER_SIZE;
    ic->internal->shortest_end = AV_NOPTS_VALUE;

    return ic;
}

int avformat_open_input(AVFormatContext **ps, const char *filename,
                        ff_const59 AVInputFormat *fmt, AVDictionary **options)
{
    AVFormatContext *s = *ps;
    int i, ret = 0;
    AVDictionary *tmp = NULL;
    ID3v2ExtraMeta *id3v2_extra_meta = NULL;

    if (!s && !(s = avformat_alloc_context()))
        return AVERROR(ENOMEM);
    if (!s->av_class) {
        av_log(NULL, AV_LOG_ERROR, "Input context has not been properly allocated by avformat_alloc_context() and is not NULL either\n");
        return AVERROR(EINVAL);
    }
    if (fmt)
        s->iformat = fmt;

    if (options)
        av_dict_copy(&tmp, *options, 0);

    if (s->pb) // must be before any goto fail
        s->flags |= AVFMT_FLAG_CUSTOM_IO;

    if ((ret = av_opt_set_dict(s, &tmp)) < 0)
        goto fail;

    if (!(s->url = av_strdup(filename ? filename : ""))) {
        ret = AVERROR(ENOMEM);
        goto fail;
    }

#if FF_API_FORMAT_FILENAME
FF_DISABLE_DEPRECATION_WARNINGS
    av_strlcpy(s->filename, filename ? filename : "", sizeof(s->filename));
FF_ENABLE_DEPRECATION_WARNINGS
#endif
    if ((ret = init_input(s, filename, &tmp)) < 0)
        goto fail;
    s->probe_score = ret;

    if (!s->protocol_whitelist && s->pb && s->pb->protocol_whitelist) {
        s->protocol_whitelist = av_strdup(s->pb->protocol_whitelist);
        if (!s->protocol_whitelist) {
            ret = AVERROR(ENOMEM);
            goto fail;
        }
    }

    if (!s->protocol_blacklist && s->pb && s->pb->protocol_blacklist) {
        s->protocol_blacklist = av_strdup(s->pb->protocol_blacklist);
        if (!s->protocol_blacklist) {
            ret = AVERROR(ENOMEM);
            goto fail;
        }
    }

    if (s->format_whitelist && av_match_list(s->iformat->name, s->format_whitelist, ',') <= 0) {
        av_log(s, AV_LOG_ERROR, "Format not on whitelist \'%s\'\n", s->format_whitelist);
        ret = AVERROR(EINVAL);
        goto fail;
    }

    avio_skip(s->pb, s->skip_initial_bytes);

    /* Check filename in case an image number is expected. */
    if (s->iformat->flags & AVFMT_NEEDNUMBER) {
        if (!av_filename_number_test(filename)) {
            ret = AVERROR(EINVAL);
            goto fail;
        }
    }

    s->duration = s->start_time = AV_NOPTS_VALUE;

    /* Allocate private data. */
    if (s->iformat->priv_data_size > 0) {
        if (!(s->priv_data = av_mallocz(s->iformat->priv_data_size))) {
            ret = AVERROR(ENOMEM);
            goto fail;
        }
        if (s->iformat->priv_class) {
            *(const AVClass **) s->priv_data = s->iformat->priv_class;
            av_opt_set_defaults(s->priv_data);
            if ((ret = av_opt_set_dict(s->priv_data, &tmp)) < 0)
                goto fail;
        }
    }

    /* e.g. AVFMT_NOFILE formats will not have a AVIOContext */
    if (s->pb)
        ff_id3v2_read_dict(s->pb, &s->internal->id3v2_meta, ID3v2_DEFAULT_MAGIC, &id3v2_extra_meta);


    if (!(s->flags&AVFMT_FLAG_PRIV_OPT) && s->iformat->read_header)
        if ((ret = s->iformat->read_header(s)) < 0)
            goto fail;

    if (!s->metadata) {
        s->metadata = s->internal->id3v2_meta;
        s->internal->id3v2_meta = NULL;
    } else if (s->internal->id3v2_meta) {
        av_log(s, AV_LOG_WARNING, "Discarding ID3 tags because more suitable tags were found.\n");
        av_dict_free(&s->internal->id3v2_meta);
    }

    if (id3v2_extra_meta) {
        if (!strcmp(s->iformat->name, "mp3") || !strcmp(s->iformat->name, "aac") ||
            !strcmp(s->iformat->name, "tta") || !strcmp(s->iformat->name, "wav")) {
            if ((ret = ff_id3v2_parse_apic(s, id3v2_extra_meta)) < 0)
                goto close;
            if ((ret = ff_id3v2_parse_chapters(s, id3v2_extra_meta)) < 0)
                goto close;
            if ((ret = ff_id3v2_parse_priv(s, id3v2_extra_meta)) < 0)
                goto close;
        } else
            av_log(s, AV_LOG_DEBUG, "demuxer does not support additional id3 data, skipping\n");
    }
    ff_id3v2_free_extra_meta(&id3v2_extra_meta);

    if ((ret = avformat_queue_attached_pictures(s)) < 0)
        goto close;

    if (!(s->flags&AVFMT_FLAG_PRIV_OPT) && s->pb && !s->internal->data_offset)
        s->internal->data_offset = avio_tell(s->pb);

    s->internal->raw_packet_buffer_remaining_size = RAW_PACKET_BUFFER_SIZE;

    update_stream_avctx(s);

    for (i = 0; i < s->nb_streams; i++)
        s->streams[i]->internal->orig_codec_id = s->streams[i]->codecpar->codec_id;

    if (options) {
        av_dict_free(options);
        *options = tmp;
    }
    *ps = s;
    return 0;

close:
    if (s->iformat->read_close)
        s->iformat->read_close(s);
fail:
    ff_id3v2_free_extra_meta(&id3v2_extra_meta);
    av_dict_free(&tmp);
    if (s->pb && !(s->flags & AVFMT_FLAG_CUSTOM_IO))
        avio_closep(&s->pb);
    avformat_free_context(s);
    *ps = NULL;
    return ret;
}

void av_format_inject_global_side_data(AVFormatContext *s)
{
    int i;
    s->internal->inject_global_side_data = 1;
    for (i = 0; i < s->nb_streams; i++) {
        AVStream *st = s->streams[i];
        st->inject_global_side_data = 1;
    }
}

int avformat_find_stream_info(AVFormatContext *ic, AVDictionary **options)
{
    int i, count = 0, ret = 0, j;
    int64_t read_size;
    AVStream *st;
    AVCodecContext *avctx;
    AVPacket pkt1;
    int64_t old_offset  = avio_tell(ic->pb);
    // new streams might appear, no options for those
    int orig_nb_streams = ic->nb_streams;
    int flush_codecs;
    int64_t max_analyze_duration = ic->max_analyze_duration;
    int64_t max_stream_analyze_duration;
    int64_t max_subtitle_analyze_duration;
    int64_t probesize = ic->probesize;
    int eof_reached = 0;
    int *missing_streams = av_opt_ptr(ic->iformat->priv_class, ic->priv_data, "missing_streams");

    flush_codecs = probesize > 0;

    av_opt_set(ic, "skip_clear", "1", AV_OPT_SEARCH_CHILDREN);

    max_stream_analyze_duration = max_analyze_duration;
    max_subtitle_analyze_duration = max_analyze_duration;
    if (!max_analyze_duration) {
        max_stream_analyze_duration =
        max_analyze_duration        = 5*AV_TIME_BASE;
        max_subtitle_analyze_duration = 30*AV_TIME_BASE;
        if (!strcmp(ic->iformat->name, "flv"))
            max_stream_analyze_duration = 90*AV_TIME_BASE;
        if (!strcmp(ic->iformat->name, "mpeg") || !strcmp(ic->iformat->name, "mpegts"))
            max_stream_analyze_duration = 7*AV_TIME_BASE;
    }

    if (ic->pb)
        av_log(ic, AV_LOG_DEBUG, "Before avformat_find_stream_info() pos: %"PRId64" bytes read:%"PRId64" seeks:%d nb_streams:%d\n",
               avio_tell(ic->pb), ic->pb->bytes_read, ic->pb->seek_count, ic->nb_streams);

    for (i = 0; i < ic->nb_streams; i++) {
        const AVCodec *codec;
        AVDictionary *thread_opt = NULL;
        st = ic->streams[i];
        avctx = st->internal->avctx;

        if (st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO ||
            st->codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE) {
/*            if (!st->time_base.num)
                st->time_base = */
            if (!avctx->time_base.num)
                avctx->time_base = st->time_base;
        }

        /* check if the caller has overridden the codec id */
#if FF_API_LAVF_AVCTX
FF_DISABLE_DEPRECATION_WARNINGS
        if (st->codec->codec_id != st->internal->orig_codec_id) {
            st->codecpar->codec_id   = st->codec->codec_id;
            st->codecpar->codec_type = st->codec->codec_type;
            st->internal->orig_codec_id = st->codec->codec_id;
        }
FF_ENABLE_DEPRECATION_WARNINGS
#endif
        // only for the split stuff
        if (!st->parser && !(ic->flags & AVFMT_FLAG_NOPARSE) && st->request_probe <= 0) {
            st->parser = av_parser_init(st->codecpar->codec_id);
            if (st->parser) {
                if (st->need_parsing == AVSTREAM_PARSE_HEADERS) {
                    st->parser->flags |= PARSER_FLAG_COMPLETE_FRAMES;
                } else if (st->need_parsing == AVSTREAM_PARSE_FULL_RAW) {
                    st->parser->flags |= PARSER_FLAG_USE_CODEC_TS;
                }
            } else if (st->need_parsing) {
                av_log(ic, AV_LOG_VERBOSE, "parser not found for codec "
                       "%s, packets or times may be invalid.\n",
                       avcodec_get_name(st->codecpar->codec_id));
            }
        }

        if (st->codecpar->codec_id != st->internal->orig_codec_id)
            st->internal->orig_codec_id = st->codecpar->codec_id;

        ret = avcodec_parameters_to_context(avctx, st->codecpar);
        if (ret < 0)
            goto find_stream_info_err;
        if (st->request_probe <= 0)
            st->internal->avctx_inited = 1;

        codec = find_probe_decoder(ic, st, st->codecpar->codec_id);

        /* Force thread count to 1 since the H.264 decoder will not extract
         * SPS and PPS to extradata during multi-threaded decoding. */
        av_dict_set(options ? &options[i] : &thread_opt, "threads", "1", 0);

        if (ic->codec_whitelist)
            av_dict_set(options ? &options[i] : &thread_opt, "codec_whitelist", ic->codec_whitelist, 0);

        /* Ensure that subtitle_header is properly set. */
        if (st->codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE
            && codec && !avctx->codec) {
            if (avcodec_open2(avctx, codec, options ? &options[i] : &thread_opt) < 0)
                av_log(ic, AV_LOG_WARNING,
                       "Failed to open codec in %s\n",__FUNCTION__);
        }

        // Try to just open decoders, in case this is enough to get parameters.
        if (!has_codec_parameters(st, NULL) && st->request_probe <= 0) {
            if (codec && !avctx->codec)
                if (avcodec_open2(avctx, codec, options ? &options[i] : &thread_opt) < 0)
                    av_log(ic, AV_LOG_WARNING,
                           "Failed to open codec in %s\n",__FUNCTION__);
        }
        if (!options)
            av_dict_free(&thread_opt);
    }

    for (i = 0; i < ic->nb_streams; i++) {
#if FF_API_R_FRAME_RATE
        ic->streams[i]->info->last_dts = AV_NOPTS_VALUE;
#endif
        ic->streams[i]->info->fps_first_dts = AV_NOPTS_VALUE;
        ic->streams[i]->info->fps_last_dts  = AV_NOPTS_VALUE;
    }

    read_size = 0;
    for (;;) {
        const AVPacket *pkt;
        int analyzed_all_streams;
        if (ff_check_interrupt(&ic->interrupt_callback)) {
            ret = AVERROR_EXIT;
            av_log(ic, AV_LOG_DEBUG, "interrupted\n");
            break;
        }

        /* check if one codec still needs to be handled */
        for (i = 0; i < ic->nb_streams; i++) {
            int fps_analyze_framecount = 20;
            int count;

            st = ic->streams[i];
            if (!has_codec_parameters(st, NULL))
                break;
            /* If the timebase is coarse (like the usual millisecond precision
             * of mkv), we need to analyze more frames to reliably arrive at
             * the correct fps. */
            if (av_q2d(st->time_base) > 0.0005)
                fps_analyze_framecount *= 2;
            if (!tb_unreliable(st->internal->avctx))
                fps_analyze_framecount = 0;
            if (ic->fps_probe_size >= 0)
                fps_analyze_framecount = ic->fps_probe_size;
            if (st->disposition & AV_DISPOSITION_ATTACHED_PIC)
                fps_analyze_framecount = 0;
            /* variable fps and no guess at the real fps */
            count = (ic->iformat->flags & AVFMT_NOTIMESTAMPS) ?
                       st->info->codec_info_duration_fields/2 :
                       st->info->duration_count;
            if (!(st->r_frame_rate.num && st->avg_frame_rate.num) &&
                st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                if (count < fps_analyze_framecount)
                    break;
            }
            // Look at the first 3 frames if there is evidence of frame delay
            // but the decoder delay is not set.
            if (st->info->frame_delay_evidence && count < 2 && st->internal->avctx->has_b_frames == 0)
                break;
            if (!st->internal->avctx->extradata &&
                (!st->internal->extract_extradata.inited ||
                 st->internal->extract_extradata.bsf) &&
                extract_extradata_check(st))
                break;
            if (st->first_dts == AV_NOPTS_VALUE &&
                !(ic->iformat->flags & AVFMT_NOTIMESTAMPS) &&
                st->codec_info_nb_frames < ((st->disposition & AV_DISPOSITION_ATTACHED_PIC) ? 1 : ic->max_ts_probe) &&
                (st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO ||
                 st->codecpar->codec_type == AVMEDIA_TYPE_AUDIO))
                break;
        }
        analyzed_all_streams = 0;
        if (!missing_streams || !*missing_streams)
            if (i == ic->nb_streams) {
                analyzed_all_streams = 1;
                /* NOTE: If the format has no header, then we need to read some
                 * packets to get most of the streams, so we cannot stop here. */
                if (!(ic->ctx_flags & AVFMTCTX_NOHEADER)) {
                    /* If we found the info for all the codecs, we can stop. */
                    ret = count;
                    av_log(ic, AV_LOG_DEBUG, "All info found\n");
                    flush_codecs = 0;
                    break;
                }
            }
        /* We did not get all the codec info, but we read too much data. */
        if (read_size >= probesize) {
            ret = count;
            av_log(ic, AV_LOG_DEBUG,
                   "Probe buffer size limit of %"PRId64" bytes reached\n", probesize);
            for (i = 0; i < ic->nb_streams; i++)
                if (!ic->streams[i]->r_frame_rate.num &&
                    ic->streams[i]->info->duration_count <= 1 &&
                    ic->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO &&
                    strcmp(ic->iformat->name, "image2"))
                    av_log(ic, AV_LOG_WARNING,
                           "Stream #%d: not enough frames to estimate rate; "
                           "consider increasing probesize\n", i);
            break;
        }

        /* NOTE: A new stream can be added there if no header in file
         * (AVFMTCTX_NOHEADER). */
        ret = read_frame_internal(ic, &pkt1);
        if (ret == AVERROR(EAGAIN))
            continue;

        if (ret < 0) {
            /* EOF or error*/
            eof_reached = 1;
            break;
        }

        if (!(ic->flags & AVFMT_FLAG_NOBUFFER)) {
            ret = avpriv_packet_list_put(&ic->internal->packet_buffer,
                                     &ic->internal->packet_buffer_end,
                                     &pkt1, NULL, 0);
            if (ret < 0)
                goto unref_then_goto_end;

            pkt = &ic->internal->packet_buffer_end->pkt;
        } else {
            pkt = &pkt1;
        }

        st = ic->streams[pkt->stream_index];
        if (!(st->disposition & AV_DISPOSITION_ATTACHED_PIC))
            read_size += pkt->size;

        avctx = st->internal->avctx;
        if (!st->internal->avctx_inited) {
            ret = avcodec_parameters_to_context(avctx, st->codecpar);
            if (ret < 0)
                goto unref_then_goto_end;
            st->internal->avctx_inited = 1;
        }

        if (pkt->dts != AV_NOPTS_VALUE && st->codec_info_nb_frames > 1) {
            /* check for non-increasing dts */
            if (st->info->fps_last_dts != AV_NOPTS_VALUE &&
                st->info->fps_last_dts >= pkt->dts) {
                av_log(ic, AV_LOG_DEBUG,
                       "Non-increasing DTS in stream %d: packet %d with DTS "
                       "%"PRId64", packet %d with DTS %"PRId64"\n",
                       st->index, st->info->fps_last_dts_idx,
                       st->info->fps_last_dts, st->codec_info_nb_frames,
                       pkt->dts);
                st->info->fps_first_dts =
                st->info->fps_last_dts  = AV_NOPTS_VALUE;
            }
            /* Check for a discontinuity in dts. If the difference in dts
             * is more than 1000 times the average packet duration in the
             * sequence, we treat it as a discontinuity. */
            if (st->info->fps_last_dts != AV_NOPTS_VALUE &&
                st->info->fps_last_dts_idx > st->info->fps_first_dts_idx &&
                (pkt->dts - (uint64_t)st->info->fps_last_dts) / 1000 >
                (st->info->fps_last_dts     - (uint64_t)st->info->fps_first_dts) /
                (st->info->fps_last_dts_idx - st->info->fps_first_dts_idx)) {
                av_log(ic, AV_LOG_WARNING,
                       "DTS discontinuity in stream %d: packet %d with DTS "
                       "%"PRId64", packet %d with DTS %"PRId64"\n",
                       st->index, st->info->fps_last_dts_idx,
                       st->info->fps_last_dts, st->codec_info_nb_frames,
                       pkt->dts);
                st->info->fps_first_dts =
                st->info->fps_last_dts  = AV_NOPTS_VALUE;
            }

            /* update stored dts values */
            if (st->info->fps_first_dts == AV_NOPTS_VALUE) {
                st->info->fps_first_dts     = pkt->dts;
                st->info->fps_first_dts_idx = st->codec_info_nb_frames;
            }
            st->info->fps_last_dts     = pkt->dts;
            st->info->fps_last_dts_idx = st->codec_info_nb_frames;
        }
        if (st->codec_info_nb_frames>1) {
            int64_t t = 0;
            int64_t limit;

            if (st->time_base.den > 0)
                t = av_rescale_q(st->info->codec_info_duration, st->time_base, AV_TIME_BASE_Q);
            if (st->avg_frame_rate.num > 0)
                t = FFMAX(t, av_rescale_q(st->codec_info_nb_frames, av_inv_q(st->avg_frame_rate), AV_TIME_BASE_Q));

            if (   t == 0
                && st->codec_info_nb_frames>30
                && st->info->fps_first_dts != AV_NOPTS_VALUE
                && st->info->fps_last_dts  != AV_NOPTS_VALUE)
                t = FFMAX(t, av_rescale_q(st->info->fps_last_dts - st->info->fps_first_dts, st->time_base, AV_TIME_BASE_Q));

            if (analyzed_all_streams)                                limit = max_analyze_duration;
            else if (avctx->codec_type == AVMEDIA_TYPE_SUBTITLE) limit = max_subtitle_analyze_duration;
            else                                                     limit = max_stream_analyze_duration;

            if (t >= limit) {
                av_log(ic, AV_LOG_VERBOSE, "max_analyze_duration %"PRId64" reached at %"PRId64" microseconds st:%d\n",
                       limit,
                       t, pkt->stream_index);
                if (ic->flags & AVFMT_FLAG_NOBUFFER)
                    av_packet_unref(&pkt1);
                break;
            }
            if (pkt->duration) {
                if (avctx->codec_type == AVMEDIA_TYPE_SUBTITLE && pkt->pts != AV_NOPTS_VALUE && st->start_time != AV_NOPTS_VALUE && pkt->pts >= st->start_time) {
                    st->info->codec_info_duration = FFMIN(pkt->pts - st->start_time, st->info->codec_info_duration + pkt->duration);
                } else
                    st->info->codec_info_duration += pkt->duration;
                st->info->codec_info_duration_fields += st->parser && st->need_parsing && avctx->ticks_per_frame ==2 ? st->parser->repeat_pict + 1 : 2;
            }
        }
        if (st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
#if FF_API_R_FRAME_RATE
            ff_rfps_add_frame(ic, st, pkt->dts);
#endif
            if (pkt->dts != pkt->pts && pkt->dts != AV_NOPTS_VALUE && pkt->pts != AV_NOPTS_VALUE)
                st->info->frame_delay_evidence = 1;
        }
        if (!st->internal->avctx->extradata) {
            ret = extract_extradata(st, pkt);
            if (ret < 0)
                goto unref_then_goto_end;
        }

        /* If still no information, we try to open the codec and to
         * decompress the frame. We try to avoid that in most cases as
         * it takes longer and uses more memory. For MPEG-4, we need to
         * decompress for QuickTime.
         *
         * If AV_CODEC_CAP_CHANNEL_CONF is set this will force decoding of at
         * least one frame of codec data, this makes sure the codec initializes
         * the channel configuration and does not only trust the values from
         * the container. */
        try_decode_frame(ic, st, pkt,
                         (options && i < orig_nb_streams) ? &options[i] : NULL);

        if (ic->flags & AVFMT_FLAG_NOBUFFER)
            av_packet_unref(&pkt1);

        st->codec_info_nb_frames++;
        count++;
    }

    if (eof_reached) {
        int stream_index;
        for (stream_index = 0; stream_index < ic->nb_streams; stream_index++) {
            st = ic->streams[stream_index];
            avctx = st->internal->avctx;
            if (!has_codec_parameters(st, NULL)) {
                const AVCodec *codec = find_probe_decoder(ic, st, st->codecpar->codec_id);
                if (codec && !avctx->codec) {
                    AVDictionary *opts = NULL;
                    if (ic->codec_whitelist)
                        av_dict_set(&opts, "codec_whitelist", ic->codec_whitelist, 0);
                    if (avcodec_open2(avctx, codec, (options && stream_index < orig_nb_streams) ? &options[stream_index] : &opts) < 0)
                        av_log(ic, AV_LOG_WARNING,
                               "Failed to open codec in %s\n",__FUNCTION__);
                    av_dict_free(&opts);
                }
            }

            // EOF already reached while reading the stream above.
            // So continue with reoordering DTS with whatever delay we have.
            if (ic->internal->packet_buffer && !has_decode_delay_been_guessed(st)) {
                update_dts_from_pts(ic, stream_index, ic->internal->packet_buffer);
            }
        }
    }

    if (flush_codecs) {
        AVPacket empty_pkt = { 0 };
        int err = 0;
        av_init_packet(&empty_pkt);

        for (i = 0; i < ic->nb_streams; i++) {

            st = ic->streams[i];

            /* flush the decoders */
            if (st->info->found_decoder == 1) {
                do {
                    err = try_decode_frame(ic, st, &empty_pkt,
                                            (options && i < orig_nb_streams)
                                            ? &options[i] : NULL);
                } while (err > 0 && !has_codec_parameters(st, NULL));

                if (err < 0) {
                    av_log(ic, AV_LOG_INFO,
                        "decoding for stream %d failed\n", st->index);
                }
            }
        }
    }

    ff_rfps_calculate(ic);

    for (i = 0; i < ic->nb_streams; i++) {
        st = ic->streams[i];
        avctx = st->internal->avctx;
        if (avctx->codec_type == AVMEDIA_TYPE_VIDEO) {
            if (avctx->codec_id == AV_CODEC_ID_RAWVIDEO && !avctx->codec_tag && !avctx->bits_per_coded_sample) {
                uint32_t tag= avcodec_pix_fmt_to_codec_tag(avctx->pix_fmt);
                if (avpriv_find_pix_fmt(avpriv_get_raw_pix_fmt_tags(), tag) == avctx->pix_fmt)
                    avctx->codec_tag= tag;
            }

            /* estimate average framerate if not set by demuxer */
            if (st->info->codec_info_duration_fields &&
                !st->avg_frame_rate.num &&
                st->info->codec_info_duration) {
                int best_fps      = 0;
                double best_error = 0.01;
                AVRational codec_frame_rate = avctx->framerate;

                if (st->info->codec_info_duration        >= INT64_MAX / st->time_base.num / 2||
                    st->info->codec_info_duration_fields >= INT64_MAX / st->time_base.den ||
                    st->info->codec_info_duration        < 0)
                    continue;
                av_reduce(&st->avg_frame_rate.num, &st->avg_frame_rate.den,
                          st->info->codec_info_duration_fields * (int64_t) st->time_base.den,
                          st->info->codec_info_duration * 2 * (int64_t) st->time_base.num, 60000);

                /* Round guessed framerate to a "standard" framerate if it's
                 * within 1% of the original estimate. */
                for (j = 0; j < MAX_STD_TIMEBASES; j++) {
                    AVRational std_fps = { get_std_framerate(j), 12 * 1001 };
                    double error       = fabs(av_q2d(st->avg_frame_rate) /
                                              av_q2d(std_fps) - 1);

                    if (error < best_error) {
                        best_error = error;
                        best_fps   = std_fps.num;
                    }

                    if (ic->internal->prefer_codec_framerate && codec_frame_rate.num > 0 && codec_frame_rate.den > 0) {
                        error       = fabs(av_q2d(codec_frame_rate) /
                                           av_q2d(std_fps) - 1);
                        if (error < best_error) {
                            best_error = error;
                            best_fps   = std_fps.num;
                        }
                    }
                }
                if (best_fps)
                    av_reduce(&st->avg_frame_rate.num, &st->avg_frame_rate.den,
                              best_fps, 12 * 1001, INT_MAX);
            }

            if (!st->r_frame_rate.num) {
                if (    avctx->time_base.den * (int64_t) st->time_base.num
                    <= avctx->time_base.num * avctx->ticks_per_frame * (uint64_t) st->time_base.den) {
                    av_reduce(&st->r_frame_rate.num, &st->r_frame_rate.den,
                              avctx->time_base.den, (int64_t)avctx->time_base.num * avctx->ticks_per_frame, INT_MAX);
                } else {
                    st->r_frame_rate.num = st->time_base.den;
                    st->r_frame_rate.den = st->time_base.num;
                }
            }
            if (st->display_aspect_ratio.num && st->display_aspect_ratio.den) {
                AVRational hw_ratio = { avctx->height, avctx->width };
                st->sample_aspect_ratio = av_mul_q(st->display_aspect_ratio,
                                                   hw_ratio);
            }
        } else if (avctx->codec_type == AVMEDIA_TYPE_AUDIO) {
            if (!avctx->bits_per_coded_sample)
                avctx->bits_per_coded_sample =
                    av_get_bits_per_sample(avctx->codec_id);
            // set stream disposition based on audio service type
            switch (avctx->audio_service_type) {
            case AV_AUDIO_SERVICE_TYPE_EFFECTS:
                st->disposition = AV_DISPOSITION_CLEAN_EFFECTS;
                break;
            case AV_AUDIO_SERVICE_TYPE_VISUALLY_IMPAIRED:
                st->disposition = AV_DISPOSITION_VISUAL_IMPAIRED;
                break;
            case AV_AUDIO_SERVICE_TYPE_HEARING_IMPAIRED:
                st->disposition = AV_DISPOSITION_HEARING_IMPAIRED;
                break;
            case AV_AUDIO_SERVICE_TYPE_COMMENTARY:
                st->disposition = AV_DISPOSITION_COMMENT;
                break;
            case AV_AUDIO_SERVICE_TYPE_KARAOKE:
                st->disposition = AV_DISPOSITION_KARAOKE;
                break;
            }
        }
    }

    if (probesize)
        estimate_timings(ic, old_offset);

    av_opt_set(ic, "skip_clear", "0", AV_OPT_SEARCH_CHILDREN);

    if (ret >= 0 && ic->nb_streams)
        /* We could not have all the codec parameters before EOF. */
        ret = -1;
    for (i = 0; i < ic->nb_streams; i++) {
        const char *errmsg;
        st = ic->streams[i];

        /* if no packet was ever seen, update context now for has_codec_parameters */
        if (!st->internal->avctx_inited) {
            if (st->codecpar->codec_type == AVMEDIA_TYPE_AUDIO &&
                st->codecpar->format == AV_SAMPLE_FMT_NONE)
                st->codecpar->format = st->internal->avctx->sample_fmt;
            ret = avcodec_parameters_to_context(st->internal->avctx, st->codecpar);
            if (ret < 0)
                goto find_stream_info_err;
        }
        if (!has_codec_parameters(st, &errmsg)) {
            char buf[256];
            avcodec_string(buf, sizeof(buf), st->internal->avctx, 0);
            av_log(ic, AV_LOG_WARNING,
                   "Could not find codec parameters for stream %d (%s): %s\n"
                   "Consider increasing the value for the 'analyzeduration' (%"PRId64") and 'probesize' (%"PRId64") options\n",
                   i, buf, errmsg, ic->max_analyze_duration, ic->probesize);
        } else {
            ret = 0;
        }
    }

    compute_chapters_end(ic);

    /* update the stream parameters from the internal codec contexts */
    for (i = 0; i < ic->nb_streams; i++) {
        st = ic->streams[i];

        if (st->internal->avctx_inited) {
            int orig_w = st->codecpar->width;
            int orig_h = st->codecpar->height;
            ret = avcodec_parameters_from_context(st->codecpar, st->internal->avctx);
            if (ret < 0)
                goto find_stream_info_err;
            ret = add_coded_side_data(st, st->internal->avctx);
            if (ret < 0)
                goto find_stream_info_err;
#if FF_API_LOWRES
            // The decoder might reduce the video size by the lowres factor.
            if (st->internal->avctx->lowres && orig_w) {
                st->codecpar->width = orig_w;
                st->codecpar->height = orig_h;
            }
#endif
        }

#if FF_API_LAVF_AVCTX
FF_DISABLE_DEPRECATION_WARNINGS
        ret = avcodec_parameters_to_context(st->codec, st->codecpar);
        if (ret < 0)
            goto find_stream_info_err;

#if FF_API_LOWRES
        // The old API (AVStream.codec) "requires" the resolution to be adjusted
        // by the lowres factor.
        if (st->internal->avctx->lowres && st->internal->avctx->width) {
            st->codec->lowres = st->internal->avctx->lowres;
            st->codec->width = st->internal->avctx->width;
            st->codec->height = st->internal->avctx->height;
        }
#endif

        if (st->codec->codec_tag != MKTAG('t','m','c','d')) {
            st->codec->time_base = st->internal->avctx->time_base;
            st->codec->ticks_per_frame = st->internal->avctx->ticks_per_frame;
        }
        st->codec->framerate = st->avg_frame_rate;

        if (st->internal->avctx->subtitle_header) {
            st->codec->subtitle_header = av_malloc(st->internal->avctx->subtitle_header_size);
            if (!st->codec->subtitle_header)
                goto find_stream_info_err;
            st->codec->subtitle_header_size = st->internal->avctx->subtitle_header_size;
            memcpy(st->codec->subtitle_header, st->internal->avctx->subtitle_header,
                   st->codec->subtitle_header_size);
        }

        // Fields unavailable in AVCodecParameters
        st->codec->coded_width = st->internal->avctx->coded_width;
        st->codec->coded_height = st->internal->avctx->coded_height;
        st->codec->properties = st->internal->avctx->properties;
FF_ENABLE_DEPRECATION_WARNINGS
#endif

        st->internal->avctx_inited = 0;
    }

find_stream_info_err:
    for (i = 0; i < ic->nb_streams; i++) {
        st = ic->streams[i];
        if (st->info)
            av_freep(&st->info->duration_error);
        avcodec_close(ic->streams[i]->internal->avctx);
        av_freep(&ic->streams[i]->info);
        av_bsf_free(&ic->streams[i]->internal->extract_extradata.bsf);
        av_packet_free(&ic->streams[i]->internal->extract_extradata.pkt);
    }
    if (ic->pb)
        av_log(ic, AV_LOG_DEBUG, "After avformat_find_stream_info() pos: %"PRId64" bytes read:%"PRId64" seeks:%d frames:%d\n",
               avio_tell(ic->pb), ic->pb->bytes_read, ic->pb->seek_count, count);
    return ret;

unref_then_goto_end:
    av_packet_unref(&pkt1);
    goto find_stream_info_err;
}

char *av_asprintf(const char *fmt, ...)
{
    char *p = NULL;
    va_list va;
    int len;

    va_start(va, fmt);
    len = vsnprintf(NULL, 0, fmt, va);
    va_end(va);
    if (len < 0)
        goto end;

    p = av_malloc(len + 1);
    if (!p)
        goto end;

    va_start(va, fmt);
    len = vsnprintf(p, len + 1, fmt, va);
    va_end(va);
    if (len < 0)
        av_freep(&p);

end:
    return p;
}


void av_dump_format(AVFormatContext *ic, int index,
                    const char *url, int is_output)
{
    int i;
    uint8_t *printed = ic->nb_streams ? av_mallocz(ic->nb_streams) : NULL;
    if (ic->nb_streams && !printed)
        return;

    av_log(NULL, AV_LOG_INFO, "%s #%d, %s, %s '%s':\n",
           is_output ? "Output" : "Input",
           index,
           is_output ? ic->oformat->name : ic->iformat->name,
           is_output ? "to" : "from", url);
    dump_metadata(NULL, ic->metadata, "  ");

    if (!is_output) {
        av_log(NULL, AV_LOG_INFO, "  Duration: ");
        if (ic->duration != AV_NOPTS_VALUE) {
            int64_t hours, mins, secs, us;
            int64_t duration = ic->duration + (ic->duration <= INT64_MAX - 5000 ? 5000 : 0);
            secs  = duration / AV_TIME_BASE;
            us    = duration % AV_TIME_BASE;
            mins  = secs / 60;
            secs %= 60;
            hours = mins / 60;
            mins %= 60;
            av_log(NULL, AV_LOG_INFO, "%02"PRId64":%02"PRId64":%02"PRId64".%02"PRId64"", hours, mins, secs,
                   (100 * us) / AV_TIME_BASE);
        } else {
            av_log(NULL, AV_LOG_INFO, "N/A");
        }
        if (ic->start_time != AV_NOPTS_VALUE) {
            int secs, us;
            av_log(NULL, AV_LOG_INFO, ", start: ");
            secs = llabs(ic->start_time / AV_TIME_BASE);
            us   = llabs(ic->start_time % AV_TIME_BASE);
            av_log(NULL, AV_LOG_INFO, "%s%d.%06d",
                   ic->start_time >= 0 ? "" : "-",
                   secs,
                   (int) av_rescale(us, 1000000, AV_TIME_BASE));
        }
        av_log(NULL, AV_LOG_INFO, ", bitrate: ");
        if (ic->bit_rate)
            av_log(NULL, AV_LOG_INFO, "%"PRId64" kb/s", ic->bit_rate / 1000);
        else
            av_log(NULL, AV_LOG_INFO, "N/A");
        av_log(NULL, AV_LOG_INFO, "\n");
    }

    for (i = 0; i < ic->nb_chapters; i++) {
        const AVChapter *ch = ic->chapters[i];
        av_log(NULL, AV_LOG_INFO, "    Chapter #%d:%d: ", index, i);
        av_log(NULL, AV_LOG_INFO,
               "start %f, ", ch->start * av_q2d(ch->time_base));
        av_log(NULL, AV_LOG_INFO,
               "end %f\n", ch->end * av_q2d(ch->time_base));

        dump_metadata(NULL, ch->metadata, "    ");
    }

    if (ic->nb_programs) {
        int j, k, total = 0;
        for (j = 0; j < ic->nb_programs; j++) {
            const AVProgram *program = ic->programs[j];
            const AVDictionaryEntry *name = av_dict_get(program->metadata,
                                                        "name", NULL, 0);
            av_log(NULL, AV_LOG_INFO, "  Program %d %s\n", program->id,
                   name ? name->value : "");
            dump_metadata(NULL, program->metadata, "    ");
            for (k = 0; k < program->nb_stream_indexes; k++) {
                dump_stream_format(ic, program->stream_index[k],
                                   index, is_output);
                printed[program->stream_index[k]] = 1;
            }
            total += program->nb_stream_indexes;
        }
        if (total < ic->nb_streams)
            av_log(NULL, AV_LOG_INFO, "  No Program\n");
    }

    for (i = 0; i < ic->nb_streams; i++)
        if (!printed[i])
            dump_stream_format(ic, i, index, is_output);

    av_free(printed);
}

int avio_pause(AVIOContext *s, int pause)
{
    if (!s->read_pause)
        return AVERROR(ENOSYS);
    return s->read_pause(s->opaque, pause);
}

int av_read_play(AVFormatContext *s)
{
    if (s->iformat->read_play)
        return s->iformat->read_play(s);
    if (s->pb)
        return avio_pause(s->pb, 0);
    return AVERROR(ENOSYS);
}

int av_read_pause(AVFormatContext *s)
{
    if (s->iformat->read_pause)
        return s->iformat->read_pause(s);
    if (s->pb)
        return avio_pause(s->pb, 1);
    return AVERROR(ENOSYS);
}

void ff_read_frame_flush(AVFormatContext *s)
{
    AVStream *st;
    int i, j;

    flush_packet_queue(s);

    /* Reset read state for each stream. */
    for (i = 0; i < s->nb_streams; i++) {
        st = s->streams[i];

        if (st->parser) {
            av_parser_close(st->parser);
            st->parser = NULL;
        }
        st->last_IP_pts = AV_NOPTS_VALUE;
        st->last_dts_for_order_check = AV_NOPTS_VALUE;
        if (st->first_dts == AV_NOPTS_VALUE)
            st->cur_dts = RELATIVE_TS_BASE;
        else
            /* We set the current DTS to an unspecified origin. */
            st->cur_dts = AV_NOPTS_VALUE;

        st->probe_packets = s->max_probe_packets;

        for (j = 0; j < MAX_REORDER_DELAY + 1; j++)
            st->pts_buffer[j] = AV_NOPTS_VALUE;

        if (s->internal->inject_global_side_data)
            st->inject_global_side_data = 1;

        st->skip_samples = 0;
    }
}

int avformat_seek_file(AVFormatContext *s, int stream_index, int64_t min_ts,
                       int64_t ts, int64_t max_ts, int flags)
{
    if (min_ts > ts || max_ts < ts)
        return -1;
    if (stream_index < -1 || stream_index >= (int)s->nb_streams)
        return AVERROR(EINVAL);

    if (s->seek2any>0)
        flags |= AVSEEK_FLAG_ANY;
    flags &= ~AVSEEK_FLAG_BACKWARD;

    if (s->iformat->read_seek2) {
        int ret;
        ff_read_frame_flush(s);

        if (stream_index == -1 && s->nb_streams == 1) {
            AVRational time_base = s->streams[0]->time_base;
            ts = av_rescale_q(ts, AV_TIME_BASE_Q, time_base);
            min_ts = av_rescale_rnd(min_ts, time_base.den,
                                    time_base.num * (int64_t)AV_TIME_BASE,
                                    AV_ROUND_UP   | AV_ROUND_PASS_MINMAX);
            max_ts = av_rescale_rnd(max_ts, time_base.den,
                                    time_base.num * (int64_t)AV_TIME_BASE,
                                    AV_ROUND_DOWN | AV_ROUND_PASS_MINMAX);
            stream_index = 0;
        }

        ret = s->iformat->read_seek2(s, stream_index, min_ts,
                                     ts, max_ts, flags);

        if (ret >= 0)
            ret = avformat_queue_attached_pictures(s);
        return ret;
    }

    if (s->iformat->read_timestamp) {
        // try to seek via read_timestamp()
    }

    // Fall back on old API if new is not implemented but old is.
    // Note the old API has somewhat different semantics.
    if (s->iformat->read_seek || 1) {
        int dir = (ts - (uint64_t)min_ts > (uint64_t)max_ts - ts ? AVSEEK_FLAG_BACKWARD : 0);
        int ret = av_seek_frame(s, stream_index, ts, flags | dir);
        if (ret<0 && ts != min_ts && max_ts != ts) {
            ret = av_seek_frame(s, stream_index, dir ? max_ts : min_ts, flags | dir);
            if (ret >= 0)
                ret = av_seek_frame(s, stream_index, ts, flags | (dir^AVSEEK_FLAG_BACKWARD));
        }
        return ret;
    }

    // try some generic seek like seek_frame_generic() but with new ts semantics
    return -1; //unreachable
}


int avpriv_packet_list_get(AVPacketList **pkt_buffer,
                           AVPacketList **pkt_buffer_end,
                           AVPacket      *pkt)
{
    AVPacketList *pktl;
    if (!*pkt_buffer)
        return AVERROR(EAGAIN);
    pktl        = *pkt_buffer;
    *pkt        = pktl->pkt;
    *pkt_buffer = pktl->next;
    if (!pktl->next)
        *pkt_buffer_end = NULL;
    av_freep(&pktl);
    return 0;
}


int av_read_frame(AVFormatContext *s, AVPacket *pkt)
{
    const int genpts = s->flags & AVFMT_FLAG_GENPTS;
    int eof = 0;
    int ret;
    AVStream *st;

    if (!genpts) {
        ret = s->internal->packet_buffer
              ? avpriv_packet_list_get(&s->internal->packet_buffer,
                                        &s->internal->packet_buffer_end, pkt)
              : read_frame_internal(s, pkt);
        if (ret < 0)
            return ret;
        goto return_packet;
    }

    for (;;) {
        AVPacketList *pktl = s->internal->packet_buffer;

        if (pktl) {
            AVPacket *next_pkt = &pktl->pkt;

            if (next_pkt->dts != AV_NOPTS_VALUE) {
                int wrap_bits = s->streams[next_pkt->stream_index]->pts_wrap_bits;
                // last dts seen for this stream. if any of packets following
                // current one had no dts, we will set this to AV_NOPTS_VALUE.
                int64_t last_dts = next_pkt->dts;
                av_assert2(wrap_bits <= 64);
                while (pktl && next_pkt->pts == AV_NOPTS_VALUE) {
                    if (pktl->pkt.stream_index == next_pkt->stream_index &&
                        av_compare_mod(next_pkt->dts, pktl->pkt.dts, 2ULL << (wrap_bits - 1)) < 0) {
                        if (av_compare_mod(pktl->pkt.pts, pktl->pkt.dts, 2ULL << (wrap_bits - 1))) {
                            // not B-frame
                            next_pkt->pts = pktl->pkt.dts;
                        }
                        if (last_dts != AV_NOPTS_VALUE) {
                            // Once last dts was set to AV_NOPTS_VALUE, we don't change it.
                            last_dts = pktl->pkt.dts;
                        }
                    }
                    pktl = pktl->next;
                }
                if (eof && next_pkt->pts == AV_NOPTS_VALUE && last_dts != AV_NOPTS_VALUE) {
                    // Fixing the last reference frame had none pts issue (For MXF etc).
                    // We only do this when
                    // 1. eof.
                    // 2. we are not able to resolve a pts value for current packet.
                    // 3. the packets for this stream at the end of the files had valid dts.
                    next_pkt->pts = last_dts + next_pkt->duration;
                }
                pktl = s->internal->packet_buffer;
            }

            /* read packet from packet buffer, if there is data */
            st = s->streams[next_pkt->stream_index];
            if (!(next_pkt->pts == AV_NOPTS_VALUE && st->discard < AVDISCARD_ALL &&
                  next_pkt->dts != AV_NOPTS_VALUE && !eof)) {
                ret = avpriv_packet_list_get(&s->internal->packet_buffer,
                                               &s->internal->packet_buffer_end, pkt);
                goto return_packet;
            }
        }

        ret = read_frame_internal(s, pkt);
        if (ret < 0) {
            if (pktl && ret != AVERROR(EAGAIN)) {
                eof = 1;
                continue;
            } else
                return ret;
        }

        ret = avpriv_packet_list_put(&s->internal->packet_buffer,
                                 &s->internal->packet_buffer_end,
                                 pkt, NULL, 0);
        if (ret < 0) {
            av_packet_unref(pkt);
            return ret;
        }
    }

return_packet:

    st = s->streams[pkt->stream_index];
    if ((s->iformat->flags & AVFMT_GENERIC_INDEX) && pkt->flags & AV_PKT_FLAG_KEY) {
        ff_reduce_index(s, st->index);
        av_add_index_entry(st, pkt->pos, pkt->dts, 0, 0, AVINDEX_KEYFRAME);
    }

    if (is_relative(pkt->dts))
        pkt->dts -= RELATIVE_TS_BASE;
    if (is_relative(pkt->pts))
        pkt->pts -= RELATIVE_TS_BASE;

    return ret;
}

static int read_thread(void *arg)
{
    VideoState *is = arg;
    AVFormatContext *ic = NULL;
    int err, i, ret;
    int st_index[AVMEDIA_TYPE_NB];
    AVPacket pkt1, *pkt = &pkt1;
    int64_t stream_start_time;
    int pkt_in_play_range = 0;
    AVDictionaryEntry *t;
    SDL_mutex *wait_mutex = SDL_CreateMutex();
    int scan_all_pmts_set = 0;
    int64_t pkt_ts;

    if (!wait_mutex)
    {
        av_log(NULL, AV_LOG_FATAL, "SDL_CreateMutex(): %s\n", SDL_GetError());
        ret = AVERROR(ENOMEM);
        goto fail;
    }

    memset(st_index, -1, sizeof(st_index));
    is->eof = 0;

    ic = avformat_alloc_context();
    if (!ic)
    {
        av_log(NULL, AV_LOG_FATAL, "Could not allocate context.\n");
        ret = AVERROR(ENOMEM);
        goto fail;
    }
    ic->interrupt_callback.callback = decode_interrupt_cb;
    ic->interrupt_callback.opaque = is;
    if (!av_dict_get(format_opts, "scan_all_pmts", NULL, AV_DICT_MATCH_CASE))
    {
        av_dict_set(&format_opts, "scan_all_pmts", "1", AV_DICT_DONT_OVERWRITE);
        scan_all_pmts_set = 1;
    }
    err = avformat_open_input(&ic, is->filename, is->iformat, &format_opts);
    if (err < 0)
    {
        print_error(is->filename, err);
        ret = -1;
        goto fail;
    }
    if (scan_all_pmts_set)
        av_dict_set(&format_opts, "scan_all_pmts", NULL, AV_DICT_MATCH_CASE);

    if ((t = av_dict_get(format_opts, "", NULL, AV_DICT_IGNORE_SUFFIX)))
    {
        av_log(NULL, AV_LOG_ERROR, "Option %s not found.\n", t->key);
        ret = AVERROR_OPTION_NOT_FOUND;
        goto fail;
    }
    is->ic = ic;

    if (genpts)
        ic->flags |= AVFMT_FLAG_GENPTS;

    av_format_inject_global_side_data(ic);

    if (find_stream_info)
    {
        AVDictionary **opts = setup_find_stream_info_opts(ic, codec_opts);
        int orig_nb_streams = ic->nb_streams;

        err = avformat_find_stream_info(ic, opts);

        for (i = 0; i < orig_nb_streams; i++)
            av_dict_free(&opts[i]);
        av_freep(&opts);

        if (err < 0)
        {
            av_log(NULL, AV_LOG_WARNING,
                   "%s: could not find codec parameters\n", is->filename);
            ret = -1;
            goto fail;
        }
    }

    if (ic->pb)
        ic->pb->eof_reached = 0; // FIXME hack, ffplay maybe should not use avio_feof() to test for the end

    if (seek_by_bytes < 0)
        seek_by_bytes = !!(ic->iformat->flags & AVFMT_TS_DISCONT) && strcmp("ogg", ic->iformat->name);

    is->max_frame_duration = (ic->iformat->flags & AVFMT_TS_DISCONT) ? 10.0 : 3600.0;

    if (!window_title && (t = av_dict_get(ic->metadata, "title", NULL, 0)))
        window_title = av_asprintf("%s - %s", t->value, input_filename);

    /* if seeking requested, we execute it */
    if (start_time != AV_NOPTS_VALUE)
    {
        int64_t timestamp;

        timestamp = start_time;
        /* add the stream start time */
        if (ic->start_time != AV_NOPTS_VALUE)
            timestamp += ic->start_time;
        ret = avformat_seek_file(ic, -1, INT64_MIN, timestamp, INT64_MAX, 0);
        if (ret < 0)
        {
            av_log(NULL, AV_LOG_WARNING, "%s: could not seek to position %0.3f\n",
                   is->filename, (double)timestamp / AV_TIME_BASE);
        }
    }

    is->realtime = is_realtime(ic);

    if (show_status)
        av_dump_format(ic, 0, is->filename, 0);

    for (i = 0; i < ic->nb_streams; i++)
    {
        AVStream *st = ic->streams[i];
        enum AVMediaType type = st->codecpar->codec_type;
        st->discard = AVDISCARD_ALL;
        if (type >= 0 && wanted_stream_spec[type] && st_index[type] == -1)
            if (avformat_match_stream_specifier(ic, st, wanted_stream_spec[type]) > 0)
                st_index[type] = i;
    }
    for (i = 0; i < AVMEDIA_TYPE_NB; i++)
    {
        if (wanted_stream_spec[i] && st_index[i] == -1)
        {
            av_log(NULL, AV_LOG_ERROR, "Stream specifier %s does not match any %s stream\n", wanted_stream_spec[i], av_get_media_type_string(i));
            st_index[i] = INT_MAX;
        }
    }

    if (!video_disable)
        st_index[AVMEDIA_TYPE_VIDEO] =
            av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO,
                                st_index[AVMEDIA_TYPE_VIDEO], -1, NULL, 0);
    if (!audio_disable)
        st_index[AVMEDIA_TYPE_AUDIO] =
            av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO,
                                st_index[AVMEDIA_TYPE_AUDIO],
                                st_index[AVMEDIA_TYPE_VIDEO],
                                NULL, 0);
    if (!video_disable && !subtitle_disable)
        st_index[AVMEDIA_TYPE_SUBTITLE] =
            av_find_best_stream(ic, AVMEDIA_TYPE_SUBTITLE,
                                st_index[AVMEDIA_TYPE_SUBTITLE],
                                (st_index[AVMEDIA_TYPE_AUDIO] >= 0 ? st_index[AVMEDIA_TYPE_AUDIO] : st_index[AVMEDIA_TYPE_VIDEO]),
                                NULL, 0);

    is->show_mode = show_mode;
    if (st_index[AVMEDIA_TYPE_VIDEO] >= 0)
    {
        AVStream *st = ic->streams[st_index[AVMEDIA_TYPE_VIDEO]];
        AVCodecParameters *codecpar = st->codecpar;
        AVRational sar = av_guess_sample_aspect_ratio(ic, st, NULL);
        if (codecpar->width)
            set_default_window_size(codecpar->width, codecpar->height, sar);
    }

    /* open the streams */
    if (st_index[AVMEDIA_TYPE_AUDIO] >= 0)
    {
        stream_component_open(is, st_index[AVMEDIA_TYPE_AUDIO]);
    }

    ret = -1;
    if (st_index[AVMEDIA_TYPE_VIDEO] >= 0)
    {
        ret = stream_component_open(is, st_index[AVMEDIA_TYPE_VIDEO]);
    }
    if (is->show_mode == SHOW_MODE_NONE)
        is->show_mode = ret >= 0 ? SHOW_MODE_VIDEO : SHOW_MODE_RDFT;

    if (st_index[AVMEDIA_TYPE_SUBTITLE] >= 0)
    {
        stream_component_open(is, st_index[AVMEDIA_TYPE_SUBTITLE]);
    }

    if (is->video_stream < 0 && is->audio_stream < 0)
    {
        av_log(NULL, AV_LOG_FATAL, "Failed to open file '%s' or configure filtergraph\n",
               is->filename);
        ret = -1;
        goto fail;
    }

    if (infinite_buffer < 0 && is->realtime)
        infinite_buffer = 1;

    for (;;)
    {
        if (is->abort_request)
            break;
        if (is->paused != is->last_paused)
        {
            is->last_paused = is->paused;
            if (is->paused)
                is->read_pause_return = av_read_pause(ic);
            else
                av_read_play(ic);
        }
        // #if CONFIG_RTSP_DEMUXER || CONFIG_MMSH_PROTOCOL
        if (is->paused &&
            (!strcmp(ic->iformat->name, "rtsp") ||
             (ic->pb && !strncmp(input_filename, "mmsh:", 5))))
        {
            /* wait 10 ms to avoid trying to get another packet */
            /* XXX: horrible */
            SDL_Delay(10);
            continue;
        }
        // #endif
        if (is->seek_req)
        {
            int64_t seek_target = is->seek_pos;
            int64_t seek_min = is->seek_rel > 0 ? seek_target - is->seek_rel + 2 : INT64_MIN;
            int64_t seek_max = is->seek_rel < 0 ? seek_target - is->seek_rel - 2 : INT64_MAX;
            // FIXME the +-2 is due to rounding being not done in the correct direction in generation
            //      of the seek_pos/seek_rel variables

            ret = avformat_seek_file(is->ic, -1, seek_min, seek_target, seek_max, is->seek_flags);
            if (ret < 0)
            {
                av_log(NULL, AV_LOG_ERROR,
                       "%s: error while seeking\n", is->ic->url);
            }
            else
            {
                if (is->audio_stream >= 0)
                {
                    packet_queue_flush(&is->audioq);
                    packet_queue_put(&is->audioq, &flush_pkt);
                }
                if (is->subtitle_stream >= 0)
                {
                    packet_queue_flush(&is->subtitleq);
                    packet_queue_put(&is->subtitleq, &flush_pkt);
                }
                if (is->video_stream >= 0)
                {
                    packet_queue_flush(&is->videoq);
                    packet_queue_put(&is->videoq, &flush_pkt);
                }
                if (is->seek_flags & AVSEEK_FLAG_BYTE)
                {
                    set_clock(&is->extclk, NAN, 0);
                }
                else
                {
                    set_clock(&is->extclk, seek_target / (double)AV_TIME_BASE, 0);
                }
            }
            is->seek_req = 0;
            is->queue_attachments_req = 1;
            is->eof = 0;
            if (is->paused)
                step_to_next_frame(is);
        }
        if (is->queue_attachments_req)
        {
            if (is->video_st && is->video_st->disposition & AV_DISPOSITION_ATTACHED_PIC)
            {
                AVPacket copy;
                if ((ret = av_packet_ref(&copy, &is->video_st->attached_pic)) < 0)
                    goto fail;
                packet_queue_put(&is->videoq, &copy);
                packet_queue_put_nullpacket(&is->videoq, is->video_stream);
            }
            is->queue_attachments_req = 0;
        }

        /* if the queue are full, no need to read more */
        if (infinite_buffer < 1 &&
            (is->audioq.size + is->videoq.size + is->subtitleq.size > MAX_QUEUE_SIZE || (stream_has_enough_packets(is->audio_st, is->audio_stream, &is->audioq) &&
                                                                                         stream_has_enough_packets(is->video_st, is->video_stream, &is->videoq) &&
                                                                                         stream_has_enough_packets(is->subtitle_st, is->subtitle_stream, &is->subtitleq))))
        {
            /* wait 10 ms */
            SDL_LockMutex(wait_mutex);
            SDL_CondWaitTimeout(is->continue_read_thread, wait_mutex, 10);
            SDL_UnlockMutex(wait_mutex);
            continue;
        }
        if (!is->paused &&
            (!is->audio_st || (is->auddec.finished == is->audioq.serial && frame_queue_nb_remaining(&is->sampq) == 0)) &&
            (!is->video_st || (is->viddec.finished == is->videoq.serial && frame_queue_nb_remaining(&is->pictq) == 0)))
        {
            if (loop != 1 && (!loop || --loop))
            {
                stream_seek(is, start_time != AV_NOPTS_VALUE ? start_time : 0, 0, 0);
            }
            else if (autoexit)
            {
                ret = AVERROR_EOF;
                goto fail;
            }
        }
        ret = av_read_frame(ic, pkt);
        if (ret < 0)
        {
            if ((ret == AVERROR_EOF || avio_feof(ic->pb)) && !is->eof)
            {
                if (is->video_stream >= 0)
                    packet_queue_put_nullpacket(&is->videoq, is->video_stream);
                if (is->audio_stream >= 0)
                    packet_queue_put_nullpacket(&is->audioq, is->audio_stream);
                if (is->subtitle_stream >= 0)
                    packet_queue_put_nullpacket(&is->subtitleq, is->subtitle_stream);
                is->eof = 1;
            }
            if (ic->pb && ic->pb->error)
            {
                if (autoexit)
                    goto fail;
                else
                    break;
            }
            SDL_LockMutex(wait_mutex);
            SDL_CondWaitTimeout(is->continue_read_thread, wait_mutex, 10);
            SDL_UnlockMutex(wait_mutex);
            continue;
        }
        else
        {
            is->eof = 0;
        }
        /* check if packet is in play range specified by user, then queue, otherwise discard */
        stream_start_time = ic->streams[pkt->stream_index]->start_time;
        pkt_ts = pkt->pts == AV_NOPTS_VALUE ? pkt->dts : pkt->pts;
        pkt_in_play_range = duration == AV_NOPTS_VALUE ||
                            (pkt_ts - (stream_start_time != AV_NOPTS_VALUE ? stream_start_time : 0)) *
                                        av_q2d(ic->streams[pkt->stream_index]->time_base) -
                                    (double)(start_time != AV_NOPTS_VALUE ? start_time : 0) / 1000000 <=
                                ((double)duration / 1000000);
        if (pkt->stream_index == is->audio_stream && pkt_in_play_range)
        {
            packet_queue_put(&is->audioq, pkt);
        }
        else if (pkt->stream_index == is->video_stream && pkt_in_play_range && !(is->video_st->disposition & AV_DISPOSITION_ATTACHED_PIC))
        {
            packet_queue_put(&is->videoq, pkt);
        }
        else if (pkt->stream_index == is->subtitle_stream && pkt_in_play_range)
        {
            packet_queue_put(&is->subtitleq, pkt);
        }
        else
        {
            av_packet_unref(pkt);
        }
    }

    ret = 0;
fail:
    if (ic && !is->ic)
        avformat_close_input(&ic);

    if (ret != 0)
    {
        SDL_Event event;

        event.type = FF_QUIT_EVENT;
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
        return NULL;
    is->last_video_stream = is->video_stream = -1;
    is->last_audio_stream = is->audio_stream = -1;
    is->last_subtitle_stream = is->subtitle_stream = -1;
    is->filename = av_strdup(filename);
    if (!is->filename)
        goto fail;
    is->iformat = iformat;
    is->ytop = 0;
    is->xleft = 0;

    /* start video display */
    if (frame_queue_init(&is->pictq, &is->videoq, VIDEO_PICTURE_QUEUE_SIZE, 1) < 0)
        goto fail;
    if (frame_queue_init(&is->subpq, &is->subtitleq, SUBPICTURE_QUEUE_SIZE, 0) < 0)
        goto fail;
    if (frame_queue_init(&is->sampq, &is->audioq, SAMPLE_QUEUE_SIZE, 1) < 0)
        goto fail;

    if (packet_queue_init(&is->videoq) < 0 ||
        packet_queue_init(&is->audioq) < 0 ||
        packet_queue_init(&is->subtitleq) < 0)
        goto fail;

    if (!(is->continue_read_thread = SDL_CreateCond()))
    {
        av_log(NULL, AV_LOG_FATAL, "SDL_CreateCond(): %s\n", SDL_GetError());
        goto fail;
    }

    init_clock(&is->vidclk, &is->videoq.serial);
    init_clock(&is->audclk, &is->audioq.serial);
    init_clock(&is->extclk, &is->extclk.serial);
    is->audio_clock_serial = -1;
    if (startup_volume < 0)
        av_log(NULL, AV_LOG_WARNING, "-volume=%d < 0, setting to 0\n", startup_volume);
    if (startup_volume > 100)
        av_log(NULL, AV_LOG_WARNING, "-volume=%d > 100, setting to 100\n", startup_volume);
    startup_volume = av_clip(startup_volume, 0, 100);
    startup_volume = av_clip(SDL_MIX_MAXVOLUME * startup_volume / 100, 0, SDL_MIX_MAXVOLUME);
    is->audio_volume = startup_volume;
    is->muted = 0;
    is->av_sync_type = av_sync_type;
    is->read_tid = SDL_CreateThread(read_thread, "read_thread", is);
    if (!is->read_tid)
    {
        av_log(NULL, AV_LOG_FATAL, "SDL_CreateThread(): %s\n", SDL_GetError());
    fail:
        stream_close(is);
        return NULL;
    }
    return is;
}

static void stream_cycle_channel(VideoState *is, int codec_type)
{
    AVFormatContext *ic = is->ic;
    int start_index, stream_index;
    int old_index;
    AVStream *st;
    AVProgram *p = NULL;
    int nb_streams = is->ic->nb_streams;

    if (codec_type == AVMEDIA_TYPE_VIDEO)
    {
        start_index = is->last_video_stream;
        old_index = is->video_stream;
    }
    else if (codec_type == AVMEDIA_TYPE_AUDIO)
    {
        start_index = is->last_audio_stream;
        old_index = is->audio_stream;
    }
    else
    {
        start_index = is->last_subtitle_stream;
        old_index = is->subtitle_stream;
    }
    stream_index = start_index;

    if (codec_type != AVMEDIA_TYPE_VIDEO && is->video_stream != -1)
    {
        p = av_find_program_from_stream(ic, NULL, is->video_stream);
        if (p)
        {
            nb_streams = p->nb_stream_indexes;
            for (start_index = 0; start_index < nb_streams; start_index++)
                if (p->stream_index[start_index] == stream_index)
                    break;
            if (start_index == nb_streams)
                start_index = -1;
            stream_index = start_index;
        }
    }

    for (;;)
    {
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
        if (st->codecpar->codec_type == codec_type)
        {
            /* check that parameters are OK */
            switch (codec_type)
            {
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
    av_log(NULL, AV_LOG_INFO, "Switch %s stream from #%d to #%d\n",
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
    do
    {
        next = (next + 1) % SHOW_MODE_NB;
    } while (next != is->show_mode && (next == SHOW_MODE_VIDEO && !is->video_st || next != SHOW_MODE_VIDEO && !is->audio_st));
    if (is->show_mode != next)
    {
        is->force_refresh = 1;
        is->show_mode = next;
    }
}

static void refresh_loop_wait_event(VideoState *is, SDL_Event *event)
{
    double remaining_time = 0.0;
    SDL_PumpEvents();
    while (!SDL_PeepEvents(event, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT))
    {
        if (!cursor_hidden && av_gettime_relative() - cursor_last_shown > CURSOR_HIDE_DELAY)
        {
            SDL_ShowCursor(0);
            cursor_hidden = 1;
        }
        if (remaining_time > 0.0)
            av_usleep((int64_t)(remaining_time * 1000000.0));
        remaining_time = REFRESH_RATE;
        if (is->show_mode != SHOW_MODE_NONE && (!is->paused || is->force_refresh))
            video_refresh(is, &remaining_time);
        SDL_PumpEvents();
    }
}

int av_compare_ts(int64_t ts_a, AVRational tb_a, int64_t ts_b, AVRational tb_b)
{
    int64_t a = tb_a.num * (int64_t)tb_b.den;
    int64_t b = tb_b.num * (int64_t)tb_a.den;
    if ((FFABS(ts_a)|a|FFABS(ts_b)|b) <= INT_MAX)
        return (ts_a*a > ts_b*b) - (ts_a*a < ts_b*b);
    if (av_rescale_rnd(ts_a, a, b, AV_ROUND_DOWN) < ts_b)
        return -1;
    if (av_rescale_rnd(ts_b, b, a, AV_ROUND_DOWN) < ts_a)
        return 1;
    return 0;
}

static void seek_chapter(VideoState *is, int incr)
{
    int64_t pos = get_master_clock(is) * AV_TIME_BASE;
    int i;

    if (!is->ic->nb_chapters)
        return;

    /* find the current chapter */
    for (i = 0; i < is->ic->nb_chapters; i++)
    {
        AVChapter *ch = is->ic->chapters[i];
        if (av_compare_ts(pos, AV_TIME_BASE_Q, ch->start, ch->time_base) < 0)
        {
            i--;
            break;
        }
    }

    i += incr;
    i = FFMAX(i, 0);
    if (i >= is->ic->nb_chapters)
        return;

    av_log(NULL, AV_LOG_VERBOSE, "Seeking to chapter %d.\n", i);
    stream_seek(is, av_rescale_q(is->ic->chapters[i]->start, is->ic->chapters[i]->time_base, AV_TIME_BASE_Q), 0, 0);
}

int64_t avio_size(AVIOContext *s)
{
    int64_t size;

    if (!s)
        return AVERROR(EINVAL);

    if (s->written)
        return s->written;

    if (!s->seek)
        return AVERROR(ENOSYS);
    size = s->seek(s->opaque, 0, AVSEEK_SIZE);
    if (size < 0) {
        if ((size = s->seek(s->opaque, -1, SEEK_END)) < 0)
            return size;
        size++;
        s->seek(s->opaque, s->pos, SEEK_SET);
    }
    return size;
}


double av_strtod(const char *numstr, char **tail)
{
    double d;
    char *next;
    if(numstr[0]=='0' && (numstr[1]|0x20)=='x') {
        d = strtoul(numstr, &next, 16);
    } else
        d = strtod(numstr, &next);
    /* if parsing succeeded, check for and interpret postfixes */
    if (next!=numstr) {
        if (next[0] == 'd' && next[1] == 'B') {
            /* treat dB as decibels instead of decibytes */
            d = ff_exp10(d / 20);
            next += 2;
        } else if (*next >= 'E' && *next <= 'z') {
            int e= si_prefixes[*next - 'E'].exp;
            if (e) {
                if (next[1] == 'i') {
                    d*= si_prefixes[*next - 'E'].bin_val;
                    next+=2;
                } else {
                    d*= si_prefixes[*next - 'E'].dec_val;
                    next++;
                }
            }
        }

        if (*next=='B') {
            d*=8;
            next++;
        }
    }
    /* if requested, fill in tail with the position after the last parsed
       character */
    if (tail)
        *tail = next;
    return d;
}


static av_always_inline int64_t avio_tell(AVIOContext *s)
{
    return avio_seek(s, 0, SEEK_CUR);
}

/* handle an event sent by the GUI */
static void event_loop(VideoState *cur_stream)
{
    SDL_Event event;
    double incr, pos, frac;

    for (;;)
    {
        double x;
        refresh_loop_wait_event(cur_stream, &event);
        switch (event.type)
        {
        case SDL_KEYDOWN:
            if (exit_on_keydown || event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_q)
            {
                do_exit(cur_stream);
                break;
            }
            // If we don't yet have a window, skip all key events, because read_thread might still be initializing...
            if (!cur_stream->width)
                continue;
            switch (event.key.keysym.sym)
            {
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
                update_volume(cur_stream, 1, SDL_VOLUME_STEP);
                break;
            case SDLK_KP_DIVIDE:
            case SDLK_9:
                update_volume(cur_stream, -1, SDL_VOLUME_STEP);
                break;
            case SDLK_s: // S: Step to next frame
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
                // #if CONFIG_AVFILTER
                if (cur_stream->show_mode == SHOW_MODE_VIDEO && cur_stream->vfilter_idx < nb_vfilters - 1)
                {
                    if (++cur_stream->vfilter_idx >= nb_vfilters)
                        cur_stream->vfilter_idx = 0;
                }
                else
                {
                    cur_stream->vfilter_idx = 0;
                    toggle_audio_display(cur_stream);
                }
                // #else
                // toggle_audio_display(cur_stream);
                // #endif
                break;
            case SDLK_PAGEUP:
                if (cur_stream->ic->nb_chapters <= 1)
                {
                    incr = 600.0;
                    goto do_seek;
                }
                seek_chapter(cur_stream, 1);
                break;
            case SDLK_PAGEDOWN:
                if (cur_stream->ic->nb_chapters <= 1)
                {
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
                if (seek_by_bytes)
                {
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
                }
                else
                {
                    pos = get_master_clock(cur_stream);
                    if (isnan(pos))
                        pos = (double)cur_stream->seek_pos / AV_TIME_BASE;
                    pos += incr;
                    if (cur_stream->ic->start_time != AV_NOPTS_VALUE && pos < cur_stream->ic->start_time / (double)AV_TIME_BASE)
                        pos = cur_stream->ic->start_time / (double)AV_TIME_BASE;
                    stream_seek(cur_stream, (int64_t)(pos * AV_TIME_BASE), (int64_t)(incr * AV_TIME_BASE), 0);
                }
                break;
            default:
                break;
            }
            break;
        case SDL_MOUSEBUTTONDOWN:
            if (exit_on_mousedown)
            {
                do_exit(cur_stream);
                break;
            }
            if (event.button.button == SDL_BUTTON_LEFT)
            {
                static int64_t last_mouse_left_click = 0;
                if (av_gettime_relative() - last_mouse_left_click <= 500000)
                {
                    toggle_full_screen(cur_stream);
                    cur_stream->force_refresh = 1;
                    last_mouse_left_click = 0;
                }
                else
                {
                    last_mouse_left_click = av_gettime_relative();
                }
            }
        case SDL_MOUSEMOTION:
            if (cursor_hidden)
            {
                SDL_ShowCursor(1);
                cursor_hidden = 0;
            }
            cursor_last_shown = av_gettime_relative();
            if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                if (event.button.button != SDL_BUTTON_RIGHT)
                    break;
                x = event.button.x;
            }
            else
            {
                if (!(event.motion.state & SDL_BUTTON_RMASK))
                    break;
                x = event.motion.x;
            }
            if (seek_by_bytes || cur_stream->ic->duration <= 0)
            {
                uint64_t size = avio_size(cur_stream->ic->pb);
                stream_seek(cur_stream, size * x / cur_stream->width, 0, 1);
            }
            else
            {
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
                av_log(NULL, AV_LOG_INFO,
                       "Seek to %2.0f%% (%2d:%02d:%02d) of total duration (%2d:%02d:%02d)       \n", frac * 100,
                       hh, mm, ss, thh, tmm, tss);
                ts = frac * cur_stream->ic->duration;
                if (cur_stream->ic->start_time != AV_NOPTS_VALUE)
                    ts += cur_stream->ic->start_time;
                stream_seek(cur_stream, ts, 0, 0);
            }
            break;
        case SDL_WINDOWEVENT:
            switch (event.window.event)
            {
            case SDL_WINDOWEVENT_SIZE_CHANGED:
                screen_width = cur_stream->width = event.window.data1;
                screen_height = cur_stream->height = event.window.data2;
                if (cur_stream->vis_texture)
                {
                    SDL_DestroyTexture(cur_stream->vis_texture);
                    cur_stream->vis_texture = NULL;
                }
            case SDL_WINDOWEVENT_EXPOSED:
                cur_stream->force_refresh = 1;
            }
            break;
        case SDL_QUIT:
        case FF_QUIT_EVENT:
            do_exit(cur_stream);
            break;
        default:
            break;
        }
    }
}

static int opt_frame_size(void *optctx, const char *opt, const char *arg)
{
    av_log(NULL, AV_LOG_WARNING, "Option -s is deprecated, use -video_size.\n");
    return opt_default(NULL, "video_size", arg);
}

double parse_number_or_die(const char *context, const char *numstr, int type,
                           double min, double max)
{
    char *tail;
    const char *error;
    double d = av_strtod(numstr, &tail);
    if (*tail)
        error = "Expected number for %s but found: %s\n";
    else if (d < min || d > max)
        error = "The value for %s was %s which is not within %f - %f\n";
    else if (type == OPT_INT64 && (int64_t)d != d)
        error = "Expected int64 for %s but found %s\n";
    else if (type == OPT_INT && (int)d != d)
        error = "Expected int for %s but found %s\n";
    else
        return d;
    av_log(NULL, AV_LOG_FATAL, error, context, numstr, min, max);
    exit_program(1);
    return 0;
}

static int opt_width(void *optctx, const char *opt, const char *arg)
{
    screen_width = parse_number_or_die(opt, arg, OPT_INT64, 1, INT_MAX);
    return 0;
}

static int opt_height(void *optctx, const char *opt, const char *arg)
{
    screen_height = parse_number_or_die(opt, arg, OPT_INT64, 1, INT_MAX);
    return 0;
}

const AVInputFormat *av_demuxer_iterate(void **opaque)
{
    static const uintptr_t size = sizeof(demuxer_list)/sizeof(demuxer_list[0]) - 1;
    uintptr_t i = (uintptr_t)*opaque;
    const AVInputFormat *f = NULL;

    if (i < size) {
        f = demuxer_list[i];
    } else if (outdev_list) {
        f = indev_list[i - size];
    }

    if (f)
        *opaque = (void*)(i + 1);
    return f;
}

ff_const59 AVInputFormat *av_find_input_format(const char *short_name)
{
    const AVInputFormat *fmt = NULL;
    void *i = 0;
    while ((fmt = av_demuxer_iterate(&i)))
        if (av_match_name(short_name, fmt->name))
            return (AVInputFormat*)fmt;
    return NULL;
}


static int opt_format(void *optctx, const char *opt, const char *arg)
{
    file_iformat = av_find_input_format(arg);
    if (!file_iformat)
    {
        av_log(NULL, AV_LOG_FATAL, "Unknown input format: %s\n", arg);
        return AVERROR(EINVAL);
    }
    return 0;
}

static int opt_frame_pix_fmt(void *optctx, const char *opt, const char *arg)
{
    av_log(NULL, AV_LOG_WARNING, "Option -pix_fmt is deprecated, use -pixel_format.\n");
    return opt_default(NULL, "pixel_format", arg);
}

static int opt_sync(void *optctx, const char *opt, const char *arg)
{
    if (!strcmp(arg, "audio"))
        av_sync_type = AV_SYNC_AUDIO_MASTER;
    else if (!strcmp(arg, "video"))
        av_sync_type = AV_SYNC_VIDEO_MASTER;
    else if (!strcmp(arg, "ext"))
        av_sync_type = AV_SYNC_EXTERNAL_CLOCK;
    else
    {
        av_log(NULL, AV_LOG_ERROR, "Unknown value for %s: %s\n", opt, arg);
        exit(1);
    }
    return 0;
}

int av_parse_time(int64_t *timeval, const char *timestr, int duration)
{
    const char *p, *q;
    int64_t t, now64;
    time_t now;
    struct tm dt = { 0 }, tmbuf;
    int today = 0, negative = 0, microseconds = 0, suffix = 1000000;
    int i;
    static const char * const date_fmt[] = {
        "%Y - %m - %d",
        "%Y%m%d",
    };
    static const char * const time_fmt[] = {
        "%H:%M:%S",
        "%H%M%S",
    };
    static const char * const tz_fmt[] = {
        "%H:%M",
        "%H%M",
        "%H",
    };

    p = timestr;
    q = NULL;
    *timeval = INT64_MIN;
    if (!duration) {
        now64 = av_gettime();
        now = now64 / 1000000;

        if (!av_strcasecmp(timestr, "now")) {
            *timeval = now64;
            return 0;
        }

        /* parse the year-month-day part */
        for (i = 0; i < FF_ARRAY_ELEMS(date_fmt); i++) {
            q = av_small_strptime(p, date_fmt[i], &dt);
            if (q)
                break;
        }

        /* if the year-month-day part is missing, then take the
         * current year-month-day time */
        if (!q) {
            today = 1;
            q = p;
        }
        p = q;

        if (*p == 'T' || *p == 't')
            p++;
        else
            while (av_isspace(*p))
                p++;

        /* parse the hour-minute-second part */
        for (i = 0; i < FF_ARRAY_ELEMS(time_fmt); i++) {
            q = av_small_strptime(p, time_fmt[i], &dt);
            if (q)
                break;
        }
    } else {
        /* parse timestr as a duration */
        if (p[0] == '-') {
            negative = 1;
            ++p;
        }
        /* parse timestr as HH:MM:SS */
        q = av_small_strptime(p, "%J:%M:%S", &dt);
        if (!q) {
            /* parse timestr as MM:SS */
            q = av_small_strptime(p, "%M:%S", &dt);
            dt.tm_hour = 0;
        }
        if (!q) {
            char *o;
            /* parse timestr as S+ */
            errno = 0;
            t = strtoll(p, &o, 10);
            if (o == p) /* the parsing didn't succeed */
                return AVERROR(EINVAL);
            if (errno == ERANGE)
                return AVERROR(ERANGE);
            q = o;
        } else {
            t = dt.tm_hour * 3600 + dt.tm_min * 60 + dt.tm_sec;
        }
    }

    /* Now we have all the fields that we can get */
    if (!q)
        return AVERROR(EINVAL);

    /* parse the .m... part */
    if (*q == '.') {
        int n;
        q++;
        for (n = 100000; n >= 1; n /= 10, q++) {
            if (!av_isdigit(*q))
                break;
            microseconds += n * (*q - '0');
        }
        while (av_isdigit(*q))
            q++;
    }

    if (duration) {
        if (q[0] == 'm' && q[1] == 's') {
            suffix = 1000;
            microseconds /= 1000;
            q += 2;
        } else if (q[0] == 'u' && q[1] == 's') {
            suffix = 1;
            microseconds = 0;
            q += 2;
        } else if (*q == 's')
            q++;
    } else {
        int is_utc = *q == 'Z' || *q == 'z';
        int tzoffset = 0;
        q += is_utc;
        if (!today && !is_utc && (*q == '+' || *q == '-')) {
            struct tm tz = { 0 };
            int sign = (*q == '+' ? -1 : 1);
            q++;
            p = q;
            for (i = 0; i < FF_ARRAY_ELEMS(tz_fmt); i++) {
                q = av_small_strptime(p, tz_fmt[i], &tz);
                if (q)
                    break;
            }
            if (!q)
                return AVERROR(EINVAL);
            tzoffset = sign * (tz.tm_hour * 60 + tz.tm_min) * 60;
            is_utc = 1;
        }
        if (today) { /* fill in today's date */
            struct tm dt2 = is_utc ? *gmtime_r(&now, &tmbuf) : *localtime_r(&now, &tmbuf);
            dt2.tm_hour = dt.tm_hour;
            dt2.tm_min  = dt.tm_min;
            dt2.tm_sec  = dt.tm_sec;
            dt = dt2;
        }
        dt.tm_isdst = is_utc ? 0 : -1;
        t = is_utc ? av_timegm(&dt) : mktime(&dt);
        t += tzoffset;
    }

    /* Check that we are at the end of the string */
    if (*q)
        return AVERROR(EINVAL);

    if (INT64_MAX / suffix < t)
        return AVERROR(ERANGE);
    t *= suffix;
    if (INT64_MAX - microseconds < t)
        return AVERROR(ERANGE);
    t += microseconds;
    *timeval = negative ? -t : t;
    return 0;
}

int64_t parse_time_or_die(const char *context, const char *timestr,
                          int is_duration)
{
    int64_t us;
    if (av_parse_time(&us, timestr, is_duration) < 0)
    {
        av_log(NULL, AV_LOG_FATAL, "Invalid %s specification for %s: %s\n",
               is_duration ? "duration" : "date", context, timestr);
        exit_program(1);
    }
    return us;
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
    show_mode = !strcmp(arg, "video") ? SHOW_MODE_VIDEO : !strcmp(arg, "waves") ? SHOW_MODE_WAVES : !strcmp(arg, "rdft") ? SHOW_MODE_RDFT : parse_number_or_die(opt, arg, OPT_INT, 0, SHOW_MODE_NB - 1);
    return 0;
}

static void opt_input_file(void *optctx, const char *filename)
{
    if (input_filename)
    {
        av_log(NULL, AV_LOG_FATAL,
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
    if (!spec)
    {
        av_log(NULL, AV_LOG_ERROR,
               "No media specifier was specified in '%s' in option '%s'\n",
               arg, opt);
        return AVERROR(EINVAL);
    }
    spec++;
    switch (spec[0])
    {
    case 'a':
        audio_codec_name = arg;
        break;
    case 's':
        subtitle_codec_name = arg;
        break;
    case 'v':
        video_codec_name = arg;
        break;
    default:
        av_log(NULL, AV_LOG_ERROR,
               "Invalid media specifier '%s' in option '%s'\n", spec, opt);
        return AVERROR(EINVAL);
    }
    return 0;
}

static int dummy;
static const OptionDef options[] = {
    CMDUTILS_COMMON_OPTIONS{"x", HAS_ARG, {.func_arg = opt_width}, "force displayed width", "width"},
    {"y", HAS_ARG, {.func_arg = opt_height}, "force displayed height", "height"},
    {"s", HAS_ARG | OPT_VIDEO, {.func_arg = opt_frame_size}, "set frame size (WxH or abbreviation)", "size"},
    {"fs", OPT_BOOL, {&is_full_screen}, "force full screen"},
    {"an", OPT_BOOL, {&audio_disable}, "disable audio"},
    {"vn", OPT_BOOL, {&video_disable}, "disable video"},
    {"sn", OPT_BOOL, {&subtitle_disable}, "disable subtitling"},
    {"ast", OPT_STRING | HAS_ARG | OPT_EXPERT, {&wanted_stream_spec[AVMEDIA_TYPE_AUDIO]}, "select desired audio stream", "stream_specifier"},
    {"vst", OPT_STRING | HAS_ARG | OPT_EXPERT, {&wanted_stream_spec[AVMEDIA_TYPE_VIDEO]}, "select desired video stream", "stream_specifier"},
    {"sst", OPT_STRING | HAS_ARG | OPT_EXPERT, {&wanted_stream_spec[AVMEDIA_TYPE_SUBTITLE]}, "select desired subtitle stream", "stream_specifier"},
    {"ss", HAS_ARG, {.func_arg = opt_seek}, "seek to a given position in seconds", "pos"},
    {"t", HAS_ARG, {.func_arg = opt_duration}, "play  \"duration\" seconds of audio/video", "duration"},
    {"bytes", OPT_INT | HAS_ARG, {&seek_by_bytes}, "seek by bytes 0=off 1=on -1=auto", "val"},
    {"seek_interval", OPT_FLOAT | HAS_ARG, {&seek_interval}, "set seek interval for left/right keys, in seconds", "seconds"},
    {"nodisp", OPT_BOOL, {&display_disable}, "disable graphical display"},
    {"noborder", OPT_BOOL, {&borderless}, "borderless window"},
    {"alwaysontop", OPT_BOOL, {&alwaysontop}, "window always on top"},
    {"volume", OPT_INT | HAS_ARG, {&startup_volume}, "set startup volume 0=min 100=max", "volume"},
    {"f", HAS_ARG, {.func_arg = opt_format}, "force format", "fmt"},
    {"pix_fmt", HAS_ARG | OPT_EXPERT | OPT_VIDEO, {.func_arg = opt_frame_pix_fmt}, "set pixel format", "format"},
    {"stats", OPT_BOOL | OPT_EXPERT, {&show_status}, "show status", ""},
    {"fast", OPT_BOOL | OPT_EXPERT, {&fast}, "non spec compliant optimizations", ""},
    {"genpts", OPT_BOOL | OPT_EXPERT, {&genpts}, "generate pts", ""},
    {"drp", OPT_INT | HAS_ARG | OPT_EXPERT, {&decoder_reorder_pts}, "let decoder reorder pts 0=off 1=on -1=auto", ""},
    {"lowres", OPT_INT | HAS_ARG | OPT_EXPERT, {&lowres}, "", ""},
    {"sync", HAS_ARG | OPT_EXPERT, {.func_arg = opt_sync}, "set audio-video sync. type (type=audio/video/ext)", "type"},
    {"autoexit", OPT_BOOL | OPT_EXPERT, {&autoexit}, "exit at the end", ""},
    {"exitonkeydown", OPT_BOOL | OPT_EXPERT, {&exit_on_keydown}, "exit on key down", ""},
    {"exitonmousedown", OPT_BOOL | OPT_EXPERT, {&exit_on_mousedown}, "exit on mouse down", ""},
    {"loop", OPT_INT | HAS_ARG | OPT_EXPERT, {&loop}, "set number of times the playback shall be looped", "loop count"},
    {"framedrop", OPT_BOOL | OPT_EXPERT, {&framedrop}, "drop frames when cpu is too slow", ""},
    {"infbuf", OPT_BOOL | OPT_EXPERT, {&infinite_buffer}, "don't limit the input buffer size (useful with realtime streams)", ""},
    {"window_title", OPT_STRING | HAS_ARG, {&window_title}, "set window title", "window title"},
    {"left", OPT_INT | HAS_ARG | OPT_EXPERT, {&screen_left}, "set the x position for the left of the window", "x pos"},
    {"top", OPT_INT | HAS_ARG | OPT_EXPERT, {&screen_top}, "set the y position for the top of the window", "y pos"},
    {"vf", OPT_EXPERT | HAS_ARG, {.func_arg = opt_add_vfilter}, "set video filters", "filter_graph"},
    {"af", OPT_STRING | HAS_ARG, {&afilters}, "set audio filters", "filter_graph"},
    {"rdftspeed", OPT_INT | HAS_ARG | OPT_AUDIO | OPT_EXPERT, {&rdftspeed}, "rdft speed", "msecs"},
    {"showmode", HAS_ARG, {.func_arg = opt_show_mode}, "select show mode (0 = video, 1 = waves, 2 = RDFT)", "mode"},
    {"default", HAS_ARG | OPT_AUDIO | OPT_VIDEO | OPT_EXPERT, {.func_arg = opt_default}, "generic catch all option", ""},
    {"i", OPT_BOOL, {&dummy}, "read specified file", "input_file"},
    {"codec", HAS_ARG, {.func_arg = opt_codec}, "force decoder", "decoder_name"},
    {"acodec", HAS_ARG | OPT_STRING | OPT_EXPERT, {&audio_codec_name}, "force audio decoder", "decoder_name"},
    {"scodec", HAS_ARG | OPT_STRING | OPT_EXPERT, {&subtitle_codec_name}, "force subtitle decoder", "decoder_name"},
    {"vcodec", HAS_ARG | OPT_STRING | OPT_EXPERT, {&video_codec_name}, "force video decoder", "decoder_name"},
    {"autorotate", OPT_BOOL, {&autorotate}, "automatically rotate video", ""},
    {"find_stream_info", OPT_BOOL | OPT_INPUT | OPT_EXPERT, {&find_stream_info}, "read and decode the streams to fill missing information with heuristics"},
    {"filter_threads", HAS_ARG | OPT_INT | OPT_EXPERT, {&filter_nbthreads}, "number of filter threads per graph"},
    {
        NULL,
    },
};

void log_callback_help(void *ptr, int level, const char *fmt, va_list vl)
{
    vfprintf(stdout, fmt, vl);
}

static void show_help_demuxer(const char *name)
{
    const AVInputFormat *fmt = av_find_input_format(name);

    if (!fmt)
    {
        av_log(NULL, AV_LOG_ERROR, "Unknown format '%s'.\n", name);
        return;
    }

    printf("Demuxer %s [%s]:\n", fmt->name, fmt->long_name);

    if (fmt->extensions)
        printf("    Common extensions: %s.\n", fmt->extensions);

    if (fmt->priv_class)
        show_help_children(fmt->priv_class, AV_OPT_FLAG_DECODING_PARAM);
}

const AVClass *avio_protocol_get_class(const char *name)
{
    int i = 0;
    for (i = 0; url_protocols[i]; i++) {
        if (!strcmp(url_protocols[i]->name, name))
            return url_protocols[i]->priv_data_class;
    }
    return NULL;
}

static void show_help_protocol(const char *name)
{
    const AVClass *proto_class;

    if (!name)
    {
        av_log(NULL, AV_LOG_ERROR, "No protocol name specified.\n");
        return;
    }

    proto_class = avio_protocol_get_class(name);
    if (!proto_class)
    {
        av_log(NULL, AV_LOG_ERROR, "Unknown protocol '%s'.\n", name);
        return;
    }

    show_help_children(proto_class, AV_OPT_FLAG_DECODING_PARAM | AV_OPT_FLAG_ENCODING_PARAM);
}

static int descriptor_compare(const void *key, const void *member)
{
    enum AVCodecID id = *(const enum AVCodecID *)key;
    const AVCodecDescriptor *desc = member;

    return id - desc->id;
}

const AVCodecDescriptor *avcodec_descriptor_get(enum AVCodecID id)
{
    return bsearch(&id, codec_descriptors, FF_ARRAY_ELEMS(codec_descriptors),
                   sizeof(codec_descriptors[0]), descriptor_compare);
}

ff_const59 AVOutputFormat *av_guess_format(const char *short_name, const char *filename,
                                const char *mime_type)
{
    const AVOutputFormat *fmt = NULL;
    AVOutputFormat *fmt_found = NULL;
    void *i = 0;
    int score_max, score;

    /* specific test for image sequences */
#if CONFIG_IMAGE2_MUXER
    if (!short_name && filename &&
        av_filename_number_test(filename) &&
        ff_guess_image2_codec(filename) != AV_CODEC_ID_NONE) {
        return av_guess_format("image2", NULL, NULL);
    }
#endif
    /* Find the proper file type. */
    score_max = 0;
    while ((fmt = av_muxer_iterate(&i))) {
        score = 0;
        if (fmt->name && short_name && av_match_name(short_name, fmt->name))
            score += 100;
        if (fmt->mime_type && mime_type && !strcmp(fmt->mime_type, mime_type))
            score += 10;
        if (filename && fmt->extensions &&
            av_match_ext(filename, fmt->extensions)) {
            score += 5;
        }
        if (score > score_max) {
            score_max = score;
            fmt_found = (AVOutputFormat*)fmt;
        }
    }
    return fmt_found;
}

static void show_help_muxer(const char *name)
{
    const AVCodecDescriptor *desc;
    const AVOutputFormat *fmt = av_guess_format(name, NULL, NULL);

    if (!fmt)
    {
        av_log(NULL, AV_LOG_ERROR, "Unknown format '%s'.\n", name);
        return;
    }

    printf("Muxer %s [%s]:\n", fmt->name, fmt->long_name);

    if (fmt->extensions)
        printf("    Common extensions: %s.\n", fmt->extensions);
    if (fmt->mime_type)
        printf("    Mime type: %s.\n", fmt->mime_type);
    if (fmt->video_codec != AV_CODEC_ID_NONE &&
        (desc = avcodec_descriptor_get(fmt->video_codec)))
    {
        printf("    Default video codec: %s.\n", desc->name);
    }
    if (fmt->audio_codec != AV_CODEC_ID_NONE &&
        (desc = avcodec_descriptor_get(fmt->audio_codec)))
    {
        printf("    Default audio codec: %s.\n", desc->name);
    }
    if (fmt->subtitle_codec != AV_CODEC_ID_NONE &&
        (desc = avcodec_descriptor_get(fmt->subtitle_codec)))
    {
        printf("    Default subtitle codec: %s.\n", desc->name);
    }

    if (fmt->priv_class)
        show_help_children(fmt->priv_class, AV_OPT_FLAG_ENCODING_PARAM);
}

enum AVMediaType avfilter_pad_get_type(const AVFilterPad *pads, int pad_idx)
{
    return pads[pad_idx].type;
}

const char *avfilter_pad_get_name(const AVFilterPad *pads, int pad_idx)
{
    return pads[pad_idx].name;
}

#if CONFIG_AVFILTER
static void show_help_filter(const char *name)
{
#if CONFIG_AVFILTER
    const AVFilter *f = avfilter_get_by_name(name);
    int i, count;

    if (!name)
    {
        av_log(NULL, AV_LOG_ERROR, "No filter name specified.\n");
        return;
    }
    else if (!f)
    {
        av_log(NULL, AV_LOG_ERROR, "Unknown filter '%s'.\n", name);
        return;
    }

    printf("Filter %s\n", f->name);
    if (f->description)
        printf("  %s\n", f->description);

    if (f->flags & AVFILTER_FLAG_SLICE_THREADS)
        printf("    slice threading supported\n");

    printf("    Inputs:\n");
    count = avfilter_pad_count(f->inputs);
    for (i = 0; i < count; i++)
    {
        printf("       #%d: %s (%s)\n", i, avfilter_pad_get_name(f->inputs, i),
               media_type_string(avfilter_pad_get_type(f->inputs, i)));
    }
    if (f->flags & AVFILTER_FLAG_DYNAMIC_INPUTS)
        printf("        dynamic (depending on the options)\n");
    else if (!count)
        printf("        none (source filter)\n");

    printf("    Outputs:\n");
    count = avfilter_pad_count(f->outputs);
    for (i = 0; i < count; i++)
    {
        printf("       #%d: %s (%s)\n", i, avfilter_pad_get_name(f->outputs, i),
               media_type_string(avfilter_pad_get_type(f->outputs, i)));
    }
    if (f->flags & AVFILTER_FLAG_DYNAMIC_OUTPUTS)
        printf("        dynamic (depending on the options)\n");
    else if (!count)
        printf("        none (sink filter)\n");

    if (f->priv_class)
        show_help_children(f->priv_class, AV_OPT_FLAG_VIDEO_PARAM | AV_OPT_FLAG_FILTERING_PARAM |
                                              AV_OPT_FLAG_AUDIO_PARAM);
    if (f->flags & AVFILTER_FLAG_SUPPORT_TIMELINE)
        printf("This filter has support for timeline through the 'enable' option.\n");
#else
    av_log(NULL, AV_LOG_ERROR, "Build without libavfilter; "
                               "can not to satisfy request\n");
#endif
}
#endif

static const enum AVCodecID codec_ids[] = {
    AV_CODEC_ID_AAC,
    AV_CODEC_ID_NONE,
};

#define PRINT_CODEC_SUPPORTED(codec, field, type, list_name, term, get_name) \
    if (codec->field)                                                        \
    {                                                                        \
        const type *p = codec->field;                                        \
                                                                             \
        printf("    Supported " list_name ":");                              \
        while (*p != term)                                                   \
        {                                                                    \
            get_name(*p);                                                    \
            printf(" %s", name);                                             \
            p++;                                                             \
        }                                                                    \
        printf("\n");                                                        \
    }

const AVBitStreamFilter *av_bsf_iterate(void **opaque)
{
    uintptr_t i = (uintptr_t)*opaque;
    const AVBitStreamFilter *f = bitstream_filters[i];

    if (f)
        *opaque = (void*)(i + 1);

    return f;
}

const AVBitStreamFilter *av_bsf_get_by_name(const char *name)
{
    const AVBitStreamFilter *f = NULL;
    void *i = 0;

    if (!name)
        return NULL;

    while ((f = av_bsf_iterate(&i))) {
        if (!strcmp(f->name, name))
            return f;
    }

    return NULL;
}

static void show_help_bsf(const char *name)
{
    const AVBitStreamFilter *bsf = av_bsf_get_by_name(name);

    if (!name)
    {
        av_log(NULL, AV_LOG_ERROR, "No bitstream filter name specified.\n");
        return;
    }
    else if (!bsf)
    {
        av_log(NULL, AV_LOG_ERROR, "Unknown bit stream filter '%s'.\n", name);
        return;
    }

    printf("Bit stream filter %s\n", bsf->name);
    PRINT_CODEC_SUPPORTED(bsf, codec_ids, enum AVCodecID, "codecs",
                          AV_CODEC_ID_NONE, GET_CODEC_NAME);
    if (bsf->priv_class)
        show_help_children(bsf->priv_class, AV_OPT_FLAG_BSF_PARAM);
}

static void print_buildconf(int flags, int level)
{
    const char *indent = flags & INDENT ? "  " : "";
    char str[] = {FFMPEG_CONFIGURATION};
    char *conflist, *remove_tilde, *splitconf;

    // Change all the ' --' strings to '~--' so that
    // they can be identified as tokens.
    while ((conflist = strstr(str, " --")) != NULL)
    {
        strncpy(conflist, "~--", 3);
    }

    // Compensate for the weirdness this would cause
    // when passing 'pkg-config --static'.
    while ((remove_tilde = strstr(str, "pkg-config~")) != NULL)
    {
        strncpy(remove_tilde, "pkg-config ", 11);
    }

    splitconf = strtok(str, "~");
    av_log(NULL, level, "\n%sconfiguration:\n", indent);
    while (splitconf != NULL)
    {
        av_log(NULL, level, "%s%s%s\n", indent, indent, splitconf);
        splitconf = strtok(NULL, "~");
    }
}

void av_log_set_callback(void (*callback)(void*, int, const char*, va_list))
{
    av_log_callback = callback;
}


int show_buildconf(void *optctx, const char *opt, const char *arg)
{
    av_log_set_callback(log_callback_help);
    print_buildconf(INDENT | 0, AV_LOG_INFO);

    return 0;
}

static void print_program_info(int flags, int level)
{
    const char *indent = flags & INDENT ? "  " : "";

    av_log(NULL, level, "%s version " FFMPEG_VERSION, program_name);
    if (flags & SHOW_COPYRIGHT)
        av_log(NULL, level, " Copyright (c) %d-%d the FFmpeg developers",
               program_birth_year, CONFIG_THIS_YEAR);
    av_log(NULL, level, "\n");
    av_log(NULL, level, "%sbuilt with %s\n", indent, CC_IDENT);

    av_log(NULL, level, "%sconfiguration: " FFMPEG_CONFIGURATION "\n", indent);
}

static int warned_cfg = 0;

#define PRINT_LIB_INFO(libname, LIBNAME, flags, level)                    \
    if (CONFIG_##LIBNAME)                                                 \
    {                                                                     \
        const char *indent = flags & INDENT ? "  " : "";                  \
        if (flags & SHOW_VERSION)                                         \
        {                                                                 \
            unsigned int version = libname##_version();                   \
            av_log(NULL, level,                                           \
                   "%slib%-11s %2d.%3d.%3d / %2d.%3d.%3d\n",              \
                   indent, #libname,                                      \
                   LIB##LIBNAME##_VERSION_MAJOR,                          \
                   LIB##LIBNAME##_VERSION_MINOR,                          \
                   LIB##LIBNAME##_VERSION_MICRO,                          \
                   AV_VERSION_MAJOR(version), AV_VERSION_MINOR(version),  \
                   AV_VERSION_MICRO(version));                            \
        }                                                                 \
        if (flags & SHOW_CONFIG)                                          \
        {                                                                 \
            const char *cfg = libname##_configuration();                  \
            if (strcmp(FFMPEG_CONFIGURATION, cfg))                        \
            {                                                             \
                if (!warned_cfg)                                          \
                {                                                         \
                    av_log(NULL, level,                                   \
                           "%sWARNING: library configuration mismatch\n", \
                           indent);                                       \
                    warned_cfg = 1;                                       \
                }                                                         \
                av_log(NULL, level, "%s%-11s configuration: %s\n",        \
                       indent, #libname, cfg);                            \
            }                                                             \
        }                                                                 \
    }

static void print_all_libs_info(int flags, int level)
{
    PRINT_LIB_INFO(avutil, AVUTIL, flags, level);
    PRINT_LIB_INFO(avcodec, AVCODEC, flags, level);
    PRINT_LIB_INFO(avformat, AVFORMAT, flags, level);
    PRINT_LIB_INFO(avdevice, AVDEVICE, flags, level);
    PRINT_LIB_INFO(avfilter, AVFILTER, flags, level);
    PRINT_LIB_INFO(avresample, AVRESAMPLE, flags, level);
    PRINT_LIB_INFO(swscale, SWSCALE, flags, level);
    PRINT_LIB_INFO(swresample, SWRESAMPLE, flags, level);
    PRINT_LIB_INFO(postproc, POSTPROC, flags, level);
}


void show_banner(int argc, char **argv, const OptionDef *options)
{
    int idx = locate_option(argc, argv, options, "version");
    if (hide_banner || idx)
        return;

    print_program_info(INDENT | SHOW_COPYRIGHT, AV_LOG_INFO);
    print_all_libs_info(INDENT | SHOW_CONFIG, AV_LOG_INFO);
    print_all_libs_info(INDENT | SHOW_VERSION, AV_LOG_INFO);
}

static int is_device(const AVClass *avclass)
{
    if (!avclass)
        return 0;
    return AV_IS_INPUT_DEVICE(avclass->category) || AV_IS_OUTPUT_DEVICE(avclass->category);
}

static const AVInputFormat *const *indev_list = NULL;
static const AVOutputFormat *const *outdev_list = NULL;
static AVMutex avpriv_register_devices_mutex = AV_MUTEX_INITIALIZER;

const AVOutputFormat *av_muxer_iterate(void **opaque)
{
    static const uintptr_t size = sizeof(muxer_list) / sizeof(muxer_list[0]) - 1;
    uintptr_t i = (uintptr_t)*opaque;
    const AVOutputFormat *f = NULL;

    if (i < size)
    {
        f = muxer_list[i];
    }
    else if (indev_list)
    {
        f = outdev_list[i - size];
    }

    if (f)
        *opaque = (void *)(i + 1);
    return f;
}

static int show_formats_devices(void *optctx, const char *opt, const char *arg, int device_only, int muxdemuxers)
{
    void *ifmt_opaque = NULL;
    const AVInputFormat *ifmt = NULL;
    void *ofmt_opaque = NULL;
    const AVOutputFormat *ofmt = NULL;
    const char *last_name;
    int is_dev;

    printf("%s\n"
           " D. = Demuxing supported\n"
           " .E = Muxing supported\n"
           " --\n",
           device_only ? "Devices:" : "File formats:");
    last_name = "000";
    for (;;)
    {
        int decode = 0;
        int encode = 0;
        const char *name = NULL;
        const char *long_name = NULL;

        if (muxdemuxers != SHOW_DEMUXERS)
        {
            ofmt_opaque = NULL;
            while ((ofmt = av_muxer_iterate(&ofmt_opaque)))
            {
                is_dev = is_device(ofmt->priv_class);
                if (!is_dev && device_only)
                    continue;
                if ((!name || strcmp(ofmt->name, name) < 0) &&
                    strcmp(ofmt->name, last_name) > 0)
                {
                    name = ofmt->name;
                    long_name = ofmt->long_name;
                    encode = 1;
                }
            }
        }
        if (muxdemuxers != SHOW_MUXERS)
        {
            ifmt_opaque = NULL;
            while ((ifmt = av_demuxer_iterate(&ifmt_opaque)))
            {
                is_dev = is_device(ifmt->priv_class);
                if (!is_dev && device_only)
                    continue;
                if ((!name || strcmp(ifmt->name, name) < 0) &&
                    strcmp(ifmt->name, last_name) > 0)
                {
                    name = ifmt->name;
                    long_name = ifmt->long_name;
                    encode = 0;
                }
                if (name && strcmp(ifmt->name, name) == 0)
                    decode = 1;
            }
        }
        if (!name)
            break;
        last_name = name;

        printf(" %s%s %-15s %s\n",
               decode ? "D" : " ",
               encode ? "E" : " ",
               name,
               long_name ? long_name : " ");
    }
    return 0;
}

int show_formats(void *optctx, const char *opt, const char *arg)
{
    return show_formats_devices(optctx, opt, arg, 0, SHOW_DEFAULT);
}

int show_muxers(void *optctx, const char *opt, const char *arg)
{
    return show_formats_devices(optctx, opt, arg, 0, SHOW_MUXERS);
}

int show_demuxers(void *optctx, const char *opt, const char *arg)
{
    return show_formats_devices(optctx, opt, arg, 0, SHOW_DEMUXERS);
}

int show_devices(void *optctx, const char *opt, const char *arg)
{
    return show_formats_devices(optctx, opt, arg, 1, SHOW_DEFAULT);
}

static void show_usage(void)
{
    av_log(NULL, AV_LOG_INFO, "Simple media player\n");
    av_log(NULL, AV_LOG_INFO, "usage: %s [options] input_file\n", program_name);
    av_log(NULL, AV_LOG_INFO, "\n");
}

const AVClass *avformat_get_class(void)
{
    return &av_format_context_class;
}

const AVClass *avfilter_get_class(void)
{
    return &avfilter_class;
}

void show_help_default(const char *opt, const char *arg)
{
    av_log_set_callback(log_callback_help);
    show_usage();
    show_help_options(options, "Main options:", 0, OPT_EXPERT, 0);
    show_help_options(options, "Advanced options:", OPT_EXPERT, 0, 0);
    printf("\n");
    show_help_children(avcodec_get_class(), AV_OPT_FLAG_DECODING_PARAM);
    show_help_children(avformat_get_class(), AV_OPT_FLAG_DECODING_PARAM);
    // #if !CONFIG_AVFILTER
    // show_help_children(sws_get_class(), AV_OPT_FLAG_ENCODING_PARAM);
    // #else
    show_help_children(avfilter_get_class(), AV_OPT_FLAG_FILTERING_PARAM);
    // #endif
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
           "left double-click   toggle full screen\n");
}

void init_opts(void)
{
    av_dict_set(&sws_dict, "flags", "bicubic", 0);
}

void uninit_opts(void)
{
    av_dict_free(&swr_opts);
    av_dict_free(&sws_dict);
    av_dict_free(&format_opts);
    av_dict_free(&codec_opts);
    av_dict_free(&resample_opts);
}

static FILE *report_file;

void av_log_format_line(void *ptr, int level, const char *fmt, va_list vl,
                        char *line, int line_size, int *print_prefix)
{
    av_log_format_line2(ptr, level, fmt, vl, line, line_size, print_prefix);
}

static void log_callback_report(void *ptr, int level, const char *fmt, va_list vl)
{
    va_list vl2;
    char line[1024];
    static int print_prefix = 1;

    va_copy(vl2, vl);
    av_log_default_callback(ptr, level, fmt, vl);
    av_log_format_line(ptr, level, fmt, vl2, line, sizeof(line), &print_prefix);
    va_end(vl2);
    if (report_file_level >= level)
    {
        fputs(line, report_file);
        fflush(report_file);
    }
}

void init_dynload(void)
{
#if HAVE_SETDLLDIRECTORY && defined(_WIN32)
    /* Calling SetDllDirectory with the empty string (but not NULL) removes the
     * current working directory from the DLL search path as a security pre-caution. */
    SetDllDirectory("");
#endif
}

static void (*program_exit)(int ret);

void register_exit(void (*cb)(int ret))
{
    program_exit = cb;
}

void exit_program(int ret)
{
    if (program_exit)
        program_exit(ret);

    exit(ret);
}

size_t av_strlcpy(char *dst, const char *src, size_t size)
{
    size_t len = 0;
    while (++len < size && *src)
        *dst++ = *src++;
    if (len <= size)
        *dst = 0;
    return len + strlen(src) - 1;
}

size_t av_strlcat(char *dst, const char *src, size_t size)
{
    size_t len = strlen(dst);
    if (size <= len + 1)
        return len + strlen(src);
    return len + av_strlcpy(dst + len, src, size - len);
}


void show_help_options(const OptionDef *options, const char *msg, int req_flags,
                       int rej_flags, int alt_flags)
{
    const OptionDef *po;
    int first;

    first = 1;
    for (po = options; po->name; po++)
    {
        char buf[128];

        if (((po->flags & req_flags) != req_flags) ||
            (alt_flags && !(po->flags & alt_flags)) ||
            (po->flags & rej_flags))
            continue;

        if (first)
        {
            printf("%s\n", msg);
            first = 0;
        }
        av_strlcpy(buf, po->name, sizeof(buf));
        if (po->argname)
        {
            av_strlcat(buf, " ", sizeof(buf));
            av_strlcat(buf, po->argname, sizeof(buf));
        }
        printf("-%-17s  %s\n", buf, po->help);
    }
    printf("\n");
}

int av_opt_show2(void *obj, void *av_log_obj, int req_flags, int rej_flags)
{
    if (!obj)
        return -1;

    av_log(av_log_obj, AV_LOG_INFO, "%s AVOptions:\n", (*(AVClass **)obj)->class_name);

    opt_list(obj, av_log_obj, NULL, req_flags, rej_flags, -1);

    return 0;
}

void show_help_children(const AVClass *class, int flags)
{
    void *iter = NULL;
    const AVClass *child;
    if (class->option)
    {
        av_opt_show2(&class, NULL, flags, 0);
        printf("\n");
    }

    while (child = av_opt_child_class_iterate(class, &iter))
        show_help_children(child, flags);
}

static const OptionDef *find_option(const OptionDef *po, const char *name)
{
    const char *p = strchr(name, ':');
    int len = p ? p - name : strlen(name);

    while (po->name)
    {
        if (!strncmp(name, po->name, len) && strlen(po->name) == len)
            break;
        po++;
    }
    return po;
}

/* _WIN32 means using the windows libc - cygwin doesn't define that
 * by default. HAVE_COMMANDLINETOARGVW is true on cygwin, while
 * it doesn't provide the actual command line via GetCommandLineW(). */
#if HAVE_COMMANDLINETOARGVW && defined(_WIN32)
#include <shellapi.h>
/* Will be leaked on exit */
static char **win32_argv_utf8 = NULL;
static int win32_argc = 0;

/**
 * Prepare command line arguments for executable.
 * For Windows - perform wide-char to UTF-8 conversion.
 * Input arguments should be main() function arguments.
 * @param argc_ptr Arguments number (including executable)
 * @param argv_ptr Arguments list.
 */
static void prepare_app_arguments(int *argc_ptr, char ***argv_ptr)
{
    char *argstr_flat;
    wchar_t **argv_w;
    int i, buffsize = 0, offset = 0;

    if (win32_argv_utf8)
    {
        *argc_ptr = win32_argc;
        *argv_ptr = win32_argv_utf8;
        return;
    }

    win32_argc = 0;
    argv_w = CommandLineToArgvW(GetCommandLineW(), &win32_argc);
    if (win32_argc <= 0 || !argv_w)
        return;

    /* determine the UTF-8 buffer size (including NULL-termination symbols) */
    for (i = 0; i < win32_argc; i++)
        buffsize += WideCharToMultiByte(CP_UTF8, 0, argv_w[i], -1,
                                        NULL, 0, NULL, NULL);

    win32_argv_utf8 = av_mallocz(sizeof(char *) * (win32_argc + 1) + buffsize);
    argstr_flat = (char *)win32_argv_utf8 + sizeof(char *) * (win32_argc + 1);
    if (!win32_argv_utf8)
    {
        LocalFree(argv_w);
        return;
    }

    for (i = 0; i < win32_argc; i++)
    {
        win32_argv_utf8[i] = &argstr_flat[offset];
        offset += WideCharToMultiByte(CP_UTF8, 0, argv_w[i], -1,
                                      &argstr_flat[offset],
                                      buffsize - offset, NULL, NULL);
    }
    win32_argv_utf8[i] = NULL;
    LocalFree(argv_w);

    *argc_ptr = win32_argc;
    *argv_ptr = win32_argv_utf8;
}
#else
static inline void prepare_app_arguments(int *argc_ptr, char ***argv_ptr)
{
    /* nothing to do */
}
#endif /* HAVE_COMMANDLINETOARGVW */

static int write_option(void *optctx, const OptionDef *po, const char *opt,
                        const char *arg)
{
    void *dst = po->flags & (OPT_OFFSET | OPT_SPEC) ? (uint8_t *)optctx + po->u.off : po->u.dst_ptr;
    int *dstcount;

    if (po->flags & OPT_SPEC)
    {
        SpecifierOpt **so = dst;
        char *p = strchr(opt, ':');
        char *str;

        dstcount = (int *)(so + 1);
        *so = grow_array(*so, sizeof(**so), dstcount, *dstcount + 1);
        str = av_strdup(p ? p + 1 : "");
        if (!str)
            return AVERROR(ENOMEM);
        (*so)[*dstcount - 1].specifier = str;
        dst = &(*so)[*dstcount - 1].u;
    }

    if (po->flags & OPT_STRING)
    {
        char *str;
        str = av_strdup(arg);
        av_freep(dst);
        if (!str)
            return AVERROR(ENOMEM);
        *(char **)dst = str;
    }
    else if (po->flags & OPT_BOOL || po->flags & OPT_INT)
    {
        *(int *)dst = parse_number_or_die(opt, arg, OPT_INT64, INT_MIN, INT_MAX);
    }
    else if (po->flags & OPT_INT64)
    {
        *(int64_t *)dst = parse_number_or_die(opt, arg, OPT_INT64, INT64_MIN, INT64_MAX);
    }
    else if (po->flags & OPT_TIME)
    {
        *(int64_t *)dst = parse_time_or_die(opt, arg, 1);
    }
    else if (po->flags & OPT_FLOAT)
    {
        *(float *)dst = parse_number_or_die(opt, arg, OPT_FLOAT, -INFINITY, INFINITY);
    }
    else if (po->flags & OPT_DOUBLE)
    {
        *(double *)dst = parse_number_or_die(opt, arg, OPT_DOUBLE, -INFINITY, INFINITY);
    }
    else if (po->u.func_arg)
    {
        int ret = po->u.func_arg(optctx, opt, arg);
        if (ret < 0)
        {
            av_log(NULL, AV_LOG_ERROR,
                   "Failed to set value '%s' for option '%s': %s\n",
                   arg, opt, av_err2str(ret));
            return ret;
        }
    }
    if (po->flags & OPT_EXIT)
        exit_program(0);

    return 0;
}

int parse_option(void *optctx, const char *opt, const char *arg,
                 const OptionDef *options)
{
    const OptionDef *po;
    int ret;

    po = find_option(options, opt);
    if (!po->name && opt[0] == 'n' && opt[1] == 'o')
    {
        /* handle 'no' bool option */
        po = find_option(options, opt + 2);
        if ((po->name && (po->flags & OPT_BOOL)))
            arg = "0";
    }
    else if (po->flags & OPT_BOOL)
        arg = "1";

    if (!po->name)
        po = find_option(options, "default");
    if (!po->name)
    {
        av_log(NULL, AV_LOG_ERROR, "Unrecognized option '%s'\n", opt);
        return AVERROR(EINVAL);
    }
    if (po->flags & HAS_ARG && !arg)
    {
        av_log(NULL, AV_LOG_ERROR, "Missing argument for option '%s'\n", opt);
        return AVERROR(EINVAL);
    }

    ret = write_option(optctx, po, opt, arg);
    if (ret < 0)
        return ret;

    return !!(po->flags & HAS_ARG);
}

void parse_options(void *optctx, int argc, char **argv, const OptionDef *options,
                   void (*parse_arg_function)(void *, const char *))
{
    const char *opt;
    int optindex, handleoptions = 1, ret;

    /* perform system-dependent conversions for arguments list */
    prepare_app_arguments(&argc, &argv);

    /* parse options */
    optindex = 1;
    while (optindex < argc)
    {
        opt = argv[optindex++];

        if (handleoptions && opt[0] == '-' && opt[1] != '\0')
        {
            if (opt[1] == '-' && opt[2] == '\0')
            {
                handleoptions = 0;
                continue;
            }
            opt++;

            if ((ret = parse_option(optctx, opt, argv[optindex], options)) < 0)
                exit_program(1);
            optindex += ret;
        }
        else
        {
            if (parse_arg_function)
                parse_arg_function(optctx, opt);
        }
    }
}

int parse_optgroup(void *optctx, OptionGroup *g)
{
    int i, ret;

    av_log(NULL, AV_LOG_DEBUG, "Parsing a group of options: %s %s.\n",
           g->group_def->name, g->arg);

    for (i = 0; i < g->nb_opts; i++)
    {
        Option *o = &g->opts[i];

        if (g->group_def->flags &&
            !(g->group_def->flags & o->opt->flags))
        {
            av_log(NULL, AV_LOG_ERROR, "Option %s (%s) cannot be applied to "
                                       "%s %s -- you are trying to apply an input option to an "
                                       "output file or vice versa. Move this option before the "
                                       "file it belongs to.\n",
                   o->key, o->opt->help,
                   g->group_def->name, g->arg);
            return AVERROR(EINVAL);
        }

        av_log(NULL, AV_LOG_DEBUG, "Applying option %s (%s) with argument %s.\n",
               o->key, o->opt->help, o->val);

        ret = write_option(optctx, o->opt, o->key, o->val);
        if (ret < 0)
            return ret;
    }

    av_log(NULL, AV_LOG_DEBUG, "Successfully parsed a group of options.\n");

    return 0;
}

int locate_option(int argc, char **argv, const OptionDef *options,
                  const char *optname)
{
    const OptionDef *po;
    int i;

    for (i = 1; i < argc; i++)
    {
        const char *cur_opt = argv[i];

        if (*cur_opt++ != '-')
            continue;

        po = find_option(options, cur_opt);
        if (!po->name && cur_opt[0] == 'n' && cur_opt[1] == 'o')
            po = find_option(options, cur_opt + 2);

        if ((!po->name && !strcmp(cur_opt, optname)) ||
            (po->name && !strcmp(optname, po->name)))
            return i;

        if (!po->name || po->flags & HAS_ARG)
            i++;
    }
    return 0;
}

static void dump_argument(const char *a)
{
    const unsigned char *p;

    for (p = a; *p; p++)
        if (!((*p >= '+' && *p <= ':') || (*p >= '@' && *p <= 'Z') ||
              *p == '_' || (*p >= 'a' && *p <= 'z')))
            break;
    if (!*p)
    {
        fputs(a, report_file);
        return;
    }
    fputc('"', report_file);
    for (p = a; *p; p++)
    {
        if (*p == '\\' || *p == '"' || *p == '$' || *p == '`')
            fprintf(report_file, "\\%c", *p);
        else if (*p < ' ' || *p > '~')
            fprintf(report_file, "\\x%02x", *p);
        else
            fputc(*p, report_file);
    }
    fputc('"', report_file);
}

static void check_options(const OptionDef *po)
{
    while (po->name)
    {
        if (po->flags & OPT_PERFILE)
            av_assert0(po->flags & (OPT_INPUT | OPT_OUTPUT));
        po++;
    }
}


void av_bprint_chars(AVBPrint *buf, char c, unsigned n)
{
    unsigned room, real_n;

    while (1) {
        room = av_bprint_room(buf);
        if (n < room)
            break;
        if (av_bprint_alloc(buf, n))
            break;
    }
    if (room) {
        real_n = FFMIN(n, room - 1);
        memset(buf->str + buf->len, c, real_n);
    }
    av_bprint_grow(buf, n);
}

static void expand_filename_template(AVBPrint *bp, const char *template,
                                     struct tm *tm)
{
    int c;

    while ((c = *(template ++)))
    {
        if (c == '%')
        {
            if (!(c = *(template ++)))
                break;
            switch (c)
            {
            case 'p':
                av_bprintf(bp, "%s", program_name);
                break;
            case 't':
                av_bprintf(bp, "%04d%02d%02d-%02d%02d%02d",
                           tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                           tm->tm_hour, tm->tm_min, tm->tm_sec);
                break;
            case '%':
                av_bprint_chars(bp, c, 1);
                break;
            }
        }
        else
        {
            av_bprint_chars(bp, c, 1);
        }
    }
}

static int init_report(const char *env)
{
    char *filename_template = NULL;
    char *key, *val;
    int ret, count = 0;
    int prog_loglevel, envlevel = 0;
    time_t now;
    struct tm *tm;
    AVBPrint filename;

    if (report_file) /* already opened */
        return 0;
    time(&now);
    tm = localtime(&now);

    while (env && *env)
    {
        if ((ret = av_opt_get_key_value(&env, "=", ":", 0, &key, &val)) < 0)
        {
            if (count)
                av_log(NULL, AV_LOG_ERROR,
                       "Failed to parse FFREPORT environment variable: %s\n",
                       av_err2str(ret));
            break;
        }
        if (*env)
            env++;
        count++;
        if (!strcmp(key, "file"))
        {
            av_free(filename_template);
            filename_template = val;
            val = NULL;
        }
        else if (!strcmp(key, "level"))
        {
            char *tail;
            report_file_level = strtol(val, &tail, 10);
            if (*tail)
            {
                av_log(NULL, AV_LOG_FATAL, "Invalid report file level\n");
                exit_program(1);
            }
            envlevel = 1;
        }
        else
        {
            av_log(NULL, AV_LOG_ERROR, "Unknown key '%s' in FFREPORT\n", key);
        }
        av_free(val);
        av_free(key);
    }

    av_bprint_init(&filename, 0, AV_BPRINT_SIZE_AUTOMATIC);
    expand_filename_template(&filename,
                             av_x_if_null(filename_template, "%p-%t.log"), tm);
    av_free(filename_template);
    if (!av_bprint_is_complete(&filename))
    {
        av_log(NULL, AV_LOG_ERROR, "Out of memory building report file name\n");
        return AVERROR(ENOMEM);
    }

    prog_loglevel = av_log_get_level();
    if (!envlevel)
        report_file_level = FFMAX(report_file_level, prog_loglevel);

    report_file = fopen(filename.str, "w");
    if (!report_file)
    {
        int ret = AVERROR(errno);
        av_log(NULL, AV_LOG_ERROR, "Failed to open report \"%s\": %s\n",
               filename.str, strerror(errno));
        return ret;
    }
    av_log_set_callback(log_callback_report);
    av_log(NULL, AV_LOG_INFO,
           "%s started on %04d-%02d-%02d at %02d:%02d:%02d\n"
           "Report written to \"%s\"\n"
           "Log level: %d\n",
           program_name,
           tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
           tm->tm_hour, tm->tm_min, tm->tm_sec,
           filename.str, report_file_level);
    av_bprint_finalize(&filename, NULL);
    return 0;
}


void parse_loglevel(int argc, char **argv, const OptionDef *options)
{
    int idx = locate_option(argc, argv, options, "loglevel");
    const char *env;

    check_options(options);

    if (!idx)
        idx = locate_option(argc, argv, options, "v");
    if (idx && argv[idx + 1])
        opt_loglevel(NULL, "loglevel", argv[idx + 1]);
    idx = locate_option(argc, argv, options, "report");
    if ((env = getenv("FFREPORT")) || idx)
    {
        init_report(env);
        if (report_file)
        {
            int i;
            fprintf(report_file, "Command line:\n");
            for (i = 0; i < argc; i++)
            {
                dump_argument(argv[i]);
                fputc(i < argc - 1 ? ' ' : '\n', report_file);
            }
            fflush(report_file);
        }
    }
    idx = locate_option(argc, argv, options, "hide_banner");
    if (idx)
        hide_banner = 1;
}

static const AVOption *opt_find(void *obj, const char *name, const char *unit,
                                int opt_flags, int search_flags)
{
    const AVOption *o = av_opt_find(obj, name, unit, opt_flags, search_flags);
    if (o && !o->flags)
        return NULL;
    return o;
}

#define FLAGS (o->type == AV_OPT_TYPE_FLAGS && (arg[0] == '-' || arg[0] == '+')) ? AV_DICT_APPEND : 0
int opt_default(void *optctx, const char *opt, const char *arg)
{
    const AVOption *o;
    int consumed = 0;
    char opt_stripped[128];
    const char *p;
    const AVClass *cc = avcodec_get_class(), *fc = avformat_get_class();
#if CONFIG_AVRESAMPLE
    const AVClass *rc = avresample_get_class();
#endif
#if CONFIG_SWSCALE
    const AVClass *sc = sws_get_class();
#endif
#if CONFIG_SWRESAMPLE
    const AVClass *swr_class = swr_get_class();
#endif

    if (!strcmp(opt, "debug") || !strcmp(opt, "fdebug"))
        av_log_set_level(AV_LOG_DEBUG);

    if (!(p = strchr(opt, ':')))
        p = opt + strlen(opt);
    av_strlcpy(opt_stripped, opt, FFMIN(sizeof(opt_stripped), p - opt + 1));

    if ((o = opt_find(&cc, opt_stripped, NULL, 0,
                      AV_OPT_SEARCH_CHILDREN | AV_OPT_SEARCH_FAKE_OBJ)) ||
        ((opt[0] == 'v' || opt[0] == 'a' || opt[0] == 's') &&
         (o = opt_find(&cc, opt + 1, NULL, 0, AV_OPT_SEARCH_FAKE_OBJ))))
    {
        av_dict_set(&codec_opts, opt, arg, FLAGS);
        consumed = 1;
    }
    if ((o = opt_find(&fc, opt, NULL, 0,
                      AV_OPT_SEARCH_CHILDREN | AV_OPT_SEARCH_FAKE_OBJ)))
    {
        av_dict_set(&format_opts, opt, arg, FLAGS);
        if (consumed)
            av_log(NULL, AV_LOG_VERBOSE, "Routing option %s to both codec and muxer layer\n", opt);
        consumed = 1;
    }
#if CONFIG_SWSCALE
    if (!consumed && (o = opt_find(&sc, opt, NULL, 0,
                                   AV_OPT_SEARCH_CHILDREN | AV_OPT_SEARCH_FAKE_OBJ)))
    {
        struct SwsContext *sws = sws_alloc_context();
        int ret = av_opt_set(sws, opt, arg, 0);
        sws_freeContext(sws);
        if (!strcmp(opt, "srcw") || !strcmp(opt, "srch") ||
            !strcmp(opt, "dstw") || !strcmp(opt, "dsth") ||
            !strcmp(opt, "src_format") || !strcmp(opt, "dst_format"))
        {
            av_log(NULL, AV_LOG_ERROR, "Directly using swscale dimensions/format options is not supported, please use the -s or -pix_fmt options\n");
            return AVERROR(EINVAL);
        }
        if (ret < 0)
        {
            av_log(NULL, AV_LOG_ERROR, "Error setting option %s.\n", opt);
            return ret;
        }

        av_dict_set(&sws_dict, opt, arg, FLAGS);

        consumed = 1;
    }
#else
    if (!consumed && !strcmp(opt, "sws_flags"))
    {
        av_log(NULL, AV_LOG_WARNING, "Ignoring %s %s, due to disabled swscale\n", opt, arg);
        consumed = 1;
    }
#endif
#if CONFIG_SWRESAMPLE
    if (!consumed && (o = opt_find(&swr_class, opt, NULL, 0,
                                   AV_OPT_SEARCH_CHILDREN | AV_OPT_SEARCH_FAKE_OBJ)))
    {
        struct SwrContext *swr = swr_alloc();
        int ret = av_opt_set(swr, opt, arg, 0);
        swr_free(&swr);
        if (ret < 0)
        {
            av_log(NULL, AV_LOG_ERROR, "Error setting option %s.\n", opt);
            return ret;
        }
        av_dict_set(&swr_opts, opt, arg, FLAGS);
        consumed = 1;
    }
#endif
#if CONFIG_AVRESAMPLE
    if ((o = opt_find(&rc, opt, NULL, 0,
                      AV_OPT_SEARCH_CHILDREN | AV_OPT_SEARCH_FAKE_OBJ)))
    {
        av_dict_set(&resample_opts, opt, arg, FLAGS);
        consumed = 1;
    }
#endif

    if (consumed)
        return 0;
    return AVERROR_OPTION_NOT_FOUND;
}

/*
 * Check whether given option is a group separator.
 *
 * @return index of the group definition that matched or -1 if none
 */
static int match_group_separator(const OptionGroupDef *groups, int nb_groups,
                                 const char *opt)
{
    int i;

    for (i = 0; i < nb_groups; i++)
    {
        const OptionGroupDef *p = &groups[i];
        if (p->sep && !strcmp(p->sep, opt))
            return i;
    }

    return -1;
}

/*
 * Finish parsing an option group.
 *
 * @param group_idx which group definition should this group belong to
 * @param arg argument of the group delimiting option
 */
static void finish_group(OptionParseContext *octx, int group_idx,
                         const char *arg)
{
    OptionGroupList *l = &octx->groups[group_idx];
    OptionGroup *g;

    GROW_ARRAY(l->groups, l->nb_groups);
    g = &l->groups[l->nb_groups - 1];

    *g = octx->cur_group;
    g->arg = arg;
    g->group_def = l->group_def;
    g->sws_dict = sws_dict;
    g->swr_opts = swr_opts;
    g->codec_opts = codec_opts;
    g->format_opts = format_opts;
    g->resample_opts = resample_opts;

    codec_opts = NULL;
    format_opts = NULL;
    resample_opts = NULL;
    sws_dict = NULL;
    swr_opts = NULL;
    init_opts();

    memset(&octx->cur_group, 0, sizeof(octx->cur_group));
}

/*
 * Add an option instance to currently parsed group.
 */
static void add_opt(OptionParseContext *octx, const OptionDef *opt,
                    const char *key, const char *val)
{
    int global = !(opt->flags & (OPT_PERFILE | OPT_SPEC | OPT_OFFSET));
    OptionGroup *g = global ? &octx->global_opts : &octx->cur_group;

    GROW_ARRAY(g->opts, g->nb_opts);
    g->opts[g->nb_opts - 1].opt = opt;
    g->opts[g->nb_opts - 1].key = key;
    g->opts[g->nb_opts - 1].val = val;
}

static void init_parse_context(OptionParseContext *octx,
                               const OptionGroupDef *groups, int nb_groups)
{
    static const OptionGroupDef global_group = {"global"};
    int i;

    memset(octx, 0, sizeof(*octx));

    octx->nb_groups = nb_groups;
    octx->groups = av_mallocz_array(octx->nb_groups, sizeof(*octx->groups));
    if (!octx->groups)
        exit_program(1);

    for (i = 0; i < octx->nb_groups; i++)
        octx->groups[i].group_def = &groups[i];

    octx->global_opts.group_def = &global_group;
    octx->global_opts.arg = "";

    init_opts();
}

void uninit_parse_context(OptionParseContext *octx)
{
    int i, j;

    for (i = 0; i < octx->nb_groups; i++)
    {
        OptionGroupList *l = &octx->groups[i];

        for (j = 0; j < l->nb_groups; j++)
        {
            av_freep(&l->groups[j].opts);
            av_dict_free(&l->groups[j].codec_opts);
            av_dict_free(&l->groups[j].format_opts);
            av_dict_free(&l->groups[j].resample_opts);

            av_dict_free(&l->groups[j].sws_dict);
            av_dict_free(&l->groups[j].swr_opts);
        }
        av_freep(&l->groups);
    }
    av_freep(&octx->groups);

    av_freep(&octx->cur_group.opts);
    av_freep(&octx->global_opts.opts);

    uninit_opts();
}

int split_commandline(OptionParseContext *octx, int argc, char *argv[],
                      const OptionDef *options,
                      const OptionGroupDef *groups, int nb_groups)
{
    int optindex = 1;
    int dashdash = -2;

    /* perform system-dependent conversions for arguments list */
    prepare_app_arguments(&argc, &argv);

    init_parse_context(octx, groups, nb_groups);
    av_log(NULL, AV_LOG_DEBUG, "Splitting the commandline.\n");

    while (optindex < argc)
    {
        const char *opt = argv[optindex++], *arg;
        const OptionDef *po;
        int ret;

        av_log(NULL, AV_LOG_DEBUG, "Reading option '%s' ...", opt);

        if (opt[0] == '-' && opt[1] == '-' && !opt[2])
        {
            dashdash = optindex;
            continue;
        }
        /* unnamed group separators, e.g. output filename */
        if (opt[0] != '-' || !opt[1] || dashdash + 1 == optindex)
        {
            finish_group(octx, 0, opt);
            av_log(NULL, AV_LOG_DEBUG, " matched as %s.\n", groups[0].name);
            continue;
        }
        opt++;

#define GET_ARG(arg)                                                                \
    do                                                                              \
    {                                                                               \
        arg = argv[optindex++];                                                     \
        if (!arg)                                                                   \
        {                                                                           \
            av_log(NULL, AV_LOG_ERROR, "Missing argument for option '%s'.\n", opt); \
            return AVERROR(EINVAL);                                                 \
        }                                                                           \
    } while (0)

        /* named group separators, e.g. -i */
        if ((ret = match_group_separator(groups, nb_groups, opt)) >= 0)
        {
            GET_ARG(arg);
            finish_group(octx, ret, arg);
            av_log(NULL, AV_LOG_DEBUG, " matched as %s with argument '%s'.\n",
                   groups[ret].name, arg);
            continue;
        }

        /* normal options */
        po = find_option(options, opt);
        if (po->name)
        {
            if (po->flags & OPT_EXIT)
            {
                /* optional argument, e.g. -h */
                arg = argv[optindex++];
            }
            else if (po->flags & HAS_ARG)
            {
                GET_ARG(arg);
            }
            else
            {
                arg = "1";
            }

            add_opt(octx, po, opt, arg);
            av_log(NULL, AV_LOG_DEBUG, " matched as option '%s' (%s) with "
                                       "argument '%s'.\n",
                   po->name, po->help, arg);
            continue;
        }

        /* AVOptions */
        if (argv[optindex])
        {
            ret = opt_default(NULL, opt, argv[optindex]);
            if (ret >= 0)
            {
                av_log(NULL, AV_LOG_DEBUG, " matched as AVOption '%s' with "
                                           "argument '%s'.\n",
                       opt, argv[optindex]);
                optindex++;
                continue;
            }
            else if (ret != AVERROR_OPTION_NOT_FOUND)
            {
                av_log(NULL, AV_LOG_ERROR, "Error parsing option '%s' "
                                           "with argument '%s'.\n",
                       opt, argv[optindex]);
                return ret;
            }
        }

        /* boolean -nofoo options */
        if (opt[0] == 'n' && opt[1] == 'o' &&
            (po = find_option(options, opt + 2)) &&
            po->name && po->flags & OPT_BOOL)
        {
            add_opt(octx, po, opt, "0");
            av_log(NULL, AV_LOG_DEBUG, " matched as option '%s' (%s) with "
                                       "argument 0.\n",
                   po->name, po->help);
            continue;
        }

        av_log(NULL, AV_LOG_ERROR, "Unrecognized option '%s'.\n", opt);
        return AVERROR_OPTION_NOT_FOUND;
    }

    if (octx->cur_group.nb_opts || codec_opts || format_opts || resample_opts)
        av_log(NULL, AV_LOG_WARNING, "Trailing option(s) found in the "
                                     "command: may be ignored.\n");

    av_log(NULL, AV_LOG_DEBUG, "Finished splitting the commandline.\n");

    return 0;
}


int av_parse_cpu_flags(const char *s)
{
#define CPUFLAG_MMXEXT   (AV_CPU_FLAG_MMX      | AV_CPU_FLAG_MMXEXT | AV_CPU_FLAG_CMOV)
#define CPUFLAG_3DNOW    (AV_CPU_FLAG_3DNOW    | AV_CPU_FLAG_MMX)
#define CPUFLAG_3DNOWEXT (AV_CPU_FLAG_3DNOWEXT | CPUFLAG_3DNOW)
#define CPUFLAG_SSE      (AV_CPU_FLAG_SSE      | CPUFLAG_MMXEXT)
#define CPUFLAG_SSE2     (AV_CPU_FLAG_SSE2     | CPUFLAG_SSE)
#define CPUFLAG_SSE2SLOW (AV_CPU_FLAG_SSE2SLOW | CPUFLAG_SSE2)
#define CPUFLAG_SSE3     (AV_CPU_FLAG_SSE3     | CPUFLAG_SSE2)
#define CPUFLAG_SSE3SLOW (AV_CPU_FLAG_SSE3SLOW | CPUFLAG_SSE3)
#define CPUFLAG_SSSE3    (AV_CPU_FLAG_SSSE3    | CPUFLAG_SSE3)
#define CPUFLAG_SSE4     (AV_CPU_FLAG_SSE4     | CPUFLAG_SSSE3)
#define CPUFLAG_SSE42    (AV_CPU_FLAG_SSE42    | CPUFLAG_SSE4)
#define CPUFLAG_AVX      (AV_CPU_FLAG_AVX      | CPUFLAG_SSE42)
#define CPUFLAG_AVXSLOW  (AV_CPU_FLAG_AVXSLOW  | CPUFLAG_AVX)
#define CPUFLAG_XOP      (AV_CPU_FLAG_XOP      | CPUFLAG_AVX)
#define CPUFLAG_FMA3     (AV_CPU_FLAG_FMA3     | CPUFLAG_AVX)
#define CPUFLAG_FMA4     (AV_CPU_FLAG_FMA4     | CPUFLAG_AVX)
#define CPUFLAG_AVX2     (AV_CPU_FLAG_AVX2     | CPUFLAG_AVX)
#define CPUFLAG_BMI2     (AV_CPU_FLAG_BMI2     | AV_CPU_FLAG_BMI1)
#define CPUFLAG_AESNI    (AV_CPU_FLAG_AESNI    | CPUFLAG_SSE42)
#define CPUFLAG_AVX512   (AV_CPU_FLAG_AVX512   | CPUFLAG_AVX2)
    static const AVOption cpuflags_opts[] = {
        { "flags"   , NULL, 0, AV_OPT_TYPE_FLAGS, { .i64 = 0 }, INT64_MIN, INT64_MAX, .unit = "flags" },
#if   ARCH_PPC
        { "altivec" , NULL, 0, AV_OPT_TYPE_CONST, { .i64 = AV_CPU_FLAG_ALTIVEC  },    .unit = "flags" },
#elif ARCH_X86
        { "mmx"     , NULL, 0, AV_OPT_TYPE_CONST, { .i64 = AV_CPU_FLAG_MMX      },    .unit = "flags" },
        { "mmxext"  , NULL, 0, AV_OPT_TYPE_CONST, { .i64 = CPUFLAG_MMXEXT       },    .unit = "flags" },
        { "sse"     , NULL, 0, AV_OPT_TYPE_CONST, { .i64 = CPUFLAG_SSE          },    .unit = "flags" },
        { "sse2"    , NULL, 0, AV_OPT_TYPE_CONST, { .i64 = CPUFLAG_SSE2         },    .unit = "flags" },
        { "sse2slow", NULL, 0, AV_OPT_TYPE_CONST, { .i64 = CPUFLAG_SSE2SLOW     },    .unit = "flags" },
        { "sse3"    , NULL, 0, AV_OPT_TYPE_CONST, { .i64 = CPUFLAG_SSE3         },    .unit = "flags" },
        { "sse3slow", NULL, 0, AV_OPT_TYPE_CONST, { .i64 = CPUFLAG_SSE3SLOW     },    .unit = "flags" },
        { "ssse3"   , NULL, 0, AV_OPT_TYPE_CONST, { .i64 = CPUFLAG_SSSE3        },    .unit = "flags" },
        { "atom"    , NULL, 0, AV_OPT_TYPE_CONST, { .i64 = AV_CPU_FLAG_ATOM     },    .unit = "flags" },
        { "sse4.1"  , NULL, 0, AV_OPT_TYPE_CONST, { .i64 = CPUFLAG_SSE4         },    .unit = "flags" },
        { "sse4.2"  , NULL, 0, AV_OPT_TYPE_CONST, { .i64 = CPUFLAG_SSE42        },    .unit = "flags" },
        { "avx"     , NULL, 0, AV_OPT_TYPE_CONST, { .i64 = CPUFLAG_AVX          },    .unit = "flags" },
        { "avxslow" , NULL, 0, AV_OPT_TYPE_CONST, { .i64 = CPUFLAG_AVXSLOW      },    .unit = "flags" },
        { "xop"     , NULL, 0, AV_OPT_TYPE_CONST, { .i64 = CPUFLAG_XOP          },    .unit = "flags" },
        { "fma3"    , NULL, 0, AV_OPT_TYPE_CONST, { .i64 = CPUFLAG_FMA3         },    .unit = "flags" },
        { "fma4"    , NULL, 0, AV_OPT_TYPE_CONST, { .i64 = CPUFLAG_FMA4         },    .unit = "flags" },
        { "avx2"    , NULL, 0, AV_OPT_TYPE_CONST, { .i64 = CPUFLAG_AVX2         },    .unit = "flags" },
        { "bmi1"    , NULL, 0, AV_OPT_TYPE_CONST, { .i64 = AV_CPU_FLAG_BMI1     },    .unit = "flags" },
        { "bmi2"    , NULL, 0, AV_OPT_TYPE_CONST, { .i64 = CPUFLAG_BMI2         },    .unit = "flags" },
        { "3dnow"   , NULL, 0, AV_OPT_TYPE_CONST, { .i64 = CPUFLAG_3DNOW        },    .unit = "flags" },
        { "3dnowext", NULL, 0, AV_OPT_TYPE_CONST, { .i64 = CPUFLAG_3DNOWEXT     },    .unit = "flags" },
        { "cmov",     NULL, 0, AV_OPT_TYPE_CONST, { .i64 = AV_CPU_FLAG_CMOV     },    .unit = "flags" },
        { "aesni"   , NULL, 0, AV_OPT_TYPE_CONST, { .i64 = CPUFLAG_AESNI        },    .unit = "flags" },
        { "avx512"  , NULL, 0, AV_OPT_TYPE_CONST, { .i64 = CPUFLAG_AVX512       },    .unit = "flags" },
#elif ARCH_ARM
        { "armv5te",  NULL, 0, AV_OPT_TYPE_CONST, { .i64 = AV_CPU_FLAG_ARMV5TE  },    .unit = "flags" },
        { "armv6",    NULL, 0, AV_OPT_TYPE_CONST, { .i64 = AV_CPU_FLAG_ARMV6    },    .unit = "flags" },
        { "armv6t2",  NULL, 0, AV_OPT_TYPE_CONST, { .i64 = AV_CPU_FLAG_ARMV6T2  },    .unit = "flags" },
        { "vfp",      NULL, 0, AV_OPT_TYPE_CONST, { .i64 = AV_CPU_FLAG_VFP      },    .unit = "flags" },
        { "vfp_vm",   NULL, 0, AV_OPT_TYPE_CONST, { .i64 = AV_CPU_FLAG_VFP_VM   },    .unit = "flags" },
        { "vfpv3",    NULL, 0, AV_OPT_TYPE_CONST, { .i64 = AV_CPU_FLAG_VFPV3    },    .unit = "flags" },
        { "neon",     NULL, 0, AV_OPT_TYPE_CONST, { .i64 = AV_CPU_FLAG_NEON     },    .unit = "flags" },
#elif ARCH_AARCH64
        { "armv8",    NULL, 0, AV_OPT_TYPE_CONST, { .i64 = AV_CPU_FLAG_ARMV8    },    .unit = "flags" },
        { "neon",     NULL, 0, AV_OPT_TYPE_CONST, { .i64 = AV_CPU_FLAG_NEON     },    .unit = "flags" },
        { "vfp",      NULL, 0, AV_OPT_TYPE_CONST, { .i64 = AV_CPU_FLAG_VFP      },    .unit = "flags" },
#elif ARCH_MIPS
        { "mmi",      NULL, 0, AV_OPT_TYPE_CONST, { .i64 = AV_CPU_FLAG_MMI      },    .unit = "flags" },
        { "msa",      NULL, 0, AV_OPT_TYPE_CONST, { .i64 = AV_CPU_FLAG_MSA      },    .unit = "flags" },
#endif
        { NULL },
    };
    static const AVClass class = {
        .class_name = "cpuflags",
        .item_name  = av_default_item_name,
        .option     = cpuflags_opts,
        .version    = LIBAVUTIL_VERSION_INT,
    };

    int flags = 0, ret;
    const AVClass *pclass = &class;

    if ((ret = av_opt_eval_flags(&pclass, &cpuflags_opts[0], s, &flags)) < 0)
        return ret;

    return flags & INT_MAX;
}


void av_force_cpu_flags(int arg){
    if (ARCH_X86 &&
           (arg & ( AV_CPU_FLAG_3DNOW    |
                    AV_CPU_FLAG_3DNOWEXT |
                    AV_CPU_FLAG_MMXEXT   |
                    AV_CPU_FLAG_SSE      |
                    AV_CPU_FLAG_SSE2     |
                    AV_CPU_FLAG_SSE2SLOW |
                    AV_CPU_FLAG_SSE3     |
                    AV_CPU_FLAG_SSE3SLOW |
                    AV_CPU_FLAG_SSSE3    |
                    AV_CPU_FLAG_SSE4     |
                    AV_CPU_FLAG_SSE42    |
                    AV_CPU_FLAG_AVX      |
                    AV_CPU_FLAG_AVXSLOW  |
                    AV_CPU_FLAG_XOP      |
                    AV_CPU_FLAG_FMA3     |
                    AV_CPU_FLAG_FMA4     |
                    AV_CPU_FLAG_AVX2     |
                    AV_CPU_FLAG_AVX512   ))
        && !(arg & AV_CPU_FLAG_MMX)) {
        av_log(NULL, AV_LOG_WARNING, "MMX implied by specified flags\n");
        arg |= AV_CPU_FLAG_MMX;
    }

    atomic_store_explicit(&cpu_flags, arg, memory_order_relaxed);
}


int opt_cpuflags(void *optctx, const char *opt, const char *arg)
{
    int ret;
    unsigned flags = av_get_cpu_flags();

    if ((ret = av_parse_cpu_caps(&flags, arg)) < 0)
        return ret;

    av_force_cpu_flags(flags);
    return 0;
}

int opt_loglevel(void *optctx, const char *opt, const char *arg)
{
    const struct
    {
        const char *name;
        int level;
    } log_levels[] = {
        {"quiet", AV_LOG_QUIET},
        {"panic", AV_LOG_PANIC},
        {"fatal", AV_LOG_FATAL},
        {"error", AV_LOG_ERROR},
        {"warning", AV_LOG_WARNING},
        {"info", AV_LOG_INFO},
        {"verbose", AV_LOG_VERBOSE},
        {"debug", AV_LOG_DEBUG},
        {"trace", AV_LOG_TRACE},
    };
    const char *token;
    char *tail;
    int flags = av_log_get_flags();
    int level = av_log_get_level();
    int cmd, i = 0;

    av_assert0(arg);
    while (*arg)
    {
        token = arg;
        if (*token == '+' || *token == '-')
        {
            cmd = *token++;
        }
        else
        {
            cmd = 0;
        }
        if (!i && !cmd)
        {
            flags = 0; /* missing relative prefix, build absolute value */
        }
        if (!strncmp(token, "repeat", 6))
        {
            if (cmd == '-')
            {
                flags |= AV_LOG_SKIP_REPEATED;
            }
            else
            {
                flags &= ~AV_LOG_SKIP_REPEATED;
            }
            arg = token + 6;
        }
        else if (!strncmp(token, "level", 5))
        {
            if (cmd == '-')
            {
                flags &= ~AV_LOG_PRINT_LEVEL;
            }
            else
            {
                flags |= AV_LOG_PRINT_LEVEL;
            }
            arg = token + 5;
        }
        else
        {
            break;
        }
        i++;
    }
    if (!*arg)
    {
        goto end;
    }
    else if (*arg == '+')
    {
        arg++;
    }
    else if (!i)
    {
        flags = av_log_get_flags(); /* level value without prefix, reset flags */
    }

    for (i = 0; i < FF_ARRAY_ELEMS(log_levels); i++)
    {
        if (!strcmp(log_levels[i].name, arg))
        {
            level = log_levels[i].level;
            goto end;
        }
    }

    level = strtol(arg, &tail, 10);
    if (*tail)
    {
        av_log(NULL, AV_LOG_FATAL, "Invalid loglevel \"%s\". "
                                   "Possible levels are numbers or:\n",
               arg);
        for (i = 0; i < FF_ARRAY_ELEMS(log_levels); i++)
            av_log(NULL, AV_LOG_FATAL, "\"%s\"\n", log_levels[i].name);
        exit_program(1);
    }

end:
    av_log_set_flags(flags);
    av_log_set_level(level);
    return 0;
}




int opt_report(void *optctx, const char *opt, const char *arg)
{
    return init_report(NULL);
}

int opt_max_alloc(void *optctx, const char *opt, const char *arg)
{
    char *tail;
    size_t max;

    max = strtol(arg, &tail, 10);
    if (*tail)
    {
        av_log(NULL, AV_LOG_FATAL, "Invalid max_alloc \"%s\".\n", arg);
        exit_program(1);
    }
    av_max_alloc(max);
    return 0;
}

int opt_timelimit(void *optctx, const char *opt, const char *arg)
{
#if HAVE_SETRLIMIT
    int lim = parse_number_or_die(opt, arg, OPT_INT64, 0, INT_MAX);
    struct rlimit rl = {lim, lim + 1};
    if (setrlimit(RLIMIT_CPU, &rl))
        perror("setrlimit");
#else
    av_log(NULL, AV_LOG_WARNING, "-%s not implemented on this OS\n", opt);
#endif
    return 0;
}

int av_strerror(int errnum, char *errbuf, size_t errbuf_size)
{
    int ret = 0, i;
    const struct error_entry *entry = NULL;

    for (i = 0; i < FF_ARRAY_ELEMS(error_entries); i++) {
        if (errnum == error_entries[i].num) {
            entry = &error_entries[i];
            break;
        }
    }
    if (entry) {
        av_strlcpy(errbuf, entry->str, errbuf_size);
    } else {
#if HAVE_STRERROR_R
        ret = AVERROR(strerror_r(AVUNERROR(errnum), errbuf, errbuf_size));
#else
        ret = -1;
#endif
        if (ret < 0)
            snprintf(errbuf, errbuf_size, "Error number %d occurred", errnum);
    }

    return ret;
}

void print_error(const char *filename, int err)
{
    char errbuf[128];
    const char *errbuf_ptr = errbuf;

    if (av_strerror(err, errbuf, sizeof(errbuf)) < 0)
        errbuf_ptr = strerror(AVUNERROR(err));
    av_log(NULL, AV_LOG_ERROR, "%s: %s\n", filename, errbuf_ptr);
}

#define INDENT 1
#define SHOW_VERSION 2
#define SHOW_CONFIG 4
#define SHOW_COPYRIGHT 8

static char get_media_type_char(enum AVMediaType type)
{
    switch (type)
    {
    case AVMEDIA_TYPE_VIDEO:
        return 'V';
    case AVMEDIA_TYPE_AUDIO:
        return 'A';
    case AVMEDIA_TYPE_DATA:
        return 'D';
    case AVMEDIA_TYPE_SUBTITLE:
        return 'S';
    case AVMEDIA_TYPE_ATTACHMENT:
        return 'T';
    default:
        return '?';
    }
}

static const AVCodec *next_codec_for_id(enum AVCodecID id, void **iter,
                                        int encoder)
{
    const AVCodec *c;
    while ((c = av_codec_iterate(iter)))
    {
        if (c->id == id &&
            (encoder ? av_codec_is_encoder(c) : av_codec_is_decoder(c)))
            return c;
    }
    return NULL;
}

static int compare_codec_desc(const void *a, const void *b)
{
    const AVCodecDescriptor *const *da = a;
    const AVCodecDescriptor *const *db = b;

    return (*da)->type != (*db)->type ? FFDIFFSIGN((*da)->type, (*db)->type) : strcmp((*da)->name, (*db)->name);
}

const AVCodecDescriptor *avcodec_descriptor_next(const AVCodecDescriptor *prev)
{
    if (!prev)
        return &codec_descriptors[0];
    if (prev - codec_descriptors < FF_ARRAY_ELEMS(codec_descriptors) - 1)
        return prev + 1;
    return NULL;
}

static unsigned get_codecs_sorted(const AVCodecDescriptor ***rcodecs)
{
    const AVCodecDescriptor *desc = NULL;
    const AVCodecDescriptor **codecs;
    unsigned nb_codecs = 0, i = 0;

    while ((desc = avcodec_descriptor_next(desc)))
        nb_codecs++;
    if (!(codecs = av_calloc(nb_codecs, sizeof(*codecs))))
    {
        av_log(NULL, AV_LOG_ERROR, "Out of memory\n");
        exit_program(1);
    }
    desc = NULL;
    while ((desc = avcodec_descriptor_next(desc)))
        codecs[i++] = desc;
    av_assert0(i == nb_codecs);
    qsort(codecs, nb_codecs, sizeof(*codecs), compare_codec_desc);
    *rcodecs = codecs;
    return nb_codecs;
}

static void print_codecs_for_id(enum AVCodecID id, int encoder)
{
    void *iter = NULL;
    const AVCodec *codec;

    printf(" (%s: ", encoder ? "encoders" : "decoders");

    while ((codec = next_codec_for_id(id, &iter, encoder)))
        printf("%s ", codec->name);

    printf(")");
}

int show_codecs(void *optctx, const char *opt, const char *arg)
{
    const AVCodecDescriptor **codecs;
    unsigned i, nb_codecs = get_codecs_sorted(&codecs);

    printf("Codecs:\n"
           " D..... = Decoding supported\n"
           " .E.... = Encoding supported\n"
           " ..V... = Video codec\n"
           " ..A... = Audio codec\n"
           " ..S... = Subtitle codec\n"
           " ...I.. = Intra frame-only codec\n"
           " ....L. = Lossy compression\n"
           " .....S = Lossless compression\n"
           " -------\n");
    for (i = 0; i < nb_codecs; i++)
    {
        const AVCodecDescriptor *desc = codecs[i];
        const AVCodec *codec;
        void *iter = NULL;

        if (strstr(desc->name, "_deprecated"))
            continue;

        printf(" ");
        printf(avcodec_find_decoder(desc->id) ? "D" : ".");
        printf(avcodec_find_encoder(desc->id) ? "E" : ".");

        printf("%c", get_media_type_char(desc->type));
        printf((desc->props & AV_CODEC_PROP_INTRA_ONLY) ? "I" : ".");
        printf((desc->props & AV_CODEC_PROP_LOSSY) ? "L" : ".");
        printf((desc->props & AV_CODEC_PROP_LOSSLESS) ? "S" : ".");

        printf(" %-20s %s", desc->name, desc->long_name ? desc->long_name : "");

        /* print decoders/encoders when there's more than one or their
         * names are different from codec name */
        while ((codec = next_codec_for_id(desc->id, &iter, 0)))
        {
            if (strcmp(codec->name, desc->name))
            {
                print_codecs_for_id(desc->id, 0);
                break;
            }
        }
        iter = NULL;
        while ((codec = next_codec_for_id(desc->id, &iter, 1)))
        {
            if (strcmp(codec->name, desc->name))
            {
                print_codecs_for_id(desc->id, 1);
                break;
            }
        }

        printf("\n");
    }
    av_free(codecs);
    return 0;
}

static void print_codecs(int encoder)
{
    const AVCodecDescriptor **codecs;
    unsigned i, nb_codecs = get_codecs_sorted(&codecs);

    printf("%s:\n"
           " V..... = Video\n"
           " A..... = Audio\n"
           " S..... = Subtitle\n"
           " .F.... = Frame-level multithreading\n"
           " ..S... = Slice-level multithreading\n"
           " ...X.. = Codec is experimental\n"
           " ....B. = Supports draw_horiz_band\n"
           " .....D = Supports direct rendering method 1\n"
           " ------\n",
           encoder ? "Encoders" : "Decoders");
    for (i = 0; i < nb_codecs; i++)
    {
        const AVCodecDescriptor *desc = codecs[i];
        const AVCodec *codec;
        void *iter = NULL;

        while ((codec = next_codec_for_id(desc->id, &iter, encoder)))
        {
            printf(" %c", get_media_type_char(desc->type));
            printf((codec->capabilities & AV_CODEC_CAP_FRAME_THREADS) ? "F" : ".");
            printf((codec->capabilities & AV_CODEC_CAP_SLICE_THREADS) ? "S" : ".");
            printf((codec->capabilities & AV_CODEC_CAP_EXPERIMENTAL) ? "X" : ".");
            printf((codec->capabilities & AV_CODEC_CAP_DRAW_HORIZ_BAND) ? "B" : ".");
            printf((codec->capabilities & AV_CODEC_CAP_DR1) ? "D" : ".");

            printf(" %-20s %s", codec->name, codec->long_name ? codec->long_name : "");
            if (strcmp(codec->name, desc->name))
                printf(" (codec %s)", desc->name);

            printf("\n");
        }
    }
    av_free(codecs);
}

int show_decoders(void *optctx, const char *opt, const char *arg)
{
    print_codecs(0);
    return 0;
}

int show_encoders(void *optctx, const char *opt, const char *arg)
{
    print_codecs(1);
    return 0;
}

int show_bsfs(void *optctx, const char *opt, const char *arg)
{
    const AVBitStreamFilter *bsf = NULL;
    void *opaque = NULL;

    printf("Bitstream filters:\n");
    while ((bsf = av_bsf_iterate(&opaque)))
        printf("%s\n", bsf->name);
    printf("\n");
    return 0;
}

const char *avio_enum_protocols(void **opaque, int output)
{
    const URLProtocol **p = *opaque;

    p = p ? p + 1 : url_protocols;
    *opaque = p;
    if (!*p) {
        *opaque = NULL;
        return NULL;
    }
    if ((output && (*p)->url_write) || (!output && (*p)->url_read))
        return (*p)->name;
    return avio_enum_protocols(opaque, output);
}

int show_protocols(void *optctx, const char *opt, const char *arg)
{
    void *opaque = NULL;
    const char *name;

    printf("Supported file protocols:\n"
           "Input:\n");
    while ((name = avio_enum_protocols(&opaque, 0)))
        printf("  %s\n", name);
    printf("Output:\n");
    while ((name = avio_enum_protocols(&opaque, 1)))
        printf("  %s\n", name);
    return 0;
}

int show_filters(void *optctx, const char *opt, const char *arg)
{
#if CONFIG_AVFILTER
    const AVFilter *filter = NULL;
    char descr[64], *descr_cur;
    void *opaque = NULL;
    int i, j;
    const AVFilterPad *pad;

    printf("Filters:\n"
           "  T.. = Timeline support\n"
           "  .S. = Slice threading\n"
           "  ..C = Command support\n"
           "  A = Audio input/output\n"
           "  V = Video input/output\n"
           "  N = Dynamic number and/or type of input/output\n"
           "  | = Source or sink filter\n");
    while ((filter = av_filter_iterate(&opaque)))
    {
        descr_cur = descr;
        for (i = 0; i < 2; i++)
        {
            if (i)
            {
                *(descr_cur++) = '-';
                *(descr_cur++) = '>';
            }
            pad = i ? filter->outputs : filter->inputs;
            for (j = 0; pad && avfilter_pad_get_name(pad, j); j++)
            {
                if (descr_cur >= descr + sizeof(descr) - 4)
                    break;
                *(descr_cur++) = get_media_type_char(avfilter_pad_get_type(pad, j));
            }
            if (!j)
                *(descr_cur++) = ((!i && (filter->flags & AVFILTER_FLAG_DYNAMIC_INPUTS)) ||
                                  (i && (filter->flags & AVFILTER_FLAG_DYNAMIC_OUTPUTS)))
                                     ? 'N'
                                     : '|';
        }
        *descr_cur = 0;
        printf(" %c%c%c %-17s %-10s %s\n",
               filter->flags & AVFILTER_FLAG_SUPPORT_TIMELINE ? 'T' : '.',
               filter->flags & AVFILTER_FLAG_SLICE_THREADS ? 'S' : '.',
               filter->process_command ? 'C' : '.',
               filter->name, descr, filter->description);
    }
#else
    printf("No filters available: libavfilter disabled\n");
#endif
    return 0;
}

const char *av_get_known_color_name(int color_idx, const uint8_t **rgbp)
{
    const ColorEntry *color;

    if ((unsigned)color_idx >= FF_ARRAY_ELEMS(color_table))
        return NULL;

    color = &color_table[color_idx];
    if (rgbp)
        *rgbp = color->rgb_color;

    return color->name;
}

int show_colors(void *optctx, const char *opt, const char *arg)
{
    const char *name;
    const uint8_t *rgb;
    int i;

    printf("%-32s #RRGGBB\n", "name");

    for (i = 0; name = av_get_known_color_name(i, &rgb); i++)
        printf("%-32s #%02x%02x%02x\n", name, rgb[0], rgb[1], rgb[2]);

    return 0;
}

enum AVPixelFormat av_pix_fmt_desc_get_id(const AVPixFmtDescriptor *desc)
{
    if (desc < av_pix_fmt_descriptors ||
        desc >= av_pix_fmt_descriptors + FF_ARRAY_ELEMS(av_pix_fmt_descriptors))
        return AV_PIX_FMT_NONE;

    return desc - av_pix_fmt_descriptors;
}

const AVPixFmtDescriptor *av_pix_fmt_desc_next(const AVPixFmtDescriptor *prev)
{
    if (!prev)
        return &av_pix_fmt_descriptors[0];
    while (prev - av_pix_fmt_descriptors < FF_ARRAY_ELEMS(av_pix_fmt_descriptors) - 1) {
        prev++;
        if (prev->name)
            return prev;
    }
    return NULL;
}

int show_pix_fmts(void *optctx, const char *opt, const char *arg)
{
    const AVPixFmtDescriptor *pix_desc = NULL;

    printf("Pixel formats:\n"
           "I.... = Supported Input  format for conversion\n"
           ".O... = Supported Output format for conversion\n"
           "..H.. = Hardware accelerated format\n"
           "...P. = Paletted format\n"
           "....B = Bitstream format\n"
           "FLAGS NAME            NB_COMPONENTS BITS_PER_PIXEL\n"
           "-----\n");

#if !CONFIG_SWSCALE
#define sws_isSupportedInput(x) 0
#define sws_isSupportedOutput(x) 0
#endif

    while ((pix_desc = av_pix_fmt_desc_next(pix_desc)))
    {
        enum AVPixelFormat av_unused pix_fmt = av_pix_fmt_desc_get_id(pix_desc);
        printf("%c%c%c%c%c %-16s       %d            %2d\n",
               sws_isSupportedInput(pix_fmt) ? 'I' : '.',
               sws_isSupportedOutput(pix_fmt) ? 'O' : '.',
               pix_desc->flags & AV_PIX_FMT_FLAG_HWACCEL ? 'H' : '.',
               pix_desc->flags & AV_PIX_FMT_FLAG_PAL ? 'P' : '.',
               pix_desc->flags & AV_PIX_FMT_FLAG_BITSTREAM ? 'B' : '.',
               pix_desc->name,
               pix_desc->nb_components,
               av_get_bits_per_pixel(pix_desc));
    }
    return 0;
}

const char *av_get_channel_name(uint64_t channel)
{
    int i;
    if (av_get_channel_layout_nb_channels(channel) != 1)
        return NULL;
    for (i = 0; i < 64; i++)
        if ((1ULL<<i) & channel)
            return get_channel_name(i);
    return NULL;
}

const char *av_get_channel_description(uint64_t channel)
{
    int i;
    if (av_get_channel_layout_nb_channels(channel) != 1)
        return NULL;
    for (i = 0; i < FF_ARRAY_ELEMS(channel_names); i++)
        if ((1ULL<<i) & channel)
            return channel_names[i].description;
    return NULL;
}

int av_get_standard_channel_layout(unsigned index, uint64_t *layout,
                                   const char **name)
{
    if (index >= FF_ARRAY_ELEMS(channel_layout_map))
        return AVERROR_EOF;
    if (layout) *layout = channel_layout_map[index].layout;
    if (name)   *name   = channel_layout_map[index].name;
    return 0;
}

int show_layouts(void *optctx, const char *opt, const char *arg)
{
    int i = 0;
    uint64_t layout, j;
    const char *name, *descr;

    printf("Individual channels:\n"
           "NAME           DESCRIPTION\n");
    for (i = 0; i < 63; i++)
    {
        name = av_get_channel_name((uint64_t)1 << i);
        if (!name)
            continue;
        descr = av_get_channel_description((uint64_t)1 << i);
        printf("%-14s %s\n", name, descr);
    }
    printf("\nStandard channel layouts:\n"
           "NAME           DECOMPOSITION\n");
    for (i = 0; !av_get_standard_channel_layout(i, &layout, &name); i++)
    {
        if (name)
        {
            printf("%-14s ", name);
            for (j = 1; j; j <<= 1)
                if ((layout & j))
                    printf("%s%s", (layout & (j - 1)) ? "+" : "", av_get_channel_name(j));
            printf("\n");
        }
    }
    return 0;
}

char *av_get_sample_fmt_string (char *buf, int buf_size, enum AVSampleFormat sample_fmt)
{
    /* print header */
    if (sample_fmt < 0)
        snprintf(buf, buf_size, "name  " " depth");
    else if (sample_fmt < AV_SAMPLE_FMT_NB) {
        SampleFmtInfo info = sample_fmt_info[sample_fmt];
        snprintf (buf, buf_size, "%-6s" "   %2d ", info.name, info.bits);
    }

    return buf;
}

int show_sample_fmts(void *optctx, const char *opt, const char *arg)
{
    int i;
    char fmt_str[128];
    for (i = -1; i < AV_SAMPLE_FMT_NB; i++)
        printf("%s\n", av_get_sample_fmt_string(fmt_str, sizeof(fmt_str), i));
    return 0;
}

FILE *get_preset_file(char *filename, size_t filename_size,
                      const char *preset_name, int is_path,
                      const char *codec_name)
{
    FILE *f = NULL;
    int i;
    const char *base[3] = {
        getenv("FFMPEG_DATADIR"),
        getenv("HOME"),
        FFMPEG_DATADIR,
    };

    if (is_path)
    {
        av_strlcpy(filename, preset_name, filename_size);
        f = fopen(filename, "r");
    }
    else
    {
#if HAVE_GETMODULEHANDLE && defined(_WIN32)
        char datadir[MAX_PATH], *ls;
        base[2] = NULL;

        if (GetModuleFileNameA(GetModuleHandleA(NULL), datadir, sizeof(datadir) - 1))
        {
            for (ls = datadir; ls < datadir + strlen(datadir); ls++)
                if (*ls == '\\')
                    *ls = '/';

            if (ls = strrchr(datadir, '/'))
            {
                *ls = 0;
                strncat(datadir, "/ffpresets", sizeof(datadir) - 1 - strlen(datadir));
                base[2] = datadir;
            }
        }
#endif
        for (i = 0; i < 3 && !f; i++)
        {
            if (!base[i])
                continue;
            snprintf(filename, filename_size, "%s%s/%s.ffpreset", base[i],
                     i != 1 ? "" : "/.ffmpeg", preset_name);
            f = fopen(filename, "r");
            if (!f && codec_name)
            {
                snprintf(filename, filename_size,
                         "%s%s/%s-%s.ffpreset",
                         base[i], i != 1 ? "" : "/.ffmpeg", codec_name,
                         preset_name);
                f = fopen(filename, "r");
            }
        }
    }

    return f;
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


static int match_stream_specifier(AVFormatContext *s, AVStream *st,
                                  const char *spec, const char **indexptr, AVProgram **p)
{
    int match = 1; /* Stores if the specifier matches so far. */
    while (*spec)
    {
        if (*spec <= '9' && *spec >= '0')
        { /* opt:index */
            if (indexptr)
                *indexptr = spec;
            return match;
        }
        else if (*spec == 'v' || *spec == 'a' || *spec == 's' || *spec == 'd' ||
                 *spec == 't' || *spec == 'V')
        { /* opt:[vasdtV] */
            enum AVMediaType type;
            int nopic = 0;

            switch (*spec++)
            {
            case 'v':
                type = AVMEDIA_TYPE_VIDEO;
                break;
            case 'a':
                type = AVMEDIA_TYPE_AUDIO;
                break;
            case 's':
                type = AVMEDIA_TYPE_SUBTITLE;
                break;
            case 'd':
                type = AVMEDIA_TYPE_DATA;
                break;
            case 't':
                type = AVMEDIA_TYPE_ATTACHMENT;
                break;
            case 'V':
                type = AVMEDIA_TYPE_VIDEO;
                nopic = 1;
                break;
            default:
                av_assert0(0);
            }
            if (*spec && *spec++ != ':') /* If we are not at the end, then another specifier must follow. */
                return AVERROR(EINVAL);

#if FF_API_LAVF_AVCTX
            FF_DISABLE_DEPRECATION_WARNINGS
            if (type != st->codecpar->codec_type && (st->codecpar->codec_type != AVMEDIA_TYPE_UNKNOWN || st->codec->codec_type != type))
                match = 0;
            FF_ENABLE_DEPRECATION_WARNINGS
#else
            if (type != st->codecpar->codec_type)
                match = 0;
#endif
            if (nopic && (st->disposition & AV_DISPOSITION_ATTACHED_PIC))
                match = 0;
        }
        else if (*spec == 'p' && *(spec + 1) == ':')
        {
            int prog_id, i, j;
            int found = 0;
            char *endptr;
            spec += 2;
            prog_id = strtol(spec, &endptr, 0);
            /* Disallow empty id and make sure that if we are not at the end, then another specifier must follow. */
            if (spec == endptr || (*endptr && *endptr++ != ':'))
                return AVERROR(EINVAL);
            spec = endptr;
            if (match)
            {
                for (i = 0; i < s->nb_programs; i++)
                {
                    if (s->programs[i]->id != prog_id)
                        continue;

                    for (j = 0; j < s->programs[i]->nb_stream_indexes; j++)
                    {
                        if (st->index == s->programs[i]->stream_index[j])
                        {
                            found = 1;
                            if (p)
                                *p = s->programs[i];
                            i = s->nb_programs;
                            break;
                        }
                    }
                }
            }
            if (!found)
                match = 0;
        }
        else if (*spec == '#' ||
                 (*spec == 'i' && *(spec + 1) == ':'))
        {
            int stream_id;
            char *endptr;
            spec += 1 + (*spec == 'i');
            stream_id = strtol(spec, &endptr, 0);
            if (spec == endptr || *endptr) /* Disallow empty id and make sure we are at the end. */
                return AVERROR(EINVAL);
            return match && (stream_id == st->id);
        }
        else if (*spec == 'm' && *(spec + 1) == ':')
        {
            AVDictionaryEntry *tag;
            char *key, *val;
            int ret;

            if (match)
            {
                spec += 2;
                val = strchr(spec, ':');

                key = val ? av_strndup(spec, val - spec) : av_strdup(spec);
                if (!key)
                    return AVERROR(ENOMEM);

                tag = av_dict_get(st->metadata, key, NULL, 0);
                if (tag)
                {
                    if (!val || !strcmp(tag->value, val + 1))
                        ret = 1;
                    else
                        ret = 0;
                }
                else
                    ret = 0;

                av_freep(&key);
            }
            return match && ret;
        }
        else if (*spec == 'u' && *(spec + 1) == '\0')
        {
            AVCodecParameters *par = st->codecpar;
#if FF_API_LAVF_AVCTX
            FF_DISABLE_DEPRECATION_WARNINGS
            AVCodecContext *codec = st->codec;
            FF_ENABLE_DEPRECATION_WARNINGS
#endif
            int val;
            switch (par->codec_type)
            {
            case AVMEDIA_TYPE_AUDIO:
                val = par->sample_rate && par->channels;
#if FF_API_LAVF_AVCTX
                val = val || (codec->sample_rate && codec->channels);
#endif
                if (par->format == AV_SAMPLE_FMT_NONE
#if FF_API_LAVF_AVCTX
                    && codec->sample_fmt == AV_SAMPLE_FMT_NONE
#endif
                )
                    return 0;
                break;
            case AVMEDIA_TYPE_VIDEO:
                val = par->width && par->height;
#if FF_API_LAVF_AVCTX
                val = val || (codec->width && codec->height);
#endif
                if (par->format == AV_PIX_FMT_NONE
#if FF_API_LAVF_AVCTX
                    && codec->pix_fmt == AV_PIX_FMT_NONE
#endif
                )
                    return 0;
                break;
            case AVMEDIA_TYPE_UNKNOWN:
                val = 0;
                break;
            default:
                val = 1;
                break;
            }
#if FF_API_LAVF_AVCTX
            return match && ((par->codec_id != AV_CODEC_ID_NONE || codec->codec_id != AV_CODEC_ID_NONE) && val != 0);
#else
            return match && (par->codec_id != AV_CODEC_ID_NONE && val != 0);
#endif
        }
        else
        {
            return AVERROR(EINVAL);
        }
    }

    return match;
}

int avformat_match_stream_specifier(AVFormatContext *s, AVStream *st,
                                    const char *spec)
{
    int ret, index;
    char *endptr;
    const char *indexptr = NULL;
    AVProgram *p = NULL;
    int nb_streams;

    ret = match_stream_specifier(s, st, spec, &indexptr, &p);
    if (ret < 0)
        goto error;

    if (!indexptr)
        return ret;

    index = strtol(indexptr, &endptr, 0);
    if (*endptr)
    { /* We can't have anything after the requested index. */
        ret = AVERROR(EINVAL);
        goto error;
    }

    /* This is not really needed but saves us a loop for simple stream index specifiers. */
    if (spec == indexptr)
        return (index == st->index);

    /* If we requested a matching stream index, we have to ensure st is that. */
    nb_streams = p ? p->nb_stream_indexes : s->nb_streams;
    for (int i = 0; i < nb_streams && index >= 0; i++)
    {
        AVStream *candidate = p ? s->streams[p->stream_index[i]] : s->streams[i];
        ret = match_stream_specifier(s, candidate, spec, NULL, NULL);
        if (ret < 0)
            goto error;
        if (ret > 0 && index-- == 0 && st == candidate)
            return 1;
    }
    return 0;

error:
    if (ret == AVERROR(EINVAL))
        av_log(s, AV_LOG_ERROR, "Invalid stream specifier: %s.\n", spec);
    return ret;
}

int check_stream_specifier(AVFormatContext *s, AVStream *st, const char *spec)
{
    int ret = avformat_match_stream_specifier(s, st, spec);
    if (ret < 0)
        av_log(s, AV_LOG_ERROR, "Invalid stream specifier: %s.\n", spec);
    return ret;
}

static inline int av_size_mult(size_t a, size_t b, size_t *r)
{
    size_t t = a * b;
    if ((a | b) >= ((size_t)1 << (sizeof(size_t) * 4)) && a && t / a != b)
        return AVERROR(EINVAL);
    *r = t;
    return 0;
}

void *av_realloc_array(void *ptr, size_t nmemb, size_t size)
{
    size_t result;
    if (av_size_mult(nmemb, size, &result) < 0)
        return NULL;
    return av_realloc(ptr, result);
}

void *grow_array(void *array, int elem_size, int *size, int new_size)
{
    if (new_size >= INT_MAX / elem_size)
    {
        av_log(NULL, AV_LOG_ERROR, "Array too big.\n");
        exit_program(1);
    }
    if (*size < new_size)
    {
        uint8_t *tmp = av_realloc_array(array, new_size, elem_size);
        if (!tmp)
        {
            av_log(NULL, AV_LOG_ERROR, "Could not alloc buffer.\n");
            exit_program(1);
        }
        memset(tmp + *size * elem_size, 0, (new_size - *size) * elem_size);
        *size = new_size;
        return tmp;
    }
    return array;
}

int ff_alloc_input_device_context(AVFormatContext **avctx, AVInputFormat *iformat, const char *format)
{
    AVFormatContext *s;
    int ret = 0;

    *avctx = NULL;
    if (!iformat && !format)
        return AVERROR(EINVAL);
    if (!(s = avformat_alloc_context()))
        return AVERROR(ENOMEM);

    if (!iformat)
        iformat = av_find_input_format(format);
    if (!iformat || !iformat->priv_class || !AV_IS_INPUT_DEVICE(iformat->priv_class->category)) {
        ret = AVERROR(EINVAL);
        goto error;
    }
    s->iformat = iformat;
    if (s->iformat->priv_data_size > 0) {
        s->priv_data = av_mallocz(s->iformat->priv_data_size);
        if (!s->priv_data) {
            ret = AVERROR(ENOMEM);
            goto error;
        }
        if (s->iformat->priv_class) {
            *(const AVClass**)s->priv_data= s->iformat->priv_class;
            av_opt_set_defaults(s->priv_data);
        }
    } else
        s->priv_data = NULL;

    *avctx = s;
    return 0;
  error:
    avformat_free_context(s);
    return ret;
}


static int list_devices_for_context(AVFormatContext *s, AVDictionary *options,
                                    AVDeviceInfoList **device_list)
{
    AVDictionary *tmp = NULL;
    int ret;

    av_dict_copy(&tmp, options, 0);
    if ((ret = av_opt_set_dict2(s, &tmp, AV_OPT_SEARCH_CHILDREN)) < 0)
        goto fail;
    ret = avdevice_list_devices(s, device_list);
  fail:
    av_dict_free(&tmp);
    avformat_free_context(s);
    return ret;
}

int avdevice_list_input_sources(AVInputFormat *device, const char *device_name,
                                AVDictionary *device_options, AVDeviceInfoList **device_list)
{
    AVFormatContext *s = NULL;
    int ret;

    if ((ret = ff_alloc_input_device_context(&s, device, device_name)) < 0)
        return ret;
    return list_devices_for_context(s, device_options, device_list);
}

avformat_alloc_output_context2(AVFormatContext **avctx, ff_const59 AVOutputFormat *oformat,
                                   const char *format, const char *filename)
{
    AVFormatContext *s = avformat_alloc_context();
    int ret = 0;

    *avctx = NULL;
    if (!s)
        goto nomem;

    if (!oformat) {
        if (format) {
            oformat = av_guess_format(format, NULL, NULL);
            if (!oformat) {
                av_log(s, AV_LOG_ERROR, "Requested output format '%s' is not a suitable output format\n", format);
                ret = AVERROR(EINVAL);
                goto error;
            }
        } else {
            oformat = av_guess_format(NULL, filename, NULL);
            if (!oformat) {
                ret = AVERROR(EINVAL);
                av_log(s, AV_LOG_ERROR, "Unable to find a suitable output format for '%s'\n",
                       filename);
                goto error;
            }
        }
    }

    s->oformat = oformat;
    if (s->oformat->priv_data_size > 0) {
        s->priv_data = av_mallocz(s->oformat->priv_data_size);
        if (!s->priv_data)
            goto nomem;
        if (s->oformat->priv_class) {
            *(const AVClass**)s->priv_data= s->oformat->priv_class;
            av_opt_set_defaults(s->priv_data);
        }
    } else
        s->priv_data = NULL;

    if (filename) {
#if FF_API_FORMAT_FILENAME
FF_DISABLE_DEPRECATION_WARNINGS
        av_strlcpy(s->filename, filename, sizeof(s->filename));
FF_ENABLE_DEPRECATION_WARNINGS
#endif
        if (!(s->url = av_strdup(filename)))
            goto nomem;

    }
    *avctx = s;
    return 0;
nomem:
    av_log(s, AV_LOG_ERROR, "Out of memory\n");
    ret = AVERROR(ENOMEM);
error:
    avformat_free_context(s);
    return ret;
}


int avdevice_list_output_sinks(AVOutputFormat *device, const char *device_name,
                               AVDictionary *device_options, AVDeviceInfoList **device_list)
{
    AVFormatContext *s = NULL;
    int ret;

    if ((ret = avformat_alloc_output_context2(&s, device, device_name, NULL)) < 0)
        return ret;
    return list_devices_for_context(s, device_options, device_list);
}

void avdevice_free_list_devices(AVDeviceInfoList **device_list)
{
    AVDeviceInfoList *list;
    AVDeviceInfo *dev;
    int i;

    av_assert0(device_list);
    list = *device_list;
    if (!list)
        return;

    for (i = 0; i < list->nb_devices; i++)
    {
        dev = list->devices[i];
        if (dev)
        {
            av_freep(&dev->device_name);
            av_freep(&dev->device_description);
            av_free(dev);
        }
    }
    av_freep(&list->devices);
    av_freep(device_list);
}

#if CONFIG_AVDEVICE
static int print_device_sources(AVInputFormat *fmt, AVDictionary *opts)
{
    int ret, i;
    AVDeviceInfoList *device_list = NULL;

    if (!fmt || !fmt->priv_class || !AV_IS_INPUT_DEVICE(fmt->priv_class->category))
        return AVERROR(EINVAL);

    printf("Auto-detected sources for %s:\n", fmt->name);
    if (!fmt->get_device_list)
    {
        ret = AVERROR(ENOSYS);
        printf("Cannot list sources. Not implemented.\n");
        goto fail;
    }

    if ((ret = avdevice_list_input_sources(fmt, NULL, opts, &device_list)) < 0)
    {
        printf("Cannot list sources.\n");
        goto fail;
    }

    for (i = 0; i < device_list->nb_devices; i++)
    {
        printf("%s %s [%s]\n", device_list->default_device == i ? "*" : " ",
               device_list->devices[i]->device_name, device_list->devices[i]->device_description);
    }

fail:
    avdevice_free_list_devices(&device_list);
    return ret;
}

static int print_device_sinks(AVOutputFormat *fmt, AVDictionary *opts)
{
    int ret, i;
    AVDeviceInfoList *device_list = NULL;

    if (!fmt || !fmt->priv_class || !AV_IS_OUTPUT_DEVICE(fmt->priv_class->category))
        return AVERROR(EINVAL);

    printf("Auto-detected sinks for %s:\n", fmt->name);
    if (!fmt->get_device_list)
    {
        ret = AVERROR(ENOSYS);
        printf("Cannot list sinks. Not implemented.\n");
        goto fail;
    }

    if ((ret = avdevice_list_output_sinks(fmt, NULL, opts, &device_list)) < 0)
    {
        printf("Cannot list sinks.\n");
        goto fail;
    }

    for (i = 0; i < device_list->nb_devices; i++)
    {
        printf("%s %s [%s]\n", device_list->default_device == i ? "*" : " ",
               device_list->devices[i]->device_name, device_list->devices[i]->device_description);
    }

fail:
    avdevice_free_list_devices(&device_list);
    return ret;
}


int av_dict_set(AVDictionary **pm, const char *key, const char *value,
                int flags)
{
    AVDictionary *m = *pm;
    AVDictionaryEntry *tag = NULL;
    char *oldval = NULL, *copy_key = NULL, *copy_value = NULL;

    if (!(flags & AV_DICT_MULTIKEY))
    {
        tag = av_dict_get(m, key, NULL, flags);
    }
    if (flags & AV_DICT_DONT_STRDUP_KEY)
        copy_key = (void *)key;
    else
        copy_key = av_strdup(key);
    if (flags & AV_DICT_DONT_STRDUP_VAL)
        copy_value = (void *)value;
    else if (copy_key)
        copy_value = av_strdup(value);
    if (!m)
        m = *pm = av_mallocz(sizeof(*m));
    if (!m || (key && !copy_key) || (value && !copy_value))
        goto err_out;

    if (tag)
    {
        if (flags & AV_DICT_DONT_OVERWRITE)
        {
            av_free(copy_key);
            av_free(copy_value);
            return 0;
        }
        if (flags & AV_DICT_APPEND)
            oldval = tag->value;
        else
            av_free(tag->value);
        av_free(tag->key);
        *tag = m->elems[--m->count];
    }
    else if (copy_value)
    {
        AVDictionaryEntry *tmp = av_realloc_array(m->elems,
                                                  m->count + 1, sizeof(*m->elems));
        if (!tmp)
            goto err_out;
        m->elems = tmp;
    }
    if (copy_value)
    {
        m->elems[m->count].key = copy_key;
        m->elems[m->count].value = copy_value;
        if (oldval && flags & AV_DICT_APPEND)
        {
            size_t len = strlen(oldval) + strlen(copy_value) + 1;
            char *newval = av_mallocz(len);
            if (!newval)
                goto err_out;
            av_strlcat(newval, oldval, len);
            av_freep(&oldval);
            av_strlcat(newval, copy_value, len);
            m->elems[m->count].value = newval;
            av_freep(&copy_value);
        }
        m->count++;
    }
    else
    {
        av_freep(&copy_key);
    }
    if (!m->count)
    {
        av_freep(&m->elems);
        av_freep(pm);
    }

    return 0;

err_out:
    if (m && !m->count)
    {
        av_freep(&m->elems);
        av_freep(pm);
    }
    av_free(copy_key);
    av_free(copy_value);
    return AVERROR(ENOMEM);
}

static int parse_key_value_pair(AVDictionary **pm, const char **buf,
                                const char *key_val_sep, const char *pairs_sep,
                                int flags)
{
    char *key = av_get_token(buf, key_val_sep);
    char *val = NULL;
    int ret;

    if (key && *key && strspn(*buf, key_val_sep))
    {
        (*buf)++;
        val = av_get_token(buf, pairs_sep);
    }

    if (key && *key && val && *val)
        ret = av_dict_set(pm, key, val, flags);
    else
        ret = AVERROR(EINVAL);

    av_freep(&key);
    av_freep(&val);

    return ret;
}

int av_dict_parse_string(AVDictionary **pm, const char *str,
                         const char *key_val_sep, const char *pairs_sep,
                         int flags)
{
    int ret;

    if (!str)
        return 0;

    /* ignore STRDUP flags */
    flags &= ~(AV_DICT_DONT_STRDUP_KEY | AV_DICT_DONT_STRDUP_VAL);

    while (*str)
    {
        if ((ret = parse_key_value_pair(pm, &str, key_val_sep, pairs_sep, flags)) < 0)
            return ret;

        if (*str)
            str++;
    }

    return 0;
}

static int show_sinks_sources_parse_arg(const char *arg, char **dev, AVDictionary **opts)
{
    int ret;
    if (arg)
    {
        char *opts_str = NULL;
        av_assert0(dev && opts);
        *dev = av_strdup(arg);
        if (!*dev)
            return AVERROR(ENOMEM);
        if ((opts_str = strchr(*dev, ',')))
        {
            *(opts_str++) = '\0';
            if (opts_str[0] && ((ret = av_dict_parse_string(opts, opts_str, "=", ":", 0)) < 0))
            {
                av_freep(dev);
                return ret;
            }
        }
    }
    else
        printf("\nDevice name is not provided.\n"
               "You can pass devicename[,opt1=val1[,opt2=val2...]] as an argument.\n\n");
    return 0;
}

static inline av_const int av_tolower(int c)
{
    if (c >= 'A' && c <= 'Z')
        c ^= 0x20;
    return c;
}

int av_strncasecmp(const char *a, const char *b, size_t n)
{
    uint8_t c1, c2;
    if (n <= 0)
        return 0;
    do
    {
        c1 = av_tolower(*a++);
        c2 = av_tolower(*b++);
    } while (--n && c1 && c1 == c2);
    return c1 - c2;
}

int av_match_name(const char *name, const char *names)
{
    const char *p;
    int len, namelen;

    if (!name || !names)
        return 0;

    namelen = strlen(name);
    while (*names)
    {
        int negate = '-' == *names;
        p = strchr(names, ',');
        if (!p)
            p = names + strlen(names);
        names += negate;
        len = FFMAX(p - names, namelen);
        if (!av_strncasecmp(name, names, len) || !strncmp("ALL", names, FFMAX(3, p - names)))
            return !negate;
        names = p + (*p == ',');
    }
    return 0;
}

static AVOnce av_format_next_init = AV_ONCE_INIT;

static void av_format_init_next(void)
{
    AVOutputFormat *prevout = NULL, *out;
    AVInputFormat *previn = NULL, *in;

    ff_mutex_lock(&avpriv_register_devices_mutex);

    for (int i = 0; (out = (AVOutputFormat *)muxer_list[i]); i++)
    {
        if (prevout)
            prevout->next = out;
        prevout = out;
    }

    if (outdev_list)
    {
        for (int i = 0; (out = (AVOutputFormat *)outdev_list[i]); i++)
        {
            if (prevout)
                prevout->next = out;
            prevout = out;
        }
    }

    for (int i = 0; (in = (AVInputFormat *)demuxer_list[i]); i++)
    {
        if (previn)
            previn->next = in;
        previn = in;
    }

    if (indev_list)
    {
        for (int i = 0; (in = (AVInputFormat *)indev_list[i]); i++)
        {
            if (previn)
                previn->next = in;
            previn = in;
        }
    }

    ff_mutex_unlock(&avpriv_register_devices_mutex);
}

AVOutputFormat *av_oformat_next(const AVOutputFormat *f)
{
    ff_thread_once(&av_format_next_init, av_format_init_next);

    if (f)
#if FF_API_AVIOFORMAT
        return f->next;
#else
        return (AVOutputFormat *)f->next;
#endif
    else
    {
        void *opaque = NULL;
        return (AVOutputFormat *)av_muxer_iterate(&opaque);
    }
}

AVInputFormat *av_iformat_next(const AVInputFormat *f)
{
    ff_thread_once(&av_format_next_init, av_format_init_next);

    if (f)
#if FF_API_AVIOFORMAT
        return f->next;
#else
        return (AVInputFormat *) f->next;
#endif
    else {
        void *opaque = NULL;
        return (AVInputFormat *)av_demuxer_iterate(&opaque);
    }
}

static void *device_next(void *prev, int output,
                         AVClassCategory c1, AVClassCategory c2)
{
    const AVClass *pc;
    AVClassCategory category = AV_CLASS_CATEGORY_NA;
    do
    {
        if (output)
        {
            if (!(prev = av_oformat_next(prev)))
                break;
            pc = ((AVOutputFormat *)prev)->priv_class;
        }
        else
        {
            if (!(prev = av_iformat_next(prev)))
                break;
            pc = ((AVInputFormat *)prev)->priv_class;
        }
        if (!pc)
            continue;
        category = pc->category;
    } while (category != c1 && category != c2);
    return prev;
}

AVInputFormat *av_input_audio_device_next(AVInputFormat *d)
{
    return device_next(d, 0, AV_CLASS_CATEGORY_DEVICE_AUDIO_INPUT,
                       AV_CLASS_CATEGORY_DEVICE_INPUT);
}

AVInputFormat *av_input_video_device_next(AVInputFormat *d)
{
    return device_next(d, 0, AV_CLASS_CATEGORY_DEVICE_VIDEO_INPUT,
                       AV_CLASS_CATEGORY_DEVICE_INPUT);
}

AVOutputFormat *av_output_audio_device_next(AVOutputFormat *d)
{
    return device_next(d, 1, AV_CLASS_CATEGORY_DEVICE_AUDIO_OUTPUT,
                       AV_CLASS_CATEGORY_DEVICE_OUTPUT);
}

AVOutputFormat *av_output_video_device_next(AVOutputFormat *d)
{
    return device_next(d, 1, AV_CLASS_CATEGORY_DEVICE_VIDEO_OUTPUT,
                       AV_CLASS_CATEGORY_DEVICE_OUTPUT);
}

int show_sources(void *optctx, const char *opt, const char *arg)
{
    AVInputFormat *fmt = NULL;
    char *dev = NULL;
    AVDictionary *opts = NULL;
    int ret = 0;
    int error_level = av_log_get_level();

    av_log_set_level(AV_LOG_ERROR);

    if ((ret = show_sinks_sources_parse_arg(arg, &dev, &opts)) < 0)
        goto fail;

    do
    {
        fmt = av_input_audio_device_next(fmt);
        if (fmt)
        {
            if (!strcmp(fmt->name, "lavfi"))
                continue; //it's pointless to probe lavfi
            if (dev && !av_match_name(dev, fmt->name))
                continue;
            print_device_sources(fmt, opts);
        }
    } while (fmt);
    do
    {
        fmt = av_input_video_device_next(fmt);
        if (fmt)
        {
            if (dev && !av_match_name(dev, fmt->name))
                continue;
            print_device_sources(fmt, opts);
        }
    } while (fmt);
fail:
    av_dict_free(&opts);
    av_free(dev);
    av_log_set_level(error_level);
    return ret;
}

int show_sinks(void *optctx, const char *opt, const char *arg)
{
    AVOutputFormat *fmt = NULL;
    char *dev = NULL;
    AVDictionary *opts = NULL;
    int ret = 0;
    int error_level = av_log_get_level();

    av_log_set_level(AV_LOG_ERROR);

    if ((ret = show_sinks_sources_parse_arg(arg, &dev, &opts)) < 0)
        goto fail;

    do
    {
        fmt = av_output_audio_device_next(fmt);
        if (fmt)
        {
            if (dev && !av_match_name(dev, fmt->name))
                continue;
            print_device_sinks(fmt, opts);
        }
    } while (fmt);
    do
    {
        fmt = av_output_video_device_next(fmt);
        if (fmt)
        {
            if (dev && !av_match_name(dev, fmt->name))
                continue;
            print_device_sinks(fmt, opts);
        }
    } while (fmt);
fail:
    av_dict_free(&opts);
    av_free(dev);
    av_log_set_level(error_level);
    return ret;
}

#endif

static AVMutex avformat_mutex = AV_MUTEX_INITIALIZER;

int ff_lock_avformat(void)
{
    return ff_mutex_lock(&avformat_mutex) ? -1 : 0;
}

int ff_network_init(void)
{
#if HAVE_WINSOCK2_H
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(1, 1), &wsaData))
        return 0;
#endif
    return 1;
}

int avformat_network_init(void)
{
    int ret;
    if ((ret = ff_network_init()) < 0)
        return ret;
    return 0;
}

void avpriv_register_devices(const AVOutputFormat *const o[], const AVInputFormat *const i[])
{
    ff_mutex_lock(&avpriv_register_devices_mutex);
    outdev_list = o;
    indev_list = i;
    ff_mutex_unlock(&avpriv_register_devices_mutex);
#if FF_API_NEXT
    av_format_init_next();
#endif
}

void avdevice_register_all(void)
{
    avpriv_register_devices(outdev_list, indev_list);
}

int main(int argc, char **argv)
{
    int flags;
    VideoState *is;

    init_dynload();

    av_log_set_flags(AV_LOG_SKIP_REPEATED);
    parse_loglevel(argc, argv, options);

    avdevice_register_all();
    avformat_network_init();

    init_opts();

    signal(SIGINT, sigterm_handler);  /* Interrupt (ANSI).    */
    signal(SIGTERM, sigterm_handler); /* Termination (ANSI).  */

    show_banner(argc, argv, options);

    parse_options(NULL, argc, argv, options, opt_input_file);

    if (!input_filename)
    {
        show_usage();
        av_log(NULL, AV_LOG_FATAL, "An input file must be specified\n");
        av_log(NULL, AV_LOG_FATAL,
               "Use -h to get full help or, even better, run 'man %s'\n", program_name);
        exit(1);
    }

    if (display_disable)
    {
        video_disable = 1;
    }
    flags = SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER;
    if (audio_disable)
        flags &= ~SDL_INIT_AUDIO;
    else
    {
        if (!SDL_getenv("SDL_AUDIO_ALSA_SET_BUFFER_SIZE"))
            SDL_setenv("SDL_AUDIO_ALSA_SET_BUFFER_SIZE", "1", 1);
    }
    if (display_disable)
        flags &= ~SDL_INIT_VIDEO;
    if (SDL_Init(flags))
    {
        av_log(NULL, AV_LOG_FATAL, "Could not initialize SDL - %s\n", SDL_GetError());
        av_log(NULL, AV_LOG_FATAL, "(Did you set the DISPLAY variable?)\n");
        exit(1);
    }

    SDL_EventState(SDL_SYSWMEVENT, SDL_IGNORE);
    SDL_EventState(SDL_USEREVENT, SDL_IGNORE);

    av_init_packet(&flush_pkt);
    flush_pkt.data = (uint8_t *)&flush_pkt;

    if (!display_disable)
    {
        int flags = SDL_WINDOW_HIDDEN;
        if (alwaysontop)
            flags |= SDL_WINDOW_ALWAYS_ON_TOP;
        if (borderless)
            flags |= SDL_WINDOW_BORDERLESS;
        else
            flags |= SDL_WINDOW_RESIZABLE;
        window = SDL_CreateWindow(program_name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, default_width, default_height, flags);
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
        if (window)
        {
            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
            if (!renderer)
            {
                av_log(NULL, AV_LOG_WARNING, "Failed to initialize a hardware accelerated renderer: %s\n", SDL_GetError());
                renderer = SDL_CreateRenderer(window, -1, 0);
            }
            if (renderer)
            {
                if (!SDL_GetRendererInfo(renderer, &renderer_info))
                    av_log(NULL, AV_LOG_VERBOSE, "Initialized %s renderer.\n", renderer_info.name);
            }
        }
        if (!window || !renderer || !renderer_info.num_texture_formats)
        {
            av_log(NULL, AV_LOG_FATAL, "Failed to create window or renderer: %s", SDL_GetError());
            do_exit(NULL);
        }
    }

    is = stream_open(input_filename, file_iformat);
    if (!is)
    {
        av_log(NULL, AV_LOG_FATAL, "Failed to initialize VideoState!\n");
        do_exit(NULL);
    }
    event_loop(is);
    return 0;
}

int ff_mpeg4audio_get_config_gb(MPEG4AudioConfig *c, GetBitContext *gb,
                                int sync_extension, void *logctx)
{
    int specific_config_bitindex, ret;
    int start_bit_index = get_bits_count(gb);
    c->object_type = get_object_type(gb);
    c->sample_rate = get_sample_rate(gb, &c->sampling_index);
    c->chan_config = get_bits(gb, 4);
    if (c->chan_config < FF_ARRAY_ELEMS(ff_mpeg4audio_channels))
        c->channels = ff_mpeg4audio_channels[c->chan_config];
    else {
        av_log(logctx, AV_LOG_ERROR, "Invalid chan_config %d\n", c->chan_config);
        return AVERROR_INVALIDDATA;
    }
    c->sbr = -1;
    c->ps  = -1;
    if (c->object_type == AOT_SBR || (c->object_type == AOT_PS &&
        // check for W6132 Annex YYYY draft MP3onMP4
        !(show_bits(gb, 3) & 0x03 && !(show_bits(gb, 9) & 0x3F)))) {
        if (c->object_type == AOT_PS)
            c->ps = 1;
        c->ext_object_type = AOT_SBR;
        c->sbr = 1;
        c->ext_sample_rate = get_sample_rate(gb, &c->ext_sampling_index);
        c->object_type = get_object_type(gb);
        if (c->object_type == AOT_ER_BSAC)
            c->ext_chan_config = get_bits(gb, 4);
    } else {
        c->ext_object_type = AOT_NULL;
        c->ext_sample_rate = 0;
    }
    specific_config_bitindex = get_bits_count(gb);

    if (c->object_type == AOT_ALS) {
        skip_bits(gb, 5);
        if (show_bits(gb, 24) != MKBETAG('\0','A','L','S'))
            skip_bits(gb, 24);

        specific_config_bitindex = get_bits_count(gb);

        ret = parse_config_ALS(gb, c);
        if (ret < 0)
            return ret;
    }

    if (c->ext_object_type != AOT_SBR && sync_extension) {
        while (get_bits_left(gb) > 15) {
            if (show_bits(gb, 11) == 0x2b7) { // sync extension
                get_bits(gb, 11);
                c->ext_object_type = get_object_type(gb);
                if (c->ext_object_type == AOT_SBR && (c->sbr = get_bits1(gb)) == 1) {
                    c->ext_sample_rate = get_sample_rate(gb, &c->ext_sampling_index);
                    if (c->ext_sample_rate == c->sample_rate)
                        c->sbr = -1;
                }
                if (get_bits_left(gb) > 11 && get_bits(gb, 11) == 0x548)
                    c->ps = get_bits1(gb);
                break;
            } else
                get_bits1(gb); // skip 1 bit
        }
    }

    //PS requires SBR
    if (!c->sbr)
        c->ps = 0;
    //Limit implicit PS to the HE-AACv2 Profile
    if ((c->ps == -1 && c->object_type != AOT_AAC_LC) || c->channels & ~0x01)
        c->ps = 0;

    return specific_config_bitindex - start_bit_index;
}

int avpriv_mpeg4audio_get_config2(MPEG4AudioConfig *c, const uint8_t *buf,
                                  int size, int sync_extension, void *logctx)
{
    GetBitContext gb;
    int ret;

    if (size <= 0)
        return AVERROR_INVALIDDATA;

    ret = init_get_bits8(&gb, buf, size);
    if (ret < 0)
        return ret;

    return ff_mpeg4audio_get_config_gb(c, &gb, sync_extension, logctx);
}

static void writeout(AVIOContext *s, const uint8_t *data, int len)
{
    if (!s->error) {
        int ret = 0;
        if (s->write_data_type)
            ret = s->write_data_type(s->opaque, (uint8_t *)data,
                                     len,
                                     s->current_type,
                                     s->last_time);
        else if (s->write_packet)
            ret = s->write_packet(s->opaque, (uint8_t *)data, len);
        if (ret < 0) {
            s->error = ret;
        } else {
            if (s->pos + len > s->written)
                s->written = s->pos + len;
        }
    }
    if (s->current_type == AVIO_DATA_MARKER_SYNC_POINT ||
        s->current_type == AVIO_DATA_MARKER_BOUNDARY_POINT) {
        s->current_type = AVIO_DATA_MARKER_UNKNOWN;
    }
    s->last_time = AV_NOPTS_VALUE;
    s->writeout_count ++;
    s->pos += len;
}

static void flush_buffer(AVIOContext *s)
{
    s->buf_ptr_max = FFMAX(s->buf_ptr, s->buf_ptr_max);
    if (s->write_flag && s->buf_ptr_max > s->buffer) {
        writeout(s, s->buffer, s->buf_ptr_max - s->buffer);
        if (s->update_checksum) {
            s->checksum     = s->update_checksum(s->checksum, s->checksum_ptr,
                                                 s->buf_ptr_max - s->checksum_ptr);
            s->checksum_ptr = s->buffer;
        }
    }
    s->buf_ptr = s->buf_ptr_max = s->buffer;
    if (!s->write_flag)
        s->buf_end = s->buffer;
}

void avio_write(AVIOContext *s, const unsigned char *buf, int size)
{
    if (s->direct && !s->update_checksum) {
        avio_flush(s);
        writeout(s, buf, size);
        return;
    }
    while (size > 0) {
        int len = FFMIN(s->buf_end - s->buf_ptr, size);
        memcpy(s->buf_ptr, buf, len);
        s->buf_ptr += len;

        if (s->buf_ptr >= s->buf_end)
            flush_buffer(s);

        buf += len;
        size -= len;
    }
}

static void fill_buffer(AVIOContext *s)
{
    int max_buffer_size = s->max_packet_size ?
                          s->max_packet_size : IO_BUFFER_SIZE;
    uint8_t *dst        = s->buf_end - s->buffer + max_buffer_size <= s->buffer_size ?
                          s->buf_end : s->buffer;
    int len             = s->buffer_size - (dst - s->buffer);

    /* can't fill the buffer without read_packet, just set EOF if appropriate */
    if (!s->read_packet && s->buf_ptr >= s->buf_end)
        s->eof_reached = 1;

    /* no need to do anything if EOF already reached */
    if (s->eof_reached)
        return;

    if (s->update_checksum && dst == s->buffer) {
        if (s->buf_end > s->checksum_ptr)
            s->checksum = s->update_checksum(s->checksum, s->checksum_ptr,
                                             s->buf_end - s->checksum_ptr);
        s->checksum_ptr = s->buffer;
    }

    /* make buffer smaller in case it ended up large after probing */
    if (s->read_packet && s->orig_buffer_size && s->buffer_size > s->orig_buffer_size && len >= s->orig_buffer_size) {
        if (dst == s->buffer && s->buf_ptr != dst) {
            int ret = ffio_set_buf_size(s, s->orig_buffer_size);
            if (ret < 0)
                av_log(s, AV_LOG_WARNING, "Failed to decrease buffer size\n");

            s->checksum_ptr = dst = s->buffer;
        }
        len = s->orig_buffer_size;
    }

    len = read_packet_wrapper(s, dst, len);
    if (len == AVERROR_EOF) {
        /* do not modify buffer if EOF reached so that a seek back can
           be done without rereading data */
        s->eof_reached = 1;
    } else if (len < 0) {
        s->eof_reached = 1;
        s->error= len;
    } else {
        s->pos += len;
        s->buf_ptr = dst;
        s->buf_end = dst + len;
        s->bytes_read += len;
    }
}


int64_t avio_seek(AVIOContext *s, int64_t offset, int whence)
{
    int64_t offset1;
    int64_t pos;
    int force = whence & AVSEEK_FORCE;
    int buffer_size;
    int short_seek;
    whence &= ~AVSEEK_FORCE;

    if(!s)
        return AVERROR(EINVAL);

    if ((whence & AVSEEK_SIZE))
        return s->seek ? s->seek(s->opaque, offset, AVSEEK_SIZE) : AVERROR(ENOSYS);

    buffer_size = s->buf_end - s->buffer;
    // pos is the absolute position that the beginning of s->buffer corresponds to in the file
    pos = s->pos - (s->write_flag ? 0 : buffer_size);

    if (whence != SEEK_CUR && whence != SEEK_SET)
        return AVERROR(EINVAL);

    if (whence == SEEK_CUR) {
        offset1 = pos + (s->buf_ptr - s->buffer);
        if (offset == 0)
            return offset1;
        if (offset > INT64_MAX - offset1)
            return AVERROR(EINVAL);
        offset += offset1;
    }
    if (offset < 0)
        return AVERROR(EINVAL);

    if (s->short_seek_get) {
        short_seek = s->short_seek_get(s->opaque);
        /* fallback to default short seek */
        if (short_seek <= 0)
            short_seek = s->short_seek_threshold;
    } else
        short_seek = s->short_seek_threshold;

    offset1 = offset - pos; // "offset1" is the relative offset from the beginning of s->buffer
    s->buf_ptr_max = FFMAX(s->buf_ptr_max, s->buf_ptr);
    if ((!s->direct || !s->seek) &&
        offset1 >= 0 && offset1 <= (s->write_flag ? s->buf_ptr_max - s->buffer : buffer_size)) {
        /* can do the seek inside the buffer */
        s->buf_ptr = s->buffer + offset1;
    } else if ((!(s->seekable & AVIO_SEEKABLE_NORMAL) ||
               offset1 <= buffer_size + short_seek) &&
               !s->write_flag && offset1 >= 0 &&
               (!s->direct || !s->seek) &&
              (whence != SEEK_END || force)) {
        while(s->pos < offset && !s->eof_reached)
            fill_buffer(s);
        if (s->eof_reached)
            return AVERROR_EOF;
        s->buf_ptr = s->buf_end - (s->pos - offset);
    } else if(!s->write_flag && offset1 < 0 && -offset1 < buffer_size>>1 && s->seek && offset > 0) {
        int64_t res;

        pos -= FFMIN(buffer_size>>1, pos);
        if ((res = s->seek(s->opaque, pos, SEEK_SET)) < 0)
            return res;
        s->buf_end =
        s->buf_ptr = s->buffer;
        s->pos = pos;
        s->eof_reached = 0;
        fill_buffer(s);
        return avio_seek(s, offset, SEEK_SET | force);
    } else {
        int64_t res;
        if (s->write_flag) {
            flush_buffer(s);
        }
        if (!s->seek)
            return AVERROR(EPIPE);
        if ((res = s->seek(s->opaque, offset, SEEK_SET)) < 0)
            return res;
        s->seek_count ++;
        if (!s->write_flag)
            s->buf_end = s->buffer;
        s->buf_ptr = s->buf_ptr_max = s->buffer;
        s->pos = offset;
    }
    s->eof_reached = 0;
    return offset;
}


void avio_flush(AVIOContext *s)
{
    int seekback = s->write_flag ? FFMIN(0, s->buf_ptr - s->buf_ptr_max) : 0;
    flush_buffer(s);
    if (seekback)
        avio_seek(s, seekback, SEEK_CUR);
}


int64_t avio_skip(AVIOContext *s, int64_t offset)
{
    return avio_seek(s, offset, SEEK_CUR);
}


int avio_feof(AVIOContext *s)
{
    if(!s)
        return 0;
    if(s->eof_reached){
        s->eof_reached=0;
        fill_buffer(s);
    }
    return s->eof_reached;
}