# System modules
import os

# CMS modules
import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing
import FWCore.PythonUtilities.LumiList as LumiList
import FWCore.ParameterSet.Types as CfgTypes
from Configuration.AlCa.GlobalTag import GlobalTag

# UWVV Modules
from UWVV.AnalysisTools.analysisFlowMaker import createFlow
from UWVV.Utilities.helpers import parseChannels, expandChannelName, pset2Dict, dict2PSet
from UWVV.Ntuplizer.makeBranchSet import makeBranchSet, makeGenBranchSet
from UWVV.Ntuplizer.eventParams import makeEventParams, makeGenEventParams

# Defining constants
genLepDefault = "hardProcessFS"
genLepChoices = {
    "hardProcess": "isHardProcess()",
    "hardProcessFS": "fromHardProcessFinalState()",
    "finalstate": "status() == 1",
    "promptFS": "isPromptFinalState()",
    "dressedHPFS": "fromHardProcessFinalState()",
    "dressedFS": "status() == 1",
    "dressedPromptFS": "isPromptFinalState()"
}
yearDefault = "2022"
yearChoices = ["2022", "2023", "2024"]

# Initializing process
process = cms.Process("Ntuple")

# Parsing command-line arguments
options = VarParsing.VarParsing("analysis")
options.maxEvents = -1
options.inputFiles = []

options.register("inputFileList", "",
        VarParsing.VarParsing.multiplicity.singleton,
        VarParsing.VarParsing.varType.string,
        "name of file that lists all inputs")
options.register("channels", "zz",
        VarParsing.VarParsing.multiplicity.list,
        VarParsing.VarParsing.varType.string,
        "channel(s) to make ntuples for")
options.register("globalTag", "",
        VarParsing.VarParsing.multiplicity.singleton,
        VarParsing.VarParsing.varType.string,
        "global tag for analysis. if empty, auto tag is chosen")
options.register("isMC", 0,
        VarParsing.VarParsing.multiplicity.singleton,
        VarParsing.VarParsing.varType.bool,
        "0: data, 1: simulation")
options.register("isPrompt", 0,
        VarParsing.VarParsing.multiplicity.singleton,
        VarParsing.VarParsing.varType.bool,
        "0: rereco, 1: prompt")
options.register("eCalib", 1,
        VarParsing.VarParsing.multiplicity.singleton,
        VarParsing.VarParsing.varType.bool,
        "electron corrections 0: off, 1: on")
options.register("muCalib", 1,
        VarParsing.VarParsing.multiplicity.singleton,
        VarParsing.VarParsing.varType.bool,
        "muon corrections 0: off, 1: on")
options.register("genInfo", 0,
        VarParsing.VarParsing.multiplicity.singleton,
        VarParsing.VarParsing.varType.bool,
        "store gen-level ntuples (only for ZZ) 0: no, 1: yes")
options.register("genLeptonType", genLepDefault,
        VarParsing.VarParsing.multiplicity.singleton,
        VarParsing.VarParsing.varType.string,
        "gen lepton type. options: " \
            + ", ".join(genLepChoices.keys()))
options.register("lheWeights", 0,
        VarParsing.VarParsing.multiplicity.singleton,
        VarParsing.VarParsing.varType.int,
        "add LHE weights from MC. 0: off, 1: scale weights (weights 0-9), "
        "2: scale weights and a set of PDF weights (weights 0-111), "
        "3: all scale and PDF weights")
options.register("datasetName", "",
        VarParsing.VarParsing.multiplicity.singleton,
        VarParsing.VarParsing.varType.string,
        "dataset name")
options.register("year", yearDefault,
        VarParsing.VarParsing.multiplicity.singleton,
        VarParsing.VarParsing.varType.string,
        "year for processing samples. options: " \
            + ", ".join(yearChoices))
options.register("postEE", 0,
        VarParsing.VarParsing.multiplicity.singleton,
        VarParsing.VarParsing.varType.bool,
        "relevant for 2022 analysis. 0: 2022C-D, 1: 2022E-G")
options.register("eventsToProcess", "",
        VarParsing.VarParsing.multiplicity.list,
        VarParsing.VarParsing.varType.string,
        "events to process")
options.register("skipEvents", 0,
        VarParsing.VarParsing.multiplicity.singleton,
        VarParsing.VarParsing.varType.int,
        "number of events to skip (for debugging)")
options.parseArguments()

# Error checking
if options.year == "2022":
    print("Running 2022")
    if options.isMC:
        print("postEE: %i" % options.postEE)
    else:
        print("isPrompt: %i" % options.isPrompt)
    options.outputFile = "ntuple2022.root"
