// Copyright (c) 2005-2015, Intel Corporation
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
/// @file AALBase.h
/// @brief Defines IBase, IAALService, dynamic_ptr(), dynamic_ref(), subclass_ptr(), and subclass_ref().
/// @ingroup CommonBase
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Joseph Grecco, Intel Corporation
///          Henry Mitchel, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 12/20/2005     JG       Initial version started
/// 02/13/2007     JG       Redefined base to include generic
///                         SendMessage() method
/// 03/13/2007     JG       Changed SendMsg() to PorcessMessage()
///                         Defined a non-transaction version of
///                         ProcessMessage
/// 03/19/2007     JG       Linux Port
/// 04/23/2007     JG       Added AAS Interfaces
/// 04/24/2007     JG       Made I_LRBase inherit from iidBase
/// 04/26/2007     JG       Deprecated I_LRBase. Replaced by
///                         IAFU
///                         Added namespace AAS
/// 05/28/2007     JG       Added cached interfaces (SubClass)
/// 06/08/2007     JG       Converted to btID keys
/// 09/25/2007     JG       Refactored out old code
/// 10/04/2007     JG       Renamed to AALBase
/// 05/08/2008     HM       Comments & License
/// 05/16/2008     HM       Added subclass_ptr and dynamic_ptr, and switched
///                            C-style casts to proper static_cast
/// 11/19/2008     JG       Added smart pointer wrappers for IBase and subclass
///                         pointers IBase_sptr and dynamic_sptr<>.
/// 01/04/2009     HM       Updated Copyright
/// 10/06/2015     JG       Removed Subclass interfaces@endverbatim
//****************************************************************************
#ifndef __AALSDK_AALBASE_H__
#define __AALSDK_AALBASE_H__
#include <aalsdk/AALTypes.h>
#include <aalsdk/AALEvent.h>
#include <aalsdk/AALTransactionID.h>


BEGIN_NAMESPACE(AAL)

/// @addtogroup CommonBase
/// @{

//=============================================================================
// Typedefs and Constants
//=============================================================================

/// Public Interface base class for AAL active objects.
class AASLIB_API IBase
{
public:
   /// IBase Destructor.
   virtual ~IBase(){};
   /// Query interface for a given interface id.
   virtual btGenericInterface Interface(btIID Interface) const = 0;
   /// Determine whether this object contains the specified interface.
   virtual btBool Has(btIID Interface) const                   = 0;
   /// IBase inequality.
   virtual btBool operator != (IBase const &rother) const      = 0;
   /// IBase equality.
   virtual btBool operator == (IBase const &rother) const      = 0;
   /// Internal state check.
   virtual btBool IsOK() const                                 = 0;
   /// Retrieve the application-specific context for the object.
   virtual btApplicationContext Context() const                = 0;
};


/// Cast an IBase const & to a T & by interface id.
template<class T>
T & dynamic_ref(btIID Name, IBase const &obj) {
   return *static_cast<T *>(obj.Interface(Name));
}
/// Cast an IBase const * to a T & by interface id.
template<class T>
T & dynamic_ref(btIID Name, IBase const *obj) {
   return *static_cast<T *>(obj->Interface(Name));
}
/// Cast an IEvent const & to a T & by interface id.
template<class T>
T & dynamic_ref(btIID Name, IEvent const &obj) {
   return *static_cast<T *>(obj.Interface(Name));
}


/// Cast an IBase const & to a T * by interface id.
/// @retval  NULL  No such interface.
template<class T>
T * dynamic_ptr(btIID Name, IBase const &obj) {
   return static_cast<T *>(obj.Interface(Name));
}
/// Cast an IBase const * to a T * by interface id.
/// @retval  NULL  No such interface.
template<class T>
T * dynamic_ptr(btIID Name, IBase const *obj) {
   return static_cast<T *>(obj->Interface(Name));
}
/// Cast an IEvent const & to a T * by interface id.
/// @retval  NULL  No such interface.
template<class T>
T * dynamic_ptr(btIID Name, IEvent const &obj) {
   return static_cast<T *>(obj.Interface(Name));
}

/// @}


/// Base interface for AAL Services.
/// @ingroup Services
class AASLIB_API IAALService
{
public:
   /// IAALService Destructor.
   virtual ~IAALService() {}

   /// Shutdown the service and release its resources.
   ///
   /// Generates a tranevtObjectDestroyed on success.
   ///
   /// @param[in]  rTranID  The TransactionID to be used for the tranevtObjectDestroyed event response.
   /// @param[in]  timeout  The amount of time to wait for the Release to occur.
   ///
   /// @retval  true   Service destruction was successful.
   /// @retval  false  An error occurred during destruction.
   virtual btBool  Release(TransactionID const &rTranID,
                           btTime               timeout=AAL_INFINITE_WAIT) = 0;

};


END_NAMESPACE(AAL)

#endif // __AALSDK_AALBASE_H__

