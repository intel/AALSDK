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
//        FILE: buf-desc.h
//     CREATED: 10/29/2011
//      AUTHOR: Henry Mitchel
//
// PURPOSE:  This file contains private memory-structure-specific definitions
//           for the Intel(R) QuickAssist Technology Accelerator Abstraction
//           Layer (AAL) SPL 2 Memory Manager Driver.
//
//           Specifically -- buf_desc is the fundamental descriptor of a
//           memory buffer, containing both its physical and kernel virtual
//           pointers, its type (memory or memory_mapped_io), whether this
//           particular entry is allocated or not, and if not, then the index
//           of the next free element.
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 10/29/2011     HM       Initial version started
//****************************************************************************
#ifndef __AALKERNEL_SPL2_PIP_DKSM_BUF_DESC_H__
#define __AALKERNEL_SPL2_PIP_DKSM_BUF_DESC_H__
#include "aalsdk/kernel/kosal.h"
#include "buf-mem-type.h"        // enum mem_type

/* Generically used to designate the end of a free list implemented as indices
 *    in an array, or any invalid array index.
 */
static const int INVALID_ENTRY = -1;

#if (1 == ENABLE_DEBUG)
   // value to which a buffer will be set after allocation
   static const unsigned char buf_init_value = 0xBE;
   // value to which a buffer will be set before freeing back to OS
   static const unsigned char buf_free_value = 0xAF;
#else
   static const unsigned char buf_init_value = 0x00;
   static const unsigned char buf_free_value = 0x00;
#endif // ENABLE_DEBUG


/* buf_desc contains all the information needed to describe a single physically
 *    contiguous block of memory.
 *
 * It is designed to be used in a heap implemented as a list in an array, so
 *    when it is "NOT free" it contains the data structure necessary to describe
 *    a block of memory. When "free", however, it contains the index of the next
 *    item on the free list.
 *
 * In a slightly tweaked implementation, this could be a list pointer, but in
 *    that case it would probably make more sense to make it a list_head, in
 *    which case it might be bulkier.
 *
 * Typical usage sequence:
 *    A loop of buf_desc_init_next() calls to initialize an array of buf_desc
 *    For each buf_desc that is actually allocated, a pair of calls to
 *       initialize it:
 *          buf_desc_set_virtaddr_and_type()
 *          buf_desc_set_physaddr_and_order()
 *       Those two calls are the most efficient, but if it not convenient to
 *          call them with that information, all the data can be added a field
 *          at a time in any order. To wit:
 *             buf_desc_set_type()
 *             buf_desc_set_virtaddr()
 *             buf_desc_set_order()
 *             buf_desc_set_physaddr()
 *    When the buffer described by the buf_desc is freed, re-initialize it using:
 *       buf_desc_init_next()
 *
 * NOTE: if changing buf_desc, be aware that subsequent usage assumes it fits
 *    nicely into power of two arrays, so its size should be a power of two.
 */

//=============================================================================
// Name:        buf_desc
// Description: Structure that defines a block of memory.
//=============================================================================

//=============================================================================
// 16-byte version -- ugly but probably worth the tradeoff for memory
//=============================================================================


// ** TODO This needs to be fixed - we cannot safely peek inside an address and manipulate fields! This is extremely unportable. **

struct buf_desc {
   union {                                // First 8 bytes.
      btUnsigned64bitInt m_virtaddr;        // Kernel virtual, returned from kmalloc or get_page (8 bytes).
                                          //    Only high 58 bits are valid, have to mask off lower bits.
                                          // Address is byte address, but cache-aligned, leaving 6 low bits free.
      struct {
         btUnsigned64bitInt      m_free:1;          // Boolean true if this is a free entry
         enum mem_type m_type:2;          // What kind of memory is it. This will never be set to e_memtype_virtualized
                                          //    because a buf_desc describes only a single physically contiguous
                                          //    chunk of address space, either RAM or IO
         btUnsigned64bitInt      m_rsvd:3;          // For future use
      };
   };
   union {                                // Second 8 bytes.
      btUnsigned64bitInt         m_physaddr;        // Physical address of block (8 bytes).
                                          //    Only high 58 bits are valid, have to mask off lower bits.
                                          // Address is byte address, but cache-aligned
      btUnsigned32bitInt         m_order:6;         // get_order() order of the block pointed to.
                                          //    E.g. 0 = 4K. Coexists with physaddr as its lower bits.
      bt32bitInt          m_next_free_index; // If this entry is free, this is the
                                          //    index of the next free entry in the chain (4 bytes)
                                          // If this is the last entry on the free list, use INVALID_INDEX.
   };
}; // struct buf_desc

