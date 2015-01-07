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
//        FILE: mem-internal-fops.h
//     CREATED: 11/26/2011
//      AUTHOR: Henry Mitchel
//
// PURPOSE:  File operations structures for the
//           Intel(R) QuickAssist Technology Accelerator Abstraction Layer (AAL)
//           SPL 2 Memory Manager Driver.
//
// HISTORY:  Copied from mem-fops.c to create a header-based implementation
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 11/26/2011     HM       Initial version started
//****************************************************************************
#ifndef __AALKERNEL_SPL2_PIP_DKSM_MEM_INTERNAL_FOPS_H__
#define __AALKERNEL_SPL2_PIP_DKSM_MEM_INTERNAL_FOPS_H__
#include "spl2mem-kern.h"
#include "mem-internal-ioctl.h"  // Declarations for internal ioctl functions


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/////////                  Internal fops functions                    /////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**
 * memmgr_internal_alloc - allocate a memmgr_session structure for someone
 * @return: allocated structure or NULL if error (assume -ENOMEM)
 */
static inline struct memmgr_session *memmgr_internal_alloc(void)
{
   struct memmgr_session *pContext;
   PTRACEIN;

   // Get memory for file-specific context
   pContext = kmalloc(sizeof(struct memmgr_session), GFP_KERNEL);

   PDEBUG("Internal mem mgr pContext=%p\n", pContext);

   if ( pContext ) {
      memset(pContext, 0, sizeof(struct memmgr_session));
   }

   PTRACEOUT_PTR(pContext);
   return pContext;
}  // memmgr_internal_alloc

/**
 * memmgr_internal_free - free the structure allocated via memmgr_internal_alloc
 * @pContext: struct memmgr_session* having previously been allocated.
 * @return:   NULL
 *
 * Note: pContext passed in NULL is handled, but unexpected.
 */
static inline struct memmgr_session * memmgr_internal_free(struct memmgr_session *pContext)
{
   PTRACEIN;

   ASSERT(pContext);
   if( pContext ) {
      memset(pContext, 0, sizeof(*pContext));
      kfree(pContext);
      pContext = NULL;
   }

   PTRACEOUT_PTR(pContext);
   return pContext;
}


/**
 * memmgr_internal_open - file open called, initialize the session structure
 * @pContext - pointer to session data
 *
 * Initialize the session data
 *
 * NOTE: pContext was already pulled from file (or wherever) and checked, so
 *       no need to revalidate.
 */
static inline int memmgr_internal_open(struct memmgr_session *pContext)
{
   int retval = 0;
   PTRACEIN;

   ASSERT(pContext);

   PVERBOSE("pContext=%p\n", pContext);

   // Initialize the memmgr_session structure
   memset(pContext, 0, sizeof(struct memmgr_session));

   // process id
   pContext->m_pid = current->tgid;
   PDEBUG("saving pid=%u\n", pContext->m_pid);

   // regular buffer descriptor
   // struct buf_desc_table      m_buf_table;
   // all fields already 0 due to above memset

   // virtual buffer descriptor
   // struct memmgr_virtmem      m_virtmem;
   // all fields already 0 due to above memset

   // Additional fields here ...
   // TODO virtual buffers

   PTRACEOUT_INT(retval);
   return retval;
}  // memmgr_internal_open

static inline int memmgr_internal_close(struct memmgr_session* pContext)
{
   int retval = 0;
   PTRACEIN;
   PVERBOSE("pContext=%p\n", pContext);

   /* Destruct the memmgr_session structure.
    *
    * But don't free it because its allocation strategy
    *    is left up to the caller.
    */

   // process id -- no way to destruct it

   // regular buffer descriptor
   // The hardware must have been shut down prior to calling this routine

   if ( buf_desc_table_is_init(&pContext->m_buf_table)) {
      // will clean up all buffers as well as table, so not called if buffers still around
      buf_desc_table_destruct( &pContext->m_buf_table);
   }
   else {
      // the buf_desc_table was never initialized
   }

   // virtual workspace buffers
   // The hardware must have been shut down prior to calling this routine

   if ( virtmem_is_init( &pContext->m_virtmem)) {
      // will clean up all buffers as well as table, so not called if buffers still around
      virtmem_destruct( &pContext->m_virtmem);
   }
   else {
      // the virtual memory sub-system was never initialized
   }

   // other destructors here

   goto done;

done:
   PTRACEOUT_INT( retval);
   return retval;
}  // memmgr_internal_close

