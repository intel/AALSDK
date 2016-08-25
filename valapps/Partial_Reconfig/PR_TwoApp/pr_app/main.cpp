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
/// @brief Partial reconfiguration of green bit stream service .
/// @ingroup Partial_Reconfig
/// @verbatim
/// AAL Partial reconfiguration test application
///
///    This application is for testing purposes only.
///    It is not intended to represent a model for developing commercially-
///       deployable applications.
///    It is designed to test PR functionality.
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
#include <aalsdk/aalclp/aalclp.h>

#include "../../PR_SingleApp/ALIReconf.h"

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

/// Command Line
BEGIN_C_DECLS

struct ALIConfigCommandLine configCmdLine = { 0,"",1,0,0 };

AALCLP_DECLARE_GCS_COMPLIANT(stdout,
                             "aliconfafu",
                             "0",
                             "",
                             help_msg_callback,
                             &configCmdLine);
END_C_DECLS


class ALIConfAFUApp: public CAASBase, public IRuntimeClient
{
public:

   ALIConfAFUApp();
   ~ALIConfAFUApp();

   // <begin IRuntimeClient interface>
   void runtimeCreateOrGetProxyFailed(IEvent const &rEvent){};    // Not Used
   void runtimeStarted(IRuntime *pRuntime, const NamedValueSet &rConfigParms);
   void runtimeStopped(IRuntime *pRuntime);
   void runtimeStartFailed(const IEvent &rEvent);
   void runtimeStopFailed(const IEvent &rEvent);
   void runtimeAllocateServiceFailed( IEvent const &rEvent);
   void runtimeAllocateServiceSucceeded(IBase *pClient, TransactionID const &rTranID);
   void runtimeEvent(const IEvent &rEvent);

   btBool allocRecongService();
   btBool runTests();
   btBool FreeRuntime();
   void PrintReconfExceptionDescription(IEvent const &theEvent);

protected:
   Runtime                   m_Runtime;           ///< AAL Runtime
   IBase                    *m_pAALService;       ///< The generic AAL Service interface for the AFU.
   CSemaphore                m_Sem;               ///< For synchronizing with the AAL runtime.
   btInt                     m_Result;            ///< Returned result v; 0 if success
   AllocatesReconfService    m_reconfAFU;
   btInt                     m_Errors;
};

///////////////////////////////////////////////////////////////////////////////
///
///  Implementation
///
///////////////////////////////////////////////////////////////////////////////

/// @brief   Constructor registers this objects client interfaces and starts
///          the AAL Runtime. The member m_bisOK is used to indicate an error.
///
ALIConfAFUApp::ALIConfAFUApp() :
   m_Runtime(this),\
   m_pAALService(NULL),
   m_Result(0)
{

   SetInterface(iidRuntimeClient, dynamic_cast<IRuntimeClient *>(this));

   m_Sem.Create(0, 1);

   NamedValueSet configArgs;
   NamedValueSet configRecord;

   // Specify that the remote resource manager is to be used.
   configRecord.Add(AALRUNTIME_CONFIG_BROKER_SERVICE, "librrmbroker");
   configArgs.Add(AALRUNTIME_CONFIG_RECORD, &configRecord);

   // Start the Runtime and wait for the callback by sitting on the semaphore.
   // the runtimeStarted() or runtimeStartFailed() callbacks should set m_bIsOK appropriately.
   if(!m_Runtime.start(configArgs)){
	   m_bIsOK = false;
      return;
   }
   m_Sem.Wait();
   m_bIsOK = true;
}

/// @brief   Destructor
///
ALIConfAFUApp::~ALIConfAFUApp()
{
   m_Sem.Destroy();
}

void ALIConfAFUApp::runtimeStarted( IRuntime            *pRuntime,
                                    const NamedValueSet &rConfigParms)
{
   m_bIsOK = true;
   m_Sem.Post(1);
}

void ALIConfAFUApp::runtimeStopped(IRuntime *pRuntime)
{
   MSG("Runtime stopped");
   m_bIsOK = false;
   m_Sem.Post(1);
}

void ALIConfAFUApp::runtimeStartFailed(const IEvent &rEvent)
{
   ERR("Runtime start failed");
   PrintExceptionDescription(rEvent);
}

