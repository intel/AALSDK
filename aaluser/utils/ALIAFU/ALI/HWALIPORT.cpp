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
/// @file HWALIPORT.cpp
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


#include "ALIAIATransactions.h"
#include "HWALIPORT.h"

BEGIN_NAMESPACE(AAL)

/// @addtogroup ALI
/// @{

//
// ctor.HWALIPORT constructor.
//
CHWALIPORT::CHWALIPORT( IBase *pSvcClient,
                        IServiceBase *pServiceBase,
                        TransactionID transID,
                        IAFUProxy *pAFUProxy): CHWALIBase(pSvcClient,pServiceBase,transID,pAFUProxy)
{

}

//
// errorGet. reads Port errors
//
btBool CHWALIPORT::errorGet( INamedValueSet &rResult )
{

   struct CCIP_ERROR *pError                = NULL;
   struct CCIP_PORT_ERROR ccip_port_error   = {0};
   btWSSize size                            = sizeof(struct CCIP_ERROR);

   // Create the Transaction
   ErrorGet transaction(size,ccipdrv_getPortError);

   // Should never fail
   if ( !transaction.IsOK() ) {
      return  false;
   }

   // Send transaction
   m_pAFUProxy->SendTransaction(&transaction);
   if(transaction.getErrno() != uid_errnumOK) {
      return false;
   }

   if(NULL == transaction.getBuffer() )  {
      return false;
   }

   pError = (struct  CCIP_ERROR *)transaction.getBuffer();

   ccip_port_error.csr =  pError->error0;
   readPortError(ccip_port_error,rResult);

   return true;
}

//
// errorGetOrder. reads Port First errors
//
btBool CHWALIPORT::errorGetOrder( INamedValueSet &rResult )
{

   struct CCIP_ERROR *pError                = NULL;
   struct CCIP_PORT_ERROR ccip_port_error   = {0};
   btWSSize size                            = sizeof(struct CCIP_ERROR);

   // Create the Transaction
   ErrorGet transaction(size,ccipdrv_getPortError);

   // Should never fail
   if ( !transaction.IsOK() ) {
      return false;
   }

   // Send transaction
   m_pAFUProxy->SendTransaction(&transaction);
   if(transaction.getErrno() != uid_errnumOK) {
      return false;
   }

   if(NULL == transaction.getBuffer() )  {
      return false;
   }

   pError = (struct  CCIP_ERROR *)transaction.getBuffer();
   ccip_port_error.csr  = pError->first_error;

   readPortError(ccip_port_error,rResult);

   return true;
}

//
// errorMaskGet. reads Port errors mask
//
btBool CHWALIPORT::errorGetMask( INamedValueSet &rResult )
{
   struct CCIP_ERROR *pError               = NULL;
   struct CCIP_PORT_ERROR ccip_port_error_msk  = {0};
   btWSSize size                           = sizeof(struct CCIP_ERROR);

   // Create the Transaction
   ErrorGet transaction(size,ccipdrv_getPortError);

   // Should never fail
   if ( !transaction.IsOK() ) {
      return  false;
   }

   // Send transaction
   m_pAFUProxy->SendTransaction(&transaction);
   if(transaction.getErrno() != uid_errnumOK) {
      return false;
   }

   if(NULL == transaction.getBuffer() )  {
      return false;
   }

   pError = (struct  CCIP_ERROR *)transaction.getBuffer();


   ccip_port_error_msk.csr = pError->error0_mask;;
   readPortError(ccip_port_error_msk,rResult);

   return true;
}

//
// errorSetMask. sets port error mask.
//
btBool CHWALIPORT::errorSetMask( const INamedValueSet &rInputArgs )
{
   struct CCIP_PORT_ERROR cip_port_error_mask   = {0};
   struct CCIP_ERROR ccip_error                 = {0};

   writePortError(&cip_port_error_mask,rInputArgs);

   ccip_error.error0_mask = cip_port_error_mask.csr;

   // Create the Transaction
   SetError transaction(ccipdrv_SetPortErrorMask,ccip_error);

   // Should never fail
   if ( !transaction.IsOK() ) {
      return  false;
   }

   // Send transaction
   m_pAFUProxy->SendTransaction(&transaction);
   if(transaction.getErrno() != uid_errnumOK) {
      return false;
   }

   return true;
}


