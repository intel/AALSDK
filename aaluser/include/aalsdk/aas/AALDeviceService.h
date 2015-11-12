// Copyright (c) 2011-2015, Intel Corporation
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
/// @file AALDeviceService.h
/// @brief This file contains the implementation of classes and templates
///   used to create AAL device services.
/// @ingroup Services
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHOR: Joseph Grecco, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 08/30/2011      JG      Initial version
/// 09/05/2011      JG      Implemented Release and clean destruction
/// 10/11/2011      TSW     Post exception from init when AIA not found in
///                          Config Record.
/// 10/26/2011      JG      Added code that cleans up AIA better with a Release
/// 01/26/2012      JG      Added support for InitComplete<> function object
///                           event template.
/// 04/19/2012      TSW     Deprecate __PRETTY_FUNCTION__ in favor of
///                          platform-agnostic __AAL_FUNCSIG__.
/// 06/07/2012      JG       Fixed Release() protocol so that it now cleans-up
///                           from the most derived toward the base.  Derived
///                           class need only implement Release() if it has work
///                           do, in which case it cleans itself up and calls
///                           parent Release().
///                          TODO AIA Release() change  from quiet to async.@endverbatim
//****************************************************************************
#ifndef __AALSDK_AAS_AALDEVICSERVICE_H__
#define __AALSDK_AAS_AALDEVICSERVICE_H__
#include <aalsdk/aas/AALService.h>
#include <aalsdk/uaia/AIA.h>         // IAFUDev{}
#include <aalsdk/uaia/AALuAIA_Messaging.h>
#include <aalsdk/uaia/uAIASession.h> // uAIASession{}
#include <aalsdk/uaia/uAIA.h>        // CAIA{}
#include <aalsdk/aas/Dispatchables.h>

#include <aalsdk/IServiceClient.h>

BEGIN_NAMESPACE(AAL)

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//////////////                                                   //////////////
///////                          DEVICE SERVICE                         ///////
//////////////                                                   //////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/// @addtogroup DeviceServiceBase
/// @{


/// Base class for uAIA hardware-based AAL Services.
///
/// Device Services are services that communicate with hardware devices using the
///  Universal Application Interface Adaptor (uAIA) and talking to a kernel
///  based Physical Interface Protocol (PIP) object.
///
/// The DeviceServiceBase extends the ServiceBase class by adding the uAIA
///   loading and device binding with the construction of a CAFU object.
/// NOTE: DeviceService allocates sub-Services (AIA) and therefore must supply
///   a IServiceClient.  However Base classes should not directly publish Client
///   interfaces so as not to interfere with subclasses that may need to publish
///   them.  So the solution is to implement IServiceCLient as a private interface
///   to this class but do not register it with the IBase.  DeviceServiceBase defines
///   an aggregator object (ServiceClient) that implements a separate IBase and registers
///   the DeviceService's IServiceClient. This avoids conflict if a subclass wants to
///   implement IServiceClient itself. Only the subclass publishes Client interfaces in the
///   common class IBase
class DeviceServiceBase : public ServiceBase, private IServiceClient
{
public:
   /// DeviceServiceBase Constructor.
   DeviceServiceBase(AALServiceModule *container,
                     IRuntime         *pRuntime,
                     IAALTransport    *ptransport   = NULL,
                     IAALMarshaller   *marshaller   = NULL,
                     IAALUnMarshaller *unmarshaller = NULL) :
      ServiceBase(container, pRuntime, ptransport, marshaller, unmarshaller),
      m_pAIA(NULL),
      m_pSession(NULL),
      m_devHandle(NULL),
      m_pAFUDev(NULL),
      m_quietRelease(false),
      m_scContainer(this)
   {}

   /// DeviceServiceBase Destructor.
   virtual ~DeviceServiceBase()
   {
      if ( ( NULL != m_pAIA ) && ( NULL != m_pSession ) ) {
         m_pAIA->DestroyAIASession(m_pSession);
         m_pSession = NULL;
      }
   }

   // <IAALService>

   btBool Release(TransactionID const &rTranID, btTime timeout)
   {

      TransactionID newTID = WrapTransactionID(rTranID);
      m_pSession->UnBind(m_devHandle, newTID);
      return true;
   }

   // </IAALService>


   // <ServiceBase>

