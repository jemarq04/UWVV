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

#replace files with UL version
mv AnalysisTools/plugins/PATMuonZZIDEmbedder.cc AnalysisTools/plugins/PATMuonZZIDEmbedder.cc_backup_old
mv AnalysisTools/python/templates/ElectronCalibration.py AnalysisTools/python/templates/ElectronCalibration.py_backup_old
mv AnalysisTools/python/templates/ZZID.py AnalysisTools/python/templates/ZZID.py_backup_old

mv AnalysisTools/plugins/PATObjectValueEmbedder.cc AnalysisTools/plugins/PATObjectValueEmbedder.cc_backup_old
mv AnalysisTools/python/templates/BadMuonFilters.py AnalysisTools/python/templates/BadMuonFilters.py_backup_old

cp AnalysisTools/plugins/PATMuonZZIDEmbedder.cc_UL AnalysisTools/plugins/PATMuonZZIDEmbedder.cc
cp AnalysisTools/python/templates/ElectronCalibrationUL.py AnalysisTools/python/templates/ElectronCalibration.py
cp AnalysisTools/python/templates/ZZID_UL.py AnalysisTools/python/templates/ZZID.py

cp AnalysisTools/plugins/PATObjectValueEmbedder.cc_UL AnalysisTools/plugins/PATObjectValueEmbedder.cc
cp AnalysisTools/python/templates/BadMuonFilters.py_UL AnalysisTools/python/templates/BadMuonFilters.py

pushd $CMSSW_BASE/src

#https://twiki.cern.ch/twiki/bin/view/CMS/EgammaUL2016To2018#Recipe_for_running_scales_and_sm
git cms-addpkg RecoEgamma/EgammaTools  ### essentially just checkout the package from CMSSW
git clone https://github.com/cms-egamma/EgammaPostRecoTools.git
mv EgammaPostRecoTools/python/EgammaPostRecoTools.py RecoEgamma/EgammaTools/python/.
git clone -b ULSSfiles_correctScaleSysMC https://github.com/jainshilpi/EgammaAnalysis-ElectronTools.git EgammaAnalysis/ElectronTools/data/
git cms-addpkg EgammaAnalysis/ElectronTools

# Muon MVA, no longer compiles currently 
#git clone https://github.com/mkovac/MuonMVAReader.git MuonMVAReader

#(To avoid compilation errors although we t need this anymore in 2017/2018: https://github.com/CJLST/ZZAnalysis/blob/miniAOD_80X/checkout_10X.csh#L89)don
git clone https://github.com/bachtis/Analysis.git -b KaMuCa_V4 KaMuCa
scram b -j 12
popd
