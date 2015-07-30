//******************************************************************************
// Part of the Intel(R) QuickAssist Technology Accelerator Abstraction Layer
//
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2012-2015, Intel Corporation.
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
//  Copyright(c) 2012-2015, Intel Corporation.
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
//        FILE: cciv4_driver_internal.h
//     CREATED: 07/27/2015
//      AUTHOR: Joseph Grecco, Intel <joe.grecco@intel.com>
//              Henry Mitchel, Intel <henry.mitchel@intel.com>
//
// PURPOSE: Internal private definitions and constants for the Intel(R)
//          Intel QuickAssist Technology CCIv4 Device Driver (PIP).
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
//****************************************************************************
#ifndef __AALKERNEL_CCIV4_DRIVER_INTERNAL_H__
#define __AALKERNEL_CCIV4_DRIVER_INTERNAL_H__
#include "aalsdk/kernel/kosal.h"
#include "aalsdk/kernel/aalbus-device.h"
#include "aalsdk/kernel/aalqueue.h"
#include "aalsdk/kernel/aalui.h"
#include "aalsdk/kernel/aalui-events.h"
#include "aalsdk/kernel/aalmafu.h"
#include "aalsdk/kernel/vafu2defs.h"

#include "cciv4_PIPsession.h"

//#include "spl2diag.h"         // Useful utilities, and may need to mimic IOCTLs


/////////////////////////////////////////////////////////////////////////////
#ifndef DRV_VERSION
# define DRV_VERSION          "EXPERIMENTAL VERSION"
#endif
#define DRV_DESCRIPTION       "Intel(R) AAL FPGA Device driver and CCIv4 Physical Interface Protocol Driver (PIP)"
#define DRV_AUTHOR            "Joseph Grecco <joe.grecco@intel.com>"
#define DRV_LICENSE           "Dual BSD/GPL"
#define DRV_COPYRIGHT         "Copyright (c) 2012-2015 Intel Corporation"

#define DEVICE_BASENAME       "cciv4"

#define CCIV4_PCI_DRIVER_NAME "aalcciv4"

////////////////////////////////////////////////////////////////////////////////

// dynamic platform detect.
#define NHM_APERTURE_PHYS     __PHYS_ADDR_CONST(0x88080000) ///< Nehalem on Emerald Ridge Platform
#define ROM_APERTURE_PHYS     __PHYS_ADDR_CONST(0xC8080000) ///< Jaketown on Romley Platform
#define QPI_APERTURE_SIZE     ( 0x10000 )                   ///< Size of device CSR region, in bytes

#define CCIV4_MMIO_SIZE       ( 0x20000 )                   /// Size of AFU MMIO space
#define CCIV4_UMSG_SIZE       ( 0x5000 )                    /// Size of uMsg space

// PCI device IDs
#define QPI_DEVICE_ID_FPGA    0xBCBC ///< QPI device ID
#define PCI_DEVICE_ID_PCIFPGA 0xBCBD ///< PCIe device ID

// CSR config
#define CCIV4_CSR_SIZE      4 ///< #bytes defining stride is 4 for write,
#define CCIV4_CSR_SPACING   4 ///< inter-CSR stride in bytes


struct cciv4_device;
//=============================================================================
// MAFUpip - MAFU and simulator PIP
//=============================================================================
struct cciv4_MAFUpip
{
   struct aal_ipip m_ipip;

   // Callbacks for simulation support
   void (*write_cci_csr32)(struct cciv4_device * , btCSROffset , bt32bitCSR );
   void (*read_cci_csr)(struct cciv4_device * , btCSROffset );
};
#define aalpipip_to_cciv4_MAFU_ppip(p)   container_of(p, struct cciv4_MAFUpip, m_ipip)
#define cciv4_MAFU_ppip_to_aalpipip(p)   (&((p)->m_ipip))


//=============================================================================
// Prototypes
//=============================================================================


bt32bitCSR read_cci_csr(struct cciv4_device * , btCSROffset );
void    write_cci_csr32(struct cciv4_device * , btCSROffset , bt32bitCSR );
void    write_cci_csr64(struct cciv4_device * , btCSROffset , bt64bitCSR );


//=============================================================================
// Name: cciv4_device
// Description: Structure describing a SPL2 device
//=============================================================================
struct cciv4_device {
#define CCIV4_DEV_FLAG_PCI_DEV_ENABLED           0x00000001
#define CCIV4_DEV_FLAG_PCI_REGION_REQUESTED      0x00000002
#define CCIV4_DEV_FLAG_ALLOW_MAP_CSR_READ_SPACE  0x00000004
#define CCIV4_DEV_FLAG_ALLOW_MAP_CSR_WRITE_SPACE 0x00000008
#define CCIV4_DEV_FLAG_SIMULATED_DEV             0x00000010
#define CCIV4_DEV_FLAG_ALLOW_MAP_MMIOR_SPACE     0x00000020
#define CCIV4_DEV_FLAG_ALLOW_MAP_UMSG_SPACE      0x00000040

