#ifndef BPHAnalysis_SpecificDecay_lbHistoSpecificDecay_h
#define BPHAnalysis_SpecificDecay_lbHistoSpecificDecay_h

#include "BPHAnalysis/RecoDecay/interface/BPHAnalyzerTokenWrapper.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/Common/interface/Ref.h"

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include <string>

class TH1F;
class TVector3;

namespace reco {
  class Candidate;
  class Vertex;
}

class lbHistoSpecificDecay:
      public BPHAnalyzerWrapper<BPHModuleWrapper::one_analyzer> {

 public:

  explicit lbHistoSpecificDecay( const edm::ParameterSet& ps );
  virtual ~lbHistoSpecificDecay();

  static void fillDescriptions( edm::ConfigurationDescriptions& descriptions );

  virtual void beginJob();
  virtual void analyze( const edm::Event& ev, const edm::EventSetup& es );
  virtual void endJob();

  class CandidateSelect {
   public:
    virtual ~CandidateSelect() {}
    virtual bool accept( const pat::CompositeCandidate& cand,
                         const reco::Vertex* pv = 0 ) const = 0 ;
  };

 private:

  std::string oniaCandsLabel;
  std::string lam0CandsLabel;
  std::string tktkCandsLabel;
  std::string LbL0CandsLabel;
  std::string LbTkCandsLabel;
  BPHTokenWrapper< std::vector<pat::CompositeCandidate> > oniaCandsToken;
  BPHTokenWrapper< std::vector<pat::CompositeCandidate> > lam0CandsToken;
  BPHTokenWrapper< std::vector<pat::CompositeCandidate> > tktkCandsToken;
  BPHTokenWrapper< std::vector<pat::CompositeCandidate> > LbL0CandsToken;
  BPHTokenWrapper< std::vector<pat::CompositeCandidate> > LbTkCandsToken;
  bool useOnia;
  bool useLam0;
  bool useTkTk;
  bool useLbL0;
  bool useLbTk;

  edm::Service<TFileService> fs;
  std::map<std::string,TH1F*> histoMap;


  void fillHisto   ( const std::string& name,
                     const pat::CompositeCandidate& cand );
  void fillHisto   ( const std::string& name, float x );
  void createHisto ( const std::string& name,
                     int nbin, float hmin, float hmax );

};

#endif
