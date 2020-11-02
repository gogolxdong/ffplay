import math
import ffmpeg, sdl2, sdl2/audio
when defined(windows):
  import winim
elif defined(posix):
  import posix

const
  FFMPEG_CONFIGURATION* = "--target-os=mingw32 --arch=x86_64 --disable-debug --disable-stripping --disable-doc --disable-w32threads --disable-static --enable-shared --enable-sdl2 --enable-vulkan --enable-dxva2 --enable-d3d11va --enable-gpl --enable-fontconfig --enable-iconv --enable-gnutls --enable-libxml2 --enable-lzma --enable-libsnappy --enable-zlib --enable-libsrt --enable-libssh --enable-libzmq --enable-libbluray --enable-libcaca --enable-libdav1d --enable-libwebp --enable-libx264 --enable-libx265 --enable-libxvid --enable-libaom --enable-libopenjpeg --enable-libvpx --enable-libass --enable-libfreetype --enable-libfribidi --enable-libvidstab --enable-libzimg --enable-libmfx --enable-libcdio --enable-libgme --enable-libmodplug --enable-libmp3lame --enable-libshine --enable-libtheora --enable-libtwolame --enable-libwavpack --enable-libilbc --enable-libgsm --enable-libopus --enable-libspeex --enable-libvorbis --enable-libbs2b --enable-libmysofa --enable-librubberband --enable-libsoxr --enable-chromaprint"
  FFMPEG_LICENSE* = "GPL version 2 or later"
  CONFIG_THIS_YEAR* = 2020
  FFMPEG_DATADIR* = "/usr/local/share/ffmpeg"
  AVCONV_DATADIR* = "/usr/local/share/ffmpeg"
  CC_IDENT* = "gcc 10.2.0 (Rev3, Built by MSYS2 project)"
  EXTERN_PREFIX* = ""
  EXTERN_ASM* = true
  BUILDSUF* = ""
  SLIBSUF* = ".dll"
  SWS_MAX_FILTER_SIZE* = 256
  ARCH_X86* = 1
  ARCH_X86_64* = 1
  
var program_name*: string = "ffplay"

var program_birth_year*: cint = 2003

const
  MAX_QUEUE_SIZE* = (15 * 1024 * 1024)
  MIN_FRAMES* = 25
  EXTERNAL_CLOCK_MIN_FRAMES* = 2
  EXTERNAL_CLOCK_MAX_FRAMES* = 10



const
  SDL_AUDIO_MIN_BUFFER_SIZE* = 512
  SDL_AUDIO_MAX_CALLBACKS_PER_SEC* = 30
  SDL_VOLUME_STEP* = (0.75)
  AV_SYNC_THRESHOLD_MIN* = 0.04
  AV_SYNC_THRESHOLD_MAX* = 0.1
  AV_SYNC_FRAMEDUP_THRESHOLD* = 0.1
  AV_NOSYNC_THRESHOLD* = 10.0
  SAMPLE_CORRECTION_PERCENT_MAX* = 10
  EXTERNAL_CLOCK_SPEED_MIN* = 0.9
  EXTERNAL_CLOCK_SPEED_MAX* = 1.01
  EXTERNAL_CLOCK_SPEED_STEP* = 0.001
  AUDIO_DIFF_AVG_NB* = 20
  REFRESH_RATE* = 0.01
  SAMPLE_ARRAY_SIZE* = (8 * 65536)
  CURSOR_HIDE_DELAY* = 1000000
  USE_ONEPASS_SUBTITLE_RENDER* = 1
  VIDEO_PICTURE_QUEUE_SIZE* = 3
  SUBPICTURE_QUEUE_SIZE* = 16
  SAMPLE_QUEUE_SIZE* = 9
  FRAME_QUEUE_SIZE* = max(SAMPLE_QUEUE_SIZE, max(VIDEO_PICTURE_QUEUE_SIZE,
      SUBPICTURE_QUEUE_SIZE))
  SWS_FAST_BILINEAR* = 1
  SWS_BILINEAR* = 2
  SWS_BICUBIC* = 4
  SWS_X* = 8
  SWS_POINT* = 0x00000010
  SWS_AREA* = 0x00000020
  SWS_BICUBLIN* = 0x00000040
  SWS_GAUSS* = 0x00000080
  SWS_SINC* = 0x00000100
  SWS_LANCZOS* = 0x00000200
  SWS_SPLINE* = 0x00000400
  SWS_SRC_V_CHR_DROP_MASK* = 0x00030000
  SWS_SRC_V_CHR_DROP_SHIFT* = 16
  SWS_PARAM_DEFAULT* = 123456
  SWS_PRINT_INFO* = 0x00001000
  SWS_FULL_CHR_H_INT* = 0x00002000
  SWS_FULL_CHR_H_INP* = 0x00004000
  SWS_DIRECT_BGR* = 0x00008000
  SWS_ACCURATE_RND* = 0x00040000
  SWS_BITEXACT* = 0x00080000
  SWS_ERROR_DIFFUSION* = 0x00800000
  SWS_MAX_REDUCE_CUTOFF* = 0.002
  SWS_CS_ITU709* = 1
  SWS_CS_FCC* = 4
  SWS_CS_ITU601* = 5
  SWS_CS_ITU624* = 5
  SWS_CS_SMPTE170M* = 5
  SWS_CS_SMPTE240M* = 7
  SWS_CS_DEFAULT* = 5
  SWS_CS_BT2020* = 9
  EPERM* = 1
  ENOENT* = 2
  ESRCH* = 3
  EINTR* = 4
  EIO* = 5
  ENXIO* = 6
  E2BIG* = 7
  ENOEXEC* = 8
  EBADF* = 9
  ECHILD* = 10
  EAGAIN* = 11
  ENOMEM* = 12
  EACCES* = 13
  EFAULT* = 14
  EBUSY* = 16
  EEXIST* = 17
  EXDEV* = 18
  ENODEV* = 19
  ENOTDIR* = 20
  EISDIR* = 21
  ENFILE* = 23
  EMFILE* = 24
  ENOTTY* = 25
  EFBIG* = 27
  ENOSPC* = 28
  ESPIPE* = 29
  EROFS* = 30
  EMLINK* = 31
  EPIPE* = 32
  EDOM* = 33
  EDEADLK* = 36
  ENAMETOOLONG* = 38
  ENOLCK* = 39
  ENOSYS* = 40
  ENOTEMPTY* = 41



type
  SDL_EventType* = enum
    SDL_FIRSTEVENT = 0,         ## *< Unused (do not remove)
                     ##  Application events
    SDL_QUIT = 0x00000100,      ## *< User-requested quit
                        ##  These application events have special meaning on iOS, see README-ios.md for details
    SDL_APP_TERMINATING, ## *< The application is being terminated by the OS
                        ##                                      Called on iOS in applicationWillTerminate()
                        ##                                      Called on Android in onDestroy()
                        ##
    SDL_APP_LOWMEMORY, ## *< The application is low on memory, free memory if possible.
                      ##                                      Called on iOS in applicationDidReceiveMemoryWarning()
                      ##                                      Called on Android in onLowMemory()
                      ##
    SDL_APP_WILLENTERBACKGROUND, ## *< The application is about to enter the background
                                ##                                      Called on iOS in applicationWillResignActive()
                                ##                                      Called on Android in onPause()
                                ##
    SDL_APP_DIDENTERBACKGROUND, ## *< The application did enter the background and may not get CPU for some time
                               ##                                      Called on iOS in applicationDidEnterBackground()
                               ##                                      Called on Android in onPause()
                               ##
    SDL_APP_WILLENTERFOREGROUND, ## *< The application is about to enter the foreground
                                ##                                      Called on iOS in applicationWillEnterForeground()
                                ##                                      Called on Android in onResume()
                                ##
    SDL_APP_DIDENTERFOREGROUND, ## *< The application is now interactive
                               ##                                      Called on iOS in applicationDidBecomeActive()
                               ##                                      Called on Android in onResume()
                               ##
                               ##  Display events
    SDL_DISPLAYEVENT = 0x00000150, ## *< Display state change
                                ##  Window events
    SDL_WINDOWEVENT = 0x00000200, ## *< Window state change
    SDL_SYSWMEVENT,           ## *< System specific event
                   ##  Keyboard events
    SDL_KEYDOWN = 0x00000300,   ## *< Key pressed
    SDL_KEYUP,                ## *< Key released
    SDL_TEXTEDITING,          ## *< Keyboard text editing (composition)
    SDL_TEXTINPUT,            ## *< Keyboard text input
    SDL_KEYMAPCHANGED, ## *< Keymap changed due to a system event such as an
                      ##                                      input language or keyboard layout change.
                      ##
                      ##  Mouse events
    SDL_MOUSEMOTION = 0x00000400, ## *< Mouse moved
    SDL_MOUSEBUTTONDOWN,      ## *< Mouse button pressed
    SDL_MOUSEBUTTONUP,        ## *< Mouse button released
    SDL_MOUSEWHEEL,           ## *< Mouse wheel motion
                   ##  Joystick events
    SDL_JOYAXISMOTION = 0x00000600, ## *< Joystick axis motion
    SDL_JOYBALLMOTION,        ## *< Joystick trackball motion
    SDL_JOYHATMOTION,         ## *< Joystick hat position change
    SDL_JOYBUTTONDOWN,        ## *< Joystick button pressed
    SDL_JOYBUTTONUP,          ## *< Joystick button released
    SDL_JOYDEVICEADDED,       ## *< A new joystick has been inserted into the system
    SDL_JOYDEVICEREMOVED,     ## *< An opened joystick has been removed
                         ##  Game controller events
    SDL_CONTROLLERAXISMOTION = 0x00000650, ## *< Game controller axis motion
    SDL_CONTROLLERBUTTONDOWN, ## *< Game controller button pressed
    SDL_CONTROLLERBUTTONUP,   ## *< Game controller button released
    SDL_CONTROLLERDEVICEADDED, ## *< A new Game controller has been inserted into the system
    SDL_CONTROLLERDEVICEREMOVED, ## *< An opened Game controller has been removed
    SDL_CONTROLLERDEVICEREMAPPED, ## *< The controller mapping was updated
                                 ##  Touch events
    SDL_FINGERDOWN = 0x00000700, SDL_FINGERUP, SDL_FINGERMOTION, ##  Gesture events
    SDL_DOLLARGESTURE = 0x00000800, SDL_DOLLARRECORD, SDL_MULTIGESTURE, ##  Clipboard events
    SDL_CLIPBOARDUPDATE = 0x00000900, ## *< The clipboard changed
                                   ##  Drag and drop events
    SDL_DROPFILE = 0x00001000,  ## *< The system requests a file open
    SDL_DROPTEXT,             ## *< text/plain drag-and-drop event
    SDL_DROPBEGIN,            ## *< A new set of drops is beginning (NULL filename)
    SDL_DROPCOMPLETE, ## *< Current set of drops is now complete (NULL filename)
                     ##  Audio hotplug events
    SDL_AUDIODEVICEADDED = 0x00001100, ## *< A new audio device is available
    SDL_AUDIODEVICEREMOVED,   ## *< An audio device has been removed.
                           ##  Sensor events
    SDL_SENSORUPDATE = 0x00001200, ## *< A sensor was updated
                                ##  Render events
    SDL_RENDER_TARGETS_RESET = 0x00002000, ## *< The render targets have been reset and their contents need to be updated
    SDL_RENDER_DEVICE_RESET, ## *< The device has been reset and all textures need to be recreated
                            ## * Events ::SDL_USEREVENT through ::SDL_LASTEVENT are for your use,
                            ##   and should be allocated with SDL_RegisterEvents()
                            ##
    SDL_USEREVENT = 0x00008000, ## *
                             ##   This last event is only for bounding internal arrays
                             ##
    SDL_LASTEVENT = 0x0000FFFF


type
  SDL_BlendOperation* = enum
    SDL_BLENDOPERATION_ADD = 0x00000001, ## *< dst + src: supported by all renderers
    SDL_BLENDOPERATION_SUBTRACT = 0x00000002, ## *< dst - src : supported by D3D9, D3D11, OpenGL, OpenGLES
    SDL_BLENDOPERATION_REV_SUBTRACT = 0x00000003, ## *< src - dst : supported by D3D9, D3D11, OpenGL, OpenGLES
    SDL_BLENDOPERATION_MINIMUM = 0x00000004, ## *< min(dst, src) : supported by D3D11
    SDL_BLENDOPERATION_MAXIMUM = 0x00000005

type
  SDL_BlendFactor* = enum
    SDL_BLENDFACTOR_ZERO = 0x00000001, ## *< 0, 0, 0, 0
    SDL_BLENDFACTOR_ONE = 0x00000002, ## *< 1, 1, 1, 1
    SDL_BLENDFACTOR_SRC_COLOR = 0x00000003, ## *< srcR, srcG, srcB, srcA
    SDL_BLENDFACTOR_ONE_MINUS_SRC_COLOR = 0x00000004, ## *< 1-srcR, 1-srcG, 1-srcB, 1-srcA
    SDL_BLENDFACTOR_SRC_ALPHA = 0x00000005, ## *< srcA, srcA, srcA, srcA
    SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA = 0x00000006, ## *< 1-srcA, 1-srcA, 1-srcA, 1-srcA
    SDL_BLENDFACTOR_DST_COLOR = 0x00000007, ## *< dstR, dstG, dstB, dstA
    SDL_BLENDFACTOR_ONE_MINUS_DST_COLOR = 0x00000008, ## *< 1-dstR, 1-dstG, 1-dstB, 1-dstA
    SDL_BLENDFACTOR_DST_ALPHA = 0x00000009, ## *< dstA, dstA, dstA, dstA
    SDL_BLENDFACTOR_ONE_MINUS_DST_ALPHA = 0x0000000A

template SDL_DEFINE_PIXELFORMAT*(t, order, layout, bits, bytes: untyped): untyped =
  ((1 shl 28) or (t.int shl 24) or (order.int shl 20) or (layout.int shl 16) or (bits.int shl 8) or (bytes.int shl 0))

template INSERT_FILT(name, arg: untyped): void =
    while true:
      var filt_ctx: ptr AVFilterContext
      ret = avfilter_graph_create_filter(addr(filt_ctx), avfilter_get_by_name(name), "ffplay_", name, arg, nil, graph)
      if ret < 0:
        break fail
      ret = avfilter_link(filt_ctx, 0, last_filter, 0)
      if ret < 0:
        break fail
      last_filter = filt_ctx
      if not 0:
        break

template `+`*[T](p: ptr T, off: int): ptr T =
  cast[ptr type(p[])](cast[ByteAddress](p) +% off * sizeof(p[]))

template `+=`*[T](p: ptr T, off: int) =
  p = p + off

template `-`*[T](p: ptr T, off: int): ptr T =
  cast[ptr type(p[])](cast[ByteAddress](p) -% off * sizeof(p[]))

template `-=`*[T](p: ptr T, off: int) =
  p = p - off

template `[]`*[T](p: ptr T, off: int): T =
  (p + off)[]

template `[]=`*[T](p: ptr T, off: int, val: T) =
  (p + off)[] = val

type
  SDL_PixelType* = enum
    SDL_PIXELTYPE_UNKNOWN, SDL_PIXELTYPE_INDEX1, SDL_PIXELTYPE_INDEX4,
    SDL_PIXELTYPE_INDEX8, SDL_PIXELTYPE_PACKED8, SDL_PIXELTYPE_PACKED16,
    SDL_PIXELTYPE_PACKED32, SDL_PIXELTYPE_ARRAYU8, SDL_PIXELTYPE_ARRAYU16,
    SDL_PIXELTYPE_ARRAYU32, SDL_PIXELTYPE_ARRAYF16, SDL_PIXELTYPE_ARRAYF32

type
  SDL_BitmapOrder* = enum
    SDL_BITMAPORDER_NONE, SDL_BITMAPORDER_4321, SDL_BITMAPORDER_1234

type
  SDL_PackedOrder* = enum
    SDL_PACKEDORDER_NONE, SDL_PACKEDORDER_XRGB, SDL_PACKEDORDER_RGBX,
    SDL_PACKEDORDER_ARGB, SDL_PACKEDORDER_RGBA, SDL_PACKEDORDER_XBGR,
    SDL_PACKEDORDER_BGRX, SDL_PACKEDORDER_ABGR, SDL_PACKEDORDER_BGRA

type
  SDL_ArrayOrder* = enum
    SDL_ARRAYORDER_NONE, SDL_ARRAYORDER_RGB, SDL_ARRAYORDER_RGBA,
    SDL_ARRAYORDER_ARGB, SDL_ARRAYORDER_BGR, SDL_ARRAYORDER_BGRA,
    SDL_ARRAYORDER_ABGR

type
  SDL_PackedLayout* = enum
    SDL_PACKEDLAYOUT_NONE, SDL_PACKEDLAYOUT_332, SDL_PACKEDLAYOUT_4444,
    SDL_PACKEDLAYOUT_1555, SDL_PACKEDLAYOUT_5551, SDL_PACKEDLAYOUT_565,
    SDL_PACKEDLAYOUT_8888, SDL_PACKEDLAYOUT_2101010, SDL_PACKEDLAYOUT_1010102

type
  SDL_PixelFormatEnum* = enum
    SDL_PIXELFORMAT_UNKNOWN, 
    # SDL_PIXELFORMAT_INDEX1LSB = SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_INDEX1, SDL_BITMAPORDER_4321, 0, 1, 0), 
    # SDL_PIXELFORMAT_INDEX1MSB = SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_INDEX1, SDL_BITMAPORDER_1234, 0, 1, 0), 
    # SDL_PIXELFORMAT_INDEX4LSB = SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_INDEX4, SDL_BITMAPORDER_4321, 0, 4, 0), 
    # SDL_PIXELFORMAT_INDEX4MSB = SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_INDEX4, SDL_BITMAPORDER_1234, 0, 4, 0), 
    # SDL_PIXELFORMAT_INDEX8 = SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_INDEX8, 0, 0, 8, 1), 
    # SDL_PIXELFORMAT_RGB332 = SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED8, SDL_PACKEDORDER_XRGB, SDL_PACKEDLAYOUT_332, 8, 1), 
    # SDL_PIXELFORMAT_RGB444 = SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_XRGB, SDL_PACKEDLAYOUT_4444, 12, 2), 
    # SDL_PIXELFORMAT_BGR444 = SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_XBGR, SDL_PACKEDLAYOUT_4444, 12, 2), 
    # SDL_PIXELFORMAT_RGB555 = SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_XRGB, SDL_PACKEDLAYOUT_1555, 15, 2), 
    # SDL_PIXELFORMAT_BGR555 = SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_XBGR, SDL_PACKEDLAYOUT_1555, 15, 2), 
    # SDL_PIXELFORMAT_ARGB4444 = SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_ARGB, SDL_PACKEDLAYOUT_4444, 16, 2), 
    # SDL_PIXELFORMAT_RGBA4444 = SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_RGBA, SDL_PACKEDLAYOUT_4444, 16, 2), 
    # SDL_PIXELFORMAT_ABGR4444 = SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_ABGR, SDL_PACKEDLAYOUT_4444, 16, 2), 
    # SDL_PIXELFORMAT_BGRA4444 = SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_BGRA, SDL_PACKEDLAYOUT_4444, 16, 2), 
    # SDL_PIXELFORMAT_ARGB1555 = SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_ARGB, SDL_PACKEDLAYOUT_1555, 16, 2), 
    # SDL_PIXELFORMAT_RGBA5551 = SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_RGBA, SDL_PACKEDLAYOUT_5551, 16, 2), 
    # SDL_PIXELFORMAT_ABGR1555 = SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_ABGR, SDL_PACKEDLAYOUT_1555, 16, 2), 
    # SDL_PIXELFORMAT_BGRA5551 = SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_BGRA, SDL_PACKEDLAYOUT_5551, 16, 2), 
    # SDL_PIXELFORMAT_RGB565 = SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_XRGB, SDL_PACKEDLAYOUT_565, 16, 2), 
    # SDL_PIXELFORMAT_BGR565 = SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED16, SDL_PACKEDORDER_XBGR, SDL_PACKEDLAYOUT_565, 16, 2), 
    # SDL_PIXELFORMAT_RGB24 = SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_ARRAYU8, SDL_ARRAYORDER_RGB, 0, 24, 3), 
    # SDL_PIXELFORMAT_BGR24 = SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_ARRAYU8, SDL_ARRAYORDER_BGR, 0, 24, 3), 
    # SDL_PIXELFORMAT_RGB888 = SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_XRGB, SDL_PACKEDLAYOUT_8888, 24, 4), 
    # SDL_PIXELFORMAT_RGBX8888 = SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_RGBX, SDL_PACKEDLAYOUT_8888, 24, 4), 
    # SDL_PIXELFORMAT_BGR888 = SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_XBGR, SDL_PACKEDLAYOUT_8888, 24, 4), 
    # SDL_PIXELFORMAT_BGRX8888 = SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_BGRX, SDL_PACKEDLAYOUT_8888, 24, 4), 
    # SDL_PIXELFORMAT_ARGB8888 = SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_ARGB, SDL_PACKEDLAYOUT_8888, 32, 4), 
    # SDL_PIXELFORMAT_RGBA8888 = SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_RGBA, SDL_PACKEDLAYOUT_8888, 32, 4), 
    # SDL_PIXELFORMAT_ABGR8888 = SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_ABGR, SDL_PACKEDLAYOUT_8888, 32, 4), 
    # SDL_PIXELFORMAT_BGRA8888 = SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_BGRA, SDL_PACKEDLAYOUT_8888, 32, 4), 
    # SDL_PIXELFORMAT_ARGB2101010 = SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_ARGB, SDL_PACKEDLAYOUT_2101010, 32,4),                   ##  Aliases for RGBA byte arrays of color data, for the current platform
    # SDL_PIXELFORMAT_YV12 = SDL_DEFINE_PIXELFOURCC('Y', 'V', '1', '2'), ## *< Planar mode: Y + V + U  (3 planes)
    # SDL_PIXELFORMAT_IYUV = SDL_DEFINE_PIXELFOURCC('I', 'Y', 'U', 'V'), ## *< Planar mode: Y + U + V  (3 planes)
    # SDL_PIXELFORMAT_YUY2 = SDL_DEFINE_PIXELFOURCC('Y', 'U', 'Y', '2'), ## *< Packed mode: Y0+U0+Y1+V0 (1 plane)
    # SDL_PIXELFORMAT_UYVY = SDL_DEFINE_PIXELFOURCC('U', 'Y', 'V', 'Y'), ## *< Packed mode: U0+Y0+V0+Y1 (1 plane)
    # SDL_PIXELFORMAT_YVYU = SDL_DEFINE_PIXELFOURCC('Y', 'V', 'Y', 'U'), ## *< Packed mode: Y0+V0+Y1+U0 (1 plane)
    # SDL_PIXELFORMAT_NV12 = SDL_DEFINE_PIXELFOURCC('N', 'V', '1', '2'), ## *< Planar mode: Y + U/V interleaved  (2 planes)
    # SDL_PIXELFORMAT_NV21 = SDL_DEFINE_PIXELFOURCC('N', 'V', '2', '1'), ## *< Planar mode: Y + V/U interleaved  (2 planes)
    # SDL_PIXELFORMAT_EXTERNAL_OES = SDL_DEFINE_PIXELFOURCC('O', 'E', 'S', ' ') ## *< Android video texture format

