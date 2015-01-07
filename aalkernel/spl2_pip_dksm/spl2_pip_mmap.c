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
//        FILE: encoder-kbae-mmap.c
//     CREATED: 02/18/2011
//      AUTHOR: Joseph Grecco - Intel Corporation
//
// PURPOSE: This file implements the mmap() method of the SPL2 PIP
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
//****************************************************************************
#include "aalsdk/kernel/kosal.h"

#define MODULE_FLAGS SPL2_DBG_MMAP

#include "spl2_session.h"
#include "mem-sess.h"
#include "mem-virt.h"
#include "aalsdk/kernel/spl2defs.h"
#include "aalsdk/kernel/fappip.h"
#include "xsid.h"

//////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////            SPL2 PIP MMAP              ////////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
//=============================================================================

//=============================================================================
// Name: csr_vmaopen
// Description: Called when the vma is mapped
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments:
//=============================================================================
#ifdef NOT_USED
static void csr_vmaopen(struct vm_area_struct *pvma)
{
   PINFO("CSR VMA OPEN.\n" );
}
#endif


//=============================================================================
// Name: wksp_vmaclose
// Description: called when vma is unmapped
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments:
//=============================================================================
#ifdef NOT_USED
static void csr_vmaclose(struct vm_area_struct *pvma)
{
   PINFO("CSR VMA CLOSE.\n" );
}
#endif

#ifdef NOT_USED
static struct vm_operations_struct csr_vma_ops =
{
   .open    = csr_vmaopen,
   .close   = csr_vmaclose,
};
#endif

//=============================================================================
// Name: internal_mmap_one_buffer
// Description: call remap_pfn_range for a single real physical buffer
// Interface: public
// Inputs: pbuf - the address of the struct memmgr_session, MUST be valid
//         pvma - standard kernel structure passed into mmap()
//         additional_offset - used when stacking multiple buffers end to end in the virtual address space
//         max_length - maximum length that can be allocated, even if buffer is longer
// Returns: kernel standard, will be returned to user app that called mmap().
// Comments:
//=============================================================================
static inline
int
internal_mmap_one_buffer(struct buf_desc       *pbuf,
                         struct vm_area_struct *pvma,
                         unsigned long          additional_offset,
                         unsigned long          max_length)
{
   int            retval;
   unsigned long  phys_pfn;   // physical address of buffer, as a pfn
   unsigned long  size;       // length of that buffer in bytes
   PTRACEIN;

   phys_pfn = buf_desc_get_physpfn(pbuf);
   size     = min( buf_desc_get_size(pbuf), max_length);

   if ( buf_desc_istype_io(pbuf) ) {
      PVERBOSE("MMAP: setting VM_IO\n");
      pvma->vm_flags |= VM_IO;
   }

   PVERBOSE( "MMAP: start 0x%lX, physaddr_pfn 0x%lX, size=%ld 0x%lX max_length=%ld flags=0x%lx\n",
             pvma->vm_start+additional_offset, phys_pfn, size, size, max_length, pvma->vm_flags);

   retval = remap_pfn_range(pvma,                              // Virtual Memory Area
                            pvma->vm_start+additional_offset,  // Start address of virtual mapping, from OS
                            phys_pfn,                          // physical memory backing store in pfn
                            size,                              // size in bytes
                            pvma->vm_page_prot);               // provided by OS
   ASSERT(0 == retval);
   if ( retval ) {
      PWARN( "remap_pfn_range failed with error %d\n", retval);
   }

   PTRACEOUT_INT(retval);
   return retval;
}  // internal_mmap_one_buffer


