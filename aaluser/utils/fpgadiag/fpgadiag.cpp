// Copyright(c) 2015-2016, Intel Corporation
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
/// @file fpgadiag.cpp
/// @brief Uses XL and IALIAFU to interact with ALI.
/// @ingroup
/// @verbatim
/// Accelerator Abstraction Layer Sample Application
///
///    This application is for example purposes only.
///    It is not intended to represent a model for developing commercially-deployable applications.
///    It is designed to show working examples of the AAL programming model and APIs.
///
/// This sample demonstrates the following:
///
/// AUTHORS: Tim Whisonant, Intel Corporation
///          Joseph Grecco, Intel Corporation
///			 Sadruta Chandrashekar, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 7/21/2014      TSW      Initial version(fpgasane).
/// 5/28/2015      SC       fpgadiag version.@endverbatim
//****************************************************************************
#include <aalsdk/AALLoggerExtern.h>
#include <aalsdk/aalclp/aalclp.h>
#include <aalsdk/kernel/ccipdriver.h>

#include <aalsdk/service/IALIAFU.h>
//#include <aalsdk/service/IALIClient.h>

#include <aalsdk/AAL.h>
#include <aalsdk/Runtime.h>

#include <aalsdk/utils/NLBVAFU.h>
#include "diag-nlb-common.h"
#include "diag-common.h"
#include "diag_defaults.h"

#define AALSDK_COPYRIGHT_STMNT "Copyright(c) 2003-2016, Intel Corporation" //TODO add in appropriate header file

USING_NAMESPACE(AAL)

// uncomment line below to use old (BDX-P) NLB AFU_IDs
//#define USE_BDX_NLB

#ifdef USE_BDX_NLB
// ---------- BDX-P AFU_IDs -----------
# ifndef NLB_MODE0_AFU_ID
#  define NLB_MODE0_AFU_ID "C000C966-0D82-4272-9AEF-FE5F84570612"
# endif
# ifndef NLB_MODE3_AFU_ID
#  define NLB_MODE3_AFU_ID "751E795F-7DA4-4CC6-8309-935132BCA9B6"
# endif
# ifndef NLB_MODE7_AFU_ID
#  define NLB_MODE7_AFU_ID "A944F6E7-15D3-4D95-9452-15DBD47C76BD"
# endif
#else
// ---------- SKX-P AFU_IDs -----------
# ifndef NLB_MODE0_AFU_ID
#  define NLB_MODE0_AFU_ID "D8424DC4-A4A3-C413-F89E-433683F9040B"
# endif
# ifndef NLB_MODE3_AFU_ID
#  define NLB_MODE3_AFU_ID "F7DF405C-BD7A-CF72-22F1-44B0B93ACD18"
# endif
# ifndef NLB_MODE7_AFU_ID
#  define NLB_MODE7_AFU_ID "7BAF4DEA-A57C-E91E-168A-455D9BDA88A3"
# endif
#endif

#ifdef INFO
# undef INFO
#endif // INFO
#if 1
# define INFO(x) AAL_INFO(LM_Any, __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() : " << x << std::endl)
#else
# define INFO(x)
#endif

#ifdef ERR
# undef ERR
#endif // ERR
#if 1
# define ERR(x) AAL_ERR(LM_Any, __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() : " << x << std::endl)
#else
# define ERR(x)
#endif

// Change DBG_HOOK to 1 if you want an opportunity to attach the debugger.
// After attaching, set gWaitForDebuggerAttach to 0 via the debugger to unblock the app.
#define DBG_HOOK 0
#if DBG_HOOK
btBool gWaitForDebuggerAttach = true;
#endif // DBG_HOOK

//#define iidIALIAFU __AAL_IID(AAL_sysAAL, 0x0007)

/// @addtogroup ALIapp
/// @{

BEGIN_C_DECLS
struct NLBCmdLine gCmdLine =
{
   AALSDK_COPYRIGHT_STMNT,
   NLB_MODE,
   NLB_TITLE,
   NLB_CMD_FLAG_BANDWIDTH,
   0,
   0,
   DEFAULT_BEGINCL,
   DEFAULT_ENDCL,
   DEFAULT_MULTICL,
   DEFAULT_DSMPHYS,
   DEFAULT_SRCPHYS,
   DEFAULT_DSTPHYS,
   DEFAULT_FPGA_CLK_FREQ,
   DEFAULT_CX,
   DEFAULT_HQW,
   DEFAULT_SQW,
   DEFAULT_STRIDES,
#if   defined( __AAL_WINDOWS__ )
# error TODO
#elif defined( __AAL_LINUX__ )
   { 0, 0 },
   0,
   0,
   0,
   0,
   0,
#endif // OS
   // defaults
   {
      NLB_MIN_CL,
      NLB_MAX_CL,
      DEFAULT_BEGINCL,
      DEFAULT_ENDCL,
      DEFAULT_MULTICL,
      DEFAULT_DSMPHYS,
      DEFAULT_SRCPHYS,
      DEFAULT_DSTPHYS,
      DEFAULT_FPGA_CLK_FREQ,
      DEFAULT_WARMFPGACACHE,
      DEFAULT_COOLFPGACACHE,
      DEFAULT_COOLCPUCACHE,
      DEFAULT_NOBW,
      DEFAULT_TABULAR,
      DEFAULT_SUPPRESSHDR,
      DEFAULT_WT,
      DEFAULT_WB,
      DEFAULT_RDS,
      DEFAULT_RDI,
      DEFAULT_CONT,
#if   defined( __AAL_WINDOWS__ )
# error TODO
#elif defined( __AAL_LINUX__ )
      DEFAULT_TOUSEC,
      DEFAULT_TOMSEC,
      DEFAULT_TOSEC,
      DEFAULT_TOMIN,
      DEFAULT_TOHOUR,
#endif // OS
      DEFAULT_POLL,
      DEFAULT_CSR_WRITE,
      DEFAULT_UMSG_DATA,
      DEFAULT_UMSG_HINT,
      DEFAULT_READ_VA,
      DEFAULT_READ_VL0,
      DEFAULT_READ_VH0,
      DEFAULT_READ_VH1,
      DEFAULT_READ_VR,
      DEFAULT_WRITE_VA,
      DEFAULT_WRITE_VL0,
      DEFAULT_WRITE_VH0,
      DEFAULT_WRITE_VH1,
      DEFAULT_WRITE_VR,
      DEFAULT_WRFENCE_VA,
      DEFAULT_WRFENCE_VL0,
      DEFAULT_WRFENCE_VH0,
      DEFAULT_WRFENCE_VH1,
      DEFAULT_AWP,
      DEFAULT_ST,
	  DEFAULT_UT,
      DEFAULT_MINCX,
      DEFAULT_MAXCX,
      DEFAULT_CX,
      DEFAULT_HQW,
      DEFAULT_SQW,
      DEFAULT_MINHQW,
      DEFAULT_MAXHQW,
      DEFAULT_MINSQW,
      DEFAULT_MAXSQW,
      DEFAULT_STRIDES,
      DEFAULT_MIN_STRIDES,
      DEFAULT_MAX_STRIDES
   },
   0,
   {
      { 0, 0, 0, 0ULL, 0.0, { 0, } },
      { 0, 0, 0, 0ULL, 0.0, { 0, } },
      { 0, 0, 0, 0ULL, 0.0, { 0, } },
      { 0, 0, 0, 0ULL, 0.0, { 0, } },
      { 0, 0, 0, 0ULL, 0.0, { 0, } },
      { 0, 0, 0, 0ULL, 0.0, { 0, } },
      { 0, 0, 0, 0ULL, 0.0, { 0, } },
      { 0, 0, 0, 0ULL, 0.0, { 0, } }
   },
   0,
   std::string(DEFAULT_TARGET_AFU),
   DEFAULT_TARGET_DEV,
   std::string(DEFAULT_TEST_MODE),
   0,
   DEFAULT_BUS_NUMBER,
   DEFAULT_DEVICE_NUMBER,
   DEFAULT_FUNCTION_NUMBER
};

