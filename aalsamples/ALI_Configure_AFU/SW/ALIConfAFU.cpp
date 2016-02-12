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
/// @file ALIConfAFU.cpp
/// @brief Basic ALI AFU interaction.
/// @ingroup HelloALINLB
/// @verbatim
/// Intel(R) Accelerator Abstraction Layer Sample Application
///
///    This application is for example purposes only.
///    It is not intended to represent a model for developing commercially-
///       deployable applications.
///    It is designed to show working examples of the AAL programming model and APIs.
///
/// AUTHORS: Joseph Grecco, Intel Corporation.
///
/// This Sample demonstrates how to use the basic ALI APIs.
///
/// This sample is designed to be used with the xyzALIAFU Service.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 12/15/2015     JG       Initial version started based on older sample code.@endverbatim
//****************************************************************************
#include <aalsdk/AALTypes.h>
#include <aalsdk/Runtime.h>
#include <aalsdk/AALLoggerExtern.h>


#include <aalsdk/service/IALIAFU.h>
#include <aalsdk/aalclp/aalclp.h>

#include <string.h>

//****************************************************************************
// UN-COMMENT appropriate #define in order to enable either Hardware or ASE.
//    DEFAULT is to use Software Simulation.
//****************************************************************************
#define  HWAFU
//#define  ASEAFU

using namespace std;
using namespace AAL;

// Convenience macros for printing messages and errors.
#ifdef MSG
# undef MSG
#endif // MSG
#define MSG(x) std::cout << __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() : " << x << std::endl
#ifdef ERR
# undef ERR
#endif // ERR
#define ERR(x) std::cerr << __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() **Error : " << x << std::endl

// Print/don't print the event ID's entered in the event handlers.
#if 1
# define EVENT_CASE(x) case x : MSG(#x);
#else
# define EVENT_CASE(x) case x :
#endif

/// Command Line
BEGIN_C_DECLS

int aliconigafu_on_nix_long_option_only(AALCLP_USER_DEFINED , const char * );
int aliconigafu_on_nix_long_option(AALCLP_USER_DEFINED , const char * , const char * );

void help_msg_callback(FILE * , struct _aalclp_gcs_compliance_data * );
void showhelp(FILE * , struct _aalclp_gcs_compliance_data * );

struct  ALIConfigCommandLine
{
   btUIntPtr          flags;
#define ALICONIFG_CMD_FLAG_HELP      0x00000001
#define ALICONIFG_CMD_FLAG_VERSION   0x00000002
#define ALICONIFG_CMD_PARSE_ERROR    0x00000003

   char    bitstream_file[100];

};
struct ALIConfigCommandLine configCmdLine = { 0, 0 };

int aliconigafu_on_non_option(AALCLP_USER_DEFINED user, const char *nonoption) {
   struct ALIConfigCommandLine *cl = (struct ALIConfigCommandLine *)user;
   flag_setf(cl->flags, ALICONIFG_CMD_PARSE_ERROR);
   printf("Invalid: %s\n", nonoption);
   return 0;
}

int aliconigafu_on_dash_only(AALCLP_USER_DEFINED user) {
   struct ALIConfigCommandLine *cl = (struct ALIConfigCommandLine *)user;
   flag_setf(cl->flags, ALICONIFG_CMD_PARSE_ERROR);
   printf("Invalid option: -\n");
   return 0;
}

int aliconigafu_on_dash_dash_only(AALCLP_USER_DEFINED user) {
   struct ALIConfigCommandLine *cl = (struct ALIConfigCommandLine *)user;
   flag_setf(cl->flags, ALICONIFG_CMD_PARSE_ERROR);
   printf("Invalid option: --\n");
   return 0;
}

int aliconigafu_on_nix_long_option_only(AALCLP_USER_DEFINED user, const char *option)
{
   struct ALIConfigCommandLine *cl = (struct ALIConfigCommandLine *)user;
   if ( 0 == strcmp("--help", option) ) {
      flag_setf(cl->flags, ALICONIFG_CMD_FLAG_HELP);
   } else if ( 0 == strcmp("--version", option) ) {
      flag_setf(cl->flags, ALICONIFG_CMD_FLAG_VERSION);
   }else  if(0 != strcmp("--bitstream=", option))  {
      printf("Invalid option: %s\n", option);
      flag_setf(cl->flags, ALICONIFG_CMD_PARSE_ERROR);
      return 0;
   }
   return 0;
}

