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
/// @file ALI.cpp
/// @brief Implementation of ALI AFU Hardware Service.
/// @ingroup ALI
/// @verbatim
/// Accelerator Abstraction Layer Sample Application
///
///    This application is for example purposes only.
///    It is not intended to represent a model for developing commercially-deployable applications.
///    It is designed to show working examples of the AAL programming model and APIs.
///
///
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 05/11/2016     HM       Initial version.@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include <aalsdk/utils/ResMgrUtilities.h>

#include "HWALIFME.h"
#include "HWALIPORT.h"
#include "HWALIAFU.h"
#include "HWALIReconf.h"
#include "HWALISigTap.h"
#include "ASEALIAFU.h"

#include "ALIBase.h"

#include "ALIAIATransactions.h"
#include "ALI.h"
#include "aalsdk/aas/Dispatchables.h"



BEGIN_NAMESPACE(AAL)

/// @addtogroup ALI
/// @{

#ifdef INFO
# undef INFO
#endif // INFO
#define INFO(x) AAL_INFO(LM_AFU, __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() : " << x << std::endl)
#ifdef ERR
# undef ERR
#endif // ERR
#define ERR(x) AAL_ERR(LM_AFU, __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() : " << x << std::endl)
#ifdef VERBOSE
# undef VERBOSE
#endif // VERBOSE
#define VERBOSE(x) AAL_VERBOSE(LM_AFU, __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() : " << x << std::endl)


// ===========================================================================
//
// ALI implementation
//
// ===========================================================================

//
// init.
//
// Inits ALI Service, possibly selective exposure of
//       interfaces based on results
//
btBool ALI::init(IBase               *pclientBase,
                 NamedValueSet const &optArgs,
                 TransactionID const &TranID)
{
   btHANDLE devHandle;
   bt32bitInt targetType;

   m_pSvcClient = pclientBase;
   ASSERT( NULL != m_pSvcClient );

   //
   // Allocate AIA service. Init is completed in serviceAllocated callback.
   //

   // ASE Resource
   if( optArgs.Has(ALIAFU_NVS_KEY_TARGET) ) {

      optArgs.Get(ALIAFU_NVS_KEY_TARGET, &targetType);

      if( targetType == ali_afu_ase) {

         m_tidSaved = TranID;
         ASEInit();

         initComplete(TranID);
         return true;

      }
   }

   // HW ALI Resource
   NamedValueSet nvsManifest;
   NamedValueSet nvsConfigRecord;

   nvsConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libaia");
   nvsConfigRecord.Add(AAL_FACTORY_CREATE_SOFTWARE_SERVICE, true);
   nvsManifest.Add(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &nvsConfigRecord);
   nvsManifest.Add(AAL_FACTORY_CREATE_SERVICENAME, "AIA");

   // add hardware handle obtained by resource manager
   if( optArgs.Has(keyRegHandle) ) {
      optArgs.Get(keyRegHandle, &devHandle);
   }else {
      initFailed(new CExceptionTransactionEvent( NULL,
                                                TranID,
                                                errBadParameter,
                                                reasNoDevice,
                                                "No device handle in Configuration Record!"));
      return true;
    }
   nvsManifest.Add(keyRegHandle, devHandle);

   // Set AIA Service Proxy interface
   if ( EObjOK != SetInterface(iidAFUProxyClient, dynamic_cast<IAFUProxyClient *>(this)) ){
      m_bIsOK = false;
   }  // for AFUProy

   m_tidSaved = TranID;
   getRuntime()->allocService(this, nvsManifest, TransactionID());

   // initComplete happens in serviceAllocated()
   return true;
}

//
// Release. Release service
//
btBool ALI::Release(TransactionID const &TranID, btTime timeout)
{
   bt32bitInt targetType;
   // Wrap original transaction id and timeout
   ReleaseContext *prc = new ReleaseContext(TranID, timeout);
   btApplicationContext appContext = reinterpret_cast<btApplicationContext>(prc);



   if( OptArgs().Has(ALIAFU_NVS_KEY_TARGET) ){
      OptArgs().Get(ALIAFU_NVS_KEY_TARGET, &targetType);

      if(targetType == ali_afu_ase) {
         (static_cast<CASEALIAFU *>(m_pALIBase))->ASERelease();
      }
   }

   if(m_pALIBase) {
      delete m_pALIBase ;
      m_pALIBase =NULL;
   }

   // Release ALI / AFUProxy
   ASSERT(m_pAALService != NULL);
   return m_pAALService->Release(TransactionID(appContext), timeout);
}


