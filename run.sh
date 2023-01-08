#!/bin/sh
gcc -fopenmp -std=c99 edit_distance.c -o ed
./ed 20 20