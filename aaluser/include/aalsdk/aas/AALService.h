// Copyright(c) 2011-2017, Intel Corporation
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
/// @file AALService.h
/// @brief This file contains the implementation of classes and templates
///    used to create AAL services.
/// @ingroup Services
/// @verbatim
/// Accelerator Abstraction Layer
///
/// AUTHOR: Joseph Grecco, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 06/02/2011     JG       Initial version
/// 08/30/2011     JG       Split out device services
/// 09/01/2011     JG       Changed AALServiceContainer into AASServiceModule
/// 09/05/2011     JG       Implemented lists of service is a module.
///                          clean-up and release of multiple services
/// 09/07/2011     JG       Refactored, removing NVS and Factories
/// 10/26/2011     JG       Added copy constructor
/// 10/31/2011     JG       Changed IService to IAALService for Windows port
/// 01/26/2012     JG       Modified the initialization protocol making the
///                          Service class hiearchy initialize in the proper
///                          order (base->most derived) even when async init
///                          was required. Major change was addition of
///                          InitComplete<> Function Object Event template.
/// 04/12/2012     JG       Fixed bugs in proxy and stubs _init() protocols
///                          which did not properly call subclass' init()
/// 04/13/2012     HM       Added eRelease to aal_service_method_id
/// 04/23/2012     HM       getmsg() returns FALSE on EOF as well as error
/// 06/07/2012     JG       Fixed Release() protocol so that it now cleans-up
///                          from the most derived toward the base.  Derived
///                          class need only implement Release() if it has work
///                          do, in which case it cleans itself up and calls
///                          parent Release().
/// 03/13/2014     JG       Added support for client callback interface
///                          signaling.
/// 7/14/2017     JG       Revised IPC SDK for updated AAL SDK @endverbatim
//****************************************************************************
#ifndef __AALSDK_AAS_AALSERVICE_H__
#define __AALSDK_AAS_AALSERVICE_H__
#include <aalsdk/aas/AALServiceModule.h>
#include <aalsdk/osal/Thread.h>
#include <aalsdk/utils/AALEventUtilities.h>
#include <aalsdk/IServiceClient.h>
#include <aalsdk/aas/IServiceRevoke.h>
#include <aalsdk/Runtime.h>

#include <queue>

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//////////////                                                   //////////////
///////                          AAL SERVICE                            ///////
//////////////                                                   //////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

BEGIN_NAMESPACE(AAL)

/// @addtogroup ServiceBase
/// @{
/// @brief Base class for all AAL Service objects
///
/// The ServiceBase class provides implementation for the canonical interfaces
/// required of a Service. In addition the ServiceBase provides convenient member
/// functions that enable the developer access to useful data and Platform Services.
/// It is expected that all AAL Services derive directly or indirectly from ServiceBase.


/// Event for posting Initialize completion.
///
/// This template is typically used by Service SDK base classes, such as DeviceServiceBase,
/// during the "init chain" processing. The "init chain" is the protocol implemented by the
/// AAL runtime that causes Service class initialization to occur from the root base class on
/// up the class hierarchy, resulting in ALL base subclasses being initialized prior to the
/// final Service class, typically the developer's Service implementation.  Subclasses use
/// the InitComplete to schedule their init() behavior. Refer to ServiceBase and DeviceServiceBase
/// _init() methods for more detail.
template <class T>
class InitComplete : public CAALEvent
{
public:
   /// InitComplete Constructor.
   ///
   /// @param[in]  That  Object on which the member function ptr is to be invoked on Dispatch().
   /// @param[in]  ptr   The member function to be invoked on Dispatch().
   /// @param[in]  rtid  TransactionID for the event.
   InitComplete(T                                  *That,
                void (T::*ptr)(TransactionID const &rtid),
                TransactionID const                &rtid) :
      m_That(That),
      m_ptr(ptr),
      m_tid(rtid)
   {}

   virtual IEvent * Clone() const
   {
      return new(std::nothrow) InitComplete<T>(*this);
   }

   /// Dispatch an event.
   ///
   /// @param[in]   target  Assumed to be of type EventHandler.
   /// @return void
   virtual void Dispatch(btObjectType target) const
   {
      (m_That->*m_ptr)(m_tid);
   }

   virtual void operator()()
   {
      (m_That->*m_ptr)(m_tid);
   }

protected:
   InitComplete(const InitComplete &other) :
      CAALEvent(other),
      m_That(other.m_That),
      m_ptr(other.m_ptr),
      m_tid(other.m_tid)
   {}

