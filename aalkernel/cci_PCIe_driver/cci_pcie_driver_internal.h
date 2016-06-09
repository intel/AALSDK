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
//  Copyright(c) 2012-2016, Intel Corporation.
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
//        FILE: cci_pcie_driver_internal.h
//     CREATED: 10/14/2015
//      AUTHOR: Joseph Grecco, Intel <joe.grecco@intel.com>
//
// PURPOSE: Internal private definitions and constants for the
//          CCI PCIe Device Driver.
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
//****************************************************************************
#ifndef __AALKERNEL_CCI_PCIE_DRIVER_INTERNAL_H__
#define __AALKERNEL_CCI_PCIE_DRIVER_INTERNAL_H__
#include "aalsdk/kernel/kosal.h"

#include <aalsdk/kernel/aalqueue.h>
#include <aalsdk/kernel/iaaldevice.h>
#include <aalsdk/kernel/aalwsservice.h>

/////////////////////////////////////////////////////////////////////////////
#ifndef DRV_VERSION
# define DRV_VERSION          "EXPERIMENTAL VERSION"
#endif
#define DRV_DESCRIPTION       "AAL FPGA PCIe Device driver and CCI Physical Interface Protocol (PIP)"
#define DRV_AUTHOR            "Joseph Grecco <joe.grecco@intel.com>"
#define DRV_LICENSE           "Dual BSD/GPL"
#define DRV_COPYRIGHT         "Copyright(c) 2015-2016, Intel Corporation"

#define DEVICE_BASENAME       "cci"

#define CCI_PCI_DRIVER_NAME   "ccidrv"

////////////////////////////////////////////////////////////////////////////////
#define CCI_MMIO_SIZE       ( 0x40000 )                   /// Size of AFU MMIO space
#define CCI_UMSG_SIZE       ( 0x5000 )                    /// Size of uMsg space

// PCI device IDs
#define PCIe_DEVICE_ID_RCiEP0    0xBCBD ///< Primary port with FIU
#define PCIe_DEVICE_ID_RCiEP1    0xBCBE ///< Null device for data transport
#define PCIe_DEVICE_ID_RCiEP2    0xBCBC ///< QPI or UPI EPt


//=============================================================================
// Prototypes
//=============================================================================

enum   cci_devtype{
   cci_dev_FME,
   cci_dev_Port,
   cci_dev_UAFU,
   cci_dev_STAP,
   cci_dev_PR
};

//=============================================================================
// Name: cci_aal_device
// Description: Structure describing a CCI AAL device. This object is used to
//              expose an allocatable object to the host via the aalbus.
//=============================================================================
struct cci_aal_device {
#define CCI_DEV_FLAG_PCI_DEV_ENABLED           0x00000001
#define CCI_DEV_FLAG_PCI_REGION_REQUESTED      0x00000002
#define CCI_DEV_FLAG_ALLOW_MAP_CSR_READ_SPACE  0x00000004
#define CCI_DEV_FLAG_ALLOW_MAP_CSR_WRITE_SPACE 0x00000008
#define CCI_DEV_FLAG_SIMULATED_DEV             0x00000010
#define CCI_DEV_FLAG_ALLOW_MAP_MMIOR_SPACE     0x00000020
#define CCI_DEV_FLAG_ALLOW_MAP_UMSG_SPACE      0x00000040

   struct aal_device         *m_aaldev;         // AAL Device from which this is derived
   kosal_pci_dev            *m_pcidev;         // Linux pci_dev pointer (or NULL if manual)

   btUnsignedInt              m_flags;

   // Used for being added to the global list of devices.
   kosal_list_head            m_list;           // List itself

   // Private semaphore
   kosal_semaphore            m_sem;

   enum aal_bus_types_e       m_boardtype;

   int                        m_simulated;

   int                        m_protocolID;

   // For background tasks handling
   kosal_work_queue           m_workq_deactivate;

   kosal_work_queue           m_workq_prconifg;

   kosal_work_queue           m_workq_revokeafu;

