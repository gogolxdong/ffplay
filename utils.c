

static const AVClass audio_data_class = {
    .class_name = "AudioData",
    .item_name  = av_default_item_name,
    .version    = LIBAVUTIL_VERSION_INT,
};

/*
 * Calculate alignment for data pointers.
 */
static void calc_ptr_alignment(AudioData *a)
{
    int p;
    int min_align = 128;

    for (p = 0; p < a->planes; p++) {
        int cur_align = 128;
        while ((intptr_t)a->data[p] % cur_align)
            cur_align >>= 1;
        if (cur_align < min_align)
            min_align = cur_align;
    }
    a->ptr_align = min_align;
}

int ff_sample_fmt_is_planar(enum AVSampleFormat sample_fmt, int channels)
{
    if (channels == 1)
        return 1;
    else
        return av_sample_fmt_is_planar(sample_fmt);
}

int ff_audio_data_set_channels(AudioData *a, int channels)
{
    if (channels < 1 || channels > AVRESAMPLE_MAX_CHANNELS ||
        channels > a->allocated_channels)
        return AVERROR(EINVAL);

    a->channels  = channels;
    a->planes    = a->is_planar ? channels : 1;

    calc_ptr_alignment(a);

    return 0;
}

int ff_audio_data_init(AudioData *a, uint8_t * const *src, int plane_size,
                       int channels, int nb_samples,
                       enum AVSampleFormat sample_fmt, int read_only,
                       const char *name)
{
    int p;

    memset(a, 0, sizeof(*a));
    a->class = &audio_data_class;

    if (channels < 1 || channels > AVRESAMPLE_MAX_CHANNELS) {
        av_log(a, AV_LOG_ERROR, "invalid channel count: %d\n", channels);
        return AVERROR(EINVAL);
    }

    a->sample_size = av_get_bytes_per_sample(sample_fmt);
    if (!a->sample_size) {
        av_log(a, AV_LOG_ERROR, "invalid sample format\n");
        return AVERROR(EINVAL);
    }
    a->is_planar = ff_sample_fmt_is_planar(sample_fmt, channels);
    a->planes    = a->is_planar ? channels : 1;
    a->stride    = a->sample_size * (a->is_planar ? 1 : channels);

    for (p = 0; p < (a->is_planar ? channels : 1); p++) {
        if (!src[p]) {
            av_log(a, AV_LOG_ERROR, "invalid NULL pointer for src[%d]\n", p);
            return AVERROR(EINVAL);
        }
        a->data[p] = src[p];
    }
    a->allocated_samples  = nb_samples * !read_only;
    a->nb_samples         = nb_samples;
    a->sample_fmt         = sample_fmt;
    a->channels           = channels;
    a->allocated_channels = channels;
    a->read_only          = read_only;
    a->allow_realloc      = 0;
    a->name               = name ? name : "{no name}";

    calc_ptr_alignment(a);
    a->samples_align = plane_size / a->stride;

    return 0;
}

int ff_audio_data_realloc(AudioData *a, int nb_samples)
{
    int ret, new_buf_size, plane_size, p;

    /* check if buffer is already large enough */
    if (a->allocated_samples >= nb_samples)
        return 0;

    /* validate that the output is not read-only and realloc is allowed */
    if (a->read_only || !a->allow_realloc)
        return AVERROR(EINVAL);

    new_buf_size = av_samples_get_buffer_size(&plane_size,
                                              a->allocated_channels, nb_samples,
                                              a->sample_fmt, 0);
    if (new_buf_size < 0)
        return new_buf_size;

    /* if there is already data in the buffer and the sample format is planar,
       allocate a new buffer and copy the data, otherwise just realloc the
       internal buffer and set new data pointers */
    if (a->nb_samples > 0 && a->is_planar) {
        uint8_t *new_data[AVRESAMPLE_MAX_CHANNELS] = { NULL };

        ret = av_samples_alloc(new_data, &plane_size, a->allocated_channels,
                               nb_samples, a->sample_fmt, 0);
        if (ret < 0)
            return ret;

        for (p = 0; p < a->planes; p++)
            memcpy(new_data[p], a->data[p], a->nb_samples * a->stride);

        av_freep(&a->buffer);
        memcpy(a->data, new_data, sizeof(new_data));
        a->buffer = a->data[0];
    } else {
        av_freep(&a->buffer);
        a->buffer = av_malloc(new_buf_size);
        if (!a->buffer)
            return AVERROR(ENOMEM);
        ret = av_samples_fill_arrays(a->data, &plane_size, a->buffer,
                                     a->allocated_channels, nb_samples,
                                     a->sample_fmt, 0);
        if (ret < 0)
            return ret;
    }
    a->buffer_size       = new_buf_size;
    a->allocated_samples = nb_samples;

    calc_ptr_alignment(a);
    a->samples_align = plane_size / a->stride;

    return 0;
}

AudioData *ff_audio_data_alloc(int channels, int nb_samples,
                               enum AVSampleFormat sample_fmt, const char *name)
{
    AudioData *a;
    int ret;

    if (channels < 1 || channels > AVRESAMPLE_MAX_CHANNELS)
        return NULL;

    a = av_mallocz(sizeof(*a));
    if (!a)
        return NULL;

    a->sample_size = av_get_bytes_per_sample(sample_fmt);
    if (!a->sample_size) {
        av_free(a);
        return NULL;
    }
    a->is_planar = ff_sample_fmt_is_planar(sample_fmt, channels);
    a->planes    = a->is_planar ? channels : 1;
    a->stride    = a->sample_size * (a->is_planar ? 1 : channels);

    a->class              = &audio_data_class;
    a->sample_fmt         = sample_fmt;
    a->channels           = channels;
    a->allocated_channels = channels;
    a->read_only          = 0;
    a->allow_realloc      = 1;
    a->name               = name ? name : "{no name}";

    if (nb_samples > 0) {
        ret = ff_audio_data_realloc(a, nb_samples);
        if (ret < 0) {
            av_free(a);
            return NULL;
        }
        return a;
    } else {
        calc_ptr_alignment(a);
        return a;
    }
}



void ff_audio_data_free(AudioData **a)
{
    if (!*a)
        return;
    av_free((*a)->buffer);
    av_freep(a);
}

int ff_audio_data_copy(AudioData *dst, AudioData *src, ChannelMapInfo *map)
{
    int ret, p;

    /* validate input/output compatibility */
    if (dst->sample_fmt != src->sample_fmt || dst->channels < src->channels)
        return AVERROR(EINVAL);

    if (map && !src->is_planar) {
        av_log(src, AV_LOG_ERROR, "cannot remap packed format during copy\n");
        return AVERROR(EINVAL);
    }

    /* if the input is empty, just empty the output */
    if (!src->nb_samples) {
        dst->nb_samples = 0;
        return 0;
    }

    /* reallocate output if necessary */
    ret = ff_audio_data_realloc(dst, src->nb_samples);
    if (ret < 0)
        return ret;

    /* copy data */
    if (map) {
        if (map->do_remap) {
            for (p = 0; p < src->planes; p++) {
                if (map->channel_map[p] >= 0)
                    memcpy(dst->data[p], src->data[map->channel_map[p]],
                           src->nb_samples * src->stride);
            }
        }
        if (map->do_copy || map->do_zero) {
            for (p = 0; p < src->planes; p++) {
                if (map->channel_copy[p])
                    memcpy(dst->data[p], dst->data[map->channel_copy[p]],
                           src->nb_samples * src->stride);
                else if (map->channel_zero[p])
                    av_samples_set_silence(&dst->data[p], 0, src->nb_samples,
                                           1, dst->sample_fmt);
            }
        }
    } else {
        for (p = 0; p < src->planes; p++)
            memcpy(dst->data[p], src->data[p], src->nb_samples * src->stride);
    }

    dst->nb_samples = src->nb_samples;

    return 0;
}

int ff_audio_data_combine(AudioData *dst, int dst_offset, AudioData *src,
                          int src_offset, int nb_samples)
{
    int ret, p, dst_offset2, dst_move_size;

    /* validate input/output compatibility */
    if (dst->sample_fmt != src->sample_fmt || dst->channels != src->channels) {
        av_log(src, AV_LOG_ERROR, "sample format mismatch\n");
        return AVERROR(EINVAL);
    }

    /* validate offsets are within the buffer bounds */
    if (dst_offset < 0 || dst_offset > dst->nb_samples ||
        src_offset < 0 || src_offset > src->nb_samples) {
        av_log(src, AV_LOG_ERROR, "offset out-of-bounds: src=%d dst=%d\n",
               src_offset, dst_offset);
        return AVERROR(EINVAL);
    }

    /* check offsets and sizes to see if we can just do nothing and return */
    if (nb_samples > src->nb_samples - src_offset)
        nb_samples = src->nb_samples - src_offset;
    if (nb_samples <= 0)
        return 0;

    /* validate that the output is not read-only */
    if (dst->read_only) {
        av_log(dst, AV_LOG_ERROR, "dst is read-only\n");
        return AVERROR(EINVAL);
    }

    /* reallocate output if necessary */
    ret = ff_audio_data_realloc(dst, dst->nb_samples + nb_samples);
    if (ret < 0) {
        av_log(dst, AV_LOG_ERROR, "error reallocating dst\n");
        return ret;
    }

    dst_offset2   = dst_offset + nb_samples;
    dst_move_size = dst->nb_samples - dst_offset;

    for (p = 0; p < src->planes; p++) {
        if (dst_move_size > 0) {
            memmove(dst->data[p] + dst_offset2 * dst->stride,
                    dst->data[p] + dst_offset  * dst->stride,
                    dst_move_size * dst->stride);
        }
        memcpy(dst->data[p] + dst_offset * dst->stride,
               src->data[p] + src_offset * src->stride,
               nb_samples * src->stride);
    }
    dst->nb_samples += nb_samples;

    return 0;
}

void ff_audio_data_drain(AudioData *a, int nb_samples)
{
    if (a->nb_samples <= nb_samples) {
        /* drain the whole buffer */
        a->nb_samples = 0;
    } else {
        int p;
        int move_offset = a->stride * nb_samples;
        int move_size   = a->stride * (a->nb_samples - nb_samples);

        for (p = 0; p < a->planes; p++)
            memmove(a->data[p], a->data[p] + move_offset, move_size);

        a->nb_samples -= nb_samples;
    }
}


void av_audio_fifo_free(AVAudioFifo *af)
{
    if (af) {
        if (af->buf) {
            int i;
            for (i = 0; i < af->nb_buffers; i++) {
                av_fifo_freep(&af->buf[i]);
            }
            av_freep(&af->buf);
        }
        av_free(af);
    }
}

AVAudioFifo *av_audio_fifo_alloc(enum AVSampleFormat sample_fmt, int channels,
                                 int nb_samples)
{
    AVAudioFifo *af;
    int buf_size, i;

    /* get channel buffer size (also validates parameters) */
    if (av_samples_get_buffer_size(&buf_size, channels, nb_samples, sample_fmt, 1) < 0)
        return NULL;

    af = av_mallocz(sizeof(*af));
    if (!af)
        return NULL;

    af->channels    = channels;
    af->sample_fmt  = sample_fmt;
    af->sample_size = buf_size / nb_samples;
    af->nb_buffers  = av_sample_fmt_is_planar(sample_fmt) ? channels : 1;

    af->buf = av_mallocz_array(af->nb_buffers, sizeof(*af->buf));
    if (!af->buf)
        goto error;

    for (i = 0; i < af->nb_buffers; i++) {
        af->buf[i] = av_fifo_alloc(buf_size);
        if (!af->buf[i])
            goto error;
    }
    af->allocated_samples = nb_samples;

    return af;

error:
    av_audio_fifo_free(af);
    return NULL;
}

int av_audio_fifo_realloc(AVAudioFifo *af, int nb_samples)
{
    int i, ret, buf_size;

    if ((ret = av_samples_get_buffer_size(&buf_size, af->channels, nb_samples,
                                          af->sample_fmt, 1)) < 0)
        return ret;

    for (i = 0; i < af->nb_buffers; i++) {
        if ((ret = av_fifo_realloc2(af->buf[i], buf_size)) < 0)
            return ret;
    }
    af->allocated_samples = nb_samples;
    return 0;
}

int av_audio_fifo_space(AVAudioFifo *af)
{
    return af->allocated_samples - af->nb_samples;
}
void av_audio_fifo_reset(AVAudioFifo *af)
{
    int i;

    for (i = 0; i < af->nb_buffers; i++)
        av_fifo_reset(af->buf[i]);

    af->nb_samples = 0;
}

int av_audio_fifo_size(AVAudioFifo *af)
{
    return af->nb_samples;
}
int av_audio_fifo_write(AVAudioFifo *af, void **data, int nb_samples)
{
    int i, ret, size;

    /* automatically reallocate buffers if needed */
    if (av_audio_fifo_space(af) < nb_samples) {
        int current_size = av_audio_fifo_size(af);
        /* check for integer overflow in new size calculation */
        if (INT_MAX / 2 - current_size < nb_samples)
            return AVERROR(EINVAL);
        /* reallocate buffers */
        if ((ret = av_audio_fifo_realloc(af, 2 * (current_size + nb_samples))) < 0)
            return ret;
    }

    size = nb_samples * af->sample_size;
    for (i = 0; i < af->nb_buffers; i++) {
        ret = av_fifo_generic_write(af->buf[i], data[i], size, NULL);
        if (ret != size)
            return AVERROR_BUG;
    }
    af->nb_samples += nb_samples;

    return nb_samples;
}

int av_audio_fifo_peek(AVAudioFifo *af, void **data, int nb_samples)
{
    int i, ret, size;

    if (nb_samples < 0)
        return AVERROR(EINVAL);
    nb_samples = FFMIN(nb_samples, af->nb_samples);
    if (!nb_samples)
        return 0;

    size = nb_samples * af->sample_size;
    for (i = 0; i < af->nb_buffers; i++) {
        if ((ret = av_fifo_generic_peek(af->buf[i], data[i], size, NULL)) < 0)
            return AVERROR_BUG;
    }

    return nb_samples;
}

int av_audio_fifo_peek_at(AVAudioFifo *af, void **data, int nb_samples, int offset)
{
    int i, ret, size;

    if (offset < 0 || offset >= af->nb_samples)
        return AVERROR(EINVAL);
    if (nb_samples < 0)
        return AVERROR(EINVAL);
    nb_samples = FFMIN(nb_samples, af->nb_samples);
    if (!nb_samples)
        return 0;
    if (offset > af->nb_samples - nb_samples)
        return AVERROR(EINVAL);

    offset *= af->sample_size;
    size = nb_samples * af->sample_size;
    for (i = 0; i < af->nb_buffers; i++) {
        if ((ret = av_fifo_generic_peek_at(af->buf[i], data[i], offset, size, NULL)) < 0)
            return AVERROR_BUG;
    }

    return nb_samples;
}

int av_audio_fifo_read(AVAudioFifo *af, void **data, int nb_samples)
{
    int i, size;

    if (nb_samples < 0)
        return AVERROR(EINVAL);
    nb_samples = FFMIN(nb_samples, af->nb_samples);
    if (!nb_samples)
        return 0;

    size = nb_samples * af->sample_size;
    for (i = 0; i < af->nb_buffers; i++) {
        if (av_fifo_generic_read(af->buf[i], data[i], size, NULL) < 0)
            return AVERROR_BUG;
    }
    af->nb_samples -= nb_samples;

    return nb_samples;
}

int av_audio_fifo_drain(AVAudioFifo *af, int nb_samples)
{
    int i, size;

    if (nb_samples < 0)
        return AVERROR(EINVAL);
    nb_samples = FFMIN(nb_samples, af->nb_samples);

    if (nb_samples) {
        size = nb_samples * af->sample_size;
        for (i = 0; i < af->nb_buffers; i++)
            av_fifo_drain(af->buf[i], size);
        af->nb_samples -= nb_samples;
    }
    return 0;
}





int ff_audio_data_add_to_fifo(AVAudioFifo *af, AudioData *a, int offset,
                              int nb_samples)
{
    uint8_t *offset_data[AVRESAMPLE_MAX_CHANNELS];
    int offset_size, p;

    if (offset >= a->nb_samples)
        return 0;
    offset_size = offset * a->stride;
    for (p = 0; p < a->planes; p++)
        offset_data[p] = a->data[p] + offset_size;

    return av_audio_fifo_write(af, (void **)offset_data, nb_samples);
}

int ff_audio_data_read_from_fifo(AVAudioFifo *af, AudioData *a, int nb_samples)
{
    int ret;

    if (a->read_only)
        return AVERROR(EINVAL);

    ret = ff_audio_data_realloc(a, nb_samples);
    if (ret < 0)
        return ret;

    ret = av_audio_fifo_read(af, (void **)a->data, nb_samples);
    if (ret >= 0)
        a->nb_samples = ret;
    return ret;
}



void ff_audio_resample_init_aarch64(ResampleContext *c,
                                    enum AVSampleFormat sample_fmt);
void ff_audio_resample_init_arm(ResampleContext *c,
                                enum AVSampleFormat sample_fmt);
int avresample_is_open(AVAudioResampleContext *avr)
{
    return !!avr->out_fifo;
}
#define MUTE_THRESHOLD_SEC 0.000333

/* scale factor for 16-bit output.
   The signal is attenuated slightly to avoid clipping */
#define S16_SCALE 32753.0f

/* scale to convert lfg from INT_MIN/INT_MAX to -0.5/0.5 */
#define LFG_SCALE (1.0f / (2.0f * INT32_MAX))
static void quantize_c(int16_t *dst, const float *src, float *dither, int len)
{
    int i;
    for (i = 0; i < len; i++)
        dst[i] = av_clip_int16(lrintf(src[i] * S16_SCALE + dither[i]));
}

static void dither_int_to_float_rectangular_c(float *dst, int *src, int len)
{
    int i;
    for (i = 0; i < len; i++)
        dst[i] = src[i] * LFG_SCALE;
}

static void dither_int_to_float_triangular_c(float *dst, int *src0, int len)
{
    int i;
    int *src1  = src0 + len;

    for (i = 0; i < len; i++) {
        float r = src0[i] * LFG_SCALE;
        r      += src1[i] * LFG_SCALE;
        dst[i]  = r;
    }
}

void ff_quantize_sse2(int16_t *dst, const float *src, float *dither, int len);
void ff_dither_int_to_float_rectangular_sse2(float *dst, int *src, int len);
void ff_dither_int_to_float_rectangular_avx(float *dst, int *src, int len);
void ff_dither_int_to_float_triangular_sse2(float *dst, int *src0, int len);
void ff_dither_int_to_float_triangular_avx(float *dst, int *src0, int len);

av_cold void ff_dither_init_x86(DitherDSPContext *ddsp,enum AVResampleDitherMethod method)
{
    int cpu_flags = av_get_cpu_flags();

    // if (EXTERNAL_SSE2(cpu_flags)) {
    //     ddsp->quantize      = ff_quantize_sse2;
    //     ddsp->ptr_align     = 16;
    //     ddsp->samples_align = 8;
    // }

    // if (method == AV_RESAMPLE_DITHER_RECTANGULAR) {
    //     if (EXTERNAL_SSE2(cpu_flags)) {
    //         ddsp->dither_int_to_float = ff_dither_int_to_float_rectangular_sse2;
    //     }
    //     if (EXTERNAL_AVX_FAST(cpu_flags)) {
    //         ddsp->dither_int_to_float = ff_dither_int_to_float_rectangular_avx;
    //     }
    // } else {
    //     if (EXTERNAL_SSE2(cpu_flags)) {
    //         ddsp->dither_int_to_float = ff_dither_int_to_float_triangular_sse2;
    //     }
    //     if (EXTERNAL_AVX_FAST(cpu_flags)) {
    //         ddsp->dither_int_to_float = ff_dither_int_to_float_triangular_avx;
    //     }
    // }
}

static void  dither_init(DitherDSPContext *ddsp,
                                enum AVResampleDitherMethod method)
{
    ddsp->quantize      = quantize_c;
    ddsp->ptr_align     = 1;
    ddsp->samples_align = 1;

    if (method == AV_RESAMPLE_DITHER_RECTANGULAR)
        ddsp->dither_int_to_float = dither_int_to_float_rectangular_c;
    else
        ddsp->dither_int_to_float = dither_int_to_float_triangular_c;

    if (ARCH_X86)
        ff_dither_init_x86(ddsp, method);
}

static const float ns_48_coef_b[4] = {
    2.2374f, -0.7339f, -0.1251f, -0.6033f
};

static const float ns_48_coef_a[4] = {
    0.9030f, 0.0116f, -0.5853f, -0.2571f
};

static const float ns_44_coef_b[4] = {
    2.2061f, -0.4707f, -0.2534f, -0.6213f
};

static const float ns_44_coef_a[4] = {
    1.0587f, 0.0676f, -0.6054f, -0.2738f
};



#include "md5.c"

void av_lfg_init(AVLFG *c, unsigned int seed)
{
    uint8_t tmp[16] = { 0 };
    int i;

    for (i = 8; i < 64; i += 4) {
        AV_WL32(tmp, seed);
        tmp[4] = i;
        av_md5_sum(tmp, tmp, 16);
        c->state[i    ] = AV_RL32(tmp);
        c->state[i + 1] = AV_RL32(tmp + 4);
        c->state[i + 2] = AV_RL32(tmp + 8);
        c->state[i + 3] = AV_RL32(tmp + 12);
    }
    c->index = 0;
}
static inline unsigned int av_lfg_get(AVLFG *c){
    unsigned a = c->state[c->index & 63] = c->state[(c->index-24) & 63] + c->state[(c->index-55) & 63];
    c->index += 1U;
    return a;
}

#define SQRT_1_6 0.40824829046386301723f

static void dither_highpass_filter(float *src, int len)
{
    int i;

    /* filter is from libswresample in FFmpeg */
    for (i = 0; i < len - 2; i++)
        src[i] = (-src[i] + 2 * src[i + 1] - src[i + 2]) * SQRT_1_6;
}
static int generate_dither_noise(DitherContext *c, DitherState *state,
                                 int min_samples)
{
    int i;
    int nb_samples  = FFALIGN(min_samples, 16) + 16;
    int buf_samples = nb_samples *
                      (c->method == AV_RESAMPLE_DITHER_RECTANGULAR ? 1 : 2);
    unsigned int *noise_buf_ui;

    av_freep(&state->noise_buf);
    state->noise_buf_size = state->noise_buf_ptr = 0;

    state->noise_buf = av_malloc(buf_samples * sizeof(*state->noise_buf));
    if (!state->noise_buf)
        return AVERROR(ENOMEM);
    state->noise_buf_size = FFALIGN(min_samples, 16);
    noise_buf_ui          = (unsigned int *)state->noise_buf;

    av_lfg_init(&state->lfg, state->seed);
    for (i = 0; i < buf_samples; i++)
        noise_buf_ui[i] = av_lfg_get(&state->lfg);

    c->ddsp.dither_int_to_float(state->noise_buf, noise_buf_ui, nb_samples);

    if (c->method == AV_RESAMPLE_DITHER_TRIANGULAR_HP)
        dither_highpass_filter(state->noise_buf, nb_samples);

    return 0;
}

AudioConvert *ff_audio_convert_alloc(AVAudioResampleContext *avr,
                                     enum AVSampleFormat out_fmt,
                                     enum AVSampleFormat in_fmt,
                                     int channels, int sample_rate,
                                     int apply_map);
void ff_dither_free(DitherContext **cp);
DitherContext *ff_dither_alloc(AVAudioResampleContext *avr,
                               enum AVSampleFormat out_fmt,
                               enum AVSampleFormat in_fmt,
                               int channels, int sample_rate, int apply_map)
{
    AVLFG seed_gen;
    DitherContext *c;
    int ch;

    if (av_get_packed_sample_fmt(out_fmt) != AV_SAMPLE_FMT_S16 ||
        av_get_bytes_per_sample(in_fmt) <= 2) {
        av_log(avr, AV_LOG_ERROR, "dithering %s to %s is not supported\n",
               av_get_sample_fmt_name(in_fmt), av_get_sample_fmt_name(out_fmt));
        return NULL;
    }

    c = av_mallocz(sizeof(*c));
    if (!c)
        return NULL;

    c->apply_map = apply_map;
    if (apply_map)
        c->ch_map_info = &avr->ch_map_info;

    if (avr->dither_method == AV_RESAMPLE_DITHER_TRIANGULAR_NS &&
        sample_rate != 48000 && sample_rate != 44100) {
        av_log(avr, AV_LOG_WARNING, "sample rate must be 48000 or 44100 Hz "
               "for triangular_ns dither. using triangular_hp instead.\n");
        avr->dither_method = AV_RESAMPLE_DITHER_TRIANGULAR_HP;
    }
    c->method = avr->dither_method;
    dither_init(&c->ddsp, c->method);

    if (c->method == AV_RESAMPLE_DITHER_TRIANGULAR_NS) {
        if (sample_rate == 48000) {
            c->ns_coef_b = ns_48_coef_b;
            c->ns_coef_a = ns_48_coef_a;
        } else {
            c->ns_coef_b = ns_44_coef_b;
            c->ns_coef_a = ns_44_coef_a;
        }
    }

    /* Either s16 or s16p output format is allowed, but s16p is used
       internally, so we need to use a temp buffer and interleave if the output
       format is s16 */
    if (out_fmt != AV_SAMPLE_FMT_S16P) {
        c->s16_data = ff_audio_data_alloc(channels, 1024, AV_SAMPLE_FMT_S16P,
                                          "dither s16 buffer");
        if (!c->s16_data)
            goto fail;

        c->ac_out = ff_audio_convert_alloc(avr, out_fmt, AV_SAMPLE_FMT_S16P,
                                           channels, sample_rate, 0);
        if (!c->ac_out)
            goto fail;
    }

    if (in_fmt != AV_SAMPLE_FMT_FLTP || c->apply_map) {
        c->flt_data = ff_audio_data_alloc(channels, 1024, AV_SAMPLE_FMT_FLTP,
                                          "dither flt buffer");
        if (!c->flt_data)
            goto fail;
    }
    if (in_fmt != AV_SAMPLE_FMT_FLTP) {
        c->ac_in = ff_audio_convert_alloc(avr, AV_SAMPLE_FMT_FLTP, in_fmt,
                                          channels, sample_rate, c->apply_map);
        if (!c->ac_in)
            goto fail;
    }

    c->state = av_mallocz(channels * sizeof(*c->state));
    if (!c->state)
        goto fail;
    c->channels = channels;

    /* calculate thresholds for turning off dithering during periods of
       silence to avoid replacing digital silence with quiet dither noise */
    c->mute_dither_threshold = lrintf(sample_rate * MUTE_THRESHOLD_SEC);
    c->mute_reset_threshold  = c->mute_dither_threshold * 4;

    /* initialize dither states */
    av_lfg_init(&seed_gen, 0xC0FFEE);
    for (ch = 0; ch < channels; ch++) {
        DitherState *state = &c->state[ch];
        state->mute = c->mute_reset_threshold + 1;
        state->seed = av_lfg_get(&seed_gen);
        generate_dither_noise(c, state, FFMAX(32768, sample_rate / 2));
    }

    return c;

fail:
    ff_dither_free(&c);
    return NULL;
}




void ff_audio_convert_set_func(AudioConvert *ac, enum AVSampleFormat out_fmt,
                               enum AVSampleFormat in_fmt, int channels,
                               int ptr_align, int samples_align,
                               const char *descr, void *conv)
{
    int found = 0;

    switch (ac->func_type) {
    case CONV_FUNC_TYPE_FLAT:
        if (av_get_packed_sample_fmt(ac->in_fmt)  == in_fmt &&
            av_get_packed_sample_fmt(ac->out_fmt) == out_fmt) {
            ac->conv_flat     = conv;
            ac->func_descr    = descr;
            ac->ptr_align     = ptr_align;
            ac->samples_align = samples_align;
            if (ptr_align == 1 && samples_align == 1) {
                ac->conv_flat_generic  = conv;
                ac->func_descr_generic = descr;
            } else {
                ac->has_optimized_func = 1;
            }
            found = 1;
        }
        break;
    case CONV_FUNC_TYPE_INTERLEAVE:
        if (ac->in_fmt == in_fmt && ac->out_fmt == out_fmt &&
            (!channels || ac->channels == channels)) {
            ac->conv_interleave = conv;
            ac->func_descr      = descr;
            ac->ptr_align       = ptr_align;
            ac->samples_align   = samples_align;
            if (ptr_align == 1 && samples_align == 1) {
                ac->conv_interleave_generic = conv;
                ac->func_descr_generic      = descr;
            } else {
                ac->has_optimized_func = 1;
            }
            found = 1;
        }
        break;
    case CONV_FUNC_TYPE_DEINTERLEAVE:
        if (ac->in_fmt == in_fmt && ac->out_fmt == out_fmt &&
            (!channels || ac->channels == channels)) {
            ac->conv_deinterleave = conv;
            ac->func_descr        = descr;
            ac->ptr_align         = ptr_align;
            ac->samples_align     = samples_align;
            if (ptr_align == 1 && samples_align == 1) {
                ac->conv_deinterleave_generic = conv;
                ac->func_descr_generic        = descr;
            } else {
                ac->has_optimized_func = 1;
            }
            found = 1;
        }
        break;
    }
    if (found) {
        av_log(ac->avr, AV_LOG_DEBUG, "audio_convert: found function: %-4s "
               "to %-4s (%s)\n", av_get_sample_fmt_name(ac->in_fmt),
               av_get_sample_fmt_name(ac->out_fmt), descr);
    }
}

