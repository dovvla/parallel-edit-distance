#!/bin/sh
./test_generator.py $1 $2 abc
gcc -fopenmp -std=c99 edit_distance.c -o ed
./ed $1 $2