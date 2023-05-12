import os

fin = open('status_info.txt')
record = False
list = []
current = ""
for line in fin:
    if "crab_" in line:
        current = line.strip()

    if "failed" in line:
        list.append(current)

for entry in list:
    print("crab resubmit -d "+entry.replace(":",""))