/**
 * memmgr_internal_ioctl - isolate ioctl operation from file descriptor
 * @pContext: pointer to session context (e.g. file->private_data)
 * @cmd:      ioctl command
 * @arg:      ioctl argument, typically a structure pointer
 * @return:   kernel standard
 */
static inline long memmgr_internal_ioctl (struct memmgr_session* pContext, unsigned int cmd, unsigned long arg)
{
   long retval;
   PTRACEIN;

   if (pContext) {
      PVERBOSE( "pContext=%p, pid=0x%u\n", pContext, pContext->m_pid);
   } else {
      PERR( "Unexpected NULL context.\n");
      retval = -ENOMEM;
      goto done;
   }

   switch (cmd) {
      case SPL2MEM_IOCTL_MEM_OP: {
         struct spl2mem_mem_op mem_op;
         PVERBOSE( "Case SPL2MEM_IOCTL_MEM_OP\n");

         retval = copy_from_user( &mem_op, (const void __user*)arg, sizeof(mem_op));
         if (retval) {
            PERR( "Downstream failed copy_from_user, EFAULT\n");
            retval = -EFAULT;
            goto done;
         }

         // actual work done here
         switch (mem_op.e_mem_operation) {
            case e_memop_get_config_space:
               PVERBOSE( "Case e_memop_alloc_config_space\n");
               retval = internal_ioctl_get_config( pContext, &mem_op);
               break;
            case e_memop_alloc_buf:
               PVERBOSE( "Case e_memop_alloc_buf\n");
               retval = internal_ioctl_alloc( pContext, &mem_op);
               break;
            case e_memop_alloc_pagetable:
               PVERBOSE( "Case e_memop_alloc_pagetable\n");
               retval = internal_ioctl_valloc( pContext, &mem_op);
               break;
            case e_memop_free:
               PVERBOSE( "Case e_memop_free\n");
               // TODO: special case -- do not allow user to free the config space
               retval = internal_ioctl_free( pContext, &mem_op);
               break;
            default:
               PERR( "Downstream failed copy_from_user, EFAULT\n");
               retval = -EINVAL;
               break;
         }

         if(retval) goto done;      // abort on failure of called routines

         switch (mem_op.e_mem_operation) {
            case e_memop_get_config_space:   // Intentional fall through
            case e_memop_alloc_buf:          // Intentional fall through
            case e_memop_alloc_pagetable:
               // Return allocation request information to user space
               retval = copy_to_user( (void __user*)arg, &mem_op, sizeof(mem_op));
               if (retval) {
                  PERR( "Return of information failed copy_to_user, EFAULT\n");
                  retval = -EFAULT;
                  goto done;
               }
               PVERBOSE( "Successfully copied to user\n");
               break;
            case e_memop_free:      // nothing to return except retval
               break;
            default:                // cannot happen, screened in previous switch
               break;
         }
         break;
      }
      case SPL2MEM_IOCTL_DEBUG: {
         struct spl2mem_debug dbg;
         PVERBOSE( "Case SPL2MEM_IOCTL_DEBUG\n");

         retval = copy_from_user( &dbg, (const void __user*)arg, sizeof(dbg));
         if (retval) {
            PERR( "Downstream failed copy_from_user, EFAULT\n");
            retval = -EFAULT;
            goto done;
         }

         retval = internal_ioctl_debug( pContext, &dbg);
         break;
      }
      default:
         PERR( "Unknown IOCTL command seen: 0x%X", cmd);
         retval = -1;
         break;
   }  // end switch (cmd)

done:;
   PTRACEOUT_LINT( retval);
   return retval;
}  // memmgr_internal_ioctl


static inline int internal_mmap_one_buffer ( struct buf_desc       *pbuf,
                                             struct vm_area_struct *pvma,
                                             unsigned long          additional_offset,
                                             unsigned long          max_length);

/**
 * memmgr_internal_mmap - real mmap() handler
 * @pContext: the address of the struct memmgr_session
 * @pvma:     standard kernel structure passed into mmap()
 * @return:   kernel standard, will be returned to user app that called mmap()
 */
