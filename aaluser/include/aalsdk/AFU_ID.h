// Copyright(c) 2010-2016, Intel Corporation
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
/// @file AFU_ID.h
/// @brief Define some globally useful AFU_IDs. These should be real GUID's,
///        and will be at some point
/// @ingroup AFU
/// @verbatim
/// Accelerator Abstraction Layer
///
/// AUTHOR: Henry Mitchel, Intel Corporation
///
/// HISTORY: 
/// WHEN:          WHO:     WHAT:
/// 09/30/2010     HM       Initial version@endverbatim
//****************************************************************************
#ifndef __AALSDK_AFU_ID_H__
#define __AALSDK_AFU_ID_H__

#define DESIRED_AFU_IDVAFU_32          "00000000-0000-0000-0000-00005F5F5F5F"
#define DESIRED_AFU_IDVAFU_64          "00000000-0000-0000-5F5F-5F5F5F5F5F5F"
#define DESIRED_AFU_IDVAFU_128         "5F5F5F5F-5F5F-5F5F-5F5F-5F5F5F5F5F5F"
#define DESIRED_AFU_MGT_32             "00000000-0000-0000-0000-0000A0A0A0A0"
#define DESIRED_AFU_MGT_64             "00000000-0000-0000-A0A0-A0A0A0A0A0A0"
#define DESIRED_AFU_MGT_128            "A0A0A0A0-A0A0-A0A0-A0A0-A0A0A0A0A0A0"
#define DESIRED_AFU_IDASM_32           "00000000-0000-0000-0000-000011111110"
#define DESIRED_AFU_IDASM_64           "00000000-0000-0000-2222-222211111110"
#define DESIRED_AFU_IDASM_128          "22222222-1111-1111-2222-222211111110"
#define DESIRED_AFU_EDGEDETECT_32      "00000000-0000-0000-0000-0000DFFF0002"
#define DESIRED_AFU_EDGEDETECT_64      "00000000-0000-0000-FFFF-0000DFFF0002"
#define DESIRED_AFU_EDGEDETECT_128     "XXXXXXXX-XXXX-XXXX-FFFF-0000DFFF0002"

#endif // __AALSDK_AFU_ID_H__

