// Copyright (c) 2011-2015, Intel Corporation
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
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
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
///                          signaling. @endverbatim
//****************************************************************************
#ifndef __AALSDK_AAS_AALSERVICE_H__
#define __AALSDK_AAS_AALSERVICE_H__
#include <aalsdk/aas/AALServiceModule.h>
#include <aalsdk/osal/Thread.h>
#include <aalsdk/utils/AALEventUtilities.h>
#include <aalsdk/IServiceClient.h>
#include <aalsdk/Runtime.h>

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
/// the InitComplete to schedule their init() behavior. Refer ServiceBase and DeviceServiceBase
/// _init() methods for more detail.
template <class T>
class InitComplete : public CAALEvent
{
public:
   /// InitComplete Constructor.
   ///
   /// @param[in]  That  Object on which the member function ptr is to be invoked on Dispatch.
   /// @param[in]  ptr   The member function to be invoked on Dispatch.
   /// @param[in]  rtid  TransactionID for the event.
   InitComplete(T                                  *That,
                void (T::*ptr)(TransactionID const &rtid),
                TransactionID const                &rtid) :
      m_That(That),
      m_ptr(ptr),
      m_rtid(rtid)
   {}

   virtual void Dispatch(btObjectType target) const
   {
      (m_That->*m_ptr)(m_rtid);
   }

