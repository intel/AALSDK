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
//        FILE: ccip_sim_mmio.c
//     CREATED:
//      AUTHOR: Joseph Grecco, Intel <joe.grecco@intel.com>
//              Ananda Ravuri, Intel <ananda.ravuri@intel.com>
// PURPOSE:
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
//****************************************************************************
#include "aalsdk/kernel/kosal.h"

#define MODULE_FLAGS CCIPCIE_DBG_MOD

#include "aalsdk/kernel/aalbus.h"
#include "aalsdk/kernel/aalinterface.h"
#include "aalsdk/kernel/aalids.h"
#include "aalsdk/kernel/aalrm.h"
#include "aalsdk/kernel/aalqueue.h"
#include "aalsdk/kernel/ccipdriver.h"
#include "cci_pcie_driver_internal.h"
#include "cci_pcie_driver_simulator.h"


#include "ccip_fme.h"
#include "ccip_port.h"

#define OFFSET 0x8

int  ccip_sim_wrt_fme_mmio(btVirtAddr pkvp_fme_mmio)
{
   struct CCIP_FME_HDR           fme_hdr;
   struct CCIP_FME_DFL_THERM     fme_tmp;
   struct CCIP_FME_DFL_PM        fme_pm;
   struct CCIP_FME_DFL_FPMON     fme_fpmon;
   struct CCIP_FME_DFL_GERROR    fme_gerror;
   struct CCIP_FME_DFL_PR        fme_pr;
   struct CCIP_PORT_AFU_OFFSET   portoffset;
   struct CCIP_FME_PCIE1_ERROR   pcie1error;
   struct CCIP_FME_ERROR0        firsterror;

   int offset =0;

   btVirtAddr ptr= pkvp_fme_mmio;

   PINFO(" ccip_sim_wrt_fme_mmio ENTER\n");

   // FME header
   fme_hdr.dfh.Type =CCIP_DFType_afu;
   fme_hdr.dfh.next_DFH_offset =0x1000;
   fme_hdr.dfh.Feature_rev =0;
   fme_hdr.dfh.Feature_ID =1;
   fme_hdr.dfh.eol =0x0;
   write_ccip_csr64(ptr,offset, fme_hdr.dfh.csr);

   // FME AFU id low
   fme_hdr.afu_id_l.afu_id_l = CCIP_FME_GUIDL;
   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,fme_hdr.afu_id_l.csr);

   // FME AFU id high
   fme_hdr.afu_id_h.afu_id_h = CCIP_FME_GUIDH;
   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,fme_hdr.afu_id_h.csr);

   // FME next afu id offset
   fme_hdr.next_afu.afu_id_offset=0x0;
   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,fme_hdr.next_afu.csr);

   // RSVD
   fme_hdr.rsvd_fmehdr =0;
   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,fme_hdr.rsvd_fmehdr);

   // FME sratchpad csr
   fme_hdr.scratchpad.scratch_pad = 0x1234568;
   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset, fme_hdr.scratchpad.csr);

   // FME Capability CSR
   fme_hdr.fab_capability.lock_bit =0x0;
   fme_hdr.fab_capability.cache_assoc =0x0;
   fme_hdr.fab_capability.cache_size = 0x10;
   fme_hdr.fab_capability.address_width_bits =0x26;
   fme_hdr.fab_capability.iommu_support =0x1;
   fme_hdr.fab_capability.qpi_link_avile =0x01;
   fme_hdr.fab_capability.pci0_link_avile =0x01;
   fme_hdr.fab_capability.pci1_link_avile =0x01;
   fme_hdr.fab_capability.socket_id =0x1;
   fme_hdr.fab_capability.fabric_verid =0x1;
   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,fme_hdr.fab_capability.csr);


   // 3 simulated ports
   //port 1
   portoffset.port_imp =0x1;
   portoffset.afu_access_control =0x1;
   portoffset.port_bar =0x2;
   portoffset.port_offset =0x00000;
   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,portoffset.csr);

   //port 2
   portoffset.port_imp =0x1;
   portoffset.afu_access_control =0x1;
   portoffset.port_bar =0x1;
   portoffset.port_offset =0x00000;
   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,portoffset.csr);

   //port 3
   portoffset.port_imp =0x0;
   portoffset.afu_access_control =0x1;
   portoffset.port_bar =0x1;
   portoffset.port_offset =0x80000;
   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,portoffset.csr);

   // FME Feature List

   // FME Temperature Management  Feature

   ptr = ptr+ fme_hdr.dfh.next_DFH_offset;
   fme_tmp.ccip_fme_tmp_dflhdr.Type =CCIP_DFType_private;
   fme_tmp.ccip_fme_tmp_dflhdr.next_DFH_offset =0x1000;
   fme_tmp.ccip_fme_tmp_dflhdr.Feature_rev =0;
   fme_tmp.ccip_fme_tmp_dflhdr.Feature_ID =CCIP_FME_DFLID_THERM;
   fme_tmp.ccip_fme_tmp_dflhdr.eol =0x0;
   offset = 0;
   write_ccip_csr64(ptr,offset,fme_tmp.ccip_fme_tmp_dflhdr.csr);


   // Temperature threshold csr
   fme_tmp.ccip_tmp_threshold.therm_trip_thshold =0xa;
   fme_tmp.ccip_tmp_threshold.therm_trip_thshold_status =0x1;
   fme_tmp.ccip_tmp_threshold.thshold1_status =0x1;
   fme_tmp.ccip_tmp_threshold.thshold2_status =0x1;
   fme_tmp.ccip_tmp_threshold.thshold_policy =0x1;
   fme_tmp.ccip_tmp_threshold.tmp_thshold1 =0xb;
   fme_tmp.ccip_tmp_threshold.tmp_thshold1_status =0x1;
   fme_tmp.ccip_tmp_threshold.tmp_thshold2 =0xc;
   fme_tmp.ccip_tmp_threshold.tmp_thshold2_status =0x1;
   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,fme_tmp.ccip_tmp_threshold.csr);

   // Temperature Sensor1 csr
   fme_tmp.ccip_tmp_rdssensor_fm1.dbg_mode =0x1;
   fme_tmp.ccip_tmp_rdssensor_fm1.tmp_reading =0x55;
   fme_tmp.ccip_tmp_rdssensor_fm1.tmp_reading_valid =0x1;
   fme_tmp.ccip_tmp_rdssensor_fm1.tmp_reading_seq_num = 0xabcd;
   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,fme_tmp.ccip_tmp_rdssensor_fm1.csr);


   // Temperature Sensor2 csr
   fme_tmp.ccip_tmp_rdssensor_fm2.rsvd =0x0;
   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,fme_tmp.ccip_tmp_rdssensor_fm2.csr);

   // FME Power Management  Feature

   ptr = ptr+ fme_tmp.ccip_fme_tmp_dflhdr.next_DFH_offset;
   fme_pm.ccip_fme_pm_dflhdr.Type =CCIP_DFType_private;
   fme_pm.ccip_fme_pm_dflhdr.next_DFH_offset =0x1000;
   fme_pm.ccip_fme_pm_dflhdr.Feature_rev =0;
   fme_pm.ccip_fme_pm_dflhdr.Feature_ID= CCIP_FME_DFLID_POWER;
   fme_pm.ccip_fme_pm_dflhdr.eol =0x0;
   offset = 0 ;
   write_ccip_csr64(ptr,offset,fme_pm.ccip_fme_pm_dflhdr.csr);


   // Power Management status
   fme_pm.pm_status.pwr_consumed = 0x299;
   fme_pm.pm_status.fpga_latency_report = 0x1;

   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,fme_pm.pm_status.csr);



   // FME Global Performance header

   ptr = ptr+ fme_pm.ccip_fme_pm_dflhdr.next_DFH_offset;
   fme_fpmon.ccip_fme_fpmon_dflhdr.Type =CCIP_DFType_private;
   fme_fpmon.ccip_fme_fpmon_dflhdr.next_DFH_offset =0x1000;
   fme_fpmon.ccip_fme_fpmon_dflhdr.Feature_rev =0;
   fme_fpmon.ccip_fme_fpmon_dflhdr.Feature_ID= CCIP_FME_DFLID_GPERF;
   fme_fpmon.ccip_fme_fpmon_dflhdr.eol =0x0;
   offset = 0;
   write_ccip_csr64(ptr,offset,fme_fpmon.ccip_fme_fpmon_dflhdr.csr);

   // Cache control
   fme_fpmon.ccip_fpmon_ch_ctl.cache_event =0x6;
   fme_fpmon.ccip_fpmon_ch_ctl.freeze =0x1;
   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,fme_fpmon.ccip_fpmon_ch_ctl.csr);

   // Cache Couter port 1
   fme_fpmon.ccip_fpmon_ch_ctr_0.cache_counter =0x99;
   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,fme_fpmon.ccip_fpmon_ch_ctr_0.csr);

   // Cache Couter port 2
   fme_fpmon.ccip_fpmon_ch_ctr_1.cache_counter =0x88;
   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,fme_fpmon.ccip_fpmon_ch_ctr_1.csr);

   // fabic control
   fme_fpmon.ccip_fpmon_fab_ctl.ccip_port_filter =0x1;
   fme_fpmon.ccip_fpmon_fab_ctl.port_id =0x1;
   fme_fpmon.ccip_fpmon_fab_ctl.fabric_evt_code =0x5;
   fme_fpmon.ccip_fpmon_fab_ctl.freeze =0x1;
   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,fme_fpmon.ccip_fpmon_fab_ctl.csr);

   // fabic ctr
   fme_fpmon.ccip_fpmon_fab_ctr.fabric_counter= 0x77;
   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,fme_fpmon.ccip_fpmon_fab_ctr.csr);

   // clock
   fme_fpmon.ccip_fpmon_clk_ctrs.afu_interf_clock =0x66;
   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,fme_fpmon.ccip_fpmon_clk_ctrs.csr);

   // FME Global Error header

   ptr = ptr+ fme_fpmon.ccip_fme_fpmon_dflhdr.next_DFH_offset;
   fme_gerror.ccip_gerror_dflhdr.Type =CCIP_DFType_private;
   fme_gerror.ccip_gerror_dflhdr.next_DFH_offset =0x1000;
   fme_gerror.ccip_gerror_dflhdr.Feature_rev =0;
   fme_gerror.ccip_gerror_dflhdr.Feature_ID= CCIP_FME_DFLID_GERR;
   fme_gerror.ccip_gerror_dflhdr.eol =0x0;
   offset = 0;
   write_ccip_csr64(ptr,offset,fme_gerror.ccip_gerror_dflhdr.csr);


   // FME error mask
   fme_gerror.fme_err_mask.fabric_err =0x1;
   fme_gerror.fme_err_mask.fabFifo_uoflow =0x1;
   fme_gerror.fme_err_mask.pcie0_poison_detected =0x1;
   fme_gerror.fme_err_mask.pcie1_poison_detected =0x1;
   fme_gerror.fme_err_mask.iommu_parity_error =0x1;
   fme_gerror.fme_err_mask.afuerr_access_mismatch =0x1;


   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,fme_gerror.fme_err_mask.csr);

   // FME error
   fme_gerror.fme_err.fabric_err =0x1;
   fme_gerror.fme_err.fabFifo_uoflow =0x1;
   fme_gerror.fme_err.pcie0_poison_detected =0x1;
   fme_gerror.fme_err.pcie1_poison_detected =0x1;
   fme_gerror.fme_err.iommu_parity_error =0x1;
   fme_gerror.fme_err.afuerr_access_mismatch =0x1;

   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,fme_gerror.fme_err.csr);

   //PCIE0 error mask
   fme_gerror.pcie0_err_mask.formattype_err = 0x1;
   fme_gerror.pcie0_err_mask.MWAddr_err = 0x1;
   fme_gerror.pcie0_err_mask.MWAddrLength_err = 0x1;
   fme_gerror.pcie0_err_mask.MRAddr_err = 0x1;
   fme_gerror.pcie0_err_mask.MRAddrLength_err = 0x1;
   fme_gerror.pcie0_err_mask.cpl_tag_err = 0x1;
   fme_gerror.pcie0_err_mask.cpl_status_err = 0x1;
   fme_gerror.pcie0_err_mask.cpl_timeout_err = 0x1;

   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,fme_gerror.pcie0_err_mask.csr);


   //PCIE0 error
   fme_gerror.pcie0_err.formattype_err = 0x1;
   fme_gerror.pcie0_err.MWAddr_err = 0x1;
   fme_gerror.pcie0_err.MWAddrLength_err = 0x1;
   fme_gerror.pcie0_err.MRAddr_err = 0x1;
   fme_gerror.pcie0_err.MRAddrLength_err = 0x1;
   fme_gerror.pcie0_err.cpl_tag_err = 0x1;
   fme_gerror.pcie0_err.cpl_status_err = 0x1;
   fme_gerror.pcie0_err.cpl_timeout_err = 0x1;

   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,fme_gerror.pcie0_err.csr);

   //PCIE1 error mask
   fme_gerror.pcie1_err_mask.formattype_err = 0x1;
   fme_gerror.pcie1_err_mask.MWAddr_err = 0x1;
   fme_gerror.pcie1_err_mask.MWAddrLength_err = 0x1;
   fme_gerror.pcie1_err_mask.MRAddr_err = 0x1;
   fme_gerror.pcie1_err_mask.MRAddrLength_err = 0x1;
   fme_gerror.pcie1_err_mask.cpl_tag_err = 0x1;
   fme_gerror.pcie1_err_mask.cpl_status_err = 0x1;
   fme_gerror.pcie1_err_mask.cpl_timeout_err = 0x1;

   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,fme_gerror.pcie1_err_mask.csr);


   //PCIE1 error
   fme_gerror.pcie1_err.formattype_err = 0x1;
   fme_gerror.pcie1_err.MWAddr_err = 0x1;
   fme_gerror.pcie1_err.MWAddrLength_err = 0x1;
   fme_gerror.pcie1_err.MRAddr_err = 0x1;
   fme_gerror.pcie1_err.MRAddrLength_err = 0x1;
   fme_gerror.pcie1_err.cpl_tag_err = 0x1;
   fme_gerror.pcie1_err.cpl_status_err = 0x1;
   fme_gerror.pcie1_err.cpl_timeout_err = 0x1;

   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,fme_gerror.pcie1_err.csr);


   // FME First error

   firsterror.fabric_err =0x1;
   firsterror.fabFifo_uoflow =0x1;
   firsterror.pcie0_poison_detected =0x1;

   fme_gerror.fme_first_err.csr = firsterror.csr;
   fme_gerror.fme_first_err.errReg_id =0x0;
   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,fme_gerror.fme_first_err.csr);

   // FME Next error
   pcie1error.MWAddr_err = 0x1;
   pcie1error.MWAddrLength_err = 0x1;
   pcie1error.MRAddr_err = 0x1;
   pcie1error.MRAddrLength_err = 0x1;
   pcie1error.cpl_tag_err = 0x1;
   pcie1error.cpl_status_err = 0x1;

   fme_gerror.fme_next_err.csr = pcie1error.csr;
   fme_gerror.fme_next_err.errReg_id =0x2;
   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,fme_gerror.fme_next_err.csr);



   // RAS Error maks
   fme_gerror.ras_gerr_mask.temp_trash_ap1 = 0x1;
   fme_gerror.ras_gerr_mask.temp_trash_ap2 = 0x1;
   fme_gerror.ras_gerr_mask.pcie_error = 0x1;
   fme_gerror.ras_gerr_mask.afufatal_error = 0x1;
   fme_gerror.ras_gerr_mask.prochot_error = 0x1;
   fme_gerror.ras_gerr_mask.afu_access_mismatch = 0x1;
   fme_gerror.ras_gerr_mask.injected_warn_error = 0x1;
   fme_gerror.ras_gerr_mask.pcie_posion_error = 0x1;
   fme_gerror.ras_gerr_mask.gb_crc_err = 0x1;
   fme_gerror.ras_gerr_mask.temp_trash_ap6 = 0x1;
   fme_gerror.ras_gerr_mask.power_trash_ap1 = 0x1;
   fme_gerror.ras_gerr_mask.power_trash_ap2 = 0x1;
   fme_gerror.ras_gerr_mask.mbp_error = 0x1;

   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,fme_gerror.ras_gerr_mask.csr);

   // RAS Error
   fme_gerror.ras_gerr.temp_trash_ap1 = 0x1;
   fme_gerror.ras_gerr.temp_trash_ap2 = 0x1;
   fme_gerror.ras_gerr.pcie_error = 0x1;
   fme_gerror.ras_gerr.afufatal_error = 0x1;
   fme_gerror.ras_gerr.prochot_error = 0x1;
   fme_gerror.ras_gerr.afu_access_mismatch = 0x1;
   fme_gerror.ras_gerr.injected_warn_error = 0x1;
   fme_gerror.ras_gerr.pcie_posion_error = 0x1;
   fme_gerror.ras_gerr.gb_crc_err = 0x1;
   fme_gerror.ras_gerr.temp_trash_ap6 = 0x1;
   fme_gerror.ras_gerr.power_trash_ap1 = 0x1;
   fme_gerror.ras_gerr.power_trash_ap2 = 0x1;
   fme_gerror.ras_gerr.mbp_error = 0x1;
