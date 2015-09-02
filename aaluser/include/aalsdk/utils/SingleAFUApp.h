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
/// @file SingleAFUApp.h
/// @brief Template for simplifying XL app creation.
/// @ingroup SingleAFUApp
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer Sample Application
///
///    This application is for example purposes only.
///    It is not intended to represent a model for developing commercially-deployable applications.
///    It is designed to show working examples of the AAL programming model and APIs.
///
/// AUTHORS: Tim Whisonant, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 6/06/2014      TSW      Initial version.@endverbatim
//****************************************************************************
#ifndef __AALSDK_UTILS_SINGLEAFUAPP_H__
#define __AALSDK_UTILS_SINGLEAFUAPP_H__

#include <aalsdk/utils/Utilities.h>    // Brings in CL, MB, GB, etc.
                                       // Here for backward compatibility.
                                       // Better to move to where SingleAFUApp.h is #included itself
#include <aalsdk/AAL.h>
#include <aalsdk/Runtime.h>

USING_NAMESPACE(AAL)

/// @addtogroup SingleAFUApp
/// @{

/// @brief Template class for expediting and simplifying creation of XL applications that interact with
/// a single AFU instance.
///
/// See @ref cciapp and @ref splapp for example implementations.
template <typename Proprietary>
class ISingleAFUApp : public IRuntimeClient,
                      public IServiceClient,
                      public CAASBase
{
public:
   ISingleAFUApp();
   virtual ~ISingleAFUApp();

   /// @brief "Stops" the application by releasing any allocated AFU and waiting for
   ///        the release notification.
   ///
   /// ISingleAFUApp's internal semaphore is posted before the call returns.
   void Stop();

   // <IRuntimeClient>
   virtual void runtimeCreateOrGetProxyFailed(IEvent const &rEvent);
   virtual void     runtimeStarted(IRuntime *,
                                   const NamedValueSet &);
   virtual void     runtimeStopped(IRuntime *);
   virtual void     runtimeStopFailed(const IEvent &rEvent);
   virtual void     runtimeStartFailed(const IEvent &);
   virtual void     runtimeAllocateServiceFailed(IEvent const &);
   virtual void     runtimeAllocateServiceSucceeded(IBase *,
                                                    TransactionID const & );
   virtual void     runtimeEvent(const IEvent & );
   // </IRuntimeClient>

   // <IServiceClient>
   virtual void      serviceAllocated(IBase *,
                                      TransactionID const & = TransactionID());
   virtual void		 serviceAllocateFailed(const IEvent &);
   virtual void      serviceReleased(TransactionID const & = TransactionID());
   virtual void      serviceReleaseFailed(const IEvent &);
   virtual void      serviceEvent(const IEvent &);
   // </IServiceClient>

   /// @brief Called in response to IRuntimeClient::runtimeStarted notification.
   /// @note Subclasses must override to implement application-specific behavior.
   ///
   /// A pointer to the IRuntime is stored in m_pRuntime prior to this call.
   virtual void OnRuntimeStarted(IRuntime *,
                                 const NamedValueSet &)        = 0;
   /// @brief Called in response to IRuntimeClient::runtimeStopped notification.
   /// @note Subclasses must override to implement application-specific behavior.
   ///
   /// ISingleAFUApp's internal semaphore is posted after this call.
   virtual void OnRuntimeStopped(IRuntime *)           = 0;
   /// @brief Called in response to IRuntimeClient::runtimeStartFailed notification.
   /// @note Subclasses must override to implement application-specific behavior.
   ///
   /// ISingleAFUApp's internal semaphore is posted after this call.
   /// m_bIsOK (inherited from CAASBase) is set to false after this call.
   virtual void OnRuntimeStartFailed(const IEvent &)           = 0;
   /// @brief Called in response to IRuntimeClient::runtimeAllocateServiceFailed notification.
   /// @note Subclasses must override to implement application-specific behavior.
   ///
   /// m_bIsOK (inherited from CAASBase) is set to false after this call.
   virtual void OnRuntimeAllocateServiceFailed(IEvent const & )     = 0;
   /// @brief Called in response to IRuntimeClient::runtimeAllocateServiceSucceeded notification.
   /// @note Subclasses must override to implement application-specific behavior.
   virtual void OnRuntimeAllocateServiceSucceeded(IBase * ,
                                                  TransactionID const & ) = 0;
   /// @brief Called in response to IRuntimeClient::runtimeEvent notification.
   /// @note Subclasses must override to implement application-specific behavior.
   virtual void OnRuntimeEvent(const IEvent &)                 = 0;

   /// @brief Called in response to IServiceClient::serviceAllocated notification.
   /// @note Subclasses must override to implement application-specific behavior.
   ///
   /// Pointers to the base IAALService interface and the proprietary service interface
   /// are cached in m_pAALService and m_pProprietary, respectively.
   ///
   /// ISingleAFUApp's internal semaphore is posted after this call.
   virtual void OnServiceAllocated(IBase *,
                                   TransactionID const &)      = 0;
   /// @brief Called in response to IServiceClient::serviceAllocateFailed notification.
   /// @note Subclasses must override to implement application-specific behavior.
   ///
   /// ISingleAFUApp's internal semaphore is posted after this call.
   /// m_bIsOK (inherited from CAASBase) is set to false after this call.
   virtual void OnServiceAllocateFailed(const IEvent &)        = 0;
   /// @brief Called in response to IServiceClient::serviceFreed notification.
   /// @note Subclasses must override to implement application-specific behavior.
   ///
   /// ISingleAFUApp's internal semaphore is posted after this call.
   virtual void OnServiceReleased(TransactionID const &)          = 0;
   /// @brief Called in response to IServiceClient::serviceReleaseFailed notification.
   /// @note Subclasses must override to implement application-specific behavior.
   ///
   /// ISingleAFUApp's internal semaphore is posted after this call.
   /// m_bIsOK (inherited from CAASBase) is set to false after this call.
   virtual void OnServiceReleaseFailed(const IEvent &)        = 0;
   /// @brief Called in response to IServiceClient::serviceEvent notification.
   /// @note Subclasses must override to implement application-specific behavior.
   virtual void OnServiceEvent(const IEvent &)                 = 0;

   /// @brief Retrieve a pointer to the Proprietary interface of the allocated AFU.
   ///
   /// The returned pointer is only guaranteed valid after the runtime has started and an
   /// AFU has been successfully allocated.
   operator Proprietary * () { return m_pProprietary; }
   /// @brief Retrieve a pointer to the base (IAALService) interface of the allocated AFU.
   ///
   /// The returned pointer is only guaranteed valid after the runtime has started and an
   /// AFU has been successfully allocated.
   operator IAALService * () { return m_pAALService;  }

   /// @brief Wait for ISingleAFUApp's semaphore to become signaled.
   void Wait();
protected:
   /// @brief Signal (one count) ISingleAFUApp's semaphore.
   void Post();

   IRuntime    *m_pRuntime;
   IAALService *m_pAALService;
   Proprietary *m_pProprietary;
   CSemaphore   m_Sem;
};

