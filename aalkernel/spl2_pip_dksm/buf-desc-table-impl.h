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
//        FILE: buf-desc-table.c
//     CREATED: 10/29/2011
//      AUTHOR: Henry Mitchel
//
// PURPOSE:  This file contains private memory-structure-specific definitions
//           for the Intel(R) QuickAssist Technology Accelerator Abstraction
//           Layer (AAL) SPL 2 Memory Manager Driver.
//
//           Specifically -- implementation of the buf_desc_table
//
//           The buf_desc_table provides the basic tool for generalizing
//           kernel buffers for application/accelerator communication while
//           using indices for buffer ids
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 10/30/2011     HM       Initial version started
// 11/06/2011     HM       Added explicit memset after allocation and before
//                            free to handle potential security concerns
//****************************************************************************
#ifndef __AALKERNEL_SPL2_PIP_DKSM_BUF_DESC_TABLE_IMPL_H_
#define __AALKERNEL_SPL2_PIP_DKSM_BUF_DESC_TABLE_IMPL_H_
#include "aalsdk/kernel/kosal.h"

#include "buf-desc-table.h"   // buf_desc_table
#include "spl2mem.h"          // External definitions, user interface
#include "kernver-utils.h"    // Kernel version utilities


/**
 * buf_desc_table_alloc - allocate a buf_desc_table
 * @return: pointer to the structure or NULL if out of memory
 */
struct buf_desc_table *buf_desc_table_alloc(void)
{
   struct buf_desc_table *p_tbl; // kmalloc'd and returned,
                                 //    freed in buf_desc_table_destroy
   PTRACEIN;

   // allocate buf_desc_table
   p_tbl = kmalloc(sizeof(struct buf_desc_table), GFP_KERNEL);
   ASSERT(p_tbl);


   PTRACEOUT_PTR(p_tbl);
   return p_tbl;
}  // buf_desc_table_alloc

/**
 * buf_desc_table_free - buf_desc_table destructor
 * @p_tbl:  the address of the buf_desc_table to free
 * @return: NULL to set the pointer to if needed
 *
 * p_tbl may be NULL , or p_tbl may point to an initialized buf_desc_table -- it is checked
 */
struct buf_desc_table *buf_desc_table_free(struct buf_desc_table *p_tbl)
{
   PTRACEIN;

   // parameter checks
   ASSERT(p_tbl);
   if( p_tbl ) {
      // free the buf_desc_table
      PVERBOSE("About to free buf_desc_table at address %p\n", p_tbl);
      kfree(p_tbl);
   }

   PTRACEOUT;
   return NULL;
}  // buf_desc_table_free

/**
 * buf_desc_table_construct - buf_desc_table constructor
 * @p_tbl:       the address of the buf_desc_table to construct
 * @max_entries: hint of maximum number of buffer descriptor entries expected
 * @return:      0 if successful, error code if not
 *
 * max_entries can be minimal (e.g. 1), as the buffer descriptor table grows
 * as necessary (however, it does not shrink). Also, the buffer descriptor
 * table starts at 1 PAGE and grows by powers of two, so it starts out being
 * able to hold PAGE_SIZE/sizeof(buf_desc), or 256 entries (at the time of
 * this writing).
 *
 * p_tbl may be NULL , or p_tbl may point to an initialized buf_desc_table -- it is checked
 */
