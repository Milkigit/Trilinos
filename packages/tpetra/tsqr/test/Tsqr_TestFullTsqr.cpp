//@HEADER
// ************************************************************************
//
//          Kokkos: Node API and Parallel Node Kernels
//              Copyright (2008) Sandia Corporation
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
// ************************************************************************
//@HEADER

#include "Tsqr_FullTsqrTest.hpp"
#include "Tsqr_Test_MpiAndKokkosScope.cpp"
#include "Teuchos_CommandLineProcessor.hpp"
#include "Teuchos_StandardCatchMacros.hpp"

#ifdef HAVE_TPETRATSQR_COMPLEX
#  include <complex>
#endif // HAVE_TPETRATSQR_COMPLEX

namespace {
  using Teuchos::parameterList;

  // Documentation string to print out if --help is a command-line
  // argument.
  const char docString[] = "This program tests correctness and "
    "accuracy of TSQR::Tsqr, which is the full implementation of "
    "TSQR.";

  // Encapsulation of all command-line parameters.
  struct CmdLineOptions {
    // Given a default valid parameter list from
    // FullTsqrVerifierCaller, fill in the command-line options with
    // their default values.
    CmdLineOptions(const Teuchos::RCP<const Teuchos::ParameterList>& testParams) :
      cacheSizeHint(testParams->get<size_t>("Cache Size Hint")),
      numRowsLocal(testParams->get<int>("numRowsLocal")),
      numCols(testParams->get<int>("numCols")),
      contiguousCacheBlocks(testParams->get<bool>("contiguousCacheBlocks")),
      testFactorExplicit(testParams->get<bool>("testFactorExplicit")),
      testRankRevealing(testParams->get<bool>("testRankRevealing")),
      printFieldNames(testParams->get<bool>("printFieldNames")),
      printResults(testParams->get<bool>("printResults")),
      failIfInaccurate(testParams->get<bool>("failIfInaccurate")),
      nodeTsqr(testParams->get<std::string>("NodeTsqr")),
#ifdef HAVE_TPETRATSQR_COMPLEX
      testComplex(true),
#else
      testComplex(false),
#endif // HAVE_TPETRATSQR_COMPLEX
      testReal(true),
      verbose(testParams->get<bool>("verbose"))
    {}

    size_t cacheSizeHint = 0;
    int numRowsLocal = 10000;
    int numCols = 5;
    bool contiguousCacheBlocks = false;
    bool testFactorExplicit = true;
    bool testRankRevealing = true;
    bool printFieldNames = true;
    bool printResults = true;
    bool failIfInaccurate = true;
    std::string nodeTsqr {"Default"};
#ifdef HAVE_TPETRATSQR_COMPLEX
    bool testComplex = true;
#else
    bool testComplex = false;
#endif // HAVE_TPETRATSQR_COMPLEX
    bool testReal = true;
    bool verbose = false;

