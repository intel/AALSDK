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
//****************************************************************************
/// @file VTPSERVICE.h
/// @brief IVTPService.
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
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 01/15/2016     EL       Initial version@endverbatim
//****************************************************************************
#ifndef __VTPSERVICE_H__
#define __VTPSERVICE_H__
#include <aalsdk/AAL.h>
#include <aalsdk/osal/OSServiceModule.h>

#include <aalsdk/service/IALIAFU.h>

using namespace AAL;

//#define VTPSERVICE_SVC_MOD         "libVTPSERVICE" AAL_SVC_MOD_EXT
//#define VTPSERVICE_SVC_ENTRY_POINT "libVTPSERVICE" AAL_SVC_MOD_ENTRY_SUFFIX

#define HWALIAFU_IBASE "HWALIAFUIBase"
#define VTP_DFH_BASE "VTPDFHBase"

// TODO: replace with actual spec'd VTP GUID
#define GUID_BBB_VTP "5BC516B4-CC6B-4ECF-9115-70E1815F0370"

//AAL_DECLARE_SVC_MOD(libVTPSERVICE, VTP_SERVICE_API)


/// @addtogroup vtp_service
/// @{

/// IVTPSERVICE interface ID.
#define iidVTPService __INTC_IID(INTC_sysSampleAFU,0x0042) // FIXME: get real IID

class IVTPService : public IALIBuffer
{
public:
/*   /// @brief Allocate shared buffer.
   ///
   /// Allocates a shared buffer of specified length. Updates the VTP page
   /// hash to add the respective virtual-to-physical mapping.
   ///
   /// Synchronous, no callback.
   ///
   /// @param[in]  Length     Length of shared buffer to allocate.
   /// @param[in]  pBufferptr Address of buffer to store virtual address in.
   /// @param[in]  pOptArgs   Pointer to NVS containing optional arguments.
   ///                        Ignored for now.
   /// @return     ali_errnumOK on success, ali_errnumSystem on failure.
   virtual ali_errnum_e bufferAllocate( btWSSize Length,
                                btVirtAddr    *pBufferptr,
                                NamedValueSet *pOptArgs) = 0;


   /// @brief Free shared buffer.
   ///
   /// Frees a previously allocated shared buffer.
   ///
   /// NOT IMPLEMENTED
   ///
   /// Synchronous, no callback.
   ///
   /// @param[in]  Address    Address of buffer to deallocate/free.
   /// @return     ali_errnumOK on success, ali_errnumSystem on failure.
   virtual ali_errnum_e bufferFree( btVirtAddr Address) = 0;
*/

   /// @brief Free all shared buffers.
   ///
   /// Frees all previously allocated shared buffer.
   /// Also flushes VTP TLB.
   ///
   /// NOT IMPLEMENTED
   ///
   /// Synchronous, no callback.
   ///
   /// @return     ali_errnumOK on success, ali_errnumSystem on failure.
   virtual ali_errnum_e bufferFreeAll() = 0;

/*
   /// @brief Get virtual-to-physical mapping
   ///
   /// Returns physical address mapped to a given virtual address inside
   /// a previously allocated buffer.
   ///
   /// Synchronous, no callback.
   ///
   /// @return     ali_errnumOK on success, ali_errnumSystem on failure.
   virtual btPhysAddr bufferGetIOVA( btVirtAddr Address) = 0;
*/

   /// IVTPService Destructor
   virtual ~IVTPService() {}
};

/// @}

// Convenience macros for printing messages and errors.
#ifndef MSG
# define MSG(x) std::cout << __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() : " << x << std::endl
#endif // MSG
#ifndef ERR
# define ERR(x) std::cerr << __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() **Error : " << x << std::endl
#endif // ERR

#endif // __VTPSERVICE_H__

