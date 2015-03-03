// Copyright (c) 2007-2015, Intel Corporation
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
/// @file XLSample2.h
/// @brief Definitions for XLSample2.
/// @ingroup XLSample2
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer Sample Application
///
///    This application is for example purposes only.
///    It is not intended to represent a model for developing commercially-deployable applications.
///    It is designed to show working examples of the AAL programming model and APIs.
///
/// AUTHORS: Joseph Grecco, Intel Corporation
///          Henry Mitchel, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 11/16/2007     JG       Initial version started
/// 05/10/2008     HM       Comments & License
/// 01/04/2009     HM       Updated Copyright@endverbatim
//****************************************************************************
#ifndef __AALSAMPLE2_H__
#define __AALSAMPLE2_H__
#include <aalsdk/AAL.h>
#include <aalsdk/xlRuntime.h>

using namespace AAL;

// Change DBG_HOOK to 1 if you want an opportunity to attach the debugger.
// After attaching, set gWaitForDebuggerAttach to 0 via the debugger to unblock the app.
#define DBG_HOOK 0

// Convenience macros for printing messages and errors.
#define MSG(x) std::cerr << __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() : " << x << std::endl
#define ERR(x) std::cerr << __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() **Error : " << x << std::endl

// Print/don't print the event ID's entered in the event handlers.
#if 1
# define EVENT_CASE(x) case x : MSG(#x);
#else
# define EVENT_CASE(x) case x :
#endif

#include "SampleAFU2Service.h"

//=============================================================================
// Classes
//=============================================================================

//--------------------------------------------------------------------------------------------
// Stream states
//--------------------------------------------------------------------------------------------
typedef enum enumStreaming {
   Stopped=0,
   Stopping,
   Streaming
} enumStreaming;

