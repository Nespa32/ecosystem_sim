#!/bin/bash

set -e

gcc -std=gnu11 -Wall -ggdb -O3 -mtune=native -march=native \
    -o ../build/ecosystem ../src/main.c

./../build/ecosystem input5x5 --no-output --test output5x5
./../build/ecosystem input10x10 --no-output --test output10x10
./../build/ecosystem input20x20 --no-output --test output20x20
./../build/ecosystem input100x100 --no-output --test output100x100
./../build/ecosystem input100x100_unbal01 --no-output --test output100x100_unbal01
./../build/ecosystem input100x100_unbal02 --no-output --test output100x100_unbal02
./../build/ecosystem input200x200 --no-output --test output200x200

