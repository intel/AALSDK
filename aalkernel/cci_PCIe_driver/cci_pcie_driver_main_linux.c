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
//        FILE: cci_pcie_driver_main.c
//     CREATED: 10/14/2015
//      AUTHOR: Joseph Grecco, Intel <joe.grecco@intel.com>
// PURPOSE: This file implements init/exit entry points for the
//          AAL FPGA device driver for
//          functionality of the AAL FPGA device driver.
// HISTORY:
// COMMENTS: Linux specific
// WHEN:          WHO:     WHAT:
// 10/14/2015     JG       Initial version started
//****************************************************************************
#include "aalsdk/kernel/kosal.h"

#define MODULE_FLAGS CCIPCIE_DBG_MOD // Prints all

#include "aalsdk/kernel/aalbus.h"
#include "aalsdk/kernel/aalinterface.h"
#include "aalsdk/kernel/ccip_defs.h"

#include "cci_pcie_driver_internal.h"

#include "cci_pcie_driver_simulator.h"

#include "ccip_fme.h"
#include "ccip_port.h"
#include "ccip_perfmon_linux.h"
#include "ccip_logging.h"
#include "ccip_logging_linux.h"

extern int print_sim_fme_device(struct fme_device *);
extern int print_sim_port_device(struct port_device *pport_dev);

/// g_device_list - Global device list for this module.
kosal_list_head g_device_list;

/// g_dev_list_sem - Global device list semaphore.
kosal_semaphore g_dev_list_sem;

static btBool isPFDriver = 0;


//////////////////////////////////////////////////////////////////////////////////////

MODULE_VERSION    (DRV_VERSION);
MODULE_DESCRIPTION(DRV_DESCRIPTION);
MODULE_AUTHOR     (DRV_AUTHOR);
MODULE_LICENSE    (DRV_LICENSE);

//=============================================================================
// Driver Parameters
//=============================================================================
// Set up configuration parameter for insmod command-line:
// Argument:
//      sim: Instantiate simualted AFUs
//       Value: Number of AFUs to instantiate
//      sriov: Activate SR-IOV
//       Value: 1 - activate
//
// Typical usage:
//    sudo insmod ccidrv           # Normal load. PCIe enumeration enabled
//    sudo insmod ccidrv sim=4     # Instantiate 4 simulated AFUs
//    sudo insmod ccidrv sriov=1   # Activate SR-IOV

unsigned long  sim = 0;
MODULE_PARM_DESC(sim, "Simulation: #=Number of simulated AFUs to instantiate");
module_param    (sim, ulong, S_IRUGO);

unsigned long  sriov = 0;
MODULE_PARM_DESC(sriov, "SR-IOV");
module_param    (sriov, ulong, S_IRUGO);

////////////////////////////////////////////////////////////////////////////////

//=============================================================================
//             D E B U G   F L A G S
//
// kern-utils.h macros expects DRV_NAME and debug declarations
// DRV_NAME is defined in mem-int.h
//=============================================================================

btUnsignedInt debug = 0
#if 0
/* Type and Level selection flags */
   | PTRACE_FLAG
   | PVERBOSE_FLAG
   | PDEBUG_FLAG
   | PINFO_FLAG
   | PNOTICE_FLAG
/* Module selection flags */
   | CCIPCIE_DBG_MOD
   | CCIPCIE_DBG_DEV
   | CCIPCIE_DBG_AFU
   | CCIPCIE_DBG_MAFU
   | CCIPCIE_DBG_MMAP
   | CCIPCIE_DBG_CMD
   | CCIPCIE_DBG_CFG
#endif
;

/******************************************************************************
 * Debug parameter global definition
 */
MODULE_PARM_DESC(debug, "module debug level");
module_param    (debug, int, 0644);


//=============================================================================
// Name: aalbus_attrib_show_debug
// Decription: Accessor for the debug parameter. Called when the sys fs
//             parameter is read.
//=============================================================================
static ssize_t ahmpip_attrib_show_debug(struct device_driver *drv, char *buf)
{
   return (snprintf(buf,PAGE_SIZE,"%x\n",debug));
}

//=============================================================================
// Name: aalbus_attrib_store_debug
// Decription: Mutator for the debug parameter. Called when the sys fs
//             parameter is written.
//=============================================================================
static ssize_t ahmpip_attrib_store_debug(struct device_driver *drv,
                                         const char *buf,
                                         size_t size)
{
   int temp = 0;
   sscanf(buf,"%x", &temp);

   debug = temp;

   printk(KERN_INFO DRV_NAME ": Attribute change - debug = %d\n", temp);
   return size;
}

// Attributes for debug
DRIVER_ATTR(debug,S_IRUGO|S_IWUSR|S_IWGRP, ahmpip_attrib_show_debug,ahmpip_attrib_store_debug);

//////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////              PCIE INTERFACES             ////////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//=============================================================================
// Prototypes
//=============================================================================
static int cci_pci_probe(struct pci_dev * , const struct pci_device_id * );
static void cci_pci_remove(struct pci_dev * );
#if 0
static
struct ccip_device *
cci_pcie_stub_probe( struct pci_dev             *pcidev,
                     const struct pci_device_id *pcidevid);
#endif
static
struct ccip_device *
cci_enumerate_device( struct pci_dev             *pcidev,
                     const struct pci_device_id *pcidevid);

