#!/bin/bash

set -e

mkdir -p build
gcc -std=gnu11 -Wall -ggdb -O3 -mtune=native -march=native -o build/ecosystem main.c
# ./build/ecosystem input.txt
