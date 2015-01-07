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
//        FILE: mem-internal-ioctl.c
//     CREATED: 10/11/2011
//      AUTHOR: Henry Mitchel
//
// PURPOSE:  The ioctl function for the
//           Intel(R) QuickAssist Technology Accelerator Abstraction Layer (AAL)
//           SPL 2 Memory Manager Driver.
//
// HISTORY:  Copied from mem-ioctl.c
//
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 10/11/2011     HM       Initial version started
//****************************************************************************
#ifndef __AALKERNEL_SPL2_PIP_DKSM_MEM_INTERNAL_IOCTL_H_
#define __AALKERNEL_SPL2_PIP_DKSM_MEM_INTERNAL_IOCTL_H_
#include "mem-sess.h"         // Memory construct definitions
#include "xsid.h"             // xsid information
#include "mem-virt-impl.h"    // virtual memory functions


/**
 * internal_ioctl_put_config - allocate a single configuration buf_desc and a Workspace ID for it
 * @pContext: IN:    pointer to memmgr_session context
 *            OUT:   pContext->m_xsid_config_space is set to the special xsid for the config space
 * @physaddr: IN:    physical byte address to be placed into buf_desc
 * @length:   IN:    length in bytes to be placed into buf_desc
 * @kvirt:    IN:    kernel virtual to be added to buf_desc
 * @return:   kernel standard
 *
 * Add the information about a configuration record into the buf_desc_table. Used internally
 *    to track this information.
 *
 * WARNING:   Sets the struct memmgr_session singleton value m_xsid_config_space
 *
 * WARNING:   Not to be called from user space. Do NOT provide a path to this function
 *            from user space.
 */
static inline int internal_ioctl_put_config (struct memmgr_session *pContext,
                                             btUnsigned64bitInt     physaddr,
                                             unsigned               length,
                                             void __iomem          *kvirt)
{
   struct buf_desc buf;
   int             retval = -ENOMEM;
   PTRACEIN;

   // initialize buf_desc to put into table
   buf_desc_set_virtaddr_and_type(&buf, (unsigned long)kvirt, e_memtype_IO);
   buf_desc_set_physaddr_and_order(&buf, physaddr, get_order(length));

   // put the buf_desc into the table, retval is index
   retval = buf_desc_table_add_buf(&pContext->m_buf_table, &buf);

   ASSERT(retval != INVALID_ENTRY);
   if ( INVALID_ENTRY == retval ) {
      // table is unchanged
      retval = -ENOMEM;
      goto DONE;
   }

   // retval is currently index into buf_desc table
   pContext->m_xsid_config_space = xsid_ctor(retval, e_memtype_IO);
   retval = 0;

DONE:
   PTRACEOUT_INT(retval);
   return retval;
}  // internal_ioctl_put_config

/**
 * internal_ioctl_get_config - retrieve information about the configuration space
 * @pContext: IN/OUT: pointer to memmgr_session context
 * @arg:      IN/OUT: pointer to struct spl2mem_mem_op command structure
 * @return:   kernel standard
 *
 * Find and return the xsid & associated info for the configuration area if it exists.
 *
 * NOTE: This is specifically for calling by user space to get an xsid in order to mmap the config space
 */
static int internal_ioctl_get_config ( struct memmgr_session* pContext, struct spl2mem_mem_op *pmem_op)
{
   struct buf_desc     *pbuf;
   int                  index;
   int                  retval = -EINVAL;
   PTRACEIN;

   if( pContext->m_xsid_config_space) {
      index = index_from_xsid( pContext->m_xsid_config_space);
      pbuf = buf_desc_table_get_bufp_from_index( &pContext->m_buf_table, index);
      if (pbuf) {
         pmem_op->length   = PAGE_SIZE << pbuf->m_order;
         pmem_op->physaddr = pbuf->m_physaddr;
         pmem_op->xsid     = pContext->m_xsid_config_space;
         retval = 0;
      } else {
         PWARN( "Config space bad workspace id\n");
      }
   } else {
      PWARN( "Config space retrieved but not initialized\n");
   }

//done:;
   PTRACEOUT_INT( retval);
   return retval;
}  // internal_ioctl_get_config


/**
 * internal_ioctl_alloc - allocate a single regular buffer and a Workspace ID for it
 * @pContext: IN/OUT: pointer to memmgr_session context
 * @arg:      IN/OUT: pointer to struct spl2mem_mem_op command structure
 * @return:   kernel standard
 *
 * The buffer is physically contiguous pinned kernel memory allocated via __get_free_pages.
 * Its address (kernel virtual and physical) and its type are stored, and the entire structure
 *    is referred to via an index internally, or externally via a xsid (Workspace ID).
 */
