//******************************************************************************
// Part of the Intel(R) QuickAssist Technology Accelerator Abstraction Layer
//
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2007-2014, Intel Corporation.
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
//  Copyright(c) 2007-2014, Intel Corporation.
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
//        FILE: spl2diag.h
//     CREATED: Q1 2008
//      AUTHOR: Alvin Chen,    Intel Corporation
//              Joseph Grecco, Intel Corporation
//              Henry Mitchel, Intel Corporation
//              Tim Whisonant, Intel Corporation
//
// PURPOSE: Diagnostic Driver Interface
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 05/11/2008     HM       Comments & License
// 06/07/2011     TSW      Misc. cleanup / Added user-mapped CSRs.
// 11/19/2011     HM       Modified for CCI/SPL2
//****************************************************************************
#ifndef __AALKERNEL_SPL2_PIP_DKSM_SPL2DIAG_H__
#define __AALKERNEL_SPL2_PIP_DKSM_SPL2DIAG_H__
#include "aalsdk/kernel/kosal.h"

#define SPL2_DIAG_DEV_NAME "spl2diag"
#define SPL2_DIAG_DEV_PATH "/dev/" SPL2_DIAG_DEV_NAME


#define DIAG_CSR_SIZE 32
#if (DIAG_CSR_SIZE == 64)
typedef btUnsigned64bitInt spl2_csr_t;
# define PRIuCSR PRIu64
# define PRIxCSR PRIx64
# define PRIXCSR PRIX64
# define __CSR_C(c)    __UINT64_T_CONST(c)
# define __CSR_CAST(c) ( (btUnsigned64bitInt)(c) )
#else // 32-bit CSRs
typedef btUnsigned32bitInt spl2_csr_t;
# define PRIuCSR "u"
# define PRIxCSR "x"
# define PRIXCSR "X"
# define __CSR_C(c)    c##U
# define __CSR_CAST(c) ( (btUnsigned32bitInt)(c) )
#endif // DIAG_CSR_SIZE


BEGIN_C_DECLS

////////////////////////////////////////////////////////////////////////////////

typedef struct _spl2_diag_print_req // SPL2_DIAG_PRINT
{
   int         len;    // IN : count of bytes of input buffer to print.
} spl2_diag_print_req;

typedef struct _spl2_diag_mem_req   // SPL2_DIAG_ALLOC, SPL2_DIAG_DEALLOC
{
   int         kind;   // IN  : INPUT_BUFFER, OUTPUT_BUFFER, or GCSR.
   btPhysAddr  base;   // OUT : (SPL2_DIAG_ALLOC) phys addr of allocation.
   size_t      size;   // IN  : (SPL2_DIAG_ALLOC) size requested.
} spl2_diag_mem_req;

typedef struct _spl2_diag_csr_req   // SPL2_DIAG_CSR
{
   int         kind;   // IN : CSR_READ, CSR_WRITE, or CSR_QUERY.
   union {
      struct { // CSR_READ, CSR_WRITE
         spl2_csr_t   offset; // IN : register offset.
         spl2_csr_t   data;   // OUT(CSR_READ)/IN(CSR_WRITE) : value read/value to write.
      } rw;
      struct { // CSR_QUERY
         btPhysAddr  csr_phys; // OUT : phys addr of CSR aperture.
         size_t      csr_size; // OUT : size of CSR aperture.
      } query;
   };
} spl2_diag_csr_req;

struct spl2_diag_req {
   union {
      spl2_diag_print_req print;
      spl2_diag_mem_req   mem;
      spl2_diag_csr_req   csr;
   };
};

////////////////////////////////////////////////////////////////////////////////
// IOCTL CMDs
////////////////////////////////////////////////////////////////////////////////
#define SPL2_DIAG_PRINT   _IOWR ('d', 0x00, struct spl2_diag_req)
#define SPL2_DIAG_ALLOC   _IOWR ('d', 0x01, struct spl2_diag_req)
#define SPL2_DIAG_DEALLOC _IOWR ('d', 0x02, struct spl2_diag_req)

#define INPUT_BUFFER  0
#define OUTPUT_BUFFER 1
#define GCSR          2

#define SPL2_DIAG_CSR     _IOWR ('d', 0x03, struct spl2_diag_req)
#define CSR_READ      0
#define CSR_WRITE     1
#define CSR_QUERY     2

////////////////////////////////////////////////////////////////////////////////

END_C_DECLS

#endif // __AALKERNEL_SPL2_PIP_DKSM_SPL2DIAG_H__

