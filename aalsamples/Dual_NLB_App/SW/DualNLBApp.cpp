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
/// @file HelloALINLB.cpp
/// @brief Basic ALI AFU interaction.
/// @ingroup HelloALINLB
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

#ifndef CL
# define CL(x)                     ((x) * 64)
#endif // CL
#ifndef LOG2_CL
# define LOG2_CL                   6
#endif // LOG2_CL
#ifndef MB
# define MB(x)                     ((x) * 1024 * 1024)
#endif // MB

// LPBK1_BUFFER_SIZE is size in cachelines that are copied
#define LPBK1_BUFFER_SIZE        CL(1)
// LPBK1_BUFFER_ALLOCATION_SIZE is the amount of space that needs to
//   be allocated due to an optimization of the NLB AFU to operate on
//   2 MiB buffer boundaries. Note that the way to get 2 MiB alignment
//   is to allocate 2 MiB.
// NOTE:
//   2 MiB alignment is not a general requirement -- it is NLB-specific
#define LPBK1_BUFFER_ALLOCATION_SIZE MB(2)

#define LPBK1_DSM_SIZE           MB(4)
#define CSR_SRC_ADDR             0x0120
#define CSR_DST_ADDR             0x0128
#define CSR_CTL                  0x0138
#define CSR_CFG                  0x0140
#define CSR_NUM_LINES            0x0130
#define DSM_STATUS_TEST_COMPLETE 0x40
#define CSR_AFU_DSM_BASEL        0x0110
#define CSR_AFU_DSM_BASEH        0x0114
#define NLB_TEST_MODE_PCIE0		 0x2000

BEGIN_C_DECLS

int dualnlb_on_nix_long_option(AALCLP_USER_DEFINED , const char * , const char * );

void help_msg_callback(FILE * , struct _aalclp_gcs_compliance_data * );
void showhelp(FILE * , struct _aalclp_gcs_compliance_data * );

struct  DualNLBConfigCommandLine
{
   btUIntPtr          flags;
#define DUALNLB_CMD_FLAG_HELP      0x00000001
#define DUALNLB_CMD_FLAG_VERSION   0x00000002
#define DUALNLB_CMD_PARSE_ERROR    0x00000004

#define DUALNLB_CMD_FLAG_BUS0      0x00000008
#define DUALNLB_CMD_FLAG_BUS1      0x00000010
#define DUALNLB_CMD_FLAG_DEV0      0x00000020
#define DUALNLB_CMD_FLAG_DEV1      0x00000040
#define DUALNLB_CMD_FLAG_FUNC0     0x00000080
#define DUALNLB_CMD_FLAG_FUNC1     0x00000100

   int     bus0;
   int     bus1;
   int     device0;
   int     device1;
   int     function0;
   int     function1;
};
struct DualNLBConfigCommandLine configCmdLine = { 0,0,0,0,0,0,0 };

int dualnlb_on_non_option(AALCLP_USER_DEFINED user, const char *nonoption) {
   struct DualNLBConfigCommandLine *cl = (struct DualNLBConfigCommandLine *)user;
   flag_setf(cl->flags, DUALNLB_CMD_PARSE_ERROR);
   printf("Invalid: %s\n", nonoption);
   return 0;
}

int dualnlb_on_dash_only(AALCLP_USER_DEFINED user) {
   struct DualNLBConfigCommandLine *cl = (struct DualNLBConfigCommandLine *)user;
   flag_setf(cl->flags, DUALNLB_CMD_PARSE_ERROR);
   printf("Invalid option: -\n");
   return 0;
}

int dualnlb_on_dash_dash_only(AALCLP_USER_DEFINED user) {
   struct DualNLBConfigCommandLine *cl = (struct DualNLBConfigCommandLine *)user;
   flag_setf(cl->flags, DUALNLB_CMD_PARSE_ERROR);
   printf("Invalid option: --\n");
   return 0;
}

