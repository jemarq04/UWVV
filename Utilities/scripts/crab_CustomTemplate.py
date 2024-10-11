# Modified from N. Smith, U. Wisconsin
from CRABClient.UserUtilities import config #, getUsernameFromSiteDB
import configparser
import os
import re
import subprocess
import sys
import datetime
import glob
import hashlib,pdb

settingsFile = "local.cfg"
if not os.path.exists(settingsFile):
    print("Please copy local.template.cfg to local.cfg and edit as appropriate")
    exit()
localSettings = configparser.ConfigParser()
localSettings.read(settingsFile)

gitDescription = subprocess.check_output(["git", "describe", "--always"]).strip()
gitStatus = subprocess.check_output(["git", "status", "--porcelain", "-uno"])
if gitStatus != "":
    print("\033[33mWARNING: git status is dirty!\033[0m")
    print(gitStatus)
    gitDescription += "*"
print("Git status is %s" % gitDescription)
if "dataset" not in localSettings["local"]:
    raise Exception("Must pass dataset argument as Data.inputDataset=...")
dataset = localSettings.get("local", "dataset")

(_, primaryDS, conditions, dataTier) = dataset.split('/')
if dataTier == 'MINIAOD':
    isMC = 0
    if "Prompt" in conditions:
        isPrompt = 1
    else:
        isPrompt = 0
elif dataTier == 'MINIAODSIM':
    isMC = 1
else:
    raise Exception("Dataset malformed? Couldn't deduce isMC parameter")

postEE = 0
year = localSettings.get("local", "year")
if year == "2022":
    postEE = localSettings.get("local", "postEE")
    print("postEE:%s"%postEE)

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
#config.Data.inputDataset = dataset #commented out since it can not be used along with userInputFiles
with open(localSettings.get("local", "datalist"), "r") as infile:
    config.Data.userInputFiles = infile.readlines()
config.Data.outputDatasetTag = conditions
if (isMC):
    if not postEE:
        globalTag=(localSettings.get("local", "mcGlobalTag"))
    else:
        globalTag=(localSettings.get("local", "postEEGlobalTag"))
elif (isPrompt):
    globalTag=(localSettings.get("local", "PromptdataGlobalTag"))
else: 
    globalTag=(localSettings.get("local", "dataGlobalTag"))
print(globalTag)
print("primaryDS:",primaryDS.lower())
if isMC:
    if any(generator in primaryDS.lower() for generator in ["mcfm", "phantom", "sherpa"]):
        lheWeight=0
    else:
        lheWeight=(localSettings.get("local", "lheWeights"))
else:
    lheWeight=0
print("lheWeights:",lheWeight)
configParams = [
    'isMC=%d' % isMC,
    'isPrompt=%i' % isPrompt,
    'postEE=%i' % postEE,
    'datasetName=%s' % dataset, #Checked the config, shouldn't matter
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
    config.General.requestName = '_'.join([campaign_name, localSettings.get("local", "requestName")])
    # Check for extension dataset, force unique request name
    m = re.match(r".*(_ext[0-9]*)-", conditions)
    if m:
        config.General.requestName += m.groups()[0]
    #config.Data.splitting = 'FileBased'
    #config.Data.unitsPerJob = getUnitsPerJob(primaryDS)
    if postEE:
        config.General.requestName += "postEE"

else:
    # Since a PD will have several eras, add conditions to name to differentiate
    config.General.requestName = '_'.join([campaign_name, primaryDS, conditions])
    #if "Run2016" in conditions:
    #    #2016 JSON
    #    config.Data.lumiMask = '/afs/cern.ch/cms/CAF/CMSCOMM/COMM_DQM/certification/Collisions16/13TeV/Legacy_2016/Cert_271036-284044_13TeV_Legacy2016_Collisions16_JSON.txt'
    #    print("Golden JSON: Cert_271036-284044_13TeV_Legacy2016_Collisions16_JSON.txt")
    if year == "2022":
        #2022 JSON
        config.Data.lumiMask = "%s/src/UWVV/Utilities/scripts/JSON/Cert_Collisions2022_355100_362760_Golden.json" % os.environ["CMSSW_BASE"]
        print("Golden JSON: Cert_Collisions2022_355100_362760_Golden.json")
    else:
        print("What kind of JSON are you running for?")
        exit()
    # Comment out in the (hopefully very rare) case where resubmit needs to 
    # be done manually
    #config.General.requestName = '_'.join([campaign_name, primaryDS, conditions, "resubmit"])
    #config.Data.lumiMask ='crab_%s/results/notFinishedLumis.json' % config.General.requestName 
    
    #config.Data.splitting = 'LumiBased'
    #config.Data.unitsPerJob = getUnitsPerJob(primaryDS)
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
config.JobType.psetName = '%s/src/UWVV/Ntuplizer/test/ntuplize_cfg.py' % os.environ["CMSSW_BASE"]
config.JobType.numCores = 1
config.JobType.inputFiles = ["%s/src/UWVV/data" % os.environ["CMSSW_BASE"]]

config.Data.inputDBS = 'global' if 'USER' not in dataset else 'phys03'
#config.Data.allowNonValidInputDataset = True
config.Data.useParent = False
config.Data.publication = False
# Useful for VBFNLO samples
#config.Site.whitelist = ['T2_DE_DESY']
#config.Site.blacklist = ['T2_ES_IFCA']
config.Data.outLFNDirBase = localSettings.get("local", "outLFNDirBase").replace("$DATE", today)
config.Data.ignoreLocality = False
config.Data.outputPrimaryDataset = primaryDS

config.Site.storageSite = localSettings.get("local", "storageSite")
