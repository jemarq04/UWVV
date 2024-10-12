import subprocess
import pdb
import os
import sys
import argparse

#=======================================
DESC='''
Usage:
Move all project folders into one folder, copy this script into the folder, initialize proxy and run it.
It will run "crab status -d" for all the project folders, put printouts in "output_crab_status_data" folder, and parse the printouts to write a summary file
If "--report" is enabled, it will also run "crab report -d" on all folders and put printouts in "output_crab_status_data" folder
Currently it only checks if results/notFinishedLumis.json exists in the project folders, and gives a warning if it does. But this may not guarantee 
all lumis are processed, since different Data.splitting mode seems to generate the reports differently...
suggest to look at the reports by something like "cat *.log"
Then if some jobs fail in some datasets, it will create a resubmission script for all such datasets.
After Checking from the status summary file that no job is in transition or still running (and other aspects), the script can be run.   

The new out folder and file/script will be named with 0,1,2 each time this python script is run      
'''
#=======================================
parser = argparse.ArgumentParser(description=DESC, formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument("--report", action="store_true", help="if provided, also run 'crab report'")
parser.add_argument("--noprocess", dest="noprocessing", action="store_true", help="if provided, skip processing")
parser.add_argument("-d", "--dir", dest="outputdir", default="output_crab_status_data", help="output status folder")
parser.add_argument("-o", "--output", dest="outname", default="status_info.txt", help="output info txt file name")
args = parser.parse_args()

if not args.outname.endswith(".txt"):
    parser.error("invalid filetype for output file %s. must be .txt" % args.outname)

print("Running scripts. Don't forget to initialize proxy first.\nSee the latest folder/files with largest index.")

crablist = [x.path for x in os.scandir(".") if x.is_dir() and x.path.startswith("./crab_")]

if os.path.isdir(args.outputdir):
    status_idx = 0
    while os.path.isdir("%s%i" % (args.outputdir, status_idx)):
        status_idx += 1
    args.outputdir = "%s%i" % (args.outputdir, status_idx)
    args.outname = "%s%i.txt" % (".".join(args.outname.split(".")[:-1]), status_idx)
print("Creating new directory %s" % args.outputdir) 
os.mkdir(args.outputdir)

if not args.noprocessing:
    count = 0
    for folder in crablist:
        command = "crab status -d %s > %s.txt 2>&1" % (folder, os.path.join(args.outputdir, folder))
        code = subprocess.call([command], shell=True)
        if args.report:
            command2 = "crab report -d %s > %s_Report.log 2>&1" % (folder, os.path.join(args.outputdir, folder))
            code2 = subprocess.call([command2], shell=True)

        if code == 0:
            count +=1
            print("Processed %s folders" % count)
        else:
            print("Error in %s" % folder)
        
        if os.path.exists(os.path.join(folder,"results","notFinishedLumis.json")):
            print("==========WARNING: %s has not-yet-processed lumi=========="%folder)

with open(args.outname, "w") as fout:
    for fname in crablist:
        with open(os.path.join(args.outputdir,fname+'.txt'), "r") as status:
            linecount=0
            record = False
            for line in status:
                if 'Jobs status:' in line:
                    record = True
                    fout.write("%s:\n" % fname)
                if not record:
                    continue

                if linecount < 3:
                    if not "No publication information (publication has been disabled in the CRAB configuration file)" in line:
                        fout.write(line)
                    linecount += 1
                else:
                    fout.write("\n")
                    break
            else:
                fout.write('\nSomething wrong with %s\n\n'%fname)

print("Info output saved as %s" % args.outname)    

print("Writing resubmit script. Before running resubmission please check status txt to make sure no job is still running or in transition")
relist = []
current = ""
with open(args.outname) as fstat:
    for line in fstat:
        if "crab_" in line:
            current = line.strip()[:-1]
        if "failed" in line:
            relist.append(current)

with open("%s-resubmit.sh" % ".".join(args.outname.split(".")[-1])) as fre:
    for entry in relist:
        fre.write("crab resubmit -d %s\n" % entry)
