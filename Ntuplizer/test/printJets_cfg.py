import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing
import FWCore.PythonUtilities.LumiList as LumiList
import FWCore.ParameterSet.Types as CfgTypes

from UWVV.AnalysisTools.analysisFlowMaker import createFlow

from UWVV.Utilities.helpers import parseChannels, expandChannelName
from UWVV.Ntuplizer.makeBranchSet import makeBranchSet, makeGenBranchSet
from UWVV.Ntuplizer.eventParams import makeEventParams, makeGenEventParams

import os

process = cms.Process("Ntuple")

options = VarParsing.VarParsing('analysis')

options.inputFiles = '/store/mc/RunIIFall15MiniAODv2/GluGluHToZZTo4L_M2500_13TeV_powheg2_JHUgenV6_pythia8/MINIAODSIM/PU25nsData2015v1_76X_mcRun2_asymptotic_v12-v1/60000/02C0EC1D-F3E4-E511-ADCA-AC162DA603B4.root'
options.outputFile = 'ntuplize.root'
options.maxEvents = -1

options.register('channels', "zz",
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.string,
                 "Channel to make ntuples for. May be comma-separated list and/or several presets like 'zz'")
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
options.register('profile', 0,
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.int,
                 "Set nonzero to run igprof.")
options.register('hzzExtra', 0,
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.int,
                 "1 if extra HZZ quantities like matrix element "
                 "discriminators and Z kinematic refit are desired.")
options.register('genInfo', 0,
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.int,
                 "1 if gen-level ntuples are desired.")
options.register('genLeptonType', 'hardProcessFS',
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.string,
                 "lepton type. Options: dressedHPFS, dressedFS, "
                 "hardProcesFS, hardProcess")
options.register('eScaleShift', 0,
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.int,
                 'Electron energy scale shift, in units of sigma.')
options.register('eRhoResShift', 0,
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.int,
                 'Electron energy smearing rho shift, in units of sigma.')
options.register('ePhiResShift', 0,
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.int,
                 'Electron energy smearing phi shift, in units of sigma.')
options.register('mClosureShift', 0,
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.int,
                 'Muon calibration closure shift, in units of sigma.')
options.register('lheWeights', 1,
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.int,
                 'Add LHE weights from Monte Carlo. Option 1 = scale weights '
                 '(weights 0-9), 2 = scale weights and one set of PDF weights '
                 '(weights 0-111), 3 = all scale and PDF weights. Default 1.')

options.parseArguments()

genLepChoices =  {"hardProcess" : "isHardProcess()",
        "hardProcessFS" : "fromHardProcessFinalState()",
        "finalstate" : "status() == 1",
        "promptFS" : "isPromptFinalState()",
        "dressedHPFS" : "fromHardProcessFinalState()",
        "dressedFS" : "status() == 1",
        "dressedPromptFS" : "isPromptFinalState()",
}
if options.genLeptonType not in genLepChoices:
    print("ERROR: Invalid GEN-lepton type %s" % options.genLeptonType)
    print("Valid optons and corresponding status flags are")
    print("    Format: keyword: status flag")
    print("    Default: hardProcessFS: fromHardProcessFinalState()")
    print("-"*80)
    for key, value in genLepChoices.items():
        print("    %s: %s" % (key, value))
    exit(1)

channels = parseChannels(options.channels)
zz = any(len(c) == 4 for c in channels)
zl = any(len(c) == 3 for c in channels)
z  = any(len(c) == 2 for c in channels)
l  = any(len(c) == 1 for c in channels)


### To use IgProf's neat memory profiling tools, run with the profile
### option and igprof like so:
###      $ igprof -d -mp -z -o igprof.mp.gz cmsRun profile=1 ...
### this will create a memory profile every 250 events so you can track use
### Turn the profile into text with
###      $ igprof-analyse -d -v -g -r MEM_LIVE igprof.yourOutputFile.gz > igreport_live.res
### To do a performance profile instead of a memory profile, change -mp to -pp
### in the first command and remove  -r MEM_LIVE from the second
### For interpretation of the output, see http://igprof.org/text-output-format.html
if options.profile:
    from IgTools.IgProf.IgProfTrigger import igprof
    process.load("IgTools.IgProf.IgProfTrigger")
    process.igprofPath = cms.Path(process.igprof)
    process.igprof.reportEventInterval     = cms.untracked.int32(250)
    process.igprof.reportToFileAtBeginJob  = cms.untracked.string("|gzip -c>igprof.begin-job.gz")
    process.igprof.reportToFileAtEvent = cms.untracked.string("|gzip -c>igprof.%I.%E.%L.%R.event.gz")


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

