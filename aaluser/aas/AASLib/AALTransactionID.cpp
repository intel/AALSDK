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
/// @file AALTransactionID.cpp
/// @brief AAL TransactionID
/// @ingroup Events
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Joseph Grecco, Intel Corporation
///          Henry Mitchel, Intel Corporation
///
/// WHEN:          WHO:     WHAT:
/// 01/04/2006     JG       Initial version
/// 04/11/2006     JG       Made transaction ID an [in|out] object that carries an
///                         application specified context. In addition the Tid has
///                         been extended to provide a listener used to scope per
///                         transaction event handling.
/// 01/04/2006     JG       Initial version started
/// 03/13/2007     JG       Added a constructor for AppContext
///                         only and removed default eventhandler
///                         argument which caused a potential
///                         unwanted type coversion of type void*
///                         to transaction ID when passing to an
///                         argument expecting a tranID &
/// 03/21/2007     JG       Ported to Linux
/// 10/04/2007     JG       Renamed to AALTransactionID
/// 05/08/2008     HM       Comments & License
/// 06/27/2008     HM       Added ostream << for TransactionID
/// 11/10/2008     JG       Replaced discrete member variables with typedef
/// 12/08/2008     HM/JG    Added new TransactionID ctor and fixed random intID
/// 01/04/2009     HM       Updated Copyright
/// 08/12/2010     HM       Added new CTOR: Application specified ID and Handler
/// 04/25/2014     JG       Added Support for IBase@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include "aalsdk/AALBase.h"
#include "aalsdk/AALTransactionID.h"

BEGIN_NAMESPACE(AAL)

//=============================================================================
// Name: _AssignTranID
// Description: Assigns a unique transaction iD
// Interface: public
// Inputs: none
// Outputs: TranID.
// Comments: This is just a quick hack implementation!!!
//=============================================================================
bt32bitInt _AssignTranID()
{
   static bt32bitInt _LastTranID = 0;

   if ( 0 == _LastTranID ) {

      srand( (unsigned)time( NULL ) );
      _LastTranID = rand();

   } else {
      _LastTranID++;
   }

   return _LastTranID;
}

//=============================================================================
// Name: TransactionID
// Description: Default constructor  - System assigned ID. Default Listener
//=============================================================================
TransactionID::TransactionID()
{
   m_tid.m_ID              = NULL;
   m_tid.m_Handler         = NULL;
   m_tid.m_IBase  = NULL;
   m_tid.m_Filter          = true;

   //Assign an internal value
   m_tid.m_intID = _AssignTranID();
}

//=============================================================================
// Name: TransactionID
// Description: Constructor for a stTransactionID_ structure
//=============================================================================
TransactionID::TransactionID(stTransactionID_t stTid)
{
   m_tid = stTid;
}

//=============================================================================
// Name: TransactionID
// Description: Constructor for a Specified Integer ID
//=============================================================================
TransactionID::TransactionID(bt32bitInt intID)
{
   m_tid.m_ID              = NULL;
   m_tid.m_Handler         = NULL;
   m_tid.m_IBase           = NULL;
   m_tid.m_Filter          = true;
   m_tid.m_intID           = intID;
}

//=============================================================================
// Name: TransactionID
// Description: Constructor for a Specified Integer ID
//=============================================================================
TransactionID::TransactionID(bt32bitInt intID,
                             btApplicationContext Ctxt)
{
   m_tid.m_ID              = Ctxt;
   m_tid.m_Handler         = NULL;
   m_tid.m_IBase           = NULL;
   m_tid.m_Filter          = true;
   m_tid.m_intID           = intID;
}

//=============================================================================
// Name: TransactionID
// Description: Copy constructor
//=============================================================================
TransactionID::TransactionID(TransactionID const &rOther)
{
   m_tid = rOther.m_tid;
}

//=============================================================================
// Name: TransactionID
// Description: Constructor for Application specified ID
//              and Handler (if provided)
//=============================================================================
TransactionID::TransactionID(btApplicationContext Ctxt)
{
   m_tid.m_ID              = Ctxt;
   m_tid.m_Handler         = NULL;
   m_tid.m_IBase           = NULL;
   m_tid.m_Filter          = true;
   m_tid.m_intID           = _AssignTranID();
}


//=============================================================================
// Name: TransactionID
// Description: Constructor for Application specified ID, Context, Handler,
//              and filter flag
//=============================================================================
TransactionID::TransactionID(bt32bitInt           intID,
                             btApplicationContext Ctxt,
                             btEventHandler       evtHandler,
                             btBool               Filter)
{
   m_tid.m_ID              = Ctxt;
   m_tid.m_Handler         = evtHandler;
   m_tid.m_IBase           = NULL;
   m_tid.m_Filter          = Filter;
   m_tid.m_intID           = intID;
}

//=============================================================================
// Name: TransactionID
// Description: Constructor for Application specified ID
//              and Handler (if provided)
//=============================================================================
TransactionID::TransactionID(btApplicationContext Ctxt,
                             btEventHandler       evtHandler,
                             btBool               Filter)
{
   m_tid.m_ID              = Ctxt;
   m_tid.m_Handler         = evtHandler;
   m_tid.m_IBase           = NULL;
   m_tid.m_Filter          = Filter;
   m_tid.m_intID           = _AssignTranID();
}


//=============================================================================
// Name: TransactionID
// Description: Constructor for Application specified ID and Handler
//=============================================================================
TransactionID::TransactionID(bt32bitInt     intID,
                             btEventHandler evtHandler,
                             btBool         Filter)
{
   m_tid.m_ID              = NULL;
   m_tid.m_Handler         = evtHandler;
   m_tid.m_IBase           = NULL;
   m_tid.m_Filter          = Filter;
   m_tid.m_intID           = intID;
}