   kosal_work_queue           m_workq_revokesigtap;

   // AFU MMIO Space
   btVirtAddr                 m_kvp_afu_mmio;   // kv address of MMIO space
   btPhysAddr                 m_phys_afu_mmio;  // Physical address of MMIO space
   btUnsigned64bitInt         m_len_afu_mmio;   // Bytes

   // AFU uMSG Space
   btVirtAddr                 m_kvp_afu_umsg;    // kv address of CSR space
   btPhysAddr                 m_phys_afu_umsg;   // Physical address of CSR space
   btUnsigned64bitInt         m_len_afu_umsg;    // Bytes

   struct cci_PIPsession     *m_pPIPSession;     // PIP session object

   enum   cci_devtype         m_devtype;        // Type of the subclass (e.g., FME, PORT, AFU)
   struct fme_device         *m_pfme;
   struct port_device        *m_pport;

   // PR command Handler semaphore
   kosal_semaphore            m_pr_sem;

};


#define pci_dev_to_cci_dev(ptr)              kosal_container_of(ptr, struct cci_aal_device. m_pcidev )
#define aaldev_to_cci_aal_device(ptr)        aaldev_to_any(struct cci_aal_device, ptr)

#define cci_aaldev_pfme(pdev)                  ( pdev->m_pfme )
#define cci_aaldev_pport(pdev)                 ( pdev->m_pport )
#define cci_aaldev_pafu(pdev)                  ( pdev->m_pafu )

#define cci_aaldev_pci_dev(pdev)               ((pdev)->m_pcidev)
   #define cci_aaldev_pci_dev_is_enabled(pdev)  ((pdev)->m_flags & CCI_DEV_FLAG_PCI_DEV_ENABLED)
   #define cci_aaldev_pci_dev_set_enabled(pdev) ((pdev)->m_flags |= CCI_DEV_FLAG_PCI_DEV_ENABLED)
   #define cci_aaldev_pci_dev_clr_enabled(pdev) ((pdev)->m_flags &= ~CCI_DEV_FLAG_PCI_DEV_ENABLED)

   #define cci_aaldev_pci_dev_is_region_requested(pdev)  ((pdev)->m_flags & CCI_DEV_FLAG_PCI_REGION_REQUESTED)
   #define cci_aaldev_pci_dev_set_region_requested(pdev) ((pdev)->m_flags |= CCI_DEV_FLAG_PCI_REGION_REQUESTED)
   #define cci_aaldev_pci_dev_clr_region_requested(pdev) ((pdev)->m_flags &= ~CCI_DEV_FLAG_PCI_REGION_REQUESTED)

   #define cci_aaldev_allow_map_csr_read_space(pdev)     ((pdev)->m_flags & CCI_DEV_FLAG_ALLOW_MAP_CSR_READ_SPACE)
   #define cci_aaldev_set_allow_map_csr_read_space(pdev) ((pdev)->m_flags |= CCI_DEV_FLAG_ALLOW_MAP_CSR_READ_SPACE)
   #define cci_aaldev_clr_allow_map_csr_read_space(pdev) ((pdev)->m_flags &= ~CCI_DEV_FLAG_ALLOW_MAP_CSR_READ_SPACE)

   #define cci_aaldev_allow_map_csr_write_space(pdev)     ((pdev)->m_flags & CCI_DEV_FLAG_ALLOW_MAP_CSR_WRITE_SPACE)
   #define cci_aaldev_set_allow_map_csr_write_space(pdev) ((pdev)->m_flags |= CCI_DEV_FLAG_ALLOW_MAP_CSR_WRITE_SPACE)
   #define cci_aaldev_clr_allow_map_csr_write_space(pdev) ((pdev)->m_flags &= ~CCI_DEV_FLAG_ALLOW_MAP_CSR_WRITE_SPACE)

   #define cci_aaldev_allow_map_csr_space(pdev) ( cci_aaldev_allow_map_csr_read_space(pdev) || cci_aaldev_allow_map_csr_write_space(pdev) )

   #define cci_aaldev_allow_map_mmior_space(pdev)     ((pdev)->m_flags & CCI_DEV_FLAG_ALLOW_MAP_MMIOR_SPACE)
   #define cci_aaldev_set_allow_map_mmior_space(pdev) ((pdev)->m_flags |= CCI_DEV_FLAG_ALLOW_MAP_MMIOR_SPACE)
   #define cci_aaldev_clr_allow_map_mmior_space(pdev) ((pdev)->m_flags &= ~CCI_DEV_FLAG_ALLOW_MAP_MMIOR_SPACE)

   #define cci_aaldev_allow_map_umsg_space(pdev)     ((pdev)->m_flags & CCI_DEV_FLAG_ALLOW_MAP_UMSG_SPACE)
   #define cci_aaldev_set_allow_map_umsg_space(pdev) ((pdev)->m_flags |= CCI_DEV_FLAG_ALLOW_MAP_UMSG_SPACE)
   #define cci_aaldev_clr_allow_map_umsg_space(pdev) ((pdev)->m_flags &= ~CCI_DEV_FLAG_ALLOW_MAP_UMSG_SPACE)

   #define cci_aaldev_is_simulated(pdev)  ((pdev)->m_flags & CCI_DEV_FLAG_SIMULATED_DEV)
   #define cci_aaldev_set_simulated(pdev) ((pdev)->m_flags |= CCI_DEV_FLAG_SIMULATED_DEV)
   #define cci_aaldev_clr_simulated(pdev) ((pdev)->m_flags &= ~CCI_DEV_FLAG_SIMULATED_DEV)

