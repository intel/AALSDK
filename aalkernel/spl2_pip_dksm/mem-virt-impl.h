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
//        FILE: mem-virt-impl.h
//     CREATED: 11/23/2011
//      AUTHOR: Henry Mitchel
//
// PURPOSE:  This file contains private memory-structure-specific definitions
//           for the Intel(R) QuickAssist Technology Accelerator Abstraction
//           Layer (AAL) SPL 2 Memory Manager Driver.
//
//           Specifically -- virtual memory page-table implementation
//           but in a header file
//
//           Operates on memmgr_virtmem structures
//
//
// HISTORY:  From mem-virt.c
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 11/23/2011     HM       Initial version started
//****************************************************************************
#ifndef __AALKERNEL_SPL2_PIP_DKSM_MEM_VIRT_IMPL_H__
#define __AALKERNEL_SPL2_PIP_DKSM_MEM_VIRT_IMPL_H__
#include "mem-virt.h"         // memmgr_virtmem, buf_desc_table, buf_desc, pte


/**
 * virtmem_alloc - allocate a struct memmgr_virtmem
 * @return pointer to the structure or NULL if out of memory
 */
struct memmgr_virtmem *virtmem_alloc (void)
{
   struct memmgr_virtmem *p_virt;         // kmalloc'd and returned,
                                          //    freed in virtmem_free
   PTRACEIN;

   // allocate memmgr_virtmem
   p_virt = kmalloc( sizeof(*p_virt), GFP_KERNEL);
   if (!p_virt) {
      PWARN( "Failed to kmalloc buf_desc_table\n");
   } else {
      memset( p_virt, 0, sizeof(*p_virt));
   }

   PTRACEOUT_PTR(p_virt);
   return p_virt;
}  // buf_desc_table_alloc

/**
 * virtmem_free - struct memmgr_virtmem destructor
 * @p_virt:  the address of the memmgr_virtmem to free
 * @return: NULL to set the pointer to if needed
 */
struct memmgr_virtmem *virtmem_free (struct memmgr_virtmem *p_virt)
{
   PTRACEIN;

   // free the memmgr_virtmem
   PVERBOSE( "About to free memmgr_virtmem at address %p\n", p_virt);
   memset( p_virt, 0, sizeof(*p_virt));
   kfree( p_virt);

   PTRACEOUT;
   return NULL;
}  // buf_desc_table_free

/**
 * virtmem_construct - struct memmgr_virtmem constructor
 * @p_virt:  the address of the memmgr_virtmem to construct
 * @vsize:   requested size in bytes of virtual workspace. Will be the size of all super-pages concatenated.
 * @order:   true hw order of super-pages (buffers), e.g. order 21 is 2MB
 * @return:  0 if successful, error code (-ENOMEM) if not
 *
 * The buffers do not reallocate, at least not yet.
 */
