@echo off

set cflags=/std:c++17 /nologo /Zi /EHsc /Isrc/deps/box2d /DDEBUG

if not exist deps.obj (cl %cflags% /c src/deps.cpp)
cl %cflags% src/spry.cpp deps.obj
