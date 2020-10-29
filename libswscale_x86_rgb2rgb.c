
static inline void rgb24tobgr32_mmx(const uint8_t *src, uint8_t *dst, int src_size)
{
    uint8_t *dest = dst;
    const uint8_t *s = src;
    const uint8_t *end;
    const uint8_t *mm_end;
    end = s + src_size;
    __asm__ volatile(" # nop""    %0"::"m"(*s):"memory");
    mm_end = end - 23;
    __asm__ volatile("movq        %0, %%mm7"::"m"(mask32a):"memory");
    while (s < mm_end) {
        __asm__ volatile(
            " # nop""  32(%1)           \n\t"
            "movd        (%1), %%mm0    \n\t"
            "punpckldq  3(%1), %%mm0    \n\t"
            "movd       6(%1), %%mm1    \n\t"
            "punpckldq  9(%1), %%mm1    \n\t"
            "movd      12(%1), %%mm2    \n\t"
            "punpckldq 15(%1), %%mm2    \n\t"
            "movd      18(%1), %%mm3    \n\t"
            "punpckldq 21(%1), %%mm3    \n\t"
            "por        %%mm7, %%mm0    \n\t"
            "por        %%mm7, %%mm1    \n\t"
            "por        %%mm7, %%mm2    \n\t"
            "por        %%mm7, %%mm3    \n\t"
            "movq""     %%mm0,   (%0)   \n\t"
            "movq""     %%mm1,  8(%0)   \n\t"
            "movq""     %%mm2, 16(%0)   \n\t"
            "movq""     %%mm3, 24(%0)"
            :: "r"(dest), "r"(s)
            :"memory");
        dest += 32;
        s += 24;
    }
    __asm__ volatile(" # nop":::"memory");
    __asm__ volatile("emms":::"memory");
    while (s < end) {
        *dest++ = *s++;
        *dest++ = *s++;
        *dest++ = *s++;
        *dest++ = 255;
    }
}
#define STORE_BGR24_MMX "psrlq         $8, %%mm2    \n\t" "psrlq         $8, %%mm3    \n\t" "psrlq         $8, %%mm6    \n\t" "psrlq         $8, %%mm7    \n\t" "pand "MANGLE(mask24l)", %%mm0\n\t" "pand "MANGLE(mask24l)", %%mm1\n\t" "pand "MANGLE(mask24l)", %%mm4\n\t" "pand "MANGLE(mask24l)", %%mm5\n\t" "pand "MANGLE(mask24h)", %%mm2\n\t" "pand "MANGLE(mask24h)", %%mm3\n\t" "pand "MANGLE(mask24h)", %%mm6\n\t" "pand "MANGLE(mask24h)", %%mm7\n\t" "por        %%mm2, %%mm0    \n\t" "por        %%mm3, %%mm1    \n\t" "por        %%mm6, %%mm4    \n\t" "por        %%mm7, %%mm5    \n\t" "movq       %%mm1, %%mm2    \n\t" "movq       %%mm4, %%mm3    \n\t" "psllq        $48, %%mm2    \n\t" "psllq        $32, %%mm3    \n\t" "por        %%mm2, %%mm0    \n\t" "psrlq        $16, %%mm1    \n\t" "psrlq        $32, %%mm4    \n\t" "psllq        $16, %%mm5    \n\t" "por        %%mm3, %%mm1    \n\t" "por        %%mm5, %%mm4    \n\t" MOVNTQ"     %%mm0,   (%0)    \n\t" MOVNTQ"     %%mm1,  8(%0)    \n\t" MOVNTQ"     %%mm4, 16(%0)"
static inline void rgb32tobgr24_mmx(const uint8_t *src, uint8_t *dst, int src_size)
{
    uint8_t *dest = dst;
    const uint8_t *s = src;
    const uint8_t *end;
    const uint8_t *mm_end;
    end = s + src_size;
    __asm__ volatile(" # nop""    %0"::"m"(*s):"memory");
    mm_end = end - 31;
    while (s < mm_end) {
        __asm__ volatile(
            " # nop""  32(%1)           \n\t"
            "movq        (%1), %%mm0    \n\t"
            "movq       8(%1), %%mm1    \n\t"
            "movq      16(%1), %%mm4    \n\t"
            "movq      24(%1), %%mm5    \n\t"
            "movq       %%mm0, %%mm2    \n\t"
            "movq       %%mm1, %%mm3    \n\t"
            "movq       %%mm4, %%mm6    \n\t"
            "movq       %%mm5, %%mm7    \n\t"
            "psrlq         $8, %%mm2    \n\t" "psrlq         $8, %%mm3    \n\t" "psrlq         $8, %%mm6    \n\t" "psrlq         $8, %%mm7    \n\t" "pand """ "mask24l"", %%mm0\n\t" "pand """ "mask24l"", %%mm1\n\t" "pand """ "mask24l"", %%mm4\n\t" "pand """ "mask24l"", %%mm5\n\t" "pand """ "mask24h"", %%mm2\n\t" "pand """ "mask24h"", %%mm3\n\t" "pand """ "mask24h"", %%mm6\n\t" "pand """ "mask24h"", %%mm7\n\t" "por        %%mm2, %%mm0    \n\t" "por        %%mm3, %%mm1    \n\t" "por        %%mm6, %%mm4    \n\t" "por        %%mm7, %%mm5    \n\t" "movq       %%mm1, %%mm2    \n\t" "movq       %%mm4, %%mm3    \n\t" "psllq        $48, %%mm2    \n\t" "psllq        $32, %%mm3    \n\t" "por        %%mm2, %%mm0    \n\t" "psrlq        $16, %%mm1    \n\t" "psrlq        $32, %%mm4    \n\t" "psllq        $16, %%mm5    \n\t" "por        %%mm3, %%mm1    \n\t" "por        %%mm5, %%mm4    \n\t" "movq""     %%mm0,   (%0)    \n\t" "movq""     %%mm1,  8(%0)    \n\t" "movq""     %%mm4, 16(%0)"
            :: "r"(dest), "r"(s)
             
            :"memory");
        dest += 24;
        s += 32;
    }
    __asm__ volatile(" # nop":::"memory");
    __asm__ volatile("emms":::"memory");
    while (s < end) {
        *dest++ = *s++;
        *dest++ = *s++;
        *dest++ = *s++;
        s++;
    }
}

static inline void rgb15to16_mmx(const uint8_t *src, uint8_t *dst, int src_size)
{
    register const uint8_t* s=src;
    register uint8_t* d=dst;
    register const uint8_t *end;
    const uint8_t *mm_end;
    end = s + src_size;
    __asm__ volatile(" # nop""    %0"::"m"(*s));
    __asm__ volatile("movq        %0, %%mm4"::"m"(mask15s));
    mm_end = end - 15;
    while (s<mm_end) {
        __asm__ volatile(
            " # nop"" 32(%1)        \n\t"
            "movq      (%1), %%mm0  \n\t"
            "movq     8(%1), %%mm2  \n\t"
            "movq     %%mm0, %%mm1  \n\t"
            "movq     %%mm2, %%mm3  \n\t"
            "pand     %%mm4, %%mm0  \n\t"
            "pand     %%mm4, %%mm2  \n\t"
            "paddw    %%mm1, %%mm0  \n\t"
            "paddw    %%mm3, %%mm2  \n\t"
            "movq""   %%mm0,  (%0)  \n\t"
            "movq""   %%mm2, 8(%0)"
            :: "r"(d), "r"(s)
        );
        d+=16;
        s+=16;
    }
    __asm__ volatile(" # nop":::"memory");
    __asm__ volatile("emms":::"memory");
    mm_end = end - 3;
    while (s < mm_end) {
        register unsigned x= *((const uint32_t *)s);
        *((uint32_t *)d) = (x&0x7FFF7FFF) + (x&0x7FE07FE0);
        d+=4;
        s+=4;
    }
    if (s < end) {
        register unsigned short x= *((const uint16_t *)s);
        *((uint16_t *)d) = (x&0x7FFF) + (x&0x7FE0);
    }
}

static inline void rgb16to15_mmx(const uint8_t *src, uint8_t *dst, int src_size)
{
    register const uint8_t* s=src;
    register uint8_t* d=dst;
    register const uint8_t *end;
    const uint8_t *mm_end;
    end = s + src_size;
    __asm__ volatile(" # nop""    %0"::"m"(*s));
    __asm__ volatile("movq        %0, %%mm7"::"m"(mask15rg));
    __asm__ volatile("movq        %0, %%mm6"::"m"(mask15b));
    mm_end = end - 15;
    while (s<mm_end) {
        __asm__ volatile(
            " # nop"" 32(%1)        \n\t"
            "movq      (%1), %%mm0  \n\t"
            "movq     8(%1), %%mm2  \n\t"
            "movq     %%mm0, %%mm1  \n\t"
            "movq     %%mm2, %%mm3  \n\t"
            "psrlq       $1, %%mm0  \n\t"
            "psrlq       $1, %%mm2  \n\t"
            "pand     %%mm7, %%mm0  \n\t"
            "pand     %%mm7, %%mm2  \n\t"
            "pand     %%mm6, %%mm1  \n\t"
            "pand     %%mm6, %%mm3  \n\t"
            "por      %%mm1, %%mm0  \n\t"
            "por      %%mm3, %%mm2  \n\t"
            "movq""   %%mm0,  (%0)  \n\t"
            "movq""   %%mm2, 8(%0)"
            :: "r"(d), "r"(s)
        );
        d+=16;
        s+=16;
    }
    __asm__ volatile(" # nop":::"memory");
    __asm__ volatile("emms":::"memory");
    mm_end = end - 3;
    while (s < mm_end) {
        register uint32_t x= *((const uint32_t*)s);
        *((uint32_t *)d) = ((x>>1)&0x7FE07FE0) | (x&0x001F001F);
        s+=4;
        d+=4;
    }
    if (s < end) {
        register uint16_t x= *((const uint16_t*)s);
        *((uint16_t *)d) = ((x>>1)&0x7FE0) | (x&0x001F);
    }
}

static inline void rgb32to16_mmx(const uint8_t *src, uint8_t *dst, int src_size)
{
    const uint8_t *s = src;
    const uint8_t *end;
    const uint8_t *mm_end;
    uint16_t *d = (uint16_t *)dst;
    end = s + src_size;
    mm_end = end - 15;
    __asm__ volatile(
        "movq           %3, %%mm5   \n\t"
        "movq           %4, %%mm6   \n\t"
        "movq           %5, %%mm7   \n\t"
        "jmp 2f                     \n\t"
        ".p2align        4          \n\t"
        "1:                         \n\t"
        " # nop""   32(%1)          \n\t"
        "movd         (%1), %%mm0   \n\t"
        "movd        4(%1), %%mm3   \n\t"
        "punpckldq   8(%1), %%mm0   \n\t"
        "punpckldq  12(%1), %%mm3   \n\t"
        "movq        %%mm0, %%mm1   \n\t"
        "movq        %%mm3, %%mm4   \n\t"
        "pand        %%mm6, %%mm0   \n\t"
        "pand        %%mm6, %%mm3   \n\t"
        "pmaddwd     %%mm7, %%mm0   \n\t"
        "pmaddwd     %%mm7, %%mm3   \n\t"
        "pand        %%mm5, %%mm1   \n\t"
        "pand        %%mm5, %%mm4   \n\t"
        "por         %%mm1, %%mm0   \n\t"
        "por         %%mm4, %%mm3   \n\t"
        "psrld          $5, %%mm0   \n\t"
        "pslld         $11, %%mm3   \n\t"
        "por         %%mm3, %%mm0   \n\t"
        "movq""      %%mm0, (%0)    \n\t"
        "add           $16,  %1     \n\t"
        "add            $8,  %0     \n\t"
        "2:                         \n\t"
        "cmp            %2,  %1     \n\t"
        " jb            1b          \n\t"
        : "+r" (d), "+r"(s)
        : "r" (mm_end), "m" (mask3216g), "m" (mask3216br), "m" (mul3216)
    );
    __asm__ volatile(" # nop":::"memory");
    __asm__ volatile("emms":::"memory");
    while (s < end) {
        register int rgb = *(const uint32_t*)s; s += 4;
        *d++ = ((rgb&0xFF)>>3) + ((rgb&0xFC00)>>5) + ((rgb&0xF80000)>>8);
    }
}

static inline void rgb32tobgr16_mmx(const uint8_t *src, uint8_t *dst, int src_size)
{
    const uint8_t *s = src;
    const uint8_t *end;
    const uint8_t *mm_end;
    uint16_t *d = (uint16_t *)dst;
    end = s + src_size;
    __asm__ volatile(" # nop""    %0"::"m"(*src):"memory");
    __asm__ volatile(
        "movq          %0, %%mm7    \n\t"
        "movq          %1, %%mm6    \n\t"
        ::"m"(red_16mask),"m"(green_16mask));
    mm_end = end - 15;
    while (s < mm_end) {
        __asm__ volatile(
            " # nop""  32(%1)           \n\t"
            "movd        (%1), %%mm0    \n\t"
            "movd       4(%1), %%mm3    \n\t"
            "punpckldq  8(%1), %%mm0    \n\t"
            "punpckldq 12(%1), %%mm3    \n\t"
            "movq       %%mm0, %%mm1    \n\t"
            "movq       %%mm0, %%mm2    \n\t"
            "movq       %%mm3, %%mm4    \n\t"
            "movq       %%mm3, %%mm5    \n\t"
            "psllq         $8, %%mm0    \n\t"
            "psllq         $8, %%mm3    \n\t"
            "pand       %%mm7, %%mm0    \n\t"
            "pand       %%mm7, %%mm3    \n\t"
            "psrlq         $5, %%mm1    \n\t"
            "psrlq         $5, %%mm4    \n\t"
            "pand       %%mm6, %%mm1    \n\t"
            "pand       %%mm6, %%mm4    \n\t"
            "psrlq        $19, %%mm2    \n\t"
            "psrlq        $19, %%mm5    \n\t"
            "pand          %2, %%mm2    \n\t"
            "pand          %2, %%mm5    \n\t"
            "por        %%mm1, %%mm0    \n\t"
            "por        %%mm4, %%mm3    \n\t"
            "por        %%mm2, %%mm0    \n\t"
            "por        %%mm5, %%mm3    \n\t"
            "psllq        $16, %%mm3    \n\t"
            "por        %%mm3, %%mm0    \n\t"
            "movq""     %%mm0, (%0)     \n\t"
            :: "r"(d),"r"(s),"m"(blue_16mask):"memory");
        d += 4;
        s += 16;
    }
    __asm__ volatile(" # nop":::"memory");
    __asm__ volatile("emms":::"memory");
    while (s < end) {
        register int rgb = *(const uint32_t*)s; s += 4;
        *d++ = ((rgb&0xF8)<<8) + ((rgb&0xFC00)>>5) + ((rgb&0xF80000)>>19);
    }
}

static inline void rgb32to15_mmx(const uint8_t *src, uint8_t *dst, int src_size)
{
    const uint8_t *s = src;
    const uint8_t *end;
    const uint8_t *mm_end;
    uint16_t *d = (uint16_t *)dst;
    end = s + src_size;
    mm_end = end - 15;
    __asm__ volatile(
        "movq           %3, %%mm5   \n\t"
        "movq           %4, %%mm6   \n\t"
        "movq           %5, %%mm7   \n\t"
        "jmp            2f          \n\t"
        ".p2align        4          \n\t"
        "1:                         \n\t"
        " # nop""   32(%1)          \n\t"
        "movd         (%1), %%mm0   \n\t"
        "movd        4(%1), %%mm3   \n\t"
        "punpckldq   8(%1), %%mm0   \n\t"
        "punpckldq  12(%1), %%mm3   \n\t"
        "movq        %%mm0, %%mm1   \n\t"
        "movq        %%mm3, %%mm4   \n\t"
        "pand        %%mm6, %%mm0   \n\t"
        "pand        %%mm6, %%mm3   \n\t"
        "pmaddwd     %%mm7, %%mm0   \n\t"
        "pmaddwd     %%mm7, %%mm3   \n\t"
        "pand        %%mm5, %%mm1   \n\t"
        "pand        %%mm5, %%mm4   \n\t"
        "por         %%mm1, %%mm0   \n\t"
        "por         %%mm4, %%mm3   \n\t"
        "psrld          $6, %%mm0   \n\t"
        "pslld         $10, %%mm3   \n\t"
        "por         %%mm3, %%mm0   \n\t"
        "movq""      %%mm0, (%0)    \n\t"
        "add           $16,  %1     \n\t"
        "add            $8,  %0     \n\t"
        "2:                         \n\t"
        "cmp            %2,  %1     \n\t"
        " jb            1b          \n\t"
        : "+r" (d), "+r"(s)
        : "r" (mm_end), "m" (mask3215g), "m" (mask3216br), "m" (mul3215)
    );
    __asm__ volatile(" # nop":::"memory");
    __asm__ volatile("emms":::"memory");
    while (s < end) {
        register int rgb = *(const uint32_t*)s; s += 4;
        *d++ = ((rgb&0xFF)>>3) + ((rgb&0xF800)>>6) + ((rgb&0xF80000)>>9);
    }
}

static inline void rgb32tobgr15_mmx(const uint8_t *src, uint8_t *dst, int src_size)
{
    const uint8_t *s = src;
    const uint8_t *end;
    const uint8_t *mm_end;
    uint16_t *d = (uint16_t *)dst;
    end = s + src_size;
    __asm__ volatile(" # nop""    %0"::"m"(*src):"memory");
    __asm__ volatile(
        "movq          %0, %%mm7    \n\t"
        "movq          %1, %%mm6    \n\t"
        ::"m"(red_15mask),"m"(green_15mask));
    mm_end = end - 15;
    while (s < mm_end) {
        __asm__ volatile(
            " # nop""  32(%1)           \n\t"
            "movd        (%1), %%mm0    \n\t"
            "movd       4(%1), %%mm3    \n\t"
            "punpckldq  8(%1), %%mm0    \n\t"
            "punpckldq 12(%1), %%mm3    \n\t"
            "movq       %%mm0, %%mm1    \n\t"
            "movq       %%mm0, %%mm2    \n\t"
            "movq       %%mm3, %%mm4    \n\t"
            "movq       %%mm3, %%mm5    \n\t"
            "psllq         $7, %%mm0    \n\t"
            "psllq         $7, %%mm3    \n\t"
            "pand       %%mm7, %%mm0    \n\t"
            "pand       %%mm7, %%mm3    \n\t"
            "psrlq         $6, %%mm1    \n\t"
            "psrlq         $6, %%mm4    \n\t"
            "pand       %%mm6, %%mm1    \n\t"
            "pand       %%mm6, %%mm4    \n\t"
            "psrlq        $19, %%mm2    \n\t"
            "psrlq        $19, %%mm5    \n\t"
            "pand          %2, %%mm2    \n\t"
            "pand          %2, %%mm5    \n\t"
            "por        %%mm1, %%mm0    \n\t"
            "por        %%mm4, %%mm3    \n\t"
            "por        %%mm2, %%mm0    \n\t"
            "por        %%mm5, %%mm3    \n\t"
            "psllq        $16, %%mm3    \n\t"
            "por        %%mm3, %%mm0    \n\t"
            "movq""     %%mm0, (%0)     \n\t"
            ::"r"(d),"r"(s),"m"(blue_15mask):"memory");
        d += 4;
        s += 16;
    }
    __asm__ volatile(" # nop":::"memory");
    __asm__ volatile("emms":::"memory");
    while (s < end) {
        register int rgb = *(const uint32_t*)s; s += 4;
        *d++ = ((rgb&0xF8)<<7) + ((rgb&0xF800)>>6) + ((rgb&0xF80000)>>19);
    }
}

static inline void rgb24tobgr16_mmx(const uint8_t *src, uint8_t *dst, int src_size)
{
    const uint8_t *s = src;
    const uint8_t *end;
    const uint8_t *mm_end;
    uint16_t *d = (uint16_t *)dst;
    end = s + src_size;
    __asm__ volatile(" # nop""    %0"::"m"(*src):"memory");
    __asm__ volatile(
        "movq         %0, %%mm7     \n\t"
        "movq         %1, %%mm6     \n\t"
        ::"m"(red_16mask),"m"(green_16mask));
    mm_end = end - 11;
    while (s < mm_end) {
        __asm__ volatile(
            " # nop""  32(%1)           \n\t"
            "movd        (%1), %%mm0    \n\t"
            "movd       3(%1), %%mm3    \n\t"
            "punpckldq  6(%1), %%mm0    \n\t"
            "punpckldq  9(%1), %%mm3    \n\t"
            "movq       %%mm0, %%mm1    \n\t"
            "movq       %%mm0, %%mm2    \n\t"
            "movq       %%mm3, %%mm4    \n\t"
            "movq       %%mm3, %%mm5    \n\t"
            "psrlq         $3, %%mm0    \n\t"
            "psrlq         $3, %%mm3    \n\t"
            "pand          %2, %%mm0    \n\t"
            "pand          %2, %%mm3    \n\t"
            "psrlq         $5, %%mm1    \n\t"
            "psrlq         $5, %%mm4    \n\t"
            "pand       %%mm6, %%mm1    \n\t"
            "pand       %%mm6, %%mm4    \n\t"
            "psrlq         $8, %%mm2    \n\t"
            "psrlq         $8, %%mm5    \n\t"
            "pand       %%mm7, %%mm2    \n\t"
            "pand       %%mm7, %%mm5    \n\t"
            "por        %%mm1, %%mm0    \n\t"
            "por        %%mm4, %%mm3    \n\t"
            "por        %%mm2, %%mm0    \n\t"
            "por        %%mm5, %%mm3    \n\t"
            "psllq        $16, %%mm3    \n\t"
            "por        %%mm3, %%mm0    \n\t"
            "movq""     %%mm0, (%0)     \n\t"
            ::"r"(d),"r"(s),"m"(blue_16mask):"memory");
        d += 4;
        s += 12;
    }
    __asm__ volatile(" # nop":::"memory");
    __asm__ volatile("emms":::"memory");
    while (s < end) {
        const int b = *s++;
        const int g = *s++;
        const int r = *s++;
        *d++ = (b>>3) | ((g&0xFC)<<3) | ((r&0xF8)<<8);
    }
}

static inline void rgb24to16_mmx(const uint8_t *src, uint8_t *dst, int src_size)
{
    const uint8_t *s = src;
    const uint8_t *end;
    const uint8_t *mm_end;
    uint16_t *d = (uint16_t *)dst;
    end = s + src_size;
    __asm__ volatile(" # nop""    %0"::"m"(*src):"memory");
    __asm__ volatile(
        "movq         %0, %%mm7     \n\t"
        "movq         %1, %%mm6     \n\t"
        ::"m"(red_16mask),"m"(green_16mask));
    mm_end = end - 15;
    while (s < mm_end) {
        __asm__ volatile(
            " # nop""  32(%1)           \n\t"
            "movd        (%1), %%mm0    \n\t"
            "movd       3(%1), %%mm3    \n\t"
            "punpckldq  6(%1), %%mm0    \n\t"
            "punpckldq  9(%1), %%mm3    \n\t"
            "movq       %%mm0, %%mm1    \n\t"
            "movq       %%mm0, %%mm2    \n\t"
            "movq       %%mm3, %%mm4    \n\t"
            "movq       %%mm3, %%mm5    \n\t"
            "psllq         $8, %%mm0    \n\t"
            "psllq         $8, %%mm3    \n\t"
            "pand       %%mm7, %%mm0    \n\t"
            "pand       %%mm7, %%mm3    \n\t"
            "psrlq         $5, %%mm1    \n\t"
            "psrlq         $5, %%mm4    \n\t"
            "pand       %%mm6, %%mm1    \n\t"
            "pand       %%mm6, %%mm4    \n\t"
            "psrlq        $19, %%mm2    \n\t"
            "psrlq        $19, %%mm5    \n\t"
            "pand          %2, %%mm2    \n\t"
            "pand          %2, %%mm5    \n\t"
            "por        %%mm1, %%mm0    \n\t"
            "por        %%mm4, %%mm3    \n\t"
            "por        %%mm2, %%mm0    \n\t"
            "por        %%mm5, %%mm3    \n\t"
            "psllq        $16, %%mm3    \n\t"
            "por        %%mm3, %%mm0    \n\t"
            "movq""     %%mm0, (%0)     \n\t"
            ::"r"(d),"r"(s),"m"(blue_16mask):"memory");
        d += 4;
        s += 12;
    }
    __asm__ volatile(" # nop":::"memory");
    __asm__ volatile("emms":::"memory");
    while (s < end) {
        const int r = *s++;
        const int g = *s++;
        const int b = *s++;
        *d++ = (b>>3) | ((g&0xFC)<<3) | ((r&0xF8)<<8);
    }
}

static inline void rgb24tobgr15_mmx(const uint8_t *src, uint8_t *dst, int src_size)
{
    const uint8_t *s = src;
    const uint8_t *end;
    const uint8_t *mm_end;
    uint16_t *d = (uint16_t *)dst;
    end = s + src_size;
    __asm__ volatile(" # nop""    %0"::"m"(*src):"memory");
    __asm__ volatile(
        "movq          %0, %%mm7    \n\t"
        "movq          %1, %%mm6    \n\t"
        ::"m"(red_15mask),"m"(green_15mask));
    mm_end = end - 11;
    while (s < mm_end) {
        __asm__ volatile(
            " # nop""  32(%1)           \n\t"
            "movd        (%1), %%mm0    \n\t"
            "movd       3(%1), %%mm3    \n\t"
            "punpckldq  6(%1), %%mm0    \n\t"
            "punpckldq  9(%1), %%mm3    \n\t"
            "movq       %%mm0, %%mm1    \n\t"
            "movq       %%mm0, %%mm2    \n\t"
            "movq       %%mm3, %%mm4    \n\t"
            "movq       %%mm3, %%mm5    \n\t"
            "psrlq         $3, %%mm0    \n\t"
            "psrlq         $3, %%mm3    \n\t"
            "pand          %2, %%mm0    \n\t"
            "pand          %2, %%mm3    \n\t"
            "psrlq         $6, %%mm1    \n\t"
            "psrlq         $6, %%mm4    \n\t"
            "pand       %%mm6, %%mm1    \n\t"
            "pand       %%mm6, %%mm4    \n\t"
            "psrlq         $9, %%mm2    \n\t"
            "psrlq         $9, %%mm5    \n\t"
            "pand       %%mm7, %%mm2    \n\t"
            "pand       %%mm7, %%mm5    \n\t"
            "por        %%mm1, %%mm0    \n\t"
            "por        %%mm4, %%mm3    \n\t"
            "por        %%mm2, %%mm0    \n\t"
            "por        %%mm5, %%mm3    \n\t"
            "psllq        $16, %%mm3    \n\t"
            "por        %%mm3, %%mm0    \n\t"
            "movq""     %%mm0, (%0)     \n\t"
            ::"r"(d),"r"(s),"m"(blue_15mask):"memory");
        d += 4;
        s += 12;
    }
    __asm__ volatile(" # nop":::"memory");
    __asm__ volatile("emms":::"memory");
    while (s < end) {
        const int b = *s++;
        const int g = *s++;
        const int r = *s++;
        *d++ = (b>>3) | ((g&0xF8)<<2) | ((r&0xF8)<<7);
    }
}

static inline void rgb24to15_mmx(const uint8_t *src, uint8_t *dst, int src_size)
{
    const uint8_t *s = src;
    const uint8_t *end;
    const uint8_t *mm_end;
    uint16_t *d = (uint16_t *)dst;
    end = s + src_size;
    __asm__ volatile(" # nop""    %0"::"m"(*src):"memory");
    __asm__ volatile(
        "movq         %0, %%mm7     \n\t"
        "movq         %1, %%mm6     \n\t"
        ::"m"(red_15mask),"m"(green_15mask));
    mm_end = end - 15;
    while (s < mm_end) {
        __asm__ volatile(
            " # nop"" 32(%1)            \n\t"
            "movd       (%1), %%mm0     \n\t"
            "movd      3(%1), %%mm3     \n\t"
            "punpckldq 6(%1), %%mm0     \n\t"
            "punpckldq 9(%1), %%mm3     \n\t"
            "movq      %%mm0, %%mm1     \n\t"
            "movq      %%mm0, %%mm2     \n\t"
            "movq      %%mm3, %%mm4     \n\t"
            "movq      %%mm3, %%mm5     \n\t"
            "psllq        $7, %%mm0     \n\t"
            "psllq        $7, %%mm3     \n\t"
            "pand      %%mm7, %%mm0     \n\t"
            "pand      %%mm7, %%mm3     \n\t"
            "psrlq        $6, %%mm1     \n\t"
            "psrlq        $6, %%mm4     \n\t"
            "pand      %%mm6, %%mm1     \n\t"
            "pand      %%mm6, %%mm4     \n\t"
            "psrlq       $19, %%mm2     \n\t"
            "psrlq       $19, %%mm5     \n\t"
            "pand         %2, %%mm2     \n\t"
            "pand         %2, %%mm5     \n\t"
            "por       %%mm1, %%mm0     \n\t"
            "por       %%mm4, %%mm3     \n\t"
            "por       %%mm2, %%mm0     \n\t"
            "por       %%mm5, %%mm3     \n\t"
            "psllq       $16, %%mm3     \n\t"
            "por       %%mm3, %%mm0     \n\t"
            "movq""    %%mm0, (%0)      \n\t"
            ::"r"(d),"r"(s),"m"(blue_15mask):"memory");
        d += 4;
        s += 12;
    }
    __asm__ volatile(" # nop":::"memory");
    __asm__ volatile("emms":::"memory");
    while (s < end) {
        const int r = *s++;
        const int g = *s++;
        const int b = *s++;
        *d++ = (b>>3) | ((g&0xF8)<<2) | ((r&0xF8)<<7);
    }
}

