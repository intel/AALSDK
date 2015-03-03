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
/// @file SampleAFU2Service-internal.h
/// @brief Definitions for Sample AFU 2 Service.
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
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 10/06/2011     JG       Initial Version@endverbatim
//****************************************************************************
#ifndef __SAMPLEAFU2SERVICE_INT_H__
#define __SAMPLEAFU2SERVICE_INT_H__
#include "SampleAFU2Service.h" // Public AFU device interface
#include <aalsdk/aas/AALService.h>

using namespace AAL;

//=============================================================================
// Name: SampleAFU2
// Description: Procuder/Consumer Sample AFU
//=============================================================================
class SampleAFU2 : public ServiceBase,
                   public ISampleAFU2Producer,
                   public ISampleAFU2Consumer

{
   enum enumStates
   {
      Idle,
      Running,
      Stopping,
      Destroying
   };
   static const btUnsignedInt constDefaultQueueSize  = 4;
   static const btUnsignedInt constDefaultBufferSize = 0x1000;

public:
   // Macro defines the constructor for a loadable AAL service.
   //  The first argument is your class name, the second argument is the
   //  name of the Service base class this service is derived from. In this
   //  example we use DeviceServiceBase as it is the class that provides the
   //  support for devices.  Software only services might use ServiceBase
   //  instead.
   //
   // Note that initializers can be declared here but are preceded by a comma
   //  rather than a colon.
   //
   // The design pattern is that the constructor do minimal work. Here we are
   //  registering the interfaces the service implements. The default (Subclass)
   //  interface is IEncode.  DeviceServiceBase provides an init() method that
   //  can be used where more sophisticated initialization is required. The
   //  init() method is called by the factory AFTER construction but before use.
   DECLARE_AAL_SERVICE_CONSTRUCTOR(SampleAFU2, ServiceBase),
      m_pConsumerTransactionID(NULL),
      m_CryptKey(0),
      m_WSSize(constDefaultBufferSize),
      m_NumBuffers(constDefaultQueueSize),
      m_pConsumerThread(NULL),
      m_pProducerThread(NULL),
      m_State(Idle)
   {
      //--------------------------------------------------------------------------
      // This sample demonstrates and AFU that presents two distinct service
      // interfaces. The Producer and consumer interfaces
      // The choice of which to make the SubClassInterface is arbitrary
      //--------------------------------------------------------------------------
      SetInterface(iidSampleAFU2Producer,
                   dynamic_cast<ISampleAFU2Producer *>(this));

      SetSubClassInterface(iidSampleAFU2Consumer,
                           dynamic_cast<ISampleAFU2Consumer *>(this));
   }

   // Hook to allow the object to initialize
   void init(TransactionID const &rtid);

protected:

   // Called when the service is released
   btBool Release(TransactionID const &rTranID, btTime timeout=AAL_INFINITE_WAIT);

   // Quiet Release. Used when Service is unloaded.
   btBool Release(btTime timeout=AAL_INFINITE_WAIT);

   void Halt();

   //---------------------------------------------------------
   // ISampleAFU2Producer
   //---------------------------------------------------------
   void RegisterConsumer(TransactionID const &rConsumerID,
                         TransactionID const &rTranID);
   void UnRegisterConsumer(TransactionID const &rTranID);

   void FreeBuffer(WSBufDesc WSBuffer);

   //---------------------------------------------------------
   // ISampleAFU2Consumer
   //---------------------------------------------------------
   void Start(TransactionID const &rTranID,
              btByte               CryptKey);

   void Stop(TransactionID const &rTranID);

   void PutBuffer(WSBufDesc WSBuffer);

   // ---------------------------------------------------------
   // Internal methods
   //----------------------------------------------------------

   // Producer and consumer functions
   void Consumer();              // Consumer procedure
   void Producer();              // Producer procedure
   static void ConsumerThread(OSLThread *pThread, void *pContext);
   static void ProducerThread(OSLThread *pThread, void *pContext);

   //Internal Workspace management functions
   btBool AllocateWorkSpaceQueue(WSBufferQueue     &BufferQ,
                                 btWSSize           WSSize,
                                 btUnsigned32bitInt NumBuffers);
   btBool FreeWorkSpaceQueue(WSBufferQueue &BufferQ);

private:
   TransactionID           *m_pConsumerTransactionID;
   TransactionID            m_ProducerTransactionID;
   btByte                   m_CryptKey;
   btWSSize                 m_WSSize;
   btUnsigned32bitInt       m_NumBuffers;
   TransactionID            m_CurrTranID;
   OSLThread               *m_pThread;
   OSLThread               *m_pConsumerThread;
   OSLThread               *m_pProducerThread;
   enumStates               m_State;
   WSBufferQueue            m_EmptyBuffers;
   WSBufferQueue            m_FullBuffers;
}; // SampleAFU2


// XL-aware event interfaces.
class SampleAFU2ExceptionFunctor : public IDispatchable
{
public:
   SampleAFU2ExceptionFunctor(IServiceClient      *pSvcClient, // recipient
                              IBase               *pAFU,       // sender
                              btID                 SubclassID, // event subclass ID
                              TransactionID const &TranID,     // orig tid
                              btID                 ExID,       // exception ID
                              btID                 Reason,     // reason code
                              btcString            Descr       // error description
                             );
   virtual void operator() ();

protected:
   IServiceClient      *m_pSvcClient;
   IBase               *m_pAFU;
   btID                 m_SubclassID;
   TransactionID const &m_TranID;
   btID                 m_ExID;
   btID                 m_Reason;
   btcString            m_Descr;
};

class SampleAFU2BufferFunctor : public IDispatchable
{
public:
   SampleAFU2BufferFunctor(IServiceClient      *pSvcClient, // recipient
                           IBase               *pAFU,       // sender
                           btID                 SubclassID, // event subclass ID
                           TransactionID const &TranID,     // orig tid
                           btVirtAddr           pBuf,       // buffer address
                           btWSSize             Bytes       // buffer size
                          );
   virtual void operator() ();

protected:
   IServiceClient      *m_pSvcClient;
   IBase               *m_pAFU;
   btID                 m_SubclassID;
   TransactionID const &m_TranID;
   btVirtAddr           m_pBuf;
   btWSSize             m_Bytes;
};

class SampleAFU2TransactionFunctor : public IDispatchable
{
public:
   SampleAFU2TransactionFunctor(IServiceClient      *pSvcClient, // recipient
                                IBase               *pAFU,       // sender
                                btID                 SubclassID, // event subclass ID
                                TransactionID const &TranID      // orig tid
                               );
   virtual void operator() ();

protected:
   IServiceClient      *m_pSvcClient;
   IBase               *m_pAFU;
   btID                 m_SubclassID;
   TransactionID const &m_TranID;
};

#endif // __SAMPLEAFU2SERVICE_INT_H__

