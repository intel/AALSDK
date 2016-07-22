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
/// @file HWALFME.cpp
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
#include "HWALIFME.h"

#define FME_FIRST_ERR_STR "First "
#define FME_NEXT_ERR_STR  "Next "

BEGIN_NAMESPACE(AAL)

/// @addtogroup HWALIAFU
/// @{

//
// ctor. CHWALIFME class constructor
//
CHWALIFME::CHWALIFME( IBase *pSvcClient,
                      IServiceBase *pServiceBase,
                      TransactionID transID,
                      IAFUProxy *pAFUProxy): CHWALIBase(pSvcClient,pServiceBase,transID,pAFUProxy)
{

}

//
// performanceCountersGet. Returns the Performance Counter Values
//
btBool CHWALIFME::performanceCountersGet ( INamedValueSet*  const  pResult,
                                           NamedValueSet    const &pOptArgs )
{
   btWSSize size                     = sizeof(struct  CCIP_PERF_COUNTERS);
   struct  CCIP_PERF_COUNTERS *pPref = NULL;

   if( NULL == pResult) {
     return  false;
   }

   // Create the Transaction
   PerfCounterGet transaction(size);

   // Should never fail
   if ( !transaction.IsOK() ) {
     return  false;
   }

   // Send transaction
   // Will eventually trigger AFUEvent(), below.
   m_pAFUProxy->SendTransaction(&transaction);

   if(transaction.getErrno() != uid_errnumOK){
       return false;
     }

   if(NULL == transaction.getBuffer() )  {
    return false;
   }

   pPref = (struct  CCIP_PERF_COUNTERS *)transaction.getBuffer();

   pResult->Add(pPref->version.name,pPref->version.value);
   pResult->Add(pPref->num_counters.name,pPref->num_counters.value);

   pResult->Add(pPref->read_hit.name,pPref->read_hit.value);
   pResult->Add(pPref->write_hit.name,pPref->write_hit.value);
   pResult->Add(pPref->read_miss.name,pPref->read_miss.value);
   pResult->Add(pPref->write_miss.name,pPref->write_miss.value);
   pResult->Add(pPref->evictions.name,pPref->evictions.value);

   pResult->Add(pPref->pcie0_read.name,pPref->pcie0_read.value);
   pResult->Add(pPref->pcie0_write.name,pPref->pcie0_write.value);
   pResult->Add(pPref->pcie1_read.name,pPref->pcie1_read.value);
   pResult->Add(pPref->pcie1_write.name,pPref->pcie1_write.value);
   pResult->Add(pPref->upi_read.name,pPref->upi_read.value);
   pResult->Add(pPref->upi_write.name,pPref->upi_write.value);

   return true;
}


//
// errorGet. Returns the FME Errors
//
btBool CHWALIFME::errorGet( INamedValueSet &rResult )
{

   struct CCIP_ERROR *pError    = NULL;
   btWSSize size                = sizeof(struct CCIP_ERROR);

   // Create the Transaction
   ErrorGet transaction(size,ccipdrv_getFMEError);

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

   readfmeError(pError,rResult );

   return true;

}

