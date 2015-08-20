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
/// @file CAALBase.cpp
/// @brief CAALBase implementation.
/// @ingroup CommonBase
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Joseph Grecco, Intel Corporation
///          Henry Mitchel, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 01/04/2006     JG       Initial version started
/// 02/14/2007     JG       Removed CLRObjectInterface base class.
///                            CLRBase is the base class for all "active" AAL
///                            objects such as services and AFUs
/// 03/13/2007     JG       Changed SendMsg() to ProcessMessage()
///                            Defined a non-transaction version of
///                            ProcessMessage
/// 03/21/2007     JG       Ported to linux
/// 04/24/2007     JG       Refactored to make LRBase based on new IBase
/// 05/28/2007     JG       Added cached interfaces (SubClass)
/// 06/08/2007     JG       Converted to btID keys
/// 09/25/2007     JG       Began stripping legacy code out
/// 10/04/2007     JG       Renamed to CALLBase.cpp
/// 11/27/2007     JG/HM    Cleaned up Leaks
/// 11/27/2007     JG/HM    Fixed static factory being freed bug
/// 12/16/2007     JG       Changed include path to expose aas/
/// 02/03/2008     HM       Tweaked #pragmas for ICC
/// 05/08/2008     HM       Comments & License
/// 01/04/2009     HM       Updated Copyright
/// 12/28/2009     JG       Changed CAALBase to CAFUBase and created a
///                           CAALBase that is an object that simply can
///                           generate events@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include "aalsdk/CAALBase.h"
#include "aalsdk/INTCDefs.h"


BEGIN_NAMESPACE(AAL)


//=============================================================================
// Typedefs and Constants
//=============================================================================


//=============================================================================
//
//  AAS Base class implementation
//
//=============================================================================
//=============================================================================
// Name: CAASBase
// Description: Constructor
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments:
//=============================================================================
CAASBase::CAASBase(btApplicationContext Context) :
   CriticalSection(),
   m_Context(Context),
   m_bIsOK(false),
   m_InterfaceMap(),
   m_ISubClass(NULL),
   m_SubClassID(0)
{
   // Add the public interfaces
   if ( SetInterface(iidCBase, dynamic_cast<CAASBase *>(this)) != EObjOK ) {
      return;
   }

   // IBase is the default native subclass interface unless overridden by a subclass
   if ( SetSubClassInterface(iidBase, dynamic_cast<IBase *>(this)) != EObjOK ) {
      return;
   }

   m_bIsOK = true;
}

//=============================================================================
// Name: ~CAASBase
// Description: Destructor
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments:
//=============================================================================
CAASBase::~CAASBase() {/*empty*/}

//=============================================================================
// Name: CAASBase::Interface
// Description: Gets a pointer to the requested interface
// Interface: public
// Inputs: Interface - name of the interface to get.
// Outputs: Interface pointer.
// Comments:
//=============================================================================
btGenericInterface CAASBase::Interface(btIID Interface) const
{
   AutoLock(this);

   IIDINTERFACE_CITR itr = m_InterfaceMap.find(Interface);

   if ( m_InterfaceMap.end() == itr ) {
      return NULL;
   }

   return (*itr).second;
}

//=============================================================================
// Name: CAASBase::Has
// Description: Returns whether an object has an interface.
// Interface: public
// Inputs: Interface - name of the interface.
// Outputs: true - has interface otherwise false
// Comments:
//=============================================================================
btBool CAASBase::Has(btIID Interface) const
{
   AutoLock(this);
   return m_InterfaceMap.end() != m_InterfaceMap.find(Interface);
}

//=============================================================================
// Name: CAASBase::ISubClass
// Description: Returns the cached pointer to the native (subclass) object
// Interface: public
// Inputs: none.
// Outputs: class pointer
// Comments:
//=============================================================================
btGenericInterface CAASBase::ISubClass() const
{
   return m_ISubClass;
}