   virtual btBool _init( IBase               *pclient,
                         TransactionID const &rtid,
                         NamedValueSet const &optArgs,
                         CAALEvent           *pcmpltEvent = NULL,
                         btBool               NoRuntimeEvent = false)
   {
      // Check to see if this is the direct super class of the most derived class
      if ( NULL != pcmpltEvent ) {
         // No - save the completion event and post it when our initialization
         //  completes
         m_pcmpltEvent = pcmpltEvent;
         return true;
      }

      // Call our direct superclass' _init()
      //  Our real initialization occurs in Doinit() which will be called
      //  after ServiceBase initializes
      return ServiceBase::_init( pclient,
                                 rtid,
                                 optArgs,
                                 new InitComplete<DeviceServiceBase>(this, &DeviceServiceBase::Doinit, rtid));
   }


   // </ServiceBase>

   // Convenience function to return the AFUdev
   btBool                 IsAFUDevOK()   { return NULL != m_pAFUDev; }
   IAFUDev &              AFUDev()       { return *m_pAFUDev;        }
   INamedValueSet const & ConfigRecord() { return *m_ConfigRecord;   }

private:
   //=============================================================================
   // Name: Doinit
   // Description: Perform any post creation initialization including establishing
   //               communications.
   // Comments: This function is virtual to allow the derived class to hook the
   //            init() routine. Derived classes should call the base
   //            implementation before adding their own functionality.
   //  NOTE: Initializing an AFU may involved a multi state state machine. It
   //  may be necessary for the derived AFU class to be notified when the object
   //  created event is being sent (and thus the object is about to be allocated).
   //  The derived class can capture this event by passing its own TransactionID
   //  with filtering on when it calls this super class function, overriding the
   //  default handler. It then must forward the event when ready.
   //=============================================================================

   //===================================================================================
   // ServiceClient
   // Description:
   //===================================================================================
   class ServiceClient : public CAASBase
   {
   public:
      ServiceClient( DeviceServiceBase *pdsb): m_pdsb(pdsb)
   {
      SetInterface(iidServiceClient, dynamic_cast<IServiceClient *>(m_pdsb));
   }

   private:
      DeviceServiceBase * m_pdsb;
   };
  /// @brief Perform any post creation initialization including establishing communications.
  ///
  ///   This function is virtual to allow the derived class to hook the
  ///            init() routine. Derived classes should call the base
  ///           implementation before adding their own functionality.
   void Doinit(TransactionID const &rtid)
   {
      // Get the config record
      if ( !OptArgs().Has(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED) ) {
         goto badparm;
      }

      OptArgs().Get(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &m_ConfigRecord);

      // Get the device handle if there is one
      if ( OptArgs().Has(keyRegHandle) ) {
         OptArgs().Get(keyRegHandle, &m_devHandle);
      } else {
         goto badparm;
      }

      // Check to see if an AIA is required for this service
      if ( ConfigRecord().Has(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_AIA_NAME) ) {
         //
         // An AIA service is required to operate this AFU so it must
         //  be acquired by the Factory.  This is an asynchronous function
         //  so the ObjectCreated event cannot be sent until later.  We
         //  end up implementing a simple state machine.

         // Get the AIA service
         btcString btcAIAName;
         ConfigRecord().Get(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_AIA_NAME, &btcAIAName);

         // Create the manifest for the AIA service
         NamedValueSet nvsServiceRecord;

         nvsServiceRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, btcAIAName);
         nvsServiceRecord.Add(AAL_FACTORY_CREATE_SOFTWARE_SERVICE, true);

         NamedValueSet nvsManifest;
         nvsManifest.Add(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &nvsServiceRecord);

         // Copy original transaction ID so that we have it when it comes time to generate the
         //  ObjectCreated Event
         //
         TransactionID tidLoadAIA = WrapTransactionID(rtid);

         // Allocate the AIA. The last parameter means do not send notification to RuntimeClient
         //   Pass the m_scContainer as the IBase containing the IServiceClient.  The ServiceClient
         //   actually publishes our private IServiceClient for us. (See note at top of class).
         allocService(&m_scContainer, nvsManifest, tidLoadAIA);
      }
      return;

