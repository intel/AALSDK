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
//        FILE: spl2_pip_main.c
//     CREATED: 02/04/2012
//      AUTHOR: Joseph Grecco, Intel <joe.grecco@intel.com>
//              Henry Mitchel, Intel <henry.mitchel@intel.com>
// PURPOSE: This file implements the initialization and cleanup code for the
//          Intel(R) Intel QuickAssist Technology SPL2 Physical Interface
//          Protocol Module.
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 05/12/2012	  HM       Move pci bus-mastering initialization code above
//                           probe to allow probe to possibly succeed.
// 10/25/2012	  TSW      Cleanup for faplib
// 04/17/2013     JG       Added rescan_index parameter for hot plug emulation
// 06/25/2013     JG       Changed rescan feature so that it reassigns addr of
//                           board being reinserted rather than assigning a
//                           new address.
// 03/06/2014     JG       Fixed bug which cause GPF if probe_fn() failed
//****************************************************************************
#include "aalsdk/kernel/kosal.h"

#define MODULE_FLAGS SPL2_DBG_MOD // Prints all

#include "aalsdk/kernel/aalbus.h"
#include "aalsdk/kernel/aalinterface.h"
#include "aalsdk/kernel/aalids.h"
#include "aalsdk/kernel/aalrm.h"
#include "aalsdk/kernel/aalqueue.h"
#include "spl2_pip_internal.h"
#include "mem-internal-fops.h"
#include "aalsdk/kernel/spl2defs.h"

static int noprobe = 0;

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
//      nopcie: Manual detection and configuration. The default is to register
//              with AAL PCIe system
//       Value: 0: Register with PCIe subsystem
//              1: Try Emerald Ridge then Romley
//              2: Use Emerald Ridge Base address (0x88080000)
//              3: Use Romley Base address (0xC8080000)
//              4: Create Simulated hardware
//             >4: use as an address.
//     Comment: 0x format is acceptable on the kernel command-line.
//
// Typical usage:
//    sudo insmod spl2pip nopcie=1
//    sudo insmod spl2pip nopcie=0x20000000

ulong nopcie = 0;
MODULE_PARM_DESC(nopcie, "Configuration Control: 0=use PCIe (default), 1=try NHM, JKT; 2=NHM; 3=JKT; 4=Simulated hardware. Other is physical base address");
module_param    (nopcie, ulong, S_IRUGO);

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
   | SPL2_DBG_MOD
   | SPL2_DBG_DEV
   | SPL2_DBG_AFU
   | SPL2_DBG_MAFU
   | SPL2_DBG_MMAP
   | SPL2_DBG_CMD
   | SPL2_DBG_CFG
;

/******************************************************************************
 * Debug global definition
 */

//=============================================================================
// /sys/bus/pci/drivers/aalspl2/recsn_index
// rescan_index: Setting this variable will cause the board at the <val>
//               position in the device list to be removed and the bus
//               rescanned. This should enable the device to be rediscovered
static void spl2_pci_remove_and_rescan(unsigned);
//
int rescan_index  = 0;  //Used to rescan device by index
MODULE_PARM_DESC(rescan_index, "Rescan device by index");
module_param    (rescan_index, int, 0774);

struct aal_device_addr rescanned_address = {0};


//=============================================================================
// Name: aalbus_attrib_store_debug
//=============================================================================
static ssize_t spl2pip_attrib_store_rescan_index( struct device_driver *drv,
                                             	  const char *buf,
                                              	  size_t size)
{
   sscanf(buf,"%d", &rescan_index);

   spl2_pci_remove_and_rescan(rescan_index);

   return size;
}

DRIVER_ATTR(rescan_index,S_IRUGO|S_IWUSR|S_IWGRP, NULL,spl2pip_attrib_store_rescan_index);


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
static struct aal_classdevice spldrv_class = {
   .m_classid = {
      .m_majorversion   = 0, //AALUI_DRV_MAJVERSION,
      .m_minorversion   = 0, //AALUI_DRV_MINVERSION,
      .m_releaseversion = 0, //AALUI_DRV_RELEASE,
      .m_classGUID      = (0x0000000000002000), // AALUI_DRV_INTC,
   },
   .m_devIIDlist = devIID_tbl,        // List of supported device module APIs
};

// Declare standard entry points
static int  spl2pip_init(void);
static void spl2pip_exit(void);

