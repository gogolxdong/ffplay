import posix,winim, sdl2
import tables, parseopt,sequtils,strutils
import x264
import macros
import strscans
import common

when defined(windows):
  import winim

type
  AVCodecTag* = ref object
    id*: AVCodecID
    tag*: cuint
  AVRational*  = tuple
    num: int                 
    den: int
  AVOptionUnion* {.union.} = object 
    i64*: int64
    dbl*: cdouble
    str*: string             
    q*: AVRational
  AVOption* = ref object
    name*: string
    help*: string
    offset*: cint
    t*: AVOptionType
    default_val*: AVOptionUnion
    min*: cdouble              
    max*: cdouble           
    flags*: cint
    unit*: string
  AVOptionRange* = ref object
    str*: string
    value_min*: cdouble
    value_max*: cdouble
    component_min*: cdouble
    component_max*: cdouble
    is_range*: cint
  AVOptionRanges* = ref object
    range*: ref AVOptionRange
    nb_ranges*: cint
    nb_components*: cint
  AVClass* = ref object
    class_name*: string
    item_name*: proc (ctx: pointer): string
    option*: ptr AVOption
    version*: cint
    log_level_offset_offset*: cint
    parent_log_context_offset*: cint
    child_next*: proc (obj: pointer; prev: pointer): pointer
    child_class_next*: proc (prev: AVClass): AVClass
    category*: AVClassCategory
    get_category*: proc (ctx: pointer): AVClassCategory
    query_ranges*: proc (a1: ptr ptr AVOptionRanges; obj: pointer; key: string;flags: cint): cint
    child_class_iterate*: proc (iter: ptr pointer): AVClass
  AVProbeData* = ref object
    filename*: string
    buf*: ptr cuchar            
    buf_size*: cint            
    mime_type*: string        
  AVIOContext* = ref object
    av_class*: AVClass
    buffer*: ptr cuchar         
    buffer_size*: cint         
    buf_ptr*: ptr cuchar        
    buf_end*: ptr cuchar 
    opaque*: pointer                            
    read_packet*: proc (opaque: pointer; buf: uint8; buf_size: cint): cint
    write_packet*: proc (opaque: pointer; buf: uint8; buf_size: cint): cint
    seek*: proc (opaque: pointer; offset: int64; whence: cint): int64
    pos*: int64              
    eof_reached*: cint         
    write_flag*: cint          
    max_packet_size*: cint
    checksum*: culong
    checksum_ptr*: ptr cuchar
    update_checksum*: proc (checksum: culong; buf: uint8; size: cuint): culong
    error*: cint               
    read_pause*: proc (opaque: pointer; pause: cint): cint
    read_seek*: proc (opaque: pointer; stream_index: cint; timestamp: int64;flags: cint): int64
    seekable*: cint
    maxsize*: int64
    direct*: cint
    bytes_read*: int64
    seek_count*: cint
    writeout_count*: cint
    orig_buffer_size*: cint
    short_seek_threshold*: cint
    protocol_whitelist*: string
    protocol_blacklist*: string
    write_data_type*: proc (opaque: pointer; buf: uint8; buf_size: cint;t: AVIODataMarkerType; time: int64): cint
    ignore_boundary_point*: cint
    current_type*: AVIODataMarkerType
    last_time*: int64
    short_seek_get*: proc (opaque: pointer): cint
    written*: int64
    buf_ptr_max*: ptr cuchar
    min_packet_size*: cint
  AVBufferRef = ref object
  AVBuffer* = object
    buffer*: ptr AVBuffer
    data*: uint8
    size*: cint
  AVPacketSideData* = ref object
    data*: uint8
    size*: cint
    t*: AVPacketSideDataType
  AVPacket* = ref object
    buf*: AVBufferRef
    pts*: int64 
    dts*: int64
    data*: uint8
    size*: cint
    stream_index*: cint
    flags*: cint
    side_data*: AVPacketSideData
    side_data_elems*: cint
    duration*: int64
    pos*: int64   
  AVCodecParameters* = ref object
    codec_type*: AVMediaType
    codec_id*: AVCodecID
    codec_tag*: uint32
    extradata*: uint8
    extradata_size*: cint
    format*: cint
    bit_rate*: int64
    bits_per_coded_sample*: cint
    bits_per_raw_sample*: cint
    profile*: cint
    level*: cint
    width*: cint
    height*: cint
    sample_aspect_ratio*: AVRational
    field_order*: AVFieldOrder
    color_range*: AVColorRange
    color_primaries*: AVColorPrimaries
    color_trc*: AVColorTransferCharacteristic
    color_space*: AVColorSpace
    chroma_location*: AVChromaLocation
    video_delay*: cint
    channel_layout*: uint64
    channels*: cint
    sample_rate*: cint
    block_align*: cint
    frame_size*: cint
    initial_padding*: cint
    trailing_padding*: cint
    seek_preroll*: cint

  AVProfile* = ref object
    profile*: cint
    name*: string             
  AVCodecDefault* = ref object
    key*: uint8
    value*: uint8
  AVSubtitleRect* = ref object
    x*: cint                   
    y*: cint                   
    w*: cint                   
    h*: cint                   
    nb_colors*: cint           
    data*: array[4, uint8]
    linesize*: array[4, cint]
    t*: AVSubtitleType
    text*: string             
    ass*: string
    flags*: cint
  AVSubtitle* = ref object
    format*: uint16         
    start_display_time*: uint32 
    end_display_time*: uint32 
    num_rects*: cuint
    rects*: AVSubtitleRect
    pts*: int64              
  AVFrameSideData* = ref object
    t*: AVFrameSideDataType
    data*: uint8
    size*: cint
    metadata*: Table[string,string]
    buf*: AVBufferRef
  AVFrame* = ref object
    data*: array[8, uint8]
    linesize*: array[8, cint]
    extended_data*: uint8
    width*: cint
    height*: cint
    nb_samples*: cint
    format*: cint
    key_frame*: cint
    pict_type*: AVPictureType
    sample_aspect_ratio*: AVRational
    pts*: int64
    pkt_dts*: int64
    coded_picture_number*: cint
    display_picture_number*: cint
    quality*: cint
    opaque*: pointer
    repeat_pict*: cint
    interlaced_frame*: cint
    top_field_first*: cint
    palette_has_changed*: cint
    reordered_opaque*: int64
    sample_rate*: cint
    channel_layout*: uint64
    buf*: array[8, AVBufferRef]
    extended_buf*: ptr AVBufferRef
    nb_extended_buf*: cint
    side_data*: ptr AVFrameSideData
    nb_side_data*: cint
    flags*: cint
    color_range*: AVColorRange
    color_primaries*: AVColorPrimaries
    color_trc*: AVColorTransferCharacteristic
    colorspace*: AVColorSpace
    chroma_location*: AVChromaLocation
    best_effort_timestamp*: int64
    pkt_pos*: int64
    pkt_duration*: int64
    metadata*: Table[string,string]
    decode_error_flags*: cint
    channels*: cint
    pkt_size*: cint
    hw_frames_ctx*: AVBufferRef
    opaque_ref*: AVBufferRef
    crop_top*: csize_t
    crop_bottom*: csize_t
    crop_left*: csize_t
    crop_right*: csize_t
    private_ref*: AVBufferRef
  ScanTable* = ref object
    scantable*: uint8
    permutated*: array[64, uint8]
    raster_end*: array[64, uint8]

type
  ThreadFrame* = ref object
    f*: AVFrame
    owner*: array[2, AVCodecContext]
    progress*: AVBufferRef

  Picture* = ref object
    f*: AVFrame
    tf*: ThreadFrame
    qscale_table_buf*: AVBufferRef
    qscale_table*: int8
    motion_val_buf*: array[2, AVBufferRef] 
    mb_type_buf*: AVBufferRef
    mb_type*: ptr uint32      
    mbskip_table_buf*: AVBufferRef
    mbskip_table*: uint8
    ref_index_buf*: array[2, AVBufferRef]
    ref_index*: array[2, int8]
    mb_var_buf*: AVBufferRef
    mb_var*: ptr uint16       
    mc_mb_var_buf*: AVBufferRef
    mc_mb_var*: ptr uint16    
    alloc_mb_width*: cint      
    alloc_mb_height*: cint     
    mb_mean_buf*: AVBufferRef
    mb_mean*: uint8       
    hwaccel_priv_buf*: AVBufferRef
    hwaccel_picture_private*: pointer 
    field_picture*: cint       
    mb_var_sum*: int64       
    mc_mb_var_sum*: int64    
    b_frame_score*: cint
    needs_realloc*: cint       
    reference*: cint
    shared*: cint
    encoding_error*: array[AV_NUM_DATA_POINTERS, uint64]
  PutBitContext* = ref object
    bit_buf*: uint64
    bit_left*: cint
    buf*: int8
    buf_ptr*: int8
    buf_end*: int8
    size_in_bits*: cint
  ScratchpadContext* = ref object
    edge_emu_buffer*: uint8 
    rd_scratchpad*: uint8 
    obmc_scratchpad*: uint8
    b_scratchpad*: uint8  
  op_fill_func* = proc (b: uint8; value: uint8; line_size: int64; h: cint)
  BlockDSPContext* = ref object
    clear_block*: proc (b: int16) 
    clear_blocks*: proc (blocks: int16) 
    fill_block_tab*: array[2, op_fill_func]
  FDCTDSPContext* = ref object
    fdct*: proc (b: int16)
    fdct248*: proc (b: int16)
  h264_chroma_mc_func* = proc (dst: uint8; src: uint8; srcStride: int64; h: cint; x: cint; y: cint)
  H264ChromaContext* = ref object
    put_h264_chroma_pixels_tab*: array[4, h264_chroma_mc_func]
    avg_h264_chroma_pixels_tab*: array[4, h264_chroma_mc_func]
  op_pixels_func* = proc (b: uint8; pixels: uint8; line_size: int64; h: cint)
  HpelDSPContext* = ref object
    put_pixels_tab*: array[4, array[4, op_pixels_func]]
    avg_pixels_tab*: array[4, array[4, op_pixels_func]]
    put_no_rnd_pixels_tab*: array[4, array[4, op_pixels_func]]
    avg_no_rnd_pixels_tab*: array[4, op_pixels_func]
  IDCTDSPContext* = ref object
    put_pixels_clamped*: proc (b: int16; pixels: uint8; line_size: int64)
    put_signed_pixels_clamped*: proc (b: int16; pixels: uint8;line_size: int64)
    add_pixels_clamped*: proc (b: int16; pixels: uint8; line_size: int64)
    idct*: proc (b: int16)
    idct_put*: proc (dest: uint8; line_size: int64; b: int16)
    idct_add*: proc (dest: uint8; line_size: int64; b: int16)
    idct_permutation*: array[64, uint8]
    perm_type*: idct_permutation_type
    mpeg4_studio_profile*: cint
  MotionEstContext*  = ref object
    avctx*: AVCodecContext
    skip*: cint                ## /< set if ME is skipped for the current MB
    co_located_mv*: array[4, array[2, cint]] ## /< mv from last P-frame for direct mode ME
    direct_basis_mv*: array[4, array[2, cint]]
    scratchpad*: uint8    ## *< data area for the ME algo, so that
                          ##  the ME does not need to malloc/free.
    best_mb*: uint8
    temp_mb*: array[2, uint8]
    temp*: uint8
    best_bits*: cint
    map*: ptr uint32          ## /< map to avoid duplicate evaluations
    score_map*: ptr uint32    ## /< map to store the scores
    map_generation*: cuint
    pre_penalty_factor*: cint
    penalty_factor*: cint      ## *< an estimate of the bits required to
                        ##  code a given mv value, e.g. (1,0) takes
                        ##  more bits than (0,0). We have to
                        ##  estimate whether any reduction in
                        ##  residual is worth the extra bits.
    sub_penalty_factor*: cint
    mb_penalty_factor*: cint
    flags*: cint
    sub_flags*: cint
    mb_flags*: cint
    pre_pass*: cint            ## /< = 1 for the pre pass
    dia_size*: cint
    xmin*: cint
    xmax*: cint
    ymin*: cint
    ymax*: cint
    pred_x*: cint
    pred_y*: cint
    src*: array[4, array[4, uint8]]
    `ref`*: array[4, array[4, uint8]]
    stride*: cint
    uvstride*: cint            ##  temp variables for picture complexity calculation
    mc_mb_var_sum_temp*: int64
    mb_var_sum_temp*: int64
    scene_change_score*: cint
    hpel_put*: array[4, op_pixels_func]
    hpel_avg*: array[4, op_pixels_func]
    qpel_put*: array[16, qpel_mc_func]
    qpel_avg*: array[16, qpel_mc_func]
    mv_penalty*: array[MAX_DMV * 2 + 1, uint8] ## /< bit amount needed to encode a MV
    current_mv_penalty*: uint8
    sub_motion_search*: proc (s: MpegEncContext; mx_ptr: cint; my_ptr: cint;dmin: cint; src_index: cint; ref_index: cint; size: cint;h: cint): cint
  Predictor*  = ref  object
    coeff*: cdouble
    count*: cdouble
    decay*: cdouble
  RateControlEntry*  = ref object
    pict_type*: cint
    qscale*: cfloat
    mv_bits*: cint
    i_tex_bits*: cint
    p_tex_bits*: cint
    misc_bits*: cint
    header_bits*: cint
    expected_bits*: uint64
    new_pict_type*: cint
    new_qscale*: cfloat
    mc_mb_var_sum*: int64
    mb_var_sum*: int64
    i_count*: cint
    skip_count*: cint
    f_code*: cint
    b_code*: cint
  AVExprUnion* {.union.} = ref object 
    func0*: proc (a1: cdouble): cdouble
    func1*: proc (a1: pointer; a2: cdouble): cdouble
    func2*: proc (a1: pointer; a2: cdouble; a3: cdouble): cdouble
  AVExpr* = ref object
    `type`*: AVExprType
    value*: cdouble            ##  is sign in other types
    const_index*: cint
    a*: AVExprUnion
    param*: array[3, ptr AVExpr]
    `var`*: cdouble
  RateControlContext* = ref object
    num_entries*: cint         ## /< number of RateControlEntries
    entry*: RateControlEntry
    buffer_index*: cdouble     ## /< amount of bits in the video/audio buffer
    pred*: array[5, Predictor]
    short_term_qsum*: cdouble  ## /< sum of recent qscales
    short_term_qcount*: cdouble ## /< count of recent qscales
    pass1_rc_eq_output_sum*: cdouble ## /< sum of the output of the rc equation, this is used for normalization
    pass1_wanted_bits*: cdouble ## /< bits which should have been output by the pass1 code (including complexity init)
    last_qscale*: cdouble
    last_qscale_for*: array[5, cdouble] ## /< last qscale for a specific pict , used for max_diff & ipb factor stuff
    last_mc_mb_var_sum*: int64
    last_mb_var_sum*: int64
    i_cplx_sum*: array[5, uint64]
    p_cplx_sum*: array[5, uint64]
    mv_bits_sum*: array[5, uint64]
    qscale_sum*: array[5, uint64]
    frame_count*: array[5, cint]
    last_non_b_pict_type*: cint
    non_lavc_opaque*: pointer  ## /< context for non lavc rc code (for example xvid)
    dry_run_qscale*: cfloat    ## /< for xvid rc
    last_picture_number*: cint ## /< for xvid rc
    rc_eq_eval*: AVExpr
  GetBitContext*  = ref object
    buffer*: uint8
    buffer_end*: uint8
    index*: cint
    size_in_bits*: cint
    size_in_bits_plus8*: cint
  ParseContext* = ref object
    buffer*: uint8
    index*: cint
    last_index*: cint
    buffer_size*: cuint
    state*: uint32           ## /< contains the last few bytes in MSB order
    frame_start_found*: cint
    overread*: cint            ## /< the number of bytes which where irreversibly read from the next frame
    overread_index*: cint      ## /< the index into ParseContext.buffer of the overread bytes
    state64*: uint64         ## /< contains the last 8 bytes in MSB order
  MJpegHuffmanCode* = ref object
    table_id*: uint8         ##  0=DC lum, 1=DC chrom, 2=AC lum, 3=AC chrom
    ## /< The Huffman table id associated with the data.
    code*: uint8             ## /< The exponent.
    mant*: uint16            ## /< The mantissa.
  MJpegContext*  = ref object
    huff_size_dc_luminance*: array[12, uint8] ## FIXME use array [3] instead of lumi / chroma, for easier addressing
    ## /< DC luminance Huffman table size.
    huff_code_dc_luminance*: array[12, uint16] ## /< DC luminance Huffman table codes.
    huff_size_dc_chrominance*: array[12, uint8] ## /< DC chrominance Huffman table size.
    huff_code_dc_chrominance*: array[12, uint16] ## /< DC chrominance Huffman table codes.
    huff_size_ac_luminance*: array[256, uint8] ## /< AC luminance Huffman table size.
    huff_code_ac_luminance*: array[256, uint16] ## /< AC luminance Huffman table codes.
    huff_size_ac_chrominance*: array[256, uint8] ## /< AC chrominance Huffman table size.
    huff_code_ac_chrominance*: array[256, uint16] ## /< AC chrominance Huffman table codes.
                                                 ## * Storage for AC luminance VLC (in MpegEncContext)
    uni_ac_vlc_len*: array[64 * 64 * 2, uint8] ## * Storage for AC chrominance VLC (in MpegEncContext)
    uni_chroma_ac_vlc_len*: array[64 * 64 * 2, uint8] ##  Default DC tables have exactly 12 values
    bits_dc_luminance*: array[17, uint8] ## /< DC luminance Huffman bits.
    val_dc_luminance*: array[12, uint8] ## /< DC luminance Huffman values.
    bits_dc_chrominance*: array[17, uint8] ## /< DC chrominance Huffman bits.
    val_dc_chrominance*: array[12, uint8] ## /< DC chrominance Huffman values.
                                         ##  8-bit JPEG has max 256 values
    bits_ac_luminance*: array[17, uint8] ## /< AC luminance Huffman bits.
    val_ac_luminance*: array[256, uint8] ## /< AC luminance Huffman values.
    bits_ac_chrominance*: array[17, uint8] ## /< AC chrominance Huffman bits.
    val_ac_chrominance*: array[256, uint8] ## /< AC chrominance Huffman values.
    huff_ncode*: csize_t       ## /< Number of current entries in the buffer.
    huff_buffer*: MJpegHuffmanCode ## /< Buffer for Huffman code values.

  MpegEncContext* = ref object
    class*: AVClass
    y_dc_scale*: cint
    c_dc_scale*: cint
    ac_pred*: cint
    block_last_index*: array[12, cint] ## /< last non zero coefficient in block
    h263_aic*: cint            ## /< Advanced INTRA Coding (AIC)
    inter_scantable*: ScanTable ## /< if inter == intra then intra should be used to reduce the cache usage
    intra_scantable*: ScanTable
    intra_h_scantable*: ScanTable
    intra_v_scantable*: ScanTable
    avctx*: AVCodecContext
    width*: cint
    height*: cint              ## /< picture size. must be a multiple of 16
    gop_size*: cint
    intra_only*: cint          ## /< if true, only intra pictures are generated
    bit_rate*: int64         ## /< wanted bit rate
    out_format*: OutputFormat  ## /< output format
    h263_pred*: cint           ## /< use MPEG-4/H.263 ac/dc predictions
    pb_frame*: cint            ## /< PB-frame mode (0 = none, 1 = base, 2 = improved)
    h263_plus*: cint           ## /< H.263+ headers
    h263_flv*: cint            ## /< use flv H.263 header
    codec_id*: AVCodecID       ##  see AV_CODEC_ID_xxx
    fixed_qscale*: cint        ## /< fixed qscale if non zero
    encoding*: cint            ## /< true if we are encoding (vs decoding)
    max_b_frames*: cint        ## /< max number of B-frames for encoding
    luma_elim_threshold*: cint
    chroma_elim_threshold*: cint
    strict_std_compliance*: cint ## /< strictly follow the std (MPEG-4, ...)
    workaround_bugs*: cint     ## /< workaround bugs in encoders which cannot be detected automatically
    codec_tag*: cint           ## /< internal codec_tag upper case converted from avctx codec_tag
    context_initialized*: cint
    input_picture_number*: cint ## /< used to set pic->display_picture_number, should not be used for/by anything else
    coded_picture_number*: cint ## /< used to set pic->coded_picture_number, should not be used for/by anything else
    picture_number*: cint      ## FIXME remove, unclear definition
    picture_in_gop_number*: cint ## /< 0-> first pic in gop, ...
    mb_width*: cint
    mb_height*: cint           ## /< number of MBs horizontally & vertically
    mb_stride*: cint           ## /< mb_width+1 used for some arrays to allow simple addressing of left & top MBs without sig11
    b8_stride*: cint           ## /< 2*mb_width+1 used for some 8x8 block arrays to allow simple addressing
    h_edge_pos*: cint
    v_edge_pos*: cint          ## /< horizontal / vertical position of the right/bottom edge (pixel replication)
    mb_num*: cint              ## /< number of MBs of a picture
    linesize*: uint64       ## /< line size, in bytes, may be different from width
    uvlinesize*: uint64     ## /< line size, for chroma in bytes, may be different from width
    picture*: Picture       ## /< main picture buffer
    input_picture*: seq[Picture] ## /< next pictures on display order for encoding
    reordered_input_picture*: seq[Picture] ## /< pointer to the next pictures in coded order for encoding
    user_specified_pts*: int64 ## /< last non-zero pts from AVFrame which was passed into avcodec_encode_video2()
    dts_delta*: int64
    reordered_pts*: int64
    pb*: PutBitContext
    start_mb_y*: cint          ## /< start mb_y of this thread (so current thread should process start_mb_y <= row < end_mb_y)
    end_mb_y*: cint            ## /< end   mb_y of this thread (so current thread should process start_mb_y <= row < end_mb_y)
    thread_context*: array[MAX_THREADS, ptr MpegEncContext]
    slice_context_count*: cint ## /< number of used thread_contexts
    last_picture*: Picture
    next_picture*: Picture
    new_picture*: Picture
    current_picture*: Picture  ## /< buffer to store the decompressed current picture
    last_picture_ptr*: Picture ## /< pointer to the previous picture.
    next_picture_ptr*: Picture ## /< pointer to the next picture (for bidir pred)
    current_picture_ptr*: Picture ## /< pointer to the current picture
    last_dc*: array[3, cint]    ## /< last DC values for MPEG-1
    dc_val_base*: int16
    dc_val*: array[3, int16] ## /< used for MPEG-4 DC prediction, all 3 arrays must be continuous
    y_dc_scale_table*: uint8 ## /< qscale -> y_dc_scale table
    c_dc_scale_table*: uint8 ## /< qscale -> c_dc_scale table
    chroma_qscale_table*: uint8 ## /< qscale -> chroma_qscale (H.263)
    coded_block_base*: uint8
    coded_block*: uint8   ## /< used for coded block pattern prediction (msmpeg4v3, wmv1)
    ac_val_base*: array[16, int16] ##  int16 (*ac_val[3])[16];      ///< used for MPEG-4 AC prediction, all 3 arrays must be continuous
    mb_skipped*: cint          ## /< MUST BE SET only during DECODING
    mbskip_table*: uint8  ## *< used to avoid copy if macroblock skipped (for black regions for example) and used for B-frame encoding & decoding (contains skip table of next P-frame)
    mbintra_table*: uint8 ## /< used to avoid setting {ac, dc, cbp}-pred stuff to zero on inter MB decoding
    cbp_table*: uint8     ## /< used to store cbp, ac_pred for partitioned decoding
    pred_dir_table*: uint8 ## /< used to store pred_dir for partitioned decoding
    sc*: ScratchpadContext
    qscale*: cint              ## /< QP
    chroma_qscale*: cint       ## /< chroma QP
    lambda*: cuint             ## /< Lagrange multiplier used in rate distortion
    lambda2*: cuint            ## /< (lambda*lambda) >> FF_LAMBDA_SHIFT
    lambda_table*: cint
    adaptive_quant*: cint      ## /< use adaptive quantization
    dquant*: cint              ## /< qscale difference to prev qscale
    closed_gop*: cint          ## /< MPEG1/2 GOP is closed
    pict_type*: cint           ## /< AV_PICTURE_TYPE_I, AV_PICTURE_TYPE_P, AV_PICTURE_TYPE_B, ...
    vbv_delay*: cint
    last_pict_type*: cint      ## FIXME removes
    last_non_b_pict_type*: cint ## /< used for MPEG-4 gmc B-frames & ratecontrol
    droppable*: cint
    frame_rate_index*: cint
    mpeg2_frame_rate_ext*: AVRational
    last_lambda_for*: array[5, cint] ## /< last lambda for a specific pict 
    skipdct*: cint             ## /< skip dct and code zero residual motion compensation
    unrestricted_mv*: cint     ## /< mv can point outside of the coded picture
    h263_long_vectors*: cint   ## /< use horrible H.263v1 long vector mode
    bdsp*: BlockDSPContext
    fdsp*: FDCTDSPContext
    h264chroma*: H264ChromaContext
    hdsp*: HpelDSPContext
    idsp*: IDCTDSPContext
    mecc*: MECmpContext
    mdsp*: MpegVideoDSPContext
    mpvencdsp*: MpegvideoEncDSPContext
    pdsp*: PixblockDSPContext
    qdsp*: QpelDSPContext
    vdsp*: VideoDSPContext
    h263dsp*: H263DSPContext
    f_code*: cint              ## /< forward MV resolution
    b_code*: cint              ## /< backward MV resolution for B-frames (MPEG-4)
    p_mv_table_base*: array[2, int16]
    b_forw_mv_table_base*: array[2, int16]
    b_back_mv_table_base*: array[2, int16]
    b_bidir_forw_mv_table_base*: array[2, int16]
    b_bidir_back_mv_table_base*: array[2, int16]
    b_direct_mv_table_base*: array[2, int16] ##  int16 (*p_field_mv_table_base[2][2])[2];
                                            ##  int16 (*b_field_mv_table_base[2][2][2])[2];
    p_mv_table*: array[2, int16] ## /< MV table (1MV per MB) P-frame encoding
    b_forw_mv_table*: array[2, int16] ## /< MV table (1MV per MB) forward mode B-frame encoding
    b_back_mv_table*: array[2, int16] ## /< MV table (1MV per MB) backward mode B-frame encoding
    b_bidir_forw_mv_table*: array[2, int16] ## /< MV table (1MV per MB) bidir mode B-frame encoding
    b_bidir_back_mv_table*: array[2, int16] ## /< MV table (1MV per MB) bidir mode B-frame encoding
    b_direct_mv_table*: array[2, int16] ## /< MV table (1MV per MB) direct mode B-frame encoding
                                       ##  int16 (*p_field_mv_table[2][2])[2];   ///< MV table (2MV per MB) interlaced P-frame encoding
                                       ##  int16 (*b_field_mv_table[2][2][2])[2];///< MV table (4MV per MB) interlaced B-frame encoding
                                       ##  uint8 (*p_field_select_table[2]);
                                       ##  uint8 (*b_field_select_table[2][2]);
    motion_est*: cint          ## /< ME algorithm
    me_penalty_compensation*: cint
    me_pre*: cint              ## /< prepass for motion estimation
    mv_dir*: cint
    mv_type*: cint
    mv*: array[2, array[4, array[2, cint]]]
    field_select*: array[2, array[2, cint]]
    last_mv*: array[2, array[2, array[2, cint]]] ## /< last MV, used for MV prediction in MPEG-1 & B-frame MPEG-4
    fcode_tab*: uint8     ## /< smallest fcode needed for each MV
    direct_scale_mv*: array[2, array[64, int16]] ## /< precomputed to avoid divisions in ff_mpeg4_set_direct_mv
    me*: MotionEstContext
    no_rounding*: cint ## *< apply no rounding to motion compensation (MPEG-4, msmpeg4, ...)
                     ##                         for B-frames rounding mode is always 0
                     ##  macroblock layer
    mb_x*: cint
    mb_y*: cint
    mb_skip_run*: cint
    mb_intra*: cint
    mb_type*: uint16      ## /< Table for candidate MB types for encoding (defines in mpegutils.h)
    block_index*: array[6, cint] ## /< index to current MB in block based arrays with edges
    block_wrap*: array[6, cint]
    dest*: array[3, uint8]
    mb_index2xy*: cint      ## /< mb_index -> mb_x + mb_y*mb_stride
                        ## * matrix transmitted in the bitstream
    intra_matrix*: array[64, uint16]
    chroma_intra_matrix*: array[64, uint16]
    inter_matrix*: array[64, uint16]
    chroma_inter_matrix*: array[64, uint16]
    force_duplicated_matrix*: cint ## /< Force duplication of mjpeg matrices, useful for rtp streaming
    intra_quant_bias*: cint    ## /< bias for the quantizer
    inter_quant_bias*: cint    ## /< bias for the quantizer
    min_qcoeff*: cint          ## /< minimum encodable coefficient
    max_qcoeff*: cint          ## /< maximum encodable coefficient
    ac_esc_length*: cint       ## /< num of bits needed to encode the longest esc
    intra_ac_vlc_length*: uint8
    intra_ac_vlc_last_length*: uint8
    intra_chroma_ac_vlc_length*: uint8
    intra_chroma_ac_vlc_last_length*: uint8
    inter_ac_vlc_length*: uint8
    inter_ac_vlc_last_length*: uint8
    luma_dc_vlc_length*: uint8
    coded_score*: array[12, cint] ## * precomputed matrix (combine qscale and DCT renorm)
    q_intra_matrix*: array[64, cint]
    q_chroma_intra_matrix*: array[64, cint]
    q_inter_matrix*: array[64, cint] ## * identical to the above but for MMX & these are not permutated, second 64 entries are bias
    q_intra_matrix16*: array[2, array[64, uint16]]
    q_chroma_intra_matrix16*: array[2, array[64, uint16]]
    q_inter_matrix16*: array[2, array[64, uint16]] ##  noise reduction
    dct_error_sum*: array[64, cint]
    dct_count*: array[2, cint]
    dct_offset*: array[64, uint16] ##  bit rate control
    total_bits*: int64
    frame_bits*: cint          ## /< bits used for the current frame
    stuffing_bits*: cint       ## /< bits used for stuffing
    next_lambda*: cint         ## /< next lambda used for retrying to encode a frame
    rc_context*: RateControlContext ## /< contains stuff only accessed in ratecontrol.c statistics, used for 2-pass encoding
    mv_bits*: cint
    header_bits*: cint
    i_tex_bits*: cint
    p_tex_bits*: cint
    i_count*: cint
    f_count*: cint
    b_count*: cint
    skip_count*: cint
    misc_bits*: cint           ## /< cbp, mb_type
    last_bits*: cint           ## /< temp var used for calculating the above vars error concealment / resync
    resync_mb_x*: cint         ## /< x position of last resync marker
    resync_mb_y*: cint         ## /< y position of last resync marker
    last_resync_gb*: GetBitContext ## /< used to search for the next resync marker
    mb_num_left*: cint         ## /< number of MBs left in this video packet (for partitioned Slices only)
    next_p_frame_damaged*: cint ## /< set if the next p frame is damaged, to avoid showing trashed B-frames
    parse_context*: ParseContext ##  H.263 specific
    gob_index*: cint
    obmc*: cint                ## /< overlapped block motion compensation
    mb_info*: cint             ## /< interval for outputting info about mb offsets as side data
    prev_mb_info*: cint
    last_mb_info*: cint
    mb_info_ptr*: uint8
    mb_info_size*: cint
    ehc_mode*: cint
    rc_strategy*: cint         ## /< deprecated
                     ##  H.263+ specific
    umvplus*: cint             ## /< == H.263+ && unrestricted_mv
    h263_aic_dir*: cint        ## /< AIC direction: 0 = left, 1 = top
    h263_slice_structured*: cint
    alt_inter_vlc*: cint       ## /< alternative inter vlc
    modified_quant*: cint
    loop_filter*: cint
    custom_pcf*: cint          ##  MPEG-4 specific
    studio_profile*: cint
    dct_precision*: cint       ## /< number of bits to represent the fractional part of time (encoder only)
    time_increment_bits*: cint
    last_time_base*: cint
    time_base*: cint           ## /< time in seconds of last I,P,S Frame
    time*: int64             ## /< time of current frame
    last_non_b_time*: int64
    pp_time*: uint16         ## /< time distance between the last 2 p,s,i frames
    pb_time*: uint16         ## /< time distance between the last b and p,s,i frame
    pp_field_time*: uint16
    pb_field_time*: uint16   ## /< like above, just for interlaced
    real_sprite_warping_points*: cint
    sprite_offset*: array[2, array[2, cint]] ## /< sprite offset[isChroma][isMVY]
    sprite_delta*: array[2, array[2, cint]] ## /< sprite_delta [isY][isMVY]
    mcsel*: cint
    quant_precision*: cint
    quarter_sample*: cint      ## /< 1->qpel, 0->half pel ME/MC
    aspect_ratio_info*: cint   ## FIXME remove
    sprite_warping_accuracy*: cint
    data_partitioning*: cint   ## /< data partitioning flag from header
    partitioned_frame*: cint   ## /< is current frame partitioned
    low_delay*: cint           ## /< no reordering needed / has no B-frames
    vo_type*: cint
    tex_pb*: PutBitContext     ## /< used for data partitioned VOPs
    pb2*: PutBitContext        ## /< used for data partitioned VOPs
    mpeg_quant*: cint
    padding_bug_score*: cint ## /< used to detect the VERY common padding bug in MPEG-4
                           ##  divx specific, used to workaround (many) bugs in divx5
    divx_packed*: cint
    bitstream_buffer*: uint8 ## Divx 5.01 puts several frames in a single one, this is used to reorder them
    bitstream_buffer_size*: cint
    allocated_bitstream_buffer_size*: cuint ##  RV10 specific
    rv10_version*: cint        ## /< RV10 version: 0 or 3
    rv10_first_dc_coded*: array[3, cint] ##  MJPEG specific
    mjpeg_ctx*: MJpegContext
    esc_pos*: cint
    pred*: cint
    huffman*: cint             ##  MSMPEG4 specific
    mv_table_index*: cint
    rl_table_index*: cint
    rl_chroma_table_index*: cint
    dc_table_index*: cint
    use_skip_mb_code*: cint
    slice_height*: cint        ## /< in macroblocks
    first_slice_line*: cint    ## /< used in MPEG-4 too to handle resync markers
    flipflop_rounding*: cint
    msmpeg4_version*: cint     ## /< 0=not msmpeg4, 1=mp41, 2=mp42, 3=mp43/divx3 4=wmv1/7 5=wmv2/8
    per_mb_rl_table*: cint
    esc3_level_length*: cint
    esc3_run_length*: cint     ## * [mb_intra][isChroma][level][run][last]
    ac_stats*: array[2, array[MAX_LEVEL + 1, array[MAX_RUN + 1, array[2, cint]]]]
    inter_intra_pred*: cint
    mspel*: cint               ##  decompression specific
    gb*: GetBitContext         ##  MPEG-1 specific
    gop_picture_number*: cint  ## /< index of the first picture of a GOP based on fake_pic_num & MPEG-1 specific
    last_mv_dir*: cint         ## /< last mv_dir, used for B-frame encoding
    vbv_delay_ptr*: uint8 ## /< pointer to vbv_delay in the bitstream MPEG-2-specific - I wished not to have to support this mess.
    progressive_sequence*: cint
    mpeg_f_code*: array[2, array[2, cint]]
    a53_cc*: cint              ##  picture structure defines are loaded from mpegutils.h
    picture_structure*: cint
    timecode_frame_start*: int64 ## /< GOP timecode frame start number, in non drop frame format
    intra_dc_precision*: cint
    frame_pred_frame_dct*: cint
    top_field_first*: cint
    concealment_motion_vectors*: cint
    q_scale_type*: cint
    brd_scale*: cint
    intra_vlc_format*: cint
    alternate_scan*: cint
    seq_disp_ext*: cint
    video_format*: cint
    repeat_first_field*: cint
    chroma_420_type*: cint
    chroma_format*: cint
    chroma_x_shift*: cint      ## depend on pix_format, that depend on chroma_format
    chroma_y_shift*: cint
    progressive_frame*: cint
    full_pel*: array[2, cint]
    interlaced_dct*: cint
    first_field*: cint         ## /< is 1 for the first field of a field picture 0 otherwise
    drop_frame_timecode*: cint ## /< timecode is in drop frame format.
    scan_offset*: cint         ## /< reserve space for SVCD scan offset user data.
                     ##  RTP specific
    rtp_mode*: cint
    rtp_payload_size*: cint
    tc_opt_str*: string       ## /< timecode option string
    tc*: AVTimecode            ## /< timecode context
    ptr_lastgob*: uint8
    swap_uv*: cint             ## vcr2 codec is an MPEG-2 variant with U and V swapped
    pack_pblocks*: cint        ## xvmc needs to keep blocks without gaps.
                      ##  int16 (*pblocks[12])[64];
    b*: array[64, int16] ## /< points to one of the following blocks
    blocks*: array[12, array[64, int16]] ##  for HQ mode we need to keep the best block
    decode_mb*: proc (s: ptr MpegEncContext; b: array[12, array[64, int16]]): cint ##  used by some codecs to avoid a switch()
    block32*: array[12, array[64, int32]]
    dpcm_direction*: cint      ##  0 = DCT, 1 = DPCM top to bottom scan, -1 = DPCM bottom to top scan
    dpcm_macroblock*: array[3, array[256, int16]]
    dct_unquantize_mpeg1_intra*: proc (s: ptr MpegEncContext; b: int16; n: cint; qscale: cint)
    dct_unquantize_mpeg1_inter*: proc (s: ptr MpegEncContext; b: int16; n: cint; qscale: cint)
    dct_unquantize_mpeg2_intra*: proc (s: ptr MpegEncContext; b: int16; n: cint; qscale: cint)
    dct_unquantize_mpeg2_inter*: proc (s: ptr MpegEncContext; b: int16; n: cint; qscale: cint)
    dct_unquantize_h263_intra*: proc (s: ptr MpegEncContext; b: int16; n: cint; qscale: cint)
    dct_unquantize_h263_inter*: proc (s: ptr MpegEncContext; b: int16; n: cint; qscale: cint)
    dct_unquantize_intra*: proc (s: ptr MpegEncContext; b: int16;n: cint; qscale: cint)
    dct_unquantize_inter*: proc (s: ptr MpegEncContext; b: int16; n: cint; qscale: cint)
    dct_quantize*: proc (s: ptr MpegEncContext; b: int16; n: cint; qscale: cint; overflow: cint): cint
    fast_dct_quantize*: proc (s: ptr MpegEncContext; b: int16; n: cint; qscale: cint; overflow: cint): cint
    denoise_dct*: proc (s: ptr MpegEncContext; b: int16)
    mpv_flags*: cint           ## /< flags set by private options
    quantizer_noise_shaping*: cint
    rc_qsquish*: cfloat
    rc_qmod_amp*: cfloat
    rc_qmod_freq*: cint
    rc_initial_cplx*: cfloat
    rc_buffer_aggressivity*: cfloat
    border_masking*: cfloat
    lmin*: cint
    lmax*: cint
    vbv_ignore_qmax*: cint
    rc_eq*: string            ##  temp buffers for rate control
    cplx_tab*: ptr cfloat
    bits_tab*: ptr cfloat ##  flag to indicate a reinitialization is required, e.g. after
                       ##  a frame size change
    context_reinit*: cint
    er*: ERContext
    error_rate*: cint
    tmp_frames*: array[MAX_B_FRAMES + 2, ptr AVFrame]
    b_frame_strategy*: cint
    b_sensitivity*: cint
    frame_skip_threshold*: cint
    frame_skip_factor*: cint
    frame_skip_exp*: cint
    frame_skip_cmp*: cint
    scenechange_threshold*: cint
    noise_reduction*: cint
    intra_penalty*: cint


  me_cmp_func* = proc (c: MpegEncContext; blk1: uint8; blk2: uint8; stride: int64; h: cint): cint
  MECmpContext* = ref object
    sum_abs_dctelem*: proc (b: int16): cint 
    sad*: array[6, me_cmp_func] 
    sse*: array[6, me_cmp_func]
    hadamard8_diff*: array[6, me_cmp_func]
    dct_sad*: array[6, me_cmp_func]
    quant_psnr*: array[6, me_cmp_func]
    bit*: array[6, me_cmp_func]
    rd*: array[6, me_cmp_func]
    vsad*: array[6, me_cmp_func]
    vsse*: array[6, me_cmp_func]
    nsse*: array[6, me_cmp_func]
    w53*: array[6, me_cmp_func]
    w97*: array[6, me_cmp_func]
    dct_max*: array[6, me_cmp_func]
    dct264_sad*: array[6, me_cmp_func]
    me_pre_cmp*: array[6, me_cmp_func]
    me_cmp*: array[6, me_cmp_func]
    me_sub_cmp*: array[6, me_cmp_func]
    mb_cmp*: array[6, me_cmp_func]
    ildct_cmp*: array[6, me_cmp_func] 
    frame_skip_cmp*: array[6, me_cmp_func] 
    pix_abs*: array[2, array[4, me_cmp_func]]
    median_sad*: array[6, me_cmp_func]

  atomic_int* = cint
  ERPicture* = ref object
    f*: AVFrame
    tf*: ThreadFrame        
    ref_index*: array[2, int8]
    mb_type*: uint32
    field_picture*: cint

  ERContext* = ref object
    avctx*: AVCodecContext
    mecc*: MECmpContext
    mecc_inited*: cint
    mb_index2xy*: cint
    mb_num*: cint
    mb_width*: cint
    mb_height*: cint
    mb_stride*: int64
    b8_stride*: int64
    error_count*: atomic_int
    error_occurred*: cint
    error_status_table*: uint8
    er_temp_buffer*: uint8
    dc_val*: array[3, int16]
    mbskip_table*: uint8
    mbintra_table*: uint8
    mv*: array[2, array[4, array[2, cint]]]
    cur_pic*: ERPicture
    last_pic*: ERPicture
    next_pic*: ERPicture
    ref_index_buf*: array[2, AVBufferRef]
    motion_val_buf*: array[2, AVBufferRef]
    pp_time*: uint16
    pb_time*: uint16
    quarter_sample*: cint
    partitioned_frame*: cint
    ref_count*: cint
    decode_mb*: proc (opaque: pointer; `ref`: cint; mv_dir: cint; mv_type: cint;mv: array[2, array[4, array[2, cint]]]; mb_x: cint; mb_y: cint;mb_intra: cint; mb_skipped: cint)
    opaque*: pointer

  AVTimecode* = ref object
    start*: cint               
    flags*: uint32           
    rate*: AVRational          
    fps*: cuint                

  MpegVideoDSPContext* = ref object
    gmc1*: proc (dst: uint8;  src: uint8; srcStride: cint; h: cint; x16: cint; y16: cint; rounder: cint)
    gmc*: proc (dst: uint8;src: uint8;stride: cint; h: cint; ox: cint; oy: cint; dxx: cint; dxy: cint; dyx: cint;dyy: cint; shift: cint; r: cint; width: cint; height: cint)

  MpegvideoEncDSPContext* = ref object
    try_8x8basis*: proc (rem: array[64, int16]; weight: array[64, int16],basis: array[64, int16]; scale: cint): cint
    add_8x8basis*: proc (rem: array[64, int16]; basis: array[64, int16]; scale: cint)
    pix_sum*: proc (pix: uint8; line_size: cint): cint
    pix_norm1*: proc (pix: uint8; line_size: cint): cint
    shrink*: proc(dst: uint8, dst_wrap: int, src:uint8, src_wrap: int,  width: int,  height:int)
    draw_edges*: proc (buf: uint8; wrap: cint; width: cint; height: cint; w: cint;h: cint; sides: cint)

  PixblockDSPContext* = ref object
    get_pixels*: proc (b: int16; pixels: uint8; stride: int64)
    get_pixels_unaligned*: proc (b: int16; pixels: uint8; stride: int64)
    diff_pixels*: proc (b: int16; s1: uint8; s2: uint8; stride: int64)
    diff_pixels_unaligned*: proc (b: int16; s1: uint8; s2: uint8;stride: int64)

  qpel_mc_func* = proc (dst: uint8; src: uint8; stride: int64)
  QpelDSPContext* = ref object
    put_qpel_pixels_tab*: array[2, array[16, qpel_mc_func]]
    avg_qpel_pixels_tab*: array[2, array[16, qpel_mc_func]]
    put_no_rnd_qpel_pixels_tab*: array[2, array[16, qpel_mc_func]]

  VideoDSPContext* = ref object
    emulated_edge_mc*: proc (dst: uint8; src: uint8; dst_linesize: int64;src_linesize: int64; block_w: cint; block_h: cint;src_x: cint; src_y: cint; w: cint; h: cint)
    prefetch*: proc (buf: uint8; stride: int64; h: cint)

  H263DSPContext* = ref object
    h263_h_loop_filter*: proc (src: uint8; stride: cint; qscale: cint)
    h263_v_loop_filter*: proc (src: uint8; stride: cint; qscale: cint)
  AVCodecHWConfig* = ref object
    pix_fmt*: AVPixelFormat
    methods*: cint
    device_type*: AVHWDeviceType
  AVHWAccel* = ref object
    name*: string
    t*: AVMediaType
    id*: AVCodecID
    pix_fmt*: AVPixelFormat
    capabilities*: cint
    alloc_frame*: proc (avctx: AVCodecContext; frame: AVFrame): cint
    start_frame*: proc (avctx: AVCodecContext; buf: uint8; buf_size: uint32): cint
    decode_params*: proc (avctx: AVCodecContext; t: cint; buf: uint8;
                        buf_size: uint32): cint
    decode_slice*: proc (avctx: AVCodecContext; buf: uint8; buf_size: uint32): cint
    end_frame*: proc (avctx: AVCodecContext): cint
    frame_priv_data_size*: cint
    decode_mb*: proc (s: MpegEncContext)
    init*: proc (avctx: AVCodecContext): cint
    uninit*: proc (avctx: AVCodecContext): cint
    priv_data_size*: cint
    caps_internal*: cint
    frame_params*: proc (avctx: AVCodecContext; hw_frames_ctx: AVBufferRef): cint

  AVCodecHWConfigInternal* = ref object
    public*: AVCodecHWConfig
    hwaccel*: ptr AVHWAccel
  
  AVCodec* = ref object
    name*: string
    long_name*: string
    t*: AVMediaType
    id*: AVCodecID
    capabilities*: cint
    supported_framerates*: ptr AVRational 
    pix_fmts*: ptr AVPixelFormat 
    supported_samplerates*: cint 
    sample_fmts*: ptr AVSampleFormat 
    channel_layouts*: uint64 
    max_lowres*: uint8
    priv_class*: AVClass    
    profiles*: ptr AVProfile    
    wrapper_name*: string
    priv_data_size*: cint
    next*: AVCodec
    init_thread_copy*: proc (a1: AVCodecContext): cint
    update_thread_context*: proc (dst: AVCodecContext; src: AVCodecContext): cint
    defaults*: ptr AVCodecDefault
    init_static_data*: proc (codec: AVCodec)
    init*: proc (a1: AVCodecContext): cint
    encode_sub*: proc (a1: AVCodecContext; buf: uint8; buf_size: cint;sub: ptr AVSubtitle): cint
    encode2*: proc (avctx: AVCodecContext; avpkt: AVPacket; frame: AVFrame;got_packet_ptr: cint): cint
    decode*: proc (a1: AVCodecContext; outdata: pointer; outdata_size: cint;avpkt: AVPacket): cint
    close*: proc (a1: AVCodecContext): cint
    send_frame*: proc (avctx: AVCodecContext; frame: AVFrame): cint
    receive_packet*: proc (avctx: AVCodecContext; avpkt: AVPacket): cint
    receive_frame*: proc (avctx: AVCodecContext; frame: AVFrame): cint
    flush*: proc (a1: AVCodecContext)
    caps_internal*: cint
    bsfs*: string
    hw_configs*: ptr ptr AVCodecHWConfigInternal

  DecodeSimpleContext* = ref object
    in_pkt*: AVPacket

  AVPacketList* = ref object
    pkt*: AVPacket
    next*: AVPacketList

  EncodeSimpleContext* = ref object
    in_frame*: AVFrame

  AVBitStreamFilter* = ref object
    name*: string
    codec_ids*: ptr AVCodecID
    priv_class*: AVClass
    priv_data_size*: cint
    init*: proc (ctx: ptr AVBSFContext): cint
    filter*: proc (ctx: ptr AVBSFContext; pkt: AVPacket): cint
    close*: proc (ctx: ptr AVBSFContext)
    flush*: proc (ctx: ptr AVBSFContext)

  AVBSFInternal* = ref object
    buffer_pkt*: AVPacket
    eof*: cint

  AVBSFContext* = ref object
    av_class*: AVClass
    filter*: ptr AVBitStreamFilter
    internal*: ptr AVBSFInternal
    priv_data*: pointer
    par_in*: AVCodecParameters
    par_out*: AVCodecParameters
    time_base_in*: AVRational
    time_base_out*: AVRational


  AVCodecInternal* = ref object
    is_copy*: cint
    last_audio_frame*: cint
    to_free*: AVFrame
    pool*: AVBufferRef
    thread_ctx*: pointer
    ds*: DecodeSimpleContext
    bsf*: ptr AVBSFContext
    last_pkt_props*: AVPacket
    pkt_props*: AVPacketList
    pkt_props_tail*: AVPacketList
    byte_buffer*: uint8
    byte_buffer_size*: cuint
    frame_thread_encoder*: pointer
    es*: EncodeSimpleContext
    skip_samples*: cint
    hwaccel_priv_data*: pointer
    draining*: cint
    buffer_pkt*: AVPacket
    buffer_frame*: AVFrame
    draining_done*: cint
    compat_decode_warned*: cint
    compat_decode_consumed*: csize_t
    compat_decode_partial_size*: csize_t
    compat_decode_frame*: AVFrame
    compat_encode_packet*: AVPacket
    showed_multi_packet_warning*: cint
    skip_samples_multiplier*: cint
    nb_draining_errors*: cint
    changed_frames_dropped*: cint
    initial_format*: cint
    initial_width*: cint
    initial_height*: cint
    initial_sample_rate*: cint
    initial_channels*: cint
    initial_channel_layout*: uint64

  RcOverride* = ref object
    start_frame*: cint
    end_frame*: cint
    qscale*: cint              
    quality_factor*: cfloat

  AVCodecDescriptor* = ref object
    id*: AVCodecID
    t*: AVMediaType
    name*: string
    long_name*: string
    props*: cint
    mime_types*: cstringArray
    profiles*: ptr AVProfile



  AVCodecContext* = ref object
    av_class*: AVClass
    log_level_offset*: cint
    codec_type*: AVMediaType   
    codec*: AVCodec
    codec_id*: AVCodecID       
    codec_tag*: cuint
    priv_data*: pointer
    internal*: ptr AVCodecInternal
    opaque*: pointer
    bit_rate*: int64
    bit_rate_tolerance*: cint
    global_quality*: cint
    compression_level*: cint
    flags*: cint
    flags2*: cint
    extradata*: uint8
    extradata_size*: cint
    time_base*: AVRational
    ticks_per_frame*: cint
    delay*: cint
    width*: cint
    height*: cint
    coded_width*: cint
    coded_height*: cint
    gop_size*: cint
    pix_fmt*: AVPixelFormat
    draw_horiz_band*: proc (s: AVCodecContext; src: AVFrame;offset: array[8, cint]; y: cint;t: cint; height: cint)
    get_format*: proc (s: AVCodecContext; fmt: ptr AVPixelFormat): AVPixelFormat
    max_b_frames*: cint
    b_quant_factor*: cfloat
    b_quant_offset*: cfloat
    has_b_frames*: cint
    i_quant_factor*: cfloat
    i_quant_offset*: cfloat
    lumi_masking*: cfloat
    temporal_cplx_masking*: cfloat
    spatial_cplx_masking*: cfloat
    p_masking*: cfloat
    dark_masking*: cfloat
    slice_count*: cint
    slice_offset*: cint
    sample_aspect_ratio*: AVRational
    me_cmp*: cint
    me_sub_cmp*: cint
    mb_cmp*: cint
    ildct_cmp*: cint
    dia_size*: cint
    last_predictor_count*: cint
    me_pre_cmp*: cint
    pre_dia_size*: cint
    me_subpel_quality*: cint
    me_range*: cint
    slice_flags*: cint
    mb_decision*: cint
    intra_matrix*: ptr uint16
    inter_matrix*: ptr uint16
    intra_dc_precision*: cint
    skip_top*: cint
    skip_bottom*: cint
    mb_lmin*: cint
    mb_lmax*: cint
    bidir_refine*: cint
    keyint_min*: cint
    refs*: cint
    mv0_threshold*: cint
    color_primaries*: AVColorPrimaries
    color_trc*: AVColorTransferCharacteristic
    colorspace*: AVColorSpace
    color_range*: AVColorRange
    chroma_sample_location*: AVChromaLocation
    slices*: cint
    field_order*: AVFieldOrder
    sample_rate*: cint         
    channels*: cint            
    sample_fmt*: AVSampleFormat 
    frame_size*: cint
    frame_number*: cint
    block_align*: cint
    cutoff*: cint
    channel_layout*: uint64
    request_channel_layout*: uint64
    audio_service_type*: AVAudioServiceType
    request_sample_fmt*: AVSampleFormat
    get_buffer2*: proc (s: AVCodecContext; frame: AVFrame; flags: cint): cint
    refcounted_frames*: cint
    qcompress*: cfloat         
    qblur*: cfloat             
    qmin*: cint
    qmax*: cint
    max_qdiff*: cint
    rc_buffer_size*: cint
    rc_override_count*: cint
    rc_override*: ptr RcOverride
    rc_max_rate*: int64
    rc_min_rate*: int64
    rc_max_available_vbv_use*: cfloat
    rc_min_vbv_overflow_use*: cfloat
    rc_initial_buffer_occupancy*: cint
    trellis*: cint
    stats_out*: string
    stats_in*: string
    workaround_bugs*: cint
    strict_std_compliance*: cint
    error_concealment*: cint
    debug*: cint
    debug_mv*: cint
    err_recognition*: cint
    reordered_opaque*: int64
    hwaccel*: ptr AVHWAccel
    hwaccel_context*: pointer
    error*: array[8, uint64]
    dct_algo*: cint
    idct_algo*: cint
    bits_per_coded_sample*: cint
    bits_per_raw_sample*: cint
    thread_count*: cint
    thread_type*: cint
    active_thread_type*: cint
    thread_safe_callbacks*: cint
    execute*: proc (c: AVCodecContext;`func`: proc (c2: AVCodecContext; arg: pointer): cint;arg2: pointer; ret: cint; count: cint; size: cint): cint
    execute2*: proc (c: AVCodecContext; `func`: proc (c2: AVCodecContext;arg: pointer; jobnr: cint; threadnr: cint): cint; arg2: pointer; ret: cint;count: cint): cint
    nsse_weight*: cint
    profile*: cint
    level*: cint
    skip_loop_filter*: AVDiscard
    skip_idct*: AVDiscard
    skip_frame*: AVDiscard
    subtitle_header*: uint8
    subtitle_header_size*: cint
    initial_padding*: cint
    framerate*: AVRational
    sw_pix_fmt*: AVPixelFormat
    pkt_timebase*: AVRational
    codec_descriptor*: ptr AVCodecDescriptor
    pts_correction_num_faulty_pts*: int64 
    pts_correction_num_faulty_dts*: int64 
    pts_correction_last_pts*: int64 
    pts_correction_last_dts*: int64 
    sub_charenc*: string
    sub_charenc_mode*: cint
    skip_alpha*: cint
    seek_preroll*: cint
    chroma_intra_matrix*: ptr uint16
    dump_separator*: uint8
    codec_whitelist*: string
    properties*: cuint
    coded_side_data*: ptr AVPacketSideData
    nb_coded_side_data*: cint
    hw_frames_ctx*: AVBufferRef
    sub_text_format*: cint
    trailing_padding*: cint
    max_pixels*: int64
    hw_device_ctx*: AVBufferRef
    hwaccel_flags*: cint
    apply_cropping*: cint
    extra_hw_frames*: cint
    discard_damaged_percentage*: cint
    max_samples*: int64
    export_side_data*: cint

  AVCodecParser* = ref object
    codec_ids*: array[5, cint]  
    priv_data_size*: cint
    parser_init*: proc (s: AVCodecParserContext): cint
    parser_parse*: proc (s: AVCodecParserContext; avctx: AVCodecContext;poutbuf: uint8; poutbuf_size: cint;buf: uint8; buf_size: cint): cint
    parser_close*: proc (s: AVCodecParserContext)
    split*: proc (avctx: AVCodecContext; buf: uint8; buf_size: cint): cint
    next*: AVCodecParser


  AVCodecParserContext* = ref object
    priv_data*: pointer
    parser*: AVCodecParser
    frame_offset*: int64     
    cur_offset*: int64       
    next_frame_offset*: int64 
    pict_type*: cint          
    repeat_pict*: cint         
    pts*: int64              
    dts*: int64              
    last_pts*: int64
    last_dts*: int64
    fetch_timestamp*: cint
    cur_frame_start_index*: cint
    cur_frame_offset*: array[AV_PARSER_PTS_NB, int64]
    cur_frame_pts*: array[AV_PARSER_PTS_NB, int64]
    cur_frame_dts*: array[AV_PARSER_PTS_NB, int64]
    flags*: cint
    offset*: int64           
    cur_frame_end*: array[AV_PARSER_PTS_NB, int64]
    key_frame*: cint
    convergence_duration*: int64
    dts_sync_point*: cint
    dts_ref_dts_delta*: cint
    pts_dts_delta*: cint
    cur_frame_pos*: array[AV_PARSER_PTS_NB, int64]
    pos*: int64
    last_pos*: int64
    duration*: cint
    field_order*: AVFieldOrder
    picture_structure*: AVPictureStructure
    output_picture_number*: cint
    width*: cint
    height*: cint
    coded_width*: cint
    coded_height*: cint
    format*: cint

  AVIndexEntry* = ref object
    pos*: int64
    timestamp*: int64
    flags* {.bitsize: 2.}: cint
    size* {.bitsize: 30.}: cint  
    min_distance*: cint  

  AVStreamInternalInner* = ref object
    bsf*: ptr AVBSFContext
    pkt*: AVPacket
    inited*: cint

  FFFrac* = ref object
    val*: int64
    num*: int64
    den*: int64
  AVStreamInternal* = ref object
    reorder*: cint
    bsfc*: AVBSFContext
    bitstream_checked*: cint
    avctx*: AVCodecContext
    avctx_inited*: cint
    orig_codec_id*: AVCodecID
    extract_extradata*: AVStreamInternalInner
    need_context_update*: cint
    is_intra_only*: cint
    priv_pts*: FFFrac
  AVStreamInnerStruct* = ref object
    last_dts*: int64
    duration_gcd*: int64
    duration_count*: cint
    rfps_duration_sum*: int64
    duration_error*: array[2, array[MAX_STD_TIMEBASES, cdouble]]
    codec_info_duration*: int64
    codec_info_duration_fields*: int64
    frame_delay_evidence*: cint
    found_decoder*: cint
    last_duration*: int64
    fps_first_dts*: int64
    fps_first_dts_idx*: cint
    fps_last_dts*: int64
    fps_last_dts_idx*: cint
  AVStream* = ref object
    index*: cint               
    id*: cint
    priv_data*: pointer
    time_base*: AVRational
    start_time*: int64
    duration*: int64
    nb_frames*: int64        
    disposition*: cint         
    `discard`*: AVDiscard      
    sample_aspect_ratio*: AVRational
    metadata*: Table[string,string]
    avg_frame_rate*: AVRational
    attached_pic*: AVPacket
    side_data*: ptr AVPacketSideData
    nb_side_data*: cint
    event_flags*: cint
    r_frame_rate*: AVRational
    codecpar*: AVCodecParameters
    info*: AVStreamInnerStruct
    pts_wrap_bits*: cint       
    first_dts*: int64
    cur_dts*: int64
    last_IP_pts*: int64
    last_IP_duration*: cint
    probe_packets*: cint
    codec_info_nb_frames*: cint
    need_parsing*: AVStreamParseType
    parser*: AVCodecParserContext
    last_in_packet_buffer*: AVPacketList
    probe_data*: AVProbeData
    pts_buffer*: array[MAX_REORDER_DELAY + 1, int64]
    index_entries*: ptr AVIndexEntry 
    nb_index_entries*: cint
    index_entries_allocated_size*: cuint
    stream_identifier*: cint
    program_num*: cint
    pmt_version*: cint
    pmt_stream_idx*: cint
    interleaver_chunk_size*: int64
    interleaver_chunk_duration*: int64
    request_probe*: cint
    skip_to_keyframe*: cint
    skip_samples*: cint
    start_skip_samples*: int64
    first_discard_sample*: int64
    last_discard_sample*: int64
    nb_decoded_frames*: cint
    mux_ts_offset*: int64
    pts_wrap_reference*: int64
    pts_wrap_behavior*: cint
    update_initial_durations_done*: cint
    pts_reorder_error*: array[MAX_REORDER_DELAY + 1, int64]
    pts_reorder_error_count*: array[MAX_REORDER_DELAY + 1, uint8]
    last_dts_for_order_check*: int64
    dts_ordered*: uint8
    dts_misordered*: uint8
    inject_global_side_data*: cint
    display_aspect_ratio*: AVRational
    internal*: AVStreamInternal
  AVProgram* = ref object
    id*: cint
    flags*: cint
    `discard`*: AVDiscard      
    stream_index*: ptr cuint
    nb_stream_indexes*: cuint
    metadata*: Table[string,string]
    program_num*: cint
    pmt_pid*: cint
    pcr_pid*: cint
    pmt_version*: cint
    start_time*: int64
    end_time*: int64
    pts_wrap_reference*: int64 
    pts_wrap_behavior*: cint   
  AVChapter* = ref object
    id*: cint                  
    time_base*: AVRational     
    start*: int64
    e*: int64            
    metadata*: Table[string,string]

  AVIOInterruptCB* = ref object
    callback*: proc (a1: pointer): cint
    opaque*: pointer

  AVFormatInternal* = ref object
    nb_interleaved_streams*: cint
    packet_buffer*: AVPacketList
    packet_buffer_end*: AVPacketList
    data_offset*: int64      
    raw_packet_buffer*: AVPacketList
    raw_packet_buffer_end*: AVPacketList
    parse_queue*: AVPacketList
    parse_queue_end*: AVPacketList
    raw_packet_buffer_remaining_size*: cint
    offset*: int64
    offset_timebase*: AVRational
    inject_global_side_data*: cint
    avoid_negative_ts_use_pts*: cint
    shortest_end*: int64
    initialized*: cint
    streams_initialized*: cint
    id3v2_meta*: Table[string,string]
    prefer_codec_framerate*: cint

  av_format_control_message* = proc (s: AVFormatContext; t: cint;data: pointer; data_size: csize_t): cint

  AVFormatContext* = ref object
    av_class*: AVClass
    iformat*: AVInputFormat
    oformat*: AVOutputFormat
    priv_data*: pointer
    pb*: AVIOContext
    ctx_flags*: cint
    nb_streams*: cuint
    streams*: seq[AVStream]
    url*: string
    start_time*: int64
    duration*: int64
    bit_rate*: int64
    packet_size*: cuint
    max_delay*: cint
    flags*: cint
    probesize*: int64
    max_analyze_duration*: int64
    key*: uint8
    keylen*: cint
    nb_programs*: cuint
    programs*: AVProgram
    video_codec_id*: AVCodecID
    audio_codec_id*: AVCodecID
    subtitle_codec_id*: AVCodecID
    max_index_size*: cuint
    max_picture_buffer*: cuint
    nb_chapters*: cuint
    chapters*: AVChapter
    metadata*: Table[string,string]
    start_time_realtime*: int64
    fps_probe_size*: cint
    error_recognition*: cint
    interrupt_callback*: AVIOInterruptCB
    debug*: cint
    max_interleave_delta*: int64
    strict_std_compliance*: cint
    event_flags*: cint
    max_ts_probe*: cint
    avoid_negative_ts*: cint
    ts_id*: cint
    audio_preload*: cint
    max_chunk_duration*: cint
    max_chunk_size*: cint
    use_wallclock_as_timestamps*: cint
    avio_flags*: cint
    duration_estimation_method*: AVDurationEstimationMethod
    skip_initial_bytes*: int64
    correct_ts_overflow*: cuint
    seek2any*: cint
    flush_packets*: cint
    probe_score*: cint
    format_probesize*: cint
    codec_whitelist*: string
    format_whitelist*: string
    internal*: AVFormatInternal
    io_repositioned*: cint
    video_codec*: AVCodec
    audio_codec*: AVCodec
    subtitle_codec*: AVCodec
    data_codec*: AVCodec
    metadata_header_padding*: cint
    opaque*: pointer
    control_message_cb*: av_format_control_message
    output_ts_offset*: int64
    dump_separator*: uint8
    data_codec_id*: AVCodecID
    protocol_whitelist*: string
    io_open*: proc (s: AVFormatContext; pb: AVIOContext; url: string;flags: cint; options: TableRef[string,string]): cint
    io_close*: proc (s: AVFormatContext; pb: AVIOContext)
    protocol_blacklist*: string
    max_streams*: cint
    skip_estimate_duration_from_pts*: cint
    max_probe_packets*: cint
  AVDeviceInfo* = ref object
    device_name*: string      
    device_description*: string 
  AVDeviceInfoList* = ref object
    devices*: AVDeviceInfo
    nb_devices*: cint          
    default_device*: cint      

  AVDeviceCapabilitiesQuery* = ref object
    av_class*: AVClass
    device_context*: AVFormatContext
    codec*: AVCodecID
    sample_format*: AVSampleFormat
    pixel_format*: AVPixelFormat
    sample_rate*: cint
    channels*: cint
    channel_layout*: int64
    window_width*: cint
    window_height*: cint
    frame_width*: cint
    frame_height*: cint
    fps*: AVRational


  AVOutputFormat* = ref object
    name*: string
    long_name*: string
    mime_type*: string
    extensions*: string       
    audio_codec*: AVCodecID    
    video_codec*: AVCodecID    
    subtitle_codec*: AVCodecID 
    flags*: cint
    codec_tag*: ptr ptr AVCodecTag
    priv_class*: AVClass    
    next*: AVOutputFormat
    priv_data_size*: cint
    write_header*: proc (a1: AVFormatContext): cint
    write_packet*: proc (a1: AVFormatContext; pkt: AVPacket): cint
    write_trailer*: proc (a1: AVFormatContext): cint
    interleave_packet*: proc (a1: AVFormatContext; `out`: AVPacket;`in`: AVPacket; flush: cint): cint
    query_codec*: proc (id: AVCodecID; std_compliance: cint): cint
    get_output_timestamp*: proc (s: AVFormatContext; stream: cint;dts: ptr int64; wall: ptr int64)
    control_message*: proc (s: AVFormatContext; t: cint; data: pointer;data_size: csize_t): cint
    write_uncoded_frame*: proc (a1: AVFormatContext; stream_index: cint;frame: ptr AVFrame; flags: cuint): cint
    get_device_list*: proc (s: AVFormatContext; device_list: AVDeviceInfoList): cint
    create_device_capabilities*: proc (s: AVFormatContext;caps: AVDeviceCapabilitiesQuery): cint
    free_device_capabilities*: proc (s: AVFormatContext;caps: AVDeviceCapabilitiesQuery): cint
    data_codec*: AVCodecID     
    init*: proc (a1: AVFormatContext): cint
    deinit*: proc (a1: AVFormatContext)
    check_bitstream*: proc (a1: AVFormatContext; pkt: AVPacket): cint
  AVInputFormat* = ref object
    name*: string
    long_name*: string
    flags*: cint
    extensions*: string
    codec_tag*: AVCodecTag
    priv_class*: AVClass    
    mime_type*: string
    next*: AVInputFormat
    raw_codec_id*: cint
    priv_data_size*: cint
    read_probe*: proc (a1: ptr AVProbeData): cint
    read_header*: proc (a1: AVFormatContext): cint
    read_packet*: proc (a1: AVFormatContext; pkt: AVPacket): cint
    read_close*: proc (a1: AVFormatContext): cint
    read_seek*: proc (a1: AVFormatContext; stream_index: cint; timestamp: int64;flags: cint): cint
    read_timestamp*: proc (s: AVFormatContext; stream_index: cint;pos: int64; pos_limit: int64): int64
    read_play*: proc (a1: AVFormatContext): cint
    read_pause*: proc (a1: AVFormatContext): cint
    read_seek2*: proc (s: AVFormatContext; stream_index: cint; min_ts: int64;ts: int64; max_ts: int64; flags: cint): cint
    get_device_list*: proc (s: AVFormatContext; device_list: AVDeviceInfoList): cint
    create_device_capabilities*: proc (s: AVFormatContext;caps: AVDeviceCapabilitiesQuery): cint
    free_device_capabilities*: proc (s: AVFormatContext;caps: AVDeviceCapabilitiesQuery): cint

  Frame* = ref object
    frame*: AVFrame
    sub*: AVSubtitle
    serial*: cint
    pts*: cdouble              
    duration*: cdouble         
    pos*: int64             
    width*: cint
    height*: cint
    format*: cint
    sar*: AVRational
    uploaded*: cint
    flip_v*: cint

  MyAVPacketList* = ref object
    pkt*: AVPacket
    next*: ptr MyAVPacketList
    serial*: cint

  PacketQueue* = ref object
    first_pkt*: ptr MyAVPacketList
    last_pkt*: ptr MyAVPacketList
    nb_packets*: cint
    size*: cint
    duration*: int64
    abort_request*: cint
    serial*: cint
    # mutex*: ptr SDL_mutex
    # cond*: ptr SDL_cond


  FrameQueue* = ref object
    queue*: array[16, Frame]
    rindex*: cint
    windex*: cint
    size*: cint
    max_size*: cint
    keep_last*: cint
    rindex_shown*: cint
    # mutex*: ptr SDL_mutex
    # cond*: ptr SDL_cond
    pktq*: ptr PacketQueue

  Decoder* = ref object
    pkt*: AVPacket
    queue*: ptr PacketQueue
    avctx*: AVCodecContext
    pkt_serial*: cint
    finished*: cint
    packet_pending*: cint
    # empty_queue_cond*: ptr SDL_cond
    start_pts*: int64
    start_pts_tb*: AVRational
    next_pts*: int64
    next_pts_tb*: AVRational
    # decoder_tid*: ptr SDL_Thread

  AudioParams* = ref object
    freq*: cint
    channels*: cint
    channel_layout*: int64
    fmt*: AVSampleFormat
    frame_size*: cint
    bytes_per_sec*: cint

  AudioData* = ref object
    ch*: array[64, uint8] 
    data*: uint8          
    ch_count*: cint            
    bps*: cint                 
    count*: cint               
    planar*: cint              
    fmt*: AVSampleFormat       


  DitherContext* = ref object
    `method`*: cint
    noise_pos*: cint
    scale*: cfloat
    noise_scale*: cfloat       
    ns_taps*: cint             
    ns_scale*: cfloat          
    ns_scale_1*: cfloat        
    ns_pos*: cint              
    ns_coeffs*: array[20, cfloat] 
    ns_errors*: array[64, array[2 * 20, cfloat]]
    noise*: AudioData          
    temp*: AudioData           
    output_sample_bits*: cint  
  conv_func_type* = proc (po: uint8; pi: uint8; videoState: cint; os: cint;e: uint8)
  simd_func_type* = proc (dst: uint8; src: uint8; len: cint)

  AudioConvert* = ref object
    channels*: cint
    in_simd_align_mask*: cint
    out_simd_align_mask*: cint
    conv_f*: conv_func_type
    simd_f*: simd_func_type
    ch_map*: cint
    silence*: array[8, uint8] 


  ResampleInner* = ref object
    resample_one*: proc (dst: pointer; src: pointer; n: cint; index: int64;incr: int64)
    resample_common*: proc (c: ptr ResampleContext; dst: pointer; src: pointer; n: cint;update_ctx: cint): cint
    resample_linear*: proc (c: ptr ResampleContext; dst: pointer; src: pointer; n: cint;update_ctx: cint): cint

  ResampleContext* = ref object
    av_class*: AVClass
    filter_bank*: uint8
    filter_length*: cint
    filter_alloc*: cint
    ideal_dst_incr*: cint
    dst_incr*: cint
    dst_incr_div*: cint
    dst_incr_mod*: cint
    index*: cint
    frac*: cint
    src_incr*: cint
    compensation_distance*: cint
    phase_count*: cint
    linear*: cint
    filter_type*: SwrFilterType
    kaiser_beta*: cdouble
    factor*: cdouble
    format*: AVSampleFormat
    felem_size*: cint
    filter_shift*: cint
    phase_count_compensation*: cint 
    dsp*: ResampleInner
  resample_init_func* = proc (c: ResampleContext; out_rate: cint; in_rate: cint;filter_size: cint; phase_shift: cint; linear: cint;cutoff: cdouble; format: AVSampleFormat;filter_type: SwrFilterType; kaiser_beta: cdouble;precision: cdouble; cheby: cint; exact_rational: cint): ResampleContext
  resample_free_func* = proc (c: ResampleContext)
  multiple_resample_func* = proc (c: ResampleContext; dst: AudioData;dst_size: cint; src: AudioData; src_size: cint;consumed: cint): cint
  resample_flush_func* = proc (c: SwrContext): cint
  set_compensation_func* = proc (c: ptr ResampleContext; sample_delta: cint;compensation_distance: cint): cint
  get_delay_func* = proc (s: SwrContext; base: int64): int64
  invert_initial_buffer_func* = proc (c: ResampleContext; dst: AudioData;src: AudioData; src_size: cint;dst_idx: cint; dst_count: cint): cint
  get_out_samples_func* = proc (s: ptr SwrContext; in_samples: cint): int64

  Resampler* = ref object
    init*: resample_init_func
    free*: resample_free_func
    multiple_resample*: multiple_resample_func
    flush*: resample_flush_func
    set_compensation*: set_compensation_func
    get_delay*: get_delay_func
    invert_initial_buffer*: invert_initial_buffer_func
    get_out_samples*: get_out_samples_func

  mix_1_1_func_type* = proc (`out`: pointer; `in`: pointer; coeffp: pointer;index: int64; len: int64)
  mix_2_1_func_type* = proc (`out`: pointer; in1: pointer; in2: pointer; coeffp: pointer;index1: int64; index2: int64; len: int64)
  mix_any_func_type* = proc (`out`: ptr int8; in1: ptr int8; coeffp: pointer;len: int64)


  SwrContext* = ref object
    av_class*: AVClass      
    log_level_offset*: cint    
    log_ctx*: pointer          
    in_sample_fmt*: AVSampleFormat 
    int_sample_fmt*: AVSampleFormat 
    out_sample_fmt*: AVSampleFormat 
    in_ch_layout*: int64     
    out_ch_layout*: int64    
    in_sample_rate*: cint      
    out_sample_rate*: cint     
    flags*: cint               
    slev*: cfloat              
    clev*: cfloat              
    lfe_mix_level*: cfloat     
    rematrix_volume*: cfloat   
    rematrix_maxval*: cfloat   
    matrix_encoding*: cint     
    channel_map*: cint      
    used_ch_count*: cint       
    engine*: cint
    user_in_ch_count*: cint    
    user_out_ch_count*: cint   
    user_used_ch_count*: cint  
    user_in_ch_layout*: int64 
    user_out_ch_layout*: int64 
    user_int_sample_fmt*: AVSampleFormat 
    user_dither_method*: cint  
    dither*: DitherContext
    filter_size*: cint         
    phase_shift*: cint         
    linear_interp*: cint       
    exact_rational*: cint      
    cutoff*: cdouble           
    filter_type*: cint         
    kaiser_beta*: cdouble      
    precision*: cdouble        
    cheby*: cint               
    min_compensation*: cfloat  
    min_hard_compensation*: cfloat 
    soft_compensation_duration*: cfloat 
    max_soft_compensation*: cfloat 
    async*: cfloat             
    firstpts_in_samples*: int64 
    resample_first*: cint      
    rematrix*: cint            
    rematrix_custom*: cint     
    `in`*: AudioData           
    postin*: AudioData         
    midbuf*: AudioData         
    preout*: AudioData         
    `out`*: AudioData          
    in_buffer*: AudioData      
    silence*: AudioData        
    drop_temp*: AudioData      
    in_buffer_index*: cint     
    in_buffer_count*: cint     
    resample_in_constraint*: cint 
    flushed*: cint             
    outpts*: int64           
    firstpts*: int64         
    drop_output*: cint         
    delayed_samples_fixup*: cdouble 
    in_convert*: ptr AudioConvert 
    out_convert*: ptr AudioConvert 
    full_convert*: ptr AudioConvert 
    resample*: ptr ResampleContext 
    resampler*: ptr Resampler   
    matrix*: array[SWR_CH_MAX, array[SWR_CH_MAX, cdouble]] 
    matrix_flt*: array[SWR_CH_MAX, array[SWR_CH_MAX, cfloat]] 
    native_matrix*: uint8
    native_one*: uint8
    native_simd_one*: uint8
    native_simd_matrix*: uint8
    matrix32*: array[SWR_CH_MAX, array[SWR_CH_MAX, int32]] 
    matrix_ch*: array[SWR_CH_MAX, array[SWR_CH_MAX + 1, uint8]] 
    mix_1_1_f*: ptr mix_1_1_func_type
    mix_1_1_simd*: ptr mix_1_1_func_type
    mix_2_1_f*: ptr mix_2_1_func_type
    mix_2_1_simd*: ptr mix_2_1_func_type
    mix_any_f*: ptr mix_any_func_type

  FFTComplex* = ref object
    re*: float
    im*: float

  FFTContext* = ref object
    nbits*: cint
    inverse*: cint
    revtab*: ptr uint16
    tmp_buf*: ptr FFTComplex
    mdct_size*: cint           
    mdct_bits*: cint           
    tcos*: float
    tsin*: float
    fft_permute*: proc (s: ptr FFTContext; z: ptr FFTComplex)
    fft_calc*: proc (s: ptr FFTContext; z: ptr FFTComplex)
    imdct_calc*: proc (s: ptr FFTContext; output: float; input: float)
    imdct_half*: proc (s: ptr FFTContext; output: float; input: float)
    mdct_calc*: proc (s: ptr FFTContext; output: float; input: float)
    mdct_calcw*: proc (s: ptr FFTContext; output: float; input: float)
    fft_permutation*: fft_permutation_type
    mdct_permutation*: mdct_permutation_type
    revtab32*: ptr uint32

  RDFTContext* = ref object
    nbits*: cint
    inverse*: cint
    sign_convention*: cint     
    tcos*: float
    tsin*: float
    negative_sin*: cint
    fft*: FFTContext
    rdft_calc*: proc (s: RDFTContext; z: float)

  pthread_once_t* = ref object
    done*: cint
    mtx*: Pthread_mutex

  AVComponentDescriptor* = tuple
    plane: int
    step: int
    offset: int
    shift: int
    depth: int
    step_minus1: int
    depth_minus1: int
    offset_plus1: int

  AVPixFmtDescriptor* = ref object
    name*: string
    nb_components*: uint8    ## /< The number of components each pixel has, (1-4)
    log2_chroma_w*: uint8
    log2_chroma_h*: uint8
    flags*: uint64
    comp*: seq[AVComponentDescriptor]
    alias*: string


  SwsFunc* = proc (context: SwsContext; src: uint8; srcStride: cint;
                srcSliceY: cint; srcSliceH: cint; dst: uint8;
                dstStride: cint): cint
  yuv2planar1_fn* = proc (src: int16; dest: uint8; dstW: cint;
                       dither: uint8; offset: cint)
  yuv2planarX_fn* = proc (filter: int16; filterSize: cint; src: ptr int16;
                       dest: uint8; dstW: cint; dither: uint8; offset: cint)
  yuv2interleavedX_fn* = proc (dstFormat: AVPixelFormat; chrDither: uint8;
                            chrFilter: int16; chrFilterSize: cint;
                            chrUSrc: ptr int16; chrVSrc: ptr int16;
                            dest: uint8; dstW: cint)
  yuv2packed1_fn* = proc (c: SwsContext; lumSrc: int16;
                       chrUSrc: array[2, int16];
                       chrVSrc: array[2, int16]; alpSrc: int16;
                       dest: uint8; dstW: cint; uvalpha: cint; y: cint)
  yuv2packed2_fn* = proc (c: SwsContext; lumSrc: array[2, int16];
                       chrUSrc: array[2, int16];
                       chrVSrc: array[2, int16];
                       alpSrc: array[2, int16]; dest: uint8; dstW: cint;
                       yalpha: cint; uvalpha: cint; y: cint)
  yuv2packedX_fn* = proc (c: SwsContext; lumFilter: int16;
                       lumSrc: ptr int16; lumFilterSize: cint;
                       chrFilter: int16; chrUSrc: ptr int16;
                       chrVSrc: ptr int16; chrFilterSize: cint;
                       alpSrc: ptr int16; dest: uint8; dstW: cint; y: cint)
  yuv2anyX_fn* = proc (c: SwsContext; lumFilter: int16;
                    lumSrc: ptr int16; lumFilterSize: cint;
                    chrFilter: int16; chrUSrc: ptr int16;
                    chrVSrc: ptr int16; chrFilterSize: cint;
                    alpSrc: ptr int16; dest: uint8; dstW: cint; y: cint)



  SwsPlane* = ref object
    available_lines*: cint     
    sliceY*: cint              
    sliceH*: cint              
    line*: uint8       
    tmp*: uint8        

  SwsSlice* = ref object
    width*: cint               
    h_chr_sub_sample*: cint    
    v_chr_sub_sample*: cint    
    is_ring*: cint             
    should_free_lines*: cint   
    fmt*: AVPixelFormat        
    plane*: array[MAX_SLICE_PLANES, SwsPlane] 



  SwsFilterDescriptor* = ref object
    src*: ptr SwsSlice          
    dst*: ptr SwsSlice          
    alpha*: cint               
    instance*: pointer         
    process*: proc (c: SwsContext; desc: ptr SwsFilterDescriptor; sliceY: cint;sliceH: cint): cint


  SwsContext* = ref object
    av_class*: AVClass
    swscale*: SwsFunc
    srcW*: cint                
    srcH*: cint                
    dstH*: cint                
    chrSrcW*: cint             
    chrSrcH*: cint             
    chrDstW*: cint             
    chrDstH*: cint             
    lumXInc*: cint
    chrXInc*: cint
    lumYInc*: cint
    chrYInc*: cint
    dstFormat*: AVPixelFormat  
    srcFormat*: AVPixelFormat  
    dstFormatBpp*: cint        
    srcFormatBpp*: cint        
    dstBpc*: cint
    srcBpc*: cint
    chrSrcHSubSample*: cint    
    chrSrcVSubSample*: cint    
    chrDstHSubSample*: cint    
    chrDstVSubSample*: cint    
    vChrDrop*: cint            
    sliceDir*: cint            
    param*: array[2, cdouble]   
    cascaded_context*: array[3, SwsContext]
    cascaded_tmpStride*: array[4, cint]
    cascaded_tmp*: array[4, uint8]
    cascaded1_tmpStride*: array[4, cint]
    cascaded1_tmp*: array[4, uint8]
    cascaded_mainindex*: cint
    gamma_value*: cdouble
    gamma_flag*: cint
    is_internal_gamma*: cint
    gamma*: ptr uint16
    inv_gamma*: ptr uint16
    numDesc*: cint
    descIndex*: array[2, cint]
    numSlice*: cint
    slice*: ptr SwsSlice
    desc*: ptr SwsFilterDescriptor
    pal_yuv*: array[256, uint32]
    pal_rgb*: array[256, uint32]
    uint2float_lut*: array[256, cfloat]
    lastInLumBuf*: cint        
    lastInChrBuf*: cint        
    formatConvBuffer*: uint8
    needAlpha*: cint
    hLumFilter*: int16    
    hChrFilter*: int16    
    vLumFilter*: int16    
    vChrFilter*: int16    
    hLumFilterPos*: ptr int32 
    hChrFilterPos*: ptr int32 
    vLumFilterPos*: ptr int32 
    vChrFilterPos*: ptr int32 
    hLumFilterSize*: cint      
    hChrFilterSize*: cint      
    vLumFilterSize*: cint      
    vChrFilterSize*: cint      
    lumMmxextFilterCodeSize*: cint 
    chrMmxextFilterCodeSize*: cint 
    lumMmxextFilterCode*: uint8 
    chrMmxextFilterCode*: uint8 
    canMMXEXTBeUsed*: cint
    warned_unuseable_bilinear*: cint
    dstY*: cint                
    flags*: cint               
    yuvTable*: pointer 
    table_gV*: array[256 + 2 * YUVRGB_TABLE_HEADROOM, cint]
    table_rV*: array[256 + 2 * YUVRGB_TABLE_HEADROOM, uint8]
    table_gU*: array[256 + 2 * YUVRGB_TABLE_HEADROOM, uint8]
    table_bU*: array[256 + 2 * YUVRGB_TABLE_HEADROOM, uint8]
    input_rgb2yuv_table*: array[16 + 40 * 4, int32] 
    dither_error*: array[4, cint]
    contrast*: cint
    brightness*: cint
    saturation*: cint          
    srcColorspaceTable*: array[4, cint]
    dstColorspaceTable*: array[4, cint]
    srcRange*: cint            
    dstRange*: cint            
    src0Alpha*: cint
    dst0Alpha*: cint
    srcXYZ*: cint
    dstXYZ*: cint
    src_h_chr_pos*: cint
    dst_h_chr_pos*: cint
    src_v_chr_pos*: cint
    dst_v_chr_pos*: cint
    yuv2rgb_y_offset*: cint
    yuv2rgb_y_coeff*: cint
    yuv2rgb_v2r_coeff*: cint
    yuv2rgb_v2g_coeff*: cint
    yuv2rgb_u2g_coeff*: cint
    yuv2rgb_u2b_coeff*: cint
    redDither*: uint64
    greenDither*: uint64
    blueDither*: uint64
    yCoeff*: uint64
    vrCoeff*: uint64
    ubCoeff*: uint64
    vgCoeff*: uint64
    ugCoeff*: uint64
    yOffset*: uint64
    uOffset*: uint64
    vOffset*: uint64
    lumMmxFilter*: array[4 * MAX_FILTER_SIZE, int32]
    chrMmxFilter*: array[4 * MAX_FILTER_SIZE, int32]
    dstW*: cint
    esp*: uint64
    vRounder*: uint64
    u_temp*: uint64
    v_temp*: uint64
    y_temp*: uint64
    alpMmxFilter*: array[4 * MAX_FILTER_SIZE, int32]
    uv_off*: int64         
    uv_offx2*: int64       
    dither16*: array[8, uint16]
    dither32*: array[8, uint32]
    chrDither8*: uint8
    lumDither8*: uint8
    use_mmx_vfilter*: cint
    xyzgamma*: int16
    rgbgamma*: int16
    xyzgammainv*: int16
    rgbgammainv*: int16
    xyz2rgb_matrix*: array[3, array[4, int16]]
    rgb2xyz_matrix*: array[3, array[4, int16]]
    yuv2plane1*: yuv2planar1_fn
    yuv2planeX*: yuv2planarX_fn
    yuv2nv12cX*: yuv2interleavedX_fn
    yuv2packed1*: yuv2packed1_fn
    yuv2packed2*: yuv2packed2_fn
    yuv2packedX*: yuv2packedX_fn
    yuv2anyX*: yuv2anyX_fn
    lumToYV12*: proc (dst: uint8; src: uint8; src2: uint8;src3: uint8; width: cint; pal: ptr uint32)
    alpToYV12*: proc (dst: uint8; src: uint8; src2: uint8;src3: uint8; width: cint; pal: ptr uint32)
    chrToYV12*: proc (dstU: uint8; dstV: uint8; src1: uint8;src2: uint8; src3: uint8; width: cint; pal: ptr uint32)
    readLumPlanar*: proc (dst: uint8; src: array[4, uint8]; width: cint;rgb2yuv: ptr int32)
    readChrPlanar*: proc (dstU: uint8; dstV: uint8;src: array[4, uint8]; width: cint; rgb2yuv: ptr int32)
    readAlpPlanar*: proc (dst: uint8; src: array[4, uint8]; width: cint;rgb2yuv: ptr int32)
    hyscale_fast*: proc (c: SwsContext; dst: int16; dstWidth: cint;src: uint8; srcW: cint; xInc: cint)
    hcscale_fast*: proc (c: SwsContext; dst1: int16; dst2: int16;dstWidth: cint; src1: uint8; src2: uint8; srcW: cint; xInc: cint)
    hyScale*: proc (c: SwsContext; dst: int16; dstW: cint; src: uint8; filter: int16; filterPos: ptr int32; filterSize: cint)
    hcScale*: proc (c: SwsContext; dst: int16; dstW: cint; src: uint8;filter: int16; filterPos: ptr int32; filterSize: cint)
    lumConvertRange*: proc (dst: int16; width: cint)
    chrConvertRange*: proc (dst1: int16; dst2: int16; width: cint)
    needs_hcscale*: cint       
    dither*: SwsDither
    alphablend*: SwsAlphaBlend

  avfilter_action_func* = proc (ctx: AVFilterContext; arg: pointer; jobnr: cint;nb_jobs: cint): cint
  avfilter_execute_func* = proc (ctx: AVFilterContext; `func`: ptr avfilter_action_func; arg: pointer;ret: cint; nb_jobs: cint): cint
  FFFrameQueueGlobal* = ref object
    dummy*: char               

  AVFilterGraphInternal* = ref object
    thread*: pointer
    thread_execute*: ptr avfilter_execute_func
    frame_queues*: FFFrameQueueGlobal

  AVFilterGraph* = ref object
    av_class*: AVClass
    filters*: ptr AVFilterContext
    nb_filters*: cuint
    scale_sws_opts*: string   
    thread_type*: cint
    nb_threads*: cint
    internal*: AVFilterGraphInternal
    opaque*: pointer
    execute*: ptr avfilter_execute_func
    aresample_swr_opts*: string 
    sink_links*: ptr AVFilterLink
    sink_links_count*: cint
    disable_auto_convert*: cuint



  VideoState* = ref object
    read_tid*: Thread[VideoState]
    iformat*: AVInputFormat
    abort_request*: cint
    force_refresh*: cint
    paused*: cint
    last_paused*: cint
    queue_attachments_req*: cint
    seek_req*: cint
    seek_flags*: cint
    seek_pos*: int64
    seek_rel*: int64
    read_pause_return*: cint
    ic*: AVFormatContext
    realtime*: cint
    audclk*: Clock
    vidclk*: Clock
    extclk*: Clock
    pictq*: FrameQueue
    subpq*: FrameQueue
    sampq*: FrameQueue
    auddec*: Decoder
    viddec*: Decoder
    subdec*: Decoder
    audio_stream*: cint
    av_sync_type*: cint
    audio_clock*: cdouble
    audio_clock_serial*: cint
    audio_diff_cum*: cdouble   
    audio_diff_avg_coef*: cdouble
    audio_diff_threshold*: cdouble
    audio_diff_avg_count*: cint
    audio_st*: AVStream
    audioq*: PacketQueue
    audio_hw_buf_size*: cint
    audio_buf*: uint8
    audio_buf1*: uint8
    audio_buf_size*: cuint     
    audio_buf1_size*: cuint
    audio_buf_index*: cint     
    audio_write_buf_size*: cint
    audio_volume*: cint
    muted*: cint
    audio_src*: AudioParams
    audio_filter_src*: AudioParams
    audio_tgt*: AudioParams
    swr_ctx*: ptr SwrContext
    frame_drops_early*: cint
    frame_drops_late*: cint
    show_mode*: ShowMode
    sample_array*: array[8 * 65536, int16]
    sample_array_index*: cint
    last_i_start*: cint
    rdft*: RDFTContext
    rdft_bits*: cint
    rdft_data*: float
    xpos*: cint
    last_vis_time*: cdouble
    vis_texture*: TexturePtr
    sub_texture*: TexturePtr
    vid_texture*: TexturePtr
    subtitle_stream*: cint
    subtitle_st*: AVStream
    subtitleq*: PacketQueue
    frame_timer*: cdouble
    frame_last_returned_time*: cdouble
    frame_last_filter_delay*: cdouble
    video_stream*: cint
    video_st*: AVStream
    videoq*: PacketQueue
    max_frame_duration*: cdouble 
    img_convert_ctx*: SwsContext
    sub_convert_ctx*: SwsContext
    eof*: cint
    filename*: string
    width*: cint
    height*: cint
    xleft*: cint
    ytop*: cint
    step*: cint
    vfilter_idx*: cint
    in_video_filter*: AVFilterContext 
    out_video_filter*: AVFilterContext 
    in_audio_filter*: AVFilterContext 
    out_audio_filter*: AVFilterContext 
    agraph*: AVFilterGraph  
    last_video_stream*: cint
    last_audio_stream*: cint
    last_subtitle_stream*: cint
    # continue_read_thread*: ptr SDL_cond
  
  AVlinkEnum* = enum
    AVLINK_UNINIT = 0,          
    AVLINK_STARTINIT,         
    AVLINK_INIT               

  AVFilterFormats* = ref object
    nb_formats*: cuint         
    formats*: cint          
    refcount*: cuint           
    refs*: ptr ptr AVFilterFormats 

  AVFilterChannelLayouts* = ref object
    channel_layouts*: uint64 
    nb_channel_layouts*: cint  
    all_layouts*: char         
    all_counts*: char          
    refcount*: cuint           
    refs*: AVFilterChannelLayouts 


  AVFilterFormatsConfig* = ref object
    formats*: AVFilterFormats
    samplerates*: AVFilterFormats
    channel_layouts*: AVFilterChannelLayouts

  FFFrameBucket* = ref object
    frame*: AVFrame

  FFFrameQueue* = ref object
    queue*: ptr FFFrameBucket
    allocated*: csize_t
    tail*: csize_t
    queued*: csize_t
    first_bucket*: FFFrameBucket
    total_frames_head*: uint64
    total_frames_tail*: uint64
    total_samples_head*: uint64
    total_samples_tail*: uint64
    samples_skipped*: cint



  AVFilterLink* = ref object
    src*: AVFilterContext   
    srcpad*: AVFilterPad   
    dst*: AVFilterContext   
    dstpad*: AVFilterPad   
    t*: AVMediaType       
    w*: cint                   
    h*: cint                   
    sample_aspect_ratio*: AVRational 
    channel_layout*: uint64  
    sample_rate*: cint         
    format*: cint              
    time_base*: AVRational
    incfg*: AVFilterFormatsConfig
    outcfg*: AVFilterFormatsConfig
    init_state*: AVlinkEnum
    graph*: AVFilterGraph
    current_pts*: int64
    current_pts_us*: int64
    age_index*: cint
    frame_rate*: AVRational
    partial_buf*: AVFrame
    partial_buf_size*: cint
    min_samples*: cint
    max_samples*: cint
    channels*: cint
    flags*: cuint
    frame_count_in*: int64
    frame_count_out*: int64
    frame_pool*: pointer
    frame_wanted_out*: cint
    hw_frames_ctx*: AVBufferRef
    fifo*: FFFrameQueue
    frame_blocked_in*: cint
    status_in*: cint
    status_in_pts*: int64
    status_out*: cint

  AVFilterPad* = ref object
    name*: string
    t*: AVMediaType
    get_video_buffer*: proc (link: AVFilterLink; w: cint; h: cint): AVFrame
    get_audio_buffer*: proc (link: AVFilterLink; nb_samples: cint): AVFrame
    filter_frame*: proc (link: AVFilterLink; frame: AVFrame): cint
    request_frame*: proc (link: AVFilterLink): cint
    config_props*: proc (link: AVFilterLink): cint
    needs_writable*: cint

  AVFilter* = ref object
    name*: string
    description*: string
    inputs*: ptr AVFilterPad
    outputs*: ptr AVFilterPad
    priv_class*: AVClass
    flags*: cint
    preinit*: proc (ctx: AVFilterContext): cint
    init*: proc (ctx: AVFilterContext): cint
    init_dict*: proc (ctx: AVFilterContext; options: Table[string,string]): cint
    uninit*: proc (ctx: AVFilterContext)
    query_formats*: proc (a1: AVFilterContext): cint
    priv_size*: cint           
    flags_internal*: cint      
    next*: ptr AVFilter
    process_command*: proc (a1: AVFilterContext; cmd: string; arg: string;res: string; res_len: cint; flags: cint): cint
    init_opaque*: proc (ctx: AVFilterContext; opaque: pointer): cint
    activate*: proc (ctx: AVFilterContext): cint

  AVFilterInternal* = ref object
    execute*: ptr avfilter_execute_func

  AVFilterCommand* = ref object
    time*: cdouble             
    command*: string          
    arg*: string              
    flags*: cint
    next*: ptr AVFilterCommand

  AVFilterContext* = ref object
    av_class*: AVClass      
    filter*: ptr AVFilter       
    name*: string             
    input_pads*: AVFilterPad
    inputs*: ptr AVFilterLink 
    nb_inputs*: cuint          
    output_pads*: AVFilterPad
    outputs*: ptr AVFilterLink 
    nb_outputs*: cuint         
    priv*: pointer             
    graph*: AVFilterGraph   
    thread_type*: cint
    internal*: ptr AVFilterInternal
    command_queue*: ptr AVFilterCommand
    enable_str*: string       
    enable*: pointer           
    var_values*: cdouble    
    is_disabled*: cint         
    hw_device_ctx*: AVBufferRef
    nb_threads*: cint
    ready*: cuint
    extra_hw_frames*: cint

  URLContext*  = object
    av_class*: AVClass      
    prot*: URLProtocol
    priv_data*: pointer
    filename*: string         
    flags*: cint
    max_packet_size*: cint     
    is_streamed*: cint         
    is_connected*: cint
    interrupt_callback*: AVIOInterruptCB
    rw_timeout*: int64       
    protocol_whitelist*: string
    protocol_blacklist*: string
    min_packet_size*: cint     


  AVIODirEntry* = ref object
    name*: string             
    t*: cint              
    utf8*: cint 
    size*: int64             
    modification_timestamp*: int64 
    access_timestamp*: int64 
    status_change_timestamp*: int64 
    user_id*: int64          
    group_id*: int64         
    filemode*: int64         


  URLProtocol* = ref object
    name*: string
    url_open*: proc (h: URLContext; url: string; flags: cint): cint
    url_open2*: proc (h: URLContext; url: string; flags: cint; options: TableRef[string,string]): cint
    url_accept*: proc (s: URLContext; c: URLContext): cint
    url_handshake*: proc (c: URLContext): cint
    url_read*: proc (h: URLContext; buf: ptr cuchar; size: cint): cint
    url_write*: proc (h: URLContext; buf: ptr cuchar; size: cint): cint
    url_seek*: proc (h: URLContext; pos: int64; whence: cint): int64
    url_close*: proc (h: URLContext): cint
    url_read_pause*: proc (h: URLContext; pause: cint): cint
    url_read_seek*: proc (h: URLContext; stream_index: cint; timestamp: int64;flags: cint): int64
    url_get_file_handle*: proc (h: URLContext): cint
    url_get_multi_file_handle*: proc (h: URLContext; handles: cint;numhandles: cint): cint
    url_get_short_seek*: proc (h: URLContext): cint
    url_shutdown*: proc (h: URLContext; flags: cint): cint
    priv_data_size*: cint
    priv_data_class*: AVClass
    flags*: cint
    url_check*: proc (h: URLContext; mask: cint): cint
    url_open_dir*: proc (h: URLContext): cint
    url_read_dir*: proc (h: URLContext; next: AVIODirEntry): cint
    url_close_dir*: proc (h: URLContext): cint
    url_delete*: proc (h: URLContext): cint
    url_move*: proc (h_src: URLContext; h_dst: URLContext): cint
    default_whitelist*: string

  DirectShowCtx* = ref object
    class*: AVClass
    # graph*: IGraphBuilder
    device_name*: array[2, string]
    device_unique_name*: array[2, string]
    video_device_number*: cint
    audio_device_number*: cint
    list_options*: cint
    list_devices*: cint
    audio_buffer_size*: cint
    crossbar_video_input_pin_number*: cint
    crossbar_audio_input_pin_number*: cint
    video_pin_name*: string
    audio_pin_name*: string
    show_video_device_dialog*: cint
    show_audio_device_dialog*: cint
    show_video_crossbar_connection_dialog*: cint
    show_audio_crossbar_connection_dialog*: cint
    show_analog_tv_tuner_dialog*: cint
    show_analog_tv_tuner_audio_dialog*: cint
    audio_filter_load_file*: string
    audio_filter_save_file*: string
    video_filter_load_file*: string
    video_filter_save_file*: string
    # device_filter*: array[2, ptr IBaseFilter]
    # device_pin*: array[2, ptr IPin]
    # capture_filter*: array[2, ptr libAVFilter]
    # capture_pin*: array[2, ptr libAVPin]
    mutex*: HANDLE
    event*: array[2, HANDLE]    
    pktl*: AVPacketList
    eof*: cint
    curbufsize*: array[2, int64]
    video_frame_num*: cuint
    # control*: ptr IMediaControl
    # media_event*: ptr IMediaEvent
    pixel_format*: AVPixelFormat
    video_codec_id*: AVCodecID
    framerate*: string
    requested_width*: cint
    requested_height*: cint
    requested_framerate*: AVRational
    sample_rate*: cint
    sample_size*: cint
    channels*: cint
  
  X264Context* = ref object
    class*: AVClass
    params*: x264_param_t
    enc*: x264_t
    pic*: x264_picture_t
    sei*: uint8
    sei_size*: cint
    preset*: string
    tune*: string
    profile*: string
    level*: string
    fastfirstpass*: cint
    wpredp*: string
    x264opts*: string
    crf*: cfloat
    crf_max*: cfloat
    cqp*: cint
    aq_mode*: cint
    aq_strength*: cfloat
    psy_rd*: string
    psy*: cint
    rc_lookahead*: cint
    weightp*: cint
    weightb*: cint
    ssim*: cint
    intra_refresh*: cint
    bluray_compat*: cint
    b_bias*: cint
    b_pyramid*: cint
    mixed_refs*: cint
    dct8x8*: cint
    fast_pskip*: cint
    aud*: cint
    mbtree*: cint
    deblock*: string
    cplxblur*: cfloat
    partitions*: string
    direct_pred*: cint
    slice_max_size*: cint
    stats*: string
    nal_hrd*: cint
    avcintra_class*: cint
    motion_est*: cint
    forced_idr*: cint
    coder*: cint
    a53_cc*: cint
    b_frame_strategy*: cint
    chroma_offset*: cint
    scenechange_threshold*: cint
    noise_reduction*: cint
    x264_params*: Table[string,string]
    nb_reordered_opaque*: cint
    next_reordered_opaque*: cint
    reordered_opaque*: X264Opaque
    roi_warned*: cint

  AVCPBProperties*  = object
    max_bitrate*: cint
    min_bitrate*: cint
    avg_bitrate*: cint
    buffer_size*: cint
    vbv_delay*: uint64

  x264_cpu_name_t* = tuple
    name: string
    flags: int

var sws_dict*: Table[string,string]
var swr_opts*: Table[string,string]
var
  format_opts*: Table[string,string]
  codec_opts*: Table[string,string]
  resample_opts*: Table[string,string]


proc init_dynload() = 
  echo SetDllDirectory("")

# proc dshow_read_header*(avctx: AVFormatContext): cint =
#   var ctx: DirectShowCtx = cast[DirectShowCtx](avctx.priv_data)
#   var graph: IGraphBuilder
#   var devenum: ICreateDevEnum
#   var control: IMediaControl
#   var media_event: IMediaEvent
#   var media_event_handle: HANDLE
#   var `proc`: HANDLE
#   var ret: cint = EIO
#   var r: cint
#   CoInitialize(0)
#   if not ctx.list_devices and not parse_device_name(avctx):
#     echo("Malformed dshow input string.\n")
#     goto error
#   ctx.video_codec_id = if avctx.video_codec_id: avctx.video_codec_id else: AV_CODEC_ID_RAWVIDEO
#   if ctx.pixel_format != AV_PIX_FMT_NONE:
#     if ctx.video_codec_id != AV_CODEC_ID_RAWVIDEO:
#       echo("Pixel format may only be set when video codec is not set or set to rawvideo\n")
#       ret = EINVAL
#       goto error
#   if ctx.framerate:
#     r = av_parse_video_rate(ctx.requested_framerate, ctx.framerate)
#   r = CoCreateInstance(CLSID_FilterGraph, nil, CLSCTX_INPROC_SERVER,IID_IGraphBuilder, cast[ptr pointer](graph))
#   ctx.graph = graph
#   r = CoCreateInstance(CLSID_SystemDeviceEnum, nil, CLSCTX_INPROC_SERVER,IID_ICreateDevEnum, cast[ptr pointer](devenum))
#   if ctx.list_devices:
#     echo(  "DirectShow video devices (some may be both video and audio devices)\n")
#     dshow_cycle_devices(avctx, devenum, VideoDevice, VideoSourceDevice, nil, nil)
#     echo(  "DirectShow audio devices\n")
#     dshow_cycle_devices(avctx, devenum, AudioDevice, AudioSourceDevice, nil, nil)
#     ret = AVERROR_EXIT
#     goto error
#   if ctx.list_options:
#     if ctx.device_name[VideoDevice]:
#       if (r = dshow_list_device_options(avctx, devenum, VideoDevice, VideoSourceDevice)):
#         ret = r
#         goto error
#     if ctx.device_name[AudioDevice]:
#       if dshow_list_device_options(avctx, devenum, AudioDevice, AudioSourceDevice):
#         if (r = dshow_list_device_options(avctx, devenum, AudioDevice, VideoSourceDevice)):
#           ret = r
#           goto error
#   if ctx.device_name[VideoDevice]:
#     if (r = dshow_open_device(avctx, devenum, VideoDevice, VideoSourceDevice)) < 0 or
#         (r = dshow_add_device(avctx, VideoDevice)) < 0:
#       ret = r
#       goto error
#   if ctx.device_name[AudioDevice]:
#     if (r = dshow_open_device(avctx, devenum, AudioDevice, AudioSourceDevice)) < 0 or
#         (r = dshow_add_device(avctx, AudioDevice)) < 0:
#       echo( 
#              "Searching for audio device within video devices for %s\n",
#              ctx.device_name[AudioDevice])
#       if (r = dshow_open_device(avctx, devenum, AudioDevice, VideoSourceDevice)) < 0 or
#           (r = dshow_add_device(avctx, AudioDevice)) < 0:
#         ret = r
#         goto error
#   if ctx.list_options:
#     ret = AVERROR_EXIT
#     goto error
#   ctx.curbufsize[0] = 0
#   ctx.curbufsize[1] = 0
#   ctx.mutex = CreateMutex(nil, 0, nil)
#   ctx.event[1] = CreateEvent(nil, 1, 0, nil)
#   r = IGraphBuilder_QueryInterface(graph, IID_IMediaControl, cast[ptr pointer](control))
#   ctx.control = control
#   r = IGraphBuilder_QueryInterface(graph, IID_IMediaEvent, cast[ptr pointer](media_event))
#   ctx.media_event = media_event
#   r = IMediaEvent_GetEventHandle(media_event, cast[pointer](media_event_handle))
#   `proc` = GetCurrentProcess()
#   r = DuplicateHandle(`proc`, media_event_handle, `proc`, ctx.event[0], 0, 0, DUPLICATE_SAME_ACCESS)
#   r = IMediaControl_Run(control)
#   if r == S_FALSE:
#     var pfs: OAFilterState
#     r = IMediaControl_GetState(control, 0, pfs)
#   ret = 0
#   if devenum:
#     ICreateDevEnum_Release(devenum)
#   if ret < 0:
#     dshow_read_close(avctx)
#   return ret

# proc dshow_check_event_queue*(media_event: ptr IMediaEvent): cint =
#   var
#     p1: LONG_PTR
#     p2: LONG_PTR
#   var code: clong
#   var ret: cint = 0
#   while IMediaEvent_GetEvent(media_event, code, p1, p2, 0) !=
#       E_ABORT:
#     if code == EC_COMPLETE or code == EC_DEVICE_LOST or code == EC_ERRORABORT:
#       ret = -1
#     IMediaEvent_FreeEventParams(media_event, code, p1, p2)
#   return ret

# proc dshow_read_packet*(s: AVFormatContext; pkt: AVPacket): cint =
#   var ctx: DirectShowCtx = cast[DirectShowCtx](s.priv_data)
#   var pktl: AVPacketList 
#   while not ctx.eof and not pktl:
#     WaitForSingleObject(ctx.mutex, INFINITE)
#     pktl = ctx.pktl
#     if pktl:
#       pkt[] = pktl.pkt
#       ctx.pktl = ctx.pktl.next
#       dec(ctx.curbufsize[pkt.stream_index], pkt.size)
#     ResetEvent(ctx.event[1])
#     ReleaseMutex(ctx.mutex)
#     if not pktl:
#       if dshow_check_event_queue(ctx.media_event) < 0:
#         ctx.eof = 1
#       elif s.flags and AVFMT_FLAG_NONBLOCK:
#         return EAGAIN
#       else:
#         WaitForMultipleObjects(2, ctx.event, 0, INFINITE)
#   return if ctx.eof: EIO else: pkt.size

# var dshow_class_option: seq[AVOption] = @[
#   AVOption("video_size", "set video size given a string such as 640x480 or hd720.", DirectShowCtx.requested_width, AV_OPT_TYPE_IMAGE_SIZE, {.str = NULL}, 0, 0, DEC ),
#   AVOption("pixel_format", "set video pixel format", DirectShowCtx.pixel_format, AV_OPT_TYPE_PIXEL_FMT, {.i64 = AV_PIX_FMT_NONE}, -1, INT_MAX, DEC),
#   AVOption("framerate", "set video frame rate", DirectShowCtx.framerate, AV_OPT_TYPE_STRING, {.str = NULL}, 0, 0, DEC),
#   AVOption("sample_rate", "set audio sample rate", DirectShowCtx.sample_rate, AV_OPT_TYPE_INT, {.i64 = 0}, 0, INT_MAX, DEC),
#   AVOption("sample_size", "set audio sample size", DirectShowCtx.sample_size, AV_OPT_TYPE_INT, {.i64 = 0}, 0, 16, DEC),
#   AVOption("channels", "set number of audio channels, such as 1 or 2", DirectShowCtx.channels, AV_OPT_TYPE_INT, {.i64 = 0}, 0, INT_MAX, DEC),
#   AVOption("audio_buffer_size", "set audio device buffer latency size in milliseconds (default is the device's default)", DirectShowCtx.audio_buffer_size, AV_OPT_TYPE_INT, {.i64 = 0}, 0, INT_MAX, DEC),
#   AVOption("list_devices", "list available devices",                      DirectShowCtx.list_devices, AV_OPT_TYPE_BOOL, {.i64=0}, 0, 1, DEC),
#   AVOption("list_options", "list available options for specified device", DirectShowCtx.list_options, AV_OPT_TYPE_BOOL, {.i64=0}, 0, 1, DEC),
#   AVOption("video_device_number", "set video device number for devices with same name (starts at 0)", DirectShowCtx.video_device_number, AV_OPT_TYPE_INT, {.i64 = 0}, 0, INT_MAX, DEC),
#   AVOption("audio_device_number", "set audio device number for devices with same name (starts at 0)", DirectShowCtx.audio_device_number, AV_OPT_TYPE_INT, {.i64 = 0}, 0, INT_MAX, DEC),
#   AVOption("video_pin_name", "select video capture pin by name", DirectShowCtx.video_pin_name,AV_OPT_TYPE_STRING, {.str = NULL},  0, 0, AV_OPT_FLAG_ENCODING_PARAM }),
#   AVOption("audio_pin_name", "select audio capture pin by name", DirectShowCtx.audio_pin_name,AV_OPT_TYPE_STRING, {.str = NULL},  0, 0, AV_OPT_FLAG_ENCODING_PARAM }),
#   AVOption("crossbar_video_input_pin_number", "set video input pin number for crossbar device", DirectShowCtx.crossbar_video_input_pin_number, AV_OPT_TYPE_INT, {.i64 = -1}, -1, INT_MAX, DEC),
#   AVOption("crossbar_audio_input_pin_number", "set audio input pin number for crossbar device", DirectShowCtx.crossbar_audio_input_pin_number, AV_OPT_TYPE_INT, {.i64 = -1}, -1, INT_MAX, DEC),
#   AVOption("show_video_device_dialog",              "display property dialog for video capture device",                            DirectShowCtx.show_video_device_dialog,              AV_OPT_TYPE_BOOL, {.i64 = 0}, 0, 1, DEC),
#   AVOption("show_audio_device_dialog",              "display property dialog for audio capture device",                            DirectShowCtx.show_audio_device_dialog,              AV_OPT_TYPE_BOOL, {.i64 = 0}, 0, 1, DEC),
#   AVOption("show_video_crossbar_connection_dialog", "display property dialog for crossbar connecting pins filter on video device", DirectShowCtx.show_video_crossbar_connection_dialog, AV_OPT_TYPE_BOOL, {.i64 = 0}, 0, 1, DEC),
#   AVOption("show_audio_crossbar_connection_dialog", "display property dialog for crossbar connecting pins filter on audio device", DirectShowCtx.show_audio_crossbar_connection_dialog, AV_OPT_TYPE_BOOL, {.i64 = 0}, 0, 1, DEC),
#   AVOption("show_analog_tv_tuner_dialog",           "display property dialog for analog tuner filter",                             DirectShowCtx.show_analog_tv_tuner_dialog,           AV_OPT_TYPE_BOOL, {.i64 = 0}, 0, 1, DEC),
#   AVOption("show_analog_tv_tuner_audio_dialog",     "display property dialog for analog tuner audio filter",                       DirectShowCtx.show_analog_tv_tuner_audio_dialog,     AV_OPT_TYPE_BOOL, {.i64 = 0}, 0, 1, DEC),
#   AVOption("audio_device_load", "load audio capture filter device (and properties) from file", DirectShowCtx.audio_filter_load_file, AV_OPT_TYPE_STRING, {.str = NULL}, 0, 0, DEC),
#   AVOption("audio_device_save", "save audio capture filter device (and properties) to file", DirectShowCtx.audio_filter_save_file, AV_OPT_TYPE_STRING, {.str = NULL}, 0, 0, DEC),
#   AVOption("video_device_load", "load video capture filter device (and properties) from file", DirectShowCtx.video_filter_load_file, AV_OPT_TYPE_STRING, {.str = NULL}, 0, 0, DEC),
#   AVOption("video_device_save", "save video capture filter device (and properties) to file", DirectShowCtx.video_filter_save_file, AV_OPT_TYPE_STRING, {.str = NULL}, 0, 0, DEC),
# ]

# var dshow_class = AVClass(class_name:"dshow indev",option:dshow_class_option,
#   version:LIBAVUTIL_VERSION_INT, category:AV_CLASS_CATEGORY_DEVICE_VIDEO_INPUT)

# var ff_dshow_demuxer = AVInputFormat(name:"dshow",long_name:"DirectShow capture",priv_data_size:sizeof DirectShowCtx
#   read_header:dshow_read_header, read_packet:dshow_read_packet,read_close:dshow_read_close,flags:AVFMT_NOFILE,priv_class:dshow_class)

var indev_list*: seq[AVInputFormat] 
var outdev_list*: seq[AVOutputFormat]
var muxer_list: seq[AVOutputFormat]

proc init_get_bits_xe*(s: var GetBitContext; buffer: uint8; bit_size: var cint;is_le: cint): cint {.inline.} =
  var buffer_size: cint
  var ret: cint = 0
  if bit_size >= INT_MAX - max(7, AV_INPUT_BUFFER_PADDING_SIZE * 8) or bit_size < 0:
    bit_size = 0
    ret = AVERROR_INVALIDDATA
  buffer_size = (bit_size + 7) shr 3
  s.buffer = buffer
  s.size_in_bits = bit_size
  s.size_in_bits_plus8 = bit_size + 8
  s.buffer_end = buffer + buffer_size.uint8
  s.index = 0

  return ret


proc init_get_bits*(s: var GetBitContext; buffer: uint8; bit_size: var cint): cint {.
    inline.} =
  return init_get_bits_xe(s, buffer, bit_size, 0)

proc init_get_bits8*(s: var GetBitContext; buffer: uint8; byte_size: var cint): cint {.inline.} =
  if byte_size > INT_MAX div 8 or byte_size < 0:
    byte_size = -1
  var bitSize = byte_size * 8
  return init_get_bits(s, buffer, bitSize)

proc init_get_bits8_le*(s: var GetBitContext; buffer: uint8; byte_size:var cint): cint {.inline.} =
  if byte_size > INT_MAX div 8 or byte_size < 0:
    byte_size = -1
  var bs = byte_size * 8
  return init_get_bits_xe(s, buffer, bs, 1)


proc skip_bits*(s: var GetBitContext; n: cint) {.inline.} =
  var re_index: cuint = s.index.cuint
  var re_cache: cuint
  var re_size_plus8: cuint = s.size_in_bits_plus8.cuint
  re_index = min(re_size_plus8, re_index + n.cuint)
  s.index = re_index.cint


proc context_to_name*(p: pointer): string =
  var avc: AVCodecContext = cast[AVCodecContext](p)
  return avc.codec.name

proc codec_child_next*(obj: pointer; prev: pointer): pointer =
  var s: AVCodecContext = cast[AVCodecContext](obj)
  return s.priv_data



var av_codec_static_init*: Pthread_once

proc x264_cpu_detect(): uint32 =  0'u32

proc x264_param_default*(param: x264_param_t) =
  ##  CPU autodetect
  param.cpu = x264_cpu_detect()
  param.i_threads = X264_THREADS_AUTO
  param.i_lookahead_threads = X264_THREADS_AUTO
  param.b_deterministic = 1
  param.i_sync_lookahead = X264_SYNC_LOOKAHEAD_AUTO
  ##  Video properties
  param.i_csp = X264_CSP_I420
  param.i_width = 0
  param.i_height = 0
  param.vui.i_sar_width = 0
  param.vui.i_sar_height = 0
  param.vui.i_overscan = 0
  ##  undef
  param.vui.i_vidformat = 5
  ##  undef
  param.vui.b_fullrange = -1
  ##  default depends on input
  param.vui.i_colorprim = 2
  ##  undef
  param.vui.i_transfer = 2
  ##  undef
  param.vui.i_colmatrix = -1
  ##  default depends on input
  param.vui.i_chroma_loc = 0
  ##  left center
  param.i_fps_num = 25
  param.i_fps_den = 1
  param.i_level_idc = -1
  param.i_slice_max_size = 0
  param.i_slice_max_mbs = 0
  param.i_slice_count = 0
  param.i_bitdepth = 8
  ##  Encoder parameters
  param.i_frame_reference = 3
  param.i_keyint_max = 250
  param.i_keyint_min = X264_KEYINT_MIN_AUTO
  param.i_bframe = 3
  param.i_scenecut_threshold = 40
  param.i_bframe_adaptive = X264_B_ADAPT_FAST
  param.i_bframe_bias = 0
  param.i_bframe_pyramid = X264_B_PYRAMID_NORMAL
  param.b_interlaced = 0
  param.b_constrained_intra = 0
  param.b_deblocking_filter = 1
  param.i_deblocking_filter_alphac0 = 0
  param.i_deblocking_filter_beta = 0
  param.b_cabac = 1
  param.i_cabac_init_idc = 0
  param.rc.i_rc_method = X264_RC_CRF
  param.rc.i_bitrate = 0
  param.rc.f_rate_tolerance = 1.0
  param.rc.i_vbv_max_bitrate = 0
  param.rc.i_vbv_buffer_size = 0
  param.rc.f_vbv_buffer_init = 0.9
  param.rc.i_qp_constant = -1
  param.rc.f_rf_constant = 23
  param.rc.i_qp_min = 0
  param.rc.i_qp_max = INT_MAX
  param.rc.i_qp_step = 4
  param.rc.f_ip_factor = 1.4
  param.rc.f_pb_factor = 1.3
  param.rc.i_aq_mode = X264_AQ_VARIANCE
  param.rc.f_aq_strength = 1.0
  param.rc.i_lookahead = 40
  param.rc.b_stat_write = 0
  param.rc.psz_stat_out = "x264_2pass.log"
  param.rc.b_stat_read = 0
  param.rc.psz_stat_in = "x264_2pass.log"
  param.rc.f_qcompress = 0.6
  param.rc.f_qblur = 0.5
  param.rc.f_complexity_blur = 20
  param.rc.i_zones = 0
  param.rc.b_mb_tree = 1
  ##  Log
  # param.pf_log = x264_log_default
  param.p_log_private = nil
  # param.i_log_level = X264_LOG_INFO
  param.analyse.intra = X264_ANALYSE_I4x4 or X264_ANALYSE_I8x8
  param.analyse.inter = X264_ANALYSE_I4x4 or X264_ANALYSE_I8x8 or X264_ANALYSE_PSUB16x16 or X264_ANALYSE_BSUB16x16
  param.analyse.i_direct_mv_pred = X264_DIRECT_PRED_SPATIAL
  param.analyse.i_me_method = X264_ME_HEX
  param.analyse.f_psy_rd = 1.0
  param.analyse.b_psy = 1
  param.analyse.f_psy_trellis = 0
  param.analyse.i_me_range = 16
  param.analyse.i_subpel_refine = 7
  param.analyse.b_mixed_references = 1
  param.analyse.b_chroma_me = 1
  param.analyse.i_mv_range_thread = -1
  param.analyse.i_mv_range = -1
  ##  set from level_idc
  param.analyse.i_chroma_qp_offset = 0
  param.analyse.b_fast_pskip = 1
  param.analyse.b_weighted_bipred = 1
  param.analyse.i_weighted_pred = X264_WEIGHTP_SMART
  param.analyse.b_dct_decimate = 1
  param.analyse.b_transform_8x8 = 1
  param.analyse.i_trellis = 1
  param.analyse.i_luma_deadzone[0] = 21
  param.analyse.i_luma_deadzone[1] = 11
  param.analyse.b_psnr = 0
  param.analyse.b_ssim = 0
  param.i_cqm_preset = X264_CQM_FLAT

  param.b_repeat_headers = 1
  param.b_annexb = 1
  param.b_aud = 0
  param.b_vfr_input = 1
  param.i_nal_hrd = 0
  param.b_tff = 1
  param.b_pic_struct = 0
  param.b_fake_interlaced = 0
  param.i_frame_packing = -1
  param.i_alternative_transfer = 2
  ##  undef
  param.b_opencl = 0
  param.i_opencl_device = 0
  param.opencl_device_id = nil
  param.psz_clbin_file = nil
  param.i_avcintra_class = 0
  param.i_avcintra_flavor = X264_AVCINTRA_FLAVOR_PANASONIC

var x264_preset_names*: seq[string] = @["ultrafast", "superfast","veryfast", "faster", "fast", "medium", "slow", "slower", "veryslow", "placebo"]


proc param_apply_preset*(param: var x264_param_t; preset: string): cint =
  var i: cint = cint parseInt($preset)
  var name: string = x264_preset_names[i]
  if name ==  "ultrafast":
    param.i_frame_reference = 1
    param.i_scenecut_threshold = 0
    param.b_deblocking_filter = 0
    param.b_cabac = 0
    param.i_bframe = 0
    param.analyse.intra = 0
    param.analyse.inter = 0
    param.analyse.b_transform_8x8 = 0
    param.analyse.i_me_method = X264_ME_DIA
    param.analyse.i_subpel_refine = 0
    param.rc.i_aq_mode = 0
    param.analyse.b_mixed_references = 0
    param.analyse.i_trellis = 0
    param.i_bframe_adaptive = X264_B_ADAPT_NONE
    param.rc.b_mb_tree = 0
    param.analyse.i_weighted_pred = X264_WEIGHTP_NONE
    param.analyse.b_weighted_bipred = 0
    param.rc.i_lookahead = 0
  elif name ==  "superfast":
    param.analyse.inter = X264_ANALYSE_I8x8 or X264_ANALYSE_I4x4
    param.analyse.i_me_method = X264_ME_DIA
    param.analyse.i_subpel_refine = 1
    param.i_frame_reference = 1
    param.analyse.b_mixed_references = 0
    param.analyse.i_trellis = 0
    param.rc.b_mb_tree = 0
    param.analyse.i_weighted_pred = X264_WEIGHTP_SIMPLE
    param.rc.i_lookahead = 0
  elif name ==  "veryfast":
    param.analyse.i_subpel_refine = 2
    param.i_frame_reference = 1
    param.analyse.b_mixed_references = 0
    param.analyse.i_trellis = 0
    param.analyse.i_weighted_pred = X264_WEIGHTP_SIMPLE
    param.rc.i_lookahead = 10
  elif name ==  "faster":
    param.analyse.b_mixed_references = 0
    param.i_frame_reference = 2
    param.analyse.i_subpel_refine = 4
    param.analyse.i_weighted_pred = X264_WEIGHTP_SIMPLE
    param.rc.i_lookahead = 20
  elif name ==  "fast":
    param.i_frame_reference = 2
    param.analyse.i_subpel_refine = 6
    param.analyse.i_weighted_pred = X264_WEIGHTP_SIMPLE
    param.rc.i_lookahead = 30
  elif name ==  "medium":
    ##  Default is medium
  elif name ==  "slow":
    param.analyse.i_subpel_refine = 8
    param.i_frame_reference = 5
    param.analyse.i_direct_mv_pred = X264_DIRECT_PRED_AUTO
    param.analyse.i_trellis = 2
    param.rc.i_lookahead = 50
  elif name ==  "slower":
    param.analyse.i_me_method = X264_ME_UMH
    param.analyse.i_subpel_refine = 9
    param.i_frame_reference = 8
    param.i_bframe_adaptive = X264_B_ADAPT_TRELLIS
    param.analyse.i_direct_mv_pred = X264_DIRECT_PRED_AUTO
    param.analyse.inter = param.analyse.inter or X264_ANALYSE_PSUB8x8
    param.analyse.i_trellis = 2
    param.rc.i_lookahead = 60
  elif name ==  "veryslow":
    param.analyse.i_me_method = X264_ME_UMH
    param.analyse.i_subpel_refine = 10
    param.analyse.i_me_range = 24
    param.i_frame_reference = 16
    param.i_bframe_adaptive = X264_B_ADAPT_TRELLIS
    param.analyse.i_direct_mv_pred = X264_DIRECT_PRED_AUTO
    param.analyse.inter = param.analyse.inter or X264_ANALYSE_PSUB8x8
    param.analyse.i_trellis = 2
    param.i_bframe = 8
    param.rc.i_lookahead = 60
  elif name ==  "placebo":
    param.analyse.i_me_method = X264_ME_TESA
    param.analyse.i_subpel_refine = 11
    param.analyse.i_me_range = 24
    param.i_frame_reference = 16
    param.i_bframe_adaptive = X264_B_ADAPT_TRELLIS
    param.analyse.i_direct_mv_pred = X264_DIRECT_PRED_AUTO
    param.analyse.inter = param.analyse.inter or X264_ANALYSE_PSUB8x8
    param.analyse.b_fast_pskip = 0
    param.analyse.i_trellis = 2
    param.i_bframe = 16
    param.rc.i_lookahead = 60
  else:
    echo("invalid preset \'%s\'\n", preset)
    return -1
  return 0

proc param_apply_tune*(param: var x264_param_t; tune: string): cint =
  var psy_tuning_used: cint = 0
  var len: cint

  if len == 4 and tune == "film":

    param.i_deblocking_filter_alphac0 = -1
    param.i_deblocking_filter_beta = -1
    param.analyse.f_psy_trellis = 0.15
  elif len == 9 and tune == "animation":

    param.i_frame_reference = if param.i_frame_reference > 1: param.i_frame_reference * 2 else: 1
    param.i_deblocking_filter_alphac0 = 1
    param.i_deblocking_filter_beta = 1
    param.analyse.f_psy_rd = 0.4
    param.rc.f_aq_strength = 0.6
    inc(param.i_bframe, 2)
  elif len == 5 and tune == "grain":

    param.i_deblocking_filter_alphac0 = -2
    param.i_deblocking_filter_beta = -2
    param.analyse.f_psy_trellis = 0.25
    param.analyse.b_dct_decimate = 0
    param.rc.f_pb_factor = 1.1
    param.rc.f_ip_factor = 1.1
    param.rc.f_aq_strength = 0.5
    param.analyse.i_luma_deadzone[0] = 6
    param.analyse.i_luma_deadzone[1] = 6
    param.rc.f_qcompress = 0.8
  elif len == 10 and tune == "stillimage":

    param.i_deblocking_filter_alphac0 = -3
    param.i_deblocking_filter_beta = -3
    param.analyse.f_psy_rd = 2.0
    param.analyse.f_psy_trellis = 0.7
    param.rc.f_aq_strength = 1.2
  elif len == 4 and tune == "psnr":

    param.rc.i_aq_mode = X264_AQ_NONE
    param.analyse.b_psy = 0
  elif len == 4 and tune == "ssim":

    param.rc.i_aq_mode = X264_AQ_AUTOVARIANCE
    param.analyse.b_psy = 0
  elif len == 10 and tune == "fastdecode":
    param.b_deblocking_filter = 0
    param.b_cabac = 0
    param.analyse.b_weighted_bipred = 0
    param.analyse.i_weighted_pred = X264_WEIGHTP_NONE
  elif len == 11 and tune == "zerolatency":
    param.rc.i_lookahead = 0
    param.i_sync_lookahead = 0
    param.i_bframe = 0
    param.b_sliced_threads = 1
    param.b_vfr_input = 0
    param.rc.b_mb_tree = 0
  elif len == 6 and tune == "touhou":

    param.i_frame_reference = if param.i_frame_reference > 1: param.i_frame_reference *
        2 else: 1
    param.i_deblocking_filter_alphac0 = -1
    param.i_deblocking_filter_beta = -1
    param.analyse.f_psy_trellis = 0.2
    param.rc.f_aq_strength = 1.3
    param.analyse.inter = param.analyse.inter or X264_ANALYSE_PSUB8x8
  else:
    echo("invalid tune \'%.*s\'\n", len, tune)
    return -1
    echo("only 1 psy tuning can be used: ignoring tune %.*s\n",len, tune)
  return 0


proc x264_param_default_preset*(param:var x264_param_t; preset: string;tune: string): cint =
  x264_param_default(param)
  if param_apply_preset(param, preset) < 0:
    return -1
  if param_apply_tune(param, tune) < 0:
    return -1
  return 0

var x264_tune_names*: seq[string] = @["film", "animation", "grain","stillimage", "psnr", "ssim", "fastdecode", "zerolatency"]

proc convert_pix_fmt*(pix_fmt: AVPixelFormat): cint =
  case pix_fmt
  of AV_PIX_FMT_YUV420P, AV_PIX_FMT_YUVJ420P, AV_PIX_FMT_YUV420P9,
    AV_PIX_FMT_YUV420P10:
    return X264_CSP_I420
  of AV_PIX_FMT_YUV422P, AV_PIX_FMT_YUVJ422P, AV_PIX_FMT_YUV422P10:
    return X264_CSP_I422
  of AV_PIX_FMT_YUV444P, AV_PIX_FMT_YUVJ444P, AV_PIX_FMT_YUV444P9,
    AV_PIX_FMT_YUV444P10:
    return X264_CSP_I444
  of AV_PIX_FMT_BGR0:
    return X264_CSP_BGRA
  of AV_PIX_FMT_BGR24:
    return X264_CSP_BGR
  of AV_PIX_FMT_RGB24:
    return X264_CSP_RGB
  of AV_PIX_FMT_NV12:
    return X264_CSP_NV12
  of AV_PIX_FMT_NV16, AV_PIX_FMT_NV20:
    return X264_CSP_NV16
  of AV_PIX_FMT_NV21:
    return X264_CSP_NV21
  of AV_PIX_FMT_GRAY8, AV_PIX_FMT_GRAY10:
    return X264_CSP_I400
  else:
    return 0

var  av_pix_fmt_descriptors: array[AV_PIX_FMT_NB.int, AVPixFmtDescriptor]
av_pix_fmt_descriptors[AV_PIX_FMT_YUV420P.int] = AVPixFmtDescriptor(name:"yuv420p",nb_components:3,log2_chroma_w:1,log2_chroma_h:1,comp: @[(0, 1, 0, 0, 8, 0, 7, 1),(1, 1, 0, 0, 8, 0, 7, 1),(2, 1, 0, 0, 8, 0, 7, 1)],flags:AV_PIX_FMT_FLAG_PLANAR)

proc av_pix_fmt_desc_get*(pix_fmt: AVPixelFormat): AVPixFmtDescriptor =
  return av_pix_fmt_descriptors[pix_fmt.int]

proc parse_cqm*(str: string; cqm: var seq[uint8]; length: cint): cint =
  var i: cint = 0
  var cqm = str.split(",").mapIt(it.parseInt.uint8)
  return 0
  # while true:
  #   var coef: cint
  #   if not scanf(str, "%d", coef) or coef < 1 or coef > 255:
  #     return -1
  #   cqm.add coef.uint8
  #   if not (i < length and (str = strchr(str, ',')) and inc(str)):
  #     break
  # return if (i == length): 0 else: -1


var x264_cpu_names*: seq[x264_cpu_name_t] = @[
    ("MMX2", X264_CPU_MMX or X264_CPU_MMX),
    ("MMXEXT", X264_CPU_MMX or X264_CPU_MMX),
    ("SSE", X264_CPU_MMX or X264_CPU_MMX2 or X264_CPU_SSE),
    ("SSE2Slow", X264_CPU_MMX or X264_CPU_MMX2 or X264_CPU_SSE or X264_CPU_SSE2 or X264_CPU_SSE2_IS_SLOW),
    ("SSE2", X264_CPU_MMX or X264_CPU_MMX2 or X264_CPU_SSE or X264_CPU_SSE2),
    ("SSE2Fast", X264_CPU_MMX or X264_CPU_MMX2 or X264_CPU_SSE or X264_CPU_SSE2 or X264_CPU_SSE2_IS_FAST),
    ("LZCNT", X264_CPU_MMX or X264_CPU_MMX2 or X264_CPU_SSE or X264_CPU_SSE2 or X264_CPU_LZCNT),
    ("SSE3", X264_CPU_MMX or X264_CPU_MMX2 or X264_CPU_SSE or X264_CPU_SSE2 or X264_CPU_SSE3),
    ("SSSE3", X264_CPU_MMX or X264_CPU_MMX2 or X264_CPU_SSE or X264_CPU_SSE2 or X264_CPU_SSE3 or X264_CPU_SSSE3),
    ("SSE4.1", X264_CPU_MMX or X264_CPU_MMX2 or X264_CPU_SSE or X264_CPU_SSE2 or X264_CPU_SSE3 or X264_CPU_SSSE3 or X264_CPU_SSE4),
    ("SSE4", X264_CPU_MMX or X264_CPU_MMX2 or X264_CPU_SSE or X264_CPU_SSE2 or X264_CPU_SSE3 or X264_CPU_SSSE3 or X264_CPU_SSE4),
    ("SSE4.2", X264_CPU_MMX or X264_CPU_MMX2 or X264_CPU_SSE or X264_CPU_SSE2 or X264_CPU_SSE3 or X264_CPU_SSSE3 or X264_CPU_SSE4 or X264_CPU_SSE42),
    ("AVX", X264_CPU_MMX or X264_CPU_MMX2 or X264_CPU_SSE or X264_CPU_SSE2 or X264_CPU_SSE3 or X264_CPU_SSSE3 or X264_CPU_SSE4 or X264_CPU_SSE42 or X264_CPU_AVX),
    ("XOP", X264_CPU_MMX or X264_CPU_MMX2 or X264_CPU_SSE or X264_CPU_SSE2 or X264_CPU_SSE3 or X264_CPU_SSSE3 or X264_CPU_SSE4 or X264_CPU_SSE42 or X264_CPU_AVX or X264_CPU_XOP),
    ("FMA4", X264_CPU_MMX or X264_CPU_MMX2 or X264_CPU_SSE or X264_CPU_SSE2 or X264_CPU_SSE3 or X264_CPU_SSSE3 or X264_CPU_SSE4 or X264_CPU_SSE42 or X264_CPU_AVX or X264_CPU_FMA4),
    ("FMA3", X264_CPU_MMX or X264_CPU_MMX2 or X264_CPU_SSE or X264_CPU_SSE2 or X264_CPU_SSE3 or X264_CPU_SSSE3 or X264_CPU_SSE4 or X264_CPU_SSE42 or X264_CPU_AVX or X264_CPU_FMA3),
    ("BMI1", X264_CPU_MMX or X264_CPU_MMX2 or X264_CPU_SSE or X264_CPU_SSE2 or X264_CPU_SSE3 or X264_CPU_SSSE3 or X264_CPU_SSE4 or X264_CPU_SSE42 or X264_CPU_AVX or X264_CPU_LZCNT or X264_CPU_BMI1),
    ("BMI2", X264_CPU_MMX or X264_CPU_MMX2 or X264_CPU_SSE or X264_CPU_SSE2 or X264_CPU_SSE3 or X264_CPU_SSSE3 or X264_CPU_SSE4 or X264_CPU_SSE42 or X264_CPU_AVX or X264_CPU_LZCNT or X264_CPU_BMI1 or X264_CPU_BMI2),
    ("AVX2", X264_CPU_MMX or X264_CPU_MMX2 or X264_CPU_SSE or X264_CPU_SSE2 or X264_CPU_SSE3 or X264_CPU_SSSE3 or X264_CPU_SSE4 or X264_CPU_SSE42 or X264_CPU_AVX or X264_CPU_FMA3 or X264_CPU_LZCNT or X264_CPU_BMI1 or X264_CPU_BMI2 or X264_CPU_AVX2),
    ("AVX512", X264_CPU_MMX or X264_CPU_MMX2 or X264_CPU_SSE or X264_CPU_SSE2 or X264_CPU_SSE3 or X264_CPU_SSSE3 or X264_CPU_SSE4 or X264_CPU_SSE42 or X264_CPU_AVX or X264_CPU_FMA3 or X264_CPU_LZCNT or X264_CPU_BMI1 or X264_CPU_BMI2 or X264_CPU_AVX2 or X264_CPU_AVX512),
    ("Cache32", X264_CPU_CACHELINE_32),
    ("Cache64", X264_CPU_CACHELINE_64),
    ("SlowAtom", X264_CPU_SLOW_ATOM),
    ("SlowPshufb", X264_CPU_SLOW_PSHUFB),
    ("SlowPalignr", X264_CPU_SLOW_PALIGNR),
    ("SlowShuffle", X264_CPU_SLOW_SHUFFLE),
    ("UnalignedStack", X264_CPU_STACK_MOD4),
    ("Altivec", X264_CPU_ALTIVEC),
    ("ARMv6", X264_CPU_ARMV6),
    ("NEON", X264_CPU_NEON),
    ("FastNeonMRC", X264_CPU_FAST_NEON_MRC),
    ("ARMv8", X264_CPU_ARMV8),
    ("NEON", X264_CPU_NEON),
    ("MSA", X264_CPU_MSA)]

var x264_direct_pred_names*: seq[string] = @["none", "spatial", "temporal","auto"]
var x264_motion_est_names*: seq[string] = @["dia", "hex", "umh", "esa","tesa"]
var x264_b_pyramid_names*: seq[string] = @["none", "strict", "normal"]
var x264_overscan_names*: seq[string] = @["undef", "show", "crop"]
var x264_vidformat_names*: seq[string] = @["component", "pal", "ntsc","secam", "mac", "undef"]
var x264_fullrange_names*: seq[string] = @["off", "on"]
var x264_colorprim_names*: seq[string] = @["", "bt709", "undef", "","bt470m", "bt470bg", "smpte170m", "smpte240m", "film", "bt2020", "smpte428", "smpte431", "smpte432"]
var x264_transfer_names*: seq[string] = @["", "bt709", "undef", "", "bt470m",
    "bt470bg", "smpte170m", "smpte240m", "linear", "log100", "log316", "iec61966-2-4",
    "bt1361e", "iec61966-2-1", "bt2020-10", "bt2020-12", "smpte2084", "smpte428",
    "arib-std-b67"]
var x264_colmatrix_names*: seq[string] = @["GBR", "bt709", "undef", "",
    "fcc", "bt470bg", "smpte170m", "smpte240m", "YCgCo", "bt2020nc", "bt2020c",
    "smpte2085", "chroma-derived-nc", "chroma-derived-c", "ICtCp"]
var x264_nal_hrd_names*: seq[string] = @["none", "vbr", "cbr"]
var x264_avcintra_flavor_names*: seq[string] = @["panasonic", "sony"]


# proc x264_param_parse*(p: x264_param_t; k: string; v: string): cint =
#   var name = k
#   var value = v
#   var name_buf: string
#   var b_error: cint = 0
#   var errortype: cint = X264_PARAM_BAD_VALUE
#   var name_was_bool: cint
#   var value_was_null: cint
#   if value == "":
#     value = "true"

#   name = name.replace("_","-")
#   if name == "no":
#     value = if parseBool(value): "false" else: "true"
#   name_was_bool = 0
#   case name
#   of "asm":
#     p.cpu = if isDigit(value[0]): uint32 parseInt(value) 
#             else: 
#               if value== "auto" or parseBool(value): x264_cpu_detect() 
#               else: 0'u32
#     if b_error:
#       var buf = $value
#       var
#         tok: seq[string]
#         init: string
#       b_error = 0
#       p.cpu = 0
#       tok = buf.split(",")
#       block findCpu:
#         for t in tok:
#           for i in 0..x264_cpu_names.high:
#             if t == x264_cpu_names[i].name:
#               p.cpu = p.cpu or x264_cpu_names[i].flags.uint32
#               break findCpu
#       if (p.cpu and X264_CPU_SSSE3) != 0 and  (p.cpu and X264_CPU_SSE2_IS_SLOW) == 0:
#         p.cpu = p.cpu or X264_CPU_SSE2_IS_FAST
#   of "threads":
#     if value== "auto":
#       p.i_threads = X264_THREADS_AUTO
#     else:
#       p.i_threads = cint parseInt(value)
#   of "lookahead-threads":
#     if value== "auto":
#       p.i_lookahead_threads = X264_THREADS_AUTO
#     else:
#       p.i_lookahead_threads = cint parseInt(value)
#   of "sliced-threads":
#     p.b_sliced_threads = parseBool(value)
#   of "sync-lookahead":
#     if value== "auto":
#       p.i_sync_lookahead = X264_SYNC_LOOKAHEAD_AUTO
#     else:
#       p.i_sync_lookahead = cint parseInt(value)
#   of "deterministic","n-deterministic":
#     p.b_deterministic = parseBool(value)
#   of "cpu-independent":
#     p.b_cpu_independent = parseBool(value)
#   of "level","level-idc":
#     if value == "1b":
#       p.i_level_idc = 9
#     elif parseFloat(value) < 7:
#       p.i_level_idc = cint(10.0 * parseFloat(value) + 0.5)
#     else:
#       p.i_level_idc = cint parseInt(value)
#   of "bluray-compat":
#     p.b_bluray_compat = parseBool(value)
#   of "avcintra-class":
#     p.i_avcintra_class = cint parseInt(value)
#   of "avcintra-flavor":
#     b_error = b_error or parseEnum(value, x264_avcintra_flavor_names, p.i_avcintra_flavor)
#   of "sar":
#     b_error = (scanf(value, "%d:%d", p.vui.i_sar_width, p.vui.i_sar_height) and scanf(value, "%d/%d", p.vui.i_sar_width, p.vui.i_sar_height))
#   of "overscan":
#     b_error = b_error or parseEnum(value, x264_overscan_names, p.vui.i_overscan)
#   of "videoformat":
#     b_error = b_error or parseEnum(value, x264_vidformat_names, p.vui.i_vidformat)
#   of "fullrange":
#     b_error = b_error or parseEnum(value, x264_fullrange_names, p.vui.b_fullrange)
#   of "colorprim":
#     b_error = b_error or parseEnum(value, x264_colorprim_names, p.vui.i_colorprim)
#   of "transfer":
#     b_error = b_error or parseEnum(value, x264_transfer_names, p.vui.i_transfer)
#   of "colormatrix": 
#     b_error = b_error or parseEnum(value, x264_colmatrix_names, p.vui.i_colmatrix)
#   of "chromaloc":
#     p.vui.i_chroma_loc = cint parseInt(value)
#     b_error = (p.vui.i_chroma_loc < 0 or p.vui.i_chroma_loc > 5)
#   of "alternative-transfer":
#     b_error = b_error or parseEnum(value, x264_transfer_names, p.i_alternative_transfer)
#   of "fps":
#     if scanf(value, "%u/%u", p.i_fps_num, p.i_fps_den) != 2:
#       var fps: cdouble = parseFloat(value)
#       if fps > 0.0 and fps <= INT_MAX div 1000:
#         p.i_fps_num = (cint)(fps * 1000 + 0.5)
#         p.i_fps_den = 1000
#       else:
#         p.i_fps_num = cint parseInt(value)
#         p.i_fps_den = 1
#   of "ref","frameref":
#     p.i_frame_reference = cint parseInt(value)
#   of "dpb-size":
#     p.i_dpb_size = cint parseInt(value)
#   of "keyint":
#     if "infinite" in value:
#       p.i_keyint_max = X264_KEYINT_MAX_INFINITE
#     else:
#       p.i_keyint_max = cint parseInt(value)
#   of "min-keyint","keyint-min":
#     p.i_keyint_min = cint parseInt(value)
#     if p.i_keyint_max < p.i_keyint_min:
#       p.i_keyint_max = p.i_keyint_min
#   of "scenecut":
#     p.i_scenecut_threshold = parseBool(value)
#     if b_error or p.i_scenecut_threshold:
#       b_error = 0
#       p.i_scenecut_threshold = cint parseInt(value)
#   of "intra-refresh":
#     p.b_intra_refresh = parseBool(value)
#   of "bframes":
#     p.i_bframe = cint parseInt(value)
#   of "b-adapt":
#     p.i_bframe_adaptive = parseBool(value)
#     if b_error:
#       b_error = 0
#       p.i_bframe_adaptive = cint parseInt(value)
#   of "b-bias":
#     p.i_bframe_bias = cint parseInt(value)
#   of "b-pyramid":
#     b_error = b_error or parseEnum(value, x264_b_pyramid_names, p.i_bframe_pyramid)
#     if b_error:
#       b_error = 0
#       p.i_bframe_pyramid = cint parseInt(value)
#   of "open-gop":
#     p.b_open_gop = parseBool(value)
#   of "nf":
#     p.b_deblocking_filter = not parseBool(value)
#   of "filter","deblock":
#     if 2 == scanf(value, "%d:%d", p.i_deblocking_filter_alphac0, p.i_deblocking_filter_beta) or 2 == scanf(value, "%d,%d", p.i_deblocking_filter_alphac0, p.i_deblocking_filter_beta):
#       p.b_deblocking_filter = 1
#     elif scanf(value, "%d", p.i_deblocking_filter_alphac0):
#       p.b_deblocking_filter = 1
#       p.i_deblocking_filter_beta = p.i_deblocking_filter_alphac0
#     else:
#       p.b_deblocking_filter = parseBool(value)
#   of "slice-max-size":
#     p.i_slice_max_size = cint parseInt(value)
#   of "slice-max-mbs":
#     p.i_slice_max_mbs = cint parseInt(value)
#   of "slice-min-mbs":
#     p.i_slice_min_mbs = cint parseInt(value)
#   of "slices":
#     p.i_slice_count = cint parseInt(value)
#   of "slices-max":
#     p.i_slice_count_max = cint parseInt(value)
#   of "cabac":
#     p.b_cabac = parseBool(value)
#   of "cabac-idc":
#     p.i_cabac_init_idc = cint parseInt(value)
#   of "interlaced":
#     p.b_interlaced = parseBool(value)
#   of "tff":
#     p.b_interlaced = p.b_tff = parseBool(value)
#   of "bff":
#     p.b_interlaced = parseBool(value)
#     p.b_tff = not p.b_interlaced
#   of "constrained-intra":
#     p.b_constrained_intra = parseBool(value)
#   of "cqm":
#     if strstr(value, "flat"):
#       p.i_cqm_preset = X264_CQM_FLAT
#     elif strstr(value, "jvt"):
#       p.i_cqm_preset = X264_CQM_JVT
#     else:
#       CHECKED_ERROR_PARAM_STRDUP(p.psz_cqm_file, p, value)
#   of "cqmfile":
#     CHECKED_ERROR_PARAM_STRDUP(p.psz_cqm_file, p, value)
#   of "cqm4":
#     p.i_cqm_preset = X264_CQM_CUSTOM
#     b_error = b_error or parse_cqm(value, p.cqm_4iy, 16)
#     b_error = b_error or parse_cqm(value, p.cqm_4py, 16)
#     b_error = b_error or parse_cqm(value, p.cqm_4ic, 16)
#     b_error = b_error or parse_cqm(value, p.cqm_4pc, 16)
#   of "cqm8":
#     p.i_cqm_preset = X264_CQM_CUSTOM
#     b_error = b_error or parse_cqm(value, p.cqm_8iy, 64)
#     b_error = b_error or parse_cqm(value, p.cqm_8py, 64)
#     b_error = b_error or parse_cqm(value, p.cqm_8ic, 64)
#     b_error = b_error or parse_cqm(value, p.cqm_8pc, 64)
#   of "cqm4i":
#     p.i_cqm_preset = X264_CQM_CUSTOM
#     b_error = b_error or parse_cqm(value, p.cqm_4iy, 16)
#     b_error = b_error or parse_cqm(value, p.cqm_4ic, 16)
#   of "cqm4p":
#     p.i_cqm_preset = X264_CQM_CUSTOM
#     b_error = b_error or parse_cqm(value, p.cqm_4py, 16)
#     b_error = b_error or parse_cqm(value, p.cqm_4pc, 16)
#   of "cqm4iy":
#     p.i_cqm_preset = X264_CQM_CUSTOM
#     b_error = b_error or parse_cqm(value, p.cqm_4iy, 16)
#   of "cqm4ic":
#     p.i_cqm_preset = X264_CQM_CUSTOM
#     b_error = b_error or parse_cqm(value, p.cqm_4ic, 16)
#   of "cqm4py":
#     p.i_cqm_preset = X264_CQM_CUSTOM
#     b_error = b_error or parse_cqm(value, p.cqm_4py, 16)
#   of "cqm4pc":
#     p.i_cqm_preset = X264_CQM_CUSTOM
#     b_error = b_error or parse_cqm(value, p.cqm_4pc, 16)
#   of "cqm8i":
#     p.i_cqm_preset = X264_CQM_CUSTOM
#     b_error = b_error or parse_cqm(value, p.cqm_8iy, 64)
#     b_error = b_error or parse_cqm(value, p.cqm_8ic, 64)
#   of "cqm8p":
#     p.i_cqm_preset = X264_CQM_CUSTOM
#     b_error = b_error or parse_cqm(value, p.cqm_8py, 64)
#     b_error = b_error or parse_cqm(value, p.cqm_8pc, 64)
#   of "log":
#     p.i_log_level = cint parseInt(value)
#   of "dump-yuv":
#     CHECKED_ERROR_PARAM_STRDUP(p.psz_dump_yuv, p, value)
#   of "analyse","partitions":
#     p.analyse.inter = 0
#     if strstr(value, "none"):
#       p.analyse.inter = 0
#     if strstr(value, "all"):
#       p.analyse.inter = not 0
#     if strstr(value, "i4x4"):
#       p.analyse.inter = p.analyse.inter or X264_ANALYSE_I4x4
#     if strstr(value, "i8x8"):
#       p.analyse.inter = p.analyse.inter or X264_ANALYSE_I8x8
#     if strstr(value, "p8x8"):
#       p.analyse.inter = p.analyse.inter or X264_ANALYSE_PSUB16x16
#     if strstr(value, "p4x4"):
#       p.analyse.inter = p.analyse.inter or X264_ANALYSE_PSUB8x8
#     if strstr(value, "b8x8"):
#       p.analyse.inter = p.analyse.inter or X264_ANALYSE_BSUB16x16
#   of "8x8dct":
#     p.analyse.b_transform_8x8 = parseBool(value)
#   of "weightb","weight-b":
#     p.analyse.b_weighted_bipred = parseBool(value)
#   of "weightp":
#     p.analyse.i_weighted_pred = cint parseInt(value)
#   of "direct","direct-pred":
#     b_error = b_error or
#         parseEnum(value, x264_direct_pred_names, p.analyse.i_direct_mv_pred)
#   of "chroma-qp-offset":
#     p.analyse.i_chroma_qp_offset = cint parseInt(value)
#   of "me":
#     b_error = b_error or
#         parseEnum(value, x264_motion_est_names, p.analyse.i_me_method)
#   of "merange","me-range":
#     p.analyse.i_me_range = cint parseInt(value)
#   of "mvrange","mv-range":
#     p.analyse.i_mv_range = cint parseInt(value)
#   of "mvrange-thread","mv-range-thread":
#     p.analyse.i_mv_range_thread = cint parseInt(value)
#   of "subme","subq":
#     p.analyse.i_subpel_refine = cint parseInt(value)
#   of "psy-rd":
#     if 2 == scanf(value, "%f:%f", p.analyse.f_psy_rd, p.analyse.f_psy_trellis) or 2 == scanf(value, "%f,%f", p.analyse.f_psy_rd, p.analyse.f_psy_trellis) or 2 == scanf(value, "%f|%f", p.analyse.f_psy_rd, p.analyse.f_psy_trellis):
#       discard
#     elif scanf(value, "%f", p.analyse.f_psy_rd):
#       p.analyse.f_psy_trellis = 0
#     else:
#       p.analyse.f_psy_rd = 0
#       p.analyse.f_psy_trellis = 0
#   of "psy":
#     p.analyse.b_psy = parseBool(value)
#   of "chroma-me":
#     p.analyse.b_chroma_me = parseBool(value)
#   of "mixed-refs":
#     p.analyse.b_mixed_references = parseBool(value)
#   of "trellis":
#     p.analyse.i_trellis = cint parseInt(value)
#   of "fast-pskip":
#     p.analyse.b_fast_pskip = parseBool(value)
#   of "dct-decimate":
#     p.analyse.b_dct_decimate = parseBool(value)
#   of "deadzone-inter":
#     p.analyse.i_luma_deadzone[0] = cint parseInt(value)
#   of "deadzone-intra":
#     p.analyse.i_luma_deadzone[1] = cint parseInt(value)
#   of "nr":
#     p.analyse.i_noise_reduction = cint parseInt(value)
#   of "bitrate":
#     p.rc.i_bitrate = cint parseInt(value)
#     p.rc.i_rc_method = X264_RC_ABR
#   of "qp","qp_constant":
#     p.rc.i_qp_constant = cint parseInt(value)
#     p.rc.i_rc_method = X264_RC_CQP
#   of "crf":
#     p.rc.f_rf_constant = parseFloat(value)
#     p.rc.i_rc_method = X264_RC_CRF
#   of "crf-max":
#     p.rc.f_rf_constant_max = parseFloat(value)
#   of "rc-lookahead":
#     p.rc.i_lookahead = cint parseInt(value)
#   of "qpmin","qp-min":
#     p.rc.i_qp_min = cint parseInt(value)
#   of "qpmax","qp-max":
#     p.rc.i_qp_max = cint parseInt(value)
#   of "qpstep","qp-step":
#     p.rc.i_qp_step = cint parseInt(value)
#   of "ratetol":
#     p.rc.f_rate_tolerance = if not strncmp("inf", value, 3): 1000000000.0 else: parseFloat(
#         value)
#   of "vbv-maxrate":
#     p.rc.i_vbv_max_bitrate = cint parseInt(value)
#   of "vbv-bufsize":
#     p.rc.i_vbv_buffer_size = cint parseInt(value)
#   of "vbv-init":
#     p.rc.f_vbv_buffer_init = parseFloat(value)
#   of "ipratio","ip-factor":
#     p.rc.f_ip_factor = parseFloat(value)
#   of "pbratio","pb-factor":
#     p.rc.f_pb_factor = parseFloat(value)
#   of "aq-mode":
#     p.rc.i_aq_mode = cint parseInt(value)
#   of "aq-strength":
#     p.rc.f_aq_strength = parseFloat(value)
#   of "pass":
#     var pass: cint = x264_clip3(parseInt(value), 0, 3)
#     p.rc.b_stat_write = pass and 1
#     p.rc.b_stat_read = pass and 2
#   of "stats":
#     CHECKED_ERROR_PARAM_STRDUP(p.rc.psz_stat_in, p, value)
#     CHECKED_ERROR_PARAM_STRDUP(p.rc.psz_stat_out, p, value)
#   of "qcomp":
#     p.rc.f_qcompress = parseFloat(value)
#   of "mbtree":
#     p.rc.b_mb_tree = parseBool(value)
#   of "qblur":
#     p.rc.f_qblur = parseFloat(value)
#   of "cplxblur","cplx-blur":
#     p.rc.f_complexity_blur = parseFloat(value)
#   of "zones":
#     CHECKED_ERROR_PARAM_STRDUP(p.rc.psz_zones, p, value)
#   of "crop-rect":
#     b_error = b_error or
#         scanf(value, "%d,%d,%d,%d", p.crop_rect.i_left,
#                p.crop_rect.i_top, p.crop_rect.i_right,
#                p.crop_rect.i_bottom) != 4
#   of "psnr":
#     p.analyse.b_psnr = parseBool(value)
#   of "ssim":
#     p.analyse.b_ssim = parseBool(value)
#   of "aud":
#     p.b_aud = parseBool(value)
#   of "sps-id":
#     p.i_sps_id = cint parseInt(value)
#   of "global-header":
#     p.b_repeat_headers = not parseBool(value)
#   of "repeat-headers":
#     p.b_repeat_headers = parseBool(value)
#   of "annexb":
#     p.b_annexb = parseBool(value)
#   of "force-cfr":
#     p.b_vfr_input = not parseBool(value)
#   of "nal-hrd":
#     b_error = b_error or parseEnum(value, x264_nal_hrd_names, p.i_nal_hrd)
#   of "filler":
#     p.rc.b_filler = parseBool(value)
#   of "pic-struct":
#     p.b_pic_struct = parseBool(value)
#   of "fake-interlaced":
#     p.b_fake_interlaced = parseBool(value)
#   of "frame-packing":
#     p.i_frame_packing = cint parseInt(value)
#   of "stitchable":
#     p.b_stitchable = parseBool(value)
#   of "opencl":
#     p.b_opencl = parseBool(value)
#   of "opencl-clbin":
#     CHECKED_ERROR_PARAM_STRDUP(p.psz_clbin_file, p, value)
#   of "opencl-device":
#     p.i_opencl_device = cint parseInt(value)
#   else:
#     b_error = 1
#     errortype = X264_PARAM_BAD_NAME
#   if name_buf:
#     free(name_buf)
#   b_error = b_error or value_was_null and not name_was_bool
#   return if b_error: errortype else: 0

template AV_CEIL_RSHIFT*(a, b: untyped): untyped =
  if  b == 0: -((-(a)) shr (b)) 
  else: ((a) + (1 shl (b)) - 1) shr (b)

proc av_clip[T]( a:T,  amin:T, amax:T): T = 
    if  a < amin: amin
    elif (a > amax): amax
    else: a



proc x264_param_apply_fastfirstpass*(param: x264_param_t) =
  if param.rc.b_stat_write and not param.rc.b_stat_read:
    param.i_frame_reference = 1
    param.analyse.b_transform_8x8 = 0
    param.analyse.inter = 0
    param.analyse.i_me_method = X264_ME_DIA
    param.analyse.i_subpel_refine = min(2, param.analyse.i_subpel_refine)
    param.analyse.i_trellis = 0
    param.analyse.b_fast_pskip = 1

proc profile_string_to_int*(str: cstring): cint =
  if str == "baseline":
    return PROFILE_BASELINE.cint
  if str == "main":
    return PROFILE_MAIN.cint
  if str == "high":
    return PROFILE_HIGH.cint
  if str == "high10":
    return PROFILE_HIGH10.cint
  if str == "high422":
    return PROFILE_HIGH422.cint
  if str == "high444":
    return PROFILE_HIGH444_PREDICTIVE.cint
  return -1.cint


proc x264_param_apply_profile*(param: x264_param_t; profile: string): cint =
  if profile == "":
    return 0
  var qp_bd_offset: cint = 6 * (param.i_bitdepth - 8)
  var p: cint = profile_string_to_int(profile)
  if p < 0:
    echo( "invalid profile: %s\n", profile)
    return -1
  if p < PROFILE_HIGH444_PREDICTIVE.cint and ((param.rc.i_rc_method == X264_RC_CQP and param.rc.i_qp_constant <= 0) or (param.rc.i_rc_method == X264_RC_CRF and (int)(param.rc.f_rf_constant.cint + qp_bd_offset) <= 0)):
    echo( "%s profile doesn\'t support lossless\n",profile)
    return -1
  if p < PROFILE_HIGH444_PREDICTIVE.cint and (param.i_csp and X264_CSP_MASK) >= X264_CSP_I444:
    echo( "%s profile doesn\'t support 4:4:4\n",profile)
    return -1
  if p < PROFILE_HIGH422.cint and (param.i_csp and X264_CSP_MASK) >= X264_CSP_I422:
    echo( "%s profile doesn\'t support 4:2:2\n",profile)
    return -1
  if p < PROFILE_HIGH10.cint and param.i_bitdepth > 8:
    echo(                      "%s profile doesn\'t support a bit depth of %d\n", profile,param.i_bitdepth)
    return -1
  if p < PROFILE_HIGH.cint and (param.i_csp and X264_CSP_MASK) == X264_CSP_I400:
    echo( "%s profile doesn\'t support 4:0:0\n",profile)
    return -1
  if p == PROFILE_BASELINE.cint:
    param.analyse.b_transform_8x8 = 0
    param.b_cabac = 0
    param.i_cqm_preset = X264_CQM_FLAT
    param.psz_cqm_file = nil
    param.i_bframe = 0
    param.analyse.i_weighted_pred = X264_WEIGHTP_NONE
    if param.b_interlaced:
      echo(                        "baseline profile doesn\'t support interlacing\n")
      return -1
    if param.b_fake_interlaced:
      echo(                        "baseline profile doesn\'t support fake interlacing\n")
      return -1
  elif p == PROFILE_MAIN.cint:
    param.analyse.b_transform_8x8 = 0
    param.i_cqm_preset = X264_CQM_FLAT
    param.psz_cqm_file = nil
  return 0


proc av_gcd*(a: int64; b: int64): int64 =
  var
    za: cint
    zb: cint
    k: cint
  var
    u: int64
    v: int64
  if a == 0:
    return b
  if b == 0:
    return a
  za = a.cint
  zb = b.cint
  k = cint min(za, zb)
  u = abs(a shr za)
  v = abs(b shr zb)
  while u != v:
    if u > v:
      swap(v, u)
    v -= u
    v = v shr v
  return (u) shl k

proc av_reduce*(dst_num: ptr cint; dst_den: ptr cint; num: int64; den: int64;max: int64): cint =
  # var
  #   a0: AVRational = (0, 1)
  #   a1: AVRational = (1, 0)
  # var sign: cint = (num < 0) xor (den < 0)
  # var gcd: int64 = av_gcd(abs(num), abs(den))
  # num = abs(num) div gcd
  # den = abs(den) div gcd
  # if num <= max and den <= max:
  #   a1 = (num, den)
  #   den = 0
  # while den:
  #   var x: uint64 = num div den
  #   var next_den: int64 = num - den * x
  #   var a2n: int64 = x * a1.num + a0.num
  #   var a2d: int64 = x * a1.den + a0.den
  #   if a2n > max or a2d > max:
  #     if a1.num:
  #       x = (max - a0.num) div a1.num
  #     if a1.den:
  #       x = min(x, (max - a0.den) div a1.den)
  #     if den * (2 * x * a1.den + a0.den) > num * a1.den:
  #       break
  #   a0 = a1
  #   a1 = (a2n, a2d)
  #   num = den
  #   den = next_den
  # dst_num[] = if sign: -a1.num else: a1.num
  # dst_den[] = a1.den
  return den == 0

proc parse_opts*(avctx: AVCodecContext; opt: cstring; param: cstring): cint =
  var x4: X264Context = cast[X264Context](avctx.priv_data)
  var ret: cint
  # ret = x264_param_parse((x4.params), opt, param)
  return ret


proc x264_encoder_open*(param: x264_param_t): x264_t 
proc x264_encoder_headers*(h: x264_t; pp_nal: x264_nal_t; pi_nal: cint): cint
proc X264_init*(avctx: AVCodecContext): cint

proc X264_init*(avctx: AVCodecContext): cint =
  var x4: X264Context = cast[X264Context](avctx.priv_data)
  var cpb_props: AVCPBProperties
  var
    sw: cint
    sh: cint
  var ret: cint
  if avctx.global_quality > 0:
    echo("-qscale is ignored, -crf is recommended.\n")
  x264_param_default(x4.params)
  x4.params.b_deblocking_filter = avctx.flags and AV_CODEC_FLAG_LOOP_FILTER
  if x264_param_default_preset(x4.params, x4.preset, x4.tune) < 0:
    echo("Error setting preset/tune %s/%s.\n", x4.preset, x4.tune)
    echo("Possible presets:")
    for i in x264_preset_names: echo i
    for i in x264_tune_names: echo i
    return EINVAL
  if avctx.level > 0:
    x4.params.i_level_idc = avctx.level
  # x4.params.pf_log = X264_log
  x4.params.p_log_private = unsafeAddr avctx
  # x4.params.i_log_level = X264_LOG_DEBUG
  x4.params.i_csp = convert_pix_fmt(avctx.pix_fmt)
  x4.params.i_bitdepth = cint av_pix_fmt_desc_get(avctx.pix_fmt).comp[0].depth
  # PARSE_X264_OPT("weightp", wpredp)
  if avctx.bit_rate div 1000 > INT_MAX or avctx.rc_max_rate div 1000 > INT_MAX:
    echo("bit_rate and rc_max_rate > %d000 not supported by libx264\n",INT_MAX)
    return EINVAL
  x4.params.rc.i_bitrate = cint avctx.bit_rate div 1000
  x4.params.rc.i_rc_method = X264_RC_ABR
  x4.params.rc.i_vbv_buffer_size = avctx.rc_buffer_size div 1000
  x4.params.rc.i_vbv_max_bitrate = cint avctx.rc_max_rate div 1000
  x4.params.rc.b_stat_write = avctx.flags and AV_CODEC_FLAG_PASS1
  if avctx.flags and AV_CODEC_FLAG_PASS2:
    x4.params.rc.b_stat_read = 1
  else:
    if x4.crf >= 0:
      x4.params.rc.i_rc_method = X264_RC_CRF
      x4.params.rc.f_rf_constant = x4.crf
    elif x4.cqp >= 0:
      x4.params.rc.i_rc_method = X264_RC_CQP
      x4.params.rc.i_qp_constant = x4.cqp
    if x4.crf_max >= 0:
      x4.params.rc.f_rf_constant_max = x4.crf_max
  if avctx.rc_initial_buffer_occupancy > 0 and (avctx.rc_initial_buffer_occupancy <= avctx.rc_buffer_size):
    x4.params.rc.f_vbv_buffer_init = cast[cfloat](avctx.rc_initial_buffer_occupancy div avctx.rc_buffer_size)
  # PARSE_X264_OPT("level", level)
  if avctx.i_quant_factor > 0:
    x4.params.rc.f_ip_factor = cfloat 1.0'f32 /  abs(avctx.i_quant_factor)
  if avctx.b_quant_factor > 0:
    x4.params.rc.f_pb_factor = avctx.b_quant_factor
  # if avctx.chromaoffset:
  #   x4.chroma_offset = avctx.chromaoffset
  if x4.chroma_offset:
    x4.params.analyse.i_chroma_qp_offset = x4.chroma_offset
  if avctx.gop_size >= 0:
    x4.params.i_keyint_max = avctx.gop_size
  if avctx.max_b_frames >= 0:
    x4.params.i_bframe = avctx.max_b_frames
  # if avctx.scenechange_threshold >= 0:
  #   x4.scenechange_threshold = avctx.scenechange_threshold
  # if x4.scenechange_threshold >= 0:
  #   x4.params.i_scenecut_threshold = x4.scenechange_threshold
  if avctx.qmin >= 0:
    x4.params.rc.i_qp_min = avctx.qmin
  if avctx.qmax >= 0:
    x4.params.rc.i_qp_max = avctx.qmax
  if avctx.max_qdiff >= 0:
    x4.params.rc.i_qp_step = avctx.max_qdiff
  if avctx.qblur >= 0:
    x4.params.rc.f_qblur = avctx.qblur
  if avctx.qcompress >= 0:
    x4.params.rc.f_qcompress = avctx.qcompress
  if avctx.refs >= 0:
    x4.params.i_frame_reference = avctx.refs
  elif x4.params.i_level_idc > 0:
    var i: cint
    var mbn: cint = AV_CEIL_RSHIFT(avctx.width, 4) * AV_CEIL_RSHIFT(avctx.height, 4)
    var scale: cint =  1
    i = 0
    while i < x264_levels[i].level_idc:
      if x264_levels[i].level_idc == x4.params.i_level_idc:
        x4.params.i_frame_reference = cint av_clip[int](x264_levels[i].dpb div mbn div scale, 1, x4.params.i_frame_reference)
      inc(i)
  if avctx.trellis >= 0:
    x4.params.analyse.i_trellis = avctx.trellis
  if avctx.me_range >= 0:
    x4.params.analyse.i_me_range = avctx.me_range
  # if avctx.noise_reduction >= 0:
  #   x4.noise_reduction = avctx.noise_reduction
  # if x4.noise_reduction >= 0:
  #   x4.params.analyse.i_noise_reduction = x4.noise_reduction
  if avctx.me_subpel_quality >= 0:
    x4.params.analyse.i_subpel_refine = avctx.me_subpel_quality
  # if avctx.b_frame_strategy >= 0:
  #   x4.b_frame_strategy = avctx.b_frame_strategy
  if avctx.keyint_min >= 0:
    x4.params.i_keyint_min = avctx.keyint_min
  # if avctx.coder_type >= 0:
  #   x4.coder = avctx.coder_type == FF_CODER_TYPE_AC
  if avctx.me_cmp >= 0:
    x4.params.analyse.b_chroma_me = avctx.me_cmp and FF_CMP_CHROMA
  if x4.aq_mode >= 0:
    x4.params.rc.i_aq_mode = x4.aq_mode
  if x4.aq_strength >= 0:
    x4.params.rc.f_aq_strength = x4.aq_strength
  # PARSE_X264_OPT("psy-rd", psy_rd)
  # PARSE_X264_OPT("deblock", deblock)
  # PARSE_X264_OPT("partitions", partitions)
  # PARSE_X264_OPT("stats", stats)
  if x4.psy >= 0:
    x4.params.analyse.b_psy = x4.psy
  if x4.rc_lookahead >= 0:
    x4.params.rc.i_lookahead = x4.rc_lookahead
  if x4.weightp >= 0:
    x4.params.analyse.i_weighted_pred = x4.weightp
  if x4.weightb >= 0:
    x4.params.analyse.b_weighted_bipred = x4.weightb
  if x4.cplxblur >= 0:
    x4.params.rc.f_complexity_blur = x4.cplxblur
  if x4.ssim >= 0:
    x4.params.analyse.b_ssim = x4.ssim
  if x4.intra_refresh >= 0:
    x4.params.b_intra_refresh = x4.intra_refresh
  if x4.bluray_compat >= 0:
    x4.params.b_bluray_compat = x4.bluray_compat
    x4.params.b_vfr_input = 0
  if x4.avcintra_class >= 0:
    x4.params.i_avcintra_class = x4.avcintra_class
  if x4.b_bias != INT_MIN:
    x4.params.i_bframe_bias = x4.b_bias
  if x4.b_pyramid >= 0:
    x4.params.i_bframe_pyramid = x4.b_pyramid
  if x4.mixed_refs >= 0:
    x4.params.analyse.b_mixed_references = x4.mixed_refs
  if x4.dct8x8 >= 0:
    x4.params.analyse.b_transform_8x8 = x4.dct8x8
  if x4.fast_pskip >= 0:
    x4.params.analyse.b_fast_pskip = x4.fast_pskip
  if x4.aud >= 0:
    x4.params.b_aud = x4.aud
  if x4.mbtree >= 0:
    x4.params.rc.b_mb_tree = x4.mbtree
  if x4.direct_pred >= 0:
    x4.params.analyse.i_direct_mv_pred = x4.direct_pred
  if x4.slice_max_size >= 0:
    x4.params.i_slice_max_size = x4.slice_max_size
  if x4.fastfirstpass:
    x264_param_apply_fastfirstpass(x4.params)
  if  x4.profile == "":
    case avctx.profile
    of FF_PROFILE_H264_BASELINE:
      x4.profile = "baseline"
    of FF_PROFILE_H264_HIGH:
      x4.profile = "high"
    of FF_PROFILE_H264_HIGH_10:
      x4.profile = "high10"
    of FF_PROFILE_H264_HIGH_422:
      x4.profile = "high422"
    of FF_PROFILE_H264_HIGH_444:
      x4.profile = "high444"
    of FF_PROFILE_H264_MAIN:
      x4.profile = "main"
    else:
      return
  if x4.nal_hrd >= 0:
    x4.params.i_nal_hrd = x4.nal_hrd
  if x4.motion_est >= 0:
    x4.params.analyse.i_me_method = x4.motion_est
  if x4.coder >= 0:
    x4.params.b_cabac = x4.coder
  # if x4.b_frame_strategy >= 0:
  #   x4.params.i_bframe_adaptive = x4.b_frame_strategy
  if x4.profile != "":
    if x264_param_apply_profile(x4.params, x4.profile) < 0:
      echo(  "Error setting profile %s.\n", x4.profile)
      echo(  "Possible profiles:")
      for p in x264_profile_names:
        echo  p
      return EINVAL
  x4.params.i_width = avctx.width
  x4.params.i_height = avctx.height
  discard av_reduce(sw.addr, sh.addr, avctx.sample_aspect_ratio.num, avctx.sample_aspect_ratio.den, 4096)
  x4.params.vui.i_sar_width = sw
  x4.params.vui.i_sar_height = sh
  x4.params.i_timebase_den = uint32 avctx.time_base.den
  x4.params.i_timebase_num = uint32 avctx.time_base.num
  if avctx.framerate.num > 0 and avctx.framerate.den > 0:
    x4.params.i_fps_num = uint32 avctx.framerate.num
    x4.params.i_fps_den = uint32 avctx.framerate.den
  else:
    x4.params.i_fps_num = uint32 avctx.time_base.den
    x4.params.i_fps_den = uint32 avctx.time_base.num * avctx.ticks_per_frame
  x4.params.analyse.b_psnr = avctx.flags and AV_CODEC_FLAG_PSNR
  x4.params.i_threads = avctx.thread_count
  if avctx.thread_type:
    x4.params.b_sliced_threads = avctx.thread_type == FF_THREAD_SLICE
  x4.params.b_interlaced = avctx.flags and AV_CODEC_FLAG_INTERLACED_DCT
  # x4.params.b_open_gop = not (avctx.flags and AV_CODEC_FLAG_CLOSED_GOP)
  x4.params.i_slice_count = avctx.slices
  x4.params.vui.b_fullrange = avctx.pix_fmt == AV_PIX_FMT_YUVJ420P or avctx.pix_fmt == AV_PIX_FMT_YUVJ422P or avctx.pix_fmt == AV_PIX_FMT_YUVJ444P or avctx.color_range == AVCOL_RANGE_JPEG
  if avctx.colorspace != AVCOL_SPC_UNSPECIFIED:
    x4.params.vui.i_colmatrix = cint avctx.colorspace
  if avctx.color_primaries != AVCOL_PRI_UNSPECIFIED:
    x4.params.vui.i_colorprim = cint avctx.color_primaries
  if avctx.color_trc != AVCOL_TRC_UNSPECIFIED:
    x4.params.vui.i_transfer = cint avctx.color_trc
  if avctx.flags and AV_CODEC_FLAG_GLOBAL_HEADER:
    x4.params.b_repeat_headers = 0
  var p: string = x4.x264opts
  # while p:
  #   var
  #     param: array[4096, char] = [0]
  #     val: array[4096, char] = [0]
  #   if scanf(p, "%4095[^:=]=%4095[^:]", param, val) == 1:
  #     ret = parse_opts(avctx, param, "1")
  #     if ret < 0:
  #       return ret
  #   else:
  #     ret = parse_opts(avctx, param, val)
  #     if ret < 0:
  #       return ret
  #   p = find(p, ':')
  #   inc(p, not not p)
  var en: string = x4.x264_params[""]
  # if (ret = x264_param_parse(x4.params, en.key, en.value)) < 0:
  #   echo("Error parsing option \'%s = %s\'.\n", en.key, en.value)
  #   if ret == X264_PARAM_ALLOC_FAILED:
  #     return (ENOMEM)
  avctx.has_b_frames = 
    if x4.params.i_bframe: 
      if x4.params.i_bframe_pyramid: 2 
      else: 1 
    else: 0
  if avctx.max_b_frames < 0:
    avctx.max_b_frames = 0
  avctx.bit_rate = x4.params.rc.i_bitrate * 1000
  x4.enc = x264_encoder_open(x4.params)
  if avctx.flags and AV_CODEC_FLAG_GLOBAL_HEADER:
    var nal: x264_nal_t
    var p: uint8
    var
      nnal: cint
      s: cint
      i: cint
    s = x264_encoder_headers(x4.enc, nal, nnal)
    avctx.extradata = p  #= av_mallocz(s + AV_INPUT_BUFFER_PADDING_SIZE)
    for i in 0..nal.high:
      if nal[i].i_type == NAL_SEI:
        echo(  "%s\n", nal[i].p_payload + 25)
        x4.sei_size = nal[i].i_payload
        x4.sei = av_malloc(x4.sei_size)
        copyMem(x4.sei, nal[i].p_payload, nal[i].i_payload)
        continue
      copyMem(p, nal[i].p_payload, nal[i].i_payload)
      p += nal[i].i_payload
    avctx.extradata_size = p - avctx.extradata
  cpb_props = ff_add_cpb_side_data(avctx)
  if not cpb_props:
    return (ENOMEM)
  cpb_props.buffer_size = x4.params.rc.i_vbv_buffer_size * 1000
  cpb_props.max_bitrate = x4.params.rc.i_vbv_max_bitrate * 1000
  cpb_props.avg_bitrate = x4.params.rc.i_bitrate * 1000
  ##  Overestimate the reordered opaque buffer size, in case a runtime
  ##  reconfigure would increase the delay (which it shouldn't).
  x4.nb_reordered_opaque = x264_encoder_maximum_delayed_frames(x4.enc) + 17
  x4.reordered_opaque = av_malloc_array(x4.nb_reordered_opaque,sizeof((x4.reordered_opaque[])))
  if not x4.reordered_opaque:
    return (ENOMEM)
  return 0

var pix_fmts_8bit*: UncheckedArray[AVPixelFormat] = [AV_PIX_FMT_YUV420P,
    AV_PIX_FMT_YUVJ420P, AV_PIX_FMT_YUV422P, AV_PIX_FMT_YUVJ422P,
    AV_PIX_FMT_YUV444P, AV_PIX_FMT_YUVJ444P, AV_PIX_FMT_NV12, AV_PIX_FMT_NV16,
    AV_PIX_FMT_NV21, AV_PIX_FMT_NONE]

var pix_fmts_9bit*: UncheckedArray[AVPixelFormat] = [AV_PIX_FMT_YUV420P9,
    AV_PIX_FMT_YUV444P9, AV_PIX_FMT_NONE]

var pix_fmts_10bit*: UncheckedArray[AVPixelFormat] = [AV_PIX_FMT_YUV420P10,
    AV_PIX_FMT_YUV422P10, AV_PIX_FMT_YUV444P10, AV_PIX_FMT_NV20, AV_PIX_FMT_NONE]

var pix_fmts_all*: UncheckedArray[AVPixelFormat] = [AV_PIX_FMT_YUV420P,
    AV_PIX_FMT_YUVJ420P, AV_PIX_FMT_YUV422P, AV_PIX_FMT_YUVJ422P,
    AV_PIX_FMT_YUV444P, AV_PIX_FMT_YUVJ444P, AV_PIX_FMT_NV12, AV_PIX_FMT_NV16,
    AV_PIX_FMT_NV21, AV_PIX_FMT_YUV420P10, AV_PIX_FMT_YUV422P10,
    AV_PIX_FMT_YUV444P10, AV_PIX_FMT_NV20, AV_PIX_FMT_GRAY8, AV_PIX_FMT_GRAY10,
    AV_PIX_FMT_NONE]

var pix_fmts_8bit_rgb*: UncheckedArray[AVPixelFormat] = [AV_PIX_FMT_BGR0,
    AV_PIX_FMT_BGR24, AV_PIX_FMT_RGB24, AV_PIX_FMT_NONE]

proc X264_init_static*(codec: ptr AVCodec) =
  codec.pix_fmts = pix_fmts_all

template OFFSET*(x: untyped): untyped =
  offsetof(X264Context, x)


##  var AVOption options[] = {
##      { "preset",        "Set the encoding preset (cf. x264 --fullhelp)",   OFFSET(preset),        AV_OPT_TYPE_STRING, { .str = "medium" }, 0, 0, VE},
##      { "tune",          "Tune the encoding params (cf. x264 --fullhelp)",  OFFSET(tune),          AV_OPT_TYPE_STRING, { 0 }, 0, 0, VE},
##      { "profile",       "Set profile restrictions (cf. x264 --fullhelp) ", OFFSET(profile),       AV_OPT_TYPE_STRING, { 0 }, 0, 0, VE},
##      { "fastfirstpass", "Use fast settings when encoding first pass",      OFFSET(fastfirstpass), AV_OPT_TYPE_BOOL, { .i64 = 1 }, 0, 1, VE},
##      {"level", "Specify level (as defined by Annex A)", OFFSET(level), AV_OPT_TYPE_STRING, {.str=NULL}, 0, 0, VE},
##      {"passlogfile", "Filename for 2 pass stats", OFFSET(stats), AV_OPT_TYPE_STRING, {.str=NULL}, 0, 0, VE},
##      {"wpredp", "Weighted prediction for P-frames", OFFSET(wpredp), AV_OPT_TYPE_STRING, {.str=NULL}, 0, 0, VE},
##      {"a53cc",          "Use A53 Closed Captions (if available)",          OFFSET(a53_cc),        AV_OPT_TYPE_BOOL,   {.i64 = 1}, 0, 1, VE},
##      {"x264opts", "x264 options", OFFSET(x264opts), AV_OPT_TYPE_STRING, {.str=NULL}, 0, 0, VE},
##      { "crf",           "Select the quality for constant quality mode",    OFFSET(crf),           AV_OPT_TYPE_FLOAT,  {.dbl = -1 }, -1, FLT_MAX, VE },
##      { "crf_max",       "In CRF mode, prevents VBV from lowering quality beyond this point.",OFFSET(crf_max), AV_OPT_TYPE_FLOAT, {.dbl = -1 }, -1, FLT_MAX, VE },
##      { "qp",            "Constant quantization parameter rate control method",OFFSET(cqp),        AV_OPT_TYPE_INT,    { .i64 = -1 }, -1, INT_MAX, VE },
##      { "aq-mode",       "AQ method",                                       OFFSET(aq_mode),       AV_OPT_TYPE_INT,    { .i64 = -1 }, -1, INT_MAX, VE, "aq_mode"},
##      { "none",          NULL,                              0, AV_OPT_TYPE_CONST, {.i64 = X264_AQ_NONE},         INT_MIN, INT_MAX, VE, "aq_mode" },
##      { "variance",      "Variance AQ (complexity mask)",   0, AV_OPT_TYPE_CONST, {.i64 = X264_AQ_VARIANCE},     INT_MIN, INT_MAX, VE, "aq_mode" },
##      { "autovariance",  "Auto-variance AQ",                0, AV_OPT_TYPE_CONST, {.i64 = X264_AQ_AUTOVARIANCE}, INT_MIN, INT_MAX, VE, "aq_mode" },
##      { "autovariance-biased", "Auto-variance AQ with bias to dark scenes", 0, AV_OPT_TYPE_CONST, {.i64 = X264_AQ_AUTOVARIANCE_BIASED}, INT_MIN, INT_MAX, VE, "aq_mode" },
##      { "aq-strength",   "AQ strength. Reduces blocking and blurring in flat and textured areas.", OFFSET(aq_strength), AV_OPT_TYPE_FLOAT, {.dbl = -1}, -1, FLT_MAX, VE},
##      { "psy",           "Use psychovisual optimizations.",                 OFFSET(psy),           AV_OPT_TYPE_BOOL,   { .i64 = -1 }, -1, 1, VE },
##      { "psy-rd",        "Strength of psychovisual optimization, in <psy-rd>:<psy-trellis> format.", OFFSET(psy_rd), AV_OPT_TYPE_STRING,  {0 }, 0, 0, VE},
##      { "rc-lookahead",  "Number of frames to look ahead for frametype and ratecontrol", OFFSET(rc_lookahead), AV_OPT_TYPE_INT, { .i64 = -1 }, -1, INT_MAX, VE },
##      { "weightb",       "Weighted prediction for B-frames.",               OFFSET(weightb),       AV_OPT_TYPE_BOOL,   { .i64 = -1 }, -1, 1, VE },
##      { "weightp",       "Weighted prediction analysis method.",            OFFSET(weightp),       AV_OPT_TYPE_INT,    { .i64 = -1 }, -1, INT_MAX, VE, "weightp" },
##      { "none",          NULL, 0, AV_OPT_TYPE_CONST, {.i64 = X264_WEIGHTP_NONE},   INT_MIN, INT_MAX, VE, "weightp" },
##      { "simple",        NULL, 0, AV_OPT_TYPE_CONST, {.i64 = X264_WEIGHTP_SIMPLE}, INT_MIN, INT_MAX, VE, "weightp" },
##      { "smart",         NULL, 0, AV_OPT_TYPE_CONST, {.i64 = X264_WEIGHTP_SMART},  INT_MIN, INT_MAX, VE, "weightp" },
##      { "ssim",          "Calculate and print SSIM stats.",                 OFFSET(ssim),          AV_OPT_TYPE_BOOL,   { .i64 = -1 }, -1, 1, VE },
##      { "intra-refresh", "Use Periodic Intra Refresh instead of IDR frames.",OFFSET(intra_refresh),AV_OPT_TYPE_BOOL,   { .i64 = -1 }, -1, 1, VE },
##      { "bluray-compat", "Bluray compatibility workarounds.",               OFFSET(bluray_compat) ,AV_OPT_TYPE_BOOL,   { .i64 = -1 }, -1, 1, VE },
##      { "b-bias",        "Influences how often B-frames are used",          OFFSET(b_bias),        AV_OPT_TYPE_INT,    { .i64 = INT_MIN}, INT_MIN, INT_MAX, VE },
##      { "b-pyramid",     "Keep some B-frames as references.",               OFFSET(b_pyramid),     AV_OPT_TYPE_INT,    { .i64 = -1 }, -1, INT_MAX, VE, "b_pyramid" },
##      { "none",          NULL,                                  0, AV_OPT_TYPE_CONST, {.i64 = X264_B_PYRAMID_NONE},   INT_MIN, INT_MAX, VE, "b_pyramid" },
##      { "strict",        "Strictly hierarchical pyramid",       0, AV_OPT_TYPE_CONST, {.i64 = X264_B_PYRAMID_STRICT}, INT_MIN, INT_MAX, VE, "b_pyramid" },
##      { "normal",        "Non-strict (not Blu-ray compatible)", 0, AV_OPT_TYPE_CONST, {.i64 = X264_B_PYRAMID_NORMAL}, INT_MIN, INT_MAX, VE, "b_pyramid" },
##      { "mixed-refs",    "One reference per partition, as opposed to one reference per macroblock", OFFSET(mixed_refs), AV_OPT_TYPE_BOOL, { .i64 = -1}, -1, 1, VE },
##      { "8x8dct",        "High profile 8x8 transform.",                     OFFSET(dct8x8),        AV_OPT_TYPE_BOOL,   { .i64 = -1 }, -1, 1, VE},
##      { "fast-pskip",    NULL,                                              OFFSET(fast_pskip),    AV_OPT_TYPE_BOOL,   { .i64 = -1 }, -1, 1, VE},
##      { "aud",           "Use access unit delimiters.",                     OFFSET(aud),           AV_OPT_TYPE_BOOL,   { .i64 = -1 }, -1, 1, VE},
##      { "mbtree",        "Use macroblock tree ratecontrol.",                OFFSET(mbtree),        AV_OPT_TYPE_BOOL,   { .i64 = -1 }, -1, 1, VE},
##      { "deblock",       "Loop filter parameters, in <alpha:beta> form.",   OFFSET(deblock),       AV_OPT_TYPE_STRING, { 0 },  0, 0, VE},
##      { "cplxblur",      "Reduce fluctuations in QP (before curve compression)", OFFSET(cplxblur), AV_OPT_TYPE_FLOAT,  {.dbl = -1 }, -1, FLT_MAX, VE},
##      { "partitions",    "A comma-separated list of partitions to consider. "
##                         "Possible values: p8x8, p4x4, b8x8, i8x8, i4x4, none, all", OFFSET(partitions), AV_OPT_TYPE_STRING, { 0 }, 0, 0, VE},
##      { "direct-pred",   "Direct MV prediction mode",                       OFFSET(direct_pred),   AV_OPT_TYPE_INT,    { .i64 = -1 }, -1, INT_MAX, VE, "direct-pred" },
##      { "none",          NULL,      0,    AV_OPT_TYPE_CONST, { .i64 = X264_DIRECT_PRED_NONE },     0, 0, VE, "direct-pred" },
##      { "spatial",       NULL,      0,    AV_OPT_TYPE_CONST, { .i64 = X264_DIRECT_PRED_SPATIAL },  0, 0, VE, "direct-pred" },
##      { "temporal",      NULL,      0,    AV_OPT_TYPE_CONST, { .i64 = X264_DIRECT_PRED_TEMPORAL }, 0, 0, VE, "direct-pred" },
##      { "auto",          NULL,      0,    AV_OPT_TYPE_CONST, { .i64 = X264_DIRECT_PRED_AUTO },     0, 0, VE, "direct-pred" },
##      { "slice-max-size","Limit the size of each slice in bytes",           OFFSET(slice_max_size),AV_OPT_TYPE_INT,    { .i64 = -1 }, -1, INT_MAX, VE },
##      { "stats",         "Filename for 2 pass stats",                       OFFSET(stats),         AV_OPT_TYPE_STRING, { 0 },  0,       0, VE },
##      { "nal-hrd",       "Signal HRD information (requires vbv-bufsize; "
##                         "cbr not allowed in .mp4)",                        OFFSET(nal_hrd),       AV_OPT_TYPE_INT,    { .i64 = -1 }, -1, INT_MAX, VE, "nal-hrd" },
##      { "none",          NULL, 0, AV_OPT_TYPE_CONST, {.i64 = X264_NAL_HRD_NONE}, INT_MIN, INT_MAX, VE, "nal-hrd" },
##      { "vbr",           NULL, 0, AV_OPT_TYPE_CONST, {.i64 = X264_NAL_HRD_VBR},  INT_MIN, INT_MAX, VE, "nal-hrd" },
##      { "cbr",           NULL, 0, AV_OPT_TYPE_CONST, {.i64 = X264_NAL_HRD_CBR},  INT_MIN, INT_MAX, VE, "nal-hrd" },
##      { "avcintra-class","AVC-Intra class 50/100/200",                      OFFSET(avcintra_class),AV_OPT_TYPE_INT,     { .i64 = -1 }, -1, 200   , VE},
##      { "me_method",    "Set motion estimation method",                     OFFSET(motion_est),    AV_OPT_TYPE_INT,    { .i64 = -1 }, -1, X264_ME_TESA, VE, "motion-est"},
##      { "motion-est",   "Set motion estimation method",                     OFFSET(motion_est),    AV_OPT_TYPE_INT,    { .i64 = -1 }, -1, X264_ME_TESA, VE, "motion-est"},
##      { "dia",           NULL, 0, AV_OPT_TYPE_CONST, { .i64 = X264_ME_DIA },  INT_MIN, INT_MAX, VE, "motion-est" },
##      { "hex",           NULL, 0, AV_OPT_TYPE_CONST, { .i64 = X264_ME_HEX },  INT_MIN, INT_MAX, VE, "motion-est" },
##      { "umh",           NULL, 0, AV_OPT_TYPE_CONST, { .i64 = X264_ME_UMH },  INT_MIN, INT_MAX, VE, "motion-est" },
##      { "esa",           NULL, 0, AV_OPT_TYPE_CONST, { .i64 = X264_ME_ESA },  INT_MIN, INT_MAX, VE, "motion-est" },
##      { "tesa",          NULL, 0, AV_OPT_TYPE_CONST, { .i64 = X264_ME_TESA }, INT_MIN, INT_MAX, VE, "motion-est" },
##      { "forced-idr",   "If forcing keyframes, force them as IDR frames.",                                  OFFSET(forced_idr),  AV_OPT_TYPE_BOOL,   { .i64 = 0 }, -1, 1, VE },
##      { "coder",    "Coder ",                                           OFFSET(coder), AV_OPT_TYPE_INT, { .i64 = -1 }, -1, 1, VE, "coder" },
##      { "default",          NULL, 0, AV_OPT_TYPE_CONST, { .i64 = -1 }, INT_MIN, INT_MAX, VE, "coder" },
##      { "cavlc",            NULL, 0, AV_OPT_TYPE_CONST, { .i64 = 0 },  INT_MIN, INT_MAX, VE, "coder" },
##      { "cabac",            NULL, 0, AV_OPT_TYPE_CONST, { .i64 = 1 },  INT_MIN, INT_MAX, VE, "coder" },
##      { "vlc",              NULL, 0, AV_OPT_TYPE_CONST, { .i64 = 0 },  INT_MIN, INT_MAX, VE, "coder" },
##      { "ac",               NULL, 0, AV_OPT_TYPE_CONST, { .i64 = 1 },  INT_MIN, INT_MAX, VE, "coder" },
##      { "b_strategy",   "Strategy to choose between I/P/B-frames",          OFFSET(b_frame_strategy), AV_OPT_TYPE_INT, { .i64 = -1 }, -1, 2, VE },
##      { "chromaoffset", "QP difference between chroma and luma",            OFFSET(chroma_offset), AV_OPT_TYPE_INT, { .i64 = 0 }, INT_MIN, INT_MAX, VE },
##      { "sc_threshold", "Scene change threshold",                           OFFSET(scenechange_threshold), AV_OPT_TYPE_INT, { .i64 = -1 }, INT_MIN, INT_MAX, VE },
##      { "noise_reduction", "Noise reduction",                               OFFSET(noise_reduction), AV_OPT_TYPE_INT, { .i64 = -1 }, INT_MIN, INT_MAX, VE },
##      { "x264-params",  "Override the x264 configuration using a :-separated list of key=value parameters", OFFSET(x264_params), AV_OPT_TYPE_DICT, { 0 }, 0, 0, VE },
##      { NULL },
##  };

var x264_defaults*: UncheckedArray[AVCodecDefault] = [["b", "0"], ["bf", "-1"],
    ["flags2", "0"], ["g", "-1"], ["i_qfactor", "-1"], ["b_qfactor", "-1"],
    ["qmin", "-1"], ["qmax", "-1"], ["qdiff", "-1"], ["qblur", "-1"], ["qcomp", "-1"],
    ["refs", "-1"], ["sc_threshold", "-1"], ["trellis", "-1"], ["nr", "-1"],
    ["me_range", "-1"], ["subq", "-1"], ["b_strategy", "-1"], ["keyint_min", "-1"],
    ["coder", "-1"], ["cmp", "-1"], ["threads", AV_STRINGIFY(X264_THREADS_AUTO)],
    ["thread_type", "0"], ["flags", "+cgop"], ["rc_init_occupancy", "-1"], [nil]]

var x264_class = AVClass(class_name = "libx264",
    item_name  = av_default_item_name,
    option     = options,
    version    = LIBAVUTIL_VERSION_INT,
)

proc X264_frame*(ctx: AVCodecContext; pkt: ptr AVPacket; frame: ptr AVFrame;got_packet: ptr cint): cint =
  var x4: ptr X264Context = ctx.priv_data
  var nal: ptr x264_nal_t
  var
    nnal: cint
    i: cint
    ret: cint
  var pic_out: x264_picture_t = [0]
  var pict_type: cint
  var bit_depth: cint
  var wallclock: int64 = 0
  var out_opaque: X264Opaque
  var sd: AVFrameSideData
  x264_picture_init(x4.pic)
  x4.pic.img.i_csp = x4.params.i_csp
  when X264_BUILD >= 153:
    bit_depth = x4.params.i_bitdepth
  else:
    bit_depth = x264_bit_depth
  if bit_depth > 8:
    x4.pic.img.i_csp = x4.pic.img.i_csp or X264_CSP_HIGH_DEPTH
  x4.pic.img.i_plane = avfmt2_num_planes(ctx.pix_fmt)
  if frame:
    i = 0
    while i < x4.pic.img.i_plane:
      x4.pic.img.plane[i] = frame.data[i]
      x4.pic.img.i_stride[i] = frame.linesize[i]
      inc(i)
    x4.pic.i_pts = frame.pts
    x4.reordered_opaque[x4.next_reordered_opaque].reordered_opaque = frame.reordered_opaque
    x4.reordered_opaque[x4.next_reordered_opaque].wallclock = wallclock
    if ctx.export_side_data and AV_CODEC_EXPORT_DATA_PRFT:
      x4.reordered_opaque[x4.next_reordered_opaque].wallclock = av_gettime()
    x4.pic.opaque = x4.reordered_opaque[x4.next_reordered_opaque]
    inc(x4.next_reordered_opaque)
    x4.next_reordered_opaque = x4.next_reordered_opaque mod x4.nb_reordered_opaque
    case frame.pict_type
    of AV_PICTURE_TYPE_I:
      x4.pic.i_type = if x4.forced_idr > 0: X264_TYPE_IDR else: X264_TYPE_KEYFRAME
    of AV_PICTURE_TYPE_P:
      x4.pic.i_type = X264_TYPE_P
    of AV_PICTURE_TYPE_B:
      x4.pic.i_type = X264_TYPE_B
    else:
      x4.pic.i_type = X264_TYPE_AUTO
    reconfig_encoder(ctx, frame)
    if x4.a53_cc:
      var sei_data: pointer
      var sei_size: csize_t
      ret = ff_alloc_a53_sei(frame, 0, sei_data, sei_size)
      if ret < 0:
        echo(ctx, "Not enough memory for closed captions, skipping\n")
      elif sei_data:
        x4.pic.extra_sei.payloads = av_mallocz(
            sizeof((x4.pic.extra_sei.payloads[0])))
        if x4.pic.extra_sei.payloads == nil:
          echo(ctx,  "Not enough memory for closed captions, skipping\n")
          av_free(sei_data)
        else:
          x4.pic.extra_sei.sei_free = av_free
          x4.pic.extra_sei.payloads[0].payload_size = sei_size
          x4.pic.extra_sei.payloads[0].payload = sei_data
          x4.pic.extra_sei.num_payloads = 1
          x4.pic.extra_sei.payloads[0].payload_type = 4
    sd = av_frame_get_side_data(frame, AV_FRAME_DATA_REGIONS_OF_INTEREST)
    if sd:
      if x4.params.rc.i_aq_mode == X264_AQ_NONE:
        if not x4.roi_warned:
          x4.roi_warned = 1
          echo(ctx, AV_LOG_WARNING, "Adaptive quantization must be enabled to use ROI encoding, skipping ROI.\n")
      else:
        if frame.interlaced_frame == 0:
          var mbx: cint = (frame.width + MB_SIZE - 1) div MB_SIZE
          var mby: cint = (frame.height + MB_SIZE - 1) div MB_SIZE
          var qp_range: cint = 51 + 6 * (bit_depth - 8)
          var nb_rois: cint
          var roi: AVRegionOfInterest
          var roi_size: uint32
          var qoffsets = newSeq[cfloat](mbx * mby)
          roi = cast[AVRegionOfInterest](sd.data)
          roi_size = roi.self_size
          if not roi_size or sd.size mod roi_size != 0:
            echo(ctx,  "Invalid AVRegionOfInterest.self_size.\n")
            return EINVAL
          nb_rois = sd.size div roi_size
          var i: cint = nb_rois - 1
          while i >= 0:
            var
              startx: cint
              endx: cint
              starty: cint
              endy: cint
            var qoffset: cfloat
            roi = cast[AVRegionOfInterest]((sd.data + roi_size * i))
            starty = min(mby, roi.top div MB_SIZE)
            endy = min(mby, (roi.bottom + MB_SIZE - 1) div MB_SIZE)
            startx = min(mbx, roi.left div MB_SIZE)
            endx = min(mbx, (roi.right + MB_SIZE - 1) div MB_SIZE)
            if roi.qoffset.den == 0:
              echo(ctx,  "AVRegionOfInterest.qoffset.den must not be zero.\n")
              return EINVAL
            qoffset = roi.qoffset.num * 1.0 div roi.qoffset.den
            qoffset = av_clipf(qoffset * qp_range, -qp_range, +qp_range)
            var y: cint = starty
            while y < endy:
              var x: cint = startx
              while x < endx:
                qoffsets[x + y * mbx] = qoffset
                inc(x)
              inc(y)
            dec(i)
          x4.pic.prop.quant_offsets = qoffsets
          x4.pic.prop.quant_offsets_free = av_free
        else:
          if not x4.roi_warned:
            x4.roi_warned = 1
            echo(ctx, AV_LOG_WARNING, "interlaced_frame not supported for ROI encoding yet, skipping ROI.\n")
  while true:
    if x264_encoder_encode(x4.enc, nal, nnal, if frame: x4.pic else: nil, pic_out) < 0:
      return AVERROR_EXTERNAL
    ret = encode_nals(ctx, pkt, nal, nnal)
    if ret < 0:
      return ret
    if not (not ret and not frame and x264_encoder_delayed_frames(x4.enc)):
      break
  if not ret:
    return 0
  pkt.pts = pic_out.i_pts
  pkt.dts = pic_out.i_dts
  out_opaque = pic_out.opaque
  if out_opaque >= x4.reordered_opaque and
      out_opaque < x4.reordered_opaque[x4.nb_reordered_opaque]:
    ctx.reordered_opaque = out_opaque.reordered_opaque
    wallclock = out_opaque.wallclock
  else:
    ##  Unexpected opaque pointer on picture output
    ctx.reordered_opaque = 0
  case pic_out.i_type
  of X264_TYPE_IDR, X264_TYPE_I:
    pict_type = AV_PICTURE_TYPE_I
  of X264_TYPE_P:
    pict_type = AV_PICTURE_TYPE_P
  of X264_TYPE_B, X264_TYPE_BREF:
    pict_type = AV_PICTURE_TYPE_B
  else:
    echo(ctx,  "Unknown picture encountered.\n")
    return AVERROR_EXTERNAL
  ctx.coded_frame.pict_type = pict_type
  pkt.flags = pkt.flags or AV_PKT_FLAG_KEY * pic_out.b_keyframe
  if ret:
    ff_side_data_set_encoder_stats(pkt, (pic_out.i_qpplus1 - 1) * FF_QP2LAMBDA, nil, 0,pict_type)
    if wallclock:
      ff_side_data_set_prft(pkt, wallclock)
    ctx.coded_frame.quality = (pic_out.i_qpplus1 - 1) * FF_QP2LAMBDA
  got_packet[] = ret
  return 0

proc X264_close*(avctx: AVCodecContext): cint =
  var x4: X264Context = avctx.priv_data
    x264_param_cleanup(x4.params)
  if x4.enc:
    x264_encoder_close(x4.enc)
    x4.enc = nil
  return 0


var ff_libx264_encoder = AVCodec(
    name             = "libx264",
    long_name        = "libx264 H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10",
    t             = AVMEDIA_TYPE_VIDEO,
    id               = AV_CODEC_ID_H264,
    priv_data_size   = sizeof(X264Context),
    init             = X264_init,
    encode2          = X264_frame,
    close            = X264_close,
    capabilities     = AV_CODEC_CAP_DELAY | AV_CODEC_CAP_AUTO_THREADS |AV_CODEC_CAP_ENCODER_REORDERED_OPAQUE,
    priv_class       = &x264_class,
    defaults         = x264_defaults,
    init_static_data = X264_init_static,
    caps_internal    = FF_CODEC_CAP_INIT_CLEANUP | FF_CODEC_CAP_INIT_THREADSAFE,
    wrapper_name     = "libx264",
};


var codec_list*: UncheckedArray[AVCodec] = [
    # ff_a64multi_encoder,
    # ff_a64multi5_encoder,
    # ff_alias_pix_encoder,
    # ff_amv_encoder,
    # ff_apng_encoder,
    # ff_asv1_encoder,
    # ff_asv2_encoder,
    # ff_avrp_encoder,
    # ff_avui_encoder,
    # ff_ayuv_encoder,
    # ff_bmp_encoder,
    # ff_cfhd_encoder,
    # ff_cinepak_encoder,
    # ff_cljr_encoder,
    # ff_comfortnoise_encoder,
    # ff_dnxhd_encoder,
    # ff_dpx_encoder,
    # ff_dvvideo_encoder,
    # ff_ffv1_encoder,
    # ff_ffvhuff_encoder,
    # ff_fits_encoder,
    # ff_flashsv_encoder,
    # ff_flashsv2_encoder,
    # ff_flv_encoder,
    # ff_gif_encoder,
    # ff_h261_encoder,
    # ff_h263_encoder,
    # ff_h263p_encoder,
    # ff_hap_encoder,
    # ff_huffyuv_encoder,
    # ff_jpeg2000_encoder,
    # ff_jpegls_encoder,
    # ff_ljpeg_encoder,
    # ff_magicyuv_encoder,
    # ff_mjpeg_encoder,
    # ff_mpeg1video_encoder,
    # ff_mpeg2video_encoder,
    # ff_mpeg4_encoder,
    # ff_msmpeg4v2_encoder,
    # ff_msmpeg4v3_encoder,
    # ff_msvideo1_encoder,
    # ff_pam_encoder,
    # ff_pbm_encoder,
    # ff_pcx_encoder,
    # ff_pgm_encoder,
    # ff_pgmyuv_encoder,
    # ff_png_encoder,
    # ff_ppm_encoder,
    # ff_prores_encoder,
    # ff_prores_aw_encoder,
    # ff_prores_ks_encoder,
    # ff_qtrle_encoder,
    # ff_r10k_encoder,
    # ff_r210_encoder,
    # ff_rawvideo_encoder,
    # ff_roq_encoder,
    # ff_rpza_encoder,
    # ff_rv10_encoder,
    # ff_rv20_encoder,
    # ff_s302m_encoder,
    # ff_sgi_encoder,
    # ff_snow_encoder,
    # ff_sunrast_encoder,
    # ff_svq1_encoder,
    # ff_targa_encoder,
    # ff_tiff_encoder,
    # ff_utvideo_encoder,
    # ff_v210_encoder,
    # ff_v308_encoder,
    # ff_v408_encoder,
    # ff_v410_encoder,
    # ff_vc2_encoder,
    # ff_wrapped_avframe_encoder,
    # ff_wmv1_encoder,
    # ff_wmv2_encoder,
    # ff_xbm_encoder,
    # ff_xface_encoder,
    # ff_xwd_encoder,
    # ff_y41p_encoder,
    # ff_yuv4_encoder,
    # ff_zlib_encoder,
    # ff_zmbv_encoder,
    # ff_aac_encoder,
    # ff_ac3_encoder,
    # ff_ac3_fixed_encoder,
    # ff_alac_encoder,
    # ff_aptx_encoder,
    # ff_aptx_hd_encoder,
    # ff_dca_encoder,
    # ff_eac3_encoder,
    # ff_flac_encoder,
    # ff_g723_1_encoder,
    # ff_mlp_encoder,
    # ff_mp2_encoder,
    # ff_mp2fixed_encoder,
    # ff_nellymoser_encoder,
    # ff_opus_encoder,
    # ff_ra_144_encoder,
    # ff_sbc_encoder,
    # ff_sonic_encoder,
    # ff_sonic_ls_encoder,
    # ff_truehd_encoder,
    # ff_tta_encoder,
    # ff_vorbis_encoder,
    # ff_wavpack_encoder,
    # ff_wmav1_encoder,
    # ff_wmav2_encoder,
    # ff_pcm_alaw_encoder,
    # ff_pcm_dvd_encoder,
    # ff_pcm_f32be_encoder,
    # ff_pcm_f32le_encoder,
    # ff_pcm_f64be_encoder,
    # ff_pcm_f64le_encoder,
    # ff_pcm_mulaw_encoder,
    # ff_pcm_s8_encoder,
    # ff_pcm_s8_planar_encoder,
    # ff_pcm_s16be_encoder,
    # ff_pcm_s16be_planar_encoder,
    # ff_pcm_s16le_encoder,
    # ff_pcm_s16le_planar_encoder,
    # ff_pcm_s24be_encoder,
    # ff_pcm_s24daud_encoder,
    # ff_pcm_s24le_encoder,
    # ff_pcm_s24le_planar_encoder,
    # ff_pcm_s32be_encoder,
    # ff_pcm_s32le_encoder,
    # ff_pcm_s32le_planar_encoder,
    # ff_pcm_s64be_encoder,
    # ff_pcm_s64le_encoder,
    # ff_pcm_u8_encoder,
    # ff_pcm_u16be_encoder,
    # ff_pcm_u16le_encoder,
    # ff_pcm_u24be_encoder,
    # ff_pcm_u24le_encoder,
    # ff_pcm_u32be_encoder,
    # ff_pcm_u32le_encoder,
    # ff_pcm_vidc_encoder,
    # ff_roq_dpcm_encoder,
    # ff_adpcm_adx_encoder,
    # ff_adpcm_argo_encoder,
    # ff_adpcm_g722_encoder,
    # ff_adpcm_g726_encoder,
    # ff_adpcm_g726le_encoder,
    # ff_adpcm_ima_apm_encoder,
    # ff_adpcm_ima_qt_encoder,
    # ff_adpcm_ima_ssi_encoder,
    # ff_adpcm_ima_wav_encoder,
    # ff_adpcm_ms_encoder,
    # ff_adpcm_swf_encoder,
    # ff_adpcm_yamaha_encoder,
    # ff_ssa_encoder,
    # ff_ass_encoder,
    # ff_dvbsub_encoder,
    # ff_dvdsub_encoder,
    # ff_movtext_encoder,
    # ff_srt_encoder,
    # ff_subrip_encoder,
    # ff_text_encoder,
    # ff_webvtt_encoder,
    # ff_xsub_encoder,
    # ff_libaom_av1_encoder,
    # ff_libgsm_encoder,
    # ff_libgsm_ms_encoder,
    # ff_libilbc_encoder,
    # ff_libmp3lame_encoder,
    # ff_libopenjpeg_encoder,
    # ff_libopus_encoder,
    # ff_libshine_encoder,
    # ff_libspeex_encoder,
    # ff_libtheora_encoder,
    # ff_libtwolame_encoder,
    # ff_libvorbis_encoder,
    # ff_libvpx_vp8_encoder,
    # ff_libvpx_vp9_encoder,
    # ff_libwavpack_encoder,
    # ff_libwebp_anim_encoder,
    # ff_libwebp_encoder,
    ff_libx264_encoder,
    # ff_libx264rgb_encoder,
    ff_libx265_encoder,
    # ff_libxvid_encoder,
    # ff_aac_mf_encoder,
    # ff_ac3_mf_encoder,
    # ff_h264_mf_encoder,
    # ff_h264_qsv_encoder,
    # ff_hevc_mf_encoder,
    # ff_hevc_qsv_encoder,
    # ff_mjpeg_qsv_encoder,
    # ff_mp3_mf_encoder,
    # ff_mpeg2_qsv_encoder,
    # ff_vp9_qsv_encoder,
    # ff_aasc_decoder,
    # ff_aic_decoder,
    # ff_alias_pix_decoder,
    # ff_agm_decoder,
    # ff_amv_decoder,
    # ff_anm_decoder,
    # ff_ansi_decoder,
    # ff_apng_decoder,
    # ff_arbc_decoder,
    # ff_asv1_decoder,
    # ff_asv2_decoder,
    # ff_aura_decoder,
    # ff_aura2_decoder,
    # ff_avrp_decoder,
    # ff_avrn_decoder,
    # ff_avs_decoder,
    # ff_avui_decoder,
    # ff_ayuv_decoder,
    # ff_bethsoftvid_decoder,
    # ff_bfi_decoder,
    # ff_bink_decoder,
    # ff_bitpacked_decoder,
    # ff_bmp_decoder,
    # ff_bmv_video_decoder,
    # ff_brender_pix_decoder,
    # ff_c93_decoder,
    # ff_cavs_decoder,
    # ff_cdgraphics_decoder,
    # ff_cdtoons_decoder,
    # ff_cdxl_decoder,
    # ff_cfhd_decoder,
    # ff_cinepak_decoder,
    # ff_clearvideo_decoder,
    # ff_cljr_decoder,
    # ff_cllc_decoder,
    # ff_comfortnoise_decoder,
    # ff_cpia_decoder,
    # ff_cscd_decoder,
    # ff_cyuv_decoder,
    # ff_dds_decoder,
    # ff_dfa_decoder,
    # ff_dirac_decoder,
    # ff_dnxhd_decoder,
    # ff_dpx_decoder,
    # ff_dsicinvideo_decoder,
    # ff_dvaudio_decoder,
    # ff_dvvideo_decoder,
    # ff_dxa_decoder,
    # ff_dxtory_decoder,
    # ff_dxv_decoder,
    # ff_eacmv_decoder,
    # ff_eamad_decoder,
    # ff_eatgq_decoder,
    # ff_eatgv_decoder,
    # ff_eatqi_decoder,
    # ff_eightbps_decoder,
    # ff_eightsvx_exp_decoder,
    # ff_eightsvx_fib_decoder,
    # ff_escape124_decoder,
    # ff_escape130_decoder,
    # ff_exr_decoder,
    # ff_ffv1_decoder,
    # ff_ffvhuff_decoder,
    # ff_fic_decoder,
    # ff_fits_decoder,
    # ff_flashsv_decoder,
    # ff_flashsv2_decoder,
    # ff_flic_decoder,
    # ff_flv_decoder,
    # ff_fmvc_decoder,
    # ff_fourxm_decoder,
    # ff_fraps_decoder,
    # ff_frwu_decoder,
    # ff_g2m_decoder,
    # ff_gdv_decoder,
    # ff_gif_decoder,
    # ff_h261_decoder,
    # ff_h263_decoder,
    # ff_h263i_decoder,
    # ff_h263p_decoder,
    ff_h264_decoder,
    # ff_h264_qsv_decoder,
    # ff_hap_decoder,
    ff_hevc_decoder,
    # ff_hevc_qsv_decoder,
    # ff_hnm4_video_decoder,
    # ff_hq_hqa_decoder,
    # ff_hqx_decoder,
    # ff_huffyuv_decoder,
    # ff_hymt_decoder,
    # ff_idcin_decoder,
    # ff_iff_ilbm_decoder,
    # ff_imm4_decoder,
    # ff_imm5_decoder,
    # ff_indeo2_decoder,
    # ff_indeo3_decoder,
    # ff_indeo4_decoder,
    # ff_indeo5_decoder,
    # ff_interplay_video_decoder,
    # ff_ipu_decoder,
    # ff_jpeg2000_decoder,
    # ff_jpegls_decoder,
    # ff_jv_decoder,
    # ff_kgv1_decoder,
    # ff_kmvc_decoder,
    # ff_lagarith_decoder,
    # ff_loco_decoder,
    # ff_lscr_decoder,
    # ff_m101_decoder,
    # ff_magicyuv_decoder,
    # ff_mdec_decoder,
    # ff_mimic_decoder,
    # ff_mjpeg_decoder,
    # ff_mjpegb_decoder,
    # ff_mmvideo_decoder,
    # ff_mobiclip_decoder,
    # ff_motionpixels_decoder,
    # ff_mpeg1video_decoder,
    # ff_mpeg2video_decoder,
    # ff_mpeg4_decoder,
    # ff_mpegvideo_decoder,
    # ff_mpeg2_qsv_decoder,
    # ff_msa1_decoder,
    # ff_mscc_decoder,
    # ff_msmpeg4v1_decoder,
    # ff_msmpeg4v2_decoder,
    # ff_msmpeg4v3_decoder,
    # ff_msrle_decoder,
    # ff_mss1_decoder,
    # ff_mss2_decoder,
    # ff_msvideo1_decoder,
    # ff_mszh_decoder,
    # ff_mts2_decoder,
    # ff_mv30_decoder,
    # ff_mvc1_decoder,
    # ff_mvc2_decoder,
    # ff_mvdv_decoder,
    # ff_mvha_decoder,
    # ff_mwsc_decoder,
    # ff_mxpeg_decoder,
    # ff_notchlc_decoder,
    # ff_nuv_decoder,
    # ff_paf_video_decoder,
    # ff_pam_decoder,
    # ff_pbm_decoder,
    # ff_pcx_decoder,
    # ff_pfm_decoder,
    # ff_pgm_decoder,
    # ff_pgmyuv_decoder,
    # ff_pgx_decoder,
    # ff_photocd_decoder,
    # ff_pictor_decoder,
    # ff_pixlet_decoder,
    # ff_png_decoder,
    # ff_ppm_decoder,
    # ff_prores_decoder,
    # ff_prosumer_decoder,
    # ff_psd_decoder,
    # ff_ptx_decoder,
    # ff_qdraw_decoder,
    # ff_qpeg_decoder,
    # ff_qtrle_decoder,
    # ff_r10k_decoder,
    # ff_r210_decoder,
    # ff_rasc_decoder,
    # ff_rawvideo_decoder,
    # ff_rl2_decoder,
    # ff_roq_decoder,
    # ff_rpza_decoder,
    # ff_rscc_decoder,
    # ff_rv10_decoder,
    # ff_rv20_decoder,
    # ff_rv30_decoder,
    # ff_rv40_decoder,
    # ff_s302m_decoder,
    # ff_sanm_decoder,
    # ff_scpr_decoder,
    # ff_screenpresso_decoder,
    # ff_sgi_decoder,
    # ff_sgirle_decoder,
    # ff_sheervideo_decoder,
    # ff_smacker_decoder,
    # ff_smc_decoder,
    # ff_smvjpeg_decoder,
    # ff_snow_decoder,
    # ff_sp5x_decoder,
    # ff_speedhq_decoder,
    # ff_srgc_decoder,
    # ff_sunrast_decoder,
    # ff_svq1_decoder,
    # ff_svq3_decoder,
    # ff_targa_decoder,
    # ff_targa_y216_decoder,
    # ff_tdsc_decoder,
    # ff_theora_decoder,
    # ff_thp_decoder,
    # ff_tiertexseqvideo_decoder,
    # ff_tiff_decoder,
    # ff_tmv_decoder,
    # ff_truemotion1_decoder,
    # ff_truemotion2_decoder,
    # ff_truemotion2rt_decoder,
    # ff_tscc_decoder,
    # ff_tscc2_decoder,
    # ff_txd_decoder,
    # ff_ulti_decoder,
    # ff_utvideo_decoder,
    # ff_v210_decoder,
    # ff_v210x_decoder,
    # ff_v308_decoder,
    # ff_v408_decoder,
    # ff_v410_decoder,
    # ff_vb_decoder,
    # ff_vble_decoder,
    # ff_vc1_decoder,
    # ff_vc1image_decoder,
    # ff_vc1_qsv_decoder,
    # ff_vcr1_decoder,
    # ff_vmdvideo_decoder,
    # ff_vmnc_decoder,
    # ff_vp3_decoder,
    # ff_vp4_decoder,
    # ff_vp5_decoder,
    # ff_vp6_decoder,
    # ff_vp6a_decoder,
    # ff_vp6f_decoder,
    # ff_vp7_decoder,
    # ff_vp8_decoder,
    # ff_vp9_decoder,
    # ff_vqa_decoder,
    # ff_webp_decoder,
    # ff_wcmv_decoder,
    # ff_wrapped_avframe_decoder,
    # ff_wmv1_decoder,
    # ff_wmv2_decoder,
    # ff_wmv3_decoder,
    # ff_wmv3image_decoder,
    # ff_wnv1_decoder,
    # ff_xan_wc3_decoder,
    # ff_xan_wc4_decoder,
    # ff_xbm_decoder,
    # ff_xface_decoder,
    # ff_xl_decoder,
    # ff_xpm_decoder,
    # ff_xwd_decoder,
    # ff_y41p_decoder,
    # ff_ylc_decoder,
    # ff_yop_decoder,
    # ff_yuv4_decoder,
    # ff_zero12v_decoder,
    # ff_zerocodec_decoder,
    # ff_zlib_decoder,
    # ff_zmbv_decoder,
    # ff_aac_decoder,
    # ff_aac_fixed_decoder,
    # ff_aac_latm_decoder,
    # ff_ac3_decoder,
    # ff_ac3_fixed_decoder,
    # ff_acelp_kelvin_decoder,
    # ff_alac_decoder,
    # ff_als_decoder,
    # ff_amrnb_decoder,
    # ff_amrwb_decoder,
    # ff_ape_decoder,
    # ff_aptx_decoder,
    # ff_aptx_hd_decoder,
    # ff_atrac1_decoder,
    # ff_atrac3_decoder,
    # ff_atrac3al_decoder,
    # ff_atrac3p_decoder,
    # ff_atrac3pal_decoder,
    # ff_atrac9_decoder,
    # ff_binkaudio_dct_decoder,
    # ff_binkaudio_rdft_decoder,
    # ff_bmv_audio_decoder,
    # ff_cook_decoder,
    # ff_dca_decoder,
    # ff_dolby_e_decoder,
    # ff_dsd_lsbf_decoder,
    # ff_dsd_msbf_decoder,
    # ff_dsd_lsbf_planar_decoder,
    # ff_dsd_msbf_planar_decoder,
    # ff_dsicinaudio_decoder,
    # ff_dss_sp_decoder,
    # ff_dst_decoder,
    # ff_eac3_decoder,
    # ff_evrc_decoder,
    # ff_fastaudio_decoder,
    # ff_ffwavesynth_decoder,
    # ff_flac_decoder,
    # ff_g723_1_decoder,
    # ff_g729_decoder,
    # ff_gsm_decoder,
    # ff_gsm_ms_decoder,
    # ff_hca_decoder,
    # ff_hcom_decoder,
    # ff_iac_decoder,
    # ff_ilbc_decoder,
    # ff_imc_decoder,
    # ff_interplay_acm_decoder,
    # ff_mace3_decoder,
    # ff_mace6_decoder,
    # ff_metasound_decoder,
    # ff_mlp_decoder,
    # ff_mp1_decoder,
    # ff_mp1float_decoder,
    # ff_mp2_decoder,
    # ff_mp2float_decoder,
    # ff_mp3float_decoder,
    # ff_mp3_decoder,
    # ff_mp3adufloat_decoder,
    # ff_mp3adu_decoder,
    # ff_mp3on4float_decoder,
    # ff_mp3on4_decoder,
    # ff_mpc7_decoder,
    # ff_mpc8_decoder,
    # ff_nellymoser_decoder,
    # ff_on2avc_decoder,
    # ff_opus_decoder,
    # ff_paf_audio_decoder,
    # ff_qcelp_decoder,
    # ff_qdm2_decoder,
    # ff_qdmc_decoder,
    # ff_ra_144_decoder,
    # ff_ra_288_decoder,
    # ff_ralf_decoder,
    # ff_sbc_decoder,
    # ff_shorten_decoder,
    # ff_sipr_decoder,
    # ff_siren_decoder,
    # ff_smackaud_decoder,
    # ff_sonic_decoder,
    # ff_tak_decoder,
    # ff_truehd_decoder,
    # ff_truespeech_decoder,
    # ff_tta_decoder,
    # ff_twinvq_decoder,
    # ff_vmdaudio_decoder,
    # ff_vorbis_decoder,
    # ff_wavpack_decoder,
    # ff_wmalossless_decoder,
    # ff_wmapro_decoder,
    # ff_wmav1_decoder,
    # ff_wmav2_decoder,
    # ff_wmavoice_decoder,
    # ff_ws_snd1_decoder,
    # ff_xma1_decoder,
    # ff_xma2_decoder,
    # ff_pcm_alaw_decoder,
    # ff_pcm_bluray_decoder,
    # ff_pcm_dvd_decoder,
    # ff_pcm_f16le_decoder,
    # ff_pcm_f24le_decoder,
    # ff_pcm_f32be_decoder,
    # ff_pcm_f32le_decoder,
    # ff_pcm_f64be_decoder,
    # ff_pcm_f64le_decoder,
    # ff_pcm_lxf_decoder,
    # ff_pcm_mulaw_decoder,
    # ff_pcm_s8_decoder,
    # ff_pcm_s8_planar_decoder,
    # ff_pcm_s16be_decoder,
    # ff_pcm_s16be_planar_decoder,
    # ff_pcm_s16le_decoder,
    # ff_pcm_s16le_planar_decoder,
    # ff_pcm_s24be_decoder,
    # ff_pcm_s24daud_decoder,
    # ff_pcm_s24le_decoder,
    # ff_pcm_s24le_planar_decoder,
    # ff_pcm_s32be_decoder,
    # ff_pcm_s32le_decoder,
    # ff_pcm_s32le_planar_decoder,
    # ff_pcm_s64be_decoder,
    # ff_pcm_s64le_decoder,
    # ff_pcm_u8_decoder,
    # ff_pcm_u16be_decoder,
    # ff_pcm_u16le_decoder,
    # ff_pcm_u24be_decoder,
    # ff_pcm_u24le_decoder,
    # ff_pcm_u32be_decoder,
    # ff_pcm_u32le_decoder,
    # ff_pcm_vidc_decoder,
    # ff_derf_dpcm_decoder,
    # ff_gremlin_dpcm_decoder,
    # ff_interplay_dpcm_decoder,
    # ff_roq_dpcm_decoder,
    # ff_sdx2_dpcm_decoder,
    # ff_sol_dpcm_decoder,
    # ff_xan_dpcm_decoder,
    # ff_adpcm_4xm_decoder,
    # ff_adpcm_adx_decoder,
    # ff_adpcm_afc_decoder,
    # ff_adpcm_agm_decoder,
    # ff_adpcm_aica_decoder,
    # ff_adpcm_argo_decoder,
    # ff_adpcm_ct_decoder,
    # ff_adpcm_dtk_decoder,
    # ff_adpcm_ea_decoder,
    # ff_adpcm_ea_maxis_xa_decoder,
    # ff_adpcm_ea_r1_decoder,
    # ff_adpcm_ea_r2_decoder,
    # ff_adpcm_ea_r3_decoder,
    # ff_adpcm_ea_xas_decoder,
    # ff_adpcm_g722_decoder,
    # ff_adpcm_g726_decoder,
    # ff_adpcm_g726le_decoder,
    # ff_adpcm_ima_amv_decoder,
    # ff_adpcm_ima_alp_decoder,
    # ff_adpcm_ima_apc_decoder,
    # ff_adpcm_ima_apm_decoder,
    # ff_adpcm_ima_cunning_decoder,
    # ff_adpcm_ima_dat4_decoder,
    # ff_adpcm_ima_dk3_decoder,
    # ff_adpcm_ima_dk4_decoder,
    # ff_adpcm_ima_ea_eacs_decoder,
    # ff_adpcm_ima_ea_sead_decoder,
    # ff_adpcm_ima_iss_decoder,
    # ff_adpcm_ima_moflex_decoder,
    # ff_adpcm_ima_mtf_decoder,
    # ff_adpcm_ima_oki_decoder,
    # ff_adpcm_ima_qt_decoder,
    # ff_adpcm_ima_rad_decoder,
    # ff_adpcm_ima_ssi_decoder,
    # ff_adpcm_ima_smjpeg_decoder,
    # ff_adpcm_ima_wav_decoder,
    # ff_adpcm_ima_ws_decoder,
    # ff_adpcm_ms_decoder,
    # ff_adpcm_mtaf_decoder,
    # ff_adpcm_psx_decoder,
    # ff_adpcm_sbpro_2_decoder,
    # ff_adpcm_sbpro_3_decoder,
    # ff_adpcm_sbpro_4_decoder,
    # ff_adpcm_swf_decoder,
    # ff_adpcm_thp_decoder,
    # ff_adpcm_thp_le_decoder,
    # ff_adpcm_vima_decoder,
    # ff_adpcm_xa_decoder,
    # ff_adpcm_yamaha_decoder,
    # ff_adpcm_zork_decoder,
    # ff_ssa_decoder,
    # ff_ass_decoder,
    # ff_ccaption_decoder,
    # ff_dvbsub_decoder,
    # ff_dvdsub_decoder,
    # ff_jacosub_decoder,
    # ff_microdvd_decoder,
    # ff_movtext_decoder,
    # ff_mpl2_decoder,
    # ff_pgssub_decoder,
    # ff_pjs_decoder,
    # ff_realtext_decoder,
    # ff_sami_decoder,
    # ff_srt_decoder,
    # ff_stl_decoder,
    # ff_subrip_decoder,
    # ff_subviewer_decoder,
    # ff_subviewer1_decoder,
    # ff_text_decoder,
    # ff_vplayer_decoder,
    # ff_webvtt_decoder,
    # ff_xsub_decoder,
    # ff_libdav1d_decoder,
    # ff_libgsm_decoder,
    # ff_libgsm_ms_decoder,
    # ff_libilbc_decoder,
    # ff_libopenjpeg_decoder,
    # ff_libopus_decoder,
    # ff_libspeex_decoder,
    # ff_libvorbis_decoder,
    # ff_libvpx_vp8_decoder,
    # ff_libvpx_vp9_decoder,
    # ff_bintext_decoder,
    # ff_xbin_decoder,
    # ff_idf_decoder,
    # ff_libaom_av1_decoder,
    # ff_av1_decoder,
    # ff_mjpeg_qsv_decoder,
    # ff_vp8_qsv_decoder,
    # ff_vp9_qsv_decoder,
 nil]

proc av_codec_init_static*() =
  var i: cint = 0
  for i in 0..codec_list[i].high:
      codec_list[i].init_static_data(codec_list[i])


proc av_codec_iterate*(opaque: pointer): AVCodec =
  var i: uint64 = cast[uint64](opaque)
  var c: AVCodec = codec_list[i]
  Pthread_once(av_codec_static_init, av_codec_init_static)
  if c:
    opaque[] = cast[pointer]((i + 1))
  return c


proc codec_child_class_next*(prev: AVClass): AVClass =
  var iter: pointer
  var c: AVCodec
  while prev and (c = av_codec_iterate(iter)):
    if c.priv_class == prev:
      break
  while c = av_codec_iterate(iter):
    if c.priv_class:
      return c.priv_class
  return nil

proc codec_child_class_iterate*(iter: pointer): AVClass =
  var c: AVCodec
  while c = av_codec_iterate(iter):
    if c.priv_class:
      return c.priv_class
  return nil

var  avcodec_options: seq[AVOption] = @[]
##  {"b", "set bitrate (in bits/s)", offsetof(AVCodecContext,bit_rate), AV_OPT_TYPE_INT64, {.i64 = AV_CODEC_DEFAULT_BITRATE }, 0, int64.high, A|V|E},
##  {"ab", "set bitrate (in bits/s)", offsetof(AVCodecContext,bit_rate), AV_OPT_TYPE_INT64, {.i64 = 128*1000 }, 0, INT_MAX, A|E},
##  {"bt", "Set video bitrate tolerance (in bits/s). In 1-pass mode, bitrate tolerance specifies how far "
##         "ratecontrol is willing to deviate from the target average bitrate value. This is not related "
##         "to minimum/maximum bitrate. Lowering tolerance too much has an adverse effect on quality.",
##         offsetof(AVCodecContext,bit_rate_tolerance), AV_OPT_TYPE_INT, {.i64 = AV_CODEC_DEFAULT_BITRATE*20 }, 1, INT_MAX, V|E},
##  {"flags", NULL, offsetof(AVCodecContext,flags), AV_OPT_TYPE_FLAGS, {.i64 = DEFAULT }, 0, UINT_MAX, V|A|S|E|D, "flags"},
##  {"unaligned", "allow decoders to produce unaligned output", 0, AV_OPT_TYPE_CONST, { .i64 = AV_CODEC_FLAG_UNALIGNED }, INT_MIN, INT_MAX, V | D, "flags" },
##  {"mv4", "use four motion vectors per macroblock (MPEG-4)", 0, AV_OPT_TYPE_CONST, {.i64 = AV_CODEC_FLAG_4MV }, INT_MIN, INT_MAX, V|E, "flags"},
##  {"qpel", "use 1/4-pel motion compensation", 0, AV_OPT_TYPE_CONST, {.i64 = AV_CODEC_FLAG_QPEL }, INT_MIN, INT_MAX, V|E, "flags"},
##  {"loop", "use loop filter", 0, AV_OPT_TYPE_CONST, {.i64 = AV_CODEC_FLAG_LOOP_FILTER }, INT_MIN, INT_MAX, V|E, "flags"},
##  {"qscale", "use fixed qscale", 0, AV_OPT_TYPE_CONST, {.i64 = AV_CODEC_FLAG_QSCALE }, INT_MIN, INT_MAX, 0, "flags"},
##  {"pass1", "use internal 2-pass ratecontrol in first  pass mode", 0, AV_OPT_TYPE_CONST, {.i64 = AV_CODEC_FLAG_PASS1 }, INT_MIN, INT_MAX, 0, "flags"},
##  {"pass2", "use internal 2-pass ratecontrol in second pass mode", 0, AV_OPT_TYPE_CONST, {.i64 = AV_CODEC_FLAG_PASS2 }, INT_MIN, INT_MAX, 0, "flags"},
##  {"gray", "only decode/encode grayscale", 0, AV_OPT_TYPE_CONST, {.i64 = AV_CODEC_FLAG_GRAY }, INT_MIN, INT_MAX, V|E|D, "flags"},
##  {"psnr", "error[?] variables will be set during encoding", 0, AV_OPT_TYPE_CONST, {.i64 = AV_CODEC_FLAG_PSNR }, INT_MIN, INT_MAX, V|E, "flags"},
##  {"truncated", "Input bitstream might be randomly truncated", 0, AV_OPT_TYPE_CONST, {.i64 = AV_CODEC_FLAG_TRUNCATED }, INT_MIN, INT_MAX, V|D, "flags"},
##  {"ildct", "use interlaced DCT", 0, AV_OPT_TYPE_CONST, {.i64 = AV_CODEC_FLAG_INTERLACED_DCT }, INT_MIN, INT_MAX, V|E, "flags"},
##  {"low_delay", "force low delay", 0, AV_OPT_TYPE_CONST, {.i64 = AV_CODEC_FLAG_LOW_DELAY }, INT_MIN, INT_MAX, V|D|E, "flags"},
##  {"global_header", "place global headers in extradata instead of every keyframe", 0, AV_OPT_TYPE_CONST, {.i64 = AV_CODEC_FLAG_GLOBAL_HEADER }, INT_MIN, INT_MAX, V|A|E, "flags"},
##  {"bitexact", "use only bitexact functions (except (I)DCT)", 0, AV_OPT_TYPE_CONST, {.i64 = AV_CODEC_FLAG_BITEXACT }, INT_MIN, INT_MAX, A|V|S|D|E, "flags"},
##  {"aic", "H.263 advanced intra coding / MPEG-4 AC prediction", 0, AV_OPT_TYPE_CONST, {.i64 = AV_CODEC_FLAG_AC_PRED }, INT_MIN, INT_MAX, V|E, "flags"},
##  {"ilme", "interlaced motion estimation", 0, AV_OPT_TYPE_CONST, {.i64 = AV_CODEC_FLAG_INTERLACED_ME }, INT_MIN, INT_MAX, V|E, "flags"},
##  {"cgop", "closed GOP", 0, AV_OPT_TYPE_CONST, {.i64 = AV_CODEC_FLAG_CLOSED_GOP }, INT_MIN, INT_MAX, V|E, "flags"},
##  {"output_corrupt", "Output even potentially corrupted frames", 0, AV_OPT_TYPE_CONST, {.i64 = AV_CODEC_FLAG_OUTPUT_CORRUPT }, INT_MIN, INT_MAX, V|D, "flags"},
##  {"drop_changed", "Drop frames whose parameters differ from first decoded frame", 0, AV_OPT_TYPE_CONST, {.i64 = AV_CODEC_FLAG_DROPCHANGED }, INT_MIN, INT_MAX, A|V|D, "flags"},
##  {"flags2", NULL, offsetof(AVCodecContext,flags2), AV_OPT_TYPE_FLAGS, {.i64 = DEFAULT}, 0, UINT_MAX, V|A|E|D|S, "flags2"},
##  {"fast", "allow non-spec-compliant speedup tricks", 0, AV_OPT_TYPE_CONST, {.i64 = AV_CODEC_FLAG2_FAST }, INT_MIN, INT_MAX, V|E, "flags2"},
##  {"noout", "skip bitstream encoding", 0, AV_OPT_TYPE_CONST, {.i64 = AV_CODEC_FLAG2_NO_OUTPUT }, INT_MIN, INT_MAX, V|E, "flags2"},
##  {"ignorecrop", "ignore cropping information from sps", 0, AV_OPT_TYPE_CONST, {.i64 = AV_CODEC_FLAG2_IGNORE_CROP }, INT_MIN, INT_MAX, V|D, "flags2"},
##  {"local_header", "place global headers at every keyframe instead of in extradata", 0, AV_OPT_TYPE_CONST, {.i64 = AV_CODEC_FLAG2_LOCAL_HEADER }, INT_MIN, INT_MAX, V|E, "flags2"},
##  {"chunks", "Frame data might be split into multiple chunks", 0, AV_OPT_TYPE_CONST, {.i64 = AV_CODEC_FLAG2_CHUNKS }, INT_MIN, INT_MAX, V|D, "flags2"},
##  {"showall", "Show all frames before the first keyframe", 0, AV_OPT_TYPE_CONST, {.i64 = AV_CODEC_FLAG2_SHOW_ALL }, INT_MIN, INT_MAX, V|D, "flags2"},
##  {"export_mvs", "export motion vectors through frame side data", 0, AV_OPT_TYPE_CONST, {.i64 = AV_CODEC_FLAG2_EXPORT_MVS}, INT_MIN, INT_MAX, V|D, "flags2"},
##  {"skip_manual", "do not skip samples and export skip information as frame side data", 0, AV_OPT_TYPE_CONST, {.i64 = AV_CODEC_FLAG2_SKIP_MANUAL}, INT_MIN, INT_MAX, A|D, "flags2"},
##  {"ass_ro_flush_noop", "do not reset ASS ReadOrder field on flush", 0, AV_OPT_TYPE_CONST, {.i64 = AV_CODEC_FLAG2_RO_FLUSH_NOOP}, INT_MIN, INT_MAX, S|D, "flags2"},
##  {"export_side_data", "Export metadata as side data", offsetof(AVCodecContext,export_side_data), AV_OPT_TYPE_FLAGS, {.i64 = DEFAULT}, 0, UINT_MAX, A|V|S|D|E, "export_side_data"},
##  {"mvs", "export motion vectors through frame side data", 0, AV_OPT_TYPE_CONST, {.i64 = AV_CODEC_EXPORT_DATA_MVS}, INT_MIN, INT_MAX, V|D, "export_side_data"},
##  {"prft", "export Producer Reference Time through packet side data", 0, AV_OPT_TYPE_CONST, {.i64 = AV_CODEC_EXPORT_DATA_PRFT}, INT_MIN, INT_MAX, A|V|S|E, "export_side_data"},
##  {"venc_params", "export video encoding parameters through frame side data", 0, AV_OPT_TYPE_CONST, {.i64 = AV_CODEC_EXPORT_DATA_VIDEO_ENC_PARAMS}, INT_MIN, INT_MAX, V|D, "export_side_data"},
##  {"time_base", NULL, offsetof(AVCodecContext,time_base), AV_OPT_TYPE_RATIONAL, {.dbl = 0}, 0, INT_MAX},
##  {"g", "set the group of picture (GOP) size", offsetof(AVCodecContext,gop_size), AV_OPT_TYPE_INT, {.i64 = 12 }, INT_MIN, INT_MAX, V|E},
##  {"ar", "set audio sampling rate (in Hz)", offsetof(AVCodecContext,sample_rate), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, 0, INT_MAX, A|D|E},
##  {"ac", "set number of audio channels", offsetof(AVCodecContext,channels), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, 0, INT_MAX, A|D|E},
##  {"cutoff", "set cutoff bandwidth", offsetof(AVCodecContext,cutoff), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, INT_MIN, INT_MAX, A|E},
##  {"frame_size", NULL, offsetof(AVCodecContext,frame_size), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, 0, INT_MAX, A|E},
##  {"frame_number", NULL, offsetof(AVCodecContext,frame_number), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, INT_MIN, INT_MAX},
##  {"delay", NULL, offsetof(AVCodecContext,delay), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, INT_MIN, INT_MAX},
##  {"qcomp", "video quantizer scale compression (VBR). Constant of ratecontrol equation. "
##            "Recommended range for default rc_eq: 0.0-1.0",
##            offsetof(AVCodecContext,qcompress), AV_OPT_TYPE_FLOAT, {.dbl = 0.5 }, -FLT_MAX, FLT_MAX, V|E},
##  {"qblur", "video quantizer scale blur (VBR)", offsetof(AVCodecContext,qblur), AV_OPT_TYPE_FLOAT, {.dbl = 0.5 }, -1, FLT_MAX, V|E},
##  {"qmin", "minimum video quantizer scale (VBR)", offsetof(AVCodecContext,qmin), AV_OPT_TYPE_INT, {.i64 = 2 }, -1, 69, V|E},
##  {"qmax", "maximum video quantizer scale (VBR)", offsetof(AVCodecContext,qmax), AV_OPT_TYPE_INT, {.i64 = 31 }, -1, 1024, V|E},
##  {"qdiff", "maximum difference between the quantizer scales (VBR)", offsetof(AVCodecContext,max_qdiff), AV_OPT_TYPE_INT, {.i64 = 3 }, INT_MIN, INT_MAX, V|E},
##  {"bf", "set maximum number of B-frames between non-B-frames", offsetof(AVCodecContext,max_b_frames), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, -1, INT_MAX, V|E},
##  {"b_qfactor", "QP factor between P- and B-frames", offsetof(AVCodecContext,b_quant_factor), AV_OPT_TYPE_FLOAT, {.dbl = 1.25 }, -FLT_MAX, FLT_MAX, V|E},
##  #if FF_API_PRIVATE_OPT
##  {"b_strategy", "strategy to choose between I/P/B-frames", offsetof(AVCodecContext,b_frame_strategy), AV_OPT_TYPE_INT, {.i64 = 0 }, INT_MIN, INT_MAX, V|E},
##  {"ps", "RTP payload size in bytes", offsetof(AVCodecContext,rtp_payload_size), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, INT_MIN, INT_MAX, V|E},
##  #endif
##  #if FF_API_STAT_BITS
##  {"mv_bits", NULL, offsetof(AVCodecContext,mv_bits), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, INT_MIN, INT_MAX},
##  {"header_bits", NULL, offsetof(AVCodecContext,header_bits), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, INT_MIN, INT_MAX},
##  {"i_tex_bits", NULL, offsetof(AVCodecContext,i_tex_bits), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, INT_MIN, INT_MAX},
##  {"p_tex_bits", NULL, offsetof(AVCodecContext,p_tex_bits), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, INT_MIN, INT_MAX},
##  {"i_count", NULL, offsetof(AVCodecContext,i_count), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, INT_MIN, INT_MAX},
##  {"p_count", NULL, offsetof(AVCodecContext,p_count), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, INT_MIN, INT_MAX},
##  {"skip_count", NULL, offsetof(AVCodecContext,skip_count), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, INT_MIN, INT_MAX},
##  {"misc_bits", NULL, offsetof(AVCodecContext,misc_bits), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, INT_MIN, INT_MAX},
##  {"frame_bits", NULL, offsetof(AVCodecContext,frame_bits), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, INT_MIN, INT_MAX},
##  #endif
##  {"codec_tag", NULL, offsetof(AVCodecContext,codec_tag), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, INT_MIN, INT_MAX},
##  {"bug", "work around not autodetected encoder bugs", offsetof(AVCodecContext,workaround_bugs), AV_OPT_TYPE_FLAGS, {.i64 = FF_BUG_AUTODETECT }, INT_MIN, INT_MAX, V|D, "bug"},
##  {"autodetect", NULL, 0, AV_OPT_TYPE_CONST, {.i64 = FF_BUG_AUTODETECT }, INT_MIN, INT_MAX, V|D, "bug"},
##  {"xvid_ilace", "Xvid interlacing bug (autodetected if FOURCC == XVIX)", 0, AV_OPT_TYPE_CONST, {.i64 = FF_BUG_XVID_ILACE }, INT_MIN, INT_MAX, V|D, "bug"},
##  {"ump4", "(autodetected if FOURCC == UMP4)", 0, AV_OPT_TYPE_CONST, {.i64 = FF_BUG_UMP4 }, INT_MIN, INT_MAX, V|D, "bug"},
##  {"no_padding", "padding bug (autodetected)", 0, AV_OPT_TYPE_CONST, {.i64 = FF_BUG_NO_PADDING }, INT_MIN, INT_MAX, V|D, "bug"},
##  {"amv", NULL, 0, AV_OPT_TYPE_CONST, {.i64 = FF_BUG_AMV }, INT_MIN, INT_MAX, V|D, "bug"},
##  {"qpel_chroma", NULL, 0, AV_OPT_TYPE_CONST, {.i64 = FF_BUG_QPEL_CHROMA }, INT_MIN, INT_MAX, V|D, "bug"},
##  {"std_qpel", "old standard qpel (autodetected per FOURCC/version)", 0, AV_OPT_TYPE_CONST, {.i64 = FF_BUG_STD_QPEL }, INT_MIN, INT_MAX, V|D, "bug"},
##  {"qpel_chroma2", NULL, 0, AV_OPT_TYPE_CONST, {.i64 = FF_BUG_QPEL_CHROMA2 }, INT_MIN, INT_MAX, V|D, "bug"},
##  {"direct_blocksize", "direct-qpel-blocksize bug (autodetected per FOURCC/version)", 0, AV_OPT_TYPE_CONST, {.i64 = FF_BUG_DIRECT_BLOCKSIZE }, INT_MIN, INT_MAX, V|D, "bug"},
##  {"edge", "edge padding bug (autodetected per FOURCC/version)", 0, AV_OPT_TYPE_CONST, {.i64 = FF_BUG_EDGE }, INT_MIN, INT_MAX, V|D, "bug"},
##  {"hpel_chroma", NULL, 0, AV_OPT_TYPE_CONST, {.i64 = FF_BUG_HPEL_CHROMA }, INT_MIN, INT_MAX, V|D, "bug"},
##  {"dc_clip", NULL, 0, AV_OPT_TYPE_CONST, {.i64 = FF_BUG_DC_CLIP }, INT_MIN, INT_MAX, V|D, "bug"},
##  {"ms", "work around various bugs in Microsoft's broken decoders", 0, AV_OPT_TYPE_CONST, {.i64 = FF_BUG_MS }, INT_MIN, INT_MAX, V|D, "bug"},
##  {"trunc", "truncated frames", 0, AV_OPT_TYPE_CONST, {.i64 = FF_BUG_TRUNCATED}, INT_MIN, INT_MAX, V|D, "bug"},
##  {"iedge", NULL, 0, AV_OPT_TYPE_CONST, {.i64 = FF_BUG_IEDGE }, INT_MIN, INT_MAX, V|D, "bug"},
##  {"strict", "how strictly to follow the standards", offsetof(AVCodecContext,strict_std_compliance), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, INT_MIN, INT_MAX, A|V|D|E, "strict"},
##  {"very", "strictly conform to a older more strict version of the spec or reference software", 0, AV_OPT_TYPE_CONST, {.i64 = FF_COMPLIANCE_VERY_STRICT }, INT_MIN, INT_MAX, A|V|D|E, "strict"},
##  {"strict", "strictly conform to all the things in the spec no matter what the consequences", 0, AV_OPT_TYPE_CONST, {.i64 = FF_COMPLIANCE_STRICT }, INT_MIN, INT_MAX, A|V|D|E, "strict"},
##  {"normal", NULL, 0, AV_OPT_TYPE_CONST, {.i64 = FF_COMPLIANCE_NORMAL }, INT_MIN, INT_MAX, A|V|D|E, "strict"},
##  {"unofficial", "allow unofficial extensions", 0, AV_OPT_TYPE_CONST, {.i64 = FF_COMPLIANCE_UNOFFICIAL }, INT_MIN, INT_MAX, A|V|D|E, "strict"},
##  {"experimental", "allow non-standardized experimental things", 0, AV_OPT_TYPE_CONST, {.i64 = FF_COMPLIANCE_EXPERIMENTAL }, INT_MIN, INT_MAX, A|V|D|E, "strict"},
##  {"b_qoffset", "QP offset between P- and B-frames", offsetof(AVCodecContext,b_quant_offset), AV_OPT_TYPE_FLOAT, {.dbl = 1.25 }, -FLT_MAX, FLT_MAX, V|E},
##  {"err_detect", "set error detection flags", offsetof(AVCodecContext,err_recognition), AV_OPT_TYPE_FLAGS, {.i64 = 0 }, INT_MIN, INT_MAX, A|V|D, "err_detect"},
##  {"crccheck", "verify embedded CRCs", 0, AV_OPT_TYPE_CONST, {.i64 = AV_EF_CRCCHECK }, INT_MIN, INT_MAX, A|V|D, "err_detect"},
##  {"bitstream", "detect bitstream specification deviations", 0, AV_OPT_TYPE_CONST, {.i64 = AV_EF_BITSTREAM }, INT_MIN, INT_MAX, A|V|D, "err_detect"},
##  {"buffer", "detect improper bitstream length", 0, AV_OPT_TYPE_CONST, {.i64 = AV_EF_BUFFER }, INT_MIN, INT_MAX, A|V|D, "err_detect"},
##  {"explode", "abort decoding on minor error detection", 0, AV_OPT_TYPE_CONST, {.i64 = AV_EF_EXPLODE }, INT_MIN, INT_MAX, A|V|D, "err_detect"},
##  {"ignore_err", "ignore errors", 0, AV_OPT_TYPE_CONST, {.i64 = AV_EF_IGNORE_ERR }, INT_MIN, INT_MAX, A|V|D, "err_detect"},
##  {"careful",    "consider things that violate the spec, are fast to check and have not been seen in the wild as errors", 0, AV_OPT_TYPE_CONST, {.i64 = AV_EF_CAREFUL }, INT_MIN, INT_MAX, A|V|D, "err_detect"},
##  {"compliant",  "consider all spec non compliancies as errors", 0, AV_OPT_TYPE_CONST, {.i64 = AV_EF_COMPLIANT | AV_EF_CAREFUL }, INT_MIN, INT_MAX, A|V|D, "err_detect"},
##  {"aggressive", "consider things that a sane encoder should not do as an error", 0, AV_OPT_TYPE_CONST, {.i64 = AV_EF_AGGRESSIVE | AV_EF_COMPLIANT | AV_EF_CAREFUL}, INT_MIN, INT_MAX, A|V|D, "err_detect"},
##  {"has_b_frames", NULL, offsetof(AVCodecContext,has_b_frames), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, 0, INT_MAX},
##  {"block_align", NULL, offsetof(AVCodecContext,block_align), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, 0, INT_MAX},
##  #if FF_API_PRIVATE_OPT
##  {"mpeg_quant", "use MPEG quantizers instead of H.263", offsetof(AVCodecContext,mpeg_quant), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, INT_MIN, INT_MAX, V|E},
##  #endif
##  {"rc_override_count", NULL, offsetof(AVCodecContext,rc_override_count), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, INT_MIN, INT_MAX},
##  {"maxrate", "maximum bitrate (in bits/s). Used for VBV together with bufsize.", offsetof(AVCodecContext,rc_max_rate), AV_OPT_TYPE_INT64, {.i64 = DEFAULT }, 0, INT_MAX, V|A|E},
##  {"minrate", "minimum bitrate (in bits/s). Most useful in setting up a CBR encode. It is of little use otherwise.",
##              offsetof(AVCodecContext,rc_min_rate), AV_OPT_TYPE_INT64, {.i64 = DEFAULT }, INT_MIN, INT_MAX, V|A|E},
##  {"bufsize", "set ratecontrol buffer size (in bits)", offsetof(AVCodecContext,rc_buffer_size), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, INT_MIN, INT_MAX, A|V|E},
##  {"i_qfactor", "QP factor between P- and I-frames", offsetof(AVCodecContext,i_quant_factor), AV_OPT_TYPE_FLOAT, {.dbl = -0.8 }, -FLT_MAX, FLT_MAX, V|E},
##  {"i_qoffset", "QP offset between P- and I-frames", offsetof(AVCodecContext,i_quant_offset), AV_OPT_TYPE_FLOAT, {.dbl = 0.0 }, -FLT_MAX, FLT_MAX, V|E},
##  {"dct", "DCT algorithm", offsetof(AVCodecContext,dct_algo), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, 0, INT_MAX, V|E, "dct"},
##  {"auto", "autoselect a good one", 0, AV_OPT_TYPE_CONST, {.i64 = FF_DCT_AUTO }, INT_MIN, INT_MAX, V|E, "dct"},
##  {"fastint", "fast integer", 0, AV_OPT_TYPE_CONST, {.i64 = FF_DCT_FASTINT }, INT_MIN, INT_MAX, V|E, "dct"},
##  {"int", "accurate integer", 0, AV_OPT_TYPE_CONST, {.i64 = FF_DCT_INT }, INT_MIN, INT_MAX, V|E, "dct"},
##  {"mmx", NULL, 0, AV_OPT_TYPE_CONST, {.i64 = FF_DCT_MMX }, INT_MIN, INT_MAX, V|E, "dct"},
##  {"altivec", NULL, 0, AV_OPT_TYPE_CONST, {.i64 = FF_DCT_ALTIVEC }, INT_MIN, INT_MAX, V|E, "dct"},
##  {"faan", "floating point AAN DCT", 0, AV_OPT_TYPE_CONST, {.i64 = FF_DCT_FAAN }, INT_MIN, INT_MAX, V|E, "dct"},
##  {"lumi_mask", "compresses bright areas stronger than medium ones", offsetof(AVCodecContext,lumi_masking), AV_OPT_TYPE_FLOAT, {.dbl = 0 }, -FLT_MAX, FLT_MAX, V|E},
##  {"tcplx_mask", "temporal complexity masking", offsetof(AVCodecContext,temporal_cplx_masking), AV_OPT_TYPE_FLOAT, {.dbl = 0 }, -FLT_MAX, FLT_MAX, V|E},
##  {"scplx_mask", "spatial complexity masking", offsetof(AVCodecContext,spatial_cplx_masking), AV_OPT_TYPE_FLOAT, {.dbl = 0 }, -FLT_MAX, FLT_MAX, V|E},
##  {"p_mask", "inter masking", offsetof(AVCodecContext,p_masking), AV_OPT_TYPE_FLOAT, {.dbl = 0 }, -FLT_MAX, FLT_MAX, V|E},
##  {"dark_mask", "compresses dark areas stronger than medium ones", offsetof(AVCodecContext,dark_masking), AV_OPT_TYPE_FLOAT, {.dbl = 0 }, -FLT_MAX, FLT_MAX, V|E},
##  {"idct", "select IDCT implementation", offsetof(AVCodecContext,idct_algo), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, 0, INT_MAX, V|E|D, "idct"},
##  {"auto", NULL, 0, AV_OPT_TYPE_CONST, {.i64 = FF_IDCT_AUTO }, INT_MIN, INT_MAX, V|E|D, "idct"},
##  {"int", NULL, 0, AV_OPT_TYPE_CONST, {.i64 = FF_IDCT_INT }, INT_MIN, INT_MAX, V|E|D, "idct"},
##  {"simple", NULL, 0, AV_OPT_TYPE_CONST, {.i64 = FF_IDCT_SIMPLE }, INT_MIN, INT_MAX, V|E|D, "idct"},
##  {"simplemmx", NULL, 0, AV_OPT_TYPE_CONST, {.i64 = FF_IDCT_SIMPLEMMX }, INT_MIN, INT_MAX, V|E|D, "idct"},
##  {"arm", NULL, 0, AV_OPT_TYPE_CONST, {.i64 = FF_IDCT_ARM }, INT_MIN, INT_MAX, V|E|D, "idct"},
##  {"altivec", NULL, 0, AV_OPT_TYPE_CONST, {.i64 = FF_IDCT_ALTIVEC }, INT_MIN, INT_MAX, V|E|D, "idct"},
##  {"simplearm", NULL, 0, AV_OPT_TYPE_CONST, {.i64 = FF_IDCT_SIMPLEARM }, INT_MIN, INT_MAX, V|E|D, "idct"},
##  {"simplearmv5te", NULL, 0, AV_OPT_TYPE_CONST, {.i64 = FF_IDCT_SIMPLEARMV5TE }, INT_MIN, INT_MAX, V|E|D, "idct"},
##  {"simplearmv6", NULL, 0, AV_OPT_TYPE_CONST, {.i64 = FF_IDCT_SIMPLEARMV6 }, INT_MIN, INT_MAX, V|E|D, "idct"},
##  {"simpleneon", NULL, 0, AV_OPT_TYPE_CONST, {.i64 = FF_IDCT_SIMPLENEON }, INT_MIN, INT_MAX, V|E|D, "idct"},
##  {"xvid", NULL, 0, AV_OPT_TYPE_CONST, {.i64 = FF_IDCT_XVID }, INT_MIN, INT_MAX, V|E|D, "idct"},
##  {"xvidmmx", "deprecated, for compatibility only", 0, AV_OPT_TYPE_CONST, {.i64 = FF_IDCT_XVID }, INT_MIN, INT_MAX, V|E|D, "idct"},
##  {"faani", "floating point AAN IDCT", 0, AV_OPT_TYPE_CONST, {.i64 = FF_IDCT_FAAN }, INT_MIN, INT_MAX, V|D|E, "idct"},
##  {"simpleauto", NULL, 0, AV_OPT_TYPE_CONST, {.i64 = FF_IDCT_SIMPLEAUTO }, INT_MIN, INT_MAX, V|E|D, "idct"},
##  {"slice_count", NULL, offsetof(AVCodecContext,slice_count), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, INT_MIN, INT_MAX},
##  {"ec", "set error concealment strategy", offsetof(AVCodecContext,error_concealment), AV_OPT_TYPE_FLAGS, {.i64 = 3 }, INT_MIN, INT_MAX, V|D, "ec"},
##  {"guess_mvs", "iterative motion vector (MV) search (slow)", 0, AV_OPT_TYPE_CONST, {.i64 = FF_EC_GUESS_MVS }, INT_MIN, INT_MAX, V|D, "ec"},
##  {"deblock", "use strong deblock filter for damaged MBs", 0, AV_OPT_TYPE_CONST, {.i64 = FF_EC_DEBLOCK }, INT_MIN, INT_MAX, V|D, "ec"},
##  {"favor_inter", "favor predicting from the previous frame", 0, AV_OPT_TYPE_CONST, {.i64 = FF_EC_FAVOR_INTER }, INT_MIN, INT_MAX, V|D, "ec"},
##  {"bits_per_coded_sample", NULL, offsetof(AVCodecContext,bits_per_coded_sample), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, 0, INT_MAX},
##  #if FF_API_PRIVATE_OPT
##  {"pred", "prediction method", offsetof(AVCodecContext,prediction_method), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, INT_MIN, INT_MAX, V|E, "pred"},
##  {"left", NULL, 0, AV_OPT_TYPE_CONST, {.i64 = FF_PRED_LEFT }, INT_MIN, INT_MAX, V|E, "pred"},
##  {"plane", NULL, 0, AV_OPT_TYPE_CONST, {.i64 = FF_PRED_PLANE }, INT_MIN, INT_MAX, V|E, "pred"},
##  {"median", NULL, 0, AV_OPT_TYPE_CONST, {.i64 = FF_PRED_MEDIAN }, INT_MIN, INT_MAX, V|E, "pred"},
##  #endif
##  {"aspect", "sample aspect ratio", offsetof(AVCodecContext,sample_aspect_ratio), AV_OPT_TYPE_RATIONAL, {.dbl = 0}, 0, 10, V|E},
##  {"sar",    "sample aspect ratio", offsetof(AVCodecContext,sample_aspect_ratio), AV_OPT_TYPE_RATIONAL, {.dbl = 0}, 0, 10, V|E},
##  {"debug", "print specific debug info", offsetof(AVCodecContext,debug), AV_OPT_TYPE_FLAGS, {.i64 = DEFAULT }, 0, INT_MAX, V|A|S|E|D, "debug"},
##  {"pict", "picture info", 0, AV_OPT_TYPE_CONST, {.i64 = FF_DEBUG_PICT_INFO }, INT_MIN, INT_MAX, V|D, "debug"},
##  {"rc", "rate control", 0, AV_OPT_TYPE_CONST, {.i64 = FF_DEBUG_RC }, INT_MIN, INT_MAX, V|E, "debug"},
##  {"bitstream", NULL, 0, AV_OPT_TYPE_CONST, {.i64 = FF_DEBUG_BITSTREAM }, INT_MIN, INT_MAX, V|D, "debug"},
##  {"mb_type", "macroblock (MB) type", 0, AV_OPT_TYPE_CONST, {.i64 = FF_DEBUG_MB_TYPE }, INT_MIN, INT_MAX, V|D, "debug"},
##  {"qp", "per-block quantization parameter (QP)", 0, AV_OPT_TYPE_CONST, {.i64 = FF_DEBUG_QP }, INT_MIN, INT_MAX, V|D, "debug"},
##  #if FF_API_DEBUG_MV
##  {"mv", "motion vector", 0, AV_OPT_TYPE_CONST, {.i64 = FF_DEBUG_MV }, INT_MIN, INT_MAX, V|D, "debug"},
##  #endif
##  {"dct_coeff", NULL, 0, AV_OPT_TYPE_CONST, {.i64 = FF_DEBUG_DCT_COEFF }, INT_MIN, INT_MAX, V|D, "debug"},
##  {"green_metadata", NULL, 0, AV_OPT_TYPE_CONST, {.i64 = FF_DEBUG_GREEN_MD }, INT_MIN, INT_MAX, V|D, "debug"},
##  {"skip", NULL, 0, AV_OPT_TYPE_CONST, {.i64 = FF_DEBUG_SKIP }, INT_MIN, INT_MAX, V|D, "debug"},
##  {"startcode", NULL, 0, AV_OPT_TYPE_CONST, {.i64 = FF_DEBUG_STARTCODE }, INT_MIN, INT_MAX, V|D, "debug"},
##  {"er", "error recognition", 0, AV_OPT_TYPE_CONST, {.i64 = FF_DEBUG_ER }, INT_MIN, INT_MAX, V|D, "debug"},
##  {"mmco", "memory management control operations (H.264)", 0, AV_OPT_TYPE_CONST, {.i64 = FF_DEBUG_MMCO }, INT_MIN, INT_MAX, V|D, "debug"},
##  {"bugs", NULL, 0, AV_OPT_TYPE_CONST, {.i64 = FF_DEBUG_BUGS }, INT_MIN, INT_MAX, V|D, "debug"},
##  #if FF_API_DEBUG_MV
##  {"vis_qp", "visualize quantization parameter (QP), lower QP are tinted greener", 0, AV_OPT_TYPE_CONST, {.i64 = FF_DEBUG_VIS_QP }, INT_MIN, INT_MAX, V|D, "debug"},
##  {"vis_mb_type", "visualize block types", 0, AV_OPT_TYPE_CONST, {.i64 = FF_DEBUG_VIS_MB_TYPE }, INT_MIN, INT_MAX, V|D, "debug"},
##  #endif
##  {"buffers", "picture buffer allocations", 0, AV_OPT_TYPE_CONST, {.i64 = FF_DEBUG_BUFFERS }, INT_MIN, INT_MAX, V|D, "debug"},
##  {"thread_ops", "threading operations", 0, AV_OPT_TYPE_CONST, {.i64 = FF_DEBUG_THREADS }, INT_MIN, INT_MAX, V|A|D, "debug"},
##  {"nomc", "skip motion compensation", 0, AV_OPT_TYPE_CONST, {.i64 = FF_DEBUG_NOMC }, INT_MIN, INT_MAX, V|A|D, "debug"},
##  {"dia_size", "diamond type & size for motion estimation", offsetof(AVCodecContext,dia_size), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, INT_MIN, INT_MAX, V|E},
##  {"last_pred", "amount of motion predictors from the previous frame", offsetof(AVCodecContext,last_predictor_count), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, INT_MIN, INT_MAX, V|E},
##  #if FF_API_PRIVATE_OPT
##  {"preme", "pre motion estimation", offsetof(AVCodecContext,pre_me), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, INT_MIN, INT_MAX, V|E},
##  #endif
##  {"pre_dia_size", "diamond type & size for motion estimation pre-pass", offsetof(AVCodecContext,pre_dia_size), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, INT_MIN, INT_MAX, V|E},
##  {"subq", "sub-pel motion estimation quality", offsetof(AVCodecContext,me_subpel_quality), AV_OPT_TYPE_INT, {.i64 = 8 }, INT_MIN, INT_MAX, V|E},
##  {"me_range", "limit motion vectors range (1023 for DivX player)", offsetof(AVCodecContext,me_range), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, INT_MIN, INT_MAX, V|E},
##  {"global_quality", NULL, offsetof(AVCodecContext,global_quality), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, INT_MIN, INT_MAX, V|A|E},
##  #if FF_API_CODER_TYPE
##  {"coder", NULL, offsetof(AVCodecContext,coder_type), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, INT_MIN, INT_MAX, V|E, "coder"},
##  {"vlc", "variable length coder / Huffman coder", 0, AV_OPT_TYPE_CONST, {.i64 = FF_CODER_TYPE_VLC }, INT_MIN, INT_MAX, V|E, "coder"},
##  {"ac", "arithmetic coder", 0, AV_OPT_TYPE_CONST, {.i64 = FF_CODER_TYPE_AC }, INT_MIN, INT_MAX, V|E, "coder"},
##  {"raw", "raw (no encoding)", 0, AV_OPT_TYPE_CONST, {.i64 = FF_CODER_TYPE_RAW }, INT_MIN, INT_MAX, V|E, "coder"},
##  {"rle", "run-length coder", 0, AV_OPT_TYPE_CONST, {.i64 = FF_CODER_TYPE_RLE }, INT_MIN, INT_MAX, V|E, "coder"},
##  #endif /* FF_API_CODER_TYPE */
##  #if FF_API_PRIVATE_OPT
##  {"context", "context model", offsetof(AVCodecContext,context_model), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, INT_MIN, INT_MAX, V|E},
##  #endif
##  {"slice_flags", NULL, offsetof(AVCodecContext,slice_flags), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, INT_MIN, INT_MAX},
##  {"mbd", "macroblock decision algorithm (high quality mode)", offsetof(AVCodecContext,mb_decision), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, 0, 2, V|E, "mbd"},
##  {"simple", "use mbcmp", 0, AV_OPT_TYPE_CONST, {.i64 = FF_MB_DECISION_SIMPLE }, INT_MIN, INT_MAX, V|E, "mbd"},
##  {"bits", "use fewest bits", 0, AV_OPT_TYPE_CONST, {.i64 = FF_MB_DECISION_BITS }, INT_MIN, INT_MAX, V|E, "mbd"},
##  {"rd", "use best rate distortion", 0, AV_OPT_TYPE_CONST, {.i64 = FF_MB_DECISION_RD }, INT_MIN, INT_MAX, V|E, "mbd"},
##  #if FF_API_PRIVATE_OPT
##  {"sc_threshold", "scene change threshold", offsetof(AVCodecContext,scenechange_threshold), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, INT_MIN, INT_MAX, V|E},
##  #endif
##  #if FF_API_PRIVATE_OPT
##  {"nr", "noise reduction", offsetof(AVCodecContext,noise_reduction), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, INT_MIN, INT_MAX, V|E},
##  #endif
##  {"rc_init_occupancy", "number of bits which should be loaded into the rc buffer before decoding starts", offsetof(AVCodecContext,rc_initial_buffer_occupancy), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, INT_MIN, INT_MAX, V|E},
##  {"threads", "set the number of threads", offsetof(AVCodecContext,thread_count), AV_OPT_TYPE_INT, {.i64 = 1 }, 0, INT_MAX, V|A|E|D, "threads"},
##  {"auto", "autodetect a suitable number of threads to use", 0, AV_OPT_TYPE_CONST, {.i64 = 0 }, INT_MIN, INT_MAX, V|E|D, "threads"},
##  {"dc", "intra_dc_precision", offsetof(AVCodecContext,intra_dc_precision), AV_OPT_TYPE_INT, {.i64 = 0 }, -8, 16, V|E},
##  {"nssew", "nsse weight", offsetof(AVCodecContext,nsse_weight), AV_OPT_TYPE_INT, {.i64 = 8 }, INT_MIN, INT_MAX, V|E},
##  {"skip_top", "number of macroblock rows at the top which are skipped", offsetof(AVCodecContext,skip_top), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, INT_MIN, INT_MAX, V|D},
##  {"skip_bottom", "number of macroblock rows at the bottom which are skipped", offsetof(AVCodecContext,skip_bottom), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, INT_MIN, INT_MAX, V|D},
##  {"profile", NULL, offsetof(AVCodecContext,profile), AV_OPT_TYPE_INT, {.i64 = FF_PROFILE_UNKNOWN }, INT_MIN, INT_MAX, V|A|E|CC, "avctx.profile"},
##  {"unknown", NULL, 0, AV_OPT_TYPE_CONST, {.i64 = FF_PROFILE_UNKNOWN }, INT_MIN, INT_MAX, V|A|E, "avctx.profile"},
##  {"main10",  NULL, 0, AV_OPT_TYPE_CONST, {.i64 = FF_PROFILE_HEVC_MAIN_10 }, INT_MIN, INT_MAX, V|E, "avctx.profile"},
##  {"level", NULL, offsetof(AVCodecContext,level), AV_OPT_TYPE_INT, {.i64 = FF_LEVEL_UNKNOWN }, INT_MIN, INT_MAX, V|A|E|CC, "avctx.level"},
##  {"unknown", NULL, 0, AV_OPT_TYPE_CONST, {.i64 = FF_LEVEL_UNKNOWN }, INT_MIN, INT_MAX, V|A|E, "avctx.level"},
##  {"lowres", "decode at 1= 1/2, 2=1/4, 3=1/8 resolutions", offsetof(AVCodecContext,lowres), AV_OPT_TYPE_INT, {.i64 = 0 }, 0, INT_MAX, V|A|D},
##  #if FF_API_PRIVATE_OPT
##  {"skip_threshold", "frame skip threshold", offsetof(AVCodecContext,frame_skip_threshold), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, INT_MIN, INT_MAX, V|E},
##  {"skip_factor", "frame skip factor", offsetof(AVCodecContext,frame_skip_factor), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, INT_MIN, INT_MAX, V|E},
##  {"skip_exp", "frame skip exponent", offsetof(AVCodecContext,frame_skip_exp), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, INT_MIN, INT_MAX, V|E},
##  {"skipcmp", "frame skip compare function", offsetof(AVCodecContext,frame_skip_cmp), AV_OPT_TYPE_INT, {.i64 = FF_CMP_DCTMAX }, INT_MIN, INT_MAX, V|E, "cmp_func"},
##  #endif
##  {"cmp", "full-pel ME compare function", offsetof(AVCodecContext,me_cmp), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, INT_MIN, INT_MAX, V|E, "cmp_func"},
##  {"subcmp", "sub-pel ME compare function", offsetof(AVCodecContext,me_sub_cmp), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, INT_MIN, INT_MAX, V|E, "cmp_func"},
##  {"mbcmp", "macroblock compare function", offsetof(AVCodecContext,mb_cmp), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, INT_MIN, INT_MAX, V|E, "cmp_func"},
##  {"ildctcmp", "interlaced DCT compare function", offsetof(AVCodecContext,ildct_cmp), AV_OPT_TYPE_INT, {.i64 = FF_CMP_VSAD }, INT_MIN, INT_MAX, V|E, "cmp_func"},
##  {"precmp", "pre motion estimation compare function", offsetof(AVCodecContext,me_pre_cmp), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, INT_MIN, INT_MAX, V|E, "cmp_func"},
##  {"sad", "sum of absolute differences, fast", 0, AV_OPT_TYPE_CONST, {.i64 = FF_CMP_SAD }, INT_MIN, INT_MAX, V|E, "cmp_func"},
##  {"sse", "sum of squared errors", 0, AV_OPT_TYPE_CONST, {.i64 = FF_CMP_SSE }, INT_MIN, INT_MAX, V|E, "cmp_func"},
##  {"satd", "sum of absolute Hadamard transformed differences", 0, AV_OPT_TYPE_CONST, {.i64 = FF_CMP_SATD }, INT_MIN, INT_MAX, V|E, "cmp_func"},
##  {"dct", "sum of absolute DCT transformed differences", 0, AV_OPT_TYPE_CONST, {.i64 = FF_CMP_DCT }, INT_MIN, INT_MAX, V|E, "cmp_func"},
##  {"psnr", "sum of squared quantization errors (avoid, low quality)", 0, AV_OPT_TYPE_CONST, {.i64 = FF_CMP_PSNR }, INT_MIN, INT_MAX, V|E, "cmp_func"},
##  {"bit", "number of bits needed for the block", 0, AV_OPT_TYPE_CONST, {.i64 = FF_CMP_BIT }, INT_MIN, INT_MAX, V|E, "cmp_func"},
##  {"rd", "rate distortion optimal, slow", 0, AV_OPT_TYPE_CONST, {.i64 = FF_CMP_RD }, INT_MIN, INT_MAX, V|E, "cmp_func"},
##  {"zero", "0", 0, AV_OPT_TYPE_CONST, {.i64 = FF_CMP_ZERO }, INT_MIN, INT_MAX, V|E, "cmp_func"},
##  {"vsad", "sum of absolute vertical differences", 0, AV_OPT_TYPE_CONST, {.i64 = FF_CMP_VSAD }, INT_MIN, INT_MAX, V|E, "cmp_func"},
##  {"vsse", "sum of squared vertical differences", 0, AV_OPT_TYPE_CONST, {.i64 = FF_CMP_VSSE }, INT_MIN, INT_MAX, V|E, "cmp_func"},
##  {"nsse", "noise preserving sum of squared differences", 0, AV_OPT_TYPE_CONST, {.i64 = FF_CMP_NSSE }, INT_MIN, INT_MAX, V|E, "cmp_func"},
##  #if CONFIG_SNOW_ENCODER
##  {"w53", "5/3 wavelet, only used in snow", 0, AV_OPT_TYPE_CONST, {.i64 = FF_CMP_W53 }, INT_MIN, INT_MAX, V|E, "cmp_func"},
##  {"w97", "9/7 wavelet, only used in snow", 0, AV_OPT_TYPE_CONST, {.i64 = FF_CMP_W97 }, INT_MIN, INT_MAX, V|E, "cmp_func"},
##  #endif
##  {"dctmax", NULL, 0, AV_OPT_TYPE_CONST, {.i64 = FF_CMP_DCTMAX }, INT_MIN, INT_MAX, V|E, "cmp_func"},
##  {"chroma", NULL, 0, AV_OPT_TYPE_CONST, {.i64 = FF_CMP_CHROMA }, INT_MIN, INT_MAX, V|E, "cmp_func"},
##  {"msad", "sum of absolute differences, median predicted", 0, AV_OPT_TYPE_CONST, {.i64 = FF_CMP_MEDIAN_SAD }, INT_MIN, INT_MAX, V|E, "cmp_func"},
##  {"mblmin", "minimum macroblock Lagrange factor (VBR)", offsetof(AVCodecContext,mb_lmin), AV_OPT_TYPE_INT, {.i64 = FF_QP2LAMBDA * 2 }, 1, FF_LAMBDA_MAX, V|E},
##  {"mblmax", "maximum macroblock Lagrange factor (VBR)", offsetof(AVCodecContext,mb_lmax), AV_OPT_TYPE_INT, {.i64 = FF_QP2LAMBDA * 31 }, 1, FF_LAMBDA_MAX, V|E},
##  #if FF_API_PRIVATE_OPT
##  {"mepc", "motion estimation bitrate penalty compensation (1.0 = 256)", offsetof(AVCodecContext,me_penalty_compensation), AV_OPT_TYPE_INT, {.i64 = 256 }, INT_MIN, INT_MAX, V|E},
##  #endif
##  {"skip_loop_filter", "skip loop filtering process for the selected frames", offsetof(AVCodecContext,skip_loop_filter), AV_OPT_TYPE_INT, {.i64 = AVDISCARD_DEFAULT }, INT_MIN, INT_MAX, V|D, "avdiscard"},
##  {"skip_idct"       , "skip IDCT/dequantization for the selected frames",    offsetof(AVCodecContext,skip_idct),        AV_OPT_TYPE_INT, {.i64 = AVDISCARD_DEFAULT }, INT_MIN, INT_MAX, V|D, "avdiscard"},
##  {"skip_frame"      , "skip decoding for the selected frames",               offsetof(AVCodecContext,skip_frame),       AV_OPT_TYPE_INT, {.i64 = AVDISCARD_DEFAULT }, INT_MIN, INT_MAX, V|D, "avdiscard"},
##  {"none"            , "discard no frame",                    0, AV_OPT_TYPE_CONST, {.i64 = AVDISCARD_NONE    }, INT_MIN, INT_MAX, V|D, "avdiscard"},
##  {"default"         , "discard useless frames",              0, AV_OPT_TYPE_CONST, {.i64 = AVDISCARD_DEFAULT }, INT_MIN, INT_MAX, V|D, "avdiscard"},
##  {"noref"           , "discard all non-reference frames",    0, AV_OPT_TYPE_CONST, {.i64 = AVDISCARD_NONREF  }, INT_MIN, INT_MAX, V|D, "avdiscard"},
##  {"bidir"           , "discard all bidirectional frames",    0, AV_OPT_TYPE_CONST, {.i64 = AVDISCARD_BIDIR   }, INT_MIN, INT_MAX, V|D, "avdiscard"},
##  {"nokey"           , "discard all frames except keyframes", 0, AV_OPT_TYPE_CONST, {.i64 = AVDISCARD_NONKEY  }, INT_MIN, INT_MAX, V|D, "avdiscard"},
##  {"nointra"         , "discard all frames except I frames",  0, AV_OPT_TYPE_CONST, {.i64 = AVDISCARD_NONINTRA}, INT_MIN, INT_MAX, V|D, "avdiscard"},
##  {"all"             , "discard all frames",                  0, AV_OPT_TYPE_CONST, {.i64 = AVDISCARD_ALL     }, INT_MIN, INT_MAX, V|D, "avdiscard"},
##  {"bidir_refine", "refine the two motion vectors used in bidirectional macroblocks", offsetof(AVCodecContext,bidir_refine), AV_OPT_TYPE_INT, {.i64 = 1 }, 0, 4, V|E},
##  #if FF_API_PRIVATE_OPT
##  {"brd_scale", "downscale frames for dynamic B-frame decision", offsetof(AVCodecContext,brd_scale), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, 0, 10, V|E},
##  #endif
##  {"keyint_min", "minimum interval between IDR-frames", offsetof(AVCodecContext,keyint_min), AV_OPT_TYPE_INT, {.i64 = 25 }, INT_MIN, INT_MAX, V|E},
##  {"refs", "reference frames to consider for motion compensation", offsetof(AVCodecContext,refs), AV_OPT_TYPE_INT, {.i64 = 1 }, INT_MIN, INT_MAX, V|E},
##  #if FF_API_PRIVATE_OPT
##  {"chromaoffset", "chroma QP offset from luma", offsetof(AVCodecContext,chromaoffset), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, INT_MIN, INT_MAX, V|E},
##  #endif
##  {"trellis", "rate-distortion optimal quantization", offsetof(AVCodecContext,trellis), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, INT_MIN, INT_MAX, V|A|E},
##  {"mv0_threshold", NULL, offsetof(AVCodecContext,mv0_threshold), AV_OPT_TYPE_INT, {.i64 = 256 }, 0, INT_MAX, V|E},
##  #if FF_API_PRIVATE_OPT
##  {"b_sensitivity", "adjust sensitivity of b_frame_strategy 1", offsetof(AVCodecContext,b_sensitivity), AV_OPT_TYPE_INT, {.i64 = 40 }, 1, INT_MAX, V|E},
##  #endif
##  {"compression_level", NULL, offsetof(AVCodecContext,compression_level), AV_OPT_TYPE_INT, {.i64 = FF_COMPRESSION_DEFAULT }, INT_MIN, INT_MAX, V|A|E},
##  #if FF_API_PRIVATE_OPT
##  {"min_prediction_order", NULL, offsetof(AVCodecContext,min_prediction_order), AV_OPT_TYPE_INT, {.i64 = -1 }, INT_MIN, INT_MAX, A|E},
##  {"max_prediction_order", NULL, offsetof(AVCodecContext,max_prediction_order), AV_OPT_TYPE_INT, {.i64 = -1 }, INT_MIN, INT_MAX, A|E},
##  {"timecode_frame_start", "GOP timecode frame start number, in non-drop-frame format", offsetof(AVCodecContext,timecode_frame_start), AV_OPT_TYPE_INT64, {.i64 = -1 }, -1, int64.high, V|E},
##  #endif
##  {"bits_per_raw_sample", NULL, offsetof(AVCodecContext,bits_per_raw_sample), AV_OPT_TYPE_INT, {.i64 = DEFAULT }, 0, INT_MAX},
##  {"channel_layout", NULL, offsetof(AVCodecContext,channel_layout), AV_OPT_TYPE_UINT64, {.i64 = DEFAULT }, 0, UINT64_MAX, A|E|D, "channel_layout"},
##  {"request_channel_layout", NULL, offsetof(AVCodecContext,request_channel_layout), AV_OPT_TYPE_UINT64, {.i64 = DEFAULT }, 0, UINT64_MAX, A|D, "request_channel_layout"},
##  {"rc_max_vbv_use", NULL, offsetof(AVCodecContext,rc_max_available_vbv_use), AV_OPT_TYPE_FLOAT, {.dbl = 0 }, 0.0, FLT_MAX, V|E},
##  {"rc_min_vbv_use", NULL, offsetof(AVCodecContext,rc_min_vbv_overflow_use),  AV_OPT_TYPE_FLOAT, {.dbl = 3 },     0.0, FLT_MAX, V|E},
##  {"ticks_per_frame", NULL, offsetof(AVCodecContext,ticks_per_frame), AV_OPT_TYPE_INT, {.i64 = 1 }, 1, INT_MAX, A|V|E|D},
##  {"color_primaries", "color primaries", offsetof(AVCodecContext,color_primaries), AV_OPT_TYPE_INT, {.i64 = AVCOL_PRI_UNSPECIFIED }, 1, INT_MAX, V|E|D, "color_primaries_type"},
##  {"bt709",       "BT.709",         0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_PRI_BT709 },        INT_MIN, INT_MAX, V|E|D, "color_primaries_type"},
##  {"unknown",     "Unspecified",    0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_PRI_UNSPECIFIED },  INT_MIN, INT_MAX, V|E|D, "color_primaries_type"},
##  {"bt470m",      "BT.470 M",       0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_PRI_BT470M },       INT_MIN, INT_MAX, V|E|D, "color_primaries_type"},
##  {"bt470bg",     "BT.470 BG",      0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_PRI_BT470BG },      INT_MIN, INT_MAX, V|E|D, "color_primaries_type"},
##  {"smpte170m",   "SMPTE 170 M",    0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_PRI_SMPTE170M },    INT_MIN, INT_MAX, V|E|D, "color_primaries_type"},
##  {"smpte240m",   "SMPTE 240 M",    0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_PRI_SMPTE240M },    INT_MIN, INT_MAX, V|E|D, "color_primaries_type"},
##  {"film",        "Film",           0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_PRI_FILM },         INT_MIN, INT_MAX, V|E|D, "color_primaries_type"},
##  {"bt2020",      "BT.2020",        0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_PRI_BT2020 },       INT_MIN, INT_MAX, V|E|D, "color_primaries_type"},
##  {"smpte428",    "SMPTE 428-1",    0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_PRI_SMPTE428 },     INT_MIN, INT_MAX, V|E|D, "color_primaries_type"},
##  {"smpte428_1",  "SMPTE 428-1",    0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_PRI_SMPTE428 },     INT_MIN, INT_MAX, V|E|D, "color_primaries_type"},
##  {"smpte431",    "SMPTE 431-2",    0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_PRI_SMPTE431 },     INT_MIN, INT_MAX, V|E|D, "color_primaries_type"},
##  {"smpte432",    "SMPTE 422-1",    0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_PRI_SMPTE432 },     INT_MIN, INT_MAX, V|E|D, "color_primaries_type"},
##  {"jedec-p22",   "JEDEC P22",      0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_PRI_JEDEC_P22 },    INT_MIN, INT_MAX, V|E|D, "color_primaries_type"},
##  {"ebu3213",     "EBU 3213-E",     0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_PRI_EBU3213 },      INT_MIN, INT_MAX, V|E|D, "color_primaries_type"},
##  {"unspecified", "Unspecified",    0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_PRI_UNSPECIFIED },  INT_MIN, INT_MAX, V|E|D, "color_primaries_type"},
##  {"color_trc", "color transfer characteristics", offsetof(AVCodecContext,color_trc), AV_OPT_TYPE_INT, {.i64 = AVCOL_TRC_UNSPECIFIED }, 1, INT_MAX, V|E|D, "color_trc_type"},
##  {"bt709",        "BT.709",           0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_TRC_BT709 },        INT_MIN, INT_MAX, V|E|D, "color_trc_type"},
##  {"unknown",      "Unspecified",      0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_TRC_UNSPECIFIED },  INT_MIN, INT_MAX, V|E|D, "color_trc_type"},
##  {"gamma22",      "BT.470 M",         0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_TRC_GAMMA22 },      INT_MIN, INT_MAX, V|E|D, "color_trc_type"},
##  {"gamma28",      "BT.470 BG",        0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_TRC_GAMMA28 },      INT_MIN, INT_MAX, V|E|D, "color_trc_type"},
##  {"smpte170m",    "SMPTE 170 M",      0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_TRC_SMPTE170M },    INT_MIN, INT_MAX, V|E|D, "color_trc_type"},
##  {"smpte240m",    "SMPTE 240 M",      0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_TRC_SMPTE240M },    INT_MIN, INT_MAX, V|E|D, "color_trc_type"},
##  {"linear",       "Linear",           0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_TRC_LINEAR },       INT_MIN, INT_MAX, V|E|D, "color_trc_type"},
##  {"log100",       "Log",              0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_TRC_LOG },          INT_MIN, INT_MAX, V|E|D, "color_trc_type"},
##  {"log316",       "Log square root",  0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_TRC_LOG_SQRT },     INT_MIN, INT_MAX, V|E|D, "color_trc_type"},
##  {"iec61966-2-4", "IEC 61966-2-4",    0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_TRC_IEC61966_2_4 }, INT_MIN, INT_MAX, V|E|D, "color_trc_type"},
##  {"bt1361e",      "BT.1361",          0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_TRC_BT1361_ECG },   INT_MIN, INT_MAX, V|E|D, "color_trc_type"},
##  {"iec61966-2-1", "IEC 61966-2-1",    0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_TRC_IEC61966_2_1 }, INT_MIN, INT_MAX, V|E|D, "color_trc_type"},
##  {"bt2020-10",    "BT.2020 - 10 bit", 0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_TRC_BT2020_10 },    INT_MIN, INT_MAX, V|E|D, "color_trc_type"},
##  {"bt2020-12",    "BT.2020 - 12 bit", 0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_TRC_BT2020_12 },    INT_MIN, INT_MAX, V|E|D, "color_trc_type"},
##  {"smpte2084",    "SMPTE 2084",       0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_TRC_SMPTE2084 },    INT_MIN, INT_MAX, V|E|D, "color_trc_type"},
##  {"smpte428",     "SMPTE 428-1",      0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_TRC_SMPTE428 },     INT_MIN, INT_MAX, V|E|D, "color_trc_type"},
##  {"arib-std-b67", "ARIB STD-B67",     0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_TRC_ARIB_STD_B67 }, INT_MIN, INT_MAX, V|E|D, "color_trc_type"},
##  {"unspecified",  "Unspecified",      0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_TRC_UNSPECIFIED },  INT_MIN, INT_MAX, V|E|D, "color_trc_type"},
##  {"log",          "Log",              0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_TRC_LOG },          INT_MIN, INT_MAX, V|E|D, "color_trc_type"},
##  {"log_sqrt",     "Log square root",  0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_TRC_LOG_SQRT },     INT_MIN, INT_MAX, V|E|D, "color_trc_type"},
##  {"iec61966_2_4", "IEC 61966-2-4",    0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_TRC_IEC61966_2_4 }, INT_MIN, INT_MAX, V|E|D, "color_trc_type"},
##  {"bt1361",       "BT.1361",          0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_TRC_BT1361_ECG },   INT_MIN, INT_MAX, V|E|D, "color_trc_type"},
##  {"iec61966_2_1", "IEC 61966-2-1",    0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_TRC_IEC61966_2_1 }, INT_MIN, INT_MAX, V|E|D, "color_trc_type"},
##  {"bt2020_10bit", "BT.2020 - 10 bit", 0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_TRC_BT2020_10 },    INT_MIN, INT_MAX, V|E|D, "color_trc_type"},
##  {"bt2020_12bit", "BT.2020 - 12 bit", 0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_TRC_BT2020_12 },    INT_MIN, INT_MAX, V|E|D, "color_trc_type"},
##  {"smpte428_1",   "SMPTE 428-1",      0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_TRC_SMPTE428 },     INT_MIN, INT_MAX, V|E|D, "color_trc_type"},
##  {"colorspace", "color space", offsetof(AVCodecContext,colorspace), AV_OPT_TYPE_INT, {.i64 = AVCOL_SPC_UNSPECIFIED }, 0, INT_MAX, V|E|D, "colorspace_type"},
##  {"rgb",               "RGB",                0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_SPC_RGB },                INT_MIN, INT_MAX, V|E|D, "colorspace_type"},
##  {"bt709",             "BT.709",             0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_SPC_BT709 },              INT_MIN, INT_MAX, V|E|D, "colorspace_type"},
##  {"unknown",           "Unspecified",        0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_SPC_UNSPECIFIED },        INT_MIN, INT_MAX, V|E|D, "colorspace_type"},
##  {"fcc",               "FCC",                0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_SPC_FCC },                INT_MIN, INT_MAX, V|E|D, "colorspace_type"},
##  {"bt470bg",           "BT.470 BG",          0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_SPC_BT470BG },            INT_MIN, INT_MAX, V|E|D, "colorspace_type"},
##  {"smpte170m",         "SMPTE 170 M",        0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_SPC_SMPTE170M },          INT_MIN, INT_MAX, V|E|D, "colorspace_type"},
##  {"smpte240m",         "SMPTE 240 M",        0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_SPC_SMPTE240M },          INT_MIN, INT_MAX, V|E|D, "colorspace_type"},
##  {"ycgco",             "YCGCO",              0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_SPC_YCGCO },              INT_MIN, INT_MAX, V|E|D, "colorspace_type"},
##  {"bt2020nc",          "BT.2020 NCL",        0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_SPC_BT2020_NCL },         INT_MIN, INT_MAX, V|E|D, "colorspace_type"},
##  {"bt2020c",           "BT.2020 CL",         0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_SPC_BT2020_CL },          INT_MIN, INT_MAX, V|E|D, "colorspace_type"},
##  {"smpte2085",         "SMPTE 2085",         0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_SPC_SMPTE2085 },          INT_MIN, INT_MAX, V|E|D, "colorspace_type"},
##  {"chroma-derived-nc", "Chroma-derived NCL", 0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_SPC_CHROMA_DERIVED_NCL }, INT_MIN, INT_MAX, V|E|D, "colorspace_type"},
##  {"chroma-derived-c",  "Chroma-derived CL",  0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_SPC_CHROMA_DERIVED_CL },  INT_MIN, INT_MAX, V|E|D, "colorspace_type"},
##  {"ictcp",             "ICtCp",              0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_SPC_ICTCP },              INT_MIN, INT_MAX, V|E|D, "colorspace_type"},
##  {"unspecified",       "Unspecified",        0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_SPC_UNSPECIFIED },        INT_MIN, INT_MAX, V|E|D, "colorspace_type"},
##  {"ycocg",             "YCGCO",              0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_SPC_YCGCO },              INT_MIN, INT_MAX, V|E|D, "colorspace_type"},
##  {"bt2020_ncl",        "BT.2020 NCL",        0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_SPC_BT2020_NCL },         INT_MIN, INT_MAX, V|E|D, "colorspace_type"},
##  {"bt2020_cl",         "BT.2020 CL",         0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_SPC_BT2020_CL },          INT_MIN, INT_MAX, V|E|D, "colorspace_type"},
##  {"color_range", "color range", offsetof(AVCodecContext,color_range), AV_OPT_TYPE_INT, {.i64 = AVCOL_RANGE_UNSPECIFIED }, 0, INT_MAX, V|E|D, "color_range_type"},
##  {"unknown", "Unspecified",     0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_RANGE_UNSPECIFIED }, INT_MIN, INT_MAX, V|E|D, "color_range_type"},
##  {"tv", "MPEG (219*2^(n-8))",   0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_RANGE_MPEG },        INT_MIN, INT_MAX, V|E|D, "color_range_type"},
##  {"pc", "JPEG (2^n-1)",         0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_RANGE_JPEG },        INT_MIN, INT_MAX, V|E|D, "color_range_type"},
##  {"unspecified", "Unspecified", 0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_RANGE_UNSPECIFIED }, INT_MIN, INT_MAX, V|E|D, "color_range_type"},
##  {"mpeg", "MPEG (219*2^(n-8))", 0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_RANGE_MPEG },        INT_MIN, INT_MAX, V|E|D, "color_range_type"},
##  {"jpeg", "JPEG (2^n-1)",       0, AV_OPT_TYPE_CONST, {.i64 = AVCOL_RANGE_JPEG },        INT_MIN, INT_MAX, V|E|D, "color_range_type"},
##  {"chroma_sample_location", "chroma sample location", offsetof(AVCodecContext,chroma_sample_location), AV_OPT_TYPE_INT, {.i64 = AVCHROMA_LOC_UNSPECIFIED }, 0, INT_MAX, V|E|D, "chroma_sample_location_type"},
##  {"unknown",     "Unspecified", 0, AV_OPT_TYPE_CONST, {.i64 = AVCHROMA_LOC_UNSPECIFIED }, INT_MIN, INT_MAX, V|E|D, "chroma_sample_location_type"},
##  {"left",        "Left",        0, AV_OPT_TYPE_CONST, {.i64 = AVCHROMA_LOC_LEFT },        INT_MIN, INT_MAX, V|E|D, "chroma_sample_location_type"},
##  {"center",      "Center",      0, AV_OPT_TYPE_CONST, {.i64 = AVCHROMA_LOC_CENTER },      INT_MIN, INT_MAX, V|E|D, "chroma_sample_location_type"},
##  {"topleft",     "Top-left",    0, AV_OPT_TYPE_CONST, {.i64 = AVCHROMA_LOC_TOPLEFT },     INT_MIN, INT_MAX, V|E|D, "chroma_sample_location_type"},
##  {"top",         "Top",         0, AV_OPT_TYPE_CONST, {.i64 = AVCHROMA_LOC_TOP },         INT_MIN, INT_MAX, V|E|D, "chroma_sample_location_type"},
##  {"bottomleft",  "Bottom-left", 0, AV_OPT_TYPE_CONST, {.i64 = AVCHROMA_LOC_BOTTOMLEFT },  INT_MIN, INT_MAX, V|E|D, "chroma_sample_location_type"},
##  {"bottom",      "Bottom",      0, AV_OPT_TYPE_CONST, {.i64 = AVCHROMA_LOC_BOTTOM },      INT_MIN, INT_MAX, V|E|D, "chroma_sample_location_type"},
##  {"unspecified", "Unspecified", 0, AV_OPT_TYPE_CONST, {.i64 = AVCHROMA_LOC_UNSPECIFIED }, INT_MIN, INT_MAX, V|E|D, "chroma_sample_location_type"},
##  {"log_level_offset", "set the log level offset", offsetof(AVCodecContext,log_level_offset), AV_OPT_TYPE_INT, {.i64 = 0 }, INT_MIN, INT_MAX },
##  {"slices", "set the number of slices, used in parallelized encoding", offsetof(AVCodecContext,slices), AV_OPT_TYPE_INT, {.i64 = 0 }, 0, INT_MAX, V|E},
##  {"thread_type", "select multithreading type", offsetof(AVCodecContext,thread_type), AV_OPT_TYPE_FLAGS, {.i64 = FF_THREAD_SLICE|FF_THREAD_FRAME }, 0, INT_MAX, V|A|E|D, "thread_type"},
##  {"slice", NULL, 0, AV_OPT_TYPE_CONST, {.i64 = FF_THREAD_SLICE }, INT_MIN, INT_MAX, V|E|D, "thread_type"},
##  {"frame", NULL, 0, AV_OPT_TYPE_CONST, {.i64 = FF_THREAD_FRAME }, INT_MIN, INT_MAX, V|E|D, "thread_type"},
##  {"audio_service_type", "audio service type", offsetof(AVCodecContext,audio_service_type), AV_OPT_TYPE_INT, {.i64 = AV_AUDIO_SERVICE_TYPE_MAIN }, 0, AV_AUDIO_SERVICE_TYPE_NB-1, A|E, "audio_service_type"},
##  {"ma", "Main Audio Service", 0, AV_OPT_TYPE_CONST, {.i64 = AV_AUDIO_SERVICE_TYPE_MAIN },              INT_MIN, INT_MAX, A|E, "audio_service_type"},
##  {"ef", "Effects",            0, AV_OPT_TYPE_CONST, {.i64 = AV_AUDIO_SERVICE_TYPE_EFFECTS },           INT_MIN, INT_MAX, A|E, "audio_service_type"},
##  {"vi", "Visually Impaired",  0, AV_OPT_TYPE_CONST, {.i64 = AV_AUDIO_SERVICE_TYPE_VISUALLY_IMPAIRED }, INT_MIN, INT_MAX, A|E, "audio_service_type"},
##  {"hi", "Hearing Impaired",   0, AV_OPT_TYPE_CONST, {.i64 = AV_AUDIO_SERVICE_TYPE_HEARING_IMPAIRED },  INT_MIN, INT_MAX, A|E, "audio_service_type"},
##  {"di", "Dialogue",           0, AV_OPT_TYPE_CONST, {.i64 = AV_AUDIO_SERVICE_TYPE_DIALOGUE },          INT_MIN, INT_MAX, A|E, "audio_service_type"},
##  {"co", "Commentary",         0, AV_OPT_TYPE_CONST, {.i64 = AV_AUDIO_SERVICE_TYPE_COMMENTARY },        INT_MIN, INT_MAX, A|E, "audio_service_type"},
##  {"em", "Emergency",          0, AV_OPT_TYPE_CONST, {.i64 = AV_AUDIO_SERVICE_TYPE_EMERGENCY },         INT_MIN, INT_MAX, A|E, "audio_service_type"},
##  {"vo", "Voice Over",         0, AV_OPT_TYPE_CONST, {.i64 = AV_AUDIO_SERVICE_TYPE_VOICE_OVER },        INT_MIN, INT_MAX, A|E, "audio_service_type"},
##  {"ka", "Karaoke",            0, AV_OPT_TYPE_CONST, {.i64 = AV_AUDIO_SERVICE_TYPE_KARAOKE },           INT_MIN, INT_MAX, A|E, "audio_service_type"},
##  {"request_sample_fmt", "sample format audio decoders should prefer", offsetof(AVCodecContext,request_sample_fmt), AV_OPT_TYPE_SAMPLE_FMT, {.i64=AV_SAMPLE_FMT_NONE}, -1, INT_MAX, A|D, "request_sample_fmt"},
##  {"pkt_timebase", NULL, offsetof(AVCodecContext,pkt_timebase), AV_OPT_TYPE_RATIONAL, {.dbl = 0 }, 0, INT_MAX, 0},
##  {"sub_charenc", "set input text subtitles character encoding", offsetof(AVCodecContext,sub_charenc), AV_OPT_TYPE_STRING, {.str = NULL}, 0, 0, S|D},
##  {"sub_charenc_mode", "set input text subtitles character encoding mode", offsetof(AVCodecContext,sub_charenc_mode), AV_OPT_TYPE_FLAGS, {.i64 = FF_SUB_CHARENC_MODE_AUTOMATIC}, -1, INT_MAX, S|D, "sub_charenc_mode"},
##  {"do_nothing",  NULL, 0, AV_OPT_TYPE_CONST, {.i64 = FF_SUB_CHARENC_MODE_DO_NOTHING},  INT_MIN, INT_MAX, S|D, "sub_charenc_mode"},
##  {"auto",        NULL, 0, AV_OPT_TYPE_CONST, {.i64 = FF_SUB_CHARENC_MODE_AUTOMATIC},   INT_MIN, INT_MAX, S|D, "sub_charenc_mode"},
##  {"pre_decoder", NULL, 0, AV_OPT_TYPE_CONST, {.i64 = FF_SUB_CHARENC_MODE_PRE_DECODER}, INT_MIN, INT_MAX, S|D, "sub_charenc_mode"},
##  {"ignore",      NULL, 0, AV_OPT_TYPE_CONST, {.i64 = FF_SUB_CHARENC_MODE_IGNORE},      INT_MIN, INT_MAX, S|D, "sub_charenc_mode"},
##  #if FF_API_ASS_TIMING
##  {"sub_text_format", "set decoded text subtitle format", offsetof(AVCodecContext,sub_text_format), AV_OPT_TYPE_INT, {.i64 = FF_SUB_TEXT_FMT_ASS_WITH_TIMINGS}, 0, 1, S|D, "sub_text_format"},
##  #else
##  {"sub_text_format", "set decoded text subtitle format", offsetof(AVCodecContext,sub_text_format), AV_OPT_TYPE_INT, {.i64 = FF_SUB_TEXT_FMT_ASS}, 0, 1, S|D, "sub_text_format"},
##  #endif
##  {"ass",              NULL, 0, AV_OPT_TYPE_CONST, {.i64 = FF_SUB_TEXT_FMT_ASS},              INT_MIN, INT_MAX, S|D, "sub_text_format"},
##  #if FF_API_ASS_TIMING
##  {"ass_with_timings", NULL, 0, AV_OPT_TYPE_CONST, {.i64 = FF_SUB_TEXT_FMT_ASS_WITH_TIMINGS}, INT_MIN, INT_MAX, S|D, "sub_text_format"},
##  #endif
##  {"refcounted_frames", NULL, offsetof(AVCodecContext,refcounted_frames), AV_OPT_TYPE_BOOL, {.i64 = 0}, 0, 1, A|V|D },
##  #if FF_API_SIDEDATA_ONLY_PKT
##  {"side_data_only_packets", NULL, offsetof(AVCodecContext,side_data_only_packets), AV_OPT_TYPE_BOOL, { .i64 = 1 }, 0, 1, A|V|E },
##  #endif
##  {"apply_cropping", NULL, offsetof(AVCodecContext,apply_cropping), AV_OPT_TYPE_BOOL, { .i64 = 1 }, 0, 1, V | D },
##  {"skip_alpha", "Skip processing alpha", offsetof(AVCodecContext,skip_alpha), AV_OPT_TYPE_BOOL, {.i64 = 0 }, 0, 1, V|D },
##  {"field_order", "Field order", offsetof(AVCodecContext,field_order), AV_OPT_TYPE_INT, {.i64 = AV_FIELD_UNKNOWN }, 0, 5, V|D|E, "field_order" },
##  {"progressive", NULL, 0, AV_OPT_TYPE_CONST, {.i64 = AV_FIELD_PROGRESSIVE }, 0, 0, V|D|E, "field_order" },
##  {"tt", NULL, 0, AV_OPT_TYPE_CONST, {.i64 = AV_FIELD_TT }, 0, 0, V|D|E, "field_order" },
##  {"bb", NULL, 0, AV_OPT_TYPE_CONST, {.i64 = AV_FIELD_BB }, 0, 0, V|D|E, "field_order" },
##  {"tb", NULL, 0, AV_OPT_TYPE_CONST, {.i64 = AV_FIELD_TB }, 0, 0, V|D|E, "field_order" },
##  {"bt", NULL, 0, AV_OPT_TYPE_CONST, {.i64 = AV_FIELD_BT }, 0, 0, V|D|E, "field_order" },
##  {"dump_separator", "set information dump field separator", offsetof(AVCodecContext,dump_separator), AV_OPT_TYPE_STRING, {.str = NULL}, 0, 0, A|V|S|D|E},
##  {"codec_whitelist", "List of decoders that are allowed to be used", offsetof(AVCodecContext,codec_whitelist), AV_OPT_TYPE_STRING, { .str = NULL },  0, 0, A|V|S|D },
##  {"pixel_format", "set pixel format", offsetof(AVCodecContext,pix_fmt), AV_OPT_TYPE_PIXEL_FMT, {.i64=AV_PIX_FMT_NONE}, -1, INT_MAX, 0 },
##  {"video_size", "set video size", offsetof(AVCodecContext,width), AV_OPT_TYPE_IMAGE_SIZE, {.str=NULL}, 0, INT_MAX, 0 },
##  {"max_pixels", "Maximum number of pixels", offsetof(AVCodecContext,max_pixels), AV_OPT_TYPE_INT64, {.i64 = INT_MAX }, 0, INT_MAX, A|V|S|D|E },
##  {"max_samples", "Maximum number of samples", offsetof(AVCodecContext,max_samples), AV_OPT_TYPE_INT64, {.i64 = INT_MAX }, 0, INT_MAX, A|D|E },
##  {"hwaccel_flags", NULL, offsetof(AVCodecContext,hwaccel_flags), AV_OPT_TYPE_FLAGS, {.i64 = AV_HWACCEL_FLAG_IGNORE_LEVEL }, 0, UINT_MAX, V|D, "hwaccel_flags"},
##  {"ignore_level", "ignore level even if the codec level used is unknown or higher than the maximum supported level reported by the hardware driver", 0, AV_OPT_TYPE_CONST, { .i64 = AV_HWACCEL_FLAG_IGNORE_LEVEL }, INT_MIN, INT_MAX, V | D, "hwaccel_flags" },
##  {"allow_high_depth", "allow to output YUV pixel formats with a different chroma sampling than 4:2:0 and/or other than 8 bits per component", 0, AV_OPT_TYPE_CONST, {.i64 = AV_HWACCEL_FLAG_ALLOW_HIGH_DEPTH }, INT_MIN, INT_MAX, V | D, "hwaccel_flags"},
##  {"allow_profile_mismatch", "attempt to decode anyway if HW accelerated decoder's supported profiles do not exactly match the stream", 0, AV_OPT_TYPE_CONST, {.i64 = AV_HWACCEL_FLAG_ALLOW_PROFILE_MISMATCH }, INT_MIN, INT_MAX, V | D, "hwaccel_flags"},
##  {"extra_hw_frames", "Number of extra hardware frames to allocate for the user", offsetof(AVCodecContext,extra_hw_frames), AV_OPT_TYPE_INT, { .i64 = -1 }, -1, INT_MAX, V|D },
##  {"discard_damaged_percentage", "Percentage of damaged samples to discard a frame", offsetof(AVCodecContext,discard_damaged_percentage), AV_OPT_TYPE_INT, {.i64 = 95 }, 0, 100, V|D },
##  {NULL},
##  };
var av_codec_context_class = AVClass(class_name= "AVCodecContext", 
    item_name= context_to_name,
    option                  = avcodec_options,
    version                 = LIBAVUTIL_VERSION_INT,
    log_level_offset_offset = offsetof(AVCodecContext, log_level_offset),
    child_next              = codec_child_next,
    child_class_next        = codec_child_class_next,
    child_class_iterate     = codec_child_class_iterate,
    category                = AV_CLASS_CATEGORY_ENCODER,
    get_category            = AVClass_get_category,
)


proc init_context_defaults*(s: AVCodecContext; codec: AVCodec): cint =
  var flags: cint = 0
  s.av_class = av_codec_context_class
  s.codec_type = if codec: codec.t else: AVMEDIA_TYPE_UNKNOWN
  if codec:
    s.codec = codec
    s.codec_id = codec.id
  if s.codec_type == AVMEDIA_TYPE_AUDIO:
    flags = AV_OPT_FLAG_AUDIO_PARAM
  elif s.codec_type == AVMEDIA_TYPE_VIDEO:
    flags = AV_OPT_FLAG_VIDEO_PARAM
  elif s.codec_type == AVMEDIA_TYPE_SUBTITLE:
    flags = AV_OPT_FLAG_SUBTITLE_PARAM
  av_opt_set_defaults2(s, flags, flags)
  s.time_base           = AVRational(0, 1)
  s.framerate           = AVRational(0, 1)
  s.pkt_timebase        = AVRational(0, 1)
  s.get_buffer2 = avcodec_default_get_buffer2
  s.get_format = avcodec_default_get_format
  s.execute = avcodec_default_execute
  s.execute2 = avcodec_default_execute2
  s.sample_aspect_ratio = AVRational(0,1)
  s.pix_fmt = AV_PIX_FMT_NONE
  s.sw_pix_fmt = AV_PIX_FMT_NONE
  s.sample_fmt = AV_SAMPLE_FMT_NONE
  s.reordered_opaque = AV_NOPTS_VALUE
  if codec and codec.priv_data_size:
    if not s.priv_data:
      s.priv_data = av_mallocz(codec.priv_data_size)
      if not s.priv_data:
        return (ENOMEM)
    if codec.priv_class:
      cast[AVClass](s.priv_data)[] = codec.priv_class
      av_opt_set_defaults(s.priv_data)
  if codec and codec.defaults:
    var ret: cint
    var d: ptr AVCodecDefault = codec.defaults
    while d.key:
      ret = av_opt_set(s, d.key, d.value, 0)
      av_assert0(ret >= 0)
      inc(d)
  return 0

proc avcodec_alloc_context3*(codec: AVCodec): AVCodecContext =
  var avctx: AVCodecContext
  if init_context_defaults(avctx, codec) < 0:
    return nil
  return avctx


proc avformat_new_stream*(s: AVFormatContext; c: AVCodec): AVStream =
  var st: AVStream
  var i: cint
  if s.nb_streams.int32 >= min(s.max_streams, INT_MAX div sizeof(AVStream)):
    if s.max_streams < INT_MAX div sizeof(AVStream):
      echo("Number of streams exceeds max_streams parameter (%d), see the documentation if you wish to increase it\n", s.max_streams)
    return nil
  s.streams.setLen s.nb_streams + 1
  st.info = AVStreamInnerStruct(last_dts:AV_NOPTS_VALUE)
  st.internal = AVStreamInternal(avctx:avcodec_alloc_context3(nil))
  st.codecpar = avcodec_parameters_alloc()
  if s.iformat:
    avpriv_set_pts_info(st, 33, 1, 90000)
    st.cur_dts = RELATIVE_TS_BASE
  else:
    st.cur_dts = AV_NOPTS_VALUE
  st.index = s.nb_streams
  st.start_time = AV_NOPTS_VALUE
  st.duration = AV_NOPTS_VALUE
  st.first_dts = AV_NOPTS_VALUE
  st.probe_packets = s.max_probe_packets
  st.pts_wrap_reference = AV_NOPTS_VALUE
  st.pts_wrap_behavior = AV_PTS_WRAP_IGNORE
  st.last_IP_pts = AV_NOPTS_VALUE
  st.last_dts_for_order_check = AV_NOPTS_VALUE
  i = 0
  while i < MAX_REORDER_DELAY + 1:
    st.pts_buffer[i] = AV_NOPTS_VALUE
    inc(i)
  st.sample_aspect_ratio = AVRational(0, 1)
  st.info.fps_first_dts = AV_NOPTS_VALUE
  st.info.fps_last_dts = AV_NOPTS_VALUE
  st.inject_global_side_data = s.internal.inject_global_side_data
  st.internal.need_context_update = 1
  s.streams[inc(s.nb_streams)] = st
  return st

proc align_get_bits*(s: GetBitContext): uint8 {.inline.} =
  var n: cint = - s.index and 7
  skip_bits(s, n)
  return s.buffer + (s.index shr 3)

proc h264_probe*(p: AVProbeData): cint =
  var code: int32 = -1
  var
    sps: cint = 0
    pps: cint = 0
    idr: cint = 0
    res: cint = 0
    sli: cint = 0
  var
    i: cint
    ret: cint
  var pps_ids: array[256 + 1, cint]
  var sps_ids: array[32 + 1, cint]
  var
    pps_id: cuint
    sps_id: cuint
  var gb: GetBitContext
  i = 0
  while i + 2 < p.buf_size:
    code = (code shl 8) + p.buf[i]
    if (code and 0xFFFFFF00) == 0x00000100:
      var ref_idc: cint = (code shr 5) and 3
      var t: cint = code and 0x0000001F
      var ref_zero: UncheckedArray[int8] = [2, 0, 0, 0, 0, -1, 1, -1, -1, 1, 1, 1, 1, -1, 2, 2, 2,2, 2, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2]
      if code and 0x00000080:
        return 0
      if ref_zero[t] == 1 and ref_idc:
        return 0
      if ref_zero[t] == -1 and not ref_idc:
        return 0
      if ref_zero[t] == 2:
        if not (code == 0x00000100 and not p.buf[i + 1] and not p.buf[i + 2]):
          inc(res)
      var bufSize = p.buf_size - i - 1
      ret = init_get_bits8(gb, p.buf + i + 1, bufSize)
      if ret < 0:
        return 0
      case t
      of 1, 5:
        get_ue_golomb_long(gb)
        if get_ue_golomb_long(gb) > 9:
          return 0
        pps_id = get_ue_golomb_long(gb)
        if pps_id > MAX_PPS_COUNT:
          return 0
        if not pps_ids[pps_id]:
          break
        if t == 1:
          inc(sli)
        else:
          inc(idr)
      of 7:
        skip_bits(gb, 14)
        if get_bits(gb, 2):
          return 0
        skip_bits(gb, 8)
        sps_id = get_ue_golomb_long(gb)
        if sps_id > MAX_SPS_COUNT:
          return 0
        sps_ids[sps_id] = 1
        inc(sps)
      of 8:
        pps_id = get_ue_golomb_long(gb)
        if pps_id > MAX_PPS_COUNT:
          return 0
        sps_id = get_ue_golomb_long(gb)
        if sps_id > MAX_SPS_COUNT:
          return 0
        if not sps_ids[sps_id]:
          break
        pps_ids[pps_id] = 1
        inc(pps)
    inc(i)
  if sps and pps and (idr or sli > 3) and res < (sps + pps + idr):
    return AVPROBE_SCORE_EXTENSION + 1
  return 0

proc ff_raw_read_partial_packet*(s: AVFormatContext; pkt: AVPacket): cint =
  var raw: FFRawDemuxerContext = s.priv_data
  var
    ret: cint
    size: cint
  size = raw.raw_packet_size
  if (ret = av_new_packet(pkt, size)) < 0:
    return ret
  pkt.pos = avio_tell(s.pb)
  pkt.stream_index = 0
  ret = avio_read_partial(s.pb, pkt.data, size)
  if ret < 0:
    av_packet_unref(pkt)
    return ret
  av_shrink_packet(pkt, ret)
  return ret

proc ff_raw_audio_read_header*(s: AVFormatContext): cint =
  var st: AVStream = avformat_new_stream(s, nil)
  if not st:
    return (ENOMEM)
  st.codecpar.codec_type = AVMEDIA_TYPE_AUDIO
  st.codecpar.codec_id = s.iformat.raw_codec_id
  st.need_parsing = AVSTREAM_PARSE_FULL_RAW
  st.start_time = 0
  return 0

proc ff_raw_video_read_header*(s: AVFormatContext): cint =
  var st: AVStream
  var s1: FFRawVideoDemuxerContext = s.priv_data
  var ret: cint = 0
  st = avformat_new_stream(s, nil)
  st.codecpar.codec_type = AVMEDIA_TYPE_VIDEO
  st.codecpar.codec_id = s.iformat.raw_codec_id
  st.need_parsing = AVSTREAM_PARSE_FULL_RAW
  st.internal.avctx.framerate = s1.framerate
  avpriv_set_pts_info(st, 64, 1, 1200000)
  return ret

proc ff_raw_subtitle_read_header*(s: AVFormatContext): cint =
  var st: AVStream = avformat_new_stream(s, nil)
  st.codecpar.codec_type = AVMEDIA_TYPE_SUBTITLE
  st.codecpar.codec_id = s.iformat.raw_codec_id
  st.start_time = 0
  return 0

proc ff_raw_data_read_header*(s: AVFormatContext): cint =
  var st: AVStream = avformat_new_stream(s, nil)
  st.codecpar.codec_type = AVMEDIA_TYPE_DATA
  st.codecpar.codec_id = s.iformat.raw_codec_id
  st.start_time = 0
  return 0

var h264_dumuxer_class: AVClass = (class_name: "h264_demuxer",option:ff_rawvideo_options,version:LIBAVUTIL_VERSION_INT)
var ff_h264_demuxer = AVInputFormat(name:"h264",
    long_name: "raw H.264 video", 
    read_probe: h264_probe, 
    read_header:ff_raw_video_read_header, 
    read_packet:ff_raw_read_partial_packet, 
    extensions: "h26l,h264,264,avc", 
    flags: AVFMT_GENERIC_INDEX, 
    raw_codec_id:id, 
    priv_data_size:sizeof(FFRawVideoDemuxerContext),
    priv_class: "h264_demuxer_class")

var demuxer_list*: UncheckedArray[AVInputFormat] = [
    # ff_aa_demuxer, 
    # ff_aac_demuxer,
    # ff_aax_demuxer, 
    # ff_ac3_demuxer, 
    # ff_acm_demuxer, 
    # ff_act_demuxer, 
    # ff_adf_demuxer,
    # ff_adp_demuxer, 
    # ff_ads_demuxer, 
    # ff_adx_demuxer, 
    # ff_aea_demuxer, 
    # ff_afc_demuxer,
    # ff_aiff_demuxer, 
    # ff_aix_demuxer, 
    # ff_alp_demuxer, 
    # ff_amr_demuxer,
    # ff_amrnb_demuxer, 
    # ff_amrwb_demuxer, 
    # ff_anm_demuxer, 
    # ff_apc_demuxer,
    # ff_ape_demuxer, 
    # ff_apm_demuxer, 
    # ff_apng_demuxer, 
    # ff_aptx_demuxer,
    # ff_aptx_hd_demuxer, 
    # ff_aqtitle_demuxer, 
    # ff_argo_asf_demuxer,
    # ff_argo_brp_demuxer, 
    # ff_asf_demuxer, 
    # ff_asf_o_demuxer, 
    # ff_ass_demuxer,
    # ff_ast_demuxer, 
    # ff_au_demuxer, 
    # ff_av1_demuxer, 
    # ff_avi_demuxer, 
    # ff_avr_demuxer,
    # ff_avs_demuxer, 
    # ff_avs2_demuxer, 
    # ff_bethsoftvid_demuxer, 
    # ff_bfi_demuxer,
    # ff_bintext_demuxer, 
    # ff_bink_demuxer, 
    # ff_bit_demuxer, 
    # ff_bmv_demuxer,
    # ff_bfstm_demuxer, 
    # ff_brstm_demuxer, 
    # ff_boa_demuxer, 
    # ff_c93_demuxer,
    # ff_caf_demuxer, 
    # ff_cavsvideo_demuxer, 
    # ff_cdg_demuxer, 
    # ff_cdxl_demuxer,
    # ff_cine_demuxer, 
    # ff_codec2_demuxer, 
    # ff_codec2raw_demuxer, 
    # ff_concat_demuxer,
    # ff_dash_demuxer, 
    # ff_data_demuxer, 
    # ff_daud_demuxer, 
    # ff_dcstr_demuxer,
    # ff_derf_demuxer, 
    # ff_dfa_demuxer, 
    # ff_dhav_demuxer, 
    # ff_dirac_demuxer,
    # ff_dnxhd_demuxer, 
    # ff_dsf_demuxer, 
    # ff_dsicin_demuxer, 
    # ff_dss_demuxer,
    # ff_dts_demuxer, 
    # ff_dtshd_demuxer, 
    # ff_dv_demuxer, 
    # ff_dvbsub_demuxer,
    # ff_dvbtxt_demuxer, 
    # ff_dxa_demuxer, 
    # ff_ea_demuxer, 
    # ff_ea_cdata_demuxer,
    # ff_eac3_demuxer, 
    # ff_epaf_demuxer, 
    # ff_ffmetadata_demuxer, 
    # ff_filmstrip_demuxer,
    # ff_fits_demuxer, 
    # ff_flac_demuxer, 
    # ff_flic_demuxer, 
    # ff_flv_demuxer,
    # ff_live_flv_demuxer, 
    # ff_fourxm_demuxer, 
    # ff_frm_demuxer, 
    # ff_fsb_demuxer,
    # ff_fwse_demuxer, 
    # ff_g722_demuxer, 
    # ff_g723_1_demuxer, 
    # ff_g726_demuxer,
    # ff_g726le_demuxer, 
    # ff_g729_demuxer, 
    # ff_gdv_demuxer, 
    # ff_genh_demuxer,
    # ff_gif_demuxer, 
    # ff_gsm_demuxer, 
    # ff_gxf_demuxer, 
    # ff_h261_demuxer,
    # ff_h263_demuxer, 
    ff_h264_demuxer, 
    ff_hevc_demuxer, 
    # ff_hca_demuxer, 
    # ff_hcom_demuxer,
    # ff_hls_demuxer, 
    # ff_hnm_demuxer, 
    # ff_ico_demuxer,
    # ff_idcin_demuxer, 
    # ff_idf_demuxer, 
    # ff_iff_demuxer, 
    # ff_ifv_demuxer,
    # ff_ilbc_demuxer, 
    # ff_image2_demuxer, 
    # ff_image2pipe_demuxer,
    # ff_image2_alias_pix_demuxer, 
    # ff_image2_brender_pix_demuxer,
    # ff_ingenient_demuxer, 
    # ff_ipmovie_demuxer, 
    # ff_ipu_demuxer, 
    # ff_ircam_demuxer,
    # ff_iss_demuxer, 
    # ff_iv8_demuxer, 
    # ff_ivf_demuxer, 
    # ff_ivr_demuxer,
    # ff_jacosub_demuxer, 
    # ff_jv_demuxer, 
    # ff_kux_demuxer, 
    # ff_kvag_demuxer,
    # ff_lmlm4_demuxer, 
    # ff_loas_demuxer,
    # ff_luodat_demuxer, 
    # ff_lrc_demuxer,
    # ff_lvf_demuxer, 
    # ff_lxf_demuxer, 
    # ff_m4v_demuxer, 
    # ff_mca_demuxer, 
    # ff_mcc_demuxer,
    # ff_matroska_demuxer, 
    # ff_mgsts_demuxer, 
    # ff_microdvd_demuxer, 
    # ff_mjpeg_demuxer,
    # ff_mjpeg_2000_demuxer, 
    # ff_mlp_demuxer, 
    # ff_mlv_demuxer, 
    # ff_mm_demuxer,
    # ff_mmf_demuxer, 
    # ff_mods_demuxer, 
    # ff_moflex_demuxer, 
    # ff_mov_demuxer,
    # ff_mp3_demuxer, 
    # ff_mpc_demuxer, 
    # ff_mpc8_demuxer, 
    # ff_mpegps_demuxer,
    # ff_mpegts_demuxer, 
    # ff_mpegtsraw_demuxer, 
    # ff_mpegvideo_demuxer,
    # ff_mpjpeg_demuxer, 
    # ff_mpl2_demuxer, 
    # ff_mpsub_demuxer, 
    # ff_msf_demuxer,
    # ff_msnwc_tcp_demuxer, 
    # ff_mtaf_demuxer, 
    # ff_mtv_demuxer, 
    # ff_musx_demuxer,
    # ff_mv_demuxer, 
    # ff_mvi_demuxer, 
    # ff_mxf_demuxer, 
    # ff_mxg_demuxer, 
    # ff_nc_demuxer,
    # ff_nistsphere_demuxer, 
    # ff_nsp_demuxer, 
    # ff_nsv_demuxer, 
    # ff_nut_demuxer,
    # ff_nuv_demuxer, 
    # ff_obu_demuxer, 
    # ff_ogg_demuxer, 
    # ff_oma_demuxer, 
    # ff_paf_demuxer,
    # ff_pcm_alaw_demuxer, 
    # ff_pcm_mulaw_demuxer, 
    # ff_pcm_vidc_demuxer,
    # ff_pcm_f64be_demuxer, 
    # ff_pcm_f64le_demuxer, 
    # ff_pcm_f32be_demuxer,
    # ff_pcm_f32le_demuxer, 
    # ff_pcm_s32be_demuxer, 
    # ff_pcm_s32le_demuxer,
    # ff_pcm_s24be_demuxer, 
    # ff_pcm_s24le_demuxer, 
    # ff_pcm_s16be_demuxer,
    # ff_pcm_s16le_demuxer, 
    # ff_pcm_s8_demuxer, 
    # ff_pcm_u32be_demuxer,
    # ff_pcm_u32le_demuxer, 
    # ff_pcm_u24be_demuxer, 
    # ff_pcm_u24le_demuxer,
    # ff_pcm_u16be_demuxer, 
    # ff_pcm_u16le_demuxer, 
    # ff_pcm_u8_demuxer, 
    # ff_pjs_demuxer,
    # ff_pmp_demuxer, 
    # ff_pp_bnk_demuxer, 
    # ff_pva_demuxer, 
    # ff_pvf_demuxer,
    # ff_qcp_demuxer, 
    # ff_r3d_demuxer, 
    # ff_rawvideo_demuxer, 
    # ff_realtext_demuxer,
    # ff_redspark_demuxer, 
    # ff_rl2_demuxer, 
    # ff_rm_demuxer, 
    # ff_roq_demuxer,
    # ff_rpl_demuxer, 
    # ff_rsd_demuxer, 
    # ff_rso_demuxer, 
    # ff_rtp_demuxer, 
    # ff_rtsp_demuxer,
    # ff_s337m_demuxer, 
    # ff_sami_demuxer, 
    # ff_sap_demuxer, 
    # ff_sbc_demuxer,
    # ff_sbg_demuxer, 
    # ff_scc_demuxer, 
    # ff_sdp_demuxer, 
    # ff_sdr2_demuxer, 
    # ff_sds_demuxer,
    # ff_sdx_demuxer, 
    # ff_segafilm_demuxer, 
    # ff_ser_demuxer, 
    # ff_shorten_demuxer,
    # ff_siff_demuxer, 
    # ff_sln_demuxer, 
    # ff_smacker_demuxer, 
    # ff_smjpeg_demuxer,
    # ff_smush_demuxer, 
    # ff_sol_demuxer, 
    # ff_sox_demuxer, 
    # ff_spdif_demuxer,
    # ff_srt_demuxer, 
    # ff_str_demuxer, 
    # ff_stl_demuxer, 
    # ff_subviewer1_demuxer,
    # ff_subviewer_demuxer, 
    # ff_sup_demuxer, 
    # ff_svag_demuxer, 
    # ff_svs_demuxer,
    # ff_swf_demuxer, 
    # ff_tak_demuxer, 
    # ff_tedcaptions_demuxer, 
    # ff_thp_demuxer,
    # ff_threedostr_demuxer, 
    # ff_tiertexseq_demuxer, 
    # ff_tmv_demuxer,
    # ff_truehd_demuxer, 
    # ff_tta_demuxer, 
    # ff_txd_demuxer, 
    # ff_tty_demuxer,
    # ff_ty_demuxer, 
    # ff_v210_demuxer, 
    # ff_v210x_demuxer, 
    # ff_vag_demuxer,
    # ff_vc1_demuxer, 
    # ff_vc1t_demuxer, 
    # ff_vividas_demuxer, 
    # ff_vivo_demuxer,
    # ff_vmd_demuxer, 
    # ff_vobsub_demuxer, 
    # ff_voc_demuxer, 
    # ff_vpk_demuxer,
    # ff_vplayer_demuxer, 
    # ff_vqf_demuxer, 
    # ff_w64_demuxer, 
    # ff_wav_demuxer,
    # ff_wc3_demuxer, 
    # ff_webm_dash_manifest_demuxer, 
    # ff_webvtt_demuxer,
    # ff_wsaud_demuxer, 
    # ff_wsd_demuxer, 
    # ff_wsvqa_demuxer, 
    # ff_wtv_demuxer,
    # ff_wve_demuxer, 
    # ff_wv_demuxer, 
    # ff_xa_demuxer, 
    # ff_xbin_demuxer, 
    # ff_xmv_demuxer,
    # ff_xvag_demuxer, 
    # ff_xwma_demuxer, 
    # ff_yop_demuxer, 
    # ff_yuv4mpegpipe_demuxer,
    # ff_image_bmp_pipe_demuxer, 
    # ff_image_dds_pipe_demuxer,
    # ff_image_dpx_pipe_demuxer, 
    # ff_image_exr_pipe_demuxer,
    # ff_image_gif_pipe_demuxer, 
    # ff_image_j2k_pipe_demuxer,
    # ff_image_jpeg_pipe_demuxer, 
    # ff_image_jpegls_pipe_demuxer,
    # ff_image_pam_pipe_demuxer, 
    # ff_image_pbm_pipe_demuxer,
    # ff_image_pcx_pipe_demuxer, 
    # ff_image_pgmyuv_pipe_demuxer,
    # ff_image_pgm_pipe_demuxer, 
    # ff_image_pgx_pipe_demuxer,
    # ff_image_photocd_pipe_demuxer, 
    # ff_image_pictor_pipe_demuxer,
    # ff_image_png_pipe_demuxer, 
    # ff_image_ppm_pipe_demuxer,
    # ff_image_psd_pipe_demuxer, 
    # ff_image_qdraw_pipe_demuxer,
    # ff_image_sgi_pipe_demuxer, 
    # ff_image_svg_pipe_demuxer,
    # ff_image_sunrast_pipe_demuxer, 
    # ff_image_tiff_pipe_demuxer,
    # ff_image_webp_pipe_demuxer, 
    # ff_image_xpm_pipe_demuxer,
    # ff_image_xwd_pipe_demuxer, 
    # ff_libgme_demuxer, 
    # ff_libmodplug_demuxer, 
    nil]


proc av_format_init_next*() =
  var prevout: AVOutputFormat = nil
  var previn: AVInputFormat = nil
  for o in muxer_list:
    if prevout != nil:
      prevout.next = o
    prevout = o
  for o in outdev_list:
    if prevout != nil:
      prevout.next = o
    prevout = o
  for i in demuxer_list:
    if previn != nil: 
      previn.next = i
    previn = i
  for i in indev_list:
    if previn != nil:
      previn.next = i
    previn = i

proc avpriv_register_devices*(o: seq[AVOutputFormat]; i: seq[AVInputFormat]) =
  outdev_list = o
  indev_list = i
  av_format_init_next()

proc init_opts*() =
  sws_dict["flags"] = "bicubic"


var url_protocols: seq[URLProtocol]


proc url_find_protocol*(filename: string): seq[URLProtocol] =
  var protocols: URLProtocol
  var
    proto_str = newString(128)
    proto_nested = newStringOfCap(128)
  var proto_len = len filename.filterIt(it in URL_SCHEME_CHARS)
  if filename[proto_len] != ':' and filename == "subfile," or ':' notin filename or filename[1] == ':':
    copyMem(proto_str[0].addr, "file".string, 4)
  else:
    copyMem(proto_str[0].addr, filename[0].unsafeAddr, min(proto_len + 1, proto_str.len))
  copyMem(proto_nested[0].addr, proto_str[0].addr, sizeof((proto_nested)))
  proto_nested[proto_nested.find '+'] = '\0'
  result = url_protocols.filterIt(it.name in [proto_str,proto_nested])
  if startsWith(filename, "https:") or startsWith(filename, "tls"):
    echo ("https protocol not found, recompile FFmpeg with openssl, gnutls or securetransport enabled.\n")
  
var ffurl_context_class*: AVClass

proc av_opt_set_defaults2*(s: pointer; mask: cint; flags: cint) =
  var opt: AVOption
  while (opt = av_opt_next(s, opt)):
    var dst: pointer = (cast[uint8](s)) + opt.offset
    if (opt.flags and mask) != flags:
      continue
    if opt.flags and AV_OPT_FLAG_READONLY:
      continue
    case opt.t
    of AV_OPT_TYPE_CONST:      
      nil
    of AV_OPT_TYPE_BOOL, AV_OPT_TYPE_FLAGS, AV_OPT_TYPE_INT, AV_OPT_TYPE_INT64,
      AV_OPT_TYPE_UINT64, AV_OPT_TYPE_DURATION, AV_OPT_TYPE_CHANNEL_LAYOUT,
      AV_OPT_TYPE_PIXEL_FMT, AV_OPT_TYPE_SAMPLE_FMT:
      write_number(s, opt, dst, 1, 1, opt.default_val.i64)
    of AV_OPT_TYPE_DOUBLE, AV_OPT_TYPE_FLOAT:
      var val: cdouble
      val = opt.default_val.dbl
      write_number(s, opt, dst, val, 1, 1)
    of AV_OPT_TYPE_RATIONAL:
      var val: AVRational
      val = av_d2q(opt.default_val.dbl, INT_MAX)
      write_number(s, opt, dst, 1, val.den, val.num)
    of AV_OPT_TYPE_COLOR:
      set_string_color(s, opt, opt.default_val.str, dst)
    of AV_OPT_TYPE_STRING:
      set_string(s, opt, opt.default_val.str, dst)
    of AV_OPT_TYPE_IMAGE_SIZE:
      set_string_image_size(s, opt, opt.default_val.str, dst)
    of AV_OPT_TYPE_VIDEO_RATE:
      set_string_video_rate(s, opt, opt.default_val.str, dst)
    of AV_OPT_TYPE_BINARY:
      set_string_binary(s, opt, opt.default_val.str, dst)
    of AV_OPT_TYPE_DICT:
      set_string_dict(s, opt, opt.default_val.str, dst)
    else:
      echo(s, AV_LOG_DEBUG, "AVOption type %d of option %s not implemented yet\n", opt.t,opt.name)

proc url_alloc_for_protocol*(puc: URLContext; up: URLProtocol;filename: string; flags: cint;int_cb: AVIOInterruptCB): cint =
  var uc: URLContext
  var err: cint

  uc = av_mallocz(sizeof((URLContext)) + strlen(filename) + 1)
  uc.av_class = ffurl_context_class
  uc.filename == filename
  uc.prot = up
  uc.flags = flags
  uc.is_streamed = 0
  uc.max_packet_size = 0
  var proto_len: cint = strlen(up.name)
  var start: string = find(uc.filename, ',')
  uc.priv_data = up.priv_data_class
  av_opt_set_defaults2(uc.priv_data,0,0)
  if not up.name == uc.filename and uc.filename + proto_len == start:
    var ret: cint = 0
    var p: string = start
    var sep: char = inc(p)[]
    var
      key: string
      val: string
    inc(p)
    if up.name == "subfile":
      ret = EINVAL
    while ret >= 0 and (key = find(p, sep)) and p < key and (val = find(key + 1, sep)):
      val[] = key[] = 0
      if p == "start" and p == "end":
        ret = AVERROR_OPTION_NOT_FOUND
      else:
        ret = av_opt_set(uc.priv_data, p, key + 1, 0)
      if ret == AVERROR_OPTION_NOT_FOUND:
        echo ("Key \'%s\' not found.\n", p)
      val[] = key[] = sep
      p = val + 1
    if ret < 0 or p != key:
      echo("Error parsing options string",start )
      err = EINVAL
      goto fail
    memmove(start, key + 1, strlen(key))
  uc.interrupt_callback = int_cb[]
  puc = uc
  return err

proc ffurl_alloc*(puc: URLContext; filename: string; flags: cint; int_cb: AVIOInterruptCB): cint =
  var p: URLProtocol 
  p = url_find_protocol(filename)
  return url_alloc_for_protocol(puc, p, filename, flags, int_cb)

proc ffurl_connect*(uc: URLContext; options: TableRef[string,string]): cint =
  var err: cint
  var tmp_opts: TableRef[string,string]
  var e: AVDictionaryEntry
  if options.len == 0:
    options = tmp_opts
  if  uc.prot.name notin uc.protocol_whitelist :
    echo ("Protocol \'%s\' not on whitelist \'%s\'!\n",uc.prot.name, uc.protocol_whitelist)
    return EINVAL
  if uc.prot.name in uc.protocol_blacklist:
    echo ("Protocol \'%s\' on blacklist \'%s\'!\n",uc.prot.name, uc.protocol_blacklist)
    return EINVAL
  if not uc.protocol_whitelist and uc.prot.default_whitelist:
    echo (uc, AV_LOG_DEBUG, "Setting default whitelist \'%s\'\n", uc.prot.default_whitelist)
    uc.protocol_whitelist = uc.prot.default_whitelist
  elif not uc.protocol_whitelist: 
    echo (uc, AV_LOG_DEBUG, "No default whitelist set\n")
  options["protocol_whitelist"] = uc.protocol_whitelist
  options["protocol_blacklist"] = uc.protocol_blacklist
  err = if uc.prot.url_open2: uc.prot.url_open2(uc, uc.filename, uc.flags, options) 
        else: uc.prot.url_open(uc, uc.filename, uc.flags)
  options["protocol_whitelist"].clear
  options["protocol_blacklist"].clear
  uc.is_connected = 1
  if (uc.flags and AVIO_FLAG_WRITE) or uc.prot.name, "file"):
    if not uc.is_streamed and ffurl_seek(uc, 0, SEEK_SET) < 0:
      uc.is_streamed = 1
  return 0

proc ffurl_open_whitelist*(puc: URLContext; filename: string; flags: cint;int_cb: AVIOInterruptCB;options: TableRef[string,string]; whitelist: string;blacklist: string; parent: URLContext): cint =
  var tmp_opts: TableRef[string,string] 
  var e: AVDictionaryEntry
  var ret: cint = ffurl_alloc(puc, filename, flags, int_cb)
  if ret < 0:
    return ret
  if parent:
    av_opt_copy(puc[], parent)
  if options and (ret = av_opt_set_dict(puc[], options)) < 0:
    goto fail
  if options and (puc[]).prot.priv_data_class and (ret = av_opt_set_dict((puc[]).priv_data, options)) < 0:
    goto fail
  if not options:
    options = tmp_opts
  options["protocol_whitelist"] = whitelist 
  options["protocol_blacklist"] = blacklist 
  if (ret = av_opt_set_dict(puc[], options)) < 0:
  ret = ffurl_connect(puc[], options)
  if not ret:
    return 0
  ffurl_closep(puc)
  return ret

proc ffurl_size*(h: ptr URLContext): int64 =
  var
    pos: int64
    size: int64
  size = ffurl_seek(h, 0, AVSEEK_SIZE)
  if size < 0:
    pos = ffurl_seek(h, 0, SEEK_CUR)
    if (size = ffurl_seek(h, -1, SEEK_END)) < 0:
      return size
    inc(size)
    ffurl_seek(h, pos, SEEK_SET)
  return size

proc ring_destroy*(ring: ptr RingBuffer) =
  av_fifo_freep(ring.fifo)

proc async_open*(h: ptr URLContext; arg: string; flags: cint;
                options: ptr ptr AVDictionary): cint =
  var c: ptr Context = h.priv_data
  var ret: cint
  
  av_strstart(arg, "async:", arg)
  ret = ring_init(c.ring, BUFFER_CAPACITY, READ_BACK_CAPACITY)
  if ret < 0:
    break fifo_fail
  c.interrupt_callback = h.interrupt_callback
  ret = ffurl_open_whitelist(c.inner, arg, flags, interrupt_callback,
                           options, h.protocol_whitelist, h.protocol_blacklist, h)
  if ret != 0:
    echo(h,  "ffurl_open failed : %s, %s\n", av_err2str(ret), arg)
    break url_fail
  c.logical_size = ffurl_size(c.inner)
  h.is_streamed = c.inner.is_streamed
  ret = pthread_mutex_init(c.mutex, nil)
  if ret != 0:
    echo(h,  "pthread_mutex_init failed : %s\n", av_err2str(ret))
    break mutex_fail
  ret = pthread_cond_init(c.cond_wakeup_main, nil)
  if ret != 0:
    echo(h,  "pthread_cond_init failed : %s\n", av_err2str(ret))
    break cond_wakeup_main_fail
  ret = pthread_cond_init(c.cond_wakeup_background, nil)
  if ret != 0:
    echo(h,  "pthread_cond_init failed : %s\n", av_err2str(ret))
    break cond_wakeup_background_fail
  ret = pthread_create(c.async_buffer_thread, nil, async_buffer_task, h)
  if ret:
    echo(h,  "pthread_create failed : %s\n", av_err2str(ret))
    break thread_fail
  return 0
  pthread_cond_destroy(c.cond_wakeup_background)
  pthread_cond_destroy(c.cond_wakeup_main)
  pthread_mutex_destroy(c.mutex)
  ffurl_closep(c.inner)
  ring_destroy(c.ring)
  return ret

proc async_close*(h: ptr URLContext): cint =
  var c: ptr Context = h.priv_data
  var ret: cint
  pthread_mutex_lock(c.mutex)
  c.abort_request = 1
  pthread_cond_signal(c.cond_wakeup_background)
  pthread_mutex_unlock(c.mutex)
  ret = pthread_join(c.async_buffer_thread, nil)
  if ret != 0:
    echo(h,  "pthread_join(): %s\n", av_err2str(ret))
  pthread_cond_destroy(c.cond_wakeup_background)
  pthread_cond_destroy(c.cond_wakeup_main)
  pthread_mutex_destroy(c.mutex)
  ffurl_closep(c.inner)
  ring_destroy(c.ring)
  return 0

proc ring_size*(ring: ptr RingBuffer): cint =
  return av_fifo_size(ring.fifo) - ring.read_pos

proc av_fifo_generic_peek_at*(f: ptr AVFifoBuffer; dest: pointer; offset: cint;
                             buf_size: cint;
                             `func`: proc (a1: pointer; a2: pointer; a3: cint)): cint =
  var rptr: uint8 = f.rptr
  av_assert2(offset >= 0)
  av_assert2(buf_size + cast[cuint](offset) <= f.wndx - f.rndx)
  if offset >= f.e - rptr:
    inc(rptr, offset - (f.e - f.buffer))
  else:
    inc(rptr, offset)
  while buf_size > 0:
    var len: cint
    if rptr >= f.e:
      dec(rptr, f.e - f.buffer)
    len = min(f.e - rptr, buf_size)
    if `func`:
      `func`(dest, rptr, len)
    else:
      copyMem(dest, rptr, len)
      dest = cast[uint8](dest) + len
    dec(buf_size, len)
    inc(rptr, len)
  return 0

proc ring_generic_read*(ring: ptr RingBuffer; dest: pointer; buf_size: cint;
                       `func`: proc (a1: pointer; a2: pointer; a3: cint)): cint =
  var ret: cint
  av_assert2(buf_size <= ring_size(ring))
  ret = av_fifo_generic_peek_at(ring.fifo, dest, ring.read_pos, buf_size, `func`)
  inc(ring.read_pos, buf_size)
  if ring.read_pos > ring.read_back_capacity:
    av_fifo_drain(ring.fifo, ring.read_pos - ring.read_back_capacity)
    ring.read_pos = ring.read_back_capacity
  return ret

proc async_read_internal*(h: ptr URLContext; dest: pointer; size: cint;read_complete: cint;`func`: proc (a1: pointer; a2: pointer; a3: cint)): cint =
  var c: ptr Context = h.priv_data
  var ring: ptr RingBuffer = c.ring
  var to_read: cint = size
  var ret: cint = 0
  pthread_mutex_lock(c.mutex)
  while to_read > 0:
    var
      fifo_size: cint
      to_copy: cint
    if async_check_interrupt(h):
      ret = AVERROR_EXIT
      break
    fifo_size = ring_size(ring)
    to_copy = min(to_read, fifo_size)
    if to_copy > 0:
      ring_generic_read(ring, dest, to_copy, `func`)
      if not `func`:
        dest = cast[uint8](dest) + to_copy
      inc(c.logical_pos, to_copy)
      dec(to_read, to_copy)
      ret = size - to_read
      if to_read <= 0 or not read_complete:
        break
    elif c.io_eof_reached:
      if ret <= 0:
        if c.io_error:
          ret = c.io_error
        else:
          ret = AVERROR_EOF
      break
    pthread_cond_signal(c.cond_wakeup_background)
    pthread_cond_wait(c.cond_wakeup_main, c.mutex)
  pthread_cond_signal(c.cond_wakeup_background)
  pthread_mutex_unlock(c.mutex)
  return ret

proc async_read*(h: ptr URLContext; buf: ptr cuchar; size: cint): cint =
  return async_read_internal(h, buf, size, 0, nil)

proc fifo_do_not_copy_func*(dest: pointer; src: pointer; size: cint) =
  discard

proc ring_size_of_read_back*(ring: ptr RingBuffer): cint =
  return ring.read_pos

proc ring_drain*(ring: ptr RingBuffer; offset: cint): cint =
  av_assert2(offset >= -ring_size_of_read_back(ring))
  av_assert2(offset <= ring_size(ring))
  inc(ring.read_pos, offset)
  return 0

proc async_seek*(h: ptr URLContext; pos: int64; whence: cint): int64 =
  var c: ptr Context = h.priv_data
  var ring: ptr RingBuffer = c.ring
  var ret: int64
  var new_logical_pos: int64
  var fifo_size: cint
  var fifo_size_of_read_back: cint
  if whence == AVSEEK_SIZE:
    echo(h, AV_LOG_TRACE, "async_seek: AVSEEK_SIZE: %", PRId64, "\n",
           cast[int64](c.logical_size))
    return c.logical_size
  elif whence == SEEK_CUR:
    echo(h, AV_LOG_TRACE, "async_seek: %", PRId64, "\n", pos)
    new_logical_pos = pos + c.logical_pos
  elif whence == SEEK_SET:
    echo(h, AV_LOG_TRACE, "async_seek: %", PRId64, "\n", pos)
    new_logical_pos = pos
  else:
    return EINVAL
  if new_logical_pos < 0:
    return EINVAL
  fifo_size = ring_size(ring)
  fifo_size_of_read_back = ring_size_of_read_back(ring)
  if new_logical_pos == c.logical_pos:
    return c.logical_pos
  elif (new_logical_pos >= (c.logical_pos - fifo_size_of_read_back)) and
      (new_logical_pos < (c.logical_pos + fifo_size + SHORT_SEEK_THRESHOLD)):
    var pos_delta: cint = (int)(new_logical_pos - c.logical_pos)
    echo("async_seek: fask_seek %", PRId64," from %d dist:%d/%d\n", new_logical_pos, cast[cint](c.logical_pos),(int)(new_logical_pos - c.logical_pos), fifo_size)
    if pos_delta > 0:
      async_read_internal(h, nil, pos_delta, 1, fifo_do_not_copy_func)
    else:
      ring_drain(ring, pos_delta)
      c.logical_pos = new_logical_pos
    return c.logical_pos
  elif c.logical_size <= 0:
    return EINVAL
  elif new_logical_pos > c.logical_size:
    return EINVAL
  pthread_mutex_lock(c.mutex)
  c.seek_request = 1
  c.seek_pos = new_logical_pos
  c.seek_whence = SEEK_SET
  c.seek_completed = 0
  c.seek_ret = 0
  while 1:
    if async_check_interrupt(h):
      ret = AVERROR_EXIT
      break
    if c.seek_completed:
      if c.seek_ret >= 0:
        c.logical_pos = c.seek_ret
      ret = c.seek_ret
      break
    pthread_cond_signal(c.cond_wakeup_background)
    pthread_cond_wait(c.cond_wakeup_main, c.mutex)
  pthread_mutex_unlock(c.mutex)
  return ret

var  ff_async_protocol = URLProtocol(name:"async",url_open2:async_open,url_read:async_read,
  url_seek:async_seek,url_close:async_close,priv_data_size:sizeof(Context),priv_data_class:async_context_class)


proc io_open_default*(s: AVFormatContext; pb: AVIOContext; url: string;flags: cint; options: TableRef[string,string]): cint =
  var loglevel: cint
  if url == s.url or s.iformat.name == "image2" or  s.oformat.name == "image2":
    loglevel = AV_LOG_DEBUG
  else:
    loglevel = AV_LOG_INFO
  return ffio_open_whitelist(pb, url, flags, s.interrupt_callback, options, s.protocol_whitelist, s.protocol_blacklist)

proc io_close_default*(s: AVFormatContext; pb: AVIOContext) =
  avio_close(pb)

proc av_demuxer_iterate*(opaque: ptr pointer): AVInputFormat =
  var size: uintptr_t = sizeof((demuxer_list) div sizeof((demuxer_list[0]))) - 1
  var i: uintptr_t = (uintptr_t) * opaque
  var f: AVInputFormat
  if i < size:
    f = demuxer_list[i]
  elif outdev_list:
    f = indev_list[i - size]
  if f:
    opaque[] = cast[pointer]((i + 1))
  return f

proc av_muxer_iterate*(opaque: ptr pointer): AVOutputFormat =
  var size: uintptr_t = sizeof((muxer_list) div sizeof((muxer_list[0]))) - 1
  var i: uintptr_t = (uintptr_t) * opaque
  var f: AVOutputFormat
  if i < size:
    f = muxer_list[i]
  elif indev_list:
    f = outdev_list[i - size]
  if f:
    opaque[] = cast[pointer]((i + 1))
  return f

proc format_child_class_iterate*(iter: ptr pointer): AVClass =
  var val: pointer = cast[pointer]((((uintptr_t) * iter) and ((1 shl ITER_STATE_SHIFT) - 1)))
  var state: cuint = ((uintptr_t) * iter) shr ITER_STATE_SHIFT
  var ret: AVClass
  if state == CHILD_CLASS_ITER_AVIO:
    ret = ff_avio_class
    inc(state)
    goto finish
  if state == CHILD_CLASS_ITER_MUX:
    var ofmt: AVOutputFormat
    while (ofmt = av_muxer_iterate(val)):
      ret = ofmt.priv_class
      if ret:
        goto finish
    val 
    inc(state)
  if state == CHILD_CLASS_ITER_DEMUX:
    var ifmt: AVInputFormat
    while (ifmt = av_demuxer_iterate(val)):
      ret = ifmt.priv_class
      if ret:
        goto finish
    val 
    inc(state)
  av_assert0(not (cast[uintptr_t](val) shr ITER_STATE_SHIFT))
  iter[] = cast[pointer]((cast[uintptr_t](val) or (state shl ITER_STATE_SHIFT)))
  return ret

proc format_child_next*(obj: pointer; prev: pointer): pointer =
  var s: AVFormatContext = obj
  if not prev and s.priv_data and
      ((s.iformat and s.iformat.priv_class) or s.oformat and s.oformat.priv_class):
    return s.priv_data
  if s.pb and s.pb.av_class and prev != s.pb:
    return s.pb
  return nil

proc AVClass_get_category*(p: pointer): AVClassCategory =
  var avctx: AVCodecContext = p
  if avctx.codec and avctx.codec.decode:
    return AV_CLASS_CATEGORY_DECODER
  else:
    return AV_CLASS_CATEGORY_ENCODER

proc format_to_name*(p: pointer): string =
  var fc: AVFormatContext = cast[AVFormatContext](p)
  if fc.iformat:
    return fc.iformat.name
  elif fc.oformat:
    return fc.oformat.name
  else:
    return "NULL"

proc avformat_alloc_context*(): AVFormatContext =
  var ic: AVFormatContext
  var internal: AVFormatInternal
  ic = av_malloc(sizeof((AVFormatContext)))
  var av_format_context_class = AVClass(class_name:"AVFormatContext", item_name:format_to_name, option: avformat_options, version:LIBAVUTIL_VERSION_INT, child_next:format_child_next, child_class_iterate:format_child_class_iterate,category:AV_CLASS_CATEGORY_MUXER,get_category:AVClass_get_category)
  s.av_class = av_format_context_class
  s.io_open = io_open_default
  s.io_close = io_close_default
  av_opt_set_defaults2(s,0,0)
  ic.internal = internal
  ic.internal.offset = AV_NOPTS_VALUE
  ic.internal.raw_packet_buffer_remaining_size = RAW_PACKET_BUFFER_SIZE
  ic.internal.shortest_end = AV_NOPTS_VALUE
  return ic


proc read_thread*(arg: VideoState): cint =
  var videoState: VideoState = arg
  var ic: AVFormatContext
  var
    err: cint
    i: cint
    ret: cint
  var st_index: array[5, cint]
  var
    pkt1: AVPacket
    pkt: AVPacket = pkt1
  var stream_start_time: int64
  var pkt_in_play_range: cint = 0
  var t: string
  var scan_all_pmts_set: cint = 0
  var pkt_ts: int64
  videoState.eof = 0
  ic = avformat_alloc_context()
  if not ic:
    echo(nil, AV_LOG_FATAL, "Could not allocate context.\n")
    ret = ENOMEM
    goto fail
  ic.interrupt_callback.callback = decode_interrupt_cb
  ic.interrupt_callback.opaque = videoState
  if not format_opts.has "scan_all_pmts":
    format_opts["scan_all_pmts"] = "1"
    scan_all_pmts_set = 1
  err = avformat_open_input(ic, videoState.filename, videoState.iformat, format_opts)
  if err < 0:
    print_error(videoState.filename, err)
    ret = -1
    goto fail
  if scan_all_pmts_set:
    format_opts["scan_all_pmts"] = ""
  videoState.ic = ic
  if genpts:
    ic.flags = ic.flags or AVFMT_FLAG_GENPTS
  av_format_inject_global_side_data(ic)
  if find_stream_info:
    var opts: Table[string,string] = setup_find_stream_info_opts(ic, codec_opts)
    var orig_nb_streams: cint = ic.nb_streams
    err = avformat_find_stream_info(ic, opts)
    i = 0
    while i < orig_nb_streams:
      inc(i)
    if err < 0:
      echo(fmt"{videoState.filename}: could not find codec parameters\n")
      ret = -1
      goto fail
  if ic.pb:
    ic.pb.eof_reached = 0
  if seek_by_bytes < 0:
    seek_by_bytes = not not (ic.iformat.flags and AVFMT_TS_DISCONT) and "ogg" == ic.iformat.name
  videoState.max_frame_duration = if (ic.iformat.flags and AVFMT_TS_DISCONT): 10.0 else: 3600.0
  if not window_title and (t = ic.metadata["title"]):
    window_title = fmt"{t} - {input_filename}"
  if start_time != AV_NOPTS_VALUE:
    var timestamp: int64
    timestamp = start_time
    if ic.start_time != AV_NOPTS_VALUE:
      inc(timestamp, ic.start_time)
    ret = avformat_seek_file(ic, -1, int64.low, timestamp, int64.high, 0)
    if ret < 0:
      echo(nil, AV_LOG_WARNING, "%s: could not seek to position %0.3f\n",videoState.filename, cast[cdouble](timestamp div AV_TIME_BASE))
  videoState.realtime = is_realtime(ic)
  if show_status:
    av_dump_format(ic, 0, videoState.filename, 0)
  i = 0
  while i < ic.nb_streams:
    var st: AVStream = ic.streams[i]
    var t: AVMediaType = st.codecpar.codec_type
    st.`discard` = AVDISCARD_ALL
    if t >= 0 and wanted_stream_spec[t] and st_index[t] == -1:
      if avformat_match_stream_specifier(ic, st, wanted_stream_spec[t]) > 0:
        st_index[t] = i
    inc(i)
  i = 0
  while i < AVMEDIA_TYPE_NB:
    if wanted_stream_spec[i] and st_index[i] == -1:
      echo(nil, 
             "Stream specifier %s does not match any %s stream\n",
             wanted_stream_spec[i], av_get_media_type_string(i))
      st_index[i] = INT_MAX
    inc(i)
  if not video_disable:
    st_index[AVMEDIA_TYPE_VIDEO] = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO,
        st_index[AVMEDIA_TYPE_VIDEO], -1, nil, 0)
  if not audio_disable:
    st_index[AVMEDIA_TYPE_AUDIO] = av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO,
        st_index[AVMEDIA_TYPE_AUDIO], st_index[AVMEDIA_TYPE_VIDEO], nil, 0)
  if not video_disable and not subtitle_disable:
    st_index[AVMEDIA_TYPE_SUBTITLE] = av_find_best_stream(ic,
        AVMEDIA_TYPE_SUBTITLE, st_index[AVMEDIA_TYPE_SUBTITLE], (if st_index[
        AVMEDIA_TYPE_AUDIO] >= 0: st_index[AVMEDIA_TYPE_AUDIO] else: st_index[
        AVMEDIA_TYPE_VIDEO]), nil, 0)
  videoState.show_mode = show_mode
  if st_index[AVMEDIA_TYPE_VIDEO] >= 0:
    var st: AVStream = ic.streams[st_index[AVMEDIA_TYPE_VIDEO]]
    var codecpar: AVCodecParameters = st.codecpar
    var sar: AVRational = av_guess_sample_aspect_ratio(ic, st, nil)
    if codecpar.width:
      set_default_window_size(codecpar.width, codecpar.height, sar)
  if st_index[AVMEDIA_TYPE_AUDIO] >= 0:
    stream_component_open(videoState, st_index[AVMEDIA_TYPE_AUDIO])
  ret = -1
  if st_index[AVMEDIA_TYPE_VIDEO] >= 0:
    ret = stream_component_open(videoState, st_index[AVMEDIA_TYPE_VIDEO])
  if videoState.show_mode == SHOW_MODE_NONE:
    videoState.show_mode = if ret >= 0: SHOW_MODE_VIDEO else: SHOW_MODE_RDFT
  if st_index[AVMEDIA_TYPE_SUBTITLE] >= 0:
    stream_component_open(videoState, st_index[AVMEDIA_TYPE_SUBTITLE])
  if videoState.video_stream < 0 and videoState.audio_stream < 0:
    echo(nil, AV_LOG_FATAL,
           "Failed to open file \'%s\' or configure filtergraph\n", videoState.filename)
    ret = -1
    goto fail
  if infinite_buffer < 0 and videoState.realtime:
    infinite_buffer = 1
  while true:
    if videoState.abort_request:
      break
    if videoState.paused != videoState.last_paused:
      videoState.last_paused = videoState.paused
      if videoState.paused:
        videoState.read_pause_return = av_read_pause(ic)
      else:
        av_read_play(ic)
    if videoState.paused and ic.iformat.name == "rtsp" or (ic.pb and not strncmp(input_filename, "mmsh:", 5))):
      
      poll 10
      # SDL_Delay(10)
      continue
    if videoState.seek_req:
      var seek_target: int64 = videoState.seek_pos
      var seek_min: int64 = if videoState.seek_rel > 0: seek_target - videoState.seek_rel + 2 else: int64.low
      var seek_max: int64 = if videoState.seek_rel < 0: seek_target - videoState.seek_rel - 2 else: int64.high
      
      ret = avformat_seek_file(videoState.ic, -1, seek_min, seek_target, seek_max,videoState.seek_flags)
      if ret < 0:
        echo(fmt"{videoState.ic.url}: error while seeking\n", )
      else:
        if videoState.audio_stream >= 0:
          packet_queue_flush(videoState.audioq)
          packet_queue_put(videoState.audioq, flush_pkt)
        if videoState.subtitle_stream >= 0:
          packet_queue_flush(videoState.subtitleq)
          packet_queue_put(videoState.subtitleq, flush_pkt)
        if videoState.video_stream >= 0:
          packet_queue_flush(videoState.videoq)
          packet_queue_put(videoState.videoq, flush_pkt)
        if videoState.seek_flags and AVSEEK_FLAG_BYTE:
          set_clock(videoState.extclk, NAN, 0)
        else:
          set_clock(videoState.extclk, seek_target div cast[cdouble](AV_TIME_BASE), 0)
      videoState.seek_req = 0
      videoState.queue_attachments_req = 1
      videoState.eof = 0
      if videoState.paused:
        step_to_next_frame(videoState)
    if videoState.queue_attachments_req:
      if videoState.video_st and
          (videoState.video_st.disposition and AV_DISPOSITION_ATTACHED_PIC):
        var copy: AVPacket
        if (ret = av_packet_ref(copy, videoState.video_st.attached_pic)) < 0:
          goto fail
        packet_queue_put(videoState.videoq, copy)
        packet_queue_put_nullpacket(videoState.videoq, videoState.video_stream)
      videoState.queue_attachments_req = 0
    if infinite_buffer < 1 and
        (videoState.audioq.size + videoState.videoq.size + videoState.subtitleq.size > MAX_QUEUE_SIZE or
        (stream_has_enough_packets(videoState.audio_st, videoState.audio_stream, videoState.audioq) and
        stream_has_enough_packets(videoState.video_st, videoState.video_stream, videoState.videoq) and
        stream_has_enough_packets(videoState.subtitle_st, videoState.subtitle_stream, videoState.subtitleq))):
      
      poll 10
      # SDL_CondWaitTimeout(videoState.continue_read_thread, wait_mutex, 10)
      continue
    if not videoState.paused and (not videoState.audio_st or (videoState.auddec.finished == videoState.audioq.serial and frame_queue_nb_remaining(videoState.sampq) == 0)) and (not videoState.video_st or (videoState.viddec.finished == videoState.videoq.serial and frame_queue_nb_remaining(videoState.pictq) == 0)):
      if loop != 1 and (not loop or dec(loop)):
        stream_seek(videoState, if start_time != AV_NOPTS_VALUE: start_time else: 0, 0, 0)
      elif autoexit:
        ret = AVERROR_EOF
        goto fail
    ret = av_read_frame(ic, pkt)
    if ret < 0:
      if (ret == AVERROR_EOF or avio_feof(ic.pb)) and not videoState.eof:
        if videoState.video_stream >= 0:
          packet_queue_put_nullpacket(videoState.videoq, videoState.video_stream)
        if videoState.audio_stream >= 0:
          packet_queue_put_nullpacket(videoState.audioq, videoState.audio_stream)
        if videoState.subtitle_stream >= 0:
          packet_queue_put_nullpacket(videoState.subtitleq, videoState.subtitle_stream)
        videoState.eof = 1
      if ic.pb and ic.pb.error:
        if autoexit:
          goto fail
        else:
          break
      poll 10
      continue
    else:
      videoState.eof = 0
    stream_start_time = ic.streams[pkt.stream_index].start_time
    pkt_ts = if pkt.pts == AV_NOPTS_VALUE: pkt.dts else: pkt.pts
    pkt_in_play_range = duration == AV_NOPTS_VALUE or (pkt_ts - (if stream_start_time != AV_NOPTS_VALUE: stream_start_time else: 0)) * av_q2d(ic.streams[pkt.stream_index].time_base) - (double)(if start_time != AV_NOPTS_VALUE: start_time else: 0) div 1000000 <= (cast[cdouble](duration div 1000000))
    if pkt.stream_index == videoState.audio_stream and pkt_in_play_range:
      packet_queue_put(videoState.audioq, pkt)
    elif pkt.stream_index == videoState.video_stream and pkt_in_play_range and
        not (videoState.video_st.disposition and AV_DISPOSITION_ATTACHED_PIC):
      packet_queue_put(videoState.videoq, pkt)
    elif pkt.stream_index == videoState.subtitle_stream and pkt_in_play_range:
      packet_queue_put(videoState.subtitleq, pkt)
    else:
      av_packet_unref(pkt)
  ret = 0
  if ic and not videoState.ic:
    avformat_close_input(ic)
  if ret != 0:
    var event: SDL_Event
    event.t = FF_QUIT_EVENT
    event.user.data1 = videoState
    SDL_PushEvent(event)
  return 0

proc frame_queue_init*(f: ptr FrameQueue; pktq: ptr PacketQueue; max_size: cint;
                      keep_last: cint): cint =
  var i: cint
  memset(f, 0, sizeof((FrameQueue)))
  if not (f.mutex = SDL_CreateMutex()):
    echo(nil, AV_LOG_FATAL, "SDL_CreateMutex(): %s\n", SDL_GetError())
    return ENOMEM
  if not (f.cond = SDL_CreateCond()):
    echo(nil, AV_LOG_FATAL, "SDL_CreateCond(): %s\n", SDL_GetError())
    return ENOMEM
  f.pktq = pktq
  f.max_size = min(max_size, FRAME_QUEUE_SIZE)
  f.keep_last = not not keep_last
  i = 0
  while i < f.max_size:
    if not (f.queue[i].frame = av_frame_alloc()):
      return ENOMEM
    inc(i)
  return 0


proc stream_open*(filename: string; iformat: AVInputFormat): VideoState =
  var videoState: VideoState
  videoState = VideoState()
  videoState.last_video_stream = videoState.video_stream = -1
  videoState.last_audio_stream = videoState.audio_stream = -1
  videoState.last_subtitle_stream = videoState.subtitle_stream = -1
  videoState.filename = filename
  videoState.iformat = iformat
  videoState.ytop = 0
  videoState.xleft = 0
  
  if frame_queue_init(videoState.pictq, videoState.videoq,VIDEO_PICTURE_QUEUE_SIZE, 1) < 0:
    goto fail
  if frame_queue_init(videoState.subpq, videoState.subtitleq,SUBPICTURE_QUEUE_SIZE, 0) < 0:
    goto fail
  if frame_queue_init(videoState.sampq, videoState.audioq, SAMPLE_QUEUE_SIZE, 1) < 0:
    goto fail
  if packet_queue_init(videoState.videoq) < 0 or
      packet_queue_init(videoState.audioq) < 0 or
      packet_queue_init(videoState.subtitleq) < 0:
    goto fail
  if not (videoState.continue_read_thread = SDL_CreateCond()):
    echo(nil, AV_LOG_FATAL, "SDL_CreateCond(): %s\n", SDL_GetError())
    goto fail
  # init_clock(videoState.vidclk), videoState.videoq.serial))
  # init_clock(videoState.audclk), videoState.audioq.serial))
  # init_clock(videoState.extclk), videoState.extclk.serial))
  videoState.audio_clock_serial = -1
  if startup_volume < 0:
    echo("-volume=%d < 0, setting to 0\n", startup_volume)
  if startup_volume > 100:
    echo("-volume=%d > 100, setting to 100\n",startup_volume)
  startup_volume = av_clip(startup_volume, 0, 100)
  startup_volume = av_clip(SDL_MIX_MAXVOLUME * startup_volume div 100, 0,SDL_MIX_MAXVOLUME)
  videoState.audio_volume = startup_volume
  videoState.muted = 0
  videoState.av_sync_type = av_sync_type
  videoState.read_tid = createThread(read_thread, "read_thread", videoState)
  if not videoState.read_tid:
    stream_close(videoState)
    return nil
  return videoState

proc main(): cint =
    var  flush_pkt = AVPacket(pts: AV_NOPTS_VALUE,dts: AV_NOPTS_VALUE,pos: -1)
    var flags: cint
    var videoState: VideoState
    init_dynload()
    # avpriv_register_devices(outev_list ,indev_list)
    # avformat_network_init()
    # init_opts()
    # signal(SIGINT, sigterm_handler)
    # signal(SIGTERM, sigterm_handler)
    #   show_banner(argc, argv, options)
    # if not input_filename:
    #     show_usage()
    #     exit(1)
    var input_filename: string
    for kind, key, val in getopt():
      if kind == cmdArgument:
        input_filename = key
        break
    flags = INIT_VIDEO or INIT_AUDIO or INIT_TIMER
    # eventState(SDL_SYSWMEVENT, SDL_IGNORE)
    # eventState(SDL_USEREVENT, SDL_IGNORE)
    # flush_pkt.data = cast[uint8](flush_pkt)
    var flags: cint = SDL_WINDOW_HIDDEN
    flags = flags or SDL_WINDOW_RESIZABLE
    window = createWindow(program_name, SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED, default_width,default_height, flags)
    setHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear")
    if window:
      renderer = createRenderer(window, -1, SDL_RENDERER_ACCELERATED or SDL_RENDERER_PRESENTVSYNC)
    if not renderer:
        renderer = createRenderer(window, -1, 0)
    if renderer:
        if not getRendererInfo(renderer, renderer_info):
          echo renderer_info.name
    if not window or not renderer or not renderer_info.num_texture_formats:
      return
    s = stream_open(input_filename, file_iformat)
    event_loop(s)
    return 0
