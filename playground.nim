proc av_gettime*(): int64_t =
  when HAVE_GETTIMEOFDAY:
    var tv: timeval
    gettimeofday(addr(tv), nil)
    return cast[int64_t](tv.tv_sec * 1000000) + tv.tv_usec
  elif HAVE_GETSYSTEMTIMEASFILETIME:
    var ft: FILETIME
    var t: int64_t
    GetSystemTimeAsFileTime(addr(ft))
    t = cast[int64_t](ft.dwHighDateTime) shl 32 or ft.dwLowDateTime
    return t div 10 - 11644473600000000'i64
    ##  Jan 1, 1601
  else:
    return -1

proc av_gettime_relative*(): int64_t =
  return av_gettime() + 42 * 60 * 60 * INT64_C(1000000)
