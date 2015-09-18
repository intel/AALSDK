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
 // Name: IAALTransport
 // Description: trasnport class interface
 //=============================================================================
///
class AASLIB_API IAALTransport
{
public:
   virtual ~IAALTransport();

   virtual btBool    connectremote(NamedValueSet const & )  = 0;
   virtual btBool    waitforconnect(NamedValueSet const & ) = 0;
   virtual btBool    disconnect()                           = 0;
   virtual btcString getmsg(btWSSize *len)                  = 0;
   virtual int       putmsg(btcString, btWSSize len)        = 0;
};

 //=============================================================================
 // Name: IAALMarshalUnMarshallerUtil
 // Description: Marshaller class interface
 //=============================================================================
class AASLIB_API IAALMarshalUnMarshallerUtil
{
public:
   virtual ~IAALMarshalUnMarshallerUtil();

   virtual ENamedValues Empty()                                                    = 0;
   virtual btBool       Has(btcString Name)                                  const = 0;
   virtual ENamedValues Delete(btStringKey Name)                                   = 0;
   virtual ENamedValues GetSize(btStringKey Name, btUnsigned32bitInt *pSize) const = 0;
   virtual ENamedValues Type(btStringKey Name, eBasicTypes *pType)           const = 0;
   virtual ENamedValues GetName(btUnsignedInt index, btStringKey *pName)     const = 0;
};

 //=============================================================================
 // Name: IAALMarshaller
 // Description: Marshaller class interface
 //=============================================================================
class AASLIB_API IAALMarshaller : public IAALMarshalUnMarshallerUtil
{
public:
   virtual ~IAALMarshaller();

   virtual ENamedValues Empty()                                                    = 0;
   virtual btBool       Has(btcString Name)                                  const = 0;
   virtual ENamedValues Delete(btStringKey Name)                                   = 0;
   virtual ENamedValues GetSize(btStringKey Name, btUnsigned32bitInt *pSize) const = 0;
   virtual ENamedValues Type(btStringKey Name, eBasicTypes *pType)           const = 0;
   virtual ENamedValues GetName(btUnsignedInt index, btStringKey *pName)     const = 0;

   virtual ENamedValues Add( btNumberKey  Name,btBool value)=0;
   virtual ENamedValues Add( btNumberKey  Name,btByte value)=0;
   virtual ENamedValues Add( btNumberKey  Name,bt32bitInt value)=0;
   virtual ENamedValues Add( btNumberKey  Name,btUnsigned32bitInt  value)=0;
   virtual ENamedValues Add( btNumberKey  Name,bt64bitInt value)=0;
   virtual ENamedValues Add( btNumberKey  Name,btUnsigned64bitInt value)=0;
   virtual ENamedValues Add( btNumberKey  Name,btFloat value)=0;
   virtual ENamedValues Add( btNumberKey  Name,btcString value)=0;
   virtual ENamedValues Add( btNumberKey  Name,NamedValueSet const &value)=0;
   virtual ENamedValues Add( btNumberKey  Name,btByteArray value,
                             btUnsigned32bitInt NumElements)=0;
   virtual ENamedValues Add( btNumberKey  Name,bt32bitIntArray value,
                             btUnsigned32bitInt NumElements)=0;
   virtual ENamedValues Add( btNumberKey  Name,btUnsigned32bitIntArray  value,
                             btUnsigned32bitInt NumElements)=0;
   virtual ENamedValues Add( btNumberKey  Name,bt64bitIntArray value,
                             btUnsigned32bitInt NumElements)=0;
   virtual ENamedValues Add( btNumberKey  Name,btUnsigned64bitIntArray value,
                             btUnsigned32bitInt NumElements)=0;
   virtual ENamedValues Add( btNumberKey  Name,btObjectType value)=0;

