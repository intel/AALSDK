// Copyright (c) 2014, Intel Corporation
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
/// file MAFUTransactions.cpp
/// brief Management AFU-specific events.
/// ingroup MAFU
/// verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Henry Mitchel, Intel Corporation
///          Joseph Grecco, Intel Corporation
///          Tim Whisonant, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 01/08/2014     TSW      Moved inline implementations from MAFUTransactions.h
///                          here. endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include "aalsdk/AALTypes.h"
#include "aalsdk/faptrans/MAFUTransactions.h"
#include "aalsdk/faptrans/MAFUTransService.h"

#if defined ( __AAL_WINDOWS__ )
# pragma warning(push)
# pragma warning(disable : 4996) // destination of copy is unsafe
#endif // __AAL_WINDOWS__

MAFUTRANS_BEGIN_MOD()
   /* No commands other than default, at the moment. */
MAFUTRANS_END_MOD()

#if defined ( __AAL_WINDOWS__ )
# pragma warning(pop)
#endif // __AAL_WINDOWS__


BEGIN_NAMESPACE(AAL)
   BEGIN_NAMESPACE(AAS)
      BEGIN_NAMESPACE(AIA)


//=============================================================================
// Name: Create_AFUTransaction
// Description:
// Parameters:
//=============================================================================
Create_AFUTransaction::Create_AFUTransaction(const NamedValueSet &manifest) :
   m_msg(),
   m_bIsOK(false),
   m_tidEmbedded(),
   m_ErrorString()
{
   memset(&m_msg, 0, sizeof(m_msg));
   if ( manifest.Has(MAFU_CONFIGURE_UEVENT) ) {
         // TODO Malloc size and get it

   } else {
      // Use the static struct
      m_msg.pmafu_createreq = &m_msg.mafu_createreq;
      m_msg.mafureq.size = sizeof(struct mafu_CreateAFU);
   }

   // Construct the request based on the manifest

   // Name
   if ( manifest.Has(MAFU_CONFIGURE_AFUNAME) ) {
      btcString name;
      manifest.Get(MAFU_CONFIGURE_AFUNAME,&name);
#if defined ( __AAL_WINDOWS__ )
# pragma warning(push)
# pragma warning(disable : 4996) // destination of copy is unsafe
#endif // __AAL_WINDOWS__

      strncpy(m_msg.pmafu_createreq->basename,name,MAFU_MAX_BASENAME_LEN);

#if defined ( __AAL_WINDOWS__ )
# pragma warning(pop)
#endif // __AAL_WINDOWS__
   } else {
      m_msg.pmafu_createreq->basename[0]='\0';
   }

   // Maxshares
   if ( manifest.Has(MAFU_CONFIGURE_SHAREMAX) ) {
      manifest.Get(MAFU_CONFIGURE_SHAREMAX,&m_msg.pmafu_createreq->maxshares);
   } else {
      m_msg.pmafu_createreq->maxshares=1;
   }

   // AFU ID
   if ( manifest.Has(MAFU_CONFIGURE_AFUIDL) ) {
      manifest.Get(MAFU_CONFIGURE_AFUIDL,reinterpret_cast<btUnsigned64bitInt*>(&m_msg.mafu_createreq.device_id.m_afuGUIDl));
   }

   if ( manifest.Has(MAFU_CONFIGURE_AFUIDH) ) {
      manifest.Get(MAFU_CONFIGURE_AFUIDH,reinterpret_cast<btUnsigned64bitInt*>(&m_msg.mafu_createreq.device_id.m_afuGUIDh));
   }

   if ( manifest.Has( MAFU_CONFIGURE_MANIFEST ) ) {

      manifest.Get( MAFU_CONFIGURE_MANIFEST,
                       static_cast<btByteArray*>(&m_msg.pmafu_createreq->manifest));

      manifest.GetSize(MAFU_CONFIGURE_MANIFEST,&m_msg.pmafu_createreq->manifest_size);
   } else {
      m_msg.pmafu_createreq->manifest=NULL;
      m_msg.pmafu_createreq->manifest_size=0;
   }

   // Set up the aalui_AFUmessage
   m_msg.afumsg.apiver   = GetAPIVer();
   m_msg.afumsg.pipver   = GetPIPVer();
   m_msg.afumsg.cmd      = aalui_mafucmd;
   m_msg.afumsg.payload  = reinterpret_cast<btVirtAddr>(&m_msg.mafureq);


   // Set up the mafu_request
   m_msg.mafureq.cmd     = aalui_mafucmdCreateAFU;
   m_msg.mafureq.payload = reinterpret_cast<btVirtAddr>(m_msg.pmafu_createreq);

   // All is good, m_ErrorString is ""
   m_bIsOK = true;
}