const
  SDL_PIXELFORMAT_RGBA32 = SDL_PIXELFORMAT_ABGR8888
  SDL_PIXELFORMAT_ARGB32 = SDL_PIXELFORMAT_BGRA8888
  SDL_PIXELFORMAT_BGRA32 = SDL_PIXELFORMAT_ARGB8888
  SDL_PIXELFORMAT_ABGR32 = SDL_PIXELFORMAT_RGBA8888



var sws_flags*: cuint = SWS_BICUBIC

type
  MyAVPacketList* {.bycopy.} = object
    pkt*: AVPacket
    next*: ptr MyAVPacketList
    serial*: cint

  PacketQueue* {.bycopy.} = object
    first_pkt*: ptr MyAVPacketList
    last_pkt*: ptr MyAVPacketList
    nb_packets*: cint
    size*: cint
    duration*: int64
    abort_request*: cint
    serial*: cint
    # mutex*: ptr SDL_mutex
    # cond*: ptr SDL_cond

  AudioParams* {.bycopy.} = object
    freq*: cint
    channels*: cint
    channel_layout*: int64
    fmt*: AVSampleFormat
    frame_size*: cint
    bytes_per_sec*: cint

  Clock* {.bycopy.} = object
    pts*: cdouble              ##  clock base
    pts_drift*: cdouble        ##  clock base minus time at which we updated the clock
    last_updated*: cdouble
    speed*: cdouble
    serial*: cint              ##  clock is based on a packet with this serial
    paused*: cint
    queue_serial*: ptr cint     ##  pointer to the current packet queue serial, used for obsolete clock detection


##  Common struct for handling all types of decoded data and allocated render buffers.

type
  Frame* {.bycopy.} = object
    frame*: ptr AVFrame
    sub*: AVSubtitle
    serial*: cint
    pts*: cdouble              ##  presentation timestamp for the frame
    duration*: cdouble         ##  estimated duration of the frame
    pos*: int64              ##  byte position of the frame in the input file
    width*: cint
    height*: cint
    format*: cint
    sar*: AVRational
    uploaded*: cint
    flip_v*: cint

  FrameQueue* {.bycopy.} = object
    queue*: array[FRAME_QUEUE_SIZE, Frame]
    rindex*: cint
    windex*: cint
    size*: cint
    max_size*: cint
    keep_last*: cint
    rindex_shown*: cint
    # mutex*: ptr SDL_mutex
    # cond*: ptr SDL_cond
    pktq*: ptr PacketQueue


const
  AV_SYNC_AUDIO_MASTER* = 0     ##  default choice
  AV_SYNC_VIDEO_MASTER* = 1
  AV_SYNC_EXTERNAL_CLOCK* = 2   ##  synchronize to an external clock

type
  Decoder* {.bycopy.} = object
    pkt*: AVPacket
    queue*: ptr PacketQueue
    avctx*: ptr AVCodecContext
    pkt_serial*: cint
    finished*: cint
    packet_pending*: cint
    # empty_queue_cond*: ptr SDL_cond
    start_pts*: int64
    start_pts_tb*: AVRational
    next_pts*: int64
    next_pts_tb*: AVRational
    # decoder_tid*: ptr SDL_Thread

  ShowMode* = enum
    SHOW_MODE_NONE = -1, SHOW_MODE_VIDEO = 0, SHOW_MODE_WAVES, SHOW_MODE_RDFT,
    SHOW_MODE_NB

type SwrContext {.header:"<libswresample/swresample_internal.h>",importc:"struct SwrContext"} = object

type
  VideoState* {.bycopy.} = object
    # read_tid*: ptr SDL_Thread
    iformat*: ptr AVInputFormat
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
    ic*: ptr AVFormatContext
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
    audio_diff_cum*: cdouble   ##  used for AV difference average computation
    audio_diff_avg_coef*: cdouble
    audio_diff_threshold*: cdouble
    audio_diff_avg_count*: cint
    audio_st*: ptr AVStream
    audioq*: PacketQueue
    audio_hw_buf_size*: cint
    audio_buf*: ptr uint8
    audio_buf1*: ptr uint8
    audio_buf_size*: cuint     ##  in bytes
    audio_buf1_size*: cuint
    audio_buf_index*: cint     ##  in bytes
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
    sample_array*: array[SAMPLE_ARRAY_SIZE, int16]
    sample_array_index*: cint
    last_i_start*: cint
    rdft*: ptr RDFTContext
    rdft_bits*: cint
    rdft_data*: ptr FFTSample
    xpos*: cint
    last_vis_time*: cdouble
    vis_texture*: TexturePtr
    sub_texture*: TexturePtr
    vid_texture*: TexturePtr
    subtitle_stream*: cint
    subtitle_st*: ptr AVStream
    subtitleq*: PacketQueue
    frame_timer*: cdouble
    frame_last_returned_time*: cdouble
    frame_last_filter_delay*: cdouble
    video_stream*: cint
    video_st*: ptr AVStream
    videoq*: PacketQueue
    max_frame_duration*: cdouble ##  maximum duration of a frame - above this, we consider the jump a timestamp discontinuity
    img_convert_ctx*: ptr SwsContext
    sub_convert_ctx*: ptr SwsContext
    eof*: cint
    filename*: cstring
    width*: cint
    height*: cint
    xleft*: cint
    ytop*: cint
    step*: cint
    vfilter_idx*: cint
    in_video_filter*: ptr AVFilterContext
    ##  the first filter in the video chain
    out_video_filter*: ptr AVFilterContext
    ##  the last filter in the video chain
    in_audio_filter*: ptr AVFilterContext
    ##  the first filter in the audio chain
    out_audio_filter*: ptr AVFilterContext
    ##  the last filter in the audio chain
    agraph*: ptr AVFilterGraph
    ##  audio filter graph
    last_video_stream*: cint
    last_audio_stream*: cint
    last_subtitle_stream*: cint
    # continue_read_thread*: ptr SDL_cond


##  options specified by the user

var file_iformat*: ptr AVInputFormat

var input_filename*: cstring

var window_title*: cstring

var default_width*: cint = 640

var default_height*: cint = 480

var screen_width*: cint = 0

var screen_height*: cint = 0

var screen_left*: cint = SDL_WINDOWPOS_CENTERED

var screen_top*: cint = SDL_WINDOWPOS_CENTERED

var audio_disable*: cint

var video_disable*: cint

var subtitle_disable*: cint

var wanted_stream_spec*: array[AVMEDIA_TYPE_NB, cstring]

var seek_by_bytes*: cint = -1

var seek_interval*: cfloat = 10

var display_disable*: cint

var borderless*: cint

var alwaysontop*: cint

var startup_volume*: cint = 100

var show_status*: cint = -1

var av_sync_type*: cint = AV_SYNC_AUDIO_MASTER

var start_time*: int64 = AV_NOPTS_VALUE

var duration*: int64 = AV_NOPTS_VALUE

var fast*: cint = 0

var genpts*: cint = 0

var lowres*: cint = 0

var decoder_reorder_pts*: cint = -1

var autoexit*: cint

var exit_on_keydown*: cint

var exit_on_mousedown*: cint

var loop*: cint = 1

var framedrop*: cint = -1

var infinite_buffer*: cint = -1

var show_mode*: ShowMode = SHOW_MODE_NONE

var audio_codec_name*: cstring

var subtitle_codec_name*: cstring

var video_codec_name*: cstring

var rdftspeed*: cdouble = 0.02

var cursor_last_shown*: int64

var cursor_hidden*: cint = 0

var vfilters_list*: cstringArray = nil
var nb_vfilters*: cint = 0
var afilters*: cstring = nil
var autorotate*: cint = 1

var find_stream_info*: cint = 1

var filter_nbthreads*: cint = 0

##  current context

var is_full_screen*: cint

var audio_callback_time*: int64

var flush_pkt*: AVPacket

const
  FF_QUIT_EVENT* = (SDL_USEREVENT.int + 2)

var window*: WindowPtr

var renderer*: RendererPtr

var renderer_info*: RendererInfo

var audio_dev*: AudioDeviceID

const sdl_texture_format_map: seq[(AVPixelFormat,int)] = @[]
##  static const struct TextureFormatEntry
##  {
##      enum AVPixelFormat format;
##      int texture_fmt;
##  } sdl_texture_format_map[] = {
##      {AV_PIX_FMT_RGB8, SDL_PIXELFORMAT_RGB332},
##      {AV_PIX_FMT_RGB444, SDL_PIXELFORMAT_RGB444},
##      {AV_PIX_FMT_RGB555, SDL_PIXELFORMAT_RGB555},
##      {AV_PIX_FMT_BGR555, SDL_PIXELFORMAT_BGR555},
##      {AV_PIX_FMT_RGB565, SDL_PIXELFORMAT_RGB565},
##      {AV_PIX_FMT_BGR565, SDL_PIXELFORMAT_BGR565},
##      {AV_PIX_FMT_RGB24, SDL_PIXELFORMAT_RGB24},
##      {AV_PIX_FMT_BGR24, SDL_PIXELFORMAT_BGR24},
##      {AV_PIX_FMT_0RGB32, SDL_PIXELFORMAT_RGB888},
##      {AV_PIX_FMT_0BGR32, SDL_PIXELFORMAT_BGR888},
##      {AV_PIX_FMT_NE(RGB0, 0BGR), SDL_PIXELFORMAT_RGBX8888},
##      {AV_PIX_FMT_NE(BGR0, 0RGB), SDL_PIXELFORMAT_BGRX8888},
##      {AV_PIX_FMT_RGB32, SDL_PIXELFORMAT_ARGB8888},
##      {AV_PIX_FMT_RGB32_1, SDL_PIXELFORMAT_RGBA8888},
##      {AV_PIX_FMT_BGR32, SDL_PIXELFORMAT_ABGR8888},
##      {AV_PIX_FMT_BGR32_1, SDL_PIXELFORMAT_BGRA8888},
##      {AV_PIX_FMT_YUV420P, SDL_PIXELFORMAT_IYUV},
##      {AV_PIX_FMT_YUYV422, SDL_PIXELFORMAT_YUY2},
##      {AV_PIX_FMT_UYVY422, SDL_PIXELFORMAT_UYVY},
##      {AV_PIX_FMT_NONE, SDL_PIXELFORMAT_UNKNOWN},
##  };

proc opt_add_vfilter*(optctx: pointer; opt: cstring; arg: cstring): cint =
#   GROW_ARRAY(vfilters_list, nb_vfilters)
  vfilters_list[nb_vfilters - 1] = arg
  return 0

proc cmp_audio_fmts*(fmt1: AVSampleFormat; channel_count1: int64;
                    fmt2: AVSampleFormat; channel_count2: int64): cint {.inline.} =
  ##  If channel count == 1, planar and non-planar formats are the same
  if channel_count1 == 1 and channel_count2 == 1:
    return cint av_get_packed_sample_fmt(fmt1) != av_get_packed_sample_fmt(fmt2)
  else:
    return cint channel_count1 != channel_count2 or fmt1 != fmt2

proc get_valid_channel_layout*(channel_layout: int64; channels: cint): int64 {.
    inline.} =
  if av_get_channel_layout_nb_channels(channel_layout.uint64) == channels:
    return channel_layout
  else:
    return 0

proc packet_queue_put_private*(q: ptr PacketQueue; pkt: ptr AVPacket): cint =
  var pkt1: ptr MyAVPacketList
  if q.abort_request != 0:
    return -1
#   pkt1 = av_malloc(sizeof((MyAVPacketList)))
  if pkt1 == nil:
    return -1
  pkt1.pkt = pkt[]
  pkt1.next = nil
  if pkt == addr(flush_pkt):
    inc(q.serial)
  pkt1.serial = q.serial
  if q.last_pkt == nil:
    q.first_pkt = pkt1
  else:
    q.last_pkt.next = pkt1
  q.last_pkt = pkt1
  inc(q.nb_packets)
  inc(q.size, pkt1.pkt.size + sizeof((pkt1[])))
  q.duration += pkt1.pkt.duration.int
  ##  XXX: should duplicate packet data in DV case
#   SDL_CondSignal(q.cond)
  return 0

proc packet_queue_put*(q: ptr PacketQueue; pkt: ptr AVPacket): cint =
  var ret: cint
#   SDL_LockMutex(q.mutex)
  ret = packet_queue_put_private(q, pkt)
#   SDL_UnlockMutex(q.mutex)
  if pkt != addr(flush_pkt) and ret < 0:
    av_packet_unref(pkt)
  return ret

proc packet_queue_put_nullpacket*(q: ptr PacketQueue; stream_index: cint): cint =
  var
    pkt1: AVPacket
    pkt: ptr AVPacket = addr(pkt1)
  av_init_packet(pkt)
  pkt.data = nil
  pkt.size = 0
  pkt.stream_index = stream_index
  return packet_queue_put(q, pkt)

##  packet queue handling

proc packet_queue_init*(q: ptr PacketQueue): cint =
#  memset(q, 0, sizeof((PacketQueue)))
#   q.mutex = SDL_CreateMutex()
#   if not q.mutex:
#     echo(nil, AV_LOG_FATAL, "SDL_CreateMutex(): %s\n", SDL_GetError())
#     return AVERROR(ENOMEM)
#   q.cond = SDL_CreateCond()
#   if not q.cond:
#     echo(nil, AV_LOG_FATAL, "SDL_CreateCond(): %s\n", SDL_GetError())
#     return AVERROR(ENOMEM)
  q.abort_request = 1
  return 0

proc packet_queue_flush*(q: ptr PacketQueue) =
  var
    pkt: ptr MyAVPacketList
    pkt1: ptr MyAVPacketList
#   SDL_LockMutex(q.mutex)
  pkt = q.first_pkt
  while pkt != nil:
    pkt1 = pkt.next
    # av_packet_unref(addr(pkt.pkt))
    # av_freep(addr(pkt))
    pkt = pkt1
  q.last_pkt = nil
  q.first_pkt = nil
  q.nb_packets = 0
  q.size = 0
  q.duration = 0
#   SDL_UnlockMutex(q.mutex)

proc packet_queue_destroy*(q: ptr PacketQueue) =
  packet_queue_flush(q)
#   SDL_DestroyMutex(q.mutex)
#   SDL_DestroyCond(q.cond)

proc packet_queue_abort*(q: ptr PacketQueue) =
#   SDL_LockMutex(q.mutex)
  q.abort_request = 1
#   SDL_CondSignal(q.cond)
#   SDL_UnlockMutex(q.mutex)

proc packet_queue_start*(q: ptr PacketQueue) =
#   SDL_LockMutex(q.mutex)
  q.abort_request = 0
  discard packet_queue_put_private(q, addr(flush_pkt))
#   SDL_UnlockMutex(q.mutex)

##  return < 0 if aborted, 0 if no packet and > 0 if packet.

proc packet_queue_get*(q: ptr PacketQueue; pkt: ptr AVPacket; b: cint;
                      serial: ptr cint): cint =
  var pkt1: ptr MyAVPacketList
  var ret: cint
#   SDL_LockMutex(q.mutex)
  while true:
    if q.abort_request != 0:
      ret = -1
      break
    pkt1 = q.first_pkt
    if pkt1 != nil:
      q.first_pkt = pkt1.next
      if q.first_pkt == nil:
        q.last_pkt = nil
      dec(q.nb_packets)
      dec(q.size, pkt1.pkt.size + sizeof((pkt1[])))
      q.duration -= pkt1.pkt.duration.int
      pkt[] = pkt1.pkt
      if serial != nil:
        serial[] = pkt1.serial
    #   av_free(pkt1)
      ret = 1
      break
    elif b == 0:
      ret = 0
      break
    # else:
    #   SDL_CondWait(q.cond, q.mutex)
#   SDL_UnlockMutex(q.mutex)
  return ret

type SDL_cond {.importc, header:"<sdl2/SDL_mutex.h>"} = object

  

proc decoder_init*(d: ptr Decoder; avctx: ptr AVCodecContext; queue: ptr PacketQueue; empty_queue_cond: SDL_cond) =
  #memset(d, 0, sizeof((Decoder)))
  d.avctx = avctx
  d.queue = queue
#   d.empty_queue_cond = empty_queue_cond
  d.start_pts = AV_NOPTS_VALUE
  d.pkt_serial = -1

proc decoder_decode_frame*(d: ptr Decoder; frame: ptr AVFrame; sub: ptr AVSubtitle): cint =
  var ret: cint = AVERROR(EAGAIN)
  while true:
    var pkt: AVPacket
    if d.queue.serial == d.pkt_serial:
      while true:
        if d.queue.abort_request != 0:
          return -1
        case d.avctx.codec_type
        of AVMEDIA_TYPE_VIDEO:
          ret = avcodec_receive_frame(d.avctx, frame)
          if ret >= 0:
            if decoder_reorder_pts == -1:
              frame.pts = frame.best_effort_timestamp
            elif decoder_reorder_pts == 0:
              frame.pts = frame.pkt_dts
        of AVMEDIA_TYPE_AUDIO:
          ret = avcodec_receive_frame(d.avctx, frame)
          if ret >= 0:
            var tb = AVRational(num:1.cint, den:frame.sample_rate)
            # if frame.pts != AV_NOPTS_VALUE: 
            #   frame.pts = av_rescale_q(frame.pts, d.avctx.pkt_timebase, tb)
            # elif d.next_pts != AV_NOPTS_VALUE:
            #   frame.pts = av_rescale_q(d.next_pts, d.next_pts_tb, tb)
            if frame.pts != AV_NOPTS_VALUE:
              d.next_pts = frame.pts + frame.nb_samples
              d.next_pts_tb = tb
        else:discard
        if ret == AVERROR_EOF:
          d.finished = d.pkt_serial
          avcodec_flush_buffers(d.avctx)
          return 0
        if ret >= 0:
          return 1
        if not (ret != AVERROR(EAGAIN)):
          break
    while true:
      if d.queue.nb_packets == 0: discard
        # SDL_CondSignal(d.empty_queue_cond)
      if d.packet_pending != 0:
        av_packet_move_ref(addr(pkt), addr(d.pkt))
        d.packet_pending = 0
      else:
        if packet_queue_get(d.queue, addr(pkt), 1, addr(d.pkt_serial)) < 0:
          return -1
      if d.queue.serial == d.pkt_serial:
        break
      av_packet_unref(addr(pkt))
    #   if not 1:
    #     break
    if pkt.data == flush_pkt.data:
      avcodec_flush_buffers(d.avctx)
      d.finished = 0
      d.next_pts = d.start_pts
      d.next_pts_tb = d.start_pts_tb
    else:
      if d.avctx.codec_type == AVMEDIA_TYPE_SUBTITLE:
        var got_frame: cint = 0
        ret = avcodec_decode_subtitle2(d.avctx, sub, addr(got_frame), addr(pkt))
        if ret < 0:
          ret = AVERROR(EAGAIN)
        else:
          if got_frame != 0 and pkt.data == nil:
            d.packet_pending = 1
            av_packet_move_ref(addr(d.pkt), addr(pkt))
          ret = if got_frame != 0: 0 else: (if pkt.data != nil: AVERROR(EAGAIN) else: AVERROR_EOF)
      else:
        if avcodec_send_packet(d.avctx, addr(pkt)) == AVERROR(EAGAIN):
          echo("Receive_frame and send_packet both returned EAGAIN, which is an API violation.\n")
          d.packet_pending = 1
          av_packet_move_ref(addr(d.pkt), addr(pkt))
      av_packet_unref(addr(pkt))

