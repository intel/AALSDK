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
/// @file CAASRegistrar.cpp
/// @brief CAASRegistrar - Implementation of Registrar.
/// @ingroup Registrar
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Henry Mitchel, Intel Corporation.
///          Joseph Grecco, Intel Corporation.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 07/02/2007     HM       Initial version started
/// 08/12/2007     HM       Integrating with back-end database
/// 08/23/2007     HM       Redoing interface after figuring out
///                            how it should REALLY work
/// 08/25/2007     HM       Additional tweaks as debugging progresses
/// 12/16/2007     JG       Changed include path to expose aas/
/// 02/03/2008     HM       Cleaned up various things brought out by ICC
/// 03/28/2008     HM       Removed references to extraneous m_bIsOK
/// 05/08/2008     HM       Comments & License
/// 06/11/2008     HM       Fixes to go with RegistrarDatabase updates
/// 06/11/2008     HM       Added nvs args path processing to Open
/// 06/14/2008     HM       Added DEBUG_REGISTRAR & DumpDatabase enabled by it
///                         Many fixes brought out during testing with nvs2
///                            testing framework
/// 06/27/2008     HM       Splitting Registrar from Database
/// 08/07/2008     HM       Modifications for Resource Manager Daemon
/// 01/04/2009     HM       Updated Copyright
/// 07/15/2009     HM       Added Shutdown logging@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H


#include "aalsdk/CAALEvent.h"
#include "aalsdk/registrar/CAASRegistrar.h"  // <aas/RegDBStub.h>
#include "aalsdk/registrar/RegDBProxy.h"
#include "aalsdk/eds/AASEventDeliveryService.h"

#include "aalsdk/kernel/aalrm.h"             // kernel transport services

#include "aalsdk/AALLoggerExtern.h"          // Logger


#ifdef __ICC                           /* Deal with Intel compiler-specific overly sensitive remarks */
//   #pragma warning( push)
//   #pragma warning(disable:68)       // warning: integer conversion resulted in a change of sign.
                                       //    This is intentional
     #pragma warning(disable:177)      // remark: variable "XXXX" was declared but never referenced- OK
//   #pragma warning(disable:383)      // remark: value copied to temporary, reference to temporary used
//   #pragma warning(disable:593)      // remark: variable "XXXX" was set but never used - OK
     #pragma warning(disable:869)      // remark: parameter "XXXX" was never referenced
//   #pragma warning(disable:981)      // remark: operands are evaluated in unspecified order
     #pragma warning(disable:1418)     // remark: external function definition with no prior declaration
//   #pragma warning(disable:1419)     // remark: external declaration in primary source file
//   #pragma warning(disable:1572)     // remark: floating-point equality and inequality comparisons are unreliable
//   #pragma warning(disable:1599)     // remark: declaration hides variable "Args", or tid - OK
#endif


//=============================================================================
// Name: _CreateRegistrarService
// Description: Public entry to the Factory
// Interface: public
// Inputs: - Class name.
// Outputs: Pointer to factory.
// Comments:
//=============================================================================
AASREGISTRAR_API AAL::CRegistrar *
CreateRegistrarService(AAL::btcString            DatabasePath,
                       AAL::btEventHandler       theEventHandler,
                       AAL::btApplicationContext Context,
                       AAL::btcObjectType        _tranID,
                       AAL::btcObjectType        _optArgs)
{
   //Note that this model could result in leaks if the user does not destroy

   // If the Registrar fails to start for some reason an event will be posted
   // with the details

   const AAL::TransactionID *tranID = reinterpret_cast<const AAL::TransactionID *>(_tranID);
   ASSERT(NULL != tranID);
   if ( NULL == tranID ) {
      return NULL;
   }

   const AAL::NamedValueSet *optArgs = reinterpret_cast<const AAL::NamedValueSet *>(_optArgs);
   ASSERT(NULL != optArgs);
   if ( NULL == optArgs ) {
      return NULL;
   }

   AAL::CRegistrar *pRegistrar = new AAL::CRegistrar(std::string(DatabasePath),
                                                     theEventHandler,
                                                     Context,
                                                     *tranID,
                                                     *optArgs);
   if ( ( NULL != pRegistrar ) &&
        !pRegistrar->IsOK() ) {
      //Destroy the pRegistrar
      delete pRegistrar;
      pRegistrar = NULL;
   }

   return pRegistrar;
}


