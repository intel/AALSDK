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
#include "arguments.h"

using namespace std;

/// Device Feature Header CSR
struct AFU_DFH {

   union {
      btUnsigned64bitInt csr;
      struct {
         btUnsigned64bitInt Feature_ID :12;     // Feature ID
         btUnsigned64bitInt Feature_rev :4;     // Feature revision
         btUnsigned64bitInt next_DFH_offset :24;// Next Device Feature header offset
         btUnsigned16bitInt eol :1;             // end of header bit
         btUnsigned64bitInt rsvd :19;           // Reserved
         btUnsigned64bitInt Type :4;            // Type of Device

      }; //end struct
   }; // end union

}; //end struct AFU_DFH

string        asePlatformStr = "ASE";
string        hwPlatformStr = "HW";
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
   m_strPlatform(NULL)
{
   // Register our Client side interfaces so that the Service can acquire them.
   //   SetInterface() is inherited from CAASBase
   SetInterface(iidServiceClient, dynamic_cast<IServiceClient *>(this));
   SetInterface(iidRuntimeClient, dynamic_cast<IRuntimeClient *>(this));

   // Initialize our internal semaphore
   m_Sem.Create(0, 1);

   // Start the AAL Runtime, setting any startup options via a NamedValueSet

   // Using Hardware Services requires the Remote Resource Manager Broker Service
   // Note that this could also be accomplished by setting the environment variable
   // AALRUNTIME_CONFIG_BROKER_SERVICE to librrmbroker
   NamedValueSet configArgs;
   NamedValueSet configRecord;

   m_strPlatform = (btString)"HW";  //set platform to HW by default

   if (hwPlatformStr.compare(m_strPlatform) == 0)
   {
       // Specify that the remote resource manager is to be used.
       configRecord.Add(AALRUNTIME_CONFIG_BROKER_SERVICE, "librrmbroker");
       configArgs.Add(AALRUNTIME_CONFIG_RECORD, &configRecord);
   }

   // Start the Runtime and wait for the callback by sitting on the semaphore.
   // the runtimeStarted() or runtimeStartFailed() callbacks should set m_bIsOK appropriately.
   if(!m_Runtime.start(configArgs)){
	   m_bIsOK = false;
      return;
   }
   m_Sem.Wait();
   m_bIsOK = true;
}

