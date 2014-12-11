//****************************************************************************
// Part of the  Intel(R)  QuickAssist Technology Accelerator Abstraction Layer
//
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//       redistributing this file, you may do so under either license.
//
//                             GPL LICENSE SUMMARY
//
//       Copyright (c) 2011-2012 Intel Corporation. All rights reserved.
//
// This program is free software;  you can  redistribute it  and/or modify  it
// under  the  terms  and  conditions  of  the  GNU  General  Public  License,
// version 2, as published by the Free Software Foundation.
//
// This program  is distributed  in the  hope it  will be useful,  but WITHOUT
// ANY WARRANTY;  without  even  the implied  warranty  of  MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the  GNU General Public License  for
// more details.
//
// You should  have received  a copy  of the  GNU General Public License along
// with this program;  if not,  write  to  the Free Software Foundation, Inc.,
// 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
//
// The full GNU General Public License is included in this distribution in the
// file called README.GPLV2-LICENSE.TXT.
//
// Contact Information:
// Henry Mitchel, henry.mitchel at intel.com
// 77 Reed Rd., Hudson, MA  01749
//
//                                 BSD LICENSE
//
//       Copyright (c) 2011-2012 Intel Corporation. All rights reserved.
//
// Redistribution  and  use  in source  and  binary  forms,  with  or  without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of  source code must retain the  above copyright notice
//     this list of conditions and the following disclaimer.
//   * Redistributions in  binary  form  must  reproduce  the  above copyright
//     notice,  this list of  conditions and  the following disclaimer  in the
//     documentation and/or other materials provided with the distribution.
//   * Neither the name of Intel Corporation nor the names of its contributors
//     may be used  to endorse or promote  products derived from this software
//     without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES,  INCLUDING,  BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT  SHALL THE COPYRIGHT OWNER  OR CONTRIBUTORS BE
// LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR
// CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT  LIMITED TO,  PROCUREMENT  OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,  DATA,  OR PROFITS;  OR BUSINESS
// INTERRUPTION)  HOWEVER  CAUSED  AND ON ANY THEORY OF LIABILITY,  WHETHER IN
// CONTRACT,  STRICT  LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,  EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//****************************************************************************
//****************************************************************************
//        FILE: WINSPL3_pip_mmap.c
//     CREATED: 02/28/2013
//      AUTHOR: Joseph Grecco - Intel Corporation
//
// PURPOSE: This file implements the mmap() method of the SPL2 PIP
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 10/26/2013     HM       Unified formatting of size parameter in debug msgs
//****************************************************************************
#include <kosal.h>
#define MODULE_FLAGS SPL2_DBG_MMAP

#include "WinSPL2.h"

#include "spl2_session.h"
#include "spl2mem.h"

#include <fappip.h>


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

