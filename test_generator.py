#!/usr/bin/python
import sys
import random
len1 = sys.argv[1]
len2 = sys.argv[2]
alphabet = ''.join(set(sys.argv[3]))
first = [random.choice(alphabet) for _ in range(int(len1))]
second = [random.choice(alphabet) for _ in range(int(len2))]
with open("input.txt", 'w') as f:
    f.write(f"{str(''.join(first))} {str(''.join(second))}")
