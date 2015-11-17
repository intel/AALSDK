// Copyright (c) 2009-2015, Intel Corporation
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
/// @file AALServiceModule.h
/// @brief Classes, macros and interfaces used by all AAL "Services" that are
///    loaded via the AAL::Factory() core service.
/// @ingroup Services
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHOR: Joseph Grecco, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 06/17/2009     JG       Initial version
/// 07/06/2009     HM       Created virtual destructor for IServiceModule
/// 07/20/2009     AC       Enhance the MICRO to fix a bug for shutdown
/// 08/16/2009     HM       Modified AALBaseProxy::Release() to make the proxy
///                            unusable.
/// 05/22/2010     JG       Added Initialize to IServiceProvider to allow for
///                            a default event handler.
/// 06/02/2011     JG       Added NamedValueSet to Initialize for Service 2.0
/// 06/21/2011     JG       Removed implementation classes like AALBaseProxy.
///                           Added Service 2.0 interfaces
/// 09/01/2011     JG       Replaced ServiceProvider with ServiceModule.
///                         Removed Proxys
/// 09/05/2011     JG       Implemented IServiceModuleCallback for releasing
///                         services without event
/// 09/07/2011     JG       Renamed AALServiceModule.h
///                           Moved AALServiceModule into here
/// 04/23/2012     HM       Added virtual destructor to IAALTransport
/// 07/14/2012     HM       Put virtual dtor in as needed to virtual classes@endverbatim
//****************************************************************************
#ifndef __AALSDK_AAS_AALSERVICEMODULE_H__
#define __AALSDK_AAS_AALSERVICEMODULE_H__
#include <aalsdk/AALTypes.h>                  // __declspec

#include <aalsdk/CAALBase.h>
#include <aalsdk/IServiceClient.h>
#include <aalsdk/osal/OSSemaphore.h>
#include <aalsdk/Runtime.h>

/// @addtogroup Services
/// @{

BEGIN_NAMESPACE(AAL)

typedef enum eservice_connection_types
{
   conn_type_udp,
   conn_type_tcp,
   conn_type_shram
} eservice_connection_types;

//=============================================================================
// @interface IAALTransport
// @brief Transport class interface.
//=============================================================================
class AASLIB_API IAALTransport
{
public:
   virtual ~IAALTransport() {}

   virtual btBool  connectremote(NamedValueSet const & )   = 0;
   virtual btBool waitforconnect(NamedValueSet const & )   = 0;
   virtual btBool     disconnect()                         = 0;
   virtual btcString      getmsg(btWSSize *len)            = 0;
   virtual int            putmsg(btcString , btWSSize len) = 0;
};

//=============================================================================
// @interface IAALMarshalUnMarshallerUtil
// @brief Marshaller class interface.
//=============================================================================
class AASLIB_API IAALMarshalUnMarshallerUtil
{
public:
   virtual ~IAALMarshalUnMarshallerUtil() {}

   virtual ENamedValues   Empty()                                              = 0;
   virtual btBool           Has(btStringKey   Name)                      const = 0;
   virtual ENamedValues  Delete(btStringKey   Name)                            = 0;
   virtual ENamedValues GetSize(btStringKey   Name,  btWSSize    *pSize) const = 0;
   virtual ENamedValues    Type(btStringKey   Name,  eBasicTypes *pType) const = 0;
   virtual ENamedValues GetName(btUnsignedInt index, btStringKey *pName) const = 0;
};

//=============================================================================
// @interface IAALMarshaller
// @brief Marshaller class interface.
//=============================================================================
class AASLIB_API IAALMarshaller : public IAALMarshalUnMarshallerUtil
{
public:
   virtual ~IAALMarshaller() {}

   virtual ENamedValues   Empty()                                                          = 0;
   virtual btBool           Has(btStringKey   Name)                        const           = 0;
   virtual ENamedValues  Delete(btStringKey   Name)                                        = 0;
   virtual ENamedValues GetSize(btStringKey   Name,  btWSSize      *pSize) const           = 0;
   virtual ENamedValues    Type(btStringKey   Name,  eBasicTypes   *pType) const           = 0;
   virtual ENamedValues GetName(btUnsignedInt index, btStringKey   *pName) const           = 0;

