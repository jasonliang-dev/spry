#!/bin/sh

if [ "$1" = "web_release" ]; then
  emcc -Wall -O2 -DRELEASE -Isrc/deps/box2d src/spry.cpp -o spry.js -lpthread -sASYNCIFY -sNO_DISABLE_EXCEPTION_CATCHING -sALLOW_MEMORY_GROWTH
elif [ "$1" = "web" ]; then
  emcc -Wall -DDEBUG -Isrc/deps/box2d src/spry.cpp -o spry.js -lpthread -sASYNCIFY -sNO_DISABLE_EXCEPTION_CATCHING -sALLOW_MEMORY_GROWTH
elif [ "$1" = "release" ]; then
  clang++ -std=c++17 -Wall -O2 -DRELEASE -Isrc/deps/box2d src/spry.cpp -o spry -lX11 -lXi -lXcursor -lasound -lGL -ldl -lpthread -lm
else
  clang++ -std=c++17 -Wall -g -DDEBUG -Isrc/deps/box2d src/spry.cpp -o spry -lX11 -lXi -lXcursor -lasound -lGL -ldl -lpthread -lm
fi