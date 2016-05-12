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
/// @file HWALIBase.h
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

#ifndef __HWALIBASE_H__
#define __HWALIBASE_H__

#include "ALIBase.h"
#include "aalsdk/kernel/ccip_defs.h"

class IAFUProxy;

BEGIN_NAMESPACE(AAL)

/// @addtogroup ALI
/// @{

/// @brief This is the Delegate of the Strategy pattern used by ALIAFU to interact with an FPGA-accelerated
///        CCI.
///

class  CHWALIBase : public CALIBase,
                    public IALIMMIO
{
public :

   CHWALIBase( IBase *pSvcClient,
               IServiceBase *pServiceBase,
               TransactionID transID,
               IAFUProxy *pAFUProxy);


   ~CHWALIBase()  { };

   // <IALIMMIO>
   virtual btVirtAddr   mmioGetAddress( void );
   virtual btCSROffset  mmioGetLength( void );

   virtual btBool  mmioRead32( const btCSROffset Offset,       btUnsigned32bitInt * const pValue);
   virtual btBool  mmioWrite32( const btCSROffset Offset, const btUnsigned32bitInt Value);
   virtual btBool  mmioRead64( const btCSROffset Offset,       btUnsigned64bitInt * const pValue);
   virtual btBool  mmioWrite64( const btCSROffset Offset, const btUnsigned64bitInt Value);
   virtual btBool  mmioGetFeatureAddress( btVirtAddr          *pFeatureAddress,
                                          NamedValueSet const &rInputArgs,
                                          NamedValueSet       &rOutputArgs );
   // overloaded version without rOutputArgs
   virtual btBool  mmioGetFeatureAddress( btVirtAddr          *pFeatureAddress,
                                          NamedValueSet const &rInputArgs );
   virtual btBool  mmioGetFeatureOffset( btCSROffset         *pFeatureOffset,
                                         NamedValueSet const &rInputArgs,
                                         NamedValueSet       &rOutputArgs );
   // overloaded version without rOutputArgs
   virtual btBool  mmioGetFeatureOffset( btCSROffset         *pFeatureOffset,
                                         NamedValueSet const &rInputArgs );
   // </IALIMMIO>

   // maps MMIO Regaion
   btBool mapMMIO();

   // AFU Event Handler
   virtual void AFUEvent(AAL::IEvent const &theEvent);

protected:

   IAFUProxy              *m_pAFUProxy;
   btVirtAddr              m_MMIORmap;
   btUnsigned32bitInt      m_MMIORsize;

   // Map to store workspace parameters
   typedef std::map<btVirtAddr, struct aalui_WSMParms> mapWkSpc_t;
   mapWkSpc_t m_mapWkSpc;

   // List to cache device feature metadata
   typedef struct {
      btCSROffset        offset;    //< MMIO offset of feature
      struct CCIP_DFH    dfh;       //< Associated device feature header
      btUnsigned64bitInt guid[2];   //< GUID (for BBBs/private features)
   } FeatureDefinition;
   typedef std::vector<FeatureDefinition> FeatureList;
   FeatureList m_featureList;

private:
   // Populate Device Feature Header
   btBool _discoverFeatures();
   btBool _validateDFL();
   void _printDFH( const struct CCIP_DFH &dfh );
};

/// @}

END_NAMESPACE(AAL)

#endif // __HWALIBASE_H__

