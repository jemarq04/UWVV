'''

Little script to get the number of events triggering any relevant path.
Uses CMSSW FWLite, so it must be run from a cmsenv.

Nate Woods, U. Wisconsin

'''

# import ROOT in batch mode
import sys
oldargv = sys.argv[:]
sys.argv = [ '-b-' ]
import ROOT
ROOT.gROOT.SetBatch(True)
sys.argv = oldargv

# FWLite C++ libs
ROOT.gSystem.Load("libFWCoreFWLite.so")
ROOT.gSystem.Load("libDataFormatsFWLite.so")
ROOT.AutoLibraryLoader.enable()

# FWLite python libs
from DataFormats.FWLite import Events, Handle

# Triggers to check for
goodTriggers = [
    'HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_v',
    'HLT_Mu17_TrkIsoVVL_TkMu8_TrkIsoVVL_DZ_v',
    'HLT_Ele17_Ele12_CaloIdL_TrackIdL_IsoVL_DZ_v',
    'HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_DZ_v',
    'HLT_Mu8_TrkIsoVVL_Ele17_CaloIdL_TrackIdL_IsoVL_v',
    'HLT_Mu17_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_Iso_IsoVL_v',
    'HLT_Ele16_Ele12_Ele8_CaloIdL_TrackIdL_v',
    'HLT_Ele23_WPLoose_Gsf_v',
]
goodTriggers = set(goodTriggers) # for fast lookup

nPass = {t:0 for t in goodTriggers}

triggerBits, triggerBitLabel = Handle("edm::TriggerResults"), ("TriggerResults","","HLT")

events = Events("root://eoscms.cern.ch//store/mc/RunIISpring16MiniAODv1/WminusH_HToZZTo4L_M150_13TeV_powheg2-minlo-HWJ_JHUgenV6_pythia8/MINIAODSIM/PUSpring16RAWAODSIM_80X_mcRun2_asymptotic_2016_v3-v1/00000/C6DF34D4-CF20-E611-A8EE-782BCB27B958.root")

for iEv, event in enumerate(events):
    if iEv % 5000 == 0:
        print("Checking event %d"%iEv)
    event.getByLabel(triggerBitLabel, triggerBits)
    names = event.object().triggerNames(triggerBits.product())
    for i in xrange(triggerBits.product().size()):
        if names.triggerName(i)[:-1] in goodTriggers:
            if triggerBits.product().accept(i):
                nPass[names.triggerName(i)[:-1]] += 1


print("%d Events passing each trigger:")
for t in goodTriggers:
    print("     {}: {}".format(t,nPass[t]))
