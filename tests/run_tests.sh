#!/bin/bash

set -e

gcc -std=gnu11 -Wall -ggdb -O3 -mtune=native -march=native -o ecosystem ../src/main.c

./ecosystem input5x5 --no-output --test output5x5
./ecosystem input10x10 --no-output --test output10x10
./ecosystem input20x20 --no-output --test output20x20
./ecosystem input100x100 --no-output --test output100x100
./ecosystem input100x100_unbal01 --no-output --test output100x100_unbal01
./ecosystem input100x100_unbal02 --no-output --test output100x100_unbal02
./ecosystem input200x200 --no-output --test output200x200