static
struct ccip_device *
cci_enumerate_vf_device( struct pci_dev             *pcidev,
                         const struct pci_device_id *pcidevid);
static int cci_pci_sriov_configure(struct pci_dev *, int);

static int  ccidrv_init(void);
static void ccidrv_exit(void);

module_init(ccidrv_init);
module_exit(ccidrv_exit);

extern int ccidrv_initDriver(void/*callback*/);
extern int ccidrv_initUMAPI(void);
void ccidrv_exitUMAPI(void);


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////            PCIE INTEGRATION               ///////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
//=============================================================================

///=================================================================
/// cci_pci_id_tbl - identify PCI devices supported by this driver
///=================================================================
static struct pci_device_id cci_pcie_id_tbl[] = {
   { PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCIe_DEVICE_ID_RCiEP0),          .driver_data = (kernel_ulong_t)cci_enumerate_device },
   { PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCIe_DEVICE_ID_VF),              .driver_data = (kernel_ulong_t)cci_enumerate_vf_device },
   { PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCIe_DEVICE_ID_RCiEP1),          .driver_data = (kernel_ulong_t)0 },
   { PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCIe_DEVICE_ID_RCiEP2),          .driver_data = (kernel_ulong_t)0 },
   { PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCIe_DEVICE_ID_RCiEP0_SKX_P),    .driver_data = (kernel_ulong_t)cci_enumerate_device },
   { PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCIe_DEVICE_ID_VF_SKX_P),        .driver_data = (kernel_ulong_t)cci_enumerate_vf_device },
   { 0, }
};
CASSERT(sizeof(void *) == sizeof(kernel_ulong_t));

MODULE_DEVICE_TABLE(pci, cci_pcie_id_tbl);

//=============================================================================
// Name: cci_pcie_driver_info
// Description: This struct represents the PCIe driver instance.
//              The Driver object is registered with the Linux PCI
//              subsystem.
//=============================================================================
struct cci_pcie_driver_info
{
   int                        isregistered;  // boolean : did we register with PCIe subsystem?
   struct pci_driver          pcidrv;        //  Linux PCIe driver structure
};

// Instantiate the structure
static struct cci_pcie_driver_info driver_info = {
   .isregistered = 0,
   .pcidrv = {
      .name             = CCI_PCI_DRIVER_NAME,
      .id_table         = cci_pcie_id_tbl,
      .probe            = cci_pci_probe,
      .remove           = cci_pci_remove,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,8,0)
      .sriov_configure  = cci_pci_sriov_configure
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(3,8,0) */
   },
};