    // \brief Read command-line options.
    //
    // We use Doxygen notation to document this function, but don't tell
    // Doxygen to generate documentation, since this method is local to
    // this test.
    //
    // \param argc [in] As usual in C(++).
    //
    // \param argv [in] As usual in C(++).
    //
    // \param testParams [in] List of test parameters for the
    //   FullTsqrVerifierCaller.
    //
    // \param err [out] Output stream to which to print error
    //   messages.  Different per (MPI) process.
    //
    // \return Whether help was printed.
    bool
    read(int argc,
         char* argv[],
         const Teuchos::RCP<const Teuchos::ParameterList>& defaultParams,
         std::ostream& err)
    {
      using Teuchos::CommandLineProcessor;
      using std::endl;

      try {
        const bool throwExceptions = true;
        const bool recognizeAllOptions = true;
        CommandLineProcessor cmdLineProc(throwExceptions,
                                         recognizeAllOptions);
        cmdLineProc.setDocString(docString);
        cmdLineProc.setOption("testReal",
                              "noTestReal",
                              &testReal,
                              "Test real Scalar types");
        cmdLineProc.setOption("testComplex",
                              "noTestComplex",
                              &testComplex,
                              "Test complex Scalar types; must be "
                              "false if complex Scalar types were "
                              "disabled at configure (pre-build) "
                              "time");
        // CommandLineProcessor takes int arguments, but not size_t
        // arguments, so we have to read in the argument as an int and
        // convert back to size_t later.
        int cacheSizeHintAsInt = cacheSizeHint;
        cmdLineProc.setOption("cacheSizeHint",
                              &cacheSizeHintAsInt,
                              defaultParams->getEntry
                              ("Cache Size Hint").docString().c_str());
        cmdLineProc.setOption("numRowsLocal",
                              &numRowsLocal,
                              defaultParams->getEntry
                              ("numRowsLocal").docString().c_str());
        cmdLineProc.setOption("numCols",
                              &numCols,
                              defaultParams->getEntry
                              ("numCols").docString().c_str());
        cmdLineProc.setOption("contiguousCacheBlocks",
                              "noContiguousCacheBlocks",
                              &contiguousCacheBlocks,
                              defaultParams->getEntry
                              ("contiguousCacheBlocks").docString().c_str());
        cmdLineProc.setOption("testFactorExplicit",
                              "noTestFactorExplicit",
                              &testFactorExplicit,
                              defaultParams->getEntry
                              ("testFactorExplicit").docString().c_str());
        cmdLineProc.setOption("testRankRevealing",
                              "noTestRankRevealing",
                              &testRankRevealing,
                              defaultParams->getEntry
                              ("testRankRevealing").docString().c_str());
        cmdLineProc.setOption("printFieldNames",
                              "noPrintFieldNames",
                              &printFieldNames,
                              defaultParams->getEntry
                              ("printFieldNames").docString().c_str());
        cmdLineProc.setOption("printResults",
                              "noPrintResults",
                              &printResults,
                              defaultParams->getEntry
                              ("printResults").docString().c_str());
        cmdLineProc.setOption("failIfInaccurate",
                              "noFailIfInaccurate",
                              &failIfInaccurate,
                              defaultParams->getEntry
                              ("failIfInaccurate").docString().c_str());
        cmdLineProc.setOption("NodeTsqr",
                              &nodeTsqr,
                              defaultParams->getEntry
                              ("NodeTsqr").docString().c_str());
        cmdLineProc.setOption("verbose",
                              "quiet",
                              &verbose,
                              defaultParams->getEntry
                              ("verbose").docString().c_str());
        cmdLineProc.parse(argc, argv);
        cacheSizeHint = size_t(cacheSizeHintAsInt);
      }
      catch(Teuchos::CommandLineProcessor::UnrecognizedOption& e) {
        err << "Unrecognized command-line option: " << e.what()
            << endl;
        throw e;
      }
      catch(Teuchos::CommandLineProcessor::HelpPrinted& e) {
        return true;
      }

      // Validate command-line options.  We provide default values
      // for unset options, so we don't have to validate those.
      TEUCHOS_TEST_FOR_EXCEPTION
        (numRowsLocal <= 0, std::invalid_argument,
         "Number of rows per process must be positive.");
      TEUCHOS_TEST_FOR_EXCEPTION
        (numCols <= 0, std::invalid_argument,
         "Number of columns must be positive.");
      return false; // Did not print help
    }
  };

