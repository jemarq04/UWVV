#ifndef DressedGenParticle_h
#define DressedGenParticle_h

#include "UWVV/DataFormats/interface/DressedGenParticleFwd.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"

class DressedGenParticle : public reco::GenParticle {
    public:
        DressedGenParticle() {}
        virtual ~DressedGenParticle();
        DressedGenParticle(const LeafCandidate & c) :
            reco::GenParticle(c) { }
        DressedGenParticle(const reco::GenParticle & c) :
            reco::GenParticle(c) { }
        DressedGenParticle(const reco::GenParticle & cand,
            //const reco::GenParticleRefVector associates, float dRmax);
            const reco::GenParticleCollection associates, float dRmax);
        DressedGenParticle(Charge q, const LorentzVector & p4, const Point & vtx,
            int pdgId, int status, bool integerCharge);
        DressedGenParticle(Charge q, const PolarLorentzVector & p4, const Point & vtx,
            int pdgId, int status, bool integerCharge);
        DressedGenParticle* clone() const;
        const LorentzVector undressedP4() const;
        float undressedPt() const;
        float undressedEta() const;
        float undressedPhi() const;
        float numAssociated() const;
        bool isAssociated(const reco::GenParticle& associated) const;
        reco::GenParticleCollection getAssociated() const;
        bool dissociate(const reco::GenParticle& associated);
    private:
        void dressParticle();
        reco::GenParticleCollection associates;
        const LorentzVector p4_undressed;
};

#endif