static inline void rgb15tobgr24_mmx(const uint8_t *src, uint8_t *dst, int src_size)
{
    const uint16_t *end;
    const uint16_t *mm_end;
    uint8_t *d = dst;
    const uint16_t *s = (const uint16_t*)src;
    end = s + src_size/2;
    __asm__ volatile(" # nop""    %0"::"m"(*s):"memory");
    mm_end = end - 7;
    while (s < mm_end) {
        __asm__ volatile(
            " # nop""  32(%1)           \n\t"
            "movq        (%1), %%mm0    \n\t"
            "movq        (%1), %%mm1    \n\t"
            "movq        (%1), %%mm2    \n\t"
            "pand          %2, %%mm0    \n\t"
            "pand          %3, %%mm1    \n\t"
            "pand          %4, %%mm2    \n\t"
            "psllq         $5, %%mm0    \n\t"
            "pmulhw        """ "mul15_mid"", %%mm0    \n\t"
            "pmulhw        """ "mul15_mid"", %%mm1    \n\t"
            "pmulhw        """ "mul15_hi"", %%mm2    \n\t"
            "movq       %%mm0, %%mm3    \n\t"
            "movq       %%mm1, %%mm4    \n\t"
            "movq       %%mm2, %%mm5    \n\t"
            "punpcklwd     %5, %%mm0    \n\t"
            "punpcklwd     %5, %%mm1    \n\t"
            "punpcklwd     %5, %%mm2    \n\t"
            "punpckhwd     %5, %%mm3    \n\t"
            "punpckhwd     %5, %%mm4    \n\t"
            "punpckhwd     %5, %%mm5    \n\t"
            "psllq         $8, %%mm1    \n\t"
            "psllq        $16, %%mm2    \n\t"
            "por        %%mm1, %%mm0    \n\t"
            "por        %%mm2, %%mm0    \n\t"
            "psllq         $8, %%mm4    \n\t"
            "psllq        $16, %%mm5    \n\t"
            "por        %%mm4, %%mm3    \n\t"
            "por        %%mm5, %%mm3    \n\t"
            "movq       %%mm0, %%mm6    \n\t"
            "movq       %%mm3, %%mm7    \n\t"
            "movq       8(%1), %%mm0    \n\t"
            "movq       8(%1), %%mm1    \n\t"
            "movq       8(%1), %%mm2    \n\t"
            "pand          %2, %%mm0    \n\t"
            "pand          %3, %%mm1    \n\t"
            "pand          %4, %%mm2    \n\t"
            "psllq         $5, %%mm0    \n\t"
            "pmulhw        """ "mul15_mid"", %%mm0    \n\t"
            "pmulhw        """ "mul15_mid"", %%mm1    \n\t"
            "pmulhw        """ "mul15_hi"", %%mm2    \n\t"
            "movq       %%mm0, %%mm3    \n\t"
            "movq       %%mm1, %%mm4    \n\t"
            "movq       %%mm2, %%mm5    \n\t"
            "punpcklwd     %5, %%mm0    \n\t"
            "punpcklwd     %5, %%mm1    \n\t"
            "punpcklwd     %5, %%mm2    \n\t"
            "punpckhwd     %5, %%mm3    \n\t"
            "punpckhwd     %5, %%mm4    \n\t"
            "punpckhwd     %5, %%mm5    \n\t"
            "psllq         $8, %%mm1    \n\t"
            "psllq        $16, %%mm2    \n\t"
            "por        %%mm1, %%mm0    \n\t"
            "por        %%mm2, %%mm0    \n\t"
            "psllq         $8, %%mm4    \n\t"
            "psllq        $16, %%mm5    \n\t"
            "por        %%mm4, %%mm3    \n\t"
            "por        %%mm5, %%mm3    \n\t"
            :"=m"(*d)
            :"r"(s),"m"(mask15b),"m"(mask15g),"m"(mask15r), "m"(mmx_null)
            
            :"memory");
        __asm__ volatile(
            "movq       %%mm0, %%mm4    \n\t"
            "movq       %%mm3, %%mm5    \n\t"
            "movq       %%mm6, %%mm0    \n\t"
            "movq       %%mm7, %%mm1    \n\t"
            "movq       %%mm4, %%mm6    \n\t"
            "movq       %%mm5, %%mm7    \n\t"
            "movq       %%mm0, %%mm2    \n\t"
            "movq       %%mm1, %%mm3    \n\t"
            "psrlq         $8, %%mm2    \n\t" "psrlq         $8, %%mm3    \n\t" "psrlq         $8, %%mm6    \n\t" "psrlq         $8, %%mm7    \n\t" "pand """ "mask24l"", %%mm0\n\t" "pand """ "mask24l"", %%mm1\n\t" "pand """ "mask24l"", %%mm4\n\t" "pand """ "mask24l"", %%mm5\n\t" "pand """ "mask24h"", %%mm2\n\t" "pand """ "mask24h"", %%mm3\n\t" "pand """ "mask24h"", %%mm6\n\t" "pand """ "mask24h"", %%mm7\n\t" "por        %%mm2, %%mm0    \n\t" "por        %%mm3, %%mm1    \n\t" "por        %%mm6, %%mm4    \n\t" "por        %%mm7, %%mm5    \n\t" "movq       %%mm1, %%mm2    \n\t" "movq       %%mm4, %%mm3    \n\t" "psllq        $48, %%mm2    \n\t" "psllq        $32, %%mm3    \n\t" "por        %%mm2, %%mm0    \n\t" "psrlq        $16, %%mm1    \n\t" "psrlq        $32, %%mm4    \n\t" "psllq        $16, %%mm5    \n\t" "por        %%mm3, %%mm1    \n\t" "por        %%mm5, %%mm4    \n\t" "movq""     %%mm0,   (%0)    \n\t" "movq""     %%mm1,  8(%0)    \n\t" "movq""     %%mm4, 16(%0)"
            :: "r"(d), "m"(*s)
             
            :"memory");
        d += 24;
        s += 8;
    }
    __asm__ volatile(" # nop":::"memory");
    __asm__ volatile("emms":::"memory");
    while (s < end) {
        register uint16_t bgr;
        bgr = *s++;
        *d++ = ((bgr&0x1F)<<3) | ((bgr&0x1F)>>2);
        *d++ = ((bgr&0x3E0)>>2) | ((bgr&0x3E0)>>7);
        *d++ = ((bgr&0x7C00)>>7) | ((bgr&0x7C00)>>12);
    }
}

static inline void rgb16tobgr24_mmx(const uint8_t *src, uint8_t *dst, int src_size)
{
    const uint16_t *end;
    const uint16_t *mm_end;
    uint8_t *d = (uint8_t *)dst;
    const uint16_t *s = (const uint16_t *)src;
    end = s + src_size/2;
    __asm__ volatile(" # nop""    %0"::"m"(*s):"memory");
    mm_end = end - 7;
    while (s < mm_end) {
        __asm__ volatile(
            " # nop""  32(%1)           \n\t"
            "movq        (%1), %%mm0    \n\t"
            "movq        (%1), %%mm1    \n\t"
            "movq        (%1), %%mm2    \n\t"
            "pand          %2, %%mm0    \n\t"
            "pand          %3, %%mm1    \n\t"
            "pand          %4, %%mm2    \n\t"
            "psllq         $5, %%mm0    \n\t"
            "psrlq         $1, %%mm2    \n\t"
            "pmulhw        """ "mul15_mid"", %%mm0    \n\t"
            "pmulhw        """ "mul16_mid"", %%mm1    \n\t"
            "pmulhw        """ "mul15_hi"", %%mm2    \n\t"
            "movq       %%mm0, %%mm3    \n\t"
            "movq       %%mm1, %%mm4    \n\t"
            "movq       %%mm2, %%mm5    \n\t"
            "punpcklwd     %5, %%mm0    \n\t"
            "punpcklwd     %5, %%mm1    \n\t"
            "punpcklwd     %5, %%mm2    \n\t"
            "punpckhwd     %5, %%mm3    \n\t"
            "punpckhwd     %5, %%mm4    \n\t"
            "punpckhwd     %5, %%mm5    \n\t"
            "psllq         $8, %%mm1    \n\t"
            "psllq        $16, %%mm2    \n\t"
            "por        %%mm1, %%mm0    \n\t"
            "por        %%mm2, %%mm0    \n\t"
            "psllq         $8, %%mm4    \n\t"
            "psllq        $16, %%mm5    \n\t"
            "por        %%mm4, %%mm3    \n\t"
            "por        %%mm5, %%mm3    \n\t"
            "movq       %%mm0, %%mm6    \n\t"
            "movq       %%mm3, %%mm7    \n\t"
            "movq       8(%1), %%mm0    \n\t"
            "movq       8(%1), %%mm1    \n\t"
            "movq       8(%1), %%mm2    \n\t"
            "pand          %2, %%mm0    \n\t"
            "pand          %3, %%mm1    \n\t"
            "pand          %4, %%mm2    \n\t"
            "psllq         $5, %%mm0    \n\t"
            "psrlq         $1, %%mm2    \n\t"
            "pmulhw        """ "mul15_mid"", %%mm0    \n\t"
            "pmulhw        """ "mul16_mid"", %%mm1    \n\t"
            "pmulhw        """ "mul15_hi"", %%mm2    \n\t"
            "movq       %%mm0, %%mm3    \n\t"
            "movq       %%mm1, %%mm4    \n\t"
            "movq       %%mm2, %%mm5    \n\t"
            "punpcklwd     %5, %%mm0    \n\t"
            "punpcklwd     %5, %%mm1    \n\t"
            "punpcklwd     %5, %%mm2    \n\t"
            "punpckhwd     %5, %%mm3    \n\t"
            "punpckhwd     %5, %%mm4    \n\t"
            "punpckhwd     %5, %%mm5    \n\t"
            "psllq         $8, %%mm1    \n\t"
            "psllq        $16, %%mm2    \n\t"
            "por        %%mm1, %%mm0    \n\t"
            "por        %%mm2, %%mm0    \n\t"
            "psllq         $8, %%mm4    \n\t"
            "psllq        $16, %%mm5    \n\t"
            "por        %%mm4, %%mm3    \n\t"
            "por        %%mm5, %%mm3    \n\t"
            :"=m"(*d)
            :"r"(s),"m"(mask15b),"m"(mask16g),"m"(mask16r),"m"(mmx_null)
            
            :"memory");
        __asm__ volatile(
            "movq       %%mm0, %%mm4    \n\t"
            "movq       %%mm3, %%mm5    \n\t"
            "movq       %%mm6, %%mm0    \n\t"
            "movq       %%mm7, %%mm1    \n\t"
            "movq       %%mm4, %%mm6    \n\t"
            "movq       %%mm5, %%mm7    \n\t"
            "movq       %%mm0, %%mm2    \n\t"
            "movq       %%mm1, %%mm3    \n\t"
            "psrlq         $8, %%mm2    \n\t" "psrlq         $8, %%mm3    \n\t" "psrlq         $8, %%mm6    \n\t" "psrlq         $8, %%mm7    \n\t" "pand """ "mask24l"", %%mm0\n\t" "pand """ "mask24l"", %%mm1\n\t" "pand """ "mask24l"", %%mm4\n\t" "pand """ "mask24l"", %%mm5\n\t" "pand """ "mask24h"", %%mm2\n\t" "pand """ "mask24h"", %%mm3\n\t" "pand """ "mask24h"", %%mm6\n\t" "pand """ "mask24h"", %%mm7\n\t" "por        %%mm2, %%mm0    \n\t" "por        %%mm3, %%mm1    \n\t" "por        %%mm6, %%mm4    \n\t" "por        %%mm7, %%mm5    \n\t" "movq       %%mm1, %%mm2    \n\t" "movq       %%mm4, %%mm3    \n\t" "psllq        $48, %%mm2    \n\t" "psllq        $32, %%mm3    \n\t" "por        %%mm2, %%mm0    \n\t" "psrlq        $16, %%mm1    \n\t" "psrlq        $32, %%mm4    \n\t" "psllq        $16, %%mm5    \n\t" "por        %%mm3, %%mm1    \n\t" "por        %%mm5, %%mm4    \n\t" "movq""     %%mm0,   (%0)    \n\t" "movq""     %%mm1,  8(%0)    \n\t" "movq""     %%mm4, 16(%0)"
            :: "r"(d), "m"(*s)
             
            :"memory");
        d += 24;
        s += 8;
    }
    __asm__ volatile(" # nop":::"memory");
    __asm__ volatile("emms":::"memory");
    while (s < end) {
        register uint16_t bgr;
        bgr = *s++;
        *d++ = ((bgr&0x1F)<<3) | ((bgr&0x1F)>>2);
        *d++ = ((bgr&0x7E0)>>3) | ((bgr&0x7E0)>>9);
        *d++ = ((bgr&0xF800)>>8) | ((bgr&0xF800)>>13);
    }
}
#define PACK_RGB32 "packuswb   %%mm7, %%mm0    \n\t" "packuswb   %%mm7, %%mm1    \n\t" "packuswb   %%mm7, %%mm2    \n\t" "punpcklbw  %%mm1, %%mm0    \n\t" "punpcklbw  %%mm6, %%mm2    \n\t" "movq       %%mm0, %%mm3    \n\t" "punpcklwd  %%mm2, %%mm0    \n\t" "punpckhwd  %%mm2, %%mm3    \n\t" MOVNTQ"     %%mm0,  (%0)    \n\t" MOVNTQ"     %%mm3, 8(%0)    \n\t"
static inline void rgb15to32_mmx(const uint8_t *src, uint8_t *dst, int src_size)
{
    const uint16_t *end;
    const uint16_t *mm_end;
    uint8_t *d = dst;
    const uint16_t *s = (const uint16_t *)src;
    end = s + src_size/2;
    __asm__ volatile(" # nop""    %0"::"m"(*s):"memory");
    __asm__ volatile("pxor    %%mm7,%%mm7    \n\t":::"memory");
    __asm__ volatile("pcmpeqd %%mm6,%%mm6    \n\t":::"memory");
    mm_end = end - 3;
    while (s < mm_end) {
        __asm__ volatile(
            " # nop""  32(%1)           \n\t"
            "movq        (%1), %%mm0    \n\t"
            "movq        (%1), %%mm1    \n\t"
            "movq        (%1), %%mm2    \n\t"
            "pand          %2, %%mm0    \n\t"
            "pand          %3, %%mm1    \n\t"
            "pand          %4, %%mm2    \n\t"
            "psllq         $5, %%mm0    \n\t"
            "pmulhw        %5, %%mm0    \n\t"
            "pmulhw        %5, %%mm1    \n\t"
            "pmulhw        """ "mul15_hi"", %%mm2    \n\t"
            "packuswb   %%mm7, %%mm0    \n\t" "packuswb   %%mm7, %%mm1    \n\t" "packuswb   %%mm7, %%mm2    \n\t" "punpcklbw  %%mm1, %%mm0    \n\t" "punpcklbw  %%mm6, %%mm2    \n\t" "movq       %%mm0, %%mm3    \n\t" "punpcklwd  %%mm2, %%mm0    \n\t" "punpckhwd  %%mm2, %%mm3    \n\t" "movq""     %%mm0,  (%0)    \n\t" "movq""     %%mm3, 8(%0)    \n\t"
            ::"r"(d),"r"(s),"m"(mask15b),"m"(mask15g),"m"(mask15r) ,"m"(mul15_mid)
             
            :"memory");
        d += 16;
        s += 4;
    }
    __asm__ volatile(" # nop":::"memory");
    __asm__ volatile("emms":::"memory");
    while (s < end) {
        register uint16_t bgr;
        bgr = *s++;
        *d++ = ((bgr&0x1F)<<3) | ((bgr&0x1F)>>2);
        *d++ = ((bgr&0x3E0)>>2) | ((bgr&0x3E0)>>7);
        *d++ = ((bgr&0x7C00)>>7) | ((bgr&0x7C00)>>12);
        *d++ = 255;
    }
}

static inline void rgb16to32_mmx(const uint8_t *src, uint8_t *dst, int src_size)
{
    const uint16_t *end;
    const uint16_t *mm_end;
    uint8_t *d = dst;
    const uint16_t *s = (const uint16_t*)src;
    end = s + src_size/2;
    __asm__ volatile(" # nop""    %0"::"m"(*s):"memory");
    __asm__ volatile("pxor    %%mm7,%%mm7    \n\t":::"memory");
    __asm__ volatile("pcmpeqd %%mm6,%%mm6    \n\t":::"memory");
    mm_end = end - 3;
    while (s < mm_end) {
        __asm__ volatile(
            " # nop""  32(%1)           \n\t"
            "movq        (%1), %%mm0    \n\t"
            "movq        (%1), %%mm1    \n\t"
            "movq        (%1), %%mm2    \n\t"
            "pand          %2, %%mm0    \n\t"
            "pand          %3, %%mm1    \n\t"
            "pand          %4, %%mm2    \n\t"
            "psllq         $5, %%mm0    \n\t"
            "psrlq         $1, %%mm2    \n\t"
            "pmulhw        %5, %%mm0    \n\t"
            "pmulhw        """ "mul16_mid"", %%mm1    \n\t"
            "pmulhw        """ "mul15_hi"", %%mm2    \n\t"
            "packuswb   %%mm7, %%mm0    \n\t" "packuswb   %%mm7, %%mm1    \n\t" "packuswb   %%mm7, %%mm2    \n\t" "punpcklbw  %%mm1, %%mm0    \n\t" "punpcklbw  %%mm6, %%mm2    \n\t" "movq       %%mm0, %%mm3    \n\t" "punpcklwd  %%mm2, %%mm0    \n\t" "punpckhwd  %%mm2, %%mm3    \n\t" "movq""     %%mm0,  (%0)    \n\t" "movq""     %%mm3, 8(%0)    \n\t"
            ::"r"(d),"r"(s),"m"(mask15b),"m"(mask16g),"m"(mask16r),"m"(mul15_mid)
             
            :"memory");
        d += 16;
        s += 4;
    }
    __asm__ volatile(" # nop":::"memory");
    __asm__ volatile("emms":::"memory");
    while (s < end) {
        register uint16_t bgr;
        bgr = *s++;
        *d++ = ((bgr&0x1F)<<3) | ((bgr&0x1F)>>2);
        *d++ = ((bgr&0x7E0)>>3) | ((bgr&0x7E0)>>9);
        *d++ = ((bgr&0xF800)>>8) | ((bgr&0xF800)>>13);
        *d++ = 255;
    }
}

static inline void rgb24tobgr24_mmx(const uint8_t *src, uint8_t *dst, int src_size)
{
    unsigned i;
    x86_reg mmx_size= 23 - src_size;
    __asm__ volatile (
        "test             %%""rax"", %%""rax""    \n\t"
        "jns                     2f                     \n\t"
        "movq     """ "mask24r"", %%mm5              \n\t"
        "movq     """ "mask24g"", %%mm6              \n\t"
        "movq     """ "mask24b"", %%mm7              \n\t"
        ".p2align                 4                     \n\t"
        "1:                                             \n\t"
        " # nop"" 32(%1, %%""rax"")                  \n\t"
        "movq    (%1, %%""rax""), %%mm0              \n\t"
        "movq    (%1, %%""rax""), %%mm1              \n\t"
        "movq   2(%1, %%""rax""), %%mm2              \n\t"
        "psllq                  $16, %%mm0              \n\t"
        "pand                 %%mm5, %%mm0              \n\t"
        "pand                 %%mm6, %%mm1              \n\t"
        "pand                 %%mm7, %%mm2              \n\t"
        "por                  %%mm0, %%mm1              \n\t"
        "por                  %%mm2, %%mm1              \n\t"
        "movq   6(%1, %%""rax""), %%mm0              \n\t"
        "movq""               %%mm1,(%2, %%""rax"")  \n\t"
        "movq   8(%1, %%""rax""), %%mm1              \n\t"
        "movq  10(%1, %%""rax""), %%mm2              \n\t"
        "pand                 %%mm7, %%mm0              \n\t"
        "pand                 %%mm5, %%mm1              \n\t"
        "pand                 %%mm6, %%mm2              \n\t"
        "por                  %%mm0, %%mm1              \n\t"
        "por                  %%mm2, %%mm1              \n\t"
        "movq  14(%1, %%""rax""), %%mm0              \n\t"
        "movq""               %%mm1, 8(%2, %%""rax"")\n\t"
        "movq  16(%1, %%""rax""), %%mm1              \n\t"
        "movq  18(%1, %%""rax""), %%mm2              \n\t"
        "pand                 %%mm6, %%mm0              \n\t"
        "pand                 %%mm7, %%mm1              \n\t"
        "pand                 %%mm5, %%mm2              \n\t"
        "por                  %%mm0, %%mm1              \n\t"
        "por                  %%mm2, %%mm1              \n\t"
        "movq""               %%mm1, 16(%2, %%""rax"") \n\t"
        "add                    $24, %%""rax""       \n\t"
        " js                     1b                     \n\t"
        "2:                                             \n\t"
        : "+a" (mmx_size)
        : "r" (src-mmx_size), "r"(dst-mmx_size)
         
    );
    __asm__ volatile(" # nop":::"memory");
    __asm__ volatile("emms":::"memory");
    if (mmx_size==23) return;
    src+= src_size;
    dst+= src_size;
    src_size= 23-mmx_size;
    src-= src_size;
    dst-= src_size;
    for (i=0; i<src_size; i+=3) {
        register uint8_t x;
        x = src[i + 2];
        dst[i + 1] = src[i + 1];
        dst[i + 2] = src[i + 0];
        dst[i + 0] = x;
    }
}

static inline void yuvPlanartoyuy2_mmx(const uint8_t *ysrc, const uint8_t *usrc, const uint8_t *vsrc, uint8_t *dst,
                                           int width, int height,
                                           int lumStride, int chromStride, int dstStride, int vertLumPerChroma)
{
    int y;
    const x86_reg chromWidth= width>>1;
    for (y=0; y<height; y++) {
        __asm__ volatile(
            "xor                 %%""rax"", %%""rax"" \n\t"
            ".p2align                    4              \n\t"
            "1:                                         \n\t"
            " # nop"" 32(%1, %%""rax"", 2)           \n\t"
            " # nop"" 32(%2, %%""rax"")              \n\t"
            " # nop"" 32(%3, %%""rax"")              \n\t"
            "movq       (%2, %%""rax""), %%mm0       \n\t"
            "movq                    %%mm0, %%mm2       \n\t"
            "movq       (%3, %%""rax""), %%mm1       \n\t"
            "punpcklbw               %%mm1, %%mm0       \n\t"
            "punpckhbw               %%mm1, %%mm2       \n\t"
            "movq     (%1, %%""rax"",2), %%mm3       \n\t"
            "movq    8(%1, %%""rax"",2), %%mm5       \n\t"
            "movq                    %%mm3, %%mm4       \n\t"
            "movq                    %%mm5, %%mm6       \n\t"
            "punpcklbw               %%mm0, %%mm3       \n\t"
            "punpckhbw               %%mm0, %%mm4       \n\t"
            "punpcklbw               %%mm2, %%mm5       \n\t"
            "punpckhbw               %%mm2, %%mm6       \n\t"
            "movq""                  %%mm3,   (%0, %%""rax"", 4)    \n\t"
            "movq""                  %%mm4,  8(%0, %%""rax"", 4)    \n\t"
            "movq""                  %%mm5, 16(%0, %%""rax"", 4)    \n\t"
            "movq""                  %%mm6, 24(%0, %%""rax"", 4)    \n\t"
            "add                        $8, %%""rax"" \n\t"
            "cmp                        %4, %%""rax"" \n\t"
            " jb                        1b               \n\t"
            ::"r"(dst), "r"(ysrc), "r"(usrc), "r"(vsrc), "g" (chromWidth)
            : "%""rax"
        );
        if ((y&(vertLumPerChroma-1)) == vertLumPerChroma-1) {
            usrc += chromStride;
            vsrc += chromStride;
        }
        ysrc += lumStride;
        dst += dstStride;
    }
    __asm__("emms""       \n\t"
            " # nop""     \n\t"
            :::"memory");
}

static inline void yv12toyuy2_mmx(const uint8_t *ysrc, const uint8_t *usrc, const uint8_t *vsrc, uint8_t *dst,
                                      int width, int height,
                                      int lumStride, int chromStride, int dstStride)
{
    yuvPlanartoyuy2_mmx(ysrc, usrc, vsrc, dst, width, height, lumStride, chromStride, dstStride, 2);
}

static inline void yuvPlanartouyvy_mmx(const uint8_t *ysrc, const uint8_t *usrc, const uint8_t *vsrc, uint8_t *dst,
                                           int width, int height,
                                           int lumStride, int chromStride, int dstStride, int vertLumPerChroma)
{
    int y;
    const x86_reg chromWidth= width>>1;
    for (y=0; y<height; y++) {
        __asm__ volatile(
            "xor             %%""rax"", %%""rax"" \n\t"
            ".p2align                   4               \n\t"
            "1:                                         \n\t"
            " # nop"" 32(%1, %%""rax"", 2)           \n\t"
            " # nop"" 32(%2, %%""rax"")              \n\t"
            " # nop"" 32(%3, %%""rax"")              \n\t"
            "movq      (%2, %%""rax""), %%mm0        \n\t"
            "movq                   %%mm0, %%mm2        \n\t"
            "movq      (%3, %%""rax""), %%mm1        \n\t"
            "punpcklbw              %%mm1, %%mm0        \n\t"
            "punpckhbw              %%mm1, %%mm2        \n\t"
            "movq    (%1, %%""rax"",2), %%mm3        \n\t"
            "movq   8(%1, %%""rax"",2), %%mm5        \n\t"
            "movq                   %%mm0, %%mm4        \n\t"
            "movq                   %%mm2, %%mm6        \n\t"
            "punpcklbw              %%mm3, %%mm0        \n\t"
            "punpckhbw              %%mm3, %%mm4        \n\t"
            "punpcklbw              %%mm5, %%mm2        \n\t"
            "punpckhbw              %%mm5, %%mm6        \n\t"
            "movq""                 %%mm0,   (%0, %%""rax"", 4)     \n\t"
            "movq""                 %%mm4,  8(%0, %%""rax"", 4)     \n\t"
            "movq""                 %%mm2, 16(%0, %%""rax"", 4)     \n\t"
            "movq""                 %%mm6, 24(%0, %%""rax"", 4)     \n\t"
            "add                       $8, %%""rax"" \n\t"
            "cmp                       %4, %%""rax"" \n\t"
            " jb                       1b               \n\t"
            ::"r"(dst), "r"(ysrc), "r"(usrc), "r"(vsrc), "g" (chromWidth)
            : "%""rax"
        );
        if ((y&(vertLumPerChroma-1)) == vertLumPerChroma-1) {
            usrc += chromStride;
            vsrc += chromStride;
        }
        ysrc += lumStride;
        dst += dstStride;
    }
    __asm__("emms""       \n\t"
            " # nop""     \n\t"
            :::"memory");
}

static inline void yv12touyvy_mmx(const uint8_t *ysrc, const uint8_t *usrc, const uint8_t *vsrc, uint8_t *dst,
                                      int width, int height,
                                      int lumStride, int chromStride, int dstStride)
{
    yuvPlanartouyvy_mmx(ysrc, usrc, vsrc, dst, width, height, lumStride, chromStride, dstStride, 2);
}

static inline void yuv422ptouyvy_mmx(const uint8_t *ysrc, const uint8_t *usrc, const uint8_t *vsrc, uint8_t *dst,
                                         int width, int height,
                                         int lumStride, int chromStride, int dstStride)
{
    yuvPlanartouyvy_mmx(ysrc, usrc, vsrc, dst, width, height, lumStride, chromStride, dstStride, 1);
}

static inline void yuv422ptoyuy2_mmx(const uint8_t *ysrc, const uint8_t *usrc, const uint8_t *vsrc, uint8_t *dst,
                                         int width, int height,
                                         int lumStride, int chromStride, int dstStride)
{
    yuvPlanartoyuy2_mmx(ysrc, usrc, vsrc, dst, width, height, lumStride, chromStride, dstStride, 1);
}

