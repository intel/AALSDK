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
/// @file ALI.h
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
#ifndef __ALI_H__
#define __ALI_H__

#include <aalsdk/AALLoggerExtern.h>
#include <aalsdk/service/ALIService.h>
#include <aalsdk/service/IALIAFU.h>
#include <aalsdk/uaia/IAFUProxy.h>

class CALIBase;

BEGIN_NAMESPACE(AAL)

/// @addtogroup ALI
/// @{

#if defined ( __AAL_WINDOWS__ )
# pragma warning(push)           // ignoring this because IALIAFU is purely abstract.
# pragma warning(disable : 4275) // non dll-interface class 'AAL::IALIAFU' used as base for dll-interface class 'AAL::HWALIAFU'
#endif // __AAL_WINDOWS__

/// @brief This is the Delegate of the Strategy pattern used by ALI to interact with an FPGA-accelerated
///        CCI.
///
/// ALI is selected by passing the Named Value pair (ALIAFU_NVS_KEY_TARGET, ALIAFU_NVS_VAL_TARGET_FPGA)
/// in the arguments to IRuntime::allocService when requesting a ALIAFU.
class ALI_API ALI: public ServiceBase,
                   public IServiceClient,
                   public IAFUProxyClient
{
#if defined ( __AAL_WINDOWS__ )
# pragma warning(pop)
#endif // __AAL_WINDOWS__
public:
   // <DeviceServiceBase>
DECLARE_AAL_SERVICE_CONSTRUCTOR(ALI,ServiceBase),
                                m_pAALService(NULL),
                                m_pAFUProxy(NULL),
                                m_tidSaved(),
                                m_pSvcClient(NULL),
                                m_pALIBase(NULL)
   {
      if ( EObjOK != SetInterface(iidServiceClient, dynamic_cast<IServiceClient *>(this)) ){
         m_bIsOK = false;
      } // for AIA

   }  // DECLARE_AAL_SERVICE_CONSTRUCTOR()

   virtual btBool init(IBase               *pclientBase,
                       NamedValueSet const &optArgs,
                       TransactionID const &rtid);

   virtual btBool Release(TransactionID const &TranID, btTime timeout=AAL_INFINITE_WAIT);
   // </DeviceServiceBase>


   // <IServiceClient>
   virtual void serviceAllocated(IBase               *pServiceBase,
                                 TransactionID const &rTranID = TransactionID());
   virtual void serviceAllocateFailed(const IEvent &rEvent);
   virtual void serviceReleased(TransactionID const &rTranID = TransactionID());
   virtual void serviceReleaseRequest(IBase *pServiceBase, const IEvent &rEvent);
   virtual void serviceReleaseFailed(const IEvent &rEvent);
   virtual void serviceEvent(const IEvent &rEvent);
   // </IServiceClient>

   // <IAFUProxyClient>
   virtual void AFUEvent(AAL::IEvent const &theEvent);
   // </IAFUProxyClient>

   // Sets FME interfaces
   btBool setFMEInterfaces();
   // Sets Reconfigure interfaces
   btBool setReconfInterfaces();
   // Sets Port interfaces
   btBool setPortInterfaces();
   // Sets AFU interfaces
   btBool setAFUInterfaces();
   // Sets Signal Tap interfaces
   btBool setSigTapInterfaces();

   // Initialize ASE
   btBool ASEInit();

protected:

   IAALService            *m_pAALService;
   IAFUProxy              *m_pAFUProxy;
   TransactionID           m_tidSaved;
   IBase                  *m_pSvcClient;
   CALIBase               *m_pALIBase;

   struct ReleaseContext {
      const TransactionID   TranID;
      const btTime          timeout;
      ReleaseContext(const TransactionID &rtid, btTime to) :
         TranID(rtid),
         timeout(to)
      {}
   };

};

/// @} group ALI

END_NAMESPACE(AAL)

#endif // __ALI_H__

