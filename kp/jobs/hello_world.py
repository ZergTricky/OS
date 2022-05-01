import sys

print("Hello world")

job_name = str(sys.argv[1])

fileName = "./jobs_results/" + job_name + ".txt"

OUT = open(fileName, "w")

OUT.write("10")

OUT.close()