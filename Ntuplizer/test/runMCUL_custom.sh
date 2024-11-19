if [[ $# -lt 2 ]]; then
  echo "usage: $0 FILE YEAR [EXTRA...]"
  echo 
  echo "FILE: input file listing input miniAOD ROOT files, 1 per line."
  echo "YEAR: any year in Run 2 for UL analysis"
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

echo "Running $year signal MC"
cmsRun ntuplize_cfg_UL.py inputFileList=$infile year=$year channels=zz isMC=1 eCalib=1 muCalib=1 isSync=0 genInfo=1 $@