#define cci_aaldev_type(pdev)                  ((pdev)->m_devtype)

#define cci_aaldev_board_type(pdev)            ((pdev)->m_boardtype)

#define cci_set_simulated(pdev)             ((pdev)->m_simulated = 1)
#define cci_clr_simulated(pdev)             ((pdev)->m_simulated = 0)
#define cci_is_simulated(pdev)             ((pdev)->m_simulated == 1)

#define cci_aaldev_protocol(pdev)              ((pdev)->m_protocolID)

#define cci_aaldev_phys_afu_mmio(pdev)         ((pdev)->m_phys_afu_mmio)
#define cci_aaldev_kvp_afu_mmio(pdev)          ((pdev)->m_kvp_afu_mmio)
#define cci_aaldev_len_afu_mmio(pdev)          ((pdev)->m_len_afu_mmio)

#define cci_aaldev_phys_afu_umsg(pdev)         ((pdev)->m_phys_afu_umsg)
#define cci_aaldev_kvp_afu_umsg(pdev)          ((pdev)->m_kvp_afu_umsg)
#define cci_aaldev_len_afu_umsg(pdev)          ((pdev)->m_len_afu_umsg)


#define cci_aaldev_to_pci_dev(pdev)            ((pdev)->m_pcidev)
#define cci_aaldev_to_aaldev(pdev)          ( (pdev)->m_aaldev )


#define cci_aaldev_list_head(pdev)             ((pdev)->m_list)
#define cci_list_to_cci_aal_device(plist)    kosal_list_entry(plist, struct cci_aal_device, m_list)

#define cci_aaldev_to_PIPsessionp(pdev)        ((pdev)->m_pPIPSession)
#define cci_aaldev_psem(pdev)                  (&(pdev)->m_sem)
#define cci_dev_pr_sem(pdev)                (&(pdev)->m_pr_sem)


#define cci_aaldev_workq_deactivate(pdev)                ((pdev)->m_workq_deactivate )
#define cci_aaldev_workq_prcconfigure(pdev)              ((pdev)->m_workq_prconifg )
#define cci_aaldev_workq_revokeafu(pdev)                 ((pdev)->m_workq_revokeafu )
#define cci_aaldev_workq_revokesigtap(pdev)              ((pdev)->m_workq_revokesigtap )

