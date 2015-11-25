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
//        FILE: cci_pcie_driver_internal.h
//     CREATED: 10/14/2015
//      AUTHOR: Joseph Grecco, Intel <joe.grecco@intel.com>
//
// PURPOSE: Internal private definitions and constants for the Intel(R)
//          Intel QuickAssist Technology CCI PCIe Device Driver.
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
//****************************************************************************
#ifndef __AALKERNEL_CCI_PCIE_DRIVER_INTERNAL_H__
#define __AALKERNEL_CCI_PCIE_DRIVER_INTERNAL_H__
#include "aalsdk/kernel/kosal.h"

#if 0
#include "aalsdk/kernel/aalbus-device.h"

#include "aalsdk/kernel/aalui.h"
#include "aalsdk/kernel/aalui-events.h"
#include "aalsdk/kernel/aalmafu.h"
#include "aalsdk/kernel/vafu2defs.h"

#include "cciv4_PIPsession.h"
#endif

#include <aalsdk/kernel/aalqueue.h>
#include <aalsdk/kernel/iaaldevice.h>
#include <aalsdk/kernel/aalwsservice.h>

/////////////////////////////////////////////////////////////////////////////
#ifndef DRV_VERSION
# define DRV_VERSION          "EXPERIMENTAL VERSION"
#endif
#define DRV_DESCRIPTION       "Intel(R) AAL FPGA PCIe Device driver and CCI Physical Interface Protocol (PIP)"
#define DRV_AUTHOR            "Joseph Grecco <joe.grecco@intel.com>"
#define DRV_LICENSE           "Dual BSD/GPL"
#define DRV_COPYRIGHT         "Copyright (c) 2015 Intel Corporation"

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
   struct pci_dev            *m_pcidev;         // Linux pci_dev pointer (or NULL if manual)

   btUnsignedInt              m_flags;

   // Used for being added to the global list of devices.
   struct list_head           m_list;           // List itself

   // Private semaphore
   struct semaphore           m_sem;

   enum aal_bus_types_e       m_boardtype;

   int                        m_simulated;

   int                        m_protocolID;
#if 0
   // PCI configuration space parameters
   btVirtAddr                 m_kvp_config;     // kv address after iomap
   btPhysAddr                 m_phys_config;    // Physical mmio address
   size_t                     m_len_config;     // Bytes

   // CCI MMIO Config Space
   btVirtAddr                 m_kvp_cci_csr;    // kv address of CSR space
   btPhysAddr                 m_phys_cci_csr;   // Physical address of CSR space
   size_t                     m_len_cci_csr;    // Bytes
#endif
   // AFU MMIO Space
   btVirtAddr                 m_kvp_afu_mmio;   // kv address of MMIO space
   btPhysAddr                 m_phys_afu_mmio;  // Physical address of MMIO space
   size_t                     m_len_afu_mmio;   // Bytes

   // AFU uMSG Space
   btVirtAddr                 m_kvp_afu_umsg;    // kv address of CSR space
   btPhysAddr                 m_phys_afu_umsg;   // Physical address of CSR space
   size_t                     m_len_afu_umsg;    // Bytes

   struct cci_PIPsession     *m_pPIPSession;     // PIP session object

   enum   cci_devtype         m_devtype;        // Type of the subclass (e.g., FME, PORT, AFU)
   struct fme_device         *m_pfme;
   struct port_device        *m_pport;
//   struct afu_device         *m_pafu;

};


#define pci_dev_to_cci_dev(ptr)              kosal_container_of(ptr, struct pci_dev, m_pcidev, struct cci_aal_device)
#define aaldev_to_cci_aal_device(ptr)        kosal_container_of(ptr, struct aal_device, m_aaldev)

#define cci_dev_pfme(pdev)                  ( pdev->m_pfme )
#define cci_dev_pport(pdev)                 ( pdev->m_pport )
#define cci_dev_pafu(pdev)                  ( pdev->m_pafu )

