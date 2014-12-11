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
//        FILE: spl2mem.h
//     CREATED: 10/20/2011
//      AUTHOR: Henry Mitchel
//
// PURPOSE:  This file contains external definitions for the
//           Intel(R) QuickAssist Technology Accelerator Abstraction Layer (AAL)
//           SPL 2 Memory Manager Driver.
//
//           This is the USER APPLICATION Include file
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 10/20/2011     HM       Initial version started
// 11/23/2011     HM       Combined memory operations into single structure
//                            with enum to specify operation to minimize
//                            copy_xx_user() code in shims
//****************************************************************************
#ifndef __AALKERNEL_SPL2_PIP_DKSM_SPL2MEM_H__
#define __AALKERNEL_SPL2_PIP_DKSM_SPL2MEM_H__
#include "aalsdk/kernel/kosal.h"


#define SPL2_MEM_NAME "spl2mem"     // /dev name
                                    // Printed identification in Debug statements
                                    // Also used as Class Name in create_class
                                    // Also used as Device Name in create_device

typedef btWSID xsid_t;              // Workspace ID
#define XSID_FMT "0x%llX"           // Printf format string for xsid_t


//-----------------------------------------------------------------------------
// Single Memory Operation structure. Enum uniquely determines contents.
//-----------------------------------------------------------------------------
enum spl2mem_op {
   e_memop_get_config_space=1,            // GetCFG: length is OUT, physaddr is OUT, xsid is OUT.
                                          //    There is only one Configuration Space.
   e_memop_alloc_buf,                     // Alloc:  length is IN/OUT, physaddr is OUT, xsid is OUT
   e_memop_alloc_pagetable,               // VAlloc: length is IN/OUT, xsid is OUT
   e_memop_free,                          // Free:   xsid is IN
};

struct spl2mem_mem_op {                   // For doing anything with a block of memory
   union {
      enum spl2mem_op    e_mem_operation; // IN: Requested operation
      btUnsigned64bitInt rsvd;            // rsvd used only to force padding to minimize 32/64-bit conversion problems
   };
   btWSSize             length;           // IN:  Requested length in bytes
                                          // OUT: Length that was actually allocated
   btPhysAddr           physaddr;         // OUT: Physical address of single buffer or page table allocated
   xsid_t               xsid;             // IN:  Free
                                          // OUT: Workspace identifier for the allocated buffer on alloc
};


//-----------------------------------------------------------------------------
// Debug Memory Operation structure. Enum uniquely determines contents.
//-----------------------------------------------------------------------------

enum debug_command {
   e_dbg_cmd_dump_desc_buf_table,   // Dump to primary buffer descriptor table
   e_dbg_cmd_print_xsid_contents,   // Print an ASCIIZ string starting at an
                                    //    offset into a Workspace at ERROR level
};

enum mem_table {
   e_dbg_memtable_regular,          // Regular memory table, consisting of single buffers
   e_dbg_memtable_virtual,          // Page table
};

struct spl2mem_debug {              // For requesting debug information
   enum debug_command   cmd;        // IN: Command described above
   union {
      struct {                      // For cmd==e_dbg_cmd_dump_desc_buf_table
         int            start;      //    Start location in the buffer table, defaults to beginning of array
         int            stop;       //    Stop location in the buffer table, defaults to end of array (e.g. if use 0)
         enum mem_table mem_type;   //    Which table to dump, individual or page table & associated structures
      };
      struct {                      // For cmd==e_dbg_cmd_print_xsid_contents
         xsid_t         xsid;       // The Workspace to dereference
         btUnsigned64bitInt offset;     // Offset into the workspace to dereference
         btUnsigned64bitInt max_len;    // Maximum length to dereference -- make sure there is a NULL before
                                    //    printing the string
      };
   };
};

#define SPL2MEM_IOCTL_MEM_OP       _IOW   ('x', 0x00, struct spl2mem_mem_op)
#define SPL2MEM_IOCTL_DEBUG        _IOW   ('x', 0x03, struct spl2mem_debug)
#define WRITTEN_FROM_THE_KERNEL    "+ Written from the Kernel" /* what is written back by the e_dbg_cmd_print_xsid_contents cmd */

#endif // __AALKERNEL_SPL2_PIP_DKSM_SPL2MEM_H__