int dualnlb_on_nix_long_option(AALCLP_USER_DEFINED user, const char *option, const char *value)
{
   struct DualNLBConfigCommandLine *pcmdline = (struct DualNLBConfigCommandLine *)user;

   char *endptr = NULL;
   //Bus number for socket 1
   if ( (0 == strcmp("--bus0", option)) || (0 == strcmp("--b0", option)) ) {

	   pcmdline->bus0 = strtoul(value, &endptr, 0);
	   flag_setf(pcmdline->flags, DUALNLB_CMD_FLAG_BUS0);
	   endptr = NULL;

   }else if ( (0 == strcmp("--bus1", option)) || (0 == strcmp("--b1", option)) ) {

	   pcmdline->bus1 = strtoul(value, &endptr, 0);
	   flag_setf(pcmdline->flags, DUALNLB_CMD_FLAG_BUS1);
	   endptr = NULL;

   }else if ( (0 == strcmp("--device0", option)) || (0 == strcmp("--d0", option)) ) {

   	   pcmdline->device0 = strtoul(value, &endptr, 0);
   	   flag_setf(pcmdline->flags, DUALNLB_CMD_FLAG_DEV0);
   	   endptr = NULL;

   }else if ( (0 == strcmp("--device1", option)) || (0 == strcmp("--d1", option)) ) {

	   pcmdline->device1 = strtoul(value, &endptr, 0);
	   flag_setf(pcmdline->flags, DUALNLB_CMD_FLAG_DEV1);
	   endptr = NULL;

   }else if ( (0 == strcmp("--function0", option)) || (0 == strcmp("--f0", option)) ) {

	   pcmdline->function0 = strtoul(value, &endptr, 0);
	   flag_setf(pcmdline->flags, DUALNLB_CMD_FLAG_FUNC0);
	   endptr = NULL;

   }else if ( (0 == strcmp("--function1", option)) || (0 == strcmp("--f1", option)) ) {

	   pcmdline->function1 = strtoul(value, &endptr, 0);
	   flag_setf(pcmdline->flags, DUALNLB_CMD_FLAG_FUNC1);
	   endptr = NULL;

   }else{

	   printf("Invalid option: %s\n", option);
	   flag_setf(pcmdline->flags,DUALNLB_CMD_PARSE_ERROR);
   }

   return 0;
}

aalclp_option       dualnlb_nix_long_option       = { dualnlb_on_nix_long_option, };
aalclp_non_option   dualnlb_non_option            = { dualnlb_on_non_option,      };
aalclp_dash_only    dualnlb_dash_only             = { dualnlb_on_dash_only,       };
aalclp_dash_only    dualnlb_dash_dash_only        = { dualnlb_on_dash_dash_only,  };

AALCLP_DECLARE_GCS_COMPLIANT(stdout,
                             "dualNLB",
                             "0",
                             "",
                             help_msg_callback,
                             &configCmdLine)

int ParseCmds(struct DualNLBConfigCommandLine *pconfigcmd, int argc, char *argv[])
{
   int    res;
   int    clean;
   aalclp clp;

   res = aalclp_init(&clp);
   if ( 0 != res ) {
      cerr << "aalclp_init() failed : " << res << ' ' << strerror(res) << endl;
      return res;
   }

   dualnlb_nix_long_option.user = pconfigcmd;
   aalclp_add_nix_long_option(&clp, &dualnlb_nix_long_option);

   dualnlb_non_option.user = pconfigcmd;
   aalclp_add_non_option(&clp, &dualnlb_non_option);

   dualnlb_dash_only.user = pconfigcmd;
   aalclp_add_dash_only(&clp, &dualnlb_dash_only);

   dualnlb_dash_dash_only.user = pconfigcmd;
   aalclp_add_dash_dash_only(&clp, &dualnlb_dash_dash_only);

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
   fprintf(fp, "   dualNLB [<BUS_NUMBER>] [<DEVICE_NUMBER>] [<FUNCTION_NUMBER>] \n\n");
   fprintf(fp, "<BUS_NUMBER>       --bus0=b       OR       --b0=b       where b is the bus number of the PCIe0\n");
   fprintf(fp, "                   --bus1=b       OR       --b1=b       where b is the bus number of the PCIe1\n");
   fprintf(fp, "<DEVICE_NUMBER>    --dev0=b       OR       --d0=d       where d is the device number of the PCIe0\n");
   fprintf(fp, "                   --dev1=b       OR       --d1=d       where d is the device number of the PCIe1\n");
   fprintf(fp, "<FUNCTION_NUMBER>  --func0=f      OR       --f0=f       where f is the function number of the PCIe0\n");
   fprintf(fp, "                   --func1=f      OR       --f1=f       where f is the function number of the PCIe1\n");
   fprintf(fp, "\n");
}

