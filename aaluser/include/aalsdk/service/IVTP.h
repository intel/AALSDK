// Copyright(c) 2015-2016, Intel Corporation
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
/// @file IVTPSERVICE.h
/// @brief IVTPService.
/// @ingroup vtp_service
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
/// Virtual-to-Physical address translation service
///
/// TODO: add verbose description
///
/// Provides service for access to the VTP BBB for address translation.
/// Assumes a VTP BBB DFH to be detected and present.
///
/// On initialization, allocates shared buffer for VTP page hash and
/// communicates its location to VTP BBB.
///
/// Provides synchronous methods to update page hash on shared buffer
/// allocation.
///
/// Does not have an explicit client callback interface, as all published
/// service methods are synchronous.
///
/// AUTHORS: Enno Luebbers, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 01/15/2016     EL       Initial version@endverbatim
//****************************************************************************
#ifndef __IVTP_H__
#define __IVTP_H__
#include <aalsdk/AAL.h>
#include <aalsdk/osal/OSServiceModule.h>

#include <aalsdk/service/IALIAFU.h>

BEGIN_NAMESPACE(AAL)

/// @addtogroup VTPService
/// @{

// Service creation parameter keys and datatypes
#define ALIAFU_IBASE_KEY "ALIAFUIBase"
#define ALIAFU_IBASE_DATATYPE btObjectType
#define VTP_DFH_ADDRESS_KEY "VTPDFHAddress"
#define VTP_DFH_ADDRESS_DATATYPE btObjectType
#define VTP_DFH_OFFSET_KEY "VTPDFHOffset"
#define VTP_DFH_OFFSET_DATATYPE btCSROffset

/// VTP BBB GUID, used to identify HW VTP component
#define VTP_BBB_GUID "C8A2982F-FF96-42BF-A705-45727F501901"

/// VTP Service IID
#ifndef iidVTPService
#define iidVTPService __INTC_IID(INTC_sysSampleAFU, 0x0010)
#endif

/// @brief VTP service interface
///
/// Provides all functions of IALIBuffer, with the difference of now allocating
/// buffers that can be accessed from the AFU using virtual addresses (through
/// VTP.
///
/// @note Since AFU reset will reset all user logic including BBBs like VTP,
///       users need to make sure to reinitialize VTP after an AFU reset
///       using vtpReset().
///
/// @see IALIBuffer

class IVTP : public IALIBuffer
{
public:
   virtual ~IVTP() {}

   /// Reinitialize VTP after AFU Reset
   virtual btBool vtpReset( void ) = 0;

};

/// @}

END_NAMESPACE(AAL)

#endif // __IVTP_H__

