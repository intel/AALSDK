// Copyright (c) 2008-2014, Intel Corporation
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
/// @file RegDBSkeleton.cpp
/// @brief External char*-based interface to a Database Daemon.
/// @ingroup ResMgr
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Henry Mitchel, Intel Corporation.
///          Joseph Grecco, Intel Corporation.
///
/// COMMENT: Many of these functions are demarshalling/remarshalling functions.
///          They take a pRegistrarCmdResp_t, convert it to real arguments, and
///          call the RegistrarDatabase function, then remarshal the return
///          values.
///          The DBSendMsg and DBRcvMsg functions handle the protocol with
///          the corresponding RegSendMsg and RegRcvMsg.
///          Their knowledge of each other is temporary
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 06/19/2008     HM       Database side of Registrar/Database protocol
/// 07/03/2008     HM       Finalizing splitting of Registrar and Database
/// 08/07/2008     HM       Modifications for Resource Manager Daemon
/// 09/12/2008     HM       Major checkin for new remote database
/// 10/13/2008     HM       Convert to AALLogger
/// 11/03/2008     HM       Fixed MarshalResponse() to copy NVS's that contain
///                            embedded nulls
/// 11/09/2008     HM       Moved to Skeleton opening/closing the database
/// 11/30/2008     HM       Added terminating signal handler, modifying closing
///                            since now handled correctly
/// 01/04/2009     HM       Updated Copyright@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include "aalsdk/rm/RegDBSkeleton.h"
#include "aalsdk/AALLoggerExtern.h"

USING_NAMESPACE(std)


BEGIN_NAMESPACE(AAL)
   BEGIN_NAMESPACE(AAS)


//=============================================================================
// Name:          RegDBSkeleton
// Description:   Ctor for the skeleton
// Inputs:        DatabasePath is default path to the database directory
//=============================================================================
RegDBSkeleton::RegDBSkeleton(NamedValueSet *pOptArgs) :
   m_pDB(NULL),
   m_fIsOK(false)
{
   AAL_DEBUG(LM_Database,"RegDBSkeleton constructor invoked\n");
   m_pDB = new(std::nothrow) CRegistrarDatabase(pOptArgs);
   if ( m_pDB && m_pDB->IsOK() ) {
      m_fIsOK = true;
   }
}  // end of RegDBSkeleton::RegDBSkeleton

//=============================================================================
// Name:          ~RegDBSkeleton
// Description:   Dtor for the skeleton
//=============================================================================
RegDBSkeleton::~RegDBSkeleton()
{
   m_fIsOK = false;
   if ( m_pDB ) {
//      m_pDB->Close(); // Close will be done during destruction, if needed
      delete m_pDB;
      m_pDB = NULL;
   }
}

//=============================================================================
// Name: RegistrarResponse_New
// Description: Ctor for the RegistrarCmdResp_t structure as a Response.
//              True ctor not allowed, as not a true class. If make it a true
//              class then cannot have the terminating szNVS
// Inputs: pCmd is the command block for which this is a response, so most
//              fields should be copied over verbatim
//         lenszNVS is the length of the NVS that adds on to the end
//=============================================================================
pRegistrarCmdResp_t RegDBSkeleton::RegistrarResponse_New ( pRegistrarCmdResp_t pCmd,
                                                           size_t              lenszNVS)
{
   pRegistrarCmdResp_t pRC = reinterpret_cast<pRegistrarCmdResp_t>(new char[sizeof(RegistrarCmdResp_t) + lenszNVS]);
   if (pRC) {
      // copy all fixed fields from pCmd, including the signature. A little slower, but doing it this way
      //    eliminates any possibilities of strangeness if padding is added by the compiler.
      memcpy (pRC, pCmd, sizeof(RegistrarCmdResp_t));

      // write in Response signature
      memcpy (&pRC->szSignature, RegistrarCmdResponseSignature, lenRegistrarCmdRespSignature);

      // set the length of the nvs
      pRC->LengthOfNVS = lenszNVS;
      pRC->LengthOfStruct = lenszNVS + sizeof(RegistrarCmdResp_t);

      // clear the nvs area
      memset (&pRC->szNVS, 0, lenszNVS);
   }
   return pRC;
} // End of RegistrarResponse_New


