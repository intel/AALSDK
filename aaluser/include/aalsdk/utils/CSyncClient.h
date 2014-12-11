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
/// @file CSyncClient.h
/// @brief Useful class for creating synchronized main routines.
/// @ingroup CSyncClient
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer Sample Application
///
///    This application is for example purposes only.
///    It is not intended to represent a model for developing commercially-deployable applications.
///    It is designed to show working examples of the AAL programming model and APIs.
///
/// AUTHORS: Henry Mitchel, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 10/08/2014     HM       Initial version.
/// 10/22/2014     HM       Added better support for runtime and service errors@endverbatim
//****************************************************************************
#ifndef __AALSDK_UTILS_CSYNCCLIENT_H__
#define __AALSDK_UTILS_CSYNCCLIENT_H__

#include <aalsdk/AAL.h>
using namespace AAL;
using namespace AAS;

#include <aalsdk/xlRuntime.h>
using namespace XL;
using namespace RT;


// Convenience macros for printing messages and errors.
#ifndef MSG
# define MSG(x) std::cout << __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() : " << x << std::endl
#endif // MSG
#ifndef ERR
# define ERR(x) std::cerr << __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() **Error : " << x << std::endl
#endif // ERR

/// @addtogroup CSyncClient
/// @{

/// @class CSyncClient
///
/// Define a combined IRuntimeClient and IServiceClient so that we can receive
///   all of the relevant notifications and synchronize them.
///
/// We also implement a Semaphore for synchronization with the AAL runtime.
/// Derive from this class to use synchronized interfaces for IRuntimeClient and
///    IServiceClient, and then add your own synchronization primitives for your
///    specific I<specificService>Client.

