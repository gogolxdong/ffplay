#include "ffplay.h"


#define AA_MAGIC 1469084982 /* this identifies an audible .aa file */
#define MAX_CODEC_SECOND_SIZE 3982
#define MAX_TOC_ENTRIES 16
#define MAX_DICTIONARY_ENTRIES 128
#define TEA_BLOCK_SIZE 8
#define CHAPTER_HEADER_SIZE 8
#define TIMEPREC 1000
#define MP3_FRAME_SIZE 104


static int get_second_size(char *codec_name)
{
    int result = -1;

    if (!strcmp(codec_name, "mp332")) {
        result = 3982;
    } else if (!strcmp(codec_name, "acelp16")) {
        result = 2000;
    } else if (!strcmp(codec_name, "acelp85")) {
        result = 1045;
    }

    return result;
}

static int aa_read_header(AVFormatContext *s)
{
    int i, j, idx, largest_idx = -1;
    uint32_t nkey, nval, toc_size, npairs, header_seed = 0, start;
    char key[128], val[128], codec_name[64] = {0};
    uint8_t output[24], dst[8], src[8];
    int64_t largest_size = -1, current_size = -1, chapter_pos;
    struct toc_entry {
        uint32_t offset;
        uint32_t size;
    } TOC[MAX_TOC_ENTRIES];
    uint32_t header_key_part[4];
    uint8_t header_key[16] = {0};
    AADemuxContext *c = s->priv_data;
    AVIOContext *pb = s->pb;
    AVStream *st;
    int ret;

    /* parse .aa header */
    avio_skip(pb, 4); // file size
    avio_skip(pb, 4); // magic string
    toc_size = avio_rb32(pb); // TOC size
    avio_skip(pb, 4); // unidentified integer
    if (toc_size > MAX_TOC_ENTRIES || toc_size < 2)
        return AVERROR_INVALIDDATA;
    for (i = 0; i < toc_size; i++) { // read TOC
        avio_skip(pb, 4); // TOC entry index
        TOC[i].offset = avio_rb32(pb); // block offset
        TOC[i].size = avio_rb32(pb); // block size
    }
    avio_skip(pb, 24); // header termination block (ignored)
    npairs = avio_rb32(pb); // read dictionary entries
    if (npairs > MAX_DICTIONARY_ENTRIES)
        return AVERROR_INVALIDDATA;
    for (i = 0; i < npairs; i++) {
        memset(val, 0, sizeof(val));
        memset(key, 0, sizeof(key));
        avio_skip(pb, 1); // unidentified integer
        nkey = avio_rb32(pb); // key string length
        nval = avio_rb32(pb); // value string length
        avio_get_str(pb, nkey, key, sizeof(key));
        avio_get_str(pb, nval, val, sizeof(val));
        if (!strcmp(key, "codec")) {
            av_log(s, AV_LOG_DEBUG, "Codec is <%s>\n", val);
            strncpy(codec_name, val, sizeof(codec_name) - 1);
        } else if (!strcmp(key, "HeaderSeed")) {
            av_log(s, AV_LOG_DEBUG, "HeaderSeed is <%s>\n", val);
            header_seed = atoi(val);
        } else if (!strcmp(key, "HeaderKey")) { // this looks like "1234567890 1234567890 1234567890 1234567890"
            av_log(s, AV_LOG_DEBUG, "HeaderKey is <%s>\n", val);

            ret = sscanf(val, "%"SCNu32"%"SCNu32"%"SCNu32"%"SCNu32,
                   &header_key_part[0], &header_key_part[1], &header_key_part[2], &header_key_part[3]);
            if (ret != 4)
                return AVERROR_INVALIDDATA;

            for (idx = 0; idx < 4; idx++) {
                AV_WB32(&header_key[idx * 4], header_key_part[idx]); // convert each part to BE!
            }
            av_log(s, AV_LOG_DEBUG, "Processed HeaderKey is ");
            for (i = 0; i < 16; i++)
                av_log(s, AV_LOG_DEBUG, "%02x", header_key[i]);
            av_log(s, AV_LOG_DEBUG, "\n");
        } else {
            av_dict_set(&s->metadata, key, val, 0);
        }
    }

    /* verify fixed key */
    if (c->aa_fixed_key_len != 16) {
        av_log(s, AV_LOG_ERROR, "aa_fixed_key value needs to be 16 bytes!\n");
        return AVERROR(EINVAL);
    }

    /* verify codec */
    if ((c->codec_second_size = get_second_size(codec_name)) == -1) {
        av_log(s, AV_LOG_ERROR, "unknown codec <%s>!\n", codec_name);
        return AVERROR(EINVAL);
    }

    /* decryption key derivation */
    c->tea_ctx = av_tea_alloc();
    if (!c->tea_ctx)
        return AVERROR(ENOMEM);
    av_tea_init(c->tea_ctx, c->aa_fixed_key, 16);
    output[0] = output[1] = 0; // purely for padding purposes
    memcpy(output + 2, header_key, 16);
    idx = 0;
    for (i = 0; i < 3; i++) { // TEA CBC with weird mixed endianness
        AV_WB32(src, header_seed);
        AV_WB32(src + 4, header_seed + 1);
        header_seed += 2;
        av_tea_crypt(c->tea_ctx, dst, src, 1, NULL, 0); // TEA ECB encrypt
        for (j = 0; j < TEA_BLOCK_SIZE && idx < 18; j+=1, idx+=1) {
            output[idx] = output[idx] ^ dst[j];
        }
    }
    memcpy(c->file_key, output + 2, 16); // skip first 2 bytes of output
    av_log(s, AV_LOG_DEBUG, "File key is ");
    for (i = 0; i < 16; i++)
        av_log(s, AV_LOG_DEBUG, "%02x", c->file_key[i]);
    av_log(s, AV_LOG_DEBUG, "\n");

    /* decoder setup */
    st = avformat_new_stream(s, NULL);
    if (!st) {
        av_freep(&c->tea_ctx);
        return AVERROR(ENOMEM);
    }
    st->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
    if (!strcmp(codec_name, "mp332")) {
        st->codecpar->codec_id = AV_CODEC_ID_MP3;
        st->codecpar->sample_rate = 22050;
        st->need_parsing = AVSTREAM_PARSE_FULL_RAW;
        avpriv_set_pts_info(st, 64, 8, 32000 * TIMEPREC);
        // encoded audio frame is MP3_FRAME_SIZE bytes (+1 with padding, unlikely)
    } else if (!strcmp(codec_name, "acelp85")) {
        st->codecpar->codec_id = AV_CODEC_ID_SIPR;
        st->codecpar->block_align = 19;
        st->codecpar->channels = 1;
        st->codecpar->sample_rate = 8500;
        st->codecpar->bit_rate = 8500;
        st->need_parsing = AVSTREAM_PARSE_FULL_RAW;
        avpriv_set_pts_info(st, 64, 8, 8500 * TIMEPREC);
    } else if (!strcmp(codec_name, "acelp16")) {
        st->codecpar->codec_id = AV_CODEC_ID_SIPR;
        st->codecpar->block_align = 20;
        st->codecpar->channels = 1;
        st->codecpar->sample_rate = 16000;
        st->codecpar->bit_rate = 16000;
        st->need_parsing = AVSTREAM_PARSE_FULL_RAW;
        avpriv_set_pts_info(st, 64, 8, 16000 * TIMEPREC);
    }

    /* determine, and jump to audio start offset */
    for (i = 1; i < toc_size; i++) { // skip the first entry!
        current_size = TOC[i].size;
        if (current_size > largest_size) {
            largest_idx = i;
            largest_size = current_size;
        }
    }
    start = TOC[largest_idx].offset;
    avio_seek(pb, start, SEEK_SET);

    // extract chapter positions. since all formats have constant bit rate, use it
    // as time base in bytes/s, for easy stream position <-> timestamp conversion
    st->start_time = 0;
    c->content_start = start;
    c->content_end = start + largest_size;

    while ((chapter_pos = avio_tell(pb)) >= 0 && chapter_pos < c->content_end) {
        int chapter_idx = s->nb_chapters;
        uint32_t chapter_size = avio_rb32(pb);
        if (chapter_size == 0) break;
        chapter_pos -= start + CHAPTER_HEADER_SIZE * chapter_idx;
        avio_skip(pb, 4 + chapter_size);
        if (!avpriv_new_chapter(s, chapter_idx, st->time_base,
            chapter_pos * TIMEPREC, (chapter_pos + chapter_size) * TIMEPREC, NULL))
                return AVERROR(ENOMEM);
    }

    st->duration = (largest_size - CHAPTER_HEADER_SIZE * s->nb_chapters) * TIMEPREC;

    ff_update_cur_dts(s, st, 0);
    avio_seek(pb, start, SEEK_SET);
    c->current_chapter_size = 0;
    c->seek_offset = 0;

    return 0;
}