//=============================================================================
// Name: CAASBase::SubClassID
// Description: Returns the subclass ID for the Object
// Interface: public
// Inputs: none
// Outputs: ID
// Comments:
//=============================================================================
btIID CAASBase::SubClassID() const
{
   return m_SubClassID;
}

//=============================================================================
// Name: CAASBase::operator !=
// Description: operator !=
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments: checks to see if each interface in one object is implemented
//           in the other.
//=============================================================================
btBool CAASBase::operator != (IBase const &rOther) const
{
   return ! this->operator == (rOther);
}

//=============================================================================
// Name: CAASBase::operator ==
// Description: operator ==
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments: checks to see if each interface in one object is implemented
//           in the other.
//
// Three criteria must be met for CAASBase equality:
//  1) Both objects must implement iidCBase (ie, both are conceptually CAASBase
//     instances).
//  2) Both objects must implement the same SubClass (ie, both conceptually have
//     the same default interface).
//  3) Both objects must implement a) the same number and b) the same types of
//     other interfaces.
//=============================================================================
btBool CAASBase::operator == (IBase const &rOther) const
{
   AutoLock(this);

   CAASBase *pOther = reinterpret_cast<CAASBase *>(rOther.Interface(iidCBase));
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

void CAASBase::SetContext(btApplicationContext context)
{
   AutoLock(this);
   m_Context = context;
}

//=============================================================================
// Name: CAASBase::SetInterface
// Description: Sets an interface pointer on the object.
// Interface: protected
// Inputs: Interface - name of the interface to set.
//         pInterface - Interface pointer
// Outputs: Interface pointer.
// Comments:
//=============================================================================
EOBJECT CAASBase::SetInterface(btIID              Interface,
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

   //Add the interface
   m_InterfaceMap[Interface] = pInterface;

   return EObjOK;
}

//=============================================================================
// Name: CAASBase::SetSubClassInterface
// Description: Sets an interface pointer on the subclass interface for the
//              object.  This function may only be called once per class.
// Interface: protected
// Inputs: Interface - name of the interface to set.
//         pInterface - Interface pointer
// Outputs: Interface pointer.
// Comments:
//=============================================================================
EOBJECT CAASBase::SetSubClassInterface(btIID              InterfaceID,
                                       btGenericInterface pInterface)
{
   EOBJECT result;

   AutoLock(this);

   if ( (result = SetInterface(InterfaceID,
                               pInterface)) != EObjOK ) {
      return result;
   }

   m_ISubClass  = pInterface;
   m_SubClassID = InterfaceID;

   return result;
}


#if DEPRECATED
//=============================================================================
// Name: CAASBase::CAASBase
// Description: Copy Constructor
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments: For each base classes copy constructor must register
//           its own interface.  This implies that all derived classes
//           must call the base classes copy constructor for the copy
//           to work.
//=============================================================================
CAASBase::CAASBase(const CAASBase &rOther) :
   CriticalSection()
{
   AutoLock(this);

   // Add the public interfaces.
   if ( SetInterface(iidCBase, dynamic_cast<CAASBase *>(this)) != EObjOK ) {
      return;
   }

   // IBase is the default native subclass interface unless overridden by a subclass
   if ( SetSubClassInterface(iidBase, dynamic_cast<IBase *>(this)) != EObjOK ) {
      return;
   }

   m_Context = rOther.m_Context;
   m_bIsOK   = rOther.m_bIsOK;
}
#endif // DEPRECATED


//=============================================================================
// Name: CAALBase
// Description: Constructor
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments:
//=============================================================================
CAALBase::CAALBase(btEventHandler       pEventHandler,
                   btApplicationContext Context) :
   CAASBase(Context),
   m_pEventHandler(pEventHandler)
{
   // Save the event handler
   if ( NULL == pEventHandler ) {
      return;
   }
   m_bIsOK = true;
}

CAALBase::CAALBase() {/*empty*/}
CAALBase::~CAALBase() {}
CAALBase::CAALBase(const CAALBase & ) {/*empty*/}
CAALBase & CAALBase::operator=(const CAALBase & ) { return *this; }


END_NAMESPACE(AAL)


