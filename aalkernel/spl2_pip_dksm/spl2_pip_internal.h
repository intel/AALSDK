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
//        FILE: spl2_pip_internal.h
//     CREATED: 02/04/2012
//      AUTHOR: Joseph Grecco, Intel <joe.grecco@intel.com>
//              Henry Mitchel, Intel <henry.mitchel@intel.com>
//
// PURPOSE: Internal private definitions and constants for the Intel(R)
//          Intel QuickAssist Technology SPL2 Physical Interface Protocol
//          Module (PIP).
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 10/25/2012     TSW      Cleanup for faplib
//****************************************************************************
#ifndef __AALKERNEL_SPL2_PIP_DKSM_SPL2_PIP_INTERNAL_H__
#define __AALKERNEL_SPL2_PIP_DKSM_SPL2_PIP_INTERNAL_H__
#include "aalsdk/kernel/kosal.h"
#include "aalsdk/kernel/aalbus-device.h"
#include "aalsdk/kernel/aalqueue.h"
#include "aalsdk/kernel/aalui.h"
#include "aalsdk/kernel/aalui-events.h"
#include "aalsdk/kernel/aalmafu.h"
#include "aalsdk/kernel/vafu2defs.h"
#include "spl2diag.h"         // Useful utilities, and may need to mimic IOCTLs


/////////////////////////////////////////////////////////////////////////////
#ifndef DRV_VERSION
# define DRV_VERSION         "EXPERIMENTAL VERSION"
#endif
#define DRV_DESCRIPTION      "Intel(R) AAL SPL2 Physical Interface Protocol (PIP)"
#define DRV_AUTHOR           "Joseph Grecco <joe.grecco@intel.com>"
#define DRV_LICENSE          "Dual BSD/GPL"
#define DRV_COPYRIGHT        "Copyright (c) 2012-2014 Intel Corporation"

#define DEVICE_BASENAME      "SPL2PIP"

#define SPL2_PCI_DRIVER_NAME "aalspl2"

////////////////////////////////////////////////////////////////////////////////

// dynamic platform detect.
#define NHM_APERTURE_PHYS     __PHYS_ADDR_CONST(0x88080000) ///< Nehalem on Emerald Ridge Platform
#define ROM_APERTURE_PHYS     __PHYS_ADDR_CONST(0xC8080000) ///< Jaketown on Romley Platform
#define QPI_APERTURE_SIZE     ( 0x10000 )                   ///< Size of device CSR region, in bytes

// PCI device IDs
#define QPI_DEVICE_ID_FPGA    0xBCBC ///< QPI device ID
#define PCI_DEVICE_ID_PCIFPGA 0xBCBD ///< PCIe device ID

// CSR config
#define SPL2PIP_CSR_SIZE      4 ///< #bytes defining stride is 4 for write,
#define SPL2PIP_CSR_SPACING   4 ///< inter-CSR stride in bytes


struct spl2_device;
//=============================================================================
// spl2_MAFUpip - MAFU and simulator PIP
//=============================================================================
struct spl2_MAFUpip
{
   struct aal_ipip m_ipip;

   // Callbacks for simulation support
   void (*write_cci_csr32)(struct spl2_device * , btCSROffset , bt32bitCSR );
   void    (*read_cci_csr)(struct spl2_device * , btCSROffset );
};
#define aalpipip_to_spl2_MAFU_ppip(p)   container_of(p, struct spl2_MAFUpip, m_ipip)
#define spl2_MAFU_ppip_to_aalpipip(p)   (&((p)->m_ipip))


//=============================================================================
// Prototypes
//=============================================================================
int cci3_init_ccidevice(struct spl2_device            *pspl2dev,
                        struct aal_device_id          *paaldevid);

int spl2_init_spl3device(struct spl2_device            *pspl2dev,
                         struct aal_device_id          *paaldevid);

int spl2_identify_device(struct spl2_device *pdev);

int
spl2_nopcie_internal_probe(struct spl2_device   *pspl2dev,
                           struct aal_device_id *paaldevid);