static int aa_read_packet(AVFormatContext *s, AVPacket *pkt)
{
    uint8_t dst[TEA_BLOCK_SIZE];
    uint8_t src[TEA_BLOCK_SIZE];
    int i;
    int trailing_bytes;
    int blocks;
    uint8_t buf[MAX_CODEC_SECOND_SIZE * 2];
    int written = 0;
    int ret;
    AADemuxContext *c = s->priv_data;
    uint64_t pos = avio_tell(s->pb);

    // are we at the end of the audio content?
    if (pos >= c->content_end) {
        return AVERROR_EOF;
    }

    // are we at the start of a chapter?
    if (c->current_chapter_size == 0) {
        c->current_chapter_size = avio_rb32(s->pb);
        if (c->current_chapter_size == 0) {
            return AVERROR_EOF;
        }
        av_log(s, AV_LOG_DEBUG, "Chapter %d (%" PRId64 " bytes)\n", c->chapter_idx, c->current_chapter_size);
        c->chapter_idx = c->chapter_idx + 1;
        avio_skip(s->pb, 4); // data start offset
        pos += 8;
        c->current_codec_second_size = c->codec_second_size;
    }

    // is this the last block in this chapter?
    if (c->current_chapter_size / c->current_codec_second_size == 0) {
        c->current_codec_second_size = c->current_chapter_size % c->current_codec_second_size;
    }

    // decrypt c->current_codec_second_size bytes
    blocks = c->current_codec_second_size / TEA_BLOCK_SIZE;
    for (i = 0; i < blocks; i++) {
        ret = avio_read(s->pb, src, TEA_BLOCK_SIZE);
        if (ret != TEA_BLOCK_SIZE)
            return (ret < 0) ? ret : AVERROR_EOF;
        av_tea_init(c->tea_ctx, c->file_key, 16);
        av_tea_crypt(c->tea_ctx, dst, src, 1, NULL, 1);
        memcpy(buf + written, dst, TEA_BLOCK_SIZE);
        written = written + TEA_BLOCK_SIZE;
    }
    trailing_bytes = c->current_codec_second_size % TEA_BLOCK_SIZE;
    if (trailing_bytes != 0) { // trailing bytes are left unencrypted!
        ret = avio_read(s->pb, src, trailing_bytes);
        if (ret != trailing_bytes)
            return (ret < 0) ? ret : AVERROR_EOF;
        memcpy(buf + written, src, trailing_bytes);
        written = written + trailing_bytes;
    }

    // update state
    c->current_chapter_size = c->current_chapter_size - c->current_codec_second_size;
    if (c->current_chapter_size <= 0)
        c->current_chapter_size = 0;

    if (c->seek_offset > written)
        c->seek_offset = 0; // ignore wrong estimate

    ret = av_new_packet(pkt, written - c->seek_offset);
    if (ret < 0)
        return ret;
    memcpy(pkt->data, buf + c->seek_offset, written - c->seek_offset);
    pkt->pos = pos;

    c->seek_offset = 0;
    return 0;
}

