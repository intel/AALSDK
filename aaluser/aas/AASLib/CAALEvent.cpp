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
/// @file CAALEvent.cpp
/// @brief CAALEvent, CTransactionEvent, CExceptionEvent, and CExceptionTransactionEvent
///  implementations.
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
///                         Changed constant definitions and
///                         exports to export base pointer rather
///                         than object allowing the object
///                         subclass to remain opaque. User uses
///                         I_LREvent * and defines in
///                         libLongRidgeCPP.h
/// 03/21/2007     JG       Ported to linux
/// 04/24/2007     JG       Made all events carry IBase pointers
/// 05/28/2007     JG       Added cached interfaces (SubClass)
/// 06/08/2007     JG       Converted to btID keys
/// 10/04/2007     JG       Renamed to CAALEvent.cpp
/// 12/16/2007     JG       Changed include path to expose aas/
/// 02/03/2008     HM       Cleaned up various things brought out by ICC
/// 05/08/2008     HM       Comments & License
/// 12/10/2008     HM       Changed IEvent.Context(), no longer caches
/// 12/15/2008     HM       Added iidEvent to second constructor
/// 01/04/2009     HM       Updated Copyright
/// 02/25/2009     HM       Re-enabled IEvent.Context cacheing - required e.g.
///                            for DestroyObject messages
/// 10/06/2015     JG       Removed ObjectxyzEvents@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include "aalsdk/CAALEvent.h"
#include "aalsdk/INTCDefs.h"
#include "aalsdk/AALLogger.h"

#ifdef __ICC                           /* Deal with Intel compiler-specific overly sensitive remarks */
//   #pragma warning( push)
//   #pragma warning(disable:68)         // warning: integer conversion resulted in a change of sign.
                                       //    This is intentional
   #pragma warning(disable:383)        // remark: value copied to temporary, reference to temporary used
//   #pragma warning(disable:981)        // remark: operands are evaluated in unspecified order
//   #pragma warning(disable:869)        // remark: parameter "XXXX" was never referenced
//   #pragma warning(disable:1418)       // remark: external function definition with no prior declaration
//   #pragma warning(disable:1572)       // remark: floating-point equality and inequality comparisons are unreliable
#endif

BEGIN_NAMESPACE(AAL)

//=============================================================================
// Name: CAALEvent
// Description: Constructor
// Comments: Concrete Base class for all events
//=============================================================================
CAALEvent::CAALEvent(IBase *pObject) :
   m_pObject(NULL),
   m_bIsOK(false),
   m_Context(NULL),
   m_pServiceClient(NULL),
   m_pRuntimeClient(NULL),
   m_pEventHandler(NULL),
   m_SubClassID(0)
{
   AutoLock(this);

   if ( SetInterface(iidCEvent, dynamic_cast<CAALEvent *>(this)) != EObjOK ) {
      return;
   }

   // IEvent is the default native subclass interface unless overriden by a subclass
   if ( SetSubClassInterface(iidEvent, dynamic_cast<IEvent *>(this)) != EObjOK ) {
      return;
   }

   // Save the objects
   m_pObject = pObject;

   // Event inherits object's Application Context Value based on m_pObject
   UpdateContext();

   m_bIsOK = true;
}

//=============================================================================
// Name: CAALEvent
// Description: Constructor
// Comments: Concrete Base class for all events
//=============================================================================
CAALEvent::CAALEvent(IBase *pObject, btIID SubClassID) :
   m_pObject(NULL),
   m_bIsOK(false),
   m_Context(NULL),
   m_pServiceClient(NULL),
   m_pRuntimeClient(NULL),
   m_pEventHandler(NULL),
   m_SubClassID(0)
{
   AutoLock(this);

   if ( SetInterface(iidCEvent, dynamic_cast<CAALEvent *>(this)) != EObjOK ) {
      return;
   }

   if ( SetInterface(iidEvent, dynamic_cast<IEvent *>(this)) != EObjOK ) {
      return;
   }

   // IEvent is the default native subclass interface unless overriden by a subclass
   if ( SetSubClassInterface(SubClassID, dynamic_cast<IEvent *>(this)) != EObjOK ) {
      return;
   }

   // Save the object
   m_pObject = pObject;

   // Event inherits object's Application Context Value based on m_pObject
   UpdateContext();

   m_bIsOK = true;
}

