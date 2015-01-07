//******************************************************************************
// Part of the Intel(R) QuickAssist Technology Accelerator Abstraction Layer
//
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2012-2015, Intel Corporation.
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
//  Copyright(c) 2012-2015, Intel Corporation.
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
//        FILE: spl2_nopcie.c
//     CREATED: 02/04/2012
//      AUTHOR: Henry Mitchel, Intel <henry.mitchel@intel.com>
//              Joseph Grecco, Intel <joe.grecco@intel.com>
//
// PURPOSE:  Functions for manually configuring device
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 11/06/2011     HM       Initial version started
// 02/04/2012     JG       Ported from ccidrv stand-alone driver
// 10/25/2012     TSW      Cleanup for faplib
//****************************************************************************
#include "aalsdk/kernel/kosal.h"              // for AAL_vendINTC

#define MODULE_FLAGS SPL2_DBG_CFG /* all prints in this file use this module flag */

#include "aalsdk/kernel/aalbus.h"              // for AAL_vendINTC
#include "aalsdk/kernel/aalids.h"              // for HOST_AHM_GUID
#include "spl2_pip_internal.h"
#include "spl2_pip_public.h"
#include "aalsdk/kernel/spl2defs.h"         // Definitions of SPL and CCI PCIe configuration space
#include "spl2mem-kern.h"     // Access to memory sub-system fops structure


//=============================================================================
// Internal function forward references
//=============================================================================
//=============================================================================
// Name: cci_nopcie_internal_probe
// Description: Detect an SPL2 device
// Returns: 0 - Success
// Comments:
//=============================================================================
int
cci_nopcie_internal_probe(struct spl2_device   *pspl2dev,
                           struct aal_device_id *paaldevid)
{
   PTRACEIN;

   spl2_dev_kvp_config(pspl2dev) = ioremap(spl2_dev_phys_config(pspl2dev), spl2_dev_len_config(pspl2dev));
   if( NULL == spl2_dev_kvp_config(pspl2dev) ) {
      PERR("Failed to map 0x%" PRIxPHYS_ADDR " into kernel space.\n",
              spl2_dev_phys_config(pspl2dev));
      return -ENXIO;
   }

   // Set CCI CSR Attributes
   spl2_dev_phys_cci_csr(pspl2dev) = spl2_dev_phys_config(pspl2dev);
   spl2_dev_kvp_cci_csr(pspl2dev) = spl2_dev_kvp_config(pspl2dev);
   spl2_dev_len_cci_csr(pspl2dev) = QPI_APERTURE_SIZE;

   return cci3_init_ccidevice(pspl2dev,paaldevid);
}

//=============================================================================
// Name: spl2_nopcie_internal_probe
// Description: Detect an SPL2 device
// Returns: 0 - Success
// Comments:
//=============================================================================
int
spl2_nopcie_internal_probe(struct spl2_device   *pspl2dev,
                           struct aal_device_id *paaldevid)
{
   int res = 0;
   PTRACEIN;

   spl2_dev_kvp_config(pspl2dev) = ioremap(spl2_dev_phys_config(pspl2dev), spl2_dev_len_config(pspl2dev));
   if( NULL == spl2_dev_kvp_config(pspl2dev) ) {
      PERR("Failed to map 0x%" PRIxPHYS_ADDR " into kernel space.\n",
              spl2_dev_phys_config(pspl2dev));
      return -ENXIO;
   }

   // Set CCI CSR Attributes
   spl2_dev_phys_cci_csr(pspl2dev) = spl2_dev_phys_config(pspl2dev);
   spl2_dev_kvp_cci_csr(pspl2dev) = spl2_dev_kvp_config(pspl2dev);
   spl2_dev_len_cci_csr(pspl2dev) = QPI_APERTURE_SIZE;

   if( 0!= spl2_identify_device(pspl2dev)){
      return -ENXIO;
   }

