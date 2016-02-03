//******************************************************************************
// Part of the Intel(R) QuickAssist Technology Accelerator Abstraction Layer
//
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2012-2016, Intel Corporation.
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
//  Copyright(c) 2012-2016, Intel Corporation.
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
/// @file kOSAL.c
/// @brief Implementation of the Kernel abstraction functions.
/// @ingroup kOSAL
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS:  Joseph Grecco, Intel Corporation
///           Tim Whisonant, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 12/27/2012     JG       Initial version
//****************************************************************************
#include <linux/vmalloc.h>
#include "aalsdk/kernel/kosal.h"
#define MODULE_FLAGS KOSAL_DBG_MOD

#if defined( __AAL_LINUX__ )
# include <linux/delay.h>
#endif // __AAL_LINUX__

#if defined( __AAL_UNKNOWN_OS__ )
# error Implement kOSAL for unknown OS.
#endif // __AAL_UNKNOWN_OS__

//=============================================================================
/// kosal_pci_read_config_dword
/// @brief     Read a dword from PCIe device Config space
/// @param[in] dev - PCI device handle
///            offset - offset into Config space starting from beginning of 
//                      header.
//             
/// @return    0 failed
///
/// @note   Windows intrinsic numbers bits 0 to 63, ffsll numbers
///         them 1 to 64. Using ffsll variation.
/// @note   Windows intrinsic must be set outside function
//
//=============================================================================
btInt _kosal_pci_read_config_dword(__ASSERT_HERE_PROTO
                                   pkosal_pci_dev      pdev,
                                   btUnsigned32bitInt  offset,
                                   btUnsigned32bitInt *pval)
{
   btInt res;
#if   defined( __AAL_WINDOWS__ )
   ULONG bytesRead = 0;
#endif // __AAL_WINDOWS__

   __ASSERT_HERE_IN_FN(NULL != pdev);
   __ASSERT_HERE_IN_FN(NULL != pval);

#if   defined( __AAL_LINUX__ )

   res = ( pci_read_config_dword(pdev, offset, pval) ? 0 : 1 );

#elif defined( __AAL_WINDOWS__ )

   bytesRead = pdev->GetBusData(pdev->Context,             // Context of bus interface
                                PCI_WHICHSPACE_CONFIG,     // Config space
                                pval,                      // Where to return it
                                offset,                    // From the beginning
                                sizeof(btUnsigned32bitInt));

   res = ( sizeof(btUnsigned32bitInt) == bytesRead ? 1 : 0 );

#endif // OS

   PPCI_HERE("kosal_pci_read_config_dword(pdev=0x%" PRIxUINTPTR_T ", offset=%u [0x%x], pval=0x%" PRIxUINTPTR_T ") : *pval=0x%x [%u]\n",
                __UINTPTR_T_CAST(pdev),
                offset, offset,
                __UINTPTR_T_CAST(pval),
                *pval, *pval);

   return res;
}

//=============================================================================
/// kosal_virt_to_phys
/// @brief     Convert a kernel virtual address to physical
/// @param[in] vaddr - kernel virtual
/// @return    physica address or NULLif  failed
//=============================================================================
btPhysAddr
kosal_virt_to_phys(btAny vaddr)
{
#if   defined( __AAL_LINUX__ )

   return (btPhysAddr)virt_to_phys(vaddr);

#elif defined( __AAL_WINDOWS__ )

   PHYSICAL_ADDRESS physaddr = { 0 };
     
   physaddr = MmGetPhysicalAddress(vaddr);

   return (btPhysAddr)physaddr.QuadPart;

#endif // OS
}

//=============================================================================
/// kosal_mdelay
/// @brief     Delay in milliseconds
/// @param[in] time in millisecs
/// @return    1 if success
/// @note      This function may only be called when IRQL <= APC_LEVEL
//=============================================================================
btInt _kosal_mdelay(__ASSERT_HERE_PROTO btTime delay)
{
   btInt res;
#if   defined( __AAL_WINDOWS__ )
   NTSTATUS      status;
   LARGE_INTEGER rel_timout_in_100ns;
# if ENABLE_ASSERT
   UNREFERENCED_PARAMETER(__file);
   UNREFERENCED_PARAMETER(__line);
   UNREFERENCED_PARAMETER(__fn);
# endif // ENABLE_ASSERT
#endif // __AAL_WINDOWS__

   PPOLLING_HERE("kosal_mdelay(delay=%llu [0x%llx]) sleeping\n", delay, delay);

#if   defined( __AAL_LINUX__ )

   mdelay(delay);
   res = 1;

#elif defined( __AAL_WINDOWS__ )

   // Relative time is specified as a negative value.
   // Timeout is specified in 100 ns.
   rel_timout_in_100ns.QuadPart = -((LONGLONG)delay * 10000);

   status = KeDelayExecutionThread(KernelMode, FALSE, &rel_timout_in_100ns);

   res = ( NT_SUCCESS(status) ? 1 : 0 );

#endif // OS

   PPOLLING_HERE("kosal_mdelay(delay=%llu [0x%llx]) waking\n", delay, delay);
   return res;
}

