//******************************************************************************
// Part of the Intel(R) QuickAssist Technology Accelerator Abstraction Layer
//
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
//        FILE: cci_common.c
//     CREATED: Jul 28, 2015
//      AUTHOR: Joseph Grecco <joe.grecco@intel.com>
//
// PURPOSE:   This file holds OS independent AAL specific implementation
//            common to this driver.
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
//****************************************************************************///
#include "aalsdk/kernel/kosal.h"
#define MODULE_FLAGS CCIPCIE_DBG_MOD // Prints all

#include "aalsdk/kernel/aalbus.h"
#include "aalsdk/kernel/ccipdriver.h"
#include "aalsdk/kernel/iaaldevice.h"
#include "aalsdk/kernel/ccip_defs.h"

#include "cci_pcie_driver_internal.h"


#include "ccip_port.h"
#include "ccip_fme.h"

extern struct cci_aal_device   *
               cci_create_AAL_SignalTap_Device( struct port_device  *,
                                                struct aal_device_id *);
extern struct cci_aal_device   *
                      cci_create_AAL_PR_Device( struct port_device  *,
                                                struct aal_device_id *);

extern struct cci_aal_device   *
                       cci_create_AAL_UAFU_Device( struct port_device  *,
                                                   btPhysAddr,
                                                   struct CCIP_AFU_Header *,
                                                   struct aal_device_id *);

struct cci_aal_device   *
               cci_create_AAL_Port_Device( struct port_device  *,
                                           btUnsigned32bitInt devnum,
                                           struct aal_device_id *);

///============================================================================
/// Name: create_ccidevice
/// @brief Creates the CCI Board device object
/// @return ccip_device
///============================================================================
struct ccip_device * create_ccidevice()
{
   // Allocate the board object
   struct ccip_device * pccipdev = (struct ccip_device * ) kosal_kzmalloc(sizeof(struct ccip_device));
   if ( NULL ==  pccipdev ) {
      PERR("Could not allocate CCI device object\n");
      return NULL;
   }

   // Initialize object
   kosal_list_init(&cci_dev_list_head(pccipdev));
   kosal_list_init(&cci_dev_list_head(pccipdev));
   kosal_list_init(&ccip_aal_dev_list(pccipdev));
   kosal_list_init(&ccip_port_dev_list(pccipdev));

   kosal_mutex_init(cci_dev_psem(pccipdev));

   return pccipdev;
}

///============================================================================
/// Name: destroy_ccidevice
/// @brief Destroys the CCI Board device object
/// @return ccip_device
///============================================================================
void  destroy_ccidevice(struct ccip_device *pccidev)
{
   struct list_head *paaldev_list = &ccip_aal_dev_list(pccidev);
   struct list_head *port_list = &ccip_port_dev_list(pccidev);

   PVERBOSE("Destroying CCI devices\n");

   // Check the aaldevice list for any registered objects
   if( !kosal_list_is_empty( paaldev_list ) ){
      struct cci_aal_device *pcci_aaldev   = NULL;
      kosal_list_head       *This          = NULL;
      kosal_list_head       *tmp           = NULL;

      // Run through list of devices.  Use safe variant
      //  as we will be deleting entries
      list_for_each_safe(This, tmp, paaldev_list) {

         pcci_aaldev = cci_list_to_cci_aal_device(This);

         cci_unpublish_aaldevice(pcci_aaldev);
         cci_destroy_aal_device(pcci_aaldev);
      }

   }else{
      PVERBOSE("No published objects to remove\n");
   }

   // If Lists not empty do it TODO
   // Check the aaldevice list for any registered objects
   if( !kosal_list_is_empty( port_list ) ){
      struct port_device    *pportdev     = NULL;
      kosal_list_head       *This         = NULL;
      kosal_list_head       *tmp          = NULL;

      // Run through list of devices.  Use safe variant
      //  as we will be deleting entries
      list_for_each_safe(This, tmp, port_list) {

         pportdev = cci_list_to_cci_port_device(This);
         destroy_port_device(pportdev);
      }

   }else{
      PVERBOSE("No port objects to remove\n");
   }

   // Destroy the FME device
   if(NULL != ccip_dev_to_fme_dev(pccidev)) {
      PVERBOSE("Freeing FME Memory\n");
      kosal_kfree(ccip_dev_to_fme_dev(pccidev),sizeof(struct fme_device ) );
   }

   // Remove ourselves from any lists
   kosal_sem_get_krnl( &pccidev->m_sem );
   kosal_list_del(&cci_dev_list_head(pccidev));
   kosal_sem_put( &pccidev->m_sem );

   kosal_kfree(pccidev, sizeof(struct ccip_device));
}

