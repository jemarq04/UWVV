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
genLepDefault = "dressedHPFS"
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

# Override inputs if input file list provided
if options.inputFileList:
    with open(options.inputFileList, "r") as f:
        options.inputFiles = [line.strip() for line in f if line[0] != "#" and not line.isspace()]

# Print configuration information (output file, flags, etc.)
print("Running", options.year, "MC")

if options.maxEvents != -1:
    outputFileDefault = outputFileDefault.replace(".root", "_numEvent%i.root" % options.maxEvents)
if options.outputFile == outputFileDefault:
    options.outputFile = "ntuple%s.root" % options.year
print("Output:", options.outputFile)

if options.year == "2022":
    print("postEE: %i" % options.postEE)
elif options.year == "2023":
    print("postBPix: %i" % options.postBPix)

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
else:
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
process.TFileService = cms.Service(
    "TFileService",
    fileName = cms.string(options.outputFile)
)
process.maxEvents = cms.untracked.PSet(
    input=cms.untracked.int32(options.maxEvents)
)

#############################################################################
#    Create workflow sequence.                                              #
#############################################################################

# Gen tree information (only ZZ)
if state_zz:
    process.genTreeSequence = cms.Sequence()

    from UWVV.AnalysisTools.templates.GenZZXsecFlow import GenZZXsecFlow

    if "dressed" in options.genLeptonType:
        from UWVV.AnalysisTools.templates.DressedGenLeptonBase import DressedGenLeptonBase
        GenFlow = createFlow(DressedGenLeptonBase, GenZZXsecFlow)
    else:
        from UWVV.AnalysisTools.templates.GenLeptonBase import GenLeptonBase
        GenFlow = createFlow(GenLeptonBase, GenZZXsecFlow)

    genFlow = GenFlow("genFlow", process,
        suffix="Gen",
        e="prunedGenParticles",
        m="prunedGenParticles",
        a="prunedGenParticles",
        j="slimmedGenJets",
        pfCands="packedGenParticles",
        leptonStatusFlag=genLepChoices[options.genLeptonType],
        isDressed="dressed" in options.genLeptonType,
    )
    pGen = genFlow.getPath()
    print(pGen)