int aliconigafu_on_nix_long_option(AALCLP_USER_DEFINED user, const char *option, const char *value)
{
   struct ALIConfigCommandLine *pcmdline     = (struct ALIConfigCommandLine *)user;
   if ( 0 == strcmp("--bitstream", option)) {
      strcpy(pcmdline->bitstream_file ,value);
      return 3;
   }
   return 0;
}

aalclp_option_only  aliconigafu_nix_long_option_only  = { aliconigafu_on_nix_long_option_only,  };
aalclp_option       aliconigafu_nix_long_option       = { aliconigafu_on_nix_long_option,       };
aalclp_non_option   aliconigafu_non_option            = { aliconigafu_on_non_option,            };
aalclp_dash_only    aliconigafu_dash_only             = { aliconigafu_on_dash_only,             };
aalclp_dash_only    aliconigafu_dash_dash_only        = { aliconigafu_on_dash_dash_only,        };

AALCLP_DECLARE_GCS_COMPLIANT(stdout,
                             "aliconfafu",
                             "0",
                             "",
                             help_msg_callback,
                             &configCmdLine)

int ParseCmds(struct ALIConfigCommandLine *pconfigcmd, int argc, char *argv[])
{
   int    res;
   int    clean;
   aalclp clp;

   res = aalclp_init(&clp);
   if ( 0 != res ) {
      cerr << "aalclp_init() failed : " << res << ' ' << strerror(res) << endl;
      return res;
   }

   aliconigafu_nix_long_option_only.user = pconfigcmd;
   aalclp_add_nix_long_option_only(&clp, &aliconigafu_nix_long_option_only);

   aliconigafu_nix_long_option.user = pconfigcmd;
   aalclp_add_nix_long_option(&clp, &aliconigafu_nix_long_option);

   aliconigafu_non_option.user = pconfigcmd;
   aalclp_add_non_option(&clp, &aliconigafu_non_option);

   aliconigafu_dash_only.user             = pconfigcmd;
   aalclp_add_dash_only(&clp,             &aliconigafu_dash_only);

   aliconigafu_dash_dash_only.user        = pconfigcmd;
   aalclp_add_dash_dash_only(&clp,        &aliconigafu_dash_dash_only);

   res = aalclp_add_gcs_compliance(&clp);
   if ( 0 != res ) {
      cerr << "aalclp_add_gcs_compliance() failed : " << res << ' ' << strerror(res) << endl;
      goto CLEANUP;
   }

   res = aalclp_scan_argv(&clp, argc, argv);
   if ( 0 != res ) {
      cerr << "aalclp_scan_argv() failed : " << res << ' ' << strerror(res) << endl;
   }

CLEANUP:
   clean = aalclp_destroy(&clp);
   if ( 0 != clean ) {
      cerr << "aalclp_destroy() failed : " << clean << ' ' << strerror(clean) << endl;
   }

   return res;
}


void help_msg_callback(FILE *fp, struct _aalclp_gcs_compliance_data *gcs)
{
   fprintf(fp, "Usage:\n");
   fprintf(fp, "   aliconfafu [--bitstream=<FILENAME>] \n");
   fprintf(fp, "\n");

}

void showhelp(FILE *fp, struct _aalclp_gcs_compliance_data *gcs)
{
   help_msg_callback(fp, gcs);
}

int verifycmds(struct ALIConfigCommandLine *cl)
{
   std::ifstream bitfile(cl->bitstream_file,std::ios::binary);

   if(!bitfile.good()) {
      printf("Invalid File : %s\n", cl->bitstream_file);
      return 3;
   }
   return 0;
}

END_C_DECLS

/// @addtogroup ALIConfAFU
/// @{


/// @brief   Since this is a simple application, our App class implements both the IRuntimeClient and IServiceClient
///           interfaces.  Since some of the methods will be redundant for a single object, they will be ignored.
///
class ALIConfAFUApp: public CAASBase, public IRuntimeClient, public IServiceClient, IALIReconfigure_Client
{
public:

   ALIConfAFUApp();
   ~ALIConfAFUApp();

   btInt run();    ///< Return 0 if success


   // <begin IServiceClient interface>
   void serviceAllocated(IBase *pServiceBase,
                         TransactionID const &rTranID);

   void serviceAllocateFailed(const IEvent &rEvent);

   void serviceReleased(const AAL::TransactionID&);

   void serviceReleaseRequest(IBase *pServiceBase, const IEvent &);

   void serviceReleaseFailed(const AAL::IEvent&);

