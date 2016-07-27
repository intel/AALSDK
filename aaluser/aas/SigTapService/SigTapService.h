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
/// @file SigTapService.h
/// @brief Definitions for CCIP SignalTap Service.
/// @ingroup SWSimALIAFU
/// @verbatim
/// Accelerator Abstraction Layer Sample Application
///
///    This application is for example purposes only.
///    It is not intended to represent a model for developing commercially-deployable applications.
///    It is designed to show working examples of the AAL programming model and APIs.
///
/// AUTHORS: Joseph Grecco, Intel Corporation
///          
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 11/19/2015     JG       Initial version.@endverbatim
//****************************************************************************
#ifndef __SIGTAPSERVICE_H__
#define __SIGTAPSERVICE_H__
#include <aalsdk/service/IALIAFU.h>
#include <aalsdk/aas/AALService.h>


/// @addtogroup SWSimALIAFU
/// @{

#if defined ( __AAL_WINDOWS__ )
# pragma warning(push)           // ignoring this because IALIAFU is purely abstract.
# pragma warning(disable : 4275) // non dll-interface class 'AAL::IALIAFU' used as base for dll-interface class 'AAL::SWSimALIAFU'
#endif // __AAL_WINDOWS__

/// @brief This is the Delegate of the Strategy pattern used by ALIAFU to interact with a
/// Software Simulation of ALI (Native Loopback).
///
/// SWSimALIAFU is selected by passing the Named Value pair (ALIAFU_NVS_KEY_TARGET, ALIAFU_NVS_VAL_TARGET_SWSIM)
/// in the arguments to IRuntime::allocService when requesting a ALIAFU.
class SigTapService : public AAL::ServiceBase,
                      public AAL::IServiceClient,     // for HWALIAFU
                      public AAL::IALISignalTap
{
#if defined ( __AAL_WINDOWS__ )
# pragma warning(pop)
#endif // __AAL_WINDOWS__
public:
   // <ServiceBase>

   /// @brief Service Constructor.
   ///
   /// Macro defines the constructor for a loadable AAL service.
   ///  The first argument is your class name, the second argument is the
   ///  name of the Service base class this service is derived from. In this
   ///  example we use ServiceBase as it is the class that provides the
   ///  support for Software-only devices.  Hardware-supported services might
   ///  use DeviceServiceBase instead.
   ///
   /// Note that initializers can be declared here but are preceded by a comma
   ///  rather than a colon.
   ///
   /// The design pattern is that the constructor does minimal work. Here we are
   ///  registering the interfaces the service implements. The default (Subclass)
   ///  interface is ISampleAFUPing.  ServiceBase provides an init() method that
   ///  can be used where more sophisticated initialization is required. The
   ///  init() method is called by the factory AFTER construction but before use.
   DECLARE_AAL_SERVICE_CONSTRUCTOR(SigTapService, AAL::ServiceBase),
   m_ALIMMIO(NULL),
   m_mmio(NULL),
   m_pAALService(NULL)
   {
      SetInterface(iidServiceClient, dynamic_cast<AAL::IServiceClient *>(this));
      SetInterface(iidALI_STAP_Service, dynamic_cast<AAL::IALISignalTap *>(this));
   }

   /// @brief Hook to allow the object to initialize.
   ///
   /// This function is called by the factory AFTER construction and AFTER
   /// _init(), insuring that base class initialization has occurred by the
   /// time this is called.
   ///
   /// The init() method is called by the factory AFTER construction but before
   /// use.
   /// @param[in] pclientBase  A pointer to the Service Client IBase interface.
   /// @param[in] optArgs      A reference to the NamedValueSet containing optional
   ///                     Service parameters.
   /// @param[in] rtid         A reference to the Transaction ID.
   /// @retval             True if the initialization is successful.
   /// @retval             False if the initialization fails.
   virtual AAL::btBool init( AAL::IBase               *pclientBase,
                             AAL::NamedValueSet const &optArgs,
                             AAL::TransactionID const &rtid);

   /// Release the Service.
   /// @param[in] TranID       A reference to the Transaction ID.
   /// @param[in] timeout      The maximum time to wait for the operation to complete.
   /// @retval             True if the initialization is successful.
   /// @retval             False if the initialization fails.
   virtual AAL::btBool Release(AAL::TransactionID const &TranID, AAL::btTime timeout=AAL_INFINITE_WAIT);
   // </ServiceBase>

   // <ALISignalTap>
   /// Get the SignalTap address.
   /// @return             The virtual address.
   virtual AAL::btVirtAddr stpGetAddress( void );
   // </ALISignalTap>

   // <IServiceClient>

   /// Callback used to notify that the Service was allocated successfully.
   /// @param[in] pServiceBase A pointer to the IBase interface of the Service.
   /// @param[in] rTranID      A reference to the Transaction ID.
   /// @return             void
   virtual void serviceAllocated(AAL::IBase               *pServiceBase,
                                 AAL::TransactionID const &rTranID = AAL::TransactionID());
   /// Callback used to notify that the Service allocation failed.
   /// @param[in] rEvent       A reference to the Event with information about the failure.
   /// @return             void
   virtual void serviceAllocateFailed(const AAL::IEvent &rEvent);

   /// Callback used to notify that the Service was Released successfully.
   /// @param[in] rTranID      A reference to the Transaction ID.
   /// @return             void
   virtual void serviceReleased(AAL::TransactionID const &rTranID = AAL::TransactionID());

   virtual void serviceReleaseRequest(IBase *pServiceBase, const AAL::IEvent &rEvent){};  // Ignored TODO better implementation

   /// Callback used to notify that the Service Release failed.
   /// @param[in] rEvent       A reference to the Event with information about the failure.
   /// @return             void
   virtual void serviceReleaseFailed(const AAL::IEvent &rEvent);

   /// Callback used to notify that an Event occurred.
   /// @param[in] rEvent       A reference to the Event.
   /// @return             void
   virtual void serviceEvent(const AAL::IEvent &rEvent);

   // <IServiceClient>


protected:
   AAL::TransactionID          m_TranID;
   AAL::IALIMMIO              *m_ALIMMIO;
   AAL::btVirtAddr             m_mmio;
   AAL::IAALService           *m_pAALService;
   AAL::btTime                 m_timeout;
};
/// @}


#endif // __SIGTAPSERVICE_H__

