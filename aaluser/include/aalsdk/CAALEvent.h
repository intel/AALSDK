// Copyright (c) 2006-2015, Intel Corporation
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
/// @file CAALEvent.h
/// @brief Definition of Event base classes.
/// @ingroup Events
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Joseph Grecco, Intel Corporation
///          Henry Mitchel, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 01/04/2006     JG       Initial version started
/// 02/14/2007     JG       Refactored event base class to
///                         remove implementation inheritance
///                         of I_LRObjectInterface.
///                         Fixed a bug in subclass ==() and !=()
///                         operators which caused recursion
/// 03/21/2007     JG       Ported to Linux
/// 04/24/2007     JG       Made all events carry IBase pointers
/// 05/28/2007     JG       Added cached interfaces (SubClass)
/// 06/08/2007     JG       Converted to btID keys
/// 07/15/2007     JG       Converted events to btID
/// 09/25/2007     JG       Began refactoring out legacy code
/// 10/04/2007     JG       Renamed to CAALEvent.h
/// 12/16/2007     JG       Changed include path to expose aas/
/// 02/03/2008     HM       Cleaned up various things brought out by ICC
/// 02/25/2008     HM       Made Parms be NamedValueSet instead of INamedValueSet
/// 05/08/2008     HM       Comments & License
/// 12/10/2008     HM       Changed IEvent.Context(), no longer caches
/// 01/04/2009     HM       Updated Copyright
/// 02/25/2009     HM       Re-enabled on IEvent.Context() to cache. This is
///                            needed in the case of an object sending a
///                            message and then deleting itself; the Context
///                            is still valid (related to the object receiving
///                            the message, not the object throwing it).
/// 04/22/2012     HM       Disabled irrelevant warning about export
///                            of CAALEvent::m_InterfaceMap for _WIN32@endverbatim
//****************************************************************************
#ifndef __AALSDK_CAALEVENT_H__
#define __AALSDK_CAALEVENT_H__
#include <aalsdk/AALEvent.h>
#include <aalsdk/osal/CriticalSection.h>
#include <aalsdk/CAALBase.h>
#include <aalsdk/CCountedObject.h>
#include <aalsdk/osal/IDispatchable.h>
#include <aalsdk/IMessageHandler.h>
#include <aalsdk/IServiceClient.h>
#include <aalsdk/Runtime.h>

#ifdef __ICC                           /* Deal with Intel compiler-specific overly sensitive remarks */
     #pragma warning( push)
//   #pragma warning( pop)
//   #pragma warning(disable:68)       // warning: integer conversion resulted in a change of sign.
                                       //    This is intentional
//   #pragma warning(disable:177)      // remark: variable "XXXX" was declared but never referenced- OK
//   #pragma warning(disable:383)      // remark: value copied to temporary, reference to temporary used
//   #pragma warning(disable:593)      // remark: variable "XXXX" was set but never used - OK
     #pragma warning(disable:869)      // remark: parameter "XXXX" was never referenced
//   #pragma warning(disable:981)      // remark: operands are evaluated in unspecified order
//   #pragma warning(disable:1418)     // remark: external function definition with no prior declaration
//   #pragma warning(disable:1419)     // remark: external declaration in primary source file
//   #pragma warning(disable:1572)     // remark: floating-point equality and inequality comparisons are unreliable
//   #pragma warning(disable:1599)     // remark: declaration hides variable "Args", or tid - OK
#endif


BEGIN_NAMESPACE(AAL)

/// @addtogroup Events
/// @{

/******************************************************************************
*                      Event Base class implementations
* The following are concrete base class implementations of the varios event
* types. Specialized events can derive from these and extend the interfaces
* but often simply inherit the implemention and set the SubclassID to the
* their btIID where no new methods are required.
******************************************************************************/

