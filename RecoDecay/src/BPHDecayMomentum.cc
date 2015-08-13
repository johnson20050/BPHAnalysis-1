/*
 *  See header file for a description of this class.
 *
 *  $Date: 2015-07-03 10:36:48 $
 *  $Revision: 1.1 $
 *  \author Paolo Ronchese INFN Padova
 *
 */

//-----------------------
// This Class' Header --
//-----------------------
#include "BPHAnalysis/RecoDecay/interface/BPHDecayMomentum.h"

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "BPHAnalysis/RecoDecay/interface/BPHRecoCandidate.h"
#include "PhysicsTools/CandUtils/interface/AddFourMomenta.h"

//---------------
// C++ Headers --
//---------------
#include <iostream>
#include <memory>

using namespace std;

//-------------------
// Initializations --
//-------------------
vector<BPHDecayMomentum::Component> BPHDecayMomentum::compList;

//----------------
// Constructors --
//----------------
BPHDecayMomentum::BPHDecayMomentum():
 updated( false ) {
  dList.reserve( 2 );
}


BPHDecayMomentum::BPHDecayMomentum(
                  const map<string,BPHDecayMomentum::Component>& daugMap ):
 updated( false ) {
  // clone and store simple particles
  clonesList( daugMap );
}


BPHDecayMomentum::BPHDecayMomentum(
                  const map<string,BPHDecayMomentum::Component>& daugMap,
                  const map<string,const BPHRecoCandidate*> compMap ):
 // store the map of names to previously reconstructed particles
 cMap( compMap ),
 updated( false ) {
  // clone and store simple particles
  clonesList( daugMap );
  // store previously reconstructed particles using information in cMap
  dCompList();
}

//--------------
// Destructor --
//--------------
BPHDecayMomentum::~BPHDecayMomentum() {
  // delete all clones
  int n = dList.size();
  while ( n-- ) delete dList[n];
}

//--------------
// Operations --
//--------------
void BPHDecayMomentum::add( const string& name,
                            const reco::Candidate* daug, double mass ) {
  if ( dMap.find( name ) != dMap.end() ) {
    cout << "daughter particle \"" << name << "\" already present, ignored"
         << endl;
  }
  setNotUpdated();
  reco::Candidate* dnew = daug->clone();
  if ( mass > 0.0 ) dnew->setMass( mass );
  nList.push_back( name );
  dList.push_back( dnew );
  dMap[name] = dnew;
  clonesMap[dnew] = daug;
  return;
}


void BPHDecayMomentum::add( const string& name,
                            const BPHRecoCandidate* comp ) {
  setNotUpdated();
  nComp.push_back( name );
  cList.push_back( comp );
  addClonesMap( comp->clonesMap );
  return;
}



const pat::CompositeCandidate& BPHDecayMomentum::composite() const {
  computeMomentum();
  return compCand;
}


const std::vector<std::string>& BPHDecayMomentum::daugNames() const {
  return nList;
}


const std::vector<std::string>& BPHDecayMomentum::compNames() const {
  return nComp;
}


const vector<const reco::Candidate*>& BPHDecayMomentum::daughters() const {
  return dList;
}


const vector<const reco::Candidate*>& BPHDecayMomentum::daughFull() const {
  // compute total momentum to update the full list of decay products
  computeMomentum();
  return dFull;
}


const reco::Candidate* BPHDecayMomentum::originalReco(
                       const reco::Candidate* daug ) const {
  // return the original particle for a given clone
  // return null pointer if not found
  map<const reco::Candidate*,
      const reco::Candidate*>::const_iterator iter = clonesMap.find( daug );
  return ( iter != clonesMap.end() ? iter->second : 0 );
}


const vector<const BPHRecoCandidate*>& BPHDecayMomentum::daughComp() const {
  // return the list of previously reconstructed particles
  return cList;
}


const reco::Candidate* BPHDecayMomentum::getDaug(
                       const std::string& name ) const {
  // return a simple particle from the name
  // return null pointer if not found
  map<const string,
      const reco::Candidate*>::const_iterator iter = dMap.find( name );
  return ( iter != dMap.end() ? iter->second : 0 );
}


