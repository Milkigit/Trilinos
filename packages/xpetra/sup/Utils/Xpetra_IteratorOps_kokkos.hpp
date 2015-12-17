// @HEADER
//
// ***********************************************************************
//
//             Xpetra: A linear algebra interface package
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
//                    Tobias Wiesner    (tawiesn@sandia.gov)
//
// ***********************************************************************
//
// @HEADER

#ifndef PACKAGES_XPETRA_SUP_UTILS_XPETRA_ITERATOROPS_HPP_
#define PACKAGES_XPETRA_SUP_UTILS_XPETRA_ITERATOROPS_HPP_

#ifdef HAVE_XPETRA_KOKKOS_REFACTOR

#include "Xpetra_ConfigDefs.hpp"

#include "Xpetra_Matrix.hpp"
#include "Xpetra_MatrixMatrix.hpp"
#include "Xpetra_CrsMatrixWrap.hpp"

#include "Xpetra_Map.hpp"
#include "Xpetra_StridedMap.hpp"
#include "Xpetra_StridedMapFactory.hpp"
#include "Xpetra_MapExtractor.hpp"
#include "Xpetra_MatrixFactory.hpp"
#include "Xpetra_BlockedCrsMatrix.hpp"

namespace Xpetra {

  // General implementation
  // Epetra variant throws
  template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node>
  void Jacobi_kokkos(
              Scalar omega,
              const Xpetra::Vector<Scalar,LocalOrdinal,GlobalOrdinal,Node> & Dinv,
              const Xpetra::Matrix<Scalar, LocalOrdinal, GlobalOrdinal, Node>& A,
              const Xpetra::Matrix<Scalar, LocalOrdinal, GlobalOrdinal, Node>& B,
              Xpetra::Matrix<Scalar, LocalOrdinal, GlobalOrdinal, Node>& C,
              bool call_FillComplete_on_result = true,
              bool doOptimizeStorage = true,
              const std::string & label = std::string()) {
    typedef Scalar        SC;
    typedef LocalOrdinal  LO;
    typedef GlobalOrdinal GO;
    typedef Node          NO;

    TEUCHOS_TEST_FOR_EXCEPTION(C.getRowMap()->isSameAs(*A.getRowMap()) == false, Exceptions::RuntimeError,
                               "XpetraExt::MatrixMatrix::Jacobi: row map of C is not same as row map of A")
    TEUCHOS_TEST_FOR_EXCEPTION(C.getRowMap()->isSameAs(*B.getRowMap()) == false, Exceptions::RuntimeError,
                               "XpetraExt::MatrixMatrix::Jacobi: row map of C is not same as row map of B");
    TEUCHOS_TEST_FOR_EXCEPTION(!A.isFillComplete(), Exceptions::RuntimeError, "A is not fill-completed");
    TEUCHOS_TEST_FOR_EXCEPTION(!B.isFillComplete(), Exceptions::RuntimeError, "B is not fill-completed");

    bool haveMultiplyDoFillComplete = call_FillComplete_on_result && doOptimizeStorage;

    if (C.getRowMap()->lib() == Xpetra::UseEpetra) {
#ifndef HAVE_XPETRA_EPETRAEXT
      throw(Xpetra::Exceptions::RuntimeError("Xpetra::MatrixMatrix::Jacobi requires EpetraExt to be compiled."));
#else
      throw(Xpetra::Exceptions::RuntimeError("Xpetra::MatrixMatrix::Jacobi requires you to use an Epetra-compatible data type."));
#endif
    } else if (C.getRowMap()->lib() == Xpetra::UseTpetra) {
#ifdef HAVE_XPETRA_TPETRA
      const Tpetra::CrsMatrix<SC,LO,GO,NO>    & tpA = Xpetra::Helpers<SC,LO,GO,NO>::Op2TpetraCrs(A);
      const Tpetra::CrsMatrix<SC,LO,GO,NO>    & tpB = Xpetra::Helpers<SC,LO,GO,NO>::Op2TpetraCrs(B);
            Tpetra::CrsMatrix<SC,LO,GO,NO>    & tpC = Xpetra::Helpers<SC,LO,GO,NO>::Op2NonConstTpetraCrs(C);
      const RCP<Tpetra::Vector<SC,LO,GO,NO> > & tpD = toTpetra(Dinv);
      Tpetra::MatrixMatrix::Jacobi(omega,*tpD,tpA,tpB,tpC,haveMultiplyDoFillComplete,label);
#else
      throw(Xpetra::Exceptions::RuntimeError("Xpetra must be compiled with Tpetra."));
#endif
    }

    if (call_FillComplete_on_result && !haveMultiplyDoFillComplete) {
      RCP<Teuchos::ParameterList> params = rcp(new Teuchos::ParameterList());
      params->set("Optimize Storage", doOptimizeStorage);
      C.fillComplete(B.getDomainMap(), B.getRangeMap(), params);
    }

    // transfer striding information
    RCP<Xpetra::Matrix<SC,LO,GO,NO> > rcpA = Teuchos::rcp_const_cast<Xpetra::Matrix<SC,LO,GO,NO> >(Teuchos::rcpFromRef(A));
    RCP<Xpetra::Matrix<SC,LO,GO,NO> > rcpB = Teuchos::rcp_const_cast<Xpetra::Matrix<SC,LO,GO,NO> >(Teuchos::rcpFromRef(B));
    C.CreateView("stridedMaps", rcpA, false, rcpB, false); // TODO use references instead of RCPs
  } // end Jacobi

#if defined(HAVE_XPETRA_EPETRA) && !defined(XPETRA_EPETRA_NO_32BIT_GLOBAL_INDICES)
  template<>
  void Jacobi_kokkos<double,int,int,EpetraNode>(double omega,
                                         const Xpetra::Vector<double,int,int,EpetraNode> & Dinv,
                                         const Xpetra::Matrix<double,int,int,EpetraNode> & A,
                                         const Xpetra::Matrix<double,int,int,EpetraNode> & B,
                                         Xpetra::Matrix<double,int,int,EpetraNode> &C,
                                         bool call_FillComplete_on_result,
                                         bool doOptimizeStorage,
                                         const std::string & label) {
    typedef double        SC;
    typedef int           LO;
    typedef int           GO;
    typedef EpetraNode    NO;

    TEUCHOS_TEST_FOR_EXCEPTION(C.getRowMap()->isSameAs(*A.getRowMap()) == false, Exceptions::RuntimeError,
                               "XpetraExt::MatrixMatrix::Jacobi: row map of C is not same as row map of A")
    TEUCHOS_TEST_FOR_EXCEPTION(C.getRowMap()->isSameAs(*B.getRowMap()) == false, Exceptions::RuntimeError,
                               "XpetraExt::MatrixMatrix::Jacobi: row map of C is not same as row map of B");
    TEUCHOS_TEST_FOR_EXCEPTION(!A.isFillComplete(), Exceptions::RuntimeError, "A is not fill-completed");
    TEUCHOS_TEST_FOR_EXCEPTION(!B.isFillComplete(), Exceptions::RuntimeError, "B is not fill-completed");

    bool haveMultiplyDoFillComplete = call_FillComplete_on_result && doOptimizeStorage;

    if (C.getRowMap()->lib() == Xpetra::UseEpetra) {
#ifndef HAVE_XPETRA_EPETRAEXT
      throw(Xpetra::Exceptions::RuntimeError("Xpetra::IteratorOps::Jacobi requires EpetraExt to be compiled."));
#else
      Epetra_CrsMatrix& epA = Xpetra::Helpers<SC,LO,GO,NO>::Op2NonConstEpetraCrs(A);
      Epetra_CrsMatrix& epB = Xpetra::Helpers<SC,LO,GO,NO>::Op2NonConstEpetraCrs(B);
      Epetra_CrsMatrix& epC = Xpetra::Helpers<SC,LO,GO,NO>::Op2NonConstEpetraCrs(C);
      // FIXME
      XPETRA_DYNAMIC_CAST(const EpetraVectorT<GO COMMA NO>, Dinv, epD, "Xpetra::IteratorOps::Jacobi() only accepts Xpetra::EpetraVector as input argument.");

      int i = EpetraExt::MatrixMatrix::Jacobi(omega, *epD.getEpetra_Vector(), epA, epB, epC, haveMultiplyDoFillComplete);
      if (i != 0) {
        std::ostringstream buf;
        buf << i;
        std::string msg = "EpetraExt::MatrixMatrix::Jacobi return value of " + buf.str();
        throw(Exceptions::RuntimeError(msg));
      }
#endif
    } else if (C.getRowMap()->lib() == Xpetra::UseTpetra) {
#ifdef HAVE_XPETRA_TPETRA
# if ((defined(EPETRA_HAVE_OMP) && (!defined(HAVE_TPETRA_INST_OPENMP) || !defined(HAVE_TPETRA_INST_INT_INT))) || \
     (!defined(EPETRA_HAVE_OMP) && (!defined(HAVE_TPETRA_INST_SERIAL) || !defined(HAVE_TPETRA_INST_INT_INT))))
      throw(Xpetra::Exceptions::RuntimeError("Xpetra must be compiled with Tpetra GO=<double,int,int> enabled."));
# else
      const Tpetra::CrsMatrix<SC,LO,GO,NO>    & tpA = Xpetra::Helpers<SC,LO,GO,NO>::Op2TpetraCrs(A);
      const Tpetra::CrsMatrix<SC,LO,GO,NO>    & tpB = Xpetra::Helpers<SC,LO,GO,NO>::Op2TpetraCrs(B);
      Tpetra::CrsMatrix<SC,LO,GO,NO>          & tpC = Xpetra::Helpers<SC,LO,GO,NO>::Op2NonConstTpetraCrs(C);
      const RCP<Tpetra::Vector<SC,LO,GO,NO> > & tpD = toTpetra(Dinv);
      Tpetra::MatrixMatrix::Jacobi(omega, *tpD, tpA, tpB, tpC, haveMultiplyDoFillComplete, label);
# endif
#else
      throw(Xpetra::Exceptions::RuntimeError("Xpetra must be compiled with Tpetra."));
#endif
    }

