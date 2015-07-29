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
//        FILE: cciv4_driver_main.c
//     CREATED: 07/27/2015
//      AUTHOR: Joseph Grecco, Intel <joe.grecco@intel.com>
//              Henry Mitchel, Intel <henry.mitchel@intel.com>
// PURPOSE: This file implements the initialization and cleanup code for the
//          Intel(R) Intel QuickAssist Technology AAL FPGA device driver.
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// /07/27/2015    JG       Initial version started
//****************************************************************************
#include "aalsdk/kernel/kosal.h"

#define MODULE_FLAGS CCIV4_DBG_MOD // Prints all

#include "aalsdk/kernel/aalbus.h"
#include "aalsdk/kernel/aalinterface.h"
#include "aalsdk/kernel/aalids.h"
#include "aalsdk/kernel/aalrm.h"
#include "aalsdk/kernel/aalqueue.h"
#include "cciv4_driver_internal.h"
//#include "mem-internal-fops.h"

#include "cciv4_simulator.h"

#include "aalsdk/kernel/spl2defs.h"

static int noprobe = 0;

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
//    sudo insmod ccidrv sim=4     # Instantiate

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
 * Debug global definition
 */

//=============================================================================
// /sys/bus/pci/drivers/aalspl2/recsn_index
// rescan_index: Setting this variable will cause the board at the <val>
//               position in the device list to be removed and the bus
//               rescanned. This should enable the device to be rediscovered
static void cciv4_pci_remove_and_rescan(unsigned);
//
int rescan_index  = 0;  //Used to rescan device by index
MODULE_PARM_DESC(rescan_index, "Rescan device by index");
module_param    (rescan_index, int, 0774);

struct aal_device_addr rescanned_address = {0};


//=============================================================================
// Name: aalbus_attrib_store_debug
//=============================================================================
static ssize_t cciv4drv_attrib_store_rescan_index( struct device_driver *drv,
                                             	  const char *buf,
                                              	  size_t size)
{
   sscanf(buf,"%d", &rescan_index);

   cciv4_pci_remove_and_rescan(rescan_index);

   return size;
}

DRIVER_ATTR(rescan_index,S_IRUGO|S_IWUSR|S_IWGRP, NULL,cciv4drv_attrib_store_rescan_index);


MODULE_PARM_DESC(debug, "module debug level");
module_param    (debug, int, 0444);
//=============================================================================
// Name: aalbus_attrib_show_debug
//=============================================================================
static ssize_t ahmpip_attrib_show_debug(struct device_driver *drv, char *buf)
{
   return (snprintf(buf,PAGE_SIZE,"%d\n",debug));
}

//=============================================================================
// Name: aalbus_attrib_store_debug
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

// Attribute accessors for debug
DRIVER_ATTR(debug,S_IRUGO|S_IWUSR|S_IWGRP, ahmpip_attrib_show_debug,ahmpip_attrib_store_debug);


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////              PCIE INTERFACES             ////////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//=============================================================================
// Name: devIID_tbl
// Description: Lists the IDs of the interfaces this service exports to the
//              AALBus interface broker.
//=============================================================================
static btID devIID_tbl[] = {
   0  // None
};

//=============================================================================
// Name: spldrv_class
// Description: Class device. Wrapper object that contains both the Linux
//              DD Model class information but also AAL specific class
//              information. The modules "class" defines its unique interface
//              attributes.
//=============================================================================
static struct aal_classdevice cciv4drv_class = {
   .m_classid = {
      .m_majorversion   = 0, //AALUI_DRV_MAJVERSION,
      .m_minorversion   = 0, //AALUI_DRV_MINVERSION,
      .m_releaseversion = 0, //AALUI_DRV_RELEASE,
      .m_classGUID      = (0x0000000000002000), // AALUI_DRV_INTC,
   },
   .m_devIIDlist = devIID_tbl,        // List of supported device module APIs
};

// Declare standard entry points
static int  cciv4drv_init(void);
static void cciv4drv_exit(void);

module_init(cciv4drv_init);
module_exit(cciv4drv_exit);



static
int
cciv4_pci_probe(struct pci_dev * , const struct pci_device_id * );
static
void
cciv4_pci_remove(struct pci_dev * );

/// cciv4_internal_probe - type of probe functions called by the module's main pci probe.
typedef int (*cciv4_internal_probe)(struct cciv4_device         * ,
                                   struct aal_device_id       * ,
                                   struct pci_dev             * ,
                                   const struct pci_device_id * );
static
int
cciv4_qpi_internal_probe(struct cciv4_device         * ,
                        struct aal_device_id       * ,
                        struct pci_dev             * ,
                        const struct pci_device_id * );

static
int
cciv4_pcie_internal_probe(struct cciv4_device         * ,
                         struct aal_device_id       * ,
                         struct pci_dev             * ,
                         const struct pci_device_id * );