/*
 * IServiceClient methods (callbacks from AIA service)
 */

// Service allocated callback
void ALI::serviceAllocated(IBase               *pServiceBase,
                           TransactionID const &rTranID)
{
   // Store ResMgrService pointer
   m_pAFUProxy = dynamic_ptr<IAFUProxy>(iidAFUProxy, pServiceBase);
   if (!m_pAFUProxy) {
      // TODO: handle error
      initFailed(new CExceptionTransactionEvent( NULL,
                                                 m_tidSaved,
                                                 errBadParameter,
                                                 reasMissingInterface,
                                                 "Error: Missing AFUProxy interface."));
      return;
   }

   // Store AAL service pointer
   m_pAALService = dynamic_ptr<IAALService>(iidService, pServiceBase);
   if (!m_pAALService) {
      // TODO: handle error
      initFailed(new CExceptionTransactionEvent( NULL,
                                                 m_tidSaved,
                                                 errBadParameter,
                                                 reasMissingInterface,
                                                 "Error: Missing service base interface."));
      return;
   }

   INamedValueSet const *pConfigRecord;
   if(!OptArgs().Has(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED)){
      AAL_ERR( LM_All, "No Config Record");
      initFailed(new CExceptionTransactionEvent( NULL,
                                                 m_tidSaved,
                                                 errAllocationFailure,
                                                 reasInvalidParameter,
                                                 "Error: AFU Configuration information invalid. No Config Record."));
      return ;
   }

   OptArgs().Get(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &pConfigRecord );

   // Service Library to use
   btcString pAFUID;
   if(!pConfigRecord->Has(keyRegAFU_ID)){
      AAL_ERR( LM_All, "No AFU ID in Config Record");
      initFailed(new CExceptionTransactionEvent( NULL,
                                                 m_tidSaved,
                                                 errAllocationFailure,
                                                 reasInvalidParameter,
                                                 "Error: AFU Configuration information invalid. No AFU ID in Config Record."));
      return ;

   }

   pConfigRecord->Get(keyRegAFU_ID, &pAFUID);

   // std::cout << " HWALI::serviceAllocated pAFUID "<< pAFUID<< std::endl;

   if( 0 == strcmp(pAFUID, CCIP_FME_AFUID)) {
      // FME Resource Allocated
      setFMEInterfaces();

   } else if( 0 == strcmp(pAFUID, ALI_AFUID_UAFU_CONFIG)) {
      // PR Resource Allocated
      setReconfInterfaces();

   } else if( 0 == strcmp(pAFUID, CCIP_PORT_AFUID)) {
      // PORT Resource Allocated
      setPortInterfaces();

   } else if ( 0 == strcmp(pAFUID, CCIP_STAP_AFUID)) {
      // Signal Tap Resource Allocated
      setSigTapInterfaces();

   } else {
      // AFU Resource Allocated
      setAFUInterfaces();
   }

   initComplete(m_tidSaved);
}

//
// setSigTapInterfaces.Sets Signal Tap ALI Interfaces.
//
btBool ALI::setSigTapInterfaces()
{
   if(NULL == m_pALIBase) {

      m_pALIBase = new CHWALISigTap(m_pSvcClient,this,m_tidSaved,m_pAFUProxy);
      if(NULL == m_pALIBase) {
         initFailed(new CExceptionTransactionEvent( NULL,
                                                    m_tidSaved,
                                                    errMemory,
                                                    reasUnknown,
                                                    "Error: Failed to allocate HW ALI AFU."));

         return false;
      }
   }

   if( EObjOK != SetInterface(iidALI_MMIO_Service, dynamic_cast<IALIMMIO *>(m_pALIBase)) ){
      goto FAIL;
   }

   if( EObjOK != SetInterface(iidALI_STAP_Service, dynamic_cast<IALISignalTap *>(m_pALIBase)) ){
      goto FAIL;
   }

   return true;
FAIL:
   m_bIsOK = false;
   initFailed(new CExceptionTransactionEvent( NULL,
                                              m_tidSaved,
                                              errCreationFailure,
                                              reasUnknown,
                                              "Error: Could not register interface."));

   return false;
}