/// Signature of CCI device specific probe function
typedef struct ccip_device * (*cci_probe_fn)( struct pci_dev                * ,
                                              const struct pci_device_id    * );

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
#if 0
//=============================================================================
// Name: cci_pcie_stub_probe
// Description: Called if either the PCIe_DEVICE_ID_RCiEP1 or PCIe_DEVICE_ID_RCiEP2
//              devices are detected. These devices have no visible host interface.
// Interface: public
// Inputs: pcidev - kernel-provided device pointer.
//         pcidevid - kernel-provided device id pointer.
// Outputs: Pointer to populated CCI device or NULL if failure
// Comments:
//=============================================================================
static
struct ccip_device * cci_pcie_stub_probe( struct pci_dev             *pcidev,
                                          const struct pci_device_id *pcidevid)
{

   ASSERT(pcidev);
   if ( NULL == pcidev ) {
      PTRACEOUT_INT(-EINVAL);
      return NULL;
   }

   ASSERT(pcidevid);
   if ( NULL == pcidevid ) {
      PTRACEOUT_INT(-EINVAL);
      return NULL;
   }
   switch(pcidevid->device)
   {
      case PCIe_DEVICE_ID_RCiEP1:
         PVERBOSE("PCIe RCiEP 1 ignored\n");
         break;
      case PCIe_DEVICE_ID_RCiEP2:
         PVERBOSE("PCIe RCiEP 2 ignored\n");
         break;
      default:
         PVERBOSE("Unknown device ID ignored\n");
         break;
   }
   return (struct ccip_device *) (-1);
}
#endif
//=============================================================================
// Name: cci_pci_probe
// Description: Called during the device probe by PCIe subsystem.
// Interface: public
// Inputs: pcidev - kernel-provided device pointer.
//         pcidevid - kernel-provided device id pointer.
// Outputs: 0 = success.
// Comments:
//=============================================================================
static
int
cci_pci_probe( struct pci_dev             *pcidev,
               const struct pci_device_id *pcidevid)
{

   int res = -EINVAL;
   cci_probe_fn  probe_fn;
   struct ccip_device      *pccidev = NULL;

   PTRACEIN;

   // Validate parameters
   ASSERT(pcidev);
   if ( NULL == pcidev ) {
      res = -EINVAL;
      PTRACEOUT_INT(res);
      return res;
   }

   ASSERT(pcidevid);
   if ( NULL == pcidevid ) {
      res = -EINVAL;
      PTRACEOUT_INT(res);
      return res;
   }

   // Display device information
   PVERBOSE("Discovered FPGA Device:");
   PVERBOSE("-VenderID %x  \n",pcidevid->vendor );
   PVERBOSE("-DeviceID  %x  \n",pcidevid->device );
   PVERBOSE("-B:D.F = %x:%x.%x  \n",pcidev->bus->number,PCI_SLOT(pcidev->devfn),PCI_FUNC(pcidev->devfn) );

   // Get the device specific probe function
   probe_fn = (cci_probe_fn)pcidevid->driver_data;
   if ( NULL == probe_fn ) {
       PVERBOSE("Ignoring hidden PCIe devcies\n");
       return 0;
   }

   // Call the probe function.  This is where the real work occurs
   pccidev = probe_fn( pcidev, pcidevid );
   ASSERT(pccidev);

   // If all went well record this device on our
   //  list of devices owned by the driver
   if(NULL != pccidev){
      kosal_list_add(&(pccidev->m_list), &g_device_list);
      res = 0;
   }
   PTRACEOUT_INT(res);
   return res;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////        BOARD DEVICE BAR ENUMERATION       ///////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
//=============================================================================
//=============================================================================
// Name: cci_pci_sriov_configure
// Description: Callback used during SRIOV device initialization
// Inputs: pcidev - kernel-provided device pointer.
//         num_vfs - Number of VFs to enable.
// Returns: 0 = success.
// Comments:
//=============================================================================
static int cci_pci_sriov_configure(struct pci_dev *pcidev, int num_vfs)
{

   // for now, we do not support configuring sr_iov through sysfs, because it
   // requires more complex deactivation and handover of AFUs between PF and
   // VF. Instead, VF capability is configured via a load parameter (vf).
   /*
   if (num_vfs == 0){
      // transfer AFU ownership back to PF
      // TODO
      // disable SR-IOV
      pci_disable_sriov(pcidev);
      PINFO("SRIOV disabled on this device\n");
   }else{
      // enable SR-IOV
      if (0 == pci_enable_sriov(pcidev, num_vfs)){
         PINFO("SRIOV Enabled on this device for %d VF%s\n",num_vfs, (num_vfs >1 ? "s" : ""));
         // transfer AFU ownership to VF
         // TODO
      }else{
         PINFO("Failed to enable SRIOV");
      }
   }
   return 0;
   */
   return 1;
}

//=============================================================================
// Name: cci_enumerate_vf_device
// Description: Called during the device probe to initiate the enumeration of
//              the Virtual Function device attributes and construct the
//              internal objects.
// Inputs: pcidev - kernel-provided device pointer.
//         pcidevid - kernel-provided device id pointer.
// Returns: 0 = success.
// Comments:
//=============================================================================
static
struct ccip_device * cci_enumerate_vf_device( struct pci_dev             *pcidev,
                                              const struct pci_device_id *pcidevid)
{

   struct ccip_device *pccipdev        = NULL;
   struct port_device *pportdev        = NULL;
   int                 res             = 0;

   PTRACEIN;

   // Check arguments
   ASSERT(pcidev);
   if ( NULL == pcidev ) {
      PTRACEOUT_INT(-EINVAL);
      return NULL;
   }
   ASSERT(pcidevid);
   if ( NULL == pcidevid ) {
      PTRACEOUT_INT(-EINVAL);
      return NULL;
    }

   // Setup the PCIe device
   //----------------------
   res = pci_enable_device(pcidev);
   if ( res < 0 ) {
      PERR("Failed to enable device res=%d\n", res);
      PTRACEOUT_INT(res);
      return NULL;
   }

   // enable PCIe error reporting
   pci_enable_pcie_error_reporting(pcidev);

   // enable bus mastering and configure DMA
   pci_set_master(pcidev);
   pci_save_state(pcidev);

   if (!dma_set_mask(&pcidev->dev, DMA_BIT_MASK(64))) {
      dma_set_coherent_mask(&pcidev->dev, DMA_BIT_MASK(64));
   } else if (!dma_set_mask(&pcidev->dev, DMA_BIT_MASK(32))) {
      dma_set_coherent_mask(&pcidev->dev, DMA_BIT_MASK(32));
   } else {
      PERR("No suitable DMA support available.\n");
      goto ERR;
   }


   // If we are on the host node (aka Dom0) then we do not publish VF objects
   if(isPFDriver){
      PINFO("VF Device detected on PF Driver.\nIgnoring.\n");
      return NULL;
   }

   // Create the CCI device object
   //  Allocate a new CCI board device object
   //  and populate it with its resource information
   //----------------------------------------------
   pccipdev = create_ccidevice();

   // Save the PCI device in the CCI object
   ccip_dev_pci_dev(pccipdev) = pcidev;

   // Acquire the BAR region
   //  64 Bit BARs are actually spread
   //  across 2 consecutive BARs. So we
   //  use 0 and 2 rather than 0 and 1
   //------------------------------------
   {
      btPhysAddr           pbarPhyAddr    = 0;
      btVirtAddr           pbarVirtAddr   = 0;
      size_t               barsize        = 0;


      // As this is a VF we only expect 1 BAR for the Port
      if( !cci_getBARAddress(  pcidev,
                               0,
                              &pbarPhyAddr,
                              &pbarVirtAddr,
                              &barsize) ){
         goto ERR;
      }

      // Save the BAR information in the CCI Device object. For a VF it's a Port
      ccip_portdev_phys_afu_mmio(pccipdev,0)    = __PHYS_ADDR_CAST(pbarPhyAddr);
      ccip_portdev_kvp_afu_mmio(pccipdev,0)     = pbarVirtAddr;
      ccip_portdev_len_afu_mmio(pccipdev,0)     = barsize;

      PDEBUG("ccip_portdev_phys_afu_mmio(pccipdev) : %lx\n", ccip_portdev_phys_afu_mmio(pccipdev,0));
      PDEBUG("ccip_portdev_kvp_afu_mmio(pccipdev) : %lx\n",(long unsigned int) ccip_portdev_kvp_afu_mmio(pccipdev,0));
      PDEBUG("ccip_portdev_len_afu_mmio(pccipdev): %zu\n", ccip_portdev_len_afu_mmio(pccipdev,0));

   }

   // Save the Bus:Device:Function of PCIe device
   ccip_dev_pcie_bustype(pccipdev)  = aal_bustype_PCIe;
   ccip_dev_pcie_busnum(pccipdev)   = pcidev->bus->number;
   ccip_dev_pcie_devnum(pccipdev)   = PCI_SLOT(pcidev->devfn);
   ccip_dev_pcie_fcnnum(pccipdev)   = PCI_FUNC(pcidev->devfn);

   // Enumerate the device
   //  Instantiate internal objects. Objects that represent
   //  objects that can be allocated through the AALBus are
   //  constructed around the aaldevice base and are published
   //  with aalbus.
   //---------------------------------------------------------

   //In a VF there should be no FME region
   if(0 != ccip_fmedev_kvp_afu_mmio(pccipdev)){
      PERR("Invalid region [FME]\n");
      goto ERR;
   }


   // Port Device initialization
   //  The resources may reside in different Bars and at different offsets.
   //  The driver must keep track of all resources it claims so it can
   //  free them later.
   //----------------------------------------------------------------------


   // Discover and create Port device
   //   Enumerates the Port feature list, creates the Port object.
   //   Then add the new port object onto the list
   //-------------------------------------------------------------
   pportdev = get_port_device( ccip_portdev_phys_afu_mmio(pccipdev,0),
                               ccip_portdev_kvp_afu_mmio(pccipdev,0),
                               ccip_portdev_len_afu_mmio(pccipdev,0));

   if ( NULL == pportdev ) {
      PERR("Could not allocate memory for FME object\n");
      res = -ENOMEM;
      goto ERR;
   }

   // Record the resource
   ccip_set_resource(pccipdev, 0);

   PDEBUG("Created Port Device\n");
   port_afu_Enable(pportdev);

   // Point to our parent
   ccip_port_to_ccidev(pportdev) = pccipdev;

   // Inherits B:D:F from board
   ccip_port_bustype(pportdev)   = ccip_dev_pcie_bustype(pccipdev);
   ccip_port_busnum(pportdev)    = ccip_dev_pcie_busnum(pccipdev);
   ccip_port_devnum(pportdev)    = ccip_dev_pcie_devnum(pccipdev);
   ccip_port_fcnnum(pportdev)    = ccip_dev_pcie_fcnnum(pccipdev);

   // Log the Port MMIO
   print_sim_port_device(pportdev);

   PDEBUG("Adding to list\n");

   // Added it to the port list
   kosal_list_add(&ccip_port_dev_list(pccipdev), &ccip_port_list_head(pportdev));

   // No access to FME for VF
   ccip_port_dev_fme(pportdev) = NULL;

   PDEBUG("Creating Allocatable objects\n");

   // Instantiate allocatable objects including AFUs if present.
   //   Subdevice addresses start at 10x the 1 based port number to leave room for
   //   10 devices beneath the port. E.e., STAP, PR, User AFU
   if(!cci_port_dev_create_AAL_allocatable_objects(pportdev, 0) ){
      goto ERR;
   }
   ccip_set_VFdev(pccipdev);


   // Start logging timer
   start_logging_timer();

   return pccipdev;
ERR:
{

  if( NULL != ccip_portdev_kvp_afu_mmio(pccipdev,0)) {
     PVERBOSE("Freeing Port BAR 0\n");
     iounmap(ccip_fmedev_kvp_afu_mmio(pccipdev));
     pci_release_region(pcidev, 0);
  }

  if ( NULL != pccipdev ) {
     kfree(pccipdev);
  }
}

   PTRACEOUT_INT(res);
   return NULL;
}

//=============================================================================
// Name: cci_enumerate_device
// Description: Called during the device probe to initiate the enumeration of
//              the device attributes and construct the internal objects.
// Inputs: pcidev - kernel-provided device pointer.
//         pcidevid - kernel-provided device id pointer.
// Returns: 0 = success.
// Comments:
//=============================================================================
static
struct ccip_device * cci_enumerate_device( struct pci_dev             *pcidev,
                                           const struct pci_device_id *pcidevid)
{

   struct ccip_device * pccipdev       = NULL;

   int                  res            = 0;

   PTRACEIN;

   // Check arguments
   ASSERT(pcidev);
   if ( NULL == pcidev ) {
      PTRACEOUT_INT(-EINVAL);
      return NULL;
   }
   ASSERT(pcidevid);
   if ( NULL == pcidevid ) {
      PTRACEOUT_INT(-EINVAL);
      return NULL;
    }

   // This is a PF driver
   isPFDriver = 1;

   // Setup the PCIe device
   //----------------------
   res = pci_enable_device(pcidev);
   if ( res < 0 ) {
      PERR("Failed to enable device res=%d\n", res);
      PTRACEOUT_INT(res);
      return NULL;
   }

   // enable PCIe error reporting
   pci_enable_pcie_error_reporting(pcidev);

   // enable bus mastering and configure DMA
   pci_set_master(pcidev);
   pci_save_state(pcidev);
   if (!dma_set_mask(&pcidev->dev, DMA_BIT_MASK(64))) {
      dma_set_coherent_mask(&pcidev->dev, DMA_BIT_MASK(64));
   } else if (!dma_set_mask(&pcidev->dev, DMA_BIT_MASK(32))) {
      dma_set_coherent_mask(&pcidev->dev, DMA_BIT_MASK(32));
   } else {
      PERR("No suitable DMA support available.\n");
      goto ERR;
   }

   // Create the CCI device object
   //  Allocate a new CCI board device object
   //  and populate it with its reource information
   //----------------------------------------------
   pccipdev = create_ccidevice();

   // Save the PCI device in the CCI object
   ccip_dev_pci_dev(pccipdev) = pcidev;

   // Acquire the BAR regions
   //  64 Bit BARs are actually spread
   //  across 2 consecutive BARs. So we
   //  use 0 and 2 rather than 0 and 1
   //------------------------------------
   {
      btPhysAddr           pbarPhyAddr    = 0;
      btVirtAddr           pbarVirtAddr   = 0;
      size_t               barsize        = 0;

      // BAR 0  is the FME
      if( !cci_getBARAddress(  pcidev,
                               0,
                              &pbarPhyAddr,
                              &pbarVirtAddr,
                              &barsize) ){
         goto ERR;
      }



      // Save the BAR information in the CCI Device object
      ccip_fmedev_phys_afu_mmio(pccipdev)    = __PHYS_ADDR_CAST(pbarPhyAddr);
      ccip_fmedev_len_afu_mmio(pccipdev)     = barsize;
      ccip_fmedev_kvp_afu_mmio(pccipdev)     = pbarVirtAddr;

      PDEBUG("ccip_fmedev_phys_afu_mmio(pccipdev) : %lx\n", ccip_fmedev_phys_afu_mmio(pccipdev));
      PDEBUG("ccip_fmedev_kvp_afu_mmio(pccipdev) : %lx\n",(long unsigned int) ccip_fmedev_kvp_afu_mmio(pccipdev));
      PDEBUG("ccip_fmedev_len_afu_mmio(pccipdev): %zu\n", ccip_fmedev_len_afu_mmio(pccipdev));

   }

   // Save the Bus:Device:Function of PCIe device
   ccip_dev_pcie_bustype(pccipdev)      = aal_bustype_PCIe;
   ccip_dev_pcie_busnum(pccipdev)       = pcidev->bus->number;
   ccip_dev_pcie_devnum(pccipdev)       = PCI_SLOT(pcidev->devfn);
   ccip_dev_pcie_fcnnum(pccipdev)       = PCI_FUNC(pcidev->devfn);
   //ccip_dev_pcie_socketnum(pccipdev)    = dev_to_node(&pcidev->dev);

   //PINFO(" Socket ID = %x   \n",dev_to_node(&pcidev->dev));

   // Enumerate the device
   //  Instantiate internal objects. Objects that represent
   //  objects that can be allocated through the AALBus are
   //  constructed around the aaldevice base and are published
   //  with aalbus.
   //---------------------------------------------------------
   //FME region
   if(0 != ccip_fmedev_kvp_afu_mmio(pccipdev)){

      PINFO(" FME mmio region   \n");

      // Create the FME MMIO device object
      //   Enumerates the FME feature list
      //----------------------------------
      ccip_dev_to_fme_dev(pccipdev) = get_fme_mmio_dev(ccip_fmedev_kvp_afu_mmio(pccipdev) );
      if ( NULL == pccipdev->m_pfme_dev ) {
         PERR("Could not allocate memory for FME object\n");
         res = -ENOMEM;
         goto ERR;
      }

      // Instantiate AAL allocatable objects including AFUs if present
      if(!cci_fme_dev_create_AAL_allocatable_objects(pccipdev)){
         ccip_destroy_fme_mmio_dev(pccipdev->m_pfme_dev);
         goto ERR;
      }

#if 1
      // print fme CSRS
      print_sim_fme_device(pccipdev->m_pfme_dev);
#endif

      create_perfmonitor(pcidev,ccip_dev_to_fme_dev(pccipdev));
   }


   // Port Device initialization
   //  Loop through each Port Offset register to determine
   //  if a Port has been implemented and where its resources are.
   //  The resources may reside in different Bars and at different offsets.
   //  The driver must keep track of all resources it claims so it can
   //  free them later.
   //----------------------------------------------------------------------
   {
      struct fme_device  *pfme_dev  = ccip_dev_to_fme_dev(pccipdev);
      struct CCIP_FME_HDR *pfme_hdr = ccip_fme_hdr(pfme_dev);

      int i=0;

      for(i=0;  0!= pfme_hdr->port_offsets[i].port_imp  ;i++){
         btPhysAddr pbarPhyAddr        = 0;
         btUnsigned32bitInt bar        = pfme_hdr->port_offsets[i].port_bar;
         btUnsigned64bitInt offset     = pfme_hdr->port_offsets[i].port_offset;
         struct port_device *pportdev  = NULL;

         PINFO("***** PORT %d MMIO region @ Bar %d offset %x *****\n",i , bar, pfme_hdr->port_offsets[i].port_offset);

         // Check to see if the resource has already been acquired
         if(!ccip_has_resource(pccipdev, bar)){

            PVERBOSE("Getting resources from BAR %d\n", bar);

            // Get the bar
            if( !cci_getBARAddress(  pcidev,
                                     bar,
                                    &pbarPhyAddr,
                                    &ccip_portdev_kvp_afu_mmio(pccipdev,bar),
                                    &ccip_portdev_len_afu_mmio(pccipdev,bar)) ){
               goto ERR;
            }
            ccip_portdev_phys_afu_mmio(pccipdev,bar)   = __PHYS_ADDR_CAST(pbarPhyAddr);

            // Record the resource
            ccip_set_resource(pccipdev, bar);

            PDEBUG("Bar phys : %" PRIxPHYS_ADDR "\n", ccip_portdev_phys_afu_mmio(pccipdev,bar));
            PDEBUG("Virt : %p\n", ccip_portdev_kvp_afu_mmio(pccipdev,bar));
            PDEBUG("Len: %zu\n", ccip_portdev_len_afu_mmio(pccipdev,bar));
         }

         // Discover and create Port device
         //   Enumerates the Port feature list, creates the Port object.
         //   Then add the new port object onto the list
         //-------------------------------------------------------------
         pportdev = get_port_device( pbarPhyAddr + offset,
                                     ccip_portdev_kvp_afu_mmio(pccipdev,bar) + offset,
                                     ccip_portdev_len_afu_mmio(pccipdev,bar));
         if ( NULL == pportdev ) {
            PERR("Could not allocate memory for FME object\n");
            res = -ENOMEM;
            goto ERR;
         }

         PDEBUG("Created Port Device\n");
         port_afu_Enable(pportdev);

         // Point to our parent
         ccip_port_to_ccidev(pportdev) = pccipdev;

         // Inherits B:D:F from board
         ccip_port_bustype(pportdev)   = ccip_dev_pcie_bustype(pccipdev);
         ccip_port_busnum(pportdev)    = ccip_dev_pcie_busnum(pccipdev);
         ccip_port_devnum(pportdev)    = ccip_dev_pcie_devnum(pccipdev);
         ccip_port_fcnnum(pportdev)    = ccip_dev_pcie_fcnnum(pccipdev);

         // Log the Port MMIO
         print_sim_port_device(pportdev);

         PDEBUG("Adding to list\n");

         // Added it to the port list
         kosal_list_add(&ccip_port_dev_list(pccipdev), &ccip_port_list_head(pportdev));

         // Save the FME parent for this port
         ccip_port_dev_fme(pportdev) = pfme_dev;

         PDEBUG("Creating Allocatable objects\n");

         // Instantiate allocatable objects including AFUs if present.
         //   Subdevice addresses start at 10x the 1 based port number to leave room for
         //   10 devices beneath the port. E.e., STAP, PR, User AFU
         if(!cci_port_dev_create_AAL_allocatable_objects(pportdev, i) ){
            goto ERR;
         }
      }// End for loop

      ccip_portdev_numports(pccipdev) = i;
      ccip_portdev_maxVFs(pccipdev) = i;     // Can't have more VFs than ports for now

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,8,0)
      if (sriov != 0) {
         // FIXME Hack to enable SR-IOV to enable poweron
         // If started with sriov module parameter, we'll activate a VF and
         // transfer ownership of the AFU. At that point, the AFU will already
         // have been enumerated, so the PF still thinks it has an AFU (and
         // anyone binding to the PF driver will see it), though it is no
         // longer reachable. THIS NEEDS TO BE FIXED! Ideally by deactivating
         // the AFU, moving it over to the VF, and re-enumerating the PF port.
         // FIXME: Assuming there is only one port
         int num_vfs = 1;
         if (0 == pci_enable_sriov(pcidev, num_vfs)){
            // get FME device
            volatile struct fme_device *pfmedev;
            struct CCIP_PORT_AFU_OFFSET port_offset;
            pfmedev = ccip_dev_to_fme_dev(pccipdev);
            if (NULL != pfmedev) {
               // transfer ownership of AFU in PORT0 to VF
               port_offset.csr = pfmedev->m_pHDR->port_offsets[0].csr;
               port_offset.afu_access_control = 0x1;
               pfmedev->m_pHDR->port_offsets[0].csr = port_offset.csr;
            } else {
               PINFO("No FME device, can't transfer ownership of VF.");
            }
            PINFO("SRIOV Enabled on this device for %d VF%s\n", num_vfs, (num_vfs > 1 ? "s" : ""));
         }else{
            PINFO("Failed to enable SRIOV");
         }
      }
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(3,8,0) */

   }

   // Start logging timer
   start_logging_timer();


   return pccipdev;
ERR:
{
   int x;
   PINFO(" -----ERROR -----   \n");
   for(x=1; x<5; x++){
      if(ccip_has_resource(pccipdev, x)){
         PVERBOSE("Freeing Port BAR %d\n",x);
         if( NULL != ccip_portdev_kvp_afu_mmio(pccipdev,x)) {
             iounmap(ccip_portdev_kvp_afu_mmio(pccipdev,x));
             pci_release_region(pcidev, x);
             ccip_portdev_kvp_afu_mmio(pccipdev,x) = NULL;
          }
      }
   }

  if( NULL != ccip_fmedev_kvp_afu_mmio(pccipdev)) {
     PVERBOSE("Freeing Port BAR 0\n");
     remove_perfmonitor(pccipdev->m_pcidev);
     iounmap(ccip_fmedev_kvp_afu_mmio(pccipdev));
     pci_release_region(pcidev, 0);

     if(NULL != ccip_dev_to_fme_dev(pccipdev)) {
         kosal_kfree(ccip_dev_to_fme_dev(pccipdev),sizeof(struct fme_device ) );
     }
     ccip_fmedev_kvp_afu_mmio(pccipdev) = NULL;

   }

   if ( NULL != pccipdev ) {
         kfree(pccipdev);
   }
}

   PTRACEOUT_INT(res);
   return NULL;
}

