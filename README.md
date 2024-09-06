# UWVV

The main reference for Run 2 UL analysis is the PdmV receipe: https://twiki.cern.ch/twiki/bin/view/CMS/PdmVRun2LegacyAnalysis

## Setup

```bash
cmssw-el7
cmsrel CMSSW_10_6_35
cd CMSSW_10_6_35/src
cmsenv
git cms-init
git clone --recursive -b Run2UL git@github.com:jemarq04/UWVV #(or your forked repository)
source UWVV/recipe/setup.sh
```

## Crab Submission

The relevant scripts to submit crab jobs are located in `UWVV/Utilities/scripts` and so will be copied into `$CMSSW_BASE/bin/$SCRAM_ARCH/` automatically when you compile with `scram`. This is done for you in the setup recipe as well. Since it is in this `bin` directory, they will be in your PATH and can be run from any directory. To avoid cluttering that directory with crab outputs, submit your crab jobs from `UWVV/Utilities/test`.

Create files like like those in `UWVV/Utilities/test/datasets/` and in each file, list line by line the datasets you want to process.

For example, in 2016MC.dat, list 2016 datasets:
```
/ZZto4L_EWK_PolarizedZ0Z0_TuneCP5_13TeV-madgraph-pythia8/RunIISummer20UL16MiniAODAPVv2-106X_mcRun2_asymptotic_preVFP_v11-v2/MINIAODSIM
/ZZto4L_EWK_PolarizedZ0Z0_TuneCP5_13TeV-madgraph-pythia8/RunIISummer20UL16MiniAODv2-106X_mcRun2_asymptotic_v17-v2/MINIAODSIM
...
```

Then change to the UWVV/Utilities/scripts folder, and in crab.py change the user name to the lxplus user name:

```
outdir = localSettings.get("local", "outLFNDirBase").replace(
    "$USER", '[your user name]').replace("$DATE", today)
```

The following commands use 2016 as example, but can be replaced with 2017 and 2018.

You can preview the CRAB submission commands by changing to the `Utilities/test` directory and running:
```
crabSubmit.sh datasets/MC2016.dat 2016UL
```
This will copy the corresponding UL config to local.txt, and print out a list of "crab submit" commands for the listed datasets in the .dat file. If the commands and dataset names look appropriate, then we can do the usual setup of the CMS proxy through `voms-proxy-init` and submit the CRAB jobs by running:
```
crabSubmit.sh datasets/MC2016.dat  2016UL | . /dev/stdin
```
Note the space between the period and `/dev/stdin`.

In the crab.py file, we can either use
```python
config.Data.splitting = 'Automatic'
config.Data.unitsPerJob = someNumber
```
or
```python
config.Data.splitting = 'FileBased'
config.Data.unitsPerJob = 1
```
Previously I encountered an issue in processing pre-legacy 2018 data using Automatic where it gave wrong impression on the amount of data not processed, so personally I think it may be safer to use FileBased.

After some processing time, we can use the latest version of this [script](https://github.com/hhe62/UWVV/blob/master/Utilities/scripts/check_crab_status.py) to check the crab status for all the CRAB project folders together:

The usage is written at the beginning of the script. We can create a folder and put all the CRAB project directories and the script into the folder, and run 

```
python check_crab_status.py
```

Then can look at the status summary in status_info.txt.

Currently it only reads up to 3 lines of status for each dataset for compactness, e.g.:
```
<pre>
crab_UWVVNtuples18_19Jul2023_DoubleMuon_Run2018A-UL2018_MiniAODv2_GT36-v1:
Jobs status:           failed                   13.9% ( 142/1022)
                       finished                 86.0% ( 879/1022)
                       running                   0.1% (   1/1022)
</pre>
```
If there is a fourth line (e.g. "transferring" in addition to "failed","finished","running") it won't show up, but for the purpose of determining the job status after sufficient processing time has passed, this should be good enough.

If there are failed jobs, the script will create a resubmission script. After making sure everything is ready (e.g. number of finished and failed jobs sum up to total, no "running" or "transferring" in the third line of status), we can run the resubmission script. It can take several resubmission until all the failed jobs disappear (or there can sometimes be persistent ones.)

If we use --report option, it will also run `crab report -d <DIR>` on all CRAB directories, and we can look at all the printouts using e.g. `cat output_crab_status_data/*.log`.

(In my case, I had one job still showing running status for a long time, and it may need some retry if it doesn't work out.)

If needed, you can also use the crab subcommands yourself through `crab status -d <DIR>` and `crab resubmit -d <DIR>` as needed.

## Local Test

In the Ntuplizer/test directory, modify the cmsRun config file ntuplize_cfg_UL.py to include the source root file, and run `./runMCUL.sh` after uncommenting the desired year in the script. 

Alternatively, you can use the script `runMCUL_custom.sh` with an input file list to avoid having to manually edit the original config file. To see how to run the script, simply run `./runMCUL_custom.sh` and a usage printout will be provided. You can keep your input files in `UWVV/Ntuplizer/test/inputs/` so that git will ignore the files. There is a `template.dat` file there that can be used as an example for how to make your own.

Note that some input options in the local test script are different from the options used for formal CRAB submission (see Utilities/scripts/CrabTemplates/local.allweights201*UL.txt).