#define CONV_FUNC_NAME(dst_fmt, src_fmt) conv_ ## src_fmt ## _to_ ## dst_fmt

#define CONV_LOOP(otype, expr)                                              \
    do {                                                                    \
        *(otype *)po = expr;                                                \
        pi += is;                                                           \
        po += os;                                                           \
    } while (po < end);                                                     \

#define CONV_FUNC_FLAT(ofmt, otype, ifmt, itype, expr)                      \
static void CONV_FUNC_NAME(ofmt, ifmt)(uint8_t *out, const uint8_t *in,     \
                                       int len)                             \
{                                                                           \
    int is       = sizeof(itype);                                           \
    int os       = sizeof(otype);                                           \
    const uint8_t *pi = in;                                                 \
    uint8_t       *po = out;                                                \
    uint8_t *end = out + os * len;                                          \
    CONV_LOOP(otype, expr)                                                  \
}

#define CONV_FUNC_INTERLEAVE(ofmt, otype, ifmt, itype, expr)                \
static void CONV_FUNC_NAME(ofmt, ifmt)(uint8_t *out, const uint8_t **in,    \
                                       int len, int channels)               \
{                                                                           \
    int ch;                                                                 \
    int out_bps = sizeof(otype);                                            \
    int is      = sizeof(itype);                                            \
    int os      = channels * out_bps;                                       \
    for (ch = 0; ch < channels; ch++) {                                     \
        const uint8_t *pi = in[ch];                                         \
        uint8_t       *po = out + ch * out_bps;                             \
        uint8_t      *end = po + os * len;                                  \
        CONV_LOOP(otype, expr)                                              \
    }                                                                       \
}

#define CONV_FUNC_DEINTERLEAVE(ofmt, otype, ifmt, itype, expr)              \
static void CONV_FUNC_NAME(ofmt, ifmt)(uint8_t **out, const uint8_t *in,    \
                                       int len, int channels)               \
{                                                                           \
    int ch;                                                                 \
    int in_bps = sizeof(itype);                                             \
    int is     = channels * in_bps;                                         \
    int os     = sizeof(otype);                                             \
    for (ch = 0; ch < channels; ch++) {                                     \
        const uint8_t *pi = in  + ch * in_bps;                              \
        uint8_t       *po = out[ch];                                        \
        uint8_t      *end = po + os * len;                                  \
        CONV_LOOP(otype, expr)                                              \
    }                                                                       \
}