static inline void yuy2toyv12_mmx(const uint8_t *src, uint8_t *ydst, uint8_t *udst, uint8_t *vdst,
                                      int width, int height,
                                      int lumStride, int chromStride, int srcStride)
{
    int y;
    const x86_reg chromWidth= width>>1;
    for (y=0; y<height; y+=2) {
        __asm__ volatile(
            "xor              %%""rax"", %%""rax""\n\t"
            "pcmpeqw                 %%mm7, %%mm7       \n\t"
            "psrlw                      $8, %%mm7       \n\t"
            ".p2align                    4              \n\t"
            "1:                \n\t"
            " # nop"" 64(%0, %%""rax"", 4)           \n\t"
            "movq    (%0, %%""rax"", 4), %%mm0       \n\t"
            "movq   8(%0, %%""rax"", 4), %%mm1       \n\t"
            "movq                    %%mm0, %%mm2       \n\t"
            "movq                    %%mm1, %%mm3       \n\t"
            "psrlw                      $8, %%mm0       \n\t"
            "psrlw                      $8, %%mm1       \n\t"
            "pand                    %%mm7, %%mm2       \n\t"
            "pand                    %%mm7, %%mm3       \n\t"
            "packuswb                %%mm1, %%mm0       \n\t"
            "packuswb                %%mm3, %%mm2       \n\t"
            "movq""                  %%mm2, (%1, %%""rax"", 2) \n\t"
            "movq  16(%0, %%""rax"", 4), %%mm1       \n\t"
            "movq  24(%0, %%""rax"", 4), %%mm2       \n\t"
            "movq                    %%mm1, %%mm3       \n\t"
            "movq                    %%mm2, %%mm4       \n\t"
            "psrlw                      $8, %%mm1       \n\t"
            "psrlw                      $8, %%mm2       \n\t"
            "pand                    %%mm7, %%mm3       \n\t"
            "pand                    %%mm7, %%mm4       \n\t"
            "packuswb                %%mm2, %%mm1       \n\t"
            "packuswb                %%mm4, %%mm3       \n\t"
            "movq""                  %%mm3, 8(%1, %%""rax"", 2) \n\t"
            "movq                    %%mm0, %%mm2       \n\t"
            "movq                    %%mm1, %%mm3       \n\t"
            "psrlw                      $8, %%mm0       \n\t"
            "psrlw                      $8, %%mm1       \n\t"
            "pand                    %%mm7, %%mm2       \n\t"
            "pand                    %%mm7, %%mm3       \n\t"
            "packuswb                %%mm1, %%mm0       \n\t"
            "packuswb                %%mm3, %%mm2       \n\t"
            "movq""                  %%mm0, (%3, %%""rax"")     \n\t"
            "movq""                  %%mm2, (%2, %%""rax"")     \n\t"
            "add                        $8, %%""rax"" \n\t"
            "cmp                        %4, %%""rax"" \n\t"
            " jb                        1b               \n\t"
            ::"r"(src), "r"(ydst), "r"(udst), "r"(vdst), "g" (chromWidth)
            : "memory", "%""rax"
        );
        ydst += lumStride;
        src += srcStride;
        __asm__ volatile(
            "xor              %%""rax"", %%""rax""\n\t"
            ".p2align                    4              \n\t"
            "1:                                         \n\t"
            " # nop"" 64(%0, %%""rax"", 4)           \n\t"
            "movq    (%0, %%""rax"", 4), %%mm0       \n\t"
            "movq   8(%0, %%""rax"", 4), %%mm1       \n\t"
            "movq  16(%0, %%""rax"", 4), %%mm2       \n\t"
            "movq  24(%0, %%""rax"", 4), %%mm3       \n\t"
            "pand                    %%mm7, %%mm0       \n\t"
            "pand                    %%mm7, %%mm1       \n\t"
            "pand                    %%mm7, %%mm2       \n\t"
            "pand                    %%mm7, %%mm3       \n\t"
            "packuswb                %%mm1, %%mm0       \n\t"
            "packuswb                %%mm3, %%mm2       \n\t"
            "movq""                  %%mm0,  (%1, %%""rax"", 2) \n\t"
            "movq""                  %%mm2, 8(%1, %%""rax"", 2) \n\t"
            "add                        $8, %%""rax""\n\t"
            "cmp                        %4, %%""rax""\n\t"
            " jb                        1b              \n\t"
            ::"r"(src), "r"(ydst), "r"(udst), "r"(vdst), "g" (chromWidth)
            : "memory", "%""rax"
        );
        udst += chromStride;
        vdst += chromStride;
        ydst += lumStride;
        src += srcStride;
    }
    __asm__ volatile("emms""       \n\t"
                     " # nop""     \n\t"
                     :::"memory");
}
static inline void uyvytoyv12_mmx(const uint8_t *src, uint8_t *ydst, uint8_t *udst, uint8_t *vdst,
                                      int width, int height,
                                      int lumStride, int chromStride, int srcStride)
{
    int y;
    const x86_reg chromWidth= width>>1;
    for (y=0; y<height; y+=2) {
        __asm__ volatile(
            "xor          %%""rax"", %%""rax"" \n\t"
            "pcmpeqw             %%mm7, %%mm7   \n\t"
            "psrlw                  $8, %%mm7   \n\t"
            ".p2align                4          \n\t"
            "1:                                 \n\t"
            " # nop"" 64(%0, %%""rax"", 4)          \n\t"
            "movq       (%0, %%""rax"", 4), %%mm0   \n\t"
            "movq      8(%0, %%""rax"", 4), %%mm1   \n\t"
            "movq                %%mm0, %%mm2   \n\t"
            "movq                %%mm1, %%mm3   \n\t"
            "pand                %%mm7, %%mm0   \n\t"
            "pand                %%mm7, %%mm1   \n\t"
            "psrlw                  $8, %%mm2   \n\t"
            "psrlw                  $8, %%mm3   \n\t"
            "packuswb            %%mm1, %%mm0   \n\t"
            "packuswb            %%mm3, %%mm2   \n\t"
            "movq""              %%mm2,  (%1, %%""rax"", 2) \n\t"
            "movq     16(%0, %%""rax"", 4), %%mm1   \n\t"
            "movq     24(%0, %%""rax"", 4), %%mm2   \n\t"
            "movq                %%mm1, %%mm3   \n\t"
            "movq                %%mm2, %%mm4   \n\t"
            "pand                %%mm7, %%mm1   \n\t"
            "pand                %%mm7, %%mm2   \n\t"
            "psrlw                  $8, %%mm3   \n\t"
            "psrlw                  $8, %%mm4   \n\t"
            "packuswb            %%mm2, %%mm1   \n\t"
            "packuswb            %%mm4, %%mm3   \n\t"
            "movq""              %%mm3, 8(%1, %%""rax"", 2) \n\t"
            "movq                %%mm0, %%mm2   \n\t"
            "movq                %%mm1, %%mm3   \n\t"
            "psrlw                  $8, %%mm0   \n\t"
            "psrlw                  $8, %%mm1   \n\t"
            "pand                %%mm7, %%mm2   \n\t"
            "pand                %%mm7, %%mm3   \n\t"
            "packuswb            %%mm1, %%mm0   \n\t"
            "packuswb            %%mm3, %%mm2   \n\t"
            "movq""              %%mm0, (%3, %%""rax"") \n\t"
            "movq""              %%mm2, (%2, %%""rax"") \n\t"
            "add                    $8, %%""rax"" \n\t"
            "cmp                    %4, %%""rax"" \n\t"
            " jb                    1b               \n\t"
            ::"r"(src), "r"(ydst), "r"(udst), "r"(vdst), "g" (chromWidth)
            : "memory", "%""rax"
        );
        ydst += lumStride;
        src += srcStride;
        __asm__ volatile(
            "xor          %%""rax"", %%""rax""  \n\t"
            ".p2align                4                \n\t"
            "1:                                       \n\t"
            " # nop"" 64(%0, %%""rax"", 4)         \n\t"
            "movq       (%0, %%""rax"", 4), %%mm0  \n\t"
            "movq      8(%0, %%""rax"", 4), %%mm1  \n\t"
            "movq     16(%0, %%""rax"", 4), %%mm2  \n\t"
            "movq     24(%0, %%""rax"", 4), %%mm3  \n\t"
            "psrlw                  $8, %%mm0   \n\t"
            "psrlw                  $8, %%mm1   \n\t"
            "psrlw                  $8, %%mm2   \n\t"
            "psrlw                  $8, %%mm3   \n\t"
            "packuswb            %%mm1, %%mm0   \n\t"
            "packuswb            %%mm3, %%mm2   \n\t"
            "movq""              %%mm0,  (%1, %%""rax"", 2) \n\t"
            "movq""              %%mm2, 8(%1, %%""rax"", 2) \n\t"
            "add                    $8, %%""rax"" \n\t"
            "cmp                    %4, %%""rax"" \n\t"
            " jb                    1b               \n\t"
            ::"r"(src), "r"(ydst), "r"(udst), "r"(vdst), "g" (chromWidth)
            : "memory", "%""rax"
        );
        udst += chromStride;
        vdst += chromStride;
        ydst += lumStride;
        src += srcStride;
    }
    __asm__ volatile("emms""       \n\t"
                     " # nop""     \n\t"
                     :::"memory");
}
static inline void rgb24toyv12_mmx(const uint8_t *src, uint8_t *ydst, uint8_t *udst, uint8_t *vdst,
                                       int width, int height,
                                       int lumStride, int chromStride, int srcStride,
                                       int32_t *rgb2yuv)
{
#define BGR2Y_IDX "16*4+16*32"
#define BGR2U_IDX "16*4+16*33"
#define BGR2V_IDX "16*4+16*34"
    int y;
    const x86_reg chromWidth= width>>1;
    if (height > 2) {
        ff_rgb24toyv12_c(src, ydst, udst, vdst, width, 2, lumStride, chromStride, srcStride, rgb2yuv);
        src += 2*srcStride;
        ydst += 2*lumStride;
        udst += chromStride;
        vdst += chromStride;
        height -= 2;
    }
    for (y=0; y<height-2; y+=2) {
        int i;
        for (i=0; i<2; i++) {
            __asm__ volatile(
                "mov                        %2, %%""rax""\n\t"
                "movq          ""16*4+16*32""(%3), %%mm6       \n\t"
                "movq       """ "ff_w1111"", %%mm5       \n\t"
                "pxor                    %%mm7, %%mm7       \n\t"
                "lea (%%""rax"", %%""rax"", 2), %%""rdx"" \n\t"
                ".p2align                    4              \n\t"
                "1:                                         \n\t"
                " # nop"" 64(%0, %%""rdx"")              \n\t"
                "movd       (%0, %%""rdx""), %%mm0       \n\t"
                "movd      3(%0, %%""rdx""), %%mm1       \n\t"
                "punpcklbw               %%mm7, %%mm0       \n\t"
                "punpcklbw               %%mm7, %%mm1       \n\t"
                "movd      6(%0, %%""rdx""), %%mm2       \n\t"
                "movd      9(%0, %%""rdx""), %%mm3       \n\t"
                "punpcklbw               %%mm7, %%mm2       \n\t"
                "punpcklbw               %%mm7, %%mm3       \n\t"
                "pmaddwd                 %%mm6, %%mm0       \n\t"
                "pmaddwd                 %%mm6, %%mm1       \n\t"
                "pmaddwd                 %%mm6, %%mm2       \n\t"
                "pmaddwd                 %%mm6, %%mm3       \n\t"
                "psrad                      $8, %%mm0       \n\t"
                "psrad                      $8, %%mm1       \n\t"
                "psrad                      $8, %%mm2       \n\t"
                "psrad                      $8, %%mm3       \n\t"
                "packssdw                %%mm1, %%mm0       \n\t"
                "packssdw                %%mm3, %%mm2       \n\t"
                "pmaddwd                 %%mm5, %%mm0       \n\t"
                "pmaddwd                 %%mm5, %%mm2       \n\t"
                "packssdw                %%mm2, %%mm0       \n\t"
                "psraw                      $7, %%mm0       \n\t"
                "movd     12(%0, %%""rdx""), %%mm4       \n\t"
                "movd     15(%0, %%""rdx""), %%mm1       \n\t"
                "punpcklbw               %%mm7, %%mm4       \n\t"
                "punpcklbw               %%mm7, %%mm1       \n\t"
                "movd     18(%0, %%""rdx""), %%mm2       \n\t"
                "movd     21(%0, %%""rdx""), %%mm3       \n\t"
                "punpcklbw               %%mm7, %%mm2       \n\t"
                "punpcklbw               %%mm7, %%mm3       \n\t"
                "pmaddwd                 %%mm6, %%mm4       \n\t"
                "pmaddwd                 %%mm6, %%mm1       \n\t"
                "pmaddwd                 %%mm6, %%mm2       \n\t"
                "pmaddwd                 %%mm6, %%mm3       \n\t"
                "psrad                      $8, %%mm4       \n\t"
                "psrad                      $8, %%mm1       \n\t"
                "psrad                      $8, %%mm2       \n\t"
                "psrad                      $8, %%mm3       \n\t"
                "packssdw                %%mm1, %%mm4       \n\t"
                "packssdw                %%mm3, %%mm2       \n\t"
                "pmaddwd                 %%mm5, %%mm4       \n\t"
                "pmaddwd                 %%mm5, %%mm2       \n\t"
                "add                       $24, %%""rdx""\n\t"
                "packssdw                %%mm2, %%mm4       \n\t"
                "psraw                      $7, %%mm4       \n\t"
                "packuswb                %%mm4, %%mm0       \n\t"
                "paddusb """ "ff_bgr2YOffset"", %%mm0    \n\t"
                "movq""                  %%mm0, (%1, %%""rax"") \n\t"
                "add                        $8,      %%""rax""  \n\t"
                " js                        1b                     \n\t"
                : : "r" (src+width*3), "r" (ydst+width), "g" ((x86_reg)-width), "r"(rgb2yuv)
                 
                : "%""rax", "%""rdx"
            );
            ydst += lumStride;
            src += srcStride;
        }
        src -= srcStride*2;
        __asm__ volatile(
            "mov                        %4, %%""rax""\n\t"
            "movq       """ "ff_w1111"", %%mm5       \n\t"
            "movq          ""16*4+16*33""(%5), %%mm6       \n\t"
            "pxor                    %%mm7, %%mm7       \n\t"
            "lea (%%""rax"", %%""rax"", 2), %%""rdx"" \n\t"
            "add              %%""rdx"", %%""rdx""\n\t"
            ".p2align                    4              \n\t"
            "1:                                         \n\t"
            " # nop"" 64(%0, %%""rdx"")              \n\t"
            " # nop"" 64(%1, %%""rdx"")              \n\t"
            "movd       (%0, %%""rdx""), %%mm0       \n\t"
            "movd       (%1, %%""rdx""), %%mm1       \n\t"
            "movd      3(%0, %%""rdx""), %%mm2       \n\t"
            "movd      3(%1, %%""rdx""), %%mm3       \n\t"
            "punpcklbw               %%mm7, %%mm0       \n\t"
            "punpcklbw               %%mm7, %%mm1       \n\t"
            "punpcklbw               %%mm7, %%mm2       \n\t"
            "punpcklbw               %%mm7, %%mm3       \n\t"
            "paddw                   %%mm1, %%mm0       \n\t"
            "paddw                   %%mm3, %%mm2       \n\t"
            "paddw                   %%mm2, %%mm0       \n\t"
            "movd      6(%0, %%""rdx""), %%mm4       \n\t"
            "movd      6(%1, %%""rdx""), %%mm1       \n\t"
            "movd      9(%0, %%""rdx""), %%mm2       \n\t"
            "movd      9(%1, %%""rdx""), %%mm3       \n\t"
            "punpcklbw               %%mm7, %%mm4       \n\t"
            "punpcklbw               %%mm7, %%mm1       \n\t"
            "punpcklbw               %%mm7, %%mm2       \n\t"
            "punpcklbw               %%mm7, %%mm3       \n\t"
            "paddw                   %%mm1, %%mm4       \n\t"
            "paddw                   %%mm3, %%mm2       \n\t"
            "paddw                   %%mm4, %%mm2       \n\t"
            "psrlw                      $2, %%mm0       \n\t"
            "psrlw                      $2, %%mm2       \n\t"
            "movq          ""16*4+16*34""(%5), %%mm1       \n\t"
            "movq          ""16*4+16*34""(%5), %%mm3       \n\t"
            "pmaddwd                 %%mm0, %%mm1       \n\t"
            "pmaddwd                 %%mm2, %%mm3       \n\t"
            "pmaddwd                 %%mm6, %%mm0       \n\t"
            "pmaddwd                 %%mm6, %%mm2       \n\t"
            "psrad                      $8, %%mm0       \n\t"
            "psrad                      $8, %%mm1       \n\t"
            "psrad                      $8, %%mm2       \n\t"
            "psrad                      $8, %%mm3       \n\t"
            "packssdw                %%mm2, %%mm0       \n\t"
            "packssdw                %%mm3, %%mm1       \n\t"
            "pmaddwd                 %%mm5, %%mm0       \n\t"
            "pmaddwd                 %%mm5, %%mm1       \n\t"
            "packssdw                %%mm1, %%mm0       \n\t"
            "psraw                      $7, %%mm0       \n\t"
            "movd     12(%0, %%""rdx""), %%mm4       \n\t"
            "movd     12(%1, %%""rdx""), %%mm1       \n\t"
            "movd     15(%0, %%""rdx""), %%mm2       \n\t"
            "movd     15(%1, %%""rdx""), %%mm3       \n\t"
            "punpcklbw               %%mm7, %%mm4       \n\t"
            "punpcklbw               %%mm7, %%mm1       \n\t"
            "punpcklbw               %%mm7, %%mm2       \n\t"
            "punpcklbw               %%mm7, %%mm3       \n\t"
            "paddw                   %%mm1, %%mm4       \n\t"
            "paddw                   %%mm3, %%mm2       \n\t"
            "paddw                   %%mm2, %%mm4       \n\t"
            "movd     18(%0, %%""rdx""), %%mm5       \n\t"
            "movd     18(%1, %%""rdx""), %%mm1       \n\t"
            "movd     21(%0, %%""rdx""), %%mm2       \n\t"
            "movd     21(%1, %%""rdx""), %%mm3       \n\t"
            "punpcklbw               %%mm7, %%mm5       \n\t"
            "punpcklbw               %%mm7, %%mm1       \n\t"
            "punpcklbw               %%mm7, %%mm2       \n\t"
            "punpcklbw               %%mm7, %%mm3       \n\t"
            "paddw                   %%mm1, %%mm5       \n\t"
            "paddw                   %%mm3, %%mm2       \n\t"
            "paddw                   %%mm5, %%mm2       \n\t"
            "movq       """ "ff_w1111"", %%mm5       \n\t"
            "psrlw                      $2, %%mm4       \n\t"
            "psrlw                      $2, %%mm2       \n\t"
            "movq          ""16*4+16*34""(%5), %%mm1       \n\t"
            "movq          ""16*4+16*34""(%5), %%mm3       \n\t"
            "pmaddwd                 %%mm4, %%mm1       \n\t"
            "pmaddwd                 %%mm2, %%mm3       \n\t"
            "pmaddwd                 %%mm6, %%mm4       \n\t"
            "pmaddwd                 %%mm6, %%mm2       \n\t"
            "psrad                      $8, %%mm4       \n\t"
            "psrad                      $8, %%mm1       \n\t"
            "psrad                      $8, %%mm2       \n\t"
            "psrad                      $8, %%mm3       \n\t"
            "packssdw                %%mm2, %%mm4       \n\t"
            "packssdw                %%mm3, %%mm1       \n\t"
            "pmaddwd                 %%mm5, %%mm4       \n\t"
            "pmaddwd                 %%mm5, %%mm1       \n\t"
            "add                       $24, %%""rdx""\n\t"
            "packssdw                %%mm1, %%mm4       \n\t"
            "psraw                      $7, %%mm4       \n\t"
            "movq                    %%mm0, %%mm1           \n\t"
            "punpckldq               %%mm4, %%mm0           \n\t"
            "punpckhdq               %%mm4, %%mm1           \n\t"
            "packsswb                %%mm1, %%mm0           \n\t"
            "paddb """ "ff_bgr2UVOffset"", %%mm0         \n\t"
            "movd                    %%mm0, (%2, %%""rax"") \n\t"
            "punpckhdq               %%mm0, %%mm0              \n\t"
            "movd                    %%mm0, (%3, %%""rax"") \n\t"
            "add                        $4, %%""rax""       \n\t"
            " js                        1b              \n\t"
            : : "r" (src+chromWidth*6), "r" (src+srcStride+chromWidth*6), "r" (udst+chromWidth), "r" (vdst+chromWidth), "g" (-chromWidth), "r"(rgb2yuv)
             
            : "%""rax", "%""rdx"
        );
        udst += chromStride;
        vdst += chromStride;
        src += srcStride*2;
    }
    __asm__ volatile("emms""       \n\t"
                     " # nop""     \n\t"
                     :::"memory");
     ff_rgb24toyv12_c(src, ydst, udst, vdst, width, height-y, lumStride, chromStride, srcStride, rgb2yuv);
}

static void interleaveBytes_mmx(const uint8_t *src1, const uint8_t *src2, uint8_t *dest,
                                    int width, int height, int src1Stride,
                                    int src2Stride, int dstStride)
{
    int h;
    for (h=0; h < height; h++) {
        int w;
        if (width >= 16) {
        __asm__(
            "xor %%""rax"", %%""rax""         \n\t"
            "1:                                     \n\t"
            " # nop"" 64(%1, %%""rax"")          \n\t"
            " # nop"" 64(%2, %%""rax"")          \n\t"
            "movq    (%1, %%""rax""), %%mm0      \n\t"
            "movq   8(%1, %%""rax""), %%mm2      \n\t"
            "movq                 %%mm0, %%mm1      \n\t"
            "movq                 %%mm2, %%mm3      \n\t"
            "movq    (%2, %%""rax""), %%mm4      \n\t"
            "movq   8(%2, %%""rax""), %%mm5      \n\t"
            "punpcklbw            %%mm4, %%mm0      \n\t"
            "punpckhbw            %%mm4, %%mm1      \n\t"
            "punpcklbw            %%mm5, %%mm2      \n\t"
            "punpckhbw            %%mm5, %%mm3      \n\t"
            "movq""               %%mm0,   (%0, %%""rax"", 2) \n\t"
            "movq""               %%mm1,  8(%0, %%""rax"", 2) \n\t"
            "movq""               %%mm2, 16(%0, %%""rax"", 2) \n\t"
            "movq""               %%mm3, 24(%0, %%""rax"", 2) \n\t"
            "add                    $16, %%""rax""            \n\t"
            "cmp                     %3, %%""rax""            \n\t"
            " jb                     1b                          \n\t"
            ::"r"(dest), "r"(src1), "r"(src2), "r" ((x86_reg)width-15)
            : "memory", "%""rax"
        );
        }
        for (w= (width&(~15)); w < width; w++) {
            dest[2*w+0] = src1[w];
            dest[2*w+1] = src2[w];
        }
        dest += dstStride;
        src1 += src1Stride;
        src2 += src2Stride;
    }
    __asm__(
            "emms""       \n\t"
            " # nop""     \n\t"
            ::: "memory"
            );
}
static inline void vu9_to_vu12_mmx(const uint8_t *src1, const uint8_t *src2,
                                       uint8_t *dst1, uint8_t *dst2,
                                       int width, int height,
                                       int srcStride1, int srcStride2,
                                       int dstStride1, int dstStride2)
{
    x86_reg x, y;
    int w,h;
    w=width/2; h=height/2;
    __asm__ volatile(
        " # nop"" %0    \n\t"
        " # nop"" %1    \n\t"
        ::"m"(*(src1+srcStride1)),"m"(*(src2+srcStride2)):"memory");
    for (y=0;y<h;y++) {
        const uint8_t* s1=src1+srcStride1*(y>>1);
        uint8_t* d=dst1+dstStride1*y;
        x=0;
        for (;x<w-31;x+=32) {
            __asm__ volatile(
                " # nop""   32(%1,%2)        \n\t"
                "movq         (%1,%2), %%mm0 \n\t"
                "movq        8(%1,%2), %%mm2 \n\t"
                "movq       16(%1,%2), %%mm4 \n\t"
                "movq       24(%1,%2), %%mm6 \n\t"
                "movq      %%mm0, %%mm1 \n\t"
                "movq      %%mm2, %%mm3 \n\t"
                "movq      %%mm4, %%mm5 \n\t"
                "movq      %%mm6, %%mm7 \n\t"
                "punpcklbw %%mm0, %%mm0 \n\t"
                "punpckhbw %%mm1, %%mm1 \n\t"
                "punpcklbw %%mm2, %%mm2 \n\t"
                "punpckhbw %%mm3, %%mm3 \n\t"
                "punpcklbw %%mm4, %%mm4 \n\t"
                "punpckhbw %%mm5, %%mm5 \n\t"
                "punpcklbw %%mm6, %%mm6 \n\t"
                "punpckhbw %%mm7, %%mm7 \n\t"
                "movq""    %%mm0,   (%0,%2,2)  \n\t"
                "movq""    %%mm1,  8(%0,%2,2)  \n\t"
                "movq""    %%mm2, 16(%0,%2,2)  \n\t"
                "movq""    %%mm3, 24(%0,%2,2)  \n\t"
                "movq""    %%mm4, 32(%0,%2,2)  \n\t"
                "movq""    %%mm5, 40(%0,%2,2)  \n\t"
                "movq""    %%mm6, 48(%0,%2,2)  \n\t"
                "movq""    %%mm7, 56(%0,%2,2)"
                :: "r"(d), "r"(s1), "r"(x)
                :"memory");
        }
        for (;x<w;x++) d[2*x]=d[2*x+1]=s1[x];
    }
    for (y=0;y<h;y++) {
        const uint8_t* s2=src2+srcStride2*(y>>1);
        uint8_t* d=dst2+dstStride2*y;
        x=0;
        for (;x<w-31;x+=32) {
            __asm__ volatile(
                " # nop""   32(%1,%2)        \n\t"
                "movq         (%1,%2), %%mm0 \n\t"
                "movq        8(%1,%2), %%mm2 \n\t"
                "movq       16(%1,%2), %%mm4 \n\t"
                "movq       24(%1,%2), %%mm6 \n\t"
                "movq      %%mm0, %%mm1 \n\t"
                "movq      %%mm2, %%mm3 \n\t"
                "movq      %%mm4, %%mm5 \n\t"
                "movq      %%mm6, %%mm7 \n\t"
                "punpcklbw %%mm0, %%mm0 \n\t"
                "punpckhbw %%mm1, %%mm1 \n\t"
                "punpcklbw %%mm2, %%mm2 \n\t"
                "punpckhbw %%mm3, %%mm3 \n\t"
                "punpcklbw %%mm4, %%mm4 \n\t"
                "punpckhbw %%mm5, %%mm5 \n\t"
                "punpcklbw %%mm6, %%mm6 \n\t"
                "punpckhbw %%mm7, %%mm7 \n\t"
                "movq""    %%mm0,   (%0,%2,2)  \n\t"
                "movq""    %%mm1,  8(%0,%2,2)  \n\t"
                "movq""    %%mm2, 16(%0,%2,2)  \n\t"
                "movq""    %%mm3, 24(%0,%2,2)  \n\t"
                "movq""    %%mm4, 32(%0,%2,2)  \n\t"
                "movq""    %%mm5, 40(%0,%2,2)  \n\t"
                "movq""    %%mm6, 48(%0,%2,2)  \n\t"
                "movq""    %%mm7, 56(%0,%2,2)"
                :: "r"(d), "r"(s2), "r"(x)
                :"memory");
        }
        for (;x<w;x++) d[2*x]=d[2*x+1]=s2[x];
    }
    __asm__(
            "emms""       \n\t"
            " # nop""     \n\t"
            ::: "memory"
        );
}

static inline void yvu9_to_yuy2_mmx(const uint8_t *src1, const uint8_t *src2, const uint8_t *src3,
                                        uint8_t *dst,
                                        int width, int height,
                                        int srcStride1, int srcStride2,
                                        int srcStride3, int dstStride)
{
    x86_reg x;
    int y,w,h;
    w=width/2; h=height;
    for (y=0;y<h;y++) {
        const uint8_t* yp=src1+srcStride1*y;
        const uint8_t* up=src2+srcStride2*(y>>2);
        const uint8_t* vp=src3+srcStride3*(y>>2);
        uint8_t* d=dst+dstStride*y;
        x=0;
        for (;x<w-7;x+=8) {
            __asm__ volatile(
                " # nop""   32(%1, %0)          \n\t"
                " # nop""   32(%2, %0)          \n\t"
                " # nop""   32(%3, %0)          \n\t"
                "movq      (%1, %0, 4), %%mm0   \n\t"
                "movq         (%2, %0), %%mm1   \n\t"
                "movq         (%3, %0), %%mm2   \n\t"
                "movq            %%mm0, %%mm3   \n\t"
                "movq            %%mm1, %%mm4   \n\t"
                "movq            %%mm2, %%mm5   \n\t"
                "punpcklbw       %%mm1, %%mm1   \n\t"
                "punpcklbw       %%mm2, %%mm2   \n\t"
                "punpckhbw       %%mm4, %%mm4   \n\t"
                "punpckhbw       %%mm5, %%mm5   \n\t"
                "movq            %%mm1, %%mm6   \n\t"
                "punpcklbw       %%mm2, %%mm1   \n\t"
                "punpcklbw       %%mm1, %%mm0   \n\t"
                "punpckhbw       %%mm1, %%mm3   \n\t"
                "movq""          %%mm0,  (%4, %0, 8)    \n\t"
                "movq""          %%mm3, 8(%4, %0, 8)    \n\t"
                "punpckhbw       %%mm2, %%mm6   \n\t"
                "movq     8(%1, %0, 4), %%mm0   \n\t"
                "movq            %%mm0, %%mm3   \n\t"
                "punpcklbw       %%mm6, %%mm0   \n\t"
                "punpckhbw       %%mm6, %%mm3   \n\t"
                "movq""          %%mm0, 16(%4, %0, 8)   \n\t"
                "movq""          %%mm3, 24(%4, %0, 8)   \n\t"
                "movq            %%mm4, %%mm6   \n\t"
                "movq    16(%1, %0, 4), %%mm0   \n\t"
                "movq            %%mm0, %%mm3   \n\t"
                "punpcklbw       %%mm5, %%mm4   \n\t"
                "punpcklbw       %%mm4, %%mm0   \n\t"
                "punpckhbw       %%mm4, %%mm3   \n\t"
                "movq""          %%mm0, 32(%4, %0, 8)   \n\t"
                "movq""          %%mm3, 40(%4, %0, 8)   \n\t"
                "punpckhbw       %%mm5, %%mm6   \n\t"
                "movq    24(%1, %0, 4), %%mm0   \n\t"
                "movq            %%mm0, %%mm3   \n\t"
                "punpcklbw       %%mm6, %%mm0   \n\t"
                "punpckhbw       %%mm6, %%mm3   \n\t"
                "movq""          %%mm0, 48(%4, %0, 8)   \n\t"
                "movq""          %%mm3, 56(%4, %0, 8)   \n\t"
                : "+r" (x)
                : "r"(yp), "r" (up), "r"(vp), "r"(d)
                :"memory");
        }
        for (; x<w; x++) {
            const int x2 = x<<2;
            d[8*x+0] = yp[x2];
            d[8*x+1] = up[x];
            d[8*x+2] = yp[x2+1];
            d[8*x+3] = vp[x];
            d[8*x+4] = yp[x2+2];
            d[8*x+5] = up[x];
            d[8*x+6] = yp[x2+3];
            d[8*x+7] = vp[x];
        }
    }
    __asm__(
            "emms""       \n\t"
            " # nop""     \n\t"
            ::: "memory"
        );
}

static void extract_even_mmx(const uint8_t *src, uint8_t *dst, x86_reg count)
{
    dst += count;
    src += 2*count;
    count= - count;
    if(count <= -16) {
        count += 15;
        __asm__ volatile(
            "pcmpeqw       %%mm7, %%mm7        \n\t"
            "psrlw            $8, %%mm7        \n\t"
            "1:                                \n\t"
            "movq -30(%1, %0, 2), %%mm0        \n\t"
            "movq -22(%1, %0, 2), %%mm1        \n\t"
            "movq -14(%1, %0, 2), %%mm2        \n\t"
            "movq  -6(%1, %0, 2), %%mm3        \n\t"
            "pand          %%mm7, %%mm0        \n\t"
            "pand          %%mm7, %%mm1        \n\t"
            "pand          %%mm7, %%mm2        \n\t"
            "pand          %%mm7, %%mm3        \n\t"
            "packuswb      %%mm1, %%mm0        \n\t"
            "packuswb      %%mm3, %%mm2        \n\t"
            "movq""        %%mm0,-15(%2, %0)   \n\t"
            "movq""        %%mm2,- 7(%2, %0)   \n\t"
            "add             $16, %0           \n\t"
            " js 1b                            \n\t"
            : "+r"(count)
            : "r"(src), "r"(dst)
        );
        count -= 15;
    }
    while(count<0) {
        dst[count]= src[2*count];
        count++;
    }
}

static void extract_odd_mmx(const uint8_t *src, uint8_t *dst, x86_reg count)
{
    src ++;
    dst += count;
    src += 2*count;
    count= - count;
    if(count < -16) {
        count += 16;
        __asm__ volatile(
            "pcmpeqw       %%mm7, %%mm7        \n\t"
            "psrlw            $8, %%mm7        \n\t"
            "1:                                \n\t"
            "movq -32(%1, %0, 2), %%mm0        \n\t"
            "movq -24(%1, %0, 2), %%mm1        \n\t"
            "movq -16(%1, %0, 2), %%mm2        \n\t"
            "movq  -8(%1, %0, 2), %%mm3        \n\t"
            "pand          %%mm7, %%mm0        \n\t"
            "pand          %%mm7, %%mm1        \n\t"
            "pand          %%mm7, %%mm2        \n\t"
            "pand          %%mm7, %%mm3        \n\t"
            "packuswb      %%mm1, %%mm0        \n\t"
            "packuswb      %%mm3, %%mm2        \n\t"
            "movq""        %%mm0,-16(%2, %0)   \n\t"
            "movq""        %%mm2,- 8(%2, %0)   \n\t"
            "add             $16, %0           \n\t"
            " js 1b                            \n\t"
            : "+r"(count)
            : "r"(src), "r"(dst)
        );
        count -= 16;
    }
    while(count<0) {
        dst[count]= src[2*count];
        count++;
    }
}

