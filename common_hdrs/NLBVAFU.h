//******************************************************************************
// Part of the Intel(R) QuickAssist Technology Accelerator Abstraction Layer
//
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2015, Intel Corporation.
//
//  This program  is  free software;  you  can redistribute it  and/or  modify
//  it  under  the  terms of  version 2 of  the GNU General Public License  as
//  published by the Free Software Foundation.
//
//  This  program  is distributed  in the  hope that it  will  be useful,  but
//  WITHOUT   ANY   WARRANTY;   without   even  the   implied   warranty    of
//  MERCHANTABILITY  or  FITNESS  FOR  A  PARTICULAR  PURPOSE.  See  the   GNU
//  General Public License for more details.
//
//  The  full  GNU  General Public License is  included in  this  distribution
//  in the file called README.GPLV2-LICENSE.TXT.
//
//  Contact Information:
//  Henry Mitchel, henry.mitchel at intel.com
//  77 Reed Rd., Hudson, MA  01749
//
//                                BSD LICENSE
//
//  Copyright(c) 2015, Intel Corporation.
//
//  Redistribution and  use  in source  and  binary  forms,  with  or  without
//  modification,  are   permitted  provided  that  the  following  conditions
//  are met:
//
//    * Redistributions  of  source  code  must  retain  the  above  copyright
//      notice, this list of conditions and the following disclaimer.
//    * Redistributions in  binary form  must  reproduce  the  above copyright
//      notice,  this  list of  conditions  and  the  following disclaimer  in
//      the   documentation   and/or   other   materials   provided  with  the
//      distribution.
//    * Neither   the  name   of  Intel  Corporation  nor  the  names  of  its
//      contributors  may  be  used  to  endorse  or promote  products derived
//      from this software without specific prior written permission.
//
//  THIS  SOFTWARE  IS  PROVIDED  BY  THE  COPYRIGHT HOLDERS  AND CONTRIBUTORS
//  "AS IS"  AND  ANY  EXPRESS  OR  IMPLIED  WARRANTIES,  INCLUDING,  BUT  NOT
//  LIMITED  TO, THE  IMPLIED WARRANTIES OF  MERCHANTABILITY  AND FITNESS  FOR
//  A  PARTICULAR  PURPOSE  ARE  DISCLAIMED.  IN NO EVENT  SHALL THE COPYRIGHT
//  OWNER OR CONTRIBUTORS BE LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,
//  SPECIAL,  EXEMPLARY,  OR  CONSEQUENTIAL   DAMAGES  (INCLUDING,   BUT   NOT
//  LIMITED  TO,  PROCUREMENT  OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF USE,
//  DATA,  OR PROFITS;  OR BUSINESS INTERRUPTION)  HOWEVER  CAUSED  AND ON ANY
//  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT LIABILITY,  OR TORT
//  (INCLUDING  NEGLIGENCE  OR OTHERWISE) ARISING  IN ANY WAY  OUT  OF THE USE
//  OF  THIS  SOFTWARE, EVEN IF ADVISED  OF  THE  POSSIBILITY  OF SUCH DAMAGE.
//******************************************************************************
//****************************************************************************
/// @file NLBVAFU.h
/// @brief Native Loopback Validation AFU definitions.
/// @ingroup Events
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHOR: Tim Whisonant, Intel Corporation
///
/// HISTORY:
/// COMMENTS:
/// WHEN:          WHO:     WHAT:
/// 01/28/2015     TSW      Initial version@endverbatim
//****************************************************************************
#ifndef __AALSDK_KERNEL_NLBVAFU_H__
#define __AALSDK_KERNEL_NLBVAFU_H__

// This file is shared across user and kernel space.
#if defined( __AAL_KERNEL__ )
# include <aalsdk/kernel/aaltypes.h>
#else
# include <aalsdk/AALTypes.h>
#endif // __AAL_USER__


BEGIN_NAMESPACE(AAL)

#define MAX_NLB_LPBK1_WKSPC       CL(16384)
#define MAX_NLB_READ_WKSPC        CL(16384)
#define MAX_NLB_WRITE_WKSPC       CL(16384)
#define MAX_NLB_TRPUT_WKSPC       CL(16384)
#define NLB_DSM_SIZE              MB(4)

#define QLP_CSR_CIPUCTL           0x280
#   define CIPUCTL_RESET_BIT      0x01000000

#define QLP_CSR_CAFU_STATUS       0x284
#   define CAFU_STATUS_READY_BIT  0x80000000

#define QLP_NUM_COUNTERS          11
#define QLP_CSR_ADDR_PERF1C       0x27c
#define QLP_CSR_ADDR_PERF1        0x28c
#   define QLP_PERF_CACHE_RD_HITS 0
#   define QLP_PERF_CACHE_WR_HITS 1
#   define QLP_PERF_CACHE_RD_MISS 2
#   define QLP_PERF_CACHE_WR_MISS 3
#   define QLP_PERF_EVICTIONS     10

#define CSR_AFU_DSM_BASEL         0x1a00
#define CSR_AFU_DSM_BASEH         0x1a04
#define CSR_SRC_ADDR              0x1a20
#define CSR_DST_ADDR              0x1a24
#define CSR_NUM_LINES             0x1a28
#define CSR_CTL                   0x1a2c
#define CSR_CFG                   0x1a34
#   define NLB_TEST_MODE_LPBK1    0x0
#   define NLB_TEST_MODE_CONT     0x2
#   define NLB_TEST_MODE_READ     0x4
#   define NLB_TEST_MODE_WRITE    0x8
#   define NLB_TEST_MODE_TRPUT    0xc
#   define NLB_TEST_MODE_MASK     0x1c

typedef struct _nlb_vafu_dsm
{
/* 0x00-0x0f */ btUnsigned32bitInt afuid[4];     // c000c9660d8242729aeffe5f84570612
/* 0x10-0x3f */ btUnsigned32bitInt _reserved[12];
/* -- cache line -- */
/* 0x40      */ btUnsigned32bitInt test_complete;
/* 0x44      */ btUnsigned32bitInt test_error;
/* 0x48      */ btUnsigned64bitInt num_clocks;
/* 0x50      */ btUnsigned32bitInt num_reads;
/* 0x54      */ btUnsigned32bitInt num_writes;
/* 0x58      */ btUnsigned32bitInt start_overhead;
/* 0x5c      */ btUnsigned32bitInt end_overhead;
/* 0x60-0x7f */ btUnsigned32bitInt mode_error[8];
/* -- cache line -- */
} nlb_vafu_dsm;
CASSERT(sizeof(nlb_vafu_dsm) == CL(2));

END_NAMESPACE(AAL)

#endif // __AALSDK_KERNEL_NLBVAFU_H__

