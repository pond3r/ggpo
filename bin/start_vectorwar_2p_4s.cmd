@echo off

REM Test the vectorwar sample by starting 2 clients connected
REM back to each other.
REM
REM Controls: Arrows to move
REM           Press 'D' to fire
REM           Press 'P' to show performance monitor
REM           Shift to strafe

pushd ..\build\bin\x64\Release
del *.log

:: no spectators
::start VectorWar.exe 7000 2 local 127.0.0.1:7001
::start VectorWar.exe 7001 2 127.0.0.1:7000 local

:: 1 spectator
::start VectorWar.exe 7000 2 local 127.0.0.1:7001 127.0.0.1:7005
::start VectorWar.exe 7001 2 127.0.0.1:7000 local
::start VectorWar.exe 7005 2 spectate 127.0.0.1:7000

:: 4 spectators
start VectorWar.exe 7000 2 local 127.0.0.1:7001 127.0.0.1:7005 127.0.0.1:7006 127.0.0.1:7007 127.0.0.1:7008 
start VectorWar.exe 7001 2 127.0.0.1:7000 local
start VectorWar.exe 7005 2 spectate 127.0.0.1:7000
start VectorWar.exe 7006 2 spectate 127.0.0.1:7000
start VectorWar.exe 7007 2 spectate 127.0.0.1:7000
start VectorWar.exe 7008 2 spectate 127.0.0.1:7000

popd