   void serviceEvent(const IEvent &rEvent);
   // <end IServiceClient interface>

   // <begin IRuntimeClient interface>
   void runtimeCreateOrGetProxyFailed(IEvent const &rEvent){};    // Not Used

   void runtimeStarted(IRuntime            *pRuntime,
                       const NamedValueSet &rConfigParms);

   void runtimeStopped(IRuntime *pRuntime);

   void runtimeStartFailed(const IEvent &rEvent);

   void runtimeStopFailed(const IEvent &rEvent);

   void runtimeAllocateServiceFailed( IEvent const &rEvent);

   void runtimeAllocateServiceSucceeded(IBase               *pClient,
                                        TransactionID const &rTranID);

   void runtimeEvent(const IEvent &rEvent);

   // <end IRuntimeClient interface>

   // <IALIReconfigure_Client interface>
   virtual void deactivateSucceeded( TransactionID const &rTranID );
   virtual void deactivateFailed( IEvent const &rEvent );
   virtual void configureSucceeded( TransactionID const &rTranID );
   virtual void configureFailed( IEvent const &rEvent );
   virtual void activateSucceeded( TransactionID const &rTranID );
   virtual void activateFailed( IEvent const &rEvent );
   // <end IALIReconfigure_Client interface>

protected:
   Runtime               m_Runtime;           ///< AAL Runtime
   IBase                *m_pAALService;       ///< The generic AAL Service interface for the AFU.
   IALIReconfigure      *m_pALIReconfService; ///< Pointer to Buffer Service
   CSemaphore            m_Sem;               ///< For synchronizing with the AAL runtime.
   btInt                 m_Result;            ///< Returned result v; 0 if success
   btBool                m_ReleaseStatus;

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
   m_pALIReconfService(NULL),
   m_Result(0),
   m_ReleaseStatus(false)
{
   // Register our Client side interfaces so that the Service can acquire them.
   //   SetInterface() is inherited from CAASBase
   SetInterface(iidServiceClient, dynamic_cast<IServiceClient *>(this));
   SetInterface(iidRuntimeClient, dynamic_cast<IRuntimeClient *>(this));
   SetInterface(iidALI_CONF_Service_Client, dynamic_cast<IALIReconfigure_Client *>(this));

   // Initialize our internal semaphore
   m_Sem.Create(0, 1);

   // Start the AAL Runtime, setting any startup options via a NamedValueSet

   // Using Hardware Services requires the Remote Resource Manager Broker Service
   //  Note that this could also be accomplished by setting the environment variable
   //   AALRUNTIME_CONFIG_BROKER_SERVICE to librrmbroker
   NamedValueSet configArgs;
   NamedValueSet configRecord;

#if defined( HWAFU )
   // Specify that the remote resource manager is to be used.
   configRecord.Add(AALRUNTIME_CONFIG_BROKER_SERVICE, "librrmbroker");
   configArgs.Add(AALRUNTIME_CONFIG_RECORD, &configRecord);
#endif

   // Start the Runtime and wait for the callback by sitting on the semaphore.
   //   the runtimeStarted() or runtimeStartFailed() callbacks should set m_bIsOK appropriately.
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

/// @brief   run() is called from main performs the following:
///             - Allocate the appropriate ALI Service depending
///               on whether a hardware, ASE or software implementation is desired.
///             - Allocates the necessary buffers to be used by the NLB AFU algorithm
///             - Executes the NLB algorithm
///             - Cleans up.
///
btInt ALIConfAFUApp::run()
{
   cout <<"============================="<<endl;
   cout <<"= ALI AFU Config NLB Sample ="<<endl;
   cout <<"============================="<<endl;

   // Request the Servcie we are interested in.

   // NOTE: This example is bypassing the Resource Manager's configuration record lookup
   //  mechanism.  Since the Resource Manager Implementation is a sample, it is subject to change.
   //  This example does illustrate the utility of having different implementations of a service all
   //  readily available and bound at run-time.
   NamedValueSet Manifest;
   NamedValueSet ConfigRecord;

#if defined( HWAFU )                /* Use FPGA hardware */
   // Service Library to use
   ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libHWALIAFU");

   // the AFUID to be passed to the Resource Manager. It will be used to locate the appropriate device.
   ConfigRecord.Add(keyRegAFU_ID,ALI_AFUID_UAFU_CONFIG);

   ConfigRecord.Add(keyRegSubDeviceNumber,0);

   // indicate that this service needs to allocate an AIAService, too to talk to the HW
   ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_AIA_NAME, "libaia");

   #elif defined ( ASEAFU )         /* Use ASE based RTL simulation */
   Manifest.Add(keyRegHandle, 20);

   ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libASEALIAFU");
   ConfigRecord.Add(AAL_FACTORY_CREATE_SOFTWARE_SERVICE,true);

   #else                            /* default is Software Simulator */
#if 0 // NOT CURRRENTLY SUPPORTED
   ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libSWSimALIAFU");
   ConfigRecord.Add(AAL_FACTORY_CREATE_SOFTWARE_SERVICE,true);
#endif
   return -1;
#endif

   // Add the Config Record to the Manifest describing what we want to allocate
   Manifest.Add(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &ConfigRecord);

   // in future, everything could be figured out by just giving the service name
   Manifest.Add(AAL_FACTORY_CREATE_SERVICENAME, "ALI Conf AFU");

   MSG("Allocating Service");

   // Allocate the Service and wait for it to complete by sitting on the
   //   semaphore. The serviceAllocated() callback will be called if successful.
   //   If allocation fails the serviceAllocateFailed() should set m_bIsOK appropriately.
   //   (Refer to the serviceAllocated() callback to see how the Service's interfaces
   //    are collected.)
   m_Runtime.allocService(dynamic_cast<IBase *>(this), Manifest);
   m_Sem.Wait();
   if(!m_bIsOK){
      ERR("Allocation failed\n");
      goto done_0;
   }




   //=============================
   // Now we have the NLB Service
   //   now we can use it
   //=============================
   MSG("Running Test");
   if(true == m_bIsOK){


      NamedValueSet nvsDeactv;

      nvsDeactv.Add(AALCONF_MILLI_TIMEOUT,static_cast<btUnsigned64bitInt>(1));


      nvsDeactv.Add(AALCONF_RECONF_ACTION,AALCONF_RECONF_ACTION_HONOR_REQUEST_ID);

      nvsDeactv.Add(AALCONF_REACTIVATE_DISABLED,false);



      // FIXME: could reuse existing empty NVS for less overhead
    //  m_pALIReconfService->reconfDeactivate(TransactionID(), nvsDeactv);
    //  m_Sem.Wait();

   //   NamedValueSet nvs;

      //nvs.Add(AALCONF_FILENAMEKEY,"/home/joe/sources/ccipTest_PR.cpp");

      //std::cout <<"BitStream File Name="<< configCmdLine.bitstream_file << std::endl;
      nvs.Add(AALCONF_FILENAMEKEY,configCmdLine.bitstream_file);

      m_pALIReconfService->reconfConfigure(TransactionID(), nvsDeactv);
      m_Sem.Wait();

      /*
      // FIXME: could reuse existing empty NVS for less overhead
      m_pALIReconfService->reconfActivate(TransactionID(), NamedValueSet());
      m_Sem.Wait();*/
   }
   MSG("Done Running Test");

   // Clean-up and return
   // Release() the Service through the Services IAALService::Release() method
   if(!m_ReleaseStatus)
   {
      MSG("Release Service");
   (dynamic_ptr<IAALService>(iidService, m_pAALService))->Release(TransactionID());
   m_Sem.Wait();
   }
   else
   {
      MSG("Release Service in Service Event");
      m_Sem.Wait();
   }

done_0:
   m_Runtime.stop();
   m_Sem.Wait();

   return m_Result;
}

//=================
//  IServiceClient
//=================

// <begin IServiceClient interface>
void ALIConfAFUApp::serviceAllocated(IBase *pServiceBase,
                                      TransactionID const &rTranID)
{
   // Save the IBase for the Service. Through it we can get any other
   //  interface implemented by the Service
   m_pAALService = pServiceBase;
   ASSERT(NULL != m_pAALService);
   if ( NULL == m_pAALService ) {
      m_bIsOK = false;
      return;
   }

   // Documentation says HWALIAFU Service publishes
   //    IALIBuffer as subclass interface. Used in Buffer Allocation and Free
   m_pALIReconfService = dynamic_ptr<IALIReconfigure>(iidALI_CONF_Service, pServiceBase);
   ASSERT(NULL != m_pALIReconfService);
   if ( NULL == m_pALIReconfService ) {
      m_bIsOK = false;
      return;
   }

   MSG("Service Allocated");
   m_Sem.Post(1);
}

void ALIConfAFUApp::serviceAllocateFailed(const IEvent &rEvent)
{
   ERR("Failed to allocate Service");
    PrintExceptionDescription(rEvent);
   ++m_Result;                     // Remember the error
   m_bIsOK = false;

   m_Sem.Post(1);
}