MMIOMapping::MMIOMapping(btString strPlatform) :
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
   m_strPlatform(strPlatform)
{
   // Register our Client side interfaces so that the Service can acquire them.
   //   SetInterface() is inherited from CAASBase
   SetInterface(iidServiceClient, dynamic_cast<IServiceClient *>(this));
   SetInterface(iidRuntimeClient, dynamic_cast<IRuntimeClient *>(this));

   // Initialize our internal semaphore
   m_Sem.Create(0, 1);

   // Start the AAL Runtime, setting any startup options via a NamedValueSet

   // Using Hardware Services requires the Remote Resource Manager Broker Service
   // Note that this could also be accomplished by setting the environment variable
   // AALRUNTIME_CONFIG_BROKER_SERVICE to librrmbroker
   NamedValueSet configArgs;
   NamedValueSet configRecord;

   if (hwPlatformStr.compare(m_strPlatform) == 0)
   {
       // Specify that the remote resource manager is to be used.
       configRecord.Add(AALRUNTIME_CONFIG_BROKER_SERVICE, "librrmbroker");
       configArgs.Add(AALRUNTIME_CONFIG_RECORD, &configRecord);
   }

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
MMIOMapping::~MMIOMapping()
{
   m_Sem.Destroy();
}

/// @brief _getALIMMIOService is called from run* functions to get ALIMMIO service
///        interface handler
///
///
btBool MMIOMapping::_getALIMMIOService(const arguments &args)
{
   // Request the Service we are interested in.

   // NOTE: This example is bypassing the Resource Manager's configuration record lookup
   //  mechanism.  Since the Resource Manager Implementation is a sample, it is subject to change.
   //  This example does illustrate the utility of having different implementations of a service all
   //  readily available and bound at run-time.
   NamedValueSet Manifest;
   NamedValueSet ConfigRecord;
   btBool        _status = true;

   if (asePlatformStr.compare(m_strPlatform) == 0)
   {
	   // Use ASE based RTL simulation
	   Manifest.Add(keyRegHandle, 20);

	   Manifest.Add(ALIAFU_NVS_KEY_TARGET, ali_afu_ase);

	   ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libALI");
	   ConfigRecord.Add(AAL_FACTORY_CREATE_SOFTWARE_SERVICE,true);
   }
   else
   {
	   // Service Library to use
	   ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libALI");

	   // the AFUID to be passed to the Resource Manager. It will be used to locate the appropriate device.
      if (static_cast<btUnsigned32bitInt>(args.get_long("test")) == 0)
      {
	      ConfigRecord.Add(keyRegAFU_ID,"D8424DC4-A4A3-C413-F89E-433683F9040B");
	   }
      else
      {
         ConfigRecord.Add(keyRegAFU_ID,"10C1BFF1-88D1-4DFB-96BF-6F5FC4038FAC");
      }
   }

   if (args.have("bus")){
       ConfigRecord.Add(keyRegBusNumber, static_cast<btUnsigned32bitInt>(args.get_long("bus")));
   }

   if (args.have("device")){
       ConfigRecord.Add(keyRegDeviceNumber, static_cast<btUnsigned32bitInt>(args.get_long("device")));
   }

   if (args.have("function")){
       ConfigRecord.Add(keyRegFunctionNumber, static_cast<btUnsigned32bitInt>(args.get_long("function")));
   }

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

/// @brief _VerifyFeatureType finds feature type in known DFH;
///
///

btBool MMIOMapping::_verifyFeatureType(btUnsigned32bitInt featureType, btUnsigned32bitInt featureID)
{
    btVirtAddr         _pfeatureAddress;
    NamedValueSet      _afuReqst;
    NamedValueSet      _afuOutput;
    btUnsigned64bitInt _fType;
    btUnsigned64bitInt _fID;
    btBool             _status = false;
    struct AFU_DFH     *_dfh;

    //Feature type = ALI_DFH_TYPE_AFU
    _afuReqst.Add(ALI_GETFEATURE_TYPE_KEY, static_cast<ALI_GETFEATURE_TYPE_DATATYPE> (featureType));

    if (m_pALIMMIOService->mmioGetFeatureAddress(&_pfeatureAddress, _afuReqst, _afuOutput) == false)
    {
      cout <<"Fail to get feature address"<<endl;
    }

    if ((_afuOutput.Get(ALI_GETFEATURE_TYPE_KEY, &_fType)!= 0) &&
            (_afuOutput.Get(ALI_GETFEATURE_ID_KEY, &_fID) != 0))
    {
       cout <<"Feature Type & ID are not found in DFH"<<endl;
    }
    else
    {
       //verify output feature type & ID
       if ((_fType == featureType) && (_fID == featureID))
       {
           _status = true;
       }
       else
       {
           cout <<"Test Failed : Wrong feature Type & ID!!!"<<endl;
       }
    }

    //parse DFH header based on feature Address
    //_dfh = &_pfeatureAddress;
    //cout <<"Output Type :"<<hex<<_dfh->Type <<"  OutPut ID : "<<hex<<_dfh->Feature_ID <<endl;
    //cout <<"Input Type :"<<hex<<featureType <<"  Input ID : "<<hex<<featureID<<endl;

    return _status;

}
/// @brief create DFH list
///
///
btInt MMIOMapping::initDFHList()
{
    btUnsigned64bitInt iData64 = 0;
    btBool retStatus = false;

    MSG("initialize DFH list");

    //Next AFU
    iData64=0x48;
    retStatus = m_pALIMMIOService->mmioWrite64(0x18, iData64);
    if (retStatus != true)
    {
        MSG("mmioWrite64 fail !!!!");
        goto done;
    }

    //Reserve
    iData64=0x0;
    retStatus = m_pALIMMIOService->mmioWrite64(0x20, iData64);
    if (retStatus != true)
    {
        MSG("mmioWrite64 fail !!!!");
        goto done;
    }

    //1st private feature header in the list
    iData64=0x30010000001010BB;
    retStatus = m_pALIMMIOService->mmioWrite64(0x28, iData64);
    if (retStatus != true)
    {
        MSG("mmioWrite64 fail !!!!");
        goto done;
    }

    //Private feature CSR
    iData64=0x1234123412341234;
    retStatus = m_pALIMMIOService->mmioWrite64(0x30, iData64);
    if (retStatus != true)
    {
        MSG("mmioWrite64 fail !!!!");
        goto done;
    }

    //2nd private feature Header
    iData64=iData64=0x2001010000101012;
    retStatus = m_pALIMMIOService->mmioWrite64(0x38, iData64);
    if (retStatus != true)
    {
        MSG("mmioWrite64 fail !!!!");
        goto done;
    }

    //Private feature CSR - BBB_ID_L
    iData64=0x0000000099224466;
    retStatus = m_pALIMMIOService->mmioWrite64(0x40, iData64);
    if (retStatus != true)
    {
        MSG("mmioWrite64 fail !!!!");
        goto done;
    }

    //Private feature CSR - BBB_ID_H
    iData64=0x1133557700000000;
    retStatus = m_pALIMMIOService->mmioWrite64(0x48, iData64);
    if (retStatus != true)
    {
        MSG("mmioWrite64 fail !!!!");
        goto done;
    }

done:
    MSG("DFH is initialized");
    if (retStatus == true)
    {
        return 0;
    }
    else
    {
        return -1;
    }

}

/// @brief initialize corrupted DFH list
///
///
btInt MMIOMapping::initCorruptedDFHList()
{
    btUnsigned64bitInt iData64 = 0;
    btBool retStatus = false;

    MSG("initialize Corrupted DFH list");

    //Next AFU
    iData64=0x48;
    retStatus = m_pALIMMIOService->mmioWrite64(0x18, iData64);
    if (retStatus != true)
    {
        MSG("mmioWrite64 fail !!!!");
        goto done;
    }

    //Reserve
    iData64=0x0;
    retStatus = m_pALIMMIOService->mmioWrite64(0x20, iData64);
    if (retStatus != true)
    {
        MSG("mmioWrite64 fail !!!!");
        goto done;
    }

    //1st private feature header in the list
    iData64=0x30010000001010BB;
    retStatus = m_pALIMMIOService->mmioWrite64(0x28, iData64);
    if (retStatus != true)
    {
        MSG("mmioWrite64 fail !!!!");
        goto done;
    }

    //Private feature CSR
    iData64=0x1234123412341234;
    retStatus = m_pALIMMIOService->mmioWrite64(0x30, iData64);
    if (retStatus != true)
    {
        MSG("mmioWrite64 fail !!!!");
        goto done;
    }

    //2nd private feature Header
    iData64=iData64=0x3002010000101012;
    retStatus = m_pALIMMIOService->mmioWrite64(0x38, iData64);
    if (retStatus != true)
    {
        MSG("mmioWrite64 fail !!!!");
        goto done;
    }

    //Private feature CSR - BBB_ID_L
    iData64=0x0000000099224466;
    retStatus = m_pALIMMIOService->mmioWrite64(0x40, iData64);
    if (retStatus != true)
    {
        MSG("mmioWrite64 fail !!!!");
        goto done;
    }

    //Private feature CSR - BBB_ID_H
    iData64=0x1133557700000000;
    retStatus = m_pALIMMIOService->mmioWrite64(0x48, iData64);
    if (retStatus != true)
    {
        MSG("mmioWrite64 fail !!!!");
        goto done;
    }

done:
    MSG("DFH is initialized");
    if (retStatus == true)
    {
        return 0;
    }
    else
    {
        return -1;
    }

}

/// @brief   run() is called from main performs the following:
///             - Allocate the appropriate ALI Service depending
///               on whether a hardware, ASE or software implementation is desired.
///             - Allocates the necessary buffers to be used by the NLB AFU algorithm
///             - Executes the Test RTL function
///             - Cleans up.
///
btInt MMIOMapping::run(btString strPlatform, btInt testNum, const arguments& args)
{
   //save platform information
   m_strPlatform = strPlatform;

   if (_getALIMMIOService(args) == false)
   {
	   cout <<"service allocation failed "<<endl;
	   m_bIsOK = false;
	   m_Result = -1;
	   goto done_0;
   }

   switch(testNum){
	  case 0 :
		  std::cout <<"MMIO 1 CL read/write "<<endl;
		  m_Result = testMMIOWriteOneCacheLine();
		  break;
	  case 1 :
		  std::cout <<"MMIO whole region read/write (32 bit API)" <<endl;
		  m_Result = MMIOWholeRegion32();
		  break;
	  case 2 :
		  std::cout <<"MMIO whole region read/write (64 bit API) "<<endl;
		  m_Result = MMIOWholeRegion64();
		  break;
	  case 3 :
		  std::cout <<"Access invalid memory region and expect no access "<<endl;
		  m_Result = InvalidMMIORegion();
		  break;
	  case 4 :
		  std::cout <<"get AFU MMIO Length" <<endl;
		  m_Result =MMIOLength();
		  break;
	  case 5 :
		  std::cout <<"Discover DFH Header" <<endl;
		  m_Result = DFHFeature();
		  break;
	  case 6 :
		  std::cout <<"Discover DFH Header with malfunction header" <<endl;
		  m_Result = CorruptDFH();
		  break;
	  case 7 :
		  std::cout <<"MMIO Stress Test" <<endl;
		  m_Result = MMIOStress();
			break;
	  case 98:
		  //can't combine this step with test because AASResourcemanager need to be started
		  std::cout <<"Initialize DFH header" <<endl;
		  m_Result = initDFHList();
		  break;
	  case 99:
	      //can't combine this step with test because AASResourcemanager need to be started
	      std::cout <<"Initialize DFH header" <<endl;
	      m_Result = initCorruptedDFHList();
	      break;
      default:
	      std::cout<<"Please type: ./MMIO_Mapping  to display help menu ! "<<endl;
   }

done_0:
   // Freed all Service through the Services IAALService::Release() method
   if (m_pAALService)
   {
      (dynamic_ptr<IAALService>(iidService, m_pAALService))->Release(TransactionID());
       m_Sem.Wait();
   }

   m_Runtime.stop();
   m_Sem.Wait();

   return m_Result;
}

//write 1 cache line to MMIO
btInt MMIOMapping::testMMIOWriteOneCacheLine()
{
   cout <<"==========================="<<endl;
   cout <<"= Test MMIO 1 CL Transfer ="<<endl;
   cout <<"==========================="<<endl;

   btInt _result = 0;

   // Now that we have the Service and have saved the IALIBuffer interface pointer
   // we can now Allocate the 3 Workspaces used by the NLB algorithm. The buffer allocate
   // function is synchronous so no need to wait on the semaphore

   // Device Status Memory (DSM) is a structure defined by the NLB implementation.

   // User Virtual address of the pointer is returned directly in the function
   if( ali_errnumOK != m_pALIBufferService->bufferAllocate(LPBK1_DSM_SIZE, &m_DSMVirt)){
      m_bIsOK = false;
      _result = -1;
      goto done_1;
   }

   // Save the size and get the IOVA from the User Virtual address. The HW only uses IOVA.
   m_DSMSize = LPBK1_DSM_SIZE;
   m_DSMPhys = m_pALIBufferService->bufferGetIOVA(m_DSMVirt);

   if(0 == m_DSMPhys){
      m_bIsOK = false;
      _result = -1;
      goto done_2;
   }

   // Repeat for the Input and Output Buffers
   if( ali_errnumOK != m_pALIBufferService->bufferAllocate(LPBK1_BUFFER_SIZE, &m_InputVirt)){
      m_bIsOK = false;
      m_Sem.Post(1);
      _result = -1;
      goto done_2;
   }

   m_InputSize = LPBK1_BUFFER_SIZE;
   m_InputPhys = m_pALIBufferService->bufferGetIOVA(m_InputVirt);
   if(0 == m_InputPhys){
      m_bIsOK = false;
      _result = -1;
      goto done_3;
   }

   if( ali_errnumOK !=  m_pALIBufferService->bufferAllocate(LPBK1_BUFFER_SIZE, &m_OutputVirt)){
      m_bIsOK = false;
      m_Sem.Post(1);
      _result = -1;
      goto done_3;
   }

   m_OutputSize = LPBK1_BUFFER_SIZE;      // Start the test
   m_OutputPhys = m_pALIBufferService->bufferGetIOVA(m_OutputVirt);
   if(0 == m_OutputPhys){
      m_bIsOK = false;
      _result = -1;
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
      ::memset( m_InputVirt,  0xAF, m_InputSize);     // Input initialized to AFter
      ::memset( m_OutputVirt, 0xBE, m_OutputSize);    // Output initialized to BEfore

      struct CacheLine {                           // Operate on cache lines
         btUnsigned32bitInt uint[16];
      };
      struct CacheLine *pCL = reinterpret_cast<struct CacheLine *>(m_InputVirt);
      //for ( btUnsigned32bitInt i = 0; i < m_InputSize / CL(1) ; ++i ) {
         pCL[0].uint[15] = 0xA;
      //};                         // Cache-Line[n] is zero except last uint = n

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
      m_pALIMMIOService->mmioWrite32(CSR_CFG,0x200); //Rdline_I

      MSG("Running test with Rdline_I by default.");

      volatile bt32bitCSR *StatusAddr = (volatile bt32bitCSR *)
                                         (m_DSMVirt  + DSM_STATUS_TEST_COMPLETE);
      // Start the test
      m_pALIMMIOService->mmioWrite32(CSR_CTL, 3);

      // Wait for test completion
      while( 0 == ((*StatusAddr)&0x1) ) {
         SleepMicro(10);
      }
      MSG("Done Running Test");

      // Stop the device
      m_pALIMMIOService->mmioWrite32(CSR_CTL, 7);

      // Check that output buffer now contains what was in input buffer, e.g. 0xAF
      if (int err = memcmp( m_OutputVirt, m_InputVirt, m_OutputSize)) {
         ERR("Output does NOT Match input, at offset " << err << "!");
         ++_result;
      } else {
         MSG("Output matches Input!");
      }
   }

   if (_result == 0)
   {
       MSG("Test Pass !!!");
   }
   else
   {
       MSG("Test Fail !!!");
   }
       // Clean-up and return
    done_4:
       m_pALIBufferService->bufferFree(m_OutputVirt);
    done_3:
       m_pALIBufferService->bufferFree(m_InputVirt);
    done_2:
       m_pALIBufferService->bufferFree(m_DSMVirt);

    done_1:
       return _result;
}

btInt MMIOMapping::MMIOWholeRegion32()
{
    btUnsigned32bitInt iData32 = Data_4_Byte;
	btCSROffset offSet = MMIO_Start_Address;
	btBool status = false;
	btUnsigned32bitInt rData32 = 0x0;
	btCSROffset endOffSet = MMIO_SIZE;  // at end of MMIO region
	btInt  _result = 0;

	cout <<"=============================================================="<<endl;
	cout <<"= Test AFU MMIO whole region by read/write every memory cell ="<<endl;
	cout <<"=============================================================="<<endl;

	//=============================
	// Now we have the AFU RTL Service
	//   now we can use it
	//=============================
	MSG("Running Test");
	if(true == m_bIsOK){

		// Initiate AFU Reset
		m_pALIResetService->afuReset();

		// If ASE, give it some time to catch up
		if (asePlatformStr.compare(m_strPlatform) == 0)
		{
			SleepSec(2);
		}

		MSG("Done ASEAFU reset");

		//write data through 32 bit interface
		for (btUnsigned32bitInt j = 0; j< (MMIO_SIZE / 4); ++j) {
			 status = m_pALIMMIOService->mmioWrite32(offSet, iData32);

			 if (status == false)
			 {
				cout <<" MMIO Region write/read Test failed !!!! " <<endl;
				_result = -1;
				goto done_0;
			 }

			 offSet = offSet + 4;
          iData32 += 1;
		};

		if (asePlatformStr.compare(m_strPlatform) == 0)
		{
			SleepSec(5);
		}

		MSG("Done writing 32 bit data");

		// read data with 32 bit interface and compare the value
		offSet = MMIO_Start_Address;  //at the beginning of the MMIO region
      iData32 = Data_4_Byte;        //re-initialize the value for verification
		for (btUnsigned32bitInt j = 0; j<20; ++j) {
			   m_pALIMMIOService->mmioRead32(offSet, &rData32);

			   if (rData32 != iData32)
			   {
				   _result = -1;
				   cout << endl << "Test Failed !!!!!!!!"<<endl;
				   goto done_0;
			   }
			   offSet = offSet + 4;
            iData32 += 1;
		}

	}

    MSG("MMIO Region Write / Read (32 bit interface) Test PASS!");

done_0:
   return _result;
}

btInt MMIOMapping::MMIOWholeRegion64()
{
    btCSROffset offSet = MMIO_Start_Address;
    btBool status = false;
    btCSROffset endOffSet = 0x40000;  // at end of MMIO region
    btInt  _result = 0;

    cout <<"=============================================================="<<endl;
    cout <<"= Test AFU MMIO whole region by read/write every memory cell ="<<endl;
    cout <<"=============================================================="<<endl;

    //=============================
    // Now we have the AFU RTL Service
    //   now we can use it
    //=============================
    MSG("Running Test");
    if(true == m_bIsOK){

        // Initiate AFU Reset
        m_pALIResetService->afuReset();

        // If ASE, give it some time to catch up
        if (asePlatformStr.compare(m_strPlatform) == 0)
        {
            SleepSec(2);
        }

        MSG("Done ASEAFU reset");

        //write & read data through 64 bit interface in MMIO region
        btUnsigned64bitInt iData64 = Data_8_Byte;
        offSet = MMIO_Start_Address;

        for (btUnsigned32bitInt j = 0; j< (MMIO_SIZE / 8); ++j) {
            m_pALIMMIOService->mmioWrite64(offSet, iData64);
            offSet = offSet + 8;
            iData64 += 1;
        }

        MSG("Done writing 64 bit data");

        // read data with 64 bit interface and compare the value
        btUnsigned64bitInt oData64 = 0x0;
        offSet = MMIO_Start_Address;  //at the beginning of the MMIO region
        iData64 = Data_8_Byte;
        for (btUnsigned32bitInt j = 0; j<20; ++j) {
             m_pALIMMIOService->mmioRead64(offSet, &oData64);

             if (oData64 != iData64)
             {
                 _result = -1;
                 cout << endl << "Test Failed !!!!!!!!"<<endl;
                 goto done_0;
             }
             offSet = offSet + 8;
             iData64 += 1;
        }
   }

   MSG("MMIO Region Write / Read Test (64 bit interface) PASS!");

   done_0:
       return _result;
}

btInt MMIOMapping::InvalidMMIORegion()
{
    btUnsigned32bitInt iData32 = Data_4_Byte;
    btCSROffset offSet = MMIO_SIZE;
    btBool status = false;
    btInt  _result = 0;

    cout <<"=============================================================="<<endl;
    cout <<"= Test Invalid AFU MMIO boundary condition                   ="<<endl;
    cout <<"=============================================================="<<endl;

    //=============================
    // Now we have the AFU RTL Service
    //   now we can use it
    //=============================
    MSG("Running Test");
    if(true == m_bIsOK){
        //write data to out off boundary MMIO region through 32 bit interface
        for (btUnsigned32bitInt j = 0; j< 2; ++j) {
             status = m_pALIMMIOService->mmioWrite32(offSet, iData32);
             if (status == true)
             {
                 MSG("MMIO out off boundary condition Test FAIL !");
                 _result = -1;
             }
             else
             {
                 MSG("MMIO out off boundary condition Test PASS !");
                 _result = 0;
                 goto done_0;
             }

             offSet = offSet + 4;
        };
    }

       // Clean-up and return
    done_0:
       return _result;
}

btInt MMIOMapping::MMIOLength()
{
    btInt  _result = 0;

    cout <<"=============================================================="<<endl;
    cout <<"= Get MMIO length and verify with HW supported MMIO size ="<<endl;
    cout <<"=============================================================="<<endl;

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
           _result = -1;
       }
   }

   if (_result == 0){

       MSG("MMIOLength Test PASS !");
   }
   else{

       MSG("MMIOLength Test FAIL !");
   }

done_0:
   return _result;
}