//=============================================================================
// Name: spl2_mmap
// Description: Method used for mapping kernel memory to user space. Called by
//              uidrv.
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments: This method front ends all operations that require mapping shared
//           memory. It examines the wsid to determine the appropriate service
//           to perform the map operation.
//=============================================================================
int
spl2_mmap(struct aaldev_ownerSession *pownerSess,
          struct aal_wsid            *wsidp,
          btAny                       os_specific)
{
   struct vm_area_struct   *pvma = (struct vm_area_struct *)os_specific;

   struct spl2_session     *sessp      = NULL;
   struct spl2_device      *pdev       = NULL;
   struct memmgr_virtmem   *p_virt     = NULL;
   struct memmgr_session   *pmem_sess  = NULL;
   struct buf_desc         *pbuf;
   xsid_t                   xsid;
   int                      i;
   unsigned long            max_length = 0; // mmap length requested by user
   int                      res        = -EINVAL;

   ASSERT(pownerSess);
   ASSERT(wsidp);

   // Get the spl2 aal_device and the memory manager session
   sessp = (struct spl2_session *)aalsess_pipHandle(pownerSess);
   ASSERT(sessp);
   if ( NULL == sessp ) {
      PDEBUG("SPL2 mmap: no Session");
      goto ERROR;
   }

   pdev = spl2_sessionp_to_spl2dev(sessp);
   ASSERT(pdev);
   if ( NULL == pdev ) {
      PDEBUG("SPL2 mmap: no SPL2 device");
      goto ERROR;
   }

   pmem_sess = spl2_dev_mem_sessionp(pdev);
   ASSERT(pmem_sess);
   if ( NULL == pmem_sess ) {
      PDEBUG("SPL2 mmap: no memory Session");
      goto ERROR;
   }

   PINFO("WS ID = 0x%llx.\n", wsidp->m_id);

   xsid = (xsid_t)wsidp->m_id;
   pvma->vm_ops = NULL;


   // Special case - check for the magic xsid for the AFU Device Status Memory.
   if ( SPL2_AFUDSM_XSID == xsid ) {
      void  *ptr;
      size_t size;

      ptr  = (void *)virt_to_phys(spl2_dev_AFUDSM(pdev));
      size = spl2_dev_AFUDSM_size;

      PVERBOSE("Got SPL2_AFUDSM_XSID, mapping AFU DSM phys=0x%p size=%" PRIuSIZE_T " at uvp=0x%p.\n",
                  ptr,
                  size,
                  (void *)pvma->vm_start);

      // Capture the special xsid for the AFU DSM and map it accordingly.
      res = remap_pfn_range(pvma,                               // Virtual Memory Area
                            pvma->vm_start,                     // Start address of virtual mapping
                            ((unsigned long)ptr) >> PAGE_SHIFT, // PFN of address of CSR map
                            size,                               // Size of workspace
                            pvma->vm_page_prot);

      if ( unlikely( res < 0 ) ) {
         PERR("remap_pfn_range error at AFU DSM mmap %d\n", res);
         goto ERROR;
      }

      // Successfully mapped AFU Device Status Memory region.
      return 0;
   }


   // Special case - check the wsid type for WSM_TYPE_CSR. If this is a request to map the SPL2
   // CSR region, then satisfy the request by mapping PCIe BAR 0.
   if ( WSM_TYPE_CSR == wsidp->m_type ) {
      void  *ptr;
      size_t size;

      if ( (WSID_CSRMAP_WRITEAREA != wsidp->m_id) &&
           (WSID_CSRMAP_READAREA  != wsidp->m_id) ) {
        PERR("Attempt to map invalid WSID type %d\n", (int)wsidp->m_id);
        goto ERROR;
      }

      // Verify that we can fulfill the request - we set flags at create time.
      if ( WSID_CSRMAP_WRITEAREA == wsidp->m_id ) {
         ASSERT(spl2_dev_allow_map_csr_write_space(pdev));

         if ( !spl2_dev_allow_map_csr_write_space(pdev) ) {
            PERR("Denying request to map CSR Write space for device 0x%p.\n", pdev);
            goto ERROR;
         }
      }

      if ( WSID_CSRMAP_READAREA == wsidp->m_id ) {
         ASSERT(spl2_dev_allow_map_csr_read_space(pdev));

         if ( !spl2_dev_allow_map_csr_read_space(pdev) ) {
            PERR("Denying request to map CSR Read space for device 0x%p.\n", pdev);
            goto ERROR;
         }
      }

      // Map the PCIe BAR as the CSR region.
      ptr  = (void *)spl2_dev_phys_afu_csr(pdev);
      size = spl2_dev_len_afu_csr(pdev);

      PVERBOSE("Mapping CSR %s Aperture phys=0x%p size=%" PRIuSIZE_T " at uvp=0x%p\n",
                  ((WSID_CSRMAP_WRITEAREA == wsidp->m_id) ? "write" : "read"),
                  ptr,
                  size,
                  (void *)pvma->vm_start);

      // Map the region to user VM
      res =  remap_pfn_range(pvma,                             // Virtual Memory Area
                             pvma->vm_start,                   // Start address of virtual mapping
                             ((unsigned long)ptr)>>PAGE_SHIFT, // Pointer in Pages (Page Frame Number)
                             size,
                             pvma->vm_page_prot);

       if ( unlikely(0 != res) ) {
         PERR("remap_pfn_range error at CSR mmap %d\n", res);
         goto ERROR;
       }

       // Successfully mapped CSR region.
       return 0;
   }


   p_virt = &pmem_sess->m_virtmem;

   // Save the user virtual address of start of workspace
   pmem_sess->m_virtmem.m_pte_array_uvaddr = pvma->vm_start;

   max_length = pvma->vm_end - pvma->vm_start;

   // Go through the page table and map each entry into user space one at a time.
   for ( i = 0 ; i < p_virt->m_num_of_valid_pte ; ++i ) {
      pbuf = buf_desc_table_get_bufp_from_index(&p_virt->m_pt_buf_table, i);
      ASSERT(pbuf);
      if ( pbuf ) {
         res = internal_mmap_one_buffer(pbuf, pvma, i * p_virt->m_len_super_page, max_length);
         ASSERT(0 == res);
         if ( res ) {
            goto ERROR;
         }
         max_length -= p_virt->m_len_super_page;
      } else {
         PWARN("Invalid page-table buf_desc based on WSID\n");
         goto ERROR;
      }
   }

   res = 0;

ERROR:
   return res;
}