//=============================================================================
// Name: CAALEvent::Interface
// Description: Gets a pointer to the requested interface
// Interface: public
// Inputs: ID - name of the interface to get.
// Outputs: Interface pointer.
// Comments:
//=============================================================================
btGenericInterface CAALEvent::Interface(btIID ID) const
{
   AutoLock(this);

   IIDINTERFACE_CITR itr = m_InterfaceMap.find(ID);
   if ( m_InterfaceMap.end() == itr ) { // not found
      return NULL;
   }

   return (*itr).second;
}

//=============================================================================
// Name: CAALEvent::Has
// Description: Returns whether an object has an interface.
// Interface: public
// Inputs: ID - name of the interface.
// Outputs: true - has interface otherwise false
// Comments:
//=============================================================================
btBool CAALEvent::Has(btIID ID) const
{
   AutoLock(this);
   return m_InterfaceMap.end() != m_InterfaceMap.find(ID);
}

btIID CAALEvent::SubClassID() const
{
   AutoLock(this);
   return m_SubClassID;
}

//=============================================================================
// Name: CAALEvent::operator !=
// Description: operator !=
// Interface: public
// Inputs: rOther - other object to compare against.
// Outputs: != ? true : flase.
// Comments: equality is defined as implementing the same
//           interfaces NOT the contents of the objects data.
//=============================================================================
btBool CAALEvent::operator != (const IEvent &rOther) const
{
   return ! this->operator == (rOther);
}

//=============================================================================
// Name: CAALEvent::operator ==
// Description: operator ==
// Interface: public
// Inputs: rOther - other object to compare against.
// Outputs: == ? true : flase.
// Comments: equality is defined as implementing the same
//           interfaces NOT the contents of the objects data.
//
// Three criteria must be met for CAALEvent equality:
//  1) Both objects must implement iidCEvent (ie, both are conceptually CAALEvent
//     instances).
//  2) Both objects must implement the same SubClass (ie, both conceptually have
//     the same default interface).
//  3) Both objects must implement a) the same number and b) the same types of
//     other interfaces.
//=============================================================================
btBool CAALEvent::operator == (const IEvent &rOther) const
{
   AutoLock(this);

   CAALEvent *pOther = reinterpret_cast<CAALEvent *>(rOther.Interface(iidCEvent));
   if ( NULL == pOther ) {
      // 1) fails
      return false;
   }

   {
      AutoLock(pOther);

      if ( SubClassID() != pOther->SubClassID() ) {
         // 2) fails
         return false;
      }

      if ( m_InterfaceMap.size() != pOther->m_InterfaceMap.size() ) {
         // 3a) fails
         return false;
      }

      IIDINTERFACE_CITR l;
      IIDINTERFACE_CITR r;

      for ( l = m_InterfaceMap.begin(), r = pOther->m_InterfaceMap.begin() ;
               l != m_InterfaceMap.end() ;
                  ++l, ++r ) {
         if ( (*l).first != (*r).first ) {
            // 3b) fails
            return false;
         }
      }
   }

   // objects are equal
   return true;
}

IBase &               CAALEvent::Object() const { AutoLock(this); return *m_pObject; }
IBase *              CAALEvent::pObject() const { AutoLock(this); return  m_pObject; }
btBool                  CAALEvent::IsOK() const { AutoLock(this); return  m_bIsOK;   }
btApplicationContext CAALEvent::Context() const { AutoLock(this); return  m_Context; }

void CAALEvent::setHandler(IServiceClient *pHandler)
{
   AutoLock(this);
   m_pServiceClient = pHandler;
   m_pRuntimeClient = NULL;
   m_pEventHandler  = NULL;
}

