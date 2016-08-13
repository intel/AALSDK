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
/// @file NLBVAFU.h
/// @brief Native Loopback Validation AFU definitions.
/// @ingroup NLBVAFU
/// @verbatim
/// Accelerator Abstraction Layer
///
/// AUTHOR: Tim Whisonant, Intel Corporation
///
/// HISTORY:
/// COMMENTS:
/// WHEN:          WHO:     WHAT:
/// 01/28/2015     TSW      Initial version@endverbatim
//****************************************************************************
#ifndef __AALSDK_UTILS_NLBVAFU_H__
#define __AALSDK_UTILS_NLBVAFU_H__

// This file can be shared across user and kernel space.
#if defined( __AAL_KERNEL__ )
# include <aalsdk/kernel/aaltypes.h>
#else
# include <aalsdk/AALTypes.h>
#endif // __AAL_USER__


BEGIN_NAMESPACE(AAL)

#define MAX_NLB_WKSPC_SIZE        CL(65536)
#define MAX_NLB_LPBK1_WKSPC       CL(65536)
#define MAX_NLB_READ_WKSPC        CL(65536)
#define MAX_NLB_WRITE_WKSPC       CL(65536)
#define MAX_NLB_TRPUT_WKSPC       CL(65536)
#define MAX_NLB_SW_WKSPC          CL(65536)
#define MAX_NLB_CCIP_LPBK1_WKSPC  CL(65536)
#define MAX_NLB_CCIP_READ_WKSPC   CL(65536)
#define MAX_NLB_CCIP_WRITE_WKSPC  CL(65536)
#define MAX_NLB_CCIP_TRPUT_WKSPC  CL(65536)
#define MAX_NLB_CCIP_SW_WKSPC  	  CL(65536)
#define NLB_DSM_SIZE              MB(4)
#define NUM_UMSGS				  32
#define MAX_UMSG_SIZE        	  CL(64*NUM_UMSGS)
#define MAX_CPU_CACHE_SIZE		  (100*1024*1024) //100 MB

#define QLP_CSR_CIPUCTL           0x280
#   define CIPUCTL_RESET_BIT      0x01000000

#define QLP_CSR_CAFU_STATUS       0x284
#   define CAFU_STATUS_READY_BIT  0x80000000
# define CAFU_CACHE_FLUSH_STATUS  0x00000010

#define QLP_NUM_COUNTERS          11
#define QLP_CSR_ADDR_PERF1C       0x27c
#define QLP_CSR_ADDR_PERF1        0x28c
#   define QLP_PERF_CACHE_RD_HITS 0
#   define QLP_PERF_CACHE_WR_HITS 1
#   define QLP_PERF_CACHE_RD_MISS 2
#   define QLP_PERF_CACHE_WR_MISS 3
#   define QLP_PERF_EVICTIONS     10

#define NUM_PERF_MONITORS         13

#define CSR_AFU_DSM_BASEL         	0x0110
#define CSR_AFU_DSM_BASEH         	0x0114
#define CSR_SRC_ADDR              	0x0120
#define CSR_DST_ADDR              	0x0128
#define CSR_NUM_LINES             	0x0130
#define CSR_CTL                   	0x0138
#define CSR_CFG                   	0x0140
#define CSR_CFG_H                  	0x0144
#define CSR_UMSG_BASE				0x03F4
#define CSR_UMSG_MODE				0x03F8
#define CSR_CIRBSTAT				0x0278
#define CSR_SW_NOTICE				0x0158
#define CSR_STRIDED_ACS             0x0178

#   define NLB_TEST_MODE_LPBK1    	0x000
#   define NLB_TEST_MODE_WRLINE_M  	0x000	//Write-back
#   define NLB_TEST_MODE_WRLINE_I  	0x001	//Write-through
#   define NLB_TEST_MODE_CONT     	0x002
#   define NLB_TEST_MODE_READ     	0x004
#   define NLB_TEST_MODE_WRITE    	0x008
#   define NLB_TEST_MODE_TRPUT    	0x00c
#   define NLB_TEST_MODE_ATOMIC    	0x014
#   define NLB_TEST_MODE_LPBK3    	0x018
#   define NLB_TEST_MODE_SW         0x01c
#   define NLB_TEST_MODE_MASK     	0x01c
#   define NLB_TEST_MODE_RDS      	0x000
#   define NLB_TEST_MODE_RDI      	0x200
//#   define NLB_TEST_MODE_RDO      	0x400
#	define NLB_TEST_MODE_UMSG_POLL	  0x0000000
#	define NLB_TEST_MODE_CSR_WRITE    0x4000000
#	define NLB_TEST_MODE_UMSG_DATA    0x8000000
#	define NLB_TEST_MODE_UMSG_HINT    0xc000000
#	define NLB_TEST_MODE_VA           0x0000
#	define NLB_TEST_MODE_READ_VL0	  0x1000
#	define NLB_TEST_MODE_READ_VH0	  0x2000
#	define NLB_TEST_MODE_READ_VH1	  0x3000
#   define NLB_TEST_MODE_READ_VR      0x4000
#   define NLB_TEST_MODE_ALT_WR_PRN   0x8000
#   define NLB_TEST_MODE_MCL2         0x0020
#   define NLB_TEST_MODE_MCL4         0x0060
#   define NLB_TEST_MODE_WRPUSH_I     0x10000
#   define NLB_TEST_MODE_WRITE_VL0    0x20000
#   define NLB_TEST_MODE_WRITE_VH0    0x40000
#   define NLB_TEST_MODE_WRITE_VH1    0x60000
#   define NLB_TEST_MODE_WRITE_VR     0x80000
#   define NLB_TEST_MODE_WRFENCE_VA   0x0
#   define NLB_TEST_MODE_WRFENCE_VL0  0x40000000
#   define NLB_TEST_MODE_WRFENCE_VH0  0x80000000
#   define NLB_TEST_MODE_WRFENCE_VH1  0xc0000000


#define TEST_MODE_CSR_HQW		 	0x000E0000

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

#endif // __AALSDK_UTILS_NLBVAFU_H__

