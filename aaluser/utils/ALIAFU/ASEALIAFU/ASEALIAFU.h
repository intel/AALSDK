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
/// @file ASEALIAFU.h
/// @brief Definitions for ASE ALI AFU Service.
/// @ingroup ASEALIAFU
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer Sample Application
///
///    This application is for example purposes only.
///    It is not intended to represent a model for developing commercially-deployable applications.
///    It is designed to show working examples of the AAL programming model and APIs.
///
/// AUTHORS: Henry Mitchel, Intel Corporation
///          Joseph Grecco, Intel Corporation
///          
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 07/20/2015     HM       Initial version.
/// 12/16/2015     RRS      Integration with ASE App-backend.@endverbatim
//****************************************************************************
#ifndef __ASEALIAFU_H__
#define __ASEALIAFU_H__
#include <aalsdk/service/IALIAFU.h>
#include <aalsdk/aas/AALService.h>
#include <aalsdk/ase/ase_common.h>

#include <aalsdk/service/ALIAFUService.h>
#include <aalsdk/service/ASEALIAFUService.h>

BEGIN_NAMESPACE(AAL)

/// @addtogroup ASEALIAFU
/// @{

/// @brief This is the Delegate of the Strategy pattern used by ALIAFU to interact with the
/// AFU Simulation Environment.
///
/// ASEALIAFU is selected by passing the Named Value pair (ALIAFU_NVS_KEY_TARGET, ALIAFU_NVS_VAL_TARGET_ASE)
/// in the arguments to IRuntime::allocService when requesting a ALIAFU.
class ASEALIAFU_API ASEALIAFU : public ServiceBase,
                              public IServiceClient,     // for AIA
                              public IALIMMIO,
                              public IALIBuffer,
                              public IALIUMsg,
                              public IALIReset/*
                              public IALIPerf*/
{
#if defined ( __AAL_WINDOWS__ )
# pragma warning(pop)
#endif // __AAL_WINDOWS__
public:
   // <ServiceBase>
   DECLARE_AAL_SERVICE_CONSTRUCTOR(ASEALIAFU, ServiceBase),
      m_Last3c4(0xffffffff),
      m_Last3cc(0xffffffff)
   {

      // TODO: at some point, these probably go into init() and be exposed based
      //       on the actual capabilities
      if ( EObjOK !=  SetInterface(iidALI_MMIO_Service, dynamic_cast<IALIMMIO *>(this)) ) {
         m_bIsOK = false;
      }

      if ( EObjOK != SetInterface(iidALI_UMSG_Service, dynamic_cast<IALIUMsg *>(this)) ){
         m_bIsOK = false;
      }

      if ( EObjOK != SetInterface(iidALI_BUFF_Service, dynamic_cast<IALIBuffer *>(this)) ){
         m_bIsOK = false;
      }
//      SetInterface(        iidALI_PERF_Service,   dynamic_cast<IALIPerf *>(this)); // still to be defined
      if ( EObjOK != SetInterface(iidALI_RSET_Service, dynamic_cast<IALIReset *>(this)) ){
         m_bIsOK = false;
      }

      if ( EObjOK != SetInterface(iidServiceClient, dynamic_cast<IServiceClient *>(this)) ){
         m_bIsOK = false;
      } // for AIA

   }


   virtual btBool init(IBase               *pclientBase,
                       NamedValueSet const &optArgs,
                       TransactionID const &rtid);

   virtual btBool Release(TransactionID const &TranID, btTime timeout=AAL_INFINITE_WAIT);
   // </DeviceServiceBase>

   // <IALIMMIO>
   virtual btVirtAddr   mmioGetAddress( void );
   virtual btCSROffset  mmioGetLength( void );

   virtual btBool   mmioRead32( const btCSROffset Offset,       btUnsigned32bitInt * const pValue);
   virtual btBool  mmioWrite32( const btCSROffset Offset, const btUnsigned32bitInt Value);
   virtual btBool   mmioRead64( const btCSROffset Offset,       btUnsigned64bitInt * const pValue);
   virtual btBool  mmioWrite64( const btCSROffset Offset, const btUnsigned64bitInt Value);
   virtual btBool  mmioGetFeature( const btString GUID, const btUnsigned16bitInt FeatureID, void ** const ppFeature);
   // </IALIMMIO>

   // <IALIBuffer>
   virtual AAL::ali_errnum_e bufferAllocate( btWSSize             Length,
                                             btVirtAddr          *pBufferptr,
                                             NamedValueSet       *pOptArgs = NULL );
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
   virtual IALIReset::e_Reset afuQuiesceAndHalt( NamedValueSet const *pOptArgs = NULL){};
   virtual IALIReset::e_Reset afuEnable( NamedValueSet const *pOptArgs = NULL){};
   virtual IALIReset::e_Reset afuReset( NamedValueSet const *pOptArgs = NULL){};
   // </IALIReset>

   // <IServiceClient>
   virtual void serviceAllocated(IBase               *pServiceBase,
                                 TransactionID const &rTranID = TransactionID()){};  // FIXME: potential dangling reference
   virtual void serviceAllocateFailed(const IEvent &rEvent){};
   virtual void serviceReleased(TransactionID const &rTranID = TransactionID()){};  // FIXME: potential dangling reference
   virtual void serviceReleaseFailed(const IEvent &rEvent){};
   virtual void serviceEvent(const IEvent &rEvent){};
   // </IServiceClient>


protected:
   typedef std::map<btVirtAddr, buffer_t> map_t;
   typedef map_t::iterator                map_iter;
   typedef map_t::const_iterator          const_map_iter;

   btVirtAddr           m_MMIORmap;
   btUnsigned32bitInt   m_MMIORsize;
   btVirtAddr           m_uMSGmap;
   btUnsigned32bitInt   m_uMSGsize;

   btCSRValue             m_Last3c4;
   btCSRValue             m_Last3cc;
   map_t                  m_WkspcMap;

   // Map to store workspace parameters
   typedef std::map<btVirtAddr, struct aalui_WSMParms> mapWkSpc_t;
   mapWkSpc_t m_mapWkSpc;  

   static CriticalSection sm_ASEMtx;
};

/// @}

END_NAMESPACE(AAL)

#endif // __ASEALIAFU_H__