//=============================================================================
// Name: TransactionID
// Description: Constructor for system assigned ID application provided
//              Handler.
//=============================================================================
TransactionID::TransactionID(btEventHandler evtHandler,
                             btBool         Filter)
{
   //Assign an internal value
   m_tid.m_ID              = NULL;
   m_tid.m_Handler         = evtHandler;
   m_tid.m_IBase           = NULL;
   m_tid.m_Filter          = Filter;
   m_tid.m_intID           = _AssignTranID();
}


//=============================================================================
// Name: TransactionID
// Description: Constructor for Application specified ID, Context, Handler,
//              and filter flag
//=============================================================================
TransactionID::TransactionID(bt32bitInt              intID,
                             btApplicationContext    Ctxt,
                             IBase        *pIBase,
                             btBool                  Filter)
{
   m_tid.m_ID              = Ctxt;
   m_tid.m_Handler         = NULL;
   m_tid.m_IBase           = pIBase;
   m_tid.m_Filter          = Filter;
   m_tid.m_intID           = intID;
}

//=============================================================================
// Name: TransactionID
// Description: Constructor for Application specified ID
//              and Handler (if provided)
//=============================================================================
TransactionID::TransactionID(btApplicationContext          Ctxt,
                             IBase              *pIBase,
                             btBool                        Filter)
{
   m_tid.m_ID              = Ctxt;
   m_tid.m_Handler         = NULL;
   m_tid.m_IBase           = pIBase;
   m_tid.m_Filter          = Filter;
   m_tid.m_intID           = _AssignTranID();
}


//=============================================================================
// Name: TransactionID
// Description: Constructor for Application specified ID and Handler
//=============================================================================
TransactionID::TransactionID(bt32bitInt              intID,
                             IBase        *pIBase,
                             btBool                  Filter)
{
   m_tid.m_ID              = NULL;
   m_tid.m_Handler         = NULL;
   m_tid.m_IBase           = pIBase;
   m_tid.m_Filter          = Filter;
   m_tid.m_intID           = intID;
}

//=============================================================================
// Name: TransactionID
// Description: Constructor for system assigned ID application provided
//              Handler.
//=============================================================================
TransactionID::TransactionID(IBase   *pIBase,
                             btBool             Filter)
{
   //Assign an internal value
   m_tid.m_ID              = NULL;
   m_tid.m_Handler         = NULL;
   m_tid.m_IBase           = pIBase;
   m_tid.m_Filter          = Filter;
   m_tid.m_intID           = _AssignTranID();
}

//=============================================================================
// Name: Context
// Description: Returns Context
//=============================================================================
btApplicationContext TransactionID::Context() const
{
   // return value
   return m_tid.m_ID;
}

//=============================================================================
// Name: Filter
// Description: Returns Filter flag
//=============================================================================
btBool TransactionID::Filter() const
{
   //Return filter flag
   return m_tid.m_Filter;
}

//=============================================================================
// Name: Handler
// Description: Returns Event Handler
//=============================================================================
btEventHandler TransactionID::Handler() const
{
   //Return handler
   return m_tid.m_Handler;
}

//=============================================================================
// Name: MsgHandler
// Description: Returns Message Handler
//=============================================================================
IBase * TransactionID::Ibase() const
{
   //Return handler
   return m_tid.m_IBase;
}

//=============================================================================
// Name: ID
// Description: Returns ID
//=============================================================================
bt32bitInt TransactionID::ID() const
{
   //Return handler
   return m_tid.m_intID;
}

void TransactionID::Context(btApplicationContext Context) { m_tid.m_ID      = Context; }
void TransactionID::Handler(btEventHandler Handler)       { m_tid.m_Handler = Handler; }
void   TransactionID::Ibase(IBase *pIBase)                { m_tid.m_IBase   = pIBase;  }
void  TransactionID::Filter(btBool Filter)                { m_tid.m_Filter  = Filter;  }
void      TransactionID::ID(bt32bitInt intID)             { m_tid.m_intID   = intID;   }

btBool TransactionID::operator == (const TransactionID &rhs) const
{
   const IBase *pMyIBase  = m_tid.m_IBase;
   const IBase *pRHSIBase = rhs.m_tid.m_IBase;

   if ( ( NULL == pMyIBase ) && ( NULL != pRHSIBase) ) {
      return false;
   }
   if ( ( NULL != pMyIBase ) && ( NULL == pRHSIBase) ) {
      return false;
   }

   if ( NULL != pMyIBase ) {
      // We have an IBase * for both lhs and rhs.
      ASSERT(NULL != pRHSIBase);

      if ( pMyIBase->operator != (*pRHSIBase) ) {
         return false;
      }
   }

   // compare the other data members.
   return m_tid.m_ID      == rhs.m_tid.m_ID      &&
          m_tid.m_Handler == rhs.m_tid.m_Handler &&
          m_tid.m_Filter  == rhs.m_tid.m_Filter  &&
          m_tid.m_intID   == rhs.m_tid.m_intID;
}

//=============================================================================
// Name: ~TransactionID
// Description: Destructor
//=============================================================================
TransactionID::~TransactionID() {}

//=============================================================================
// Name: operator <<
// Description: print out a TransactionID
//=============================================================================
std::ostream & operator << (std::ostream &s, const TransactionID &TranID)
{
   s << "TransactionID: ID=" << TranID.ID() << " Context=" << TranID.Context()
     << " Handler=" << TranID.Handler() << " Filter=" << TranID.Filter() << std::endl;
   return s;
}

END_NAMESPACE(AAL)


