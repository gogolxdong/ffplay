# ffplay

An attempt to build singleton ffplay in pure Nim.

 /c/msys64/mingw64/bin/gcc ffplay.c /c/FFmpeg/fftools/cmdutils.o /c/FFmpeg/libavutil/opt.o -Ic:/FFmpeg -IC:/msys64/mingw64/include/SDL2 -LC:/msys64/mingw64/lib  -lmingw32 -lSDL2main -lSDL2 -lshell32 -lavutil -lavcodec -lavfilter -lswscale -lavformat -lswresample -lavdevice -lpostproc -o ffplay.exe

* first : make ffplay compile dependent on SDL2 only

 /c/msys64/mingw64/bin/gcc ffplay.c  -Ic:/FFmpeg -IC:/msys64/mingw64/include/SDL2 -LC:/msys64/mingw64/lib  -o ffplay.exe -Wfatal-errors