import subprocess
import pdb
import os
import sys
from optparse import OptionParser

processing=True
print("Running scripts. Don't forget to initialize proxy first.\n Rename folder if want to reprocess.")
parser=OptionParser()
parser.add_option("-f", dest="folder",help="Output Status folder")
parser.add_option("-o", dest="output",help="Output info file name")
(options,args)=parser.parse_args()

outputdir='output_crab_status_data/'
if options.folder:
    outputdir=options.folder+'/'

listname="folder_list.txt"
outname="status_info.txt"
if options.output:
    outname=options.output

subprocess.call("ls > %s"%listname,shell=True)

if os.path.isdir(outputdir):
    print("folder %s already exists, reading from previous results"%outputdir)
    processing=False
else:
    os.mkdir(outputdir)

if processing:
    with open(listname) as fin:
        #pdb.set_trace()
        count = 0
        for line in fin:
            if not 'UWVV' in line:
                continue
            folder=line.strip()

            command = 'crab status -d '+folder+' > '+outputdir+folder+'.txt'
            code = subprocess.call([command], shell=True)
            if code == 0:
                count +=1
                print("Processed %s files"%count)
            else:
                print("Error in %s"%folder)

fout = open(outname, 'w')

with open(listname) as flist:
    for line in flist:
        if not "UWVV" in line:
            continue
        fname = line.strip()
        linecount=0
        with open(outputdir+fname+'.txt') as fc:
            wrong=True
            for linec in fc:
                if 'Jobs status:' in linec or linecount==1:
                    linecount+=1
                    wrong=False
                if linecount==1:
                    fout.write(fname+':\n'+linec.strip()+'\n')
                if linecount==2:
                    fout.write('                                '+linec.strip()+'\n')
                    linecount=0
            if wrong:
                fout.write('\nSomething wrong with %s\n\n'%fname)

fout.close()
print("Info output saved as %s"%outname)    
