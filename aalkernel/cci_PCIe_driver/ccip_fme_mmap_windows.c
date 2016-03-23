//****************************************************************************
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//       redistributing this file, you may do so under either license.
//
//                             GPL LICENSE SUMMARY
//
//       Copyright (c) 2011-2016 Intel Corporation. All rights reserved.
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
//       Copyright (c) 2011-2016 Intel Corporation. All rights reserved.
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
//        FILE: ccip_fme_mmap_windows.c
//     CREATED: 02/28/2013
//      AUTHOR: Joseph Grecco - Intel Corporation
//
// PURPOSE: This file implements the mmap() method of the SPL2 PIP
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 10/26/2013     HM       Unified formatting of size parameter in debug msgs
//****************************************************************************
#include <aalsdk/kernel/kosal.h>
#define MODULE_FLAGS CCIPCIE_DBG_MOD // Prints all

#include "aalsdk/kernel/ccipdriver.h"
#include "cci_pcie_driver_PIPsession.h"

//=============================================================================
// Name: cci_mmap
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
cci_mmap(struct aaldev_ownerSession *pownerSess,
          struct aal_wsid            *wsidp,
          btAny                       pVirt)
{  

   ASSERT(pownerSess);
   ASSERT(wsidp);
   ASSERT(pVirt);

   *(btVirtAddr*)pVirt=NULL;


   struct cci_PIPsession      *pSess = NULL;
   struct cci_aal_device      *pdev = NULL;
   int                         res = -EINVAL;

   ASSERT( pownerSess );
   ASSERT( wsidp );

   // Get the aal_device and the memory manager session
   pSess = ( struct cci_PIPsession * ) aalsess_pipHandle( pownerSess );
   ASSERT( pSess );
   if( NULL == pSess ) {
      PDEBUG( "CCI mmap: no Session" );
      goto ERROR;
   }

   pdev = cci_PIPsessionp_to_ccidev( pSess );
   ASSERT( pdev );
   if( NULL == pdev ) {
      PDEBUG( "CCI mmap: no device" );
      goto ERROR;
   }

   PINFO( "WS ID = 0x%llx.\n", wsidp->m_id );

   // Special case - check the wsid type for WSM_TYPE_CSR. If this is a request to map the
   // CSR region, then satisfy the request by mapping PCIe BAR 0.
   if( WSM_TYPE_MMIO == wsidp->m_type ) {
      void *ptr               = NULL;
      btUnsigned64bitInt size = 0;
      switch( wsidp->m_id ) {
         case WSID_CSRMAP_WRITEAREA:
         case WSID_CSRMAP_READAREA:
         case WSID_MAP_MMIOR:
         case WSID_MAP_UMSG:
            break;
         default:
            PERR( "Attempt to map invalid WSID type %d\n", (int)wsidp->m_id );
            goto ERROR;
      }

      // Verify that we can fulfill the request - we set flags at create time.
      if( WSID_CSRMAP_WRITEAREA == wsidp->m_id ) {
         ASSERT( cci_dev_allow_map_csr_write_space( pdev ) );

         if( !cci_dev_allow_map_csr_write_space( pdev ) ) {
            PERR( "Denying request to map CSR Write space for device 0x%p.\n", pdev );
            goto ERROR;
         }
      }

      if( WSID_CSRMAP_READAREA == wsidp->m_id ) {
         ASSERT( cci_dev_allow_map_csr_read_space( pdev ) );

         if( !cci_dev_allow_map_csr_read_space( pdev ) ) {
            PERR( "Denying request to map CSR Read space for device 0x%p.\n", pdev );
            goto ERROR;
         }
      }
      
      switch( wsidp->m_id ) {
         case WSID_MAP_MMIOR:
         {
            if( !cci_dev_allow_map_mmior_space( pdev ) ) {
               PERR( "Denying request to map cci_dev_allow_map_mmior_space Read space for device 0x%p.\n", pdev );
               goto ERROR;
            }

            ptr = (void *)cci_dev_kvp_afu_mmio( pdev );
            size = cci_dev_len_afu_mmio( pdev );
         }
         break;

         case WSID_MAP_UMSG:
         {
            if( !cci_dev_allow_map_umsg_space( pdev ) ) {
               PERR( "Denying request to map cci_dev_allow_map_umsg_space Read space for device 0x%p.\n", pdev );
               goto ERROR;
            }

            ptr = (void *)cci_dev_phys_afu_umsg( pdev );
            size = cci_dev_len_afu_umsg( pdev );
         }
         break;
         
         default:
            PERR( "Unrecognized MMIO region ID %d\n", wsidp->m_id );
            return res;
      }
      // Create an MDL around the system memory
      wsid_to_maphandle( wsidp ) = IoAllocateMdl( ptr, (ULONG)size, FALSE, FALSE, NULL );
      if( NULL == wsid_to_maphandle( wsidp ) ) {
         PERR("Failed to create MDL\n");
         goto ERROR;
      }

      MmBuildMdlForNonPagedPool( wsid_to_maphandle( wsidp ) );

      try{
         // Map the memory into user land
         *(btVirtAddr*)pVirt = MmMapLockedPagesSpecifyCache( wsid_to_maphandle( wsidp ),
                                                               UserMode,
                                                               MmNonCached,
                                                               NULL,
                                                               FALSE,
                                                               HighPagePriority);
      } except(EXCEPTION_EXECUTE_HANDLER){
         *(btVirtAddr*)pVirt = NULL;
      }
      return 0;
   } // End MMIO Mappings
#if 0 //TODO FME DOES NOT ALLOCATE WS BUT IF THIS BECOMES COMMON CODE IT WILL HAVE TO DO NORMAL WS

   //------------------------
   // Map normal workspace
   //------------------------

   max_length = min( wsidp->m_size, (btWSSize)( pvma->vm_end - pvma->vm_start ) );

   PVERBOSE( "MMAP: start 0x%lx, end 0x%lx, KVP 0x%p, size=%" PRIu64 " 0x%" PRIx64 " max_length=%ld flags=0x%lx\n",
             pvma->vm_start, pvma->vm_end, (btVirtAddr)wsidp->m_id, wsidp->m_size, wsidp->m_size, max_length, pvma->vm_flags );

   res = remap_pfn_range( pvma,                              // Virtual Memory Area
                          pvma->vm_start,                    // Start address of virtual mapping, from OS
                          ( kosal_virt_to_phys( (btVirtAddr)wsidp->m_id ) >> PAGE_SHIFT ),   // physical memory backing store in pfn
                          max_length,                        // size in bytes
                          pvma->vm_page_prot );               // provided by OS
   if( unlikely( 0 != res ) ) {
      PERR( "remap_pfn_range error at workspace mmap %d\n", res );
      goto ERROR;
   }
#endif


ERROR:
   return res;
}
