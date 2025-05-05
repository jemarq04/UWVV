# UWVV

The main reference for Run 3 analysis is the [PdmV recipe](https://twiki.cern.ch/twiki/bin/viewauth/CMS/PdmVRun3Analysis).
Additional README files can be found in `AnalysisTools/` and `Ntuplizer/` that explain how to create/modify new analysis steps and 
how to understand event/object branches, respectively.

## Setup

To use this framework, you must be in a fresh CMSSW environment. Instructions are below.

```bash
#Create your CMSSW environment
cmsrel CMSSW_14_0_9
cd CMSSW_14_0_9/src

#Initialize
cmsenv
git cms-init

#Clone UWVV repo
git clone -b Run3 git@github.com:jemarq04/UWVV #(or your forked repository)
source UWVV/setup.sh
```

## Running local jobs

Jobs can be run locally in the `Ntuplizer/test` directory with the command `cmsRun ntuplize_cfg.py <OPTIONS>`, where `<OPTIONS>` denotes your desired processing settings. 
Available options are listed within `ntuplize_cfg.py`, and in theory you should be able to specify modifications to the job using the command-line (instead of hard-coding
it into the config script). 

There are two helper scripts for quick submission (and can act as a template for commands): `runMC.sh` and `runData.sh`. You can get a basic
help screen for these commands by running them without any arguments. The first argument for these scripts is a file that lists each of the input files for the process.
An example can be found in `inputs/template.dat`. All files included in the `inputs/` directory are ignored by git. 
An example usage of these scripts is: `./runMC.sh inputs/2022MC.dat 2022 jetsUL=1 outputFile=someName.root`.

## Submitting CRAB jobs

This branch is currently in development - more documentation to come.
