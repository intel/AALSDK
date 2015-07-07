// Copyright (c) 2014-2015, Intel Corporation
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
//        FILE: Runtime.h
//     CREATED: Mar 3, 2014
//      AUTHOR: Joseph Grecco <joe.grecco@intel.com>
//
// PURPOSE:   Public definitions for the I_AALRUNTIME interface.
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
//****************************************************************************///
#ifndef __AAL_RUNTIME_H__
#define __AAL_RUNTIME_H__
#include <aalsdk/AALTypes.h>           // AAL Types
#include <aalsdk/AALIDDefs.h>

#include <aalsdk/osal/CriticalSection.h>

#include <aalsdk/AALNamedValueSet.h>   // Named Value Sets
#include <aalsdk/CUnCopyable.h>        // Uncopyable class
#include <aalsdk/CAALBase.h>           // CAASBase
#include <aalsdk/IServiceClient.h>
#include <aalsdk/osal/IDispatchable.h>


#include <vector>
using namespace std;

/// @addtogroup Runtime
/// @{
/** The Runtime object is exposed by the AAL Runtime implementation as a means to provide its services
 *  to the rest of the framework. The Runtime needs to be started, and it will communicate
 *  back via the IRuntimeClient interface, which the client application must instantiate.
 *  Typically, this can be done in a template or re-usable super-class, but for high-performance
 *  applications one may wish to handle it directly.
 */


BEGIN_NAMESPACE(AAL)

///======================================
///  AAL Runtime IDs
///======================================
/// Event IDs.
#define extranevtRuntimeCreateorProxy     __AAL_ExTranEvt(AAL_sysAAL, 0x0100)
#define extranevtRuntimeDestroyorRelease  __AAL_ExTranEvt(AAL_sysAAL, 0x0101)
#define extranevtStartFailed              __AAL_ExTranEvt(AAL_sysAAL, 0x0102)
#define extranevtProxyStopped             __AAL_ExTranEvt(AAL_sysAAL, 0x0103)
#define extranevtServiceAllocateFailed    __AAL_ExTranEvt(AAL_sysAAL, 0x0104)

#define reasNotOwner                     AAL_ReasCode(0x0100)
#define reasInitError                    AAL_ReasCode(0x0101)


class  IRuntime;

#define AALRUNTIME_CONFIG_RECORD "AALRUNTIME_CONFIG_RECORD"
#define AALRUNTIME_CONFIG_BROKER_SERVICE  "AALRUNTIME_CONFIG_BROKER_SERVICE"




//=============================================================================
/// @class        IRuntimeClient
/// @brief        Public Interface class for the AAL AALRUNTIME Client object.
///
///   An object that
///   wants to use the XL Runtime instantiates an instance of this class,
///   encapsulates it's this pointer into an IBase, and passes that to an
///   instantiated Runtime object via that object's Runtime.start() function.
///   That establishes a binding between the two objects so that the Runtime
///   object can call this object when the Runtime needs to notify its client.
//=============================================================================

class IRuntimeClient
{
public:
   /// @brief     Called by a Runtime object to indicate that it failed to
   ///               successfully allocate a new instance or to return a proxy
   ///               after a call to Runtime() or getRuntimeProxy().
   /// @param[in] rEvent will be an exception event that can be parsed to determine
   ///               the error that occurred.
   /// @return    void
   virtual void runtimeCreateOrGetProxyFailed(IEvent const &rEvent) = 0;


   /// @brief     Called by a Runtime object to indicate that it started successfully
   ///               after a call to Runtime.start()
   /// @param[in] pRuntime Pointer to the Runtime object that is calling back
   ///               indicating that it has started successfully after
   ///               Runtime.start() was called.
   /// @param[in] rConfigParms Copy of the configuration parameters passed in to
   ///               Runtime.start() call.
   /// @return    void
   virtual void runtimeStarted(IRuntime            *pRuntime,
                               const NamedValueSet &rConfigParms) = 0;

