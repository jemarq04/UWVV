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

username = "marquez"
settingsFile = "local.cfg"
if not os.path.exists(settingsFile):
    print("Please copy local.template.cfg to local.cfg and edit as appropriate")
    exit()
localSettings = configparser.ConfigParser()
localSettings.read(settingsFile)

#gitDescription = subprocess.check_output(["git", "describe", "--always"]).strip()
#gitStatus = subprocess.check_output(["git", "status", "--porcelain", "-uno"])
#if gitStatus != "":
#    print("\033[33mWARNING: git status is dirty!\033[0m")
#    print(gitStatus)
#    gitDescription += "*"
#print("Git status is %s" % gitDescription)
# We have to hack our way around how crab parses command line arguments :<
customMC = False
for arg in sys.argv:
    if 'Data.inputDataset=' in arg:
        dataset = arg.split('=')[1]
        print("Submitting job for %s" % dataset)
        break
else:
    if "dataset" in localSettings["local"]:
        dataset = localSettings.get("local", "dataset")
        customMC = True
        print("No input dataset provided. Submitting custom job for %s" % dataset)
    else:
        raise Exception("Must pass dataset argument as Data.inputDataset=... or include dataset argument in config file")

(_, primaryDS, conditions, dataTier) = dataset.split('/')
isPrompt = 0
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

if customMC:
    if not isMC:
        raise Exception("Custom jobs can only be submitted for private MC samples!")
    if any(name not in localSettings["local"] for name in ["requestName", "datalist"]):
        raise Exception("Custom jobs require three extra options: dataset, requestName, and datalist.")
    if not os.path.isfile(localSettings.get("local", "datalist")):
        raise Exception("Datalist file not found: %s" % localSettings.get("local", "datalist"))

postEE = postBPix = 0
year = localSettings.get("local", "year")
if year == "2022":
    if "postEE" in localSettings["local"]:
        postEE = int(localSettings.get("local", "postEE"))
    elif not isMC:
        postEE = 1 if any("Run2022%s" % subera in conditions for subera in ["E", "F", "G"]) else 0
    else:
        postEE = 1 if "postEE" in conditions else 0
    print("postEE: %s"%postEE)
elif year == "2023":
    if "postBPix" in localSettings["local"]:
        postBPix = int(localSettings.get("local", "postBPix"))
    elif not isMC:
        postBPix = 1 if "Run2023D" in conditions else 0
    else:
        postBPix = 1 if "postBPix" in conditions else 0
    print("postBPix: %s" % postBPix)

dataPeriod = ""
if not isMC:
    dataPeriod = conditions.split("Run%s" % year)[1][0]
    if year == "2023":
        dataPeriod += "v" + conditions.split("_v")[1][0]
    print("dataPeriod:", dataPeriod)

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
if not customMC:
    config.Data.inputDataset = dataset
else:
    with open(localSettings.get("local", "datalist"), "r") as infile:
        config.Data.userInputFiles = [line for line in infile.readlines() if line and line[0] != "#"]
config.Data.outputDatasetTag = conditions
if (isMC):
    if year == "2022" and postEE:
        globalTag = (localSettings.get("local", "postEEGlobalTag"))
    elif year == "2023" and postBPix:
        globalTag = (localSettings.get("local", "postBPixGlobalTag"))
    else:
        globalTag=(localSettings.get("local", "mcGlobalTag"))
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
    'jetsUL=%s' % localSettings.get("local", "jetsUL", fallback=0),
    'datasetName=%s' % dataset,
    "year=%s" % year,
    "channels=%s" % localSettings.get("local", "channels"),
    "lheWeights=%s" % lheWeight,
    "genInfo=%s" % localSettings.get("local", "genInfo"),
    "genLeptonType=%s" % localSettings.get("local", "genLeptonType"),
    "eCalib=%s" % localSettings.get("local", "eCalib"),
    "muCalib=%s" % localSettings.get("local", "muCalib"),
    "globalTag=%s" % globalTag,
    "postEE=%i" % postEE,
    "postBPix=%i" % postBPix,
]
today = (datetime.date.today()).strftime("%d%b%Y")
campaign_name = localSettings.get("local", "campaign").replace("$DATE", today)
if isMC:
    config.General.requestName = '_'.join([campaign_name, primaryDS if not customMC else localSettings.get("local", "requestName")])
    # Check for extension dataset, force unique request name
    m = re.match(r".*(_ext[0-9]*)-", conditions)
    if m:
        config.General.requestName += m.groups()[0]
    #config.Data.splitting = 'FileBased'
    #config.Data.unitsPerJob = getUnitsPerJob(primaryDS)
    if year == "2022" and postEE:
        config.General.requestName += "postEE"
    elif year == "2023" and postBPix:
        config.General.requestName += "postBPix"
else:
    configParams.append("dataPeriod=%s" % dataPeriod)
    # Since a PD will have several eras, add conditions to name to differentiate
    config.General.requestName = '_'.join([campaign_name, primaryDS, conditions])
    #if "Run2016" in conditions:
    #    #2016 JSON
    #    config.Data.lumiMask = '/afs/cern.ch/cms/CAF/CMSCOMM/COMM_DQM/certification/Collisions16/13TeV/Legacy_2016/Cert_271036-284044_13TeV_Legacy2016_Collisions16_JSON.txt'
    #    print("Golden JSON: Cert_271036-284044_13TeV_Legacy2016_Collisions16_JSON.txt")
    jsonFileName = ""
    if year == "2022":
        #2022 JSON
        jsonFileName = "Cert_Collisions2022_355100_362760_Golden.json"
    elif year == "2023":
        #2023 JSON
        jsonFileName = "Cert_Collisions2023_366442_370790_Golden.json"
    elif year == "2024":
        #2024 JSON
        jsonFileName = "Cert_Collisions2024_378981_386951_Golden.json"
    else:
        print("What kind of JSON are you running for?")
        exit()
    config.Data.lumiMask = "%s/src/UWVV/Utilities/scripts/JSON/%s" % (os.environ["CMSSW_BASE"], jsonFileName)
    print("Golden JSON: %s" % jsonFileName)
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
config.Data.outLFNDirBase = localSettings.get("local", "outLFNDirBase").replace("$USER", username).replace("$DATE", today)
config.Data.ignoreLocality = False
if customMC:
    config.Data.outputPrimaryDataset = primaryDS

config.Site.storageSite = localSettings.get("local", "storageSite")
