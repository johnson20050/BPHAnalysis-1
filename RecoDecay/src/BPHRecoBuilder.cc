/*
 *  See header file for a description of this class.
 *
 *  $Date: 2015-07-06 11:20:00 $
 *  $Revision: 1.1 $
 *  \author Paolo Ronchese INFN Padova
 *
 */

//-----------------------
// This Class' Header --
//-----------------------
#include "BPHAnalysis/RecoDecay/interface/BPHRecoBuilder.h"

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "BPHAnalysis/RecoDecay/interface/BPHRecoSelect.h"
#include "BPHAnalysis/RecoDecay/interface/BPHMomentumSelect.h"
#include "BPHAnalysis/RecoDecay/interface/BPHVertexSelect.h"
#include "BPHAnalysis/RecoDecay/interface/BPHFitSelect.h"
#include "BPHAnalysis/RecoDecay/interface/BPHRecoCandidate.h"
#include "DataFormats/RecoCandidate/interface/RecoCandidate.h"

//---------------
// C++ Headers --
//---------------
#include <iostream>
#include <map>

using namespace std;

//-------------------
// Initializations --
//-------------------


//----------------
// Constructors --
//----------------
BPHRecoBuilder::BPHRecoBuilder( const edm::EventSetup& es ):
 evSetup( &es ),
  minPDiff( -1.0 ) {
  msList.reserve( 5 );
  vsList.reserve( 5 );
}

//--------------
// Destructor --
//--------------
BPHRecoBuilder::~BPHRecoBuilder() {
  int n = sourceList.size();
  while ( n-- ) delete sourceList[n];
  int m = srCompList.size();
  while ( m-- ) delete srCompList[m];
  while ( compCollectList.size() ) {
    const std::vector<const BPHRecoCandidate*>* cCollection =
      *compCollectList.begin();
    delete cCollection;
    compCollectList.erase( cCollection );
  }
}

//--------------
// Operations --
//--------------
void BPHRecoBuilder::filter( const std::string& name,
                             const BPHRecoSelect& sel ) const {
  std::map<std::string,int>::const_iterator iter = sourceId.find( name );
  if ( iter == sourceId.end() ) return;
  BPHRecoSource* rs = sourceList[iter->second];
  rs->selector.push_back( &sel );
  return;
}


void BPHRecoBuilder::filter( const std::string& name,
                             const BPHMomentumSelect& sel ) const {
  std::map<std::string,int>::const_iterator iter = srCompId.find( name );
  if ( iter == sourceId.end() ) return;
  BPHCompSource* cs = srCompList[iter->second];
  cs->momSelector.push_back( &sel );
  return;
}


void BPHRecoBuilder::filter( const std::string& name,
                             const BPHVertexSelect& sel ) const {
  std::map<std::string,int>::const_iterator iter = srCompId.find( name );
  if ( iter == sourceId.end() ) return;
  BPHCompSource* cs = srCompList[iter->second];
  cs->vtxSelector.push_back( &sel );
  return;
}


void BPHRecoBuilder::filter( const std::string& name,
                             const BPHFitSelect& sel ) const {
  std::map<std::string,int>::const_iterator iter = srCompId.find( name );
  if ( iter == sourceId.end() ) return;
  BPHCompSource* cs = srCompList[iter->second];
  cs->fitSelector.push_back( &sel );
  return;
}


void BPHRecoBuilder::filter( const BPHMomentumSelect& sel ) {
  msList.push_back( &sel );
  return;
}


void BPHRecoBuilder::filter( const BPHVertexSelect& sel ) {
  vsList.push_back( &sel );
  return;
}


void BPHRecoBuilder::filter( const BPHFitSelect& sel ) {
  fsList.push_back( &sel );
  return;
}


bool BPHRecoBuilder::accept( const BPHRecoCandidate& cand ) const {
  int i;
  int n;
  n = msList.size();
  for ( i = 0; i < n; ++i ) {
    if ( !msList[i]->accept( cand ) ) return false;
  }
  n = vsList.size();
  for ( i = 0; i < n; ++i ) {
    if ( !vsList[i]->accept( cand ) ) return false;
  }
  n = fsList.size();
  for ( i = 0; i < n; ++i ) {
    if ( !fsList[i]->accept( cand ) ) return false;
  }
  return true;
}


void BPHRecoBuilder::setMinPDiffererence( double pMin ) {
  minPDiff = pMin;
  return;
}


std::vector<BPHRecoBuilder::ComponentSet> BPHRecoBuilder::build() const {
  daugMap().clear();
  compMap().clear();
  std::vector<ComponentSet> candList;
  ComponentSet compSet;
  build( candList, compSet,
         sourceList.begin(), sourceList.end(),
         srCompList.begin(), srCompList.end() );
  return candList;
}


