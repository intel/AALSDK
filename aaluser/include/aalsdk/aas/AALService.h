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
#include <aalsdk/xlRuntime.h>

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

   virtual void  operator()()
   {
      (m_That->*m_ptr)(m_rtid);
   }

protected:
   T             *m_That;
   void          (T::*m_ptr)(TransactionID const &);
   TransactionID const m_rtid;
};


//=============================================================================
///   Base class for software-based AAL Services.
///
///   The ServiceBase class is intended to be inherited by AAL Services.
///   The purpose is to provide the canonical implementation for the Service.
//=============================================================================
class AASLIB_API ServiceBase : public CAASBase, public IAALService
{
public:
/// @brief  Used to create the appropriate Constructor signature for Services derived from ServiceBase.
///
/// @param[in]  C  The name of the class being constructed.
/// @param[in]  P  The name of the parent class being implemented, ie ServiceBase, DeviceServiceBase, etc.
#define DECLARE_AAL_SERVICE_CONSTRUCTOR(C, P) C(AAL::AALServiceModule *container,                             \
                                                AAL::IAALTransport    *ptransport   = NULL,                   \
                                                AAL::IAALMarshaller   *marshaller   = NULL,                   \
                                                AAL::IAALUnMarshaller *unmarshaller = NULL) : P(container,    \
                                                                                                ptransport,   \
                                                                                                marshaller,   \
                                                                                                unmarshaller)

   /// @brief     Constructor called by Service Broker
   /// @param[in] container Pointer to the Service Module
   /// @param[in] ptransport Transport if not in-proc
   /// @param[in] marshaller Marshaller if not in-proc
   /// @param[in] unmarshaller Unmarshaller if not in-proc
   ServiceBase(AALServiceModule *container,
               IAALTransport    *ptransport   = NULL,
               IAALMarshaller   *marshaller   = NULL,
               IAALUnMarshaller *unmarshaller = NULL);

   /// ServiceBase Copy Constructor.
   ServiceBase(ServiceBase const &rother);


   /// ServiceBase Destructor.
   virtual ~ServiceBase();

   // <IAALService>

   /// @brief     Called to release Service and free its resources
   ///
   /// Services derived from ServiceBase are expected to call the base class implementation
   ///  AFTER completing their Release() implementation to insure proper clean-up
   /// @param[in] rTranID TransactionID if not atomic
   /// @param[in] timeout Timeout
   virtual btBool Release(TransactionID const &rTranID, btTime timeout = AAL_INFINITE_WAIT);
   virtual btBool Release(btTime timeout = AAL_INFINITE_WAIT);

   // </IAALService>
#if 0
   // Hook for base classes to perform any post-creation initialization.
   //
   // Only base classes have _init(). This function is designed to be
   // called by the factory to perform canonical initialization. The
   // factory will call the most derived base class' (super base class)
   // _init() function.  It is the responsibility of the base class to
   // call its direct ancestor's _init() FIRST to ensure that the class
   // hierarchy _init() called.
   //
   //
   // This is called BEFORE connection has been established with remote.
   //
   //  The function comes in two flavors.
   //  Client is using event handler method of signaling
   // @param[in]  eventHandler  The event handler for the Service.
   // @param[in]  context       Application-specific context.
   // @param[in]  rtid          TransactionID for routing event responses.
   // @param[in]  optArgs       Optional arguments.
   // @param[in]  pcmpltEvent   Optional completion event. Used internally to
   //  traverse the inheritance hierarchy during initialization.
   virtual IBase * _init(btEventHandler       eventHandler,
                         btApplicationContext context,
                         TransactionID const &rtid,
                         NamedValueSet const &optArgs,
                         CAALEvent           *pcmpltEvent = NULL);

#endif
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
   // TODO
   virtual void   Destroy(void){}
