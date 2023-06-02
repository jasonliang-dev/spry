@echo off

cl /std:c++17 /nologo /Zi /EHsc /DNOMINMAX /Isrc/deps/box2d src/spry.cpp