static int internal_ioctl_alloc ( struct memmgr_session* pContext, struct spl2mem_mem_op *pmem_op)
{
   struct buf_desc *pbuf_desc;
   int              retval;
   int              index;
   PTRACEIN;

   // Make sure a descriptor table exists
   if ( !buf_desc_table_is_init(&pContext->m_buf_table) ) {
      PVERBOSE("Initializing buffer descriptor table structure on initial allocation\n");
      retval = buf_desc_table_construct(&pContext->m_buf_table, 1); // -ENOMEM on failure
      if ( retval ) {
         PDEBUG("Buffer descriptor table structure could not be created\n");
         goto done;
      }
   }

   // Allocate the buffer
   index = buf_desc_table_alloc_buf(&pContext->m_buf_table, pmem_op->length);
   if ( INVALID_ENTRY == index ) {
      retval = -ENOMEM;       // PWARN message printed already inside buf_desc_table_alloc_buf
      goto done;
   }

   // Load the structure with the data to be returned
   pbuf_desc = buf_desc_table_get_bufp_from_index(&pContext->m_buf_table, index);
   if ( NULL == pbuf_desc ) {
      buf_desc_table_free_buf(&pContext->m_buf_table, index);
      retval = -ENOMEM;
      goto done;
   }
   pmem_op->length   = buf_desc_get_size(pbuf_desc);
   pmem_op->physaddr = buf_desc_get_physaddr(pbuf_desc);
   pmem_op->xsid     = xsid_ctor(index, e_memtype_regular);

done:
   PTRACEOUT_INT(retval);
   return retval;
}  // internal_ioctl_alloc



/**
 * internal_ioctl_free - given a workspace ID, free it
 * @pContext: IN/OUT: pointer to memmgr_session context
 * @arg:      IN/OUT: pointer to struct spl2mem_mem_op command structure
 * @return:   kernel standard
 */
static int internal_ioctl_free  ( struct memmgr_session* pContext, struct spl2mem_mem_op *pmem_op)
{
   int retval = 0;
   PTRACEIN;

   switch (memtype_of_xsid(pmem_op->xsid)) {
      case e_memtype_regular:    // Intentional fall through
      case e_memtype_IO:
         retval = buf_desc_table_free_buf( &pContext->m_buf_table, index_from_xsid( pmem_op->xsid));
         break;
      case e_memtype_virtualized:
         if( index_from_xsid( pmem_op->xsid) != 0) {
            PWARN( "Virtual free failed due to bad xsid index. Should be 0, but is %d\n",
                   index_from_xsid( pmem_op->xsid));
            retval = -EINVAL;
         }
         virtmem_destruct( &pContext->m_virtmem);
         break;
      default:
         PERR( "Free failed due to bad xsid (Workspace ID). Value=" XSID_FMT ", EFAULT\n", pmem_op->xsid);
         retval = -EINVAL;
         break;
   }

   PTRACEOUT_INT( retval);
   return retval;
}  // internal_ioctl_free



/**
 * internal_ioctl_valloc - allocate a virtual workspace return a Workspace ID for it
 * @pContext: IN/OUT: pointer to memmgr_session context
 * @arg:      IN/OUT: pointer to struct spl2mem_mem_op command structure
 * @return:   kernel standard
 */
static int internal_ioctl_valloc( struct memmgr_session* pContext, struct spl2mem_mem_op *pmem_op)
{
   int                   retval;
   PTRACEIN;

   // Check if virtual workspace has been initialized
   if( virtmem_is_init( &pContext->m_virtmem)) {
      PWARN( "Cannot allocate multiple virtual workspaces. Aborting EPERM/n");
      retval = -EPERM;
   } else {
      // Normal flow -- create the needed virtual workspace. Always use 2 MB buffers.
      retval = virtmem_construct( &pContext->m_virtmem, pmem_op->length, 21);
      if (retval) goto done;
   }

   // Returned values after successful allocation
   pmem_op->length   = pContext->m_virtmem.m_vsize;
   pmem_op->physaddr = pContext->m_virtmem.m_pte_array_physaddr;

   // Only support one virtual workspace, so only one virtual xsid
   pmem_op->xsid = xsid_ctor( 0, e_memtype_virtualized);

done:;
   PTRACEOUT_INT( retval);
   return retval;
}  // internal_ioctl_valloc


/**
 * internal_ioctl_debug - provide debugging information about the memory subsystem
 * @pContext: IN/OUT: pointer to memmgr_session context
 * @arg:      IN/OUT: pointer to struct spl2mem_mem_op command structure
 * @return:   kernel standard
 */