void showhelp(FILE *fp, struct _aalclp_gcs_compliance_data *gcs)
{
   help_msg_callback(fp, gcs);
}

int verifycmds(struct DualNLBConfigCommandLine *cl)
{
   return 0;
}

END_C_DECLS

/// @addtogroup DualNLBApp
/// @{
///////////////////////////////////////////////////////////////////////////////
///
/// @brief   Since this is a simple application, our App class implements both the IRuntimeClient and IServiceClient
///           interfaces.  Since some of the methods will be redundant for a single object, they will be ignored.
///
///////////////////////////////////////////////////////////////////////////////
class DualNLBApp: public CAASBase, public IRuntimeClient, public IServiceClient
{
public:

   DualNLBApp();
   ~DualNLBApp();

   btInt run();    ///< Return 0 if success

   // <begin IServiceClient interface>
   void serviceAllocated(IBase *pServiceBase,
                         TransactionID const &rTranID);

   void serviceAllocateFailed(const IEvent &rEvent);

   void serviceReleased(const AAL::TransactionID&);
   void serviceReleaseRequest(IBase *pServiceBase, const IEvent &rEvent);
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

   btBool isOK()  {return m_bIsOK;}

   // <end IRuntimeClient interface>
protected:
   Runtime        m_Runtime;           ///< AAL Runtime
   IBase         *m_pAALService;       ///< The generic AAL Service interface for the AFU.
   IALIBuffer    *m_pALIBufferService; ///< Pointer to Buffer Service
   IALIMMIO      *m_pALIMMIOService;   ///< Pointer to MMIO Service
   IALIReset     *m_pALIResetService;  ///< Pointer to AFU Reset Service
   CSemaphore     m_Sem;               ///< For synchronizing with the AAL runtime.
   btInt          m_Result;            ///< Returned result value; 0 if success

   // Workspace info
   btVirtAddr     m_DSMVirt;        ///< DSM workspace virtual address.
   btPhysAddr     m_DSMPhys;        ///< DSM workspace physical address.
   btWSSize       m_DSMSize;        ///< DSM workspace size in bytes.
   btVirtAddr     m_InputVirt;      ///< Input workspace virtual address.
   btPhysAddr     m_InputPhys;      ///< Input workspace physical address.
   btWSSize       m_InputSize;      ///< Input workspace size in bytes.
   btVirtAddr     m_OutputVirt;     ///< Output workspace virtual address.
   btPhysAddr     m_OutputPhys;     ///< Output workspace physical address.
   btWSSize       m_OutputSize;     ///< Output workspace size in bytes.
};

///////////////////////////////////////////////////////////////////////////////
///
/// @brief   Implementation
///          Constructor registers this objects client interfaces and starts
///          the AAL Runtime. The member m_bisOK is used to indicate an error.
///
///////////////////////////////////////////////////////////////////////////////

