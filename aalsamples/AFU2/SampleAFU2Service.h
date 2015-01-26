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
/// @file SampleAFU2Service.h
/// @brief ISampleAFU2Producer, ISampleAFU2Consumer, and related interfaces.
/// @ingroup sample_afu2
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
/// 10/06/2011      JG      Based on original samples from AAL SDK version 2.20@endverbatim
//****************************************************************************
#ifndef __AALSDK_SERVICE_SAMPLEAFU2SERVICE_H__
#define __AALSDK_SERVICE_SAMPLEAFU2SERVICE_H__
#include <aalsdk/AAL.h>

const std::string strConfigRecord("9 20 ConfigRecordIncluded\n \
      \t10\n \
          \t\t9 17 ServiceExecutable\n \
            \t\t\t9 13 libsampleafu2\n \
         \t\t9 18 _CreateSoftService\n \
         \t\t0 1\n \
   9 29 ---- End of embedded NVS ----\n \
      9999\n");


#ifndef SAMPLEAFU2_VERSION_CURRENT
# define SAMPLEAFU2_VERSION_CURRENT  0
#endif // SAMPLEAFU2_VERSION_CURRENT
#ifndef SAMPLEAFU2_VERSION_REVISION
# define SAMPLEAFU2_VERSION_REVISION 0
#endif // SAMPLEAFU2_VERSION_REVISION
#ifndef SAMPLEAFU2_VERSION_AGE
# define SAMPLEAFU2_VERSION_AGE      0
#endif // SAMPLEAFU2_VERSION_AGE
#ifndef SAMPLEAFU2_VERSION
# define SAMPLEAFU2_VERSION          "0.0.0"
#endif // SAMPLEAFU2_VERSION

#if defined ( __AAL_WINDOWS__ )
# ifdef SAMPLEAFU2_EXPORTS
#    define SAMPLEAFU2_API __declspec(dllexport)
# else
#    define SAMPLEAFU2_API __declspec(dllimport)
# endif // SAMPLEAFUw_EXPORTS
#else
# ifndef __declspec
#    define __declspec(x)
# endif // __declspec
# define SAMPLEAFU2_API    __declspec(0)
#endif // __AAL_WINDOWS__

#define SAMPLEAFU2_SVC_MOD         "libsampleafu2" AAL_SVC_MOD_EXT
#define SAMPLEAFU2_SVC_ENTRY_POINT "libsampleafu2" AAL_SVC_MOD_ENTRY_SUFFIX

AAL_DECLARE_MOD(libsampleafu2, SAMPLEAFU2_API)


BEGIN_NAMESPACE(AAL)

/// @addtogroup sample_afu2
/// @{

//===============================================================================
//                                   AALSample2
//===============================================================================

/// @brief Struct used to store buffer information
struct WSBufDesc
{
   WSBufDesc() :
      pBuf(NULL),
      size(0)
   {}

   WSBufDesc(btVirtAddr buf, btWSSize sz) :
      pBuf(buf),
      size(sz)
   {}

   btVirtAddr pBuf; ///< Pointer to start of buffer.
   btWSSize   size; ///< Buffer size in bytes.
};

/// @brief Queue holding Workspace output buffers.
///
/// Uses semaphore and critical section objects from the AAL OS
//  abstraction layer library (OSAL).
class WSBufferQueue : public CriticalSection
{
public:
   /// WSBufferQueue Constructor
   WSBufferQueue() {}

   /// Initializes the queue to contain Size buffers.
   ///
   /// @param[in]  Size  Initial number of buffers in the queue.
   void Initialize(btUnsignedInt Size)
   {
      m_Semaphore.Create(0, Size);
   }

   /// WSBufferQueue Destructor
   ~WSBufferQueue()
   {
      AutoLock(this);
      size_t x = m_Queue.size();
      while ( x > 0 ) {
         --x;
         m_Semaphore.Wait(); // Clear the semaphore
      }
      m_Queue.empty();
   }

   /// Gets the next pointer, waiting for one to become ready if none exists.
   ///
   /// @retval Pointer to buffer.
   WSBufDesc GetNext()
   {
      // Wait for an buffer to be posted
      m_Semaphore.Wait();
      Lock();
      WSBufDesc pQItem = m_Queue.front();
      m_Queue.pop();
      Unlock();
      return pQItem;
   }

   /// Places a buffer pointer on the queue.
   ///
   /// @param[in]  bufDesc  Buffer to enqueue.
   void Put(WSBufDesc bufDesc)
   {
      AutoLock(this);   // Locks the critical section. Unlocks on exit.
      m_Queue.push(bufDesc);
      m_Semaphore.Post(1);
   }

   /// Retrieve the number of queued buffers.
   size_t Size()
   {
      AutoLock(this);
      return m_Queue.size();
   }

private:
   std::queue<WSBufDesc> m_Queue;
   CSemaphore            m_Semaphore;
};

/// ISampleAFU2Consumer interface ID.
#define iidSampleAFU2Consumer __INTC_IID(INTC_sysSampleAFU, 0x0002)

/// AFU's Consumer Interface used by the data producer.
class ISampleAFU2Consumer
{
public:
   /// @brief Start requesting data
   ///
   /// Sends tranevtSampleAFU2Start when started.
   ///
   /// @param[in]  rTranID   For outbound events.
   /// @param[in]  CryptKey  Key for buffer cipher.
   virtual  void Start(AAL::TransactionID const &rTranID,
                       btByte                    CryptKey) = 0;

