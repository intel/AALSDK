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
/// @file HWALIAFU.cpp
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

#ifndef __HWALIAFU11_H__
#define __HWALIAFU11_H__

#include <aalsdk/service/IALIAFU.h>
#include "HWALIBase.h"


BEGIN_NAMESPACE(AAL)

/// @addtogroup ALI
/// @{

/// @brief This is the Delegate of the Strategy pattern used by ALIAFU to interact with an FPGA-accelerated
///        CCI.
///


class  CHWALIAFU : public CHWALIBase,
                   public IALIBuffer,
                   public IALIUMsg,
                   public IALIReset


{
public :

   CHWALIAFU( IBase *pSvcClient,
              IServiceBase *pServiceBase,
              TransactionID transID,
              IAFUProxy *pAFUProxy);

   ~CHWALIAFU()  { };

   // <IALIBuffer>
   virtual AAL::ali_errnum_e bufferAllocate( btWSSize             Length,
                                           btVirtAddr          *pBufferptr ) { return bufferAllocate(Length, pBufferptr, AAL::NamedValueSet()); }
   virtual AAL::ali_errnum_e bufferAllocate( btWSSize             Length,
                                           btVirtAddr          *pBufferptr,
                                           NamedValueSet const &rInputArgs )
   {
      NamedValueSet temp = NamedValueSet();
      return bufferAllocate(Length, pBufferptr, rInputArgs, temp);
   }
   virtual AAL::ali_errnum_e bufferAllocate( btWSSize             Length,
                                           btVirtAddr          *pBufferptr,
                                           NamedValueSet const &rInputArgs,
                                           NamedValueSet       &rOutputArgs );

   virtual AAL::ali_errnum_e bufferFree( btVirtAddr           Address);
   virtual btPhysAddr bufferGetIOVA( btVirtAddr Address);
   // </IALIBuffer>

   // <IALIUMsg>
   virtual btUnsignedInt umsgGetNumber( void );
   virtual btVirtAddr   umsgGetAddress( const btUnsignedInt UMsgNumber );
   virtual void          umsgTrigger64( const btVirtAddr pUMsg,
                                        const btUnsigned64bitInt Value );
   virtual bool      umsgSetAttributes( NamedValueSet const &nvsArgs);
   // </IALIUMsg>

   // <IALIReset>
   virtual e_Reset afuQuiesceAndHalt( void ) { return afuQuiesceAndHalt(NamedValueSet()); }
   virtual e_Reset afuQuiesceAndHalt( NamedValueSet const &rInputArgs );
   virtual e_Reset afuEnable( void ) { return afuEnable(NamedValueSet()); }
   virtual e_Reset afuEnable( NamedValueSet const &rInputArgs);
   virtual e_Reset afuReset( void ) { return afuReset(NamedValueSet()); }
   virtual e_Reset afuReset( NamedValueSet const &rInputArgs );
   // </IALIReset>

   // AFU Event Handler
   virtual void AFUEvent(AAL::IEvent const &theEvent);

private:

   btVirtAddr              m_uMSGmap;
   btUnsigned32bitInt      m_uMSGsize;

};

/// @} group ALI

END_NAMESPACE(AAL)

#endif // __HWALIAFU11_H__

