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
//  Copyright(c) 2011-2015, Intel Corporation.
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

int cci_create_sim_afu(btVirtAddr,uint ,struct aal_device_id*,struct list_head *);

#define CCIV4_MMIO_UMSG_TEST 0

#if CCIV4_MMIO_UMSG_TEST
// Turn on in AFUdev.cpp, as well
static char mmioafustring[]    = "CCIv4 MMIO test  \n";
static char umesgafustring[]   = "CCIv4 UMSG test  \n";
#endif
#if 0
static int
CommandHandler(struct aaldev_ownerSession *,
               struct aal_pipmessage);
int
cci_sim_mmap(struct aaldev_ownerSession *pownerSess,
               struct aal_wsid *wsidp,
               btAny os_specific);
#endif
static
struct ccip_device * cci_enumerate_simulated_device( btVirtAddr bar0,
                                                     btVirtAddr bar2,
                                                     struct aal_device_id *pdevid);
#if 0
//=============================================================================
// cci_simAFUpip
// Description: Physical Interface Protocol Interface for the SPL2 AFU
//              kernel based AFU engine.
//=============================================================================
struct aal_ipip cci_simAFUpip = {
   .m_messageHandler = {
      .sendMessage   = CommandHandler,       // Command Handler
      .bindSession   = BindSession,          // Session binder
      .unBindSession = UnbindSession,        // Session unbinder
   },

   .m_fops = {
     .mmap = cci_sim_mmap,
   },

   // Methods for binding and unbinding PIP to generic aal_device
   //  Unused in this PIP
   .binddevice    = NULL,      // Binds the PIP to the device
   .unbinddevice  = NULL,      // Binds the PIP to the device
};
#endif
//=============================================================================
// nextAFU_addr - Keeps the next available address for new AFUs
//=============================================================================
static struct aal_device_addr nextAFU_addr = {
   .m_bustype   = aal_bustype_Host,
   .m_busnum    = 1,       //
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
int cci_sim_discover_devices(ulong numdevices,
                             struct list_head *pg_device_list)
{
   struct aal_device_id aalid;
   btVirtAddr           bar0, bar2        = NULL;

   PVERBOSE("Creating %ld simulated CCI devices", numdevices);

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

   // Save the Bus:Device:Function of simulated device
   ccip_dev_pcie_bustype(pccipdev)  = aal_bustype_Host;
   ccip_dev_pcie_busnum(pccipdev)   = aaldevid_devaddr_busnum(*pdevid);
   ccip_dev_pcie_devnum(pccipdev)   = aaldevid_devaddr_devnum(*pdevid);
   ccip_dev_pcie_fcnnum(pccipdev)   = aaldevid_devaddr_fcnnum(*pdevid);

   // Mark thsi device as simulated
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
         pportdev = get_port_device( virt_to_phys(ccip_portdev_kvp_afu_mmio(pccipdev,0)) + pfme_hdr->port_offsets[i].port_offset,
                                     ccip_portdev_kvp_afu_mmio(pccipdev,0) + pfme_hdr->port_offsets[i].port_offset);
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
         ccip_port_busnum(pportdev)    = ccip_dev_pcie_busnum(pccipdev);
         ccip_port_devnum(pportdev)    = ccip_dev_pcie_devnum(pccipdev);
         ccip_port_fcnnum(pportdev)    = ccip_dev_pcie_fcnnum(pccipdev);

         // Log the Port MMIO
         print_sim_port_device(pportdev);

         PDEBUG("Adding to list\n");

         // Added it to the port list
         kosal_list_add(&ccip_port_dev_list(pccipdev), &ccip_port_list_head(pportdev));

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
         kfree(pccipdev);
   }


   PTRACEOUT_INT(res);
   return NULL;
}
#if 0
//=============================================================================
// Name: cci_create_sim_afu
// Description: Performs the functionality of the PCIe Probe functions for real
//              hardware. It causes AAL Devices to be instantiated for the
//              number of simulated devices "discovered".
// Inputs: virtAddr - Virtual Address of aperture.
//         size - size of aperture.
//         g_device_list - List head of global device list.
// Outputs: 0 - success.
// Comments: The steps are:
//               Create the CCI device object.
//               Set the CCI attributes (e.g., MMIO regions)
//               Attach an aal_device to the CCI device (used by framework)
//               Publish the new device with the framework
//               Add the device to the driver's global list
//=============================================================================
int cci_create_sim_afu( btVirtAddr virtAddr,
                        uint size,
                        struct aal_device_id *paalid,
                        struct list_head *pdevice_list)
{
   struct cci_aal_device   *pcci_aaldev   = NULL;
   btVirtAddr               ptemp         = NULL;
   int                      ret           = 0;

