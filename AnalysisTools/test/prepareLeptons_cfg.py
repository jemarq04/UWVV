import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing
import FWCore.PythonUtilities.LumiList as LumiList
import FWCore.ParameterSet.Types as CfgTypes

from UWVV.AnalysisTools.analysisFlowMaker import createFlow

import os

process = cms.Process("LEPTONS")

options = VarParsing.VarParsing('analysis')

options.inputFiles = '/store/mc/RunIIFall15MiniAODv2/GluGluHToZZTo4L_M2500_13TeV_powheg2_JHUgenV6_pythia8/MINIAODSIM/PU25nsData2015v1_76X_mcRun2_asymptotic_v12-v1/60000/02C0EC1D-F3E4-E511-ADCA-AC162DA603B4.root'
options.outputFile = 'ntuplize.root'
options.maxEvents = -1

options.register('globalTag', "", 
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.string,
                 "Global tag. If empty (default), auto tag is chosen based on isMC")
options.register('isMC', 1,
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.int,
                 "1 if simulation, 0 if data")
options.register('eCalib', 0,
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.int,
                 "1 if electron energy corrections are desired")
options.register('muCalib', 0,
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.int,
                 "1 if muon momentum corrections are desired")
options.register('isSync', 0,
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.int,
                 "1 if this is for synchronization purposes")
options.register('skipEvents', 0,
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.int,
                 "Number of events to skip (for debugging).")
options.register('lumiMask', '',
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.string,
                 "Lumi mask (for data only).")

options.parseArguments()



### To use IgProf's neat memory profiling tools, uncomment the following 
### lines then run this cfg with igprof like so:
###      $ igprof -d -mp -z -o igprof.mp.gz cmsRun ... 
### this will create a memory profile every 250 events so you can track use
### Turn the profile into text with
###      $ igprof-analyse -d -v -g -r MEM_LIVE igprof.yourOutputFile.gz > igreport_live.res
### To do a performance profile instead of a memory profile, change -mp to -pp
### in the first command and remove  -r MEM_LIVE from the second
### For interpretation of the output, see http://igprof.org/text-output-format.html
# from IgTools.IgProf.IgProfTrigger import igprof
# process.load("IgTools.IgProf.IgProfTrigger")
# process.igprofPath = cms.Path(process.igprof)
# process.igprof.reportEventInterval     = cms.untracked.int32(250)
# process.igprof.reportToFileAtBeginJob  = cms.untracked.string("|gzip -c>igprof.begin-job.gz")
# process.igprof.reportToFileAtEvent = cms.untracked.string("|gzip -c>igprof.%I.%E.%L.%R.event.gz")


# Basic stuff for all jobs

process.load("Configuration.StandardSequences.GeometryRecoDB_cff")

process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
from Configuration.AlCa.GlobalTag import GlobalTag

if options.globalTag:
    gt = options.globalTag
elif options.isMC:
    gt = 'auto:run2_mc'
else:
    gt = 'auto:run2_data'
process.GlobalTag = GlobalTag(process.GlobalTag, gt)

process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.schedule = cms.Schedule()

process.MessageLogger.cerr.FwkReport.reportEvery = 100

process.source = cms.Source(
    "PoolSource",
    fileNames = cms.untracked.vstring(options.inputFiles),
    skipEvents = cms.untracked.uint32(options.skipEvents),
    )

if options.lumiMask:
    lumiJSON = options.lumiMask
    if not os.path.exists(lumiJSON):
        raise IOError("Lumi mask file {} not found.".format(lumiJSON))
    lumiList = LumiList.LumiList(filename = lumiJSON)
    runs = lumiList.getRuns()
    lumisToProcess = CfgTypes.untracked(CfgTypes.VLuminosityBlockRange())
    lumisToProcess.extend(lumiList.getCMSSWString().split(','))
    process.source.lumisToProcess = lumisToProcess

process.maxEvents = cms.untracked.PSet(
    input=cms.untracked.int32(options.maxEvents)
    )

#############################################################################
#    Make the analysis flow. It is assembled from a list of classes, each   #
#    of which adds related steps to the sequence.                           #
#############################################################################
FlowSteps = []

# everybody needs vertex cleaning
from UWVV.AnalysisTools.templates.VertexCleaning import VertexCleaning
FlowSteps.append(VertexCleaning)

# everybody needs basic lepton stuff
from UWVV.AnalysisTools.templates.ElectronBaseFlow import ElectronBaseFlow
FlowSteps.append(ElectronBaseFlow)
from UWVV.AnalysisTools.templates.RecomputeElectronID import RecomputeElectronID
FlowSteps.append(RecomputeElectronID)

from UWVV.AnalysisTools.templates.MuonBaseFlow import MuonBaseFlow
FlowSteps.append(MuonBaseFlow)

from UWVV.AnalysisTools.templates.MuonScaleFactors import MuonScaleFactors
FlowSteps.append(MuonScaleFactors)