static int aa_read_seek(AVFormatContext *s,
                        int stream_index, int64_t timestamp, int flags)
{
    AADemuxContext *c = s->priv_data;
    AVChapter *ch;
    int64_t chapter_pos, chapter_start, chapter_size;
    int chapter_idx = 0;

    // find chapter containing seek timestamp
    if (timestamp < 0)
        timestamp = 0;

    while (chapter_idx < s->nb_chapters && timestamp >= s->chapters[chapter_idx]->end) {
        ++chapter_idx;
    }

    if (chapter_idx >= s->nb_chapters) {
        chapter_idx = s->nb_chapters - 1;
        if (chapter_idx < 0) return -1; // there is no chapter.
        timestamp = s->chapters[chapter_idx]->end;
    }

    ch = s->chapters[chapter_idx];

    // sync by clamping timestamp to nearest valid block position in its chapter
    chapter_size = ch->end / TIMEPREC - ch->start / TIMEPREC;
    chapter_pos = av_rescale_rnd((timestamp - ch->start) / TIMEPREC,
        1, c->codec_second_size,
        (flags & AVSEEK_FLAG_BACKWARD) ? AV_ROUND_DOWN : AV_ROUND_UP)
        * c->codec_second_size;
    if (chapter_pos >= chapter_size)
        chapter_pos = chapter_size;
    chapter_start = c->content_start + (ch->start / TIMEPREC) + CHAPTER_HEADER_SIZE * (1 + chapter_idx);

    // reinit read state
    avio_seek(s->pb, chapter_start + chapter_pos, SEEK_SET);
    c->current_codec_second_size = c->codec_second_size;
    c->current_chapter_size = chapter_size - chapter_pos;
    c->chapter_idx = 1 + chapter_idx;

    // for unaligned frames, estimate offset of first frame in block (assume no padding)
    if (s->streams[0]->codecpar->codec_id == AV_CODEC_ID_MP3) {
        c->seek_offset = (MP3_FRAME_SIZE - chapter_pos % MP3_FRAME_SIZE) % MP3_FRAME_SIZE;
    }

    ff_update_cur_dts(s, s->streams[0], ch->start + (chapter_pos + c->seek_offset) * TIMEPREC);

    return 1;
}

