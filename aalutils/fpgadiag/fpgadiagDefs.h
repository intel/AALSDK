// Copyright (c) 2013-2015, Intel Corporation
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
// @file hwval-inner.h
// @brief <brief>
// @ingroup
// @verbatim
// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
//
// AUTHORS: Tim Whisonant, Intel Corporation
//			Sadruta Chandrashekar, Intel Corporation
//
// HISTORY:
// WHEN:          WHO:     WHAT:
// 07/14/2015     SC      Initial version.@endverbatim
//****************************************************************************
#ifndef __FPGADIAG_DEFS_H__
#define __FPGADIAG_DEFS_H__
#include <unistd.h>  // isatty()
#include <new>       // std::nothrow
#include <string>
#include <cstring>   // memcpy()
#include <algorithm> // min(), max()
#include <iterator>  // iterator
#include <vector>
#include <list>
#include <queue>
#include <map>
#include <iomanip>   // std::setw(), etc.
#include <iostream>
#include <sstream>
#include <fstream>
using namespace std;

#if WITH_GTEST
#include "gtest/gtest_prod.h"
#endif // WITH_GTEST

#include <aalsdk/AALTypes.h>
#include <aalsdk/osal/CriticalSection.h>
#include <aalsdk/osal/Sleep.h>
#include <aalsdk/osal/Thread.h>
#include <aalsdk/osal/Timer.h>


//#define CL(x)                        ((x) * 64)
//#define KB(x)                        ((x) * 1024)
//#define MB(x)                        ((x) * (1024ULL * 1024ULL))
//#define GB(x)                        ((x) * (1024ULL * 1024ULL * 1024ULL))
#define __WKSPC_SIZE_CONST(x)        x##ULL

#define CL_ALIGNED_ADDR(x)           ((x) >> 6)

#define MILLISEC_PER_SEC(x)          ((x) * 1000)

#define MICROSEC_PER_SEC(x)          ((x) * 1000 * 1000)
#define MICROSEC_PER_MILLI(x)        ((x) * 1000)

#define NANOSEC_PER_SEC(x)           ((x) * 1000 * 1000 * 1000)
#define NANOSEC_PER_MILLI(x)         ((x) * 1000 * 1000)
#define NANOSEC_PER_MICRO(x)         ((x) * 1000)

#define KHZ(x)                       ((x) * 1000ULL)
#define MHZ(x)                       ((x) * 1000000ULL)
#define GHZ(x)                       ((x) * 1000000000ULL)
#define __FREQ_CONST(x)              x##ULL

#define DEFAULT_FPGA_CLK_FREQ        MHZ(200)
#define DEFAULT_NLB_CONT_TIMEOUT_SEC 0
#define DEFAULT_NLB_CONT_TIMEOUT_NS  NANOSEC_PER_MILLI(1)
#define DEFAULT_NLB_DSM_SIZE         ((wkspc_size_type)MB(4))
#define DEFAULT_NLB_WKSPC_SIZE       CL(1)
//#define DEFAULT_NLB_CACHE_INIT       INLBVAFU::eNLBCI_FILL_NONE
//#define DEFAULT_NLB_WRITE_TYPE       INLBVAFU::eNLBWT_WRITE_BACK
#define DEFAULT_NLB_POSTED_WRITES    false

#define NULL_WKSPC_TAGNAME           "<NULL Wkspc>"

// When using cout << utility(Printable, <uint_type>) [see PrintFormatter.h],
//  <uint_type>, which is normally meant to specify the Level 1 indentation, instead
//  specifies the following bit flags:

// Output in tabular mode, like the legacy NLBTest output.
#define NLB_TABULAR(x) (NLB_TABULAR_FLAG | (x))
#define NLB_TABULAR_FLAG   0x80000000

#define NLB_TABULAR_NO_BW  0x00000001
#define NLB_TABULAR_DO_HDR 0x00000002

typedef AAL::btUnsigned64bitInt wkspc_size_type;
typedef AAL::btUnsigned64bitInt freq_type;
typedef AAL::btUnsigned32bitInt size_type;
typedef AAL::btVirtAddr         virt_type;
typedef AAL::btPhysAddr         phys_type;
typedef AAL::btUnsigned32bitInt csr_type;

typedef AAL::bt32bitInt         sint_type;
typedef AAL::btUnsigned32bitInt uint_type;

typedef AAL::btByte             u8_type;
typedef AAL::btUnsigned16bitInt u16_type;
typedef AAL::btUnsigned32bitInt u32_type;
typedef AAL::btUnsigned64bitInt u64_type;

#if (8 == sizeof_void_ptr)
# define ptr_to_u32(p) (u32_type)((u64_type)(p))
#else
# define ptr_to_u32(p) (u32_type)(p)
#endif // sizeof_void_ptr


#endif // __FPGADIAG_DEFS_H__

