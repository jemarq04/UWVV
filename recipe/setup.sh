#!/usr/bin/bash

if [[ -z $CMSSW_BASE ]]; then
  echo "CMSSW environment not set. Don't forget to run cmsenv!"
  exit 1
fi

pushd $CMSSW_BASE/src

#https://twiki.cern.ch/twiki/bin/view/CMS/EgammaUL2016To2018#Recipe_for_running_scales_and_sm
git cms-addpkg RecoEgamma/EgammaTools  ### essentially just checkout the package from CMSSW
git clone https://github.com/cms-egamma/EgammaPostRecoTools.git
cd EgammaPostRecoTools
git reset --hard 209673a # Needed for Run 2 analysis
cd ..
mv EgammaPostRecoTools/python/EgammaPostRecoTools.py RecoEgamma/EgammaTools/python/.
git clone -b ULSSfiles_correctScaleSysMC https://github.com/jainshilpi/EgammaAnalysis-ElectronTools.git EgammaAnalysis/ElectronTools/data/
git cms-addpkg EgammaAnalysis/ElectronTools

# For now all that we copy over are the text files that contain the corrections as the .h and .cc files are already in UWVV
# If they undergo significant changes, we should use the second commented command instead.
# (For now they are slight organizational changes, so we omit them. Any significant changes should be committed.)
git clone --recursive ssh://git@gitlab.cern.ch:7999/akhukhun/roccor.git
[[ -d roccor ]] && mv roccor/RoccoR*.txt UWVV/data/RochesterCorrections/ || echo "ERROR: error cloning roccor gitlab repo"
#[[ -d roccor ]] && mv roccor/RoccoR*.* UWVV/data/RochesterCorrections/ || echo "ERROR: error cloning roccor gitlab repo"

scram b -j 12
popd
