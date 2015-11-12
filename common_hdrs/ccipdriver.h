//******************************************************************************
// Part of the Intel(R) QuickAssist Technology Accelerator Abstraction Layer
//
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2015, Intel Corporation.
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
//  Copyright(c) 2015, Intel Corporation.
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
//        FILE: ccipdriver.h
//     CREATED: Nov. 2, 2015
//      AUTHOR: Joseph Grecco, Intel  <joe.grecco@intel.com>
//
// PURPOSE: Definitions for the CCIP device driver
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 11/02/15       JG       Initial version.
//****************************************************************************
#ifndef __AALSDK_CCIP_DRIVER_H__
#define __AALSDK_CCIP_DRIVER_H__
#include <aalsdk/kernel/AALWorkspace.h>
#include <aalsdk/kernel/AALTransactionID_s.h>

BEGIN_NAMESPACE(AAL)

BEGIN_C_DECLS

///=================================================================
/// IDs used by the devices and objects
///=================================================================

// FPGA Management Engine GUID
#define CCIP_FME_GUIDL              (0x82FE38F0F9E17764ULL)
#define CCIP_FME_GUIDH              (0xBFAf2AE94A5246E3ULL)
#define CCIP_FME_PIPIID             (0x4DDEA2705E7344D1ULL)

#define CCIP_DEV_FME_SUBDEV         0
#define CCIP_DEV_PORT_SUBDEV(s)     (s + 0x10)
#define CCIP_DEV_AFU_SUBDEV(s)      (s + 0x20)

#define PORT_SUBDEV(s)              (s - 0x10 )
#define AFU_SUBDEV(s)               (s - 0x20 )

/// FPGA Port GUID
#define CCIP_PORT_GUIDL             (0x9642B06C6B355B87ULL)
#define CCIP_PORT_GUIDH             (0x3AB49893138D42EBULL)
#define CCIP_PORT_PIPIID            (0x5E82B04A50E59F20ULL)

/// AFU GUID
#define CCIP_AFU_PIPIID             (0x26F67D4CAD054DFCULL)

/// Signal Tap GUID
#define CCIP_STAP_GUIDL             (0xB6B03A385883AB8DULL)
#define CCIP_STAP_GUIDH             (0x022F85B12CC24C9DULL)
#define CCIP_STAP_PIPIID            (0xA710C842F06E45E0ULL)

/// Partial Reconfiguration GUID
#define CCIP_PR_GUIDL               (0x83B54FD5E5216870ULL)
#define CCIP_PR_GUIDH               (0xA3AAB28579A04572ULL)
#define CCIP_PR_PIPIID              (0x7C4D41EA156C4D81ULL)


/// Vender ID and Device ID
#define CCIP_FPGA_VENDER_ID         0x8086

/// PCI Device ID
#define PCIe_DEVICE_ID_RCiEP0       0xBCBD
#define PCIe_DEVICE_ID_RCiEP1       0xBCBE

/// QPI Device ID
#define PCIe_DEVICE_ID_RCiEP2       0xBCBC

/// MMIO Space map
#define FME_DFH_AFUIDL  0x8
#define FME_DFH_AFUIDH  0x10
#define FME_DFH_NEXTAFU 0x18


typedef enum
{
    ccipdrv_afucmdWKSP_ALLOC=1,
    ccipdrv_afucmdWKSP_FREE,

    ccipdrv_getMMIORmap,
    ccipdrv_getFeatureRegion
} ccipdrv_afuCmdID_e;

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

struct ccidrvreq
{
   struct ahm_req    ahmreq;
   stTransactionID_t afutskTranID;
   btTime            pollrate;
};


END_C_DECLS

END_NAMESPACE(AAL)

#endif // __AALSDK_CCIP_DRIVER_H__