END_C_DECLS


////////////////////////////////////////////////////////////////////////////////
BEGIN_C_DECLS


AALCLP_DECLARE_GCS_COMPLIANT(stdout,
                             "fpgadiag",
                             "0.0.0",
                             "",
                             nlb_help_message_callback,
                             &gCmdLine)

int ParseCmds(struct NLBCmdLine * , int , char *[] );
//int verifycmds(struct NLBCmdLine * );
END_C_DECLS
////////////////////////////////////////////////////////////////////////////////

#define NANOSEC_PER_MILLI(x)      ((x) * 1000 * 1000)

//////////////////////////////////////////////////////////////////////////////


inline std::ostream & PASS(std::ostream &os)
{
   if ( isatty(1) ) {
      const char Esc[] = { 0x1b, '[', '3', '2', 'm', 0 };
      os << Esc;
   }
   return os;
}
inline std::ostream & FAIL(std::ostream &os)
{
   if ( isatty(1) ) {
      const char Esc[] = { 0x1b, '[', '3', '1', 'm', 0 };
      os << Esc;
   }
   return os;
}
inline std::ostream & NORMAL(std::ostream &os)
{
   if ( isatty(1) ) {
      const char Esc[] = { 0x1b, '[', '0', 'm', 0 };
      os << Esc;
   }
   return os;
}

////////////////////////////////////////////////////////////////////////////////

CMyApp::CMyApp() :
   m_AFUTarget(DEFAULT_TARGET_AFU),
   m_DevTarget(DEFAULT_TARGET_DEV),
   m_pRuntime(NULL),
   m_pNLBService(NULL),
   m_pFMEService(NULL),
   m_pDiagBufferService(NULL),
   m_pALIBufferService(NULL),
   m_pALIMMIOService(NULL),
   m_pALIResetService(NULL),
   m_pALIuMSGService(NULL),
   m_pALIPerf(NULL),
   m_isOK(false),
   m_pVTP_AALService(NULL),
   m_pVTPService(NULL),
   m_VTPDFHOffset(-1),
   m_VTPActive(false),
   m_DSMVirt(NULL),
   m_DSMPhys(0),
   m_DSMSize(0),
   m_InputVirt(NULL),
   m_InputPhys(0),
   m_InputSize(0),
   m_OutputVirt(NULL),
   m_OutputPhys(0),
   m_OutputSize(0),
   m_UMsgVirt(NULL),
   m_UMsgPhys(0),
   m_UMsgSize(0)
{
	m_Sem.Create(0, 1);
	SetInterface(iidRuntimeClient, dynamic_cast<IRuntimeClient *>(this));
    SetInterface(iidServiceClient, dynamic_cast<IServiceClient *>(this));
}

CMyApp::~CMyApp()
{
//   Stop();
   m_Sem.Destroy();
}


void CMyApp::Stop()
{

   // Freed all three so now Release() the Service through the Services IAALService::Release() method
   if ( NULL != m_pFMEService ) {
		 (dynamic_ptr<IAALService>(iidService, m_pFMEService))->Release(TransactionID());
		 Wait(); // For service freed notification.
		 m_pFMEService = NULL;
   }

   if ( NULL != m_pNLBService ) {
		 (dynamic_ptr<IAALService>(iidService, m_pNLBService))->Release(TransactionID());
		 Wait(); // For service freed notification.
		 m_pNLBService = NULL;
   }

   if (NULL != m_pVTP_AALService){
	   (dynamic_ptr<IAALService>(iidService, m_pVTP_AALService))->Release(TransactionID());
		Wait(); // For service freed notification.
		m_pVTP_AALService = NULL;
   }

   if ( NULL != m_pRuntime ) {
      m_pRuntime->stop();
      Wait(); // For runtime stopped notification.
      m_pRuntime = NULL;
   }

   Post(); // Wake up main, if waiting
}

