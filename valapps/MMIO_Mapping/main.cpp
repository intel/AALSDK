// INTEL CONFIDENTIAL - For Intel Internal Use Only
//
// valapps/MMIO_Mapping/main.cpp
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
/// @brief Test Basic moving 1 64bit Cache Line data with NLB0 RTL.
/// @ingroup MMIO_Mapping
/// @verbatim
/// AAL MMIO mapping test application
///
///    This application is for tesing purposes only.
///    It is not intended to represent a model for developing commercially-
///       deployable applications.
///    It is designed to test MMIO functionality.
///
///
/// This Sample demonstrates how to use the basic ALI APIs.
///
/// This sample is designed to be used with the xyzALIAFU Service.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 05/09/2016     RP       Initial version started based on older sample
//****************************************************************************

#include "MMIOmapping.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////
///
///  Implementation
///
///////////////////////////////////////////////////////////////////////////////

/// @brief   Constructor registers this objects client interfaces and starts
///          the AAL Runtime. The member m_bisOK is used to indicate an error.
///
MMIOMapping::MMIOMapping() :
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
   m_OutputSize(0),
   m_TestNum(0)

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
MMIOMapping::~MMIOMapping()
{
   m_Sem.Destroy();
}

/// @brief _getALIMMIOService is called from run* functions to get ALIMMIO service
///        interface handler
///
///
btBool MMIOMapping::_getALIMMIOService()
{
   // Request the Service we are interested in.

   // NOTE: This example is bypassing the Resource Manager's configuration record lookup
   //  mechanism.  Since the Resource Manager Implementation is a sample, it is subject to change.
   //  This example does illustrate the utility of having different implementations of a service all
   //  readily available and bound at run-time.
   NamedValueSet Manifest;
   NamedValueSet ConfigRecord;
   btBool        _status = true;

#if defined( HWAFU )                /* Use FPGA hardware */
   // Service Library to use
   ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libHWALIAFU");

   // the AFUID to be passed to the Resource Manager. It will be used to locate the appropriate device.
   ConfigRecord.Add(keyRegAFU_ID,"949C47DE-DA1A-EEB8-3B56-1FBB2ADE456D");


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
   Manifest.Add(AAL_FACTORY_CREATE_SERVICENAME, "Test MMIO complete map");

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
      _status = false;
   }

   return _status;
}


