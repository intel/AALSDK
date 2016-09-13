// Copyright(c) 2006-2016, Intel Corporation
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
/// @file AASResourceManager.cpp
/// @brief Registrar and ResourceManager Daemon.
///        Originally from aalktest.c by Joseph Grecco.
/// @ingroup ResMgr
/// @verbatim
/// Accelerator Abstraction Layer
///
/// AUTHORS: Henry Mitchel, Intel Corporation.
///          Joseph Grecco, Intel Corporation.
///
/// HISTORY: 
/// WHEN:          WHO:     WHAT:
/// 02/24/2008     JG       Initial version of aalrmsserver created
/// 08/10/2008     HM       Moved to AASResourceManager as dedicated file
/// 08/31/2008     HM       Moved CResMgr code to CAASResourceManager.cpp
/// 09/12/2008     HM       Major checkin
/// 09/12/2008     HM       Added configuration update message support
/// 10/05/2008     HM       Converted to use AALLogger
/// 11/13/2008     HM       Verified that 0-length arrays in NVS are okay
/// 11/19/2008     HM       More test code tweaks
/// 11/21/2008     HM       Cleaned up header includes
/// 11/28/2008     HM       Fixed legal header
/// 11/30/2008     HM       Added signal handler and moved test code to bottom
/// 12/14/2008     HM       DestroyIoctlReq name change to DestroyRMIoctlReq
/// 01/04/2009     HM       Updated Copyright
/// 01/14/2009     HM       Removed memory leaks of pIoctlReq from main loop
/// 02/09/2009     HM       Made central loop a bit more robust and chatty for
///                            error cases. Bumped maxErrors to 10, as we are
///                            now testing more severe error cases.
/// 02/19/2009     HM       If maxErrors is 0, do not abort no matter how many
///                            errors are seen.
/// 02/11/2010     JG       Added support for glib version 4.4@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include <stdio.h>

#ifdef STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# ifdef HAVE_STDLIB_H
#    include <stdlib.h>
# else
#    error Required system header stdlib.h not found.
# endif // HAVE_STDLIB_H
#endif // STDC_HEADERS

#include <signal.h>                        // sigaction
#include <errno.h>                         // EINTR

#include "aalsdk/AALLoggerExtern.h"        // theLogger, include before AAL headers
#include "aalsdk/rm/CAASResourceManager.h" // Brings in the skeleton, which brings
                                           // in the database, the proxy, and <string>
#include "aalsdk/kernel/KernelStructs.h"   // for print kernel structs and enums
#include <aalsdk/AAL.h>
#include <aalsdk/Runtime.h>

#include "aalsdk/osal/Env.h"               // for checking environment vars

AAL::IResMgrService *pResMgr;				  // used by signal handler to retrieve file descriptor
const unsigned maxErrors =  10;           // abort (or in the future reset, perhaps) after this many errors have been seen
                                          //    The value 0 means do not test

USING_NAMESPACE(std)
USING_NAMESPACE(AAL)


// Convenience macros for printing messages and errors.
#ifndef MSG
# define MSG(x) std::cout << __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() : " << x << std::endl
#endif // MSG
#ifndef ERR
# define ERR(x) std::cerr << __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() **Error : " << x << std::endl
#endif // ERR


/*
 * Forward declarations for test hooks for various things that are convenient
 *    to test in this code. Actual functions are at the bottom of the file.
 */
void testcode0(void);
void testcode1(void);

//=============================================================================
// Name:          signal_handler
// Description:   Ctrl-C etc handler
// Interface:     public
// Inputs:        context points to CResMgr singleton object
// Outputs:
// Comments:      initiates shutdown
//=============================================================================
void signal_handler(int signo, siginfo_t *info, void *ctx)
{
    struct aalrm_ioctlreq req;
    memset(&req, 0, sizeof(req));
    req.id = reqid_Shutdown;
    req.result_code = rms_resultOK;
    req.data = rms_shutdownReasonMaint;

    if (pResMgr) {
        if (pResMgr->fdServer() != -1) {
            AAL_INFO(LM_ResMgr, "Signal Handler: sending shutdown command\n");
            if (ioctl(pResMgr->fdServer(), AALRM_IOCTL_SENDMSG, &req) == -1) {
                perror(
                        "Send Shutdown message from AASResourceManager to itself, failed");
            } else {
                AAL_INFO(LM_ResMgr, "Signal Handler: sent shutdown command\n");
            }
        } else {
            AAL_WARNING(LM_ResMgr,
                    "Signal Handler: file not open, no way to send shutdown command\n");
        }
    } else {
        AAL_WARNING(LM_ResMgr,
                "Signal Handler: can't see resource manager, no way to send shutdown command\n");
    }
}  // end of signal_handler


