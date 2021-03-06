#include <memory>
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDFilter.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"


#include "DataFormats/PatCandidates/interface/GenericParticle.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include <vector>

//typedef std::vector<pat::Muon> MuonList;
typedef pat::GenericParticle myTrack;
typedef std::vector<myTrack> myTrackList;
typedef pat::Muon myMuon;
typedef std::vector<myMuon> myMuonList;


//------------------------------------------------------------------------------
//   Class Definition
//------------------------------------------------------------------------------
class TrackProducer : public edm::stream::EDFilter<> {
public:
   explicit TrackProducer(const edm::ParameterSet&);
   ~TrackProducer();
   static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

   // Public for use !
   bool IsSelectedTrack( const myTrack& ) const ;
   bool IsVetoTrack( const myTrack& ) const;
   bool IsMuonTrack( const myTrack& tk, const myMuonList& muList ) const;
private:
   virtual bool filter(edm::Event&, const edm::EventSetup&) override;

   // Data Members
   //const edm::EDGetTokenT<reco::BeamSpot>   _bsToken;
   const edm::EDGetTokenT<myMuonList>       _muonToken;
   const edm::EDGetTokenT<myTrackList>      _tkToken;

   //edm::Handle<reco::BeamSpot>              _bsHandle;
   edm::Handle<myMuonList>                  _muonHandle;
   edm::Handle<myTrackList>                 _tkHandle;


};

//------------------------------------------------------------------------------
//   Constructor and destructor
//------------------------------------------------------------------------------

TrackProducer::TrackProducer(const edm::ParameterSet& iConfig):
    //_bsToken( consumes<reco::BeamSpot>(iConfig.getParameter<edm::InputTag>("bssrc"))),
    _muonToken( consumes<myMuonList>(iConfig.getParameter<edm::InputTag>("muonsrc"))),
    _tkToken( consumes<myTrackList>(iConfig.getParameter<edm::InputTag>("tracksrc")))
{
   produces<myTrackList>();
}

TrackProducer::~TrackProducer()
{
}

//------------------------------------------------------------------------------
//   Main Control Flow
//------------------------------------------------------------------------------
bool TrackProducer::filter( edm::Event& iEvent, const edm::EventSetup& iSetup)
{
    //iEvent.getByToken( _bsToken, _bsHandle );
    iEvent.getByToken( _muonToken, _muonHandle);
    iEvent.getByToken( _tkToken, _tkHandle );
    //const reco::BeamSpot& bs = *_bsHandle;
    const myMuonList muList = *_muonHandle;

    std::unique_ptr<myTrackList> selectedTracks( new myTrackList );
    selectedTracks->reserve( _tkHandle->size() );

    // Object selection
    for ( const myTrack& tk : *( _tkHandle.product() ) )
    {
        if ( IsVetoTrack(tk) ) continue;
        if ( IsMuonTrack(tk, muList) ) continue;
        if ( IsSelectedTrack(tk) )
            selectedTracks->push_back(tk);
    }
    if ( selectedTracks->size() < 5 ) return false;

    // get selected tracks info {{{
    //std::cout << "TrackProducer::inputTracks = " <<_tkHandle->size() << ", selectedTracks = " << selectedTracks->size() <<std::endl;
    //std::cout << "TrackProducer::inputMuons =  " << _muonHandle->size() << std::endl;
    // get selected tracks info end }}}
    iEvent.put( std::move(selectedTracks) );
    return true;
}

//------------------------------------------------------------------------------
//   Selection Criterias
//------------------------------------------------------------------------------

bool TrackProducer::IsSelectedTrack( const myTrack& tk ) const
//bool TrackProducer::IsSelectedTrack( const myTrack& tk, const reco::BeamSpot& bs ) const
{
   if (tk.pt()<0.8)                                return false;
   if (tk.track()->hitPattern().numberOfValidStripHits()<4) return false;
   if (tk.track()->hitPattern().numberOfValidPixelHits()<1) return false;
   return true;
}

bool TrackProducer::IsVetoTrack( const myTrack& tk ) const
{
   if (tk.track()->normalizedChi2()>5)             return false;
   if (tk.p()>200 || tk.pt()>200)                  return false;
   if (fabs(tk.eta()) > 2.5)                       return false;
   return false;
}
bool TrackProducer::IsMuonTrack( const myTrack& tk, const myMuonList& muList ) const
{
    bool isMuonTrack = false;
    for ( const myMuon& mu : muList )
    {
        if ( mu.track().isNonnull() ) continue;
        if (fabs(tk.pt() -mu.track()->pt() )<0.00001 &&
            fabs(tk.eta()-mu.track()->eta())<0.00001 &&
            fabs(tk.phi()-mu.track()->phi())<0.00001 )
        {
            isMuonTrack = true;
            break;
        }
    }
    return isMuonTrack;
}


//------------------------------------------------------------------------------
//   EDM Plugin requirements
//------------------------------------------------------------------------------
void TrackProducer::fillDescriptions(edm::ConfigurationDescriptions& descriptions)
{
   //The following says we do not know what parameters are allowed so do no validation
   // Please change this to state exactly what you do use, even if it is no parameters
   edm::ParameterSetDescription desc;
   desc.setUnknown();
   descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(TrackProducer);