   virtual ENamedValues Add( btNumberKey  Name,btFloatArray value,
                             btUnsigned32bitInt NumElements)=0;
   virtual ENamedValues Add( btNumberKey  Name,btStringArray value,
                             btUnsigned32bitInt NumElements)=0;
   virtual ENamedValues Add( btNumberKey  Name,btObjectArray value,
                             btUnsigned32bitInt  NumElements)=0;

   virtual ENamedValues Add( btStringKey  Name,btBool value)=0;
   virtual ENamedValues Add( btStringKey  Name,btByte value)=0;
   virtual ENamedValues Add( btStringKey  Name,bt32bitInt value)=0;
   virtual ENamedValues Add( btStringKey  Name,btUnsigned32bitInt  value)=0;
   virtual ENamedValues Add( btStringKey  Name,bt64bitInt value)=0;
   virtual ENamedValues Add( btStringKey  Name,btUnsigned64bitInt value)=0;
   virtual ENamedValues Add( btStringKey  Name,btFloat value)=0;
   virtual ENamedValues Add( btStringKey  Name,btcString value)=0;
   virtual ENamedValues Add( btStringKey  Name,NamedValueSet const &value)=0;
   virtual ENamedValues Add( btStringKey  Name,btByteArray value,
                             btUnsigned32bitInt NumElements)=0;
   virtual ENamedValues Add( btStringKey  Name,bt32bitIntArray value,
                             btUnsigned32bitInt NumElements)=0;
   virtual ENamedValues Add( btStringKey  Name,btUnsigned32bitIntArray  value,
                             btUnsigned32bitInt NumElements)=0;
   virtual ENamedValues Add( btStringKey  Name,bt64bitIntArray value,
                             btUnsigned32bitInt NumElements)=0;
   virtual ENamedValues Add( btStringKey  Name,btUnsigned64bitIntArray value,
                             btUnsigned32bitInt NumElements)=0;
   virtual ENamedValues Add( btStringKey  Name,btObjectType value)=0;

   virtual ENamedValues Add( btStringKey  Name,btFloatArray value,
                             btUnsigned32bitInt NumElements)=0;
   virtual ENamedValues Add( btStringKey  Name,btStringArray value,
                             btUnsigned32bitInt NumElements)=0;
   virtual ENamedValues Add( btStringKey  Name,btObjectArray value,
                             btUnsigned32bitInt  NumElements)=0;

   virtual char const *pmsgp(btWSSize *len)=0;
};

 //=============================================================================
 // Name: IAALUnMarshaller
 // Description: UnMarshaller class interface
 //=============================================================================
class AASLIB_API IAALUnMarshaller :public IAALMarshalUnMarshallerUtil
{
public:
   virtual ~IAALUnMarshaller();

   virtual ENamedValues Empty()                                                    = 0;
   virtual btBool       Has(btcString Name)                                  const = 0;
   virtual ENamedValues Delete(btStringKey Name)                                   = 0;
   virtual ENamedValues GetSize(btStringKey Name, btUnsigned32bitInt *pSize) const = 0;
   virtual ENamedValues Type(btStringKey Name, eBasicTypes *pType)           const = 0;
   virtual ENamedValues GetName(btUnsignedInt index, btStringKey *pName)     const = 0;

