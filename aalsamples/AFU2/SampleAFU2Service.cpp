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
/// @file SampleAFU2Service.cpp
/// @brief Implementation of a Simple AFU Service.
/// @ingroup sample_afu2
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer Sample Application
///
///    This application is for example purposes only.
///    It is not intended to represent a model for developing commercially-deployable applications.
///    It is designed to show working examples of the AAL programming model and APIs.
///
/// AUTHOR: Joseph Grecco, Intel Corporation
///
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 10/06/2011     JG       Initial version.@endverbatim
//****************************************************************************
#include <sstream>
#include <stdlib.h>                              // for rand()

#include <aalsdk/AAL.h>
#include <aalsdk/aas/AALServiceModule.h>
#include <aalsdk/osal/Sleep.h>

#include <aalsdk/AALLoggerExtern.h>              // Logger
#include <aalsdk/utils/AALEventUtilities.h>      // Used for UnWrapAndReThrow()

#include <aalsdk/aas/AALInProcServiceFactory.h>  // Defines InProc Service Factory

#include "SampleAFU2Service-internal.h"

//=============================================================================
// Typedefs and Constants
//=============================================================================


// The following declarations implement the AAL Service factory and entry
//  point.

// Define the factory to use for this service. In this example the service
//  will be implemented in-process.  There are other implementations available for
//  services implemented remotely, for example via TCP/IP.
#define SERVICE_FACTORY InProcSvcsFact< SampleAFU2 >

#if defined ( __AAL_WINDOWS__ )
# pragma warning(push)
# pragma warning(disable : 4996) // destination of copy is unsafe
#endif // __AAL_WINDOWS__

AAL_BEGIN_SVC_MOD(SERVICE_FACTORY, libsampleafu2, SAMPLEAFU2_API, SAMPLEAFU2_VERSION, SAMPLEAFU2_VERSION_CURRENT, SAMPLEAFU2_VERSION_REVISION, SAMPLEAFU2_VERSION_AGE)
   /* No commands other than default, at the moment. */
AAL_END_SVC_MOD()

#if defined ( __AAL_WINDOWS__ )
# pragma warning(pop)
#endif // __AAL_WINDOWS__

/////////////////////////////////////////////////////////////////////////////
//////                                                                ///////
//////                                                                ///////
/////                         SAMPLE AFU 2                             //////
//////                                                                ///////
//////                                                                ///////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

//=============================================================================
// Name: CBufferFullTransactionEvent
// Description: CBufferFullTransactionEvent
// IID: tranevtSampleAFU2BufferFull
//=============================================================================
class CBufferFullTransactionEvent : public CTransactionEvent,
                                    public IBufferFullTransactionEvent
{
public:
CBufferFullTransactionEvent(IBase               *pObject,
                            btVirtAddr           WorkSpaceAddress,
                            btWSSize             WorkSpaceSize,
                            TransactionID const &TranID) :
   CTransactionEvent(pObject, TranID),
   m_WorkSpaceAddress(WorkSpaceAddress),
   m_WorkSpaceSize(WorkSpaceSize)
{
   SetSubClassInterface(tranevtSampleAFU2BufferFull,
                        dynamic_cast<IBufferFullTransactionEvent *>(this));
}

virtual btVirtAddr WorkSpaceAddress() { return m_WorkSpaceAddress; }
virtual btWSSize      WorkSpaceSize() { return m_WorkSpaceSize;    }

protected:
   btVirtAddr m_WorkSpaceAddress;
   btWSSize   m_WorkSpaceSize;
};

//=============================================================================
// Name: CBufferEmptyTransactionEvent
// Description: CBufferEmptyTransactionEvent
// IID: tranevtSampleAFU2BufferEmpty
//=============================================================================
class CBufferEmptyTransactionEvent : public CTransactionEvent,
                                     public IBufferEmptyTransactionEvent
{
public:
CBufferEmptyTransactionEvent(IBase               *pObject,
                             btVirtAddr           WorkSpaceAddress,
                             btWSSize             WorkSpaceSize,
                             TransactionID const &TranID) :
   CTransactionEvent(pObject, TranID),
   m_WorkSpaceAddress(WorkSpaceAddress),
   m_WorkSpaceSize(WorkSpaceSize)
{
   SetSubClassInterface(tranevtSampleAFU2BufferEmpty,
                        dynamic_cast<IBufferEmptyTransactionEvent *>(this));
}

btVirtAddr WorkSpaceAddress() { return m_WorkSpaceAddress; }
btWSSize      WorkSpaceSize() { return m_WorkSpaceSize;    }

protected:
   btVirtAddr m_WorkSpaceAddress;
   btWSSize   m_WorkSpaceSize;
};


