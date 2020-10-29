import sequtils
var s = @["ff_yuv420p_to_argb_neon","ff_yuv420p_to_rgba_neon","ff_yuv420p_to_abgr_neon","ff_yuv420p_to_bgra_neon","ff_yuv422p_to_argb_neon","ff_yuv422p_to_rgba_neon","ff_yuv422p_to_abgr_neon","ff_yuv422p_to_bgra_neon","ff_nv12_to_argb_neon","ff_nv12_to_rgba_neon","ff_nv12_to_abgr_neon","ff_nv12_to_bgra_neon","ff_nv21_to_argb_neon","ff_nv21_to_rgba_neon","ff_nv21_to_abgr_neon","ff_nv21_to_bgra_neon","ff_yuv_420_rgb15_mmx","ff_yuv_420_rgb16_mmx","ff_yuv_420_rgb32_mmx","ff_yuv_420_bgr32_mmx","ff_yuva_420_rgb32_mmx","ff_yuva_420_bgr32_mmx","ff_yuv_420_rgb24_mmx","ff_yuv_420_bgr24_mmx","ff_yuv_420_rgb24_mmxext","ff_yuv_420_bgr24_mmxext","ff_yuv_420_rgb15_ssse3","ff_yuv_420_rgb16_ssse3","ff_yuv_420_rgb32_ssse3","ff_yuv_420_bgr32_ssse3","ff_yuva_420_rgb32_ssse3","ff_yuva_420_bgr32_ssse3","ff_yuv_420_rgb24_ssse3","ff_yuv_420_bgr24_ssse3","ff_cpu_cpuid","ff_cpu_cpuid","ff_cpu_xgetbv","ff_cpu_cpuid","ff_cpu_cpuid","ff_cpu_cpuid","libiconv_open","libiconv","libiconv","libiconv_close","BCryptOpenAlgorithmProvider","BCryptGenRandom","BCryptCloseAlgorithmProvider","ff_w1111","ff_bgr2YOffset","ff_w1111","ff_w1111","ff_bgr2UVOffset","ff_w1111","ff_bgr2YOffset","ff_w1111","ff_bgr2UVOffset","ff_nv12ToUV_sse2","ff_nv12ToUV_avx","ff_w1111","ff_bgr2YOffset","ff_w1111","ff_bgr2UVOffset","__imp_WSACleanup","av_buffersink_get_time_base","av_buffersink_get_frame_rate","av_buffersink_get_time_base","soxr_io_spec","soxr_quality_spec","soxr_delete","soxr_create","soxr_delete","soxr_delay","soxr_process","soxr_process","soxr_delay","soxr_set_num_channels","soxr_set_error","soxr_process","soxr_delay","soxr_delay","swri_noise_shaping_int16","swri_noise_shaping_int32","swri_noise_shaping_float","swri_noise_shaping_double","libiconv_open","libiconv_close","av_buffersink_get_sample_rate","av_buffersink_get_channels","av_buffersink_get_channel_layout","__imp_WSAStartup","libAVPin_Release","libAVPin_Release","libAVFilter_Release","libAVFilter_Release","__imp_CoUninitialize","__imp_CoTaskMemFree","__imp_CoTaskMemFree","__imp_CoTaskMemFree","__imp_CoTaskMemFree","__imp_OleCreatePropertyFrame","__imp_CoTaskMemFree","__imp_CoTaskMemFree","libAVFilter_Create","libAVPin_AddRef","__imp_CoCreateInstance","libAVPin_ConnectionMediaType","__imp_CoTaskMemFree","__imp_CoInitialize","__imp_CoCreateInstance","__imp_CoCreateInstance","uncompress","av_fourcc_make_string","av_opt_eval_flags","av_opt_eval_flags","IID_IMediaEvent","IID_IMediaControl","CLSID_SystemDeviceEnum","IID_ICreateDevEnum","CLSID_FilterGraph","IID_IGraphBuilder","CLSID_CaptureGraphBuilder2","IID_ICaptureGraphBuilder2","IID_IAMTVAudio","IID_IAMTVTuner","IID_IBaseFilter","LOOK_UPSTREAM_ONLY","IID_IAMCrossbar","PIN_CATEGORY_CAPTURE","AMPROPSETID_Pin","IID_IKsPropertySet","MEDIATYPE_Audio","MEDIATYPE_Video","IID_IUnknown","IID_ISpecifyPropertyPages","IID_IAMBufferNegotiation","FORMAT_WaveFormatEx","FORMAT_VideoInfo2","FORMAT_VideoInfo","IID_IAMStreamConfig","CLSID_AudioInputDeviceCategory","CLSID_VideoInputDeviceCategory","codec_list","ff_float_to_int32_a_avx2","ff_pack_8ch_float_to_int32_a_avx","ff_pack_8ch_int32_to_float_a_avx","ff_pack_8ch_float_to_float_a_avx","ff_unpack_6ch_float_to_int32_a_avx","ff_unpack_6ch_int32_to_float_a_avx","ff_unpack_6ch_float_to_float_a_avx","ff_pack_6ch_float_to_int32_a_avx","ff_pack_6ch_int32_to_float_a_avx","ff_pack_6ch_float_to_float_a_avx","ff_int32_to_float_a_avx","ff_unpack_2ch_int16_to_float_a_ssse3","ff_unpack_2ch_int16_to_int32_a_ssse3","ff_unpack_2ch_int16_to_int16_a_ssse3","ff_pack_8ch_float_to_int32_a_sse2","ff_pack_8ch_int32_to_float_a_sse2","ff_pack_8ch_float_to_float_a_sse2","ff_unpack_6ch_float_to_int32_a_sse2","ff_unpack_6ch_int32_to_float_a_sse2","ff_pack_6ch_float_to_int32_a_sse2","ff_pack_6ch_int32_to_float_a_sse2","ff_unpack_2ch_float_to_int16_a_sse2","ff_unpack_2ch_int16_to_float_a_sse2","ff_unpack_2ch_float_to_int32_a_sse2","ff_unpack_2ch_int32_to_float_a_sse2","ff_pack_2ch_float_to_int16_a_sse2","ff_pack_2ch_int16_to_float_a_sse2","ff_pack_2ch_float_to_int32_a_sse2","ff_pack_2ch_int32_to_float_a_sse2","ff_unpack_2ch_int32_to_int16_a_sse2","ff_unpack_2ch_int16_to_int32_a_sse2","ff_unpack_2ch_int16_to_int16_a_sse2","ff_unpack_2ch_int32_to_int32_a_sse2","ff_pack_2ch_int32_to_int16_a_sse2","ff_pack_2ch_int16_to_int32_a_sse2","ff_pack_2ch_int16_to_int16_a_sse2","ff_pack_2ch_int32_to_int32_a_sse2","ff_float_to_int16_a_sse2","ff_float_to_int32_a_sse2","ff_int16_to_float_a_sse2","ff_int32_to_float_a_sse2","ff_unpack_6ch_float_to_float_a_sse","ff_pack_6ch_float_to_float_a_sse","ff_pack_6ch_float_to_float_a_mmx","ff_int32_to_int16_a_sse2","ff_int16_to_int32_a_sse2","ff_int32_to_int16_a_mmx","ff_int16_to_int32_a_mmx","ff_resample_common_double_fma3","ff_resample_linear_double_fma3","ff_resample_common_double_avx","ff_resample_linear_double_avx","ff_resample_common_double_sse2","ff_resample_linear_double_sse2","ff_resample_common_float_fma4","ff_resample_linear_float_fma4","ff_resample_common_float_fma3","ff_resample_linear_float_fma3","ff_resample_common_float_avx","ff_resample_linear_float_avx","ff_resample_common_float_sse","ff_resample_linear_float_sse","ff_resample_common_int16_xop","ff_resample_linear_int16_xop","ff_resample_common_int16_sse2","ff_resample_linear_int16_sse2","ff_fft_calc_avx","ff_imdct_half_avx","ff_fft_calc_sse","ff_fft_permute_sse","ff_imdct_half_sse","ff_imdct_calc_sse","ff_yuv2nv21cX_avx2","ff_yuv2nv12cX_avx2","rgb16tobgr32","rgb15tobgr32","rgb16tobgr16","rgb15tobgr16","rgb16tobgr15","rgb15tobgr15","rgb12tobgr12","rgb24to32","rgb32to24","rgb16to24","rgb15to24","rgb12to15","rgb64to48_bswap","rgb64to48_nobswap","rgb64tobgr48_bswap","rgb64tobgr48_nobswap","rgb48to64_bswap","rgb48to64_nobswap","rgb48tobgr64_bswap","rgb48tobgr64_nobswap","rgb48tobgr48_bswap","rgb48tobgr48_nobswap","ff_uyvytoyuv422_avx","ff_shuffle_bytes_3210_ssse3","ff_shuffle_bytes_3012_ssse3","ff_shuffle_bytes_1230_ssse3","ff_shuffle_bytes_2103_ssse3","ff_shuffle_bytes_0321_ssse3","ff_uyvytoyuv422_sse2","ff_shuffle_bytes_2103_mmxext"]
s = s.deduplicate
echo s.len
for i in s:
    echo i
