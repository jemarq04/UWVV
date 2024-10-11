# Modified from N. Smith, U. Wisconsin
# Usage examples:
# . crabSubmit.sh twoLepton-tranche4.txt | grep 'DYJets' | . /dev/stdin
# . crabSubmit.sh twoLepton-data.txt | . /dev/stdin
if [ $# -eq 0 ]; then
    echo "You need to specify a file containing your list of datasets"
    echo "You can also specify a year so that a template can be copied to local.cfg"
    echo "    Usage: ${0##*/} datasetList.txt [year]"
    exit 1
fi
scripts_path=$CMSSW_BASE/src/UWVV/Utilities/scripts
config_path=$CMSSW_BASE/src/UWVV/Utilities/test/CrabTemplates
if [[ $2 == "2022" ]]; then
  config=$config_path/local.allweights2022UL.cfg
elif [[ $2 == *"NoLHEWeights"* ]]; then
  config=$config_path/local.noweights.cfg
elif [[ $2 == *"LHEScaleWeights"* ]]; then
  config=$config_path/local.onlyscaleweights.cfg
elif [[ ! -z $2 ]]; then
  echo "Template not found for $2"
  exit 1
fi
if [[ ! -z $config ]]; then
  (>&2 echo "Using config file $config")
  cp $config local.cfg
elif [[ ! -f local.cfg ]]; then
  echo "If you don't specify a year for the template, you must"
  echo " already have a 'local.cfg' config file!"
  exit 1
else (>&2 echo "Using config file local.cfg")
fi
grep -v -e '^#' -e '^ *$' $1 | while read dataset
do
  # uncomment to record nevents
  # grep -q "${dataset}" nevents.txt && continue
  # echo $dataset $(das_client.py --format=json --query="dataset dataset=${dataset}" | grep -o "nevents[^,]*," -m 1)
  echo crab submit -c $scripts_path/crab.py Data.inputDataset=$dataset
done
