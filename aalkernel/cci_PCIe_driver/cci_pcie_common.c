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
#include "ccip_port.h"
#include "ccip_fme.h"

///============================================================================
/// create_ccidevice
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
/// destroy_ccidevice
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

   // Remove ourselves from any lists
   kosal_sem_get_krnl( &pccidev->m_sem );
   kosal_list_del(&cci_dev_list_head(pccidev));
   kosal_sem_put( &pccidev->m_sem );

   kosal_kfree(pccidev, sizeof(struct ccip_device));
}

///============================================================================
/// cci_fme_dev_create_AAL_allocatable_objects
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


   // First create FME objects

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
   set_cci_dev_subclass(pcci_aaldev, ccip_dev_to_fme_dev(pccipdev));

   // Setup the AAL device's ID. This is the collection of attributes
   //  that uniquely identifies the AAL device, usually for the purpose
   //  of allocation through Resource Management
   //------------------------------------------------------------------
   aaldevid_devaddr_bustype(aalid)     =  ccip_dev_pcie_bustype(pccipdev);

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
   cci_dev_set_allow_map_mmior_space(pcci_aaldev);


   // Create the AAL device and attach it to the CCI device object
   pcci_aaldev->m_aaldev =  aaldev_create( "CCIPFME",           // AAL device base name
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
      PERR("Failed to initialize AAL Device for FME[%d:%d:%d:%d]",aaldevid_devaddr_busnum(aalid),
                                                                  aaldevid_devaddr_devnum(aalid),
                                                                  aaldevid_devaddr_fcnnum(aalid),
                                                                  aaldevid_devaddr_subdevnum(aalid));
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
/// cci_port_dev_create_AAL_allocatable_objects
/// @brief Creates and registers Port objects (resources) we want to expose
///        through AAL.
///
/// @param[in] pportdev - CCI Port object .
/// @return    error code
///============================================================================
btBool cci_port_dev_create_AAL_allocatable_objects(struct port_device  *pportdev,
                                                   btUnsigned32bitInt devnum)
{
   struct cci_aal_device   *pcci_aaldev = NULL;
   struct aal_device_id     aalid;
   int                      ret        = 0;


   //=====================================================================
   // Create the Port AAL device. The Port AAL device is the class
   //   is registered with AAL Bus to enable it to be allocated by an
   //   application. AAL device objects represent the application usable
   //   devices. AAL device objects have their own low level communication
   //   function called the Physical Interface Protocol (PIP). This enables
   //   us to easily create object specific interfaces in a single driver.

   // Construct the cci_aal_device object
   pcci_aaldev = cci_create_aal_device();

   ASSERT(NULL != pcci_aaldev);

   // Make it an Port by setting the type field and giving a pointer to the
   //  Port device object of the CCIP board device
   cci_dev_type(pcci_aaldev) = cci_dev_Port;
   set_cci_dev_subclass(pcci_aaldev, pportdev);

   // Setup the AAL device's ID. This is the collection of attributes
   //  that uniquely identifies the AAL device, usually for the purpose
   //  of allocation through Resource Management
   //------------------------------------------------------------------
   aaldevid_devaddr_bustype(aalid)     =   ccip_port_bustype(pportdev);

   // The AAL address maps to the PCIe address. The Subdevice number is
   //  vendor defined and in this case the FME object has the value CCIP_DEV_FME_SUBDEV
   aaldevid_devaddr_busnum(aalid)      = ccip_port_busnum(pportdev);
   aaldevid_devaddr_devnum(aalid)      = ccip_port_devnum(pportdev);
   aaldevid_devaddr_fcnnum(aalid)      = ccip_port_fcnnum(pportdev);
   aaldevid_devaddr_subdevnum(aalid)   = devnum;

   // The following attributes describe the interfaces supported by the device
   aaldevid_afuguidl(aalid)            = CCIP_PORT_GUIDL;
   aaldevid_afuguidh(aalid)            = CCIP_PORT_GUIDH;
   aaldevid_devtype(aalid)             = aal_devtypeAFU;
   aaldevid_pipguid(aalid)             = CCIP_PORT_PIPIID;
   aaldevid_vendorid(aalid)            = AAL_vendINTC;

   // Set the interface permissions
   // Enable MMIO-R
   cci_dev_set_allow_map_mmior_space(pcci_aaldev);


   // Create the AAL device and attach it to the CCI device object
   pcci_aaldev->m_aaldev =  aaldev_create( "CCIPPORT",           // AAL device base name
                                           &aalid,             // AAL ID
                                           &cci_Portpip);

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
      PERR("Failed to initialize AAL Device for FME[%d:%d:%d:%d]",aaldevid_devaddr_busnum(aalid),
                                                                  aaldevid_devaddr_devnum(aalid),
                                                                  aaldevid_devaddr_fcnnum(aalid),
                                                                  aaldevid_devaddr_subdevnum(aalid));
      kosal_kfree(cci_dev_kvp_afu_mmio(pcci_aaldev), cci_dev_len_afu_mmio(pcci_aaldev));
      kosal_kfree(cci_dev_kvp_afu_umsg(pcci_aaldev),cci_dev_len_afu_umsg(pcci_aaldev));
      cci_destroy_aal_device(pcci_aaldev);
      return false;
   }

   // Add the device to the CCI Board device's device list
   kosal_list_add( &cci_dev_list_head(pcci_aaldev), &ccip_aal_dev_list( ccip_port_to_ccidev(pportdev) ));

   //=========================================
   // Instantiate a User AFU if one is present
   {
      // Get the AFU header pointer by adding the offset to the port header address
      struct CCIP_AFU_Header        *pafu_hdr = (struct CCIP_AFU_Header *)(((btVirtAddr)pportdev->m_pport_hdr) + pportdev->m_pport_hdr->ccip_port_next_afu.afu_id_offset);

      if(0xFFFFFFFFFFFFFFFFULL != pafu_hdr->ccip_dfh.csr){

         PVERBOSE("Instantiating User AFU\n");
         PVERBOSE("User AFU\n");
         PVERBOSE( "> Feature_ID = %x \n",pafu_hdr->ccip_dfh.Feature_ID);
         PVERBOSE("> Feature_rev = %x \n",pafu_hdr->ccip_dfh.Feature_rev);
         PVERBOSE( "> Type = %x \n",pafu_hdr->ccip_dfh.Type);
         PVERBOSE( "> afu_id_l.afu_id_l= %lx \n",( long unsigned int)pafu_hdr->ccip_afu_id_l.afu_id_l);
         PVERBOSE( "> afu_id_h.afu_id_h= %lx \n",( long unsigned int)pafu_hdr->ccip_afu_id_h.afu_id_h);
         PVERBOSE( "> next_DFH_offset = %x \n",pafu_hdr->ccip_dfh.next_DFH_offset);
         PVERBOSE( "> next_afu.afu_id_offset= %x \n",pafu_hdr->ccip_next_afu.afu_id_offset);

         // Construct the cci_aal_device object
         pcci_aaldev = cci_create_aal_device();

         ASSERT(NULL != pcci_aaldev);

         // Make it a User AFU
         cci_dev_type(pcci_aaldev) = cci_dev_AFU;

         // Bind the AAL device to the Port's AFU object. This enables the driver
         //   to get to the Port AFU object from the AAL device.
         set_cci_dev_subclass(pcci_aaldev, &ccip_port_afu_dev(pportdev));

         // Device Address is the same as the Port. Set the AFU ID information
         // The following attributes describe the interfaces supported by the device
         aaldevid_afuguidl(aalid)            = pafu_hdr->ccip_afu_id_l.afu_id_l;
         aaldevid_afuguidh(aalid)            = pafu_hdr->ccip_afu_id_h.afu_id_h;
         aaldevid_devtype(aalid)             = aal_devtypeAFU;
         aaldevid_pipguid(aalid)             = CCIP_AFU_PIPIID;
         aaldevid_vendorid(aalid)            = AAL_vendINTC;

         // Set the interface permissions
         // Enable MMIO-R
         cci_dev_set_allow_map_mmior_space(pcci_aaldev);


         // Create the AAL device and attach it to the CCI device object
         pcci_aaldev->m_aaldev =  aaldev_create( "CCIPAFU",          // AAL device base name
                                                 &aalid,             // AAL ID
                                                 &cci_AFUpip);

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
            PERR("Failed to initialize AAL Device for FME[%d:%d:%d:%d]",aaldevid_devaddr_busnum(aalid),
                                                                        aaldevid_devaddr_devnum(aalid),
                                                                        aaldevid_devaddr_fcnnum(aalid),
                                                                        aaldevid_devaddr_subdevnum(aalid));
            kosal_kfree(cci_dev_kvp_afu_mmio(pcci_aaldev), cci_dev_len_afu_mmio(pcci_aaldev));
            kosal_kfree(cci_dev_kvp_afu_umsg(pcci_aaldev),cci_dev_len_afu_umsg(pcci_aaldev));
            cci_destroy_aal_device(pcci_aaldev);
            return false;
         }

         // Add the device to the CCI Board device's device list
         kosal_list_add( &cci_dev_list_head(pcci_aaldev), &ccip_aal_dev_list( ccip_port_to_ccidev(pportdev) ));

      } // End if(0xFFFFFFFFFFFFFFFFULL == pafu_hdr->ccip_dfh.csr){
   } //End block

   return true;
}

//=============================================================================
// Name: cci_create_aal_device
// Description: Constructor for a cci_aal_device object
// Outputs: pointer to object.
// Comments: none.
//=============================================================================
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
      PERR("Attemptiong to destroy NULL pointer to cci_aal_device object\n");
      return -EINVAL;
   }

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

   if ( cci_aaldev_to_aaldev(pcci_aaldev) ) {
      PINFO("Removing AAL device\n");
      aalbus_get_bus()->unregister_device(cci_aaldev_to_aaldev(pcci_aaldev));
      cci_aaldev_to_aaldev(pcci_aaldev) = NULL;
   }

} // cci_unpublish_aaldevice

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

         if(NULL != ccip_dev_to_fme_dev(pccipdev)) {
            PVERBOSE("Freeing FME Memory\n");
            kosal_kfree(ccip_dev_to_fme_dev(pccipdev),sizeof(struct fme_device ) );
         }

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


