#!/bin/bash

echo "WARNING: jetid is known to be out-of-date. At present, newer corrections crash due to correctionlib."

dirmap=(
  2018_UL,Run2-2018-UL-NanoAODv9
  2022_Summer22,Run3-22CDSep23-Summer22-NanoAODv12
  2022_Summer22EE,Run3-22EFGSep23-Summer22EE-NanoAODv12
  2023_Summer23,Run3-23CSep23-Summer23-NanoAODv12
  2023_Summer23BPix,Run3-23DSep23-Summer23BPix-NanoAODv12
  2024_Summer24,Run3-24CDEReprocessingFGHIPrompt-Summer24-NanoAODv15
)

for pog in data/XPOG/*/; do
  for mapping in "${dirmap[@]}"; do
    era=$(cut -d , -f 1 <<< $mapping)
    eradir=$(cut -d , -f 2 <<< $mapping)
    if [[ -d ${pog}/${era} ]]; then
      for f in ${pog}/${era}/*.json.gz; do
        [[ ! -f $f || -L $f ]] && continue

        centralfile="/cvmfs/cms-griddata.cern.ch/cat/metadata/$(basename $pog)/${eradir}/latest/$(basename $f)"
        f=${f/\/\//\/}; centralfile=${centralfile/\/\//\/}
        if [[ ! -f $centralfile ]]; then
          echo "centrail file not found: $centralfile"
          continue
        fi
        if ! diff $f $centralfile >& /dev/null; then
          echo
          echo $f is out-of-date.
          read -p "Update the file? (y/n): " choice
          [[ $choice == [yY] ]] && cp -v $centralfile $f
        fi
      done
    fi
  done
done

if [[ -d ../MuonScaReKIT ]]; then
  pushd ../MuonScaReKIT/ >& /dev/null
  git pull
  popd >& /dev/null

  for f in data/MuonCorrections/*.json; do
    muoncorr="../MuonScaReKIT/corrections/$(basename $f)"
    if ! diff $f $muoncorr >& /dev/null; then
      echo
      echo $f is out-of-date.
      read -p "Update the file? (y/n): " choice
      [[ $choice == [yY] ]] && cp -v $muoncorr $f
    fi
  done
fi
