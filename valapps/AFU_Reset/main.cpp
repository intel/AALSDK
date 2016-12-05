// INTEL CONFIDENTIAL - For Intel Internal Use Only
//
// valapps/AFU_Reset/main.cpp
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
/// 09/06/2016     AJ       Replaced simple_afu with standard NLB0 for reset test
//****************************************************************************

#include "AFU_Reset.h"
#include "arguments.h"

#define NLB_CSR_SCRATCHPAD 0x0104

using namespace std;

///////////////////////////////////////////////////////////////////////////////
///
///  Implementation
///
///////////////////////////////////////////////////////////////////////////////

/// @brief   Constructor registers this objects client interfaces and starts
///          the AAL Runtime. The member m_bisOK is used to indicate an error.
///
AFU_Reset::AFU_Reset() :
    m_Runtime(this),
    m_pAALService(NULL),
    m_pALIBufferService(NULL),
    m_pALIMMIOService(NULL),
    m_pALIResetService(NULL),
    m_Result(0),
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
AFU_Reset::~AFU_Reset()
{
    m_Sem.Destroy();
}

/// @brief _getALIMMIOService is called from run* functions to get ALIMMIO service
///        interface handler
///
///
btBool AFU_Reset::_getALIMMIOService(const arguments& args)
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
    ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libALI");

    // the AFUID to be passed to the Resource Manager. It will be used to locate the appropriate device.
    ConfigRecord.Add(keyRegAFU_ID,"D8424DC4-A4A3-C413-F89E-433683F9040B");
    if (args.have("bus")){
        ConfigRecord.Add(keyRegBusNumber, static_cast<btUnsigned32bitInt>(args.get_long("bus")));
    }

    if (args.have("device")){
        ConfigRecord.Add(keyRegDeviceNumber, static_cast<btUnsigned32bitInt>(args.get_long("device")));
    }

    if (args.have("function")){
        ConfigRecord.Add(keyRegFunctionNumber, static_cast<btUnsigned32bitInt>(args.get_long("function")));
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

/// @brief   run(arguments& args) is called from main performs the following:
///             - Allocate the appropriate ALI Service depending
///               on whether a hardware, ASE or software implementation is desired.
///             - Allocates the necessary buffers to be used by the NLB AFU algorithm
///             - Executes the Test RTL function
///             - Cleans up.
///
btInt AFU_Reset::run(const arguments& args)
{
    btUnsigned32bitInt x;
    cout <<"==========================="<<endl;
    cout <<"= Testing reset on NLB0 ="<<endl;
    cout <<"==========================="<<endl;

    if (_getALIMMIOService(args) == false)
    {
        cout <<"service allocation failed "<<endl;
        m_bIsOK = false;
        m_Result = -1;
        goto done_0;
    }

    //=============================
    // Now we have the NLB Service
    //   now we can use it
    //=============================
    MSG("Running Test");
    if(true == m_bIsOK)
    {

        // Initiate AFU Reset
        m_pALIResetService->afuReset();

        // Write the value on scratchpad region
        x = 0x00001000;
        printf("Write x = 0x%08x\n", x);
        m_pALIMMIOService->mmioWrite32(CSR_SCRATCHPAD, (btUnsigned32bitInt)x);
        SleepMicro(1000);

        // Read value in x
        m_pALIMMIOService->mmioRead32(CSR_SCRATCHPAD, &x);
        printf("Read x = 0x%08x\n", x);

        // Initiate AFU Reset
        m_pALIResetService->afuReset();

        // Read value in x should be 0
        m_pALIMMIOService->mmioRead32(0x104, &x);
        printf("Read x = 0x%08x\n", x);
        if ( x != 0 )
        {
            cerr<<"AFU reset signal not working."<<endl;
        }
        else
        {
            cout<<"Success. AFU reset signal seems to be working."<<endl;
        }
        m_Result = x;
        MSG("Done Running Test");

    }

    if (m_Result == 0)
    {
        MSG("Test Pass !!!");
    }
    else
    {
        MSG("Test Fail !!!");
    }
    // Clean-up and return

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
void AFU_Reset::serviceAllocated(IBase *pServiceBase,
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

void AFU_Reset::serviceAllocateFailed(const IEvent &rEvent)
{
    ERR("Failed to allocate Service");
    PrintExceptionDescription(rEvent);
    ++m_Result;                     // Remember the error
    m_bIsOK = false;

    m_Sem.Post(1);
}

void AFU_Reset::serviceReleased(TransactionID const &rTranID)
{
    MSG("Service Released");
    // Unblock Main()
    m_Sem.Post(1);
}

void AFU_Reset::serviceReleaseRequest(IBase *pServiceBase, const IEvent &rEvent)
{
    MSG("Service unexpected requested back");
    if(NULL != m_pAALService){
        IAALService *pIAALService = dynamic_ptr<IAALService>(iidService, m_pAALService);
        ASSERT(pIAALService);
        pIAALService->Release(TransactionID());
    }
}


void AFU_Reset::serviceReleaseFailed(const IEvent        &rEvent)
{
    ERR("Failed to release a Service");
    PrintExceptionDescription(rEvent);
    m_bIsOK = false;
    m_Sem.Post(1);
}


void AFU_Reset::serviceEvent(const IEvent &rEvent)
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

void AFU_Reset::runtimeStarted( IRuntime            *pRuntime,
                                const NamedValueSet &rConfigParms)
{
    m_bIsOK = true;
    m_Sem.Post(1);
}

void AFU_Reset::runtimeStopped(IRuntime *pRuntime)
{
    MSG("Runtime stopped");
    m_bIsOK = false;
    m_Sem.Post(1);
}

void AFU_Reset::runtimeStartFailed(const IEvent &rEvent)
{
    ERR("Runtime start failed");
    PrintExceptionDescription(rEvent);
}

void AFU_Reset::runtimeStopFailed(const IEvent &rEvent)
{
    MSG("Runtime stop failed");
    m_bIsOK = false;
    m_Sem.Post(1);
}

void AFU_Reset::runtimeAllocateServiceFailed( IEvent const &rEvent)
{
    ERR("Runtime AllocateService failed");
    PrintExceptionDescription(rEvent);
}

void AFU_Reset::runtimeAllocateServiceSucceeded(IBase *pClient,
                                                TransactionID const &rTranID)
{
    MSG("Runtime Allocate Service Succeeded");
}

void AFU_Reset::runtimeEvent(const IEvent &rEvent)
{
    MSG("Generic message handler (runtime)");
}
// <begin IRuntimeClient interface>

/// @} group AFU_Reset


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
    argparse("bus", 'b', optional_argument, "pci bus number")
            ("feature", 'f', optional_argument, "pci feature number")
            ("device", 'd', optional_argument, "pci device number");
    if (!argparse.parse(argc, argv)) return -1;

    btInt Result = 0;

    AFU_Reset theApp;

    if(!theApp.isOK()){
        ERR("Runtime Failed to Start");
        exit(1);
    }

    // Init the AAL logger.
    //pAALLogger()->AddToMask(LM_All, 8); // All subsystems
    //pAALLogger()->SetDestination(ILogger::CERR);

    std::cout <<"MMIO 1 CL read/write "<<endl;
    Result = theApp.run(argparse);

    MSG("Done.");
    return Result;
}