static int internal_ioctl_debug( struct memmgr_session* pContext, struct spl2mem_debug *pdbg)
{
   int                  retval;
   btUnsigned64bitInt             u;
   PTRACEIN;

   // default to bad parameter error return
   retval = -EINVAL;

   switch (pdbg->cmd) {
      case e_dbg_cmd_dump_desc_buf_table: {
         PDEBUG( "dbg_cmd_dump_desc_buf_table, start %d, stop %d\n",
                 pdbg->start, pdbg->stop);
         switch (pdbg->mem_type) {
            case e_dbg_memtable_regular:
               buf_desc_table_dump( &pContext->m_buf_table, pdbg->start, pdbg->stop);
               retval = 0;
               break;
            case e_dbg_memtable_virtual: {
               virtmem_dump( &pContext->m_virtmem, pdbg->start, pdbg->stop);
               retval = 0;
               break;
            }
            default:
               PWARN( "Debug print failed due to bad mem_type enum, value %d, EFAULT\n", pdbg->mem_type);
               break;
         }  // (memtype_of_xsid(free.xsid))
         break;
      }
      case e_dbg_cmd_print_xsid_contents: {
         size_t   lenstr;                       // length of passed in string
         PDEBUG( "dbg_cmd_print_xsid_contents, xsid " XSID_FMT "\n", pdbg->xsid);

         switch (memtype_of_xsid(pdbg->xsid)) {
            case e_memtype_regular: {
               struct buf_desc *pbuf;
               int index = index_from_xsid( pdbg->xsid);
               pbuf = buf_desc_table_get_bufp_from_index( &pContext->m_buf_table, index);

               if (pbuf) {
                  btUnsigned64bitInt kvaddr_buf;    // kernel virtual address of buffer
                  kvaddr_buf = buf_desc_get_virtaddr( pbuf) + pdbg->offset;

                  // make sure there is a null in there before dumping the string
                  for (u=0; u<pdbg->max_len; ++u) {
                     if ( 0 == *(char*)(kvaddr_buf+u)) break;
                  }
                  if( u == pdbg->max_len) {
                     PERR( "No string end found within max length, cannot print\n");
                     break;
                  }

                  // dump the string
                  PERR( "String from workspace xsid " XSID_FMT " at offset 0x%llX is '%s'\n",
                        pdbg->xsid, pdbg->offset, (char*)kvaddr_buf);

                  // write something back :)
                  lenstr = strlen( (char*)kvaddr_buf);
                  strcpy( (char*)kvaddr_buf+lenstr, WRITTEN_FROM_THE_KERNEL "\0");
                  retval = 0;
               } else {
                  PERR( "Bad xsid passed in. Value is " XSID_FMT "\n", pdbg->xsid);
                  break;
               }
               break;
            }
            case e_memtype_virtualized: {
               struct buf_desc *pbuf;
               int index = index_from_xsid( pdbg->xsid);
               if (index) {
                  PERR( "Bad xsid passed in, index part should be 0. Value is " XSID_FMT "\n", pdbg->xsid);
                  break;
               }

               index = pdbg->offset / pContext->m_virtmem.m_len_super_page;      // re-use of index for different meaning
               pbuf = buf_desc_table_get_bufp_from_index( &pContext->m_virtmem.m_pt_buf_table, index);

               if (pbuf) {
                  btUnsigned64bitInt kvaddr_buf;    // kernel virtual address of buffer
                  kvaddr_buf = buf_desc_get_virtaddr( pbuf) + pdbg->offset % pContext->m_virtmem.m_len_super_page ;

                  // make sure there is a null in there before dumping the string
                  for (u=0; u<pdbg->max_len; ++u) {
                     if ( 0 == *(char*)(kvaddr_buf+u)) break;
                  }
                  if( u == pdbg->max_len) {
                     PERR( "No string end found within max length, cannot print\n");
                     break;
                  }

                  // dump the string
                  PERR( "String from workspace xsid " XSID_FMT " at offset 0x%llX is '%s'\n",
                        pdbg->xsid, pdbg->offset, (char*)kvaddr_buf);

                  // write something back :)
                  lenstr = strlen( (char*)kvaddr_buf);
                  strcpy( (char*)kvaddr_buf+lenstr, WRITTEN_FROM_THE_KERNEL "\0");
                  retval = 0;
               } else {
                  PERR( "Bad xsid passed in. Value is " XSID_FMT "\n", pdbg->xsid);
                  break;
               }
               break;
            }
            default:
               PWARN( "Debug print failed due to bad xsid (Workspace ID) memory type, EFAULT\n");
               break;
         }  // (memtype_of_xsid(free.xsid))
         break;
      }
      default:
         PERR( "Unknown debug command seen\n");
         retval = -1;
         break;
   }  // switch (pdbg->cmd)

//done:;
   PTRACEOUT_INT( retval);
   return retval;
}  // internal_ioctl_debug

#endif // __AALKERNEL_SPL2_PIP_DKSM_MEM_INTERNAL_IOCTL_H_