void CHWALIFME::readfmeError( struct CCIP_ERROR *pError, INamedValueSet &rResult,btBool errMask  )
{
   struct CCIP_FME_ERROR0  fme_error0              = {0};
   struct CCIP_FME_PCIE0_ERROR pcie0_error         = {0};
   struct CCIP_FME_PCIE1_ERROR pcie1_error         = {0};
   struct CCIP_FME_RAS_GERROR  ras_gerr            = {0};
   struct CCIP_FME_RAS_BERROR ras_berror           = {0};
   struct CCIP_FME_RAS_WARNERROR ras_warnerror     = {0};

   if(false == errMask) {
      fme_error0.csr       = pError->error0;
      pcie0_error.csr      = pError->pcie0_error;
      pcie1_error.csr      = pError->pcie1_error;
      ras_gerr.csr         = pError->ras_gerr;
      ras_berror.csr       = pError->ras_berror;
      ras_warnerror.csr    = pError->ras_warnerror;
   } else  {

      fme_error0.csr       = pError->error0_mask;
      pcie0_error.csr      = pError->pcie0_error_mask;
      pcie1_error.csr      = pError->pcie1_error_mask;
      ras_gerr.csr         = pError->ras_gerr_mask;
      ras_berror.csr       = pError->ras_berror_mask;
      ras_warnerror.csr    = pError->ras_warnerror_mask;
   }


   if(fme_error0.fabFifo_underflow) {
      rResult.Add(AAL_ERR_FME_FAB_UNDERFLOW,true);
   }

   if(fme_error0.fabFifo_overflow) {
      rResult.Add(AAL_ERR_FME_FAB_OVERFLOW,true);
   }

   if(fme_error0.poison_detected) {
      rResult.Add(AAL_ERR_FME_POSION_DETECT,true);
   }

   if(fme_error0.parity_error) {
        rResult.Add(AAL_ERR_FME_PARIRY,true);
   }

   if(pcie0_error.formattype_err) {
        rResult.Add(AAL_ERR_PCIE0_FORMAT,true);
   }

   if(pcie0_error.MWAddr_err) {
        rResult.Add(AAL_ERR_PCIE0_MWADDR,true);
   }

   if(pcie0_error.MWAddrLength_err) {
        rResult.Add(AAL_ERR_PCIE0_MWLEN,true);
   }

   if(pcie0_error.MRAddr_err) {
        rResult.Add(AAL_ERR_PCIE0_MRADDR,true);
   }

   if(pcie0_error.MRAddrLength_err) {
        rResult.Add(AAL_ERR_PCIE0_MRLEN,true);
   }

   if(pcie0_error.cpl_tag_err) {
      rResult.Add(AAL_ERR_PCIE0_COMPTAG,true);
   }

   if(pcie0_error.cpl_status_err) {
      rResult.Add(AAL_ERR_PCIE0_COMPSTAT,true);
   }

   if(pcie0_error.cpl_timeout_err) {
      rResult.Add(AAL_ERR_PCIE0_TIMEOUT,true);
   }

   if(pcie1_error.formattype_err) {
      rResult.Add(AAL_ERR_PCIE1_FORMAT,true);
   }

   if(pcie1_error.MWAddr_err) {
      rResult.Add(AAL_ERR_PCIE1_MWADDR,true);
   }

   if(pcie1_error.MWAddrLength_err) {
      rResult.Add(AAL_ERR_PCIE1_MWLEN,true);
   }

   if(pcie1_error.MRAddr_err) {
      rResult.Add(AAL_ERR_PCIE1_MRADDR,true);
   }

   if(pcie1_error.MRAddrLength_err) {
      rResult.Add(AAL_ERR_PCIE1_MRLEN,true);
   }

   if(pcie1_error.cpl_tag_err) {
      rResult.Add(AAL_ERR_PCIE1_COMPTAG,true);
   }

   if(pcie1_error.cpl_status_err) {
      rResult.Add(AAL_ERR_PCIE1_COMPSTAT,true);
   }

   if(pcie1_error.cpl_timeout_err) {
      rResult.Add(AAL_ERR_PCIE1_TIMEOUT,true);
   }

   if(ras_gerr.therm_warn0) {
      rResult.Add(AAL_ERR_RAS_THERMWARN0,true);
   }

   if(ras_gerr.therm_warn1) {
      rResult.Add(AAL_ERR_RAS_THERMWARN1,true);
   }

   if(ras_gerr.pcie_error) {
      rResult.Add(AAL_ERR_RAS_PCIE,true);
   }

   if(ras_gerr.afufatal_error) {
      rResult.Add(AAL_ERR_RAS_AFUFATAL,true);
   }

   if(ras_gerr.gb_crc_err) {
      rResult.Add(AAL_ERR_RAS_GBCRC,true);
   }

   if(ras_berror.ktilink_fatal_err) {
      rResult.Add(AAL_ERR_RAS_GBCRC,true);
   }

   if(ras_berror.tagcch_fatal_err) {
      rResult.Add(AAL_ERR_RAS_TAGCCH_FATAL,true);
   }

   if(ras_berror.cci_fatal_err) {
      rResult.Add(AAL_ERR_RAS_CCI_FATAL,true);
   }

   if(ras_berror.ktiprpto_fatal_err) {
      rResult.Add(AAL_ERR_RAS_KTIPROTO_FATAL,true);
   }

   if(ras_berror.dma_fatal_err) {
      rResult.Add(AAL_ERR_RAS_DMA_FATAL,true);
   }

   if(ras_berror.iommu_fatal_err) {
      rResult.Add(AAL_ERR_RAS_IOMMU_FATAL,true);
   }

   if(ras_berror.iommu_catast_err) {
      rResult.Add(AAL_ERR_RAS_IOMMU_CATAS,true);
   }

   if(ras_berror.crc_catast_err) {
      rResult.Add(AAL_ERR_RAS_CRC_CATAS,true);
   }

   if(ras_berror.therm_catast_err) {
      rResult.Add(AAL_ERR_RAS_THER_CATAS,true);
   }

   if(ras_warnerror.event_warn_err) {
      rResult.Add(AAL_ERR_RAS_GB_FATAL,true);
   }

}
//
// errorGetFirst. Returns the FME First Errors
//
btBool CHWALIFME::errorGetOrder( INamedValueSet &rResult )
{
   struct CCIP_ERROR *pError                = NULL;
   btWSSize size                            = sizeof(struct CCIP_ERROR);

   // Create the Transaction
   ErrorGet transaction(size,ccipdrv_getFMEError);

   // Should never fail
   if ( !transaction.IsOK() ) {
     return  false;
   }

   // Send transaction
   m_pAFUProxy->SendTransaction(&transaction);
   if(transaction.getErrno() != uid_errnumOK) {
     return false;
   }

   if(NULL == transaction.getBuffer() ) {
     return false;
   }

   pError = (struct  CCIP_ERROR *)transaction.getBuffer();

   readOrderError(pError,rResult);

    return true;
}
void CHWALIFME::readOrderError( struct CCIP_ERROR *pError, INamedValueSet &rResult)
{
   struct CCIP_FME_ERROR0  fme_error0              = {0};
   struct CCIP_FME_PCIE0_ERROR pcie0_error         = {0};
   struct CCIP_FME_PCIE1_ERROR pcie1_error         = {0};

   struct CCIP_FME_FIRST_ERROR fme_first_err       = {0};
   struct CCIP_FME_NEXT_ERROR fme_next_err         = {0};


   fme_first_err.csr    = pError->first_error;
   fme_next_err.csr     = pError->next_error;

   if(0x0 == fme_first_err.errReg_id )
   {
      fme_error0.csr    = pError->first_error;

      if(fme_error0.fabFifo_underflow) {
          std::string str(FME_FIRST_ERR_STR);
          rResult.Add(str.append(AAL_ERR_FME_FAB_UNDERFLOW).c_str(),true);
      }

      if(fme_error0.fabFifo_overflow) {
         std::string str(FME_FIRST_ERR_STR);
         rResult.Add(str.append(AAL_ERR_FME_FAB_OVERFLOW).c_str(),true);
      }

      if(fme_error0.poison_detected) {
         std::string str(FME_FIRST_ERR_STR);
         rResult.Add(str.append(AAL_ERR_FME_POSION_DETECT).c_str(),true);
      }

      if(fme_error0.parity_error) {
           std::string str(FME_FIRST_ERR_STR);
           rResult.Add(str.append(AAL_ERR_FME_PARIRY).c_str(),true);
      }

   }

   if(0x0 == fme_next_err.errReg_id ) {

      fme_error0.csr    = pError->next_error;

      if(fme_error0.fabFifo_underflow) {
         std::string str(FME_NEXT_ERR_STR);
         rResult.Add(str.append(AAL_ERR_FME_FAB_UNDERFLOW).c_str(),true);
      }

      if(fme_error0.fabFifo_overflow) {
         std::string str(FME_NEXT_ERR_STR);
         rResult.Add(str.append(AAL_ERR_FME_FAB_OVERFLOW).c_str(),true);
      }

      if(fme_error0.poison_detected) {
         std::string str(FME_NEXT_ERR_STR);
         rResult.Add(str.append(AAL_ERR_FME_POSION_DETECT).c_str(),true);
      }

      if(fme_error0.parity_error) {
           std::string str(FME_NEXT_ERR_STR);
           rResult.Add(str.append(AAL_ERR_FME_PARIRY).c_str(),true);
      }
   }

   if(0x1 == fme_first_err.errReg_id ) {

      pcie0_error.csr    = pError->first_error;

      if(pcie0_error.formattype_err) {
         std::string str(FME_FIRST_ERR_STR);
         rResult.Add(str.append(AAL_ERR_PCIE0_FORMAT).c_str(),true);
      }

      if(pcie0_error.MWAddr_err) {
         std::string str(FME_FIRST_ERR_STR);
         rResult.Add(str.append(AAL_ERR_PCIE0_MWADDR).c_str(),true);
      }

      if(pcie0_error.MWAddrLength_err) {
         std::string str(FME_FIRST_ERR_STR);
         rResult.Add(str.append(AAL_ERR_PCIE0_MWLEN).c_str(),true);
      }

      if(pcie0_error.MRAddr_err) {
         std::string str(FME_FIRST_ERR_STR);
         rResult.Add(str.append(AAL_ERR_PCIE0_MRADDR).c_str(),true);
      }

      if(pcie0_error.MRAddrLength_err) {
         std::string str(FME_FIRST_ERR_STR);
         rResult.Add(str.append(AAL_ERR_PCIE0_MRLEN).c_str(),true);
      }

      if(pcie0_error.cpl_tag_err) {
         std::string str(FME_FIRST_ERR_STR);
         rResult.Add(str.append(AAL_ERR_PCIE0_COMPTAG).c_str(),true);
      }

      if(pcie0_error.cpl_status_err) {
         std::string str(FME_FIRST_ERR_STR);
         rResult.Add(str.append(AAL_ERR_PCIE0_COMPSTAT).c_str(),true);
      }

      if(pcie0_error.cpl_timeout_err) {
         std::string str(FME_FIRST_ERR_STR);
         rResult.Add(str.append(AAL_ERR_PCIE0_TIMEOUT).c_str(),true);
      }
   }

   if(0x1 == fme_next_err.errReg_id ) {

      pcie0_error.csr    = pError->next_error;

      if(pcie0_error.formattype_err) {
         std::string str(FME_NEXT_ERR_STR);
         rResult.Add(str.append(AAL_ERR_PCIE0_FORMAT).c_str(),true);
      }

      if(pcie0_error.MWAddr_err) {
         std::string str(FME_NEXT_ERR_STR);
         rResult.Add(str.append(AAL_ERR_PCIE0_MWADDR).c_str(),true);
      }

      if(pcie0_error.MWAddrLength_err) {
         std::string str(FME_NEXT_ERR_STR);
         rResult.Add(str.append(AAL_ERR_PCIE0_MWLEN).c_str(),true);
      }

      if(pcie0_error.MRAddr_err) {
         std::string str(FME_NEXT_ERR_STR);
         rResult.Add(str.append(AAL_ERR_PCIE0_MRADDR).c_str(),true);
      }

      if(pcie0_error.MRAddrLength_err) {
         std::string str(FME_NEXT_ERR_STR);
         rResult.Add(str.append(AAL_ERR_PCIE0_MRLEN).c_str(),true);
      }

      if(pcie0_error.cpl_tag_err) {
         std::string str(FME_NEXT_ERR_STR);
         rResult.Add(str.append(AAL_ERR_PCIE0_COMPTAG).c_str(),true);
      }

      if(pcie0_error.cpl_status_err) {
         std::string str(FME_NEXT_ERR_STR);
         rResult.Add(str.append(AAL_ERR_PCIE0_COMPSTAT).c_str(),true);
      }

      if(pcie0_error.cpl_timeout_err) {
         std::string str(FME_NEXT_ERR_STR);
         rResult.Add(str.append(AAL_ERR_PCIE0_TIMEOUT).c_str(),true);
      }
   }

   if(0x2 == fme_first_err.errReg_id ) {

      pcie1_error.csr    = pError->first_error;

      if(pcie1_error.formattype_err) {
         std::string str(FME_FIRST_ERR_STR);
         rResult.Add(str.append(AAL_ERR_PCIE1_FORMAT).c_str(),true);
      }

      if(pcie1_error.MWAddr_err) {
         std::string str(FME_FIRST_ERR_STR);
         rResult.Add(str.append(AAL_ERR_PCIE1_MWADDR).c_str(),true);
      }

      if(pcie1_error.MWAddrLength_err) {
         std::string str(FME_FIRST_ERR_STR);
         rResult.Add(str.append(AAL_ERR_PCIE1_MWLEN).c_str(),true);
      }

      if(pcie1_error.MRAddr_err) {
         std::string str(FME_FIRST_ERR_STR);
         rResult.Add(str.append(AAL_ERR_PCIE1_MRADDR).c_str(),true);
      }

      if(pcie1_error.MRAddrLength_err) {
         std::string str(FME_FIRST_ERR_STR);
         rResult.Add(str.append(AAL_ERR_PCIE1_MRLEN).c_str(),true);
      }

      if(pcie1_error.cpl_tag_err) {
         std::string str(FME_FIRST_ERR_STR);
         rResult.Add(str.append(AAL_ERR_PCIE1_COMPTAG).c_str(),true);
      }

      if(pcie1_error.cpl_status_err) {
         std::string str(FME_FIRST_ERR_STR);
         rResult.Add(str.append(AAL_ERR_PCIE1_COMPSTAT).c_str(),true);
      }

      if(pcie1_error.cpl_timeout_err) {
         std::string str(FME_FIRST_ERR_STR);
         rResult.Add(str.append(AAL_ERR_PCIE1_TIMEOUT).c_str(),true);
      }
   }

   if(0x2 == fme_next_err.errReg_id ) {

      pcie1_error.csr    = pError->next_error;

      if(pcie1_error.formattype_err) {
         std::string str(FME_NEXT_ERR_STR);
         rResult.Add(str.append(AAL_ERR_PCIE1_FORMAT).c_str(),true);
      }

      if(pcie1_error.MWAddr_err) {
         std::string str(FME_NEXT_ERR_STR);
         rResult.Add(str.append(AAL_ERR_PCIE1_MWADDR).c_str(),true);
      }

      if(pcie1_error.MWAddrLength_err) {
         std::string str(FME_NEXT_ERR_STR);
         rResult.Add(str.append(AAL_ERR_PCIE1_MWLEN).c_str(),true);
      }

      if(pcie1_error.MRAddr_err) {
         std::string str(FME_NEXT_ERR_STR);
         rResult.Add(str.append(AAL_ERR_PCIE1_MRADDR).c_str(),true);
      }

      if(pcie1_error.MRAddrLength_err) {
         std::string str(FME_NEXT_ERR_STR);
         rResult.Add(str.append(AAL_ERR_PCIE1_MRLEN).c_str(),true);
      }

      if(pcie1_error.cpl_tag_err) {
         std::string str(FME_NEXT_ERR_STR);
         rResult.Add(str.append(AAL_ERR_PCIE1_COMPTAG).c_str(),true);
      }

      if(pcie1_error.cpl_status_err) {
         std::string str(FME_NEXT_ERR_STR);
         rResult.Add(str.append(AAL_ERR_PCIE1_COMPSTAT).c_str(),true);
      }

      if(pcie1_error.cpl_timeout_err) {
         std::string str(FME_NEXT_ERR_STR);
         rResult.Add(str.append(AAL_ERR_PCIE1_TIMEOUT).c_str(),true);
      }
   }

}

