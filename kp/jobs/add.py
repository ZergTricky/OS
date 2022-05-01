import sys

job_name = str(sys.argv[1])

fileName = "./jobs_results/" + job_name + ".txt"

OUT = open(fileName, "w")

args = [0] * (len(sys.argv) - 2)

for i in range(len(sys.argv) - 2):
    out = open(sys.argv[i + 2], "r")
    args[i] = int(out.readline())
    out.close()

OUT.write(str(args[0] + args[1]))

OUT.close()