void CAALEvent::setHandler(IRuntimeClient *pHandler)
{
   AutoLock(this);
   m_pServiceClient = NULL;
   m_pRuntimeClient = pHandler;
   m_pEventHandler  = NULL;
}

void CAALEvent::setHandler(btEventHandler pHandler)
{
   AutoLock(this);
   m_pServiceClient = NULL;
   m_pRuntimeClient = NULL;
   m_pEventHandler  = pHandler;
}

btApplicationContext CAALEvent::SetContext(btApplicationContext Ctx)
{
   AutoLock(this);
   btApplicationContext res = m_Context;
   m_Context = Ctx;
   return res;
}

IEvent * CAALEvent::Clone() const
{
   return new(std::nothrow) CAALEvent(*this);
}

//=============================================================================
// Name: CAALEvent::SetSubClassInterface
// Description: Sets an interface pointer on the subclass interface for the
//              object.  This function may only be called once per class.
// Interface: protected
// Inputs: Interface - name of the interface to set.
//         pInterface - Interface pointer
// Outputs: Interface pointer.
// Comments:
//=============================================================================
EOBJECT CAALEvent::SetSubClassInterface(btIID              InterfaceID,
                                        btGenericInterface pInterface)
{
   EOBJECT result;

   AutoLock(this);

   if ( (result = SetInterface(InterfaceID,
                               pInterface)) != EObjOK ) {
      return result;
   }

   m_SubClassID = InterfaceID;

   return result;
}

//=============================================================================
// Name:          CAALEvent::SetObject
// Description:   Update the object pointed to by the Event, and its associated
//                   cached context
// Interface:     public
// Inputs:        pointer to new IBase
// Outputs:       void
// Comments:      Used by ReThrow, where the message needs to be modified but
//                   not completely reconstructed
//=============================================================================
void CAALEvent::SetObject(IBase *pObject)
{
   AutoLock(this);
   m_pObject = pObject;
   UpdateContext();
}

//=============================================================================
// Name: CAALEvent::operator ()
// Description: Functor operator
// Interface: public
// Inputs: context - payload
// Comments: Canonical processing:
//                 By default Events can only be delivered to static Event
//                 Handlers (btEventhandler) pre 4.0 or to IMessageHanders
//                 TransactionID() processing for event delivery override can
//                 ONLY be sent to btEventhandler types. Override this operator
//                 to alter behavior.
//=============================================================================
void CAALEvent::operator()()
{
   if ( ProcessEventTranID() ) {
      return;
   }

   if ( NULL != m_pEventHandler ) {
      m_pEventHandler(*this);
   }

   btBool del = false;

   if ( NULL != m_pServiceClient ) {
      del = true;
      m_pServiceClient->serviceEvent(*this);
   }

   if ( NULL != m_pRuntimeClient ) {
      del = true;
      m_pRuntimeClient->runtimeEvent(*this);
   }

   if ( del ) {
      delete this;
   }
}

//=============================================================================
// Name: CAALEvent
// Description: Destructor
//=============================================================================
void CAALEvent::Delete()
{
   delete this;
}

//=============================================================================
// Name:          CAALEvent::UpdateContext
// Description:   Update m_Context based on m_pObject, cache the object context
// Interface:     protected
// Inputs:        object member m_pObject
// Outputs:       void
// Comments:      Utility function used for internal state maintenance
//=============================================================================
void CAALEvent::UpdateContext()
{
   AutoLock(this);
   if ( (NULL != m_pObject) && m_pObject->IsOK() ) {
      m_Context = m_pObject->Context();
   }
}