   virtual ENamedValues Add(btNumberKey Name, btBool                Value)                 = 0;
   virtual ENamedValues Add(btNumberKey Name, btByte                Value)                 = 0;
   virtual ENamedValues Add(btNumberKey Name, bt32bitInt            Value)                 = 0;
   virtual ENamedValues Add(btNumberKey Name, btUnsigned32bitInt    Value)                 = 0;
   virtual ENamedValues Add(btNumberKey Name, bt64bitInt            Value)                 = 0;
   virtual ENamedValues Add(btNumberKey Name, btUnsigned64bitInt    Value)                 = 0;
   virtual ENamedValues Add(btNumberKey Name, btFloat               Value)                 = 0;
   virtual ENamedValues Add(btNumberKey Name, btcString             Value)                 = 0;
   virtual ENamedValues Add(btNumberKey Name, btObjectType          Value)                 = 0;
   virtual ENamedValues Add(btNumberKey Name, const INamedValueSet *Value)                 = 0;

   virtual ENamedValues Add(btNumberKey Name,
                            btByteArray             Value, btUnsigned32bitInt NumElements) = 0;
   virtual ENamedValues Add(btNumberKey Name,
                            bt32bitIntArray         Value, btUnsigned32bitInt NumElements) = 0;
   virtual ENamedValues Add(btNumberKey Name,
                            btUnsigned32bitIntArray Value, btUnsigned32bitInt NumElements) = 0;
   virtual ENamedValues Add(btNumberKey Name,
                            bt64bitIntArray         Value, btUnsigned32bitInt NumElements) = 0;
   virtual ENamedValues Add(btNumberKey Name,
                            btUnsigned64bitIntArray Value, btUnsigned32bitInt NumElements) = 0;
   virtual ENamedValues Add(btNumberKey Name,
                            btFloatArray            Value, btUnsigned32bitInt NumElements) = 0;
   virtual ENamedValues Add(btNumberKey Name,
                            btStringArray           Value, btUnsigned32bitInt NumElements) = 0;
   virtual ENamedValues Add(btNumberKey Name,
                            btObjectArray           Value, btUnsigned32bitInt NumElements) = 0;

   virtual ENamedValues Add(btStringKey Name, btBool                Value)                 = 0;
   virtual ENamedValues Add(btStringKey Name, btByte                Value)                 = 0;
   virtual ENamedValues Add(btStringKey Name, bt32bitInt            Value)                 = 0;
   virtual ENamedValues Add(btStringKey Name, btUnsigned32bitInt    Value)                 = 0;
   virtual ENamedValues Add(btStringKey Name, bt64bitInt            Value)                 = 0;
   virtual ENamedValues Add(btStringKey Name, btUnsigned64bitInt    Value)                 = 0;
   virtual ENamedValues Add(btStringKey Name, btFloat               Value)                 = 0;
   virtual ENamedValues Add(btStringKey Name, btcString             Value)                 = 0;
   virtual ENamedValues Add(btStringKey Name, btObjectType          Value)                 = 0;
   virtual ENamedValues Add(btStringKey Name, const INamedValueSet *Value)                 = 0;

   virtual ENamedValues Add(btStringKey Name,
                            btByteArray             Value, btUnsigned32bitInt NumElements) = 0;
   virtual ENamedValues Add(btStringKey Name,
                            bt32bitIntArray         Value, btUnsigned32bitInt NumElements) = 0;
   virtual ENamedValues Add(btStringKey Name,
                            btUnsigned32bitIntArray Value, btUnsigned32bitInt NumElements) = 0;
   virtual ENamedValues Add(btStringKey Name,
                            bt64bitIntArray         Value, btUnsigned32bitInt NumElements) = 0;
   virtual ENamedValues Add(btStringKey Name,
                            btUnsigned64bitIntArray Value, btUnsigned32bitInt NumElements) = 0;
   virtual ENamedValues Add(btStringKey Name,
                            btFloatArray            Value, btUnsigned32bitInt NumElements) = 0;
   virtual ENamedValues Add(btStringKey Name,
                            btStringArray           Value, btUnsigned32bitInt NumElements) = 0;
   virtual ENamedValues Add(btStringKey Name,
                            btObjectArray           Value, btUnsigned32bitInt NumElements) = 0;