   //=============================================================
   // Create the CCI device structure. The CCI device is the class
   // used by the Low Level Communications (PIP). It holds the
   // hardware specific attributes.

   // Construct the cci_aal_device object
   pcci_aaldev = cci_create_aal_device();

   //========================================================
   // Set up the cci_aal_device attributes used by the driver
   //

   // Set up Simulated Config Space
   cci_dev_len_config(pcci_aaldev)  = size;
   cci_dev_kvp_config(pcci_aaldev)  = virtAddr;
   cci_dev_phys_config(pcci_aaldev) = virt_to_phys(cci_dev_kvp_config(pcci_aaldev));

   // Set CCI CSR Attributes
   cci_dev_phys_cci_csr(pcci_aaldev) = cci_dev_phys_config(pcci_aaldev);
   cci_dev_kvp_cci_csr(pcci_aaldev)  = cci_dev_kvp_config(pcci_aaldev);
   cci_dev_len_cci_csr(pcci_aaldev)  = CCI_SIM_APERTURE_SIZE;

   // Allocate uMSG space
   ptemp = kosal_kzmalloc(CCI_UMSG_SIZE);
   if ( NULL == ptemp ) {
      PERR("Unable to allocate system memory for uMsg area\n");
      cci_destroy_aal_device(pcci_aaldev);
      return -ENOMEM;
   }

#if CCIV4_MMIO_UMSG_TEST
   strncpy((char*)ptemp,umesgafustring,strlen(umesgafustring));
#endif

   cci_dev_len_afu_umsg(pcci_aaldev) = size;
   cci_dev_kvp_afu_umsg(pcci_aaldev) = ptemp;
   cci_dev_phys_afu_umsg(pcci_aaldev) = virt_to_phys(ptemp);

   // Allocate MMIO space
   ptemp = kosal_kzmalloc(CCI_MMIO_SIZE);
   if ( NULL == ptemp ) {
      PERR("Unable to allocate system memory for MMIO area\n");
      kosal_kfree(cci_dev_kvp_afu_umsg(pcci_aaldev), cci_dev_len_afu_umsg(pcci_aaldev));
      cci_destroy_aal_device(pcci_aaldev);
      return -ENOMEM;
   }

#if CCIV4_MMIO_UMSG_TEST
   strncpy((char*)ptemp,mmioafustring,strlen(mmioafustring));
#endif

   cci_dev_len_afu_mmio(pcci_aaldev) = size;
   cci_dev_kvp_afu_mmio(pcci_aaldev) = ptemp;
   cci_dev_phys_afu_mmio(pcci_aaldev) = virt_to_phys(cci_dev_kvp_afu_mmio(pcci_aaldev));

   // Direct user mode CSR interface not supported
   cci_dev_clr_allow_map_csr_write_space(pcci_aaldev);
   cci_dev_clr_allow_map_csr_read_space(pcci_aaldev);

   // Enable MMIO-R and uMSG space
   cci_dev_set_allow_map_mmior_space(pcci_aaldev);
   cci_dev_set_allow_map_umsg_space(pcci_aaldev);

   // Mark as simulated
   cci_set_simulated(pcci_aaldev);

   //===========================================================
   // Create the AAL device structure. The AAL device is the
   // base class for all devices in AAL. This object is used by
   // the AAL kernel framework. It also provides the pointer to
   // the low level physical interface protocol module (PIP).

   // Create the ID of the AAL device. Used for Resource Management.
   aaldevid_afuguidl(*paalid) = CCI_SIM_AFUIDL;
   aaldevid_afuguidh(*paalid) = CCI_SIM_AFUIDH;
   aaldevid_devtype(*paalid)  = aal_devtypeAFU;
   aaldevid_pipguid(*paalid)  = CCI_SIMAFUPIP_IID;
   aaldevid_vendorid(*paalid) = AAL_vendINTC;
   aaldevid_ahmguid(*paalid)  = HOST_AHM_GUID;

