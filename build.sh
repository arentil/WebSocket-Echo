#!/bin/bash

mkdir -p build
cd build

cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=/home/marcin/Qt/6.9.1/gcc_64 -DCMAKE_COLOR_DIAGNOSTICS=On

cmake --build . -j
