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
//        FILE: cci_pcie_driver_pdde.c
//     CREATED: 10/14/2015
//      AUTHOR: Joseph Grecco, Intel <joe.grecco@intel.com>
// PURPOSE: This file implements the Physical Device Discovery and Enumeration
//          functionality of the Intel(R) Intel QuickAssist Technology AAL
//          FPGA device driver.
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 10/14/2015    JG       Initial version started
//****************************************************************************
#include "aalsdk/kernel/kosal.h"

#define MODULE_FLAGS CCIPCIE_DBG_MOD // Prints all

#include "aalsdk/kernel/aalbus.h"
#include "aalsdk/kernel/aalinterface.h"
//#include "aalsdk/kernel/aalids.h"
//#include "aalsdk/kernel/aalrm.h"
//#include "aalsdk/kernel/aalqueue.h"

#include "cci_pcie_driver_internal.h"

#include "cci_pcie_driver_simulator.h"

//#include "aalsdk/kernel/spl2defs.h"


/// g_device_list - Global device list for this module.
struct list_head g_device_list;

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
//
// Typical usage:
//    sudo insmod ccidrv           # Normal load. PCIe enumeration enabled
//    sudo insmod ccidrv sim=4     # Instantiate 4 simulated AFUs

ulong sim = 0;
MODULE_PARM_DESC(sim, "Simulation: #=Number of simulated AFUs to instantiate");
module_param    (sim, ulong, S_IRUGO);

////////////////////////////////////////////////////////////////////////////////

//=============================================================================
//             D E B U G   F L A G S
//
// kern-utils.h macros expects DRV_NAME and debug declarations
// DRV_NAME is defined in mem-int.h
//=============================================================================

btUnsignedInt debug = 0
/* Type and Level selection flags */
   | PTRACE_FLAG
   | PVERBOSE_FLAG
   | PDEBUG_FLAG
   | PINFO_FLAG
   | PNOTICE_FLAG
/* Module selection flags */
   | CCIV4_DBG_MOD
   | CCIV4_DBG_DEV
   | CCIV4_DBG_AFU
   | CCIV4_DBG_MAFU
   | CCIV4_DBG_MMAP
   | CCIV4_DBG_CMD
   | CCIV4_DBG_CFG
;

/******************************************************************************
 * Debug parameter global definition
 */
MODULE_PARM_DESC(debug, "module debug level");
module_param    (debug, int, 0444);


//=============================================================================
// Name: aalbus_attrib_show_debug
// Decription: Accessor for the debug parameter. Called when the sys fs
//             parameter is read.
//=============================================================================
static ssize_t ahmpip_attrib_show_debug(struct device_driver *drv, char *buf)
{
   return (snprintf(buf,PAGE_SIZE,"%d\n",debug));
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
   sscanf(buf,"%d", &temp);

   debug = temp;

   printk(KERN_INFO DRV_NAME ": Attribute change - debug = %d\n", temp);
   return size;
}

// Attributes for debug
DRIVER_ATTR(debug,S_IRUGO|S_IWUSR|S_IWGRP, ahmpip_attrib_show_debug,ahmpip_attrib_store_debug);



///////////////////////////////////////////////////////////////////////////////
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
static int cci_qpi_internal_probe(struct cci_device         * ,
                                    struct aal_device_id       * ,
                                    struct pci_dev             * ,
                                    const struct pci_device_id * );

static int cci_pcie_internal_probe(struct cci_device         * ,
                                     struct aal_device_id       * ,
                                     struct pci_dev             * ,
                                     const struct pci_device_id * );


/// cci_internal_probe - type of probe functions called by the module's main pci probe.
typedef int (*cci_internal_probe)(struct cci_device         * ,
                                    struct aal_device_id       * ,
                                    struct pci_dev             * ,
                                    const struct pci_device_id * );
