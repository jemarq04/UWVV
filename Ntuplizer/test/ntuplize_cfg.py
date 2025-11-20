#############################################################################
#    Module imports                                                         #
#############################################################################

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
from UWVV.Utilities.helpers import parseChannels, expandChannelName
from UWVV.Ntuplizer.makeBranchSet import makeBranchSet, makeGenBranchSet
from UWVV.Ntuplizer.eventParams import makeEventParams, makeGenEventParams

#############################################################################
#    Configuring command-line options                                       #
#############################################################################

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
yearChoices = ["2022", "2023", "2024", "2025"]
outputFileDefault = "ntuple.root"

# Parsing command-line arguments
options = VarParsing.VarParsing("analysis")
options.maxEvents = -1
options.inputFiles = []
options.setDefault("outputFile", outputFileDefault)

options.register("debug", 0,
        VarParsing.VarParsing.multiplicity.singleton,
        VarParsing.VarParsing.varType.bool,
        "1: extra print statements for debugging")
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
options.register("lumiMask", "",
        VarParsing.VarParsing.multiplicity.singleton,
        VarParsing.VarParsing.varType.string,
        "lumi mask (for data only)")
options.register("isMC", 0,
        VarParsing.VarParsing.multiplicity.singleton,
        VarParsing.VarParsing.varType.bool,
        "0: data, 1: simulation")
options.register("isPrompt", 0,
        VarParsing.VarParsing.multiplicity.singleton,
        VarParsing.VarParsing.varType.bool,
        "0: rereco, 1: prompt")
options.register("dataPeriod", "",
        VarParsing.VarParsing.multiplicity.singleton,
        VarParsing.VarParsing.varType.string,
        "period for data, given as a character with optional version (e.g. A, Cv3, ...)")
options.register("eCalib", 1,
        VarParsing.VarParsing.multiplicity.singleton,
        VarParsing.VarParsing.varType.bool,
        "electron corrections 0: off, 1: on")
options.register("muCalib", 1,
        VarParsing.VarParsing.multiplicity.singleton,
        VarParsing.VarParsing.varType.bool,
        "muon corrections 0: off, 1: on")
options.register("electronsUL", 1,
        VarParsing.VarParsing.multiplicity.singleton,
        VarParsing.VarParsing.varType.bool,
        "use 2018UL MVA for electron ID 0: off, 1: on")
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
options.register("postBPix", 0,
        VarParsing.VarParsing.multiplicity.singleton,
        VarParsing.VarParsing.varType.bool,
        "relevant for 2023 analysis. 0: 2023C, 1: 2023D")
options.register("eventsToProcess", "",
        VarParsing.VarParsing.multiplicity.list,
        VarParsing.VarParsing.varType.string,
        "events to process")
options.register("skipEvents", 0,
        VarParsing.VarParsing.multiplicity.singleton,
        VarParsing.VarParsing.varType.int,
        "number of events to skip (for debugging)")
options.parseArguments()

#############################################################################
#    Error checking and process configuration                               #
#############################################################################
if options.year not in yearChoices:
    print("ERROR: Invalid year %s" % options.year)
    print("Valid options are %s" % ", ".join(yearChoices))
    print("Default: %s" % yearDefault)
    exit(1)

if options.genLeptonType not in genLepChoices:
    print("ERROR: Invalid GEN lepton type %s" % options.genLeptonType)
    print("Valid options and corresponding status flags are")
    for key,val in genLepChoices:
        print("    %s (%s)" % (key, val))
    print("Default: %s" % genLepDefault)
    exit(1)

if options.isMC:
    if options.isPrompt:
        print("ERROR: option mismatch. isPrompt is for data.")
        exit(1)
elif options.year == "2022": #data period only needed for 2022 jet corrections
    if not options.dataPeriod:
        print("ERROR: for 2022 jet corrections, the data period must be provided (e.g. A, B, C, ...)")
        exit(1)
    vals = options.dataPeriod.split("v")
    if not vals[0].isalpha() or (len(vals) == 2 and not vals[1].isdigit()):
        print("ERROR: Invalid data period '%s'" % options.dataPeriod)
        print("Must be a single character with optional version (e.g. A, Cv3, ...)")
        exit(1)
    options.dataPeriod = options.dataPeriod.title()

