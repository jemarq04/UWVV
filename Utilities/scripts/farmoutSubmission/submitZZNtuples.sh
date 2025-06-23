#!/bin/bash

### Little script to submit all necessary ntuplization jobs for the 2017 ZZ analysis
###     Specify an identifier for the jobs (e.g. 18JAN2017_0) and, optionally,
###     which types of ntuples to submit. Options are --data, --mc, --zz, --zl, and --z,
###     or a combination, for running data, monte carlo, 4l final states, 3l final states,
###     and 2l final states; e.g., --data --zz --zl would run only 4l and 3l final states
###     for data, no 2l final states and no monte carlo. If none of those are specified,
###     all are run


if [ "$1" == "" ] || [ "$1" == "-h" ] || [ "$1" == "--help" ]
then
    echo "$0 usage: ./$0 [-h|--help] id [--data] [--mc] [--zz] [--zl] [--z] [--no-submit]"
    echo "    id: string identifying this set of ntuples, e.g. a date."
    echo "    --[data/mc]: run only on data/mc."
    echo "    --[zz/zl/z]: run only 4l/3l/2l ntuples. More than one may be specified."
    echo "    --syst: (with --zz and --mc) run signal MC several more times with various systematic shifts."
    echo "    --no-submit: Set up the submit directories and write the submit scripts, but do not actually submit the jobs."
    echo "    --aTGC: run the aTGC samples too."
    exit 1
fi

JOBID="$1"
shift

while [ "$1" ]
do
    case "$1" in
        --data)
            DATA=1
            ;;
        --mc)
            MC=1
            ;;
        --zz)
            ZZ=1
            ;;
        --zl)
            ZL=1
            ;;
        --z)
            Z=1
            ;;
        --syst)
            SYST=1
            ;;
        --no-submit)
            NO_SUB=1
            ;;
        --aTGC)
            ATGC=1
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac

    shift
done

if [ "$DATA" == '' ] && [ "$MC" == '' ]
then
    DATA=1
    MC=1
fi

if [ "$ZZ" == '' ] && [ "$ZL" == '' ] && [ "$Z" == '' ]
then
    ZZ=1
    ZL=1
    Z=1
fi

if [ ! -d "$CMSSW_BASE"/src/UWVV ]
then
    echo "You can only submit ntuples from a CMSSW environment with UWVV set up!"
    exit 1
fi

pushd "$CMSSW_BASE"/src/UWVV

# Check for a valid voms proxy (not a perfect check; just makes sure there's
# regular output from voms-proxy-info but no error output.
VOMS_OUT=$( voms-proxy-info 2> /dev/null )
VOMS_ERR=$( voms-proxy-info 2>&1 > /dev/null )
if [ ! "$VOMS_OUT" ] || [ "$VOMS_ERR" ]
then
    echo 'Something is wrong with your VOMS proxy -- please renew it or check it'
    exit 1
fi

# global tags
GT_MC=94X_mc2017_realistic_v13
GT_DATA=94X_dataRun2_v6
#GT_DATA_F=94X_dataRun2_ReReco_EOY17_v2

if [ "$ATGC" ]
then
    EXTRA_SIGNALS='ZZ*aTGC* ZZTo4L_13TeV-sherpa'
else
    EXTRA_SIGNALS=""
fi


