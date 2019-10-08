REM Test the vectorwar sample by starting 2 clients connected
REM back to each other.
REM
REM Controls: Arrows to move
REM           Press 'D' to fire
REM           Press 'P' to show performance monitor
REM           Shift to strafe

pushd ..\build\VS2019\x64\Debug
del *.log
start VectorWar.exe 7000 2 local 127.0.0.1:7001 
start VectorWar.exe 7001 2 127.0.0.1:7000 local
popd