// Copyright (c) 2014-2015, Intel Corporation
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
/// @file SPLAFU.h
/// @brief Definitions for SPL AFU Service.
/// @ingroup SPLAFU
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer Sample Application
///
///    This application is for example purposes only.
///    It is not intended to represent a model for developing commercially-deployable applications.
///    It is designed to show working examples of the AAL programming model and APIs.
///
/// AUTHORS: Tim Whisonant, Intel Corporation
///          Joseph Grecco, Intel Corporation
///          
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 07/18/2014     TSW      Initial version.@endverbatim
//****************************************************************************
#ifndef __SPLAFU_H__
#define __SPLAFU_H__
#include <aalsdk/aas/AALService.h>
#include <aalsdk/service/ISPLAFU.h>
#include <aalsdk/service/ISPLClient.h>
#include <aalsdk/service/SPLAFUService.h>

BEGIN_NAMESPACE(AAL)

/// @addtogroup SPLAFU
/// @{

#if defined ( __AAL_WINDOWS__ )
# pragma warning(push)           // ignoring this because ISPLAFU and ISPLClient are purely abstract.
# pragma warning(disable : 4275) // non dll-interface class 'AAL::ISPLAFU' used as base for dll-interface class 'AAL::SPLAFU'
#endif // __AAL_WINDOWS__

/// SPLAFU is the outer shell of the Strategy pattern. It delegates the responsibilities
/// of its ISPLAFU interface to either a HWSPLAFU, an ASESPLAFU, or a SWSimSPLAFU which it
/// allocates during its initialization. This nested allocation of AFU's is why SPLAFU
/// must implement IServiceClient.
///
/// SPLAFU also implements ISPLClient in order to relay the event notifications from the
/// HWSPLAFU, ASESPLAFU, or SWSimSPLAFU back to the outer client of the SPLAFU.
///
/// The underlying AFU implementation is chosen by the outer client's call to IRuntime::allocService.
/// The client populates the Named Value Set key SPLAFU_NVS_KEY_TARGET with one of..
/// <ul>
///   <li>SPLAFU_NVS_VAL_TARGET_FPGA</li>
///   <li>SPLAFU_NVS_VAL_TARGET_ASE</li>
///   <li>SPLAFU_NVS_VAL_TARGET_SWSIM</li>
/// </ul>
///
/// See @ref splapp for more details.
class SPLAFU_API SPLAFU : public ServiceBase,
                          public ISPLAFU,
                          public IServiceClient,
                          public ISPLClient
{
#if defined ( __AAL_WINDOWS__ )
# pragma warning(pop)
#endif // __AAL_WINDOWS__
public:
   // <ServiceBase>
   DECLARE_AAL_SERVICE_CONSTRUCTOR(SPLAFU, ServiceBase),
      m_pDelegate(NULL),
      m_TargetAFU(NULL)
   {
      SetInterface(iidSPLAFU, dynamic_cast<ISPLAFU *>(this));
      SetInterface(iidCCIClient,      dynamic_cast<ICCIClient *>(this));
      SetInterface(iidSPLClient,      dynamic_cast<ISPLClient *>(this));
      SetInterface(iidServiceClient,  dynamic_cast<IServiceClient *>(this));
   }

   ~SPLAFU()
   {
   };

   virtual btBool init( IBase *pclientBase,
                        NamedValueSet const &optArgs,
                        TransactionID const &rtid);

   virtual btBool Release(TransactionID const &TranID, btTime timeout=AAL_INFINITE_WAIT);
   virtual btBool Release(btTime timeout=AAL_INFINITE_WAIT);
   // </ServiceBase>


   // <IServiceClient>
   virtual void      serviceAllocated(IBase               *pServiceBase,
                                      TransactionID const &TranID);

   virtual void serviceAllocateFailed(const IEvent        &Event);

   virtual void          serviceReleased(TransactionID const &TranID);

   virtual void serviceReleaseFailed(const IEvent        &Event);

   virtual void          serviceEvent(const IEvent        &Event);
   // </IServiceClient>


   // <ISPLAFU>
   virtual void WorkspaceAllocate(btWSSize             Length,
                                  TransactionID const &TranID);

   virtual void     WorkspaceFree(btVirtAddr           Address,
                                  TransactionID const &TranID);

   virtual btBool         CSRRead(btCSROffset CSR,
                                  btCSRValue *pValue);

   virtual btBool        CSRWrite(btCSROffset CSR,
                                  btCSRValue  Value);
   virtual btBool      CSRWrite64(btCSROffset CSR,
                                  bt64bitCSR  Value);

   virtual void StartTransactionContext(TransactionID const &TranID,
                                        btVirtAddr           Address=NULL,
                                        btTime               Pollrate=0);

   virtual void StopTransactionContext(TransactionID const &TranID);

   virtual void SetContextWorkspace(TransactionID const &TranID,
                                    btVirtAddr           Address,
                                    btTime               Pollrate=0);
   // </ISPLAFU>


   // <ISPLClient>
   virtual void      OnWorkspaceAllocated(TransactionID const &TranID,
                                          btVirtAddr           WkspcVirt,
                                          btPhysAddr           WkspcPhys,
                                          btWSSize             WkspcSize);

   virtual void OnWorkspaceAllocateFailed(const IEvent &Event);

   virtual void          OnWorkspaceFreed(TransactionID const &TranID);

   virtual void     OnWorkspaceFreeFailed(const IEvent &Event);

   virtual void      OnTransactionStarted(TransactionID const &TranID,
                                          btVirtAddr           AFUDSM,
                                          btWSSize             AFUDSMSize);

   virtual void     OnContextWorkspaceSet(TransactionID const &TranID);

   virtual void       OnTransactionFailed(const IEvent &Event);

   virtual void     OnTransactionComplete(TransactionID const &TranID);

   virtual void     OnTransactionStopped(TransactionID const &TranID);
   // </ISPLClient>

protected:
   ISPLAFU       *m_pDelegate;          ///< Pointer to allocated Delegate AFU instance.
   btcString      m_TargetAFU;          ///< Saved value of SPLAFU_NVS_KEY_TARGET. Helps us extract the appropriate Delegate interface.
   TransactionID  m_TranIDFrominit;     ///< Saved from the call to ServiceBase::init, until the Delegate AFU is allocated.
   TransactionID  m_TranIDFromRelease;  ///< Saved from the call to ServiceBase::Release, until the Delegate AFU is freed.
   btTime         m_TimeoutFromRelease; ///< Saved from the call to ServiceBase::Release, until the Delegate AFU is freed.
};

/// @}

END_NAMESPACE(AAL)

#endif // __SPLAFU_H__

