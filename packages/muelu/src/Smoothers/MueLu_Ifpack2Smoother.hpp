#ifndef MUELU_IFPACK2_SMOOTHER_HPP
#define MUELU_IFPACK2_SMOOTHER_HPP

#include "MueLu_ConfigDefs.hpp"

#ifdef HAVE_MUELU_IFPACK2

#include "Ifpack2_Factory.hpp"

#include "MueLu_VerboseObject.hpp"
#include "MueLu_SmootherBase.hpp"
#include "MueLu_SmootherPrototype.hpp"
#include "MueLu_Level.hpp"
#include "MueLu_Utilities.hpp"

namespace MueLu {


  /*!
    @class IfpackSmoother2
    @brief Class that encapsulates Ifpack2 smoothers.

    //   This class creates an Ifpack2 preconditioner factory. The factory creates a smoother based on the
    //   type and ParameterList passed into the constructor. See the constructor for more information.
    */

  template <class Scalar = double, class LocalOrdinal = int, class GlobalOrdinal = LocalOrdinal, class Node = Kokkos::DefaultNode::DefaultNodeType, class LocalMatOps = typename Kokkos::DefaultKernels<void,LocalOrdinal,Node>::SparseOps> //TODO: or BlockSparseOp ?
  class Ifpack2Smoother : public SmootherPrototype<Scalar,LocalOrdinal,GlobalOrdinal,Node,LocalMatOps>
  {

#include "MueLu_UseShortNames.hpp"

  public:

    //! @name Constructors / destructors
    //@{
    //TODO: update doc for Ifpack2. Right now, it's a copy of the doc of IfpackSmoother
    /*! @brief Constructor

    The options passed into Ifpack2Smoother are those given in the Ifpack2 user's manual.

    @param type smoother type
    @param list options for the particular smoother (e.g., fill factor or damping parameter)

    Here is how to select some of the most common smoothers.

    - Gauss-Seidel
    - <tt>type</tt> = <tt>point relaxation stand-alone</tt>
    - parameter list options
    - <tt>relaxation: type</tt> = <tt>Gauss-Seidel</tt>
    - <tt>relaxation: damping factor</tt>
    - symmetric Gauss-Seidel
    - <tt>type</tt> = <tt>point relaxation stand-alone</tt>
    - parameter list options
    - <tt>relaxation: type</tt> = <tt>symmetric Gauss-Seidel</tt>
    - <tt>relaxation: damping factor</tt>
    - Chebyshev
    - <tt>type</tt> = <tt>Chebyshev</tt>
    - parameter list options
    - <tt>chebyshev: ratio eigenvalue</tt>
    - <tt>chebyshev: min eigenvalue</tt>
    - <tt>chebyshev: max eigenvalue</tt>
    - <tt>chebyshev: degree</tt>
    - <tt>chebyshev: zero starting solution</tt> (defaults to <tt>true</tt>)
    - ILU
    - <tt>type</tt> = <tt>ILU</tt>
    - parameter list options
    - <tt>fact: level-of-fill</tt>

    See also Ifpack2_Relaxation, Ifpack2_Chebyshev, Ifpack2_ILUT.
    */
    Ifpack2Smoother(std::string const & type, Teuchos::ParameterList const & paramList = Teuchos::ParameterList(), LO const &overlap=0, RCP<FactoryBase> AFact = Teuchos::null) //TODO: empty paramList valid for Ifpack??
      : type_(type), paramList_(paramList), overlap_(overlap), AFact_(AFact)
    { }

    //! Destructor
    virtual ~Ifpack2Smoother() {}

    //@}

    //! @name Set/Get methods
    //@{

   //! Set smoother parameters
    void SetParameters(Teuchos::ParameterList const & paramList) {
      paramList_ = paramList;

      if (SmootherPrototype::IsSetup()) {
        // It might be invalid to change parameters after the setup, but it depends entirely on Ifpack implementation.
        // TODO: I don't know if Ifpack returns an error code or exception or ignore parameters modification in this case...

        Teuchos::ParameterList nonConstParamList = paramList; // because Ifpack SetParameters() input argument is not const...
        prec_->SetParameters(nonConstParamList);
      }
    }

    //! Get smoother parameters
    Teuchos::ParameterList const & GetParameters() { return paramList_; }

    //@}

    //! Input
    //@{

    void DeclareInput(Level &currentLevel) const {
        currentLevel.DeclareInput("A", AFact_.get());
    }

    //@}

    //! @name Computational methods.
    //@{

    /*! @brief Set up the smoother.

    This creates the underlying Ifpack2 smoother object, copies any parameter list options
    supplied to the constructor to the Ifpack2 object, and computes the preconditioner.

    TODO The eigenvalue estimate should come from A_, not the Ifpack2 parameter list.
    */
    void Setup(Level &currentLevel) {
      Monitor m(*this, "Setup Smoother");
      if (this->IsSetup() == true) this->GetOStream(Warnings0, 0) << "Warning: MueLu::Ifpack2Smoother::Setup(): Setup() has already been called";

      RCP<Operator> A = currentLevel.Get< RCP<Operator> >("A", AFact_.get());

      if (type_ == "CHEBYSHEV") {
        Scalar maxEigenValue = paramList_.get("chebyshev: max eigenvalue",(Scalar)-1.0);
        if (maxEigenValue == -1.0) {
          maxEigenValue = Utils::PowerMethod(*A,true,10,1e-4);
          paramList_.set("chebyshev: max eigenvalue", maxEigenValue);

          this->GetOStream(Statistics1, 0) << "chebyshev: max eigenvalue" << " = " << maxEigenValue << std::endl;
        }
      }

      RCP<const Tpetra::CrsMatrix<SC, LO, GO, NO, LMO> > tpA = Utils::Op2NonConstTpetraCrs(A);
      prec_ = Ifpack2::Factory::create(type_, tpA, overlap_);

      prec_->setParameters(paramList_);
      prec_->initialize();
      prec_->compute();

      SmootherPrototype::IsSetup(true);
    }

