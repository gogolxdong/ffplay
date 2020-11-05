#include "ffplay2.h"

static void *  worker(void *v);
void ff_frame_thread_encoder_free(AVCodecContext *avctx){
    int i;
    ThreadContext *c= avctx->internal->frame_thread_encoder;

    pthread_mutex_lock(&c->task_fifo_mutex);
    atomic_store(&c->exit, 1);
    pthread_cond_broadcast(&c->task_fifo_cond);
    pthread_mutex_unlock(&c->task_fifo_mutex);

    for (i=0; i<avctx->thread_count; i++) {
         pthread_join(c->worker[i], NULL);
    }

    while (av_fifo_size(c->task_fifo) > 0) {
        Task task;
        AVFrame *frame;
        av_fifo_generic_read(c->task_fifo, &task, sizeof(task), NULL);
        frame = task.indata;
        av_frame_free(&frame);
        task.indata = NULL;
    }

    for (i=0; i<BUFFER_SIZE; i++) {
        if (c->finished_tasks[i].outdata != NULL) {
            AVPacket *pkt = c->finished_tasks[i].outdata;
            av_packet_free(&pkt);
            c->finished_tasks[i].outdata = NULL;
        }
    }

    pthread_mutex_destroy(&c->task_fifo_mutex);
    pthread_mutex_destroy(&c->finished_task_mutex);
    pthread_mutex_destroy(&c->buffer_mutex);
    pthread_cond_destroy(&c->task_fifo_cond);
    pthread_cond_destroy(&c->finished_task_cond);
    av_fifo_freep(&c->task_fifo);
    av_freep(&avctx->internal->frame_thread_encoder);
}


int ff_frame_thread_encoder_init(AVCodecContext *avctx, AVDictionary *options){
    int i=0;
    ThreadContext *c;


    if(   !(avctx->thread_type & FF_THREAD_FRAME)
       || !(avctx->codec->capabilities & AV_CODEC_CAP_FRAME_THREADS))
        return 0;

    if(   !avctx->thread_count
       && avctx->codec_id == AV_CODEC_ID_MJPEG
       && !(avctx->flags & AV_CODEC_FLAG_QSCALE)) {
        av_log(avctx, AV_LOG_DEBUG,
               "Forcing thread count to 1 for MJPEG encoding, use -thread_type slice "
               "or a constant quantizer if you want to use multiple cpu cores\n");
        avctx->thread_count = 1;
    }
    if(   avctx->thread_count > 1
       && avctx->codec_id == AV_CODEC_ID_MJPEG
       && !(avctx->flags & AV_CODEC_FLAG_QSCALE))
        av_log(avctx, AV_LOG_WARNING,
               "MJPEG CBR encoding works badly with frame multi-threading, consider "
               "using -threads 1, -thread_type slice or a constant quantizer.\n");

    if (avctx->codec_id == AV_CODEC_ID_HUFFYUV ||
        avctx->codec_id == AV_CODEC_ID_FFVHUFF) {
        int warn = 0;
        int context_model = 0;
        AVDictionaryEntry *con = av_dict_get(options, "context", NULL, AV_DICT_MATCH_CASE);

        if (con && con->value)
            context_model = atoi(con->value);

        if (avctx->flags & AV_CODEC_FLAG_PASS1)
            warn = 1;
        else if(context_model > 0) {
            AVDictionaryEntry *t = av_dict_get(options, "non_deterministic",
                                               NULL, AV_DICT_MATCH_CASE);
            warn = !t || !t->value || !atoi(t->value) ? 1 : 0;
        }
        // huffyuv does not support these with multiple frame threads currently
        if (warn) {
            av_log(avctx, AV_LOG_WARNING,
               "Forcing thread count to 1 for huffyuv encoding with first pass or context 1\n");
            avctx->thread_count = 1;
        }
    }

    if(!avctx->thread_count) {
        avctx->thread_count = av_cpu_count();
        avctx->thread_count = FFMIN(avctx->thread_count, MAX_THREADS);
    }

    if(avctx->thread_count <= 1)
        return 0;

    if(avctx->thread_count > MAX_THREADS)
        return AVERROR(EINVAL);

    av_assert0(!avctx->internal->frame_thread_encoder);
    c = avctx->internal->frame_thread_encoder = av_mallocz(sizeof(ThreadContext));
    if(!c)
        return AVERROR(ENOMEM);

    c->parent_avctx = avctx;

    c->task_fifo = av_fifo_alloc_array(BUFFER_SIZE, sizeof(Task));
    if(!c->task_fifo)
        goto fail;

    pthread_mutex_init(&c->task_fifo_mutex, NULL);
    pthread_mutex_init(&c->finished_task_mutex, NULL);
    pthread_mutex_init(&c->buffer_mutex, NULL);
    pthread_cond_init(&c->task_fifo_cond, NULL);
    pthread_cond_init(&c->finished_task_cond, NULL);
    atomic_init(&c->exit, 0);

    for(i=0; i<avctx->thread_count ; i++){
        AVDictionary *tmp = NULL;
        int ret;
        void *tmpv;
        AVCodecContext *thread_avctx = avcodec_alloc_context3(avctx->codec);
        if(!thread_avctx)
            goto fail;
        tmpv = thread_avctx->priv_data;
        *thread_avctx = *avctx;
        ret = av_opt_copy(thread_avctx, avctx);
        if (ret < 0)
            goto fail;
        thread_avctx->priv_data = tmpv;
        thread_avctx->internal = NULL;
        if (avctx->codec->priv_class) {
            int ret = av_opt_copy(thread_avctx->priv_data, avctx->priv_data);
            if (ret < 0)
                goto fail;
        } else if (avctx->codec->priv_data_size) {
            memcpy(thread_avctx->priv_data, avctx->priv_data, avctx->codec->priv_data_size);
        }
        thread_avctx->thread_count = 1;
        thread_avctx->active_thread_type &= ~FF_THREAD_FRAME;

        av_dict_copy(&tmp, options, 0);
        av_dict_set(&tmp, "threads", "1", 0);
        if(avcodec_open2(thread_avctx, avctx->codec, &tmp) < 0) {
            av_dict_free(&tmp);
            goto fail;
        }
        av_dict_free(&tmp);
        av_assert0(!thread_avctx->internal->frame_thread_encoder);
        thread_avctx->internal->frame_thread_encoder = c;
        if(pthread_create(&c->worker[i], NULL, worker, thread_avctx)) {
            goto fail;
        }
    }

    avctx->active_thread_type = FF_THREAD_FRAME;

    return 0;
fail:
    avctx->thread_count = i;
    av_log(avctx, AV_LOG_ERROR, "ff_frame_thread_encoder_init failed\n");
    ff_frame_thread_encoder_free(avctx);
    return -1;
}
int ff_thread_init(AVCodecContext *s)
{
    return -1;
}


static void codec_parameters_reset(AVCodecParameters *par)
{
    av_freep(&par->extradata);

    memset(par, 0, sizeof(*par));

    par->codec_type          = AVMEDIA_TYPE_UNKNOWN;
    par->codec_id            = AV_CODEC_ID_NONE;
    par->format              = -1;
    par->field_order         = AV_FIELD_UNKNOWN;
    par->color_range         = AVCOL_RANGE_UNSPECIFIED;
    par->color_primaries     = AVCOL_PRI_UNSPECIFIED;
    par->color_trc           = AVCOL_TRC_UNSPECIFIED;
    par->color_space         = AVCOL_SPC_UNSPECIFIED;
    par->chroma_location     = AVCHROMA_LOC_UNSPECIFIED;
    par->sample_aspect_ratio = (AVRational){ 0, 1 };
    par->profile             = FF_PROFILE_UNKNOWN;
    par->level               = FF_LEVEL_UNKNOWN;
}


int avcodec_parameters_from_context(AVCodecParameters *par,
                                    const AVCodecContext *codec)
{
    codec_parameters_reset(par);

    par->codec_type = codec->codec_type;
    par->codec_id   = codec->codec_id;
    par->codec_tag  = codec->codec_tag;

    par->bit_rate              = codec->bit_rate;
    par->bits_per_coded_sample = codec->bits_per_coded_sample;
    par->bits_per_raw_sample   = codec->bits_per_raw_sample;
    par->profile               = codec->profile;
    par->level                 = codec->level;

    switch (par->codec_type) {
    case AVMEDIA_TYPE_VIDEO:
        par->format              = codec->pix_fmt;
        par->width               = codec->width;
        par->height              = codec->height;
        par->field_order         = codec->field_order;
        par->color_range         = codec->color_range;
        par->color_primaries     = codec->color_primaries;
        par->color_trc           = codec->color_trc;
        par->color_space         = codec->colorspace;
        par->chroma_location     = codec->chroma_sample_location;
        par->sample_aspect_ratio = codec->sample_aspect_ratio;
        par->video_delay         = codec->has_b_frames;
        break;
    case AVMEDIA_TYPE_AUDIO:
        par->format           = codec->sample_fmt;
        par->channel_layout   = codec->channel_layout;
        par->channels         = codec->channels;
        par->sample_rate      = codec->sample_rate;
        par->block_align      = codec->block_align;
        par->frame_size       = codec->frame_size;
        par->initial_padding  = codec->initial_padding;
        par->trailing_padding = codec->trailing_padding;
        par->seek_preroll     = codec->seek_preroll;
        break;
    case AVMEDIA_TYPE_SUBTITLE:
        par->width  = codec->width;
        par->height = codec->height;
        break;
    }

    if (codec->extradata) {
        par->extradata = av_mallocz(codec->extradata_size + AV_INPUT_BUFFER_PADDING_SIZE);
        if (!par->extradata)
            return AVERROR(ENOMEM);
        memcpy(par->extradata, codec->extradata, codec->extradata_size);
        par->extradata_size = codec->extradata_size;
    }

    return 0;
}

int ff_decode_bsfs_init(AVCodecContext *avctx)
{
    AVCodecInternal *avci = avctx->internal;
    int ret;

    if (avci->bsf)
        return 0;

    ret = av_bsf_list_parse_str(avctx->codec->bsfs, &avci->bsf);
    if (ret < 0) {
        av_log(avctx, AV_LOG_ERROR, "Error parsing decoder bitstream filters '%s': %s\n", avctx->codec->bsfs, av_err2str(ret));
        if (ret != AVERROR(ENOMEM))
            ret = AVERROR_BUG;
        goto fail;
    }

    
    avci->bsf->time_base_in = (AVRational){ 1, 90000 };
    ret = avcodec_parameters_from_context(avci->bsf->par_in, avctx);
    if (ret < 0)
        goto fail;

    ret = av_bsf_init(avci->bsf);
    if (ret < 0)
        goto fail;

    return 0;
fail:
    av_bsf_free(&avci->bsf);
    return ret;
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

void ff_slice_thread_free(AVCodecContext *avctx)
{
    SliceThreadContext *c = avctx->internal->thread_ctx;
    int i;

    avpriv_slicethread_free(&c->thread);

    for (i = 0; i < c->thread_count; i++) {
        pthread_mutex_destroy(&c->progress_mutex[i]);
        pthread_cond_destroy(&c->progress_cond[i]);
    }

    av_freep(&c->entries);
    av_freep(&c->progress_mutex);
    av_freep(&c->progress_cond);
    av_freep(&avctx->internal->thread_ctx);
}


static void async_lock(FrameThreadContext *fctx)
{
    pthread_mutex_lock(&fctx->async_mutex);
    while (fctx->async_lock)
        pthread_cond_wait(&fctx->async_cond, &fctx->async_mutex);
    fctx->async_lock = 1;
    pthread_mutex_unlock(&fctx->async_mutex);
}
static void async_unlock(FrameThreadContext *fctx)
{
    pthread_mutex_lock(&fctx->async_mutex);
    av_assert0(fctx->async_lock);
    fctx->async_lock = 0;
    pthread_cond_broadcast(&fctx->async_cond);
    pthread_mutex_unlock(&fctx->async_mutex);
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
int av_buffer_replace(AVBufferRef **pdst, AVBufferRef *src)
{
    AVBufferRef *dst = *pdst;
    AVBufferRef *tmp;

    if (!src) {
        av_buffer_unref(pdst);
        return 0;
    }

    if (dst && dst->buffer == src->buffer) {
        
        dst->data = src->data;
        dst->size = src->size;
        return 0;
    }

    tmp = av_buffer_ref(src);
    if (!tmp)
        return AVERROR(ENOMEM);

    av_buffer_unref(pdst);
    *pdst = tmp;
    return 0;
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
        dst->coded_frame = src->coded_frame;
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
        av_assert0(p->avctx->codec_type == AVMEDIA_TYPE_VIDEO ||
                   p->avctx->codec_type == AVMEDIA_TYPE_AUDIO);
        f = p->released_buffers[--p->num_released_buffers];
        f->extended_data = f->data;
        av_frame_unref(f);

        pthread_mutex_unlock(&fctx->buffer_mutex);
    }
}
void ff_frame_thread_free(AVCodecContext *avctx, int thread_count)
{
    FrameThreadContext *fctx = avctx->internal->thread_ctx;
    const AVCodec *codec = avctx->codec;
    int i, j;

    park_frame_worker_threads(fctx, thread_count);

    if (fctx->prev_thread && avctx->internal->hwaccel_priv_data !=
                             fctx->prev_thread->avctx->internal->hwaccel_priv_data) {
        if (update_context_from_thread(avctx, fctx->prev_thread->avctx, 1) < 0) {
            av_log(avctx, AV_LOG_ERROR, "Failed to update user thread.\n");
        }
    }

    if (fctx->prev_thread && fctx->prev_thread != fctx->threads)
        if (update_context_from_thread(fctx->threads->avctx, fctx->prev_thread->avctx, 0) < 0) {
            av_log(avctx, AV_LOG_ERROR, "Final thread update failed\n");
            fctx->prev_thread->avctx->internal->is_copy = fctx->threads->avctx->internal->is_copy;
            fctx->threads->avctx->internal->is_copy = 1;
        }

    for (i = 0; i < thread_count; i++) {
        PerThreadContext *p = &fctx->threads[i];

        pthread_mutex_lock(&p->mutex);
        p->die = 1;
        pthread_cond_signal(&p->input_cond);
        pthread_mutex_unlock(&p->mutex);

        if (p->thread_init)
            pthread_join(p->thread, NULL);
        p->thread_init=0;

        if (codec->close && p->avctx)
            codec->close(p->avctx);

        release_delayed_buffers(p);
        av_frame_free(&p->frame);
    }

    for (i = 0; i < thread_count; i++) {
        PerThreadContext *p = &fctx->threads[i];

        pthread_mutex_destroy(&p->mutex);
        pthread_mutex_destroy(&p->progress_mutex);
        pthread_cond_destroy(&p->input_cond);
        pthread_cond_destroy(&p->progress_cond);
        pthread_cond_destroy(&p->output_cond);
        av_packet_unref(&p->avpkt);

        for (j = 0; j < p->released_buffers_allocated; j++)
            av_frame_free(&p->released_buffers[j]);
        av_freep(&p->released_buffers);

        if (p->avctx) {
            if (codec->priv_class)
                av_opt_free(p->avctx->priv_data);
            av_freep(&p->avctx->priv_data);

            av_freep(&p->avctx->slice_offset);
        }

        if (p->avctx) {
            av_buffer_unref(&p->avctx->internal->pool);
            av_freep(&p->avctx->internal);
            av_buffer_unref(&p->avctx->hw_frames_ctx);
        }

        av_freep(&p->avctx);
    }

    av_freep(&fctx->threads);
    pthread_mutex_destroy(&fctx->buffer_mutex);
    pthread_mutex_destroy(&fctx->hwaccel_mutex);
    pthread_mutex_destroy(&fctx->async_mutex);
    pthread_cond_destroy(&fctx->async_cond);

    av_freep(&avctx->internal->thread_ctx);

    if (avctx->priv_data && avctx->codec && avctx->codec->priv_class)
        av_opt_free(avctx->priv_data);
    avctx->codec = NULL;
}
void ff_thread_free(AVCodecContext *avctx)
{
    if (avctx->active_thread_type&FF_THREAD_FRAME)
        ff_frame_thread_free(avctx, avctx->thread_count);
    else
        ff_slice_thread_free(avctx);
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
        p->got_frame = 0;
        av_frame_unref(p->frame);
        p->result = 0;

        release_delayed_buffers(p);

        if (avctx->codec->flush)
            avctx->codec->flush(p->avctx);
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



static const int BUF_BITS = 8 * sizeof(BitBuf);
static inline void init_put_bits(PutBitContext *s, uint8_t *buffer,
                                 int buffer_size)
{
    if (buffer_size < 0) {
        buffer_size = 0;
        buffer      = NULL;
    }

    s->size_in_bits = 8 * buffer_size;
    s->buf          = buffer;
    s->buf_end      = s->buf + buffer_size;
    s->buf_ptr      = s->buf;
    s->bit_left     = BUF_BITS;
    s->bit_buf      = 0;
}

static inline void put_bits_no_assert(PutBitContext *s, int n, BitBuf value)
{
    BitBuf bit_buf;
    int bit_left;

    bit_buf  = s->bit_buf;
    bit_left = s->bit_left;

    /* XXX: optimize */
#ifdef BITSTREAM_WRITER_LE
    bit_buf |= value << (BUF_BITS - bit_left);
    if (n >= bit_left) {
        if (s->buf_end - s->buf_ptr >= sizeof(BitBuf)) {
            AV_WLBUF(s->buf_ptr, bit_buf);
            s->buf_ptr += sizeof(BitBuf);
        } else {
            av_log(NULL, AV_LOG_ERROR, "Internal error, put_bits buffer too small\n");
            av_assert2(0);
        }
        bit_buf     = value >> bit_left;
        bit_left   += BUF_BITS;
    }
    bit_left -= n;
#else
    if (n < bit_left) {
        bit_buf     = (bit_buf << n) | value;
        bit_left   -= n;
    } else {
        bit_buf   <<= bit_left;
        bit_buf    |= value >> (n - bit_left);
        if (s->buf_end - s->buf_ptr >= sizeof(BitBuf)) {
            AV_WBBUF(s->buf_ptr, bit_buf);
            s->buf_ptr += sizeof(BitBuf);
        } else {
            av_log(NULL, AV_LOG_ERROR, "Internal error, put_bits buffer too small\n");
            av_assert2(0);
        }
        bit_left   += BUF_BITS - n;
        bit_buf     = value;
    }
#endif

    s->bit_buf  = bit_buf;
    s->bit_left = bit_left;
}

static inline void put_bits(PutBitContext *s, int n, BitBuf value)
{
    av_assert2(n <= 31 && value < (1UL << n));
    put_bits_no_assert(s, n, value);
}

static inline void put_bits_le(PutBitContext *s, int n, BitBuf value)
{
    BitBuf bit_buf;
    int bit_left;

    av_assert2(n <= 31 && value < (1UL << n));

    bit_buf  = s->bit_buf;
    bit_left = s->bit_left;

    bit_buf |= value << (BUF_BITS - bit_left);
    if (n >= bit_left) {
        if (s->buf_end - s->buf_ptr >= sizeof(BitBuf)) {
            AV_WLBUF(s->buf_ptr, bit_buf);
            s->buf_ptr += sizeof(BitBuf);
        } else {
            av_log(NULL, AV_LOG_ERROR, "Internal error, put_bits buffer too small\n");
            av_assert2(0);
        }
        bit_buf     = value >> bit_left;
        bit_left   += BUF_BITS;
    }
    bit_left -= n;

    s->bit_buf  = bit_buf;
    s->bit_left = bit_left;
}

static inline void put_sbits(PutBitContext *pb, int n, int32_t value)
{
    av_assert2(n >= 0 && n <= 31);

    put_bits(pb, n, av_mod_uintp2(value, n));
}


static void  put_bits32(PutBitContext *s, uint32_t value)
{
    BitBuf bit_buf;
    int bit_left;

    if (BUF_BITS > 32) {
        put_bits_no_assert(s, 32, value);
        return;
    }

    bit_buf  = s->bit_buf;
    bit_left = s->bit_left;

#ifdef BITSTREAM_WRITER_LE
    bit_buf |= (BitBuf)value << (BUF_BITS - bit_left);
    if (s->buf_end - s->buf_ptr >= sizeof(BitBuf)) {
        AV_WLBUF(s->buf_ptr, bit_buf);
        s->buf_ptr += sizeof(BitBuf);
    } else {
        av_log(NULL, AV_LOG_ERROR, "Internal error, put_bits buffer too small\n");
        av_assert2(0);
    }
    bit_buf     = (uint64_t)value >> bit_left;
#else
    bit_buf     = (uint64_t)bit_buf << bit_left;
    bit_buf    |= (BitBuf)value >> (BUF_BITS - bit_left);
    if (s->buf_end - s->buf_ptr >= sizeof(BitBuf)) {
        AV_WBBUF(s->buf_ptr, bit_buf);
        s->buf_ptr += sizeof(BitBuf);
    } else {
        av_log(NULL, AV_LOG_ERROR, "Internal error, put_bits buffer too small\n");
        av_assert2(0);
    }
    bit_buf     = value;
#endif

    s->bit_buf  = bit_buf;
    s->bit_left = bit_left;
}


static inline void put_bits64(PutBitContext *s, int n, uint64_t value)
{
    av_assert2((n == 64) || (n < 64 && value < (UINT64_C(1) << n)));

    if (n < 32)
        put_bits(s, n, value);
    else if (n == 32)
        put_bits32(s, value);
    else if (n < 64) {
        uint32_t lo = value & 0xffffffff;
        uint32_t hi = value >> 32;
#ifdef BITSTREAM_WRITER_LE
        put_bits32(s, lo);
        put_bits(s, n - 32, hi);
#else
        put_bits(s, n - 32, hi);
        put_bits32(s, lo);
#endif
    } else {
        uint32_t lo = value & 0xffffffff;
        uint32_t hi = value >> 32;
#ifdef BITSTREAM_WRITER_LE
        put_bits32(s, lo);
        put_bits32(s, hi);
#else
        put_bits32(s, hi);
        put_bits32(s, lo);
#endif

    }
}


static inline uint8_t *put_bits_ptr(PutBitContext *s)
{
    return s->buf_ptr;
}


static inline void skip_put_bytes(PutBitContext *s, int n)
{
    av_assert2((put_bits_count(s) & 7) == 0);
    av_assert2(s->bit_left == BUF_BITS);
    av_assert0(n <= s->buf_end - s->buf_ptr);
    s->buf_ptr += n;
}


static inline void skip_put_bits(PutBitContext *s, int n)
{
    unsigned bits = BUF_BITS - s->bit_left + n;
    s->buf_ptr += sizeof(BitBuf) * (bits / BUF_BITS);
    s->bit_left = BUF_BITS - (bits & (BUF_BITS - 1));
}


static inline void set_put_bits_buffer_size(PutBitContext *s, int size)
{
    av_assert0(size <= INT_MAX/8 - BUF_BITS);
    s->buf_end = s->buf + size;
    s->size_in_bits = 8*size;
}

static inline void flush_put_bits(PutBitContext *s)
{
#ifndef BITSTREAM_WRITER_LE
    if (s->bit_left < BUF_BITS)
        s->bit_buf <<= s->bit_left;
#endif
    while (s->bit_left < BUF_BITS) {
        av_assert0(s->buf_ptr < s->buf_end);
#ifdef BITSTREAM_WRITER_LE
        *s->buf_ptr++ = s->bit_buf;
        s->bit_buf  >>= 8;
#else
        *s->buf_ptr++ = s->bit_buf >> (BUF_BITS - 8);
        s->bit_buf  <<= 8;
#endif
        s->bit_left  += 8;
    }
    s->bit_left = BUF_BITS;
    s->bit_buf  = 0;
}

#include "utils.c"


static void *  worker(void *v){
    AVCodecContext *avctx = v;
    ThreadContext *c = avctx->internal->frame_thread_encoder;
    AVPacket *pkt = NULL;

    while (!atomic_load(&c->exit)) {
        int got_packet = 0, ret;
        AVFrame *frame;
        Task task;

        if(!pkt) pkt = av_packet_alloc();
        if(!pkt) continue;
        av_init_packet(pkt);

        pthread_mutex_lock(&c->task_fifo_mutex);
        while (av_fifo_size(c->task_fifo) <= 0 || atomic_load(&c->exit)) {
            if (atomic_load(&c->exit)) {
                pthread_mutex_unlock(&c->task_fifo_mutex);
                goto end;
            }
            pthread_cond_wait(&c->task_fifo_cond, &c->task_fifo_mutex);
        }
        av_fifo_generic_read(c->task_fifo, &task, sizeof(task), NULL);
        pthread_mutex_unlock(&c->task_fifo_mutex);
        frame = task.indata;

        ret = avctx->codec->encode2(avctx, pkt, frame, &got_packet);
        if(got_packet) {
            int ret2 = av_packet_make_refcounted(pkt);
            if (ret >= 0 && ret2 < 0)
                ret = ret2;
            pkt->pts = pkt->dts = frame->pts;
        } else {
            pkt->data = NULL;
            pkt->size = 0;
        }
        pthread_mutex_lock(&c->buffer_mutex);
        av_frame_unref(frame);
        pthread_mutex_unlock(&c->buffer_mutex);
        av_frame_free(&frame);
        pthread_mutex_lock(&c->finished_task_mutex);
        c->finished_tasks[task.index].outdata = pkt; pkt = NULL;
        c->finished_tasks[task.index].return_code = ret;
        pthread_cond_signal(&c->finished_task_cond);
        pthread_mutex_unlock(&c->finished_task_mutex);
    }
end:
    av_free(pkt);
    pthread_mutex_lock(&c->buffer_mutex);
    avcodec_close(avctx);
    pthread_mutex_unlock(&c->buffer_mutex);
    av_freep(&avctx);
    return NULL;
}


int ff_thread_video_encode_frame(AVCodecContext *avctx, AVPacket *pkt, const AVFrame *frame, int *got_packet_ptr){
    ThreadContext *c = avctx->internal->frame_thread_encoder;
    Task task;
    int ret;

    av_assert1(!*got_packet_ptr);

    if(frame){
        AVFrame *new = av_frame_alloc();
        if(!new)
            return AVERROR(ENOMEM);
        ret = av_frame_ref(new, frame);
        if(ret < 0) {
            av_frame_free(&new);
            return ret;
        }

        task.index = c->task_index;
        task.indata = (void*)new;
        pthread_mutex_lock(&c->task_fifo_mutex);
        av_fifo_generic_write(c->task_fifo, &task, sizeof(task), NULL);
        pthread_cond_signal(&c->task_fifo_cond);
        pthread_mutex_unlock(&c->task_fifo_mutex);

        c->task_index = (c->task_index+1) % BUFFER_SIZE;
    }

    pthread_mutex_lock(&c->finished_task_mutex);
    if (c->task_index == c->finished_task_index ||
        (frame && !c->finished_tasks[c->finished_task_index].outdata &&
         (c->task_index - c->finished_task_index) % BUFFER_SIZE <= avctx->thread_count)) {
            pthread_mutex_unlock(&c->finished_task_mutex);
            return 0;
        }

    while (!c->finished_tasks[c->finished_task_index].outdata) {
        pthread_cond_wait(&c->finished_task_cond, &c->finished_task_mutex);
    }
    task = c->finished_tasks[c->finished_task_index];
    *pkt = *(AVPacket*)(task.outdata);
    if(pkt->data)
        *got_packet_ptr = 1;
    av_freep(&c->finished_tasks[c->finished_task_index].outdata);
    c->finished_task_index = (c->finished_task_index+1) % BUFFER_SIZE;
    pthread_mutex_unlock(&c->finished_task_mutex);

    return task.return_code;
}

static inline int64_t ff_samples_to_time_base(AVCodecContext *avctx, int64_t samples)
{
    if(samples == AV_NOPTS_VALUE)
        return AV_NOPTS_VALUE;
    return av_rescale_q(samples, (AVRational){ 1, avctx->sample_rate },
                        avctx->time_base);
}


static inline float ff_exp2fi(int x) {
    
    if (-126 <= x && x <= 128)
        return av_int2float((x+127) << 23);
    
    else if (x > 128)
        return INFINITY;
    
    else if (x > -150)
        return av_int2float(1 << (x+149));
    
    else
        return 0;
}


void show_help_default(const char *opt, const char *arg);

double av_display_rotation_get(const int32_t matrix[9])
{
    double rotation, scale[2];

    scale[0] = hypot(CONV_FP(matrix[0]), CONV_FP(matrix[3]));
    scale[1] = hypot(CONV_FP(matrix[1]), CONV_FP(matrix[4]));

    if (scale[0] == 0.0 || scale[1] == 0.0)
        return NAN;

    rotation = atan2(CONV_FP(matrix[1]) / scale[1],
                     CONV_FP(matrix[0]) / scale[0]) * 180 / M_PI;

    return -rotation;
}

#include "cmdutils.c"


static int opt_add_vfilter(void *optctx, const char *opt, const char *arg)
{
    vfilters_list = grow_array(vfilters_list, sizeof(*vfilters_list), &nb_vfilters, nb_vfilters + 1);
    vfilters_list[nb_vfilters - 1] = arg;
    return 0;
}

static inline int cmp_audio_fmts(enum AVSampleFormat fmt1, int64_t channel_count1,
                                 enum AVSampleFormat fmt2, int64_t channel_count2)
{
    if (channel_count1 == 1 && channel_count2 == 1)
        return av_get_packed_sample_fmt(fmt1) != av_get_packed_sample_fmt(fmt2);
    else
        return channel_count1 != channel_count2 || fmt1 != fmt2;
}

static inline int64_t get_valid_channel_layout(int64_t channel_layout, int channels)
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
    if (pkt1 == NULL)
        return -1;
    pkt1->pkt = *pkt;
    pkt1->next = NULL;
    if (pkt == &flush_pkt)
        q->serial++;
    pkt1->serial = q->serial;
    if (q->last_pkt == NULL)
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
        NULL;
    pkt->size = 0;
    pkt->stream_index = stream_index;
    return packet_queue_put(q, pkt);
}

static int packet_queue_init(PacketQueue *q)
{
    memset(q, 0, sizeof(PacketQueue));
    q->mutex = SDL_CreateMutex();
    if (!q->mutex)
    {
        av_log(
            NULL, 8, "SDL_CreateMutex(): %s\n", SDL_GetError());
        return (-(
            12));
    }
    q->cond = SDL_CreateCond();
    if (!q->cond)
    {
        av_log(
            NULL, 8, "SDL_CreateCond(): %s\n", SDL_GetError());
        return (-(
            12));
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
    q->last_pkt =
        NULL;
    q->first_pkt =
        NULL;
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
                q->last_pkt =
                    NULL;
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
    d->start_pts = 0;
    d->pkt_serial = -1;
}

static int decoder_decode_frame(Decoder *d, AVFrame *frame, AVSubtitle *sub)
{
    int ret = (-(11));
    for (;;)
    {
        AVPacket pkt;
        if (d->queue->serial == d->pkt_serial)
        {
            do
            {
                if (d->queue->abort_request != 0)
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
                        AVRational tb = (AVRational){1, frame->sample_rate};
                        if (frame->pts != 0)
                            frame->pts = av_rescale_q(frame->pts, d->avctx->pkt_timebase, tb);
                        else if (d->next_pts != 0)
                            frame->pts = av_rescale_q(d->next_pts, d->next_pts_tb, tb);
                        if (frame->pts != 0)
                        {
                            d->next_pts = frame->pts + frame->nb_samples;
                            d->next_pts_tb = tb;
                        }
                    }
                    break;
                }
                if (ret == (-(int)(('1') | (('O') << 8) | (('F') << 16) | ((unsigned)(' ') << 24))))
                {
                    d->finished = d->pkt_serial;
                    avcodec_flush_buffers(d->avctx);
                    return 0;
                }
                if (ret >= 0)
                    return 1;
            } while (ret != (-(11)));
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
                    ret = (-(11));
                }
                else
                {
                    if (got_frame && !pkt.data)
                    {
                        d->packet_pending = 1;
                        av_packet_move_ref(&d->pkt, &pkt);
                    }
                    ret = got_frame ? 0 : (pkt.data ? (-(11)) : (-(int)(('1') | (('O') << 8) | (('F') << 16) | ((unsigned)(' ') << 24))));
                }
            }
            else
            {
                if (avcodec_send_packet(d->avctx, &pkt) == (-(
                                                               11)))
                {
                    av_log(d->avctx, 16, "Receive_frame and send_packet both returned EAGAIN, which is an API violation.\n");
                    d->packet_pending = 1;
                    av_packet_move_ref(&d->pkt, &pkt);
                }
            }
            av_packet_unref(&pkt);
        }
    }
}

