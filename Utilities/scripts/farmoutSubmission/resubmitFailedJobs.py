#!/usr/bin/env python
"""

Allows the resubmission of failed Condor jobs provided they have a rescue dag
file.

Author: Nate Woods, U. Wisconsin, loosely based on the version by D. Austin
Belknap and Devin Taylor

"""

from os import system as _bash
from os import environ as _env
from os.path import join as _join
from re import compile as _reCompile
from glob import glob as _glob
from argparse import ArgumentParser as _ArgParser
from socket import gethostname as _hostname
import logging
from sys import stdout as _stdout

_log = logging.getLogger("resubmitFailedJobs")
logging.basicConfig(level=logging.INFO, stream=_stdout, format="%(message)s")


def resubmit(sample, dryrun=False, quiet=False):
    """
    Check the dag status file of the sample for failed jobs. If any, submit
    the rescue dag files to farmoutAnalysisJobs.
    Sample should be a path to the submit directory.
    """
    statusDag = "%s/dags/dag.status" % sample

    pattern = _reCompile(r"Nodes(?P<status>[A-Za-z]+) = (?P<nNodes>\d+)")

    results = {}
    try:
        with open(statusDag, "r") as f:
            for line in f:
                # we only care about the summary block, which is first
                if "]" in line:
                    break

                match = pattern.search(line)
                if match:
                    results[match.group("status")] = int(match.group("nNodes"))
    except IOError:
        _log.error(
            "Unable to find DAG status file for {} -- did it get submitted?".format(
                sample
            )
        )
        raise

    try:
        total = results["Total"]
        succeeded = results["Done"]
        failed = results["Failed"]
        inProgress = (
            results["Pre"] + results["Post"] + results["Queued"] + results["Ready"]
        )
        ignore = results["Unready"]  # email job or something
    except KeyError:
        _log.error("DAG status file {} is broken somehow".format(statusDag))
        raise

    if failed or not quiet:
        _log.info("    " + sample)
        _log.info(
            "        Total: {0} Done: {1} Queued: {2} Failed: {3}".format(
                total - ignore, succeeded, inProgress, failed
            )
        )

    if inProgress and (failed or not quiet):
        _log.info("        Not done, try again later")
    elif failed:
        _log.info("        Resubmitting...")
        rescue_dag = max(_glob("{}/dags/*dag.rescue[0-9][0-9][0-9]".format(sample)))
        _log.info("        Rescue file: {0}".format(rescue_dag))
        if not dryrun:
            cmd = "farmoutAnalysisJobs --rescue-dag-file={}".format(rescue_dag)
            _bash(cmd)

    return succeeded, failed, inProgress


def generate_submit_dirs(jobids):
    """
    Make a list of submit directories from an input argument.
    If two or more forward slashes ('/') appear in a jobid, it is interpreted
    as a path to a submit directory (which is resubmitted) or directory
    containing submit directories, all of which are resubmitted.
    If there are no forward slashes, it is interpreted as a jobid, and its
    submit directories are found in /<submit_base>/<username>/jobid, where
    <submit_base> is '/data' on uwlogin and '/nfs_scratch' on login0*, and
    all subdirectories are resubmitted.
    If there is exactly one forward slash, it is considered a jobid/sample pair
    and the sample is resubmitted.
    Either way, UNIX-style wildcards are allowed.
    """
    dirs = []

    if "uwlogin" in _hostname():
        scratch = "/data"
    else:
        scratch = "/nfs_scratch"

    user = _env["USER"]

    for job in jobids:
        if job.count("/") > 1:  # full path
            unixPath = job
        else:  # jobid or jobid/sample
            unixPath = _join(scratch, user, job)

        subdirs = _glob("%s/*" % unixPath)
        if any("dags" in s for s in subdirs):  # this is a sample
            dirs += _glob(unixPath)
        else:
            dirs += subdirs

    return dirs


if __name__ == "__main__":
    parser = _ArgParser(description="Resubmit failed Condor jobs")

    parser.add_argument(
        "jobids",
        nargs="+",
        help="Provide the FSA sample(s) in"
        " one of the following formats (UNIX wildcards allowed):\n"
        "jobID \n"
        "jobID/sample \n"
        "/path/to/job/or/submit/directory",
    )

    parser.add_argument(
        "--dry-run",
        dest="dryrun",
        action="store_true",
        help="Show samples to submit without submitting them",
    )
    parser.add_argument(
        "--quiet",
        "-q",
        action="store_true",
        help="Only print information about samples with failed jobs",
    )

    args = parser.parse_args()

    if args.dryrun:
        _log.info(
            "Pretending to resubmit samples from {}".format(", ".join(args.jobids))
        )
    else:
        _log.info("Trying to resubmit samples from {}".format(", ".join(args.jobids)))

    samples = generate_submit_dirs(args.jobids)

    succeeded = 0
    failed = 0
    inProgress = 0
    resubmitted = 0

    for s in samples:
        try:
            succ, fail, prog = resubmit(s, args.dryrun, args.quiet)
        except (KeyError, IOError):
            _log.error("Error in sample {} -- skipping!".format(s))
            continue
        succeeded += succ
        failed += fail
        inProgress += prog
        if not prog:
            resubmitted += fail

    _log.info(
        "Total: {}  Done: {}  In Progress: {}  Failed: {}".format(
            succeeded + failed + inProgress, succeeded, inProgress, failed
        )
    )
    if resubmitted:
        _log.info("Resubmitted: {}".format(resubmitted))
    if inProgress:
        _log.info("Still working -- try again later.")
    if failed + inProgress == 0:
        _log.info("Done!")