//=============================================================================
// Name: MarshalResponse
// Description: Do the grunt work to create a command buffer
// Comments: pCmd points to the command buffer, copy most of it over
//           pnvs and prdb default to NULL for those uncomplicated situations
//=============================================================================
pRegistrarCmdResp_t RegDBSkeleton::MarshalResponse ( const pRegistrarCmdResp_t  pCmd,
                                                     const eReg                 Response,
                                                     const CRegDB*              prdb,
                                                     const NamedValueSet*       pnvs)
{
   size_t lenszNVS;
   ostringstream oss;

   if ( pnvs ) {
      oss << *pnvs;
      lenszNVS = oss.str().length() +1;
   } else {
      lenszNVS = 0;
   }

   pRegistrarCmdResp_t pRC = RegistrarResponse_New ( pCmd, lenszNVS );
   if ( pRC ) {
      pRC->Response = Response;
      if( prdb ){                      // if no RegDB, just leave the fields alone
         write_CRegDB_to_RCP( pRC, prdb);
      }
      if ( pnvs ) {
//         strcpy ( pRC->szNVS, oss.str().c_str() );
         oss.str().copy(pRC->szNVS,lenszNVS);
      }
   }
   return pRC;
}

#if 0
//=============================================================================
// Name:        DBRcvMsg
// Description: Database's Receive a Message from Registrar's Sender
// Comment:     In fact, the message is pushed here
//=============================================================================
void RegDBSkeleton::DBRcvMsg ( pRegistrarCmdResp_t pBlockToProcess)
{
   ParseCommand( pBlockToProcess);
}

//=============================================================================
// Name:        DBSendMsg
// Description: Database's Send Message to Registrar's Receiver
//=============================================================================
void RegDBSkeleton::DBSendMsg (pRegistrarCmdResp_t pBlockToSend)
{
   // recover the pIoctlReq
   if (1 == pBlockToSend->iContext) {

   }
   else {
      cerr << "RegDBSkeleton::DBSendMsg sees iContext!=1. Doing nothing.\n";
   }
}
// use ioctl to place pBlock to send on the kernel's queue
}
#endif

//=============================================================================
// Name: ParseCommand
// Description: Parse command, call function, return its result
//=============================================================================
pRegistrarCmdResp_t RegDBSkeleton::ParseCommand (pRegistrarCmdResp_t p)
{
   AAL_DEBUG(LM_Database,"RegDBSkeleton::ParseCommand seen\n");

   // check signature; not equal means different, therefore error
   if ( memcmp ( p->szSignature, RegistrarCmdCommandSignature, lenRegistrarCmdRespSignature ) ) {
      cerr << "RegDBSkeleton::ParseCommand szSignature not Command signature, aborting\n";
      // TODO: call global exception handler with EXCEPTION - not clear how to do this with the correct transaction ID
      return NULL;
   }

   AAL_DEBUG(LM_Database,"RegistrarCmdResp_t is:" << *p);

   switch ( p->Command ) {
      case eCmdOpen: {
         return Open_Skeleton ( p );
      }
      case eCmdClose: {
         return Close_Skeleton ( p );
      }
      case eCmdRegister: {
         return Register_Skeleton ( p );
      }
      case eCmdDelete: {
         return Delete_Skeleton ( p );
      }
      case eCmdGetByKey: {
         return GetByKey_Skeleton ( p );
      }
      case eCmdGetByPattern: {
         return GetByPattern_Skeleton ( p );
      }
      case eCmdCommit: {
         return Commit_Skeleton ( p );
      }
      case eCmdFindExactBegin: {
         return FindExactBegin_Skeleton ( p );
      }
      case eCmdFindExactNext: {
         return FindExactNext_Skeleton ( p );
      }
      case eCmdFindSubsetBegin: {
         return FindSubsetBegin_Skeleton ( p );
      }
      case eCmdFindSubsetNext: {
         return FindSubsetNext_Skeleton ( p );
      }
      case eCmdDumpDatabase: {
         return DumpDatabase_Skeleton ( p );
      }
      case eCmdIsOK: {
         return IsOK_Skeleton ( p );
      }
      default: {
         cerr << "RegDBSkeleton::ParseCommand: Unknown or unimplemented command \n";
         return NULL;
      }
   }  // End switch (p->Command)
}  // End of RegDBSkeleton::ParseCommand

