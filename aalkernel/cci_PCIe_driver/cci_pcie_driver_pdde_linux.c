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
//        FILE: cci_pcie_driver_pdde_linux.c
//     CREATED: 10/14/2015
//      AUTHOR: Joseph Grecco, Intel <joe.grecco@intel.com>
//              Ananda Ravuri, Intel <ananda.ravuri@intel.com>
// PURPOSE: This file implements the Physical Device Discovery and Enumeration
//          functionality of the Intel(R) Intel QuickAssist Technology AAL
//          FPGA device driver.
// HISTORY:
// COMMENTS: Linux specific
// WHEN:          WHO:     WHAT:
// 10/14/2015    JG       Initial version started
//****************************************************************************
#include "aalsdk/kernel/kosal.h"

#define MODULE_FLAGS CCIPCIE_DBG_MOD // Prints all

#include "aalsdk/kernel/aalbus.h"
#include "aalsdk/kernel/aalinterface.h"
#include "aalsdk/kernel/ccip_defs.h"
//#include "aalsdk/kernel/aalids.h"
//#include "aalsdk/kernel/aalrm.h"
//#include "aalsdk/kernel/aalqueue.h"

#include "cci_pcie_driver_internal.h"

#include "cci_pcie_driver_simulator.h"

#include "ccip_fme.h"
#include "ccip_port.h"
#include "ccip_perfmon.h"

//#include "aalsdk/kernel/spl2defs.h"

extern int print_sim_fme_device(struct fme_device *);
extern int print_sim_port_device(struct port_device *pport_dev);

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
#if 1
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

static
struct ccip_device *
cci_pcie_stub_probe( struct pci_dev             *pcidev,
                     const struct pci_device_id *pcidevid);

static
struct ccip_device *
cci_enumerate_device( struct pci_dev             *pcidev,
                     const struct pci_device_id *pcidevid);

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
   { PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCIe_DEVICE_ID_RCiEP0   ), .driver_data = (kernel_ulong_t)cci_enumerate_device },
   { PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCIe_DEVICE_ID_RCiEP1),    .driver_data = (kernel_ulong_t)cci_pcie_stub_probe },
   { PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCIe_DEVICE_ID_RCiEP2),    .driver_data = (kernel_ulong_t)cci_pcie_stub_probe },
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
      .name     = CCI_PCI_DRIVER_NAME,
      .id_table = cci_pcie_id_tbl,
      .probe    = cci_pci_probe,
      .remove   = cci_pci_remove,
   },
};

/// Signature of CCI device specific probe function
typedef struct ccip_device * (*cci_probe_fn)( struct pci_dev                * ,
                                              const struct pci_device_id    * );


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
   return NULL;
}

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
   ASSERT(NULL != probe_fn);
   if ( NULL == probe_fn ) {
       PERR("NULL cci_enumerate function from probe\n");
       return res;
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

   // enable bus mastering (if not already set)
   pci_set_master(pcidev);
   pci_save_state(pcidev);

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
         pportdev = get_port_device( pbarPhyAddr + offset, ccip_portdev_kvp_afu_mmio(pccipdev,bar) + offset );
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

         PDEBUG("Creating Allocatable objects\n");

         // Instantiate allocatable objects including AFUs if present.
         //   Subdevice addresses start at 10x the 1 based port number to leave room for
         //   10 devices beneath the port. E.e., STAP, PR, User AFU
         if(!cci_port_dev_create_AAL_allocatable_objects(pportdev, (i+1)) * 10){
            goto ERR;
         }
      }// End for loop
   }

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
      list_for_each_safe(This, tmp, &g_device_list) {

         pccidev = ccip_list_to_ccip_device(This);

         if ( pccidev->m_pcidev == pcidev ) {
            ++found;

            PDEBUG("Deleting device 0x%p with list head 0x%p from list 0x%p\n",
                  pccidev, This, &g_device_list);
            cci_remove_device(pccidev);
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
   for(x=1; x<5; x++){
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
         iounmap(ccip_fmedev_kvp_afu_mmio(pccipdev));
         pci_release_region(ccip_dev_to_pci_dev(pccipdev), 0);
      }else{
         kosal_kfree(ccip_fmedev_kvp_afu_mmio(pccipdev),ccip_fmedev_len_afu_mmio(pccipdev) );
      }
      ccip_fmedev_kvp_afu_mmio(pccipdev) = NULL;
   }

   if( cci_dev_pci_dev_is_enabled(pccipdev) ) {
      if(!ccip_is_simulated(pccipdev)){
         PVERBOSE("Disabling PCIe device\n");
         pci_disable_device(cci_dev_pci_dev(pccipdev));
      }
      cci_dev_pci_dev_clr_enabled(pccipdev);
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
   struct ccip_device *pccidev      = NULL;
   struct list_head   *This         = NULL;
   struct list_head   *tmp          = NULL;

   PTRACEIN;

   // Remove/destroy any devices that were not registered with the PCIe subsystem.

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

   //aalbus_get_bus()->unregister_driver( &cci_pci_driver_info.aaldriver );

   PINFO("<- %s removed.\n", DRV_DESCRIPTION);
   PTRACEOUT;
}