proc decoder_destroy*(d: ptr Decoder) =
  av_packet_unref(addr(d.pkt))
  avcodec_free_context(addr(d.avctx))

proc frame_queue_unref_item*(vp: ptr Frame) =
  av_frame_unref(vp.frame)
  avsubtitle_free(addr(vp.sub))

proc frame_queue_init*(f: ptr FrameQueue; pktq: ptr PacketQueue; max_size: cint;
                      keep_last: cint): cint =
  var i: cint
  #memset(f, 0, sizeof((FrameQueue)))
#   if not (f.mutex = SDL_CreateMutex()):
#     echo(nil, AV_LOG_FATAL, "SDL_CreateMutex(): %s\n", SDL_GetError())
#     return AVERROR(ENOMEM)
#   if not (f.cond = SDL_CreateCond()):
#     echo(nil, AV_LOG_FATAL, "SDL_CreateCond(): %s\n", SDL_GetError())
#     return AVERROR(ENOMEM)
  f.pktq = pktq
  f.max_size = FFMIN(max_size, FRAME_QUEUE_SIZE)
  f.keep_last = not not keep_last
  i = 0
  while i < f.max_size:
    f.queue[i].frame = av_frame_alloc()
    if f.queue[i].frame == nil:
      return AVERROR(ENOMEM)
    inc(i)
  return 0

proc frame_queue_destory*(f: ptr FrameQueue) =
  var i: cint
  i = 0
  while i < f.max_size:
    var vp: ptr Frame = addr(f.queue[i])
    frame_queue_unref_item(vp)
    av_frame_free(addr(vp.frame))
    inc(i)
#   SDL_DestroyMutex(f.mutex)
#   SDL_DestroyCond(f.cond)

proc frame_queue_signal*(f: ptr FrameQueue) = discard
#   SDL_LockMutex(f.mutex)
#   SDL_CondSignal(f.cond)
#   SDL_UnlockMutex(f.mutex)

proc frame_queue_peek*(f: ptr FrameQueue): ptr Frame =
  return addr(f.queue[(f.rindex + f.rindex_shown) mod f.max_size])

proc frame_queue_peek_next*(f: ptr FrameQueue): ptr Frame =
  return addr(f.queue[(f.rindex + f.rindex_shown + 1) mod f.max_size])

proc frame_queue_peek_last*(f: ptr FrameQueue): ptr Frame =
  return addr(f.queue[f.rindex])

proc frame_queue_peek_writable*(f: ptr FrameQueue): ptr Frame =
  ##  wait until we have space to put a new frame
#   SDL_LockMutex(f.mutex)
#   while f.size >= f.max_size and not f.pktq.abort_request:
#     SDL_CondWait(f.cond, f.mutex)
#   SDL_UnlockMutex(f.mutex)
  if f.pktq.abort_request != 0:
    return nil
  return addr(f.queue[f.windex])

proc frame_queue_peek_readable*(f: ptr FrameQueue): ptr Frame =
  ##  wait until we have a readable a new frame
#   SDL_LockMutex(f.mutex)
#   while f.size - f.rindex_shown <= 0 and not f.pktq.abort_request:
#     SDL_CondWait(f.cond, f.mutex)
#   SDL_UnlockMutex(f.mutex)
  if f.pktq.abort_request != 0:
    return nil
  return addr(f.queue[(f.rindex + f.rindex_shown) mod f.max_size])

proc frame_queue_push*(f: ptr FrameQueue) =
    inc(f.windex)
    if  f.windex == f.max_size:
        f.windex = 0
    #   SDL_LockMutex(f.mutex)
    inc(f.size)
    #   SDL_CondSignal(f.cond)
    #   SDL_UnlockMutex(f.mutex)

proc frame_queue_next*(f: ptr FrameQueue) =
  if f.keep_last != 0 and f.rindex_shown == 0:
    f.rindex_shown = 1
    return
  frame_queue_unref_item(addr(f.queue[f.rindex]))
  inc(f.rindex)
  if f.rindex == f.max_size:
    f.rindex = 0
#   SDL_LockMutex(f.mutex)
  dec(f.size)
#   SDL_CondSignal(f.cond)
#   SDL_UnlockMutex(f.mutex)

##  return the number of undisplayed frames in the queue

proc frame_queue_nb_remaining*(f: ptr FrameQueue): cint =
  return f.size - f.rindex_shown

##  return last shown position

proc frame_queue_last_pos*(f: ptr FrameQueue): int64 =
  var fp: ptr Frame = addr(f.queue[f.rindex])
  if f.rindex_shown != 0 and fp.serial == f.pktq.serial:
    return fp.pos
  else:
    return -1

proc decoder_abort*(d: ptr Decoder; fq: ptr FrameQueue) =
  packet_queue_abort(d.queue)
  frame_queue_signal(fq)
#   SDL_WaitThread(d.decoder_tid, nil)
#   d.decoder_tid = nil
  packet_queue_flush(d.queue)

proc fill_rectangle*(x: cint; y: cint; w: cint; h: cint) {.inline.} =
  var rect: sdl2.Rect
  rect.x = x
  rect.y = y
  rect.w = w
  rect.h = h
  if w != 0  and h != 0:
    fillRect(renderer, addr(rect))

proc realloc_texture*(texture: var TexturePtr; new_format: uint32;new_width: cint; new_height: cint; blendmode: BlendMode;init_texture: cint): cint =
  var format: uint32
  var
    access: cint
    w: cint
    h: cint
  if texture != nil or queryTexture(texture, addr(format), addr(access), addr(w), addr(h)).int < 0 or new_width != w or new_height != h or new_format != format:
    var pixels: pointer
    var pitch: cint
    if texture != nil:
      destroy(texture)
    texture = createTexture(renderer, new_format, SDL_TEXTUREACCESS_STREAMING, new_width,new_height)
    setTextureBlendMode(texture, blendmode)
    if init_texture != 0.cint:
      if lockTexture(texture, nil, addr(pixels), addr(pitch)).int < 0:
        return -1
      #memset(pixels, 0, pitch * new_height)
      unlockTexture(texture)
    echo("Created %dx%d texture with %s.\n", new_width, new_height, getPixelFormatName(new_format))
  return 0

type
  AVRounding* = enum
    AV_ROUND_ZERO = 0,         
    AV_ROUND_INF = 1,           
    AV_ROUND_DOWN = 2,          
    AV_ROUND_UP = 3,            
    AV_ROUND_NEAR_INF = 5,      
    AV_ROUND_PASS_MINMAX = 8192

proc av_rescale_rnd*(a: int64; b: int64; c: int64; rnd: var cint): int64 =
  var r: int64 = 0

  if c <= 0 or b < 0 or not ((rnd.cint and not AV_ROUND_PASS_MINMAX.cint) <= 5 and (rnd.cint and not AV_ROUND_PASS_MINMAX.cint) != 4):
    return int64.low
  if (rnd.cint and AV_ROUND_PASS_MINMAX.cint) != 0:
    if a == int64.low or a == int64.high:
      return a
    rnd.dec AV_ROUND_PASS_MINMAX.cint
  if a < 0:
    var tmp = rnd xor ((rnd shr 1) and 1)
    return - (av_rescale_rnd(-max(a, -int64.high), b, c, tmp))
  if rnd == AV_ROUND_NEAR_INF.cint:
    r = c div 2
  elif (rnd and 1) != 0:
    r = c - 1
  if b <= int.high and c <= int.high:
    if a <= int.high:
      return (a * b + r) div c
    else:
      var ad: int64 = a div c
      var a2: int64 = (a mod c * b + r) div c
      if ad >= int32.high and b != 0 and ad > (int64.high - a2) div b:
        return int64.low
      return ad * b + a2
  else:
    var a0: uint64 = uint64 a and 0xFFFFFFFF
    var a1: uint64 = uint64 a shr 32
    var b0: uint64 = uint64 b and 0xFFFFFFFF
    var b1: uint64 = uint64 b shr 32
    var t1: uint64 = uint64 a0 * b1 + a1 * b0
    var t1a: uint64 = uint64 t1 shl 32
    var i: cint
    a0 = a0 * b0 + t1a
    a1 = a1 * b1 + (t1 shr 32) + (a0 < t1a).uint64
    inc(a0, r.int)
    a1.inc (a0 < r.uint64).int
    i = 63
    while i >= 0:
      a1.inc int a1 + ((a0 shr i) and 1)
      t1.inc t1.int
      if c.uint64 <= a1:
        dec(a1, c.int)
        inc(t1)
      dec(i)
    if t1 > int64.high.uint64:
      return int64.low
    return t1.int64

proc av_rescale*(a: int64; b: int64; c: int64): int64 =
    var tmp = AV_ROUND_NEAR_INF.cint
    return av_rescale_rnd(a, b, c, tmp)




proc calculate_display_rect*(rect: ptr sdl2.Rect; scr_xleft: cint; scr_ytop: cint; scr_width: cint; scr_height: cint; pic_width: cint; pic_height: cint; pic_sar: AVRational) =
  var aspect_ratio = pic_sar
  var
    width: int64
    height: int64
    x: int64
    y: int64
  if av_cmp_q(aspect_ratio, av_make_q(0, 1)) <= 0:
    aspect_ratio = av_make_q(1, 1)
  aspect_ratio = av_mul_q(aspect_ratio, av_make_q(pic_width, pic_height))
  height = scr_height
  width = av_rescale(height, aspect_ratio.num, aspect_ratio.den) and not 1
  if width > scr_width:
    width = scr_width
    height = av_rescale(width, aspect_ratio.den, aspect_ratio.num) and not 1
  x = (scr_width - width) div 2
  y = (scr_height - height) div 2
  rect.x = scr_xleft + x.cint
  rect.y = scr_ytop + y.cint
  rect.w = max(cast[cint](width), 1)
  rect.h = max(cast[cint](height), 1)

proc get_sdl_pix_fmt_and_blendmode*(format: cint; sdl_pix_fmt: ptr uint32;sdl_blendmode: var BlendMode) =
  var i: cint
  sdl_blendmode = BlendMode_None
  sdl_pix_fmt[] = SDL_PIXELFORMAT_UNKNOWN.uint32
  if format == AV_PIX_FMT_RGB32.cint or format == AV_PIX_FMT_RGB32_1.cint or format == AV_PIX_FMT_BGR32.cint or format == AV_PIX_FMT_BGR32_1.cint:
    sdl_blendmode = BlendMode_Blend
  for (format, texture_fmt) in sdl_texture_format_map:
    if format == format:
      sdl_pix_fmt[] = texture_fmt.uint32
      return

proc updateYUVTexture*(texture:var TexturePtr; rect: ptr sdl2.Rect;
                          Yplane: ptr uint8; Ypitch: cint; Uplane: ptr uint8;
                          Upitch: cint; Vplane: ptr uint8; Vpitch: cint): cint {.importc.}

proc upload_texture*(tex: var TexturePtr; frame: ptr AVFrame;img_convert_ctx: ptr ptr SwsContext): cint =
  var ret: cint = 0
  var sdl_pix_fmt: uint32
  var sdl_blendmode: BlendMode
  get_sdl_pix_fmt_and_blendmode(frame.format, addr(sdl_pix_fmt), sdl_blendmode)
  if realloc_texture(tex, if sdl_pix_fmt == SDL_PIXELFORMAT_UNKNOWN.uint32: SDL_PIXELFORMAT_ARGB8888 else: sdl_pix_fmt,
                    frame.width, frame.height, sdl_blendmode, 0) < 0:
    return -1
  case sdl_pix_fmt
  of SDL_PIXELFORMAT_UNKNOWN.uint32:  ##  This should only happen if we are not using avfilter...
    img_convert_ctx[] = sws_getCachedContext(img_convert_ctx[], frame.width,frame.height, frame.format.AVPixelFormat, frame.width, frame.height, AV_PIX_FMT_BGRA,sws_flags.cint, nil, nil, nil)
    if img_convert_ctx[] != nil:
      var pixels: array[4, ptr uint8]
      var pitch: array[4, cint]
      if not lockTexture(tex, cast[ptr sdl2.Rect](nil), cast[ptr pointer](pixels.addr), pitch[0].addr):
        discard sws_scale(img_convert_ctx[], frame.data[0].addr, frame.linesize[0].addr, 0, frame.height, pixels[0].addr, pitch[0].addr)
        unlockTexture(tex)
    else:
      echo("Cannot initialize the conversion context\n")
      ret = -1
  of SDL_PIXELFORMAT_IYUV.uint32:
    if frame.linesize[0] > 0 and frame.linesize[1] > 0 and frame.linesize[2] > 0:
      ret = updateYUVTexture(tex, cast[ptr sdl2.Rect](nil), frame.data[0], frame.linesize[0],frame.data[1], frame.linesize[1], frame.data[2],frame.linesize[2])
    elif frame.linesize[0] < 0 and frame.linesize[1] < 0 and frame.linesize[2] < 0:
      ret = updateYUVTexture(tex, cast[ptr sdl2.Rect](nil), frame.data[0] + frame.linesize[0] * (frame.height - 1), -frame.linesize[0], frame.data[1] + frame.linesize[1] * (AV_CEIL_RSHIFT(frame.height, 1) - 1),-frame.linesize[1], frame.data[2] + frame.linesize[2] * (AV_CEIL_RSHIFT(frame.height, 1) - 1), -frame.linesize[2])
    else:
      echo("Mixed negative and positive linesizes are not supported.\n")
      return -1
  else:
    if frame.linesize[0] < 0:
      ret = cint updateTexture(tex, cast[ptr sdl2.Rect](nil), frame.data[0] + frame.linesize[0] * (frame.height - 1), -frame.linesize[0])
    else:
      ret = cint updateTexture(tex, cast[ptr sdl2.Rect](nil), frame.data[0], frame.linesize[0])
  return ret

type
  SDL_YUV_CONVERSION_MODE* = enum
    SDL_YUV_CONVERSION_JPEG,  ## *< Full range JPEG
    SDL_YUV_CONVERSION_BT601, ## *< BT.601 (the default)
    SDL_YUV_CONVERSION_BT709, ## *< BT.709
    SDL_YUV_CONVERSION_AUTOMATIC ## *< BT.601 for SD content, BT.709 for HD content

var sdl_YUV_ConversionMode*: SDL_YUV_CONVERSION_MODE = SDL_YUV_CONVERSION_BT601

proc SDL_SetYUVConversionMode*(mode: SDL_YUV_CONVERSION_MODE) =
  sdl_YUV_ConversionMode = mode


proc set_sdl_yuv_conversion_mode*(frame: ptr AVFrame) =
  var mode: SDL_YUV_CONVERSION_MODE = SDL_YUV_CONVERSION_AUTOMATIC
  if (frame.format == AV_PIX_FMT_YUV420P.cint or frame.format == AV_PIX_FMT_YUYV422.cint or frame.format == AV_PIX_FMT_UYVY422.cint):
    if frame.color_range == AVCOL_RANGE_JPEG:
      mode = SDL_YUV_CONVERSION_JPEG
    elif frame.colorspace == AVCOL_SPC_BT709:
      mode = SDL_YUV_CONVERSION_BT709
    elif frame.colorspace == AVCOL_SPC_BT470BG or
        frame.colorspace == AVCOL_SPC_SMPTE170M or
        frame.colorspace == AVCOL_SPC_SMPTE240M:
      mode = SDL_YUV_CONVERSION_BT601
  SDL_SetYUVConversionMode(mode)

proc video_image_display*(vs: ptr VideoState) =
  var vp: ptr Frame
  var sp: ptr Frame = nil
  var rect: sdl2.Rect
  vp = frame_queue_peek_last(addr(vs.pictq))
  if vs.subtitle_st != nil:
    if frame_queue_nb_remaining(addr(vs.subpq)) > 0:
      sp = frame_queue_peek(addr(vs.subpq))
      if vp.pts >= sp.pts + (cast[cfloat](sp.sub.start_display_time div 1000)):
        if sp.uploaded == 0:
          var pixels: array[4, ptr uint8]
          var pitch: array[4, cint]
          var i: cint
          if sp.width == 0 or sp.height == 0:
            sp.width = vp.width
            sp.height = vp.height
          if realloc_texture(vs.sub_texture, SDL_PIXELFORMAT_ARGB8888,sp.width, sp.height, BlendMode_Blend, 1) < 0:
            return
          i = 0
          while i.cuint < sp.sub.num_rects:
            var sub_rect: ptr AVSubtitleRect = sp.sub.rects[i]
            sub_rect.x = av_clip_c(sub_rect.x, 0, sp.width)
            sub_rect.y = av_clip_c(sub_rect.y, 0, sp.height)
            sub_rect.w = av_clip_c(sub_rect.w, 0, sp.width - sub_rect.x)
            sub_rect.h = av_clip_c(sub_rect.h, 0, sp.height - sub_rect.y)
            vs.sub_convert_ctx = sws_getCachedContext(vs.sub_convert_ctx,
                sub_rect.w, sub_rect.h, AV_PIX_FMT_PAL8, sub_rect.w, sub_rect.h,
                AV_PIX_FMT_BGRA, 0, nil, nil, nil)
            if vs.sub_convert_ctx == nil:
              echo("Cannot initialize the conversion context\n")
              return
            if lockTexture(vs.sub_texture, cast[ptr sdl2.Rect](sub_rect), cast[ptr pointer](pixels[0].addr), pitch[0].addr).int == 0:
              discard sws_scale(vs.sub_convert_ctx,cast[ptr ptr uint8](sub_rect.data.addr), sub_rect.linesize[0].addr, 0, sub_rect.h, pixels[0].addr, pitch[0].addr)
              unlockTexture(vs.sub_texture)
            inc(i)
          sp.uploaded = 1
      else:
        sp = nil
  calculate_display_rect(addr(rect), vs.xleft, vs.ytop, vs.width, vs.height,vp.width, vp.height, vp.sar)
  if vp.uploaded == 0:
    if upload_texture(vs.vid_texture, vp.frame, addr(vs.img_convert_ctx)) < 0:
      return
    vp.uploaded = 1
    vp.flip_v = cint vp.frame.linesize[0] < 0
  set_sdl_yuv_conversion_mode(vp.frame)
  copyEx(renderer, vs.vid_texture, nil, addr(rect), 0, nil,
                   if vp.flip_v != 0: SDL_FLIP_VERTICAL else: 0)
  set_sdl_yuv_conversion_mode(nil)
  if sp != nil:
    copy(renderer, vs.sub_texture, nil, addr(rect))

proc compute_mod*(a: cint; b: cint): cint {.inline.} =
  return if a < 0: a mod b + b else: a mod b

proc av_gettime*(): int64 =
  # when defined(linux):
  #   var tv: Timeval
  #   gettimeofday(addr(tv), nil)
  #   return cast[int64](tv.tv_sec * 1000000) + tv.tv_usec
  # elif defined(windows):
    var ft: FILETIME
    var t: int64
    GetSystemTimeAsFileTime(addr(ft))
    t = cast[int64](ft.dwHighDateTime) shl 32 or ft.dwLowDateTime
    return t div 10 - 11644473600000000'i64
  # else:
  #   return -1

proc av_gettime_relative*(): int64 =
  return av_gettime() + 42 * 60 * 60 * 1000000

