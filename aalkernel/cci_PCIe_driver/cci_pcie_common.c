//******************************************************************************
// Part of the Intel(R) QuickAssist Technology Accelerator Abstraction Layer
//
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2011-2015, Intel Corporation.
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
//        FILE: cci_common.c
//     CREATED: Jul 28, 2015
//      AUTHOR: Joseph Grecco <joe.grecco@intel.com>
//
// PURPOSE:   This file holds OS independent implementation common to this
//            driver
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
//****************************************************************************///
#include "aalsdk/kernel/kosal.h"
#define MODULE_FLAGS CCIV4_DBG_MOD // Prints all

#include "aalsdk/kernel/aalbus.h"
#include "aalsdk/kernel/ccipdriver.h"
#include "aalsdk/kernel/iaaldevice.h"

#include "cci_pcie_driver_internal.h"

#include "ccip_defs.h"


extern struct aal_ipip cci_FMEpip;

///============================================================================
/// cci_dev_create_allocatable_objects
/// @brief Construct the FME MMIO object.
///
/// @param[in] fme_device fme device object .
/// @param[in] pkvp_fme_mmio fme mmio virtual address
/// @return    error code
///============================================================================
btBool cci_dev_create_allocatable_objects(struct ccip_device * pccipdev)
{
   struct cci_aal_device   *pCCIdev    = NULL;
   struct aal_device_id     aalid;
   int                      ret        = 0;


   // First create FME objects

   //=============================================================
   // Create the CCI device structure. The CCI device is the class
   // used by the Low Level Communications (PIP). It holds the
   // hardware specific attributes.

   // Construct the cci_aal_device object
   pCCIdev = cci_create_device();

   ASSERT(NULL != pCCIdev);

   // Make it an FME by setting the type field and giving a pointer to the
   //  FME device object of the CCIP board device
   cci_dev_type(pCCIdev) = cci_dev_FME;
   set_cci_dev_subclass(pCCIdev, ccip_dev_to_fme_dev(pccipdev));

   // Setup the AAL device's ID. This is the collection of attributes
   //  that uniquely identifies the AAL device, usually for the purpose
   //  of allocation through Resource Management
   aaldevid_devaddr_bustype(aalid)     = aal_bustype_PCIe;

   // The AAL address maps to the PCIe address. The Subdevice number is
   //  vendor defined and in this case the FME object has the value CCIP_DEV_FME_SUBDEV
   aaldevid_devaddr_busnum(aalid)      = ccip_dev_pcie_busnum(pccipdev);
   aaldevid_devaddr_devnum(aalid)      = ccip_dev_pcie_devnum(pccipdev);
   aaldevid_devaddr_fcnnum(aalid)      = ccip_dev_pcie_fcnnum(pccipdev);
   aaldevid_devaddr_subdevnum(aalid)   = CCIP_DEV_FME_SUBDEV;

   // The following attributes describe the interfaces supported by the device
   aaldevid_afuguidl(aalid)            = CCIP_FME_GUIDL;
   aaldevid_afuguidh(aalid)            = CCIP_FME_GUIDH;
   aaldevid_devtype(aalid)             = aal_devtypeAFU;
   aaldevid_pipguid(aalid)             = CCIP_FME_PIPIID;
   aaldevid_vendorid(aalid)            = AAL_vendINTC;

   // Set the interface permissions
   // Enable MMIO-R
   cci_dev_set_allow_map_mmior_space(pCCIdev);


   // Create the AAL device and attach it to the CCI device object
   pCCIdev->m_aaldev =  aaldev_create( "CCIPFME",           // AAL device base name
                                        &aalid,             // AAL ID
                                        &cci_FMEpip);

   //===========================================================
   // Set up the optional aal_device attributes
   //

   // Set how many owners are allowed access to this device simultaneously
   pCCIdev->m_aaldev->m_maxowners = 1;

   // Set the config space mapping permissions
   cci_dev_to_aaldev(pCCIdev)->m_mappableAPI = AAL_DEV_APIMAP_NONE;
   if( cci_dev_allow_map_csr_read_space(pCCIdev) ){
      cci_dev_to_aaldev(pCCIdev)->m_mappableAPI |= AAL_DEV_APIMAP_CSRWRITE;
   }

   if( cci_dev_allow_map_csr_write_space(pCCIdev) ){
      cci_dev_to_aaldev(pCCIdev)->m_mappableAPI |= AAL_DEV_APIMAP_CSRREAD;
   }

   if( cci_dev_allow_map_mmior_space(pCCIdev) ){
      cci_dev_to_aaldev(pCCIdev)->m_mappableAPI |= AAL_DEV_APIMAP_MMIOR;
   }

   if( cci_dev_allow_map_umsg_space(pCCIdev) ){
      cci_dev_to_aaldev(pCCIdev)->m_mappableAPI |= AAL_DEV_APIMAP_UMSG;
   }

   // The PIP uses the PIP context to get a handle to the CCI Device from the generic device.
   aaldev_pip_context(cci_dev_to_aaldev(pCCIdev)) = (void*)pCCIdev;

   // Method called when the device is released (i.e., its destructor)
   //  The canonical release device calls the user's release method.
   //  If NULL is provided then only the canonical behavior is done
   dev_setrelease(cci_dev_to_aaldev(pCCIdev), cci_release_device);

      // Device is ready for use.  Publish it with the Configuration Management Subsystem
   ret = cci_publish_aaldevice(pCCIdev);
   ASSERT(ret == 0);
   if(0> ret){
      PERR("Failed to initialize AAL Device for FME[%d:%d:%d:%d]",aaldevid_devaddr_busnum(aalid),
                                                                  aaldevid_devaddr_devnum(aalid),
                                                                  aaldevid_devaddr_fcnnum(aalid),
                                                                  aaldevid_devaddr_subdevnum(aalid));
      kosal_kfree(cci_dev_kvp_afu_mmio(pCCIdev), cci_dev_len_afu_mmio(pCCIdev));
      kosal_kfree(cci_dev_kvp_afu_umsg(pCCIdev),cci_dev_len_afu_umsg(pCCIdev));
      cci_destroy_device(pCCIdev);
      return -EINVAL;
   }

   // Add the device to theCCI Board device's device list
   kosal_list_add(&ccip_aal_dev_list(pccipdev), &cci_dev_list_head(pCCIdev));

   return true;
}

