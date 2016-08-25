///////////////////////////////////////////////////////////////////////////////
///
///  Constructor and destructor
///
///////////////////////////////////////////////////////////////////////////////

#include <appconstants.h>
#include <appbase.h>

/// @brief   Constructor registers this objects client interfaces and starts
///          the AAL Runtime. The member m_bisOK is used to indicate an appERRor.
///
appbase::appbase () :
    m_AppType("HW"),
    m_AFUId("D8424DC4-A4A3-C413-F89E-433683F9040B"), // initialize the value using default AFU ID
    m_RequestBuffers(false),
    m_AppName("Base Application"),
    m_Runtime(this),
    m_pAALService(NULL),
    m_pALIBufferService(NULL),
    m_pALIMMIOService(NULL),
    m_pALIResetService(NULL),
    m_Result(0),
    m_Status(0),
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
    runtimeStart();
}

appbase::appbase(const string& appType,
                 const string& AFUId,
                 const string& appName) :
    m_AppType(appType),
    m_AFUId(AFUId),
    m_RequestBuffers(false),
    m_AppName(appName),
    m_Runtime(this),
    m_pAALService(NULL),
    m_pALIBufferService(NULL),
    m_pALIMMIOService(NULL),
    m_pALIResetService(NULL),
    m_Result(0),
    m_Status(0),
    m_DSMVirt(NULL),
    m_DSMPhys(0),
    m_DSMSize(LPBK1_DSM_SIZE),
    m_InputVirt(NULL),
    m_InputPhys(0),
    m_InputSize(LPBK1_BUFFER_SIZE),
    m_OutputVirt(NULL),
    m_OutputPhys(0),
    m_OutputSize(LPBK1_BUFFER_SIZE)
{
    runtimeStart();
}

appbase::appbase(const string& appType,
                 const string& AFUId,
                 const string& appName,
                 btWSSize dsmSize,
                 btWSSize inputSize,
                 btWSSize outputSize) :
m_AppType(appType),
    m_AFUId(AFUId),
    m_RequestBuffers(true),
    m_AppName(appName),
    m_Runtime(this),
    m_pAALService(NULL),
    m_pALIBufferService(NULL),
    m_pALIMMIOService(NULL),
    m_pALIResetService(NULL),
    m_Result(0),
    m_Status(0),
    m_DSMVirt(NULL),
    m_DSMPhys(0),
    m_DSMSize(dsmSize),
    m_InputVirt(NULL),
    m_InputPhys(0),
    m_InputSize(inputSize),
    m_OutputVirt(NULL),
    m_OutputPhys(0),
    m_OutputSize(outputSize)
{
    runtimeStart();
}

btBool appbase::runtimeStart()
{

    // Register our Client side interfaces so that the Service can acquire them.
    // SetInterface() is inherited from CAASBase
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

    if (m_AppType == "HW") {
        // Specify that the remote resource manager is to be used.
        configRecord.Add(AALRUNTIME_CONFIG_BROKER_SERVICE, "librrmbroker");
        configArgs.Add(AALRUNTIME_CONFIG_RECORD, &configRecord);
    }

    // Start the Runtime and wait for the callback by sitting on the semaphore.
    // the runtimeStarted() or runtimeStartFailed() callbacks should set m_bIsOK appropriately.
    if(!m_Runtime.start(configArgs)){
        m_bIsOK = false;
        return m_bIsOK;
    }
    m_Sem.Wait();
    m_bIsOK = true;

    string label = "= Hello " + m_AppName + " =";
    cout << string(label.length(), '=') << endl;
    cout <<label<<endl;
    cout << string(label.length(), '=') << endl;

    return m_bIsOK;
}

/// @brief   Destructor
///
appbase::~appbase()
{
    m_Sem.Destroy();
}

///////////////////////////////////////////////////////////////////////////////
///
///  Implementation
///
///////////////////////////////////////////////////////////////////////////////