   virtual char const *pmsgp(btWSSize *len)                                                = 0;
};

//=============================================================================
// @interface IAALUnMarshaller
// @brief UnMarshaller class interface.
//=============================================================================
class AASLIB_API IAALUnMarshaller : public IAALMarshalUnMarshallerUtil
{
public:
   virtual ~IAALUnMarshaller() {}

   virtual ENamedValues   Empty()                                                     = 0;
   virtual btBool           Has(btStringKey   Name)                      const        = 0;
   virtual ENamedValues  Delete(btStringKey   Name)                                   = 0;
   virtual ENamedValues GetSize(btStringKey   Name,  btWSSize    *pSize) const        = 0;
   virtual ENamedValues    Type(btStringKey   Name,  eBasicTypes *pType) const        = 0;
   virtual ENamedValues GetName(btUnsignedInt index, btStringKey *pName) const        = 0;

   virtual ENamedValues Get(btNumberKey Name, btBool                   *pValue) const = 0;
   virtual ENamedValues Get(btNumberKey Name, btByte                   *pValue) const = 0;
   virtual ENamedValues Get(btNumberKey Name, bt32bitInt               *pValue) const = 0;
   virtual ENamedValues Get(btNumberKey Name, btUnsigned32bitInt       *pValue) const = 0;
   virtual ENamedValues Get(btNumberKey Name, bt64bitInt               *pValue) const = 0;
   virtual ENamedValues Get(btNumberKey Name, btUnsigned64bitInt       *pValue) const = 0;
   virtual ENamedValues Get(btNumberKey Name, btFloat                  *pValue) const = 0;
   virtual ENamedValues Get(btNumberKey Name, btcString                *pValue) const = 0;
   virtual ENamedValues Get(btNumberKey Name, btObjectType             *pValue) const = 0;
   virtual ENamedValues Get(btNumberKey Name, INamedValueSet const    **pValue) const = 0;
   virtual ENamedValues Get(btNumberKey Name, btByteArray              *pValue) const = 0;
   virtual ENamedValues Get(btNumberKey Name, bt32bitIntArray          *pValue) const = 0;
   virtual ENamedValues Get(btNumberKey Name, btUnsigned32bitIntArray  *pValue) const = 0;
   virtual ENamedValues Get(btNumberKey Name, bt64bitIntArray          *pValue) const = 0;
   virtual ENamedValues Get(btNumberKey Name, btUnsigned64bitIntArray  *pValue) const = 0;
   virtual ENamedValues Get(btNumberKey Name, btFloatArray             *pValue) const = 0;
   virtual ENamedValues Get(btNumberKey Name, btStringArray            *pValue) const = 0;
   virtual ENamedValues Get(btNumberKey Name, btObjectArray            *pValue) const = 0;

   virtual ENamedValues Get(btStringKey Name, btBool                   *pValue) const = 0;
   virtual ENamedValues Get(btStringKey Name, btByte                   *pValue) const = 0;
   virtual ENamedValues Get(btStringKey Name, bt32bitInt               *pValue) const = 0;
   virtual ENamedValues Get(btStringKey Name, btUnsigned32bitInt       *pValue) const = 0;
   virtual ENamedValues Get(btStringKey Name, bt64bitInt               *pValue) const = 0;
   virtual ENamedValues Get(btStringKey Name, btUnsigned64bitInt       *pValue) const = 0;
   virtual ENamedValues Get(btStringKey Name, btFloat                  *pValue) const = 0;
   virtual ENamedValues Get(btStringKey Name, btcString                *pValue) const = 0;
   virtual ENamedValues Get(btStringKey Name, btObjectType             *pValue) const = 0;
   virtual ENamedValues Get(btStringKey Name, INamedValueSet const    **pValue) const = 0;
   virtual ENamedValues Get(btStringKey Name, btByteArray              *pValue) const = 0;
   virtual ENamedValues Get(btStringKey Name, bt32bitIntArray          *pValue) const = 0;
   virtual ENamedValues Get(btStringKey Name, btUnsigned32bitIntArray  *pValue) const = 0;
   virtual ENamedValues Get(btStringKey Name, bt64bitIntArray          *pValue) const = 0;
   virtual ENamedValues Get(btStringKey Name, btUnsigned64bitIntArray  *pValue) const = 0;
   virtual ENamedValues Get(btStringKey Name, btFloatArray             *pValue) const = 0;
   virtual ENamedValues Get(btStringKey Name, btStringArray            *pValue) const = 0;
   virtual ENamedValues Get(btStringKey Name, btObjectArray            *pValue) const = 0;