   badparm:
      // Post the object created exception
      initFailed(new CExceptionTransactionEvent( dynamic_cast<IBase *>(this),
                                                 rtid,
                                                 errCreationFailure,
                                                 reasMissingParameter,
                                                 "No AIA specified in Config Record."));
   }
  

   void serviceAllocated(IBase *pServiceBase, TransactionID const &rTranID)
   {
      // TODO check for NULL
      m_pAIA = dynamic_ptr<CAIA>(iidAIA, pServiceBase);

      // Add the context used much later in the protocol
      m_pAIA->SetContext(this);

      //---------------------------------------------------
      // Create a session with the AIA which serves as a
      //  connection between this object and the kernel
      //  services including the device we bind to.
      //----------------------------------------------------
      m_pSession = m_pAIA->CreateAIASession(static_cast<IBase *>(this),
                                             this,
                                             DeviceServiceBase::_DefaultAIAHandler,
                                             this);


      if ((NULL == m_pSession) || !m_pSession->IsOK()) {
         m_pAIA->DestroyAIASession(m_pSession);
         m_pSession = NULL;
         return;
      }

      m_pSession->Bind(m_devHandle, NamedValueSet(), rTranID);

   }
   

   void serviceAllocateFailed(const IEvent &rEvent)
   {
      std::cerr << "TODO FAILDE ALLOCATE IN DEVICE SERVICE BASE\n";
   }
   
   
   void serviceReleased(TransactionID const &rTranID = TransactionID())
   {
      std::cerr << "TODO FREED\n";
      // AIA Released now Unbind
   }

   void serviceReleaseFailed(const IEvent &rEvent)
   {
      std::cerr << "TODO FAILDE ALLOCATE IN DEVICE SERVICE BASE\n";
   }


   void serviceEvent(const IEvent &rEvent)
   {
      _EventCallbackHandler(rEvent);
   }

   //=============================================================================
   // Name: _EventCallbackHandler()
   // Description: static Callback Handler
   // Interface: public
   // Inputs: none
   // Outputs: none.
   // Comments:
   //=============================================================================
   ///@brief static Callback Handler
   ///
   ///Gets this pointer from the object's context which was provided when we created it.
   static void _EventCallbackHandler(IEvent const &theEvent)
   {
      // Get this pointer from the object's context which was provided when we created it.
      DeviceServiceBase *This = reinterpret_cast<DeviceServiceBase *>(theEvent.Object().Context());
      This->EventCallbackHandler(theEvent);
   }

   //=============================================================================
   // Name: EventCallbackHandler()
   // Description: Delegate callback handler
   // Interface: public
   // Inputs: none
   // Outputs: none.
   // Comments:
   //=============================================================================
   /// @brief Delegate callback handler
   ///
   ///  Create a session with the AIA which serves as a
   ///  connection between this object and the kernel
   ///  services including the device we bind to.
   void EventCallbackHandler(IEvent const &theEvent)
   {
      // TODO check for NULL
      m_pAIA = dynamic_ptr<CAIA>(iidAIA, theEvent.Object());

      if ( AAL_IS_EXCEPTION(theEvent.SubClassID()) ) {
         //Print the description string.
         PrintExceptionDescription(theEvent);
         // Send the failure event. Unwrap and return the original TrasnactionID from the event
         initFailed( new CExceptionTransactionEvent(dynamic_cast<IBase *>(this),
                                                    UnWrapTransactionIDFromEvent(theEvent),
                                                    errCreationFailure,
                                                    dynamic_ref<IExceptionTransactionEvent>(iidTranEvent, theEvent).Reason(),
                                                    dynamic_ref<IExceptionTransactionEvent>(iidTranEvent, theEvent).Description()));
         return;
      }



      //---------------------------------------------------
      // Create a session with the AIA which serves as a
      //  connection between this object and the kernel
      //  services including the device we bind to.
      //----------------------------------------------------

      m_pSession = m_pAIA->CreateAIASession(static_cast<IBase *>(this),
                                            this,
                                            DeviceServiceBase::_DefaultAIAHandler,
                                            this);

      if ( (NULL == m_pSession) || !m_pSession->IsOK() ) {
          m_pAIA->DestroyAIASession(m_pSession);
          m_pSession = NULL;
          return;
      }

      m_pSession->Bind(m_devHandle, NamedValueSet(), event_to_tranevent(theEvent).TranID());

   }  // DeviceServiceBase::EventCallbackHandler

   //=============================================================================
   // Name: _DefaultAIAHandler
   // Description: Static event handler
   // Interface: public
   // Inputs: none.
   // Outputs: none.
   // Comments:
   //=============================================================================
   /// @brief Static event handler
   ///
   /// The object that generated the event (AIAProxy) has our this as its context
   static void _DefaultAIAHandler(IEvent const &theEvent)
   {
      // The object that generated the event (AIAProxy) has our this as its context
      DeviceServiceBase *This = static_cast<DeviceServiceBase *>(theEvent.Object().Context());
      This->DefaultAIAHandler(theEvent);
   }

   //=============================================================================
   // Name: DefaultAIAHandler
   // Description: Delegate AIA handler
   // Interface: public
   // Inputs: none.
   // Outputs: none.
   // Comments:
   //=============================================================================
   /// @brief Delegate AIA handler
   ///
   /// Delegate AIA handler
   void DefaultAIAHandler(IEvent const &theEvent)
   {
      // Check for exception
      if ( AAL_IS_EXCEPTION(theEvent.SubClassID()) ) {
         PrintExceptionDescription(theEvent);
         return;
      }

      // Canonical binding code
      switch ( theEvent.SubClassID() ) {

         // Device has been bound to this process. The kernel device (PIP) is
         // is abstsacted via the CAFUdev class
         case tranevtBindAFUDevEvent : {
            IBindAFUDevEvent &revt = dynamic_ref<IBindAFUDevEvent>(tranevtBindAFUDevEvent, theEvent);

            // Obtain the AFUDev, which represents the device handle, and which will be used subsequently
            //    for communication with the device and its workspace.
            m_pAFUDev = revt.pAFUDev();

            // The transaction ID has a copy of the original TransactionID as its context
            //  NOTE: this function also deletes the original transaction ID that was allocated
            //  before the call to Bind().
            TransactionID origTID = UnWrapTransactionIDFromEvent(theEvent);


            //===========================
            //
            //  INITIALIZATION COMPLETE
            //
            //===========================

            // If _init() was called by a subclass then queue its initialization
            //
            // if(NULL != m_pcmpltEvent){
            //    AASQueueEvent(m_pcmpltEvent;
            // }else{

            // Last superclass before most derived so call init()
            init(getServiceClientBase(), OptArgs(), origTID);

         } break;

         // The kernel device has been unbound from this process.
         //  Clean up and send an event unless quiet mode has been enabled.
         //  Quiet mode implements a more synchronous protocol, relying on the
         //  Released() method to notify upstream.  This is used when service module
         //  is about to be unloaded.
         case tranevtUnBindAFUDevEvent : {
            // The transaction ID has the original TransactionID as its context
            TransactionID origTID = UnWrapTransactionIDFromEvent(theEvent);
#if 0
            // If it is not a quiet release
            if ( !m_quietRelease ) {
               // Generate the event
               m_pAIA->getRuntime()->schedDispatchable( new CObjectDestroyedTransactionEvent( Client(),
                                                                                              dynamic_cast<IBase *>(this),
                                                                                              origTID,
                                                                                              Context()));
            }
#endif
            // Destroy the AIA session
            if ( m_pAIA != NULL ) {
               m_pAIA->DestroyAIASession(m_pSession);
               m_pSession = NULL;
            }
//DO OPPOSITE OF START UP. Release AIA

            m_devHandle = NULL;

            // Quiet release for now  TODO perhaps should be regular release
            if ( m_pAIA != NULL ) {
               m_pAIA->Delete();
            }

            // If it is not a quiet release
            if ( !m_quietRelease ) {

               ServiceBase::Release(origTID, 0);
/*
               // Generate the event
               getRuntime()->schedDispatchable( new ServiceClientCallback( ServiceClientCallback::Released,
                                                                           Client(),
                                                                           dynamic_cast<IBase *>(this),
                                                                           origTID));
*/
            }else{
               // MUST call parent Release
               ServiceBase::ReleaseComplete();
            }
         } break;

         default:
            AAL_ERR(LM_AFU, __AAL_FUNCSIG__ << ": unknown tranevt\n");
            break;
      }  // end switch (theEvent.SubClassID())

   } // DeviceServiceBase::DefaultAIAHandler

   CAALEvent           *m_pcmpltEvent;

protected:

   CAIA                 *m_pAIA;         // AIA service
   uAIASession          *m_pSession;     // AIA Session
   btObjectType          m_devHandle;    // low-level device handle
   IAFUDev              *m_pAFUDev;      // AFU device
   INamedValueSet const *m_ConfigRecord; // Config Record
   btBool                m_quietRelease; // Used for quiet release
   ServiceClient         m_scContainer;  // IBase container for presenting ServiceClient
};


/// @}


END_NAMESPACE(AAL)


#endif // __AALSDK_AAS_AALDEVICESERVICE_H__

