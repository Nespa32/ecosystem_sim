#!/bin/bash

set -e

mkdir -p build
gcc -std=gnu11 -Wall -ggdb -o build/ecosystem main.c
# ./build/ecosystem input.txt