template <typename Proprietary>
ISingleAFUApp<Proprietary>::ISingleAFUApp() :
   m_pRuntime(NULL),
   m_pAALService(NULL),
   m_pProprietary(NULL)
{
   m_Sem.Create(0, 1);
   SetSubClassInterface(iidRuntimeClient, dynamic_cast<IRuntimeClient *>(this));
   SetInterface(iidServiceClient, dynamic_cast<IServiceClient *>(this));
}

template <typename Proprietary>
ISingleAFUApp<Proprietary>::~ISingleAFUApp()
{
   Stop();
   m_Sem.Destroy();
}

template <typename Proprietary>
void ISingleAFUApp<Proprietary>::Stop()
{
   if ( NULL != m_pAALService ) {
      m_pAALService->Release(TransactionID(), 0);
      Wait(); // For service freed notification.
      m_pAALService = NULL;
   }

   if ( NULL != m_pRuntime ) {
      m_pRuntime->stop();
      Wait(); // For runtime stopped notification.
      m_pRuntime = NULL;
   }

   Post(); // Wake up main, if waiting
}

template <typename Proprietary>
void ISingleAFUApp<Proprietary>::runtimeCreateOrGetProxyFailed(IEvent const &e)
{
   if ( AAL_IS_EXCEPTION(e.SubClassID()) ) {
      PrintExceptionDescription(e);
   }
   OnRuntimeEvent(e);
   m_bIsOK = false;
   Post();
}

