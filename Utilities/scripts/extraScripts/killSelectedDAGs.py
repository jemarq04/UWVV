from re import compile
from os import system, environ
from os.path import join, exists
from socket import gethostname
from glob import glob
import logging
from sys import stderr


log = logging.getLogger("killSelectedDAGs")
logging.basicConfig(level=logging.INFO, stream=stderr,
                    format='%(message)s')

rePattern = r'000 \((?P<cluster>\d+)\.000\.000\) \d\d/\d\d \d\d:\d\d:\d\d Job submitted from host: <\S*>'
clusterNumRegEx = compile(rePattern)

def killOneJob(clusterNum):
    system('condor_rm {}'.format(clusterNum))


def getCluster(sampleDir):
    '''
    Given the directory with all the DAG-related files, get the cluster ID
    of the corresponding DAGMAN. Returns 0 if the log file is missing.
    '''
    logFile = join(sampleDir, 'dags', 'dag.dagman.log')

    # if everything is done, there will be a "metrics" file and nothing
    # for us to do
    metricsFile = join(sampleDir, 'dags', 'dag.metrics')
    if exists(metricsFile):
        log.info('jobs from {} already done, nothing to do'.format(logFile))
        return 0

    if not exists(logFile):
        log.warning('Can\'t find log file {}'.format(logFile))
        return 0

    with open(logFile, 'r') as f:
        for line in f:
            result = clusterNumRegEx.match(line)
            if result is not None:
                return result.group('cluster')

    log.warning('Cluster number not found in log file {}'.format(logFile))
    return 0


def killSelectedDAGs(*paths, **kwargs):
    dryRun = kwargs.pop('dryRun',False)

    if 'uwlogin' in gethostname():
        scratchDir = '/data'
    else:
        scratchDir = '/nfs_scratch'
    username = environ['USER']

    basePath = join(scratchDir, username)

    for path in paths:
        fullPath = join(basePath, path) # basepath ignored if path is absolute

        for p in glob(fullPath):
            cluster = getCluster(p)

            if not cluster:
                log.warning('Unable to kill dag in {}, skipping'.format(p))
                continue

            if dryRun:
                log.info("Would kill {} with cluster ID {} if this weren't a dry run".format('/'.join(p.split('/')[-2:]), cluster))
                continue

            killOneJob(cluster)


if __name__ == '__main__':
    from argparse import ArgumentParser

    parser = ArgumentParser()
    parser.add_argument('--dryRun', '--dry-run',
                        action='store_true',
                        help='Print what processes would be killed instead of actually killing them')
    parser.add_argument('paths', type=str, nargs='+',
                        help='Paths to sample directories (including wildcards). '
                        'May be absolute paths or relative to /data/username or /nfs_scratch/username.')

    args = parser.parse_args()

    killSelectedDAGs(*args.paths, dryRun=args.dryRun)