   // Create the AAL device
   cci_aaldev_to_aaldev(pcci_aaldev)  =  aaldev_create( "CCISIMAFU",
                                        paalid,
                                        &cci_simAFUpip);

   //===========================================================
   // Set up the optional aal_device attributes
   //

   // Set how many owners are allowed access to this device simultaneously
   pcci_aaldev->m_aaldev->m_maxowners = 1;

   // Set the config space mapping permissions
   cci_aaldev_to_aaldev(pcci_aaldev)->m_mappableAPI = AAL_DEV_APIMAP_NONE;
   if( cci_dev_allow_map_csr_read_space(pcci_aaldev) ){
      cci_aaldev_to_aaldev(pcci_aaldev)->m_mappableAPI |= AAL_DEV_APIMAP_CSRWRITE;
   }

   if( cci_dev_allow_map_csr_write_space(pcci_aaldev) ){
      cci_aaldev_to_aaldev(pcci_aaldev)->m_mappableAPI |= AAL_DEV_APIMAP_CSRREAD;
   }

   if( cci_dev_allow_map_mmior_space(pcci_aaldev) ){
      cci_aaldev_to_aaldev(pcci_aaldev)->m_mappableAPI |= AAL_DEV_APIMAP_MMIOR;
   }

   if( cci_dev_allow_map_umsg_space(pcci_aaldev) ){
      cci_aaldev_to_aaldev(pcci_aaldev)->m_mappableAPI |= AAL_DEV_APIMAP_UMSG;
   }

   // The PIP uses the PIP context to get a handle to the CCI Device from the generic device.
   aaldev_pip_context(cci_aaldev_to_aaldev(pcci_aaldev)) = (void*)pcci_aaldev;

   // Method called when the device is released (i.e., its destructor)
   //  The canonical release device calls the user's release method.
   //  If NULL is provided then only the canonical behavior is done
   dev_setrelease(cci_aaldev_to_aaldev(pcci_aaldev), cci_release_device);

      // Device is ready for use.  Publish it with the Configuration Management Subsystem
   ret = cci_publish_aaldevice(pcci_aaldev);
   ASSERT(ret == 0);
   if(0> ret){
      PERR("Failed to initialize AAL Device for simulated CCIV4 AFU[%d:%d:%d]",aaldevid_devaddr_busnum(*paalid),
                                                                               aaldevid_devaddr_devnum(*paalid),
                                                                               aaldevid_devaddr_subdevnum(*paalid));
      kosal_kfree(cci_dev_kvp_afu_mmio(pcci_aaldev), cci_dev_len_afu_mmio(pcci_aaldev));
      kosal_kfree(cci_dev_kvp_afu_umsg(pcci_aaldev),cci_dev_len_afu_umsg(pcci_aaldev));
      cci_destroy_aal_device(pcci_aaldev);
      return -EINVAL;
   }

   // Add the device to the device list
   kosal_list_add(&cci_dev_list_head(pcci_aaldev), pdevice_list);

   return 0;
}


//=============================================================================
// Name: CommandHandler
// Description: Implements the PIP command handler
// Interface: public
// Inputs: pownerSess - Session between App and device
//         Message - Message to process
// Outputs: none.
// Comments:
//=============================================================================
static int
CommandHandler(struct aaldev_ownerSession *pownerSess,
               struct aal_pipmessage       Message)
{
#if (1 == ENABLE_DEBUG)
#define AFU_COMMAND_CASE(x) case x : PDEBUG("%s\n", #x);
#else
#define AFU_COMMAND_CASE(x) case x :
#endif // ENABLE_DEBUG

   // Private session object set at session bind time (i.e., when object allocated)
   struct cci_PIPsession *pSess = (struct cci_PIPsession *)aalsess_pipHandle(pownerSess);
   struct cci_aal_device  *pdev  = NULL;

   // Overall return value for this function. Set before exiting if there is an error.
   //    retval = 0 means good return.
   int retval = 0;

   // UI Driver message
   struct aalui_CCIdrvMessage *pmsg = (struct aalui_CCIdrvMessage *) Message.m_message;

   // Used by WS allocation
   struct aal_wsid                        *wsidp            = NULL;

   // if we return a request error, return this.  usually it's an invalid request error.
   uid_errnum_e request_error = uid_errnumInvalidRequest;

   PINFO("In CCI Command handler, AFUCommand().\n");

