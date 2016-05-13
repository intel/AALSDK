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
/// @file HWALIReconf.cpp
/// @brief Definitions for ALI Hardware AFU Service.
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
#include "ALIAIATransactions.h"
#include "aalsdk/aas/Dispatchables.h"
#include "HWALIReconf.h"

BEGIN_NAMESPACE(AAL)

/// @addtogroup ALI
/// @{

// Bitstream File extension
#define BITSTREAM_FILE_EXTENSION ".rbf"

//
// ctor. CHWALIFME class constructor
//
CHWALIReconf::CHWALIReconf( IBase *pSvcClient,
                            IServiceBase *pServiceBase,
                            TransactionID transID,
                            IAFUProxy *pAFUProxy): CHWALIBase(pSvcClient,pServiceBase,transID,pAFUProxy),
                            m_pReconClient(NULL)
{

}

//
// setReconfClientInterface, sets Reconfigure client interfaces
//
btBool CHWALIReconf::setReconfClientInterface()
{

   m_pReconClient = dynamic_ptr<IALIReconfigure_Client>(iidALI_CONF_Service_Client, m_pSvcClient);

   ASSERT( NULL != m_pReconClient ); //QUEUE object failed
   if(NULL == m_pReconClient) {

         m_pServiceBase->initFailed(new CExceptionTransactionEvent( NULL,
                                                                    m_tidSaved,
                                                                    errBadParameter,
                                                                    reasMissingInterface,
                                                                    "Client did not publish IALIReconfigure_Client Interface"));
         return false;
      }
   return true;
}
// ---------------------------------------------------------------------------
// IALIReconfigure interface implementation
// ---------------------------------------------------------------------------

/// @brief Deactivate an AFU in preparation for it being reconfigured.
///
/// Basically, if there is an AFU currently instantiated and connected to an
///    application, then this will send an exception to the application indicating
///    that it should release the AFU. There can be a timeout option that specifies
///    that if the application does not Release within a particular time, then
///    the AFU will be yanked. Then, a CleanIt!(tm) AFU will be loaded to clear
///    out all the gates and clear the FPGA memory banks.
///
/// TODO: Implementation needs to be via driver transaction
///
/// @param[in]  pNVS Pointer to Optional Arguments if needed. Defaults to NULL.
/// @return     void. Callback in IALIReconfigureClient.
///
void CHWALIReconf::reconfDeactivate( TransactionID const &rTranID,
                                 NamedValueSet const &rInputArgs)
{
   AFUDeactivateTransaction deactivatetrans(rTranID,rInputArgs);

   if(!deactivatetrans.IsOK() ){

      AAL_ERR( LM_All,"Deactivate failed");
      getRuntime()->schedDispatchable(new AFUDeactivateFailed( m_pReconClient,new CExceptionTransactionEvent( NULL,
                                                                                                              rTranID,
                                                                                                              errCauseUnknown,
                                                                                                              reasBadConfiguration,
                                                                                                              "Error: Bad input Configuration")));

      return;
     }

   // Send transaction
   m_pAFUProxy->SendTransaction(&deactivatetrans);
   if(deactivatetrans.getErrno() != uid_errnumOK){
      AAL_ERR( LM_All,"Deactivate failed");

      getRuntime()->schedDispatchable(new AFUDeactivateFailed( m_pReconClient,new CExceptionTransactionEvent( NULL,
                                                                                                              rTranID,
                                                                                                              errCauseUnknown,
                                                                                                              reasUnknown,
                                                                                                              "Error: Failed transaction")));
      return;
   }

}

