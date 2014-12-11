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
/// @file AALuAIA_Messaging.cpp
/// @brief Resolves recursive import/export dependency between AASLib and uAIA.
/// @ingroup uAIA
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHOR: Tim Whisonant, Intel Corporation.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 1/22/2013      TSW      Separating out UIDriverClient::msgPayload{} and
///                          UIDriverClient::uidrvManip to resolve recursive
///                          dependency resulting from AASLib on these.
/// 03/12/2013     JG       Fixed a bug where in Windows casting operator for
///                           TransactionID (stTransactionID &) did not work.
///                           This may be a compiler bug in Windows.@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include "aalsdk/uaia/AALuAIA_Messaging.h"


BEGIN_NAMESPACE(AAL)
   BEGIN_NAMESPACE(AAS)
      BEGIN_NAMESPACE(AIA)


IAFUTransaction::~IAFUTransaction() {}
IAFUCSRMap::~IAFUCSRMap() {}
IAFUDev::~IAFUDev() {}
IManagement::~IManagement() {}
IProvisioning::~IProvisioning() {}


UIDriverClient_msgPayload::UIDriverClient_msgPayload(btVirtAddr ptr, btWSSize size) :
   m_free(false),
   m_ptr(ptr),
   m_size(size)
{}

UIDriverClient_msgPayload::UIDriverClient_msgPayload(UIDriverClient_msgPayload const &rOther) :
   m_free(rOther.m_free),
   m_ptr(rOther.m_ptr),
   m_size(rOther.m_size)
{
   //TODO THIS WHOLE THING IS WEIRD AND NEEDS REFACTORING

   // Make sure only last copy will delete the buffer
   // This is a weird sort of clever pointer wrapper
   if ( rOther.m_free ) {
      const_cast<UIDriverClient_msgPayload &>(rOther).m_free = false;
   }

}

UIDriverClient_msgPayload::UIDriverClient_msgPayload(btWSSize size) :
   m_ptr(NULL),
   m_size(size)
{
   m_free = true;
   m_ptr  = (btVirtAddr)new btByte[(btUnsignedInt)size];
}

UIDriverClient_msgPayload::~UIDriverClient_msgPayload()
{
   if ( ( NULL != m_ptr ) && m_free ) {
      delete[] m_ptr;
   }
}



UIDriverClient_uidrvManip::UIDriverClient_uidrvManip(UIDriverClient_uidrvMarshaler_t fop,
                                                     uid_msgIDs_e                    cmd,
                                                     UIDriverClient_msgPayload       payload,
                                                     TransactionID const            &tranID,
                                                     uidrvMessageRoute              *mgsRoutep,
                                                     btObjectType                    dev) :
   m_fop(fop),
   m_cmd(cmd),
   m_payload(payload),
   m_mgsRoutep(mgsRoutep),
   m_DevObject(dev)
{
   stTransactionID_t temp = tranID;
// m_tid = (stTransactionID_t &)tranID;
   m_tid = tranID;
}

UIDriverClient_uidrvManip::~UIDriverClient_uidrvManip() {}


TranIDWrapper::TranIDWrapper(btObjectType argContext, const TransactionID &rTranID, btObjectType pCAFUDev_arg) :
   Context(argContext),
   origTranID(rTranID),
   pCAFUDev(pCAFUDev_arg)
{}


      END_NAMESPACE(AIA)
   END_NAMESPACE(AAS)
END_NAMESPACE(AAL)


