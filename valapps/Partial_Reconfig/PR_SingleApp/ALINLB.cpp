// INTEL CONFIDENTIAL - For Intel Internal Use Only
//
// valapps/Partial_Reconfig/PR_SingleApp/ALINLB.cpp
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
/// @file ALINLB.cpp
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

#include "../PR_SingleApp/ALINLB.h"
using namespace std;
using namespace AAL;


// Convenience macros for printing messages and errors.
#ifdef MSG
# undef MSG
#endif // MSG
//#define MSG(x) std::cout << __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() : " << x << std::endl
#define MSG(x)
#ifdef ERR
# undef ERR
#endif // ERR
#define ERR(x) std::cerr << __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() **Error : " << x << std::endl


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
#define NLB_TEST_MODE_PCIE0      0x2000

AllocatesNLBService::AllocatesNLBService(const arguments &args) :
   m_pRuntime(NULL),
   m_pNLBService(NULL),
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
   m_OutputSize(0),
   m_ReleaseService(true),
   m_args(args)
{
   SetInterface(iidServiceClient, dynamic_cast<IServiceClient *>(this));

   m_Sem.Create(0, 1);
   m_bIsOK = true;
}

AllocatesNLBService::~AllocatesNLBService()
{
   m_Sem.Destroy();
}

///
btInt AllocatesNLBService::run()
{

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



      // Set input workspace address
      m_pALIMMIOService->mmioWrite64(CSR_SRC_ADDR, CACHELINE_ALIGNED_ADDR(m_InputPhys));

      // Set output workspace address
      m_pALIMMIOService->mmioWrite64(CSR_DST_ADDR, CACHELINE_ALIGNED_ADDR(m_OutputPhys));

      // Set the number of cache lines for the test
      m_pALIMMIOService->mmioWrite32(CSR_NUM_LINES, LPBK1_BUFFER_SIZE / CL(1));

      // Set the test mode
      m_pALIMMIOService->mmioWrite32(CSR_CFG,0);

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
   return m_Result;
}

//=================
//  IServiceClient
//=================

// <begin IServiceClient interface>
void AllocatesNLBService::serviceAllocated(IBase *pServiceBase,
                                      TransactionID const &rTranID)
{
   m_pNLBService = pServiceBase;
   ASSERT(NULL != m_pNLBService);
   if ( NULL == m_pNLBService ) {
      m_bIsOK = false;
      return;
   }

   m_pALIBufferService = dynamic_ptr<IALIBuffer>(iidALI_BUFF_Service, pServiceBase);
   ASSERT(NULL != m_pALIBufferService);
   if ( NULL == m_pALIBufferService ) {
      m_bIsOK = false;
      return;
   }


   m_pALIMMIOService = dynamic_ptr<IALIMMIO>(iidALI_MMIO_Service, pServiceBase);
   ASSERT(NULL != m_pALIMMIOService);
   if ( NULL == m_pALIMMIOService ) {
      m_bIsOK = false;
      return;
   }

   m_pALIResetService = dynamic_ptr<IALIReset>(iidALI_RSET_Service, pServiceBase);
   ASSERT(NULL != m_pALIResetService);
   if ( NULL == m_pALIResetService ) {
      m_bIsOK = false;
      return;
   }

   MSG("Service Allocated");
   m_Sem.Post(1);
}

void AllocatesNLBService::serviceAllocateFailed(const IEvent &rEvent)
{
   ERR("Failed to allocate Service");
    PrintExceptionDescription(rEvent);
   ++m_Result;                     // Remember the error
   m_bIsOK = false;

   m_Sem.Post(1);
}

void AllocatesNLBService::serviceReleased(TransactionID const &rTranID)
{
    MSG("Service Released");
   // Unblock Main()
   m_Sem.Post(1);
}

void AllocatesNLBService::serviceReleaseRequest(IBase *pServiceBase, const IEvent &rEvent)
{
   MSG("Service unexpected requested back -> serviceReleaseRequest");

   if(m_ReleaseService == false) {
      MSG("Not Release Service");
      return;
   }

   MSG("Release Service");
   if(NULL != m_pNLBService){
      IAALService *pIAALService = dynamic_ptr<IAALService>(iidService, m_pNLBService);
      ASSERT(pIAALService);
      pIAALService->Release(TransactionID());
   }
}


