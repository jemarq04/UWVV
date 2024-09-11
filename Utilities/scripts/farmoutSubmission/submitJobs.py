#!/usr/bin/env python

'''

Script to set up everything needed for a big submission to the UW cluster and
write a bash script to do the actual submission.

Nate Woods, U. Wisconsin, based on:
    https://github.com/uwcms/FinalStateAnalysis/blob/miniAOD_dev_80X/Utilities/scripts/submit_job.py

'''


import argparse
import os
import sys
from socket import gethostname
import logging
import fnmatch
from UWVV.Utilities.dbsinterface import get_das_info
import datetime
from re import compile as _compileRE


log = logging.getLogger("submitJobs")
logging.basicConfig(level=logging.INFO, stream=sys.stderr)


def writeFarmoutCommand(cfg, jobid, dataset, fullDataset,
                        outdir='srm://cmssrm.hep.wisc.edu:8443/srm/v2/server?SFN=/hdfs/store/{}/{}/{}/',
                        **args):
    '''
    Write the actual farmout command.
    cfg (str): config file to run
    jobid (str): unique ID for this set of jobs
    dataset (str): short name for the dataset
    fullDataset (str): full dataset name with path
    outdir (str): where to put the output
    args: command line arguments
    '''
    user = os.environ['USER']

    if 'uwlogin' in gethostname():
        scratchDir = '/data'
        #Jobs17 = 'ZZ2017Jobs'
    else:
        scratchDir = '/nfs_scratch'

    submitDir = os.path.join(scratchDir, user, jobid, dataset, 'submit')

    cmsRunArgs = list(args.get('cmsRunArgs', []))

    if os.path.exists(submitDir):
        log.warning("Submit directory for sample %s already exists, skipping", dataset)
        return "# Submit directory for sample {} already exists, skipping".format(dataset)

    log.info("Building submit files for sample %s", dataset)

    dagDir = '/' + os.path.join(*(submitDir.split('/')[:-1]+['dags', 'dag']))

    dasFilesCmd = 'file dataset={}'.format(fullDataset)

    #print("submitDir:",submitDir)
    
    #print("dagDir:",dagDir)
    # das throws a lot of exceptions, but they're usually transient, so try a
    # few times if needed
    for i in range(5):
        try:
            dasFiles = get_das_info(dasFilesCmd)
        except RuntimeError as ex:
            continue
        else:
            break
    else:
        raise RuntimeError("Failed to get file list from DAS with exception {}."
                           " Check connection to client.".format(ex.message))

    mkdirCmd = "mkdir -p {}inputs".format(dagDir)
    os.system(mkdirCmd)
    inputListFile = os.path.join(dagDir+"inputs", '{}_inputFiles.txt'.format(dataset))
    with open(inputListFile, 'w') as f:
        f.write('\n'.join(dasFiles))
        #print(dasFiles)
    cmds = [
        'farmoutAnalysisJobs',
        '--infer-cmssw-path',
        '"--submit-dir={}"'.format(submitDir),
        '"--output-dag-file={}"'.format(dagDir),
        '"--output-dir={}"'.format(outdir.format(user=user, jobid=jobid, dataset=dataset)),
        '--input-files-per-job={}'.format(args.get('filesPerJob', 1)),
        '--input-file-list={}'.format(inputListFile),
        '--assume-input-files-exist',
        '--input-dir=/',
        '--max-usercode-size=150',
        ]

    if args.get('extraUsercodeFiles', []):
        cmds.append('--extra-usercode-files="{}"'.format(' '.join(args.get('extraUsercodeFiles'))))

    cmds.append('{}-{}'.format(jobid, dataset))
    cmds.append(cfg)
    cmds.append("'inputFiles=$inputFileNames'")
    cmds.append("'outputFile=$outputFileName'")

    if args.get('applyLumiMask', False):
        lumiMask = args.get('lumiMaskJSON')

        # Prepend with the standard JSON repository. If lumiMask is a full path
        # (starting with '/'), path.join will ignore the first argument.
        defaultJSONPath = '/afs/cern.ch/cms/CAF/CMSCOMM/COMM_DQM/certification/Collisions17/13TeV'
        lumiMask = os.path.join(defaultJSONPath,
                                lumiMask)
        cmds.append('lumiMask={}'.format(lumiMask))

    cmds += cmsRunArgs

    command = '# Submit file for sample {}\n'.format(dataset)
    command += 'mkdir -p {}\n'.format(os.path.dirname(dagDir))
    command += ' '.join(cmds) + '\n'

    return command


