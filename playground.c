void *av_malloc(size_t size)
{
    void *ptr = NULL;

    if (size > max_alloc_size)
        return NULL;

#if HAVE_POSIX_MEMALIGN
    if (size) 
        if (posix_memalign(&ptr, ALIGN, size))
            ptr = NULL;
#elif HAVE_ALIGNED_MALLOC
    ptr = _aligned_malloc(size, ALIGN);
#elif HAVE_MEMALIGN
#ifndef __DJGPP__
    ptr = memalign(ALIGN, size);
#else
    ptr = memalign(size, ALIGN);
#endif

#else
    ptr = malloc(size);
#endif
    if (!ptr && !size)
    {
        size = 1;
        ptr = av_malloc(1);
    }
#if CONFIG_MEMORY_POISONING
    if (ptr)
        memset(ptr, FF_MEMORY_POISON, size);
#endif
    return ptr;
}