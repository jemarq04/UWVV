from UWVV.AnalysisTools.AnalysisFlowBase import AnalysisFlowBase

import FWCore.ParameterSet.Config as cms

from UWVV.Utilities.helpers import parseChannels

class WZLeptonCounters(AnalysisFlowBase):
    def __init__(self, *args, **kwargs):
        super(WZLeptonCounters, self).__init__(*args, **kwargs)

    def makeAnalysisStep(self, stepName, **inputs):
        step = super(WZLeptonCounters, self).makeAnalysisStep(stepName, **inputs)
        
        if stepName == 'initialStateEmbedding':
            muCounters = {
                "WZLooseMuon"   : self.getWZLooseMuonID(),
                "WZMediumMuon"   : self.getWZMediumMuonID(),
                "WZTightMuon"   : self.getWZTightMuonID(),
            }
                         
            mod = cms.EDProducer(
                "PATMuonCounter",
                src = step.getObjTag('m'),
                labels = cms.vstring(*muCounters.keys()),
                cuts = cms.vstring(*muCounters.values()),
            )
            step.addModule("muCounter", mod)

            eCounters = {
                "CBVIDTightElec" : 'pt() > 10 && abs(eta) < 2.5 && userFloat("IsCBVIDTightwIP")',
                "CBVIDMediumElec" :  'pt() > 10 && abs(eta) < 2.5 && userFloat("IsCBVIDMediumwIP")',
                "CBVIDLooseElec" :  'pt() > 10 && abs(eta) < 2.5 && userFloat("IsCBVIDLoosewIP")',
                "CBVIDVetoElec" :  'pt() > 10 && abs(eta) < 2.5 && userFloat("IsCBVIDVetowIP")',
                "CBVIDVetoElecNoIP" :  'pt() > 10 && abs(eta) < 2.5 && userFloat("IsCBVIDVeto")',
                "CBVIDHLTSafeElec" :  'pt() > 10 && abs(eta) < 2.5 && userFloat("IsCBVIDHLTSafewIP")',
                "CBVIDHLTSafeElecNoIP" :  'pt() > 10 && abs(eta) < 2.5 && userFloat("IsCBVIDHLTSafe")',
                "WWLooseCBVIDMedElec" :  'pt() > 10 && abs(eta) < 2.5 && userInt("IsWWLoose") && userFloat("IsCBVIDMediumwIP")',
                "WWLooseElec" :  'pt() > 10 && abs(eta) < 2.5 && userInt("IsWWLoose")',
            }
                         
            mod = cms.EDProducer(
                "PATElectronCounter",
                src = step.getObjTag('e'),
                labels = cms.vstring(*eCounters.keys()),
                cuts = cms.vstring(*eCounters.values()),
            )
            step.addModule("elecCounter", mod)

            counters = {'n'+label : 'muCounter:'+label for label in muCounters.keys()}
            counters.update({'n'+label : 'elecCounter:'+label for label in eCounters.keys()})

            labels = list(counters.keys())
            tags = [cms.InputTag(counters[label]) for label in labels]

            for chan in parseChannels('zl'):
                countEmbedding = cms.EDProducer(
                    'PATCompositeCandidateValueEmbedder',
                    src = step.getObjTag(chan),
                    intLabels = cms.vstring(*labels),
                    intSrc = cms.VInputTag(*tags),
                )
                step.addModule(chan+'CountEmbedding', countEmbedding, chan)

        return step