proc video_audio_display*(s: ptr VideoState) =
  var
    i: cint
    i_start: cint
    x: cint
    y1: cint
    y: cint
    ys: cint
    delay: cint
    n: cint
    nb_display_channels: cint
  var
    ch: cint
    channels: cint
    h: cint
    h2: cint
  var time_diff: int64
  var
    rdft_bits: cint
    nb_freq: cint
  rdft_bits = 1
  while (1 shl rdft_bits) < 2 * s.height:
    inc(rdft_bits)
  nb_freq = cint 1 shl (rdft_bits - 1)
  ##  compute display index : center on currently output samples
  channels = s.audio_tgt.channels
  nb_display_channels = channels
  if s.paused == 0:
    var data_used: cint = if s.show_mode == SHOW_MODE_WAVES: s.width else: (2 * nb_freq)
    n = 2 * channels
    delay = s.audio_write_buf_size
    delay = cint delay / n
    ##  to be more precise, we take into account the time spent since
    ## the last buffer computation
    if audio_callback_time != 0:
      time_diff = av_gettime_relative() - audio_callback_time
      dec(delay, (time_diff * s.audio_tgt.freq) div 1000000)
    inc(delay, 2 * data_used)
    if delay < data_used:
      delay = data_used
    x = compute_mod(s.sample_array_index - delay * channels, SAMPLE_ARRAY_SIZE)
    i_start = x
    if s.show_mode == SHOW_MODE_WAVES:
      h = int.low
      i = 0
      while i < 1000:
        var idx: cint = (SAMPLE_ARRAY_SIZE + x - i) mod SAMPLE_ARRAY_SIZE
        var a: cint = s.sample_array[idx]
        var b: cint = s.sample_array[(idx + 4 * channels) mod SAMPLE_ARRAY_SIZE]
        var c: cint = s.sample_array[(idx + 5 * channels) mod SAMPLE_ARRAY_SIZE]
        var d: cint = s.sample_array[(idx + 9 * channels) mod SAMPLE_ARRAY_SIZE]
        var score: cint = a - d
        if h < score and (b xor c) < 0:
          h = score
          i_start = idx
        inc(i, channels)
    s.last_i_start = i_start
  else:
    i_start = s.last_i_start
  if s.show_mode == SHOW_MODE_WAVES:
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255)
    h = s.height div nb_display_channels
    h2 = (h * 9) div 20
    ch = 0
    while ch < nb_display_channels:
      i = i_start + ch
      y1 = s.ytop + ch * h + (h div 2)
      ##  position of center line
      x = 0
      while x < s.width:
        y = (s.sample_array[i] * h2) shr 15
        if y < 0:
          y = -y
          ys = y1 - y
        else:
          ys = y1
        fill_rectangle(s.xleft + x, ys, 1, y)
        inc(i, channels)
        if i >= SAMPLE_ARRAY_SIZE:
          dec(i, SAMPLE_ARRAY_SIZE)
        inc(x)
      inc(ch)
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255)
    ch = 1
    while ch < nb_display_channels:
      y = s.ytop + ch * h
      fill_rectangle(s.xleft, y, s.width, 1)
      inc(ch)
  else:
    if realloc_texture(addr(s.vis_texture), SDL_PIXELFORMAT_ARGB8888, s.width,
                      s.height, SDL_BLENDMODE_NONE, 1) < 0:
      return
    nb_display_channels = min(nb_display_channels, 2)
    if rdft_bits != s.rdft_bits:
      av_rdft_end(s.rdft)
      av_free(s.rdft_data)
      s.rdft = av_rdft_init(rdft_bits, DFT_R2C)
      s.rdft_bits = rdft_bits
      s.rdft_data = av_malloc_array(nb_freq, 4 * sizeof((s.rdft_data[])))
    if not s.rdft or not s.rdft_data:
      echo(nil, AV_LOG_ERROR,
           "Failed to allocate buffers for RDFT, switching to waves display\n")
      s.show_mode = SHOW_MODE_WAVES
    else:
      var data: array[2, ptr FFTSample]
      ##  sdl2.Rect rect = {.x = s->xpos, .y = 0, .w = 1, .h = s->height};
      var pixels: ptr uint32_t
      var pitch: cint
      ch = 0
      while ch < nb_display_channels:
        data[ch] = s.rdft_data + 2 * nb_freq * ch
        i = i_start + ch
        x = 0
        while x < 2 * nb_freq:
          var w: cdouble = (x - nb_freq) * (1.0 div nb_freq)
          data[ch][x] = s.sample_array[i] * (1.0 - w * w)
          inc(i, channels)
          if i >= SAMPLE_ARRAY_SIZE:
            dec(i, SAMPLE_ARRAY_SIZE)
          inc(x)
        av_rdft_calc(s.rdft, data[ch])
        inc(ch)
      if not lockTexture(s.vis_texture, addr(rect), cast[ptr pointer](addr(pixels)),
                      addr(pitch)):
        pitch = pitch shr 2
        inc(pixels, pitch * s.height)
        y = 0
        while y < s.height:
          var w: cdouble = 1 div sqrt(nb_freq)
          var a: cint = sqrt(w *
              sqrt(data[0][2 * y + 0] * data[0][2 * y + 0] +
              data[0][2 * y + 1] * data[0][2 * y + 1]))
          var b: cint = if (nb_display_channels == 2): sqrt(
              w * hypot(data[1][2 * y + 0], data[1][2 * y + 1])) else: a
          a = min(a, 255)
          b = min(b, 255)
          dec(pixels, pitch)
          pixels[] = (a shl 16) + (b shl 8) + ((a + b) shr 1)
          inc(y)
        unlockTexture(s.vis_texture)
      copy(renderer, s.vis_texture, nil, nil)
    if not s.paused:
      inc(s.xpos)
    if s.xpos >= s.width:
      s.xpos = s.xleft

proc stream_component_close*(vs: ptr VideoState; stream_index: cint) =
  var ic: ptr AVFormatContext = vs.ic
  var codecpar: ptr AVCodecParameters
  if stream_index < 0 or stream_index >= ic.nb_streams:
    return
  codecpar = ic.streams[stream_index].codecpar
  case codecpar.codec_type
  of AVMEDIA_TYPE_AUDIO:
    decoder_abort(addr(vs.auddec), addr(vs.sampq))
    SDL_CloseAudioDevice(audio_dev)
    decoder_destroy(addr(vs.auddec))
    swr_free(addr(vs.swr_ctx))
    av_freep(addr(vs.audio_buf1))
    vs.audio_buf1_size = 0
    vs.audio_buf = nil
    if vs.rdft:
      av_rdft_end(vs.rdft)
      av_freep(addr(vs.rdft_data))
      vs.rdft = nil
      vs.rdft_bits = 0
  of AVMEDIA_TYPE_VIDEO:
    decoder_abort(addr(vs.viddec), addr(vs.pictq))
    decoder_destroy(addr(vs.viddec))
  of AVMEDIA_TYPE_SUBTITLE:
    decoder_abort(addr(vs.subdec), addr(vs.subpq))
    decoder_destroy(addr(vs.subdec))
  else:
    nil
  ic.streams[stream_index].`discard` = AVDISCARD_ALL
  case codecpar.codec_type
  of AVMEDIA_TYPE_AUDIO:
    vs.audio_st = nil
    vs.audio_stream = -1
  of AVMEDIA_TYPE_VIDEO:
    vs.video_st = nil
    vs.video_stream = -1
  of AVMEDIA_TYPE_SUBTITLE:
    vs.subtitle_st = nil
    vs.subtitle_stream = -1
  else:
    nil

proc stream_close*(vs: ptr VideoState) =
  ##  XXX: use a special url_shutdown call to abort parse cleanly
  vs.abort_request = 1
  SDL_WaitThread(vs.read_tid, nil)
  ##  close each stream
  if vs.audio_stream >= 0:
    stream_component_close(vs, vs.audio_stream)
  if vs.video_stream >= 0:
    stream_component_close(vs, vs.video_stream)
  if vs.subtitle_stream >= 0:
    stream_component_close(vs, vs.subtitle_stream)
  avformat_close_input(addr(vs.ic))
  packet_queue_destroy(addr(vs.videoq))
  packet_queue_destroy(addr(vs.audioq))
  packet_queue_destroy(addr(vs.subtitleq))
  ##  free all pictures
  frame_queue_destory(addr(vs.pictq))
  frame_queue_destory(addr(vs.sampq))
  frame_queue_destory(addr(vs.subpq))
  SDL_DestroyCond(vs.continue_read_thread)
  sws_freeContext(vs.img_convert_ctx)
  sws_freeContext(vs.sub_convert_ctx)
  av_free(vs.filename)
  if vs.vis_texture:
    SDL_DestroyTexture(vs.vis_texture)
  if vs.vid_texture:
    SDL_DestroyTexture(vs.vid_texture)
  if vs.sub_texture:
    SDL_DestroyTexture(vs.sub_texture)
  av_free(vs)

proc do_exit*(vs: ptr VideoState) =
  if vs:
    stream_close(vs)
  if renderer:
    SDL_DestroyRenderer(renderer)
  if window:
    SDL_DestroyWindow(window)
  uninit_opts()
  when CONFIG_AVFILTER:
    av_freep(addr(vfilters_list))
  avformat_network_deinit()
  if show_status:
    printf("\n")
  SDL_Quit()
  echo(nil, AV_LOG_QUIET, "%s", "")
  exit(0)

proc sigterm_handler*(sig: cint) =
  exit(123)

proc set_default_window_size*(width: cint; height: cint; sar: AVRational) =
  var rect: sdl2.Rect
  var max_width: cint = if screen_width: screen_width else: int.high
  var max_height: cint = if screen_height: screen_height else: int.high
  if max_width == int.high and max_height == int.high:
    max_height = height
  calculate_display_rect(addr(rect), 0, 0, max_width, max_height, width, height, sar)
  default_width = rect.w
  default_height = rect.h

proc video_open*(vs: ptr VideoState): cint =
  var
    w: cint
    h: cint
  w = if screen_width: screen_width else: default_width
  h = if screen_height: screen_height else: default_height
  if not window_title:
    window_title = input_filename
  SDL_SetWindowTitle(window, window_title)
  SDL_SetWindowSize(window, w, h)
  SDL_SetWindowPosition(window, screen_left, screen_top)
  if is_full_screen:
    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP)
  SDL_ShowWindow(window)
  vs.width = w
  vs.height = h
  return 0

##  display the current picture, if any

proc video_display*(vs: ptr VideoState) =
  if not vs.width:
    video_open(vs)
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255)
  SDL_RenderClear(renderer)
  if vs.audio_st and vs.show_mode != SHOW_MODE_VIDEO:
    video_audio_display(vs)
  elif vs.video_st:
    video_image_display(vs)
  SDL_RenderPresent(renderer)



proc get_clock*(c: ptr Clock): cdouble =
  if c.queue_serial[] != c.serial:
    return NAN
  if c.paused:
    return c.pts
  else:
    var time: cdouble = av_gettime_relative() div 1000000.0
    return c.pts_drift + time - (time - c.last_updated) * (1.0 - c.speed)

proc set_clock_at*(c: ptr Clock; pts: cdouble; serial: cint; time: cdouble) =
  c.pts = pts
  c.last_updated = time
  c.pts_drift = c.pts - time
  c.serial = serial

proc set_clock*(c: ptr Clock; pts: cdouble; serial: cint) =
  var time: cdouble = av_gettime_relative() div 1000000.0
  set_clock_at(c, pts, serial, time)

proc set_clock_speed*(c: ptr Clock; speed: cdouble) =
  set_clock(c, get_clock(c), c.serial)
  c.speed = speed

proc init_clock*(c: ptr Clock; queue_serial: ptr cint) =
  c.speed = 1.0
  c.paused = 0
  c.queue_serial = queue_serial
  set_clock(c, NAN, -1)

proc sync_clock_to_slave*(c: ptr Clock; slave: ptr Clock) =
  var clock: cdouble = get_clock(c)
  var slave_clock: cdouble = get_clock(slave)
  if not isnan(slave_clock) and
      (isnan(clock) or fabs(clock - slave_clock) > AV_NOSYNC_THRESHOLD):
    set_clock(c, slave_clock, slave.serial)

proc get_master_sync_type*(vs: ptr VideoState): cint =
  if vs.av_sync_type == AV_SYNC_VIDEO_MASTER:
    if vs.video_st:
      return AV_SYNC_VIDEO_MASTER
    else:
      return AV_SYNC_AUDIO_MASTER
  elif vs.av_sync_type == AV_SYNC_AUDIO_MASTER:
    if vs.audio_st:
      return AV_SYNC_AUDIO_MASTER
    else:
      return AV_SYNC_EXTERNAL_CLOCK
  else:
    return AV_SYNC_EXTERNAL_CLOCK

##  get the current master clock value

proc get_master_clock*(vs: ptr VideoState): cdouble =
  var val: cdouble
  case get_master_sync_type(vs)
  of AV_SYNC_VIDEO_MASTER:
    val = get_clock(addr(vs.vidclk))
  of AV_SYNC_AUDIO_MASTER:
    val = get_clock(addr(vs.audclk))
  else:
    val = get_clock(addr(vs.extclk))
  return val

proc check_external_clock_speed*(vs: ptr VideoState) =
  if vs.video_stream >= 0 and
      vs.videoq.nb_packets <= EXTERNAL_CLOCK_MIN_FRAMES or
      vs.audio_stream >= 0 and
      vs.audioq.nb_packets <= EXTERNAL_CLOCK_MIN_FRAMES:
    set_clock_speed(addr(vs.extclk), max(EXTERNAL_CLOCK_SPEED_MIN,
        vs.extclk.speed - EXTERNAL_CLOCK_SPEED_STEP))
  elif (vs.video_stream < 0 or
      vs.videoq.nb_packets > EXTERNAL_CLOCK_MAX_FRAMES) and
      (vs.audio_stream < 0 or
      vs.audioq.nb_packets > EXTERNAL_CLOCK_MAX_FRAMES):
    set_clock_speed(addr(vs.extclk), FFMIN(EXTERNAL_CLOCK_SPEED_MAX,
        vs.extclk.speed + EXTERNAL_CLOCK_SPEED_STEP))
  else:
    var speed: cdouble = vs.extclk.speed
    if speed != 1.0:
      set_clock_speed(addr(vs.extclk), speed +
          EXTERNAL_CLOCK_SPEED_STEP * (1.0 - speed) div fabs(1.0 - speed))

##  seek in the stream

proc stream_seek*(vs: ptr VideoState; pos: int64; rel: int64; seek_by_bytes: cint) =
  if not vs.seek_req:
    vs.seek_pos = pos
    vs.seek_rel = rel
    vs.seek_flags = vs.seek_flags and not AVSEEK_FLAG_BYTE
    if seek_by_bytes:
      vs.seek_flags = vs.seek_flags or AVSEEK_FLAG_BYTE
    vs.seek_req = 1
    SDL_CondSignal(vs.continue_read_thread)

##  pause or resume the video

proc stream_toggle_pause*(vs: ptr VideoState) =
  if vs.paused:
    inc(vs.frame_timer,
        av_gettime_relative() div 1000000.0 - vs.vidclk.last_updated)
    if vs.read_pause_return != AVERROR(ENOSYS):
      vs.vidclk.paused = 0
    set_clock(addr(vs.vidclk), get_clock(addr(vs.vidclk)), vs.vidclk.serial)
  set_clock(addr(vs.extclk), get_clock(addr(vs.extclk)), vs.extclk.serial)
  vs.paused = vs.audclk.paused = vs.vidclk.paused = vs.extclk.paused = not vs.paused

proc toggle_pause*(vs: ptr VideoState) =
  stream_toggle_pause(vs)
  vs.step = 0

proc toggle_mute*(vs: ptr VideoState) =
  vs.muted = not vs.muted

proc update_volume*(vs: ptr VideoState; sign: cint; step: cdouble) =
  var volume_level: cdouble = if vs.audio_volume: (20 * log(vs.audio_volume div cast[cdouble](SDL_MIX_MAXVOLUME)) div log(10)) else: -1000.0
  var new_volume: cint = lrint(SDL_MIX_MAXVOLUME * pow(10.0, (volume_level + sign * step) div 20.0))
  vs.audio_volume = av_clip_c(if vs.audio_volume == new_volume: (vs.audio_volume + sign) else: new_volume, 0, SDL_MIX_MAXVOLUME)

proc step_to_next_frame*(vs: ptr VideoState) =
  ##  if the stream is paused unpause it, then step
  if vs.paused:
    stream_toggle_pause(vs)
  vs.step = 1

proc compute_target_delay*(delay: cdouble; vs: ptr VideoState): cdouble =
  var
    sync_threshold: cdouble
    diff: cdouble = 0
  ##  update delay to follow master synchronisation source
  if get_master_sync_type(vs) != AV_SYNC_VIDEO_MASTER:
    ##  if video is slave, we try to correct big delays by
    ##            duplicating or deleting a frame
    diff = get_clock(addr(vs.vidclk)) - get_master_clock(vs)
    ##  skip or repeat frame. We take into account the
    ##            delay to compute the threshold. I still don't know
    ##            if it is the best guess
    sync_threshold = max(AV_SYNC_THRESHOLD_MIN,
                         FFMIN(AV_SYNC_THRESHOLD_MAX, delay))
    if not isnan(diff) and fabs(diff) < vs.max_frame_duration:
      if diff <= -sync_threshold:
        delay = max(0, delay + diff)
      elif diff >= sync_threshold and delay > AV_SYNC_FRAMEDUP_THRESHOLD:
        delay = delay + diff
      elif diff >= sync_threshold:
        delay = 2 * delay
  echo(nil, AV_LOG_TRACE, "video: delay=%0.3f A-V=%f\n", delay, -diff)
  return delay

proc vp_duration*(vs: ptr VideoState; vp: ptr Frame; nextvp: ptr Frame): cdouble =
  if vp.serial == nextvp.serial:
    var duration: cdouble = nextvp.pts - vp.pts
    if isnan(duration) or duration <= 0 or duration > vs.max_frame_duration:
      return vp.duration
    else:
      return duration
  else:
    return 0.0

proc update_video_pts*(vs: ptr VideoState; pts: cdouble; pos: int64; serial: cint) =
  ##  update current video pts
  set_clock(addr(vs.vidclk), pts, serial)
  sync_clock_to_slave(addr(vs.extclk), addr(vs.vidclk))

##  called to display each frame

proc video_refresh*(opaque: pointer; remaining_time: ptr cdouble) =
  var vs: ptr VideoState = opaque
  var time: cdouble
  var
    sp: ptr Frame
    sp2: ptr Frame
  if not vs.paused and get_master_sync_type(vs) == AV_SYNC_EXTERNAL_CLOCK and
      vs.realtime:
    check_external_clock_speed(vs)
  if not display_disable and vs.show_mode != SHOW_MODE_VIDEO and vs.audio_st:
    time = av_gettime_relative() div 1000000.0
    if vs.force_refresh or vs.last_vis_time + rdftspeed < time:
      video_display(vs)
      vs.last_vis_time = time
    remaining_time[] = FFMIN(remaining_time[],
                           vs.last_vis_time + rdftspeed - time)
  if vs.video_st:
    if frame_queue_nb_remaining(addr(vs.pictq)) == 0:
      ##  nothing to do, no picture to display in the queue
    else:
      var
        last_duration: cdouble
        duration: cdouble
        delay: cdouble
      var
        vp: ptr Frame
        lastvp: ptr Frame
      ##  dequeue the picture
      lastvp = frame_queue_peek_last(addr(vs.pictq))
      vp = frame_queue_peek(addr(vs.pictq))
      if vp.serial != vs.videoq.serial:
        frame_queue_next(addr(vs.pictq))
        break retry
      if lastvp.serial != vp.serial:
        vs.frame_timer = av_gettime_relative() div 1000000.0
      if vs.paused:
        break display
      last_duration = vp_duration(vs, lastvp, vp)
      delay = compute_target_delay(last_duration, vs)
      time = av_gettime_relative() div 1000000.0
      if time < vs.frame_timer + delay:
        remaining_time[] = FFMIN(vs.frame_timer + delay - time, remaining_time[])
        break display
      inc(vs.frame_timer, delay)
      if delay > 0 and time - vs.frame_timer > AV_SYNC_THRESHOLD_MAX:
        vs.frame_timer = time
      SDL_LockMutex(vs.pictq.mutex)
      if not isnan(vp.pts):
        update_video_pts(vs, vp.pts, vp.pos, vp.serial)
      SDL_UnlockMutex(vs.pictq.mutex)
      if frame_queue_nb_remaining(addr(vs.pictq)) > 1:
        var nextvp: ptr Frame = frame_queue_peek_next(addr(vs.pictq))
        duration = vp_duration(vs, vp, nextvp)
        if not vs.step and
            (framedrop > 0 or
            (framedrop and get_master_sync_type(vs) != AV_SYNC_VIDEO_MASTER)) and
            time > vs.frame_timer + duration:
          inc(vs.frame_drops_late)
          frame_queue_next(addr(vs.pictq))
          break retry
      if vs.subtitle_st:
        while frame_queue_nb_remaining(addr(vs.subpq)) > 0:
          sp = frame_queue_peek(addr(vs.subpq))
          if frame_queue_nb_remaining(addr(vs.subpq)) > 1:
            sp2 = frame_queue_peek_next(addr(vs.subpq))
          else:
            sp2 = nil
          if sp.serial != vs.subtitleq.serial or
              (vs.vidclk.pts >
              (sp.pts + (cast[cfloat](sp.sub.end_display_time div 1000)))) or
              (sp2 and
              vs.vidclk.pts >
              (sp2.pts + (cast[cfloat](sp2.sub.start_display_time div 1000)))):
            if sp.uploaded:
              var i: cint
              i = 0
              while i < sp.sub.num_rects:
                var sub_rect: ptr AVSubtitleRect = sp.sub.rects[i]
                var pixels: ptr uint8
                var
                  pitch: cint
                  j: cint
                if not lockTexture(vs.sub_texture,
                                     cast[ptr sdl2.Rect](sub_rect),
                                     cast[ptr pointer](addr(pixels)), addr(pitch)):
                  j = 0
                  while j < sub_rect.h:
                    #memset(pixels, 0, sub_rect.w shl 2)
                    inc(j)
                    inc(pixels, pitch)
                  unlockTexture(vs.sub_texture)
                inc(i)
            frame_queue_next(addr(vs.subpq))
          else:
            break
      frame_queue_next(addr(vs.pictq))
      vs.force_refresh = 1
      if vs.step and not vs.paused:
        stream_toggle_pause(vs)
    ##  display picture
    if not display_disable and vs.force_refresh and
        vs.show_mode == SHOW_MODE_VIDEO and vs.pictq.rindex_shown:
      video_display(vs)
  vs.force_refresh = 0
  if show_status:
    var buf: AVBPrint
    var last_time: int64
    var cur_time: int64
    var
      aqsize: cint
      vqsize: cint
      sqsize: cint
    var av_diff: cdouble
    cur_time = av_gettime_relative()
    if not last_time or (cur_time - last_time) >= 30000:
      aqsize = 0
      vqsize = 0
      sqsize = 0
      if vs.audio_st:
        aqsize = vs.audioq.size
      if vs.video_st:
        vqsize = vs.videoq.size
      if vs.subtitle_st:
        sqsize = vs.subtitleq.size
      av_diff = 0
      if vs.audio_st and vs.video_st:
        av_diff = get_clock(addr(vs.audclk)) - get_clock(addr(vs.vidclk))
      elif vs.video_st:
        av_diff = get_master_clock(vs) - get_clock(addr(vs.vidclk))
      elif vs.audio_st:
        av_diff = get_master_clock(vs) - get_clock(addr(vs.audclk))
      av_bprint_init(addr(buf), 0, AV_BPRINT_SIZE_AUTOMATIC)
      av_bprintf(addr(buf),
                 "%7.2f %s:%7.3f fd=%4d aq=%5dKB vq=%5dKB sq=%5dB f=%", PRId64,
                 "/%", PRId64, "   \c", get_master_clock(vs), if (
          vs.audio_st and vs.video_st): "A-V" else: (
          if vs.video_st: "M-V" else: (if vs.audio_st: "M-A" else: "   ")),
                 av_diff, vs.frame_drops_early + vs.frame_drops_late,
                 aqsize div 1024, vqsize div 1024, sqsize, if vs.video_st: vs.viddec.avctx.pts_correction_num_faulty_dts else: 0, if vs.video_st: vs.viddec.avctx.pts_correction_num_faulty_pts else: 0)
      if show_status == 1 and AV_LOG_INFO > av_log_get_level():
        fprintf(stderr, "%s", buf.str)
      else:
        echo(nil, AV_LOG_INFO, "%s", buf.str)
      fflush(stderr)
      av_bprint_finalize(addr(buf), nil)
      last_time = cur_time

