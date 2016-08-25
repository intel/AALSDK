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
//  Copyright(c) 2011-2016, Intel Corporation.
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
//        FILE: cci_pcie_driver_simulator.c
//     CREATED: Oct 14, 2015
//      AUTHOR: Joseph Grecco, Intel <joe.grecco@intel.com>
//
// PURPOSE:   This file contains the implementation of the simulated CCI
//            device
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
//****************************************************************************///
#include "aalsdk/kernel/kosal.h"

#define MODULE_FLAGS CCIPCIE_DBG_MOD

#include "aalsdk/kernel/aalids.h"
#include "aalsdk/kernel/aalbus.h"
#include "aalsdk/kernel/ccip_defs.h"

#include "cci_pcie_driver_PIPsession.h"

#include "aalsdk/kernel/ccipdriver.h"
#include "aalsdk/kernel/iaaldevice.h"


#include "ccip_fme.h"
#include "ccip_port.h"
#include "cci_pcie_driver_simulator.h"
#include "ccipdrv-events.h"


extern int  ccip_sim_wrt_port_mmio(btVirtAddr);
extern int  ccip_sim_wrt_fme_mmio(btVirtAddr);

extern int print_sim_fme_device(struct fme_device *);
extern int print_sim_port_device(struct port_device *);

int cci_create_sim_afu(btVirtAddr,unsigned ,struct aal_device_id*,kosal_list_head *);


static
struct ccip_device * cci_enumerate_simulated_device( btVirtAddr bar0,
                                                     btVirtAddr bar2,
                                                     struct aal_device_id *pdevid);
//=============================================================================
// nextAFU_addr - Keeps the next available address for new AFUs
//=============================================================================
static struct aal_device_addr nextAFU_addr = {
   .m_bustype   = aal_bustype_Host,
   { .m_busnum  = 1 },     //
   .m_devicenum = 0,       //
   .m_functnum  = 1,       //
   .m_subdevnum = 0        // AFU
};


//=============================================================================
// Name: cci_sim_alloc_next_afu_addr
// Description: Allocate the next AFU address
// Interface: public
// Returns 0 - success
// Inputs: none
// Outputs: none.
// Comments: Allocates sequential addresses.  This is a hack for simulation
//           but is adequate.
//=============================================================================
struct aal_device_addr
cci_sim_alloc_next_afu_addr(void)
{
   ++(nextAFU_addr.m_devicenum);
   if( 0 == (nextAFU_addr.m_devicenum &= 0xffff) ) {
      ++(nextAFU_addr.m_busnum);
      nextAFU_addr.m_busnum &= 0xffff;
   }
   return nextAFU_addr;
} // cci_alloc_next_afu_addr


//=============================================================================
// Name: cci_sim_discover_devices
// Description: Performs the functionality of the PCIe OS enumeration functions
//              for real hardware.
// Inputs: numdevices - Number of AFUs to "discover".
//         g_device_list - List head of global device list.
// Outputs: 0 - success.
// Comments: none.
//=============================================================================
int cci_sim_discover_devices(unsigned numdevices,
                             kosal_list_head *pg_device_list)
{
   struct aal_device_id aalid;
   btVirtAddr           bar0, bar2        = NULL;

   PVERBOSE("Creating %d simulated CCI devices", numdevices);

   // Loop through and probe each simulated device
   while(numdevices--){
      struct ccip_device *pccidev = NULL;

      // Allocate the BAR for the simulated device
      bar0 = kosal_kzmalloc( CCI_SIM_APERTURE_SIZE );
      if(NULL == bar0){
         PERR("Unable to allocate system memory for simulated BAR 0\n");
         return -EINVAL;
      }

      // Allocate the BAR for the simulated device
      bar2 = kosal_kzmalloc( CCI_SIM_APERTURE_SIZE );
      if(NULL == bar2){
         PERR("Unable to allocate system memory for simulated BAR 2\n");
         if(bar0){
            kosal_kfree(bar0, CCI_SIM_APERTURE_SIZE);
         }
         return -EINVAL;
      }

      // Create Channel AFU for this device
      aaldevid_addr(aalid) = cci_sim_alloc_next_afu_addr();

      aaldevid_devaddr_bustype(aalid) = aal_bustype_Host;

      // Enumerate the features of the simulated device
      pccidev = cci_enumerate_simulated_device( bar0,
                                                bar2,
                                                &aalid);

      // If all went well record this device on our
      //  list of devices owned by the driver
      if(NULL == pccidev){
         kosal_kfree(bar0, CCI_SIM_APERTURE_SIZE);
         kosal_kfree(bar2, CCI_SIM_APERTURE_SIZE);
         continue;
      }

      kosal_list_add(&(pccidev->m_list), pg_device_list);

   }// end while

   if(kosal_list_is_empty(pg_device_list)){
      return -EIO;
   }
   return 0;
}