class CSyncClient: public CAASBase,
                   public AAL::XL::RT::IRuntimeClient,
                   public AAL::AAS::IServiceClient
{
public:
   /// Default Constructor just gets memory initialized and registers interfaces
   CSyncClient() :
         m_bIsOK(false),
         m_runTime(),
         m_bRunningStatus(false),
         m_pCurrentService(NULL),
         m_pAALService(NULL)
   { // Publish our interface
      SetInterface(iidRuntimeClient, dynamic_cast<AAL::XL::RT::IRuntimeClient*>(this));
      SetInterface(iidServiceClient, dynamic_cast<AAL::AAS::IServiceClient*>(this));
      m_Sem.Create(0, INT_MAX);
      m_bIsOK = true;
   }
   /// Default Destructor
   ~CSyncClient()
   {
      m_Sem.Destroy();
   }
   /// Check internal state of CSyncClient object.
   ///
   /// Since derived class can also set m_bIsOK directly, it provides access to the
   ///   state of the entire stack of classes instantiated in the object.
   btBool IsOK() const { return m_bIsOK && CAASBase::IsOK(); }
   /// Post on the client's internal semaphore.
   void Post(btInt count=1) { m_Sem.Post(count); }
   /// Wait on the client's internal semaphore.
   void Wait()              { m_Sem.Wait();      }

   ///////////////////////////////////////////////////////////////////////////
   // <begin IRuntimeClient interface>

   /// CSyncClient implementation of IRuntimeClient::runtimeStarted
   void runtimeStarted(IRuntime *pRuntime,
         const NamedValueSet &rConfigParms)
   {
      m_bRunningStatus = pRuntime->IsOK();
      Post();
   }
   /// CSyncClient implementation of IRuntimeClient::runtimeStopped
   void runtimeStopped(IRuntime *pRuntime)
   {
      m_bRunningStatus = false;
      Post();
   }
   /// CSyncClient implementation of IRuntimeClient::runtimeStartFailed
   void runtimeStartFailed(const IEvent &rEvent)
   {
      m_bIsOK = false;
      m_bRunningStatus = false;
      ERR("Runtime start failed");
      PrintExceptionDescription(rEvent);  // Built-in function to print exception events
      Post();
   }
   /// CSyncClient implementation of IRuntimeClient::runtimeAllocateServiceFailed
   void runtimeAllocateServiceFailed(IEvent const &rEvent)
   { /* Ignored */
   }
   /// CSyncClient implementation of IRuntimeClient::runtimeAllocateServiceSucceeded
   void runtimeAllocateServiceSucceeded(AAL::IBase *pClient,
         TransactionID const &rTranID)
   { /* Ignored */
   }
   /// CSyncClient implementation of IRuntimeClient::runtimeEvent
   void runtimeEvent(const IEvent &rEvent)
   {
      ERR("Generic message handler (runtime)");
      ERR("Unexpected event 0x" << std::hex << rEvent.SubClassID());
   }
   // <end IRuntimeClient interface>
   ///////////////////////////////////////////////////////////////////////////

   ///////////////////////////////////////////////////////////////////////////
   // ADDITIONAL __synchronous__ RUNTIME CLIENT FUNCTIONS
   //
   /// First call to the syncClient. Start up the Runtime, and return status.
   /// @param[in] configArgs defines Runtime-specific values for configuring the runtime
   bool syncStart( const NamedValueSet &configArgs)
   {
      // Can return false if no way to call back, if so then die immediately
      m_bRunningStatus = m_runTime.start(this, configArgs);
      if (m_bRunningStatus) {                // Posted and Set in runtimeStarted
         Wait();                             //    or runtimeStartFailed
      }
      return m_bRunningStatus;
   }
   /// Shutdown the RunTime Client, and therefore the RunTime itself
   void syncStop()
   {
      m_runTime.stop();
      Wait();                             // Posted in runtimeStopped
   }
   ///////////////////////////////////////////////////////////////////////////

   ///////////////////////////////////////////////////////////////////////////
   // <begin IServiceClient interface>

   /// CSyncClient implementation of IServiceClient::serviceAllocated
   void serviceAllocated( AAL::IBase *pServiceBase, TransactionID const &rTranID)
   {
      // pServiceBase should be valid, because if the call to allocService() failed
      //    it would come back in serviceAllocateFailed.
      // However, if the Service ended up calling serviceAllocated with a NULL
      //    pointer there is nothing the framework can do. So it is possible
      //    and needs to be handled.
      ASSERT( m_pCurrentService == NULL); m_pCurrentService = pServiceBase;
      ASSERT( m_pAALService == NULL);     m_pAALService = NULL;
      ASSERT( pServiceBase );    // if false, then Service threw a bad pointer
      if ( !pServiceBase ) {
         m_bIsOK = false;
         Post();
         return;
      }
      m_pAALService = dynamic_ptr<IAALService>( iidService, pServiceBase);
      ASSERT( m_pAALService );
      if ( !pServiceBase ) {
         m_bIsOK = false;
      }
      Post();
   }
   /// CSyncClient implementation of IServiceClient::serviceAllocateFailed
   void serviceAllocateFailed(const IEvent &rEvent)
   {
      m_bIsOK = false;
      m_pCurrentService = NULL;
      ERR("Allocate Service Failed to allocate Service");
      PrintExceptionDescription(rEvent); // Builtin function to print exception events
      Post();
   }
   /// CSyncClient implementation of IServiceClient::serviceFreed
   void serviceFreed(TransactionID const &rTranID)
   {
      m_pCurrentService = NULL;
      Post();
   }
   /// CSyncClient implementation of IServiceClient::serviceEvent
   void serviceEvent(const IEvent &rEvent)
   {
      ERR("CSyncClient::serviceEvent Generic message handler (service)");
      ERR("Unexpected event 0x" << std::hex << rEvent.SubClassID());
   }
   // <end IServiceClient interface>
   ///////////////////////////////////////////////////////////////////////////

   ///////////////////////////////////////////////////////////////////////////
   // ADDITIONAL SERVICE CLIENT FUNCTIONS

   /// Synchronous version of Runtime::allocService()
   ///
   /// Called to allocate a Service, it calls the broker, which comes back in
   ///    serviceAllocated() or serviceAllocateFailed(). Those functions
   ///    set m_pCurrentService and m_pAALService, and then Post(),
   ///    which wakes up here in the Wait().
   /// @param[in] rManifest defines the Service to be Allocated
   AAL::IBase *syncAllocService(const NamedValueSet &rManifest)
   {
      m_runTime.allocService(this, rManifest);
      Wait();                                // Posted and Set in serviceAllocated
      if (!m_bIsOK) return NULL;             //    or serviceAllocateFailed
      return m_pCurrentService;
   }
   /// Synchronous version of IAALService::Release()
   void syncRelease(TransactionID const &TranID, btTime timeout=AAL_INFINITE_WAIT)
   {
      m_pAALService->Release( TranID, timeout);
      Wait();                                // Posted in CSyncClient::serviceFreed
   }
   ///////////////////////////////////////////////////////////////////////////

protected:
   btBool               m_bIsOK;             ///< Tracks status of this object
   AAL::XL::RT::Runtime m_runTime;           ///< XL framework
   CSemaphore           m_Sem;               ///< For synchronizing with the AAL runtime.
   bool                 m_bRunningStatus;    ///< tracks if the runtime is up and running
   AAL::IBase          *m_pCurrentService;   ///< Cached value from AllocateService. Pointer
                                             //    base object containing the Service.
                                             //    Should not normally be needed.
   AAL::IAALService    *m_pAALService;       ///< The generic AAL Service interface for the AFU,
                                             //    used for Release.
}; // CSyncClient

/// @} group CSyncClient

#endif // __AALSDK_UTILS_CSYNCCLIENT_H__