proc queue_picture*(vs: ptr VideoState; src_frame: ptr AVFrame; pts: cdouble;
                   duration: cdouble; pos: int64; serial: cint): cint =
  var vp: ptr Frame
  if not (vp = frame_queue_peek_writable(addr(vs.pictq))):
    return -1
  vp.sar = src_frame.sample_aspect_ratio
  vp.uploaded = 0
  vp.width = src_frame.width
  vp.height = src_frame.height
  vp.format = src_frame.format
  vp.pts = pts
  vp.duration = duration
  vp.pos = pos
  vp.serial = serial
  set_default_window_size(vp.width, vp.height, vp.sar)
  av_frame_move_ref(vp.frame, src_frame)
  frame_queue_push(addr(vs.pictq))
  return 0

proc get_video_frame*(vs: ptr VideoState; frame: ptr AVFrame): cint =
  var got_picture: cint
  if (got_picture = decoder_decode_frame(addr(vs.viddec), frame, nil)) < 0:
    return -1
  if got_picture:
    var dpts: cdouble = NAN
    if frame.pts != AV_NOPTS_VALUE:
      dpts = av_q2d(vs.video_st.time_base) * frame.pts
    frame.sample_aspect_ratio = av_guess_sample_aspect_ratio(vs.ic,
        vs.video_st, frame)
    if framedrop > 0 or
        (framedrop and get_master_sync_type(vs) != AV_SYNC_VIDEO_MASTER):
      if frame.pts != AV_NOPTS_VALUE:
        var diff: cdouble = dpts - get_master_clock(vs)
        if not isnan(diff) and fabs(diff) < AV_NOSYNC_THRESHOLD and
            diff - vs.frame_last_filter_delay < 0 and
            vs.viddec.pkt_serial == vs.vidclk.serial and
            vs.videoq.nb_packets:
          inc(vs.frame_drops_early)
          av_frame_unref(frame)
          got_picture = 0
  return got_picture

proc configure_filtergraph*(graph: ptr AVFilterGraph; filtergraph: cstring;
                           source_ctx: ptr AVFilterContext;
                           sink_ctx: ptr AVFilterContext): cint =
  var
    ret: cint
    i: cint
  var nb_filters: cint = graph.nb_filters
  var
    outputs: ptr AVFilterInOut = nil
    inputs: ptr AVFilterInOut = nil
  if filtergraph:
    outputs = avfilter_inout_alloc()
    inputs = avfilter_inout_alloc()
    if not outputs or not inputs:
      ret = AVERROR(ENOMEM)
      break fail
    outputs.name = av_strdup("in")
    outputs.filter_ctx = source_ctx
    outputs.pad_idx = 0
    outputs.next = nil
    inputs.name = av_strdup("out")
    inputs.filter_ctx = sink_ctx
    inputs.pad_idx = 0
    inputs.next = nil
    if (ret = avfilter_graph_parse_ptr(graph, filtergraph, addr(inputs),
                                    addr(outputs), nil)) < 0:
      break fail
  else:
    if (ret = avfilter_link(source_ctx, 0, sink_ctx, 0)) < 0:
      break fail
  ##  Reorder the filters to ensure that inputs of the custom filters are merged first
  ##  for (i = 0; i < graph->nb_filters - nb_filters; i++)
  ##      FFSWAP(AVFilterContext *, graph->filters[i], graph->filters[i + nb_filters]);
  ret = avfilter_graph_config(graph, nil)
  avfilter_inout_free(addr(outputs))
  avfilter_inout_free(addr(inputs))
  return ret

proc configure_video_filters*(graph: ptr AVFilterGraph; vs: ptr VideoState;
                             vfilters: cstring; frame: ptr AVFrame): cint =
  var pix_fmts: array[FF_ARRAY_ELEMS(sdl_texture_format_map), AVPixelFormat]
  var sws_flags_str: array[512, char] = ""
  var buffersrc_args: array[256, char]
  var ret: cint
  var
    filt_src: ptr AVFilterContext = nil
    filt_out: ptr AVFilterContext = nil
    last_filter: ptr AVFilterContext = nil
  var codecpar: ptr AVCodecParameters = vs.video_st.codecpar
  var fr: AVRational = av_guess_frame_rate(vs.ic, vs.video_st, nil)
  var e: ptr AVDictionaryEntry = nil
  var nb_pix_fmts: cint = 0
  var
    i: cint
    j: cint
  i = 0
  while i < renderer_info.num_texture_formats:
    j = 0
    while j < FF_ARRAY_ELEMS(sdl_texture_format_map) - 1:
      if renderer_info.texture_formats[i] ==
          sdl_texture_format_map[j].texture_fmt:
        pix_fmts[inc(nb_pix_fmts)] = sdl_texture_format_map[j].format
        break
      inc(j)
    inc(i)
  pix_fmts[nb_pix_fmts] = AV_PIX_FMT_NONE
  while (e = av_dict_get(sws_dict, "", e, AV_DICT_IGNORE_SUFFIX)):
    if not strcmp(e.key, "sws_flags"):
      av_strlcatf(sws_flags_str, sizeof((sws_flags_str)), "%s=%s:", "flags", e.value)
    else:
      av_strlcatf(sws_flags_str, sizeof((sws_flags_str)), "%s=%s:", e.key, e.value)
  if len(sws_flags_str):
    sws_flags_str[len(sws_flags_str) - 1] = '\x00'
  graph.scale_sws_opts = av_strdup(sws_flags_str)
  snprintf(buffersrc_args, sizeof((buffersrc_args)),
           "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
           frame.width, frame.height, frame.format, vs.video_st.time_base.num,
           vs.video_st.time_base.den, codecpar.sample_aspect_ratio.num,
           max(codecpar.sample_aspect_ratio.den, 1))
  if fr.num and fr.den:
    av_strlcatf(buffersrc_args, sizeof((buffersrc_args)), ":frame_rate=%d/%d",
                fr.num, fr.den)
  if (ret = avfilter_graph_create_filter(addr(filt_src),
                                      avfilter_get_by_name("buffer"),
                                      "ffplay_buffer", buffersrc_args, nil, graph)) <
      0:
    break fail
  ret = avfilter_graph_create_filter(addr(filt_out),
                                   avfilter_get_by_name("buffersink"),
                                   "ffplay_buffersink", nil, nil, graph)
  if ret < 0:
    break fail
  if (ret = av_opt_set_int_list(filt_out, "pix_fmts", pix_fmts, AV_PIX_FMT_NONE,
                             AV_OPT_SEARCH_CHILDREN)) < 0:
    break fail
  last_filter = filt_out
  ##  Note: this macro adds a filter before the lastly added filter, so the
  ##  processing order of the filters is in reverse


  if autorotate:
    var theta: cdouble = get_rotation(vs.video_st)
    if fabs(theta - 90) < 1.0:
      INSERT_FILT("transpose", "clock")
    elif fabs(theta - 180) < 1.0:
      INSERT_FILT("hflip", nil)
      INSERT_FILT("vflip", nil)
    elif fabs(theta - 270) < 1.0:
      INSERT_FILT("transpose", "cclock")
    elif fabs(theta) > 1.0:
      var rotate_buf: array[64, char]
      snprintf(rotate_buf, sizeof((rotate_buf)), "%f*PI/180", theta)
      INSERT_FILT("rotate", rotate_buf)
  if (ret = configure_filtergraph(graph, vfilters, filt_src, last_filter)) < 0:
    break fail
  vs.in_video_filter = filt_src
  vs.out_video_filter = filt_out
  return ret

proc configure_audio_filters*(vs: ptr VideoState; afilters: cstring; force_output_format: cint): cint =
  var sample_fmts: UncheckedArray[AVSampleFormat] = [AV_SAMPLE_FMT_S16,
      AV_SAMPLE_FMT_NONE]
  var sample_rates: array[2, cint] = [0, -1]
  var channel_layouts: array[2, int64] = [0, -1]
  var channels: array[2, cint] = [0, -1]
  var
    filt_asrc: ptr AVFilterContext = nil
    filt_asink: ptr AVFilterContext = nil
  var aresample_swr_opts: array[512, char] = ""
  var e: ptr AVDictionaryEntry = nil
  var asrc_args: array[256, char]
  var ret: cint
  avfilter_graph_free(addr(vs.agraph))
  if not (vs.agraph = avfilter_graph_alloc()):
    return AVERROR(ENOMEM)
  vs.agraph.nb_threads = filter_nbthreads
  while (e = av_dict_get(swr_opts, "", e, AV_DICT_IGNORE_SUFFIX)):
    av_strlcatf(aresample_swr_opts, sizeof((aresample_swr_opts)), "%s=%s:", e.key, e.value)
  if len(aresample_swr_opts):
    aresample_swr_opts[len(aresample_swr_opts) - 1] = '\x00'
  av_opt_set(vs.agraph, "aresample_swr_opts", aresample_swr_opts, 0)
  ret = snprintf(asrc_args, sizeof((asrc_args)), "sample_rate=%d:sample_fmt=%s:channels=%d:time_base=%d/%d", vs.audio_filter_src.freq, av_get_sample_fmt_name(vs.audio_filter_src.fmt), vs.audio_filter_src.channels, 1, vs.audio_filter_src.freq)
  if vs.audio_filter_src.channel_layout:
    snprintf(asrc_args + ret, sizeof((asrc_args)) - ret, ":channel_layout=0x%", PRIx64, vs.audio_filter_src.channel_layout)
  ret = avfilter_graph_create_filter(addr(filt_asrc),avfilter_get_by_name("abuffer"), "ffplay_abuffer", asrc_args, nil, vs.agraph)
  if ret < 0:
    goto e
  ret = avfilter_graph_create_filter(addr(filt_asink), avfilter_get_by_name("abuffersink"), "ffplay_abuffersink", nil, nil, vs.agraph)
  if ret < 0:
    goto e
  if (ret = av_opt_set_int_list(filt_asink, "sample_fmts", sample_fmts, AV_SAMPLE_FMT_NONE, AV_OPT_SEARCH_CHILDREN)) < 0:
    goto e
  if (ret = av_opt_set_int(filt_asink, "all_channel_counts", 1, AV_OPT_SEARCH_CHILDREN)) < 0:
    goto e
  if force_output_format:
    channel_layouts[0] = vs.audio_tgt.channel_layout
    channels[0] = if vs.audio_tgt.channel_layout: -1 else: vs.audio_tgt.channels
    sample_rates[0] = vs.audio_tgt.freq
    if (ret = av_opt_set_int(filt_asink, "all_channel_counts", 0, AV_OPT_SEARCH_CHILDREN)) < 0:
      goto e
    if (ret = av_opt_set_int_list(filt_asink, "channel_layouts", channel_layouts, -1,AV_OPT_SEARCH_CHILDREN)) < 0:
      goto e
    if (ret = av_opt_set_int_list(filt_asink, "channel_counts", channels, -1, AV_OPT_SEARCH_CHILDREN)) < 0:
      goto e
    if (ret = av_opt_set_int_list(filt_asink, "sample_rates", sample_rates, -1, AV_OPT_SEARCH_CHILDREN)) < 0:
      goto e
  if (ret = configure_filtergraph(vs.agraph, afilters, filt_asrc, filt_asink)) < 0:
    goto e
  vs.in_audio_filter = filt_asrc
  vs.out_audio_filter = filt_asink
  if ret < 0:
    avfilter_graph_free(addr(vs.agraph))
  return ret

proc audio_thread*(arg: pointer): cint =
  var vs: ptr VideoState = arg
  var frame: ptr AVFrame = av_frame_alloc()
  var af: ptr Frame
  var last_serial: cint = -1
  var dec_channel_layout: int64
  var reconfigure: cint
  var got_frame: cint = 0
  var tb: AVRational
  var ret: cint = 0
  if not frame:
    return AVERROR(ENOMEM)
  while true:
    if (got_frame = decoder_decode_frame(addr(vs.auddec), frame, nil)) < 0:
      break the_end
    if got_frame:
      ##  tb = (AVRational){1, frame->sample_rate};
      dec_channel_layout = get_valid_channel_layout(frame.channel_layout,
          frame.channels)
      reconfigure = cmp_audio_fmts(vs.audio_filter_src.fmt,
                                 vs.audio_filter_src.channels, frame.format,
                                 frame.channels) or
          vs.audio_filter_src.channel_layout != dec_channel_layout or
          vs.audio_filter_src.freq != frame.sample_rate or
          vs.auddec.pkt_serial != last_serial
      if reconfigure:
        var
          buf1: array[1024, char]
          buf2: array[1024, char]
        av_get_channel_layout_string(buf1, sizeof((buf1)), -1,
                                     vs.audio_filter_src.channel_layout)
        av_get_channel_layout_string(buf2, sizeof((buf2)), -1, dec_channel_layout)
        echo(nil, AV_LOG_DEBUG, "Audio frame changed from rate:%d ch:%d fmt:%s layout:%s serial:%d to rate:%d ch:%d fmt:%s layout:%s serial:%d\n",
               vs.audio_filter_src.freq, vs.audio_filter_src.channels,
               av_get_sample_fmt_name(vs.audio_filter_src.fmt), buf1,
               last_serial, frame.sample_rate, frame.channels,
               av_get_sample_fmt_name(frame.format), buf2, vs.auddec.pkt_serial)
        vs.audio_filter_src.fmt = frame.format
        vs.audio_filter_src.channels = frame.channels
        vs.audio_filter_src.channel_layout = dec_channel_layout
        vs.audio_filter_src.freq = frame.sample_rate
        last_serial = vs.auddec.pkt_serial
        if (ret = configure_audio_filters(vs, afilters, 1)) < 0:
          break the_end
      if (ret = av_buffersrc_add_frame(vs.in_audio_filter, frame)) < 0:
        break the_end
      while (ret = av_buffersink_get_frame_flags(vs.out_audio_filter, frame, 0)) >=
          0:
        tb = av_buffersink_get_time_base(vs.out_audio_filter)
        if not (af = frame_queue_peek_writable(addr(vs.sampq))):
          break the_end
        af.pts = if (frame.pts == AV_NOPTS_VALUE): NAN else: frame.pts * av_q2d(tb)
        af.pos = frame.pkt_pos
        af.serial = vs.auddec.pkt_serial
        ##  af->duration = av_q2d((AVRational){frame->nb_samples, frame->sample_rate});
        av_frame_move_ref(af.frame, frame)
        frame_queue_push(addr(vs.sampq))
        if vs.audioq.serial != vs.auddec.pkt_serial:
          break
      if ret == AVERROR_EOF:
        vs.auddec.finished = vs.auddec.pkt_serial
    if not (ret >= 0 or ret == AVERROR(EAGAIN) or ret == AVERROR_EOF):
      break
  avfilter_graph_free(addr(vs.agraph))
  av_frame_free(addr(frame))
  return ret

proc decoder_start*(d: ptr Decoder; fn: proc (a1: pointer): cint; thread_name: cstring;
                   arg: pointer): cint =
  packet_queue_start(d.queue)
  d.decoder_tid = SDL_CreateThread(fn, thread_name, arg)
  if not d.decoder_tid:
    echo(nil, AV_LOG_ERROR, "SDL_CreateThread(): %s\n", SDL_GetError())
    return AVERROR(ENOMEM)
  return 0

proc video_thread*(arg: pointer): cint =
  var vs: ptr VideoState = arg
  var frame: ptr AVFrame = av_frame_alloc()
  var pts: cdouble
  var duration: cdouble
  var ret: cint
  var tb: AVRational = vs.video_st.time_base
  var frame_rate: AVRational = av_guess_frame_rate(vs.ic, vs.video_st, nil)
  var graph: ptr AVFilterGraph = nil
  var
    filt_out: ptr AVFilterContext = nil
    filt_in: ptr AVFilterContext = nil
  var last_w: cint = 0
  var last_h: cint = 0
  var last_format: AVPixelFormat = -2
  var last_serial: cint = -1
  var last_vfilter_idx: cint = 0
  if not frame:
    return AVERROR(ENOMEM)
  while true:
    ret = get_video_frame(vs, frame)
    if ret < 0:
      break the_end
    if not ret:
      continue
    if last_w != frame.width or last_h != frame.height or last_format != frame.format or
        last_serial != vs.viddec.pkt_serial or
        last_vfilter_idx != vs.vfilter_idx:
      echo(nil, AV_LOG_DEBUG, "Video frame changed from size:%dx%d format:%s serial:%d to size:%dx%d format:%s serial:%d\n",
             last_w, last_h, cast[cstring](av_x_if_null(
          av_get_pix_fmt_name(last_format), "none")), last_serial, frame.width,
             frame.height, cast[cstring](av_x_if_null(
          av_get_pix_fmt_name(frame.format), "none")), vs.viddec.pkt_serial)
      avfilter_graph_free(addr(graph))
      graph = avfilter_graph_alloc()
      if not graph:
        ret = AVERROR(ENOMEM)
        break the_end
      graph.nb_threads = filter_nbthreads
      if (ret = configure_video_filters(graph, vs, if vfilters_list: vfilters_list[
          vs.vfilter_idx] else: nil, frame)) < 0:
        var event: SDL_Event
        event.`type` = FF_QUIT_EVENT
        event.user.data1 = vs
        SDL_PushEvent(addr(event))
        break the_end
      filt_in = vs.in_video_filter
      filt_out = vs.out_video_filter
      last_w = frame.width
      last_h = frame.height
      last_format = frame.format
      last_serial = vs.viddec.pkt_serial
      last_vfilter_idx = vs.vfilter_idx
      frame_rate = av_buffersink_get_frame_rate(filt_out)
    ret = av_buffersrc_add_frame(filt_in, frame)
    if ret < 0:
      break the_end
    while ret >= 0:
      vs.frame_last_returned_time = av_gettime_relative() div 1000000.0
      ret = av_buffersink_get_frame_flags(filt_out, frame, 0)
      if ret < 0:
        if ret == AVERROR_EOF:
          vs.viddec.finished = vs.viddec.pkt_serial
        ret = 0
        break
      vs.frame_last_filter_delay = av_gettime_relative() div 1000000.0 -
          vs.frame_last_returned_time
      if fabs(vs.frame_last_filter_delay) > AV_NOSYNC_THRESHOLD div 10.0:
        vs.frame_last_filter_delay = 0
      tb = av_buffersink_get_time_base(filt_out)
      ##  duration = (frame_rate.num && frame_rate.den ? av_q2d((AVRational){frame_rate.den, frame_rate.num}) : 0);
      pts = if (frame.pts == AV_NOPTS_VALUE): NAN else: frame.pts * av_q2d(tb)
      ret = queue_picture(vs, frame, pts, duration, frame.pkt_pos,
                        vs.viddec.pkt_serial)
      av_frame_unref(frame)
      if vs.videoq.serial != vs.viddec.pkt_serial:
        break
    if ret < 0:
      break the_end
  when CONFIG_AVFILTER:
    avfilter_graph_free(addr(graph))
  av_frame_free(addr(frame))
  return 0

