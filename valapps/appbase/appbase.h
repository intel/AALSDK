#ifndef APPBASE_H
# define APPBASE_H

//****************************************************************************
#include <aalsdk/AALTypes.h>
#include <aalsdk/Runtime.h>
#include <aalsdk/AALLoggerExtern.h>
#include <aalsdk/service/IALIAFU.h>
#include <aalsdk/AALIDDefs.h>
#include <aalsdk/CAALBase.h>

#include <iostream>
#include <string>
#include <list>

//#include <config.h>
#include <appbuffer.h>
#include "arguments.h"
//****************************************************************************
// UN-COMMENT appropriate # define in order to enable either Hardware or ASE.
//****************************************************************************
# define  HWAFU
//# define  ASEAFU

using namespace std;
using namespace AAL;

template<class T>
void appMSG(T x) {
    std::cout << __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() : " << x << std::endl;
}

template<class T>
void appERR(T x) {
    std::cerr << __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() **Error : " << x << std::endl;
}

// Application conditions
enum APPMESSAGES {
    APP_IS_OK = 0,
    NO_SERVICE_ALLOCATE = 101,
    NO_BUFFER_ALLOCATE = 102,
    NO_BUFFER_GETVA = 103
};

/// @brief   Since this is a simple application, our App class implements both the IRuntimeClient and IServiceClient
///          interfaces.  Since some of the methods will be redundant for a single object, they will be ignored.
class appbase: public CAASBase, public IRuntimeClient, public IServiceClient
{
public:

    appbase();
    appbase(const string& appType,
            const string& AFUId,
            const string& appName);
    appbase(const string& appType,
            const string& AFUId,
            const string& appName,
            btWSSize dsmSize,
            btWSSize inputSize,
            btWSSize outputSize);

    ~appbase();

    btBool isOK()  {return m_bIsOK;}

    // Custom derivations
    virtual btInt run(const arguments& args) = 0;    ///< Return 0 if success

    btInt requestService(const arguments &args);   /// < Return 0 if success

    btInt requestBuffers();   /// < Return 0 if success

    btInt requestBuffers(btWSSize dsmSize, btWSSize inputSize, btWSSize outputSize);   /// < Return 0 if success

    btInt releaseBuffers();   /// < Return 0 if success

    btInt requestExit(); /// < Return 0 if success

    void setRequestBuffers(btBool value);

    btWSSize getDSMSize() {return m_DSMSize;}

    btWSSize getInputSize() {return m_InputSize;}

    btWSSize getOutputSize() {return m_OutputSize;}

protected:

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

    void runtimeStarted(IRuntime *pRuntime,
                        const NamedValueSet &rConfigParms);

    void runtimeStopped(IRuntime *pRuntime);

    void runtimeStartFailed(const IEvent &rEvent);

    void runtimeStopFailed(const IEvent &rEvent);

    void runtimeAllocateServiceFailed(IEvent const &rEvent);

    void runtimeAllocateServiceSucceeded(IBase *pClient,
                                         TransactionID const &rTranID);

    void runtimeEvent(const IEvent &rEvent);
    // <end IRuntimeClient interface>

// Original code started from here
    Runtime     m_Runtime;      // AAL Runtime
    IBase      *m_pAALService;  // The generic AAL Service interface for the AFU.
    IALIBuffer *m_pALIBufferService; // Pointer to Buffer Service
    IALIMMIO   *m_pALIMMIOService; // Pointer to MMIO Service
    IALIReset  *m_pALIResetService; // Pointer to AFU Reset Service
    CSemaphore  m_Sem;          // For synchronizing with the AAL runtime.
    btInt       m_Result;       // Returned result value; 0 if success

    // Workspace member data
    btVirtAddr m_DSMVirt;       // DSM workspace virtual address
    btPhysAddr m_DSMPhys;       // DSM workspace physical address
    btWSSize   m_DSMSize;       // DSM workspace size in bytes
    btVirtAddr m_InputVirt;     ///< Input workspace virtual address.
    btPhysAddr m_InputPhys;     ///< Input workspace physical address.
    btWSSize   m_InputSize;     ///< Input workspace size in bytes.
    btVirtAddr m_OutputVirt;    ///< Output workspace virtual address.
    btPhysAddr m_OutputPhys;    ///< Output workspace physical address.
    btWSSize   m_OutputSize;    ///< Output workspace size in bytes.

    // NOTE: This example is bypassing the Resource Manager's configuration record lookup
    // mechanism.  Since the Resource Manager Implementation is a sample, it is subject to change.
    // This example does illustrate the utility of having different implementations of a service all
    // readily available and bound at run-time.
    NamedValueSet Manifest;
    NamedValueSet ConfigRecord;

    // Custom derivations (properties)
    btInt m_Status;
    btBool m_RequestBuffers;
    string m_AppType;
    string m_AppName;
    string m_AFUId;

    // Custom derivations (helpers)
    btInt allocateBuffer(btWSSize& reqSize, btVirtAddr& virtAddress, btPhysAddr& physAddress);

    // Custom derivations (methods)
    btBool runtimeStart();   /// < Return true if success

};

#endif