else:
    print("Run3 config still in progresss. Only 2022 is able to be processed.")
    exit(1)

if options.genLeptonType not in genLepChoices:
    print("ERROR: Invalid GEN lepton type %s" % options.genLeptonType)
    print("Valid options and corresponding status flags are")
    for key,val in genLepChoices:
        print("    %s (%s)" % (key, val))
    print("Default: %s" % genLepDefault)
    exit(1)

if (options.isMC and options.isPrompt) or (not options.isMC and options.postEE):
    print("ERROR: option mismatch. isPrompt is for data and postEE is for MC")
    exit(1)

if not options.isMC:
    options.lheWeights = 0

# Load CMS CFIs
process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")

# Retrieve list of channels from intermediate steps
channels = parseChannels(",".join(options.channels))
zz = any(len(c) == 4 for c in channels)
zl = any(len(c) == 3 for c in channels)
z  = any(len(c) == 2 for c in channels)
l  = any(len(c) == 1 for c in channels)
wz = any("wz" in c   for c in channels)

# Determine global tag
# https://docs.google.com/presentation/d/1F4ndU7DBcyvrEEyLfYqb29NGkBPs20EAnBxe_l7AEII/edit#slide=id.g289f499aa6b_2_52
if options.globalTag:
    gt = options.globalTag
elif options.isMC:
    if options.year == "2022":
        if not options.postEE:
            gt = "130X_mcRun3_2022_realistic_v5"
        else:
            gt = "130X_mcRun3_2022_realistic_postEE_v6"
else:
    if options.year == "2022":
        if not options.isPrompt:
            gt = "124X_dataRun3_v15"
        else:
            gt = "124X_dataRun3_PromptAnalysis_v2"

# Override inputs if input file list provided
if options.inputFileList:
    with open(options.inputFileList, "r") as f:
        options.inputFiles = [line.strip() for line in f if line[0] != "#" and not line.isspace()]

print("globalTag: %s" % gt)
process.GlobalTag = GlobalTag(process.GlobalTag, gt)

# Set process variables
process.schedule = cms.Schedule()
process.MessageLogger.cerr.FwkReport.reportEvery = 1
process.source = cms.Source(
    "PoolSource",
    #inputCommands("keep *", "drop LHERunInfoProduct_*_*_*"),
    fileNames = cms.untracked.vstring(options.inputFiles),
    skipEvents = cms.untracked.uint32(options.skipEvents),
    eventsToProcess = cms.untracked.VEventRange(options.eventsToProcess)
)
process.TFileService = cms.Service(
    "TFileService",
    fileName = cms.string(options.outputFile)
)
process.maxEvents = cms.untracked.PSet(
    input=cms.untracked.int32(options.maxEvents)
)

# Initialize extra option-dependent branches
extraInitialStateBranches = []
extraIntermediateStateBranches = []
extraFinalObjectBranches = {
    "e": [],
    "m": [],
}

#############################################################################
#    Make the analysis flow. It is assembled from a list of classes, each   #
#    of which adds related steps to the sequence.                           #
#############################################################################
FlowSteps = []

# Vertex cleaning
from UWVV.AnalysisTools.templates.VertexCleaning import VertexCleaning
FlowSteps.append(VertexCleaning)

# Basic lepton steps
from UWVV.AnalysisTools.templates.ElectronBaseFlow import ElectronBaseFlow
FlowSteps.append(ElectronBaseFlow)

from UWVV.AnalysisTools.templates.MuonBaseFlow import MuonBaseFlow
FlowSteps.append(MuonBaseFlow)

# Lepton corrections
if options.eCalib:
    from UWVV.AnalysisTools.templates.ElectronCalibration import ElectronCalibration
    FlowSteps.append(ElectronCalibration)

if options.muCalib:
    from UWVV.AnalysisTools.templates.MuonCalibration import MuonCalibration
    FlowSteps.append(MuonCalibration)

    from UWVV.Ntuplizer.templates.muonBranches import muonCalibrationBranches
    extraFinalObjectBranches["m"].append(muonCalibrationBranches)

# Basic jet steps + JEC
from UWVV.AnalysisTools.templates.JetBaseFlow import JetBaseFlow
FlowSteps.append(JetBaseFlow)
if options.isMC:
    from UWVV.Ntuplizer.templates.eventBranches import jetSystematicBranches
    extraInitialStateBranches.append(jetSystematicBranches)

