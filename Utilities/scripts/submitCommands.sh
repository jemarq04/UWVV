source /cvmfs/cms.cern.ch/crab3/crab.sh
source /cvmfs/cms.cern.ch/common/crab-setup.sh
##Advised not to run them all at once. Test locally with a sample file from one dataset in each .dat file before submitting all the datasets

##2016 ZZ Ntuples
./crabSubmit.sh /data/uhussain/ZZTo4l/FullRun2/uwvv/src/UWVV/MetaData/ZZDatasets/ZZ2016_SignalMonteCarlo.dat 2016 | grep "ZZ" | . /dev/stdin

./crabSubmit.sh /data/uhussain/ZZTo4l/FullRun2/uwvv/src/UWVV/MetaData/ZZDatasets/ZZ2016Data_MiniAOD.dat 2016 | grep "ZZ" | . /dev/stdin

./crabSubmit.sh /data/uhussain/ZZTo4l/FullRun2/uwvv/src/UWVV/MetaData/ZZDatasets/ZZ2016_BackgroundMonteCarlo.dat 2016 | grep "ZZ" | . /dev/stdin

##2017 ZZ Ntuples
./crabSubmit.sh /data/uhussain/ZZTo4l/FullRun2/uwvv/src/UWVV/MetaData/ZZDatasets/ZZ2017Data_MiniAOD.dat 2017 | grep "ZZ" | . /dev/stdin

./crabSubmit.sh /data/uhussain/ZZTo4l/FullRun2/uwvv/src/UWVV/MetaData/ZZDatasets/ZZ2017_BackgroundMonteCarlo.dat 2017 | grep "ZZ" | . /dev/stdin

./crabSubmit.sh /data/uhussain/ZZTo4l/FullRun2/uwvv/src/UWVV/MetaData/ZZDatasets/ZZ2017_SignalMonteCarlo.dat 2017 | grep "ZZ" | . /dev/stdin

##ZL Ntuples

./crabSubmit.sh /data/uhussain/ZZTo4l/FullRun2/uwvv/src/UWVV/MetaData/ZZDatasets/ZL2016.dat ZL2016 | grep "ZL" | . /dev/stdin

./crabSubmit.sh /data/uhussain/ZZTo4l/FullRun2/uwvv/src/UWVV/MetaData/ZZDatasets/ZL2017.dat ZL2017 | grep "ZL" | . /dev/stdin