/// @brief Configure an AFU.
///
/// Download the defined bitstream to the PR region. Initially, the bitstream
///    is a file name. Later, it might be a goal record, and that is why the
///    parameter is an NVS. It is also possible in the NVS to specify a PR number
///    if that is relevant, e.g. for the PF driver.
///
/// TODO: Implementation needs to be via driver transaction
///
/// @param[in]  pNVS Pointer to Optional Arguments. Initially need a bitstream.
/// @return     void. Callback in IALIReconfigureClient.
///
void CHWALIReconf::reconfConfigure( TransactionID const &rTranID,
                                NamedValueSet const &rInputArgs)
{
   btByte *bufptr                = NULL;
   std::streampos filesize       = 0;

   if(rInputArgs.Has(AALCONF_FILENAMEKEY)){
      btcString filename;
      rInputArgs.Get(AALCONF_FILENAMEKEY, &filename);

      // File extension is not .rbf , Dispatch error Message "Wrong bitstream file extension"
      std::string bitfilename(filename);
      if(BITSTREAM_FILE_EXTENSION != (bitfilename.substr(bitfilename.find_last_of("."))))  {
         // file extension invalid
         AAL_ERR( LM_All, "Wrong bitstream file extension \n");
         getRuntime()->schedDispatchable(new AFUReconfigureFailed( m_pReconClient,new CExceptionTransactionEvent( NULL,
                                                                                                                  rTranID,
                                                                                                                  errBadParameter,
                                                                                                                  reasParameterNameInvalid,
                                                                                                                  "Error: Wrong bitstream file extension.")));

         return ;
      }

      std::ifstream bitfile(filename, std::ios::binary );

      if(!bitfile.good()) {
         // file is invalid, Dispatch error Message "Wrong bitstream file path"
         AAL_ERR( LM_All, "Wrong bitstream file path \n ");
         getRuntime()->schedDispatchable(new AFUReconfigureFailed( m_pReconClient,new CExceptionTransactionEvent( NULL,
                                                                                                                  rTranID,
                                                                                                                  errFileError,
                                                                                                                  reasParameterNameInvalid,
                                                                                                                  "Error: Wrong bitstream file path.")));

         return ;
      }

      bitfile.seekg( 0, std::ios::end );
      filesize = bitfile.tellg();

      if(0 == filesize) {
         // file size is 0, Dispatch error Message "Zero bitstream file size"
         AAL_ERR( LM_All, "Zero bitstream file size \n");
         getRuntime()->schedDispatchable(new AFUReconfigureFailed( m_pReconClient,new CExceptionTransactionEvent( NULL,
                                                                                                                  rTranID,
                                                                                                                  errFileError,
                                                                                                                  reasParameterValueInvalid,
                                                                                                                  "Error: Zero bitstream file size.")));

         return ;
      }


      bitfile.seekg( 0, std::ios::beg );
      bufptr = new(std::nothrow) btByte[filesize];

      if(NULL == bufptr) {
         // Memory  allocation failed  error Message "Failed to allocate file buffer"
         AAL_ERR( LM_All, "Failed to allocate bitstream file buffer \n");
         getRuntime()->schedDispatchable(new AFUReconfigureFailed( m_pReconClient,new CExceptionTransactionEvent( NULL,
                                                                                                                  rTranID,
                                                                                                                  errAllocationFailure,
                                                                                                                  reasUnknown,
                                                                                                                  "Error: Failed to allocate file buffer.")));
         return ;
      }

      bitfile.read(reinterpret_cast<char *>(bufptr), filesize);

   }else{

      getRuntime()->schedDispatchable(new AFUReconfigureFailed( m_pReconClient,new CExceptionTransactionEvent( NULL,
                                                                                                               rTranID,
                                                                                                               errBadParameter,
                                                                                                               reasMissingParameter,
                                                                                                               "Error: No bitfile source.")));
      return;

#if 0 // Test code
      bufptr = reinterpret_cast<btByte*>(malloc(100));
      *bufptr = '<';
      memset(bufptr+1,'*',97);
      *(bufptr+98) = '>';
      *(bufptr+99) = 0;
      filesize = 100;
#endif
   }

   AFUConfigureTransaction configuretrans(reinterpret_cast<btVirtAddr>(bufptr), filesize, rTranID,rInputArgs);
   // Send transaction
   m_pAFUProxy->SendTransaction(&configuretrans);
   if(configuretrans.getErrno() != uid_errnumOK){
      AAL_ERR( LM_All,"Reconfigure failed");
      getRuntime()->schedDispatchable(new AFUReconfigureFailed( m_pReconClient,new CExceptionTransactionEvent( NULL,
                                                                                                               rTranID,
                                                                                                               errCauseUnknown,
                                                                                                               reasUnknown,
                                                                                                               "Error: Failed transaction")));
      if(bufptr)
         delete bufptr;
      return;
   }
   if(bufptr)
      delete bufptr;
}