void CMyApp::runtimeStarted(IRuntime            *pRT,
							const NamedValueSet &Args)
{
   ASSERT(pRT->IsOK());
   if ( !pRT->IsOK() ) {
      return;
   }

   m_pRuntime = pRT;

   btcString AFUName = "ALIAFU";

   INFO("Allocating " << AFUName << " Service");

   NamedValueSet Manifest;
   NamedValueSet ConfigRecord;

   if ( 0 == strcmp(AFUTarget().c_str(), "ALIAFUTarget_FPGA") ) {      // Use FPGA hardware

  	   ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libHWALIAFU");
  	   ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_AIA_NAME, "libAASUAIA");

           if (flag_is_set(gCmdLine.cmdflags, NLB_CMD_FLAG_BUS_NUMBER)) {
              cout << "Using PCIe bus 0x" << hex << uint_type(gCmdLine.busnum) << endl;
              ConfigRecord.Add(keyRegBusNumber, uint_type(gCmdLine.busnum));
           }
           if (flag_is_set(gCmdLine.cmdflags, NLB_CMD_FLAG_DEVICE_NUMBER)) {
              cout << "Using PCIe device 0x" << hex << uint_type(gCmdLine.devnum) << endl;
              ConfigRecord.Add(keyRegDeviceNumber, uint_type(gCmdLine.devnum));
           }
           if (flag_is_set(gCmdLine.cmdflags, NLB_CMD_FLAG_FUNCTION_NUMBER)) {
              cout << "Using PCIe function 0x" << hex << uint_type(gCmdLine.funnum) << endl;
              ConfigRecord.Add(keyRegfuntionNumber, uint_type(gCmdLine.funnum));
           }

  	   if(0 == strcmp(TestMode().c_str(), "TestMode_read") ||
		  0 == strcmp(TestMode().c_str(), "TestMode_write") ||
		  0 == strcmp(TestMode().c_str(), "TestMode_trput")){

  		   ConfigRecord.Add(keyRegAFU_ID, NLB_MODE3_AFU_ID);
  		   Manifest.Add(keyRegAFU_ID, NLB_MODE3_AFU_ID);

  	   }else if(0 == strcmp(TestMode().c_str(), "TestMode_lpbk1")){

  		   ConfigRecord.Add(keyRegAFU_ID, NLB_MODE0_AFU_ID);
  		   Manifest.Add(keyRegAFU_ID, NLB_MODE0_AFU_ID);

  	   }else if(0 == strcmp(TestMode().c_str(), "TestMode_sw")){

 		   ConfigRecord.Add(keyRegAFU_ID, NLB_MODE7_AFU_ID);
 		   Manifest.Add(keyRegAFU_ID, NLB_MODE7_AFU_ID);

  	   }else if(0 == strcmp(TestMode().c_str(), "TestMode_atomic")){

		   /*ConfigRecord.Add(keyRegAFU_ID,"41BAFB9D-D97E-43CF-967D-22E837CD2182");
		   Manifest.Add(keyRegAFU_ID,"41BAFB9D-D97E-43CF-967D-22E837CD2182");*/

         ConfigRecord.Add(keyRegAFU_ID,"C000C966-0D82-4272-9AEF-FE5F84570612");
         Manifest.Add(keyRegAFU_ID,"C000C966-0D82-4272-9AEF-FE5F84570612"); //TODO: Remove me and uncomment about lines

 	   }else{

  		  cout << "Unsupported Test mode." << endl;
  		  exit(1);
  	   }

  	   if(-1 != DevTarget()){

  		   ConfigRecord.Add(keyRegSubDeviceNumber, DevTarget());
  	   }
  }else if ( 0 == strcasecmp(AFUTarget().c_str(), "ALIAFUTarget_ASE") ) {         // Use ASE based RTL simulation

	   Manifest.Add(keyRegHandle, 20);

	   ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libASEALIAFU");
  	   ConfigRecord.Add(AAL_FACTORY_CREATE_SOFTWARE_SERVICE,true);

   }else if ( 0 == strcasecmp(AFUTarget().c_str(), "ALIAFUTarget_SWSIM") ) {

//      ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libSWSimALIAFU");
//      ConfigRecord.Add(AAL_FACTORY_CREATE_SOFTWARE_SERVICE,true);

      ERR("--target=swsim is unsupported in this release. Please choose one of 'ase' or 'fpga'.");
      exit(1);
   }

  	Manifest.Add(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &ConfigRecord);
  	Manifest.Add(AAL_FACTORY_CREATE_SERVICENAME, AFUName);
  	Manifest.Add(ALIAFU_NVS_KEY_TARGET, AFUTarget().c_str());

   #if DBG_HOOK
  	INFO(Manifest);
   #endif // DBG_HOOK

  	TransactionID afu_tid(CMyApp::AFU);
  	pRT->allocService(dynamic_cast<IBase *>(this), Manifest, afu_tid);

  	// Modify the manifest for the NLB AFU
    Manifest.Delete(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED);
    ConfigRecord.Delete(keyRegAFU_ID);

    ConfigRecord.Add(keyRegAFU_ID, "BFAF2AE9-4A52-46E3-82FE-38F0F9E17764");
    Manifest.Add(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &ConfigRecord);

	#if DBG_HOOK
	INFO(Manifest);
	#endif // DBG_HOOK

	// Allocate the AFU
	TransactionID fme_tid(CMyApp::FME);
	pRT->allocService(dynamic_cast<IBase *>(this), Manifest, fme_tid);
}

void CMyApp::runtimeStopped(IRuntime *pRT)
{
   INFO("Runtime Stopped");
   Post();
}

void CMyApp::runtimeStartFailed(const IEvent &e)
{
   if ( AAL_IS_EXCEPTION(e.SubClassID()) ) {
      PrintExceptionDescription(e);
   }
   m_bIsOK = false;
   INFO("Runtime Start Failed");
   Post();
}

void CMyApp::runtimeAllocateServiceFailed(IEvent const &e)
{
   if ( AAL_IS_EXCEPTION(e.SubClassID()) ) {
      PrintExceptionDescription(e);
   }
   m_bIsOK = false;
   ERR("Service Allocate Failed (rt)");
}

void CMyApp::runtimeAllocateServiceSucceeded(IBase               *pServiceBase,
											 TransactionID const &tid)
{
   INFO("Service Allocated (rt)");
}

void CMyApp::runtimeEvent(const IEvent &e)
{
   if ( AAL_IS_EXCEPTION(e.SubClassID()) ) {
        PrintExceptionDescription(e);
        m_bIsOK = false;
        Post();
        return;
     }

     INFO("Unknown event (rt)");
     Post();
}

void CMyApp::runtimeStopFailed(const IEvent &e)
{
   if ( AAL_IS_EXCEPTION(e.SubClassID()) ) {
      PrintExceptionDescription(e);
   }
   m_bIsOK = false;
   INFO("Runtime Stop Failed");
   Post();
}

void CMyApp::runtimeCreateOrGetProxyFailed(const IEvent &e)
{
   if ( AAL_IS_EXCEPTION(e.SubClassID()) ) {
      PrintExceptionDescription(e);
   }
   m_bIsOK = false;
   INFO("Runtime Create or Get Proxy Failed");
   Post();
}