static inline int memmgr_internal_mmap  (struct memmgr_session* pContext, struct vm_area_struct *pvma)
{
   struct memmgr_virtmem  *p_virt;
   unsigned long           max_length;    // mmap length requested by user
   xsid_t                  xsid;
   struct buf_desc        *pbuf;
   int                     i;
   int                     retval = -EFAULT;
   PTRACEIN;
   PVERBOSE( "pContext=%p\n", pContext);

   if(pvma->vm_pgoff == 0 ) {
      PWARN( "Invalid WSID==0\n");
      goto done;
   }

   retval = -EINVAL;          // assume bad parameters

   // get xsid
   xsid = xsid_from_vm_pgoff( pvma->vm_pgoff);
   PDEBUG( "vm_start=0x%lX, Offset=0x%lX, xsid=" XSID_FMT "\n", pvma->vm_start, pvma->vm_pgoff, xsid);

   // compute maximum length. E.g. if buffer is 8K but user mmapped only 4K, only map 4K
   max_length = pvma->vm_end - pvma->vm_start;

   switch (memtype_of_xsid(xsid)) {
      case e_memtype_IO:            // configuration space, fall-through on purpose
      case e_memtype_regular: {     // a single buffer
         // internal description of memory backing the mapping
         pbuf = buf_desc_table_get_bufp_from_index(
                  &pContext->m_buf_table,
                  index_from_xsid( xsid));
         if( pbuf) {
            retval = internal_mmap_one_buffer( pbuf, pvma, 0, max_length);
         } else {
            PWARN( "Invalid regular or config buf_desc based on WSID\n");
         }
         break;
      }
      case e_memtype_virtualized:      // page table -- many buffers
         p_virt = &pContext->m_virtmem;
         for( i=0 ; i<p_virt->m_num_of_valid_pte; ++i) {
            pbuf = buf_desc_table_get_bufp_from_index( &p_virt->m_pt_buf_table, i);
            if( pbuf) {
               retval = internal_mmap_one_buffer( pbuf, pvma, i * p_virt->m_len_super_page, max_length);
               max_length -= p_virt->m_len_super_page;
            } else {
               PWARN( "Invalid page-table buf_desc based on WSID\n");
            }
         };
         break;
      default:
         PERR( "Free failed due to bad xsid (Workspace ID). Value=" XSID_FMT ", EFAULT\n", xsid);
         retval = -EFAULT;
         break;
   }  // switch (memtype_of_xsid(xsid))

done:;
   PTRACEOUT_INT( retval);
   return retval;
}  // memmgr_internal_mmap

/**
 * internal_mmap_one_buffer - call remap_pfn_range for a single real physical buffer
 * @pbuf:              the address of the struct memmgr_session, MUST be valid
 * @pvma:              standard kernel structure passed into mmap()
 * @additional_offset: used when stacking multiple buffers end to end in the virtual address space
 * @max_length:        maximum length that can be allocated, even if buffer is longer
 * @return:            kernel standard, will be returned to user app that called mmap()
 *
 * additional_offset is used when stacking multiple buffers end to end in the virtual address space
 */
static inline int internal_mmap_one_buffer ( struct buf_desc       *pbuf,
                                             struct vm_area_struct *pvma,
                                             unsigned long          additional_offset,
                                             unsigned long          max_length)
{
   int            retval;
   unsigned long  phys_pfn;   // physical address of buffer, as a pfn
   unsigned long  size;       // length of that buffer in bytes
   PTRACEIN;

   phys_pfn = buf_desc_get_physpfn( pbuf);
   size     = min( buf_desc_get_size( pbuf), max_length);

   PVERBOSE( "MMAP: start 0x%lX, physaddr_pfn 0x%lX, size=%ld 0x%lX\n",
             pvma->vm_start+additional_offset, phys_pfn, size, size);

   if (buf_desc_istype_io( pbuf)) pvma->vm_flags |= VM_IO;


   retval = remap_pfn_range(pvma,                              // Virtual Memory Area
                            pvma->vm_start+additional_offset,  // Start address of virtual mapping, from OS
                            phys_pfn,                          // physical memory backing store in pfn
                            size,                              // size in bytes
                            pvma->vm_page_prot);               // provided by OS
   if ( retval) {
      PWARN( "remap_pfn_range failed with error %d\n", retval);
   }

//done:
   PTRACEOUT_INT( retval);
   return retval;
}  // internal_mmap_one_buffer

#endif // __AALKERNEL_SPL2_PIP_DKSM_MEM_INTERNAL_FOPS_H__