BEGIN_NAMESPACE(AAL)


//=============================================================================
// Name: CRegistrar
// Description: Constructor
// Interface: public
// Inputs: none
// Outputs: none
// Comments:
//=============================================================================
AASREGISTRAR_API
    CRegistrar::CRegistrar(const std::string    &DatabasePath,
                           btEventHandler        theEventHandler,
                           btApplicationContext  Context,
                           TransactionID const  &tranID,
                           NamedValueSet const  &optArgs,
                           std::string           sResMgrClientDevName )
     :CAASBase(Context),
      m_theEventHandler(theEventHandler),
      m_optArgs(optArgs),
      m_DatabasePath(DatabasePath),
//      m_pDB(NULL),
      m_pEventDispatcher(NULL),
      m_DatabaseIsLoaded(false),
      m_fKeepRunning(true),
      m_fdRMClient(-1),          // Flag value means uninitialized file handle
      m_sResMgrClientDevName(sResMgrClientDevName),
      m_pClientMP(NULL)
{
   AutoLock(this);
   m_bIsOK = false;              // CAASBase set it to true

   if(SetSubClassInterface(iidRegistrar,dynamic_cast<IRegistrar*>(this))!= EObjOK)
      return;                    // m_bIsOK will be false

   // Initialize the semaphore
   m_Semaphore.Create(0,INT_MAX);

   // Create the Message delivery thread
   m_pClientMP = new OSLThread( RegRcvMsg,
                                OSLThread::THREADPRIORITY_NORMAL,
                                this);
   // Make sure that the kernel pipe to the database is open. It is initialized in RegRcvMsg.
   SemWait();

   // Happiness
   m_bIsOK = true;
}


//=============================================================================
// Name: ~CRegistrar
// Description: Destructor
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments:
//=============================================================================
AASREGISTRAR_API CRegistrar::~CRegistrar()
{
//   AutoLock(this);

//   m_State = Shutdown;    // may want something like this for later if the shutdown gets more complicated

   struct aalrm_ioctlreq req;
   memset(&req, 0, sizeof(req));

   req.id = reqid_Shutdown;
   req.size  =0;
   req.payload = NULL;
   req.tranID = (stTransactionID_t &)TransactionID(123456);
   req.context = (void*)67890;

   DEBUG_CERR("~CRegistrar: Sending Service Shutdown Request. 1 of 6\n");
   AAL_DEBUG(LM_Shutdown, "~CRegistrar: Sending Service Shutdown Request. 1 of 6\n");

   if (ioctl (m_fdRMClient, AALRM_IOCTL_SENDMSG, &req) == -1){
      DEBUG_CERR("~CRegistrar: Service Shutdown Request failed. Reason code "
                 << pAALLogger()->GetErrorString(errno) << endl);
      AAL_ERR(LM_Registrar, "~CRegistrar: Service Shutdown Request failed. Reason code "
            << pAALLogger()->GetErrorString(errno) << std::endl);
   }

   m_bIsOK = false;

   DEBUG_CERR("~CRegistrar: m_pClientMP is " << (void*)m_pClientMP << ". 2 of 6\n");
   AAL_DEBUG(LM_Shutdown, "~CRegistrar: m_pClientMP is " << (void*)m_pClientMP << ". 2 of 6\n");

   // message pump is running, need to wait for it to terminate
   if ( m_pClientMP != NULL ) {
      // Wait for the Message delivery thread to terminate
      DEBUG_CERR("~CRegistrar: waiting for Receive Thread to Join. 3 of 6\n");
      AAL_DEBUG(LM_Shutdown, "~CRegistrar: waiting for Receive Thread to Join. 3 of 6\n");

      m_pClientMP->Join();

      DEBUG_CERR("~CRegistrar: Receive Thread has Joined. 4 of 6\n");
      AAL_DEBUG(LM_Shutdown, "~CRegistrar: Receive Thread has Joined. 4 of 6\n");

      delete m_pClientMP;
      m_pClientMP = NULL;
   }

   // Close the physical device
   DEBUG_CERR("~CRegistrar: shutting down RM Client file. 5 of 6\n");
   AAL_DEBUG(LM_Shutdown,"~CRegistrar: shutting down RM Client file. 5 of 6\n");
   if( -1 != m_fdRMClient ) {
      int ret = close( m_fdRMClient );
      if(ret != 0) {
         DEBUG_CERR("~CRegistrar: shutting down RM Client file failed. Reason code "
                    << pAALLogger()->GetErrorString(errno) << std::endl);
         AAL_ERR(LM_Shutdown, "~CRegistrar: shutting down RM Client file failed. Reason code "
               << pAALLogger()->GetErrorString(errno) << std::endl);
      }
      m_fdRMClient = -1;
   }
   DEBUG_CERR("~CRegistrar: done. 6 of 6\n");
   AAL_INFO(LM_Shutdown, "~CRegistrar: done. 6 of 6\n");
}  // End of CRegistrar::~CRegistrar


