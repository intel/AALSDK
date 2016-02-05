// Copyright(c) 2006-2016, Intel Corporation
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
/// @file CAALBase.h
/// @brief Defines all of the base classes for AAL.
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
///                            CLRBase is the base class for all
///                            "active" LR objects such as MPCs and
///                            AFUs
/// 03/13/2007     JG       Changed SendMsg() to PorcessMessage()
///                            Defined a non-transaction version of
///                            ProcessMessage
/// 03/21/2007     JG       Ported to Linux
/// 04/23/2007     JG       Added AAS Interfaces
/// 04/24/2007     JG       Refactored to make LRBase based on
///                            new IBase
/// 06/08/2007     JG       Converted to btID keys
/// 09/25/2007     JG       Began refactoring out lagacy code
/// 10/04/2007     JG       Renamed to CAALBase
/// 10/31/2007     HM       #ifndef __GNUC__ away various #pragmas
/// 11/27/2007     JG/HM    Added Assassin Class
/// 12/16/2007     JG       Changed include path to expose aas/
/// 02/03/2008     HM       Pragmas for ICC, copyright, pragmas for VisStudio
/// 05/08/2008     HM       Comments & License
/// 12/14/2008     HM       Added virtual over-rides to CAALBase
/// 01/04/2009     HM       Updated Copyright
/// 06/20/2009     JG       Changes in preparation for shutdown
/// 12/28/2009     JG       Changed CAALBase to CAFUBase and created a
///                           CAALBase that is an object that simply can
///                           generate events
/// 10/06/2015     JG        Removed Subclass interfaces@endverbatim
//****************************************************************************
#ifndef __AALSDK_CAALBASE_H__
#define __AALSDK_CAALBASE_H__
#include <aalsdk/AALBase.h>
#include <aalsdk/osal/CriticalSection.h>

BEGIN_NAMESPACE(AAL)


//=============================================================================
// Typedefs and Constants
//=============================================================================

/// Values returned by interface member functions.
typedef enum EOBJECT
{
   EObjOK = 0,        ///< Operation completed successfully.
   EObjNameNotFound,  ///< Name lookup failed.
   EObjDuplicateName, ///< Name already exists.
   EObjBadObject      ///< Invalid object state.
} EOBJECT;

typedef std::map<btID, btGenericInterface>                 iidInterfaceMap_t;
typedef std::map<btID, btGenericInterface>::const_iterator IIDINTERFACE_CITR;
typedef std::map<btID, btGenericInterface>::iterator       IIDINTERFACE_ITR;

/// Concrete base class for objects.
class AASLIB_API CAASBase : public    IBase,
                            protected CriticalSection
{
public:
   /// CAASBase construct with optional btApplicationContext.
   CAASBase(btApplicationContext Context=NULL);
   /// CAASBase Destructor.
   virtual ~CAASBase();

   // <IBase>
   btGenericInterface   Interface(btIID Interface)     const;
   virtual btBool             Has(btIID Interface)     const;
   btBool            operator != (IBase const &rother) const;
   btBool            operator == (IBase const &rother) const;
   btBool                    IsOK()                    const;
   btApplicationContext   Context()                    const;
   // </IBase>

   void SetContext(btApplicationContext context);

protected:
   EOBJECT     SetInterface(btIID              Interface,
                            btGenericInterface pInterface);

   EOBJECT ReplaceInterface(btIID              Interface,
                            btGenericInterface pInterface);

   btApplicationContext m_Context;
   btBool               m_bIsOK;

private:
   // No copying allowed
   CAASBase(const CAASBase & );
   CAASBase & operator = (const CAASBase & );

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4251)
#endif // _MSC_VER
   iidInterfaceMap_t  m_InterfaceMap;
#ifdef _MSC_VER
# pragma warning(pop)
#endif // _MSC_VER
};

/// Concrete base class for objects that generate events.
class AASLIB_API CAALBase : public CAASBase
{
public:
   /// CAALBase construct with Event Handler and Application-specific Context.
   CAALBase(btEventHandler       pEventHandler,
            btApplicationContext Context=NULL);

   /// Destroy this CAALBase.
   virtual void Destroy(TransactionID const &TransID) = 0;

protected:
   btEventHandler m_pEventHandler;

private:
   /// Cannot construct without event handler
   CAALBase();
   // No copying allowed
   CAALBase(const CAALBase & );
   CAALBase & operator = (const CAALBase & );
};


END_NAMESPACE(AAL)

#endif // __AALSDK_CAALBASE_H__