//=============================================================================
// Name:
// Description:
// Interface: public
// Inputs:
// Outputs: none.
// Comments:
//=============================================================================
void SampleAFU2::init(TransactionID const &rtid)
{

   //---------------------------------------------------------
   // Allocate the internal workspace
   // Buffers.
   // NOTE: OptArgs could be used here to override defaults.
   // The optargs would be passed to IFactory::Create()
   //---------------------------------------------------------
   m_EmptyBuffers.Initialize(m_NumBuffers + 1);   // One extra to accomodate termination protocol
   m_FullBuffers.Initialize(m_NumBuffers + 1);
   if ( !AllocateWorkSpaceQueue(m_EmptyBuffers, m_WSSize, m_NumBuffers) ) {
      return;
   }

   //---------------------------------------
   // Launch the Producer. It always runs.
   //---------------------------------------
   m_pProducerThread = new OSLThread(ProducerThread,
                                     OSLThread::THREADPRIORITY_NORMAL,
                                     this);
   // Wait for the Consumer to be ready
   m_pProducerThread->Wait();

   m_bIsOK = true;

   QueueAASEvent( new ObjectCreatedEvent( getRuntimeClient(),
                                          Client(),
                                          dynamic_cast<IBase *>(this), rtid) );
}



//=============================================================================
// Name: AllocateWorkSpaceQueue
// Description: Allocates internal workspace
// Interface: private
// Inputs: BufferQ - Buffer queue to fill with buffers
//         WSSize - Size of each buffer
//         NumBuffers.- Number of buffers in the queue
// Outputs: none.
// Comments: This routine would be replaced with an IPIP version on a HW
//           based implementation. To that end it returns a status.
//=============================================================================

btBool SampleAFU2::AllocateWorkSpaceQueue(WSBufferQueue     &BufferQ,
                                          btWSSize           WSSize,
                                          btUnsigned32bitInt NumBuffers)
{
   btUnsigned32bitInt cnt;
   for ( cnt = 0 ; cnt < NumBuffers ; ++cnt ) {
      WSBufDesc bufDesc((btVirtAddr)(new(std::nothrow) btByte[WSSize]), WSSize);
      BufferQ.Put(bufDesc);
   }
   return true;
}

//=============================================================================
// Name: FreeWorkSpaceQueue
// Description: Free the buffers in the queue
// Interface: private
// Inputs: BufferQ - Buffer queue to fill with buffers
// Outputs: none.
// Comments: This routine would be replaced with an IPIP version on a HW
//           based implementation. To that end it returns a status.
//=============================================================================
btBool SampleAFU2::FreeWorkSpaceQueue(WSBufferQueue &BufferQ)
{
   size_t QSize = BufferQ.Size();
   size_t cnt   = 0;

   for( cnt = 0 ; cnt < QSize ; ++cnt ) {
      WSBufDesc bufDesc = BufferQ.GetNext();

      if ( NULL != bufDesc.pBuf ) {
         delete[] bufDesc.pBuf;
      }
   }

   return true;
}

//=============================================================================
// Name:          Release
// Description:   Release our resources
// Interface:     public
// Inputs:        rTrnaID - transactionID
//                timeout - maximum time to take
// Comments:      Should call ServiceDevice::Release when through with
//                class specifc cleanup
//=============================================================================
btBool SampleAFU2::Release(TransactionID const &rTranID, btTime timeout)
{
   // Stop the resource
   Halt();

   // MUST call our parent class Release()
   return ServiceBase::Release(rTranID, timeout);
}

//=============================================================================
// Name:          Release
// Description:   Release our resources
// Interface:     public
// Inputs:        rTrnaID - transactionID
//                timeout - maximum time to take
// Comments:      Should call ServiceDevice::Release when through with
//                class specifc cleanup
//=============================================================================
btBool SampleAFU2::Release(btTime timeout)
{
   // Stop the resource
   Halt();

   // MUST call our parent class Release()
   return ServiceBase::Release(timeout);
}

//=============================================================================
// Name:          Halt
// Description:   Stop our resources
// Interface:     public
// Comments:
//=============================================================================
void SampleAFU2::Halt()
{
   // Mark the AFU as no longer ready
    m_bIsOK = false;

    // Kill the Producer thread.
    //  NULL while in destroying state wakes it up and tells it to terminate
    m_State = Destroying;
    WSBufDesc temp;
    temp.pBuf = NULL;
    m_FullBuffers.Put(temp);

    //Wait for it to end
    m_pProducerThread->Join();
    delete m_pProducerThread;
    m_pProducerThread = NULL;
    FreeWorkSpaceQueue(m_EmptyBuffers);

    if ( m_pConsumerTransactionID != NULL) {
       delete m_pConsumerTransactionID;
       m_pConsumerTransactionID = NULL;
    }
}

