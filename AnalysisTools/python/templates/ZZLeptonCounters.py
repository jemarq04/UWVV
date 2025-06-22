from UWVV.AnalysisTools.AnalysisFlowBase import AnalysisFlowBase

from UWVV.Utilities.helpers import parseChannels, getObjName

import FWCore.ParameterSet.Config as cms


class ZZLeptonCounters(AnalysisFlowBase):
    def __init__(self, *args, **kwargs):
        super(ZZLeptonCounters, self).__init__(*args, **kwargs)


    def makeAnalysisStep(self, stepName, **inputs):
        step = super(ZZLeptonCounters, self).makeAnalysisStep(stepName, **inputs)
        
        if stepName == 'initialStateEmbedding':
            counters = {
                'Loose' : 'userFloat("{}") > 0.5'.format(self.getZZIDLabel()),
                'Tight' : 'userFloat("{}") > 0.5'.format(self.getZZIDLabel()+'Tight'),
                'TightIso' : ('userFloat("{}") > 0.5 && '
                              'userFloat("{}") > 0.5').format(self.getZZIDLabel()+'Tight', 
                                                              self.getZZIsoLabel())
                }
            countTags = []
            countLabels = []
            for lep in 'e','m':
                labels = ['nZZ'+name+getObjName(lep, True)+'s' for name in counters.keys()]
                countLabels += labels
                mod = cms.EDProducer(
                    "PAT{}Counter".format(getObjName(lep, True)),
                    src = step.getObjTag(lep),
                    labels = cms.vstring(*labels),
                    cuts = cms.vstring(*[counters[label] for label in counters.keys()]),
                    )
                moduleName = 'zz{0}Counter'.format('Elec' if lep == 'e' else 'Mu')
                step.addModule(moduleName, mod)

                countTags += [cms.InputTag('{name}:{label}'.format(name=moduleName,label=lab)) for lab in labels]

            for chan in parseChannels('zz')+parseChannels('zl')+parseChannels('z'):
                if chan not in step.outputs:
                    continue
                countEmbedding = cms.EDProducer(
                    'PATCompositeCandidateValueEmbedder',
                    src = step.getObjTag(chan),
                    intLabels = cms.vstring(*countLabels),
                    intSrc = cms.VInputTag(*countTags),
                    )
                step.addModule(chan+'ZZCountEmbedding', countEmbedding, chan)

        return step