if [ "$DATA" ]
then

    for era in B C D E
    do

        if [ "$ZZ" ]
        then
            echo Creating submission infrastructure for ZZ Data Ntuples as UWVVZZ_DATA_2017"$era"_"$JOBID"

            python Utilities/scripts/submitJobs.py --applyLumiMask --dataEra "Run2017""$era""-17Nov2017-v1" --samples MuonEG SingleElectron SingleMuon DoubleMuon DoubleEG --filesPerJob 3 -o /data/uhussain/uwvvZZ_data_2017"$era"_"$JOBID".sh UWVVZZ_DATA_2017"$era"_"$JOBID" Ntuplizer/test/ntuplize_cfg.py channels='zz' isMC=0 eCalib=1 muCalib=1 globalTag="$GT_DATA"

            if [ "$NO_SUB" == '' ]
            then
                echo Submitting ZZ Data Ntuples as UWVVZZ_DATA_2017"$era"_"$JOBID"
                bash /data/uhussain/uwvvZZ_data_2017"$era"_"$JOBID".sh
            fi
        fi

        if [ "$ZL" ]
        then
            echo Creating submission infrastructure for Z+L Data Ntuples as UWVVZPLUSL_DATA_2017"$era"_"$JOBID"

            python Utilities/scripts/submitJobs.py --applyLumiMask --dataEra "Run2017""$era""-17Nov2017-v1" --samples MuonEG SingleElectron SingleMuon DoubleMuon DoubleEG --filesPerJob 3 -o /data/uhussain/uwvvZPlusL_data_2017"$era"_"$JOBID".sh UWVVZPLUSL_DATA_2017"$era"_"$JOBID" Ntuplizer/test/ntuplize_cfg.py channels='zl' isMC=0 eCalib=1 muCalib=1 globalTag="$GT_DATA"

            if [ "$NO_SUB" == '' ]
            then
                echo Submitting Z+L Data Ntuples as UWVVZPLUSL_DATA_2017"$era"_"$JOBID"
                nohup bash /data/uhussain/uwvvZPlusL_data_2017"$era"_"$JOBID".sh &
            fi
        fi

        if [ "$Z" ]
        then
            echo Creating submission infrastructure for Single Z Data Ntuples as UWVVSINGLEZ_DATA_2017"$era"_"$JOBID"

            python Utilities/scripts/submitJobs.py --applyLumiMask --dataEra "Run2017""$era""-17Nov2017-v1" --samples SingleElectron SingleMuon DoubleMuon DoubleEG --filesPerJob 3 -o /data/uhussain/uwvvSingleZ_data_2017"$era"_"$JOBID".sh UWVVSINGLEZ_DATA_2017"$era"_"$JOBID" Ntuplizer/test/ntuplize_cfg.py channels='z' isMC=0 eCalib=1 muCalib=1 globalTag="$GT_DATA"

            if [ "$NO_SUB" == '' ]
            then
                echo Submitting Single Z Data Ntuples as UWVVSINGLEZ_DATA_2017"$era"_"$JOBID"
                nohup bash /data/uhussain/uwvvSingleZ_data_2017"$era"_"$JOBID".sh &
            fi
        fi
    done

    if [ "$ZZ" ]
    then
        echo Creating submission infrastructure for ZZ Data Ntuples as UWVVZZ_DATA_2017F_"$JOBID"

        python Utilities/scripts/submitJobs.py --applyLumiMask --dataEra "Run2017F-17Nov2017-v1" --samples MuonEG SingleElectron SingleMuon DoubleMuon DoubleEG --filesPerJob 2 -o /data/uhussain/uwvvZZ_data_2017F_"$JOBID".sh UWVVZZ_DATA_2017F_"$JOBID" Ntuplizer/test/ntuplize_cfg.py channels='zz' isMC=0 eCalib=1 muCalib=1 globalTag="$GT_DATA"

        if [ "$NO_SUB" == '' ]
        then
            echo Submitting ZZ Data Ntuples as UWVVZZ_DATA_2017F_"$JOBID"
            bash /data/uhussain/uwvvZZ_data_2017F_"$JOBID".sh
        fi
    fi

    if [ "$ZL" ]
    then
        echo Creating submission infrastructure for Z+L Data Ntuples as UWVVZPLUSL_DATA_2017F_"$JOBID"

        python Utilities/scripts/submitJobs.py --applyLumiMask --dataEra "Run2017F-17Nov2017-v1" --samples MuonEG SingleElectron SingleMuon DoubleMuon DoubleEG --filesPerJob 2 -o /data/uhussain/uwvvZPlusL_data_2017F_"$JOBID".sh UWVVZPLUSL_DATA_2017F_"$JOBID" Ntuplizer/test/ntuplize_cfg.py channels='zl' isMC=0 eCalib=1 muCalib=1 globalTag="$GT_DATA"

        if [ "$NO_SUB" == '' ]
        then
            echo Submitting Z+L Data Ntuples as UWVVZPLUSL_DATA_2017F_"$JOBID"
            nohup bash /data/uhussain/uwvvZPlusL_data_2017F_"$JOBID".sh &
        fi
    fi

    if [ "$Z" ]
    then
        echo Creating submission infrastructure for Single Z Data Ntuples as UWVVSINGLEZ_DATA_2017F_"$JOBID"

        python Utilities/scripts/submitJobs.py --applyLumiMask --dataEra "Run2017F-17Nov2017-v1" --samples SingleElectron SingleMuon DoubleMuon DoubleEG --filesPerJob 2 -o /data/uhussain/uwvvSingleZ_data_2017F_"$JOBID".sh UWVVSINGLEZ_DATA_2017F_"$JOBID" Ntuplizer/test/ntuplize_cfg.py channels='z' isMC=0 eCalib=1 muCalib=1 globalTag="$GT_DATA"

        if [ "$NO_SUB" == '' ]
        then
            echo Submitting Single Z Data Ntuples as UWVVSINGLEZ_DATA_2017F_"$JOBID"
            nohup bash /data/uhussain/uwvvSingleZ_data_2017F_"$JOBID".sh &
        fi
    fi
