// Copyright (c) 2007-2015, Intel Corporation
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
/// @file CAASRegistrar.h
/// @brief Registrar - Defines AAL Registrar subsystem
/// @ingroup Registrar
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Henry Mitchel, Intel Corporation.
///          Joseph Grecco, Intel Corporation.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 07/03/2007     HM       Initial version started
/// 08/22/2007     HM       Implemented FindResult, DBRecord, and
///                            events
/// 08/23/2007     HM       More tweaks to interface
/// 08/25/2007     HM       Heavy modifications to DBRecord for non-constness
/// 12/16/2007     JG       Changed include path to expose aas/
/// 03/28/2008     HM       Removed references to extraneous m_bIsOK
/// 05/08/2008     HM       Comments & License
/// 06/11/2008     HM       Fixes to go with RegistrarDatabase updates
/// 06/14/2008     HM       Added DEBUG_REGISTRAR & DumpDatabase enabled by it
/// 06/19/2008     HM       Changed from RegistrarDatabase.h to RegDBProxy.h
/// 06/27/2008     HM       Splitting Registrar from Database
/// 07/03/2008     HM       Finalizing splitting of Registrar and Database
/// 07/03/2008     HM       Added GetCopyOfFindResult(), GetCopyOfDBRecord()
/// 07/06/2008     HM       Added default constructor and copy / assignment
///                            as those are now allowed for FindResult and
///                            and DBRecord
/// 09/12/2008     HM       Major checkin for new remote database
/// 01/04/2009     HM       Updated Copyright
/// 04/25/2012     HM       Disabled irrelevant warning about export of
///                            template iterator for _WIN32 for private variable@endverbatim
//****************************************************************************
#ifndef __AALSDK_REGISTRAR_CAASREGISTRAR_H__
#define __AALSDK_REGISTRAR_CAASREGISTRAR_H__

//#define DEBUG_REGISTRAR  /* for earlier debugging */
//#define DBG_REG_CLIENT   /* for Client message pump debugging */

#include <aalsdk/registrar/AASRegistrar.h>       // client interface, this file defines object that implements it
#include <aalsdk/AALTransactionID.h>
#include <aalsdk/CAALEvent.h>
#include <aalsdk/osal/CriticalSection.h>
#include <aalsdk/registrar/RegDBStub.h>          // defines itr
#include <aalsdk/osal/Thread.h>

/// @todo Document IRegistrar, CRegistrar, and related.

//
// Debug logging macros
//
#ifdef DBG_REG_CLIENT
#  define DPOUT(m,...) do { m } while (0);
#else
#  define DPOUT(m,...) do {} while (0);
#endif
// Always want cerr
#define DPERR(m,...) do { m } while (0);


BEGIN_NAMESPACE(AAL)

      class CRegistrar;
      class CRegistrarDatabase;
      class CAASServiceContainer;
      class IEventDispatcher;

#if DEPRECATED
      //==========================================================================
      // Typedef for entry point signature
      //==========================================================================
      typedef  CRegistrar * (*Registrar_t)(const std::string    &DatabasePath,
                                           CAASServiceContainer *pServiceContainer,
                                           btEventHandler        theEventHandler,
                                           btApplicationContext  Context,
                                           TransactionID const  &tranID,
                                           NamedValueSet const  &optArgs );


      //==========================================================================
      // Name: _CreateRegistrarService Extern
      // Description: Registrar Factory Declaration
      //==========================================================================
 
BEGIN_C_DECLS
         AASREGISTRAR_API CRegistrar *
         _CreateRegistrarService(const std::string    &DatabasePath,
                                 CAASServiceContainer *pServiceContainer,
                                 btEventHandler        theEventHandler,
                                 btApplicationContext  Context,
                                 TransactionID const  &tranID,
                                 NamedValueSet const  &optArgs );
END_C_DECLS