proc subtitle_thread*(arg: pointer): cint =
  var vs: ptr VideoState = arg
  var sp: ptr Frame
  var got_subtitle: cint
  var pts: cdouble
  while true:
    if not (sp = frame_queue_peek_writable(addr(vs.subpq))):
      return 0
    if (got_subtitle = decoder_decode_frame(addr(vs.subdec), nil, addr(sp.sub))) <
        0:
      break
    pts = 0
    if got_subtitle and sp.sub.format == 0:
      if sp.sub.pts != AV_NOPTS_VALUE:
        pts = sp.sub.pts div cast[cdouble](AV_TIME_BASE)
      sp.pts = pts
      sp.serial = vs.subdec.pkt_serial
      sp.width = vs.subdec.avctx.width
      sp.height = vs.subdec.avctx.height
      sp.uploaded = 0
      ##  now we can update the picture count
      frame_queue_push(addr(vs.subpq))
    elif got_subtitle:
      avsubtitle_free(addr(sp.sub))
  return 0

##  copy samples for viewing in editor window

proc update_sample_display*(vs: ptr VideoState; samples: ptr cshort;
                           samples_size: cint) =
  var
    size: cint
    len: cint
  size = samples_size div sizeof((short))
  while size > 0:
    len = SAMPLE_ARRAY_SIZE - vs.sample_array_index
    if len > size:
      len = size
    memcpy(vs.sample_array + vs.sample_array_index, samples,
           len * sizeof((short)))
    inc(samples, len)
    inc(vs.sample_array_index, len)
    if vs.sample_array_index >= SAMPLE_ARRAY_SIZE:
      vs.sample_array_index = 0
    dec(size, len)

##  return the wanted number of samples to get better sync if sync_type is video
##  or external master clock

proc synchronize_audio*(vs: ptr VideoState; nb_samples: cint): cint =
  var wanted_nb_samples: cint = nb_samples
  ##  if not master, then we try to remove or add samples to correct the clock
  if get_master_sync_type(vs) != AV_SYNC_AUDIO_MASTER:
    var
      diff: cdouble
      avg_diff: cdouble
    var
      min_nb_samples: cint
      max_nb_samples: cint
    diff = get_clock(addr(vs.audclk)) - get_master_clock(vs)
    if not isnan(diff) and fabs(diff) < AV_NOSYNC_THRESHOLD:
      vs.audio_diff_cum = diff + vs.audio_diff_avg_coef * vs.audio_diff_cum
      if vs.audio_diff_avg_count < AUDIO_DIFF_AVG_NB:
        ##  not enough measures to have a correct estimate
        inc(vs.audio_diff_avg_count)
      else:
        ##  estimate the A-V difference
        avg_diff = vs.audio_diff_cum * (1.0 - vs.audio_diff_avg_coef)
        if fabs(avg_diff) >= vs.audio_diff_threshold:
          wanted_nb_samples = nb_samples + (int)(diff * vs.audio_src.freq)
          min_nb_samples = ((nb_samples * (100 - SAMPLE_CORRECTION_PERCENT_MAX) div
              100))
          max_nb_samples = ((nb_samples * (100 + SAMPLE_CORRECTION_PERCENT_MAX) div
              100))
          wanted_nb_samples = av_clip_c(wanted_nb_samples, min_nb_samples,
                                    max_nb_samples)
        echo(nil, AV_LOG_TRACE,
               "diff=%f adiff=%f sample_diff=%d apts=%0.3f %f\n", diff, avg_diff,
               wanted_nb_samples - nb_samples, vs.audio_clock,
               vs.audio_diff_threshold)
    else:
      ##  too big difference : may be initial PTS errors, so
      ##                reset A-V filter
      vs.audio_diff_avg_count = 0
      vs.audio_diff_cum = 0
  return wanted_nb_samples

## *
##  Decode one audio frame and return its uncompressed size.
##
##  The processed audio frame is decoded, converted if required, and
##  stored in is->audio_buf, with size in bytes given by the return
##  value.
##

proc audio_decode_frame*(vs: ptr VideoState): cint =
  var
    data_size: cint
    resampled_data_size: cint
  var dec_channel_layout: int64
  var audio_clock0: cdouble
  var wanted_nb_samples: cint
  var af: ptr Frame
  if vs.paused:
    return -1
  while true:
    while frame_queue_nb_remaining(addr(vs.sampq)) == 0:
      if (av_gettime_relative() - audio_callback_time) >
          1000000 * vs.audio_hw_buf_size div vs.audio_tgt.bytes_per_sec div 2:
        return -1
      av_usleep(1000)
    if not (af = frame_queue_peek_readable(addr(vs.sampq))):
      return -1
    frame_queue_next(addr(vs.sampq))
    if not (af.serial != vs.audioq.serial):
      break
  data_size = av_samples_get_buffer_size(nil, af.frame.channels,
                                       af.frame.nb_samples, af.frame.format, 1)
  dec_channel_layout = if (af.frame.channel_layout and
      af.frame.channels ==
      av_get_channel_layout_nb_channels(af.frame.channel_layout)): af.frame.channel_layout else: av_get_default_channel_layout(
      af.frame.channels)
  wanted_nb_samples = synchronize_audio(vs, af.frame.nb_samples)
  if af.frame.format != vs.audio_src.fmt or
      dec_channel_layout != vs.audio_src.channel_layout or
      af.frame.sample_rate != vs.audio_src.freq or
      (wanted_nb_samples != af.frame.nb_samples and not vs.swr_ctx):
    swr_free(addr(vs.swr_ctx))
    vs.swr_ctx = swr_alloc_set_opts(nil, vs.audio_tgt.channel_layout,
                                    vs.audio_tgt.fmt, vs.audio_tgt.freq,
                                    dec_channel_layout, af.frame.format,
                                    af.frame.sample_rate, 0, nil)
    if not vs.swr_ctx or swr_init(vs.swr_ctx) < 0:
      echo(nil, AV_LOG_ERROR, "Cannot create sample rate converter for conversion of %d Hz %s %d channels to %d Hz %s %d channels!\n",
             af.frame.sample_rate, av_get_sample_fmt_name(af.frame.format),
             af.frame.channels, vs.audio_tgt.freq,
             av_get_sample_fmt_name(vs.audio_tgt.fmt), vs.audio_tgt.channels)
      swr_free(addr(vs.swr_ctx))
      return -1
    vs.audio_src.channel_layout = dec_channel_layout
    vs.audio_src.channels = af.frame.channels
    vs.audio_src.freq = af.frame.sample_rate
    vs.audio_src.fmt = af.frame.format
  if vs.swr_ctx:
    var `in`: ptr ptr uint8 = cast[ptr ptr uint8](af.frame.extended_data)
    var `out`: ptr ptr uint8 = addr(vs.audio_buf1)
    var out_count: cint = cast[int64](wanted_nb_samples * vs.audio_tgt.freq div
        af.frame.sample_rate) + 256
    var out_size: cint = av_samples_get_buffer_size(nil, vs.audio_tgt.channels,
        out_count, vs.audio_tgt.fmt, 0)
    var len2: cint
    if out_size < 0:
      echo(nil, AV_LOG_ERROR, "av_samples_get_buffer_size() failed\n")
      return -1
    if wanted_nb_samples != af.frame.nb_samples:
      if swr_set_compensation(vs.swr_ctx, (
          wanted_nb_samples - af.frame.nb_samples) * vs.audio_tgt.freq div
          af.frame.sample_rate, wanted_nb_samples * vs.audio_tgt.freq div
          af.frame.sample_rate) < 0:
        echo(nil, AV_LOG_ERROR, "swr_set_compensation() failed\n")
        return -1
    av_fast_malloc(addr(vs.audio_buf1), addr(vs.audio_buf1_size), out_size)
    if not vs.audio_buf1:
      return AVERROR(ENOMEM)
    len2 = swr_convert(vs.swr_ctx, `out`, out_count, `in`, af.frame.nb_samples)
    if len2 < 0:
      echo(nil, AV_LOG_ERROR, "swr_convert() failed\n")
      return -1
    if len2 == out_count:
      echo(nil, AV_LOG_WARNING, "audio buffer is probably too small\n")
      if swr_init(vs.swr_ctx) < 0:
        swr_free(addr(vs.swr_ctx))
    vs.audio_buf = vs.audio_buf1
    resampled_data_size = len2 * vs.audio_tgt.channels *
        av_get_bytes_per_sample(vs.audio_tgt.fmt)
  else:
    vs.audio_buf = af.frame.data[0]
    resampled_data_size = data_size
  audio_clock0 = vs.audio_clock
  ##  update the audio clock with the pts
  if not isnan(af.pts):
    vs.audio_clock = af.pts +
        cast[cdouble](af.frame.nb_samples div af.frame.sample_rate)
  else:
    vs.audio_clock = NAN
  vs.audio_clock_serial = af.serial
  return resampled_data_size

##  prepare a new audio buffer

proc sdl_audio_callback*(opaque: pointer; stream: ptr uint8; len: cint) =
  var vs: ptr VideoState = opaque
  var
    audio_size: cint
    len1: cint
  audio_callback_time = av_gettime_relative()
  while len > 0:
    if vs.audio_buf_index >= vs.audio_buf_size:
      audio_size = audio_decode_frame(vs)
      if audio_size < 0:
        ##  if error, just output silence
        vs.audio_buf = nil
        vs.audio_buf_size = SDL_AUDIO_MIN_BUFFER_SIZE div
            vs.audio_tgt.frame_size * vs.audio_tgt.frame_size
      else:
        if vs.show_mode != SHOW_MODE_VIDEO:
          update_sample_display(vs, cast[ptr int16](vs.audio_buf), audio_size)
        vs.audio_buf_size = audio_size
      vs.audio_buf_index = 0
    len1 = vs.audio_buf_size - vs.audio_buf_index
    if len1 > len:
      len1 = len
    if not vs.muted and vs.audio_buf and
        vs.audio_volume == SDL_MIX_MAXVOLUME:
      memcpy(stream, cast[ptr uint8](vs.audio_buf) + vs.audio_buf_index, len1)
    else:
      #memset(stream, 0, len1)
      if not vs.muted and vs.audio_buf:
        SDL_MixAudioFormat(stream, cast[ptr uint8](vs.audio_buf) +
            vs.audio_buf_index, AUDIO_S16SYS, len1, vs.audio_volume)
    dec(len, len1)
    inc(stream, len1)
    inc(vs.audio_buf_index, len1)
  vs.audio_write_buf_size = vs.audio_buf_size - vs.audio_buf_index
  ##  Let's assume the audio driver that is used by SDL has two periods.
  if not isnan(vs.audio_clock):
    set_clock_at(addr(vs.audclk), vs.audio_clock -
        (double)(2 * vs.audio_hw_buf_size + vs.audio_write_buf_size) div
        vs.audio_tgt.bytes_per_sec, vs.audio_clock_serial,
                 audio_callback_time div 1000000.0)
    sync_clock_to_slave(addr(vs.extclk), addr(vs.audclk))

proc audio_open*(opaque: pointer; wanted_channel_layout: int64;
                wanted_nb_channels: cint; wanted_sample_rate: cint;
                audio_hw_params: ptr AudioParams): cint =
  var
    wanted_spec: SDL_AudioSpec
    spec: SDL_AudioSpec
  var env: cstring
  var next_nb_channels: UncheckedArray[cint] = [0, 0, 1, 6, 2, 6, 4, 6]
  var next_sample_rates: UncheckedArray[cint] = [0, 44100, 48000, 96000, 192000]
  var next_sample_rate_idx: cint = FF_ARRAY_ELEMS(next_sample_rates) - 1
  env = SDL_getenv("SDL_AUDIO_CHANNELS")
  if env:
    wanted_nb_channels = atoi(env)
    wanted_channel_layout = av_get_default_channel_layout(wanted_nb_channels)
  if not wanted_channel_layout or
      wanted_nb_channels !=
      av_get_channel_layout_nb_channels(wanted_channel_layout):
    wanted_channel_layout = av_get_default_channel_layout(wanted_nb_channels)
    wanted_channel_layout = wanted_channel_layout and
        not AV_CH_LAYOUT_STEREO_DOWNMIX
  wanted_nb_channels = av_get_channel_layout_nb_channels(wanted_channel_layout)
  wanted_spec.channels = wanted_nb_channels
  wanted_spec.freq = wanted_sample_rate
  if wanted_spec.freq <= 0 or wanted_spec.channels <= 0:
    echo(nil, AV_LOG_ERROR, "Invalid sample rate or channel count!\n")
    return -1
  while next_sample_rate_idx and
      next_sample_rates[next_sample_rate_idx] >= wanted_spec.freq:
    dec(next_sample_rate_idx)
  wanted_spec.format = AUDIO_S16SYS
  wanted_spec.silence = 0
  wanted_spec.samples = max(SDL_AUDIO_MIN_BUFFER_SIZE, 2 shl
      av_log2(wanted_spec.freq div SDL_AUDIO_MAX_CALLBACKS_PER_SEC))
  wanted_spec.callback = sdl_audio_callback
  wanted_spec.userdata = opaque
  while not (audio_dev = SDL_OpenAudioDevice(nil, 0, addr(wanted_spec), addr(spec),
      SDL_AUDIO_ALLOW_FREQUENCY_CHANGE or SDL_AUDIO_ALLOW_CHANNELS_CHANGE)):
    echo(nil, AV_LOG_WARNING, "SDL_OpenAudio (%d channels, %d Hz): %s\n",
           wanted_spec.channels, wanted_spec.freq, SDL_GetError())
    wanted_spec.channels = next_nb_channels[FFMIN(7, wanted_spec.channels)]
    if not wanted_spec.channels:
      wanted_spec.freq = next_sample_rates[dec(next_sample_rate_idx)]
      wanted_spec.channels = wanted_nb_channels
      if not wanted_spec.freq:
        echo(nil, AV_LOG_ERROR,
               "No more combinations to try, audio open failed\n")
        return -1
    wanted_channel_layout = av_get_default_channel_layout(wanted_spec.channels)
  if spec.format != AUDIO_S16SYS:
    echo(nil, AV_LOG_ERROR, "SDL advised audio format %d is not supported!\n",
           spec.format)
    return -1
  if spec.channels != wanted_spec.channels:
    wanted_channel_layout = av_get_default_channel_layout(spec.channels)
    if not wanted_channel_layout:
      echo(nil, AV_LOG_ERROR,
             "SDL advised channel count %d is not supported!\n", spec.channels)
      return -1
  audio_hw_params.fmt = AV_SAMPLE_FMT_S16
  audio_hw_params.freq = spec.freq
  audio_hw_params.channel_layout = wanted_channel_layout
  audio_hw_params.channels = spec.channels
  audio_hw_params.frame_size = av_samples_get_buffer_size(nil,
      audio_hw_params.channels, 1, audio_hw_params.fmt, 1)
  audio_hw_params.bytes_per_sec = av_samples_get_buffer_size(nil,
      audio_hw_params.channels, audio_hw_params.freq, audio_hw_params.fmt, 1)
  if audio_hw_params.bytes_per_sec <= 0 or audio_hw_params.frame_size <= 0:
    echo(nil, AV_LOG_ERROR, "av_samples_get_buffer_size failed\n")
    return -1
  return spec.size

##  open a given stream. Return 0 if OK

proc stream_component_open*(vs: ptr VideoState; stream_index: cint): cint =
  var ic: ptr AVFormatContext = vs.ic
  var avctx: ptr AVCodecContext
  var codec: ptr AVCodec
  var forced_codec_name: cstring = nil
  var opts: ptr AVDictionary = nil
  var t: ptr AVDictionaryEntry = nil
  var
    sample_rate: cint
    nb_channels: cint
  var channel_layout: int64
  var ret: cint = 0
  var stream_lowres: cint = lowres
  if stream_index < 0 or stream_index >= ic.nb_streams:
    return -1
  avctx = avcodec_alloc_context3(nil)
  if not avctx:
    return AVERROR(ENOMEM)
  ret = avcodec_parameters_to_context(avctx, ic.streams[stream_index].codecpar)
  if ret < 0:
    break fail
  avctx.pkt_timebase = ic.streams[stream_index].time_base
  codec = avcodec_find_decoder(avctx.codec_id)
  case avctx.codec_type
  of AVMEDIA_TYPE_AUDIO:
    vs.last_audio_stream = stream_index
    forced_codec_name = audio_codec_name
  of AVMEDIA_TYPE_SUBTITLE:
    vs.last_subtitle_stream = stream_index
    forced_codec_name = subtitle_codec_name
  of AVMEDIA_TYPE_VIDEO:
    vs.last_video_stream = stream_index
    forced_codec_name = video_codec_name
  if forced_codec_name:
    codec = avcodec_find_decoder_by_name(forced_codec_name)
  if not codec:
    if forced_codec_name:
      echo(nil, AV_LOG_WARNING, "No codec could be found with name \'%s\'\n",
             forced_codec_name)
    else:
      echo(nil, AV_LOG_WARNING, "No decoder could be found for codec %s\n",
             avcodec_get_name(avctx.codec_id))
    ret = AVERROR(EINVAL)
    break fail
  avctx.codec_id = codec.id
  if stream_lowres > codec.max_lowres:
    echo(avctx, AV_LOG_WARNING,
           "The maximum value for lowres supported by the decoder is %d\n",
           codec.max_lowres)
    stream_lowres = codec.max_lowres
  avctx.lowres = stream_lowres
  if fast:
    avctx.flags2 = avctx.flags2 or AV_CODEC_FLAG2_FAST
  opts = filter_codec_opts(codec_opts, avctx.codec_id, ic, ic.streams[stream_index],
                         codec)
  if not av_dict_get(opts, "threads", nil, 0):
    av_dict_set(addr(opts), "threads", "auto", 0)
  if stream_lowres:
    av_dict_set_int(addr(opts), "lowres", stream_lowres, 0)
  if avctx.codec_type == AVMEDIA_TYPE_VIDEO or
      avctx.codec_type == AVMEDIA_TYPE_AUDIO:
    av_dict_set(addr(opts), "refcounted_frames", "1", 0)
  if (ret = avcodec_open2(avctx, codec, addr(opts))) < 0:
    break fail
  if (t = av_dict_get(opts, "", nil, AV_DICT_IGNORE_SUFFIX)):
    echo(nil, AV_LOG_ERROR, "Option %s not found.\n", t.key)
    ret = AVERROR_OPTION_NOT_FOUND
    break fail
  vs.eof = 0
  ic.streams[stream_index].`discard` = AVDISCARD_DEFAULT
  case avctx.codec_type
  of AVMEDIA_TYPE_AUDIO:
    var sink: ptr AVFilterContext
    vs.audio_filter_src.freq = avctx.sample_rate
    vs.audio_filter_src.channels = avctx.channels
    vs.audio_filter_src.channel_layout = get_valid_channel_layout(
        avctx.channel_layout, avctx.channels)
    vs.audio_filter_src.fmt = avctx.sample_fmt
    if (ret = configure_audio_filters(vs, afilters, 0)) < 0:
      break fail
    sink = vs.out_audio_filter
    sample_rate = av_buffersink_get_sample_rate(sink)
    nb_channels = av_buffersink_get_channels(sink)
    channel_layout = av_buffersink_get_channel_layout(sink)
    ##  prepare audio output
    if (ret = audio_open(vs, channel_layout, nb_channels, sample_rate,
                      addr(vs.audio_tgt))) < 0:
      break fail
    vs.audio_hw_buf_size = ret
    vs.audio_src = vs.audio_tgt
    vs.audio_buf_size = 0
    vs.audio_buf_index = 0
    ##  init averaging filter
    vs.audio_diff_avg_coef = exp(log(0.01) div AUDIO_DIFF_AVG_NB)
    vs.audio_diff_avg_count = 0
    ##  since we do not have a precise anough audio FIFO fullness,
    ##            we correct audio sync only if larger than this threshold
    vs.audio_diff_threshold = (double)(vs.audio_hw_buf_size) div
        vs.audio_tgt.bytes_per_sec
    vs.audio_stream = stream_index
    vs.audio_st = ic.streams[stream_index]
    decoder_init(addr(vs.auddec), avctx, addr(vs.audioq),
                 vs.continue_read_thread)
    if (vs.ic.iformat.flags and
        (AVFMT_NOBINSEARCH or AVFMT_NOGENSEARCH or AVFMT_NO_BYTE_SEEK)) and
        not vs.ic.iformat.read_seek:
      vs.auddec.start_pts = vs.audio_st.start_time
      vs.auddec.start_pts_tb = vs.audio_st.time_base
    if (ret = decoder_start(addr(vs.auddec), audio_thread, "audio_decoder", vs)) <
        0:
      break `out`
    SDL_PauseAudioDevice(audio_dev, 0)
  of AVMEDIA_TYPE_VIDEO:
    vs.video_stream = stream_index
    vs.video_st = ic.streams[stream_index]
    decoder_init(addr(vs.viddec), avctx, addr(vs.videoq),
                 vs.continue_read_thread)
    if (ret = decoder_start(addr(vs.viddec), video_thread, "video_decoder", vs)) <
        0:
      break `out`
    vs.queue_attachments_req = 1
  of AVMEDIA_TYPE_SUBTITLE:
    vs.subtitle_stream = stream_index
    vs.subtitle_st = ic.streams[stream_index]
    decoder_init(addr(vs.subdec), avctx, addr(vs.subtitleq),
                 vs.continue_read_thread)
    if (ret = decoder_start(addr(vs.subdec), subtitle_thread, "subtitle_decoder",
                         vs)) < 0:
      break `out`
  else:
    nil
  break `out`
  avcodec_free_context(addr(avctx))
  av_dict_free(addr(opts))
  return ret

