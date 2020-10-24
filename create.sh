#bin/sh
cmake ecs175-demo.v3 &
wait $!
cmake --build . &
wait $!
./run_p1 ecs175-demo.v3/projects/p1_skeleton/input1.txt