//=============================================================================
// Name: cci_pci_remove
// Description: Entry point called when a device registered with the PCIe
//              subsystem is being removed
// Interface: public
// Inputs: pcidev - kernel-provided device pointer.
// Outputs: none.
// Comments: Searches through @g_device_list to find any spl2dev which has a
//           cached pointer to pcidev and removes/destroys any found.
//=============================================================================
static
void
cci_pci_remove(struct pci_dev *pcidev)
{
   struct ccip_device   *pccidev    = NULL;
   struct list_head     *This       = NULL;
   struct list_head     *tmp        = NULL;

   int found = 0;

   PTRACEIN;

   ASSERT(pcidev);
   if ( NULL == pcidev ) {
      PERR("PCI remove called with NULL.\n");
      return;
   }

   // Search through our list of devices to find the one matching pcidev
   if ( !kosal_list_is_empty(&g_device_list) ) {

      // Run through list of devices.  Use safe variant
      //  as we will be deleting entries
      kosal_list_for_each_safe(This, tmp, &g_device_list) {

         pccidev = ccip_list_to_ccip_device(This);

         if ( pccidev->m_pcidev == pcidev ) {
            ++found;

            PDEBUG("Deleting device 0x%p with list head 0x%p from list 0x%p\n",
                  pccidev, This, &g_device_list);
            cci_remove_device(pccidev);
         }

      }

   }

   if ( 0 == found ) {
      PINFO("struct pci_dev * 0x%p not found in device list.\n", pcidev);
   } else if ( found > 1 ) {
      PINFO("struct pci_dev * 0x%p found in device list multiple times.\n", pcidev);
   }

   PTRACEOUT;
}