#endif // DEPRECATED

   //
   // IServiceClient - For backward compatibility with < version 4.0
   void messageHandler(const IEvent &rEvent);

   // Accessors for marshalers and transport
   IAALMarshaller   & marshall()   { return *m_pmarshaller;   }
   IAALUnMarshaller & unmarshall() { return *m_punmarshaller; }
   IAALTransport    & recvr()      { return *m_ptransport;    }
   IAALTransport    & sender()     { return *m_ptransport;    }


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
   virtual void processmsg() {}

   //=============================================================================
   // Name: startMDS
   // @brief   Starts the message deliver system.
   //
   //
   // Comments: Used by objects that implement asynchronous message delivery.
   //           This starts a thread that continuously polls the receiver and
   //           when a message is available calls proceemsg().
   //=============================================================================
   virtual btBool startMDS();

   //=============================================================================
   // Name: MessageSeliveryThread
   // Description: polls message queue and  dispatches
   //=============================================================================
   static void _MessageDeliveryThread(OSLThread *pThread, void *pContext);
   void MessageDeliveryThread();

   //--------------------------------------------------
   //                   Utilities
   //--------------------------------------------------
   btBool HasMarshaller()   { return NULL != m_pmarshaller;   }
   btBool HasUnMarshaller() { return NULL != m_punmarshaller; }
   btBool HasTransport()    { return NULL != m_ptransport;    }

   // Hook to allow Services to expose parameters. Default is not supported
   virtual btBool SetParms(NamedValueSet const &rparms){return false;}

   /// Accessor to optional arguments passed during allocateService
   ///
   NamedValueSet const        &OptArgs()     { return m_optArgs;        }

   btEventHandler              Handler()     { return m_eventHandler;   }

   /// Accessor to pointer to the Service's Client's Interface
   IServiceClient * Client()      { return m_pclient;        }

   /// Accessor to pointer to the Service's Client's IBase Interface
   IBase *          ClientBase()  { return m_pclientbase;    }

   /// Accessor to pointer to the Runtime Services
   IXLRuntimeServices * getRuntimeServiceProvider() { return pAALServiceModule()->getRuntimeServiceProvider(); }

   /// Accessor to pointer to the Runtime to be used in ObjectCreated
   ///
   ///   Use getRuntimeServiceProvider to get a usable Runtime pointer
   IRuntime * getRuntime() { return pAALServiceModule()->getRuntime(); }

   /// Accessor to pointer to the Runtime Client
   IRuntimeClient * getRuntimeClient() { return m_RuntimeClient; }

   //---------------------------------
   //  Accessors to utility functions
   //---------------------------------

   /// Enable service to allocate another Service. NOTE: default is the RuntimeClient is NOT notified
   void allocService(IBase                  *pClient,
                     NamedValueSet const    &rManifest = NamedValueSet(),
                     TransactionID const    &rTranID   = TransactionID());

   /// Send a Dispatchable
   /// @param[in]  pMessage  Pointer to IDispatchable.
   void SendMsg(IDispatchable *pMessage);

   /// Convenience function to Queue an event to owner
   /// @param[in]  pMessage  Pointer to Event.
   btBool QueueAASEvent(CAALEvent *pEvent);

   // Convenience function to Queue an event to anyone
   btBool QueueAASEvent(btEventHandler Eventhandler, CAALEvent *pEvent);

   // Queue and event to be dispatched through an object
   btBool QueueAASEvent(btObjectType target, CAALEvent *pEvent);
    
   // Accessor to this Services Service Module
   AALServiceModule     * pAALServiceModule()    { return m_pcontainer; }


   void Released();

protected:
   // operator= not allowed
   ServiceBase & operator = (const ServiceBase & );

   //=============================================================================
   // Name: initComplete
   // Description:
   //=============================================================================
   void initComplete(TransactionID const &rtid);

   NamedValueSet                     m_optArgs;
   IRuntimeClient                   *m_RuntimeClient;
   btEventHandler                    m_eventHandler;
   IServiceClient                   *m_pclient;
   IBase                            *m_pclientbase;
   AALServiceModule                 *m_pcontainer;
   IAALTransport                    *m_ptransport;
   IAALMarshaller                   *m_pmarshaller;
   IAALUnMarshaller                 *m_punmarshaller;
   btBool                            m_runMDT;
   OSLThread                        *m_pMDT;
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
#if 0
    virtual IBase * _init(btEventHandler       eventHandler,
                          btApplicationContext context,
                          TransactionID const &rtid,
                          NamedValueSet const &optArgs,
                          CAALEvent           *pcmpltEvent = NULL);
#endif
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
#if 0
   virtual IBase * _init(btEventHandler       eventHandler,
                         btApplicationContext context,
                         TransactionID const &rtid,
                         NamedValueSet const &optArgs,
                         CAALEvent           *pcmpltEvent = NULL);
#endif

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