/// @brief   run() is called from main performs the following:
///             - Allocate the appropriate ALI Service depending
///               on whether a hardware, ASE or software implementation is desired.
///             - Allocates the necessary buffers to be used by the NLB AFU algorithm
///             - Executes the NLB algorithm
///             - Cleans up.
///
btInt MMIOMapping::run()
{
   cout <<"==========================="<<endl;
   cout <<"= Test MMIO 1 CL Transfer ="<<endl;
   cout <<"==========================="<<endl;

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
   Manifest.Add(AAL_FACTORY_CREATE_SERVICENAME, "Test MMIO with 1 CL transaction");

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
   if( ali_errnumOK != m_pALIBufferService->bufferAllocate(LPBK1_BUFFER_SIZE, &m_InputVirt)){
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

   if( ali_errnumOK !=  m_pALIBufferService->bufferAllocate(LPBK1_BUFFER_SIZE, &m_OutputVirt)){
      m_bIsOK = false;
      m_Sem.Post(1);
      m_Result = -1;
      goto done_3;
   }

   m_OutputSize = LPBK1_BUFFER_SIZE;      // Start the test
   m_pALIMMIOService->mmioWrite32(CSR_CTL, 3);
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
      ::memset( m_InputVirt,  0, m_InputSize);     // Input initialized to 0
      ::memset( m_OutputVirt, 0, m_OutputSize);    // Output initialized to 0

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
   // Freed all three so now Release() the Service through the Services IAALService::Release() method
   (dynamic_ptr<IAALService>(iidService, m_pAALService))->Release(TransactionID());
   m_Sem.Wait();

done_0:
   m_Runtime.stop();
   m_Sem.Wait();

   return m_Result;
}

btInt MMIOMapping::runMMIOWholeRegion32()
{
    btUnsigned32bitInt iData32 = Data_4_Byte;
    btCSROffset offSet = MMIO_Start_Address;
    btBool status = false;
    btUnsigned32bitInt rData32 = 0x0;
    btCSROffset endOffSet = MMIO_SIZE;  // at end of MMIO region

	cout <<"=============================================================="<<endl;
	cout <<"= Test AFU MMIO whole region by read/write every memory cell ="<<endl;
	cout <<"=============================================================="<<endl;

	if (_getALIMMIOService() == false)
    {
        cout <<"service allocation failed "<<endl;
        goto done_0;
    }

    //=============================
    // Now we have the AFU RTL Service
    //   now we can use it
    //=============================
    MSG("Running Test");
    if(true == m_bIsOK){

        // Initiate AFU Reset
        m_pALIResetService->afuReset();

        // If ASE, give it some time to catch up
        #if defined (ASEAFU)
            SleepSec(2);
        #endif

        MSG("Done ASEAFU reset");

        //write data through 32 bit interface
        for (btUnsigned32bitInt j = 0; j< (MMIO_SIZE / 4); ++j) {
             status = m_pALIMMIOService->mmioWrite32(offSet, iData32);

             if (status == false)
             {
                cout <<" MMIO Region write/read Test failed !!!! " <<endl;
                goto done_0;
             }

             offSet = offSet + 4;
        };

        cout <<endl;

        // If ASE, give it some time to catch up
        #if defined (ASEAFU)
            SleepSec(10);
        #endif

        MSG("Done writing 32 bit data");

          // read data with 32 bit interface and compare the value
          offSet = MMIO_Start_Address;  //at the beginning of the MMIO region

          for (btUnsigned32bitInt j = 0; j<20; ++j) {
               m_pALIMMIOService->mmioRead32(offSet, &rData32);

               cout <<"0x"<< hex << offSet <<"  " << hex << rData32 << endl ;

               if (rData32 != Data_4_Byte)
               {
                   m_Result = -1;
                   cout << endl << "Test Failed !!!!!!!!"<<endl;
                   //goto done_0;
               }
               offSet = offSet + 4;
          }

          rData32 = 0x0;

          for (btUnsigned32bitInt j = 0; j< 20; ++j) {
               m_pALIMMIOService->mmioRead32(endOffSet, &rData32);
               cout << "End: 0x"<< hex << endOffSet << " "<< hex << rData32 << endl ;
               endOffSet = endOffSet - 4;
          }
    }

   MSG("MMIO Region Write / Read (32 bit interface) Test PASS!");

	done_0:
	   m_Runtime.stop();
	   m_Sem.Wait();

	   return m_Result;
}

btInt MMIOMapping::runMMIOWholeRegion64()
{
    btCSROffset offSet = MMIO_Start_Address;
    btBool status = false;
    btCSROffset endOffSet = 0x40000;  // at end of MMIO region

    cout <<"=============================================================="<<endl;
    cout <<"= Test AFU MMIO whole region by read/write every memory cell ="<<endl;
    cout <<"=============================================================="<<endl;

    if (_getALIMMIOService() == false)
    {
        cout <<"service allocation failed "<<endl;
        goto done_0;
    }

    //=============================
    // Now we have the AFU RTL Service
    //   now we can use it
    //=============================
    MSG("Running Test");
    if(true == m_bIsOK){

    // Initiate AFU Reset
    m_pALIResetService->afuReset();

    // If ASE, give it some time to catch up
    #if defined (ASEAFU)
        SleepSec(2);
    #endif

    MSG("Done ASEAFU reset");

    //write & read data through 64 bit interface in MMIO region
    btUnsigned64bitInt iData64 = Data_4_Byte;
    offSet = MMIO_Start_Address;

      for (btUnsigned32bitInt j = 0; j< (MMIO_SIZE / 8); ++j) {
             m_pALIMMIOService->mmioWrite64(offSet, iData64);
             offSet = offSet + 8;
      };

      MSG("Done writing 64 bit data");

      // Initiate AFU Reset
      m_pALIResetService->afuReset();

      // If ASE, give it some time to catch up
      #if defined (ASEAFU)
          SleepSec(2);
      #endif

      MSG("Done ASEAFU reset");

      // read data with 64 bit interface and compare the value

      btUnsigned64bitInt oData64 = 0x0;
      offSet = MMIO_Start_Address;  //at the beginning of the MMIO region

      for (btUnsigned32bitInt j = 0; j<20; ++j) {
           m_pALIMMIOService->mmioRead64(offSet, &oData64);

           cout <<"0x"<< hex << offSet <<"  " << hex << oData64 << endl ;

           if (oData64 != Data_4_Byte)
           {
               m_Result = -1;
               cout << endl << "Test Failed !!!!!!!!"<<endl;
               //goto done_0;
           }
           offSet = offSet + 8;
      }

      oData64 = 0x0;
      endOffSet = 0x40000;  // at end of MMIO region

      for (btUnsigned32bitInt j = 0; j< 20; ++j) {
           m_pALIMMIOService->mmioRead64(endOffSet, &oData64);
           cout << "End: 0x"<< hex << endOffSet << " "<< hex << oData64 << endl ;
           endOffSet = endOffSet - 8;
      }
   }

   MSG("MMIO Region Write / Read Test (64 bit interface) PASS!");

    done_0:
       m_Runtime.stop();
       m_Sem.Wait();

       return m_Result;
}


btInt MMIOMapping::runInvalidMMIORegion()
{
    btUnsigned32bitInt iData32 = Data_4_Byte;
    btCSROffset offSet = MMIO_SIZE;
    btBool status = false;

    cout <<"=============================================================="<<endl;
    cout <<"= Test AFU MMIO whole region by read/write every memory cell ="<<endl;
    cout <<"=============================================================="<<endl;

    if (_getALIMMIOService() == false)
    {
        cout <<"service allocation failed "<<endl;
        goto done_0;
    }

    //=============================
    // Now we have the AFU RTL Service
    //   now we can use it
    //=============================
    MSG("Running Test");
    if(true == m_bIsOK){
        //write data to out off boundary MMIo region through 32 bit interface
        for (btUnsigned32bitInt j = 0; j< 2; ++j) {
             status = m_pALIMMIOService->mmioWrite32(offSet, iData32);
             if (status == true)
             {
                 MSG("MMIO out off boundary condition Test FAIL !");
                 m_Result = -1;
             }
             else
             {
                 MSG("MMIO out off boundary condition Test PASS !");
                 goto done_0;
             }

             offSet = offSet + 4;
        };
    }

       // Clean-up and return
    done_0:
       m_Runtime.stop();
       m_Sem.Wait();

       return m_Result;
}

btInt MMIOMapping::runMMIOLength()
{
    MSG("runMMIOWholeRegion");

    cout <<"=============================================================="<<endl;
    cout <<"= Get MMIO length and verify with HW supported MMIO size ="<<endl;
    cout <<"=============================================================="<<endl;


    if (_getALIMMIOService() == false)
    {
        cout <<"service allocation failed "<<endl;
        goto done_0;
    }

   //=============================
   // Now we have the AFU RTL Service
   //   now we can use it
   //=============================
   MSG("Running Test");
   if(true == m_bIsOK){
       //get MMIO size
       btCSROffset afuMMIOSize = 0x0;

       afuMMIOSize = m_pALIMMIOService->mmioGetLength();


       if (afuMMIOSize != SAS_HW_MMIO_SIZE){
           cout << "HW MMIO Size  is not: 0x " << hex << afuMMIOSize << endl;
           m_Result = -1;
       }
   }

   MSG("MMIOLength Test PASS !");

done_0:
   m_Runtime.stop();
   m_Sem.Wait();

   return m_Result;
}
//=================
//  IServiceClient
//=================

// <begin IServiceClient interface>
void MMIOMapping::serviceAllocated(IBase *pServiceBase,
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

void MMIOMapping::serviceAllocateFailed(const IEvent &rEvent)
{
   ERR("Failed to allocate Service");
    PrintExceptionDescription(rEvent);
   ++m_Result;                     // Remember the error
   m_bIsOK = false;

   m_Sem.Post(1);
}

 void MMIOMapping::serviceReleased(TransactionID const &rTranID)
{
    MSG("Service Released");
   // Unblock Main()
   m_Sem.Post(1);
}

 void MMIOMapping::serviceReleaseRequest(IBase *pServiceBase, const IEvent &rEvent)
 {
    MSG("Service unexpected requested back");
    if(NULL != m_pAALService){
       IAALService *pIAALService = dynamic_ptr<IAALService>(iidService, m_pAALService);
       ASSERT(pIAALService);
       pIAALService->Release(TransactionID());
    }
 }


 void MMIOMapping::serviceReleaseFailed(const IEvent        &rEvent)
 {
    ERR("Failed to release a Service");
    PrintExceptionDescription(rEvent);
    m_bIsOK = false;
    m_Sem.Post(1);
 }


 void MMIOMapping::serviceEvent(const IEvent &rEvent)
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

 void MMIOMapping::runtimeStarted( IRuntime            *pRuntime,
                                      const NamedValueSet &rConfigParms)
 {
    m_bIsOK = true;
    m_Sem.Post(1);
 }

 void MMIOMapping::runtimeStopped(IRuntime *pRuntime)
  {
     MSG("Runtime stopped");
     m_bIsOK = false;
     m_Sem.Post(1);
  }

 void MMIOMapping::runtimeStartFailed(const IEvent &rEvent)
 {
    ERR("Runtime start failed");
    PrintExceptionDescription(rEvent);
 }

 void MMIOMapping::runtimeStopFailed(const IEvent &rEvent)
 {
     MSG("Runtime stop failed");
     m_bIsOK = false;
     m_Sem.Post(1);
 }

 void MMIOMapping::runtimeAllocateServiceFailed( IEvent const &rEvent)
 {
    ERR("Runtime AllocateService failed");
    PrintExceptionDescription(rEvent);
 }

 void MMIOMapping::runtimeAllocateServiceSucceeded(IBase *pClient,
                                                     TransactionID const &rTranID)
 {
     MSG("Runtime Allocate Service Succeeded");
 }

 void MMIOMapping::runtimeEvent(const IEvent &rEvent)
 {
     MSG("Generic message handler (runtime)");
 }
 // <begin IRuntimeClient interface>

/// @} group MMIOMapping


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
   btInt Result = 0;

   MMIOMapping theApp;

   if(!theApp.isOK()){
      ERR("Runtime Failed to Start");
      exit(1);
   }

   if (argc < 2)
   {
	   std::cout << "Usage: MMIO_Mapping x \n";
	   std::cout << "x: 0 - MMIO 1 CL read/write \n";
       std::cout << "x: 1 - MMIO whole region read/write (32 bit API)\n";
       std::cout << "x: 2 - MMIO whole region read/write (64 bit API)\n";
       std::cout << "x: 3 - Access invalid memory region and expect seg fault \n";
       std::cout << "x: 4 - get AFU MMIO Length\n";
   }
   else
   {
	   switch(*argv[1]){
	      case '0' :
	    	  std::cout <<"MMIO 1 CL read/write \n";
	    	  Result = theApp.run();
	    	  break;
	      case '1' :
	    	  std::cout <<"MMIO whole region read/write \n";
	    	  Result = theApp.runMMIOWholeRegion32();
	    	  break;
	      case '2' :
              std::cout <<"MMIO whole region read/write \n";
              Result = theApp.runMMIOWholeRegion64();
              break;
	      case '3' :
	    	  std::cout <<"Access invalid memory region and expect seg fault";
	    	  Result = theApp.runInvalidMMIORegion();
	    	  break;
	      case '4' :
              std::cout <<"get AFU MMIO Length";
              Result = theApp.runMMIOLength();
              break;
	      default:
	    	  std::cout << "not valid test selection number !";
	   }
   }
   MSG("Done");
   return Result;
}