const edm::EventSetup* BPHRecoBuilder::eventSetup() const {
  return evSetup;
}


std::map<std::string,const reco::Candidate*>& BPHRecoBuilder::daugMap() {
  static std::map<std::string,const reco::Candidate*> stMap;
  return stMap;
}


std::map<std::string,const BPHRecoCandidate*>& BPHRecoBuilder::compMap() {
  static std::map<std::string,const BPHRecoCandidate*> stMap;
  return stMap;
}


void BPHRecoBuilder::addCollection( const std::string& name,
                                    BPHGenericCollection* collection,
                                    double mass,
                                    double msig ) {
  BPHRecoSource* rs;
  if ( sourceId.find( name ) != sourceId.end() ) {
    cout << "Decay product already inserted with name " << name
         << " , skipped" << endl;
    return;
  }
  rs = new BPHRecoSource;
  rs->name =
      &sourceId.insert( make_pair( name, sourceList.size() ) ).first->first;
  rs->collection = collection;
  rs->selector.reserve( 5 );
  rs->mass = mass;
  rs->msig = msig;
  sourceList.push_back( rs );
  return;
}


void BPHRecoBuilder::addCollection( const std::string& name,
                                    const std::vector<const BPHRecoCandidate*>*
                                          collection ) {
  BPHCompSource* cs;
  if ( srCompId.find( name ) != srCompId.end() ) {
    cout << "Decay product already inserted with name " << name
         << " , skipped" << endl;
    return;
  }
  cs = new BPHCompSource;
  cs->name =
      &srCompId.insert( make_pair( name, srCompList.size() ) ).first->first;
  cs->collection = collection;
  srCompList.push_back( cs );
  return;
}


void BPHRecoBuilder::build( std::vector<ComponentSet>& compList,
                            ComponentSet& compSet,
                            std::vector<BPHRecoSource*>::const_iterator r_iter,
                            std::vector<BPHRecoSource*>::const_iterator r_iend,
                            std::vector<BPHCompSource*>::const_iterator c_iter,
                            std::vector<BPHCompSource*>::const_iterator c_iend ) const {
  if ( r_iter == r_iend ) {
    if ( c_iter == c_iend ) {
      compSet.compMap = compMap();
      compList.push_back( compSet );
      return;
    }
    BPHCompSource* source = *c_iter++;
    const std::vector<const BPHRecoCandidate*>* collection = source->collection;
    std::vector<const BPHMomentumSelect*> momSelector = source->momSelector;
    std::vector<const BPHVertexSelect*>   vtxSelector = source->vtxSelector;
    std::vector<const BPHFitSelect*>      fitSelector = source->fitSelector;
    int i;
    int j;
    int n = collection->size();
    int m;
    bool skip;
    for ( i = 0; i < n; ++i ) {
      skip = false;
      const BPHRecoCandidate* cand = collection->at( i );
      if ( contained( compSet, cand ) ) continue;
      m = momSelector.size();
      for ( j = 0; j < m; ++j ) {
        if ( !momSelector[j]->accept( *cand ) ) skip = true;
      }
      m = vtxSelector.size();
      for ( j = 0; j < m; ++j ) {
        if ( !vtxSelector[j]->accept( *cand ) ) skip = true;
      }
      m = fitSelector.size();
      for ( j = 0; j < m; ++j ) {
        if ( !fitSelector[j]->accept( *cand ) ) skip = true;
      }
      if ( skip ) continue;
      compMap()[*source->name] = cand;
      build( compList, compSet,
             r_iter, r_iend,
             c_iter, c_iend );
      compMap().erase( *source->name );
    }
    return;
  }
  BPHRecoSource* source = *r_iter++;
  BPHGenericCollection* collection = source->collection;
  std::vector<const BPHRecoSelect*>& selector = source->selector;
  int i;
  int j;
  int n = collection->size();
  int m = selector.size();
  bool skip;
  for ( i = 0; i < n; ++i ) {
    const reco::Candidate& cand = collection->get( i );
    if ( contained( compSet, &cand ) ) continue;
    skip = false;
    for ( j = 0; j < m; ++j ) {
      if ( !selector[j]->accept( cand ) ) skip = true;
    }
    if ( skip ) continue;
    BPHDecayMomentum::Component comp;
    comp.cand = &cand;
    comp.mass = source->mass;
    comp.msig = source->msig;
    compSet.daugMap[*source->name] = comp;
    daugMap()[*source->name] = &cand;
    build( compList, compSet,
           r_iter, r_iend,
           c_iter, c_iend );
    daugMap().erase( *source->name );
    compSet.daugMap.erase( *source->name );
  }
  return;
}


