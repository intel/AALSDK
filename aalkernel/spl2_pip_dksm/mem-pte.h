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
//        FILE: mem-pte.h
//     CREATED: 11/03/2011
//      AUTHOR: Henry Mitchel
//
// PURPOSE:  This file contains private memory-structure-specific definitions
//           for the Intel(R) QuickAssist Technology Accelerator Abstraction
//           Layer (AAL) SPL 2 Memory Manager Driver.
//
//           Specifically -- the internals of the pte (Page Table Entry)
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 11/03/2011     HM       Initial version started
//****************************************************************************
#ifndef __AALKERNEL_SPL2_PIP_DKSM_MEM_PTE_H__
#define __AALKERNEL_SPL2_PIP_DKSM_MEM_PTE_H__
#include "aalsdk/kernel/kosal.h"

//=============================================================================
// Name:        memmgr_pte
// Description: Structure that defines a page table entry.
//=============================================================================
struct memmgr_pte {
   union {
      btUnsigned64bitInt    pte;        // Physical address of a super-block, CL aligned
      struct {
         unsigned valid:1;    // First bit defines whether or not it is valid
         unsigned rsvd:5;     // Other bits in first 6 must be 0
      };
      btUnsigned64bitInt    CL[8];      // The structure is a cache-line long
   };
}; // struct memmgr_pte
CASSERT(sizeof(struct memmgr_pte) == 64);

//=============================================================================
// Name:        memmgr_clear_pte
// Description: destruct a pte and make it invalid
//=============================================================================
static inline void memmgr_clear_pte (struct memmgr_pte *ppte)
{
   ASSERT(ppte);
   memset( ppte, 0, sizeof(*ppte));
}  // memmgr_clear_pte

//=============================================================================
// Name:        memmgr_set_pte
// Description: construct a pte from its constituents
//=============================================================================
static inline void memmgr_set_pte (struct memmgr_pte *ppte, btUnsigned64bitInt physaddr)
{
   // geeky, but expect a mutator to know the guts of its data structure
   memmgr_clear_pte( ppte);
   ppte->pte = (physaddr & ~0x3ELLU) | 0x01ULL;
}  // memmgr_set_pte

/*
//=============================================================================
// Name:        set_pte_valid
// Description: given a pointer to a pte, set the valid bit
//=============================================================================
static inline void memmgr_set_pte_valid (struct memmgr_pte *ppte)
{
   ppte->valid = 1;
}  // memmgr_set_pte_valid
*/

#endif // __AALKERNEL_SPL2_PIP_DKSM_MEM_PTE_H__

