# WebSocket Echo

## Introduction
This is a simple WebSocket based, client-server echo project.

## Building WebSocket-Echo

### Supported Platforms

* Microsoft Windows
* ~~Linux~~(TBD)

### Build

Project is set to use C++23.

#### Windows

Setup and build with:
* CMake
* MSVC version 19.34(Visual Studio 2022 v17.4 or later)

Using CLI:
```
    $ mkdir build && cd build
    $ cmake .. -G "Visual Studio 17 2022"
    $ msbuild WebSocket-Echo.sln -m
```