void CMyApp::serviceAllocated(IBase               *pServiceBase,
							  TransactionID const &tid)
{

	if(tid.ID() == CMyApp::AFU){

	      // Save the IBase for the Service. Through it we can get any other
	      //  interface implemented by the Service
	      m_pNLBService = pServiceBase;
	      ASSERT(NULL != m_pNLBService);
	      if ( NULL == m_pNLBService ) {
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

	      m_pDiagBufferService = m_pALIBufferService;

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

	      // Documentation says HWALIAFU Service publishes
	      //    IALIUMsg as subclass interface
	      m_pALIuMSGService = dynamic_ptr<IALIUMsg>(iidALI_UMSG_Service, pServiceBase);
	      ASSERT(NULL != m_pALIuMSGService);
	      if ( NULL == m_pALIuMSGService ) {
	         m_bIsOK = false;
	         return;
	      }
   }else if(tid.ID() == CMyApp::FME){

	   m_pFMEService = pServiceBase;
	   ASSERT(NULL != m_pFMEService);
	   if ( NULL == m_pFMEService ) {
		  m_bIsOK = false;
		  return;
	   }

	   // Documentation says HWALIAFU Service publishes
	   //    IALIPerf as subclass interface. Used to access performance monitors
	   m_pALIPerf = dynamic_ptr<IALIPerf>(iidALI_PERF_Service, pServiceBase);
	  /* ASSERT(NULL != m_pALIPerf);
	   if ( NULL == m_pALIPerf ) {
		  m_bIsOK = false;
		  return;
	   }
	   */
   }else if(tid.ID() == CMyApp::VTP){
	  // Save the IBase for the VTP Service.
	  m_pVTP_AALService = pServiceBase;
	  ASSERT(NULL != m_pVTP_AALService);
	  if ( NULL == m_pVTP_AALService ) {
		 m_bIsOK = false;
		 return;
	  }

	  // Documentation says VTP Service publishes
	  //    IVTP as subclass interface. Used for allocating shared
	  //    buffers that support virtual addresses from AFU
	  m_pVTPService = dynamic_ptr<IMPFVTP>(iidMPFVTPService, pServiceBase);
	  ASSERT(NULL != m_pVTPService);
	  if ( NULL == m_pVTPService ) {
		 m_bIsOK = false;
		 return;
	  }

	  m_pVTPService->vtpReset();
	  m_VTPActive = true;
	  m_pDiagBufferService = dynamic_cast<IALIBuffer *>(m_pVTPService);
   }

	if( m_pFMEService &&
		m_pNLBService)
	{
		if(true == m_VTPActive){
			if ( m_pVTPService ){
	        INFO("Service Allocated");
	        allocateWorkspaces();
	        Post();
			}
		}else{
	   	  INFO("Service Allocated");
	   	  allocateWorkspaces();
	   	  Post();
		}
	}
}

void CMyApp::allocateWorkspaces()
{
	// Allocate first of 3 Workspaces needed.  Use the TransactionID to tell which was allocated.
   //   In workspaceAllocated() callback we allocate the rest

   m_DSMSize = NLB_DSM_SIZE;
   if( ali_errnumOK != m_pDiagBufferService->bufferAllocate(NLB_DSM_SIZE, &m_DSMVirt)){
	  m_bIsOK = false;
	  return;
   }

   m_InputSize = MAX_NLB_WKSPC_SIZE;
   if( ali_errnumOK != m_pDiagBufferService->bufferAllocate(MAX_NLB_WKSPC_SIZE, &m_InputVirt)){
	  m_bIsOK = false;
	  return;
   }

   m_OutputSize = MAX_NLB_WKSPC_SIZE;
   if( ali_errnumOK != m_pDiagBufferService->bufferAllocate(MAX_NLB_WKSPC_SIZE, &m_OutputVirt)){
	  m_bIsOK = false;
	  return;
   }

   if (m_VTPActive) {	//FIXME: In case of VTP, PhysAddr member variables are actually pointing to VirtAddr.
	   m_DSMPhys = btPhysAddr(m_DSMVirt);
	   m_InputPhys = btPhysAddr(m_InputVirt);
	   m_OutputPhys = btPhysAddr(m_OutputVirt);

   }else {
	   m_DSMPhys = m_pDiagBufferService->bufferGetIOVA(m_DSMVirt);
	   m_InputPhys = m_pDiagBufferService->bufferGetIOVA(m_InputVirt);
	   m_OutputPhys = m_pDiagBufferService->bufferGetIOVA(m_OutputVirt);
   }

   btUnsignedInt numUmsg = m_pALIuMSGService->umsgGetNumber();
   m_UMsgVirt = m_pALIuMSGService->umsgGetAddress(0);
   m_UMsgSize = numUmsg * KB(4);

   if(NULL == m_UMsgVirt){
	  ERR("No uMSG support");
   }
}

void CMyApp::serviceAllocateFailed(const IEvent &e)
{
   if ( AAL_IS_EXCEPTION(e.SubClassID()) ) {
      PrintExceptionDescription(e);
   }

  if ( e.Has(iidExTranEvent) &&
	   0 == strcmp (dynamic_ref<IExceptionTransactionEvent>(iidExTranEvent, e).Description(), "Resources unavailable")) {
         //ExceptionTransaction

	  cout << FAIL << TestMode() << " is unsupported in the current bitstream loaded. Please program the correct bitstream and try again." << NORMAL << endl;
   }

   m_bIsOK = false;
   ERR("Service Allocate Failed");
   Post();
}


void CMyApp::serviceEvent(const IEvent &e)
{
  if ( AAL_IS_EXCEPTION(e.SubClassID()) ) {
	  PrintExceptionDescription(e);
	  m_bIsOK = false;
	  Post();
	  return;
   }

   INFO("Unknown event");
   Post();
}

void CMyApp::serviceReleased(TransactionID const &tid)
{
   INFO("Service Released");
   Post();
}
void CMyApp::serviceReleaseFailed(const IEvent &e)
{
   if ( AAL_IS_EXCEPTION(e.SubClassID()) ) {
      PrintExceptionDescription(e);
   }
   m_bIsOK = false;
   ERR("Service Release Failed");
   Post();
}

void CMyApp::serviceFreed(TransactionID const &tid)
{
	if( NULL != m_pDiagBufferService ) {
	  // Release the Workspaces
	  m_pDiagBufferService->bufferFree(m_InputVirt);
	  m_pDiagBufferService->bufferFree(m_OutputVirt);
	  m_pDiagBufferService->bufferFree(m_DSMVirt);
	}
    INFO("Service Freed");
    Post();
}

void CMyApp::StartVTP()
{
	// Ask ALI for a BBB with MPF's feature ID and the expected VTP GUID
	NamedValueSet filter;
	filter.Add( ALI_GETFEATURE_TYPE_KEY, static_cast<ALI_GETFEATURE_TYPE_DATATYPE>(ALI_DFH_TYPE_BBB) );
	filter.Add( ALI_GETFEATURE_GUID_KEY, (ALI_GETFEATURE_GUID_DATATYPE)MPF_VTP_BBB_GUID );

	// FIXME: This is here only because of a caching bug in
	// ASEALIAFU::mmioGetFeatureAddress() in SR-5.0.2-Beta. Once fixed
	// this call can be removed.

	m_pALIMMIOService->mmioGetAddress();

	if ( false == m_pALIMMIOService->mmioGetFeatureOffset( &m_VTPDFHOffset, filter ) ) {
	 // No VTP found - this could mean that VTP is not enabled in MPF
	  m_VTPActive = false;
	  return;
	}

   m_VTPActive = true;

   INFO("Searching for VTP Service");
   NamedValueSet Manifest;
   NamedValueSet ConfigRecord;

   // Allocate VTP service
   // Service Library to use
   ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libMPF");
   ConfigRecord.Add(AAL_FACTORY_CREATE_SOFTWARE_SERVICE, true);

   // Add the Config Record to the Manifest describing what we want to allocate
   Manifest.Add(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &ConfigRecord);

   // The VTPService will reuse the already established interfaces presented by
   // the ALIAFU service
   Manifest.Add(ALIAFU_IBASE_KEY, static_cast<ALIAFU_IBASE_DATATYPE>(m_pNLBService));

   // MPFs feature ID, used to find correct features in DFH list
   Manifest.Add(MPF_FEATURE_ID_KEY, static_cast<MPF_FEATURE_ID_DATATYPE>(1));

   // In the future, everything could be figured out by just giving the service name.
   Manifest.Add(AAL_FACTORY_CREATE_SERVICENAME, "VTP");

   TransactionID vtp_tid(CMyApp::VTP);
   m_pRuntime->allocService(dynamic_cast<IBase *>(this), Manifest, vtp_tid);
   m_Sem.Wait();
   if (!m_bIsOK || !m_VTPActive) {
      INFO("No VTP service found\n");
   } else {
      INFO("Using VTP translation...\n");
   }

}


int main(int argc, char *argv[])
{
   btInt res      = 0;
   btInt totalres = 0;

   if ( argc < 2 ) {
	   MyNLBShowHelp(stdout, &_aalclp_gcs_data);
	   return 1;
   } else if ( 0 != ParseCmds(&gCmdLine, argc, argv) ) {
      cerr << "Error scanning command line." << endl;
      return 2;
   } else if ( flag_is_set(gCmdLine.flags, MY_CMD_FLAG_HELP|MY_CMD_FLAG_VERSION) ) {
      return 0; // per GCS
   } else if ( !NLBVerifyCmdLine(gCmdLine, std::cout) ) {
      return 3;
   }

   if ( flag_is_set(gCmdLine.cmdflags, NLB_CMD_FLAG_HELP) ) {
       return 0; // Exit after displaying the help menu
      }

   cout << endl
        << "FpgaDiag - FPGA Diagnostics Test:" << endl;

#if DBG_HOOK
   cerr << "Waiting for debugger attach.." << endl;
  /* while ( gWaitForDebuggerAttach ) {
      SleepSec(1);
   }*/
   // Init the AAL logger.
   pAALLogger()->AddToMask(LM_All, 8); // All subsystems
   pAALLogger()->SetDestination(ILogger::CERR);
#else
   if ( gCmdLine.LogLevel > 0 ) {
      pAALLogger()->AddToMask(LM_All, gCmdLine.LogLevel);
      //pAALLogger()->SetDestination(ILogger::CERR);
      pAALLogger()->SetLogPID(true);
      pAALLogger()->SetLogTimeStamp(true);
   }
#endif // DBG_HOOK

   CMyApp        myapp;
   NamedValueSet args;
   Runtime       aal(&myapp);

   myapp.AFUTarget(gCmdLine.AFUTarget);
   myapp.DevTarget(gCmdLine.DevTarget);
   myapp.TestMode(gCmdLine.TestMode);

   if ( (0 == myapp.AFUTarget().compare(ALIAFU_NVS_VAL_TARGET_ASE)) ||
        (0 == myapp.AFUTarget().compare(ALIAFU_NVS_VAL_TARGET_SWSIM)) ) {
      args.Add(SYSINIT_KEY_SYSTEM_NOKERNEL, true);
   } else {
      NamedValueSet ConfigRecord;
      ConfigRecord.Add(AALRUNTIME_CONFIG_BROKER_SERVICE, "librrmbroker");
      args.Add(AALRUNTIME_CONFIG_RECORD, &ConfigRecord);
   }

   INFO("Starting the AAL Runtime");
   if ( aal.start(args) ) {
      myapp.Wait(); // For service allocated notification.
   } else {
      ERR("AAL Runtime start failed");
      return 4;     // Runtime could not start
   }

   if ( !myapp.IsOK() ) {
      // runtime start failed.
	  myapp.Stop();
	  myapp.Wait(); // For runtime stopped notification.
      return 5;
   }

   cout << "Trying VTP\n";
   myapp.StartVTP();
   if ( true == myapp.VTPActive()){
	   cout << "VTP Active.\n";
   }else{
	   cout << "VTP not Active.\n";
   }

   if ( (0 == myapp.TestMode().compare(NLB_TESTMODE_LPBK1)))
      {
   		// Run NLB test, which performs sw data verification.
   		CNLBLpbk1 nlb_lpbk1(&myapp);

   		cout << " * Data Copy - LPBK1" << flush;
   		res = nlb_lpbk1.RunTest(gCmdLine);
   		totalres += res;
   		if ( 0 == res ) {
   		  cout << PASS << "PASS - DATA VERIFIED";
   		} else {
   		  cout << FAIL << "ERROR";
   		}
   		cout << NORMAL << endl;
	}
	else if ( (0 == myapp.TestMode().compare(NLB_TESTMODE_READ)))
	{
   		// Run NLB read test.
	      CNLBMode3 nlb_read(&myapp);

   		cout << " * Read Bandwidth from Memory - READ" << flush;
   		res = nlb_read.RunTest(gCmdLine);
   		totalres += res;
   		if ( 0 == res ) {
   		  cout << PASS << "PASS - DATA VERIFICATION DISABLED";
   		} else {
   		  cout << FAIL << "ERROR";
   		}
   		cout << NORMAL << endl;
	}
	else if ( (0 == myapp.TestMode().compare(NLB_TESTMODE_WRITE)))
	{
   		// Run NLB write test.
	      CNLBMode3 nlb_write(&myapp);

   		cout << " * Write Bandwidth from Memory - WRITE" << flush;
   		res = nlb_write.RunTest(gCmdLine);
   		totalres += res;
   		if ( 0 == res ) {
   		  cout << PASS << "PASS - DATA VERIFICATION DISABLED";
   		} else {
   		  cout << FAIL << "ERROR";
   		}
   		cout << NORMAL << endl;
	}
	else if ( (0 == myapp.TestMode().compare(NLB_TESTMODE_TRPUT)))
	{
   		// Run NLB  trput test.
	      CNLBMode3 nlb_trput(&myapp);

   		cout << " * Simultaneous Read/Write Bandwidth - TRPUT" << flush;
   		res = nlb_trput.RunTest(gCmdLine);
   		totalres += res;
   		if ( 0 == res ) {
   		  cout << PASS << "PASS - DATA VERIFICATION DISABLED";
   		} else {
   		  cout << FAIL << "ERROR";
   		}
   		cout << NORMAL << endl;
	}
	else if ( (0 == myapp.TestMode().compare(NLB_TESTMODE_SW)))
	{
   	   // Run an SW Test..
   	   // * report bandwidth in GiB/s
   	   CNLBSW nlb_sw(&myapp);

   	   cout << " * SW test " << flush;
   	   res = nlb_sw.RunTest(gCmdLine);
   	   totalres += res;
   	   if ( 0 == res ) {
   		  cout << PASS << "PASS - DATA VERIFIED";
   	   } else {
   		  cout << FAIL << "ERROR";
   	   }
   	   cout << NORMAL << endl
   			<< endl;
     }
	else if ( (0 == myapp.TestMode().compare(NLB_TESTMODE_ATOMIC)))
		{
	   	   // Run an SW Test..
	   	   // * report bandwidth in GiB/s
	   	   CNLBAtomic nlb_atomic(&myapp);

	   	   cout << " * Atomic test " << flush;
	   	   res = nlb_atomic.RunTest(gCmdLine);
	   	   totalres += res;
	   	   if ( 0 == res ) {
	   		  cout << PASS << "PASS";
	   	   } else {
	   		  cout << FAIL << "ERROR";
	   	   }
	   	   cout << NORMAL << endl
	   			<< endl;
	     }

   INFO("Stopping the AAL Runtime");
   myapp.Stop();

   if ( totalres > 0 ) {
      ERR("Test FAILED with " << res << " error" << ((res > 1) ? "s." : "."));
   } else {
      INFO("Test PASSED");
   }

   return totalres;
}

btInt INLB::ResetHandshake()
{
   btInt      res = 0;
   bt32bitCSR csr;
   bt32bitCSR tmp;

   volatile nlb_vafu_dsm *pAFUDSM = (volatile nlb_vafu_dsm *)m_pMyApp->DSMPhys();

   const btInt StatusTimeoutMillis = 250;
   btInt MaxPoll = NANOSEC_PER_MILLI(StatusTimeoutMillis);

   // Assert CAFU Reset
   csr = 0;
   m_pALIMMIOService->mmioRead32(QLP_CSR_CIPUCTL, &csr);
   flag_setf(csr, CIPUCTL_RESET_BIT);
   m_pALIMMIOService->mmioWrite32(QLP_CSR_CIPUCTL, csr);

   // Poll CAFU Status until ready.
   do
   {
      SleepNano(500);
      MaxPoll -= 500;
      csr = 0;
      m_pALIMMIOService->mmioRead32(QLP_CSR_CAFU_STATUS, &csr);
   }while( flag_is_clr(csr, CAFU_STATUS_READY_BIT) && (MaxPoll >= 0) );

   if ( MaxPoll < 0 ) {
      cerr << "The maximum timeout for CAFU_STATUS ready bit was exceeded." << endl;
      return 1;
   }

   // De-assert CAFU Reset
   csr = 0;
   m_pALIMMIOService->mmioRead32(QLP_CSR_CIPUCTL, &csr);
   flag_clrf(csr, CIPUCTL_RESET_BIT);
   m_pALIMMIOService->mmioWrite32(QLP_CSR_CIPUCTL, csr);

   tmp = 0;
   m_pALIMMIOService->mmioRead32(QLP_CSR_CIPUCTL, &tmp);
   ASSERT(csr == tmp);


   const btInt AFUIDTimeoutMillis = 250;
   MaxPoll = NANOSEC_PER_MILLI(AFUIDTimeoutMillis);

   // zero the DSM status fields
   ::memset((void *)pAFUDSM, 0, sizeof(nlb_vafu_dsm));

   // Set DSM base, high then low
   m_pALIMMIOService->mmioWrite64(CSR_AFU_DSM_BASEL, m_pMyApp->DSMPhys());

   // Poll for AFU ID
   do
   {
      SleepNano(500);
      MaxPoll -= 500;
   }while( (0 == pAFUDSM->afuid[0]) && (MaxPoll >= 0) );

   if ( MaxPoll < 0 ) {
      cerr << "The maximum timeout for AFU ID was exceeded." << endl;
      return 1;
   }


   ASSERT(0x84570612 == pAFUDSM->afuid[0]);
   if ( 0x84570612 != pAFUDSM->afuid[0] ) {
      ++res;
   }

   ASSERT(0x9aeffe5f == pAFUDSM->afuid[1]);
   if ( 0x9aeffe5f != pAFUDSM->afuid[1] ) {
      ++res;
   }

   ASSERT(0x0d824272 == pAFUDSM->afuid[2]);
   if ( 0x0d824272 != pAFUDSM->afuid[2] ) {
      ++res;
   }

   ASSERT(0xc000c966 == pAFUDSM->afuid[3]);
   if ( 0xc000c966 != pAFUDSM->afuid[3] ) {
      ++res;
   }

   return res;
}

btInt INLB::CacheCooldown(btVirtAddr CoolVirt, btPhysAddr CoolPhys, btWSSize CoolSize, const NLBCmdLine &cmd)
{
   btInt res = 0;
   const btInt StopTimeoutMillis = 250;
   btInt MaxPoll = StopTimeoutMillis;

   const btUnsigned32bitInt CoolOffData = 0xc001c001;

   volatile btUnsigned32bitInt *pCoolOff    = (volatile btUnsigned32bitInt *)CoolVirt;
   volatile btUnsigned32bitInt *pEndCoolOff = (volatile btUnsigned32bitInt *)pCoolOff +
                                     (CoolSize / sizeof(btUnsigned32bitInt));

   for ( ; pCoolOff < pEndCoolOff ; ++pCoolOff ) {
      *pCoolOff = CoolOffData;
   }

   if(NULL != m_pVTPService){
      m_pVTPService->vtpReset();
   }

   //Set DSM base, high then low
   m_pALIMMIOService->mmioWrite64(CSR_AFU_DSM_BASEL, m_pMyApp->DSMPhys());

   volatile nlb_vafu_dsm *pAFUDSM = (volatile nlb_vafu_dsm *)m_pMyApp->DSMVirt();

   // Assert Device Reset
   m_pALIMMIOService->mmioWrite32(CSR_CTL, 0);

   // Clear the DSM status fields
   ::memset((void *)pAFUDSM, 0, sizeof(nlb_vafu_dsm));

   // De-assert Device Reset
   m_pALIMMIOService->mmioWrite32(CSR_CTL, 1);

   // Set input workspace address
   m_pALIMMIOService->mmioWrite64(CSR_SRC_ADDR, CACHELINE_ALIGNED_ADDR(CoolPhys));

   // Set the number of cache lines for the test
   m_pALIMMIOService->mmioWrite32(CSR_NUM_LINES, CoolSize / CL(1));

   csr_type cfc_cfg = (csr_type)NLB_TEST_MODE_READ;

   // Select the read channel.
   if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_READ_VL0)){
      cfc_cfg |= (csr_type)NLB_TEST_MODE_READ_VL0;
   }
   else if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_READ_VH0)){
      cfc_cfg |= (csr_type)NLB_TEST_MODE_READ_VH0;
   }
   else if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_READ_VH1)){
      cfc_cfg |= (csr_type)NLB_TEST_MODE_READ_VH1;
   }
   else if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_READ_VR)){
      cfc_cfg |= (csr_type)NLB_TEST_MODE_READ_VR;
   }

   // Select the write channel.
   if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_WRITE_VL0)){
      cfc_cfg |= (csr_type)NLB_TEST_MODE_WRITE_VL0;
   }
   else if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_WRITE_VH0)){
      cfc_cfg |= (csr_type)NLB_TEST_MODE_WRITE_VH0;
   }
   else if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_WRITE_VH1)){
      cfc_cfg |= (csr_type)NLB_TEST_MODE_WRITE_VH1;
   }
   else if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_WRITE_VR)){
      cfc_cfg |= (csr_type)NLB_TEST_MODE_WRITE_VR;
   }

   // Set the test mode
   m_pALIMMIOService->mmioWrite32(CSR_CFG, 0);
   m_pALIMMIOService->mmioWrite32(CSR_CFG, cfc_cfg); // non-continuous mode

   // Start the test
   m_pALIMMIOService->mmioWrite32(CSR_CTL, 3);

   // Wait for test completion
   while ( 0 == pAFUDSM->test_complete &&
         ( MaxPoll >= 0 )) {
            MaxPoll -= 1;
            SleepMilli(1);
     }

   // Stop the device
   m_pALIMMIOService->mmioWrite32(CSR_CTL, 7);
   m_pALIMMIOService->mmioWrite32(CSR_CTL, 0);

   // Check the device status
   if ( MaxPoll < 0 ) {
     cerr << "The maximum timeout for test stop was exceeded." << endl;
     ++res;
   }
   if ( 0 != pAFUDSM->test_error ) {
      ++res;
   }

   return res;
}