void AllocatesNLBService::serviceReleaseFailed(const IEvent        &rEvent)
{
   ERR("Failed to release a Service");
   PrintExceptionDescription(rEvent);
   m_bIsOK = false;
   m_Sem.Post(1);
}


void AllocatesNLBService::serviceEvent(const IEvent &rEvent)
{
   ERR("unexpected event 0x" << hex << rEvent.SubClassID());

}
// <end IServiceClient interface>

btBool AllocatesNLBService::AllocateNLBService(Runtime *pRuntime)
{
   NamedValueSet Manifest;
   NamedValueSet ConfigRecord;

   ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libALI");
   ConfigRecord.Add(keyRegAFU_ID, "D8424DC4-A4A3-C413-F89E-433683F9040B");

   if (m_args.have("bus")){
       ConfigRecord.Add(keyRegBusNumber, static_cast<btUnsigned32bitInt>(m_args.get_long("bus")));
   }

   if (m_args.have("device")){
       ConfigRecord.Add(keyRegDeviceNumber, static_cast<btUnsigned32bitInt>(m_args.get_long("device")));
   }

   if (m_args.have("function")){
       ConfigRecord.Add(keyRegFunctionNumber, static_cast<btUnsigned32bitInt>(m_args.get_long("function")));
   }

   // Add the Config Record to the Manifest describing what we want to allocate
   Manifest.Add(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &ConfigRecord);

   // in future, everything could be figured out by just giving the service name
   Manifest.Add(AAL_FACTORY_CREATE_SERVICENAME, "Hello ALI NLB");

   m_pRuntime =pRuntime;

   m_pRuntime->allocService(dynamic_cast<IBase *>(this), Manifest);
   m_Sem.Wait();
   if(!m_bIsOK){
      ERR("Allocation failed\n");
      return false;
   }
   return true;
}

btBool AllocatesNLBService::FreeNLBService()
{
   //  MSG("Release Service");
   (dynamic_ptr<IAALService>(iidService, m_pNLBService))->Release(TransactionID());
   m_Sem.Wait();
   return true;
}


btInt AllocatesNLBService::runInLoop()
{
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

      // Initiate AFU Reset
      m_pALIResetService->afuReset();


      // Initiate DSM Reset
      // Set DSM base, high then low
      m_pALIMMIOService->mmioWrite64(CSR_AFU_DSM_BASEL, m_DSMPhys);

      // Assert AFU reset
      m_pALIMMIOService->mmioWrite32(CSR_CTL, 0);

      //De-Assert AFU reset
      m_pALIMMIOService->mmioWrite32(CSR_CTL, 1);

      // Set input workspace address
      m_pALIMMIOService->mmioWrite64(CSR_SRC_ADDR, CACHELINE_ALIGNED_ADDR(m_InputPhys));

      // Set output workspace address
      m_pALIMMIOService->mmioWrite64(CSR_DST_ADDR, CACHELINE_ALIGNED_ADDR(m_OutputPhys));

      // Set the number of cache lines for the test
      m_pALIMMIOService->mmioWrite32(CSR_NUM_LINES, LPBK1_BUFFER_SIZE / CL(1));

      // Set the test mode
      m_pALIMMIOService->mmioWrite32(CSR_CFG,0);

      volatile bt32bitCSR *StatusAddr = (volatile bt32bitCSR *)
                                    (   m_DSMVirt  + DSM_STATUS_TEST_COMPLETE);
      // Start the test
      m_pALIMMIOService->mmioWrite32(CSR_CTL, 3);

      while(true)
      { }
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

return m_Result;
}

void  AllocatesNLBService::NLBThread(OSLThread *pThread, void *pContext)
{
   MSG("helloNLBThread ");
   AllocatesNLBService *hellonlb = static_cast<AllocatesNLBService *>(pContext);
   hellonlb->runInLoop();
}