   /// @brief     Called by a Runtime object to indicate that it has stopped successfully
   ///               after a call to Runtime.stop()
   /// @param[in] pRuntime Pointer to the Runtime object that is calling back
   ///               indicating that it has stopped successfully after
   ///               Runtime.stop() was called.
   /// @return    void
   virtual void runtimeStopped(IRuntime *pRuntime) = 0;

   /// @brief     Called by a Runtime object to indicate that it failed to start
   ///               successfully after a call to Runtime.start().
   ///
   ///            Although not started, the object still exists, and if dynamically
   ///               allocated will still need to be freed.
   /// @param[in] rEvent will be an exception event that can be parsed to determine
   ///               the error that occurred.
   /// @return    void
   virtual void runtimeStartFailed(const IEvent &rEvent) = 0;

   /// @brief     Called by a Runtime object to indicate that it failed to stop
   ///               successfully after a call to Runtime.stop().
   ///
   ///            This will usually occur when trying to stop a proxy.
   ///
   /// @param[in] rEvent will be an exception event that can be parsed to determine
   ///               the error that occurred.
   /// @return    void
   virtual void runtimeStopFailed(const IEvent &rEvent) = 0;

   /// @brief     Called by a Runtime object to indicate that it failed to
   ///               successfully allocate a service after a call to
   ///               Runtime.allocService().
   /// @param[in] rEvent will be an exception event that can be parsed to determine
   ///               the error that occurred.
   /// @return    void
   virtual void runtimeAllocateServiceFailed( IEvent const &rEvent) = 0;

   /// @brief     Called by a Runtime object to indicate that it
   ///               successfully allocated a service after a call to
   ///               Runtime.allocService().
   ///
   /// Note that the Service version of this function and the Runtime version of this function
   ///   are both called. One can choose to use either or both.
   ///
   /// @param[in] pServiceBase is an IBase that will contain the pointer to the service
   ///               that was allocated. The actual pointer is extracted via the
   ///               dynamic_ptr<> operator. It will also contain a pointer to the
   ///               IService interface of the Service that was allocated, through which
   ///               Release() will need to be called.
   /// @param[in] rTranID is reference to the TransactionID that was passed to
   ///               Runtime.allocService().
   /// @return    void
   ///
   /// @code
   /// void runtimeAllocateServiceSucceeded( IBase *pServiceBase,
   ///                                       TransactionID const &rTranID) {
   ///    ASSERT( pServiceBase );        // if false, then Service threw a bad pointer
   ///
   ///    ISampleAFUPing *m_pAALService; // used to call Release on the Service
   ///    m_pAALService = dynamic_ptr<IAALService>( iidService, pServiceBase);
   ///    ASSERT( m_pAALService );
   ///
   ///    ISampleAFUPing *m_pPingAFU;    // used for Specific Service (in this case Ping)
   ///    m_pPingAFU = dynamic_ptr<ISampleAFUPing>( iidSampleAFUPing, pServiceBase);
   ///    ASSERT( m_pPingAFU );
   /// }
   /// @endcode
   virtual void runtimeAllocateServiceSucceeded( IBase               *pServiceBase,
                                                 TransactionID const &rTranID) = 0;

   /// @brief     Called by a Runtime object to pass exceptions and other
   ///               unsolicited messages.
   /// @param[in] rEvent will be an event that can be parsed to determine
   ///               what occurred.
   /// @return    void
   virtual void runtimeEvent(const IEvent &rEvent) = 0;

   /// @brief     Destructor
   virtual ~IRuntimeClient() {}
};

