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

int cciv4_probe(btVirtAddr, uint, struct list_head *);

//=============================================================================
// nextAFU_addr - Keeps the next available address for new AFUs
//=============================================================================
static struct aal_device_addr nextAFU_addr = {
   .m_bustype   = aal_bustype_Host,
   .m_busnum    = 0,       //
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
   int ret = 0;
   PVERBOSE("Discovering %ld Simulated CCIv4 devices", numdevices);

   // Loop through and probe each simulated device
   while(numdevices--){
      // Allocate the BAR for the simulated device
      btVirtAddr pAperture = kosal_kzmalloc( CCIV4_SIM_APERTURE_SIZE );
      if(NULL == pAperture){
         PERR("Unable to allocate system memory for simulated BAR\n");
         return -EINVAL;
      }

      ret = cciv4_probe(pAperture, CCIV4_SIM_APERTURE_SIZE, g_device_list);
      ASSERT(0 == ret);
      if(0>ret){
         kfree(pAperture);
         PERR("Unable to create simulated device\n");
      }

   }// end while
   return 0;
}

//=============================================================================
// Name: cciv4_probe
// Description: Performs the functionality of the PCIe Probe functions for real
//              hardware. It causes AAL Devices to be instantiated for the
//              number of simulated devices "discovered".
// Inputs: virtAddr - Virtual Address of aperture.
//         size - size of aperture.
//         g_device_list - List head of global device list.
// Outputs: 0 - success.
// Comments: none.
//=============================================================================
int cciv4_probe(btVirtAddr virtAddr, uint size, struct list_head *pdevice_list)
{
   struct cciv4_device *pCCIv4dev = NULL;
   btVirtAddr ptemp               = NULL;
   struct aal_device_id aaldevid;
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

   // Setup the ID for this instance
   aaldevid_addr(aaldevid) = cciv4_sim_alloc_next_afu_addr();

   // Set  AFU specifics
   aaldevid_afuguidl(aaldevid) = CCIV4_SIM_AFUIDL;
   aaldevid_afuguidh(aaldevid) = CCIV4_SIM_AFUIDH;
   aaldevid_devtype(aaldevid)  = aal_devtypeAFU;
   aaldevid_pipguid(aaldevid)  = CCIV4_SIMAFUPIP_IID;
   aaldevid_vendorid(aaldevid) = AAL_vendINTC;
   aaldevid_ahmguid(aaldevid)  = HOST_AHM_GUID;

   // Direct user mode CSR interface not supported
   cciv4_dev_clr_allow_map_csr_write_space(pCCIv4dev);
   cciv4_dev_clr_allow_map_csr_read_space(pCCIv4dev);


   // Device is ready for use.  Publish it with the Configuration Management Subsystem
   ret = cciv4_publish_aaldevice(pCCIv4dev, &aaldevid, &CCIV4_SIMAFUpip, 1);
   ASSERT(ret == 0);
   if(0> ret){
      PERR("Failed to initialize AAL Device for simulated CCIV4 AFU[%d:%d:%d]",aaldevid_devaddr_busnum(aaldevid),
                                                                              aaldevid_devaddr_devnum(aaldevid),
                                                                              aaldevid_devaddr_subdevnum(aaldevid));
      kosal_kfree(cciv4_dev_kvp_afu_mmio(pCCIv4dev), cciv4_dev_len_afu_mmio(pCCIv4dev));
      kosal_kfree(cciv4_dev_kvp_afu_umsg(pCCIv4dev),cciv4_dev_len_afu_umsg(pCCIv4dev));
      cciv4_destroy_device(pCCIv4dev);
      return -EINVAL;
   }

   // Add the device to the device list
   kosal_list_add(&cciv4_dev_list_head(pCCIv4dev), pdevice_list);

   return 0;
}
