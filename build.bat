@echo off
setlocal

REM Utwórz folder "build", jeśli nie istnieje
if not exist build (
    mkdir build
)

REM Wejdź do folderu "build"
cd build

REM Uruchom CMake z katalogiem nadrzędnym (..)
cmake .. -G "Visual Studio 17 2022"

REM Znajdź pierwszy plik .sln w katalogu build i użyj go z msbuild
msbuild WebSocket-Echo.sln /p:Configuration=Debug

endlocal
pause