//=============================================================================
// Name:          AASResourceManagerDaemon
// Description:   Application class for Resource Manager Service daemon
// Interface:     public
// Inputs:
// Outputs:       none
// Comments:      wraps runtime client and service client implementations
//                for allocating and running a Resource Manager Service
//                (CResMgr via IResMgr).
//=============================================================================
class AASResourceManagerDaemon : public CAASBase,
                                 public IRuntimeClient,
                                 public IServiceClient
{
public:
    AASResourceManagerDaemon();

    ~AASResourceManagerDaemon();

    // <IRuntimeClient>
    void runtimeCreateOrGetProxyFailed(const IEvent &rEvent);
    void runtimeStarted(IRuntime *pRuntime, const NamedValueSet &rConfigParms);
    void runtimeStopped(IRuntime *pRuntime);
    void runtimeStartFailed(const IEvent &rEvent);
    void runtimeStopFailed(const IEvent &rEvent);
    void runtimeAllocateServiceFailed(const IEvent &rEvent);
    void runtimeAllocateServiceSucceeded(IBase *pClient,
                                         const TransactionID &rTranID);
    void runtimeEvent(const IEvent &rEvent);

    // IServiceClient interface
    void serviceAllocated(IBase *pServiceBase, const TransactionID &rTranID);
    void serviceAllocateFailed(const IEvent &rEvent);
    void serviceReleaseFailed(const AAL::IEvent&);
    void serviceReleased(TransactionID const &rTranID);
    virtual void serviceReleaseRequest(IBase *pServiceBase, const IEvent &rEvent){};  // Ignored TODO better implementation
    void serviceEvent(const IEvent &rEvent);

    // Class-specific methods
    // Start application and block until complete
    btBool start();
    btBool isOK() { return m_isOK; };

protected:
    IBase          *m_pAALService;      // Generic AAL service interface
    IResMgrService *m_pResMgrService;   // Service-specific interface
    Runtime         m_Runtime;          // Runtime object
    IRuntime       *m_pRuntime;         // Pointer to our Runtime instance
    CSemaphore      m_Sem;              // used to block main(), sync with
                                        //   callbacks
    btBool          m_isOK;             // Signal OK status
};


/*
 * Daemon c'tor
 */
AASResourceManagerDaemon::AASResourceManagerDaemon() : m_pAALService(NULL),
                                                       m_pResMgrService(NULL),
                                                       m_pRuntime(NULL),
                                                       m_Runtime(this),
                                                       m_Sem(),
                                                       m_isOK(false)
{
    // Register interfaces with our IBase
    SetInterface(iidRuntimeClient, dynamic_cast<IRuntimeClient*>(this));
    SetInterface(iidServiceClient, dynamic_cast<IServiceClient*>(this));
    // there is no IResMgrClient yet

    // Initialize semaphore used to block start()
    m_Sem.Create(0, 1);
    m_isOK = true;
}


AASResourceManagerDaemon::~AASResourceManagerDaemon()
{
    m_Sem.Destroy();
}


/*
 * Main body of application, run from main thread.
 *
 * Starts runtime and then runs the service
 */
btBool AASResourceManagerDaemon::start()
{
    NamedValueSet runtimeConfigArgs;
    NamedValueSet Manifest;
    NamedValueSet ConfigRecord;

    ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME,
                     "libAASResMgr");
    ConfigRecord.Add(AAL_FACTORY_CREATE_SOFTWARE_SERVICE, true);

    Manifest.Add(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &ConfigRecord);
    Manifest.Add(AAL_FACTORY_CREATE_SERVICENAME, "AASResourceManager");

    MSG("Starting runtime");

    m_isOK = false;
    m_Runtime.start(runtimeConfigArgs);

    // Block on semaphore (wait for Runtime to be started)
    m_Sem.Wait();
    if (!isOK()) {
        ERR("Runtime failed to start");
        exit(1);
    }

    MSG("Allocating Resource Manager service");

    m_pRuntime->allocService(dynamic_cast<IBase *>(this), Manifest);

    // Block on semaphore again (wait for service to be allocated and run)
    m_Sem.Wait();

    m_Runtime.stop();

    // Block on semaphore again (wait for runtime to end)
    m_Sem.Wait();
    return true;
}