  //
  // Given a default valid parameter list from FullTsqrVerifierCaller,
  // and the values of command-line options (that were read in from
  // the command line), return a parameter list describing the test.
  //
  Teuchos::RCP<Teuchos::ParameterList>
  testParameters(const Teuchos::RCP<const Teuchos::ParameterList>& validParams,
                 const CmdLineOptions& options)
  {
    auto testParams = parameterList ("FullTsqrVerifier");
    testParams->set("Cache Size Hint", options.cacheSizeHint);
    testParams->set("numRowsLocal", options.numRowsLocal);
    testParams->set("numCols", options.numCols);
    testParams->set("testFactorExplicit",
                    options.testFactorExplicit);
    testParams->set("testRankRevealing", options.testRankRevealing);
    testParams->set("contiguousCacheBlocks",
                    options.contiguousCacheBlocks);
    testParams->set("printFieldNames", options.printFieldNames);
    testParams->set("printResults", options.printResults);
    testParams->set("failIfInaccurate", options.failIfInaccurate);
    testParams->set("NodeTsqr", options.nodeTsqr);
    testParams->set("verbose", options.verbose);

    testParams->validateParametersAndSetDefaults(*validParams);
    return testParams;
  }

  // Return true if all tests were successful, else false.
  bool
  test(int argc,
       char* argv[],
       const Teuchos::RCP<const Teuchos::Comm<int> >& comm,
       std::ostream& err)
  {
    using TSQR::Test::NullCons;
    using TSQR::Test::Cons;
    using Teuchos::null;
    using Teuchos::ParameterList;
    using Teuchos::parameterList;
    using Teuchos::RCP;
    using Teuchos::rcp;

    // The Caller iterates the test over all Scalar types.
    using caller_type = TSQR::Test::FullTsqrVerifierCaller;
    caller_type caller(comm, caller_type::defaultRandomSeed ());

    // Read command-line options
    auto defaultParams = caller.getValidParameterList();
    CmdLineOptions cmdLineOpts(defaultParams);
    const bool printedHelp =
      cmdLineOpts.read(argc, argv, defaultParams, err);
    // Don't run the tests (and do succeed) if help was printed.
    if(printedHelp) {
      return true;
    }

    //
    // Use read-in command-line options to set up test parameters.
    //
    auto testParams = testParameters(defaultParams, cmdLineOpts);
    defaultParams = null; // save a little space

    // Define lists of Scalar types to test.  We keep separate lists
    // for real and complex types, since callers can control whether
    // each of these is tested independently on the command line.
    using real_type_list = Cons<float, Cons<double, NullCons>>;
#ifdef HAVE_TPETRATSQR_COMPLEX
    using complex_type_list = Cons<std::complex<float>, Cons<std::complex<double>, NullCons>>;
#endif // HAVE_TPETRATSQR_COMPLEX

    // Run the tests.  If the tests are set up to fail on
    // insufficiently inaccurate results, run() will throw an
    // exception in that case.  Otherwise, the tests return nothing,
    // and "succeed" if they don't crash or throw an exception.
    //
    // The testReal and testComplex options are read in at the command
    // line, but since they do not apply to all Scalar types, they
    // don't belong in testParams.
    const bool realResult = cmdLineOpts.testReal ?
      caller.run<real_type_list>(testParams) :
      true;
#ifdef HAVE_TPETRATSQR_COMPLEX
    const bool complexResult = cmdLineOpts.testComplex ?
      caller.run<complex_type_list>(testParams) :
      true;
#else
    const bool complexResult = true;
#endif // HAVE_TPETRATSQR_COMPLEX

    return realResult && complexResult;
  }
} // namespace (anonymous)


int
main (int argc, char* argv[])
{
  using std::endl;
  TSQR::Test::MpiAndKokkosScope testScope(&argc, &argv);
  auto comm = testScope.getComm();
  std::ostream& out = testScope.outStream();
  std::ostream& err = testScope.errStream();

  constexpr bool actually_print_caught_exceptions = true;
  bool success = false; // hopefully this will be true later
  try {
    success = test(argc, argv, comm, err);
    if(success) {
      // The Trilinos test framework expects a message like this.
      out << "\nEnd Result: TEST PASSED" << endl;
    }
  }
  TEUCHOS_STANDARD_CATCH_STATEMENTS
    (actually_print_caught_exceptions, err, success);
  return ( success ? EXIT_SUCCESS : EXIT_FAILURE );
}
