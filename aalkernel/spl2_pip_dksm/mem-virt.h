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
//        FILE: mem-virt.h
//     CREATED: 11/04/2011
//      AUTHOR: Henry Mitchel
//
// PURPOSE:  This file contains private memory-structure-specific definitions
//           for the Intel(R) QuickAssist Technology Accelerator Abstraction
//           Layer (AAL) SPL 2 Memory Manager Driver.
//
//           Specifically -- virtual memory page-table definitions
//
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 11/04/2011     HM       Initial version started
//****************************************************************************
#ifndef __AALKERNEL_SPL2_PIP_DKSM_MEM_VIRT_H__
#define __AALKERNEL_SPL2_PIP_DKSM_MEM_VIRT_H__
#include "mem-pte.h"                // Page Table Entry (pte)
#include "buf-desc-table-impl.h"    // buf_desc_table and friends

//=============================================================================
// Name:        memmgr_virtmem
// Description: Structure that defines a page table.
//=============================================================================
struct memmgr_virtmem {
   struct memmgr_pte      *m_pte_array;       // Kernel virtual pointer to page table
   struct buf_desc_table   m_pt_buf_table;    // Describe the buffers that make up the page table

   // Page table meta information
   unsigned long      m_vsize;                // Actual bytes allocated in WorkSpace
   btPhysAddr         m_pte_array_physaddr;   // Physical address of page table
   btUnsigned64bitInt m_pte_array_uvaddr;     // User virtual address of page table
   unsigned           m_pte_array_order;      // Order of the page table as __get_free_pages
                                              //    understands it, e.g. 0 is 4096
   unsigned           m_pte_order_hw;         // Order of each page table entry as
                                              //    as the hw understands it.
                                              //    Typically this will be 12 (4KB) or 21 (2MB)
   unsigned           m_len_super_page;       // Length of a super-page in bytes
   unsigned           m_num_of_valid_pte;     // Number of valid page table entries in the page table
   unsigned           m_max_num_of_pte;       // Number of page table entries allocatable in the page table
}; // struct memmgr_single_mem



//=============================================================================
// virtmem_alloc allocator (kmalloc)
//=============================================================================
struct memmgr_virtmem *virtmem_alloc (void);

//=============================================================================
// virtmem_free free (kfree)
//=============================================================================
struct memmgr_virtmem *virtmem_free (struct memmgr_virtmem *p_virt);

//=============================================================================
// struct memmgr_virtmem constructor (initialize the data structure)
//=============================================================================
int virtmem_construct (struct memmgr_virtmem *p_virt, unsigned long vsize, unsigned order);

//=============================================================================
// struct memmgr_virtmem destructor (clean up the data structure)
//=============================================================================
void virtmem_destruct (struct memmgr_virtmem *p_virt);

//=============================================================================
// determine if struct memmgr_virtmem contains initialized data
//=============================================================================
int virtmem_is_init (struct memmgr_virtmem *p_virt);

//=============================================================================
// determine if struct memmgr_virtmem contains allocated buffers
//=============================================================================
int virtmem_contains_buffers (struct memmgr_virtmem *p_virt);

//=============================================================================
// print out page_table information
//=============================================================================
void virtmem_dump (struct memmgr_virtmem *p_virt, int start, int stop);

#endif // __AALKERNEL_SPL2_PIP_DKSM_MEM_VIRT_H__