///============================================================================
/// Name: ccip_device
/// @brief  CCIP board device
///============================================================================
struct ccip_device
{
   // Used for being added to the global list of devices.
   kosal_list_head            m_list;

   // Head of the list of AAL devices created
   kosal_list_head            m_devlisthead;

   // Head of the list of ports devices
   kosal_list_head            m_portlisthead;

   int                        m_isVF;
   int                        m_numVFs;
   int                        m_maxVFs;

   struct fme_device         *m_pfme_dev;       // FME Device

   kosal_pci_dev            *m_pcidev;         // Linux pci_dev pointer

   btUnsignedInt              m_flags;

   // Private semaphore
   kosal_semaphore            m_sem;

   int                        m_simulated;

   enum aal_bus_types_e       m_bustype;
   btUnsigned32bitInt         m_busNum;
   btUnsigned16bitInt         m_devicenum;      // device number
   btUnsigned16bitInt         m_functnum;       // function number

   btInt                      m_resources;      // Bit mask indicating bars that have been reserved

   // FME MMIO Space
   btVirtAddr                 m_kvp_fme_mmio;   // kv address of MMIO space
   btPhysAddr                 m_phys_fme_mmio;  // Physical address of MMIO space
   size_t                     m_len_fme_mmio;   // Bytes

   btUnsigned64bitInt         m_num_ports;
   btVirtAddr                 m_kvp_port_mmio[5];   // kv address of MMIO space
   btPhysAddr                 m_phys_port_mmio[5];  // Physical address of MMIO space
   size_t                     m_len_port_mmio[5];

   // AFU MMIO Space
   btVirtAddr                 m_kvp_afu_mmio;   // kv address of MMIO space
   btPhysAddr                 m_phys_afu_mmio;  // Physical address of MMIO space
   size_t                     m_len_afu_mmio;   // Bytes


}; // end struct ccip_afu_device

#define pci_dev_to_ccip_dev(ptr)             ccip_container_of(ptr, kosal_pci_dev, m_pcidev, struct ccip_device)
#define ccip_dev_to_pci_dev(pdev)            ((pdev)->m_pcidev)
#define ccip_dev_to_aaldev(pdev)             ((pdev)->m_aaldev)
#define ccip_dev_to_fme_dev(pdev)            ((pdev)->m_pfme_dev)
#define ccip_dev_to_port_dev(pdev)           ((pdev)->m_pfme_dev)

#define ccip_dev_pci_dev(pdev)               ((pdev)->m_pcidev)

#define cci_aaldev_board_type(pdev)             ((pdev)->m_boardtype)

#define ccip_set_simulated(pdev)             ((pdev)->m_simulated = 1)
#define ccip_clr_simulated(pdev)             ((pdev)->m_simulated = 0)
#define ccip_is_simulated(pdev)              ((pdev)->m_simulated == 1)

#define ccip_set_VFdev(pdev)                 ((pdev)->m_isVF = 1)
#define ccip_is_VFdev(pdev)                  ((pdev)->m_isVF == 1)

#define ccip_set_resource(pdev,r)            ((pdev)->m_resources |= (1<<r))
#define ccip_has_resource(pdev,r)            ((pdev)->m_resources & (1<<r))
#define ccip_clr_resource(pdev,r)            ((pdev)->m_resources &= ~(1<<r))

#define ccip_list_to_ccip_device(plist)      kosal_list_entry(plist, struct ccip_device, m_list)
#define aaldev_to_ccip_device(plist)         kosal_list_entry(plist, struct ccip_device, m_list)
#define ccip_dev_to_PIPsessionp(pdev)        ((pdev)->m_pPIPSession)
#define ccip_dev_psem(pdev)                  (&(pdev)->m_sem)

#define ccip_dev_list_head(pdev)             ((pdev)->m_list)
#define ccip_aal_dev_list(pdev)              ((pdev)->m_devlisthead)
#define ccip_port_dev_list(pdev)             ((pdev)->m_portlisthead)

