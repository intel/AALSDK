// Copyright(c) 2014-2016, Intel Corporation
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
//        FILE: IServiceRevoked.h
//     CREATED: Feb 4, 2016
//      AUTHOR: Joseph Grecco <joe.grecco@intel.com>
//
// PURPOSE:   Definitions for the public IServiceRevoked interface
// HISTORY:
// COMMENTS:  This interface is called when resources associated with a Service
//            are forcibly revoked from the Service.
//            This API is intended to be used by the ServiceBase class.
// WHEN:          WHO:     WHAT:
//****************************************************************************///
#ifndef __AALSDK_AAS_ISERVICEREVOKE_H__
#define __AALSDK_AAS_ISERVICEREVOKE_H__


BEGIN_NAMESPACE(AAL)

/// @ingroup Service
/// @{

/// @interface IServiceRevoke
/// @brief Interface to enable Revoke Service Requests.
///
/// The intent of this API is to enable the Service at the root
///  of a Service hierarchy to Release. This call is typically invoked by a
///  lower level component to cause the Service stack to unwind cleanly. An
///   example of the intended use is:  An arbitrarily deep Service stack (i.e.,
///   a Service has allocated Services that may allocate Services). In the case that
///   a resource fails or becomes unavailable the Service stack may no longer be usable.
///   In order to clean up gracefully a Release() must be called on the root Service. This
///   interface is implemented by the root Service.  The Service is expected to call
///   Release() on itself.
class IServiceRevoke
{
public:

   /// Implemented by a Service to clean up gracefully when resources are
   /// revoked from the Service.
   /// @return void
   virtual void serviceRevoke() = 0;

   virtual ~IServiceRevoke() {}
};

/// @}

END_NAMESPACE(AAL)

#endif // __AALSDK_AAS_ISERVICEREVOKE_H__

