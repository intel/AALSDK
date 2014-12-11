// Copyright (c) 2011-2014, Intel Corporation
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
/// @file SampleAFU1Service-internal.h
/// @brief Definitions for Sample 1 AFU Service.
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
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 09/30/2011     JG       Initial version
/// 06/07/2012     JG       Added implementation of Release@endverbatim
//****************************************************************************
#ifndef __SAMPLEAFU1SERVICE_INT_H__
#define __SAMPLEAFU1SERVICE_INT_H__
#include "SampleAFU1Service.h" // Public AFU device interface
#include <aalsdk/aas/AALService.h>
#include <aalsdk/IServiceClient.h>
#include <aalsdk/osal/IDispatchable.h>

BEGIN_NAMESPACE(AAL)


//=============================================================================
// Name: PingAFU
// Description: Simple sample AFU (i.e., Device) AAL Service
// Interface: ISampleAFUPing
// Comments:
//=============================================================================
class PingAFU : public AAL::AAS::ServiceBase, public ISampleAFUPing
{
public:

   // Macro defines the constructor for a loadable AAL service.
   //  The first argument is your class name, the second argument is the
   //  name of the Service base class this service is derived from. In this
   //  example we use ServiceBase as it is the class that provides the
   //  support for Software-only devices.  Hardware-supported services might
   //  use DeviceServiceBase instead.
   //
   // Note that initializers can be declared here but are preceded by a comma
   //  rather than a colon.
   //
   // The design pattern is that the constructor does minimal work. Here we are
   //  registering the interfaces the service implements. The default (Subclass)
   //  interface is ISampleAFUPing.  ServiceBase provides an init() method that
   //  can be used where more sophisticated initialization is required. The
   //  init() method is called by the factory AFTER construction but before use.
   DECLARE_AAL_SERVICE_CONSTRUCTOR(PingAFU, AAL::AAS::ServiceBase),
      m_pSvcClient(NULL),
      m_pPingClient(NULL),
      m_pThread(NULL),
      m_ppThreads(NULL)
   {
      SetSubClassInterface(iidSampleAFUPing, dynamic_cast<ISampleAFUPing *>(this));
   }
   // Hook to allow the object to initialize
   void init(TransactionID const &rtid);

   void Ping(btcString sMessage,
             TransactionID const &rTranID);
   void PingOne(btcString sMessage,
                TransactionID &rTranID);
   void PingSingleThread(btcString sMessage, unsigned int n);
   void PingMultiThread(btcString sMessage, unsigned int n, btBool wait = false );

   static void WorkerThread(OSLThread *pThread, void *pContext);
   static void SinglePingThread(OSLThread *pThread, void *pContext);

   // Called when the service is released
   btBool Release(TransactionID const &rTranID, btTime timeout=AAL_INFINITE_WAIT);

   // Quiet Release. Used when Service is unloaded.
   btBool Release(btTime timeout=AAL_INFINITE_WAIT);

protected:
   AAL::AAS::IServiceClient  *m_pSvcClient;
   AAL::ISampleAFUPingClient *m_pPingClient;
   OSLThread                 *m_pThread;
   OSLThread                **m_ppThreads;
   TransactionID              m_CurrTranID;
};

// XL - aware event for generating the Ping response.
class SampleAFUPingFunctor : public IDispatchable
{
public:
   SampleAFUPingFunctor(AAL::ISampleAFUPingClient *pSvcClient, IBase *pPingAFU, TransactionID const &rTranID);
   virtual void operator() ();

protected:
   AAL::ISampleAFUPingClient *m_pSvcClient;
   IBase                     *m_pPingAFU;
   TransactionID const       &m_TranID;
};

END_NAMESPACE(AAL)

#endif //__SAMPLEAFU1SERVICE_INT_H__