int virtmem_construct (struct memmgr_virtmem *p_virt, unsigned long vsize, unsigned order)
{
   unsigned long u;                 // loop counter
   unsigned long buffer_end;        // end address of the page table in bytes
   int           i;                 // array index
   int           remainder;         // scratch variable
   int           retval = -EINVAL;  // assume bad parameters
   PTRACEIN;

   // basic parameter check
   if( !p_virt) {
      PDEBUG( "NULL struct memmgr_virtmem passed in. Aborting.\n");
      goto done;
   }

   // check and initialize requested size of workspace
   if( !vsize) {
      PWARN( "Requested workspace size is 0, but cannot be. Aborting EINVAL.\n");
      goto done;
   }

   // Check and initialize size of pte entries. For now must be 2 MB, or order 21.
   // Checking at this level is probably inappropriate. Just a warning issued and we continue
   if( order != 21) {
      PWARN( "Virtual page table entry length should be 2^21 (2 MB). Value passed in was %d. Going head with passed in value.\n", order);
   }
   p_virt->m_pte_order_hw = order;

   // Compute number of super-page table entries. Need to compute based on multiples of order.
   //    So if vsize is not exact multiple of order size, then round up.
   //    E.g. if vsize is 1 MB, need it to be 2 MB.
   p_virt->m_len_super_page = 1 << p_virt->m_pte_order_hw;
   remainder = vsize % p_virt->m_len_super_page ? 1 : 0; // temp is 1 if vsize not exact multiple, 0 if it is
   p_virt->m_num_of_valid_pte = vsize/p_virt->m_len_super_page + remainder;

   // initialize actual size of workspace
   p_virt->m_vsize = p_virt->m_len_super_page * p_virt->m_num_of_valid_pte;

   // Double-check number of entries. Must be within hardware limitations.
   // Specifically, in range of 1 to 1024.
   if( p_virt->m_num_of_valid_pte < 1 || p_virt->m_num_of_valid_pte > 1024) {
      PWARN( "Virtual super-page-table length must be between 1 and 1024 entries. "
               "\n\tRequested workspace size 0x%lX, rounded workspace size is 0x%lX, "
               "requested super-page order is 0x%d. Aborting EINVAL.\n",
               vsize, p_virt->m_vsize,
               order);
      goto cleanup;
   }

   // compute and initialize size of the pte array, as needed by GFP
   p_virt->m_pte_array_order = get_order( p_virt->m_num_of_valid_pte * sizeof(struct memmgr_pte));

   // remember the maximum number of entries possible in the page table
   p_virt->m_max_num_of_pte = (PAGE_SIZE << p_virt->m_pte_array_order) / sizeof(struct memmgr_pte);

   // Debug
   PDEBUG( "Requested workspace size in bytes is 0x%lX, rounded workspace size is 0x%lX"
           "\n\tnum entries in super-page table is %d,"
           "\n\tpte_array_length in bytes is 0x%lX, pte_array_length order for __get_free_pages is %d,"
           "\n\tsuper-page size in bytes 0x%X, super-page true hw order is %d"
           "\n\tmaximum number of pte's that could be in the page table %d"
           "\n",
           vsize, p_virt->m_vsize,
           p_virt->m_num_of_valid_pte,
           PAGE_SIZE<<p_virt->m_pte_array_order, p_virt->m_pte_array_order,
           p_virt->m_len_super_page, p_virt->m_pte_order_hw,
           p_virt->m_max_num_of_pte
           );

   // Create the buffer descriptor table to hold all the entries
   retval = buf_desc_table_construct( &p_virt->m_pt_buf_table, p_virt->m_num_of_valid_pte);
   if (retval) {
      PWARN( "Could not allocate buffer descriptor table\n");
      goto cleanup;
   }

   // Allocate the super page table itself
   p_virt->m_pte_array = (struct memmgr_pte *)__get_free_pages( GFP_KERNEL, p_virt->m_pte_array_order);
   if (!p_virt->m_pte_array) {
      PWARN( "Could not allocate super-page table\n");
      retval = -ENOMEM;
      goto clear_buf_desc;
   }

   // Set the super page table to Reserved since it can be hit by HW
   buffer_end = (unsigned long)p_virt->m_pte_array + (PAGE_SIZE << p_virt->m_pte_array_order);
   for( u = (unsigned long)p_virt->m_pte_array; u < buffer_end; u += PAGE_SIZE) {
      SetPageReserved( virt_to_page( u));
   }

   // Clear the super page table
   PVERBOSE( "Clearing the super-page table at kv address %p\n", p_virt->m_pte_array);
   memset( p_virt->m_pte_array, 0, PAGE_SIZE << p_virt->m_pte_array_order);

   // Remember the page table's physical address
   p_virt->m_pte_array_physaddr = virt_to_phys( p_virt->m_pte_array);

   // Debug
   PDEBUG( "Kernel virtual address of page table is %p, physical address is 0x%" PRIXPHYS_ADDR "\n",
           p_virt->m_pte_array,
           p_virt->m_pte_array_physaddr
           );

   ///////////////////////////////////////////////////////////////////////////////////////
   // ASSERTION: At this point all elements of struct memmgr_virtmem have been initialized
   ///////////////////////////////////////////////////////////////////////////////////////

   // Allocate every super-page, and record it in the page table. Note that it will
   //    already be recorded in the buf_desc_table once the allocator returns.
   // Be ready to back out if allocation fails.
   for ( i = 0 ; i < p_virt->m_num_of_valid_pte ; ++i ) {
      struct buf_desc *p_buf;

      // Allocate super-page, record in buf_desc_table, and retrieve reference to it.
      int index = buf_desc_table_alloc_buf(&p_virt->m_pt_buf_table, p_virt->m_len_super_page);
      if ( INVALID_ENTRY == index ) {
         break;
      }

      p_buf = buf_desc_table_get_bufp_from_index(&p_virt->m_pt_buf_table, index);

      if ( NULL == p_buf ) {
         break;
      }

      // Get the allocation information and copy it into the page table
      memmgr_set_pte(&p_virt->m_pte_array[i], buf_desc_get_physaddr( p_buf));
   }
   // Check for early abort and back out if needed
   if( i != p_virt->m_num_of_valid_pte) {
      int num_allocated = i;
      PWARN("Could not allocate entry super-page table entry %d.\n", i);
      for( i=0; i<num_allocated; ++i) {
         buf_desc_table_free_buf( &p_virt->m_pt_buf_table, i);
      }
      goto delete_page_table;
   }

   // Normal exit
   retval = 0;
   goto done;

   // Abort exits
delete_page_table:
   for( u = (unsigned long)p_virt->m_pte_array; u < buffer_end; u += PAGE_SIZE) {
      ClearPageReserved( virt_to_page( u));
   }
   free_pages( (unsigned long)p_virt->m_pte_array, p_virt->m_pte_array_order);
clear_buf_desc:
   buf_desc_table_destruct( &p_virt->m_pt_buf_table);
cleanup:;
   memset( p_virt, 0, sizeof(*p_virt));
done:
   PTRACEOUT_INT(retval);
   return retval;
}  // buf_desc_table_construct

