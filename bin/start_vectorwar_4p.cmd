@echo off

REM Test the vectorwar sample by starting 2 clients connected
REM back to each other.
REM
REM Controls: Arrows to move
REM           Press 'D' to fire
REM           Press 'P' to show performance monitor
REM           Shift to strafe

del *.log
start vectorwar 7000 4 local 127.0.0.1:7001 127.0.0.1:7002 127.0.0.1:7003
start vectorwar 7001 4 127.0.0.1:7000 local 127.0.0.1:7002 127.0.0.1:7003
start vectorwar 7002 4 127.0.0.1:7000 127.0.0.1:7001 local 127.0.0.1:7003
start vectorwar 7003 4 127.0.0.1:7000 127.0.0.1:7001 127.0.0.1:7002 local

