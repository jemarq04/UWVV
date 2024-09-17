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
            #TODO: Determine Run3 HZZ4l E Selections
            '''
            if LeptonSetup=="2016":
                eIDEmbedder = cms.EDProducer(
                    "PATElectronZZIDEmbedder",
                    src = step.getObjTag('e'),
                    idLabel = cms.string(self.getZZIDLabel()),
                    vtxSrc = step.getObjTag('v'),
                    #Cuts and IDs differ by year: https://twiki.cern.ch/twiki/bin/viewauth/CMS/HiggsZZ4lRunIILegacy#Electrons
                    #New: Current UL 3 years cuts use https://twiki.cern.ch/twiki/bin/view/CMS/MultivariateElectronIdentificationRun2#HZZ_MVA_training_details_and_wor
                    bdtLabel=cms.string("ElectronMVAEstimatorRun2Summer16ULIdIsoValues"),#Old comments:use fall17v2 instead of custom https://twiki.cern.ch/twiki/bin/view/CMS/EgammaMiniAODV2#ID_information
                    idCutLowPtLowEta = cms.double(1.8949071018), 
                    idCutLowPtMedEta = cms.double(1.80714210202),
                    idCutLowPtHighEta = cms.double(1.64751528517),
                    idCutHighPtLowEta = cms.double(0.339697782473),
                    idCutHighPtMedEta = cms.double(0.252039219555),
                    idCutHighPtHighEta = cms.double(-0.686263559006),
                    missingHitsCut = cms.int32(999),
                    ptCut = cms.double(5.), 
                )
            if LeptonSetup=="2017":
                eIDEmbedder = cms.EDProducer(
                    "PATElectronZZIDEmbedder",
                    src = step.getObjTag('e'),
                    idLabel = cms.string(self.getZZIDLabel()),
                    vtxSrc = step.getObjTag('v'),
                    bdtLabel=cms.string("ElectronMVAEstimatorRun2Summer17ULIdIsoValues"),
                    idCutLowPtLowEta = cms.double(1.54440585808),
                    idCutLowPtMedEta = cms.double(1.50294621563),
                    idCutLowPtHighEta = cms.double(1.77306202112),
                    idCutHighPtLowEta = cms.double(0.157262554087),
                    idCutHighPtMedEta = cms.double(0.0273932225081),
                    idCutHighPtHighEta = cms.double(-0.623050463489),
                    missingHitsCut = cms.int32(999),
                    ptCut = cms.double(5.), 
                )
            if LeptonSetup=="2018":
                print("LeptonSetup:", LeptonSetup)
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
            '''

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