module_init(spl2pip_init);
module_exit(spl2pip_exit);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////             AAL PCIE INTERFACES          ////////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static
int
spl2_pci_probe(struct pci_dev * , const struct pci_device_id * );
static
void
spl2_pci_remove(struct pci_dev * );

/// spl2_internal_probe - type of probe functions called by the module's main pci probe.
typedef int (*spl2_internal_probe)(struct spl2_device         * ,
                                   struct aal_device_id       * ,
                                   struct pci_dev             * ,
                                   const struct pci_device_id * );
static
int
spl2_qpi_internal_probe(struct spl2_device         * ,
                        struct aal_device_id       * ,
                        struct pci_dev             * ,
                        const struct pci_device_id * );

static
int
spl2_pcie_internal_probe(struct spl2_device         * ,
                         struct aal_device_id       * ,
                         struct pci_dev             * ,
                         const struct pci_device_id * );


/// spl2_pci_id_tbl - identify PCI devices supported by this module
static struct pci_device_id spl2_pci_id_tbl[] = {
   { PCI_DEVICE(PCI_VENDOR_ID_INTEL, QPI_DEVICE_ID_FPGA   ), .driver_data = (kernel_ulong_t)spl2_qpi_internal_probe  },
   { PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_PCIFPGA), .driver_data = (kernel_ulong_t)spl2_pcie_internal_probe },
   { 0, }
};
CASSERT(sizeof(void *) == sizeof(kernel_ulong_t));

MODULE_DEVICE_TABLE(pci, spl2_pci_id_tbl);

//=============================================================================
// Name: spl2_pci_driver_info
// Description: This struct represents the QPI PCIe driver instance for the
//              DKSM. The Driver object is registered with the Linux PCI
//              subsystem.
//=============================================================================
struct spl2_pci_driver_info
{
   int                        isregistered; // boolean : did we register with PCIe subsystem?
   struct pci_driver          pcidrv;
   struct aal_driver          aaldriver;
};
static struct spl2_pci_driver_info spl2_pci_driver_info = {
   .isregistered = 0,
   .pcidrv = {
      .name     = SPL2_PCI_DRIVER_NAME,
      .id_table = spl2_pci_id_tbl,
      .probe    = spl2_pci_probe,
      .remove   = spl2_pci_remove,
   },
   .aaldriver = {
         // Base structure
         .m_driver =  {
            .owner = THIS_MODULE,
            .name  = "cci-splpip",
         },
   },
};


/// g_device_list - Global device list for this module.
struct list_head g_device_list;


/// spl2_pci_probe - Called during the device probe by PCI subsystem.
/// @pcidev: kernel-provided device pointer.
/// @pcidevid: kernel-provided device id pointer.
static
int
spl2_pci_probe(struct pci_dev             *pcidev,
               const struct pci_device_id *pcidevid)
{
   struct spl2_device  *pspl2dev = NULL;
   struct aal_device_id aaldevid;
   spl2_internal_probe  probe_fn;

   int res = -EINVAL;

   PTRACEIN;

   if ( noprobe ) {
	   PDEBUG("Probe disabled\n");
	   return res;
   }

   probe_fn = (spl2_internal_probe)pcidevid->driver_data;

   ASSERT(NULL != probe_fn);
   if ( NULL == probe_fn ) {
      PERR("NULL spl2_internal_probe in 0x%p\n", pcidevid);
      goto ERR;
   }

   pspl2dev = kzalloc(sizeof(struct spl2_device), GFP_KERNEL);
   if ( NULL == pspl2dev ) {
      res = -ENOMEM;
      goto ERR;
   }

   memset(&aaldevid, 0, sizeof(aaldevid));

   res = probe_fn(pspl2dev, &aaldevid, pcidev, pcidevid);

   ASSERT(0 == res);
   if ( 0 != res ) {
      goto ERR;
   }

   res = spl2_spl_device_init(pspl2dev, &aaldevid, &g_device_list);
   if ( 0 != res ) {
      // Assumption is that since init failed there pspl2dev has
      //  not been initialized. I.e., it can simply be freed
      goto ERR;
   }

   res = 0;

   PTRACEOUT_INT(res);
   return res;

ERR:

   if ( NULL != pspl2dev ) {
      kfree(pspl2dev);
   }

   PTRACEOUT_INT(res);
   return res;
}

