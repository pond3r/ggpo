#! /bin/env sh

# Test the vectorwar sample by starting 2 clients connected
# back to each other.
#
# Controls: Arrows to move
#           Press 'D' to fire
#           Press 'P' to show performance monitor
#           Shift to strafe

cd ../src/apps/vectorwar/
rm *.log

# no spectators
#start VectorWar.exe 7000 2 local 127.0.0.1:7001
#start VectorWar.exe 7001 2 127.0.0.1:7000 local

# 1 spectator
#start VectorWar.exe 7000 2 local 127.0.0.1:7001 127.0.0.1:7005
#start VectorWar.exe 7001 2 127.0.0.1:7000 local
#start VectorWar.exe 7005 2 spectate 127.0.0.1:7000

# 4 spectators
./VectorWar 7000 2 local 127.0.0.1:7001 127.0.0.1:7005 127.0.0.1:7006 127.0.0.1:7007 127.0.0.1:7008 &
./VectorWar 7001 2 127.0.0.1:7000 local &
./VectorWar 7005 2 spectate 127.0.0.1:7000 &
./VectorWar 7006 2 spectate 127.0.0.1:7000 &
./VectorWar 7007 2 spectate 127.0.0.1:7000 &
./VectorWar 7008 2 spectate 127.0.0.1:7000 &

cd -
