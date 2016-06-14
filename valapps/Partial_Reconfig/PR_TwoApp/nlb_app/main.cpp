// INTEL CONFIDENTIAL - For Intel Internal Use Only
//
// valapps/Partial_Reconfig/PR_TwoApp/main.cpp
//
// Copyright(c) 2007-2016, Intel Corporation
//
// Redistribution  and  use  in source  and  binary  forms,  with  or  without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of  source code  must retain the  above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name  of Intel Corporation  nor the names of its contributors
//   may be used to  endorse or promote  products derived  from this  software
//   without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  BUT NOT LIMITED TO,  THE
// IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT  SHALL THE COPYRIGHT OWNER  OR CONTRIBUTORS BE
// LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR
// CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT LIMITED  TO,  PROCUREMENT  OF
// SUBSTITUTE GOODS OR SERVICES;  LOSS OF USE,  DATA, OR PROFITS;  OR BUSINESS
// INTERRUPTION)  HOWEVER CAUSED  AND ON ANY THEORY  OF LIABILITY,  WHETHER IN
// CONTRACT,  STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE  OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,  EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//****************************************************************************
/// @file main.cpp
/// @brief NLB Service and run NLB algorithm on fpga  .
/// @ingroup Partial_Reconfig
/// @verbatim
/// AAL NLB test application
///
///    This application is for testing purposes only.
///    It is not intended to represent a model for developing commercially-
///       deployable applications.
///    It is designed to test NLB functionality.
///
///
/// This Sample demonstrates how to use the basic ALI APIs.
///
/// This sample is designed to be used with the xyzALIAFU Service.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 06/06/2016     RP       Initial version started based on older sample
//****************************************************************************

#include <aalsdk/AALTypes.h>
#include <aalsdk/Runtime.h>
#include <aalsdk/AALLoggerExtern.h>
#include <aalsdk/service/IALIAFU.h>

#include "../../PR_SingleApp/ALINLB.h"

using namespace std;
using namespace AAL;

// Convenience macros for printing messages and errors.
#ifdef MSG
# undef MSG
#endif // MSG
#define MSG(x) std::cout << __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() : " << x << std::endl
#define TEST_CASE(x) std::cout << std::endl << x << std::endl
#ifdef ERR
# undef ERR
#endif // ERR
#define ERR(x) std::cerr << __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() **Error : " << x << std::endl



class HelloNLBTestApp: public CAASBase, public IRuntimeClient
{
public:

   HelloNLBTestApp();
   ~HelloNLBTestApp();

   // <begin IRuntimeClient interface>
   void runtimeCreateOrGetProxyFailed(IEvent const &rEvent){};
   void runtimeStarted(IRuntime  *pRuntime, const NamedValueSet &rConfigParms);
   void runtimeStopped(IRuntime *pRuntime);
   void runtimeStartFailed(const IEvent &rEvent);
   void runtimeStopFailed(const IEvent &rEvent);
   void runtimeAllocateServiceFailed( IEvent const &rEvent);
   void runtimeAllocateServiceSucceeded(IBase  *pClient, TransactionID const &rTranID);
   void runtimeEvent(const IEvent &rEvent);
   // <end IRuntimeClient interface>

   btBool FreeRuntime();
   btBool runTests(btBool brunloop,btBool bReleaseMode);
   btBool allocNLBService();
   Runtime* getRuntime() { return &m_Runtime;}
   btInt Errors() const { return m_Errors; }
   void setReleaseSevice(btBool bReleaseService) { m_bReleaseService = bReleaseService ; }
   void setTestMode(btBool btestMode) { m_btestMode = btestMode ; }

protected:
   Runtime               m_Runtime;
   CSemaphore            m_Sem;
   btInt                 m_Result;
   btInt                 m_Errors;
   AllocatesNLBService   m_helloALINLB ;
   btBool                m_btestMode ;
   btBool                m_bReleaseService;

};

///////////////////////////////////////////////////////////////////////////////
///
///  Implementation
///
///////////////////////////////////////////////////////////////////////////////

/// @brief   Constructor registers this objects client interfaces and starts
///          the AAL Runtime. The member m_bisOK is used to indicate an error.
///
HelloNLBTestApp::HelloNLBTestApp() :
   m_Runtime(this),
   m_Result(0),
   m_Errors(0),
   m_btestMode(false),
   m_bReleaseService(true)


