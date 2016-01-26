// Copyright(c) 2015-2016, Intel Corporation
//
// Redistribution  and  use  in source  and  binary  forms,  with  or  without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of  source code  must retain the  above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name  of Intel Corporation  nor the names of its contributors
//   may be used to  endorse or promote  products derived  from this  software
//   without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  BUT NOT LIMITED TO,  THE
// IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT  SHALL THE COPYRIGHT OWNER  OR CONTRIBUTORS BE
// LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR
// CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT LIMITED  TO,  PROCUREMENT  OF
// SUBSTITUTE GOODS OR SERVICES;  LOSS OF USE,  DATA, OR PROFITS;  OR BUSINESS
// INTERRUPTION)  HOWEVER CAUSED  AND ON ANY THEORY  OF LIABILITY,  WHETHER IN
// CONTRACT,  STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE  OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,  EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//****************************************************************************
/// @file VTPService.cpp
/// @brief Implementation of IVTPService.
/// @ingroup vtp_service
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
/// Virtual-to-Physical address translation service
///
/// TODO: add verbose description
///
/// Provides service for access to the VTP BBB for address translation.
/// Assumes a VTP BBB DFH to be detected and present.
///
/// On initialization, allocates shared buffer for VTP page hash and
/// communicates its location to VTP BBB.
///
/// Provides synchronous methods to update page hash on shared buffer
/// allocation.
///
/// Does not have an explicit client callback interface, as all published
/// service methods are synchronous.
///
/// AUTHORS: Enno Luebbers, Intel Corporation
///          Michael Adler, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 01/15/2016     EL       Initial version@endverbatim
//****************************************************************************
#include <aalsdk/AAL.h>
#include <aalsdk/aas/AALServiceModule.h>
#include <aalsdk/osal/Sleep.h>

#include <aalsdk/AALLoggerExtern.h>              // Logger
#include <aalsdk/utils/AALEventUtilities.h>      // Used for UnWrapAndReThrow()

#include <aalsdk/aas/AALInProcServiceFactory.h>  // Defines InProc Service Factory
#include <aalsdk/service/IALIAFU.h>

#include "VTPService-internal.h"


//=============================================================================
// Typedefs and Constants
//=============================================================================

#ifndef VTPSERVICE_VERSION_CURRENT
# define VTPSERVICE_VERSION_CURRENT  5
#endif // VTPSERVICE_VERSION_CURRENT
#ifndef VTPSERVICE_VERSION_REVISION
# define VTPSERVICE_VERSION_REVISION 0
#endif // VTPSERVICE_VERSION_REVISION
#ifndef VTPSERVICE_VERSION_AGE
# define VTPSERVICE_VERSION_AGE      0
#endif // VTPSERVICE_VERSION_AGE
#ifndef VTPSERVICE_VERSION
# define VTPSERVICE_VERSION          "5.0.0"
#endif // VTPSERVICE_VERSION

#if defined ( __AAL_WINDOWS__ )
# ifdef VTP_SERVICE_EXPORTS
#    define VTP_SERVICE_API __declspec(dllexport)
# else
#    define VTP_SERVICE_API __declspec(dllimport)
# endif // VTP_SERVICE_EXPORTS
#else
# ifndef __declspec
#    define __declspec(x)
# endif // __declspec
# define VTP_SERVICE_API    __declspec(0)
#endif // __AAL_WINDOWS__

#define SERVICE_FACTORY AAL::InProcSvcsFact< VTPService >

#if defined ( __AAL_WINDOWS__ )
# pragma warning(push)
# pragma warning(disable : 4996) // destination of copy is unsafe
#endif // __AAL_WINDOWS__

AAL_BEGIN_SVC_MOD(SERVICE_FACTORY, libVTPService, VTP_SERVICE_API, VTPSERVICE_VERSION, VTPSERVICE_VERSION_CURRENT, VTPSERVICE_VERSION_REVISION, VTPSERVICE_VERSION_AGE)
   /* No commands other than default, at the moment. */
AAL_END_SVC_MOD()

#if defined ( __AAL_WINDOWS__ )
# pragma warning(pop)
#endif // __AAL_WINDOWS__


