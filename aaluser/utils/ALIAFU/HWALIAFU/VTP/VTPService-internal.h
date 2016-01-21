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
/// @file VTPService-internal.h
/// @brief Definitions for VTP Service.
/// @ingroup vtp_service
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
/// Virtual-to-Physical address translation service
///
/// TODO: add verbose description
///
/// Provides service for access to the VTP BBB for address translation.
/// Assumes a VTP BBB DFH to be detected and present.
///
/// On initialization, allocates shared buffer for VTP page hash and
/// communicates its location to VTP BBB.
///
/// Provides synchronous methods to update page hash on shared buffer
/// allocation.
///
/// Does not have an explicit client callback interface, as all published
/// service methods are synchronous.
///
/// AUTHORS: Enno Luebbers, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 01/15/2016     EL       Initial version@endverbatim
//****************************************************************************
#ifndef __VTPSERVICE_INT_H__
#define __VTPSERVICE_INT_H__
#include <aalsdk/service/VTPService.h> // Public VTP service interface
#include <aalsdk/aas/AALService.h>
#include <aalsdk/IServiceClient.h>
#include <aalsdk/osal/IDispatchable.h>

#include <aalsdk/service/IALIAFU.h>
#include <aalsdk/uaia/IAFUProxy.h>

using namespace AAL;

/// @addtogroup vtp_service
/// @{

//=============================================================================
// Name: VTPService
// Description: Virtual-To-Physical address translation service
// Interface: IVTPService
// Comments:
//=============================================================================
/// @brief Virtual-To-Physical address translation service.
class VTPService : public ServiceBase, public IVTPService
{
public:

   /// VTP service constructor
   DECLARE_AAL_SERVICE_CONSTRUCTOR(VTPService, ServiceBase),
      m_pSvcClient(NULL),
      m_pHWALIAFU(NULL),
      m_pDFHBaseAddr(NULL)
   {
      SetInterface(iidVTPService, dynamic_cast<IVTPService *>(this));
   }
   /// @brief Service initialization hook.
   ///
   /// Expects the following in the optArgs passed to it:
   ///
   ///   HWALIAFU_IBASE       pointer to HWALIAFU
   ///   VTP_DFH_BASE         pointer to MMIO space where VTP DFH resides
   btBool init( IBase *pclientBase,
                NamedValueSet const &optArgs,
                TransactionID const &rtid);

   /// Called when the service is released
   btBool Release(TransactionID const &rTranID, btTime timeout=AAL_INFINITE_WAIT);

// <IVTPService>
   ali_errnum_e bufferAllocate( btWSSize       Length,
                                btVirtAddr    *pBufferptr,
                                NamedValueSet *pOptArgs);
   ali_errnum_e bufferFree(     btVirtAddr     Address);
   ali_errnum_e bufferFreeAll();
   btPhysAddr   bufferGetIOVA(  btVirtAddr     Address);
// </IVTPService>


protected:
   IServiceClient        *m_pSvcClient;
   IBase                 *m_pHWALIAFU;
   btVirtAddr             m_pDFHBaseAddr;
};

/// @}

#endif //__SAMPLEAFU1SERVICE_INT_H__

