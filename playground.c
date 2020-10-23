typedef struct SwsContext
{
    const AVClass *av_class;
    SwsFunc swscale;
    int srcW;    ///< Width  of source      luma/alpha planes.
    int srcH;    ///< Height of source      luma/alpha planes.
    int dstH;    ///< Height of destination luma/alpha planes.
    int chrSrcW; ///< Width  of source      chroma     planes.
    int chrSrcH; ///< Height of source      chroma     planes.
    int chrDstW; ///< Width  of destination chroma     planes.
    int chrDstH; ///< Height of destination chroma     planes.
    int lumXInc, chrXInc;
    int lumYInc, chrYInc;
    enum AVPixelFormat dstFormat; ///< Destination pixel format.
    enum AVPixelFormat srcFormat; ///< Source      pixel format.
    int dstFormatBpp;             ///< Number of bits per pixel of the destination pixel format.
    int srcFormatBpp;             ///< Number of bits per pixel of the source      pixel format.
    int dstBpc, srcBpc;
    int chrSrcHSubSample; ///< Binary logarithm of horizontal subsampling factor between luma/alpha and chroma planes in source      image.
    int chrSrcVSubSample; ///< Binary logarithm of vertical   subsampling factor between luma/alpha and chroma planes in source      image.
    int chrDstHSubSample; ///< Binary logarithm of horizontal subsampling factor between luma/alpha and chroma planes in destination image.
    int chrDstVSubSample; ///< Binary logarithm of vertical   subsampling factor between luma/alpha and chroma planes in destination image.
    int vChrDrop;         ///< Binary logarithm of extra vertical subsampling factor in source image chroma planes specified by user.
    int sliceDir;         ///< Direction that slices are fed to the scaler (1 = top-to-bottom, -1 = bottom-to-top).
    double param[2];      ///< Input parameters for scaling algorithms that need them.

    struct SwsContext *cascaded_context[3];
    int cascaded_tmpStride[4];
    uint8_t *cascaded_tmp[4];
    int cascaded1_tmpStride[4];
    uint8_t *cascaded1_tmp[4];
    int cascaded_mainindex;
    double gamma_value;
    int gamma_flag;
    int is_internal_gamma;
    uint16_t *gamma;
    uint16_t *inv_gamma;
    int numDesc;
    int descIndex[2];
    int numSlice;
    struct SwsSlice *slice;
    struct SwsFilterDescriptor *desc;
    uint32_t pal_yuv[256];
    uint32_t pal_rgb[256];
    float uint2float_lut[256];
    int lastInLumBuf; ///< Last scaled horizontal luma/alpha line from source in the ring buffer.
    int lastInChrBuf; ///< Last scaled horizontal chroma     line from source in the ring buffer.
    //@}
    uint8_t *formatConvBuffer;
    int needAlpha;
    int16_t *hLumFilter;    ///< Array of horizontal filter coefficients for luma/alpha planes.
    int16_t *hChrFilter;    ///< Array of horizontal filter coefficients for chroma     planes.
    int16_t *vLumFilter;    ///< Array of vertical   filter coefficients for luma/alpha planes.
    int16_t *vChrFilter;    ///< Array of vertical   filter coefficients for chroma     planes.
    int32_t *hLumFilterPos; ///< Array of horizontal filter starting positions for each dst[i] for luma/alpha planes.
    int32_t *hChrFilterPos; ///< Array of horizontal filter starting positions for each dst[i] for chroma     planes.
    int32_t *vLumFilterPos; ///< Array of vertical   filter starting positions for each dst[i] for luma/alpha planes.
    int32_t *vChrFilterPos; ///< Array of vertical   filter starting positions for each dst[i] for chroma     planes.
    int hLumFilterSize;     ///< Horizontal filter size for luma/alpha pixels.
    int hChrFilterSize;     ///< Horizontal filter size for chroma     pixels.
    int vLumFilterSize;     ///< Vertical   filter size for luma/alpha pixels.
    int vChrFilterSize;     ///< Vertical   filter size for chroma     pixels.
    //@}
    int lumMmxextFilterCodeSize;  ///< Runtime-generated MMXEXT horizontal fast bilinear scaler code size for luma/alpha planes.
    int chrMmxextFilterCodeSize;  ///< Runtime-generated MMXEXT horizontal fast bilinear scaler code size for chroma planes.
    uint8_t *lumMmxextFilterCode; ///< Runtime-generated MMXEXT horizontal fast bilinear scaler code for luma/alpha planes.
    uint8_t *chrMmxextFilterCode; ///< Runtime-generated MMXEXT horizontal fast bilinear scaler code for chroma planes.
    int canMMXEXTBeUsed;
    int warned_unuseable_bilinear;
    int dstY;       ///< Last destination vertical line output from last slice.
    int flags;      ///< Flags passed by the user to select scaler algorithm, optimizations, subsampling, etc...
    void *yuvTable; // pointer to the yuv->rgb table start so it can be freed()

    DECLARE_ALIGNED(16, int, table_gV)
    [256 + 2 * YUVRGB_TABLE_HEADROOM];
    uint8_t *table_rV[256 + 2 * YUVRGB_TABLE_HEADROOM];
    uint8_t *table_gU[256 + 2 * YUVRGB_TABLE_HEADROOM];
    uint8_t *table_bU[256 + 2 * YUVRGB_TABLE_HEADROOM];
    DECLARE_ALIGNED(16, int32_t, input_rgb2yuv_table)
    [16 + 40 * 4]; // This table can contain both C and SIMD formatted values, the C vales are always at the XY_IDX points
    int *dither_error[4];
    int contrast, brightness, saturation; // for sws_getColorspaceDetails
    int srcColorspaceTable[4];
    int dstColorspaceTable[4];
    int srcRange; ///< 0 = MPG YUV range, 1 = JPG YUV range (source      image).
    int dstRange; ///< 0 = MPG YUV range, 1 = JPG YUV range (destination image).
    int src0Alpha;
    int dst0Alpha;
    int srcXYZ;
    int dstXYZ;
    int src_h_chr_pos;
    int dst_h_chr_pos;
    int src_v_chr_pos;
    int dst_v_chr_pos;
    int yuv2rgb_y_offset;
    int yuv2rgb_y_coeff;
    int yuv2rgb_v2r_coeff;
    int yuv2rgb_v2g_coeff;
    int yuv2rgb_u2g_coeff;
    int yuv2rgb_u2b_coeff;

    DECLARE_ALIGNED(8, uint64_t, redDither);
    DECLARE_ALIGNED(8, uint64_t, greenDither);
    DECLARE_ALIGNED(8, uint64_t, blueDither);

    DECLARE_ALIGNED(8, uint64_t, yCoeff);
    DECLARE_ALIGNED(8, uint64_t, vrCoeff);
    DECLARE_ALIGNED(8, uint64_t, ubCoeff);
    DECLARE_ALIGNED(8, uint64_t, vgCoeff);
    DECLARE_ALIGNED(8, uint64_t, ugCoeff);
    DECLARE_ALIGNED(8, uint64_t, yOffset);
    DECLARE_ALIGNED(8, uint64_t, uOffset);
    DECLARE_ALIGNED(8, uint64_t, vOffset);
    int32_t lumMmxFilter[4 * MAX_FILTER_SIZE];
    int32_t chrMmxFilter[4 * MAX_FILTER_SIZE];
    int dstW; ///< Width  of destination luma/alpha planes.
    DECLARE_ALIGNED(8, uint64_t, esp);
    DECLARE_ALIGNED(8, uint64_t, vRounder);
    DECLARE_ALIGNED(8, uint64_t, u_temp);
    DECLARE_ALIGNED(8, uint64_t, v_temp);
    DECLARE_ALIGNED(8, uint64_t, y_temp);
    int32_t alpMmxFilter[4 * MAX_FILTER_SIZE];
    // alignment of these values is not necessary, but merely here
    // to maintain the same offset across x8632 and x86-64. Once we
    // use proper offset macros in the asm, they can be removed.
    DECLARE_ALIGNED(8, ptrdiff_t, uv_off);   ///< offset (in pixels) between u and v planes
    DECLARE_ALIGNED(8, ptrdiff_t, uv_offx2); ///< offset (in bytes) between u and v planes
    DECLARE_ALIGNED(8, uint16_t, dither16)
    [8];
    DECLARE_ALIGNED(8, uint32_t, dither32)
    [8];

    const uint8_t *chrDither8, *lumDither8;

#if HAVE_ALTIVEC
    vector signed short CY;
    vector signed short CRV;
    vector signed short CBU;
    vector signed short CGU;
    vector signed short CGV;
    vector signed short OY;
    vector unsigned short CSHIFT;
    vector signed short *vYCoeffsBank, *vCCoeffsBank;
#endif

    int use_mmx_vfilter;

    int16_t *xyzgamma;
    int16_t *rgbgamma;
    int16_t *xyzgammainv;
    int16_t *rgbgammainv;
    int16_t xyz2rgb_matrix[3][4];
    int16_t rgb2xyz_matrix[3][4];
    yuv2planar1_fn yuv2plane1;
    yuv2planarX_fn yuv2planeX;
    yuv2interleavedX_fn yuv2nv12cX;
    yuv2packed1_fn yuv2packed1;
    yuv2packed2_fn yuv2packed2;
    yuv2packedX_fn yuv2packedX;
    yuv2anyX_fn yuv2anyX;

    void (*lumToYV12)(uint8_t *dst, const uint8_t *src, const uint8_t *src2, const uint8_t *src3,
                      int width, uint32_t *pal);
    void (*alpToYV12)(uint8_t *dst, const uint8_t *src, const uint8_t *src2, const uint8_t *src3,
                      int width, uint32_t *pal);
    void (*chrToYV12)(uint8_t *dstU, uint8_t *dstV,
                      const uint8_t *src1, const uint8_t *src2, const uint8_t *src3,
                      int width, uint32_t *pal);

    void (*readLumPlanar)(uint8_t *dst, const uint8_t *src[4], int width, int32_t *rgb2yuv);
    void (*readChrPlanar)(uint8_t *dstU, uint8_t *dstV, const uint8_t *src[4],
                          int width, int32_t *rgb2yuv);
    void (*readAlpPlanar)(uint8_t *dst, const uint8_t *src[4], int width, int32_t *rgb2yuv);
    void (*hyscale_fast)(struct SwsContext *c,
                         int16_t *dst, int dstWidth,
                         const uint8_t *src, int srcW, int xInc);
    void (*hcscale_fast)(struct SwsContext *c,
                         int16_t *dst1, int16_t *dst2, int dstWidth,
                         const uint8_t *src1, const uint8_t *src2,
                         int srcW, int xInc);
    void (*hyScale)(struct SwsContext *c, int16_t *dst, int dstW,
                    const uint8_t *src, const int16_t *filter,
                    const int32_t *filterPos, int filterSize);
    void (*hcScale)(struct SwsContext *c, int16_t *dst, int dstW,
                    const uint8_t *src, const int16_t *filter,
                    const int32_t *filterPos, int filterSize);
    void (*lumConvertRange)(int16_t *dst, int width);
    void (*chrConvertRange)(int16_t *dst1, int16_t *dst2, int width);
    int needs_hcscale; ///< Set if there are chroma planes to be converted.
    SwsDither dither;

    SwsAlphaBlend alphablend;
} SwsContext;