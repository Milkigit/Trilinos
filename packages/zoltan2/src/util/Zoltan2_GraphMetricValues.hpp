// @HEADER
//
// ***********************************************************************
//
//   Zoltan2: A package of combinatorial algorithms for scientific computing
//                  Copyright 2012 Sandia Corporation
//
// Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
// the U.S. Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact Karen Devine      (kddevin@sandia.gov)
//                    Erik Boman        (egboman@sandia.gov)
//                    Siva Rajamanickam (srajama@sandia.gov)
//
// ***********************************************************************
//
// @HEADER

/*! \file Zoltan2_GraphMetricValues.hpp
 */

#ifndef ZOLTAN2_GRAPHCMETRICVALUES_HPP
#define ZOLTAN2_GRAPHCMETRICVALUES_HPP

#include <Zoltan2_GraphModel.hpp>
#include <Zoltan2_MetricBase.hpp>
#include <Zoltan2_MetricFunctions.hpp>

namespace Zoltan2{

/*! \GraphMetricValues class containing the metrics for one measurable item. */
template <typename scalar_t>
  class GraphMetricValues : public MetricBase<scalar_t> {

public:

/*! \brief Constructor */
GraphMetricValues(std::string mname) : MetricBase<scalar_t>(evalNumMetrics, mname) {}

/*! \brief Constructor */
GraphMetricValues() : MetricBase<scalar_t>(evalNumMetrics) {}

/*! \brief Print a standard header */
static void printHeader(std::ostream &os);	// MDM - note we may want to make this a regular method

/*! \brief Print a standard line of data that fits under the header. */
virtual void printLine(std::ostream &os) const;

/*! \brief Set the global sum.  */
void setGlobalSum(scalar_t x) { this->values_[evalGlobalSum] = x;}

/*! \brief Set the global maximum across parts.  */
void setGlobalMax(scalar_t x) { this->values_[evalGlobalMax] = x;}

/*! \brief Get the global sum of edge cuts for all parts. */
scalar_t getGlobalSum() const { return this->values_[evalGlobalSum];}

/*! \brief Get the global maximum of edge cuts per part across all parts. */
scalar_t getGlobalMax() const { return this->values_[evalGlobalMax];}

/*! \brief  Enumerator for offsets into metric data. */
enum metricOffset{
  evalGlobalSum,   /*!< the global total on all parts */
  evalGlobalMax,   /*!< the maximum across all parts */
  evalNumMetrics    /*!< the number of metric values_ */
};

/// \brief Return a metric value specified by name
///
/// @param metric_name Name of metric to return
/// @param[out] value metric value returned by reference
///
/// @return Returns a boolean indicated whether or not the metric was returned
virtual scalar_t getMetricValue(const std::string & metric_name) const {
  if (metric_name == "global maximum") {
    return this->getGlobalMax();
  } else if (metric_name == "global sum") {
    return this->getGlobalSum();
  } else {
    return 0.0; // throw error
  }
}

/*! \setup a static set of strings. */
static std::set<std::string> static_graphMetrics_;

/*! \this method is enforced by the base class. */
virtual const std::set<std::string> & getMetrics() const { return GraphMetricValues<scalar_t>::static_graphMetrics_; }

};  // end class

// MDM - Need to figure out a better way to encapsulate this
template <typename scalar_t>
std::set<std::string> GraphMetricValues<scalar_t>::static_graphMetrics_ = {
  "global sum",
  "global maximum"
};

// MDM - These are methods I will move to the .cpp
template <typename scalar_t>
  void GraphMetricValues<scalar_t>::printHeader(std::ostream &os)
{
  os << std::setw(20) << " ";
  os << std::setw(24) << "----------SUM----------";
  os << std::endl;

  os << std::setw(20) << " ";
  os << std::setw(12) << "sum" << std::setw(12) << "max";
  os << std::endl;
}

template <typename scalar_t>
  void GraphMetricValues<scalar_t>::printLine(std::ostream &os) const
{
  std::string label( MetricBase<scalar_t>::getName() );

  os << std::setw(20) << label;
  os << std::setw(12) << std::setprecision(4) << this->values_[evalGlobalSum];
  os << std::setw(12) << std::setprecision(4) << this->values_[evalGlobalMax];
  os << std::endl;
}

} //namespace Zoltan2
#endif
