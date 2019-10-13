@echo off
cmake -G "Visual Studio 16 2019" -A x64 -B build -DBUILD_SHARED_LIBS=off
pause 