#define ccip_fmedev_phys_afu_mmio(pdev)      ((pdev)->m_phys_fme_mmio)
#define ccip_fmedev_kvp_afu_mmio(pdev)       ((pdev)->m_kvp_fme_mmio)
#define ccip_fmedev_len_afu_mmio(pdev)       ((pdev)->m_len_fme_mmio)

#define ccip_portdev_numports(pdev)          ((pdev)->m_num_ports)
#define ccip_portdev_phys_afu_mmio(pdev,n)   ((pdev)->m_phys_port_mmio[n])
#define ccip_portdev_kvp_afu_mmio(pdev,n)    ((pdev)->m_kvp_port_mmio[n])
#define ccip_portdev_len_afu_mmio(pdev,n)    ((pdev)->m_len_port_mmio[n])

#define ccip_portdev_maxVFs(pdev)            ((pdev)->m_maxVFs)
#define ccip_portdev_numVFs(pdev)            ((pdev)->m_numVFs)

#define ccip_dev_pcie_bustype(pdev)          ((pdev)->m_bustype)
#define ccip_dev_pcie_busnum(pdev)           ((pdev)->m_busNum)
#define ccip_dev_pcie_devnum(pdev)           ((pdev)->m_devicenum)
#define ccip_dev_pcie_fcnnum(pdev)           ((pdev)->m_functnum)

/// @brief   Writes 64 bit control and status registers.
///
/// @param[in]  baseAddress   base CSR address.
/// @param[in]  offset        offset of CSR  .
/// @param[in]  value    value  going to be write in CSR.
/// @return   void
int write_ccip_csr64(btVirtAddr baseAddress, btUnsigned64bitInt offset,bt64bitCSR value);

/// @brief   read 64 bit control and status registers.
///
/// @param[in]  baseAddress   base CSR address.
/// @param[in]  offset        offset of CSR  .
/// @return    64 bit  CSR value
bt64bitCSR read_ccip_csr64( btVirtAddr baseAddress, btUnsigned64bitInt offset );


//=============================================================================
//=============================================================================
//                                PROTOTYPES
//=============================================================================
//=============================================================================
struct ccip_device;   // forward reference
struct port_device;

struct ccip_device * create_ccidevice(void);
void  destroy_ccidevice(struct ccip_device *pccidev);
extern btBool cci_fme_dev_create_AAL_allocatable_objects(struct ccip_device *);
btBool cci_port_dev_create_AAL_allocatable_objects(struct port_device  *,
                                                   btUnsigned32bitInt);
extern struct cci_aal_device* cci_create_aal_device(void);
extern int cci_destroy_aal_device( struct cci_aal_device*);
extern int cci_publish_aaldevice(struct cci_aal_device *);
extern void cci_unpublish_aaldevice(struct cci_aal_device *pcci_aaldev);
extern void cci_remove_device(struct ccip_device *);
extern void cci_release_device(pkosal_os_dev pdev);
extern void ccidrv_exitDriver(void);

extern struct ccidrv_session * ccidrv_session_create(btPID );
extern btInt ccidrv_session_destroy(struct ccidrv_session * );
extern struct aal_wsid *find_wsid( const struct ccidrv_session *,
                                   btWSID);
extern struct aal_wsid * ccidrv_valwsid(btWSID);
extern btInt ccidrv_freewsid(struct aal_wsid *pwsid);
extern struct aal_wsid* ccidrv_getwsid( struct aal_device *pdev,
                                        unsigned long long id);
extern btInt
ccidrv_sendevent( struct aaldev_ownerSession *,
                  struct aal_q_item *);

extern inline void GetCSR(btUnsigned64bitInt *ptr, bt32bitCSR *pcsrval);
extern inline void SetCSR(btUnsigned64bitInt *ptr, bt32bitCSR *csrval);
extern inline void Get64CSR(btUnsigned64bitInt *ptr, bt64bitCSR *pcsrval);
extern inline void Set64CSR(btUnsigned64bitInt *ptr, bt64bitCSR *csrval);


#endif // __AALKERNEL_CCI_PCIE_DRIVER_INTERNAL_H__