//=============================================================================
/// kosal_udelay
/// @brief     Delay in microseconds
/// @param[in] time in micro seconds
/// @return    1 if success
/// @note      This function may only be called when IRQL <= APC_LEVEL
//=============================================================================
btInt _kosal_udelay(__ASSERT_HERE_PROTO btTime delay)
{
   btInt res;
#if   defined( __AAL_WINDOWS__ )
   NTSTATUS      status;
   LARGE_INTEGER rel_timout_in_100ns;
# if ENABLE_ASSERT
   UNREFERENCED_PARAMETER(__file);
   UNREFERENCED_PARAMETER(__line);
   UNREFERENCED_PARAMETER(__fn);
# endif // ENABLE_ASSERT
#endif // __AAL_WINDOWS__

   PPOLLING_HERE("kosal_udelay(delay=%llu [0x%llx]) sleeping\n", delay, delay);

#if   defined( __AAL_LINUX__ )

   mdelay(delay);
   res = 1;

#elif defined( __AAL_WINDOWS__ )
   
   // Relative time is specified as a negative value.
   // Timeout is specified in 100 ns.
   rel_timout_in_100ns.QuadPart = -((LONGLONG)delay * 10);

   status = KeDelayExecutionThread(KernelMode, FALSE, &rel_timout_in_100ns);
   res = ( NT_SUCCESS(status) ? 1 : 0 );

#endif // OS

   PPOLLING_HERE("kosal_udelay(delay=%llu [0x%llx]) waking\n", delay, delay);
   return res;
}

btVirtAddr _kosal_kmalloc(__ASSERT_HERE_PROTO btWSSize size_in_bytes)
{
   btVirtAddr krnl_virt = NULL;

   __ASSERT_HERE_IN_FN(size_in_bytes > 0);

#if   defined( __AAL_LINUX__ )
# ifdef __i386__
   krnl_virt = __get_free_pages(GFP_KERNEL, get_order(kosal_round_up_to_page_size(size_in_bytes)));
# else
   krnl_virt = kmalloc((size_t)size_in_bytes, GFP_KERNEL);
# endif // __i386__
#elif defined( __AAL_WINDOWS__ )

   krnl_virt = (btVirtAddr)MmAllocateNonCachedMemory((SIZE_T)size_in_bytes);

#endif // OS

   __ASSERT_HERE_IN_FN(NULL != krnl_virt);

   PMEMORY_HERE("kosal_kmalloc(size=%llu [0x%llx]) = 0x%" PRIxUINTPTR_T " [phys=0x%" PRIxPHYS_ADDR "]\n",
                   size_in_bytes, size_in_bytes,
                   __UINTPTR_T_CAST(krnl_virt),
                   kosal_virt_to_phys(krnl_virt));

   return krnl_virt;
}

btVirtAddr _kosal_kzmalloc(__ASSERT_HERE_PROTO btWSSize size_in_bytes)
{
   btVirtAddr krnl_virt = NULL;

   __ASSERT_HERE_IN_FN(size_in_bytes > 0);

#if   defined( __AAL_LINUX__ )
# ifdef __i386__
   krnl_virt = __get_free_pages(GFP_KERNEL, get_order(kosal_round_up_to_page_size(size_in_bytes)));
   if(krnl_virt){
      memset(krnl_virt, 0 , (size_t)size_in_bytes);
   }
# else
   krnl_virt = kmalloc((size_t)size_in_bytes, GFP_KERNEL);
   if(krnl_virt){
      memset(krnl_virt, 0 , (size_t)size_in_bytes);
   }
# endif // __i386__
#elif defined( __AAL_WINDOWS__ )

   krnl_virt = (btVirtAddr)MmAllocateNonCachedMemory((SIZE_T)size_in_bytes);
   if(krnl_virt){
      RtlZeroMemory(krnl_virt, 0 , (SIZE_T)size_in_bytes);
   }
#endif // OS

   __ASSERT_HERE_IN_FN(NULL != krnl_virt);

   PMEMORY_HERE("kosal_kzmalloc(size=%llu [0x%llx]) = 0x%" PRIxUINTPTR_T " [phys=0x%" PRIxPHYS_ADDR "]\n",
                   size_in_bytes, size_in_bytes,
                   __UINTPTR_T_CAST(krnl_virt),
                   kosal_virt_to_phys(krnl_virt));

   return krnl_virt;
}

