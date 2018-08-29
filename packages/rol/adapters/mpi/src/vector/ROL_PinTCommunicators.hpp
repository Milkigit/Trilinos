// @HEADER
// ************************************************************************
//
//               Rapid Optimization Library (ROL) Package
//                 Copyright (2014) Sandia Corporation
//
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
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
// Questions? Contact lead developers:
//              Drew Kouri   (dpkouri@sandia.gov) and
//              Denis Ridzal (dridzal@sandia.gov)
//
// ************************************************************************
// @HEADER

#ifndef ROL_PINTCOMMUNICATORS_H
#define ROL_PINTCOMMUNICATORS_H

#include <ostream>

#include "mpi.h"

/** @ingroup la_group
    \class ROL::PinTVector
    \brief Defines a parallel in time vector.
*/

namespace ROL {

/** 
 * This class handles PinT communication. Its
 * based on the approach taken in X-Braid.
 */
class PinTCommunicators {
public:
  PinTCommunicators(MPI_Comm parent,int spatialProcs)
  { 
    parentComm_ = parent; 

    int myGlobalRank = -1;
    int globalProcs = -1;
    MPI_Comm_size(parentComm_, &globalProcs);
    MPI_Comm_rank(parentComm_, &myGlobalRank);

    // make sure they divide evenly (for sanity!)
    assert(globalProcs % spatialProcs == 0);

    int space = myGlobalRank / spatialProcs;
    int time  = myGlobalRank % spatialProcs;

    // this decomposition comes from X-Braid 
    MPI_Comm_split(parentComm_,space,myGlobalRank,&spaceComm_); 
    MPI_Comm_split(parentComm_, time,myGlobalRank, &timeComm_); 

    MPI_Comm_size(timeComm_, &timeSize_);
    MPI_Comm_rank(timeComm_, &timeRank_);

    MPI_Comm_size(spaceComm_, &spaceSize_);
    MPI_Comm_rank(spaceComm_, &spaceRank_);
  }

  // cleanup
  ~PinTCommunicators()
  { MPI_Comm_free(&spaceComm_);  
    MPI_Comm_free(&timeComm_); }

   MPI_Comm getParentCommunicator() const { return parentComm_; }
   MPI_Comm getSpaceCommunicator() const { return spaceComm_; }
   MPI_Comm getTimeCommunicator() const { return timeComm_; }

   int getTimeRank() const { return timeRank_; }
   int getTimeSize() const { return timeSize_; }

   int getSpaceRank() const { return spaceRank_; }
   int getSpaceSize() const { return spaceSize_; }

private:

  MPI_Comm parentComm_;

  MPI_Comm spaceComm_;
  MPI_Comm timeComm_;
  int timeSize_;
  int timeRank_;
  int spaceSize_;
  int spaceRank_;
};

} // namespace ROL

#endif
