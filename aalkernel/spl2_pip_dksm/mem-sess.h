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
//        FILE: mem-sess.h
//     CREATED: 10/11/2011
//      AUTHOR: Henry Mitchel
//
// PURPOSE:  This file contains private memory-structure-specific definitions
//           for the Intel(R) QuickAssist Technology Accelerator Abstraction
//           Layer (AAL) SPL 2 Memory Manager Driver.
//
//           Specifically -- the internals of the memory data structures
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 10/11/2011     HM       Initial version started
//****************************************************************************
#ifndef __AALKERNEL_SPL2_PIP_DKSM_MEM_SESS_H__
#define __AALKERNEL_SPL2_PIP_DKSM_MEM_SESS_H__
#include "mem-virt.h"         // memmgr_virtmem, buf_desc_table, buf_desc, pte

/* Data structure overview
 *
 * +-----------------------+
 * |memmgr_driver singleton|  No need to link individual processes together
 * +-----------------------+  Currently this is a static singleton
 * |m_session_sem          |  -- not used
 * |m_session_list         |  -- not used
 * +-----------------------+
 *
 * +---------------------------------------+  One of these per process, file-->private data.
 * | memmgr_session                        +  Always created on open.
 * +---------------------------------------+  Always destroyed on close.
 * | pid - process id                      |
 * | regular buffers in a buf_desc_table   |-->buf_desc[]
 * | page table structure                  |
 * |    buf_desc array for each super-page |-->buf_desc[]------->Super-Page
 * |    actual super-page table            |-->pte[]---+         ^
 * +---------------------------------------+           +---------|  // two things point to same actual buffer
 */

//=============================================================================
// Name:        memmgr_session
// Description: Session structure holds state and other context for a user
//              session with the device subsystem. A user session consists of
//              all the information associated with a particular Process ID.
// NOTE:        This is the context structure for all internal operations.
//              It is initialized in mem-fops.c::internal_open()
// NOTE:        Only one Virtual Memory Workspaces is supported.
// NOTE:        Initialized in memmgr_internal_open()
//=============================================================================
#define NUM_VIRT_WORKSPACES 1
struct memmgr_session {
   struct buf_desc_table      m_buf_table;         // table for regular buffers
   struct memmgr_virtmem      m_virtmem;           // description of virtual memory workspace
   xsid_t                     m_xsid_config_space; // Config space xsid. 0 means none.
   pid_t                      m_pid;               // process ID of caller
}; // struct memmgr_session

// Macros to abstract structure details.
#define spl2_memsession_virtmem(p,x)       ((p)->m_virtmem)
// Currently xsid not used. Must be zero
#define spl2_memsession_valid_xsid(x)      (xsid_ctor(0, e_memtype_virtualized) == x)

#endif // __AALKERNEL_SPL2_PIP_DKSM_MEM_SESS_H__