   T                  *m_That;
   void           (T::*m_ptr)(TransactionID const &);
   const TransactionID m_tid;
};


//=============================================================================
/// @interface IServiceBase
/// @brief ServiceBase Interface used for framework operations not intended to be
///  exposed to users.
///
///   The ServiceBase class is intended to be inherited by AAL Services.
///   The purpose is to provide the canonical implementation for the Service.
//=============================================================================
class AASLIB_API IServiceBase
{
public:
   virtual ~IServiceBase() {}

   /// @brief Called once when the Service is done initializing.
   /// @retval True if the Service initialize successfully.
   /// @retval False if the Service did not initialize successfully, the Service's
   ///         IServiceBase interface failed to initialize, the Service could not
   ///         be added to the list of Services, or notification of the Service
   ///         Client failed.
   virtual btBool                     initComplete(TransactionID const &rtid) = 0;

   /// @brief Called if the Service fails to initialize successfully.
   /// @retval True if the Service was created, the IServiceBase was created, and
   ///              the thread to destroy the service was started.
   /// @retval False if the Service did not initialize successfully, if the
   ///               IServiceBase interface was not initialized, or the thread
   ///               to destroy the Service failed to start.
   virtual btBool                       initFailed(IEvent const *ptheEvent)   = 0;

   /// @brief Final step when a Service is released.
   /// @retval True.
   virtual btBool                  ReleaseComplete()                          = 0;

   /// @brief Accessor function to get the optional arguments supplied to
   ///        allocService() when the Service was created.
   /// @return The named value set describing the optional arguments.
   virtual NamedValueSet const &           OptArgs() const                    = 0;

   /// @brief Accessor to a pointer to the Service's Client's Interface.
   /// @return A pointer to the IServiceClient interface.
   virtual IServiceClient *       getServiceClient() const                    = 0;

   /// @brief Accessor to a pointer to the Service's Client's IBase Interface
   /// @return A pointer to the IBase interface.
   virtual IBase *            getServiceClientBase() const                    = 0;

   /// @brief Accessor to a pointer to the Runtime to be used in ObjectCreated
   /// @return A pointer to the IRuntime interface.
   virtual IRuntime *                   getRuntime() const                    = 0;

   /// @brief Accessor to a pointer to the Runtime Client
   /// @return A pointer to the IRuntimeClient interface.
   virtual IRuntimeClient *       getRuntimeClient() const                    = 0;

   /// @brief Accessor to this Services Service Module
   /// @return A pointer to the AALServiceModule interface.
   virtual AALServiceModule *  getAALServiceModule() const                    = 0;
};