static int aa_probe(const AVProbeData *p)
{
    uint8_t *buf = p->buf;

    // first 4 bytes are file size, next 4 bytes are the magic
    if (AV_RB32(buf+4) != AA_MAGIC)
        return 0;

    return AVPROBE_SCORE_MAX / 2;
}

static int aa_read_close(AVFormatContext *s)
{
    AADemuxContext *c = s->priv_data;

    av_freep(&c->tea_ctx);

    return 0;
}

#define OFFSET(x) offsetof(AADemuxContext, x)
static const AVOption aa_options[] = {
    { "aa_fixed_key", // extracted from libAAX_SDK.so and AAXSDKWin.dll files!
        "Fixed key used for handling Audible AA files", OFFSET(aa_fixed_key),
        AV_OPT_TYPE_BINARY, {.str="77214d4b196a87cd520045fd2a51d673"},
        .flags = AV_OPT_FLAG_DECODING_PARAM },
    { NULL },
};

static const AVClass aa_class = {
    .class_name = "aa",
    .item_name  = av_default_item_name,
    .option     = aa_options,
    .version    = LIBAVUTIL_VERSION_INT,
};

AVInputFormat ff_aa_demuxer = {
    .name           = "aa",
    .long_name      = NULL_IF_CONFIG_SMALL("Audible AA format files"),
    .priv_class     = &aa_class,
    .priv_data_size = sizeof(AADemuxContext),
    .extensions     = "aa",
    .read_probe     = aa_probe,
    .read_header    = aa_read_header,
    .read_packet    = aa_read_packet,
    .read_seek      = aa_read_seek,
    .read_close     = aa_read_close,
    .flags          = AVFMT_NO_BYTE_SEEK | AVFMT_NOGENSEARCH,
};

#define ADTS_HEADER_SIZE 7

static int adts_aac_probe(const AVProbeData *p)
{
    int max_frames = 0, first_frames = 0;
    int fsize, frames;
    const uint8_t *buf0 = p->buf;
    const uint8_t *buf2;
    const uint8_t *buf;
    const uint8_t *end = buf0 + p->buf_size - 7;

    buf = buf0;

    for (; buf < end; buf = buf2 + 1) {
        buf2 = buf;

        for (frames = 0; buf2 < end; frames++) {
            uint32_t header = AV_RB16(buf2);
            if ((header & 0xFFF6) != 0xFFF0) {
                if (buf != buf0) {
                    // Found something that isn't an ADTS header, starting
                    // from a position other than the start of the buffer.
                    // Discard the count we've accumulated so far since it
                    // probably was a false positive.
                    frames = 0;
                }
                break;
            }
            fsize = (AV_RB32(buf2 + 3) >> 13) & 0x1FFF;
            if (fsize < 7)
                break;
            fsize = FFMIN(fsize, end - buf2);
            buf2 += fsize;
        }
        max_frames = FFMAX(max_frames, frames);
        if (buf == buf0)
            first_frames = frames;
    }

    if (first_frames >= 3)
        return AVPROBE_SCORE_EXTENSION + 1;
    else if (max_frames > 100)
        return AVPROBE_SCORE_EXTENSION;
    else if (max_frames >= 3)
        return AVPROBE_SCORE_EXTENSION / 2;
    else if (first_frames >= 1)
        return 1;
    else
        return 0;
}