static void extract_even2_mmx(const uint8_t *src, uint8_t *dst0, uint8_t *dst1, x86_reg count)
{
    dst0+= count;
    dst1+= count;
    src += 4*count;
    count= - count;
    if(count <= -8) {
        count += 7;
        __asm__ volatile(
            "pcmpeqw       %%mm7, %%mm7        \n\t"
            "psrlw            $8, %%mm7        \n\t"
            "1:                                \n\t"
            "movq -28(%1, %0, 4), %%mm0        \n\t"
            "movq -20(%1, %0, 4), %%mm1        \n\t"
            "movq -12(%1, %0, 4), %%mm2        \n\t"
            "movq  -4(%1, %0, 4), %%mm3        \n\t"
            "pand          %%mm7, %%mm0        \n\t"
            "pand          %%mm7, %%mm1        \n\t"
            "pand          %%mm7, %%mm2        \n\t"
            "pand          %%mm7, %%mm3        \n\t"
            "packuswb      %%mm1, %%mm0        \n\t"
            "packuswb      %%mm3, %%mm2        \n\t"
            "movq          %%mm0, %%mm1        \n\t"
            "movq          %%mm2, %%mm3        \n\t"
            "psrlw            $8, %%mm0        \n\t"
            "psrlw            $8, %%mm2        \n\t"
            "pand          %%mm7, %%mm1        \n\t"
            "pand          %%mm7, %%mm3        \n\t"
            "packuswb      %%mm2, %%mm0        \n\t"
            "packuswb      %%mm3, %%mm1        \n\t"
            "movq""        %%mm0,- 7(%3, %0)   \n\t"
            "movq""        %%mm1,- 7(%2, %0)   \n\t"
            "add              $8, %0           \n\t"
            " js 1b                            \n\t"
            : "+r"(count)
            : "r"(src), "r"(dst0), "r"(dst1)
        );
        count -= 7;
    }
    while(count<0) {
        dst0[count]= src[4*count+0];
        dst1[count]= src[4*count+2];
        count++;
    }
}

static void extract_even2avg_mmx(const uint8_t *src0, const uint8_t *src1, uint8_t *dst0, uint8_t *dst1, x86_reg count)
{
    dst0 += count;
    dst1 += count;
    src0 += 4*count;
    src1 += 4*count;
    count= - count;
    while(count<0) {
        dst0[count]= (src0[4*count+0]+src1[4*count+0])>>1;
        dst1[count]= (src0[4*count+2]+src1[4*count+2])>>1;
        count++;
    }
}

static void extract_odd2_mmx(const uint8_t *src, uint8_t *dst0, uint8_t *dst1, x86_reg count)
{
    dst0+= count;
    dst1+= count;
    src += 4*count;
    count= - count;
    if(count <= -8) {
        count += 7;
        __asm__ volatile(
            "pcmpeqw       %%mm7, %%mm7        \n\t"
            "psrlw            $8, %%mm7        \n\t"
            "1:                                \n\t"
            "movq -28(%1, %0, 4), %%mm0        \n\t"
            "movq -20(%1, %0, 4), %%mm1        \n\t"
            "movq -12(%1, %0, 4), %%mm2        \n\t"
            "movq  -4(%1, %0, 4), %%mm3        \n\t"
            "psrlw            $8, %%mm0        \n\t"
            "psrlw            $8, %%mm1        \n\t"
            "psrlw            $8, %%mm2        \n\t"
            "psrlw            $8, %%mm3        \n\t"
            "packuswb      %%mm1, %%mm0        \n\t"
            "packuswb      %%mm3, %%mm2        \n\t"
            "movq          %%mm0, %%mm1        \n\t"
            "movq          %%mm2, %%mm3        \n\t"
            "psrlw            $8, %%mm0        \n\t"
            "psrlw            $8, %%mm2        \n\t"
            "pand          %%mm7, %%mm1        \n\t"
            "pand          %%mm7, %%mm3        \n\t"
            "packuswb      %%mm2, %%mm0        \n\t"
            "packuswb      %%mm3, %%mm1        \n\t"
            "movq""        %%mm0,- 7(%3, %0)   \n\t"
            "movq""        %%mm1,- 7(%2, %0)   \n\t"
            "add              $8, %0           \n\t"
            " js 1b                            \n\t"
            : "+r"(count)
            : "r"(src), "r"(dst0), "r"(dst1)
        );
        count -= 7;
    }
    src++;
    while(count<0) {
        dst0[count]= src[4*count+0];
        dst1[count]= src[4*count+2];
        count++;
    }
}

static void extract_odd2avg_mmx(const uint8_t *src0, const uint8_t *src1, uint8_t *dst0, uint8_t *dst1, x86_reg count)
{
    dst0 += count;
    dst1 += count;
    src0 += 4*count;
    src1 += 4*count;
    count= - count;
    src0++;
    src1++;
    while(count<0) {
        dst0[count]= (src0[4*count+0]+src1[4*count+0])>>1;
        dst1[count]= (src0[4*count+2]+src1[4*count+2])>>1;
        count++;
    }
}

static void yuyvtoyuv420_mmx(uint8_t *ydst, uint8_t *udst, uint8_t *vdst, const uint8_t *src,
                                 int width, int height,
                                 int lumStride, int chromStride, int srcStride)
{
    int y;
    const int chromWidth = (!__builtin_constant_p(1) ? -((-(width)) >> (1)) : ((width) + (1<<(1)) - 1) >> (1));
    for (y=0; y<height; y++) {
        extract_even_mmx(src, ydst, width);
        if(y&1) {
            extract_odd2avg_mmx(src-srcStride, src, udst, vdst, chromWidth);
            udst+= chromStride;
            vdst+= chromStride;
        }
        src += srcStride;
        ydst+= lumStride;
    }
    __asm__(
            "emms""       \n\t"
            " # nop""     \n\t"
            ::: "memory"
        );
}

static void yuyvtoyuv422_mmx(uint8_t *ydst, uint8_t *udst, uint8_t *vdst, const uint8_t *src,
                                 int width, int height,
                                 int lumStride, int chromStride, int srcStride)
{
    int y;
    const int chromWidth = (!__builtin_constant_p(1) ? -((-(width)) >> (1)) : ((width) + (1<<(1)) - 1) >> (1));
    for (y=0; y<height; y++) {
        extract_even_mmx(src, ydst, width);
        extract_odd2_mmx(src, udst, vdst, chromWidth);
        src += srcStride;
        ydst+= lumStride;
        udst+= chromStride;
        vdst+= chromStride;
    }
    __asm__(
            "emms""       \n\t"
            " # nop""     \n\t"
            ::: "memory"
        );
}

static void uyvytoyuv420_mmx(uint8_t *ydst, uint8_t *udst, uint8_t *vdst, const uint8_t *src,
                                 int width, int height,
                                 int lumStride, int chromStride, int srcStride)
{
    int y;
    const int chromWidth = (!__builtin_constant_p(1) ? -((-(width)) >> (1)) : ((width) + (1<<(1)) - 1) >> (1));
    for (y=0; y<height; y++) {
        extract_odd_mmx(src, ydst, width);
        if(y&1) {
            extract_even2avg_mmx(src-srcStride, src, udst, vdst, chromWidth);
            udst+= chromStride;
            vdst+= chromStride;
        }
        src += srcStride;
        ydst+= lumStride;
    }
    __asm__(
            "emms""       \n\t"
            " # nop""     \n\t"
            ::: "memory"
        );
}

static void uyvytoyuv422_mmx(uint8_t *ydst, uint8_t *udst, uint8_t *vdst, const uint8_t *src,
                                 int width, int height,
                                 int lumStride, int chromStride, int srcStride)
{
    int y;
    const int chromWidth = (!__builtin_constant_p(1) ? -((-(width)) >> (1)) : ((width) + (1<<(1)) - 1) >> (1));
    for (y=0; y<height; y++) {
        extract_odd_mmx(src, ydst, width);
        extract_even2_mmx(src, udst, vdst, chromWidth);
        src += srcStride;
        ydst+= lumStride;
        udst+= chromStride;
        vdst+= chromStride;
    }
    __asm__(
            "emms""       \n\t"
            " # nop""     \n\t"
            ::: "memory"
        );
}

static void rgb2rgb_init_mmx(void)
{
    rgb15to16 = rgb15to16_mmx;
    rgb15tobgr24 = rgb15tobgr24_mmx;
    rgb15to32 = rgb15to32_mmx;
    rgb16tobgr24 = rgb16tobgr24_mmx;
    rgb16to32 = rgb16to32_mmx;
    rgb16to15 = rgb16to15_mmx;
    rgb24tobgr16 = rgb24tobgr16_mmx;
    rgb24tobgr15 = rgb24tobgr15_mmx;
    rgb24tobgr32 = rgb24tobgr32_mmx;
    rgb32to16 = rgb32to16_mmx;
    rgb32to15 = rgb32to15_mmx;
    rgb32tobgr24 = rgb32tobgr24_mmx;
    rgb24to15 = rgb24to15_mmx;
    rgb24to16 = rgb24to16_mmx;
    rgb24tobgr24 = rgb24tobgr24_mmx;
    rgb32tobgr16 = rgb32tobgr16_mmx;
    rgb32tobgr15 = rgb32tobgr15_mmx;
    yv12toyuy2 = yv12toyuy2_mmx;
    yv12touyvy = yv12touyvy_mmx;
    yuv422ptoyuy2 = yuv422ptoyuy2_mmx;
    yuv422ptouyvy = yuv422ptouyvy_mmx;
    yuy2toyv12 = yuy2toyv12_mmx;
    vu9_to_vu12 = vu9_to_vu12_mmx;
    yvu9_to_yuy2 = yvu9_to_yuy2_mmx;
    uyvytoyuv422 = uyvytoyuv422_mmx;
    yuyvtoyuv422 = yuyvtoyuv422_mmx;
    ff_rgb24toyv12 = rgb24toyv12_mmx;
    yuyvtoyuv420 = yuyvtoyuv420_mmx;
    uyvytoyuv420 = uyvytoyuv420_mmx;
    interleaveBytes = interleaveBytes_mmx;
}

static inline void rgb24tobgr32_mmxext(const uint8_t *src, uint8_t *dst, int src_size)
{
    uint8_t *dest = dst;
    const uint8_t *s = src;
    const uint8_t *end;
    const uint8_t *mm_end;
    end = s + src_size;
    __asm__ volatile("prefetchnta""    %0"::"m"(*s):"memory");
    mm_end = end - 23;
    __asm__ volatile("movq        %0, %%mm7"::"m"(mask32a):"memory");
    while (s < mm_end) {
        __asm__ volatile(
            "prefetchnta""  32(%1)           \n\t"
            "movd        (%1), %%mm0    \n\t"
            "punpckldq  3(%1), %%mm0    \n\t"
            "movd       6(%1), %%mm1    \n\t"
            "punpckldq  9(%1), %%mm1    \n\t"
            "movd      12(%1), %%mm2    \n\t"
            "punpckldq 15(%1), %%mm2    \n\t"
            "movd      18(%1), %%mm3    \n\t"
            "punpckldq 21(%1), %%mm3    \n\t"
            "por        %%mm7, %%mm0    \n\t"
            "por        %%mm7, %%mm1    \n\t"
            "por        %%mm7, %%mm2    \n\t"
            "por        %%mm7, %%mm3    \n\t"
            "movntq""     %%mm0,   (%0)   \n\t"
            "movntq""     %%mm1,  8(%0)   \n\t"
            "movntq""     %%mm2, 16(%0)   \n\t"
            "movntq""     %%mm3, 24(%0)"
            :: "r"(dest), "r"(s)
            :"memory");
        dest += 32;
        s += 24;
    }
    __asm__ volatile("sfence":::"memory");
    __asm__ volatile("emms":::"memory");
    while (s < end) {
        *dest++ = *s++;
        *dest++ = *s++;
        *dest++ = *s++;
        *dest++ = 255;
    }
}
#define STORE_BGR24_MMX "psrlq         $8, %%mm2    \n\t" "psrlq         $8, %%mm3    \n\t" "psrlq         $8, %%mm6    \n\t" "psrlq         $8, %%mm7    \n\t" "pand "MANGLE(mask24l)", %%mm0\n\t" "pand "MANGLE(mask24l)", %%mm1\n\t" "pand "MANGLE(mask24l)", %%mm4\n\t" "pand "MANGLE(mask24l)", %%mm5\n\t" "pand "MANGLE(mask24h)", %%mm2\n\t" "pand "MANGLE(mask24h)", %%mm3\n\t" "pand "MANGLE(mask24h)", %%mm6\n\t" "pand "MANGLE(mask24h)", %%mm7\n\t" "por        %%mm2, %%mm0    \n\t" "por        %%mm3, %%mm1    \n\t" "por        %%mm6, %%mm4    \n\t" "por        %%mm7, %%mm5    \n\t" "movq       %%mm1, %%mm2    \n\t" "movq       %%mm4, %%mm3    \n\t" "psllq        $48, %%mm2    \n\t" "psllq        $32, %%mm3    \n\t" "por        %%mm2, %%mm0    \n\t" "psrlq        $16, %%mm1    \n\t" "psrlq        $32, %%mm4    \n\t" "psllq        $16, %%mm5    \n\t" "por        %%mm3, %%mm1    \n\t" "por        %%mm5, %%mm4    \n\t" MOVNTQ"     %%mm0,   (%0)    \n\t" MOVNTQ"     %%mm1,  8(%0)    \n\t" MOVNTQ"     %%mm4, 16(%0)"
static inline void rgb32tobgr24_mmxext(const uint8_t *src, uint8_t *dst, int src_size)
{
    uint8_t *dest = dst;
    const uint8_t *s = src;
    const uint8_t *end;
    const uint8_t *mm_end;
    end = s + src_size;
    __asm__ volatile("prefetchnta""    %0"::"m"(*s):"memory");
    mm_end = end - 31;
    while (s < mm_end) {
        __asm__ volatile(
            "prefetchnta""  32(%1)           \n\t"
            "movq        (%1), %%mm0    \n\t"
            "movq       8(%1), %%mm1    \n\t"
            "movq      16(%1), %%mm4    \n\t"
            "movq      24(%1), %%mm5    \n\t"
            "movq       %%mm0, %%mm2    \n\t"
            "movq       %%mm1, %%mm3    \n\t"
            "movq       %%mm4, %%mm6    \n\t"
            "movq       %%mm5, %%mm7    \n\t"
            "psrlq         $8, %%mm2    \n\t" "psrlq         $8, %%mm3    \n\t" "psrlq         $8, %%mm6    \n\t" "psrlq         $8, %%mm7    \n\t" "pand """ "mask24l"", %%mm0\n\t" "pand """ "mask24l"", %%mm1\n\t" "pand """ "mask24l"", %%mm4\n\t" "pand """ "mask24l"", %%mm5\n\t" "pand """ "mask24h"", %%mm2\n\t" "pand """ "mask24h"", %%mm3\n\t" "pand """ "mask24h"", %%mm6\n\t" "pand """ "mask24h"", %%mm7\n\t" "por        %%mm2, %%mm0    \n\t" "por        %%mm3, %%mm1    \n\t" "por        %%mm6, %%mm4    \n\t" "por        %%mm7, %%mm5    \n\t" "movq       %%mm1, %%mm2    \n\t" "movq       %%mm4, %%mm3    \n\t" "psllq        $48, %%mm2    \n\t" "psllq        $32, %%mm3    \n\t" "por        %%mm2, %%mm0    \n\t" "psrlq        $16, %%mm1    \n\t" "psrlq        $32, %%mm4    \n\t" "psllq        $16, %%mm5    \n\t" "por        %%mm3, %%mm1    \n\t" "por        %%mm5, %%mm4    \n\t" "movntq""     %%mm0,   (%0)    \n\t" "movntq""     %%mm1,  8(%0)    \n\t" "movntq""     %%mm4, 16(%0)"
            :: "r"(dest), "r"(s)
             
            :"memory");
        dest += 24;
        s += 32;
    }
    __asm__ volatile("sfence":::"memory");
    __asm__ volatile("emms":::"memory");
    while (s < end) {
        *dest++ = *s++;
        *dest++ = *s++;
        *dest++ = *s++;
        s++;
    }
}

static inline void rgb15to16_mmxext(const uint8_t *src, uint8_t *dst, int src_size)
{
    register const uint8_t* s=src;
    register uint8_t* d=dst;
    register const uint8_t *end;
    const uint8_t *mm_end;
    end = s + src_size;
    __asm__ volatile("prefetchnta""    %0"::"m"(*s));
    __asm__ volatile("movq        %0, %%mm4"::"m"(mask15s));
    mm_end = end - 15;
    while (s<mm_end) {
        __asm__ volatile(
            "prefetchnta"" 32(%1)        \n\t"
            "movq      (%1), %%mm0  \n\t"
            "movq     8(%1), %%mm2  \n\t"
            "movq     %%mm0, %%mm1  \n\t"
            "movq     %%mm2, %%mm3  \n\t"
            "pand     %%mm4, %%mm0  \n\t"
            "pand     %%mm4, %%mm2  \n\t"
            "paddw    %%mm1, %%mm0  \n\t"
            "paddw    %%mm3, %%mm2  \n\t"
            "movntq""   %%mm0,  (%0)  \n\t"
            "movntq""   %%mm2, 8(%0)"
            :: "r"(d), "r"(s)
        );
        d+=16;
        s+=16;
    }
    __asm__ volatile("sfence":::"memory");
    __asm__ volatile("emms":::"memory");
    mm_end = end - 3;
    while (s < mm_end) {
        register unsigned x= *((const uint32_t *)s);
        *((uint32_t *)d) = (x&0x7FFF7FFF) + (x&0x7FE07FE0);
        d+=4;
        s+=4;
    }
    if (s < end) {
        register unsigned short x= *((const uint16_t *)s);
        *((uint16_t *)d) = (x&0x7FFF) + (x&0x7FE0);
    }
}

static inline void rgb16to15_mmxext(const uint8_t *src, uint8_t *dst, int src_size)
{
    register const uint8_t* s=src;
    register uint8_t* d=dst;
    register const uint8_t *end;
    const uint8_t *mm_end;
    end = s + src_size;
    __asm__ volatile("prefetchnta""    %0"::"m"(*s));
    __asm__ volatile("movq        %0, %%mm7"::"m"(mask15rg));
    __asm__ volatile("movq        %0, %%mm6"::"m"(mask15b));
    mm_end = end - 15;
    while (s<mm_end) {
        __asm__ volatile(
            "prefetchnta"" 32(%1)        \n\t"
            "movq      (%1), %%mm0  \n\t"
            "movq     8(%1), %%mm2  \n\t"
            "movq     %%mm0, %%mm1  \n\t"
            "movq     %%mm2, %%mm3  \n\t"
            "psrlq       $1, %%mm0  \n\t"
            "psrlq       $1, %%mm2  \n\t"
            "pand     %%mm7, %%mm0  \n\t"
            "pand     %%mm7, %%mm2  \n\t"
            "pand     %%mm6, %%mm1  \n\t"
            "pand     %%mm6, %%mm3  \n\t"
            "por      %%mm1, %%mm0  \n\t"
            "por      %%mm3, %%mm2  \n\t"
            "movntq""   %%mm0,  (%0)  \n\t"
            "movntq""   %%mm2, 8(%0)"
            :: "r"(d), "r"(s)
        );
        d+=16;
        s+=16;
    }
    __asm__ volatile("sfence":::"memory");
    __asm__ volatile("emms":::"memory");
    mm_end = end - 3;
    while (s < mm_end) {
        register uint32_t x= *((const uint32_t*)s);
        *((uint32_t *)d) = ((x>>1)&0x7FE07FE0) | (x&0x001F001F);
        s+=4;
        d+=4;
    }
    if (s < end) {
        register uint16_t x= *((const uint16_t*)s);
        *((uint16_t *)d) = ((x>>1)&0x7FE0) | (x&0x001F);
    }
}

static inline void rgb32to16_mmxext(const uint8_t *src, uint8_t *dst, int src_size)
{
    const uint8_t *s = src;
    const uint8_t *end;
    const uint8_t *mm_end;
    uint16_t *d = (uint16_t *)dst;
    end = s + src_size;
    mm_end = end - 15;
    __asm__ volatile(
        "movq           %3, %%mm5   \n\t"
        "movq           %4, %%mm6   \n\t"
        "movq           %5, %%mm7   \n\t"
        "jmp 2f                     \n\t"
        ".p2align        4          \n\t"
        "1:                         \n\t"
        "prefetchnta""   32(%1)          \n\t"
        "movd         (%1), %%mm0   \n\t"
        "movd        4(%1), %%mm3   \n\t"
        "punpckldq   8(%1), %%mm0   \n\t"
        "punpckldq  12(%1), %%mm3   \n\t"
        "movq        %%mm0, %%mm1   \n\t"
        "movq        %%mm3, %%mm4   \n\t"
        "pand        %%mm6, %%mm0   \n\t"
        "pand        %%mm6, %%mm3   \n\t"
        "pmaddwd     %%mm7, %%mm0   \n\t"
        "pmaddwd     %%mm7, %%mm3   \n\t"
        "pand        %%mm5, %%mm1   \n\t"
        "pand        %%mm5, %%mm4   \n\t"
        "por         %%mm1, %%mm0   \n\t"
        "por         %%mm4, %%mm3   \n\t"
        "psrld          $5, %%mm0   \n\t"
        "pslld         $11, %%mm3   \n\t"
        "por         %%mm3, %%mm0   \n\t"
        "movntq""      %%mm0, (%0)    \n\t"
        "add           $16,  %1     \n\t"
        "add            $8,  %0     \n\t"
        "2:                         \n\t"
        "cmp            %2,  %1     \n\t"
        " jb            1b          \n\t"
        : "+r" (d), "+r"(s)
        : "r" (mm_end), "m" (mask3216g), "m" (mask3216br), "m" (mul3216)
    );
    __asm__ volatile("sfence":::"memory");
    __asm__ volatile("emms":::"memory");
    while (s < end) {
        register int rgb = *(const uint32_t*)s; s += 4;
        *d++ = ((rgb&0xFF)>>3) + ((rgb&0xFC00)>>5) + ((rgb&0xF80000)>>8);
    }
}

static inline void rgb32tobgr16_mmxext(const uint8_t *src, uint8_t *dst, int src_size)
{
    const uint8_t *s = src;
    const uint8_t *end;
    const uint8_t *mm_end;
    uint16_t *d = (uint16_t *)dst;
    end = s + src_size;
    __asm__ volatile("prefetchnta""    %0"::"m"(*src):"memory");
    __asm__ volatile(
        "movq          %0, %%mm7    \n\t"
        "movq          %1, %%mm6    \n\t"
        ::"m"(red_16mask),"m"(green_16mask));
    mm_end = end - 15;
    while (s < mm_end) {
        __asm__ volatile(
            "prefetchnta""  32(%1)           \n\t"
            "movd        (%1), %%mm0    \n\t"
            "movd       4(%1), %%mm3    \n\t"
            "punpckldq  8(%1), %%mm0    \n\t"
            "punpckldq 12(%1), %%mm3    \n\t"
            "movq       %%mm0, %%mm1    \n\t"
            "movq       %%mm0, %%mm2    \n\t"
            "movq       %%mm3, %%mm4    \n\t"
            "movq       %%mm3, %%mm5    \n\t"
            "psllq         $8, %%mm0    \n\t"
            "psllq         $8, %%mm3    \n\t"
            "pand       %%mm7, %%mm0    \n\t"
            "pand       %%mm7, %%mm3    \n\t"
            "psrlq         $5, %%mm1    \n\t"
            "psrlq         $5, %%mm4    \n\t"
            "pand       %%mm6, %%mm1    \n\t"
            "pand       %%mm6, %%mm4    \n\t"
            "psrlq        $19, %%mm2    \n\t"
            "psrlq        $19, %%mm5    \n\t"
            "pand          %2, %%mm2    \n\t"
            "pand          %2, %%mm5    \n\t"
            "por        %%mm1, %%mm0    \n\t"
            "por        %%mm4, %%mm3    \n\t"
            "por        %%mm2, %%mm0    \n\t"
            "por        %%mm5, %%mm3    \n\t"
            "psllq        $16, %%mm3    \n\t"
            "por        %%mm3, %%mm0    \n\t"
            "movntq""     %%mm0, (%0)     \n\t"
            :: "r"(d),"r"(s),"m"(blue_16mask):"memory");
        d += 4;
        s += 16;
    }
    __asm__ volatile("sfence":::"memory");
    __asm__ volatile("emms":::"memory");
    while (s < end) {
        register int rgb = *(const uint32_t*)s; s += 4;
        *d++ = ((rgb&0xF8)<<8) + ((rgb&0xFC00)>>5) + ((rgb&0xF80000)>>19);
    }
}

static inline void rgb32to15_mmxext(const uint8_t *src, uint8_t *dst, int src_size)
{
    const uint8_t *s = src;
    const uint8_t *end;
    const uint8_t *mm_end;
    uint16_t *d = (uint16_t *)dst;
    end = s + src_size;
    mm_end = end - 15;
    __asm__ volatile(
        "movq           %3, %%mm5   \n\t"
        "movq           %4, %%mm6   \n\t"
        "movq           %5, %%mm7   \n\t"
        "jmp            2f          \n\t"
        ".p2align        4          \n\t"
        "1:                         \n\t"
        "prefetchnta""   32(%1)          \n\t"
        "movd         (%1), %%mm0   \n\t"
        "movd        4(%1), %%mm3   \n\t"
        "punpckldq   8(%1), %%mm0   \n\t"
        "punpckldq  12(%1), %%mm3   \n\t"
        "movq        %%mm0, %%mm1   \n\t"
        "movq        %%mm3, %%mm4   \n\t"
        "pand        %%mm6, %%mm0   \n\t"
        "pand        %%mm6, %%mm3   \n\t"
        "pmaddwd     %%mm7, %%mm0   \n\t"
        "pmaddwd     %%mm7, %%mm3   \n\t"
        "pand        %%mm5, %%mm1   \n\t"
        "pand        %%mm5, %%mm4   \n\t"
        "por         %%mm1, %%mm0   \n\t"
        "por         %%mm4, %%mm3   \n\t"
        "psrld          $6, %%mm0   \n\t"
        "pslld         $10, %%mm3   \n\t"
        "por         %%mm3, %%mm0   \n\t"
        "movntq""      %%mm0, (%0)    \n\t"
        "add           $16,  %1     \n\t"
        "add            $8,  %0     \n\t"
        "2:                         \n\t"
        "cmp            %2,  %1     \n\t"
        " jb            1b          \n\t"
        : "+r" (d), "+r"(s)
        : "r" (mm_end), "m" (mask3215g), "m" (mask3216br), "m" (mul3215)
    );
    __asm__ volatile("sfence":::"memory");
    __asm__ volatile("emms":::"memory");
    while (s < end) {
        register int rgb = *(const uint32_t*)s; s += 4;
        *d++ = ((rgb&0xFF)>>3) + ((rgb&0xF800)>>6) + ((rgb&0xF80000)>>9);
    }
}

static inline void rgb32tobgr15_mmxext(const uint8_t *src, uint8_t *dst, int src_size)
{
    const uint8_t *s = src;
    const uint8_t *end;
    const uint8_t *mm_end;
    uint16_t *d = (uint16_t *)dst;
    end = s + src_size;
    __asm__ volatile("prefetchnta""    %0"::"m"(*src):"memory");
    __asm__ volatile(
        "movq          %0, %%mm7    \n\t"
        "movq          %1, %%mm6    \n\t"
        ::"m"(red_15mask),"m"(green_15mask));
    mm_end = end - 15;
    while (s < mm_end) {
        __asm__ volatile(
            "prefetchnta""  32(%1)           \n\t"
            "movd        (%1), %%mm0    \n\t"
            "movd       4(%1), %%mm3    \n\t"
            "punpckldq  8(%1), %%mm0    \n\t"
            "punpckldq 12(%1), %%mm3    \n\t"
            "movq       %%mm0, %%mm1    \n\t"
            "movq       %%mm0, %%mm2    \n\t"
            "movq       %%mm3, %%mm4    \n\t"
            "movq       %%mm3, %%mm5    \n\t"
            "psllq         $7, %%mm0    \n\t"
            "psllq         $7, %%mm3    \n\t"
            "pand       %%mm7, %%mm0    \n\t"
            "pand       %%mm7, %%mm3    \n\t"
            "psrlq         $6, %%mm1    \n\t"
            "psrlq         $6, %%mm4    \n\t"
            "pand       %%mm6, %%mm1    \n\t"
            "pand       %%mm6, %%mm4    \n\t"
            "psrlq        $19, %%mm2    \n\t"
            "psrlq        $19, %%mm5    \n\t"
            "pand          %2, %%mm2    \n\t"
            "pand          %2, %%mm5    \n\t"
            "por        %%mm1, %%mm0    \n\t"
            "por        %%mm4, %%mm3    \n\t"
            "por        %%mm2, %%mm0    \n\t"
            "por        %%mm5, %%mm3    \n\t"
            "psllq        $16, %%mm3    \n\t"
            "por        %%mm3, %%mm0    \n\t"
            "movntq""     %%mm0, (%0)     \n\t"
            ::"r"(d),"r"(s),"m"(blue_15mask):"memory");
        d += 4;
        s += 16;
    }
    __asm__ volatile("sfence":::"memory");
    __asm__ volatile("emms":::"memory");
    while (s < end) {
        register int rgb = *(const uint32_t*)s; s += 4;
        *d++ = ((rgb&0xF8)<<7) + ((rgb&0xF800)>>6) + ((rgb&0xF80000)>>19);
    }
}

