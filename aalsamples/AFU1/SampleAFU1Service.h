// Copyright (c) 2007-2014, Intel Corporation
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
/// @file SampleAFU1Service.h
/// @brief ISampleAFUPing and related interfaces.
/// @ingroup sample_afu1
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer Sample Application
///
///    This application is for example purposes only.
///    It is not intended to represent a model for developing commercially-deployable applications.
///    It is designed to show working examples of the AAL programming model and APIs.
///
/// AUTHORS: Joseph Grecco, Intel Corporation
///          Henry Mitchel, Intel Corporation
///          Tim Whisonant, Intel Corporation
///          Sadruta Chandrashekar, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 09/30/2011      JG      Based on original samples from AAL SDK version 2.20@endverbatim
//****************************************************************************
#ifndef __AALSDK_SERVICE_SAMPLEAFU1SERVICE_H__
#define __AALSDK_SERVICE_SAMPLEAFU1SERVICE_H__
#include <aalsdk/AAL.h>
#include <aalsdk/osal/OSServiceModule.h>

const std::string strConfigRecord("9 20 ConfigRecordIncluded\n \
      \t10\n \
          \t\t9 17 ServiceExecutable\n \
            \t\t\t9 13 libsampleafu1\n \
         \t\t9 18 _CreateSoftService\n \
         \t\t0 1\n \
   9 29 ---- End of embedded NVS ----\n \
      9999\n");


#ifndef SAMPLEAFU1_VERSION_CURRENT
# define SAMPLEAFU1_VERSION_CURRENT  0
#endif // SAMPLEAFU1_VERSION_CURRENT
#ifndef SAMPLEAFU1_VERSION_REVISION
# define SAMPLEAFU1_VERSION_REVISION 0
#endif // SAMPLEAFU1_VERSION_REVISION
#ifndef SAMPLEAFU1_VERSION_AGE
# define SAMPLEAFU1_VERSION_AGE      0
#endif // SAMPLEAFU1_VERSION_AGE
#ifndef SAMPLEAFU1_VERSION
# define SAMPLEAFU1_VERSION          "0.0.0"
#endif // SAMPLEAFU1_VERSION

#if defined ( __AAL_WINDOWS__ )
# ifdef SAMPLEAFU1_EXPORTS
#    define SAMPLEAFU1_API __declspec(dllexport)
# else
#    define SAMPLEAFU1_API __declspec(dllimport)
# endif // SAMPLEAFU1_EXPORTS
#else
# ifndef __declspec
#    define __declspec(x)
# endif // __declspec
# define SAMPLEAFU1_API    __declspec(0)
#endif // __AAL_WINDOWS__

#define SAMPLEAFU1_SVC_MOD         "libsampleafu1" AAL_SVC_MOD_EXT
#define SAMPLEAFU1_SVC_ENTRY_POINT "libsampleafu1" AAL_SVC_MOD_ENTRY_SUFFIX

AAL_DECLARE_SVC_MOD(libsampleafu1, SAMPLEAFU1_API)


BEGIN_NAMESPACE(AAL)

/// @addtogroup sample_afu1
/// @{

/// Event subclass ID for ping messages.
#define tranevtSampleAFUPing   __INTC_TranEvt(INTC_sysSampleAFU,0x0001)
/// Event subclass ID for ping exceptions.
#define extranevtSampleAFUPing __INTC_ExTranEvt(INTC_sysSampleAFU,0x0001)


#define iidSampleAFUPingClient __INTC_IID(INTC_sysSampleAFU,0x0002)
/// @brief Client Interface for ISampleAFUPing
class ISampleAFUPingClient
{
public:
   /// @brief Callback Client Interface for ISampleAFUPing
   ///
   /// Called by the Service when asked to so by the client of the Service.
   /// @param[in]  rTranID   For messages sent back to the caller.
   virtual void PingReceived(AAL::TransactionID const &rTranID) = 0;
};

/// ISampleAFUPing interface ID.
#define iidSampleAFUPing __INTC_IID(INTC_sysSampleAFU,0x0001)

/// Example of a custom interface for a Service
class ISampleAFUPing
{
public:
   /// @brief An illustration of an AFU custom interface.
   ///
   /// Spawns a worker thread which sends 5 tranevtSampleAFUPing
   /// messages back to the caller.
   ///
   /// @param[in]  sMessage  A message to be received by this AFU.
   /// @param[in]  rTranID   For messages sent back to the caller.
   virtual void Ping(AAL::btcString            sMessage,
                     AAL::TransactionID const &rTranID) = 0;

   /// @brief An illustration of an AFU custom interface.
   ///
   /// Sends a single tranevtSampleAFUPing back to the caller from the current thread.
   ///
   /// @param[in]  sMessage  A message to be received by this AFU.
   /// @param[in]  rTranID   For messages sent back to the caller.
   virtual void PingOne(AAL::btcString      sMessage,
                        AAL::TransactionID &rTranID) = 0;

   /// @brief An illustration of an AFU custom interface.
   ///
   /// Sends N tranevtSampleAFUPing messages back to the caller from the current thread.
   ///
   /// @param[in]  sMessage  A message to be received by this AFU.
   /// @param[in]  n         The number of messages to send.
   virtual void PingSingleThread(AAL::btcString sMessage, unsigned int n) = 0;

   /// @brief An illustration of an AFU custom interface.
   ///
   /// Spawns N worker threads, each of which sends a tranevtSampleAFUPing
   /// back to the caller. This thread optionally waits for all of the messages to
   /// be sent.
   ///
   /// @param[in]  sMessage  A message to be received by this AFU.
   /// @param[in]  n         The number of messages to send.
   /// @param[in]  wait      Whether or not to wait for messages to be sent.
   virtual void PingMultiThread(AAL::btcString sMessage, unsigned int n, btBool wait=false) = 0;

   /// @brief Releases the Service, allowing the framework to destroy it.
   ///
   /// @param[in]  rTranID   For messages sent back to the caller.
   /// @param[in]  timeout   How long to wait for the Service to be destroyed.
   virtual btBool Release(TransactionID const &rTranID, btTime timeout=AAL_INFINITE_WAIT) = 0;

   /// ISampleAFUPing Destructor
   virtual ~ISampleAFUPing() {}
};

/// @} group sample_afu1

// Convenience macros for printing messages and errors.
#ifndef MSG
# define MSG(x) std::cout << __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() : " << x << std::endl
#endif // MSG
#ifndef ERR
# define ERR(x) std::cerr << __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() **Error : " << x << std::endl
#endif // ERR

END_NAMESPACE(AAL)

#endif // __AALSDK_SERVICE_SAMPLEAFU1SERVICE_H__

