@echo off

cl /std:c++17 /nologo /fsanitize=address /Zi /EHsc src/tests.cpp src/test_deps.cpp
