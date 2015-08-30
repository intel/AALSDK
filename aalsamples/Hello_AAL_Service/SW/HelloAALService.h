// Copyright (c) 2015, Intel Corporation
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
//****************************************************************************
/// @file HelloAALService.h
/// @brief IHelloAALClient and IHelloAALService.
/// @ingroup hello_service
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
#ifndef __AALSDK_SERVICE_HELLOAALSERVICE_H__
#define __AALSDK_SERVICE_HELLOAALSERVICE_H__
#include <aalsdk/AAL.h>
#include <aalsdk/osal/OSServiceModule.h>

using namespace AAL;

//#define HELLOAALSERVICE_SVC_MOD         "libhelloaalservice" AAL_SVC_MOD_EXT
//#define HELLOAALSERVICE_SVC_ENTRY_POINT "libhelloaalservice" AAL_SVC_MOD_ENTRY_SUFFIX

//AAL_DECLARE_SVC_MOD(libHELLOAALSERVICE, HELLOAAL_SERVICE_API)


/// @addtogroup hello_service
/// @{

#define iidSampleHelloAALClient __INTC_IID(INTC_sysSampleAFU,0x0002)
/// @brief Client Interface for IHelloAALClient
class IHelloAALClient
{
public:
   /// @brief Callback Client Interface for IHelloAALClient
   ///
   /// Called by the Service when asked to so by the client of the Service.
   /// @param[in]  rTranID   For messages sent back to the caller.
   virtual void HelloApp(TransactionID const &rTranID) = 0;
};

/// IHelloAALService interface ID.
#define iidSampleHelloAAL __INTC_IID(INTC_sysSampleAFU,0x0001)

/// Example of a custom interface for a Service
class IHelloAALService
{
public:
   /// @brief An illustration of an AFU custom interface.
   ///
   /// Writes the passed in message and calls the client back.
   ///
   /// @param[in]  sMessage  A message to be received by this AFU.
   /// @param[in]  rTranID   For messages sent back to the caller.
   virtual void Hello(btcString            sMessage,
                      TransactionID const &rTranID) = 0;

   /// ISampleAFUPing Destructor
   virtual ~IHelloAALService() {}
};

/// @}

// Convenience macros for printing messages and errors.
#ifndef MSG
# define MSG(x) std::cout << __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() : " << x << std::endl
#endif // MSG
#ifndef ERR
# define ERR(x) std::cerr << __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() **Error : " << x << std::endl
#endif // ERR

#endif // __AALSDK_SERVICE_HELLOAALSERVICE_H__