//=============================================================================
// Name: Open
// Description: Open the database
// Interface: public
// Inputs: optArgs, to be used in the future if the need arises
// Outputs: if error, thrown TransactionExceptionEvent and return false
// Comments:
//=============================================================================
AASREGISTRAR_API
void CRegistrar::Open(const NamedValueSet& rnvsOptArgs,
                      const TransactionID& rTransID)
{
   AutoLock(this);

   // Open the database
   pRegistrarCmdResp_t pRC = MarshalCommand( eCmdOpen, rTransID, &CRegistrar::Open_ret, &rnvsOptArgs);

   if( pRC ){
      AAL_DEBUG(LM_Registrar, "Open Call, Command Block is:\n" << pRC << std::endl);
      RegSendMsg (pRC);
      RegistrarCmdResp_Destroy (pRC);
   }
}  // end of CRegistrar::Open

//=============================================================================
// Name: Open_ret
// Description: Return function after opening the database
// Interface: private
// Inputs: pointer to RegistrarCmdResp
// Outputs:
// Comments:
//=============================================================================
void CRegistrar::Open_ret( pRegistrarCmdResp_t p)
{
   AutoLock(this);

   if (eRegOK == p->Response) {
      m_DatabaseIsLoaded = true;
   }
   else {
      m_DatabaseIsLoaded = false;
   }
   AAL_DEBUG(LM_Registrar, "CRegistrar::Open, DumpDatabase after CRegistrarDatabase::Open\n");
   #if defined(AAL_LOG_LEVEL) && (AAL_LOG_LEVEL >= LOG_DEBUG)
   if (pAALLogger()->IfLog(LM_Registrar,LOG_DEBUG)) DumpDatabase();
   #endif

   m_pEventDispatcher->QueueEvent( m_theEventHandler,
      new COpenTransactionEvent(
         dynamic_cast<IBase*>(this),
         p->TranID,
         m_DatabaseIsLoaded));
}  // end of CRegistrar::Open_ret

//=============================================================================
// Name: Close
// Description: Close the database
// Interface: public
// Inputs: optArgs, to be used in the future if the need arises
// Outputs: if error, thrown TransactionExceptionEvent and return false
// Comments:
//=============================================================================
AASREGISTRAR_API
void CRegistrar::Close(const TransactionID& rTransID)
{
   AutoLock(this);

   // Check that the database is loaded
   if (DatabaseNotLoaded( rTransID, reasRegClose))
      return;

   // Open the database
   pRegistrarCmdResp_t pRC = MarshalCommand( eCmdClose, rTransID, &CRegistrar::Close_ret);

   if( pRC ){
      AAL_DEBUG(LM_Registrar,"Close Call, Command Block is:\n" << pRC << std::endl);
      RegSendMsg (pRC);
      RegistrarCmdResp_Destroy (pRC);
   }
}  // end of CRegistrar::Close

