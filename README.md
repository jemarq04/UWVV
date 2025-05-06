# UWVV

The main reference for Run 3 analysis is the [PdmV recipe](https://twiki.cern.ch/twiki/bin/viewauth/CMS/PdmVRun3Analysis).
Additional README files can be found in `AnalysisTools/` and `Ntuplizer/` that explain how to create/modify new analysis steps and 
how to understand event/object branches, respectively. The README in `AnalysisTools/` is essential for understanding the analysis workflow.
You can also look through `Utilities/Ntuplizer/ntuplize_cfg.py`, as it is carefully commented and organized to make things easy to understand.

## Table of Contents

- [Setup](#setup)
- [Running local jobs](#running-local-jobs)
- [Submitting CRAB jobs](#submitting-crab-jobs)
   * [Submitting CRAB jobs for custom MC](#submitting-crab-jobs-for-custom-mc)

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
it into the config script). To get information on all of the available options, you can run `cmsRun ntuplize_cfg.py --help`. This will display each option, its default, and
its description.

There are two helper scripts for quick submission (and can act as a template for commands): `runMC.sh` and `runData.sh`. You can get a basic
help screen for these commands by running them without any arguments. The first argument for these scripts is a file that lists each of the input files for the process.
An example can be found in `inputs/template.dat`. All files included in the `inputs/` directory are ignored by git. 
An example usage of these scripts is: `./runMC.sh inputs/2022MC.dat 2022 jetsUL=1 outputFile=someName.root`.

## Submitting CRAB jobs

Before submitting any CRAB jobs, you must set up the CMS proxy through `voms-proxy-init`. You must also go into `Utilities/scripts/crab.py` and edit the `username`
variable near the top of the script to your CERN username.

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
of the analysis. If this is provided, the file `Utilities/test/CrabTemplates/local.<YEAR>.cfg` will be copied to your current directory as `local.cfg`. 
**NOTE**: This will overwrite a pre-existing config file in that directory! If you do not provide this argument, the script will search for a config file in your directory 
named `local.cfg`. This can be helpful to avoid overwriting any temporary changes made to the config.

The helper script will print out commands to run to submit the jobs using `Utilities/scripts/crab.py`. You can pipe this output to `stdin` and run them immediately. 
An example usage of the script is:

```bash
cd Utilities/test
crabSubmit.sh datasets/2022MC_qqZZ.dat | . /dev/stdin
```

Again, to preview the commands you can run without piping to `stdin` simply with `crabSubmit.sh datasets/2022MC_qqZZ.dat`. This script will submit a CRAB job using 
`Utilities/scripts/crab.py` and configure it with the `local.cfg` file in your current directory. This script will automatically determine certain options not specified
in the config file, such as whether the sample is MC or data, prompt analysis, pre- or postEE, etc. Based on these checks, the relevant global tag will be read from the
config file.

Note that Run 3 data CRAB jobs are submitted using the golden JSON files present in `Utilities/scripts/JSON`. These were downloaded from the PdmV website 
(linked at the beginning of this file), but may need updating later on.

You can view the status of running/completed CRAB jobs with `crab status -d <DIR>` or view the final report of a completed job with `crab report -d <DIR>`. The report is 
especially useful for seeing how much luminosity was processed out of the expected amount.

### Submitting CRAB jobs for custom MC

To submit jobs with custom MC, once again you should go to `Utilities/test` and use the helper script `crabSubmitCustom.sh`. This script will also submit a CRAB job using
`Utilities/scripts/crab.py` and configure it with the `local.cfg` file in your current directory. Calling the `crabSubmitCustom.sh` helper script will run the python script
with a new flag, so that it recognizes it is running private MC samples. The python script will act almost identically to the process 
mentioned above, except it requires three new variables in the config file that are normally ignored. The lines are the following:

```
dataset: /CustomSet/%(mcGlobalTag)s/MINIAODSIM
requestName: CustomRequestName
datalist: CustomData.dat
#postEE: 1
#postBPix: 1
```

Feel free to edit the values in these lines however you wish for the submission. The only exception is "MINIAODSIM" must be kept to denote this as MC. 
The `datalist` option points to an input file (similar to those mentioned in the local job submission section) that lists all input files for the job. Once again,
empty lines or lines beginning with `#` are ignored.

**NOTE**: The `postEE` and `postBPix` options are derived from the dataset conditions (the second string in the /-separated list), so for these custom submissions
you need to specify if you want either of these options yourself. You can uncomment the relevant line in the code snippet above.

