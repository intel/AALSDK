// Copyright (c) 2014-2015, Intel Corporation
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
//        FILE: IServiceClient.h
//     CREATED: Mar 23, 2014
//      AUTHOR: Joseph Grecco <joe.grecco@intel.com>
//
// PURPOSE:   Common Interface implemented by clients of AAL Services
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
//****************************************************************************///
#ifndef __AALSDK_ISERVICECLIENT_H__
#define __AALSDK_ISERVICECLIENT_H__
#include <aalsdk/AALTypes.h>
#include <aalsdk/AALBase.h>

/// @addtogroup XLRuntime
/// @{

BEGIN_NAMESPACE(AAL)
   BEGIN_NAMESPACE(AAS)

//=============================================================================
/// @class        IServiceClient
/// @brief        Client Interface for any Service object in the AAL XLRuntime.
///
///   An object that
///   wants to use a Service in the XL Runtime instantiates an instance of this class,
///   encapsulates it's this pointer into an IBase, and passes that to an
///   instantiated Runtime object via that object's Runtime.allocService() function.
///   The Service is instantiated, handed this object's pointer, and can now call
///   it back. This mechanism establishes a binding between the two objects so that
///   each object can communicate with the other.
//=============================================================================

class IServiceClient
{
public:
   /// @brief     Called by a Runtime object to indicate that it
   ///               successfully allocated a service after a call to
   ///               Runtime.allocService().
   ///
   /// Note that the Service version of this function and the Runtime version of this function
   ///   are both called. One can choose to use either or both.
   ///
   /// @param[in] pServiceBase is an IBase that will contain the pointer to the service
   ///               that was allocated. The actual pointer is extracted via the
   ///               dynamic_ptr<> operator. It will also contain a pointer to the
   ///               IService interface of the Service that was allocated, through which
   ///               Release() will need to be called.
   /// @param[in] rTranID is reference to the TransactionID that was passed to
   ///               Runtime.allocService().
   /// @return    void
   ///
   /// @code
   /// void serviceAllocated( AAL::IBase *pServiceBase, TransactionID const &rTranID) {
   ///    ASSERT( pServiceBase );        // if false, then Service threw a bad pointer
   ///
   ///    ISampleAFUPing *m_pAALService; // used to call Release on the Service
   ///    m_pAALService = dynamic_ptr<IAALService>( iidService, pServiceBase);
   ///    ASSERT( m_pAALService );
   ///
   ///    ISampleAFUPing *m_pPingAFU;    // used for Specific Service (in this case Ping)
   ///    m_pPingAFU = dynamic_ptr<ISampleAFUPing>( iidSampleAFUPing, pServiceBase);
   ///    ASSERT( m_pPingAFU );
   /// }
   /// @endcode
   virtual void      serviceAllocated(AAL::IBase *pServiceBase,
                                      TransactionID const &rTranID = TransactionID()) = 0;

   /// @brief     Called by a Runtime object to indicate that it failed to
   ///               successfully allocate a service after a call to
   ///               Runtime.allocService().
   /// @param[in] rEvent will be an exception event that can be parsed to determine
   ///               the error that occurred.
   /// @return    void
   virtual void      serviceAllocateFailed(const IEvent &rEvent) = 0;

   /// @brief     Called by a Service object to indicate that it has stopped successfully
   ///               and been freed after a call to IAALService.Release().
   /// @param[in] rTranID is reference to the TransactionID that was passed to
   ///               IAALService.Release().
   /// @return    void
   virtual void      serviceFreed(TransactionID const &rTranID = TransactionID()) = 0;

   /// @brief     Called by a Service object to pass exceptions and other
   ///               unsolicited messages.
   /// @param[in] rEvent will be an event that can be parsed to determine
   ///               what occurred.
   /// @return    void
   virtual void      serviceEvent(const IEvent &rEvent) = 0;

   /// @brief     Destructor
   virtual ~IServiceClient() {}
};


   END_NAMESPACE(AAS)
END_NAMESPACE(AAL)

#endif // __AALSDK_ISERVICECLIENT_H__

/// @} group XLRuntime