btInt MMIOMapping::DFHFeature()
{
    btCSROffset offSet = MMIO_Start_Address;
    ALI_GETFEATURE_ID_DATATYPE featureID = 0x12;
    btUnsigned64bitInt oData64 = 0x0;
    btInt  _result = 0;

    //discovery of DFH features using mmioGetFeatureOffset()
    btCSROffset       _featureOffSet;
    btBool            _retVal = false;
    NamedValueSet     _afuReqst;

    cout <<"=============================================================="<<endl;
    cout <<"= Discover DFH features using mmioGetFeaturesOffset()        ="<<endl;
    cout <<"=============================================================="<<endl;

   //=============================
   // Now we have the AFU RTL Service
   //   now we can use it
   //=============================
   MSG("Running Test");

   if(true == m_bIsOK){

       // Initiate AFU Reset
       m_pALIResetService->afuReset();

       // If ASE, give it some time to catch up
       if (asePlatformStr.compare(m_strPlatform) == 0)
       {
            SleepSec(2);
       }

       MSG("Done ASEAFU reset");

       for (btUnsigned64bitInt j = 0; j<8; ++j) {
         m_pALIMMIOService->mmioRead64(offSet, &oData64);

         cout <<"0x"<< hex << offSet <<"  " << hex << oData64 << endl ;

         offSet = offSet + 8;
       }

       _afuReqst.Add(ALI_GETFEATURE_ID_KEY, featureID);

       _retVal = m_pALIMMIOService->mmioGetFeatureOffset(&_featureOffSet, _afuReqst);

       //check if featureOffset from the API is from the correct location in DFH
       if ((_retVal == false) || (_featureOffSet != TargetFeatureOffSet))
       {
           cout << "Feature OffSet 0x:" << hex << _featureOffSet <<endl;
           cout <<"feature ID 0x:"<<hex <<featureID << " is not found !!!!" <<endl;
           MSG("DFH Feature discovery Test FAIL !");
           _result = -1;
           goto done_0;
       }

       cout << "Feature OffSet 0x:" << hex << _featureOffSet <<endl;

   }

   MSG("DFH Feature discovery Test PASS !");

done_0:
   return _result;
}