;

   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,fme_gerror.ras_gerr.csr);


   // RAS Error Mask
   fme_gerror.ras_berror_mask.ktilink_fatal_err = 0x1;
   fme_gerror.ras_berror_mask.tagcch_fatal_err = 0x1;
   fme_gerror.ras_berror_mask.cci_fatal_err = 0x1;
   fme_gerror.ras_berror_mask.ktiprpto_fatal_err = 0x1;
   fme_gerror.ras_berror_mask.dma_fatal_err = 0x1;
   fme_gerror.ras_berror_mask.iommu_catast_err = 0x1;
   fme_gerror.ras_berror_mask.crc_catast_err = 0x1;
   fme_gerror.ras_berror_mask.therm_catast_err = 0x1;
   fme_gerror.ras_berror_mask.injected_fatal_err = 0x1;


   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,fme_gerror.ras_berror_mask.csr);


   // RAS Error
   fme_gerror.ras_berror.ktilink_fatal_err = 0x1;
   fme_gerror.ras_berror.tagcch_fatal_err = 0x1;
   fme_gerror.ras_berror.cci_fatal_err = 0x1;
   fme_gerror.ras_berror.ktiprpto_fatal_err = 0x1;
   fme_gerror.ras_berror.dma_fatal_err = 0x1;
   fme_gerror.ras_berror.iommu_catast_err = 0x1;
   fme_gerror.ras_berror.crc_catast_err = 0x1;
   fme_gerror.ras_berror.therm_catast_err = 0x1;
   fme_gerror.ras_berror.injected_fatal_err = 0x1;

   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,fme_gerror.ras_berror.csr);

   // RAS warning Error Mask
   fme_gerror.ras_warnerror_mask.event_warn_err= 0x1;

   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,fme_gerror.ras_warnerror_mask.csr);


   // RAS warning Error
   fme_gerror.ras_warnerror.event_warn_err= 0x1;

   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,fme_gerror.ras_warnerror.csr);


   // FME PR
   ptr = ptr+ fme_gerror.ccip_gerror_dflhdr.next_DFH_offset;
   fme_pr.ccip_pr_dflhdr.Type =CCIP_DFType_private;
   fme_pr.ccip_pr_dflhdr.next_DFH_offset =0x0;
   fme_pr.ccip_pr_dflhdr.Feature_rev =0;
   fme_pr.ccip_pr_dflhdr.Feature_ID= CCIP_FME_DFLID_PR;
   fme_pr.ccip_pr_dflhdr.eol =0x1;
   offset = 0;
   write_ccip_csr64(ptr,offset,fme_pr.ccip_pr_dflhdr.csr);

   // PR control
   fme_pr.ccip_fme_pr_control.pr_push_complete =0x1;
   fme_pr.ccip_fme_pr_control.pr_start_req =0x1;
   fme_pr.ccip_fme_pr_control.pr_regionid =0x1;
   fme_pr.ccip_fme_pr_control.enable_pr_port_access =0x1;
   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,fme_pr.ccip_fme_pr_control.csr);

   // PR status
   fme_pr.ccip_fme_pr_status.pr_host_status =0x5;
   fme_pr.ccip_fme_pr_status.pr_contoller_status =0x1;
   fme_pr.ccip_fme_pr_status.pr_status =0x1;
   fme_pr.ccip_fme_pr_status.pr_credit =0x55;

   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,fme_pr.ccip_fme_pr_status.csr);

   // PR Data
   fme_pr.ccip_fme_pr_data.pr_data_raw =0x123456;
   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,fme_pr.ccip_fme_pr_data.csr);

    fme_pr.ccip_fme_pr_err.PR_operation_err =0x1;
   fme_pr.ccip_fme_pr_err.PR_CRC_err =0x1;
   fme_pr.ccip_fme_pr_err.PR_bitstream_err =0x1;
   fme_pr.ccip_fme_pr_err.PR_IP_err =0x1;
   fme_pr.ccip_fme_pr_err.PR_FIFIO_err =0x1;
   fme_pr.ccip_fme_pr_err.PR_timeout_err =0x1;

   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,fme_pr.ccip_fme_pr_err.csr);

   PINFO(" ccip_sim_wrt_fme_mmio end\n");
   return 0;
}