//=============================================================================
// Name: Close_ret
// Description: Return function after closing the database
// Interface: private
// Inputs: pointer to RegistrarCmdResp
// Outputs:
// Comments:
//=============================================================================
void CRegistrar::Close_ret( pRegistrarCmdResp_t p)
{
   AutoLock(this);

   m_pEventDispatcher->QueueEvent( m_theEventHandler,
      new CTransactionEvent(
         dynamic_cast<IBase*>(this),
         tranevtRegistrarClose,
         p->TranID));
}  // end of CRegistrar::Close_ret

//=============================================================================
// Name: CRegistrar::Register
// Description: Insert a new record in the Database
// Interface: public
// Inputs: in parameter list
// Outputs: Transaction or TransactionException
// Comments:
//=============================================================================
AASREGISTRAR_API
void CRegistrar::Register(NamedValueSet const& rRecord,
                      const TransactionID& rTransID)
{
   AutoLock(this);

   // Check that the database is loaded
   if (DatabaseNotLoaded( rTransID, reasRegRegister))
      return;

   // CRegDB itr;                               // not necessary, but consistent
   // eReg eRetVal = m_pDB->Register(rRecord, itr);    // Insert it with a new key

   // Insert with a new key
   pRegistrarCmdResp_t pRC = MarshalCommand( eCmdRegister, rTransID,
                                             &CRegistrar::Register_ret,
                                             &rRecord);
   if ( pRC ) {
      AAL_DEBUG(LM_Registrar, "Register Call, Command Block is:\n" << pRC << std::endl);
      RegSendMsg (pRC);
      RegistrarCmdResp_Destroy (pRC);
   }
}  // end of CRegistrar::Register

//=============================================================================
// Name: CRegistrar::Register_ret
// Description: Return function after inserting a new record in the Database
// Interface: private
// Inputs: pointer to RegistrarCmdResp
// Outputs:
// Comments:
//=============================================================================
void CRegistrar::Register_ret    ( pRegistrarCmdResp_t p )
{
   AutoLock(this);

//   ITR_t itr;      // could at some point marshal back the itr if application
                     // e.g. wanted to immediately have its handle

   m_pEventDispatcher->QueueEvent( m_theEventHandler,
      new CRegisterTransactionEvent(
         dynamic_cast<IBase*>(this),
         p->TranID,
         eRegOK==p->Response));

   // TODO: really need an exception event possibly several different ones depending on
   // what the return value was

#if 0 /* deprecated */
   {                                      // Already one in the database, return false
      m_pEventDispatcher->QueueEvent( m_theEventHandler,
         new CRegisterExceptionTransactionEvent(
            dynamic_cast<IBase*>(this),
            rTransID,
            errRegRegisterFailed,
            reasRegDuplicateEntry,
            strRegDuplicateEntry,
            new CFindResult( itr, NVS(itr))));
   }
   else
   {
      m_pDB->Insert(rRecord);             // Insert it and return success
      m_pEventDispatcher->QueueEvent( m_theEventHandler, new CTransactionEvent(
            dynamic_cast<IBase*>(this),
            tranevtRegistrarRegister,
            rTransID));
   }
#endif
}

