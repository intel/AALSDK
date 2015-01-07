//******************************************************************************
// Part of the Intel(R) QuickAssist Technology Accelerator Abstraction Layer
//
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2011-2015, Intel Corporation.
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
//  Copyright(c) 2011-2015, Intel Corporation.
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
//        FILE: ccidefs.h
//     CREATED: 12/05/2011
//      AUTHOR: Henry Mitchel
//
// PURPOSE:  This file contains external definitions for the
//           Intel(R) QuickAssist Technology Accelerator Abstraction Layer (AAL)
//           SPL 2 CCI HW interface.
//
//           CCI User Mode fundamental definitions file
//
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 12/05/2011     HM       Initial version started
// 02/06/2012     JG       AAL integration
// 07/30/2014     JG       CCI 3 support
//****************************************************************************
#ifndef __AALSDK_KERNEL_CCIDEFS_H__
#define __AALSDK_KERNEL_CCIDEFS_H__
#include <aalsdk/kernel/aaltypes.h>

/*
 * CCI-specific constants based on QLP2 implementation
 *
 * DSM == Device Status Memory, i.e. a place for the device to communicate to the host
 *
 * All 64-bit CSR's are written in 32-bit halves, upper half first. HW takes writing
 *    of lower 32-bit half as signal that both are written and the entire value is valid.
 *    Note that because these CSRs are in the PCIe configuration space, they are serialized,
 *    so that the above assumption is accurate.
 */


/******************************************************************************
 * Length of Cache Line figures prominently in many calculations
 ******************************************************************************/
#define  lenCL    64

/******************************************************************************
 * 32-bit Feature ID CSR
 ******************************************************************************/
#define byte_offset_PCIE_FEATURE_HDR_F2  0x208
struct PCIE_FEATURE_HDR_F2 {
   union {
      btUnsigned32bitInt csr;              // the entire csr as a 32-bit entity
      struct {
         btUnsigned32bitInt rsvd1:16;
         btUnsigned32bitInt FeatureID:4;   // 0: old CCI, 1: old SPL, 2: new CCI-AFU protocol of self-discovery
                                           //    based on the DSM mechanism described above
         btUnsigned32bitInt rsvd:12;
      };
   };
}; // struct PCIE_FEATURE_HDR_F2
static const int FeatureID_CCI2 = 2;
static const int FeatureID_CCI3 = 3;

#define byte_offset_PCIE_FEATURE_HDR_F3  0x308
struct PCIE_FEATURE_HDR_F3 {
   union {
      btUnsigned32bitInt csr;              // the entire csr as a 32-bit entity
      struct {
         btUnsigned32bitInt rsvd1:16;
         btUnsigned32bitInt protocol:4;    // 0 = CCI; 1 = SPL
                                           //    based on the DSM mechanism described above
         btUnsigned32bitInt rsvd:12;
      };
   };
}; // struct PCIE_FEATURE_HDR_F3


static const int PCIE_FEATURE_HDR3_PROTOCOL_CCI3 = 0;
static const int PCIE_FEATURE_HDR3_PROTOCOL_SPL  = 1;

/******************************************************************************
 * CCI-AFU DSM (Device Status Memory) definition
 ******************************************************************************/
/*
 * Written with 64-bit physical address of the CCI-AFU's DSM
 *
 * CCI-AFU will then respond with an ID at that location to identify itself
 *
 * See spl2defs.h for the details of the structure of the DSM and its ID
 */
//#define byte_offset_CCIAFU_DSM_BASE    0x910

// In documentation, SPL is used instead of the more accurate CCIAFU designation
//static const int byte_offset_SPL_DSM_BASE      =  byte_offset_CCIAFU_DSM_BASE;
// The intention is to make byte_offset_SPL_DSM_BASE initialized by
//    byte_offset_CCIAFU_DSM_BASE, but it causes 'error: initializer element is not constant'
//    error sometimes -- I don't know why.
#define byte_offset_SPL_DSM_BASE       0x1000

/*
 * Actual structure of CCIAFU Device Status Memory area is unknown to CCI,
 *    except for the first cache-line, defined here
 */
struct CCIAFU_DSM {
   union {                                // SPL_ID
      btUnsigned64bitInt CL_0[8];         //    first cache-line
      btUnsigned32bitInt cci_afu_id;      // if cci_afu_id == SPL2_ID, then this cci_afu is SPL 2
   };
};

#endif // __AALSDK_KERNEL_CCIDEFS_H__