int buf_desc_table_construct(struct buf_desc_table *p_tbl, unsigned max_entries)
{
   unsigned long array_length;  // in bytes
   int           i;             // array index
   int           retval = -EINVAL;

   PTRACEIN;

   // parameter checks
   ASSERT(p_tbl);
   if ( !p_tbl ) {
      goto DONE;
   }
   ASSERT(!buf_desc_table_is_init(p_tbl));
   if ( buf_desc_table_is_init(p_tbl) ) {
      goto DONE;
   }

   // Compute size of buf_desc array, minimum length is PAGE_SIZE.
   // Final result will always be 2^N * PAGE_SIZE, where 0<=N<=???.
   // First ensure that max_entries is > 0.
   max_entries        = max_entries ? max_entries : 1;
   array_length       = max_entries * sizeof(struct buf_desc);
   p_tbl->order       = get_order(array_length);
   array_length       = PAGE_SIZE << p_tbl->order; // recompute array_length because get_order rounded up
   p_tbl->max_entries = array_length / sizeof(struct buf_desc);

   // Debug
   PVERBOSE("max_entries=%d, array_length=%ld, order=%d\n", p_tbl->max_entries,
               array_length, p_tbl->order);

   // allocate buf_desc array
   p_tbl->pbuf_desc = (struct buf_desc *)__get_free_pages(GFP_KERNEL, p_tbl->order);

   ASSERT(p_tbl->pbuf_desc);
   if ( !p_tbl->pbuf_desc ) {
      retval = -ENOMEM;
      goto DONE;
   }

   // Debug
   PVERBOSE("array address is %p\n", p_tbl->pbuf_desc);

   // initialize buf_desc array along with free list
   p_tbl->index_free = 0;

   // initialize the first (max_entries-1) entries in the table as the free list
   for ( i = 0 ; i < p_tbl->max_entries - 1 ; ++i ) {
      buf_desc_init_next(&p_tbl->pbuf_desc[i], i+1);
   }
   // initialize max_entry-1 (i.e. that last entry) to show no more entries in the free list
   buf_desc_init_next(&p_tbl->pbuf_desc[i], INVALID_ENTRY);

   // if got here, then all went well
   retval = 0;

   // SPECIAL HACK TO TEST REALLOCATION
//   p_tbl->index_free = i;  // now first allocation will go to the INVALID ENTRY, and second will force reallocation

DONE:
   PTRACEOUT_INT(retval);
   return retval;
}  // buf_desc_table_construct

/**
 * buf_desc_table_destruct - buf_desc_table destructor
 * @p_tbl:  the address of the buf_desc_table to clean up
 * @return: void
 *
 * p_tbl may be NULL, or p_tbl may point to an uninitialized buf_desc_table -- it is checked
 *
 * Any allocated buffers will be freed !!!
 */
void buf_desc_table_destruct(struct buf_desc_table *p_tbl)
{
   int   i;             // array index
   PTRACEIN;

   // parameter checks
   ASSERT(p_tbl);
   if( !p_tbl ) {
      return;
   }
   ASSERT(buf_desc_table_is_init(p_tbl));
   if( !buf_desc_table_is_init(p_tbl) ) {
      return;
   }

   // walk through array, freeing any valid pointers
   for( i = 0 ; i < p_tbl->max_entries ; ++i ) {
      #if 1
         struct buf_desc *p = buf_desc_table_get_bufp_from_index(p_tbl, i);
         if( p ) {
            buf_desc_table_free_buf(p_tbl, i);      // Avoids error messages on already-free entries
         }
      #else
         struct buf_desc *p = buf_desc_table_get_bufp_from_index( p_tbl, i);
         if(p && (p->m_type == e_memtype_regular)) {
            unsigned long u, buffer_size;
            PVERBOSE( "About to free entry %d, buffer address 0X%llX\n",
                      i, buf_desc_get_virtaddr(p));
            buffer_size = PAGE_SIZE << p->m_order;
            for (u = 0; u < buffer_size; u += PAGE_SIZE) {
               ClearPageReserved( virt_to_page( buf_desc_get_virtaddr(p) + u));
            }
            free_pages( buf_desc_get_virtaddr(p), buf_desc_get_order(p));
            buf_desc_init_next( p, INVALID_ENTRY);
         }
      #endif /* 0 */
   }

   // Free the array. No need to Clear Reserved because not hardware touchable.
   PVERBOSE( "About to free descriptor array at address %p, of order %u, size %lu 0x%lX\n",
             p_tbl->pbuf_desc, p_tbl->order, PAGE_SIZE << p_tbl->order, PAGE_SIZE << p_tbl->order);
   free_pages((unsigned long)p_tbl->pbuf_desc, p_tbl->order);

   // clean up the data structure itself
   memset(p_tbl, 0, sizeof(struct buf_desc_table));

   PTRACEOUT;
   return;
}  // buf_desc_table_destruct

/**
 * buf_desc_table_realloc - Grow the buf_desc_table
 * @p_tbl:  address of the buf_desc_table to grow
 * @return: index of the next free entry in the new buf_desc_table
 *         return INVALID_ENTRY on failure (meaning out of memory)
 *
 * Internal working function used by buf_desc_table_alloc_buf &
 *    buf_desc_table_add_buf.
 * Grow that buf_desc_table by doubling it.
 * If fail, return the table unchanged.
 *
 * p_tbl must be valid
 */