//
// errorGetMask. Returns the FME  Errors Masks.
//
btBool CHWALIFME::errorGetMask( INamedValueSet &rResult )
{
   struct CCIP_ERROR *pError                      = NULL;

   // Create the Transaction
   ErrorGet transaction(sizeof( struct CCIP_ERROR),ccipdrv_getFMEError);

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

   readfmeError(pError,rResult ,true);

   return true;
}


//
// errorSetMask, Sets FME Error mask
//
btBool CHWALIFME::errorSetMask( const INamedValueSet &rInputArgs )
{
   struct CCIP_ERROR ccip_error      = {0};

   writewriteError(&ccip_error,rInputArgs,true);

   // Create the Transaction
   SetError transaction(ccipdrv_SetFMEErrorMask,ccip_error);

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
// errorClear, Clears Errors
//
btBool CHWALIFME::errorClear(const INamedValueSet &rInputArgs )
{
   struct CCIP_ERROR ccip_error  = {0};

   writewriteError(&ccip_error,rInputArgs,false);

   // Create the Transaction
   SetError transaction(ccipdrv_ClearFMEError,ccip_error);

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

void CHWALIFME::writewriteError( struct CCIP_ERROR *pError,const INamedValueSet &rInputArgs,btBool errMask)
{

   struct CCIP_FME_ERROR0  fme_error0              = {0};
   struct CCIP_FME_PCIE0_ERROR pcie0_error         = {0};
   struct CCIP_FME_PCIE1_ERROR pcie1_error         = {0};
   struct CCIP_FME_RAS_GERROR  ras_gerr            = {0};
   struct CCIP_FME_RAS_BERROR ras_berror           = {0};
   struct CCIP_FME_RAS_WARNERROR ras_warnerror     = {0};


   if(rInputArgs.Has(AAL_ERR_FME_FAB_UNDERFLOW)) {
      fme_error0.fabFifo_underflow = 0x1;
   }

   if(rInputArgs.Has(AAL_ERR_FME_FAB_OVERFLOW)) {
      fme_error0.fabFifo_overflow = 0x1;
   }

   if(rInputArgs.Has(AAL_ERR_FME_POSION_DETECT)) {
      fme_error0.poison_detected = 0x1;
   }

   if(rInputArgs.Has(AAL_ERR_FME_PARIRY)) {
      fme_error0.parity_error = 0x1;
   }

   if(rInputArgs.Has(AAL_ERR_PCIE0_FORMAT)) {
      pcie0_error.formattype_err = 0x1;
   }

   if(rInputArgs.Has(AAL_ERR_PCIE0_MWADDR)) {
      pcie0_error.MWAddr_err = 0x1;
   }

   if(rInputArgs.Has(AAL_ERR_PCIE0_MWLEN)) {
      pcie0_error.MWAddrLength_err = 0x1;
   }

   if(rInputArgs.Has(AAL_ERR_PCIE0_MRADDR)) {
      pcie0_error.MRAddr_err = 0x1;
   }

   if(rInputArgs.Has(AAL_ERR_PCIE0_MRLEN)) {
      pcie0_error.MRAddrLength_err = 0x1;
   }

   if(rInputArgs.Has(AAL_ERR_PCIE0_COMPTAG)) {
      pcie0_error.cpl_tag_err = 0x1;
   }

   if(rInputArgs.Has(AAL_ERR_PCIE0_COMPSTAT)) {
      pcie0_error.cpl_status_err = 0x1;
   }

   if(rInputArgs.Has(AAL_ERR_PCIE0_TIMEOUT)) {
      pcie0_error.cpl_timeout_err = 0x1;
   }

   if(rInputArgs.Has(AAL_ERR_PCIE1_FORMAT)) {
      pcie1_error.formattype_err = 0x1;
   }

   if(rInputArgs.Has(AAL_ERR_PCIE1_MWADDR)) {
      pcie1_error.MWAddr_err = 0x1;
   }

   if(rInputArgs.Has(AAL_ERR_PCIE1_MWLEN)) {
      pcie1_error.MWAddrLength_err = 0x1;
   }

   if(rInputArgs.Has(AAL_ERR_PCIE1_MWADDR)) {
      pcie1_error.MRAddr_err = 0x1;
   }

   if(rInputArgs.Has(AAL_ERR_PCIE1_MRLEN)) {
      pcie1_error.MRAddrLength_err = 0x1;
   }

   if(rInputArgs.Has(AAL_ERR_PCIE1_COMPTAG)) {
      pcie1_error.cpl_tag_err = 0x1;
   }

   if(rInputArgs.Has(AAL_ERR_PCIE1_COMPSTAT)) {
      pcie1_error.cpl_status_err = 0x1;
   }

   if(rInputArgs.Has(AAL_ERR_PCIE1_TIMEOUT)) {
      pcie1_error.cpl_timeout_err = 0x1;
   }

   if(rInputArgs.Has(AAL_ERR_RAS_THERMWARN0)) {
      ras_gerr.therm_warn0 = 0x1;
   }

   if(rInputArgs.Has(AAL_ERR_RAS_THERMWARN1)) {
      ras_gerr.therm_warn1 = 0x1;
   }

   if(rInputArgs.Has(AAL_ERR_RAS_PCIE)) {
      ras_gerr.pcie_error = 0x1;
   }

   if(rInputArgs.Has(AAL_ERR_RAS_AFUFATAL)) {
      ras_gerr.afufatal_error = 0x1;
   }

   if(rInputArgs.Has(AAL_ERR_RAS_GBCRC)) {
      ras_gerr.gb_crc_err = 0x1;
   }

   if(rInputArgs.Has(AAL_ERR_RAS_GBCRC)) {
      ras_berror.ktilink_fatal_err = 0x1;
   }

   if(rInputArgs.Has(AAL_ERR_RAS_TAGCCH_FATAL)) {
      ras_berror.tagcch_fatal_err = 0x1;
   }

   if(rInputArgs.Has(AAL_ERR_RAS_CCI_FATAL)) {
      ras_berror.cci_fatal_err = 0x1;
   }

   if(rInputArgs.Has(AAL_ERR_RAS_KTIPROTO_FATAL)) {
      ras_berror.ktiprpto_fatal_err = 0x1;
   }

   if(rInputArgs.Has(AAL_ERR_RAS_DMA_FATAL)) {
      ras_berror.dma_fatal_err = 0x1;
   }

   if(rInputArgs.Has(AAL_ERR_RAS_IOMMU_FATAL)) {
      ras_berror.iommu_fatal_err = 0x1;
   }

   if(rInputArgs.Has(AAL_ERR_RAS_IOMMU_CATAS)) {
      ras_berror.iommu_catast_err = 0x1;
   }

   if(rInputArgs.Has(AAL_ERR_RAS_CRC_CATAS)) {
      ras_berror.crc_catast_err = 0x1;
   }

   if(rInputArgs.Has(AAL_ERR_RAS_THER_CATAS)) {
      ras_berror.therm_catast_err = 0x1;
   }

   if(rInputArgs.Has(AAL_ERR_RAS_GB_FATAL)) {
      ras_warnerror.event_warn_err = 0x1;
   }

   if(false == errMask) {
      pError->error0                 = fme_error0.csr;
      pError->pcie0_error            = pcie0_error.csr;
      pError->pcie1_error            = pcie1_error.csr;
      pError->ras_gerr               = ras_gerr.csr;
      pError->ras_berror             = ras_berror.csr;
      pError->ras_warnerror          = ras_warnerror.csr;
   } else {

      pError->error0_mask            = fme_error0.csr;
      pError->pcie0_error_mask       = pcie0_error.csr;
      pError->pcie1_error_mask       = pcie1_error.csr;
      pError->ras_gerr_mask          = ras_gerr.csr;
      pError->ras_berror_mask        = ras_berror.csr;
      pError->ras_warnerror_mask     = ras_warnerror.csr;
   }

}

//
// errorClearAll, Clears all Errors.
//
btBool CHWALIFME::errorClearAll()
{
   struct CCIP_ERROR ccip_error  = {0};

   // Create the Transaction
   SetError transaction(ccipdrv_ClearAllFMEErrors,ccip_error);

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
btBool CHWALIFME::printAllErrors()
{
   struct CCIP_ERROR *pError  = NULL;
   btWSSize size              = sizeof(struct CCIP_ERROR);

   // Create the Transaction
   ErrorGet transaction(size,ccipdrv_getFMEError);

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

   pirntFMEErrors(pError);

   return true;
}

void CHWALIFME::pirntFMEErrors(struct CCIP_ERROR *pError)
{
   NamedValueSet fmeErrornvs;
   btUnsignedInt count          = 0;

   // print CSR
   std::cout << " FME Error CSR 0x: "<< std::hex << pError->error0 << std::endl;
   std::cout << " FME Error Mask CSR 0x: "<<std::hex << pError->error0_mask << std::endl;

   std::cout << " PCIe0 Error CSR 0x: "<<std::hex << pError->pcie0_error << std::endl;
   std::cout << " PCIe0 Error Mask CSR 0x: "<<std::hex << pError->pcie0_error_mask << std::endl;

   std::cout << " PCIe1 Error CSR 0x: "<<std::hex << pError->pcie1_error << std::endl;
   std::cout << " PCIe1 Error Mask CSR 0x: "<<std::hex << pError->pcie1_error_mask << std::endl;

   std::cout << " First Error CSR 0x: "<<std::hex << pError->first_error << std::endl;
   std::cout << " Next Error Mask CSR 0x: "<<std::hex << pError->next_error << std::endl;

   std::cout << " RAS Green BS CSR 0x: "<<std::hex << pError->ras_gerr << std::endl;
   std::cout << " RAS Green BS Mask CSR 0x: "<<std::hex << pError->ras_gerr_mask << std::endl;

   std::cout << " RAS blue bitstream  CSR 0x: "<<std::hex << pError->ras_berror << std::endl;
   std::cout << " RAS blue bitstream  Mask CSR 0x: "<<std::hex << pError->ras_berror_mask << std::endl;

   std::cout << " RAS warning CSR 0x: "<<std::hex << pError->ras_warnerror << std::endl;
   std::cout << " RAS warning Mask CSR 0x: "<<std::hex << pError->ras_warnerror_mask << std::endl;

   // FME Error
   fmeErrornvs.Empty();
   readfmeError(pError,fmeErrornvs,false );
   fmeErrornvs.GetNumNames(&count);

   for(int i=0;i<count ;i++) {
      btStringKey type;
      fmeErrornvs.GetName(i,&type);
      std::cout << "Error: " << type <<"  Set"<< std::endl;
   }

   // FME Error Mask
   fmeErrornvs.Empty();
   readfmeError(pError,fmeErrornvs,true );
   fmeErrornvs.GetNumNames(&count);

   for(int i=0;i<count ;i++) {
      btStringKey type;
      fmeErrornvs.GetName(i,&type);
      std::cout << "Error Mask: " << type <<" Set"<< std::endl;
   }

   // First Error & Next Error
   fmeErrornvs.Empty();
   readOrderError(pError,fmeErrornvs);
   fmeErrornvs.GetNumNames(&count);

   for(int i=0;i<count ;i++) {
      btStringKey type;
      fmeErrornvs.GetName(i,&type);
      std::cout  << type <<" Set"<< std::endl;
   }

}

//
// thermalGetValues, returns FPGA thermal threshold values
//
btBool CHWALIFME::thermalGetValues( INamedValueSet &rResult)
{
   struct CCIP_THERMAL_PWR   *pthermal_pwr            = NULL;
   struct CCIP_TEMP_THRESHOLD  temp_threshold         = {0};
   struct CCIP_TEMP_RDSSENSOR_FMT1 temp_rdssensor_fm1 = {0};
   struct CCIP_TEMP_RDSSENSOR_FMT2 temp_rdssensor_fm2 = {0};
   btWSSize size                                      = sizeof(struct CCIP_THERMAL_PWR);

   // Create the Transaction
   ThermalPwrGet transaction(size,ccipdrv_gertThermal);

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

   pthermal_pwr = (struct  CCIP_THERMAL_PWR *)transaction.getBuffer();

   temp_threshold.csr = pthermal_pwr->tmp_threshold  ;

   if(temp_threshold.thshold1_status  ) {
      rResult.Add(AALTEMP_THRESHOLD1,temp_threshold.tmp_thshold1);
   }

   if(temp_threshold.thshold2_status  ) {
      rResult.Add(AALTEMP_THRESHOLD2,temp_threshold.tmp_thshold2);
   }

   rResult.Add(AALTEMP_THERM_TRIP,temp_threshold.therm_trip_thshold);

   if((temp_threshold.thshold1_status) &&
      (temp_threshold.thshold_policy == 0) ) {
         rResult.Add(AALTEMP_THSHLD_STATUS1_AP1,true);
   }

   if((temp_threshold.thshold1_status) &&
      (temp_threshold.thshold_policy == 1) ) {
         rResult.Add(AALTEMP_THSHLD_STATUS1_AP2,temp_threshold.tmp_thshold2);
   }

   if(temp_threshold.thshold2_status) {
      rResult.Add(AALTEMP_THSHLD_STATUS1_AP6,true);
    }

   temp_rdssensor_fm1.csr = pthermal_pwr->tmp_rdsensor1 ;
   temp_rdssensor_fm2.csr = pthermal_pwr->tmp_rdsensor2 ;

   if(temp_rdssensor_fm1.tmp_reading_valid ==1  ) {
      rResult.Add(AALTEMP_READING_SEQNUM,temp_rdssensor_fm1.tmp_reading_seq_num);
      rResult.Add(AALTEMP_FPGA_TEMP_SENSOR1,temp_rdssensor_fm1.tmp_reading);
   }
   /*
   if(temp_rdssensor_fm1.tmp_reading_valid ==1  ) {
      rResult.Add(AALTEMP_READING_SEQNUM,temp_rdssensor_fm1.tmp_reading_seq_num);
      rResult.Add(AALTEMP_FPGA_TEMP_SENSOR2,temp_rdssensor_fm1.tmp_reading);
   }
   */

   return true;
}

//
// powerGetValues, returns FPGA power consumed values
//
btBool CHWALIFME::powerGetValues(INamedValueSet &rResult )
{
   struct CCIP_THERMAL_PWR   *pthermal_pwr   = NULL;
   struct CCIP_PM_STATUS    pm_status        = {0};
   btWSSize size                             = sizeof(struct CCIP_THERMAL_PWR);

   // Create the Transaction
   ThermalPwrGet transaction(size,ccipdrv_getPower);

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

   pthermal_pwr = (struct  CCIP_THERMAL_PWR *)transaction.getBuffer();

   pm_status.csr = pthermal_pwr->pwr_status ;

   if(pm_status.pwr_consumed !=0 ) {
      rResult.Add(AALPOWER_CONSUMPTION,pm_status.pwr_consumed);
    }

   return true;
}

//
// AFUEvent,AFU Event Handler.
//
void CHWALIFME::AFUEvent(AAL::IEvent const &theEvent)
{
   CHWALIBase::AFUEvent(theEvent);
}

/// @} group HWALIAFU

END_NAMESPACE(AAL)