void ALIConfAFUApp::runtimeStopFailed(const IEvent &rEvent)
{
   MSG("Runtime stop failed");
   m_bIsOK = false;
   m_Sem.Post(1);
}

void ALIConfAFUApp::runtimeAllocateServiceFailed( IEvent const &rEvent)
{
   ERR("Runtime AllocateService failed");
   PrintExceptionDescription(rEvent);
}

void ALIConfAFUApp::runtimeAllocateServiceSucceeded(IBase *pClient,
                                              TransactionID const &rTranID)
{
   MSG("Runtime Allocate Service Succeeded");
}

void ALIConfAFUApp::runtimeEvent(const IEvent &rEvent)
{
   MSG("Generic message handler (runtime)");
}

void ALIConfAFUApp::PrintReconfExceptionDescription(IEvent const &rEvent)
{
   if ( rEvent.Has(iidExTranEvent) ) {
      std::cerr << "Description: " << dynamic_ref<IExceptionTransactionEvent>(iidExTranEvent, rEvent).Description() << std::endl;
      std::cerr << "ExceptionNumber: " << dynamic_ref<IExceptionTransactionEvent>(iidExTranEvent, rEvent).ExceptionNumber() << std::endl;
      std::cerr << "Reason: " << dynamic_ref<IExceptionTransactionEvent>(iidExTranEvent, rEvent).Reason() << std::endl;
   }
}
 // <begin IRuntimeClient interface>


btBool ALIConfAFUApp::allocRecongService()
{
   m_reconfAFU.AllocatePRService(&m_Runtime);
   //strcpy(configCmdLine.bitstream_file,BitStreamFile);

   if(false == m_reconfAFU.IsOK() ) {
      ++m_Errors;
      ERR("--- Failed to Allocate Reconfigure Service --- ");
      return false;
   }
   m_reconfAFU.setreconfnvs(configCmdLine.bitstream_file1,1000,AALCONF_RECONF_ACTION_HONOR_OWNER_ID,false);
   return true;

}

btBool ALIConfAFUApp::runTests()
{
   if( false == allocRecongService() ) {
      ERR("--- Failed to allocate Reconf Service  --- ");
      return false;
   }

   m_reconfAFU.setreconfnvs(configCmdLine.bitstream_file1,configCmdLine.reconftimeout,configCmdLine.reconfAction,configCmdLine.reactivateDisabled);

   TEST_CASE(" configCmdLine.reconfInterface");
   cout << "configCmdLine.reconfInterface" << configCmdLine.reconfInterface<< endl;


   if ( 0 == strcmp("D", configCmdLine.reconfInterface)) {

      // DeActiavte AFU
      m_reconfAFU.reconfDeactivate();

   } else if ( 0 == strcmp("A", configCmdLine.reconfInterface)) {

      // Actiavte AFU
      m_reconfAFU.reconfActivate();

   }else if ( 0 == strcmp("C", configCmdLine.reconfInterface))
   {
      // Reconfigure  AFU
      m_reconfAFU.reconfConfigure();

   } else if ( 0 == strcmp("ALL", configCmdLine.reconfInterface))  {

      m_reconfAFU.reconfDeactivate();
      m_reconfAFU.reconfConfigure();
      m_reconfAFU.reconfActivate();
   }
   m_reconfAFU.FreePRService();

   return true;
}

btBool ALIConfAFUApp::FreeRuntime()
{
   m_Runtime.stop();
   m_Sem.Wait();
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
   btInt Result =0;

   if ( argc < 2 ) {
      showhelp(stdout, &_aalclp_gcs_data);
      return 1;
   } else if ( 0!= ParseCmds(&configCmdLine, argc, argv) ) {
      cerr << "Error scanning command line." << endl;
      return 2;
   }else if ( flag_is_set(configCmdLine.flags, ALICONIFG_CMD_FLAG_HELP|ALICONIFG_CMD_FLAG_VERSION) ) {
      return 0;
   }

   ALIConfAFUApp theApp;
   if(!theApp.IsOK()){
      ERR("Runtime Failed to Start");
      exit(1);
   }

   theApp.runTests();
   theApp.FreeRuntime();
   MSG("Done");
   return Result;
}