//=============================================================================
/// @interface ServiceBase
/// @brief Base class for software-based AAL Services.
///
///   The ServiceBase class is intended to be inherited by AAL Services.
///   The purpose is to provide the canonical implementation for the Service.
//=============================================================================
class AASLIB_API ServiceBase : public CAASBase,
                               public IServiceBase,
                               public IRuntimeClient,
                               public IAALService,
                               public IServiceRevoke
{
public:
/// @brief  Used to create the appropriate Constructor signature for Services derived from ServiceBase.
///
///
/// @param[in]  C  The name of the class being constructed.
/// @param[in]  P  The name of the parent class being implemented, ie ServiceBase, DeviceServiceBase, etc.
#define DECLARE_AAL_SERVICE_CONSTRUCTOR(C, P) C(AAL::AALServiceModule *container,                             \
                                                AAL::IRuntime         *pAALRUNTIME,                           \
                                                AAL::IAALTransport    *ptransport   = NULL,                   \
                                                AAL::IAALMarshaller   *marshaller   = NULL,                   \
                                                AAL::IAALUnMarshaller *unmarshaller = NULL) : P(container,    \
                                                                                                pAALRUNTIME,  \
                                                                                                ptransport,   \
                                                                                                marshaller,   \
                                                                                                unmarshaller)

   /// @brief     Constructor called by Service Broker.
   /// @param[in] container Pointer to the Service Module to contain the Service.
   /// @param[in] pAALRuntime Pointer to the AAL Runtime to use.
   /// @param[in] ptransport Pointer to the Transport if the not in-process.
   /// @param[in] marshaller Pointer to the Marshaller if not in-process.
   /// @param[in] unmarshaller Pointer to the Unmarshaller if not in-process.
   /// @return void
   ServiceBase(AALServiceModule *container,
               IRuntime         *pAALRuntime,
               IAALTransport    *ptransport   = NULL,
               IAALMarshaller   *marshaller   = NULL,
               IAALUnMarshaller *unmarshaller = NULL);

   // ServiceBase Destructor.
   virtual ~ServiceBase();

   /// @brief Hook for derived classes to perform any post-creation
   ///        initialization.
   ///
   /// This function is called by the factory AFTER construction and AFTER
   /// _init(), insuring that base class initialization has occurred by the
   ///  time this is called.
   /// @param[in]  pclientBase   Pointer to Interface of client of service.
   /// @param[in]  optArgs       Optional arguments.
   /// @param[in]  rtid          Reference to the TransactionID for routing event responses.
   /// @retval True 
   /// @retval False 
   virtual btBool init(IBase               *pclientBase,
                       NamedValueSet const &optArgs,
                       TransactionID const &rtid) = 0;


   /// @brief Perform any post creation initialization including establishing
   ///               communications.
   ///
   ///           Only base classes have  _init(). This function is designed to be
   ///           called by the factory to perform canonical initialization. The
   ///           factory will call the most derived base class' (super base class)
   ///           _init() function.  It is the responsibility of the base class to
   ///           call its direct ancestor's _init() FIRST to ensure that the class
   ///           hiearchy _init() called.
   /// @param[in]  pclientBase   Pointer to Interface of client of service.
   /// @param[in]  rtid          Reference to the TransactionID for routing event responses.
   /// @param[in]  optArgs       Optional arguments.
   /// @param[in]  pcmpltEvent   Optional completion event. Used internally to
   ///  traverse the inheritance hierarchy during initialization.
   /// @retval True if this class and all derived classes initialized successfully.
   /// @retval False if the ServiceClient interface is not found or invalid, the
   ///               Revoke Service interface cannot be loaded, the init() method
   ///               of this class failed, or the dispatch to the next derived 
   //                class failed.
   ///
   //  Client is using callback interface method of signaling
   virtual btBool _init(IBase               *pclientBase,
                        TransactionID const &rtid,
                        NamedValueSet const &optArgs,
                        CAALEvent           *pcmpltEvent = NULL);

   // <IAALService>

   /// @brief     Called to release Service and free its resources
   ///
   /// Services derived from ServiceBase are expected to call the base class implementation
   ///  AFTER completing their Release() implementation to insure proper clean-up.
   /// @param[in] rTranID Reference to the TransactionID if not atomic.
   /// @param[in] timeout The maximum time to wait for the Release to complete (default = infinite).
   /// @retval True if the Dispatchable to release the Service is added to a work queue.
   /// @retval False if the Dispatchable could not be added to a work queue.
   virtual btBool Release(TransactionID const &rTranID, btTime timeout=AAL_INFINITE_WAIT);

   // </IAALService>

   // <IServiceBase>

   virtual btBool                    initComplete(TransactionID const &rtid);

   virtual btBool                      initFailed(IEvent const        *ptheEvent);

   virtual btBool                 ReleaseComplete();

   virtual NamedValueSet const &          OptArgs() const;

   virtual IServiceClient *      getServiceClient() const;

   virtual IBase *           getServiceClientBase() const;

   virtual IRuntime *                  getRuntime() const;

   virtual IRuntimeClient *      getRuntimeClient() const;

   virtual AALServiceModule * getAALServiceModule() const;

   // </IServiceBase>

   // <IRuntimeClient>
   virtual void runtimeCreateOrGetProxyFailed(IEvent const & rEvent) {/*to be implemented by the service*/}


   virtual void runtimeStarted(IRuntime            *pRuntime,
                               const NamedValueSet &rConfigParms) {/*to be implemented by the service*/}

   virtual void runtimeStopped(IRuntime *pRuntime) {/*to be implemented by the service*/}

   virtual void runtimeStartFailed(const IEvent &rEvent) {/*to be implemented by the service*/}

   virtual void runtimeStopFailed(const IEvent &rEvent) {/*to be implemented by the service*/}

   virtual void runtimeAllocateServiceFailed(IEvent const &rEvent) {/*to be implemented by the service*/}

   virtual void runtimeAllocateServiceSucceeded(IBase               *pServiceBase,
                                                TransactionID const &rTranID) {/*to be implemented by the service*/}

   virtual void runtimeEvent(const IEvent &rEvent) {/*to be implemented by the service*/}

   // </IRuntimeClient>

   // <IServiceRevoke>
   virtual void serviceRevoke();

   // </IServiceRevoke>
   //=============================================================================
   // Name: sendmsg
   /// @brief Convenience function for sending the message currently carried
   ///              by the marshaller.
   //
   //
   /// Comments: This is a destructive send. It empties the marshaller once sent.
   /// @retval True if the message was sent.
   /// @retval False otherwise.
   //=============================================================================
   btBool sendmsg();

   //=============================================================================
   // Name:        getmsg
   /// @brief Convenience function for getting the next message into
   ///              unmarshaller
   //
   /// Comments:    This is a destructive get. It empties the unmarshaller before
   ///              getting.
   /// @retval True if message is in the unmarshaller.
   /// @retval False if the message is not in the unmarshaller.
   //=============================================================================
   btBool getmsg();

   //=============================================================================
   // Name: processmsg
   /// @brief Process a single received message.
   //
   /// Comments This will be called by some scheduling entity dependent on the
   ///           container.
   /// @return void
   //=============================================================================
   // TODO
   virtual void processmsg() {/*to be implemented by the service*/}

   //=============================================================================
   // Name: startMDS
   /// @brief   Starts the message deliver system.
   //
   //
   /// Comments: Used by objects that implement asynchronous message delivery.
   ///           This starts a thread that continuously polls the receiver and
   ///           when a message is available calls proceemsg().
   /// @retval True if the message delivery system was started.
   /// @retval False if the message delivery system was not started - it was
   ///               already running or it failed to start.
   //=============================================================================
   virtual btBool startMDS();

   //--------------------------------------------------
   //                   Utilities
   //--------------------------------------------------

   // Accessors for marshallers and transport
   /// Accessor to get the Marshaller interface.
   /// @return A Reference to an IAALMarshaller interface.
   IAALMarshaller   &   marshall();
   /// Accessor to get the UnMarshaller interface.
   /// @return A Reference to an IAALUnMarshaller interface.
   IAALUnMarshaller & unmarshall();
   /// Accessor to get the receiver Transport interface.
   /// @return A Reference to the receiver IAALTransport interface.
   IAALTransport    &      recvr();
   /// Accessor to get the sender Transport interface.
   /// @return A Reference to the sender IAALTransport interface.
   IAALTransport    &     sender();

   /// Accessor to discover whether or not the Service has a Marshaller interface.
   /// @retval  True The Service has an IAALMarshaller interface.
   /// @retval  False The Service does not have a IAALMarshaller interface.
   btBool          HasMarshaller() const;
   /// Accessor to discover whether or not the Service has an UnMarshaller interface.
   /// @retval  True The Service has an IAALUnMarshaller interface.
   /// @retval  False The Service does not have an IAALUnMarshaller interface.
   btBool        HasUnMarshaller() const;
   /// Accessor to discover whether or not the Service has an IAALTransport interface.
   /// @retval  True The Service has an IAALTransport interface.
   /// @retval  False The Service does not have an IAALTransport interface.
   btBool           HasTransport() const;

   ///@brief Hook to allow Services to expose parameters.
   /// Default is to not support parameters (Services can override to implement parameters).
   /// <B>Parameters</B> [in]  Reference to a named value set describing the parameters.
   /// @retval False.
   virtual btBool SetParms(NamedValueSet const & ) { return false; }

   //---------------------------------
   //  Accessors to utility functions
   //---------------------------------

   /// Enable service to allocate another Service.
   /// @param[in] pClient Pointer to client's callback interface.
   /// @param[in] rManifest Reference to the Manifest describing the Service.
   /// @param[in] rTranID Reference to the Optional TransactionID.
   /// @return void
   void allocService(IBase                  *pClient,
                     NamedValueSet const    &rManifest = NamedValueSet(),
                     TransactionID const    &rTranID   = TransactionID());