template <typename Proprietary>
void ISingleAFUApp<Proprietary>::runtimeStopFailed(IEvent const &e)
{
   if ( AAL_IS_EXCEPTION(e.SubClassID()) ) {
      PrintExceptionDescription(e);
   }
   OnRuntimeEvent(e);
   m_bIsOK = false;
   Post();
}

template <typename Proprietary>
void ISingleAFUApp<Proprietary>::runtimeStarted(IRuntime            *pRT,
                                                const NamedValueSet &Args)
{
   ASSERT(pRT->IsOK());
   if ( !pRT->IsOK() ) {
      return;
   }

   m_pRuntime = pRT;

   OnRuntimeStarted(m_pRuntime, Args);
}

template <typename Proprietary>
void ISingleAFUApp<Proprietary>::runtimeStopped(IRuntime *pRT)
{
   OnRuntimeStopped(pRT);
   Post();
}

template <typename Proprietary>
void ISingleAFUApp<Proprietary>::runtimeStartFailed(const IEvent &e)
{
   if ( AAL_IS_EXCEPTION(e.SubClassID()) ) {
      PrintExceptionDescription(e);
   }
   OnRuntimeStartFailed(e);
   m_bIsOK = false;
   Post();
}

template <typename Proprietary>
void ISingleAFUApp<Proprietary>::runtimeAllocateServiceFailed(IEvent const &e)
{
   if ( AAL_IS_EXCEPTION(e.SubClassID()) ) {
      PrintExceptionDescription(e);
   }
   OnRuntimeAllocateServiceFailed(e);
   m_bIsOK = false;
}

template <typename Proprietary>
void ISingleAFUApp<Proprietary>::runtimeAllocateServiceSucceeded(IBase               *pServiceBase,
                                                                 TransactionID const &tid)
{
   m_pAALService = dynamic_ptr<IAALService>(iidService, pServiceBase);
   ASSERT(NULL != m_pAALService);

   m_pProprietary = subclass_ptr<Proprietary>(pServiceBase);
   ASSERT(NULL != m_pProprietary);

   OnRuntimeAllocateServiceSucceeded(pServiceBase, tid);
}

template <typename Proprietary>
void ISingleAFUApp<Proprietary>::runtimeEvent(const IEvent &e)
{
   if ( AAL_IS_EXCEPTION(e.SubClassID()) ) {
      PrintExceptionDescription(e);
   }
   OnRuntimeEvent(e);
}

template <typename Proprietary>
void ISingleAFUApp<Proprietary>::serviceAllocated(IBase               *pServiceBase,
                                                  TransactionID const &tid)
{
   m_pAALService = dynamic_ptr<IAALService>(iidService, pServiceBase);
   ASSERT(NULL != m_pAALService);

   m_pProprietary = subclass_ptr<Proprietary>(pServiceBase);
   ASSERT(NULL != m_pProprietary);

   OnServiceAllocated(pServiceBase, tid);
   Post();
}

template <typename Proprietary>
void ISingleAFUApp<Proprietary>::serviceAllocateFailed(const IEvent &e)
{
   if ( AAL_IS_EXCEPTION(e.SubClassID()) ) {
      PrintExceptionDescription(e);
   }
   OnServiceAllocateFailed(e);
   m_bIsOK = false;
   Post();
}

template <typename Proprietary>
void ISingleAFUApp<Proprietary>::serviceReleased(TransactionID const &tid)
{
   OnServiceReleased(tid);
   Post();
}

template <typename Proprietary>
void ISingleAFUApp<Proprietary>::serviceReleaseFailed(IEvent const &e)
{
   if ( AAL_IS_EXCEPTION(e.SubClassID()) ) {
      PrintExceptionDescription(e);
   }
   OnServiceReleaseFailed(e);
   m_bIsOK = false;
}

template <typename Proprietary>
void ISingleAFUApp<Proprietary>::serviceEvent(const IEvent &e)
{
   if ( AAL_IS_EXCEPTION(e.SubClassID()) ) {
      PrintExceptionDescription(e);
   }
   OnServiceEvent(e);
}

template <typename Proprietary>
void ISingleAFUApp<Proprietary>::Wait() { m_Sem.Wait();  }

template <typename Proprietary>
void ISingleAFUApp<Proprietary>::Post() { m_Sem.Post(1); }

/// @}

#endif // __AALSDK_UTILS_SINGLEAFUAPP_H__

