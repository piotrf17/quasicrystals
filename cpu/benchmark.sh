#!/bin/bash

make clean && make
time ./quasicrystal --width=1920 --height=1920 --view_mode=false --num_waves=7 --freq=0.2 --benchmark_steps=50


# History:
# real	1m31.435s   -- initial version, fixed phase velocities