DualNLBApp::DualNLBApp() :
   m_Runtime(this),
   m_pAALService(NULL),
   m_pALIBufferService(NULL),
   m_pALIMMIOService(NULL),
   m_pALIResetService(NULL),
   m_Result(0),
   m_DSMVirt(NULL),
   m_DSMPhys(0),
   m_DSMSize(0),
   m_InputVirt(NULL),
   m_InputPhys(0),
   m_InputSize(0),
   m_OutputVirt(NULL),
   m_OutputPhys(0),
   m_OutputSize(0)
{
   // Register our Client side interfaces so that the Service can acquire them.
   //   SetInterface() is inherited from CAASBase
   SetInterface(iidServiceClient, dynamic_cast<IServiceClient *>(this));
   SetInterface(iidRuntimeClient, dynamic_cast<IRuntimeClient *>(this));

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
DualNLBApp::~DualNLBApp()
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
btInt DualNLBApp::run()
{
   cout <<"========================"<<endl;
   cout <<"= Hello ALI NLB Sample ="<<endl;
   cout <<"========================"<<endl;

   // Request the Servcie we are interested in.

   // NOTE: This example is bypassing the Resource Manager's configuration record lookup
   //  mechanism.  Since the Resource Manager Implementation is a sample, it is subject to change.
   //  This example does illustrate the utility of having different implementations of a service all
   //  readily available and bound at run-time.
   NamedValueSet Manifest;
   NamedValueSet ConfigRecord;

#if defined( HWAFU )                /* Use FPGA hardware */
   // Service Library to use
   ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libALI");

   // the AFUID to be passed to the Resource Manager. It will be used to locate the appropriate device.
   ConfigRecord.Add(keyRegAFU_ID,"D8424DC4-A4A3-C413-F89E-433683F9040B");


   #elif defined ( ASEAFU )         /* Use ASE based RTL simulation */
   Manifest.Add(keyRegHandle, 20);

   Manifest.Add(ALIAFU_NVS_KEY_TARGET, ali_afu_ase);

   ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libALI");
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
   Manifest.Add(AAL_FACTORY_CREATE_SERVICENAME, "Hello ALI NLB");

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

   // Now that we have the Service and have saved the IALIBuffer interface pointer
   //  we can now Allocate the 3 Workspaces used by the NLB algorithm. The buffer allocate
   //  function is synchronous so no need to wait on the semaphore

   // Device Status Memory (DSM) is a structure defined by the NLB implementation.

   // User Virtual address of the pointer is returned directly in the function
   if( ali_errnumOK != m_pALIBufferService->bufferAllocate(LPBK1_DSM_SIZE, &m_DSMVirt)){
      m_bIsOK = false;
      m_Result = -1;
      goto done_1;
   }

   // Save the size and get the IOVA from teh User Virtual address. The HW only uses IOVA.
   m_DSMSize = LPBK1_DSM_SIZE;
   m_DSMPhys = m_pALIBufferService->bufferGetIOVA(m_DSMVirt);

   if(0 == m_DSMPhys){
      m_bIsOK = false;
      m_Result = -1;
      goto done_2;
   }

   // Repeat for the Input and Output Buffers
   if( ali_errnumOK != m_pALIBufferService->bufferAllocate(LPBK1_BUFFER_ALLOCATION_SIZE, &m_InputVirt)){
      m_bIsOK = false;
      m_Sem.Post(1);
      m_Result = -1;
      goto done_2;
   }

   m_InputSize = LPBK1_BUFFER_SIZE;
   m_InputPhys = m_pALIBufferService->bufferGetIOVA(m_InputVirt);
   if(0 == m_InputPhys){
      m_bIsOK = false;
      m_Result = -1;
      goto done_3;
   }

   if( ali_errnumOK !=  m_pALIBufferService->bufferAllocate(LPBK1_BUFFER_ALLOCATION_SIZE, &m_OutputVirt)){
      m_bIsOK = false;
      m_Sem.Post(1);
      m_Result = -1;
      goto done_3;
   }

   m_OutputSize = LPBK1_BUFFER_SIZE;
   m_OutputPhys = m_pALIBufferService->bufferGetIOVA(m_OutputVirt);
   if(0 == m_OutputPhys){
      m_bIsOK = false;
      m_Result = -1;
      goto done_4;
   }



   //=============================
   // Now we have the NLB Service
   //   now we can use it
   //=============================
   MSG("Running Test");
   if(true == m_bIsOK){

      // Clear the DSM
      ::memset( m_DSMVirt, 0, m_DSMSize);

      // Initialize the source and destination buffers
      ::memset( m_InputVirt,  0xAF, m_InputSize);  // Input initialized to AFter
      ::memset( m_OutputVirt, 0xBE, m_OutputSize); // Output initialized to BEfore

      struct CacheLine {                           // Operate on cache lines
         btUnsigned32bitInt uint[16];
      };
      struct CacheLine *pCL = reinterpret_cast<struct CacheLine *>(m_InputVirt);
      for ( btUnsigned32bitInt i = 0; i < m_InputSize / CL(1) ; ++i ) {
         pCL[i].uint[15] = i;
      };                         // Cache-Line[n] is zero except last uint = n


      // Original code puts DSM Reset prior to AFU Reset, but ccipTest
      //    reverses that. We are following ccipTest here.

      // Initiate AFU Reset
      m_pALIResetService->afuReset();


      // Initiate DSM Reset
      // Set DSM base, high then low
      m_pALIMMIOService->mmioWrite64(CSR_AFU_DSM_BASEL, m_DSMPhys);

      // Assert AFU reset
      m_pALIMMIOService->mmioWrite32(CSR_CTL, 0);

      //De-Assert AFU reset
      m_pALIMMIOService->mmioWrite32(CSR_CTL, 1);

      // If ASE, give it some time to catch up
      /*
      #if defined ( ASEAFU )
      SleepSec(5);
      #endif*/ /* ASE AFU */


      // Set input workspace address
      m_pALIMMIOService->mmioWrite64(CSR_SRC_ADDR, CACHELINE_ALIGNED_ADDR(m_InputPhys));

      // Set output workspace address
      m_pALIMMIOService->mmioWrite64(CSR_DST_ADDR, CACHELINE_ALIGNED_ADDR(m_OutputPhys));

      // Set the number of cache lines for the test
      m_pALIMMIOService->mmioWrite32(CSR_NUM_LINES, LPBK1_BUFFER_SIZE / CL(1));

      // Set the test mode
      m_pALIMMIOService->mmioWrite32(CSR_CFG,0x42000);

      volatile bt32bitCSR *StatusAddr = (volatile bt32bitCSR *)
                                         (m_DSMVirt  + DSM_STATUS_TEST_COMPLETE);
      // Start the test
      m_pALIMMIOService->mmioWrite32(CSR_CTL, 3);


      // Wait for test completion
      while( 0 == ((*StatusAddr)&0x1) ) {
         SleepMicro(100);
      }
      MSG("Done Running Test");

      // Stop the device
      m_pALIMMIOService->mmioWrite32(CSR_CTL, 7);

      // Check that output buffer now contains what was in input buffer, e.g. 0xAF
      if (int err = memcmp( m_OutputVirt, m_InputVirt, m_OutputSize)) {
         ERR("Output does NOT Match input, at offset " << err << "!");
         ++m_Result;
      } else {
         MSG("Output matches Input!");
      }
   }
   MSG("Done Running Test");

   // Clean-up and return
done_4:
   m_pALIBufferService->bufferFree(m_OutputVirt);
done_3:
   m_pALIBufferService->bufferFree(m_InputVirt);
done_2:
   m_pALIBufferService->bufferFree(m_DSMVirt);

done_1:
   // Freed all three so now Release() the Service through the Services IAALService::Release() method
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
void DualNLBApp::serviceAllocated(IBase *pServiceBase,
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
   m_pALIBufferService = dynamic_ptr<IALIBuffer>(iidALI_BUFF_Service, pServiceBase);
   ASSERT(NULL != m_pALIBufferService);
   if ( NULL == m_pALIBufferService ) {
      m_bIsOK = false;
      return;
   }

   // Documentation says HWALIAFU Service publishes
   //    IALIMMIO as subclass interface. Used to set/get MMIO Region
   m_pALIMMIOService = dynamic_ptr<IALIMMIO>(iidALI_MMIO_Service, pServiceBase);
   ASSERT(NULL != m_pALIMMIOService);
   if ( NULL == m_pALIMMIOService ) {
      m_bIsOK = false;
      return;
   }

   // Documentation says HWALIAFU Service publishes
   //    IALIReset as subclass interface. Used for resetting the AFU
   m_pALIResetService = dynamic_ptr<IALIReset>(iidALI_RSET_Service, pServiceBase);
   ASSERT(NULL != m_pALIResetService);
   if ( NULL == m_pALIResetService ) {
      m_bIsOK = false;
      return;
   }

   MSG("Service Allocated");
   m_Sem.Post(1);
}

void DualNLBApp::serviceAllocateFailed(const IEvent &rEvent)
{
   ERR("Failed to allocate Service");
    PrintExceptionDescription(rEvent);
   ++m_Result;                     // Remember the error
   m_bIsOK = false;

   m_Sem.Post(1);
}

 void DualNLBApp::serviceReleased(TransactionID const &rTranID)
{
    MSG("Service Released");
   // Unblock Main()
   m_Sem.Post(1);
}

 void DualNLBApp::serviceReleaseRequest(IBase *pServiceBase, const IEvent &rEvent)
 {
    MSG("Service unexpected requested back");
    if(NULL != m_pAALService){
       IAALService *pIAALService = dynamic_ptr<IAALService>(iidService, m_pAALService);
       ASSERT(pIAALService);
       pIAALService->Release(TransactionID());
    }
 }


 void DualNLBApp::serviceReleaseFailed(const IEvent        &rEvent)
 {
    ERR("Failed to release a Service");
    PrintExceptionDescription(rEvent);
    m_bIsOK = false;
    m_Sem.Post(1);
 }


 void DualNLBApp::serviceEvent(const IEvent &rEvent)
{
   ERR("unexpected event 0x" << hex << rEvent.SubClassID());
   // The state machine may or may not stop here. It depends upon what happened.
   // A fatal error implies no more messages and so none of the other Post()
   //    will wake up.
   // OTOH, a notification message will simply print and continue.
}
// <end IServiceClient interface>


 //=================
 //  IRuntimeClient
 //=================

  // <begin IRuntimeClient interface>
 // Because this simple example has one object implementing both IRuntieCLient and IServiceClient
 //   some of these interfaces are redundant. We use the IServiceClient in such cases and ignore
 //   the RuntimeClient equivalent e.g.,. runtimeAllocateServiceSucceeded()

 void DualNLBApp::runtimeStarted( IRuntime            *pRuntime,
                                      const NamedValueSet &rConfigParms)
 {
    m_bIsOK = true;
    m_Sem.Post(1);
 }

 void DualNLBApp::runtimeStopped(IRuntime *pRuntime)
  {
     MSG("Runtime stopped");
     m_bIsOK = false;
     m_Sem.Post(1);
  }

 void DualNLBApp::runtimeStartFailed(const IEvent &rEvent)
 {
    ERR("Runtime start failed");
    PrintExceptionDescription(rEvent);
 }

 void DualNLBApp::runtimeStopFailed(const IEvent &rEvent)
 {
     MSG("Runtime stop failed");
     m_bIsOK = false;
     m_Sem.Post(1);
 }

 void DualNLBApp::runtimeAllocateServiceFailed( IEvent const &rEvent)
 {
    ERR("Runtime AllocateService failed");
    PrintExceptionDescription(rEvent);
 }

 void DualNLBApp::runtimeAllocateServiceSucceeded(IBase *pClient,
                                                     TransactionID const &rTranID)
 {
     MSG("Runtime Allocate Service Succeeded");
 }

 void DualNLBApp::runtimeEvent(const IEvent &rEvent)
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
	  cout << "Error scanning command line." << endl;
	  return 2;
	}else if ( flag_is_set(configCmdLine.flags, DUALNLB_CMD_FLAG_HELP|DUALNLB_CMD_FLAG_VERSION) ) {
		cout << "help!\n";
	  return 0;
	} else if ( verifycmds(&configCmdLine) ) {
	  cout << "verify cmd line\n";
	  return 3;
	}

   DualNLBApp theApp;
   if(!theApp.isOK()){
      ERR("Runtime Failed to Start");
      exit(1);
   }
   btInt Result = theApp.run();

   MSG("Done");
   return Result;
}

