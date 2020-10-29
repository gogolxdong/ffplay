
void rgb48tobgr48_nobswap(const uint8_t *src, uint8_t *dst, int src_size)
{
    uint16_t *d = (uint16_t *)dst;
    uint16_t *s = (uint16_t *)src;
    int i, num_pixels = src_size >> 1;
    for (i = 0; i < num_pixels; i += 3)
    {
        d[i] = 0 ? av_bswap16(s[i + 2]) : s[i + 2];
        d[i + 1] = 0 ? av_bswap16(s[i + 1]) : s[i + 1];
        d[i + 2] = 0 ? av_bswap16(s[i]) : s[i];
    }
}
void rgb48tobgr48_bswap(const uint8_t *src, uint8_t *dst, int src_size)
{
    uint16_t *d = (uint16_t *)dst;
    uint16_t *s = (uint16_t *)src;
    int i, num_pixels = src_size >> 1;
    for (i = 0; i < num_pixels; i += 3)
    {
        d[i] = 1 ? av_bswap16(s[i + 2]) : s[i + 2];
        d[i + 1] = 1 ? av_bswap16(s[i + 1]) : s[i + 1];
        d[i + 2] = 1 ? av_bswap16(s[i]) : s[i];
    }
}
void rgb64tobgr48_nobswap(const uint8_t *src, uint8_t *dst, int src_size)
{
    uint16_t *d = (uint16_t *)dst;
    uint16_t *s = (uint16_t *)src;
    int i, num_pixels = src_size >> 3;
    for (i = 0; i < num_pixels; i++)
    {
        d[3 * i] = 0 ? av_bswap16(s[4 * i + 2]) : s[4 * i + 2];
        d[3 * i + 1] = 0 ? av_bswap16(s[4 * i + 1]) : s[4 * i + 1];
        d[3 * i + 2] = 0 ? av_bswap16(s[4 * i]) : s[4 * i];
    }
}
void rgb64tobgr48_bswap(const uint8_t *src, uint8_t *dst, int src_size)
{
    uint16_t *d = (uint16_t *)dst;
    uint16_t *s = (uint16_t *)src;
    int i, num_pixels = src_size >> 3;
    for (i = 0; i < num_pixels; i++)
    {
        d[3 * i] = 1 ? av_bswap16(s[4 * i + 2]) : s[4 * i + 2];
        d[3 * i + 1] = 1 ? av_bswap16(s[4 * i + 1]) : s[4 * i + 1];
        d[3 * i + 2] = 1 ? av_bswap16(s[4 * i]) : s[4 * i];
    }
}
void rgb64to48_nobswap(const uint8_t *src, uint8_t *dst, int src_size)
{
    uint16_t *d = (uint16_t *)dst;
    uint16_t *s = (uint16_t *)src;
    int i, num_pixels = src_size >> 3;
    for (i = 0; i < num_pixels; i++)
    {
        d[3 * i] = 0 ? av_bswap16(s[4 * i]) : s[4 * i];
        d[3 * i + 1] = 0 ? av_bswap16(s[4 * i + 1]) : s[4 * i + 1];
        d[3 * i + 2] = 0 ? av_bswap16(s[4 * i + 2]) : s[4 * i + 2];
    }
}
void rgb64to48_bswap(const uint8_t *src, uint8_t *dst, int src_size)
{
    uint16_t *d = (uint16_t *)dst;
    uint16_t *s = (uint16_t *)src;
    int i, num_pixels = src_size >> 3;
    for (i = 0; i < num_pixels; i++)
    {
        d[3 * i] = 1 ? av_bswap16(s[4 * i]) : s[4 * i];
        d[3 * i + 1] = 1 ? av_bswap16(s[4 * i + 1]) : s[4 * i + 1];
        d[3 * i + 2] = 1 ? av_bswap16(s[4 * i + 2]) : s[4 * i + 2];
    }
}
void rgb48tobgr64_nobswap(const uint8_t *src, uint8_t *dst, int src_size)
{
    uint16_t *d = (uint16_t *)dst;
    uint16_t *s = (uint16_t *)src;
    int i, num_pixels = src_size / 6;
    for (i = 0; i < num_pixels; i++)
    {
        d[4 * i] = 0 ? av_bswap16(s[3 * i + 2]) : s[3 * i + 2];
        d[4 * i + 1] = 0 ? av_bswap16(s[3 * i + 1]) : s[3 * i + 1];
        d[4 * i + 2] = 0 ? av_bswap16(s[3 * i]) : s[3 * i];
        d[4 * i + 3] = 0xFFFF;
    }
}
void rgb48tobgr64_bswap(const uint8_t *src, uint8_t *dst, int src_size)
{
    uint16_t *d = (uint16_t *)dst;
    uint16_t *s = (uint16_t *)src;
    int i, num_pixels = src_size / 6;
    for (i = 0; i < num_pixels; i++)
    {
        d[4 * i] = 1 ? av_bswap16(s[3 * i + 2]) : s[3 * i + 2];
        d[4 * i + 1] = 1 ? av_bswap16(s[3 * i + 1]) : s[3 * i + 1];
        d[4 * i + 2] = 1 ? av_bswap16(s[3 * i]) : s[3 * i];
        d[4 * i + 3] = 0xFFFF;
    }
}
void rgb48to64_nobswap(const uint8_t *src, uint8_t *dst, int src_size)
{
    uint16_t *d = (uint16_t *)dst;
    uint16_t *s = (uint16_t *)src;
    int i, num_pixels = src_size / 6;
    for (i = 0; i < num_pixels; i++)
    {
        d[4 * i] = 0 ? av_bswap16(s[3 * i]) : s[3 * i];
        d[4 * i + 1] = 0 ? av_bswap16(s[3 * i + 1]) : s[3 * i + 1];
        d[4 * i + 2] = 0 ? av_bswap16(s[3 * i + 2]) : s[3 * i + 2];
        d[4 * i + 3] = 0xFFFF;
    }
}
void rgb48to64_bswap(const uint8_t *src, uint8_t *dst, int src_size)
{
    uint16_t *d = (uint16_t *)dst;
    uint16_t *s = (uint16_t *)src;
    int i, num_pixels = src_size / 6;
    for (i = 0; i < num_pixels; i++)
    {
        d[4 * i] = 1 ? av_bswap16(s[3 * i]) : s[3 * i];
        d[4 * i + 1] = 1 ? av_bswap16(s[3 * i + 1]) : s[3 * i + 1];
        d[4 * i + 2] = 1 ? av_bswap16(s[3 * i + 2]) : s[3 * i + 2];
        d[4 * i + 3] = 0xFFFF;
    }
}
