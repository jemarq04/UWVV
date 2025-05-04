from UWVV.AnalysisTools.AnalysisFlowBase import AnalysisFlowBase

import FWCore.ParameterSet.Config as cms

from os import environ

class ZZID(AnalysisFlowBase):
    def __init__(self, *args, **kwargs):
        if not hasattr(self, 'year'):
            self.year = kwargs.pop('year', '2022')
        if not hasattr(self, 'debug'):
            self.debug = kwargs.pop('debug', False)
        if not hasattr(self, 'electronsUL'):
            self.electronsUL = kwargs.pop('electronsUL', True)

        super(ZZID, self).__init__(*args, **kwargs)

    def makeAnalysisStep(self, stepName, **inputs):
        step = super(ZZID, self).makeAnalysisStep(stepName, **inputs)

        if stepName == 'embedding':
            if self.year in ["2022", "2023"]:
                # TODO: update 2023 when available
                eIDEmbedder = cms.EDProducer(
                    "PATElectronZZIDEmbedder",
                    src = step.getObjTag('e'),
                    idLabel = cms.string(self.getZZIDLabel()),
                    vtxSrc = step.getObjTag('v'),
                    mvaLabel = cms.string("mvaEleID-Winter22-HZZ-V1"),
                    useMVA = cms.bool(int(environ["CMSSW_VERSION"].split("_")[1]) >= 14 and not self.electronsUL),
                    bdtLabel = cms.string("ElectronMVAEstimatorRun2Summer18ULIdIsoValues"),
                    idCutLowPtLowEta = cms.double(0.9044286167),
                    idCutLowPtMedEta = cms.double(0.9094166886),
                    idCutLowPtHighEta = cms.double(0.9443653660),
                    idCutHighPtLowEta = cms.double(0.1968600840),
                    idCutHighPtMedEta = cms.double(0.0759172100),
                    idCutHighPtHighEta = cms.double(-0.5169136775),
                    missingHitsCut = cms.int32(999),
                    ptCut = cms.double(7.), 
                    etaCut = cms.double(2.5),
                )
            step.addModule("eZZIDEmbedder", eIDEmbedder, 'e')

            mIDEmbedder = cms.EDProducer(
                "PATMuonZZIDEmbedder",
                src = step.getObjTag('m'),
                vtxSrc = step.getObjTag('v'),
                rhoSrc = cms.InputTag("fixedGridRhoFastjetAll"),
                setup = cms.int32(int(self.year)),
                ptCut = cms.double(5.),
                etaCut = cms.double(2.4),
                idLabel = cms.string(self.getZZIDLabel()),
            )
            step.addModule("mZZIDEmbedder", mIDEmbedder, 'm')

            if self.debug:
                step.addBasicCounter('e', "ZZIDElectronCounting",
                    nElectrons="",
                    nLooseElectrons='userFloat("%s") > 0.5' % self.getZZIDLabel(),
                    nTightElectrons='userFloat("%sTight") > 0.5' % self.getZZIDLabel(),
                )
                step.addBasicCounter('m', "ZZIDMuonCounting",
                    nMuons="",
                    nLooseMuons='userFloat("%s") > 0.5' % self.getZZIDLabel(),
                    nTightMuons='userFloat("%sTight") > 0.5' % self.getZZIDLabel(),
                )

        return step

    def getZZIDLabel(self):
        return 'ZZIDPass'