fi

if [ "$MC" ]
then

    if [ "$ZZ" ]
    then
        echo Creating submission infrastructure for ZZ MC Ntuples as UWVVZZ_MC_"$JOBID"

        # signal (include gen info -- except MCFM and Phantom samples, where LHE info is automatically excluded)
        # No genInfo for now. genInfo==0
        python Utilities/scripts/submitJobs.py --campaign 'RunIIFall17MiniAOD-94X_mc2017_realistic_*v1' --samples 'GluGluHToZZTo4L_M125_13TeV_powheg*' 'VBF_HToZZTo4L_M125_13TeV_powheg2*' 'WplusH_HToZZTo4L_M125_13TeV_powheg2-*' 'WminusH_HToZZTo4L_M125_13TeV_powheg2-*' 'ZH_HToZZ_4LFilter_M125_13TeV_powheg2-*' 'ttH_HToZZ_4LFilter_M125_13TeV_powheg2_*' "$EXTRA_SIGNALS" --filesPerJob 2 -o /data/uhussain/uwvvZZ_mc_"$JOBID".sh UWVVZZ_MC_"$JOBID" Ntuplizer/test/ntuplize_cfg.py channels='zz' isMC=1 eCalib=1 muCalib=1 genInfo=0 globalTag="$GT_MC" genLeptonType=dressedHPFS lheWeights=2

        if [ "$NO_SUB" == '' ]
        then
            echo Submitting ZZ MC Ntuples as UWVVZZ_MC_"$JOBID"
            nohup bash /data/uhussain/uwvvZZ_mc_"$JOBID".sh &
        fi

        # background with 2017v1 samples (no gen info)
        python Utilities/scripts/submitJobs.py --campaign 'RunIIFall17MiniAOD-94X_mc2017_realistic_*v1' --samples 'DYJetsToLL_M-50_TuneCP5_13TeV*' 'ZZTo4L_13TeV*pythia8' 'GluGluToContinToZZTo4e*' 'GluGluToContinToZZTo4mu*' 'GluGluToContinToZZTo4tau*' 'GluGluToContinToZZTo2e2tau*' 'GluGluToContinToZZTo2e2mu*' 'GluGluToContinToZZTo2mu2tau*' 'TTJets_TuneCP5_13TeV*' 'TTTo2L2Nu_TuneCP5_PSweights_13TeV*'  --filesPerJob 2 -o /data/uhussain/uwvvZZ_mcV1NoGen_"$JOBID".sh UWVVZZ_MC_"$JOBID" Ntuplizer/test/ntuplize_cfg.py channels='zz' isMC=1 eCalib=1 muCalib=1 genInfo=0 globalTag="$GT_MC" genLeptonType=dressedHPFS

        if [ "$NO_SUB" == '' ]
        then
            nohup bash /data/uhussain/uwvvZZ_mcV1NoGen_"$JOBID".sh &
        fi

        # background with 2017v1 samples (no gen info)
        python Utilities/scripts/submitJobs.py --campaign 'RunIIFall17MiniAOD-94X_mc2017_realistic_*v2' --samples 'WZTo3LNu_TuneCP5_13TeV-*' 'DYJetsToLL_M-10to50_TuneCP5_13TeV*' 'TTTo2L2Nu_TuneCP5_*' 'VBF_HToZZTo4L_M125_13TeV_powheg2*' 'ZZTo4L_13TeV*pythia8' 'GluGluToContinToZZTo2e2tau*' 'GluGluToContinToZZTo2e2mu*' 'GluGluToContinToZZTo2mu2tau*' --filesPerJob 2 -o /data/uhussain/uwvvZZ_mcV2NoGen_"$JOBID".sh UWVVZZ_MC_"$JOBID" Ntuplizer/test/ntuplize_cfg.py channels='zz' isMC=1 eCalib=1 muCalib=1 genInfo=0 globalTag="$GT_MC" genLeptonType=dressedHPFS


        if [ "$NO_SUB" == '' ]
        then
            nohup bash /data/uhussain/uwvvZZ_mcV2NoGen_"$JOBID".sh &
        fi
        # background with 2017v2 (no gen info)
        #python Utilities/scripts/submitJobs.py --campaign 'RunIIFall17MiniAOD-94X_mc2017_realistic_*v2' --samples 'WZTo3LNu_TuneCP5_13TeV-*' 'DYJetsToLL_M-10to50_TuneCP5_13TeV*' 'TTTo2L2Nu_TuneCP5_*' --filesPerJob 2 -o /data/uhussain/uwvvZZ_mcV2NoGen_"$JOBID".sh UWVVZZ_MC_"$JOBID" Ntuplizer/test/ntuplize_cfg.py channels='zz' isMC=1 eCalib=1 muCalib=1 genInfo=0 globalTag="$GT_MC" genLeptonType=dressedHPFS

        #if [ "$NO_SUB" == '' ]
        #then
        #    nohup bash /data/uhussain/uwvvZZ_mcV2NoGen_"$JOBID".sh &
        #fi


        # with systematic shifts
        if [ "$SYST" ]
        then
            echo Creating submission infrastructure for ZZ MC Ntuples with Systematic Shifts

            # EES+
            python Utilities/scripts/submitJobs.py --campaign 'RunIIFall17MiniAOD-94X_mc2017_realistic_*v1' --samples 'ZZTo4L_13TeV*pythia8' 'GluGluHToZZTo4L_M125_13TeV_powheg*' 'GluGlu*ZZ*DefaultShower*' 'GluGluToContinToZZTo2e2tau*' 'GluGluToContinToZZTo2mu2tau*' 'ZZJJTo4L*' 'WZZ*' 'ZZZ*' 'WWZ*' 'TTZToLLNuNu_M-10_TuneCUETP8M1_13TeV-amcatnlo-pythia8' 'ZZTo4L_*Jets_ZZOnShell_13TeV-amcatnloFXFX-madspin-pythia8' 'VBFToHiggs0PMContinToZZTo*' "$EXTRA_SIGNALS" --filesPerJob 2 -o /data/uhussain/uwvvZZ_mc_eScaleUp_"$JOBID".sh UWVVZZ_MC_ESCALEUP_"$JOBID" Ntuplizer/test/ntuplize_cfg.py channels='eeee,eemm' isMC=1 eCalib=1 muCalib=1 lheWeights=0 eScaleShift=1 globalTag="$GT_MC" genLeptonType=dressedHPFS

            if [ "$NO_SUB" == '' ]
            then
                echo Submitting ZZ MC Ntuples with Systematic Shifts
                nohup bash /data/uhussain/uwvvZZ_mc_eScaleUp_"$JOBID".sh &
            fi

            # EES-
            python Utilities/scripts/submitJobs.py --campaign 'RunIIFall17MiniAOD-94X_mc2017_realistic_*v1' --samples 'ZZTo4L_13TeV*pythia8' 'GluGluHToZZTo4L_M125_13TeV_powheg*' 'GluGlu*ZZ*DefaultShower*' 'GluGluToContinToZZTo2e2tau*' 'GluGluToContinToZZTo2mu2tau*' 'ZZJJTo4L*' 'WZZ*' 'ZZZ*' 'WWZ*' 'TTZToLLNuNu_M-10_TuneCUETP8M1_13TeV-amcatnlo-pythia8' 'ZZTo4L_*Jets_ZZOnShell_13TeV-amcatnloFXFX-madspin-pythia8' 'VBFToHiggs0PMContinToZZTo*' "$EXTRA_SIGNALS" --filesPerJob 2 -o /data/uhussain/uwvvZZ_mc_eScaleDn_"$JOBID".sh UWVVZZ_MC_ESCALEDN_"$JOBID" Ntuplizer/test/ntuplize_cfg.py channels='eeee,eemm' isMC=1 eCalib=1 muCalib=1 lheWeights=0 eScaleShift=-1 globalTag="$GT_MC" genLeptonType=dressedHPFS

            if [ "$NO_SUB" == '' ]
            then
                nohup bash /data/uhussain/uwvvZZ_mc_eScaleDn_"$JOBID".sh &
            fi

            # EER+ (rho)
            python Utilities/scripts/submitJobs.py --campaign 'RunIIFall17MiniAOD-94X_mc2017_realistic_*v1' --samples 'ZZTo4L_13TeV*pythia8' 'GluGluHToZZTo4L_M125_13TeV_powheg*' 'GluGlu*ZZ*DefaultShower*' 'GluGluToContinToZZTo2e2tau*' 'GluGluToContinToZZTo2mu2tau*' 'ZZJJTo4L*' 'WZZ*' 'ZZZ*' 'WWZ*' 'TTZToLLNuNu_M-10_TuneCUETP8M1_13TeV-amcatnlo-pythia8' 'ZZTo4L_*Jets_ZZOnShell_13TeV-amcatnloFXFX-madspin-pythia8' 'VBFToHiggs0PMContinToZZTo*' "$EXTRA_SIGNALS" --filesPerJob 2 -o /data/uhussain/uwvvZZ_mc_eRhoResUp_"$JOBID".sh UWVVZZ_MC_ERHORESUP_"$JOBID" Ntuplizer/test/ntuplize_cfg.py channels='eeee,eemm' isMC=1 eCalib=1 muCalib=1 lheWeights=0 eRhoResShift=1 globalTag="$GT_MC" genLeptonType=dressedHPFS

            if [ "$NO_SUB" == '' ]
            then
                nohup bash /data/uhussain/uwvvZZ_mc_eRhoResUp_"$JOBID".sh &
            fi

            # EER- (rho)
            python Utilities/scripts/submitJobs.py --campaign 'RunIIFall17MiniAOD-94X_mc2017_realistic_*v1' --samples 'ZZTo4L_13TeV*pythia8' 'GluGluHToZZTo4L_M125_13TeV_powheg*' 'GluGlu*ZZ*DefaultShower*' 'GluGluToContinToZZTo2e2tau*' 'GluGluToContinToZZTo2mu2tau*' 'ZZJJTo4L*' 'WZZ*' 'ZZZ*' 'WWZ*' 'TTZToLLNuNu_M-10_TuneCUETP8M1_13TeV-amcatnlo-pythia8' 'ZZTo4L_*Jets_ZZOnShell_13TeV-amcatnloFXFX-madspin-pythia8' 'VBFToHiggs0PMContinToZZTo*' "$EXTRA_SIGNALS" --filesPerJob 2 -o /data/uhussain/uwvvZZ_mc_eRhoResDn_"$JOBID".sh UWVVZZ_MC_ERHORESDN_"$JOBID" Ntuplizer/test/ntuplize_cfg.py channels='eeee,eemm' isMC=1 eCalib=1 muCalib=1 lheWeights=0 eRhoResShift=-1 globalTag="$GT_MC" genLeptonType=dressedHPFS

            if [ "$NO_SUB" == '' ]
            then
                nohup bash /data/uhussain/uwvvZZ_mc_eRhoResDn_"$JOBID".sh &
            fi

            # EER+ (phi)
            python Utilities/scripts/submitJobs.py --campaign 'RunIIFall17MiniAOD-94X_mc2017_realistic_*v1' --samples 'ZZTo4L_13TeV*pythia8' 'GluGluHToZZTo4L_M125_13TeV_powheg*' 'GluGlu*ZZ*DefaultShower*' 'GluGluToContinToZZTo2e2tau*' 'GluGluToContinToZZTo2mu2tau*' 'ZZJJTo4L*' 'WZZ*' 'ZZZ*' 'WWZ*' 'TTZToLLNuNu_M-10_TuneCUETP8M1_13TeV-amcatnlo-pythia8' 'ZZTo4L_*Jets_ZZOnShell_13TeV-amcatnloFXFX-madspin-pythia8' 'VBFToHiggs0PMContinToZZTo*' "$EXTRA_SIGNALS" --filesPerJob 2 -o /data/uhussain/uwvvZZ_mc_ePhiResUp_"$JOBID".sh UWVVZZ_MC_EPHIRESUP_"$JOBID" Ntuplizer/test/ntuplize_cfg.py channels='eeee,eemm' isMC=1 eCalib=1 muCalib=1 lheWeights=0 ePhiResShift=1 globalTag="$GT_MC" genLeptonType=dressedHPFS

            if [ "$NO_SUB" == '' ]
            then
                nohup bash /data/uhussain/uwvvZZ_mc_ePhiResUp_"$JOBID".sh &
            fi

            # MES/MER+
            python Utilities/scripts/submitJobs.py --campaign 'RunIIFall17MiniAOD-94X_mc2017_realistic_*v1' --samples 'ZZTo4L_13TeV*pythia8' 'GluGluHToZZTo4L_M125_13TeV_powheg*' 'GluGlu*ZZ*DefaultShower*' 'GluGluToContinToZZTo2e2tau*' 'GluGluToContinToZZTo2mu2tau*' 'ZZJJTo4L*' 'WZZ*' 'ZZZ*' 'WWZ*' 'TTZToLLNuNu_M-10_TuneCUETP8M1_13TeV-amcatnlo-pythia8' 'ZZTo4L_*Jets_ZZOnShell_13TeV-amcatnloFXFX-madspin-pythia8' 'VBFToHiggs0PMContinToZZTo*' "$EXTRA_SIGNALS" --filesPerJob 2 -o /data/uhussain/uwvvZZ_mc_mClosureUp_"$JOBID".sh UWVVZZ_MC_MCLOSUREUP_"$JOBID" Ntuplizer/test/ntuplize_cfg.py channels='eemm,mmmm' isMC=1 eCalib=1 muCalib=1 lheWeights=0 mClosureShift=1 globalTag="$GT_MC" genLeptonType=dressedHPFS

            if [ "$NO_SUB" == '' ]
            then
                nohup bash /data/uhussain/uwvvZZ_mc_mClosureUp_"$JOBID".sh &
            fi

            # MES/MER-
            python Utilities/scripts/submitJobs.py --campaign 'RunIIFall17MiniAOD-94X_mc2017_realistic_*v1' --samples 'ZZTo4L_13TeV*pythia8' 'GluGluHToZZTo4L_M125_13TeV_powheg*' 'GluGlu*ZZ*DefaultShower*' 'GluGluToContinToZZTo2e2tau*' 'GluGluToContinToZZTo2mu2tau*' 'ZZJJTo4L*' 'WZZ*' 'ZZZ*' 'WWZ*' 'TTZToLLNuNu_M-10_TuneCUETP8M1_13TeV-amcatnlo-pythia8' 'ZZTo4L_*Jets_ZZOnShell_13TeV-amcatnloFXFX-madspin-pythia8' 'VBFToHiggs0PMContinToZZTo*' "$EXTRA_SIGNALS" --filesPerJob 2 -o /data/uhussain/uwvvZZ_mc_mClosureDn_"$JOBID".sh UWVVZZ_MC_MCLOSUREDN_"$JOBID" Ntuplizer/test/ntuplize_cfg.py channels='eemm,mmmm' isMC=1 eCalib=1 muCalib=1 lheWeights=0 mClosureShift=-1 globalTag="$GT_MC" genLeptonType=dressedHPFS

            if [ "$NO_SUB" == '' ]
            then
                nohup bash /data/uhussain/uwvvZZ_mc_mClosureDn_"$JOBID".sh &
            fi

        fi
    fi

    if [ "$ZL" ]
    then
        echo Creating submission infrastructure for Z+L MC Ntuples as UWVVZPLUSL_MC_"$JOBID"

        python Utilities/scripts/submitJobs.py --campaign 'RunIIFall17MiniAOD-94X_mc2017_realistic_*v1' --samples 'ZZTo4L_13TeV*' 'GluGlu*ZZ*DefaultShower*' 'DYJetsToLL_M-50_TuneCUETP8M1_13TeV-madgraphMLM-pythia8' 'TTTo2L2Nu_TuneCUETP8M2_ttHtranche3_13TeV-powheg-pythia8' 'WZTo3LNu_TuneCUETP8M1_13TeV-powheg-pythia8' 'DYToLL_*J_13TeV-amcatnloFXFX-pythia8' --filesPerJob 2 -o /data/uhussain/uwvvZPlusL_mc_"$JOBID".sh UWVVZPLUSL_MC_"$JOBID" Ntuplizer/test/ntuplize_cfg.py channels='zl' isMC=1 eCalib=1 muCalib=1 genInfo=0 lheWeights=0 globalTag="$GT_MC"

        if [ "$NO_SUB" == '' ]
        then
            echo Submitting Z+L MC Ntuples as UWVVZPLUSL_MC_"$JOBID"
            nohup bash /data/uhussain/uwvvZPlusL_mc_"$JOBID".sh &
        fi

    fi

    if [ "$Z" ]
    then
        echo Creating submission infrastructure for Single Z MC Ntuples as UWVVSINGLEZ_MC_"$JOBID"

        python Utilities/scripts/submitJobs.py --campaign 'RunIIFall17MiniAOD-94X_mc2017_realistic_*v1' --samples 'DYJetsToLL_M-50_TuneCUETP8M1_13TeV-madgraphMLM-pythia8' 'TTTo2L2Nu_TuneCUETP8M2_ttHtranche3_13TeV-powheg-pythia8' 'DYToLL_*J_13TeV-amcatnloFXFX-pythia8' 'TTJets_TuneCUETP8M2T4_13TeV-amcatnloFXFX-pythia8' --filesPerJob 2 -o /data/uhussain/uwvvSingleZ_mc_"$JOBID".sh UWVVSINGLEZ_MC_"$JOBID" Ntuplizer/test/ntuplize_cfg.py channels='z' isMC=1 eCalib=1 muCalib=1 genInfo=0 lheWeights=0 globalTag="$GT_MC"

        if [ "$NO_SUB" == '' ]
        then
            echo Submitting Single Z MC Ntuples as UWVVSINGLEZ_MC_"$JOBID"
            nohup bash /data/uhussain/uwvvSingleZ_mc_"$JOBID".sh &
        fi
    fi
fi

popd