#endif
///=================================================================
/// cci_pci_id_tbl - identify PCI devices supported by this module
///=================================================================
static struct pci_device_id cci_pcie_id_tbl[] = {
   { PCI_DEVICE(PCI_VENDOR_ID_INTEL, QPI_DEVICE_ID_FPGA   ), .driver_data = 0/*(kernel_ulong_t)cci_qpi_internal_probe*/  },
   { PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_PCIFPGA), .driver_data = 0/*(kernel_ulong_t)cci_pcie_internal_probe*/ },
   { 0, }
};
CASSERT(sizeof(void *) == sizeof(kernel_ulong_t));

MODULE_DEVICE_TABLE(pci, cci_pcie_id_tbl);

//=============================================================================
// Name: cci_pcie_driver_info
// Description: This struct represents the QPI PCIe driver instance for the
//              driver. The Driver object is registered with the Linux PCI
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
      .name     = CCIV4_PCI_DRIVER_NAME,
      .id_table = cci_pcie_id_tbl,
      .probe    = cci_pci_probe,
      .remove   = cci_pci_remove,
   },
};


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

   PTRACEIN;

   // TODO FILL IN DETAILS

   PTRACEOUT_INT(res);
   return res;
}

#if 0
//=============================================================================
// Name: cci_pcie_internal_probe
// Description: Called during the device probe by cci_pci_probe
//                  when the device id matches PCI_DEVICE_ID_PCIFPGA.
// Interface: public
// Inputs: pspl2dev - module-specific device to be populated.
//         paaldevid - AAL device id to be populated.
//         pcidev - kernel-provided device pointer.
//         pcidevid - kernel-provided device id pointer.
// Outputs: 0 = success.
// Comments:
//=============================================================================
static
int
cci_pcie_internal_probe(struct cci_device         *pspl2dev,
                          struct aal_device_id       *paaldevid,
                          struct pci_dev             *pcidev,
                          const struct pci_device_id *pcidevid)
{

   int res = EINVAL;

   PTRACEIN;

   // TODO FILL IN DETAILS

   PTRACEOUT_INT(res);
   return res;
}


/// cci_qpi_internal_probe - C

//=============================================================================
// Name: cci_qpi_internal_probe
// Description: Called during the device probe by cci_pci_probe
//                  when the device id matches QPI_DEVICE_ID_FPGA.
// Interface: public
// Inputs: pspl2dev - module-specific device to be populated.
//         paaldevid - AAL device id to be populated.
//         pcidev - kernel-provided device pointer.
//         pcidevid - kernel-provided device id pointer.
// Outputs: 0 = success.
// Comments:
//=============================================================================
static
int
cci_qpi_internal_probe(struct cci_device         *pspl2dev,
                         struct aal_device_id       *paaldevid,
                         struct pci_dev             *pcidev,
                         const struct pci_device_id *pcidevid)
{

   int res =0;

   PTRACEIN;

   // TODO FILL IN DETAILS

   PTRACEOUT_INT(res);
   return res;
}


//=============================================================================
// Name: cci_pci_remove_and_rescan
// Description: Used to force a rescan of the PCIe subsystem.
// Interface: public
// Inputs: index - zero based index indicating which board to remove and rescan.
// Outputs: none.
// Comments: Searches through g_device_list to find a board at the index entry
//            in the devicelist and then removes it. Then a rescan on the
//            parent bus is issued.
//=============================================================================
static void
cci_pci_remove_and_rescan(unsigned index)
{
   struct list_head   *This     		= NULL;
   struct list_head   *tmp      		= NULL;
   struct cci_device *pspl2dev 		= NULL;
   struct pci_dev     *pcidev 			= NULL;
   unsigned int bustype					= 0;
   unsigned cnt 						= index;

   PTRACEIN;

   // Search through our list of devices to find the one matching pcidev
   if ( !list_empty(&g_device_list) ) {

      // Run through list of devices.  Use safe variant
      //  as we will be deleting entries
      list_for_each_safe(This, tmp, &g_device_list) {

         pspl2dev = cci_list_to_cci_device(This);

         // If this is it
         if( 0 == cnt ) {

            struct pci_bus *parent = NULL;
            struct aal_device * paaldev = cci_dev_to_aaldev(pspl2dev);

            noprobe =1;

            // Save device information
            pcidev = cci_dev_pci_dev(pspl2dev);
            bustype = cci_dev_board_type(pspl2dev);

            parent = pcidev->bus->parent;

            PDEBUG("Removing the SPL2 device %p\n", pcidev);

            // Save the address so it can be restored
            rescanned_address = aaldev_devaddr(paaldev);

#if !defined(RHEL_RELEASE_VERSION)
#define RHEL_RELEASE_VERSION(a,b) (((a) << 8) + (b))
#endif
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,4,0) && !defined(RHEL_RELEASE_CODE)) || (defined(RHEL_RELEASE_CODE) && RHEL_RELEASE_CODE < RHEL_RELEASE_VERSION(6, 7))
            pci_stop_bus_device(pcidev);
            pci_remove_bus_device(pcidev);
#else
            pci_stop_and_remove_bus_device(pcidev);
#endif
            noprobe =0;

            if(NULL != parent){
                   PDEBUG("Rescanning bus\n");
                   pci_rescan_bus(parent);
            }
            goto DONE;
         }// if( 0 == cnt )
         cnt--;
      }// list_for_each_safe

   }// !list_empty
   PDEBUG("No device at index %d\n", index);