//=============================================================================
// Name: Open_Skeleton
// TODO: Deprecate the Open_Skeleton interface
// Description: Unmarshal and call native Open()
// Interface: private
// Inputs: pointer to RegistrarCmdResp
// Outputs:
// Comments:
//=============================================================================
pRegistrarCmdResp_t RegDBSkeleton::Open_Skeleton ( pRegistrarCmdResp_t p )
{
   NamedValueSet nvs;
   read_nvs_from_RCP (p, &nvs);

   // New mode - do not allow external calls to open. The Database should already be open.
   eReg eRetVal;
   // eRetVal = m_pDB->Open(nvs);

   if (IsOK() && m_pDB->IsOK()) {
      eRetVal = eRegOK;
   } else {
      eRetVal = eRegDBNotLoaded;
   }

   return MarshalResponse (p, eRetVal);
}

//=============================================================================
// Name: Close_Skeleton
// TODO: Deprecate the Close_Skeleton interface
// Description: Unmarshal and call native Close()
// Interface: private
// Inputs: pointer to RegistrarCmdResp
// Outputs:
// Comments:
//=============================================================================
pRegistrarCmdResp_t RegDBSkeleton::Close_Skeleton ( pRegistrarCmdResp_t p )
{
   // New mode - do not allow external calls to open. The Database is closed at the end.
   // Reg eRetVal = m_pDB->Close();

   return MarshalResponse (p, eRegOK);
}

//=============================================================================
// Name: Register_Skeleton
// Description: Unmarshal and call native Register()
// Interface: private
// Inputs: pointer to RegistrarCmdResp
// Outputs:
// Comments:
//=============================================================================
pRegistrarCmdResp_t RegDBSkeleton::Register_Skeleton ( pRegistrarCmdResp_t p )
{
   NamedValueSet nvs;
   read_nvs_from_RCP (p, &nvs);

   CRegDB itr;
   eReg eRetVal = m_pDB->Register( nvs, itr);

   return MarshalResponse (p, eRetVal, &itr);
}

//=============================================================================
// Name: Delete_Skeleton
// Description: Unmarshal and call native Delete()
// Interface: private
// Inputs: pointer to RegistrarCmdResp
// Outputs:
// Comments:
//=============================================================================
pRegistrarCmdResp_t RegDBSkeleton::Delete_Skeleton ( pRegistrarCmdResp_t p )
{
   CRegDB itr;
   read_CRegDB_from_RCP (p, &itr);

   eReg eRetVal = m_pDB->Delete( itr);

   return MarshalResponse (p, eRetVal, &itr);   // itr is valid after call
}

//=============================================================================
// Name: GetByKey_Skeleton
// Description: Unmarshal and call native GetByKey()
// Interface: private
// Inputs: pointer to RegistrarCmdResp
// Outputs:
// Comments:
//=============================================================================
pRegistrarCmdResp_t RegDBSkeleton::GetByKey_Skeleton ( pRegistrarCmdResp_t p )
{
   CRegDB itr;
   read_CRegDB_from_RCP (p, &itr);

   eReg eRetVal = m_pDB->GetByKey( itr);       // itr is modified

   return MarshalResponse (p, eRetVal, &itr, itr.NVS());
}

//=============================================================================
// Name: GetByPattern_Skeleton
// Description: Unmarshal and call native GetByPattern()
// Interface: private
// Inputs: pointer to RegistrarCmdResp
// Outputs:
// Comments:
//=============================================================================
pRegistrarCmdResp_t RegDBSkeleton::GetByPattern_Skeleton ( pRegistrarCmdResp_t p )
{
   NamedValueSet nvs;
   read_nvs_from_RCP (p, &nvs);

   CRegDB itr;
   eReg eRetVal = m_pDB->GetByPattern( nvs, itr);  // itr is modified

   return MarshalResponse (p, eRetVal, &itr, itr.NVS());
}

//=============================================================================
// Name: Commit_Skeleton
// Description: Unmarshal and call native Commit()
// Interface: private
// Inputs: pointer to RegistrarCmdResp
// Outputs:
// Comments:
//=============================================================================
pRegistrarCmdResp_t RegDBSkeleton::Commit_Skeleton ( pRegistrarCmdResp_t p )
{
   NamedValueSet nvs;
   read_nvs_from_RCP (p, &nvs);

   CRegDB itr;
   read_CRegDB_from_RCP (p, &itr);

   eReg eRetVal = m_pDB->Commit( nvs, itr);

   return MarshalResponse (p, eRetVal, &itr, itr.NVS());
}

//=============================================================================
// Name: FindExactBegin_Skeleton
// Description: Unmarshal and call native FindExactBegin()
// Interface: private
// Inputs: pointer to RegistrarCmdResp
// Outputs:
// Comments:
//=============================================================================
pRegistrarCmdResp_t RegDBSkeleton::FindExactBegin_Skeleton ( pRegistrarCmdResp_t p )
{
   cout << "FindExactBegin_Skeleton not yet implemented\n" ;
   return NULL;
}

