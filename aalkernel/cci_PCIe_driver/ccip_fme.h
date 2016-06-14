//******************************************************************************
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2015-2016, Intel Corporation.
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
//  Copyright(c) 2015-2016, Intel Corporation.
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
/// @file ccip_def.h
/// @brief  Definitions for ccip.
/// @ingroup aalkernel_ccip
/// @verbatim
//        FILE: ccip_fme_mmio.h
//     CREATED: Sept 24, 2015
//      AUTHOR: Ananda Ravuri, Intel <ananda.ravuri@intel.com>
//              Joseph Grecco, Intel <joe.grecco@intel.com>
//
// PURPOSE:   This file contains the defintions of the CCIP FME
//             Device Feature List and CSR.
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
//****************************************************************************///
#ifndef __AALKERNEL_CCIP_FME_DEF_H_
#define __AALKERNEL_CCIP_FME_DEF_H_
#include <aalsdk/kernel/kosal.h>

#include <aalsdk/kernel/aaltypes.h>
#include "cci_pcie_driver_internal.h"
#include "aalsdk/kernel/ccipdriver.h"
#include "aalsdk/kernel/ccip_defs.h"

BEGIN_NAMESPACE(AAL)

///============================================================================
/// Name: fme_device
/// @brief   FPGA Management engine device struct
///============================================================================
struct fme_device
{

   struct CCIP_FME_HDR        *m_pHDR;          // FME Header
   struct CCIP_FME_DFL_THERM  *m_pThermmgmt;    // FME  Thermal Management DFL
   struct CCIP_FME_DFL_PM     *m_pPowermgmt;    // FME  Power Management DFL
   struct CCIP_FME_DFL_FPMON  *m_pPerf;         // FME  Global Performance  DFL
   struct CCIP_FME_DFL_GERROR *m_pGerror;       // FME  Global Error  DFL
   struct CCIP_FME_DFL_PR     *m_pPRmgmt;       // FME  PR Management  DFL


   kosal_pci_dev            *m_pcidev;         // Linux pci_dev pointer (or NULL if manual)

   // Private semaphore
   kosal_semaphore            m_sem;

   // struct ccip_PIPsession   *m_pPIPSession;     // PIP session object

   // btUnsignedInt              m_flags;

   // struct aal_device         m_aal_dev;         // AAL Device from which this is derived
   struct CCIP_FME_DFL_GERROR   m_lastGerror;
   struct CCIP_FME_DFL_THERM    m_lastThermmgmt;

}; // end struct fme_device

#define ccip_fme_dev_board_type(pdev)         ((pdev)->m_boardtype)


#define ccip_fme_dev_pci_dev(pdev)           ((pdev)->m_pcidev)

#define ccip_fme_aal_dev(pdev)               ((pdev)->m_aal_dev)
#define ccip_fme_hdr(pdev)                   ((pdev)->m_pHDR)
#define ccip_fme_therm(pdev)                 ((pdev)->m_pThermmgmt)
#define ccip_fme_power(pdev)                 ((pdev)->m_pPowermgmt)
#define ccip_fme_perf(pdev)                  ((pdev)->m_pPerf)
#define ccip_fme_gerr(pdev)                  ((pdev)->m_pGerror)
#define ccip_fme_pr(pdev)                    ((pdev)->m_pPRmgmt)

#define ccip_fme_lastgerr(pdev)               ((pdev)->m_lastGerror)
#define ccip_fme_lasttherm(pdev)              ((pdev)->m_lastThermmgmt)

#define ccip_fme_mem_sessionp(pdev)              ((pdev)->m_pmem_session)

#define ccip_dev_fme_phys_mmio(pdev)              ((pdev)->m_phys_fme_mmio)
#define ccip_dev_fme_kvp_mmio(pdev)               ((pdev)->m_kvp_fme_mmio)
#define ccip_dev_fme_len_mmio(pdev)               ((pdev)->m_len_fme_mmio)

#define ccip_dev_fme_to_pci_dev(pdev)            ((pdev)->m_pcidev)
#define ccip_dev_fme_to_aaldev(pdev)             ((pdev)->m_aal_dev)

