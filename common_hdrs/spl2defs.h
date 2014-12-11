//******************************************************************************
// Part of the Intel(R) QuickAssist Technology Accelerator Abstraction Layer
//
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2011-2014, Intel Corporation.
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
//  Copyright(c) 2011-2014, Intel Corporation.
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
//        FILE: spl2defs.h
//     CREATED: 12/05/2011
//      AUTHOR: Henry Mitchel
//
// PURPOSE:  This file contains external definitions for the
//           Intel(R) QuickAssist Technology Accelerator Abstraction Layer (AAL)
//           SPL 2 HW interface.
//
//           SPL 2 User Mode fundamental definitions file
//
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 12/05/2011     HM       Initial version started
// 02/06/2012     JG       AAL integration
// 04/05/2012     TSW      Add CASSERT's for struct sizes
// 07/30/2014     JG       CCI 3 support
//****************************************************************************
#ifndef __AALSDK_KERNEL_SPL2DEFS_H__
#define __AALSDK_KERNEL_SPL2DEFS_H__
#include <aalsdk/kernel/aaltypes.h>
#include <aalsdk/kernel/ccidefs.h> // struct CCIAFU_DSM

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

/*****************************************************************************
 *****************************************************************************
 * 32-bit CSR that controls CCI
 *****************************************************************************
 *****************************************************************************/
#define byte_offset_CCI_CH_CTRL  0x280      // CIPUCTL

struct CCI_CH_CTRL {
   union {
      btUnsigned32bitInt csr;           // the entire csr as a 32-bit entity
      struct {
         btUnsigned32bitInt Reset:   1; // writing 0 has no effect, writing 1 causes SPL channel & AFU reset
         btUnsigned32bitInt Enable:  1; // writing 0 disables SPL channel and AFU,
                              //    writing 1 re-enables them to pick up where they left off
         btUnsigned32bitInt rsvd:   30;
      };
   };
}; // struct CCI_CH_CTRL
CASSERT(sizeof(struct CCI_CH_CTRL) == 4);

#define byte_offset_CCI_CH_STAT0  0x284      // CIPUSTAT0

#define byte_offset_CCI_CH_STAT1  0x288      // CIPUSTAT1


#define byte_offset_CSR_AFU_DSM_BASE 0x1a00


/*
 * SPL 2-specific constants based on QLP2 implementation
 *
 * DSM == Device Status Memory, i.e. a place for the device to communicate to the host
 *
 * All 64-bit CSR's are written in 32-bit halves, upper half first. HW takes writing
 *    of lower 32-bit half as signal that both are written and the entire value is valid.
 *    Note that because these CSRs are in the PCIe configuration space, they are serialized,
 *    so that the above assumption is accurate.
 */

/*****************************************************************************
 *****************************************************************************
 * SPL 2's CONTEXT STRUCTURE, which defines an SPL 2 task
 *****************************************************************************
 *****************************************************************************/
/*
 * Written with the 64-bit physical address of SPL 2's context structure
 */
#define byte_offset_SPL2_CNTXT_BASE  0x1008

/*
 * The structure that goes there is the SPL2_CNTXT structure
 */
struct SPL2_CNTXT {
   btUnsigned64bitInt phys_addr_page_table;      // physical byte address, low 6 bits 0
   btUnsigned64bitInt virt_addr_afu_context;     // user virtual address of AFU's
                                       //    workspace, low 6 bits 0
                                       // same value that is written to the
                                       //    byte_offset_AFU_CNTXT_BASE csr
   // 3rd qword, low 32-bits
   union {
      btUnsigned32bitInt page_table_flags;
      struct {
         btUnsigned32bitInt page_size: 1;        // 1 = 2 MB, 0 = 4 KB. ONLY 2MB is valid.
         btUnsigned32bitInt rsvd1:    31;
      };
   };
   // 3rd qword, high 32-bits
   btUnsigned32bitInt num_valid_ptes;            // number of valid pte's in page-table

   // 4th qword
   union {
      btUnsigned64bitInt control_flags;
      struct {
         btUnsigned64bitInt mode:      1;        // 0 = physical, 1 = virtual.
                                                 //    Only virtual allowed
         btUnsigned64bitInt rsvd2:    63;
      };
   };

   // 5th qword
   btUnsigned64bitInt sw_handle;                 // for SW use, un-touched by HW

   // 6th qword
   btUnsigned64bitInt afu_dsm_phys;              // Physical address of AFU DSM

   // rest of the cacheline
   btUnsigned64bitInt rsvd3[2];                  // qwords 7-8
}; // struct SPL2_CNTXT
CASSERT(sizeof(struct SPL2_CNTXT) == 64);

