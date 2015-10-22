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
/// @file AALRESourceManager.cpp
/// @brief Implementation of the Remote Resource Manager Service
/// @ingroup ResMgr
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Joseph Grecco, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 10/27/2008     JG       Initial version started@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

// AAL Runtime definitions
#include "aalsdk/AALTypes.h"

#include <aalsdk/Dispatchables.h>

#include "CResourceManager.h"
#include <aalsdk/aas/AALInProcServiceFactory.h>  // Defines InProc Service Factory

#include "aalsdk/CAALEvent.h"

#include "aalsdk/AALLoggerExtern.h"          // Logger
#include "aalsdk/osal/Thread.h"

#include "aalsdk/osal/Env.h"

#ifdef __ICC                           /* Deal with Intel compiler-specific overly sensitive remarks */
//   #pragma warning( push)
//   #pragma warning(disable:68)       // warning: integer conversion resulted in a change of sign.
                                       //    This is intentional
     #pragma warning(disable:177)      // remark: variable "XXXX" was declared but never referenced- OK
//   #pragma warning(disable:383)      // remark: value copied to temporary, reference to temporary used
//   #pragma warning(disable:593)      // remark: variable "XXXX" was set but never used - OK
     #pragma warning(disable:869)      // remark: parameter "XXXX" was never referenced
//   #pragma warning(disable:981)      // remark: operands are evaluated in unspecified order
     #pragma warning(disable:1418)     // remark: external function definition with no prior declaration
//   #pragma warning(disable:1419)     // remark: external declaration in primary source file
//   #pragma warning(disable:1572)     // remark: floating-point equality and inequality comparisons are unreliable
//   #pragma warning(disable:1599)     // remark: declaration hides variable "Args", or tid - OK
#endif



#define SERVICE_FACTORY AAL::InProcSvcsFact< AAL::CResourceManager >

#if defined ( __AAL_WINDOWS__ )
# pragma warning(push)
# pragma warning(disable : 4996) // destination of copy is unsafe
#endif // __AAL_WINDOWS__

#define RRM_VERSION_CURRENT  0
#define RRM_VERSION_REVISION 0
#define RRM_VERSION_AGE      0
#define RRM_VERSION          "0.0.0"

AAL_BEGIN_BUILTIN_SVC_MOD(SERVICE_FACTORY, librrm, AALRESOURCEMANAGER_API, RRM_VERSION, RRM_VERSION_CURRENT, RRM_VERSION_REVISION, RRM_VERSION_AGE)
   // Only default service commands for now.
AAL_END_SVC_MOD()

BEGIN_NAMESPACE(AAL)

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////             Resource Manager             ////////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
//=============================================================================
// The Resource Manager implements the AAL Resource Manager used by the Broker
//  It communicates with the Resource Manager Service through the Resource
//   Manager Proxy, abstracting the location and transport used by the RM
//   Service.

