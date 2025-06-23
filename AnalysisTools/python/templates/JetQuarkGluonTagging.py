from UWVV.AnalysisTools.AnalysisFlowBase import AnalysisFlowBase
from UWVV.Utilities.helpers import UWVV_BASE_PATH

import FWCore.ParameterSet.Config as cms
from os import path

class JetQuarkGluonTagging(AnalysisFlowBase):
    def __init__(self, *args, **kwargs):
        super(JetQuarkGluonTagging, self).__init__(*args, **kwargs)

    def makeAnalysisStep(self, stepName, **inputs):
        step = super(JetQuarkGluonTagging, self).makeAnalysisStep(stepName, **inputs)

        if stepName == 'preliminary': 
            self.process.load("CondCore.CondDB.CondDB_cfi")

            # Make Q/G tag ValueMap
            QGPoolDBESSource = cms.ESSource(
                "PoolDBESSource",
                self.process.CondDB,
                toGet = cms.VPSet(
                    cms.PSet(
                        record = cms.string('QGLikelihoodRcd'),
                        tag = cms.string('QGLikelihoodObject_80X_AK4PFchs'),
                        label = cms.untracked.string('QGL_AK4PFchs'),
                        ),
                    ),
                )

            # Use this version to get it from the Frontier database
            # frontierConnection = 'frontier://FrontierProd/CMS_CONDITIONS'
            # QGPoolDBESSource.connect = cms.string(frontierConnection)
            
            # Use this version to get it from a local db file
            dbPath = 'sqlite_file:' + path.join(UWVV_BASE_PATH, 'data', 
                                                'QuarkGluonTagging', 
                                                'QGL_80X.db')
            QGPoolDBESSource.connect = cms.string(dbPath)

            step.addModule('QGPoolDBESSource', QGPoolDBESSource)

            self.process.es_prefer_qg = cms.ESPrefer('PoolDBESSource', 'QGPoolDBESSource')

            self.process.load('RecoJets.JetProducers.QGTagger_cfi')
            self.process.QGTagger.srcJets = step.getObjTag('j')
            self.process.QGTagger.jetsLabel = cms.string('QGL_AK4PFchs')
            step.addModule('QGTagger', self.process.QGTagger)

            embedQGLikelihood = cms.EDProducer(
                "PATJetValueMapEmbedder",
                src = step.getObjTag('j'),
                floatLabels = cms.untracked.vstring(self.qgLikelihoodLabel()),
                floatVals = cms.untracked.VInputTag("QGTagger:qgLikelihood"),
                )
            step.addModule("qgLikelihoodEmbedding", embedQGLikelihood, 'j')            

        return step

    
    def qgLikelihoodLabel(self):
        return "qgLikelihood"

        
