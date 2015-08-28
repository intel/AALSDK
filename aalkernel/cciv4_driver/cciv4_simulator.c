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
//        FILE: cciv3_simulator.c
//     CREATED: Jul 27, 2015
//      AUTHOR: Joseph Grecco <joe.grecco@intel.com>
//
// PURPOSE:   This file contains the implementation of the simulated CCIv3
//            device
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
//****************************************************************************///
#include "aalsdk/kernel/kosal.h"
#include "aalsdk/kernel/aalids.h"
#include "aalsdk/kernel/aalbus.h"

#include "cciv4_driver_internal.h"
#include "cciv4_simulator.h"

#define MODULE_FLAGS CCIV4_DBG_MOD // Prints all

int cciv4_create_sim_Mafu(btVirtAddr,uint ,struct aal_device_id*,struct aal_ipip*,struct list_head *);
int cciv4_create_sim_afu(btVirtAddr,uint ,struct aal_device_id*,struct list_head *);

#define CCIV4_MMIO_UMSG_TEST 1

#if CCIV4_MMIO_UMSG_TEST
static char mmioafustring[]    = "CCIv4 MMIO test  \n";
static char umesgafustring[]   = "CCIv4 UMSG test  \n";
#endif

//=============================================================================
// nextAFU_addr - Keeps the next available address for new AFUs
//=============================================================================
static struct aal_device_addr nextAFU_addr = {
   .m_bustype   = aal_bustype_Host,
   .m_busnum    = 1,       //
   .m_devicenum = 1,       //
   .m_subdevnum = 0        // AFU
};


//=============================================================================
// Name: cciv4_alloc_next_afu_addr
// Description: Allocate the next AFU address
// Interface: public
// Returns 0 - success
// Inputs: none
// Outputs: none.
// Comments: Allocates sequential addresses using only the 16 LSBs of the
//           device number and address.  Eventually the address structure may
//           change to encode the device and subdevice into a single 32 bit int.
//=============================================================================
struct aal_device_addr
cciv4_sim_alloc_next_afu_addr(void)
{
   ++(nextAFU_addr.m_subdevnum);
   if( 0 == (nextAFU_addr.m_subdevnum &= 0xffff) ) {
      ++(nextAFU_addr.m_devicenum);
      nextAFU_addr.m_devicenum &= 0xffff;
   }
   return nextAFU_addr;
} // cciv4_alloc_next_afu_addr


//=============================================================================
// Name: cciv4_sim_discover_devices
// Description: Performs the functionality of the PCIe OS enumeration functions
//              for real hardware.
// Inputs: numdevices - Number of AFUs to "discover".
//         g_device_list - List head of global device list.
// Outputs: 0 - success.
// Comments: none.
//=============================================================================
int cciv4_sim_discover_devices(ulong numdevices,
                               struct list_head *g_device_list)
{
   int ret                       = 0;
   struct aal_device_id aalid;
   btVirtAddr pAperture          = NULL;

   PVERBOSE("Discovering %ld Simulated CCIv4 devices", numdevices);

   // Setup the ID for this AHM. AFU's are at bus 1 and AHM MAFUs on bus 0
   //   and Channel MAFU is bus 2
   aaldevid_addr(aalid) = nextAFU_addr;      // 0:1:0
   aaldevid_devaddr_busnum(aalid) = 0;

   pAperture = kosal_kzmalloc( CCIV4_SIM_APERTURE_SIZE );
    if(NULL == pAperture){
       PERR("Unable to allocate system memory for simulated BAR\n");
       return -EINVAL;
    }

   cciv4_create_sim_Mafu( pAperture,
                          CCIV4_SIM_APERTURE_SIZE,
                          &aalid,
                          &cciv4_simMAFUpip,
                          g_device_list);

