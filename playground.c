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

    echo(s, loglevel, "Opening \'%s\' for %s\n", url, flags & AVIO_FLAG_WRITE ? "writing" : "reading");

    return ffio_open_whitelist(pb, url, flags, &s->interrupt_callback, options, s->protocol_whitelist, s->protocol_blacklist);
}

static void io_close_default(AVFormatContext *s, AVIOContext *pb)
{
    avio_close(pb);
}
// static const AVOption avformat_options[] = {
// {"avioflags", NULL, offsetof(AVFormatContext,avio_flags), AV_OPT_TYPE_FLAGS, {.i64 = DEFAULT }, INT_MIN, INT_MAX, D|E, "avioflags"},
// {"direct", "reduce buffering", 0, AV_OPT_TYPE_CONST, {.i64 = AVIO_FLAG_DIRECT }, INT_MIN, INT_MAX, D|E, "avioflags"},
// {"probesize", "set probing size", offsetof(AVFormatContext,probesize), AV_OPT_TYPE_INT64, {.i64 = 5000000 }, 32, INT64_MAX, D},
// {"formatprobesize", "number of bytes to probe file format", offsetof(AVFormatContext,format_probesize), AV_OPT_TYPE_INT, {.i64 = PROBE_BUF_MAX}, 0, INT_MAX-1, D},
// {"packetsize", "set packet size", offsetof(AVFormatContext,packet_size), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, 0, INT_MAX, E},
// {"fflags", NULL, offsetof(AVFormatContext,flags), AV_OPT_TYPE_FLAGS, {.i64 = AVFMT_FLAG_AUTO_BSF }, INT_MIN, INT_MAX, D|E, "fflags"},
// {"flush_packets", "reduce the latency by flushing out packets immediately", 0, AV_OPT_TYPE_CONST, {.i64 = AVFMT_FLAG_FLUSH_PACKETS }, INT_MIN, INT_MAX, E, "fflags"},
// {"ignidx", "ignore index", 0, AV_OPT_TYPE_CONST, {.i64 = AVFMT_FLAG_IGNIDX }, INT_MIN, INT_MAX, D, "fflags"},
// {"genpts", "generate pts", 0, AV_OPT_TYPE_CONST, {.i64 = AVFMT_FLAG_GENPTS }, INT_MIN, INT_MAX, D, "fflags"},
// {"nofillin", "do not fill in missing values that can be exactly calculated", 0, AV_OPT_TYPE_CONST, {.i64 = AVFMT_FLAG_NOFILLIN }, INT_MIN, INT_MAX, D, "fflags"},
// {"noparse", "disable AVParsers, this needs nofillin too", 0, AV_OPT_TYPE_CONST, {.i64 = AVFMT_FLAG_NOPARSE }, INT_MIN, INT_MAX, D, "fflags"},
// {"igndts", "ignore dts", 0, AV_OPT_TYPE_CONST, {.i64 = AVFMT_FLAG_IGNDTS }, INT_MIN, INT_MAX, D, "fflags"},
// {"discardcorrupt", "discard corrupted frames", 0, AV_OPT_TYPE_CONST, {.i64 = AVFMT_FLAG_DISCARD_CORRUPT }, INT_MIN, INT_MAX, D, "fflags"},
// {"sortdts", "try to interleave outputted packets by dts", 0, AV_OPT_TYPE_CONST, {.i64 = AVFMT_FLAG_SORT_DTS }, INT_MIN, INT_MAX, D, "fflags"},
// {"fastseek", "fast but inaccurate seeks", 0, AV_OPT_TYPE_CONST, {.i64 = AVFMT_FLAG_FAST_SEEK }, INT_MIN, INT_MAX, D, "fflags"},
// {"nobuffer", "reduce the latency introduced by optional buffering", 0, AV_OPT_TYPE_CONST, {.i64 = AVFMT_FLAG_NOBUFFER }, 0, INT_MAX, D, "fflags"},
// {"bitexact", "do not write random/volatile data", 0, AV_OPT_TYPE_CONST, { .i64 = AVFMT_FLAG_BITEXACT }, 0, 0, E, "fflags" },
// {"shortest", "stop muxing with the shortest stream", 0, AV_OPT_TYPE_CONST, { .i64 = AVFMT_FLAG_SHORTEST }, 0, 0, E, "fflags" },
// {"autobsf", "add needed bsfs automatically", 0, AV_OPT_TYPE_CONST, { .i64 = AVFMT_FLAG_AUTO_BSF }, 0, 0, E, "fflags" },
// {"seek2any", "allow seeking to non-keyframes on demuxer level when supported", offsetof(AVFormatContext,seek2any), AV_OPT_TYPE_BOOL, {.i64 = 0 }, 0, 1, D},
// {"analyzeduration", "specify how many microseconds are analyzed to probe the input", offsetof(AVFormatContext,max_analyze_duration), AV_OPT_TYPE_INT64, {.i64 = 0 }, 0, INT64_MAX, D},
// {"cryptokey", "decryption key", offsetof(AVFormatContext,key), AV_OPT_TYPE_BINARY, {.dbl = 0}, 0, 0, D},
// {"indexmem", "max memory used for timestamp index (per stream)", offsetof(AVFormatContext,max_index_size), AV_OPT_TYPE_INT, {.i64 = 1<<20 }, 0, INT_MAX, D},
// {"rtbufsize", "max memory used for buffering real-time frames", offsetof(AVFormatContext,max_picture_buffer), AV_OPT_TYPE_INT, {.i64 = 3041280 }, 0, INT_MAX, D}, /* defaults to 1s of 15fps 352x288 YUYV422 video */
// {"fdebug", "print specific debug info", offsetof(AVFormatContext,debug), AV_OPT_TYPE_FLAGS, {.i64 = DEFAULT }, 0, INT_MAX, E|D, "fdebug"},
// {"ts", NULL, 0, AV_OPT_TYPE_CONST, {.i64 = FF_FDEBUG_TS }, INT_MIN, INT_MAX, E|D, "fdebug"},
// {"max_delay", "maximum muxing or demuxing delay in microseconds", offsetof(AVFormatContext,max_delay), AV_OPT_TYPE_INT, {.i64 = -1 }, -1, INT_MAX, E|D},
// {"start_time_realtime", "wall-clock time when stream begins (PTS==0)", offsetof(AVFormatContext,start_time_realtime), AV_OPT_TYPE_INT64, {.i64 = AV_NOPTS_VALUE}, INT64_MIN, INT64_MAX, E},
// {"fpsprobesize", "number of frames used to probe fps", offsetof(AVFormatContext,fps_probe_size), AV_OPT_TYPE_INT, {.i64 = -1}, -1, INT_MAX-1, D},
// {"audio_preload", "microseconds by which audio packets should be interleaved earlier", offsetof(AVFormatContext,audio_preload), AV_OPT_TYPE_INT, {.i64 = 0}, 0, INT_MAX-1, E},
// {"chunk_duration", "microseconds for each chunk", offsetof(AVFormatContext,max_chunk_duration), AV_OPT_TYPE_INT, {.i64 = 0}, 0, INT_MAX-1, E},
// {"chunk_size", "size in bytes for each chunk", offsetof(AVFormatContext,max_chunk_size), AV_OPT_TYPE_INT, {.i64 = 0}, 0, INT_MAX-1, E},

