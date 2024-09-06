from UWVV.AnalysisTools.AnalysisFlowBase import AnalysisFlowBase
from UWVV.Utilities.helpers import parseChannels
import FWCore.ParameterSet.Config as cms

# From https://twiki.cern.ch/twiki/bin/viewauth/CMS/MissingETOptionalFiltersRun2
class BadMuonFilters(AnalysisFlowBase):
    def __init__(self, *args, **kwargs):
        super(BadMuonFilters, self).__init__(*args, **kwargs)

    def makeAnalysisStep(self, stepName, **inputs):
        step = super(BadMuonFilters, self).makeAnalysisStep(stepName, **inputs)
        
        if stepName == 'initialStateEmbedding': #For UL, access filters directly from miniAOD, and embed the bools like before
            
            #from RecoMET.METFilters.BadPFMuonFilter_cfi import BadPFMuonFilter 
            #BadPFMuonFilter.muons = step.getObjTag('m') 
            #BadPFMuonFilter.PFCandidates = cms.InputTag("packedPFCandidates")
            #step.addModule("BadPFMuonFilter", BadPFMuonFilter)
            
            #from RecoMET.METFilters.BadChargedCandidateFilter_cfi import BadChargedCandidateFilter
            #BadChargedCandidateFilter.muons = step.getObjTag('m') 
            #BadChargedCandidateFilter.PFCandidates = cms.InputTag("packedPFCandidates")
            #step.addModule("BadChargedCandidateFilter", BadChargedCandidateFilter)

            channels = inputs.pop('initialstate_chans', [])
            for chan in channels:
                filterEmbedding = cms.EDProducer(
                    'PATCompositeCandidateValueEmbedder',
                    src = step.getObjTag(chan),
                    boolLabels = cms.vstring("Flag_BadPFMuonFilterPass", "Flag_BadChargedCandidateFilterPass"),
                    replaceFlag = cms.bool(True),
                    #boolSrc = cms.VInputTag("BadPFMuonFilter", "BadChargedCandidateFilter"),
                    )
                step.addModule(chan+'filterEmbedding', filterEmbedding, chan)

        return step