//=============================================================================
/// @class IRuntime
/// Public pure virtual interface class the AAL AALRUNTIME object.
///
/// NOTE: Although one can directly instantiate a Runtime object for convenience,
///   the framework is designed to use pointers to interfaces, which for Runtime
///   is IRuntime.
//=============================================================================
class AALRUNTIME_API IRuntime
{
public:
   /// @brief     Starts the Runtime and all internal Services.
   /// @param[in] rconfigParms provides configuration parameters for this instance
   /// @return    true = success.  IRuntime client also receives callback.
   virtual btBool start(const NamedValueSet &rconfigParms)                       = 0;

   /// @brief     Stops the Runtime. releases any resources and shutsdown all
   ///               Services.
   /// @return    void
   virtual void   stop()                                                         = 0;

   /// @brief     Allocates a Service
   /// @param[in] pClient - IBase of client object.
   ///            rManifest - reference to a Manifest describing teh Service desired
   ///            rTranID - Transaction ID
   /// @return    void
   virtual void allocService(IBase                *pClient,
                             NamedValueSet const  &rManifest = NamedValueSet(),
                             TransactionID const  &rTranID   = TransactionID())  = 0;

   /// @brief     Schedule a Dispatchable
   /// @param[in] pdispatchable - Pointer Dispatchable to schedule.
   /// @return    void
   virtual btBool schedDispatchable(IDispatchable *pdispatchable)                = 0;

   /// @brief     Returns a unique pointer to the Runtime. This enables subordinate
   ///               objects to use the Runtime independently. Note that this pointer
   ///               references the singleton Runtime instance.
   /// @param[in] rEvent will be an event that can be parsed to determine
   ///               what occurred.
   /// @return    void
   virtual IRuntime *getRuntimeProxy(IRuntimeClient *pClient)                    = 0;


   /// @brief     Releases pointer to the Runtime acquired from getRuntimeProxy.
   /// @param[in] Optional pointer to proxy to release. Self if not present
   /// @return    true - success
   virtual btBool releaseRuntimeProxy(IRuntime *pRuntime)                        = 0;
   virtual btBool releaseRuntimeProxy()                                          = 0;

   /// @brief     Gets the pointer to the Runtime interface attached to this Proxy.
   /// @return    Client Pointer
   virtual IRuntimeClient *getRuntimeClient()                                    = 0;

   /// @brief     Returns status of Runtime
   /// @return    void
   virtual btBool IsOK()                                                         = 0;

protected:
   // Only Runtime can be destroyed. Interface may be to a proxy.
   virtual ~IRuntime() {}
};

//=============================================================================
/// @class Runtime
/// Public wrapper class for the AAL AALRUNTIME object.
///
/// Instantiate this object
///   and one has access to the service allocation and asynchronous eventing
///   system of the AALRUNTIME.
//=============================================================================
class _runtime; //Forward reference

class AALRUNTIME_API Runtime : private CUnCopyable, public CAASBase, public IRuntime
{
public:
   Runtime(IRuntimeClient *pClient, Runtime *pParent = NULL);

   virtual btBool start(const NamedValueSet &rconfigParms);
   virtual void   stop();

   virtual void allocService( IBase               *pClient,
                              const NamedValueSet &rManifest = NamedValueSet(),
                              TransactionID const &rTranID   = TransactionID() );

   virtual btBool schedDispatchable(IDispatchable *pdispatchable);

   IRuntime *getRuntimeProxy(IRuntimeClient *pClient);

   btBool releaseRuntimeProxy(IRuntime *pRuntime);

   btBool releaseRuntimeProxy();

   IRuntimeClient *getRuntimeClient();

   btBool IsOK();

   virtual ~Runtime();


private:
   Runtime();                             // No empty construction
   _runtime          *m_pImplementation;  // Implementation of runtime
   IRuntime          *m_pParent;          // Parent of this proxy
   btBool             m_status;           // OK or not
   IRuntimeClient    *m_pClient;
   vector<Runtime*>   m_proxyList;        // List of Runtime proxies created through this object
};


END_NAMESPACE(AAL)

/// @} group Runtime

#endif // __AAL_RUNTIME_H__

