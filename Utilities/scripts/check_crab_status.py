import subprocess
import pdb
import os
import sys
from optparse import OptionParser

#=======================================
#Usage:
#Move all project folders into one folder, copy this script into the folder, initialize proxy and run it: python check_crab_status.py [--report]
#It will run "crab status -d" for all the project folders, put printouts in "output_crab_status_data" folder, and parse the printouts to write a summary file
#If "--report" is enabled, it will also run "crab report -d" on all folders and put printouts in "output_crab_status_data" folder
#Currently it only checks if results/notFinishedLumis.json exists in the project folders, and gives a warning if it does. But this may not guarantee 
#all lumis are processed, since different Data.splitting mode seems to generate the reports differently...
#suggest to look at the reports by something like "cat *.log"
#Then if some jobs fail in some datasets, it will create a resubmission script for all such datasets.
#After Checking from the status summary file that no job is in transition or still running (and other aspects), the script can be run.   

#The new out folder and file/script will be named with 0,1,2 each time this python script is run      
#=======================================
processing=True
print("Running scripts. Don't forget to initialize proxy first.\nSee the latest folder/files with largest index.")
parser=OptionParser()
#parser.add_option("-f", dest="folder",help="Output Status folder")
#parser.add_option("-o", dest="output",help="Output info file name")
parser.add_option("--report", dest="report",help="Whether to run crab report",default=False,action="store_true")
(options,args)=parser.parse_args()

outputdir='output_crab_status_data'
outputdir0='output_crab_status_data'
#if options.folder:
#    outputdir=options.folder

listname="folder_list.txt"
#outname="status_info.txt"
#if options.output:
#    outname=options.output

subprocess.call("ls > %s"%listname,shell=True)

folderind = 0
if os.path.isdir(outputdir):
    
    while os.path.isdir(outputdir):
        print("folder %s already exists"%outputdir)
        folderind+=1
        outputdir = outputdir0 + str(folderind)
    print("Creating new directory %s"%outputdir)
    os.mkdir(outputdir)
    
else:
    os.mkdir(outputdir)

outname = "status_info%s.txt"%(folderind)
if processing:
    with open(listname) as fin:
        #pdb.set_trace()
        count = 0
        for line in fin:
            if not 'UWVV' in line:
                continue
            folder=line.strip()

            command = 'crab status -d '+folder+' > '+os.path.join(outputdir,folder+'.txt 2>&1')
            command2 = 'crab report -d '+folder+' > '+os.path.join(outputdir,folder+'_Report.log 2>&1')
            code = subprocess.call([command], shell=True)
            if options.report:
                code2 = subprocess.call([command2], shell=True)
            if code == 0:
                count +=1
                print("Processed %s folders"%count)
            else:
                print("Error in %s"%folder)
            
            if os.path.exists(os.path.join(folder,"results","notFinishedLumis.json")):
                print("==========WARNING: %s has not-yet-processed lumi=========="%folder)

fout = open(outname, 'w')

with open(listname) as flist:
    for line in flist:
        if not "UWVV" in line:
            continue
        fname = line.strip()
        
        with open(os.path.join(outputdir,fname+'.txt')) as fc:
            linecount=0
            wrong=True
            record = False
            for linec in fc:
                if 'Jobs status:' in linec:
                    record = True
                    wrong=False
                    fout.write(fname+':\n')
                if record and linecount<3:
                    if not "No publication information (publication has been disabled in the CRAB configuration file)" in linec:
                        fout.write(linec)
                    linecount +=1
                if linecount >=3:
                    record = False
                    linecount = 0
                    fout.write("\n")
                
            if wrong:
                fout.write('\nSomething wrong with %s\n\n'%fname)

fout.close()
print("Info output saved as %s"%outname)    

print("Writing resubmit script. Before running resubmission please check status txt to make sure no job is still running or in transition")
fstat = open(outname)
fre = open(outname.replace("status_info","resubmit").replace(".txt",".sh"),"w")
relist = []
current = ""
for line in fstat:
    if "crab_" in line:
        current = line.strip()

    if "failed" in line:
        relist.append(current)

for entry in relist:
    fre.write("crab resubmit -d "+entry.replace(":","")+"\n")