//=============================================================================
// Name: CRegistrar::Find
// Description: Find a record in the Database, from the beginning, using subset
// Interface: public
// Inputs: in parameter list
// Outputs: Transaction or TransactionException
// Comments:
//=============================================================================
AASREGISTRAR_API
void CRegistrar::Find(const NamedValueSet& rPattern,
                      const TransactionID& rTransID) const
{
   AutoLock(this);

   // Check that the database is loaded
   if (DatabaseNotLoaded( rTransID, reasRegFind))
      return;

   pRegistrarCmdResp_t pRC = MarshalCommand( eCmdFindSubsetBegin,
                                             rTransID,
                                             &CRegistrar::Find_ret,
                                             &rPattern);
   if ( pRC ) {
      AAL_DEBUG(LM_Registrar,"Find Call, Command Block is:\n" << pRC << std::endl);
      RegSendMsg (pRC);
      RegistrarCmdResp_Destroy (pRC);
   }
}  // end of CRegistrar::Find

//=============================================================================
// Name: CRegistrar::FindNext
// Description: Find the next record in the Database
// Interface: public
// Inputs: in parameter list
// Outputs: Transaction or TransactionException
// Comments:
//=============================================================================
AASREGISTRAR_API
void CRegistrar::FindNext(const NamedValueSet& rPattern,
                          const IFindResult& rFindResult,
                          const TransactionID& rTransID) const
{
   AutoLock(this);

   // Check that the database is loaded
   if (DatabaseNotLoaded( rTransID, reasRegFindNext))
      return;

   pRegistrarCmdResp_t pRC = MarshalCommand( eCmdFindSubsetNext,
                                             rTransID,
                                             &CRegistrar::Find_ret,  // exactly the same return
                                             &rPattern,
                                             &dynamic_cast<const CFindResult&>(rFindResult).itr());
   if ( pRC ) {
      AAL_DEBUG(LM_Registrar, "FindNext Call, Command Block is:\n" << pRC << std::endl);
      RegSendMsg (pRC);
      RegistrarCmdResp_Destroy (pRC);
   }
}  // end of CRegistrar::FindNext

//=============================================================================
// Name: CRegistrar::Find_ret
// Description: Return function after finding a record in the Database
// Interface: private
// Inputs: pointer to RegistrarCmdResp
// Outputs:
// Comments:
//=============================================================================
void CRegistrar::Find_ret ( pRegistrarCmdResp_t p )
{
   AutoLock(this);

   ITR_t itr;
   read_itr_from_RCP (p, &itr);

   NamedValueSet nvs;
   read_nvs_from_RCP (p, &nvs);

   btBool fFound = (eRegOK == p->Response);

   m_pEventDispatcher->QueueEvent( m_theEventHandler,
      new CFindTransactionEvent(
         dynamic_cast<IBase*>(const_cast<CRegistrar*>(this)),
         p->TranID,
         fFound,
         fFound ? new CFindResult( itr, nvs)       // copies the nvs, would like a better way
                : new CFindResult( itr)
         ));
} // end of Find_ret

//=============================================================================
// Name: CRegistrar::Get (Pattern)
// Description: Find a record in the Database, and retrieve a locked mutable copy
// Interface: public
// Inputs: in parameter list
// Outputs: Transaction or TransactionException
// Comments:
//=============================================================================
AASREGISTRAR_API
void CRegistrar::Get(const NamedValueSet& rPattern,
                     const TransactionID& rTransID) const
{
   AutoLock(this);

   // Check that the database is loaded
   if (DatabaseNotLoaded( rTransID, reasRegFind))
      return;

   pRegistrarCmdResp_t pRC = MarshalCommand( eCmdGetByPattern,
                                             rTransID,
                                             &CRegistrar::Get_ret,
                                             &rPattern);
   if ( pRC ) {
      AAL_DEBUG(LM_Registrar, "Get_By_Pattern Call, Command Block is:\n" << pRC << std::endl);
      RegSendMsg (pRC);
      RegistrarCmdResp_Destroy (pRC);
   }
}  // end of CRegistrar::Get (Pattern)

