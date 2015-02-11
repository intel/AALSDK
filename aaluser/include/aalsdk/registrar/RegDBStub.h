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
/// @file RegDBStub.h
/// @brief External char*-based interface to a Database Daemon.
/// @ingroup Registrar
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Henry Mitchel, Intel Corporation.
///          Joseph Grecco, Intel Corporation.
///
/// COMMENT: The RegSendMsg and RegRcvMsg functions handle the protocol with
///          the corresponding DBSendMsg and DBRcvMsg.
///          Their knowledge of each other is temporary
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 06/19/2008     HM       Registrar side of transport to/from Database
/// 07/03/2008     HM       Finalizing splitting of Registrar and Database
/// 09/12/2008     HM       Major checkin for new remote database
/// 01/04/2009     HM       Updated Copyright@endverbatim
//****************************************************************************
#ifndef __AALSDK_REGISTRAR_REGDBSTUB_H__
#define __AALSDK_REGISTRAR_REGDBSTUB_H__
#include <aalsdk/registrar/RegDBProxy.h>

BEGIN_NAMESPACE(AAL)

      //=============================================================================
      // RegKey I/O routines for RegistrarCmdResp
      //=============================================================================
      class RegKey;     // forward reference for MarshalCommand, declared below
      typedef RegKey ITR_t, *pITR_t;
      void  write_itr_to_RCP(pRegistrarCmdResp_t       pRCR, const ITR_t* pitr);
      void read_itr_from_RCP(const pRegistrarCmdResp_t pRCR,       pITR_t pitr);

      //=============================================================================
      // RegistrarCmdResp is the target of a serialization for marshalling.
      //    It cannot be a true class because if it is then it cannot have the
      //    terminating szNVS. As a non-class, it does not have ctor/dtor & member
      //    functions (otherwise it needs a function table pointer, and as a struct
      //    that would hose up the binary interface). Net result, class-type ctor &
      //    dtor and member functions are declared and implemented without being
      //    scoped by a class - thus the strange naming.
      //=============================================================================
      pRegistrarCmdResp_t RegistrarCommand_New(size_t lenszNVS);         // ctor for a struct RegistrarCmdResp as a Command
      pRegistrarCmdResp_t MarshalCommand(eCmd ,
                                         const TransactionID & ,
                                         pRegRetFunc ,
                                         const NamedValueSet * = NULL,
                                         const ITR_t         * = NULL);

      //=============================================================================
      // RegKey contains the iterator and lock fields that the Registrar needs,
      //    Without exposing the guts of the database.
      //    It is very similar to CRegDB, but that is for the database exclusively
      //=============================================================================
      class RegKey
      {
      public:
         DatabaseKey_t  m_PrimaryKey;
         LockKey_t      m_LockKey;
         RegKey(): m_PrimaryKey(NoKey), m_LockKey(Unlocked) {};
      };
//      std::ostream& operator << (std::ostream &s, const RegKey &dbr);


END_NAMESPACE(AAL)

#endif // __AALSDK_REGISTRAR_REGDBSTUB_H__