//=============================================================================
// buf_desc constructors
//=============================================================================
static inline void buf_desc_default_ctor        ( struct buf_desc *pdesc)
{
   // Implementation specific but fast
   pdesc->m_virtaddr = 1UL;            // virtaddr = 0, m_free = 1, m_type invalid.

   // Cleaner
   // pdesc->m_virtaddr = 0;
   // buf_desc_set_free( pdesc, 1);

   pdesc->m_physaddr = 0;
}
// use when initializing an array of these and setting up the free list
static inline void buf_desc_init_next           ( struct buf_desc *pdesc, int nextfree)
{
   buf_desc_default_ctor(pdesc);
   pdesc->m_next_free_index = nextfree;
}

//=============================================================================
// buf_desc accessors
//=============================================================================
static inline btUnsigned64bitInt buf_desc_get_virtaddr    ( struct buf_desc *pdesc)
{
   return pdesc->m_virtaddr & ~0x3FUL;
}
static inline unsigned buf_desc_get_free        ( struct buf_desc *pdesc)
{
   return pdesc->m_free;
}
static inline enum mem_type buf_desc_get_type   ( struct buf_desc *pdesc)
{
   return pdesc->m_type;
}
static inline unsigned buf_desc_istype_mem      ( struct buf_desc *pdesc)
{
   return pdesc->m_type == e_memtype_regular;
}
static inline unsigned buf_desc_istype_io       ( struct buf_desc *pdesc)
{
   return pdesc->m_type == e_memtype_IO;
}
static inline btUnsigned64bitInt buf_desc_get_physaddr    ( struct buf_desc *pdesc)
{
   return pdesc->m_physaddr&~0x3FUL;
}
static inline btUnsigned64bitInt buf_desc_get_physpfn     ( struct buf_desc *pdesc)
{
   return buf_desc_get_physaddr(pdesc) >> PAGE_SHIFT;
}
static inline unsigned buf_desc_get_order       ( struct buf_desc *pdesc)
{
   return pdesc->m_order;
}
static inline unsigned long buf_desc_get_size   ( struct buf_desc *pdesc)
{
   return PAGE_SIZE << buf_desc_get_order( pdesc);
}
static inline int buf_desc_get_freeindex        ( struct buf_desc *pdesc)
{
   return pdesc->m_next_free_index;
}

//=============================================================================
// buf_desc mutators
//=============================================================================

static inline void buf_desc_set_type         ( struct buf_desc *pdesc, enum mem_type type)
{
   pdesc->m_type = type;
}
#if 0
   //unused so far
   //static inline void buf_desc_set_virtaddr     ( struct buf_desc *pdesc, unsigned long vaddr)
   //{
   //   unsigned temp = buf_desc_get_type( pdesc);
   //   pdesc->m_virtaddr = vaddr & ~0x3FUL;
   //   buf_desc_set_type(pdesc, temp);
   //}
#endif
static inline void buf_desc_set_virtaddr_and_type( struct buf_desc *pdesc,
                                                   unsigned long    vaddr,
                                                   enum mem_type    type)
{
   pdesc->m_virtaddr = vaddr & ~0x3FUL;
   buf_desc_set_type(pdesc, type);
}
#if 0
   //unused so far
   //static inline void buf_desc_set_free         ( struct buf_desc *pdesc, unsigned free)
   //{
   //   // probably not actually useful, use buf_desc_default_ctor() instead
   //   pdesc->m_free = free;
   //}
#endif
static inline void buf_desc_set_order         ( struct buf_desc *pdesc, unsigned order)
{
   pdesc->m_order = order;
}
#if 0
   //unused so far
   //static inline void buf_desc_set_physaddr     ( struct buf_desc *pdesc, btPhysAddr physaddr)
   //{
   //   unsigned order = buf_desc_get_order( pdesc);
   //   pdesc->m_physaddr = physaddr & ~0x3FUL;
   //   buf_desc_set_order(pdesc, order);
   //}
#endif
static inline void buf_desc_set_physaddr_and_order( struct buf_desc *pdesc,
                                                    unsigned long    physaddr,
                                                    unsigned         order)
{  // slightly more efficient
   pdesc->m_physaddr = physaddr&~0x3FUL;
   buf_desc_set_order(pdesc, order);
}
#if 0
   //unused so far
   //static inline void buf_desc_set_freeindex     ( struct buf_desc *pdesc, int nextfree)
   //{
   //   // probably want to use buf_desc_init_next() instead
   //   pdesc->m_next_free_index = nextfree;
   //}
#endif

#endif // __AALKERNEL_SPL2_PIP_DKSM_BUF_DESC_H__