/// spl2_viddid_is_supported - Determine whether the board signature encoded
/// in @viddid (PCIe VendorID/DeviceID) is that of an SPL2 device.
/// @viddid: PCIe encoded VendorID/DeviceID to examine.
/// @returns: non-zero if @viddid is supported.
static inline
int
spl2_viddid_is_supported(u32 viddid)
{
   u16 vid   = viddid & 0xffff;
   u16 did   = viddid >> 16;
   int valid = 1;

   valid = valid && ( PCI_VENDOR_ID_INTEL == vid );
   valid = valid && ( ( QPI_DEVICE_ID_FPGA == did ) || ( PCI_DEVICE_ID_PCIFPGA == did ) );

   return valid;
}

/// spl2_alloc_next_afu_addr - Returns a unique AAL device address, each time called.
struct aal_device_addr
spl2_alloc_next_afu_addr(void);

/// spl2_nopcie_internal_probe - Called to probe actual SPL2 devices during manual configuration.
/// @pspl2dev: The module device to be populated.
/// @paaldevid: The module device id to be populated.
/// @returns: 0 on success.
int
spl2_nopcie_internal_probe(struct spl2_device   *pspl2dev,
                           struct aal_device_id *paaldevid);

int
cci_nopcie_internal_probe(struct spl2_device   *pspl2dev,
                          struct aal_device_id *paaldevid);

/// spl2_sim_internal_probe - Called to probe simulated SPL2 devices during manual configuration.
/// @pspl2dev: The module device to be populated.
/// @paaldevid: The module device id to be populated.
/// @returns: 0 on success.
int
spl2_sim_internal_probe(struct spl2_device   *pspl2dev,
                        struct aal_device_id *paaldevid);

/// spl2_device_init - Called to initialize a @pspl2dev that has been successfully probed.
/// Allocates and initializes the internal data structures for the module device.
/// Creates an AFU device, for the given @paaldevid.
/// Adds @pspl2dev to @pdevlist on success.
///
/// @pspl2dev: The module device to be initialized.
/// @paaldevid: The module device id to be initialized.
/// @pdevlist: (optional) The list on which @pspl2dev should be inserted on success.
/// @returns: 0 on success.
int
spl2_spl_device_init(struct spl2_device   *pspl2dev,
                     struct aal_device_id *paaldevid,
                     struct list_head     *pdevlist);


int
spl2_cci_device_init(struct spl2_device   *pspl2dev,
                     struct aal_device_id *paaldevid,
                     struct list_head     *pdevlist);

/// spl2_create_discovered_afu - Create the AAL AFU object detected by a successful device probe.
/// @pspl2dev: The module device for the AFU.
/// @paaldevid: The module device id for the AFU.
///
/// Note: Called by @spl2_device_init.
///
/// @returns: 0 on success.
int
spl2_create_discovered_afu(struct spl2_device   *pspl2dev,
                           struct aal_device_id *paaldevid,
                           struct aal_ipip      *pafupip);

/// spl2_device_destroy - Called to destroy a @pspl2dev.
/// @pspl2dev: The module device to be destroyed.
/// @returns: 0 on success.
void
spl2_device_destroy(struct spl2_device *pspl2dev);

struct spl2_session;

/// spl2_trans_setup - Claims @pspl2sess for @pspl2dev. Initializes the DSMs.
/// @pspl2sess: The session to be claimed.
/// @pspl2dev: The module device to acquire the session.
/// @returns: uid_errnum_OK on success.
uid_errnum_e
spl2_trans_setup(struct spl2_session *pspl2sess,
                 struct spl2_device  *pspl2dev);

/// spl2_trans_start - Sets the AFU/SPL contexts. Enables the device. Initiates the transaction.
/// @pspl2sess: The session associated with the transaction.
/// @pspl2dev: The module device handling the transaction.
/// @pAFUCtxwsid: Buffer structure for the AFU context.
/// @pollrate: Device status poll rate in milliseconds.
/// @returns: uid_errnum_OK on success.
uid_errnum_e
spl2_trans_start(struct spl2_session *pspl2sess,
                 struct spl2_device  *pspl2dev,
                 struct aal_wsid     *pAFUCtxwsid,
                 unsigned             pollrate);

/// spl2_trans_end - Disables/resets the device. Releases the session, if any.
/// @pspl2dev: The active module device.
void
spl2_trans_end(struct spl2_device *pspl2dev);