protected:
   // no copies
   ServiceBase(const ServiceBase & );
   ServiceBase & operator = (const ServiceBase & );

   //=============================================================================
   // Name: MessageDeliveryThread
   // @brief Polls message queue and dispatches messages.
   static void _MessageDeliveryThread(OSLThread *pThread, void *pContext);
   void MessageDeliveryThread();

   //=============================================================================

   void Released();

   btUnsigned32bitInt m_Flags;
#define SERVICEBASE_IS_RELEASED 0x00000001
   IRuntimeClient    *m_RuntimeClient;
   IRuntime          *m_Runtime;
   IServiceClient    *m_pclient;
   IBase             *m_pclientbase;
   AALServiceModule  *m_pcontainer;
   IAALTransport     *m_ptransport;
   IAALMarshaller    *m_pmarshaller;
   IAALUnMarshaller  *m_punmarshaller;
   btBool             m_runMDT;
   OSLThread         *m_pMDT;
   NamedValueSet      m_optArgs;
};


// Standard messsage keys
#define AAL_SERVICE_PROXY_INTERFACE_NEW            "AAL_SERVICE_PROXY_INTERFACE_NEW"
#define AAL_SERVICE_PROXY_INTERFACE_NEW_OPTARGS    "AAL_SERVICE_PROXY_INTERFACE_NEW_OPTARGS"    // Used to pass NVS opt Args
#define AAL_SERVICE_PROXY_INTERFACE                "AAL_SERVICE_PROXY_INTERFACE"                // Used to reference Proxy Interface
#define AAL_SERVICE_STUB_INTERFACE                 "AAL_SERVICE_STUB_INTERFACE"                 // Used to reference Stub Interface
#define AAL_SERVICE_PROXY_INTERFACE_METHOD         "AAL_SERVICE_PROXY_INTERFACE_METHOD"         // Used for method/response IDs