   struct aal_device         *m_aaldev;         // AAL Device from which this is derived
   struct pci_dev            *m_pcidev;         // Linux pci_dev pointer (or NULL if manual)

   btUnsignedInt              m_flags;

   // Used for being added to the global list of devices.
   struct list_head           m_list;           // List itself

   // Private semaphore
   struct semaphore           m_sem;

   enum aal_bus_types_e       m_boardtype;

   int                        m_simulated;

   int                        m_protocolID;

   // PCI configuration space parameters
   btVirtAddr                 m_kvp_config;     // kv address after iomap
   btPhysAddr                 m_phys_config;    // Physical mmio address
   size_t                     m_len_config;     // Bytes

   // CCI MMIO Config Space
   btVirtAddr                 m_kvp_cci_csr;    // kv address of CSR space
   btPhysAddr                 m_phys_cci_csr;   // Physical address of CSR space
   size_t                     m_len_cci_csr;    // Bytes

   // AFU MMIO Space
   btVirtAddr                 m_kvp_afu_mmio;   // kv address of MMIO space
   btPhysAddr                 m_phys_afu_mmio;  // Physical address of MMIO space
   size_t                     m_len_afu_mmio;   // Bytes

   // AFU uMSG Space
   btVirtAddr                 m_kvp_afu_umsg;    // kv address of CSR space
   btPhysAddr                 m_phys_afu_umsg;   // Physical address of CSR space
   size_t                     m_len_afu_umsg;    // Bytes

   struct cciv4_PIPsession   *m_pPIPSession;     // PIP session object
};


#define pci_dev_to_cciv4_dev(ptr)             cciv4_container_of(ptr, struct pci_dev, m_pcidev, struct cciv4_device)

#define cciv4_dev_pci_dev(pdev)               ((pdev)->m_pcidev)
   #define cciv4_dev_pci_dev_is_enabled(pdev)  ((pdev)->m_flags & CCIV4_DEV_FLAG_PCI_DEV_ENABLED)
   #define cciv4_dev_pci_dev_set_enabled(pdev) ((pdev)->m_flags |= CCIV4_DEV_FLAG_PCI_DEV_ENABLED)
   #define cciv4_dev_pci_dev_clr_enabled(pdev) ((pdev)->m_flags &= ~CCIV4_DEV_FLAG_PCI_DEV_ENABLED)

   #define cciv4_dev_pci_dev_is_region_requested(pdev)  ((pdev)->m_flags & CCIV4_DEV_FLAG_PCI_REGION_REQUESTED)
   #define cciv4_dev_pci_dev_set_region_requested(pdev) ((pdev)->m_flags |= CCIV4_DEV_FLAG_PCI_REGION_REQUESTED)
   #define cciv4_dev_pci_dev_clr_region_requested(pdev) ((pdev)->m_flags &= ~CCIV4_DEV_FLAG_PCI_REGION_REQUESTED)

   #define cciv4_dev_allow_map_csr_read_space(pdev)     ((pdev)->m_flags & CCIV4_DEV_FLAG_ALLOW_MAP_CSR_READ_SPACE)
   #define cciv4_dev_set_allow_map_csr_read_space(pdev) ((pdev)->m_flags |= CCIV4_DEV_FLAG_ALLOW_MAP_CSR_READ_SPACE)
   #define cciv4_dev_clr_allow_map_csr_read_space(pdev) ((pdev)->m_flags &= ~CCIV4_DEV_FLAG_ALLOW_MAP_CSR_READ_SPACE)

   #define cciv4_dev_allow_map_csr_write_space(pdev)     ((pdev)->m_flags & CCIV4_DEV_FLAG_ALLOW_MAP_CSR_WRITE_SPACE)
   #define cciv4_dev_set_allow_map_csr_write_space(pdev) ((pdev)->m_flags |= CCIV4_DEV_FLAG_ALLOW_MAP_CSR_WRITE_SPACE)
   #define cciv4_dev_clr_allow_map_csr_write_space(pdev) ((pdev)->m_flags &= ~CCIV4_DEV_FLAG_ALLOW_MAP_CSR_WRITE_SPACE)

   #define cciv4_dev_allow_map_csr_space(pdev) ( cciv4_dev_allow_map_csr_read_space(pdev) || cciv4_dev_allow_map_csr_write_space(pdev) )

   #define cciv4_dev_allow_map_mmior_space(pdev)     ((pdev)->m_flags & CCIV4_DEV_FLAG_ALLOW_MAP_MMIOR_SPACE)
   #define cciv4_dev_set_allow_map_mmior_space(pdev) ((pdev)->m_flags |= CCIV4_DEV_FLAG_ALLOW_MAP_MMIOR_SPACE)
   #define cciv4_dev_clr_allow_map_mmior_space(pdev) ((pdev)->m_flags &= ~CCIV4_DEV_FLAG_ALLOW_MAP_MMIOR_SPACE)

   #define cciv4_dev_allow_map_umsg_space(pdev)     ((pdev)->m_flags & CCIV4_DEV_FLAG_ALLOW_MAP_UMSG_SPACE)
   #define cciv4_dev_set_allow_map_umsg_space(pdev) ((pdev)->m_flags |= CCIV4_DEV_FLAG_ALLOW_MAP_UMSG_SPACE)
   #define cciv4_dev_clr_allow_map_umsg_space(pdev) ((pdev)->m_flags &= ~CCIV4_DEV_FLAG_ALLOW_MAP_UMSG_SPACE)

   #define cciv4_dev_is_simulated(pdev)  ((pdev)->m_flags & CCIV4_DEV_FLAG_SIMULATED_DEV)
   #define cciv4_dev_set_simulated(pdev) ((pdev)->m_flags |= CCIV4_DEV_FLAG_SIMULATED_DEV)
   #define cciv4_dev_clr_simulated(pdev) ((pdev)->m_flags &= ~CCIV4_DEV_FLAG_SIMULATED_DEV)

