#!/usr/bin/env bash

dasgoclient -limit 50 -query "file dataset=/ZZto4L_TuneCP5_13p6TeV_powheg-pythia8/Run3Summer22MiniAODv4-130X_mcRun3_2022_realistic_v5-v2/MINIAODSIM" > .temp_inputs.dat
cmsRun xsec_cfg.py inputFileList=inputs/xsec/ZZto4L_TuneCP5_13p6TeV_powheg-pythia8.dat 2>&1 | tee out_zz4l.txt

dasgoclient -limit 50 -query "file dataset=/GluGlutoContinto2Zto4E_TuneCP5_13p6TeV_mcfm-pythia8/Run3Summer22MiniAODv4-130X_mcRun3_2022_realistic_v5-v2/MINIAODSIM" > .temp_inputs.dat
cmsRun xsec_cfg.py inputFileList=inputs/xsec/GluGlutoContinto2Zto4E_TuneCP5_13p6TeV_mcfm-pythia8.dat scale=0.00305851 2>&1 | tee out_ggZZ4e.txt

dasgoclient -limit 50 -query "file dataset=/GluGlutoContinto2Zto4Mu_TuneCP5_13p6TeV_mcfm-pythia8/Run3Summer22MiniAODv4-130X_mcRun3_2022_realistic_v5-v2/MINIAODSIM" > .temp_inputs.dat
cmsRun xsec_cfg.py inputFileList=inputs/xsec/GluGlutoContinto2Zto4Mu_Tun3CP5_13p6TeV_mcfm-pythia8.dat scale=0.00305851 2>&1 | tee out_ggZZ4m.txt

dasgoclient -limit 50 -query "file dataset=/GluGlutoContinto2Zto2E2Mu_TuneCP5_13p6TeV_mcfm701-pythia8/Run3Summer22MiniAODv4-130X_mcRun3_2022_realistic_v5-v1/MINIAODSIM" > .temp_inputs.dat
cmsRun xsec_cfg.py inputFileList=inputs/xsec/GluGlutoContinto2Zto2E2Mu_TuneCP5_13p6TeV_mcfm701-pythia8.dat scale=0.00624157 2>&1 | tee out_ggZZ2e2m.txt

rm .temp_inputs.dat