bt32bitCSR read_cci_csr(struct spl2_device * , btCSROffset );
void    write_cci_csr32(struct spl2_device * , btCSROffset , bt32bitCSR );
void    write_cci_csr64(struct spl2_device * , btCSROffset , bt64bitCSR );

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////                 SPL2 MACROS              ////////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
//=============================================================================


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////            SPL2 AFU Device            ////////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
//=============================================================================
#define SPL2_DISABLE 0
#define SPL2_ENABLE  1
//=============================================================================
// Name: spl2_device
// Description: Structure describing a SPL2 device
//=============================================================================
struct spl2_device {
#define SPL2_DEV_FLAG_PCI_DEV_ENABLED           0x00000001
#define SPL2_DEV_FLAG_PCI_REGION_REQUESTED      0x00000002
#define SPL2_DEV_FLAG_ALLOW_MAP_CSR_READ_SPACE  0x00000004
#define SPL2_DEV_FLAG_ALLOW_MAP_CSR_WRITE_SPACE 0x00000008
#define SPL2_DEV_FLAG_SIMULATED_DEV             0x00000010
#define CCI_PROTOCOL_CCI                        1
#define CCI_PROTOCOL_SPL                        0
   btUnsignedInt              m_flags;

   // List of devices in existence.  Maintained by the PIP module (ko).
   struct semaphore           m_session_sem;    // List protection
   struct list_head           m_session_list;   // List itself

   // Private semaphore
   struct semaphore           m_sem;

   //
   struct spl2_session       *m_activesession;
   struct semaphore           m_tran_done_sem;

   // Device pair. m_aaldev is the "real" AAL device
   //  m_simdev is used only if this device is simulated
   struct aal_device         *m_aaldev;

   struct aal_device         *m_simdev;        // Simulator device
   struct spl2_MAFUpip       *m_simPIP;        // Simulator's driver interface
   struct spl2_session       *m_simsess;       // Session
   int                        m_simulated;     // bool 1=simulated
   struct aal_device_id       m_simdevid;      // Attributes of real device to simulate

   struct VAFU2_DSM          *m_AFUDSM;
   struct aal_wsid           *m_AFUDSM_wsid;
   struct aalui_WSMParms      m_AFUDSM_WSMParms;
   struct SPL2_DSM           *m_SPL2DSM;
   struct SPL2_CNTXT         *m_SPL2CTX;

   struct pci_dev            *m_pcidev;         // Linux pci_dev pointer (or NULL if manual)
   enum aal_bus_types_e       m_boardtype;

   int                        m_protocolID;

   // PCI configuration space parameters
   void __iomem              *m_kvp_config;     // kv address after iomap
   btPhysAddr                 m_phys_config;    // Physical mmio address
   size_t                     m_len_config;     // Bytes

   // CCI CSR Space
   void __iomem              *m_kvp_cci_csr;    // kv address of CSR space
   btPhysAddr                 m_phys_cci_csr;   // Physical address of CSR space
   size_t                     m_len_cci_csr;    // Bytes

   // AFU CSR Space
   void __iomem              *m_kvp_afu_csr;    // kv address of CSR space
   btPhysAddr                 m_phys_afu_csr;   // Physical address of CSR space
   size_t                     m_len_afu_csr;    // Bytes

   // Memory subsystem, one per device, currently only support one board
   struct memmgr_session     *m_pmem_session;   // memory session context used by memory sub-system

   // Worker thread items
   struct workqueue_struct   *m_workq;
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,19)
   struct delayed_work        task_handler;
#else
   struct work_struct         task_handler;
#endif
};

#define spl2_dev_AFUDSM(pdev)                ((pdev)->m_AFUDSM)
#define spl2_dev_AFUDSM_type                 struct VAFU2_DSM
#define spl2_dev_AFUDSM_size                 sizeof( spl2_dev_AFUDSM_type )
#define spl2_dev_AFUDSM_wsid(pdev)           ((pdev)->m_AFUDSM_wsid)
#define spl2_dev_AFUDSM_WSMParms(pdev)       ((pdev)->m_AFUDSM_WSMParms)

#define spl2_dev_SPLDSM(pdev)                ((pdev)->m_SPL2DSM)
#define spl2_dev_SPLDSM_type                 struct SPL2_DSM
#define spl2_dev_SPLDSM_size                 sizeof( spl2_dev_SPLDSM_type )