static inline void rgb24tobgr16_mmxext(const uint8_t *src, uint8_t *dst, int src_size)
{
    const uint8_t *s = src;
    const uint8_t *end;
    const uint8_t *mm_end;
    uint16_t *d = (uint16_t *)dst;
    end = s + src_size;
    __asm__ volatile("prefetchnta""    %0"::"m"(*src):"memory");
    __asm__ volatile(
        "movq         %0, %%mm7     \n\t"
        "movq         %1, %%mm6     \n\t"
        ::"m"(red_16mask),"m"(green_16mask));
    mm_end = end - 11;
    while (s < mm_end) {
        __asm__ volatile(
            "prefetchnta""  32(%1)           \n\t"
            "movd        (%1), %%mm0    \n\t"
            "movd       3(%1), %%mm3    \n\t"
            "punpckldq  6(%1), %%mm0    \n\t"
            "punpckldq  9(%1), %%mm3    \n\t"
            "movq       %%mm0, %%mm1    \n\t"
            "movq       %%mm0, %%mm2    \n\t"
            "movq       %%mm3, %%mm4    \n\t"
            "movq       %%mm3, %%mm5    \n\t"
            "psrlq         $3, %%mm0    \n\t"
            "psrlq         $3, %%mm3    \n\t"
            "pand          %2, %%mm0    \n\t"
            "pand          %2, %%mm3    \n\t"
            "psrlq         $5, %%mm1    \n\t"
            "psrlq         $5, %%mm4    \n\t"
            "pand       %%mm6, %%mm1    \n\t"
            "pand       %%mm6, %%mm4    \n\t"
            "psrlq         $8, %%mm2    \n\t"
            "psrlq         $8, %%mm5    \n\t"
            "pand       %%mm7, %%mm2    \n\t"
            "pand       %%mm7, %%mm5    \n\t"
            "por        %%mm1, %%mm0    \n\t"
            "por        %%mm4, %%mm3    \n\t"
            "por        %%mm2, %%mm0    \n\t"
            "por        %%mm5, %%mm3    \n\t"
            "psllq        $16, %%mm3    \n\t"
            "por        %%mm3, %%mm0    \n\t"
            "movntq""     %%mm0, (%0)     \n\t"
            ::"r"(d),"r"(s),"m"(blue_16mask):"memory");
        d += 4;
        s += 12;
    }
    __asm__ volatile("sfence":::"memory");
    __asm__ volatile("emms":::"memory");
    while (s < end) {
        const int b = *s++;
        const int g = *s++;
        const int r = *s++;
        *d++ = (b>>3) | ((g&0xFC)<<3) | ((r&0xF8)<<8);
    }
}

static inline void rgb24to16_mmxext(const uint8_t *src, uint8_t *dst, int src_size)
{
    const uint8_t *s = src;
    const uint8_t *end;
    const uint8_t *mm_end;
    uint16_t *d = (uint16_t *)dst;
    end = s + src_size;
    __asm__ volatile("prefetchnta""    %0"::"m"(*src):"memory");
    __asm__ volatile(
        "movq         %0, %%mm7     \n\t"
        "movq         %1, %%mm6     \n\t"
        ::"m"(red_16mask),"m"(green_16mask));
    mm_end = end - 15;
    while (s < mm_end) {
        __asm__ volatile(
            "prefetchnta""  32(%1)           \n\t"
            "movd        (%1), %%mm0    \n\t"
            "movd       3(%1), %%mm3    \n\t"
            "punpckldq  6(%1), %%mm0    \n\t"
            "punpckldq  9(%1), %%mm3    \n\t"
            "movq       %%mm0, %%mm1    \n\t"
            "movq       %%mm0, %%mm2    \n\t"
            "movq       %%mm3, %%mm4    \n\t"
            "movq       %%mm3, %%mm5    \n\t"
            "psllq         $8, %%mm0    \n\t"
            "psllq         $8, %%mm3    \n\t"
            "pand       %%mm7, %%mm0    \n\t"
            "pand       %%mm7, %%mm3    \n\t"
            "psrlq         $5, %%mm1    \n\t"
            "psrlq         $5, %%mm4    \n\t"
            "pand       %%mm6, %%mm1    \n\t"
            "pand       %%mm6, %%mm4    \n\t"
            "psrlq        $19, %%mm2    \n\t"
            "psrlq        $19, %%mm5    \n\t"
            "pand          %2, %%mm2    \n\t"
            "pand          %2, %%mm5    \n\t"
            "por        %%mm1, %%mm0    \n\t"
            "por        %%mm4, %%mm3    \n\t"
            "por        %%mm2, %%mm0    \n\t"
            "por        %%mm5, %%mm3    \n\t"
            "psllq        $16, %%mm3    \n\t"
            "por        %%mm3, %%mm0    \n\t"
            "movntq""     %%mm0, (%0)     \n\t"
            ::"r"(d),"r"(s),"m"(blue_16mask):"memory");
        d += 4;
        s += 12;
    }
    __asm__ volatile("sfence":::"memory");
    __asm__ volatile("emms":::"memory");
    while (s < end) {
        const int r = *s++;
        const int g = *s++;
        const int b = *s++;
        *d++ = (b>>3) | ((g&0xFC)<<3) | ((r&0xF8)<<8);
    }
}

static inline void rgb24tobgr15_mmxext(const uint8_t *src, uint8_t *dst, int src_size)
{
    const uint8_t *s = src;
    const uint8_t *end;
    const uint8_t *mm_end;
    uint16_t *d = (uint16_t *)dst;
    end = s + src_size;
    __asm__ volatile("prefetchnta""    %0"::"m"(*src):"memory");
    __asm__ volatile(
        "movq          %0, %%mm7    \n\t"
        "movq          %1, %%mm6    \n\t"
        ::"m"(red_15mask),"m"(green_15mask));
    mm_end = end - 11;
    while (s < mm_end) {
        __asm__ volatile(
            "prefetchnta""  32(%1)           \n\t"
            "movd        (%1), %%mm0    \n\t"
            "movd       3(%1), %%mm3    \n\t"
            "punpckldq  6(%1), %%mm0    \n\t"
            "punpckldq  9(%1), %%mm3    \n\t"
            "movq       %%mm0, %%mm1    \n\t"
            "movq       %%mm0, %%mm2    \n\t"
            "movq       %%mm3, %%mm4    \n\t"
            "movq       %%mm3, %%mm5    \n\t"
            "psrlq         $3, %%mm0    \n\t"
            "psrlq         $3, %%mm3    \n\t"
            "pand          %2, %%mm0    \n\t"
            "pand          %2, %%mm3    \n\t"
            "psrlq         $6, %%mm1    \n\t"
            "psrlq         $6, %%mm4    \n\t"
            "pand       %%mm6, %%mm1    \n\t"
            "pand       %%mm6, %%mm4    \n\t"
            "psrlq         $9, %%mm2    \n\t"
            "psrlq         $9, %%mm5    \n\t"
            "pand       %%mm7, %%mm2    \n\t"
            "pand       %%mm7, %%mm5    \n\t"
            "por        %%mm1, %%mm0    \n\t"
            "por        %%mm4, %%mm3    \n\t"
            "por        %%mm2, %%mm0    \n\t"
            "por        %%mm5, %%mm3    \n\t"
            "psllq        $16, %%mm3    \n\t"
            "por        %%mm3, %%mm0    \n\t"
            "movntq""     %%mm0, (%0)     \n\t"
            ::"r"(d),"r"(s),"m"(blue_15mask):"memory");
        d += 4;
        s += 12;
    }
    __asm__ volatile("sfence":::"memory");
    __asm__ volatile("emms":::"memory");
    while (s < end) {
        const int b = *s++;
        const int g = *s++;
        const int r = *s++;
        *d++ = (b>>3) | ((g&0xF8)<<2) | ((r&0xF8)<<7);
    }
}

static inline void rgb24to15_mmxext(const uint8_t *src, uint8_t *dst, int src_size)
{
    const uint8_t *s = src;
    const uint8_t *end;
    const uint8_t *mm_end;
    uint16_t *d = (uint16_t *)dst;
    end = s + src_size;
    __asm__ volatile("prefetchnta""    %0"::"m"(*src):"memory");
    __asm__ volatile(
        "movq         %0, %%mm7     \n\t"
        "movq         %1, %%mm6     \n\t"
        ::"m"(red_15mask),"m"(green_15mask));
    mm_end = end - 15;
    while (s < mm_end) {
        __asm__ volatile(
            "prefetchnta"" 32(%1)            \n\t"
            "movd       (%1), %%mm0     \n\t"
            "movd      3(%1), %%mm3     \n\t"
            "punpckldq 6(%1), %%mm0     \n\t"
            "punpckldq 9(%1), %%mm3     \n\t"
            "movq      %%mm0, %%mm1     \n\t"
            "movq      %%mm0, %%mm2     \n\t"
            "movq      %%mm3, %%mm4     \n\t"
            "movq      %%mm3, %%mm5     \n\t"
            "psllq        $7, %%mm0     \n\t"
            "psllq        $7, %%mm3     \n\t"
            "pand      %%mm7, %%mm0     \n\t"
            "pand      %%mm7, %%mm3     \n\t"
            "psrlq        $6, %%mm1     \n\t"
            "psrlq        $6, %%mm4     \n\t"
            "pand      %%mm6, %%mm1     \n\t"
            "pand      %%mm6, %%mm4     \n\t"
            "psrlq       $19, %%mm2     \n\t"
            "psrlq       $19, %%mm5     \n\t"
            "pand         %2, %%mm2     \n\t"
            "pand         %2, %%mm5     \n\t"
            "por       %%mm1, %%mm0     \n\t"
            "por       %%mm4, %%mm3     \n\t"
            "por       %%mm2, %%mm0     \n\t"
            "por       %%mm5, %%mm3     \n\t"
            "psllq       $16, %%mm3     \n\t"
            "por       %%mm3, %%mm0     \n\t"
            "movntq""    %%mm0, (%0)      \n\t"
            ::"r"(d),"r"(s),"m"(blue_15mask):"memory");
        d += 4;
        s += 12;
    }
    __asm__ volatile("sfence":::"memory");
    __asm__ volatile("emms":::"memory");
    while (s < end) {
        const int r = *s++;
        const int g = *s++;
        const int b = *s++;
        *d++ = (b>>3) | ((g&0xF8)<<2) | ((r&0xF8)<<7);
    }
}

static inline void rgb15tobgr24_mmxext(const uint8_t *src, uint8_t *dst, int src_size)
{
    const uint16_t *end;
    const uint16_t *mm_end;
    uint8_t *d = dst;
    const uint16_t *s = (const uint16_t*)src;
    end = s + src_size/2;
    __asm__ volatile("prefetchnta""    %0"::"m"(*s):"memory");
    mm_end = end - 7;
    while (s < mm_end) {
        __asm__ volatile(
            "prefetchnta""  32(%1)           \n\t"
            "movq        (%1), %%mm0    \n\t"
            "movq        (%1), %%mm1    \n\t"
            "movq        (%1), %%mm2    \n\t"
            "pand          %2, %%mm0    \n\t"
            "pand          %3, %%mm1    \n\t"
            "pand          %4, %%mm2    \n\t"
            "psllq         $5, %%mm0    \n\t"
            "pmulhw        """ "mul15_mid"", %%mm0    \n\t"
            "pmulhw        """ "mul15_mid"", %%mm1    \n\t"
            "pmulhw        """ "mul15_hi"", %%mm2    \n\t"
            "movq       %%mm0, %%mm3    \n\t"
            "movq       %%mm1, %%mm4    \n\t"
            "movq       %%mm2, %%mm5    \n\t"
            "punpcklwd     %5, %%mm0    \n\t"
            "punpcklwd     %5, %%mm1    \n\t"
            "punpcklwd     %5, %%mm2    \n\t"
            "punpckhwd     %5, %%mm3    \n\t"
            "punpckhwd     %5, %%mm4    \n\t"
            "punpckhwd     %5, %%mm5    \n\t"
            "psllq         $8, %%mm1    \n\t"
            "psllq        $16, %%mm2    \n\t"
            "por        %%mm1, %%mm0    \n\t"
            "por        %%mm2, %%mm0    \n\t"
            "psllq         $8, %%mm4    \n\t"
            "psllq        $16, %%mm5    \n\t"
            "por        %%mm4, %%mm3    \n\t"
            "por        %%mm5, %%mm3    \n\t"
            "movq       %%mm0, %%mm6    \n\t"
            "movq       %%mm3, %%mm7    \n\t"
            "movq       8(%1), %%mm0    \n\t"
            "movq       8(%1), %%mm1    \n\t"
            "movq       8(%1), %%mm2    \n\t"
            "pand          %2, %%mm0    \n\t"
            "pand          %3, %%mm1    \n\t"
            "pand          %4, %%mm2    \n\t"
            "psllq         $5, %%mm0    \n\t"
            "pmulhw        """ "mul15_mid"", %%mm0    \n\t"
            "pmulhw        """ "mul15_mid"", %%mm1    \n\t"
            "pmulhw        """ "mul15_hi"", %%mm2    \n\t"
            "movq       %%mm0, %%mm3    \n\t"
            "movq       %%mm1, %%mm4    \n\t"
            "movq       %%mm2, %%mm5    \n\t"
            "punpcklwd     %5, %%mm0    \n\t"
            "punpcklwd     %5, %%mm1    \n\t"
            "punpcklwd     %5, %%mm2    \n\t"
            "punpckhwd     %5, %%mm3    \n\t"
            "punpckhwd     %5, %%mm4    \n\t"
            "punpckhwd     %5, %%mm5    \n\t"
            "psllq         $8, %%mm1    \n\t"
            "psllq        $16, %%mm2    \n\t"
            "por        %%mm1, %%mm0    \n\t"
            "por        %%mm2, %%mm0    \n\t"
            "psllq         $8, %%mm4    \n\t"
            "psllq        $16, %%mm5    \n\t"
            "por        %%mm4, %%mm3    \n\t"
            "por        %%mm5, %%mm3    \n\t"
            :"=m"(*d)
            :"r"(s),"m"(mask15b),"m"(mask15g),"m"(mask15r), "m"(mmx_null)
            
            :"memory");
        __asm__ volatile(
            "movq       %%mm0, %%mm4    \n\t"
            "movq       %%mm3, %%mm5    \n\t"
            "movq       %%mm6, %%mm0    \n\t"
            "movq       %%mm7, %%mm1    \n\t"
            "movq       %%mm4, %%mm6    \n\t"
            "movq       %%mm5, %%mm7    \n\t"
            "movq       %%mm0, %%mm2    \n\t"
            "movq       %%mm1, %%mm3    \n\t"
            "psrlq         $8, %%mm2    \n\t" "psrlq         $8, %%mm3    \n\t" "psrlq         $8, %%mm6    \n\t" "psrlq         $8, %%mm7    \n\t" "pand """ "mask24l"", %%mm0\n\t" "pand """ "mask24l"", %%mm1\n\t" "pand """ "mask24l"", %%mm4\n\t" "pand """ "mask24l"", %%mm5\n\t" "pand """ "mask24h"", %%mm2\n\t" "pand """ "mask24h"", %%mm3\n\t" "pand """ "mask24h"", %%mm6\n\t" "pand """ "mask24h"", %%mm7\n\t" "por        %%mm2, %%mm0    \n\t" "por        %%mm3, %%mm1    \n\t" "por        %%mm6, %%mm4    \n\t" "por        %%mm7, %%mm5    \n\t" "movq       %%mm1, %%mm2    \n\t" "movq       %%mm4, %%mm3    \n\t" "psllq        $48, %%mm2    \n\t" "psllq        $32, %%mm3    \n\t" "por        %%mm2, %%mm0    \n\t" "psrlq        $16, %%mm1    \n\t" "psrlq        $32, %%mm4    \n\t" "psllq        $16, %%mm5    \n\t" "por        %%mm3, %%mm1    \n\t" "por        %%mm5, %%mm4    \n\t" "movntq""     %%mm0,   (%0)    \n\t" "movntq""     %%mm1,  8(%0)    \n\t" "movntq""     %%mm4, 16(%0)"
            :: "r"(d), "m"(*s)
             
            :"memory");
        d += 24;
        s += 8;
    }
    __asm__ volatile("sfence":::"memory");
    __asm__ volatile("emms":::"memory");
    while (s < end) {
        register uint16_t bgr;
        bgr = *s++;
        *d++ = ((bgr&0x1F)<<3) | ((bgr&0x1F)>>2);
        *d++ = ((bgr&0x3E0)>>2) | ((bgr&0x3E0)>>7);
        *d++ = ((bgr&0x7C00)>>7) | ((bgr&0x7C00)>>12);
    }
}

static inline void rgb16tobgr24_mmxext(const uint8_t *src, uint8_t *dst, int src_size)
{
    const uint16_t *end;
    const uint16_t *mm_end;
    uint8_t *d = (uint8_t *)dst;
    const uint16_t *s = (const uint16_t *)src;
    end = s + src_size/2;
    __asm__ volatile("prefetchnta""    %0"::"m"(*s):"memory");
    mm_end = end - 7;
    while (s < mm_end) {
        __asm__ volatile(
            "prefetchnta""  32(%1)           \n\t"
            "movq        (%1), %%mm0    \n\t"
            "movq        (%1), %%mm1    \n\t"
            "movq        (%1), %%mm2    \n\t"
            "pand          %2, %%mm0    \n\t"
            "pand          %3, %%mm1    \n\t"
            "pand          %4, %%mm2    \n\t"
            "psllq         $5, %%mm0    \n\t"
            "psrlq         $1, %%mm2    \n\t"
            "pmulhw        """ "mul15_mid"", %%mm0    \n\t"
            "pmulhw        """ "mul16_mid"", %%mm1    \n\t"
            "pmulhw        """ "mul15_hi"", %%mm2    \n\t"
            "movq       %%mm0, %%mm3    \n\t"
            "movq       %%mm1, %%mm4    \n\t"
            "movq       %%mm2, %%mm5    \n\t"
            "punpcklwd     %5, %%mm0    \n\t"
            "punpcklwd     %5, %%mm1    \n\t"
            "punpcklwd     %5, %%mm2    \n\t"
            "punpckhwd     %5, %%mm3    \n\t"
            "punpckhwd     %5, %%mm4    \n\t"
            "punpckhwd     %5, %%mm5    \n\t"
            "psllq         $8, %%mm1    \n\t"
            "psllq        $16, %%mm2    \n\t"
            "por        %%mm1, %%mm0    \n\t"
            "por        %%mm2, %%mm0    \n\t"
            "psllq         $8, %%mm4    \n\t"
            "psllq        $16, %%mm5    \n\t"
            "por        %%mm4, %%mm3    \n\t"
            "por        %%mm5, %%mm3    \n\t"
            "movq       %%mm0, %%mm6    \n\t"
            "movq       %%mm3, %%mm7    \n\t"
            "movq       8(%1), %%mm0    \n\t"
            "movq       8(%1), %%mm1    \n\t"
            "movq       8(%1), %%mm2    \n\t"
            "pand          %2, %%mm0    \n\t"
            "pand          %3, %%mm1    \n\t"
            "pand          %4, %%mm2    \n\t"
            "psllq         $5, %%mm0    \n\t"
            "psrlq         $1, %%mm2    \n\t"
            "pmulhw        """ "mul15_mid"", %%mm0    \n\t"
            "pmulhw        """ "mul16_mid"", %%mm1    \n\t"
            "pmulhw        """ "mul15_hi"", %%mm2    \n\t"
            "movq       %%mm0, %%mm3    \n\t"
            "movq       %%mm1, %%mm4    \n\t"
            "movq       %%mm2, %%mm5    \n\t"
            "punpcklwd     %5, %%mm0    \n\t"
            "punpcklwd     %5, %%mm1    \n\t"
            "punpcklwd     %5, %%mm2    \n\t"
            "punpckhwd     %5, %%mm3    \n\t"
            "punpckhwd     %5, %%mm4    \n\t"
            "punpckhwd     %5, %%mm5    \n\t"
            "psllq         $8, %%mm1    \n\t"
            "psllq        $16, %%mm2    \n\t"
            "por        %%mm1, %%mm0    \n\t"
            "por        %%mm2, %%mm0    \n\t"
            "psllq         $8, %%mm4    \n\t"
            "psllq        $16, %%mm5    \n\t"
            "por        %%mm4, %%mm3    \n\t"
            "por        %%mm5, %%mm3    \n\t"
            :"=m"(*d)
            :"r"(s),"m"(mask15b),"m"(mask16g),"m"(mask16r),"m"(mmx_null)
            
            :"memory");
        __asm__ volatile(
            "movq       %%mm0, %%mm4    \n\t"
            "movq       %%mm3, %%mm5    \n\t"
            "movq       %%mm6, %%mm0    \n\t"
            "movq       %%mm7, %%mm1    \n\t"
            "movq       %%mm4, %%mm6    \n\t"
            "movq       %%mm5, %%mm7    \n\t"
            "movq       %%mm0, %%mm2    \n\t"
            "movq       %%mm1, %%mm3    \n\t"
            "psrlq         $8, %%mm2    \n\t" "psrlq         $8, %%mm3    \n\t" "psrlq         $8, %%mm6    \n\t" "psrlq         $8, %%mm7    \n\t" "pand """ "mask24l"", %%mm0\n\t" "pand """ "mask24l"", %%mm1\n\t" "pand """ "mask24l"", %%mm4\n\t" "pand """ "mask24l"", %%mm5\n\t" "pand """ "mask24h"", %%mm2\n\t" "pand """ "mask24h"", %%mm3\n\t" "pand """ "mask24h"", %%mm6\n\t" "pand """ "mask24h"", %%mm7\n\t" "por        %%mm2, %%mm0    \n\t" "por        %%mm3, %%mm1    \n\t" "por        %%mm6, %%mm4    \n\t" "por        %%mm7, %%mm5    \n\t" "movq       %%mm1, %%mm2    \n\t" "movq       %%mm4, %%mm3    \n\t" "psllq        $48, %%mm2    \n\t" "psllq        $32, %%mm3    \n\t" "por        %%mm2, %%mm0    \n\t" "psrlq        $16, %%mm1    \n\t" "psrlq        $32, %%mm4    \n\t" "psllq        $16, %%mm5    \n\t" "por        %%mm3, %%mm1    \n\t" "por        %%mm5, %%mm4    \n\t" "movntq""     %%mm0,   (%0)    \n\t" "movntq""     %%mm1,  8(%0)    \n\t" "movntq""     %%mm4, 16(%0)"
            :: "r"(d), "m"(*s)
             
            :"memory");
        d += 24;
        s += 8;
    }
    __asm__ volatile("sfence":::"memory");
    __asm__ volatile("emms":::"memory");
    while (s < end) {
        register uint16_t bgr;
        bgr = *s++;
        *d++ = ((bgr&0x1F)<<3) | ((bgr&0x1F)>>2);
        *d++ = ((bgr&0x7E0)>>3) | ((bgr&0x7E0)>>9);
        *d++ = ((bgr&0xF800)>>8) | ((bgr&0xF800)>>13);
    }
}
#define PACK_RGB32 "packuswb   %%mm7, %%mm0    \n\t" "packuswb   %%mm7, %%mm1    \n\t" "packuswb   %%mm7, %%mm2    \n\t" "punpcklbw  %%mm1, %%mm0    \n\t" "punpcklbw  %%mm6, %%mm2    \n\t" "movq       %%mm0, %%mm3    \n\t" "punpcklwd  %%mm2, %%mm0    \n\t" "punpckhwd  %%mm2, %%mm3    \n\t" MOVNTQ"     %%mm0,  (%0)    \n\t" MOVNTQ"     %%mm3, 8(%0)    \n\t"
static inline void rgb15to32_mmxext(const uint8_t *src, uint8_t *dst, int src_size)
{
    const uint16_t *end;
    const uint16_t *mm_end;
    uint8_t *d = dst;
    const uint16_t *s = (const uint16_t *)src;
    end = s + src_size/2;
    __asm__ volatile("prefetchnta""    %0"::"m"(*s):"memory");
    __asm__ volatile("pxor    %%mm7,%%mm7    \n\t":::"memory");
    __asm__ volatile("pcmpeqd %%mm6,%%mm6    \n\t":::"memory");
    mm_end = end - 3;
    while (s < mm_end) {
        __asm__ volatile(
            "prefetchnta""  32(%1)           \n\t"
            "movq        (%1), %%mm0    \n\t"
            "movq        (%1), %%mm1    \n\t"
            "movq        (%1), %%mm2    \n\t"
            "pand          %2, %%mm0    \n\t"
            "pand          %3, %%mm1    \n\t"
            "pand          %4, %%mm2    \n\t"
            "psllq         $5, %%mm0    \n\t"
            "pmulhw        %5, %%mm0    \n\t"
            "pmulhw        %5, %%mm1    \n\t"
            "pmulhw        """ "mul15_hi"", %%mm2    \n\t"
            "packuswb   %%mm7, %%mm0    \n\t" "packuswb   %%mm7, %%mm1    \n\t" "packuswb   %%mm7, %%mm2    \n\t" "punpcklbw  %%mm1, %%mm0    \n\t" "punpcklbw  %%mm6, %%mm2    \n\t" "movq       %%mm0, %%mm3    \n\t" "punpcklwd  %%mm2, %%mm0    \n\t" "punpckhwd  %%mm2, %%mm3    \n\t" "movntq""     %%mm0,  (%0)    \n\t" "movntq""     %%mm3, 8(%0)    \n\t"
            ::"r"(d),"r"(s),"m"(mask15b),"m"(mask15g),"m"(mask15r) ,"m"(mul15_mid)
             
            :"memory");
        d += 16;
        s += 4;
    }
    __asm__ volatile("sfence":::"memory");
    __asm__ volatile("emms":::"memory");
    while (s < end) {
        register uint16_t bgr;
        bgr = *s++;
        *d++ = ((bgr&0x1F)<<3) | ((bgr&0x1F)>>2);
        *d++ = ((bgr&0x3E0)>>2) | ((bgr&0x3E0)>>7);
        *d++ = ((bgr&0x7C00)>>7) | ((bgr&0x7C00)>>12);
        *d++ = 255;
    }
}

static inline void rgb16to32_mmxext(const uint8_t *src, uint8_t *dst, int src_size)
{
    const uint16_t *end;
    const uint16_t *mm_end;
    uint8_t *d = dst;
    const uint16_t *s = (const uint16_t*)src;
    end = s + src_size/2;
    __asm__ volatile("prefetchnta""    %0"::"m"(*s):"memory");
    __asm__ volatile("pxor    %%mm7,%%mm7    \n\t":::"memory");
    __asm__ volatile("pcmpeqd %%mm6,%%mm6    \n\t":::"memory");
    mm_end = end - 3;
    while (s < mm_end) {
        __asm__ volatile(
            "prefetchnta""  32(%1)           \n\t"
            "movq        (%1), %%mm0    \n\t"
            "movq        (%1), %%mm1    \n\t"
            "movq        (%1), %%mm2    \n\t"
            "pand          %2, %%mm0    \n\t"
            "pand          %3, %%mm1    \n\t"
            "pand          %4, %%mm2    \n\t"
            "psllq         $5, %%mm0    \n\t"
            "psrlq         $1, %%mm2    \n\t"
            "pmulhw        %5, %%mm0    \n\t"
            "pmulhw        """ "mul16_mid"", %%mm1    \n\t"
            "pmulhw        """ "mul15_hi"", %%mm2    \n\t"
            "packuswb   %%mm7, %%mm0    \n\t" "packuswb   %%mm7, %%mm1    \n\t" "packuswb   %%mm7, %%mm2    \n\t" "punpcklbw  %%mm1, %%mm0    \n\t" "punpcklbw  %%mm6, %%mm2    \n\t" "movq       %%mm0, %%mm3    \n\t" "punpcklwd  %%mm2, %%mm0    \n\t" "punpckhwd  %%mm2, %%mm3    \n\t" "movntq""     %%mm0,  (%0)    \n\t" "movntq""     %%mm3, 8(%0)    \n\t"
            ::"r"(d),"r"(s),"m"(mask15b),"m"(mask16g),"m"(mask16r),"m"(mul15_mid)
             
            :"memory");
        d += 16;
        s += 4;
    }
    __asm__ volatile("sfence":::"memory");
    __asm__ volatile("emms":::"memory");
    while (s < end) {
        register uint16_t bgr;
        bgr = *s++;
        *d++ = ((bgr&0x1F)<<3) | ((bgr&0x1F)>>2);
        *d++ = ((bgr&0x7E0)>>3) | ((bgr&0x7E0)>>9);
        *d++ = ((bgr&0xF800)>>8) | ((bgr&0xF800)>>13);
        *d++ = 255;
    }
}

static inline void rgb24tobgr24_mmxext(const uint8_t *src, uint8_t *dst, int src_size)
{
    unsigned i;
    x86_reg mmx_size= 23 - src_size;
    __asm__ volatile (
        "test             %%""rax"", %%""rax""    \n\t"
        "jns                     2f                     \n\t"
        "movq     """ "mask24r"", %%mm5              \n\t"
        "movq     """ "mask24g"", %%mm6              \n\t"
        "movq     """ "mask24b"", %%mm7              \n\t"
        ".p2align                 4                     \n\t"
        "1:                                             \n\t"
        "prefetchnta"" 32(%1, %%""rax"")                  \n\t"
        "movq    (%1, %%""rax""), %%mm0              \n\t"
        "movq    (%1, %%""rax""), %%mm1              \n\t"
        "movq   2(%1, %%""rax""), %%mm2              \n\t"
        "psllq                  $16, %%mm0              \n\t"
        "pand                 %%mm5, %%mm0              \n\t"
        "pand                 %%mm6, %%mm1              \n\t"
        "pand                 %%mm7, %%mm2              \n\t"
        "por                  %%mm0, %%mm1              \n\t"
        "por                  %%mm2, %%mm1              \n\t"
        "movq   6(%1, %%""rax""), %%mm0              \n\t"
        "movntq""               %%mm1,(%2, %%""rax"")  \n\t"
        "movq   8(%1, %%""rax""), %%mm1              \n\t"
        "movq  10(%1, %%""rax""), %%mm2              \n\t"
        "pand                 %%mm7, %%mm0              \n\t"
        "pand                 %%mm5, %%mm1              \n\t"
        "pand                 %%mm6, %%mm2              \n\t"
        "por                  %%mm0, %%mm1              \n\t"
        "por                  %%mm2, %%mm1              \n\t"
        "movq  14(%1, %%""rax""), %%mm0              \n\t"
        "movntq""               %%mm1, 8(%2, %%""rax"")\n\t"
        "movq  16(%1, %%""rax""), %%mm1              \n\t"
        "movq  18(%1, %%""rax""), %%mm2              \n\t"
        "pand                 %%mm6, %%mm0              \n\t"
        "pand                 %%mm7, %%mm1              \n\t"
        "pand                 %%mm5, %%mm2              \n\t"
        "por                  %%mm0, %%mm1              \n\t"
        "por                  %%mm2, %%mm1              \n\t"
        "movntq""               %%mm1, 16(%2, %%""rax"") \n\t"
        "add                    $24, %%""rax""       \n\t"
        " js                     1b                     \n\t"
        "2:                                             \n\t"
        : "+a" (mmx_size)
        : "r" (src-mmx_size), "r"(dst-mmx_size)
         
    );
    __asm__ volatile("sfence":::"memory");
    __asm__ volatile("emms":::"memory");
    if (mmx_size==23) return;
    src+= src_size;
    dst+= src_size;
    src_size= 23-mmx_size;
    src-= src_size;
    dst-= src_size;
    for (i=0; i<src_size; i+=3) {
        register uint8_t x;
        x = src[i + 2];
        dst[i + 1] = src[i + 1];
        dst[i + 2] = src[i + 0];
        dst[i + 0] = x;
    }
}

