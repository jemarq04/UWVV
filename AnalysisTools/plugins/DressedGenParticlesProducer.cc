#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "UWVV/DataFormats/interface/DressedGenParticle.h"
#include "CommonTools/Utils/interface/PtComparator.h"

class DressedGenParticlesProducer : public edm::stream::EDProducer<> {
    public:

        DressedGenParticlesProducer(const edm::ParameterSet& cfg);
        virtual ~DressedGenParticlesProducer(){}
        void produce(edm::Event& event, const edm::EventSetup& es);
//        void DressedGenParticlesProducer::disambiguateAssociatesByDR(
//            auto overlap, reco::GenParticleCollection dressed1,
//            reco::GenParticleCollection dressed2);
        void removeDoubleCounting(reco::CandidateCollection& dressedParticles);
        bool allUniqueAssociates(reco::CandidateCollection& dressedParticles);
    private:
        edm::EDGetTokenT<reco::GenParticleCollection> baseCollectionToken_;
        edm::EDGetTokenT<reco::GenParticleCollection> associatesToken_;
        const double dRmax_;
};

DressedGenParticlesProducer::DressedGenParticlesProducer(
        const edm::ParameterSet& cfg):
        baseCollectionToken_(consumes<reco::GenParticleCollection>(
            cfg.getParameter<edm::InputTag>("baseCollection"))),
        associatesToken_(consumes<reco::GenParticleCollection>(
            cfg.getParameter<edm::InputTag>("associates"))),
        dRmax_(cfg.getUntrackedParameter<double>("dRmax", 0.1)) {
    produces<std::vector<DressedGenParticle> >();
}

void DressedGenParticlesProducer::produce(edm::Event& event, const edm::EventSetup& es) {
    std::unique_ptr<std::vector<DressedGenParticle> > dressedCollection = std::make_unique<std::vector<DressedGenParticle> >();

    edm::Handle<reco::GenParticleCollection> baseCollection;
    event.getByToken(baseCollectionToken_, baseCollection);

    edm::Handle<reco::GenParticleCollection> associates;
    event.getByToken(associatesToken_, associates);

    for (const auto& base_particle : *baseCollection) {
        DressedGenParticle dressed_part = DressedGenParticle(base_particle,
            *associates, dRmax_);
        dressedCollection->push_back(dressed_part);
    }
    //std::cout << "All associates were unique? " << allUniqueAssociates(*dressedCollection)
    //          << std::endl;
    //std::sort(dressedCollection->begin(),dressedCollection->end(),
    //    [](const reco::Candidate& part1, const reco::Candidate& part2)
    //        {return part1.pt() > part2.pt();});
        //GreaterByPt<reco::Candidate>());

    event.put(std::move(dressedCollection));
}
bool DressedGenParticlesProducer::allUniqueAssociates(
        reco::CandidateCollection& dressedParticles) {
    std::vector<reco::GenParticle> allAssociated;
    for (const auto& particle : dressedParticles) {
        const DressedGenParticle& dressed_part =
            dynamic_cast<const DressedGenParticle&>(particle);
        reco::GenParticleCollection associated = dressed_part.getAssociated();
        allAssociated.insert(allAssociated.end(),
            associated.begin(), associated.end());
    }
    std::set<reco::GenParticle, GreaterByPt<reco::Candidate>>
        uniqueAssociated(allAssociated.begin(), allAssociated.end());
    if (allAssociated.size() != uniqueAssociated.size()) {
        std::cout << "----------------------------" << std::endl;
        std::cout << "Photons for events with duplicates!" << std::endl;
        for (const auto& assoc : allAssociated) {
            std::cout << " pdgId " << assoc.pdgId() << " pt "
                      << assoc.pt() << " eta " << assoc.eta() << std::endl;
        }
        removeDoubleCounting(dressedParticles);
    }
    return (allAssociated.size() == uniqueAssociated.size());
}
void DressedGenParticlesProducer::removeDoubleCounting(
        reco::CandidateCollection& dressedParticles) {
    for(size_t i = 0; i < dressedParticles.size(); i++)
    {
        const DressedGenParticle& dressed_part_i =
            dynamic_cast<const DressedGenParticle&>(dressedParticles[i]);
        reco::GenParticleCollection associates = dressed_part_i.getAssociated();
        for(size_t j = i + 1; j < dressedParticles.size(); j++) {
            const DressedGenParticle& dressed_part_j =
                dynamic_cast<const DressedGenParticle&>(dressedParticles[j]);
            std::cout << std::endl << "Lepton # " << j;
            std::cout << std::endl << "Associated Photons = " << dressed_part_j.numAssociated();
            std::cout << std::endl << "Lepton kinematics:";
            std::cout << std::endl << " pdgId " << dressed_part_j.pdgId() << " pt "
                        << dressed_part_j.pt() << " eta " << dressed_part_j.eta() << std::endl;
            for (const auto& assoc : associates) {
                std::cout << "Associated = " << dressed_part_j.isAssociated(assoc);
                std::cout << std::endl << " pdgId " << assoc.pdgId() << " pt "
                        << assoc.pt() << " eta " << assoc.eta() << std::endl;
            }
            //auto overlap = std::set_intersection(associates,
            //    dressedParticles[j].getAssociated());
            //disambiguateAssociates(overlap,
            //    dressedParticles[i], dressedParticles[j]);
        }
    }
}
//void DressedGenParticlesProducer::disambiguateAssociatesByDR(auto overlap,
//        reco::GenParticleCollection dressed1,
//        reco::GenParticleCollection dressed2) {
//    for (auto cand : overlap) {
//        if (reco::Candidate::deltaR(cand.p4(), dressed1.undressedp4())
//            > reco::Candidate::deltaR(cand.p4(), dressed2.undressedp4()))
//            dressed1.disocciate(cand);
//        else
//            dressed2.disocciate(cand);
//    }
//}
#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(DressedGenParticlesProducer);