#define spl2_dev_workq(pdev)                 ((pdev)->m_workq)
#define spl2_dev_task_handler(pdev)          (&(pdev)->task_handler)
#define spl2_dev_pollrate(pdev)              ((pdev)->m_activesession->pollrate)

#define spl2_dev_semp(pdev)                  (&(pdev)->m_sem)

#define spl2_dev_activesess(pdev)            ((pdev)->m_activesession)
#define spl2_dev_tran_done_semp(pdev)        (&(pdev)->m_tran_done_sem)

#define spl2_dev_SPLCTX(pdev)                ((pdev)->m_SPL2CTX)
#define spl2_dev_SPLCTX_type                 struct SPL2_CNTXT
#define spl2_dev_SPLCTX_size                 sizeof( spl2_dev_SPLCTX_type )
#define spl2_dev_clrSPLCTX(pdev)             memset(spl2_dev_SPLCTX(pdev), 0, spl2_dev_SPLCTX_size)

#define pci_dev_to_spl2_dev(ptr)             spl2_container_of(ptr, struct pci_dev, m_pcidev, struct spl2_device)

#define spl2_dev_pci_dev(pdev)               ((pdev)->m_pcidev)
   #define spl2_dev_pci_dev_is_enabled(pdev)  ((pdev)->m_flags & SPL2_DEV_FLAG_PCI_DEV_ENABLED)
   #define spl2_dev_pci_dev_set_enabled(pdev) ((pdev)->m_flags |= SPL2_DEV_FLAG_PCI_DEV_ENABLED)
   #define spl2_dev_pci_dev_clr_enabled(pdev) ((pdev)->m_flags &= ~SPL2_DEV_FLAG_PCI_DEV_ENABLED)

   #define spl2_dev_pci_dev_is_region_requested(pdev)  ((pdev)->m_flags & SPL2_DEV_FLAG_PCI_REGION_REQUESTED)
   #define spl2_dev_pci_dev_set_region_requested(pdev) ((pdev)->m_flags |= SPL2_DEV_FLAG_PCI_REGION_REQUESTED)
   #define spl2_dev_pci_dev_clr_region_requested(pdev) ((pdev)->m_flags &= ~SPL2_DEV_FLAG_PCI_REGION_REQUESTED)

   #define spl2_dev_allow_map_csr_read_space(pdev)     ((pdev)->m_flags & SPL2_DEV_FLAG_ALLOW_MAP_CSR_READ_SPACE)
   #define spl2_dev_set_allow_map_csr_read_space(pdev) ((pdev)->m_flags |= SPL2_DEV_FLAG_ALLOW_MAP_CSR_READ_SPACE)
   #define spl2_dev_clr_allow_map_csr_read_space(pdev) ((pdev)->m_flags &= ~SPL2_DEV_FLAG_ALLOW_MAP_CSR_READ_SPACE)

   #define spl2_dev_allow_map_csr_write_space(pdev)     ((pdev)->m_flags & SPL2_DEV_FLAG_ALLOW_MAP_CSR_WRITE_SPACE)
   #define spl2_dev_set_allow_map_csr_write_space(pdev) ((pdev)->m_flags |= SPL2_DEV_FLAG_ALLOW_MAP_CSR_WRITE_SPACE)
   #define spl2_dev_clr_allow_map_csr_write_space(pdev) ((pdev)->m_flags &= ~SPL2_DEV_FLAG_ALLOW_MAP_CSR_WRITE_SPACE)

   #define spl2_dev_allow_map_csr_space(pdev) ( spl2_dev_allow_map_csr_read_space(pdev) || spl2_dev_allow_map_csr_write_space(pdev) )

   #define spl2_dev_is_simulated(pdev)  ((pdev)->m_flags & SPL2_DEV_FLAG_SIMULATED_DEV)
   #define spl2_dev_set_simulated(pdev) ((pdev)->m_flags |= SPL2_DEV_FLAG_SIMULATED_DEV)
   #define spl2_dev_clr_simulated(pdev) ((pdev)->m_flags &= ~SPL2_DEV_FLAG_SIMULATED_DEV)

