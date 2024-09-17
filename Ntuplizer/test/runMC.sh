if [[ $# -lt 2 ]]; then
	echo "usage: $0 FILE YEAR [LHE]"
  echo 
  echo "FILE: input file listing input miniAOD ROOT files, 1 per line."
  echo "YEAR: any year in Run 3 for analysis"
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

cmsRun ntuplize_cfg.py inputFileList=$1 year=$2 lheWeights=$lhe channels=zz isMC=1 eCalib=1 muCalib=1 genInfo=1