//=============================================================================
// Name: CAALEvent::SetInterface
// Description: Sets an interface pointer on the object.
// Interface: protected
// Inputs: Interface - name of the interface to set.
//         pInterface - Interface pointer
// Outputs: Interface pointer.
// Comments:
//=============================================================================
EOBJECT CAALEvent::SetInterface(btIID              Interface,
                                btGenericInterface pInterface)
{
   if ( NULL == pInterface ) {
      return EObjBadObject;
   }

   AutoLock(this);

   // Make sure there is not an implementation already.
   if ( Has(Interface) ) {
      return EObjDuplicateName;
   }

   // Add the interface
   m_InterfaceMap[Interface] = pInterface;

   return EObjOK;
}

//=============================================================================
// Name:         CAALEvent::ProcessEventTranID
// Description:  Process the transaction ID for Event Handling
// Interface:    protected
// Comments:
//=============================================================================
btBool CAALEvent::ProcessEventTranID() {/* no TransactionID override in regular events */ return false; }

CAALEvent::CAALEvent() {/*empty*/}
CAALEvent::~CAALEvent() {/*empty*/}

//=============================================================================
// Name: CTransactionEvent
// Description: constructor CTransactionEvent base class
// Comments: Must Initialize base to ensure Interface registration.
//           Must be constructed with an object and a TranID.
//=============================================================================
CTransactionEvent::CTransactionEvent(IBase               *pObject,
                                     TransactionID const &TranID) :
   CAALEvent(pObject),
   m_TranID(TranID)
{
   AutoLock(this);

   // ITransactionEvent is the default native subclass interface unless overridden by a subclass.
   if ( SetSubClassInterface(iidTranEvent, dynamic_cast<ITransactionEvent *>(this)) != EObjOK ) {
      m_bIsOK = false;
      return;
   }
}

//=============================================================================
// Name: CTransactionEvent
// Description: constructor CTransactionEvent base class
// Comments: Must Initialize base to ensure Interface registration.
//           Must be constructed with an object and a TranID.
//=============================================================================
CTransactionEvent::CTransactionEvent(IBase               *pObject,
                                     btIID                SubClassID,
                                     TransactionID const &TranID) :
   CAALEvent(pObject),
   m_TranID(TranID)
{
   AutoLock(this);

   // ITransactionEvent is the default native subclass interface unless overridden by a subclass.
   if ( SetInterface(iidTranEvent, dynamic_cast<ITransactionEvent *>(this)) != EObjOK ) {
      m_bIsOK = false;
      return;
   }

   // Set the subclass to the one provided
   if ( SetSubClassInterface(SubClassID, dynamic_cast<ITransactionEvent *>(this)) != EObjOK ) {
      m_bIsOK = false;
      return;
   }
}

btBool CTransactionEvent::operator == (const IEvent &rhs) const
{
   AutoLock(this);

   if ( CAALEvent::operator==(rhs) ) {

      ITransactionEvent *pIOther = reinterpret_cast<ITransactionEvent *>(rhs.Interface(iidTranEvent));
      CTransactionEvent *pCOther = dynamic_cast<CTransactionEvent *>(pIOther);

      if ( NULL == pCOther ) {
         return false;
      }

      {
         AutoLock(pCOther);

         if ( m_TranID == pCOther->m_TranID ) {
            // objects are equal
            return true;
         }
      }

   }
   return false;
}

IEvent * CTransactionEvent::Clone() const
{
   return new(std::nothrow) CTransactionEvent(*this);
}

TransactionID CTransactionEvent::TranID() const
{
   AutoLock(this);
   return m_TranID;
}

void CTransactionEvent::SetTranID(TransactionID const &TranID)
{
   AutoLock(this);
   m_TranID = TranID;
}

CTransactionEvent::CTransactionEvent() {/*empty*/}
CTransactionEvent::CTransactionEvent(IBase * ) {/*empty*/}

btBool CTransactionEvent::ProcessEventTranID()
{
   btBool ret = false;
   // If a static handler has been assigned
   if ( NULL != m_TranID.Handler() ) {
      //Call it
      m_TranID.Handler()(*this);
      ret =  m_TranID.Filter();
      // If an IMessageHandler has been assigned
   }
   // Return filter value or false if no override at all
   return ret;
}

