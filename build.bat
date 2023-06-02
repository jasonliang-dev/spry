@echo off

cl /std:c++17 /nologo /Zi /EHsc /DNOMINMAX /DDEBUG /Isrc/deps/box2d src/spry.cpp
