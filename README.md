# ffplay

An attempt to build a ffplay in pure Nim with IPFS protocol.

* Compile command for now: 
  
  /c/msys64/mingw64/bin/gcc ffplay.c /c/FFmpeg/fftools/cmdutils.o /c/FFmpeg/libavutil/opt.o -Ic:/FFmpeg -IC:/msys64/mingw64/include/SDL2 -LC:/msys64/mingw64/lib  -lmingw32 -lSDL2main -lSDL2 -lshell32 -lavutil -lavcodec -lavfilter -lswscale -lavformat -lswresample -lavdevice -lpostproc -o ffplay.exe

* WIP: make ffplay compile dependent on SDL2 only

  /c/msys64/mingw64/bin/gcc ffplay.c  -Ic:/FFmpeg -IC:/msys64/mingw64/include/SDL2 -LC:/msys64/mingw64/lib -lSDL2 -o ffplay.exe -Wfatal-errors

 * Future:

   nim c ffplay.nim