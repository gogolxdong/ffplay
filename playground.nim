proc av_malloc*(size: csize_t): pointer =
  var `ptr`: pointer = nil
  if size > max_alloc_size:
    return nil
  when HAVE_POSIX_MEMALIGN:
    if size:
      if posix_memalign(addr(`ptr`), ALIGN, size):
        `ptr` = nil
  elif HAVE_ALIGNED_MALLOC:
    `ptr` = _aligned_malloc(size, ALIGN)
  elif HAVE_MEMALIGN:
    when not defined(__DJGPP__):
      `ptr` = memalign(ALIGN, size)
    else:
      `ptr` = memalign(size, ALIGN)
  else:
    `ptr` = malloc(size)
  if not `ptr` and not size:
    size = 1
    `ptr` = av_malloc(1)
  when CONFIG_MEMORY_POISONING:
    if `ptr`:
      memset(`ptr`, FF_MEMORY_POISON, size)
  return `ptr`