/**
 * virtmem_destruct - buf_desc_table destructor
 * @p_virt: the address of the buf_desc_table to clean up
 */
void virtmem_destruct (struct memmgr_virtmem *p_virt)
{
   unsigned long u;           // loop index
   unsigned long buffer_end;  // end address of the page table in bytes
   int           i;           // array index
   PTRACEIN;

   // Clear and free all super-page buffers
   PDEBUG( "About to free all %d super-page buffers\n", p_virt->m_num_of_valid_pte);
   for ( i = 0 ; i < p_virt->m_num_of_valid_pte ; ++i ) {
      // Get access to the description of the i'th super-page buffer
      struct buf_desc *p_buf = buf_desc_table_get_bufp_from_index(&p_virt->m_pt_buf_table, i);

      if ( NULL == p_buf ) {
         continue;
      }

      // Clear it
      memset((void*)buf_desc_get_virtaddr(p_buf), buf_free_value, buf_desc_get_size(p_buf));
      // Actual free
      buf_desc_table_free_buf(&p_virt->m_pt_buf_table, i);
   }

   // Delete page table
   PDEBUG( "About to free page table @%p\n", p_virt->m_pte_array);

   buffer_end = (unsigned long)p_virt->m_pte_array + (PAGE_SIZE << p_virt->m_pte_array_order);
   for( u = (unsigned long)p_virt->m_pte_array; u < buffer_end; u += PAGE_SIZE) {
      ClearPageReserved( virt_to_page( u));
   }
   free_pages( (unsigned long)p_virt->m_pte_array, p_virt->m_pte_array_order);

   // Delete the buf_desc_table
   buf_desc_table_destruct( &p_virt->m_pt_buf_table);

   // clean up the data structure itself
   memset( p_virt, 0, sizeof(*p_virt));

   PTRACEOUT;
   return;
}  // virtmem_destruct


/**
 * virtmem_is_init - return true if the struct virtmem has been initialized
 * @p_virt: pointer to struct memmgr_virtmem to check
 * @return: 1 if initialized, 0 if not
 */
int virtmem_is_init (struct memmgr_virtmem *p_virt)
{
   return buf_desc_table_is_init( &p_virt->m_pt_buf_table);
}  // virtmem_is_init


/**
 * virtmem_contains_buffers - return true if the struct virtmem has been initialized
 * @p_virt: pointer to struct memmgr_virtmem to check
 * @return: 1 if initialized, 0 if not
 */