   virtual ENamedValues Get( btNumberKey  Name,btBool *pValue)const=0;
   virtual ENamedValues Get( btNumberKey  Name,btByte *pValue)const=0;
   virtual ENamedValues Get( btNumberKey  Name,bt32bitInt *pValue)const=0;
   virtual ENamedValues Get( btNumberKey  Name,btUnsigned32bitInt  *pValue)const=0;
   virtual ENamedValues Get( btNumberKey  Name,bt64bitInt *pValue)const=0;
   virtual ENamedValues Get( btNumberKey  Name,btUnsigned64bitInt *pValue)const=0;
   virtual ENamedValues Get( btNumberKey  Name,btFloat *pValue)const=0;
   virtual ENamedValues Get( btNumberKey  Name,btcString *pValue)const=0;
   virtual ENamedValues Get( btNumberKey  Name,NamedValueSet const**pValue)const=0;
   virtual ENamedValues Get( btNumberKey  Name,btByteArray *pValue)const=0;
   virtual ENamedValues Get( btNumberKey  Name,bt32bitIntArray *pValue)const=0;
   virtual ENamedValues Get( btNumberKey  Name,btUnsigned32bitIntArray  *pValue)const=0;
   virtual ENamedValues Get( btNumberKey  Name,bt64bitIntArray *pValue)const=0;
   virtual ENamedValues Get( btNumberKey  Name,btUnsigned64bitIntArray *pValue)const=0;
   virtual ENamedValues Get( btNumberKey  Name,btObjectType *pValue)const=0;
   virtual ENamedValues Get( btNumberKey  Name,btFloatArray *pValue)const=0;
   virtual ENamedValues Get( btNumberKey  Name,btStringArray *pValue)const=0;
   virtual ENamedValues Get( btNumberKey  Name,btObjectArray *pValue)const=0;

   virtual ENamedValues Get( btStringKey  Name,btBool *pValue)const=0;
   virtual ENamedValues Get( btStringKey  Name,btByte *pValue)const=0;
   virtual ENamedValues Get( btStringKey  Name,bt32bitInt *pValue)const=0;
   virtual ENamedValues Get( btStringKey  Name,btUnsigned32bitInt  *pValue)const=0;
   virtual ENamedValues Get( btStringKey  Name,bt64bitInt *pValue)const=0;
   virtual ENamedValues Get( btStringKey  Name,btUnsigned64bitInt *pValue)const=0;
   virtual ENamedValues Get( btStringKey  Name,btFloat *pValue)const=0;
   virtual ENamedValues Get( btStringKey  Name,btcString *pValue)const=0;
   virtual ENamedValues Get( btStringKey  Name,NamedValueSet const**pValue)const=0;
   virtual ENamedValues Get( btStringKey  Name,btByteArray *pValue)const=0;
   virtual ENamedValues Get( btStringKey  Name,bt32bitIntArray *pValue)const=0;
   virtual ENamedValues Get( btStringKey  Name,btUnsigned32bitIntArray  *pValue)const=0;
   virtual ENamedValues Get( btStringKey  Name,bt64bitIntArray *pValue)const=0;
   virtual ENamedValues Get( btStringKey  Name,btUnsigned64bitIntArray *pValue)const=0;
   virtual ENamedValues Get( btStringKey  Name,btObjectType *pValue)const=0;
   virtual ENamedValues Get( btStringKey  Name,btFloatArray *pValue)const=0;
   virtual ENamedValues Get( btStringKey  Name,btStringArray *pValue)const=0;
   virtual ENamedValues Get( btStringKey  Name,btObjectArray *pValue)const=0;

   virtual void importmsg(char const * pmsg, btWSSize len)=0;
};


class AALServiceModule;
/// AAL Service factory interface.
///
/// The object which implements the ISvcsFact interface understands the intricate
/// details of the Service being created. ISvcsFact is able to instantiate the specific
/// concrete Service instance.
class AASLIB_API ISvcsFact
{
public:
   /// ISvcsFact Destructor.
   virtual ~ISvcsFact();

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
                                       IRuntime            *pRuntime) = 0;

   // Used to destroy and uninitialized Service Object
   virtual void DestroyServiceObject(IBase    *pServiceBase) = 0;

   virtual btBool InitializeService( IBase               *newService,
                                     IBase               *Client,
                                     TransactionID const &rtid,
                                     NamedValueSet const &optArgs) = 0;
};


/// Interface of objects which implement the Module entry point and access to Service factory.
///
/// The Service Module is the object that is dynamically-loaded from the Service executable.
/// It acts as a container for the concrete ISvcsFact object, which is capable of creating
/// concrete Service instances. It also tracks and synchronizes with each Service instance during
/// shutdown.
class AASLIB_API IServiceModule
{
public:
   /// IServiceModule Destructor.
   virtual ~IServiceModule();

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
   virtual void   Destroy() = 0;


