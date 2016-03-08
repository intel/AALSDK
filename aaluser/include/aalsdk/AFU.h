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
/// @file AFU.h
/// @brief Definitions for AFU interface.
/// @ingroup AFU
/// @verbatim
/// Accelerator Abstraction Layer
///
/// AUTHOR: Joseph Grecco, Intel Corporation.
///         Henry Mitchel, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 04/30/2007     JG       Initial version started
/// 06/12/2007     JG       Converted to use AALID_t
/// 05/08/2008     HM       Comments & License
/// 12/14/2008     HM       Allocate and Free done, changes to IAFU
/// 01/04/2009     HM       Updated Copyright
/// 06/07/2012     JG       Eliminated deprecated IAFU and AFU namespace
/// 07/14/2012     HM       Put virtual dtor in as needed to virtual classes@endverbatim
//****************************************************************************
#ifndef __AALSDK_AFU_H__
#define __AALSDK_AFU_H__
#include <aalsdk/AALTypes.h>
#include <aalsdk/AALTransactionID.h>

BEGIN_NAMESPACE(AAL)

/// @addtogroup SysEvents
/// @{

//=============================================================================
// Name: IAFUAllocateWorkSpaceTransactionEvent
// Description:
//=============================================================================
class UAIA_API IAFUAllocateWorkSpaceTransactionEvent
{
public:
   virtual btVirtAddr WorkSpaceAddress() = 0;
   virtual btWSID     WorkSpaceID()      = 0;  // To be deprecated
   virtual btWSSize   WorkSpaceSize()    = 0;

   virtual ~IAFUAllocateWorkSpaceTransactionEvent();
};

//=============================================================================
// Name: IAFUFreeWorkSpaceTransactionEvent
// Description:
//=============================================================================
class UAIA_API IAFUFreeWorkSpaceTransactionEvent
{
public:
   virtual btWSID WorkSpaceID() = 0; // to be deprecated

   virtual ~IAFUFreeWorkSpaceTransactionEvent();
};


/// @}


//=============================================================================
// Name: IWorkspce
/// @brief Public Interface class for Workspace
//=============================================================================
class UAIA_API IWorkspace
{
public:
    virtual void AllocateWorkSpace(btWSSize             Size,
                                   TransactionID const &TranID=TransactionID()) = 0;

    virtual void FreeWorkSpace(btVirtAddr           WorkspaceAddress,
                               TransactionID const &TranID=TransactionID()) = 0;
    virtual ~IWorkspace();
};


END_NAMESPACE(AAL)

#endif // __AALSDK_AFU_H__