//
// setPortInterfaces.Sets Port ALI Interfaces.
//
btBool ALI::setPortInterfaces()
{
   if(NULL == m_pALIBase) {

      m_pALIBase = new CHWALIPORT(m_pSvcClient,this,m_tidSaved,m_pAFUProxy);
      if(NULL == m_pALIBase) {
         initFailed(new CExceptionTransactionEvent( NULL,
                                                    m_tidSaved,
                                                    errMemory,
                                                    reasUnknown,
                                                    "Error: Failed to allocate HW ALI AFU."));

         return false;
      }
   }

   if( EObjOK != SetInterface(iidALI_MMIO_Service, dynamic_cast<IALIMMIO *>(m_pALIBase)) ){
      goto FAIL;
   }

   if( EObjOK != SetInterface(iidALI_PORTERR_Service, dynamic_cast<IALIPortError *>(m_pALIBase)) ){
      goto FAIL;
   }

   if(false == (dynamic_cast<CHWALIPORT *>(m_pALIBase))->mapMMIO()) {
      goto FAIL;
   }

   return true;
FAIL:
   m_bIsOK = false;
   initFailed(new CExceptionTransactionEvent( NULL,
                                              m_tidSaved,
                                              errCreationFailure,
                                              reasUnknown,
                                              "Error: Could not register interface."));
   return false;
}

//
// setAFUInterfaces.Sets AFU ALI Interfaces.
//
btBool ALI::setAFUInterfaces()
{
   if(NULL == m_pALIBase) {

      m_pALIBase = new CHWALIAFU(m_pSvcClient,this,m_tidSaved,m_pAFUProxy);
      if(NULL == m_pALIBase) {
         initFailed(new CExceptionTransactionEvent( NULL,
                                                    m_tidSaved,
                                                    errMemory,
                                                    reasUnknown,
                                                    "Error: Failed to allocate HW ALI AFU."));

         return false;
      }
   }

   if( EObjOK != SetInterface(iidALI_MMIO_Service, dynamic_cast<IALIMMIO *>(m_pALIBase)) ){
      goto FAIL;
   }

   if( EObjOK != SetInterface(iidALI_UMSG_Service, dynamic_cast<IALIUMsg *>(m_pALIBase)) ){
      goto FAIL;
   }

   if( EObjOK != SetInterface(iidALI_BUFF_Service, dynamic_cast<IALIBuffer *>(m_pALIBase)) ){
      goto FAIL;
   }

   if( EObjOK != SetInterface(iidALI_RSET_Service, dynamic_cast<IALIReset *>(m_pALIBase)) ){
      goto FAIL;
   }

   if(false == (dynamic_cast<CHWALIAFU *>(m_pALIBase))->mapMMIO()) {
      goto FAIL;
   }

   return true;

FAIL:
   m_bIsOK = false;
   initFailed(new CExceptionTransactionEvent( NULL,
                                              m_tidSaved,
                                              errCreationFailure,
                                              reasUnknown,
                                              "Error: Could not register interface."));
   return false;
}

//
// setReconfInterfaces.Sets PR ALI Interfaces.
//
btBool ALI::setReconfInterfaces()
{
   if(NULL == m_pALIBase) {

      m_pALIBase = new CHWALIReconf(m_pSvcClient,this,m_tidSaved,m_pAFUProxy);

      if(NULL == m_pALIBase) {
         initFailed(new CExceptionTransactionEvent( NULL,
                                                    m_tidSaved,
                                                    errMemory,
                                                    reasUnknown,
                                                    "Error: Failed to allocate HW ALI AFU."));

         return false;
      }
   }

   // Sets Reconfigure client interfaces
   (dynamic_cast<CHWALIReconf *>(m_pALIBase))->setReconfClientInterface();

   if( EObjOK != SetInterface(iidALI_CONF_Service, dynamic_cast<IALIReconfigure *>(m_pALIBase)) ){
      initFailed(new CExceptionTransactionEvent( NULL,
                                                 m_tidSaved,
                                                 errCreationFailure,
                                                 reasUnknown,
                                                " Error: Could not register interface."));
      return false;
   }
   return true;
}

