#!/usr/bin/env python3

import argparse
from array import array
import itertools
import math

import ROOT
from PhysicsTools.NanoAODTools.postprocessing.framework.datamodel import Collection
from PhysicsTools.NanoAODTools.postprocessing.framework.eventloop import Module
from PhysicsTools.NanoAODTools.postprocessing.framework.postprocessor import PostProcessor


class GenZZXsecAnalyzer(Module):
    def __init__(self, scale=1.0, useStatusFlags=False, total_units_in_pb=True, bestZ=False):
        # Class customization
        self.writeHistFile = True
        self.scale = scale
        self.useStatusFlags = useStatusFlags
        self.total_units_in_pb = total_units_in_pb
        self.bestZ = bestZ

        self.best_mZ1 = -1
        self.best_mZ2 = -1

        # Sum weights
        self.numEventsTotal = 0
        self.sumWeightsTotal = 0.0
        self.sumWeightsOnShell = [0.0, 0.0, 0.0]
        self.sumWeightsFiducial = [0.0, 0.0, 0.0]

        # Fiducial cuts
        self.z1MinMass = 60
        self.z1MaxMass = 120
        self.z2MinMass = 60
        self.z2MaxMass = 120

        self.lepEtaMax = 2.5
        self.lepLeadingPtMin = 20
        self.lepSubleadingPtMin = 10
        self.lepPtMin = 5

        self.ossfMinMass = 4

    def getZZLeptons(self, leptons):
        zzleptons = []

        min_dMZ = 1e10
        max_z2LepPt = 0
        for zzCand in itertools.permutations(leptons, 4):
            # ensure OSSF pairs
            if zzCand[0].pdgId != -zzCand[1].pdgId:
                continue
            if zzCand[2].pdgId != -zzCand[3].pdgId:
                continue

            dMZ1 = abs((zzCand[0].p4() + zzCand[1].p4()).M() - 91.1876)
            dMZ2 = abs((zzCand[2].p4() + zzCand[3].p4()).M() - 91.1876)
            if dMZ2 < dMZ1:
                continue # get it on next permutation
            z2LepPt = zzCand[2].pt + zzCand[3].pt

            if dMZ1 < min_dMZ or (dMZ1 == min_dMZ and z2LepPt > max_z2LepPt):
                # zCands = [zzCand[0].p4() + zzCand[1].p4(), zzCand[2].p4() + zzCand[3].p4()]
                zzleptons = list(zzCand)
                min_dMZ = dMZ1
                max_z2LepPt = z2LepPt
        return zzleptons

    def selectionOnShell(self):
        z1Pass = self.z1MinMass < self.best_mZ1 < self.z1MaxMass
        z2Pass = self.z2MinMass < self.best_mZ2 < self.z2MaxMass
        return z1Pass and z2Pass

    def selectionFiducial(self, leptons):
        if any(abs(lep.eta) > self.lepEtaMax or lep.pt < self.lepPtMin for lep in leptons):
            return False

        for i in range(len(leptons)):
            for j in range(i + 1, len(leptons)):
                if leptons[i].pdgId == -leptons[j].pdgId and (leptons[i].p4() + leptons[j].p4()).M() < self.ossfMinMass:
                    return False

        leppt = sorted([lep.pt for lep in leptons], reverse=True)
        if leppt[0] < self.lepLeadingPtMin or leppt[1] < self.lepSubleadingPtMin:
            return False

        return True

    def beginJob(self, histFile=None, histDirName=None):
        Module.beginJob(self, histFile, histDirName)

        self.addObject(ROOT.TH1F("h_genWeight", "Generator Weight", 100, 0, 0))
        self.addObject(ROOT.TH1F("h_numZs", "Number of Z Bosons", 10, 0, 10))

        zMassBins = array("d", [0, 2, 4, 7, 10, 15, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120])
        self.addObject(ROOT.TH1F("h_ZZMass", "ZZ Mass", 20, 0, 1000))
        self.addObject(ROOT.TH1F("h_ZMass", "Z Candidate Mass", 16, zMassBins))
        self.addObject(ROOT.TH1F("h_ZMass_2e2m", "Z Candidate Mass", 16, zMassBins))
        self.addObject(ROOT.TH1F("h_ZMass_4l", "Z Candidate Mass", 16, zMassBins))
        self.addObject(ROOT.TH1F("h_ZMass_alt", "Z Candidate Mass", 16, zMassBins))
        self.addObject(ROOT.TH1F("h_ZMass_2e2m_alt", "Z Candidate Mass", 16, zMassBins))
        self.addObject(ROOT.TH1F("h_ZMass_4l_alt", "Z Candidate Mass", 16, zMassBins))

        self.addObject(ROOT.TH1F("h_LepPt", "Lepton Pt", 20, 0, 200))

    def endJob(self):
        Module.endJob(self)

        separator = "---------"
        channels = ["eemm", "mmmm", "eeee"]

        def printXsec(label, sumWeights, unit="pb", scale=1.0):
            scale *= 1.0 if unit == "pb" else 1000.0
            sumWeights *= scale
            # denom = self.numEventsTotal
            denom = self.sumWeightsTotal

            print(f"{label:14} xsec: {sumWeights/denom:.3f} {unit} ({sumWeights:.3f}/{denom:.3f})")

        print(separator)
        unit = "pb" if self.total_units_in_pb else "fb"
        printXsec("Total", self.sumWeightsTotal, unit, self.scale)

        print(separator)
        for i, chan in enumerate(channels):
            printXsec(f"On Shell {chan}", self.sumWeightsOnShell[i], "fb", self.scale)

        print(separator)
        for i, chan in enumerate(channels):
            printXsec(f"Fiducial {chan}", self.sumWeightsFiducial[i], "fb", self.scale)

        print(separator)
        print()

    def analyze(self, event):
        weight = event.genWeight
        # weight = event.Generator_weight

        self.numEventsTotal += 1
        self.sumWeightsTotal += weight
        if weight != 1.0:
            self.h_genWeight.Fill(weight)

        # Define collections
        genparticles = Collection(event, "GenPart")
        if self.useStatusFlags:
            leptons = [
                part for part in genparticles if abs(part.pdgId) in [11, 13] and part.statusflag("isHardProcess")
            ]
        else:
            leptons = [
                part
                for part in genparticles
                if abs(part.pdgId) in [11, 13]
                and part.genPartIdxMother >= 0
                and abs(genparticles[part.genPartIdxMother].pdgId) == 23
            ]

        # Check number of leptons
        if len(leptons) < 4:
            # print(f"Less than four leptons found")
            return False
        elif len(leptons) > 4:
            print("WARNING: Over 4 final state leptons!")
            return False

        # Sort leptons by mother particle and get Z particles
        leptons = sorted(leptons, key=lambda part: part.genPartIdxMother)
        zCands = [genparticles[idx] for idx in list({lep.genPartIdxMother for lep in leptons})]

        # Check number of Zs
        if len(zCands) != 2:
            print("WARNING: More than two Z bosons:", len(zCands))
            return False

        # Sort Zs and leptons by best Z
        if abs(zCands[1].mass - 91.1876) < abs(zCands[0].mass - 91.1876):
            zCands.reverse()
            leptons.reverse()

        # Determine channel
        num_electrons = sum(1 for lep in leptons if abs(lep.pdgId) == 11)
        num_electrons_to_channel = {
            2: 0,
            0: 1,
            4: 2,
        }
        channel = num_electrons_to_channel[num_electrons]

        # Get best Z masses
        if self.bestZ:
            zzleptons = self.getZZLeptons(leptons)
            self.best_mZ1 = (zzleptons[0].p4() + zzleptons[1].p4()).M()
            self.best_mZ2 = (zzleptons[2].p4() + zzleptons[3].p4()).M()
        else:
            self.best_mZ1 = zCands[0].mass
            self.best_mZ2 = zCands[1].mass

        # Fill histograms
        self.h_ZZMass.Fill((zCands[0].p4() + zCands[1].p4()).M())
        for cand in zCands:
            self.h_ZMass.Fill(cand.mass, weight)
            if num_electrons == 2:
                self.h_ZMass_2e2m.Fill(cand.mass, weight)
            else:
                self.h_ZMass_4l.Fill(cand.mass, weight)
        self.h_ZMass_alt.Fill(self.best_mZ1, weight)
        self.h_ZMass_alt.Fill(self.best_mZ2, weight)
        if num_electrons == 2:
            self.h_ZMass_2e2m_alt.Fill(self.best_mZ1, weight)
            self.h_ZMass_2e2m_alt.Fill(self.best_mZ2, weight)
        else:
            self.h_ZMass_4l_alt.Fill(self.best_mZ1, weight)
            self.h_ZMass_4l_alt.Fill(self.best_mZ2, weight)
        for lep in leptons:
            self.h_LepPt.Fill(lep.pt, weight)

        # Apply on-shell cut
        if not self.selectionOnShell():
            return False
        self.sumWeightsOnShell[channel] += weight

        # Apply fiducial cut
        if not self.selectionFiducial(leptons):
            return False
        self.sumWeightsFiducial[channel] += weight

        return True