/////////////////////////////////////////////////////////////////////////////
//////                                                                ///////
//////                                                                ///////
/////                            VTP Service                           //////
//////                                                                ///////
//////                                                                ///////
/////////////////////////////////////////////////////////////////////////////


/// @addtogroup vtp_service
/// @{

//=============================================================================
// Name: init()
// Description: Initialize the Service
// Interface: public
// Inputs: pclientBase - Pointer to the IBase for the Service Client
//         optArgs - Arguments passed to the Service
//         rtid - Transaction ID
// Outputs: none.
// Comments: Should only return False in case of severe failure that prevents
//           sending a response or calling initFailed.
//=============================================================================
btBool VTPService::init( IBase *pclientBase,
                              NamedValueSet const &optArgs,
                              TransactionID const &rtid)
{
   btObjectType tmp;

   // check for HWALIAFU's IBase in optargs
   if ( ENamedValuesOK != optArgs.Get(ALIAFU_IBASE, &tmp) ) {
      initFailed(new CExceptionTransactionEvent( NULL,
                                                 rtid,
                                                 errBadParameter,
                                                 reasMissingParameter,
                                                 "No HWALIAFU IBase in optArgs."));
      return true;
   }
   m_pHWALIAFU = reinterpret_cast<IBase *>(tmp);

   // Get IALIBuffer interface to AFU
   m_pALIBuffer = dynamic_ptr<IALIBuffer>(iidALI_BUFF_Service, m_pHWALIAFU);
   ASSERT(NULL != m_pALIBuffer);
   if ( NULL == m_pALIBuffer ) {
      initFailed(new CExceptionTransactionEvent( NULL,
                                                 rtid,
                                                 errBadParameter,
                                                 reasMissingInterface,
                                                 "No IALIBuffer interface in HWALIAFU."));
      m_bIsOK = false;
      return true;
   }

   // check for VTP MMIO base in optargs
   if ( ENamedValuesOK != optArgs.Get(VTP_DFH_BASE, &tmp) ) {
      initFailed(new CExceptionTransactionEvent( NULL,
                                                 rtid,
                                                 errBadParameter,
                                                 reasMissingParameter,
                                                 "No VTP DFH base address in optArgs."));
      return true;
   }
   m_pDFHBaseAddr = reinterpret_cast<btVirtAddr>(tmp);

   ali_errnum_e err;

   // Allocate the page table.  The size of the page table is a function
   // of the PTE index space.
   size_t pt_size = (1LL << CCI_PT_LINE_IDX_BITS) * CL(1);
   err = m_pALIBuffer->bufferAllocate(pt_size, &m_pPageTable);
   ASSERT(err == ali_errnumOK && m_pPageTable);

   // clear table
   // FIXME: might not be necessary
   memset(m_pPageTable, 0, pt_size);

   // FIXME: bufferGetIOVA should take a btVirtAddr to be consistent
   m_PageTablePA = m_pALIBuffer->bufferGetIOVA((unsigned char *)m_pPageTable);

   m_pPageTableEnd = m_pPageTable + pt_size;

   // The page table is hashed.  It begins with lines devoted to the hash
   // table.  The remainder of the buffer is available for overflow lines.
   // Initialize the free pointer of overflow lines, which begins at the
   // end of the hash table.
   m_pPageTableFree = m_pPageTable + (1LL << CCI_PT_VA_IDX_BITS) * CL(1);
   ASSERT(m_pPageTableFree <= m_pPageTableEnd);

   // Tell the hardware the address of the table
   // FIXME: do we want to use HWALIAFU's MMIO interface? In hat case we need
   //        the DFH's offset into the MMIO space...
#if 0
   *(m_pDFHBaseAddr+CCI_MPF_VTP_CSR_PAGE_TABLE_PADDR) = m_pageTablePA / CL(1);
#endif

   initComplete(rtid);
   return true;
}

btBool VTPService::Release(TransactionID const &rTranID, btTime timeout)
{
   // freedom to all buffers!
   // bufferFreeAll();
   return ServiceBase::Release(rTranID, timeout);
}