static int adts_aac_resync(AVFormatContext *s)
{
    uint16_t state;

    // skip data until an ADTS frame is found
    state = avio_r8(s->pb);
    while (!avio_feof(s->pb) && avio_tell(s->pb) < s->probesize) {
        state = (state << 8) | avio_r8(s->pb);
        if ((state >> 4) != 0xFFF)
            continue;
        avio_seek(s->pb, -2, SEEK_CUR);
        break;
    }
    if (s->pb->eof_reached)
        return AVERROR_EOF;
    if ((state >> 4) != 0xFFF)
        return AVERROR_INVALIDDATA;

    return 0;
}

static int adts_aac_read_header(AVFormatContext *s)
{
    AVStream *st;
    int ret;

    st = avformat_new_stream(s, NULL);
    if (!st)
        return AVERROR(ENOMEM);

    st->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
    st->codecpar->codec_id   = s->iformat->raw_codec_id;
    st->need_parsing         = AVSTREAM_PARSE_FULL_RAW;

    ff_id3v1_read(s);
    if ((s->pb->seekable & AVIO_SEEKABLE_NORMAL) &&
        !av_dict_get(s->metadata, "", NULL, AV_DICT_IGNORE_SUFFIX)) {
        int64_t cur = avio_tell(s->pb);
        ff_ape_parse_tag(s);
        avio_seek(s->pb, cur, SEEK_SET);
    }

    ret = adts_aac_resync(s);
    if (ret < 0)
        return ret;

    // LCM of all possible ADTS sample rates
    avpriv_set_pts_info(st, 64, 1, 28224000);

    return 0;
}

static int handle_id3(AVFormatContext *s, AVPacket *pkt)
{
    AVDictionary *metadata = NULL;
    AVIOContext ioctx;
    ID3v2ExtraMeta *id3v2_extra_meta = NULL;
    int ret;

    ret = av_append_packet(s->pb, pkt, ff_id3v2_tag_len(pkt->data) - pkt->size);
    if (ret < 0) {
        return ret;
    }

    ffio_init_context(&ioctx, pkt->data, pkt->size, 0, NULL, NULL, NULL, NULL);
    ff_id3v2_read_dict(&ioctx, &metadata, ID3v2_DEFAULT_MAGIC, &id3v2_extra_meta);
    if ((ret = ff_id3v2_parse_priv_dict(&metadata, id3v2_extra_meta)) < 0)
        goto error;

    if (metadata) {
        if ((ret = av_dict_copy(&s->metadata, metadata, 0)) < 0)
            goto error;
        s->event_flags |= AVFMT_EVENT_FLAG_METADATA_UPDATED;
    }

error:
    av_packet_unref(pkt);
    ff_id3v2_free_extra_meta(&id3v2_extra_meta);
    av_dict_free(&metadata);

    return ret;
}

static int adts_aac_read_packet(AVFormatContext *s, AVPacket *pkt)
{
    int ret, fsize;

retry:
    ret = av_get_packet(s->pb, pkt, ADTS_HEADER_SIZE);
    if (ret < 0)
        return ret;

    if (ret < ADTS_HEADER_SIZE) {
        return AVERROR(EIO);
    }

    if ((AV_RB16(pkt->data) >> 4) != 0xfff) {
        // Parse all the ID3 headers between frames
        int append = ID3v2_HEADER_SIZE - ADTS_HEADER_SIZE;

        av_assert2(append > 0);
        ret = av_append_packet(s->pb, pkt, append);
        if (ret != append) {
            return AVERROR(EIO);
        }
        if (!ff_id3v2_match(pkt->data, ID3v2_DEFAULT_MAGIC)) {
            av_packet_unref(pkt);
            ret = adts_aac_resync(s);
        } else
            ret = handle_id3(s, pkt);
        if (ret < 0)
            return ret;

        goto retry;
    }

    fsize = (AV_RB32(pkt->data + 3) >> 13) & 0x1FFF;
    if (fsize < ADTS_HEADER_SIZE) {
        return AVERROR_INVALIDDATA;
    }

    ret = av_append_packet(s->pb, pkt, fsize - pkt->size);

    return ret;
}

typedef struct AAXColumn {
    uint8_t flag;
    uint8_t type;
    const char *name;
    uint32_t offset;
    int size;
} AAXColumn;

typedef struct AAXSegment {
    int64_t start;
    int64_t end;
} AAXSegment;

