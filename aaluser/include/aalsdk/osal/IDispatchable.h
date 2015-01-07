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
///        @file IDispatchable.h
///        @ingroup OSAL
///        @brief Defines the interface for dispatchable objects.
/// @verbatim
///     CREATED: Mar 6, 2014
///      AUTHOR: Joseph Grecco <joe.grecco@intel.com>
///
///          Defines the interface for dispatchable objects.  Dispatchable
///          objects are those which are typically sent to a recipient such
///          as an event or message. Dispatchables are dispatched via a
///          "Dispatcher" such as the Event Delivery Service.
///
///          The dispatch function allows the object to "dispatch" itself as
///          appropriate.
///
/// HISTORY:
/// COMMENTS: Dispatchables need not actually deliver themselves to anything but
///           can effectively be functors by invoking the desired behavior in
///           the operator () member.
/// WHEN:          WHO:     WHAT:
/// @endverbatim
//****************************************************************************///
#ifndef __IDISPATCHABLE_H__
#define __IDISPATCHABLE_H__
#include <aalsdk/AALTypes.h>

/// @addtogroup Dispatchable
/// @{

/// @brief Object used to schedule work
class OSAL_API IDispatchable
{
public:
   /// @brief  Where the work happens. The function performed here can be virtually anything.
   /// Most often used to schedule a callback.
   virtual void operator() () = 0;

   virtual ~IDispatchable() {}
};
/// @} group Dispatchable

#endif // __IDISPATCHABLE_H__