/// spl2_pcie_internal_probe - Called during the device probe by spl2_pci_probe
///                            when the device id matches PCI_DEVICE_ID_PCIFPGA.
/// @pspl2dev: module-specific device to be populated.
/// @paaldevid: AAL device id to be populated.
/// @pcidev: kernel-provided device pointer.
/// @pcidevid: kernel-provided device id pointer.
static
int
spl2_pcie_internal_probe(struct spl2_device         *pspl2dev,
                         struct aal_device_id       *paaldevid,
                         struct pci_dev             *pcidev,
                         const struct pci_device_id *pcidevid)
{
   int pcibar = 0;
   u32 viddid = 0;

   int res;

   PTRACEIN;

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

   spl2_dev_pci_dev(pspl2dev) = pcidev;

   res = pci_enable_device(pcidev);
   if ( res < 0 ) {
      PERR("Failed to enable device res=%d\n", res);
      PTRACEOUT_INT(res);
      return res;
   }
   spl2_dev_pci_dev_set_enabled(pspl2dev);

   res = pci_request_region(pcidev, pcibar, SPL2_PCI_DRIVER_NAME);
   if ( res ) {
      PERR("Failed to obtain pci region bar=%d \"%s\" (%d)\n",
              pcibar,
              SPL2_PCI_DRIVER_NAME,
              res);
      res = -EBUSY;
      PTRACEOUT_INT(res);
      return res;
   }

   // After this point ERR will insure region released
   spl2_dev_pci_dev_set_region_requested(pspl2dev);

   // for actual PCIe card (not QPI), enable PCIe error reporting
   pci_enable_pcie_error_reporting(pcidev);

   // enable bus mastering (if not already set)
   pci_set_master(pcidev);
   pci_save_state(pcidev);

   // Verify the device signature.
   res = pci_read_config_dword(pcidev, 0, &viddid);
   ASSERT(0 == res);
   if ( 0 != res ) {
      goto ERR;
   }
#if 0
   if ( !spl2_viddid_is_supported(viddid) ) {
      res = -ENXIO;
      goto ERR;
   }
#endif
   // Grab the base address register.
   spl2_dev_phys_config(pspl2dev) = __PHYS_ADDR_CAST( pci_resource_start(pcidev, pcibar) );
   spl2_dev_len_config(pspl2dev)  = (size_t)pci_resource_len(pcidev, pcibar);

   // Remap the bar0 region into kernel space.
   spl2_dev_kvp_config(pspl2dev) = ioremap(spl2_dev_phys_config(pspl2dev), spl2_dev_len_config(pspl2dev));
   if( NULL == spl2_dev_kvp_config(pspl2dev) ) {
      PERR("Failed to map BAR %d into kernel space.\n", pcibar);
      res = -ENXIO;
      goto ERR;
   }

   PNOTICE("pcie device 0x%X found at phys=0x%" PRIxPHYS_ADDR " kvp=0x%p len=%" PRIuSIZE_T "\n",
              viddid,
              spl2_dev_phys_config(pspl2dev),
              spl2_dev_kvp_config(pspl2dev),
              spl2_dev_len_config(pspl2dev));

   // Populate an AAL device ID for it.
   if(aal_bustype_unknown == aaldevaddr_bustype(rescanned_address) ){
	   aaldevid_addr(*paaldevid)            = spl2_alloc_next_afu_addr();
   }else {
	   PINFO("Reassigning rescanned address");
	   aaldevid_addr(*paaldevid)             = rescanned_address;

	   // Reset the rescanned_address as undefined
	   aaldevaddr_bustype(rescanned_address) = aal_bustype_unknown;
   }
   aaldevid_devaddr_bustype(*paaldevid) = aal_bustype_PCIe;

   // Set the device type and its interface (ie. PIP)
   aaldevid_devtype(*paaldevid)         = aal_devtypeAFU;
   aaldevid_vendorid(*paaldevid)        = AAL_vendINTC;
   aaldevid_pipguid(*paaldevid)         = SPL2_AFUPIP_IID;
   aaldevid_ahmguid(*paaldevid)         = QPI_AHM_GUID;

   spl2_dev_board_type(pspl2dev)        = aal_bustype_PCIe;

   res = 0;

ERR:
   pci_release_region(pcidev, pcibar);
   PTRACEOUT_INT(res);
   return res;
}