int  ccip_sim_wrt_port_mmio(btVirtAddr pkvp_fme_mmio)
{
   struct CCIP_PORT_HDR          port_hdr;
   struct CCIP_PORT_DFL_ERR      port_err;
   struct CCIP_PORT_DFL_UMSG     port_umsg;
   struct CCIP_PORT_DFL_PR       port_pr;
   struct CCIP_PORT_DFL_STAP     port_stap;
   struct CCIP_AFU_Header        afu_hdr;

   btVirtAddr           afuptr;
   btUnsigned64bitInt   afu_id_offset;

   btVirtAddr           ptr = pkvp_fme_mmio;


   btUnsigned64bitInt offset =0;

   PINFO(" ccip_sim_wrt_port_mmio ENTER\n");

   afuptr = ptr;           // First AFU offset relative to First Port

   // Port header
   port_hdr.ccip_port_dfh.Type = CCIP_DFType_afu;
   port_hdr.ccip_port_dfh.next_DFH_offset =0x1000;
   port_hdr.ccip_port_dfh.Feature_rev =0;
   port_hdr.ccip_port_dfh.Feature_ID =1;
   port_hdr.ccip_port_dfh.eol =0x0;
   write_ccip_csr64(ptr,offset,port_hdr.ccip_port_dfh.csr);

   //port afu id low
   port_hdr.ccip_port_afuidl.afu_id_l = CCIP_PORT_GUIDL;
   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,port_hdr.ccip_port_afuidl.csr);

   //port afu id high
   port_hdr.ccip_port_afuidh.afu_id_h = CCIP_PORT_GUIDH;
   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,port_hdr.ccip_port_afuidh.csr);

   //port next afu offset
   port_hdr.ccip_port_next_afu.csr = 0;				// Make sure the whole things is zero
   port_hdr.ccip_port_next_afu.afu_id_offset=0x0;   // Temporary value. Will be filled in when we know the end
   offset = offset + OFFSET;
   afu_id_offset = offset;       // Save this so it can be populated at the end
   write_ccip_csr64(ptr,offset,port_hdr.ccip_port_next_afu.csr) ;

   //rsvd
   port_hdr.rsvd_porthdr =0;
   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,port_hdr.rsvd_porthdr);

   // Port scratchpad csr
   port_hdr.ccip_port_scratchpad.scratch_pad = 0x1234568;
   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset, port_hdr.ccip_port_scratchpad.csr);

   // Port capability csr
   port_hdr.ccip_port_capability.interrupts =0x1;
   port_hdr.ccip_port_capability.mmio_size = 0x20;
   port_hdr.ccip_port_capability.port_id =0x1;
   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset, port_hdr.ccip_port_capability.csr);

   // Port control csr
   port_hdr.ccip_port_control.ccip_outstanding_request =0x1;
   port_hdr.ccip_port_control.afu_latny_rep =0x1;
   port_hdr.ccip_port_control.port_sftreset_control =0x1;
   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset, port_hdr.ccip_port_control.csr);


   //Port status
   port_hdr.ccip_port_status.afu_pwr_state =0x6;
   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,port_hdr.ccip_port_status.csr);

   // Port Error

   // port error header
   ptr = ptr+ port_hdr.ccip_port_dfh.next_DFH_offset;
   offset =0;
   port_err.ccip_port_err_dflhdr.Type =CCIP_DFType_private;
   port_err.ccip_port_err_dflhdr.next_DFH_offset =0x1000;
   port_err.ccip_port_err_dflhdr.Feature_rev =0;
   port_err.ccip_port_err_dflhdr.Feature_ID= CCIP_PORT_DFLID_ERROR;
   port_err.ccip_port_err_dflhdr.eol =0x0;
   write_ccip_csr64(ptr,offset, port_err.ccip_port_err_dflhdr.csr);


   //Port Error Mask
   offset = offset + OFFSET;
   port_err.ccip_port_error_mask.tx_ch0_overflow =0x0;
   port_err.ccip_port_error_mask.tx_ch0_invalidreq =0x0;
   port_err.ccip_port_error_mask.tx_ch0_req_cl_len3 =0x0;
   port_err.ccip_port_error_mask.tx_ch0_req_cl_len2 =0x0;
   port_err.ccip_port_error_mask.tx_ch0_req_cl_len4 =0x0;
   port_err.ccip_port_error_mask.rsvd =0x0;

   port_err.ccip_port_error_mask.tx_ch1_overflow =0x0;
   port_err.ccip_port_error_mask.tx_ch1_invalidreq =0x0;
   port_err.ccip_port_error_mask.tx_ch1_req_cl_len3 =0x0;
   port_err.ccip_port_error_mask.tx_ch1_req_cl_len2 =0x0;
   port_err.ccip_port_error_mask.tx_ch1_req_cl_len4 =0x0;

   port_err.ccip_port_error_mask.tx_ch1_insuff_datapayload =0x0;
   port_err.ccip_port_error_mask.tx_ch1_datapayload_overrun =0x0;
   port_err.ccip_port_error_mask.tx_ch1_incorr_addr =0x0;
   port_err.ccip_port_error_mask.tx_ch1_sop_detcted =0x0;
   port_err.ccip_port_error_mask.tx_ch1_atomic_req =0x0;
   port_err.ccip_port_error_mask.rsvd1 =0x0;

   port_err.ccip_port_error_mask.mmioread_timeout =0x0;
   port_err.ccip_port_error_mask.tx_ch2_fifo_overflow =0x0;
   port_err.ccip_port_error_mask.rsvd2 =0x0;
   port_err.ccip_port_error_mask.num_pending_req_overflow =0x0;
   port_err.ccip_port_error_mask.rsvd3 =0x0;
   write_ccip_csr64(ptr,offset, port_err.ccip_port_error_mask.csr);

     //Port Error Mask
   offset = offset + OFFSET;
   port_err.ccip_port_error.tx_ch0_overflow =0x1;
   port_err.ccip_port_error.tx_ch0_invalidreq =0x1;
   port_err.ccip_port_error.tx_ch0_req_cl_len3 =0x1;
   port_err.ccip_port_error.tx_ch0_req_cl_len2 =0x1;
   port_err.ccip_port_error.tx_ch0_req_cl_len4 =0x1;


   port_err.ccip_port_error.tx_ch1_overflow =0x1;
   port_err.ccip_port_error.tx_ch1_invalidreq =0x1;
   port_err.ccip_port_error.tx_ch1_req_cl_len3 =0x1;
   port_err.ccip_port_error.tx_ch1_req_cl_len2 =0x1;
   port_err.ccip_port_error.tx_ch1_req_cl_len4 =0x1;

   port_err.ccip_port_error.tx_ch1_insuff_datapayload =0x1;
   port_err.ccip_port_error.tx_ch1_datapayload_overrun =0x1;
   port_err.ccip_port_error.tx_ch1_incorr_addr =0x1;
   port_err.ccip_port_error.tx_ch1_sop_detcted =0x1;
   port_err.ccip_port_error.tx_ch1_atomic_req =0x1;


   port_err.ccip_port_error.mmioread_timeout =0x1;
   port_err.ccip_port_error.tx_ch2_fifo_overflow =0x1;

   port_err.ccip_port_error.unexp_mmio_resp =0x1;
   port_err.ccip_port_error.num_pending_req_overflow =0x1;

   port_err.ccip_port_error.llpr_smrr_err = 0x1;
   port_err.ccip_port_error.llpr_smrr2_err = 0x1;
   port_err.ccip_port_error.llpr_mesg_err = 0x1;
   port_err.ccip_port_error.genport_range_err = 0x1;
   port_err.ccip_port_error.legrange_low_err = 0x1;
   port_err.ccip_port_error.legrange_hight_err = 0x1;
   port_err.ccip_port_error.vgmem_range_err = 0x1;
   port_err.ccip_port_error.page_fault_err = 0x1;
   port_err.ccip_port_error.pmr_err = 0x1;
   port_err.ccip_port_error.ap6_event = 0x1;
   port_err.ccip_port_error.vfflr_accesseror = 0x1;

   write_ccip_csr64(ptr,offset, port_err.ccip_port_error.csr);

   //Port first error
   offset = offset + OFFSET;
   port_err.ccip_port_first_error.tx_ch0_overflow =0x1;
   port_err.ccip_port_first_error.tx_ch0_invalidreq =0x1;
   port_err.ccip_port_first_error.tx_ch0_req_cl_len3 =0x1;
   port_err.ccip_port_first_error.tx_ch0_req_cl_len2 =0x1;
   port_err.ccip_port_first_error.tx_ch0_req_cl_len4 =0x1;


   port_err.ccip_port_first_error.tx_ch1_overflow =0x1;
   port_err.ccip_port_first_error.tx_ch1_invalidreq =0x1;
   port_err.ccip_port_first_error.tx_ch1_req_cl_len3 =0x1;
   port_err.ccip_port_first_error.tx_ch1_req_cl_len2 =0x1;
   port_err.ccip_port_first_error.tx_ch1_req_cl_len4 =0x1;

   port_err.ccip_port_first_error.tx_ch1_insuff_datapayload =0x1;
   port_err.ccip_port_first_error.tx_ch1_datapayload_overrun =0x1;
   port_err.ccip_port_first_error.tx_ch1_incorr_addr =0x1;
   port_err.ccip_port_first_error.tx_ch1_sop_detcted =0x1;
   port_err.ccip_port_first_error.tx_ch1_atomic_req =0x1;


   port_err.ccip_port_first_error.mmioread_timeout =0x1;
   port_err.ccip_port_first_error.tx_ch2_fifo_overflow =0x1;
   port_err.ccip_port_first_error.unexp_mmio_resp =0x1;
   port_err.ccip_port_first_error.num_pending_req_overflow =0x1;

   port_err.ccip_port_first_error.llpr_smrr_err = 0x1;
   port_err.ccip_port_first_error.llpr_smrr2_err = 0x1;
   port_err.ccip_port_first_error.llpr_mesg_err = 0x1;
   port_err.ccip_port_first_error.genport_range_err = 0x1;
   port_err.ccip_port_first_error.legrange_low_err = 0x1;
   port_err.ccip_port_first_error.legrange_hight_err = 0x1;
   port_err.ccip_port_first_error.vgmem_range_err = 0x1;
   port_err.ccip_port_first_error.page_fault_err = 0x1;
   port_err.ccip_port_first_error.pmr_err = 0x1;
   port_err.ccip_port_first_error.ap6_event = 0x1;
   port_err.ccip_port_first_error.vfflr_accesseror = 0x1;

   write_ccip_csr64(ptr,offset, port_err.ccip_port_first_error.csr);

   // port malformed request csr
   offset = offset + OFFSET;
   port_err.ccip_port_malformed_req_0.malfrd_req_lsb=0x123457;
   write_ccip_csr64(ptr,offset,port_err.ccip_port_malformed_req_0.csr);

   // port malformed request csr
   offset = offset + OFFSET;
   port_err.ccip_port_malformed_req_1.malfrd_req_msb=0x7654321;
   write_ccip_csr64(ptr,offset, port_err.ccip_port_malformed_req_1.csr);


   // USMG DFH
   // umsg header
   ptr = ptr+ port_err.ccip_port_err_dflhdr.next_DFH_offset;
   offset =0;
   port_umsg.ccip_port_umsg_dflhdr.Type =CCIP_DFType_private;
   port_umsg.ccip_port_umsg_dflhdr.next_DFH_offset =0x1000;
   port_umsg.ccip_port_umsg_dflhdr.Feature_rev =0;
   port_umsg.ccip_port_umsg_dflhdr.Feature_ID= CCIP_PORT_DFLID_USMG;
   port_umsg.ccip_port_umsg_dflhdr.eol =0x0;
   write_ccip_csr64(ptr,offset,port_umsg.ccip_port_umsg_dflhdr.csr);


   // USMG capability
   offset = offset + OFFSET;
   port_umsg.ccip_umsg_capability.no_umsg_alloc_port =0x8;
   port_umsg.ccip_umsg_capability.status_umsg_engine =0x0;
   port_umsg.ccip_umsg_capability.umsg_init_status =0x1;
   write_ccip_csr64(ptr,offset,port_umsg.ccip_umsg_capability.csr);

   //USMG base address
   offset = offset + OFFSET;
   port_umsg.ccip_umsg_base_address.umsg_base_address =0x0;
   write_ccip_csr64(ptr,offset,port_umsg.ccip_umsg_base_address.csr);

   // USMG Mode
   offset = offset + OFFSET;
   port_umsg.ccip_umsg_mode.umsg_hit =0x00001001;
   write_ccip_csr64(ptr,offset,port_umsg.ccip_umsg_mode.csr);

   // PR CSR Header

   ptr = ptr+ port_umsg.ccip_port_umsg_dflhdr.next_DFH_offset;
   offset =0;

   port_pr.ccip_port_pr_dflhdr.Type =CCIP_DFType_private;
   port_pr.ccip_port_pr_dflhdr.next_DFH_offset =0x1000;
   port_pr.ccip_port_pr_dflhdr.Feature_rev =0;
   port_pr.ccip_port_pr_dflhdr.Feature_ID= CCIP_PORT_DFLID_PR;
   port_pr.ccip_port_pr_dflhdr.eol =0x0;
   write_ccip_csr64(ptr,offset,port_pr.ccip_port_pr_dflhdr.csr);

   // PR Control CSR
   offset = offset + OFFSET;
   port_pr.ccip_port_pr_control.pr_start_req =0x1;
   write_ccip_csr64(ptr,offset,port_pr.ccip_port_pr_control.csr);

   // PR STATUS CSR
   offset = offset + OFFSET;
   port_pr.ccip_port_pr_status.pr_status =0x7;
   port_pr.ccip_port_pr_status.pr_mega_fstatus=0x5;
   port_pr.ccip_port_pr_status.pr_data_ovrferr =0x1;
   port_pr.ccip_port_pr_status.pr_engine_error =0x1;
   port_pr.ccip_port_pr_status.pr_timeout_error =0x1;
   port_pr.ccip_port_pr_status.pr_access =0x1;
   port_pr.ccip_port_pr_status.pr_acces_grant =0x1;
   port_pr.ccip_port_pr_status.pr_credit =0x1;
   write_ccip_csr64(ptr,offset,port_pr.ccip_port_pr_status.csr);


   // PR DATA CSR
   offset = offset + OFFSET;
   port_pr.ccip_port_pr_data.pr_data_raw =0xabcd;
   write_ccip_csr64(ptr,offset,port_pr.ccip_port_pr_data.csr);

   // PR power Budget  CSR
   offset = offset + OFFSET;
   port_pr.ccip_port_pr_pbudget.pwr_format =0x4321;
   write_ccip_csr64(ptr,offset,port_pr.ccip_port_pr_pbudget.csr);

   offset = offset + OFFSET;
   port_pr.ccip_usr_clk_freq.rsvd =0;
   write_ccip_csr64(ptr,offset,port_pr.ccip_usr_clk_freq.csr);


   // Signal Tap DFH
   ptr = ptr+ port_pr.ccip_port_pr_dflhdr.next_DFH_offset;
   offset =0;

   port_stap.ccip_port_stap_dflhdr.Type =CCIP_DFType_private;
   port_stap.ccip_port_stap_dflhdr.next_DFH_offset =0x0;
   port_stap.ccip_port_stap_dflhdr.Feature_rev =0;
   port_stap.ccip_port_stap_dflhdr.Feature_ID= CCIP_PORT_DFLID_STP;
   port_stap.ccip_port_stap_dflhdr.eol =0x1;
   write_ccip_csr64(ptr,offset,port_stap.ccip_port_stap_dflhdr.csr);


   // SiganlTap CSR
   offset = offset + OFFSET;
   port_stap.ccip_port_stap.rsvd =0;
   write_ccip_csr64(ptr,offset,port_stap.ccip_port_stap.csr);

   // First AFU offset is the next free location (ptr + OFFSET) - the base address of Port HDR afuptr
   offset = (ptr + OFFSET) - afuptr;

   // User AFU - re write the value
   port_hdr.ccip_port_next_afu.afu_id_offset=offset;
   write_ccip_csr64(afuptr, afu_id_offset, port_hdr.ccip_port_next_afu.csr);

    ptr = afuptr + port_hdr.ccip_port_next_afu.csr;

   PINFO("AFU offset is %llx beyond PORT header at %p = %p\n", offset, afuptr, ptr );

   offset = 0;

   // Port header
   afu_hdr.ccip_dfh.Type = CCIP_DFType_afu;
   afu_hdr.ccip_dfh.next_DFH_offset =0x0;
   afu_hdr.ccip_dfh.Feature_rev =0;
   afu_hdr.ccip_dfh.Feature_ID =1;
   write_ccip_csr64(ptr,offset,afu_hdr.ccip_dfh.csr);

   //port afu id low
   afu_hdr.ccip_afu_id_l.afu_id_l = CCI_SIM_AFUIDL;
   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,afu_hdr.ccip_afu_id_l.csr);

   //port afu id high
   afu_hdr.ccip_afu_id_h.afu_id_h = CCI_SIM_AFUIDH;
   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,afu_hdr.ccip_afu_id_h.csr);

   //port next afu offset
   afu_hdr.ccip_next_afu.afu_id_offset = 0x0;        // Temporary value
   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,afu_hdr.ccip_next_afu.csr);

   PINFO(" ccip_sim_wrt_port_mmio EXIT \n");


   return 0;
}