# Gen information
if options.isMC:
    if options.lheWeights == 1:
        from UWVV.Ntuplizer.templates.eventBranches import lheScaleWeightBranches
        extraInitialStateBranches.append(lheScaleWeightBranches)
    elif options.lheWeights == 2:
        from UWVV.Ntuplizer.templates.eventBranches import lheScaleAndPDFWeightBranches
        extraInitialStateBranches.append(lheScaleAndPDFWeightBranches)
    elif options.lheWeights >= 3:
        from UWVV.Ntuplizer.templates.eventBranches import lheAllWeightBranches
        extraInitialStateBranches.append(lheAllWeightBranches)

    from UWVV.Ntuplizer.templates.eventBranches import eventGenBranches
    extraInitialStateBranches.append(eventGenBranches)
    from UWVV.Ntuplizer.templates.leptonBranches import matchedGenLeptonBranches
    extraFinalObjectBranches["e"].append(matchedGenLeptonBranches)
    extraFinalObjectBranches["m"].append(matchedGenLeptonBranches)

# Basic ZZ workflow
if not wz:
    from UWVV.AnalysisTools.templates.ZZFlow import ZZFlow
    FlowSteps.append(ZZFlow)

# Create initial states
if zz or l:
    # Add ZZ information along with jetPUSF
    if zz:
        from UWVV.AnalysisTools.templates.ZZInitialStateBaseFlow import ZZInitialStateBaseFlow
        FlowSteps.append(ZZInitialStateBaseFlow)
        
        from UWVV.AnalysisTools.templates.JetPUSFEmbedder import ZZJetPUSFEmbedder
        FlowSteps.append(ZZJetPUSFEmbedder)

    from UWVV.AnalysisTools.templates.ZZSkim import ZZSkim
    FlowSteps.append(ZZSkim)
elif zl or z or wz:
    from UWVV.AnalysisTools.templates.ZPlusXBaseFlow import ZPlusXBaseFlow
    FlowSteps.append(ZPlusXBaseFlow)

    if wz or zl:
        from UWVV.AnalysisTools.templates.ZPlusXInitialStateBaseFlow import ZPlusXInitialStateBaseFlow
        FlowSteps.append(ZPlusXInitialStateBaseFlow)

        from UWVV.AnalysisTools.templates.WZID import WZID
        FlowSteps.append(WZID)
        from UWVV.AnalysisTools.templates.WZLeptonCounters import WZLeptonCounters
        FlowSteps.append(WZLeptonCounters)

        from UWVV.Ntuplizer.templates.countBranches import wzCountBranches
        extraInitialStateBranches.append(wzCountBranches)

if (zz or zl or z) and not wz:
    for step in FlowSteps:
        if step.__name__ in ["ZZFSR", "ZZFlow"]:
            from UWVV.Ntuplizer.templates.fsrBranches import compositeObjectFSRBranches, leptonFSRBranches
            extraInitialStateBranches.append(compositeObjectFSRBranches)
            extraIntermediateStateBranches.append(compositeObjectFSRBranches)
            extraFinalObjectBranches['e'].append(leptonFSRBranches)
            extraFinalObjectBranches['m'].append(leptonFSRBranches)
            break
    for step in FlowSteps:
        if step.__name__ in ["ZZID", "ZZIso", "ZZFlow"]:
            from UWVV.AnalysisTools.templates.ZZLeptonCounters import ZZLeptonCounters
            FlowSteps.append(ZZLeptonCounters)
            from UWVV.Ntuplizer.templates.countBranches import zzCountBranches
            extraInitialStateBranches.append(zzCountBranches)
            break

# VBS variables for ZZ/WZ
if zz or wz:
    from UWVV.Ntuplizer.templates.vbsBranches import vbsPrimitiveBranches
    extraInitialStateBranches.append(vbsPrimitiveBranches)
    if zz:
        from UWVV.Ntuplizer.templates.vbsBranches import vbsDerivedBranches
        extraInitialStateBranches.append(vbsDerivedBranches)
    if options.isMC:
        from UWVV.Ntuplizer.templates.vbsBranches import vbsPrimitiveSystematicBranches
        extraInitialStateBranches.append(vbsPrimitiveSystematicBranches)
        if zz:
            from UWVV.Ntuplizer.templates.vbsBranches import vbsDerivedSystematicBranches
            extraInitialStateBranches.append(vbsDerivedSystematicBranches)

# Set FlowClass options
flowOpts = {
    "isMC": bool(options.isMC),
    "year": options.year,
    "calibEEera22": "%sEE" % ("post" if options.postEE else "pre"),
}

# Turn all these into a single flow class
FlowClass = createFlow(*FlowSteps)
flow = FlowClass("flow", process, initialstate_chans=channels, **flowOpts)

