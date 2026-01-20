import FWCore.ParameterSet.Config as cms

from UWVV.AnalysisTools.analysisFlowMaker import createFlow
from UWVV.AnalysisTools.templates.ZZFlow import ZZFlow
from UWVV.AnalysisTools.templates.VertexCleaning import VertexCleaning
from UWVV.AnalysisTools.templates.ZZInitialStateBaseFlow import ZZInitialStateBaseFlow
from UWVV.Ntuplizer.makeBranchSet import makeBranchSet
from UWVV.Ntuplizer.eventParams import makeEventParams
from UWVV.Ntuplizer.templates.triggerBranches import triggerBranches


process = cms.Process("TestNtuple")

process.load("Configuration.StandardSequences.GeometryRecoDB_cff")

process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
from Configuration.AlCa.GlobalTag import GlobalTag

process.GlobalTag = GlobalTag(process.GlobalTag, "auto:run2_mc")

process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.schedule = cms.Schedule()

process.MessageLogger.cerr.FwkReport.reportEvery = 100

process.source = cms.Source(
    "PoolSource",
    fileNames=cms.untracked.vstring(
        "/store/mc/RunIIFall15MiniAODv2/GluGluHToZZTo4L_M2500_13TeV_powheg2_JHUgenV6_pythia8/MINIAODSIM/PU25nsData2015v1_76X_mcRun2_asymptotic_v12-v1/60000/02C0EC1D-F3E4-E511-ADCA-AC162DA603B4.root"
    ),
)

process.TFileService = cms.Service(
    "TFileService",
    fileName=cms.string("testNtuple.root"),
)

process.maxEvents = cms.untracked.PSet(input=cms.untracked.int32(10000))

ZZFlowClass = createFlow(VertexCleaning, ZZInitialStateBaseFlow, ZZFlow)
zzFlow = ZZFlowClass("zzFlow", process)

process.treeMakerEEEE = cms.EDAnalyzer(
    "TreeGeneratorEEEE",
    src=zzFlow.finalObjTag("zz4e"),
    branches=makeBranchSet("eeee"),
    eventParams=makeEventParams(zzFlow.finalTags()),
    triggers=triggerBranches,
)
process.treeMakerEEMuMu = cms.EDAnalyzer(
    "TreeGeneratorEEMuMu",
    src=zzFlow.finalObjTag("zz2e2m"),
    branches=makeBranchSet("eemm"),
    eventParams=makeEventParams(zzFlow.finalTags()),
    triggers=triggerBranches,
)
process.treeMakerMuMuMuMu = cms.EDAnalyzer(
    "TreeGeneratorMuMuMuMu",
    src=zzFlow.finalObjTag("zz4m"),
    branches=makeBranchSet("mmmm"),
    eventParams=makeEventParams(zzFlow.finalTags()),
    triggers=triggerBranches,
)

process.metaTreeMaker = cms.EDAnalyzer(
    "MetaTreeGenerator",
    eventParams=makeEventParams(zzFlow.finalTags()),
)

process.makeTrees = cms.Path(
    process.treeMakerEEEE * process.treeMakerEEMuMu * process.treeMakerMuMuMuMu * process.metaTreeMaker
)

process.schedule.append(process.makeTrees)