process.TFileService = cms.Service(
    "TFileService",
    fileName = cms.string(options.outputFile),
    )

process.maxEvents = cms.untracked.PSet(
    input=cms.untracked.int32(options.maxEvents)
    )

# option-dependent branches go here
extraInitialStateBranches = []
extraIntermediateStateBranches = []
extraFinalObjectBranches = {'e':[],'m':[]}

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
from UWVV.AnalysisTools.templates.ElectronScaleFactors import ElectronScaleFactors
FlowSteps.append(ElectronScaleFactors)

# data and MCFM samples never have LHE info
if not options.isMC or 'mcfm' in options.inputFiles[0].lower():
    options.lheWeights = 0

# jet energy corrections and basic preselection
from UWVV.AnalysisTools.templates.JetBaseFlow import JetBaseFlow
FlowSteps.append(JetBaseFlow)
if options.isMC:
    from UWVV.Ntuplizer.templates.eventBranches import jetSystematicBranches
    extraInitialStateBranches.append(jetSystematicBranches)

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
    extraFinalObjectBranches['e'].append(matchedGenLeptonBranches)
    extraFinalObjectBranches['m'].append(matchedGenLeptonBranches)

if any(len(c) == 4 for c in channels):
    from UWVV.Ntuplizer.templates.eventBranches import centralJetBranches
    extraInitialStateBranches.append(centralJetBranches)

# FSR and ZZ/HZZ stuff
from UWVV.AnalysisTools.templates.ZZFlow import ZZFlow
FlowSteps.append(ZZFlow)

# make final states
if zz:
    from UWVV.AnalysisTools.templates.ZZInitialStateBaseFlow import ZZInitialStateBaseFlow
    FlowSteps.append(ZZInitialStateBaseFlow)

    if options.hzzExtra:
        # k factors if a gg sample
        if options.isMC and any('GluGlu' in f for f in options.inputFiles):
            from UWVV.AnalysisTools.templates.GGHZZKFactors import GGHZZKFactors
            FlowSteps.append(GGHZZKFactors)

        # HZZ discriminants and categorization
        from UWVV.AnalysisTools.templates.ZZClassification import ZZClassification
        FlowSteps.append(ZZClassification)

        from UWVV.AnalysisTools.templates.ZKinematicFitting import ZKinematicFitting
        FlowSteps.append(ZKinematicFitting)

        from UWVV.Ntuplizer.templates.zzDiscriminantBranches import zzDiscriminantBranches, kinFitBranches
        extraInitialStateBranches += [zzDiscriminantBranches, kinFitBranches]

    from UWVV.AnalysisTools.templates.ZZSkim import ZZSkim
    FlowSteps.append(ZZSkim)

elif zl or z:
    from UWVV.AnalysisTools.templates.ZPlusXBaseFlow import ZPlusXBaseFlow
    FlowSteps.append(ZPlusXBaseFlow)
    if zl:
        from UWVV.AnalysisTools.templates.ZPlusXInitialStateBaseFlow import ZPlusXInitialStateBaseFlow
        FlowSteps.append(ZPlusXInitialStateBaseFlow)

        from UWVV.AnalysisTools.templates.WZLeptonCounters import WZLeptonCounters
        FlowSteps.append(WZLeptonCounters)

        from UWVV.AnalysisTools.templates.BJetCounters import BJetCounters
        FlowSteps.append(BJetCounters)

        from UWVV.Ntuplizer.templates.countBranches import wzCountBranches
        extraInitialStateBranches.append(wzCountBranches)
elif l:
    from UWVV.AnalysisTools.templates.ZZSkim import ZZSkim
    FlowSteps.append(ZZSkim)