   if(PCIE_FEATURE_HDR3_PROTOCOL_SPL == spl2_dev_protocol(pspl2dev) ){
      PVERBOSE("Detected SPL3 Device.\n");
      res = spl2_init_spl3device(pspl2dev, paaldevid);
   }else if(PCIE_FEATURE_HDR3_PROTOCOL_CCI3 == spl2_dev_protocol(pspl2dev)){
      PVERBOSE("Detected CCI3 Device.\n");
      res = cci3_init_ccidevice(pspl2dev, paaldevid);
   }else{
      PERR("Invalid protocol %d. Only SPL3 or CCI3 supported\n",spl2_dev_protocol(pspl2dev));
      return -ENXIO;
   }

   return res;
}

//=============================================================================
// Name: spl2_sim_internal_probe
// Description: Create a simulated device
// Input: pdev - device to attach simulator to.
// Comment: When a simulated device is created the "real" device does not get
//          instantiated. A MAFU device  is created instead. The
//          simulator application is responsible for instantiating the "real"
//          AFU device using the MAFU.
// Returns: 0 - Success
// Comments:
//=============================================================================
int
spl2_sim_internal_probe(struct spl2_device   *pspl2dev,
                        struct aal_device_id *paaldevid)
{
   struct PCIE_FEATURE_HDR_F2 feature_id;

   // Allocate the config area
   spl2_dev_kvp_config(pspl2dev) = kmalloc(spl2_dev_len_config(pspl2dev), GFP_KERNEL);
   ASSERT(spl2_dev_kvp_config(pspl2dev));
   if ( NULL == spl2_dev_kvp_config(pspl2dev) ) {
      return -ENOMEM;
   }
   spl2_dev_phys_config(pspl2dev) = virt_to_phys(spl2_dev_kvp_config(pspl2dev));

   spl2_dev_set_simulated(pspl2dev);
   pspl2dev->m_simPIP = &MAFUpip;

   // Simulate the signature
   *(btUnsigned32bitInt *)spl2_dev_kvp_config(pspl2dev) =
            (((btUnsigned32bitInt)QPI_DEVICE_ID_FPGA) << 16) | ((btUnsigned32bitInt)PCI_VENDOR_ID_INTEL);

   PNOTICE("Simulating board at phys=0x%" PRIxPHYS_ADDR " size=0x%x, kvirt=%p, with signature 0x%X\n",
              spl2_dev_phys_config(pspl2dev),
              (unsigned int)spl2_dev_len_config(pspl2dev),
              spl2_dev_kvp_config(pspl2dev),
              *(btUnsigned32bitInt *)spl2_dev_kvp_config(pspl2dev));


   // Device simulates CCI2
   memset(&feature_id, 0, sizeof(struct PCIE_FEATURE_HDR_F2));

   feature_id.FeatureID = FeatureID_CCI2;
   write_cci_csr32(pspl2dev, byte_offset_PCIE_FEATURE_HDR_F2, feature_id.csr);

   // Setup the MAFU device ID
   aaldevid_afuguidl(*paaldevid) = SPL2_SIM_AFUIDL;
   aaldevid_afuguidh(*paaldevid) = SPL2_SIM_AFUIDH;
   aaldevid_devtype(*paaldevid)  = aal_devtypeMgmtAFU;
   aaldevid_pipguid(*paaldevid)  = SPL2_MAFUPIP_IID;
   aaldevid_vendorid(*paaldevid) = AAL_vendINTC;
   aaldevid_ahmguid(*paaldevid)  = HOST_AHM_GUID;

   // Setup device address - TODO only a single simulated device supported for now
   aaldevid_devaddr_bustype(*paaldevid)   = aal_bustype_Host;
   aaldevid_devaddr_busnum(*paaldevid)    = 0;
   aaldevid_devaddr_devnum(*paaldevid)    = 0;
   aaldevid_devaddr_subdevnum(*paaldevid) = 0;

   // TODO
   //spl2_dev_board_type(pspl2dev) = aal_bustype_QPI;

   return 0;
}

