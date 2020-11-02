import nimterop/[build, cimport]


cPlugin:
    import strutils
    proc onSymbol*(sym: var Symbol) {.exportc, dynlib.} =
        sym.name = sym.name.strip(chars={'_'})
        if sym.name == "SDL_ThreadID": sym.name = "SDL_ThreadID1" 


# when defined(windows):
#     cIncludeDir"C:/SDL2-2.0.12/x86_64-w64-mingw32/include/SDL2"
#     cIncludeDir "c:/FFmpeg"
#     cIncludeDir "c:/FFmpeg/fftools"
# else:
#     cIncludeDir"/mnt/c/SDL2-2.0.12/x86_64-w64-mingw32/include/SDL2"
#     cIncludeDir"/mnt/c/FFmpeg"
#     cIncludeDir"/mnt/c/FFmpeg/fftools"


# cImport("c:/FFmpeg/fftools/ffplay.c", recurse=true,flags="-c",nimFile="c:/ffplay/ffplaytest.nim")
cImport("c:/ffplay/ffplay.c", recurse=true,flags="-c",nimFile="c:/ffplay/ffplay.nim")
# c2nImport("c:/FFmpeg/fftools/ffplay.c", recurse=true,flags="--skipcomments --concat",nimFile="c:/ffplay/ffplaytest.nim")