/// @brief   allocService() is called from run to perform the following:
///             - Allocate the appropriate ALI Service depending
///               on whether a hardware, ASE or software implementation is desired.
btInt appbase::requestService()
{

if (m_AppType == "HW") {               /* Use FPGA hardware */
    // Service Library to use
    ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libHWALIAFU");

    // the AFUID to be passed to the Resource Manager. It will be used to locate the appropriate device.
    ConfigRecord.Add(keyRegAFU_ID, m_AFUId.c_str());
    //ConfigRecord.Add(keyRegBusNumber, (btUnsigned32bitInt)0xbe);

    // indicate that this service needs to allocate an AIAService, too to talk to the HW
    ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_AIA_NAME, "libaia");
} else if (m_AppType == "ASE") {
    Manifest.Add(keyRegHandle, 20);

    ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libASEALIAFU");
    ConfigRecord.Add(AAL_FACTORY_CREATE_SOFTWARE_SERVICE, true);
} else if (m_AppType == "SIM") {
    ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libSWSimALIAFU");
    ConfigRecord.Add(AAL_FACTORY_CREATE_SOFTWARE_SERVICE, true);
} else {
    return -1;
}

    // Add the Config Record to the Manifest describing what we want to allocate
    Manifest.Add(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &ConfigRecord);

    // in future, everything could be figured out by just giving the service name
    Manifest.Add(AAL_FACTORY_CREATE_SERVICENAME, m_AppName.c_str());
    appMSG("Allocating Service");

    // Allocate the Service and wait for it to complete by sitting on the
    // semaphore. The serviceAllocated() callback will be called if successful.
    // If allocation fails the serviceAllocateFailed() should set m_bIsOK appropriately.
    // (Refer to the serviceAllocated() callback to see how the Service's interfaces
    // are collected.)
    m_Runtime.allocService(dynamic_cast<IBase *>(this), Manifest);
    m_Sem.Wait();
    if(!m_bIsOK){
        appERR("Allocation failed\n");
        m_Result = -1;
        m_Status = NO_SERVICE_ALLOCATE;
        return m_Status;
    }

    return m_Status;

}

/**
 * Enable buffer request
 *
 * @param value Enables object to request buffers
 */
void appbase::setRequestBuffers(btBool value)
{
    m_RequestBuffers = value;
}

/**
 * Allocates virtual memory buffer
 *
 * @param reqSize      requested memory size
 * @param virtAddress  obtained virtual address
 * @param physAddress  obtained physical address (required by the FPGA)
 *
 * @return
 */
btInt appbase::allocateBuffer(btWSSize& reqSize,
                              btVirtAddr& virtAddress,
                              btPhysAddr& physAddress)
{

    if (( true == m_RequestBuffers ) && ( m_Status == APP_IS_OK ) && ( reqSize > 0 )) {
        // Now that we have the service, we can allocate the MMIOs that we are using
        // Buffer allocate is synchronous, no need to wait on semaphore
        if (ali_errnumOK != m_pALIBufferService->bufferAllocate(reqSize, &virtAddress)) {
            m_bIsOK = false;
            m_Result = -1;
            m_Status = NO_BUFFER_ALLOCATE;
            appERR("Cannot allocate buffer.");
            return m_Status;
        }

        // Save the size and get the IOVA from the User Virtual address. The HW needs the IOVA
        physAddress = m_pALIBufferService->bufferGetIOVA(virtAddress);
        if (0 == physAddress) {
            m_bIsOK = false;
            m_Result = -1;
            m_Status = NO_BUFFER_GETVA;
            appERR("Cannot get VA for the buffer.");
            return m_Status;
        }

        // Clear the DSM
        ::memset(virtAddress, 0, reqSize);
    }

    return m_Status;

}

btInt appbase::requestBuffers()
{

    m_Status = allocateBuffer(m_DSMSize, m_OutputVirt, m_OutputPhys);
    m_Status = allocateBuffer(m_InputSize, m_InputVirt, m_InputPhys);
    m_Status = allocateBuffer(m_OutputSize, m_OutputVirt, m_OutputPhys);

    return m_Status;

}

btInt appbase::requestBuffers(btWSSize dsmSize, btWSSize inputSize, btWSSize outputSize)
{

    m_RequestBuffers = true;
    m_DSMSize = dsmSize;
    m_InputSize = inputSize;
    m_OutputSize = outputSize;

    requestBuffers();
}

btInt appbase::releaseBuffers()
{
    if (true == m_RequestBuffers && m_DSMPhys != 0) {
        m_pALIBufferService->bufferFree(m_DSMVirt);
        m_DSMPhys = 0;
    }

    if (true == m_RequestBuffers && m_InputPhys != 0) {
        m_pALIBufferService->bufferFree(m_InputVirt);
        m_InputPhys = 0;
    }

    if (true == m_RequestBuffers && m_OutputPhys != 0) {
        m_pALIBufferService->bufferFree(m_OutputVirt);
        m_OutputPhys = 0;
    }
    return m_Status;
}

btInt appbase::requestExit()
{

    switch(m_Status) {
        case APP_IS_OK:
            appMSG("Cleaning application.");

        case NO_BUFFER_GETVA:
            releaseBuffers();

        case NO_BUFFER_ALLOCATE:
            // Freed all three so now Release() the Service through the Services IAALService::Release() method
            (dynamic_ptr<IAALService>(iidService, m_pAALService))->Release(TransactionID());
            m_Sem.Wait();

        case NO_SERVICE_ALLOCATE:
            m_Runtime.stop();
            m_Sem.Wait();
            break;

        default:
            appERR("Unsupported application status.");
    }

    return m_Status;

}