btVirtAddr
internal_mmap_one_buffer(struct buf_desc       *pbuf,
                         btVirtAddr             uvstartaddr,
                         unsigned long          max_length);


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
          btAny                       pVirt)
{
   struct spl2_session     *sessp      = NULL;
   struct spl2_device      *pdev       = NULL;
   struct memmgr_virtmem   *p_virt     = NULL;
   struct memmgr_session   *pmem_sess  = NULL;
   struct buf_desc         *pbuf;
   xsid_t                   xsid;
   unsigned int             i;
   unsigned long            max_length = 0; // mmap length requested by user
   int                      res        = -1;
   btVirtAddr               nextAddr = NULL;
   

   ASSERT(pownerSess);
   ASSERT(wsidp);
   ASSERT(pVirt);

   *(btVirtAddr*)pVirt=NULL;


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

   // Special case - check for the magic xsid for the AFU Device Status Memory.
   if ( SPL2_AFUDSM_XSID == xsid ) {
      void  *ptr = NULL;
      ULONG size;

      size = spl2_dev_AFUDSM_size;
      
      // Needed for cleanup.
      sessp->m_wswsidp = wsidp;

      // Create an MDL around the system memory
      sessp->m_wsmap = IoAllocateMdl((void*)spl2_dev_AFUDSM(pdev),size,FALSE,FALSE,NULL);
      if(NULL == sessp->m_wsmap){
         PERR("Failed to create MDL\n");
         goto ERROR;
      }

      MmBuildMdlForNonPagedPool(sessp->m_wsmap);

      PVERBOSE("Got SPL2_AFUDSM_XSID, mapping AFU DSM phys=0x%p size=0x%lX\n",ptr, size);

      try{
         // Map the memory into user land
         sessp->m_wsuvAddr = MmMapLockedPagesSpecifyCache( sessp->m_wsmap,
                                                           UserMode,
                                                           MmNonCached,
                                                           NULL,
                                                           FALSE,
                                                           HighPagePriority);
      } except(EXCEPTION_EXECUTE_HANDLER){
         sessp->m_wsuvAddr=NULL;
      }
      // Successfully mapped AFU Device Status Memory region.
      *(btVirtAddr*)pVirt = sessp->m_wsuvAddr;
      return 0;
   }


   // Special case - check the wsid type for WSM_TYPE_CSR. If this is a request to map the SPL2
   // CSR region, then satisfy the request by mapping PCIe BAR 0.
   // TODO MAKE SURE YOU CAN"T MAP TWICE
   if ( WSM_TYPE_CSR == wsidp->m_type ) {
      ULONG  size;
      
      // This will have to be cleaned up
      sessp->m_csrwsidp = wsidp;

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
      size = (ULONG)spl2_dev_len_config(pdev);

      PVERBOSE("Mapping CSR %s Aperture phys=0x%p virt %p size=0x%lX",
                  ((WSID_CSRMAP_WRITEAREA == wsidp->m_id) ? "write" : "read"),
                  (void*)spl2_dev_phys_config(pdev),
                  (void*)spl2_dev_kvp_config(pdev),
                  size);

        // Create an MDL around the system memory
      sessp->m_csrmap = IoAllocateMdl((void*)spl2_dev_kvp_config(pdev),size,FALSE,FALSE,NULL);
      if(NULL == sessp->m_csrmap){
         PERR("Failed to create MDL\n");
         goto ERROR;
      }

      MmBuildMdlForNonPagedPool(sessp->m_csrmap);

      PVERBOSE("Got SPL2_AFUDSM_XSID, mapping AFU DSM phys=0x%p size=0x%lX\n",(void*)spl2_dev_kvp_config(pdev),size);
      sessp->m_csruvAddr = NULL;
      try{
         // Map the memory into user land
         sessp->m_csruvAddr = MmMapLockedPagesSpecifyCache( sessp->m_csrmap,
                                                            UserMode,
                                                            MmNonCached,
                                                            NULL,
                                                            FALSE,
                                                            HighPagePriority);
      } except(EXCEPTION_EXECUTE_HANDLER){
         sessp->m_csruvAddr = NULL;
         *(btVirtAddr*)pVirt=NULL;
      }
       // Successfully mapped CSR region.
      *(btVirtAddr*)pVirt = sessp->m_csruvAddr;
      return 0;
   }


   p_virt = &pmem_sess->m_virtmem;

   // Let kernel set first address
   pmem_sess->m_virtmem.m_pte_array_uvaddr = NULL;

   max_length = (ULONG)wsidp->m_size;
   nextAddr = NULL;
   // Go through the page table and map each entry into user space one at a time.
   for ( i = 0 ; (0 != max_length) && i < memmgr_virtmem_max_sw_ptes(p_virt) ; ++i ) {
      pbuf = buf_desc_table_get_bufp_from_index(&p_virt->m_pt_buf_table, i);
      
      ASSERT(pbuf);
      if ( pbuf ) {
         nextAddr = internal_mmap_one_buffer(pbuf, nextAddr, max_length);
         ASSERT(0 != nextAddr);
         if ( NULL == nextAddr ) {
            // TODO REALLY NEED TO CLEAN UP PREVIOUS ALLOCATIONS
            goto ERROR;
         }

         // Save the beginning of the buffer in user space
         if(NULL == pmem_sess->m_virtmem.m_pte_array_uvaddr){
            pmem_sess->m_virtmem.m_pte_array_uvaddr = nextAddr;
         }
         max_length -= memmgr_virtmem_sp_size_in_bytes(p_virt);
         nextAddr += memmgr_virtmem_sp_size_in_bytes(p_virt);

      } else {
         PWARN("Invalid page-table buf_desc based on WSID\n");
         goto ERROR;
      }
   }

   *(btVirtAddr*)pVirt = pmem_sess->m_virtmem.m_pte_array_uvaddr;
   return 0;

ERROR:
   return res;
}

//=============================================================================
// Name: internal_mmap_one_buffer
// Description: call remap_pfn_range for a single real physical buffer
// Interface: public
// Inputs: pbuf - the address of the struct memmgr_session, MUST be valid
//         uvaddr - user virtual address to start at
//         additional_offset - used when stacking multiple buffers end to end in the virtual address space
//         max_length - maximum length that can be allocated, even if buffer is longer
// Returns: kernel standard, will be returned to user app that called mmap().
// Comments:
//=============================================================================
btVirtAddr
internal_mmap_one_buffer(struct buf_desc       *pbuf,
                         btVirtAddr             uvstartaddr,
                         unsigned long          max_length)
{
   btVirtAddr     kvbufp = NULL;     // kernel virtual buffer
   unsigned long  size = 0;       // length of that buffer in bytes

   PTRACEIN;


   size   = (unsigned long)min( buf_desc_get_size_in_bytes(pbuf), max_length);
   kvbufp = buf_desc_get_virtaddr(pbuf);
   

   PVERBOSE( "MMAP: start 0x%lX, size=0x%lX max_length=%ld\n",uvstartaddr, size, max_length);
   // Create an MDL around the system memory
   pbuf->m_pmdl = IoAllocateMdl(kvbufp,size,FALSE,FALSE,NULL);

   if(NULL == buf_desc_pmdl(pbuf)){
      PERR("Failed to crrate MDL\n");
      return NULL;
   }

   MmBuildMdlForNonPagedPool(pbuf->m_pmdl);

   PVERBOSE("Got SPL2_AFUDSM_XSID, mapping AFU DSM phys=0x%p size=0x%lX\n",kvbufp,size);

   try{
      // Map the memory into user land
      pbuf->m_uvaddr = MmMapLockedPagesSpecifyCache( pbuf->m_pmdl ,
                                                     UserMode,
                                                     MmNonCached,
                                                     uvstartaddr,
                                                     FALSE,
                                                     HighPagePriority);
   } except(EXCEPTION_EXECUTE_HANDLER){
      PDEBUG("Exception code %lx\n",GetExceptionCode());
      IoFreeMdl(pbuf->m_pmdl);
      pbuf->m_pmdl = NULL;
      pbuf->m_uvaddr = NULL;
   }

   PTRACEOUT_INT(pbuf->m_uvaddr);
   return pbuf->m_uvaddr;
}  // internal_mmap_one_buffer
