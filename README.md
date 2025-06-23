# UWVV

The main reference for Run 3 analysis is the [PdmV recipe](https://twiki.cern.ch/twiki/bin/viewauth/CMS/PdmVRun3Analysis).
Additional README files can be found in [`AnalysisTools/`](AnalysisTools/README.md) and [`Ntuplizer/`](Ntuplizer/README.md) that explain how to
create/modify new analysis steps and how to understand event/object branches, respectively. The README in `AnalysisTools/` is essential for understanding the
analysis workflow. You can also look through [`ntuplize_cfg.py`](Utilities/Ntuplizer/ntuplize_cfg.py), as it is carefully commented and organized to make things easy to
understand. Note that this framework has been upgraded to Run 3, but only for anything necessary for Run 3 ZZ analysis. You may need to edit modules if you want
anything other than ZZ.

## Table of Contents

- [Setup](#setup)
- [Running local jobs](#running-local-jobs)
- [Submitting CRAB jobs](#submitting-crab-jobs)
   * [Submitting CRAB jobs for custom MC](#submitting-crab-jobs-for-custom-mc)
- [Running `pre-commit`](#running-pre-commit)

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

Jobs can be run locally in the [`Ntuplizer/test`](Ntuplizer/test) directory with the command `cmsRun ntuplize_cfg.py <OPTIONS>`, where `<OPTIONS>` denotes your
desired processing settings. Available options are listed within [`ntuplize_cfg.py`](Ntuplizer/test/ntuplize_cfg.py), and in theory you should be able to specify
modifications to the job using the command-line (instead of hard-coding it into the config script). To get information on all of the available options, you
can run `cmsRun ntuplize_cfg.py --help`. This will display each option, its default, and its description.

There are two helper scripts for quick submission (and can act as a template for commands): [`runMC.sh`](Ntuplizer/test/runMC.sh) and
[`runData.sh`](Ntuplizer/test/runData.sh). You can get a basic help screen for these commands by running them without any arguments. The first argument for these
scripts is a file that lists each of the input files for the process. An example can be found in [`inputs/template.dat`](Ntuplizer/test/inputs/template.dat).
All files included in the [`inputs/`](Ntuplizer/test/inputs) directory are ignored by git. An example usage of these scripts is:
`./runMC.sh inputs/2022MC.dat 2022 jetsUL=1 outputFile=someName.root`.

## Submitting CRAB jobs

Before submitting any CRAB jobs, you must set up the CMS proxy through `voms-proxy-init`. You must also go into [`Utilities/test/local.cfg`](Utilities/test/local.cfg)
and edit the `username` option in the `DEFAULT` section to your CERN username.

The relevant scripts for CRAB job submissions are located in [`Utilities/scripts`](Utilities/scripts). When the CMSSW environment is compiled with `scram`,
any executable scripts in this directory will be copied into `$CMSSW_BASE/bin/$SCRAM_ARCH` and will be available to run from anywhere.
The main script in this directory is a helper script to submit CRAB jobs for central samples: [`crabSubmit.sh`](Utilities/scripts/crabSubmit.sh).
To see how to use this script, run it without any arguments. **NOTE**: To submit jobs, go to the [`Utilities/test`](Utilities/test) directory. This is just
to avoid cluttering other directories (and possible recursive tarballs for submissions - ouch!).

Another important file for CRAB submissions is the file [`Utilities/test/local.cfg`](Utilities/test/local.cfg). This file defines all of the settings that the
CRAB script will use to configure the job. The values in the `DEFAULT` section should remain untouched - the only exception is the `setup` value. This option should
name a section in the config file for the CRAB script to access. For example, if you want to submit jobs with the `2022` settings, the line would read `setup: 2022`.

The first argument of the `crabSubmit.sh` script is an input file listing all the desired input datasets. Input files for Run 3 can be found in
[`Utilities/test/datasets`](Utilities/test/datasets). For example, the file [`2022MC_qqZZ.dat`](Utilities/test/datasets/2022MC_qqZZ.dat) contains the following:

```bash
#preEE
/ZZto4L_TuneCP5_13p6TeV_powheg-pythia8/Run3Summer22MiniAODv4-130X_mcRun3_2022_realistic_v5-v2/MINIAODSIM
/ZZto4L_TuneCP5_13p6TeV_powheg-pythia8/Run3Summer22MiniAODv4-130X_mcRun3_2022_realistic_v5_ext1-v2/MINIAODSIM

#postEE
/ZZto4L_TuneCP5_13p6TeV_powheg-pythia8/Run3Summer22EEMiniAODv4-130X_mcRun3_2022_realistic_postEE_v6-v2/MINIAODSIM
/ZZto4L_TuneCP5_13p6TeV_powheg-pythia8/Run3Summer22EEMiniAODv4-130X_mcRun3_2022_realistic_postEE_v6_ext1-v2/MINIAODSIM
```

Empty lines or lines beginning with `#` are ignored so that the files can be made readable. The `crabSubmit.sh` helper script also takes an optional second argument
specifying the settings you wish to use for the job. If this argument is not provided, the `setup` value in the `DEFAULT` section is unchanged and the CRAB script
uses the settings from the named section. If a second argument is provided to the script, it will attempt to find a section name matching that argument. If found,
the `local.cfg` file will be modified to change the `setup` value to the provided name. In the special case that `auto` is provided as the second argument, the script
will look for a section name matching the first four characters of the base filename provided as the first argument. For example:

```bash
cd Utilities/test
crabSubmit.sh datasets/2022MC.dat auto
# With 'auto' specified, the script will look at the base filename '2022MC'
#  and look for a section named '2022'. Since it exists, it will change the
#  value of the 'setup' option to '2022' and run the CRAB script.
```

There is also an autocompletion helper script for running `crabSubmit.sh`. This will provide options to the terminal when hitting `<Tab>` while typing the command. To
enable this, run `source Utilities/test/autocomp.sh`. Note that this will need to be run each time you start a new terminal session - it would be good to add to some
sort of setup script you may have.

The helper script will print out commands to run to submit the jobs using `Utilities/scripts/crab.py`. You can pipe this output to `stdin` to run them immediately.
An example usage of the script is:

```bash
cd Utilities/test
crabSubmit.sh datasets/2022MC_qqZZ.dat | . /dev/stdin
```

Again, to preview the commands you can run without piping to `stdin` simply with `crabSubmit.sh datasets/2022MC_qqZZ.dat`. This script will submit a CRAB job using
[`Utilities/scripts/crab.py`](Utilities/scripts/crab.py) and configure it with the `local.cfg` file in your current directory. This script will automatically
determine certain options not specified in the config file, such as whether the sample is MC or data, prompt analysis, pre- or postEE, etc.
Based on these checks, the relevant global tag will be read from the config file.

Note that Run 3 data CRAB jobs are submitted using the golden JSON files present in [`Utilities/scripts/JSON`](Utilities/scripts/JSON). These were downloaded from
the PdmV website (linked at the beginning of this file), but may need updating later on.

You can view the status of running/completed CRAB jobs with `crab status -d <DIR>` or view the final report of a completed job with `crab report -d <DIR>`. The report is
especially useful for seeing how much luminosity was processed out of the expected amount.

### Submitting CRAB jobs for custom MC

To submit jobs with custom MC, once again you should go to [`Utilities/test`](Utilities/test) and use the helper script
[`crabSubmitCustom.sh`](Utilities/scripts/crabSubmitCustom.sh). This script will also submit a CRAB job using [`Utilities/scripts/crab.py`](Utilities/scripts/crab.py)
and configure it with the [`local.cfg`](Utilities/test/local.cfg) file in your current directory. Calling the `crabSubmitCustom.sh` helper script will run the python script
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

## Running `pre-commit`

This repository has some configuration set up for minor linting/formatting checks. Note that this is not required, but it can keep the code nicely
formatted. To do this, you need to install `pre-commit` either through `pip` or using a virtual environment (such as through `uv`). For a user
installation through `pip`, simply run `pip install --user pre-commit`. Virtual environments can be helpful if you only need certain packages
while in certain environments, but will need some initial setup. A new tool for this is `uv`, and it works well. If you want to use `uv`, you can run
the following code.

```bash
pip install --user uv
uv venv
uv pip install pre-commit ruff
uv pip uninstall virtualenv # to avoid conflicts with the one from CMSSW
source .venv/bin/activate
#to deactivate this virtual environment and restore your usual python, run 'deactivate'
```

After you've set up the virtual environment, you need to 'activate' it by running `source .venv/bin/activate`. This will bring this local python
environment into scope and allow you to run `pre-commit`. To deactivate this virtual environment and restore your setup, run `deactivate`.

To run the command, simply run `pre-commit run -a`. It will go through a series of checks/formats over all of the files. The configuration for
this command can be found in [`.pre-commit-config.yaml`](.pre-commit-config.yaml). Other linting/formatting checks are done through `ruff` and are
configured in [`.ruff.toml`](.ruff.toml).