//
// errorClear. clears port errors.
//
btBool CHWALIPORT::errorClear(const INamedValueSet &rInputArgs )
{
   struct CCIP_PORT_ERROR port_error = {0};
   struct CCIP_ERROR ccip_error      = {0};

   writePortError(&port_error,rInputArgs);

   ccip_error.error0 = port_error.csr;

   // Create the Transaction
   SetError transaction(ccipdrv_ClearPortError,ccip_error);

   // Should never fail
   if ( !transaction.IsOK() ) {
      return  false;
   }

   // Send transaction
   m_pAFUProxy->SendTransaction(&transaction);
   if(transaction.getErrno() != uid_errnumOK) {
      return false;
   }

   return true;
}

//
// errorClearAll. clears all port errors
//
btBool CHWALIPORT::errorClearAll()
{
   struct CCIP_ERROR ccip_error      = {0};
   // Create the Transaction
   SetError transaction(ccipdrv_ClearAllPortErrors,ccip_error);

   // Should never fail
   if ( !transaction.IsOK() ) {
    return  false;
   }

   // Send transaction
   m_pAFUProxy->SendTransaction(&transaction);
   if(transaction.getErrno() != uid_errnumOK) {
    return false;
   }

   return true;
}

//
// errorGetPortMalformedReq. get port malormed request.
//
btBool CHWALIPORT::errorGetPortMalformedReq( INamedValueSet &rResult )
{
   struct CCIP_ERROR *pError                = NULL;
   struct CCIP_PORT_ERROR  ccip_port_error  = {0};
   btWSSize size                            = sizeof(struct CCIP_ERROR);

   // Create the Transaction
   ErrorGet transaction(size,ccipdrv_getPortError);

   // Should never fail
   if ( !transaction.IsOK() ) {
      return  false;
   }

   // Send transaction
   m_pAFUProxy->SendTransaction(&transaction);
   if(transaction.getErrno() != uid_errnumOK) {
      return false;
   }

   if(NULL == transaction.getBuffer() )  {
      return false;
   }

   pError = (struct  CCIP_ERROR *)transaction.getBuffer();

   if(pError->malreq0  !=0) {
      rResult.Add(AAL_ERR_PORT_MALFORMED_REQ_0,pError->malreq0);
   }

   if(pError->malreq1  !=0) {
      rResult.Add(AAL_ERR_PORT_MALFORMED_REQ_0,pError->malreq1);
   }

   return true;
}

//
// printAllErrors. Prints all errors.
//
btBool CHWALIPORT::printAllErrors()
{

   struct CCIP_ERROR *pError    =  NULL;
   btWSSize size                = sizeof(struct CCIP_ERROR);

   // Create the Transaction
   ErrorGet transaction(size,ccipdrv_getPortError);

   // Should never fail
   if ( !transaction.IsOK() ) {
      return  false;
   }

   // Send transaction
   m_pAFUProxy->SendTransaction(&transaction);
   if(transaction.getErrno() != uid_errnumOK) {
      return false;
   }

   if(NULL == transaction.getBuffer() )  {
      return false;
   }

   pError = (struct  CCIP_ERROR *)transaction.getBuffer();

   pirntPortErrors(pError);

   return true;

}