//=============================================================================
// Name: ProducerThread
// Description: Sends full buffers messages
// Interface: public
// Inputs:  pThread - The thread object
//          pContext - pointer to SampleAFU2 instance
// Outputs: none.
// Comments:
//=============================================================================
void SampleAFU2::ProducerThread(OSLThread *pThread,
                                void      *pContext)
{
   SampleAFU2 *This = static_cast<SampleAFU2 *>(pContext);

   // Wake up whomever created us.
   pThread->Signal();

   // Execute the producer
   This->Producer();
}

//=============================================================================
// Name: Producer
// Description: Feeds dispatches full buffers to the application
// Interface: public
// Inputs: none
// Outputs: none.
// Comments:
//=============================================================================
void SampleAFU2::Producer()
{
   // Loop continuously generating buffer Full events
   while ( true ) {
      WSBufDesc CurrBuffer = m_FullBuffers.GetNext();

      // While we don't get a NULL buffer pointer, produce.
      if ( NULL == CurrBuffer.pBuf ) {
         // If the AFU is destroying, then time to exit.
         if ( Destroying == m_State ) {
            return;
         }

         SendMsg( new SampleAFU2ExceptionFunctor(Client(),
                                                 static_cast<IBase *>(this),
                                                 extranevtSampleAFU2PutData,
                                                 TransactionID(),
                                                 errMethodFailure,
                                                 reasInternalError,
                                                 "SampleAFU2::Producer: Unexpected Null buffer"
                                                ) );

         // Otherwise continue waiting
         continue;
      }

      // Encrypt the data
      btUnsigned32bitInt x;
      for ( x = 0 ; x < CurrBuffer.size ; ++x ) {
         CurrBuffer.pBuf[x] ^= m_CryptKey;
      }

      // Send the buffer on its way
      // Use the Consumer tranID if one has been registered, otherwise use the Producer.
      SendMsg( new SampleAFU2BufferFunctor(Client(),
                                           static_cast<IBase *>(this),
                                           tranevtSampleAFU2BufferFull,
                                           (NULL == m_pConsumerTransactionID ) ? m_ProducerTransactionID : *m_pConsumerTransactionID,
                                           CurrBuffer.pBuf,
                                           CurrBuffer.size) );
   }

}


//=============================================================================
// Name: ConsumerThread
// Description: Consumer thread. Provides thread to Consumer. Wrapper function.
// Interface: public
// Inputs: pThread - The thread object
//         pContext - pointer to SampleAFU2 instance
// Outputs: none.
// Comments:
//=============================================================================
void SampleAFU2::ConsumerThread(OSLThread *pThread,
                                void      *pContext)
{
   SampleAFU2 *This = static_cast<SampleAFU2 *>(pContext);

   // Wake up main
   pThread->Signal();

   // Execute the consumer
   This->Consumer();
}

//=============================================================================
// Name: Consumer
// Description: Consumer Proceedure. Feeds empty buffers to the application
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments:
//=============================================================================
void SampleAFU2::Consumer()
{
   // Loop continuously generating buffer empty events
   while ( true ) {
      WSBufDesc CurrBuffer;

      // While we don't get a NULL buffer pointer, produce.
      CurrBuffer = m_EmptyBuffers.GetNext();
      if ( NULL == CurrBuffer.pBuf ) {
         // NULL is sent when the Transaction is complete.
         return;
      }

      // Send the buffer on its way.
      SendMsg( new SampleAFU2BufferFunctor(Client(),
                                           static_cast<IBase *>(this),
                                           tranevtSampleAFU2BufferEmpty,
                                           m_ProducerTransactionID,
                                           CurrBuffer.pBuf,
                                           CurrBuffer.size) );
   }
}

