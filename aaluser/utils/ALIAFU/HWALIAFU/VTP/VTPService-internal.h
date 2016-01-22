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
/// @file VTPService-internal.h
/// @brief Definitions for VTP Service.
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
#ifndef __VTPSERVICE_INT_H__
#define __VTPSERVICE_INT_H__
#include <aalsdk/service/VTPService.h> // Public VTP service interface
#include <aalsdk/aas/AALService.h>
#include <aalsdk/IServiceClient.h>
#include <aalsdk/osal/IDispatchable.h>

#include <aalsdk/service/IALIAFU.h>
#include <aalsdk/uaia/IAFUProxy.h>

#include "cci_mpf_shim_vtp_params.h"

using namespace AAL;

/// @addtogroup vtp_service
/// @{

//=============================================================================
// Name: VTPService
// Description: Virtual-To-Physical address translation service
// Interface: IVTPService
// Comments:
//=============================================================================
/// @brief Virtual-To-Physical address translation service.
class VTPService : public ServiceBase, public IVTPService
{
public:

   /// VTP service constructor
   DECLARE_AAL_SERVICE_CONSTRUCTOR(VTPService, ServiceBase),
      m_pSvcClient(NULL),
      m_pHWALIAFU(NULL),
      m_pDFHBaseAddr(NULL)
   {
      SetInterface(iidVTPService, dynamic_cast<IVTPService *>(this));
   }
   /// @brief Service initialization hook.
   ///
   /// Expects the following in the optArgs passed to it:
   ///
   ///   HWALIAFU_IBASE       pointer to HWALIAFU
   ///   VTP_DFH_BASE         pointer to MMIO space where VTP DFH resides
   btBool init( IBase *pclientBase,
                NamedValueSet const &optArgs,
                TransactionID const &rtid);

   /// Called when the service is released
   btBool Release(TransactionID const &rTranID, btTime timeout=AAL_INFINITE_WAIT);

// <IVTPService>
   ali_errnum_e bufferAllocate( btWSSize       Length,
                                btVirtAddr    *pBufferptr,
                                NamedValueSet *pOptArgs);
   ali_errnum_e bufferFree(     btVirtAddr     Address);
   ali_errnum_e bufferFreeAll();
   btPhysAddr   bufferGetIOVA(  btVirtAddr     Address);
// </IVTPService>


protected:
   IServiceClient        *m_pSvcClient;
   IBase                 *m_pHWALIAFU;
   IALIBuffer            *m_pALIBuffer;
   btVirtAddr             m_pDFHBaseAddr;

   uint8_t               *m_pPageTable;
   btPhysAddr             m_PageTablePA;
   uint8_t               *m_pPageTableEnd;
   uint8_t               *m_pPageTableFree;

private:
  //
  // Add a new page to the table.
  //
  void InsertPageMapping(const void* va, btPhysAddr pa);

  //
  // Convert addresses to their component bit ranges
  //
  inline void AddrComponentsFromVA(uint64_t va,
                                   uint64_t& tag,
                                   uint64_t& idx,
                                   uint64_t& byteOffset);

  inline void AddrComponentsFromVA(const void* va,
                                   uint64_t& tag,
                                   uint64_t& idx,
                                   uint64_t& byteOffset);

  inline void AddrComponentsFromPA(uint64_t pa,
                                   uint64_t& idx,
                                   uint64_t& byteOffset);

  //
  // Construct a PTE from a virtual/physical address pair.
  //
  inline uint64_t AddrToPTE(uint64_t va, uint64_t pa);
  inline uint64_t AddrToPTE(const void* va, uint64_t pa);

  //
  // Read a PTE or table index currently in the table.
  //
  void ReadPTE(const uint8_t* pte, uint64_t& vaTag, uint64_t& paIdx);
  uint64_t ReadTableIdx(const uint8_t* p);

  //
  // Read a PTE or table index to the table.
  //
  void WritePTE(uint8_t* pte, uint64_t vaTag, uint64_t paIdx);
  void WriteTableIdx(uint8_t* p, uint64_t idx);

  // Dump the page table (debugging)
  void DumpPageTable();

   const size_t pageSize = MB(2);
   const size_t pageMask = ~(pageSize - 1);

   // Number of tag bits for a VA.  Tags are the VA bits not covered by
   // the page offset and the hash table index.
   const uint32_t vaTagBits = CCI_PT_VA_BITS -
                              CCI_PT_VA_IDX_BITS -
                              CCI_PT_PAGE_OFFSET_BITS;

   // Size of a single PTE.  PTE is a tuple: VA tag and PA page index.
   // The size is rounded up to a multiple of bytes.
   const uint32_t pteBytes = (vaTagBits + CCI_PT_PA_IDX_BITS + 7) / 8;

   // Size of a page table pointer rounded up to a multiple of bytes
   const uint32_t ptIdxBytes = (CCI_PT_PA_IDX_BITS + 7) / 8;

   // Number of PTEs that fit in a line.  A line is the basic entry in
   // the hash table.  It holds as many PTEs as fit and ends with a pointer
   // to the next line, where the list of PTEs continues.
   const uint32_t ptesPerLine = (CL(1) - ptIdxBytes) / pteBytes;

};



inline void
VTPService::AddrComponentsFromVA(
    uint64_t va,
    uint64_t& tag,
    uint64_t& idx,
    uint64_t& byteOffset)
{
    uint64_t v = va;

    byteOffset = v & ((1LL << CCI_PT_PAGE_OFFSET_BITS) - 1);
    v >>= CCI_PT_PAGE_OFFSET_BITS;

    idx = v & ((1LL << CCI_PT_VA_IDX_BITS) - 1);
    v >>= CCI_PT_VA_IDX_BITS;

    tag = v & ((1LL << vaTagBits) - 1);

    // Make sure no address bits were lost in the conversion.  The high bits
    // beyond CCI_PT_VA_BITS are sign extended.
    if (CCI_PT_VA_BITS != 64)
    {
        int64_t va_check = va;
        // Shift all but the high bit of the VA range to the right.  All the
        // resulting bits must match.
        va_check >>= (CCI_PT_VA_BITS - 1);
        ASSERT((va_check == 0) || (va_check == -1));
    }
}


inline void
VTPService::AddrComponentsFromVA(
    const void *va,
    uint64_t& tag,
    uint64_t& idx,
    uint64_t& byteOffset)
{
    AddrComponentsFromVA(uint64_t(va), tag, idx, byteOffset);
}

inline void
VTPService::AddrComponentsFromPA(
    uint64_t pa,
    uint64_t& idx,
    uint64_t& byteOffset)
{
    uint64_t p = pa;

    byteOffset = p & ((1LL << CCI_PT_PAGE_OFFSET_BITS) - 1);
    p >>= CCI_PT_PAGE_OFFSET_BITS;

    idx = p & ((1LL << CCI_PT_PA_IDX_BITS) - 1);
    p >>= CCI_PT_PA_IDX_BITS;

    // PA_IDX_BITS must be large enough to represent all physical pages
    ASSERT(p == 0);
}

inline uint64_t
VTPService::AddrToPTE(
    uint64_t va,
    uint64_t pa)
{
    ASSERT((pa & ~((1LL << CCI_PT_PA_IDX_BITS) - 1)) == 0);

    return ((va << CCI_PT_PA_IDX_BITS) | pa);
}

inline uint64_t
VTPService::AddrToPTE(
    const void* va,
    uint64_t pa)
{
    return AddrToPTE(uint64_t(va), pa);
}




/// @}

#endif //__SAMPLEAFU1SERVICE_INT_H__

