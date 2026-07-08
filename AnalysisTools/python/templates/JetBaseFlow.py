from UWVV.AnalysisTools.AnalysisFlowBase import AnalysisFlowBase
from UWVV.Utilities.helpers import getCorrectionFile

import FWCore.ParameterSet.Config as cms

from PhysicsTools.PatAlgos.tools.jetTools import setupPuppiForPackedPF


class JetBaseFlow(AnalysisFlowBase):
    def __init__(self, *args, **kwargs):
        if not hasattr(self, "isMC"):
            self.isMC = kwargs.pop("isMC", True)
        if not hasattr(self, "year"):
            self.year = kwargs.pop("year", "2022")
        if not hasattr(self, "calibEra22"):
            self.calibEra22 = kwargs.pop("calibEra22", "preEE")
        if not hasattr(self, "calibEra23"):
            self.calibEra23 = kwargs.pop("calibEra23", "preBPix")
        super(JetBaseFlow, self).__init__(*args, **kwargs)

    def makeAnalysisStep(self, stepName, **inputs):
        step = super(JetBaseFlow, self).makeAnalysisStep(stepName, **inputs)

        if stepName == "preliminary":
            # Pileup ID
            # This puts the IDs in the event stream, not an updated jet collection
            if self.year in ["2022", "2023"]:
                # this producer will create a ValueMap<int> filled with the given value,
                # so that all jets receive a 'passing' PUID for 2022-2023
                # (eff. ignoring PUID for these years as recommended)
                self.process.pileupJetIdUpdated = cms.EDProducer(
                    "PATJetValueMapProducer",
                    src=step.getObjTag("j"),
                    intVals=cms.vint32(7),
                    intLabels=cms.vstring("fullId"),
                )
            elif self.year in ["2024", "2025"]:
                puppiLabel, _ = setupPuppiForPackedPF(self.process)
                self.process.load("RecoJets.JetProducers.PileupJetID_cfi")
                self.process.pileupJetIdUpdated = self.process.pileupJetIdPuppi.clone(
                    jets=step.getObjTag("j"),
                    srcConstituentWeights=puppiLabel,
                    inputIsCorrected=True,
                    applyJec=True,
                    vertexes=step.getObjTag("v"),
                )

                self.process.jetPUIDSequence = cms.Sequence(
                    getattr(self.process, puppiLabel) * self.process.pileupJetIdUpdated
                )
                step.addModule(puppiLabel, getattr(self.process, puppiLabel))
            step.addModule("pileupJetIdUpdated", self.process.pileupJetIdUpdated, "puID", puID="fullId")

            jetPUIDEmbedder = cms.EDProducer(
                "PATJetValueMapEmbedder",
                src=step.getObjTag("j"),
                intLabels=cms.untracked.vstring("pileupJetIdUpdated:fullId"),
                intVals=cms.untracked.VInputTag("pileupJetIdUpdated:fullId"),
            )
            step.addModule("jetPUIDEmbedder", jetPUIDEmbedder, "j")

            # Setup/configuration
            yearstring = self.year
            jesConfig = jerConfig = ""
            if self.year == "2022":
                yearstring += "" if self.calibEra22 == "preEE" else "EE"
                jesConfig = "Summer22%s_22Sep2023_V4" % ("" if self.calibEra22 == "preEE" else "EE")
                jerConfig = "Summer22%s_22Sep2023_JRV2" % ("" if self.calibEra22 == "preEE" else "EE")
            elif self.year == "2023":
                yearstring += "" if self.calibEra23 == "preBPix" else "BPix"
                jesConfig = "Summer23%sPrompt23_V4" % ("" if self.calibEra23 == "preBPix" else "BPix")
                jerConfig = "Summer23%sPrompt23_%s_JRV2" % (
                    "" if self.calibEra23 == "preBPix" else "BPix",
                    "RunCv1234" if self.calibEra23 == "preBPix" else "RunD",
                )
            elif self.year == "2024":
                jesConfig = "Summer24Prompt24_V3"
                jerConfig = "Summer24Prompt24_JRV1"
            elif self.year == "2025":
                jesConfig = "Summer24Prompt25_V1"
                jerConfig = "Summer24Prompt25_JRV1"

            scaleFile = getCorrectionFile("JME", yearstring, "jet_jerc.json.gz")
            vetoFile = getCorrectionFile("JME", yearstring, "jetvetomaps.json.gz")
            idFile = getCorrectionFile("JME", yearstring, "jetid.json.gz")

            # Jet energy corrections + uncertainties (JES)
            jetCorrector = cms.EDProducer(
                "PATJetCorrector",
                src=step.getObjTag("j"),
                rhoSrc=cms.InputTag("fixedGridRhoFastjetAll"),
                scaleFile=cms.string(scaleFile),
                config=cms.string(jesConfig),
                isMC=cms.bool(self.isMC),
                algo=cms.string("AK4PFPuppi"),
            )
            if self.isMC:
                step.addModule(
                    "jetCorrectorMC", jetCorrector, "j", "j_jesUp", "j_jesDown", j_jesUp="jesUp", j_jesDown="jesDown"
                )
            else:
                step.addModule("jetCorrectorData", jetCorrector, "j")

            # Gen matching
            if self.isMC:
                patJetGenJetMatch = cms.EDProducer(
                    "GenJetMatcher",  # cut on deltaR; pick best by deltaR
                    src=step.getObjTag("j"),  # RECO jets (any View<Jet> is ok)
                    matched=cms.InputTag("slimmedGenJets"),  # GEN jets  (must be GenJetCollection)
                    mcPdgId=cms.vint32(),  # n/a
                    mcStatus=cms.vint32(),  # n/a
                    checkCharge=cms.bool(False),  # n/a
                    maxDeltaR=cms.double(0.4),  # Minimum deltaR for the match
                    # maxDPtRel   = cms.double(3.0),                  # Minimum deltaPt/Pt for the match (not used in GenJetMatcher)
                    resolveAmbiguities=cms.bool(True),  # Forbid two RECO objects to match to the same GEN object
                    resolveByMatchQuality=cms.bool(
                        False
                    ),  # False = just match input in order; True = pick lowest deltaR pair first
                )

                step.addModule("patJetGenJetMatch", patJetGenJetMatch)  # store RECO/gen jet association in the event

                # Print jet information
                # jetMatchViewerMy = cms.EDAnalyzer('JetMatchViewerMy',src=step.getObjTag('j'),match=cms.InputTag("patJetGenJetMatch"),
                # tag=cms.string(step.getObjTagString('j')+'/after PUJetIDUpdated')
                #              )
                # step.addModule('jetMatchViewerMy',jetMatchViewerMy)

            # UWVV Jet ID
            jetIDEmbedding = cms.EDProducer(
                "PATJetIDEmbedder",
                src=step.getObjTag("j"),
                domatch=cms.bool(self.isMC),
                idFile=cms.string(idFile),
                config=cms.string("AK4PUPPI_Tight"),
                lepveto=cms.string("AK4PUPPI_TightLeptonVeto"),
            )
            step.addModule("jetIDEmbedding", jetIDEmbedding, "j")

            # Apply jet veto map
            jetVetoFilter = cms.EDFilter(
                "PATJetVetoFilter",
                jets=step.getObjTag("j"),
                vetoFile=cms.string(vetoFile),
            )
            step.addModule("jetVetoFilter", jetVetoFilter)

            if self.isMC:
                # UWVV Jet ID (JES Up/Down)
                jetIDEmbedding_jesUp = jetIDEmbedding.clone(src=step.getObjTag("j_jesUp"), domatch=cms.bool(False))
                step.addModule("jetIDEmbeddingJESUp", jetIDEmbedding_jesUp, "j_jesUp")

                jetIDEmbedding_jesDown = jetIDEmbedding.clone(src=step.getObjTag("j_jesDown"), domatch=cms.bool(False))
                step.addModule("jetIDEmbeddingJESDown", jetIDEmbedding_jesDown, "j_jesDown")

                # Jet smearing + uncertainties (JER)
                jetSmearing = cms.EDProducer(
                    "PATJetSmearing",
                    src=step.getObjTag("j"),
                    rhoSrc=cms.InputTag("fixedGridRhoFastjetAll"),
                    scaleFile=cms.string(scaleFile),
                    config=cms.string(jerConfig),
                    systematics=cms.bool(True),
                    algo=cms.string("AK4PFPuppi"),
                )
                step.addModule(
                    "jetSmearing", jetSmearing, "j", "j_jerUp", "j_jerDown", j_jerUp="jerUp", j_jerDown="jerDown"
                )

                jetSmearing_jesUp = jetSmearing.clone(
                    src=step.getObjTag("j_jesUp"),
                    systematics=cms.bool(False),
                )
                step.addModule("jetSmearingJESUp", jetSmearing_jesUp, "j_jesUp")

                jetSmearing_jesDown = jetSmearing.clone(
                    src=step.getObjTag("j_jesDown"),
                    systematics=cms.bool(False),
                )
                step.addModule("jetSmearingJESDown", jetSmearing_jesDown, "j_jesDown")

                # need to re-sort now that we're calibrated
                jSort_jesUp = cms.EDProducer(
                    "PATJetCollectionSorter",
                    src=step.getObjTag("j_jesUp"),
                    function=cms.string("pt"),
                )
                step.addModule("jetSortingJESUp", jSort_jesUp, "j_jesUp")

                jSort_jesDn = cms.EDProducer(
                    "PATJetCollectionSorter",
                    src=step.getObjTag("j_jesDown"),
                    function=cms.string("pt"),
                )
                step.addModule("jetSortingJESDn", jSort_jesDn, "j_jesDown")

                jSort_jerUp = cms.EDProducer(
                    "PATJetCollectionSorter",
                    src=step.getObjTag("j_jerUp"),
                    function=cms.string("pt"),
                )
                step.addModule("jetSortingJERUp", jSort_jerUp, "j_jerUp")

                jSort_jerDn = cms.EDProducer(
                    "PATJetCollectionSorter",
                    src=step.getObjTag("j_jerDown"),
                    function=cms.string("pt"),
                )
                step.addModule("jetSortingJERDn", jSort_jerDn, "j_jerDown")

            # need to re-sort now that we're calibrated
            jSort = cms.EDProducer(
                "PATJetCollectionSorter",
                src=step.getObjTag("j"),
                function=cms.string("pt"),
            )
            step.addModule("jetSorting", jSort, "j")

        elif stepName == "preselection":
            # For now, we're not using the PU ID, but we'll store it in the
            # ntuples later
            selectionString = (
                'pt > 20. && abs(eta) < 4.7 && userFloat("idTight") > 0.5 && (userInt("{}") >= 0||pt>50.)'
            ).format(step.getObjTagString("puID"))

            selectionString2 = (
                'pt > 20. && abs(eta) < 4.7 && userFloat("idTight") > 0.5 && (userInt("{}") >= 7||pt>50.)'
            ).format(step.getObjTagString("puID"))

            extraSelection = (
                " && (abs(eta) < 2.5 || abs(eta) > 3.0 || pt > 50)"  # asserts pt>50 in 2.5 < abs(eta) < 3.0 range
            )
            if self.year in ["2022", "2023"]:
                extraSelection += " && (abs(eta) < 3 || pt > 50)"
            selectionString += extraSelection
            selectionString2 += extraSelection

            if self.isMC:
                step.addBasicSelector(
                    "j", selectionString
                )  # not apply PU id here in order to calculate PU SF multiplication factor
            else:
                step.addBasicSelector("j", selectionString2)
            if self.isMC:
                step.addBasicSelector("j_jesUp", selectionString2)
                step.addBasicSelector("j_jesDown", selectionString2)
                step.addBasicSelector("j_jerUp", selectionString2)
                step.addBasicSelector("j_jerDown", selectionString2)

        return step
