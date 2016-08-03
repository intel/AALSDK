// Copyright(c) 2013-2016, Intel Corporation
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
// @file diag-defaults.h
// @brief Functionality common to all NLB utils.
// @ingroup
// @verbatim
// Accelerator Abstraction Layer
//
// AUTHORS: Tim Whisonant, Intel Corporation
// 			Sadruta Chandrashekar, Intel Corporation
//
// HISTORY:
// WHEN:          WHO:     WHAT:
// 06/09/2013     TSW      Initial version.
// 01/07/2015	  SC	   fpgadiag version.@endverbatim
//****************************************************************************

#ifndef __DIAG_DEFAULTS_H__
#define __DIAG_DEFAULTS_H__

#include "nlb-specific.h"

#define DEFAULT_BEGINCL     	NLB_MIN_CL
#define DEFAULT_ENDCL       	NLB_MIN_CL
#define DEFAULT_MULTICL       1
#define DEFAULT_DSMPHYS     	0
#define DEFAULT_SRCPHYS     	0
#define DEFAULT_DSTPHYS     	0
#define DEFAULT_WARMFPGACACHE "off"
#define DEFAULT_COOLFPGACACHE "off"
#define DEFAULT_COOLCPUCACHE 	"off"
#define DEFAULT_NOBW        	"off"
#define DEFAULT_TABULAR     	"on"
#define DEFAULT_SUPPRESSHDR 	"off"
#define DEFAULT_WT          	"off"
#define DEFAULT_WB          	"on"
#define DEFAULT_RDS         	"on"
#define DEFAULT_RDI         	"off"
#define DEFAULT_CONT        	"off"
#define DEFAULT_TOUSEC      	0
#define DEFAULT_TOMSEC      	(DEFAULT_NLB_CONT_TIMEOUT_NS / NANOSEC_PER_MILLI(1))
#define DEFAULT_TOSEC       	0
#define DEFAULT_TOMIN       	0
#define DEFAULT_TOHOUR      	0
#define DEFAULT_POLL       	"on"
#define DEFAULT_CSR_WRITE  	"off"
#define DEFAULT_UMSG_DATA  	"off"
#define DEFAULT_UMSG_HINT  	"off"
#define DEFAULT_READ_VA   		"on"
#define DEFAULT_READ_VL0	   "off"
#define DEFAULT_READ_VH0		"off"
#define DEFAULT_READ_VH1		"off"
#define DEFAULT_READ_VR       "off"
#define DEFAULT_WRITE_VA      "on"
#define DEFAULT_WRITE_VL0     "off"
#define DEFAULT_WRITE_VH0     "off"
#define DEFAULT_WRITE_VH1     "off"
#define DEFAULT_WRITE_VR      "off"
#define DEFAULT_WRFENCE_VA    "<WRITE-VC>"
#define DEFAULT_WRFENCE_VL0   "<WRITE-VC>"
#define DEFAULT_WRFENCE_VH0   "<WRITE-VC>"
#define DEFAULT_WRFENCE_VH1   "<WRITE-VC>"
#define DEFAULT_AWP           "off"
#define DEFAULT_ST		   	"off"
#define DEFAULT_UT		   	"on"
#define DEFAULT_CX            2
#define DEFAULT_MINCX         2
#define DEFAULT_MAXCX         0xFFFE
#define DEFAULT_HQW           0
#define DEFAULT_SQW           0
#define DEFAULT_MINHQW        0
#define DEFAULT_MAXHQW        7
#define DEFAULT_MINSQW        0
#define DEFAULT_MAXSQW        7
#define DEFAULT_STRIDES       1
#define DEFAULT_MIN_STRIDES   1
#define DEFAULT_MAX_STRIDES   64
//#define DEFAULT_FPGA_CLK_FREQ 	200000000ULL
#define DEFAULT_BUS_NUMBER      0xFFFFFFFF
#define DEFAULT_DEVICE_NUMBER   0xFFFFFFFF
#define DEFAULT_FUNCTION_NUMBER 0xFFFFFFFF

#endif