   // Perform some basic checks while assigning the pdev
   ASSERT(NULL != pSess );
   if ( NULL == pSess ) {
      PDEBUG("Error: No PIP session\n");
      return -EIO;
   }

   // Get the cci device
   pdev = cci_PIPsessionp_to_ccidev(pSess);
   if ( NULL == pdev ) {
      PDEBUG("Error: No device\n");
      return -EIO;
   }

   //=====================
   // Message processor
   //=====================
   switch ( pmsg->cmd ) {
      struct ccipdrv_event_afu_response_event *pafuws_evt       = NULL;
      // Returns a workspace ID for the Config Space
      AFU_COMMAND_CASE(ccipdrv_getMMIORmap) {
         struct ccidrvreq *preq = (struct ccidrvreq *)pmsg->payload;

         if ( !cci_dev_allow_map_mmior_space(pdev) ) {
            PERR("Failed ccipdrv_getMMIOR map Permission\n");
            pafuws_evt = ccipdrv_event_afu_afugetmmiomap_create(pownerSess->m_device,
                                                             0,
                                                             (btPhysAddr)NULL,
                                                             0,
                                                             Message.m_tranID,
                                                             Message.m_context,
                                                             uid_errnumPermission);
            PERR("Direct API access not permitted on this device\n");

            retval = -EPERM;
         } else {

            //------------------------------------------------------------
            // Create the WSID object and add to the list for this session
            //------------------------------------------------------------
            if ( WSID_MAP_MMIOR != preq->ahmreq.u.wksp.m_wsid ) {
               PERR("Failed ccipdrv_getMMIOR map Parameter\n");
               pafuws_evt = ccipdrv_event_afu_afugetmmiomap_create(pownerSess->m_device,
                                                                0,
                                                                (btPhysAddr)NULL,
                                                                0,
                                                                Message.m_tranID,
                                                                Message.m_context,
                                                                uid_errnumBadParameter);
               PERR("Bad WSID on ccipdrv_getMMIORmap\n");

               retval = -EINVAL;
            } else {

               wsidp = ccidrv_getwsid(pownerSess->m_device, preq->ahmreq.u.wksp.m_wsid);
               if ( NULL == wsidp ) {
                  PERR("Could not allocate workspace\n");
                  retval = -ENOMEM;
                  /* generate a failure event back to the caller? */
                  goto ERROR;
               }

               wsidp->m_type = WSM_TYPE_MMIO;
               PDEBUG("Getting CSR %s Aperature WSID %p using id %llx .\n",
                         ((WSID_CSRMAP_WRITEAREA == preq->ahmreq.u.wksp.m_wsid) ? "Write" : "Read"),
                         wsidp,
                         preq->ahmreq.u.wksp.m_wsid);

               PDEBUG("Apt = %" PRIxPHYS_ADDR " Len = %d.\n",cci_dev_phys_cci_csr(pdev), (int)cci_dev_len_cci_csr(pdev));

               // Return the event with all of the appropriate aperture descriptor information
               pafuws_evt = ccipdrv_event_afu_afugetmmiomap_create( pownerSess->m_device,
                                                                   wsidobjp_to_wid(wsidp),
                                                                   cci_dev_phys_cci_csr(pdev),        // Return the requested aperture
                                                                   cci_dev_len_cci_csr(pdev),         // Return the requested aperture size
                                                                   Message.m_tranID,
                                                                   Message.m_context,
                                                                   uid_errnumOK);

               PVERBOSE("Sending ccipdrv_getMMIORmap Event Event ID = %d\n",((struct aalui_WSMEvent*)(pafuws_evt->m_payload))->evtID );

               retval = 0;
            }
         }

         ccidrv_sendevent( aalsess_uiHandle(pownerSess),
                           aalsess_aaldevicep(pownerSess),
                           AALQIP(pafuws_evt),
                           Message.m_context);

         if ( 0 != retval ) {
            goto ERROR;
         }

      } break;



   default: {
      struct ccipdrv_event_afu_response_event *pafuresponse_evt = NULL;

      PDEBUG("Unrecognized command %" PRIu64 " or 0x%" PRIx64 " in AFUCommand\n", pmsg->cmd, pmsg->cmd);

      pafuresponse_evt = ccipdrv_event_afu_afuinavlidrequest_create(pownerSess->m_device,
                                                                  &Message.m_tranID,
                                                                  Message.m_context,
                                                                  request_error);

     ccidrv_sendevent( pownerSess->m_UIHandle,
                       pownerSess->m_device,
                       AALQIP(pafuresponse_evt),
                       Message.m_context);

      retval = -EINVAL;
   } break;
   } // switch (pmsg->cmd)