#define cci_dev_pci_dev(pdev)               ((pdev)->m_pcidev)
   #define cci_dev_pci_dev_is_enabled(pdev)  ((pdev)->m_flags & CCI_DEV_FLAG_PCI_DEV_ENABLED)
   #define cci_dev_pci_dev_set_enabled(pdev) ((pdev)->m_flags |= CCI_DEV_FLAG_PCI_DEV_ENABLED)
   #define cci_dev_pci_dev_clr_enabled(pdev) ((pdev)->m_flags &= ~CCI_DEV_FLAG_PCI_DEV_ENABLED)

   #define cci_dev_pci_dev_is_region_requested(pdev)  ((pdev)->m_flags & CCI_DEV_FLAG_PCI_REGION_REQUESTED)
   #define cci_dev_pci_dev_set_region_requested(pdev) ((pdev)->m_flags |= CCI_DEV_FLAG_PCI_REGION_REQUESTED)
   #define cci_dev_pci_dev_clr_region_requested(pdev) ((pdev)->m_flags &= ~CCI_DEV_FLAG_PCI_REGION_REQUESTED)

   #define cci_dev_allow_map_csr_read_space(pdev)     ((pdev)->m_flags & CCI_DEV_FLAG_ALLOW_MAP_CSR_READ_SPACE)
   #define cci_dev_set_allow_map_csr_read_space(pdev) ((pdev)->m_flags |= CCI_DEV_FLAG_ALLOW_MAP_CSR_READ_SPACE)
   #define cci_dev_clr_allow_map_csr_read_space(pdev) ((pdev)->m_flags &= ~CCI_DEV_FLAG_ALLOW_MAP_CSR_READ_SPACE)

   #define cci_dev_allow_map_csr_write_space(pdev)     ((pdev)->m_flags & CCI_DEV_FLAG_ALLOW_MAP_CSR_WRITE_SPACE)
   #define cci_dev_set_allow_map_csr_write_space(pdev) ((pdev)->m_flags |= CCI_DEV_FLAG_ALLOW_MAP_CSR_WRITE_SPACE)
   #define cci_dev_clr_allow_map_csr_write_space(pdev) ((pdev)->m_flags &= ~CCI_DEV_FLAG_ALLOW_MAP_CSR_WRITE_SPACE)

   #define cci_dev_allow_map_csr_space(pdev) ( cci_dev_allow_map_csr_read_space(pdev) || cci_dev_allow_map_csr_write_space(pdev) )

   #define cci_dev_allow_map_mmior_space(pdev)     ((pdev)->m_flags & CCI_DEV_FLAG_ALLOW_MAP_MMIOR_SPACE)
   #define cci_dev_set_allow_map_mmior_space(pdev) ((pdev)->m_flags |= CCI_DEV_FLAG_ALLOW_MAP_MMIOR_SPACE)
   #define cci_dev_clr_allow_map_mmior_space(pdev) ((pdev)->m_flags &= ~CCI_DEV_FLAG_ALLOW_MAP_MMIOR_SPACE)

   #define cci_dev_allow_map_umsg_space(pdev)     ((pdev)->m_flags & CCI_DEV_FLAG_ALLOW_MAP_UMSG_SPACE)
   #define cci_dev_set_allow_map_umsg_space(pdev) ((pdev)->m_flags |= CCI_DEV_FLAG_ALLOW_MAP_UMSG_SPACE)
   #define cci_dev_clr_allow_map_umsg_space(pdev) ((pdev)->m_flags &= ~CCI_DEV_FLAG_ALLOW_MAP_UMSG_SPACE)

   #define cci_dev_is_simulated(pdev)  ((pdev)->m_flags & CCI_DEV_FLAG_SIMULATED_DEV)
   #define cci_dev_set_simulated(pdev) ((pdev)->m_flags |= CCI_DEV_FLAG_SIMULATED_DEV)
   #define cci_dev_clr_simulated(pdev) ((pdev)->m_flags &= ~CCI_DEV_FLAG_SIMULATED_DEV)

#define cci_dev_type(pdev)                  ((pdev)->m_devtype)

#define cci_dev_board_type(pdev)            ((pdev)->m_boardtype)

#define cci_set_simulated(pdev)             ((pdev)->m_simulated = 1)
#define cci_clr_simulated(pdev)             ((pdev)->m_simulated = 0)
#define cci_is_simulated(pdev)             ((pdev)->m_simulated == 1)

#define cci_dev_protocol(pdev)              ((pdev)->m_protocolID)

#define cci_dev_phys_afu_mmio(pdev)         ((pdev)->m_phys_afu_mmio)
#define cci_dev_kvp_afu_mmio(pdev)          ((pdev)->m_kvp_afu_mmio)
#define cci_dev_len_afu_mmio(pdev)          ((pdev)->m_len_afu_mmio)

