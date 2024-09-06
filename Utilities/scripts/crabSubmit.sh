# Modified from N. Smith, U. Wisconsin
# Usage examples:
# . crabSubmit.sh twoLepton-tranche4.txt | grep 'DYJets' | . /dev/stdin
# . crabSubmit.sh twoLepton-data.txt | . /dev/stdin
if [ $# -eq 0 ]; then
    echo "You need to specify a file containing your list of datasets"
    echo "    Usage: . crabSubmit.sh datasetList.txt"
    return 1
fi
scripts_path=$CMSSW_BASE/src/UWVV/Utilities/scripts
config_path=$scripts_path/CrabTemplates
#config=$config_path/local.allweights2016.txt
if [[ $2 == "2016" ]]; then
  config=$config_path/local.allweights2016UL.txt
elif [[ $2 == "2017" ]]; then
  config=$config_path/local.allweights2017UL.txt 
elif [[ $2 == "2018" ]]; then
  config=$config_path/local.allweights2018UL.txt
elif [[ $2 == "2016UL" ]]; then
  config=$config_path/local.allweights2016UL.txt
elif [[ $2 == "2017UL" ]]; then
  config=$config_path/local.allweights2017UL.txt
elif [[ $2 == "2018UL" ]]; then
  config=$config_path/local.allweights2018UL.txt
elif [[ $2 == "ZL2016" ]]; then
  config=$config_path/local.ZL2016.txt
elif [[ $2 == "ZL2017" ]]; then
  config=$config_path/local.ZL2017.txt 
elif [[ $2 == "ZL2018" ]]; then
  config=$config_path/local.ZL2018.txt
elif [[ $2 == *"NoLHEWeights"* ]]; then
  config=$config_path/local.noweights.txt
elif [[ $2 == *"LHEScaleWeights"* ]]; then
  config=$config_path/local.onlyscaleweights.txt
fi 
(>&2 echo "Using config file $config")
cp $config local.txt
grep -v -e '^#' -e '^ *$' $1 | while read dataset
do
  # uncomment to record nevents
  # grep -q "${dataset}" nevents.txt && continue
  # echo $dataset $(das_client.py --format=json --query="dataset dataset=${dataset}" | grep -o "nevents[^,]*," -m 1)
  echo crab submit -c $scripts_path/crab.py Data.inputDataset=$dataset
done
