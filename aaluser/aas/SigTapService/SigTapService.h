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
/// @file SigTapService.h
/// @brief Definitions for CCIP SignalTap Service.
/// @ingroup SWSimALIAFU
/// @verbatim
/// Accelerator Abstraction Layer Sample Application
///
///    This application is for example purposes only.
///    It is not intended to represent a model for developing commercially-deployable applications.
///    It is designed to show working examples of the AAL programming model and APIs.
///
/// AUTHORS: Joseph Grecco, Intel Corporation
///          
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 11/19/2015     JG       Initial version.@endverbatim
//****************************************************************************
#ifndef __SIGTAPSERVICE_H__
#define __SIGTAPSERVICE_H__
#include <aalsdk/service/ALIAFUService.h>
#include <aalsdk/service/IALIAFU.h>
#include <aalsdk/aas/AALService.h>


/// @addtogroup SWSimALIAFU
/// @{

#if defined ( __AAL_WINDOWS__ )
# pragma warning(push)           // ignoring this because IALIAFU is purely abstract.
# pragma warning(disable : 4275) // non dll-interface class 'AAL::IALIAFU' used as base for dll-interface class 'AAL::SWSimALIAFU'
#endif // __AAL_WINDOWS__

/// @brief This is the Delegate of the Strategy pattern used by ALIAFU to interact with a
/// Software Simulation of ALI (Native Loopback).
///
/// SWSimALIAFU is selected by passing the Named Value pair (ALIAFU_NVS_KEY_TARGET, ALIAFU_NVS_VAL_TARGET_SWSIM)
/// in the arguments to IRuntime::allocService when requesting a ALIAFU.
class SigTapService : public AAL::ServiceBase,
                      public AAL::IServiceClient,     // for HWALIAFU
                      public AAL::IALISignalTap
{
#if defined ( __AAL_WINDOWS__ )
# pragma warning(pop)
#endif // __AAL_WINDOWS__
public:
   // <ServiceBase>
   DECLARE_AAL_SERVICE_CONSTRUCTOR(SigTapService, AAL::ServiceBase),
   m_ALIMMIO(NULL),
   m_mmio(NULL),
   m_pAALService(NULL)
   {
      SetInterface(iidServiceClient, dynamic_cast<AAL::IServiceClient *>(this));
      SetInterface(iidALI_STAP_Service, dynamic_cast<AAL::IALISignalTap *>(this));
   }

   virtual AAL::btBool init( AAL::IBase               *pclientBase,
                             AAL::NamedValueSet const &optArgs,
                             AAL::TransactionID const &rtid);

   virtual AAL::btBool Release(AAL::TransactionID const &TranID, AAL::btTime timeout=AAL_INFINITE_WAIT);
   // </ServiceBase>

   // <ALISignalTap>
   virtual AAL::btVirtAddr stpGetAddress( void );
   // </ALISignalTap>

   // <IServiceClient>
   virtual void serviceAllocated(AAL::IBase               *pServiceBase,
                                 AAL::TransactionID const &rTranID = AAL::TransactionID());
   virtual void serviceAllocateFailed(const AAL::IEvent &rEvent);

   virtual void serviceReleased(AAL::TransactionID const &rTranID = AAL::TransactionID());

   virtual void serviceReleaseRequest(IBase *pServiceBase, const AAL::IEvent &rEvent){};  // Ignored TODO better implementation

   virtual void serviceReleaseFailed(const AAL::IEvent &rEvent);

   virtual void serviceEvent(const AAL::IEvent &rEvent);

   // <IServiceClient>


protected:
   AAL::TransactionID          m_TranID;
   AAL::IALIMMIO              *m_ALIMMIO;
   AAL::btVirtAddr             m_mmio;
   AAL::IAALService           *m_pAALService;
   AAL::btTime                 m_timeout;
};
/// @}


#endif // __SIGTAPSERVICE_H__