   /// @brief Stop requesting data
   ///
   /// Sends tranevtSampleAFU2Stop when stopped.
   ///
   /// @param[in]  rTranID   For outbound events.
   virtual void Stop(AAL::TransactionID const &rTranID) = 0;

   /// @brief Put a filled workspace buffer for consumption.
   ///
   /// This function is one-way unless failed.
   ///
   /// @param[in]  WSBuffer   Buffer to put.
   virtual void PutBuffer(WSBufDesc WSBuffer) = 0;

   /// ISampleAFU2Consumer Destructor
   virtual ~ISampleAFU2Consumer() {}
};

/// ISampleAFU2Producer interface ID.
#define iidSampleAFU2Producer __INTC_IID(INTC_sysSampleAFU, 0x0003)

/// AFU's Producer Interface used by the data consumer.
class ISampleAFU2Producer
{
public:

   /// Allows the App. to register a special transaction ID for consumer
   /// events. This allows the App to assign a unique ID or more typically a
   /// specific handler for these events. Sends tranevtSampleAFU2Register.
   ///
   /// @param[in]  rConsumerID   The TransactionID to register.
   /// @param[in]  rTranID       For outbound events.
   virtual void RegisterConsumer(AAL::TransactionID const &rConsumerID,
                                 AAL::TransactionID const &rTranID) = 0;


   /// Unregisters any previous consumer TransactionID.
   ///
   /// Sends tranevtSampleAFU2UnRegister when unregistered.
   ///
   /// @param[in]  rTranID       For outbound events.
   virtual void UnRegisterConsumer(AAL::TransactionID const &rTranID) = 0;

   /// Free a workspace buffer.
   ///
   /// This function is one-way unless failed.
   ///
   /// @param[in]  WSBuffer   Buffer to free.
   virtual void FreeBuffer(WSBufDesc WSBuffer) = 0;

   /// ISampleAFU2Producer Destructor
   virtual ~ISampleAFU2Producer() {}
};

/// Event subclass ID for buffer empty messages.
#define tranevtSampleAFU2BufferEmpty __INTC_TranEvt(INTC_sysSampleAFU, 0x0002)

/// Interface for Buffer Empty events.
class IBufferEmptyTransactionEvent
{
public:
   /// Retrieve the buffer base address.
   virtual btVirtAddr WorkSpaceAddress() = 0;
   /// Retrieve the buffer size in bytes.
   virtual btWSSize      WorkSpaceSize() = 0;

   /// IBufferEmptyTransactionEvent Destructor
   virtual ~IBufferEmptyTransactionEvent() {}

   /// Type conversion operator to WSBufDesc
   operator WSBufDesc() { return WSBufDesc(WorkSpaceAddress(), WorkSpaceSize()); }
};

/// Event subclass ID for buffer full messages.
#define tranevtSampleAFU2BufferFull __INTC_TranEvt(INTC_sysSampleAFU, 0x0003)

/// Interface for Buffer Full events.
class IBufferFullTransactionEvent
{
public:
   /// Retrieve the buffer base address.
   virtual btVirtAddr WorkSpaceAddress() = 0;
   /// Retrieve the buffer size in bytes.
   virtual btWSSize      WorkSpaceSize() = 0;

   /// IBufferFullTransactionEvent Destructor
   virtual ~IBufferFullTransactionEvent() {}

   /// Type conversion operator to WSBufDesc
   operator WSBufDesc() { return WSBufDesc(WorkSpaceAddress(), WorkSpaceSize()); }
};

/// When consumer registered.
#define tranevtSampleAFU2Register      __INTC_TranEvt(INTC_sysSampleAFU, 0x0004)
/// When Consumer unregistered.
#define tranevtSampleAFU2UnRegister    __INTC_TranEvt(INTC_sysSampleAFU, 0x0005)
/// When Consumer started.
#define tranevtSampleAFU2Start         __INTC_TranEvt(INTC_sysSampleAFU, 0x0006)
/// When Consumer stopped.
#define tranevtSampleAFU2Stop          __INTC_TranEvt(INTC_sysSampleAFU, 0x0007)
#define tranevtSampleAFU2PutData       __INTC_TranEvt(INTC_sysSampleAFU, 0x0008)
#define tranevtSampleAFU2PutFree       __INTC_TranEvt(INTC_sysSampleAFU, 0x0009)

#define extranevtSampleAFU2Register    __INTC_ExTranEvt(INTC_sysSampleAFU, 0x0004)
#define extranevtSampleAFU2UnRegister  __INTC_ExTranEvt(INTC_sysSampleAFU, 0x0005)
#define extranevtSampleAFU2Start       __INTC_ExTranEvt(INTC_sysSampleAFU, 0x0006)
#define extranevtSampleAFU2Stop        __INTC_ExTranEvt(INTC_sysSampleAFU, 0x0007)
#define extranevtSampleAFU2PutData     __INTC_ExTranEvt(INTC_sysSampleAFU, 0x0008)
#define extranevtSampleAFU2PutFree     __INTC_ExTranEvt(INTC_sysSampleAFU, 0x0009)

/// @} group sample_afu2

END_NAMESPACE(AAL)

#endif // __AALSDK_SERVICE_SAMPLEAFU2SERVICE_H__

