if [[ -z $CMSSW_BASE ]]; then
  echo "CMSSW environment not set. Don't forget to run cmsenv!"
  exit 1
elif [[ ! -d $CMSSW_BASE/src/.git ]]; then
  echo "CMSSW environment has not been set up yet"
  echo "Please run 'git cms-init' before cloning this directory"
  exit 2
fi

pushd $CMSSW_BASE/src

# === Run 3 EGM Tools ===
#https://twiki.cern.ch/twiki/bin/view/CMS/EgammaUL2016To2018#Recipe_for_running_scales_and_sm
git cms-addpkg RecoEgamma/EgammaTools  ### essentially just checkout the package from CMSSW
git clone https://github.com/cms-egamma/EgammaPostRecoTools.git
cd EgammaPostRecoTools
cd ..
mv EgammaPostRecoTools/python/EgammaPostRecoTools.py RecoEgamma/EgammaTools/python/.
git clone -b ULSSfiles_correctScaleSysMC https://github.com/jainshilpi/EgammaAnalysis-ElectronTools.git EgammaAnalysis/ElectronTools/data/
git cms-addpkg EgammaAnalysis/ElectronTools

# === Run 3 Muon Corrections ===
git clone https://github.com/cms-cat/nanoAOD-tools-modules.git PhysicsTools/NATModules

# === Calibrations/Corrections ===
UWVV/check_corrections.sh

scram b -j 12
popd