//=============================================================================
// Name: init
// Description: Initialize the object
// Interface: public
// Inputs: rtid - reference to a transaction ID
// Comments:
//   This is called via the base class construction chain.  Since this class is
//   derived from ServiceBase it can assume that all of the base members have
//.  been initialized.
//=============================================================================
btBool CResourceManager::init( IBase *pclientBase,
                               NamedValueSet const &optArgs,
                               TransactionID const &rtid)
{
   INamedValueSet const *pConfigRecord = NULL;
   btcString             sStartupMode   = NULL;       // for parsing environment
   std::string           strStartupMode;              // for reading environment

   // Save the client interface
   m_pResMgrClient = dynamic_ptr<IResourceManagerClient>(iidResMgrClient, getServiceClientBase());
   if( NULL == m_pResMgrClient ){
      // Sends a Service Client serviceAllocated callback
      initFailed(new CExceptionTransactionEvent( NULL,
                                                 rtid,
                                                 errBadParameter,
                                                 reasInvalidParameter,
                                                 strInvalidParameter));

   }

   //
   // Determine mode for allocating remote resource manager service.
   //

   // Check environment and optargs
   if ( Environment::GetObj()->Get("AAL_RESOURCEMANAGER_CONFIG_INPROC", strStartupMode) ) {
      sStartupMode = strStartupMode.c_str();
   } else {
      if ( ENamedValuesOK == optArgs.Get(AALRUNTIME_CONFIG_RECORD, &pConfigRecord) ) {
         if ( ENamedValuesOK != pConfigRecord->Get("AAL_RESOURCEMANAGER_CONFIG_INPROC", &sStartupMode) ) {
              // Not in config record
              sStartupMode = NULL;
         }
      }
   }

   // Parse envvar string, set m_rrmStartupMode accordingly (or leave at default)
   if (NULL != sStartupMode) {
      if (strcmp("always", sStartupMode) == 0) {
         m_rrmStartupMode = always;
      } else if (strcmp("never", sStartupMode) == 0) {
         m_rrmStartupMode = never;
      }
   }

   // RRM 'always' startup happens right here
   if (m_rrmStartupMode == always) {
      if (!startRRMService()) {
         initFailed(new CExceptionTransactionEvent(NULL,
                                                   rtid,
                                                   errSysSystemStarted,
                                                   reasInitError,
                                                   "RRM failed to start in 'always' mode"));
         return false;
      }
   }

   // Create an open channel to the remote resource manager
   if ( !m_RMProxy.Open() ) {
      // Sends a Service Client serviceAllocated callback
      initFailed(new CExceptionTransactionEvent( NULL,
                                                 rtid,
                                                 errDevice,
                                                 reasNoDevice,
                                                 strNoDevice));
      return true;
   }

   // Kick off the polling loop on the Proxy
   m_pProxyPoll = new OSLThread( CResourceManager::ProxyPollThread,
                                 OSLThread::THREADPRIORITY_NORMAL,
                                 this);
   if(NULL == m_pProxyPoll){
      m_RMProxy.Close();
      initFailed(new CExceptionTransactionEvent( NULL,
                                                 rtid,
                                                 errInternal,
                                                 reasCauseUnknown,
                                                 "Could not create RM Proxy Poll thread."));
   }
   // Sends a Service Client serviceAllocated callback
   initComplete(rtid);
   return true;
}

//=============================================================================
// Name: RequestResource
// Description: Used to request a service config record
// Interface: public
// Inputs: nvsManifest - describes the device to be requested
//                 tid - Transaction ID
// Outputs: none.
//=============================================================================
void CResourceManager::RequestResource(NamedValueSet const &nvsManifest,
                                       TransactionID const &tid)
{
   // Only allow one command be sent at a time
   AutoLock(this);

   // check if we need to start a remote resource manager
   // RequestResource is only called for services that are not SW-only, so
   // we don't need to check for that
   if (m_rrmStartupMode == automatic && !isRRMPresent()) {
      if (!startRRMService()) {
         getRuntime()->schedDispatchable(new ResourceManagerClientMessage(m_pResMgrClient,
                                                                          nvsManifest,
                                                                          ResourceManagerClientMessage::AllocateFailed,
                                                                          new CExceptionTransactionEvent( NULL,
                                                                                                          tid,
                                                                                                          errInternal,
                                                                                                          reasCauseUnknown,
                                                                                                          "Could not create RRM Service (in proc).")));
         return;
      }
   }

   // Send the request to the Resource Manager
   if(false == m_RMProxy.SendRequest(nvsManifest,tid) ){

      // Failed to send the request so schedule a callback
      getRuntime()->schedDispatchable(new ResourceManagerClientMessage(m_pResMgrClient,
                                                                       nvsManifest,
                                                                       ResourceManagerClientMessage::AllocateFailed,
                                                                       new CExceptionTransactionEvent(this,
                                                                                                      tid,
                                                                                                      errInternal,
                                                                                                      reasCauseUnknown,
                                                                                                      "Failed SendRequest on RM Proxy") ) );
      return;
  }

}

//=============================================================================
// Name: ProxyPollThread
// Description: Thread used to poll remote resource manager
// Interface: private
// Inputs: pThread - thread object
//         pContext - context - contains parameters for operation
// Outputs: none.
// Comments:
//=============================================================================
void CResourceManager::ProxyPollThread( OSLThread *pThread,
                                        void      *pContext)
{
   CResourceManager *This = static_cast<CResourceManager*>(pContext);
   This->ProcessRMMessages();
}