//=============================================================================
// Name: cci_enumerate_simulated_device
// Description: Called during the device probe to initiate the enumeration of
//              the device attributes and construct the internal objects.
// Inputs: pcidev - kernel-provided device pointer.
//         pcidevid - kernel-provided device id pointer.
// Returns: 0 = success.
// Comments:
//=============================================================================
static
struct ccip_device * cci_enumerate_simulated_device( btVirtAddr bar0,
                                                     btVirtAddr bar2,
                                                     struct aal_device_id *pdevid)
{

   struct ccip_device * pccipdev       = NULL;

   int                  res            = 0;

   PTRACEIN;

   // Check arguments
   ASSERT(bar0);
   if ( NULL == bar0 ) {
      PTRACEOUT_INT(-EINVAL);
      return NULL;
   }
   ASSERT(bar2);
   if ( NULL == bar2 ) {
      PTRACEOUT_INT(-EINVAL);
      return NULL;
    }
   ASSERT(pdevid);
   if ( NULL == pdevid ) {
      PTRACEOUT_INT(-EINVAL);
      return NULL;
    }

   // Create the CCI device object
   pccipdev = create_ccidevice();
   ccip_dev_pci_dev(pccipdev) = NULL;

   // Save the Bus:Device:Function of simulated device
   ccip_dev_pcie_bustype(pccipdev)  = aal_bustype_Host;
   ccip_dev_pcie_busnum(pccipdev)   = aaldevid_devaddr_busnum(*pdevid);
   ccip_dev_pcie_devnum(pccipdev)   = aaldevid_devaddr_devnum(*pdevid);
   ccip_dev_pcie_fcnnum(pccipdev)   = aaldevid_devaddr_fcnnum(*pdevid);

   // Mark this device as simulated
   ccip_set_simulated(pccipdev);

   // FME Device initialization
   //---------------------------

   // Save the FME information in the CCI Device object
   ccip_fmedev_phys_afu_mmio(pccipdev)    = __PHYS_ADDR_CAST(kosal_virt_to_phys(bar0));
   ccip_fmedev_len_afu_mmio(pccipdev)     = CCI_SIM_APERTURE_SIZE;
   ccip_fmedev_kvp_afu_mmio(pccipdev)     = bar0;

   PDEBUG("ccip_fmedev_phys_afu_mmio(pccipdev) : %lx\n", ccip_fmedev_phys_afu_mmio(pccipdev));
   PDEBUG("ccip_fmedev_kvp_afu_mmio(pccipdev) : %lx\n",(long unsigned int) ccip_fmedev_kvp_afu_mmio(pccipdev));
   PDEBUG("ccip_fmedev_len_afu_mmio(pccipdev): %zu\n", ccip_fmedev_len_afu_mmio(pccipdev));

   // Now populate the simulated MMIO
   ccip_sim_wrt_fme_mmio(ccip_fmedev_kvp_afu_mmio(pccipdev));

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

      // Instantiate allocatable objects
      if(!cci_fme_dev_create_AAL_allocatable_objects(pccipdev)){
         goto ERR;
      }


      // Creates AAL Power device object
      if(!cci_create_AAL_power_Device(pccipdev)){
         goto ERR;
      }

