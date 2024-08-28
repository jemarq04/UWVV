import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing
import FWCore.PythonUtilities.LumiList as LumiList
import FWCore.ParameterSet.Types as CfgTypes
import pdb

from UWVV.AnalysisTools.analysisFlowMaker import createFlow

from UWVV.Utilities.helpers import parseChannels, expandChannelName, pset2Dict, dict2PSet
from UWVV.Ntuplizer.makeBranchSet import makeBranchSet, makeGenBranchSet
from UWVV.Ntuplizer.eventParams import makeEventParams, makeGenEventParams

import os

#pdb.set_trace()
process = cms.Process("Ntuple")

options = VarParsing.VarParsing('analysis')
#DataFiles to find these 5 events that I am cutting out but HZZ is not
#options.inputFiles='/store/data/Run2017B/DoubleEG/MINIAOD/17Nov2017-v1/50000/A81533F4-21D3-E711-A594-00259073E544.root'
#,'/store/data/Run2017C/DoubleEG/MINIAOD/17Nov2017-v1/70000/4E9E1C78-79E4-E711-8ACB-001E677923E2.root','/store/data/Run2017E/DoubleEG/MINIAOD/17Nov2017-v1/40000/AA05D9DE-B7D3-E711-A4C8-02163E011B82.root','/store/data/Run2017E/DoubleEG/MINIAOD/17Nov2017-v1/40000/821C2067-01D6-E711-B347-A4BF0112BC8A.root','/store/data/Run2017F/DoubleEG/MINIAOD/17Nov2017-v1/60000/6AFE90B3-37E1-E711-BABD-0025905C5502.root','/store/data/Run2017B/DoubleMuon/MINIAOD/17Nov2017-v1/50000/E819D0A8-80D4-E711-86FD-FA163E17588A.root','/store/data/Run2017C/DoubleMuon/MINIAOD/17Nov2017-v1/50000/EA3BC5D6-7BD3-E711-AF74-02163E01431D.root','/store/data/Run2017E/DoubleMuon/MINIAOD/17Nov2017-v1/30000/78BD22A9-71D5-E711-BEB5-0242AC130002.root','/store/data/Run2017E/DoubleMuon/MINIAOD/17Nov2017-v1/30000/F4B552AD-69D5-E711-A729-141877410ACD.root','/store/data/Run2017F/DoubleMuon/MINIAOD/17Nov2017-v1/60000/D675115D-73DE-E711-A807-02163E0129DF.root',
#options.inputFiles = '/store/mc/RunIIFall17MiniAOD/GluGluHToZZTo4L_M125_13TeV_powheg2_JHUGenV7011_pythia8/MINIAODSIM/94X_mc2017_realistic_v10-v1/40000/205E2EB6-2600-E811-A8D9-A0369FC5E090.root','/store/mc/RunIIFall17MiniAOD/VBF_HToZZTo4L_M125_13TeV_powheg2_JHUGenV7011_pythia8/MINIAODSIM/94X_mc2017_realistic_v10-v2/00000/E8505BB6-5F07-E811-B009-002590DE6E88.root','/store/mc/RunIIFall17MiniAOD/WminusH_HToZZTo4L_M125_13TeV_powheg2-minlo-HWJ_JHUGenV7011_pythia8/MINIAODSIM/94X_mc2017_realistic_v10-v1/10000/80B92986-8501-E811-99BB-002590200900.root'
#options.inputFiles='/store/data/Run2017B/DoubleEG/MINIAOD/31Mar2018-v1/00000/000A6D14-8037-E811-A09B-0CC47A5FBDC1.root'
###Run2018A-v3 DoubleMuon
#options.inputFiles='/store/data/Run2018B/DoubleMuon/MINIAOD/PromptReco-v1/000/317/320/00000/1C222BA8-AF68-E811-93B7-FA163EF4913C.root'
#'/store/data/Run2018A/DoubleMuon/MINIAOD/PromptReco-v3/000/316/569/00000/0CBC961D-6264-E811-B36E-FA163E4C1970.root'
#'/store/data/Run2018D/DoubleMuon/MINIAOD/PromptReco-v2/000/320/757/00000/F2C0E5CD-4D98-E811-A8BF-FA163E987883.root'
#'/store/data/Run2018A/DoubleMuon/MINIAOD/PromptReco-v3/000/316/569/00000/0CBC961D-6264-E811-B36E-FA163E4C1970.root'
#options.inputFiles='/store/data/Run2018A/EGamma/MINIAOD/PromptReco-v1/000/316/219/00000/F4905DCC-C758-E811-8305-FA163E4EC205.root '
#options.inputFiles='/store/data/Run2018A/DoubleMuon/MINIAOD/17Sep2018-v2/00000/0732276A-A475-8D48-AF54-4C5E7B830F4C.root'
#options.inputFiles='/store/mc/RunIIAutumn18MiniAOD/GluGluHToZZTo4L_M125_13TeV_powheg2_JHUGenV7011_pythia8/MINIAODSIM/102X_upgrade2018_realistic_v15-v2/80000/D65A4D51-2E80-AD41-B50D-E4083BA2A668.root'
#' /store/mc/RunIIFall17MiniAOD/ZZTo4L_13TeV_powheg_pythia8/MINIAODSIM/94X_mc2017_realistic_v10_ext1-v1/00000/005E8ACC-A60A-E811-825F-A0369FC522F0.root'
#options.inputFiles='/store/mc/RunIISummer16MiniAODv3/ZZTo4L_13TeV_powheg_pythia8/MINIAODSIM/PUMoriond17_94X_mcRun2_asymptotic_v3-v1/270000/E8AB22C0-D7C6-E811-8EF9-001A649D4925.root'
#options.inputFiles='/store/mc/RunIIFall17MiniAODv2/ZZTo4L_13TeV_powheg_pythia8/MINIAODSIM/PU2017_12Apr2018_94X_mc2017_realistic_v14_ext1-v1/90000/F4E3EBBD-BA42-E811-8154-0025905B85D8.root'
#options.inputFiles='/store/mc/RunIIAutumn18MiniAOD/ZZZJetsTo4L2Nu_4f_TuneCP5_13TeV_amcatnloFXFX_pythia8/MINIAODSIM/102X_upgrade2018_realistic_v15-v1/80000/E45B6B6A-81B4-3D45-ABCF-FF9C118E755F.root'
#options.inputFiles='/store/data/Run2018D/DoubleMuon/MINIAOD/PromptReco-v2/000/320/917/00000/92B9D22F-D19B-E811-909F-FA163E8F1F8F.root'
#options.inputFiles=' /store/mc/RunIIAutumn18MiniAOD/ZZTo4L_TuneCP5_13TeV_powheg_pythia8/MINIAODSIM/102X_upgrade2018_realistic_v15_ext2-v2/90000/DF088D7E-E24C-3C46-B83D-F4B359623203.root'
#options.inputFiles='/store/mc/RunIIAutumn18MiniAOD/GluGluHToZZTo4L_M125_13TeV_powheg2_JHUGenV7011_pythia8/MINIAODSIM/102X_upgrade2018_realistic_v15-v2/270000/E5E2F122-AA57-5248-8177-594EC87DD494.root','/store/mc/RunIIAutumn18MiniAOD/VBF_HToZZTo4L_M125_13TeV_powheg2_JHUGenV7011_pythia8/MINIAODSIM/102X_upgrade2018_realistic_v15-v2/90000/96A5F68D-DCB8-3D4E-8615-919D86D1534F.root','/store/mc/RunIIAutumn18MiniAOD/ttH_HToZZ_4LFilter_M125_13TeV_powheg2_JHUGenV7011_pythia8/MINIAODSIM/102X_upgrade2018_realistic_v15-v2/60000/19B6ADC2-4F62-AA4D-9488-F53CE2936856.root'
#options.outputFile = 'Sync2018/ntuplize_ExtraUsama.root'