///============================================================================
/// Name: cci_fme_dev_create_AAL_allocatable_objects
/// @brief Creates and registers FME objects (resources) we want to expose
///        Through AAL.
///
/// @param[in] pccipdev - CCI Board object .
/// @return    error code
///============================================================================
btBool cci_fme_dev_create_AAL_allocatable_objects(struct ccip_device * pccipdev)
{
   struct cci_aal_device   *pcci_aaldev = NULL;
   struct aal_device_id     aalid;
   int                      ret        = 0;

   //=====================================================================
   // Create the FME AAL device. The FME AAL device is the class
   //   is registered with AAL Bus to enable it to be allocated by an
   //   application. AAL device objects represent the application usable
   //   devices. AAL device objects have their own low level communication
   //   function called the Physical Interface Protocol (PIP). This enables
   //   us to easily create object specific interfaces in a single driver.

   // Construct the cci_aal_device object
   pcci_aaldev = cci_create_aal_device();

   ASSERT(NULL != pcci_aaldev);

   // Make it an FME by setting the type field and giving a pointer to the
   //  FME device object of the CCIP board device
   cci_dev_type(pcci_aaldev) = cci_dev_FME;
   cci_dev_pfme(pcci_aaldev) = ccip_dev_to_fme_dev(pccipdev);


   // Setup the AAL device's ID. This is the collection of attributes
   //  that uniquely identifies the AAL device, usually for the purpose
   //  of allocation through Resource Management
   //------------------------------------------------------------------
   aaldevid_devaddr_bustype(aalid)     =  ccip_dev_pcie_bustype(pccipdev);

   // The AAL address maps to the PCIe address. The Subdevice number is
   //  vendor defined and in this case the FME object has the value CCIP_DEV_FME_SUBDEV
   //
   // Device addressing follows the convention of B:D:F is the PCIe address.
   //  The B:D:F uniquely identifies a physical board which is the root of the
   //  device hierarchy. All AAL devices under this board will have the same B:D:F
   //  The subdevice number is used to identify the Port number in the hierarchy. For
   //  the FME object it is 0xFFFF.
   //  Because every AAL device exposed through the AAL kernel framework must have a unique
   //  address, the instance number is used to uniquely identify Objects under the subdevice.
   //  The instance number simply increments within an adress space and should not be assumed
   //  to relate to a particular object type.
   //  Example:  For B:D:F  		00010001:0:0
   //       FME address: 			00010001:0:0:FFFF:0
   //       Port 0 address:			00010001:0:0:0:0
   //       PR Port 0 address:  	00010001:0:0:0:1   (the instance number is not have a define value other than to create a unique address)
   //       UAFU Port 0 address:	00010001:0:0:0:2
   //       Port 1 address:			00010001:0:0:1:0
   //       PR Port 1 address:  	00010001:0:0:1:1   (the instance number is not have a define value other than to create a unique address)
   //       UAFU Port 1 address:	00010001:0:0:1:2

   aaldevid_devaddr_busnum(aalid)      = ccip_dev_pcie_busnum(pccipdev);
   aaldevid_devaddr_devnum(aalid)      = ccip_dev_pcie_devnum(pccipdev);
   aaldevid_devaddr_fcnnum(aalid)      = ccip_dev_pcie_fcnnum(pccipdev);
   aaldevid_devaddr_subdevnum(aalid)   = CCIP_DEV_FME_SUBDEV;	// FME subdevice number is constant
   aaldevid_devaddr_instanceNum(aalid) = 0;						// FME is always instance 0

   // The following attributes describe the interfaces supported by the device
   aaldevid_afuguidl(aalid)            = CCIP_FME_GUIDL;
   aaldevid_afuguidh(aalid)            = CCIP_FME_GUIDH;
   aaldevid_devtype(aalid)             = aal_devtypeAFU;
   aaldevid_pipguid(aalid)             = CCIP_FME_PIPIID;
   aaldevid_vendorid(aalid)            = AAL_vendINTC;

   // Set the interface permissions
   // Enable MMIO-R
   cci_dev_set_allow_map_mmior_space(pcci_aaldev);


   // Setup the MMIO region parameters
   cci_dev_kvp_afu_mmio(pcci_aaldev)   = ccip_fmedev_kvp_afu_mmio(pccipdev);
   cci_dev_len_afu_mmio(pcci_aaldev)   = ccip_fmedev_len_afu_mmio(pccipdev);
   cci_dev_phys_afu_mmio(pcci_aaldev)  = ccip_fmedev_phys_afu_mmio(pccipdev);

   // Create the AAL device and attach it to the CCI device object
   cci_aaldev_to_aaldev(pcci_aaldev) =  aaldev_create( "CCIPFME",           // AAL device base name
                                                       &aalid,             // AAL ID
                                                       &cci_FMEpip);

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
      PERR("Failed to initialize AAL Device for FME[%d:%d:%d:%x:%d]",aaldevid_devaddr_busnum(aalid),
                                                                     aaldevid_devaddr_devnum(aalid),
                                                                     aaldevid_devaddr_fcnnum(aalid),
                                                                     aaldevid_devaddr_subdevnum(aalid),
																	 aaldevid_devaddr_instanceNum(aalid));
      kosal_kfree(cci_dev_kvp_afu_mmio(pcci_aaldev), cci_dev_len_afu_mmio(pcci_aaldev));
      kosal_kfree(cci_dev_kvp_afu_umsg(pcci_aaldev),cci_dev_len_afu_umsg(pcci_aaldev));
      cci_destroy_aal_device(pcci_aaldev);
      return -EINVAL;
   }

   // Add the device to the CCI Board device's device list
   kosal_list_add(&cci_dev_list_head(pcci_aaldev), &ccip_aal_dev_list(pccipdev));

   return true;
}