   virtual void setRuntime(IRuntime *pRuntime) = 0;
   virtual IRuntime  *getRuntime() = 0;
};


/// Interface of object notified when a Service instance is destroyed.
class AASLIB_API IServiceModuleCallback
{
public:
   /// IServiceModuleCallback Destructor.
   virtual ~IServiceModuleCallback();
   /// Callback invoked by the Service to indicate that it is released.
   /// @param[in]  pService  The Service that has been released.
   virtual void ServiceReleased(IBase *pService) = 0;

   /// Callback invoked by the Service to indicate that it has been initialized.
   /// @param[in]  pService  The Service that has been initialized.
   virtual void ServiceInitialized(IBase *pService) = 0;
};


/// Abstraction of the dynamic load aspect of an AAL Service.
///
/// The AALServiceModule is the object that is queried from the dynamically-loaded
///  Service executable.
///
/// Used in conjunction with DEFINE_SERVICE_PROVIDER_2_0_ACCESSOR and ISvcsFact to
/// implement the means of Constructing and Destroying an AAL Service.
class AASLIB_API AALServiceModule : public CAASBase,
                                    public IServiceModule,
                                    public IServiceModuleCallback
{
public:
   /// AALServiceModule Constructor.
   ///
   /// Constructed with a concrete instance of the ISvcsFact interface within the
   ///  well-known entry point of the dynamically-loaded Service executable.
   AALServiceModule(ISvcsFact &fact);
   /// AALServiceModule Destructor.
   virtual ~AALServiceModule();

   // <IServiceModule>

    virtual btBool Construct( IRuntime            *pAALRUNTIME,
                              IBase               *Client,
                              TransactionID const &tid = TransactionID(),
                              NamedValueSet const &optArgs = NamedValueSet());
   virtual void Destroy();

   // </IServiceModule>

   // <IServiceModuleCallback>
   virtual void ServiceReleased(IBase *pService);
   virtual void ServiceInitialized(IBase *pService);
   // </IServiceModuleCallback>

   void setRuntime(IRuntime *pRuntime)
   {
      // Lock
      m_Runtime = pRuntime;
   }

   // AAL 4.0 Service Interface
   IRuntime *getRuntime()
   {
      // Lock
      return m_Runtime;
   }

   IRuntimeClient *getRuntimeClient()
   {
      // Lock
      return m_RuntimeClient;
   }

   void setRuntimeClient(IRuntimeClient *pRTC)
   {
      // Lock
      m_RuntimeClient = pRTC;
      return ;
   }


private:
   //=============================================================================
   // Name: AddToServiceList
   // Description: Add service to  service list
   //      Inputs: pService - IBase of service
   //     Returns: false - service already registered
   //=============================================================================
   btBool AddToServiceList(IBase *pService);

   //=============================================================================
   // Name: RemovefromServiceList
   // Description: Add servcie to  service list
   //      Inputs: pService - IBase of service
   //     Returns: false - was not registered
   //=============================================================================
   btBool RemovefromServiceList(IBase *pService);

   //=============================================================================
   // Name: ServiceInstanceRegistered
   // Description: Determine if a service is already registerd
   //      Inputs: pService - IBase of service
   //     Returns: true  - service registered
   //=============================================================================
   btBool ServiceInstanceRegistered(IBase *pService);

   ////////////////////////////////////////////////////////////////////////////
   ////                          Shutdown                                  ////
   ////////////////////////////////////////////////////////////////////////////

   //=============================================================================
   // Name: SendReleaseToAll
   // Description: Send a Release to all services
   //      Inputs: pService - IBase of service
   // Comments: The object remains locked through the loop of Releases to prevent
   //           Services being removed in the background and corrupting the iter
   //           The Release() used here is a quiet one. It does not generate an event.
   //           This is to allow the Service to cleanly shutdown for unloading.
   //=============================================================================
   void SendReleaseToAll();

protected:
   typedef std::map<IBase *, IBase *>                 list_type;
   typedef std::map<IBase *, IBase *>::const_iterator const_iterator;

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4251)
#endif // _MSC_VER
   list_type             m_serviceList;