static inline int buf_desc_table_realloc(struct buf_desc_table *p_tbl)
{
   struct buf_desc  *p_new_array;            // Temporaries, return table unmodified
   int               new_desc_table_order;   //    and still valid if fail
   int               new_max_entries;
   int               i;
   int               retval = INVALID_ENTRY; // assume failure
   PTRACEIN;

   PINFO( "Doubling the size of the descriptor table to %d\n", p_tbl->max_entries*2);

   // Get new buffer
   new_desc_table_order = p_tbl->order + 1;  // size is times 2
   new_max_entries      = p_tbl->max_entries * 2;
   p_new_array          = (struct buf_desc *)__get_free_pages(GFP_KERNEL, new_desc_table_order);

   ASSERT(p_new_array);
   if ( !p_new_array ) {
      PWARN( "Could not allocate memory for new descriptor table size %lu (0x%lX) bytes\n",
            PAGE_SIZE << new_desc_table_order, PAGE_SIZE << new_desc_table_order);
      goto DONE;
   }

   // Copy old data into first half of new table
   memcpy(p_new_array, p_tbl->pbuf_desc, PAGE_SIZE << p_tbl->order);

   // Initialize new space
   for ( i = p_tbl->max_entries ; i < new_max_entries - 1 ; ++i ) {
      buf_desc_init_next(&p_new_array[i], i+1);
   }
   buf_desc_init_next(&p_new_array[i], INVALID_ENTRY);

   // Delete old array
   free_pages((unsigned long)p_tbl->pbuf_desc, p_tbl->order);

   // Update buf_desc_table structure
   p_tbl->pbuf_desc   = p_new_array;
   p_tbl->order       = new_desc_table_order;
   p_tbl->index_free  = p_tbl->max_entries;  // old max_entries is index of beginning of new array
   p_tbl->max_entries = new_max_entries;

   retval             = p_tbl->index_free;   // new table has a slot here

DONE:
   PTRACEOUT_INT(retval);
   return retval;
}  // buf_desc_table_realloc

/**
 * buf_desc_table_alloc_buf - Allocate a memory buffer and add it to the table
 * @p_tbl:  address of the buf_desc_table to receive the buffer descriptor.
 * @size:   size (in bytes) of the desired buffer.
 * @return: index by which to refer to the allocated buffer
 *         return INVALID_ENTRY on failure (meaning out of memory)
 *
 * Actual buffer size will be rounded to nearest power of two >= PAGE_SIZE.
 * This is NOT for kmalloc sorts of operations -- these are for big pinned buffers.
 *
 * p_tbl may be NULL, or p_tbl may point to an uninitialized buf_desc_table -- it is checked
 */