typedef struct AAXContext {
    int64_t table_size;
    uint16_t version;
    int64_t rows_offset;
    int64_t strings_offset;
    int64_t data_offset;
    int64_t name_offset;
    uint16_t columns;
    uint16_t row_width;
    uint32_t nb_segments;
    int64_t schema_offset;
    int64_t strings_size;
    char *string_table;

    uint32_t current_segment;

    AAXColumn *xcolumns;
    AAXSegment *segments;
} AAXContext;

static int aax_probe(const AVProbeData *p)
{
    if (AV_RB32(p->buf) != MKBETAG('@','U','T','F'))
        return 0;
    if (AV_RB32(p->buf + 4) == 0)
        return 0;
    if (AV_RB16(p->buf + 8) > 1)
        return 0;
    if (AV_RB32(p->buf + 28) < 1)
        return 0;

    return AVPROBE_SCORE_MAX;
}

enum ColumnFlag {
    COLUMN_FLAG_NAME            = 0x1,
    COLUMN_FLAG_DEFAULT         = 0x2,
    COLUMN_FLAG_ROW             = 0x4,
    COLUMN_FLAG_UNDEFINED       = 0x8 /* shouldn't exist */
};

enum ColumnType {
    COLUMN_TYPE_UINT8           = 0x00,
    COLUMN_TYPE_SINT8           = 0x01,
    COLUMN_TYPE_UINT16          = 0x02,
    COLUMN_TYPE_SINT16          = 0x03,
    COLUMN_TYPE_UINT32          = 0x04,
    COLUMN_TYPE_SINT32          = 0x05,
    COLUMN_TYPE_UINT64          = 0x06,
    COLUMN_TYPE_SINT64          = 0x07,
    COLUMN_TYPE_FLOAT           = 0x08,
    COLUMN_TYPE_DOUBLE          = 0x09,
    COLUMN_TYPE_STRING          = 0x0a,
    COLUMN_TYPE_VLDATA          = 0x0b,
    COLUMN_TYPE_UINT128         = 0x0c, /* for GUIDs */
    COLUMN_TYPE_UNDEFINED       = -1
};

static int64_t get_pts(AVFormatContext *s, int64_t pos, int size)
{
    AAXContext *a = s->priv_data;
    int64_t pts = 0;

    for (int seg = 0; seg < a->current_segment; seg++)
        pts += (a->segments[seg].end - a->segments[seg].start) / size;

    pts += ((pos - a->segments[a->current_segment].start) / size);

    return pts;
}