    if(call_FillComplete_on_result && !haveMultiplyDoFillComplete) {
      RCP<Teuchos::ParameterList> params = rcp(new Teuchos::ParameterList());
      params->set("Optimize Storage", doOptimizeStorage);
      C.fillComplete(B.getDomainMap(), B.getRangeMap(), params);
    }

    // transfer striding information
    Teuchos::RCP<Xpetra::Matrix<SC,LO,GO,NO> > rcpA = Teuchos::rcp_const_cast<Xpetra::Matrix<SC,LO,GO,NO> >(Teuchos::rcpFromRef(A));
    Teuchos::RCP<Xpetra::Matrix<SC,LO,GO,NO> > rcpB = Teuchos::rcp_const_cast<Xpetra::Matrix<SC,LO,GO,NO> >(Teuchos::rcpFromRef(B));
    C.CreateView("stridedMaps", rcpA, false, rcpB, false); // TODO use references instead of RCPs
  }
#endif

#if defined(HAVE_XPETRA_EPETRA) && !defined(XPETRA_EPETRA_NO_64BIT_GLOBAL_INDICES)
  template<>
  void Jacobi_kokkos<double,int,long long,EpetraNode>(double omega,
                                               const Xpetra::Vector<double,int,long long,EpetraNode> & Dinv,
                                               const Xpetra::Matrix<double,int,long long,EpetraNode> & A,
                                               const Xpetra::Matrix<double,int,long long,EpetraNode> & B,
                                               Xpetra::Matrix<double,int,long long,EpetraNode> &C,
                                               bool call_FillComplete_on_result,
                                               bool doOptimizeStorage,
                                               const std::string & label) {
    typedef double        SC;
    typedef int           LO;
    typedef long long     GO;
    typedef EpetraNode    NO;

    TEUCHOS_TEST_FOR_EXCEPTION(C.getRowMap()->isSameAs(*A.getRowMap()) == false, Exceptions::RuntimeError,
                               "XpetraExt::MatrixMatrix::Jacobi: row map of C is not same as row map of A")
    TEUCHOS_TEST_FOR_EXCEPTION(C.getRowMap()->isSameAs(*B.getRowMap()) == false, Exceptions::RuntimeError,
                               "XpetraExt::MatrixMatrix::Jacobi: row map of C is not same as row map of B");
    TEUCHOS_TEST_FOR_EXCEPTION(!A.isFillComplete(), Exceptions::RuntimeError, "A is not fill-completed");
    TEUCHOS_TEST_FOR_EXCEPTION(!B.isFillComplete(), Exceptions::RuntimeError, "B is not fill-completed");

    bool haveMultiplyDoFillComplete = call_FillComplete_on_result && doOptimizeStorage;

    if (C.getRowMap()->lib() == Xpetra::UseEpetra) {
#ifndef HAVE_XPETRA_EPETRAEXT
      throw(Xpetra::Exceptions::RuntimeError("Xpetra::IteratorOps::Jacobi requires EpetraExt to be compiled."));
#else
      Epetra_CrsMatrix& epA = Xpetra::Helpers<SC,LO,GO,NO>::Op2NonConstEpetraCrs(A);
      Epetra_CrsMatrix& epB = Xpetra::Helpers<SC,LO,GO,NO>::Op2NonConstEpetraCrs(B);
      Epetra_CrsMatrix& epC = Xpetra::Helpers<SC,LO,GO,NO>::Op2NonConstEpetraCrs(C);
      // FIXME
      XPETRA_DYNAMIC_CAST(const EpetraVectorT<GO COMMA NO>, Dinv, epD, "Xpetra::IteratorOps::Jacobi() only accepts Xpetra::EpetraVector as input argument.");

      int i = EpetraExt::MatrixMatrix::Jacobi(omega, *epD.getEpetra_Vector(), epA, epB, epC, haveMultiplyDoFillComplete);
      if (i != 0) {
        std::ostringstream buf;
        buf << i;
        std::string msg = "EpetraExt::MatrixMatrix::Jacobi return value of " + buf.str();
        throw(Exceptions::RuntimeError(msg));
      }
#endif
    } else if (C.getRowMap()->lib() == Xpetra::UseTpetra) {
#ifdef HAVE_XPETRA_TPETRA
# if ((defined(EPETRA_HAVE_OMP) && (!defined(HAVE_TPETRA_INST_OPENMP) || !defined(HAVE_TPETRA_INST_INT_LONG_LONG))) || \
     (!defined(EPETRA_HAVE_OMP) && (!defined(HAVE_TPETRA_INST_SERIAL) || !defined(HAVE_TPETRA_INST_INT_LONG_LONG))))
      throw(Xpetra::Exceptions::RuntimeError("Xpetra must be compiled with Tpetra GO=<double,int,long long> enabled."));
# else
      const Tpetra::CrsMatrix<SC,LO,GO,NO>    & tpA = Xpetra::Helpers<SC,LO,GO,NO>::Op2TpetraCrs(A);
      const Tpetra::CrsMatrix<SC,LO,GO,NO>    & tpB = Xpetra::Helpers<SC,LO,GO,NO>::Op2TpetraCrs(B);
      Tpetra::CrsMatrix<SC,LO,GO,NO>          & tpC = Xpetra::Helpers<SC,LO,GO,NO>::Op2NonConstTpetraCrs(C);
      const RCP<Tpetra::Vector<SC,LO,GO,NO> > & tpD = toTpetra(Dinv);
      Tpetra::MatrixMatrix::Jacobi(omega, *tpD, tpA, tpB, tpC, haveMultiplyDoFillComplete, label);
# endif
#else
      throw(Xpetra::Exceptions::RuntimeError("Xpetra must be compiled with Tpetra."));
#endif
    }

    if(call_FillComplete_on_result && !haveMultiplyDoFillComplete) {
      RCP<Teuchos::ParameterList> params = rcp(new Teuchos::ParameterList());
      params->set("Optimize Storage", doOptimizeStorage);
      C.fillComplete(B.getDomainMap(), B.getRangeMap(), params);
    }

    // transfer striding information
    Teuchos::RCP<Xpetra::Matrix<SC,LO,GO,NO> > rcpA = Teuchos::rcp_const_cast<Xpetra::Matrix<SC,LO,GO,NO> >(Teuchos::rcpFromRef(A));
    Teuchos::RCP<Xpetra::Matrix<SC,LO,GO,NO> > rcpB = Teuchos::rcp_const_cast<Xpetra::Matrix<SC,LO,GO,NO> >(Teuchos::rcpFromRef(B));
    C.CreateView("stridedMaps", rcpA, false, rcpB, false); // TODO use references instead of RCPs
  }