#define spl2_dev_board_type(pdev)            ((pdev)->m_boardtype)

#define spl2_dev_protocol(pdev)              ((pdev)->m_protocolID)

#define spl2_dev_kvp_config(pdev)            ((pdev)->m_kvp_config)
#define spl2_dev_phys_config(pdev)           ((pdev)->m_phys_config)
#define spl2_dev_len_config(pdev)            ((pdev)->m_len_config)
#define spl2_dev_mem_sessionp(pdev)          ((pdev)->m_pmem_session)

#define spl2_dev_phys_cci_csr(pdev)          ((pdev)->m_phys_cci_csr)
#define spl2_dev_kvp_cci_csr(pdev)           ((pdev)->m_kvp_cci_csr)
#define spl2_dev_len_cci_csr(pdev)           ((pdev)->m_len_cci_csr)

#define spl2_dev_phys_afu_csr(pdev)          ((pdev)->m_phys_afu_csr)
#define spl2_dev_kvp_afu_csr(pdev)           ((pdev)->m_kvp_afu_csr)
#define spl2_dev_len_afu_csr(pdev)           ((pdev)->m_len_afu_csr)

#define spl2_dev_to_pci_dev(pdev)            ((pdev)->m_pcidev)
#define spl2_dev_to_aaldev(pdev)             ((pdev)->m_aaldev)
#define spl2_dev_to_aalsimdev(pdev)          ((pdev)->m_simdev)
#define spl2_dev_to_simpip(pdev)             (*(pdev)->m_simPIP)
#define spl2_dev_to_simsession(pdev)         ((pdev)->m_simsess)
#define spl2_dev_to_sim_devid(pdev)          ((pdev)->m_simdevid)

#define spl2_list_to_spl2_device(plist)      list_entry(plist, struct spl2_device, m_session_list)
#define aaldev_to_spl2_device(plist)         list_entry(plist, struct spl2_device, m_session_list)

//=============================================================================
//=============================================================================
//                                PROTOTYPES
//=============================================================================
//=============================================================================
void AFUrelease_device( struct device *pdev );
int spl2_mmap(struct aaldev_ownerSession* pownerSess,
              struct aal_wsid *wsidp,
              btAny os_specific);
int cci3_mmap(struct aaldev_ownerSession* pownerSess,
              struct aal_wsid *wsidp,
              btAny os_specific);

extern struct spl2_MAFUpip MAFUpip;
extern struct aal_ipip SPLAFUpip;
extern struct aal_ipip CCIAFUpip;


//=============================================================================
//=============================================================================
//                                INLINE PRIMITIVES
//=============================================================================
//=============================================================================

//=============================================================================
// Name: spl2_enable
// Description: Enable or disable an AFU
// Input: pdev - device to enable or disable
//        enable - SPL2_ENABLE enable; SPL2_DISABLE - disable
// Comment:
// Returns: none
// Comments:
//=============================================================================
static inline
void
spl2_enable(struct spl2_device *pdev, int enable)
{
   struct SPL2_CH_CTRL ch_ctrl_csr;
   ch_ctrl_csr.csr    = 0;                               // Clear overall csr to 0
   ch_ctrl_csr.Enable = (SPL2_ENABLE == enable ? 1 : 0); // Enable = 1, Disable = 0
   write_cci_csr32(pdev, byte_offset_SPL2_CH_CTRL, ch_ctrl_csr.csr);
} // spl2_enable

//=============================================================================
// Name: spl2_spl_reset
// Description: Reset the SPL
// Input: pdev - device to reset
// Comment:
// Returns: none
// Comments:
//=============================================================================
static inline
void
spl2_spl_reset(struct spl2_device *pdev)
{
   struct SPL2_CH_CTRL ch_ctrl_csr;
   ch_ctrl_csr.csr   = 0; // Clear overall csr to 0
   ch_ctrl_csr.Reset = 1; // Reset the SPL -- next interaction is expected to be DSM_SPL probe
   write_cci_csr32(pdev, byte_offset_SPL2_CH_CTRL, ch_ctrl_csr.csr);
} // spl2_spl_reset

#endif // __AALKERNEL_SPL2_PIP_DKSM_SPL2_PIP_INTERNAL_H__