DONE:

   PTRACEOUT;
}
#endif
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
   struct cci_device    *pCCIdev    = NULL;
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
   if ( !list_empty(&g_device_list) ) {

      // Run through list of devices.  Use safe variant
      //  as we will be deleting entries
      list_for_each_safe(This, tmp, &g_device_list) {

         pCCIdev = cci_list_to_cci_device(This);

         if ( pCCIdev->m_pcidev == pcidev ) {
            ++found;

            PDEBUG("Deleting device 0x%p with list head 0x%p from list 0x%p\n",
                  pCCIdev, This, &g_device_list);

// TODO            cci_remove_device(pCCIv4dev);
         }

      }

   }
   ASSERT(1 == found);

   if ( 0 == found ) {
      PERR("struct pci_dev * 0x%p not found in device list.\n", pcidev);
   } else if ( found > 1 ) {
      PERR("struct pci_dev * 0x%p found in device list multiple times.\n", pcidev);
   }

   PTRACEOUT;
}



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
   INIT_LIST_HEAD(&g_device_list);

   // Display whether we are running with real or simulated hardware
   PINFO("Using %s configuration.\n", (0 == sim) ? "FPGA hardware" : "simulated hardware");

   // Process command line arguments
   if ( 0 == sim ) {
     // Expecting real hardware. Register with PCIe subsystem and wait for OS enumeration
      ret =0;

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

   } else {

      // Enumerate and Instantiate the Simulated Devices
      ret  = cci_sim_discover_devices(sim, &g_device_list);
      ASSERT(0 == ret);
      if(0 >ret){
         PERR("Failed to create simulated CCI devices.\n");
         // If cci_sim_discover_devices() fails it will have cleaned up.
         goto ERR;
      }

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
   struct cci_device  *pCCIdev      = NULL;
   struct list_head   *This         = NULL;
   struct list_head   *tmp          = NULL;

   PTRACEIN;

   // Remove/destroy any devices that were not registered with the PCIe subsystem.

   if( !kosal_list_is_empty(&g_device_list) ){

      // Run through list of devices.  Use safe variant
      //  as we will be deleting entries
      kosal_list_for_each_safe(This, tmp, &g_device_list) {

         // Get the device from the list entry
         pCCIdev = cci_list_to_cci_device(This);

         PDEBUG("<- Deleting device 0x%p with list head 0x%p from list 0x%p\n", pCCIdev,
                                                                                This,
                                                                                &g_device_list);
         cci_remove_device(pCCIdev);
      }// kosal_list_for_each_safe

   }else {
      PDEBUG("No registered Devices");

   } // if( !kosal_list_is_empty(&g_device_list) )

   if ( driver_info.isregistered ) {
      pci_unregister_driver(&driver_info.pcidrv);
      driver_info.isregistered = 0;
   }

   //aalbus_get_bus()->unregister_driver( &cci_pci_driver_info.aaldriver );

   PINFO("<- %s removed.\n", DRV_DESCRIPTION);
   PTRACEOUT;
}