//=============================================================================
// Name: CRegistrar::Get (FindResult)
// Description: Retrieve a locked mutable copy of a database record based
//    the result of a previous find
// Interface: public
// Inputs: in parameter list
// Outputs: Transaction or TransactionException
// Comments:
//=============================================================================
AASREGISTRAR_API
void CRegistrar::Get(const IFindResult& rFindResult,
                     const TransactionID& rTransID) const
{
   AutoLock(this);

   // Check that the database is loaded
   if (DatabaseNotLoaded( rTransID, reasRegFindNext))
      return;

   pRegistrarCmdResp_t pRC = MarshalCommand( eCmdGetByKey,
                                             rTransID,
                                             &CRegistrar::Get_ret,
                                             NULL,                   // no pattern
                                             &dynamic_cast<const CFindResult&>(rFindResult).itr());
   if ( pRC ) {
      AAL_DEBUG(LM_Registrar,"GetFindResult Call, Command Block is:\n" << pRC << std::endl);
      RegSendMsg (pRC);
      RegistrarCmdResp_Destroy (pRC);
   }
}  // end of CRegistrar::Get (FindResult)

//=============================================================================
// Name: CRegistrar::Get_ret
// Description: Handle return after retrieving a locked mutable copy of a
//              database record
// Interface: private
// Inputs:
// Outputs:
// Comments:
//=============================================================================
void CRegistrar::Get_ret ( pRegistrarCmdResp_t p )
{
   AutoLock(this);

   ITR_t itr;
   read_itr_from_RCP (p, &itr);

   NamedValueSet nvs;
   read_nvs_from_RCP (p, &nvs);

   btBool fFound = (eRegOK == p->Response);

   m_pEventDispatcher->QueueEvent( m_theEventHandler,
      new CGetTransactionEvent(
         dynamic_cast<IBase*>(const_cast<CRegistrar*>(this)),
         p->TranID,
         fFound,
         fFound ? new CDBRecord( itr, nvs)       // copies the nvs, would like a better way
                : new CDBRecord( itr)
         ));
}  // end of CRegistrar::Get_ret

//=============================================================================
// Name: CRegistrar::Commit
// Description: Store a record from a CDBRecord into the database
// Interface: public
// Inputs: in parameter list
// Outputs: Transaction or TransactionException
// Comments:
//=============================================================================
AASREGISTRAR_API
void CRegistrar::Commit(const IDBRecord& rRecord,
                        const TransactionID& rTransID)
{
   AutoLock(this);

   // Check that the database is loaded
   if (DatabaseNotLoaded( rTransID, reasRegFindNext))
      return;

   const CDBRecord& cdbr = dynamic_cast<const CDBRecord&>(rRecord);

   pRegistrarCmdResp_t pRC = MarshalCommand( eCmdCommit,
                                             rTransID,
                                             &CRegistrar::Commit_ret,
                                             &cdbr.constNVS(),
                                             &cdbr.itr());
   if ( pRC ) {
      AAL_DEBUG(LM_Registrar,"Commit Call, Command Block is:\n" << pRC << std::endl);
      RegSendMsg (pRC);
      RegistrarCmdResp_Destroy (pRC);
   }
}  // end of CRegistrar::Commit

//=============================================================================
// Name: CRegistrar::Commit_ret
// Description: Handle return after store a record from a CDBRecord into the database
// Interface: private
// Inputs:
// Outputs: Transaction or TransactionException
// Comments:
//=============================================================================
void CRegistrar::Commit_ret ( pRegistrarCmdResp_t p )
{
   AutoLock ( this );

   ITR_t itr;
   read_itr_from_RCP ( p, &itr );

   NamedValueSet nvs;
   read_nvs_from_RCP ( p, &nvs );

   btBool fFound = ( eRegOK == p->Response );

   m_pEventDispatcher->QueueEvent ( m_theEventHandler,
                                    new CCommitTransactionEvent (
                                       dynamic_cast<IBase*> ( this ),
                                       p->TranID,
                                       fFound,
                                       fFound ? new CFindResult ( itr, nvs )     // copies the nvs, would like a better way
                                              : new CFindResult ( itr )
                                    ) );
}  // end of CRegistrar::Commit_ret