Create_AFUTransaction::~Create_AFUTransaction()
{
   if ( m_msg.pmafu_createreq == &m_msg.mafu_createreq ) {
      return;
   }
   // TODO Free pointer

}

btVirtAddr     Create_AFUTransaction::GetPayloadPtr() { return reinterpret_cast<btVirtAddr>(&m_msg); }
btWSSize       Create_AFUTransaction::GetSize()       { return sizeof(m_msg);     }
TransactionID *Create_AFUTransaction::GetTranIDPtr()  { return NULL;              }
uid_msgIDs_e   Create_AFUTransaction::GetCommand()    { return reqid_UID_SendAFU; }
btID           Create_AFUTransaction::GetPIPVer()     { return HOST_MAFUPIP_IID;  }
btID           Create_AFUTransaction::GetAPIVer()     { return HOST_MAFUAPI_IID;  }
btBool         Create_AFUTransaction::IsOK()          { return m_bIsOK;           }
std::string    Create_AFUTransaction::GetError()      { return m_ErrorString;     }


//=============================================================================
// Name: Destroy_AFUTransaction
// Description:
// Parameters:
//=============================================================================
Destroy_AFUTransaction::Destroy_AFUTransaction(const NamedValueSet &manifest) :
   m_msg(),
   m_bIsOK(false),
   m_tidEmbedded(),
   m_ErrorString()
{
   memset(&m_msg, 0, sizeof(m_msg));

   union {
      btByteArray     barr;
      afu_descriptor *pafdesc;
   } pdesc;

   // Get the work manifest.
   //   In the case of destroy it contains the afu_descriptor to destroy
   if ( manifest.Has(MAFU_CONFIGURE_MANIFEST) ) {
         // Malloc size and get it
      manifest.Get(MAFU_CONFIGURE_MANIFEST, &pdesc.barr);
   } else {
      return;
   }

   // Copy the descriptor
   m_msg.afudestroyreq.dev_desc = *pdesc.pafdesc;

   // Set up the aalui_AFUmessage
   m_msg.afumsg.apiver   = GetAPIVer();
   m_msg.afumsg.pipver   = GetPIPVer();
   m_msg.afumsg.cmd      = aalui_mafucmd;
   m_msg.afumsg.payload  = reinterpret_cast<btVirtAddr>(&m_msg.mafureq);

   // Set up the mafu_request
   m_msg.mafureq.cmd     = aalui_mafucmdDestroyAFU;
   m_msg.mafureq.payload = reinterpret_cast<btVirtAddr>(&m_msg.afudestroyreq);
   m_msg.mafureq.size    = sizeof(struct mafu_DestroyAFU);

   // All is good, m_ErrorString is ""
   m_bIsOK = true;
}

btVirtAddr     Destroy_AFUTransaction::GetPayloadPtr() { return reinterpret_cast<btVirtAddr>(&m_msg); }
btWSSize       Destroy_AFUTransaction::GetSize()       { return sizeof(m_msg);     }
TransactionID *Destroy_AFUTransaction::GetTranIDPtr()  { return NULL;              }
uid_msgIDs_e   Destroy_AFUTransaction::GetCommand()    { return reqid_UID_SendAFU; }
btID           Destroy_AFUTransaction::GetPIPVer()     { return HOST_MAFUPIP_IID;  }
btID           Destroy_AFUTransaction::GetAPIVer()     { return HOST_MAFUAPI_IID;  }
btBool         Destroy_AFUTransaction::IsOK()          { return m_bIsOK;           }
std::string    Destroy_AFUTransaction::GetError()      { return m_ErrorString;     }


      END_NAMESPACE(AIA)
   END_NAMESPACE(AAS)
END_NAMESPACE(AAL)


