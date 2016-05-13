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
/// @file PCIeErrMon.cpp
/// @brief Basic ALI AFU interaction.
/// @ingroup PCIeErrMon
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

#include <xmmintrin.h>
#include <emmintrin.h>
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
#define LPBK1_BUFFER_SIZE        CL(1)

#define LPBK1_DSM_SIZE           MB(4)
#define CSR_SRC_ADDR             0x0120
#define CSR_DST_ADDR             0x0128
#define CSR_CTL                  0x0138
#define CSR_CFG                  0x0140
#define CSR_NUM_LINES            0x0130
#define DSM_STATUS_TEST_COMPLETE 0x40
#define CSR_AFU_DSM_BASEL        0x0110
#define CSR_AFU_DSM_BASEH        0x0114
#	define NLB_TEST_MODE_PCIE0		0x2000

/// Command Line
BEGIN_C_DECLS

int pciErrMon_on_nix_long_option_only(AALCLP_USER_DEFINED , const char * );
int pciErrMon_on_nix_long_option(AALCLP_USER_DEFINED , const char * , const char * );

void help_msg_callback(FILE * , struct _aalclp_gcs_compliance_data * );
void showhelp(FILE * , struct _aalclp_gcs_compliance_data * );

struct  PCIeErrMonCmdLine
{
   btUnsigned64bitInt          flags;
   #define PCIERRMON_CMD_FLAG_HELP      (btUnsigned64bitInt)0x00000001
   #define PCIERRMON_CMD_FLAG_VERSION   (btUnsigned64bitInt)0x00000002
   #define PCIERRMON_CMD_PARSE_ERROR    (btUnsigned64bitInt)0x00000004
   #define PCIERRMON_CMD_WRITE_ADDR     (btUnsigned64bitInt)0x00000008
   #define PCIERRMON_CMD_READ_ADDR      (btUnsigned64bitInt)0x00000010
   #define PCIERRMON_CMD_WRITE_LENGTH   (btUnsigned64bitInt)0x00000040
   #define PCIERRMON_CMD_READ_LENGTH    (btUnsigned64bitInt)0x00000080
   #define PCIERRMON_CMD_CONFIG_READ    (btUnsigned64bitInt)0x00000100


};
struct PCIeErrMonCmdLine errMonCmdLine = { PCIERRMON_CMD_FLAG_HELP };


int pciErrMon_on_non_option(AALCLP_USER_DEFINED user, const char *nonoption) {
   struct PCIeErrMonCmdLine *cl = (struct PCIeErrMonCmdLine *)user;
   flag_setf(cl->flags, PCIERRMON_CMD_PARSE_ERROR);
   printf("Invalid: %s\n", nonoption);
   return 0;
}

int pciErrMon_on_dash_only(AALCLP_USER_DEFINED user) {
   struct PCIeErrMonCmdLine *cl = (struct PCIeErrMonCmdLine *)user;
   flag_setf(cl->flags, PCIERRMON_CMD_PARSE_ERROR);
   printf("Invalid option: -\n");
   return 0;
}

int pciErrMon_on_dash_dash_only(AALCLP_USER_DEFINED user) {
   struct PCIeErrMonCmdLine *cl = (struct PCIeErrMonCmdLine *)user;
   flag_setf(cl->flags, PCIERRMON_CMD_PARSE_ERROR);
   printf("Invalid option: --\n");
   return 0;
}