int buf_desc_table_alloc_buf(struct buf_desc_table *p_tbl, unsigned size)
{
   unsigned long     u;          // loop index
   unsigned long     buffer_end; // one past the end of the allocated buffer in kernel virtual address space
   unsigned long     pdata;      // real allocated buffer
   struct buf_desc  *p_buf;      // buf_desc describing allocated buffer
   int               buf_order;  // order of requested size of allocated buffer
   int               retval;

   PTRACEIN;
   retval = INVALID_ENTRY;       // assume error, no memory available

   // parameter checks
   ASSERT(p_tbl);
   if( !p_tbl ) {
      goto DONE;
   }
   if( !buf_desc_table_is_init(p_tbl) ) {
      int temp_retval;

      PDEBUG("buf_desc_table has not been initialized. Initializing with minimum size.\n");
      temp_retval = buf_desc_table_construct(p_tbl, 1);

      ASSERT(0 == temp_retval);
      if ( temp_retval ) {
         goto DONE;
      }
   }

   // if no more room, reallocate the table
   if ( INVALID_ENTRY == p_tbl->index_free ) {
      retval = buf_desc_table_realloc(p_tbl);

      ASSERT(INVALID_ENTRY != retval);
      if ( INVALID_ENTRY == retval ) {
         goto DONE;
      }
   }

   // allocate the data buffer
   PVERBOSE("Allocation request for buffer of size %d\n", size);
   buf_order = get_order(size);
   pdata     = __get_free_pages(GFP_KERNEL, buf_order);
   ASSERT(pdata);
   if ( !pdata ) {
      PWARN( "Could not allocate memory for buffer of size %d, rounded to size %lu\n",
            size, PAGE_SIZE << buf_order);
      goto DONE;
   } else {
      PDEBUG( "Allocation request satisfied for contiguous buffer size of %lu\n"
//              "\tkv address 0x%lX, phys address 0x%lX, stored at index %d\n",
              "\tkv address 0x%lX, phys address %p, stored at index %d\n",
              PAGE_SIZE << buf_order,
              pdata,
              (void*)virt_to_phys((volatile void *)pdata),
              p_tbl->index_free);
      memset( (void*)pdata, buf_init_value, PAGE_SIZE << buf_order);
   }

   // the entry on the free list is the index where the information is to be stored
   retval = p_tbl->index_free;
   p_buf = &p_tbl->pbuf_desc[retval];

   PVERBOSE( "buf_desc for index %d is %p\n", p_tbl->index_free, p_buf);
   PVERBOSE( "next_free_index is %d\n", p_buf->m_next_free_index);
   p_tbl->index_free = p_buf->m_next_free_index;

   // Cache the number of buffer allocations in the descriptor table
   p_tbl->num_allocated++;
   PVERBOSE( "Number of allocated buffers is %d\n", p_tbl->num_allocated);

   // initialize buf_desc data structure with new information
   buf_desc_set_virtaddr_and_type(p_buf, pdata, e_memtype_regular);
   buf_desc_set_physaddr_and_order(p_buf, virt_to_phys((volatile void *)pdata), buf_order);

   // Set Reserved for regular memory
   buffer_end = buf_desc_get_virtaddr(p_buf) + buf_desc_get_size(p_buf);
   for ( u = buf_desc_get_virtaddr(p_buf) ; u < buffer_end ; u += PAGE_SIZE ) {
      SetPageReserved(virt_to_page(u));
   }

DONE:
   PTRACEOUT_INT(retval);
   return retval;
}  // buf_desc_table_alloc_buf

/**
 * buf_desc_table_add_buf - Allocate a memory buffer and add it to the table
 * @p_tbl:  address of the buf_desc_table to receive the buffer descriptor.
 * @p:      buf_desc to add to table.
 * @return: index by which to refer to the allocated buffer
 *         return INVALID_ENTRY on failure (meaning out of memory)
 *
 * p_tbl may be NULL, or p_tbl may point to an uninitialized buf_desc_table -- it is checked
 */
int buf_desc_table_add_buf(struct buf_desc_table *p_tbl, const struct buf_desc *p_in)
{
   struct buf_desc *p_buf;
   int              retval = INVALID_ENTRY;    // assume error, no memory available
   PTRACEIN;

   // parameter checks
   ASSERT(p_tbl);
   if( !p_tbl ) {
      goto DONE;
   }

   if ( !buf_desc_table_is_init(p_tbl) ) {
      int temp_retval;

      PDEBUG("buf_desc_table has not been initialized. Initializing with minimum size.\n");
      temp_retval = buf_desc_table_construct(p_tbl, 1);

      ASSERT(0 == temp_retval);
      if ( temp_retval ) {
         goto DONE;
      }
   }

   // if no more room, reallocate the table
   if ( INVALID_ENTRY == p_tbl->index_free ) {
      retval = buf_desc_table_realloc(p_tbl);

      ASSERT(INVALID_ENTRY != retval);
      if ( INVALID_ENTRY == retval ) {
         goto DONE;
      }
   }

   // the entry on the free list is the index where the information is to be stored
   retval = p_tbl->index_free;
   p_buf  = &p_tbl->pbuf_desc[retval];

   PVERBOSE("buf_desc for index %d is %p\n", p_tbl->index_free, p_buf);
   PVERBOSE("next_free_index is %d\n", p_buf->m_next_free_index);
   p_tbl->index_free = p_buf->m_next_free_index;

   // Cache the number of buffer allocations in the descriptor table
   p_tbl->num_allocated++;
   PVERBOSE("Number of allocated buffers is %d\n", p_tbl->num_allocated);

   // initialize buf_desc data structure with new information
   *p_buf = *p_in;

DONE:
   PTRACEOUT_INT(retval);
   return retval;
}  // buf_desc_table_add_buf