/// spl2_qpi_internal_probe - Called during the device probe by spl2_pci_probe
///                            when the device id matches QPI_DEVICE_ID_FPGA.
/// @pspl2dev: module-specific device to be populated.
/// @paaldevid: AAL device id to be populated.
/// @pcidev: kernel-provided device pointer.
/// @pcidevid: kernel-provided device id pointer.
static
int
spl2_qpi_internal_probe(struct spl2_device         *pspl2dev,
                        struct aal_device_id       *paaldevid,
                        struct pci_dev             *pcidev,
                        const struct pci_device_id *pcidevid)
{
   int pcibar = 0;
   u32 viddid = 0;
   int res =0;

   PTRACEIN;

   ASSERT(pcidev);
   if ( NULL == pcidev ) {
      return -EINVAL;
   }
   ASSERT(pcidevid);
   if ( NULL == pcidevid ) {
      return -EINVAL;
   }
   ASSERT(QPI_DEVICE_ID_FPGA == pcidevid->device);
   if ( QPI_DEVICE_ID_FPGA != pcidevid->device ) {
      return -EINVAL;
   }

   spl2_dev_pci_dev(pspl2dev) = pcidev;

   res = pci_enable_device(pcidev);
   if ( res < 0 ) {
      PERR("Failed to enable device res=%d\n", res);
      return res;
   }
   spl2_dev_pci_dev_set_enabled(pspl2dev);

   res = pci_request_region(pcidev, pcibar, SPL2_PCI_DRIVER_NAME);
   if ( res ) {
      PERR("Failed to obtain pci region bar=%d \"%s\" (%d)\n",
              pcibar,
              SPL2_PCI_DRIVER_NAME,
              res);
      res = -EBUSY;
      goto ERR;
   }
   spl2_dev_pci_dev_set_region_requested(pspl2dev);

   // Do not enable PCIe error reporting for QPI

   // enable bus mastering (if not already set)
   pci_set_master(pcidev);
   pci_save_state(pcidev);

   // Verify the device signature.
   res = pci_read_config_dword(pcidev, 0, &viddid);
   ASSERT(0 == res);
   if ( 0 != res ) {
      goto ERR;
   }

   if ( !spl2_viddid_is_supported(viddid) ) {
      res = -ENXIO;
      goto ERR;
   }

   // Grab the base address register.
   spl2_dev_phys_config(pspl2dev) = __PHYS_ADDR_CAST( pci_resource_start(pcidev, pcibar) );
   //spl2_dev_len_config(pspl2dev)  = (size_t)pci_resource_len(pcidev, pcibar);
   spl2_dev_len_config(pspl2dev)  = QPI_APERTURE_SIZE;

   PNOTICE("OS REPORTS Aperture size of len=%" PRIuSIZE_T "\n",(unsigned long)pci_resource_len(pcidev, pcibar));

   // Remap the bar0 region into kernel space.
   spl2_dev_kvp_config(pspl2dev) = ioremap_nocache(spl2_dev_phys_config(pspl2dev),
                                                   spl2_dev_len_config(pspl2dev));
   if( NULL == spl2_dev_kvp_config(pspl2dev) ) {
      PERR("Failed to map BAR %d (0x%" PRIxPHYS_ADDR ") into kernel space.\n",
              pcibar,
              spl2_dev_phys_config(pspl2dev));
      res = -ENXIO;
      goto ERR;
   }

#if 0
   // Set CCI CSR Attributes
   spl2_dev_phys_cci_csr(pspl2dev) = spl2_dev_phys_config(pspl2dev);
   spl2_dev_kvp_cci_csr(pspl2dev) = spl2_dev_kvp_config(pspl2dev);
   spl2_dev_len_cci_csr(pspl2dev) = QPI_APERTURE_SIZE;

   PNOTICE("qpi device 0x%X found at phys=0x%" PRIxPHYS_ADDR " kvp=0x%p len=%" PRIuSIZE_T "\n",
              viddid,
              spl2_dev_phys_config(pspl2dev),
              spl2_dev_kvp_config(pspl2dev),
              spl2_dev_len_config(pspl2dev));

   // Verify the QLP version - note this is specific to QPI.
   res = check_qlp_feature_id(pspl2dev);
   ASSERT(0 == res);
   if ( 0 != res ) {
      PERR("Failed to verify CCI 3 protocol.\n");
      res = -ENXIO;
      goto ERR;
   }
   PVERBOSE("Verified that device is running CCI 3 protocol.\n");

   // Get the Interface Protocol. CCI or SPL.
   spl2_dev_protocol(pspl2dev) = check_pip_type_id(pspl2dev);

   if(PCIE_FEATURE_HDR3_PROTOCOL_SPL == spl2_dev_protocol(pspl2dev) ){
      res =  spl2_init_spl3device(pspl2dev, paaldevid);
   }else if(PCIE_FEATURE_HDR3_PROTOCOL_CCI3 == spl2_dev_protocol(pspl2dev)){
      res = cci3_init_ccidevice(pspl2dev, paaldevid);
   }else{
      PERR("Invalid protocol %d. Only SPL3 or CCI3 supported\n",spl2_dev_protocol(pspl2dev));
      res = -ENXIO;
      goto ERR;
   }
#endif

   // Identify the device
   res = spl2_identify_device(pspl2dev);
   if(PCIE_FEATURE_HDR3_PROTOCOL_SPL == spl2_dev_protocol(pspl2dev) ){
      res =  spl2_init_spl3device(pspl2dev, paaldevid);
   }else if(PCIE_FEATURE_HDR3_PROTOCOL_CCI3 == spl2_dev_protocol(pspl2dev)){
      res = cci3_init_ccidevice(pspl2dev, paaldevid);
   }else{
      PERR("Invalid protocol %d. Only SPL3 or CCI3 supported\n",spl2_dev_protocol(pspl2dev));
      res = -ENXIO;
   }
ERR:
   PTRACEOUT_INT(res);
   return res;
}