//-------------------------------------------------------------------------
// IRuntimeClient interface
//-------------------------------------------------------------------------

void AASResourceManagerDaemon::runtimeCreateOrGetProxyFailed(
        IEvent const &rEvent)
{
    MSG("Runtime Create or Get Proxy failed");
    m_isOK = false;
    m_Sem.Post(1);
}

void AASResourceManagerDaemon::runtimeStarted(IRuntime *pRuntime,
                                        const NamedValueSet &rConfigParms)
{
    // Save a copy of our runtime interface instance.
    MSG("Runtime started.");
    m_pRuntime = pRuntime;
    m_isOK = true;
    m_Sem.Post(1);
}

void AASResourceManagerDaemon::runtimeStopped(IRuntime *pRuntime)
{
    MSG("Runtime stopped");
    m_isOK = false;
    m_Sem.Post(1);
}

void AASResourceManagerDaemon::runtimeStartFailed(const IEvent &rEvent)
{
    IExceptionTransactionEvent * pExEvent = dynamic_ptr<
            IExceptionTransactionEvent>(iidExTranEvent, rEvent);
    ERR("Runtime start failed");
    ERR(pExEvent->Description());
}

void AASResourceManagerDaemon::runtimeStopFailed(const IEvent &rEvent)
{
    MSG("Runtime stop failed");
}

void AASResourceManagerDaemon::runtimeAllocateServiceFailed(
        IEvent const &rEvent)
{
    IExceptionTransactionEvent * pExEvent =
            dynamic_ptr<IExceptionTransactionEvent>(iidExTranEvent, rEvent);
    ERR("Runtime AllocateService failed");
    ERR(pExEvent->Description());
}

void AASResourceManagerDaemon::runtimeAllocateServiceSucceeded(IBase *pClient,
                                                    TransactionID const &rTranID)
{
    TransactionID const * foo = &rTranID;
    MSG("Runtime Allocate Service Succeeded");
}

void AASResourceManagerDaemon::runtimeEvent(const IEvent &rEvent)
{
    MSG("Generic message handler (AASResourceManagerDaemon)");
}


//-------------------------------------------------------------------------
// IServiceClient interface
//-------------------------------------------------------------------------

void AASResourceManagerDaemon::serviceAllocated(IBase *pServiceBase,
                                                TransactionID const &rTranID)
{
    int theErrorCode;

    m_pAALService = pServiceBase;
    ASSERT(NULL != m_pAALService);
    if (NULL == m_pAALService){
        return;
    }

    IResMgrService *ptheService = dynamic_ptr<IResMgrService>(iidResMgrService, pServiceBase);

    ASSERT(NULL != ptheService);
    if ( NULL == ptheService) {
        return;
    }

    // save reference to service in global variable
    // to be used by signal handler
    pResMgr = ptheService;

    MSG("Service Allocated");

    // We have the Service, now start it
    // Resource Manager will be run in main thread
    theErrorCode = ptheService->start(TransactionID(), false);
    if (theErrorCode != 0) {
        AAL_ERR(LM_ResMgr,
                "AASResourceManager::message pump bailed with error code " <<
                theErrorCode << ". Attempting to shut down.\n");
        // do we need to do something special?
    } else {
        AAL_INFO(LM_ResMgr,
                "AASResourceManager::Exited message pump cleanly. "
                "Attempting to shut down.\n");
    }

    // on return, release service
    MSG("Releasing service");
    pResMgr = NULL;	// delete reference for signal handler
    IAALService *pIAALService = dynamic_ptr<IAALService>(iidService,
            m_pAALService);
    ASSERT(pIAALService);
    pIAALService->Release(TransactionID());
}


void AASResourceManagerDaemon::serviceAllocateFailed(const IEvent &rEvent)
{
    IExceptionTransactionEvent * pExEvent =
            dynamic_ptr<IExceptionTransactionEvent>(iidExTranEvent, rEvent);
    ERR("Failed to allocate a Service");
    ERR(pExEvent->Description());
    m_Sem.Post(1);
}

void AASResourceManagerDaemon::serviceReleaseFailed(const IEvent &rEvent)
{
    MSG("Failed to Release a Service");
    m_Sem.Post(1);
}

void AASResourceManagerDaemon::serviceReleased(TransactionID const &rTranID)
{
    MSG("Service Released");
    m_Sem.Post(1);
}

void AASResourceManagerDaemon::serviceEvent(const IEvent &rEvent)
{
    ERR("unexpected event 0x" << hex << rEvent.SubClassID());
}

