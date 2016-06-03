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

   struct CCIP_ERROR *pError                = NULL;
   struct CCIP_FME_ERROR0  fme_error0         = {0};
   struct CCIP_FME_ERROR1  fme_error1         = {0};
   struct CCIP_FME_ERROR2  fme_error2         = {0};

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

   if(NULL == transaction.getBuffer() )  {
      return false;
   }

   pError = (struct  CCIP_ERROR *)transaction.getBuffer();
   fme_error0.csr = pError->error0;
   fme_error1.csr = pError->error1;
   fme_error2.csr = pError->error2;

   return true;

}


//
// errorGetFirst. Returns the FME First Errors
//
btBool CHWALIFME::errorGetOrder( INamedValueSet &rResult )
{
   struct CCIP_ERROR *pError                   = NULL;
   struct CCIP_FME_FIRST_ERROR fme_first_error = {0};
   struct CCIP_FME_NEXT_ERROR  fme_next_error  = {0};
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
   fme_first_error.csr = pError->first_error;
   fme_next_error.csr = pError->next_error;


    return true;
}


//
// errorGetMask. Returns the FME  Errors Masks.
//
btBool CHWALIFME::errorGetMask( INamedValueSet &rResult )
{
   struct CCIP_ERROR *pError                      = NULL;
   struct CCIP_FME_ERROR0  fme_error0_mask         = {0};
   struct CCIP_FME_ERROR1  fme_error1_mask         = {0};
   struct CCIP_FME_ERROR2  fme_error2_mask         = {0};

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
   fme_error0_mask.csr = pError->error0_mask;
   fme_error1_mask.csr = pError->error1_mask;
   fme_error2_mask.csr = pError->error2_mask;

   return true;
}


//
// errorSetMask, Sets FME Error mask
//
btBool CHWALIFME::errorSetMask( const INamedValueSet &rInputArgs )
{
   struct CCIP_ERROR ccip_error      = {0};

   struct CCIP_FME_ERROR0  fme_error0_mask         = {0};
   struct CCIP_FME_ERROR1  fme_error1_mask         = {0};
   struct CCIP_FME_ERROR2  fme_error2_mask         = {0};

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

   struct CCIP_FME_ERROR0  fme_error0         = {0};
   struct CCIP_FME_ERROR1  fme_error1         = {0};
   struct CCIP_FME_ERROR2  fme_error2         = {0};


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

   return true;
}

void CHWALIFME::pirntFMEErrors(struct CCIP_ERROR *pError)
{
   btUnsignedInt count                             = 0;
   struct CCIP_FME_ERROR0  fme_error0             = {0};
   struct CCIP_FME_ERROR1  fme_error1             = {0};
   struct CCIP_FME_ERROR2  fme_error2             = {0};
   struct CCIP_FME_FIRST_ERROR  fme_first_error   = {0};
   struct CCIP_FME_NEXT_ERROR  fme_next_error     = {0};
   NamedValueSet fmeErrornvs;

   // FME Error0
   fmeErrornvs.Empty();
   fme_error0.csr =  pError->error0;
   fmeErrornvs.GetNumNames(&count);

   for(int i=0;i<count ;i++) {

      btStringKey type;
      fmeErrornvs.GetName(i,&type);
      std::cout << " FME Error: " << type <<"  Set"<< std::endl;
   }

   // FME Error0 Mask
   fmeErrornvs.Empty();
   fme_error0.csr =  pError->error0_mask;
   fmeErrornvs.GetNumNames(&count);

   for(int i=0;i<count ;i++) {

      btStringKey type;
      fmeErrornvs.GetName(i,&type);
      std::cout << " FME Error Mask: " << type <<"  Set"<< std::endl;
   }

   // FME Error1
   fmeErrornvs.Empty();
   fme_error1.csr =  pError->error1;
   fmeErrornvs.GetNumNames(&count);

   for(int i=0;i<count ;i++) {
      btStringKey type;
      fmeErrornvs.GetName(i,&type);
      std::cout << " FME Error: " << type <<"  Set"<< std::endl;
   }

   // FME Error1 Mask
   fmeErrornvs.Empty();
   fme_error1.csr =  pError->error1_mask;
   fmeErrornvs.GetNumNames(&count);

   for(int i=0;i<count ;i++) {

      btStringKey type;
      fmeErrornvs.GetName(i,&type);
      std::cout << " FME Error Mask: " << type <<"  Set"<< std::endl;
   }

   // FME Error2
   fmeErrornvs.Empty();
   fme_error2.csr =  pError->error2;
   fmeErrornvs.GetNumNames(&count);

   for(int i=0;i<count ;i++) {
      btStringKey type;
      fmeErrornvs.GetName(i,&type);
      std::cout << " FME Error: " << type <<"  Set"<< std::endl;
   }

   // FME Error2 Mask
   fmeErrornvs.Empty();
   fme_error2.csr =  pError->error2_mask;
   fmeErrornvs.GetNumNames(&count);

   for(int i=0;i<count ;i++) {
      btStringKey type;
      fmeErrornvs.GetName(i,&type);
      std::cout << " FME Error Mask: " << type <<"  Set"<< std::endl;
   }

   // FME First Error
   fmeErrornvs.Empty();
   fme_first_error.csr =  pError->first_error;
   fmeErrornvs.GetNumNames(&count);

   for(int i=0;i<count ;i++) {

      btStringKey type;
      fmeErrornvs.GetName(i,&type);
      std::cout << " FME First Error: " << type <<"  Set"<< std::endl;
   }

   // FME Next Error
   fmeErrornvs.Empty();
   fme_next_error.csr =  pError->next_error;
   fmeErrornvs.GetNumNames(&count);

   for(int i=0;i<count ;i++) {
      btStringKey type;
      fmeErrornvs.GetName(i,&type);
      std::cout << " FME Next Error: " << type <<"  Set"<< std::endl;
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