//=============================================================================
// Name: CRegistrar::DeRegister
// Description: Remove an existing record in the Database
//    after retrieving for update
// Interface: public
// Inputs: in parameter list
// Outputs: Transaction or TransactionException
// Comments:
//=============================================================================
AASREGISTRAR_API
void CRegistrar::DeRegister(const IDBRecord& rRecord,
                            const TransactionID& rTransID)
{
   AutoLock(this);

   // Check that the database is loaded
   if (DatabaseNotLoaded( rTransID, reasRegDeRegister))
      return;

   pRegistrarCmdResp_t pRC = MarshalCommand( eCmdDelete,
                                             rTransID,
                                             &CRegistrar::DeRegister_ret,
                                             NULL,
                                             &dynamic_cast<const CDBRecord&>(rRecord).itr());
   if ( pRC ) {
      AAL_DEBUG(LM_Registrar,"DeRegister Call, Command Block is:\n" << pRC << std::endl);
      RegSendMsg (pRC);
      RegistrarCmdResp_Destroy (pRC);
   }
}  // end of CRegistrar::DeRegister

//=============================================================================
// Name: CRegistrar::DeRegister_ret
// Description: Remove an existing record in the Database
//    after retrieving for update
// Interface: public
// Inputs: in parameter list
// Outputs: Transaction or TransactionException
// Comments:
//=============================================================================
void CRegistrar::DeRegister_ret ( pRegistrarCmdResp_t p )
{
   AutoLock(this);

   // Note that iterator is valid, if desired. It is set to
   // unlocked, primary key left untouched

   m_pEventDispatcher->QueueEvent( m_theEventHandler,
      new CTransactionEvent(
         dynamic_cast<IBase*>(this),
         tranevtRegistrarDeRegister,
         p->TranID));

}  // end of CRegistrar::DeRegister_ret

//=============================================================================
// Name:          CRegistrar::DatabaseNotLoaded
// Description:   Checks if database loaded. If not, returns
//                   extranevtRegistrarDatabaseNotOpen
// Interface:     public
// Inputs:        in parameter list
// Outputs:       Transaction or TransactionException
// Comments:      Worker routine for main interfaces, factored out code
//=============================================================================
AASREGISTRAR_API
btBool CRegistrar::DatabaseNotLoaded(const TransactionID &rTransID,
                                           btID           reasCode) const
{
   AutoLock(this);

   // Check that the database is loaded
   if (m_DatabaseIsLoaded) {
      return false;                    // Database is not NotLoaded
   }

   // Not loaded, ship exception and return true
   m_pEventDispatcher->QueueEvent( m_theEventHandler,
      new CExceptionTransactionEvent(
         dynamic_cast<IBase*>(const_cast<CRegistrar*>(this)),
         extranevtRegistrarDatabaseNotOpen,
         rTransID,
         errRegDatabaseNotLoaded,
         reasCode,
         strRegDatabaseNotLoaded));

   AAL_INFO(LM_Registrar,"Registrar Database is Not Loaded\n");

   return true;
}  // end of CRegistrar::DatabaseNotLoaded

//=============================================================================
// Name: CRegistrar::DumpDatabase
// Description: prints database to cout
// Interface: public - although should be protected somehow
// Inputs:
// Outputs:
// Comments:   No return routine defined, no return expected, fire and forget
//=============================================================================
void CRegistrar::DumpDatabase() const
{
   AutoLock(this);

   TransactionID tid;
   pRegistrarCmdResp_t pRC = MarshalCommand( eCmdDumpDatabase,
                                             tid,
                                             NULL);
   if ( pRC ) {
      AAL_DEBUG(LM_Registrar, "DumpDatabase Call, Command Block is:\n" << pRC << std::endl);
      RegSendMsg (pRC);
      RegistrarCmdResp_Destroy (pRC);
   }
}


END_NAMESPACE(AAL)