int print_sim_fme_device(struct fme_device *pfme_dev)
{
   int res =0;
   int i =0;


   if (NULL == pfme_dev ) {
      PERR("Unable to allocate system memory for pfme_dev object\n");
      res = -1;
      goto EROR;
   }

   PDEBUG( "*****************************************************************\n");
   PDEBUG( "*****************************************************************\n");
   PDEBUG( "****************         FME MMIO CONTENT        ****************\n");
   PDEBUG( "*****************************************************************\n");
   PDEBUG( "*****************************************************************\n");

   if(pfme_dev->m_pHDR)
   {
      PDEBUG( "FME Header START \n \n");

      PDEBUG( "dfh.Type= %x \n",pfme_dev->m_pHDR->dfh.Type);
      PDEBUG( "dfh.Feature_ID= %x \n",pfme_dev->m_pHDR->dfh.Feature_ID);
      PDEBUG( "dfh.Feature_rev= %x \n",pfme_dev->m_pHDR->dfh.Feature_rev);
      PDEBUG( "dfh.next_DFH_offset= %x \n",pfme_dev->m_pHDR->dfh.next_DFH_offset);

      PDEBUG( "afu_id_l.afu_id_l= %lx \n",(long unsigned int)pfme_dev->m_pHDR->afu_id_l.afu_id_l);
      PDEBUG( "afu_id_h.afu_id_h= %lx \n",(long unsigned int) pfme_dev->m_pHDR->afu_id_h.afu_id_h);

      PDEBUG( "next_afu.afu_id_offset= %x \n",pfme_dev->m_pHDR->next_afu.afu_id_offset);

      PDEBUG( "scratch_pad= %lx \n",(long unsigned int)pfme_dev->m_pHDR->scratchpad.scratch_pad);


      PDEBUG( "fabric_verid= %x \n",pfme_dev->m_pHDR->fab_capability.fabric_verid);
      PDEBUG( "socket_id= %x \n",pfme_dev->m_pHDR->fab_capability.socket_id);
      PDEBUG( "pci0_link_avile= %x \n",pfme_dev->m_pHDR->fab_capability.pci0_link_avile);
      PDEBUG( "pci1_link_avile= %x \n",pfme_dev->m_pHDR->fab_capability.pci1_link_avile);
      PDEBUG( "qpi_link_avile= %x \n",pfme_dev->m_pHDR->fab_capability.qpi_link_avile);
      PDEBUG( "iommu_support= %x \n",pfme_dev->m_pHDR->fab_capability.iommu_support);

      PDEBUG( "address_width_bits= %x \n",pfme_dev->m_pHDR->fab_capability.address_width_bits);
      PDEBUG( "cache_size= %x \n",pfme_dev->m_pHDR->fab_capability.cache_size);
      PDEBUG( "cache_assoc= %x \n",pfme_dev->m_pHDR->fab_capability.cache_assoc);
      PDEBUG( "lock_bit= %x \n",pfme_dev->m_pHDR->fab_capability.lock_bit);

      PDEBUG( "FME Header END \n \n");

   }

   // Print list of ports
   for ( i=0 ; i<NUM_ELEMENTS(pfme_dev->m_pHDR->port_offsets) ; i++)
   {
      PDEBUG( "i = %d \n",i);
      if(0 != pfme_dev->m_pHDR->port_offsets[i].port_imp)
      {
         PDEBUG( "PORT count = %d \n",i);
         PDEBUG( "port_imp = %x \n",pfme_dev->m_pHDR->port_offsets[i].port_imp);
         PDEBUG( "port_arbit_poly = %x \n",pfme_dev->m_pHDR->port_offsets[i].afu_access_control);
         PDEBUG( "port_bar = %x \n",pfme_dev->m_pHDR->port_offsets[i].port_bar);
         PDEBUG( "port_offset = %x \n",pfme_dev->m_pHDR->port_offsets[i].port_offset);

      }
   }

   if(pfme_dev->m_pThermmgmt)
   {
      PDEBUG( "FME Temp Feature  START \n \n");

      PDEBUG( "Feature_ID = %x \n",pfme_dev->m_pThermmgmt->ccip_fme_tmp_dflhdr.Feature_ID);
      PDEBUG( "Feature_rev = %x \n",pfme_dev->m_pThermmgmt->ccip_fme_tmp_dflhdr.Feature_rev);
      PDEBUG( "next_DFH_offset = %x \n",pfme_dev->m_pThermmgmt->ccip_fme_tmp_dflhdr.next_DFH_offset);
      PDEBUG( "Type = %x \n",pfme_dev->m_pThermmgmt->ccip_fme_tmp_dflhdr.Type);
      PDEBUG( "End of List = %x \n",pfme_dev->m_pThermmgmt->ccip_fme_tmp_dflhdr.eol);


      PDEBUG( "therm_trip_thshold = %x \n",pfme_dev->m_pThermmgmt->ccip_tmp_threshold.therm_trip_thshold);
      PDEBUG( "therm_trip_thshold_status = %x \n",pfme_dev->m_pThermmgmt->ccip_tmp_threshold.therm_trip_thshold_status);
      PDEBUG( "thshold1_status = %x \n",pfme_dev->m_pThermmgmt->ccip_tmp_threshold.thshold1_status);
      PDEBUG( "thshold2_status = %x \n",pfme_dev->m_pThermmgmt->ccip_tmp_threshold.thshold2_status);
      PDEBUG( "thshold_policy = %x \n",pfme_dev->m_pThermmgmt->ccip_tmp_threshold.thshold_policy);

      PDEBUG( "tmp_thshold1 = %x \n",pfme_dev->m_pThermmgmt->ccip_tmp_threshold.tmp_thshold1);
      PDEBUG( "tmp_thshold1_status = %x \n",pfme_dev->m_pThermmgmt->ccip_tmp_threshold.tmp_thshold1_status);
      PDEBUG( "tmp_thshold2 = %x \n",pfme_dev->m_pThermmgmt->ccip_tmp_threshold.tmp_thshold2);
      PDEBUG( "tmp_thshold2_status = %x \n",pfme_dev->m_pThermmgmt->ccip_tmp_threshold.tmp_thshold2_status);

      PDEBUG( "dbg_mode = %x \n",pfme_dev->m_pThermmgmt->ccip_tmp_rdssensor_fm1.dbg_mode);
      PDEBUG( "tmp_reading = %d \n",pfme_dev->m_pThermmgmt->ccip_tmp_rdssensor_fm1.tmp_reading);
      PDEBUG( "tmp_reading_valid = %d \n",pfme_dev->m_pThermmgmt->ccip_tmp_rdssensor_fm1.tmp_reading_valid);
      PDEBUG( "tmp_reading_seq_num = %d \n",pfme_dev->m_pThermmgmt->ccip_tmp_rdssensor_fm1.tmp_reading_seq_num);

      PDEBUG( "FM2 rsvd = %lu \n",(long unsigned int)pfme_dev->m_pThermmgmt->ccip_tmp_rdssensor_fm2.rsvd);

      PDEBUG( "FME Temp Feature  END \n \n");
   }

   if(pfme_dev->m_pPowermgmt)
   {

      PDEBUG( "FME Power Feature  START \n \n");

      PDEBUG( "Feature_ID = %x \n",pfme_dev->m_pPowermgmt->ccip_fme_pm_dflhdr.Feature_ID);
      PDEBUG( "Feature_rev = %x \n",pfme_dev->m_pPowermgmt->ccip_fme_pm_dflhdr.Feature_rev);
      PDEBUG( "Type = %x \n",pfme_dev->m_pPowermgmt->ccip_fme_pm_dflhdr.Type);
      PDEBUG( "next_DFH_offset = %x \n",pfme_dev->m_pPowermgmt->ccip_fme_pm_dflhdr.next_DFH_offset);
      PDEBUG( "End of List = %x \n",pfme_dev->m_pPowermgmt->ccip_fme_pm_dflhdr.eol);


      PDEBUG( "pwr_consumed = %x \n",pfme_dev->m_pPowermgmt->pm_status.pwr_consumed);
      PDEBUG( "fpga_latency_report = %d \n",pfme_dev->m_pPowermgmt->pm_status.fpga_latency_report);


      PDEBUG( "FME Power Feature  END \n \n");

   }

   if(pfme_dev->m_pPerf)
   {

      PDEBUG( "FME   Global Performance START \n \n");

      PDEBUG( "Feature_ID = %x \n",pfme_dev->m_pPerf->ccip_fme_fpmon_dflhdr.Feature_ID);
      PDEBUG( "Feature_rev = %x \n",pfme_dev->m_pPerf->ccip_fme_fpmon_dflhdr.Feature_rev);
      PDEBUG( "Type = %x \n",pfme_dev->m_pPerf->ccip_fme_fpmon_dflhdr.Type);
      PDEBUG( "next_DFH_offset = %x \n",pfme_dev->m_pPerf->ccip_fme_fpmon_dflhdr.next_DFH_offset);
      PDEBUG( "End of List = %x \n",pfme_dev->m_pPerf->ccip_fme_fpmon_dflhdr.eol);

      PDEBUG( "cache_event = %x \n",pfme_dev->m_pPerf->ccip_fpmon_ch_ctl.cache_event);
      PDEBUG( "freeze = %x \n",pfme_dev->m_pPerf->ccip_fpmon_ch_ctl.freeze);


      PDEBUG( "cache_counter = %x \n",( unsigned int)pfme_dev->m_pPerf->ccip_fpmon_ch_ctr_0.cache_counter);

      PDEBUG( "cache_counter = %x \n",( unsigned int)pfme_dev->m_pPerf->ccip_fpmon_ch_ctr_1.cache_counter);

      PDEBUG( "ccip_port_filter = %x \n",pfme_dev->m_pPerf->ccip_fpmon_fab_ctl.ccip_port_filter);
      PDEBUG( "port_id = %x \n",pfme_dev->m_pPerf->ccip_fpmon_fab_ctl.port_id);
      PDEBUG( "fabric_evt_code = %x \n",pfme_dev->m_pPerf->ccip_fpmon_fab_ctl.fabric_evt_code);
      PDEBUG( "freeze = %x \n",pfme_dev->m_pPerf->ccip_fpmon_fab_ctl.freeze);

      PDEBUG( "fabric_counter = %x \n",( unsigned int)pfme_dev->m_pPerf->ccip_fpmon_fab_ctr.fabric_counter);

      PDEBUG( "afu_interf_clock = %lx \n",( long unsigned int)pfme_dev->m_pPerf->ccip_fpmon_clk_ctrs.afu_interf_clock);

      PDEBUG( "FME   Global Performance END \n \n");
   }

   if(pfme_dev->m_pGerror)
   {
      PDEBUG( "FME   Global Erroe START \n \n");

      PDEBUG( "Feature_ID = %x \n",pfme_dev->m_pGerror->ccip_gerror_dflhdr.Feature_ID);
      PDEBUG( "Feature_rev = %x \n",pfme_dev->m_pGerror->ccip_gerror_dflhdr.Feature_rev);
      PDEBUG( "Type = %x \n",pfme_dev->m_pGerror->ccip_gerror_dflhdr.Type);
      PDEBUG( "next_DFH_offset = %x \n",pfme_dev->m_pGerror->ccip_gerror_dflhdr.next_DFH_offset);
      PDEBUG( "End of List = %x \n",pfme_dev->m_pGerror->ccip_gerror_dflhdr.eol);

      PDEBUG( "FME Error fabric_err  = %x \n",pfme_dev->m_pGerror->fme_err.fabric_err);
      PDEBUG( "FME Error fabFifo_uoflow  = %x \n",pfme_dev->m_pGerror->fme_err.fabFifo_uoflow);
      PDEBUG( "FME Error PCIe0 poison_detected  = %x \n",pfme_dev->m_pGerror->fme_err.pcie0_poison_detected);
      PDEBUG( "FME Error PCIe0 poison_detected  = %x \n",pfme_dev->m_pGerror->fme_err.pcie1_poison_detected);
      PDEBUG( "FME Error IOMMU Parity Erro  = %x \n",pfme_dev->m_pGerror->fme_err.iommu_parity_error);
      PDEBUG( "FME Error AFU error mismatch  = %x \n",pfme_dev->m_pGerror->fme_err.afuerr_access_mismatch);


      PDEBUG( "FME Error mask  fabric_err  = %x \n",pfme_dev->m_pGerror->fme_err_mask.fabric_err);
      PDEBUG( "FME Error mask fabFifo_uoflow  = %x \n",pfme_dev->m_pGerror->fme_err_mask.fabFifo_uoflow);
      PDEBUG( "FME Error PCIe0 poison_detected  = %x \n",pfme_dev->m_pGerror->fme_err_mask.pcie0_poison_detected);
      PDEBUG( "FME Error PCIe0 poison_detected  = %x \n",pfme_dev->m_pGerror->fme_err_mask.pcie1_poison_detected);
      PDEBUG( "FME Error IOMMU Parity Erro  = %x \n",pfme_dev->m_pGerror->fme_err_mask.iommu_parity_error);
      PDEBUG( "FME Error AFU error mismatch  = %x \n",pfme_dev->m_pGerror->fme_err_mask.afuerr_access_mismatch);


      PDEBUG( "PCIe0 Error mask  formattype_err  = %x \n",pfme_dev->m_pGerror->pcie0_err_mask.formattype_err);
      PDEBUG( "PCIe0 Error mask  MWAddr_err  = %x \n",pfme_dev->m_pGerror->pcie0_err_mask.MWAddr_err);
      PDEBUG( "PCIe0 Error mask  MWAddrLength_err  = %x \n",pfme_dev->m_pGerror->pcie0_err_mask.MWAddrLength_err);
      PDEBUG( "PCIe0 Error mask  MRAddr_err  = %x \n",pfme_dev->m_pGerror->pcie0_err_mask.MRAddr_err);
      PDEBUG( "PCIe0 Error mask  MRAddrLength_err  = %x \n",pfme_dev->m_pGerror->pcie0_err_mask.MRAddrLength_err);
      PDEBUG( "PCIe0 Error mask  cpl_tag_err  = %x \n",pfme_dev->m_pGerror->pcie0_err_mask.cpl_tag_err);
      PDEBUG( "PCIe0 Error mask  cpl_status_err  = %x \n",pfme_dev->m_pGerror->pcie0_err_mask.cpl_status_err);
      PDEBUG( "PCIe0 Error mask  formattype_err  = %x \n",pfme_dev->m_pGerror->pcie0_err_mask.cpl_timeout_err);


      PDEBUG( "PCIe0 Error   formattype_err  = %x \n",pfme_dev->m_pGerror->pcie0_err.formattype_err);
      PDEBUG( "PCIe0 Error   MWAddr_err  = %x \n",pfme_dev->m_pGerror->pcie0_err.MWAddr_err);
      PDEBUG( "PCIe0 Error   MWAddrLength_err  = %x \n",pfme_dev->m_pGerror->pcie0_err.MWAddrLength_err);
      PDEBUG( "PCIe0 Error   MRAddr_err  = %x \n",pfme_dev->m_pGerror->pcie0_err.MRAddr_err);
      PDEBUG( "PCIe0 Error   MRAddrLength_err  = %x \n",pfme_dev->m_pGerror->pcie0_err.MRAddrLength_err);
      PDEBUG( "PCIe0 Error   cpl_tag_err  = %x \n",pfme_dev->m_pGerror->pcie0_err.cpl_tag_err);
      PDEBUG( "PCIe0 Error   cpl_status_err  = %x \n",pfme_dev->m_pGerror->pcie0_err.cpl_status_err);
      PDEBUG( "PCIe0 Error   formattype_err  = %x \n",pfme_dev->m_pGerror->pcie0_err.cpl_timeout_err);



      PDEBUG( "PCIe1 Error mask  formattype_err  = %x \n",pfme_dev->m_pGerror->pcie1_err_mask.formattype_err);
      PDEBUG( "PCIe1 Error mask  MWAddr_err  = %x \n",pfme_dev->m_pGerror->pcie1_err_mask.MWAddr_err);
      PDEBUG( "PCIe1 Error mask  MWAddrLength_err  = %x \n",pfme_dev->m_pGerror->pcie1_err_mask.MWAddrLength_err);
      PDEBUG( "PCIe1 Error mask  MRAddr_err  = %x \n",pfme_dev->m_pGerror->pcie1_err_mask.MRAddr_err);
      PDEBUG( "PCIe1 Error mask  MRAddrLength_err  = %x \n",pfme_dev->m_pGerror->pcie1_err_mask.MRAddrLength_err);
      PDEBUG( "PCIe1 Error mask  cpl_tag_err  = %x \n",pfme_dev->m_pGerror->pcie1_err_mask.cpl_tag_err);
      PDEBUG( "PCIe1 Error mask  cpl_status_err  = %x \n",pfme_dev->m_pGerror->pcie1_err_mask.cpl_status_err);
      PDEBUG( "PCIe1 Error mask  formattype_err  = %x \n",pfme_dev->m_pGerror->pcie1_err_mask.cpl_timeout_err);


      PDEBUG( "PCIe1 Error   formattype_err  = %x \n",pfme_dev->m_pGerror->pcie1_err.formattype_err);
      PDEBUG( "PCIe1 Error   MWAddr_err  = %x \n",pfme_dev->m_pGerror->pcie1_err.MWAddr_err);
      PDEBUG( "PCIe1 Error   MWAddrLength_err  = %x \n",pfme_dev->m_pGerror->pcie1_err.MWAddrLength_err);
      PDEBUG( "PCIe1 Error   MRAddr_err  = %x \n",pfme_dev->m_pGerror->pcie1_err.MRAddr_err);
      PDEBUG( "PCIe1 Error   MRAddrLength_err  = %x \n",pfme_dev->m_pGerror->pcie1_err.MRAddrLength_err);
      PDEBUG( "PCIe1 Error   cpl_tag_err  = %x \n",pfme_dev->m_pGerror->pcie1_err.cpl_tag_err);
      PDEBUG( "PCIe1 Error   cpl_status_err  = %x \n",pfme_dev->m_pGerror->pcie1_err.cpl_status_err);
      PDEBUG( "PCIe1 Error   formattype_err  = %x \n",pfme_dev->m_pGerror->pcie1_err.cpl_timeout_err);


      PDEBUG( "Green BS Error mask  thremal threshold AP1  = %x \n",pfme_dev->m_pGerror->ras_gerr_mask.temp_trash_ap1);
      PDEBUG( "Green BS Error mask  thremal threshold AP2  = %x \n",pfme_dev->m_pGerror->ras_gerr_mask.temp_trash_ap2);
      PDEBUG( "Green BS Error mask  pcie_error  = %x \n",pfme_dev->m_pGerror->ras_gerr_mask.pcie_error);
      PDEBUG( "Green BS Error mask  afufatal_error  = %x \n",pfme_dev->m_pGerror->ras_gerr_mask.afufatal_error);
      PDEBUG( "Green BS Error mask  proc hot error  = %x \n",pfme_dev->m_pGerror->ras_gerr_mask.prochot_error);
      PDEBUG( "Green BS Error mask  afu access mode  error  = %x \n",pfme_dev->m_pGerror->ras_gerr_mask.afu_access_mismatch);
      PDEBUG( "Green BS Error mask  Injected warning  errorr  = %x \n",pfme_dev->m_pGerror->ras_gerr_mask.injected_warn_error);
      PDEBUG( "Green BS Error mask  PCIe poison port  error  = %x \n",pfme_dev->m_pGerror->ras_gerr_mask.pcie_posion_error);
      PDEBUG( "Green BS Error mask  gb_crc_err  = %x \n",pfme_dev->m_pGerror->ras_gerr_mask.gb_crc_err);
      PDEBUG( "Green BS Error mask  thremal threshold AP  = %x \n",pfme_dev->m_pGerror->ras_gerr_mask.temp_trash_ap6);
      PDEBUG( "Green BS Error mask  Power threshold AP1  = %x \n",pfme_dev->m_pGerror->ras_gerr_mask.power_trash_ap1);
      PDEBUG( "Green BS Error mask  Power threshold AP6  = %x \n",pfme_dev->m_pGerror->ras_gerr_mask.power_trash_ap2);
      PDEBUG( "Green BS Error mask  MBP error Error  = %x \n",pfme_dev->m_pGerror->ras_gerr_mask.mbp_error);


      PDEBUG( "Green BS Error mask  thremal threshold AP1  = %x \n",pfme_dev->m_pGerror->ras_gerr.temp_trash_ap1);
      PDEBUG( "Green BS Error mask  thremal threshold AP2  = %x \n",pfme_dev->m_pGerror->ras_gerr.temp_trash_ap2);
      PDEBUG( "Green BS Error mask  pcie_error  = %x \n",pfme_dev->m_pGerror->ras_gerr.pcie_error);
      PDEBUG( "Green BS Error mask  afufatal_error  = %x \n",pfme_dev->m_pGerror->ras_gerr.afufatal_error);
      PDEBUG( "Green BS Error mask  proc hot error  = %x \n",pfme_dev->m_pGerror->ras_gerr.prochot_error);
      PDEBUG( "Green BS Error mask  afu access mode  error  = %x \n",pfme_dev->m_pGerror->ras_gerr.afu_access_mismatch);
      PDEBUG( "Green BS Error mask  Injected warning  errorr  = %x \n",pfme_dev->m_pGerror->ras_gerr.injected_warn_error);
      PDEBUG( "Green BS Error mask  PCIe poison port  error  = %x \n",pfme_dev->m_pGerror->ras_gerr.pcie_posion_error);
      PDEBUG( "Green BS Error mask  gb_crc_err  = %x \n",pfme_dev->m_pGerror->ras_gerr.gb_crc_err);
      PDEBUG( "Green BS Error mask  thremal threshold AP  = %x \n",pfme_dev->m_pGerror->ras_gerr.temp_trash_ap6);
      PDEBUG( "Green BS Error mask  Power threshold AP1  = %x \n",pfme_dev->m_pGerror->ras_gerr.power_trash_ap1);
      PDEBUG( "Green BS Error mask  Power threshold AP6  = %x \n",pfme_dev->m_pGerror->ras_gerr.power_trash_ap2);
      PDEBUG( "Green BS Error mask  MBP error Error  = %x \n",pfme_dev->m_pGerror->ras_gerr.mbp_error);



      PDEBUG( "blue BS Error mask  ktilink_fatal_err  = %x \n",pfme_dev->m_pGerror->ras_berror_mask.ktilink_fatal_err);
      PDEBUG( "blue BS Error mask  tagcch_fatal_err  = %x \n",pfme_dev->m_pGerror->ras_berror_mask.tagcch_fatal_err);
      PDEBUG( "blue BS Error mask  cci_fatal_err  = %x \n",pfme_dev->m_pGerror->ras_berror_mask.cci_fatal_err);
      PDEBUG( "blue BS Error mask  ktiprpto_fatal_err  = %x \n",pfme_dev->m_pGerror->ras_berror_mask.ktiprpto_fatal_err);
      PDEBUG( "blue BS Error mask  dma_fatal_err  = %x \n",pfme_dev->m_pGerror->ras_berror_mask.dma_fatal_err);
      PDEBUG( "blue BS Error mask  iommu_catast_err  = %x \n",pfme_dev->m_pGerror->ras_berror_mask.iommu_catast_err);
      PDEBUG( "blue BS Error mask  crc_catast_err  = %x \n",pfme_dev->m_pGerror->ras_berror_mask.crc_catast_err);
      PDEBUG( "blue BS Error mask  therm_catast_err  = %x \n",pfme_dev->m_pGerror->ras_berror_mask.therm_catast_err);


      PDEBUG( "blue BS Error   ktilink_fatal_err  = %x \n",pfme_dev->m_pGerror->ras_berror.ktilink_fatal_err);
      PDEBUG( "blue BS Error   tagcch_fatal_err  = %x \n",pfme_dev->m_pGerror->ras_berror.tagcch_fatal_err);
      PDEBUG( "blue BS Error   cci_fatal_err  = %x \n",pfme_dev->m_pGerror->ras_berror.cci_fatal_err);
      PDEBUG( "blue BS Error   ktiprpto_fatal_err  = %x \n",pfme_dev->m_pGerror->ras_berror.ktiprpto_fatal_err);
      PDEBUG( "blue BS Error   dma_fatal_err  = %x \n",pfme_dev->m_pGerror->ras_berror.dma_fatal_err);
      PDEBUG( "blue BS Error   iommu_catast_err  = %x \n",pfme_dev->m_pGerror->ras_berror.iommu_catast_err);
      PDEBUG( "blue BS Error   crc_catast_err  = %x \n",pfme_dev->m_pGerror->ras_berror.crc_catast_err);
      PDEBUG( "blue BS Error   therm_catast_err  = %x \n",pfme_dev->m_pGerror->ras_berror.therm_catast_err);


      PDEBUG( "RAS Warning mask  event_warn_err  = %x \n",pfme_dev->m_pGerror->ras_warnerror_mask.event_warn_err);
      PDEBUG( "RAS Warning   event_warn_err  = %x \n",pfme_dev->m_pGerror->ras_warnerror.event_warn_err);


      PDEBUG( "FME   Global Error END \n \n");

   }

   if(pfme_dev->m_pPRmgmt)
   {
      PDEBUG( "FME PR Feature  START \n \n");

      PDEBUG( "Feature_ID = %x \n",pfme_dev->m_pPRmgmt->ccip_pr_dflhdr.Feature_ID);
      PDEBUG( "Feature_rev = %x \n",pfme_dev->m_pPRmgmt->ccip_pr_dflhdr.Feature_rev);
      PDEBUG( "Type = %x \n",pfme_dev->m_pPRmgmt->ccip_pr_dflhdr.Type);
      PDEBUG( "next_DFH_offset = %x \n",pfme_dev->m_pPRmgmt->ccip_pr_dflhdr.next_DFH_offset);
      PDEBUG( "End of List = %x \n",pfme_dev->m_pPRmgmt->ccip_pr_dflhdr.eol);

      PDEBUG( "enable_pr_port_access = %x \n",pfme_dev->m_pPRmgmt->ccip_fme_pr_control.enable_pr_port_access);
      PDEBUG( "pr_regionid = %x \n",pfme_dev->m_pPRmgmt->ccip_fme_pr_control.pr_regionid);
      PDEBUG( "pr_start_req = %x \n",pfme_dev->m_pPRmgmt->ccip_fme_pr_control.pr_start_req);
      PDEBUG( "pr_push_complete = %x \n",pfme_dev->m_pPRmgmt->ccip_fme_pr_control.pr_push_complete);


      PDEBUG( "pr_credit = %x \n",pfme_dev->m_pPRmgmt->ccip_fme_pr_status.pr_credit);
      PDEBUG( "pr_status = %x \n",pfme_dev->m_pPRmgmt->ccip_fme_pr_status.pr_status);
      PDEBUG( "pr_contoller_status = %x \n",pfme_dev->m_pPRmgmt->ccip_fme_pr_status.pr_contoller_status);
      PDEBUG( "pr_host_status = %x \n",pfme_dev->m_pPRmgmt->ccip_fme_pr_status.pr_host_status);


      PDEBUG( "pr_data_raw = %x \n",pfme_dev->m_pPRmgmt->ccip_fme_pr_data.pr_data_raw);

      PDEBUG( "PR_operation_err = %x \n",pfme_dev->m_pPRmgmt->ccip_fme_pr_err.PR_operation_err);
      PDEBUG( "PR_CRC_err = %x \n",pfme_dev->m_pPRmgmt->ccip_fme_pr_err.PR_CRC_err);
      PDEBUG( "PR_bitstream_err = %x \n",pfme_dev->m_pPRmgmt->ccip_fme_pr_err.PR_bitstream_err);
      PDEBUG( "PR_IP_err = %x \n",pfme_dev->m_pPRmgmt->ccip_fme_pr_err.PR_IP_err);
      PDEBUG( "PR_FIFIO_err = %x \n",pfme_dev->m_pPRmgmt->ccip_fme_pr_err.PR_FIFIO_err);
      PDEBUG( "PR_timeout_err = %x \n",pfme_dev->m_pPRmgmt->ccip_fme_pr_err.PR_timeout_err);


      PDEBUG( "FME PR Feature  END \n \n");
   }

   PDEBUG( "*****************************************************************\n");
   PDEBUG( "*****************************************************************\n");

   EROR:

   return res;
}