#define cci_dev_phys_afu_umsg(pdev)         ((pdev)->m_phys_afu_umsg)
#define cci_dev_kvp_afu_umsg(pdev)          ((pdev)->m_kvp_afu_umsg)
#define cci_dev_len_afu_umsg(pdev)          ((pdev)->m_len_afu_umsg)


#define cci_dev_to_pci_dev(pdev)            ((pdev)->m_pcidev)
#define cci_aaldev_to_aaldev(pdev)          ((pdev)->m_aaldev)


#define cci_dev_list_head(pdev)             ((pdev)->m_list)
#define cci_list_to_cci_aal_device(plist)    kosal_list_entry(plist, struct cci_aal_device, m_list)

#define cci_dev_to_PIPsessionp(pdev)        ((pdev)->m_pPIPSession)
#define cci_dev_psem(pdev)                  (&(pdev)->m_sem)



//=============================================================================
//=============================================================================
//                                INLINE PRIMITIVES
//=============================================================================
//=============================================================================

//=============================================================================
// Name: cci_getBARAddress
// Description: Called during the device probe by cci_pci_probe
//                  when the device id matches PCI_DEVICE_ID_PCIFPGA.
// Interface: public
// Inputs:  ppcidev - Pointer to PICe device
//          pphysaddr - Pointer to where to return the physical address
//          pvirtaddr - Pointer to where to return the mapped virtual address
//          psize - BAR region size
// Outputs: 1 = success.
// Comments:
//=============================================================================
static inline int cci_getBARAddress( struct pci_dev   *ppcidev,
                                     int               barnum,
                                     btPhysAddr       *pphysaddr,
                                     btVirtAddr       *pvirtaddr,
                                     size_t           *psize)
{
   if ( 0 == pci_request_region(ppcidev, barnum, CCI_PCI_DRIVER_NAME) ) {
      // get the low base address register.
      *pphysaddr = pci_resource_start(ppcidev, barnum);
      *psize  = (size_t)pci_resource_len(ppcidev, barnum);

      PVERBOSE("BAR=%d phy Address : %" PRIxPHYS_ADDR "\n",barnum, *pphysaddr);
      PVERBOSE("BAR=%d size : %zd\n",barnum, *psize);

   }else{
      PERR("Failed to obtian PCI BAR=%d \"%s\". Using Bar 0.\n", barnum, CCI_PCI_DRIVER_NAME);
      return 0;
   }

   // Only non-zero regions make sense
   if((0 == *pphysaddr) || (0 == *psize)){
      pci_release_region(ppcidev, barnum);
      return 0;
   }
   // Get the KVP for the region
   *pvirtaddr = ioremap_nocache(*pphysaddr, *psize);
   return 1;
}

//=============================================================================
//=============================================================================
//                                PROTOTYPES
//=============================================================================
//=============================================================================
struct ccip_device;   // forward reference
struct port_device;

extern int ccidrv_initDriver(void/*callback*/);
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
extern void cci_release_device(struct device *pdev);
extern void ccidrv_exitDriver(void);

extern int ccidrv_initUMAPI(void);
void ccidrv_exitUMAPI(void);

extern struct ccidrv_session * ccidrv_session_create(btPID );
extern btInt ccidrv_session_destroy(struct ccidrv_session * );
extern struct aal_wsid *find_wsid( const struct ccidrv_session *,
                                   btWSID);
extern struct aal_wsid * ccidrv_valwsid(btWSID);
extern btInt ccidrv_freewsid(struct aal_wsid *pwsid);
extern struct aal_wsid* ccidrv_getwsid( struct aal_device *pdev,
                                        unsigned long long id);
extern btInt
ccidrv_sendevent( btObjectType,
                  struct aal_device *,
                  struct aal_q_item *,
                  btObjectType);


#if 0
void cci_release_device( struct device *pdev );

extern int
cci_sim_mmap(struct aaldev_ownerSession* pownerSess,
               struct aal_wsid *wsidp,
               btAny os_specific);

extern int
cci_publish_aaldevice(struct cci_aal_device *);



extern int
cci_destroy_device(struct cci_aal_device* );

extern void
cci_remove_device(struct cci_aal_device *);


extern void
cci_flush_all_wsids(struct cci_PIPsession *psess);
#endif

#endif // __AALKERNEL_CCI_PCIE_DRIVER_INTERNAL_H__

