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
/// @ingroup ALIConfAFU
/// @verbatim
/// Accelerator Abstraction Layer Sample Application
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
#include <getopt.h>

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

# define CMD_PARSE_ERR        300
/// Command Line
BEGIN_C_DECLS

#define GETOPT_STRING ":hb:t:a:d:f:B:D:F:"

struct option longopts[] = {
      {"help",                no_argument,       NULL, 'h'},
      {"bitstream",           required_argument, NULL, 'b'},
      {"reconftimeout",       required_argument, NULL, 't'},
      {"reconfaction",        required_argument, NULL, 'a'},
      {"reactivateDisabled",  required_argument, NULL, 'd'},
      {"bus",                 required_argument, NULL, 'B'},
      {"device",              required_argument, NULL, 'D'},
      {"function",            required_argument, NULL, 'F'},
      {0, 0, 0, 0}
};

struct  ALIConfigCommandLine
{
   btUIntPtr          flags;
#define ALICONIFG_CMD_FLAG_HELP      0x00000001
#define ALICONIFG_CMD_FLAG_VERSION   0x00000002
#define ALICONIFG_CMD_PARSE_ERROR    0x00000003

#define ALICONIFG_CMD_FLAG_BUS       0x00000008
#define ALICONIFG_CMD_FLAG_DEV       0x00000010
#define ALICONIFG_CMD_FLAG_FUNC      0x00000020

   char    bitstream_file[200];
   int     reconftimeout;
   int     reconfAction;
   bool    reactivateDisabled;
   int     bus;
   int     device;
   int     function;

};
struct ALIConfigCommandLine configCmdLine = { 0,"",1,0,0,0,0,0 };

void AliConfigShowHelp()
{
   cout << "Usage:\n";
   cout << "   aliconfafu [<BITSTREAM>] [<RECONF-TIMEOUT>] [<RECONF-ACTION>]";
   cout << " [<REACTIVATE-DISABLED>] [<BUS>] [<DEVICE>] [<FUNCTION>] \n\n";
   cout << "<BITSTREAM>           --bitstream=<FILENAME>       OR  -b=<FILENAME>\n";
   cout << "<RECONF-TIMEOUT>      --reconftimeout=<SECONDS>    OR  -t=<SECONDS>\n";
   cout << "<RECONF-ACTION>       --reconfaction=A             OR  -a=A                ";
   cout << "where A = <ACTION_HONOR_REQUEST or ACTION_HONOR_OWNER >\n";
   cout << "<REACTIVATE-DISABLED> --reactivateDisabled=C       OR  -d=C                ";
   cout << "where C = <TRUE or FALSE>\n";
   cout << "<BUS>                 --bus=<BUS_NUMBER>           OR  -B=<BUS_NUMBER>\n";
   cout << "<DEVICE>              --device=<DEVICE_NUMBER>     OR  -D=<DEVICE_NUMBER>\n";
   cout << "<FUNCTION>            --function=<FUNCTION_NUMBER> OR  -F=<FUNCTION_NUMBER>\n";
   cout << "\n";

}

