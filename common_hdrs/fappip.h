//******************************************************************************
// Part of the Intel(R) QuickAssist Technology Accelerator Abstraction Layer
//
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2007-2016, Intel Corporation.
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
//  Copyright(c) 2007-2016, Intel Corporation.
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
//        FILE: fappip.h module interface
//     CREATED: Q1 2007
//      AUTHOR: Michael Liao, Joseph Grecco and Alvin Chen
//
// PURPOSE: Defines the module interface
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 11-14-07       JG       Converted to FAPDRV
// 12-03/07       JG       Added UNBLOCK ioctl
// 10/22/08       JG       Converted to AAL 1.0
// 11/11/2008     JG       Added legal header
// 11/25/2008     HM       Large merge
// 12/16/2008     JG       Began support for abort and shutdown
//                            Added Support for WSID object
//                            Major interface changes.
// 12/20/2008     HM       Added structures to support CSR Get/Set, and Multi-
//                            Descriptor Task Submit
// 01/03/2009     HM       Moved SOT/MOT/EOT definitions to AALWorkspace.h
// 01/04/2009     HM       Updated Copyright
// 03/17/2009     JG       Moved AFU CSR out and removed old IOCTL defs
// 03/17/2009     AC       Change pointer to array in submit_descriptor_req
// 12/27/2009     JG       Added reserved WSIDs for CSR maps
// 09/18/2010     HM       Formatting and comment cleanup
// 03/05/2012     JG       Added SPL2 support
//****************************************************************************
#ifndef __AALSDK_KERNEL_FAPPIP_H__
#define __AALSDK_KERNEL_FAPPIP_H__
#include <aalsdk/kernel/aalui.h>
#include <aalsdk/kernel/AALWorkspace.h>

BEGIN_NAMESPACE(AAL)

BEGIN_C_DECLS


///////////////////////////////////////////////////////////////////////////////////////
// API IIDs TODO should come from aal ids
// The type is btID, which is the same as btID         // HM 20081201
#define AAL_AHMPIP_IID_1_0        (0xff00000000000010LL)
#define AAL_AFUPIP_IID_1_0        (0xff00000000000110LL)

#define AAL_AHMPIP_IID_0_7        (0xff00000000000007LL)
#define AAL_ASMPIP_IID_1_0        (0xff00000000001000LL)

// Message interfaces
#define AAL_AFUAPI_IID_1_0        (0xfa00000000000010LL)
#define AAL_AHMAPI_IID_1_0        (0xfa00000000000011LL)
#define AAL_ASMAPI_IID_1_0        (0xfa00000000001001LL)

#define AAL_AHMAPI_IID_0_7        (0xfa00000000000007LL)




typedef enum
{
    fappip_afucmdWKSP_ALLOC=1,
    fappip_afucmdWKSP_VALLOC,
    fappip_afucmdWKSP_FREE,
    fappip_afucmdWKSP_VFREE,
    fappip_afucmdWKSP_GET_PHYS,

    fappip_afucmdDESC_SUBMIT,
    fappip_afucmdDESC_QUERY,
    fappip_afucmdTASK_ABORT,
    fappip_afucmdSTART_SPL2_TRANSACTION,
    fappip_afucmdSET_SPL2_CONTEXT_WORKSPACE,
    fappip_afucmdCSR_GETSET,
    fappip_afucmdDESC_MULTISUBMIT,
    fappip_afucmdSTOP_SPL2_TRANSACTION,
    fappip_getCSRmap,
    fappip_getMMIORmap,
    fappip_getuMSGmap

} fappip_afuCmdID_e;


// Legacy defines deprecating
#define AHM_WKSP_RESET          fappip_afucmdWKSP_RESET
#define AHM_WKSP_ALLOC          fappip_afucmdWKSP_ALLOC
#define AHM_WKSP_VALLOC         fappip_afucmdWKSP_VALLOC
#define AHM_WKSP_FREE           fappip_afucmdWKSP_FREE
#define AHM_WKSP_VFREE          fappip_afucmdWKSP_VFREE
#define AHM_WKSP_GET_PHYS       fappip_afucmdWKSP_GET_PHYS

#define AHM_DESC_SUBMIT         fappip_afucmdDESC_SUBMIT
#define AHM_DESC_QUERY          fappip_afucmdDESC_QUERY
#define AHM_TASK_ABORT          fappip_afucmdTASK_ABORT
#define AHM_DESC_FREE           AHM_DESC_ABORT
#define AHM_TASK_UNBLOCK        fappip_afucmdTASK_UNBLOCK

#define AHM_CSR_GETSET          fappip_afucmdCSR_GETSET
#define AHM_DESC_MULTISUBMIT    fappip_afucmdDESC_MULTISUBMIT

///////////////////////////////////////////////////////////////////////////////////////
enum
{
   ACPM_HWTASK_STATUS_IDLE = 0,
   ACPM_HWTASK_STATUS_RUNNING,
   ACPM_HWTASK_STATUS_ERROR,
   ACPM_HWTASK_STATUS_DONE
};


struct ahm_req
{
   union {
      // mem_alloc
      struct {
         btWSID   m_wsid;     // IN
         btWSSize m_size;     // IN
         btWSSize m_pgsize;
      } wksp;

// Special workspace IDs for CSR Aperture mapping
// XXX These must match aaldevice.h:AAL_DEV_APIMAP_CSR*
#define WSID_CSRMAP_READAREA  0x00000001
#define WSID_CSRMAP_WRITEAREA 0x00000002
#define WSID_MAP_MMIOR        0x00000003
#define WSID_MAP_UMSG         0x00000004
      // mem_get_cookie
      struct {
         btWSID             m_wsid;   /* IN  */
         btUnsigned64bitInt m_cookie; /* OUT */
      } wksp_cookie;

      struct {
         btVirtAddr         vaddr; /* IN   */
         btWSSize           size;   /* IN   */
         btUnsigned64bitInt mem_id; /* OUT  */
      } mem_uv2id;
   } u;
};

struct spl2req
{
   struct ahm_req    ahmreq;
   stTransactionID_t afutskTranID;
   btTime            pollrate;
};

// Alias for now
#define cciv4req spl2req

/*
 * Used to submit a group of descriptors that constitute a single task.
 */
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct desc_share_info
{
   btVirtAddr         m_uvptr;     /* In: User Virtual pointer to buffer inside wsid */
   btVirtAddr         m_context;   /* In: Application context */
   btWSID             m_wsid;      /* In: the workspace id. Should use buffer address to compute it. */
   btWSID             m_vwsid;     /* In: the workspace for the virtual table. Should use buffer address to compute it.*/
   TDESC_TYPE         m_type;      // IN    IN_DESC: '0'   OUT_DESC: '1'
   btWSSize           m_len;       /* In: length of the buffer in bytes, currently unchecked. Maybe just make it cachelines? */
   btUnsigned16bitInt m_no_notify; /* In: flag indicating whether an event should be sent on completion. 1 = no event */
   btUnsigned16bitInt m_delim;     // In: SOT, MOT, EOT
} desc_share_info_t;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct submit_descriptors_req
{
   TTASK_MODE               m_mode;            /* Slave, PMaster, VMaster. Will be repeated in each descriptor */
   struct stTransactionID_t m_tranID;          /* Transaction ID for entire task */
   btUnsigned16bitInt       m_nDesc;           /* number of descriptors in the array */
   desc_share_info_t        m_arrDescInfo[1];  /* all descriptors, output first, in correct submission order*/
} submit_descriptors_req_t;


END_C_DECLS

END_NAMESPACE(AAL)

#endif // __AALSDK_KERNEL_FAPPIP_H__

