/**  \file posterior_mcmc.hpp \brief Posterior distribution on GPs
     based on MCMC over kernel parameters */
/*
-------------------------------------------------------------------------
   This file is part of BayesOpt, an efficient C++ library for 
   Bayesian optimization.

   Copyright (C) 2011-2014 Ruben Martinez-Cantin <rmcantin@unizar.es>
 
   BayesOpt is free software: you can redistribute it and/or modify it 
   under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   BayesOpt is distributed in the hope that it will be useful, but 
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with BayesOpt.  If not, see <http://www.gnu.org/licenses/>.
------------------------------------------------------------------------
*/


#ifndef  _POSTERIOR_MCMC_HPP_
#define  _POSTERIOR_MCMC_HPP_

#include <boost/ptr_container/ptr_vector.hpp>
#include "criteria_functors.hpp"
#include "posteriormodel.hpp"
#include "mcmc_sampler.hpp"

namespace bayesopt {


  /**
   * \brief Posterior model of nonparametric processes/criteria based
   * on MCMC samples.
   *
   * For computational reasons we store a copy of each conditional
   * models with the corresponding particle generated by MCMC. That is
   * to avoid costly operations like matrix inversions for every
   * kernel parameter in a GP prediction. Thus, we assume that the
   * number of particles is not very large.
   */
  class MCMCModel: public PosteriorModel
  {
  public:

    typedef boost::ptr_vector<NonParametricProcess>  GPVect;
    typedef boost::ptr_vector<Criteria>  CritVect;

    /** 
     * \brief Constructor (Note: default constructor is private)
     * 
     * @param dim number of input dimensions
     * @param params configuration parameters (see parameters.hpp)
     * @param eng random number generation engine (boost)
     */
    MCMCModel(size_t dim, Parameters params, randEngine& eng);

    virtual ~MCMCModel();

    void updateHyperParameters();
    void fitSurrogateModel();
    void updateSurrogateModel();

    double evaluateCriteria(const vectord& query);
    void updateCriteria(const vectord& query);

    bool criteriaRequiresComparison();
    void setFirstCriterium();
    bool setNextCriterium(const vectord& prevResult);
    std::string getBestCriteria(vectord& best);

    ProbabilityDistribution* getPrediction(const vectord& query);
   
  private:
    void setSurrogateModel(randEngine& eng);    
    void setCriteria(randEngine& eng);

  private:  // Members
    size_t nParticles;
    GPVect mGP;                ///< Pointer to surrogate model
    CritVect mCrit;                    ///< Metacriteria model

    boost::scoped_ptr<MCMCSampler> kSampler;

  private: //Forbidden
    MCMCModel();
    MCMCModel(MCMCModel& copy);
  };

  /**@}*/

  inline void MCMCModel::fitSurrogateModel()
  { 
    for(GPVect::iterator it=mGP.begin(); it != mGP.end(); ++it)
      it->fitSurrogateModel(); 
  };

  inline void MCMCModel::updateSurrogateModel()
  {     
    for(GPVect::iterator it=mGP.begin(); it != mGP.end(); ++it)
      it->updateSurrogateModel(); 
  };

  inline double MCMCModel::evaluateCriteria(const vectord& query)
  { 
    double sum = 0.0;
    for(CritVect::iterator it=mCrit.begin(); it != mCrit.end(); ++it)
      {
	sum += it->evaluate(query); 
      }
    return sum/static_cast<double>(nParticles);
  };

  inline void MCMCModel::updateCriteria(const vectord& query)
  { 
    for(CritVect::iterator it=mCrit.begin(); it != mCrit.end(); ++it)
      {
	it->update(query); 
      }
  };


  inline bool MCMCModel::criteriaRequiresComparison()
  {return mCrit[0].requireComparison(); };
    
  inline void MCMCModel::setFirstCriterium()
  { 
    for(CritVect::iterator it=mCrit.begin(); it != mCrit.end(); ++it)
      {
	it->initialCriteria();
      }
  };

  // Although we change the criteria for all MCMC particles, we use
  // only the first element to compute de Hedge algorithm, because it
  // should be based on the average result, thus being common for all
  // the particles.
  inline bool MCMCModel::setNextCriterium(const vectord& prevResult)
  { 
    bool rotated;
    mCrit[0].pushResult(prevResult);
    for(CritVect::iterator it=mCrit.begin(); it != mCrit.end(); ++it)
      {
	rotated = it->rotateCriteria();
      }
    return rotated; 
  };

  inline std::string MCMCModel::getBestCriteria(vectord& best)
  { return mCrit[0].getBestCriteria(best); };

  inline 
  ProbabilityDistribution* MCMCModel::getPrediction(const vectord& query)
  { return mGP[0].prediction(query); };



} //namespace bayesopt


#endif