//=============================================================================
// Name: ProcessRMMessages
// Description: Process resource Manager Service Messages
// Interface: private
// Inputs: none
// Outputs: none.
// Comments: Processes all events until none are left
//=============================================================================
void CResourceManager::ProcessRMMessages()
{
   TransactionID  tid;
   NamedValueSet  nvs;
   btIID          result_code;

   while ( true == m_RMProxy.GetMessage(nvs, tid) ) {
      if(!nvs.Has(RM_MESSAGE_KEY_RESULTCODE)){
         // Cannot process improper message.
         //  log error and possibly throw error
         continue;
      }
      nvs.Get(RM_MESSAGE_KEY_RESULTCODE, &result_code);

      if ( errOK != result_code ) {

         if ( !m_RMProxy.IsOK() ) {
            perror("CResourceManager::ProcessRMMessages failed GetMessage()");
            //Proxy failed so generate an error
            getRuntime()->schedDispatchable(new ResourceManagerClientMessage(m_pResMgrClient,
                                                                             NamedValueSet(),
                                                                             ResourceManagerClientMessage::AllocateFailed,
                                                                             new CExceptionTransactionEvent(this,
                                                                                                            tid,
                                                                                                            errInternal,
                                                                                                            reasCauseUnknown,
                                                                                                            "CResourceManager::ProcessRMMessages failed GetMessage()") ) );
         } else {
            btIID reason_code;
            btcString reason_string;

            nvs.Get(RM_MESSAGE_KEY_REASONCODE, &reason_code);
            nvs.Get(RM_MESSAGE_KEY_REASONSTRING, &reason_string);

            //Generate an error
            getRuntime()->schedDispatchable(new ResourceManagerClientMessage(m_pResMgrClient,
                                                                             nvs,
                                                                             ResourceManagerClientMessage::AllocateFailed,
                                                                             new CExceptionTransactionEvent(this,
                                                                                                            tid,
                                                                                                            result_code,
                                                                                                            reason_code,
                                                                                                            reason_string) ) );
         }
      } else {   // pMessage->result_code() == rms_resultOK
         rm_msg_ids  id;
         if(!nvs.Has(RM_MESSAGE_KEY_ID)){
            // Cannot process improper message.
            //  log error and possibly throw error
            continue;
         }
         nvs.Get(RM_MESSAGE_KEY_ID, reinterpret_cast<btUnsignedInt*>(&id));

         if ( rm_msg_id_shutdown == id ) {
            return;                                // NORMAL EXIT FROM FUNCTION
         } else {

            // TODO Message processing very limited at this time
            btObjectType handle = NULL;
            if( ENamedValuesOK != nvs.Get(keyRegHandle, &handle) ){
               handle = NULL;
            }

            if( NULL == handle ){
               getRuntime()->schedDispatchable(new ResourceManagerClientMessage(m_pResMgrClient,
                                                                                nvs,
                                                                                ResourceManagerClientMessage::AllocateFailed,
                                                                                new CExceptionTransactionEvent(this,
                                                                                                               tid,
                                                                                                               errAllocationFailure,
                                                                                                               reasResourcesNotAvailable,
                                                                                                               strNoResourceDescr) ) );
            }else{
               // Generate the event
               getRuntime()->schedDispatchable(new ResourceManagerClientMessage(m_pResMgrClient,
                                                                                nvs,
                                                                                ResourceManagerClientMessage::Allocated,
                                                                                tid) );
            }
         }
      }
      nvs.Empty();
   }
}  // CResourceManager::ProcessRMMessages()


//=============================================================================
// Name: ~CResourceManagerClientService
// Description: Destructor
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments:
//=============================================================================
void CResourceManager::StopMessagePump()
{

   // Shutdown the message pump by putting in a reqid_Shutdown to force a
   //    wake-up and let the kernel know it is going down.

   DEBUG_CERR("~StopMessagePump: sending reqid_Shutdown.\n");

   m_RMProxy.SendStop();

   // if message pump thread is running, need to wait for it to terminate
   if ( NULL != m_pProxyPoll ) {
      // Wait for the Message delivery thread to terminate
      DEBUG_CERR("~CResourceManagerClientService: waiting for Receive Thread to Join. 2 of 5\n");

      m_pProxyPoll->Join();

      DEBUG_CERR("~StopMessagePump: Receive Thread has Joined.\n");

      delete m_pProxyPoll;
      m_pProxyPoll = NULL;
   }

   // Close the physical device
   DEBUG_CERR("~StopMessagePump: shutting down UI Client file.\n");

   // Close the channel
   m_RMProxy.Close();

   DEBUG_CERR("~StopMessagePump: done.\n");

}

//=============================================================================
// Name: ~CResourceManagerClientService
// Description: Destructor
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments:
//=============================================================================
CResourceManager::~CResourceManager()
{

} // End of CResourceManagerClientService::~CResourceManagerClientService

