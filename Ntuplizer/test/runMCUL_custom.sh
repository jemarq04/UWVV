if [[ $# -lt 2 ]]; then
	echo "usage: ./runMCUL.sh FILE YEAR [LHE]"
  echo 
  echo "FILE: input file listing input miniAOD ROOT files, 1 per line."
  echo "YEAR: any year between 2016-2018 for UL analysis"
  echo "LHE: optional argument to provide lheWeights variable to config file. (default: 0)"
	exit 1
fi

if [[ ! -f "${1}" ]]; then
	echo "input file ${1} not found"
	exit 2
fi

lhe=$3
if [[ -z $lhe ]]; then
  lhe=0
elif [[ ! $lhe =~ ^[0-9]+$ ]]; then
  echo "invalid lheWeights value"
  exit 3
fi

if [[ $2 == "2016"* ]]; then
  echo "Running 2016 signal MC"
  cmsRun ntuplize_cfg_UL.py channels=zz isMC=1 eCalib=1 muCalib=1 isSync=0 year=2016 genInfo=1 globalTag=106X_mcRun2_asymptotic_v17 lheWeights=$lhe inputFileList=$1
elif [[ $2 == "2017"* ]]; then
  echo "Running 2017 signal MC"
  cmsRun ntuplize_cfg_UL.py channels=zz isMC=1 eCalib=1 muCalib=1 isSync=0 year=2017 genInfo=1 globalTag=106X_mc2017_realistic_v10 lheWeights=$lhe inputFileList=$1
elif [[ $2 == "2018"* ]]; then
  echo "Running 2018 signal MC"
  cmsRun ntuplize_cfg_UL.py channels=zz isMC=1 eCalib=1 muCalib=1 isSync=0 year=2018 genInfo=1 globalTag=106X_upgrade2018_realistic_v16_L1v1 lheWeights=$lhe inputFileList=$1
else
  echo "invalid year provided: must be 2016-2018"
  exit 4
fi