//=============================================================================
// Name: cci_remove_device
// Description: Performs generic cleanup and deletion of CCI object
// Input: pccipdev - device to remove
// Comment:
// Returns: none
// Comments:
//=============================================================================
void
cci_remove_device(struct ccip_device *pccipdev)
{
   int x;
   PDEBUG("Removing CCI device\n");

   // Call PIP to ensure the object is idle and ready for removal
   // TODO

   // Release the resources used for ports
   for(x=0; x<5; x++){
      if(ccip_has_resource(pccipdev, x)){
         if( NULL != ccip_portdev_kvp_afu_mmio(pccipdev,x)) {
            if(!ccip_is_simulated(pccipdev)){
               PVERBOSE("Freeing Port BAR %d\n",x);
               iounmap(ccip_portdev_kvp_afu_mmio(pccipdev,x));
               pci_release_region(ccip_dev_to_pci_dev(pccipdev), x);
            }else{
               kosal_kfree(ccip_portdev_kvp_afu_mmio(pccipdev,x),ccip_portdev_len_afu_mmio(pccipdev,x) );
            }
             ccip_portdev_kvp_afu_mmio(pccipdev,x) = NULL;
          }
      }
   }

   // Release FME Resources
   if( NULL != ccip_fmedev_kvp_afu_mmio(pccipdev)) {
      if(!ccip_is_simulated(pccipdev)){
         PVERBOSE("Freeing FME BAR 0\n");
         remove_perfmonitor(pccipdev->m_pcidev);
         iounmap(ccip_fmedev_kvp_afu_mmio(pccipdev));
         pci_release_region(ccip_dev_to_pci_dev(pccipdev), 0);
      }else{
         kosal_kfree(ccip_fmedev_kvp_afu_mmio(pccipdev),ccip_fmedev_len_afu_mmio(pccipdev) );
      }
      ccip_fmedev_kvp_afu_mmio(pccipdev) = NULL;
   }

   if( cci_aaldev_pci_dev_is_enabled(pccipdev) ) {
      if(!ccip_is_simulated(pccipdev)){
         PVERBOSE("Disabling PCIe device\n");
         pci_disable_device(cci_aaldev_pci_dev(pccipdev));
      }
      cci_aaldev_pci_dev_clr_enabled(pccipdev);
   }

   // Destroy the device
   //  Cleans up any child objects.
   destroy_ccidevice(pccipdev);

} // cci_remove_device


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////                CCI    Driver             ////////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
//=============================================================================

