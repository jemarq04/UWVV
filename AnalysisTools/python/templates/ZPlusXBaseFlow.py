from UWVV.AnalysisTools.AnalysisFlowBase import AnalysisFlowBase
#from UWVV.Utilities.helpers import mapObjects, parseChannels
import FWCore.ParameterSet.Config as cms


class ZPlusXBaseFlow(AnalysisFlowBase):
    def __init__(self, *args, **kwargs):
        if not hasattr(self, 'debug'):
            self.debug = kwargs.pop('debug', False)
        super(ZPlusXBaseFlow, self).__init__(*args, **kwargs)

    def makeAnalysisStep(self, stepName, **inputs):
        step = super(ZPlusXBaseFlow, self).makeAnalysisStep(stepName, **inputs)

        if stepName == 'intermediateStateCreation':
            self.addZCreation(step)
            
        #This is added to run cleaned Jet Collection for channels=z  
        #if stepName == 'initialStateEmbedding':
        #    self.embedCleanedJets(step)
        return step


    def addZCreation(self, step):
        '''
        Add modules to make Z candidates
        '''
        zEEMod = cms.EDProducer(
            'PATCandViewShallowCloneCombiner',
            decay = cms.string('{0}@+ {0}@-'.format(step.getObjTagString('e'))),
            roles = cms.vstring('e1', 'e2'),
            cut = cms.string(self.__class__.getZEECuts()),
            checkCharge = cms.bool(True),
            setPdgId = cms.int32(23),
            )

        zMuMuMod = cms.EDProducer(
            'PATCandViewShallowCloneCombiner',
            decay = cms.string('{0}@+ {0}@-'.format(step.getObjTagString('m'))),
            roles = cms.vstring('m1', 'm2'),
            cut = cms.string(self.__class__.getZMMCuts()),
            checkCharge = cms.bool(True),
            setPdgId = cms.int32(23),
            )

        step.addModule("zEECreation", zEEMod, 'ee')
        step.addModule("zMuMuCreation", zMuMuMod, 'mm')
        if self.debug:
            step.addBasicCounter('ee', 'zEECounting', nElecZs='')
            step.addBasicCounter('mm', 'zMMCounting', nMuZs='')
    
    #This is added to run cleaned Jet Collection for channels=z  
    #def embedCleanedJets(self, step):
    #    '''
    #    Add modules to embed jet collection cleaned leptons 
    #    selected in the initial state object
    #    '''
    #    for chan in parseChannels('z'):
    #        try:
    #            mod = cms.EDProducer(
    #                'CleanedJetCollectionEmbedder',
    #                src = step.getObjTag(chan),
    #                jetSrc = step.getObjTag('j'),
    #                jesUpJetSrc = step.getObjTag('j_jesUp'),
    #                jesDownJetSrc = step.getObjTag('j_jesDown'),
    #                jerUpJetSrc = step.getObjTag('j_jerUp'),
    #                jerDownJetSrc = step.getObjTag('j_jerDown'),
    #            )
    #        except KeyError:
    #            mod = cms.EDProducer(
    #                'CleanedJetCollectionEmbedder',
    #                src = step.getObjTag(chan),
    #                jetSrc = step.getObjTag('j'),
    #            )
    #        step.addModule(chan+'CleanedJetsEmbed', mod, chan)
                    

    @classmethod
    def getZEECuts(cls):
        return ('daughter("e1").pt > 7 && '
                'daughter("e2").pt > 7 && '
                'abs(daughter("e1").eta) < 2.5 && '
                'abs(daughter("e2").eta) < 2.5')


    @classmethod
    def getZMMCuts(cls):
        return ('daughter("m1").pt > 5 && '
                'daughter("m2").pt > 5 && '
                'abs(daughter("m1").eta) < 2.4 && '
                'abs(daughter("m2").eta) < 2.4')


class ZPlusXBaseFlowGen(ZPlusXBaseFlow):
    @classmethod
    def getZEECuts(cls):
        return ('daughter("e1").pt > 5 && '
                'daughter("e2").pt > 5 && '
                'abs(daughter("e1").eta) < 2.5 && '
                'abs(daughter("e2").eta) < 2.5')


    @classmethod
    def getZMMCuts(cls):
        return ('daughter("m1").pt > 5 && '
                'daughter("m2").pt > 5 && '
                'abs(daughter("m1").eta) < 2.5 && '
                'abs(daughter("m2").eta) < 2.5')