   // Loop through and probe each simulated device
   while(numdevices--){

      // Create Channel AFU for this device
      aaldevid_addr(aalid) = cciv4_sim_alloc_next_afu_addr();

      // Allocate the BAR for the simulated device
      pAperture = kosal_kzmalloc( CCIV4_SIM_APERTURE_SIZE );
      if(NULL == pAperture){
         PERR("Unable to allocate system memory for simulated BAR\n");
         return -EINVAL;
      }

      // Create the AFU
      aaldevid_devaddr_busnum(aalid) = 1;
      ret = cciv4_create_sim_afu(pAperture, CCIV4_SIM_APERTURE_SIZE, &aalid, g_device_list);
      ASSERT(0 == ret);
      if(0>ret){
         kfree(pAperture);
         PERR("Unable to create simulated device\n");
      }

      //-----------------------------------
      // Create Channel MAFU for this device
      aaldevid_devaddr_busnum(aalid) = 2;       // CMAFU is on bus 2 same dev,subdev

      // Allocate the BAR for the simulated device
      pAperture = kosal_kzmalloc( CCIV4_SIM_APERTURE_SIZE );
      if(NULL == pAperture){
         PERR("Unable to allocate system memory for simulated BAR\n");
         return -EINVAL;
      }

      // Create the CMAFU
      ret = cciv4_create_sim_Mafu(pAperture,
                                  CCIV4_SIM_APERTURE_SIZE,
                                  &aalid,
                                  &cciv4_simCMAFUpip,
                                  g_device_list);
      ASSERT(0 == ret);
      if(0>ret){
         kfree(pAperture);
         PERR("Unable to create simulated device\n");
      }

   }// end while
   return 0;
}

//=============================================================================
// Name: cciv4_create_sim_Mafu
// Description: Performs the functionality of the PCIe Probe functions for real
//              hardware. It causes AAL Devices to be instantiated for the
//              number of simulated devices "discovered".
// Inputs: virtAddr - Virtual Address of aperture.
//         size - size of aperture.
//         g_device_list - List head of global device list.
// Outputs: 0 - success.
// Comments: none.
//=============================================================================
int cciv4_create_sim_Mafu(btVirtAddr virtAddr,
                          uint size,
                          struct aal_device_id *paalid,
                          struct aal_ipip* pPIP,
                          struct list_head *pdevice_list)
{
   struct cciv4_device *pCCIv4dev = NULL;
   btVirtAddr ptemp               = NULL;
   int ret                        = 0;

   // Construct the cciv4_device object
   pCCIv4dev = cciv4_create_device();

   // Set up Simulated Config Space
   cciv4_dev_len_config(pCCIv4dev)  = size;
   cciv4_dev_kvp_config(pCCIv4dev)  = virtAddr;
   cciv4_dev_phys_config(pCCIv4dev) = virt_to_phys(cciv4_dev_kvp_config(pCCIv4dev));

   // Set CCI CSR Attributes
   cciv4_dev_phys_cci_csr(pCCIv4dev) = cciv4_dev_phys_config(pCCIv4dev);
   cciv4_dev_kvp_cci_csr(pCCIv4dev)  = cciv4_dev_kvp_config(pCCIv4dev);
   cciv4_dev_len_cci_csr(pCCIv4dev)  = QPI_APERTURE_SIZE;

   // Allocate uMSG space K
   ptemp = kosal_kzmalloc(CCIV4_UMSG_SIZE);
   if ( NULL == ptemp ) {
      PERR("Unable to allocate system memory for uMsg area\n");
      cciv4_destroy_device(pCCIv4dev);
      return -ENOMEM;
   }

   cciv4_dev_len_afu_umsg(pCCIv4dev) = size;
   cciv4_dev_kvp_afu_umsg(pCCIv4dev) = ptemp;
   cciv4_dev_phys_afu_umsg(pCCIv4dev) = virt_to_phys(ptemp);

   // Allocate MMIO space
   ptemp = kosal_kzmalloc(CCIV4_MMIO_SIZE);
   if ( NULL == ptemp ) {
      PERR("Unable to allocate system memory for MMIO area\n");
      kosal_kfree(cciv4_dev_kvp_afu_umsg(pCCIv4dev), cciv4_dev_len_afu_umsg(pCCIv4dev));
      cciv4_destroy_device(pCCIv4dev);
      return -ENOMEM;
   }