//=============================================================================
// Name: cci_create_device
// Description: Constructor for a cci_aal_device object
// Outputs: pointer to object.
// Comments: none.
//=============================================================================
 struct cci_aal_device* cci_create_device()
{
   struct cci_aal_device* pCCIdev = NULL;

   // Allocate the cci_aal_device object
   pCCIdev = (struct cci_aal_device*) kosal_kzmalloc(sizeof(struct cci_aal_device));
   if ( NULL == pCCIdev ) {
      PERR("Unable to allocate system memory for cci_aal_device object\n");
      return NULL;
   }

   // Initialize object
   kosal_list_init(&cci_dev_list_head(pCCIdev));
   kosal_mutex_init(cci_dev_psem(pCCIdev));

   return pCCIdev;
}

//=============================================================================
// Name: cci_destroy_device
// Description: Destructor for a cci_aal_device object
// Inputs: pointer to object.
// Outputs: 0 - success
// Comments: none.
//=============================================================================
int cci_destroy_device( struct cci_aal_device* pCCIdev)
{
   ASSERT(NULL != pCCIdev);
   if(NULL == pCCIdev){
      PERR("Attemptiong to destroy NULL pointer to cci_aal_device object\n");
      return -EINVAL;
   }

   kosal_kfree(pCCIdev, sizeof(struct cci_aal_device));
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
cci_release_device(struct device *pdev)
{
#if ENABLE_DEBUG
   struct aal_device *paaldev = basedev_to_aaldev(pdev);
#endif // ENABLE_DEBUG

   PTRACEIN;

   PDEBUG("Called with struct aal_device * 0x%p\n", paaldev);

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
int cci_publish_aaldevice(struct cci_aal_device * pCCIdev)
{
   btInt ret = 0;

   // Pointer to the AAL Bus interface
   struct aal_bus       *pAALbus = aalbus_get_bus();

   ASSERT(pCCIdev);

   ret = pAALbus->register_device(cci_dev_to_aaldev(pCCIdev));
   if ( 0 != ret ) {
      return -ENODEV;
   }
   PVERBOSE("Published CCIv4 device\n");

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
cci_unpublish_aaldevice(struct cci_aal_device *pCCIdev)
{
   PVERBOSE("Removing CCIv4 device from configuration\n");

   if ( cci_dev_to_aaldev(pCCIdev) ) {
      PINFO("Removing AAL device\n");
      aalbus_get_bus()->unregister_device(cci_dev_to_aaldev(pCCIdev));
      cci_dev_to_aaldev(pCCIdev) = NULL;
   }

} // cci_unpublish_aaldevice

//=============================================================================
// Name: cci_remove_device
// Description: Performs generic cleanup and deletion of CCIv4 object
// Input: pCCIdev - device to remove
// Comment:
// Returns: none
// Comments:
//=============================================================================
void
cci_remove_device(struct cci_aal_device *pCCIdev)
{
   PDEBUG("Removing CCI device\n");

   // Call PIP to ensure the object is idle and ready for removal
   // TODO

   // Remove ourselves from any lists
   kosal_sem_get_krnl( &pCCIdev->m_sem );
   kosal_list_del(&cci_dev_list_head(pCCIdev));
   kosal_sem_put( &pCCIdev->m_sem );


   if ( cci_dev_pci_dev_is_region_requested(pCCIdev) ) {
      pci_release_regions(cci_dev_pci_dev(pCCIdev));
      cci_dev_pci_dev_clr_region_requested(pCCIdev);
   }

   if ( cci_dev_pci_dev_is_enabled(pCCIdev) ) {
      pci_disable_device(cci_dev_pci_dev(pCCIdev));
      cci_dev_pci_dev_clr_enabled(pCCIdev);
   }

   // If simulated free the BAR
   if( cci_is_simulated(pCCIdev) ){
      if(NULL != cci_dev_kvp_config(pCCIdev)){
         kosal_kfree(cci_dev_kvp_config(pCCIdev), cci_dev_len_config(pCCIdev));
      }
      if(NULL != cci_dev_kvp_afu_mmio(pCCIdev)){
         kosal_kfree(cci_dev_kvp_afu_mmio(pCCIdev), cci_dev_len_afu_mmio(pCCIdev));
      }
      if(NULL != cci_dev_kvp_afu_umsg(pCCIdev)){
         kosal_kfree(cci_dev_kvp_afu_umsg(pCCIdev),cci_dev_len_afu_umsg(pCCIdev));
       }
   }

   // Make it unavailable
   cci_unpublish_aaldevice(pCCIdev);
   cci_destroy_device(pCCIdev);

} // cci_remove_device


