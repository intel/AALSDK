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
/// @file AALTransactionID.h
/// @brief Defines TransactionID class.
/// @ingroup Events
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Joseph Grecco, Intel Corporation
///          Henry Mitchel, Intel Corporation
///
/// HISTORY: Initial version transaction ID was an [out] parameter whose value
///          was exclusively set by the system.
///          04/11/06 - Made transaction ID an [in|out] object that carries an
///                     application specified context. In addition the Tid has
///                     been extended to provide a listener, used to scope per
///                     per transaction event handling.
///
/// WHEN:          WHO:     WHAT:
/// 01/04/2006     JG       Initial version started
/// 04/11/2006     JG       Extended TransactionID as above
/// 03/13/2007     JG       Added a constructor for AppContext
///                         only and removed default eventhandler
///                         argument which caused a potential
///                         unwanted type coversion of type void*
///                         to transaction ID when passing to an
///                         argument expecting a tranID &
/// 03/19/2007     JG       Linux Port
/// 08/20/2007     JG       Added assignment operator
/// 10/04/2007     JG       Renamed to AALTranssactionID.h
/// 05/08/2008     HM       Comments & License
/// 05/16/2008     HM       Add true ID field to TransactionID, an int
/// 11/10/2008     JG       Replaced discrete member variables with typedef
///                         Added support for stTransactionID_t operators
/// 12/08/2008     HM/JG    Added new TransactionID ctor and fixed random intID
/// 01/04/2009     HM       Updated Copyright
/// 08/12/2010     HM       Added new CTOR: Application specified ID and Handler
/// 10/21/2011     JG       Changes for Windows compatibility
/// 05/15/2015     JG       Added Support for IBase@endverbatim
//****************************************************************************
#ifndef __AALSDK_AALTRANSACTIONID_H__
#define __AALSDK_AALTRANSACTIONID_H__
#include <aalsdk/AALTypes.h>
#include <aalsdk/kernel/AALTransactionID_s.h>

#ifdef __cplusplus

BEGIN_NAMESPACE(AAL)

/// @addtogroup Events
/// @{

//Creates a new transaction ID
AASLIB_API bt32bitInt _AssignTranID();

//=============================================================================
/// @brief TransactionIDs provide handles for interacting with AAS.
/// @todo finish filling me in.
///
/// TransactionIDs are simple, small passive objects that are directly constructed and copied in the usual C++
/// manner.
/// - TransactionID::Context: carries context information for the transaction, e.g. a pointer to an
/// object or a structure containing state that is specific to this transaction. It is provided in
/// addition to the standard Object Context.
/// - TransactionID::ID: a 32-bit integer that is by default initialized to a unique number, but may be set by
/// the user to any desired value. As a unique number it can be used directly to match a function invocation with
/// its returned event. If set by the user, it can represent any useful tidbit of information, e.g. an array
/// index indicating which buffer is being manipulated.
/// - TransactionID::EventHandler: a pointer to an Event Handler Function. If non-NULL, this function will be
/// called by the Transaction Method.
/// - TransactionID::Filter: a Boolean, which if true, “filters out” the call to the Default Event Handler after the call to the
/// EventHandler in the TransactionID is made. Filter is only meaningful if EventHandler is non-NULL. If false,
/// then no filtering is performed; that is, both the Event Handler in the TransactionID (if non-NULL) is called,
/// and then the default Event Handler is called.
//=============================================================================
class AASLIB_API TransactionID
{
public:
   /// @brief Default Constructor
   ///
   /// ID is system assigned and unique, EventHandler is NULL, Context is NULL
   TransactionID();

   /// @brief Construct from structure
   /// @param[in] stTid XXX from kernel ioctl?
   TransactionID(stTransactionID_t stTid);

   /// @brief Copy Constructor
   /// @param[in] rOther TransactionID to copy
   TransactionID(TransactionID const &rOther);

   /// @brief Application specified ID
   /// @param[in] ID application context
   TransactionID(btApplicationContext ID);

   /// @brief Specified Integer ID
   /// @param intID 32-bit integer
   TransactionID(bt32bitInt intID);

   /// @brief Application specified Context and Handler
   /// @param ID
   /// @param EventHandler
   /// @param Filter
   TransactionID(btApplicationContext ID,
                 btEventHandler       EventHandler,
                 btBool               Filter = true);

   /// @brief Application specified ID and Handler
   /// @param intID
   /// @param EventHandler
   /// @param Filter
   TransactionID(bt32bitInt     intID,
                 btEventHandler EventHandler,
                 btBool         Filter=true);

   /// @brief Application specified ID, Context, Handler, Filter
   /// @param intID
   /// @param ID
   /// @param EventHandler
   /// @param Filter
   TransactionID(bt32bitInt           intID,
                 btApplicationContext ID,
                 btEventHandler       EventHandler,
                 btBool               Filter=true);

   /// @brief System assigned ID application provided Handler.
   /// @param EventHandler
   /// @param Filter
   TransactionID(btEventHandler EventHandler,
                 btBool         Filter=true);


  /// @brief Application specified Context and Handler
   /// @param ID
   /// @param iBase
   /// @param Filter
   TransactionID(btApplicationContext         ID,
                 IBase                       *pIBase,
                 btBool                       Filter = true);

   /// @brief Application specified ID and Handler
   /// @param intID
   /// @param iBase
   /// @param Filter
   TransactionID(bt32bitInt                   intID,
                 IBase                       *pIBase,
                 btBool                       Filter=true);

   /// @brief Application specified ID, Context, Handler, Filter
   /// @param intID
   /// @param ID
   /// @param iBase
   /// @param Filter
   TransactionID(bt32bitInt                   intID,
                 btApplicationContext         ID,
                 IBase                       *pIBase,
                 btBool                       Filter=true);

   /// @brief System assigned ID application provided Handler.
   /// @param iBase
   /// @param Filter
   TransactionID(IBase                       *pIBase,
                 btBool                       Filter=true);

   /// TransactionID Destructor.
   virtual ~TransactionID();

   //Accessors
   btApplicationContext        Context() const;
   btEventHandler              Handler() const;
   IBase                      *Ibase() const;
   btBool                      Filter()  const;
   bt32bitInt                  ID()      const;

   // Mutators
   void Context(btApplicationContext Context)             { m_tid.m_ID = Context;                }
   void Handler(btEventHandler Handler)                   { m_tid.m_Handler = Handler;           }
   void Ibase(IBase *pIBase)                              { m_tid.m_IBase = pIBase;              }
   void Filter(btBool Filter)                             { m_tid.m_Filter = Filter;             }
   void ID(bt32bitInt intID)                              { m_tid.m_intID = intID;               }

   TransactionID & operator = (TransactionID const &rOther) {
      m_tid = rOther.m_tid;
      return *this;
   }

   TransactionID & operator = (stTransactionID_t const &tid) {
      m_tid = tid;
      return *this;
   }

   operator stTransactionID_t const & () const {
      return m_tid;
   }

   operator stTransactionID_t & () {
      return m_tid;
   }

   btBool operator < (const TransactionID &rOther) const {
      return m_tid.m_intID < rOther.m_tid.m_intID;
   }

   btBool operator > (const TransactionID  &rOther) const{
       return m_tid.m_intID > rOther.m_tid.m_intID;
   }

private:
   stTransactionID_t  m_tid;
};

/// TransactionID streamer.
AASLIB_API std::ostream & operator << (std::ostream &s, const TransactionID &TranID);

/// @}

#endif //__cplusplus

END_NAMESPACE(AAL)

#endif // __AALSDK_AALTRANSACTIONID_H__

