// @HEADER
//
// ***********************************************************************
//
//        MueLu: A package for multigrid based preconditioning
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
// Questions? Contact
//                    Jonathan Hu       (jhu@sandia.gov)
//                    Andrey Prokopenko (aprokop@sandia.gov)
//                    Ray Tuminaro      (rstumin@sandia.gov)
//
// ***********************************************************************
//
// @HEADER
#ifndef MUELU_AVATARINTERFACE_HPP
#define MUELU_AVATARINTERFACE_HPP

#include <string>
#include "Teuchos_Comm.hpp"
#include "Teuchos_RCP.hpp"
#include "Teuchos_ArrayRCP.hpp"
#include "Teuchos_ParameterList.hpp"
#include "MueLu_BaseClass.hpp"


#ifdef HAVE_MUELU_AVATAR
namespace MueLu {

  /*! @class 
    Manages the interface to the Avatar machine learning library.

    Note only proc 0 (as defined by comm) will actually have avatar instantiated.  The options determined
    by avatar will be broadcast to all processors
  */
  class AvatarInterface : public BaseClass {

  public:

    AvatarInterface(Teuchos::RCP<const Teuchos::Comm<int> >& comm ):comm_(comm) {}

    AvatarInterface(Teuchos::RCP<const Teuchos::Comm<int> >& comm, Teuchos::ParameterList& inParams):comm_(comm),params_(inParams){};

    Teuchos::RCP<const Teuchos::ParameterList> GetValidParameterList() const;

    // Sets the input parameters for the AvatarInterface
    void SetParameterList(Teuchos::ParameterList& inParams) {params_ = inParams;}

    // Sets up Avatar
    void Setup();

    // Calls Avatar to set MueLu Parameters
    void SetMueLuParameters(const Teuchos::ParameterList & problemFeatures, Teuchos::ParameterList & mueluParams) const;


  private:
    Teuchos::RCP<const Teuchos::Comm<int> > comm_;

    Teuchos::ArrayRCP<std::string> ReadAvatarStringsFromFiles() const;

    // Cached data
    Teuchos::ParameterList params_;
    Teuchos::ArrayRCP<std::string> avatarStrings_;


    // FIXME: Need to store the handle to avatar


  };


} // namespace MueLu

#endif// HAVE_MUELU_AVATAR

#endif // MUELU_AVATARINTERFACE_HPP