   cciv4_dev_len_afu_mmio(pCCIv4dev) = size;
   cciv4_dev_kvp_afu_mmio(pCCIv4dev) = ptemp;
   cciv4_dev_phys_afu_mmio(pCCIv4dev) = virt_to_phys(cciv4_dev_kvp_afu_mmio(pCCIv4dev));

   // Set  AFU specifics
   aaldevid_afuguidl(*paalid) = CCIV4_SIM_MAFUIDL;
   aaldevid_afuguidh(*paalid) = CCIV4_SIM_MAFUIDH;
   aaldevid_devtype(*paalid)  = aal_devtypeMgmtAFU;
   aaldevid_pipguid(*paalid)  = 0;    // Not really used here since PIP is specified by pointer in "publish"
   aaldevid_vendorid(*paalid) = AAL_vendINTC;
   aaldevid_ahmguid(*paalid)  = HOST_AHM_GUID;

   // Enable CSR, MMIO-R and UMSG space
   cciv4_dev_set_allow_map_csr_write_space(pCCIv4dev);
   cciv4_dev_set_allow_map_csr_read_space(pCCIv4dev);
   cciv4_dev_set_allow_map_mmior_space(pCCIv4dev);
   cciv4_dev_set_allow_map_umsg_space(pCCIv4dev);

   // Mark as simulated
   cciv4_set_simulated(pCCIv4dev);

   // Device is ready for use.  Publish it with the Configuration Management Subsystem
   ret = cciv4_publish_aaldevice(pCCIv4dev, paalid, pPIP, 1);
   ASSERT(ret == 0);
   if(0> ret){
      PERR("Failed to initialize AAL Device for simulated CCIV4 MAFU[%d:%d:%d]",aaldevid_devaddr_busnum(*paalid),
                                                                                aaldevid_devaddr_devnum(*paalid),
                                                                                aaldevid_devaddr_subdevnum(*paalid));
      kosal_kfree(cciv4_dev_kvp_afu_mmio(pCCIv4dev), cciv4_dev_len_afu_mmio(pCCIv4dev));
      kosal_kfree(cciv4_dev_kvp_afu_umsg(pCCIv4dev),cciv4_dev_len_afu_umsg(pCCIv4dev));
      cciv4_destroy_device(pCCIv4dev);
      return -EINVAL;
   }

   // Add the device to the device list
   kosal_list_add(&cciv4_dev_list_head(pCCIv4dev), pdevice_list);
   PVERBOSE("Initialize AAL Device for simulated CCIV4 MAFU[%d:%d:%d]",aaldevid_devaddr_busnum(*paalid),
                                                                       aaldevid_devaddr_devnum(*paalid),
                                                                       aaldevid_devaddr_subdevnum(*paalid));
   return 0;
}


//=============================================================================
// Name: cciv4_create_sim_afu
// Description: Performs the functionality of the PCIe Probe functions for real
//              hardware. It causes AAL Devices to be instantiated for the
//              number of simulated devices "discovered".
// Inputs: virtAddr - Virtual Address of aperture.
//         size - size of aperture.
//         g_device_list - List head of global device list.
// Outputs: 0 - success.
// Comments: none.
//=============================================================================
int cciv4_create_sim_afu(btVirtAddr virtAddr,
                         uint size,
                         struct aal_device_id *paalid,
                         struct list_head *pdevice_list)
{
   struct cciv4_device *pCCIv4dev = NULL;
   btVirtAddr ptemp               = NULL;
   int ret                        = 0;

   // Construct the cciv4_device object
   pCCIv4dev = cciv4_create_device();

   // Set up Simulated Config Space
   cciv4_dev_len_config(pCCIv4dev)  = size;
   cciv4_dev_kvp_config(pCCIv4dev)  = virtAddr;
   cciv4_dev_phys_config(pCCIv4dev) = virt_to_phys(cciv4_dev_kvp_config(pCCIv4dev));

