# Calculates the gen-level cross-sections for born/dressed leptons with
# on-shell and fiducial cuts.
# Intended to be used with pp->ZZ->4l sample (e.g. POWHEG) and gg->ZZ->4l

import FWCore.ParameterSet.Config as cms
from FWCore.ParameterSet.VarParsing import VarParsing

# parse command-line inputs
options = VarParsing("analysis")
options.maxEvents = -1
options.inputFiles = []

options.register(
    "inputFileList",
    "",
    VarParsing.multiplicity.singleton,
    VarParsing.varType.string,
    "name of file that lists all inputs",
)

options.register(
    "scale",
    1.0,
    VarParsing.multiplicity.singleton,
    VarParsing.varType.float,
    "value by which to scale calculated cross-section",
)

options.parseArguments()

# Initialize process
process = cms.Process("XSec")
process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = 100000
process.maxEvents = cms.untracked.PSet(input=cms.untracked.int32(options.maxEvents))
process.p = cms.Path()

# Set inputs
if options.inputFileList:
    with open(options.inputFileList, "r") as infile:
        templist = [line.strip() for line in infile if line[0] != "#" and not line.isspace()]
        options.inputFiles = [line.replace("/hdfs", "") if line.startswith("/hdfs") else line for line in templist]
if not options.inputFiles:
    print("ERROR: empty input files")
    exit(1)

process.source = cms.Source(
    "PoolSource",
    fileNames=cms.untracked.vstring(options.inputFiles),
    secondaryFileNames=cms.untracked.vstring(),
)

# Calculate cross-sections
process.xsec = cms.EDAnalyzer(
    "GenZZXsecAnalyzer",
    src=cms.InputTag("prunedGenParticles"),
    scale=cms.double(options.scale),
    verbose=cms.bool(True),
)
process.p += process.xsec

process.dressedxsec = cms.EDAnalyzer(
    "GenZZDressedXsecAnalyzer",
    src=cms.InputTag("prunedGenParticles"),
    label=cms.string("dressed"),
    scale=cms.double(options.scale),
    verbose=cms.bool(True),
)
process.p += process.dressedxsec

# Schedule
process.schedule = cms.Schedule(process.p)
