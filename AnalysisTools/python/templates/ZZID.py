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
            self.electronsUL = kwargs.pop('electronsUL', False)

        super(ZZID, self).__init__(*args, **kwargs)

    def makeAnalysisStep(self, stepName, **inputs):
        step = super(ZZID, self).makeAnalysisStep(stepName, **inputs)

        if stepName == 'embedding':
            # TODO: update MVA for 2023-2025 when available
            eIDEmbedder = cms.EDProducer(
                "PATElectronZZIDEmbedder",
                src = step.getObjTag('e'),
                idLabel = cms.string(self.getZZIDLabel()),
                vtxSrc = step.getObjTag('v'),
                mvaLabel = cms.string("mvaEleID-Winter22-HZZ-V1" if not self.electronsUL else "mvaEleID-Summer18UL-ID-ISO-HZZ"),
                ptCut = cms.double(7.),
                etaCut = cms.double(2.5),
            )
            step.addModule("eZZIDEmbedder", eIDEmbedder, 'e')

            mIDEmbedder = cms.EDProducer(
                "PATMuonZZIDEmbedder",
                src = step.getObjTag('m'),
                vtxSrc = step.getObjTag('v'),
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
