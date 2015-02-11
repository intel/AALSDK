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
/// @file SampleAFU1Service.cpp
/// @brief Implementation of SampleAFUPing - a Simple AFU Service.
/// @ingroup sample_afu1
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer Sample Application
///
///    This application is for example purposes only.
///    It is not intended to represent a model for developing commercially-deployable applications.
///    It is designed to show working examples of the AAL programming model and APIs.
///
/// AUTHOR: Joseph Grecco, Intel Corporation
///         Tim Whisonant, Intel Corporation
///         Sadruta Chandrashekar, Intel Corporation
///
/// This sample demonstrates how to create an AFU Service that is software only.
/// This model also works for AFUs that use proprietary hardware.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 08/17/2011     JG       Initial version@endverbatim
//****************************************************************************
#include <aalsdk/AAL.h>
#include <aalsdk/aas/AALServiceModule.h>
#include <aalsdk/osal/Sleep.h>

#include <aalsdk/AALLoggerExtern.h>              // Logger
#include <aalsdk/utils/AALEventUtilities.h>      // Used for UnWrapAndReThrow()

#include <aalsdk/aas/AALInProcServiceFactory.h>  // Defines InProc Service Factory

#include "SampleAFU1Service-internal.h"


//=============================================================================
// Typedefs and Constants
//=============================================================================


// The following declarations implement the AAL Service factory and entry
//  point.

// Define the factory to use for this service. In this example the service
//  will be implemented in-process.  There are other implementations available for
//  services implemented remotely, for example via TCP/IP.
#define SERVICE_FACTORY AAL::InProcSvcsFact< PingAFU >

#if defined ( __AAL_WINDOWS__ )
# pragma warning(push)
# pragma warning(disable : 4996) // destination of copy is unsafe
#endif // __AAL_WINDOWS__

AAL_BEGIN_SVC_MOD(SERVICE_FACTORY, libsampleafu1, SAMPLEAFU1_API, SAMPLEAFU1_VERSION, SAMPLEAFU1_VERSION_CURRENT, SAMPLEAFU1_VERSION_REVISION, SAMPLEAFU1_VERSION_AGE)
   /* No commands other than default, at the moment. */
AAL_END_SVC_MOD()

#if defined ( __AAL_WINDOWS__ )
# pragma warning(pop)
#endif // __AAL_WINDOWS__


/////////////////////////////////////////////////////////////////////////////
//////                                                                ///////
//////                                                                ///////
/////                         SAMPLE AFU 1                             //////
//////                                                                ///////
//////                                                                ///////
/////////////////////////////////////////////////////////////////////////////
//=============================================================================
// Name:
// Description:
// Interface: public
// Inputs:
// Outputs: none.
// Comments:
//=============================================================================
void PingAFU::init(TransactionID const &TranID)
{
   m_pPingClient = dynamic_ptr<ISampleAFUPingClient>(iidSampleAFUPingClient, ClientBase());
   ASSERT( NULL != m_pPingClient ); //QUEUE object failed
   QueueAASEvent(new ObjectCreatedEvent( getRuntimeClient(),
                                         Client(),
                                         dynamic_cast<IBase *>(this),
                                         TranID) );
}


//=============================================================================
// Name: WorkerThread
// Description: Performs the pings
// Interface: public
// Inputs: Message - Message to send
//         pTranID
// Outputs: none.
// Comments:
//=============================================================================
void PingAFU::WorkerThread(OSLThread *pThread,
                           void      *pContext)
{
   PingAFU *pThis = reinterpret_cast<PingAFU *>(pContext);

   ASSERT(NULL != pThis);
   if ( NULL == pThis ) {
      return;
   }

   int x;
   for ( x = 0 ; (x<5) && pThis->IsOK() ; ++x ) {

      if ( NULL == pThis->Client() ) {
         // No IServiceClient - process the event the old way.
         pThis->QueueAASEvent(new CTransactionEvent( (IBase *)pThis,
                                                     tranevtSampleAFUPing,
                                                     pThis->m_CurrTranID) );
      } else {
         pThis->SendMsg(new SampleAFUPingFunctor(pThis->m_pPingClient, (IBase *)pThis, pThis->m_CurrTranID));
      }

   }

}

//=============================================================================
// Name: SinglePingThread
// Description: Performs the pings
// Interface: public
// Inputs: Message - Message to send
//         pTranID
// Outputs: none.
// Comments:
//=============================================================================
void PingAFU::SinglePingThread(OSLThread *pThread,
                               void      *pContext)
{
   PingAFU *pThis = (PingAFU*) pContext;
   static btUnsigned64bitInt  n = 0;
          btUnsigned64bitInt id = 0;

   dynamic_cast<CriticalSection*>(pThis)->Lock();
   id = ++n;
   TransactionID tid(reinterpret_cast<btApplicationContext>(id));
   dynamic_cast<CriticalSection*>(pThis)->Unlock();

   pThis->SendMsg(new SampleAFUPingFunctor(pThis->m_pPingClient, (IBase *)pThis, pThis->m_CurrTranID));
}