//
// setFMEInterfaces.Sets FME ALI Interfaces.
//
btBool ALI::setFMEInterfaces()
{
   if(NULL == m_pALIBase) {

     m_pALIBase = new CHWALIFME(m_pSvcClient,this,m_tidSaved,m_pAFUProxy);
     if(NULL == m_pALIBase) {
         initFailed(new CExceptionTransactionEvent( NULL,
                                                    m_tidSaved,
                                                    errMemory,
                                                    reasUnknown,
                                                    "Error: Failed to allocate HW ALI AFU."));

         return false;
         }
   }

   if( EObjOK != SetInterface(iidALI_MMIO_Service, dynamic_cast<IALIMMIO *>(m_pALIBase)) ){
       goto FAIL;
   }

   if( EObjOK != SetInterface(iidALI_PERF_Service, dynamic_cast<IALIPerf *>(m_pALIBase)) ){
      goto FAIL;
   }

   if( EObjOK != SetInterface(iidALI_FMEERR_Service, dynamic_cast<IALIFMEError *>(m_pALIBase)) ){
      goto FAIL;
   }

   if( EObjOK != SetInterface(iidALI_POWER_Service, dynamic_cast<IALIPower *>(m_pALIBase)) ){
      goto FAIL;
   }

   if( EObjOK != SetInterface(iidALI_TEMP_Service, dynamic_cast<IALITemperature *>(m_pALIBase)) ){
      goto FAIL;
   }

   if(false == (dynamic_cast<CHWALIFME *>(m_pALIBase))->mapMMIO()) {
      goto FAIL;
   }

   return true;
FAIL:
   m_bIsOK = false;
   initFailed(new CExceptionTransactionEvent( NULL,
                                              m_tidSaved,
                                              errCreationFailure,
                                              reasUnknown,
                                              "Error: Could not register interface."));
   return false;
}

//
// ASEInit.Sets FME ALI Interfaces.
//
btBool ALI::ASEInit()
{
   if(m_pALIBase == NULL) {

      m_pALIBase = new CASEALIAFU(m_pSvcClient,this,m_tidSaved);

      if(m_pALIBase == NULL) {
         initFailed(new CExceptionTransactionEvent( NULL,
                                                    m_tidSaved,
                                                    errMemory,
                                                    reasUnknown,
                                                    "Error: Failed to allocate HW ALI AFU."));

         return false;
         }
   }
   return ((dynamic_cast<CASEALIAFU *>(m_pALIBase))->ASEInit());
}

void ALI::serviceReleaseRequest(IBase *pServiceBase, const IEvent &rEvent)
{
   ERR("Recieved unhandled serviceReleaseRequest() from AFU PRoxy\n");
}

// Service allocated failed callback
void ALI::serviceAllocateFailed(const IEvent &rEvent) {

   m_bIsOK = false;
   initFailed(new CExceptionTransactionEvent( NULL,
                                              m_tidSaved,
                                              errAllocationFailure,
                                              reasUnknown,
                                              "Error: Failed to allocate AIA ."));

}

// Service released callback
void ALI::serviceReleased(TransactionID const &rTranID) {
   ReleaseContext *prc = reinterpret_cast<ReleaseContext *>(rTranID.Context());
   ServiceBase::Release(prc->TranID, prc->timeout);
}

// Service released failed callback
void ALI::serviceReleaseFailed(const IEvent &rEvent) {
   m_bIsOK = false;
}

// Callback for generic events
void ALI::serviceEvent(const IEvent &rEvent) {
   // TODO: handle unexpected events
   ASSERT(false);
}

// ---------------------------------------------------------------------------
// IAFUProxyClient interface implementation
// ---------------------------------------------------------------------------

// Callback for ALIAFUProxy
void ALI::AFUEvent(AAL::IEvent const &theEvent) {

   (static_cast<CHWALIBase *>(m_pALIBase))->AFUEvent(theEvent);
}

/// @} group ALI

END_NAMESPACE(AAL)


#if defined( __AAL_WINDOWS__ )

BOOL APIENTRY DllMain(HANDLE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved)
{
   switch ( ul_reason_for_call ) {
      case DLL_PROCESS_ATTACH :
         break;
      case DLL_THREAD_ATTACH  :
         break;
      case DLL_THREAD_DETACH  :
         break;
      case DLL_PROCESS_DETACH :
         break;
   }
   return TRUE;
}

#endif // __AAL_WINDOWS__



// The following declarations implement the AAL Service factory and entry
//  point.

// Define the factory to use for this service. In this example the service
//  will be implemented in-process.  There are other implementations available for
//  services implemented remotely, for example via TCP/IP.
#define SERVICE_FACTORY AAL::InProcSvcsFact< AAL::ALI  >


#if defined ( __AAL_WINDOWS__ )
# pragma warning(push)
# pragma warning(disable : 4996) // destination of copy is unsafe
#endif // __AAL_WINDOWS__

ALI_BEGIN_SVC_MOD(SERVICE_FACTORY)
   /* No commands other than default, at the moment. */
ALI_END_SVC_MOD()

#if defined ( __AAL_WINDOWS__ )
# pragma warning(pop)
#endif // __AAL_WINDOWS__




