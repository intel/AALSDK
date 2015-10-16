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
//      AUTHOR: Joseph Grecco <joe.grecco@intel.com>
//
// PURPOSE:   This file contains the implementation of the simulated CCI
//            device
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
//****************************************************************************///
#include "aalsdk/kernel/kosal.h"
#include "aalsdk/kernel/aalids.h"
#include "aalsdk/kernel/aalbus.h"

#include "cci_pcie_driver_internal.h"
#include "cci_pcie_driver_simulator.h"

#define MODULE_FLAGS CCIPCIE_DBG_MOD

int cci_create_sim_afu(btVirtAddr,uint ,struct aal_device_id*,struct list_head *);

#define CCIV4_MMIO_UMSG_TEST 0

#if CCIV4_MMIO_UMSG_TEST
// Turn on in AFUdev.cpp, as well
static char mmioafustring[]    = "CCIv4 MMIO test  \n";
static char umesgafustring[]   = "CCIv4 UMSG test  \n";
#endif


struct aal_ipip cci_simAFUpip; //TEMP

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
// Name: cci_sim_alloc_next_afu_addr
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
cci_sim_alloc_next_afu_addr(void)
{
   ++(nextAFU_addr.m_subdevnum);
   if( 0 == (nextAFU_addr.m_subdevnum &= 0xffff) ) {
      ++(nextAFU_addr.m_devicenum);
      nextAFU_addr.m_devicenum &= 0xffff;
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
                             struct list_head *g_device_list)
{
   int                  ret               = 0;
   struct aal_device_id aalid;
   btVirtAddr           pAperture         = NULL;

   PVERBOSE("Creating %ld simulated CCI devices", numdevices);

   // Loop through and probe each simulated device
   while(numdevices--){

      // Create Channel AFU for this device
      aaldevid_addr(aalid) = cci_sim_alloc_next_afu_addr();

      // Allocate the BAR for the simulated device
      pAperture = kosal_kzmalloc( CCI_SIM_APERTURE_SIZE );
      if(NULL == pAperture){
         PERR("Unable to allocate system memory for simulated BAR\n");
         return -EINVAL;
      }

      // Create the AFU
      ret = cci_create_sim_afu(pAperture, CCI_SIM_APERTURE_SIZE, &aalid, g_device_list);
      ASSERT(0 == ret);
      if(0>ret){
         kfree(pAperture);
         PERR("Unable to create simulated device\n");
      }
   }// end while
   return 0;
}


//=============================================================================
// Name: cci_create_sim_afu
// Description: Performs the functionality of the PCIe Probe functions for real
//              hardware. It causes AAL Devices to be instantiated for the
//              number of simulated devices "discovered".
// Inputs: virtAddr - Virtual Address of aperture.
//         size - size of aperture.
//         g_device_list - List head of global device list.
// Outputs: 0 - success.
// Comments: none.
//=============================================================================
int cci_create_sim_afu( btVirtAddr virtAddr,
                        uint size,
                        struct aal_device_id *paalid,
                        struct list_head *pdevice_list)
{
   struct cci_device *pCCIdev    = NULL;
   btVirtAddr         ptemp      = NULL;
   int                ret        = 0;

   // Construct the cci_device object
   pCCIdev = cci_create_device();

   // Set up Simulated Config Space
   cci_dev_len_config(pCCIdev)  = size;
   cci_dev_kvp_config(pCCIdev)  = virtAddr;
   cci_dev_phys_config(pCCIdev) = virt_to_phys(cci_dev_kvp_config(pCCIdev));

   // Set CCI CSR Attributes
   cci_dev_phys_cci_csr(pCCIdev) = cci_dev_phys_config(pCCIdev);
   cci_dev_kvp_cci_csr(pCCIdev)  = cci_dev_kvp_config(pCCIdev);
   cci_dev_len_cci_csr(pCCIdev)  = CCI_SIM_APERTURE_SIZE;

   // Allocate uMSG space
   ptemp = kosal_kzmalloc(CCI_UMSG_SIZE);
   if ( NULL == ptemp ) {
      PERR("Unable to allocate system memory for uMsg area\n");
      cci_destroy_device(pCCIdev);
      return -ENOMEM;
   }

#if CCIV4_MMIO_UMSG_TEST
   strncpy((char*)ptemp,umesgafustring,strlen(umesgafustring));
#endif

   cci_dev_len_afu_umsg(pCCIdev) = size;
   cci_dev_kvp_afu_umsg(pCCIdev) = ptemp;
   cci_dev_phys_afu_umsg(pCCIdev) = virt_to_phys(ptemp);

   // Allocate MMIO space
   ptemp = kosal_kzmalloc(CCI_MMIO_SIZE);
   if ( NULL == ptemp ) {
      PERR("Unable to allocate system memory for MMIO area\n");
      kosal_kfree(cci_dev_kvp_afu_umsg(pCCIdev), cci_dev_len_afu_umsg(pCCIdev));
      cci_destroy_device(pCCIdev);
      return -ENOMEM;
   }

#if CCIV4_MMIO_UMSG_TEST
   strncpy((char*)ptemp,mmioafustring,strlen(mmioafustring));
#endif

   cci_dev_len_afu_mmio(pCCIdev) = size;
   cci_dev_kvp_afu_mmio(pCCIdev) = ptemp;
   cci_dev_phys_afu_mmio(pCCIdev) = virt_to_phys(cci_dev_kvp_afu_mmio(pCCIdev));

   // Set  AFU specifics
   aaldevid_afuguidl(*paalid) = CCI_SIM_AFUIDL;
   aaldevid_afuguidh(*paalid) = CCI_SIM_AFUIDH;
   aaldevid_devtype(*paalid)  = aal_devtypeAFU;
   aaldevid_pipguid(*paalid)  = CCI_SIMAFUPIP_IID;
   aaldevid_vendorid(*paalid) = AAL_vendINTC;
   aaldevid_ahmguid(*paalid)  = HOST_AHM_GUID;

   // Direct user mode CSR interface not supported
   cci_dev_clr_allow_map_csr_write_space(pCCIdev);
   cci_dev_clr_allow_map_csr_read_space(pCCIdev);

   // Enable MMIO-R and uMSG space
   cci_dev_set_allow_map_mmior_space(pCCIdev);
   cci_dev_set_allow_map_umsg_space(pCCIdev);

   // Mark as simulated
   cci_set_simulated(pCCIdev);

   // Create the AAL device structure. The AAL device is the
   //  base class for all devices in AAL. Among other things it
   //  holds the device name, address and pointer to Low Level Communications
   //  module (PIP)
   pCCIdev->m_aaldev =  aaldev_create( "CCISIMAFU",
                                        paalid,
                                        &cci_simAFUpip);

   // Set how many owners are allowed access to this device simultaneously
   pCCIdev->m_aaldev->m_maxowners = 1;

      // Device is ready for use.  Publish it with the Configuration Management Subsystem
   ret = cci_publish_aaldevice(pCCIdev);
   ASSERT(ret == 0);
   if(0> ret){
      PERR("Failed to initialize AAL Device for simulated CCIV4 AFU[%d:%d:%d]",aaldevid_devaddr_busnum(*paalid),
                                                                               aaldevid_devaddr_devnum(*paalid),
                                                                               aaldevid_devaddr_subdevnum(*paalid));
      kosal_kfree(cci_dev_kvp_afu_mmio(pCCIdev), cci_dev_len_afu_mmio(pCCIdev));
      kosal_kfree(cci_dev_kvp_afu_umsg(pCCIdev),cci_dev_len_afu_umsg(pCCIdev));
      cci_destroy_device(pCCIdev);
      return -EINVAL;
   }

   // Add the device to the device list
   kosal_list_add(&cci_dev_list_head(pCCIdev), pdevice_list);

   return 0;
}