/*****************************************************************************
 *****************************************************************************
 * 32-bit CSR that controls SPL2
 *****************************************************************************
 *****************************************************************************/
#define byte_offset_SPL2_CH_CTRL 0x1010
struct SPL2_CH_CTRL {
   union {
      btUnsigned32bitInt csr;           // the entire csr as a 32-bit entity
      struct {
         btUnsigned32bitInt Reset:   1; // writing 0 has no effect, writing 1 causes SPL channel & AFU reset
         btUnsigned32bitInt Enable:  1; // writing 0 disables SPL channel and AFU,
                              //    writing 1 re-enables them to pick up where they left off
         btUnsigned32bitInt rsvd:   30;
      };
   };
}; // struct SPL2_CH_CTRL
CASSERT(sizeof(struct SPL2_CH_CTRL) == 4);

/*****************************************************************************
 *****************************************************************************
 * SPL2_DSM data structure, needs to be aligned on 4K boundary
 *****************************************************************************
 *****************************************************************************/

/*
 * This is what resides at the address defined in ccidefs.h, as
 *    static const int byte_offset_CCIAFU_DSM_BASE   =  0x910;
 */

// SPL 2 id
#define SPL2_ID 0x11100101

// If there is an error, what type of error was it?
enum e_SPL_Status_Error {
   PTE_Load_Error = 1,
   PT_Access_Violation,
};

// If there is a fault, what type of request caused it?
enum e_SPL_Error_Request_Type {
   Read = 1,
   Write,
};

// Actual structure of SPL 2 Device Status Memory area is two cache-lines long
struct SPL2_DSM {
   // first cache-line
   struct CCIAFU_DSM cci;                 // SPL_ID, first cache-line
                                          //    cci.cci_afu_id == SPL2_ID ?
   // second cache-line
   union {                                // SPL_Status
      btUnsigned64bitInt CL_1[8];                   //    second cache-line
      struct {
         // first qword
         btUnsigned64bitInt Valid:               1; // true if the task has terminated
         btUnsigned64bitInt Error:               1; // true if this termination is in error
         btUnsigned64bitInt rsvd1:               6;
         enum e_SPL_Status_Error
                  Error_Status:       24; // Valid if Error is true
         enum e_SPL_Error_Request_Type
                  Fault_Request_Type: 32; // Valid if Error is true

         // second qword
         btUnsigned64bitInt Error_Address;          // access to this 64-bit virtual
                                          //    address caused the error
      };
   };
}; // struct SPL2_DSM
CASSERT(sizeof(struct SPL2_DSM) == (2 * 64));

/*****************************************************************************
 *****************************************************************************
 * SPL2 expects its AFU to follow certain conventions, defined here
 *****************************************************************************
 *****************************************************************************/

/*****************************************************************************
 * Written with 64-bit physical address of SPL's AFU's DSM (Device Status Memory)
 *****************************************************************************/
#define byte_offset_AFU_DSM_BASE    0x8A00

/*****************************************************************************
 * SPL 2 expects the AFU's DSM to contain one cache-line, reserved by SPL 2,
 *    for the AFU's AFU_ID
 *****************************************************************************/
struct SPL2AFU_DSM{
   btUnsigned64bitInt AFU_ID[2];        // 128 bit GUID. Current implementation only
                              //    uses lowest 64-bits, i.e. AFU_ID[0]
   btUnsigned64bitInt rsvd[6];
};
CASSERT(sizeof(struct SPL2AFU_DSM) == 64);

/*****************************************************************************
 * Written with 64-bit virtual address into which the AFU's CNTXT (Page Table)
 *    member PTE's have been mmap()'d. This is the same value written into
 *    the SPL2_CNTXT structure member virt_addr_afu_context.
 * The mechanism of writing the same value twice allows the SPL not to have
 *    to send the value to the AFU -- instead the SW does.
 *****************************************************************************/
#define byte_offset_AFU_CNTXT_BASE   0x8A08

/*****************************************************************************
 * CCI CSR Offsets from system config space based on AFU number
 *****************************************************************************/
static const int CCI3_AFU0_CSR_OFFSET=0x1000;


/*****************************************************************************
 * SPL3 CSR Offsets from system config space based on AFU number
 *****************************************************************************/
static const int SPL3_AFU0_CSR_OFFSET=0x8000;
static const int SPL3_AFU1_CSR_OFFSET=0x9000;
static const int SPL3_AFU2_CSR_OFFSET=0xA000;
static const int SPL3_AFU3_CSR_OFFSET=0xB000;

static const int SPL3_CSR_SPACE_SIZE=0x1000;

#endif // __AALSDK_KERNEL_SPL2DEFS_H__

