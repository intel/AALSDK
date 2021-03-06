// Copyright(c) 2007-2016, Intel Corporation
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
/// @file uidrvMessage.cpp
/// @brief Implementation uidrvMessage class. This is an abstraction of the low
///        level message structure used by the driver interface.
/// @ingroup uAIA
/// @verbatim
/// Accelerator Abstraction Layer
///
/// AUTHOR: Tim Whisonant, Intel Corporation.
///         Joseph Grecco, Intel Corporation.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 1/22/2013      TSW      uidrvMessage::uidrvMessageRoute -> uidrvMessageRoute{}
/// 03/12/2013     JG       Changed uidrvMessage to support link-less ioctlreq
/// 09/15/2015     JG       Removed message route and fixed up for 4.0@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include "uidrvMessage.h"
#include "aalsdk/AALBase.h" // IBase


BEGIN_NAMESPACE(AAL)

uidrvMessage::uidrvMessage() :
   m_pmessage(NULL)
{}

uidrvMessage::~uidrvMessage()
{
   if ( NULL != m_pmessage ) {
      delete m_pmessage;
      m_pmessage = NULL;
   }
}

void uidrvMessage::size(btWSSize PayloadSize)
{
   if ( NULL != m_pmessage ) {
      delete m_pmessage;
   }
   m_msgsize  = (btUnsignedInt)PayloadSize + sizeof(ccipui_ioctlreq);
   m_pmessage = (struct ccipui_ioctlreq*)new btByte[m_msgsize];
   memset(m_pmessage, 0, m_msgsize);
   m_pmessage->size = PayloadSize;
}

btVirtAddr  uidrvMessage::payload() const
{ ASSERT(NULL != m_pmessage);
   btVirtAddr ptr = reinterpret_cast<btVirtAddr>(m_pmessage->payload);
   return reinterpret_cast<btVirtAddr>(m_pmessage->payload);
}

END_NAMESPACE(AAL)

