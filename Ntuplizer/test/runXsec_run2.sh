#!/usr/bin/env bash

maxFiles=50
maxEvents=100000

dasgoclient -limit $maxFiles -query "file dataset=/ZZTo4L_13TeV_powheg_pythia8/RunIISummer16MiniAODv2-PUMoriond17_80X_mcRun2_asymptotic_2016_TrancheIV_v6-v1/MINIAODSIM" > .temp_inputs.dat
cmsRun xsec_cfg.py inputFileList=.temp_inputs.dat scale=1.256 maxEvents=$maxEvents 2>&1 | tee out_zz4l_run2.txt

dasgoclient -limit $maxFiles -query "file dataset=/GluGluToContinToZZTo4e_13TeV_MCFM701_pythia8/RunIISummer16MiniAODv2-PUMoriond17_80X_mcRun2_asymptotic_2016_TrancheIV_v6-v1/MINIAODSIM" > .temp_inputs.dat
#dasgoclient -limit $maxFiles -query "file dataset=/GluGluToContinToZZTo4e_13TeV_MCFM701_pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15-v2/MINIAODSIM" > .temp_inputs.dat
cmsRun xsec_cfg.py inputFileList=.temp_inputs.dat scale=0.00159 maxEvents=$maxEvents 2>&1 | tee out_ggZZ4e_run2.txt

dasgoclient -limit $maxFiles -query "file dataset=/GluGluToContinToZZTo4mu_13TeV_MCFM701_pythia8/RunIISummer16MiniAODv2-PUMoriond17_80X_mcRun2_asymptotic_2016_TrancheIV_v6-v1/MINIAODSIM" > .temp_inputs.dat
cmsRun xsec_cfg.py inputFileList=.temp_inputs.dat scale=0.00270241 maxEvents=$maxEvents 2>&1 | tee out_ggZZ4m_run2.txt

dasgoclient -limit $maxFiles -query "file dataset=/GluGluToContinToZZTo2e2mu_13TeV_MCFM701_pythia8/RunIISummer16MiniAODv2-PUMoriond17_80X_mcRun2_asymptotic_2016_TrancheIV_v6-v1/MINIAODSIM" > .temp_inputs.dat
#dasgoclient -limit $maxFiles -query "file dataset=/GluGluToContinToZZTo2e2mu_13TeV_MCFM701_pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15-v3/MINIAODSIM" > .temp_inputs.dat
cmsRun xsec_cfg.py inputFileList=.temp_inputs.dat scale=0.00319 maxEvents=$maxEvents 2>&1 | tee out_ggZZ2e2m_run2.txt

rm .temp_inputs.dat