//=============================================================================
// Name: Release
// Description: Release the service
// Interface: public
// Comments:
//=============================================================================
btBool CResourceManager::Release(TransactionID const &rTranID, btTime timeout)
{
   // TODO  - Send the shutdown to the driver and wait until done before issuing this

   // FIXME - there's a race condition here - it happens that the runtime
   // already shut down the remote resource manager; in that case,
   // m_pRRMAALService will not be NULL, but calling Release() on it will
   // segfault / call pure virtual.
   // Temporary fix: never shut down remote resource manager, but always let
   // runtime cleanup handle it.

#if 0
   // If we never allocated a remote resource manager service (i.e. it's
   // running externally), go ahead and release self.
   if (m_pRRMAALService == NULL) {
#endif
      StopMessagePump();
      ServiceBase::Release(rTranID, timeout);   // This function blocks until pump is stopped.

      return true;
#if 0
   }

   // If we have allocated a remote resource manager service, wrap original
   // transaction id and timeout and release it.
   ReleaseContext *prc = new ReleaseContext(rTranID, timeout);
   btApplicationContext appContext = reinterpret_cast<btApplicationContext>(prc);
   return m_pRRMAALService->Release(TransactionID(appContext));

   // in the latter case, further shutdown happens in serviceReleased below.
#endif
}


btBool CResourceManager::startRRMService() {
   NamedValueSet ResMgrManifest;
   NamedValueSet ResMgrConfigRecord;

   // Construct config record and manifest
   ResMgrConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME,
                    "libAASResMgr");
   ResMgrConfigRecord.Add(AAL_FACTORY_CREATE_SOFTWARE_SERVICE, true);

   ResMgrManifest.Add(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &ResMgrConfigRecord);
   ResMgrManifest.Add(AAL_FACTORY_CREATE_SERVICENAME, "CAASResourceManager");

   // allocate service, will be started in serviceAllocated()
   getRuntime()->allocService(this, ResMgrManifest, TransactionID());
   m_sem.Wait();
   return IsOK();
}


// TODO: if successful, might want to keep file open and pass fd to
//       service construction. Otherwise, something might happen between our
//       close() and the service's open().
btBool CResourceManager::isRRMPresent()
{
   int fd = open("/dev/aalrms", O_RDWR);
   if (fd == -1) {
      // Could not open file, check reason
      if (errno == EBUSY) {
         // EBUSY probably means RRM is running
         return true;
      } else {
         // Unexpected error code
         AAL_ERR(LM_ResMgr, "open of /dev/aalrms failed unexpectedly with errno " << errno << "(" << strerror(errno) << ")");
         return false;
      }
   } else {
      // Could open file, no RRM present
      close(fd);
      return false;
   }
}


/*
 * IServiceClient methods
 */
void CResourceManager::serviceAllocated(IBase               *pServiceBase,
                              TransactionID const &rTranID)
{
   // Store ResMgrService pointer
   m_pRRMService = dynamic_ptr<IResMgrService>(iidResMgrService, pServiceBase);
   if (!m_pRRMService) {
      // TODO: handle error
      return;
   }

   // Store AAL service pointer (needed for Release())
   m_pRRMAALService = dynamic_ptr<IAALService>(iidService, pServiceBase);
   ASSERT(NULL != m_pRRMAALService);

   // run remote resource manager in separate thread
   m_pRRMService->start(TransactionID());
   // unblock semaphore
   m_sem.Post(1);
}

void CResourceManager::serviceAllocateFailed(const IEvent &rEvent)
{
   AAL_ERR(LM_ResMgr, "Allocation of CResMgr failed.");
   PrintExceptionDescription(rEvent);
   m_bIsOK = false;
   m_sem.Post(1);
}

void CResourceManager::serviceReleased(TransactionID const &rTranID)
{
   // clean up
   ReleaseContext *prc = reinterpret_cast<ReleaseContext *>(rTranID.Context());
   StopMessagePump();

   ServiceBase::Release(prc->tranID, prc->timeout);   // This function blocks until pump is stopped.
}

void CResourceManager::serviceReleaseFailed(const IEvent &rEvent)
{
   AAL_ERR(LM_ResMgr, "Release of CResMgr failed.");
}

void CResourceManager::serviceEvent(const IEvent &rEvent)
{
   AAL_ERR(LM_ResMgr, "Unexpected service event.");
}
/*
 * End of IServiceClient methods
 */

END_NAMESPACE(AAL)