#define CONV_FUNC_GROUP(ofmt, otype, ifmt, itype, expr) \
CONV_FUNC_FLAT(        ofmt,      otype, ifmt,      itype, expr) \
CONV_FUNC_INTERLEAVE(  ofmt,      otype, ifmt ## P, itype, expr) \
CONV_FUNC_DEINTERLEAVE(ofmt ## P, otype, ifmt,      itype, expr)

CONV_FUNC_GROUP(AV_SAMPLE_FMT_U8,  uint8_t, AV_SAMPLE_FMT_U8,  uint8_t,  *(const uint8_t *)pi)
CONV_FUNC_GROUP(AV_SAMPLE_FMT_S16, int16_t, AV_SAMPLE_FMT_U8,  uint8_t, (*(const uint8_t *)pi - 0x80) <<  8)
CONV_FUNC_GROUP(AV_SAMPLE_FMT_S32, int32_t, AV_SAMPLE_FMT_U8,  uint8_t, (*(const uint8_t *)pi - 0x80) << 24)
CONV_FUNC_GROUP(AV_SAMPLE_FMT_FLT, float,   AV_SAMPLE_FMT_U8,  uint8_t, (*(const uint8_t *)pi - 0x80) * (1.0f / (1 << 7)))
CONV_FUNC_GROUP(AV_SAMPLE_FMT_DBL, double,  AV_SAMPLE_FMT_U8,  uint8_t, (*(const uint8_t *)pi - 0x80) * (1.0  / (1 << 7)))
CONV_FUNC_GROUP(AV_SAMPLE_FMT_U8,  uint8_t, AV_SAMPLE_FMT_S16, int16_t, (*(const int16_t *)pi >> 8) + 0x80)
CONV_FUNC_GROUP(AV_SAMPLE_FMT_S16, int16_t, AV_SAMPLE_FMT_S16, int16_t,  *(const int16_t *)pi)
CONV_FUNC_GROUP(AV_SAMPLE_FMT_S32, int32_t, AV_SAMPLE_FMT_S16, int16_t,  *(const int16_t *)pi << 16)
CONV_FUNC_GROUP(AV_SAMPLE_FMT_FLT, float,   AV_SAMPLE_FMT_S16, int16_t,  *(const int16_t *)pi * (1.0f / (1 << 15)))
CONV_FUNC_GROUP(AV_SAMPLE_FMT_DBL, double,  AV_SAMPLE_FMT_S16, int16_t,  *(const int16_t *)pi * (1.0  / (1 << 15)))
CONV_FUNC_GROUP(AV_SAMPLE_FMT_U8,  uint8_t, AV_SAMPLE_FMT_S32, int32_t, (*(const int32_t *)pi >> 24) + 0x80)
CONV_FUNC_GROUP(AV_SAMPLE_FMT_S16, int16_t, AV_SAMPLE_FMT_S32, int32_t,  *(const int32_t *)pi >> 16)
CONV_FUNC_GROUP(AV_SAMPLE_FMT_S32, int32_t, AV_SAMPLE_FMT_S32, int32_t,  *(const int32_t *)pi)
CONV_FUNC_GROUP(AV_SAMPLE_FMT_FLT, float,   AV_SAMPLE_FMT_S32, int32_t,  *(const int32_t *)pi * (1.0f / (1U << 31)))
CONV_FUNC_GROUP(AV_SAMPLE_FMT_DBL, double,  AV_SAMPLE_FMT_S32, int32_t,  *(const int32_t *)pi * (1.0  / (1U << 31)))
CONV_FUNC_GROUP(AV_SAMPLE_FMT_U8,  uint8_t, AV_SAMPLE_FMT_FLT, float,   av_clip_uint8(  lrintf(*(const float *)pi * (1  <<  7)) + 0x80))
CONV_FUNC_GROUP(AV_SAMPLE_FMT_S16, int16_t, AV_SAMPLE_FMT_FLT, float,   av_clip_int16(  lrintf(*(const float *)pi * (1  << 15))))
CONV_FUNC_GROUP(AV_SAMPLE_FMT_S32, int32_t, AV_SAMPLE_FMT_FLT, float,   av_clipl_int32(llrintf(*(const float *)pi * (1U << 31))))
CONV_FUNC_GROUP(AV_SAMPLE_FMT_FLT, float,   AV_SAMPLE_FMT_FLT, float,   *(const float *)pi)
CONV_FUNC_GROUP(AV_SAMPLE_FMT_DBL, double,  AV_SAMPLE_FMT_FLT, float,   *(const float *)pi)
CONV_FUNC_GROUP(AV_SAMPLE_FMT_U8,  uint8_t, AV_SAMPLE_FMT_DBL, double,  av_clip_uint8(  lrint(*(const double *)pi * (1  <<  7)) + 0x80))
CONV_FUNC_GROUP(AV_SAMPLE_FMT_S16, int16_t, AV_SAMPLE_FMT_DBL, double,  av_clip_int16(  lrint(*(const double *)pi * (1  << 15))))
CONV_FUNC_GROUP(AV_SAMPLE_FMT_S32, int32_t, AV_SAMPLE_FMT_DBL, double,  av_clipl_int32(llrint(*(const double *)pi * (1U << 31))))
CONV_FUNC_GROUP(AV_SAMPLE_FMT_FLT, float,   AV_SAMPLE_FMT_DBL, double,  *(const double *)pi)
CONV_FUNC_GROUP(AV_SAMPLE_FMT_DBL, double,  AV_SAMPLE_FMT_DBL, double,  *(const double *)pi)

#define SET_CONV_FUNC_GROUP(ofmt, ifmt)                                                             \
ff_audio_convert_set_func(ac, ofmt,      ifmt,      0, 1, 1, "C", CONV_FUNC_NAME(ofmt,      ifmt)); \
ff_audio_convert_set_func(ac, ofmt ## P, ifmt,      0, 1, 1, "C", CONV_FUNC_NAME(ofmt ## P, ifmt)); \
ff_audio_convert_set_func(ac, ofmt,      ifmt ## P, 0, 1, 1, "C", CONV_FUNC_NAME(ofmt,      ifmt ## P));

static void set_generic_function(AudioConvert *ac)
{
    SET_CONV_FUNC_GROUP(AV_SAMPLE_FMT_U8,  AV_SAMPLE_FMT_U8)
    SET_CONV_FUNC_GROUP(AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_U8)
    SET_CONV_FUNC_GROUP(AV_SAMPLE_FMT_S32, AV_SAMPLE_FMT_U8)
    SET_CONV_FUNC_GROUP(AV_SAMPLE_FMT_FLT, AV_SAMPLE_FMT_U8)
    SET_CONV_FUNC_GROUP(AV_SAMPLE_FMT_DBL, AV_SAMPLE_FMT_U8)
    SET_CONV_FUNC_GROUP(AV_SAMPLE_FMT_U8,  AV_SAMPLE_FMT_S16)
    SET_CONV_FUNC_GROUP(AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_S16)
    SET_CONV_FUNC_GROUP(AV_SAMPLE_FMT_S32, AV_SAMPLE_FMT_S16)
    SET_CONV_FUNC_GROUP(AV_SAMPLE_FMT_FLT, AV_SAMPLE_FMT_S16)
    SET_CONV_FUNC_GROUP(AV_SAMPLE_FMT_DBL, AV_SAMPLE_FMT_S16)
    SET_CONV_FUNC_GROUP(AV_SAMPLE_FMT_U8,  AV_SAMPLE_FMT_S32)
    SET_CONV_FUNC_GROUP(AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_S32)
    SET_CONV_FUNC_GROUP(AV_SAMPLE_FMT_S32, AV_SAMPLE_FMT_S32)
    SET_CONV_FUNC_GROUP(AV_SAMPLE_FMT_FLT, AV_SAMPLE_FMT_S32)
    SET_CONV_FUNC_GROUP(AV_SAMPLE_FMT_DBL, AV_SAMPLE_FMT_S32)
    SET_CONV_FUNC_GROUP(AV_SAMPLE_FMT_U8,  AV_SAMPLE_FMT_FLT)
    SET_CONV_FUNC_GROUP(AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_FLT)
    SET_CONV_FUNC_GROUP(AV_SAMPLE_FMT_S32, AV_SAMPLE_FMT_FLT)
    SET_CONV_FUNC_GROUP(AV_SAMPLE_FMT_FLT, AV_SAMPLE_FMT_FLT)
    SET_CONV_FUNC_GROUP(AV_SAMPLE_FMT_DBL, AV_SAMPLE_FMT_FLT)
    SET_CONV_FUNC_GROUP(AV_SAMPLE_FMT_U8,  AV_SAMPLE_FMT_DBL)
    SET_CONV_FUNC_GROUP(AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_DBL)
    SET_CONV_FUNC_GROUP(AV_SAMPLE_FMT_S32, AV_SAMPLE_FMT_DBL)
    SET_CONV_FUNC_GROUP(AV_SAMPLE_FMT_FLT, AV_SAMPLE_FMT_DBL)
    SET_CONV_FUNC_GROUP(AV_SAMPLE_FMT_DBL, AV_SAMPLE_FMT_DBL)
}

void ff_conv_s16_to_s32_sse2(int16_t *dst, const int32_t *src, int len);

void ff_conv_s16_to_flt_sse2(float *dst, const int16_t *src, int len);
void ff_conv_s16_to_flt_sse4(float *dst, const int16_t *src, int len);

void ff_conv_s32_to_s16_mmx (int16_t *dst, const int32_t *src, int len);
void ff_conv_s32_to_s16_sse2(int16_t *dst, const int32_t *src, int len);

void ff_conv_s32_to_flt_sse2(float *dst, const int32_t *src, int len);
void ff_conv_s32_to_flt_avx (float *dst, const int32_t *src, int len);

void ff_conv_flt_to_s16_sse2(int16_t *dst, const float *src, int len);

void ff_conv_flt_to_s32_sse2(int32_t *dst, const float *src, int len);
void ff_conv_flt_to_s32_avx (int32_t *dst, const float *src, int len);

/* interleave conversions */

void ff_conv_s16p_to_s16_2ch_sse2(int16_t *dst, int16_t *const *src,
                                  int len, int channels);
void ff_conv_s16p_to_s16_2ch_avx (int16_t *dst, int16_t *const *src,
                                  int len, int channels);

void ff_conv_s16p_to_s16_6ch_sse2(int16_t *dst, int16_t *const *src,
                                  int len, int channels);
void ff_conv_s16p_to_s16_6ch_sse2slow(int16_t *dst, int16_t *const *src,
                                      int len, int channels);
void ff_conv_s16p_to_s16_6ch_avx (int16_t *dst, int16_t *const *src,
                                  int len, int channels);

void ff_conv_s16p_to_flt_2ch_sse2(float *dst, int16_t *const *src,
                                  int len, int channels);
void ff_conv_s16p_to_flt_2ch_avx (float *dst, int16_t *const *src,
                                  int len, int channels);

void ff_conv_s16p_to_flt_6ch_sse2 (float *dst, int16_t *const *src,
                                   int len, int channels);
void ff_conv_s16p_to_flt_6ch_ssse3(float *dst, int16_t *const *src,
                                  int len, int channels);
void ff_conv_s16p_to_flt_6ch_avx  (float *dst, int16_t *const *src,
                                   int len, int channels);

void ff_conv_fltp_to_s16_2ch_sse2 (int16_t *dst, float *const *src,
                                   int len, int channels);
void ff_conv_fltp_to_s16_2ch_ssse3(int16_t *dst, float *const *src,
                                   int len, int channels);

void ff_conv_fltp_to_s16_6ch_sse (int16_t *dst, float *const *src,
                                  int len, int channels);
void ff_conv_fltp_to_s16_6ch_sse2(int16_t *dst, float *const *src,
                                  int len, int channels);
void ff_conv_fltp_to_s16_6ch_avx (int16_t *dst, float *const *src,
                                  int len, int channels);

void ff_conv_fltp_to_flt_2ch_sse(float *dst, float *const *src, int len,
                                 int channels);
void ff_conv_fltp_to_flt_2ch_avx(float *dst, float *const *src, int len,
                                 int channels);

void ff_conv_fltp_to_flt_6ch_mmx (float *dst, float *const *src, int len,
                                  int channels);
void ff_conv_fltp_to_flt_6ch_sse4(float *dst, float *const *src, int len,
                                  int channels);
void ff_conv_fltp_to_flt_6ch_avx (float *dst, float *const *src, int len,
                                  int channels);

/* deinterleave conversions */

void ff_conv_s16_to_s16p_2ch_sse2(int16_t *const *dst, int16_t *src,
                                  int len, int channels);
void ff_conv_s16_to_s16p_2ch_ssse3(int16_t *const *dst, int16_t *src,
                                   int len, int channels);
void ff_conv_s16_to_s16p_2ch_avx (int16_t *const *dst, int16_t *src,
                                  int len, int channels);

void ff_conv_s16_to_s16p_6ch_sse2 (int16_t *const *dst, int16_t *src,
                                   int len, int channels);
void ff_conv_s16_to_s16p_6ch_ssse3(int16_t *const *dst, int16_t *src,
                                   int len, int channels);
void ff_conv_s16_to_s16p_6ch_avx  (int16_t *const *dst, int16_t *src,
                                   int len, int channels);

void ff_conv_s16_to_fltp_2ch_sse2(float *const *dst, int16_t *src,
                                  int len, int channels);
void ff_conv_s16_to_fltp_2ch_avx (float *const *dst, int16_t *src,
                                  int len, int channels);

void ff_conv_s16_to_fltp_6ch_sse2 (float *const *dst, int16_t *src,
                                   int len, int channels);
void ff_conv_s16_to_fltp_6ch_ssse3(float *const *dst, int16_t *src,
                                   int len, int channels);
void ff_conv_s16_to_fltp_6ch_sse4 (float *const *dst, int16_t *src,
                                   int len, int channels);
void ff_conv_s16_to_fltp_6ch_avx  (float *const *dst, int16_t *src,
                                   int len, int channels);

void ff_conv_flt_to_s16p_2ch_sse2(int16_t *const *dst, float *src,
                                  int len, int channels);
void ff_conv_flt_to_s16p_2ch_avx (int16_t *const *dst, float *src,
                                  int len, int channels);

void ff_conv_flt_to_s16p_6ch_sse2 (int16_t *const *dst, float *src,int len, int channels);
void ff_conv_flt_to_s16p_6ch_ssse3(int16_t *const *dst, float *src,int len, int channels);
void ff_conv_flt_to_s16p_6ch_avx  (int16_t *const *dst, float *src,int len, int channels);

void ff_conv_flt_to_fltp_2ch_sse(float *const *dst, float *src, int len,int channels);
void ff_conv_flt_to_fltp_2ch_avx(float *const *dst, float *src, int len,int channels);

void ff_conv_flt_to_fltp_6ch_sse2(float *const *dst, float *src, int len,int channels);
void ff_conv_flt_to_fltp_6ch_avx (float *const *dst, float *src, int len,int channels);

av_cold void ff_audio_convert_init_x86(AudioConvert *ac)
{
    int cpu_flags = av_get_cpu_flags();

    // if (EXTERNAL_MMX(cpu_flags)) {
    //     ff_audio_convert_set_func(ac, AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_S32, 0, 1, 8, "MMX", ff_conv_s32_to_s16_mmx);
    //     ff_audio_convert_set_func(ac, AV_SAMPLE_FMT_FLT, AV_SAMPLE_FMT_FLTP,6, 1, 4, "MMX", ff_conv_fltp_to_flt_6ch_mmx);
    // }
    // if (EXTERNAL_SSE(cpu_flags)) {
    //     ff_audio_convert_set_func(ac, AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_FLTP,6, 1, 2, "SSE", ff_conv_fltp_to_s16_6ch_sse);
    //     ff_audio_convert_set_func(ac, AV_SAMPLE_FMT_FLT, AV_SAMPLE_FMT_FLTP,2, 16, 8, "SSE", ff_conv_fltp_to_flt_2ch_sse);
    //     ff_audio_convert_set_func(ac, AV_SAMPLE_FMT_FLTP, AV_SAMPLE_FMT_FLT,2, 16, 4, "SSE", ff_conv_flt_to_fltp_2ch_sse);
    // }
    // if (EXTERNAL_SSE2(cpu_flags)) {
    //     if (!(cpu_flags & AV_CPU_FLAG_SSE2SLOW)) {
    //         ff_audio_convert_set_func(ac, AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_S32,0, 16, 16, "SSE2", ff_conv_s32_to_s16_sse2);
    //         ff_audio_convert_set_func(ac, AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_S16P,6, 16, 8, "SSE2", ff_conv_s16p_to_s16_6ch_sse2);
    //         ff_audio_convert_set_func(ac, AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_FLTP,6, 16, 4, "SSE2", ff_conv_fltp_to_s16_6ch_sse2);
    //     } else {
    //         ff_audio_convert_set_func(ac, AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_S16P,6, 1, 4, "SSE2SLOW", ff_conv_s16p_to_s16_6ch_sse2slow);
    //     }
    //     ff_audio_convert_set_func(ac, AV_SAMPLE_FMT_S32, AV_SAMPLE_FMT_S16,0, 16, 8, "SSE2", ff_conv_s16_to_s32_sse2);
    //     ff_audio_convert_set_func(ac, AV_SAMPLE_FMT_FLT, AV_SAMPLE_FMT_S16,0, 16, 8, "SSE2", ff_conv_s16_to_flt_sse2);
    //     ff_audio_convert_set_func(ac, AV_SAMPLE_FMT_FLT, AV_SAMPLE_FMT_S32,0, 16, 8, "SSE2", ff_conv_s32_to_flt_sse2);
    //     ff_audio_convert_set_func(ac, AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_FLT,0, 16, 16, "SSE2", ff_conv_flt_to_s16_sse2);
    //     ff_audio_convert_set_func(ac, AV_SAMPLE_FMT_S32, AV_SAMPLE_FMT_FLT,0, 16, 16, "SSE2", ff_conv_flt_to_s32_sse2);
    //     ff_audio_convert_set_func(ac, AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_S16P,2, 16, 16, "SSE2", ff_conv_s16p_to_s16_2ch_sse2);
    //     ff_audio_convert_set_func(ac, AV_SAMPLE_FMT_FLT, AV_SAMPLE_FMT_S16P,2, 16, 8, "SSE2", ff_conv_s16p_to_flt_2ch_sse2);
    //     ff_audio_convert_set_func(ac, AV_SAMPLE_FMT_FLT, AV_SAMPLE_FMT_S16P,6, 16, 4, "SSE2", ff_conv_s16p_to_flt_6ch_sse2);
    //     ff_audio_convert_set_func(ac, AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_FLTP,2, 16, 4, "SSE2", ff_conv_fltp_to_s16_2ch_sse2);
    //     ff_audio_convert_set_func(ac, AV_SAMPLE_FMT_S16P, AV_SAMPLE_FMT_S16,2, 16, 8, "SSE2", ff_conv_s16_to_s16p_2ch_sse2);
    //     ff_audio_convert_set_func(ac, AV_SAMPLE_FMT_S16P, AV_SAMPLE_FMT_S16,6, 16, 4, "SSE2", ff_conv_s16_to_s16p_6ch_sse2);
    //     ff_audio_convert_set_func(ac, AV_SAMPLE_FMT_FLTP, AV_SAMPLE_FMT_S16,2, 16, 8, "SSE2", ff_conv_s16_to_fltp_2ch_sse2);
    //     ff_audio_convert_set_func(ac, AV_SAMPLE_FMT_FLTP, AV_SAMPLE_FMT_S16,6, 16, 4, "SSE2", ff_conv_s16_to_fltp_6ch_sse2);
    //     ff_audio_convert_set_func(ac, AV_SAMPLE_FMT_S16P, AV_SAMPLE_FMT_FLT,2, 16, 8, "SSE2", ff_conv_flt_to_s16p_2ch_sse2);
    //     ff_audio_convert_set_func(ac, AV_SAMPLE_FMT_S16P, AV_SAMPLE_FMT_FLT,6, 16, 4, "SSE2", ff_conv_flt_to_s16p_6ch_sse2);
    //     ff_audio_convert_set_func(ac, AV_SAMPLE_FMT_FLTP, AV_SAMPLE_FMT_FLT,6, 16, 4, "SSE2", ff_conv_flt_to_fltp_6ch_sse2);
    // }
    // if (EXTERNAL_SSSE3(cpu_flags)) {
    //     ff_audio_convert_set_func(ac, AV_SAMPLE_FMT_FLT, AV_SAMPLE_FMT_S16P,
    //                               6, 16, 4, "SSSE3", ff_conv_s16p_to_flt_6ch_ssse3);
    //     ff_audio_convert_set_func(ac, AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_FLTP,
    //                               2, 16, 4, "SSSE3", ff_conv_fltp_to_s16_2ch_ssse3);
    //     ff_audio_convert_set_func(ac, AV_SAMPLE_FMT_S16P, AV_SAMPLE_FMT_S16,
    //                               2, 16, 8, "SSSE3", ff_conv_s16_to_s16p_2ch_ssse3);
    //     ff_audio_convert_set_func(ac, AV_SAMPLE_FMT_S16P, AV_SAMPLE_FMT_S16,
    //                               6, 16, 4, "SSSE3", ff_conv_s16_to_s16p_6ch_ssse3);
    //     ff_audio_convert_set_func(ac, AV_SAMPLE_FMT_FLTP, AV_SAMPLE_FMT_S16,
    //                               6, 16, 4, "SSSE3", ff_conv_s16_to_fltp_6ch_ssse3);
    //     ff_audio_convert_set_func(ac, AV_SAMPLE_FMT_S16P, AV_SAMPLE_FMT_FLT,
    //                               6, 16, 4, "SSSE3", ff_conv_flt_to_s16p_6ch_ssse3);
    // }
    // if (EXTERNAL_SSE4(cpu_flags)) {
    //     ff_audio_convert_set_func(ac, AV_SAMPLE_FMT_FLT, AV_SAMPLE_FMT_S16,
    //                               0, 16, 8, "SSE4", ff_conv_s16_to_flt_sse4);
    //     ff_audio_convert_set_func(ac, AV_SAMPLE_FMT_FLT, AV_SAMPLE_FMT_FLTP,
    //                               6, 16, 4, "SSE4", ff_conv_fltp_to_flt_6ch_sse4);
    // }
    // if (EXTERNAL_AVX_FAST(cpu_flags)) {
    //     ff_audio_convert_set_func(ac, AV_SAMPLE_FMT_FLT, AV_SAMPLE_FMT_S32,
    //                               0, 32, 16, "AVX", ff_conv_s32_to_flt_avx);
    //     ff_audio_convert_set_func(ac, AV_SAMPLE_FMT_S32, AV_SAMPLE_FMT_FLT,
    //                               0, 32, 32, "AVX", ff_conv_flt_to_s32_avx);
    // }
    // if (EXTERNAL_AVX(cpu_flags)) {
    //     ff_audio_convert_set_func(ac, AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_S16P,
    //                               2, 16, 16, "AVX", ff_conv_s16p_to_s16_2ch_avx);
    //     ff_audio_convert_set_func(ac, AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_S16P,
    //                               6, 16, 8, "AVX", ff_conv_s16p_to_s16_6ch_avx);
    //     ff_audio_convert_set_func(ac, AV_SAMPLE_FMT_FLT, AV_SAMPLE_FMT_S16P,
    //                               2, 16, 8, "AVX", ff_conv_s16p_to_flt_2ch_avx);
    //     ff_audio_convert_set_func(ac, AV_SAMPLE_FMT_FLT, AV_SAMPLE_FMT_S16P,
    //                               6, 16, 4, "AVX", ff_conv_s16p_to_flt_6ch_avx);
    //     ff_audio_convert_set_func(ac, AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_FLTP,
    //                               6, 16, 4, "AVX", ff_conv_fltp_to_s16_6ch_avx);
    //     ff_audio_convert_set_func(ac, AV_SAMPLE_FMT_FLT, AV_SAMPLE_FMT_FLTP,
    //                               6, 16, 4, "AVX", ff_conv_fltp_to_flt_6ch_avx);
    //     ff_audio_convert_set_func(ac, AV_SAMPLE_FMT_S16P, AV_SAMPLE_FMT_S16,
    //                               2, 16, 8, "AVX", ff_conv_s16_to_s16p_2ch_avx);
    //     ff_audio_convert_set_func(ac, AV_SAMPLE_FMT_S16P, AV_SAMPLE_FMT_S16,
    //                               6, 16, 4, "AVX", ff_conv_s16_to_s16p_6ch_avx);
    //     ff_audio_convert_set_func(ac, AV_SAMPLE_FMT_FLTP, AV_SAMPLE_FMT_S16,
    //                               2, 16, 8, "AVX", ff_conv_s16_to_fltp_2ch_avx);
    //     ff_audio_convert_set_func(ac, AV_SAMPLE_FMT_FLTP, AV_SAMPLE_FMT_S16,
    //                               6, 16, 4, "AVX", ff_conv_s16_to_fltp_6ch_avx);
    //     ff_audio_convert_set_func(ac, AV_SAMPLE_FMT_S16P, AV_SAMPLE_FMT_FLT,
    //                               2, 16, 8, "AVX", ff_conv_flt_to_s16p_2ch_avx);
    //     ff_audio_convert_set_func(ac, AV_SAMPLE_FMT_S16P, AV_SAMPLE_FMT_FLT,
    //                               6, 16, 4, "AVX", ff_conv_flt_to_s16p_6ch_avx);
    //     ff_audio_convert_set_func(ac, AV_SAMPLE_FMT_FLTP, AV_SAMPLE_FMT_FLT,
    //                               2, 16, 4, "AVX", ff_conv_flt_to_fltp_2ch_avx);
    //     ff_audio_convert_set_func(ac, AV_SAMPLE_FMT_FLTP, AV_SAMPLE_FMT_FLT,
    //                               6, 16, 4, "AVX", ff_conv_flt_to_fltp_6ch_avx);
    // }
}

AudioConvert *ff_audio_convert_alloc(AVAudioResampleContext *avr,
                                     enum AVSampleFormat out_fmt,
                                     enum AVSampleFormat in_fmt,
                                     int channels, int sample_rate,
                                     int apply_map)
{
    AudioConvert *ac;
    int in_planar, out_planar;

    ac = av_mallocz(sizeof(*ac));
    if (!ac)
        return NULL;

    ac->avr      = avr;
    ac->out_fmt  = out_fmt;
    ac->in_fmt   = in_fmt;
    ac->channels = channels;
    ac->apply_map = apply_map;

    if (avr->dither_method != AV_RESAMPLE_DITHER_NONE          &&
        av_get_packed_sample_fmt(out_fmt) == AV_SAMPLE_FMT_S16 &&
        av_get_bytes_per_sample(in_fmt) > 2) {
        ac->dc = ff_dither_alloc(avr, out_fmt, in_fmt, channels, sample_rate,
                                 apply_map);
        if (!ac->dc) {
            av_free(ac);
            return NULL;
        }
        return ac;
    }

    in_planar  = ff_sample_fmt_is_planar(in_fmt, channels);
    out_planar = ff_sample_fmt_is_planar(out_fmt, channels);

    if (in_planar == out_planar) {
        ac->func_type = CONV_FUNC_TYPE_FLAT;
        ac->planes    = in_planar ? ac->channels : 1;
    } else if (in_planar)
        ac->func_type = CONV_FUNC_TYPE_INTERLEAVE;
    else
        ac->func_type = CONV_FUNC_TYPE_DEINTERLEAVE;

    set_generic_function(ac);

    // if (ARCH_AARCH64)
    //     ff_audio_convert_init_aarch64(ac);
    // if (ARCH_ARM)
    //     ff_audio_convert_init_arm(ac);
    if (ARCH_X86)
        ff_audio_convert_init_x86(ac);

    return ac;
}


static double bessel(double x)
{
    double v     = 1;
    double lastv = 0;
    double t     = 1;
    int i;

    x = x * x / 4;
    for (i = 1; v != lastv; i++) {
        lastv = v;
        t    *= x / (i * i);
        v    += t;
    }
    return v;
}
static int build_filter(ResampleContext *c, double factor)
{
    int ph, i;
    double x, y, w;
    double *tab;
    int tap_count    = c->filter_length;
    int phase_count  = 1 << c->phase_shift;
    const int center = (tap_count - 1) / 2;

    tab = av_malloc(tap_count * sizeof(*tab));
    if (!tab)
        return AVERROR(ENOMEM);

    for (ph = 0; ph < phase_count; ph++) {
        double norm = 0;
        for (i = 0; i < tap_count; i++) {
            x = M_PI * ((double)(i - center) - (double)ph / phase_count) * factor;
            if (x == 0) y = 1.0;
            else        y = sin(x) / x;
            switch (c->filter_type) {
            case AV_RESAMPLE_FILTER_TYPE_CUBIC: {
                const float d = -0.5; //first order derivative = -0.5
                x = fabs(((double)(i - center) - (double)ph / phase_count) * factor);
                if (x < 1.0) y = 1 - 3 * x*x + 2 * x*x*x + d * (                -x*x + x*x*x);
                else         y =                           d * (-4 + 8 * x - 5 * x*x + x*x*x);
                break;
            }
            case AV_RESAMPLE_FILTER_TYPE_BLACKMAN_NUTTALL:
                w  = 2.0 * x / (factor * tap_count) + M_PI;
                y *= 0.3635819 - 0.4891775 * cos(    w) +
                                 0.1365995 * cos(2 * w) -
                                 0.0106411 * cos(3 * w);
                break;
            case AV_RESAMPLE_FILTER_TYPE_KAISER:
                w  = 2.0 * x / (factor * tap_count * M_PI);
                y *= bessel(c->kaiser_beta * sqrt(FFMAX(1 - w * w, 0)));
                break;
            }

            tab[i] = y;
            norm  += y;
        }
        /* normalize so that an uniform color remains the same */
        for (i = 0; i < tap_count; i++)
            tab[i] = tab[i] / norm;

        c->set_filter(c->filter_bank, tab, ph, tap_count);
    }

    av_free(tab);
    return 0;
}



ResampleContext *ff_audio_resample_init(AVAudioResampleContext *avr)
{
    ResampleContext *c;
    int out_rate    = avr->out_sample_rate;
    int in_rate     = avr->in_sample_rate;
    double factor   = FFMIN(out_rate * avr->cutoff / in_rate, 1.0);
    int phase_count = 1 << avr->phase_shift;
    int felem_size;

    if (avr->internal_sample_fmt != AV_SAMPLE_FMT_S16P &&
        avr->internal_sample_fmt != AV_SAMPLE_FMT_S32P &&
        avr->internal_sample_fmt != AV_SAMPLE_FMT_FLTP &&
        avr->internal_sample_fmt != AV_SAMPLE_FMT_DBLP) {
        av_log(avr, AV_LOG_ERROR, "Unsupported internal format for "
               "resampling: %s\n",
               av_get_sample_fmt_name(avr->internal_sample_fmt));
        return NULL;
    }
    c = av_mallocz(sizeof(*c));
    if (!c)
        return NULL;

    c->avr           = avr;
    c->phase_shift   = avr->phase_shift;
    c->phase_mask    = phase_count - 1;
    c->linear        = avr->linear_interp;
    c->filter_length = FFMAX((int)ceil(avr->filter_size / factor), 1);
    c->filter_type   = avr->filter_type;
    c->kaiser_beta   = avr->kaiser_beta;

    switch (avr->internal_sample_fmt) {
    // case AV_SAMPLE_FMT_DBLP:
    //     c->resample_one  = c->linear ? resample_linear_dbl : resample_one_dbl;
    //     c->resample_nearest = resample_nearest_dbl;
    //     c->set_filter    = set_filter_dbl;
    //     break;
    // case AV_SAMPLE_FMT_FLTP:
    //     c->resample_one  = c->linear ? resample_linear_flt : resample_one_flt;
    //     c->resample_nearest = resample_nearest_flt;
    //     c->set_filter    = set_filter_flt;
    //     break;
    // case AV_SAMPLE_FMT_S32P:
    //     c->resample_one  = c->linear ? resample_linear_s32 : resample_one_s32;
    //     c->resample_nearest = resample_nearest_s32;
    //     c->set_filter    = set_filter_s32;
    //     break;
    // case AV_SAMPLE_FMT_S16P:
    //     c->resample_one  = c->linear ? resample_linear_s16 : resample_one_s16;
    //     c->resample_nearest = resample_nearest_s16;
    //     c->set_filter    = set_filter_s16;
    //     break;
    }

    if (ARCH_AARCH64)
        ff_audio_resample_init_aarch64(c, avr->internal_sample_fmt);
    if (ARCH_ARM)
        ff_audio_resample_init_arm(c, avr->internal_sample_fmt);

    felem_size = av_get_bytes_per_sample(avr->internal_sample_fmt);
    c->filter_bank = av_mallocz(c->filter_length * (phase_count + 1) * felem_size);
    if (!c->filter_bank)
        goto error;

    if (build_filter(c, factor) < 0)
        goto error;

    memcpy(&c->filter_bank[(c->filter_length * phase_count + 1) * felem_size],
           c->filter_bank, (c->filter_length - 1) * felem_size);
    memcpy(&c->filter_bank[c->filter_length * phase_count * felem_size],
           &c->filter_bank[(c->filter_length - 1) * felem_size], felem_size);

    c->compensation_distance = 0;
    if (!av_reduce(&c->src_incr, &c->dst_incr, out_rate,
                   in_rate * (int64_t)phase_count, INT32_MAX / 2))
        goto error;
    c->ideal_dst_incr = c->dst_incr;

    c->padding_size   = (c->filter_length - 1) / 2;
    c->initial_padding_filled = 0;
    c->index = 0;
    c->frac  = 0;

    /* allocate internal buffer */
    c->buffer = ff_audio_data_alloc(avr->resample_channels, c->padding_size,
                                    avr->internal_sample_fmt,
                                    "resample buffer");
    if (!c->buffer)
        goto error;
    c->buffer->nb_samples      = c->padding_size;
    c->initial_padding_samples = c->padding_size;

    av_log(avr, AV_LOG_DEBUG, "resample: %s from %d Hz to %d Hz\n",
           av_get_sample_fmt_name(avr->internal_sample_fmt),
           avr->in_sample_rate, avr->out_sample_rate);

    return c;

error:
    ff_audio_data_free(&c->buffer);
    av_free(c->filter_bank);
    av_free(c);
    return NULL;
}

static const char * const coeff_type_names[] = { "q8", "q15", "flt" };

void ff_audio_mix_set_func(AudioMix *am, enum AVSampleFormat fmt,
                           enum AVMixCoeffType coeff_type, int in_channels,
                           int out_channels, int ptr_align, int samples_align,
                           const char *descr, void *mix_func)
{
    if (fmt == am->fmt && coeff_type == am->coeff_type &&
        ( in_channels ==  am->in_matrix_channels ||  in_channels == 0) &&
        (out_channels == am->out_matrix_channels || out_channels == 0)) {
        char chan_str[16];
        am->mix           = mix_func;
        am->func_descr    = descr;
        am->ptr_align     = ptr_align;
        am->samples_align = samples_align;
        if (ptr_align == 1 && samples_align == 1) {
            am->mix_generic        = mix_func;
            am->func_descr_generic = descr;
        } else {
            am->has_optimized_func = 1;
        }
        if (in_channels) {
            if (out_channels)
                snprintf(chan_str, sizeof(chan_str), "[%d to %d] ",
                         in_channels, out_channels);
            else
                snprintf(chan_str, sizeof(chan_str), "[%d to any] ",
                         in_channels);
        } else if (out_channels) {
                snprintf(chan_str, sizeof(chan_str), "[any to %d] ",
                         out_channels);
        } else {
            snprintf(chan_str, sizeof(chan_str), "[any to any] ");
        }
        av_log(am->avr, AV_LOG_DEBUG, "audio_mix: found function: [fmt=%s] "
               "[c=%s] %s(%s)\n", av_get_sample_fmt_name(fmt),
               coeff_type_names[coeff_type], chan_str, descr);
    }
}


#define MIX_FUNC_NAME(fmt, cfmt) mix_any_ ## fmt ##_## cfmt ##_c

#define MIX_FUNC_GENERIC(fmt, cfmt, stype, ctype, sumtype, expr)            \
static void MIX_FUNC_NAME(fmt, cfmt)(stype **samples, ctype **matrix,       \
                                     int len, int out_ch, int in_ch)        \
{                                                                           \
    int i, in, out;                                                         \
    stype temp[AVRESAMPLE_MAX_CHANNELS];                                    \
    for (i = 0; i < len; i++) {                                             \
        for (out = 0; out < out_ch; out++) {                                \
            sumtype sum = 0;                                                \
            for (in = 0; in < in_ch; in++)                                  \
                sum += samples[in][i] * matrix[out][in];                    \
            temp[out] = expr;                                               \
        }                                                                   \
        for (out = 0; out < out_ch; out++)                                  \
            samples[out][i] = temp[out];                                    \
    }                                                                       \
}

MIX_FUNC_GENERIC(FLTP, FLT, float,   float,   float,   sum)
MIX_FUNC_GENERIC(S16P, FLT, int16_t, float,   float,   av_clip_int16(lrintf(sum)))
MIX_FUNC_GENERIC(S16P, Q15, int16_t, int32_t, int64_t, av_clip_int16(sum >> 15))
MIX_FUNC_GENERIC(S16P, Q8,  int16_t, int16_t, int32_t, av_clip_int16(sum >>  8))

/* TODO: templatize the channel-specific C functions */

static void mix_2_to_1_fltp_flt_c(float **samples, float **matrix, int len,
                                  int out_ch, int in_ch)
{
    float *src0 = samples[0];
    float *src1 = samples[1];
    float *dst  = src0;
    float m0    = matrix[0][0];
    float m1    = matrix[0][1];

    while (len > 4) {
        *dst++ = *src0++ * m0 + *src1++ * m1;
        *dst++ = *src0++ * m0 + *src1++ * m1;
        *dst++ = *src0++ * m0 + *src1++ * m1;
        *dst++ = *src0++ * m0 + *src1++ * m1;
        len -= 4;
    }
    while (len > 0) {
        *dst++ = *src0++ * m0 + *src1++ * m1;
        len--;
    }
}

static void mix_2_to_1_s16p_flt_c(int16_t **samples, float **matrix, int len,
                                  int out_ch, int in_ch)
{
    int16_t *src0 = samples[0];
    int16_t *src1 = samples[1];
    int16_t *dst  = src0;
    float m0      = matrix[0][0];
    float m1      = matrix[0][1];

    while (len > 4) {
        *dst++ = av_clip_int16(lrintf(*src0++ * m0 + *src1++ * m1));
        *dst++ = av_clip_int16(lrintf(*src0++ * m0 + *src1++ * m1));
        *dst++ = av_clip_int16(lrintf(*src0++ * m0 + *src1++ * m1));
        *dst++ = av_clip_int16(lrintf(*src0++ * m0 + *src1++ * m1));
        len -= 4;
    }
    while (len > 0) {
        *dst++ = av_clip_int16(lrintf(*src0++ * m0 + *src1++ * m1));
        len--;
    }
}

static void mix_2_to_1_s16p_q8_c(int16_t **samples, int16_t **matrix, int len,
                                 int out_ch, int in_ch)
{
    int16_t *src0 = samples[0];
    int16_t *src1 = samples[1];
    int16_t *dst  = src0;
    int16_t m0    = matrix[0][0];
    int16_t m1    = matrix[0][1];

    while (len > 4) {
        *dst++ = (*src0++ * m0 + *src1++ * m1) >> 8;
        *dst++ = (*src0++ * m0 + *src1++ * m1) >> 8;
        *dst++ = (*src0++ * m0 + *src1++ * m1) >> 8;
        *dst++ = (*src0++ * m0 + *src1++ * m1) >> 8;
        len -= 4;
    }
    while (len > 0) {
        *dst++ = (*src0++ * m0 + *src1++ * m1) >> 8;
        len--;
    }
}

static void mix_1_to_2_fltp_flt_c(float **samples, float **matrix, int len,
                                  int out_ch, int in_ch)
{
    float v;
    float *dst0 = samples[0];
    float *dst1 = samples[1];
    float *src  = dst0;
    float m0    = matrix[0][0];
    float m1    = matrix[1][0];

    while (len > 4) {
        v = *src++;
        *dst0++ = v * m0;
        *dst1++ = v * m1;
        v = *src++;
        *dst0++ = v * m0;
        *dst1++ = v * m1;
        v = *src++;
        *dst0++ = v * m0;
        *dst1++ = v * m1;
        v = *src++;
        *dst0++ = v * m0;
        *dst1++ = v * m1;
        len -= 4;
    }
    while (len > 0) {
        v = *src++;
        *dst0++ = v * m0;
        *dst1++ = v * m1;
        len--;
    }
}

static void mix_6_to_2_fltp_flt_c(float **samples, float **matrix, int len,
                                  int out_ch, int in_ch)
{
    float v0, v1;
    float *src0 = samples[0];
    float *src1 = samples[1];
    float *src2 = samples[2];
    float *src3 = samples[3];
    float *src4 = samples[4];
    float *src5 = samples[5];
    float *dst0 = src0;
    float *dst1 = src1;
    float *m0   = matrix[0];
    float *m1   = matrix[1];

    while (len > 0) {
        v0 = *src0++;
        v1 = *src1++;
        *dst0++ = v0      * m0[0] +
                  v1      * m0[1] +
                  *src2   * m0[2] +
                  *src3   * m0[3] +
                  *src4   * m0[4] +
                  *src5   * m0[5];
        *dst1++ = v0      * m1[0] +
                  v1      * m1[1] +
                  *src2++ * m1[2] +
                  *src3++ * m1[3] +
                  *src4++ * m1[4] +
                  *src5++ * m1[5];
        len--;
    }
}

static void mix_2_to_6_fltp_flt_c(float **samples, float **matrix, int len,
                                  int out_ch, int in_ch)
{
    float v0, v1;
    float *dst0 = samples[0];
    float *dst1 = samples[1];
    float *dst2 = samples[2];
    float *dst3 = samples[3];
    float *dst4 = samples[4];
    float *dst5 = samples[5];
    float *src0 = dst0;
    float *src1 = dst1;

    while (len > 0) {
        v0 = *src0++;
        v1 = *src1++;
        *dst0++ = v0 * matrix[0][0] + v1 * matrix[0][1];
        *dst1++ = v0 * matrix[1][0] + v1 * matrix[1][1];
        *dst2++ = v0 * matrix[2][0] + v1 * matrix[2][1];
        *dst3++ = v0 * matrix[3][0] + v1 * matrix[3][1];
        *dst4++ = v0 * matrix[4][0] + v1 * matrix[4][1];
        *dst5++ = v0 * matrix[5][0] + v1 * matrix[5][1];
        len--;
    }
}
#define CONVERT_MATRIX(type, expr)                                          \
    am->matrix_## type[0] = av_mallocz(am->out_matrix_channels *            \
                                       am->in_matrix_channels  *            \
                                       sizeof(*am->matrix_## type[0]));     \
    if (!am->matrix_## type[0])                                             \
        return AVERROR(ENOMEM);                                             \
    for (o = 0, o0 = 0; o < am->out_channels; o++) {                        \
        if (am->output_zero[o] || am->output_skip[o])                       \
            continue;                                                       \
        if (o0 > 0)                                                         \
            am->matrix_## type[o0] = am->matrix_## type[o0 - 1] +           \
                                     am->in_matrix_channels;                \
        for (i = 0, i0 = 0; i < am->in_channels; i++) {                     \
            double v;                                                       \
            if (am->input_skip[i] || am->output_zero[i])                    \
                continue;                                                   \
            v = matrix[o * stride + i];                                     \
            am->matrix_## type[o0][i0] = expr;                              \
            i0++;                                                           \
        }                                                                   \
        o0++;                                                               \
    }                                                                       \
    am->matrix = (void **)am->matrix_## type;


static av_cold int mix_function_init(AudioMix *am)
{
    am->func_descr = am->func_descr_generic = "n/a";
    am->mix = am->mix_generic = NULL;

    /* no need to set a mix function when we're skipping mixing */
    if (!am->in_matrix_channels || !am->out_matrix_channels)
        return 0;

    /* any-to-any C versions */

    ff_audio_mix_set_func(am, AV_SAMPLE_FMT_FLTP, AV_MIX_COEFF_TYPE_FLT,
                          0, 0, 1, 1, "C", MIX_FUNC_NAME(FLTP, FLT));

    ff_audio_mix_set_func(am, AV_SAMPLE_FMT_S16P, AV_MIX_COEFF_TYPE_FLT,
                          0, 0, 1, 1, "C", MIX_FUNC_NAME(S16P, FLT));

    ff_audio_mix_set_func(am, AV_SAMPLE_FMT_S16P, AV_MIX_COEFF_TYPE_Q15,
                          0, 0, 1, 1, "C", MIX_FUNC_NAME(S16P, Q15));

    ff_audio_mix_set_func(am, AV_SAMPLE_FMT_S16P, AV_MIX_COEFF_TYPE_Q8,
                          0, 0, 1, 1, "C", MIX_FUNC_NAME(S16P, Q8));

    /* channel-specific C versions */

    ff_audio_mix_set_func(am, AV_SAMPLE_FMT_FLTP, AV_MIX_COEFF_TYPE_FLT,
                          2, 1, 1, 1, "C", mix_2_to_1_fltp_flt_c);

    ff_audio_mix_set_func(am, AV_SAMPLE_FMT_S16P, AV_MIX_COEFF_TYPE_FLT,
                          2, 1, 1, 1, "C", mix_2_to_1_s16p_flt_c);

    ff_audio_mix_set_func(am, AV_SAMPLE_FMT_S16P, AV_MIX_COEFF_TYPE_Q8,
                          2, 1, 1, 1, "C", mix_2_to_1_s16p_q8_c);

    ff_audio_mix_set_func(am, AV_SAMPLE_FMT_FLTP, AV_MIX_COEFF_TYPE_FLT,
                          1, 2, 1, 1, "C", mix_1_to_2_fltp_flt_c);

    ff_audio_mix_set_func(am, AV_SAMPLE_FMT_FLTP, AV_MIX_COEFF_TYPE_FLT,
                          6, 2, 1, 1, "C", mix_6_to_2_fltp_flt_c);

    ff_audio_mix_set_func(am, AV_SAMPLE_FMT_FLTP, AV_MIX_COEFF_TYPE_FLT,
                          2, 6, 1, 1, "C", mix_2_to_6_fltp_flt_c);

    // if (ARCH_X86)
    //     ff_audio_mix_init_x86(am);

    if (!am->mix) {
        av_log(am->avr, AV_LOG_ERROR, "audio_mix: NO FUNCTION FOUND: [fmt=%s] "
               "[c=%s] [%d to %d]\n", av_get_sample_fmt_name(am->fmt),
               coeff_type_names[am->coeff_type], am->in_channels,
               am->out_channels);
        return AVERROR_PATCHWELCOME;
    }
    return 0;
}

static void reduce_matrix(AudioMix *am, const double *matrix, int stride)
{
    int i, o;

    memset(am->output_zero, 0, sizeof(am->output_zero));
    memset(am->input_skip,  0, sizeof(am->input_skip));
    memset(am->output_skip, 0, sizeof(am->output_skip));

    /* exclude output channels if they can be zeroed instead of mixed */
    for (o = 0; o < am->out_channels; o++) {
        int zero = 1;

        /* check if the output is always silent */
        for (i = 0; i < am->in_channels; i++) {
            if (matrix[o * stride + i] != 0.0) {
                zero = 0;
                break;
            }
        }
        /* check if the corresponding input channel makes a contribution to
           any output channel */
        if (o < am->in_channels) {
            for (i = 0; i < am->out_channels; i++) {
                if (matrix[i * stride + o] != 0.0) {
                    zero = 0;
                    break;
                }
            }
        }
        if (zero) {
            am->output_zero[o] = 1;
            am->out_matrix_channels--;
            if (o < am->in_channels)
                am->in_matrix_channels--;
        }
    }
    if (am->out_matrix_channels == 0 || am->in_matrix_channels == 0) {
        am->out_matrix_channels = 0;
        am->in_matrix_channels = 0;
        return;
    }

    /* skip input channels that contribute fully only to the corresponding
       output channel */
    for (i = 0; i < FFMIN(am->in_channels, am->out_channels); i++) {
        int skip = 1;

        for (o = 0; o < am->out_channels; o++) {
            int i0;
            if ((o != i && matrix[o * stride + i] != 0.0) ||
                (o == i && matrix[o * stride + i] != 1.0)) {
                skip = 0;
                break;
            }
            /* if the input contributes fully to the output, also check that no
               other inputs contribute to this output */
            if (o == i) {
                for (i0 = 0; i0 < am->in_channels; i0++) {
                    if (i0 != i && matrix[o * stride + i0] != 0.0) {
                        skip = 0;
                        break;
                    }
                }
            }
        }
        if (skip) {
            am->input_skip[i] = 1;
            am->in_matrix_channels--;
        }
    }
    /* skip input channels that do not contribute to any output channel */
    for (; i < am->in_channels; i++) {
        int contrib = 0;

        for (o = 0; o < am->out_channels; o++) {
            if (matrix[o * stride + i] != 0.0) {
                contrib = 1;
                break;
            }
        }
        if (!contrib) {
            am->input_skip[i] = 1;
            am->in_matrix_channels--;
        }
    }
    if (am->in_matrix_channels == 0) {
        am->out_matrix_channels = 0;
        return;
    }

    /* skip output channels that only get full contribution from the
       corresponding input channel */
    for (o = 0; o < FFMIN(am->in_channels, am->out_channels); o++) {
        int skip = 1;
        int o0;

        for (i = 0; i < am->in_channels; i++) {
            if ((o != i && matrix[o * stride + i] != 0.0) ||
                (o == i && matrix[o * stride + i] != 1.0)) {
                skip = 0;
                break;
            }
        }
        /* check if the corresponding input channel makes a contribution to
           any other output channel */
        i = o;
        for (o0 = 0; o0 < am->out_channels; o0++) {
            if (o0 != i && matrix[o0 * stride + i] != 0.0) {
                skip = 0;
                break;
            }
        }
        if (skip) {
            am->output_skip[o] = 1;
            am->out_matrix_channels--;
        }
    }
    if (am->out_matrix_channels == 0) {
        am->in_matrix_channels = 0;
        return;
    }
}
int ff_audio_mix_set_matrix(AudioMix *am, const double *matrix, int stride)
{
    int i, o, i0, o0, ret;
    char in_layout_name[128];
    char out_layout_name[128];

    if ( am->in_channels <= 0 ||  am->in_channels > AVRESAMPLE_MAX_CHANNELS ||
        am->out_channels <= 0 || am->out_channels > AVRESAMPLE_MAX_CHANNELS) {
        av_log(am->avr, AV_LOG_ERROR, "Invalid channel counts\n");
        return AVERROR(EINVAL);
    }

    if (am->matrix) {
        av_free(am->matrix[0]);
        am->matrix = NULL;
    }

    am->in_matrix_channels  = am->in_channels;
    am->out_matrix_channels = am->out_channels;

    reduce_matrix(am, matrix, stride);



    if (am->in_matrix_channels && am->out_matrix_channels) {
        switch (am->coeff_type) {
        case AV_MIX_COEFF_TYPE_Q8:
            CONVERT_MATRIX(q8, av_clip_int16(lrint(256.0 * v)))
            break;
        case AV_MIX_COEFF_TYPE_Q15:
            CONVERT_MATRIX(q15, av_clipl_int32(llrint(32768.0 * v)))
            break;
        case AV_MIX_COEFF_TYPE_FLT:
            CONVERT_MATRIX(flt, v)
            break;
        default:
            av_log(am->avr, AV_LOG_ERROR, "Invalid mix coeff type\n");
            return AVERROR(EINVAL);
        }
    }

    ret = mix_function_init(am);
    if (ret < 0)
        return ret;

    av_get_channel_layout_string(in_layout_name, sizeof(in_layout_name),
                                 am->in_channels, am->in_layout);
    av_get_channel_layout_string(out_layout_name, sizeof(out_layout_name),
                                 am->out_channels, am->out_layout);
    av_log(am->avr, AV_LOG_DEBUG, "audio_mix: %s to %s\n",
           in_layout_name, out_layout_name);
    av_log(am->avr, AV_LOG_DEBUG, "matrix size: %d x %d\n",
           am->in_matrix_channels, am->out_matrix_channels);
    for (o = 0; o < am->out_channels; o++) {
        for (i = 0; i < am->in_channels; i++) {
            if (am->output_zero[o])
                av_log(am->avr, AV_LOG_DEBUG, "  (ZERO)");
            else if (am->input_skip[i] || am->output_zero[i] || am->output_skip[o])
                av_log(am->avr, AV_LOG_DEBUG, "  (SKIP)");
            else
                av_log(am->avr, AV_LOG_DEBUG, "  %0.3f ",
                       matrix[o * am->in_channels + i]);
        }
        av_log(am->avr, AV_LOG_DEBUG, "\n");
    }

    return 0;
}

#define FRONT_LEFT              0
#define FRONT_RIGHT             1
#define FRONT_CENTER            2
#define LOW_FREQUENCY           3
#define BACK_LEFT               4
#define BACK_RIGHT              5
#define FRONT_LEFT_OF_CENTER    6
#define FRONT_RIGHT_OF_CENTER   7
#define BACK_CENTER             8
#define SIDE_LEFT               9
#define SIDE_RIGHT             10
#define TOP_CENTER             11
#define TOP_FRONT_LEFT         12
#define TOP_FRONT_CENTER       13
#define TOP_FRONT_RIGHT        14
#define TOP_BACK_LEFT          15
#define TOP_BACK_CENTER        16
#define TOP_BACK_RIGHT         17
#define STEREO_LEFT            29
#define STEREO_RIGHT           30
#define WIDE_LEFT              31
#define WIDE_RIGHT             32
#define SURROUND_DIRECT_LEFT   33
#define SURROUND_DIRECT_RIGHT  34
#define LOW_FREQUENCY_2        35

#define SQRT3_2      1.22474487139158904909  /* sqrt(3/2) */

static inline int even(uint64_t layout)
{
    return (!layout || !!(layout & (layout - 1)));
}

static int sane_layout(uint64_t layout)
{
    /* check that there is at least 1 front speaker */
    if (!(layout & AV_CH_LAYOUT_SURROUND))
        return 0;

    /* check for left/right symmetry */
    if (!even(layout & (AV_CH_FRONT_LEFT           | AV_CH_FRONT_RIGHT))           ||
        !even(layout & (AV_CH_SIDE_LEFT            | AV_CH_SIDE_RIGHT))            ||
        !even(layout & (AV_CH_BACK_LEFT            | AV_CH_BACK_RIGHT))            ||
        !even(layout & (AV_CH_FRONT_LEFT_OF_CENTER | AV_CH_FRONT_RIGHT_OF_CENTER)) ||
        !even(layout & (AV_CH_TOP_FRONT_LEFT       | AV_CH_TOP_FRONT_RIGHT))       ||
        !even(layout & (AV_CH_TOP_BACK_LEFT        | AV_CH_TOP_BACK_RIGHT))        ||
        !even(layout & (AV_CH_STEREO_LEFT          | AV_CH_STEREO_RIGHT))          ||
        !even(layout & (AV_CH_WIDE_LEFT            | AV_CH_WIDE_RIGHT))            ||
        !even(layout & (AV_CH_SURROUND_DIRECT_LEFT | AV_CH_SURROUND_DIRECT_RIGHT)))
        return 0;

    return 1;
}

int avresample_build_matrix(uint64_t in_layout, uint64_t out_layout,
                            double center_mix_level, double surround_mix_level,
                            double lfe_mix_level, int normalize,
                            double *matrix_out, int stride,
                            enum AVMatrixEncoding matrix_encoding)
{
    int i, j, out_i, out_j;
    double matrix[64][64] = {{0}};
    int64_t unaccounted;
    double maxcoef = 0;
    int in_channels, out_channels;

    if ((out_layout & AV_CH_LAYOUT_STEREO_DOWNMIX) == AV_CH_LAYOUT_STEREO_DOWNMIX) {
        out_layout = AV_CH_LAYOUT_STEREO;
    }

    unaccounted = in_layout & ~out_layout;

    in_channels  = av_get_channel_layout_nb_channels( in_layout);
    out_channels = av_get_channel_layout_nb_channels(out_layout);

    memset(matrix_out, 0, out_channels * stride * sizeof(*matrix_out));

    /* check if layouts are supported */
    if (!in_layout || in_channels > AVRESAMPLE_MAX_CHANNELS)
        return AVERROR(EINVAL);
    if (!out_layout || out_channels > AVRESAMPLE_MAX_CHANNELS)
        return AVERROR(EINVAL);

    /* check if layouts are unbalanced or abnormal */
    if (!sane_layout(in_layout) || !sane_layout(out_layout))
        return AVERROR_PATCHWELCOME;

    /* route matching input/output channels */
    for (i = 0; i < 64; i++) {
        if (in_layout & out_layout & (1ULL << i))
            matrix[i][i] = 1.0;
    }

    /* mix front center to front left/right */
    if (unaccounted & AV_CH_FRONT_CENTER) {
        if ((out_layout & AV_CH_LAYOUT_STEREO) == AV_CH_LAYOUT_STEREO) {
            if ((in_layout & AV_CH_LAYOUT_STEREO) == AV_CH_LAYOUT_STEREO) {
                matrix[FRONT_LEFT ][FRONT_CENTER] += center_mix_level;
                matrix[FRONT_RIGHT][FRONT_CENTER] += center_mix_level;
            } else {
                matrix[FRONT_LEFT ][FRONT_CENTER] += M_SQRT1_2;
                matrix[FRONT_RIGHT][FRONT_CENTER] += M_SQRT1_2;
            }
        } else
            return AVERROR_PATCHWELCOME;
    }
    /* mix front left/right to center */
    if (unaccounted & AV_CH_LAYOUT_STEREO) {
        if (out_layout & AV_CH_FRONT_CENTER) {
            matrix[FRONT_CENTER][FRONT_LEFT ] += M_SQRT1_2;
            matrix[FRONT_CENTER][FRONT_RIGHT] += M_SQRT1_2;
            /* mix left/right/center to center */
            if (in_layout & AV_CH_FRONT_CENTER)
                matrix[FRONT_CENTER][FRONT_CENTER] = center_mix_level * M_SQRT2;
        } else
            return AVERROR_PATCHWELCOME;
    }
    /* mix back center to back, side, or front */
    if (unaccounted & AV_CH_BACK_CENTER) {
        if (out_layout & AV_CH_BACK_LEFT) {
            matrix[BACK_LEFT ][BACK_CENTER] += M_SQRT1_2;
            matrix[BACK_RIGHT][BACK_CENTER] += M_SQRT1_2;
        } else if (out_layout & AV_CH_SIDE_LEFT) {
            matrix[SIDE_LEFT ][BACK_CENTER] += M_SQRT1_2;
            matrix[SIDE_RIGHT][BACK_CENTER] += M_SQRT1_2;
        } else if (out_layout & AV_CH_FRONT_LEFT) {
            if (matrix_encoding == AV_MATRIX_ENCODING_DOLBY ||
                matrix_encoding == AV_MATRIX_ENCODING_DPLII) {
                if (unaccounted & (AV_CH_BACK_LEFT | AV_CH_SIDE_LEFT)) {
                    matrix[FRONT_LEFT ][BACK_CENTER] -= surround_mix_level * M_SQRT1_2;
                    matrix[FRONT_RIGHT][BACK_CENTER] += surround_mix_level * M_SQRT1_2;
                } else {
                    matrix[FRONT_LEFT ][BACK_CENTER] -= surround_mix_level;
                    matrix[FRONT_RIGHT][BACK_CENTER] += surround_mix_level;
                }
            } else {
                matrix[FRONT_LEFT ][BACK_CENTER] += surround_mix_level * M_SQRT1_2;
                matrix[FRONT_RIGHT][BACK_CENTER] += surround_mix_level * M_SQRT1_2;
            }
        } else if (out_layout & AV_CH_FRONT_CENTER) {
            matrix[FRONT_CENTER][BACK_CENTER] += surround_mix_level * M_SQRT1_2;
        } else
            return AVERROR_PATCHWELCOME;
    }
    /* mix back left/right to back center, side, or front */
    if (unaccounted & AV_CH_BACK_LEFT) {
        if (out_layout & AV_CH_BACK_CENTER) {
            matrix[BACK_CENTER][BACK_LEFT ] += M_SQRT1_2;
            matrix[BACK_CENTER][BACK_RIGHT] += M_SQRT1_2;
        } else if (out_layout & AV_CH_SIDE_LEFT) {
            /* if side channels do not exist in the input, just copy back
               channels to side channels, otherwise mix back into side */
            if (in_layout & AV_CH_SIDE_LEFT) {
                matrix[SIDE_LEFT ][BACK_LEFT ] += M_SQRT1_2;
                matrix[SIDE_RIGHT][BACK_RIGHT] += M_SQRT1_2;
            } else {
                matrix[SIDE_LEFT ][BACK_LEFT ] += 1.0;
                matrix[SIDE_RIGHT][BACK_RIGHT] += 1.0;
            }
        } else if (out_layout & AV_CH_FRONT_LEFT) {
            if (matrix_encoding == AV_MATRIX_ENCODING_DOLBY) {
                matrix[FRONT_LEFT ][BACK_LEFT ] -= surround_mix_level * M_SQRT1_2;
                matrix[FRONT_LEFT ][BACK_RIGHT] -= surround_mix_level * M_SQRT1_2;
                matrix[FRONT_RIGHT][BACK_LEFT ] += surround_mix_level * M_SQRT1_2;
                matrix[FRONT_RIGHT][BACK_RIGHT] += surround_mix_level * M_SQRT1_2;
            } else if (matrix_encoding == AV_MATRIX_ENCODING_DPLII) {
                matrix[FRONT_LEFT ][BACK_LEFT ] -= surround_mix_level * SQRT3_2;
                matrix[FRONT_LEFT ][BACK_RIGHT] -= surround_mix_level * M_SQRT1_2;
                matrix[FRONT_RIGHT][BACK_LEFT ] += surround_mix_level * M_SQRT1_2;
                matrix[FRONT_RIGHT][BACK_RIGHT] += surround_mix_level * SQRT3_2;
            } else {
                matrix[FRONT_LEFT ][BACK_LEFT ] += surround_mix_level;
                matrix[FRONT_RIGHT][BACK_RIGHT] += surround_mix_level;
            }
        } else if (out_layout & AV_CH_FRONT_CENTER) {
            matrix[FRONT_CENTER][BACK_LEFT ] += surround_mix_level * M_SQRT1_2;
            matrix[FRONT_CENTER][BACK_RIGHT] += surround_mix_level * M_SQRT1_2;
        } else
            return AVERROR_PATCHWELCOME;
    }
    /* mix side left/right into back or front */
    if (unaccounted & AV_CH_SIDE_LEFT) {
        if (out_layout & AV_CH_BACK_LEFT) {
            /* if back channels do not exist in the input, just copy side
               channels to back channels, otherwise mix side into back */
            if (in_layout & AV_CH_BACK_LEFT) {
                matrix[BACK_LEFT ][SIDE_LEFT ] += M_SQRT1_2;
                matrix[BACK_RIGHT][SIDE_RIGHT] += M_SQRT1_2;
            } else {
                matrix[BACK_LEFT ][SIDE_LEFT ] += 1.0;
                matrix[BACK_RIGHT][SIDE_RIGHT] += 1.0;
            }
        } else if (out_layout & AV_CH_BACK_CENTER) {
            matrix[BACK_CENTER][SIDE_LEFT ] += M_SQRT1_2;
            matrix[BACK_CENTER][SIDE_RIGHT] += M_SQRT1_2;
        } else if (out_layout & AV_CH_FRONT_LEFT) {
            if (matrix_encoding == AV_MATRIX_ENCODING_DOLBY) {
                matrix[FRONT_LEFT ][SIDE_LEFT ] -= surround_mix_level * M_SQRT1_2;
                matrix[FRONT_LEFT ][SIDE_RIGHT] -= surround_mix_level * M_SQRT1_2;
                matrix[FRONT_RIGHT][SIDE_LEFT ] += surround_mix_level * M_SQRT1_2;
                matrix[FRONT_RIGHT][SIDE_RIGHT] += surround_mix_level * M_SQRT1_2;
            } else if (matrix_encoding == AV_MATRIX_ENCODING_DPLII) {
                matrix[FRONT_LEFT ][SIDE_LEFT ] -= surround_mix_level * SQRT3_2;
                matrix[FRONT_LEFT ][SIDE_RIGHT] -= surround_mix_level * M_SQRT1_2;
                matrix[FRONT_RIGHT][SIDE_LEFT ] += surround_mix_level * M_SQRT1_2;
                matrix[FRONT_RIGHT][SIDE_RIGHT] += surround_mix_level * SQRT3_2;
            } else {
                matrix[FRONT_LEFT ][SIDE_LEFT ] += surround_mix_level;
                matrix[FRONT_RIGHT][SIDE_RIGHT] += surround_mix_level;
            }
        } else if (out_layout & AV_CH_FRONT_CENTER) {
            matrix[FRONT_CENTER][SIDE_LEFT ] += surround_mix_level * M_SQRT1_2;
            matrix[FRONT_CENTER][SIDE_RIGHT] += surround_mix_level * M_SQRT1_2;
        } else
            return AVERROR_PATCHWELCOME;
    }
    /* mix left-of-center/right-of-center into front left/right or center */
    if (unaccounted & AV_CH_FRONT_LEFT_OF_CENTER) {
        if (out_layout & AV_CH_FRONT_LEFT) {
            matrix[FRONT_LEFT ][FRONT_LEFT_OF_CENTER ] += 1.0;
            matrix[FRONT_RIGHT][FRONT_RIGHT_OF_CENTER] += 1.0;
        } else if (out_layout & AV_CH_FRONT_CENTER) {
            matrix[FRONT_CENTER][FRONT_LEFT_OF_CENTER ] += M_SQRT1_2;
            matrix[FRONT_CENTER][FRONT_RIGHT_OF_CENTER] += M_SQRT1_2;
        } else
            return AVERROR_PATCHWELCOME;
    }
    /* mix LFE into front left/right or center */
    if (unaccounted & AV_CH_LOW_FREQUENCY) {
        if (out_layout & AV_CH_FRONT_CENTER) {
            matrix[FRONT_CENTER][LOW_FREQUENCY] += lfe_mix_level;
        } else if (out_layout & AV_CH_FRONT_LEFT) {
            matrix[FRONT_LEFT ][LOW_FREQUENCY] += lfe_mix_level * M_SQRT1_2;
            matrix[FRONT_RIGHT][LOW_FREQUENCY] += lfe_mix_level * M_SQRT1_2;
        } else
            return AVERROR_PATCHWELCOME;
    }

    /* transfer internal matrix to output matrix and calculate maximum
       per-channel coefficient sum */
    for (out_i = i = 0; out_i < out_channels && i < 64; i++) {
        double sum = 0;
        for (out_j = j = 0; out_j < in_channels && j < 64; j++) {
            matrix_out[out_i * stride + out_j] = matrix[i][j];
            sum += fabs(matrix[i][j]);
            if (in_layout & (1ULL << j))
                out_j++;
        }
        maxcoef = FFMAX(maxcoef, sum);
        if (out_layout & (1ULL << i))
            out_i++;
    }

    /* normalize */
    if (normalize && maxcoef > 1.0) {
        for (i = 0; i < out_channels; i++)
            for (j = 0; j < in_channels; j++)
                matrix_out[i * stride + j] /= maxcoef;
    }

    return 0;
}


AudioMix *ff_audio_mix_alloc(AVAudioResampleContext *avr)
{
    AudioMix *am;
    int ret;

    am = av_mallocz(sizeof(*am));
    if (!am)
        return NULL;
    am->avr = avr;

    if (avr->internal_sample_fmt != AV_SAMPLE_FMT_S16P &&
        avr->internal_sample_fmt != AV_SAMPLE_FMT_FLTP) {
        av_log(avr, AV_LOG_ERROR, "Unsupported internal format for "
               "mixing: %s\n",
               av_get_sample_fmt_name(avr->internal_sample_fmt));
        goto error;
    }

    am->fmt          = avr->internal_sample_fmt;
    am->coeff_type   = avr->mix_coeff_type;
    am->in_layout    = avr->in_channel_layout;
    am->out_layout   = avr->out_channel_layout;
    am->in_channels  = avr->in_channels;
    am->out_channels = avr->out_channels;

    /* build matrix if the user did not already set one */
    if (avr->mix_matrix) {
        ret = ff_audio_mix_set_matrix(am, avr->mix_matrix, avr->in_channels);
        if (ret < 0)
            goto error;
        av_freep(&avr->mix_matrix);
    } else {
        double *matrix_dbl = av_mallocz(avr->out_channels * avr->in_channels *
                                        sizeof(*matrix_dbl));
        if (!matrix_dbl)
            goto error;

        ret = avresample_build_matrix(avr->in_channel_layout,
                                      avr->out_channel_layout,
                                      avr->center_mix_level,
                                      avr->surround_mix_level,
                                      avr->lfe_mix_level,
                                      avr->normalize_mix_level,
                                      matrix_dbl,
                                      avr->in_channels,
                                      avr->matrix_encoding);
        if (ret < 0) {
            av_free(matrix_dbl);
            goto error;
        }

        ret = ff_audio_mix_set_matrix(am, matrix_dbl, avr->in_channels);
        if (ret < 0) {
            av_log(avr, AV_LOG_ERROR, "error setting mix matrix\n");
            av_free(matrix_dbl);
            goto error;
        }

        av_free(matrix_dbl);
    }

    return am;

error:
    av_free(am);
    return NULL;
}

void ff_audio_convert_free(AudioConvert **ac);
void ff_dither_free(DitherContext **cp)
{
    DitherContext *c = *cp;
    int ch;

    if (!c)
        return;
    ff_audio_data_free(&c->flt_data);
    ff_audio_data_free(&c->s16_data);
    ff_audio_convert_free(&c->ac_in);
    ff_audio_convert_free(&c->ac_out);
    for (ch = 0; ch < c->channels; ch++)
        av_free(c->state[ch].noise_buf);
    av_free(c->state);
    av_freep(cp);
}
void ff_audio_convert_free(AudioConvert **ac)
{
    if (!*ac)
        return;
    ff_dither_free(&(*ac)->dc);
    av_freep(ac);
}

void ff_audio_resample_free(ResampleContext **c)
{
    if (!*c)
        return;
    ff_audio_data_free(&(*c)->buffer);
    av_free((*c)->filter_bank);
    av_freep(c);
}

void ff_audio_mix_free(AudioMix **am_p)
{
    AudioMix *am;

    if (!*am_p)
        return;
    am = *am_p;

    if (am->matrix) {
        av_free(am->matrix[0]);
        am->matrix = NULL;
    }
    memset(am->matrix_q8,  0, sizeof(am->matrix_q8 ));
    memset(am->matrix_q15, 0, sizeof(am->matrix_q15));
    memset(am->matrix_flt, 0, sizeof(am->matrix_flt));

    av_freep(am_p);
}
void avresample_close(AVAudioResampleContext *avr)
{
    ff_audio_data_free(&avr->in_buffer);
    ff_audio_data_free(&avr->resample_out_buffer);
    ff_audio_data_free(&avr->out_buffer);
    av_audio_fifo_free(avr->out_fifo);
    avr->out_fifo = NULL;
    ff_audio_convert_free(&avr->ac_in);
    ff_audio_convert_free(&avr->ac_out);
    ff_audio_resample_free(&avr->resample);
    ff_audio_mix_free(&avr->am);
    av_freep(&avr->mix_matrix);

    avr->use_channel_map = 0;
}
int avresample_open(AVAudioResampleContext *avr)
{
    int ret;

    if (avresample_is_open(avr)) {
        av_log(avr, AV_LOG_ERROR, "The resampling context is already open.\n");
        return AVERROR(EINVAL);
    }

    /* set channel mixing parameters */
    avr->in_channels = av_get_channel_layout_nb_channels(avr->in_channel_layout);
    if (avr->in_channels <= 0 || avr->in_channels > AVRESAMPLE_MAX_CHANNELS) {
        av_log(avr, AV_LOG_ERROR, "Invalid input channel layout: %"PRIu64"\n",
               avr->in_channel_layout);
        return AVERROR(EINVAL);
    }
    avr->out_channels = av_get_channel_layout_nb_channels(avr->out_channel_layout);
    if (avr->out_channels <= 0 || avr->out_channels > AVRESAMPLE_MAX_CHANNELS) {
        av_log(avr, AV_LOG_ERROR, "Invalid output channel layout: %"PRIu64"\n",
               avr->out_channel_layout);
        return AVERROR(EINVAL);
    }
    avr->resample_channels = FFMIN(avr->in_channels, avr->out_channels);
    avr->downmix_needed    = avr->in_channels  > avr->out_channels;
    avr->upmix_needed      = avr->out_channels > avr->in_channels ||
                             (!avr->downmix_needed && (avr->mix_matrix ||
                              avr->in_channel_layout != avr->out_channel_layout));
    avr->mixing_needed     = avr->downmix_needed || avr->upmix_needed;

    /* set resampling parameters */
    avr->resample_needed   = avr->in_sample_rate != avr->out_sample_rate ||
                             avr->force_resampling;

    /* select internal sample format if not specified by the user */
    if (avr->internal_sample_fmt == AV_SAMPLE_FMT_NONE &&
        (avr->mixing_needed || avr->resample_needed)) {
        enum AVSampleFormat  in_fmt = av_get_planar_sample_fmt(avr->in_sample_fmt);
        enum AVSampleFormat out_fmt = av_get_planar_sample_fmt(avr->out_sample_fmt);
        int max_bps = FFMAX(av_get_bytes_per_sample(in_fmt),
                            av_get_bytes_per_sample(out_fmt));
        if (max_bps <= 2) {
            avr->internal_sample_fmt = AV_SAMPLE_FMT_S16P;
        } else if (avr->mixing_needed) {
            avr->internal_sample_fmt = AV_SAMPLE_FMT_FLTP;
        } else {
            if (max_bps <= 4) {
                if (in_fmt  == AV_SAMPLE_FMT_S32P ||
                    out_fmt == AV_SAMPLE_FMT_S32P) {
                    if (in_fmt  == AV_SAMPLE_FMT_FLTP ||
                        out_fmt == AV_SAMPLE_FMT_FLTP) {
                        /* if one is s32 and the other is flt, use dbl */
                        avr->internal_sample_fmt = AV_SAMPLE_FMT_DBLP;
                    } else {
                        /* if one is s32 and the other is s32, s16, or u8, use s32 */
                        avr->internal_sample_fmt = AV_SAMPLE_FMT_S32P;
                    }
                } else {
                    /* if one is flt and the other is flt, s16 or u8, use flt */
                    avr->internal_sample_fmt = AV_SAMPLE_FMT_FLTP;
                }
            } else {
                /* if either is dbl, use dbl */
                avr->internal_sample_fmt = AV_SAMPLE_FMT_DBLP;
            }
        }
        av_log(avr, AV_LOG_DEBUG, "Using %s as internal sample format\n",
               av_get_sample_fmt_name(avr->internal_sample_fmt));
    }

    /* we may need to add an extra conversion in order to remap channels if
       the output format is not planar */
    if (avr->use_channel_map && !avr->mixing_needed && !avr->resample_needed &&
        !ff_sample_fmt_is_planar(avr->out_sample_fmt, avr->out_channels)) {
        avr->internal_sample_fmt = av_get_planar_sample_fmt(avr->out_sample_fmt);
    }

    /* set sample format conversion parameters */
    if (avr->resample_needed || avr->mixing_needed)
        avr->in_convert_needed = avr->in_sample_fmt != avr->internal_sample_fmt;
    else
        avr->in_convert_needed = avr->use_channel_map &&
                                 !ff_sample_fmt_is_planar(avr->out_sample_fmt, avr->out_channels);

    if (avr->resample_needed || avr->mixing_needed || avr->in_convert_needed)
        avr->out_convert_needed = avr->internal_sample_fmt != avr->out_sample_fmt;
    else
        avr->out_convert_needed = avr->in_sample_fmt != avr->out_sample_fmt;

    avr->in_copy_needed = !avr->in_convert_needed && (avr->mixing_needed ||
                          (avr->use_channel_map && avr->resample_needed));

    if (avr->use_channel_map) {
        if (avr->in_copy_needed) {
            avr->remap_point = REMAP_IN_COPY;
            av_log(avr, AV_LOG_TRACE, "remap channels during in_copy\n");
        } else if (avr->in_convert_needed) {
            avr->remap_point = REMAP_IN_CONVERT;
            av_log(avr, AV_LOG_TRACE, "remap channels during in_convert\n");
        } else if (avr->out_convert_needed) {
            avr->remap_point = REMAP_OUT_CONVERT;
            av_log(avr, AV_LOG_TRACE, "remap channels during out_convert\n");
        } else {
            avr->remap_point = REMAP_OUT_COPY;
            av_log(avr, AV_LOG_TRACE, "remap channels during out_copy\n");
        }


    } else
        avr->remap_point = REMAP_NONE;

    /* allocate buffers */
    if (avr->in_copy_needed || avr->in_convert_needed) {
        avr->in_buffer = ff_audio_data_alloc(FFMAX(avr->in_channels, avr->out_channels),
                                             0, avr->internal_sample_fmt,
                                             "in_buffer");
        if (!avr->in_buffer) {
            ret = AVERROR(EINVAL);
            goto error;
        }
    }
    if (avr->resample_needed) {
        avr->resample_out_buffer = ff_audio_data_alloc(avr->out_channels,
                                                       1024, avr->internal_sample_fmt,
                                                       "resample_out_buffer");
        if (!avr->resample_out_buffer) {
            ret = AVERROR(EINVAL);
            goto error;
        }
    }
    if (avr->out_convert_needed) {
        avr->out_buffer = ff_audio_data_alloc(avr->out_channels, 0,
                                              avr->out_sample_fmt, "out_buffer");
        if (!avr->out_buffer) {
            ret = AVERROR(EINVAL);
            goto error;
        }
    }
    avr->out_fifo = av_audio_fifo_alloc(avr->out_sample_fmt, avr->out_channels,
                                        1024);
    if (!avr->out_fifo) {
        ret = AVERROR(ENOMEM);
        goto error;
    }

    /* setup contexts */
    if (avr->in_convert_needed) {
        avr->ac_in = ff_audio_convert_alloc(avr, avr->internal_sample_fmt,
                                            avr->in_sample_fmt, avr->in_channels,
                                            avr->in_sample_rate,
                                            avr->remap_point == REMAP_IN_CONVERT);
        if (!avr->ac_in) {
            ret = AVERROR(ENOMEM);
            goto error;
        }
    }
    if (avr->out_convert_needed) {
        enum AVSampleFormat src_fmt;
        if (avr->in_convert_needed)
            src_fmt = avr->internal_sample_fmt;
        else
            src_fmt = avr->in_sample_fmt;
        avr->ac_out = ff_audio_convert_alloc(avr, avr->out_sample_fmt, src_fmt,
                                             avr->out_channels,
                                             avr->out_sample_rate,
                                             avr->remap_point == REMAP_OUT_CONVERT);
        if (!avr->ac_out) {
            ret = AVERROR(ENOMEM);
            goto error;
        }
    }
    if (avr->resample_needed) {
        avr->resample = ff_audio_resample_init(avr);
        if (!avr->resample) {
            ret = AVERROR(ENOMEM);
            goto error;
        }
    }
    if (avr->mixing_needed) {
        avr->am = ff_audio_mix_alloc(avr);
        if (!avr->am) {
            ret = AVERROR(ENOMEM);
            goto error;
        }
    }

    return 0;

error:
    avresample_close(avr);
    return ret;
}





void avresample_free(AVAudioResampleContext **avr)
{
    if (!*avr)
        return;
    avresample_close(*avr);
    av_opt_free(*avr);
    av_freep(avr);
}

static int handle_buffered_output(AVAudioResampleContext *avr,
                                  AudioData *output, AudioData *converted)
{
    int ret;

    if (!output || av_audio_fifo_size(avr->out_fifo) > 0 ||
        (converted && output->allocated_samples < converted->nb_samples)) {
        if (converted) {
            /* if there are any samples in the output FIFO or if the
               user-supplied output buffer is not large enough for all samples,
               we add to the output FIFO */
            av_log(avr, AV_LOG_TRACE, "[FIFO] add %s to out_fifo\n", converted->name);
            ret = ff_audio_data_add_to_fifo(avr->out_fifo, converted, 0,
                                            converted->nb_samples);
            if (ret < 0)
                return ret;
        }

        /* if the user specified an output buffer, read samples from the output
           FIFO to the user output */
        if (output && output->allocated_samples > 0) {
            av_log(avr, AV_LOG_TRACE, "[FIFO] read from out_fifo to output\n");
            av_log(avr, AV_LOG_TRACE, "[end conversion]\n");
            return ff_audio_data_read_from_fifo(avr->out_fifo, output,
                                                output->allocated_samples);
        }
    } else if (converted) {
        /* copy directly to output if it is large enough or there is not any
           data in the output FIFO */
        av_log(avr, AV_LOG_TRACE, "[copy] %s to output\n", converted->name);
        output->nb_samples = 0;
        ret = ff_audio_data_copy(output, converted,
                                 avr->remap_point == REMAP_OUT_COPY ?
                                 &avr->ch_map_info : NULL);
        if (ret < 0)
            return ret;
        av_log(avr, AV_LOG_TRACE, "[end conversion]\n");
        return output->nb_samples;
    }
    av_log(avr, AV_LOG_TRACE, "[end conversion]\n");
    return 0;
}

int ff_audio_convert(AudioConvert *ac, AudioData *out, AudioData *in);




int swr_is_initialized(struct SwrContext *s) {
    return !!s->in_buffer.ch_count;
}
#define ALIGN 32
int swri_realloc_audio(SWresampleAudioData *a, int count){
    int i, countb;
    SWresampleAudioData old;

    if(count < 0 || count > INT_MAX/2/a->bps/a->ch_count)
        return AVERROR(EINVAL);

    if(a->count >= count)
        return 0;

    count*=2;

    countb= FFALIGN(count*a->bps, ALIGN);
    old= *a;

    av_assert0(a->bps);
    av_assert0(a->ch_count);

    a->data= av_mallocz_array(countb, a->ch_count);
    if(!a->data)
        return AVERROR(ENOMEM);
    for(i=0; i<a->ch_count; i++){
        a->ch[i]= a->data + i*(a->planar ? countb : a->bps);
        if(a->count && a->planar) memcpy(a->ch[i], old.ch[i], a->count*a->bps);
    }
    if(a->count && !a->planar) memcpy(a->ch[0], old.ch[0], a->count*a->ch_count*a->bps);
    av_freep(&old.data);
    a->count= count;

    return 1;
}


static void reversefill_audiodata(SWresampleAudioData *out, uint8_t *in_arg [SWR_CH_MAX]){
    int i;
    if(out->planar){
        for(i=0; i<out->ch_count; i++)
            in_arg[i]= out->ch[i];
    }else{
        in_arg[0]= out->ch[0];
    }
}

static void fill_audiodata(SWresampleAudioData *out, uint8_t *in_arg [SWR_CH_MAX]){
    int i;
    if(!in_arg){
        memset(out->ch, 0, sizeof(out->ch));
    }else if(out->planar){
        for(i=0; i<out->ch_count; i++)
            out->ch[i]= in_arg[i];
    }else{
        for(i=0; i<out->ch_count; i++)
            out->ch[i]= in_arg[0] + i*out->bps;
    }
}
int swri_audio_convert(SWresampleAudioConvert *ctx, SWresampleAudioData *out, SWresampleAudioData *in, int len)
{
    int ch;
    int off=0;
    const int os= (out->planar ? 1 :out->ch_count) *out->bps;
    unsigned misaligned = 0;

    av_assert0(ctx->channels == out->ch_count);

    if (ctx->in_simd_align_mask) {
        int planes = in->planar ? in->ch_count : 1;
        unsigned m = 0;
        for (ch = 0; ch < planes; ch++)
            m |= (intptr_t)in->ch[ch];
        misaligned |= m & ctx->in_simd_align_mask;
    }
    if (ctx->out_simd_align_mask) {
        int planes = out->planar ? out->ch_count : 1;
        unsigned m = 0;
        for (ch = 0; ch < planes; ch++)
            m |= (intptr_t)out->ch[ch];
        misaligned |= m & ctx->out_simd_align_mask;
    }

    //FIXME optimize common cases

    if(ctx->simd_f && !ctx->ch_map && !misaligned){
        off = len&~15;
        av_assert1(off>=0);
        av_assert1(off<=len);
        av_assert2(ctx->channels == SWR_CH_MAX || !in->ch[ctx->channels]);
        if(off>0){
            if(out->planar == in->planar){
                int planes = out->planar ? out->ch_count : 1;
                for(ch=0; ch<planes; ch++){
                    ctx->simd_f(out->ch+ch, (const uint8_t **)in->ch+ch, off * (out->planar ? 1 :out->ch_count));
                }
            }else{
                ctx->simd_f(out->ch, (const uint8_t **)in->ch, off);
            }
        }
        if(off == len)
            return 0;
    }

    for(ch=0; ch<ctx->channels; ch++){
        const int ich= ctx->ch_map ? ctx->ch_map[ch] : ch;
        const int is= ich < 0 ? 0 : (in->planar ? 1 : in->ch_count) * in->bps;
        const uint8_t *pi= ich < 0 ? ctx->silence : in->ch[ich];
        uint8_t       *po= out->ch[ch];
        uint8_t *end= po + os*len;
        if(!po)
            continue;
        ctx->conv_f(po+off*os, pi+off*is, is, os, end);
    }
    return 0;
}

static void copy(SWresampleAudioData *out, SWresampleAudioData *in,int count){
    av_assert0(out->planar == in->planar);
    av_assert0(out->bps == in->bps);
    av_assert0(out->ch_count == in->ch_count);
    if(out->planar){
        int ch;
        for(ch=0; ch<out->ch_count; ch++)
            memcpy(out->ch[ch], in->ch[ch], count*out->bps);
    }else
        memcpy(out->ch[0], in->ch[0], count*out->ch_count*out->bps);
}


static void buf_set(SWresampleAudioData *out, SWresampleAudioData *in, int count){
    int ch;
    if(in->planar){
        for(ch=0; ch<out->ch_count; ch++)
            out->ch[ch]= in->ch[ch] + count*out->bps;
    }else{
        for(ch=out->ch_count-1; ch>=0; ch--)
            out->ch[ch]= in->ch[0] + (ch + count*out->ch_count) * out->bps;
    }
}

static int swresample(SwrContext *s, SWresampleAudioData *out_param, int out_count,const SWresampleAudioData * in_param, int in_count){
    SWresampleAudioData in, out, tmp;
    int ret_sum=0;
    int border=0;
    int padless = ARCH_X86 && s->engine == SWR_ENGINE_SWR ? 7 : 0;

    av_assert1(s->in_buffer.ch_count == in_param->ch_count);
    av_assert1(s->in_buffer.planar   == in_param->planar);
    av_assert1(s->in_buffer.fmt      == in_param->fmt);

    tmp=out=*out_param;
    in =  *in_param;

    border = s->resampler->invert_initial_buffer(s->resample, &s->in_buffer, &in, in_count, &s->in_buffer_index, &s->in_buffer_count);
    if (border == INT_MAX) {
        return 0;
    } else if (border < 0) {
        return border;
    } else if (border) {
        buf_set(&in, &in, border);
        in_count -= border;
        s->resample_in_constraint = 0;
    }

    do{
        int ret, size, consumed;
        if(!s->resample_in_constraint && s->in_buffer_count){
            buf_set(&tmp, &s->in_buffer, s->in_buffer_index);
            ret= s->resampler->multiple_resample(s->resample, &out, out_count, &tmp, s->in_buffer_count, &consumed);
            out_count -= ret;
            ret_sum += ret;
            buf_set(&out, &out, ret);
            s->in_buffer_count -= consumed;
            s->in_buffer_index += consumed;

            if(!in_count)
                break;
            if(s->in_buffer_count <= border){
                buf_set(&in, &in, -s->in_buffer_count);
                in_count += s->in_buffer_count;
                s->in_buffer_count=0;
                s->in_buffer_index=0;
                border = 0;
            }
        }

        if((s->flushed || in_count > padless) && !s->in_buffer_count){
            s->in_buffer_index=0;
            ret= s->resampler->multiple_resample(s->resample, &out, out_count, &in, FFMAX(in_count-padless, 0), &consumed);
            out_count -= ret;
            ret_sum += ret;
            buf_set(&out, &out, ret);
            in_count -= consumed;
            buf_set(&in, &in, consumed);
        }

        //TODO is this check sane considering the advanced copy avoidance below
        size= s->in_buffer_index + s->in_buffer_count + in_count;
        if(   size > s->in_buffer.count
           && s->in_buffer_count + in_count <= s->in_buffer_index){
            buf_set(&tmp, &s->in_buffer, s->in_buffer_index);
            copy(&s->in_buffer, &tmp, s->in_buffer_count);
            s->in_buffer_index=0;
        }else
            if((ret=swri_realloc_audio(&s->in_buffer, size)) < 0)
                return ret;

        if(in_count){
            int count= in_count;
            if(s->in_buffer_count && s->in_buffer_count+2 < count && out_count) count= s->in_buffer_count+2;

            buf_set(&tmp, &s->in_buffer, s->in_buffer_index + s->in_buffer_count);
            copy(&tmp, &in, /*in_*/count);
            s->in_buffer_count += count;
            in_count -= count;
            border += count;
            buf_set(&in, &in, count);
            s->resample_in_constraint= 0;
            if(s->in_buffer_count != count || in_count)
                continue;
            if (padless) {
                padless = 0;
                continue;
            }
        }
        break;
    }while(1);

    s->resample_in_constraint= !!out_count;

    return ret_sum;
}


static int resample(ResampleContext *c, void *dst, const void *src,
                    int *consumed, int src_size, int dst_size, int update_ctx,
                    int nearest_neighbour)
{
    int dst_index;
    unsigned int index = c->index;
    int frac          = c->frac;
    int dst_incr_frac = c->dst_incr % c->src_incr;
    int dst_incr      = c->dst_incr / c->src_incr;
    int compensation_distance = c->compensation_distance;

    if (!dst != !src)
        return AVERROR(EINVAL);

    if (nearest_neighbour) {
        uint64_t index2 = ((uint64_t)index) << 32;
        int64_t incr   = (1LL << 32) * c->dst_incr / c->src_incr;
        dst_size       = FFMIN(dst_size,
                               (src_size-1-index) * (int64_t)c->src_incr /
                               c->dst_incr);

        if (dst) {
            for(dst_index = 0; dst_index < dst_size; dst_index++) {
                c->resample_nearest(dst, dst_index, src, index2 >> 32);
                index2 += incr;
            }
        } else {
            dst_index = dst_size;
        }
        index += dst_index * dst_incr;
        index += (frac + dst_index * (int64_t)dst_incr_frac) / c->src_incr;
        frac   = (frac + dst_index * (int64_t)dst_incr_frac) % c->src_incr;
    } else {
        for (dst_index = 0; dst_index < dst_size; dst_index++) {
            int sample_index = index >> c->phase_shift;

            if (sample_index + c->filter_length > src_size)
                break;

            if (dst)
                c->resample_one(c, dst, dst_index, src, index, frac);

            frac  += dst_incr_frac;
            index += dst_incr;
            if (frac >= c->src_incr) {
                frac -= c->src_incr;
                index++;
            }
            if (dst_index + 1 == compensation_distance) {
                compensation_distance = 0;
                dst_incr_frac = c->ideal_dst_incr % c->src_incr;
                dst_incr      = c->ideal_dst_incr / c->src_incr;
            }
        }
    }
    if (consumed)
        *consumed = index >> c->phase_shift;

    if (update_ctx) {
        index &= c->phase_mask;

        if (compensation_distance) {
            compensation_distance -= dst_index;
            if (compensation_distance <= 0)
                return AVERROR_BUG;
        }
        c->frac     = frac;
        c->index    = index;
        c->dst_incr = dst_incr_frac + c->src_incr*dst_incr;
        c->compensation_distance = compensation_distance;
    }

    return dst_index;
}


int swri_rematrix(SwrContext *s, SWresampleAudioData *out, SWresampleAudioData *in, int len, int mustcopy){
    int out_i, in_i, i, j;
    int len1 = 0;
    int off = 0;

    if(s->mix_any_f) {
        s->mix_any_f(out->ch, (const uint8_t **)in->ch, s->native_matrix, len);
        return 0;
    }

    if(s->mix_2_1_simd || s->mix_1_1_simd){
        len1= len&~15;
        off = len1 * out->bps;
    }

    av_assert0(!s->out_ch_layout || out->ch_count == av_get_channel_layout_nb_channels(s->out_ch_layout));
    av_assert0(!s-> in_ch_layout || in ->ch_count == av_get_channel_layout_nb_channels(s-> in_ch_layout));

    for(out_i=0; out_i<out->ch_count; out_i++){
        switch(s->matrix_ch[out_i][0]){
        case 0:
            if(mustcopy)
                memset(out->ch[out_i], 0, len * av_get_bytes_per_sample(s->int_sample_fmt));
            break;
        case 1:
            in_i= s->matrix_ch[out_i][1];
            if(s->matrix[out_i][in_i]!=1.0){
                if(s->mix_1_1_simd && len1)
                    s->mix_1_1_simd(out->ch[out_i]    , in->ch[in_i]    , s->native_simd_matrix, in->ch_count*out_i + in_i, len1);
                if(len != len1)
                    s->mix_1_1_f   (out->ch[out_i]+off, in->ch[in_i]+off, s->native_matrix, in->ch_count*out_i + in_i, len-len1);
            }else if(mustcopy){
                memcpy(out->ch[out_i], in->ch[in_i], len*out->bps);
            }else{
                out->ch[out_i]= in->ch[in_i];
            }
            break;
        case 2: {
            int in_i1 = s->matrix_ch[out_i][1];
            int in_i2 = s->matrix_ch[out_i][2];
            if(s->mix_2_1_simd && len1)
                s->mix_2_1_simd(out->ch[out_i]    , in->ch[in_i1]    , in->ch[in_i2]    , s->native_simd_matrix, in->ch_count*out_i + in_i1, in->ch_count*out_i + in_i2, len1);
            else
                s->mix_2_1_f   (out->ch[out_i]    , in->ch[in_i1]    , in->ch[in_i2]    , s->native_matrix, in->ch_count*out_i + in_i1, in->ch_count*out_i + in_i2, len1);
            if(len != len1)
                s->mix_2_1_f   (out->ch[out_i]+off, in->ch[in_i1]+off, in->ch[in_i2]+off, s->native_matrix, in->ch_count*out_i + in_i1, in->ch_count*out_i + in_i2, len-len1);
            break;}
        default:
            if(s->int_sample_fmt == AV_SAMPLE_FMT_FLTP){
                for(i=0; i<len; i++){
                    float v=0;
                    for(j=0; j<s->matrix_ch[out_i][0]; j++){
                        in_i= s->matrix_ch[out_i][1+j];
                        v+= ((float*)in->ch[in_i])[i] * s->matrix_flt[out_i][in_i];
                    }
                    ((float*)out->ch[out_i])[i]= v;
                }
            }else if(s->int_sample_fmt == AV_SAMPLE_FMT_DBLP){
                for(i=0; i<len; i++){
                    double v=0;
                    for(j=0; j<s->matrix_ch[out_i][0]; j++){
                        in_i= s->matrix_ch[out_i][1+j];
                        v+= ((double*)in->ch[in_i])[i] * s->matrix[out_i][in_i];
                    }
                    ((double*)out->ch[out_i])[i]= v;
                }
            }else{
                for(i=0; i<len; i++){
                    int v=0;
                    for(j=0; j<s->matrix_ch[out_i][0]; j++){
                        in_i= s->matrix_ch[out_i][1+j];
                        v+= ((int16_t*)in->ch[in_i])[i] * s->matrix32[out_i][in_i];
                    }
                    ((int16_t*)out->ch[out_i])[i]= (v + 16384)>>15;
                }
            }
        }
    }
    return 0;
}


int swri_get_dither(SwrContext *s, void *dst, int len, unsigned seed, enum AVSampleFormat noise_fmt) {
    double scale = s->dither.noise_scale;
#define TMP_EXTRA 2
    double *tmp = av_malloc_array(len + TMP_EXTRA, sizeof(double));
    int i;

    if (!tmp)
        return AVERROR(ENOMEM);

    for(i=0; i<len + TMP_EXTRA; i++){
        double v;
        seed = seed* 1664525 + 1013904223;

        switch(s->dither.method){
            case SWR_DITHER_RECTANGULAR: v= ((double)seed) / UINT_MAX - 0.5; break;
            default:
                av_assert0(s->dither.method < SWR_DITHER_NB);
                v = ((double)seed) / UINT_MAX;
                seed = seed*1664525 + 1013904223;
                v-= ((double)seed) / UINT_MAX;
                break;
        }
        tmp[i] = v;
    }

    for(i=0; i<len; i++){
        double v;

        switch(s->dither.method){
            default:
                av_assert0(s->dither.method < SWR_DITHER_NB);
                v = tmp[i];
                break;
            case SWR_DITHER_TRIANGULAR_HIGHPASS :
                v = (- tmp[i] + 2*tmp[i+1] - tmp[i+2]) / sqrt(6);
                break;
        }

        v*= scale;

        switch(noise_fmt){
            case AV_SAMPLE_FMT_S16P: ((int16_t*)dst)[i] = v; break;
            case AV_SAMPLE_FMT_S32P: ((int32_t*)dst)[i] = v; break;
            case AV_SAMPLE_FMT_FLTP: ((float  *)dst)[i] = v; break;
            case AV_SAMPLE_FMT_DBLP: ((double *)dst)[i] = v; break;
            default: av_assert0(0);
        }
    }

    av_free(tmp);
    return 0;
}

static int swr_convert_internal(struct SwrContext *s, SWresampleAudioData *out, int out_count,SWresampleAudioData *in , int  in_count){
    SWresampleAudioData *postin, *midbuf, *preout;
    int ret/*, in_max*/;
    SWresampleAudioData preout_tmp, midbuf_tmp;

    if(s->full_convert){
        av_assert0(!s->resample);
        swri_audio_convert(s->full_convert, out, in, in_count);
        return out_count;
    }

    // in_max= out_count*(int64_t)s->in_sample_rate / s->out_sample_rate + resample_filter_taps;
    // in_count= FFMIN(in_count, in_in + 2 - s->hist_buffer_count);

    if((ret=swri_realloc_audio(&s->postin, in_count))<0)
        return ret;
    if(s->resample_first){
        av_assert0(s->midbuf.ch_count == s->used_ch_count);
        if((ret=swri_realloc_audio(&s->midbuf, out_count))<0)
            return ret;
    }else{
        av_assert0(s->midbuf.ch_count ==  s->out.ch_count);
        if((ret=swri_realloc_audio(&s->midbuf,  in_count))<0)
            return ret;
    }
    if((ret=swri_realloc_audio(&s->preout, out_count))<0)
        return ret;

    postin= &s->postin;

    midbuf_tmp= s->midbuf;
    midbuf= &midbuf_tmp;
    preout_tmp= s->preout;
    preout= &preout_tmp;

    if(s->int_sample_fmt == s-> in_sample_fmt && s->in.planar && !s->channel_map)
        postin= in;

    if(s->resample_first ? !s->resample : !s->rematrix)
        midbuf= postin;

    if(s->resample_first ? !s->rematrix : !s->resample)
        preout= midbuf;

    if(s->int_sample_fmt == s->out_sample_fmt && s->out.planar
       && !(s->out_sample_fmt==AV_SAMPLE_FMT_S32P && (s->dither.output_sample_bits&31))){
        if(preout==in){
            out_count= FFMIN(out_count, in_count); //TODO check at the end if this is needed or redundant
            av_assert0(s->in.planar); //we only support planar internally so it has to be, we support copying non planar though
            copy(out, in, out_count);
            return out_count;
        }
        else if(preout==postin) preout= midbuf= postin= out;
        else if(preout==midbuf) preout= midbuf= out;
        else                    preout= out;
    }

    if(in != postin){
        swri_audio_convert(s->in_convert, postin, in, in_count);
    }

    if(s->resample_first){
        if(postin != midbuf)
            out_count= swresample(s, midbuf, out_count, postin, in_count);
        if(midbuf != preout)
            swri_rematrix(s, preout, midbuf, out_count, preout==out);
    }else{
        if(postin != midbuf)
            swri_rematrix(s, midbuf, postin, in_count, midbuf==out);
        if(midbuf != preout)
            out_count= swresample(s, preout, out_count, midbuf, in_count);
    }

    if(preout != out && out_count){
        SWresampleAudioData *conv_src = preout;
        if(s->dither.method){
            int ch;
            int dither_count= FFMAX(out_count, 1<<16);

            if (preout == in) {
                conv_src = &s->dither.temp;
                if((ret=swri_realloc_audio(&s->dither.temp, dither_count))<0)
                    return ret;
            }

            if((ret=swri_realloc_audio(&s->dither.noise, dither_count))<0)
                return ret;
            if(ret)
                for(ch=0; ch<s->dither.noise.ch_count; ch++)
                    if((ret=swri_get_dither(s, s->dither.noise.ch[ch], s->dither.noise.count, (12345678913579ULL*ch + 3141592) % 2718281828U, s->dither.noise.fmt))<0)
                        return ret;
            av_assert0(s->dither.noise.ch_count == preout->ch_count);

            if(s->dither.noise_pos + out_count > s->dither.noise.count)
                s->dither.noise_pos = 0;

            if (s->dither.method < SWR_DITHER_NS){
                if (s->mix_2_1_simd) {
                    int len1= out_count&~15;
                    int off = len1 * preout->bps;

                    if(len1)
                        for(ch=0; ch<preout->ch_count; ch++)
                            s->mix_2_1_simd(conv_src->ch[ch], preout->ch[ch], s->dither.noise.ch[ch] + s->dither.noise.bps * s->dither.noise_pos, s->native_simd_one, 0, 0, len1);
                    if(out_count != len1)
                        for(ch=0; ch<preout->ch_count; ch++)
                            s->mix_2_1_f(conv_src->ch[ch] + off, preout->ch[ch] + off, s->dither.noise.ch[ch] + s->dither.noise.bps * s->dither.noise_pos + off, s->native_one, 0, 0, out_count - len1);
                } else {
                    for(ch=0; ch<preout->ch_count; ch++)
                        s->mix_2_1_f(conv_src->ch[ch], preout->ch[ch], s->dither.noise.ch[ch] + s->dither.noise.bps * s->dither.noise_pos, s->native_one, 0, 0, out_count);
                }
            } else {
                switch(s->int_sample_fmt) {
                // case AV_SAMPLE_FMT_S16P :swri_noise_shaping_int16(s, conv_src, preout, &s->dither.noise, out_count); break;
                // case AV_SAMPLE_FMT_S32P :swri_noise_shaping_int32(s, conv_src, preout, &s->dither.noise, out_count); break;
                // case AV_SAMPLE_FMT_FLTP :swri_noise_shaping_float(s, conv_src, preout, &s->dither.noise, out_count); break;
                // case AV_SAMPLE_FMT_DBLP :swri_noise_shaping_double(s,conv_src, preout, &s->dither.noise, out_count); break;
                }
            }
            s->dither.noise_pos += out_count;
        }
//FIXME packed doesn't need more than 1 chan here!
        swri_audio_convert(s->out_convert, out, conv_src, out_count);
    }
    return out_count;
}
int  swr_convert(struct SwrContext *s, uint8_t *out_arg[SWR_CH_MAX], int out_count,
                                                    const uint8_t *in_arg [SWR_CH_MAX], int  in_count){
    SWresampleAudioData * in= &s->in;
    SWresampleAudioData *out= &s->out;
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
        SWresampleAudioData tmp= *in;
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






static void quantize_triangular_ns(DitherContext *c, DitherState *state,
                                   int16_t *dst, const float *src,
                                   int nb_samples)
{
    int i, j;
    float *dither = &state->noise_buf[state->noise_buf_ptr];

    if (state->mute > c->mute_reset_threshold)
        memset(state->dither_a, 0, sizeof(state->dither_a));

    for (i = 0; i < nb_samples; i++) {
        float err = 0;
        float sample = src[i] * S16_SCALE;

        for (j = 0; j < 4; j++) {
            err += c->ns_coef_b[j] * state->dither_b[j] -
                   c->ns_coef_a[j] * state->dither_a[j];
        }
        for (j = 3; j > 0; j--) {
            state->dither_a[j] = state->dither_a[j - 1];
            state->dither_b[j] = state->dither_b[j - 1];
        }
        state->dither_a[0] = err;
        sample -= err;

        if (state->mute > c->mute_dither_threshold) {
            dst[i]             = av_clip_int16(lrintf(sample));
            state->dither_b[0] = 0;
        } else {
            dst[i]             = av_clip_int16(lrintf(sample + dither[i]));
            state->dither_b[0] = av_clipf(dst[i] - sample, -1.5f, 1.5f);
        }

        state->mute++;
        if (src[i])
            state->mute = 0;
    }
}
static int convert_samples(DitherContext *c, int16_t **dst, float * const *src,
                           int channels, int nb_samples)
{
    int ch, ret;
    int aligned_samples = FFALIGN(nb_samples, 16);

    for (ch = 0; ch < channels; ch++) {
        DitherState *state = &c->state[ch];

        if (state->noise_buf_size < aligned_samples) {
            ret = generate_dither_noise(c, state, nb_samples);
            if (ret < 0)
                return ret;
        } else if (state->noise_buf_size - state->noise_buf_ptr < aligned_samples) {
            state->noise_buf_ptr = 0;
        }

        if (c->method == AV_RESAMPLE_DITHER_TRIANGULAR_NS) {
            quantize_triangular_ns(c, state, dst[ch], src[ch], nb_samples);
        } else {
            c->quantize(dst[ch], src[ch],
                        &state->noise_buf[state->noise_buf_ptr],
                        FFALIGN(nb_samples, c->samples_align));
        }

        state->noise_buf_ptr += aligned_samples;
    }

    return 0;
}


int ff_convert_dither(DitherContext *c, AudioData *dst, AudioData *src)
{
    int ret;
    AudioData *flt_data;

    /* output directly to dst if it is planar */
    if (dst->sample_fmt == AV_SAMPLE_FMT_S16P)
        c->s16_data = dst;
    else {
        /* make sure s16_data is large enough for the output */
        ret = ff_audio_data_realloc(c->s16_data, src->nb_samples);
        if (ret < 0)
            return ret;
    }

    if (src->sample_fmt != AV_SAMPLE_FMT_FLTP || c->apply_map) {
        /* make sure flt_data is large enough for the input */
        ret = ff_audio_data_realloc(c->flt_data, src->nb_samples);
        if (ret < 0)
            return ret;
        flt_data = c->flt_data;
    }

    if (src->sample_fmt != AV_SAMPLE_FMT_FLTP) {
        /* convert input samples to fltp and scale to s16 range */
        ret = ff_audio_convert(c->ac_in, flt_data, src);
        if (ret < 0)
            return ret;
    } else if (c->apply_map) {
        ret = ff_audio_data_copy(flt_data, src, c->ch_map_info);
        if (ret < 0)
            return ret;
    } else {
        flt_data = src;
    }

    /* check alignment and padding constraints */
    if (c->method != AV_RESAMPLE_DITHER_TRIANGULAR_NS) {
        int ptr_align     = FFMIN(flt_data->ptr_align,     c->s16_data->ptr_align);
        int samples_align = FFMIN(flt_data->samples_align, c->s16_data->samples_align);
        int aligned_len   = FFALIGN(src->nb_samples, c->ddsp.samples_align);

        if (!(ptr_align % c->ddsp.ptr_align) && samples_align >= aligned_len) {
            c->quantize      = c->ddsp.quantize;
            c->samples_align = c->ddsp.samples_align;
        } else {
            c->quantize      = quantize_c;
            c->samples_align = 1;
        }
    }

    ret = convert_samples(c, (int16_t **)c->s16_data->data,
                          (float * const *)flt_data->data, src->channels,
                          src->nb_samples);
    if (ret < 0)
        return ret;

    c->s16_data->nb_samples = src->nb_samples;

    /* interleave output to dst if needed */
    if (dst->sample_fmt == AV_SAMPLE_FMT_S16) {
        ret = ff_audio_convert(c->ac_out, dst, c->s16_data);
        if (ret < 0)
            return ret;
    } else
        c->s16_data = NULL;

    return 0;
}
int ff_audio_convert(AudioConvert *ac, AudioData *out, AudioData *in)
{
    int use_generic = 1;
    int len         = in->nb_samples;
    int p;

    if (ac->dc) {
        /* dithered conversion */
        av_log(ac->avr, AV_LOG_TRACE, "%d samples - audio_convert: %s to %s (dithered)\n",
                len, av_get_sample_fmt_name(ac->in_fmt),
                av_get_sample_fmt_name(ac->out_fmt));

        return ff_convert_dither(ac->dc, out, in);
    }

    /* determine whether to use the optimized function based on pointer and
       samples alignment in both the input and output */
    if (ac->has_optimized_func) {
        int ptr_align     = FFMIN(in->ptr_align,     out->ptr_align);
        int samples_align = FFMIN(in->samples_align, out->samples_align);
        int aligned_len   = FFALIGN(len, ac->samples_align);
        if (!(ptr_align % ac->ptr_align) && samples_align >= aligned_len) {
            len = aligned_len;
            use_generic = 0;
        }
    }
    av_log(ac->avr, AV_LOG_TRACE, "%d samples - audio_convert: %s to %s (%s)\n", len,
            av_get_sample_fmt_name(ac->in_fmt),
            av_get_sample_fmt_name(ac->out_fmt),
            use_generic ? ac->func_descr_generic : ac->func_descr);

    if (ac->apply_map) {
        ChannelMapInfo *map = &ac->avr->ch_map_info;

        if (!ff_sample_fmt_is_planar(ac->out_fmt, ac->channels)) {
            av_log(ac->avr, AV_LOG_ERROR, "cannot remap packed format during conversion\n");
            return AVERROR(EINVAL);
        }

        if (map->do_remap) {
            if (ff_sample_fmt_is_planar(ac->in_fmt, ac->channels)) {
                conv_func_flat *convert = use_generic ? ac->conv_flat_generic :
                                                        ac->conv_flat;

                for (p = 0; p < ac->planes; p++)
                    if (map->channel_map[p] >= 0)
                        convert(out->data[p], in->data[map->channel_map[p]], len);
            } else {
                uint8_t *data[AVRESAMPLE_MAX_CHANNELS];
                conv_func_deinterleave *convert = use_generic ?
                                                  ac->conv_deinterleave_generic :
                                                  ac->conv_deinterleave;

                for (p = 0; p < ac->channels; p++)
                    data[map->input_map[p]] = out->data[p];

                convert(data, in->data[0], len, ac->channels);
            }
        }
        if (map->do_copy || map->do_zero) {
            for (p = 0; p < ac->planes; p++) {
                if (map->channel_copy[p])
                    memcpy(out->data[p], out->data[map->channel_copy[p]],
                           len * out->stride);
                else if (map->channel_zero[p])
                    av_samples_set_silence(&out->data[p], 0, len, 1, ac->out_fmt);
            }
        }
    } else {
        switch (ac->func_type) {
        case CONV_FUNC_TYPE_FLAT: {
            if (!in->is_planar)
                len *= in->channels;
            if (use_generic) {
                for (p = 0; p < ac->planes; p++)
                    ac->conv_flat_generic(out->data[p], in->data[p], len);
            } else {
                for (p = 0; p < ac->planes; p++)
                    ac->conv_flat(out->data[p], in->data[p], len);
            }
            break;
        }
        case CONV_FUNC_TYPE_INTERLEAVE:
            if (use_generic)
                ac->conv_interleave_generic(out->data[0], in->data, len,
                                            ac->channels);
            else
                ac->conv_interleave(out->data[0], in->data, len, ac->channels);
            break;
        case CONV_FUNC_TYPE_DEINTERLEAVE:
            if (use_generic)
                ac->conv_deinterleave_generic(out->data, in->data[0], len,
                                              ac->channels);
            else
                ac->conv_deinterleave(out->data, in->data[0], len,
                                      ac->channels);
            break;
        }
    }

    out->nb_samples = in->nb_samples;
    return 0;
}



int ff_audio_mix(AudioMix *am, AudioData *src)
{
    int use_generic = 1;
    int len = src->nb_samples;
    int i, j;

    /* determine whether to use the optimized function based on pointer and
       samples alignment in both the input and output */
    if (am->has_optimized_func) {
        int aligned_len = FFALIGN(len, am->samples_align);
        if (!(src->ptr_align % am->ptr_align) &&
            src->samples_align >= aligned_len) {
            len = aligned_len;
            use_generic = 0;
        }
    }
    av_log(am->avr, AV_LOG_TRACE, "audio_mix: %d samples - %d to %d channels (%s)\n",
            src->nb_samples, am->in_channels, am->out_channels,
            use_generic ? am->func_descr_generic : am->func_descr);

    if (am->in_matrix_channels && am->out_matrix_channels) {
        uint8_t **data;
        uint8_t *data0[AVRESAMPLE_MAX_CHANNELS] = { NULL };

        if (am->out_matrix_channels < am->out_channels ||
             am->in_matrix_channels <  am->in_channels) {
            for (i = 0, j = 0; i < FFMAX(am->in_channels, am->out_channels); i++) {
                if (am->input_skip[i] || am->output_skip[i] || am->output_zero[i])
                    continue;
                data0[j++] = src->data[i];
            }
            data = data0;
        } else {
            data = src->data;
        }

        if (use_generic)
            am->mix_generic(data, am->matrix, len, am->out_matrix_channels,
                            am->in_matrix_channels);
        else
            am->mix(data, am->matrix, len, am->out_matrix_channels,
                    am->in_matrix_channels);
    }

    if (am->out_matrix_channels < am->out_channels) {
        for (i = 0; i < am->out_channels; i++)
            if (am->output_zero[i])
                av_samples_set_silence(&src->data[i], 0, len, 1, am->fmt);
    }

    ff_audio_data_set_channels(src, am->out_channels);

    return 0;
}


int ff_audio_resample(ResampleContext *c, AudioData *dst, AudioData *src)
{
    int ch, in_samples, in_leftover, consumed = 0, out_samples = 0;
    int ret = AVERROR(EINVAL);
    int nearest_neighbour = (c->compensation_distance == 0 &&
                             c->filter_length == 1 &&
                             c->phase_shift == 0);

    in_samples  = src ? src->nb_samples : 0;
    in_leftover = c->buffer->nb_samples;

    /* add input samples to the internal buffer */
    if (src) {
        ret = ff_audio_data_combine(c->buffer, in_leftover, src, 0, in_samples);
        if (ret < 0)
            return ret;
    } else if (in_leftover <= c->final_padding_samples) {
        /* no remaining samples to flush */
        return 0;
    }

    if (!c->initial_padding_filled) {
        int bps = av_get_bytes_per_sample(c->avr->internal_sample_fmt);
        int i;

        if (src && c->buffer->nb_samples < 2 * c->padding_size)
            return 0;

        for (i = 0; i < c->padding_size; i++)
            for (ch = 0; ch < c->buffer->channels; ch++) {
                if (c->buffer->nb_samples > 2 * c->padding_size - i) {
                    memcpy(c->buffer->data[ch] + bps * i,
                           c->buffer->data[ch] + bps * (2 * c->padding_size - i), bps);
                } else {
                    memset(c->buffer->data[ch] + bps * i, 0, bps);
                }
            }
        c->initial_padding_filled = 1;
    }

    if (!src && !c->final_padding_filled) {
        int bps = av_get_bytes_per_sample(c->avr->internal_sample_fmt);
        int i;

        ret = ff_audio_data_realloc(c->buffer,
                                    FFMAX(in_samples, in_leftover) +
                                    c->padding_size);
        if (ret < 0) {
            av_log(c->avr, AV_LOG_ERROR, "Error reallocating resampling buffer\n");
            return AVERROR(ENOMEM);
        }

        for (i = 0; i < c->padding_size; i++)
            for (ch = 0; ch < c->buffer->channels; ch++) {
                if (in_leftover > i) {
                    memcpy(c->buffer->data[ch] + bps * (in_leftover + i),
                           c->buffer->data[ch] + bps * (in_leftover - i - 1),
                           bps);
                } else {
                    memset(c->buffer->data[ch] + bps * (in_leftover + i),
                           0, bps);
                }
            }
        c->buffer->nb_samples   += c->padding_size;
        c->final_padding_samples = c->padding_size;
        c->final_padding_filled  = 1;
    }


    /* calculate output size and reallocate output buffer if needed */
    /* TODO: try to calculate this without the dummy resample() run */
    if (!dst->read_only && dst->allow_realloc) {
        out_samples = resample(c, NULL, NULL, NULL, c->buffer->nb_samples,
                               INT_MAX, 0, nearest_neighbour);
        ret = ff_audio_data_realloc(dst, out_samples);
        if (ret < 0) {
            av_log(c->avr, AV_LOG_ERROR, "error reallocating output\n");
            return ret;
        }
    }

    /* resample each channel plane */
    for (ch = 0; ch < c->buffer->channels; ch++) {
        out_samples = resample(c, (void *)dst->data[ch],
                               (const void *)c->buffer->data[ch], &consumed,
                               c->buffer->nb_samples, dst->allocated_samples,
                               ch + 1 == c->buffer->channels, nearest_neighbour);
    }
    if (out_samples < 0) {
        av_log(c->avr, AV_LOG_ERROR, "error during resampling\n");
        return out_samples;
    }

    /* drain consumed samples from the internal buffer */
    ff_audio_data_drain(c->buffer, consumed);
    c->initial_padding_samples = FFMAX(c->initial_padding_samples - consumed, 0);

    av_log(c->avr, AV_LOG_TRACE, "resampled %d in + %d leftover to %d out + %d leftover\n",
            in_samples, in_leftover, out_samples, c->buffer->nb_samples);

    dst->nb_samples = out_samples;
    return 0;
}
int  avresample_convert(AVAudioResampleContext *avr,
                                           uint8_t **output, int out_plane_size,
                                           int out_samples,
                                           uint8_t * const *input,
                                           int in_plane_size, int in_samples)
{
    AudioData input_buffer;
    AudioData output_buffer;
    AudioData *current_buffer;
    int ret, direct_output;

    /* reset internal buffers */
    if (avr->in_buffer) {
        avr->in_buffer->nb_samples = 0;
        ff_audio_data_set_channels(avr->in_buffer,
                                   avr->in_buffer->allocated_channels);
    }
    if (avr->resample_out_buffer) {
        avr->resample_out_buffer->nb_samples = 0;
        ff_audio_data_set_channels(avr->resample_out_buffer,
                                   avr->resample_out_buffer->allocated_channels);
    }
    if (avr->out_buffer) {
        avr->out_buffer->nb_samples = 0;
        ff_audio_data_set_channels(avr->out_buffer,
                                   avr->out_buffer->allocated_channels);
    }

    av_log(avr, AV_LOG_TRACE, "[start conversion]\n");

    /* initialize output_buffer with output data */
    direct_output = output && av_audio_fifo_size(avr->out_fifo) == 0;
    if (output) {
        ret = ff_audio_data_init(&output_buffer, output, out_plane_size,
                                 avr->out_channels, out_samples,
                                 avr->out_sample_fmt, 0, "output");
        if (ret < 0)
            return ret;
        output_buffer.nb_samples = 0;
    }

    if (input) {
        /* initialize input_buffer with input data */
        ret = ff_audio_data_init(&input_buffer, input, in_plane_size,
                                 avr->in_channels, in_samples,
                                 avr->in_sample_fmt, 1, "input");
        if (ret < 0)
            return ret;
        current_buffer = &input_buffer;

        if (avr->upmix_needed && !avr->in_convert_needed && !avr->resample_needed &&
            !avr->out_convert_needed && direct_output && out_samples >= in_samples) {
            /* in some rare cases we can copy input to output and upmix
               directly in the output buffer */
            av_log(avr, AV_LOG_TRACE, "[copy] %s to output\n", current_buffer->name);
            ret = ff_audio_data_copy(&output_buffer, current_buffer,
                                     avr->remap_point == REMAP_OUT_COPY ?
                                     &avr->ch_map_info : NULL);
            if (ret < 0)
                return ret;
            current_buffer = &output_buffer;
        } else if (avr->remap_point == REMAP_OUT_COPY &&
                   (!direct_output || out_samples < in_samples)) {
            /* if remapping channels during output copy, we may need to
             * use an intermediate buffer in order to remap before adding
             * samples to the output fifo */
            av_log(avr, AV_LOG_TRACE, "[copy] %s to out_buffer\n", current_buffer->name);
            ret = ff_audio_data_copy(avr->out_buffer, current_buffer,
                                     &avr->ch_map_info);
            if (ret < 0)
                return ret;
            current_buffer = avr->out_buffer;
        } else if (avr->in_copy_needed || avr->in_convert_needed) {
            /* if needed, copy or convert input to in_buffer, and downmix if
               applicable */
            if (avr->in_convert_needed) {
                ret = ff_audio_data_realloc(avr->in_buffer,
                                            current_buffer->nb_samples);
                if (ret < 0)
                    return ret;
                av_log(avr, AV_LOG_TRACE, "[convert] %s to in_buffer\n", current_buffer->name);
                ret = ff_audio_convert(avr->ac_in, avr->in_buffer,
                                       current_buffer);
                if (ret < 0)
                    return ret;
            } else {
                av_log(avr, AV_LOG_TRACE, "[copy] %s to in_buffer\n", current_buffer->name);
                ret = ff_audio_data_copy(avr->in_buffer, current_buffer,
                                         avr->remap_point == REMAP_IN_COPY ?
                                         &avr->ch_map_info : NULL);
                if (ret < 0)
                    return ret;
            }
            ff_audio_data_set_channels(avr->in_buffer, avr->in_channels);
            if (avr->downmix_needed) {
                av_log(avr, AV_LOG_TRACE, "[downmix] in_buffer\n");
                ret = ff_audio_mix(avr->am, avr->in_buffer);
                if (ret < 0)
                    return ret;
            }
            current_buffer = avr->in_buffer;
        }
    } else {
        /* flush resampling buffer and/or output FIFO if input is NULL */
        if (!avr->resample_needed)
            return handle_buffered_output(avr, output ? &output_buffer : NULL,
                                          NULL);
        current_buffer = NULL;
    }

    if (avr->resample_needed) {
        AudioData *resample_out;

        if (!avr->out_convert_needed && direct_output && out_samples > 0)
            resample_out = &output_buffer;
        else
            resample_out = avr->resample_out_buffer;
        av_log(avr, AV_LOG_TRACE, "[resample] %s to %s\n",
                current_buffer ? current_buffer->name : "null",
                resample_out->name);
        ret = ff_audio_resample(avr->resample, resample_out,
                                current_buffer);
        if (ret < 0)
            return ret;

        /* if resampling did not produce any samples, just return 0 */
        if (resample_out->nb_samples == 0) {
            av_log(avr, AV_LOG_TRACE, "[end conversion]\n");
            return 0;
        }

        current_buffer = resample_out;
    }

    if (avr->upmix_needed) {
        av_log(avr, AV_LOG_TRACE, "[upmix] %s\n", current_buffer->name);
        ret = ff_audio_mix(avr->am, current_buffer);
        if (ret < 0)
            return ret;
    }

    /* if we resampled or upmixed directly to output, return here */
    if (current_buffer == &output_buffer) {
        av_log(avr, AV_LOG_TRACE, "[end conversion]\n");
        return current_buffer->nb_samples;
    }

    if (avr->out_convert_needed) {
        if (direct_output && out_samples >= current_buffer->nb_samples) {
            /* convert directly to output */
            av_log(avr, AV_LOG_TRACE, "[convert] %s to output\n", current_buffer->name);
            ret = ff_audio_convert(avr->ac_out, &output_buffer, current_buffer);
            if (ret < 0)
                return ret;

            av_log(avr, AV_LOG_TRACE, "[end conversion]\n");
            return output_buffer.nb_samples;
        } else {
            ret = ff_audio_data_realloc(avr->out_buffer,
                                        current_buffer->nb_samples);
            if (ret < 0)
                return ret;
            av_log(avr, AV_LOG_TRACE, "[convert] %s to out_buffer\n", current_buffer->name);
            ret = ff_audio_convert(avr->ac_out, avr->out_buffer,
                                   current_buffer);
            if (ret < 0)
                return ret;
            current_buffer = avr->out_buffer;
        }
    }

    return handle_buffered_output(avr, output ? &output_buffer : NULL,
                                  current_buffer);
}

int avresample_config(AVAudioResampleContext *avr, AVFrame *out, AVFrame *in)
{
    if (avresample_is_open(avr)) {
        avresample_close(avr);
    }

    if (in) {
        avr->in_channel_layout  = in->channel_layout;
        avr->in_sample_rate     = in->sample_rate;
        avr->in_sample_fmt      = in->format;
    }

    if (out) {
        avr->out_channel_layout = out->channel_layout;
        avr->out_sample_rate    = out->sample_rate;
        avr->out_sample_fmt     = out->format;
    }

    return 0;
}

static int config_changed(AVAudioResampleContext *avr,
                          AVFrame *out, AVFrame *in)
{
    int ret = 0;

    if (in) {
        if (avr->in_channel_layout != in->channel_layout ||
            avr->in_sample_rate    != in->sample_rate ||
            avr->in_sample_fmt     != in->format) {
            ret |= AVERROR_INPUT_CHANGED;
        }
    }

    if (out) {
        if (avr->out_channel_layout != out->channel_layout ||
            avr->out_sample_rate    != out->sample_rate ||
            avr->out_sample_fmt     != out->format) {
            ret |= AVERROR_OUTPUT_CHANGED;
        }
    }

    return ret;
}

static inline int convert_frame(AVAudioResampleContext *avr,
                                AVFrame *out, AVFrame *in)
{
    int ret;
    uint8_t **out_data = NULL, **in_data = NULL;
    int out_linesize = 0, in_linesize = 0;
    int out_nb_samples = 0, in_nb_samples = 0;

    if (out) {
        out_data       = out->extended_data;
        out_linesize   = out->linesize[0];
        out_nb_samples = out->nb_samples;
    }

    if (in) {
        in_data       = in->extended_data;
        in_linesize   = in->linesize[0];
        in_nb_samples = in->nb_samples;
    }

    ret = avresample_convert(avr, out_data, out_linesize,
                             out_nb_samples,
                             in_data, in_linesize,
                             in_nb_samples);

    if (ret < 0) {
        if (out)
            out->nb_samples = 0;
        return ret;
    }

    if (out)
        out->nb_samples = ret;

    return 0;
}

int avresample_get_delay(AVAudioResampleContext *avr)
{
    ResampleContext *c = avr->resample;

    if (!avr->resample_needed || !avr->resample)
        return 0;

    return FFMAX(c->buffer->nb_samples - c->padding_size, 0);
}
int avresample_available(AVAudioResampleContext *avr)
{
    return av_audio_fifo_size(avr->out_fifo);
}

int avresample_get_out_samples(AVAudioResampleContext *avr, int in_nb_samples)
{
    int64_t samples = avresample_get_delay(avr) + (int64_t)in_nb_samples;

    if (avr->resample_needed) {
        samples = av_rescale_rnd(samples,
                                 avr->out_sample_rate,
                                 avr->in_sample_rate,
                                 AV_ROUND_UP);
    }

    samples += avresample_available(avr);

    if (samples > INT_MAX)
        return AVERROR(EINVAL);

    return samples;
}


static inline int available_samples(AVFrame *out)
{
    int samples;
    int bytes_per_sample = av_get_bytes_per_sample(out->format);
    if (!bytes_per_sample)
        return AVERROR(EINVAL);

    samples = out->linesize[0] / bytes_per_sample;
    if (av_sample_fmt_is_planar(out->format)) {
        return samples;
    } else {
        int channels = av_get_channel_layout_nb_channels(out->channel_layout);
        return samples / channels;
    }
}

int avresample_convert_frame(AVAudioResampleContext *avr,
                             AVFrame *out, AVFrame *in)
{
    int ret, setup = 0;

    if (!avresample_is_open(avr)) {
        if ((ret = avresample_config(avr, out, in)) < 0)
            return ret;
        if ((ret = avresample_open(avr)) < 0)
            return ret;
        setup = 1;
    } else {
        // return as is or reconfigure for input changes?
        if ((ret = config_changed(avr, out, in)))
            return ret;
    }

    if (out) {
        if (!out->linesize[0]) {
            out->nb_samples = avresample_get_out_samples(avr, in->nb_samples);
            if ((ret = av_frame_get_buffer(out, 0)) < 0) {
                if (setup)
                    avresample_close(avr);
                return ret;
            }
        } else {
            if (!out->nb_samples)
                out->nb_samples = available_samples(out);
        }
    }

    return convert_frame(avr, out, in);
}


int ff_audio_mix_get_matrix(AudioMix *am, double *matrix, int stride)
{
    int i, o, i0, o0;

    if ( am->in_channels <= 0 ||  am->in_channels > AVRESAMPLE_MAX_CHANNELS ||
        am->out_channels <= 0 || am->out_channels > AVRESAMPLE_MAX_CHANNELS) {
        av_log(am->avr, AV_LOG_ERROR, "Invalid channel counts\n");
        return AVERROR(EINVAL);
    }

#define GET_MATRIX_CONVERT(suffix, scale)                                   \
    if (!am->matrix_ ## suffix[0]) {                                        \
        av_log(am->avr, AV_LOG_ERROR, "matrix is not set\n");               \
        return AVERROR(EINVAL);                                             \
    }                                                                       \
    for (o = 0, o0 = 0; o < am->out_channels; o++) {                        \
        for (i = 0, i0 = 0; i < am->in_channels; i++) {                     \
            if (am->input_skip[i] || am->output_zero[o])                    \
                matrix[o * stride + i] = 0.0;                               \
            else                                                            \
                matrix[o * stride + i] = am->matrix_ ## suffix[o0][i0] *    \
                                         (scale);                           \
            if (!am->input_skip[i])                                         \
                i0++;                                                       \
        }                                                                   \
        if (!am->output_zero[o])                                            \
            o0++;                                                           \
    }

    switch (am->coeff_type) {
    case AV_MIX_COEFF_TYPE_Q8:
        GET_MATRIX_CONVERT(q8, 1.0 / 256.0);
        break;
    case AV_MIX_COEFF_TYPE_Q15:
        GET_MATRIX_CONVERT(q15, 1.0 / 32768.0);
        break;
    case AV_MIX_COEFF_TYPE_FLT:
        GET_MATRIX_CONVERT(flt, 1.0);
        break;
    default:
        av_log(am->avr, AV_LOG_ERROR, "Invalid mix coeff type\n");
        return AVERROR(EINVAL);
    }

    return 0;
}
int avresample_get_matrix(AVAudioResampleContext *avr, double *matrix,
                          int stride)
{
    int in_channels, out_channels, i, o;

    if (avr->am)
        return ff_audio_mix_get_matrix(avr->am, matrix, stride);

    in_channels  = av_get_channel_layout_nb_channels(avr->in_channel_layout);
    out_channels = av_get_channel_layout_nb_channels(avr->out_channel_layout);

    if ( in_channels <= 0 ||  in_channels > AVRESAMPLE_MAX_CHANNELS ||
        out_channels <= 0 || out_channels > AVRESAMPLE_MAX_CHANNELS) {
        av_log(avr, AV_LOG_ERROR, "Invalid channel layouts\n");
        return AVERROR(EINVAL);
    }

    if (!avr->mix_matrix) {
        av_log(avr, AV_LOG_ERROR, "matrix is not set\n");
        return AVERROR(EINVAL);
    }

    for (o = 0; o < out_channels; o++)
        for (i = 0; i < in_channels; i++)
            matrix[o * stride + i] = avr->mix_matrix[o * in_channels + i];

    return 0;
}






int avresample_set_matrix(AVAudioResampleContext *avr, const double *matrix,
                          int stride)
{
    int in_channels, out_channels, i, o;

    if (avr->am)
        return ff_audio_mix_set_matrix(avr->am, matrix, stride);

    in_channels  = av_get_channel_layout_nb_channels(avr->in_channel_layout);
    out_channels = av_get_channel_layout_nb_channels(avr->out_channel_layout);

    if ( in_channels <= 0 ||  in_channels > AVRESAMPLE_MAX_CHANNELS ||
        out_channels <= 0 || out_channels > AVRESAMPLE_MAX_CHANNELS) {
        av_log(avr, AV_LOG_ERROR, "Invalid channel layouts\n");
        return AVERROR(EINVAL);
    }

    if (avr->mix_matrix)
        av_freep(&avr->mix_matrix);
    avr->mix_matrix = av_malloc(in_channels * out_channels *
                                sizeof(*avr->mix_matrix));
    if (!avr->mix_matrix)
        return AVERROR(ENOMEM);

    for (o = 0; o < out_channels; o++)
        for (i = 0; i < in_channels; i++)
            avr->mix_matrix[o * in_channels + i] = matrix[o * stride + i];

    return 0;
}

int avresample_set_channel_mapping(AVAudioResampleContext *avr,
                                   const int *channel_map)
{
    ChannelMapInfo *info = &avr->ch_map_info;
    int in_channels, ch, i;

    in_channels = av_get_channel_layout_nb_channels(avr->in_channel_layout);
    if (in_channels <= 0 ||  in_channels > AVRESAMPLE_MAX_CHANNELS) {
        av_log(avr, AV_LOG_ERROR, "Invalid input channel layout\n");
        return AVERROR(EINVAL);
    }

    memset(info, 0, sizeof(*info));
    memset(info->input_map, -1, sizeof(info->input_map));

    for (ch = 0; ch < in_channels; ch++) {
        if (channel_map[ch] >= in_channels) {
            av_log(avr, AV_LOG_ERROR, "Invalid channel map\n");
            return AVERROR(EINVAL);
        }
        if (channel_map[ch] < 0) {
            info->channel_zero[ch] =  1;
            info->channel_map[ch]  = -1;
            info->do_zero          =  1;
        } else if (info->input_map[channel_map[ch]] >= 0) {
            info->channel_copy[ch] = info->input_map[channel_map[ch]];
            info->channel_map[ch]  = -1;
            info->do_copy          =  1;
        } else {
            info->channel_map[ch]            = channel_map[ch];
            info->input_map[channel_map[ch]] = ch;
            info->do_remap                   =  1;
        }
    }
    /* Fill-in unmapped input channels with unmapped output channels.
       This is used when remapping during conversion from interleaved to
       planar format. */
    for (ch = 0, i = 0; ch < in_channels && i < in_channels; ch++, i++) {
        while (ch < in_channels && info->input_map[ch] >= 0)
            ch++;
        while (i < in_channels && info->channel_map[i] >= 0)
            i++;
        if (ch >= in_channels || i >= in_channels)
            break;
        info->input_map[ch] = i;
    }

    avr->use_channel_map = 1;
    return 0;
}





int avresample_read(AVAudioResampleContext *avr, uint8_t **output, int nb_samples)
{
    if (!output)
        return av_audio_fifo_drain(avr->out_fifo, nb_samples);
    return av_audio_fifo_read(avr->out_fifo, (void**)output, nb_samples);
}

unsigned avresample_version(void)
{
    return LIBAVRESAMPLE_VERSION_INT;
}
#define AVRESAMPLE_LICENSE_PREFIX "libavresample license: "

const char *avresample_license(void)
{
    return AVRESAMPLE_LICENSE_PREFIX FFMPEG_LICENSE + sizeof(AVRESAMPLE_LICENSE_PREFIX) - 1;
}

const char *avresample_configuration(void)
{
    return FFMPEG_CONFIGURATION;
}

#if HAVE_SIMD_ALIGN_64
#   define STRIDE_ALIGN 64 /* AVX-512 */
#elif HAVE_SIMD_ALIGN_32
#   define STRIDE_ALIGN 32
#elif HAVE_SIMD_ALIGN_16
#   define STRIDE_ALIGN 16
#else
#   define STRIDE_ALIGN 8
#endif

#define emms_c() do {} while(0)

#define MAKE_ACCESSORS(str, name, type, field) \
    type av_##name##_get_##field(const str *s) { return s->field; } \
    void av_##name##_set_##field(str *s, type v) { s->field = v; }

static AVMutex codec_mutex = AV_MUTEX_INITIALIZER;

void av_fast_padded_malloc(void *ptr, unsigned int *size, size_t min_size)
{
    uint8_t **p = ptr;
    if (min_size > SIZE_MAX - AV_INPUT_BUFFER_PADDING_SIZE) {
        av_freep(p);
        *size = 0;
        return;
    }
    if (!ff_fast_malloc(p, size, min_size + AV_INPUT_BUFFER_PADDING_SIZE, 1))
        memset(*p + min_size, 0, AV_INPUT_BUFFER_PADDING_SIZE);
}

void av_fast_padded_mallocz(void *ptr, unsigned int *size, size_t min_size)
{
    uint8_t **p = ptr;
    if (min_size > SIZE_MAX - AV_INPUT_BUFFER_PADDING_SIZE) {
        av_freep(p);
        *size = 0;
        return;
    }
    if (!ff_fast_malloc(p, size, min_size + AV_INPUT_BUFFER_PADDING_SIZE, 1))
        memset(*p, 0, min_size + AV_INPUT_BUFFER_PADDING_SIZE);
}

int av_codec_is_encoder(const AVCodec *codec)
{
    return codec && (codec->encode_sub || codec->encode2 || codec->receive_packet);
}

int av_codec_is_decoder(const AVCodec *codec)
{
    return codec && (codec->decode || codec->receive_frame);
}

int ff_set_dimensions(AVCodecContext *s, int width, int height)
{
    int ret = av_image_check_size2(width, height, s->max_pixels, AV_PIX_FMT_NONE, 0, s);

    if (ret < 0)
        width = height = 0;

    s->coded_width  = width;
    s->coded_height = height;
    s->width        = AV_CEIL_RSHIFT(width,  s->lowres);
    s->height       = AV_CEIL_RSHIFT(height, s->lowres);

    return ret;
}

int ff_set_sar(AVCodecContext *avctx, AVRational sar)
{
    int ret = av_image_check_sar(avctx->width, avctx->height, sar);

    if (ret < 0) {
        av_log(avctx, AV_LOG_WARNING, "ignoring invalid SAR: %d/%d\n",
               sar.num, sar.den);
        avctx->sample_aspect_ratio = (AVRational){ 0, 1 };
        return ret;
    } else {
        avctx->sample_aspect_ratio = sar;
    }
    return 0;
}

int ff_side_data_update_matrix_encoding(AVFrame *frame,
                                        enum AVMatrixEncoding matrix_encoding)
{
    AVFrameSideData *side_data;
    enum AVMatrixEncoding *data;

    side_data = av_frame_get_side_data(frame, AV_FRAME_DATA_MATRIXENCODING);
    if (!side_data)
        side_data = av_frame_new_side_data(frame, AV_FRAME_DATA_MATRIXENCODING,
                                           sizeof(enum AVMatrixEncoding));

    if (!side_data)
        return AVERROR(ENOMEM);

    data  = (enum AVMatrixEncoding*)side_data->data;
    *data = matrix_encoding;

    return 0;
}

void avcodec_align_dimensions2(AVCodecContext *s, int *width, int *height,
                               int linesize_align[AV_NUM_DATA_POINTERS])
{
    int i;
    int w_align = 1;
    int h_align = 1;
    AVPixFmtDescriptor const *desc = av_pix_fmt_desc_get(s->pix_fmt);

    if (desc) {
        w_align = 1 << desc->log2_chroma_w;
        h_align = 1 << desc->log2_chroma_h;
    }

    switch (s->pix_fmt) {
    case AV_PIX_FMT_YUV420P:
    case AV_PIX_FMT_YUYV422:
    case AV_PIX_FMT_YVYU422:
    case AV_PIX_FMT_UYVY422:
    case AV_PIX_FMT_YUV422P:
    case AV_PIX_FMT_YUV440P:
    case AV_PIX_FMT_YUV444P:
    case AV_PIX_FMT_GBRP:
    case AV_PIX_FMT_GBRAP:
    case AV_PIX_FMT_GRAY8:
    case AV_PIX_FMT_GRAY16BE:
    case AV_PIX_FMT_GRAY16LE:
    case AV_PIX_FMT_YUVJ420P:
    case AV_PIX_FMT_YUVJ422P:
    case AV_PIX_FMT_YUVJ440P:
    case AV_PIX_FMT_YUVJ444P:
    case AV_PIX_FMT_YUVA420P:
    case AV_PIX_FMT_YUVA422P:
    case AV_PIX_FMT_YUVA444P:
    case AV_PIX_FMT_YUV420P9LE:
    case AV_PIX_FMT_YUV420P9BE:
    case AV_PIX_FMT_YUV420P10LE:
    case AV_PIX_FMT_YUV420P10BE:
    case AV_PIX_FMT_YUV420P12LE:
    case AV_PIX_FMT_YUV420P12BE:
    case AV_PIX_FMT_YUV420P14LE:
    case AV_PIX_FMT_YUV420P14BE:
    case AV_PIX_FMT_YUV420P16LE:
    case AV_PIX_FMT_YUV420P16BE:
    case AV_PIX_FMT_YUVA420P9LE:
    case AV_PIX_FMT_YUVA420P9BE:
    case AV_PIX_FMT_YUVA420P10LE:
    case AV_PIX_FMT_YUVA420P10BE:
    case AV_PIX_FMT_YUVA420P16LE:
    case AV_PIX_FMT_YUVA420P16BE:
    case AV_PIX_FMT_YUV422P9LE:
    case AV_PIX_FMT_YUV422P9BE:
    case AV_PIX_FMT_YUV422P10LE:
    case AV_PIX_FMT_YUV422P10BE:
    case AV_PIX_FMT_YUV422P12LE:
    case AV_PIX_FMT_YUV422P12BE:
    case AV_PIX_FMT_YUV422P14LE:
    case AV_PIX_FMT_YUV422P14BE:
    case AV_PIX_FMT_YUV422P16LE:
    case AV_PIX_FMT_YUV422P16BE:
    case AV_PIX_FMT_YUVA422P9LE:
    case AV_PIX_FMT_YUVA422P9BE:
    case AV_PIX_FMT_YUVA422P10LE:
    case AV_PIX_FMT_YUVA422P10BE:
    case AV_PIX_FMT_YUVA422P12LE:
    case AV_PIX_FMT_YUVA422P12BE:
    case AV_PIX_FMT_YUVA422P16LE:
    case AV_PIX_FMT_YUVA422P16BE:
    case AV_PIX_FMT_YUV440P10LE:
    case AV_PIX_FMT_YUV440P10BE:
    case AV_PIX_FMT_YUV440P12LE:
    case AV_PIX_FMT_YUV440P12BE:
    case AV_PIX_FMT_YUV444P9LE:
    case AV_PIX_FMT_YUV444P9BE:
    case AV_PIX_FMT_YUV444P10LE:
    case AV_PIX_FMT_YUV444P10BE:
    case AV_PIX_FMT_YUV444P12LE:
    case AV_PIX_FMT_YUV444P12BE:
    case AV_PIX_FMT_YUV444P14LE:
    case AV_PIX_FMT_YUV444P14BE:
    case AV_PIX_FMT_YUV444P16LE:
    case AV_PIX_FMT_YUV444P16BE:
    case AV_PIX_FMT_YUVA444P9LE:
    case AV_PIX_FMT_YUVA444P9BE:
    case AV_PIX_FMT_YUVA444P10LE:
    case AV_PIX_FMT_YUVA444P10BE:
    case AV_PIX_FMT_YUVA444P12LE:
    case AV_PIX_FMT_YUVA444P12BE:
    case AV_PIX_FMT_YUVA444P16LE:
    case AV_PIX_FMT_YUVA444P16BE:
    case AV_PIX_FMT_GBRP9LE:
    case AV_PIX_FMT_GBRP9BE:
    case AV_PIX_FMT_GBRP10LE:
    case AV_PIX_FMT_GBRP10BE:
    case AV_PIX_FMT_GBRP12LE:
    case AV_PIX_FMT_GBRP12BE:
    case AV_PIX_FMT_GBRP14LE:
    case AV_PIX_FMT_GBRP14BE:
    case AV_PIX_FMT_GBRP16LE:
    case AV_PIX_FMT_GBRP16BE:
    case AV_PIX_FMT_GBRAP12LE:
    case AV_PIX_FMT_GBRAP12BE:
    case AV_PIX_FMT_GBRAP16LE:
    case AV_PIX_FMT_GBRAP16BE:
        w_align = 16; //FIXME assume 16 pixel per macroblock
        h_align = 16 * 2; // interlaced needs 2 macroblocks height
        break;
    case AV_PIX_FMT_YUV411P:
    case AV_PIX_FMT_YUVJ411P:
    case AV_PIX_FMT_UYYVYY411:
        w_align = 32;
        h_align = 16 * 2;
        break;
    case AV_PIX_FMT_YUV410P:
        if (s->codec_id == AV_CODEC_ID_SVQ1) {
            w_align = 64;
            h_align = 64;
        }
        break;
    case AV_PIX_FMT_RGB555:
        if (s->codec_id == AV_CODEC_ID_RPZA) {
            w_align = 4;
            h_align = 4;
        }
        if (s->codec_id == AV_CODEC_ID_INTERPLAY_VIDEO) {
            w_align = 8;
            h_align = 8;
        }
        break;
    case AV_PIX_FMT_PAL8:
    case AV_PIX_FMT_BGR8:
    case AV_PIX_FMT_RGB8:
        if (s->codec_id == AV_CODEC_ID_SMC ||
            s->codec_id == AV_CODEC_ID_CINEPAK) {
            w_align = 4;
            h_align = 4;
        }
        if (s->codec_id == AV_CODEC_ID_JV ||
            s->codec_id == AV_CODEC_ID_INTERPLAY_VIDEO) {
            w_align = 8;
            h_align = 8;
        }
        break;
    case AV_PIX_FMT_BGR24:
        if ((s->codec_id == AV_CODEC_ID_MSZH) ||
            (s->codec_id == AV_CODEC_ID_ZLIB)) {
            w_align = 4;
            h_align = 4;
        }
        break;
    case AV_PIX_FMT_RGB24:
        if (s->codec_id == AV_CODEC_ID_CINEPAK) {
            w_align = 4;
            h_align = 4;
        }
        break;
    default:
        break;
    }

    if (s->codec_id == AV_CODEC_ID_IFF_ILBM) {
        w_align = FFMAX(w_align, 8);
    }

    *width  = FFALIGN(*width, w_align);
    *height = FFALIGN(*height, h_align);
    if (s->codec_id == AV_CODEC_ID_H264 || s->lowres ||
        s->codec_id == AV_CODEC_ID_VP5  || s->codec_id == AV_CODEC_ID_VP6 ||
        s->codec_id == AV_CODEC_ID_VP6F || s->codec_id == AV_CODEC_ID_VP6A
    ) {
        // some of the optimized chroma MC reads one line too much
        // which is also done in mpeg decoders with lowres > 0
        *height += 2;

        // H.264 uses edge emulation for out of frame motion vectors, for this
        // it requires a temporary area large enough to hold a 21x21 block,
        // increasing witdth ensure that the temporary area is large enough,
        // the next rounded up width is 32
        *width = FFMAX(*width, 32);
    }

    for (i = 0; i < 4; i++)
        linesize_align[i] = STRIDE_ALIGN;
}

void avcodec_align_dimensions(AVCodecContext *s, int *width, int *height)
{
    const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(s->pix_fmt);
    int chroma_shift = desc->log2_chroma_w;
    int linesize_align[AV_NUM_DATA_POINTERS];
    int align;

    avcodec_align_dimensions2(s, width, height, linesize_align);
    align               = FFMAX(linesize_align[0], linesize_align[3]);
    linesize_align[1] <<= chroma_shift;
    linesize_align[2] <<= chroma_shift;
    align               = FFMAX3(align, linesize_align[1], linesize_align[2]);
    *width              = FFALIGN(*width, align);
}

int avcodec_enum_to_chroma_pos(int *xpos, int *ypos, enum AVChromaLocation pos)
{
    if (pos <= AVCHROMA_LOC_UNSPECIFIED || pos >= AVCHROMA_LOC_NB)
        return AVERROR(EINVAL);
    pos--;

    *xpos = (pos&1) * 128;
    *ypos = ((pos>>1)^(pos<4)) * 128;

    return 0;
}

enum AVChromaLocation avcodec_chroma_pos_to_enum(int xpos, int ypos)
{
    int pos, xout, yout;

    for (pos = AVCHROMA_LOC_UNSPECIFIED + 1; pos < AVCHROMA_LOC_NB; pos++) {
        if (avcodec_enum_to_chroma_pos(&xout, &yout, pos) == 0 && xout == xpos && yout == ypos)
            return pos;
    }
    return AVCHROMA_LOC_UNSPECIFIED;
}

int avcodec_fill_audio_frame(AVFrame *frame, int nb_channels,
                             enum AVSampleFormat sample_fmt, const uint8_t *buf,
                             int buf_size, int align)
{
    int ch, planar, needed_size, ret = 0;

    needed_size = av_samples_get_buffer_size(NULL, nb_channels,
                                             frame->nb_samples, sample_fmt,
                                             align);
    if (buf_size < needed_size)
        return AVERROR(EINVAL);

    planar = av_sample_fmt_is_planar(sample_fmt);
    if (planar && nb_channels > AV_NUM_DATA_POINTERS) {
        if (!(frame->extended_data = av_mallocz_array(nb_channels,
                                                sizeof(*frame->extended_data))))
            return AVERROR(ENOMEM);
    } else {
        frame->extended_data = frame->data;
    }

    if ((ret = av_samples_fill_arrays(frame->extended_data, &frame->linesize[0],
                                      (uint8_t *)(intptr_t)buf, nb_channels, frame->nb_samples,
                                      sample_fmt, align)) < 0) {
        if (frame->extended_data != frame->data)
            av_freep(&frame->extended_data);
        return ret;
    }
    if (frame->extended_data != frame->data) {
        for (ch = 0; ch < AV_NUM_DATA_POINTERS; ch++)
            frame->data[ch] = frame->extended_data[ch];
    }

    return ret;
}

void ff_color_frame(AVFrame *frame, const int c[4])
{
    const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(frame->format);
    int p, y;

    av_assert0(desc->flags & AV_PIX_FMT_FLAG_PLANAR);

    for (p = 0; p<desc->nb_components; p++) {
        uint8_t *dst = frame->data[p];
        int is_chroma = p == 1 || p == 2;
        int bytes  = is_chroma ? AV_CEIL_RSHIFT(frame->width,  desc->log2_chroma_w) : frame->width;
        int height = is_chroma ? AV_CEIL_RSHIFT(frame->height, desc->log2_chroma_h) : frame->height;
        if (desc->comp[0].depth >= 9) {
            ((uint16_t*)dst)[0] = c[p];
            av_memcpy_backptr(dst + 2, 2, bytes - 2);
            dst += frame->linesize[p];
            for (y = 1; y < height; y++) {
                memcpy(dst, frame->data[p], 2*bytes);
                dst += frame->linesize[p];
            }
        } else {
            for (y = 0; y < height; y++) {
                memset(dst, c[p], bytes);
                dst += frame->linesize[p];
            }
        }
    }
}

int avcodec_default_execute(AVCodecContext *c, int (*func)(AVCodecContext *c2, void *arg2), void *arg, int *ret, int count, int size)
{
    int i;

    for (i = 0; i < count; i++) {
        int r = func(c, (char *)arg + i * size);
        if (ret)
            ret[i] = r;
    }
    emms_c();
    return 0;
}

int avcodec_default_execute2(AVCodecContext *c, int (*func)(AVCodecContext *c2, void *arg2, int jobnr, int threadnr), void *arg, int *ret, int count)
{
    int i;

    for (i = 0; i < count; i++) {
        int r = func(c, arg, i, 0);
        if (ret)
            ret[i] = r;
    }
    emms_c();
    return 0;
}

enum AVPixelFormat avpriv_find_pix_fmt(const PixelFormatTag *tags,
                                       unsigned int fourcc)
{
    while (tags->pix_fmt >= 0) {
        if (tags->fourcc == fourcc)
            return tags->pix_fmt;
        tags++;
    }
    return AV_PIX_FMT_NONE;
}

#if FF_API_CODEC_GET_SET
MAKE_ACCESSORS(AVCodecContext, codec, AVRational, pkt_timebase)
MAKE_ACCESSORS(AVCodecContext, codec, const AVCodecDescriptor *, codec_descriptor)
MAKE_ACCESSORS(AVCodecContext, codec, int, lowres)
MAKE_ACCESSORS(AVCodecContext, codec, int, seek_preroll)
MAKE_ACCESSORS(AVCodecContext, codec, uint16_t*, chroma_intra_matrix)

unsigned av_codec_get_codec_properties(const AVCodecContext *codec)
{
    return codec->properties;
}

int av_codec_get_max_lowres(const AVCodec *codec)
{
    return codec->max_lowres;
}
#endif

int avpriv_codec_get_cap_skip_frame_fill_param(const AVCodec *codec){
    return !!(codec->caps_internal & FF_CODEC_CAP_SKIP_FRAME_FILL_PARAM);
}

static int64_t get_bit_rate(AVCodecContext *ctx)
{
    int64_t bit_rate;
    int bits_per_sample;

    switch (ctx->codec_type) {
    case AVMEDIA_TYPE_VIDEO:
    case AVMEDIA_TYPE_DATA:
    case AVMEDIA_TYPE_SUBTITLE:
    case AVMEDIA_TYPE_ATTACHMENT:
        bit_rate = ctx->bit_rate;
        break;
    case AVMEDIA_TYPE_AUDIO:
        bits_per_sample = av_get_bits_per_sample(ctx->codec_id);
        bit_rate = bits_per_sample ? ctx->sample_rate * (int64_t)ctx->channels * bits_per_sample : ctx->bit_rate;
        break;
    default:
        bit_rate = 0;
        break;
    }
    return bit_rate;
}


static void ff_lock_avcodec(AVCodecContext *log_ctx, const AVCodec *codec)
{
    if (!(codec->caps_internal & FF_CODEC_CAP_INIT_THREADSAFE) && codec->init)
        ff_mutex_lock(&codec_mutex);
}

static void ff_unlock_avcodec(const AVCodec *codec)
{
    if (!(codec->caps_internal & FF_CODEC_CAP_INIT_THREADSAFE) && codec->init)
        ff_mutex_unlock(&codec_mutex);
}

int  ff_codec_open2_recursive(AVCodecContext *avctx, const AVCodec *codec, AVDictionary **options)
{
    int ret = 0;

    ff_unlock_avcodec(codec);

    ret = avcodec_open2(avctx, codec, options);

    ff_lock_avcodec(avctx, codec);
    return ret;
}




int  avcodec_open2(AVCodecContext *avctx, const AVCodec *codec, AVDictionary **options)
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
        avctx->coded_frame = av_frame_alloc();
        if (!avctx->coded_frame) {
            ret = AVERROR(ENOMEM);
            goto free_and_end;
        }
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
        av_frame_free(&avctx->coded_frame);
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

void avcodec_flush_buffers(AVCodecContext *avctx)
{
    AVCodecInternal *avci = avctx->internal;

    if (av_codec_is_encoder(avctx->codec)) {
        int caps = avctx->codec->capabilities;

        if (!(caps & AV_CODEC_CAP_ENCODER_FLUSH)) {
            // Only encoders that explicitly declare support for it can be
            // flushed. Otherwise, this is a no-op.
            av_log(avctx, AV_LOG_WARNING, "Ignoring attempt to flush encoder "
                   "that doesn't support it\n");
            return;
        }

        // We haven't implemented flushing for frame-threaded encoders.
        av_assert0(!(caps & AV_CODEC_CAP_FRAME_THREADS));
    }

    avci->draining      = 0;
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

void avsubtitle_free(AVSubtitle *sub)
{
    int i;

    for (i = 0; i < sub->num_rects; i++) {
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

int avcodec_close(AVCodecContext *avctx)
{
    int i;

    if (!avctx)
        return 0;

    if (avcodec_is_open(avctx)) {
        if (CONFIG_FRAME_THREAD_ENCODER &&
            avctx->internal->frame_thread_encoder && avctx->thread_count > 1) {
            ff_frame_thread_encoder_free(avctx);
        }
        if (HAVE_THREADS && avctx->internal->thread_ctx)
            ff_thread_free(avctx);
        if (avctx->codec && avctx->codec->close)
            avctx->codec->close(avctx);
        avctx->internal->byte_buffer_size = 0;
        av_freep(&avctx->internal->byte_buffer);
        av_frame_free(&avctx->internal->to_free);
        av_frame_free(&avctx->internal->compat_decode_frame);
        av_frame_free(&avctx->internal->buffer_frame);
        av_packet_free(&avctx->internal->compat_encode_packet);
        av_packet_free(&avctx->internal->buffer_pkt);
        av_packet_free(&avctx->internal->last_pkt_props);
        avpriv_packet_list_free(&avctx->internal->pkt_props,
                                &avctx->internal->pkt_props_tail);

        av_packet_free(&avctx->internal->ds.in_pkt);
        av_frame_free(&avctx->internal->es.in_frame);

        av_buffer_unref(&avctx->internal->pool);

        if (avctx->hwaccel && avctx->hwaccel->uninit)
            avctx->hwaccel->uninit(avctx);
        av_freep(&avctx->internal->hwaccel_priv_data);

        av_bsf_free(&avctx->internal->bsf);

        av_freep(&avctx->internal);
    }

    for (i = 0; i < avctx->nb_coded_side_data; i++)
        av_freep(&avctx->coded_side_data[i].data);
    av_freep(&avctx->coded_side_data);
    avctx->nb_coded_side_data = 0;

    av_buffer_unref(&avctx->hw_frames_ctx);
    av_buffer_unref(&avctx->hw_device_ctx);

    if (avctx->priv_data && avctx->codec && avctx->codec->priv_class)
        av_opt_free(avctx->priv_data);
    av_opt_free(avctx);
    av_freep(&avctx->priv_data);
    if (av_codec_is_encoder(avctx->codec)) {
        av_freep(&avctx->extradata);
#if FF_API_CODED_FRAME
        av_frame_free(&avctx->coded_frame);
#endif
    }
    avctx->codec = NULL;
    avctx->active_thread_type = 0;

    return 0;
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

#if FF_API_TAG_STRING
size_t av_get_codec_tag_string(char *buf, size_t buf_size, unsigned int codec_tag)
{
    int i, len, ret = 0;

#define TAG_PRINT(x)                                              \
    (((x) >= '0' && (x) <= '9') ||                                \
     ((x) >= 'a' && (x) <= 'z') || ((x) >= 'A' && (x) <= 'Z') ||  \
     ((x) == '.' || (x) == ' ' || (x) == '-' || (x) == '_'))

    for (i = 0; i < 4; i++) {
        len = snprintf(buf, buf_size,
                       TAG_PRINT(codec_tag & 0xFF) ? "%c" : "[%d]", codec_tag & 0xFF);
        buf        += len;
        buf_size    = buf_size > len ? buf_size - len : 0;
        ret        += len;
        codec_tag >>= 8;
    }
    return ret;
}
#endif

void avcodec_string(char *buf, int buf_size, AVCodecContext *enc, int encode)
{
    const char *codec_type;
    const char *codec_name;
    const char *profile = NULL;
    int64_t bitrate;
    int new_line = 0;
    AVRational display_aspect_ratio;
    const char *separator = enc->dump_separator ? (const char *)enc->dump_separator : ", ";

    if (!buf || buf_size <= 0)
        return;
    codec_type = av_get_media_type_string(enc->codec_type);
    codec_name = avcodec_get_name(enc->codec_id);
    profile = avcodec_profile_name(enc->codec_id, enc->profile);

    snprintf(buf, buf_size, "%s: %s", codec_type ? codec_type : "unknown",
             codec_name);
    buf[0] ^= 'a' ^ 'A'; /* first letter in uppercase */

    if (enc->codec && strcmp(enc->codec->name, codec_name))
        snprintf(buf + strlen(buf), buf_size - strlen(buf), " (%s)", enc->codec->name);

    if (profile)
        snprintf(buf + strlen(buf), buf_size - strlen(buf), " (%s)", profile);
    if (   enc->codec_type == AVMEDIA_TYPE_VIDEO
        && av_log_get_level() >= AV_LOG_VERBOSE
        && enc->refs)
        snprintf(buf + strlen(buf), buf_size - strlen(buf),
                 ", %d reference frame%s",
                 enc->refs, enc->refs > 1 ? "s" : "");

    if (enc->codec_tag)
        snprintf(buf + strlen(buf), buf_size - strlen(buf), " (%s / 0x%04X)",
                 av_fourcc2str(enc->codec_tag), enc->codec_tag);

    switch (enc->codec_type) {
    case AVMEDIA_TYPE_VIDEO:
        {
            char detail[256] = "(";

            av_strlcat(buf, separator, buf_size);

            snprintf(buf + strlen(buf), buf_size - strlen(buf),
                 "%s", enc->pix_fmt == AV_PIX_FMT_NONE ? "none" :
                     av_get_pix_fmt_name(enc->pix_fmt));
            if (enc->bits_per_raw_sample && enc->pix_fmt != AV_PIX_FMT_NONE &&
                enc->bits_per_raw_sample < av_pix_fmt_desc_get(enc->pix_fmt)->comp[0].depth)
                av_strlcatf(detail, sizeof(detail), "%d bpc, ", enc->bits_per_raw_sample);
            if (enc->color_range != AVCOL_RANGE_UNSPECIFIED)
                av_strlcatf(detail, sizeof(detail), "%s, ",
                            av_color_range_name(enc->color_range));

            if (enc->colorspace != AVCOL_SPC_UNSPECIFIED ||
                enc->color_primaries != AVCOL_PRI_UNSPECIFIED ||
                enc->color_trc != AVCOL_TRC_UNSPECIFIED) {
                if (enc->colorspace != (int)enc->color_primaries ||
                    enc->colorspace != (int)enc->color_trc) {
                    new_line = 1;
                    av_strlcatf(detail, sizeof(detail), "%s/%s/%s, ",
                                av_color_space_name(enc->colorspace),
                                av_color_primaries_name(enc->color_primaries),
                                av_color_transfer_name(enc->color_trc));
                } else
                    av_strlcatf(detail, sizeof(detail), "%s, ",
                                av_get_colorspace_name(enc->colorspace));
            }

            if (enc->field_order != AV_FIELD_UNKNOWN) {
                const char *field_order = "progressive";
                if (enc->field_order == AV_FIELD_TT)
                    field_order = "top first";
                else if (enc->field_order == AV_FIELD_BB)
                    field_order = "bottom first";
                else if (enc->field_order == AV_FIELD_TB)
                    field_order = "top coded first (swapped)";
                else if (enc->field_order == AV_FIELD_BT)
                    field_order = "bottom coded first (swapped)";

                av_strlcatf(detail, sizeof(detail), "%s, ", field_order);
            }

            if (av_log_get_level() >= AV_LOG_VERBOSE &&
                enc->chroma_sample_location != AVCHROMA_LOC_UNSPECIFIED)
                av_strlcatf(detail, sizeof(detail), "%s, ",
                            av_chroma_location_name(enc->chroma_sample_location));

            if (strlen(detail) > 1) {
                detail[strlen(detail) - 2] = 0;
                av_strlcatf(buf, buf_size, "%s)", detail);
            }
        }

        if (enc->width) {
            av_strlcat(buf, new_line ? separator : ", ", buf_size);

            snprintf(buf + strlen(buf), buf_size - strlen(buf),
                     "%dx%d",
                     enc->width, enc->height);

            if (av_log_get_level() >= AV_LOG_VERBOSE &&
                (enc->width != enc->coded_width ||
                 enc->height != enc->coded_height))
                snprintf(buf + strlen(buf), buf_size - strlen(buf),
                         " (%dx%d)", enc->coded_width, enc->coded_height);

            if (enc->sample_aspect_ratio.num) {
                av_reduce(&display_aspect_ratio.num, &display_aspect_ratio.den,
                          enc->width * (int64_t)enc->sample_aspect_ratio.num,
                          enc->height * (int64_t)enc->sample_aspect_ratio.den,
                          1024 * 1024);
                snprintf(buf + strlen(buf), buf_size - strlen(buf),
                         " [SAR %d:%d DAR %d:%d]",
                         enc->sample_aspect_ratio.num, enc->sample_aspect_ratio.den,
                         display_aspect_ratio.num, display_aspect_ratio.den);
            }
            if (av_log_get_level() >= AV_LOG_DEBUG) {
                int g = av_gcd(enc->time_base.num, enc->time_base.den);
                snprintf(buf + strlen(buf), buf_size - strlen(buf),
                         ", %d/%d",
                         enc->time_base.num / g, enc->time_base.den / g);
            }
        }
        if (encode) {
            snprintf(buf + strlen(buf), buf_size - strlen(buf),
                     ", q=%d-%d", enc->qmin, enc->qmax);
        } else {
            if (enc->properties & FF_CODEC_PROPERTY_CLOSED_CAPTIONS)
                snprintf(buf + strlen(buf), buf_size - strlen(buf),
                         ", Closed Captions");
            if (enc->properties & FF_CODEC_PROPERTY_LOSSLESS)
                snprintf(buf + strlen(buf), buf_size - strlen(buf),
                         ", lossless");
        }
        break;
    case AVMEDIA_TYPE_AUDIO:
        av_strlcat(buf, separator, buf_size);

        if (enc->sample_rate) {
            snprintf(buf + strlen(buf), buf_size - strlen(buf),
                     "%d Hz, ", enc->sample_rate);
        }
        av_get_channel_layout_string(buf + strlen(buf), buf_size - strlen(buf), enc->channels, enc->channel_layout);
        if (enc->sample_fmt != AV_SAMPLE_FMT_NONE) {
            snprintf(buf + strlen(buf), buf_size - strlen(buf),
                     ", %s", av_get_sample_fmt_name(enc->sample_fmt));
        }
        if (   enc->bits_per_raw_sample > 0
            && enc->bits_per_raw_sample != av_get_bytes_per_sample(enc->sample_fmt) * 8)
            snprintf(buf + strlen(buf), buf_size - strlen(buf),
                     " (%d bit)", enc->bits_per_raw_sample);
        if (av_log_get_level() >= AV_LOG_VERBOSE) {
            if (enc->initial_padding)
                snprintf(buf + strlen(buf), buf_size - strlen(buf),
                         ", delay %d", enc->initial_padding);
            if (enc->trailing_padding)
                snprintf(buf + strlen(buf), buf_size - strlen(buf),
                         ", padding %d", enc->trailing_padding);
        }
        break;
    case AVMEDIA_TYPE_DATA:
        if (av_log_get_level() >= AV_LOG_DEBUG) {
            int g = av_gcd(enc->time_base.num, enc->time_base.den);
            if (g)
                snprintf(buf + strlen(buf), buf_size - strlen(buf),
                         ", %d/%d",
                         enc->time_base.num / g, enc->time_base.den / g);
        }
        break;
    case AVMEDIA_TYPE_SUBTITLE:
        if (enc->width)
            snprintf(buf + strlen(buf), buf_size - strlen(buf),
                     ", %dx%d", enc->width, enc->height);
        break;
    default:
        return;
    }
    if (encode) {
        if (enc->flags & AV_CODEC_FLAG_PASS1)
            snprintf(buf + strlen(buf), buf_size - strlen(buf),
                     ", pass 1");
        if (enc->flags & AV_CODEC_FLAG_PASS2)
            snprintf(buf + strlen(buf), buf_size - strlen(buf),
                     ", pass 2");
    }
    bitrate = get_bit_rate(enc);
    if (bitrate != 0) {
        snprintf(buf + strlen(buf), buf_size - strlen(buf),
                 ", %"PRId64" kb/s", bitrate / 1000);
    } else if (enc->rc_max_rate > 0) {
        snprintf(buf + strlen(buf), buf_size - strlen(buf),
                 ", max. %"PRId64" kb/s", enc->rc_max_rate / 1000);
    }
}

const char *av_get_profile_name(const AVCodec *codec, int profile)
{
    const AVProfile *p;
    if (profile == FF_PROFILE_UNKNOWN || !codec->profiles)
        return NULL;

    for (p = codec->profiles; p->profile != FF_PROFILE_UNKNOWN; p++)
        if (p->profile == profile)
            return p->name;

    return NULL;
}

const char *avcodec_profile_name(enum AVCodecID codec_id, int profile)
{
    const AVCodecDescriptor *desc = avcodec_descriptor_get(codec_id);
    const AVProfile *p;

    if (profile == FF_PROFILE_UNKNOWN || !desc || !desc->profiles)
        return NULL;

    for (p = desc->profiles; p->profile != FF_PROFILE_UNKNOWN; p++)
        if (p->profile == profile)
            return p->name;

    return NULL;
}

unsigned avcodec_version(void)
{
    av_assert0(AV_CODEC_ID_PCM_S8_PLANAR==65563);
    av_assert0(AV_CODEC_ID_ADPCM_G722==69660);
    av_assert0(AV_CODEC_ID_SRT==94216);
    av_assert0(LIBAVCODEC_VERSION_MICRO >= 100);

    return LIBAVCODEC_VERSION_INT;
}

const char *avcodec_configuration(void)
{
    return FFMPEG_CONFIGURATION;
}
#define AVCODEC_LICENSE_PREFIX "libavcodec license: "

const char *avcodec_license(void)
{
    return &AVCODEC_LICENSE_PREFIX FFMPEG_LICENSE[sizeof(AVCODEC_LICENSE_PREFIX) - 1];
}

int av_get_exact_bits_per_sample(enum AVCodecID codec_id)
{
    switch (codec_id) {
    case AV_CODEC_ID_8SVX_EXP:
    case AV_CODEC_ID_8SVX_FIB:
    case AV_CODEC_ID_ADPCM_ARGO:
    case AV_CODEC_ID_ADPCM_CT:
    case AV_CODEC_ID_ADPCM_IMA_APC:
    case AV_CODEC_ID_ADPCM_IMA_APM:
    case AV_CODEC_ID_ADPCM_IMA_EA_SEAD:
    case AV_CODEC_ID_ADPCM_IMA_OKI:
    case AV_CODEC_ID_ADPCM_IMA_WS:
    case AV_CODEC_ID_ADPCM_IMA_SSI:
    case AV_CODEC_ID_ADPCM_G722:
    case AV_CODEC_ID_ADPCM_YAMAHA:
    case AV_CODEC_ID_ADPCM_AICA:
        return 4;
    case AV_CODEC_ID_DSD_LSBF:
    case AV_CODEC_ID_DSD_MSBF:
    case AV_CODEC_ID_DSD_LSBF_PLANAR:
    case AV_CODEC_ID_DSD_MSBF_PLANAR:
    case AV_CODEC_ID_PCM_ALAW:
    case AV_CODEC_ID_PCM_MULAW:
    case AV_CODEC_ID_PCM_VIDC:
    case AV_CODEC_ID_PCM_S8:
    case AV_CODEC_ID_PCM_S8_PLANAR:
    case AV_CODEC_ID_PCM_U8:
    case AV_CODEC_ID_SDX2_DPCM:
    case AV_CODEC_ID_DERF_DPCM:
        return 8;
    case AV_CODEC_ID_PCM_S16BE:
    case AV_CODEC_ID_PCM_S16BE_PLANAR:
    case AV_CODEC_ID_PCM_S16LE:
    case AV_CODEC_ID_PCM_S16LE_PLANAR:
    case AV_CODEC_ID_PCM_U16BE:
    case AV_CODEC_ID_PCM_U16LE:
        return 16;
    case AV_CODEC_ID_PCM_S24DAUD:
    case AV_CODEC_ID_PCM_S24BE:
    case AV_CODEC_ID_PCM_S24LE:
    case AV_CODEC_ID_PCM_S24LE_PLANAR:
    case AV_CODEC_ID_PCM_U24BE:
    case AV_CODEC_ID_PCM_U24LE:
        return 24;
    case AV_CODEC_ID_PCM_S32BE:
    case AV_CODEC_ID_PCM_S32LE:
    case AV_CODEC_ID_PCM_S32LE_PLANAR:
    case AV_CODEC_ID_PCM_U32BE:
    case AV_CODEC_ID_PCM_U32LE:
    case AV_CODEC_ID_PCM_F32BE:
    case AV_CODEC_ID_PCM_F32LE:
    case AV_CODEC_ID_PCM_F24LE:
    case AV_CODEC_ID_PCM_F16LE:
        return 32;
    case AV_CODEC_ID_PCM_F64BE:
    case AV_CODEC_ID_PCM_F64LE:
    case AV_CODEC_ID_PCM_S64BE:
    case AV_CODEC_ID_PCM_S64LE:
        return 64;
    default:
        return 0;
    }
}

enum AVCodecID av_get_pcm_codec(enum AVSampleFormat fmt, int be)
{
    static const enum AVCodecID map[][2] = {
        [AV_SAMPLE_FMT_U8  ] = { AV_CODEC_ID_PCM_U8,    AV_CODEC_ID_PCM_U8    },
        [AV_SAMPLE_FMT_S16 ] = { AV_CODEC_ID_PCM_S16LE, AV_CODEC_ID_PCM_S16BE },
        [AV_SAMPLE_FMT_S32 ] = { AV_CODEC_ID_PCM_S32LE, AV_CODEC_ID_PCM_S32BE },
        [AV_SAMPLE_FMT_FLT ] = { AV_CODEC_ID_PCM_F32LE, AV_CODEC_ID_PCM_F32BE },
        [AV_SAMPLE_FMT_DBL ] = { AV_CODEC_ID_PCM_F64LE, AV_CODEC_ID_PCM_F64BE },
        [AV_SAMPLE_FMT_U8P ] = { AV_CODEC_ID_PCM_U8,    AV_CODEC_ID_PCM_U8    },
        [AV_SAMPLE_FMT_S16P] = { AV_CODEC_ID_PCM_S16LE, AV_CODEC_ID_PCM_S16BE },
        [AV_SAMPLE_FMT_S32P] = { AV_CODEC_ID_PCM_S32LE, AV_CODEC_ID_PCM_S32BE },
        [AV_SAMPLE_FMT_S64P] = { AV_CODEC_ID_PCM_S64LE, AV_CODEC_ID_PCM_S64BE },
        [AV_SAMPLE_FMT_FLTP] = { AV_CODEC_ID_PCM_F32LE, AV_CODEC_ID_PCM_F32BE },
        [AV_SAMPLE_FMT_DBLP] = { AV_CODEC_ID_PCM_F64LE, AV_CODEC_ID_PCM_F64BE },
    };
    if (fmt < 0 || fmt >= FF_ARRAY_ELEMS(map))
        return AV_CODEC_ID_NONE;
    if (be < 0 || be > 1)
        be = AV_NE(1, 0);
    return map[fmt][be];
}

int av_get_bits_per_sample(enum AVCodecID codec_id)
{
    switch (codec_id) {
    case AV_CODEC_ID_ADPCM_SBPRO_2:
        return 2;
    case AV_CODEC_ID_ADPCM_SBPRO_3:
        return 3;
    case AV_CODEC_ID_ADPCM_SBPRO_4:
    case AV_CODEC_ID_ADPCM_IMA_WAV:
    case AV_CODEC_ID_ADPCM_IMA_QT:
    case AV_CODEC_ID_ADPCM_SWF:
    case AV_CODEC_ID_ADPCM_MS:
        return 4;
    default:
        return av_get_exact_bits_per_sample(codec_id);
    }
}

static int get_audio_frame_duration(enum AVCodecID id, int sr, int ch, int ba,
                                    uint32_t tag, int bits_per_coded_sample, int64_t bitrate,
                                    uint8_t * extradata, int frame_size, int frame_bytes)
{
    int bps = av_get_exact_bits_per_sample(id);
    int framecount = (ba > 0 && frame_bytes / ba > 0) ? frame_bytes / ba : 1;

    /* codecs with an exact constant bits per sample */
    if (bps > 0 && ch > 0 && frame_bytes > 0 && ch < 32768 && bps < 32768)
        return (frame_bytes * 8LL) / (bps * ch);
    bps = bits_per_coded_sample;

    /* codecs with a fixed packet duration */
    switch (id) {
    case AV_CODEC_ID_ADPCM_ADX:    return   32;
    case AV_CODEC_ID_ADPCM_IMA_QT: return   64;
    case AV_CODEC_ID_ADPCM_EA_XAS: return  128;
    case AV_CODEC_ID_AMR_NB:
    case AV_CODEC_ID_EVRC:
    case AV_CODEC_ID_GSM:
    case AV_CODEC_ID_QCELP:
    case AV_CODEC_ID_RA_288:       return  160;
    case AV_CODEC_ID_AMR_WB:
    case AV_CODEC_ID_GSM_MS:       return  320;
    case AV_CODEC_ID_MP1:          return  384;
    case AV_CODEC_ID_ATRAC1:       return  512;
    case AV_CODEC_ID_ATRAC9:
    case AV_CODEC_ID_ATRAC3:
        if (framecount > INT_MAX/1024)
            return 0;
        return 1024 * framecount;
    case AV_CODEC_ID_ATRAC3P:      return 2048;
    case AV_CODEC_ID_MP2:
    case AV_CODEC_ID_MUSEPACK7:    return 1152;
    case AV_CODEC_ID_AC3:          return 1536;
    }

    if (sr > 0) {
        /* calc from sample rate */
        if (id == AV_CODEC_ID_TTA)
            return 256 * sr / 245;
        else if (id == AV_CODEC_ID_DST)
            return 588 * sr / 44100;

        if (ch > 0) {
            /* calc from sample rate and channels */
            if (id == AV_CODEC_ID_BINKAUDIO_DCT)
                return (480 << (sr / 22050)) / ch;
        }

        if (id == AV_CODEC_ID_MP3)
            return sr <= 24000 ? 576 : 1152;
    }

    if (ba > 0) {
        /* calc from block_align */
        if (id == AV_CODEC_ID_SIPR) {
            switch (ba) {
            case 20: return 160;
            case 19: return 144;
            case 29: return 288;
            case 37: return 480;
            }
        } else if (id == AV_CODEC_ID_ILBC) {
            switch (ba) {
            case 38: return 160;
            case 50: return 240;
            }
        }
    }

    if (frame_bytes > 0) {
        /* calc from frame_bytes only */
        if (id == AV_CODEC_ID_TRUESPEECH)
            return 240 * (frame_bytes / 32);
        if (id == AV_CODEC_ID_NELLYMOSER)
            return 256 * (frame_bytes / 64);
        if (id == AV_CODEC_ID_RA_144)
            return 160 * (frame_bytes / 20);

        if (bps > 0) {
            /* calc from frame_bytes and bits_per_coded_sample */
            if (id == AV_CODEC_ID_ADPCM_G726 || id == AV_CODEC_ID_ADPCM_G726LE)
                return frame_bytes * 8 / bps;
        }

        if (ch > 0 && ch < INT_MAX/16) {
            /* calc from frame_bytes and channels */
            switch (id) {
            case AV_CODEC_ID_FASTAUDIO:
                return frame_bytes / (40 * ch) * 256;
            case AV_CODEC_ID_ADPCM_IMA_MOFLEX:
                return (frame_bytes - 4 * ch) / (128 * ch) * 256;
            case AV_CODEC_ID_ADPCM_AFC:
                return frame_bytes / (9 * ch) * 16;
            case AV_CODEC_ID_ADPCM_PSX:
            case AV_CODEC_ID_ADPCM_DTK:
                return frame_bytes / (16 * ch) * 28;
            case AV_CODEC_ID_ADPCM_4XM:
            case AV_CODEC_ID_ADPCM_IMA_DAT4:
            case AV_CODEC_ID_ADPCM_IMA_ISS:
                return (frame_bytes - 4 * ch) * 2 / ch;
            case AV_CODEC_ID_ADPCM_IMA_SMJPEG:
                return (frame_bytes - 4) * 2 / ch;
            case AV_CODEC_ID_ADPCM_IMA_AMV:
                return (frame_bytes - 8) * 2 / ch;
            case AV_CODEC_ID_ADPCM_THP:
            case AV_CODEC_ID_ADPCM_THP_LE:
                if (extradata)
                    return frame_bytes * 14 / (8 * ch);
                break;
            case AV_CODEC_ID_ADPCM_XA:
                return (frame_bytes / 128) * 224 / ch;
            case AV_CODEC_ID_INTERPLAY_DPCM:
                return (frame_bytes - 6 - ch) / ch;
            case AV_CODEC_ID_ROQ_DPCM:
                return (frame_bytes - 8) / ch;
            case AV_CODEC_ID_XAN_DPCM:
                return (frame_bytes - 2 * ch) / ch;
            case AV_CODEC_ID_MACE3:
                return 3 * frame_bytes / ch;
            case AV_CODEC_ID_MACE6:
                return 6 * frame_bytes / ch;
            case AV_CODEC_ID_PCM_LXF:
                return 2 * (frame_bytes / (5 * ch));
            case AV_CODEC_ID_IAC:
            case AV_CODEC_ID_IMC:
                return 4 * frame_bytes / ch;
            }

            if (tag) {
                /* calc from frame_bytes, channels, and codec_tag */
                if (id == AV_CODEC_ID_SOL_DPCM) {
                    if (tag == 3)
                        return frame_bytes / ch;
                    else
                        return frame_bytes * 2 / ch;
                }
            }

            if (ba > 0) {
                /* calc from frame_bytes, channels, and block_align */
                int blocks = frame_bytes / ba;
                switch (id) {
                case AV_CODEC_ID_ADPCM_IMA_WAV:
                    if (bps < 2 || bps > 5)
                        return 0;
                    return blocks * (1 + (ba - 4 * ch) / (bps * ch) * 8);
                case AV_CODEC_ID_ADPCM_IMA_DK3:
                    return blocks * (((ba - 16) * 2 / 3 * 4) / ch);
                case AV_CODEC_ID_ADPCM_IMA_DK4:
                    return blocks * (1 + (ba - 4 * ch) * 2 / ch);
                case AV_CODEC_ID_ADPCM_IMA_RAD:
                    return blocks * ((ba - 4 * ch) * 2 / ch);
                case AV_CODEC_ID_ADPCM_MS:
                    return blocks * (2 + (ba - 7 * ch) * 2 / ch);
                case AV_CODEC_ID_ADPCM_MTAF:
                    return blocks * (ba - 16) * 2 / ch;
                }
            }

            if (bps > 0) {
                /* calc from frame_bytes, channels, and bits_per_coded_sample */
                switch (id) {
                case AV_CODEC_ID_PCM_DVD:
                    if(bps<4 || frame_bytes<3)
                        return 0;
                    return 2 * ((frame_bytes - 3) / ((bps * 2 / 8) * ch));
                case AV_CODEC_ID_PCM_BLURAY:
                    if(bps<4 || frame_bytes<4)
                        return 0;
                    return (frame_bytes - 4) / ((FFALIGN(ch, 2) * bps) / 8);
                case AV_CODEC_ID_S302M:
                    return 2 * (frame_bytes / ((bps + 4) / 4)) / ch;
                }
            }
        }
    }

    /* Fall back on using frame_size */
    if (frame_size > 1 && frame_bytes)
        return frame_size;

    //For WMA we currently have no other means to calculate duration thus we
    //do it here by assuming CBR, which is true for all known cases.
    if (bitrate > 0 && frame_bytes > 0 && sr > 0 && ba > 1) {
        if (id == AV_CODEC_ID_WMAV1 || id == AV_CODEC_ID_WMAV2)
            return  (frame_bytes * 8LL * sr) / bitrate;
    }

    return 0;
}

int av_get_audio_frame_duration(AVCodecContext *avctx, int frame_bytes)
{
    return get_audio_frame_duration(avctx->codec_id, avctx->sample_rate,
                                    avctx->channels, avctx->block_align,
                                    avctx->codec_tag, avctx->bits_per_coded_sample,
                                    avctx->bit_rate, avctx->extradata, avctx->frame_size,
                                    frame_bytes);
}

int av_get_audio_frame_duration2(AVCodecParameters *par, int frame_bytes)
{
    return get_audio_frame_duration(par->codec_id, par->sample_rate,
                                    par->channels, par->block_align,
                                    par->codec_tag, par->bits_per_coded_sample,
                                    par->bit_rate, par->extradata, par->frame_size,
                                    frame_bytes);
}


unsigned int av_xiphlacing(unsigned char *s, unsigned int v)
{
    unsigned int n = 0;

    while (v >= 0xff) {
        *s++ = 0xff;
        v -= 0xff;
        n++;
    }
    *s = v;
    n++;
    return n;
}

int ff_match_2uint16(const uint16_t(*tab)[2], int size, int a, int b)
{
    int i;
    for (i = 0; i < size && !(tab[i][0] == a && tab[i][1] == b); i++) ;
    return i;
}

const AVCodecHWConfig *avcodec_get_hw_config(const AVCodec *codec, int index)
{
    int i;
    if (!codec->hw_configs || index < 0)
        return NULL;
    for (i = 0; i <= index; i++)
        if (!codec->hw_configs[i])
            return NULL;
    return &codec->hw_configs[index]->public;
}

#if FF_API_USER_VISIBLE_AVHWACCEL
AVHWAccel *av_hwaccel_next(const AVHWAccel *hwaccel)
{
    return NULL;
}

void av_register_hwaccel(AVHWAccel *hwaccel)
{
}
#endif

#if FF_API_LOCKMGR
int av_lockmgr_register(int (*cb)(void **mutex, enum AVLockOp op))
{
    return 0;
}
#endif

unsigned int avpriv_toupper4(unsigned int x)
{
    return av_toupper(x & 0xFF) +
          (av_toupper((x >>  8) & 0xFF) << 8)  +
          (av_toupper((x >> 16) & 0xFF) << 16) +
((unsigned)av_toupper((x >> 24) & 0xFF) << 24);
}


void ff_thread_release_buffer(AVCodecContext *avctx, ThreadFrame *f)
{
    if (f->f)
        av_frame_unref(f->f);
}
int ff_thread_ref_frame(ThreadFrame *dst, const ThreadFrame *src)
{
    int ret;

    dst->owner[0] = src->owner[0];
    dst->owner[1] = src->owner[1];

    ret = av_frame_ref(dst->f, src->f);
    if (ret < 0)
        return ret;

    av_assert0(!dst->progress);

    if (src->progress &&
        !(dst->progress = av_buffer_ref(src->progress))) {
        ff_thread_release_buffer(dst->owner[0], dst);
        return AVERROR(ENOMEM);
    }

    return 0;
}

#if !HAVE_THREADS

enum AVPixelFormat ff_thread_get_format(AVCodecContext *avctx, const enum AVPixelFormat *fmt)
{
    return ff_get_format(avctx, fmt);
}

int ff_thread_get_buffer(AVCodecContext *avctx, ThreadFrame *f, int flags)
{
    f->owner[0] = f->owner[1] = avctx;
    return ff_get_buffer(avctx, f->f, flags);
}



void ff_thread_finish_setup(AVCodecContext *avctx)
{
}

void ff_thread_report_progress(ThreadFrame *f, int progress, int field)
{
}

void ff_thread_await_progress(ThreadFrame *f, int progress, int field)
{
}

int ff_thread_can_start_frame(AVCodecContext *avctx)
{
    return 1;
}

int ff_alloc_entries(AVCodecContext *avctx, int count)
{
    return 0;
}

void ff_reset_entries(AVCodecContext *avctx)
{
}

void ff_thread_await_progress2(AVCodecContext *avctx, int field, int thread, int shift)
{
}

void ff_thread_report_progress2(AVCodecContext *avctx, int field, int thread, int n)
{
}

#endif

int avcodec_is_open(AVCodecContext *s)
{
    return !!s->internal;
}

int avpriv_bprint_to_extradata(AVCodecContext *avctx, struct AVBPrint *buf)
{
    int ret;
    char *str;

    ret = av_bprint_finalize(buf, &str);
    if (ret < 0)
        return ret;
    if (!av_bprint_is_complete(buf)) {
        av_free(str);
        return AVERROR(ENOMEM);
    }

    avctx->extradata = str;
    /* Note: the string is NUL terminated (so extradata can be read as a
     * string), but the ending character is not accounted in the size (in
     * binary formats you are likely not supposed to mux that character). When
     * extradata is copied, it is also padded with AV_INPUT_BUFFER_PADDING_SIZE
     * zeros. */
    avctx->extradata_size = buf->len;
    return 0;
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

AVCPBProperties *av_cpb_properties_alloc(size_t *size)
{
    AVCPBProperties *props = av_mallocz(sizeof(AVCPBProperties));
    if (!props)
        return NULL;

    if (size)
        *size = sizeof(*props);

    props->vbv_delay = UINT64_MAX;

    return props;
}

AVCPBProperties *ff_add_cpb_side_data(AVCodecContext *avctx)
{
    AVPacketSideData *tmp;
    AVCPBProperties  *props;
    size_t size;
    int i;

    for (i = 0; i < avctx->nb_coded_side_data; i++)
        if (avctx->coded_side_data[i].type == AV_PKT_DATA_CPB_PROPERTIES)
            return (AVCPBProperties *)avctx->coded_side_data[i].data;

    props = av_cpb_properties_alloc(&size);
    if (!props)
        return NULL;

    tmp = av_realloc_array(avctx->coded_side_data, avctx->nb_coded_side_data + 1, sizeof(*tmp));
    if (!tmp) {
        av_freep(&props);
        return NULL;
    }

    avctx->coded_side_data = tmp;
    avctx->nb_coded_side_data++;

    avctx->coded_side_data[avctx->nb_coded_side_data - 1].type = AV_PKT_DATA_CPB_PROPERTIES;
    avctx->coded_side_data[avctx->nb_coded_side_data - 1].data = (uint8_t*)props;
    avctx->coded_side_data[avctx->nb_coded_side_data - 1].size = size;

    return props;
}



AVCodecParameters *avcodec_parameters_alloc(void)
{
    AVCodecParameters *par = av_mallocz(sizeof(*par));

    if (!par)
        return NULL;
    codec_parameters_reset(par);
    return par;
}

void avcodec_parameters_free(AVCodecParameters **ppar)
{
    AVCodecParameters *par = *ppar;

    if (!par)
        return;
    codec_parameters_reset(par);

    av_freep(ppar);
}

int avcodec_parameters_copy(AVCodecParameters *dst, const AVCodecParameters *src)
{
    codec_parameters_reset(dst);
    memcpy(dst, src, sizeof(*dst));

    dst->extradata      = NULL;
    dst->extradata_size = 0;
    if (src->extradata) {
        dst->extradata = av_mallocz(src->extradata_size + AV_INPUT_BUFFER_PADDING_SIZE);
        if (!dst->extradata)
            return AVERROR(ENOMEM);
        memcpy(dst->extradata, src->extradata, src->extradata_size);
        dst->extradata_size = src->extradata_size;
    }

    return 0;
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

static unsigned bcd2uint(uint8_t bcd)
{
    unsigned low  = bcd & 0xf;
    unsigned high = bcd >> 4;
    if (low > 9 || high > 9)
        return 0;
    return low + 10*high;
}

int ff_alloc_timecode_sei(const AVFrame *frame, AVRational rate, size_t prefix_len,
                     void **data, size_t *sei_size)
{
    AVFrameSideData *sd = NULL;
    uint8_t *sei_data;
    PutBitContext pb;
    uint32_t *tc;
    int m;

    if (frame)
        sd = av_frame_get_side_data(frame, AV_FRAME_DATA_S12M_TIMECODE);

    if (!sd) {
        *data = NULL;
        return 0;
    }
    tc =  (uint32_t*)sd->data;
    m  = tc[0] & 3;

    *sei_size = sizeof(uint32_t) * 4;
    *data = av_mallocz(*sei_size + prefix_len);
    if (!*data)
        return AVERROR(ENOMEM);
    sei_data = (uint8_t*)*data + prefix_len;

    init_put_bits(&pb, sei_data, *sei_size);
    put_bits(&pb, 2, m); // num_clock_ts

    for (int j = 1; j <= m; j++) {
        uint32_t tcsmpte = tc[j];
        unsigned hh   = bcd2uint(tcsmpte     & 0x3f);    // 6-bit hours
        unsigned mm   = bcd2uint(tcsmpte>>8  & 0x7f);    // 7-bit minutes
        unsigned ss   = bcd2uint(tcsmpte>>16 & 0x7f);    // 7-bit seconds
        unsigned ff   = bcd2uint(tcsmpte>>24 & 0x3f);    // 6-bit frames
        unsigned drop = tcsmpte & 1<<30 && !0;  // 1-bit drop if not arbitrary bit

        /* Calculate frame number of HEVC by SMPTE ST 12-1:2014 Sec 12.2 if rate > 30FPS */
        if (av_cmp_q(rate, (AVRational) {30, 1}) == 1) {
            unsigned pc;
            ff *= 2;
            if (av_cmp_q(rate, (AVRational) {50, 1}) == 0)
                pc = !!(tcsmpte & 1 << 7);
            else
                pc = !!(tcsmpte & 1 << 23);
            ff = (ff + pc) & 0x7f;
        }

        put_bits(&pb, 1, 1); // clock_timestamp_flag
        put_bits(&pb, 1, 1); // units_field_based_flag
        put_bits(&pb, 5, 0); // counting_type
        put_bits(&pb, 1, 1); // full_timestamp_flag
        put_bits(&pb, 1, 0); // discontinuity_flag
        put_bits(&pb, 1, drop);
        put_bits(&pb, 9, ff);
        put_bits(&pb, 6, ss);
        put_bits(&pb, 6, mm);
        put_bits(&pb, 5, hh);
        put_bits(&pb, 5, 0);
    }
    flush_put_bits(&pb);

    return 0;
}

int64_t ff_guess_coded_bitrate(AVCodecContext *avctx)
{
    AVRational framerate = avctx->framerate;
    int bits_per_coded_sample = avctx->bits_per_coded_sample;
    int64_t bitrate;

    if (!(framerate.num && framerate.den))
        framerate = av_inv_q(avctx->time_base);
    if (!(framerate.num && framerate.den))
        return 0;

    if (!bits_per_coded_sample) {
        const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(avctx->pix_fmt);
        bits_per_coded_sample = av_get_bits_per_pixel(desc);
    }
    bitrate = (int64_t)bits_per_coded_sample * avctx->width * avctx->height *
              framerate.num / framerate.den;

    return bitrate;
}

int ff_int_from_list_or_default(void *ctx, const char * val_name, int val,
                                const int * array_valid_values, int default_value)
{
    int i = 0, ref_val;

    while (1) {
        ref_val = array_valid_values[i];
        if (ref_val == INT_MAX)
            break;
        if (val == ref_val)
            return val;
        i++;
    }
    /* val is not a valid value */
    av_log(ctx, AV_LOG_DEBUG,
           "%s %d are not supported. Set to default value : %d\n", val_name, val, default_value);
    return default_value;
}

typedef void (*ff_parse_key_val_cb)(void *context, const char *key,
                                    int key_len, char **dest, int *dest_len);
void ff_parse_key_value(const char *str, ff_parse_key_val_cb callback_get_buf,
                        void *context)
{
    const char *ptr = str;

    /* Parse key=value pairs. */
    for (;;) {
        const char *key;
        char *dest = NULL, *dest_end;
        int key_len, dest_len = 0;

        /* Skip whitespace and potential commas. */
        while (*ptr && (av_isspace(*ptr) || *ptr == ','))
            ptr++;
        if (!*ptr)
            break;

        key = ptr;

        if (!(ptr = strchr(key, '=')))
            break;
        ptr++;
        key_len = ptr - key;

        callback_get_buf(context, key, key_len, &dest, &dest_len);
        dest_end = dest + dest_len - 1;

        if (*ptr == '\"') {
            ptr++;
            while (*ptr && *ptr != '\"') {
                if (*ptr == '\\') {
                    if (!ptr[1])
                        break;
                    if (dest && dest < dest_end)
                        *dest++ = ptr[1];
                    ptr += 2;
                } else {
                    if (dest && dest < dest_end)
                        *dest++ = *ptr;
                    ptr++;
                }
            }
            if (*ptr == '\"')
                ptr++;
        } else {
            for (; *ptr && !(av_isspace(*ptr) || *ptr == ','); ptr++)
                if (dest && dest < dest_end)
                    *dest++ = *ptr;
        }
        if (dest)
            *dest = 0;
    }
}

char *ff_data_to_hex(char *buff, const uint8_t *src, int s, int lowercase)
{
    int i;
    static const char hex_table_uc[16] = { '0', '1', '2', '3',
                                           '4', '5', '6', '7',
                                           '8', '9', 'A', 'B',
                                           'C', 'D', 'E', 'F' };
    static const char hex_table_lc[16] = { '0', '1', '2', '3',
                                           '4', '5', '6', '7',
                                           '8', '9', 'a', 'b',
                                           'c', 'd', 'e', 'f' };
    const char *hex_table = lowercase ? hex_table_lc : hex_table_uc;

    for (i = 0; i < s; i++) {
        buff[i * 2]     = hex_table[src[i] >> 4];
        buff[i * 2 + 1] = hex_table[src[i] & 0xF];
    }

    return buff;
}

char *ff_urldecode(const char *url, int decode_plus_sign)
{
    int s = 0, d = 0, url_len = 0;
    char c;
    char *dest = NULL;

    if (!url)
        return NULL;

    url_len = strlen(url) + 1;
    dest = av_malloc(url_len);

    if (!dest)
        return NULL;

    while (s < url_len) {
        c = url[s++];

        if (c == '%' && s + 2 < url_len) {
            char c2 = url[s++];
            char c3 = url[s++];
            if (av_isxdigit(c2) && av_isxdigit(c3)) {
                c2 = av_tolower(c2);
                c3 = av_tolower(c3);

                if (c2 <= '9')
                    c2 = c2 - '0';
                else
                    c2 = c2 - 'a' + 10;

                if (c3 <= '9')
                    c3 = c3 - '0';
                else
                    c3 = c3 - 'a' + 10;

                dest[d++] = 16 * c2 + c3;

            } else { /* %zz or something other invalid */
                dest[d++] = c;
                dest[d++] = c2;
                dest[d++] = c3;
            }
        } else if (c == '+' && decode_plus_sign) {
            dest[d++] = ' ';
        } else {
            dest[d++] = c;
        }

    }

    return dest;
}