import FWCore.ParameterSet.Config as cms

from UWVV.AnalysisTools.analysisFlowMaker import createFlow
from UWVV.AnalysisTools.templates.ElectronBaseFlow import ElectronBaseFlow
from UWVV.AnalysisTools.templates.MuonBaseFlow import MuonBaseFlow
from UWVV.AnalysisTools.templates.ZZInitialStateBaseFlow import ZZInitialStateBaseFlow


process = cms.Process("PrintZZ")

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
        "/store/data/Run2017C/MuonEG/MINIAOD/PromptReco-v3/000/300/742/00000/0A1C877F-617E-E711-A3F8-02163E01A332.root"
    ),
)

process.maxEvents = cms.untracked.PSet(input=cms.untracked.int32(100))

LeptonFlowClass = createFlow(ElectronBaseFlow, MuonBaseFlow)
leptonFlow = LeptonFlowClass("leptonFlow", process)

InitialStateFlowClass = createFlow(ZZInitialStateBaseFlow)
initialStateFlow = InitialStateFlowClass("initialStateFlow", process, **leptonFlow.outputs[-1])

process.print4ECands = cms.EDAnalyzer(
    "CandidatePrinter",
    src=initialStateFlow.finalObjTag("eeee"),
)

process.print2E2MuCands = cms.EDAnalyzer(
    "CandidatePrinter",
    src=initialStateFlow.finalObjTag("eemm"),
)

process.print4MuCands = cms.EDAnalyzer(
    "CandidatePrinter",
    src=initialStateFlow.finalObjTag("mmmm"),
)

process.printCands = cms.EndPath(process.print4ECands * process.print2E2MuCands * process.print4MuCands)

process.schedule.append(process.printCands)
