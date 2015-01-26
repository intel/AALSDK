// Copyright (c) 2009-2015, Intel Corporation
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
/// file MAFUTransactions.h
/// brief Definitions for the AAL Management AFU C++ transactions
/// ingroup MAFU
/// verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHOR: Joseph Grecco, Intel Corporation.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT: endverbatim
//****************************************************************************
#ifndef __AALSDK_MAFUTRANSACTIONS_H__
#define __AALSDK_MAFUTRANSACTIONS_H__
#include <aalsdk/kernel/aalmafu.h>
#include <aalsdk/kernel/aalids.h>

#include <aalsdk/uAIALib.h>
#include <aalsdk/AALMAFU.h>


BEGIN_NAMESPACE(AAL)
   BEGIN_NAMESPACE(AAS)
      BEGIN_NAMESPACE(AIA)


//=============================================================================
// Name:          Create_AFUTransaction
// Description:   Create an AFU Transaction
// Parameters:    manifest - Contains all of the information required to create
//                           the desired device
// Comments: The mafu_CreateAFU structure is a variable length struct, with the
//           end of the struct containing optional uevent payload.  As an
//           optimization a minimal size mafu_CreateAFU is declared in the
//           transaction object and is used when no uevent data is present. If
//           uevent data is present then a malloced struct must be used. A
//           pointer points to the struct to use, The destuctor determines by
//           examining the pointer whether or not to free the buffer.
//=============================================================================
class Create_AFUTransaction : public IAFUTransaction
{
public:
   Create_AFUTransaction(const NamedValueSet &manifest);
   virtual ~Create_AFUTransaction();

   btVirtAddr     GetPayloadPtr();
   btWSSize       GetSize();
   TransactionID *GetTranIDPtr();
   uid_msgIDs_e   GetCommand();
   btID           GetPIPVer();         // Return the PIP version, burned in
   btID           GetAPIVer();         // Return the API version, burned in
   btBool         IsOK();
   std::string    GetError();          // Called if IsOK() fails

protected:
   struct big {
      struct aalui_AFUmessage afumsg;
      struct mafu_request     mafureq;
      struct mafu_CreateAFU  *pmafu_createreq;
      struct mafu_CreateAFU   mafu_createreq;
   };

   struct big    m_msg;
   btBool        m_bIsOK;
   TransactionID m_tidEmbedded;
   std::string   m_ErrorString;
};


//=============================================================================
// Name:          Destroy_AFUTransaction
// Description:   Destroy an AFU Transaction
// Parameters:    devicehandle
//=============================================================================
class Destroy_AFUTransaction : public IAFUTransaction
{
public:
   Destroy_AFUTransaction(const NamedValueSet &manifest);

   btVirtAddr     GetPayloadPtr();
   btWSSize       GetSize();
   TransactionID *GetTranIDPtr();
   uid_msgIDs_e   GetCommand();
   btID           GetPIPVer();         // Return the PIP version, burned in
   btID           GetAPIVer();         // Return the API version, burned in
   btBool         IsOK();
   std::string    GetError();          // Called if IsOK() fails

protected:
   struct big {
      struct aalui_AFUmessage afumsg;
      struct mafu_request     mafureq;
      mafu_DestroyAFU         afudestroyreq;
   };

   struct big    m_msg;
   btBool        m_bIsOK;
   TransactionID m_tidEmbedded;
   std::string   m_ErrorString;
};


      END_NAMESPACE(AIA)
   END_NAMESPACE(AAS)
END_NAMESPACE(AAL)


#endif // __AALSDK_MAFUTRANSACTIONS_H__