//=============================================================================
// Name:          main
// Description:   main loop
// Interface:     public
// Inputs:
// Outputs:       none
// Comments:
//=============================================================================
int main(int argc, char **argv)
{

    // Set up the logger
// pAALLogger()->AddToMask(LM_ResMgr,LOG_INFO);
    //pAALLogger()->AddToMask(LM_ResMgr,LOG_DEBUG);
    pAALLogger()->AddToMask(LM_ResMgr, LOG_VERBOSE);

// pAALLogger()->AddToMask(LM_Database,LOG_DEBUG);
    pAALLogger()->AddToMask(LM_Database, LOG_INFO);

    pAALLogger()->SetLogPID(true);
    pAALLogger()->SetLogTimeStamp(true);
    pAALLogger()->SetLogErrorLevelPrepend(true);
    pAALLogger()->SetLogPrepend("");
    pAALLogger()->SetDestination(ILogger::COUT);

    testcode0();      // generally, do nothing

    /*
     * set up signal handler
     */
    struct sigaction sa;
    sigfillset(&sa.sa_mask);               // Don't mask any signals
    sa.sa_sigaction = signal_handler;      // Our handler
    sa.sa_flags = SA_SIGINFO | SA_RESTART; // Don't fail interrupted call
                                           // AND use our handler

    // Replace these signal handlers
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGHUP, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    // TODO: when a daemon, will want to respond to SIGHUP by cleaning up, reloading the configuration file and
    // restarting with the new values.

// AAL_DEBUG(LM_ResMgr,"AASResourceManager argv[0] is " << argv[0] << endl);

    // TODO: parse for demon status, other?

    // Check for conflicting environment
    std::string strBuf;
    if ( Environment::GetObj()->Get("AAL_RESOURCEMANAGER_CONFIG_INPROC", strBuf) ) {
       std::cerr << "AAL_RESOURCEMANAGER_CONFIG_INPROC environment variable is "
                    "set.\nRefusing to start Resource Manager Daemon." <<
                    std::endl;
       exit(1);
    }

    AASResourceManagerDaemon *app = new (nothrow) AASResourceManagerDaemon();

    if (!app) {
        std::cerr <<
                "Could not create a AASResourceManagerDaemon (insufficient "
                "memory?), exiting with status 1\n" << std::endl;
        exit(1);
    }

    if (!app->isOK()) {
       std::cerr <<
                "AASResourceManagerDaemon failed to start (ctor failed), "
                "exiting with status 2\n" << std::endl;
        exit(2);
    }

    app->start();

    testcode1();         // Generally, do nothing

    AAL_INFO(LM_ResMgr, "AASResourceManagerDaemon done, deleting.");
    delete app;

    AAL_INFO(LM_ResMgr, "Shutdown completed. Exiting now.\n");

    exit(0);

}  // end of main()