//=============================================================================
// Name: CExceptionEvent
// Description: Constructor
// Comments: Must Initialize base to ensure Interface registration.
//           Must be constructed with an object.
//=============================================================================
CExceptionEvent::CExceptionEvent(IBase    *pObject,
                                 btID      ExceptionNumber,
                                 btID      Reason,
                                 btcString Description) :
   CAALEvent(pObject),
   m_ExceptionNumber(ExceptionNumber),
   m_Reason(Reason),
   m_strDescription(Description)

{
   AutoLock(this);

   // default native subclass interface unless overridden by a subclass.
   if ( SetSubClassInterface(iidExEvent, dynamic_cast<IExceptionEvent *>(this)) != EObjOK ) {
      m_bIsOK = false;
      return;
   }
}

//=============================================================================
// Name: CExceptionEvent
// Description: Constructor
// Comments: Must Initialize base to ensure Interface registration.
//           Must be constructed with an object.
//=============================================================================
CExceptionEvent::CExceptionEvent(IBase    *pObject,
                                 btIID     SubClassID,
                                 btID      ExceptionNumber,
                                 btID      Reason,
                                 btcString Description) :
   CAALEvent(pObject),
   m_ExceptionNumber(ExceptionNumber),
   m_Reason(Reason),
   m_strDescription(Description)
{
   AutoLock(this);

   // iidExEvent is the default native subclass interface unless overriden by a subclass
   if ( SetInterface(iidExEvent, dynamic_cast<IExceptionEvent *>(this)) != EObjOK ) {
      m_bIsOK = false;
      return;
   }

   // default native subclass interface unless overriden by a subclass
   if ( SetSubClassInterface(SubClassID, dynamic_cast<IExceptionEvent *>(this)) != EObjOK ) {
      m_bIsOK = false;
      return;
   }
}

btBool CExceptionEvent::operator == (const IEvent &rhs) const
{
   AutoLock(this);

   if ( CAALEvent::operator==(rhs) ) {

      IExceptionEvent *pIOther = reinterpret_cast<IExceptionEvent *>(rhs.Interface(iidExEvent));
      CExceptionEvent *pCOther = dynamic_cast<CExceptionEvent *>(pIOther);

      if ( NULL == pCOther ) {
         return false;
      }

      {
         AutoLock(pCOther);

         if ( m_ExceptionNumber == pCOther->m_ExceptionNumber &&
              m_Reason          == pCOther->m_Reason          &&
              0 == m_strDescription.compare(pCOther->m_strDescription) ) {
            // objects are equal
            return true;
         }
      }

   }
   return false;
}

IEvent * CExceptionEvent::Clone() const
{
   return new(std::nothrow) CExceptionEvent(*this);
}

btID CExceptionEvent::ExceptionNumber() const { AutoLock(this); return m_ExceptionNumber; }
btID          CExceptionEvent::Reason() const { AutoLock(this); return m_Reason;          }

btString CExceptionEvent::Description() const
{
   AutoLock(this);
   return (btString)(char *)m_strDescription.c_str();
}

CExceptionEvent::CExceptionEvent() {/*empty*/}

//=============================================================================
// Name: CExceptionTransactionEvent
// Description: IMMExceptionTransactionEvent base class
// Comments: Must Initialize base to ensure Interface registration.
//           Must be constructed with an object and a TranID.
//=============================================================================
CExceptionTransactionEvent::CExceptionTransactionEvent(IBase               *pObject,
                                                       TransactionID const &TranID,
                                                       btID                 ExceptionNumber,
                                                       btID                 Reason,
                                                       btcString            Description) :
   CAALEvent(pObject),
   m_TranID(TranID),
   m_ExceptionNumber(ExceptionNumber),
   m_Reason(Reason),
   m_strDescription(Description)
{
   AutoLock(this);

   if ( SetInterface(iidTranEvent, dynamic_cast<ITransactionEvent *>(this)) != EObjOK ) {
      m_bIsOK = false;
      return;
   }

   // Default native subclass interface unless overridden by a subclass.
   if ( SetSubClassInterface(iidExTranEvent, dynamic_cast<IExceptionTransactionEvent *>(this)) != EObjOK ) {
      m_bIsOK = false;
      return;
   }
}

