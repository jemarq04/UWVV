#!/usr/bin/env bash

for year in 2022 2023 2024; do
  echo $year
  for f in inputs/testing/${year}*.dat; do
    [[ $f = *Data* ]] && isMC=0 || isMC=1
    extra=""
    [[ $year = 2022 && $isMC ]] && extra="dataPeriod=C"
    cmsRun ntuplize_cfg.py inputFileList=$f year=$year channels=zz isMC=$isMC eCalib=1 muCalib=1 genInfo=$isMC maxEvents=100 outputFile=testing.root $extra
    #cmsRun ntuplize_cfg.py inputFileList=$f year=$year channels=zl isMC=$isMC eCalib=1 muCalib=1 genInfo=$isMC maxEvents=100 outputFile=testing.root $extra
    break
  done
  break
done

rm -f testing*.root
