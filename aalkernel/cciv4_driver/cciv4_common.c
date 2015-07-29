// Copyright (c) 2015, Intel Corporation
//
// Redistribution  and  use  in source  and  binary  forms,  with  or  without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of  source code  must retain the  above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name  of Intel Corporation  nor the names of its contributors
//   may be used to  endorse or promote  products derived  from this  software
//   without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  BUT NOT LIMITED TO,  THE
// IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT  SHALL THE COPYRIGHT OWNER  OR CONTRIBUTORS BE
// LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR
// CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT LIMITED  TO,  PROCUREMENT  OF
// SUBSTITUTE GOODS OR SERVICES;  LOSS OF USE,  DATA, OR PROFITS;  OR BUSINESS
// INTERRUPTION)  HOWEVER CAUSED  AND ON ANY THEORY  OF LIABILITY,  WHETHER IN
// CONTRACT,  STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE  OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,  EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//****************************************************************************
//        FILE: cciv4_common.c
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

#include "cciv4_driver_internal.h"




//=============================================================================
// Name: cciv4_create_device
// Description: Constructor for a cciv4_device object
// Outputs: pointer to object.
// Comments: none.
//=============================================================================
 struct cciv4_device* cciv4_create_device()
{
   struct cciv4_device* pCCIv4dev = NULL;

   // Allocate the cciv4_device object
   pCCIv4dev = (struct cciv4_device*) kosal_kzmalloc(sizeof(struct cciv4_device));
   if ( NULL == pCCIv4dev ) {
      PERR("Unable to allocate system memory for cciv4_device object\n");
      return NULL;
   }

   // Initialize object
   kosal_list_init(&cciv4_dev_list_head(pCCIv4dev));
   kosal_mutex_init(cciv4_dev_psem(pCCIv4dev));

   return pCCIv4dev;
}

//=============================================================================
// Name: cciv4_destroy_device
// Description: Destructor for a cciv4_device object
// Inputs: pointer to object.
// Outputs: 0 - success
// Comments: none.
//=============================================================================
int cciv4_destroy_device( struct cciv4_device* pCCIv4dev)
{
   ASSERT(NULL != pCCIv4dev);
   if(NULL == pCCIv4dev){
      PERR("Attemptiong to destroy NULL pointer to cciv4_device object\n");
      return -EINVAL;
   }

   kosal_kfree(pCCIv4dev, sizeof(struct cciv4_device));
   return 0;
}

//=============================================================================
// Name: cciv4_publish_aaldevice
// Description: Publishes an AAL Device with the Configuration Management
//              subsystem.
// Inputs: pCCIv4dev - Device Object .
//         pdevid - Device ID.
//         pPIP - PIP for this device.
//         maxshares - Concurrent ownership
// Outputs: 0 - success.
// Comments: none.
//=============================================================================
int cciv4_publish_aaldevice(struct cciv4_device * pCCIv4dev,
                            struct aal_device_id* pdevid,
                            struct aal_ipip* pPIP,
                            btUnsignedInt maxshares)
{
   // Used to send the create request to aalbus::afu_factory
   struct mafu_CreateAFU request;
   struct aal_device   *pdev   = NULL;

   // Pointer to the AAL Bus interface
   struct aal_bus       *pAALbus = aalbus_get_bus();

   ASSERT(pCCIv4dev);
   ASSERT(pdevid);
   ASSERT(pAALbus);
   ASSERT(pPIP);
   ASSERT(maxshares >0 );

   if(0 == maxshares ){
      PERR("Invalid maxshares of zero!\n");
      return -EINVAL;
   }
   // Prepare the request
   memset(&request, 0, sizeof(struct mafu_CreateAFU));

   // Set the ID for the AAL Device to publish
   request.device_id = *pdevid;

   // Name that will appear in sysfs
   strncpy(request.basename, DEVICE_BASENAME, sizeof(request.basename));

   // Concurrent ownership
   request.maxshares = maxshares;

   // Config space mapping
   request.enableDirectAPI = AAL_DEV_APIMAP_NONE;
   if( cciv4_dev_allow_map_csr_read_space(pCCIv4dev) ){
      request.enableDirectAPI |= AAL_DEV_APIMAP_CSRWRITE;
   }

   if( cciv4_dev_allow_map_csr_write_space(pCCIv4dev) ){
      request.enableDirectAPI |= AAL_DEV_APIMAP_CSRREAD;
   }

   // Create the AAL Device.
   //   Notes:
   //   When creating an AAL Device a Physical Interface Protocol (PIP) is
   //   associated with the object. The PIP represents the command interface
   //   to the object. The PIP is often implemented as an AAL kernel service
   //   so that it may be shared by different device implementations. The ID
   //   of the PIP to associate with the AFU is specified in the aal_device_id
   //   (see above).  There are 2 ways that the PIP implementation can be associated
   //   (ie bound) to a new AFU.  If the PIP is implemented in a module (KSM)
   //   that is different from the module creating the AFU, then AAL kernel services
   //   will bind the PIP to the AFU in the kernel framework.  This will cause the
   //   module implementing the PIP to have its reference count incremented so that
   //   it cannot be removed while the PIP is in use.
   //
   //   If the PIP is implemented in the same module in which the AAL Device is being created,
   //   as in this example, then the PIP pointer should be directly provided
   //   to the AAL device factory at construction time. Not providing it at
   //   construction time will cause "THIS" module's reference count to increment,
   //   which may prevent the module from being removed unless special provisions
   //   have been put in place by the module designer.

   //   First arg is the request describing the AAL Device to create, 2nd is a pointer
   //   to call when the AFU is released (removed from the system) and the 3rd
   //   is a pointer to the device's PIP (see above explanation).
   pdev = aaldev_factp(pAALbus).create(&request, cciv4_release_device, pPIP);
   ASSERT(pdev);
   if ( NULL == pdev ) {
      return -ENODEV;
   }
   // Save the AAL Device
   cciv4_dev_to_aaldev(pCCIv4dev) = pdev;

   // The PIP uses the PIP context to get a handle to the AAL Device.
   //  The generic (i.e., base class) pointer is passed around in messages to the PIP
   //  typically through the owner session object.  .
   aaldev_pip_context(pdev) = (void*)pdev;

   PVERBOSE("Publishing CCIv4 device\n");

   return 0;
} //cciv4_publish_aaldevice