//=============================================================================
// Name: Ping
// Description: Ping method send 5 replies
// Interface: public
// Inputs: Message - Message to send
//         pTranID
// Outputs: none.
// Comments:
//=============================================================================
void PingAFU::Ping(btcString sMessage, TransactionID const &rTranID)
{
   AutoLock(this);

   m_CurrTranID = rTranID;

   MSG("Received ping message '"<< sMessage << "'. Sending 5 replies.");

   m_pThread = new OSLThread(PingAFU::WorkerThread,
                             OSLThread::THREADPRIORITY_NORMAL,
                             this);

   // Wait for the thread to complete. This would not normally be done in
   // a real AFU as it effectively blocks the calling thread until the
   // the work is complete, defeating the purpose of asynchronous operation.
   // IT IS DONE THIS WAY ONLY TO SIMPLIFY THE DEMO.
   m_pThread->Join();
   delete m_pThread;
   m_pThread = NULL;
}

//=============================================================================
// Name: PingOne
// Description: Ping method sned 1 reply in same thread
// Interface: public
// Inputs: Message - Message to send
//         pTranID
// Outputs: none.
// Comments:
//=============================================================================
void PingAFU::PingOne(btcString sMessage, TransactionID &rTranID)
{
   AutoLock(this);
   //   unsigned int evID = (unsigned int)rTranID.Context();
   btUnsigned64bitInt evID = ( btUnsigned64bitInt ) rTranID.Context();
   try {
      QueueAASEvent( new CTransactionEvent((IBase *)this,
                                           tranevtSampleAFUPing,
                                           rTranID) );
   } catch( ... ) {
      cerr <<"BOOOM" <<endl;
   }
   cerr << "AFU sends Event #" << evID << endl;
}

//=============================================================================
// Name: PingSingleThread
// Description: Ping method sned n replies in same thread
// Interface: public
// Inputs: Message - Message to send
//         pTranID
// Outputs: none.
// Comments:
//=============================================================================
void PingAFU::PingSingleThread(btcString sMessage, unsigned int n)
{
   AutoLock(this);
   // Lock the object to prevent premature destruction
   btUnsigned64bitInt i;
   for ( i = 0; ( i < (btUnsigned64bitInt)n ) && this->IsOK() ; i++ ) {
      TransactionID tid(reinterpret_cast<btApplicationContext>(i + 1));
      QueueAASEvent(new CTransactionEvent((IBase *)this,
                                               tranevtSampleAFUPing,
                                               tid));

      cerr << "AFU sends Event #" << i + 1 << endl;
   }
}


//=============================================================================
// Name: PingMultiThread
// Description: Ping method send n replies in multiple threads
// Interface: public
// Inputs: Message - Message to send
//         pTranID
// Outputs: none.
// Comments:
//=============================================================================
void PingAFU::PingMultiThread(btcString sMessage, unsigned int n, btBool wait)
{
   m_ppThreads = new OSLThread *[n];
   unsigned int i;

   for ( i = 0 ; i < n ; ++i ) {
      if ( wait ) {
         SleepMicro( rand() % 1000 ); // 1 to 32 microseconds
      }
      cerr << "AFU sends Event #" << i + 1 << endl;
      m_ppThreads[i]= new OSLThread(PingAFU::SinglePingThread,
                                    OSLThread::THREADPRIORITY_NORMAL,
                                    this);
   }

   // Wait for the threads to complete. This would not normally be done in
   // a real AFU as it effectively blocks the calling thread until the
   // the work is complete, defeating the purpose of asynchronous operation.
   // IT IS DONE THIS WAY ONLY TO SIMPLIFY THE DEMO.

   // Brute force algorithm to wait for every thread to delete.
   // Does not consider order of thread completion.
   //  This is not an efficient method but simplifies the demo
   for ( i = 0 ; i < n ; ++i ) {
      m_ppThreads[i]->Join();
      delete m_ppThreads[i];
   }
   delete [] m_ppThreads;
   m_ppThreads=NULL;
}

btBool PingAFU::Release(TransactionID const &rTranID, btTime timeout)
{
   return ServiceBase::Release(rTranID, timeout);
}

btBool PingAFU::Release(btTime timeout)
{
   return ServiceBase::Release(timeout);
}


SampleAFUPingFunctor::SampleAFUPingFunctor(ISampleAFUPingClient *pPingClient,
                                           IBase                *pPingAFU,
                                           TransactionID const  &rTranID) :
   m_pSvcClient(pPingClient),
   m_pPingAFU(pPingAFU),
   m_TranID(rTranID)
{}

void SampleAFUPingFunctor::operator() ()
{
   m_pSvcClient->PingReceived(m_TranID);
   delete this;
}