/// cciv4_pci_id_tbl - identify PCI devices supported by this module
static struct pci_device_id cciv4_pci_id_tbl[] = {
   { PCI_DEVICE(PCI_VENDOR_ID_INTEL, QPI_DEVICE_ID_FPGA   ), .driver_data = (kernel_ulong_t)cciv4_qpi_internal_probe  },
   { PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_PCIFPGA), .driver_data = (kernel_ulong_t)cciv4_pcie_internal_probe },
   { 0, }
};
CASSERT(sizeof(void *) == sizeof(kernel_ulong_t));

MODULE_DEVICE_TABLE(pci, cciv4_pci_id_tbl);

//=============================================================================
// Name: cciv4_pci_driver_info
// Description: This struct represents the QPI PCIe driver instance for the
//              DKSM. The Driver object is registered with the Linux PCI
//              subsystem.
//=============================================================================
struct cciv4_pci_driver_info
{
   int                        isregistered; // boolean : did we register with PCIe subsystem?
   struct pci_driver          pcidrv;
   struct aal_driver          aaldriver;
};
static struct cciv4_pci_driver_info cciv4_pci_driver_info = {
   .isregistered = 0,
   .pcidrv = {
      .name     = CCIV4_PCI_DRIVER_NAME,
      .id_table = cciv4_pci_id_tbl,
      .probe    = cciv4_pci_probe,
      .remove   = cciv4_pci_remove,
   },
   .aaldriver = {
         // Base structure
         .m_driver =  {
            .owner = THIS_MODULE,
            .name  = DEVICE_BASENAME,
         },
   },
};

/// cciv4_pci_probe - Called during the device probe by PCI subsystem.
/// @pcidev: kernel-provided device pointer.
/// @pcidevid: kernel-provided device id pointer.
static
int
cciv4_pci_probe(struct pci_dev             *pcidev,
                const struct pci_device_id *pcidevid)
{

   int res = -EINVAL;

   PTRACEIN;

   // TODO FILL IN DETAILS

   PTRACEOUT_INT(res);
   return res;
}