ali_errnum_e VTPService::bufferAllocate( btWSSize             Length,
                                         btVirtAddr          *pBufferptr,
                                         NamedValueSet const &rInputArgs,
                                         NamedValueSet       &rOutputArgs ) {
   AutoLock(this);

   // FIXME: Input/OUtputArgs are ignored here...
   // FIXME: we can support optArg ALI_MMAP_TARGET_VADDR also for
   //        large VTP mappings (need to add MAP_FIXED to the first mmap
   //        below).

   // Align request to page size
   Length = (Length + pageSize - 1) & pageMask;

   // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
   //
   //  This method of allocating a large virtual workspace is temporary.
   //  When AAL is capable of doing it internally this hack will be
   //  replaced with a simple allocation call.
   //
   // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *


   // Map a region of the requested size.  This will reserve a virtual
   // memory address space.  As pages are allocated they will be
   // mapped into this space.
   //
   // An extra page is added to the request in order to enable alignment
   // of the base address.  Linux is only guaranteed to return 4 KB aligned
   // addresses and we want large page aligned virtual addresses.
   void* va_base;
   size_t va_base_len = Length + pageSize;
   va_base = mmap(NULL, va_base_len,
                  PROT_READ | PROT_WRITE,
                  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
   ASSERT(va_base != MAP_FAILED);
   printf("va_base %p\n", va_base);

   void* va_aligned = (void*)((size_t(va_base) + pageSize - 1) & pageMask);
   printf("va_aligned %p\n", va_aligned);

   // Trim off the unnecessary extra space after alignment
   size_t trim = pageSize - (size_t(va_aligned) - size_t(va_base));
   printf("va_base_len trimmed by 0x%llx to 0x%llx\n", trim, va_base_len - trim);
   ASSERT(mremap(va_base, va_base_len, va_base_len - trim, 0) == va_base);
   va_base_len -= trim;

   // How many page size buffers are needed to satisfy the request?
   size_t n_buffers = Length / pageSize;

   // Buffer mapping will begin at the end of the va_aligned region
   void* va_alloc = (void*)(size_t(va_aligned) + pageSize * (n_buffers - 1));

   // Prepare bufferAllocate's optional argument to mmap() to a specific address
   NamedValueSet *bufAllocArgs = new NamedValueSet();

   // Allocate the buffers
   for (size_t i = 0; i < n_buffers; i++)
   {
      // Shrink the reserved area in order to make a hole in the virtual
      // address space.
      if (va_base_len == pageSize)
      {
         munmap(va_base, va_base_len);
         va_base_len = 0;
      }
      else
      {
         ASSERT(mremap(va_base, va_base_len, va_base_len - pageSize, 0) == va_base);
         va_base_len -= pageSize;
      }

      // set target virtual address for new buffer
      bufAllocArgs->Add(ALI_MMAP_TARGET_VADDR, va_alloc);

      // Get a page size buffer
      void *buffer;
      ali_errnum_e err = m_pALIBuffer->bufferAllocate(pageSize, (btVirtAddr*)&buffer, *bufAllocArgs);
      ASSERT(err == ali_errnumOK && buffer != NULL);

      // If we didn't get the mapping on our bufferAllocate(), move the shared
      // buffer's VA to the proper slot
      // This should not happen, as we requested the proper VA above.
      // TODO: remove
      ASSERT(buffer == va_alloc);
      if (buffer != va_alloc)
      {
         printf("remap %p to %p\n", (void*)buffer, va_alloc);
         ASSERT(mremap((void*)buffer, pageSize, pageSize,
                        MREMAP_MAYMOVE | MREMAP_FIXED,
                        va_alloc) == va_alloc);
      }

      // Add the mapping to the page table
      InsertPageMapping(va_alloc, m_pALIBuffer->bufferGetIOVA((unsigned char *)buffer));

      // Next VA
      va_alloc = (void*)(size_t(va_alloc) - pageSize);

      ASSERT((m_pALIBuffer->bufferGetIOVA((unsigned char *)buffer) & ~pageMask) == 0);

      // prepare optArgs for next allocation
      bufAllocArgs->Delete(ALI_MMAP_TARGET_VADDR);

   }

   delete bufAllocArgs;

   if (va_base_len != 0)
   {
       munmap(va_base, va_base_len);
   }

   DumpPageTable();

   *pBufferptr = (btVirtAddr)va_aligned;
   return ali_errnumOK;
}

ali_errnum_e VTPService::bufferFreeAll() {
   ERR("NOT IMPLEMENTED");
   return ali_errnumNoMem;
}

ali_errnum_e VTPService::bufferFree( btVirtAddr Address) {
   ERR("NOT IMPLEMENTED");
   return ali_errnumNoMem;
}

btPhysAddr VTPService::bufferGetIOVA(  btVirtAddr     Address) {
   {
       // Get the hash index and VA tag
       uint64_t tag;
       uint64_t idx;
       uint64_t offset;
       AddrComponentsFromVA(Address, tag, idx, offset);

       // The idx field is the hash bucket in which the VA will be found.
       uint8_t* pte = m_pPageTable + idx * CL(1);

       // Search for a matching tag in the hash bucket.  The bucket is a set
       // of vectors PTEs chained in a linked list.
       while (true)
       {
           // Walk through one vector in one line
           for (int n = 0; n < ptesPerLine; n += 1)
           {
               uint64_t va_tag;
               uint64_t pa_idx;
               ReadPTE(pte, va_tag, pa_idx);

               if (va_tag == tag)
               {
                   // Found it!
                   return (pa_idx << CCI_PT_PAGE_OFFSET_BITS) | offset;
               }

               // End of the PTE list?
               if (va_tag == 0)
               {
                   // Failed to find an entry for VA
                   return 0;
               }

               pte += pteBytes;
           }

           // VA not found in current line.  Does this line of PTEs link to
           // another?
           pte = m_pPageTable + ReadTableIdx(pte) * CL(1);

           // End of list?  (Table index was NULL.)
           if (pte == m_pPageTable)
           {
               return 0;
           }
       }
   }
}



//// All taken from cci_mpf_shim_vtp.cpp

void
VTPService::InsertPageMapping(const void* va, btPhysAddr pa)
{
    printf("Map %p at 0x%08lx\n", va, pa);

    //
    // VA components are the offset within the 2MB-aligned page, the index
    // within the direct-mapped page table hash vector and the remaining high
    // address bits: the tag.
    //
    uint64_t va_tag;
    uint64_t va_idx;
    uint64_t va_offset;
    AddrComponentsFromVA(va, va_tag, va_idx, va_offset);
    ASSERT(va_offset == 0);

    //
    // PA components are the offset within the 2MB-aligned page and the
    // index of the 2MB aligned physical page (low bits dropped).
    //
    uint64_t pa_idx;
    uint64_t pa_offset;
    AddrComponentsFromPA(pa, pa_idx, pa_offset);
    ASSERT(pa_offset == 0);

    //
    // The page table is hashed by the VA index.  Compute the address of
    // the line given the hash.
    //
    uint8_t* p = m_pPageTable + va_idx * CL(1);

    //
    // Find a free entry.
    //
    uint32_t n = 0;
    while (true)
    {
        if (n++ != ptesPerLine)
        {
            // Walking PTEs in a line
            uint64_t tmp_va_tag;
            uint64_t tmp_pa_idx;
            ReadPTE(p, tmp_va_tag, tmp_pa_idx);

            if (tmp_va_tag == 0)
            {
                // Found a free entry
                break;
            }

            // Entry was busy.  Move on to the next one.
            p += pteBytes;
        }
        else
        {
            // End of the line.  Is there an overflow line already?
            n = 0;

            uint64_t next_idx = ReadTableIdx(p);
            if (next_idx != 0)
            {
                // Overflow allocated.  Switch to it and keep searching.
                p = m_pPageTable + next_idx * CL(1);
            }
            else
            {
                // Need a new overflow line.  Is there space in the page table?
                ASSERT(m_pPageTableFree < m_pPageTableEnd);

                // Add a next line pointer to the current entry.
                WriteTableIdx(p, (m_pPageTableFree - m_pPageTable) / CL(1));
                p = m_pPageTableFree;
                m_pPageTableFree += CL(1);

                // Write the new PTE at p.
                break;
            }
        }
    }

    // Add the new PTE
    WritePTE(p, va_tag, pa_idx);
}


void
VTPService::ReadPTE(
    const uint8_t* pte,
    uint64_t& vaTag,
    uint64_t& paIdx)
{
    // Might not be a natural size so use memcpy
    uint64_t e = 0;
    memcpy(&e, pte, pteBytes);

    paIdx = e & ((1LL << CCI_PT_PA_IDX_BITS) - 1);

    vaTag = e >> CCI_PT_PA_IDX_BITS;
    vaTag &= (1LL << vaTagBits) - 1;

    // VA is sign extended from its size to 64 bits
    if (CCI_PT_VA_BITS != 64)
    {
        vaTag <<= (64 - vaTagBits);
        vaTag = uint64_t(int64_t(vaTag) >> (64 - vaTagBits));
    }
}


uint64_t
VTPService::ReadTableIdx(
    const uint8_t* p)
{
    // Might not be a natural size
    uint64_t e = 0;
    memcpy(&e, p, (CCI_PT_LINE_IDX_BITS + 7) / 8);

    return e & ((1LL << CCI_PT_LINE_IDX_BITS) - 1);
}


void
VTPService::WritePTE(
    uint8_t* pte,
    uint64_t vaTag,
    uint64_t paIdx)
{
    uint64_t p = AddrToPTE(vaTag, paIdx);

    // Might not be a natural size so use memcpy
    memcpy(pte, &p, pteBytes);
}


void
VTPService::WriteTableIdx(
    uint8_t* p,
    uint64_t idx)
{
    // Might not be a natural size
    memcpy(p, &idx, (CCI_PT_LINE_IDX_BITS + 7) / 8);
}


void
VTPService::DumpPageTable()
{
    printf("Page table dump:\n");
    printf("  %lld lines, %ld PTEs per line, max. memory represented in PTE %lld GB\n",
           1LL << CCI_PT_LINE_IDX_BITS,
           ptesPerLine,
           ((1LL << CCI_PT_LINE_IDX_BITS) * ptesPerLine * 2) / 1024);

    // Loop through all lines in the hash table
    for (int hash_idx = 0; hash_idx < (1LL << CCI_PT_VA_IDX_BITS); hash_idx += 1)
    {
        int cur_idx = hash_idx;
        uint8_t* pte = m_pPageTable + hash_idx * CL(1);

        // Loop over all lines in the hash group
        while (true)
        {
            int n;
            // Loop over all PTEs in a single line
            for (n = 0; n < ptesPerLine; n += 1)
            {
                uint64_t va_tag;
                uint64_t pa_idx;
                ReadPTE(pte, va_tag, pa_idx);

                // End of the PTE list within the current hash group?
                if (va_tag == 0) break;

                //
                // The VA in a PTE is the combination of the tag (stored
                // in the PTE) and the hash table index.  The table index
                // is mapped directly from the low bits of the VA's line
                // address.
                //
                // The PA in a PTE is stored as the index of the 2MB-aligned
                // physical address.
                printf("    %d/%d:\t\tVA 0x%016llx -> PA 0x%016llx\n",
                       hash_idx, cur_idx,
                       (va_tag << (CCI_PT_VA_IDX_BITS + CCI_PT_PAGE_OFFSET_BITS)) |
                       (uint64_t(hash_idx) << CCI_PT_PAGE_OFFSET_BITS),
                       pa_idx << CCI_PT_PAGE_OFFSET_BITS);

                pte += pteBytes;
            }

            // If the PTE list within the current hash group is incomplete then
            // we have walked all PTEs in the line.
            if (n != ptesPerLine) break;

            // Follow the next pointer to the connected line holding another
            // vector of PTEs.
            cur_idx = ReadTableIdx(pte);
            pte = m_pPageTable + cur_idx * CL(1);
            // End of list?  (Table index was NULL.)
            if (pte == m_pPageTable) break;
        }
    }
}


/// @}