static inline void yuvPlanartoyuy2_mmxext(const uint8_t *ysrc, const uint8_t *usrc, const uint8_t *vsrc, uint8_t *dst,
                                           int width, int height,
                                           int lumStride, int chromStride, int dstStride, int vertLumPerChroma)
{
    int y;
    const x86_reg chromWidth= width>>1;
    for (y=0; y<height; y++) {
        __asm__ volatile(
            "xor                 %%""rax"", %%""rax"" \n\t"
            ".p2align                    4              \n\t"
            "1:                                         \n\t"
            "prefetchnta"" 32(%1, %%""rax"", 2)           \n\t"
            "prefetchnta"" 32(%2, %%""rax"")              \n\t"
            "prefetchnta"" 32(%3, %%""rax"")              \n\t"
            "movq       (%2, %%""rax""), %%mm0       \n\t"
            "movq                    %%mm0, %%mm2       \n\t"
            "movq       (%3, %%""rax""), %%mm1       \n\t"
            "punpcklbw               %%mm1, %%mm0       \n\t"
            "punpckhbw               %%mm1, %%mm2       \n\t"
            "movq     (%1, %%""rax"",2), %%mm3       \n\t"
            "movq    8(%1, %%""rax"",2), %%mm5       \n\t"
            "movq                    %%mm3, %%mm4       \n\t"
            "movq                    %%mm5, %%mm6       \n\t"
            "punpcklbw               %%mm0, %%mm3       \n\t"
            "punpckhbw               %%mm0, %%mm4       \n\t"
            "punpcklbw               %%mm2, %%mm5       \n\t"
            "punpckhbw               %%mm2, %%mm6       \n\t"
            "movntq""                  %%mm3,   (%0, %%""rax"", 4)    \n\t"
            "movntq""                  %%mm4,  8(%0, %%""rax"", 4)    \n\t"
            "movntq""                  %%mm5, 16(%0, %%""rax"", 4)    \n\t"
            "movntq""                  %%mm6, 24(%0, %%""rax"", 4)    \n\t"
            "add                        $8, %%""rax"" \n\t"
            "cmp                        %4, %%""rax"" \n\t"
            " jb                        1b               \n\t"
            ::"r"(dst), "r"(ysrc), "r"(usrc), "r"(vsrc), "g" (chromWidth)
            : "%""rax"
        );
        if ((y&(vertLumPerChroma-1)) == vertLumPerChroma-1) {
            usrc += chromStride;
            vsrc += chromStride;
        }
        ysrc += lumStride;
        dst += dstStride;
    }
    __asm__("emms""       \n\t"
            "sfence""     \n\t"
            :::"memory");
}

static inline void yv12toyuy2_mmxext(const uint8_t *ysrc, const uint8_t *usrc, const uint8_t *vsrc, uint8_t *dst,
                                      int width, int height,
                                      int lumStride, int chromStride, int dstStride)
{
    yuvPlanartoyuy2_mmxext(ysrc, usrc, vsrc, dst, width, height, lumStride, chromStride, dstStride, 2);
}

static inline void yuvPlanartouyvy_mmxext(const uint8_t *ysrc, const uint8_t *usrc, const uint8_t *vsrc, uint8_t *dst,
                                           int width, int height,
                                           int lumStride, int chromStride, int dstStride, int vertLumPerChroma)
{
    int y;
    const x86_reg chromWidth= width>>1;
    for (y=0; y<height; y++) {
        __asm__ volatile(
            "xor             %%""rax"", %%""rax"" \n\t"
            ".p2align                   4               \n\t"
            "1:                                         \n\t"
            "prefetchnta"" 32(%1, %%""rax"", 2)           \n\t"
            "prefetchnta"" 32(%2, %%""rax"")              \n\t"
            "prefetchnta"" 32(%3, %%""rax"")              \n\t"
            "movq      (%2, %%""rax""), %%mm0        \n\t"
            "movq                   %%mm0, %%mm2        \n\t"
            "movq      (%3, %%""rax""), %%mm1        \n\t"
            "punpcklbw              %%mm1, %%mm0        \n\t"
            "punpckhbw              %%mm1, %%mm2        \n\t"
            "movq    (%1, %%""rax"",2), %%mm3        \n\t"
            "movq   8(%1, %%""rax"",2), %%mm5        \n\t"
            "movq                   %%mm0, %%mm4        \n\t"
            "movq                   %%mm2, %%mm6        \n\t"
            "punpcklbw              %%mm3, %%mm0        \n\t"
            "punpckhbw              %%mm3, %%mm4        \n\t"
            "punpcklbw              %%mm5, %%mm2        \n\t"
            "punpckhbw              %%mm5, %%mm6        \n\t"
            "movntq""                 %%mm0,   (%0, %%""rax"", 4)     \n\t"
            "movntq""                 %%mm4,  8(%0, %%""rax"", 4)     \n\t"
            "movntq""                 %%mm2, 16(%0, %%""rax"", 4)     \n\t"
            "movntq""                 %%mm6, 24(%0, %%""rax"", 4)     \n\t"
            "add                       $8, %%""rax"" \n\t"
            "cmp                       %4, %%""rax"" \n\t"
            " jb                       1b               \n\t"
            ::"r"(dst), "r"(ysrc), "r"(usrc), "r"(vsrc), "g" (chromWidth)
            : "%""rax"
        );
        if ((y&(vertLumPerChroma-1)) == vertLumPerChroma-1) {
            usrc += chromStride;
            vsrc += chromStride;
        }
        ysrc += lumStride;
        dst += dstStride;
    }
    __asm__("emms""       \n\t"
            "sfence""     \n\t"
            :::"memory");
}

static inline void yv12touyvy_mmxext(const uint8_t *ysrc, const uint8_t *usrc, const uint8_t *vsrc, uint8_t *dst,
                                      int width, int height,
                                      int lumStride, int chromStride, int dstStride)
{
    yuvPlanartouyvy_mmxext(ysrc, usrc, vsrc, dst, width, height, lumStride, chromStride, dstStride, 2);
}

static inline void yuv422ptouyvy_mmxext(const uint8_t *ysrc, const uint8_t *usrc, const uint8_t *vsrc, uint8_t *dst,
                                         int width, int height,
                                         int lumStride, int chromStride, int dstStride)
{
    yuvPlanartouyvy_mmxext(ysrc, usrc, vsrc, dst, width, height, lumStride, chromStride, dstStride, 1);
}

static inline void yuv422ptoyuy2_mmxext(const uint8_t *ysrc, const uint8_t *usrc, const uint8_t *vsrc, uint8_t *dst,
                                         int width, int height,
                                         int lumStride, int chromStride, int dstStride)
{
    yuvPlanartoyuy2_mmxext(ysrc, usrc, vsrc, dst, width, height, lumStride, chromStride, dstStride, 1);
}

static inline void yuy2toyv12_mmxext(const uint8_t *src, uint8_t *ydst, uint8_t *udst, uint8_t *vdst,
                                      int width, int height,
                                      int lumStride, int chromStride, int srcStride)
{
    int y;
    const x86_reg chromWidth= width>>1;
    for (y=0; y<height; y+=2) {
        __asm__ volatile(
            "xor              %%""rax"", %%""rax""\n\t"
            "pcmpeqw                 %%mm7, %%mm7       \n\t"
            "psrlw                      $8, %%mm7       \n\t"
            ".p2align                    4              \n\t"
            "1:                \n\t"
            "prefetchnta"" 64(%0, %%""rax"", 4)           \n\t"
            "movq    (%0, %%""rax"", 4), %%mm0       \n\t"
            "movq   8(%0, %%""rax"", 4), %%mm1       \n\t"
            "movq                    %%mm0, %%mm2       \n\t"
            "movq                    %%mm1, %%mm3       \n\t"
            "psrlw                      $8, %%mm0       \n\t"
            "psrlw                      $8, %%mm1       \n\t"
            "pand                    %%mm7, %%mm2       \n\t"
            "pand                    %%mm7, %%mm3       \n\t"
            "packuswb                %%mm1, %%mm0       \n\t"
            "packuswb                %%mm3, %%mm2       \n\t"
            "movntq""                  %%mm2, (%1, %%""rax"", 2) \n\t"
            "movq  16(%0, %%""rax"", 4), %%mm1       \n\t"
            "movq  24(%0, %%""rax"", 4), %%mm2       \n\t"
            "movq                    %%mm1, %%mm3       \n\t"
            "movq                    %%mm2, %%mm4       \n\t"
            "psrlw                      $8, %%mm1       \n\t"
            "psrlw                      $8, %%mm2       \n\t"
            "pand                    %%mm7, %%mm3       \n\t"
            "pand                    %%mm7, %%mm4       \n\t"
            "packuswb                %%mm2, %%mm1       \n\t"
            "packuswb                %%mm4, %%mm3       \n\t"
            "movntq""                  %%mm3, 8(%1, %%""rax"", 2) \n\t"
            "movq                    %%mm0, %%mm2       \n\t"
            "movq                    %%mm1, %%mm3       \n\t"
            "psrlw                      $8, %%mm0       \n\t"
            "psrlw                      $8, %%mm1       \n\t"
            "pand                    %%mm7, %%mm2       \n\t"
            "pand                    %%mm7, %%mm3       \n\t"
            "packuswb                %%mm1, %%mm0       \n\t"
            "packuswb                %%mm3, %%mm2       \n\t"
            "movntq""                  %%mm0, (%3, %%""rax"")     \n\t"
            "movntq""                  %%mm2, (%2, %%""rax"")     \n\t"
            "add                        $8, %%""rax"" \n\t"
            "cmp                        %4, %%""rax"" \n\t"
            " jb                        1b               \n\t"
            ::"r"(src), "r"(ydst), "r"(udst), "r"(vdst), "g" (chromWidth)
            : "memory", "%""rax"
        );
        ydst += lumStride;
        src += srcStride;
        __asm__ volatile(
            "xor              %%""rax"", %%""rax""\n\t"
            ".p2align                    4              \n\t"
            "1:                                         \n\t"
            "prefetchnta"" 64(%0, %%""rax"", 4)           \n\t"
            "movq    (%0, %%""rax"", 4), %%mm0       \n\t"
            "movq   8(%0, %%""rax"", 4), %%mm1       \n\t"
            "movq  16(%0, %%""rax"", 4), %%mm2       \n\t"
            "movq  24(%0, %%""rax"", 4), %%mm3       \n\t"
            "pand                    %%mm7, %%mm0       \n\t"
            "pand                    %%mm7, %%mm1       \n\t"
            "pand                    %%mm7, %%mm2       \n\t"
            "pand                    %%mm7, %%mm3       \n\t"
            "packuswb                %%mm1, %%mm0       \n\t"
            "packuswb                %%mm3, %%mm2       \n\t"
            "movntq""                  %%mm0,  (%1, %%""rax"", 2) \n\t"
            "movntq""                  %%mm2, 8(%1, %%""rax"", 2) \n\t"
            "add                        $8, %%""rax""\n\t"
            "cmp                        %4, %%""rax""\n\t"
            " jb                        1b              \n\t"
            ::"r"(src), "r"(ydst), "r"(udst), "r"(vdst), "g" (chromWidth)
            : "memory", "%""rax"
        );
        udst += chromStride;
        vdst += chromStride;
        ydst += lumStride;
        src += srcStride;
    }
    __asm__ volatile("emms""       \n\t"
                     "sfence""     \n\t"
                     :::"memory");
}

static inline void planar2x_mmxext(const uint8_t *src, uint8_t *dst, int srcWidth, int srcHeight, int srcStride, int dstStride)
{
    int x,y;
    dst[0]= src[0];
    for (x=0; x<srcWidth-1; x++) {
        dst[2*x+1]= (3*src[x] + src[x+1])>>2;
        dst[2*x+2]= ( src[x] + 3*src[x+1])>>2;
    }
    dst[2*srcWidth-1]= src[srcWidth-1];
    dst+= dstStride;
    for (y=1; y<srcHeight; y++) {
        x86_reg mmxSize= srcWidth&~15;
        if (mmxSize) {
        __asm__ volatile(
            "mov                       %4, %%""rax"" \n\t"
            "movq        """ "mmx_ff"", %%mm0    \n\t"
            "movq      (%0, %%""rax""), %%mm4    \n\t"
            "movq                   %%mm4, %%mm2    \n\t"
            "psllq                     $8, %%mm4    \n\t"
            "pand                   %%mm0, %%mm2    \n\t"
            "por                    %%mm2, %%mm4    \n\t"
            "movq      (%1, %%""rax""), %%mm5    \n\t"
            "movq                   %%mm5, %%mm3    \n\t"
            "psllq                     $8, %%mm5    \n\t"
            "pand                   %%mm0, %%mm3    \n\t"
            "por                    %%mm3, %%mm5    \n\t"
            "1:                                     \n\t"
            "movq      (%0, %%""rax""), %%mm0    \n\t"
            "movq      (%1, %%""rax""), %%mm1    \n\t"
            "movq     1(%0, %%""rax""), %%mm2    \n\t"
            "movq     1(%1, %%""rax""), %%mm3    \n\t"
            "pavgb""                  %%mm0, %%mm5    \n\t"
            "pavgb""                  %%mm0, %%mm3    \n\t"
            "pavgb""                  %%mm0, %%mm5    \n\t"
            "pavgb""                  %%mm0, %%mm3    \n\t"
            "pavgb""                  %%mm1, %%mm4    \n\t"
            "pavgb""                  %%mm1, %%mm2    \n\t"
            "pavgb""                  %%mm1, %%mm4    \n\t"
            "pavgb""                  %%mm1, %%mm2    \n\t"
            "movq                   %%mm5, %%mm7    \n\t"
            "movq                   %%mm4, %%mm6    \n\t"
            "punpcklbw              %%mm3, %%mm5    \n\t"
            "punpckhbw              %%mm3, %%mm7    \n\t"
            "punpcklbw              %%mm2, %%mm4    \n\t"
            "punpckhbw              %%mm2, %%mm6    \n\t"
            "movntq""                 %%mm5,  (%2, %%""rax"", 2)  \n\t"
            "movntq""                 %%mm7, 8(%2, %%""rax"", 2)  \n\t"
            "movntq""                 %%mm4,  (%3, %%""rax"", 2)  \n\t"
            "movntq""                 %%mm6, 8(%3, %%""rax"", 2)  \n\t"
            "add                       $8, %%""rax""            \n\t"
            "movq    -1(%0, %%""rax""), %%mm4    \n\t"
            "movq    -1(%1, %%""rax""), %%mm5    \n\t"
            " js                       1b           \n\t"
            :: "r" (src + mmxSize ), "r" (src + srcStride + mmxSize ),
               "r" (dst + mmxSize*2), "r" (dst + dstStride + mmxSize*2),
               "g" (-mmxSize)
              
            : "%""rax"
        );
        } else {
            mmxSize = 1;
            dst[0] = (src[0] * 3 + src[srcStride]) >> 2;
            dst[dstStride] = (src[0] + 3 * src[srcStride]) >> 2;
        }
        for (x=mmxSize-1; x<srcWidth-1; x++) {
            dst[2*x +1]= (3*src[x+0] + src[x+srcStride+1])>>2;
            dst[2*x+dstStride+2]= ( src[x+0] + 3*src[x+srcStride+1])>>2;
            dst[2*x+dstStride+1]= ( src[x+1] + 3*src[x+srcStride ])>>2;
            dst[2*x +2]= (3*src[x+1] + src[x+srcStride ])>>2;
        }
        dst[srcWidth*2 -1 ]= (3*src[srcWidth-1] + src[srcWidth-1 + srcStride])>>2;
        dst[srcWidth*2 -1 + dstStride]= ( src[srcWidth-1] + 3*src[srcWidth-1 + srcStride])>>2;
        dst+=dstStride*2;
        src+=srcStride;
    }
    dst[0]= src[0];
    for (x=0; x<srcWidth-1; x++) {
        dst[2*x+1]= (3*src[x] + src[x+1])>>2;
        dst[2*x+2]= ( src[x] + 3*src[x+1])>>2;
    }
    dst[2*srcWidth-1]= src[srcWidth-1];
    __asm__ volatile("emms""       \n\t"
                     "sfence""     \n\t"
                     :::"memory");
}
static inline void uyvytoyv12_mmxext(const uint8_t *src, uint8_t *ydst, uint8_t *udst, uint8_t *vdst,
                                      int width, int height,
                                      int lumStride, int chromStride, int srcStride)
{
    int y;
    const x86_reg chromWidth= width>>1;
    for (y=0; y<height; y+=2) {
        __asm__ volatile(
            "xor          %%""rax"", %%""rax"" \n\t"
            "pcmpeqw             %%mm7, %%mm7   \n\t"
            "psrlw                  $8, %%mm7   \n\t"
            ".p2align                4          \n\t"
            "1:                                 \n\t"
            "prefetchnta"" 64(%0, %%""rax"", 4)          \n\t"
            "movq       (%0, %%""rax"", 4), %%mm0   \n\t"
            "movq      8(%0, %%""rax"", 4), %%mm1   \n\t"
            "movq                %%mm0, %%mm2   \n\t"
            "movq                %%mm1, %%mm3   \n\t"
            "pand                %%mm7, %%mm0   \n\t"
            "pand                %%mm7, %%mm1   \n\t"
            "psrlw                  $8, %%mm2   \n\t"
            "psrlw                  $8, %%mm3   \n\t"
            "packuswb            %%mm1, %%mm0   \n\t"
            "packuswb            %%mm3, %%mm2   \n\t"
            "movntq""              %%mm2,  (%1, %%""rax"", 2) \n\t"
            "movq     16(%0, %%""rax"", 4), %%mm1   \n\t"
            "movq     24(%0, %%""rax"", 4), %%mm2   \n\t"
            "movq                %%mm1, %%mm3   \n\t"
            "movq                %%mm2, %%mm4   \n\t"
            "pand                %%mm7, %%mm1   \n\t"
            "pand                %%mm7, %%mm2   \n\t"
            "psrlw                  $8, %%mm3   \n\t"
            "psrlw                  $8, %%mm4   \n\t"
            "packuswb            %%mm2, %%mm1   \n\t"
            "packuswb            %%mm4, %%mm3   \n\t"
            "movntq""              %%mm3, 8(%1, %%""rax"", 2) \n\t"
            "movq                %%mm0, %%mm2   \n\t"
            "movq                %%mm1, %%mm3   \n\t"
            "psrlw                  $8, %%mm0   \n\t"
            "psrlw                  $8, %%mm1   \n\t"
            "pand                %%mm7, %%mm2   \n\t"
            "pand                %%mm7, %%mm3   \n\t"
            "packuswb            %%mm1, %%mm0   \n\t"
            "packuswb            %%mm3, %%mm2   \n\t"
            "movntq""              %%mm0, (%3, %%""rax"") \n\t"
            "movntq""              %%mm2, (%2, %%""rax"") \n\t"
            "add                    $8, %%""rax"" \n\t"
            "cmp                    %4, %%""rax"" \n\t"
            " jb                    1b               \n\t"
            ::"r"(src), "r"(ydst), "r"(udst), "r"(vdst), "g" (chromWidth)
            : "memory", "%""rax"
        );
        ydst += lumStride;
        src += srcStride;
        __asm__ volatile(
            "xor          %%""rax"", %%""rax""  \n\t"
            ".p2align                4                \n\t"
            "1:                                       \n\t"
            "prefetchnta"" 64(%0, %%""rax"", 4)         \n\t"
            "movq       (%0, %%""rax"", 4), %%mm0  \n\t"
            "movq      8(%0, %%""rax"", 4), %%mm1  \n\t"
            "movq     16(%0, %%""rax"", 4), %%mm2  \n\t"
            "movq     24(%0, %%""rax"", 4), %%mm3  \n\t"
            "psrlw                  $8, %%mm0   \n\t"
            "psrlw                  $8, %%mm1   \n\t"
            "psrlw                  $8, %%mm2   \n\t"
            "psrlw                  $8, %%mm3   \n\t"
            "packuswb            %%mm1, %%mm0   \n\t"
            "packuswb            %%mm3, %%mm2   \n\t"
            "movntq""              %%mm0,  (%1, %%""rax"", 2) \n\t"
            "movntq""              %%mm2, 8(%1, %%""rax"", 2) \n\t"
            "add                    $8, %%""rax"" \n\t"
            "cmp                    %4, %%""rax"" \n\t"
            " jb                    1b               \n\t"
            ::"r"(src), "r"(ydst), "r"(udst), "r"(vdst), "g" (chromWidth)
            : "memory", "%""rax"
        );
        udst += chromStride;
        vdst += chromStride;
        ydst += lumStride;
        src += srcStride;
    }
    __asm__ volatile("emms""       \n\t"
                     "sfence""     \n\t"
                     :::"memory");
}
static inline void rgb24toyv12_mmxext(const uint8_t *src, uint8_t *ydst, uint8_t *udst, uint8_t *vdst,
                                       int width, int height,
                                       int lumStride, int chromStride, int srcStride,
                                       int32_t *rgb2yuv)
{
#define BGR2Y_IDX "16*4+16*32"
#define BGR2U_IDX "16*4+16*33"
#define BGR2V_IDX "16*4+16*34"
    int y;
    const x86_reg chromWidth= width>>1;
    if (height > 2) {
        ff_rgb24toyv12_c(src, ydst, udst, vdst, width, 2, lumStride, chromStride, srcStride, rgb2yuv);
        src += 2*srcStride;
        ydst += 2*lumStride;
        udst += chromStride;
        vdst += chromStride;
        height -= 2;
    }
    for (y=0; y<height-2; y+=2) {
        int i;
        for (i=0; i<2; i++) {
            __asm__ volatile(
                "mov                        %2, %%""rax""\n\t"
                "movq          ""16*4+16*32""(%3), %%mm6       \n\t"
                "movq       """ "ff_w1111"", %%mm5       \n\t"
                "pxor                    %%mm7, %%mm7       \n\t"
                "lea (%%""rax"", %%""rax"", 2), %%""rdx"" \n\t"
                ".p2align                    4              \n\t"
                "1:                                         \n\t"
                "prefetchnta"" 64(%0, %%""rdx"")              \n\t"
                "movd       (%0, %%""rdx""), %%mm0       \n\t"
                "movd      3(%0, %%""rdx""), %%mm1       \n\t"
                "punpcklbw               %%mm7, %%mm0       \n\t"
                "punpcklbw               %%mm7, %%mm1       \n\t"
                "movd      6(%0, %%""rdx""), %%mm2       \n\t"
                "movd      9(%0, %%""rdx""), %%mm3       \n\t"
                "punpcklbw               %%mm7, %%mm2       \n\t"
                "punpcklbw               %%mm7, %%mm3       \n\t"
                "pmaddwd                 %%mm6, %%mm0       \n\t"
                "pmaddwd                 %%mm6, %%mm1       \n\t"
                "pmaddwd                 %%mm6, %%mm2       \n\t"
                "pmaddwd                 %%mm6, %%mm3       \n\t"
                "psrad                      $8, %%mm0       \n\t"
                "psrad                      $8, %%mm1       \n\t"
                "psrad                      $8, %%mm2       \n\t"
                "psrad                      $8, %%mm3       \n\t"
                "packssdw                %%mm1, %%mm0       \n\t"
                "packssdw                %%mm3, %%mm2       \n\t"
                "pmaddwd                 %%mm5, %%mm0       \n\t"
                "pmaddwd                 %%mm5, %%mm2       \n\t"
                "packssdw                %%mm2, %%mm0       \n\t"
                "psraw                      $7, %%mm0       \n\t"
                "movd     12(%0, %%""rdx""), %%mm4       \n\t"
                "movd     15(%0, %%""rdx""), %%mm1       \n\t"
                "punpcklbw               %%mm7, %%mm4       \n\t"
                "punpcklbw               %%mm7, %%mm1       \n\t"
                "movd     18(%0, %%""rdx""), %%mm2       \n\t"
                "movd     21(%0, %%""rdx""), %%mm3       \n\t"
                "punpcklbw               %%mm7, %%mm2       \n\t"
                "punpcklbw               %%mm7, %%mm3       \n\t"
                "pmaddwd                 %%mm6, %%mm4       \n\t"
                "pmaddwd                 %%mm6, %%mm1       \n\t"
                "pmaddwd                 %%mm6, %%mm2       \n\t"
                "pmaddwd                 %%mm6, %%mm3       \n\t"
                "psrad                      $8, %%mm4       \n\t"
                "psrad                      $8, %%mm1       \n\t"
                "psrad                      $8, %%mm2       \n\t"
                "psrad                      $8, %%mm3       \n\t"
                "packssdw                %%mm1, %%mm4       \n\t"
                "packssdw                %%mm3, %%mm2       \n\t"
                "pmaddwd                 %%mm5, %%mm4       \n\t"
                "pmaddwd                 %%mm5, %%mm2       \n\t"
                "add                       $24, %%""rdx""\n\t"
                "packssdw                %%mm2, %%mm4       \n\t"
                "psraw                      $7, %%mm4       \n\t"
                "packuswb                %%mm4, %%mm0       \n\t"
                "paddusb """ "ff_bgr2YOffset"", %%mm0    \n\t"
                "movntq""                  %%mm0, (%1, %%""rax"") \n\t"
                "add                        $8,      %%""rax""  \n\t"
                " js                        1b                     \n\t"
                : : "r" (src+width*3), "r" (ydst+width), "g" ((x86_reg)-width), "r"(rgb2yuv)
                 
                : "%""rax", "%""rdx"
            );
            ydst += lumStride;
            src += srcStride;
        }
        src -= srcStride*2;
        __asm__ volatile(
            "mov                        %4, %%""rax""\n\t"
            "movq       """ "ff_w1111"", %%mm5       \n\t"
            "movq          ""16*4+16*33""(%5), %%mm6       \n\t"
            "pxor                    %%mm7, %%mm7       \n\t"
            "lea (%%""rax"", %%""rax"", 2), %%""rdx"" \n\t"
            "add              %%""rdx"", %%""rdx""\n\t"
            ".p2align                    4              \n\t"
            "1:                                         \n\t"
            "prefetchnta"" 64(%0, %%""rdx"")              \n\t"
            "prefetchnta"" 64(%1, %%""rdx"")              \n\t"
            "movq       (%0, %%""rdx""), %%mm0       \n\t"
            "movq       (%1, %%""rdx""), %%mm1       \n\t"
            "movq      6(%0, %%""rdx""), %%mm2       \n\t"
            "movq      6(%1, %%""rdx""), %%mm3       \n\t"
            "pavgb""                   %%mm1, %%mm0       \n\t"
            "pavgb""                   %%mm3, %%mm2       \n\t"
            "movq                    %%mm0, %%mm1       \n\t"
            "movq                    %%mm2, %%mm3       \n\t"
            "psrlq                     $24, %%mm0       \n\t"
            "psrlq                     $24, %%mm2       \n\t"
            "pavgb""                   %%mm1, %%mm0       \n\t"
            "pavgb""                   %%mm3, %%mm2       \n\t"
            "punpcklbw               %%mm7, %%mm0       \n\t"
            "punpcklbw               %%mm7, %%mm2       \n\t"
            "movq          ""16*4+16*34""(%5), %%mm1       \n\t"
            "movq          ""16*4+16*34""(%5), %%mm3       \n\t"
            "pmaddwd                 %%mm0, %%mm1       \n\t"
            "pmaddwd                 %%mm2, %%mm3       \n\t"
            "pmaddwd                 %%mm6, %%mm0       \n\t"
            "pmaddwd                 %%mm6, %%mm2       \n\t"
            "psrad                      $8, %%mm0       \n\t"
            "psrad                      $8, %%mm1       \n\t"
            "psrad                      $8, %%mm2       \n\t"
            "psrad                      $8, %%mm3       \n\t"
            "packssdw                %%mm2, %%mm0       \n\t"
            "packssdw                %%mm3, %%mm1       \n\t"
            "pmaddwd                 %%mm5, %%mm0       \n\t"
            "pmaddwd                 %%mm5, %%mm1       \n\t"
            "packssdw                %%mm1, %%mm0       \n\t"
            "psraw                      $7, %%mm0       \n\t"
            "movq     12(%0, %%""rdx""), %%mm4       \n\t"
            "movq     12(%1, %%""rdx""), %%mm1       \n\t"
            "movq     18(%0, %%""rdx""), %%mm2       \n\t"
            "movq     18(%1, %%""rdx""), %%mm3       \n\t"
            "pavgb""                   %%mm1, %%mm4       \n\t"
            "pavgb""                   %%mm3, %%mm2       \n\t"
            "movq                    %%mm4, %%mm1       \n\t"
            "movq                    %%mm2, %%mm3       \n\t"
            "psrlq                     $24, %%mm4       \n\t"
            "psrlq                     $24, %%mm2       \n\t"
            "pavgb""                   %%mm1, %%mm4       \n\t"
            "pavgb""                   %%mm3, %%mm2       \n\t"
            "punpcklbw               %%mm7, %%mm4       \n\t"
            "punpcklbw               %%mm7, %%mm2       \n\t"
            "movq          ""16*4+16*34""(%5), %%mm1       \n\t"
            "movq          ""16*4+16*34""(%5), %%mm3       \n\t"
            "pmaddwd                 %%mm4, %%mm1       \n\t"
            "pmaddwd                 %%mm2, %%mm3       \n\t"
            "pmaddwd                 %%mm6, %%mm4       \n\t"
            "pmaddwd                 %%mm6, %%mm2       \n\t"
            "psrad                      $8, %%mm4       \n\t"
            "psrad                      $8, %%mm1       \n\t"
            "psrad                      $8, %%mm2       \n\t"
            "psrad                      $8, %%mm3       \n\t"
            "packssdw                %%mm2, %%mm4       \n\t"
            "packssdw                %%mm3, %%mm1       \n\t"
            "pmaddwd                 %%mm5, %%mm4       \n\t"
            "pmaddwd                 %%mm5, %%mm1       \n\t"
            "add                       $24, %%""rdx""\n\t"
            "packssdw                %%mm1, %%mm4       \n\t"
            "psraw                      $7, %%mm4       \n\t"
            "movq                    %%mm0, %%mm1           \n\t"
            "punpckldq               %%mm4, %%mm0           \n\t"
            "punpckhdq               %%mm4, %%mm1           \n\t"
            "packsswb                %%mm1, %%mm0           \n\t"
            "paddb """ "ff_bgr2UVOffset"", %%mm0         \n\t"
            "movd                    %%mm0, (%2, %%""rax"") \n\t"
            "punpckhdq               %%mm0, %%mm0              \n\t"
            "movd                    %%mm0, (%3, %%""rax"") \n\t"
            "add                        $4, %%""rax""       \n\t"
            " js                        1b              \n\t"
            : : "r" (src+chromWidth*6), "r" (src+srcStride+chromWidth*6), "r" (udst+chromWidth), "r" (vdst+chromWidth), "g" (-chromWidth), "r"(rgb2yuv)
             
            : "%""rax", "%""rdx"
        );
        udst += chromStride;
        vdst += chromStride;
        src += srcStride*2;
    }
    __asm__ volatile("emms""       \n\t"
                     "sfence""     \n\t"
                     :::"memory");
     ff_rgb24toyv12_c(src, ydst, udst, vdst, width, height-y, lumStride, chromStride, srcStride, rgb2yuv);
}

