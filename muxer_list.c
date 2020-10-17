#include "ffplay.h"

static int a64_write_header(AVFormatContext *s)
{
    AVCodecParameters *par = s->streams[0]->codecpar;
    uint8_t header[5] = {
        0x00, //load
        0x40, //address
        0x00, //mode
        0x00, //charset_lifetime (multi only)
        0x00  //fps in 50/fps;
    };

    if (par->extradata_size < 4) {
        av_log(s, AV_LOG_ERROR, "Missing extradata\n");
        return AVERROR_INVALIDDATA;
    }

    switch (par->codec_id) {
    case AV_CODEC_ID_A64_MULTI:
        header[2] = 0x00;
        header[3] = AV_RB32(par->extradata+0);
        header[4] = 2;
        break;
    case AV_CODEC_ID_A64_MULTI5:
        header[2] = 0x01;
        header[3] = AV_RB32(par->extradata+0);
        header[4] = 3;
        break;
    default:
        return AVERROR_INVALIDDATA;
    }
    avio_write(s->pb, header, 2);
    return 0;
}

int ff_raw_write_packet(AVFormatContext *s, AVPacket *pkt)
{
    avio_write(s->pb, pkt->data, pkt->size);
    return 0;
}

AVOutputFormat ff_a64_muxer = {
    .name           = "a64",
    .long_name      = NULL_IF_CONFIG_SMALL("a64 - video for Commodore 64"),
    .extensions     = "a64, A64",
    .video_codec    = AV_CODEC_ID_A64_MULTI,
    .write_header   = a64_write_header,
    .write_packet   = ff_raw_write_packet,
};

static int force_one_stream(AVFormatContext *s)
{
    if (s->nb_streams != 1) {
        av_log(s, AV_LOG_ERROR, "%s files have exactly one stream\n",
               s->oformat->name);
        return AVERROR(EINVAL);
    }
    if (   s->oformat->audio_codec != AV_CODEC_ID_NONE
        && s->streams[0]->codecpar->codec_type != AVMEDIA_TYPE_AUDIO) {
        av_log(s, AV_LOG_ERROR, "%s files have exactly one audio stream\n",
               s->oformat->name);
        return AVERROR(EINVAL);
    }
    if (   s->oformat->video_codec != AV_CODEC_ID_NONE
        && s->streams[0]->codecpar->codec_type != AVMEDIA_TYPE_VIDEO) {
        av_log(s, AV_LOG_ERROR, "%s files have exactly one video stream\n",
               s->oformat->name);
        return AVERROR(EINVAL);
    }
    return 0;
}


#if CONFIG_AC3_MUXER
AVOutputFormat ff_ac3_muxer = {
    .name              = "ac3",
    .long_name         = NULL_IF_CONFIG_SMALL("raw AC-3"),
    .mime_type         = "audio/x-ac3",
    .extensions        = "ac3",
    .audio_codec       = AV_CODEC_ID_AC3,
    .video_codec       = AV_CODEC_ID_NONE,
    .write_header      = force_one_stream,
    .write_packet      = ff_raw_write_packet,
    .flags             = AVFMT_NOTIMESTAMPS,
};
#endif
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