{
   SetInterface(iidRuntimeClient, dynamic_cast<IRuntimeClient *>(this));

   m_Sem.Create(0, 1);

   NamedValueSet configArgs;
   NamedValueSet configRecord;

   // Specify that the remote resource manager is to be used.
   configRecord.Add(AALRUNTIME_CONFIG_BROKER_SERVICE, "librrmbroker");
   configArgs.Add(AALRUNTIME_CONFIG_RECORD, &configRecord);

   if(!m_Runtime.start(configArgs)){
      m_bIsOK = false;
      return;
   }
   m_Sem.Wait();
   m_bIsOK = true;
}

/// @brief   Destructor
///
HelloNLBTestApp::~HelloNLBTestApp()
{
   m_Sem.Destroy();
}


void HelloNLBTestApp::runtimeStarted( IRuntime            *pRuntime,
                                      const NamedValueSet &rConfigParms)
{
   m_bIsOK = true;
   m_Sem.Post(1);
}

void HelloNLBTestApp::runtimeStopped(IRuntime *pRuntime)
{
   MSG("Runtime stopped");
   m_bIsOK = false;
   m_Sem.Post(1);
}

void HelloNLBTestApp::runtimeStartFailed(const IEvent &rEvent)
{
   ERR("Runtime start failed");
   PrintExceptionDescription(rEvent);
}

void HelloNLBTestApp::runtimeStopFailed(const IEvent &rEvent)
{
   MSG("Runtime stop failed");
   m_bIsOK = false;
   m_Sem.Post(1);
}

void HelloNLBTestApp::runtimeAllocateServiceFailed( IEvent const &rEvent)
{
   ERR("Runtime AllocateService failed");
   PrintExceptionDescription(rEvent);
}

void HelloNLBTestApp::runtimeAllocateServiceSucceeded(IBase *pClient,
                                                      TransactionID const &rTranID)
{
   MSG("Runtime Allocate Service Succeeded");
}

void HelloNLBTestApp::runtimeEvent(const IEvent &rEvent)
{
   MSG("Generic message handler (runtime)");
}

btBool HelloNLBTestApp::FreeRuntime()
{
   m_Runtime.stop();
   m_Sem.Wait();

}

btBool HelloNLBTestApp::allocNLBService()
{
   m_helloALINLB.AllocateNLBService(&m_Runtime);

   if(false == m_helloALINLB.IsOK() ) {
      ++m_Errors;
      ERR("--- Failed to Allocate Reconfigure Service --- ");
      return false;
   }

}

btBool HelloNLBTestApp::runTests(btBool brunloop,btBool bReleaseMode)
{
   if( false == allocNLBService() )
   {
      ERR("--- Failed to allocate Reconf Service  --- ");
      return false;
   }

   TEST_CASE("-------- START TEST CASES  ---------");

   m_helloALINLB.setReleaseService(bReleaseMode);
   if(brunloop == true)
      m_helloALINLB.runInLoop();
   else
      m_helloALINLB.run();

   TEST_CASE("-------- END TEST CASES  ---------");

   m_helloALINLB.FreeNLBService();

   return true;
}

//=============================================================================
// Name: main
// Description: Entry point to the application
// Inputs: none
// Outputs: none
// Comments: Main initializes the system. The rest of the example is implemented
//           in the object theApp.
//=============================================================================
int main(int argc, char *argv[])
{
   btBool brunloop = false;
   btBool bReleaseMode = true;
   btInt Result;

   HelloNLBTestApp theApp;
   if(!theApp.IsOK()){
      ERR("Runtime Failed to Start");
      exit(1);
   }

   if( argc>1 ) { // process command line arg of "-c"
      if (0 == strcmp (argv[1], "--loop")) brunloop = true ;
   }

   if( argc>2 ) { // process command line arg of "-c"
      if (0 == strcmp (argv[2], "--NoRelease")) bReleaseMode = false ;
   }

   // Run Test cases
   theApp.runTests(brunloop, bReleaseMode);

   theApp.FreeRuntime();
   MSG("Done");
   return Result;
}

