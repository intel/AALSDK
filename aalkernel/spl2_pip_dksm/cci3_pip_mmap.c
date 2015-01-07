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
//        FILE: cci3_pip_mmap.c
//     CREATED: 10/7/2014
//      AUTHOR: Joseph Grecco - Intel Corporation
//              Henry Mitchel - Intel Corporation
//              Tim Whisonant - Intel Corporation
//
// PURPOSE: This file implements the mmap() method of the CCI3 PIP
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
/////////////////            CCI3 PIP MMAP              ////////////////////
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
// Name: cci3_mmap
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
cci3_mmap(struct aaldev_ownerSession *pownerSess,
          struct aal_wsid            *wsidp,
          btAny                       os_specific)
{
   struct vm_area_struct   *pvma = (struct vm_area_struct *)os_specific;

   struct spl2_session     *sessp      = NULL;
   struct spl2_device      *pdev       = NULL;
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

   PINFO("WS ID = 0x%llx.\n", wsidp->m_id);

   pvma->vm_ops = NULL;


   // Special case - check for the magic xsid for the AFU Device Status Memory.
   if ( SPL2_AFUDSM_XSID == wsidp->m_id ) {
      void  *ptr;
      size_t size;

      ptr  = (void *)kosal_virt_to_phys(spl2_dev_AFUDSM(pdev));
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

      PVERBOSE("Mapping CSR %s Aperture Physical=0x%p size=%" PRIuSIZE_T " at uvp=0x%p\n",
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


   //------------------------
   // Map normal workspace
   //------------------------

   max_length = min(wsidp->m_size,(btWSSize)(pvma->vm_end - pvma->vm_start));

   PVERBOSE( "MMAP: start 0x%lx, end 0x%lx, KVP 0x%p, size=%" PRIu64 " 0x%" PRIx64 " max_length=%ld flags=0x%lx\n",
              pvma->vm_start, pvma->vm_end, (btVirtAddr)wsidp->m_id, wsidp->m_size, wsidp->m_size, max_length, pvma->vm_flags);

    res = remap_pfn_range(pvma,                              // Virtual Memory Area
                          pvma->vm_start,                    // Start address of virtual mapping, from OS
                          (kosal_virt_to_phys((btVirtAddr)wsidp->m_id)>>PAGE_SHIFT),   // physical memory backing store in pfn
                          max_length,                        // size in bytes
                          pvma->vm_page_prot);               // provided by OS
    if ( unlikely(0 != res) ) {
      PERR("remap_pfn_range error at workspace mmap %d\n", res);
      goto ERROR;
    }

ERROR:
   return res;
}