int print_sim_port_device(struct port_device *pport_dev)
{
   int res = 0;
   if ( NULL == pport_dev ) {
      PERR("Unable to allocate system memory for pfme_dev object\n");
      res = -1;
      goto ERR;
   }

   PDEBUG( "*****************************************************************\n");
   PDEBUG( "*****************************************************************\n");
   PDEBUG( "****************        PORT MMIO CONTENT        ****************\n");
   PDEBUG( "*****************************************************************\n");
   PDEBUG( "*****************************************************************\n");

   if(pport_dev->m_pport_hdr)
   {

      PDEBUG( "PORT Header START \n \n");

      PDEBUG( "dfh.Type= %x \n",pport_dev->m_pport_hdr->ccip_port_dfh.Type);
      PDEBUG( "dfh.Feature_ID= %x \n",pport_dev->m_pport_hdr->ccip_port_dfh.Feature_ID);
      PDEBUG( "dfh.Feature_rev= %x \n",pport_dev->m_pport_hdr->ccip_port_dfh.Feature_rev);
      PDEBUG( "dfh.next_DFH_offset= %x \n",pport_dev->m_pport_hdr->ccip_port_dfh.next_DFH_offset);
      PDEBUG( "End of List = %x \n",pport_dev->m_pport_hdr->ccip_port_dfh.eol);


      PDEBUG( "afu_id_l.afu_id_l= %lx \n",( long unsigned int)pport_dev->m_pport_hdr->ccip_port_afuidl.afu_id_l);
      PDEBUG( "afu_id_h.afu_id_h= %lx \n",( long unsigned int)pport_dev->m_pport_hdr->ccip_port_afuidh.afu_id_h);

      PDEBUG( "next_afu.afu_id_offset= %x \n",pport_dev->m_pport_hdr->ccip_port_next_afu.afu_id_offset);

      PDEBUG( "scratch_pad= %lx \n",( long unsigned int)pport_dev->m_pport_hdr->ccip_port_scratchpad.scratch_pad);

      PDEBUG( "interrupts= %x \n",pport_dev->m_pport_hdr->ccip_port_capability.interrupts);
      PDEBUG( "mmio_size= %x \n",pport_dev->m_pport_hdr->ccip_port_capability.mmio_size);
      PDEBUG( "port_id= %x \n",pport_dev->m_pport_hdr->ccip_port_capability.port_id);
      // PDEBUG( "usmg_size= %x \n",pport_dev->m_pport_hdr->ccip_port_capability.usmg_size);

      PDEBUG( "ccip_outstanding_request= %x \n",pport_dev->m_pport_hdr->ccip_port_control.ccip_outstanding_request);
      PDEBUG( "afu_latny_rep= %x \n",pport_dev->m_pport_hdr->ccip_port_control.afu_latny_rep);
      PDEBUG( "port_sftreset_control= %x \n",pport_dev->m_pport_hdr->ccip_port_control.port_sftreset_control);

      PDEBUG( "afu_pwr_state= %x \n",pport_dev->m_pport_hdr->ccip_port_status.afu_pwr_state);

      PDEBUG( "PORT Header END \n \n");

   }

   if(pport_dev->m_pport_err)
   {

      PDEBUG( "PORT ERROR  Feature  START \n \n");

      PDEBUG( "Feature_ID = %x \n",pport_dev->m_pport_err->ccip_port_err_dflhdr.Feature_ID);
      PDEBUG( "Feature_rev = %x \n",pport_dev->m_pport_err->ccip_port_err_dflhdr.Feature_rev);
      PDEBUG( "Type = %x \n",pport_dev->m_pport_err->ccip_port_err_dflhdr.Type);

      PDEBUG( "next_DFH_offset = %x \n",pport_dev->m_pport_err->ccip_port_err_dflhdr.next_DFH_offset);
      PDEBUG( "End of List = %x \n",pport_dev->m_pport_err->ccip_port_err_dflhdr.eol);

      PDEBUG( "rsvd = %x \n",(  unsigned int)pport_dev->m_pport_err->ccip_port_error_mask.rsvd);

      PDEBUG( "Port Error tx_ch0_overflow = %x \n",pport_dev->m_pport_err->ccip_port_error.tx_ch0_overflow);
      PDEBUG( "Port Error tx_ch0_invalidreq = %x \n",pport_dev->m_pport_err->ccip_port_error.tx_ch0_invalidreq);
      PDEBUG( "Port Error tx_ch0_req_cl_len3 = %x \n",pport_dev->m_pport_err->ccip_port_error.tx_ch0_req_cl_len3);
      PDEBUG( "Port Error tx_ch0_req_cl_len2 = %x \n",pport_dev->m_pport_err->ccip_port_error.tx_ch0_req_cl_len2);
      PDEBUG( "Port Error tx_ch0_req_cl_len2 = %x \n",pport_dev->m_pport_err->ccip_port_error.tx_ch0_req_cl_len4);

      PDEBUG( "Port Error tx_ch1_overflow = %x \n",pport_dev->m_pport_err->ccip_port_error.tx_ch1_overflow);
      PDEBUG( "Port Error tx_ch1_invalidreq = %x \n",pport_dev->m_pport_err->ccip_port_error.tx_ch1_invalidreq);
      PDEBUG( "Port Error tx_ch1_req_cl_len3 = %x \n",pport_dev->m_pport_err->ccip_port_error.tx_ch1_req_cl_len3);
      PDEBUG( "Port Error tx_ch1_req_cl_len2 = %x \n",pport_dev->m_pport_err->ccip_port_error.tx_ch1_req_cl_len2);
      PDEBUG( "Port Error tx_ch1_req_cl_len4 = %x \n",pport_dev->m_pport_err->ccip_port_error.tx_ch1_req_cl_len4);

      PDEBUG( "Port Error tx_ch1_insuff_datapayload = %x \n",pport_dev->m_pport_err->ccip_port_error.tx_ch1_insuff_datapayload);
      PDEBUG( "Port Error tx_ch1_datapayload_overrun = %x \n",pport_dev->m_pport_err->ccip_port_error.tx_ch1_datapayload_overrun);
      PDEBUG( "Port Error tx_ch1_incorr_addr = %x \n",pport_dev->m_pport_err->ccip_port_error.tx_ch1_incorr_addr);
      PDEBUG( "Port Error tx_ch1_sop_detcted = %x \n",pport_dev->m_pport_err->ccip_port_error.tx_ch1_sop_detcted);
      PDEBUG( "Port Error tx_ch1_atomic_req = %x \n",pport_dev->m_pport_err->ccip_port_error.tx_ch1_atomic_req);
      PDEBUG( "Port Error mmioread_timeout = %x \n",pport_dev->m_pport_err->ccip_port_error.mmioread_timeout);
      PDEBUG( "Port Error tx_ch2_fifo_overflow = %x \n",pport_dev->m_pport_err->ccip_port_error.tx_ch2_fifo_overflow);
      PDEBUG( "Port Error num_pending_req_overflow = %x \n",pport_dev->m_pport_err->ccip_port_error.num_pending_req_overflow);

      PDEBUG( "Port Error unexp_mmio_resp = %x \n",pport_dev->m_pport_err->ccip_port_error.unexp_mmio_resp);

      PDEBUG( "Port Error llpr_smrr_err = %x \n",pport_dev->m_pport_err->ccip_port_error.llpr_smrr_err);
      PDEBUG( "Port Error llpr_smrr2_err = %x \n",pport_dev->m_pport_err->ccip_port_error.llpr_smrr2_err);
      PDEBUG( "Port Error llpr_mesg_err = %x \n",pport_dev->m_pport_err->ccip_port_error.llpr_mesg_err);
      PDEBUG( "Port Error genport_range_err = %x \n",pport_dev->m_pport_err->ccip_port_error.genport_range_err);
      PDEBUG( "Port Error legrange_low_err = %x \n",pport_dev->m_pport_err->ccip_port_error.legrange_low_err);
      PDEBUG( "Port Error legrange_hight_err = %x \n",pport_dev->m_pport_err->ccip_port_error.legrange_hight_err);
      PDEBUG( "Port Error vgmem_range_err = %x \n",pport_dev->m_pport_err->ccip_port_error.vgmem_range_err);

      PDEBUG( "Port Error page_fault_err = %x \n",pport_dev->m_pport_err->ccip_port_error.page_fault_err);
      PDEBUG( "Port Error pmr_err = %x \n",pport_dev->m_pport_err->ccip_port_error.pmr_err);
      PDEBUG( "Port Error ap6_event = %x \n",pport_dev->m_pport_err->ccip_port_error.ap6_event);
      PDEBUG( "Port Error vfflr_accesseror = %x \n",pport_dev->m_pport_err->ccip_port_error.vfflr_accesseror);


      PDEBUG( "Port First Error tx_ch0_overflow = %x \n",pport_dev->m_pport_err->ccip_port_first_error.tx_ch0_overflow);
      PDEBUG( "Port First Error tx_ch0_invalidreq = %x \n",pport_dev->m_pport_err->ccip_port_first_error.tx_ch0_invalidreq);
      PDEBUG( "Port First Error tx_ch0_req_cl_len3 = %x \n",pport_dev->m_pport_err->ccip_port_first_error.tx_ch0_req_cl_len3);
      PDEBUG( "Port First Error tx_ch0_req_cl_len2 = %x \n",pport_dev->m_pport_err->ccip_port_first_error.tx_ch0_req_cl_len2);
      PDEBUG( "Port First Error tx_ch0_req_cl_len2 = %x \n",pport_dev->m_pport_err->ccip_port_first_error.tx_ch0_req_cl_len4);

      PDEBUG( "Port First Error tx_ch1_overflow = %x \n",pport_dev->m_pport_err->ccip_port_first_error.tx_ch1_overflow);
      PDEBUG( "Port First Error tx_ch1_invalidreq = %x \n",pport_dev->m_pport_err->ccip_port_first_error.tx_ch1_invalidreq);
      PDEBUG( "Port First Error tx_ch1_req_cl_len3 = %x \n",pport_dev->m_pport_err->ccip_port_first_error.tx_ch1_req_cl_len3);
      PDEBUG( "Port First Error tx_ch1_req_cl_len2 = %x \n",pport_dev->m_pport_err->ccip_port_first_error.tx_ch1_req_cl_len2);
      PDEBUG( "Port First Error tx_ch1_req_cl_len4 = %x \n",pport_dev->m_pport_err->ccip_port_first_error.tx_ch1_req_cl_len4);

      PDEBUG( "Port First Error tx_ch1_insuff_datapayload = %x \n",pport_dev->m_pport_err->ccip_port_first_error.tx_ch1_insuff_datapayload);
      PDEBUG( "Port First Error tx_ch1_datapayload_overrun = %x \n",pport_dev->m_pport_err->ccip_port_first_error.tx_ch1_datapayload_overrun);
      PDEBUG( "Port First Error tx_ch1_incorr_addr = %x \n",pport_dev->m_pport_err->ccip_port_first_error.tx_ch1_incorr_addr);
      PDEBUG( "Port First Error tx_ch1_sop_detcted = %x \n",pport_dev->m_pport_err->ccip_port_first_error.tx_ch1_sop_detcted);
      PDEBUG( "Port First Error tx_ch1_atomic_req = %x \n",pport_dev->m_pport_err->ccip_port_first_error.tx_ch1_atomic_req);
      PDEBUG( "Port First Error mmioread_timeout = %x \n",pport_dev->m_pport_err->ccip_port_first_error.mmioread_timeout);
      PDEBUG( "Port First Error tx_ch2_fifo_overflow = %x \n",pport_dev->m_pport_err->ccip_port_first_error.tx_ch2_fifo_overflow);
      PDEBUG( "Port First Error num_pending_req_overflow = %x \n",pport_dev->m_pport_err->ccip_port_first_error.num_pending_req_overflow);


      PDEBUG( "malfrd_req_lsb = %lx \n",( long unsigned int)pport_dev->m_pport_err->ccip_port_malformed_req_0.malfrd_req_lsb);

      PDEBUG( "malfrd_req_msb = %lx \n",( long unsigned int)pport_dev->m_pport_err->ccip_port_malformed_req_1.malfrd_req_msb);

      PDEBUG( "PORT ERROR  Feature  END \n \n");

   }

   if(pport_dev->m_pport_umsg)
   {
      PDEBUG( "PORT USMG  START \n \n");

      PDEBUG( "Feature_ID = %x \n",pport_dev->m_pport_umsg->ccip_port_umsg_dflhdr.Feature_ID);
      PDEBUG( "Feature_rev = %x \n",pport_dev->m_pport_umsg->ccip_port_umsg_dflhdr.Feature_rev);
      PDEBUG( "Type = %x \n",pport_dev->m_pport_umsg->ccip_port_umsg_dflhdr.Type);
      PDEBUG( "next_DFH_offset = %x \n",pport_dev->m_pport_umsg->ccip_port_umsg_dflhdr.next_DFH_offset);
      PDEBUG( "End of List = %x \n",pport_dev->m_pport_umsg->ccip_port_umsg_dflhdr.eol);


      PDEBUG( "no_umsg_alloc_port = %x \n",pport_dev->m_pport_umsg->ccip_umsg_capability.no_umsg_alloc_port);
      PDEBUG( "status_umsg_engine = %x \n",pport_dev->m_pport_umsg->ccip_umsg_capability.status_umsg_engine);
      PDEBUG( "umsg_init_status = %x \n",pport_dev->m_pport_umsg->ccip_umsg_capability.umsg_init_status);


      PDEBUG( "umsg_base_address = %lx \n",( long unsigned int)pport_dev->m_pport_umsg->ccip_umsg_base_address.umsg_base_address);

      PDEBUG( "umsg_hit = %x \n",pport_dev->m_pport_umsg->ccip_umsg_mode.umsg_hit);

      PDEBUG( "PORT USMG  END \n \n");
   }


   if(pport_dev->m_pport_pr)
   {
      PDEBUG( "PORT PR   START \n \n");
      PDEBUG( "Feature_ID = %x \n",pport_dev->m_pport_pr->ccip_port_pr_dflhdr.Feature_ID);
      PDEBUG( "Feature_rev = %x \n",pport_dev->m_pport_pr->ccip_port_pr_dflhdr.Feature_rev);
      PDEBUG( "Type = %x \n",pport_dev->m_pport_pr->ccip_port_pr_dflhdr.Type);
      PDEBUG( "next_DFH_offset = %x \n",pport_dev->m_pport_pr->ccip_port_pr_dflhdr.next_DFH_offset);
      PDEBUG( "End of List = %x \n",pport_dev->m_pport_pr->ccip_port_pr_dflhdr.eol);

      PDEBUG( "pr_start_req = %x \n",pport_dev->m_pport_pr->ccip_port_pr_control.pr_start_req);

      PDEBUG( "pr_status = %x \n",pport_dev->m_pport_pr->ccip_port_pr_status.pr_status);
      PDEBUG( "pr_mega_fstatus = %x \n",pport_dev->m_pport_pr->ccip_port_pr_status.pr_mega_fstatus);
      PDEBUG( "pr_data_ovrferr = %x \n",pport_dev->m_pport_pr->ccip_port_pr_status.pr_data_ovrferr);
      PDEBUG( "pr_engine_error = %x \n",pport_dev->m_pport_pr->ccip_port_pr_status.pr_engine_error);
      PDEBUG( "pr_timeout_error = %x \n",pport_dev->m_pport_pr->ccip_port_pr_status.pr_timeout_error);
      PDEBUG( "pr_access = %x \n",pport_dev->m_pport_pr->ccip_port_pr_status.pr_access);
      PDEBUG( "pr_credit = %x \n",pport_dev->m_pport_pr->ccip_port_pr_status.pr_credit);

      PDEBUG( "pr_data_raw = %x \n",pport_dev->m_pport_pr->ccip_port_pr_data.pr_data_raw);

      PDEBUG( "pwr_format = %x \n",( unsigned int)pport_dev->m_pport_pr->ccip_port_pr_pbudget.pwr_format);

      PDEBUG( "PORT PR   END \n \n");

   }

   if(pport_dev->m_pport_stap)
   {

      PDEBUG( "PORT SIGNAL TAP   START \n \n");
      PDEBUG( "Feature_ID = %x \n",pport_dev->m_pport_stap->ccip_port_stap_dflhdr.Feature_ID);
      PDEBUG( "Feature_rev = %x \n",pport_dev->m_pport_stap->ccip_port_stap_dflhdr.Feature_rev);
      PDEBUG( "Type = %x \n",pport_dev->m_pport_stap->ccip_port_stap_dflhdr.Type);
      PDEBUG( "next_DFH_offset = %x \n",pport_dev->m_pport_stap->ccip_port_stap_dflhdr.next_DFH_offset);
      PDEBUG( "End of List = %x \n",pport_dev->m_pport_stap->ccip_port_stap_dflhdr.eol);

      PDEBUG( "Signal tap rsvd = %x \n",(  unsigned int)pport_dev->m_pport_stap->ccip_port_stap.rsvd);

      PDEBUG( "PORT SIGNAL tap  END \n \n");

   }

   if(0 != pport_dev->m_pport_hdr->ccip_port_next_afu.afu_id_offset) {

      // Get the AFU Header
      struct CCIP_AFU_Header *pafu_hdr = (struct CCIP_AFU_Header *)(((btVirtAddr)pport_dev->m_pport_hdr) + pport_dev->m_pport_hdr->ccip_port_next_afu.afu_id_offset);

      PDEBUG( "USER AFU   START %p \n \n", pafu_hdr);

      PDEBUG( "Feature_ID = %x \n",pafu_hdr->ccip_dfh.Feature_ID);
      PDEBUG( "Feature_rev = %x \n",pafu_hdr->ccip_dfh.Feature_rev);
      PDEBUG( "Type = %x \n",pafu_hdr->ccip_dfh.Type);
      PDEBUG( "next_DFH_offset = %x \n",pafu_hdr->ccip_dfh.next_DFH_offset);

      PDEBUG( "afu_id_l.afu_id_l= %lx \n",( long unsigned int)pafu_hdr->ccip_afu_id_l.afu_id_l);
      PDEBUG( "afu_id_h.afu_id_h= %lx \n",( long unsigned int)pafu_hdr->ccip_afu_id_h.afu_id_h);

      PDEBUG( "next_afu.afu_id_offset= %x \n",pafu_hdr->ccip_next_afu.afu_id_offset);


      PDEBUG( "USER AFU  END \n \n");
 //     pafu_hdr = NULL;  // COMPILER COMPLAINS VARIABLE NOT USED UNLESS WE DO THIS
   }


   PDEBUG( "*****************************************************************\n");
   PDEBUG( "*****************************************************************\n");

   ERR:

   return res;

}



