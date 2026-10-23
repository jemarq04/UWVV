from UWVV.AnalysisTools.AnalysisFlowBase import AnalysisFlowBase

import FWCore.ParameterSet.Config as cms


class ECALBadCalibFilter(AnalysisFlowBase):
    def __init__(self, *args, **kwargs):
        super(ECALBadCalibFilter, self).__init__(*args, **kwargs)

    def makeAnalysisStep(self, stepName, **inputs):
        step = super(ECALBadCalibFilter, self).makeAnalysisStep(stepName, **inputs)

        if stepName == "preliminary":
            self.process.load("RecoMET.METFilters.ecalBadCalibFilter_cfi")
            self.process.load("Geometry.CaloEventSetup.CaloGeometryBuilder_cfi")
            # self.process.load("Geometry.ForwardGeometry.CastorGeometryDBReader_cfi")
            # self.process.load("Geometry.CaloEventSetup.CaloGeometry_cff")

            self.process.CaloGeometryBuilder.SelectedCalos = ["EcalBarrel"]

            mod = self.process.ecalBadCalibFilter.clone(
                EcalRecHitSource=cms.InputTag("reducedEgamma:reducedEBRecHits"),
                ecalMinEt=cms.double(50.0),
                baddetEcal=cms.vuint32([838871812]),
                taggingMode=cms.bool(True),
                debug=cms.bool(False),
            )
            step.addModule("ecalBadCalibReducedMINIAODFilter", mod)

        return step