   virtual void   importmsg(char const *pmsg, btWSSize len)                           = 0;
};


class AALServiceModule;

/// @interface ISvcsFact
/// @brief AAL Service factory interface.
///
/// The object which implements the ISvcsFact interface understands the intricate
/// details of the Service being created. ISvcsFact is able to instantiate the specific
/// concrete Service instance.
class AASLIB_API ISvcsFact
{
public:
   /// ISvcsFact Destructor.
   virtual ~ISvcsFact() {}

   /// Create an instance of a Service object.
   ///
   /// @param[in]  container  The AALServiceModule which contains this factory.
   /// @param[in]  Client   The interface to receive the serviceAllocated call. The
   ///   object contained within the call may be queried for the desired Service interfaces.
   /// @param[in]  tid        TransactionID for the event.
   /// @param[in]  optArgs    Optional Service-specific arguments.
   ///
   /// @retval  IBase *  On success.
   /// @retval  NULL     On failure.
   virtual IBase * CreateServiceObject(AALServiceModule    *container,
                                       IRuntime            *pRuntime)     = 0;

   // Used to destroy and uninitialized Service Object
   virtual void   DestroyServiceObject(IBase               *pServiceBase) = 0;

   virtual btBool    InitializeService(IBase               *newService,
                                       IBase               *Client,
                                       TransactionID const &rtid,
                                       NamedValueSet const &optArgs)      = 0;
};


/// @interface IServiceModule
/// @brief Interface of objects which implement the Module entry point and access to Service factory.
///
/// The Service Module is the object that is dynamically-loaded from the Service executable.
/// It acts as a container for the concrete ISvcsFact object, which is capable of creating
/// concrete Service instances. It also tracks and synchronizes with each Service instance during
/// shutdown.
class AASLIB_API IServiceModule
{
public:
   /// IServiceModule Destructor.
   virtual ~IServiceModule() {}

   /// Uses ISvcsFact to create the Service object. Note that success of this function
   ///  does not guarantee the object creates successfully. It only indicates that the
   ///  request is being serviced.  Final response is delivered via callback.
   ///
   /// @param[in]  Client  The callback interface to receive the serviceAllocated. The
   ///   object passed in the call may be queried for the desired Service interfaces.
   /// @param[in]  tid       Optional TransactionID for the event.
   /// @param[in]  optArgs   Optional Service-specific arguments.
   ///
   /// @retval  IBase *  On success.
   /// @retval  false     On failure.
   virtual btBool Construct(IRuntime            *pAALRUNTIME,
                            IBase               *Client,
                            TransactionID const &tid = TransactionID(),
                            NamedValueSet const &optArgs = NamedValueSet()) = 0;

   /// Forcefully destroy the Service objects created by this module.
   ///
   /// This is an atomic function as once it returns the executable
   ///  will be deleted. The call stack and all threads within this
   ///  modules address space MUST be deleted.
   virtual void Destroy() = 0;

};


/// @interface IServiceModuleCallback
/// @brief Interface of object notified when a Service instance is destroyed.
class AASLIB_API IServiceModuleCallback
{
public:
   /// IServiceModuleCallback Destructor.
   virtual ~IServiceModuleCallback() {}

   /// Callback invoked by the Service to indicate that it is released.
   /// @param[in]  pService  The Service that has been released.
   virtual void      ServiceReleased(IBase *pService)                            = 0;

