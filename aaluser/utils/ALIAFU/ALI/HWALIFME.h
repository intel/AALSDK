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
/// @file HWALIFME.h
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
#ifndef __HWALIFME_H__
#define __HWALIFME_H__

#include "HWALIBase.h"

BEGIN_NAMESPACE(AAL)

/// @addtogroup ALI
/// @{

/// @brief This is the Delegate of the Strategy pattern used by ALIAFU to interact with an FPGA-accelerated
///        CCI.
///

class  CHWALIFME : public CHWALIBase,
                   public IALIPerf,
                   public IALIFMEError,
                   public IALITemperature,
                   public IALIPower
{
public :

   CHWALIFME( IBase *pSvcClient,
              IServiceBase *pServiceBase,
              TransactionID transID,
              IAFUProxy *pAFUProxy);

   ~CHWALIFME()  { };

   //<IALIPerf>
   virtual btBool performanceCountersGet ( INamedValueSet * const  pResult ) { return performanceCountersGet(pResult, NamedValueSet()); }
   virtual btBool performanceCountersGet ( INamedValueSet * const  pResult,
                                           NamedValueSet    const &pOptArgs );
   //</IALIPerf>

   // <IALIError>
   virtual btBool errorGet( INamedValueSet &rResult );
   virtual btBool errorGet( btUnsigned64bitInt &error );

   virtual btBool errorGetFirst( INamedValueSet &rResult );
   virtual btBool errorGetFirst( btUnsigned64bitInt &stError );

   virtual btBool errorGetMask( INamedValueSet &rResult ) ;
   virtual btBool errorGetMask( btUnsigned64bitInt &errorMask );

   virtual btBool errorSetMask( const INamedValueSet &rInputArgs) ;
   virtual btBool errorSetMask(const btUnsigned64bitInt errorMask);

   virtual btBool errorClear( const INamedValueSet &rInputArgs);
   virtual btBool errorClear( const btUnsigned64bitInt error);

   virtual btBool errorClearAll() ;
   // </IALIError>

   // <IALITemperature>
   virtual btBool thermalGetValues( INamedValueSet &rResult );
   // </IALITemperature>

   // <IALIPower>
   virtual btBool powerGetValues(INamedValueSet &rResult );
   // </IALIPower>

   // AFU Event Handler
   virtual void AFUEvent(AAL::IEvent const &theEvent);
};

/// @}
END_NAMESPACE(AAL)

#endif // __HWALIFME_H__

