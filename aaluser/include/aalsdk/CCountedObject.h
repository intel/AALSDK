// Copyright (c) 2008-2015, Intel Corporation
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
/// @file CCountedObject.h
/// @brief Definition of class CCountedObject.
/// @ingroup AASUtils
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Henry Mitchel, Intel Corporation
///          Joseph Grecco, Intel Corporation
///          Tim Whisonant, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 01/23/2013     TSW      Move class def for CCountedObject to its own header.@endverbatim
//****************************************************************************
#ifndef __AALSDK_CCOUNTEDOBJECT_H__
#define __AALSDK_CCOUNTEDOBJECT_H__
#include <aalsdk/AALTypes.h>

/// @addtogroup AASUtils
/// @{

/// Counted object base class.
class AASLIB_API CCountedObject
{
public:
   /// CCountedObject Default Constructor.
   CCountedObject();
   /// CCountedObject Destructor.
   virtual ~CCountedObject();

   /// Increment this object's reference count by one.
   ///
   /// @return The newly-incremented reference count.
   AAL::btUnsigned32bitInt IncRef() { return ++m_RefCnt; }
   /// If this object's refernece count is non-zero, decrement the reference count by one.
   ///
   /// @return The old reference count before any decrement.
   AAL::btUnsigned32bitInt DecRef() { return m_RefCnt ? m_RefCnt-- : 0; }

protected:
   AAL::btUnsigned32bitInt m_RefCnt;
};

/// @} group AASUtils

#endif // __AALSDK_CCOUNTEDOBJECT_H__