   // Set CCI CSR Attributes
   cciv4_dev_phys_cci_csr(pCCIv4dev) = cciv4_dev_phys_config(pCCIv4dev);
   cciv4_dev_kvp_cci_csr(pCCIv4dev)  = cciv4_dev_kvp_config(pCCIv4dev);
   cciv4_dev_len_cci_csr(pCCIv4dev)  = QPI_APERTURE_SIZE;

   // Allocate uMSG space K
   ptemp = kosal_kzmalloc(CCIV4_UMSG_SIZE);
   if ( NULL == ptemp ) {
      PERR("Unable to allocate system memory for uMsg area\n");
      cciv4_destroy_device(pCCIv4dev);
      return -ENOMEM;
   }

#if CCIV4_MMIO_UMSG_TEST
   strncpy((char*)ptemp,umesgafustring,strlen(umesgafustring));
#endif

   cciv4_dev_len_afu_umsg(pCCIv4dev) = size;
   cciv4_dev_kvp_afu_umsg(pCCIv4dev) = ptemp;
   cciv4_dev_phys_afu_umsg(pCCIv4dev) = virt_to_phys(ptemp);

   // Allocate MMIO space
   ptemp = kosal_kzmalloc(CCIV4_MMIO_SIZE);
   if ( NULL == ptemp ) {
      PERR("Unable to allocate system memory for MMIO area\n");
      kosal_kfree(cciv4_dev_kvp_afu_umsg(pCCIv4dev), cciv4_dev_len_afu_umsg(pCCIv4dev));
      cciv4_destroy_device(pCCIv4dev);
      return -ENOMEM;
   }

#if CCIV4_MMIO_UMSG_TEST
   strncpy((char*)ptemp,mmioafustring,strlen(mmioafustring));
#endif

   cciv4_dev_len_afu_mmio(pCCIv4dev) = size;
   cciv4_dev_kvp_afu_mmio(pCCIv4dev) = ptemp;
   cciv4_dev_phys_afu_mmio(pCCIv4dev) = virt_to_phys(cciv4_dev_kvp_afu_mmio(pCCIv4dev));

   // Set  AFU specifics
   aaldevid_afuguidl(*paalid) = CCIV4_SIM_AFUIDL;
   aaldevid_afuguidh(*paalid) = CCIV4_SIM_AFUIDH;
   aaldevid_devtype(*paalid)  = aal_devtypeAFU;
   aaldevid_pipguid(*paalid)  = CCIV4_SIMAFUPIP_IID;
   aaldevid_vendorid(*paalid) = AAL_vendINTC;
   aaldevid_ahmguid(*paalid)  = HOST_AHM_GUID;

   // Direct user mode CSR interface not supported
   cciv4_dev_clr_allow_map_csr_write_space(pCCIv4dev);
   cciv4_dev_clr_allow_map_csr_read_space(pCCIv4dev);

   // Enable MMIO-R and UMSG space
   cciv4_dev_set_allow_map_mmior_space(pCCIv4dev);
   cciv4_dev_set_allow_map_umsg_space(pCCIv4dev);

   // Mark as simulated
   cciv4_set_simulated(pCCIv4dev);

   // Device is ready for use.  Publish it with the Configuration Management Subsystem
   ret = cciv4_publish_aaldevice(pCCIv4dev, paalid, &cciv4_simAFUpip, 1);
   ASSERT(ret == 0);
   if(0> ret){
      PERR("Failed to initialize AAL Device for simulated CCIV4 AFU[%d:%d:%d]",aaldevid_devaddr_busnum(*paalid),
                                                                              aaldevid_devaddr_devnum(*paalid),
                                                                              aaldevid_devaddr_subdevnum(*paalid));
      kosal_kfree(cciv4_dev_kvp_afu_mmio(pCCIv4dev), cciv4_dev_len_afu_mmio(pCCIv4dev));
      kosal_kfree(cciv4_dev_kvp_afu_umsg(pCCIv4dev),cciv4_dev_len_afu_umsg(pCCIv4dev));
      cciv4_destroy_device(pCCIv4dev);
      return -EINVAL;
   }

   // Add the device to the device list
   kosal_list_add(&cciv4_dev_list_head(pCCIv4dev), pdevice_list);

   return 0;
}
