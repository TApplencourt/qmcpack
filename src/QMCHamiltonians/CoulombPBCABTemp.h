//////////////////////////////////////////////////////////////////
// (c) Copyright 2003-  by Jeongnim Kim and Kris Delaney
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//   National Center for Supercomputing Applications &
//   Materials Computation Center
//   University of Illinois, Urbana-Champaign
//   Urbana, IL 61801
//   e-mail: jnkim@ncsa.uiuc.edu
//   Tel:    217-244-6319 (NCSA) 217-333-3324 (MCC)
//
// Supported by 
//   National Center for Supercomputing Applications, UIUC
//   Materials Computation Center, UIUC
//////////////////////////////////////////////////////////////////
// -*- C++ -*-
#ifndef QMCPLUSPLUS_COULOMBPBCAB_TEMP_H
#define QMCPLUSPLUS_COULOMBPBCAB_TEMP_H
#include "QMCHamiltonians/QMCHamiltonianBase.h"
#include "LongRange/LRCoulombSingleton.h"
#include "Numerics/OneDimGridBase.h"
#include "Numerics/OneDimGridFunctor.h"
#include "Numerics/OneDimCubicSpline.h"

namespace qmcplusplus {

  /** @ingroup hamiltonian
   *\brief Calculates the AA Coulomb potential using PBCs
   *
   * Functionally identical to CoulombPBCAB but uses a templated version of
   * LRHandler.
   */
  struct CoulombPBCABTemp: public QMCHamiltonianBase {

    typedef LRCoulombSingleton::LRHandlerType LRHandlerType;
    typedef LRCoulombSingleton::GridType GridType;
    typedef LRCoulombSingleton::RadFunctorType RadFunctorType;

    ParticleSet* PtclA;
    ParticleSet* PtclB;
    LRHandlerType* AB;
    DistanceTableData* d_ab;

    ///boolean used to update Sk by this
    bool FirstSkUser;
    int NumSpeciesA;
    int NumSpeciesB;
    int ChargeAttribIndxA;
    int ChargeAttribIndxB;
    int MemberAttribIndxA;
    int MemberAttribIndxB;
    int NptclA;
    int NptclB;
    RealType myConst;
    RealType myRcut;

    vector<int> NofSpeciesA;
    vector<int> NofSpeciesB;
    vector<RealType> Zat,Zspec; 
    vector<RealType> Qat,Qspec; 
    /*@{
     * @brief temporary data for pbyp evaluation
     */
    ///new value for the proposed move
    RealType NewValue;
    ///short-range part for the moved particle
    RealType SRtmp;
    ///long-range part for the moved particle
    RealType LRtmp;
    ///short-range per particle
    vector<RealType> SRpart;
    ///long-range per particle
    vector<RealType> LRpart;
    /*@}*/

    ///radial grid
    GridType* myGrid;
    ///Always mave a radial functor for the bare coulomb
    RadFunctorType* V0;
    ///Short-range potential for each ion
    vector<RadFunctorType*> Vat;
    ///Short-range potential for each species
    vector<RadFunctorType*> Vspec;

    //This is set to true if the K_c of structure-factors are different
    bool kcdifferent; 
    RealType minkc;

    CoulombPBCABTemp(ParticleSet& ions, ParticleSet& elns);

    ///// copy constructor
    //CoulombPBCABTemp(const CoulombPBCABTemp& c);
    
    ~CoulombPBCABTemp();

    void resetTargetParticleSet(ParticleSet& P);

    Return_t evaluate(ParticleSet& P);

    inline Return_t evaluate(ParticleSet& P, vector<NonLocalData>& Txy) {
      return evaluate(P);
    }

    Return_t registerData(ParticleSet& P, BufferType& buffer);
    void copyFromBuffer(ParticleSet& P, BufferType& buf);
    void copyToBuffer(ParticleSet& P, BufferType& buf);
    Return_t evaluatePbyP(ParticleSet& P, int iat);
    void acceptMove(int iat);
    void rejectMove(int iat);

    /** Do nothing */
    bool put(xmlNodePtr cur) {
      return true;
    }

    bool get(std::ostream& os) const {
      os << "CoulombPBCAB potential: " << PtclA->getName() << "-" << PtclB->getName();
      return true;
    }

    QMCHamiltonianBase* makeClone(ParticleSet& qp, TrialWaveFunction& psi);

    void initBreakup();

    Return_t evalSR();
    Return_t evalLR();
    Return_t evalConsts();
    void add(int groupID, RadFunctorType* ppot);
  };

}
#endif

/***************************************************************************
 * $RCSfile$   $Author$
 * $Revision$   $Date$
 * $Id$ 
 ***************************************************************************/

