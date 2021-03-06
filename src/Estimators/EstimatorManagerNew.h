//////////////////////////////////////////////////////////////////////////////////////
// This file is distributed under the University of Illinois/NCSA Open Source License.
// See LICENSE file in top directory for details.
//
// Copyright (c) 2020 QMCPACK developers.
//
// File developed by: Peter Doak, doakpw@ornl.gov, Oak Ridge National Lab
//
// File refactored from: EstimatorManagerBase.h
//////////////////////////////////////////////////////////////////////////////////////

#ifndef QMCPLUSPLUS_ESTIMATORMANAGERNEW_H
#define QMCPLUSPLUS_ESTIMATORMANAGERNEW_H

#include "Configuration.h"
#include "Utilities/Timer.h"
#include "Utilities/PooledData.h"
#include "Message/Communicate.h"
#include "Estimators/ScalarEstimatorBase.h"
#include "Estimators/EstimatorManagerInterface.h"
#include "Particle/Walker.h"
#include "OhmmsPETE/OhmmsVector.h"
#include "OhmmsData/HDFAttribIO.h"
#include "type_traits/template_types.hpp"
#include <bitset>

namespace qmcplusplus
{
class QMCHamiltonian;
class CollectablesEstimator;

namespace testing
{
class EstimatorManagerNewTest;
} // namespace testing


/** Class to manage a set of ScalarEstimators */
class EstimatorManagerNew
{
public:
  /// This is to deal with vague expression of precision in legacy code. Don't use in new code.
  typedef QMCTraits::FullPrecRealType RealType;
  using FullPrecRealType = QMCTraits::FullPrecRealType;

  typedef ScalarEstimatorBase EstimatorType;
  using FPRBuffer =  std::vector<FullPrecRealType>;
  using MCPWalker = Walker<QMCTraits, PtclOnLatticeTraits>;

  ///name of the primary estimator name
  std::string MainEstimatorName;
  ///the root file name
  std::string RootName;
  ///energy
  TinyVector<RealType, 4> RefEnergy;
  ///default constructor
  EstimatorManagerNew(Communicate* c = 0);
  ///copy constructor
  EstimatorManagerNew(EstimatorManagerNew& em);
  ///destructor
  virtual ~EstimatorManagerNew();

  /** set the communicator */
  void setCommunicator(Communicate* c);

  /** return the communicator
   */
  Communicate* getCommunicator() { return my_comm_; }

  ///return the number of ScalarEstimators
  inline int size() const { return Estimators.size(); }

  /** add a property with a name
   * @param aname name of the column
   * @return the property index so that its value can be set by setProperty(i)
   *
   * Append a named column. BlockProperties do not contain any meaning data
   * but manages the name to index map for PropertyCache.
   */
  inline int addProperty(const char* aname) { return BlockProperties.add(aname); }

  /** set the value of the i-th column with a value v
   * @param i column index
   * @param v value
   */
  inline void setProperty(int i, RealType v) { PropertyCache[i] = v; }

  inline RealType getProperty(int i) const { return PropertyCache[i]; }

  int addObservable(const char* aname);

  inline RealType getObservable(int i) const { return TotalAverages[i]; }

  void getData(int i, std::vector<RealType>& values);

  /** add an Estimator
   * @param newestimator New Estimator
   * @param aname name of the estimator
   * @return locator of newestimator
   */
  int add(EstimatorType* newestimator, const std::string& aname);

  /** add a main estimator
   * @param newestimator New Estimator
   * @return locator of newestimator
   */
  int add(EstimatorType* newestimator) { return add(newestimator, MainEstimatorName); }

  ///return a pointer to the estimator aname
  EstimatorType* getEstimator(const std::string& a);

  ///return the average for estimator i
  inline RealType average(int i) const { return Estimators[i]->average(); }

  ///returns a variance for estimator i
  inline RealType variance(int i) const { return Estimators[i]->variance(); }

  ///process xml tag associated with estimators
  bool put(QMCHamiltonian& H, xmlNodePtr cur);

  /** reset the estimator
   */
  void reset();

  /** start a run
   * @param blocks number of blocks
   * @param record if true, will write to a file
   *
   * Replace reportHeader and reset functon.
   */
  void start(int blocks, bool record = true);