/// Concrete implementation of IEvent.
class AASLIB_API CAALEvent : protected CriticalSection,
                             public CCountedObject,
                             public IDispatchable,
                             public IEvent
{
public:
   /// CAALEvent construct from IBase *. Sub-class interface id is iidEvent.
   CAALEvent(IBase *pObject, IMessageHandler *pHandler=NULL);
   /// CAALEvent construct from IBase * and sub-class ID.
   CAALEvent(IBase *pObject, btIID SubClassID, IMessageHandler *pHandler=NULL);
   /// CAALEvent Copy Constructor. Sub-class interface id is iidEvent.
   CAALEvent(const CAALEvent &rOther);

   // <IEvent>
   virtual btGenericInterface   Interface(btIID Interface) const;
   virtual btBool               Has(btIID Interface)       const;
   virtual btGenericInterface   ISubClass()                const;
   virtual btIID                SubClassID()               const;
   virtual btBool               operator != (IEvent &rOther);
   virtual btBool               operator == (IEvent &rOther); ///< Equality is defined here as instance exact.
   virtual IBase &              Object()                   const { return *m_pObject; }
   virtual IBase *              pObject()                  const { return m_pObject;  }
   virtual btBool               IsOK()                     const { return m_bIsOK; }
   virtual btApplicationContext Context()                  const { return m_Context; } // Re-enabled HM 20090225, see file header comments
   virtual void                 setHandler(IServiceClient  *pHandler) { m_pServiceClient = pHandler; m_pRuntimeClient = NULL;     m_pEventHandler = NULL;     m_pMessageHandler = NULL;     }
   virtual void                 setHandler(IRuntimeClient  *pHandler) { m_pServiceClient = NULL;     m_pRuntimeClient = pHandler; m_pEventHandler = NULL;     m_pMessageHandler = NULL;     }
   virtual void                 setHandler(btEventHandler   pHandler) { m_pServiceClient = NULL;     m_pRuntimeClient = NULL;     m_pEventHandler = pHandler; m_pMessageHandler = NULL;     }
   virtual void                 setHandler(IMessageHandler *pHandler) { m_pServiceClient = NULL;     m_pRuntimeClient = NULL;     m_pEventHandler = NULL;     m_pMessageHandler = pHandler; }
   virtual btApplicationContext SetContext(btApplicationContext Ctx)
   {
      btApplicationContext res = m_Context;
      m_Context = Ctx;
      return res;
   }
   // </IEvent>

   /// Set the native subclass interface for this Event.
   ///
   /// @param[in]  Interface   The native subclass interface id.
   /// @param[in]  pInterface  The interface pointer.
   ///
   /// @retval  EObjOK  On success.
   EOBJECT SetSubClassInterface(btIID              Interface,
                                btGenericInterface pInterface);

   /// Update the object pointed to by the Event, and its associated cached context.
   void SetObject(IBase *pObject);  // needed for ReThrow

   /// Dispatch self to target.
   ///
   /// @param[in]  target  Assumed to be of type EventHandler.
   ///             Functors can be created by overriding this function
   virtual void Dispatch(btObjectType target) const;

   virtual void  operator()();

   /// Deletes this.
   virtual void Delete();

protected:
   void UpdateContext();   // Update m_Context based on m_pObject

   EOBJECT SetInterface(btIID              Interface,
                        btGenericInterface pInterface);

   btBool ProcessEventTranID();

   CAALEvent();
   CAALEvent & operator = (const CAALEvent & );
   virtual ~CAALEvent();

protected:
   IBase                   *m_pObject;
   btBool                   m_bIsOK;
   btApplicationContext     m_Context;
   IMessageHandler         *m_pMessageHandler;
   IServiceClient          *m_pServiceClient;
   IRuntimeClient          *m_pRuntimeClient;
   btEventHandler           m_pEventHandler;
   TransactionID            m_TranID;  // Only accessible outside from TransactionEvents

#if defined( __AAL_WINDOWS__ )
# pragma warning( push )
# pragma warning( disable:4251 )  // Cannot export template definitions
#endif
   iidInterfaceMap_t    m_InterfaceMap;
#if defined( __AAL_WINDOWS__ )
# pragma warning( pop )
#endif

   btGenericInterface   m_ISubClass;
   btIID                m_SubClassID;
};


