#!/bin/bash

#Usage: source recipe/copyDataUL.sh at UWVV
mkdir -p data/jetPUSF
pushd data/jetPUSF/
curl -O https://twiki.cern.ch/twiki/pub/CMS/PileupJetID/effcyPUID_81Xtraining.root
curl -O https://twiki.cern.ch/twiki/pub/CMS/PileupJetID/scalefactorsPUID_81Xtraining.root
popd
pushd data/RochesterCorrections
rm RoccoR2018.txt
curl -O https://twiki.cern.ch/twiki/pub/CMS/RochcorMuon/roccor.Run2.v5.tgz
tar -xf roccor.Run2.v5.tgz
rm roccor.Run2.v5.tgz

#Checked that these files remain the same as in UWVV repository, or can also copy them to update
rm RoccoR.h
rm RoccoR.cc
popd