/**
 * buf_desc_table_free_buf - de-allocate a single buffer in the buf_desc_table, return it to the free-list
 * @p_tbl:  pointer to buf_desc_table that describes all the memory
 * @index:  index of buf_desc to free
 * @return: kernel value
 *
 * p_tbl may be NULL, or p_tbl may point to an uninitialized buf_desc_table -- it is checked
 */
int buf_desc_table_free_buf(struct buf_desc_table *p_tbl, int index)
{
   struct buf_desc *p;
   int retval = 0;      // assume happy
   PTRACEIN;

   // parameter checks done is buf_desc_table_get_bufp_from_index()

   p = buf_desc_table_get_bufp_from_index(p_tbl, index);
   // check if free (p will be NULL if the index refers to a previously freed descriptor)
   ASSERT(p);
   if ( p ) {
      PDEBUG( "About to free index %d, virtaddr 0x%llX, physaddr 0x%llX\n",
              index, buf_desc_get_virtaddr(p), buf_desc_get_physaddr(p));

      // Actually clear and free regular memory. IO memory just de-alloc the buf_desc
      if ( e_memtype_regular == p->m_type ) {
         unsigned long u, buffer_end;
         buffer_end = buf_desc_get_virtaddr(p) + buf_desc_get_size(p);
         for ( u = buf_desc_get_virtaddr(p) ; u < buffer_end ; u += PAGE_SIZE ) {
            ClearPageReserved(virt_to_page(u));
         }
         memset((void*)buf_desc_get_virtaddr(p), buf_free_value, buf_desc_get_size(p));
         free_pages(buf_desc_get_virtaddr(p), buf_desc_get_order(p));
      }

      buf_desc_init_next(p, p_tbl->index_free);
      p_tbl->index_free = index;
      p_tbl->num_allocated--;
   } else {
      PWARN( "Buffer already free; possible double-free. Index=%d\n", index);
      retval = -EINVAL;
   }

   PTRACEOUT_INT(retval);
   return retval;
}  // buf_desc_table_free_buf

/**
 * buf_desc_table_get_bufp_from_index - return a buf_desc based on index
 * @p_tbl:  pointer to buf_desc_table that describes all the memory
 * @index:  index of buf_desc to return
 * @return: pointer to the buf_desc, or NULL if error or the entry is free.
 *
 * p_tbl may be NULL, or p_tbl may point to an uninitialized buf_desc_table -- it is checked
 */
struct buf_desc *buf_desc_table_get_bufp_from_index(struct buf_desc_table *p_tbl, int index)
{
   struct buf_desc *p;
//   PTRACEIN;       /* Tracein/out on this function just gets TOOOOO chatty */
   p = NULL;         // Assume failure

   // parameter checks
   ASSERT(p_tbl);
   if( !p_tbl ) {
      goto DONE;
   }
   ASSERT(buf_desc_table_is_init(p_tbl));
   if( !buf_desc_table_is_init(p_tbl) ) {
      goto DONE;
   }

   // check out of bounds
   ASSERT((INVALID_ENTRY != index) && (index < p_tbl->max_entries));
   if ( (INVALID_ENTRY != index) && (index < p_tbl->max_entries) ) {
      // Check if free. p_tbl->pbuf_desc is known not-NULL from buf_desc_is_init() above
      p = &p_tbl->pbuf_desc[index];
      if ( p->m_free ) {
         p = NULL;      // Signal failure to calling program
      }
   } else {
      PWARN( "Index out of range. Index=%d, Last Entry=%d\n", index, p_tbl->max_entries-1);
   }

DONE:
//   PTRACEOUT_PTR(p);
   return p;
}  // buf_desc_table_get_bufp_from_index


/**
 * buf_desc_table_is_init - return true if the buf_desc_table has been initialized
 * @p_tbl:  pointer to buf_desc_table
 * @return: 1 if initialized, 0 if not
 *
 * p_tbl may be NULL, or p_tbl may point to an uninitialized buf_desc_table -- it is checked
 */
