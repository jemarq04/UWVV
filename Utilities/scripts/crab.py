# Modified from N. Smith, U. Wisconsin
from CRABClient.UserUtilities import config #, getUsernameFromSiteDB
import ConfigParser
import os
import re
import subprocess
import sys
import datetime
import glob
import hashlib,pdb

settingsFile = "local.txt"
if not os.path.exists(settingsFile):
    print "Please copy local.template.txt to local.txt and edit as appropriate"
    exit()
localSettings = ConfigParser.ConfigParser()
localSettings.read(settingsFile)

gitDescription = subprocess.check_output(["git", "describe", "--always"]).strip()
gitStatus = subprocess.check_output(["git", "status", "--porcelain", "-uno"])
if gitStatus != "":
    print "\033[33mWARNING: git status is dirty!\033[0m"
    print gitStatus
    gitDescription += "*"
print "Git status is %s" % gitDescription
# We have to hack our way around how crab parses command line arguments :<
dataset = 'dummy'
for arg in sys.argv:
    if 'Data.inputDataset=' in arg:
        dataset = arg.split('=')[1]
if dataset == 'dummy':
    raise Exception("Must pass dataset argument as Data.inputDataset=...")

(_, primaryDS, conditions, dataTier) = dataset.split('/')
if dataTier == 'MINIAOD':
    isMC = 0
    if "Prompt" in conditions or "22Jan2019" in conditions: #latter correspond to process string for 2018D in some cases
        isPrompt = 1
    else:
        isPrompt = 0
elif dataTier == 'MINIAODSIM':
    isMC = 1
else:
    raise Exception("Dataset malformed? Couldn't deduce isMC parameter")

isUL = 0
isAPV = 0
if "Summer20UL" in conditions:
    isUL = 1
else:
    isUL = 0
print("isUL:%s"%isUL)

year = localSettings.get("local", "year")
if year == "2016":
    isAPV = localSettings.get("local", "isAPV")
print("isAPV:%s"%isAPV)

def getUnitsPerJob(ds):
    if isMC == 0:
        # Data is split by lumisection
        # The difference is due to trigger rates
        if 'Double' in ds:
            return 150
        elif 'MuonEG' in ds:
            return 150
        elif 'Single' in ds:
            return 80
        else:
            return 150
    else:
        return 20

config = config()
config.Data.inputDataset = dataset
config.Data.outputDatasetTag = conditions
if (isMC):
    if not isAPV:
        globalTag=(localSettings.get("local", "mcGlobalTag"))
    else:
        globalTag=(localSettings.get("local", "APVGlobalTag"))
elif (isPrompt):
    globalTag=(localSettings.get("local", "PromptdataGlobalTag"))
else: 
    globalTag=(localSettings.get("local", "dataGlobalTag"))
print(globalTag)
print ("primaryDS: ",primaryDS.lower())
if isMC:
    if (("mcfm" in primaryDS.lower()) or ("phantom" in primaryDS.lower()) or ("sherpa" in primaryDS.lower())):
        lheWeight=0
    else:
        lheWeight=(localSettings.get("local", "lheWeights"))
else:
    lheWeight=0
print("lheWeights: ",lheWeight)
configParams = [
    'isSync=0',
    #'isSync=%i' % (1 if "WZ" in dataset or "DYJets" in dataset else 0),
    'isMC=%d' % isMC,
    'datasetName=%s' % dataset,
    "year=%s" % year,
    "channels=%s" % localSettings.get("local", "channels"),
    "lheWeights=%s" % lheWeight,
    "genInfo=%s" % localSettings.get("local", "genInfo"),
    "genLeptonType=%s" % localSettings.get("local", "genLeptonType"),
    "eCalib=%s" % localSettings.get("local", "eCalib"),
    "muCalib=%s" % localSettings.get("local", "muCalib"),
    "globalTag=%s" % globalTag,
]
today = (datetime.date.today()).strftime("%d%b%Y")
campaign_name = localSettings.get("local", "campaign").replace("$DATE", today)
#campaign_name = localSettings.get("local", "campaign").replace("$DATE", "25Jan2019")
if isMC:
    config.General.requestName = '_'.join([campaign_name, primaryDS])
    # Check for extension dataset, force unique request name
    m = re.match(r".*(_ext[0-9]*)-", conditions)
    if m:
        config.General.requestName += m.groups()[0]
    #config.Data.splitting = 'FileBased'
    #config.Data.unitsPerJob = getUnitsPerJob(primaryDS)
    if isUL and isAPV:
        config.General.requestName += "preVFP"
    
    if isUL and (not isAPV) and "RunIISummer20UL16MiniAOD" in conditions:
        config.General.requestName += "postVFP"
        
