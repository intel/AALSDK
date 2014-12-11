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
/// @file uAIADefs.h
/// @brief Definitions for the  Universal Application Interface Adaptor.
/// @ingroup uAIA
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHOR: Joseph Grecco, Intel Corporation.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 11/28/2008     HM       Fixed legal header, spelling update
/// 01/04/2009     HM       Updated Copyright@endverbatim
//****************************************************************************
#ifndef __AALSDK_UAIA_UAIADEFS_H__
#define __AALSDK_UAIA_UAIADEFS_H__
#include <aalsdk/INTCDefs.h>
#include <aalsdk/osal/OSServiceModule.h>
#include <aalsdk/AALTransactionID.h>
#include <aalsdk/AALEvent.h>


BEGIN_NAMESPACE(AAL)
   BEGIN_NAMESPACE(AAS)
      BEGIN_NAMESPACE(AIA)


/******************************************************************************
* Define Vendor-INTC macros for this system
******************************************************************************/

// Device ID
#define  AHMV1_D1B1M1L1    (0x00010109)

// Device Mask                Dom B DL
#define  AHMV1_DeviceMask  (0xFFFEFEF6)

/******************************************************************************
*                      Extended Published Interfaces
******************************************************************************/

//=============================================================================
//
// Message structures and defines
//
//=============================================================================
typedef  enum uAIAcmd{
   aiaCmd_ResetLogicalBud =1,
   aiaCmd_ResetAll,
   aiaCmd_AFUCmd,
   aiaCMd_CancelTransaction
}uAIAcmd;

// AIA Command based descriptor structure
struct aia_msg {
  uAIAcmd  Command;
  union {
    // aiaCmd_AFUcmd
    struct {
      void *   Srcaddr;       // Source workspace address
      size_t   sSize;         // Source length
      void *   Dstaddr;       // Destination workspace address
      size_t   dSize;         // Destination length
      void*    cookie;        // AIA defined cookie
    } AFUCmd;
   }u;
   TransactionID   tid;       // Used for cancel transaction
};


/******************************************************************************
* Define Vendor-INTC specific macros for this system
******************************************************************************/
#define __FSBV1AIA_TranEvt(Num)    __INTC_TranEvt(INTC_sysFSBV1AIA, (Num))
#define __FSBV1AIA_ExTranEvt(Num)  __INTC_ExTranEvt(INTC_sysFSBV1AIA, (Num))



//=============================================================================
// Name: tranevtFSBV1AIASendMessage
// Description: AIA exception Event
// Implements: IExceptionTransactEvent
//=============================================================================
#define tranevtFSBV1AIASendMessage             __FSBV1AIA_TranEvt(0x0001)


//=============================================================================
// Name: extranevtFSBV1AIAFactory
// Description: AIA exception Event
// Implements: IExceptionTransactEvent
//=============================================================================
#define extranevtFSBV1AIA                      __FSBV1AIA_ExTranEvt(0x0001)


      END_NAMESPACE(AIA)
   END_NAMESPACE(AAS)
END_NAMESPACE(AAL)


#endif // __AALSDK_UAIA_UAIADEFS_H__