int buf_desc_table_is_init(struct buf_desc_table *p_tbl)
{
   if ( p_tbl ) {
      return p_tbl->pbuf_desc ? 1 : 0;
   } else {
      return 0;
   }
}  // buf_desc_table_is_init


/**
 * buf_desc_table_contains_buffers - return true if the buf_desc_table contains allocated buffers
 * @p_tbl:  pointer to buf_desc_table
 * @return: 1 if contains allocated buffers, 0 if not
 *
 * p_tbl may be NULL, or p_tbl may point to an uninitialized buf_desc_table -- it is checked
 */
int buf_desc_table_contains_buffers(struct buf_desc_table *p_tbl)
{
   if ( buf_desc_table_is_init(p_tbl) ) {
      return p_tbl->num_allocated ? 1 : 0;
   } else {
      return 0;
   }
}  // buf_desc_table_contains_buffers


/**
 * buf_desc_table_dump - print out the buf_desc_table
 * @p_tbl: pointer to buf_desc_table that describes all the memory
 * @start: index to start, typically 0
 * @stop:  index to stop. If want end of array use 0. If past end of array it will truncate.
 * @return: void
 */
void buf_desc_table_dump(struct buf_desc_table *p_tbl, int start, int stop)
{
   static const int GRANULARITY = 4;
   int i;
   PTRACEIN;

   // Parameter check
   ASSERT(p_tbl);
   if( !p_tbl ) {
      return;
   }

   // Check Start value
   ASSERT((start >= 0) && ((unsigned)start <= p_tbl->max_entries));
   if ( start < 0 || ((unsigned)start > p_tbl->max_entries) ) {
      start = 0;
   }
   start = (start / GRANULARITY) * GRANULARITY; // force to lower GRANULARITY boundary

   // Check Stop value
   ASSERT((stop >= start) && (stop <= p_tbl->max_entries));
   if ( 0 == stop ) {                             // special case default
      stop = p_tbl->max_entries;
   } else if (stop >= start && stop <= p_tbl->max_entries) {
      // Stop is in range, now force to upper GRANULARITY boundary.
      int add = (stop % GRANULARITY) ? 1 : 0 ;
      stop = ( (stop / GRANULARITY) + add ) * GRANULARITY;
   } else {
      stop = p_tbl->max_entries;
   }

   // Debug
   PVERBOSE("Start %d, Stop %d, Max_entries %d\n", start, stop, p_tbl->max_entries);

   // Printout
   PINFO( "Dump of buf_desc_table:\n"
            "\tbuf_desc_table %p\n"
            "\tbuf_desc array %p\n"
            "\torder          %d\n"
            "\tnum_allocated  %d 0x%X\n"
            "\tmax_entries    %d 0x%X\n"
            "\tindex_free     %d 0x%X\n"
            "\tstart index    %d 0x%X\n"
            "\tstop  index    %d 0x%X\n"
            "\tNOTE: v@0x1 means entry free, and then p@XXX means next entry in free list is index XXX\n",
            p_tbl,
            p_tbl->pbuf_desc,
            p_tbl->order,
            p_tbl->num_allocated,
            p_tbl->num_allocated,
            p_tbl->max_entries,
            p_tbl->max_entries,
            p_tbl->index_free,
            p_tbl->index_free,
            start, start,
            stop, stop
   );
   for ( i = start ; i < stop ; i += GRANULARITY ) {
      PINFO( "%d: v@0x%llX p@0x%llX,  v@0x%llX p@0x%llX,  v@0x%llX p@0x%llX,  v@0x%llX p@0x%llX\n",
             i,
             p_tbl->pbuf_desc[i+0].m_virtaddr, p_tbl->pbuf_desc[i+0].m_physaddr,
             p_tbl->pbuf_desc[i+1].m_virtaddr, p_tbl->pbuf_desc[i+1].m_physaddr,
             p_tbl->pbuf_desc[i+2].m_virtaddr, p_tbl->pbuf_desc[i+2].m_physaddr,
             p_tbl->pbuf_desc[i+3].m_virtaddr, p_tbl->pbuf_desc[i+3].m_physaddr
      );
   }

   PTRACEOUT;
}  // buf_desc_table_dump


#endif // __AALKERNEL_SPL2_PIP_DKSM_BUF_DESC_TABLE_IMPL_H_

