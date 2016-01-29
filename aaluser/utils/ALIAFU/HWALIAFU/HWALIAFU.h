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
/// @file HWALIAFU.h
/// @brief Definitions for ALI Hardware AFU Service.
/// @ingroup HWALIAFU
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
/// 07/20/2015     HM       Initial version.@endverbatim
//****************************************************************************
#ifndef __HWALIAFU_H__
#define __HWALIAFU_H__
#include <aalsdk/AALLoggerExtern.h>
#include <aalsdk/service/ALIAFUService.h>
#include <aalsdk/service/HWALIAFUService.h>
#include <aalsdk/service/IALIAFU.h>

//#include <aalsdk/utils/AALEventUtilities.h>     // UnWrapTransactionIDFromEvent
#include <aalsdk/uaia/IAFUProxy.h>

BEGIN_NAMESPACE(AAL)

/// @addtogroup HWALIAFU
/// @{

#if defined ( __AAL_WINDOWS__ )
# pragma warning(push)           // ignoring this because IALIAFU is purely abstract.
# pragma warning(disable : 4275) // non dll-interface class 'AAL::IALIAFU' used as base for dll-interface class 'AAL::HWALIAFU'
#endif // __AAL_WINDOWS__

/// @brief This is the Delegate of the Strategy pattern used by ALIAFU to interact with an FPGA-accelerated
///        CCI.
///
/// HWALIAFU is selected by passing the Named Value pair (ALIAFU_NVS_KEY_TARGET, ALIAFU_NVS_VAL_TARGET_FPGA)
/// in the arguments to IRuntime::allocService when requesting a ALIAFU.
class HWALIAFU_API HWALIAFU : public ServiceBase,
                              public IServiceClient,     // for AIA
                              public IAFUProxyClient,
                              public IALIMMIO,
                              public IALIBuffer,
                              public IALIUMsg,
                              public IALIReset,
                              public IALIPerf,
                              public IALIReconfigure
{
#if defined ( __AAL_WINDOWS__ )
# pragma warning(pop)
#endif // __AAL_WINDOWS__
public:
   // <DeviceServiceBase>
DECLARE_AAL_SERVICE_CONSTRUCTOR(HWALIAFU,ServiceBase),
      m_pAALService(NULL),
      m_pAFUProxy(NULL),
      m_tidSaved(),
      m_pSvcClient(NULL),
      m_mapWkSpc(),
      m_MMIORmap(NULL),
      m_MMIORsize(0),
      m_uMSGmap(NULL),
      m_uMSGsize(0),
      m_pReconClient(NULL)
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

      if ( EObjOK != SetInterface(iidALI_RSET_Service, dynamic_cast<IALIReset *>(this)) ){
         m_bIsOK = false;
      }

      if ( EObjOK != SetInterface(iidALI_PERF_Service, dynamic_cast<IALIPerf *>(this)) ){
         m_bIsOK = false;
      }

      if ( EObjOK != SetInterface(iidServiceClient, dynamic_cast<IServiceClient *>(this)) ){
         m_bIsOK = false;
      } // for AIA

      if ( EObjOK != SetInterface(iidAFUProxyClient, dynamic_cast<IAFUProxyClient *>(this)) ){
         m_bIsOK = false;
      }  // for AFUProy
   }  // DECLARE_AAL_SERVICE_CONSTRUCTOR()

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
   virtual btBool  mmioGetFeature( btVirtAddr          *pFeature,
                                   NamedValueSet const &rInputArgs,
                                   NamedValueSet       &rOutputArgs );
   // overloaded version without rOutputArgs
   virtual btBool  mmioGetFeature( btVirtAddr          *pFeature,
                                   NamedValueSet const &rInputArgs )
   {
      NamedValueSet temp;
      return mmioGetFeature(pFeature, rInputArgs, temp);
   }
   // </IALIMMIO>

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

   // <IServiceClient>
   virtual void serviceAllocated(IBase               *pServiceBase,
                                 TransactionID const &rTranID = TransactionID());  // FIXME: potential dangling reference
   virtual void serviceAllocateFailed(const IEvent &rEvent);
   virtual void serviceReleased(TransactionID const &rTranID = TransactionID());  // FIXME: potential dangling reference
   virtual void serviceReleaseFailed(const IEvent &rEvent);
   virtual void serviceEvent(const IEvent &rEvent);
   // </IServiceClient>

   // <IAFUProxyClient>
   virtual void AFUEvent(AAL::IEvent const &theEvent);
   // </IAFUProxyClient>

   //<IALIPerf>
   virtual btBool performanceCountersGet ( INamedValueSet * const  pResult ) { return performanceCountersGet(pResult, NamedValueSet()); }
   virtual btBool performanceCountersGet ( INamedValueSet * const  pResult,
                                           NamedValueSet    const &pOptArgs );
   //</IALIPerf>

   // <ALIReconfigure>
   virtual void reconfDeactivate( TransactionID const &rTranID,
                                  NamedValueSet const &rInputArgs );
   virtual void reconfConfigure( TransactionID const &rTranID,
                                 NamedValueSet const &rInputArgs );
   virtual void reconfActivate( TransactionID const &rTranID,
                                NamedValueSet const &rInputArgs );
   // </ALIReconfigure>
   enum InitTransaction {
      GetMMIO = 1,
      GetUMSG
   };

protected:
   btBool configureForAFU();


   IAALService            *m_pAALService;
   IAFUProxy              *m_pAFUProxy;
   TransactionID           m_tidSaved;
   IBase                  *m_pSvcClient;
   btVirtAddr              m_MMIORmap;
   btUnsigned32bitInt      m_MMIORsize;
   btVirtAddr              m_uMSGmap;
   btUnsigned32bitInt      m_uMSGsize;
   IALIReconfigure_Client *m_pReconClient;
   struct ReleaseContext {
      const TransactionID   TranID;
      const btTime          timeout;
      ReleaseContext(const TransactionID &rtid, btTime to) :
         TranID(rtid),
         timeout(to)
      {}
   };


   // Map to store workspace parameters
   typedef std::map<btVirtAddr, struct aalui_WSMParms> mapWkSpc_t;
   mapWkSpc_t m_mapWkSpc;

};

/// @}

END_NAMESPACE(AAL)

#endif // __HWALIAFU_H__