/// cciv4_pcie_internal_probe - Called during the device probe by cciv4_pci_probe
///                            when the device id matches PCI_DEVICE_ID_PCIFPGA.
/// @pspl2dev: module-specific device to be populated.
/// @paaldevid: AAL device id to be populated.
/// @pcidev: kernel-provided device pointer.
/// @pcidevid: kernel-provided device id pointer.
static
int
cciv4_pcie_internal_probe(struct cciv4_device         *pspl2dev,
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


/// cciv4_qpi_internal_probe - Called during the device probe by cciv4_pci_probe
///                            when the device id matches QPI_DEVICE_ID_FPGA.
/// @pspl2dev: module-specific device to be populated.
/// @paaldevid: AAL device id to be populated.
/// @pcidev: kernel-provided device pointer.
/// @pcidevid: kernel-provided device id pointer.
static
int
cciv4_qpi_internal_probe(struct cciv4_device         *pspl2dev,
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


/// cciv4_pci_remove_and_rescan - Used to force a rescan of the device
/// @index: zero based index indicating which board to remove and rescan
///
/// Searches through @g_device_list to find a board at the index entry in the device
/// list and then removes it. Then a rescan on teh parent bus is issued.
static void
cciv4_pci_remove_and_rescan(unsigned index)
{
   struct list_head   *This     		= NULL;
   struct list_head   *tmp      		= NULL;
   struct cciv4_device *pspl2dev 		= NULL;
   struct pci_dev     *pcidev 			= NULL;
   unsigned int bustype					= 0;
   unsigned cnt 						= index;

   PTRACEIN;

   // Search through our list of devices to find the one matching pcidev
   if ( !list_empty(&g_device_list) ) {

      // Run through list of devices.  Use safe variant
      //  as we will be deleting entries
      list_for_each_safe(This, tmp, &g_device_list) {

         pspl2dev = cciv4_list_to_cciv4_device(This);

         // If this is
         if( 0 == cnt ) {

        	struct pci_bus *parent = NULL;
        	struct aal_device * paaldev = cciv4_dev_to_aaldev(pspl2dev);

        	noprobe =1;

        	// Save device information
       	    pcidev = cciv4_dev_pci_dev(pspl2dev);
       	    bustype = cciv4_dev_board_type(pspl2dev);

       	    parent = pcidev->bus->parent;

            PDEBUG("Removing the SPL2 device %p\n", pcidev);

            // Save the address so it can be restored

            rescanned_address = aaldev_devaddr(paaldev);

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,4,0)
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
         }
         cnt--;
      }

   }
   PDEBUG("No device at index %d\n", index);
DONE:

   PTRACEOUT;
}

/// cciv4_pci_remove - Entry point called when a device registered with the PCIe
///                   subsystem is being removed.
/// @pcidev: kernel-provided device pointer.
///
/// Searches through @g_device_list to find any spl2dev which has a cached
/// pointer to @pcidev and removes/destroys any found.
static
void
cciv4_pci_remove(struct pci_dev *pcidev)
{
   struct cciv4_device  *pCCIv4dev = NULL;
   struct list_head     *This      = NULL;
   struct list_head     *tmp       = NULL;

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

         pCCIv4dev = cciv4_list_to_cciv4_device(This);

         if ( pCCIv4dev->m_pcidev == pcidev ) {
            ++found;

            PDEBUG("Deleting device 0x%p with list head 0x%p from list 0x%p\n",
                  pCCIv4dev, This, &g_device_list);

            cciv4_remove_device(pCCIv4dev);
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
/////////////////                CCI V4 Driver             ////////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
//=============================================================================
struct aal_interface             cciv4_afu_pip_interface;
struct aal_interface             cciv4_simafu_pip_interface;


//=============================================================================
// Name: cciv4drv_init
// Description: Entry point called when the module is loaded
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments: none.
//=============================================================================
static int
cciv4drv_init(void)
{
   int ret                          = 0;     // Return code

   PTRACEIN;

   //--------------------
   // Display the sign-on
   //--------------------
   PINFO("Intel(R) QuickAssist Technology Accelerator Abstraction Layer\n");
   PINFO("-> %s\n",         DRV_DESCRIPTION);
   PINFO("-> Version %s\n", DRV_VERSION);
   PINFO("-> License %s\n", DRV_LICENSE);
   PINFO("-> %s\n",       DRV_COPYRIGHT);

   // Initialize the list of devices
   INIT_LIST_HEAD(&g_device_list);

   PINFO("Using %s configuration.\n", (0 == sim) ? "normal hardware" : "simulated hardware");

   // Install an OS driver for this module.  This enables probe() semantics.
   //   This functionality will be moving to AAL Bus subsystem as it is canonical.
   ret = aalbus_get_bus()->init_driver((kosal_ownermodule *)THIS_MODULE,
                                        &cciv4_pci_driver_info.aaldriver,
                                        &cciv4drv_class,
                                        "cciv4",  //TODO do this right
                                        0);
   ASSERT(ret >= 0);
   if(0 > ret){
      return ret;
   }
   //----------------------------------
   // Create the common /sys parameters
   //----------------------------------
   // Create the Debug /sys argument
   if(driver_create_file(&cciv4_pci_driver_info.aaldriver.m_driver,&driver_attr_debug)){
       DPRINTF (AHMPIP_DBG_MOD, ": Failed to create debug attribute - Unloading module\n");
       // Unregister the driver with the bus
       aalbus_get_bus()->unregister_driver( &cciv4_pci_driver_info.aaldriver );
       return -EIO;
   }

   // Process command line arguments
   if ( 0 == sim ) {
      return -EIO;
#if 0
      // Expecting real hardware. Register with PCIe subsystem and wait for OS enumeration
      int ret =0;

      // Attempt to register with the kernel PCIe subsystem.
      ret = pci_register_driver(&cciv4_pci_driver_info.pcidrv);
      ASSERT(0 == ret);
      if( 0 != ret ) {
         PERR("Failed to register PCI driver. (%d)\n", ret);
         goto ERR;
      }

      // Record the fact that the driver is registered with OS.
      //  Used during uninstall.
      cciv4_pci_driver_info.isregistered = 1;

      // Create the Rescan /sys parameter used to force a PCIe rescan.
      //  This parameter is only used with real hardware
      if ( driver_create_file(&cciv4_pci_driver_info.pcidrv.driver,&driver_attr_rescan_index) ) {
          PDEBUG("Failed to create rescan_index attribute - Unloading module\n");
          goto ERR;
      }

      // Initialize the aal_interface
      aal_interface_init( cciv4_afu_pip_interface,  // Interface container
                         &CCIV4_AFUpip,             // PIP Service interface (Vtable)
                          QPI_CCIAFUPIP_IID);       // Interface ID

      // Register with the service interface broker
      PVERBOSE("Registering cciv4 PIP service interface 0x%Lx\n", (long long)CCIV4_AFUPIP_IID);

      ret = aalbus_get_bus()->register_service_interface(&cciv4_afu_pip_interface);
      ASSERT(ret >= 0);
      if ( ret < 0 ) {
         PERR("Failed to register cciv4 AFU PIP service interface.\n");
         goto ERR;
      }
#endif
   } else {

      // Initialize the aal_interface
      aal_interface_init( cciv4_simafu_pip_interface,  // Interface container
                         &CCIV4_SIMAFUpip,             // PIP Service interface (Vtable)
                          CCIV4_SIMAFUPIP_IID);        // Interface ID

      // Register with the service interface broker
      PVERBOSE("Registering Simulated cciv4 PIP service interface 0x%Lx\n", (long long)CCIV4_SIMAFUPIP_IID);

      ret = aalbus_get_bus()->register_service_interface(&cciv4_simafu_pip_interface);
      ASSERT(ret >= 0);
      if ( ret < 0 ) {
         aalbus_get_bus()->unregister_driver( &cciv4_pci_driver_info.aaldriver );
         PERR("Failed to register Simulated cciv4 AFU PIP service interface.\n");
         return ret;
      }

      // Enumerate and Instantiate the Simulated Devices
      ret  = cciv4_sim_discover_devices(sim, &g_device_list);
      ASSERT(0 == ret);
      if(0 >ret){
         aalbus_get_bus()->unregister_driver( &cciv4_pci_driver_info.aaldriver );
         // If cciv4_sim_discover_devices() fails it will have cleaned up.
         return ret;
      }

   }

   PTRACEOUT_INT(ret);
   return ret;

ERR:
   // TODO error path needs to deal with all possibilitied. E.g., device initialized but interface reg failed need to destroy
   if ( aalinterface_is_registered(&cciv4_afu_pip_interface) ) {
      aalbus_get_bus()->unregister_service_interface(&cciv4_afu_pip_interface);
   }

   if ( aalinterface_is_registered(&cciv4_simafu_pip_interface) ) {
      aalbus_get_bus()->unregister_service_interface(&cciv4_simafu_pip_interface);
   }

   if ( cciv4_pci_driver_info.isregistered ) {
      pci_unregister_driver(&cciv4_pci_driver_info.pcidrv);
      cciv4_pci_driver_info.isregistered = 0;
   }

   aalbus_get_bus()->unregister_driver( &cciv4_pci_driver_info.aaldriver );

   PTRACEOUT_INT(ret);
   return ret;
}

/// cciv4drv_exit - Exit function when unloading the module.
///
/// Walks @g_device_list, destroying any device not registered with
/// the PCIe subsystem.
/// Unregisters the module AFU/MAFU interfaces.
/// Unregisters the driver with the PCI subsystem, if needed.

//=============================================================================
// Name: cciv4drv_exit
// Description: Exit called when module is unloaded
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments: Walks @g_device_list, destroying any device not registered with
//             the PCIe subsystem.
//=============================================================================
static
void
cciv4drv_exit(void)
{
   struct cciv4_device *pCCIv4dev   = NULL;
   struct list_head   *This         = NULL;
   struct list_head   *tmp          = NULL;

   PTRACEIN;

   // Remove/destroy any devices that were not registered with the PCIe subsystem.

   if( !kosal_list_is_empty(&g_device_list) ){

      // Run through list of devices.  Use safe variant
      //  as we will be deleting entries
      kosal_list_for_each_safe(This, tmp, &g_device_list) {

         // Get the device from the list entry
         pCCIv4dev = cciv4_list_to_cciv4_device(This);

         PDEBUG("<- Deleting device 0x%p with list head 0x%p from list 0x%p\n", pCCIv4dev,
                                                                                This,
                                                                                &g_device_list);

         cciv4_remove_device(pCCIv4dev);
      }// kosal_list_for_each_safe

   }else {
      PDEBUG("No registered Devices")

   } // if( !kosal_list_is_empty(&g_device_list) )

   if( aalinterface_is_registered(&cciv4_afu_pip_interface) ) {
      aalbus_get_bus()->unregister_service_interface(&cciv4_afu_pip_interface);
   }

   if( aalinterface_is_registered(&cciv4_simafu_pip_interface) ) {
      aalbus_get_bus()->unregister_service_interface(&cciv4_simafu_pip_interface);
   }

   if ( cciv4_pci_driver_info.isregistered ) {
      pci_unregister_driver(&cciv4_pci_driver_info.pcidrv);
      cciv4_pci_driver_info.isregistered = 0;
   }

   aalbus_get_bus()->release_driver(&cciv4_pci_driver_info.aaldriver, &cciv4drv_class);

   //aalbus_get_bus()->unregister_driver( &cciv4_pci_driver_info.aaldriver );

   PINFO("<- %s removed.\n", DRV_DESCRIPTION);
   PTRACEOUT;
}

/// AFUrelease_device - callback for notification that an AFU is being destroyed.
/// @pdev: kernel-provided generic device structure.
///
/// NOTE: AALBus does the AFU destruction
void
AFUrelease_device(struct device *pdev)
{
   struct aal_device *paaldev = basedev_to_aaldev(pdev);

   PTRACEIN;

   PDEBUG("Called with struct aal_device * 0x%p\n", paaldev);

   // DO NOT call factory release here. It will be done by the framework.
   PTRACEOUT;
}