//=============================================================================
// Name: ccidrv_initDriver
// Description: Initialized the CCI driver framework
// Interface: public
// Inputs: Callback structure - ???.
// Outputs: none.
// Comments: none.
//=============================================================================
int
ccidrv_initDriver(void/*callback*/)
{
   int ret                          = 0;     // Return code

   PTRACEIN;

   // Initialize the list of devices controlled by this driver
   kosal_list_init(&g_device_list);
   kosal_mutex_init(&g_dev_list_sem);

   // Display whether we are running with real or simulated hardware
   PINFO("Using %s configuration.\n", (0 == sim) ? "FPGA hardware" : "simulated hardware");

   // Process command line arguments
   if ( 0 == sim ) {
     // Expecting real hardware. Register with PCIe subsystem and wait for OS enumeration
      ret =0;

      // creates logging timer
      create_logging_timer();

      // Attempt to register with the kernel PCIe subsystem.
      ret = pci_register_driver(&driver_info.pcidrv);
      ASSERT(0 == ret);
      if( 0 != ret ) {
         PERR("Failed to register PCI driver. (%d)\n", ret);
         goto ERR;
      }

      // Record the fact that the driver is registered with OS.
      //  Used during uninstall.
      driver_info.isregistered = 1;

      // Create the Debug /sys argument
      //  For now this is only instantiated when using real hardware as we don't have a
      //  registered driver to hang the parameter off of otherwise.
      if(driver_create_file(&driver_info.pcidrv.driver,&driver_attr_debug)){
          DPRINTF (CCIPCIE_DBG_MOD, ": Failed to create debug attribute - Unloading module\n");
          // Unregister the driver with the bus
          ret = -EIO;
          goto ERR;
      }

      // create the logging sysfs argument
      if( create_logging_timervalue_sysfs(&driver_info.pcidrv.driver) ) {

         DPRINTF (CCIPCIE_DBG_MOD, ": Failed to create Logging timer attributes\n");
      }


   } else {

      // Enumerate and Instantiate the Simulated Devices
      ret  = cci_sim_discover_devices(sim, &g_device_list);
      ASSERT(0 == ret);
      if(0 >ret){
         PERR("Failed to create simulated CCI devices.\n");
         // If cci_sim_discover_devices() fails it will have cleaned up.
         goto ERR;
      }

      // creates logging timer
      create_logging_timer();
      // Start logging timer
      start_logging_timer();

   }

   PTRACEOUT_INT(ret);
   return ret;

ERR:

   if ( driver_info.isregistered ) {
      pci_unregister_driver(&driver_info.pcidrv);
      driver_info.isregistered = 0;
   }

   PTRACEOUT_INT(ret);
   return ret;
}


