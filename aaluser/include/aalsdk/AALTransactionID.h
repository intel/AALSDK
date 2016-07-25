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
/// @file AALTransactionID.h
/// @brief Defines TransactionID class.
/// @ingroup Events
/// @verbatim
/// Accelerator Abstraction Layer
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
#include <aalsdk/osal/CriticalSection.h>
#include <aalsdk/kernel/AALTransactionID_s.h>

#ifdef __cplusplus

BEGIN_NAMESPACE(AAL)

/// @addtogroup Events
/// @{

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
   ///@brief Default Constructor
   //
   /// ID is system assigned and unique, EventHandler is NULL, Context is NULL
   /// @return void
   TransactionID();

   /// @brief Construct from structure.
   // @param[in] stTid XXX from kernel ioctl?
   /// @param[in] stTid A reference to a TransactionID structure.
   /// @return void
   TransactionID(const stTransactionID_t &stTid);

   /// @brief Copy Constructor.
   /// @param[in] rOther TransactionID to copy.
   /// @return void
   TransactionID(const TransactionID &rOther);

   /// @brief Construct using system assigned unique ID, default event handler
   ///        and application specified context.
   /// @param[in] Context Application specified context.
   /// @return void
   TransactionID(btApplicationContext Context);

   /// @brief Construct using application specified integer ID.
   /// @param[in] intID 32-bit integer initialized to a random number if not
   ///                  supplied by the caller.
   /// @return void
   TransactionID(btID                 intID);

   /// @brief Construct using default Event Handler and application specified
   ///        integer ID and Context.
   /// @param[in] intID 32-bit integer initialized to a random number if not
   ///                  supplied by the caller.
   /// @param[in] Context Application specified context.
   /// @return void
   TransactionID(btID                 intID,
                 btApplicationContext Context);

   /// @brief Construct using system assigned unique ID, and application
   ///        provided Context, Handler and optional Event Filter setting.
   /// @param[in] Context Application specified context.
   /// @param[in] EventHandler A pointer to an Event Handler Function.
   /// @param[in] Filter If true, the Event is not sent to the default
   ///                   handler after the supplied EventHandler processes
   ///                   it.
   /// @return void
   TransactionID(btApplicationContext Context,
                 btEventHandler       EventHandler,
                 btBool               Filter=true);

   /// @brief Construct using application specified integer ID, Handler and
   ///        optional Event Filter setting.
   /// @param[in] intID 32-bit integer initialized to a random number if not
   ///                  supplied by the caller.
   /// @param[in] EventHandler A pointer to an Event Handler Function
   /// @param[in] Filter If true, the Event is not sent to the default
   ///                   handler after the supplied EventHandler processes
   ///                   it.
   /// @return void
   TransactionID(btID                 intID,
                 btEventHandler       EventHandler,
                 btBool               Filter=true);

   /// @brief Construct using application specified integer ID, Context,
   ///        Handler and optional Event Filter setting.
   /// @param[in] intID 32-bit integer initialized to a random number if not
   ///                  supplied by the caller.
   /// @param[in] Context Application specified context.
   /// @param[in] EventHandler A pointer to an Event Handler Function
   /// @param[in] Filter If true, the Event is not sent to the default
   ///                   handler after the supplied EventHandler processes
   ///                   it.
   /// @return void
   TransactionID(btID                 intID,
                 btApplicationContext Context,
                 btEventHandler       EventHandler,
                 btBool               Filter=true);

   /// @brief Construct using system assigned unique ID, and application
   ///        provided Handler and optional Event Filter setting.
   /// @param[in] EventHandler A pointer to an Event Handler Function
   /// @param[in] Filter If true, the Event is not sent to the default
   ///                   handler after the supplied EventHandler processes
   ///                   it.
   /// @return void
   TransactionID(btEventHandler       EventHandler,
                 btBool               Filter=true);


   /// @brief Construct using system assigned ID, and application provided
   ///        Context, iBase interface pointer, and optional Event Filter
   ///        setting.
   /// @param[in] Context Application specified context.
   /// @param[in] pIBase A pointer to an IBase interface.
   /// @param[in] Filter If true, the Event is not sent to the default
   ///                   handler after the supplied EventHandler processes
   ///                   it.
   /// @return void
   TransactionID(btApplicationContext Context,
                 IBase               *pIBase,
                 btBool               Filter=true);

   /// @brief Construct using application specified ID,  an IBase interface
   ///        pointer, and optional Event Filter setting.
   /// @param[in] intID 32-bit integer initialized to a random number if not
   ///                  supplied by the caller.
   /// @param[in] pIBase A pointer to an IBase interface.
   /// @param[in] Filter If true, the Event is not sent to the default
   ///                   handler after the supplied EventHandler processes
   ///                   it.
   /// @return void
   TransactionID(btID                 intID,
                 IBase               *pIBase,
                 btBool               Filter=true);

   /// @brief Construct using application specified ID, Context, an IBase
   ///        interface pointer, and optional Event Filter setting.
   /// @param[in] intID 32-bit integer initialized to a random number if not
   ///                  supplied by the caller.
   /// @param[in] Context Application specified context.
   /// @param[in] pIBase A pointer to an IBase interface.
   /// @param[in] Filter If true, the Event is not sent to the default
   ///                   handler after the supplied EventHandler processes
   ///                   it.
   /// @return void
   TransactionID(btID                 intID,
                 btApplicationContext Context,
                 IBase               *pIBase,
                 btBool               Filter=true);

   /// @brief Construct using system assigned ID, and application provided
   ///        IBase interface pointer and optional Event Filter setting.
   /// @brief System assigned ID application provided Handler.
   /// @param[in] pIBase A pointer to an IBase interface.
   /// @param[in] Filter If true, the Event is not sent to the default
   ///                   handler after the supplied EventHandler processes
   ///                   it.
   /// @return void
   TransactionID(IBase               *pIBase,
                 btBool               Filter=true);

   // TransactionID Destructor.
   virtual ~TransactionID();

   // Accessors
   /// @brief Access the TransactionID application context.
   /// @return The application context set in the TransactionID.
   btApplicationContext Context() const;
   /// @brief Access the TransactionID Event Handler.
   /// @return The event handler set in the TransactionID.
   btEventHandler       Handler() const;
   /// @brief Access the TransactionID interface.
   /// @return A pointer to the IBase interface Event Handler in the
   ///         TransactionID.
   IBase                 *Ibase() const;
   /// @brief Access the TransactionID interface pointer.
   /// @retval True if the TransactionID Event Handler filter is set.
   /// @retval False if the TransactionID Event Handler filter is not set.
   btBool                Filter() const;
   /// @brief Access the TransactionID ID.
   /// @return The ID of the TransactionID.
   btID                      ID() const;

   // Mutators
   /// @brief Set the TransactionID application context.
   /// @param[in] Context The application context to set in the TransactionID.
   /// @return void
   void Context(btApplicationContext Context);
   /// @brief Set the TransactionID Event Handler.
   /// @param[in] Handler The event handler to set in the TransactionID.
   /// @return void
   void Handler(btEventHandler       Handler);
   /// @brief Access the TransactionID interface pointer.
   /// @param[in] pIBase A pointer to an IBase interface to set in the TransactionID.
   /// @return void
   void   Ibase(IBase               *pIBase);
   /// @brief Set the TransactionID Filter value.
   /// @param[in] Filter The event Filter behavior to set in the TransactionID.
   /// @return void
   void  Filter(btBool               Filter);
   /// @brief Set the TransactionID ID.
   /// @param[in] intID The ID to set in the TransactionID.
   /// @return void
   void      ID(btID                 intID);

   /// @brief Assignment operator - assigns the transaction ID.
   /// <B>Parameters:</B> [in]  A reference to a TransactionID to copy the
   ///                          transaction ID from.
   TransactionID & operator = (const TransactionID     & );
   /// @brief Assignment operator - assigns the transaction ID.
   /// <B>Parameters:</B> [in]  A reference to a transaction ID to set.
   TransactionID & operator = (const stTransactionID_t & );

   /// @brief Operator to access the transaction ID.
   /// @retval A reference to the transaction ID.
   operator const stTransactionID_t & () const;
   /// @brief Operator to access the transaction ID.
   /// @return A reference to the transaction ID.
   operator stTransactionID_t &       ();

   /// @brief Equality operator - Are the transactions equivalent?
   /// <B>Parameters:</B> [in]  A reference to a TransactionID to compare.
   /// @retval True if the TransactionID is equal.
   /// @retval False if the TransactionID in not equal.
   btBool operator == (const TransactionID & ) const;

   /// @brief Get a unique ID for a TransactionID.
   /// @return A randomly generated 32-bit integer ID.
   static btID NextUniqueID();

private:
   stTransactionID_t m_tid;

   static btID            sm_NextUniqueID;
   static CriticalSection sm_CS;
};

/// TransactionID streamer.
/// @param[in] s       A reference to the stream.
/// @param[in] TranID  A reference to the TransactionID to stream.
/// @return void
AASLIB_API std::ostream & operator << (std::ostream &s, const TransactionID &TranID);

/// @}

#endif //__cplusplus

END_NAMESPACE(AAL)

#endif // __AALSDK_AALTRANSACTIONID_H__