proc decode_interrupt_cb*(ctx: pointer): cint =
  var vs: ptr VideoState = ctx
  return vs.abort_request

proc stream_has_enough_packets*(st: ptr AVStream; stream_id: cint;
                               queue: ptr PacketQueue): cint =
  return stream_id < 0 or queue.abort_request or
      (st.disposition and AV_DISPOSITION_ATTACHED_PIC) or
      queue.nb_packets > MIN_FRAMES and
      (not queue.duration or av_q2d(st.time_base) * queue.duration > 1.0)

proc is_realtime*(s: ptr AVFormatContext): cint =
  if not strcmp(s.iformat.name, "rtp") or not strcmp(s.iformat.name, "rtsp") or
      not strcmp(s.iformat.name, "sdp"):
    return 1
  if s.pb and (not strncmp(s.url, "rtp:", 4) or not strncmp(s.url, "udp:", 4)):
    return 1
  return 0

##  this thread gets the stream from the disk or the network

proc read_thread*(arg: pointer): cint =
  var vs: ptr VideoState = arg
  var ic: ptr AVFormatContext = nil
  var
    err: cint
    i: cint
    ret: cint
  var st_index: array[AVMEDIA_TYPE_NB, cint]
  var
    pkt1: AVPacket
    pkt: ptr AVPacket = addr(pkt1)
  var stream_start_time: int64
  var pkt_in_play_range: cint = 0
  var t: ptr AVDictionaryEntry
  var wait_mutex: ptr SDL_mutex = SDL_CreateMutex()
  var scan_all_pmts_set: cint = 0
  var pkt_ts: int64
  if not wait_mutex:
    echo(nil, AV_LOG_FATAL, "SDL_CreateMutex(): %s\n", SDL_GetError())
    ret = AVERROR(ENOMEM)
    break fail
  #memset(st_index, -1, sizeof((st_index)))
  vs.eof = 0
  ic = avformat_alloc_context()
  if not ic:
    echo(nil, AV_LOG_FATAL, "Could not allocate context.\n")
    ret = AVERROR(ENOMEM)
    break fail
  ic.interrupt_callback.callback = decode_interrupt_cb
  ic.interrupt_callback.opaque = vs
  if not av_dict_get(format_opts, "scan_all_pmts", nil, AV_DICT_MATCH_CASE):
    av_dict_set(addr(format_opts), "scan_all_pmts", "1", AV_DICT_DONT_OVERWRITE)
    scan_all_pmts_set = 1
  err = avformat_open_input(addr(ic), vs.filename, vs.iformat, addr(format_opts))
  if err < 0:
    print_error(vs.filename, err)
    ret = -1
    break fail
  if scan_all_pmts_set:
    av_dict_set(addr(format_opts), "scan_all_pmts", nil, AV_DICT_MATCH_CASE)
  if (t = av_dict_get(format_opts, "", nil, AV_DICT_IGNORE_SUFFIX)):
    echo(nil, AV_LOG_ERROR, "Option %s not found.\n", t.key)
    ret = AVERROR_OPTION_NOT_FOUND
    break fail
  vs.ic = ic
  if genpts:
    ic.flags = ic.flags or AVFMT_FLAG_GENPTS
  av_format_inject_global_side_data(ic)
  if find_stream_info:
    var opts: ptr ptr AVDictionary = setup_find_stream_info_opts(ic, codec_opts)
    var orig_nb_streams: cint = ic.nb_streams
    err = avformat_find_stream_info(ic, opts)
    i = 0
    while i < orig_nb_streams:
      av_dict_free(addr(opts[i]))
      inc(i)
    av_freep(addr(opts))
    if err < 0:
      echo(nil, AV_LOG_WARNING, "%s: could not find codec parameters\n",
             vs.filename)
      ret = -1
      break fail
  if ic.pb:
    ic.pb.eof_reached = 0
  if seek_by_bytes < 0:
    seek_by_bytes = not not (ic.iformat.flags and AVFMT_TS_DISCONT) and
        strcmp("ogg", ic.iformat.name)
  vs.max_frame_duration = if (ic.iformat.flags and AVFMT_TS_DISCONT): 10.0 else: 3600.0
  if not window_title and (t = av_dict_get(ic.metadata, "title", nil, 0)):
    window_title = av_asprintf("%s - %s", t.value, input_filename)
  if start_time != AV_NOPTS_VALUE:
    var timestamp: int64
    timestamp = start_time
    ##  add the stream start time
    if ic.start_time != AV_NOPTS_VALUE:
      inc(timestamp, ic.start_time)
    ret = avformat_seek_file(ic, -1, int64.low, timestamp, int64.high, 0)
    if ret < 0:
      echo(nil, AV_LOG_WARNING, "%s: could not seek to position %0.3f\n",
             vs.filename, cast[cdouble](timestamp div AV_TIME_BASE))
  vs.realtime = is_realtime(ic)
  if show_status:
    av_dump_format(ic, 0, vs.filename, 0)
  i = 0
  while i < ic.nb_streams:
    var st: ptr AVStream = ic.streams[i]
    var `type`: AVMediaType = st.codecpar.codec_type
    st.`discard` = AVDISCARD_ALL
    if `type` >= 0 and wanted_stream_spec[`type`] and st_index[`type`] == -1:
      if avformat_match_stream_specifier(ic, st, wanted_stream_spec[`type`]) > 0:
        st_index[`type`] = i
    inc(i)
  i = 0
  while i < AVMEDIA_TYPE_NB:
    if wanted_stream_spec[i] and st_index[i] == -1:
      echo(nil, AV_LOG_ERROR,
             "Stream specifier %s does not match any %s stream\n",
             wanted_stream_spec[i], av_get_media_type_string(i))
      st_index[i] = int.high
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
  vs.show_mode = show_mode
  if st_index[AVMEDIA_TYPE_VIDEO] >= 0:
    var st: ptr AVStream = ic.streams[st_index[AVMEDIA_TYPE_VIDEO]]
    var codecpar: ptr AVCodecParameters = st.codecpar
    var sar: AVRational = av_guess_sample_aspect_ratio(ic, st, nil)
    if codecpar.width:
      set_default_window_size(codecpar.width, codecpar.height, sar)
  if st_index[AVMEDIA_TYPE_AUDIO] >= 0:
    stream_component_open(vs, st_index[AVMEDIA_TYPE_AUDIO])
  ret = -1
  if st_index[AVMEDIA_TYPE_VIDEO] >= 0:
    ret = stream_component_open(vs, st_index[AVMEDIA_TYPE_VIDEO])
  if vs.show_mode == SHOW_MODE_NONE:
    vs.show_mode = if ret >= 0: SHOW_MODE_VIDEO else: SHOW_MODE_RDFT
  if st_index[AVMEDIA_TYPE_SUBTITLE] >= 0:
    stream_component_open(vs, st_index[AVMEDIA_TYPE_SUBTITLE])
  if vs.video_stream < 0 and vs.audio_stream < 0:
    echo(nil, AV_LOG_FATAL,
           "Failed to open file \'%s\' or configure filtergraph\n", vs.filename)
    ret = -1
    break fail
  if infinite_buffer < 0 and vs.realtime:
    infinite_buffer = 1
  while true:
    if vs.abort_request:
      break
    if vs.paused != vs.last_paused:
      vs.last_paused = vs.paused
      if vs.paused:
        vs.read_pause_return = av_read_pause(ic)
      else:
        av_read_play(ic)
    if vs.paused and
        (not strcmp(ic.iformat.name, "rtsp") or
        (ic.pb and not strncmp(input_filename, "mmsh:", 5))):
      ##  wait 10 ms to avoid trying to get another packet
      ##  XXX: horrible
      SDL_Delay(10)
      continue
    if vs.seek_req:
      var seek_target: int64 = vs.seek_pos
      var seek_min: int64 = if vs.seek_rel > 0: seek_target - vs.seek_rel + 2 else: int64.low
      var seek_max: int64 = if vs.seek_rel < 0: seek_target - vs.seek_rel - 2 else: int64.high
      ##  FIXME the +-2 is due to rounding being not done in the correct direction in generation
      ##       of the seek_pos/seek_rel variables
      ret = avformat_seek_file(vs.ic, -1, seek_min, seek_target, seek_max,
                             vs.seek_flags)
      if ret < 0:
        echo(nil, AV_LOG_ERROR, "%s: error while seeking\n", vs.ic.url)
      else:
        if vs.audio_stream >= 0:
          packet_queue_flush(addr(vs.audioq))
          packet_queue_put(addr(vs.audioq), addr(flush_pkt))
        if vs.subtitle_stream >= 0:
          packet_queue_flush(addr(vs.subtitleq))
          packet_queue_put(addr(vs.subtitleq), addr(flush_pkt))
        if vs.video_stream >= 0:
          packet_queue_flush(addr(vs.videoq))
          packet_queue_put(addr(vs.videoq), addr(flush_pkt))
        if vs.seek_flags and AVSEEK_FLAG_BYTE:
          set_clock(addr(vs.extclk), NAN, 0)
        else:
          set_clock(addr(vs.extclk),
                    seek_target div cast[cdouble](AV_TIME_BASE), 0)
      vs.seek_req = 0
      vs.queue_attachments_req = 1
      vs.eof = 0
      if vs.paused:
        step_to_next_frame(vs)
    if vs.queue_attachments_req:
      if vs.video_st and
          (vs.video_st.disposition and AV_DISPOSITION_ATTACHED_PIC):
        var copy: AVPacket
        if (ret = av_packet_ref(addr(copy), addr(vs.video_st.attached_pic))) < 0:
          break fail
        packet_queue_put(addr(vs.videoq), addr(copy))
        packet_queue_put_nullpacket(addr(vs.videoq), vs.video_stream)
      vs.queue_attachments_req = 0
    if infinite_buffer < 1 and
        (vs.audioq.size + vs.videoq.size + vs.subtitleq.size > MAX_QUEUE_SIZE or
        (stream_has_enough_packets(vs.audio_st, vs.audio_stream,
                                   addr(vs.audioq)) and
        stream_has_enough_packets(vs.video_st, vs.video_stream,
                                  addr(vs.videoq)) and
        stream_has_enough_packets(vs.subtitle_st, vs.subtitle_stream,
                                  addr(vs.subtitleq)))):
      ##  wait 10 ms
      SDL_LockMutex(wait_mutex)
      SDL_CondWaitTimeout(vs.continue_read_thread, wait_mutex, 10)
      SDL_UnlockMutex(wait_mutex)
      continue
    if not vs.paused and
        (not vs.audio_st or
        (vs.auddec.finished == vs.audioq.serial and
        frame_queue_nb_remaining(addr(vs.sampq)) == 0)) and
        (not vs.video_st or
        (vs.viddec.finished == vs.videoq.serial and
        frame_queue_nb_remaining(addr(vs.pictq)) == 0)):
      if loop != 1 and (not loop or dec(loop)):
        stream_seek(vs, if start_time != AV_NOPTS_VALUE: start_time else: 0, 0, 0)
      elif autoexit:
        ret = AVERROR_EOF
        break fail
    ret = av_read_frame(ic, pkt)
    if ret < 0:
      if (ret == AVERROR_EOF or avio_feof(ic.pb)) and not vs.eof:
        if vs.video_stream >= 0:
          packet_queue_put_nullpacket(addr(vs.videoq), vs.video_stream)
        if vs.audio_stream >= 0:
          packet_queue_put_nullpacket(addr(vs.audioq), vs.audio_stream)
        if vs.subtitle_stream >= 0:
          packet_queue_put_nullpacket(addr(vs.subtitleq), vs.subtitle_stream)
        vs.eof = 1
      if ic.pb and ic.pb.error:
        if autoexit:
          break fail
        else:
          break
      SDL_LockMutex(wait_mutex)
      SDL_CondWaitTimeout(vs.continue_read_thread, wait_mutex, 10)
      SDL_UnlockMutex(wait_mutex)
      continue
    else:
      vs.eof = 0
    ##  check if packet is in play range specified by user, then queue, otherwise discard
    stream_start_time = ic.streams[pkt.stream_index].start_time
    pkt_ts = if pkt.pts == AV_NOPTS_VALUE: pkt.dts else: pkt.pts
    pkt_in_play_range = duration == AV_NOPTS_VALUE or
        (pkt_ts -
        (if stream_start_time != AV_NOPTS_VALUE: stream_start_time else: 0)) *
        av_q2d(ic.streams[pkt.stream_index].time_base) -
        (double)(if start_time != AV_NOPTS_VALUE: start_time else: 0) div 1000000 <=
        (cast[cdouble](duration div 1000000))
    if pkt.stream_index == vs.audio_stream and pkt_in_play_range:
      packet_queue_put(addr(vs.audioq), pkt)
    elif pkt.stream_index == vs.video_stream and pkt_in_play_range and
        not (vs.video_st.disposition and AV_DISPOSITION_ATTACHED_PIC):
      packet_queue_put(addr(vs.videoq), pkt)
    elif pkt.stream_index == vs.subtitle_stream and pkt_in_play_range:
      packet_queue_put(addr(vs.subtitleq), pkt)
    else:
      av_packet_unref(pkt)
  ret = 0
  if ic and not vs.ic:
    avformat_close_input(addr(ic))
  if ret != 0:
    var event: SDL_Event
    event.`type` = FF_QUIT_EVENT
    event.user.data1 = vs
    SDL_PushEvent(addr(event))
  SDL_DestroyMutex(wait_mutex)
  return 0

proc stream_open*(filename: cstring; iformat: ptr AVInputFormat): ptr VideoState =
  var vs: ptr VideoState
  vs = av_mallocz(sizeof((VideoState)))
  if not vs:
    return nil
  vs.last_video_stream = vs.video_stream = -1
  vs.last_audio_stream = vs.audio_stream = -1
  vs.last_subtitle_stream = vs.subtitle_stream = -1
  vs.filename = av_strdup(filename)
  if not vs.filename:
    break fail
  vs.iformat = iformat
  vs.ytop = 0
  vs.xleft = 0
  ##  start video display
  if frame_queue_init(addr(vs.pictq), addr(vs.videoq),
                     VIDEO_PICTURE_QUEUE_SIZE, 1) < 0:
    break fail
  if frame_queue_init(addr(vs.subpq), addr(vs.subtitleq),
                     SUBPICTURE_QUEUE_SIZE, 0) < 0:
    break fail
  if frame_queue_init(addr(vs.sampq), addr(vs.audioq), SAMPLE_QUEUE_SIZE, 1) < 0:
    break fail
  if packet_queue_init(addr(vs.videoq)) < 0 or
      packet_queue_init(addr(vs.audioq)) < 0 or
      packet_queue_init(addr(vs.subtitleq)) < 0:
    break fail
  if not (vs.continue_read_thread = SDL_CreateCond()):
    echo(nil, AV_LOG_FATAL, "SDL_CreateCond(): %s\n", SDL_GetError())
    break fail
  init_clock(addr(vs.vidclk), addr(vs.videoq.serial))
  init_clock(addr(vs.audclk), addr(vs.audioq.serial))
  init_clock(addr(vs.extclk), addr(vs.extclk.serial))
  vs.audio_clock_serial = -1
  if startup_volume < 0:
    echo(nil, AV_LOG_WARNING, "-volume=%d < 0, setting to 0\n", startup_volume)
  if startup_volume > 100:
    echo(nil, AV_LOG_WARNING, "-volume=%d > 100, setting to 100\n",
           startup_volume)
  startup_volume = av_clip_c(startup_volume, 0, 100)
  startup_volume = av_clip_c(SDL_MIX_MAXVOLUME * startup_volume div 100, 0,
                         SDL_MIX_MAXVOLUME)
  vs.audio_volume = startup_volume
  vs.muted = 0
  vs.av_sync_type = av_sync_type
  vs.read_tid = SDL_CreateThread(read_thread, "read_thread", vs)
  if not vs.read_tid:
    echo(nil, AV_LOG_FATAL, "SDL_CreateThread(): %s\n", SDL_GetError())
    stream_close(vs)
    return nil
  return vs

proc stream_cycle_channel*(vs: ptr VideoState; codec_type: cint) =
  var ic: ptr AVFormatContext = vs.ic
  var
    start_index: cint
    stream_index: cint
  var old_index: cint
  var st: ptr AVStream
  var p: ptr AVProgram = nil
  var nb_streams: cint = vs.ic.nb_streams
  if codec_type == AVMEDIA_TYPE_VIDEO:
    start_index = vs.last_video_stream
    old_index = vs.video_stream
  elif codec_type == AVMEDIA_TYPE_AUDIO:
    start_index = vs.last_audio_stream
    old_index = vs.audio_stream
  else:
    start_index = vs.last_subtitle_stream
    old_index = vs.subtitle_stream
  stream_index = start_index
  if codec_type != AVMEDIA_TYPE_VIDEO and vs.video_stream != -1:
    p = av_find_program_from_stream(ic, nil, vs.video_stream)
    if p:
      nb_streams = p.nb_stream_indexes
      start_index = 0
      while start_index < nb_streams:
        if p.stream_index[start_index] == stream_index:
          break
        inc(start_index)
      if start_index == nb_streams:
        start_index = -1
      stream_index = start_index
  while true:
    if inc(stream_index) >= nb_streams:
      if codec_type == AVMEDIA_TYPE_SUBTITLE:
        stream_index = -1
        vs.last_subtitle_stream = -1
        break the_end
      if start_index == -1:
        return
      stream_index = 0
    if stream_index == start_index:
      return
    st = vs.ic.streams[if p: p.stream_index[stream_index] else: stream_index]
    if st.codecpar.codec_type == codec_type:
      ##  check that parameters are OK
      case codec_type
      of AVMEDIA_TYPE_AUDIO:
        if st.codecpar.sample_rate != 0 and st.codecpar.channels != 0:
          break the_end
      of AVMEDIA_TYPE_VIDEO, AVMEDIA_TYPE_SUBTITLE:
        break the_end
      else:
        nil
  if p and stream_index != -1:
    stream_index = p.stream_index[stream_index]
  echo(nil, AV_LOG_INFO, "Switch %s stream from #%d to #%d\n",
         av_get_media_type_string(codec_type), old_index, stream_index)
  stream_component_close(vs, old_index)
  stream_component_open(vs, stream_index)

proc toggle_full_screen*(vs: ptr VideoState) =
  is_full_screen = not is_full_screen
  SDL_SetWindowFullscreen(window, if is_full_screen: SDL_WINDOW_FULLSCREEN_DESKTOP else: 0)

proc toggle_audio_display*(vs: ptr VideoState) =
  var next: cint = vs.show_mode
  while true:
    next = (next + 1) mod SHOW_MODE_NB
    if not (next != vs.show_mode and
        (next == SHOW_MODE_VIDEO and not vs.video_st or
        next != SHOW_MODE_VIDEO and not vs.audio_st)):
      break
  if vs.show_mode != next:
    vs.force_refresh = 1
    vs.show_mode = next

proc refresh_loop_wait_event*(vs: ptr VideoState; event: ptr SDL_Event) =
  var remaining_time: cdouble = 0.0
  SDL_PumpEvents()
  while not SDL_PeepEvents(event, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT):
    if not cursor_hidden and
        av_gettime_relative() - cursor_last_shown > CURSOR_HIDE_DELAY:
      SDL_ShowCursor(0)
      cursor_hidden = 1
    if remaining_time > 0.0:
      av_usleep((int64)(remaining_time * 1000000.0))
    remaining_time = REFRESH_RATE
    if vs.show_mode != SHOW_MODE_NONE and
        (not vs.paused or vs.force_refresh):
      video_refresh(vs, addr(remaining_time))
    SDL_PumpEvents()

proc seek_chapter*(vs: ptr VideoState; incr: cint) =
  var pos: int64 = get_master_clock(vs) * AV_TIME_BASE
  var i: cint
  if not vs.ic.nb_chapters:
    return
  i = 0
  while i < vs.ic.nb_chapters:
    var ch: ptr AVChapter = vs.ic.chapters[i]
    if av_compare_ts(pos, AV_TIME_BASE_Q, ch.start, ch.time_base) < 0:
      dec(i)
      break
    inc(i)
  inc(i, incr)
  i = max(i, 0)
  if i >= vs.ic.nb_chapters:
    return
  echo(nil, AV_LOG_VERBOSE, "Seeking to chapter %d.\n", i)
  stream_seek(vs, av_rescale_q(vs.ic.chapters[i].start,
                                vs.ic.chapters[i].time_base, AV_TIME_BASE_Q), 0,
              0)

##  handle an event sent by the GUI