//
// pirntPortErrors. Prints all port errors.
//
void CHWALIPORT::pirntPortErrors(struct CCIP_ERROR *pError)
{
   btUnsignedInt count;
   NamedValueSet portErrornvs;
   struct CCIP_PORT_ERROR ccip_port_error   = {0};

   // Port Error
   portErrornvs.Empty();
   ccip_port_error.csr =  pError->error0;
   readPortError(ccip_port_error,portErrornvs);
   portErrornvs.GetNumNames(&count);

   for(int i=0;i<count ;i++) {

      btStringKey type;
      portErrornvs.GetName(i,&type);
      std::cout << " PORT Error: " << type <<"  Set"<< std::endl;
   }

   // Port Error Mask
   portErrornvs.Empty();
   ccip_port_error.csr =  pError->error0_mask;
   readPortError(ccip_port_error,portErrornvs);
   portErrornvs.GetNumNames(&count);

   for(int i=0;i<count ;i++) {
      btStringKey type;
      portErrornvs.GetName(i,&type);
      std::cout << " PORT Mask: " << type <<"  Set"<< std::endl;
   }

   // Port Firt Error
   portErrornvs.Empty();
   ccip_port_error.csr =  pError->first_error;
   readPortError(ccip_port_error,portErrornvs);
   portErrornvs.GetNumNames(&count);

   for(int i=0;i<count ;i++) {
      btStringKey type;
      portErrornvs.GetName(i,&type);
      std::cout << " PORT First: " << type <<"  Set"<< std::endl;
   }

   // Malformed Request count
   if(0x0 != pError->malreq0) {
      std::cout << "Port malformed request0 Error CSR:0x"<< std::hex << pError->malreq0 << std::endl;
   }

   if(0x0 != pError->malreq1) {
      std::cout << "Port malformed request0 Error CSR:0x"<< std::hex << pError->malreq1 << std::endl;
   }

}
//
// AFUEvent,AFU Event Handler.
//
void CHWALIPORT::AFUEvent(AAL::IEvent const &theEvent)
{
   CHWALIBase::AFUEvent(theEvent);
}

//
// readPortError. adds port errors to Name value Set
//
void CHWALIPORT::readPortError( struct CCIP_PORT_ERROR port_error,
                                INamedValueSet &rResult )
{

   if(port_error.tx_ch0_overflow) {
     rResult.Add(AAL_ERR_PORT_TX_CH0_OVERFLOW,true);
   }

   if(port_error.tx_ch0_invalidreq) {
     rResult.Add(AAL_ERR_PORT_TX_CH0_INVALIDREQ,true);
   }

   if(port_error.tx_ch0_req_cl_len3) {
      rResult.Add(AAL_ERR_PORT_TX_CH0_REQ_CL_LEN3,true);
   }

   if(port_error.tx_ch0_req_cl_len2) {
     rResult.Add(AAL_ERR_PORT_TX_CH0_REQ_CL_LEN2,true);
   }

   if(port_error.tx_ch0_req_cl_len4) {
      rResult.Add(AAL_ERR_PORT_TX_CH0_REQ_CL_LEN4,true);
   }

   if(port_error.tx_ch1_overflow) {
      rResult.Add(AAL_ERR_PORT_TX_CH1_OVERFLOW,true);
   }

   if(port_error.tx_ch1_invalidreq) {
      rResult.Add(AAL_ERR_PORT_TX_CH1_INVALIDREQ,true);
   }

   if(port_error.tx_ch1_req_cl_len3) {
      rResult.Add(AAL_ERR_PORT_TX_CH1_REQ_CL_LEN3,true);
   }

   if(port_error.tx_ch1_req_cl_len2) {
      rResult.Add(AAL_ERR_PORT_TX_CH1_REQ_CL_LEN2,true);
   }

   if(port_error.tx_ch1_req_cl_len4) {
      rResult.Add(AAL_ERR_PORT_TX_CH1_REQ_CL_LEN4,true);
   }

   if(port_error.tx_ch1_insuff_datapayload) {
      rResult.Add(AAL_ERR_PORT_TX_CH1_INSUFF_DATAPYL,true);
   }

   if(port_error.tx_ch1_datapayload_overrun) {
      rResult.Add(AAL_ERR_PORT_TX_CH1_DATAPYL_OVERRUN,true);
   }

   if(port_error.tx_ch1_incorr_addr) {
      rResult.Add(AAL_ERR_PORT_TX_CH1_INCORR_ADDR,true);
   }

   if(port_error.tx_ch1_sop_detcted) {
      rResult.Add(AAL_ERR_PORT_TX_CH1_SOP_DETECTED,true);
   }

   if(port_error.tx_ch1_atomic_req) {
      rResult.Add(AAL_ERR_PORT_TX_CH1_ATOMIC_REQ,true);
   }

   if(port_error.mmioread_timeout) {
      rResult.Add(AAL_ERR_PORT_MMIOREAD_TIMEOUT,true);
   }

   if(port_error.tx_ch2_fifo_overflow) {
      rResult.Add(AAL_ERR_PORT_TX_CH2_FIFO_OVERFLOW,true);
   }

   if(port_error.num_pending_req_overflow) {
      rResult.Add(AAL_ERR_PORT_NUM_PENDREQ_OVERFLOW,true);
   }
}