#define ccip_dev_fme_to_PIPsessionp(pdev)        ((pdev)->m_pPIPSession)
#define ccip_dev_fme_psem(pdev)                  (&(pdev)->m_sem)

/// @brief   Get the FPGA Management Engine Device Object.
///
/// @param[in] fme_device fme device pointer.
/// @param[in] pkvp_fme_mmio fme mmio virtual address
/// @return    Device object; NULL == failure
struct fme_device * get_fme_mmio_dev(btVirtAddr pkvp_fme_mmio );

/// @brief Destroy the FME MMIO object.
///
/// @param[in] fme_device fme device object .
/// @return    None
void ccip_destroy_fme_mmio_dev(struct fme_device *);

/// @brief   reads FME header from MMIO.
///
/// @param[in] pkvp_fme_mmio fme mmio virtual address
/// @return    FME Header object; NULL ==failure
struct CCIP_FME_HDR* get_fme_dev_header(btVirtAddr pkvp_fme_mmio );


/// @brief   reads FME header from MMIO.
///
/// @param[in] fme_device fme device pointer.
/// @param[in] pkvp_fme_mmio fme mmio virtual address
/// @return    error code
bt32bitInt get_fme_dev_featurelist(struct fme_device *pfme_dev,btVirtAddr pkvp_fme_mmio );

/// @brief   reads FME Temperature Management CSR
///
/// @param[in] fme_device fme device pointer.
/// @param[in] pkvp_fme_mmio fme mmio virtual address
/// @return    error code
bt32bitInt get_fme_dev_tmp_rev0(struct fme_device *pfme_dev,btVirtAddr pkvp_fme_mmio );;

/// @brief   reads FME Power Management CSR
///
/// @param[in] fme_device fme device pointer.
/// @param[in] pkvp_fme_mmio fme mmio virtual address
/// @return    error code
bt32bitInt get_fme_dev_pm_rev0(struct fme_device *pfme_dev,btVirtAddr pkvp_fme_mmio );

/// @brief   reads FME Global performance CSR
///
/// @param[in] fme_device fme device pointer.
/// @param[in] pkvp_fme_mmio fme mmio virtual address
/// @return    error code
bt32bitInt get_fme_dev_fpmon_rev0(struct fme_device *pfme_dev,btVirtAddr pkvp_fme_mmio );

/// @brief   reads FME Global error CSR
///
/// @param[in] fme_device fme device pointer.
/// @param[in] pkvp_fme_mmio fme mmio virtual address
/// @return    error code
bt32bitInt get_fme_dev_gerr_rev0(struct fme_device *pfme_dev,btVirtAddr pkvp_fme_mmio );

/// @brief   reads FME PR CSR
///
/// @param[in] fme_device fme device pointer.
/// @param[in] pkvp_fme_mmio fme mmio virtual address
/// @return    error code
bt32bitInt get_fme_dev_pr_rev0(struct fme_device *pfme_dev,btVirtAddr pkvp_fme_mmio );

/// @brief   freee FME Device feature list memory
///
/// @param[in] fme_device fme device pointer .
/// @return    void
void ccip_fme_mem_free(struct fme_device *pfme_dev );

/// @brief   get global error
///
/// @param[in] pfme_dev fme device pointer.
/// @param[in] pfme_error ccip error structure  pointer
/// @return    error code
bt32bitInt get_fme_error(struct fme_device   *pfme_dev,
                         struct CCIP_ERROR   *pfme_error);

/// @brief   get fpga power consumed values.
///
/// @param[in] pfme_dev fme device pointer.
/// @param[in] pfme_power ccip power structure  pointer
/// @return    error code
bt32bitInt get_fme_power(struct fme_device         *pfme_dev,
                         struct CCIP_THERMAL_PWR   *pthermal_power);

/// @brief   get fpga thermal values
///
/// @param[in] pfme_dev fme device pointer.
/// @param[in] pPerf ccip thermal structure  pointer
/// @return    error code
bt32bitInt get_fme_thermal(struct fme_device        *pfme_dev,
                           struct CCIP_THERMAL_PWR  *pthermal_power);


extern struct aal_ipip cci_FMEpip;

END_NAMESPACE(AAL)

#endif /* __AALKERNEL_CCIP_FME_DEF_H_ */