else:
    # Since a PD will have several eras, add conditions to name to differentiate
    config.General.requestName = '_'.join([campaign_name, primaryDS, conditions])
    #config.Data.lumiMask = '/afs/cern.ch/cms/CAF/CMSCOMM/COMM_DQM/certification/Collisions18/13TeV/PromptReco/Cert_314472-325175_13TeV_PromptReco_Collisions18_JSON.txt'
    if "Run2016" in conditions:
        #2016 JSON
        config.Data.lumiMask = '/afs/cern.ch/cms/CAF/CMSCOMM/COMM_DQM/certification/Collisions16/13TeV/ReReco/Final/Cert_271036-284044_13TeV_ReReco_07Aug2017_Collisions16_JSON.txt'
        print "Golden JSON: Cert_271036-284044_13TeV_ReReco_07Aug2017_Collisions16_JSON.txt",
    elif "Run2017" in conditions:
        #2017 JSON
        config.Data.lumiMask = '/afs/cern.ch/cms/CAF/CMSCOMM/COMM_DQM/certification/Collisions17/13TeV/ReReco/Cert_294927-306462_13TeV_EOY2017ReReco_Collisions17_JSON_v1.txt'
        print "Golden JSON: Cert_294927-306462_13TeV_EOY2017ReReco_Collisions17_JSON_v1.txt",
        #config.Data.lumiMask = '/afs/cern.ch/cms/CAF/CMSCOMM/COMM_DQM/certification/Collisions17/13TeV/ReReco/Cert_294927-306462_13TeV_EOY2017ReReco_Collisions17_JSON.txt'
        #print "Golden JSON: Cert_294927-306462_13TeV_EOY2017ReReco_Collisions17_JSON.txt",
    elif "Run2018" in conditions:
        #2018 JSON
        config.Data.lumiMask = '/afs/cern.ch/cms/CAF/CMSCOMM/COMM_DQM/certification/Collisions18/13TeV/ReReco/Cert_314472-325175_13TeV_17SeptEarlyReReco2018ABC_PromptEraD_Collisions18_JSON.txt' 
        print "Golden JSON: Cert_314472-325175_13TeV_17SeptEarlyReReco2018ABC_PromptEraD_Collisions18_JSON.txt",
        #config.Data.lumiMask = '/afs/cern.ch/cms/CAF/CMSCOMM/COMM_DQM/certification/Collisions18/13TeV/PromptReco/Cert_314472-325175_13TeV_PromptReco_Collisions18_JSON.txt' 
        #print "Golden JSON: Cert_314472-325175_13TeV_PromptReco_Collisions18_JSON.txt",
    else:
        print "What kind of JSON are you running for?"
        exit()
    # Comment out in the (hopefully very rare) case where resubmit needs to 
    # be done manually
    #config.General.requestName = '_'.join([campaign_name, primaryDS, conditions, "resubmit"])
    #config.Data.lumiMask ='crab_%s/results/notFinishedLumis.json' % config.General.requestName 
    
    #config.Data.splitting = 'LumiBased'
    #config.Data.unitsPerJob = getUnitsPerJob(primaryDS)
#CRAB server blows up if we run "Automatic splitting" on these DY Datasets so require them to be split "FileBased"
if "DYJetsToLL_M-50" not in primaryDS:
    config.Data.splitting = 'Automatic'
    #config.Data.unitsPerJob = 180
    config.Data.unitsPerJob = 540
else:
    print("Its a DYJetsToLL_M-50 dataset")
    config.Data.splitting = 'FileBased'
    config.Data.unitsPerJob = 1
    
config.Data.totalUnits = -1

# Max requestName is 100 characters
if len(config.General.requestName) > 100:
    bits = 5
    h = hashlib.sha256(config.General.requestName).hexdigest()
    # Replace last 5 characters with hash in case of duplicates after truncation
    config.General.requestName = config.General.requestName[:(100-bits)] + h[:bits]

config.JobType.pyCfgParams = configParams

# Things that don't change with dataset
config.General.workArea = '.'
config.General.transferOutputs = True
config.General.transferLogs = True
#This is a temporary fix to the problem with Automatic splitting in crab
#if "DYJetsToLL_M-50" not in primaryDS:
#config.General.instance = 'preprod'

config.JobType.pluginName = 'ANALYSIS'
config.JobType.allowUndistributedCMSSW = True 
if not isUL:
    config.JobType.psetName = '%s/src/UWVV/Ntuplizer/test/ntuplize_cfg.py' % os.environ["CMSSW_BASE"]
else:
    config.JobType.psetName = '%s/src/UWVV/Ntuplizer/test/ntuplize_cfg_UL.py' % os.environ["CMSSW_BASE"]
config.JobType.numCores = 1
config.JobType.inputFiles = ["%s/src/UWVV/data" % os.environ["CMSSW_BASE"]]

config.Data.inputDBS = 'global' if 'USER' not in dataset else 'phys03'
#config.Data.allowNonValidInputDataset = True
config.Data.useParent = False
config.Data.publication = False
outdir = localSettings.get("local", "outLFNDirBase").replace(
    "$USER", 'marquez').replace("$DATE", today)
#outdir = localSettings.get("local", "outLFNDirBase").replace(
#    "$USER", getUsernameFromSiteDB()).replace("$DATE", "25Jan2019")
# Useful for VBFNLO samples
#config.Site.whitelist = ['T2_DE_DESY']
#config.Site.blacklist = ['T2_ES_IFCA']
config.Data.outLFNDirBase = outdir 
config.Data.ignoreLocality = False

config.Site.storageSite = localSettings.get("local", "storageSite")
