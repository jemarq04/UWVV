#!/bin/bash

#---------Usage: source recipe/copyDataUL.sh at UWVV
#---------These data files need authorization for downloading, so probably want to do them manually


#mkdir -p data/jetPUSF
#pushd data/jetPUSF/
#curl -O https://twiki.cern.ch/twiki/pub/CMS/PileupJetID/effcyPUID_81Xtraining.root #doesn't download the actual file
#curl -O https://twiki.cern.ch/twiki/pub/CMS/PileupJetID/scalefactorsPUID_81Xtraining.root #doesn't download the actual file
#popd

#pushd data/RochesterCorrections
#rm RoccoR2018.txt
#curl -O https://twiki.cern.ch/twiki/pub/CMS/RochcorMuon/roccor.Run2.v5.tgz #doesn't download the actual file
#tar -xf roccor.Run2.v5.tgz
#mv roccor/* .
#rmdir roccor/
#rm roccor.Run2.v5.tgz

#-----------Checked that these files remain the same as in UWVV repository, or can also copy them to update
#rm RoccoR.h
#rm RoccoR.cc
#popd