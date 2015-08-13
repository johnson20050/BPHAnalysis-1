#ifndef BPHDecayMomentum_H
#define BPHDecayMomentum_H
/** \class BPHDecayMomentum
 *
 *  Description: 
 *     Lowest-level base class to contain decay products and
 *     compute total momentum
 *
 *  $Date: 2015-07-03 10:36:48 $
 *  $Revision: 1.1 $
 *  \author Paolo Ronchese INFN Padova
 *
 */

//----------------------
// Base Class Headers --
//----------------------


//------------------------------------
// Collaborating Class Declarations --
//------------------------------------
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
class BPHRecoBuilder;
class BPHRecoCandidate;

//---------------
// C++ Headers --
//---------------
#include <vector>
#include <map>

//              ---------------------
//              -- Class Interface --
//              ---------------------

class BPHDecayMomentum {

  friend class BPHRecoBuilder;

 public:

  /** Constructors are protected
   *  this object can exist only as part of a derived class
   */

  /** Destructor
   */
  virtual ~BPHDecayMomentum();

  /** Operations
   */

  /// add a simple particle giving it a name
  /// particles are cloned, eventually specifying a different mass
  virtual void add( const std::string& name,
                    const reco::Candidate* daug, double mass = -1.0 );
  /// add a previously reconstructed particle giving it a name
  virtual void add( const std::string& name,
                    const BPHRecoCandidate* comp );

  /// get a composite by the simple sum of simple particles
  virtual const pat::CompositeCandidate& composite() const;

  /// get the list of names of simple particles directly produced in the decay
  /// e.g. in JPsi -> mu+mu-   returns the two names used for muons
  ///      in B+   -> JPsi K+  returns the name used for the K+
  ///      in Bs   -> JPsi Phi returns an empty list
  virtual const std::vector<std::string>& daugNames() const;

  /// get the list of names of previously reconstructed particles
  /// e.g. in JPsi -> mu+mu-   returns an empty list
  ///      in B+   -> JPsi K+  returns the name used for the JPsi
  ///      in Bs   -> JPsi Phi returns the two names used for JPsi and Phi
  virtual const std::vector<std::string>& compNames() const;

  /// get the list of simple particles directly produced in the decay
  /// e.g. in JPsi -> mu+mu-   returns the two muons
  ///      in B+   -> JPsi K+  returns the K+
  ///      in Bs   -> JPsi Phi returns an empty list
  /// (clones are actually returned)
  virtual const std::vector<const reco::Candidate*>& daughters() const;

  /// get the full list of simple particles produced in the decay,
  /// directly or through cascade decays
  /// e.g. in B+   -> JPsi K+ ; JPsi -> mu+mu- returns the mu+, mu-, K+
  ///      in Bs   -> JPsi Phi; JPsi -> mu+mu-; Phi -> K+K-
  ///                                          returns the mu+, mu-, K+, K-
  /// (clones are actually returned)
  virtual const std::vector<const reco::Candidate*>& daughFull() const;

  /// get the original particle from the clone
  virtual const reco::Candidate* originalReco(
          const reco::Candidate* daug ) const;

  /// get the list of previously reconstructed particles
  /// e.g. in JPsi -> mu+mu-   returns an empty list
  ///      in B+   -> JPsi K+  returns the JPsi
  ///      in Bs   -> JPsi Phi returns the JPsi and Phi
  virtual const std::vector<const BPHRecoCandidate*>& daughComp() const;

  /// get a simple particle from the name
  /// return null pointer if not found
  virtual const reco::Candidate*  getDaug( const std::string& name ) const;

  /// get a previously reconstructed particle from the name
  /// return null pointer if not found
  virtual const BPHRecoCandidate* getComp( const std::string& name ) const;

 protected:

  struct Component {
    const reco::Candidate* cand;
    double mass;
    double msig;
  };

  // get a static object filled in the constructor
  // to be used in the creation of other bases of BPHRecoCandidate
  static const std::vector<Component>& componentList();

  // map linking cloned particles to original ones
  std::map<const reco::Candidate*, const reco::Candidate*> clonesMap;

  // constructors
  BPHDecayMomentum();
  BPHDecayMomentum( const std::map<std::string,
                                   Component>& daugMap );
  BPHDecayMomentum( const std::map<std::string,
                                   Component>& daugMap,
                    const std::map<std::string,
                                   const BPHRecoCandidate*> compMap );

  // utility function used to cash reconstruction results
  virtual void setNotUpdated() const;

 private:

  // static object filled in the constructor
  // to be used in the creation of other bases of BPHRecoCandidate
  static std::vector<Component> compList;

  // names used for simple and previously reconstructed particles
  std::vector<std::string> nList;
  std::vector<std::string> nComp;

  // pointers to simple and previously reconstructed particles
  // (clones stored for simple particles)
  std::vector<const reco::Candidate*> dList;
  std::vector<const BPHRecoCandidate*> cList;

  // maps linking names to decay products
  // (simple and previously reconstructed particles)
  std::map<std::string,const reco::Candidate*> dMap;
  std::map<std::string,const BPHRecoCandidate*> cMap;

  // reconstruction results cache
  mutable bool updated;
  mutable std::vector<const reco::Candidate*> dFull;
  mutable pat::CompositeCandidate compCand;

  // create clones of simple particles, store them and their names
  void clonesList( const std::map<std::string,
                   Component>& daugMap );

  // fill lists of previously reconstructed particles and their names
  // and retrieve cascade decay products
  void dCompList();

  // include in the map of clones to original particles the
  // corresponding map for previously reconstructed particles
  void addClonesMap( const std::map<const reco::Candidate*,
                                    const reco::Candidate*>& clMap );

  // compute the total momentum of simple particles, produced
  // directly or in cascade decays
  virtual void sumMomentum(
               const std::vector<const reco::Candidate*> dl ) const;

  // recursively fill the list of simple particles, produced
  // directly or in cascade decays
  virtual void fillDaug( std::vector<const reco::Candidate*>& ad ) const;

  // compute the total momentum and cache it
  virtual void computeMomentum() const;

};


#endif // BPHDecayMomentum_H