int ParseCmds(struct ALIConfigCommandLine *pconfigcmd, int argc, char *argv[])
{
   int getopt_ret;
   int option_index;
   char *endptr = NULL;

   while( -1 != ( getopt_ret = getopt_long(argc, argv, GETOPT_STRING, longopts, &option_index))){
      const char *tmp_optarg = optarg;

      if((optarg) &&
         ('=' == *tmp_optarg)){
         ++tmp_optarg;
      }

      if((!optarg) &&
         (NULL != argv[optind]) &&
         ('-' != argv[optind][0]) ) {
         tmp_optarg = argv[optind++];
      }

      switch(getopt_ret){

         case 'h':    /* help option */
            flag_setf(pconfigcmd->flags, ALICONIFG_CMD_FLAG_HELP);
            AliConfigShowHelp();
            break;

         case 'b':    /* bitstream option */
            ASSERT(NULL != tmp_optarg);
            if (NULL == tmp_optarg) break;
            strncpy(pconfigcmd->bitstream_file, tmp_optarg, sizeof(pconfigcmd->bitstream_file)-1);
            pconfigcmd->bitstream_file[sizeof(pconfigcmd->bitstream_file)-1] = 0;
            break;

         case 't':    /* reconftimeout option */
            ASSERT(NULL != tmp_optarg);
            if (NULL == tmp_optarg) break;
            endptr = NULL;
            pconfigcmd->reconftimeout = strtoul(tmp_optarg, &endptr, 0);
            break;

         case 'a':    /* reconfAction option */
            ASSERT(NULL != tmp_optarg);
            if (NULL == tmp_optarg) break;
            if ( 0 == strcasecmp("ACTION_HONOR_OWNER", tmp_optarg) ) {
               pconfigcmd->reconfAction =AALCONF_RECONF_ACTION_HONOR_OWNER_ID;
            }else{
               pconfigcmd->reconfAction =AALCONF_RECONF_ACTION_HONOR_REQUEST_ID;
            }
            break;

         case 'd':    /* reactivateDisabled option */
            ASSERT(NULL != tmp_optarg);
            if (NULL == tmp_optarg) break;
            if ( 0 == strcasecmp("TRUE", tmp_optarg) ) {
               pconfigcmd->reactivateDisabled =true;
            } else {
               pconfigcmd->reactivateDisabled =false;
            }
            break;

         case 'B':    /* bus option */
            ASSERT(NULL != tmp_optarg);
            if (NULL == tmp_optarg) break;
            endptr = NULL;
            pconfigcmd->bus = strtoul(tmp_optarg, &endptr, 0);
            flag_setf(pconfigcmd->flags, ALICONIFG_CMD_FLAG_BUS);
            break;

         case 'D':    /* device option */
            ASSERT(NULL != tmp_optarg);
            if (NULL == tmp_optarg) break;
            endptr = NULL;
            pconfigcmd->device = strtoul(tmp_optarg, &endptr, 0);
            flag_setf(pconfigcmd->flags, ALICONIFG_CMD_FLAG_DEV);
            break;

         case 'F':    /* function option */
            ASSERT(NULL != tmp_optarg);
            if (NULL == tmp_optarg) break;
            endptr = NULL;
            pconfigcmd->function = strtoul(tmp_optarg, &endptr, 0);
            flag_setf(pconfigcmd->flags, ALICONIFG_CMD_FLAG_FUNC);
            break;

         case ':':   /* missing option argument */
            cout << "Missing option argument.\n";
            return CMD_PARSE_ERR;

         case '?':
         default:    /* invalid option */
            cout << "Invalid cmdline options.\n";
            return CMD_PARSE_ERR;
      }
   }
   return 0;
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

   void PrintReconfExceptionDescription(IEvent const &theEvent);

protected:
   Runtime               m_Runtime;           ///< AAL Runtime
   IBase                *m_pAALService;       ///< The generic AAL Service interface for the AFU.
   IALIReconfigure      *m_pALIReconfService; ///< Pointer to Buffer Service
   IBase                *m_pFMEAALService;    ///< The generic AAL Service interface for the FME
   IALIMMIO             *m_pFMEMMIOService;   ///< Pointer to MMIO Service
   CSemaphore            m_Sem;               ///< For synchronizing with the AAL runtime.
   btInt                 m_Result;            ///< Returned result v; 0 if success
   TransactionID         m_tranIDFME;         ///< Transaction ID for FME service
   TransactionID         m_tranIDPR;          ///< Transaction ID for PR service
};

///////////////////////////////////////////////////////////////////////////////
///
///  Implementation
///
///////////////////////////////////////////////////////////////////////////////

