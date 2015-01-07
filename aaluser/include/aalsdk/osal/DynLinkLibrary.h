// Copyright (c) 2007-2015, Intel Corporation
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
/// @file DynLinkLibrary.h
/// @brief Implements Abstractions for Dynamic Link Libraries
/// @ingroup OSAL
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Joseph Grecco, Intel Corporation
///          Henry Mitchel, Intel Corporation
///          Tim Whisonant, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 03/21/2007     JG       Initial version started
/// 11/27/2007     JG/HM    Fixed Leak
/// 05/08/2008     HM       Comments & License
/// 01/04/2009     HM       Updated Copyright@endverbatim
//****************************************************************************
#ifndef __AALSDK_OSAL_DYNLINKLIBRARY_H__
#define __AALSDK_OSAL_DYNLINKLIBRARY_H__
#include <aalsdk/AALTypes.h>

/// @addtogroup OSAL
/// @{

/// Interface to dynamically-loaded shared libraries.
class OSAL_API DynLinkLibrary
{
public:
   /// DynLinkLibrary Constructor.
   DynLinkLibrary(const std::string &LibraryName);
   /// DynLinkLibrary Destructor.
   virtual ~DynLinkLibrary();
   /// Gets the address of the named symbol.
   ///
   /// @retval  NULL  On failure.
   void * GetSymAddress(const std::string &SymbolName);
   /// Internal state check.
   AAL::btBool IsOK() { return NULL != m_hDLL; }

private:
   AAL::btObjectType m_hDLL;

   DynLinkLibrary();
   DynLinkLibrary(const DynLinkLibrary & );
   DynLinkLibrary & operator = (const DynLinkLibrary & );
};

/// @} group OSAL

#endif // __AALSDK_OSAL_DYNLINKLIBRARY_H__