//---------------------------------------------------------
// ISampleAFU2Producer
//---------------------------------------------------------
//=============================================================================
// Name: RegisterConsumer
// Description: Register the consumer transaction ID
// Interface: public
// Inputs: WorkspaceID
//         pTranID
// Outputs: none.
// Comments: This design pattern demonstrates how the AFU designer can
//           implement a sophisticated interface consisting of seperate
//           callbacks to the application using standard EDS and TransactionIDs
//           In this sample we allow the application to register an event
//           handler used specifically for being sent buffer events.
//=============================================================================
void SampleAFU2::RegisterConsumer(TransactionID const &rConsumerID,
                                  TransactionID const &rTranID)
{
   AutoLock(this);

   // Make sure the AFU is not active and there is not already a handler.
   if ( ( Idle != m_State ) || ( NULL != m_pConsumerTransactionID ) ) {
      SendMsg( new SampleAFU2ExceptionFunctor(Client(),
                                              static_cast<IBase *>(this),
                                              extranevtSampleAFU2Register,
                                              rTranID,
                                              errMethodFailure,
                                              reasInvalidState,
                                              strInvalidState
                                             ) );
      return;
   }

   m_pConsumerTransactionID = new TransactionID(rConsumerID);

   SendMsg( new SampleAFU2TransactionFunctor(Client(),
                                             static_cast<IBase *>(this),
                                             tranevtSampleAFU2Register,
                                             rTranID) );
}


//=============================================================================
// Name: UnRegisterConsumer
// Description: Unregister the Consumer transaction ID
// Interface: public
// Inputs: TranID - Transaction ID for this command
// Outputs: none.
// Comments:
//=============================================================================
void SampleAFU2::UnRegisterConsumer(TransactionID const &rTranID)
{
   AutoLock(this);

   if ( NULL == m_pConsumerTransactionID ) {
      SendMsg( new SampleAFU2ExceptionFunctor(Client(),
                                              static_cast<IBase *>(this),
                                              extranevtSampleAFU2UnRegister,
                                              rTranID,
                                              errMethodFailure,
                                              reasInvalidState,
                                              strInvalidState
                                             ) );
      return;
   }

   delete m_pConsumerTransactionID;
   m_pConsumerTransactionID = NULL;

   SendMsg( new SampleAFU2TransactionFunctor(Client(),
                                             static_cast<IBase *>(this),
                                             tranevtSampleAFU2UnRegister,
                                             rTranID) );
}

//=============================================================================
// Name: FreeBuffer
// Description: Free an empty buffer.
// Interface: public
// Inputs: Workspace
// Outputs: none.
// Comments:
//=============================================================================
void SampleAFU2::FreeBuffer(WSBufDesc WSBuffer)
{
   AutoLock(this);

   // Put the buffer on the empty buffer Queue. If the buffer is NULL indicating
   // the transaction is done then it will be removed by the conusmer thread.
   // Empty buffers will no longer be sent to the app
   m_EmptyBuffers.Put(WSBuffer);

   // If we are stopped and the last buffer is freed then generate event
   if ( Running != m_State ) {
      // Wait for the consumer to die and clean-up
      if ( NULL != m_pConsumerThread ) {
         m_pConsumerThread->Join();
         delete m_pConsumerThread;
         m_pConsumerThread = NULL;
      }

      if ( m_EmptyBuffers.Size() == m_NumBuffers ) {

         // Send the stop event
         SendMsg( new SampleAFU2TransactionFunctor(Client(),
                                                   static_cast<IBase *>(this),
                                                   tranevtSampleAFU2Stop,
                                                   m_ProducerTransactionID) );

         m_State = Idle;
      }

   }

}

//---------------------------------------------------------
// ISampleAFU2Consumer
//---------------------------------------------------------
//=============================================================================
// Name: Start
// Description: Start transcoding
// Interface: public
// Inputs:
// Outputs: none.
// Comments:
//=============================================================================
void SampleAFU2::Start(TransactionID const &rTranID, btByte CryptKey)
{
   AutoLock(this);   // Prevent re-entrance until started

   //-------------------------------------
   // Check to make sure we are not active
   //------------------------------------
   if ( Idle != m_State ) {
      SendMsg( new SampleAFU2ExceptionFunctor(Client(),
                                              static_cast<IBase *>(this),
                                              extranevtSampleAFU2Start,
                                              rTranID,
                                              errMethodFailure,
                                              reasDeviceBusy,
                                              strDeviceBusy
                                             ) );
      return;
   }

   //---------------------------------------
   // Launch the Consumer AFU buffer manager
   //---------------------------------------
   m_CryptKey = CryptKey;
   if ( NULL == m_pConsumerThread ) {

      // Save the producer tranID for sending Consumer events
      m_ProducerTransactionID = rTranID;

      //Change state
      m_State = Running;

      // Return the started event
      SendMsg( new SampleAFU2TransactionFunctor(Client(),
                                                static_cast<IBase *>(this),
                                                tranevtSampleAFU2Start,
                                                rTranID) );

      m_pConsumerThread = new OSLThread(SampleAFU2::ConsumerThread,
                                        OSLThread::THREADPRIORITY_NORMAL,
                                        this);
      // Wait for Consumer to start
      m_pConsumerThread->Wait();

   } else {
      SendMsg( new SampleAFU2ExceptionFunctor(Client(),
                                              static_cast<IBase *>(this),
                                              extranevtSampleAFU2Start,
                                              rTranID,
                                              errInternal,
                                              reasInvalidState,
                                              strInvalidState
                                             ) );
   }

}

