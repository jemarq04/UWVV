from UWVV.AnalysisTools.AnalysisFlowBase import AnalysisFlowBase

import FWCore.ParameterSet.Config as cms


class MuonBaseFlow(AnalysisFlowBase):
    def __init__(self, *args, **kwargs):
        if not hasattr(self, 'debug'):
            self.debug = kwargs.pop('debug', False)
        super(MuonBaseFlow, self).__init__(*args, **kwargs)

    def makeAnalysisStep(self, stepName, **inputs):
        step = super(MuonBaseFlow, self).makeAnalysisStep(stepName, **inputs)
        
        if stepName == 'preselection':
            if self.debug:
                step.addBasicCounter('m', 
                    nMuons="",
                    nPreselMuons="pt > 5 && (isGlobalMuon || isTrackerMuon)",
                    nPtMuons="pt > 5",
                    nGlobalMuons="pt > 5 && isGlobalMuon",
                    nTrackerMuons="pt > 5 && isTrackerMuon",
                    nStandAloneMuons="pt > 5 && isStandAloneMuon",
                    nCaloMuons="pt > 5 && isTrackerMuon",
                    nPFMuons="pt > 5 && isTrackerMuon",
                    nRPCMuons="pt > 5 && isTrackerMuon",
                    nGEMMuons="pt > 5 && isTrackerMuon",
                    nME0Muons="pt > 5 && isTrackerMuon",
                    nTrueMuons="pt > 5 && isMuon",
                )
            step.addBasicSelector('m', 'pt > 5 && (isGlobalMuon || isTrackerMuon)')
        elif stepName == 'embedding':
            self.addMuonPOGIDs(step)

        return step

    def addMuonPOGIDs(self, step):
        '''
        Add Muon POG IDs as UserInts

        '''
        embedMuId = cms.EDProducer(
                "MuonIdEmbedder",
                src = step.getObjTag('m'),
                vertexSrc = step.getObjTag('v')
            )
        step.addModule("muonIDembedding", embedMuId, 'm')

