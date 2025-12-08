#!/usr/bin/env bash

# Modified from N. Smith, U. Wisconsin
# Usage examples:
#  crabSubmit.sh datasets/2022all.dat | grep 'EGamma' | . /dev/stdin
#  crabSubmit.sh datasets/2022MC_qqZZ.dat | . /dev/stdin

settingsFile=local.cfg
scripts_path=$CMSSW_BASE/src/UWVV/Utilities/scripts
if [ $# -eq 0 ]; then
  (>&2 echo "usage: ${0##*/} datasetList.dat [config]")
  (>&2 echo "")
  (>&2 echo "datasetList.dat: A file containing a list of datasets to process")
  (>&2 echo "[config]: optional identifier for section within '${settingsFile}' file with the appropriate settings")
  (>&2 echo "  if 'auto' is given, the script attempts to determine the section name from")
  (>&2 echo "  the first four characters of the dataset list filename. (e.g. 2022MC.dat -> 2022)")
  exit 1
fi

if [[ ! -f ${settingsFile} ]]; then
  (>&2 echo "error: config file '${settingsFile}' not found")
  exit 1
fi

(>&2 echo "Using config file ${settingsFile}")
if [[ ! -z $2 ]]; then
  if [[ $2 = auto ]]; then
    name=$(basename $1)
    name=${name:0:4}
    if [[ ! -z $(grep "^s*\[${name}\]\s*$" ${settingsFile}) ]]; then
      (>&2 echo "Using setup auto -> ${name}")
      sed -i -e "s|^setup: .*|setup: ${name}|" ${settingsFile}
    else
      (>&2 echo "Setup ${name} not found. Using setup found in ${settingsFile}")
    fi
  elif [[ ! -z $(grep "^\s*\[$2\]\s*$" ${settingsFile}) ]]; then
    (>&2 echo "Using setup $2")
    sed -i -e "s|^setup: .*|setup: $2|" ${settingsFile}
  else
    (>&2 echo "Section name $2 not found in ${settingsFile}")
    exit 1
  fi
fi

grep -v -e '^#' -e '^ *$' $1 | while read dataset; do
  # uncomment to record nevents
  # grep -q "${dataset}" nevents.txt && continue
  # echo $dataset $(das_client.py --format=json --query="dataset dataset=${dataset}" | grep -o "nevents[^,]*," -m 1)
  echo crab submit -c $scripts_path/crab.py Data.inputDataset=$dataset
done