/// @brief use bad DFH header to trigger error path in
///        mmioGetFeatureOffset API by using feature type
///        as the files and e
///
///
btInt MMIOMapping::CorruptDFH()
{
    btCSROffset offSet = MMIO_Start_Address;
    ALI_GETFEATURE_TYPE_DATATYPE featureType = ALI_DFH_TYPE_BBB;
    btUnsigned64bitInt oData64 = 0x0;

    //discovery of DFH features offset using mmioGetFeatureOffset()
    btCSROffset       _featureOffSet;
    btBool            _retVal = false;
    NamedValueSet     _afuReqst;
    btInt             _result = 0;

    cout <<"=============================================================="<<endl;
    cout <<"= Discover DFH features offset using mmioGetFeaturesOffset() ="<<endl;
    cout <<"=============================================================="<<endl;

   //=============================
   // Now we have the AFU RTL Service
   //   now we can use it
   //=============================
   MSG("Running Test");
   if(true == m_bIsOK){

       // Initiate AFU Reset
       m_pALIResetService->afuReset();

       // If ASE, give it some time to catch up
       if (asePlatformStr.compare(m_strPlatform) == 0)
       {
           SleepSec(2);
       }

       MSG("Done ASEAFU reset");

       for (btUnsigned64bitInt j = 0; j<13; ++j) {
         m_pALIMMIOService->mmioRead64(offSet, &oData64);

         cout <<"0x"<< hex << offSet <<"  " << hex << oData64 << endl ;

         offSet = offSet + 8;
       }

       _afuReqst.Add(ALI_GETFEATURE_TYPE_KEY, featureType);

       _retVal = m_pALIMMIOService->mmioGetFeatureOffset(&_featureOffSet, _afuReqst);

       //check if featureOffset from the API is from the correct location in DFH
       if (_retVal == true)
       {
           cout << "Feature OffSet 0x:" << hex << _featureOffSet <<endl;
           cout <<"feature Type 0x:"<<hex <<featureType << " is found !!!!" <<endl;
           MSG("Corrupt DFH Feature discovery Test FAIL !");
           _result = -1;
           goto done_0;
       }

       cout << "Corrupt DFH Feature OffSet 0x:" << hex << _featureOffSet <<endl;

   }

   MSG("Corrupt DFH Feature discovery Test PASS !");

done_0:
   return _result;
}