      // print FME MMIO
      print_sim_fme_device(ccip_dev_to_fme_dev(pccipdev));

   } // End FME region

   // Port Device initialization
   //---------------------------

   // Save the Port information in the CCI Device object

   ccip_portdev_phys_afu_mmio(pccipdev,0)    = __PHYS_ADDR_CAST(kosal_virt_to_phys(bar2));
   ccip_portdev_len_afu_mmio(pccipdev,0)     = CCI_SIM_APERTURE_SIZE;
   ccip_portdev_kvp_afu_mmio(pccipdev,0)     = bar2;

   PDEBUG("ccip_portdev_phys_afu_mmio(pccipdev) : %" PRIxPHYS_ADDR "\n", ccip_portdev_phys_afu_mmio(pccipdev,0));
   PDEBUG("ccip_portdev_len_afu_mmio(pccipdev): %zu\n", ccip_portdev_len_afu_mmio(pccipdev,0));
   PDEBUG("ccip_portdev_kvp_afu_mmio(pccipdev) : %p\n", ccip_portdev_kvp_afu_mmio(pccipdev,0));
   PDEBUG("End of Port Space %p\n", ccip_portdev_kvp_afu_mmio(pccipdev,0) + CCI_SIM_APERTURE_SIZE);
   // Now populate the simulated Port MMIO
   ccip_sim_wrt_port_mmio(ccip_portdev_kvp_afu_mmio(pccipdev,0));

   // Enumerate the devices
   //  Loop through each Port Offset register to determine
   //  if a Port has been implemented and where its resources are.
   //  Since this is a simulated device all of the Ports are in the
   //  ccip_portdev_kvp_afu_mmio() location.  In real hardware the
   //  resources may reside in different Bars and at different offsets.
   //  The driver must keep track of all resources it claims so it can
   //  free them later.
   //------------------------------------------------------------------
   {
      struct fme_device  *pfme_dev  = ccip_dev_to_fme_dev(pccipdev);
      struct CCIP_FME_HDR *pfme_hdr = ccip_fme_hdr(pfme_dev);
      struct port_device *pportdev  = NULL;
      int i=0;

      for(i=0;  0!= pfme_hdr->port_offsets[i].port_imp  ;i++){

         PINFO("***** PORT %d MMIO region @ Bar %d offset %x *****\n",i , pfme_hdr->port_offsets[i].port_bar, pfme_hdr->port_offsets[i].port_offset);

         // Discover and create Port device
         //   Enumerates the Port feature list, creates the Port object.
         //   Then add the new port object onto the list
         //-------------------------------------------------------------
         pportdev = get_port_device( kosal_virt_to_phys(ccip_portdev_kvp_afu_mmio(pccipdev,0)) + pfme_hdr->port_offsets[i].port_offset,
                                     ccip_portdev_kvp_afu_mmio(pccipdev,0) + pfme_hdr->port_offsets[i].port_offset,
                                     ccip_portdev_len_afu_mmio(pccipdev,0));
         if ( NULL == pportdev ) {
            PERR("Could not allocate memory for FME object\n");
            res = -ENOMEM;
            goto ERR;
         }

         ccip_set_resource(pccipdev, pfme_hdr->port_offsets[i].port_bar);

         PDEBUG("Created Port Device\n");
         // Point to our parent
         ccip_port_to_ccidev(pportdev) = pccipdev;

         // Inherits B:D:F from board
         ccip_port_bustype(pportdev)   = ccip_dev_pcie_bustype(pccipdev);
         ccip_port_busnum(pportdev)    = (btUnsigned16bitInt) ccip_dev_pcie_busnum(pccipdev);
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

         // Instantiate allocatable objects including AFUs if present. Port subdevice address is 0 based.
         if(!cci_port_dev_create_AAL_allocatable_objects(pportdev, i)){
            goto ERR;
         }
      }// End for

   }// end block

   return pccipdev;
ERR:

   PERR(" -----ERROR -----   \n");

   if( NULL != ccip_fmedev_kvp_afu_mmio(pccipdev)) {
      if(ccip_dev_to_fme_dev(pccipdev)){
         ccip_destroy_fme_mmio_dev(ccip_dev_to_fme_dev(pccipdev));
      }
      ccip_fmedev_kvp_afu_mmio(pccipdev) = NULL;
   }

   if( NULL != ccip_portdev_kvp_afu_mmio(pccipdev,0)) {
      kosal_kfree(ccip_portdev_kvp_afu_mmio(pccipdev,0), ccip_portdev_len_afu_mmio(pccipdev,0) );
      ccip_portdev_kvp_afu_mmio(pccipdev,0) = NULL;
   }

   if ( NULL != pccipdev ) {
      kosal_kfree( pccipdev, sizeof(struct ccip_device) );
   }


   PTRACEOUT_INT(res);
   return NULL;
}

