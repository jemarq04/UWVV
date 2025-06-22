'''

Merge data files into one file, removing duplicate events. Assumes events are
the same in all data sets, which for bizarre reasons isn't necessarily valid.

Author: Nate Woods, U. Wisconsin

'''

import logging
from rootpy import log as rlog
rlog = rlog['mergeDataFiles']
logging.basicConfig(level=logging.WARNING)
rlog['/rootpy.tree.chain'].setLevel(rlog.WARNING)


from rootpy.io import root_open
from rootpy.tree import Tree, TreeChain

from UWVV.Utilities.helpers import parseChannels

def mergeChannel(channel, fileList):
    '''
    Merge one channel for all files, return merged tree. 
    Should be run with a file open.
    '''
    if isinstance(fileList, str):
        fileList = [fileList]

    chain = TreeChain('{}/ntuple'.format(channel), fileList)
    out = Tree('ntuple')
    out.set_buffer(chain._buffer, create_branches=True)

    found = set()
    
    for ev in chain:
        evID = (ev.run, ev.lumi, ev.evt)
        if evID in found:
            continue
        
        out.fill()
        found.add(evID)

    return out

if __name__ == "__main__":
    from argparse import ArgumentParser
    from glob import glob

    parser = ArgumentParser(description='Merge many data files, removing duplicate events.')
    parser.add_argument('channels', type=str, nargs=1,
                        help='Comma-separated list of channels or channel shorthands.')
    parser.add_argument('input', type=str, nargs=1,
                        help='Comma-separated list of input files. May contain wildcars.')
    parser.add_argument('output', type=str, nargs=1,
                        help='Output file name')

    args = parser.parse_args()

    channels = parseChannels(args.channels)

    infiles = []
    for fset in args.input[0].split(','):
        infiles += glob(fset)

    if not len(infiles):
        raise IOError("No files found matching {}".format(args.input[0]))

    with root_open(args.output[0], 'recreate') as f:
        for c in channels:
            f.cd()
            d = f.mkdir(c)
            d.cd()
            n = mergeChannel(c, infiles)
            n.write()
    # temp = TemporaryFile()
    # 
    # trees = {}
    # for c in channels:
    #     trees[c] = mergeChannel(c, infiles)
    #     
    # with root_open(args.output[0], "recreate") as f:
    #     treesOut = {}
    #     for c,t in trees.iteritems():
    #         treesOut[c] = asrootpy(t.copy_tree(''))
    #     for c,t in treesOut.iteritems():
    #         t.write()