def buildScript(cfg, jobid, scriptFile='',
                outdir='srm://cmssrm.hep.wisc.edu:8443/srm/v2/server?SFN=/hdfs/store/user/{}/{}/{}/',
                *samples, **args):
    '''
    Build script to submit jobs to ntuplize all requested samples.
    cfg (str): config file to run
    jobid (str): unique ID for this set of jobs
    scriptFile (str or None): file to write script to, or None for stderr
    outdir (str): where to put the output
    samples(strs): which datasets/samples to use
    args: command line arguments
    '''
    lines = []
    lines.append('# Condor submission script\n')
    lines.append('# Generated with submit_job.py at %s\n' % datetime.datetime.now())
    lines.append('# The command was: %s\n\n' % ' '.join(sys.argv))

    campaign = args.get('campaign', '')
    dataEra = args.get('dataEra', '')

    # mc
    if campaign:
        assert not dataEra, "You can't specify a data era and an MC campaign tag."
        datasetStr = '/*/{}/MINIAODSIM'.format(campaign)
        #print("datasetStr", datasetStr)
        postfixPattern = _compileRE('(?<=_)(?:(?:backup)|(?:ext\\d))')

    # data
    else:
        assert dataEra, "You must specify a data era or an MC campaign tag."
        datasetStr = '/*/{}/MINIAOD'.format(dataEra)

        #print("datasetStr", datasetStr)
        postfixPattern = _compileRE('(?<=Run201\\d[B-F]-)[a-zA-Z0-9_-]*')

    # das throws a lot of exceptions, but they're usually transient, so try a
    # few times if needed
    
    dasDatasetCmd = 'dataset='+datasetStr
    #print(dasDatasetCmd)
    for i in range(5):
        try:
            datasets = get_das_info(dasDatasetCmd)
        except RuntimeError as ex:
            continue
        else:
            break
    else:
        raise RuntimeError("Failed to get dataset list from DAS with "
                           "exception {}. Check connection to "
                           "client.".format(ex.message))

    found = set()
    #print("samples:", samples)
    #print("datasets:",datasets)

    for s in samples:
        matches = [d for d in datasets if fnmatch.fnmatchcase(d.split('/')[1], s)]

        if not matches:
            log.warning("No datasets found matching {}".format(s))
            continue

        for match in matches:
            postfix = postfixPattern.findall(match)
            name = '_'.join([match.split('/')[1]]+postfix)

            if name in found:
                raise ValueError("Two samples named {}.".format(name))
            found.add(name)

            lines.append(writeFarmoutCommand(cfg, jobid, name, match,
                                             outdir, **args))
            lines.append("\n") # Separate each command by new line

    if not lines:
        log.error("No datasets found matching {}, script will be empty!".format(samples))

    if not scriptFile:
        for l in lines:
            print(l)
    else:
        with open(scriptFile, 'w') as f:
            for l in lines:
                f.write(l)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()

    parser.add_argument('jobid', type=str,
                        help='Unique identifier for set of jobs.')
    parser.add_argument('cfg', type=str,
                        help='CMS config file to run.')
    parser.add_argument('--applyLumiMask', action='store_true',
                        help='Pass the appropriate lumi-mask JSON (data only).')
    parser.add_argument('--lumiMaskJSON', type=str,
                        default='ReReco/Cert_294927-306462_13TeV_EOY2017ReReco_Collisions17_JSON.txt', #41.367 /fb (recorded)
                        #default='ReReco/Final/Cert_271036-284044_13TeV_23Sep2016ReReco_Collisions16_JSON.txt', # 36.814/fb
                        #default='Final/Cert_271036-284044_13TeV_PromptReco_Collisions16_JSON.txt', # 36.42
                        #default='Cert_271036-277148_13TeV_PromptReco_Collisions16_JSON.txt', # 15.9/fb
                        help=('Lumi mask JSON. Assumed to be in the standard '
                              'certification area unless a full path is given.'))
    parser.add_argument('--campaign', type=str, default='',
                        help='MC generation campaign for samples (MC only).')
    parser.add_argument('--dataEra', type=str, default='',
                        help='Run over data.')
    parser.add_argument('--samples', nargs='+', type=str, required=False,
                        help='Samples to run. May contain wildcards.')
    parser.add_argument('--outdir', type=str,
                        default='srm://cmssrm.hep.wisc.edu:8443/srm/v2/server?SFN=/hdfs/store/user/{user}/{jobid}/{dataset}/',
                        help = 'Where to put the output.  Default: %(default)s.')
    parser.add_argument('--filesPerJob', type=int, default=1,
                        help="Number of input files to pass to each job.")
    parser.add_argument('--extraUsercodeFiles', nargs='*', type=str,
                        help='List of extra directories that need to be '
                        'included in the user_code tarball sent with the job. '
                        'Paths relative to $CMSSW_BASE.')
    parser.add_argument('-o', type=str, dest="scriptFile", default="", required=False,
                        help='File to write output bash script to (default: stdout).')
    parser.add_argument('cmsRunArgs', nargs=argparse.REMAINDER,
                        help="Arguments to cmsRun.")

    args = parser.parse_args()

    buildScript(args.cfg, args.jobid, args.scriptFile, args.outdir,
                *args.samples, applyLumiMask=args.applyLumiMask,
                lumiMaskJSON=args.lumiMaskJSON, campaign=args.campaign,
                dataEra=args.dataEra, filesPerJob=args.filesPerJob,
                extraUsercodeFiles=args.extraUsercodeFiles,
                cmsRunArgs=args.cmsRunArgs)