static int aax_read_header(AVFormatContext *s)
{
    AAXContext *a = s->priv_data;
    AVIOContext *pb = s->pb;
    AVCodecParameters *par;
    AVStream *st;
    int64_t column_offset = 0;
    int ret, extradata_size;
    char *codec;

    avio_skip(pb, 4);
    a->table_size      = avio_rb32(pb) + 8LL;
    a->version         = avio_rb16(pb);
    a->rows_offset     = avio_rb16(pb) + 8LL;
    a->strings_offset  = avio_rb32(pb) + 8LL;
    a->data_offset     = avio_rb32(pb) + 8LL;
    a->name_offset     = avio_rb32(pb);
    a->columns         = avio_rb16(pb);
    a->row_width       = avio_rb16(pb);
    a->nb_segments     = avio_rb32(pb);

    if (a->nb_segments < 1)
        return AVERROR_INVALIDDATA;

    a->schema_offset   = 0x20;
    a->strings_size    = a->data_offset - a->strings_offset;

    if (a->rows_offset > a->table_size ||
        a->strings_offset > a->table_size ||
        a->data_offset > a->table_size)
        return AVERROR_INVALIDDATA;
    if (a->strings_size <= 0 || a->name_offset >= a->strings_size ||
        a->strings_size > UINT16_MAX)
        return AVERROR_INVALIDDATA;
    if (a->columns <= 0)
        return AVERROR_INVALIDDATA;

    a->segments = av_calloc(a->nb_segments, sizeof(*a->segments));
    if (!a->segments)
        return AVERROR(ENOMEM);

    a->xcolumns = av_calloc(a->columns, sizeof(*a->xcolumns));
    if (!a->xcolumns) {
        ret = AVERROR(ENOMEM);
        goto fail;
    }

    a->string_table = av_calloc(a->strings_size + 1, sizeof(*a->string_table));
    if (!a->string_table) {
        ret = AVERROR(ENOMEM);
        goto fail;
    }

    for (int c = 0; c < a->columns; c++) {
        uint8_t info = avio_r8(pb);
        uint32_t offset = avio_rb32(pb);
        int value_size;

        if (offset >= a->strings_size) {
            ret = AVERROR_INVALIDDATA;
            goto fail;
        }

        a->xcolumns[c].flag = info >>   4;
        a->xcolumns[c].type = info & 0x0F;

        switch (a->xcolumns[c].type) {
        case COLUMN_TYPE_UINT8:
        case COLUMN_TYPE_SINT8:
            value_size = 0x01;
            break;
        case COLUMN_TYPE_UINT16:
        case COLUMN_TYPE_SINT16:
            value_size = 0x02;
            break;
        case COLUMN_TYPE_UINT32:
        case COLUMN_TYPE_SINT32:
        case COLUMN_TYPE_FLOAT:
        case COLUMN_TYPE_STRING:
            value_size = 0x04;
            break;
        case COLUMN_TYPE_VLDATA:
            value_size = 0x08;
            break;
        case COLUMN_TYPE_UINT128:
            value_size = 0x10;
            break;
        default:
            ret = AVERROR_INVALIDDATA;
            goto fail;
        }

        a->xcolumns[c].size = value_size;

        if (a->xcolumns[c].flag & COLUMN_FLAG_NAME)
            a->xcolumns[c].name = a->string_table + offset;

        if (a->xcolumns[c].flag & COLUMN_FLAG_DEFAULT) {
            /* data is found relative to columns start */
            a->xcolumns[c].offset = avio_tell(pb) - a->schema_offset;
            avio_skip(pb, value_size);
        }

        if (a->xcolumns[c].flag & COLUMN_FLAG_ROW) {
            /* data is found relative to row start */
            a->xcolumns[c].offset = column_offset;
            column_offset += value_size;
        }
    }

    avio_seek(pb, a->strings_offset, SEEK_SET);
    ret = avio_read(pb, a->string_table, a->strings_size);
    if (ret != a->strings_size) {
        if (ret < 0)
            goto fail;
        ret = AVERROR(EIO);
        goto fail;
    }

    for (int c = 0; c < a->columns; c++) {
        int64_t data_offset = 0;
        int64_t col_offset;
        int flag, type;

        if (strcmp(a->xcolumns[c].name, "data"))
            continue;

        type = a->xcolumns[c].type;
        flag = a->xcolumns[c].flag;
        col_offset = a->xcolumns[c].offset;

        for (uint64_t r = 0; r < a->nb_segments; r++) {
            if (flag & COLUMN_FLAG_DEFAULT) {
                data_offset = a->schema_offset + col_offset;
            } else if (flag & COLUMN_FLAG_ROW) {
                data_offset = a->rows_offset + r * a->row_width + col_offset;
            } else {
                ret = AVERROR_INVALIDDATA;
                goto fail;
            }

            avio_seek(pb, data_offset, SEEK_SET);
            if (type == COLUMN_TYPE_VLDATA) {
                int64_t start, size;

                start = avio_rb32(pb);
                size  = avio_rb32(pb);
                a->segments[r].start = start + a->data_offset;
                a->segments[r].end   = a->segments[r].start + size;
            } else {
                ret = AVERROR_INVALIDDATA;
                goto fail;
            }
        }
    }

    st = avformat_new_stream(s, NULL);
    if (!st) {
        ret = AVERROR(ENOMEM);
        goto fail;
    }
    st->start_time = 0;
    par = s->streams[0]->codecpar;
    par->codec_type = AVMEDIA_TYPE_AUDIO;

    codec = a->string_table + a->name_offset;
    if (!strcmp(codec, "AAX")) {
        par->codec_id = AV_CODEC_ID_ADPCM_ADX;
        avio_seek(pb, a->segments[0].start, SEEK_SET);
        if (avio_rb16(pb) != 0x8000) {
            ret = AVERROR_INVALIDDATA;
            goto fail;
        }
        extradata_size = avio_rb16(pb) + 4;
        if (extradata_size < 12) {
            ret = AVERROR_INVALIDDATA;
            goto fail;
        }
        avio_seek(pb, -4, SEEK_CUR);
        ret = ff_get_extradata(s, par, pb, extradata_size);
        if (ret < 0) {
            goto fail;
        }
        par->channels    = AV_RB8 (par->extradata + 7);
        par->sample_rate = AV_RB32(par->extradata + 8);
        if (!par->channels || !par->sample_rate) {
            ret = AVERROR_INVALIDDATA;
            goto fail;
        }

        avpriv_set_pts_info(st, 64, 32, par->sample_rate);
  /*} else if (!strcmp(codec, "HCA") ){
        par->codec_id = AV_CODEC_ID_HCA;*/
    } else {
        ret = AVERROR_INVALIDDATA;
        goto fail;
    }

    return 0;
fail:
    av_freep(&a->string_table);
    av_freep(&a->xcolumns);
    av_freep(&a->segments);

    return ret;
}