//
// errorMaskGet. reads Port errors from NVS and write to PORT CSR
//
void CHWALIPORT::writePortError(struct CCIP_PORT_ERROR *pPort_Error,const INamedValueSet &rInputArgs)
{

   if(rInputArgs.Has(AAL_ERR_PORT_TX_CH0_OVERFLOW)) {
      pPort_Error->tx_ch0_overflow =0x1;
   }

   if(rInputArgs.Has(AAL_ERR_PORT_TX_CH0_INVALIDREQ)) {
      pPort_Error->tx_ch0_invalidreq =0x1;
   }

   if(rInputArgs.Has(AAL_ERR_PORT_TX_CH0_REQ_CL_LEN3)) {
       pPort_Error->tx_ch0_req_cl_len3 =0x1;
    }

   if(rInputArgs.Has(AAL_ERR_PORT_TX_CH0_REQ_CL_LEN2)) {
       pPort_Error->tx_ch0_req_cl_len2=0x1;
    }

   if(rInputArgs.Has(AAL_ERR_PORT_TX_CH0_REQ_CL_LEN4)) {
       pPort_Error->tx_ch0_req_cl_len4=0x1;
    }

   if(rInputArgs.Has(AAL_ERR_PORT_TX_CH1_OVERFLOW)) {
       pPort_Error->tx_ch1_overflow=0x1;
    }

   if(rInputArgs.Has(AAL_ERR_PORT_TX_CH1_REQ_CL_LEN3)) {
       pPort_Error->tx_ch1_req_cl_len3=0x1;
    }

   if(rInputArgs.Has(AAL_ERR_PORT_TX_CH1_REQ_CL_LEN2)) {
       pPort_Error->tx_ch1_req_cl_len2=0x1;
    }

   if(rInputArgs.Has(AAL_ERR_PORT_TX_CH1_REQ_CL_LEN4)) {
       pPort_Error->tx_ch1_req_cl_len4=0x1;
    }

   if(rInputArgs.Has(AAL_ERR_PORT_TX_CH1_DATAPYL_OVERRUN)) {
       pPort_Error->tx_ch1_datapayload_overrun=0x1;
    }

   if(rInputArgs.Has(AAL_ERR_PORT_TX_CH1_INCORR_ADDR)) {
       pPort_Error->tx_ch1_incorr_addr=0x1;
    }

   if(rInputArgs.Has(AAL_ERR_PORT_TX_CH1_SOP_DETECTED)) {
       pPort_Error->tx_ch1_sop_detcted=0x1;
    }

   if(rInputArgs.Has(AAL_ERR_PORT_TX_CH1_ATOMIC_REQ)) {
       pPort_Error->tx_ch1_atomic_req=0x1;
    }

   if(rInputArgs.Has(AAL_ERR_PORT_MMIOREAD_TIMEOUT)) {
       pPort_Error->mmioread_timeout=0x1;
    }

   if(rInputArgs.Has(AAL_ERR_PORT_TX_CH2_FIFO_OVERFLOW)) {
       pPort_Error->tx_ch2_fifo_overflow=0x1;
    }

   if(rInputArgs.Has(AAL_ERR_PORT_NUM_PENDREQ_OVERFLOW)) {
       pPort_Error->num_pending_req_overflow=0x1;
    }

}

/// @} group HWALIAFU

END_NAMESPACE(AAL)