   /// Callback invoked by the Service to indicate that it has been initialized.
   /// @param[in]  pService  The Service that has been initialized.
   virtual btBool ServiceInitialized(IBase *pService, TransactionID const &rtid) = 0;

   virtual btBool  ServiceInitFailed(IBase *pService, IEvent const *pEvent)      = 0;
};


/// Abstraction of the dynamic load aspect of an AAL Service.
///
/// The AALServiceModule is the object that is queried from the dynamically-loaded
///  Service executable.
///
/// Used in conjunction with DEFINE_SERVICE_PROVIDER_2_0_ACCESSOR and ISvcsFact to
/// implement the means of Constructing and Destroying an AAL Service.
class AASLIB_API AALServiceModule : public CAASBase,
                                    public IServiceClient,
                                    public IServiceModule,
                                    public IServiceModuleCallback
{
public:
   /// AALServiceModule Constructor.
   ///
   /// Constructed with a concrete instance of the ISvcsFact interface within the
   ///  well-known entry point of the dynamically-loaded Service executable.
   AALServiceModule(ISvcsFact *fact);

   // <IServiceModule>
   virtual btBool Construct(IRuntime            *pRuntime,
                            IBase               *Client,
                            TransactionID const &tid = TransactionID(),
                            NamedValueSet const &optArgs = NamedValueSet());

   virtual void Destroy();
   // </IServiceModule>

   // <IServiceModuleCallback>
   virtual void      ServiceReleased(IBase *pService);
   virtual btBool ServiceInitialized(IBase *pService, TransactionID const &rtid);
   virtual btBool  ServiceInitFailed(IBase *pService, IEvent        const *pEvent);
   // </IServiceModuleCallback>

   // <IServiceClient>
   virtual void      serviceAllocated(IBase               *pServiceBase,
                                      TransactionID const &rTranID = TransactionID());
   virtual void serviceAllocateFailed(const IEvent &rEvent);
   virtual void       serviceReleased(TransactionID const &rTranID = TransactionID());
   virtual void  serviceReleaseFailed(const IEvent &rEvent);
   virtual void          serviceEvent(const IEvent &rEvent);
   // </IServiceClient>

   //=============================================================================
   // Name: AddToServiceList
   /// @brief Adds service to  service list
   /// @param[in] pService  IBase of service
   /// @return false service already registered
   //=============================================================================
   btBool AddToServiceList(IBase *pService);

   //=============================================================================
   // Name: RemovefromServiceList
   /// @brief  Remove service from service list
   /// @param[in] pService - IBase of service
   /// @return false - was not registered
   //=============================================================================
   btBool RemovefromServiceList(IBase *pService);

   //=============================================================================
   // Name: ServiceInstanceRegistered
   /// @brief Determines if a service is already registered
   /// @param[in]   pService IBase of service
   /// @return true if service registered
   //=============================================================================
   btBool ServiceInstanceRegistered(IBase *pService);
private:
   ////////////////////////////////////////////////////////////////////////////
   ////                          Shutdown                                  ////
   ////////////////////////////////////////////////////////////////////////////

   //=============================================================================
   // Name: SendReleaseToAll
   /// @brief Sends a Release to all services
   /// @param[in]   pService IBase of service
   /// 			The object remains locked through the loop of Releases to prevent
   ///           Services being removed in the background and corrupting the iter
   ///           The Release() used here is a quiet one. It does not generate an event.
   ///           This is to allow the Service to cleanly shutdown for unloading.
   //=============================================================================
   void SendReleaseToAll();

protected:

   typedef std::list< IBase * >      list_type;
   typedef list_type::iterator       list_iter;
   typedef list_type::const_iterator list_citer;

   ISvcsFact           *m_SvcsFact;
   IRuntimeClient      *m_RuntimeClient;
   btUnsignedInt        m_pendingcount;
   CSemaphore           m_srvcCount;
#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4251)
#endif // _MSC_VER
   list_type            m_serviceList;
#ifdef _MSC_VER
# pragma warning(pop)
#endif // _MSC_VER
};

END_NAMESPACE(AAL)
/// @}


#endif // __AALSDK_AAS_AALSERVICEMODULE_H__