/// Concrete implementation of ITransactionEvent.
///
/// CTransactionEvent serves as the base class for all events requiring an event response.
class AASLIB_API CTransactionEvent : public CAALEvent, public ITransactionEvent
{
public:
   /// CTransactionEvent Constructor.
   ///
   /// The native sub-class id is set to iidTranEvent.
   ///
   /// @param[in]  pObject     An IBase associated with this event.
   /// @param[in]  rTranID     For routing event responses.
   /// @param[in]  pHandler    For specific routing
   CTransactionEvent(IBase               *pObject,
                     TransactionID const &rTranID,
                     IMessageHandler *pHandler=NULL);
   /// CTransactionEvent Constructor.
   ///
   /// @param[in]  pObject     An IBase associated with this event.
   /// @param[in]  SubClassID  The native sub-class id for this event.
   /// @param[in]  rTranID     For routing event responses.
   /// @param[in]  pHandler    For specific routing
   CTransactionEvent(IBase               *pObject,
                     btIID                SubClassID,
                     TransactionID const &rTranID,
                     IMessageHandler *pHandler=NULL);
   /// CTransactionEvent Copy Constructor.
   CTransactionEvent(CTransactionEvent const &rOther);
   /// CTransactionEvent Destructor.
   virtual ~CTransactionEvent();

   // <ITransactionEvent>
   TransactionID TranID()                      { return m_TranID;   }
   void SetTranID(TransactionID const &TranID) { m_TranID = TranID; }
   // </ITransactionEvent>

protected:

   // These Constructor forms are invalid.
   CTransactionEvent();
   CTransactionEvent(IBase * );

   CTransactionEvent & operator = (const CTransactionEvent & );
};


/// Concrete implementation of IExceptionEvent.
class AASLIB_API CExceptionEvent : public CAALEvent, public IExceptionEvent
{
public:
   /// CExceptionEvent Constructor.
   ///
   /// The native sub-class id is set to iidExEvent.
   ///
   /// @param[in]  pObject          An IBase associated with this event.
   /// @param[in]  ExceptionNumber  Numeric id for the exception.
   /// @param[in]  Reason           Numeric reason code.
   /// @param[in]  Description      A textual description of the exception.
   /// @param[in]  pHandler         For specific routing
   CExceptionEvent(IBase           *pObject,
                   btID             ExceptionNumber,
                   btID             Reason,
                   btcString        Description,
                   IMessageHandler *pHandler=NULL);
   /// CExceptionEvent Constructor.
   ///
   /// @param[in]  pObject          An IBase associated with this event.
   /// @param[in]  SubClassID       The native sub-class id for this event.
   /// @param[in]  ExceptionNumber  Numeric id for the exception.
   /// @param[in]  Reason           Numeric reason code.
   /// @param[in]  Description      A textual description of the exception.
   /// @param[in]  pHandler         For specific routing
   CExceptionEvent(IBase           *pObject,
                   btIID            SubClassID,
                   btID             ExceptionNumber,
                   btID             Reason,
                   btcString        Description,
                   IMessageHandler *pHandler=NULL);
   /// CExceptionEvent Copy Constructor.
   CExceptionEvent(const CExceptionEvent &rOther);
   /// CExceptionEvent Destructor.
   virtual ~CExceptionEvent();

   // <IExceptionEvent>
   btID ExceptionNumber() { return m_ExceptionNumber; }
   btID Reason()          { return m_Reason;          }
   btString Description() { return (btString)(char *)m_strDescription.c_str(); }
   // </IExceptionEvent>

protected:
   btID        m_ExceptionNumber;
   btID        m_Reason;
#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4251)
#endif // _MSC_VER
   std::string m_strDescription;
#ifdef _MSC_VER
# pragma warning(pop)
#endif // _MSC_VER