int pciErrMon_on_nix_long_option_only(AALCLP_USER_DEFINED user, const char *option)
{
   struct PCIeErrMonCmdLine *cl = (struct PCIeErrMonCmdLine *)user;
   if ( 0 == strcmp("--help", option) ) {
      flag_setf(cl->flags, PCIERRMON_CMD_FLAG_HELP);

   } else if ( 0 == strcmp("--version", option) ) {
      flag_setf(cl->flags, PCIERRMON_CMD_FLAG_VERSION);

   }else  if((0 == strcmp("--write-addr", option)) ||
             (0 == strcmp("--wa", option)))  {
      flag_setf(cl->flags, PCIERRMON_CMD_WRITE_ADDR);

   }else  if((0 == strcmp("--read-addr", option)) ||
             (0 == strcmp("--ra", option)))  {
      flag_setf(cl->flags, PCIERRMON_CMD_READ_ADDR);

   }else  if((0 == strcmp("--write-length", option)) ||
             (0 == strcmp("--wl", option)))  {
      flag_setf(cl->flags, PCIERRMON_CMD_WRITE_LENGTH);

   }else  if((0 == strcmp("--read-length", option)) ||
             (0 == strcmp("--rl", option)))  {
      flag_setf(cl->flags, PCIERRMON_CMD_READ_LENGTH);

   }else  if((0 == strcmp("--config-read", option)) ||
             (0 == strcmp("--cr", option)))  {
      flag_setf(cl->flags, PCIERRMON_CMD_CONFIG_READ);

   }else{
      flag_setf(cl->flags, PCIERRMON_CMD_PARSE_ERROR);
      printf("Invalid option\n");
      return 0;
   }
   return 0;
}


int pciErrMon_on_nix_long_option(AALCLP_USER_DEFINED user, const char *option, const char *value)
{
   return 0;
}

aalclp_option_only  pciErrMon_nix_long_option_only  = { pciErrMon_on_nix_long_option_only,  };
aalclp_option       pciErrMon_nix_long_option       = { pciErrMon_on_nix_long_option,       };
aalclp_non_option   pciErrMon_non_option            = { pciErrMon_on_non_option,            };
aalclp_dash_only    pciErrMon_dash_only             = { pciErrMon_on_dash_only,             };
aalclp_dash_only    pciErrMon_dash_dash_only        = { pciErrMon_on_dash_dash_only,        };

AALCLP_DECLARE_GCS_COMPLIANT(stdout,
                             "pciErrMon",
                             "0",
                             "",
                             help_msg_callback,
                             &errMonCmdLine)

int ParseCmds(struct PCIeErrMonCmdLine *pErrMonCmd, int argc, char *argv[])
{
   int    res;
   int    clean;
   aalclp clp;

   res = aalclp_init(&clp);
   if ( 0 != res ) {
      cerr << "aalclp_init() failed : " << res << ' ' << strerror(res) << endl;
      return res;
   }

   pciErrMon_nix_long_option_only.user = pErrMonCmd;
   aalclp_add_nix_long_option_only(&clp, &pciErrMon_nix_long_option_only);

   pciErrMon_nix_long_option.user = pErrMonCmd;
   aalclp_add_nix_long_option(&clp, &pciErrMon_nix_long_option);

   pciErrMon_non_option.user = pErrMonCmd;
   aalclp_add_non_option(&clp, &pciErrMon_non_option);

   pciErrMon_dash_only.user             = pErrMonCmd;
   aalclp_add_dash_only(&clp,             &pciErrMon_dash_only);

   pciErrMon_dash_dash_only.user        = pErrMonCmd;
   aalclp_add_dash_dash_only(&clp,        &pciErrMon_dash_dash_only);

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
   fprintf(fp, "   helloPCIeErrMon  <ERROR TYPE> \n\n");
   fprintf(fp, "      <ERROR TYPE>    --write-addr      OR     --wa         MMIO Write with Unaligned address \n");
   fprintf(fp, "                      --read-addr       OR     --ra         MMIO Read with Unaligned address \n");
   fprintf(fp, "                      --write-length    OR     --wl         MMIO Write with Unsupported length \n");
   fprintf(fp, "                      --read-length     OR     --rl         MMIO Read with Unsupported length \n");
   fprintf(fp, "                      --config-read     OR     --cr         Config Read to NLB CSR \n");
   fprintf(fp, "\n");

}

void showhelp(FILE *fp, struct _aalclp_gcs_compliance_data *gcs)
{
   help_msg_callback(fp, gcs);
}


END_C_DECLS


/// @addtogroup PCIeErrMon
/// @{


/// @brief   Since this is a simple application, our App class implements both the IRuntimeClient and IServiceClient
///           interfaces.  Since some of the methods will be redundant for a single object, they will be ignored.
///
class PCIeErrMonApp: public CAASBase, public IRuntimeClient, public IServiceClient
{
public:

   PCIeErrMonApp();
   ~PCIeErrMonApp();

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
///  Implementation
///
///////////////////////////////////////////////////////////////////////////////

/// @brief   Constructor registers this objects client interfaces and starts
///          the AAL Runtime. The member m_bisOK is used to indicate an error.
///
PCIeErrMonApp::PCIeErrMonApp() :
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
PCIeErrMonApp::~PCIeErrMonApp()
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
btInt PCIeErrMonApp::run()
{
   cout <<"=============================="<<endl;
   cout <<"= PCIe Error Injector Sample ="<<endl;
   cout <<"=============================="<<endl;

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
   ConfigRecord.Add(keyRegAFU_ID,"C000C966-0D82-4272-9AEF-FE5F84570612");


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

   if( ali_errnumOK != m_pALIBufferService->bufferAllocate(LPBK1_BUFFER_SIZE, &m_InputVirt)){
      m_bIsOK = false;
      m_Sem.Post(1);
      m_Result = -1;
      goto done_1;
   }

   m_InputSize = LPBK1_BUFFER_SIZE;
   m_InputPhys = m_pALIBufferService->bufferGetIOVA(m_InputVirt);
   if(0 == m_InputPhys){
      m_bIsOK = false;
      m_Result = -1;
      goto done_2;
   }

   if( ali_errnumOK !=  m_pALIBufferService->bufferAllocate(LPBK1_BUFFER_SIZE, &m_OutputVirt)){
      m_bIsOK = false;
      m_Sem.Post(1);
      m_Result = -1;
      goto done_2;
   }

   m_OutputSize = LPBK1_BUFFER_SIZE;
   m_OutputPhys = m_pALIBufferService->bufferGetIOVA(m_OutputVirt);
   if(0 == m_OutputPhys){
      m_bIsOK = false;
      m_Result = -1;
      goto done_3;
   }

   //=============================
   // Now we have the NLB Service
   //   now we can use it
   //=============================
   MSG("Running Test");
   if(true == m_bIsOK){

      ::memset( m_InputVirt, 0, m_InputSize);    // Input initialized to 0
      ::memset( m_OutputVirt, 0, m_OutputSize);    // Output initialized to 0

      struct CacheLine {                           // Operate on cache lines
         btUnsigned32bitInt uint[16];
      };
      struct CacheLine *pCL = reinterpret_cast<struct CacheLine *>(m_InputVirt);
      for ( btUnsigned32bitInt i = 0; i < m_InputSize / CL(1) ; ++i ) {
         pCL[i].uint[15] = i;
      };                         // Cache-Line[n] is zero except last uint = n


      if ( flag_is_set(errMonCmdLine.flags, PCIERRMON_CMD_WRITE_ADDR)){
         // MMIO Write with Unaligned address
         cout << "WRITE_ADDR\n";
         m_pALIMMIOService->mmioWrite64(CSR_SRC_ADDR, m_InputPhys);
      }
      if ( flag_is_set(errMonCmdLine.flags, PCIERRMON_CMD_READ_ADDR)){
         // MMIO Write with Unaligned address
         cout << "READ_ADDR\n";
         btUnsigned64bitInt     SrcContent;
         m_pALIMMIOService->mmioRead64(CSR_SRC_ADDR, &SrcContent);
      }
      if ( flag_is_set(errMonCmdLine.flags, PCIERRMON_CMD_WRITE_LENGTH)){
         // MMIO Write with unsupported length
         cout << "WRITE_LENGTH\n";

      }
      if ( flag_is_set(errMonCmdLine.flags, PCIERRMON_CMD_READ_LENGTH)){
         // MMIO Read with unsupported length
         cout << "READ_LENGTH\n";

         __m128i *pSrc_128  = NULL;
         __m128i *pDest_128 = NULL;

         pSrc_128  = (__m128i *)(m_pALIMMIOService->mmioGetAddress()+CSR_SRC_ADDR);
         pDest_128 = (__m128i *)(m_pALIMMIOService->mmioGetAddress()+CSR_DST_ADDR);

         ASSERT((NULL != pSrc_128) && (NULL != pDest_128));

         __m128i xmm0 = _mm_setzero_si128();

         xmm0 = _mm_load_si128(pSrc_128);
        _mm_stream_si128(pDest_128, xmm0);

         if ( memcmp((void *)pSrc_128, (void *)pDest_128, 16) != 0 ){
            cerr << "Data mismatch in src and dest buffers.\n";
         }

      }
      if ( flag_is_set(errMonCmdLine.flags, PCIERRMON_CMD_CONFIG_READ)){
         // Config Read to NLB CSR
         cout << "CONFIG_READ\n";

      }
   }
   MSG("Done Running Test");

   // Clean-up and return

done_3:
   m_pALIBufferService->bufferFree(m_OutputVirt);

done_2:
   m_pALIBufferService->bufferFree(m_InputVirt);

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
void PCIeErrMonApp::serviceAllocated(IBase *pServiceBase,
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

void PCIeErrMonApp::serviceAllocateFailed(const IEvent &rEvent)
{
   ERR("Failed to allocate Service");
    PrintExceptionDescription(rEvent);
   ++m_Result;                     // Remember the error
   m_bIsOK = false;

   m_Sem.Post(1);
}

 void PCIeErrMonApp::serviceReleased(TransactionID const &rTranID)
{
    MSG("Service Released");
   // Unblock Main()
   m_Sem.Post(1);
}

 void PCIeErrMonApp::serviceReleaseRequest(IBase *pServiceBase, const IEvent &rEvent)
 {
    MSG("Service unexpected requested back");
    if(NULL != m_pAALService){
       IAALService *pIAALService = dynamic_ptr<IAALService>(iidService, m_pAALService);
       ASSERT(pIAALService);
       pIAALService->Release(TransactionID());
    }
 }


 void PCIeErrMonApp::serviceReleaseFailed(const IEvent        &rEvent)
 {
    ERR("Failed to release a Service");
    PrintExceptionDescription(rEvent);
    m_bIsOK = false;
    m_Sem.Post(1);
 }


 void PCIeErrMonApp::serviceEvent(const IEvent &rEvent)
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

 void PCIeErrMonApp::runtimeStarted( IRuntime            *pRuntime,
                                      const NamedValueSet &rConfigParms)
 {
    m_bIsOK = true;
    m_Sem.Post(1);
 }

 void PCIeErrMonApp::runtimeStopped(IRuntime *pRuntime)
  {
     MSG("Runtime stopped");
     m_bIsOK = false;
     m_Sem.Post(1);
  }

 void PCIeErrMonApp::runtimeStartFailed(const IEvent &rEvent)
 {
    ERR("Runtime start failed");
    PrintExceptionDescription(rEvent);
 }

 void PCIeErrMonApp::runtimeStopFailed(const IEvent &rEvent)
 {
     MSG("Runtime stop failed");
     m_bIsOK = false;
     m_Sem.Post(1);
 }

 void PCIeErrMonApp::runtimeAllocateServiceFailed( IEvent const &rEvent)
 {
    ERR("Runtime AllocateService failed");
    PrintExceptionDescription(rEvent);
 }

 void PCIeErrMonApp::runtimeAllocateServiceSucceeded(IBase *pClient,
                                                     TransactionID const &rTranID)
 {
     MSG("Runtime Allocate Service Succeeded");
 }

 void PCIeErrMonApp::runtimeEvent(const IEvent &rEvent)
 {
     MSG("Generic message handler (runtime)");
 }
 // <begin IRuntimeClient interface>

/// @} group PCIeErrMon


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
   } else if ( 0!= ParseCmds(&errMonCmdLine, argc, argv) ) {
      cerr << "Error scanning command line." << endl;
      return 2;
   }

   PCIeErrMonApp theApp;
   if(!theApp.isOK()){
      ERR("Runtime Failed to Start");
      exit(1);
   }
   btInt Result = theApp.run();

   MSG("Done");
   return Result;
}