#endif // DEPRECATED

      //=============================================================================
      // Name: CRegistrar
      // Description: Declaration of concrete class implementing IRegistrar
      //=============================================================================
      class AASREGISTRAR_API CRegistrar: public IRegistrar, public CAASBase
      {
      private:
         btEventHandler        m_theEventHandler;
         NamedValueSet         m_optArgs;
#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4251)
#endif // _MSC_VER
         std::string           m_DatabasePath;
#ifdef _MSC_VER
# pragma warning(pop)
#endif // _MSC_VER
         IEventDispatcher     *m_pEventDispatcher;
         btBool                m_DatabaseIsLoaded;
         btBool                m_fKeepRunning;            // Used in receive message thread
         btInt                 m_fdRMClient;              // File Descriptor of Resource Manager Client
                                                          // Initialized before use, if needed, in RegRcvMsg or RegSendMsg
#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4251)
#endif // _MSC_VER
         std::string           m_sResMgrClientDevName;    // Initialized in public constructor, below
#ifdef _MSC_VER
# pragma warning(pop)
#endif // _MSC_VER
         OSLThread            *m_pClientMP;               // Client Message Pump thread
         CSemaphore            m_Semaphore;               // General synchronization as needed



         CRegistrar();                       // Disable null constructor
         CRegistrar(const CRegistrar & );    // Disable copy & assignment constructors
         CRegistrar & operator = (const CRegistrar & );

         // Callback functions to handle return event from remote database
         void Open_ret        ( pRegistrarCmdResp_t pBlockToReceive );
         void Close_ret       ( pRegistrarCmdResp_t pBlockToReceive );
         void Register_ret    ( pRegistrarCmdResp_t pBlockToReceive );
         void Find_ret        ( pRegistrarCmdResp_t pBlockToReceive );
         void Get_ret         ( pRegistrarCmdResp_t pBlockToReceive );
         void Commit_ret      ( pRegistrarCmdResp_t pBlockToReceive );
         void DeRegister_ret  ( pRegistrarCmdResp_t pBlockToReceive );

         // Worker functions
         btBool DatabaseNotLoaded  (const TransactionID& rTransID, btID reasCode) const; // Checks if database loaded. If not, returns extranevtRegistrarDatabaseNotOpen
         void SemWait() {m_Semaphore.Wait();}
         void SemPost() {m_Semaphore.Post(1);}

         // Transport
         void        RegSendMsg    (pRegistrarCmdResp_t pBlockToSend) const;  // Send message to the database
         static void RegRcvMsg     (OSLThread *pThread, void *pContext);      // Message pump for receipt of return messages
         void        ParseResponse (pRegistrarCmdResp_t pBlockToReceive);     // Do initial parse of response
         btBool      InitClientFileDescriptor();                              // Initialize Client file descriptor

      public:
         CRegistrar (const std::string    &DatabasePath,
                     btEventHandler        theEventHandler,
                     btApplicationContext  Context,
                     TransactionID const  &tranID,
                     NamedValueSet const  &optArgs,
                     std::string           sResMgrClientDevName = "/dev/aalresmgr");
         virtual ~CRegistrar();

         btBool IsOK() const {return m_bIsOK;};

                                       // Open the database.
                                       // If rnvsOptArgs contains the pair RegistrarPathKeyName,"some path name",
                                       //    that pathname will be used for the registrar
         void Open(        const NamedValueSet& rnvsOptArgs,
                           const TransactionID& rTransID=TransactionID());

                                       // Close the database
         void Close(       const TransactionID& rTransID=TransactionID());

                                       // Insert a new record in the Database
         void Register(    const NamedValueSet& rRecord,
                           const TransactionID& rTransID=TransactionID());

                                       // Find an existing record in the Database
         void Find(        const NamedValueSet& rPattern,
                           const TransactionID& rTransID=TransactionID()) const;

                                       // Find next existing record in the Database
         void FindNext(    const NamedValueSet& rPattern,
                           const IFindResult&   rFindResult,
                           const TransactionID& rTransID=TransactionID()) const;

                                       // Retrieve a record for update based on a pattern
         void Get(         const NamedValueSet& rPattern,
                           const TransactionID& rTransID=TransactionID()) const;

                                       // Retrieve a record for update based on
                                       //    the result of a previous find
         void Get(         const IFindResult&   rFindResult,
                           const TransactionID& rTransID=TransactionID()) const;

                                       // Store a record after getting it
         void Commit(      const IDBRecord&     rRecord,
                           const TransactionID& rTransID=TransactionID());

                                       // Remove an existing record in the Database
                                       //    after retrieving for update
         void DeRegister(  const IDBRecord&     rRecord,
                           const TransactionID& rTransID=TransactionID());

         void DumpDatabase() const;    // For debugging. Left in production code in case needed, currently disabled

      }; // class CRegistrar

      //=============================================================================
      // Name: CFindResult
      // Description: Declaration of concrete class implementing IFindResult
      //=============================================================================
      class AASREGISTRAR_API CFindResult: public IFindResult, public CriticalSection
      {
      public:
         CFindResult ( const ITR_t& ritr, const NamedValueSet& rnvs):
               m_itr(ritr),
               m_nvs(rnvs)
               {}
         CFindResult ( const ITR_t& ritr):   // No valid nvs returned
               m_itr(ritr),
               m_nvs()                       // empty NVS
               {}
         CFindResult ()                      // default constructor empty for creation if client desires
               {}                            // default itr is no key no lock, default empty nvs
         CFindResult(const CFindResult &r)   // Copy constructor
               {m_itr = r.m_itr; m_nvs = r.m_nvs;};
         CFindResult & operator = (const CFindResult &r) { // Assignment, can throw bad_alloc from nvs copy
            if ( &r != this ) {
               m_itr = r.m_itr;
               m_nvs = r.m_nvs;
            }
            return *this;
         }
         ~CFindResult(){}

         const NamedValueSet& NVS() const    // Retrieve R/O reference to record
               { return m_nvs; };
         const ITR_t& itr() const            // Retrieve copy of the CRegDB
               { return m_itr; };

         // Use GetCopyOfFindResult to access the IFindResult outside of the Event Handler function.
         // Must delete the result when done with it.
         IFindResult* GetCopyOfFindResult() const
         {  CFindResult* p = new CFindResult ( m_itr,m_nvs ); return p;};

      private:
#if defined( _MSC_VER )
# pragma warning( push )
# pragma warning( disable:4251 )  // Cannot export template definitions
#endif // _MSC_VER

         ITR_t          m_itr;

#if defined( _MSC_VER )
# pragma warning( pop )
#endif // _MSC_VER

         NamedValueSet  m_nvs;
      }; // class CFindResult

      //=============================================================================
      // Name: CDBRecord
      // Description: Declaration of concrete class implementing IDBRecord
      //=============================================================================
      class AASREGISTRAR_API CDBRecord: public IDBRecord, public CriticalSection
      {
      public:
         CDBRecord ( const ITR_t& ritr, const NamedValueSet& rnvs):
               m_itr(ritr),               // Normal constructor indicates key and lock
               m_nvs(rnvs)
               {};
         CDBRecord ( const ITR_t& ritr):  // Failure constructure, key, no lock, no nvs
               m_itr(ritr),
               m_nvs()                    // empty NVS
               {};
         CDBRecord()                      // default constructor empty for creation if client desires
               {};                        // default itr is no key no lock, default empty nvs
         CDBRecord(const CDBRecord &r)             // Copy constructor
               {m_itr = r.m_itr; m_nvs = r.m_nvs;}
         CDBRecord & operator = (const CDBRecord &r) {  // Assignment, can throw bad_alloc from nvs copy
            if ( &r != this ) {
               m_itr = r.m_itr;
               m_nvs = r.m_nvs;
            }
            return *this;
         }
         virtual ~CDBRecord(){}
                                          // Get R/O reference passing to const functions, DB already locked
         const NamedValueSet &constNVS() const {return m_nvs;};
                                          // Get R/W reference, DB already locked
         NamedValueSet &NVS() {return m_nvs;};
                                          // Overwrite the record
         NamedValueSet &NVS(const NamedValueSet& rnvs)
               { m_nvs = rnvs; return m_nvs;};

         const ITR_t& itr() const         // Retrieve copy of the RegKey, used internally
               { return m_itr; };

         // Use GetCopyOfDBRecord to access the IFindResult outside of the Event Handler function.
         // Must delete the result when done with it.
         IDBRecord* GetCopyOfDBRecord() const
         {  CDBRecord* p = new CDBRecord ( m_itr,m_nvs ); return p;};

      private:
#if defined( _MSC_VER )
# pragma warning( push )
# pragma warning( disable:4251 )  // Cannot export template definitions
#endif // _MSC_VER

         ITR_t          m_itr;

#if defined( _MSC_VER )
# pragma warning( pop )
#endif // _MSC_VER

         NamedValueSet  m_nvs;
      }; // class CDBRecord