options.maxEvents = -1

#print options.inputFiles
#options.register('inputFiles', '', VarParsing.VarParsing.multiplicity.list,VarParsing.VarParsing.varType.string, 'Manual file list input, will query DAS if empty')
options.register('inputFileList', '', VarParsing.VarParsing.multiplicity.singleton, 
                 VarParsing.VarParsing.varType.string, 'Manual file list input, will query DAS if empty')
options.register('channels', "zz",
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.string,
                 "Channel to make ntuples for. May be comma-separated list and/or several presets like 'zz'")
options.register('globalTag', "",
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.string,
                 "Global tag. If empty (default), auto tag is chosen based on isMC")
options.register('isMC', 0,
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.int,
                 "1 if simulation, 0 if data")
options.register('eCalib', 1,
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.int,
                 "1 if electron energy corrections are desired")
options.register('RecomputeElectronID', 0,
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.int,
                 "1 if electron ID needs to be recomputed, use 1 in 2017 MC")
options.register('muCalib', 1,
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
options.register('lheWeights', 0,
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.int,
                 'Add LHE weights from Monte Carlo. Option 1 = scale weights '
                 '(weights 0-9), 2 = scale weights and one set of PDF weights '
                 '(weights 0-111), 3 = all scale and PDF weights. Default 1.')
options.register('datasetName', '',
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.string,
                 "dataset name")
options.register('year', '2016',
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.string,
                 "Which year are you processing. Options: 2016, 2017, 2018")
# add a list of strings for events to process
options.register ('eventsToProcess',  '',
                 VarParsing.VarParsing.multiplicity.list,
                 VarParsing.VarParsing.varType.string,
                 "Events to process")
options.register('calibVFP', '', #allowed = 'pre' or 'post'
                VarParsing.VarParsing.multiplicity.singleton,
                VarParsing.VarParsing.varType.string,
                "If the year provided is 2016 (default) you must specify "
                "if this is preVFP or postVFP by providing 'pre' or 'post'.")
options.parseArguments()

if options.year == "2016":
    #options.inputFiles = '/store/mc/RunIISummer16MiniAODv2/GluGluHToZZTo4L_M125_13TeV_powheg2_JHUGenV709_pythia8/MINIAODSIM/PUMoriond17_80X_mcRun2_asymptotic_2016_TrancheIV_v6-v1/20000/1A5C54BE-BED3-E711-B0A4-44A84224053C.root'
    #options.inputFiles='/store/mc/RunIISummer16MiniAODv2/ZZTo4L_13TeV_powheg_pythia8/MINIAODSIM/PUMoriond17_80X_mcRun2_asymptotic_2016_TrancheIV_v6-v1/120000/221CC46F-2FC6-E611-8FFC-0CC47A1E0488.root'
    #options.inputFiles='file:ZZPowheg2016MiniAOD.root'
    #options.inputFiles='/store/mc/RunIISummer16MiniAODv3/ZZTo4L_13TeV_powheg_pythia8/MINIAODSIM/PUMoriond17_94X_mcRun2_asymptotic_v3-v1/270000/E8AB22C0-D7C6-E811-8EF9-001A649D4925.root'
    #options.inputFiles='/store/mc/RunIISummer16MiniAODv3/GluGluHToZZTo4L_M125_13TeV_powheg2_JHUgenV6_pythia8/MINIAODSIM/PUMoriond17_94X_mcRun2_asymptotic_v3-v1/40000/90B61523-A720-E911-A35B-1866DA890B10.root'
    options.inputFiles='file:qqZZ16phUL_C79571E3-F3DF-3A4C-8B4F-5837AAD31C95.root' #'/store/data/Run2016H/DoubleMuon/MINIAOD/17Jul2018-v1/00000/02359251-628E-E811-BA72-0242AC1C0500.root'
    options.outputFile = 'ntuple2016.root'
    if options.calibVFP.lower() not in ["pre", "post"]:
        print("ERROR: When analyzing 2016 data the option 'calibVFP' must be provided as either 'pre' or 'post'.")
        print("This is to specify preVFP and postVFP for the analysis flow (e.g. lepton calibration)")
        exit(1)
    options.calibVFP = options.calibVFP.lower()

if options.year == "2017":
   # options.inputFiles = '/store/mc/RunIIFall17MiniAODv2/VBF_HToZZTo4L_M125_13TeV_powheg2_JHUGenV7011_pythia8/MINIAODSIM/PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/60000/3450B123-E8BF-E811-B895-FA163E9604CF.root'
    #options.inputFiles = '/store/mc/RunIIFall17MiniAODv2/ZZTo4L_13TeV_powheg_pythia8/MINIAODSIM/PU2017_12Apr2018_94X_mc2017_realistic_v14_ext2-v1/280000/E6CABE1D-2D78-E911-90C6-008CFA1C6458.root'
    #options.inputFiles='/store/mc/RunIIFall17MiniAODv2/VBFHToTauTau_M125_13TeV_powheg_pythia8/MINIAODSIM/PU2017_12Apr2018_new_pmx_94X_mc2017_realistic_v14-v1/280000/FE202169-8B1C-E911-8DF5-0CC47A13CB02.root'
    options.inputFiles ='file:/afs/cern.ch/work/h/hehe/ggZZ_new_files/crab_AllMid_ggZZ012j_MiniAODv2_v3/SMP-RunIIFall17MiniAODv2-99999_1.root'
    options.outputFile = 'ntuple2017.root' #'TestOutput_mediumPUidan50.root'

if options.year == "2018":
    #options.inputFiles = '/store/mc/RunIIAutumn18MiniAOD/ttH_HToZZ_4LFilter_M125_13TeV_powheg2_JHUGenV7011_pythia8/MINIAODSIM/102X_upgrade2018_realistic_v15-v2/60000/DCB7927B-269F-3B4B-9DA3-EFE07A37FC9E.root'
    #options.inputFiles='/store/mc/RunIIAutumn18MiniAOD/ZZTo4L_TuneCP5_13TeV_powheg_pythia8/MINIAODSIM/102X_upgrade2018_realistic_v15_ext2-v2/90000/F9BD3565-7BA4-604B-B7ED-994931E803B4.root'
    options.inputFiles='file:1_miniAOD.root' #'/store/data/Run2018A/SingleMuon/MINIAOD/17Sep2018-v2/00000/11697BCC-C4AB-204B-91A9-87F952F9F2C6.root'
    options.outputFile = 'ntuple2018.root'

genLepChoices =  {"hardProcess" : "isHardProcess()",
        "hardProcessFS" : "fromHardProcessFinalState()",
        "finalstate" : "status() == 1",
        "promptFS" : "isPromptFinalState()",
        "dressedHPFS" : "fromHardProcessFinalState()",
        "dressedFS" : "status() == 1",
        "dressedPromptFS" : "isPromptFinalState()",
}
if options.genLeptonType not in genLepChoices:
    print "ERROR: Invalid GEN-lepton type %s" % options.genLeptonType
    print "Valid optons and corresponding status flags are"
    print "    Format: keyword: status flag"
    print "    Default: hardProcessFS: fromHardProcessFinalState()"
    print "-"*80
    for key, value in genLepChoices.iteritems():
        print "    %s: %s" % (key, value)
    exit(1)

channels = parseChannels(options.channels)
zz = any(len(c) == 4 for c in channels)
zl = any(len(c) == 3 for c in channels)
wz = "wz" in options.channels
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
    if options.year == "2016":
        if options.calibVFP == "pre":
            gt = '106X_mcRun2_asymptotic_preVFP_v11'
        else:
            gt = '106X_mcRun2_asymptotic_v17'
    if options.year == "2017":
        gt = '106X_mc2017_realistic_v10'
    if options.year == "2018" or options.year == "2018Prompt":
        gt = '106X_upgrade2018_realistic_v16_L1v1'
else:
    gt = '106X_dataRun2_v35'
    # Keeping this in just in case, but this doesn't seem to be up-to-date or needed
    if options.year == "2018Prompt":
        gt = '102X_dataRun2_Prompt_v15'
        

print "globalTag: ",gt
process.GlobalTag = GlobalTag(process.GlobalTag, gt)
#pdb.set_trace()
process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.schedule = cms.Schedule()

process.MessageLogger.cerr.FwkReport.reportEvery = 1

#Input source
if len(options.inputFileList) > 0 :
    with open(options.inputFileList) as f :
        inputFiles = list((line.strip() for line in f if line[0] != "#"))
else:
    inputFiles = cms.untracked.vstring(options.inputFiles)
process.source = cms.Source(
    "PoolSource",
    # Avoid problem with excessive memory use in LHERunInfoProduct
    inputCommands = cms.untracked.vstring('keep *', 'drop LHERunInfoProduct_*_*_*'),
    fileNames = cms.untracked.vstring(inputFiles),
    skipEvents = cms.untracked.uint32(options.skipEvents),
    # Example of how to select specific lumi/event
    eventsToProcess = cms.untracked.VEventRange (options.eventsToProcess) 
    #eventsToProcess=1:7991:767126,1:7992:767206,1:7992:767243,1:7999:767830,1:8062:773914,1:8457:811860,1:8458:811922,1:8463:812434,1:8490:815001,1:8491:815098,1:8750:839949,1:8802:845000,1:8806:845309,1:8824:847022,1:8826:847268,1:8835:848108,1:8837:848340,1:8841:848714,1:8917:855988,1:8919:856166,1:8921:856394,1:8921:856409,1:8925:856773,1:8962:860299,1:8964:860546,1:8965:860566,1:8965:860601,1:9025:866354,1:9029:866730,1:9270:889871,1:9273:890182,1:9431:905303,1:9436:905855,1:9468:908863,1:9746:935619,1:9746:935610,1:9749:935841,1:9949:955034,1:9949:955112,1:9950:955154,1:9952:955345,1:9952:955360,1:9954:955537,1:10004:960363,1:8016:769451,1:8019:769746,1:8021:769949,1:8021:769990,1:8519:817806,1:8570:822678,1:8575:823143,1:8590:824609,1:8604:825937,1:8604:825941,1:8606:826178,1:9017:865609,1:9019:865774,1:9021:866002,1:9467:908754,1:10007:960649,1:200:17762,1:2400:213557,1:2400:213567,1:2401:213620,1:2467:219492,1:2469:219725,1:2490:221582,1:2587:230223,1:2833:252129,1:2834:252185,1:2834:252269,1:2835:252297,1:2916:259440,1:2996:266623,1:2997:266660,1:3017:268437,1:3018:268565,1:3101:275931,1:3104:276297,1:3202:284921,1:3204:285131,1:3511:312402,1:3511:312443,1:3512:312507,1:3886:345832,1:4746:422349,1:471:38465,1:471:38470,1:472:38637,1:608:49773,1:608:49752,1:1318:107775,1:1323:108191,1:1324:108284,1:2247:183794,1:2254:184352
    #lumisToProcess = cms.untracked.VLuminosityBlockRange(':457'),
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

# Vertex cleaning removed for WZ
# Note that most analyses seem to prefer not to use this.
# In addition, a condition is made available in the MET filters,
# though in this case the event is rejected if a bad primary vertex is found,
# rather than just filtering out the bad vertex and allowing another to be promoted.

#L1ECALPrefiring module for 2016 and 2017
if options.isMC and (options.year == "2016" or options.year == "2017"):

    from UWVV.Ntuplizer.templates.eventBranches import L1ECALPrefiringBranches  
    extraInitialStateBranches.append(L1ECALPrefiringBranches)
    
    #from PhysicsTools.PatUtils.l1ECALPrefiringWeightProducer_cfi import l1ECALPrefiringWeightProducer
    from PhysicsTools.PatUtils.l1PrefiringWeightProducer_cfi import l1PrefiringWeightProducer

    if options.year == "2016":
        if options.calibVFP == "pre":
            process.prefiringweight = l1PrefiringWeightProducer.clone(
            TheJets = cms.InputTag("updatedPatJetsUpdatedJEC"), #this should be the slimmedJets collection with up to date JECs !
            DataEraECAL = cms.string("UL2016preVFP"),
            DataEraMuon = cms.string("2016preVFP"),
            UseJetEMPt = cms.bool(False),
            PrefiringRateSystematicUnctyECAL = cms.double(0.2),
            PrefiringRateSystematicUnctyMuon = cms.double(0.2)
            )

        else:
            process.prefiringweight = l1PrefiringWeightProducer.clone(
            TheJets = cms.InputTag("updatedPatJetsUpdatedJEC"), #this should be the slimmedJets collection with up to date JECs !
            DataEraECAL = cms.string("UL2016postVFP"),
            DataEraMuon = cms.string("2016postVFP"),
            UseJetEMPt = cms.bool(False),
            PrefiringRateSystematicUnctyECAL = cms.double(0.2),
            PrefiringRateSystematicUnctyMuon = cms.double(0.2)
            )

        '''process.prefiringweight = l1ECALPrefiringWeightProducer.clone(
                DataEra = cms.string("2016BtoH"), #Use 2016BtoH for 2016
                UseJetEMPt = cms.bool(False),
                PrefiringRateSystematicUncty = cms.double(0.2),
                SkipWarnings = False)'''

    if options.year == "2017":
        print "2017 L1Prefiring"
        process.prefiringweight = l1PrefiringWeightProducer.clone(
        TheJets = cms.InputTag("updatedPatJetsUpdatedJEC"), #this should be the slimmedJets collection with up to date JECs !
        DataEraECAL = cms.string("UL2017BtoF"),
        DataEraMuon = cms.string("20172018"),
        UseJetEMPt = cms.bool(False),
        PrefiringRateSystematicUnctyECAL = cms.double(0.2),
        PrefiringRateSystematicUnctyMuon = cms.double(0.2)
        )

        '''process.prefiringweight = l1ECALPrefiringWeightProducer.clone(
                DataEra = cms.string("2017BtoF"), #Use 2016BtoH for 2016
                UseJetEMPt = cms.bool(False),
                PrefiringRateSystematicUncty = cms.double(0.2),
                SkipWarnings = False)'''
    '''
    if options.year == "2018":
        print "2018 L1Prefiring"
        process.prefiringweight = l1PrefiringWeightProducer.clone(
        TheJets = cms.InputTag("updatedPatJetsUpdatedJEC"), #this should be the slimmedJets collection with up to date JECs !
        DataEraECAL = cms.string("None"),
        DataEraMuon = cms.string("20172018"),
        UseJetEMPt = cms.bool(False),
        PrefiringRateSystematicUnctyECAL = cms.double(0.2),
        PrefiringRateSystematicUnctyMuon = cms.double(0.2)
        )
    '''
    process.prefiring = cms.Path(process.prefiringweight)
    process.schedule.append(process.prefiring)

if not wz:
    from UWVV.AnalysisTools.templates.VertexCleaning import VertexCleaning
    FlowSteps.append(VertexCleaning)

# everybody needs basic lepton stuff
from UWVV.AnalysisTools.templates.ElectronBaseFlow import ElectronBaseFlow
FlowSteps.append(ElectronBaseFlow)

from UWVV.AnalysisTools.templates.MuonBaseFlow import MuonBaseFlow
FlowSteps.append(MuonBaseFlow)

# Lepton calibrations
if options.eCalib:
    from UWVV.AnalysisTools.templates.ElectronCalibration import ElectronCalibration
    FlowSteps.append(ElectronCalibration)

if options.muCalib:
    from UWVV.AnalysisTools.templates.MuonCalibration import MuonCalibration
    FlowSteps.append(MuonCalibration)

    from UWVV.Ntuplizer.templates.muonBranches import muonCalibrationBranches
    extraFinalObjectBranches['m'].append(muonCalibrationBranches)

#if options.RecomputeElectronID:
#    from UWVV.AnalysisTools.templates.RecomputeElectronID import RecomputeElectronID
#    FlowSteps.append(RecomputeElectronID)

from UWVV.AnalysisTools.templates.MuonGhostCleaning import MuonGhostCleaning
FlowSteps.append(MuonGhostCleaning)

#Apply Scale Factors downstream with TSelectors after skimming
#from UWVV.AnalysisTools.templates.MuonScaleFactors import MuonScaleFactors
#FlowSteps.append(MuonScaleFactors)
#from UWVV.AnalysisTools.templates.ElectronScaleFactors import ElectronScaleFactors
#FlowSteps.append(ElectronScaleFactors)

from UWVV.AnalysisTools.templates.BadMuonFilters import BadMuonFilters
FlowSteps.append(BadMuonFilters)

# data and MCFM samples never have LHE info
if not options.isMC or 'mcfm' in inputFiles[0].lower() \
        or 'sherpa' in inputFiles[0].lower() \
        or 'phantom' in inputFiles[0].lower():
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

if not wz:
    # FSR and ZZ/HZZ stuff including IDs
    from UWVV.AnalysisTools.templates.ZZFlow import ZZFlow
    FlowSteps.append(ZZFlow)

# make final states
if zz:
    from UWVV.AnalysisTools.templates.ZZInitialStateBaseFlow import ZZInitialStateBaseFlow
    FlowSteps.append(ZZInitialStateBaseFlow)

    if options.hzzExtra:
        # k factors if a gg sample
        if options.isMC and any('GluGlu' in f for f in inputFiles):
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
elif l:
    from UWVV.AnalysisTools.templates.ZZSkim import ZZSkim
    FlowSteps.append(ZZSkim)


if (zz or zl or z) and not wz:
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


# VBS variables for ZZ
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

#Determining 2016 era for use in setupEgammaPostRecoSeq
if options.calibVFP == "pre":
    CalibULera16 = "2016preVFP-UL"
else:
    CalibULera16 = '2016postVFP-UL'

flowOpts = {
    'isMC' : bool(options.isMC),
    'isSync' : bool(options.isMC) and bool(options.isSync),
    'year' : options.year,
    'electronScaleShift' : options.eScaleShift,
    'electronRhoResShift' : options.eRhoResShift,
    'electronPhiResShift' : options.ePhiResShift,
    'muonClosureShift' : options.mClosureShift,
    'CalibULera16' : CalibULera16,
    }

# Turn all these into a single flow class
#pdb.set_trace()
FlowClass = createFlow(*FlowSteps)
flow = FlowClass('flow', process, initialstate_chans=channels, **flowOpts)



### Set up tree makers

# meta info tree first
process.metaInfo = cms.EDAnalyzer(
    'MetaTreeGenerator',
    eventParams = makeEventParams(flow.finalTags()),
    datasetName = cms.string(options.datasetName),
    )
process.metaTreePath = cms.Path(process.metaInfo)
process.schedule.append(process.metaTreePath)

is2016H = 'Run2016H' in inputFiles[0] or "Run2016H" in options.datasetName
is2016G = 'Run2016G' in inputFiles[0] or "Run2016G" in options.datasetName

if wz:
    from UWVV.Ntuplizer.templates.triggerBranches import verboseTriggerBranches
    trgBranches = verboseTriggerBranches
else:
    if options.year == "2016":
        from UWVV.Ntuplizer.templates.triggerBranches import triggerBranches_2016
        trgBranches = triggerBranches_2016
    if options.year == "2017":
        from UWVV.Ntuplizer.templates.triggerBranches import triggerBranches_2017
        trgBranches = triggerBranches_2017
    if options.year == "2018":
        from UWVV.Ntuplizer.templates.triggerBranches import triggerBranches_2018
        trgBranches = triggerBranches_2018

#This can be used for 2017 & 2018. But I'm missing one filter which is outdated in MiniAOD
#The recipe is a bit cumbersome/still work in progress so I don't use this filter for now because I'm lazy and I really don't need these filters
#https://twiki.cern.ch/twiki/bin/view/CMS/MissingETOptionalFiltersRun2#How_to_run_ecal_BadCalibReducedM
if options.isMC:
     if options.year=="2016":
        from UWVV.Ntuplizer.templates.filterBranches import metFilters_2016
        filterBranches = metFilters_2016
     else:
        from UWVV.Ntuplizer.templates.filterBranches import metFilters
        filterBranches = metFilters
else:
     if options.year=="2016":
        from UWVV.Ntuplizer.templates.filterBranches import metAndBadMuonFilters_2016
        filterBranches = metAndBadMuonFilters_2016
     else:
        from UWVV.Ntuplizer.templates.filterBranches import metAndBadMuonFilters
        filterBranches = metAndBadMuonFilters
#I guess if you don't want any extra filters
#filterBranches = trgBranches.clone(trigNames=cms.vstring())

process.treeSequence = cms.Sequence()
# then the ntuples
#pdb.set_trace()
for chan in channels:
    mod = cms.EDAnalyzer(
        'TreeGenerator{}'.format(expandChannelName(chan)),
        src = flow.finalObjTag(chan),
        branches = makeBranchSet(chan, extraInitialStateBranches,
                                 extraIntermediateStateBranches,
                                 **extraFinalObjectBranches),
        eventParams = makeEventParams(flow.finalTags(),chan)
            if not options.isMC else makeEventParams(flow.finalTags(),chan),
        triggers = trgBranches,
        filters = filterBranches,
        )

    setattr(process, chan, mod)
    process.treeSequence += mod

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
    #pdb.set_trace()
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
#pdb.set_trace()