/// @brief Activate an AFU after it has been reconfigured.
///
/// Once the AFU has been reconfigured there needs to be a "probe" to load
///    the AFU configuration information, e.g. AFU_ID, so that the associated
///    service can be loaded and the whole shebang returned to the application.
///
/// TODO: Implementation needs to be via driver transaction
///
/// @param[in]  pNVS Pointer to Optional Arguments if needed. Defaults to NULL.
/// @return     void. Callback in IALIReconfigureClient.
///
void CHWALIReconf::reconfActivate( TransactionID const &rTranID,
                               NamedValueSet const &rInputArgs)
{
   AFUActivateTransaction activatetrans(rTranID);
   // Send transaction
   m_pAFUProxy->SendTransaction(&activatetrans);
   if(activatetrans.getErrno() != uid_errnumOK){
      AAL_ERR( LM_All,"Activate failed");

      getRuntime()->schedDispatchable(new AFUActivateFailed( m_pReconClient,new CExceptionTransactionEvent( NULL,
                                                                                                            rTranID,
                                                                                                            errCauseUnknown,
                                                                                                            reasUnknown,
                                                                                                            "Error: Failed transaction")));
      return;
   }

}

//
// AFUEvent,AFU Event Handler.
//
void CHWALIReconf::AFUEvent(AAL::IEvent const &theEvent)
{
   IUIDriverEvent *puidEvent = dynamic_ptr<IUIDriverEvent>(evtUIDriverClientEvent,
                                                           theEvent);

   ASSERT(NULL != puidEvent);

   std::cerr << "Got CHWALIReconf AFU event CHWALIReconf type " << puidEvent->MessageID() << "\n" << std::endl;

   switch(puidEvent->MessageID())
   {

   case rspid_AFU_Response:
   {

      struct aalui_AFUResponse* presp = reinterpret_cast<struct aalui_AFUResponse *>(puidEvent->Payload());
      switch(presp->respID)
           {
           case uid_afurespDeactivateComplete:
              {
                 std::cerr << "uid_afurespDeactivateComplete "  << "\n" << std::endl;
                 if( uid_errnumOK != puidEvent->ResultCode()){
                    getRuntime()->schedDispatchable(new AFUDeactivateFailed(m_pReconClient,new CExceptionTransactionEvent(NULL,
                                                                                                                          puidEvent->msgTranID(),
                                                                                                                          puidEvent->ResultCode(),
                                                                                                                          reasUnknown,
                                                                                                                          "Error: Deactivate failed. Check Exception number against uid_errnum_e codes")));
                 }else{
                    getRuntime()->schedDispatchable(new AFUDeactivated(m_pReconClient, TransactionID(puidEvent->msgTranID())));
                 }
                 return;
              }
           case uid_afurespActivateComplete:
              {
                 std::cerr << "uid_afurespActivateComplete "  << "\n" << std::endl;
                 if( uid_errnumOK != puidEvent->ResultCode()){
                     getRuntime()->schedDispatchable(new AFUActivateFailed(m_pReconClient,new CExceptionTransactionEvent(NULL,
                                                                                                                         puidEvent->msgTranID(),
                                                                                                                         puidEvent->ResultCode(),
                                                                                                                         reasUnknown,
                                                                                                                         "Error: Activate failed. Check Exception number against uid_errnum_e codes")));
                  }else{
                     getRuntime()->schedDispatchable(new AFUActivated(m_pReconClient, TransactionID(puidEvent->msgTranID())));
                  }
                 return;
              }
           case uid_afurespConfigureComplete:
              {
                 std::cerr << "uid_afurespConfigureComplete "  << "\n" << std::endl;
                 if( uid_errnumOK !=puidEvent->ResultCode()){
                     getRuntime()->schedDispatchable(new AFUReconfigureFailed(m_pReconClient,new CExceptionTransactionEvent(NULL,
                                                                                                                            puidEvent->msgTranID(),
                                                                                                                            puidEvent->ResultCode(),
                                                                                                                            reasUnknown,
                                                                                                                            "Error: Configure failed. Check Exception number against uid_errnum_e codes")));
                  }else{
                     getRuntime()->schedDispatchable(new AFUReconfigured(m_pReconClient, TransactionID(puidEvent->msgTranID())));
                  }
                 return;
              }

           default:
              break;
           }

   }
   default:
      break;
   }

   CHWALIBase::AFUEvent(theEvent);
}


/// @} group HWALIAFU

END_NAMESPACE(AAL)