//=============================================================================
// Name: cciv4_unpublish_aaldevice
// Description: Removes AAL Device from Configuration
// Input: pCCIv4dev - device to remove
// Comment:
// Returns: none
// Comments:
//=============================================================================
void
cciv4_unpublish_aaldevice(struct cciv4_device *pCCIv4dev)
{
   PVERBOSE("Removing CCIv4 device from configuration\n");

   if ( cciv4_dev_to_aaldev(pCCIv4dev) ) {
      PINFO("Removing AAL device\n");
      aalbus_get_bus()->unregister_device(cciv4_dev_to_aaldev(pCCIv4dev));
      cciv4_dev_to_aaldev(pCCIv4dev) = NULL;
   }

} // cciv4_unpublish_aaldevice

//=============================================================================
// Name: cciv4_remove_device
// Description: Performs generic cleanup and deletion of CCIv4 object
// Input: pCCIv4dev - device to remove
// Comment:
// Returns: none
// Comments:
//=============================================================================
void
cciv4_remove_device(struct cciv4_device *pCCIv4dev)
{
   PVERBOSE("Removing CCIv4 device\n");

   // Call PIP to ensure the object is idle and ready for removal
   // TODO

   // Make it unavailable
   cciv4_unpublish_aaldevice(pCCIv4dev);

   // Remove ourselves from any lists
   kosal_sem_get_krnl( &pCCIv4dev->m_sem );
   kosal_list_del(&cciv4_dev_list_head(pCCIv4dev));
   kosal_sem_put( &pCCIv4dev->m_sem );


   if ( cciv4_dev_pci_dev_is_region_requested(pCCIv4dev) ) {
      pci_release_regions(cciv4_dev_pci_dev(pCCIv4dev));
      cciv4_dev_pci_dev_clr_region_requested(pCCIv4dev);
   }else{
      if(NULL != cciv4_dev_kvp_config(pCCIv4dev)){
         kosal_kfree(cciv4_dev_kvp_config(pCCIv4dev), cciv4_dev_len_config(pCCIv4dev));
      }
   }

   if ( cciv4_dev_pci_dev_is_enabled(pCCIv4dev) ) {
      pci_disable_device(cciv4_dev_pci_dev(pCCIv4dev));
      cciv4_dev_pci_dev_clr_enabled(pCCIv4dev);
   }

   kosal_kfree(cciv4_dev_kvp_afu_mmio(pCCIv4dev), cciv4_dev_len_afu_mmio(pCCIv4dev));
   kosal_kfree(cciv4_dev_kvp_afu_umsg(pCCIv4dev),cciv4_dev_len_afu_umsg(pCCIv4dev));



   cciv4_destroy_device(pCCIv4dev);

} // cciv4_remove_device
