// Copyright (c) 2008-2015, Intel Corporation
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
/// @file RegDBSkeleton.h
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
/// 07/09/2008     HM       #def'd out debug string
/// 08/07/2008     HM       Modifications for Resource Manager Daemon
/// 09/12/2008     HM       Major checkin for new remote database
/// 11/13/2008     HM       Fixed constructor bug that allowed incomplete
///                            construction to pass unnoticed
/// 01/04/2009     HM       Updated Copyright@endverbatim
//****************************************************************************
#ifndef __AALSDK_RM_REGDBSKELETON_H__
#define __AALSDK_RM_REGDBSKELETON_H__
#include <aalsdk/rm/CRegistrarDatabase.h>     // includes RegDBProxy.h in turn


BEGIN_NAMESPACE(AAL)
   BEGIN_NAMESPACE(AAS)


//=============================================================================
// RegistrarCmdResp_t is the target of a serialization for marshalling.
//    It cannot be a true class because if it is then it cannot have the
//    terminating szNVS. As a non-class, it does not have ctor/dtor & member
//    functions (otherwise it needs a function table pointer, and as a struct
//    that would hose up the binary interface). Net result, class-type ctor &
//    dtor and member functions are declared and implemented without being
//    scoped by a class - thus the strange naming.
//=============================================================================
class RegDBSkeleton
{
   CRegistrarDatabase *m_pDB;
   btBool              m_fIsOK;
public:
   RegDBSkeleton(NamedValueSet *pOptArgs);                     // Ctor
   virtual ~RegDBSkeleton();                                   // Dtor
   btBool IsOK() { return m_fIsOK && m_pDB && m_pDB->IsOK(); } // Check before use
   CRegistrarDatabase * Database () const { return m_pDB; }    // accessor for a local database user


   // ctor for a struct RegistrarCmdResp_t as a Response
   pRegistrarCmdResp_t RegistrarResponse_New(pRegistrarCmdResp_t , size_t );

   // utility functions
   pRegistrarCmdResp_t MarshalResponse(const pRegistrarCmdResp_t  pCmd,
                                       const eReg                 Response,
                                       const CRegDB              *prdb = NULL,
                                       const NamedValueSet       *pnvs = NULL);

   // Driver for skeleton functions below
   pRegistrarCmdResp_t ParseCommand            (pRegistrarCmdResp_t pBlockToReceive);

   // Skeleton functions that unmarshall, call database, remarshall, and return
   pRegistrarCmdResp_t Open_Skeleton           (pRegistrarCmdResp_t pBlockToReceive);
   pRegistrarCmdResp_t Close_Skeleton          (pRegistrarCmdResp_t pBlockToReceive);
   pRegistrarCmdResp_t Register_Skeleton       (pRegistrarCmdResp_t pBlockToReceive);
   pRegistrarCmdResp_t Delete_Skeleton         (pRegistrarCmdResp_t pBlockToReceive);
   pRegistrarCmdResp_t GetByKey_Skeleton       (pRegistrarCmdResp_t pBlockToReceive);
   pRegistrarCmdResp_t GetByPattern_Skeleton   (pRegistrarCmdResp_t pBlockToReceive);
   pRegistrarCmdResp_t Commit_Skeleton         (pRegistrarCmdResp_t pBlockToReceive);
   pRegistrarCmdResp_t FindExactBegin_Skeleton (pRegistrarCmdResp_t pBlockToReceive);
   pRegistrarCmdResp_t FindExactNext_Skeleton  (pRegistrarCmdResp_t pBlockToReceive);
   pRegistrarCmdResp_t FindSubsetBegin_Skeleton(pRegistrarCmdResp_t pBlockToReceive);
   pRegistrarCmdResp_t FindSubsetNext_Skeleton (pRegistrarCmdResp_t pBlockToReceive);
   pRegistrarCmdResp_t DumpDatabase_Skeleton   (pRegistrarCmdResp_t pBlockToReceive);
   pRegistrarCmdResp_t IsOK_Skeleton           (pRegistrarCmdResp_t pBlockToReceive);

private:
   // No copying allowed
   RegDBSkeleton(const RegDBSkeleton & );
   RegDBSkeleton & operator = (const RegDBSkeleton & );
};

//=============================================================================
// CRegDB I/O routines for RegistrarCmdResp
//=============================================================================
void  write_CRegDB_to_RCP(pRegistrarCmdResp_t pRCR, const CRegDB *pitr);
void read_CRegDB_from_RCP(const pRegistrarCmdResp_t pRCR, CRegDB *pitr);


   END_NAMESPACE(AAS)
END_NAMESPACE(AAL)

#endif // __AALSDK_RM_REGDBSKELETON_H__