void INLB::ReadPerfMonitors()
{
	NamedValueSet PerfMon;
	btUnsigned64bitInt     value;

	if (NULL != m_pALIPerf){
		m_pALIPerf->performanceCountersGet(&PerfMon);

		if (PerfMon.Has(AALPERF_VERSION)) {
		   PerfMon.Get( AALPERF_VERSION, &value);
		   m_PerfMonitors[VERSION] = value;
		}
		if (PerfMon.Has(AALPERF_READ_HIT)) {
		   PerfMon.Get( AALPERF_READ_HIT, &value);
		   m_PerfMonitors[READ_HIT] = value;
		}
		if (PerfMon.Has(AALPERF_WRITE_HIT)) {
		   PerfMon.Get( AALPERF_WRITE_HIT, &value);
		   m_PerfMonitors[WRITE_HIT] = value;
		}
		if (PerfMon.Has(AALPERF_READ_MISS)) {
		   PerfMon.Get( AALPERF_READ_MISS, &value);
		   m_PerfMonitors[READ_MISS] = value;
		}
		if (PerfMon.Has(AALPERF_WRITE_MISS)) {
		   PerfMon.Get( AALPERF_WRITE_MISS, &value);
		   m_PerfMonitors[WRITE_MISS] = value;
		}
		if (PerfMon.Has(AALPERF_EVICTIONS)) {
			PerfMon.Get( AALPERF_EVICTIONS, &value);
			m_PerfMonitors[EVICTIONS] = value;
		}
		if (PerfMon.Has(AALPERF_PCIE0_READ)) {
			PerfMon.Get( AALPERF_PCIE0_READ, &value);
			m_PerfMonitors[PCIE0_READ] = value;
		}
		if (PerfMon.Has(AALPERF_PCIE0_WRITE)) {
			PerfMon.Get( AALPERF_PCIE0_WRITE, &value);
			m_PerfMonitors[PCIE0_WRITE] = value;
		}
		if (PerfMon.Has(AALPERF_PCIE1_READ)) {
			PerfMon.Get( AALPERF_PCIE1_READ, &value);
			m_PerfMonitors[PCIE1_READ] = value;
		}
		if (PerfMon.Has(AALPERF_PCIE1_WRITE)) {
			PerfMon.Get( AALPERF_PCIE1_WRITE, &value);
			m_PerfMonitors[PCIE1_WRITE] = value;
		}
		if (PerfMon.Has(AALPERF_UPI_READ)) {
			PerfMon.Get( AALPERF_UPI_READ, &value);
			m_PerfMonitors[UPI_READ] = value;
		}
		if (PerfMon.Has(AALPERF_UPI_WRITE)) {
			PerfMon.Get( AALPERF_UPI_WRITE, &value);
			m_PerfMonitors[UPI_WRITE] = value;
		}
	}
}