static void decoder_destroy(Decoder *d)
{
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
    if (!(f->mutex = SDL_CreateMutex()))
    {
        av_log(NULL, 8, "SDL_CreateMutex(): %s\n", SDL_GetError());
        return (-(12));
    }
    if (!(f->cond = SDL_CreateCond()))
    {
        av_log(NULL, 8, "SDL_CreateCond(): %s\n", SDL_GetError());
        return (-(12));
    }
    f->pktq = pktq;
    f->max_size = (max_size > 16 ? 16 : max_size);
    f->keep_last = !!keep_last;
    for (i = 0; i < f->max_size; i++)
        if (!(f->queue[i].frame = av_frame_alloc()))
            return (-(12));
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
    SDL_WaitThread(d->decoder_tid,NULL);
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

static int realloc_texture(SDL_Texture **texture, uint32_t new_format, int new_width, int new_height, SDL_BlendMode blendmode, int init_texture)
{
    uint32_t format;
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
        av_log(NULL, 40, "Created %dx%d texture with %s.\n", new_width, new_height, SDL_GetPixelFormatName(new_format));
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
    if (width > scr_width)
    {
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

static void get_sdl_pix_fmt_and_blendmode(int format, uint32_t *sdl_pix_fmt, SDL_BlendMode *sdl_blendmode)
{
    int i;
    *sdl_blendmode = SDL_BLENDMODE_NONE;
    *sdl_pix_fmt = SDL_PIXELFORMAT_UNKNOWN;
    if (format == AV_PIX_FMT_BGRA ||
        format == AV_PIX_FMT_ABGR ||
        format == AV_PIX_FMT_RGBA ||
        format == AV_PIX_FMT_ARGB)
        *sdl_blendmode = SDL_BLENDMODE_BLEND;
    for (i = 0; i < (sizeof(sdl_texture_format_map) / sizeof((sdl_texture_format_map)[0])) - 1; i++)
    {
        if (format == sdl_texture_format_map[i].format)
        {
            *sdl_pix_fmt = sdl_texture_format_map[i].texture_fmt;
            return;
        }
    }
}

static int upload_texture(SDL_Texture **tex, AVFrame *frame, struct SwsContext **img_convert_ctx)
{
    int ret = 0;
    uint32_t sdl_pix_fmt;
    SDL_BlendMode sdl_blendmode;
    get_sdl_pix_fmt_and_blendmode(frame->format, &sdl_pix_fmt, &sdl_blendmode);
    if (realloc_texture(tex, sdl_pix_fmt == SDL_PIXELFORMAT_UNKNOWN ? SDL_PIXELFORMAT_ARGB8888 : sdl_pix_fmt, frame->width, frame->height, sdl_blendmode, 0) < 0)
        return -1;
    switch (sdl_pix_fmt)
    {
    case SDL_PIXELFORMAT_UNKNOWN:
        *img_convert_ctx = sws_getCachedContext(*img_convert_ctx,
                                                frame->width, frame->height, frame->format, frame->width, frame->height,
                                                AV_PIX_FMT_BGRA, sws_flags,
                                                NULL,
                                                NULL,
                                                NULL);
        if (*img_convert_ctx != NULL)
        {
            uint8_t *pixels[4];
            int pitch[4];
            if (!SDL_LockTexture(*tex,NULL, (void **)pixels, pitch))
            {
                sws_scale(*img_convert_ctx, (const uint8_t *const *)frame->data, frame->linesize,
                          0, frame->height, pixels, pitch);
                SDL_UnlockTexture(*tex);
            }
        }
        else
        {
            av_log(NULL, 8, "Cannot initialize the conversion context\n");
            ret = -1;
        }
        break;
    case SDL_PIXELFORMAT_IYUV:
        if (frame->linesize[0] > 0 && frame->linesize[1] > 0 && frame->linesize[2] > 0)
        {
            ret = SDL_UpdateYUVTexture(*tex,
                                       NULL, frame->data[0], frame->linesize[0],
                                       frame->data[1], frame->linesize[1],
                                       frame->data[2], frame->linesize[2]);
        }
        else if (frame->linesize[0] < 0 && frame->linesize[1] < 0 && frame->linesize[2] < 0)
        {
            ret = SDL_UpdateYUVTexture(*tex,
                                       NULL, frame->data[0] + frame->linesize[0] * (frame->height - 1), -frame->linesize[0],
                                       frame->data[1] + frame->linesize[1] * (AV_CEIL_RSHIFT(frame->height, 1) - 1), -frame->linesize[1],
                                       frame->data[2] + frame->linesize[2] * (AV_CEIL_RSHIFT(frame->height, 1) - 1), -frame->linesize[2]);
        }
        else
        {
            av_log(
                NULL, 16, "Mixed negative and positive linesizes are not supported.\n");
            return -1;
        }
        break;
    default:
        if (frame->linesize[0] < 0)
        {
            ret = SDL_UpdateTexture(*tex,NULL, frame->data[0] + frame->linesize[0] * (frame->height - 1), -frame->linesize[0]);
        }
        else
        {
            ret = SDL_UpdateTexture(*tex,NULL, frame->data[0], frame->linesize[0]);
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
                        sub_rect->x = av_clip_c(sub_rect->x, 0, sp->width);
                        sub_rect->y = av_clip_c(sub_rect->y, 0, sp->height);
                        sub_rect->w = av_clip_c(sub_rect->w, 0, sp->width - sub_rect->x);
                        sub_rect->h = av_clip_c(sub_rect->h, 0, sp->height - sub_rect->y);
                        is->sub_convert_ctx = sws_getCachedContext(is->sub_convert_ctx,
                                                                   sub_rect->w, sub_rect->h, AV_PIX_FMT_PAL8,
                                                                   sub_rect->w, sub_rect->h, AV_PIX_FMT_BGRA,
                                                                   0,
                                                                   NULL,
                                                                   NULL,
                                                                   NULL);
                        if (!is->sub_convert_ctx)
                        {
                            av_log(NULL, 8, "Cannot initialize the conversion context\n");
                            return;
                        }
                        if (!SDL_LockTexture(is->sub_texture, (SDL_Rect *)sub_rect, (void **)pixels, pitch))
                        {
                            sws_scale(is->sub_convert_ctx, (const uint8_t *const *)sub_rect->data, sub_rect->linesize, 0, sub_rect->h, pixels, pitch);
                            SDL_UnlockTexture(is->sub_texture);
                        }
                    }
                    sp->uploaded = 1;
                }
            }
            else
                sp =
                    NULL;
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
    SDL_RenderCopyEx(renderer, is->vid_texture,
                     NULL, &rect, 0,
                     NULL, vp->flip_v ? SDL_FLIP_VERTICAL : 0);
    set_sdl_yuv_conversion_mode(NULL);
    if (sp)
    {
        SDL_RenderCopy(renderer, is->sub_texture, NULL, &rect);
    }
}

static inline int compute_mod(int a, int b)
{
    return a < 0 ? a % b + b : a % b;
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
    if (!s->paused)
    {
        int data_used = s->show_mode == SHOW_MODE_WAVES ? s->width : (2 * nb_freq);
        n = 2 * channels;
        delay = s->audio_write_buf_size;
        delay /= n;
        if (audio_callback_time)
        {
            time_diff = av_gettime_relative() - audio_callback_time;
            delay -= (time_diff * s->audio_tgt.freq) / 1000000;
        }
        delay += 2 * data_used;
        if (delay < data_used)
            delay = data_used;
        i_start = x = compute_mod(s->sample_array_index - delay * channels, (8 * 65536));
        if (s->show_mode == SHOW_MODE_WAVES)
        {
            h =
                (-0x7fffffff - 1);
            for (i = 0; i < 1000; i += channels)
            {
                int idx = ((8 * 65536) + x - i) % (8 * 65536);
                int a = s->sample_array[idx];
                int b = s->sample_array[(idx + 4 * channels) % (8 * 65536)];
                int c = s->sample_array[(idx + 5 * channels) % (8 * 65536)];
                int d = s->sample_array[(idx + 9 * channels) % (8 * 65536)];
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
        h = s->height / nb_display_channels;
        h2 = (h * 9) / 20;
        for (ch = 0; ch < nb_display_channels; ch++)
        {
            i = i_start + ch;
            y1 = s->ytop + ch * h + (h / 2);
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
                if (i >= (8 * 65536))
                    i -= (8 * 65536);
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
        nb_display_channels = ((nb_display_channels) > (2) ? (2) : (nb_display_channels));
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
            av_log(
                NULL, 16, "Failed to allocate buffers for RDFT, switching to waves display\n");
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
                    if (i >= (8 * 65536))
                        i -= (8 * 65536);
                }
                av_rdft_calc(s->rdft, data[ch]);
            }
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
                    a = ((a) > (255) ? (255) : (a));
                    b = ((b) > (255) ? (255) : (b));
                    pixels -= pitch;
                    *pixels = (a << 16) + (b << 8) + ((a + b) >> 1);
                }
                SDL_UnlockTexture(s->vis_texture);
            }
            SDL_RenderCopy(renderer, s->vis_texture,
                           NULL,
                           NULL);
        }
        if (!s->paused)
            s->xpos++;
        if (s->xpos >= s->width)
            s->xpos = s->xleft;
    }
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

static void stream_close(VideoState *is)
{
    is->abort_request = 1;
    SDL_WaitThread(is->read_tid,
                   NULL);
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
    if (is != NULL)
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
    av_log(
        NULL, -8, "%s", "");
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
    int w, h;
    w = screen_width ? screen_width : default_width;
    h = screen_height ? screen_height : default_height;
    if (window_title == NULL)
        window_title = input_filename;
    SDL_SetWindowTitle(window, window_title);
    SDL_SetWindowSize(window, w, h);
    SDL_SetWindowPosition(window, screen_left, screen_top);
    if (is_full_screen != 0)
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
    set_clock(c,
              NAN, -1);
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
    if (is->video_stream >= 0 && is->videoq.nb_packets <= 2 ||
        is->audio_stream >= 0 && is->audioq.nb_packets <= 2)
    {
        set_clock_speed(&is->extclk, ((0.900) > (is->extclk.speed - 0.001) ? (0.900) : (is->extclk.speed - 0.001)));
    }
    else if ((is->video_stream < 0 || is->videoq.nb_packets > 10) &&
             (is->audio_stream < 0 || is->audioq.nb_packets > 10))
    {
        set_clock_speed(&is->extclk, ((1.010) > (is->extclk.speed + 0.001) ? (is->extclk.speed + 0.001) : (1.010)));
    }
    else
    {
        double speed = is->extclk.speed;
        if (speed != 1.0)
            set_clock_speed(&is->extclk, speed + 0.001 * (1.0 - speed) / fabs(1.0 - speed));
    }
}

static void stream_seek(VideoState *is, int64_t pos, int64_t rel, int seek_by_bytes)
{
    if (!is->seek_req)
    {
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
    if (is->paused)
    {
        is->frame_timer += av_gettime_relative() / 1000000.0 - is->vidclk.last_updated;
        if (is->read_pause_return != (-(
                                         40)))
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
    if (get_master_sync_type(is) != AV_SYNC_VIDEO_MASTER)
    {
        diff = get_clock(&is->vidclk) - get_master_clock(is);
        sync_threshold = ((0.04) > (((0.1) > (delay) ? (delay) : (0.1))) ? (0.04) : (((0.1) > (delay) ? (delay) : (0.1))));
        if (!isnan(diff) && fabs(diff) < is->max_frame_duration)
        {
            if (diff <= -sync_threshold)
                delay = ((0) > (delay + diff) ? (0) : (delay + diff));
            else if (diff >= sync_threshold && delay > 0.1)
                delay = delay + diff;
            else if (diff >= sync_threshold)
                delay = 2 * delay;
        }
    }
    av_log(NULL, 56, "video: delay=%0.3f A-V=%f\n",delay, -diff);
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
    if (!display_disable && is->show_mode != SHOW_MODE_VIDEO && is->audio_st)
    {
        time = av_gettime_relative() / 1000000.0;
        if (is->force_refresh || is->last_vis_time + rdftspeed < time)
        {
            video_display(is);
            is->last_vis_time = time;
        }
        *remaining_time = ((*remaining_time) > (is->last_vis_time + rdftspeed - time) ? (is->last_vis_time + rdftspeed - time) : (*remaining_time));
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
            last_duration = vp_duration(is, lastvp, vp);
            delay = compute_target_delay(last_duration, is);
            time = av_gettime_relative() / 1000000.0;
            if (time < is->frame_timer + delay)
            {
                *remaining_time = ((is->frame_timer + delay - time) > (*remaining_time) ? (*remaining_time) : (is->frame_timer + delay - time));
                goto display;
            }
            is->frame_timer += delay;
            if (delay > 0 && time - is->frame_timer > 0.1)
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
                        sp2 =
                            NULL;
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
            av_bprint_init(&buf, 0, 1);
            av_bprintf(&buf,
                       "%7.2f %s:%7.3f fd=%4d aq=%5dKB vq=%5dKB sq=%5dB f=%"
                       "I64d"
                       "/%"
                       "I64d"
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
                    (&__iob_func()[2]), "%s", buf.str);
            else
                av_log(
                    NULL, 32, "%s", buf.str);
            fflush(
                (&__iob_func()[2]));
            av_bprint_finalize(&buf,
                               NULL);
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
    if ((got_picture = decoder_decode_frame(&is->viddec, frame,NULL)) < 0)
        return -1;
    if (got_picture != 0)
    {
        double dpts = NAN;
        if (frame->pts != 0)
            dpts = av_q2d(is->video_st->time_base) * frame->pts;
        frame->sample_aspect_ratio = av_guess_sample_aspect_ratio(is->ic, is->video_st, frame);
        if (framedrop > 0 || (framedrop && get_master_sync_type(is) != AV_SYNC_VIDEO_MASTER))
        {
            if (frame->pts != 0)
            {
                double diff = dpts - get_master_clock(is);
                if (!isnan(diff) && fabs(diff) < AV_NOSYNC_THRESHOLD && diff - is->frame_last_filter_delay < 0 && is->viddec.pkt_serial == is->vidclk.serial && is->videoq.nb_packets) {
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
    AVFilterInOut *outputs = NULL,
                  *inputs = NULL;
    if (filtergraph)
    {
        outputs = avfilter_inout_alloc();
        inputs = avfilter_inout_alloc();
        if (!outputs || !inputs)
        {
            ret = (-12);
            goto fail;
        }
        outputs->name = av_strdup("in");
        outputs->filter_ctx = source_ctx;
        outputs->pad_idx = 0;
        outputs->next = NULL;
        inputs->name = av_strdup("out");
        inputs->filter_ctx = sink_ctx;
        inputs->pad_idx = 0;
        inputs->next =
            NULL;
        if ((ret = avfilter_graph_parse_ptr(graph, filtergraph, &inputs, &outputs, NULL)) < 0)
            goto fail;
    }
    else
    {
        if ((ret = avfilter_link(source_ctx, 0, sink_ctx, 0)) < 0)
            goto fail;
    }
    for (i = 0; i < graph->nb_filters - nb_filters; i++)
        do
        {
            AVFilterContext *SWAP_tmp = graph->filters[i + nb_filters];
            graph->filters[i + nb_filters] = graph->filters[i];
            graph->filters[i] = SWAP_tmp;
        } while (0);
    ret = avfilter_graph_config(graph,
                                NULL);
fail:
    avfilter_inout_free(&outputs);
    avfilter_inout_free(&inputs);
    return ret;
}
#define INSERT_FILT(name, arg)                                                                                       \
    do                                                                                                               \
    {                                                                                                                \
        AVFilterContext *filt_ctx;                                                                                   \
        ret = avfilter_graph_create_filter(&filt_ctx, avfilter_get_by_name(name), "ffplay_" name, arg, NULL, graph); \
        if (ret < 0)                                                                                                 \
            goto fail;                                                                                               \
        ret = avfilter_link(filt_ctx, 0, last_filter, 0);                                                            \
        if (ret < 0)                                                                                                 \
            goto fail;                                                                                               \
        last_filter = filt_ctx;                                                                                      \
    } while (0)

static int configure_video_filters(AVFilterGraph *graph, VideoState *is, const char *vfilters, AVFrame *frame)
{
    enum AVPixelFormat pix_fmts[(sizeof(sdl_texture_format_map) / sizeof((sdl_texture_format_map)[0]))];
    char sws_flags_str[512] = "";
    char buffersrc_args[256];
    int ret;
    AVFilterContext *filt_src = NULL,
                    *filt_out = NULL,
                    *last_filter = NULL;
    AVCodecParameters *codecpar = is->video_st->codecpar;
    AVRational fr = av_guess_frame_rate(is->ic, is->video_st, NULL);
    AVDictionaryEntry *e = NULL;
    int nb_pix_fmts = 0;
    int i, j;
    for (i = 0; i < renderer_info.num_texture_formats; i++)
    {
        for (j = 0; j < (sizeof(sdl_texture_format_map) / sizeof((sdl_texture_format_map)[0])) - 1; j++)
        {
            if (renderer_info.texture_formats[i] == sdl_texture_format_map[j].texture_fmt)
            {
                pix_fmts[nb_pix_fmts++] = sdl_texture_format_map[j].format;
                break;
            }
        }
    }
    pix_fmts[nb_pix_fmts] = AV_PIX_FMT_NONE;
    while ((e = av_dict_get(sws_dict, "", e, 2)))
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
             codecpar->sample_aspect_ratio.num, ((codecpar->sample_aspect_ratio.den) > (1) ? (codecpar->sample_aspect_ratio.den) : (1)));
    if (fr.num && fr.den)
        av_strlcatf(buffersrc_args, sizeof(buffersrc_args), ":frame_rate=%d/%d", fr.num, fr.den);
    if ((ret = avfilter_graph_create_filter(&filt_src,
                                            avfilter_get_by_name("buffer"),
                                            "ffplay_buffer", buffersrc_args,
                                            NULL,
                                            graph)) < 0)
        goto fail;
    ret = avfilter_graph_create_filter(&filt_out,
                                       avfilter_get_by_name("buffersink"),
                                       "ffplay_buffersink",
                                       NULL,
                                       NULL, graph);
    if (ret < 0)
        goto fail;
    if ((ret = (av_int_list_length_for_size(sizeof(*(pix_fmts)), pix_fmts, AV_PIX_FMT_NONE) > 0x7fffffff / sizeof(*(pix_fmts)) ? (-22)
                                                                                                                               : av_opt_set_bin(filt_out, "pix_fmts", (const uint8_t *)(pix_fmts), av_int_list_length_for_size(sizeof(*(pix_fmts)), pix_fmts, AV_PIX_FMT_NONE) * sizeof(*(pix_fmts)), (1 << 0)))) < 0)
        goto fail;
    last_filter = filt_out;
    if (autorotate)
    {
        double theta = get_rotation(is->video_st);
        if (fabs(theta - 90) < 1.0)
        {
            
                AVFilterContext *filt_ctx;
                ret = avfilter_graph_create_filter(&filt_ctx, avfilter_get_by_name("transpose"), "ffplay_transpose","clock",NULL, graph);
                if (ret < 0)
                    goto fail;
                ret = avfilter_link(filt_ctx, 0, last_filter, 0);
                if (ret < 0)
                    goto fail;
                last_filter = filt_ctx;
        }
        else if (fabs(theta - 180) < 1.0)
        {
            
                AVFilterContext *filt_ctx;
                ret = avfilter_graph_create_filter(&filt_ctx, avfilter_get_by_name("hflip"), "ffplay_hflip",NULL,NULL, graph);
                if (ret < 0)
                    goto fail;
                ret = avfilter_link(filt_ctx, 0, last_filter, 0);
                if (ret < 0)
                    goto fail;
                last_filter = filt_ctx;
          
                ret = avfilter_graph_create_filter(&filt_ctx, avfilter_get_by_name("vflip"), "ffplay_vflip",NULL,NULL, graph);
                if (ret < 0)
                    goto fail;
                ret = avfilter_link(filt_ctx, 0, last_filter, 0);
                if (ret < 0)
                    goto fail;
                last_filter = filt_ctx;
        }
        else if (fabs(theta - 270) < 1.0)
        {
      
                AVFilterContext *filt_ctx;
                ret = avfilter_graph_create_filter(&filt_ctx, avfilter_get_by_name("transpose"), "ffplay_transpose","cclock",NULL, graph);
                if (ret < 0)
                    goto fail;
                ret = avfilter_link(filt_ctx, 0, last_filter, 0);
                if (ret < 0)
                    goto fail;
                last_filter = filt_ctx;
        }
        else if (fabs(theta) > 1.0)
        {
            char rotate_buf[64];
            snprintf(rotate_buf, sizeof(rotate_buf), "%f*PI/180", theta);
     
                AVFilterContext *filt_ctx;
                ret = avfilter_graph_create_filter(&filt_ctx, avfilter_get_by_name("rotate"), "ffplay_rotate",rotate_buf,NULL, graph);
                if (ret < 0)
                    goto fail;
                ret = avfilter_link(filt_ctx, 0, last_filter, 0);
                if (ret < 0)
                    goto fail;
                last_filter = filt_ctx;
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
    static const enum AVSampleFormat sample_fmts[] = {AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_NONE};
    int sample_rates[2] = {0, -1};
    int64_t channel_layouts[2] = {0, -1};
    int channels[2] = {0, -1};
    AVFilterContext *filt_asrc =
                        NULL,
                    *filt_asink =
                        NULL;
    char aresample_swr_opts[512] = "";
    AVDictionaryEntry *e =
        NULL;
    char asrc_args[256];
    int ret;
    avfilter_graph_free(&is->agraph);
    if (!(is->agraph = avfilter_graph_alloc()))
        return (-(12));
    is->agraph->nb_threads = filter_nbthreads;
    while ((e = av_dict_get(swr_opts, "", e, 2)))
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
                 ":channel_layout=0x%"
                 "I64x",
                 is->audio_filter_src.channel_layout);
    ret = avfilter_graph_create_filter(&filt_asrc,
                                       avfilter_get_by_name("abuffer"), "ffplay_abuffer",
                                       asrc_args,
                                       NULL, is->agraph);
    if (ret < 0)
        goto end;
    ret = avfilter_graph_create_filter(&filt_asink,
                                       avfilter_get_by_name("abuffersink"), "ffplay_abuffersink",
                                       NULL,
                                       NULL, is->agraph);
    if (ret < 0)
        goto end;
    if ((ret = (av_int_list_length_for_size(sizeof(*(sample_fmts)), sample_fmts, AV_SAMPLE_FMT_NONE) > 0x7fffffff / sizeof(*(sample_fmts)) ? (-(22))
                                                                                                                                           : av_opt_set_bin(filt_asink, "sample_fmts", (const uint8_t *)(sample_fmts), av_int_list_length_for_size(sizeof(*(sample_fmts)), sample_fmts, AV_SAMPLE_FMT_NONE) * sizeof(*(sample_fmts)), (1 << 0)
                                                                                                                                                                ))) < 0)
        goto end;
    if ((ret = av_opt_set_int(filt_asink, "all_channel_counts", 1, (1 << 0))) < 0)
        goto end;
    if (force_output_format)
    {
        channel_layouts[0] = is->audio_tgt.channel_layout;
        channels[0] = is->audio_tgt.channel_layout ? -1 : is->audio_tgt.channels;
        sample_rates[0] = is->audio_tgt.freq;
        if ((ret = av_opt_set_int(filt_asink, "all_channel_counts", 0, (1 << 0))) < 0)
            goto end;
        if ((ret = (av_int_list_length_for_size(sizeof(*(channel_layouts)), channel_layouts, -1) > 0x7fffffff / sizeof(*(channel_layouts)) ? (-(
                                                                                                                                                 22))
                                                                                                                                           : av_opt_set_bin(filt_asink, "channel_layouts", (const uint8_t *)(channel_layouts), av_int_list_length_for_size(sizeof(*(channel_layouts)), channel_layouts, -1) * sizeof(*(channel_layouts)), (1 << 0)
                                                                                                                                                                ))) < 0)
            goto end;
        if ((ret = (av_int_list_length_for_size(sizeof(*(channels)), channels, -1) > 0x7fffffff / sizeof(*(channels)) ? (-(
                                                                                                                            22))
                                                                                                                      : av_opt_set_bin(filt_asink, "channel_counts", (const uint8_t *)(channels), av_int_list_length_for_size(sizeof(*(channels)), channels, -1) * sizeof(*(channels)), (1 << 0)
                                                                                                                                           ))) < 0)
            goto end;
        if ((ret = (av_int_list_length_for_size(sizeof(*(sample_rates)), sample_rates, -1) > 0x7fffffff / sizeof(*(sample_rates)) ? (-(
                                                                                                                                        22))
                                                                                                                                  : av_opt_set_bin(filt_asink, "sample_rates", (const uint8_t *)(sample_rates), av_int_list_length_for_size(sizeof(*(sample_rates)), sample_rates, -1) * sizeof(*(sample_rates)), (1 << 0)
                                                                                                                                                       ))) < 0)
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

    do
    {
        if ((got_frame = decoder_decode_frame(&is->auddec, frame,NULL)) < 0)
            goto the_end;
        if (got_frame)
        {
            tb = (AVRational){1, frame->sample_rate};
            dec_channel_layout = get_valid_channel_layout(frame->channel_layout, frame->channels);
            reconfigure =
                cmp_audio_fmts(is->audio_filter_src.fmt, is->audio_filter_src.channels,frame->format, frame->channels) ||
                is->audio_filter_src.channel_layout != dec_channel_layout ||
                is->audio_filter_src.freq != frame->sample_rate ||
                is->auddec.pkt_serial != last_serial;
            if (reconfigure)
            {
                char buf1[1024], buf2[1024];
                av_get_channel_layout_string(buf1, sizeof(buf1), -1, is->audio_filter_src.channel_layout);
                av_get_channel_layout_string(buf2, sizeof(buf2), -1, dec_channel_layout);
                av_log(
                    NULL, 48,
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
                af->pts = (frame->pts == 0) ? NAN : frame->pts * av_q2d(tb);
                af->pos = frame->pkt_pos;
                af->serial = is->auddec.pkt_serial;
                af->duration = av_q2d((AVRational){frame->nb_samples, frame->sample_rate});
                av_frame_move_ref(af->frame, frame);
                frame_queue_push(&is->sampq);
                if (is->audioq.serial != is->auddec.pkt_serial)
                    break;
            }
            if (ret == (-(int)(('1') | (('O') << 8) | (('F') << 16) | ((unsigned)(' ') << 24))))
                is->auddec.finished = is->auddec.pkt_serial;
        }
    } while (ret >= 0 || ret == (-(11)) || ret == (-(int)(('1') | (('O') << 8) | (('F') << 16) | ((unsigned)(' ') << 24))));
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
        av_log(NULL, 16, "SDL_CreateThread(): %s\n", SDL_GetError());
        return (-(
            12));
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
        return (-(12));
    for (;;)
    {
        ret = get_video_frame(is, frame);
        if (ret < 0)
            goto the_end;
        if (!ret)
            continue;
        if (last_w != frame->width || last_h != frame->height || last_format != frame->format || last_serial != is->viddec.pkt_serial || last_vfilter_idx != is->vfilter_idx)
        {
            av_log(
                NULL, 48,
                "Video frame changed from size:%dx%d format:%s serial:%d to size:%dx%d format:%s serial:%d\n",
                last_w, last_h,
                (const char *)av_x_if_null(av_get_pix_fmt_name(last_format), "none"), last_serial,
                frame->width, frame->height,
                (const char *)av_x_if_null(av_get_pix_fmt_name(frame->format), "none"), is->viddec.pkt_serial);
            avfilter_graph_free(&graph);
            graph = avfilter_graph_alloc();
            if (!graph)
            {
                ret = (-(
                    12));
                goto the_end;
            }
            graph->nb_threads = filter_nbthreads;
            if ((ret = configure_video_filters(graph, is, vfilters_list ? vfilters_list[is->vfilter_idx] : NULL, frame)) < 0)
            {
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
        while (ret >= 0)
        {
            is->frame_last_returned_time = av_gettime_relative() / 1000000.0;
            ret = av_buffersink_get_frame_flags(filt_out, frame, 0);
            if (ret < 0)
            {
                if (ret == (-(int)(('1') | (('O') << 8) | (('F') << 16) | ((unsigned)(' ') << 24))))
                    is->viddec.finished = is->viddec.pkt_serial;
                ret = 0;
                break;
            }
            is->frame_last_filter_delay = av_gettime_relative() / 1000000.0 - is->frame_last_returned_time;
            if (fabs(is->frame_last_filter_delay) > 10.0 / 10.0)
                is->frame_last_filter_delay = 0;
            tb = av_buffersink_get_time_base(filt_out);
            duration = (frame_rate.num && frame_rate.den ? av_q2d((AVRational){frame_rate.den, frame_rate.num}) : 0);
            pts = (frame->pts == 0) ? NAN
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
    for (;;)
    {
        if (!(sp = frame_queue_peek_writable(&is->subpq)))
            return 0;
        if ((got_subtitle = decoder_decode_frame(&is->subdec,
                                                 NULL, &sp->sub)) < 0)
            break;
        pts = 0;
        if (got_subtitle && sp->sub.format == 0)
        {
            if (sp->sub.pts != 0)
                pts = sp->sub.pts / (double)1000000;
            sp->pts = pts;
            sp->serial = is->subdec.pkt_serial;
            sp->width = is->subdec.avctx->width;
            sp->height = is->subdec.avctx->height;
            sp->uploaded = 0;
            frame_queue_push(&is->subpq);
        }
        else if (got_subtitle)
        {
            avsubtitle_free(&sp->sub);
        }
    }
    return 0;
}

static void update_sample_display(VideoState *is, short *samples, int samples_size)
{
    int size, len;
    size = samples_size / sizeof(short);
    while (size > 0)
    {
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
    if (get_master_sync_type(is) != AV_SYNC_AUDIO_MASTER)
    {
        double diff, avg_diff;
        int min_nb_samples, max_nb_samples;
        diff = get_clock(&is->audclk) - get_master_clock(is);
        if (!isnan(diff) && fabs(diff) < 10.0)
        {
            is->audio_diff_cum = diff + is->audio_diff_avg_coef * is->audio_diff_cum;
            if (is->audio_diff_avg_count < 20)
            {
                is->audio_diff_avg_count++;
            }
            else
            {
                avg_diff = is->audio_diff_cum * (1.0 - is->audio_diff_avg_coef);
                if (fabs(avg_diff) >= is->audio_diff_threshold)
                {
                    wanted_nb_samples = nb_samples + (int)(diff * is->audio_src.freq);
                    min_nb_samples = ((nb_samples * (100 - 10) / 100));
                    max_nb_samples = ((nb_samples * (100 + 10) / 100));
                    wanted_nb_samples = av_clip_c(wanted_nb_samples, min_nb_samples, max_nb_samples);
                }
                av_log(
                    NULL, 56, "diff=%f adiff=%f sample_diff=%d apts=%0.3f %f\n",
                    diff, avg_diff, wanted_nb_samples - nb_samples,
                    is->audio_clock, is->audio_diff_threshold);
            }
        }
        else
        {
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
    do
    {
        while (frame_queue_nb_remaining(&is->sampq) == 0)
        {
            if ((av_gettime_relative() - audio_callback_time) > 1000000LL * is->audio_hw_buf_size / is->audio_tgt.bytes_per_sec / 2)
                return -1;
            av_usleep(1000);
        }
        if (!(af = frame_queue_peek_readable(&is->sampq)))
            return -1;
        frame_queue_next(&is->sampq);
    } while (af->serial != is->audioq.serial);
    data_size = av_samples_get_buffer_size(
        NULL, af->frame->channels,
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
        is->swr_ctx = swr_alloc_set_opts(
            NULL,
            is->audio_tgt.channel_layout, is->audio_tgt.fmt, is->audio_tgt.freq,
            dec_channel_layout, af->frame->format, af->frame->sample_rate,
            0,
            NULL);
        if (!is->swr_ctx || swr_init(is->swr_ctx) < 0)
        {
            av_log(
                NULL, 16,
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
        int out_size = av_samples_get_buffer_size(
            NULL, is->audio_tgt.channels, out_count, is->audio_tgt.fmt, 0);
        int len2;
        if (out_size < 0)
        {
            av_log(
                NULL, 16, "av_samples_get_buffer_size() failed\n");
            return -1;
        }
        if (wanted_nb_samples != af->frame->nb_samples)
        {
            if (swr_set_compensation(is->swr_ctx, (wanted_nb_samples - af->frame->nb_samples) * is->audio_tgt.freq / af->frame->sample_rate,
                                     wanted_nb_samples * is->audio_tgt.freq / af->frame->sample_rate) < 0)
            {
                av_log(
                    NULL, 16, "swr_set_compensation() failed\n");
                return -1;
            }
        }
        av_fast_malloc(&is->audio_buf1, &is->audio_buf1_size, out_size);
        if (!is->audio_buf1)
            return (-(
                12));
        len2 = swr_convert(is->swr_ctx, out, out_count, in, af->frame->nb_samples);
        if (len2 < 0)
        {
            av_log(
                NULL, 16, "swr_convert() failed\n");
            return -1;
        }
        if (len2 == out_count)
        {
            av_log(
                NULL, 24, "audio buffer is probably too small\n");
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
    if (!isnan(af->pts))
        is->audio_clock = af->pts + (double)af->frame->nb_samples / af->frame->sample_rate;
    else
        is->audio_clock = NAN;
    is->audio_clock_serial = af->serial;
    return resampled_data_size;
}

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
                is->audio_buf =
                    NULL;
                is->audio_buf_size = 512 / is->audio_tgt.frame_size * is->audio_tgt.frame_size;
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
        if (!is->muted && is->audio_buf && is->audio_volume == 128)
            memcpy(stream, (uint8_t *)is->audio_buf + is->audio_buf_index, len1);
        else
        {
            memset(stream, 0, len1);
            if (!is->muted && is->audio_buf)
                SDL_MixAudioFormat(stream, (uint8_t *)is->audio_buf + is->audio_buf_index, 0x8010, len1, is->audio_volume);
        }
        len -= len1;
        stream += len1;
        is->audio_buf_index += len1;
    }
    is->audio_write_buf_size = is->audio_buf_size - is->audio_buf_index;
    if (!isnan(is->audio_clock))
    {
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
    if (env)
    {
        wanted_nb_channels = atoi(env);
        wanted_channel_layout = av_get_default_channel_layout(wanted_nb_channels);
    }
    if (!wanted_channel_layout || wanted_nb_channels != av_get_channel_layout_nb_channels(wanted_channel_layout))
    {
        wanted_channel_layout = av_get_default_channel_layout(wanted_nb_channels);
        wanted_channel_layout &= ~(0x20000000 | 0x40000000);
    }
    wanted_nb_channels = av_get_channel_layout_nb_channels(wanted_channel_layout);
    wanted_spec.channels = wanted_nb_channels;
    wanted_spec.freq = wanted_sample_rate;
    if (wanted_spec.freq <= 0 || wanted_spec.channels <= 0)
    {
        av_log(
            NULL, 16, "Invalid sample rate or channel count!\n");
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
                 NULL, 0, &wanted_spec, &spec, 0x00000001 | 0x00000004)))
    {
        av_log(
            NULL, 24, "SDL_OpenAudio (%d channels, %d Hz): %s\n",
            wanted_spec.channels, wanted_spec.freq, SDL_GetError());
        wanted_spec.channels = next_nb_channels[((7) > (wanted_spec.channels) ? (wanted_spec.channels) : (7))];
        if (!wanted_spec.channels)
        {
            wanted_spec.freq = next_sample_rates[next_sample_rate_idx--];
            wanted_spec.channels = wanted_nb_channels;
            if (!wanted_spec.freq)
            {
                av_log(
                    NULL, 16,
                    "No more combinations to try, audio open failed\n");
                return -1;
            }
        }
        wanted_channel_layout = av_get_default_channel_layout(wanted_spec.channels);
    }
    if (spec.format != 0x8010)
    {
        av_log(
            NULL, 16,
            "SDL advised audio format %d is not supported!\n", spec.format);
        return -1;
    }
    if (spec.channels != wanted_spec.channels)
    {
        wanted_channel_layout = av_get_default_channel_layout(spec.channels);
        if (!wanted_channel_layout)
        {
            av_log(
                NULL, 16,
                "SDL advised channel count %d is not supported!\n", spec.channels);
            return -1;
        }
    }
    audio_hw_params->fmt = AV_SAMPLE_FMT_S16;
    audio_hw_params->freq = spec.freq;
    audio_hw_params->channel_layout = wanted_channel_layout;
    audio_hw_params->channels = spec.channels;
    audio_hw_params->frame_size = av_samples_get_buffer_size(
        NULL, audio_hw_params->channels, 1, audio_hw_params->fmt, 1);
    audio_hw_params->bytes_per_sec = av_samples_get_buffer_size(
        NULL, audio_hw_params->channels, audio_hw_params->freq, audio_hw_params->fmt, 1);
    if (audio_hw_params->bytes_per_sec <= 0 || audio_hw_params->frame_size <= 0)
    {
        av_log(NULL, 16, "av_samples_get_buffer_size failed\n");
        return -1;
    }
    return spec.size;
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
    if (avctx == NULL)
        return (-(12));
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
    if (codec == NULL)
    {
        if (forced_codec_name != NULL)
            av_log(NULL, 24,"No codec could be found with name '%s'\n", forced_codec_name);
        else
            av_log(NULL, 24,"No decoder could be found for codec %s\n", avcodec_get_name(avctx->codec_id));
        ret = (-(22));
        goto fail;
    }
    avctx->codec_id = codec->id;
    if (stream_lowres > codec->max_lowres)
    {
        av_log(avctx, 24, "The maximum value for lowres supported by the decoder is %d\n",codec->max_lowres);
        stream_lowres = codec->max_lowres;
    }
    avctx->lowres = stream_lowres;
    if (fast != 0)
        avctx->flags2 |= (1 << 0);
    opts = filter_codec_opts(codec_opts, avctx->codec_id, ic, ic->streams[stream_index], codec);
    t = av_dict_get(opts, "threads",NULL, 0);
    if (t == NULL)
        av_dict_set(&opts, "threads", "auto", 0);
    if (stream_lowres != 0)
        av_dict_set_int(&opts, "lowres", stream_lowres, 0);
    if (avctx->codec_type == AVMEDIA_TYPE_VIDEO || avctx->codec_type == AVMEDIA_TYPE_AUDIO)
        av_dict_set(&opts, "refcounted_frames", "1", 0);
    ret = avcodec_open2(avctx, codec, &opts);
    if ( ret< 0)
    {
        goto fail;
    }
    t = av_dict_get(opts, "",NULL, 2);
    if (t != NULL)
    {
        av_log(NULL, 16, "Option %s not found.\n", t->key);
        ret = (-(int)((0xF8) | (('O') << 8) | (('P') << 16) | ((unsigned)('T') << 24)));
        goto fail;
    }
    is->eof = 0;
    ic->streams[stream_index]->discard = AVDISCARD_DEFAULT;
    switch (avctx->codec_type)
    {
    case AVMEDIA_TYPE_AUDIO:
    {
        AVFilterContext *sink;
        is->audio_filter_src.freq = avctx->sample_rate;
        is->audio_filter_src.channels = avctx->channels;
        is->audio_filter_src.channel_layout = get_valid_channel_layout(avctx->channel_layout, avctx->channels);
        is->audio_filter_src.fmt = avctx->sample_fmt;
        ret = configure_audio_filters(is, afilters, 0);
        if (ret < 0)
            goto fail;
        sink = is->out_audio_filter;
        sample_rate = av_buffersink_get_sample_rate(sink);
        nb_channels = av_buffersink_get_channels(sink);
        channel_layout = av_buffersink_get_channel_layout(sink);
    }
    ret = audio_open(is, channel_layout, nb_channels, sample_rate, &is->audio_tgt);
        if (ret < 0)
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
        if ((is->ic->iformat->flags & (0x2000 | 0x4000 | 0x8000)) && is->ic->iformat->read_seek == NULL)
        {
            is->auddec.start_pts = is->audio_st->start_time;
            is->auddec.start_pts_tb = is->audio_st->time_base;
        }
        ret = decoder_start(&is->auddec, audio_thread, "audio_decoder", is);
        if ( ret < 0)
            goto out;
        SDL_PauseAudioDevice(audio_dev, 0);
        break;
    case AVMEDIA_TYPE_VIDEO:
        is->video_stream = stream_index;
        is->video_st = ic->streams[stream_index];
        decoder_init(&is->viddec, avctx, &is->videoq, is->continue_read_thread);
        ret = decoder_start(&is->viddec, video_thread, "video_decoder", is);
        if (ret < 0)
            goto out;
        is->queue_attachments_req = 1;
        break;
    case AVMEDIA_TYPE_SUBTITLE:
        is->subtitle_stream = stream_index;
        is->subtitle_st = ic->streams[stream_index];
        decoder_init(&is->subdec, avctx, &is->subtitleq, is->continue_read_thread);
        ret = decoder_start(&is->subdec, subtitle_thread, "subtitle_decoder", is);
        if (ret < 0)
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
           (st->disposition & 0x0400) ||
           queue->nb_packets > 25 && (!queue->duration || av_q2d(st->time_base) * queue->duration > 1.0);
}

static int is_realtime(AVFormatContext *s)
{
    if (!strcmp(s->iformat->name, "rtp") || !strcmp(s->iformat->name, "rtsp") || !strcmp(s->iformat->name, "sdp"))
        return 1;
    if (s->pb && (!strncmp(s->url, "rtp:", 4) || !strncmp(s->url, "udp:", 4)))
        return 1;
    return 0;
}


int ff_id3v2_match(const uint8_t *buf, const char *magic)
{
    return  buf[0]         == magic[0] &&
            buf[1]         == magic[1] &&
            buf[2]         == magic[2] &&
            buf[3]         != 0xff     &&
            buf[4]         != 0xff     &&
           (buf[6] & 0x80) == 0        &&
           (buf[7] & 0x80) == 0        &&
           (buf[8] & 0x80) == 0        &&
           (buf[9] & 0x80) == 0;
}
int ff_id3v2_tag_len(const uint8_t *buf)
{
    int len = ((buf[6] & 0x7f) << 21) +
              ((buf[7] & 0x7f) << 14) +
              ((buf[8] & 0x7f) << 7) +
              (buf[9] & 0x7f) +
              ID3v2_HEADER_SIZE;
    if (buf[5] & 0x10)
        len += ID3v2_HEADER_SIZE;
    return len;
}

int av_match_ext(const char *filename, const char *extensions)
{
    const char *ext;
    if (filename == NULL)
        return 0;
    ext = strrchr(filename, '.');
    if (ext)
        return av_match_name(ext + 1, extensions);
    return 0;
}
ff_const59 AVInputFormat *av_probe_input_format3(ff_const59 AVProbeData *pd, int is_opened,int *score_ret)
{
    AVProbeData lpd = *pd;
    const AVInputFormat *fmt1 = NULL;
    ff_const59 AVInputFormat *fmt = NULL;
    int score, score_max = 0;
    void *i = 0;
    const static uint8_t zerobuffer[AVPROBE_PADDING_SIZE];
    enum nodat {
        NO_ID3,
        ID3_ALMOST_GREATER_PROBE,
        ID3_GREATER_PROBE,
        ID3_GREATER_MAX_PROBE,
    } nodat = NO_ID3;
    if (lpd.buf == NULL)
        lpd.buf = (unsigned char *) zerobuffer;
    if (lpd.buf_size > 10 && ff_id3v2_match(lpd.buf, ID3v2_DEFAULT_MAGIC)) {
        int id3len = ff_id3v2_tag_len(lpd.buf);
        if (lpd.buf_size > id3len + 16) {
            if (lpd.buf_size < 2LL*id3len + 16)
                nodat = ID3_ALMOST_GREATER_PROBE;
            lpd.buf      += id3len;
            lpd.buf_size -= id3len;
        } else if (id3len >= PROBE_BUF_MAX) {
            nodat = ID3_GREATER_MAX_PROBE;
        } else
            nodat = ID3_GREATER_PROBE;
    }
    while ((fmt1 = av_demuxer_iterate(&i))) {
        if (!is_opened == !(fmt1->flags & AVFMT_NOFILE) && strcmp(fmt1->name, "image2"))
            continue;
        score = 0;
        if (fmt1->read_probe) {
            score = fmt1->read_probe(&lpd);
            if (score != 0)
                av_log(NULL, AV_LOG_TRACE, "Probing %s score:%d size:%d\n", fmt1->name, score, lpd.buf_size);
            if (fmt1->extensions && av_match_ext(lpd.filename, fmt1->extensions)) {
                switch (nodat) {
                case NO_ID3:
                    score = FFMAX(score, 1);
                    break;
                case ID3_GREATER_PROBE:
                case ID3_ALMOST_GREATER_PROBE:
                    score = FFMAX(score, AVPROBE_SCORE_EXTENSION / 2 - 1);
                    break;
                case ID3_GREATER_MAX_PROBE:
                    score = FFMAX(score, AVPROBE_SCORE_EXTENSION);
                    break;
                }
            }
        } else if (fmt1->extensions) {
            if (av_match_ext(lpd.filename, fmt1->extensions))
                score = AVPROBE_SCORE_EXTENSION;
        }
        if (av_match_name(lpd.mime_type, fmt1->mime_type)) {
            if (AVPROBE_SCORE_MIME > score) {
                av_log(NULL, AV_LOG_DEBUG, "Probing %s score:%d increased to %d due to MIME type\n", fmt1->name, score, AVPROBE_SCORE_MIME);
                score = AVPROBE_SCORE_MIME;
            }
        }
        if (score > score_max) {
            score_max = score;
            fmt       = (AVInputFormat*)fmt1;
        } else if (score == score_max)
            fmt = NULL;
    }
    if (nodat == ID3_GREATER_PROBE)
        score_max = FFMIN(AVPROBE_SCORE_EXTENSION / 2 - 1, score_max);
    *score_ret = score_max;
    return fmt;
}
ff_const59 AVInputFormat *av_probe_input_format2(ff_const59 AVProbeData *pd, int is_opened, int *score_max)
{
    int score_ret;
    ff_const59 AVInputFormat *fmt = av_probe_input_format3(pd, is_opened, &score_ret);
    if (score_ret > *score_max) {
        *score_max = score_ret;
        return fmt;
    } else
        return NULL;
}
int ffio_rewind_with_probe_data(AVIOContext *s, unsigned char **bufp, int buf_size)
{
    int64_t buffer_start;
    int buffer_size;
    int overlap, new_size, alloc_size;
    uint8_t *buf = *bufp;
    if (s->write_flag) {
        av_freep(bufp);
        return AVERROR(EINVAL);
    }
    buffer_size = s->buf_end - s->buffer;
    if ((buffer_start = s->pos - buffer_size) > buf_size) {
        av_freep(bufp);
        return AVERROR(EINVAL);
    }
    overlap = buf_size - buffer_start;
    new_size = buf_size + buffer_size - overlap;
    alloc_size = FFMAX(s->buffer_size, new_size);
    if (alloc_size > buf_size)
        if (!(buf = (*bufp) = av_realloc_f(buf, 1, alloc_size)))
            return AVERROR(ENOMEM);
    if (new_size > buf_size) {
        memcpy(buf + buf_size, s->buffer + overlap, buffer_size - overlap);
        buf_size = new_size;
    }
    av_free(s->buffer);
    s->buf_ptr = s->buffer = buf;
    s->buffer_size = alloc_size;
    s->pos = buf_size;
    s->buf_end = s->buf_ptr + buf_size;
    s->eof_reached = 0;
    return 0;
}
static int read_packet_wrapper(AVIOContext *s, uint8_t *buf, int size)
{
    int ret;
    if (!s->read_packet)
        return AVERROR(EINVAL);
    ret = s->read_packet(s->opaque, buf, size);
#if FF_API_OLD_AVIO_EOF_0
    if (!ret && !s->max_packet_size) {
        av_log(NULL, AV_LOG_WARNING, "Invalid return value 0 for stream protocol\n");
        ret = AVERROR_EOF;
    }
#else
    av_assert2(ret || s->max_packet_size);
#endif
    return ret;
}
static int url_resetbuf(AVIOContext *s, int flags)
{
    av_assert1(flags == AVIO_FLAG_WRITE || flags == AVIO_FLAG_READ);
    if (flags & AVIO_FLAG_WRITE) {
        s->buf_end = s->buffer + s->buffer_size;
        s->write_flag = 1;
    } else {
        s->buf_end = s->buffer;
        s->write_flag = 0;
    }
    return 0;
}
#define IO_BUFFER_SIZE 32768
int ffio_set_buf_size(AVIOContext *s, int buf_size)
{
    uint8_t *buffer;
    buffer = av_malloc(buf_size);
    if (!buffer)
        return AVERROR(ENOMEM);
    av_free(s->buffer);
    s->buffer = buffer;
    s->orig_buffer_size =
    s->buffer_size = buf_size;
    s->buf_ptr = s->buf_ptr_max = buffer;
    url_resetbuf(s, s->write_flag ? AVIO_FLAG_WRITE : AVIO_FLAG_READ);
    return 0;
}
static void fill_buffer(AVIOContext *s)
{
    int max_buffer_size = s->max_packet_size ?
                          s->max_packet_size : IO_BUFFER_SIZE;
    uint8_t *dst        = s->buf_end - s->buffer + max_buffer_size <= s->buffer_size ?
                          s->buf_end : s->buffer;
    int len             = s->buffer_size - (dst - s->buffer);
    
    if (!s->read_packet && s->buf_ptr >= s->buf_end)
        s->eof_reached = 1;
    
    if (s->eof_reached)
        return;
    if (s->update_checksum && dst == s->buffer) {
        if (s->buf_end > s->checksum_ptr)
            s->checksum = s->update_checksum(s->checksum, s->checksum_ptr,
                                             s->buf_end - s->checksum_ptr);
        s->checksum_ptr = s->buffer;
    }
    
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
int avio_read(AVIOContext *s, unsigned char *buf, int size)
{
    int len, size1;
    size1 = size;
    while (size > 0) {
        len = FFMIN(s->buf_end - s->buf_ptr, size);
        if (len == 0 || s->write_flag) {
            if((s->direct || size > s->buffer_size) && !s->update_checksum) {
                len = read_packet_wrapper(s, buf, size);
                if (len == AVERROR_EOF) {
                    
                    s->eof_reached = 1;
                    break;
                } else if (len < 0) {
                    s->eof_reached = 1;
                    s->error= len;
                    break;
                } else {
                    s->pos += len;
                    s->bytes_read += len;
                    size -= len;
                    buf += len;
                    s->buf_ptr = s->buffer;
                    s->buf_end = s->buffer;
                }
            } else {
                fill_buffer(s);
                len = s->buf_end - s->buf_ptr;
                if (len == 0)
                    break;
            }
        } else {
            memcpy(buf, s->buf_ptr, len);
            buf += len;
            s->buf_ptr += len;
            size -= len;
        }
    }
    if (size1 == size) {
        if (s->error)      return s->error;
        if (avio_feof(s))  return AVERROR_EOF;
    }
    return size1 - size;
}
int av_probe_input_buffer2(AVIOContext *pb, ff_const59 AVInputFormat **fmt, const char *filename, void *logctx, unsigned int offset, unsigned int max_probe_size)
{
    AVProbeData pd = { filename ? filename : "" };
    uint8_t *buf = NULL;
    int ret = 0, probe_size, buf_offset = 0;
    int score = 0;
    int ret2;
    if (!max_probe_size)
        max_probe_size = PROBE_BUF_MAX;
    else if (max_probe_size < PROBE_BUF_MIN) {
        av_log(logctx, AV_LOG_ERROR, "Specified probe size value %u cannot be < %u\n", max_probe_size, PROBE_BUF_MIN);
        return AVERROR(EINVAL);
    }
    if (offset >= max_probe_size)
        return AVERROR(EINVAL);
    if (pb->av_class) {
        uint8_t *mime_type_opt = NULL;
        char *semi;
        av_opt_get(pb, "mime_type", AV_OPT_SEARCH_CHILDREN, &mime_type_opt);
        pd.mime_type = (const char *)mime_type_opt;
        semi = pd.mime_type ? strchr(pd.mime_type, ';') : NULL;
        if (semi != NULL) {
            *semi = '\0';
        }
    }
    for (probe_size = PROBE_BUF_MIN; probe_size <= max_probe_size && !*fmt; probe_size = FFMIN(probe_size << 1, FFMAX(max_probe_size, probe_size + 1))) {
        score = probe_size < max_probe_size ? AVPROBE_SCORE_RETRY : 0;
        ret = av_reallocp(&buf, probe_size + AVPROBE_PADDING_SIZE);
        if (ret < 0)
            goto fail;
        ret = avio_read(pb, buf + buf_offset, probe_size - buf_offset);
        if (ret < 0) {
            if (ret != AVERROR_EOF)
                goto fail;
            score = 0;
            ret   = 0;          
        }
        buf_offset += ret;
        if (buf_offset < offset)
            continue;
        pd.buf_size = buf_offset - offset;
        pd.buf = &buf[offset];
        memset(pd.buf + pd.buf_size, 0, AVPROBE_PADDING_SIZE);
        
        *fmt = av_probe_input_format2(&pd, 1, &score);
        if (*fmt != NULL) {
            if (score <= AVPROBE_SCORE_RETRY) {
                av_log(logctx, AV_LOG_WARNING, "Format %s detected only with low score of %d, " "misdetection possible!\n", (*fmt)->name, score);
            } else
                av_log(logctx, AV_LOG_DEBUG, "Format %s probed with size=%d and score=%d\n", (*fmt)->name, probe_size, score);
        }
    }
    if ( *fmt == NULL)
        ret = AVERROR_INVALIDDATA;

fail:
    
    ret2 = ffio_rewind_with_probe_data(pb, &buf, buf_offset);
    if (ret >= 0)
        ret = ret2;
    av_freep(&pd.mime_type);
    return ret < 0 ? ret : score;
}
static int init_input(AVFormatContext *s, const char *filename, AVDictionary **options)
{
    int ret;
    AVProbeData pd = { filename, NULL, 0 };
    int score = AVPROBE_SCORE_RETRY;
    if (s->pb) {
        s->flags |= AVFMT_FLAG_CUSTOM_IO;
        if (!s->iformat)
            return av_probe_input_buffer2(s->pb, &s->iformat, filename,s, 0, s->format_probesize);
        else if (s->iformat->flags & AVFMT_NOFILE)
            av_log(s, AV_LOG_WARNING, "Custom AVIOContext makes no sense and ""will be ignored with AVFMT_NOFILE format.\n");
        return 0;
    }
    if ((s->iformat && s->iformat->flags & AVFMT_NOFILE) ||
        (!s->iformat && (s->iformat = av_probe_input_format2(&pd, 0, &score))))
        return score;
    ret = s->io_open(s, &s->pb, filename, AVIO_FLAG_READ | s->avio_flags, options);
    if (ret < 0)
        return ret;
    if (s->iformat)
        return 0;
    return av_probe_input_buffer2(s->pb, &s->iformat, filename,s, 0, s->format_probesize);
}
#define IO_BUFFER_SIZE 32768

static void update_checksum(AVIOContext *s)
{
    if (s->update_checksum && s->buf_ptr > s->checksum_ptr) {
        s->checksum = s->update_checksum(s->checksum, s->checksum_ptr,s->buf_ptr - s->checksum_ptr);
    }
}
int ffio_ensure_seekback(AVIOContext *s, int64_t buf_size)
{
    uint8_t *buffer;
    int max_buffer_size = s->max_packet_size ?
                          s->max_packet_size : IO_BUFFER_SIZE;
    ptrdiff_t filled = s->buf_end - s->buf_ptr;
    if (buf_size <= s->buf_end - s->buf_ptr)
        return 0;
    buf_size += max_buffer_size - 1;
    if (buf_size + s->buf_ptr - s->buffer <= s->buffer_size || s->seekable || !s->read_packet)
        return 0;
    av_assert0(!s->write_flag);
    if (buf_size <= s->buffer_size) {
        update_checksum(s);
        memmove(s->buffer, s->buf_ptr, filled);
    } else {
        buffer = av_malloc(buf_size);
        if (!buffer)
            return AVERROR(ENOMEM);
        update_checksum(s);
        memcpy(buffer, s->buf_ptr, filled);
        av_free(s->buffer);
        s->buffer = buffer;
        s->buffer_size = buf_size;
    }
    s->buf_ptr = s->buffer;
    s->buf_end = s->buffer + filled;
    s->checksum_ptr = s->buffer;
    return 0;
}
unsigned int avio_rl24(AVIOContext *s)
{
    unsigned int val;
    val = avio_rl16(s);
    val |= avio_r8(s) << 16;
    return val;
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
typedef struct DynBuffer
{
    int pos, size, allocated_size;
    uint8_t *buffer;
    int io_buffer_size;
    uint8_t io_buffer[1];
} DynBuffer;
int avio_close_dyn_buf(AVIOContext *s, uint8_t **pbuffer)
{
    DynBuffer *d;
    int size;
    static const char padbuf[AV_INPUT_BUFFER_PADDING_SIZE] = {0};
    int padding = 0;
    if (!s) {
        *pbuffer = NULL;
        return 0;
    }
    
    if (!s->max_packet_size) {
        avio_write(s, padbuf, sizeof(padbuf));
        padding = AV_INPUT_BUFFER_PADDING_SIZE;
    }
    avio_flush(s);
    d = s->opaque;
    *pbuffer = d->buffer;
    size = d->size;
    av_free(d);
    avio_context_free(&s);
    return size - padding;
}

#define PUT_UTF8(val, tmp, PUT_BYTE)                      \
    {                                                     \
        int bytes, shift;                                 \
        uint32_t in = val;                                \
        if (in < 0x80)                                    \
        {                                                 \
            tmp = in;                                     \
            PUT_BYTE                                      \
        }                                                 \
        else                                              \
        {                                                 \
            bytes = (av_log2(in) + 4) / 5;                \
            shift = (bytes - 1) * 6;                      \
            tmp = (256 - (256 >> bytes)) | (in >> shift); \
            PUT_BYTE                                      \
            while (shift >= 6)                            \
            {                                             \
                shift -= 6;                               \
                tmp = 0x80 | ((in >> shift) & 0x3f);      \
                PUT_BYTE                                  \
            }                                             \
        }                                                 \
    }
    void ffio_free_dyn_buf(AVIOContext **s)
{
    DynBuffer *d;
    if (!*s)
        return;
    d = (*s)->opaque;
    av_free(d->buffer);
    av_free(d);
    avio_context_free(s);
}
#define GET_UTF16(val, GET_16BIT, ERROR)     \
    val = (GET_16BIT);                       \
    {                                        \
        unsigned int hi = val - 0xD800;      \
        if (hi < 0x800)                      \
        {                                    \
            val = (GET_16BIT)-0xDC00;        \
            if (val > 0x3FFU || hi > 0x3FFU) \
            {                                \
                ERROR                        \
            }                                \
            val += (hi << 10) + 0x10000;     \
        }                                    \
    }

static int decode_str(AVFormatContext *s, AVIOContext *pb, int encoding,
                      uint8_t **dst, int *maxread)
{
    int ret;
    uint8_t tmp;
    uint32_t ch = 1;
    int left = *maxread;
    unsigned int (*get)(AVIOContext*) = avio_rb16;
    AVIOContext *dynbuf;
    if ((ret = avio_open_dyn_buf(&dynbuf)) < 0) {
        av_log(s, AV_LOG_ERROR, "Error opening memory stream\n");
        return ret;
    }
    switch (encoding) {
    case ID3v2_ENCODING_ISO8859:
        while (left && ch) {
            ch = avio_r8(pb);
            PUT_UTF8(ch, tmp, avio_w8(dynbuf, tmp);)
            left--;
        }
        break;
    case ID3v2_ENCODING_UTF16BOM:
        if ((left -= 2) < 0) {
            av_log(s, AV_LOG_ERROR, "Cannot read BOM value, input too short\n");
            ffio_free_dyn_buf(&dynbuf);
            *dst = NULL;
            return AVERROR_INVALIDDATA;
        }
        switch (avio_rb16(pb)) {
        case 0xfffe:
            get = avio_rl16;
        case 0xfeff:
            break;
        default:
            av_log(s, AV_LOG_ERROR, "Incorrect BOM value\n");
            ffio_free_dyn_buf(&dynbuf);
            *dst = NULL;
            *maxread = left;
            return AVERROR_INVALIDDATA;
        }
    case ID3v2_ENCODING_UTF16BE:
        while ((left > 1) && ch) {
            GET_UTF16(ch, ((left -= 2) >= 0 ? get(pb) : 0), break;)
            PUT_UTF8(ch, tmp, avio_w8(dynbuf, tmp);)
        }
        if (left < 0)
            left += 2;  
        break;
    case ID3v2_ENCODING_UTF8:
        while (left && ch) {
            ch = avio_r8(pb);
            avio_w8(dynbuf, ch);
            left--;
        }
        break;
    default:
        av_log(s, AV_LOG_WARNING, "Unknown encoding\n");
    }
    if (ch)
        avio_w8(dynbuf, 0);
    avio_close_dyn_buf(dynbuf, dst);
    *maxread = left;
    return 0;
}
static void read_comment(AVFormatContext *s, AVIOContext *pb, int taglen,
                      AVDictionary **metadata)
{
    const char *key = "comment";
    uint8_t *dst;
    int encoding, dict_flags = AV_DICT_DONT_OVERWRITE | AV_DICT_DONT_STRDUP_VAL;
    av_unused int language;
    if (taglen < 4)
        return;
    encoding = avio_r8(pb);
    language = avio_rl24(pb);
    taglen -= 4;
    if (decode_str(s, pb, encoding, &dst, &taglen) < 0) {
        av_log(s, AV_LOG_ERROR, "Error reading comment frame, skipped\n");
        return;
    }
    if (dst && !*dst)
        av_freep(&dst);
    if (dst) {
        key = (const char *) dst;
        dict_flags |= AV_DICT_DONT_STRDUP_KEY;
    }
    if (decode_str(s, pb, encoding, &dst, &taglen) < 0) {
        av_log(s, AV_LOG_ERROR, "Error reading comment frame, skipped\n");
        if (dict_flags & AV_DICT_DONT_STRDUP_KEY)
            av_freep((void*)&key);
        return;
    }
    if (dst)
        av_dict_set(metadata, key, (const char *) dst, dict_flags);
}

static unsigned int get_size(AVIOContext *s, int len)
{
    int v = 0;
    while (len--)
        v = (v << 7) + (avio_r8(s) & 0x7F);
    return v;
}


static int is_tag(const char *buf, unsigned int len)
{
    if (!len)
        return 0;
    while (len--)
        if ((buf[len] < 'A' ||
             buf[len] > 'Z') &&
            (buf[len] < '0' ||
             buf[len] > '9'))
            return 0;
    return 1;
}
static int check_tag(AVIOContext *s, int offset, unsigned int len)
{
    char tag[4];
    if (len > 4 ||
        avio_seek(s, offset, SEEK_SET) < 0 ||
        avio_read(s, tag, len) < (int)len)
        return -1;
    else if (!AV_RB32(tag) || is_tag(tag, len))
        return 1;
    return 0;
}
static unsigned int size_to_syncsafe(unsigned int size)
{
    return (((size) & (0x7f <<  0)) >> 0) +
           (((size) & (0x7f <<  8)) >> 1) +
           (((size) & (0x7f << 16)) >> 2) +
           (((size) & (0x7f << 24)) >> 3);
}

void ff_metadata_conv(AVDictionary **pm, const AVMetadataConv *d_conv,const AVMetadataConv *s_conv)
{
    
    const AVMetadataConv *sc, *dc;
    AVDictionaryEntry *mtag = NULL;
    AVDictionary *dst = NULL;
    const char *key;
    if (d_conv == s_conv || !pm)
        return;
    while ((mtag = av_dict_get(*pm, "", mtag, AV_DICT_IGNORE_SUFFIX))) {
        key = mtag->key;
        if (s_conv)
            for (sc=s_conv; sc->native; sc++)
                if (!av_strcasecmp(key, sc->native)) {
                    key = sc->generic;
                    break;
                }
        if (d_conv)
            for (dc=d_conv; dc->native; dc++)
                if (!av_strcasecmp(key, dc->generic)) {
                    key = dc->native;
                    break;
                }
        av_dict_set(&dst, key, mtag->value, 0);
    }
    av_dict_free(pm);
    *pm = dst;
}

static void read_ttag(AVFormatContext *s, AVIOContext *pb, int taglen,AVDictionary **metadata, const char *key)
{
    uint8_t *dst;
    int encoding, dict_flags = AV_DICT_DONT_OVERWRITE | AV_DICT_DONT_STRDUP_VAL;
    unsigned genre;
    if (taglen < 1)
        return;
    encoding = avio_r8(pb);
    taglen--; 
    if (decode_str(s, pb, encoding, &dst, &taglen) < 0) {
        av_log(s, AV_LOG_ERROR, "Error reading frame %s, skipped\n", key);
        return;
    }
    if (!(strcmp(key, "TCON") && strcmp(key, "TCO"))                         &&
        (sscanf(dst, "(%d)", &genre) == 1 || sscanf(dst, "%d", &genre) == 1) &&
        genre <= ID3v1_GENRE_MAX) {
        av_freep(&dst);
        dst = av_strdup(ff_id3v1_genre_str[genre]);
    } else if (!(strcmp(key, "TXXX") && strcmp(key, "TXX"))) {
        
        key = dst;
        if (decode_str(s, pb, encoding, &dst, &taglen) < 0) {
            av_log(s, AV_LOG_ERROR, "Error reading frame %s, skipped\n", key);
            av_freep(&key);
            return;
        }
        dict_flags |= AV_DICT_DONT_STRDUP_KEY;
    } else if (!*dst)
        av_freep(&dst);
    if (dst)
        av_dict_set(metadata, key, dst, dict_flags);
}
static void free_chapter(void *obj)
{
    ID3v2ExtraMetaCHAP *chap = obj;
    av_freep(&chap->element_id);
    av_dict_free(&chap->meta);
}
static void read_chapter(AVFormatContext *s, AVIOContext *pb, int len, const char *ttag, ID3v2ExtraMeta **extra_meta, int isv34)
{
    int taglen;
    char tag[5];
    ID3v2ExtraMeta *new_extra = NULL;
    ID3v2ExtraMetaCHAP *chap  = NULL;
    new_extra = av_mallocz(sizeof(*new_extra));
    if (!new_extra)
        return;
    chap = &new_extra->data.chap;
    if (decode_str(s, pb, 0, &chap->element_id, &len) < 0)
        goto fail;
    if (len < 16)
        goto fail;
    chap->start = avio_rb32(pb);
    chap->end   = avio_rb32(pb);
    avio_skip(pb, 8);
    len -= 16;
    while (len > 10) {
        if (avio_read(pb, tag, 4) < 4)
            goto fail;
        tag[4] = 0;
        taglen = avio_rb32(pb);
        avio_skip(pb, 2);
        len -= 10;
        if (taglen < 0 || taglen > len)
            goto fail;
        if (tag[0] == 'T')
            read_ttag(s, pb, taglen, &chap->meta, tag);
        else
            avio_skip(pb, taglen);
        len -= taglen;
    }
    ff_metadata_conv(&chap->meta, NULL, ff_id3v2_34_metadata_conv);
    ff_metadata_conv(&chap->meta, NULL, ff_id3v2_4_metadata_conv);
    new_extra->tag  = "CHAP";
    new_extra->next = *extra_meta;
    *extra_meta     = new_extra;
    return;

fail:
    free_chapter(chap);
    av_freep(&new_extra);
}

static void free_priv(void *obj)
{
    ID3v2ExtraMetaPRIV *priv = obj;
    av_freep(&priv->owner);
    av_freep(&priv->data);
}
static void read_priv(AVFormatContext *s, AVIOContext *pb, int taglen,
                      const char *tag, ID3v2ExtraMeta **extra_meta, int isv34)
{
    ID3v2ExtraMeta *meta;
    ID3v2ExtraMetaPRIV *priv;
    meta = av_mallocz(sizeof(*meta));
    if (!meta)
        return;
    priv = &meta->data.priv;
    if (decode_str(s, pb, ID3v2_ENCODING_ISO8859, &priv->owner, &taglen) < 0)
        goto fail;
    priv->data = av_malloc(taglen);
    if (!priv->data)
        goto fail;
    priv->datasize = taglen;
    if (avio_read(pb, priv->data, priv->datasize) != priv->datasize)
        goto fail;
    meta->tag   = "PRIV";
    meta->next  = *extra_meta;
    *extra_meta = meta;
    return;

fail:
    free_priv(priv);
    av_freep(&meta);
}
static void free_geobtag(void *obj)
{
    ID3v2ExtraMetaGEOB *geob = obj;
    av_freep(&geob->mime_type);
    av_freep(&geob->file_name);
    av_freep(&geob->description);
    av_freep(&geob->data);
}
#define SIZE_SPECIFIER "zu"
static void read_geobtag(AVFormatContext *s, AVIOContext *pb, int taglen,
                         const char *tag, ID3v2ExtraMeta **extra_meta,
                         int isv34)
{
    ID3v2ExtraMetaGEOB *geob_data = NULL;
    ID3v2ExtraMeta *new_extra     = NULL;
    char encoding;
    unsigned int len;
    if (taglen < 1)
        return;
    new_extra = av_mallocz(sizeof(ID3v2ExtraMeta));
    if (!new_extra) {
        av_log(s, AV_LOG_ERROR, "Failed to alloc %"SIZE_SPECIFIER" bytes\n",
               sizeof(ID3v2ExtraMeta));
        return;
    }
    geob_data = &new_extra->data.geob;
    
    encoding = avio_r8(pb);
    taglen--;
    
    if (decode_str(s, pb, ID3v2_ENCODING_ISO8859, &geob_data->mime_type,
                   &taglen) < 0 ||
        taglen <= 0)
        goto fail;
    
    if (decode_str(s, pb, encoding, &geob_data->file_name, &taglen) < 0 ||
        taglen <= 0)
        goto fail;
    
    if (decode_str(s, pb, encoding, &geob_data->description, &taglen) < 0 ||
        taglen < 0)
        goto fail;
    if (taglen) {
        
        geob_data->data = av_malloc(taglen);
        if (!geob_data->data) {
            av_log(s, AV_LOG_ERROR, "Failed to alloc %d bytes\n", taglen);
            goto fail;
        }
        if ((len = avio_read(pb, geob_data->data, taglen)) < taglen)
            av_log(s, AV_LOG_WARNING,
                   "Error reading GEOB frame, data truncated.\n");
        geob_data->datasize = len;
    } else {
        geob_data->data     = NULL;
        geob_data->datasize = 0;
    }
    
    new_extra->tag  = "GEOB";
    new_extra->next = *extra_meta;
    *extra_meta     = new_extra;
    return;

fail:
    av_log(s, AV_LOG_ERROR, "Error reading frame %s, skipped\n", tag);
    free_geobtag(geob_data);
    av_free(new_extra);
    return;
}

static void rstrip_spaces(char *buf)
{
    size_t len = strlen(buf);
    while (len > 0 && buf[len - 1] == ' ')
        buf[--len] = 0;
}
static void free_apic(void *obj)
{
    ID3v2ExtraMetaAPIC *apic = obj;
    av_buffer_unref(&apic->buf);
    av_freep(&apic->description);
}
static void read_apic(AVFormatContext *s, AVIOContext *pb, int taglen,const char *tag, ID3v2ExtraMeta **extra_meta,int isv34)
{
    int enc, pic_type;
    char mimetype[64] = {0};
    const CodecMime *mime     = ff_id3v2_mime_tags;
    enum AVCodecID id         = AV_CODEC_ID_NONE;
    ID3v2ExtraMetaAPIC *apic  = NULL;
    ID3v2ExtraMeta *new_extra = NULL;
    int64_t end               = avio_tell(pb) + taglen;
    if (taglen <= 4 || (!isv34 && taglen <= 6))
        goto fail;
    new_extra = av_mallocz(sizeof(*new_extra));
    if (!new_extra)
        goto fail;
    apic = &new_extra->data.apic;
    enc = avio_r8(pb);
    taglen--;
    
    if (isv34) {
        taglen -= avio_get_str(pb, taglen, mimetype, sizeof(mimetype));
    } else {
        if (avio_read(pb, mimetype, 3) < 0)
            goto fail;
        mimetype[3] = 0;
        taglen    -= 3;
    }
    while (mime->id != AV_CODEC_ID_NONE) {
        if (!av_strncasecmp(mime->str, mimetype, sizeof(mimetype))) {
            id = mime->id;
            break;
        }
        mime++;
    }
    if (id == AV_CODEC_ID_NONE) {
        av_log(s, AV_LOG_WARNING,
               "Unknown attached picture mimetype: %s, skipping.\n", mimetype);
        goto fail;
    }
    apic->id = id;
    
    pic_type = avio_r8(pb);
    taglen--;
    if (pic_type < 0 || pic_type >= FF_ARRAY_ELEMS(ff_id3v2_picture_types)) {
        av_log(s, AV_LOG_WARNING, "Unknown attached picture type %d.\n",
               pic_type);
        pic_type = 0;
    }
    apic->type = ff_id3v2_picture_types[pic_type];
    
    if (decode_str(s, pb, enc, &apic->description, &taglen) < 0) {
        av_log(s, AV_LOG_ERROR,
               "Error decoding attached picture description.\n");
        goto fail;
    }
    apic->buf = av_buffer_alloc(taglen + AV_INPUT_BUFFER_PADDING_SIZE);
    if (!apic->buf || !taglen || avio_read(pb, apic->buf->data, taglen) != taglen)
        goto fail;
    memset(apic->buf->data + taglen, 0, AV_INPUT_BUFFER_PADDING_SIZE);
    new_extra->tag  = "APIC";
    new_extra->next = *extra_meta;
    *extra_meta     = new_extra;
    rstrip_spaces(apic->description);
    return;

fail:
    if (apic)
        free_apic(apic);
    av_freep(&new_extra);
    avio_seek(pb, end, SEEK_SET);
}
static const ID3v2EMFunc id3v2_extra_meta_funcs[] = {
    { "GEO", "GEOB", read_geobtag, free_geobtag },
    { "PIC", "APIC", read_apic,    free_apic    },
    { "CHAP","CHAP", read_chapter, free_chapter },
    { "PRIV","PRIV", read_priv,    free_priv    },
    { NULL }
};
static const ID3v2EMFunc *get_extra_meta_func(const char *tag, int isv34)
{
    int i = 0;
    while (id3v2_extra_meta_funcs[i].tag3) {
        if (tag && !memcmp(tag,
                    (isv34 ? id3v2_extra_meta_funcs[i].tag4 :
                             id3v2_extra_meta_funcs[i].tag3),
                    (isv34 ? 4 : 3)))
            return &id3v2_extra_meta_funcs[i];
        i++;
    }
    return NULL;
}
#define SHORT_SEEK_THRESHOLD (256 * 1024)
int ffio_init_context(AVIOContext *s,
                  unsigned char *buffer,
                  int buffer_size,
                  int write_flag,
                  void *opaque,
                  int (*read_packet)(void *opaque, uint8_t *buf, int buf_size),
                  int (*write_packet)(void *opaque, uint8_t *buf, int buf_size),
                  int64_t (*seek)(void *opaque, int64_t offset, int whence))
{
    memset(s, 0, sizeof(AVIOContext));
    s->buffer      = buffer;
    s->orig_buffer_size =
    s->buffer_size = buffer_size;
    s->buf_ptr     = buffer;
    s->buf_ptr_max = buffer;
    s->opaque      = opaque;
    s->direct      = 0;
    url_resetbuf(s, write_flag ? AVIO_FLAG_WRITE : AVIO_FLAG_READ);
    s->write_packet    = write_packet;
    s->read_packet     = read_packet;
    s->seek            = seek;
    s->pos             = 0;
    s->eof_reached     = 0;
    s->error           = 0;
    s->seekable        = seek ? AVIO_SEEKABLE_NORMAL : 0;
    s->min_packet_size = 0;
    s->max_packet_size = 0;
    s->update_checksum = NULL;
    s->short_seek_threshold = SHORT_SEEK_THRESHOLD;
    if (!read_packet && !write_flag) {
        s->pos     = buffer_size;
        s->buf_end = s->buffer + buffer_size;
    }
    s->read_pause = NULL;
    s->read_seek  = NULL;
    s->write_data_type       = NULL;
    s->ignore_boundary_point = 0;
    s->current_type          = AVIO_DATA_MARKER_UNKNOWN;
    s->last_time             = AV_NOPTS_VALUE;
    s->short_seek_get        = NULL;
    s->written               = 0;
    return 0;
}
static void read_uslt(AVFormatContext *s, AVIOContext *pb, int taglen,
                      AVDictionary **metadata)
{
    uint8_t lang[4];
    uint8_t *descriptor = NULL; 
    uint8_t *text;
    char *key;
    int encoding;
    int ok = 0;
    if (taglen < 1)
        goto error;
    encoding = avio_r8(pb);
    taglen--;
    if (avio_read(pb, lang, 3) < 3)
        goto error;
    lang[3] = '\0';
    taglen -= 3;
    if (decode_str(s, pb, encoding, &descriptor, &taglen) < 0)
        goto error;
    if (decode_str(s, pb, encoding, &text, &taglen) < 0)
        goto error;
    key = av_asprintf("lyrics-%s%s%s", descriptor[0] ? (char *)descriptor : "",
                                       descriptor[0] ? "-" : "",
                                       lang);
    if (!key) {
        av_free(text);
        goto error;
    }
    av_dict_set(metadata, key, text,
                AV_DICT_DONT_STRDUP_KEY | AV_DICT_DONT_STRDUP_VAL);
    ok = 1;
error:
    if (!ok)
        av_log(s, AV_LOG_ERROR, "Error reading lyrics, skipped\n");
    av_free(descriptor);
}
static void id3v2_parse(AVIOContext *pb, AVDictionary **metadata,AVFormatContext *s, int len, uint8_t version,uint8_t flags, ID3v2ExtraMeta **extra_meta)
{
    int isv34, unsync;
    unsigned tlen;
    char tag[5];
    int64_t next, end = avio_tell(pb) + len;
    int taghdrlen;
    const char *reason = NULL;
    AVIOContext pb_local;
    AVIOContext *pbx;
    unsigned char *buffer = NULL;
    int buffer_size       = 0;
    const ID3v2EMFunc *extra_func = NULL;
    unsigned char *uncompressed_buffer = NULL;
    av_unused int uncompressed_buffer_size = 0;
    const char *comm_frame;
    av_log(s, AV_LOG_DEBUG, "id3v2 ver:%d flags:%02X len:%d\n", version, flags, len);
    switch (version) {
    case 2:
        if (flags & 0x40) {
            reason = "compression";
            goto error;
        }
        isv34     = 0;
        taghdrlen = 6;
        comm_frame = "COM";
        break;
    case 3:
    case 4:
        isv34     = 1;
        taghdrlen = 10;
        comm_frame = "COMM";
        break;
    default:
        reason = "version";
        goto error;
    }
    unsync = flags & 0x80;
    if (isv34 && flags & 0x40) { 
        int extlen = get_size(pb, 4);
        if (version == 4)
            
            extlen -= 4;
        if (extlen < 0) {
            reason = "invalid extended header length";
            goto error;
        }
        avio_skip(pb, extlen);
        len -= extlen + 4;
        if (len < 0) {
            reason = "extended header too long.";
            goto error;
        }
    }
    while (len >= taghdrlen) {
        unsigned int tflags = 0;
        int tunsync         = 0;
        int tcomp           = 0;
        int tencr           = 0;
        unsigned long av_unused dlen;
        if (isv34) {
            if (avio_read(pb, tag, 4) < 4)
                break;
            tag[4] = 0;
            if (version == 3) {
                tlen = avio_rb32(pb);
            } else {
                
                tlen = avio_rb32(pb);
                if (tlen > 0x7f) {
                    if (tlen < len) {
                        int64_t cur = avio_tell(pb);
                        if (ffio_ensure_seekback(pb, 2 ))
                            break;
                        if (check_tag(pb, cur + 2 + size_to_syncsafe(tlen), 4) == 1)
                            tlen = size_to_syncsafe(tlen);
                        else if (check_tag(pb, cur + 2 + tlen, 4) != 1)
                            break;
                        avio_seek(pb, cur, SEEK_SET);
                    } else
                        tlen = size_to_syncsafe(tlen);
                }
            }
            tflags  = avio_rb16(pb);
            tunsync = tflags & ID3v2_FLAG_UNSYNCH;
        } else {
            if (avio_read(pb, tag, 3) < 3)
                break;
            tag[3] = 0;
            tlen   = avio_rb24(pb);
        }
        if (tlen > (1<<28))
            break;
        len -= taghdrlen + tlen;
        if (len < 0)
            break;
        next = avio_tell(pb) + tlen;
        if (!tlen) {
            if (tag[0])
                av_log(s, AV_LOG_DEBUG, "Invalid empty frame %s, skipping.\n",
                       tag);
            continue;
        }
        if (tflags & ID3v2_FLAG_DATALEN) {
            if (tlen < 4)
                break;
            dlen = avio_rb32(pb);
            tlen -= 4;
        } else
            dlen = tlen;
        tcomp = tflags & ID3v2_FLAG_COMPRESSION;
        tencr = tflags & ID3v2_FLAG_ENCRYPTION;
        
        if (tencr || (!CONFIG_ZLIB && tcomp)) {
            const char *type;
            if (!tcomp)
                type = "encrypted";
            else if (!tencr)
                type = "compressed";
            else
                type = "encrypted and compressed";
            av_log(s, AV_LOG_WARNING, "Skipping %s ID3v2 frame %s.\n", type, tag);
            avio_skip(pb, tlen);
        
        } else if (tag[0] == 'T' ||
                   !memcmp(tag, "USLT", 4) ||
                   !strcmp(tag, comm_frame) ||
                   (extra_meta &&
                    (extra_func = get_extra_meta_func(tag, isv34)))) {
            pbx = pb;
            if (unsync || tunsync || tcomp) {
                av_fast_malloc(&buffer, &buffer_size, tlen);
                if (!buffer) {
                    av_log(s, AV_LOG_ERROR, "Failed to alloc %d bytes\n", tlen);
                    goto seek;
                }
            }
            if (unsync || tunsync) {
                uint8_t *b = buffer;
                uint8_t *t = buffer;
                uint8_t *end = t + tlen;
                if (avio_read(pb, buffer, tlen) != tlen) {
                    av_log(s, AV_LOG_ERROR, "Failed to read tag data\n");
                    goto seek;
                }
                while (t != end) {
                    *b++ = *t++;
                    if (t != end && t[-1] == 0xff && !t[0])
                        t++;
                }
                ffio_init_context(&pb_local, buffer, b - buffer, 0, NULL, NULL, NULL, NULL);
                tlen = b - buffer;
                pbx  = &pb_local; 
            }
#if CONFIG_ZLIB
                if (tcomp) {
                    int err;
                    av_log(s, AV_LOG_DEBUG, "Compresssed frame %s tlen=%d dlen=%ld\n", tag, tlen, dlen);
                    av_fast_malloc(&uncompressed_buffer, &uncompressed_buffer_size, dlen);
                    if (!uncompressed_buffer) {
                        av_log(s, AV_LOG_ERROR, "Failed to alloc %ld bytes\n", dlen);
                        goto seek;
                    }
                    if (!(unsync || tunsync)) {
                        err = avio_read(pb, buffer, tlen);
                        if (err < 0) {
                            av_log(s, AV_LOG_ERROR, "Failed to read compressed tag\n");
                            goto seek;
                        }
                        tlen = err;
                    }
                    err = uncompress(uncompressed_buffer, &dlen, buffer, tlen);
                    if (err != Z_OK) {
                        av_log(s, AV_LOG_ERROR, "Failed to uncompress tag: %d\n", err);
                        goto seek;
                    }
                    ffio_init_context(&pb_local, uncompressed_buffer, dlen, 0, NULL, NULL, NULL, NULL);
                    tlen = dlen;
                    pbx = &pb_local; 
                }
#endif
            if (tag[0] == 'T')
                
                read_ttag(s, pbx, tlen, metadata, tag);
            else if (!memcmp(tag, "USLT", 4))
                read_uslt(s, pbx, tlen, metadata);
            else if (!strcmp(tag, comm_frame))
                read_comment(s, pbx, tlen, metadata);
            else
                
                extra_func->read(s, pbx, tlen, tag, extra_meta, isv34);
        } else if (!tag[0]) {
            if (tag[1])
                av_log(s, AV_LOG_WARNING, "invalid frame id, assuming padding\n");
            avio_skip(pb, tlen);
            break;
        }
        
seek:
        avio_seek(pb, next, SEEK_SET);
    }
    
    if (version == 4 && flags & 0x10)
        end += 10;

error:
    if (reason)
        av_log(s, AV_LOG_INFO, "ID3v2.%d tag skipped, cannot handle %s\n",
               version, reason);
    avio_seek(pb, end, SEEK_SET);
    av_free(buffer);
    av_free(uncompressed_buffer);
    return;
}
static const AVMetadataConv id3v2_2_metadata_conv[] = {
    { "TAL", "album"        },
    { "TCO", "genre"        },
    { "TCP", "compilation"  },
    { "TT2", "title"        },
    { "TEN", "encoded_by"   },
    { "TP1", "artist"       },
    { "TP2", "album_artist" },
    { "TP3", "performer"    },
    { "TRK", "track"        },
    { 0 }
};
static int is_number(const char *str)
{
    while (*str >= '0' && *str <= '9')
        str++;
    return !*str;
}
static AVDictionaryEntry *get_date_tag(AVDictionary *m, const char *tag)
{
    AVDictionaryEntry *t;
    if ((t = av_dict_get(m, tag, NULL, AV_DICT_MATCH_CASE)) &&
        strlen(t->value) == 4 && is_number(t->value))
        return t;
    return NULL;
}
static void merge_date(AVDictionary **m)
{
    AVDictionaryEntry *t;
    char date[17] = { 0 };      
    if (!(t = get_date_tag(*m, "TYER")) &&
        !(t = get_date_tag(*m, "TYE")))
        return;
    av_strlcpy(date, t->value, 5);
    av_dict_set(m, "TYER", NULL, 0);
    av_dict_set(m, "TYE", NULL, 0);
    if (!(t = get_date_tag(*m, "TDAT")) &&
        !(t = get_date_tag(*m, "TDA")))
        goto finish;
    snprintf(date + 4, sizeof(date) - 4, "-%.2s-%.2s", t->value + 2, t->value);
    av_dict_set(m, "TDAT", NULL, 0);
    av_dict_set(m, "TDA", NULL, 0);
    if (!(t = get_date_tag(*m, "TIME")) &&
        !(t = get_date_tag(*m, "TIM")))
        goto finish;
    snprintf(date + 10, sizeof(date) - 10,
             " %.2s:%.2s", t->value, t->value + 2);
    av_dict_set(m, "TIME", NULL, 0);
    av_dict_set(m, "TIM", NULL, 0);

finish:
    if (date[0])
        av_dict_set(m, "date", date, 0);
}
static void id3v2_read_internal(AVIOContext *pb, AVDictionary **metadata,AVFormatContext *s, const char *magic,ID3v2ExtraMeta **extra_meta, int64_t max_search_size)
{
    int len, ret;
    uint8_t buf[ID3v2_HEADER_SIZE];
    int found_header;
    int64_t start, off;
    if (max_search_size && max_search_size < ID3v2_HEADER_SIZE)
        return;
    start = avio_tell(pb);
    do {
        
        off = avio_tell(pb);
        if (max_search_size && off - start >= max_search_size - ID3v2_HEADER_SIZE) {
            avio_seek(pb, off, SEEK_SET);
            break;
        }
        ret = ffio_ensure_seekback(pb, ID3v2_HEADER_SIZE);
        if (ret >= 0)
            ret = avio_read(pb, buf, ID3v2_HEADER_SIZE);
        if (ret != ID3v2_HEADER_SIZE) {
            avio_seek(pb, off, SEEK_SET);
            break;
        }
        found_header = ff_id3v2_match(buf, magic);
        if (found_header) {
            
            len = ((buf[6] & 0x7f) << 21) |
                  ((buf[7] & 0x7f) << 14) |
                  ((buf[8] & 0x7f) << 7) |
                   (buf[9] & 0x7f);
            id3v2_parse(pb, metadata, s, len, buf[3], buf[5], extra_meta);
        } else {
            avio_seek(pb, off, SEEK_SET);
        }
    } while (found_header);
    ff_metadata_conv(metadata, NULL, ff_id3v2_34_metadata_conv);
    ff_metadata_conv(metadata, NULL, id3v2_2_metadata_conv);
    ff_metadata_conv(metadata, NULL, ff_id3v2_4_metadata_conv);
    merge_date(metadata);
}

void ff_id3v2_read_dict(AVIOContext *pb, AVDictionary **metadata,
                        const char *magic, ID3v2ExtraMeta **extra_meta)
{
    id3v2_read_internal(pb, metadata, NULL, magic, extra_meta, 0);
}
#define AV_RB64(p) AV_RB(64, p)
#define PNGSIG 0x89504e470d0a1a0a
#define MNGSIG 0x8a4d4e470d0a1a0
int ff_id3v2_parse_apic(AVFormatContext *s, ID3v2ExtraMeta *extra_meta)
{
    ID3v2ExtraMeta *cur;
    for (cur = extra_meta; cur; cur = cur->next) {
        ID3v2ExtraMetaAPIC *apic;
        AVStream *st;
        if (strcmp(cur->tag, "APIC"))
            continue;
        apic = &cur->data.apic;
        if (!(st = avformat_new_stream(s, NULL)))
            return AVERROR(ENOMEM);
        st->disposition      |= AV_DISPOSITION_ATTACHED_PIC;
        st->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
        st->codecpar->codec_id   = apic->id;
        if (AV_RB64(apic->buf->data) == PNGSIG)
            st->codecpar->codec_id = AV_CODEC_ID_PNG;
        if (apic->description[0])
            av_dict_set(&st->metadata, "title", apic->description, 0);
        av_dict_set(&st->metadata, "comment", apic->type, 0);
        av_init_packet(&st->attached_pic);
        st->attached_pic.buf          = apic->buf;
        st->attached_pic.data         = apic->buf->data;
        st->attached_pic.size         = apic->buf->size - AV_INPUT_BUFFER_PADDING_SIZE;
        st->attached_pic.stream_index = st->index;
        st->attached_pic.flags       |= AV_PKT_FLAG_KEY;
        apic->buf = NULL;
    }
    return 0;
}
#ifdef __GNUC__
#define dynarray_add(tab, nb_ptr, elem)\
do {\
    __typeof__(tab) _tab = (tab);\
    __typeof__(elem) _elem = (elem);\
    (void)sizeof(**_tab == _elem); \
    av_dynarray_add(_tab, nb_ptr, _elem);\
} while(0)
#else
#define dynarray_add(tab, nb_ptr, elem)\
do {\
    av_dynarray_add((tab), nb_ptr, (elem));\
} while(0)
#endif
AVChapter *avpriv_new_chapter(AVFormatContext *s, int id, AVRational time_base,
                              int64_t start, int64_t end, const char *title)
{
    AVChapter *chapter = NULL;
    int i;
    if (end != AV_NOPTS_VALUE && start > end) {
        av_log(s, AV_LOG_ERROR, "Chapter end time %"PRId64" before start %"PRId64"\n", end, start);
        return NULL;
    }
    for (i = 0; i < s->nb_chapters; i++)
        if (s->chapters[i]->id == id)
            chapter = s->chapters[i];
    if (!chapter) {
        chapter = av_mallocz(sizeof(AVChapter));
        if (!chapter)
            return NULL;
        dynarray_add(&s->chapters, &s->nb_chapters, chapter);
    }
    av_dict_set(&chapter->metadata, "title", title, 0);
    chapter->id        = id;
    chapter->time_base = time_base;
    chapter->start     = start;
    chapter->end       = end;
    return chapter;
}
int ff_id3v2_parse_chapters(AVFormatContext *s, ID3v2ExtraMeta *extra_meta)
{
    int ret = 0;
    ID3v2ExtraMeta *cur;
    AVRational time_base = {1, 1000};
    ID3v2ExtraMetaCHAP **chapters = NULL;
    int num_chapters = 0;
    int i;
    for (cur = extra_meta; cur; cur = cur->next) {
        ID3v2ExtraMetaCHAP *chap;
        if (strcmp(cur->tag, "CHAP"))
            continue;
        chap = &cur->data.chap;
        if ((ret = av_dynarray_add_nofree(&chapters, &num_chapters, chap)) < 0)
            goto end;
    }
    for (i = 0; i < (num_chapters / 2); i++) {
        ID3v2ExtraMetaCHAP *right;
        int right_index;
        right_index = (num_chapters - 1) - i;
        right = chapters[right_index];
        chapters[right_index] = chapters[i];
        chapters[i] = right;
    }
    for (i = 0; i < num_chapters; i++) {
        ID3v2ExtraMetaCHAP *chap;
        AVChapter *chapter;
        chap = chapters[i];
        chapter = avpriv_new_chapter(s, i, time_base, chap->start, chap->end, chap->element_id);
        if (!chapter)
            continue;
        if ((ret = av_dict_copy(&chapter->metadata, chap->meta, 0)) < 0)
            goto end;
    }

end:
    av_freep(&chapters);
    return ret;
}
#define ID3v2_PRIV_METADATA_PREFIX "id3v2_priv."
int ff_id3v2_parse_priv_dict(AVDictionary **metadata, ID3v2ExtraMeta *extra_meta)
{
    ID3v2ExtraMeta *cur;
    int dict_flags = AV_DICT_DONT_OVERWRITE | AV_DICT_DONT_STRDUP_KEY | AV_DICT_DONT_STRDUP_VAL;
    for (cur = extra_meta; cur; cur = cur->next) {
        if (!strcmp(cur->tag, "PRIV")) {
            ID3v2ExtraMetaPRIV *priv = &cur->data.priv;
            AVBPrint bprint;
            char *escaped, *key;
            int i, ret;
            if ((key = av_asprintf(ID3v2_PRIV_METADATA_PREFIX "%s", priv->owner)) == NULL) {
                return AVERROR(ENOMEM);
            }
            av_bprint_init(&bprint, priv->datasize + 1, AV_BPRINT_SIZE_UNLIMITED);
            for (i = 0; i < priv->datasize; i++) {
                if (priv->data[i] < 32 || priv->data[i] > 126 || priv->data[i] == '\\') {
                    av_bprintf(&bprint, "\\x%02x", priv->data[i]);
                } else {
                    av_bprint_chars(&bprint, priv->data[i], 1);
                }
            }
            if ((ret = av_bprint_finalize(&bprint, &escaped)) < 0) {
                av_free(key);
                return ret;
            }
            if ((ret = av_dict_set(metadata, key, escaped, dict_flags)) < 0) {
                return ret;
            }
        }
    }
    return 0;
}
int ff_id3v2_parse_priv(AVFormatContext *s, ID3v2ExtraMeta *extra_meta)
{
    return ff_id3v2_parse_priv_dict(&s->metadata, extra_meta);
}
void ff_id3v2_free_extra_meta(ID3v2ExtraMeta **extra_meta)
{
    ID3v2ExtraMeta *current = *extra_meta, *next;
    const ID3v2EMFunc *extra_func;
    while (current) {
        if ((extra_func = get_extra_meta_func(current->tag, 1)))
            extra_func->free(&current->data);
        next = current->next;
        av_freep(&current);
        current = next;
    }
    *extra_meta = NULL;
}
static int update_stream_avctx(AVFormatContext *s)
{
    int i, ret;
    for (i = 0; i < s->nb_streams; i++) {
        AVStream *st = s->streams[i];
        if (!st->internal->need_context_update)
            continue;
        
        if (st->parser && st->internal->avctx->codec_id != st->codecpar->codec_id) {
            av_parser_close(st->parser);
            st->parser = NULL;
        }
        
        ret = avcodec_parameters_to_context(st->internal->avctx, st->codecpar);
        if (ret < 0)
            return ret;
#if FF_API_LAVF_AVCTX
        
        ret = avcodec_parameters_to_context(st->codec, st->codecpar);
        if (ret < 0)
            return ret;
#endif
        st->internal->need_context_update = 0;
    }
    return 0;
}
#define RAW_PACKET_BUFFER_SIZE 2500000
int avformat_open_input(AVFormatContext **ps, const char *filename,ff_const59 AVInputFormat *fmt, AVDictionary **options)
{
    AVFormatContext *s = *ps;
    int i, ret = 0;
    AVDictionary *tmp = NULL;
    ID3v2ExtraMeta *id3v2_extra_meta = NULL;

    if (s->av_class == NULL) {
        av_log(NULL, AV_LOG_ERROR, "Input context has not been properly allocated by avformat_alloc_context() and is not NULL either\n");
        return AVERROR(EINVAL);
    }
    if (fmt != NULL)
        s->iformat = fmt;
    if (options != NULL)
        av_dict_copy(&tmp, *options, 0);
    if (s->pb != NULL) 
        s->flags |= AVFMT_FLAG_CUSTOM_IO;
    ret = av_opt_set_dict(s, &tmp);
    if (ret < 0)
        goto fail;
    if (!(s->url = av_strdup(filename ? filename : ""))) {
        ret = AVERROR(ENOMEM);
        goto fail;
    }
#if FF_API_FORMAT_FILENAME
    av_strlcpy(s->filename, filename ? filename : "", sizeof(s->filename));
#endif
    ret = init_input(s, filename, &tmp);
    if (ret < 0)
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
    
    if (s->iformat->flags & AVFMT_NEEDNUMBER) {
        if (!av_filename_number_test(filename)) {
            ret = AVERROR(EINVAL);
            goto fail;
        }
    }
    s->duration = s->start_time = AV_NOPTS_VALUE;
    
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
    if (id3v2_extra_meta != NULL) {
        if (!strcmp(s->iformat->name, "mp3") || !strcmp(s->iformat->name, "aac") ||
            !strcmp(s->iformat->name, "tta") || !strcmp(s->iformat->name, "wav")) {
            ret = ff_id3v2_parse_apic(s, id3v2_extra_meta);
            if (ret < 0)
                goto close;
            ret = ff_id3v2_parse_chapters(s, id3v2_extra_meta);
            if (ret < 0)
                goto close;
            ret = ff_id3v2_parse_priv(s, id3v2_extra_meta);
            if ( ret< 0)
                goto close;
        } else
            av_log(s, AV_LOG_DEBUG, "demuxer does not support additional id3 data, skipping\n");
    }
    ff_id3v2_free_extra_meta(&id3v2_extra_meta);
    ret = avformat_queue_attached_pictures(s);
    if (ret < 0)
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

int ff_check_interrupt(AVIOInterruptCB *cb)
{
    if (cb && cb->callback)
        return cb->callback(cb->opaque);
    return 0;
}

static inline int retry_transfer_wrapper(URLContext *h, uint8_t *buf,int size, int size_min, int (*transfer_func)(URLContext *h,uint8_t *buf,int size))
{
    int ret, len;
    int fast_retries = 5;
    int64_t wait_since = 0;

    len = 0;
    while (len < size_min) {
        if (ff_check_interrupt(&h->interrupt_callback))
            return AVERROR_EXIT;
        ret = transfer_func(h, buf + len, size - len);
        if (ret == AVERROR(EINTR))
            continue;
        if (h->flags & AVIO_FLAG_NONBLOCK)
            return ret;
        if (ret == AVERROR(EAGAIN)) {
            ret = 0;
            if (fast_retries) {
                fast_retries--;
            } else {
                if (h->rw_timeout) {
                    if (!wait_since)
                        wait_since = av_gettime_relative();
                    else if (av_gettime_relative() > wait_since + h->rw_timeout)
                        return AVERROR(EIO);
                }
                av_usleep(1000);
            }
        } else if (ret == AVERROR_EOF)
            return (len > 0) ? len : AVERROR_EOF;
        else if (ret < 0)
            return ret;
        if (ret) {
            fast_retries = FFMAX(fast_retries, 2);
            wait_since = 0;
        }
        len += ret;
    }
    return len;
}
int ffurl_read(URLContext *h, unsigned char *buf, int size)
{
    if (!(h->flags & AVIO_FLAG_READ))
        return AVERROR(EIO);
    return retry_transfer_wrapper(h, buf, size, 1, h->prot->url_read);
}

int ffurl_write(URLContext *h, const unsigned char *buf, int size)
{
    if (!(h->flags & AVIO_FLAG_WRITE))
        return AVERROR(EIO);
    
    if (h->max_packet_size && size > h->max_packet_size)
        return AVERROR(EIO);

    return retry_transfer_wrapper(h, (unsigned char *)buf, size, size,
                                  (int (*)(struct URLContext *, uint8_t *, int))
                                  h->prot->url_write);
}
int64_t ffurl_seek(URLContext *h, int64_t pos, int whence)
{
    int64_t ret;

    if (!h->prot->url_seek)
        return AVERROR(ENOSYS);
    ret = h->prot->url_seek(h, pos, whence & ~AVSEEK_FORCE);
    return ret;
}

int ffurl_get_short_seek(URLContext *h)
{
    if (!h || !h->prot || !h->prot->url_get_short_seek)
        return AVERROR(ENOSYS);
    return h->prot->url_get_short_seek(h);
}

static void *ff_avio_child_next(void *obj, void *prev)
{
    AVIOContext *s = obj;
    return prev ? NULL : s->opaque;
}

static const char *urlcontext_to_name(void *ptr)
{
    URLContext *h = (URLContext *)ptr;
    if (h->prot)
        return h->prot->name;
    else
        return "NULL";
}
static void *urlcontext_child_next(void *obj, void *prev)
{
    URLContext *h = obj;
    if (prev == NULL && h->priv_data != NULL && h->prot->priv_data_class != NULL)
        return h->priv_data;
    return NULL;
}

#ifndef S_ISFIFO
#  ifdef S_IFIFO
#    define S_ISFIFO(m) (((m) & S_IFMT) == S_IFIFO)
#  else
#    define S_ISFIFO(m) 0
#  endif
#endif


#ifndef S_ISLNK
#  ifdef S_IFLNK
#    define S_ISLNK(m) (((m) & S_IFLNK) == S_IFLNK)
#  else
#    define S_ISLNK(m) 0
#  endif
#endif


#ifndef S_ISSOCK
#  ifdef S_IFSOCK
#    define S_ISSOCK(m) (((m) & S_IFMT) == S_IFSOCK)
#  else
#    define S_ISSOCK(m) 0
#  endif
#endif



typedef struct FileContext {
    const AVClass *class;
    int fd;
    int trunc;
    int blocksize;
    int follow;
    int seekable;
// #if HAVE_DIRENT_H
//     DIR *dir;
// #endif
} FileContext;

static const AVOption file_options[] = {
    { "truncate", "truncate existing files on write", offsetof(FileContext, trunc), AV_OPT_TYPE_BOOL, { .i64 = 1 }, 0, 1, AV_OPT_FLAG_ENCODING_PARAM },
    { "blocksize", "set I/O operation maximum block size", offsetof(FileContext, blocksize), AV_OPT_TYPE_INT, { .i64 = INT_MAX }, 1, INT_MAX, AV_OPT_FLAG_ENCODING_PARAM },
    { "follow", "Follow a file as it is being written", offsetof(FileContext, follow), AV_OPT_TYPE_INT, { .i64 = 0 }, 0, 1, AV_OPT_FLAG_DECODING_PARAM },
    { "seekable", "Sets if the file is seekable", offsetof(FileContext, seekable), AV_OPT_TYPE_INT, { .i64 = -1 }, -1, 0, AV_OPT_FLAG_DECODING_PARAM | AV_OPT_FLAG_ENCODING_PARAM },
    { NULL }
};

static const AVOption pipe_options[] = {
    { "blocksize", "set I/O operation maximum block size", offsetof(FileContext, blocksize), AV_OPT_TYPE_INT, { .i64 = INT_MAX }, 1, INT_MAX, AV_OPT_FLAG_ENCODING_PARAM },
    { NULL }
};

static const AVClass file_class = {
    .class_name = "file",
    .item_name  = av_default_item_name,
    .option     = file_options,
    .version    = LIBAVUTIL_VERSION_INT,
};

static const AVClass pipe_class = {
    .class_name = "pipe",
    .item_name  = av_default_item_name,
    .option     = pipe_options,
    .version    = LIBAVUTIL_VERSION_INT,
};

static int file_read(URLContext *h, unsigned char *buf, int size)
{
    FileContext *c = h->priv_data;
    int ret;
    size = FFMIN(size, c->blocksize);
    ret = read(c->fd, buf, size);
    if (ret == 0 && c->follow)
        return AVERROR(EAGAIN);
    if (ret == 0)
        return AVERROR_EOF;
    return (ret == -1) ? AVERROR(errno) : ret;
}

static int file_write(URLContext *h, const unsigned char *buf, int size)
{
    FileContext *c = h->priv_data;
    int ret;
    size = FFMIN(size, c->blocksize);
    ret = write(c->fd, buf, size);
    return (ret == -1) ? AVERROR(errno) : ret;
}

static int file_get_handle(URLContext *h)
{
    FileContext *c = h->priv_data;
    return c->fd;
}

static int file_check(URLContext *h, int mask)
{
    int ret = 0;
    const char *filename = h->filename;
    av_strstart(filename, "file:", &filename);

    {
#if HAVE_ACCESS && defined(R_OK)
    if (access(filename, F_OK) < 0)
        return AVERROR(errno);
    if (mask&AVIO_FLAG_READ)
        if (access(filename, R_OK) >= 0)
            ret |= AVIO_FLAG_READ;
    if (mask&AVIO_FLAG_WRITE)
        if (access(filename, W_OK) >= 0)
            ret |= AVIO_FLAG_WRITE;
#else
    struct stat st;
#   ifndef _WIN32
    ret = stat(filename, &st);
#   else
    ret = win32_stat(filename, &st);
#   endif
    if (ret < 0)
        return AVERROR(errno);

    ret |= st.st_mode&S_IRUSR ? mask&AVIO_FLAG_READ  : 0;
    ret |= st.st_mode&S_IWUSR ? mask&AVIO_FLAG_WRITE : 0;
#endif
    }
    return ret;
}

static int file_delete(URLContext *h)
{
#if HAVE_UNISTD_H
    int ret;
    const char *filename = h->filename;
    av_strstart(filename, "file:", &filename);

    ret = rmdir(filename);
    if (ret < 0 && (errno == ENOTDIR
#   ifdef _WIN32
        || errno == EINVAL
#   endif
        ))
        ret = unlink(filename);
    if (ret < 0)
        return AVERROR(errno);

    return ret;
#else
    return AVERROR(ENOSYS);
#endif 
}

static int file_move(URLContext *h_src, URLContext *h_dst)
{
    const char *filename_src = h_src->filename;
    const char *filename_dst = h_dst->filename;
    av_strstart(filename_src, "file:", &filename_src);
    av_strstart(filename_dst, "file:", &filename_dst);

    if (rename(filename_src, filename_dst) < 0)
        return AVERROR(errno);

    return 0;
}

int avpriv_open(const char *filename, int flags, ...)
{
    int fd;
    unsigned int mode = 0;
    va_list ap;

    va_start(ap, flags);
    if (flags & O_CREAT)
        mode = va_arg(ap, unsigned int);
    va_end(ap);

#ifdef O_CLOEXEC
    flags |= O_CLOEXEC;
#endif
#ifdef O_NOINHERIT
    flags |= O_NOINHERIT;
#endif

    fd = open(filename, flags, mode);
#if HAVE_FCNTL
    if (fd != -1) {
        if (fcntl(fd, F_SETFD, FD_CLOEXEC) == -1)
            av_log(NULL, AV_LOG_DEBUG, "Failed to set close on exec\n");
    }
#endif

    return fd;
}

static int file_open(URLContext *h, const char *filename, int flags)
{
    FileContext *c = h->priv_data;
    int access;
    int fd;
    struct stat st;

    av_strstart(filename, "file:", &filename);

    if (flags & AVIO_FLAG_WRITE && flags & AVIO_FLAG_READ) {
        access = O_CREAT | O_RDWR;
        if (c->trunc)
            access |= O_TRUNC;
    } else if (flags & AVIO_FLAG_WRITE) {
        access = O_CREAT | O_WRONLY;
        if (c->trunc)
            access |= O_TRUNC;
    } else {
        access = O_RDONLY;
    }
#ifdef O_BINARY
    access |= O_BINARY;
#endif
    fd = avpriv_open(filename, access, 0666);
    if (fd == -1)
        return AVERROR(errno);
    c->fd = fd;

    h->is_streamed = !fstat(fd, &st) && S_ISFIFO(st.st_mode);

    
    if (!h->is_streamed && flags & AVIO_FLAG_WRITE)
        h->min_packet_size = h->max_packet_size = 262144;

    if (c->seekable >= 0)
        h->is_streamed = !c->seekable;

    return 0;
}


static int64_t file_seek(URLContext *h, int64_t pos, int whence)
{
    FileContext *c = h->priv_data;
    int64_t ret;

    if (whence == AVSEEK_SIZE) {
        struct stat st;
        ret = fstat(c->fd, &st);
        return ret < 0 ? AVERROR(errno) : (S_ISFIFO(st.st_mode) ? 0 : st.st_size);
    }

    ret = lseek(c->fd, pos, whence);

    return ret < 0 ? AVERROR(errno) : ret;
}

static int file_close(URLContext *h)
{
    FileContext *c = h->priv_data;
    return close(c->fd);
}

static int file_open_dir(URLContext *h)
{
#if HAVE_LSTAT
    FileContext *c = h->priv_data;

    c->dir = opendir(h->filename);
    if (!c->dir)
        return AVERROR(errno);

    return 0;
#else
    return AVERROR(ENOSYS);
#endif 
}

static int file_read_dir(URLContext *h, AVIODirEntry **next)
{
#if HAVE_LSTAT
    FileContext *c = h->priv_data;
    struct dirent *dir;
    char *fullpath = NULL;

    *next = ff_alloc_dir_entry();
    if (!*next)
        return AVERROR(ENOMEM);
    do {
        errno = 0;
        dir = readdir(c->dir);
        if (!dir) {
            av_freep(next);
            return AVERROR(errno);
        }
    } while (!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, ".."));

    fullpath = av_append_path_component(h->filename, dir->d_name);
    if (fullpath) {
        struct stat st;
        if (!lstat(fullpath, &st)) {
            if (S_ISDIR(st.st_mode))
                (*next)->type = AVIO_ENTRY_DIRECTORY;
            else if (S_ISFIFO(st.st_mode))
                (*next)->type = AVIO_ENTRY_NAMED_PIPE;
            else if (S_ISCHR(st.st_mode))
                (*next)->type = AVIO_ENTRY_CHARACTER_DEVICE;
            else if (S_ISBLK(st.st_mode))
                (*next)->type = AVIO_ENTRY_BLOCK_DEVICE;
            else if (S_ISLNK(st.st_mode))
                (*next)->type = AVIO_ENTRY_SYMBOLIC_LINK;
            else if (S_ISSOCK(st.st_mode))
                (*next)->type = AVIO_ENTRY_SOCKET;
            else if (S_ISREG(st.st_mode))
                (*next)->type = AVIO_ENTRY_FILE;
            else
                (*next)->type = AVIO_ENTRY_UNKNOWN;

            (*next)->group_id = st.st_gid;
            (*next)->user_id = st.st_uid;
            (*next)->size = st.st_size;
            (*next)->filemode = st.st_mode & 0777;
            (*next)->modification_timestamp = INT64_C(1000000) * st.st_mtime;
            (*next)->access_timestamp =  INT64_C(1000000) * st.st_atime;
            (*next)->status_change_timestamp = INT64_C(1000000) * st.st_ctime;
        }
        av_free(fullpath);
    }

    (*next)->name = av_strdup(dir->d_name);
    return 0;
#else
    return AVERROR(ENOSYS);
#endif 
}

static int file_close_dir(URLContext *h)
{
#if HAVE_LSTAT
    FileContext *c = h->priv_data;
    closedir(c->dir);
    return 0;
#else
    return AVERROR(ENOSYS);
#endif 
}
const URLProtocol ff_file_protocol = {
    .name                = "file",
    .url_open            = file_open,
    .url_read            = file_read,
    .url_write           = file_write,
    .url_seek            = file_seek,
    .url_close           = file_close,
    .url_get_file_handle = file_get_handle,
    .url_check           = file_check,
    .url_delete          = file_delete,
    .url_move            = file_move,
    .priv_data_size      = sizeof(FileContext),
    .priv_data_class     = &file_class,
    .url_open_dir        = file_open_dir,
    .url_read_dir        = file_read_dir,
    .url_close_dir       = file_close_dir,
    .default_whitelist   = "file,crypto,data"
};

#define URL_PROTOCOL_FLAG_NESTED_SCHEME 1 
#define URL_PROTOCOL_FLAG_NETWORK 2       

static int match_host_pattern(const char *pattern, const char *hostname)
{
    int len_p, len_h;
    if (!strcmp(pattern, "*"))
        return 1;
    // Skip a possible *. at the start of the pattern
    if (pattern[0] == '*')
        pattern++;
    if (pattern[0] == '.')
        pattern++;
    len_p = strlen(pattern);
    len_h = strlen(hostname);
    if (len_p > len_h)
        return 0;
    // Simply check if the end of hostname is equal to 'pattern'
    if (!strcmp(pattern, &hostname[len_h - len_p])) {
        if (len_h == len_p)
            return 1; // Exact match
        if (hostname[len_h - len_p - 1] == '.')
            return 1; // The matched substring is a domain and not just a substring of a domain
    }
    return 0;
}

int ff_http_match_no_proxy(const char *no_proxy, const char *hostname)
{
    char *buf, *start;
    int ret = 0;
    if (!no_proxy)
        return 0;
    if (!hostname)
        return 0;
    buf = av_strdup(no_proxy);
    if (!buf)
        return 0;
    start = buf;
    while (start) {
        char *sep, *next = NULL;
        start += strspn(start, " ,");
        sep = start + strcspn(start, " ,");
        if (*sep) {
            next = sep + 1;
            *sep = '\0';
        }
        if (match_host_pattern(start, hostname)) {
            ret = 1;
            break;
        }
        start = next;
    }
    av_free(buf);
    return ret;
}

static const URLProtocol * const url_protocols[];

#if FF_API_CHILD_CLASS_NEXT
const AVClass *ff_urlcontext_child_class_next(const AVClass *prev)
{
    int i;

    
    for (i = 0; prev && url_protocols[i]; i++) {
        if (url_protocols[i]->priv_data_class == prev) {
            i++;
            break;
        }
    }

    
    for (; url_protocols[i]; i++)
        if (url_protocols[i]->priv_data_class)
            return url_protocols[i]->priv_data_class;
    return NULL;
}

const AVClass *ff_urlcontext_child_class_iterate(void **iter)
{
    const AVClass *ret = NULL;
    uintptr_t i;

    for (i = (uintptr_t)*iter; url_protocols[i]; i++) {
        ret = url_protocols[i]->priv_data_class;
        if (ret)
            break;
    }

    *iter = (void*)(uintptr_t)(url_protocols[i] ? i + 1 : i);
    return ret;
}
const AVClass ffurl_context_class = {
    .class_name       = "URLContext",
    .item_name        = urlcontext_to_name,
    .option           = (AVOption[]) {
    {"protocol_whitelist", "List of protocols that are allowed to be used", offsetof(URLContext,protocol_whitelist), AV_OPT_TYPE_STRING, { .str = NULL },  0, 0, 2 },
    {"protocol_blacklist", "List of protocols that are not allowed to be used", offsetof(URLContext,protocol_blacklist), AV_OPT_TYPE_STRING, { .str = NULL },  0, 0, 2 },
    {"rw_timeout", "Timeout for IO operations (in microseconds)", offsetof(URLContext, rw_timeout), AV_OPT_TYPE_INT64, { .i64 = 0 }, 0, INT64_MAX, AV_OPT_FLAG_ENCODING_PARAM | AV_OPT_FLAG_DECODING_PARAM },
    { NULL }
},
    .version          = LIBAVUTIL_VERSION_INT,
    .child_next       = urlcontext_child_next,
#if FF_API_CHILD_CLASS_NEXT
    .child_class_next = ff_urlcontext_child_class_next,
#endif
    .child_class_iterate = ff_urlcontext_child_class_iterate,
};

static const AVClass *ff_avio_child_class_next(const AVClass *prev)
{
    return prev ? NULL : &ffurl_context_class;
}
#endif
int ff_network_init(void)
{
#if HAVE_WINSOCK2_H
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(1, 1), &wsaData))
        return 0;
#endif
    return 1;
}
void ff_network_close(void)
{
#if HAVE_WINSOCK2_H
    WSACleanup();
#endif
}
static int url_alloc_for_protocol(URLContext **puc, const URLProtocol *up,const char *filename, int flags,const AVIOInterruptCB *int_cb)
{
    URLContext *uc;
    int err;

#if CONFIG_NETWORK
    if (up->flags & URL_PROTOCOL_FLAG_NETWORK && !ff_network_init())
        return AVERROR(EIO);
#endif
    if ((flags & AVIO_FLAG_READ) && !up->url_read) {
        av_log(NULL, AV_LOG_ERROR,
               "Impossible to open the '%s' protocol for reading\n", up->name);
        return AVERROR(EIO);
    }
    if ((flags & AVIO_FLAG_WRITE) && !up->url_write) {
        av_log(NULL, AV_LOG_ERROR,
               "Impossible to open the '%s' protocol for writing\n", up->name);
        return AVERROR(EIO);
    }
    uc = av_mallocz(sizeof(URLContext) + strlen(filename) + 1);
    if (!uc) {
        err = AVERROR(ENOMEM);
        goto fail;
    }
    uc->av_class = &ffurl_context_class;
    uc->filename = (char *)&uc[1];
    strcpy(uc->filename, filename);
    uc->prot            = up;
    uc->flags           = flags;
    uc->is_streamed     = 0; 
    uc->max_packet_size = 0; 
    if (up->priv_data_size) {
        uc->priv_data = av_mallocz(up->priv_data_size);
        if (!uc->priv_data) {
            err = AVERROR(ENOMEM);
            goto fail;
        }
        if (up->priv_data_class) {
            int proto_len= strlen(up->name);
            char *start = strchr(uc->filename, ',');
            *(const AVClass **)uc->priv_data = up->priv_data_class;
            av_opt_set_defaults(uc->priv_data);
            if(!strncmp(up->name, uc->filename, proto_len) && uc->filename + proto_len == start){
                int ret= 0;
                char *p= start;
                char sep= *++p;
                char *key, *val;
                p++;

                if (strcmp(up->name, "subfile"))
                    ret = AVERROR(EINVAL);

                while(ret >= 0 && (key= strchr(p, sep)) && p<key && (val = strchr(key+1, sep))){
                    *val= *key= 0;
                    if (strcmp(p, "start") && strcmp(p, "end")) {
                        ret = AVERROR_OPTION_NOT_FOUND;
                    } else
                        ret= av_opt_set(uc->priv_data, p, key+1, 0);
                    if (ret == AVERROR_OPTION_NOT_FOUND)
                        av_log(uc, AV_LOG_ERROR, "Key '%s' not found.\n", p);
                    *val= *key= sep;
                    p= val+1;
                }
                if(ret<0 || p!=key){
                    av_log(uc, AV_LOG_ERROR, "Error parsing options string %s\n", start);
                    av_freep(&uc->priv_data);
                    av_freep(&uc);
                    err = AVERROR(EINVAL);
                    goto fail;
                }
                memmove(start, key+1, strlen(key));
            }
        }
    }
    if (int_cb)
        uc->interrupt_callback = *int_cb;

    *puc = uc;
    return 0;
fail:
    *puc = NULL;
    if (uc)
        av_freep(&uc->priv_data);
    av_freep(&uc);
#if CONFIG_NETWORK
    if (up->flags & URL_PROTOCOL_FLAG_NETWORK)
        ff_network_close();
#endif
    return err;
}
static const struct URLProtocol *url_find_protocol(const char *filename);
int ffurl_alloc(URLContext **puc, const char *filename, int flags,
                const AVIOInterruptCB *int_cb)
{
    const URLProtocol *p = NULL;

    p = url_find_protocol(filename);
    if (p != NULL)
       return url_alloc_for_protocol(puc, p, filename, flags, int_cb);

    *puc = NULL;
    return AVERROR_PROTOCOL_NOT_FOUND;
}

int ffurl_closep(URLContext **hh)
{
    URLContext *h= *hh;
    int ret = 0;
    if (!h)
        return 0;     

    if (h->is_connected && h->prot->url_close)
        ret = h->prot->url_close(h);
#if CONFIG_NETWORK
    if (h->prot->flags & URL_PROTOCOL_FLAG_NETWORK)
        ff_network_close();
#endif
    if (h->prot->priv_data_size) {
        if (h->prot->priv_data_class)
            av_opt_free(h->priv_data);
        av_freep(&h->priv_data);
    }
    av_opt_free(h);
    av_freep(hh);
    return ret;
}

int ffurl_connect(URLContext *uc, AVDictionary **options)
{
    int err;
    AVDictionary *tmp_opts = NULL;
    AVDictionaryEntry *e;

    if (!options)
        options = &tmp_opts;

    av_assert0(!(e=av_dict_get(*options, "protocol_whitelist", NULL, 0)) ||
               (uc->protocol_whitelist && !strcmp(uc->protocol_whitelist, e->value)));
    av_assert0(!(e=av_dict_get(*options, "protocol_blacklist", NULL, 0)) ||
               (uc->protocol_blacklist && !strcmp(uc->protocol_blacklist, e->value)));

    if (uc->protocol_whitelist && av_match_list(uc->prot->name, uc->protocol_whitelist, ',') <= 0) {
        av_log(uc, AV_LOG_ERROR, "Protocol '%s' not on whitelist '%s'!\n", uc->prot->name, uc->protocol_whitelist);
        return AVERROR(EINVAL);
    }

    if (uc->protocol_blacklist && av_match_list(uc->prot->name, uc->protocol_blacklist, ',') > 0) {
        av_log(uc, AV_LOG_ERROR, "Protocol '%s' on blacklist '%s'!\n", uc->prot->name, uc->protocol_blacklist);
        return AVERROR(EINVAL);
    }

    if (!uc->protocol_whitelist && uc->prot->default_whitelist) {
        av_log(uc, AV_LOG_DEBUG, "Setting default whitelist '%s'\n", uc->prot->default_whitelist);
        uc->protocol_whitelist = av_strdup(uc->prot->default_whitelist);
        if (!uc->protocol_whitelist) {
            return AVERROR(ENOMEM);
        }
    } else if (!uc->protocol_whitelist)
        av_log(uc, AV_LOG_DEBUG, "No default whitelist set\n"); 

    if ((err = av_dict_set(options, "protocol_whitelist", uc->protocol_whitelist, 0)) < 0)
        return err;
    if ((err = av_dict_set(options, "protocol_blacklist", uc->protocol_blacklist, 0)) < 0)
        return err;

    err =
        uc->prot->url_open2 ? uc->prot->url_open2(uc,
                                                  uc->filename,
                                                  uc->flags,
                                                  options) :
        uc->prot->url_open(uc, uc->filename, uc->flags);

    av_dict_set(options, "protocol_whitelist", NULL, 0);
    av_dict_set(options, "protocol_blacklist", NULL, 0);

    if (err)
        return err;
    uc->is_connected = 1;
    
    if ((uc->flags & AVIO_FLAG_WRITE) || !strcmp(uc->prot->name, "file"))
        if (!uc->is_streamed && ffurl_seek(uc, 0, SEEK_SET) < 0)
            uc->is_streamed = 1;
    return 0;
}

int ffurl_open_whitelist(URLContext **puc, const char *filename, int flags,const AVIOInterruptCB *int_cb, AVDictionary **options,const char *whitelist, const char* blacklist,URLContext *parent)
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

int ff_url_join(char *str, int size, const char *proto,
                const char *authorization, const char *hostname,
                int port, const char *fmt, ...)
{
#if CONFIG_NETWORK
    struct addrinfo hints = { 0 }, *ai;
#endif

    str[0] = '\0';
    if (proto)
        av_strlcatf(str, size, "%s://", proto);
    if (authorization && authorization[0])
        av_strlcatf(str, size, "%s@", authorization);
#if CONFIG_NETWORK && defined(AF_INET6)
    /* Determine if hostname is a numerical IPv6 address,
     * properly escape it within [] in that case. */
    hints.ai_flags = AI_NUMERICHOST;
    if (!getaddrinfo(hostname, NULL, &hints, &ai)) {
        if (ai->ai_family == AF_INET6) {
            av_strlcat(str, "[", size);
            av_strlcat(str, hostname, size);
            av_strlcat(str, "]", size);
        } else {
            av_strlcat(str, hostname, size);
        }
        freeaddrinfo(ai);
    } else
#endif
        /* Not an IPv6 address, just output the plain string. */
        av_strlcat(str, hostname, size);

    if (port >= 0)
        av_strlcatf(str, size, ":%d", port);
    if (fmt) {
        va_list vl;
        size_t len = strlen(str);

        va_start(vl, fmt);
        vsnprintf(str + len, size > len ? size - len : 0, fmt, vl);
        va_end(vl);
    }
    return strlen(str);
}

static const char *find_delim(const char *delim, const char *cur, const char *end)
{
    while (cur < end && !strchr(delim, *cur))
        cur++;
    return cur;
}

int ff_url_decompose(URLComponents *uc, const char *url, const char *end)
{
    const char *cur, *aend, *p;

    av_assert0(url);
    if (!end)
        end = url + strlen(url);
    cur = uc->url = url;

    /* scheme */
    uc->scheme = cur;
    p = find_delim(":/", cur, end); /* lavf "schemes" can contain options */
    if (*p == ':')
        cur = p + 1;

    /* authority */
    uc->authority = cur;
    if (end - cur >= 2 && cur[0] == '/' && cur[1] == '/') {
        cur += 2;
        aend = find_delim("/?#", cur, end);

        /* userinfo */
        uc->userinfo = cur;
        p = find_delim("@", cur, aend);
        if (*p == '@')
            cur = p + 1;

        /* host */
        uc->host = cur;
        if (*cur == '[') { /* hello IPv6, thanks for using colons! */
            p = find_delim("]", cur, aend);
            if (*p != ']')
                return AVERROR(EINVAL);
            if (p + 1 < aend && p[1] != ':')
                return AVERROR(EINVAL);
            cur = p + 1;
        } else {
            cur = find_delim(":", cur, aend);
        }

        /* port */
        uc->port = cur;
        cur = aend;
    } else {
        uc->userinfo = uc->host = uc->port = cur;
    }

    /* path */
    uc->path = cur;
    cur = find_delim("?#", cur, end);

    /* query */
    uc->query = cur;
    if (*cur == '?')
        cur = find_delim("#", cur, end);

    /* fragment */
    uc->fragment = cur;

    uc->end = end;
    return 0;
}

static int append_path(char *root, char *out_end, char **rout,
                       const char *in, const char *in_end)
{
    char *out = *rout;
    const char *d, *next;

    if (in < in_end && *in == '/')
        in++; /* already taken care of */
    while (in < in_end) {
        d = find_delim("/", in, in_end);
        next = d + (d < in_end && *d == '/');
        if (d - in == 1 && in[0] == '.') {
            /* skip */
        } else if (d - in == 2 && in[0] == '.' && in[1] == '.') {
            av_assert1(out[-1] == '/');
            if (out - root > 1)
                while (out > root && (--out)[-1] != '/');
        } else {
            if (out_end - out < next - in)
                return AVERROR(ENOMEM);
            memmove(out, in, next - in);
            out += next - in;
        }
        in = next;
    }
    *rout = out;
    return 0;
}

int ff_make_absolute_url(char *buf, int size, const char *base,
                          const char *rel)
{
    URLComponents ub, uc;
    char *out, *out_end, *path;
    const char *keep, *base_path_end;
    int use_base_path, simplify_path = 0, ret;

    if (!size)
        return AVERROR(ENOMEM);
    out = buf;
    out_end = buf + size - 1;

    if (!base)
        base = "";
    if ((ret = ff_url_decompose(&ub, base, NULL) < 0) ||
        (ret = ff_url_decompose(&uc, rel,  NULL) < 0))
        goto error;

    keep = ub.url;
#define KEEP(component, also) do { \
        if (uc.url_component_end_##component == uc.url && \
            ub.url_component_end_##component > keep) { \
            keep = ub.url_component_end_##component; \
            also \
        } \
    } while (0)
    KEEP(scheme, );
    KEEP(authority_full, simplify_path = 1;);
    KEEP(path,);
    KEEP(query,);
    KEEP(fragment,);
#undef KEEP
#define COPY(start, end) do { \
        size_t len = end - start; \
        if (len > out_end - out) { \
            ret = AVERROR(ENOMEM);  \
            goto error; \
        } \
        memmove(out, start, len); \
        out += len; \
    } while (0)
    COPY(ub.url, keep);
    COPY(uc.url, uc.path);

    use_base_path = URL_COMPONENT_HAVE(ub, path) && keep <= ub.path;
    if (uc.path > uc.url)
        use_base_path = 0;
    if (URL_COMPONENT_HAVE(uc, path) && uc.path[0] == '/')
        use_base_path = 0;
    if (use_base_path) {
        base_path_end = ub.url_component_end_path;
        if (URL_COMPONENT_HAVE(uc, path))
            while (base_path_end > ub.path && base_path_end[-1] != '/')
                base_path_end--;
    }
    if (keep > ub.path)
        simplify_path = 0;
    if (URL_COMPONENT_HAVE(uc, scheme))
        simplify_path = 0;
    if (URL_COMPONENT_HAVE(uc, authority))
        simplify_path = 1;
    /* No path at all, leave it */
    if (!use_base_path && !URL_COMPONENT_HAVE(uc, path))
        simplify_path = 0;

    if (simplify_path) {
        const char *root = "/";
        COPY(root, root + 1);
        path = out;
        if (use_base_path) {
            ret = append_path(path, out_end, &out, ub.path, base_path_end);
            if (ret < 0)
                goto error;
        }
        if (URL_COMPONENT_HAVE(uc, path)) {
            ret = append_path(path, out_end, &out, uc.path, uc.url_component_end_path);
            if (ret < 0)
                goto error;
        }
    } else {
        if (use_base_path)
            COPY(ub.path, base_path_end);
        COPY(uc.path, uc.url_component_end_path);
    }

    COPY(uc.url_component_end_path, uc.end);
#undef COPY
    *out = 0;
    return 0;

error:
    snprintf(buf, size, "invalid:%s",
             ret == AVERROR(ENOMEM) ? "truncated" :
             ret == AVERROR(EINVAL) ? "syntax_error" : "");
    return ret;
}

AVIODirEntry *ff_alloc_dir_entry(void)
{
    AVIODirEntry *entry = av_mallocz(sizeof(AVIODirEntry));
    if (entry) {
        entry->type = AVIO_ENTRY_UNKNOWN;
        entry->size = -1;
        entry->modification_timestamp = -1;
        entry->access_timestamp = -1;
        entry->status_change_timestamp = -1;
        entry->user_id = -1;
        entry->group_id = -1;
        entry->filemode = -1;
    }
    return entry;
}

#include "sha.c"

static int read_random(uint32_t *dst, const char *file)
{
#if HAVE_UNISTD_H
    int fd = avpriv_open(file, O_RDONLY);
    int err = -1;

    if (fd == -1)
        return -1;
    err = read(fd, dst, sizeof(*dst));
    close(fd);

    return err;
#else
    return -1;
#endif
}
#ifndef TEST
#define TEST 0
#endif
static uint32_t get_generic_seed(void)
{
    uint64_t tmp[120/8];
    struct AVSHA *sha = (void*)tmp;
    clock_t last_t  = 0;
    clock_t last_td = 0;
    clock_t init_t = 0;
    static uint64_t i = 0;
    static uint32_t buffer[512] = { 0 };
    unsigned char digest[20];
    uint64_t last_i = i;

    av_assert0(sizeof(tmp) >= av_sha_size);

    if(TEST){
        memset(buffer, 0, sizeof(buffer));
        last_i = i = 0;
    }else{
#ifdef AV_READ_TIME
        buffer[13] ^= AV_READ_TIME();
        buffer[41] ^= AV_READ_TIME()>>32;
#endif
    }

    for (;;) {
        clock_t t = clock();
        if (last_t + 2*last_td + (CLOCKS_PER_SEC > 1000) >= t) {
            last_td = t - last_t;
            buffer[i & 511] = 1664525*buffer[i & 511] + 1013904223 + (last_td % 3294638521U);
        } else {
            last_td = t - last_t;
            buffer[++i & 511] += last_td % 3294638521U;
            if ((t - init_t) >= CLOCKS_PER_SEC>>5)
                if (last_i && i - last_i > 4 || i - last_i > 64 || TEST && i - last_i > 8)
                    break;
        }
        last_t = t;
        if (!init_t)
            init_t = t;
    }

    if(TEST) {
        buffer[0] = buffer[1] = 0;
    } else {
#ifdef AV_READ_TIME
        buffer[111] += AV_READ_TIME();
#endif
    }

    av_sha_init(sha, 160);
    av_sha_update(sha, (const uint8_t *)buffer, sizeof(buffer));
    av_sha_final(sha, digest);
    return AV_RB32(digest) + AV_RB32(digest + 16);
}
uint32_t av_get_random_seed(void)
{
    uint32_t seed;

#if HAVE_BCRYPT
    BCRYPT_ALG_HANDLE algo_handle;
    NTSTATUS ret = BCryptOpenAlgorithmProvider(&algo_handle, BCRYPT_RNG_ALGORITHM,
                                               MS_PRIMITIVE_PROVIDER, 0);
    if (BCRYPT_SUCCESS(ret)) {
        NTSTATUS ret = BCryptGenRandom(algo_handle, (UCHAR*)&seed, sizeof(seed), 0);
        BCryptCloseAlgorithmProvider(algo_handle, 0);
        if (BCRYPT_SUCCESS(ret))
            return seed;
    }
#endif

#if HAVE_ARC4RANDOM
    return arc4random();
#endif

    if (read_random(&seed, "/dev/urandom") == sizeof(seed))
        return seed;
    if (read_random(&seed, "/dev/random")  == sizeof(seed))
        return seed;
    return get_generic_seed();
}

int ffurl_close(URLContext *h)
{
    return ffurl_closep(&h);
}

int ffurl_get_file_handle(URLContext *h)
{
    if (!h || !h->prot || !h->prot->url_get_file_handle)
        return -1;
    return h->prot->url_get_file_handle(h);
}
static int inet_aton(const char *str, struct in_addr *add)
{
    unsigned int add1 = 0, add2 = 0, add3 = 0, add4 = 0;

    if (sscanf(str, "%d.%d.%d.%d", &add1, &add2, &add3, &add4) != 4)
        return 0;

    if (!add1 || (add1 | add2 | add3 | add4) > 255)
        return 0;

    add->s_addr = htonl((add1 << 24) + (add2 << 16) + (add3 << 8) + add4);

    return 1;
}

int ff_getaddrinfo(const char *node, const char *service,
                   const struct addrinfo *hints, struct addrinfo **res)
{
    struct hostent *h = NULL;
    struct addrinfo *ai;
    struct sockaddr_in *sin;

    *res = NULL;
    sin  = av_mallocz(sizeof(struct sockaddr_in));
    if (!sin)
        return EAI_FAIL;
    sin->sin_family = AF_INET;

    if (node) {
        if (!inet_aton(node, &sin->sin_addr)) {
            if (hints && (hints->ai_flags & AI_NUMERICHOST)) {
                av_free(sin);
                return EAI_FAIL;
            }
            h = gethostbyname(node);
            if (!h) {
                av_free(sin);
                return EAI_FAIL;
            }
            memcpy(&sin->sin_addr, h->h_addr_list[0], sizeof(struct in_addr));
        }
    } else {
        if (hints && (hints->ai_flags & AI_PASSIVE))
            sin->sin_addr.s_addr = INADDR_ANY;
        else
            sin->sin_addr.s_addr = INADDR_LOOPBACK;
    }

    /* Note: getaddrinfo allows service to be a string, which
     * should be looked up using getservbyname. */
    if (service)
        sin->sin_port = htons(atoi(service));

    ai = av_mallocz(sizeof(struct addrinfo));
    if (!ai) {
        av_free(sin);
        return EAI_FAIL;
    }

    *res            = ai;
    ai->ai_family   = AF_INET;
    ai->ai_socktype = hints ? hints->ai_socktype : 0;
    switch (ai->ai_socktype) {
    case SOCK_STREAM:
        ai->ai_protocol = IPPROTO_TCP;
        break;
    case SOCK_DGRAM:
        ai->ai_protocol = IPPROTO_UDP;
        break;
    default:
        ai->ai_protocol = 0;
        break;
    }

    ai->ai_addr    = (struct sockaddr *)sin;
    ai->ai_addrlen = sizeof(struct sockaddr_in);
    if (hints && (hints->ai_flags & AI_CANONNAME))
        ai->ai_canonname = h ? av_strdup(h->h_name) : NULL;

    ai->ai_next = NULL;
    return 0;
}
void ff_freeaddrinfo(struct addrinfo *res)
{
    av_freep(&res->ai_canonname);
    av_freep(&res->ai_addr);
    av_freep(&res);
}

#include "http.c"
static const URLProtocol * const url_protocols[] = {
    // &ff_async_protocol,
    // &ff_bluray_protocol,
    // &ff_cache_protocol,
    // &ff_concat_protocol,
    // &ff_crypto_protocol,
    // &ff_data_protocol,
    // &ff_ffrtmphttp_protocol,
    &ff_file_protocol,
    // &ff_ftp_protocol,
    // &ff_gopher_protocol,
    // &ff_hls_protocol,
    &ff_http_protocol,
    // &ff_httpproxy_protocol,
    // &ff_https_protocol,
    // &ff_icecast_protocol,
    // &ff_mmsh_protocol,
    // &ff_mmst_protocol,
    // &ff_md5_protocol,
    // &ff_pipe_protocol,
    // &ff_prompeg_protocol,
    // &ff_rtmp_protocol,
    // &ff_rtmps_protocol,
    // &ff_rtmpt_protocol,
    // &ff_rtmpts_protocol,
    // &ff_rtp_protocol,
    // &ff_srtp_protocol,
    // &ff_subfile_protocol,
    // &ff_tee_protocol,
    // &ff_tcp_protocol,
    // &ff_tls_protocol,
    // &ff_udp_protocol,
    // &ff_udplite_protocol,
    // &ff_libsrt_protocol,
    // &ff_libssh_protocol,
    // &ff_libzmq_protocol,
    NULL };



static const AVClass *child_class_iterate(void **iter)
{
    const AVClass *c = *iter ? NULL : &ffurl_context_class;
    *iter = (void*)(uintptr_t)c;
    return c;
}
static const AVOption ff_avio_options[] = {
    {"protocol_whitelist", "List of protocols that are allowed to be used", offsetof(AVIOContext,protocol_whitelist), AV_OPT_TYPE_STRING, { .str = NULL },  0, 0, 2 },
    { NULL },
};
const AVClass ff_avio_class = {
    .class_name = "AVIOContext",
    .item_name  = av_default_item_name,
    .version    = LIBAVUTIL_VERSION_INT,
    .option     = ff_avio_options,
    .child_next = ff_avio_child_next,
#if FF_API_CHILD_CLASS_NEXT
    .child_class_next = ff_avio_child_class_next,
#endif
    .child_class_iterate = child_class_iterate,
};
int ffio_fdopen(AVIOContext **s, URLContext *h)
{
    uint8_t *buffer = NULL;
    int buffer_size, max_packet_size;

    max_packet_size = h->max_packet_size;
    if (max_packet_size) {
        buffer_size = max_packet_size; 
    } else {
        buffer_size = IO_BUFFER_SIZE;
    }
    if (!(h->flags & AVIO_FLAG_WRITE) && h->is_streamed) {
        if (buffer_size > INT_MAX/2)
            return AVERROR(EINVAL);
        buffer_size *= 2;
    }
    buffer = av_malloc(buffer_size);
    if (!buffer)
        return AVERROR(ENOMEM);

    *s = avio_alloc_context(buffer, buffer_size, h->flags & AVIO_FLAG_WRITE, h,
                            (int (*)(void *, uint8_t *, int))  ffurl_read,
                            (int (*)(void *, uint8_t *, int))  ffurl_write,
                            (int64_t (*)(void *, int64_t, int))ffurl_seek);
    if (!*s)
        goto fail;

    (*s)->protocol_whitelist = av_strdup(h->protocol_whitelist);
    if (!(*s)->protocol_whitelist && h->protocol_whitelist) {
        avio_closep(s);
        goto fail;
    }
    (*s)->protocol_blacklist = av_strdup(h->protocol_blacklist);
    if (!(*s)->protocol_blacklist && h->protocol_blacklist) {
        avio_closep(s);
        goto fail;
    }
    (*s)->direct = h->flags & AVIO_FLAG_DIRECT;

    (*s)->seekable = h->is_streamed ? 0 : AVIO_SEEKABLE_NORMAL;
    (*s)->max_packet_size = max_packet_size;
    (*s)->min_packet_size = h->min_packet_size;
    if(h->prot) {
        (*s)->read_pause = (int (*)(void *, int))h->prot->url_read_pause;
        (*s)->read_seek  =
            (int64_t (*)(void *, int, int64_t, int))h->prot->url_read_seek;

        if (h->prot->url_read_seek)
            (*s)->seekable |= AVIO_SEEKABLE_TIME;
    }
    (*s)->short_seek_get = (int (*)(void *))ffurl_get_short_seek;
    (*s)->av_class = &ff_avio_class;
    return 0;
fail:
    av_freep(&buffer);
    return AVERROR(ENOMEM);
}
#define URL_SCHEME_CHARS         \
    "abcdefghijklmnopqrstuvwxyz" \
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ" \
    "0123456789+-."
    static inline int is_dos_path(const char *path)
{
#if HAVE_DOS_PATHS
    if (path[0] && path[1] == ':')
        return 1;
#endif
    return 0;
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

static const struct URLProtocol *url_find_protocol(const char *filename)
{
    const URLProtocol **protocols;
    char proto_str[128], proto_nested[128], *ptr;
    size_t proto_len = strspn(filename, URL_SCHEME_CHARS);
    int i;

    if (filename[proto_len] != ':' &&
        (strncmp(filename, "subfile,", 8) || !strchr(filename + proto_len + 1, ':')) || is_dos_path(filename))
        strcpy(proto_str, "file");
    else
        av_strlcpy(proto_str, filename, FFMIN(proto_len + 1, sizeof(proto_str)));

    av_strlcpy(proto_nested, proto_str, sizeof(proto_nested));
    ptr = strchr(proto_nested, '+');
    if (ptr != NULL)
        *ptr = '\0';

    protocols = ffurl_get_protocols(NULL, NULL);
    if (protocols == NULL)
        return NULL;
    av_log(NULL, AV_LOG_DEBUG, "%s\n", proto_str);

    for (i = 0; protocols[i]; i++) {
            const URLProtocol *up = protocols[i];

        if (!strcmp(proto_str, up->name)) {
            av_freep(&protocols);
            return up;
        }
        if (up->flags & URL_PROTOCOL_FLAG_NESTED_SCHEME && !strcmp(proto_nested, up->name)) {
            av_freep(&protocols);
            return up;
        }
    }
    av_freep(&protocols);
    if (av_strstart(filename, "https:", NULL) || av_strstart(filename, "tls:", NULL))
        av_log(NULL, AV_LOG_WARNING, "https protocol not found, recompile FFmpeg with openssl, gnutls or securetransport enabled.\n");

    return NULL;
}







int ffio_open_whitelist(AVIOContext **s, const char *filename, int flags, const AVIOInterruptCB *int_cb, AVDictionary **options,const char *whitelist, const char *blacklist)
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
    if (s->open_cb)
        return s->open_cb(s, pb, url, flags, &s->interrupt_callback, options);
#endif

    return ffio_open_whitelist(pb, url, flags, &s->interrupt_callback, options, s->protocol_whitelist, s->protocol_blacklist);
}

static void io_close_default(AVFormatContext *s, AVIOContext *pb)
{
    avio_close(pb);
}
static const AVOption avformat_options[] = {
{"avioflags", NULL, offsetof(AVFormatContext,avio_flags), AV_OPT_TYPE_FLAGS, {.i64 = 0 }, INT_MIN, INT_MAX, 3, "avioflags"},
{"direct", "reduce buffering", 0, AV_OPT_TYPE_CONST, {.i64 = AVIO_FLAG_DIRECT }, INT_MIN, INT_MAX, 3, "avioflags"},
{"probesize", "set probing size", offsetof(AVFormatContext,probesize), AV_OPT_TYPE_INT64, {.i64 = 5000000 }, 32, INT64_MAX, 2},
{"formatprobesize", "number of bytes to probe file format", offsetof(AVFormatContext,format_probesize), AV_OPT_TYPE_INT, {.i64 = PROBE_BUF_MAX}, 0, INT_MAX-1, 2},
{"packetsize", "set packet size", offsetof(AVFormatContext,packet_size), AV_OPT_TYPE_INT, {.i64 = 0 }, 0, INT_MAX, 1},
{"fflags", NULL, offsetof(AVFormatContext,flags), AV_OPT_TYPE_FLAGS, {.i64 = AVFMT_FLAG_AUTO_BSF }, INT_MIN, INT_MAX, 3, "fflags"},
{"flush_packets", "reduce the latency by flushing out packets immediately", 0, AV_OPT_TYPE_CONST, {.i64 = AVFMT_FLAG_FLUSH_PACKETS }, INT_MIN, INT_MAX, 1, "fflags"},
{"ignidx", "ignore index", 0, AV_OPT_TYPE_CONST, {.i64 = AVFMT_FLAG_IGNIDX }, INT_MIN, INT_MAX, 2, "fflags"},
{"genpts", "generate pts", 0, AV_OPT_TYPE_CONST, {.i64 = AVFMT_FLAG_GENPTS }, INT_MIN, INT_MAX, 2, "fflags"},
{"nofillin", "do not fill in missing values that can be exactly calculated", 0, AV_OPT_TYPE_CONST, {.i64 = AVFMT_FLAG_NOFILLIN }, INT_MIN, INT_MAX, 2, "fflags"},
{"noparse", "disable AVParsers, this needs nofillin too", 0, AV_OPT_TYPE_CONST, {.i64 = AVFMT_FLAG_NOPARSE }, INT_MIN, INT_MAX, 2, "fflags"},
{"igndts", "ignore dts", 0, AV_OPT_TYPE_CONST, {.i64 = AVFMT_FLAG_IGNDTS }, INT_MIN, INT_MAX, 2, "fflags"},
{"discardcorrupt", "discard corrupted frames", 0, AV_OPT_TYPE_CONST, {.i64 = AVFMT_FLAG_DISCARD_CORRUPT }, INT_MIN, INT_MAX, 2, "fflags"},
{"sortdts", "try to interleave outputted packets by dts", 0, AV_OPT_TYPE_CONST, {.i64 = AVFMT_FLAG_SORT_DTS }, INT_MIN, INT_MAX, 2, "fflags"},
#if FF_API_LAVF_KEEPSIDE_FLAG
{"keepside", "deprecated, does nothing", 0, AV_OPT_TYPE_CONST, {.i64 = AVFMT_FLAG_KEEP_SIDE_DATA }, INT_MIN, INT_MAX, 2, "fflags"},
#endif
{"fastseek", "fast but inaccurate seeks", 0, AV_OPT_TYPE_CONST, {.i64 = AVFMT_FLAG_FAST_SEEK }, INT_MIN, INT_MAX, 2, "fflags"},
#if FF_API_LAVF_MP4A_LATM
{"latm", "deprecated, does nothing", 0, AV_OPT_TYPE_CONST, {.i64 = AVFMT_FLAG_MP4A_LATM }, INT_MIN, INT_MAX, 1, "fflags"},
#endif
{"nobuffer", "reduce the latency introduced by optional buffering", 0, AV_OPT_TYPE_CONST, {.i64 = AVFMT_FLAG_NOBUFFER }, 0, INT_MAX, 2, "fflags"},
{"bitexact", "do not write random/volatile data", 0, AV_OPT_TYPE_CONST, { .i64 = AVFMT_FLAG_BITEXACT }, 0, 0, 1, "fflags" },
{"shortest", "stop muxing with the shortest stream", 0, AV_OPT_TYPE_CONST, { .i64 = AVFMT_FLAG_SHORTEST }, 0, 0, 1, "fflags" },
{"autobsf", "add needed bsfs automatically", 0, AV_OPT_TYPE_CONST, { .i64 = AVFMT_FLAG_AUTO_BSF }, 0, 0, 1, "fflags" },
{"seek2any", "allow seeking to non-keyframes on demuxer level when supported", offsetof(AVFormatContext,seek2any), AV_OPT_TYPE_BOOL, {.i64 = 0 }, 0, 1, 2},
{"analyzeduration", "specify how many microseconds are analyzed to probe the input", offsetof(AVFormatContext,max_analyze_duration), AV_OPT_TYPE_INT64, {.i64 = 0 }, 0, INT64_MAX, 2},
{"cryptokey", "decryption key", offsetof(AVFormatContext,key), AV_OPT_TYPE_BINARY, {.dbl = 0}, 0, 0, 2},
{"indexmem", "max memory used for timestamp index (per stream)", offsetof(AVFormatContext,max_index_size), AV_OPT_TYPE_INT, {.i64 = 1<<20 }, 0, INT_MAX, 2},
{"rtbufsize", "max memory used for buffering real-time frames", offsetof(AVFormatContext,max_picture_buffer), AV_OPT_TYPE_INT, {.i64 = 3041280 }, 0, INT_MAX, 2}, 
{"fdebug", "print specific debug info", offsetof(AVFormatContext,debug), AV_OPT_TYPE_FLAGS, {.i64 = 0 }, 0, INT_MAX, 3, "fdebug"},
{"ts", NULL, 0, AV_OPT_TYPE_CONST, {.i64 = FF_FDEBUG_TS }, INT_MIN, INT_MAX, 3, "fdebug"},
{"max_delay", "maximum muxing or demuxing delay in microseconds", offsetof(AVFormatContext,max_delay), AV_OPT_TYPE_INT, {.i64 = -1 }, -1, INT_MAX, 3},
{"start_time_realtime", "wall-clock time when stream begins (PTS==0)", offsetof(AVFormatContext,start_time_realtime), AV_OPT_TYPE_INT64, {.i64 = AV_NOPTS_VALUE}, INT64_MIN, INT64_MAX, 1},
{"fpsprobesize", "number of frames used to probe fps", offsetof(AVFormatContext,fps_probe_size), AV_OPT_TYPE_INT, {.i64 = -1}, -1, INT_MAX-1, 2},
{"audio_preload", "microseconds by which audio packets should be interleaved earlier", offsetof(AVFormatContext,audio_preload), AV_OPT_TYPE_INT, {.i64 = 0}, 0, INT_MAX-1, 1},
{"chunk_duration", "microseconds for each chunk", offsetof(AVFormatContext,max_chunk_duration), AV_OPT_TYPE_INT, {.i64 = 0}, 0, INT_MAX-1, 1},
{"chunk_size", "size in bytes for each chunk", offsetof(AVFormatContext,max_chunk_size), AV_OPT_TYPE_INT, {.i64 = 0}, 0, INT_MAX-1, 1},

{"f_err_detect", "set error detection flags (deprecated; use err_detect, save via avconv)", offsetof(AVFormatContext,error_recognition), AV_OPT_TYPE_FLAGS, {.i64 = AV_EF_CRCCHECK }, INT_MIN, INT_MAX, 2, "err_detect"},
{"err_detect", "set error detection flags", offsetof(AVFormatContext,error_recognition), AV_OPT_TYPE_FLAGS, {.i64 = AV_EF_CRCCHECK }, INT_MIN, INT_MAX, 2, "err_detect"},
{"crccheck", "verify embedded CRCs", 0, AV_OPT_TYPE_CONST, {.i64 = AV_EF_CRCCHECK }, INT_MIN, INT_MAX, 2, "err_detect"},
{"bitstream", "detect bitstream specification deviations", 0, AV_OPT_TYPE_CONST, {.i64 = AV_EF_BITSTREAM }, INT_MIN, INT_MAX, 2, "err_detect"},
{"buffer", "detect improper bitstream length", 0, AV_OPT_TYPE_CONST, {.i64 = AV_EF_BUFFER }, INT_MIN, INT_MAX, 2, "err_detect"},
{"explode", "abort decoding on minor error detection", 0, AV_OPT_TYPE_CONST, {.i64 = AV_EF_EXPLODE }, INT_MIN, INT_MAX, 2, "err_detect"},
{"ignore_err", "ignore errors", 0, AV_OPT_TYPE_CONST, {.i64 = AV_EF_IGNORE_ERR }, INT_MIN, INT_MAX, 2, "err_detect"},
{"careful",    "consider things that violate the spec, are fast to check and have not been seen in the wild as errors", 0, AV_OPT_TYPE_CONST, {.i64 = AV_EF_CAREFUL }, INT_MIN, INT_MAX, 2, "err_detect"},
{"compliant",  "consider all spec non compliancies as errors", 0, AV_OPT_TYPE_CONST, {.i64 = AV_EF_COMPLIANT | AV_EF_CAREFUL }, INT_MIN, INT_MAX, 2, "err_detect"},
{"aggressive", "consider things that a sane encoder shouldn't do as an error", 0, AV_OPT_TYPE_CONST, {.i64 = AV_EF_AGGRESSIVE | AV_EF_COMPLIANT | AV_EF_CAREFUL}, INT_MIN, INT_MAX, 2, "err_detect"},
{"use_wallclock_as_timestamps", "use wallclock as timestamps", offsetof(AVFormatContext,use_wallclock_as_timestamps), AV_OPT_TYPE_BOOL, {.i64 = 0}, 0, 1, 2},
{"skip_initial_bytes", "set number of bytes to skip before reading header and frames", offsetof(AVFormatContext,skip_initial_bytes), AV_OPT_TYPE_INT64, {.i64 = 0}, 0, INT64_MAX-1, 2},
{"correct_ts_overflow", "correct single timestamp overflows", offsetof(AVFormatContext,correct_ts_overflow), AV_OPT_TYPE_BOOL, {.i64 = 1}, 0, 1, 2},
{"flush_packets", "enable flushing of the I/O context after each packet", offsetof(AVFormatContext,flush_packets), AV_OPT_TYPE_INT, {.i64 = -1}, -1, 1, 1},
{"metadata_header_padding", "set number of bytes to be written as padding in a metadata header", offsetof(AVFormatContext,metadata_header_padding), AV_OPT_TYPE_INT, {.i64 = -1}, -1, INT_MAX, 1},
{"output_ts_offset", "set output timestamp offset", offsetof(AVFormatContext,output_ts_offset), AV_OPT_TYPE_DURATION, {.i64 = 0}, -INT64_MAX, INT64_MAX, 1},
{"max_interleave_delta", "maximum buffering duration for interleaving", offsetof(AVFormatContext,max_interleave_delta), AV_OPT_TYPE_INT64, { .i64 = 10000000 }, 0, INT64_MAX, 1 },
{"f_strict", "how strictly to follow the standards (deprecated; use strict, save via avconv)", offsetof(AVFormatContext,strict_std_compliance), AV_OPT_TYPE_INT, {.i64 = 0 }, INT_MIN, INT_MAX, 3, "strict"},
{"strict", "how strictly to follow the standards", offsetof(AVFormatContext,strict_std_compliance), AV_OPT_TYPE_INT, {.i64 = 0 }, INT_MIN, INT_MAX, 3, "strict"},
{"very", "strictly conform to a older more strict version of the spec or reference software", 0, AV_OPT_TYPE_CONST, {.i64 = FF_COMPLIANCE_VERY_STRICT }, INT_MIN, INT_MAX, 3, "strict"},
{"strict", "strictly conform to all the things in the spec no matter what the consequences", 0, AV_OPT_TYPE_CONST, {.i64 = FF_COMPLIANCE_STRICT }, INT_MIN, INT_MAX, 3, "strict"},
{"normal", NULL, 0, AV_OPT_TYPE_CONST, {.i64 = FF_COMPLIANCE_NORMAL }, INT_MIN, INT_MAX, 3, "strict"},
{"unofficial", "allow unofficial extensions", 0, AV_OPT_TYPE_CONST, {.i64 = FF_COMPLIANCE_UNOFFICIAL }, INT_MIN, INT_MAX, 3, "strict"},
{"experimental", "allow non-standardized experimental variants", 0, AV_OPT_TYPE_CONST, {.i64 = FF_COMPLIANCE_EXPERIMENTAL }, INT_MIN, INT_MAX, 3, "strict"},
{"max_ts_probe", "maximum number of packets to read while waiting for the first timestamp", offsetof(AVFormatContext,max_ts_probe), AV_OPT_TYPE_INT, { .i64 = 50 }, 0, INT_MAX, 2 },
{"avoid_negative_ts", "shift timestamps so they start at 0", offsetof(AVFormatContext,avoid_negative_ts), AV_OPT_TYPE_INT, {.i64 = -1}, -1, 2, 1, "avoid_negative_ts"},
{"auto",              "enabled when required by target format",    0, AV_OPT_TYPE_CONST, {.i64 = AVFMT_AVOID_NEG_TS_AUTO },              INT_MIN, INT_MAX, 1, "avoid_negative_ts"},
{"disabled",          "do not change timestamps",                  0, AV_OPT_TYPE_CONST, {.i64 = 0 },                                    INT_MIN, INT_MAX, 1, "avoid_negative_ts"},
{"make_non_negative", "shift timestamps so they are non negative", 0, AV_OPT_TYPE_CONST, {.i64 = AVFMT_AVOID_NEG_TS_MAKE_NON_NEGATIVE }, INT_MIN, INT_MAX, 1, "avoid_negative_ts"},
{"make_zero",         "shift timestamps so they start at 0",       0, AV_OPT_TYPE_CONST, {.i64 = AVFMT_AVOID_NEG_TS_MAKE_ZERO },         INT_MIN, INT_MAX, 1, "avoid_negative_ts"},
{"dump_separator", "set information dump field separator", offsetof(AVFormatContext,dump_separator), AV_OPT_TYPE_STRING, {.str = ", "}, 0, 0, 3},
{"codec_whitelist", "List of decoders that are allowed to be used", offsetof(AVFormatContext,codec_whitelist), AV_OPT_TYPE_STRING, { .str = NULL },  0, 0, 2 },
{"format_whitelist", "List of demuxers that are allowed to be used", offsetof(AVFormatContext,format_whitelist), AV_OPT_TYPE_STRING, { .str = NULL },  0, 0, 2 },
{"protocol_whitelist", "List of protocols that are allowed to be used", offsetof(AVFormatContext,protocol_whitelist), AV_OPT_TYPE_STRING, { .str = NULL },  0, 0, 2 },
{"protocol_blacklist", "List of protocols that are not allowed to be used", offsetof(AVFormatContext,protocol_blacklist), AV_OPT_TYPE_STRING, { .str = NULL },  0, 0, 2 },
{"max_streams", "maximum number of streams", offsetof(AVFormatContext,max_streams), AV_OPT_TYPE_INT, { .i64 = 1000 }, 0, INT_MAX, 2 },
{"skip_estimate_duration_from_pts", "skip duration calculation in estimate_timings_from_pts", offsetof(AVFormatContext,skip_estimate_duration_from_pts), AV_OPT_TYPE_BOOL, {.i64 = 0}, 0, 1, 2},
{"max_probe_packets", "Maximum number of packets to probe a codec", offsetof(AVFormatContext,max_probe_packets), AV_OPT_TYPE_INT, { .i64 = 2500 }, 0, INT_MAX, 2 },
{NULL},
};

#define ITER_STATE_SHIFT 16

static const AVClass *format_child_class_iterate(void **iter)
{

    void *val = (void*)(((uintptr_t)*iter) & ((1 << ITER_STATE_SHIFT) - 1));
    unsigned int state = ((uintptr_t)*iter) >> ITER_STATE_SHIFT;
    const AVClass *ret = NULL;

    if (state == CHILD_CLASS_ITER_AVIO) {
        ret = &ff_avio_class;
        state++;
        goto finish;
    }

    if (state == CHILD_CLASS_ITER_MUX) {
        const AVOutputFormat *ofmt;

        while ((ofmt = av_muxer_iterate(&val))) {
            ret = ofmt->priv_class;
            if (ret)
                goto finish;
        }

        val = NULL;
        state++;
    }

    if (state == CHILD_CLASS_ITER_DEMUX) {
        const AVInputFormat *ifmt;

        while ((ifmt = av_demuxer_iterate(&val))) {
            ret = ifmt->priv_class;
            if (ret)
                goto finish;
        }
        val = NULL;
        state++;
    }
finish:
    av_assert0(!((uintptr_t)val >> ITER_STATE_SHIFT));
    *iter = (void*)((uintptr_t)val | (state << ITER_STATE_SHIFT));
    return ret;
}
static void *format_child_next(void *obj, void *prev)
{
    AVFormatContext *s = obj;
    if (!prev && s->priv_data &&
        ((s->iformat && s->iformat->priv_class) ||
          s->oformat && s->oformat->priv_class))
        return s->priv_data;
    if (s->pb && s->pb->av_class && prev != s->pb)
        return s->pb;
    return NULL;
}
static const char* format_to_name(void* ptr)
{
    AVFormatContext* fc = (AVFormatContext*) ptr;
    if(fc->iformat) return fc->iformat->name;
    else if(fc->oformat) return fc->oformat->name;
    else return "NULL";
}
static AVClassCategory AVClass_get_category(void *ptr)
{
    AVCodecContext* avctx = ptr;
    if(avctx->codec && avctx->codec->decode) return AV_CLASS_CATEGORY_DECODER;
    else                                     return AV_CLASS_CATEGORY_ENCODER;
}
static const AVClass av_format_context_class = {
    .class_name     = "AVFormatContext",
    .item_name      = format_to_name,
    .option         = avformat_options,
    .version        = LIBAVUTIL_VERSION_INT,
    .child_next     = format_child_next,
    .child_class_iterate = format_child_class_iterate,
    .category       = AV_CLASS_CATEGORY_MUXER,
    .get_category   = AVClass_get_category,
};
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
    if (wait_mutex == NULL)
    {
        av_log(NULL, 8, "SDL_CreateMutex(): %s\n", SDL_GetError());
        ret = (-(12));
        goto fail;
    }
    memset(st_index, -1, sizeof(st_index));
    is->eof = 0;
    ic = avformat_alloc_context();
    if (ic == NULL)
    {
        av_log(NULL, 8, "Could not allocate context.\n");
        ret = (-(12));
        goto fail;
    }
    ic->interrupt_callback.callback = decode_interrupt_cb;
    ic->interrupt_callback.opaque = is;
    t = av_dict_get(format_opts, "scan_all_pmts", NULL, 1);
    if (t == NULL)
    {
        av_dict_set(&format_opts, "scan_all_pmts", "1", 16);
        scan_all_pmts_set = 1;
    }
    err = avformat_open_input(&ic, is->filename, is->iformat, &format_opts);
    if (err < 0)
    {
        print_error(is->filename, err);
        ret = -1;
        goto fail;
    }
    if (scan_all_pmts_set != 0)
        av_dict_set(&format_opts, "scan_all_pmts",NULL, 1);
    t = av_dict_get(format_opts, "", NULL, 2);
    if (t != NULL)
    {
        av_log(NULL, 16, "Option %s not found.\n", t->key);
        ret = (-(int)(0xF8 | ('O' << 8) | ('P' << 16) | (unsigned)('T') << 24));
        goto fail;
    }
    is->ic = ic;
    if (genpts != 0)
        ic->flags |= 0x0001;
    av_format_inject_global_side_data(ic);
    if (find_stream_info != 0)
    {
        AVDictionary **opts = setup_find_stream_info_opts(ic, codec_opts);
        int orig_nb_streams = ic->nb_streams;
        err = avformat_find_stream_info(ic, opts);
        for (i = 0; i < orig_nb_streams; i++)
            av_dict_free(&opts[i]);
        av_freep(&opts);
        if (err < 0)
        {
            av_log(NULL, 24,"%s: could not find codec parameters\n", is->filename);
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
    if (start_time != 0)
    {
        int64_t timestamp;
        timestamp = start_time;
        if (ic->start_time != 0)
            timestamp += ic->start_time;
        ret = avformat_seek_file(ic, -1, (-9223372036854775807LL - 1), timestamp, 9223372036854775807LL, 0);
        if (ret < 0)
        {
            av_log(NULL, 24, "%s: could not seek to position %0.3f\n", is->filename, (double)timestamp / 1000000);
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
            av_log(NULL, 16, "Stream specifier %s does not match any %s stream\n", wanted_stream_spec[i], av_get_media_type_string(i));
            st_index[i] = 0x7fffffff;
        }
    }
    if (video_disable == 0)
        st_index[AVMEDIA_TYPE_VIDEO] =
            av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, st_index[AVMEDIA_TYPE_VIDEO], -1, NULL, 0);
    if (audio_disable == 0)
        st_index[AVMEDIA_TYPE_AUDIO] = av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO, st_index[AVMEDIA_TYPE_AUDIO], st_index[AVMEDIA_TYPE_VIDEO], NULL, 0);
    if (video_disable == 0 && subtitle_disable == 0)
        st_index[AVMEDIA_TYPE_SUBTITLE] = av_find_best_stream(ic, AVMEDIA_TYPE_SUBTITLE, st_index[AVMEDIA_TYPE_SUBTITLE], (st_index[AVMEDIA_TYPE_AUDIO] >= 0 ? st_index[AVMEDIA_TYPE_AUDIO] : st_index[AVMEDIA_TYPE_VIDEO]),NULL, 0);
    is->show_mode = show_mode;
    if (st_index[AVMEDIA_TYPE_VIDEO] >= 0)
    {
        AVStream *st = ic->streams[st_index[AVMEDIA_TYPE_VIDEO]];
        AVCodecParameters *codecpar = st->codecpar;
        AVRational sar = av_guess_sample_aspect_ratio(ic, st, NULL);
        if (codecpar->width)
            set_default_window_size(codecpar->width, codecpar->height, sar);
    }
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
        av_log(NULL, 8, "Failed to open file '%s' or configure filtergraph\n", is->filename);
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
        if (is->paused && (!strcmp(ic->iformat->name, "rtsp") || (ic->pb && !strncmp(input_filename, "mmsh:", 5))))
        {
            SDL_Delay(10);
            continue;
        }
        if (is->seek_req)
        {
            int64_t seek_target = is->seek_pos;
            int64_t seek_min = is->seek_rel > 0 ? seek_target - is->seek_rel + 2 : (-9223372036854775807LL - 1);
            int64_t seek_max = is->seek_rel < 0 ? seek_target - is->seek_rel - 2 : 9223372036854775807LL;
            // FIXME the +-2 is due to rounding being not done in the correct direction in generation of the seek_pos/seek_rel variables
            ret = avformat_seek_file(is->ic, -1, seek_min, seek_target, seek_max, is->seek_flags);
            if (ret < 0)
            {
                av_log(NULL, 16,"%s: error while seeking\n", is->ic->url);
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
                if (is->seek_flags & 2)
                {
                    set_clock(&is->extclk, NAN, 0);
                }
                else
                {
                    set_clock(&is->extclk, seek_target / (double)1000000, 0);
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
            if (is->video_st && is->video_st->disposition & 0x0400)
            {
                AVPacket copy;
                if ((ret = av_packet_ref(&copy, &is->video_st->attached_pic)) < 0)
                    goto fail;
                packet_queue_put(&is->videoq, &copy);
                packet_queue_put_nullpacket(&is->videoq, is->video_stream);
            }
            is->queue_attachments_req = 0;
        }
        if (infinite_buffer < 1 &&
            (is->audioq.size + is->videoq.size + is->subtitleq.size > (15 * 1024 * 1024) || (stream_has_enough_packets(is->audio_st, is->audio_stream, &is->audioq) &&
                                                                                             stream_has_enough_packets(is->video_st, is->video_stream, &is->videoq) &&
                                                                                             stream_has_enough_packets(is->subtitle_st, is->subtitle_stream, &is->subtitleq))))
        {
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
                stream_seek(is, start_time != 0 ? start_time : 0, 0, 0);
            }
            else if (autoexit)
            {
                ret = (-(int)(('1') | (('O') << 8) | (('F') << 16) | ((unsigned)(' ') << 24)));
                goto fail;
            }
        }
        ret = av_read_frame(ic, pkt);
        if (ret < 0)
        {
            if ((ret == (-(int)(('1') | (('O') << 8) | (('F') << 16) | ((unsigned)(' ') << 24))) || avio_feof(ic->pb)) && !is->eof)
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
        stream_start_time = ic->streams[pkt->stream_index]->start_time;
        pkt_ts = pkt->pts == 0 ? pkt->dts : pkt->pts;
        pkt_in_play_range = duration == 0 ||
                            (pkt_ts - (stream_start_time != 0 ? stream_start_time : 0)) * av_q2d(ic->streams[pkt->stream_index]->time_base) - (double)(start_time != 0 ? start_time : 0) / 1000000 <= ((double)duration / 1000000);
        if (pkt->stream_index == is->audio_stream && pkt_in_play_range)
        {
            packet_queue_put(&is->audioq, pkt);
        }
        else if (pkt->stream_index == is->video_stream && pkt_in_play_range && !(is->video_st->disposition & 0x0400))
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
    if (!(is->continue_read_thread = SDL_CreateCond()))
    {
        av_log(NULL, 8, "SDL_CreateCond(): %s\n", SDL_GetError());
        goto fail;
    }
    init_clock(&is->vidclk, &is->videoq.serial);
    init_clock(&is->audclk, &is->audioq.serial);
    init_clock(&is->extclk, &is->extclk.serial);
    is->audio_clock_serial = -1;
    if (startup_volume < 0)
        av_log(NULL, 24, "-volume=%d < 0, setting to 0\n", startup_volume);
    if (startup_volume > 100)
        av_log(NULL, 24, "-volume=%d > 100, setting to 100\n", startup_volume);
    startup_volume = av_clip_c(startup_volume, 0, 100);
    startup_volume = av_clip_c(128 * startup_volume / 100, 0, 128);
    is->audio_volume = startup_volume;
    is->muted = 0;
    is->av_sync_type = av_sync_type;
    is->read_tid = SDL_CreateThread(read_thread, "read_thread", is);
    if (!is->read_tid)
    {
        av_log(NULL, 8, "SDL_CreateThread(): %s\n", SDL_GetError());
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
        p = av_find_program_from_stream(ic,
                                        NULL, is->video_stream);
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
    av_log(NULL, 32, "Switch %s stream from #%d to #%d\n",
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
        if (!cursor_hidden && av_gettime_relative() - cursor_last_shown > 1000000)
        {
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
    for (i = 0; i < is->ic->nb_chapters; i++)
    {
        AVChapter *ch = is->ic->chapters[i];
        if (av_compare_ts(pos, (AVRational){1, 1000000}, ch->start, ch->time_base) < 0)
        {
            i--;
            break;
        }
    }
    i += incr;
    i = (i > 0 ? i : 0);
    if (i >= is->ic->nb_chapters)
        return;
    av_log(NULL, 40, "Seeking to chapter %d.\n", i);
    stream_seek(is, av_rescale_q(is->ic->chapters[i]->start, is->ic->chapters[i]->time_base, (AVRational){1, 1000000}), 0, 0);
}

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
                update_volume(cur_stream, 1, (0.75));
                break;
            case SDLK_KP_DIVIDE:
            case SDLK_9:
                update_volume(cur_stream, -1, (0.75));
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
                        pos = (double)cur_stream->seek_pos / 1000000;
                    pos += incr;
                    if (cur_stream->ic->start_time != 0 && pos < cur_stream->ic->start_time / (double)1000000)
                        pos = cur_stream->ic->start_time / (double)1000000;
                    stream_seek(cur_stream, (int64_t)(pos * 1000000), (int64_t)(incr * 1000000), 0);
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
            if (event.button.button == 1)
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
                if (event.button.button != 3)
                    break;
                x = event.button.x;
            }
            else
            {
                if (!(event.motion.state & (1 << ((3) - 1))))
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
                av_log(NULL, 32,"Seek to %2.0f%% (%2d:%02d:%02d) of total duration (%2d:%02d:%02d)       \n", frac * 100,hh, mm, ss, thh, tmm, tss);
                ts = frac * cur_stream->ic->duration;
                if (cur_stream->ic->start_time != 0)
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
                    cur_stream->vis_texture =
                        NULL;
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
    av_log(NULL, 24, "Option -s is deprecated, use -video_size.\n");
    return opt_default(NULL, "video_size", arg);
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
    if (!file_iformat)
    {
        av_log(NULL, 8, "Unknown input format: %s\n", arg);
        return (-(
            22));
    }
    return 0;
}

static int opt_frame_pix_fmt(void *optctx, const char *opt, const char *arg)
{
    av_log(NULL, 24, "Option -pix_fmt is deprecated, use -pixel_format.\n");
    return opt_default(
        NULL, "pixel_format", arg);
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
        av_log(NULL, 16, "Unknown value for %s: %s\n", opt, arg);
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
    show_mode = !strcmp(arg, "video") ? SHOW_MODE_VIDEO : !strcmp(arg, "waves") ? SHOW_MODE_WAVES : !strcmp(arg, "rdft") ? SHOW_MODE_RDFT : parse_number_or_die(opt, arg, 0x0080, 0, SHOW_MODE_NB - 1);
    return 0;
}

static void opt_input_file(void *optctx, const char *filename)
{
    if (input_filename)
    {
        av_log(NULL, 8,"Argument '%s' provided as input filename, but '%s' was already specified.\n",filename, input_filename);
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
        av_log(NULL, 16,
            "No media specifier was specified in '%s' in option '%s'\n",
            arg, opt);
        return (-(22));
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
        av_log(NULL, 16,"Invalid media specifier '%s' in option '%s'\n", spec, opt);
        return (-(22));
    }
    return 0;
}

static int dummy;

static const OptionDef help_options[] = {
    {"L", 0x0800, {.func_arg = show_license}, "show license"},
    {"h", 0x0800, {.func_arg = show_help}, "show help", "topic"},
    {"?", 0x0800, {.func_arg = show_help}, "show help", "topic"},
    {"help", 0x0800, {.func_arg = show_help}, "show help", "topic"},
    {"-help", 0x0800, {.func_arg = show_help}, "show help", "topic"},
    {"version", 0x0800, {.func_arg = show_version}, "show version"},
    {"buildconf", 0x0800, {.func_arg = show_buildconf}, "show build configuration"},
    {"formats", 0x0800, {.func_arg = show_formats}, "show available formats"},
    {"muxers", 0x0800, {.func_arg = show_muxers}, "show available muxers"},
    {"demuxers", 0x0800, {.func_arg = show_demuxers}, "show available demuxers"},
    {"devices", 0x0800, {.func_arg = show_devices}, "show available devices"},
    {"codecs", 0x0800, {.func_arg = show_codecs}, "show available codecs"},
    {"decoders", 0x0800, {.func_arg = show_decoders}, "show available decoders"},
    {"encoders", 0x0800, {.func_arg = show_encoders}, "show available encoders"},
    {"bsfs", 0x0800, {.func_arg = show_bsfs}, "show available bit stream filters"},
    {"protocols", 0x0800, {.func_arg = show_protocols}, "show available protocols"},
    {"filters", 0x0800, {.func_arg = show_filters}, "show available filters"},
    {"pix_fmts", 0x0800, {.func_arg = show_pix_fmts}, "show available pixel formats"},
    {"layouts", 0x0800, {.func_arg = show_layouts}, "show standard channel layouts"},
    {"sample_fmts", 0x0800, {.func_arg = show_sample_fmts}, "show available audio sample formats"},
    {"colors", 0x0800, {.func_arg = show_colors}, "show available color names"},
    {"loglevel", 0x0001, {.func_arg = opt_loglevel}, "set logging level", "loglevel"},
    {"v", 0x0001, {.func_arg = opt_loglevel}, "set logging level", "loglevel"},
    {"report", 0, {.func_arg = opt_report}, "generate a report"},
    {"max_alloc", 0x0001, {.func_arg = opt_max_alloc}, "set maximum size of a single allocated block", "bytes"},
    {"cpuflags", 0x0001 | 0x0004, {.func_arg = opt_cpuflags}, "force specific cpu flags", "flags"},
    {"hide_banner", 0x0002 | 0x0004, {&hide_banner}, "do not show program banner", "hide_banner"},
    {"sources", 0x0800 | 0x0001, {.func_arg = show_sources}, "list sources of the input device", "device"},
    {"sinks", 0x0800 | 0x0001, {.func_arg = show_sinks}, "list sinks of the output device", "device"},
    {"x", 0x0001, {.func_arg = opt_width}, "force displayed width", "width"},
    {"y", 0x0001, {.func_arg = opt_height}, "force displayed height", "height"},
    {"s", 0x0001 | 0x0010, {.func_arg = opt_frame_size}, "set frame size (WxH or abbreviation)", "size"},
    {"fs", 0x0002, {&is_full_screen}, "force full screen"},
    {"an", 0x0002, {&audio_disable}, "disable audio"},
    {"vn", 0x0002, {&video_disable}, "disable video"},
    {"sn", 0x0002, {&subtitle_disable}, "disable subtitling"},
    {"ast", 0x0008 | 0x0001 | 0x0004, {&wanted_stream_spec[AVMEDIA_TYPE_AUDIO]}, "select desired audio stream", "stream_specifier"},
    {"vst", 0x0008 | 0x0001 | 0x0004, {&wanted_stream_spec[AVMEDIA_TYPE_VIDEO]}, "select desired video stream", "stream_specifier"},
    {"sst", 0x0008 | 0x0001 | 0x0004, {&wanted_stream_spec[AVMEDIA_TYPE_SUBTITLE]}, "select desired subtitle stream", "stream_specifier"},
    {"ss", 0x0001, {.func_arg = opt_seek}, "seek to a given position in seconds", "pos"},
    {"t", 0x0001, {.func_arg = opt_duration}, "play  \"duration\" seconds of audio/video", "duration"},
    {"bytes", 0x0080 | 0x0001, {&seek_by_bytes}, "seek by bytes 0=off 1=on -1=auto", "val"},
    {"seek_interval", 0x0100 | 0x0001, {&seek_interval}, "set seek interval for left/right keys, in seconds", "seconds"},
    {"nodisp", 0x0002, {&display_disable}, "disable graphical display"},
    {"noborder", 0x0002, {&borderless}, "borderless window"},
    {"alwaysontop", 0x0002, {&alwaysontop}, "window always on top"},
    {"volume", 0x0080 | 0x0001, {&startup_volume}, "set startup volume 0=min 100=max", "volume"},
    {"f", 0x0001, {.func_arg = opt_format}, "force format", "fmt"},
    {"pix_fmt", 0x0001 | 0x0004 | 0x0010, {.func_arg = opt_frame_pix_fmt}, "set pixel format", "format"},
    {"stats", 0x0002 | 0x0004, {&show_status}, "show status", ""},
    {"fast", 0x0002 | 0x0004, {&fast}, "non spec compliant optimizations", ""},
    {"genpts", 0x0002 | 0x0004, {&genpts}, "generate pts", ""},
    {"drp", 0x0080 | 0x0001 | 0x0004, {&decoder_reorder_pts}, "let decoder reorder pts 0=off 1=on -1=auto", ""},
    {"lowres", 0x0080 | 0x0001 | 0x0004, {&lowres}, "", ""},
    {"sync", 0x0001 | 0x0004, {.func_arg = opt_sync}, "set audio-video sync. type (type=audio/video/ext)", "type"},
    {"autoexit", 0x0002 | 0x0004, {&autoexit}, "exit at the end", ""},
    {"exitonkeydown", 0x0002 | 0x0004, {&exit_on_keydown}, "exit on key down", ""},
    {"exitonmousedown", 0x0002 | 0x0004, {&exit_on_mousedown}, "exit on mouse down", ""},
    {"loop", 0x0080 | 0x0001 | 0x0004, {&loop}, "set number of times the playback shall be looped", "loop count"},
    {"framedrop", 0x0002 | 0x0004, {&framedrop}, "drop frames when cpu is too slow", ""},
    {"infbuf", 0x0002 | 0x0004, {&infinite_buffer}, "don't limit the input buffer size (useful with realtime streams)", ""},
    {"window_title", 0x0008 | 0x0001, {&window_title}, "set window title", "window title"},
    {"left", 0x0080 | 0x0001 | 0x0004, {&screen_left}, "set the x position for the left of the window", "x pos"},
    {"top", 0x0080 | 0x0001 | 0x0004, {&screen_top}, "set the y position for the top of the window", "y pos"},
    {"vf", 0x0004 | 0x0001, {.func_arg = opt_add_vfilter}, "set video filters", "filter_graph"},
    {"af", 0x0008 | 0x0001, {&afilters}, "set audio filters", "filter_graph"},
    {"rdftspeed", 0x0080 | 0x0001 | 0x0020 | 0x0004, {&rdftspeed}, "rdft speed", "msecs"},
    {"showmode", 0x0001, {.func_arg = opt_show_mode}, "select show mode (0 = video, 1 = waves, 2 = RDFT)", "mode"},
    {"default", 0x0001 | 0x0020 | 0x0010 | 0x0004, {.func_arg = opt_default}, "generic catch all option", ""},
    {"i", 0x0002, {&dummy}, "read specified file", "input_file"},
    {"codec", 0x0001, {.func_arg = opt_codec}, "force decoder", "decoder_name"},
    {"acodec", 0x0001 | 0x0008 | 0x0004, {&audio_codec_name}, "force audio decoder", "decoder_name"},
    {"scodec", 0x0001 | 0x0008 | 0x0004, {&subtitle_codec_name}, "force subtitle decoder", "decoder_name"},
    {"vcodec", 0x0001 | 0x0008 | 0x0004, {&video_codec_name}, "force video decoder", "decoder_name"},
    {"autorotate", 0x0002, {&autorotate}, "automatically rotate video", ""},
    {"find_stream_info", 0x0002 | 0x40000 | 0x0004, {&find_stream_info}, "read and decode the streams to fill missing information with heuristics"},
    {"filter_threads", 0x0001 | 0x0080 | 0x0004, {&filter_nbthreads}, "number of filter threads per graph"},
    {
        NULL,
    },
};

static void show_usage(void)
{
    av_log(NULL, 32, "Simple media player\n");
    av_log(NULL, 32, "usage: %s [options] input_file\n", program_name);
    av_log(NULL, 32, "\n");
}

void show_help_default(const char *opt, const char *arg)
{
    av_log_set_callback(log_callback_help);
    show_usage();
    show_help_options(help_options, "Main options:", 0, 0x0004, 0);
    show_help_options(help_options, "Advanced options:", 0x0004, 0, 0);
    printf("\n");
    show_help_children(avcodec_get_class(), 2);
    show_help_children(avformat_get_class(), 2);
    show_help_children(avfilter_get_class(), (1 << 16));
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

int main(int argc, char **argv)
{
    int flags;
    VideoState *is;
    init_dynload();
    av_log_set_flags(1);
    parse_loglevel(argc, argv, help_options);
    avdevice_register_all();
    avformat_network_init();
    init_opts();
    signal(2, sigterm_handler);
    signal(15, sigterm_handler);
    show_banner(argc, argv, help_options);
    parse_options(NULL, argc, argv, help_options, opt_input_file);
    if (!input_filename)
    {
        show_usage();
        av_log(NULL, 8, "An input file must be specified\n");
        av_log(NULL, 8,"Use -h to get full help or, even better, run 'man %s'\n", program_name);
        exit(1);
    }
    if (display_disable)
    {
        video_disable = 1;
    }
    flags = 0x00000020u | 0x00000010u | 0x00000001u;
    if (audio_disable)
        flags &= ~0x00000010u;
    else
    {
        if (!SDL_getenv("SDL_AUDIO_ALSA_SET_BUFFER_SIZE"))
            SDL_setenv("SDL_AUDIO_ALSA_SET_BUFFER_SIZE", "1", 1);
    }
    if (display_disable)
        flags &= ~0x00000020u;
    if (SDL_Init(flags))
    {
        av_log(NULL, 8, "Could not initialize SDL - %s\n", SDL_GetError());
        av_log(NULL, 8, "(Did you set the DISPLAY variable?)\n");
        exit(1);
    }
    SDL_EventState(SDL_SYSWMEVENT, 0);
    SDL_EventState(SDL_USEREVENT, 0);
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
        window = SDL_CreateWindow(program_name, (0x1FFF0000u | (0)), (0x1FFF0000u | (0)), default_width, default_height, flags);
        SDL_SetHint("SDL_RENDER_SCALE_QUALITY", "linear");
        if (window)
        {
            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
            if (!renderer)
            {
                av_log(NULL, 24, "Failed to initialize a hardware accelerated renderer: %s\n", SDL_GetError());
                renderer = SDL_CreateRenderer(window, -1, 0);
            }
            if (renderer)
            {
                if (!SDL_GetRendererInfo(renderer, &renderer_info))
                    av_log(
                        NULL, 40, "Initialized %s renderer.\n", renderer_info.name);
            }
        }
        if (!window || !renderer || !renderer_info.num_texture_formats)
        {
            av_log(NULL, 8, "Failed to create window or renderer: %s", SDL_GetError());
            do_exit(NULL);
        }
    }
    is = stream_open(input_filename, file_iformat);
    if (!is)
    {
        av_log(NULL, 8, "Failed to initialize VideoState!\n");
        do_exit(NULL);
    }
    event_loop(is);
    return 0;
}
