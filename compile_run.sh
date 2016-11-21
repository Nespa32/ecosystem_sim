#!/bin/bash

set -e

gcc -std=gnu11 -Wall -ggdb -o ecosystem main.c
./ecosystem input.txt