void INLB::SavePerfMonitors()
{
   btCSRValue i;
   for ( i = 0 ; i < sizeof(m_PerfMonitors) / sizeof(m_PerfMonitors[0]) ; ++i ) {
      m_SavedPerfMonitors[i] = m_PerfMonitors[i];
   }
}

btUnsigned64bitInt INLB::GetPerfMonitor(btUnsignedInt i) const
{
   switch ( i ) {
      case VERSION 		: // FALL THROUGH
      case READ_HIT 	: // FALL THROUGH
      case WRITE_HIT 	: // FALL THROUGH
      case READ_MISS 	: // FALL THROUGH
      case WRITE_MISS 	: // FALL THROUGH
      case EVICTIONS 	: // FALL THROUGH
      case PCIE0_READ 	: // FALL THROUGH
      case PCIE0_WRITE  : // FALL THROUGH
      case PCIE1_READ 	: // FALL THROUGH
      case PCIE1_WRITE 	: // FALL THROUGH
      case UPI_READ 	: // FALL THROUGH
      case UPI_WRITE 	: return m_PerfMonitors[i] - m_SavedPerfMonitors[i];

      default : return 0;
   }
}

std::string INLB::CalcReadBandwidth(const NLBCmdLine &cmd)
{
   const btUnsigned32bitInt clockfreq = cmd.clkfreq;
   const double             giga      = 1000.0 * 1000.0 * 1000.0;

   nlb_vafu_dsm *pAFUDSM = (nlb_vafu_dsm *)m_pMyApp->DSMVirt();

   bt64bitCSR rawticks     = pAFUDSM->num_clocks;
   bt32bitCSR startpenalty = pAFUDSM->start_overhead;
   bt32bitCSR endpenalty   = pAFUDSM->end_overhead;
   bt32bitCSR rds          = pAFUDSM->num_reads;
   bt64bitCSR ticks;

   if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_CONT))// cont mode
   {
	   ticks = rawticks - startpenalty;
   }
   else //non-cont mode
   {
	   ticks = rawticks - (startpenalty + endpenalty);
   }
   const double Rds   = (double)rds;
   const double Ticks = (double)ticks;
   const double Hz    = (double)clockfreq;

   double bw = (Rds * (CL(1) * Hz)) / Ticks;

   bw /= giga;

   std::ostringstream oss;

   oss.precision(3);
   oss.setf(std::ios::fixed, std::ios::floatfield);
   oss << bw << " GB/s";

   return oss.str();
}