   // These Constructor forms are invalid.
   CExceptionEvent();
   CExceptionEvent & operator = (const CExceptionEvent & );
};


/// Concrete implementation of IExceptionTransactionEvent.
///
/// CExceptionTransactionEvent serves as the base class for all exceptions requiring an event response.
class AASLIB_API CExceptionTransactionEvent : public CAALEvent, public IExceptionTransactionEvent
{
public:
   /// CExceptionTransactionEvent Constructor.
   ///
   /// The native sub-class id is set to iidExTranEvent.
   ///
   /// @param[in]  pObject          An IBase associated with this event.
   /// @param[in]  rTranID          For routing event responses.
   /// @param[in]  ExceptionNumber  Numeric id for the exception.
   /// @param[in]  Reason           Numeric reason code.
   /// @param[in]  Description      A textual description of the exception.
   /// @param[in]  pHandler         For specific routing
   CExceptionTransactionEvent(IBase               *pObject,
                              TransactionID const &rTranID,
                              btID                 ExceptionNumber,
                              btID                 Reason,
                              btcString            Description,
                              IMessageHandler     *pHandler=NULL);
   /// CExceptionTransactionEvent Constructor.
   ///
   /// @param[in]  pObject          An IBase associated with this event.
   /// @param[in]  SubClassID       The native sub-class id for this event.
   /// @param[in]  rTranID          For routing event responses.
   /// @param[in]  ExceptionNumber  Numeric id for the exception.
   /// @param[in]  Reason           Numeric reason code.
   /// @param[in]  Description      A textual description of the exception.
   /// @param[in]  pHandler         For specific routing
   CExceptionTransactionEvent(IBase               *pObject,
                              btID                 SubClassID,
                              TransactionID const &rTranID,
                              btID                 ExceptionNumber,
                              btID                 Reason,
                              btcString            Description,
                              IMessageHandler     *pHandler=NULL);
   /// CExceptionTransactionEvent Copy Constructor.
   CExceptionTransactionEvent(CExceptionTransactionEvent const &rOther);
   /// CExceptionTransactionEvent Destructor.
   virtual ~CExceptionTransactionEvent();

   // <IExceptionEvent>
   btID ExceptionNumber() { return m_ExceptionNumber; }
   btID Reason()          { return m_Reason;          }
   btString Description() { return (btString)(char *)m_strDescription.c_str(); }
   // </IExceptionEvent>

   // <ITransactionEvent>
   TransactionID TranID()                      { return m_TranID;   }
   void SetTranID(TransactionID const &TranID) { m_TranID = TranID; }
   // </ITransactionEvent>

protected:
   TransactionID m_TranID;
   btID          m_ExceptionNumber;
   btID          m_Reason;
#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4251)
#endif // _MSC_VER
   std::string	  m_strDescription;
#ifdef _MSC_VER
# pragma warning(pop)
#endif // _MSC_VER

   // These Constructor forms are invalid.
   CExceptionTransactionEvent();
   CExceptionTransactionEvent(IBase * );
   CExceptionTransactionEvent & operator = (const CExceptionTransactionEvent & );
};


/// Concrete implementation of IObjectCreatedEvent.
///
/// The event contains an IBase * which may be queried for the desired Service interfaces.
///
/// ie
/// @code
/// if ( tranevtFactoryCreate == theEvent.SubClassID() ) {
///
///    IAALService *pService = dynamic_ptr<IAALService>(iidService, theEvent.Object());
///
///    if ( NULL != pService ) {
///       // Access the Service interface via pService.
///    }
///
/// }
/// @endcode
class AASLIB_API ObjectCreatedEvent : public CTransactionEvent, public IObjectCreatedEvent
{
public:

   /// ObjectCreatedEvent Constructor.
   ///
   /// The native sub-class interface id is tranevtFactoryCreate.
   ///
   /// @param[in] prtClient Pointer to Runtime Client
   /// @param[in]  pClient  ServiceClient.
   /// @param[in]  pObject  The Service requested by the IFactory::Create call.
   /// @param[in]  TranID   The original TransactionID from IFactory::Create.
   /// @param[in]  OptArgs  The NamedValueSet from the IFactory::Create call.
   ObjectCreatedEvent(IRuntimeClient       *prtClient,
                      IServiceClient       *pClient,
                      IBase                *pObject,
                      TransactionID         TranID,
                      const NamedValueSet  &OptArgs = NamedValueSet());

   void operator()();

   /// ObjectCreatedEvent Destructor.
   virtual ~ObjectCreatedEvent();

   // <IObjectCreatedEvent>
   const NamedValueSet & GetOptArgs() const { return m_OptArgs; }
   // </IObjectCreatedEvent>

protected:
   ObjectCreatedEvent();

   IRuntimeClient  *m_prtClient;
   IServiceClient  *m_pClient;
   NamedValueSet    m_OptArgs;
};

/// Created in response to a failed IFactory::Create.
class AASLIB_API ObjectCreatedExceptionEvent : public CExceptionTransactionEvent
{
public:

   /// ObjectCreatedExceptionEvent Constructor.
   ObjectCreatedExceptionEvent(IRuntimeClient     *prtClient,
                               IServiceClient     *pClient,
                               IBase              *pObject,
                               TransactionID       TranID,
                               btUnsigned64bitInt  ExceptionNumber,
                               btUnsigned64bitInt  Reason,
                               btcString           Description);

   void operator()();

   /// ObjectCreatedExceptionEvent Destructor.
   virtual ~ObjectCreatedExceptionEvent();

protected:
   ObjectCreatedExceptionEvent();

   IRuntimeClient  *m_prtClient;
   IServiceClient  *m_pClient;
};

/// Created in response to IAALService::Release.
class AASLIB_API CObjectDestroyedTransactionEvent : public CTransactionEvent
{
public:

   /// CObjectDestroyedTransactionEvent Constructor.
   CObjectDestroyedTransactionEvent(IServiceClient       *pClient,
                                    IBase                *pObject,
                                    TransactionID const  &TransID,
                                    btApplicationContext  Context);

   void operator()();

   /// CObjectDestroyedTransactionEvent Copy Constructor.
   CObjectDestroyedTransactionEvent(const CObjectDestroyedTransactionEvent &rOther);

   /// CObjectDestroyedTransactionEvent Destructor.
   virtual ~CObjectDestroyedTransactionEvent();

protected:
   CObjectDestroyedTransactionEvent();

   IServiceClient *m_pClient;
   IRuntimeClient *m_prtClient;
};


//=============================================================================
// Name: CApplicationEvent
// Description: CApplicationEvent class
//=============================================================================
class AASLIB_API CApplicationEvent : public CAALEvent, public IApplicationEvent
{
public:
   CApplicationEvent(btIID          subclass,
                     IBase         *pObject,
                     NamedValueSet &pNamedValues);

   CApplicationEvent(const CApplicationEvent& ApplicationEvent);
   virtual ~CApplicationEvent(void);

   IBase &Object() const  { return *m_pObject;      }

   // IApplicationEvent
   NamedValueSet &Parms() { return *m_pNamedValues; }
// INamedValueSet &Parms() {return (INamedValueSet &)*m_pNamedValues;};

   //Dynamic interface methods
   btGenericInterface Interface(btIID Interface) const
   { return CAALEvent::Interface(Interface); }

   virtual btBool Has(btIID Interface) const
   { return CAALEvent::Has(Interface); }

protected:
   IBase         *m_pObject;
   NamedValueSet *m_pNamedValues;
   btIID          m_subclass;

   CApplicationEvent();
   CApplicationEvent & operator = (const CApplicationEvent & );
};

/// @} group Events

END_NAMESPACE(AAL)

#ifdef __ICC                           /* Deal with Intel compiler-specific overly sensitive remarks */
   #pragma warning( pop)
#endif

#endif // __AALSDK_CAALEVENT_H__

