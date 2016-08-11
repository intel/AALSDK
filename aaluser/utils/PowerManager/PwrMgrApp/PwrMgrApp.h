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
/// @file PwrMgrApp.h
/// @brief Basic PwrMgr interaction.
/// @ingroup PwrMgerApp
/// @verbatim
/// Accelerator Abstraction Layer Sample Application
///
///    This application is for example purposes only.
///    It is not intended to represent a model for developing commercially-deployable applications.
///    It is designed to show working examples of the AAL programming model and APIs.
///
/// AUTHORS: Ananda Ravuri, Intel Corporation.
///
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 07/25/2016     AR       Initial version started based on older sample code.@endverbatim
//****************************************************************************
/// @}


#ifndef __PWRMGER_APP_H__
#define __PWRMGER_APP_H__
#include <aalsdk/AAL.h>
#include <aalsdk/Runtime.h>
#include <aalsdk/service/IALIAFU.h>
#include <aalsdk/service/IPwrMgr.h>

using namespace std;
using namespace AAL;



#ifdef MSG
# undef MSG
#endif // MSG
#define MSG(x) std::cout << __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() : " << x << std::endl
#ifdef ERR
# undef ERR
#endif // ERR
#define ERR(x) std::cerr << __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() **Error : " << x << std::endl


class PwrMgrRuntime : public CAASBase,
                      public IRuntimeClient
{
public:
   PwrMgrRuntime();
   btInt Errors() const { return m_Errors; }

   // <IRuntimeClient>
   void   runtimeCreateOrGetProxyFailed(IEvent const        &rEvent);
   void                  runtimeStarted(IRuntime            *pRuntime,
                                        const NamedValueSet &rConfigParms);
   void                  runtimeStopped(IRuntime            *pRuntime);
   void              runtimeStartFailed(const IEvent        &rEvent);
   void               runtimeStopFailed(const IEvent        &rEvent);
   void    runtimeAllocateServiceFailed(IEvent const        &rEvent);
   void runtimeAllocateServiceSucceeded(IBase               *pClient,
                                        TransactionID const &rTranID);
   void                    runtimeEvent(const IEvent        &rEvent);
   // </IRuntimeClient>
   Runtime* getRuntime() { return &m_Runtime;}
   void runtimeStop();

protected:
   Runtime              m_Runtime;       ///< AAL Runtime
   btInt                m_Errors;        ///< Returned result value; 0 if success
   CSemaphore           m_Sem;           ///< For synchronizing with the AAL runtime.
};


class PwrMgrService : public CAASBase,
                      public IPwrMgr_Client,
                      public IServiceClient
{
public:
   PwrMgrService(Runtime *pRuntime);

   btInt Errors() const { return m_Errors; }
   btInt AllocateService();
   btInt FreeService();


   // <IServiceClient>
   void      serviceAllocated(AAL::IBase               *pServiceBase,
                              AAL::TransactionID const &rTranID);
   void serviceAllocateFailed(const AAL::IEvent        &rEvent);
   void       serviceReleased(const AAL::TransactionID &rTranID);
   void serviceReleaseRequest(AAL::IBase               *pServiceBase,
                              const AAL::IEvent        &rEvent);
   void  serviceReleaseFailed(const AAL::IEvent        &rEvent);
   void          serviceEvent(const AAL::IEvent        &rEvent);
   // </IServiceClient>

   // <IPwrMgr_Client>
   void reconfPowerRequest( TransactionID &rTranID,IEvent const &rEvent ,INamedValueSet &rInputArgs);
   // </IPwrMgr_Client>

   btInt CoreIdler(btInt &FPIWatts, btInt &socket);


protected:
   Runtime             *m_pRuntime;        /// AAL Runtime
   btInt                m_Errors;          ///< Returned result value; 0 if success
   CSemaphore           m_Sem;             ///< For synchronizing with the AAL runtime.
   IBase               *m_pPwrMgrService;  /// Power manger Service pointer
};

#endif // __PWRMGER_APP_H__