std::string INLB::CalcWriteBandwidth(const NLBCmdLine &cmd)
{
   const btUnsigned32bitInt clockfreq = cmd.clkfreq;
   const double             giga      = 1000.0 * 1000.0 * 1000.0;

   nlb_vafu_dsm *pAFUDSM = (nlb_vafu_dsm *)m_pMyApp->DSMVirt();

   bt64bitCSR rawticks     = pAFUDSM->num_clocks;
   bt32bitCSR startpenalty = pAFUDSM->start_overhead;
   bt32bitCSR endpenalty   = pAFUDSM->end_overhead;
   bt32bitCSR wrs          = pAFUDSM->num_writes;
   bt64bitCSR ticks;
   if ( flag_is_set(cmd.cmdflags, NLB_CMD_FLAG_CONT))// cont mode
   {
	   ticks = rawticks - startpenalty;
   }
   else //non-cont mode
   {
	   ticks = rawticks - (startpenalty + endpenalty);
   }

   const double Wrs   = (double)wrs;
   const double Ticks = (double)ticks;
   const double Hz    = (double)clockfreq;

   double bw = (Wrs * (CL(1) * Hz)) / Ticks;

   bw /= giga;

   std::ostringstream oss;

   oss.precision(3);
   oss.setf(std::ios::fixed, std::ios::floatfield);

   oss << bw << " GB/s";

   return oss.str();
}