/// @brief   Constructor registers this objects client interfaces and starts
///          the AAL Runtime. The member m_bisOK is used to indicate an error.
ALIConfAFUApp::ALIConfAFUApp() :
   m_Runtime(this),\
   m_pAALService(NULL),
   m_pALIReconfService(NULL),
   m_pFMEAALService(NULL),
   m_pFMEMMIOService(NULL),
   m_tranIDPR(),
   m_tranIDFME(),
   m_Result(0)
{
   // Register our Client side interfaces so that the Service can acquire them.
   // SetInterface() is inherited from CAASBase
   SetInterface(iidServiceClient, dynamic_cast<IServiceClient *>(this));
   SetInterface(iidRuntimeClient, dynamic_cast<IRuntimeClient *>(this));
   SetInterface(iidALI_CONF_Service_Client, dynamic_cast<IALIReconfigure_Client *>(this));

   // Initialize our internal semaphore
   m_Sem.Create(0, 1);

   // Start the AAL Runtime, setting any startup options via a NamedValueSet

   // Using Hardware Services requires the Remote Resource Manager Broker Service
   // Note that this could also be accomplished by setting the environment variable
   // AALRUNTIME_CONFIG_BROKER_SERVICE to librrmbroker
   NamedValueSet configArgs;
   NamedValueSet configRecord;

#if defined( HWAFU )
   // Specify that the remote resource manager is to be used.
   configRecord.Add(AALRUNTIME_CONFIG_BROKER_SERVICE, "librrmbroker");
   configArgs.Add(AALRUNTIME_CONFIG_RECORD, &configRecord);
#endif

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
ALIConfAFUApp::~ALIConfAFUApp()
{
   m_Sem.Destroy();
}

/// @brief   run() is called from main performs the following:
///             - Allocate an ALI FME service to retrieve BBS interface ID
///             - Check BBS metadata against BBS interface ID
///             - Allocate the appropriate ALI PR Service depending
///               on whether a hardware, ASE or software implementation is desired.
///             - Initiate PR operation
///             - Cleans up.
btInt ALIConfAFUApp::run()
{
   cout <<"============================="<<endl;
   cout <<"= ALI AFU Configure Sample ="<<endl;
   cout <<"============================="<<endl;

   // Request the Servcie we are interested in.

   // NOTE: This example is bypassing the Resource Manager's configuration record lookup
   //  mechanism.  Since the Resource Manager Implementation is a sample, it is subject to change.
   //  This example does illustrate the utility of having different implementations of a service all
   //  readily available and bound at run-time.
   NamedValueSet Manifest;
   NamedValueSet ConfigRecord;
   NamedValueSet reconfnvs;

   //
   // Check green bitstream file agains BBS interface ID
   //   The BBS interface ID is retrieved from FME registers
   //

   std::ifstream bitfile(configCmdLine.bitstream_file,std::ios::binary);
   btUnsigned32bitInt magic;
   btUnsigned64bitInt expected_ifid_l;
   btUnsigned64bitInt expected_ifid_h;
   btUnsigned64bitInt bitstream_id;
   btUnsigned64bitInt ifid_l;
   btUnsigned64bitInt ifid_h;

   // Read and check metadata header
   bitfile.read( (char *)&magic, sizeof( magic ) );
   if (magic != 0x1d1f8680) { // little endian, magic sequence is 0x80 0x86 0x1f 0x1d
      ERR(configCmdLine.bitstream_file << " does not appear to be a valid GBS file (header mismatch).");
      goto done_0;
   }

   // Read and store expected bitstream ID from GBS metadata
   bitfile.read( (char *)&expected_ifid_l, sizeof( expected_ifid_l ) );
   bitfile.read( (char *)&expected_ifid_h, sizeof( expected_ifid_h ) );

   // Read interface ID off blue bitstream (FME registers)
   //  this functionality will eventually be part of the driver
   //  FME_PR_INTFC_ID_L: FME offset 0x50A8
   //  FME_PR_INTFC_ID_H: FME offset 0x50B0

   // Prepare FME service allocation
   ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libALI");
   ConfigRecord.Add(keyRegAFU_ID,CCIP_FME_AFUID);

   // Select specific bus/device/function, if desired
   if (flag_is_set(configCmdLine.flags, ALICONIFG_CMD_FLAG_BUS)) {
      ConfigRecord.Add(keyRegBusNumber, btUnsigned32bitInt(configCmdLine.bus));
   }
   if (flag_is_set(configCmdLine.flags, ALICONIFG_CMD_FLAG_DEV)) {
      ConfigRecord.Add(keyRegDeviceNumber, btUnsigned32bitInt(configCmdLine.device));
   }
   if (flag_is_set(configCmdLine.flags, ALICONIFG_CMD_FLAG_FUNC)) {
      ConfigRecord.Add(keyRegFunctionNumber, btUnsigned32bitInt(configCmdLine.function));
   }
   Manifest.Add(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &ConfigRecord);
   Manifest.Add(AAL_FACTORY_CREATE_SERVICENAME, "FME");

   // Allocate FME service
   MSG("Allocating FME Service to check interface ID");
   m_Runtime.allocService(dynamic_cast<IBase *>(this), Manifest, m_tranIDFME);
   m_Sem.Wait();
   if(!m_bIsOK){
      ERR("Allocation failed\n");
      goto done_0;
   }

   // Read FME CSRs
   m_pFMEMMIOService->mmioRead64( 0x60, &bitstream_id );
   m_pFMEMMIOService->mmioRead64( 0x50A8, &ifid_l );
   m_pFMEMMIOService->mmioRead64( 0x50B0, &ifid_h );
   MSG( "BBS bitstream ID is 0x" << std::hex << bitstream_id );
   MSG( "BBS interface ID is 0x" << std::hex << ifid_h << ifid_l );

   // Release FME service
   MSG("Releasing FME Service");
   (dynamic_ptr<IAALService>(iidService, m_pFMEAALService))->Release(TransactionID());
   m_Sem.Wait();

   // Compare expected and actual interface ID
   if ( expected_ifid_l!= ifid_l || expected_ifid_h!= ifid_h ) {
      ERR("BBS interface ID does not match GBS metadata (0x" << std::hex <<
            expected_ifid_h << expected_ifid_l << ")");
      goto done_0;
   }
   MSG( "Interface ID matches" );

   // Clear ConfigRecord and Manifest for next service allocation
   ConfigRecord.Empty();
   Manifest.Empty();

   //
   // Allocate PR service
   //

#if defined( HWAFU )                /* Use FPGA hardware */
   // Service Library to use
   ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libALI");

   // the AFUID to be passed to the Resource Manager. It will be used to locate the appropriate device.
   ConfigRecord.Add(keyRegAFU_ID,ALI_AFUID_UAFU_CONFIG);

   ConfigRecord.Add(keyRegSubDeviceNumber,0);

   if (flag_is_set(configCmdLine.flags, ALICONIFG_CMD_FLAG_BUS)) {
      ConfigRecord.Add(keyRegBusNumber, btUnsigned32bitInt(configCmdLine.bus));
   }
   if (flag_is_set(configCmdLine.flags, ALICONIFG_CMD_FLAG_DEV)) {
      ConfigRecord.Add(keyRegDeviceNumber, btUnsigned32bitInt(configCmdLine.device));
   }
   if (flag_is_set(configCmdLine.flags, ALICONIFG_CMD_FLAG_FUNC)) {
      ConfigRecord.Add(keyRegFunctionNumber, btUnsigned32bitInt(configCmdLine.function));
   }

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
   m_Runtime.allocService(dynamic_cast<IBase *>(this), Manifest, m_tranIDPR);
   m_Sem.Wait();
   if(!m_bIsOK){
      ERR("Allocation failed\n");
      goto done_0;
   }


   //=============================
   // Now we have the ALIReconfigure Service
   //   now we can use it
   //=============================
   MSG("Running Test");
   if(true == m_bIsOK){

      // Reconfigure timeout
      reconfnvs.Add(AALCONF_MILLI_TIMEOUT,(static_cast<btUnsigned64bitInt>(configCmdLine.reconftimeout)) *1000);

      // Reconfigure action
      if(AALCONF_RECONF_ACTION_HONOR_OWNER_ID == configCmdLine.reconfAction) {
         reconfnvs.Add(AALCONF_RECONF_ACTION,AALCONF_RECONF_ACTION_HONOR_OWNER_ID);
      }
      else {
         reconfnvs.Add(AALCONF_RECONF_ACTION,AALCONF_RECONF_ACTION_HONOR_REQUEST_ID);
      }

      // ReActivated state
      if(configCmdLine.reactivateDisabled) {
         reconfnvs.Add(AALCONF_REACTIVATE_DISABLED,true);
      }
      else {
         reconfnvs.Add(AALCONF_REACTIVATE_DISABLED,false);
      }

      //reconfnvs.Add(AALCONF_FILENAMEKEY,"/home/lab/pr/bitstream.rbf");
      reconfnvs.Add(AALCONF_FILENAMEKEY,configCmdLine.bitstream_file);

      /*// Deactivate AFU Resource
      m_pALIReconfService->reconfDeactivate(TransactionID(), reconfnvs);
      m_Sem.Wait();
      if(!m_bIsOK){
         ERR("Deactivate failed\n");
         goto done_1;
      }*/

      // reconfigure with Bitstream
      m_pALIReconfService->reconfConfigure(TransactionID(), reconfnvs);
      m_Sem.Wait();
      if(!m_bIsOK){
         ERR("ReConfigure failed\n");
         goto done_1;
      }

      // reactivate AFU Resource
      if(configCmdLine.reactivateDisabled) {
         m_pALIReconfService->reconfActivate(TransactionID(), NamedValueSet());
         m_Sem.Wait();
         if(!m_bIsOK){
            ERR("Activate failed\n");
            goto done_1;
         }
      }


   }
   MSG("Done Running Test");



done_1:
   // Clean-up and return
   // Release() the Service through the Services IAALService::Release() method
   MSG("Release Service");
   (dynamic_ptr<IAALService>(iidService, m_pAALService))->Release(TransactionID());
   m_Sem.Wait();

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
   if (rTranID == m_tranIDPR) {
      // PR service allocated:
      //   Save generic service pointer and IALIReconfigure pointer to PR
      //   service

      m_pAALService = pServiceBase;
      ASSERT(NULL != m_pAALService);
      if ( NULL == m_pAALService ) {
         m_bIsOK = false;
         return;
      }

      m_pALIReconfService = dynamic_ptr<IALIReconfigure>(iidALI_CONF_Service, pServiceBase);
      ASSERT(NULL != m_pALIReconfService);
      if ( NULL == m_pALIReconfService ) {
         m_bIsOK = false;
         return;
      }

   } else if (rTranID == m_tranIDFME) {

      // FME service allocated:
      //   Save generic service pointer and IALIMMIO pointer to FME service

      m_pFMEAALService = pServiceBase;
      ASSERT(NULL != m_pFMEAALService);
      if ( NULL == m_pFMEAALService ) {
         m_bIsOK = false;
         return;
      }

      m_pFMEMMIOService = dynamic_ptr<IALIMMIO>(iidALI_MMIO_Service, pServiceBase);
      ASSERT(NULL != m_pFMEMMIOService);
      if ( NULL == m_pFMEMMIOService ) {
         m_bIsOK = false;
         return;
      }

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
   if ( rEvent.Has(iidReleaseRequestEvent) ) {

      std::cerr << "Description: " << dynamic_ref<IReleaseRequestEvent>(iidReleaseRequestEvent, rEvent).Description() << std::endl;
      std::cerr << "Timeout: " << dynamic_ref<IReleaseRequestEvent>(iidReleaseRequestEvent, rEvent).Timeout() << std::endl;
      std::cerr << "Reason: " << dynamic_ref<IReleaseRequestEvent>(iidReleaseRequestEvent, rEvent).Reason() << std::endl;

   }

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


}
// <end IServiceClient interface>


void ALIConfAFUApp::deactivateSucceeded( TransactionID const &rTranID )
{
   MSG("deactivateSucceeded");
   m_Sem.Post(1);
}
void ALIConfAFUApp::deactivateFailed( IEvent const &rEvent )
{
   ERR("Failed deactivate");
   PrintExceptionDescription(rEvent);
   PrintReconfExceptionDescription(rEvent);
   ++m_Result;                     // Remember the error
   m_bIsOK = false;
   m_Sem.Post(1);
}

void ALIConfAFUApp::configureSucceeded( TransactionID const &rTranID )
{
   MSG("configureSucceeded");
   m_Sem.Post(1);
}
void ALIConfAFUApp::configureFailed( IEvent const &rEvent )
{
   ERR("configureFailed");
   PrintExceptionDescription(rEvent);
   PrintReconfExceptionDescription(rEvent);
   ++m_Result;                     // Remember the error
   m_bIsOK = false;
   m_Sem.Post(1);
}
void ALIConfAFUApp::activateSucceeded( TransactionID const &rTranID )
{
   MSG("activateSucceeded");
   m_Sem.Post(1);
}
void ALIConfAFUApp::activateFailed( IEvent const &rEvent )
{
   ERR("activateFailed");
   PrintExceptionDescription(rEvent);
   PrintReconfExceptionDescription(rEvent);
   ++m_Result;                     // Remember the error
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

 void ALIConfAFUApp::PrintReconfExceptionDescription(IEvent const &rEvent)
 {
    if ( rEvent.Has(iidExTranEvent) ) {
      std::cerr << "Description: " << dynamic_ref<IExceptionTransactionEvent>(iidExTranEvent, rEvent).Description() << std::endl;
      std::cerr << "ExceptionNumber: " << dynamic_ref<IExceptionTransactionEvent>(iidExTranEvent, rEvent).ExceptionNumber() << std::endl;
      std::cerr << "Reason: " << dynamic_ref<IExceptionTransactionEvent>(iidExTranEvent, rEvent).Reason() << std::endl;
   }
 }
 // <begin IRuntimeClient interface>

/// @} group ALIConfAFUApp


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
      AliConfigShowHelp();
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