#define cciv4_dev_board_type(pdev)            ((pdev)->m_boardtype)

#define cciv4_set_simulated(pdev)             ((pdev)->m_simulated = 1)
#define cciv4_clr_simulated(pdev)             ((pdev)->m_simulated = 0)
#define cciv4_is_simulated(pdev)             ((pdev)->m_simulated == 1)

#define cciv4_dev_protocol(pdev)              ((pdev)->m_protocolID)

#define cciv4_dev_kvp_config(pdev)            ((pdev)->m_kvp_config)
#define cciv4_dev_phys_config(pdev)           ((pdev)->m_phys_config)
#define cciv4_dev_len_config(pdev)            ((pdev)->m_len_config)
#define cciv4_dev_mem_sessionp(pdev)          ((pdev)->m_pmem_session)

#define cciv4_dev_phys_cci_csr(pdev)          ((pdev)->m_phys_cci_csr)
#define cciv4_dev_kvp_cci_csr(pdev)           ((pdev)->m_kvp_cci_csr)
#define cciv4_dev_len_cci_csr(pdev)           ((pdev)->m_len_cci_csr)

#define cciv4_dev_phys_afu_mmio(pdev)         ((pdev)->m_phys_afu_mmio)
#define cciv4_dev_kvp_afu_mmio(pdev)          ((pdev)->m_kvp_afu_mmio)
#define cciv4_dev_len_afu_mmio(pdev)          ((pdev)->m_len_afu_mmio)

#define cciv4_dev_phys_afu_umsg(pdev)         ((pdev)->m_phys_afu_umsg)
#define cciv4_dev_kvp_afu_umsg(pdev)          ((pdev)->m_kvp_afu_umsg)
#define cciv4_dev_len_afu_umsg(pdev)          ((pdev)->m_len_afu_umsg)


#define cciv4_dev_to_pci_dev(pdev)            ((pdev)->m_pcidev)
#define cciv4_dev_to_aaldev(pdev)             ((pdev)->m_aaldev)


#define cciv4_dev_list_head(pdev)             ((pdev)->m_list)
#define cciv4_list_to_cciv4_device(plist)     kosal_list_entry(plist, struct cciv4_device, m_list)
#define aaldev_to_cciv4_device(plist)         kosal_list_entry(plist, struct cciv4_device, m_list)
#define cciv4_dev_to_PIPsessionp(pdev)        ((pdev)->m_pPIPSession)
#define cciv4_dev_psem(pdev)                  (&(pdev)->m_sem)

//=============================================================================
//=============================================================================
//                                PROTOTYPES
//=============================================================================
//=============================================================================
void cciv4_release_device( struct device *pdev );

extern int
cciv4_sim_mmap(struct aaldev_ownerSession* pownerSess,
               struct aal_wsid *wsidp,
               btAny os_specific);

extern int
cciv4_publish_aaldevice(struct cciv4_device *,
                        struct aal_device_id*,
                        struct aal_ipip*,
                        btUnsignedInt maxshares);

extern struct cciv4_device* cciv4_create_device(void);

extern int
cciv4_destroy_device(struct cciv4_device* );

extern void
cciv4_remove_device(struct cciv4_device *);

extern struct aal_ipip cciv4_simAFUpip;
extern struct aal_ipip cciv4_simMAFUpip;
extern struct aal_ipip cciv4_simCMAFUpip;

extern void
cciv4_flush_all_wsids(struct cciv4_PIPsession *psess);

//=============================================================================
//=============================================================================
//                                INLINE PRIMITIVES
//=============================================================================
//=============================================================================



#endif // __AALKERNEL_CCIV4_DRIVER_INTERNAL_H__