// {"f_err_detect", "set error detection flags (deprecated; use err_detect, save via avconv)", offsetof(AVFormatContext,error_recognition), AV_OPT_TYPE_FLAGS, {.i64 = AV_EF_CRCCHECK }, INT_MIN, INT_MAX, D, "err_detect"},
// {"err_detect", "set error detection flags", offsetof(AVFormatContext,error_recognition), AV_OPT_TYPE_FLAGS, {.i64 = AV_EF_CRCCHECK }, INT_MIN, INT_MAX, D, "err_detect"},
// {"crccheck", "verify embedded CRCs", 0, AV_OPT_TYPE_CONST, {.i64 = AV_EF_CRCCHECK }, INT_MIN, INT_MAX, D, "err_detect"},
// {"bitstream", "detect bitstream specification deviations", 0, AV_OPT_TYPE_CONST, {.i64 = AV_EF_BITSTREAM }, INT_MIN, INT_MAX, D, "err_detect"},
// {"buffer", "detect improper bitstream length", 0, AV_OPT_TYPE_CONST, {.i64 = AV_EF_BUFFER }, INT_MIN, INT_MAX, D, "err_detect"},
// {"explode", "abort decoding on minor error detection", 0, AV_OPT_TYPE_CONST, {.i64 = AV_EF_EXPLODE }, INT_MIN, INT_MAX, D, "err_detect"},
// {"ignore_err", "ignore errors", 0, AV_OPT_TYPE_CONST, {.i64 = AV_EF_IGNORE_ERR }, INT_MIN, INT_MAX, D, "err_detect"},
// {"careful",    "consider things that violate the spec, are fast to check and have not been seen in the wild as errors", 0, AV_OPT_TYPE_CONST, {.i64 = AV_EF_CAREFUL }, INT_MIN, INT_MAX, D, "err_detect"},
// {"compliant",  "consider all spec non compliancies as errors", 0, AV_OPT_TYPE_CONST, {.i64 = AV_EF_COMPLIANT | AV_EF_CAREFUL }, INT_MIN, INT_MAX, D, "err_detect"},
// {"aggressive", "consider things that a sane encoder shouldn't do as an error", 0, AV_OPT_TYPE_CONST, {.i64 = AV_EF_AGGRESSIVE | AV_EF_COMPLIANT | AV_EF_CAREFUL}, INT_MIN, INT_MAX, D, "err_detect"},
// {"use_wallclock_as_timestamps", "use wallclock as timestamps", offsetof(AVFormatContext,use_wallclock_as_timestamps), AV_OPT_TYPE_BOOL, {.i64 = 0}, 0, 1, D},
// {"skip_initial_bytes", "set number of bytes to skip before reading header and frames", offsetof(AVFormatContext,skip_initial_bytes), AV_OPT_TYPE_INT64, {.i64 = 0}, 0, INT64_MAX-1, D},
// {"correct_ts_overflow", "correct single timestamp overflows", offsetof(AVFormatContext,correct_ts_overflow), AV_OPT_TYPE_BOOL, {.i64 = 1}, 0, 1, D},
// {"flush_packets", "enable flushing of the I/O context after each packet", offsetof(AVFormatContext,flush_packets), AV_OPT_TYPE_INT, {.i64 = -1}, -1, 1, E},
// {"metadata_header_padding", "set number of bytes to be written as padding in a metadata header", offsetof(AVFormatContext,metadata_header_padding), AV_OPT_TYPE_INT, {.i64 = -1}, -1, INT_MAX, E},
// {"output_ts_offset", "set output timestamp offset", offsetof(AVFormatContext,output_ts_offset), AV_OPT_TYPE_DURATION, {.i64 = 0}, -INT64_MAX, INT64_MAX, E},
// {"max_interleave_delta", "maximum buffering duration for interleaving", offsetof(AVFormatContext,max_interleave_delta), AV_OPT_TYPE_INT64, { .i64 = 10000000 }, 0, INT64_MAX, E },
// {"f_strict", "how strictly to follow the standards (deprecated; use strict, save via avconv)", offsetof(AVFormatContext,strict_std_compliance), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, INT_MIN, INT_MAX, D|E, "strict"},
// {"strict", "how strictly to follow the standards", offsetof(AVFormatContext,strict_std_compliance), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, INT_MIN, INT_MAX, D|E, "strict"},
// {"very", "strictly conform to a older more strict version of the spec or reference software", 0, AV_OPT_TYPE_CONST, {.i64 = FF_COMPLIANCE_VERY_STRICT }, INT_MIN, INT_MAX, D|E, "strict"},
// {"strict", "strictly conform to all the things in the spec no matter what the consequences", 0, AV_OPT_TYPE_CONST, {.i64 = FF_COMPLIANCE_STRICT }, INT_MIN, INT_MAX, D|E, "strict"},
// {"normal", NULL, 0, AV_OPT_TYPE_CONST, {.i64 = FF_COMPLIANCE_NORMAL }, INT_MIN, INT_MAX, D|E, "strict"},
// {"unofficial", "allow unofficial extensions", 0, AV_OPT_TYPE_CONST, {.i64 = FF_COMPLIANCE_UNOFFICIAL }, INT_MIN, INT_MAX, D|E, "strict"},
// {"experimental", "allow non-standardized experimental variants", 0, AV_OPT_TYPE_CONST, {.i64 = FF_COMPLIANCE_EXPERIMENTAL }, INT_MIN, INT_MAX, D|E, "strict"},
// {"max_ts_probe", "maximum number of packets to read while waiting for the first timestamp", offsetof(AVFormatContext,max_ts_probe), AV_OPT_TYPE_INT, { .i64 = 50 }, 0, INT_MAX, D },
// {"avoid_negative_ts", "shift timestamps so they start at 0", offsetof(AVFormatContext,avoid_negative_ts), AV_OPT_TYPE_INT, {.i64 = -1}, -1, 2, E, "avoid_negative_ts"},
// {"auto",              "enabled when required by target format",    0, AV_OPT_TYPE_CONST, {.i64 = AVFMT_AVOID_NEG_TS_AUTO },              INT_MIN, INT_MAX, E, "avoid_negative_ts"},
// {"disabled",          "do not change timestamps",                  0, AV_OPT_TYPE_CONST, {.i64 = 0 },                                    INT_MIN, INT_MAX, E, "avoid_negative_ts"},
// {"make_non_negative", "shift timestamps so they are non negative", 0, AV_OPT_TYPE_CONST, {.i64 = AVFMT_AVOID_NEG_TS_MAKE_NON_NEGATIVE }, INT_MIN, INT_MAX, E, "avoid_negative_ts"},
// {"make_zero",         "shift timestamps so they start at 0",       0, AV_OPT_TYPE_CONST, {.i64 = AVFMT_AVOID_NEG_TS_MAKE_ZERO },         INT_MIN, INT_MAX, E, "avoid_negative_ts"},
// {"dump_separator", "set information dump field separator", offsetof(AVFormatContext,dump_separator), AV_OPT_TYPE_STRING, {.str = ", "}, 0, 0, D|E},
// {"codec_whitelist", "List of decoders that are allowed to be used", offsetof(AVFormatContext,codec_whitelist), AV_OPT_TYPE_STRING, { .str = NULL },  0, 0, D },
// {"format_whitelist", "List of demuxers that are allowed to be used", offsetof(AVFormatContext,format_whitelist), AV_OPT_TYPE_STRING, { .str = NULL },  0, 0, D },
// {"protocol_whitelist", "List of protocols that are allowed to be used", offsetof(AVFormatContext,protocol_whitelist), AV_OPT_TYPE_STRING, { .str = NULL },  0, 0, D },
// {"protocol_blacklist", "List of protocols that are not allowed to be used", offsetof(AVFormatContext,protocol_blacklist), AV_OPT_TYPE_STRING, { .str = NULL },  0, 0, D },
// {"max_streams", "maximum number of streams", offsetof(AVFormatContext,max_streams), AV_OPT_TYPE_INT, { .i64 = 1000 }, 0, INT_MAX, D },
// {"skip_estimate_duration_from_pts", "skip duration calculation in estimate_timings_from_pts", offsetof(AVFormatContext,skip_estimate_duration_from_pts), AV_OPT_TYPE_BOOL, {.i64 = 0}, 0, 1, D},
// {"max_probe_packets", "Maximum number of packets to probe a codec", offsetof(AVFormatContext,max_probe_packets), AV_OPT_TYPE_INT, { .i64 = 2500 }, 0, INT_MAX, D },
// {NULL},
// };
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
static AVClassCategory AVClass_get_category(void *ptr)
{
    AVCodecContext* avctx = ptr;
    if(avctx->codec && avctx->codec->decode) return AV_CLASS_CATEGORY_DECODER;
    else                                     return AV_CLASS_CATEGORY_ENCODER;
}
static const char* format_to_name(void* ptr)
{
    AVFormatContext* fc = (AVFormatContext*) ptr;
    if(fc->iformat) return fc->iformat->name;
    else if(fc->oformat) return fc->oformat->name;
    else return "NULL";
}
// static const AVClass av_format_context_class = {
//     .class_name     = "AVFormatContext",
//     .item_name      = format_to_name,
//     .option         = avformat_options,
//     .version        = LIBAVUTIL_VERSION_INT,
//     .child_next     = format_child_next,
//     .child_class_iterate = format_child_class_iterate,
//     .category       = AV_CLASS_CATEGORY_MUXER,
//     .get_category   = AVClass_get_category,
// };
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