# jet energy corrections and basic preselection
from UWVV.AnalysisTools.templates.JetBaseFlow import JetBaseFlow
FlowSteps.append(JetBaseFlow)
if options.isMC:
    from UWVV.AnalysisTools.templates.JetEnergySmearing import JetEnergySmearing
    FlowSteps.append(JetEnergySmearing)


# Lepton calibrations
if options.eCalib:
    from UWVV.AnalysisTools.templates.ElectronCalibration import ElectronCalibration
    FlowSteps.append(ElectronCalibration)

if options.muCalib:
    from UWVV.AnalysisTools.templates.MuonCalibration import MuonCalibration
    FlowSteps.append(MuonCalibration)

# ZZ stuff
from UWVV.AnalysisTools.templates.ZZFlow import ZZFlow
FlowSteps.append(ZZFlow)

# but not actually the Z/ZZ combinations
from UWVV.AnalysisTools.templates.NoCombinedStates import NoCombinedStates
FlowSteps.append(NoCombinedStates)

# Turn all these into a single flow class
FlowClass = createFlow(*FlowSteps)
flow = FlowClass('flow', process, 
                 isMC=bool(options.isMC), isSync=bool(options.isSync))

outputCommands = cms.untracked.vstring(
    'drop *',
    'keep *_packedGenParticles_*_*',
    'keep *_prunedGenParticles_*_*',
    'keep *_packedPFCandidates_*_*',
    'keep *_offlineSlimmedPrimaryVertices_*_*',
    )
for obj, objName in flow.finalTags().items():
    print(obj, objName)
triggerBranches_2018 = cms.PSet(
    trigNames = cms.vstring(
         "HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL",#2018diEle
         "HLT_DoubleEle25_CaloIdL_MW",#2018diEle
         "HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass3p8",#2018diMu
         "HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL",#2018MuEle
         'HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_DZ',#2018MuEle
         "HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ",#2018MuEle
         "HLT_Mu12_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ",#2018MuEle
         "HLT_DiMu9_Ele9_CaloIdL_TrackIdL_DZ",#2018MuEle
         "HLT_Mu8_DiEle12_CaloIdL_TrackIdL_DZ",#2018MuEle
         "HLT_TripleMu_10_5_5_DZ",#2018triMu
         "HLT_TripleMu_12_10_5",#2018triMu
         "HLT_Ele32_WPTight_Gsf",#2018SingleEle
         "HLT_IsoMu24",#2018SingleMu
    ),
    HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVLPaths = cms.vstring('HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_v[0-9]+'),#2018diEle
    HLT_DoubleEle25_CaloIdL_MWPaths = cms.vstring('HLT_DoubleEle25_CaloIdL_MW_v[0-9]+'),#2018diEle
    HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass3p8Paths = cms.vstring('HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass3p8_v[0-9]+'),#2018diMu
    HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVLPaths = cms.vstring('HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_v[0-9]+'),#2018MuEle
    HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_DZPaths = cms.vstring('HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_DZ_v[0-9]+'),#2018MuEle
    HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZPaths = cms.vstring('HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ_v[0-9]+'),#2018MuEle
    HLT_Mu12_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZPaths = cms.vstring('HLT_Mu12_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ_v[0-9]+'),#2018MuEle
    HLT_DiMu9_Ele9_CaloIdL_TrackIdL_DZPaths = cms.vstring('HLT_DiMu9_Ele9_CaloIdL_TrackIdL_DZ_v[0-9]+'),#2018MuEle
    HLT_Mu8_DiEle12_CaloIdL_TrackIdL_DZPaths = cms.vstring('HLT_Mu8_DiEle12_CaloIdL_TrackIdL_DZ_v[0-9]+'),#2018MuEle
    HLT_TripleMu_10_5_5_DZPaths = cms.vstring('HLT_TripleMu_10_5_5_DZ_v[0-9]+'),#2018triMu
    HLT_TripleMu_12_10_5Paths = cms.vstring('HLT_TripleMu_12_10_5_v[0-9]+'),#2018triMu
    HLT_Ele32_WPTight_GsfPaths = cms.vstring('HLT_Ele32_WPTight_Gsf_v[0-9]+'),#2018SingleEle
    HLT_IsoMu24Paths = cms.vstring('HLT_IsoMu24_v[0-9]+'),#2018SingleMu
    
    trigResultsSrc = cms.InputTag("TriggerResults", "", "HLT"),
    trigPrescaleSrc = cms.InputTag("patTrigger"),

    checkPrescale = cms.bool(False),
    ignoreMissing = cms.untracked.bool(True),
    )

    outputCommands.append('keep *_{}_*_*'.format(objName.split(':')[0]))

process.out = cms.OutputModule(
    "PoolOutputModule",
    fileName = cms.untracked.string(options.outputFile),
    dropMetaData = cms.untracked.string("ALL"),
    outputCommands = outputCommands,
    )

process.save = cms.EndPath(process.out)

process.schedule.append(flow.getPath())
process.schedule.append(process.save)