///============================================================================
/// Name: cci_port_dev_create_AAL_allocatable_objects
/// @brief Creates and registers Port objects (resources) we want to expose
///        through AAL.
///
/// @param[in] pportdev - CCI Port object
/// @param[in] devnum - Port number
/// @return    error code
///============================================================================
btBool cci_port_dev_create_AAL_allocatable_objects(struct port_device  *pportdev,
                                                   btUnsigned32bitInt devnum)
{
   struct cci_aal_device   *pcci_aaldev = NULL;
   struct aal_device_id     aalid;


   //==================================================
   // Instantiate a Port device
   //  Creates the Port AAL devuce. Initializes
   //  the aalid used for all of the child devices to
   //  be created later.
   pcci_aaldev = cci_create_AAL_Port_Device(pportdev, devnum, &aalid);
   ASSERT(NULL != pcci_aaldev);

   if(NULL == pcci_aaldev){
      PDEBUG("ERROR: Creating port device\n");
      return false;     // TODO This is a BUG if we get here but should cleanup correctly.
   }

   // Add the device to the CCI Board device's device list
   kosal_list_add( &cci_dev_list_head(pcci_aaldev), &ccip_aal_dev_list( ccip_port_to_ccidev(pportdev) ));

   //=======================================
   // Instantiate a Signal Tap device
   aaldevid_devaddr_instanceNum(aalid)++;
   pcci_aaldev = cci_create_AAL_SignalTap_Device(pportdev, &aalid);
   ASSERT(NULL != pcci_aaldev);

   if(NULL == pcci_aaldev){
      PDEBUG("ERROR: Creating Signal Tap device\n");
      return false;     // TODO This is a BUG if we get here but should cleanup correctly.
   }

   // Add the device to the CCI Board device's device list
   kosal_list_add( &cci_dev_list_head(pcci_aaldev), &ccip_aal_dev_list( ccip_port_to_ccidev(pportdev) ));

   //========================
   // Instantiate a PR Device
   aaldevid_devaddr_instanceNum(aalid)++;
   pcci_aaldev = cci_create_AAL_PR_Device(pportdev, &aalid);
   ASSERT(NULL != pcci_aaldev);

   if(NULL == pcci_aaldev){
      PDEBUG("ERROR: Creating PR device\n");
      return false;     // TODO This is a BUG if we get here but should cleanup correctly.
   }

   // Add the device to the CCI Board device's device list
   kosal_list_add( &cci_dev_list_head(pcci_aaldev), &ccip_aal_dev_list( ccip_port_to_ccidev(pportdev) ));

   //=========================================
   // Instantiate a User AFU if one is present
   {
      // Get the AFU header pointer by adding the offset to the port header address
      struct CCIP_AFU_Header        *pafu_hdr = (struct CCIP_AFU_Header *)(((btVirtAddr)ccip_port_hdr(pportdev) ) + ccip_port_hdr(pportdev)->ccip_port_next_afu.afu_id_offset);
      btPhysAddr                     pafu_phys = ccip_port_phys_mmio(pportdev) + ccip_port_hdr(pportdev)->ccip_port_next_afu.afu_id_offset;

      // If the device is present
      if(~0ULL != pafu_hdr->ccip_dfh.csr){

         // Instantiate it
         aaldevid_devaddr_instanceNum(aalid)++;
         pcci_aaldev = cci_create_AAL_UAFU_Device(  pportdev,
                                                    pafu_phys,
                                                    pafu_hdr,
                                                   &aalid);
         ASSERT(NULL != pcci_aaldev);

         if(NULL == pcci_aaldev){
            PDEBUG("ERROR: Creating User AFU device\n");
            return false;     // TODO This is a BUG if we get here but should cleanup correctly.
         }

         // Add the device to the CCI Board device's device list
         kosal_list_add( &cci_dev_list_head(pcci_aaldev), &ccip_aal_dev_list( ccip_port_to_ccidev(pportdev) ));

      } // End if(~0ULL == pafu_hdr->ccip_dfh.csr){
   } //End block

   return true;
}

