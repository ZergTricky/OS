import sys
import os
import random
import time

job_name = str(sys.argv[1])

fileName = "./jobs_results/" + job_name + ".txt"

OUT = open(fileName, "w")

val = random.randint(0,100)

OUT.write(str(val))

OUT.close()