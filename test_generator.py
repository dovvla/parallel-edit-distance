#!/usr/bin/python
import sys
import random
import subprocess
from datetime import datetime


test_dimension = [
    (100, 100),
    (1000, 1000),
    (10000, 10000),
    (100, 100000),
    (100000, 100),
    (80000, 800000),
    (800000, 800000),
]

if sys.argv[2] != "onlyrun":
    alphabet = "".join(set(sys.argv[1]))
    for (len1, len2) in test_dimension:
        first = [random.choice(alphabet) for _ in range(int(len1))]
        second = [random.choice(alphabet) for _ in range(int(len2))]
        with open(f"input{len1}X{len2}.txt", "w") as f:
            f.write(f"{str(''.join(first))} {str(''.join(second))}")


for (len1, len2) in test_dimension:
    for num_threads in [2, 4, 8, 16]:
        outputs = []
        subprocess.run(
            ["gcc", "-fopenmp", "edit_distance.c", "-o", "ed"]
        )  # gcc -fopenmp edit_distance.c -o ed
        print(f"running {len1} {len2} \n")
        outputs.append(
            subprocess.run(
                [
                    "./ed",
                    f"{len1}",
                    f"{len2}",
                    f"input{len1}X{len2}.txt",
                    f"{num_threads}",
                ],
                capture_output=True,
                text=True,
            ).stdout
        )  # ./ed $1 $2 $3
        print(str(outputs[-1]))
    with open("test_results.txt", "a") as f:
        now = datetime.now()
        f.write(f"-----------------------{now}-----------------------")
        f.write(f"-----------------------{num_threads}-----------------------")

        f.writelines(outputs)
        f.write(f"\\-----------------------{now}-----------------------\\")