  /** start  a block
   * @param steps number of steps in a block
   */
  void startBlock(int steps);

  void setNumberOfBlocks(int blocks)
  {
    for (int i = 0; i < Estimators.size(); i++)
      Estimators[i]->setNumberOfBlocks(blocks);
  }

  /** unified: stop a block
   * @param accept acceptance rate of this block
   * \param[in] accept
   * \param[in] reject
   * \param[in] block_weight
   * \param[in] cpu_block_time Timer returns double so this is not altered by "mixed" precision
   */
  void stopBlock(unsigned long accept, unsigned long reject, RealType block_weight, double cpu_block_time);

  /** At end of block collect the scalar estimators for the entire rank
   *   
   *  Each is currently accumulates on for crowd of 1 or more walkers
   *  returns the total weight across all crowds. 
   */
  RealType collectScalarEstimators(const RefVector<ScalarEstimatorBase>& scalar_estimators);

  /** get the average of per-block energy and variance of all the blocks
   * Note: this is not weighted average. It can be the same as weighted average only when block weights are identical.
   */
  void getApproximateEnergyVariance(RealType& e, RealType& var);

  template<class CT>
  void write(CT& anything, bool doappend)
  {
    anything.write(h_file, doappend);
  }

  auto& get_AverageCache() { return AverageCache; }
  auto& get_SquaredAverageCache() { return SquaredAverageCache; }

protected:
  friend class EstimatorManagerCrowd;
  //  TODO: fix needless use of bitset instead of clearer more visible booleans
  std::bitset<8> Options;
  ///size of the message buffer
  int BufferSize;
  ///number of records in a block
  int RecordCount;
  ///index for the block weight PropertyCache(weightInd)
  int weightInd;
  ///index for the block cpu PropertyCache(cpuInd)
  int cpuInd;
  ///index for the accept counter PropertyCache(acceptInd)
  int acceptRatioInd;
  ///hdf5 handler
  hid_t h_file;
  ///total weight accumulated in a block
  RealType BlockWeight;
  ///file handler to write data
  std::ofstream* Archive;
  ///file handler to write data for debugging
  std::ofstream* DebugArchive;
  ///communicator to handle communication
  Communicate* my_comm_;
  /** pointer to the primary ScalarEstimatorBase
   */
  ScalarEstimatorBase* MainEstimator;
  /** pointer to the CollectablesEstimator
   *
   * Do not need to clone: owned by the master thread
   */
  CollectablesEstimator* Collectables;
  /** accumulator for the energy
   *
   * @todo expand it for all the scalar observables to report the final results
   */
  ScalarEstimatorBase::accumulator_type energyAccumulator;
  /** accumulator for the variance **/
  ScalarEstimatorBase::accumulator_type varAccumulator;
  ///cached block averages of the values
  Vector<RealType> AverageCache;
  ///cached block averages of the squared values
  Vector<RealType> SquaredAverageCache;
  ///cached block averages of properties, e.g. BlockCPU
  Vector<RealType> PropertyCache;
  ///manager of scalar data
  RecordNamedProperty<RealType> BlockAverages;
  ///manager of property data
  RecordNamedProperty<RealType> BlockProperties;
  ///block averages: name to value
  RecordNamedProperty<RealType> TotalAverages;
  ///data accumulated over the blocks
  Matrix<RealType> TotalAveragesData;
  ///index mapping between BlockAverages and TotalAverages
  std::vector<int> Block2Total;
  ///column map
  std::map<std::string, int> EstimatorMap;
  ///estimators of simple scalars
  std::vector<EstimatorType*> Estimators;
  ///convenient descriptors for hdf5
  std::vector<observable_helper*> h5desc;
  /////estimators of composite data
  //CompositeEstimatorSet* CompEstimators;
  ///Timer
  Timer MyTimer;

private:
  ///number of maximum data for a scalar.dat
  int max4ascii;

  /// collect data and write
  void makeBlockAverages(unsigned long accept, unsigned long reject);

  ///add header to an std::ostream
  void addHeader(std::ostream& o);
  size_t FieldWidth;

  friend class qmcplusplus::testing::EstimatorManagerNewTest;
};
} // namespace qmcplusplus
#endif