//=============================================================================
// Name: FindExactNext_Skeleton
// Description: Unmarshal and call native FindExactNext()
// Interface: private
// Inputs: pointer to RegistrarCmdResp
// Outputs:
// Comments:
//=============================================================================
pRegistrarCmdResp_t RegDBSkeleton::FindExactNext_Skeleton ( pRegistrarCmdResp_t p )
{
   cout << "FindExactNext_Skeleton not yet implemented\n" ;
   return NULL;
}

//=============================================================================
// Name: FindSubsetBegin_Skeleton
// Description: Unmarshal and call native FindSubsetBegin()
// Interface: private
// Inputs: pointer to RegistrarCmdResp
// Outputs:
// Comments:
//=============================================================================
pRegistrarCmdResp_t RegDBSkeleton::FindSubsetBegin_Skeleton ( pRegistrarCmdResp_t p )
{
   NamedValueSet nvs;
   read_nvs_from_RCP (p, &nvs);

   CRegDB itr;
   eReg eRetVal = m_pDB->FindSubsetBegin( nvs, itr);

   return MarshalResponse (p, eRetVal, &itr, itr.NVS());
}

//=============================================================================
// Name: FindSubsetNext_Skeleton
// Description: Unmarshal and call native FindSubsetNext()
// Interface: private
// Inputs: pointer to RegistrarCmdResp
// Outputs:
// Comments:
//=============================================================================
pRegistrarCmdResp_t RegDBSkeleton::FindSubsetNext_Skeleton ( pRegistrarCmdResp_t p )
{
   NamedValueSet nvs;
   read_nvs_from_RCP (p, &nvs);

   CRegDB itr;
   read_CRegDB_from_RCP (p, &itr);

   eReg eRetVal = m_pDB->FindSubsetNext( nvs, itr);

   return MarshalResponse (p, eRetVal, &itr, itr.NVS());
}

//=============================================================================
// Name: DumpDatabase_Skeleton
// Description: Unmarshal and call native DumpDatabase()
// Interface: private
// Inputs: pointer to RegistrarCmdResp
// Outputs:
// Comments:
//=============================================================================
pRegistrarCmdResp_t RegDBSkeleton::DumpDatabase_Skeleton( pRegistrarCmdResp_t p )
{
   m_pDB->DumpDatabase();
   return NULL;
}

//=============================================================================
// Name: IsOK_Skeleton
// Description: Unmarshal and call native IsOK()
// Interface: private
// Inputs: pointer to RegistrarCmdResp
// Outputs:
// Comments:
//=============================================================================
pRegistrarCmdResp_t RegDBSkeleton::IsOK_Skeleton ( pRegistrarCmdResp_t p )
{
   cout << "IsOK_Skeleton not yet implemented\n" ;
   return NULL;
}


RegDBSkeleton::RegDBSkeleton(const RegDBSkeleton & ) {/*empty*/}
RegDBSkeleton & RegDBSkeleton::operator=(const RegDBSkeleton & ) { return *this; }


//=============================================================================
// Name: write_CRegDB_to_RCP
// Description: write the contents of an itr out to a RegistrarCmdResp
// Use like this:
//    CRegDB itr;
//    pRegistrarCmdResp_t p;
//    AAL::AAS::write_CRegDB_to_RCP (p, &itr)
// Comments: Note similar code in RegDBStub.cpp
//=============================================================================
void write_CRegDB_to_RCP(pRegistrarCmdResp_t pRCR, const CRegDB *pitr)
{
   pRCR->Key  = pitr->m_PrimaryKey;
   pRCR->Lock = pitr->m_LockKey;
}

//=============================================================================
// Name: read_CRegDB_from_RCP
// Description: read the contents of an "itr", a generic iterator, in from
//              a RegistrarCmdResp
// Use like this:
//    CRegDB itr;
//    pRegistrarCmdResp_t p;
//    AAL::AAS::read_CRegDB_from_RCP (p, &itr)
// Comments: Note similar code in RegDBStub.cpp
//=============================================================================
void read_CRegDB_from_RCP(const pRegistrarCmdResp_t pRCR, CRegDB *pitr)
{
   pitr->m_PrimaryKey = pRCR->Key;
   pitr->m_LockKey    = pRCR->Lock;
}


   END_NAMESPACE(AAS)
END_NAMESPACE(AAL)