/// spl2_pci_remove_and_rescan - Used to force a rescan of the device
/// @index: zero based index indicating which board to remove and rescan
///
/// Searches through @g_device_list to find a board at the index entry in the device
/// list and then removes it. Then a rescan on teh parent bus is issued.
static void
spl2_pci_remove_and_rescan(unsigned index)
{
   struct list_head   *This     		= NULL;
   struct list_head   *tmp      		= NULL;
   struct spl2_device *pspl2dev 		= NULL;
   struct pci_dev     *pcidev 			= NULL;
   unsigned int bustype					= 0;
   unsigned cnt 						= index;

   PTRACEIN;

   // Search through our list of devices to find the one matching pcidev
   if ( !list_empty(&g_device_list) ) {

      // Run through list of devices.  Use safe variant
      //  as we will be deleting entries
      list_for_each_safe(This, tmp, &g_device_list) {

         pspl2dev = spl2_list_to_spl2_device(This);

         // If this is
         if( 0 == cnt ) {

        	struct pci_bus *parent = NULL;
        	struct aal_device * paaldev = spl2_dev_to_aaldev(pspl2dev);

        	noprobe =1;

        	// Save device information
       	    pcidev = spl2_dev_pci_dev(pspl2dev);
       	    bustype = spl2_dev_board_type(pspl2dev);

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
   PDEBUG("No SPL2 device at index %d\n", index);
DONE:

   PTRACEOUT;
}



/// spl2_pci_remove - Entry point called when a device registered with the PCIe
///                   subsystem is being removed.
/// @pcidev: kernel-provided device pointer.
///
/// Searches through @g_device_list to find any spl2dev which has a cached
/// pointer to @pcidev and removes/destroys any found.
static
void
spl2_pci_remove(struct pci_dev *pcidev)
{
   struct spl2_device *pspl2dev = NULL;
   struct list_head   *This     = NULL;
   struct list_head   *tmp      = NULL;

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

         pspl2dev = spl2_list_to_spl2_device(This);

         if ( pspl2dev->m_pcidev == pcidev ) {
            ++found;

            PDEBUG("Deleting device 0x%p with list head 0x%p from list 0x%p\n",
                      pspl2dev, This, &g_device_list);

            // Remove from the list
            list_del(This);

            spl2_device_destroy(pspl2dev);
            kfree(pspl2dev);
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
/////////////////             SERVICE INTERFACES           ////////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
//=============================================================================
//
// The AAL Device Kernel Service Module (DKSM) implements a Physical Interface
// Protocol (PIP) Kernel Service Object.  In AAL, a Kernel Service Object is
// one that registers its interface with the AAL Bus Service Interface Broker.
//
// The PIP Object is implemented as a struct that implements or extends the
//  m_ipip struct type.  The m_ipip contains methods for:
//   - binding/unbinding a device to a PIP. The PIP implements the device
//       specific signaling protocol. Typically the PIP
//       will be specific to a particular class of physical device.
//   - the message handler for the PIP. The implementation of the AFU sits
//       behind the PIP message handler.
//       Commands to the AFU get routed to the implementation through the
//       PIP message Handler.
// As an AAL Kernel Service Object, each PIP's interface is wrapped in an
// aal_interface wrapper and registered with AAL Bus Interface Broker Service.
//=============================================================================
//=============================================================================


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////                SPL2 AFU                  ////////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
//=============================================================================

// Used when registering service interfaces with AAL bus. This is a wrapper
//  for interfaces registered with the AAL Bus' Service Interface Broker
struct aal_interface splafu_pip_interface;
struct aal_interface mafu_pip_interface;
struct aal_interface cciafu_pip_interface;

// The PIP which is the interface to the encoder object
extern struct aal_ipip SPLAFUpip;
extern struct aal_ipip CCIAFUpip;


struct memmgr_internal_fops spl2mem_fops = {
   .alloc            = memmgr_internal_alloc,
   .free             = memmgr_internal_free,
   .open             = memmgr_internal_open,
   .close            = memmgr_internal_close,
   .ioctl            = memmgr_internal_ioctl,
   .mmap             = memmgr_internal_mmap,
   .ioctl_put_config = internal_ioctl_put_config,
   .ioctl_getconfig  = internal_ioctl_get_config,
   .ioctl_alloc      = internal_ioctl_alloc,
   .ioctl_valloc     = internal_ioctl_valloc,
   .ioctl_free       = internal_ioctl_free
};


//=============================================================================
// Name: spl2pip_init
// Description: Entry point called when the module is loaded
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments: none.
//=============================================================================

/// spl2pip_init - Entry point called when the module is loaded.
///
/// Initializes the module device list and performs either manual or automatic
/// configuration, based on the value of module param @nopcie.
/// Registers the module AFU/MAFU interfaces.
static int
spl2pip_init(void)
{
   int res = 0;
   struct aal_device_id aaldevid;
   struct spl2_device  *pspl2dev = NULL;

   //--------------------
   // Display the sign-on
   //--------------------
   PTRACEIN;

   PINFO("Intel(R) QuickAssist Technology Accelerator Abstraction Layer\n");
   PINFO("-> %s\n",         DRV_DESCRIPTION);
   PINFO("-> Version %s\n", DRV_VERSION);
   PINFO("-> License %s\n", DRV_LICENSE);
   PINFO("-> %s\n",       DRV_COPYRIGHT);

   // Initialize the list of devices
   INIT_LIST_HEAD(&g_device_list);

   PINFO("Using %s configuration.\n", (0 == nopcie) ? "Automatic" : "Manual");

   if ( 0 == nopcie ) {
      int res =0;

      // If module param nopcie is zero (default), then attempt to register with
      // the kernel PCIe subsystem.

      res = pci_register_driver(&spl2_pci_driver_info.pcidrv);
      ASSERT(0 == res);
      if( 0 != res ) {
         PERR("Failed to register PCI driver. (%d)\n", res);
         goto ERR;
      }
      spl2_pci_driver_info.isregistered = 1;

      if ( driver_create_file(&spl2_pci_driver_info.pcidrv.driver,&driver_attr_rescan_index) ) {
          PDEBUG("Failed to create rescan_index attribute - Unloading module\n");
          goto ERR;
      }

      res = aalbus_get_bus()->init_driver((kosal_ownermodule *)THIS_MODULE,
                                         &spl2_pci_driver_info.aaldriver,
                                         &spldrv_class,
                                          "cci-splpip",  //TODO do this right
                                          0);
      ASSERT(res >= 0);

      if(driver_create_file(&spl2_pci_driver_info.aaldriver.m_driver,&driver_attr_debug)){
          DPRINTF (AHMPIP_DBG_MOD, ": Failed to create debug attribute - Unloading module\n");
          // Unregister the driver with the bus
          aalbus_get_bus()->unregister_driver( &spl2_pci_driver_info.aaldriver );
          return -EIO;
      }

   } else {
      // Passing a non-zero value for nopcie at insmod time causes the driver to fall
      // back to manual configuration.
      pspl2dev = kzalloc(sizeof(struct spl2_device), GFP_KERNEL);
      if ( NULL == pspl2dev ) {
         res = -ENOMEM;
         goto ERR;
      }

      memset(&aaldevid, 0, sizeof(aaldevid));

      spl2_dev_len_config(pspl2dev) = QPI_APERTURE_SIZE;

      //  1: Try Emerald Ridge then Romley
      //  2: Use Emerald Ridge Base address (0x88080000)
      //  3: Use Romley Base address (0xC8080000)
      //  4: Simulated hardware
      // >4: use it as an address.

      switch ( nopcie ) {

         case 1 : { // Attempt dynamic configuration
            spl2_dev_phys_config(pspl2dev) = NHM_APERTURE_PHYS;
            res = spl2_nopcie_internal_probe(pspl2dev, &aaldevid);
            if( res ) {
               spl2_dev_phys_config(pspl2dev) = ROM_APERTURE_PHYS;
               res = spl2_nopcie_internal_probe(pspl2dev, &aaldevid);
            }
         } break;

         case 2 : { // Nehalem Emerald Ridge
            spl2_dev_phys_config(pspl2dev) = NHM_APERTURE_PHYS;
            res = spl2_nopcie_internal_probe(pspl2dev, &aaldevid);
         } break;

         case 3 : { // Romley Emerald Ridge
            spl2_dev_phys_config(pspl2dev) = ROM_APERTURE_PHYS;
            res = spl2_nopcie_internal_probe(pspl2dev, &aaldevid);
         } break;

         case 4 : { // Create simulated device
            PINFO("Creating a simulated device\n");
            res = spl2_sim_internal_probe(pspl2dev, &aaldevid);
         } break;

         default : { // Go for broke and try the passed in value -- maybe BOOM
            spl2_dev_phys_config(pspl2dev) = __PHYS_ADDR_CAST(nopcie);
            res = spl2_nopcie_internal_probe(pspl2dev, &aaldevid);
         } break;

      }

      ASSERT(0 == res);
      if ( 0 != res ) {
         PERR("Failed to manually detect board %d\n", res);
         spl2_device_destroy(pspl2dev);
         kfree(pspl2dev);
         goto ERR;
      }

   }

   //---------------------------------------------------------------
   // Create and register the service interfaces
   //  Initialize the aal_interface container struct with the
   //  service interface and the ID. Then register the interface
   //  with the AAL Bus Service Interface Broker. The aal_interface
   //  object keeps a reference count of users of the interface so
   //  that it cannot be destroyed while still held.
   //---------------------------------------------------------------

   //---------------------------------------------------------------
   // Create and register the AFU PIP service interface
   //---------------------------------------------------------------
   PVERBOSE("Registering SPL2 AFU/MAFU PIP services\n");

   // Initialize the aal_interface
   aal_interface_init(splafu_pip_interface, // Interface container
                      &SPLAFUpip,           // PIP Service interface (Vtable)
                      SPL2_AFUPIP_IID);  // Interface ID

   // Register with the service interface broker
   PVERBOSE("Registering service interface 0x%Lx\n", (long long)SPL2_AFUPIP_IID);

   res = aalbus_get_bus()->register_service_interface(&splafu_pip_interface);
   ASSERT(res >= 0);
   if ( res < 0 ) {
      PERR("Failed to register SPL2 AFU PIP service interface.\n");
      goto ERR;
   }

   // Initialize the aal_interface
   aal_interface_init(cciafu_pip_interface, // Interface container
                      &CCIAFUpip,           // PIP Service interface (Vtable)
                      QPI_CCIAFUPIP_IID);   // Interface ID

   // Register with the service interface broker
   PVERBOSE("Registering service interface 0x%Lx\n", (long long)QPI_CCIAFUPIP_IID);

   res = aalbus_get_bus()->register_service_interface(&cciafu_pip_interface);
   ASSERT(res >= 0);
   if ( res < 0 ) {
      PERR("Failed to register CCI AFU PIP service interface.\n");
      goto ERR;
   }

   // Initialize the aal_interface
   aal_interface_init(mafu_pip_interface, // Interface container
                      &MAFUpip,           // PIP Service interface (Vtable)
                      SPL2_MAFUPIP_IID);  // Interface ID

   // Register with the service interface broker
   PVERBOSE("Registering service interface 0x%Lx\n", (long long)SPL2_MAFUPIP_IID);

   res = aalbus_get_bus()->register_service_interface(&mafu_pip_interface);
   ASSERT(res >= 0);
   if ( res < 0 ) {
      PERR("Failed register SPL2 MAFU PIP service interface.\n");
      goto ERR;
   }

   // If manual enumeration initialize discovered devices
   if(0 != nopcie){
      PVERBOSE("Instantiating manually enumerated device\n");
      if(NULL == pspl2dev){
         return -ENXIO;
      }
      // Now that PIPs are registered
      if(PCIE_FEATURE_HDR3_PROTOCOL_SPL == spl2_dev_protocol(pspl2dev) ){
         PVERBOSE("Readying SPL Device\n");
         res = spl2_spl_device_init(pspl2dev, &aaldevid, &g_device_list);

         if( 0 != res ) {
            spl2_device_destroy(pspl2dev);
            kfree(pspl2dev);
            goto ERR;
             }

      }else if(PCIE_FEATURE_HDR3_PROTOCOL_CCI3 == spl2_dev_protocol(pspl2dev)){
         PVERBOSE("Readying CCI Device\n");
         res = spl2_cci_device_init(pspl2dev, &aaldevid, &g_device_list);
         if ( 0 != res ) {
            spl2_device_destroy(pspl2dev);
            kfree(pspl2dev);
            goto ERR;
         }
      }else{
         res = -ENXIO;
      }
   }

   PTRACEOUT_INT(res);
   return res;

ERR:
   // TODO error path needs to deal with all possibilitied. E.g., device initialized but interface reg failed need to destroy
   if ( aalinterface_is_registered(&mafu_pip_interface) ) {
      aalbus_get_bus()->unregister_service_interface(&mafu_pip_interface);
   }

   if ( aalinterface_is_registered(&splafu_pip_interface) ) {
      aalbus_get_bus()->unregister_service_interface(&splafu_pip_interface);
   }

   if ( aalinterface_is_registered(&cciafu_pip_interface) ) {
      aalbus_get_bus()->unregister_service_interface(&cciafu_pip_interface);
   }

   if ( spl2_pci_driver_info.isregistered ) {
      pci_unregister_driver(&spl2_pci_driver_info.pcidrv);
      spl2_pci_driver_info.isregistered = 0;
   }

   PTRACEOUT_INT(res);
   return res;
}

/// spl2pip_exit - Exit function when unloading the module.
///
/// Walks @g_device_list, destroying any device not registered with
/// the PCIe subsystem.
/// Unregisters the module AFU/MAFU interfaces.
/// Unregisters the driver with the PCI subsystem, if needed.
static
void
spl2pip_exit(void)
{
   struct spl2_device *pspl2dev = NULL;
   struct list_head   *This     = NULL;
   struct list_head   *tmp      = NULL;

   PTRACEIN;

   // Remove/destroy any devices that were not registered with the PCIe subsystem.

   if ( !list_empty(&g_device_list) ) {

      // Run through list of devices.  Use safe variant
      //  as we will be deleting entries
      list_for_each_safe(This, tmp, &g_device_list) {

         pspl2dev = spl2_list_to_spl2_device(This);

         down( &pspl2dev->m_sem );

         if ( !spl2_dev_pci_dev_is_enabled(pspl2dev) ) {

            // Remove from the list
            list_del(This);

            up( &pspl2dev->m_sem );

            PDEBUG("<- Deleting device 0x%p with list head 0x%p from list 0x%p\n",
                      pspl2dev, This, &g_device_list);

            spl2_device_destroy(pspl2dev);
            kfree(pspl2dev);

         } else {
            up( &pspl2dev->m_sem );
         }

      }

   }

   if ( aalinterface_is_registered(&mafu_pip_interface) ) {
      aalbus_get_bus()->unregister_service_interface(&mafu_pip_interface);
   }

   if ( aalinterface_is_registered(&splafu_pip_interface) ) {
      aalbus_get_bus()->unregister_service_interface(&splafu_pip_interface);
   }

   if ( aalinterface_is_registered(&cciafu_pip_interface) ) {
      aalbus_get_bus()->unregister_service_interface(&cciafu_pip_interface);
   }
   if ( spl2_pci_driver_info.isregistered ) {
      pci_unregister_driver(&spl2_pci_driver_info.pcidrv);
      spl2_pci_driver_info.isregistered = 0;
   }

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
#if ENABLE_DEBUG
   struct aal_device *paaldev = basedev_to_aaldev(pdev);
#endif // ENABLE_DEBUG

   PTRACEIN;

   PDEBUG("Called with struct aal_device * 0x%p\n", paaldev);

   // DO NOT call factory release here. It will be done by the framework.
   PTRACEOUT;
}