#############################################################################
#    Make the tree generators.                                              #
#############################################################################

# Meta info tree
process.metaInfo = cms.EDAnalyzer(
    "MetaTreeGenerator",
    eventParams = makeEventParams(flow.finalTags()),
    datasetName = cms.string(options.datasetName),
)
process.metaTreePath = cms.Path(process.metaInfo)
process.schedule.append(process.metaTreePath)

# Get trigger branches
if not wz:
    if options.year == "2022":
        from UWVV.Ntuplizer.templates.triggerBranches import triggerBranches_2022
        trgBranches = triggerBranches_2022

# Get filter branches
if options.isMC:
    from UWVV.Ntuplizer.templates.filterBranches import metFiltersSIM
    filterBranches = metFiltersSIM
else:
    from UWVV.Ntuplizer.templates.filterBranches import metFilters
    filterBranches = metFilters
#If you don't want extra filters, uncomment below
#filterBranches = trgBranches.clone(trigNames=cms.vstring())

# Channel trees
process.treeSequence = cms.Sequence()
for chan in channels:
    module = cms.EDAnalyzer(
        "TreeGenerator%s" % expandChannelName(chan),
        src = flow.finalObjTag(chan),
        branches = makeBranchSet(chan, extraInitialStateBranches,
                                 extraIntermediateStateBranches,
                                 **extraFinalObjectBranches),
        eventParams = makeEventParams(flow.finalTags(), chan),
        triggers = trgBranches,
        filters = filterBranches,
    )
    setattr(process, chan, module)
    process.treeSequence += module

# Gen tree information (only ZZ)
if zz and options.isMC and options.genInfo:
    process.genTreeSequence = cms.Sequence()

    from UWVV.AnalysisTools.templates.GenZZBase import GenZZBase
    from UWVV.Ntuplizer.templates.vbsBranches import vbsGenBranches

    if "dressed" in options.genLeptonType:
        from UWVV.AnalysisTools.templates.DressedGenLeptonBase import DressedGenLeptonBase
        from UWVV.Ntuplizer.templates.leptonBranches import dressedGenLeptonBranches
        GenFlow = createFlow(DressedGenLeptonBase, GenZZBase)
    else:
        from UWVV.AnalysisTools.templates.GenLeptonBase import GenLeptonBase
        GenFlow = createFlow(GenLeptonBase, GenZZBase)

    genFlow = GenFlow('genFlow', process, suffix='Gen', e='prunedGenParticles',
                    m='prunedGenParticles', a='prunedGenParticles', j='slimmedGenJets',
                    pfCands='packedGenParticles',
                    leptonStatusFlag=genLepChoices[options.genLeptonType])
    genTrg = trgBranches.clone(trigNames=cms.vstring())

    extraInitialStateBranchesGen = [vbsGenBranches]
    if options.lheWeights == 1:
        extraInitialStateBranchesGen.append(lheScaleWeightBranches)
    elif options.lheWeights == 2:
        extraInitialStateBranchesGen.append(lheScaleAndPDFWeightBranches)
    elif options.lheWeights >= 3:
        extraInitialStateBranchesGen.append(lheAllWeightBranches)

    extraIntermediateStateBranchesGen = []

    if "dressed" in options.genLeptonType.lower():
        from UWVV.Ntuplizer.templates.eventBranches import dressedGenCompositeStateBranches
        extraInitialStateBranchesGen.append(dressedGenCompositeStateBranches)
        extraIntermediateStateBranchesGen.append(dressedGenCompositeStateBranches)

    for chan in channels:
        if 'dressed' in options.genLeptonType.lower():
            genBranches = makeGenBranchSet(chan,
                                           extraInitialStateBranches=extraInitialStateBranchesGen,
                                           extraIntermediateStateBranches=extraIntermediateStateBranchesGen,
                                           e=dressedGenLeptonBranches,
                                           m=dressedGenLeptonBranches)
        else:
            genBranches = makeGenBranchSet(chan,
                                           extraInitialStateBranches=extraInitialStateBranchesGen,
                                           extraIntermediateStateBranches=extraIntermediateStateBranchesGen)
        genMod = cms.EDAnalyzer(
            'GenTreeGeneratorZZ',
            src = genFlow.finalObjTag(chan),
            branches = genBranches,
            eventParams = makeGenEventParams(genFlow.finalTags()),
            triggers = genTrg,
            filters = genTrg,
            )

        setattr(process, chan+'Gen', genMod)
        process.genTreeSequence += genMod

    pGen = genFlow.getPath()
    pGen += process.genTreeSequence

p = flow.getPath()
p += process.treeSequence