void testcode0(void)
{
#if 0
   // Test afu guid stuff
   // NLB GUID is c000c9660d8242729aeffe5f84570612
   btUnsigned64bitInt u64High = 0xc000c9660d824272UL;
   btUnsigned64bitInt u64Low = 0x9aeffe5f84570612UL;

   cout << "NLB AFU ID" << endl;
   cout << "high 64 bits " << hex << u64High << endl;
   cout << "low  64 bits " << hex << u64Low  << endl;

   AAL_GUID_t afu_id;
   memset(&afu_id, 0xFF, sizeof(afu_id));

   afu_id = GUIDStructFrom2xU64( u64High, u64Low);
//   cout << "guid struct " << afu_id << endl;

   cout << "guid string " << GUIDStringFromStruct( afu_id ) << endl;

   // VAFU ID
   cout << endl;
   cout << "VAFU AFU ID" << endl;
   memset(&afu_id, 0xFF, sizeof(afu_id));
   u64High = 0x0UL;
   u64Low = 0x11100181UL;
   cout << "high 64 bits " << hex << u64High << endl;
   cout << "low  64 bits " << hex << u64Low  << endl;
   afu_id = GUIDStructFrom2xU64( u64High, u64Low);
   cout << "guid string " << GUIDStringFromStruct( afu_id ) << endl;

   exit(0);

#endif

#if 0
#include<aalsdk/osal/IDispatchable.h>
#include <aalsdk/osal/Sleep.h>
#include <aalsdk/osal/ThreadGroup.h>
   // Functor for Fire-and-Forget test.
   class FireAndForget : public IDispatchable
   {
   public:
      FireAndForget(unsigned long delay_in_micros) : m_micros(delay_in_micros)
      {}

      void operator() ()
      {
         SleepMicro(m_micros);
         delete this;
      }

   protected:
      unsigned long  m_micros;
   };

   int n=1;

   OSLThreadGroup *tg = new OSLThreadGroup(2);
   CSemaphore     sem;
   int x;
   std::cerr<< "Launchin "<< n << std::endl;
   for(int x=0; x<=5; x++){
      tg->Add( new FireAndForget(6-x) );
   }
   std::cerr<< "Deletin"<<std::endl;
   delete tg;
   n++;

   CSemaphore foo;
   foo.Wait();
   exit(5);
#endif
#if 0
   //---------------------   TEST  CODE ------------------------
   // Test getTSC
   unsigned long long timestamp = getTSC();

   cout << "timestamp1 = " << std::hex << timestamp;

   exit(5);
   //---------------------   TEST  CODE ------------------------
#endif

#if 0
   //---------------------   TEST  CODE ------------------------
   // Test FormattedNVS output, works just fine ...
   NamedValueSet nvs1;
   nvs1.Add("Shared name",35);
   nvs1.Add("name in nvs1","data");
   nvs1.Add("name 2 in nvs1", "data2");

   cout << "Test - regular NVS output:\n" << nvs1;
   cout << "Test - Formatted NVS output with tab=2:\n" << FormattedNVS( nvs1, 2);

   exit(5);
   //---------------------   TEST  CODE ------------------------
#endif

#if 0
   //---------------------   TEST  CODE ------------------------
   // Test sizeof various things on this machine

   AAL_DEBUG(LM_ResMgr, "sizeof unsigned char      is: " << sizeof(unsigned char) << endl);
   AAL_DEBUG(LM_ResMgr, "sizeof unsigned short     is: " << sizeof(unsigned short) << endl);
   AAL_DEBUG(LM_ResMgr, "sizeof unsigned int       is: " << sizeof(unsigned int) << endl);
   AAL_DEBUG(LM_ResMgr, "sizeof unsigned long      is: " << sizeof(unsigned long) << endl);
   AAL_DEBUG(LM_ResMgr, "sizeof unsigned long long is: " << sizeof(unsigned long long) << endl);
   AAL_DEBUG(LM_ResMgr, "sizeof void*              is: " << sizeof(void*) << endl);

// Output
//   DEBUG [8524] 0000:086552 sizeof unsigned char      is: 1
//   DEBUG [8524] 0000:086571 sizeof unsigned short     is: 2
//   DEBUG [8524] 0000:086580 sizeof unsigned int       is: 4
//   DEBUG [8524] 0000:086588 sizeof unsigned long      is: 8
//   DEBUG [8524] 0000:086596 sizeof unsigned long long is: 8
//   DEBUG [8524] 0000:086604 sizeof void*              is: 8

   exit(5);
   //---------------------   TEST  CODE ------------------------
#endif

#if 0
   //---------------------   TEST  CODE ------------------------
   // Test NVSMerge - it works
   NamedValueSet nvs1, nvs2;
   nvs1.Add("Shared name",35);
   nvs1.Add("name in nvs1","data");
   nvs1.Add("name 2 in nvs1", "data2");
   nvs2.Add("name in nvs2", "data");
   nvs2.Add("Shared name","data from nvs2");

   AAL_DEBUG(LM_ResMgr, "nvs1\n" << nvs1);
   AAL_DEBUG(LM_ResMgr, "nvs2\n" << nvs2);
   NVSMerge( nvs2, nvs1);
   AAL_DEBUG(LM_ResMgr, "nvs2 after merging in nvs1\n" << nvs2);

   exit(5);
   //---------------------   TEST  CODE ------------------------
#endif

#if 0
   //---------------------   TEST  CODE ------------------------
   // Test the movement of binary codes through strings and string streams.
   // As long as done correctly, it works.
   NamedValueSet nvs;
   btByte                  rgb8TestTemp[] = {' ','B','Y','T','E','S',0x0,'M','O'};
   btByteArray             rgb8Test = rgb8TestTemp;
   nvs.Add("A", rgb8Test, NUM_ELEMENTS(rgb8TestTemp));

   fstream ofs;
   ofs.open("tempfile", ios::binary | ios::out);
   ofs << nvs;

   string s = StringFromNamedValueSet(nvs);
   int len = s.length();
   char* pBuf = new char(len);
   CharFromString( pBuf, s);

   nvs.Empty();
   NamedValueSetFromCharString( pBuf, len, nvs);
   ofs << nvs;
   ofs.close();
   exit(5);
   //---------------------   TEST  CODE ------------------------
#endif

#if 0
   //---------------------   TEST  CODE ------------------------
   // Test GUID String<-->Struct conversions. They work.
   string sKeyName = StringNameFromAIA_ID(0x123456789ABCDEF0LL);
   btUnsigned64bitInt u64 = AIA_IDFromStringName(sKeyName);
   AAL_DEBUG(LM_ResMgr, std::hex << std::uppercase << u64 << endl);

//   typedef struct {
//      btUnsigned32bitInt Data1;
//      unsigned short     Data2;
//      unsigned short     Data3;
//      char               Data4[8];
//   } AAL_GUID_t;
   AAL_GUID_t gs;
   gs.Data1 = 0x01020304;  // 16909060
   gs.Data2 = 0x0506;      // 1286
   gs.Data3 = 0x0708;      // 1800
   gs.Data4[0] = 0x09;
   gs.Data4[1] = 0x0A;
   gs.Data4[2] = 0x0B;
   gs.Data4[3] = 0x0C;
   gs.Data4[4] = 0x0D;
   gs.Data4[5] = 0x0E;
   gs.Data4[6] = 0x0F;
   gs.Data4[7] = 0xFF;

   std::string s = GUIDStringFromStruct(gs);
   AAL_DEBUG(LM_ResMgr, std::hex << s << endl);

   memset(&gs, 0, sizeof(AAL_GUID_t));
   GUIDStructFromString(s, &gs);
   AAL_DEBUG(LM_ResMgr, "struct\t" << std::hex << std::uppercase <<
                        gs.Data1 << "\t" <<
                        gs.Data2 << "\t" <<
                        gs.Data3 << "\t" <<
                        gs.Data4 << "\t" <<
                        endl);

   std::string t = AFU_IDNameFromGUID( gs);
   AAL_DEBUG(LM_ResMgr, std::hex << t << endl);

   memset(&gs, 0, sizeof(AAL_GUID_t));
   GUIDFromAFU_IDName( t, &gs);
   AAL_DEBUG(LM_ResMgr, "struct\t" << std::hex << std::uppercase <<
                        gs.Data1 << "\t" <<
                        gs.Data2 << "\t" <<
                        gs.Data3 << "\t" <<
                        gs.Data4 << "\t" <<
                        endl);
   exit(5);
   //---------------------   TEST  CODE ------------------------
#endif

#if 0
   //---------------------   TEST  CODE ------------------------
   // Testing device address formation into an Integer Name
   // and back again. Works.
   aal_device_addr devaddr;
   devaddr.m_busnum = 3;
   devaddr.m_bustype = aal_bustype_FSB;
   devaddr.m_devicenum = 4;
   devaddr.m_subdevnum = 5;

   btNumberKey key = IntNameFromDeviceAddress(&devaddr);

   AAL_DEBUG(LM_ResMgr, "key is: " << std::hex << std::uppercase << key << endl);

   memset(&devaddr, 0, sizeof(aal_device_addr));

   DeviceAddressFromIntName( key, &devaddr);
   AAL_DEBUG(LM_ResMgr, devaddr);
   exit(5);
   //---------------------   TEST  CODE ------------------------
#endif

}  // end of testcode0

void testcode1(void)
{
#if 0
   //---------------------   TEST  CODE ------------------------
   // Test ComputeGoalRecords and GetPolicyResults
   // Build a fake manifest
   NamedValueSet nvsManifest;
   NamedValueSet nvsEmbedded;
   nvsEmbedded.Add("fakename","fakedata");
   nvsManifest.Add(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, nvsEmbedded);

   AAL_DEBUG(LM_ResMgr, "Manifest\n:" << nvsManifest);

   // Get a return list
   nvsList listGoalRecords;
   btBool test = pResMgr->ComputeGoalRecords( nvsManifest, listGoalRecords);

   // Get a return Goal record
   NamedValueSet nvsGoal;
   test = pResMgr->GetPolicyResults( listGoalRecords, nvsGoal);

   AAL_DEBUG(LM_ResMgr, "keyRegHandle:" << keyRegHandle << endl);
   AAL_DEBUG(LM_ResMgr, "Goal Record:\n" << nvsGoal);

   exit(5);
   //---------------------   TEST  CODE ------------------------
#endif

}  // end of testcode1


