#!/bin/sh

if [ "$1" = "web" ]; then
  emcc -Wall -O2 src/spry.cpp -o spry.js -lpthread --preload-file data
elif [ "$1" = "release" ]; then
  clang++ -std=c++17 -Wall -O2 -DRELEASE -Isrc/deps/box2d src/spry.cpp -o spry -lX11 -lXi -lXcursor -lasound -lGL -ldl -lpthread -lm
else
  clang++ -std=c++17 -Wall -g -DDEBUG -Isrc/deps/box2d src/spry.cpp -o spry -lX11 -lXi -lXcursor -lasound -lGL -ldl -lpthread -lm
fi