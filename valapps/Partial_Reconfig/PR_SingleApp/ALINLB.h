// INTEL CONFIDENTIAL - For Intel Internal Use Only
//
// valapps/Partial_Reconfig/PR_SingleApp/ALINLB.h
//
// Copyright(c) 2007-2016, Intel Corporation
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
/// @file ALINLB.h
/// @brief NLB Service and run NLB algorithm on fpga  .
/// @ingroup Partial_Reconfig
/// @verbatim
/// AAL NLB test application
///
///    This application is for testing purposes only.
///    It is not intended to represent a model for developing commercially-
///       deployable applications.
///    It is designed to test NLB functionality.
///
///
/// This Sample demonstrates how to use the basic ALI APIs.
///
/// This sample is designed to be used with the xyzALIAFU Service.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 06/06/2016     RP       Initial version started based on older sample
//****************************************************************************


#ifndef __ALI_NLB__
#define __ALI_NLB__

#include <aalsdk/AALTypes.h>
#include <aalsdk/Runtime.h>
#include <aalsdk/AALLoggerExtern.h>

#include <aalsdk/service/IALIAFU.h>

#include <string.h>
#include "arguments.h"

using namespace std;
using namespace AAL;


class AllocatesNLBService: public CAASBase, public IServiceClient
{
public:

   AllocatesNLBService(const arguments &args);
   ~AllocatesNLBService();

   // <begin IServiceClient interface>
   void serviceAllocated(IBase *pServiceBase,TransactionID const &rTranID);
   void serviceAllocateFailed(const IEvent &rEvent);
   void serviceReleased(const AAL::TransactionID&);
   void serviceReleaseRequest(IBase *pServiceBase, const IEvent &rEvent);
   void serviceReleaseFailed(const AAL::IEvent&);
   void serviceEvent(const IEvent &rEvent);
   // <end IServiceClient interface>

   btBool FreeNLBService();
   btBool AllocateNLBService(Runtime *pRuntime);
   btInt  runInLoop();
   btInt  run();
   btID getErrnum() {  return m_errNum; }
   void setReleaseService(btBool releaseService)  { m_ReleaseService= releaseService; }
   static void NLBThread(OSLThread *pThread, void *pContext);

protected:
   Runtime       *m_pRuntime;           ///< AAL Runtime
   IBase         *m_pNLBService;       ///< The generic AAL Service interface for the AFU.
   IALIBuffer    *m_pALIBufferService; ///< Pointer to Buffer Service
   IALIMMIO      *m_pALIMMIOService;   ///< Pointer to MMIO Service
   IALIReset     *m_pALIResetService;  ///< Pointer to AFU Reset Service
   CSemaphore     m_Sem;               ///< For synchronizing with the AAL runtime.
   btInt          m_Result;            ///< Returned result value; 0 if success

   // Workspace info
   btVirtAddr     m_DSMVirt;        ///< DSM workspace virtual address.
   btPhysAddr     m_DSMPhys;        ///< DSM workspace physical address.
   btWSSize       m_DSMSize;        ///< DSM workspace size in bytes.
   btVirtAddr     m_InputVirt;      ///< Input workspace virtual address.
   btPhysAddr     m_InputPhys;      ///< Input workspace physical address.
   btWSSize       m_InputSize;      ///< Input workspace size in bytes.
   btVirtAddr     m_OutputVirt;     ///< Output workspace virtual address.
   btPhysAddr     m_OutputPhys;     ///< Output workspace physical address.
   btWSSize       m_OutputSize;     ///< Output workspace size in bytes.
   btID           m_errNum;
   btBool         m_ReleaseService;

private:
   arguments      m_args;
};

#endif