static void interleaveBytes_mmxext(const uint8_t *src1, const uint8_t *src2, uint8_t *dest,
                                    int width, int height, int src1Stride,
                                    int src2Stride, int dstStride)
{
    int h;
    for (h=0; h < height; h++) {
        int w;
        if (width >= 16) {
        __asm__(
            "xor %%""rax"", %%""rax""         \n\t"
            "1:                                     \n\t"
            "prefetchnta"" 64(%1, %%""rax"")          \n\t"
            "prefetchnta"" 64(%2, %%""rax"")          \n\t"
            "movq    (%1, %%""rax""), %%mm0      \n\t"
            "movq   8(%1, %%""rax""), %%mm2      \n\t"
            "movq                 %%mm0, %%mm1      \n\t"
            "movq                 %%mm2, %%mm3      \n\t"
            "movq    (%2, %%""rax""), %%mm4      \n\t"
            "movq   8(%2, %%""rax""), %%mm5      \n\t"
            "punpcklbw            %%mm4, %%mm0      \n\t"
            "punpckhbw            %%mm4, %%mm1      \n\t"
            "punpcklbw            %%mm5, %%mm2      \n\t"
            "punpckhbw            %%mm5, %%mm3      \n\t"
            "movntq""               %%mm0,   (%0, %%""rax"", 2) \n\t"
            "movntq""               %%mm1,  8(%0, %%""rax"", 2) \n\t"
            "movntq""               %%mm2, 16(%0, %%""rax"", 2) \n\t"
            "movntq""               %%mm3, 24(%0, %%""rax"", 2) \n\t"
            "add                    $16, %%""rax""            \n\t"
            "cmp                     %3, %%""rax""            \n\t"
            " jb                     1b                          \n\t"
            ::"r"(dest), "r"(src1), "r"(src2), "r" ((x86_reg)width-15)
            : "memory", "%""rax"
        );
        }
        for (w= (width&(~15)); w < width; w++) {
            dest[2*w+0] = src1[w];
            dest[2*w+1] = src2[w];
        }
        dest += dstStride;
        src1 += src1Stride;
        src2 += src2Stride;
    }
    __asm__(
            "emms""       \n\t"
            "sfence""     \n\t"
            ::: "memory"
            );
}
static inline void vu9_to_vu12_mmxext(const uint8_t *src1, const uint8_t *src2,
                                       uint8_t *dst1, uint8_t *dst2,
                                       int width, int height,
                                       int srcStride1, int srcStride2,
                                       int dstStride1, int dstStride2)
{
    x86_reg x, y;
    int w,h;
    w=width/2; h=height/2;
    __asm__ volatile(
        "prefetchnta"" %0    \n\t"
        "prefetchnta"" %1    \n\t"
        ::"m"(*(src1+srcStride1)),"m"(*(src2+srcStride2)):"memory");
    for (y=0;y<h;y++) {
        const uint8_t* s1=src1+srcStride1*(y>>1);
        uint8_t* d=dst1+dstStride1*y;
        x=0;
        for (;x<w-31;x+=32) {
            __asm__ volatile(
                "prefetchnta""   32(%1,%2)        \n\t"
                "movq         (%1,%2), %%mm0 \n\t"
                "movq        8(%1,%2), %%mm2 \n\t"
                "movq       16(%1,%2), %%mm4 \n\t"
                "movq       24(%1,%2), %%mm6 \n\t"
                "movq      %%mm0, %%mm1 \n\t"
                "movq      %%mm2, %%mm3 \n\t"
                "movq      %%mm4, %%mm5 \n\t"
                "movq      %%mm6, %%mm7 \n\t"
                "punpcklbw %%mm0, %%mm0 \n\t"
                "punpckhbw %%mm1, %%mm1 \n\t"
                "punpcklbw %%mm2, %%mm2 \n\t"
                "punpckhbw %%mm3, %%mm3 \n\t"
                "punpcklbw %%mm4, %%mm4 \n\t"
                "punpckhbw %%mm5, %%mm5 \n\t"
                "punpcklbw %%mm6, %%mm6 \n\t"
                "punpckhbw %%mm7, %%mm7 \n\t"
                "movntq""    %%mm0,   (%0,%2,2)  \n\t"
                "movntq""    %%mm1,  8(%0,%2,2)  \n\t"
                "movntq""    %%mm2, 16(%0,%2,2)  \n\t"
                "movntq""    %%mm3, 24(%0,%2,2)  \n\t"
                "movntq""    %%mm4, 32(%0,%2,2)  \n\t"
                "movntq""    %%mm5, 40(%0,%2,2)  \n\t"
                "movntq""    %%mm6, 48(%0,%2,2)  \n\t"
                "movntq""    %%mm7, 56(%0,%2,2)"
                :: "r"(d), "r"(s1), "r"(x)
                :"memory");
        }
        for (;x<w;x++) d[2*x]=d[2*x+1]=s1[x];
    }
    for (y=0;y<h;y++) {
        const uint8_t* s2=src2+srcStride2*(y>>1);
        uint8_t* d=dst2+dstStride2*y;
        x=0;
        for (;x<w-31;x+=32) {
            __asm__ volatile(
                "prefetchnta""   32(%1,%2)        \n\t"
                "movq         (%1,%2), %%mm0 \n\t"
                "movq        8(%1,%2), %%mm2 \n\t"
                "movq       16(%1,%2), %%mm4 \n\t"
                "movq       24(%1,%2), %%mm6 \n\t"
                "movq      %%mm0, %%mm1 \n\t"
                "movq      %%mm2, %%mm3 \n\t"
                "movq      %%mm4, %%mm5 \n\t"
                "movq      %%mm6, %%mm7 \n\t"
                "punpcklbw %%mm0, %%mm0 \n\t"
                "punpckhbw %%mm1, %%mm1 \n\t"
                "punpcklbw %%mm2, %%mm2 \n\t"
                "punpckhbw %%mm3, %%mm3 \n\t"
                "punpcklbw %%mm4, %%mm4 \n\t"
                "punpckhbw %%mm5, %%mm5 \n\t"
                "punpcklbw %%mm6, %%mm6 \n\t"
                "punpckhbw %%mm7, %%mm7 \n\t"
                "movntq""    %%mm0,   (%0,%2,2)  \n\t"
                "movntq""    %%mm1,  8(%0,%2,2)  \n\t"
                "movntq""    %%mm2, 16(%0,%2,2)  \n\t"
                "movntq""    %%mm3, 24(%0,%2,2)  \n\t"
                "movntq""    %%mm4, 32(%0,%2,2)  \n\t"
                "movntq""    %%mm5, 40(%0,%2,2)  \n\t"
                "movntq""    %%mm6, 48(%0,%2,2)  \n\t"
                "movntq""    %%mm7, 56(%0,%2,2)"
                :: "r"(d), "r"(s2), "r"(x)
                :"memory");
        }
        for (;x<w;x++) d[2*x]=d[2*x+1]=s2[x];
    }
    __asm__(
            "emms""       \n\t"
            "sfence""     \n\t"
            ::: "memory"
        );
}

static inline void yvu9_to_yuy2_mmxext(const uint8_t *src1, const uint8_t *src2, const uint8_t *src3,
                                        uint8_t *dst,
                                        int width, int height,
                                        int srcStride1, int srcStride2,
                                        int srcStride3, int dstStride)
{
    x86_reg x;
    int y,w,h;
    w=width/2; h=height;
    for (y=0;y<h;y++) {
        const uint8_t* yp=src1+srcStride1*y;
        const uint8_t* up=src2+srcStride2*(y>>2);
        const uint8_t* vp=src3+srcStride3*(y>>2);
        uint8_t* d=dst+dstStride*y;
        x=0;
        for (;x<w-7;x+=8) {
            __asm__ volatile(
                "prefetchnta""   32(%1, %0)          \n\t"
                "prefetchnta""   32(%2, %0)          \n\t"
                "prefetchnta""   32(%3, %0)          \n\t"
                "movq      (%1, %0, 4), %%mm0   \n\t"
                "movq         (%2, %0), %%mm1   \n\t"
                "movq         (%3, %0), %%mm2   \n\t"
                "movq            %%mm0, %%mm3   \n\t"
                "movq            %%mm1, %%mm4   \n\t"
                "movq            %%mm2, %%mm5   \n\t"
                "punpcklbw       %%mm1, %%mm1   \n\t"
                "punpcklbw       %%mm2, %%mm2   \n\t"
                "punpckhbw       %%mm4, %%mm4   \n\t"
                "punpckhbw       %%mm5, %%mm5   \n\t"
                "movq            %%mm1, %%mm6   \n\t"
                "punpcklbw       %%mm2, %%mm1   \n\t"
                "punpcklbw       %%mm1, %%mm0   \n\t"
                "punpckhbw       %%mm1, %%mm3   \n\t"
                "movntq""          %%mm0,  (%4, %0, 8)    \n\t"
                "movntq""          %%mm3, 8(%4, %0, 8)    \n\t"
                "punpckhbw       %%mm2, %%mm6   \n\t"
                "movq     8(%1, %0, 4), %%mm0   \n\t"
                "movq            %%mm0, %%mm3   \n\t"
                "punpcklbw       %%mm6, %%mm0   \n\t"
                "punpckhbw       %%mm6, %%mm3   \n\t"
                "movntq""          %%mm0, 16(%4, %0, 8)   \n\t"
                "movntq""          %%mm3, 24(%4, %0, 8)   \n\t"
                "movq            %%mm4, %%mm6   \n\t"
                "movq    16(%1, %0, 4), %%mm0   \n\t"
                "movq            %%mm0, %%mm3   \n\t"
                "punpcklbw       %%mm5, %%mm4   \n\t"
                "punpcklbw       %%mm4, %%mm0   \n\t"
                "punpckhbw       %%mm4, %%mm3   \n\t"
                "movntq""          %%mm0, 32(%4, %0, 8)   \n\t"
                "movntq""          %%mm3, 40(%4, %0, 8)   \n\t"
                "punpckhbw       %%mm5, %%mm6   \n\t"
                "movq    24(%1, %0, 4), %%mm0   \n\t"
                "movq            %%mm0, %%mm3   \n\t"
                "punpcklbw       %%mm6, %%mm0   \n\t"
                "punpckhbw       %%mm6, %%mm3   \n\t"
                "movntq""          %%mm0, 48(%4, %0, 8)   \n\t"
                "movntq""          %%mm3, 56(%4, %0, 8)   \n\t"
                : "+r" (x)
                : "r"(yp), "r" (up), "r"(vp), "r"(d)
                :"memory");
        }
        for (; x<w; x++) {
            const int x2 = x<<2;
            d[8*x+0] = yp[x2];
            d[8*x+1] = up[x];
            d[8*x+2] = yp[x2+1];
            d[8*x+3] = vp[x];
            d[8*x+4] = yp[x2+2];
            d[8*x+5] = up[x];
            d[8*x+6] = yp[x2+3];
            d[8*x+7] = vp[x];
        }
    }
    __asm__(
            "emms""       \n\t"
            "sfence""     \n\t"
            ::: "memory"
        );
}

static void extract_even_mmxext(const uint8_t *src, uint8_t *dst, x86_reg count)
{
    dst += count;
    src += 2*count;
    count= - count;
    if(count <= -16) {
        count += 15;
        __asm__ volatile(
            "pcmpeqw       %%mm7, %%mm7        \n\t"
            "psrlw            $8, %%mm7        \n\t"
            "1:                                \n\t"
            "movq -30(%1, %0, 2), %%mm0        \n\t"
            "movq -22(%1, %0, 2), %%mm1        \n\t"
            "movq -14(%1, %0, 2), %%mm2        \n\t"
            "movq  -6(%1, %0, 2), %%mm3        \n\t"
            "pand          %%mm7, %%mm0        \n\t"
            "pand          %%mm7, %%mm1        \n\t"
            "pand          %%mm7, %%mm2        \n\t"
            "pand          %%mm7, %%mm3        \n\t"
            "packuswb      %%mm1, %%mm0        \n\t"
            "packuswb      %%mm3, %%mm2        \n\t"
            "movntq""        %%mm0,-15(%2, %0)   \n\t"
            "movntq""        %%mm2,- 7(%2, %0)   \n\t"
            "add             $16, %0           \n\t"
            " js 1b                            \n\t"
            : "+r"(count)
            : "r"(src), "r"(dst)
        );
        count -= 15;
    }
    while(count<0) {
        dst[count]= src[2*count];
        count++;
    }
}

static void extract_odd_mmxext(const uint8_t *src, uint8_t *dst, x86_reg count)
{
    src ++;
    dst += count;
    src += 2*count;
    count= - count;
    if(count < -16) {
        count += 16;
        __asm__ volatile(
            "pcmpeqw       %%mm7, %%mm7        \n\t"
            "psrlw            $8, %%mm7        \n\t"
            "1:                                \n\t"
            "movq -32(%1, %0, 2), %%mm0        \n\t"
            "movq -24(%1, %0, 2), %%mm1        \n\t"
            "movq -16(%1, %0, 2), %%mm2        \n\t"
            "movq  -8(%1, %0, 2), %%mm3        \n\t"
            "pand          %%mm7, %%mm0        \n\t"
            "pand          %%mm7, %%mm1        \n\t"
            "pand          %%mm7, %%mm2        \n\t"
            "pand          %%mm7, %%mm3        \n\t"
            "packuswb      %%mm1, %%mm0        \n\t"
            "packuswb      %%mm3, %%mm2        \n\t"
            "movntq""        %%mm0,-16(%2, %0)   \n\t"
            "movntq""        %%mm2,- 8(%2, %0)   \n\t"
            "add             $16, %0           \n\t"
            " js 1b                            \n\t"
            : "+r"(count)
            : "r"(src), "r"(dst)
        );
        count -= 16;
    }
    while(count<0) {
        dst[count]= src[2*count];
        count++;
    }
}

static void extract_even2_mmxext(const uint8_t *src, uint8_t *dst0, uint8_t *dst1, x86_reg count)
{
    dst0+= count;
    dst1+= count;
    src += 4*count;
    count= - count;
    if(count <= -8) {
        count += 7;
        __asm__ volatile(
            "pcmpeqw       %%mm7, %%mm7        \n\t"
            "psrlw            $8, %%mm7        \n\t"
            "1:                                \n\t"
            "movq -28(%1, %0, 4), %%mm0        \n\t"
            "movq -20(%1, %0, 4), %%mm1        \n\t"
            "movq -12(%1, %0, 4), %%mm2        \n\t"
            "movq  -4(%1, %0, 4), %%mm3        \n\t"
            "pand          %%mm7, %%mm0        \n\t"
            "pand          %%mm7, %%mm1        \n\t"
            "pand          %%mm7, %%mm2        \n\t"
            "pand          %%mm7, %%mm3        \n\t"
            "packuswb      %%mm1, %%mm0        \n\t"
            "packuswb      %%mm3, %%mm2        \n\t"
            "movq          %%mm0, %%mm1        \n\t"
            "movq          %%mm2, %%mm3        \n\t"
            "psrlw            $8, %%mm0        \n\t"
            "psrlw            $8, %%mm2        \n\t"
            "pand          %%mm7, %%mm1        \n\t"
            "pand          %%mm7, %%mm3        \n\t"
            "packuswb      %%mm2, %%mm0        \n\t"
            "packuswb      %%mm3, %%mm1        \n\t"
            "movntq""        %%mm0,- 7(%3, %0)   \n\t"
            "movntq""        %%mm1,- 7(%2, %0)   \n\t"
            "add              $8, %0           \n\t"
            " js 1b                            \n\t"
            : "+r"(count)
            : "r"(src), "r"(dst0), "r"(dst1)
        );
        count -= 7;
    }
    while(count<0) {
        dst0[count]= src[4*count+0];
        dst1[count]= src[4*count+2];
        count++;
    }
}

static void extract_even2avg_mmxext(const uint8_t *src0, const uint8_t *src1, uint8_t *dst0, uint8_t *dst1, x86_reg count)
{
    dst0 += count;
    dst1 += count;
    src0 += 4*count;
    src1 += 4*count;
    count= - count;
    if(count <= -8) {
        count += 7;
        __asm__ volatile(
            "pcmpeqw        %%mm7, %%mm7        \n\t"
            "psrlw             $8, %%mm7        \n\t"
            "1:                                \n\t"
            "movq  -28(%1, %0, 4), %%mm0        \n\t"
            "movq  -20(%1, %0, 4), %%mm1        \n\t"
            "movq  -12(%1, %0, 4), %%mm2        \n\t"
            "movq   -4(%1, %0, 4), %%mm3        \n\t"
            "pavgb"" -28(%2, %0, 4), %%mm0        \n\t"
            "pavgb"" -20(%2, %0, 4), %%mm1        \n\t"
            "pavgb"" -12(%2, %0, 4), %%mm2        \n\t"
            "pavgb"" - 4(%2, %0, 4), %%mm3        \n\t"
            "pand           %%mm7, %%mm0        \n\t"
            "pand           %%mm7, %%mm1        \n\t"
            "pand           %%mm7, %%mm2        \n\t"
            "pand           %%mm7, %%mm3        \n\t"
            "packuswb       %%mm1, %%mm0        \n\t"
            "packuswb       %%mm3, %%mm2        \n\t"
            "movq           %%mm0, %%mm1        \n\t"
            "movq           %%mm2, %%mm3        \n\t"
            "psrlw             $8, %%mm0        \n\t"
            "psrlw             $8, %%mm2        \n\t"
            "pand           %%mm7, %%mm1        \n\t"
            "pand           %%mm7, %%mm3        \n\t"
            "packuswb       %%mm2, %%mm0        \n\t"
            "packuswb       %%mm3, %%mm1        \n\t"
            "movntq""         %%mm0,- 7(%4, %0)   \n\t"
            "movntq""         %%mm1,- 7(%3, %0)   \n\t"
            "add               $8, %0           \n\t"
            " js 1b                            \n\t"
            : "+r"(count)
            : "r"(src0), "r"(src1), "r"(dst0), "r"(dst1)
        );
        count -= 7;
    }
    while(count<0) {
        dst0[count]= (src0[4*count+0]+src1[4*count+0])>>1;
        dst1[count]= (src0[4*count+2]+src1[4*count+2])>>1;
        count++;
    }
}

static void extract_odd2_mmxext(const uint8_t *src, uint8_t *dst0, uint8_t *dst1, x86_reg count)
{
    dst0+= count;
    dst1+= count;
    src += 4*count;
    count= - count;
    if(count <= -8) {
        count += 7;
        __asm__ volatile(
            "pcmpeqw       %%mm7, %%mm7        \n\t"
            "psrlw            $8, %%mm7        \n\t"
            "1:                                \n\t"
            "movq -28(%1, %0, 4), %%mm0        \n\t"
            "movq -20(%1, %0, 4), %%mm1        \n\t"
            "movq -12(%1, %0, 4), %%mm2        \n\t"
            "movq  -4(%1, %0, 4), %%mm3        \n\t"
            "psrlw            $8, %%mm0        \n\t"
            "psrlw            $8, %%mm1        \n\t"
            "psrlw            $8, %%mm2        \n\t"
            "psrlw            $8, %%mm3        \n\t"
            "packuswb      %%mm1, %%mm0        \n\t"
            "packuswb      %%mm3, %%mm2        \n\t"
            "movq          %%mm0, %%mm1        \n\t"
            "movq          %%mm2, %%mm3        \n\t"
            "psrlw            $8, %%mm0        \n\t"
            "psrlw            $8, %%mm2        \n\t"
            "pand          %%mm7, %%mm1        \n\t"
            "pand          %%mm7, %%mm3        \n\t"
            "packuswb      %%mm2, %%mm0        \n\t"
            "packuswb      %%mm3, %%mm1        \n\t"
            "movntq""        %%mm0,- 7(%3, %0)   \n\t"
            "movntq""        %%mm1,- 7(%2, %0)   \n\t"
            "add              $8, %0           \n\t"
            " js 1b                            \n\t"
            : "+r"(count)
            : "r"(src), "r"(dst0), "r"(dst1)
        );
        count -= 7;
    }
    src++;
    while(count<0) {
        dst0[count]= src[4*count+0];
        dst1[count]= src[4*count+2];
        count++;
    }
}

static void extract_odd2avg_mmxext(const uint8_t *src0, const uint8_t *src1, uint8_t *dst0, uint8_t *dst1, x86_reg count)
{
    dst0 += count;
    dst1 += count;
    src0 += 4*count;
    src1 += 4*count;
    count= - count;
    if(count <= -8) {
        count += 7;
        __asm__ volatile(
            "pcmpeqw        %%mm7, %%mm7        \n\t"
            "psrlw             $8, %%mm7        \n\t"
            "1:                                \n\t"
            "movq  -28(%1, %0, 4), %%mm0        \n\t"
            "movq  -20(%1, %0, 4), %%mm1        \n\t"
            "movq  -12(%1, %0, 4), %%mm2        \n\t"
            "movq   -4(%1, %0, 4), %%mm3        \n\t"
            "pavgb"" -28(%2, %0, 4), %%mm0        \n\t"
            "pavgb"" -20(%2, %0, 4), %%mm1        \n\t"
            "pavgb"" -12(%2, %0, 4), %%mm2        \n\t"
            "pavgb"" - 4(%2, %0, 4), %%mm3        \n\t"
            "psrlw             $8, %%mm0        \n\t"
            "psrlw             $8, %%mm1        \n\t"
            "psrlw             $8, %%mm2        \n\t"
            "psrlw             $8, %%mm3        \n\t"
            "packuswb       %%mm1, %%mm0        \n\t"
            "packuswb       %%mm3, %%mm2        \n\t"
            "movq           %%mm0, %%mm1        \n\t"
            "movq           %%mm2, %%mm3        \n\t"
            "psrlw             $8, %%mm0        \n\t"
            "psrlw             $8, %%mm2        \n\t"
            "pand           %%mm7, %%mm1        \n\t"
            "pand           %%mm7, %%mm3        \n\t"
            "packuswb       %%mm2, %%mm0        \n\t"
            "packuswb       %%mm3, %%mm1        \n\t"
            "movntq""         %%mm0,- 7(%4, %0)   \n\t"
            "movntq""         %%mm1,- 7(%3, %0)   \n\t"
            "add               $8, %0           \n\t"
            " js 1b                            \n\t"
            : "+r"(count)
            : "r"(src0), "r"(src1), "r"(dst0), "r"(dst1)
        );
        count -= 7;
    }
    src0++;
    src1++;
    while(count<0) {
        dst0[count]= (src0[4*count+0]+src1[4*count+0])>>1;
        dst1[count]= (src0[4*count+2]+src1[4*count+2])>>1;
        count++;
    }
}

static void yuyvtoyuv420_mmxext(uint8_t *ydst, uint8_t *udst, uint8_t *vdst, const uint8_t *src,
                                 int width, int height,
                                 int lumStride, int chromStride, int srcStride)
{
    int y;
    const int chromWidth = (!__builtin_constant_p(1) ? -((-(width)) >> (1)) : ((width) + (1<<(1)) - 1) >> (1));
    for (y=0; y<height; y++) {
        extract_even_mmxext(src, ydst, width);
        if(y&1) {
            extract_odd2avg_mmxext(src-srcStride, src, udst, vdst, chromWidth);
            udst+= chromStride;
            vdst+= chromStride;
        }
        src += srcStride;
        ydst+= lumStride;
    }
    __asm__(
            "emms""       \n\t"
            "sfence""     \n\t"
            ::: "memory"
        );
}

static void yuyvtoyuv422_mmxext(uint8_t *ydst, uint8_t *udst, uint8_t *vdst, const uint8_t *src,
                                 int width, int height,
                                 int lumStride, int chromStride, int srcStride)
{
    int y;
    const int chromWidth = (!__builtin_constant_p(1) ? -((-(width)) >> (1)) : ((width) + (1<<(1)) - 1) >> (1));
    for (y=0; y<height; y++) {
        extract_even_mmxext(src, ydst, width);
        extract_odd2_mmxext(src, udst, vdst, chromWidth);
        src += srcStride;
        ydst+= lumStride;
        udst+= chromStride;
        vdst+= chromStride;
    }
    __asm__(
            "emms""       \n\t"
            "sfence""     \n\t"
            ::: "memory"
        );
}

static void uyvytoyuv420_mmxext(uint8_t *ydst, uint8_t *udst, uint8_t *vdst, const uint8_t *src,
                                 int width, int height,
                                 int lumStride, int chromStride, int srcStride)
{
    int y;
    const int chromWidth = (!__builtin_constant_p(1) ? -((-(width)) >> (1)) : ((width) + (1<<(1)) - 1) >> (1));
    for (y=0; y<height; y++) {
        extract_odd_mmxext(src, ydst, width);
        if(y&1) {
            extract_even2avg_mmxext(src-srcStride, src, udst, vdst, chromWidth);
            udst+= chromStride;
            vdst+= chromStride;
        }
        src += srcStride;
        ydst+= lumStride;
    }
    __asm__(
            "emms""       \n\t"
            "sfence""     \n\t"
            ::: "memory"
        );
}

static void uyvytoyuv422_mmxext(uint8_t *ydst, uint8_t *udst, uint8_t *vdst, const uint8_t *src,
                                 int width, int height,
                                 int lumStride, int chromStride, int srcStride)
{
    int y;
    const int chromWidth = (!__builtin_constant_p(1) ? -((-(width)) >> (1)) : ((width) + (1<<(1)) - 1) >> (1));
    for (y=0; y<height; y++) {
        extract_odd_mmxext(src, ydst, width);
        extract_even2_mmxext(src, udst, vdst, chromWidth);
        src += srcStride;
        ydst+= lumStride;
        udst+= chromStride;
        vdst+= chromStride;
    }
    __asm__(
            "emms""       \n\t"
            "sfence""     \n\t"
            ::: "memory"
        );
}

static void rgb2rgb_init_mmxext(void)
{
    rgb15to16 = rgb15to16_mmxext;
    rgb15tobgr24 = rgb15tobgr24_mmxext;
    rgb15to32 = rgb15to32_mmxext;
    rgb16tobgr24 = rgb16tobgr24_mmxext;
    rgb16to32 = rgb16to32_mmxext;
    rgb16to15 = rgb16to15_mmxext;
    rgb24tobgr16 = rgb24tobgr16_mmxext;
    rgb24tobgr15 = rgb24tobgr15_mmxext;
    rgb24tobgr32 = rgb24tobgr32_mmxext;
    rgb32to16 = rgb32to16_mmxext;
    rgb32to15 = rgb32to15_mmxext;
    rgb32tobgr24 = rgb32tobgr24_mmxext;
    rgb24to15 = rgb24to15_mmxext;
    rgb24to16 = rgb24to16_mmxext;
    rgb24tobgr24 = rgb24tobgr24_mmxext;
    rgb32tobgr16 = rgb32tobgr16_mmxext;
    rgb32tobgr15 = rgb32tobgr15_mmxext;
    yv12toyuy2 = yv12toyuy2_mmxext;
    yv12touyvy = yv12touyvy_mmxext;
    yuv422ptoyuy2 = yuv422ptoyuy2_mmxext;
    yuv422ptouyvy = yuv422ptouyvy_mmxext;
    yuy2toyv12 = yuy2toyv12_mmxext;
    vu9_to_vu12 = vu9_to_vu12_mmxext;
    yvu9_to_yuy2 = yvu9_to_yuy2_mmxext;
    uyvytoyuv422 = uyvytoyuv422_mmxext;
    yuyvtoyuv422 = yuyvtoyuv422_mmxext;
    planar2x = planar2x_mmxext;
    ff_rgb24toyv12 = rgb24toyv12_mmxext;
    yuyvtoyuv420 = yuyvtoyuv420_mmxext;
    uyvytoyuv420 = uyvytoyuv420_mmxext;
    interleaveBytes = interleaveBytes_mmxext;
}

static void interleaveBytes_sse2(const uint8_t *src1, const uint8_t *src2, uint8_t *dest,
                                    int width, int height, int src1Stride,
                                    int src2Stride, int dstStride)
{
    int h;
    for (h=0; h < height; h++) {
        int w;
        if (width >= 16) {
            if (!((((intptr_t)src1) | ((intptr_t)src2) | ((intptr_t)dest))&15)) {
        __asm__(
            "xor              %%""rax"", %%""rax""  \n\t"
            "1:                                     \n\t"
            "prefetchnta"" 64(%1, %%""rax"")          \n\t"
            "prefetchnta"" 64(%2, %%""rax"")          \n\t"
            "movdqa  (%1, %%""rax""), %%xmm0     \n\t"
            "movdqa  (%1, %%""rax""), %%xmm1     \n\t"
            "movdqa  (%2, %%""rax""), %%xmm2     \n\t"
            "punpcklbw           %%xmm2, %%xmm0     \n\t"
            "punpckhbw           %%xmm2, %%xmm1     \n\t"
            "movntdq             %%xmm0,   (%0, %%""rax"", 2) \n\t"
            "movntdq             %%xmm1, 16(%0, %%""rax"", 2) \n\t"
            "add                    $16, %%""rax""            \n\t"
            "cmp                     %3, %%""rax""            \n\t"
            " jb                     1b             \n\t"
            ::"r"(dest), "r"(src1), "r"(src2), "r" ((x86_reg)width-15)
            : "memory", "xmm0", "xmm1", "xmm2", "%""rax"
        );
            } else
        __asm__(
            "xor %%""rax"", %%""rax""         \n\t"
            "1:                                     \n\t"
            "prefetchnta"" 64(%1, %%""rax"")          \n\t"
            "prefetchnta"" 64(%2, %%""rax"")          \n\t"
            "movq    (%1, %%""rax""), %%mm0      \n\t"
            "movq   8(%1, %%""rax""), %%mm2      \n\t"
            "movq                 %%mm0, %%mm1      \n\t"
            "movq                 %%mm2, %%mm3      \n\t"
            "movq    (%2, %%""rax""), %%mm4      \n\t"
            "movq   8(%2, %%""rax""), %%mm5      \n\t"
            "punpcklbw            %%mm4, %%mm0      \n\t"
            "punpckhbw            %%mm4, %%mm1      \n\t"
            "punpcklbw            %%mm5, %%mm2      \n\t"
            "punpckhbw            %%mm5, %%mm3      \n\t"
            "movntq""               %%mm0,   (%0, %%""rax"", 2) \n\t"
            "movntq""               %%mm1,  8(%0, %%""rax"", 2) \n\t"
            "movntq""               %%mm2, 16(%0, %%""rax"", 2) \n\t"
            "movntq""               %%mm3, 24(%0, %%""rax"", 2) \n\t"
            "add                    $16, %%""rax""            \n\t"
            "cmp                     %3, %%""rax""            \n\t"
            " jb                     1b                          \n\t"
            ::"r"(dest), "r"(src1), "r"(src2), "r" ((x86_reg)width-15)
            : "memory", "%""rax"
        );
        }
        for (w= (width&(~15)); w < width; w++) {
            dest[2*w+0] = src1[w];
            dest[2*w+1] = src2[w];
        }
        dest += dstStride;
        src1 += src1Stride;
        src2 += src2Stride;
    }
    __asm__(
            "emms""       \n\t"
            "sfence""     \n\t"
            ::: "memory"
            );
}