static const int   eProprietarymethodID=0xf000;

// Generic commands from proxy to the stub
enum aal_service_method_id
{
      eNew=1,                 // create the stub object
      eRemoteInitialized,     // remote is ready to be used
      eGetInterface,          // get the stub object's interface
      eGetInterfaceresp,      // response for GetInterface
      eRelease,               // release the stub object
      eReleaseResp            // response to the release message
};


//=============================================================================
// Name: ServiceProxyBase
/// @brief Base class for all services
//=============================================================================
class AASLIB_API ServiceProxyBase: public ServiceBase
{
public:
   ServiceProxyBase(AALServiceModule *container,
                    IRuntime         *pAALRUNTIME,
                    IAALTransport    *ptransport   = NULL,
                    IAALMarshaller   *marshaller   = NULL,
                    IAALUnMarshaller *unmarshaller = NULL);

   //=============================================================================
   // Name: _init
   // @brief Perform any post creation initialization including establishing
   //               communications.
   //
   //           Only base classes have  _init(). This function is designed to be
   //           called by the factory to perform canonical initialization. The
   //           factory will call the most derived base class' (super base class)
   //           _init() function.  It is the responsibility of the base class to
   //           call its direct ancestor's _init() FIRST to ensure that the class
   //           hiearchy _init() called.
   //=============================================================================
   virtual btBool _init( IBase                   *pclient,
                         TransactionID const      &rtid,
                         NamedValueSet const      &optArgs,
                         CAALEvent                *pcmpltEvent = NULL);
private:
    //=============================================================================
    // Name: Doinit
    /// @brief Real initialization function
    // Comments:
    //=============================================================================
    void Doinit(TransactionID const &rtid);

    std::deque<CAALEvent *> m_cmpltEventQueue;
};

//=============================================================================
// Name: ServiceStubBase
/// @brief Base class for all service stubs
//=============================================================================
class AASLIB_API ServiceStubBase: public ServiceBase
{
public:
   ServiceStubBase(AALServiceModule *container,
                   IRuntime         *pAALRUNTIME,
                   IAALTransport    *ptransport  = NULL,
                   IAALMarshaller   *marshaller  = NULL,
                   IAALUnMarshaller *unmarshaller= NULL);

   //=============================================================================
   // Name: _init
   // @brief Perform any post creation initialization including establishing
   //               communications.
   //           Only base classes have  _init(). This function is designed to be
   //           called by the factory to perform canonical initialization. The
   //           factory will call the most derived base class' (super base class)
   //           _init() function.  It is the responsibility of the base class to
   //           call its direct ancestor's _init() FIRST to ensure that the class
   //           hiearchy _init() called.
   //=============================================================================
   virtual btBool _init( IBase               *pclient,
                         TransactionID const &rtid,
                         NamedValueSet const &optArgs,
                         CAALEvent           *pcmpltEvent = NULL);

private:
   //=============================================================================
   // Name: Doinit
   /// @brief Real initialization function
   // Comments:
   //=============================================================================
   void Doinit(TransactionID const &rtid);

   std::deque<CAALEvent *> m_cmpltEventQueue;
};

/// @}

END_NAMESPACE(AAL)

#endif // __AALSDK_AAS_AALSERVICE_H__

