#!/bin/sh

em_cflags="-Wall -Isrc/deps/box2d"
em_lflags="-lpthread -sASYNCIFY -sNO_DISABLE_EXCEPTION_CATCHING -sALLOW_MEMORY_GROWTH -sUSE_WEBGL2"

cflags="-std=c++17 -Wall -DDEBUG -Isrc/deps/box2d"
lflags="-lX11 -lXi -lXcursor -lasound -lGL -ldl -lpthread -lm"

srcs="src/spry.cpp src/deps.cpp"

if [ "$1" = "web_release" ]; then
  emcc $em_cflags -O2 -DRELEASE $srcs -o spry.js $em_lflags
elif [ "$1" = "web" ]; then
  emcc $em_cflags -DDEBUG $srcs -o spry.js $em_lflags
elif [ "$1" = "release" ]; then
  clang++ $cflags -O2 -DRELEASE $srcs -o spry $lflags
else
  clang++ $cflags -g -DDEBUG $srcs -o spry $lflags
fi