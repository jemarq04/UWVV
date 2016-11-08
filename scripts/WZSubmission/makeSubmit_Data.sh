#!/bin/bash
DATE=`date +%Y-%m-%d`
echo $DATE
if [ -z "$CMSSW_BASE" ]; then
    echo "Must submit within a CMS environment"
    exit 1
fi

uwvv=$CMSSW_BASE/src/UWVV
${uwvv}/Utilities/scripts/submitJobs.py \
    -o ${uwvv}/submit_scripts/submitWZData_Run2016B-PromptReco-v2_$DATE.sh \
    --dataEra \
        Run2016B-PromptReco-v2 \
    --samples \
        DoubleEG \
        DoubleMuon \
        MuonEG \
    --filesPerJob 2 \
    UWVVNtuples_WZData_Run2016B-PromptReco-v2_$DATE \
    $uwvv/Ntuplizer/test/ntuplize_cfg.py \
        channels=zl \
        isMC=0 \
        eCalib=1 
        muCalib=1 \
        globalTag=80X_dataRun2_Prompt_ICHEP16JEC_v0
uwvv=$CMSSW_BASE/src/UWVV
${uwvv}/Utilities/scripts/submitJobs.py \
    -o ${uwvv}/submit_scripts/submitWZData_Run2016C-PromptReco-v2_$DATE.sh \
    --dataEra \
        Run2016C-PromptReco-v2 \
    --samples \
        DoubleEG \
        DoubleMuon \
        MuonEG \
    --filesPerJob 2 \
    UWVVNtuples_WZData_Run2016C-PromptReco-v2_$DATE \
    $uwvv/Ntuplizer/test/ntuplize_cfg.py \
        channels=zl \
        isMC=0 \
        eCalib=1 
        muCalib=1 \
        globalTag=80X_dataRun2_Prompt_ICHEP16JEC_v0
${uwvv}/Utilities/scripts/submitJobs.py \
    -o ${uwvv}/submit_scripts/submitWZData_Run2016D-PromptReco-v2_$DATE.sh \
    --dataEra \
        Run2016D-PromptReco-v2 \
    --samples \
        DoubleEG \
        DoubleMuon \
        MuonEG \
    --filesPerJob 2 \
    UWVVNtuples_WZData_Run2016D-PromptReco-v2_$DATE \
    $uwvv/Ntuplizer/test/ntuplize_cfg.py \
        channels=zl \
        isMC=0 \
        eCalib=1 
        muCalib=1 \
        globalTag=80X_dataRun2_Prompt_ICHEP16JEC_v0
chmod +x $CMSSW_BASE/src/UWVV/submit_scripts/submitWZData_Run2016B-PromptReco-v2_$DATE.sh
$CMSSW_BASE/src/UWVV/submit_scripts/submitWZData_Run2016B-PromptReco-v2_$DATE.sh
chmod +x $CMSSW_BASE/src/UWVV/submit_scripts/submitWZData_Run2016C-PromptReco-v2_$DATE.sh
$CMSSW_BASE/src/UWVV/submit_scripts/submitWZData_Run2016C-PromptReco-v2_$DATE.sh
chmod +x $CMSSW_BASE/src/UWVV/submit_scripts/submitWZData_Run2016D-PromptReco-v2_$DATE.sh
$CMSSW_BASE/src/UWVV/submit_scripts/submitWZData_Run2016D-PromptReco-v2_$DATE.sh
