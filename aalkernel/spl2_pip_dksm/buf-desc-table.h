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
//        FILE: buf-desc-table.h
//     CREATED: 10/29/2011
//      AUTHOR: Henry Mitchel
//
// PURPOSE:  This file contains private memory-structure-specific definitions
//           for the Intel(R) QuickAssist Technology Accelerator Abstraction
//           Layer (AAL) SPL 2 Memory Manager Driver.
//
//           Specifically -- the buf_desc_table manages a table of buf_desc's.
//
//           The buf_desc_table provides the basic tool for generalizing
//           kernel buffers for application/accelerator communication while
//           using indices for buffer ids
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 10/29/2011     HM       Initial version started
//****************************************************************************
#ifndef __AALKERNEL_SPL2_PIP_DKSM_BUF_DESC_TABLE_H__
#define __AALKERNEL_SPL2_PIP_DKSM_BUF_DESC_TABLE_H__
#include "buf-desc.h"            // buf_desc

//=============================================================================
// Name:        buf_desc_table
// Description: Structure that defines a table that define many blocks of memory.
//              Free entries (those entries in the table that are not pointing
//                to allocated memory blocks) are kept in free list
//              The array is automatically grown when more entries are needed,
//                but it is never shrunk, as outstanding buffer ID's are
//                indices into the array.
//=============================================================================

struct buf_desc_table
{
   struct buf_desc  *pbuf_desc;     // Kernel virtual pointer to array of buf_desc structures
   unsigned          order;         // Order of buf_desc array in pages, e.g. 0 would be 4096 bytes
   unsigned          max_entries;   // Maximum possible number of entries in the array
   unsigned          num_allocated; // Number of entries in the array that point to
                                    //    allocated buffers. Redundant but easy to track and
                                    //    rather hard to derive.
   int               index_free;    // Points to first entry in free list
                                    //    not valid if array full (e.g. no free list)
                                    // Set to INVALID_INDEX if array is full
}; // struct buf_desc_table


//=============================================================================
// buf_desc_table allocator (kmalloc)
//=============================================================================
static inline struct buf_desc_table *buf_desc_table_alloc (void);

//=============================================================================
// buf_desc_table free (kfree)
//    p_tbl may be NULL
//    return is always NULL
//=============================================================================
static inline struct buf_desc_table *buf_desc_table_free (struct buf_desc_table *p_tbl);

//=============================================================================
// buf_desc_table constructor (initialize the data structure)
//    p_tbl may be NULL, buf_desc_table may have already been initialized
//=============================================================================
static inline int buf_desc_table_construct (struct buf_desc_table *p_tbl, unsigned max_entries);

//=============================================================================
// buf_desc_table destructor (clean up the data structure)
//    p_tbl may be NULL, buf_desc_table may not have been initialized
//=============================================================================
static inline void buf_desc_table_destruct (struct buf_desc_table *p_tbl);

//=============================================================================
// allocate a single buffer to the buf_desc_table, return its index
//    return INVALID_ENTRY on failure (meaning out of memory).
//    p_tbl may be NULL, buf_desc_table may not have been initialized
//=============================================================================
static inline int buf_desc_table_alloc_buf (struct buf_desc_table *p_tbl, unsigned size);

//=============================================================================
// add a buffer descriptor to the buf_desc_table
//    p_tbl may be NULL, buf_desc_table may not have been initialized
//    return its index or INVALID_ENTRY on error (meaning out of memory)
//=============================================================================
static inline int buf_desc_table_add_buf (struct buf_desc_table *p_tbl, const struct buf_desc *p_in);

//=============================================================================
// de-allocate a single buffer in the buf_desc_table, return it to the free-list
//    p_tbl may be NULL, buf_desc_table may not have been initialized
//    return EINVAL on bad index (e.g. double-free)
//=============================================================================
static inline int buf_desc_table_free_buf (struct buf_desc_table *p_tbl, int index);

//=============================================================================
// return a buf_desc based on index
//    p_tbl may be NULL, buf_desc_table may not have been initialized
//    returns NULL on error
//=============================================================================
static inline struct buf_desc *buf_desc_table_get_bufp_from_index (struct buf_desc_table *p_tbl, int index);

//=============================================================================
// determine if buf_desc_table has been initialized
//    p_tbl may be NULL, or buf_desc_table may not have been initialized
//=============================================================================
static inline int buf_desc_table_is_init (struct buf_desc_table *p_tbl);

//=============================================================================
// determine if buf_desc_table contains allocated buffers
//=============================================================================
static inline int buf_desc_table_contains_buffers (struct buf_desc_table *p_tbl);

//=============================================================================
// print the contents of the buf_desc_table
// start and stop are optional indices in the table at which to start and stop
//    and they default to 0 and end of array.
// the actual start and stop values will be rounded down and up so printing
//    can occur on full lines
// p_tbl may be NULL, buf_desc_table may not have been initialized
//=============================================================================
static inline void buf_desc_table_dump (struct buf_desc_table *p_tbl, int start, int stop);

#endif // __AALKERNEL_SPL2_PIP_DKSM_BUF_DESC_TABLE_H__

