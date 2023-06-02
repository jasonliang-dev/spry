#!/bin/sh

if [ "$1" = "web" ]; then
  emcc -Wall -O2 src/spry.cpp -o spry.js -lpthread --preload-file data
else
  clang++ -std=c++14 -Wall -g src/spry.cpp -o spry -lX11 -lXi -lXcursor -lasound -lGL -ldl -lpthread -lm
fi