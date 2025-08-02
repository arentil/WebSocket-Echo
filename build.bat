@echo off
setlocal

REM Create "build" folder if not exist
if not exist build (
    mkdir build
)

REM Enter "build" directory
cd build

REM Cmake ..
cmake .. -G "Visual Studio 17 2022" -DCMAKE_BUILD_TYPE=Debug

REM Build solution(/v:diag for verbose)
msbuild WebSocket-Echo.sln -m

endlocal