static int aax_read_packet(AVFormatContext *s, AVPacket *pkt)
{
    AAXContext *a = s->priv_data;
    AVCodecParameters *par = s->streams[0]->codecpar;
    AVIOContext *pb = s->pb;
    const int size = 18 * par->channels;
    int ret, extradata_size = 0;
    uint8_t *extradata = NULL;
    int skip = 0;

    if (avio_feof(pb))
        return AVERROR_EOF;

    pkt->pos = avio_tell(pb);

    for (uint32_t seg = 0; seg < a->nb_segments; seg++) {
        int64_t start = a->segments[seg].start;
        int64_t end   = a->segments[seg].end;

        if (pkt->pos >= start && pkt->pos <= end) {
            a->current_segment = seg;
            if (par->codec_id == AV_CODEC_ID_ADPCM_ADX)
                skip = (end - start) - ((end - start) / size) * size;
            break;
        }
    }

    if (pkt->pos >= a->segments[a->current_segment].end - skip) {
        if (a->current_segment + 1 == a->nb_segments)
            return AVERROR_EOF;
        a->current_segment++;
        avio_seek(pb, a->segments[a->current_segment].start, SEEK_SET);

        if (par->codec_id == AV_CODEC_ID_ADPCM_ADX) {
            if (avio_rb16(pb) != 0x8000)
                return AVERROR_INVALIDDATA;
            extradata_size = avio_rb16(pb) + 4;
            avio_seek(pb, -4, SEEK_CUR);
            if (extradata_size < 12)
                return AVERROR_INVALIDDATA;
            extradata = av_malloc(extradata_size + AV_INPUT_BUFFER_PADDING_SIZE);
            if (!extradata)
                return AVERROR(ENOMEM);
            if (avio_read(pb, extradata, extradata_size) != extradata_size) {
                av_free(extradata);
                return AVERROR(EIO);
            }
            memset(extradata + extradata_size, 0, AV_INPUT_BUFFER_PADDING_SIZE);
        }
    }

    ret = av_get_packet(pb, pkt, size);
    if (ret != size) {
        av_free(extradata);
        return ret < 0 ? ret : AVERROR(EIO);
    }
    pkt->duration = 1;
    pkt->stream_index = 0;
    pkt->pts = get_pts(s, pkt->pos, size);

    if (extradata) {
        ret = av_packet_add_side_data(pkt, AV_PKT_DATA_NEW_EXTRADATA, extradata, extradata_size);
        if (ret < 0) {
            av_free(extradata);
            return ret;
        }
    }

    return ret;
}

static int aax_read_close(AVFormatContext *s)
{
    AAXContext *a = s->priv_data;

    av_freep(&a->segments);
    av_freep(&a->xcolumns);
    av_freep(&a->string_table);

    return 0;
}

AVInputFormat ff_aax_demuxer = {
    .name           = "aax",
    .long_name      = NULL_IF_CONFIG_SMALL("CRI AAX"),
    .priv_data_size = sizeof(AAXContext),
    .read_probe     = aax_probe,
    .read_header    = aax_read_header,
    .read_packet    = aax_read_packet,
    .read_close     = aax_read_close,
    .extensions     = "aax",
    .flags          = AVFMT_GENERIC_INDEX,
};

AVInputFormat ff_aac_demuxer = {
    .name         = "aac",
    .long_name    = NULL_IF_CONFIG_SMALL("raw ADTS AAC (Advanced Audio Coding)"),
    .read_probe   = adts_aac_probe,
    .read_header  = adts_aac_read_header,
    .read_packet  = adts_aac_read_packet,
    .flags        = AVFMT_GENERIC_INDEX,
    .extensions   = "aac",
    .mime_type    = "audio/aac,audio/aacp,audio/x-aac",
    .raw_codec_id = AV_CODEC_ID_AAC,
};

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
