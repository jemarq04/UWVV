#!/usr/bin/env bash

for year in 2022 2023 2024 2025; do
  for f in inputs/testing/${year}*.dat; do
    [[ ! -f $f ]] && break
    echo $f
    [[ $f = *Data* ]] && isMC=0 || isMC=1
    extra=""
    [[ $year = 2022 && $isMC ]] && extra="$extra dataPeriod=C"
    [[ $year = 2022 && $f = *postEE* ]] && extra="$extra postEE=1"
    [[ $year = 2023 && $f = *postBPix* ]] && extra="$extra postBPix=1"
    [[ $year = 2024 ]] && extra="$extra electronsUL=0"
    [[ $year = 2025 && $isMC -eq 0 ]] && extra="$extra isPrompt=1"
    [[ $year = 2025 ]] && extra="$extra electronsUL=0 eCalib=0 muCalib=0"
    cmsRun ntuplize_cfg.py inputFileList=$f year=$year channels=zz isMC=$isMC genInfo=$isMC maxEvents=100 outputFile=testing.root $extra
    #cmsRun ntuplize_cfg.py inputFileList=$f year=$year channels=zl isMC=$isMC eCalib=1 muCalib=1 genInfo=$isMC maxEvents=100 outputFile=testing.root $extra
  done
done

rm -f testing*.root
