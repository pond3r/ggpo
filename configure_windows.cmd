@echo off

cmake -G "Visual Studio 16 2019" -A x64 -B build -DBUILD_SHARED_LIBS=off

IF "%1"=="--no-prompt" goto :done
:: pause so the user can see the output if they double clicked the configure script
pause 

:done