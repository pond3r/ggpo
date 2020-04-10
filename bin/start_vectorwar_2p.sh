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

./VectorWar 7000 2 local 127.0.0.1:7001 &
./VectorWar 7001 2 127.0.0.1:7000 local &

cd -
