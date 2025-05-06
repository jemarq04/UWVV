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

Before submitting any CRAB jobs, you must set up the CMS proxy through `voms-proxy-init`.

The relevant scripts for CRAB job submissions are located in `Utilities/scripts`. When the CMSSW environment is compiled with `scram`, 
these scripts will be copied into `$CMSSW_BASE/bin/$SCRAM_ARCH` and will be available to run from anywhere. To submit jobs, go to the `Utilities/test` directory.
CRAB jobs for central samples are submitted using a helper script `crabSubmit.sh`. To see how to use this script, run it without any arguments.

The first argument of the `crabSubmit.sh` script is an input file listing all the desired input datasets. Input files for Run 3 can be found in `Utilities/test/datasets`. 
For example, the file `Utilities/test/datasets/2022MC_qqZZ.dat` contains the following:

```bash
#preEE
/ZZto4L_TuneCP5_13p6TeV_powheg-pythia8/Run3Summer22MiniAODv4-130X_mcRun3_2022_realistic_v5-v2/MINIAODSIM
/ZZto4L_TuneCP5_13p6TeV_powheg-pythia8/Run3Summer22MiniAODv4-130X_mcRun3_2022_realistic_v5_ext1-v2/MINIAODSIM

#postEE
/ZZto4L_TuneCP5_13p6TeV_powheg-pythia8/Run3Summer22EEMiniAODv4-130X_mcRun3_2022_realistic_postEE_v6-v2/MINIAODSIM
/ZZto4L_TuneCP5_13p6TeV_powheg-pythia8/Run3Summer22EEMiniAODv4-130X_mcRun3_2022_realistic_postEE_v6_ext1-v2/MINIAODSIM
```

Empty lines or lines beginning with `#` are ignored so that the files can be made readable. The second argument of the `crabSubmit.sh` script is optional and is the year
of the analysis. If this is provided, the file `Utilities/test/CrabTemplates/local.allweights<YEAR>.cfg` will be copied to your current directory as `local.cfg`. *NOTE*: This will overwrite a pre-existing config file in that directory! If you do not provide this argument, the script will search for a config file in your directory 
named `local.cfg`. This can be helpful to avoid overwriting any temporary changes made to the config.

The helper script will print out commands to run to submit the jobs using `Utilities/scripts/crab.py`. You can pipe this output to `stdin` and run them immediately. 
An example usage of the script is:

```bash
cd Utilities/test
crabSubmit.sh datasets/2022MC_qqZZ.dat | . /dev/stdin
```

Again, to preview the commands you can run without piping to `stdin` simply with `crabSubmit.sh datasets/2022MC_qqZZ.dat`.

### Submitting CRAB jobs for custom MC

This branch is currently in development - more documentation to come.

