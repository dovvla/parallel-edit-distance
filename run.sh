#!/bin/sh
./test_generator.py $1 $2 abc
gcc -fopenmp edit_distance.c -o ed
./ed $1 $2 $3