#ifdef _MSC_VER
# pragma warning(pop)
#endif // _MSC_VER
   IAALService                            *m_pService;
   btUnsignedInt                           m_pendingcount;

   IRuntime                               *m_Runtime;
   IRuntimeClient                         *m_RuntimeClient;
   ISvcsFact                              &m_SvcsFact;
   CSemaphore                              m_srvcCount;
};


END_NAMESPACE(AAL)


//=============================================================================
// Name: DECLARE_SERVICE_PROVIDER_ACCESSOR
// Description: Implements the Service Provider accessor for the package.
//              Add this macro to the Service Package source module to enable
//              loading by factory.
// Interface: public
// Inputs: - Class name.
// Outputs: Pointer to the service provider.
// Comments: Macro expands to generate code for type _CLASSNAME_.
//           _CLASSNAME must be the exact name of the Factory class.
//           The _CLASSNAME_ provider MUST be derived from IBase and must be a
//           fully implemented IBase (e.g., CAASBase). The _CLASSNAME_ provider
//           MAY implement IServiceModule as an aggregated object.
//           Note that dynamic_cast<> is used on the provider object to get the
//           IBase interface pointer but dynamic_ptr<> is used to get
//           IServiceModule.
//=============================================================================

#define DECLARE_SERVICE_PROVIDER_ACCESSOR "_ServiceModule"

#define DEFINE_SERVICE_PROVIDER_ACCESSOR(_CLASSNAME_) extern "C" {                                            \
__declspec(dllexport) AAL::IServiceModule * _ServiceModule(AAL::btEventHandler         eventHandler,          \
                                                           AAL::TransactionID const   &tranID,                \
                                                           AAL::btApplicationContext   context,               \
                                                           AAL::CAASServiceContainer  *ServiceContainer,      \
                                                           AAL::IBase                **ppService,             \
                                                           AAL::NamedValueSet const   &optArgs)               \
{                                                                                                             \
   static _CLASSNAME_ theServiceProvider;                                                                     \
   theServiceProvider.Construct(eventHandler, tranID, context);                                               \
   if ( !theServiceProvider.IsOK() ) {                                                                        \
      return NULL;                                                                                            \
   }                                                                                                          \
   theServiceProvider.ServiceContainer(ServiceContainer);                                                     \
   *ppService = dynamic_cast<IBase *>(&theServiceProvider);                                                   \
   return dynamic_ptr<AAL::IServiceModule>(iidServiceProvider, *ppService);                                   \
}                                                                                                             \
                                                                                                              \
__declspec(dllexport) unsigned long _GetServiceProviderVersion()                                              \
{                                                                                                             \
   return AAL_Proxy_Interface_Version;                                                                        \
}                                                                                                             \
}

/// Declares an instance of the well-known Service executable entry point.
///
/// AAL Services use this macro to define the well-known entry point.
///
/// @param[in]  N  The type of Service factory used by the module. N must be a type derived from
///   ISvcsFact.
#define DEFINE_SERVICE_PROVIDER_2_0_ACCESSOR(N) extern "C" {                                                  \
static N ServiceFactory;                                                                                      \
__declspec(dllexport) AAL::IServiceModule * _ServiceModule(AAL::CAASServiceContainer *AASServiceContainer)    \
{                                                                                                             \
   static AAL::AALServiceModule theServiceProvider(ServiceFactory);                                           \
   if ( !theServiceProvider.IsOK() ) {                                                                        \
      return NULL;                                                                                            \
   }                                                                                                          \
   theServiceProvider.ServiceContainer(AASServiceContainer);                                                  \
   return dynamic_ptr<AAL::IServiceModule>(iidServiceProvider, &theServiceProvider);                          \
}                                                                                                             \
}


/// @} group Services


#endif // __AALSDK_AAS_AALSERVICEMODULE_H__

