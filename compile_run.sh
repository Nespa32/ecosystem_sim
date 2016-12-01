#!/bin/bash

set -e

mkdir -p build
gcc -std=gnu11 -Wall -ggdb -O3 -mtune=native -march=native -o build/ecosystem src/main.c

time ./build/ecosystem tests/input200x200 --no-output