//--------------------------------------------------------------------------------------------
// System context -
//--------------------------------------------------------------------------------------------
//=============================================================================
// Name: AppContext
// Description: Global Context used by all threads. The AppContext is an object
//              which holds the consumer and producer Queues, synchronization
//              objects and cached copies of the various AFU
//              interfaces.  This object is accessed by multiple threads
//              simultaneously and must be thread safe.
// Comments: We lock all but the Wait and Post methods to protect critical
//           sections from renterancy.  A bit of over kill since in most cases
//           no 2 threads will be accessing the same methods. We do it to
//           stress the importance of thread safety.
// ============================================================================
class AppContext : public CAASBase,
                   public IRuntimeClient,
                   public IServiceClient
{
public:
   AppContext() :
      m_pRuntime(NULL),
      m_pAALService(NULL),
      m_StreamState(Stopped),
      m_bIsOK(false),
      m_pService(NULL),
      m_pIConsumer(NULL),
      m_pIProducer(NULL),
      m_pConsumerThread(NULL)
   {
      AutoLock(this);

      SetInterface(iidRuntimeClient, dynamic_cast<IRuntimeClient *>(this));
      SetInterface(iidServiceClient, dynamic_cast<IServiceClient *>(this));

      m_Sem.Create(0, 1);
      // Initialize the queues
      m_WSConsumerQueue.Initialize(INT_MAX);
      m_WSProducerQueue.Initialize(INT_MAX);
   }

   ~AppContext()
   {
      m_Sem.Destroy();
   }

   void Wait() { m_Sem.Wait(); }
   void Post() { m_Sem.Post(1); }

   IRuntime * Runtime() const
   {
      return m_pRuntime;
   }
   void Runtime(IRuntime *pRT)
   {
      AutoLock(this);
      m_pRuntime = pRT;
   }

   enumStreaming StreamState() const
   {
      return m_StreamState;
   }
   void StreamState(enumStreaming st)
   {
      AutoLock(this);
      m_StreamState = st;
   }

   btBool IsOK() const
   {
      return m_bIsOK;
   }
   void IsOK(btBool ok)
   {
      AutoLock(this);
      m_bIsOK = ok;
   }

   IAALService * Service() const
   {
      return m_pService;
   }
   void Service(IAALService *pAALService)
   {
      AutoLock(this);
      m_pService = pAALService;
   }

   ISampleAFU2Consumer * ConsumerAFU() const
   {
      return m_pIConsumer;
   }
   void ConsumerAFU(ISampleAFU2Consumer *pCons)
   {
      AutoLock(this);
      m_pIConsumer = pCons;
   }

   ISampleAFU2Producer * ProducerAFU() const
   {
      return m_pIProducer;
   }
   void ProducerAFU(ISampleAFU2Producer *pProd)
   {
      AutoLock(this);
      m_pIProducer = pProd;
   }

   OSLThread * ConsumerThread() const
   {
      return m_pConsumerThread;
   }
   void ConsumerThread(OSLThread *pThr)
   {
      AutoLock(this);
      m_pConsumerThread = pThr;
   }

   WSBufferQueue & ConsumerQ()
   {
      return m_WSConsumerQueue;
   }

   WSBufferQueue & ProducerQ()
   {
      return m_WSProducerQueue;
   }

   // <begin IRuntimeClient interface>
   virtual void runtimeStarted(IRuntime            *pRuntime,
                               const NamedValueSet &rConfigParms)
   {
      // Save a copy of our runtime interface instance.
      Runtime(pRuntime);
      IsOK(true);
      MSG("Runtime started");
      Post();
   }

   virtual void runtimeStopped(IRuntime *pRuntime)
   {
      MSG("Runtime stopped");
      Post();
   }

   virtual void runtimeStartFailed(const IEvent &rEvent)
   {
      IsOK(false);
      MSG("Runtime start failed");
      Post();
   }

   virtual void runtimeEvent(const IEvent &theEvent)
   {
   }
   void runtimeAllocateServiceFailed( IEvent const &rEvent)
   {
      MSG("Runtime AllocateService failed");
   }

   void runtimeAllocateServiceSucceeded(IBase               *pClient,
                                        TransactionID const &rTranID)
   {
      MSG("Runtime Allocate Service Succeeded");
   }

   virtual void serviceEvent(const IEvent &theEvent)
   {
      //=========================================
      // Use the AAL_IS_EXCEPTION macro to
      // efficiently check to see if the event
      // is derived from Exception. This is more
      // efficient than using the Has() method.
      //=========================================
      if ( AAL_IS_EXCEPTION(theEvent.SubClassID()) ) {
         // Print the description string.
         PrintExceptionDescription(theEvent);

         IsOK(false);
         return;
      }

      //====================================================================================================
      // Process the event  - For events it is typically desirable to perform an action based on the
      // event type that was generated. Every object implements one or more interfaces. The type of an event
      // is determined by the interface(s) it implements. In practice the SubClassID is used to determine
      // the type of an object, or in this case event. The SubClassID typically represents the most commonly
      // used interface of an object. As such the objects SubClass interface is cached in the object and the
      // ID is accessable through SubClassID().
      //====================================================================================================
      //============================
      // Switch on the event Type ID
      //============================
      switch ( theEvent.SubClassID() ) {

         // Consumer Events
         EVENT_CASE(tranevtSampleAFU2Register) {
            MSG("Consumer Registered");
            ConsumerThread()->Signal();
         } break;

         EVENT_CASE(tranevtSampleAFU2UnRegister) {
            MSG("Consumer UnRegistered");
            ConsumerThread()->Signal();
         } break;

         EVENT_CASE(tranevtSampleAFU2BufferFull) {
            // Cast the event to an IBufferFullTransactionEvent (which is its most derived class)
            // Then get the workspace descriptor from the BufferFull event and queue it on the producer queue
            // NOTE: The event defines a conversion operator that allows you to do this put. Refer to
            // IBufferFullTransactionEvent.
            // This rather involved line of code gets the ISampleAFU2BufferFull interface from the event
            // using dynamic_ref<> , it then uses the conversion operator (WSBufDesc) to extract a
            // WSDescBuf and then passes that to Put()
            ProducerQ().Put((WSBufDesc)dynamic_ref<IBufferFullTransactionEvent>(tranevtSampleAFU2BufferFull,
                                                                                theEvent));
         } break;

         // Producer Events
         EVENT_CASE(tranevtSampleAFU2Start) {
            MSG("Transform Started");
            Post();
         } break;

         EVENT_CASE(tranevtSampleAFU2Stop) {
            MSG("Transform Stopped");
            Post();
         } break;

         EVENT_CASE(tranevtSampleAFU2BufferEmpty) {
            //
            // This is a critical section during the final stage of the transaction
            // When the application stops the streaming it will continue to receive
            // empty buffers from the AFU until the AFU is stopped. The main thread
            // will return any buffers that have already been sent. To insure no
            // empties get queued between the time the stop is issued and the stop
            // completed (i.e. inflight events) we immediatly free buffers we receive.
            // The Streaming flag is used to indicate that we should discard buffers.
            // There is a race where the code has passed the check and the main thread
            // gets scheduled to run. If the main thread executes past the point where
            // it returns the buffers in the ConsumerQ then we could end up with an
            // orphaned buffer and never get the StopComplete.
            //
            Lock();        // Prevent Buffer empty events

            if ( Streaming == StreamState() ) {
               // Cast the event to an IBufferFullTransactionEvent (which is its most derived class)
               // Then get the workspace pointer from the BufferFull event and queue it on the producer queue

               //See the description of tranevtSampleAFU2BufferFull handling above for more detail on whats being
               // done here
               ConsumerQ().Put((WSBufDesc)dynamic_ref<IBufferEmptyTransactionEvent>(tranevtSampleAFU2BufferEmpty,
                                                                                    theEvent));
            } else {
               // Free the buffer immediately
               ProducerAFU()->FreeBuffer((WSBufDesc)dynamic_ref<IBufferEmptyTransactionEvent>(tranevtSampleAFU2BufferEmpty,
                                                                                              theEvent));
            }

            Unlock();

         } break;

         default: {
            ERR("unexpected event" << hex << theEvent.SubClassID() << dec);
            IsOK(false);
         } break;

      }

   }
   // <end IRuntimeClient interface>

   // <begin IServiceClient interface>
   virtual void serviceAllocated(IBase               *pServiceBase,
                                 TransactionID const &rTranID)
   {
      // Get the IAALService
      Service(dynamic_ptr<IAALService>(iidService, pServiceBase));
      ASSERT(NULL != Service());

      // Get the Producer
      ProducerAFU(dynamic_ptr<ISampleAFU2Producer>(iidSampleAFU2Producer, pServiceBase));
      ASSERT(NULL != ProducerAFU());

      // Get the Consumer
      ConsumerAFU(dynamic_ptr<ISampleAFU2Consumer>(iidSampleAFU2Consumer, pServiceBase));
      ASSERT(NULL != ConsumerAFU());

      MSG("Sample AFU 2 Allocated");

      Post();
   }

   virtual void serviceAllocateFailed(const IEvent        &rEvent)
   {
      IsOK(false);
      MSG("Failed to allocate a Sample AFU 2");
      Post();
   }

   virtual void serviceFreed(TransactionID const &rTranID)
   {
      MSG("Sample AFU 2 Freed");
      Post();
   }

private:
   IRuntime              *m_pRuntime;    // Locally-cached pointer to our XL runtime instance.
   IAALService           *m_pAALService; // The generic AAL Service interface for the AFU.
   enumStreaming          m_StreamState;
   btBool                 m_bIsOK;
   IAALService           *m_pService;
   ISampleAFU2Consumer   *m_pIConsumer;
   ISampleAFU2Producer   *m_pIProducer;
   OSLThread             *m_pConsumerThread;
   CSemaphore             m_Sem;
   WSBufferQueue          m_WSConsumerQueue;
   WSBufferQueue          m_WSProducerQueue;
};

#endif // __AALSAMPLE2_H__