def main():
    ROOT.PyConfig.IgnoreCommandLineOptions = True

    sample_map = {
        "qqZZ_run2": {
            "files": [
                "root://cmsxrootd.fnal.gov//store/mc/RunIISummer16NanoAODv3/ZZTo4L_13TeV_powheg_pythia8/NANOAODSIM/PUMoriond17_94X_mcRun2_asymptotic_v3-v1/100000/26F9E9B6-33C7-E811-86C7-001A649D4A45.root"
            ],
            "scale": 1.256,
        },
        "ggZZ4e_run2": {
            "files": [
                "root://cmsxrootd.fnal.gov//store/mc/RunIISummer16NanoAODv3/GluGluToContinToZZTo4e_13TeV_MCFM701_pythia8/NANOAODSIM/PUMoriond17_94X_mcRun2_asymptotic_v3-v2/20000/FE68BB95-03FA-E811-A4A1-509A4C72D5CE.root",
            ],
            "scale": 0.00159,
        },
        "ggZZ2e2m_run2": {
            "files": [
                "root://cmsxrootd.fnal.gov//store/mc/RunIISummer16NanoAODv3/GluGluToContinToZZTo2e2mu_13TeV_MCFM701_pythia8/NANOAODSIM/PUMoriond17_94X_mcRun2_asymptotic_v3-v2/110000/B40666DF-A1EB-E811-B01B-001517FB21CC.root",
            ],
            "scale": 0.00319,
        },
        "qqZZ_run3": {
            "files": [
                # "root://cmsxrootd.fnal.gov//store/mc/Run3Summer22NanoAODv12/ZZto4L_TuneCP5_13p6TeV_powheg-pythia8/NANOAODSIM/130X_mcRun3_2022_realistic_v5-v2/2520000/56a9348d-c4ae-4f88-8ba3-502e7dfca8ad.root",
                "root://cmsxrootd.fnal.gov//store/mc/Run3Summer22NanoAODv12/ZZto4L_TuneCP5_13p6TeV_powheg-pythia8/NANOAODSIM/130X_mcRun3_2022_realistic_v5-v2/2520000/1f701563-9fb0-4ef5-be31-9cd38c73dbf1.root"
            ],
            "scale": 1.390,
        },
    }

    parser = argparse.ArgumentParser()
    parser.add_argument("-s", "--sample", choices=sample_map.keys(), required=True, help="sample to calculate")
    parser.add_argument("--maxEvents", type=int, default=10_000, help="maximum number of entries")
    parser.add_argument("--statusFlags", action="store_true", help="identify final state leptons with status flags")
    parser.add_argument("--bestZ", action="store_true", help="determine Z masses from best lepton combination")
    args = parser.parse_args()

    if args.maxEvents < 0:
        args.maxEvents = None

    mod_args = {
        "useStatusFlags": args.statusFlags,
        "bestZ": args.bestZ,
    }

    p = PostProcessor(
        ".",
        sample_map[args.sample]["files"],
        cut=None,
        branchsel=None,
        maxEntries=args.maxEvents,
        modules=[
            GenZZXsecAnalyzer(
                scale=sample_map[args.sample].get("scale", 1.0), total_units_in_pb="ggZZ" not in args.sample, **mod_args
            )
        ],
        noOut=True,
        histFileName=f"histout_{args.sample}.root",
        histDirName=args.sample,
    )
    p.run()


if __name__ == "__main__":
    main()
