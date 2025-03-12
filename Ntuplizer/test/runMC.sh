if [[ $# -lt 2 ]]; then
	echo "usage: $0 FILE YEAR [EXTRA...]"
  echo 
  echo "FILE: input file listing input miniAOD ROOT files, 1 per line."
  echo "YEAR: any year in Run 3 for analysis"
  echo "EXTRA: optional argument(s) passed directly to cmsRun"
	exit 1
fi

infile=$1
year=$2
shift 2

if [[ ! -f $infile ]]; then
	echo "input file $infile not found"
	exit 2
fi

cmsRun ntuplize_cfg.py inputFileList=$infile year=$year channels=zz isMC=1 eCalib=1 muCalib=1 genInfo=1 $@
