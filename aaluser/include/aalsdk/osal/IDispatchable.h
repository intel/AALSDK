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

BEGIN_NAMESPACE(AAL)

/// @brief Object used to schedule work
class OSAL_API IDispatchable
{
public:
   /// @brief  Where the work happens. The function performed here can be virtually anything.
   /// Most often used to schedule a callback.
   ///
   /// @returns void
   virtual void operator() () = 0;

   virtual ~IDispatchable() {}
};

/// @brief Groups one or more IDispatchable's so that they are executed as one.
class OSAL_API DispatchableGroup : public IDispatchable
{
public:
   /// @brief DispatchableGroup constructor.
   ///
   /// Create a group with a single dispatchable item.
   ///
   /// @param[in] p0 A pointer to an IDispatchable, something to be dispatched.
   DispatchableGroup(IDispatchable *p0)
   {
      ASSERT(NULL != p0);
      m_DispList.push_back(p0);
   }
   /// @brief DispatchableGroup constructor.
   ///
   /// Create a group with 2 dispatchable items.
   ///
   /// @param[in] p0 A pointer to an IDispatchable, something to be dispatched.
   /// @param[in] p1 A pointer to an IDispatchable, something to be dispatched.
   DispatchableGroup(IDispatchable *p0,
                     IDispatchable *p1)
   {
      ASSERT(NULL != p0);
      m_DispList.push_back(p0);
      ASSERT(NULL != p1);
      m_DispList.push_back(p1);
   }
   /// @brief DispatchableGroup constructor.
   ///
   /// Create a group with 3 dispatchable items.
   ///
   /// @param[in] p0 A pointer to an IDispatchable, something to be dispatched.
   /// @param[in] p1 A pointer to an IDispatchable, something to be dispatched.
   /// @param[in] p2 A pointer to an IDispatchable, something to be dispatched.
   DispatchableGroup(IDispatchable *p0,
                     IDispatchable *p1,
                     IDispatchable *p2)
   {
      ASSERT(NULL != p0);
      m_DispList.push_back(p0);
      ASSERT(NULL != p1);
      m_DispList.push_back(p1);
      ASSERT(NULL != p2);
      m_DispList.push_back(p2);
   }
   /// @brief DispatchableGroup constructor.
   ///
   /// Create a group with 4 dispatchable items.
   ///
   /// @param[in] p0 A pointer to an IDispatchable, something to be dispatched.
   /// @param[in] p1 A pointer to an IDispatchable, something to be dispatched.
   /// @param[in] p2 A pointer to an IDispatchable, something to be dispatched.
   /// @param[in] p3 A pointer to an IDispatchable, something to be dispatched.
   DispatchableGroup(IDispatchable *p0,
                     IDispatchable *p1,
                     IDispatchable *p2,
                     IDispatchable *p3)
   {
      ASSERT(NULL != p0);
      m_DispList.push_back(p0);
      ASSERT(NULL != p1);
      m_DispList.push_back(p1);
      ASSERT(NULL != p2);
      m_DispList.push_back(p2);
      ASSERT(NULL != p3);
      m_DispList.push_back(p3);
   }
   /// @brief DispatchableGroup constructor.
   ///
   /// Create a group with 5 dispatchable items.
   ///
   /// @param[in] p0 A pointer to an IDispatchable, something to be dispatched.
   /// @param[in] p1 A pointer to an IDispatchable, something to be dispatched.
   /// @param[in] p2 A pointer to an IDispatchable, something to be dispatched.
   /// @param[in] p3 A pointer to an IDispatchable, something to be dispatched.
   /// @param[in] p4 A pointer to an IDispatchable, something to be dispatched.
   DispatchableGroup(IDispatchable *p0,
                     IDispatchable *p1,
                     IDispatchable *p2,
                     IDispatchable *p3,
                     IDispatchable *p4)
   {
      ASSERT(NULL != p0);
      m_DispList.push_back(p0);
      ASSERT(NULL != p1);
      m_DispList.push_back(p1);
      ASSERT(NULL != p2);
      m_DispList.push_back(p2);
      ASSERT(NULL != p3);
      m_DispList.push_back(p3);
      ASSERT(NULL != p4);
      m_DispList.push_back(p4);
   }

   virtual void operator() ()
   {
      std::list<IDispatchable *>::iterator iter;
      for ( iter = m_DispList.begin() ; iter != m_DispList.end() ; ++iter ) {
         (*iter)->operator() ();
      }
      delete this;
   }

protected:
   DispatchableGroup() {}

#if defined( _MSC_VER )
#pragma warning( push )
#pragma warning( disable:4251 )  // Cannot export template definitions
#endif // _MSC_VER
   std::list<IDispatchable *> m_DispList;
#if defined( _MSC_VER )
#pragma warning( pop )
#endif // _MSC_VER
};

END_NAMESPACE(AAL)

/// @}

#endif // __IDISPATCHABLE_H__