//=================
//  IServiceClient
//=================

// <begin IServiceClient interface>
void appbase::serviceAllocated(IBase *pServiceBase,
                               TransactionID const &rTranID)
{
// Save the IBase for the Service. Through it we can get any other
// interface implemented by the Service
    m_pAALService = pServiceBase;
    ASSERT(NULL != m_pAALService);
    if ( NULL == m_pAALService ) {
        m_bIsOK = false;
        return;
    }

// Documentation says HWALIAFU Service publishes
// IALIBuffer as subclass interface. Used in Buffer Allocation and Free
    m_pALIBufferService = dynamic_ptr<IALIBuffer>(iidALI_BUFF_Service, pServiceBase);
    ASSERT(NULL != m_pALIBufferService);
    if ( NULL == m_pALIBufferService ) {
        m_bIsOK = false;
        return;
    }

// Documentation says HWALIAFU Service publishes
// IALIMMIO as subclass interface. Used to set/get MMIO Region
    m_pALIMMIOService = dynamic_ptr<IALIMMIO>(iidALI_MMIO_Service, pServiceBase);
    ASSERT(NULL != m_pALIMMIOService);
    if ( NULL == m_pALIMMIOService ) {
        m_bIsOK = false;
        return;
    }

// Documentation says HWALIAFU Service publishes
// IALIReset as subclass interface. Used for resetting the AFU
    m_pALIResetService = dynamic_ptr<IALIReset>(iidALI_RSET_Service, pServiceBase);
    ASSERT(NULL != m_pALIResetService);
    if ( NULL == m_pALIResetService ) {
        m_bIsOK = false;
        return;
    }

    appMSG("Service Allocated");
    m_Sem.Post(1);
}

void appbase::serviceAllocateFailed(const IEvent &rEvent)
{
    appERR("Failed to allocate Service");
    PrintExceptionDescription(rEvent);
    ++m_Result;                     // Remember the appERRor
    m_bIsOK = false;

    m_Sem.Post(1);
}

void appbase::serviceReleased(TransactionID const &rTranID)
{
    appMSG("Service Released");
// Unblock Main()
    m_Sem.Post(1);
}

void appbase::serviceReleaseRequest(IBase *pServiceBase, const IEvent &rEvent)
{
    appMSG("Service unexpected requested back");
    if(NULL != m_pAALService){
        IAALService *pIAALService = dynamic_ptr<IAALService>(iidService, m_pAALService);
        ASSERT(pIAALService);
        pIAALService->Release(TransactionID());
    }
}

void appbase::serviceReleaseFailed(const IEvent        &rEvent)
{
    appERR("Failed to release a Service");
    PrintExceptionDescription(rEvent);
    m_bIsOK = false;
    m_Sem.Post(1);
}


void appbase::serviceEvent(const IEvent &rEvent)
{
    // appERR("unexpected event 0x" << hex << rEvent.SubClassID());
    // The state machine may or may not stop here. It depends upon what happened.
    // A fatal appERRor implies no more messages and so none of the other Post()
    // will wake up.
    // OTOH, a notification message will simply print and continue.
}
// <end IServiceClient interface>


//=================
//  IRuntimeClient
//=================

// <begin IRuntimeClient interface>
// Because this simple example has one object implementing both IRuntieCLient and IServiceClient
// some of these interfaces are redundant. We use the IServiceClient in such cases and ignore
// the RuntimeClient equivalent e.g.,. runtimeAllocateServiceSucceeded()

void appbase::runtimeStarted(IRuntime *pRuntime,
                             const NamedValueSet &rConfigParms)
{
    m_bIsOK = true;
    m_Sem.Post(1);
}

void appbase::runtimeStopped(IRuntime *pRuntime)
{
    appMSG("Runtime stopped");
    m_bIsOK = false;
    m_Sem.Post(1);
}

void appbase::runtimeStartFailed(const IEvent &rEvent)
{
    appERR("Runtime start failed");
    PrintExceptionDescription(rEvent);
}

void appbase::runtimeStopFailed(const IEvent &rEvent)
{
    appMSG("Runtime stop failed");
    m_bIsOK = false;
    m_Sem.Post(1);
}

void appbase::runtimeAllocateServiceFailed( IEvent const &rEvent)
{
    appERR("Runtime AllocateService failed");
    PrintExceptionDescription(rEvent);
}

void appbase::runtimeAllocateServiceSucceeded(IBase *pClient,
                                              TransactionID const &rTranID)
{
    appMSG("Runtime Allocate Service Succeeded");
}

void appbase::runtimeEvent(const IEvent &rEvent)
{
    appMSG("Generic message handler (runtime)");
}

// <end IRuntimeClient interface>