int virtmem_contains_buffers (struct memmgr_virtmem *p_virt)
{
   return buf_desc_table_contains_buffers( &p_virt->m_pt_buf_table);
}  // virtmem_contains_buffers


/**
 * virtmem_dump - print out the buf_desc_table and associate page table entries
 * @p_virt: the address of the memmgr_virtmem to construct
 * @start:  index to start, typically 0
 * @stop:   index to stop. If want end of array use 0. If past end of array it will truncate.
 * @return: void
 */
void virtmem_dump (struct memmgr_virtmem *p_virt, int start, int stop)
{
   static const int GRANULARITY = 8;
   int i;
   PTRACEIN;

   // Parameter check
   if( !p_virt) {
      PDEBUG( "NULL input memmgr_virtmem pointer. Aborting\n");
      return;
   }

   // Check Start value
   if( start < 0 || (unsigned)start > p_virt->m_max_num_of_pte) {
      PNOTICE( "Start index passed in out of bounds, set to 0\n");
      start = 0;
   }
   start = (start / GRANULARITY) * GRANULARITY; // force to lower GRANULARITY boundary

   // Check Stop value
   if (stop == 0) {                             // special case default
      stop = p_virt->m_max_num_of_pte;
   } else if (stop >= start && stop <= p_virt->m_max_num_of_pte) {
      // Stop is in range, now force to upper GRANULARITY boundary.
      int add = (stop % GRANULARITY) ? 1 : 0 ;
      stop = ( (stop / GRANULARITY) + add ) * GRANULARITY;
   } else {
      PNOTICE( "Stop index passed in out of bounds, set to end of array\n");
      stop = p_virt->m_max_num_of_pte;
   }

   // Debug
   PVERBOSE( "Start %d, Stop %d, Max_entries %d\n", start, stop,
             p_virt->m_max_num_of_pte);

   // Printout memmgr_virtmem info
   PINFO( "Dump of virtual memory information:\n"
            "\t memmgr_virtmem address %p\n"
            "\t m_pte_array            %p\n"
            "\t @m_pt_buf_table        %p\n"
            "\t m_vsize                %ld\t0x%lX\n"
            "\t m_pte_array_physaddr   %" PRIXPHYS_ADDR "\n"
            "\t m_pte_array_order      %d\n"
            "\t m_pte_order_hw         %d\n"
            "\t m_num_of_valid_pte     %d\n"
            "\t m_max_num_of_pte       %d\n"
            "\t start index            %d\t0x%X\n"
            "\t stop  index            %d\t0x%X\n"
            "\tNOTE: rightmost bit=1 means the entry is valid\n",
            p_virt,
            p_virt->m_pte_array,
            &p_virt->m_pt_buf_table,
            p_virt->m_vsize, p_virt->m_vsize,
            p_virt->m_pte_array_physaddr,
            p_virt->m_pte_array_order,
            p_virt->m_pte_order_hw,
            p_virt->m_num_of_valid_pte,
            p_virt->m_max_num_of_pte,
            start, start,
            stop, stop
   );

   // print out the page table
   if(p_virt->m_pte_array) {
      PINFO( "Page Table Itself:\n");
      for (i=start; i<stop; i+=GRANULARITY) {
         PINFO( "page_table_cl[%d]: 0x%llX 0x%llX 0x%llX 0x%llX  0x%llX 0x%llX 0x%llX 0x%llX\n",
                i,
                p_virt->m_pte_array[i+0].pte,
                p_virt->m_pte_array[i+1].pte,
                p_virt->m_pte_array[i+2].pte,
                p_virt->m_pte_array[i+3].pte,
                p_virt->m_pte_array[i+4].pte,
                p_virt->m_pte_array[i+5].pte,
                p_virt->m_pte_array[i+6].pte,
                p_virt->m_pte_array[i+7].pte
         );
      }  // for
      PINFO( "End of Page Table\n");
   }

   // Print out buf_desc_table info
   buf_desc_table_dump( &p_virt->m_pt_buf_table, start, stop);

   PTRACEOUT;
   return;
}  // buf_desc_table_dump

#endif // __AALKERNEL_SPL2_PIP_DKSM_MEM_VIRT_IMPL_H__