//=============================================================================
// Name: ccidrv_exitDriver
// Description: Exit called when module is unloaded
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments: Walks @g_device_list, destroying any device not registered with
//             the PCIe subsystem.
//=============================================================================
void
ccidrv_exitDriver(void)
{
   struct ccip_device *pccidev      = NULL;
   struct list_head   *This         = NULL;
   struct list_head   *tmp          = NULL;

   PTRACEIN;

   // Remove/destroy any devices that were not registered with the PCIe subsystem.

   // Stop & Remove logging timer
   if(0 ==sim) {

      if( remove_logging_timervalue_syfs(&driver_info.pcidrv.driver) ) {
            DPRINTF (CCIPCIE_DBG_MOD, ": Failed to Remove Logging timer attributes\n");
      }
   }

   stop_logging_timer();
   remove_logging_timer();


   if( !kosal_list_is_empty(&g_device_list) ){

      // Run through list of devices.  Use safe variant
      //  as we will be deleting entries
      kosal_list_for_each_safe(This, tmp, &g_device_list) {

         // Get the device from the list entry
         pccidev = ccip_list_to_ccip_device(This);

         PDEBUG("<- Deleting device 0x%p with list head 0x%p from list 0x%p\n", pccidev,
                                                                                This,
                                                                                &g_device_list);
         cci_remove_device(pccidev);
      }// kosal_list_for_each_safe

   }else {
      PDEBUG("No registered Devices");

   } // if( !kosal_list_is_empty(&g_device_list) )

   if ( driver_info.isregistered ) {
      pci_unregister_driver(&driver_info.pcidrv);
      driver_info.isregistered = 0;
   }


   PINFO("<- %s removed.\n", DRV_DESCRIPTION);
   PTRACEOUT;
}



//=============================================================================
// Name: ccidrv_init
// Description: Entry point called when the module is loaded
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments: none.
//=============================================================================
static int
ccidrv_init(void)
{
   int ret                          = 0;     // Return code

   PTRACEIN;

   //--------------------
   // Display the sign-on
   //--------------------
   PINFO("Accelerator Abstraction Layer\n");
   PINFO("-> %s\n",         DRV_DESCRIPTION);
   PINFO("-> Version %s\n", DRV_VERSION);
   PINFO("-> License %s\n", DRV_LICENSE);
   PINFO("-> %s\n",       DRV_COPYRIGHT);

   // Call the framework initialization
   ret = ccidrv_initDriver(/* Callback */);
   if( 0 == ret ){

      // Initialize the User mode interface
      ret = ccidrv_initUMAPI();
   }

   PTRACEOUT_INT(ret);
   return ret;
}


//=============================================================================
// Name: cciv4drv_exit
// Description: Exit called when module is unloaded
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments:
//=============================================================================
static
void
ccidrv_exit(void)
{
   // Exit the framework
   ccidrv_exitUMAPI();
   ccidrv_exitDriver();
}