if (zz or zl or z) and not "wz" in options.channels:
    for f in FlowSteps:
        if f.__name__ in ['ZZFSR', 'ZZFlow']:
            from UWVV.Ntuplizer.templates.fsrBranches import compositeObjectFSRBranches, leptonFSRBranches
            extraInitialStateBranches.append(compositeObjectFSRBranches)
            extraIntermediateStateBranches.append(compositeObjectFSRBranches)
            extraFinalObjectBranches['e'].append(leptonFSRBranches)
            extraFinalObjectBranches['m'].append(leptonFSRBranches)
            break
    for f in FlowSteps:
        if f.__name__ in ['ZZID', 'ZZIso', 'ZZFlow']:
            from UWVV.AnalysisTools.templates.ZZLeptonCounters import ZZLeptonCounters
            FlowSteps.append(ZZLeptonCounters)
            from UWVV.Ntuplizer.templates.countBranches import zzCountBranches
            extraInitialStateBranches.append(zzCountBranches)
            break

# Lepton calibrations
if options.eCalib:
    from UWVV.AnalysisTools.templates.ElectronCalibration import ElectronCalibration
    FlowSteps.append(ElectronCalibration)

if options.muCalib:
    from UWVV.AnalysisTools.templates.MuonCalibration import MuonCalibration
    FlowSteps.append(MuonCalibration)

    from UWVV.Ntuplizer.templates.muonBranches import muonCalibrationBranches
    extraFinalObjectBranches['m'].append(muonCalibrationBranches)


# VBS variables for ZZ
if zz:
    from UWVV.Ntuplizer.templates.vbsBranches import vbsBranches
    extraInitialStateBranches.append(vbsBranches)
    if options.isMC:
        from UWVV.Ntuplizer.templates.vbsBranches import vbsSystematicBranches
        extraInitialStateBranches.append(vbsSystematicBranches)


# Trigger info is only in MC from reHLT campaign
if 'RunIISpring16' in options.inputFiles[0] and 'reHLT' not in options.inputFiles[0] and 'withHLT' not in options.inputFiles[0]:
    trgBranches = cms.PSet(
        trigNames=cms.vstring(),
        trigResultsSrc = cms.InputTag("TriggerResults", "", "HLT"),
        trigPrescaleSrc = cms.InputTag("patTrigger"),
        )
elif 'Run2016G' in options.inputFiles[0]:
    from UWVV.Ntuplizer.templates.triggerBranches import triggerBranches_2016G
    trgBranches = triggerBranches_2016G
elif 'Run2016H' in options.inputFiles[0]:
    from UWVV.Ntuplizer.templates.triggerBranches import triggerBranches_2016H
    trgBranches = triggerBranches_2016H
else:
    from UWVV.Ntuplizer.templates.triggerBranches import triggerBranches
    trgBranches = triggerBranches

    if 'reHLT' in options.inputFiles[0]:
        trgBranches = trgBranches.clone(trigResultsSrc=cms.InputTag("TriggerResults", "", "HLT2"))


# Gen ntuples if desired
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
            )

        setattr(process, chan+'Gen', genMod)
        process.genTreeSequence += genMod

    pGen = genFlow.getPath()
    pGen += process.genTreeSequence
    process.schedule.append(pGen)


flowOpts = {
    'isMC' : bool(options.isMC),
    'isSync' : bool(options.isMC) and bool(options.isSync),

    'electronScaleShift' : options.eScaleShift,
    'electronRhoResShift' : options.eRhoResShift,
    'electronPhiResShift' : options.ePhiResShift,
    'muonClosureShift' : options.mClosureShift,
    }

# include gen initial states' input tags if needed
if zz and options.isMC and options.genInfo:
    for chan in channels:
        flowOpts[chan+'Gen'] = genFlow.finalObjTagString(chan)
    from UWVV.Ntuplizer.templates.eventBranches import genInitialStateBranches
    extraInitialStateBranches.append(genInitialStateBranches)

# Turn all these into a single flow class
FlowClass = createFlow(*FlowSteps)
flow = FlowClass('flow', process, **flowOpts)


process.printJets = cms.EDAnalyzer(
    'JetPrinter',
    src = flow.finalObjTag('j'),
    )
p = flow.getPath()
process.schedule.append(p)
process.pPrint = cms.Path(process.printJets)
process.schedule.append(process.pPrint)