//=============================================================================
// Name: Stop
// Description: stop transcoding
// Interface: public
// Inputs: TranID
// Outputs: none.
// Comments:
//=============================================================================
void SampleAFU2::Stop(TransactionID const &rTranID)
{
   AutoLock(this);
   //
   // Check to make sure we are active
   //
   if ( Stopping == m_State ) {
      SendMsg( new SampleAFU2ExceptionFunctor(Client(),
                                              static_cast<IBase *>(this),
                                              extranevtSampleAFU2Stop,
                                              rTranID,
                                              errMethodFailure,
                                              reasInvalidState,
                                              strInvalidState
                                             ) );
      return;
   }

   // The actual stop will occur in the FreeBuffer when the last buffer is returned
   m_State = Stopping;

   //-----------------------------------------------------------------------------
   // Calling FreeBuffer with NULL does a number of things. It signals to the
   // Consumer thread to terminate. If there are no outstanding buffers to free,
   // then this will also kick off the termination of the transaction.
   //-----------------------------------------------------------------------------
   FreeBuffer(WSBufDesc()); // Causes Consumer to shutdown and transaction to complete
}

//=============================================================================
// Name: PutBuffer
// Description: Put a full buffer into the work queue
// Interface: public
// Inputs: Workspace pointer
// Outputs: none.
// Comments:
//=============================================================================
void SampleAFU2::PutBuffer(WSBufDesc WSBuffer)
{
   if ( Running == m_State ) {
      m_FullBuffers.Put(WSBuffer);
   } else {
      FreeBuffer(WSBuffer);
   }
}



SampleAFU2ExceptionFunctor::SampleAFU2ExceptionFunctor(IServiceClient      *pSvcClient,
                                                       IBase               *pAFU,
                                                       btID                 SubclassID,
                                                       TransactionID const &TranID,
                                                       btID                 ExID,
                                                       btID                 Reason,
                                                       btcString            Descr) :
   m_pSvcClient(pSvcClient),
   m_pAFU(pAFU),
   m_SubclassID(SubclassID),
   m_TranID(TranID),
   m_ExID(ExID),
   m_Reason(Reason),
   m_Descr(Descr)
{}

void SampleAFU2ExceptionFunctor::operator() ()
{
   m_pSvcClient->serviceEvent( CExceptionTransactionEvent(m_pAFU,
                                                          m_SubclassID,
                                                          m_TranID,
                                                          m_ExID,
                                                          m_Reason,
                                                          m_Descr) );
   delete this;
}

SampleAFU2BufferFunctor::SampleAFU2BufferFunctor(IServiceClient      *pSvcClient,
                                                 IBase               *pAFU,
                                                 btID                 SubclassID,
                                                 TransactionID const &TranID,
                                                 btVirtAddr           pBuf,
                                                 btWSSize             Bytes) :
   m_pSvcClient(pSvcClient),
   m_pAFU(pAFU),
   m_SubclassID(SubclassID),
   m_TranID(TranID),
   m_pBuf(pBuf),
   m_Bytes(Bytes)
{}

void SampleAFU2BufferFunctor::operator() ()
{
   if ( tranevtSampleAFU2BufferFull == m_SubclassID ) {
      m_pSvcClient->serviceEvent( CBufferFullTransactionEvent(m_pAFU, m_pBuf, m_Bytes, m_TranID) );
   } else if ( tranevtSampleAFU2BufferEmpty == m_SubclassID ) {
      m_pSvcClient->serviceEvent( CBufferEmptyTransactionEvent(m_pAFU, m_pBuf, m_Bytes, m_TranID) );
   }
   delete this;
}

SampleAFU2TransactionFunctor::SampleAFU2TransactionFunctor(IServiceClient      *pSvcClient,
                                                           IBase               *pAFU,
                                                           btID                 SubclassID,
                                                           TransactionID const &TranID) :
   m_pSvcClient(pSvcClient),
   m_pAFU(pAFU),
   m_SubclassID(SubclassID),
   m_TranID(TranID)
{}

void SampleAFU2TransactionFunctor::operator() ()
{
   m_pSvcClient->serviceEvent( CTransactionEvent(m_pAFU, m_SubclassID, m_TranID) );
   delete this;
}


