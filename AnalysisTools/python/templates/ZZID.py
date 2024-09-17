from UWVV.AnalysisTools.AnalysisFlowBase import AnalysisFlowBase

import FWCore.ParameterSet.Config as cms


class ZZID(AnalysisFlowBase):
    def __init__(self, *args, **kwargs):
        if not hasattr(self, 'year'):
            self.year = kwargs.pop('year', '2022')

        super(ZZID, self).__init__(*args, **kwargs)

    def makeAnalysisStep(self, stepName, **inputs):
        step = super(ZZID, self).makeAnalysisStep(stepName, **inputs)

        LeptonSetup = cms.string(self.year)
        if stepName == 'embedding':
            pass
            #TODO: Determine Run3 HZZ4l E Selections. For now use 2018UL WPs
            if LeptonSetup=="2022":
                eIDEmbedder = cms.EDProducer(
                    "PATElectronZZIDEmbedder",
                    src = step.getObjTag('e'),
                    idLabel = cms.string(self.getZZIDLabel()),
                    vtxSrc = step.getObjTag('v'),
                    bdtLabel=cms.string("ElectronMVAEstimatorRun2Summer18ULIdIsoValues"),
                    idCutLowPtLowEta = cms.double(1.49603193295), 
                    idCutLowPtHighEta = cms.double(1.77694249574),
                    idCutHighPtLowEta = cms.double(0.199463934736),
                    idCutHighPtMedEta = cms.double(0.076063564084),
                    idCutHighPtHighEta = cms.double(-0.572118857519),
                    missingHitsCut = cms.int32(999),
                    ptCut = cms.double(5.), 
                )
                #HZZWP = cms.string("mvaEleID-Fall17-iso-V2-wpHZZ"),#2018 version
            step.addModule("eZZIDEmbedder", eIDEmbedder, 'e')

            mIDEmbedder = cms.EDProducer(
                "PATMuonZZIDEmbedder",
                src = step.getObjTag('m'),
                vtxSrc = step.getObjTag('v'),
                rhoSrc = cms.InputTag("fixedGridRhoFastjetAll"),
                setup = cms.int32(int(self.year)),
                ptCut = cms.double(3.),
                idLabel = cms.string(self.getZZIDLabel()),
                )
            step.addModule("mZZIDEmbedder", mIDEmbedder, 'm')

        return step

    def getZZIDLabel(self):
        return 'ZZIDPass'