#if DEPRECATED

      // RegisterExceptionTransactionEvent

      class AASREGISTRAR_API CRegisterExceptionTransactionEvent:
         public IRegisterExceptionTransactionEvent,
         public CExceptionTransactionEvent
      {
      public:
         CRegisterExceptionTransactionEvent(
               IBase *pObject,
               TransactionID const &TranID,
               btID ExceptionNumber,
               btID Reason,
               btString Description,
               CFindResult* pCFindResult):
            CExceptionTransactionEvent(
               pObject,
               extranevtRegistrarRegister,
               TranID,
               ExceptionNumber,
               Reason,
               Description),
            m_pCFindResult(pCFindResult)
            {};
         virtual ~CRegisterExceptionTransactionEvent() {if (NULL != m_pCFindResult) delete m_pCFindResult;};

         const    IFindResult& FindResult() const {return *dynamic_cast<const IFindResult*>(m_pCFindResult);};
      private:
         const    CFindResult* m_pCFindResult;
         CRegisterExceptionTransactionEvent();
         CRegisterExceptionTransactionEvent(const CRegisterExceptionTransactionEvent & );
         CRegisterExceptionTransactionEvent & operator = (const CRegisterExceptionTransactionEvent & );
      };
#endif // DEPRECATED

      // RegisterTransactionEvent

      class AASREGISTRAR_API CRegisterTransactionEvent:
         public IRegisterTransactionEvent,
         public CTransactionEvent
      {
      public:
         CRegisterTransactionEvent( IBase *pObject,
                                    TransactionID const &TranID,
                                    btBool fRegisterSucceeded):
             CTransactionEvent(     pObject,
                                    TranID),
             m_fRegisterSucceeded ( fRegisterSucceeded)
             { SetInterface(tranevtRegistrarRegister,
                                    dynamic_cast<IRegisterTransactionEvent*>(this));
             };
         virtual ~CRegisterTransactionEvent() {};

         btBool   fRegisterSucceeded() const {return m_fRegisterSucceeded;};
      private:
         const    btBool   m_fRegisterSucceeded;
         CRegisterTransactionEvent();
         CRegisterTransactionEvent(const CRegisterTransactionEvent & );
         CRegisterTransactionEvent & operator = (const CRegisterTransactionEvent & );
      };

      // OpenTransactionEvent

      class AASREGISTRAR_API COpenTransactionEvent:
         public IOpenTransactionEvent,
         public CTransactionEvent
      {
      public:
         COpenTransactionEvent( IBase *pObject,
                                TransactionID const &TranID,
                                btBool fOpenSucceeded):
             CTransactionEvent( pObject,
                                TranID),
             m_fOpenSucceeded ( fOpenSucceeded)
             { SetInterface(tranevtRegistrarOpen,
                                    dynamic_cast<IOpenTransactionEvent*>(this));
             };
         virtual ~COpenTransactionEvent() {};

         btBool   fOpenSucceeded() const {return m_fOpenSucceeded;};
      private:
         const    btBool   m_fOpenSucceeded;
         COpenTransactionEvent();
         COpenTransactionEvent(const COpenTransactionEvent & );
         COpenTransactionEvent & operator = (const COpenTransactionEvent & );
      };

      // FindTransactionEvent

      class AASREGISTRAR_API CFindTransactionEvent:
         public IFindTransactionEvent,
         public CTransactionEvent
      {
      public:
         CFindTransactionEvent( IBase *pObject,
                                TransactionID const &TranID,
                                btBool fFindSucceeded,
                                CFindResult* pCFindResult):
             CTransactionEvent( pObject,
                                TranID),
             m_fFindSucceeded ( fFindSucceeded),
             m_pCFindResult   ( pCFindResult)
             { SetInterface(tranevtRegistrarFind,
                                    dynamic_cast<IFindTransactionEvent*>(this));
             };
         virtual ~CFindTransactionEvent() {if (NULL != m_pCFindResult) delete m_pCFindResult;};

         btBool   fFindSucceeded() const {return m_fFindSucceeded;};
         const    IFindResult& FindResult() const {return *dynamic_cast<const IFindResult*>(m_pCFindResult);};
      private:
         const    btBool       m_fFindSucceeded;
         const    CFindResult* m_pCFindResult;
         CFindTransactionEvent();
         CFindTransactionEvent(const CFindTransactionEvent & );
         CFindTransactionEvent & operator = (const CFindTransactionEvent & );
      };

      // GetTransactionEvent

      class AASREGISTRAR_API CGetTransactionEvent:
         public IGetTransactionEvent,
         public CTransactionEvent
      {
      public:
         CGetTransactionEvent( IBase *pObject,
                               TransactionID const &TranID,
                               btBool fGetSucceeded,
                               CDBRecord* pCDBRecord):
            CTransactionEvent( pObject,
                               TranID),
            m_fGetSucceeded  ( fGetSucceeded),
            m_pCDBRecord     ( pCDBRecord)
             { SetInterface(tranevtRegistrarGet,
                                    dynamic_cast<IGetTransactionEvent*>(this));
             };
         virtual ~CGetTransactionEvent() {if (NULL != m_pCDBRecord) delete m_pCDBRecord;};

         btBool   fGetSucceeded() const {return m_fGetSucceeded;};
                  IDBRecord& DBRecord() {return *dynamic_cast<IDBRecord*>(m_pCDBRecord);};
      private:
         const    btBool     m_fGetSucceeded;
         CDBRecord* const    m_pCDBRecord;
         CGetTransactionEvent();
         CGetTransactionEvent(const CGetTransactionEvent & );
         CGetTransactionEvent & operator = (const CGetTransactionEvent & );
      };

      // CommitTransactionEvent

      class AASREGISTRAR_API CCommitTransactionEvent:
               public ICommitTransactionEvent,
               public CTransactionEvent
      {
      public:
         CCommitTransactionEvent ( IBase *pObject,
                                   TransactionID const &TranID,
                                   btBool fCommitSucceeded,
                                   CFindResult* pCFindResult ) :
               CTransactionEvent ( pObject,
                                   TranID ),
               m_fCommitSucceeded ( fCommitSucceeded ),
               m_pCFindResult ( pCFindResult )
         {
            SetInterface ( tranevtRegistrarCommit,
                                   dynamic_cast<ICommitTransactionEvent*> ( this ) );
         };
         virtual ~CCommitTransactionEvent() {if ( NULL != m_pCFindResult ) delete m_pCFindResult;};

         btBool   fCommitSucceeded() const {return m_fCommitSucceeded;};
         const    IFindResult& FindResult() const {return *dynamic_cast<const IFindResult*> ( m_pCFindResult );};

      private:
         const    btBool   m_fCommitSucceeded;
         const    CFindResult* m_pCFindResult;
         CCommitTransactionEvent();
         CCommitTransactionEvent(const CCommitTransactionEvent & );
         CCommitTransactionEvent & operator = (const CCommitTransactionEvent & );
      };


END_NAMESPACE(AAL)

#endif // __AALSDK_REGISTRAR_CAASREGISTRAR_H__

