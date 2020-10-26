proc av_format_init_next*() =
  var
    prevout: ptr AVOutputFormat = nil
    `out`: ptr AVOutputFormat
  var
    previn: ptr AVInputFormat = nil
    `in`: ptr AVInputFormat
  ff_mutex_lock(addr(avpriv_register_devices_mutex))
  var i: cint = 0
  while (`out` = cast[ptr AVOutputFormat](muxer_list[i])):
    if prevout:
      prevout.next = `out`
    prevout = `out`
    inc(i)
  if outdev_list:
    var i: cint = 0
    while (`out` = cast[ptr AVOutputFormat](outdev_list[i])):
      if prevout:
        prevout.next = `out`
      prevout = `out`
      inc(i)
  var i: cint = 0
  while (`in` = cast[ptr AVInputFormat](demuxer_list[i])):
    if previn:
      previn.next = `in`
    previn = `in`
    inc(i)
  if indev_list:
    var i: cint = 0
    while (`in` = cast[ptr AVInputFormat](indev_list[i])):
      if previn:
        previn.next = `in`
      previn = `in`
      inc(i)
  ff_mutex_unlock(addr(avpriv_register_devices_mutex))
