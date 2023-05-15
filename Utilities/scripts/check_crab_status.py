import subprocess
import pdb
import os
import sys
from optparse import OptionParser

processing=True
print("Running scripts. Don't forget to initialize proxy first.\n See the latest folder with largest index.")
parser=OptionParser()
parser.add_option("-f", dest="folder",help="Output Status folder")
parser.add_option("-o", dest="output",help="Output info file name")
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

            command = 'crab status -d '+folder+' > '+os.path.join(outputdir,folder+'.txt')
            code = subprocess.call([command], shell=True)
            if code == 0:
                count +=1
                print("Processed %s folders"%count)
            else:
                print("Error in %s"%folder)

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
                    fout.write(linec)
                    linecount +=1
                if linecount >=3:
                    record = False
                    linecount = 0
                
            if wrong:
                fout.write('\nSomething wrong with %s\n\n'%fname)

fout.close()
print("Info output saved as %s"%outname)    