# Override inputs if input file list provided
if options.inputFileList:
    with open(options.inputFileList, "r") as f:
        options.inputFiles = [line.strip() for line in f if line[0] != "#" and not line.isspace()]

# Switch off LHE if (1) data or (2) matches a given MC generator
if not options.isMC: #or all(any(x in fname.lower() for x in ["mcfm", "sherpa", "phantom"]) for fname in options.inputFiles):
    options.lheWeights = 0

# Print configuration information (output file, flags, etc.)
print("Running", options.year, "MC" if options.isMC else "Data")

if options.maxEvents != -1:
    outputFileDefault = outputFileDefault.replace(".root", "_numEvent%i.root" % options.maxEvents)
if options.outputFile == outputFileDefault:
    options.outputFile = "ntuple%s.root" % options.year
print("Output:", options.outputFile)

if options.year == "2022":
    print("postEE: %i" % options.postEE)
elif options.year == "2023":
    print("postBPix: %i" % options.postBPix)

if not options.isMC:
    print("isPrompt: %i" % options.isPrompt)
for var in ["electronsUL", "debug"]:
    if getattr(options, var):
        print("%s flag on" % var)


#############################################################################
#    Prepare CMSSW workflow                                                 #
#############################################################################

# Initializing process
process = cms.Process("Ntuple")

# Load CMS configs
process.load("Configuration.StandardSequences.GeometryRecoDB_cff")
process.load("Configuration.StandardSequences.Services_cff")
process.load("Configuration.StandardSequences.MagneticField_38T_cff")
process.load("TrackingTools.TransientTrack.TransientTrackBuilder_cfi")
process.load("Geometry.CaloEventSetup.CaloTowerConstituents_cfi")
process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")

# Retrieve list of channels from intermediate steps
channels = parseChannels(",".join(options.channels))
state_zz = any(len(c) == 4 for c in channels)
state_zl = any(len(c) == 3 for c in channels)
state_z  = any(len(c) == 2 for c in channels)
state_l  = any(len(c) == 1 for c in channels)
state_wz = "wz" in options.channels

# Determine global tag
# 2022: https://docs.google.com/presentation/d/1F4ndU7DBcyvrEEyLfYqb29NGkBPs20EAnBxe_l7AEII/edit?usp=sharing
# 2023: https://docs.google.com/presentation/d/1TjPem5jX0fzqvTGl271_nQFoVBabsrdrO0i8Qo1uD5E/edit?usp=sharing
# 2024: https://docs.google.com/presentation/d/1EHxQcWzw8IxPgCn8hm1prwSP-EktFtiuaEzH8WkQNVY/edit?usp=sharing
# 2025: https://docs.google.com/presentation/d/1H_WuzeAGkW3xZvo3oN4qGZWfw2eBuUzMhPLeShYAQoU/edit?usp=sharing
if options.globalTag:
    gt = options.globalTag
elif options.isMC:
    if options.year == "2022":
        if not options.postEE:
            gt = "140X_mcRun3_2022_realistic_v12"
        else:
            gt = "140X_mcRun3_2022_realistic_postEE_v3"
    elif options.year == "2023":
        if not options.postBPix:
            gt = "140X_mcRun3_2023_realistic_v9"
        else:
            gt = "140X_mcRun3_2023_realistic_postBPix_v3"
    elif options.year == "2024":
        gt = "150X_mcRun3_2024_realistic_v2"
else:
    if options.year == "2022":
        if not options.isPrompt:
            gt = "140X_dataRun3_v17"
        else:
            gt = "130X_dataRun3_PromptAnalysis_v1"
    elif options.year == "2023":
        if not options.isPrompt:
            gt = "140X_dataRun3_v17"
        else:
            gt = "130X_dataRun3_PromptAnalysis_v1"
    elif options.year == "2024":
        if not options.isPrompt:
            gt = "150X_dataRun3_v2"
        else:
            gt = "140X_dataRun3_Prompt_v4"
    elif options.year == "2025":
        if not options.isPrompt:
            gt = ""
        else:
            gt = "150X_dataRun3_Prompt_v1"

print("globalTag: %s" % gt)
process.GlobalTag = GlobalTag(process.GlobalTag, gt)

