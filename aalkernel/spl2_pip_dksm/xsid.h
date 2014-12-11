//******************************************************************************
// Part of the Intel(R) QuickAssist Technology Accelerator Abstraction Layer
//
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2011-2014, Intel Corporation.
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
//  Copyright(c) 2011-2014, Intel Corporation.
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
//        FILE: xsid.h
//     CREATED: 11/12/2011
//      AUTHOR: Henry Mitchel
//
// PURPOSE:  This file contains private memory-structure-specific definitions
//           for the Intel(R) QuickAssist Technology Accelerator Abstraction
//           Layer (AAL) SPL 2 Memory Manager Driver.
//
//           Specifically -- workspace identifier conversion routines.
//
//           WS_ID is externally an opaque cookie. Internally it consists of
//           an index into a buf_desc array, and flags specifying WHICH
//           array.
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 11/12/2011     HM       Initial version started
//****************************************************************************
#ifndef __AALKERNEL_SPL2_PIP_DKSM_XSID_H__
#define __AALKERNEL_SPL2_PIP_DKSM_XSID_H__
#include "aalsdk/kernel/kosal.h"

#include "kernver-utils.h"
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27))
   #include <asm/page_types.h>   // PAGE_SHIFT
#else
   #include <asm/page.h>         // PAGE_SHIFT
#endif

#include "spl2mem.h"          // for xsid_t typedef
#include "buf-mem-type.h"     // enum mem_type

#define SPL2_AFUDSM_XSID ((xsid_t)-1)

/* Structure of a xsid
 *
 * It needs to contain an index into an array, typically 32-bit signed. Signed
 *    to allow for signaling a bad index. 32 bits because that is way more than
 *    enough. 0 is a valid index. -1 (== INVALID_ENTRY) is not.
 *
 * It also needs to specify which buf_desc_table it resides in. This is done
 *    by the enum mem_type.
 *
 * It is nice to not allow a pure 0 value to be valid. This is done via forcing
 *    the enum to never be 0. Thus, any xsid that is all zero is in error and
 *    was probably never initialized.
 *
 * The entire working structure of the xsid fits into 32-bits if the buf_desc
 *    array length is <= 16K, or 32 GB @ 2 MB/buffer for virtual mode.
 *    To avoid weird problems, use 32-bit for the index, for generally
 *    things will work (up to 16K entries) in a potential 32-bit world
 */

union xsid_union {
   xsid_t                xsid;                       // regular external xsid definition (btWSID)
   struct {
      btUnsigned64bitInt reserved_page :PAGE_SHIFT;  // Low 12 bits Always 0
      enum mem_type type               :2;           // Specify type of buffer
      btUnsigned64bitInt rsvd_2        :2;           // Bring it up to 16 bits
      btUnsigned64bitInt u_index       :32;          // unsigned 16-bit index -- has to be
                                                     //    separately made signed
                                                     //    Index 0 is legal.
      btUnsigned64bitInt rsvd32        :16;          // Rest of the 64-bits
   };
};

//
static inline xsid_t xsid_from_vm_pgoff( unsigned long vm_pgoff)
{
   union xsid_union wis;
   wis.xsid = vm_pgoff << PAGE_SHIFT;
   return wis.xsid;
}  // xsid_to_index

static inline int index_from_xsid( xsid_t xsid)
{
   union xsid_union wis;
   wis.xsid = xsid;
   return (int)wis.u_index;
}  // xsid_to_index

static inline enum mem_type memtype_of_xsid( xsid_t xsid)
{
   union xsid_union wis;
   wis.xsid = xsid;
   return wis.type;
}  // xsid_to_index

static inline xsid_t xsid_ctor( int index, enum mem_type type)
{
   union xsid_union wis;
   wis.xsid    = 0;                   // init everything to 0
   wis.u_index = (btUnsigned32bitInt)index;
   wis.type    = type;
   return wis.xsid;
} // xsid_ctor


#endif // __AALKERNEL_SPL2_PIP_DKSM_XSID_H__

