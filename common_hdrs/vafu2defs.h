//******************************************************************************
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2011-2016, Intel Corporation.
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
//  Copyright(c) 2011-2016, Intel Corporation.
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
/// @file vafu2defs.h
/// @brief External definitions for the SPL2 hardware interface.
/// @ingroup AALCore
/// @verbatim
/// Accelerator Abstraction Layer
///
/// AUTHOR: Henry Mitchel, Intel Corporation
///
/// HISTORY:
/// COMMENTS:
/// WHEN:          WHO:     WHAT:
/// 12/05/2011     HM       Initial version started
/// 02/13/2012     JG       AAL integration
/// 04/26/2012     TSW      Add CASSERTs for struct sizes.@endverbatim
//****************************************************************************
#ifndef __AALSDK_KERNEL_VAFU2DEFS_H__
#define __AALSDK_KERNEL_VAFU2DEFS_H__
#include <aalsdk/kernel/aaltypes.h>
#include <aalsdk/kernel/spl2defs.h> // SPL2AFU_DSM

BEGIN_NAMESPACE(AAL)

/// @addtogroup AALCore
/// @{

/*
 * VAFU 2-specific constants based on QLP2 implementation
 *
 * DSM == Device Status Memory, i.e. a place for the device to communicate to the host
 *
 * All 64-bit CSR's are written in 32-bit halves, upper half first. HW takes writing
 *    of lower 32-bit half as signal that both are written and the entire value is valid.
 *    Note that because these CSRs are in the PCIe configuration space, they are serialized,
 *    so that the above assumption is accurate.
 */

/*****************************************************************************
 * CSRs inherited from SPL2 are:
 *    static const int byte_offset_AFU_DSM_BASE     =  0xA00;
 *    static const int byte_offset_AFU_CNTXT_BASE   =  0xA08;
 *****************************************************************************/

/*****************************************************************************
 * 32-bit value provided as a check write/read mechanism
 *****************************************************************************/
static const int byte_offset_AFU_CSR_SCRATCH = 0xAFC;

/*
 * VAFU2_DSM data structure, needs to be aligned on 4K boundary
 */

// VAFU 2 id (fake GUID)
static const btUnsigned64bitInt VAFU2_ID_LO = 0x11100181;
static const btUnsigned64bitInt VAFU2_ID_HI = 0;

/*****************************************************************************
 *****************************************************************************
 * Structure of VAFU 2 Device Status Memory area
 *****************************************************************************
 *****************************************************************************/
struct VAFU2_DSM {
   // cache-line 0                        // AKA AFU_DSM_ID
   struct SPL2AFU_DSM vafu2;              // Inherited from SPL, the AFU_ID definition
                                          // If vafu2.AFU_ID[0] == VAFU2_ID_LO &&
                                          //    vafu2.AFU_ID[1] == VAFU2_ID_HI, then this is VAFU2
   // cache-lines 1-3
   btUnsigned64bitInt           rsvd0[8][3];        // Don't use these

   // cache-line 4
   union {                                // Latency counter
      btUnsigned64bitInt        rsvd1[8];           // Make it a cache-line
      btUnsigned64bitInt        AFU_DSM_LATENCY;    // Clocks?
   };

   // cache-line 5
   union {                                // Performance counter
      btUnsigned64bitInt        rsvd2[8];           // Make it a cache-line
      btUnsigned64bitInt        AFU_DSM_PERFORMANCE;// Number of cycles elapsed between start and end of task
   };

   // cache-lines 6-62
   btUnsigned64bitInt           rsvd3[8][62-6+1];   // Don't use these

   // cache-line 63
   union {                                // Performance counter
      btUnsigned64bitInt        rsvd4[8];           // Make it a cache-line
      btUnsigned32bitInt        AFU_DSM_SCRATCH;    // Value written to byte_offset_AFU_CSR_SCRATCH
   };
}; // struct VAFU2_DSM
CASSERT(sizeof(struct VAFU2_DSM) == (64 * 64));

/// Structure of Validation AFU Context for SPL2
/// @todo Doxygen cannot handle nested anonymous structs/unions.
struct VAFU2_CNTXT {
   union {
      btUnsigned64bitInt          qword0[8];       // make it a whole cacheline
      struct {
         union {                                   // first qword
            btUnsigned64bitInt    dword0;
            struct {
               btUnsigned64bitInt rsvd0:   32;
               btUnsigned64bitInt delay:   16;     // undefined, but in the structure definition, set to 0
            };
         };
         void          *pSource;                   ///< Source user-mode virtual address, cache-line aligned.
         void          *pDest;                     ///< Destination user-mode virtual address, cache-line aligned.
         btUnsigned32bitInt       num_cl;          ///< Number of cache lines to copy.
      };
   };
   union {
      btUnsigned64bitInt          qword1[8];       // make it a whole cacheline
      struct {
         btUnsigned32bitInt       Status;          ///< Bit0 true if done
#define  VAFU2_CNTXT_STATUS_DONE   0x00000001      ///< Bit0 selector
      };
   };
}; // struct VAFU2_CNTXT
//CASSERT(sizeof(struct VAFU2_CNTXT) == 64);       // no longer true

/// @}

END_NAMESPACE(AAL)

#endif // __AALSDK_KERNEL_VAFU2DEFS_H__