# Set process variables
process.schedule = cms.Schedule()
process.MessageLogger.cerr.FwkReport.reportEvery = 1
process.source = cms.Source(
    "PoolSource",
    # Avoid problem with excessive memory use in LHERunInfoProduct
    inputCommands = cms.untracked.vstring("keep *", "drop LHERunInfoProduct_*_*_*"),
    fileNames = cms.untracked.vstring(options.inputFiles),
    skipEvents = cms.untracked.uint32(options.skipEvents),
    eventsToProcess = cms.untracked.VEventRange(options.eventsToProcess)
)
if options.lumiMask:
    if not os.path.exists(options.lumiMask):
        raise IOError("Lumi mask file %s not found." % options.lumiMask)
    lumiList = LumiList.LumiList(filename=options.lumiMask)
    runs = lumiList.getRuns()
    lumisToProcess = CfgTypes.untracked(CfgTypes.VLuminosityBlockRange())
    lumisToProcess.extend(lumiList.getCMSSWString().split(","))
    process.source.lumisToProcess = lumisToProcess
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
if not state_wz:
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

# Basic jet steps + JERC
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
if not state_wz:
    from UWVV.AnalysisTools.templates.ZZFlow import ZZFlow
    FlowSteps.append(ZZFlow)

# Create final states
if state_zz or state_l:
    # Add ZZ information (including jets + jetPUSF)
    if state_zz:
        from UWVV.AnalysisTools.templates.ZZInitialStateBaseFlow import ZZInitialStateBaseFlow
        FlowSteps.append(ZZInitialStateBaseFlow)

        from UWVV.Ntuplizer.templates.altZZBranches import altZZBranches
        extraInitialStateBranches.append(altZZBranches)

    from UWVV.AnalysisTools.templates.ZZSkim import ZZSkim
    FlowSteps.append(ZZSkim)
elif state_zl or state_z or state_wz:
    from UWVV.AnalysisTools.templates.ZPlusXBaseFlow import ZPlusXBaseFlow
    FlowSteps.append(ZPlusXBaseFlow)

    if state_wz or state_zl:
        from UWVV.AnalysisTools.templates.ZPlusXInitialStateBaseFlow import ZPlusXInitialStateBaseFlow
        FlowSteps.append(ZPlusXInitialStateBaseFlow) # also embeds jets (channel zl)

        if state_wz:
            from UWVV.AnalysisTools.templates.WZFlow import WZFlow
            FlowSteps.append(WZFlow)

            from UWVV.Ntuplizer.templates.countBranches import wzCountBranches
            extraInitialStateBranches.append(wzCountBranches)
    else:
        from UWVV.AnalysisTools.templates.ZInitialStateBaseFlow import ZInitialStateBaseFlow
        FlowSteps.append(ZInitialStateBaseFlow) # also embeds jets (channel z)

if (state_zz or state_zl or state_z) and not state_wz:
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
if state_zz or state_wz:
    from UWVV.Ntuplizer.templates.vbsBranches import vbsPrimitiveBranches
    extraInitialStateBranches.append(vbsPrimitiveBranches)
    if state_zz:
        from UWVV.Ntuplizer.templates.vbsBranches import vbsDerivedBranches
        extraInitialStateBranches.append(vbsDerivedBranches)
    if options.isMC:
        from UWVV.Ntuplizer.templates.vbsBranches import vbsPrimitiveSystematicBranches
        extraInitialStateBranches.append(vbsPrimitiveSystematicBranches)
        if state_zz:
            from UWVV.Ntuplizer.templates.vbsBranches import vbsDerivedSystematicBranches
            extraInitialStateBranches.append(vbsDerivedSystematicBranches)

# Set FlowClass options
flowOpts = {
    "debug": options.debug,
    "isMC": bool(options.isMC),
    "year": options.year,
    "calibEra22": "%sEE" % ("post" if options.postEE else "pre"),
    "calibEra23": "%sBPix" % ("post" if options.postBPix else "pre"),
    "dataPeriod": options.dataPeriod,
    "electronsUL": bool(options.electronsUL)
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
if not state_wz:
    from UWVV.Ntuplizer.templates.triggerBranches import triggerBranches_2022
    trgBranches = triggerBranches_2022
else:
    from UWVV.Ntuplizer.templates.triggerBranches import verboseTriggerBranches
    trgBranches = verboseTriggerBranches

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
if state_zz and options.isMC and options.genInfo:
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
