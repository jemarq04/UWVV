# UWVV

# Documentation on Run 2 UL Processing

## Updating to Run 2 UL
The UWVV codes are originally for processing pre-legacy Run 2 samples and creating ntuples. To update the codes to process Run 2 UL samples while keeping the original working codes for pre-legacy, as a temporary solution, UL versions are created for certain files, and they replace the original files during initial setup for UL (see setup instructions below). The main reference for updating to UL is the PdmV receipe: https://twiki.cern.ch/twiki/bin/view/CMS/PdmVRun2LegacyAnalysis

### Updates that have been done so far:

1. **Muon ID:** \
We are no longer able to use the Muon MVA from HZZ group designed for pre-legacy (used for 2018 in pre-legacy) as it gives errors with higher CMSSW release. Currently for all three years the UL muon ID is set to use vertex+kinematic+type requirements + PF muon or high pt muon (used for 2016 and 2017 in pre-legacy). To see the muon IDs that actually get used among the ones stored in the ntuple, refer to ZZSelector codes (can be different muon ID names in different years).\
\
[major commit here](https://github.com/hhe62/UWVV/commit/04884f66e966f5c303ce1d9f99f593c414e14f42)

2. **Muon Rochester corrections:**\
The [muon calibration configuration file](https://github.com/hhe62/UWVV/blob/master/AnalysisTools/python/templates/MuonCalibrationUL.py) has been updated according to the [muon Rochester information](https://twiki.cern.ch/twiki/bin/viewauth/CMS/RochcorMuon). UL Rochester txt files need to be manually downloaded as in the setup instructions below. In the [configuration file]((https://github.com/hhe62/UWVV/blob/master/AnalysisTools/python/templates/MuonCalibrationUL.py)), for 2016, whether to use "RoccoR2016aUL" or "RoccoR2016bUL" as the input file is currently determined from the input global tag for the dataset (see the [main cmsRun configuration file](https://github.com/hhe62/UWVV/blob/master/Ntuplizer/test/ntuplize_cfg_UL.py) for the "CalibULera16" variable). **This doesn't work for data and modification is needed** to use the correct input file for the corresponding preVFP/postVFP data period (if not manually editing it each time). The central [plugin](https://github.com/hhe62/UWVV/blob/master/AnalysisTools/plugins/RochesterPATMuonCorrector.cc) that applies the correction is inheritted from pre-legacy version without changes.
   
3. **Electron ID**:\
For Egamma the overall reference page is [here](https://twiki.cern.ch/twiki/bin/view/CMS/EgammaUL2016To2018). The HZZ electron MVA is detailed [here](https://twiki.cern.ch/twiki/bin/view/CMS/MultivariateElectronIdentificationRun2#HZZ_MVA_training_details_and_wor). Currently we ask the [EgammaPostRecoTools](https://twiki.cern.ch/twiki/bin/view/CMS/EgammaPostRecoRecipes) (see next item) to run VID for UL HZZ MVA in the [calibration config file](https://github.com/hhe62/UWVV/blob/master/AnalysisTools/python/templates/ElectronCalibrationUL.py), and the corresponding MVA cut values are entered in the [ZZ ID config file](https://github.com/hhe62/UWVV/blob/master/AnalysisTools/python/templates/ZZID_UL.py).\
\
Some **concern**\: In [EgammaPostRecoTools page](https://twiki.cern.ch/twiki/bin/view/CMS/EgammaPostRecoRecipes#Runing_E_gamma_Post_reco_tools) it says that we need to load a few services to run VID, but for pre-legacy UWVV our process.load() commands are somewhat different from this instruction, for example, we don't call process.load("Configuration.StandardSequences.MagneticField_cff") explicitly. I didn't add the suggested load commands in the UL version either. So far it doesn't seem to give error but it may be worth looking into and verifying. \
\
Major commits:
[1](https://github.com/hhe62/UWVV/commit/d1b96085c9648b49f421105bed0b0623292a152e)
[2](https://github.com/hhe62/UWVV/commit/eb32a3f51aba5b0f744a559f9c27abfe92d6b92f)

4. **Electron/photon energy scale and smearing:**\
   The [setup script](https://github.com/hhe62/UWVV/blob/master/recipe/setup_UL.sh) and [electron calibration configuration file](https://github.com/hhe62/UWVV/blob/master/AnalysisTools/python/templates/ElectronCalibrationUL.py) have been updated to use the EgammaPostRecoTools following the [UL recipe (1)](https://twiki.cern.ch/twiki/bin/viewauth/CMS/EgammaUL2016To2018#Recipe_for_running_scales_and_sm). Currently for 2016 it is determined from the input global tag whether to use "2016preVFP-UL" or "2016postVFP-UL" for the era option (see the [main cmsRun configuration file](https://github.com/hhe62/UWVV/blob/master/Ntuplizer/test/ntuplize_cfg_UL.py) for the "CalibULera16" variable), but this only works for MC, and **modifications are still needed to use the proper era for 2016 data** (if not editing the config manually each time). As in [PATElectronZZIDEmbedder.cc](https://github.com/hhe62/UWVV/blob/master/AnalysisTools/plugins/PATElectronZZIDEmbedder.cc), the correction is applied following the [recipe (2)](https://twiki.cern.ch/twiki/bin/view/CMS/EgammaMiniAODV2#Energy_Scale_and_Smearing) (see the part containing "ecalTrkEnergyPostCorr"). According to [receipe (1)](https://twiki.cern.ch/twiki/bin/viewauth/CMS/EgammaUL2016To2018#Recipe_for_running_scales_and_sm), the two variables energyScaleUp/energyScaleDown are the ones to use for systematic shifts; they are stored during PATElectronZZIDEmbedder.cc, but we didn't look into this systematic unc. in the ZZ+jets analysis.\
\
Small **concern**:
The setup commands for EgammaPostRecoTools from the [main egamma page](https://twiki.cern.ch/twiki/bin/viewauth/CMS/EgammaUL2016To2018#Recipe_for_running_scales_and_sm) is not the same as on the [EgammaPostRecoTools page](https://twiki.cern.ch/twiki/bin/view/CMS/EgammaPostRecoRecipes). Currently the recipe/setup_UL.sh script just follows the former.

5. **L1 prefiring weight**:\
The [main configuration file](https://github.com/hhe62/UWVV/blob/master/Ntuplizer/test/ntuplize_cfg_UL.py) for cmsRun has been updated following the [L1PrefiringWeight recipe](https://twiki.cern.ch/twiki/bin/view/CMS/L1PrefiringWeightRecipe). For 2016 it is also determined from the input global tag whether to use "2016preVFP-UL" or "2016postVFP-UL" for the era, but we don't need to worry about data for L1 prefiring weights. One can refer to ZZSelector codes where the L1preifirng weights are applied. As in the [L1PrefiringWeight recipe](https://twiki.cern.ch/twiki/bin/view/CMS/L1PrefiringWeightRecipe) and the comment in this [file](https://github.com/hhe62/UWVV/blob/master/Ntuplizer/python/eventParams.py), to look at weight for individual object (not needed by default), we need to e.g. replace 'prefweight' : 'prefiringweight:nonPrefiringProb' with 'prefiringweight:nonPrefiringProbECAL'.\
\
Major commits:
[1](https://github.com/hhe62/UWVV/commit/9fcd0e793ea626a9b3dd83645754cb9112d3e559)
[2](https://github.com/hhe62/UWVV/commit/7c2de2332799ee4c569e2d5900bc7eb32fc5cf20)
[3](https://github.com/hhe62/UWVV/commit/9a574a16584f33327012a87d490fb46907cdbd24)

6. **Sample processing**:\
The configuration files Utilities/scripts/CrabTemplates/local.allweights201*UL.txt that set the input options for CRAB submission have been updated to UL according to [PdmV receipe](https://twiki.cern.ch/twiki/bin/view/CMS/PdmVRun2LegacyAnalysis) at the time of the update, but updates to e.g. global tags may be needed in the future. The major [crab.py file](https://github.com/hhe62/UWVV/blob/master/Utilities/scripts/crab.py) has been updated to determine if the sample is UL (and for 2016 if it is preVFP/postVFP) based on the condition in the sample name, and handles different cases. However, the condition names are hard-coded and may be changed in the future, and **the golden json files in config.Data.lumiMask for the 3 years still need to be updated to UL golden json file names** (see the [lumi information page](https://twiki.cern.ch/twiki/bin/view/CMS/LumiRecommendationsRun2)). Also as mentioned above, though 2016 preVFP/postVFP is known from sample name for both MC and data, during the acutal processing, currently it is only determined from globaltag, which works for MC but not data, and **modification is needed**.
   
8. **Additional**:\
To resolve error, had to make changes to access BadChargedCandidateFilter and BadPFMuonFilter within miniAOD directly instead of running some modules to obtain them as in pre-legacy. According to previous comments they were used for MET filters with recommendation [here](https://twiki.cern.ch/twiki/bin/viewauth/CMS/MissingETOptionalFiltersRun2#MET_Filter_Recommendations_for_R). I haven't looked into the actual use of these flags after stored into ntuple, just for keeping the codes running. May need to verify them if need to use them in the future.\
\
Major commits: 
[1](https://github.com/hhe62/UWVV/commit/596db5ad6297353534aaa824a26d8a305c92a9ea)
[2](https://github.com/hhe62/UWVV/commit/c49a069da7e51ddb8f31342d4a4dcc68c743cbd4)

### Possible updates/actions still needed
Again the main reference to check is the [PdmV receipe](https://twiki.cern.ch/twiki/bin/view/CMS/PdmVRun2LegacyAnalysis). Some items have been discussed above but also repeated here.

1. Making sure the lepton IDs and SFs are the correct and optimal ones to use. In particular, in pre-legacy we use lepton SF files and recipe from HZZ group, but we currently don't have the updated version of SFs for UL and are just applying the old ones in later processing stage.

2. ~~Jet related contents still need to be reviewed and updated, in particular JEC, JER and Pileup Jet ID in the PdmV corrections list.~~ JEC and JetPUID match PdmV Run2UL. JER differs, but matches the HZZ4l analysis.

3. Modifying the codes to use the appropriate names/tags for 2016 preVFP/postVFP samples. As discussed in the list of updated items above, during processing, currently preVFP/postVFP is only determined from global tag, which works for MC but not data. Need to maybe pass some other variable into processing to provide this information for data. Also will need to handle/combine the two eras properly in later processing and plotting.

4. Keeping datasets, global tags, lumi, golden json files etc. up to date according to UL Run 2 recommendations. Since the recommendations may be updated from time to time, we may need to confirm with relevant experts the most up-to-date recommendations. For example, we see alternative /DoubleMuon/Run2018 datasets in this [table](https://pdmv-pages.web.cern.ch/rereco_ul/full_table.html?miniaod_dataset=miniaodv2&nanoaod_dataset=!JME&aod_dataset=15Feb2022) (BParking vs GT36 which we currently use, but the alternative one may not be the preferred one). As discussed above, the golden json files in config.Data.lumiMask for the 3 years need to be updated to UL golden json file names if not already (see the [lumi information page](https://twiki.cern.ch/twiki/bin/view/CMS/LumiRecommendationsRun2)).
   
5. Pileup and trigger are in tbc status in PdmV page, not sure.

6. Maybe verify additional items such as the two concerns and MET filter mentioned in the list of updated items above. 

## Setup


```bash
cmsrel CMSSW_10_6_30
cd CMSSW_10_6_30/src
cmsenv
git cms-init
git clone --recursive git@github.com:jemarq04/UWVV (or the forked repository)
cd UWVV
source recipe/setup_UL.sh
```
In UWVV directory, create data/jetPUSF directory 

```
 mkdir -p data/jetPUSF
```
and download effcyPUID_81Xtraining.root and scalefactorsPUID_81Xtraining.root from the bottom attachments of https://twiki.cern.ch/twiki/bin/viewauth/CMS/PileupJetID, and then put them into the created directory. These files are for jet PU ID, and they are currently needed to make the codes run.

~~Download roccor.Run2.v5.tgz from https://twiki.cern.ch/twiki/bin/viewauth/CMS/RochcorMuon, and after uncompressing, put all the RoccoR*.txt files into the UWVV/data/RochesterCorrections folder (can overwrite the old 2018.txt file).~~ This has now been added to the `setup.sh` and `setup_UL.sh` recipes using the CERN gitlab repository for RoccoR corrections.

## Crab Submission
Create files like 2016MC.dat, 2017Data.dat, ..., and in each file, list line by line the datasets you want to process.

For example, in 2016MC.dat, list 2016 datasets:\
/ZZto4L_EWK_PolarizedZ0Z0_TuneCP5_13TeV-madgraph-pythia8/RunIISummer20UL16MiniAODAPVv2-106X_mcRun2_asymptotic_preVFP_v11-v2/MINIAODSIM \
/ZZto4L_EWK_PolarizedZ0Z0_TuneCP5_13TeV-madgraph-pythia8/RunIISummer20UL16MiniAODv2-106X_mcRun2_asymptotic_v17-v2/MINIAODSIM\
...

Then change to the UWVV/Utilities/scripts folder, and in crab.py change the user name to the lxplus user name:

```
outdir = localSettings.get("local", "outLFNDirBase").replace(
    "$USER", '[your user name]').replace("$DATE", today)
```

The following commands use 2016 as example, but can be replaced with 2017 and 2018.

You can preview the CRAB submission commands by running:
```
./crabSubmit.sh [path to]/MC2016.dat  2016UL
```
and this will copy the corresponding UL config to local.txt, and print out a list of "crab submit" commands for the listed datasets in the .dat file. If the commands and dataset names look appropriate, then we can do the usual voms-proxy-init... and submit the CRAB jobs by running:

```
 ./crabSubmit.sh [path to]/MC2016.dat  2016UL|. /dev/stdin
```

In the crab.py file, we can either use\
\
config.Data.splitting = 'Automatic'\
config.Data.unitsPerJob = someNumber\
\
or\
\
config.Data.splitting = 'FileBased'\
config.Data.unitsPerJob = 1\
\
Previously I encountered an issue in processing pre-legacy 2018 data using Automatic where it gave wrong impression on the amount of data not processed, so personally I think it may be safer to use FileBased.

After some processing time, we can use the latest version of this [script](https://github.com/hhe62/UWVV/blob/master/Utilities/scripts/check_crab_status.py) to check the crab status for all the CRAB project folders together:

The usage is written at the beginning of the script. We can create a folder and put all the CRAB project directories and the script into the folder, and run 

```
python check_crab_status.py
```

Then can look at the status summary in status_info.txt.

Currently it only reads up to 3 lines of status for each dataset for compactness, e.g.:

<pre>
crab_UWVVNtuples18_19Jul2023_DoubleMuon_Run2018A-UL2018_MiniAODv2_GT36-v1:
Jobs status:           failed                   13.9% ( 142/1022)
                       finished                 86.0% ( 879/1022)
                       running                   0.1% (   1/1022)
</pre>

so if there is a fourth line (e.g. "transferring" in addition to "failed","finished","running") it won't show up, but for purpose of determining the job status after sufficient processing time has passed, this should be good enough.

If there are failed jobs, the script will create a resubmission script. After making sure everything is ready (e.g. number of finished and failed jobs sum up to total, no "running" or "transferring" in the third line of status), we can run the resubmission script. It can take several resubmission until all the failed jobs disappear (or there can sometimes be persistent ones.)

If we use --report option, it will also run "crab report -d" on all CRAB directories, and we can look at all the printouts using e.g. cat output_crab_status_data/*.log.

(In my case, I had one job still showing running status for a long time, and it may need some retry if it doesn't work out.)

## Local Test

In the Ntuplizer/test directory, modify the cmsRun config file ntuplize_cfg_UL.py to include the source root file, and run

```
./runMCUL.sh
```
after uncommenting the desired year in the script. Note that some input options in the local test script are different from the options used for formal CRAB submission (see Utilities/scripts/CrabTemplates/local.allweights201*UL.txt).

# Previous prescription on pre-legacy RUn 2 processing
Some tools for CMS analyses

UWVV is designed for analyses that use final state particles (typically leptons) to reconstruct intermediate and initial states. For example, in the H->ZZ->4l analysis, electron and muon pairs are built into Z candidates, and the Z candidates are built into Higgs candidates. It contains tools for building a full analysis flow out of CMS EDM modules, and for making flat ntuples where each row represents one initial state candidate.

It uses the [CMSSW framework](https://github.com/cms-sw/cmssw) and expects [miniAOD](https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookMiniAOD2017) input. Much of the inspiration (and a little bit of the code) comes from [FSA](https://github.com/uwcms/FinalStateAnalysis/). A few tools, like the batch submission scripts, are specific to the computing infrastructure at the University of Wisconsin - Madison.

## Setup
Current supported CMSSW release: `CMSSW_10_2_X`

```bash
scram pro -n uwvv CMSSW CMSSW_10_2_15
cd uwvv/src
cmsenv
git cms-init
git clone --recursive git@github.com:uhussain/UWVV.git
cd UWVV
source recipe/setup.sh
pushd ..
(To avoid compilation errors although we don’t need this anymore in 2017/2018: https://github.com/CJLST/ZZAnalysis/blob/miniAOD_80X/checkout_10X.csh#L89)
git clone https://github.com/bachtis/Analysis.git -b KaMuCa_V4 KaMuCa
scram b -j 12
cd UWVV/Ntuplizer/test/
./runMC.sh
```
## Use
To make a basic ntuple of four-lepton final state candidates, do

```bash
cmsRun ntuplize_cfg.py channels=zz isMC=1 eCalib=1 muCalib=1 year=2018
```

For more on how to build your own analysis, see the `AnalysisTools` directory. For more on making ntuples, see the `Ntuplizer` directory.

For submitting jobs using crab, see the `Utilities` directory.
```bash
./crabSubmit.sh /data/uhussain/ZZTo4l/ZZ2018/uwvv/src/UWVV/MetaData/ZZDatasets/ZZ2018Data_MiniAOD.dat | grep "ZZ" | . /dev/stdin
```
