// Copyright(c) 2015-2016, Intel Corporation
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
/// @file UIDriverInterfaceAdapter.h
/// @brief Defines the object wrapper for the AAL Universal Device Driver.
/// @ingroup AIA
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHOR: Joseph Grecco, Intel Corporation.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 09/02/2015     JG       Created @endverbatim
//****************************************************************************
#ifndef __AALSDK_AIASERVICE_UIDRVERINTERFACEADAPTER_H__
#define __AALSDK_AIASERVICE_UIDRVERINTERFACEADAPTER_H__
#include <aalsdk/kernel/ccipdriver.h>  // uid_msgIDs_e, uid_errnum_e, aalui_ioctlreq

#include <aalsdk/AALTypes.h>

#include <aalsdk/osal/CriticalSection.h>
#include <aalsdk/CUnCopyable.h>

#include <aalsdk/AALTransactionID.h>

#include "AIATransactions.h"
#include "aalsdk/uaia/IAFUProxy.h"
#include "uidrvMessage.h"

#ifdef __AAL_UNKNOWN_OS__
# error Define UIDriverInterfaceAdapter IPC for unknown OS.
#endif // __AAL_UNKNOWN_OS__

BEGIN_NAMESPACE(AAL)

//==========================================================================
// Name: UIDriverInterfaceAdapter
// Description: The UIDriverInterfaceAdapter is a wrapper object around the
//              low level driver interface.
//==========================================================================
class AIASERVICE_API UIDriverInterfaceAdapter : private CriticalSection,
                                                public  CUnCopyable
{
   public:
      // Constructor
      UIDriverInterfaceAdapter();
      virtual ~UIDriverInterfaceAdapter();

      void   IsOK(AAL::btBool flag) { m_bIsOK = flag; }
      AAL::btBool IsOK()            { return m_bIsOK; }

      // Open/Close the channel to the kernel subsystem
      void Open(const char *devName = NULL);
      void Close();

      AAL::btBool MapWSID(AAL::btWSSize Size, AAL::btWSID wsid, AAL::btVirtAddr *pRet);
      void UnMapWSID(AAL::btVirtAddr ptr, AAL::btWSSize Size);

      // Polls for messages and returns when one is available
      AAL::btBool GetMessage(uidrvMessage *uidrvMessagep);

      // Sends a message down the UIDriver channel
      AAL::btBool SendMessage( AAL::btHANDLE devHandle,
                               IAIATransaction *pMessage,
                               IAFUProxyClient *pProxyClient);




   private:
      #if defined( __AAL_WINDOWS__ )
      HANDLE m_hClient;
      #elif defined( __AAL_LINUX__ )
      AAL::btInt  m_fdClient;
      #endif // OS

      AAL::btBool m_bIsOK;

}; // class UIDriverInterfaceAdapter{}

END_NAMESPACE(AAL)

#endif // __AALSDK_AIASERVICE_UIDRVERINTERFACEADAPTER_H__