/// @brief:MMIO stress test by using mmioGetFeatureAddress()
///        to parse the AFU DHF
///

btInt MMIOMapping::MMIOStress()
{
    btCSROffset        _offSet = MMIO_Start_Address;
    btCSROffset        _featureOffSet;
   // ALI_GETFEATURE_TYPE_DATATYPE _featureType = ALI_DFH_TYPE_AFU;
    btUnsigned64bitInt _oData64 = 0x0;
    btVirtAddr         _mmioAfuBaseAddr;
    btUnsigned64bitInt *_pFeature_ID;
    btUnsigned64bitInt featureType;
    btUnsigned32bitInt typeArry[] = {ALI_DFH_TYPE_AFU, \
                                    ALI_DFH_TYPE_BBB,  \
                                    ALI_DFH_TYPE_PRIVATE};
    btUnsigned32bitInt IDrry[] = {0xA, 0x12, 0xBB};

    btVirtAddr        _pfeatureAddress;
    NamedValueSet     _afuReqst;
    NamedValueSet     _afuOutput;
    btInt             _result = 0;


    cout <<"=============================================================="<<endl;
    cout <<"= Stress test using mmioGetFeaturesOffset() ="<<endl;
    cout <<"=============================================================="<<endl;


   //=============================
   // Now we have the AFU RTL Service
   //   now we can use it
   //=============================
   MSG("Running Test");
   if(true == m_bIsOK){

       // Initiate AFU Reset
       m_pALIResetService->afuReset();

       // If ASE, give it some time to catch up
       if (asePlatformStr.compare(m_strPlatform) == 0)
       {
           SleepSec(2);
       }

       MSG("Done ASEAFU reset");

       for (btUnsigned64bitInt j = 0; j<13; ++j) {
         m_pALIMMIOService->mmioRead64(_offSet, &_oData64);

         cout <<"0x"<< hex << _offSet <<"  "<< hex << _oData64 << endl ;

         _offSet = _offSet + 8;
       }

       for (int i = 0; i< 3; i++)
       {
           if(_verifyFeatureType(typeArry[i], IDrry[i]) == false)
           {
               cout <<"Feature type not found !!!"<<endl;
               MSG("MMIO stress Test FAIL !!!!");
               _result = -1;
               goto done_0;
           }
       }

   }

   MSG("MMIO stress Test PASS !");

done_0:
   return _result;
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

 void usage()
 {
	   std::cout << "Usage: MMIO_Mapping x     ---> use HW platform by default"<<endl;
	   std::cout << "Usage: MMIO_Mapping x ASE ---> use ASE SIM platform"<<endl;
	   std::cout << "x: 0 - MMIO 1 CL read/write "<<endl;
	   std::cout << "x: 1 - MMIO whole region read/write (32 bit API)"<<endl;
	   std::cout << "x: 2 - MMIO whole region read/write (64 bit API)"<<endl;
	   std::cout << "x: 3 - Access invalid memory region and expect no access"<<endl;
	   std::cout << "x: 4 - get AFU MMIO Length"<<endl;
	   std::cout << "x: 5 - discover device feature offset"<<endl;
	   std::cout << "x: 6 - trigger error path when searching by feature type" <<endl;
	   std::cout << "x: 7 - MMIO stress test"<<endl;
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
   arguments argparse;
   btInt Result = 0;

   argparse("bus",      'b', optional_argument, "pci bus number")
           ("feature",  'f', optional_argument, "pci feature number")
           ("device",   'd', optional_argument, "pci device number")
           ("test",     't', required_argument, "test number to run")
           ("platform", 'p', optional_argument, "platform (HW?)");
   if (!argparse.parse(argc, argv))
   {
       usage();
       return -1;
   }
  
   std::string  strPlatform = argparse.get_string("platform", "HW");  //use HW as default platform
   std::cout << strPlatform <<endl;
  
   btInt testNumber = argparse.get_long("test");
   std::cout<<"test number: "<<testNumber<<endl;

   MMIOMapping theApp((btString)strPlatform.c_str());

   if(!theApp.isOK()){
      ERR("Runtime Failed to Start");
      exit(1);
   }

   Result = theApp.run((btString)strPlatform.c_str(), testNumber, argparse);

done_1:
    return Result;
}