///============================================================================
/// Name: cci_create_aal_device
/// @brief Constructor for a cci_aal_device object.
/// @return    pointer to object
///============================================================================
 struct cci_aal_device* cci_create_aal_device()
{
   struct cci_aal_device* pcci_aaldev = NULL;

   // Allocate the cci_aal_device object
   pcci_aaldev = (struct cci_aal_device*) kosal_kzmalloc(sizeof(struct cci_aal_device));
   if ( NULL == pcci_aaldev ) {
      PERR("Unable to allocate system memory for cci_aal_device object\n");
      return NULL;
   }

   // Initialize object
   kosal_list_init(&cci_dev_list_head(pcci_aaldev));
   kosal_mutex_init(cci_dev_psem(pcci_aaldev));

   return pcci_aaldev;
}

//=============================================================================
// Name: cci_destroy_aal_device
// Description: Destructor for a cci_aal_device object
// Inputs: pointer to object.
// Outputs: 0 - success
// Comments: none.
//=============================================================================
int cci_destroy_aal_device( struct cci_aal_device* pcci_aaldev)
{
   ASSERT(NULL != pcci_aaldev);
   if(NULL == pcci_aaldev){
      PERR("Attempting to destroy NULL pointer to cci_aal_device object\n");
      return -EINVAL;
   }

   kosal_list_del( &cci_dev_list_head(pcci_aaldev));

   kosal_kfree(pcci_aaldev, sizeof(struct cci_aal_device));
   return 0;
}

//=============================================================================
// Name: cci_release_device
// Description: callback for notification that an AAL Device is being destroyed.
// Interface: public
// Inputs: pdev: kernel-provided generic device structure.
// Outputs: none.
// Comments:
//=============================================================================
void
cci_release_device(pkosal_os_dev pdev)
{

   struct aal_device *paaldev = basedev_to_aaldev(pdev);
//   struct cci_aal_device *pcci_aaldev = aaldev_to_cci_aal_device(paaldev);

   PTRACEIN;

   PDEBUG("Called with struct aal_device * 0x%p\n", paaldev);
//   kosal_list_del( &cci_dev_list_head(pcci_aaldev));

   // DO NOT call factory release here. It will be done by the framework.
   PTRACEOUT;
}

//=============================================================================
// Name: cci_publish_aaldevice
// Description: Publishes an AAL Device with the Configuration Management
//              subsystem.
// Inputs: pCCIdev - Device Object .
// Outputs: 0 - success.
// Comments: none.
//=============================================================================
int cci_publish_aaldevice(struct cci_aal_device * pcci_aaldev)
{
   btInt ret = 0;

   // Pointer to the AAL Bus interface
   struct aal_bus       *pAALbus = aalbus_get_bus();

   ASSERT(pcci_aaldev);

   ret = pAALbus->register_device(cci_aaldev_to_aaldev(pcci_aaldev));
   if ( 0 != ret ) {
      return -ENODEV;
   }
   PVERBOSE("Published CCI device\n");

   return 0;
} //cci_publish_aaldevice

//=============================================================================
// Name: cci_unpublish_aaldevice
// Description: Removes AAL Device from Configuration
// Input: pCCIdev - device to remove
// Comment:
// Returns: none
// Comments:
//=============================================================================
void
cci_unpublish_aaldevice(struct cci_aal_device *pcci_aaldev)
{
   PVERBOSE("Removing CCI device from configuration\n");

   if ( NULL!= cci_aaldev_to_aaldev(pcci_aaldev) ) {
      PINFO("Removing AAL device\n");
      aalbus_get_bus()->unregister_device(cci_aaldev_to_aaldev(pcci_aaldev));
      cci_aaldev_to_aaldev(pcci_aaldev) = NULL;
   }

} // cci_unpublish_aaldevice



