from UWVV.AnalysisTools.AnalysisFlowBase import AnalysisFlowBase

import FWCore.ParameterSet.Config as cms


class ZZID(AnalysisFlowBase):
    def __init__(self, *args, **kwargs):
        if not hasattr(self, 'year'):
            self.year = kwargs.pop('year', '2016')
        if not hasattr(self, 'debug'):
            self.debug = kwargs.pop('debug', False)

        super(ZZID, self).__init__(*args, **kwargs)

    def makeAnalysisStep(self, stepName, **inputs):
        step = super(ZZID, self).makeAnalysisStep(stepName, **inputs)

        LeptonSetup = cms.string(self.year)
        if stepName == 'embedding':
            if LeptonSetup=="2016":
                eIDEmbedder = cms.EDProducer(
                    "PATElectronZZIDEmbedder",
                    src = step.getObjTag('e'),
                    idLabel = cms.string(self.getZZIDLabel()),
                    vtxSrc = step.getObjTag('v'),
                    #Cuts and IDs differ by year: https://twiki.cern.ch/twiki/bin/viewauth/CMS/HiggsZZ4lRunIILegacy#Electrons
                    #New: Current UL 3 years cuts use https://twiki.cern.ch/twiki/bin/view/CMS/MultivariateElectronIdentificationRun2#HZZ_MVA_training_details_and_wor
                    # The raw BDT cuts have been normalized so they can be used with the normalized values from the BDT using f(x) = 2.0/(1.0 + exp(-2*x)) - 1
                    bdtLabel=cms.string("ElectronMVAEstimatorRun2Summer16ULIdIsoValues"),#Old comments:use fall17v2 instead of custom https://twiki.cern.ch/twiki/bin/view/CMS/EgammaMiniAODV2#ID_information
                    idCutLowPtLowEta = cms.double(0.955799325575), 
                    idCutLowPtMedEta = cms.double(0.947540657027),
                    idCutLowPtHighEta = cms.double(0.928515872106),
                    idCutHighPtLowEta = cms.double(0.327207560829),
                    idCutHighPtMedEta = cms.double(0.246834599508),
                    idCutHighPtHighEta = cms.double(-0.595576281354),
                    missingHitsCut = cms.int32(999),
                )
            if LeptonSetup=="2017":
                eIDEmbedder = cms.EDProducer(
                    "PATElectronZZIDEmbedder",
                    src = step.getObjTag('e'),
                    idLabel = cms.string(self.getZZIDLabel()),
                    vtxSrc = step.getObjTag('v'),
                    bdtLabel=cms.string("ElectronMVAEstimatorRun2Summer17ULIdIsoValues"),
                    idCutLowPtLowEta = cms.double(0.912857745833),
                    idCutLowPtMedEta = cms.double(0.905679236827),
                    idCutLowPtHighEta = cms.double(0.943944057497),
                    idCutHighPtLowEta = cms.double(0.155978805354),
                    idCutHighPtMedEta = cms.double(0.0273863727098),
                    idCutHighPtHighEta = cms.double(-0.553248366549),
                    missingHitsCut = cms.int32(999),
                )
            if LeptonSetup=="2018":
                print "LeptonSetup:",LeptonSetup
                eIDEmbedder = cms.EDProducer(
                    "PATElectronZZIDEmbedder",
                    src = step.getObjTag('e'),
                    idLabel = cms.string(self.getZZIDLabel()),
                    vtxSrc = step.getObjTag('v'),
                    bdtLabel=cms.string("ElectronMVAEstimatorRun2Summer18ULIdIsoValues"),
                    idCutLowPtLowEta = cms.double(0.90442861665), 
                    idCutLowPtMedEta = cms.double(0.909416688565),
                    idCutLowPtHighEta = cms.double(0.944365365981),
                    idCutHighPtLowEta = cms.double(0.196860083999),
                    idCutHighPtMedEta = cms.double(0.0759172099904),
                    idCutHighPtHighEta = cms.double(-0.516913677482),
                    #idCutLowPtLowEta = cms.double(1.49603193295), # EB1_5
                    #idCutLowPtMedEta = cms.double(1.52414154008), # EB2_5
                    #idCutLowPtHighEta = cms.double(1.77694249574), # EE_5
                    #idCutHighPtLowEta = cms.double(0.199463934736), # EB1_10
                    #idCutHighPtMedEta = cms.double(0.076063564084), # EB2_10
                    #idCutHighPtHighEta = cms.double(-0.572118857519), # EE_10
                    missingHitsCut = cms.int32(999),
                )
                #HZZWP = cms.string("mvaEleID-Fall17-iso-V2-wpHZZ"),#2018 version

            mIDEmbedder = cms.EDProducer(
                "PATMuonZZIDEmbedder",
                src = step.getObjTag('m'),
                vtxSrc = step.getObjTag('v'),
                rhoSrc = cms.InputTag("fixedGridRhoFastjetAll"),
                setup = cms.int32(int(self.year)),
                idLabel = cms.string(self.getZZIDLabel()),
                )

            step.addModule("eZZIDEmbedder", eIDEmbedder, 'e')
            step.addModule("mZZIDEmbedder", mIDEmbedder, 'm')
            if self.debug:
                step.addBasicCounter('e', "ZZIDElectronCounting", 
                    nElectrons="",
                    nLooseElectrons='userFloat("%s") > 0.5' % self.getZZIDLabel(),
                    nTightElectrons='userFloat("%sTight") > 0.5' % self.getZZIDLabel(),
                )
                step.addBasicCounter('m', "ZZIDMuonCounting", 
                    nZZIDMuons="",
                    nLooseMuons='userFloat("%s") > 0.5' % self.getZZIDLabel(),
                    nTightMuons='userFloat("%sTight") > 0.5' % self.getZZIDLabel(),
                )

        return step

    def getZZIDLabel(self):
        return 'ZZIDPass'