   virtual void operator()()
   {
      (m_That->*m_ptr)(m_rtid);
   }

protected:
   T                  *m_That;
   void           (T::*m_ptr)(TransactionID const &);
   TransactionID const m_rtid;
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
   virtual btBool Release(btTime timeout=AAL_INFINITE_WAIT) = 0;
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
                               public IAALService
{
public:
/// @brief  Used to create the appropriate Constructor signature for Services derived from ServiceBase.
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

   /// @brief     Constructor called by Service Broker
   /// @param[in] container Pointer to the Service Module
   /// @param[in] pAALRUNTIME runtime to use
   /// @param[in] ptransport Transport if not in-proc
   /// @param[in] marshaller Marshaller if not in-proc
   /// @param[in] unmarshaller Unmarshaller if not in-proc
   ServiceBase(AALServiceModule *container,
               IRuntime         *pAALRUNTIME,
               IAALTransport    *ptransport   = NULL,
               IAALMarshaller   *marshaller   = NULL,
               IAALUnMarshaller *unmarshaller = NULL);

   /// ServiceBase Destructor.
   virtual ~ServiceBase();

   // <IAALService> - Empty defaults

   /// @brief     Called to release Service and free its resources
   ///
   /// Services derived from ServiceBase are expected to call the base class implementation
   ///  AFTER completing their Release() implementation to insure proper clean-up
   /// @param[in] rTranID TransactionID if not atomic
   /// @param[in] timeout Timeout
   virtual btBool Release(TransactionID const &rTranID, btTime timeout=AAL_INFINITE_WAIT);

   // Final Release.  This should only be called by the framework or in the case of an unrecoverable error.
   //   This function destroys the Service object.
   virtual btBool Release(btTime timeout=AAL_INFINITE_WAIT);

   // </IAALService>


   // <IRuntimeClient>
   /// @brief     Called by a Runtime object to indicate that it failed to
   ///               successfully allocate a new instance or to return a proxy
   ///               after a call to Runtime() or getRuntimeProxy().
   /// @param[in] rEvent will be an exception event that can be parsed to determine
   ///               the error that occurred.
   virtual void runtimeCreateOrGetProxyFailed(IEvent const & ) {/*to be implemented by the service*/}


   /// @brief     Called by a Runtime object to indicate that it started successfully
   ///               after a call to Runtime.start()
   /// @param[in] pRuntime Pointer to the Runtime object that is calling back
   ///               indicating that it has started successfully after
   ///               Runtime.start() was called.
   /// @param[in] rConfigParms Copy of the configuration parameters passed in to
   ///               Runtime.start() call.
   virtual void runtimeStarted(IRuntime            * ,
                               const NamedValueSet & ) {/*to be implemented by the service*/}

   /// @brief     Called by a Runtime object to indicate that it has stopped successfully
   ///               after a call to Runtime.stop()
   /// @param[in] pRuntime Pointer to the Runtime object that is calling back
   ///               indicating that it has stopped successfully after
   ///               Runtime.stop() was called.
   virtual void runtimeStopped(IRuntime * ) {/*to be implemented by the service*/}

   /// @brief     Called by a Runtime object to indicate that it failed to start
   ///               successfully after a call to Runtime.start().
   ///
   ///            Although not started, the object still exists, and if dynamically
   ///               allocated will still need to be freed.
   /// @param[in] rEvent will be an exception event that can be parsed to determine
   ///               the error that occurred.
   virtual void runtimeStartFailed(const IEvent & ) {/*to be implemented by the service*/}

   /// @brief     Called by a Runtime object to indicate that it failed to stop
   ///               successfully after a call to Runtime.stop().
   ///
   ///            This will usually occur when trying to stop a proxy.
   ///
   /// @param[in] rEvent will be an exception event that can be parsed to determine
   ///               the error that occurred.
   virtual void runtimeStopFailed(const IEvent & ) {/*to be implemented by the service*/}

   /// @brief     Called by a Runtime object to indicate that it failed to
   ///               successfully allocate a service after a call to
   ///               Runtime.allocService().
   /// @param[in] rEvent will be an exception event that can be parsed to determine
   ///               the error that occurred.
   virtual void runtimeAllocateServiceFailed(IEvent const & ) {/*to be implemented by the service*/}

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
   virtual void runtimeAllocateServiceSucceeded(IBase               * ,
                                                TransactionID const & ) {/*to be implemented by the service*/}

   /// @brief     Called by a Runtime object to pass exceptions and other
   ///               unsolicited messages.
   /// @param[in] rEvent will be an event that can be parsed to determine
   ///               what occurred.
   virtual void runtimeEvent(const IEvent & ) {/*to be implemented by the service*/}
   // </IRuntimeClient>

   // @param[in]  pclient       Interface of client of service.
   // @param[in]  rtid          TransactionID for routing event responses.
   // @param[in]  optArgs       Optional arguments.
   // @param[in]  pcmpltEvent   Optional completion event. Used internally to
   //  traverse the inheritance hierarchy during initialization.
   //
   //  Client is using callback interface method of signaling
   virtual IBase * _init(IBase               *pclient,
                         TransactionID const &rtid,
                         NamedValueSet const &optArgs,
                         CAALEvent           *pcmpltEvent = NULL);

   /// Hook for derived classes to perform any post-creation initialization.
   ///
   /// This function is called by the factory AFTER _init(), insuring
   /// that base class initialization has occurred by the time this is called.
   virtual void init(TransactionID const &rtid) = 0;

#if DEPRECATED
   // IServiceClient - For backward compatibility with < version 4.0
   void messageHandler(const IEvent &rEvent);
#endif // DEPRECATED

   //=============================================================================
   // Name: sendmsg
   // @brief Convenience function for sending the message currently carried
   //              by the marshaler.
   //
   //
   // Comments: This is a destructive send. It empties the marshaler once sent.
   //=============================================================================
   btBool sendmsg();

   //=============================================================================
   // Name:        getmsg
   // @brief Convenience function for getting the next message into
   //              unmarshaller
   //
   // Comments:    This is a destructive get. It empties the unmarshaller before
   //              getting.
   // @return      true if data in unmarshaller, false if not
   //=============================================================================
   btBool getmsg();

   //=============================================================================
   // Name: processmsg
   // @brief Process a single received message
   //
   // Comments This will be called by some scheduling entity dependent on the
   //           container
   //=============================================================================
   // TODO
   virtual void processmsg() {/*to be implemented by the service*/}

   //=============================================================================
   // Name: startMDS
   // @brief   Starts the message deliver system.
   //
   //
   // Comments: Used by objects that implement asynchronous message delivery.
   //           This starts a thread that continuously polls the receiver and
   //           when a message is available calls processmsg().
   //=============================================================================
   virtual btBool startMDS();

   //--------------------------------------------------
   //                   Utilities
   //--------------------------------------------------

   // Accessors for marshalers and transport
   IAALMarshaller   &   marshall();
   IAALUnMarshaller & unmarshall();
   IAALTransport    &      recvr();
   IAALTransport    &     sender();

   btBool          HasMarshaller() const;
   btBool        HasUnMarshaller() const;
   btBool           HasTransport() const;

   // Hook to allow Services to expose parameters.
   // Default is to not support parameters (Services can override to implement parameters).
   virtual btBool SetParms(NamedValueSet const & ) { return false; }

   /// Accessor to optional arguments passed during allocateService
   NamedValueSet const &        OptArgs() const;

   /// Accessor to pointer to the Service's Client's Interface
   IServiceClient *              Client() const;

   /// Accessor to pointer to the Service's Client's IBase Interface
   IBase *                   ClientBase() const;

   /// Accessor to pointer to the Runtime to be used in ObjectCreated
   IRuntime *                getRuntime() const;

   /// Accessor to pointer to the Runtime Client
   IRuntimeClient *    getRuntimeClient() const;

   /// Accessor to this Services Service Module
   AALServiceModule * pAALServiceModule() const;

   //---------------------------------
   //  Accessors to utility functions
   //---------------------------------

   /// Enable service to allocate another Service.
   void allocService(IBase                  *pClient,
                     NamedValueSet const    &rManifest = NamedValueSet(),
                     TransactionID const    &rTranID   = TransactionID());

protected:
   // no copies
   ServiceBase(const ServiceBase & );
   ServiceBase & operator = (const ServiceBase & );

   //=============================================================================
   // Name: MessageDeliveryThread
   // Description: polls message queue and dispatches
   //=============================================================================
   static void _MessageDeliveryThread(OSLThread *pThread, void *pContext);
   void MessageDeliveryThread();

   //=============================================================================
   // Name: initComplete
   // Description:
   //=============================================================================
   void initComplete(TransactionID const &rtid);

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
#define AAL_SERVICE_PROXY_INTERFACE_NEW_OPTARGS    "AAL_SERVICE_PROXY_INTERFACE_NEW_OPTARGS"    // USed to pass NVS opt Args
#define AAL_SERVICE_PROXY_INTERFACE                "AAL_SERVICE_PROXY_INTERFACE"                // Used to reference Proxy Interface
#define AAL_SERVICE_STUB_INTERFACE                 "AAL_SERVICE_STUB_INTERFACE"                 // Used to reference Stub Interface
#define AAL_SERVICE_PROXY_INTERFACE_METHOD         "AAL_SERVICE_PROXY_INTERFACE_METHOD"         // Used for method/response IDs

static const int   eProprietarymethodID=0xf000;

// Generic commands from proxy to the stub
enum aal_service_method_id
{
      eNew=1,                 // create the stub object
      eGetInterface,          // get the stub object's interface
      eGetInterfaceresp,      // response for GetInterface
      eRelease,               // release the stub object
      eReleaseResp            // response to the release message
};


//=============================================================================
// Name: ServiceProxyBase
// Description: Base class for all services
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
   // Description: Perform any post creation initialization including establishing
   //               communications.
   // Comments: Only base classes have  _init(). This function is designed to be
   //           called by the factory to perform canonical initialization. The 
   //           factory will call the most derived base class' (super base class)
   //           _init() function.  It is the responsibility of the base class to
   //           call its direct ancestor's _init() FIRST to ensure that the class
   //           hiearchy _init() called.
   //=============================================================================
   virtual IBase * _init(IBase                   *pclient,
                          TransactionID const      &rtid,
                          NamedValueSet const      &optArgs,
                          CAALEvent                *pcmpltEvent = NULL);
private:
    //=============================================================================
    // Name: Doinit
    // Description: Real initialization function
    // Comments:
    //=============================================================================
    void Doinit(TransactionID const &rtid);

    CAALEvent *m_pcmpltEvent;
};

//=============================================================================
// Name: ServiceStubBase
// Description: Base class for all service stubs
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
   // Name: init
   // Description: Perform any post creation initialization including establishing
   //               communications.
   // Comments: Only base classes have  _init(). This function is designed to be
   //           called by the factory to perform canonical initialization. The 
   //           factory will call the most derived base class' (super base class)
   //           _init() function.  It is the responsibility of the base class to
   //           call its direct ancestor's _init() FIRST to ensure that the class
   //           hiearchy _init() called.
   //=============================================================================
   virtual IBase * _init(IBase               *pclient,
                         TransactionID const &rtid,
                         NamedValueSet const &optArgs,
                         CAALEvent           *pcmpltEvent = NULL);

private:
   //=============================================================================
   // Name: Doinit
   // Description: Real initialization function
   // Comments:
   //=============================================================================
   void Doinit(TransactionID const &rtid);

   CAALEvent *m_pcmpltEvent;
};

/// @} group Services

END_NAMESPACE(AAL)

#endif // __AALSDK_AAS_AALSERVICE_H__