 void ALIConfAFUApp::serviceReleased(TransactionID const &rTranID)
{
    MSG("Service Released");
   // Unblock Main()
   m_Sem.Post(1);
}

 void ALIConfAFUApp::serviceReleaseRequest(IBase *pServiceBase, const IEvent &rEvent)
  {
     MSG("Service unexpected requested back");
     if(NULL != pServiceBase){
        IAALService *pIAALService = dynamic_ptr<IAALService>(iidService, pServiceBase);
        ASSERT(pIAALService);
        pIAALService->Release(TransactionID());
     }
  }

 void ALIConfAFUApp::serviceReleaseFailed(const IEvent        &rEvent)
 {
    ERR("Failed to release a Service");
    PrintExceptionDescription(rEvent);
    m_bIsOK = false;
    m_Sem.Post(1);
 }


 void ALIConfAFUApp::serviceEvent(const IEvent &rEvent)
{
   //ERR("unexpected event 0x" << hex << rEvent.SubClassID());
   std::cerr << "ALIConfAFUApp::serviceEvent \n" << std::endl;
   // The state machine may or may not stop here. It depends upon what happened.
   // A fatal error implies no more messages and so none of the other Post()
   //    will wake up.
   // OTOH, a notification message will simply print and continue.
   if ( rEvent.Has(iidExEvent) ) {
        //ExceptionEvent

      std::cerr << "\n Description  " << dynamic_ref<IExceptionEvent>(iidExEvent, rEvent).Description() << std::endl;
      std::cerr << "\n ExceptionNumber:  " << dynamic_ref<IExceptionEvent>(iidExEvent, rEvent).ExceptionNumber() << std::endl;
      std::cerr << "\n Reason:  " << dynamic_ref<IExceptionEvent>(iidExEvent, rEvent).Reason() << std::endl;

     }
   (dynamic_ptr<IAALService>(iidService, m_pAALService))->Release(TransactionID());
   m_ReleaseStatus = true;

}
// <end IServiceClient interface>


void ALIConfAFUApp::deactivateSucceeded( TransactionID const &rTranID )
{
   m_Sem.Post(1);
}
void ALIConfAFUApp::deactivateFailed( IEvent const &rEvent )
{
   ERR("Failed deactivate");
   PrintExceptionDescription(rEvent);
   m_bIsOK = false;
   m_Sem.Post(1);
}

void ALIConfAFUApp::configureSucceeded( TransactionID const &rTranID )
{
   m_Sem.Post(1);
}
void ALIConfAFUApp::configureFailed( IEvent const &rEvent )
{
   ERR("configureFailed");
   PrintExceptionDescription(rEvent);
   m_bIsOK = false;
   m_Sem.Post(1);
}
void ALIConfAFUApp::activateSucceeded( TransactionID const &rTranID )
{
   m_Sem.Post(1);
}
void ALIConfAFUApp::activateFailed( IEvent const &rEvent )
{
   ERR("activateFailed");
   PrintExceptionDescription(rEvent);
   m_bIsOK = false;
   m_Sem.Post(1);
}


 //=================
 //  IRuntimeClient
 //=================

  // <begin IRuntimeClient interface>
 // Because this simple example has one object implementing both IRuntieCLient and IServiceClient
 //   some of these interfaces are redundant. We use the IServiceClient in such cases and ignore
 //   the RuntimeClient equivalent e.g.,. runtimeAllocateServiceSucceeded()

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
 // <begin IRuntimeClient interface>

/// @} group HelloALINLB


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
   if ( argc < 2 ) {
      showhelp(stdout, &_aalclp_gcs_data);
      return 1;
   } else if ( 0!= ParseCmds(&configCmdLine, argc, argv) ) {
      cerr << "Error scanning command line." << endl;
      return 2;
   }else if ( flag_is_set(configCmdLine.flags, ALICONIFG_CMD_FLAG_HELP|ALICONIFG_CMD_FLAG_VERSION) ) {
      return 0;
   } else if ( verifycmds(&configCmdLine) ) {
      return 3;
   }

   ALIConfAFUApp theApp;
   if(!theApp.IsOK()){
      ERR("Runtime Failed to Start");
      exit(1);
   }
   btInt Result = theApp.run();

   MSG("Done");
   return Result;
}