    /*! @brief Apply the preconditioner.

    Solves the linear system <tt>AX=B</tt> using the constructed smoother.

    @param X initial guess
    @param B right-hand side
    @param InitialGuessIsZero (optional) If false, some work can be avoided. Whether this actually saves any work depends on the underlying Ifpack2 implementation.
    */
    void Apply(MultiVector& X, MultiVector const &B, bool const &InitialGuessIsZero=false) const
    {
      TEUCHOS_TEST_FOR_EXCEPTION(SmootherPrototype::IsSetup() == false, Exceptions::RuntimeError, "MueLu::IfpackSmoother::Apply(): Setup() has not been called");

      // Forward the InitialGuessIsZero option to Ifpack2
      //  TODO: It might be nice to switch back the internal
      //        "zero starting solution" option of the ifpack2 object prec_ to his
      //        initial value at the end but there is no way right now to get
      //        the current value of the "zero starting solution" in ifpack2.
      //        It's not really an issue, as prec_  can only be used by this method.
      Teuchos::ParameterList  paramList;
      if (type_ == "CHEBYSHEV") {
        paramList.set("chebyshev: zero starting solution", InitialGuessIsZero);
      }
      else if (type_ == "RELAXATION") {
        paramList.set("relaxation: zero starting solution", InitialGuessIsZero);
      }
      else if (type_ == "ILUT") {
        if (InitialGuessIsZero == false) {
          if (this->IsPrint(Warnings0, 0)) {
            static int warning_only_once=0;
            if ((warning_only_once++) == 0)
              this->GetOStream(Warnings0, 0) << "Warning: MueLu::Ifpack2Smoother::Apply(): ILUT has no provision for a nonzero initial guess." << std::endl;
            // TODO: ILUT using correction equation should be implemented in ifpack2 directly
            //       I think that an option named "zero starting solution"
            //       is also appropriate for ILUT
          }
        }
      } else {
        // TODO: When https://software.sandia.gov/bugzilla/show_bug.cgi?id=5283#c2 is done
        // we should remove the if/else/elseif and just test if this
        // option is supported by current ifpack2 preconditioner
        TEUCHOS_TEST_FOR_EXCEPTION(true, Exceptions::RuntimeError,"IfpackSmoother::Apply(): Ifpack preconditioner '"+type_+"' not supported");
      }
      prec_->setParameters(paramList);

      // Apply
      Tpetra::MultiVector<SC,LO,GO,NO> &tpX = Utils::MV2NonConstTpetraMV(X);
      Tpetra::MultiVector<SC,LO,GO,NO> const &tpB = Utils::MV2TpetraMV(B);
      prec_->apply(tpB,tpX);
    }

    //@}

    //! @name Utilities
    //@{

    RCP<SmootherPrototype> Copy() const {
      return rcp(new Ifpack2Smoother(*this) );
    }

    //@}

    //! @name Overridden from Teuchos::Describable 
    //@{
    
    //! Return a simple one-line description of this object.
    std::string description() const {
      std::ostringstream out;
      out << SmootherPrototype::description();
      out << "{type = " << type_ << "}";
      return out.str();
    }
    
    //! Print the object with some verbosity level to an FancyOStream object.
    //using MueLu::Describable::describe; // overloading, not hiding
    //void describe(Teuchos::FancyOStream &out, const VerbLevel verbLevel = Default) const {
    void print(Teuchos::FancyOStream &out, const VerbLevel verbLevel = Default) const {
      MUELU_DESCRIBE;

      if (verbLevel & Parameters0) {
        out0 << "Prec. type: " << type_ << endl;
      }
      
      if (verbLevel & Parameters1) { 
        out0 << "Parameter list: " << endl; { Teuchos::OSTab tab2(out); out << paramList_; }
        out0 << "Overlap: "        << overlap_ << std::endl;
      }
      
      if (verbLevel & External) {
        if (prec_ != Teuchos::null) { Teuchos::OSTab tab2(out); out << *prec_ << std::endl; }
      }

      if (verbLevel & Debug) {
        out0 << "IsSetup: " << Teuchos::toString(SmootherPrototype::IsSetup()) << endl
             << "-" << endl
             << "RCP<prec_>: " << prec_ << std::endl;
      }
    }

    //@}

  private:

    //! ifpack2-specific key phrase that denote smoother type
    std::string type_;

    //! parameter list that is used by Ifpack internally
    Teuchos::ParameterList paramList_;

    //! overlap when using the smoother in additive Schwarz mode
    LO overlap_;

    //! pointer to Ifpack2 preconditioner object
    RCP<Ifpack2::Preconditioner<Scalar,LocalOrdinal,GlobalOrdinal,Node> > prec_;

    //! A Factory
    RCP<FactoryBase> AFact_;

  }; // class Ifpack2Smoother

} // namespace MueLu

#define MUELU_IFPACK2_SMOOTHER_SHORT
#endif //ifdef HAVE_MUELU_IFPACK2
#endif //ifndef MUELU_IFPACK2_SMOOTHER_HPP