void _kosal_kfree(__ASSERT_HERE_PROTO btAny krnl_virt, btWSSize size_in_bytes)
{
   __ASSERT_HERE_IN_FN(NULL != krnl_virt);
   __ASSERT_HERE_IN_FN(size_in_bytes > 0);

   PMEMORY_HERE("kosal_kfree(0x%" PRIxUINTPTR_T " [phys=0x%" PRIxPHYS_ADDR "], size=%llu [0x%llx])\n",
                   __UINTPTR_T_CAST(krnl_virt),
                   kosal_virt_to_phys(krnl_virt),
                   size_in_bytes, size_in_bytes);

   if ( NULL != krnl_virt ) {

#if   defined( __AAL_LINUX__ )
# ifdef __i386__
      free_pages(krnl_virt, get_order(kosal_round_up_to_page_size(size_in_bytes)));
# else
      UNREFERENCED_PARAMETER(size_in_bytes);
      kfree(krnl_virt);
# endif // __i386__
#elif defined( __AAL_WINDOWS__ )

      MmFreeNonCachedMemory(krnl_virt, (SIZE_T)size_in_bytes);

#endif // OS

   }

}

//=============================================================================
/// kosal_alloc_contiguous_mem_nocache
/// @brief     Allocate a buffer of contiguous physical pages
/// @param[in] size in bytes
/// @return    pointer to memory. NULL if failure.
/// @note      
//=============================================================================
btVirtAddr _kosal_alloc_contiguous_mem_nocache(__ASSERT_HERE_PROTO btWSSize size_in_bytes)
{
#if   defined( __AAL_LINUX__ )
   btVirtAddr pg;
   btVirtAddr buffer_end;
#elif defined( __AAL_WINDOWS__ )
   PHYSICAL_ADDRESS minaddr = { 0ULL };
   PHYSICAL_ADDRESS maxaddr = { 0xFFFFFFFFFFFFFFFFULL };
   PHYSICAL_ADDRESS alignment;
   maxaddr.LowPart= 0xffffffff;
   maxaddr.HighPart= 0xffffffff;
#endif // __AAL_WINDOWS__

   btVirtAddr krnl_virt = NULL;

   __ASSERT_HERE_IN_FN(size_in_bytes > 0);
   


#if   defined( __AAL_LINUX__ )

   krnl_virt = (btVirtAddr)__get_free_pages(GFP_KERNEL, get_order(size_in_bytes));
   __ASSERT_HERE_IN_FN(NULL != krnl_virt);
   if ( NULL == krnl_virt ) {
      return NULL;
   }
   
   // Set each page as reserved so that the swapper will not page them out.
   buffer_end = krnl_virt + size_in_bytes;
   for ( pg = krnl_virt ; pg < buffer_end ; pg += PAGE_SIZE ) {
      SetPageReserved( virt_to_page((unsigned long)pg) );
   }

#elif defined( __AAL_WINDOWS__ )
   // MmAllocateContiguousMemorySpecifyCache allocates a block of nonpaged memory that is contiguous
   //  in physical address space. The routine maps this block to a contiguous block of virtual memory
   //  in the system address space and returns the virtual address of the base of this block.
   //  The routine aligns the starting address of a contiguous memory allocation to a memory page boundary.
   
   alignment.QuadPart = kosal_round_up_to_page_size(size_in_bytes);
   krnl_virt = (btVirtAddr)MmAllocateContiguousMemorySpecifyCache((SIZE_T)size_in_bytes,
                                                                  minaddr,
                                                                  maxaddr,
                                                                  alignment,
                                                                  MmNonCached);
   __ASSERT_HERE_IN_FN(NULL != krnl_virt);
   if ( NULL == krnl_virt ) {
      return NULL;
   }

#endif // OS

   // Recommended security practice..
   memset(krnl_virt, 0, (size_t)size_in_bytes);

   PMEMORY_HERE("kosal_alloc_contiguous_mem_nocache(size=%llu [0x%llx]) = 0x%" PRIxUINTPTR_T " [phys=0x%" PRIxPHYS_ADDR "]\n",
                   size_in_bytes, size_in_bytes,
                   __UINTPTR_T_CAST(krnl_virt),
                   kosal_virt_to_phys(krnl_virt));

   return krnl_virt;
}