#endif

  /*!
    @class IteratorOps
    @brief Xpetra utility class containing iteration operators.

    Currently it only contains routines to generate the Jacobi iteration operator

*/
  template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node>
  class IteratorOps_kokkos {
#undef XPETRA_ITERATOROPS_KOKKOS_SHORT
#include "Xpetra_UseShortNames.hpp"
  public:

    static RCP<Matrix>
    Jacobi(Scalar omega, const Vector& Dinv, const Matrix& A, const Matrix& B, RCP<Matrix> C_in, Teuchos::FancyOStream &fos, const std::string & label) {
      TEUCHOS_TEST_FOR_EXCEPTION(!A.isFillComplete(), Exceptions::RuntimeError, "A is not fill-completed");
      TEUCHOS_TEST_FOR_EXCEPTION(!B.isFillComplete(), Exceptions::RuntimeError, "B is not fill-completed");

      RCP<Matrix> C = C_in;
      if (C == Teuchos::null)
        C = MatrixFactory::Build(B.getRowMap(),Teuchos::OrdinalTraits<LO>::zero());

      Xpetra::Jacobi_kokkos<Scalar,LocalOrdinal,GlobalOrdinal,Node>(omega, Dinv, A, B, *C, true, true,label);
      C->CreateView("stridedMaps", rcpFromRef(A),false, rcpFromRef(B), false);

      return C;
    }

  };

} // end namespace Xpetra

#define XPETRA_ITERATOROPS_KOKKOS_SHORT

#endif // #ifdef HAVE_XPETRA_KOKKOS_REFACTOR

#endif /* PACKAGES_XPETRA_SUP_UTILS_XPETRA_ITERATOROPS_HPP_ */
