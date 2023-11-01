#!/bin/sh

em_cflags="-Wall -Isrc/deps/box2d"
em_lflags="-lpthread -sASYNCIFY -sNO_DISABLE_EXCEPTION_CATCHING -sALLOW_MEMORY_GROWTH"

cflags="-std=c++17 -Wall -Isrc/deps/box2d"
lflags="-lX11 -lXi -lXcursor -lasound -lGL -ldl -lpthread -lm"

if [ "$1" = "web_release" ]; then
  emcc $em_cflags -O2 -DRELEASE src/spry.cpp -o spry.js $em_lflags
elif [ "$1" = "web" ]; then
  emcc $em_cflags -DDEBUG src/spry.cpp -o spry.js $em_lflags
elif [ "$1" = "release" ]; then
  clang++ $cflags -O2 -DRELEASE src/spry.cpp -o spry $lflags
else
  clang++ $cflags -g -DDEBUG src/spry.cpp -o spry $lflags
fi