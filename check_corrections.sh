#!/bin/bash

if [[ -z $CMSSW_BASE ]]; then
  echo "CMSSW environment not set. Don't forget to run cmsenv!"
  exit 1
fi

eras=(
  Run3-22CDSep23-Summer22-NanoAODv12
  Run3-22EFGSep23-Summer22EE-NanoAODv12
  Run3-23CSep23-Summer23-NanoAODv12
  Run3-23DSep23-Summer23BPix-NanoAODv12
  Run3-24CDEReprocessingFGHIPrompt-Summer24-NanoAODv15
  Run3-25Prompt-Winter25-NanoAODv15
  Run3-25Prompt-Summer24-NanoAODv15
)

POGs=(
  EGM
  MUO
  JME
)

basepath="/cvmfs/cms-griddata.cern.ch/cat/metadata"
outdir="$CMSSW_BASE/src/UWVV/data/XPOG"
for pog in "${POGs[@]}"; do
  for era in "${eras[@]}"; do
    if [[ ! -d $outdir/$pog/$era ]]; then
      [[ ! -d $basepath/$pog/$era ]] && continue
      mkdir -p $outdir/$pog/$era
      cp -vr $basepath/$pog/$era/latest $outdir/$pog/$era
      continue
    fi
    for f in $outdir/$pog/$era/latest/*.json.gz; do
      [[ ! -f $f ]] && continue
      centralfile="$basepath/$pog/$era/latest/$(basename $f)"
      if [[ ! -f $centralfile ]]; then
        echo "central file not found: $centralfile"
        continue
      fi
      if ! diff $f $centralfile >& /dev/null; then
        echo
        echo ${f/$outdir\//} is out-of-date.
        read -p "Update the file? (y/n): " choice
        [[ $choice == [yY] ]] && cp -v $centralfile $f
      fi
    done
  done
done
