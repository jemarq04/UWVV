#!/usr/bin/bash


if [ "$1" == "-h" ] || [ "$1" == "--help" ]
then
    echo "$0 usage: ./$0 [-h|--help] [--hzzExtras] [-j NTHREADS]"
    echo "    --hzzExtras: Get and compile HZZ matrix element and kinematic fit stuff, and generate the UWVV plugins that use them."
    echo "               This is not the default because most people do not need them and one of the packages' authors frequently make changes that break everything without intervention on our side."
    echo "               NB if you use this option and later use scram b clean, you should rerun this script with this option or your CONDOR jobs may fail."
    echo "    --met: Download updated MET correction recipes (needed for MET filters and uncertainties)"
    echo "    -j NTHREADS: [with --hzzExtras] Compile ZZMatrixElement package with NTHREADS threads (default 12)."
    exit 1
fi

while [ "$1" ]
do
    case "$1" in
        --hzzExtras)
            HZZ=1
            ;;
        --met)
            MET=1
            ;;
        -j)
            shift
            UWVVNTHREADS="$1"
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac

    shift
done

if [ ! "$UWVVNTHREADS" ]; then
    UWVVNTHREADS=12
fi

pushd $CMSSW_BASE/src

#https://twiki.cern.ch/twiki/bin/view/CMS/EgammaPostRecoRecipes#2018_Data_MC
git cms-merge-topic cms-egamma:EgammaPostRecoTools #just adds in an extra file to have a setup function to make things easier
git cms-addpkg EgammaAnalysis/ElectronTools

rm -rf EgammaAnalysis/ElectronTools/data
git clone git@github.com:cms-data/EgammaAnalysis-ElectronTools.git EgammaAnalysis/ElectronTools/data

# For now all that we copy over are the text files that contain the corrections as the .h and .cc files are already in UWVV
# If they undergo significant changes, we should use the second commented command instead.
# (For now they are slight organizational changes, so we omit them. Any significant changes should be committed.)
git clone --recursive ssh://git@gitlab.cern.ch:7999/akhukhun/roccor.git
[[ -d roccor ]] && mv roccor/RoccoR*.txt UWVV/data/RochesterCorrections/ || echo "ERROR: error cloning roccor gitlab repo"
#[[ -d roccor ]] && mv roccor/RoccoR*.* UWVV/data/RochesterCorrections/ || echo "ERROR: error cloning roccor gitlab repo"

#To get the electron BDT you have to checkout a package like this:
git cms-merge-topic mkovac:Electron_XGBoost_MVA_2016_and_2018_CMSSW_10_2_15

# Muon MVA
git clone https://github.com/mkovac/MuonMVAReader.git MuonMVAReader

popd