std::string INLB::Normalized(const NLBCmdLine &cmd ) const throw()
{
	const btUnsigned32bitInt clockfreq = cmd.clkfreq;
	std::ostringstream oss;

   oss << std::dec;

   if ( clockfreq >= GHZ(1) ) {
      oss << (clockfreq / GHZ(1)) << " GHz";
      return oss.str();
   }
   if ( clockfreq >= MHZ(1) ) {
      oss << (clockfreq / MHZ(1)) << " MHz";
      return oss.str();
   }
   if ( clockfreq >= KHZ(1) ) {
      oss << (clockfreq / KHZ(1)) << " KHz";
      return oss.str();
   }
   oss << clockfreq << " Hz";
   return oss.str();

}

#if defined ( __AAL_WINDOWS__ )
# define strcasecmp _stricmp
#endif // __AAL_WINDOWS__


BEGIN_C_DECLS

int ParseCmds(struct NLBCmdLine *nlbcl, int argc, char *argv[])
{
   int    res;
   int    clean;
   aalclp clp;

   res = aalclp_init(&clp);
   if ( 0 != res ) {
      cerr << "aalclp_init() failed : " << res << ' ' << strerror(res) << endl;
      return res;
   }

   NLBSetupCmdLineParser(&clp, nlbcl);

   res = aalclp_add_gcs_compliance(&clp);
   if ( 0 != res ) {
      cerr << "aalclp_add_gcs_compliance() failed : " << res << ' ' << strerror(res) << endl;
      goto CLEANUP;
   }

   res = aalclp_scan_argv(&clp, argc, argv);
   if ( 0 != res ) {
      cerr << "aalclp_scan_argv() failed : " << res << ' ' << strerror(res) << endl;
   }

   if ( flag_is_set(nlbcl->cmdflags, NLB_CMD_PARSE_ERROR) ) {
         res = 99;
      }

CLEANUP:
   clean = aalclp_destroy(&clp);
   if ( 0 != clean ) {
      cerr << "aalclp_destroy() failed : " << clean << ' ' << strerror(clean) << endl;
   }

   return res;
}


END_C_DECLS


/**
@addtogroup ALIapp
@{

@verbatim
This application is for example purposes only.
It is not intended to represent a model for developing commercially-deployable applications.
It is designed to show working examples of the AAL programming model and APIs.@endverbatim

This Sample demonstrates the following:

<ul>
  <li>An IALIClient implementation.</li>
  <li>Use of the ISingleAFUApp class template.</li>
  <li>Runtime selection of AFU targets with ALIAFU.</li>
  <li>Native Loopback with IALIAFU.</li>
</ul>

This sample is designed to be used with ALIAFU.

1 Summary of Operation

ALIapp relies on its instantiation of ISingleAFUApp inherited by CMyApp to
perform the brunt of the XL runtime interaction. An XL Runtime object is declared
in main() to handle the runtime startup and shutdown.

An object instance of CMyApp is declared in main(), and the selection of AFU target
implementation is specified based on the command line parameters to ALIapp.

Some AFU-specific Runtime parameter configuration is performed prior to passing the
CMyApp instance to the Runtime's start routine. When the override of ISingleAFUApp::OnRuntimeStarted
is called, an instance of ALIAFU is requested.

The service allocation request is satisfied by ISingleAFUApp::serviceAllocated, which
pulses the internal semaphore, waking the thread blocked in main(). Once awake, some basic
error checking is done on the CMyApp instance and the Native Loopback test is performed.

When the Native Loopback test is complete, ISingleAFUApp::Stop is called to release the AFU and to
wait for the release notification from the runtime.

Finally, the Native Loopback test status is reported, and the application exits.

2 Running the application

2.0 Online Help

@code
$ ALIapp --help

Usage:
   ALIapp [--target=<TARGET>]

      <TARGET> = one of { fpga ase swsim }@endcode

2.1 ALI FPGA (HWALIAFU)

Prerequisites for running the sample with an FPGA:
<ul>
  <li>The ALI AAL device drivers must be loaded.</li>
  <li>The AAL Resource Manager must be running.</li>
  <li>The FPGA module connected to the system must be programmed with an appropriate ALI AFU bit stream.</li>
</ul>

@code
$ ALIapp --target=fpga@endcode

2.2 ALI AFU Simulation Environment (ASEALIAFU)

Prerequisites for running the sample with ASE:
<ul>
  <li>The ASE simulation-side application must be running on the system.</li>
</ul>

@code
$ ALIapp --target=ase@endcode

2.3 ALI Software Simulation (SWSimALIAFU)

Prerequisites for running the sample with Software Simulation:
<ul>
  <li>(none)
</ul>

@code
$ ALIapp --target=swsim@endcode

@}
*/

