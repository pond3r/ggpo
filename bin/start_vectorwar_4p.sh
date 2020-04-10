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

./VectorWar 7000 4 local 127.0.0.1:7001 127.0.0.1:7002 127.0.0.1:7003 &
./VectorWar 7001 4 127.0.0.1:7000 local 127.0.0.1:7002 127.0.0.1:7003 &
./VectorWar 7002 4 127.0.0.1:7000 127.0.0.1:7001 local 127.0.0.1:7003 &
./VectorWar 7003 4 127.0.0.1:7000 127.0.0.1:7001 127.0.0.1:7002 local &

cd -
