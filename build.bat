@echo off

if not exist .\build mkdir .\build
pushd .\build

set CommonCompilerFlags=      ^
    /O2                       ^
    /Oi                       ^
    /Ot                       ^
    /GL                       ^
    /Qpar                     ^
    /fp:fast                  ^
    /fp:except-               ^
    /Ob3                      ^
    /Oy-                      ^
    /GS-                      ^
    /guard:cf-                ^
    /Qvec-report:2            ^
    /arch:AVX512              ^
    /nologo                   ^
    /FC                       ^
    /WX                       ^
    /W4                       ^
    /wd4201                   ^
    /wd4100                   ^
    /wd4505                   ^
    /wd4189                   ^
    /wd4457                   ^
    /wd4456                   ^
    /wd4819                   ^
    /wd4715                   ^
    /MT                       ^
    /GR-                      ^
    /Gm-                      ^
    /EHa-                     ^
    /Zi                       ^
    /D_CRT_SECURE_NO_WARNINGS ^
    /DNDEBUG

set CommonLinkerFlags=^
    /LTCG             ^
    /OPT:REF          ^
    /OPT:ICF          ^
    /INCREMENTAL:NO   ^
    user32.lib        ^
    gdi32.lib         ^
    winmm.lib         ^
    /time

del *.pdb > NUL 2> NUL

cl.exe %CommonCompilerFlags% /Fe:sch.exe ..\main.cpp ..\getopt.cpp -link %CommonLinkerFlags%

set LastError=%ERRORLEVEL%
popd

if not %LastError% == 0 goto :end

.\build\sch.exe "zjl" -r

:end
