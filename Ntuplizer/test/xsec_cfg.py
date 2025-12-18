#############################################################################
#    Module imports                                                         #
#############################################################################

# CMS modules
import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing

# UWVV Modules
from UWVV.AnalysisTools.analysisFlowMaker import createFlow
from UWVV.Utilities.helpers import parseChannels

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
options.register("genLeptonType", genLepDefault,
        VarParsing.VarParsing.multiplicity.singleton,
        VarParsing.VarParsing.varType.string,
        "gen lepton type. options: " \
            + ", ".join(genLepChoices.keys()))
options.parseArguments()

#############################################################################
#    Error checking and process configuration                               #
#############################################################################
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

#############################################################################
#    Prepare CMSSW workflow                                                 #
#############################################################################

# Initializing process
process = cms.Process("Ntuple")

# Retrieve list of channels from intermediate steps
channels = parseChannels(",".join(options.channels))
state_zz = any(len(c) == 4 for c in channels)
state_zl = any(len(c) == 3 for c in channels)
state_z  = any(len(c) == 2 for c in channels)
state_l  = any(len(c) == 1 for c in channels)
state_wz = "wz" in options.channels

# Set process variables
process.schedule = cms.Schedule()
process.MessageLogger.cerr.FwkReport.reportEvery = 1
process.source = cms.Source(
    "PoolSource",
    # Avoid problem with excessive memory use in LHERunInfoProduct
    inputCommands = cms.untracked.vstring("keep *", "drop LHERunInfoProduct_*_*_*"),
    fileNames = cms.untracked.vstring(options.inputFiles),
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