//=============================================================================
// Name: CExceptionTransactionEvent
// Description: CExceptionTransactionEvent base class
// Comments: Must Initialize base to ensure Interface registration.
//           Must be constructed with an object and a TranID.
//=============================================================================
CExceptionTransactionEvent::CExceptionTransactionEvent(IBase               *pObject,
                                                       btIID                SubClassID,
                                                       TransactionID const &TranID,
                                                       btID                 ExceptionNumber,
                                                       btID                 Reason,
                                                       btcString            Description) :
   CAALEvent(pObject),
   m_TranID(TranID),
   m_ExceptionNumber(ExceptionNumber),
   m_Reason(Reason),
   m_strDescription(Description)
{
   AutoLock(this);

   if ( SetInterface(iidTranEvent, dynamic_cast<ITransactionEvent *>(this)) != EObjOK ) {
      m_bIsOK = false;
      return;
   }

   if ( SetInterface(iidExTranEvent, dynamic_cast<IExceptionTransactionEvent *>(this)) != EObjOK ) {
      m_bIsOK = false;
      return;
   }

   // Default native subclass interface unless overridden by a subclass.
   if ( SetSubClassInterface(SubClassID, dynamic_cast<IExceptionTransactionEvent *>(this)) != EObjOK ) {
      m_bIsOK = false;
      return;
   }
}

btBool CExceptionTransactionEvent::operator == (const IEvent &rhs) const
{
   AutoLock(this);

   if ( CAALEvent::operator==(rhs) ) {

      IExceptionTransactionEvent *pIOther = reinterpret_cast<IExceptionTransactionEvent *>(rhs.Interface(iidExTranEvent));
      CExceptionTransactionEvent *pCOther = dynamic_cast<CExceptionTransactionEvent *>(pIOther);

      if ( NULL == pCOther ) {
         return false;
      }

      {
         AutoLock(pCOther);

         if ( m_TranID          == pCOther->m_TranID          &&
              m_ExceptionNumber == pCOther->m_ExceptionNumber &&
              m_Reason          == pCOther->m_Reason          &&
              0 == m_strDescription.compare(pCOther->m_strDescription) ) {
            // objects are equal
            return true;
         }
      }

   }
   return false;
}

IEvent * CExceptionTransactionEvent::Clone() const
{
   return new(std::nothrow) CExceptionTransactionEvent(*this);
}

btID CExceptionTransactionEvent::ExceptionNumber() const { AutoLock(this); return m_ExceptionNumber; }
btID          CExceptionTransactionEvent::Reason() const { AutoLock(this); return m_Reason;          }

btString CExceptionTransactionEvent::Description() const
{
   AutoLock(this);
   return (btString)(char *)m_strDescription.c_str();
}

TransactionID CExceptionTransactionEvent::TranID() const { AutoLock(this); return m_TranID; }

void CExceptionTransactionEvent::SetTranID(TransactionID const &TranID)
{
   AutoLock(this);
   m_TranID = TranID;
}

btBool CExceptionTransactionEvent::ProcessEventTranID()
{
   btBool ret = false;
   // If a static handler has been assigned
   if ( NULL != m_TranID.Handler() ) {
      //Call it
      m_TranID.Handler()(*this);
      ret =  m_TranID.Filter();
      // If an IMessageHandler has been assigned
   }
   // Return filter value or false if no override at all
   return ret;
}

CExceptionTransactionEvent::CExceptionTransactionEvent() {/*empty*/}
CExceptionTransactionEvent::CExceptionTransactionEvent(IBase * ) {/*empty*/}


END_NAMESPACE(AAL)