bool BPHRecoBuilder::contained( ComponentSet& compSet,
                                const reco::Candidate* cand ) const {
  reco::TrackRef tkCandRef = cand->get<reco::TrackRef>();
  const reco::RecoCandidate* rrCand =
        dynamic_cast<const reco::RecoCandidate*>( cand );
  std::map<std::string,BPHDecayMomentum::Component>& dMap = compSet.daugMap;
  map<std::string,
      BPHDecayMomentum::Component>::const_iterator d_iter;
  map<std::string,
      BPHDecayMomentum::Component>::const_iterator d_iend = dMap.end();
  for ( d_iter = dMap.begin(); d_iter != d_iend; ++d_iter ) {
    const reco::Candidate* cChk = d_iter->second.cand;
    if ( cand == cChk ) return true;
    if ( sameTrack( cand, tkCandRef, rrCand,
                    cChk, cChk->get<reco::TrackRef>(),
                    dynamic_cast<const reco::RecoCandidate*>( cChk ) ) )
         return true;
  }
  return false;

}


bool BPHRecoBuilder::contained( ComponentSet& compSet,
                                const BPHRecoCandidate* cCand ) const {

  const std::map<std::string,const BPHRecoCandidate*>& cMap = compMap();
  map<std::string,
      const BPHRecoCandidate*>::const_iterator c_iter;
  map<std::string,
      const BPHRecoCandidate*>::const_iterator c_iend = cMap.end();
  const vector<const reco::Candidate*>& dCand = cCand->daughFull();
  int j;
  int m = dCand.size();
  int k;
  int l;
  for ( j = 0; j < m; ++j ) {

    const reco::Candidate* cand = cCand->originalReco( dCand[j] );
    reco::TrackRef tkCandRef = cand->get<reco::TrackRef>();
      const reco::RecoCandidate* rrCand =
            dynamic_cast<const reco::RecoCandidate*>( cand );
    std::map<std::string,BPHDecayMomentum::Component>& dMap = compSet.daugMap;
    map<std::string,
        BPHDecayMomentum::Component>::const_iterator d_iter;
    map<std::string,
        BPHDecayMomentum::Component>::const_iterator d_iend = dMap.end();
    for ( d_iter = dMap.begin(); d_iter != d_iend; ++d_iter ) {
      const reco::Candidate* cChk = d_iter->second.cand;
      if ( cand == cChk ) return true;
      if ( sameTrack( cand, tkCandRef, rrCand,
                      cChk, cChk->get<reco::TrackRef>(),
                      dynamic_cast<const reco::RecoCandidate*>( cChk ) ) )
           return true;
    }

    for ( c_iter = cMap.begin(); c_iter != c_iend; ++c_iter ) {
      const pair<std::string,const BPHRecoCandidate*>& entry = *c_iter;
      const BPHRecoCandidate* cCChk = entry.second;
      const vector<const reco::Candidate*>& dCChk = cCChk->daughFull();
      l = dCChk.size();
      for ( k = 0; k < l; ++k ) {
        const reco::Candidate* cChk = cCChk->originalReco( dCChk[j] );
        if ( &cand == &cChk ) return true;
        if ( sameTrack( cand, tkCandRef, rrCand,
                        cChk, cChk->get<reco::TrackRef>(),
                        dynamic_cast<const reco::RecoCandidate*>( cChk ) ) )
             return true;
      }
    }

  }

  return false;

}


bool BPHRecoBuilder::sameTrack( const reco::Candidate* cand,
                                const reco::TrackRef& tkCandRef,
                                const reco::RecoCandidate* rrCand,
                                const reco::Candidate* cChk,
                                const reco::TrackRef& tkCChkRef,
                                const reco::RecoCandidate* rrCChk ) const {
  reco::Candidate::Vector pDiff = ( cand->momentum() -
                                    cChk->momentum() );
  reco::Candidate::Vector pMean = ( cand->momentum() +
                                    cChk->momentum() );
  double pDMod = pDiff.mag2();
  double pMMod = pMean.mag2();
  if ( ( ( pDMod / pMMod ) < minPDiff ) &&
       ( cand->charge() == cChk->charge() ) ) return true;
  if ( !tkCandRef.isNull() && !tkCChkRef.isNull() ) {
    if ( tkCandRef       == tkCChkRef       ) return true;
    if ( tkCandRef.get() == tkCChkRef.get() ) return true;
  }
  if ( rrCand == 0 ) return false;
  if ( rrCChk == 0 ) return false;
  reco::TrackRef tkCandRtk = rrCand->get<reco::TrackRef>();
  reco::TrackRef tkCChkRtk = rrCChk->get<reco::TrackRef>();
  if ( !tkCandRtk.isNull() && !tkCChkRtk.isNull() ) {
    if ( tkCandRtk       == tkCChkRtk       ) return true;
    if ( tkCandRtk.get() == tkCChkRtk.get() ) return true;
  }
  return false;
}