void ff_nv12ToUV_sse2(uint8_t *dstU, uint8_t *dstV,
                         const uint8_t *unused,
                         const uint8_t *src1,
                         const uint8_t *src2,
                         int w,
                         uint32_t *unused2);
static void deinterleaveBytes_sse2(const uint8_t *src, uint8_t *dst1, uint8_t *dst2,
                                      int width, int height, int srcStride,
                                      int dst1Stride, int dst2Stride)
{
    int h;
    for (h = 0; h < height; h++) {
        ff_nv12ToUV_sse2(dst1, dst2, 
                                       ((void *)0)
                                           , src, 
                                                  ((void *)0)
                                                      , width, 
                                                               ((void *)0)
                                                                   );
        src += srcStride;
        dst1 += dst1Stride;
        dst2 += dst2Stride;
    }
    __asm__(
            "sfence""     \n\t"
            ::: "memory"
            );
}
static void rgb2rgb_init_sse2(void)
{
    interleaveBytes = interleaveBytes_sse2;
    deinterleaveBytes = deinterleaveBytes_sse2;
}

void ff_nv12ToUV_avx(uint8_t *dstU, uint8_t *dstV,
                         const uint8_t *unused,
                         const uint8_t *src1,
                         const uint8_t *src2,
                         int w,
                         uint32_t *unused2);
static void deinterleaveBytes_avx(const uint8_t *src, uint8_t *dst1, uint8_t *dst2,
                                      int width, int height, int srcStride,
                                      int dst1Stride, int dst2Stride)
{
    int h;
    for (h = 0; h < height; h++) {
        ff_nv12ToUV_avx(dst1, dst2, 
                                       ((void *)0)
                                           , src, 
                                                  ((void *)0)
                                                      , width, 
                                                               ((void *)0)
                                                                   );
        src += srcStride;
        dst1 += dst1Stride;
        dst2 += dst2Stride;
    }
    __asm__(
            "sfence""     \n\t"
            ::: "memory"
            );
}
static void rgb2rgb_init_avx(void)
{
    deinterleaveBytes = deinterleaveBytes_avx;
}

static inline void planar2x_3dnow(const uint8_t *src, uint8_t *dst, int srcWidth, int srcHeight, int srcStride, int dstStride)
{
    int x,y;
    dst[0]= src[0];
    for (x=0; x<srcWidth-1; x++) {
        dst[2*x+1]= (3*src[x] + src[x+1])>>2;
        dst[2*x+2]= ( src[x] + 3*src[x+1])>>2;
    }
    dst[2*srcWidth-1]= src[srcWidth-1];
    dst+= dstStride;
    for (y=1; y<srcHeight; y++) {
        x86_reg mmxSize= srcWidth&~15;
        if (mmxSize) {
        __asm__ volatile(
            "mov                       %4, %%""rax"" \n\t"
            "movq        """ "mmx_ff"", %%mm0    \n\t"
            "movq      (%0, %%""rax""), %%mm4    \n\t"
            "movq                   %%mm4, %%mm2    \n\t"
            "psllq                     $8, %%mm4    \n\t"
            "pand                   %%mm0, %%mm2    \n\t"
            "por                    %%mm2, %%mm4    \n\t"
            "movq      (%1, %%""rax""), %%mm5    \n\t"
            "movq                   %%mm5, %%mm3    \n\t"
            "psllq                     $8, %%mm5    \n\t"
            "pand                   %%mm0, %%mm3    \n\t"
            "por                    %%mm3, %%mm5    \n\t"
            "1:                                     \n\t"
            "movq      (%0, %%""rax""), %%mm0    \n\t"
            "movq      (%1, %%""rax""), %%mm1    \n\t"
            "movq     1(%0, %%""rax""), %%mm2    \n\t"
            "movq     1(%1, %%""rax""), %%mm3    \n\t"
            "pavgusb""                  %%mm0, %%mm5    \n\t"
            "pavgusb""                  %%mm0, %%mm3    \n\t"
            "pavgusb""                  %%mm0, %%mm5    \n\t"
            "pavgusb""                  %%mm0, %%mm3    \n\t"
            "pavgusb""                  %%mm1, %%mm4    \n\t"
            "pavgusb""                  %%mm1, %%mm2    \n\t"
            "pavgusb""                  %%mm1, %%mm4    \n\t"
            "pavgusb""                  %%mm1, %%mm2    \n\t"
            "movq                   %%mm5, %%mm7    \n\t"
            "movq                   %%mm4, %%mm6    \n\t"
            "punpcklbw              %%mm3, %%mm5    \n\t"
            "punpckhbw              %%mm3, %%mm7    \n\t"
            "punpcklbw              %%mm2, %%mm4    \n\t"
            "punpckhbw              %%mm2, %%mm6    \n\t"
            "movq""                 %%mm5,  (%2, %%""rax"", 2)  \n\t"
            "movq""                 %%mm7, 8(%2, %%""rax"", 2)  \n\t"
            "movq""                 %%mm4,  (%3, %%""rax"", 2)  \n\t"
            "movq""                 %%mm6, 8(%3, %%""rax"", 2)  \n\t"
            "add                       $8, %%""rax""            \n\t"
            "movq    -1(%0, %%""rax""), %%mm4    \n\t"
            "movq    -1(%1, %%""rax""), %%mm5    \n\t"
            " js                       1b           \n\t"
            :: "r" (src + mmxSize ), "r" (src + srcStride + mmxSize ),
               "r" (dst + mmxSize*2), "r" (dst + dstStride + mmxSize*2),
               "g" (-mmxSize)
              
            : "%""rax"
        );
        } else {
            mmxSize = 1;
            dst[0] = (src[0] * 3 + src[srcStride]) >> 2;
            dst[dstStride] = (src[0] + 3 * src[srcStride]) >> 2;
        }
        for (x=mmxSize-1; x<srcWidth-1; x++) {
            dst[2*x +1]= (3*src[x+0] + src[x+srcStride+1])>>2;
            dst[2*x+dstStride+2]= ( src[x+0] + 3*src[x+srcStride+1])>>2;
            dst[2*x+dstStride+1]= ( src[x+1] + 3*src[x+srcStride ])>>2;
            dst[2*x +2]= (3*src[x+1] + src[x+srcStride ])>>2;
        }
        dst[srcWidth*2 -1 ]= (3*src[srcWidth-1] + src[srcWidth-1 + srcStride])>>2;
        dst[srcWidth*2 -1 + dstStride]= ( src[srcWidth-1] + 3*src[srcWidth-1 + srcStride])>>2;
        dst+=dstStride*2;
        src+=srcStride;
    }
    dst[0]= src[0];
    for (x=0; x<srcWidth-1; x++) {
        dst[2*x+1]= (3*src[x] + src[x+1])>>2;
        dst[2*x+2]= ( src[x] + 3*src[x+1])>>2;
    }
    dst[2*srcWidth-1]= src[srcWidth-1];
    __asm__ volatile("femms""       \n\t"
                     " # nop""     \n\t"
                     :::"memory");
}
static inline void rgb24toyv12_3dnow(const uint8_t *src, uint8_t *ydst, uint8_t *udst, uint8_t *vdst,
                                       int width, int height,
                                       int lumStride, int chromStride, int srcStride,
                                       int32_t *rgb2yuv)
{
#define BGR2Y_IDX "16*4+16*32"
#define BGR2U_IDX "16*4+16*33"
#define BGR2V_IDX "16*4+16*34"
    int y;
    const x86_reg chromWidth= width>>1;
    if (height > 2) {
        ff_rgb24toyv12_c(src, ydst, udst, vdst, width, 2, lumStride, chromStride, srcStride, rgb2yuv);
        src += 2*srcStride;
        ydst += 2*lumStride;
        udst += chromStride;
        vdst += chromStride;
        height -= 2;
    }
    for (y=0; y<height-2; y+=2) {
        int i;
        for (i=0; i<2; i++) {
            __asm__ volatile(
                "mov                        %2, %%""rax""\n\t"
                "movq          ""16*4+16*32""(%3), %%mm6       \n\t"
                "movq       """ "ff_w1111"", %%mm5       \n\t"
                "pxor                    %%mm7, %%mm7       \n\t"
                "lea (%%""rax"", %%""rax"", 2), %%""rdx"" \n\t"
                ".p2align                    4              \n\t"
                "1:                                         \n\t"
                "prefetch"" 64(%0, %%""rdx"")              \n\t"
                "movd       (%0, %%""rdx""), %%mm0       \n\t"
                "movd      3(%0, %%""rdx""), %%mm1       \n\t"
                "punpcklbw               %%mm7, %%mm0       \n\t"
                "punpcklbw               %%mm7, %%mm1       \n\t"
                "movd      6(%0, %%""rdx""), %%mm2       \n\t"
                "movd      9(%0, %%""rdx""), %%mm3       \n\t"
                "punpcklbw               %%mm7, %%mm2       \n\t"
                "punpcklbw               %%mm7, %%mm3       \n\t"
                "pmaddwd                 %%mm6, %%mm0       \n\t"
                "pmaddwd                 %%mm6, %%mm1       \n\t"
                "pmaddwd                 %%mm6, %%mm2       \n\t"
                "pmaddwd                 %%mm6, %%mm3       \n\t"
                "psrad                      $8, %%mm0       \n\t"
                "psrad                      $8, %%mm1       \n\t"
                "psrad                      $8, %%mm2       \n\t"
                "psrad                      $8, %%mm3       \n\t"
                "packssdw                %%mm1, %%mm0       \n\t"
                "packssdw                %%mm3, %%mm2       \n\t"
                "pmaddwd                 %%mm5, %%mm0       \n\t"
                "pmaddwd                 %%mm5, %%mm2       \n\t"
                "packssdw                %%mm2, %%mm0       \n\t"
                "psraw                      $7, %%mm0       \n\t"
                "movd     12(%0, %%""rdx""), %%mm4       \n\t"
                "movd     15(%0, %%""rdx""), %%mm1       \n\t"
                "punpcklbw               %%mm7, %%mm4       \n\t"
                "punpcklbw               %%mm7, %%mm1       \n\t"
                "movd     18(%0, %%""rdx""), %%mm2       \n\t"
                "movd     21(%0, %%""rdx""), %%mm3       \n\t"
                "punpcklbw               %%mm7, %%mm2       \n\t"
                "punpcklbw               %%mm7, %%mm3       \n\t"
                "pmaddwd                 %%mm6, %%mm4       \n\t"
                "pmaddwd                 %%mm6, %%mm1       \n\t"
                "pmaddwd                 %%mm6, %%mm2       \n\t"
                "pmaddwd                 %%mm6, %%mm3       \n\t"
                "psrad                      $8, %%mm4       \n\t"
                "psrad                      $8, %%mm1       \n\t"
                "psrad                      $8, %%mm2       \n\t"
                "psrad                      $8, %%mm3       \n\t"
                "packssdw                %%mm1, %%mm4       \n\t"
                "packssdw                %%mm3, %%mm2       \n\t"
                "pmaddwd                 %%mm5, %%mm4       \n\t"
                "pmaddwd                 %%mm5, %%mm2       \n\t"
                "add                       $24, %%""rdx""\n\t"
                "packssdw                %%mm2, %%mm4       \n\t"
                "psraw                      $7, %%mm4       \n\t"
                "packuswb                %%mm4, %%mm0       \n\t"
                "paddusb """ "ff_bgr2YOffset"", %%mm0    \n\t"
                "movq""                  %%mm0, (%1, %%""rax"") \n\t"
                "add                        $8,      %%""rax""  \n\t"
                " js                        1b                     \n\t"
                : : "r" (src+width*3), "r" (ydst+width), "g" ((x86_reg)-width), "r"(rgb2yuv)
                 
                : "%""rax", "%""rdx"
            );
            ydst += lumStride;
            src += srcStride;
        }
        src -= srcStride*2;
        __asm__ volatile(
            "mov                        %4, %%""rax""\n\t"
            "movq       """ "ff_w1111"", %%mm5       \n\t"
            "movq          ""16*4+16*33""(%5), %%mm6       \n\t"
            "pxor                    %%mm7, %%mm7       \n\t"
            "lea (%%""rax"", %%""rax"", 2), %%""rdx"" \n\t"
            "add              %%""rdx"", %%""rdx""\n\t"
            ".p2align                    4              \n\t"
            "1:                                         \n\t"
            "prefetch"" 64(%0, %%""rdx"")              \n\t"
            "prefetch"" 64(%1, %%""rdx"")              \n\t"
            "movq       (%0, %%""rdx""), %%mm0       \n\t"
            "movq       (%1, %%""rdx""), %%mm1       \n\t"
            "movq      6(%0, %%""rdx""), %%mm2       \n\t"
            "movq      6(%1, %%""rdx""), %%mm3       \n\t"
            "pavgusb""                   %%mm1, %%mm0       \n\t"
            "pavgusb""                   %%mm3, %%mm2       \n\t"
            "movq                    %%mm0, %%mm1       \n\t"
            "movq                    %%mm2, %%mm3       \n\t"
            "psrlq                     $24, %%mm0       \n\t"
            "psrlq                     $24, %%mm2       \n\t"
            "pavgusb""                   %%mm1, %%mm0       \n\t"
            "pavgusb""                   %%mm3, %%mm2       \n\t"
            "punpcklbw               %%mm7, %%mm0       \n\t"
            "punpcklbw               %%mm7, %%mm2       \n\t"
            "movq          ""16*4+16*34""(%5), %%mm1       \n\t"
            "movq          ""16*4+16*34""(%5), %%mm3       \n\t"
            "pmaddwd                 %%mm0, %%mm1       \n\t"
            "pmaddwd                 %%mm2, %%mm3       \n\t"
            "pmaddwd                 %%mm6, %%mm0       \n\t"
            "pmaddwd                 %%mm6, %%mm2       \n\t"
            "psrad                      $8, %%mm0       \n\t"
            "psrad                      $8, %%mm1       \n\t"
            "psrad                      $8, %%mm2       \n\t"
            "psrad                      $8, %%mm3       \n\t"
            "packssdw                %%mm2, %%mm0       \n\t"
            "packssdw                %%mm3, %%mm1       \n\t"
            "pmaddwd                 %%mm5, %%mm0       \n\t"
            "pmaddwd                 %%mm5, %%mm1       \n\t"
            "packssdw                %%mm1, %%mm0       \n\t"
            "psraw                      $7, %%mm0       \n\t"
            "movq     12(%0, %%""rdx""), %%mm4       \n\t"
            "movq     12(%1, %%""rdx""), %%mm1       \n\t"
            "movq     18(%0, %%""rdx""), %%mm2       \n\t"
            "movq     18(%1, %%""rdx""), %%mm3       \n\t"
            "pavgusb""                   %%mm1, %%mm4       \n\t"
            "pavgusb""                   %%mm3, %%mm2       \n\t"
            "movq                    %%mm4, %%mm1       \n\t"
            "movq                    %%mm2, %%mm3       \n\t"
            "psrlq                     $24, %%mm4       \n\t"
            "psrlq                     $24, %%mm2       \n\t"
            "pavgusb""                   %%mm1, %%mm4       \n\t"
            "pavgusb""                   %%mm3, %%mm2       \n\t"
            "punpcklbw               %%mm7, %%mm4       \n\t"
            "punpcklbw               %%mm7, %%mm2       \n\t"
            "movq          ""16*4+16*34""(%5), %%mm1       \n\t"
            "movq          ""16*4+16*34""(%5), %%mm3       \n\t"
            "pmaddwd                 %%mm4, %%mm1       \n\t"
            "pmaddwd                 %%mm2, %%mm3       \n\t"
            "pmaddwd                 %%mm6, %%mm4       \n\t"
            "pmaddwd                 %%mm6, %%mm2       \n\t"
            "psrad                      $8, %%mm4       \n\t"
            "psrad                      $8, %%mm1       \n\t"
            "psrad                      $8, %%mm2       \n\t"
            "psrad                      $8, %%mm3       \n\t"
            "packssdw                %%mm2, %%mm4       \n\t"
            "packssdw                %%mm3, %%mm1       \n\t"
            "pmaddwd                 %%mm5, %%mm4       \n\t"
            "pmaddwd                 %%mm5, %%mm1       \n\t"
            "add                       $24, %%""rdx""\n\t"
            "packssdw                %%mm1, %%mm4       \n\t"
            "psraw                      $7, %%mm4       \n\t"
            "movq                    %%mm0, %%mm1           \n\t"
            "punpckldq               %%mm4, %%mm0           \n\t"
            "punpckhdq               %%mm4, %%mm1           \n\t"
            "packsswb                %%mm1, %%mm0           \n\t"
            "paddb """ "ff_bgr2UVOffset"", %%mm0         \n\t"
            "movd                    %%mm0, (%2, %%""rax"") \n\t"
            "punpckhdq               %%mm0, %%mm0              \n\t"
            "movd                    %%mm0, (%3, %%""rax"") \n\t"
            "add                        $4, %%""rax""       \n\t"
            " js                        1b              \n\t"
            : : "r" (src+chromWidth*6), "r" (src+srcStride+chromWidth*6), "r" (udst+chromWidth), "r" (vdst+chromWidth), "g" (-chromWidth), "r"(rgb2yuv)
             
            : "%""rax", "%""rdx"
        );
        udst += chromStride;
        vdst += chromStride;
        src += srcStride*2;
    }
    __asm__ volatile("femms""       \n\t"
                     " # nop""     \n\t"
                     :::"memory");
     ff_rgb24toyv12_c(src, ydst, udst, vdst, width, height-y, lumStride, chromStride, srcStride, rgb2yuv);
}
static void extract_even_3dnow(const uint8_t *src, uint8_t *dst, x86_reg count)
{
    dst += count;
    src += 2*count;
    count= - count;
    if(count <= -16) {
        count += 15;
        __asm__ volatile(
            "pcmpeqw       %%mm7, %%mm7        \n\t"
            "psrlw            $8, %%mm7        \n\t"
            "1:                                \n\t"
            "movq -30(%1, %0, 2), %%mm0        \n\t"
            "movq -22(%1, %0, 2), %%mm1        \n\t"
            "movq -14(%1, %0, 2), %%mm2        \n\t"
            "movq  -6(%1, %0, 2), %%mm3        \n\t"
            "pand          %%mm7, %%mm0        \n\t"
            "pand          %%mm7, %%mm1        \n\t"
            "pand          %%mm7, %%mm2        \n\t"
            "pand          %%mm7, %%mm3        \n\t"
            "packuswb      %%mm1, %%mm0        \n\t"
            "packuswb      %%mm3, %%mm2        \n\t"
            "movq""        %%mm0,-15(%2, %0)   \n\t"
            "movq""        %%mm2,- 7(%2, %0)   \n\t"
            "add             $16, %0           \n\t"
            " js 1b                            \n\t"
            : "+r"(count)
            : "r"(src), "r"(dst)
        );
        count -= 15;
    }
    while(count<0) {
        dst[count]= src[2*count];
        count++;
    }
}

static void extract_odd_3dnow(const uint8_t *src, uint8_t *dst, x86_reg count)
{
    src ++;
    dst += count;
    src += 2*count;
    count= - count;
    if(count < -16) {
        count += 16;
        __asm__ volatile(
            "pcmpeqw       %%mm7, %%mm7        \n\t"
            "psrlw            $8, %%mm7        \n\t"
            "1:                                \n\t"
            "movq -32(%1, %0, 2), %%mm0        \n\t"
            "movq -24(%1, %0, 2), %%mm1        \n\t"
            "movq -16(%1, %0, 2), %%mm2        \n\t"
            "movq  -8(%1, %0, 2), %%mm3        \n\t"
            "pand          %%mm7, %%mm0        \n\t"
            "pand          %%mm7, %%mm1        \n\t"
            "pand          %%mm7, %%mm2        \n\t"
            "pand          %%mm7, %%mm3        \n\t"
            "packuswb      %%mm1, %%mm0        \n\t"
            "packuswb      %%mm3, %%mm2        \n\t"
            "movq""        %%mm0,-16(%2, %0)   \n\t"
            "movq""        %%mm2,- 8(%2, %0)   \n\t"
            "add             $16, %0           \n\t"
            " js 1b                            \n\t"
            : "+r"(count)
            : "r"(src), "r"(dst)
        );
        count -= 16;
    }
    while(count<0) {
        dst[count]= src[2*count];
        count++;
    }
}
static void extract_even2avg_3dnow(const uint8_t *src0, const uint8_t *src1, uint8_t *dst0, uint8_t *dst1, x86_reg count)
{
    dst0 += count;
    dst1 += count;
    src0 += 4*count;
    src1 += 4*count;
    count= - count;
    if(count <= -8) {
        count += 7;
        __asm__ volatile(
            "pcmpeqw        %%mm7, %%mm7        \n\t"
            "psrlw             $8, %%mm7        \n\t"
            "1:                                \n\t"
            "movq  -28(%1, %0, 4), %%mm0        \n\t"
            "movq  -20(%1, %0, 4), %%mm1        \n\t"
            "movq  -12(%1, %0, 4), %%mm2        \n\t"
            "movq   -4(%1, %0, 4), %%mm3        \n\t"
            "pavgusb"" -28(%2, %0, 4), %%mm0        \n\t"
            "pavgusb"" -20(%2, %0, 4), %%mm1        \n\t"
            "pavgusb"" -12(%2, %0, 4), %%mm2        \n\t"
            "pavgusb"" - 4(%2, %0, 4), %%mm3        \n\t"
            "pand           %%mm7, %%mm0        \n\t"
            "pand           %%mm7, %%mm1        \n\t"
            "pand           %%mm7, %%mm2        \n\t"
            "pand           %%mm7, %%mm3        \n\t"
            "packuswb       %%mm1, %%mm0        \n\t"
            "packuswb       %%mm3, %%mm2        \n\t"
            "movq           %%mm0, %%mm1        \n\t"
            "movq           %%mm2, %%mm3        \n\t"
            "psrlw             $8, %%mm0        \n\t"
            "psrlw             $8, %%mm2        \n\t"
            "pand           %%mm7, %%mm1        \n\t"
            "pand           %%mm7, %%mm3        \n\t"
            "packuswb       %%mm2, %%mm0        \n\t"
            "packuswb       %%mm3, %%mm1        \n\t"
            "movq""         %%mm0,- 7(%4, %0)   \n\t"
            "movq""         %%mm1,- 7(%3, %0)   \n\t"
            "add               $8, %0           \n\t"
            " js 1b                            \n\t"
            : "+r"(count)
            : "r"(src0), "r"(src1), "r"(dst0), "r"(dst1)
        );
        count -= 7;
    }
    while(count<0) {
        dst0[count]= (src0[4*count+0]+src1[4*count+0])>>1;
        dst1[count]= (src0[4*count+2]+src1[4*count+2])>>1;
        count++;
    }
}
static void extract_odd2avg_3dnow(const uint8_t *src0, const uint8_t *src1, uint8_t *dst0, uint8_t *dst1, x86_reg count)
{
    dst0 += count;
    dst1 += count;
    src0 += 4*count;
    src1 += 4*count;
    count= - count;
    if(count <= -8) {
        count += 7;
        __asm__ volatile(
            "pcmpeqw        %%mm7, %%mm7        \n\t"
            "psrlw             $8, %%mm7        \n\t"
            "1:                                \n\t"
            "movq  -28(%1, %0, 4), %%mm0        \n\t"
            "movq  -20(%1, %0, 4), %%mm1        \n\t"
            "movq  -12(%1, %0, 4), %%mm2        \n\t"
            "movq   -4(%1, %0, 4), %%mm3        \n\t"
            "pavgusb"" -28(%2, %0, 4), %%mm0        \n\t"
            "pavgusb"" -20(%2, %0, 4), %%mm1        \n\t"
            "pavgusb"" -12(%2, %0, 4), %%mm2        \n\t"
            "pavgusb"" - 4(%2, %0, 4), %%mm3        \n\t"
            "psrlw             $8, %%mm0        \n\t"
            "psrlw             $8, %%mm1        \n\t"
            "psrlw             $8, %%mm2        \n\t"
            "psrlw             $8, %%mm3        \n\t"
            "packuswb       %%mm1, %%mm0        \n\t"
            "packuswb       %%mm3, %%mm2        \n\t"
            "movq           %%mm0, %%mm1        \n\t"
            "movq           %%mm2, %%mm3        \n\t"
            "psrlw             $8, %%mm0        \n\t"
            "psrlw             $8, %%mm2        \n\t"
            "pand           %%mm7, %%mm1        \n\t"
            "pand           %%mm7, %%mm3        \n\t"
            "packuswb       %%mm2, %%mm0        \n\t"
            "packuswb       %%mm3, %%mm1        \n\t"
            "movq""         %%mm0,- 7(%4, %0)   \n\t"
            "movq""         %%mm1,- 7(%3, %0)   \n\t"
            "add               $8, %0           \n\t"
            " js 1b                            \n\t"
            : "+r"(count)
            : "r"(src0), "r"(src1), "r"(dst0), "r"(dst1)
        );
        count -= 7;
    }
    src0++;
    src1++;
    while(count<0) {
        dst0[count]= (src0[4*count+0]+src1[4*count+0])>>1;
        dst1[count]= (src0[4*count+2]+src1[4*count+2])>>1;
        count++;
    }
}

static void yuyvtoyuv420_3dnow(uint8_t *ydst, uint8_t *udst, uint8_t *vdst, const uint8_t *src,
                                 int width, int height,
                                 int lumStride, int chromStride, int srcStride)
{
    int y;
    const int chromWidth = (!__builtin_constant_p(1) ? -((-(width)) >> (1)) : ((width) + (1<<(1)) - 1) >> (1));
    for (y=0; y<height; y++) {
        extract_even_3dnow(src, ydst, width);
        if(y&1) {
            extract_odd2avg_3dnow(src-srcStride, src, udst, vdst, chromWidth);
            udst+= chromStride;
            vdst+= chromStride;
        }
        src += srcStride;
        ydst+= lumStride;
    }
    __asm__(
            "femms""       \n\t"
            " # nop""     \n\t"
            ::: "memory"
        );
}
static void uyvytoyuv420_3dnow(uint8_t *ydst, uint8_t *udst, uint8_t *vdst, const uint8_t *src,
                                 int width, int height,
                                 int lumStride, int chromStride, int srcStride)
{
    int y;
    const int chromWidth = (!__builtin_constant_p(1) ? -((-(width)) >> (1)) : ((width) + (1<<(1)) - 1) >> (1));
    for (y=0; y<height; y++) {
        extract_odd_3dnow(src, ydst, width);
        if(y&1) {
            extract_even2avg_3dnow(src-srcStride, src, udst, vdst, chromWidth);
            udst+= chromStride;
            vdst+= chromStride;
        }
        src += srcStride;
        ydst+= lumStride;
    }
    __asm__(
            "femms""       \n\t"
            " # nop""     \n\t"
            ::: "memory"
        );
}
static void rgb2rgb_init_3dnow(void)
{
    planar2x = planar2x_3dnow;
    ff_rgb24toyv12 = rgb24toyv12_3dnow;
    yuyvtoyuv420 = yuyvtoyuv420_3dnow;
    uyvytoyuv420 = uyvytoyuv420_3dnow;
}
void ff_shuffle_bytes_2103_mmxext(const uint8_t *src, uint8_t *dst, int src_size);
void ff_shuffle_bytes_2103_ssse3(const uint8_t *src, uint8_t *dst, int src_size);
void ff_shuffle_bytes_0321_ssse3(const uint8_t *src, uint8_t *dst, int src_size);
void ff_shuffle_bytes_1230_ssse3(const uint8_t *src, uint8_t *dst, int src_size);
void ff_shuffle_bytes_3012_ssse3(const uint8_t *src, uint8_t *dst, int src_size);
void ff_shuffle_bytes_3210_ssse3(const uint8_t *src, uint8_t *dst, int src_size);

void ff_uyvytoyuv422_sse2(uint8_t *ydst, uint8_t *udst, uint8_t *vdst,
                          const uint8_t *src, int width, int height,
                          int lumStride, int chromStride, int srcStride);
void ff_uyvytoyuv422_avx(uint8_t *ydst, uint8_t *udst, uint8_t *vdst,
                         const uint8_t *src, int width, int height,
                         int lumStride, int chromStride, int srcStride);
 void rgb2rgb_init_x86(void)
{
    int cpu_flags = av_get_cpu_flags();
    if ((1 && ((cpu_flags) & 0x0001)))
        rgb2rgb_init_mmx();
    if ((1 && ((cpu_flags) & 0x0004)))
        rgb2rgb_init_3dnow();
    if ((1 && ((cpu_flags) & 0x0002)))
        rgb2rgb_init_mmxext();
    if ((1 && ((cpu_flags) & 0x0010)))
        rgb2rgb_init_sse2();
    if ((1 && ((cpu_flags) & 0x4000)))
        rgb2rgb_init_avx();
    if ((1 && ((cpu_flags) & 0x0002))) {
        shuffle_bytes_2103 = ff_shuffle_bytes_2103_mmxext;
    }
    if ((1 && ((cpu_flags) & 0x0010))) {
        uyvytoyuv422 = ff_uyvytoyuv422_sse2;
    }
    if ((1 && ((cpu_flags) & 0x0080))) {
        shuffle_bytes_0321 = ff_shuffle_bytes_0321_ssse3;
        shuffle_bytes_2103 = ff_shuffle_bytes_2103_ssse3;
        shuffle_bytes_1230 = ff_shuffle_bytes_1230_ssse3;
        shuffle_bytes_3012 = ff_shuffle_bytes_3012_ssse3;
        shuffle_bytes_3210 = ff_shuffle_bytes_3210_ssse3;
    }
    if ((1 && ((cpu_flags) & 0x4000))) {
        uyvytoyuv422 = ff_uyvytoyuv422_avx;
    }
}
