#!/bin/bash

echo "WARNING: jetid is known to be out-of-date. At present, newer corrections crash due to correctionlib."

for pog in data/XPOG/*/; do
  for era in ${pog}*/; do
    for f in ${era}*.json.gz; do
      [[ -L $f ]] && continue

      centralfile="/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/${f#data\/XPOG\/}"
      if ! diff $f $centralfile >& /dev/null; then
        echo
        echo $f is out-of-date.
        read -p "Update the file? (y/n): " choice
        [[ $choice == [yY] ]] && cp -v $centralfile $f
      fi
    done
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