//=============================================================================
/// kosal_free_contiguous_mem
/// @brief     Free buffer of contiguous physical pages allocated through
///            kosal_alloc_contiguous_mem_nocache
/// @param[in] size in bytes
/// @return    pointer to memory. NULL if failure.
/// @note      
//=============================================================================
void _kosal_free_contiguous_mem(__ASSERT_HERE_PROTO btAny krnl_virt, btWSSize size_in_bytes)
{
#if   defined( __AAL_WINDOWS__ )
   UNREFERENCED_PARAMETER(size_in_bytes);
#endif // __AAL_WINDOWS__

   __ASSERT_HERE_IN_FN(NULL != krnl_virt);
   __ASSERT_HERE_IN_FN(size_in_bytes > 0);

   PMEMORY_HERE("kosal_free_contiguous_mem(ptr=0x%" PRIxUINTPTR_T " [phys=0x%" PRIxPHYS_ADDR "], bytes=%llu [0x%llx])\n",
                   __UINTPTR_T_CAST(krnl_virt),
                   kosal_virt_to_phys(krnl_virt),
                   size_in_bytes, size_in_bytes);

   // Recommended security practice..
   if ( NULL != krnl_virt ) {
      memset(krnl_virt, 0, (size_t)size_in_bytes);
   }

#if   defined( __AAL_LINUX__ )

   if ( NULL != krnl_virt ) {
      // Clear the reserved bit.
      btVirtAddr pg;
      btVirtAddr buffer_end = krnl_virt + size_in_bytes;

      for ( pg = krnl_virt ; pg < buffer_end ; pg += PAGE_SIZE ) {
         ClearPageReserved( virt_to_page((unsigned long)pg) );
      }

      free_pages((unsigned long)krnl_virt, get_order(size_in_bytes));
   }

#elif defined( __AAL_WINDOWS__ )

   if ( NULL != krnl_virt ) {
      MmFreeContiguousMemory(krnl_virt);
   }

#endif // OS
}


#if   defined( __AAL_LINUX__ )

void task_poller(struct work_struct *work)
{
   struct delayed_work *delayedWork = container_of(work, struct delayed_work, work);
   pwork_object         pwork       = container_of(delayedWork, work_object, workobj);

   pwork->callback(pwork->context,NULL);

}

#elif defined( __AAL_WINDOWS__ )

void WorkItemCallback(IN PDEVICE_OBJECT pdevObject, IN PVOID Context) {
   pwork_object pwork = (pwork_object)Context;
   UNREFERENCED_PARAMETER(pdevObject);
   kosal_mdelay(pwork->msec_delay);
   pwork->fnct(pwork->context);
   return;
}

void kosal_queue_delayed_work(kosal_work_queue wq, pwork_object pwo, btTime msec)
{
   pwo->msec_delay = msec;
   IoQueueWorkItem(wq, WorkItemCallback, DelayedWorkQueue, pwo);
}

void kosal_wake_up_interruptible(kosal_poll_object *pwaitq)
{
   NTSTATUS Status;
   if ( NULL != *pwaitq ) {
      // Check to see if the IRP was canceled already   
      Status = WdfRequestUnmarkCancelable(*pwaitq);
      if ( Status != STATUS_CANCELLED ) {
         // Complete normally
         WdfRequestComplete(*pwaitq, STATUS_SUCCESS);
      }
   }
   kosal_poll_object_consume(pwaitq);
}

#endif // __AAL_WINDOWS__

btVirtAddr _kosal_get_user_buffer( __ASSERT_HERE_PROTO btVirtAddr user_prt, btWSSize size_in_bytes)
{
#if   defined( __AAL_LINUX__ )
   unsigned long ret;
   btVirtAddr pkbuffer = vmalloc(size_in_bytes);
   if(NULL== pkbuffer){
      return NULL;
   }

   memset(pkbuffer,0,size_in_bytes);
   ret = copy_from_user(pkbuffer, user_prt, size_in_bytes);
   if(ret != 0){
      vfree(pkbuffer);
      return NULL;
   }
   return pkbuffer;
#endif
}

void _kosal_free_user_buffer(__ASSERT_HERE_PROTO btVirtAddr user_prt,  btWSSize size_in_bytes)
{
#if   defined( __AAL_LINUX__ )
   vfree(user_prt);
#endif
}