   ASSERT(0 == retval);

ERROR:
   return retval;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////             CCI SIM PIP MMAP             ////////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
//=============================================================================

//=============================================================================
// Name: csr_vmaopen
// Description: Called when the vma is mapped
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments:
//=============================================================================
#ifdef NOT_USED
static void csr_vmaopen(struct vm_area_struct *pvma)
{
   PINFO("CSR VMA OPEN.\n" );
}
#endif


//=============================================================================
// Name: wksp_vmaclose
// Description: called when vma is unmapped
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments:
//=============================================================================
#ifdef NOT_USED
static void csr_vmaclose(struct vm_area_struct *pvma)
{
   PINFO("CSR VMA CLOSE.\n" );
}
#endif

#ifdef NOT_USED
static struct vm_operations_struct csr_vma_ops =
{
   .open    = csr_vmaopen,
   .close   = csr_vmaclose,
};
#endif


//=============================================================================
// Name: cci_sim_mmap
// Description: Method used for mapping kernel memory to user space. Called by
//              uidrv.
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments: This method front ends all operations that require mapping shared
//           memory. It examines the wsid to determine the appropriate service
//           to perform the map operation.
//=============================================================================
int
cci_sim_mmap(struct aaldev_ownerSession *pownerSess,
               struct aal_wsid *wsidp,
               btAny os_specific)
{

   struct vm_area_struct     *pvma = (struct vm_area_struct *) os_specific;

   struct cci_PIPsession   *pSess = NULL;
   struct cci_aal_device       *pdev = NULL;
   unsigned long              max_length = 0; // mmap length requested by user
   int                        res = -EINVAL;

   ASSERT(pownerSess);
   ASSERT(wsidp);

   // Get the spl2 aal_device and the memory manager session
   pSess = (struct cci_PIPsession *) aalsess_pipHandle(pownerSess);
   ASSERT(pSess);
   if ( NULL == pSess ) {
      PDEBUG("CCIV4 Simulator mmap: no Session");
      goto ERROR;
   }

   pdev = cci_PIPsessionp_to_ccidev(pSess);
   ASSERT(pdev);
   if ( NULL == pdev ) {
      PDEBUG("CCIV4 Simulator mmap: no device");
      goto ERROR;
   }

   PINFO("WS ID = 0x%llx.\n", wsidp->m_id);

   pvma->vm_ops = NULL;

   // Special case - check the wsid type for WSM_TYPE_CSR. If this is a request to map the
   // CSR region, then satisfy the request by mapping PCIe BAR 0.
   if ( WSM_TYPE_MMIO == wsidp->m_type ) {
      void *ptr;
      size_t size;
      switch ( wsidp->m_id )
      {
         case WSID_CSRMAP_WRITEAREA:
            case WSID_CSRMAP_READAREA:
            case WSID_MAP_MMIOR:
            case WSID_MAP_UMSG:
            break;
         default:
            PERR("Attempt to map invalid WSID type %d\n", (int) wsidp->m_id);
            goto ERROR;
      }

      // Verify that we can fulfill the request - we set flags at create time.
      if ( WSID_CSRMAP_WRITEAREA == wsidp->m_id ) {
         ASSERT(cci_dev_allow_map_csr_write_space(pdev));

         if ( !cci_dev_allow_map_csr_write_space(pdev) ) {
            PERR("Denying request to map CSR Write space for device 0x%p.\n", pdev);
            goto ERROR;
         }
      }

      if ( WSID_CSRMAP_READAREA == wsidp->m_id ) {
         ASSERT(cci_dev_allow_map_csr_read_space(pdev));

         if ( !cci_dev_allow_map_csr_read_space(pdev) ) {
            PERR("Denying request to map CSR Read space for device 0x%p.\n", pdev);
            goto ERROR;
         }
      }

      if ( WSID_MAP_MMIOR == wsidp->m_id )
      {
         if ( !cci_dev_allow_map_mmior_space(pdev) ) {
            PERR("Denying request to map cci_dev_allow_map_mmior_space Read space for device 0x%p.\n", pdev);
            goto ERROR;
         }

         ptr = (void *) cci_dev_phys_afu_mmio(pdev);
         size = cci_dev_len_afu_mmio(pdev);

         PVERBOSE("Mapping CSR %s Aperture Physical=0x%p size=%" PRIuSIZE_T " at uvp=0x%p\n",
            ((WSID_CSRMAP_WRITEAREA == wsidp->m_id) ? "write" : "read"),
            ptr,
            size,
            (void *)pvma->vm_start);

         // Map the region to user VM
         res = remap_pfn_range(pvma,               // Virtual Memory Area
            pvma->vm_start,                        // Start address of virtual mapping
            ((unsigned long) ptr) >> PAGE_SHIFT,   // Pointer in Pages (Page Frame Number)
            size,
            pvma->vm_page_prot);

         if ( unlikely(0 != res) ) {
            PERR("remap_pfn_range error at CSR mmap %d\n", res);
            goto ERROR;
         }

         // Successfully mapped MMR region.
         return 0;
      }

      if ( WSID_MAP_UMSG == wsidp->m_id )
      {
         if ( !cci_dev_allow_map_umsg_space(pdev) ) {
            PERR("Denying request to map cci_dev_allow_map_umsg_space Read space for device 0x%p.\n", pdev);
            goto ERROR;
         }

         ptr = (void *) cci_dev_phys_afu_umsg(pdev);
         size = cci_dev_len_afu_umsg(pdev);

         PVERBOSE("Mapping CSR %s Aperture Physical=0x%p size=%" PRIuSIZE_T " at uvp=0x%p\n",
            ((WSID_CSRMAP_WRITEAREA == wsidp->m_id) ? "write" : "read"),
            ptr,
            size,
            (void *)pvma->vm_start);

         // Map the region to user VM
         res = remap_pfn_range(pvma,                             // Virtual Memory Area
            pvma->vm_start,                   // Start address of virtual mapping
            ((unsigned long) ptr) >> PAGE_SHIFT, // Pointer in Pages (Page Frame Number)
            size,
            pvma->vm_page_prot);

         if ( unlikely(0 != res) ) {
            PERR("remap_pfn_range error at CSR mmap %d\n", res);
            goto ERROR;
         }

         // Successfully mapped UMSG region.
         return 0;
      }

      // TO REST OF CHECKS

      // Map the PCIe BAR as the CSR region.
      ptr = (void *) cci_dev_phys_cci_csr(pdev);
      size = cci_dev_len_cci_csr(pdev);

      PVERBOSE("Mapping CSR %s Aperture Physical=0x%p size=%" PRIuSIZE_T " at uvp=0x%p\n",
         ((WSID_CSRMAP_WRITEAREA == wsidp->m_id) ? "write" : "read"),
         ptr,
         size,
         (void *)pvma->vm_start);

      // Map the region to user VM
      res = remap_pfn_range(pvma,                             // Virtual Memory Area
         pvma->vm_start,                   // Start address of virtual mapping
         ((unsigned long) ptr) >> PAGE_SHIFT, // Pointer in Pages (Page Frame Number)
         size,
         pvma->vm_page_prot);

      if ( unlikely(0 != res) ) {
         PERR("remap_pfn_range error at CSR mmap %d\n", res);
         goto ERROR;
      }

      // Successfully mapped CSR region.
      return 0;
   }

   //------------------------
   // Map normal workspace
   //------------------------

   max_length = min(wsidp->m_size, (btWSSize)(pvma->vm_end - pvma->vm_start));

   PVERBOSE( "MMAP: start 0x%lx, end 0x%lx, KVP 0x%p, size=%" PRIu64 " 0x%" PRIx64 " max_length=%ld flags=0x%lx\n",
      pvma->vm_start, pvma->vm_end, (btVirtAddr)wsidp->m_id, wsidp->m_size, wsidp->m_size, max_length, pvma->vm_flags);

   res = remap_pfn_range(pvma,                              // Virtual Memory Area
      pvma->vm_start,                    // Start address of virtual mapping, from OS
      (kosal_virt_to_phys((btVirtAddr) wsidp->m_id) >> PAGE_SHIFT),   // physical memory backing store in pfn
      max_length,                        // size in bytes
      pvma->vm_page_prot);               // provided by OS
   if ( unlikely(0 != res) ) {
      PERR("remap_pfn_range error at workspace mmap %d\n", res);
      goto ERROR;
   }

   ERROR:
   return res;
}
#endif