proc event_loop*(cur_stream: ptr VideoState) =
  var event: SDL_Event
  var
    incr: cdouble
    pos: cdouble
    frac: cdouble
  while true:
    var x: cdouble
    refresh_loop_wait_event(cur_stream, addr(event))
    case event.`type`
    of SDL_KEYDOWN:
      if exit_on_keydown or event.key.keysym.sym == SDLK_ESCAPE or
          event.key.keysym.sym == SDLK_q:
        do_exit(cur_stream)
        break
      if not cur_stream.width:
        continue
      case event.key.keysym.sym
      of SDLK_f:
        toggle_full_screen(cur_stream)
        cur_stream.force_refresh = 1
      of SDLK_p, SDLK_SPACE:
        toggle_pause(cur_stream)
      of SDLK_m:
        toggle_mute(cur_stream)
      of SDLK_KP_MULTIPLY, SDLK_0:
        update_volume(cur_stream, 1, SDL_VOLUME_STEP)
      of SDLK_KP_DIVIDE, SDLK_9:
        update_volume(cur_stream, -1, SDL_VOLUME_STEP)
      of SDLK_s:               ##  S: Step to next frame
        step_to_next_frame(cur_stream)
      of SDLK_a:
        stream_cycle_channel(cur_stream, AVMEDIA_TYPE_AUDIO)
      of SDLK_v:
        stream_cycle_channel(cur_stream, AVMEDIA_TYPE_VIDEO)
      of SDLK_c:
        stream_cycle_channel(cur_stream, AVMEDIA_TYPE_VIDEO)
        stream_cycle_channel(cur_stream, AVMEDIA_TYPE_AUDIO)
        stream_cycle_channel(cur_stream, AVMEDIA_TYPE_SUBTITLE)
      of SDLK_t:
        stream_cycle_channel(cur_stream, AVMEDIA_TYPE_SUBTITLE)
      of SDLK_w:
        if cur_stream.show_mode == SHOW_MODE_VIDEO and
            cur_stream.vfilter_idx < nb_vfilters - 1:
          if inc(cur_stream.vfilter_idx) >= nb_vfilters:
            cur_stream.vfilter_idx = 0
        else:
          cur_stream.vfilter_idx = 0
          toggle_audio_display(cur_stream)
      of SDLK_PAGEUP:
        if cur_stream.ic.nb_chapters <= 1:
          incr = 600.0
          break do_seek
        seek_chapter(cur_stream, 1)
      of SDLK_PAGEDOWN:
        if cur_stream.ic.nb_chapters <= 1:
          incr = -600.0
          break do_seek
        seek_chapter(cur_stream, -1)
      of SDLK_LEFT:
        incr = if seek_interval: -seek_interval else: -10.0
        break do_seek
      of SDLK_RIGHT:
        incr = if seek_interval: seek_interval else: 10.0
        break do_seek
      of SDLK_UP:
        incr = 60.0
        break do_seek
      of SDLK_DOWN:
        incr = -60.0
        if seek_by_bytes:
          pos = -1
          if pos < 0 and cur_stream.video_stream >= 0:
            pos = frame_queue_last_pos(addr(cur_stream.pictq))
          if pos < 0 and cur_stream.audio_stream >= 0:
            pos = frame_queue_last_pos(addr(cur_stream.sampq))
          if pos < 0:
            pos = avio_tell(cur_stream.ic.pb)
          if cur_stream.ic.bit_rate:
            incr = incr * (cur_stream.ic.bit_rate div 8.0)
          else:
            incr = incr * 180000.0
          inc(pos, incr)
          stream_seek(cur_stream, pos, incr, 1)
        else:
          pos = get_master_clock(cur_stream)
          if isnan(pos):
            pos = cast[cdouble](cur_stream.seek_pos div AV_TIME_BASE)
          inc(pos, incr)
          if cur_stream.ic.start_time != AV_NOPTS_VALUE and
              pos < cur_stream.ic.start_time div cast[cdouble](AV_TIME_BASE):
            pos = cur_stream.ic.start_time div cast[cdouble](AV_TIME_BASE)
          stream_seek(cur_stream, (int64)(pos * AV_TIME_BASE),
                      (int64)(incr * AV_TIME_BASE), 0)
      else:
        nil
    of SDL_MOUSEBUTTONDOWN:
      if exit_on_mousedown:
        do_exit(cur_stream)
        break
      if event.button.button == SDL_BUTTON_LEFT:
        var last_mouse_left_click: int64 = 0
        if av_gettime_relative() - last_mouse_left_click <= 500000:
          toggle_full_screen(cur_stream)
          cur_stream.force_refresh = 1
          last_mouse_left_click = 0
        else:
          last_mouse_left_click = av_gettime_relative()
    of SDL_MOUSEMOTION:
      if cursor_hidden:
        SDL_ShowCursor(1)
        cursor_hidden = 0
      cursor_last_shown = av_gettime_relative()
      if event.`type` == SDL_MOUSEBUTTONDOWN:
        if event.button.button != SDL_BUTTON_RIGHT:
          break
        x = event.button.x
      else:
        if not (event.motion.state and SDL_BUTTON_RMASK):
          break
        x = event.motion.x
      if seek_by_bytes or cur_stream.ic.duration <= 0:
        var size: uint64 = avio_size(cur_stream.ic.pb)
        stream_seek(cur_stream, size * x div cur_stream.width, 0, 1)
      else:
        var ts: int64
        var
          ns: cint
          hh: cint
          mm: cint
          ss: cint
        var
          tns: cint
          thh: cint
          tmm: cint
          tss: cint
        tns = cur_stream.ic.duration div 1000000
        thh = tns div 3600
        tmm = (tns mod 3600) div 60
        tss = (tns mod 60)
        frac = x div cur_stream.width
        ns = frac * tns
        hh = ns div 3600
        mm = (ns mod 3600) div 60
        ss = (ns mod 60)
        echo(nil, AV_LOG_INFO, "Seek to %2.0f%% (%2d:%02d:%02d) of total duration (%2d:%02d:%02d)       \n",
               frac * 100, hh, mm, ss, thh, tmm, tss)
        ts = frac * cur_stream.ic.duration
        if cur_stream.ic.start_time != AV_NOPTS_VALUE:
          inc(ts, cur_stream.ic.start_time)
        stream_seek(cur_stream, ts, 0, 0)
    of SDL_WINDOWEVENT:
      case event.window.event
      of SDL_WINDOWEVENT_SIZE_CHANGED:
        screen_width = cur_stream.width = event.window.data1
        screen_height = cur_stream.height = event.window.data2
        if cur_stream.vis_texture:
          SDL_DestroyTexture(cur_stream.vis_texture)
          cur_stream.vis_texture = nil
      of SDL_WINDOWEVENT_EXPOSED:
        cur_stream.force_refresh = 1
    of SDL_QUIT, FF_QUIT_EVENT:
      do_exit(cur_stream)
    else:
      nil

proc opt_frame_size*(optctx: pointer; opt: cstring; arg: cstring): cint =
  echo(nil, AV_LOG_WARNING, "Option -s is deprecated, use -video_size.\n")
  return opt_default(nil, "video_size", arg)

proc opt_width*(optctx: pointer; opt: cstring; arg: cstring): cint =
  screen_width = parse_number_or_die(opt, arg, OPT_INT64, 1, int.high)
  return 0

proc opt_height*(optctx: pointer; opt: cstring; arg: cstring): cint =
  screen_height = parse_number_or_die(opt, arg, OPT_INT64, 1, int.high)
  return 0

proc opt_format*(optctx: pointer; opt: cstring; arg: cstring): cint =
  file_iformat = av_find_input_format(arg)
  if not file_iformat:
    echo(nil, AV_LOG_FATAL, "Unknown input format: %s\n", arg)
    return AVERROR(EINVAL)
  return 0

proc opt_frame_pix_fmt*(optctx: pointer; opt: cstring; arg: cstring): cint =
  echo(nil, AV_LOG_WARNING,
         "Option -pix_fmt is deprecated, use -pixel_format.\n")
  return opt_default(nil, "pixel_format", arg)

proc opt_sync*(optctx: pointer; opt: cstring; arg: cstring): cint =
  if not strcmp(arg, "audio"):
    av_sync_type = AV_SYNC_AUDIO_MASTER
  elif not strcmp(arg, "video"):
    av_sync_type = AV_SYNC_VIDEO_MASTER
  elif not strcmp(arg, "ext"):
    av_sync_type = AV_SYNC_EXTERNAL_CLOCK
  else:
    echo(nil, AV_LOG_ERROR, "Unknown value for %s: %s\n", opt, arg)
    exit(1)
  return 0

proc opt_seek*(optctx: pointer; opt: cstring; arg: cstring): cint =
  start_time = parse_time_or_die(opt, arg, 1)
  return 0

proc opt_duration*(optctx: pointer; opt: cstring; arg: cstring): cint =
  duration = parse_time_or_die(opt, arg, 1)
  return 0

proc opt_show_mode*(optctx: pointer; opt: cstring; arg: cstring): cint =
  show_mode = if not strcmp(arg, "video"): SHOW_MODE_VIDEO else: if not strcmp(arg,
      "waves"): SHOW_MODE_WAVES else: if not strcmp(arg, "rdft"): SHOW_MODE_RDFT else: parse_number_or_die(
      opt, arg, OPT_INT, 0, SHOW_MODE_NB - 1)
  return 0

proc opt_input_file*(optctx: pointer; filename: cstring) =
  if input_filename:
    echo(nil, AV_LOG_FATAL, "Argument \'%s\' provided as input filename, but \'%s\' was already specified.\n",
           filename, input_filename)
    exit(1)
  if not strcmp(filename, "-"):
    filename = "pipe:"
  input_filename = filename

proc opt_codec*(optctx: pointer; opt: cstring; arg: cstring): cint =
  var spec: cstring = strchr(opt, ':')
  if not spec:
    echo(nil, AV_LOG_ERROR,
           "No media specifier was specified in \'%s\' in option \'%s\'\n", arg,
           opt)
    return AVERROR(EINVAL)
  inc(spec)
  case spec[0]
  of 'a':
    audio_codec_name = arg
  of 's':
    subtitle_codec_name = arg
  of 'v':
    video_codec_name = arg
  else:
    echo(nil, AV_LOG_ERROR, "Invalid media specifier \'%s\' in option \'%s\'\n",
           spec, opt)
    return AVERROR(EINVAL)
  return 0

var dummy*: cint

##  static const OptionDef options[] = {
##      CMDUTILS_COMMON_OPTIONS{"x", HAS_ARG, {.func_arg = opt_width}, "force displayed width", "width"},
##      {"y", HAS_ARG, {.func_arg = opt_height}, "force displayed height", "height"},
##      {"s", HAS_ARG | OPT_VIDEO, {.func_arg = opt_frame_size}, "set frame size (WxH or abbreviation)", "size"},
##      {"fs", OPT_BOOL, {&is_full_screen}, "force full screen"},
##      {"an", OPT_BOOL, {&audio_disable}, "disable audio"},
##      {"vn", OPT_BOOL, {&video_disable}, "disable video"},
##      {"sn", OPT_BOOL, {&subtitle_disable}, "disable subtitling"},
##      {"ast", OPT_STRING | HAS_ARG | OPT_EXPERT, {&wanted_stream_spec[AVMEDIA_TYPE_AUDIO]}, "select desired audio stream", "stream_specifier"},
##      {"vst", OPT_STRING | HAS_ARG | OPT_EXPERT, {&wanted_stream_spec[AVMEDIA_TYPE_VIDEO]}, "select desired video stream", "stream_specifier"},
##      {"sst", OPT_STRING | HAS_ARG | OPT_EXPERT, {&wanted_stream_spec[AVMEDIA_TYPE_SUBTITLE]}, "select desired subtitle stream", "stream_specifier"},
##      {"ss", HAS_ARG, {.func_arg = opt_seek}, "seek to a given position in seconds", "pos"},
##      {"t", HAS_ARG, {.func_arg = opt_duration}, "play  \"duration\" seconds of audio/video", "duration"},
##      {"bytes", OPT_INT | HAS_ARG, {&seek_by_bytes}, "seek by bytes 0=off 1=on -1=auto", "val"},
##      {"seek_interval", OPT_FLOAT | HAS_ARG, {&seek_interval}, "set seek interval for left/right keys, in seconds", "seconds"},
##      {"nodisp", OPT_BOOL, {&display_disable}, "disable graphical display"},
##      {"noborder", OPT_BOOL, {&borderless}, "borderless window"},
##      {"alwaysontop", OPT_BOOL, {&alwaysontop}, "window always on top"},
##      {"volume", OPT_INT | HAS_ARG, {&startup_volume}, "set startup volume 0=min 100=max", "volume"},
##      {"f", HAS_ARG, {.func_arg = opt_format}, "force format", "fmt"},
##      {"pix_fmt", HAS_ARG | OPT_EXPERT | OPT_VIDEO, {.func_arg = opt_frame_pix_fmt}, "set pixel format", "format"},
##      {"stats", OPT_BOOL | OPT_EXPERT, {&show_status}, "show status", ""},
##      {"fast", OPT_BOOL | OPT_EXPERT, {&fast}, "non spec compliant optimizations", ""},
##      {"genpts", OPT_BOOL | OPT_EXPERT, {&genpts}, "generate pts", ""},
##      {"drp", OPT_INT | HAS_ARG | OPT_EXPERT, {&decoder_reorder_pts}, "let decoder reorder pts 0=off 1=on -1=auto", ""},
##      {"lowres", OPT_INT | HAS_ARG | OPT_EXPERT, {&lowres}, "", ""},
##      {"sync", HAS_ARG | OPT_EXPERT, {.func_arg = opt_sync}, "set audio-video sync. type (type=audio/video/ext)", "type"},
##      {"autoexit", OPT_BOOL | OPT_EXPERT, {&autoexit}, "exit at the end", ""},
##      {"exitonkeydown", OPT_BOOL | OPT_EXPERT, {&exit_on_keydown}, "exit on key down", ""},
##      {"exitonmousedown", OPT_BOOL | OPT_EXPERT, {&exit_on_mousedown}, "exit on mouse down", ""},
##      {"loop", OPT_INT | HAS_ARG | OPT_EXPERT, {&loop}, "set number of times the playback shall be looped", "loop count"},
##      {"framedrop", OPT_BOOL | OPT_EXPERT, {&framedrop}, "drop frames when cpu is too slow", ""},
##      {"infbuf", OPT_BOOL | OPT_EXPERT, {&infinite_buffer}, "don't limit the input buffer size (useful with realtime streams)", ""},
##      {"window_title", OPT_STRING | HAS_ARG, {&window_title}, "set window title", "window title"},
##      {"left", OPT_INT | HAS_ARG | OPT_EXPERT, {&screen_left}, "set the x position for the left of the window", "x pos"},
##      {"top", OPT_INT | HAS_ARG | OPT_EXPERT, {&screen_top}, "set the y position for the top of the window", "y pos"},
##      {"vf", OPT_EXPERT | HAS_ARG, {.func_arg = opt_add_vfilter}, "set video filters", "filter_graph"},
##      {"af", OPT_STRING | HAS_ARG, {&afilters}, "set audio filters", "filter_graph"},
##      {"rdftspeed", OPT_INT | HAS_ARG | OPT_AUDIO | OPT_EXPERT, {&rdftspeed}, "rdft speed", "msecs"},
##      {"showmode", HAS_ARG, {.func_arg = opt_show_mode}, "select show mode (0 = video, 1 = waves, 2 = RDFT)", "mode"},
##      {"default", HAS_ARG | OPT_AUDIO | OPT_VIDEO | OPT_EXPERT, {.func_arg = opt_default}, "generic catch all option", ""},
##      {"i", OPT_BOOL, {&dummy}, "read specified file", "input_file"},
##      {"codec", HAS_ARG, {.func_arg = opt_codec}, "force decoder", "decoder_name"},
##      {"acodec", HAS_ARG | OPT_STRING | OPT_EXPERT, {&audio_codec_name}, "force audio decoder", "decoder_name"},
##      {"scodec", HAS_ARG | OPT_STRING | OPT_EXPERT, {&subtitle_codec_name}, "force subtitle decoder", "decoder_name"},
##      {"vcodec", HAS_ARG | OPT_STRING | OPT_EXPERT, {&video_codec_name}, "force video decoder", "decoder_name"},
##      {"autorotate", OPT_BOOL, {&autorotate}, "automatically rotate video", ""},
##      {"find_stream_info", OPT_BOOL | OPT_INPUT | OPT_EXPERT, {&find_stream_info}, "read and decode the streams to fill missing information with heuristics"},
##      {"filter_threads", HAS_ARG | OPT_INT | OPT_EXPERT, {&filter_nbthreads}, "number of filter threads per graph"},
##      {
##          NULL,
##      },
##  };

proc show_usage*() =
  echo(nil, AV_LOG_INFO, "Simple media player\n")
  echo(nil, AV_LOG_INFO, "usage: %s [options] input_file\n", program_name)
  echo(nil, AV_LOG_INFO, "\n")

proc show_help_default*(opt: cstring; arg: cstring) =
  av_log_set_callback(log_callback_help)
  show_usage()
  show_help_options(options, "Main options:", 0, OPT_EXPERT, 0)
  show_help_options(options, "Advanced options:", OPT_EXPERT, 0, 0)
  printf("\n")
  show_help_children(avcodec_get_class(), AV_OPT_FLAG_DECODING_PARAM)
  show_help_children(avformat_get_class(), AV_OPT_FLAG_DECODING_PARAM)
  show_help_children(avfilter_get_class(), AV_OPT_FLAG_FILTERING_PARAM)
  printf("\nWhile playing:\nq, ESC              quit\nf                   toggle full screen\np, SPC              pause\nm                   toggle mute\n9, 0                decrease and increase volume respectively\n/, *                decrease and increase volume respectively\na                   cycle audio channel in the current program\nv                   cycle video channel\nt                   cycle subtitle channel in the current program\nc                   cycle program\nw                   cycle video filters or show modes\ns                   activate frame-step mode\nleft/right          seek backward/forward 10 seconds or to custom interval if -seek_interval is set\ndown/up             seek backward/forward 1 minute\npage down/page up   seek backward/forward 10 minutes\nright mouse click   seek to percentage in file corresponding to fraction of width\nleft double-click   toggle full screen\n")

##  Called from the main

proc main*(argc: cint; argv: cstringArray): cint =
  var flags: cint
  var vs: ptr VideoState
  init_dynload()
  av_log_set_flags(AV_LOG_SKIP_REPEATED)
  parse_loglevel(argc, argv, options)
  ##  register all codecs, demux and protocols
  avdevice_register_all()
  avformat_network_init()
  init_opts()
  signal(SIGINT, sigterm_handler)
  ##  Interrupt (ANSI).
  signal(SIGTERM, sigterm_handler)
  ##  Termination (ANSI).
  show_banner(argc, argv, options)
  parse_options(nil, argc, argv, options, opt_input_file)
  if not input_filename:
    show_usage()
    echo(nil, AV_LOG_FATAL, "An input file must be specified\n")
    echo(nil, AV_LOG_FATAL,
           "Use -h to get full help or, even better, run \'man %s\'\n",
           program_name)
    exit(1)
  if display_disable:
    video_disable = 1
  flags = SDL_INIT_VIDEO or SDL_INIT_AUDIO or SDL_INIT_TIMER
  if audio_disable:
    flags = flags and not SDL_INIT_AUDIO
  else:
    ##  Try to work around an occasional ALSA buffer underflow issue when the
    ##  period size is NPOT due to ALSA resampling by forcing the buffer size.
    if not SDL_getenv("SDL_AUDIO_ALSA_SET_BUFFER_SIZE"):
      SDL_setenv("SDL_AUDIO_ALSA_SET_BUFFER_SIZE", "1", 1)
  if display_disable:
    flags = flags and not SDL_INIT_VIDEO
  if SDL_Init(flags):
    echo(nil, AV_LOG_FATAL, "Could not initialize SDL - %s\n", SDL_GetError())
    echo(nil, AV_LOG_FATAL, "(Did you set the DISPLAY variable?)\n")
    exit(1)
  SDL_EventState(SDL_SYSWMEVENT, SDL_IGNORE)
  SDL_EventState(SDL_USEREVENT, SDL_IGNORE)
  av_init_packet(addr(flush_pkt))
  flush_pkt.data = cast[ptr uint8](addr(flush_pkt))
  if not display_disable:
    var flags: cint = SDL_WINDOW_HIDDEN
    if alwaysontop:
      flags = flags or SDL_WINDOW_ALWAYS_ON_TOP
    if borderless:
      flags = flags or SDL_WINDOW_BORDERLESS
    else:
      flags = flags or SDL_WINDOW_RESIZABLE
    window = SDL_CreateWindow(program_name, SDL_WINDOWPOS_UNDEFINED,
                            SDL_WINDOWPOS_UNDEFINED, default_width,
                            default_height, flags)
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear")
    if window:
      renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED or
          SDL_RENDERER_PRESENTVSYNC)
      if not renderer:
        echo(nil, AV_LOG_WARNING,
               "Failed to initialize a hardware accelerated renderer: %s\n",
               SDL_GetError())
        renderer = SDL_CreateRenderer(window, -1, 0)
      if renderer:
        if not SDL_GetRendererInfo(renderer, addr(renderer_info)):
          echo(nil, AV_LOG_VERBOSE, "Initialized %s renderer.\n",
                 renderer_info.name)
    if not window or not renderer or not renderer_info.num_texture_formats:
      echo(nil, AV_LOG_FATAL, "Failed to create window or renderer: %s",
             SDL_GetError())
      do_exit(nil)
  vs = stream_open(input_filename, file_iformat)
  if not vs:
    echo(nil, AV_LOG_FATAL, "Failed to initialize VideoState!\n")
    do_exit(nil)
  event_loop(vs)
  return 0