const BPHRecoCandidate* BPHDecayMomentum::getComp(
                       const std::string& name ) const {
  // return a previously reconstructed particle from the name
  // return null pointer if not found
  map<const string,
      const BPHRecoCandidate*>::const_iterator iter = cMap.find( name );
  return ( iter != cMap.end() ? iter->second : 0 );
}


const vector<BPHDecayMomentum::Component>& BPHDecayMomentum::componentList() {
  // return a static object filled in the constructor
  // to be used in the creation of other bases of BPHRecoCandidate
  return compList;
}


void BPHDecayMomentum::setNotUpdated() const {
  updated = false;
  return;
}


void BPHDecayMomentum::clonesList( const map<string,Component>& daugMap ) {
  int n = daugMap.size();
  dList.resize( n );
  nList.resize( n );
  // reset and fill a static object
  // to be used in the creation of other bases of BPHRecoCandidate
  compList.clear();
  compList.reserve( n );
  // loop over daughters
  int i = 0;
  double mass;
  reco::Candidate* dnew;
  map<string,Component>::const_iterator iter = daugMap.begin();
  map<string,Component>::const_iterator iend = daugMap.end();
  while ( iter != iend ) {
    const pair<string,Component>& entry = *iter++;
    const Component& comp = entry.second;
    const reco::Candidate* cand = comp.cand;
    // store component for usage
    // in the creation of other bases of BPHRecoCandidate
    compList.push_back( comp );
    // clone particle and store it with its name
    dList[i] = dnew = cand->clone();
    dMap[nList[i++] = entry.first] = dnew;
    clonesMap[dnew] = cand;
    // set daughter mass if requested
    mass = comp.mass;
    if ( mass > 0 ) dnew->setMass( mass );
  }
  return;
}


void BPHDecayMomentum::dCompList() {
  // fill lists of previously reconstructed particles and their names
  // and retrieve cascade decay products
  int n = cMap.size();
  cList.resize( n );
  nComp.resize( n );
  int i = 0;
  map<string,const BPHRecoCandidate*>::const_iterator iter = cMap.begin();
  map<string,const BPHRecoCandidate*>::const_iterator iend = cMap.end();
  while ( iter != iend ) {
    const pair<string,const BPHRecoCandidate*>& entry = *iter++;
    nComp[i] = entry.first;
    const BPHRecoCandidate* comp = entry.second;
    cList[i++] = comp;
    addClonesMap( comp->clonesMap );
  }
  return;
}


void BPHDecayMomentum::addClonesMap(
                       const std::map<const reco::Candidate*,
                                      const reco::Candidate*>& clMap ) {
  // include in the map of clones to original particles the
  // corresponding map for previously reconstructed particles
  map<const reco::Candidate*,
      const reco::Candidate*>::const_iterator iter = clMap.begin();
  map<const reco::Candidate*,
      const reco::Candidate*>::const_iterator iend = clMap.end();
  while ( iter != iend ) clonesMap.insert( *iter++ );
  return;
}


void BPHDecayMomentum::sumMomentum(
                       const vector<const reco::Candidate*> dl ) const {
  // add the particles to pat::CompositeCandidate
  int n = dl.size();
  while ( n-- ) compCand.addDaughter( *dl[n] );  
  return;
}


void BPHDecayMomentum::fillDaug( vector<const reco::Candidate*>& ad ) const {
  // recursively fill the list of simple particles, produced
  // directly or in cascade decays
  int n = dList.size();
  while ( n-- ) ad.push_back( dList[n] );
  int m = cList.size();
  while ( m-- ) cList[m]->fillDaug( ad );
  return;
}


void BPHDecayMomentum::computeMomentum() const {
  // check cached result
  if ( updated ) return;
  // reset full list of daughters
  dFull.clear();
  fillDaug( dFull );
  // reset and fill pat::CompositeCandidate
  compCand.clearDaughters();
  sumMomentum( dFull );
  // compute the total momentum
  AddFourMomenta addP4;
  addP4.set( compCand );
  updated = true;
  return;
}
