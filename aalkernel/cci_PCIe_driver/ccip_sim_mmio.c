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

#include "ccip_fme.h"
#include "ccip_port.h"

#define OFFSET 0x8;

int  ccip_sim_wrt_fme_mmio(btVirtAddr pkvp_fme_mmio)
{
   struct CCIP_FME_HDR           fme_hdr;
   struct CCIP_FME_DFL_THERM     fme_tmp;
   struct CCIP_FME_DFL_PM        fme_pm;
   struct CCIP_FME_DFL_FPMON     fme_fpmon;
   struct CCIP_FME_DFL_GERROR    fme_gerror;
   struct CCIP_FME_DFL_PR        fme_pr;
   struct CCIP_PORT_AFU_OFFSET   portoffset;

   int offset =0;

   btVirtAddr ptr= pkvp_fme_mmio;

   PINFO(" ccip_sim_wrt_fme_mmio ENTER\n");

   // FME header
   fme_hdr.dfh.Type =CCIP_DFType_afu;
   fme_hdr.dfh.next_DFH_offset =0x1000;
   fme_hdr.dfh.Feature_rev =0;
   fme_hdr.dfh.Feature_ID =1;
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
   fme_hdr.fme_scratchpad.scratch_pad = 0x1234568;
   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset, fme_hdr.fme_scratchpad.csr);

   // FME Capability CSR
   fme_hdr.ccip_fme_capability.lock_bit =0x0;
   fme_hdr.ccip_fme_capability.cache_assoc =0x0;
   fme_hdr.ccip_fme_capability.cache_size = 0x10;
   fme_hdr.ccip_fme_capability.address_width_bits =0x26;
   fme_hdr.ccip_fme_capability.iommu_support =0x0;
   fme_hdr.ccip_fme_capability.qpi_link_avile =0x01;
   fme_hdr.ccip_fme_capability.pci0_link_avile =0x01;
   fme_hdr.ccip_fme_capability.pci1_link_avile =0x01;
   fme_hdr.ccip_fme_capability.socket_id =0x1;
   fme_hdr.ccip_fme_capability.fabric_verid =0x1;
   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,fme_hdr.ccip_fme_capability.csr);


   // 3 simulated ports
   //port 1
   portoffset.port_imp =0x1;
   portoffset.port_arbit_poly =0x1;
   portoffset.port_bar =0x1;
   portoffset.port_offset =0x60000;
   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,portoffset.csr);

   //port 2
   portoffset.port_imp =0x1;
   portoffset.port_arbit_poly =0x1;
   portoffset.port_bar =0x1;
   portoffset.port_offset =0x70000;
   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,portoffset.csr);

   //port 3
   portoffset.port_imp =0x1;
   portoffset.port_arbit_poly =0x1;
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
   offset = 0;
   write_ccip_csr64(ptr,offset,fme_tmp.ccip_fme_tmp_dflhdr.csr);


   // Temperature threshold csr
   fme_tmp.ccip_tmp_threshold.force_proc_hot  =0x1;
   fme_tmp.ccip_tmp_threshold.proc_hot_setpoint =0x1;
   fme_tmp.ccip_tmp_threshold.setproc_temp_reach_setpoint =0x1;
   fme_tmp.ccip_tmp_threshold.therm_proc_hot =0x1 ;
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
   offset = 0 ;
   write_ccip_csr64(ptr,offset,fme_pm.ccip_fme_pm_dflhdr.csr);


   // Power Management Threshold CSR
   fme_pm.ccip_pm_threshold.fpga_latency_report =0x1;
   fme_pm.ccip_pm_threshold.threshold1 =0x22 ;
   fme_pm.ccip_pm_threshold.threshold1_sts =0x1;
   fme_pm.ccip_pm_threshold.threshold2 =0x33;
   fme_pm.ccip_pm_threshold.threshold2_sts =0x1;
   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,fme_pm.ccip_pm_threshold.csr);

   // Power Management Read Write Voltage Regulator CSR
   fme_pm.ccip_pm_rdvr.clock_buffer_supply_voltvalid = 0x1;
   fme_pm.ccip_pm_rdvr.core_supply_voltvalid = 0x1;
   fme_pm.ccip_pm_rdvr.trans_supply_voltvalid = 0x1;
   fme_pm.ccip_pm_rdvr.fpga_supply_voltvalid =0x1;
   fme_pm.ccip_pm_rdvr.clock_buffer_supply_voltvalue =0xAA;
   fme_pm.ccip_pm_rdvr.core_supply_voltvalue =0xBB;
   fme_pm.ccip_pm_rdvr.trans_supply_voltvalue =0xCC;
   fme_pm.ccip_pm_rdvr.fpga_supply_voltvalue =0xDD;
   fme_pm.ccip_pm_rdvr.volt_regulator_readmods =0x1;
   fme_pm.ccip_pm_rdvr.volt_regulator_readmod_value =0x110;
   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,fme_pm.ccip_pm_rdvr.csr);

   // Power Management Record Maximum Voltage Regulator CSR
   fme_pm.ccip_pm_mrdvr.hw_set_field =0x1;
   fme_pm.ccip_pm_mrdvr.max_clock_supply_voltrec =0xAA;
   fme_pm.ccip_pm_mrdvr.max_core_supply_voltrec =0xBB;
   fme_pm.ccip_pm_mrdvr.max_fpga_supply_voltrec =0xCC;
   fme_pm.ccip_pm_mrdvr.max_trans_supply_voltrec =0xDD;
   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,fme_pm.ccip_pm_mrdvr.csr);


   // FME Global Performance header

   ptr = ptr+ fme_pm.ccip_fme_pm_dflhdr.next_DFH_offset;
   fme_fpmon.ccip_fme_fpmon_dflhdr.Type =CCIP_DFType_private;
   fme_fpmon.ccip_fme_fpmon_dflhdr.next_DFH_offset =0x1000;
   fme_fpmon.ccip_fme_fpmon_dflhdr.Feature_rev =0;
   fme_fpmon.ccip_fme_fpmon_dflhdr.Feature_ID= CCIP_FME_DFLID_GPERF;
   offset = 0;
   write_ccip_csr64(ptr,offset,fme_fpmon.ccip_fme_fpmon_dflhdr.csr);

   // Cache control
   fme_fpmon.ccip_fpmon_ch_ctl.cache_event =0x6;
   fme_fpmon.ccip_fpmon_ch_ctl.freeze =0x1;
   fme_fpmon.ccip_fpmon_ch_ctl.reset_counter =0x1;
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
   fme_fpmon.ccip_fpmon_fab_ctl.reset_counter =0x1;
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
   offset = 0;
   write_ccip_csr64(ptr,offset,fme_gerror.ccip_gerror_dflhdr.csr);

   // FME error mask
   fme_gerror.ccip_fme_error_mask.rsvd=0;
   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,fme_gerror.ccip_fme_error_mask.csr);

   // FME error
   fme_gerror.ccip_fme_error.rsvd=0;
   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,fme_gerror.ccip_fme_error.csr);

   // FME First error
   fme_gerror.ccip_fme_first_error.rsvd =0;
   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,fme_gerror.ccip_fme_first_error.csr);


   // FME PR
   ptr = ptr+ fme_gerror.ccip_gerror_dflhdr.next_DFH_offset;
   fme_pr.ccip_pr_dflhdr.Type =CCIP_DFType_private;
   fme_pr.ccip_pr_dflhdr.next_DFH_offset =0x0;
   fme_pr.ccip_pr_dflhdr.Feature_rev =0;
   fme_pr.ccip_pr_dflhdr.Feature_ID= CCIP_FME_DFLID_PR;
   offset = 0;
   write_ccip_csr64(ptr,offset,fme_pr.ccip_pr_dflhdr.csr);

   // PR control
   fme_pr.ccip_fme_pr_control.pr_start_req =0x1;
   fme_pr.ccip_fme_pr_control.pr_regionid =0x1;
   fme_pr.ccip_fme_pr_control.pr_port_access =0x1;
   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,fme_pr.ccip_fme_pr_control.csr);

   // PR status
   fme_pr.ccip_fme_pr_status.pr_status =0x5;
   fme_pr.ccip_fme_pr_status.pr_mega_fun_status =0x7;
   fme_pr.ccip_fme_pr_status.pr_engine_error =0x1;
   fme_pr.ccip_fme_pr_status.pr_data_overfw_error =0x1;
   fme_pr.ccip_fme_pr_status.pr_timeout_err =0x1;
   fme_pr.ccip_fme_pr_status.pr_credit =0x55;
   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,fme_pr.ccip_fme_pr_status.csr);

   // PR Data
   fme_pr.ccip_fme_pr_data.pr_data_raw =0x123456;
   offset = offset + OFFSET;
   write_ccip_csr64(ptr,offset,fme_pr.ccip_fme_pr_data.csr);


   return 0;
   PINFO(" ccip_sim_wrt_fme_mmio end\n");

}
int  ccip_sim_wrt_port_mmio(btVirtAddr pkvp_fme_mmio)
{
   struct CCIP_PORT_HDR         port_hdr;
   struct CCIP_PORT_DFL_ERR     port_err;
   struct CCIP_PORT_DFL_UMSG    port_umsg;
   struct CCIP_PORT_DFL_PR      port_pr;
   struct CCIP_PORT_DFL_STAP    port_stap;


   btVirtAddr ptr = pkvp_fme_mmio;
   int offset =0;

   PINFO(" ccip_sim_wrt_port_mmio ENTER\n");

   // Port header
   port_hdr.ccip_port_dfh.Type = CCIP_DFType_afu;
   port_hdr.ccip_port_dfh.next_DFH_offset =0x1000;
   port_hdr.ccip_port_dfh.Feature_rev =0;
   port_hdr.ccip_port_dfh.Feature_ID =1;
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
   port_hdr.ccip_port_next_afu.afu_id_offset=0x0;
   offset = offset + OFFSET;
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
   port_hdr.ccip_port_control.ccip_outstaning_request =0x1;
   port_hdr.ccip_port_control.afu_latny_rep =0x1;
   port_hdr.ccip_port_control.port_freeze =0x1;
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
   write_ccip_csr64(ptr,offset, port_err.ccip_port_err_dflhdr.csr);


   //Port Error Mask
   offset = offset + OFFSET;
   port_err.ccip_port_error_mask.rsvd =0;
   write_ccip_csr64(ptr,offset, port_err.ccip_port_error_mask.csr);

   //Port Error Mask
   offset = offset + OFFSET;
   port_err.ccip_port_error.tx_cfg_status =0x0;
   port_err.ccip_port_error.tx_channel0_encode =0x1;
   port_err.ccip_port_error.tx_channel0_invalidreq =0x1;
   port_err.ccip_port_error.tx_channel0_overflow =0x1;
   port_err.ccip_port_error.tx_channel0_sop =0x1;
   port_err.ccip_port_error.tx_channel1_encode =0x1;
   port_err.ccip_port_error.tx_channel1_incorrect =0x1;
   port_err.ccip_port_error.tx_channel1_invalidreq =0x1;
   port_err.ccip_port_error.tx_channel1_overflow =0x1;
   port_err.ccip_port_error.tx_channel1_payloadinsuff =0x1;
   port_err.ccip_port_error.tx_channel1_ploverrun =0x1;
   write_ccip_csr64(ptr,offset, port_err.ccip_port_error.csr);

   //Port first error
   offset = offset + OFFSET;
   port_err.ccip_port_first_error.tx_cfg_status =0x1;
   port_err.ccip_port_first_error.tx_channel0_encode =0x1;
   port_err.ccip_port_first_error.tx_channel0_invalidreq =0x1;
   port_err.ccip_port_first_error.tx_channel0_overflow =0x1;
   port_err.ccip_port_first_error.tx_channel0_sop =0x1;
   port_err.ccip_port_first_error.tx_channel1_encode =0x1;
   port_err.ccip_port_first_error.tx_channel1_incorrect =0x1;
   port_err.ccip_port_first_error.tx_channel1_invalidreq =0x1;
   port_err.ccip_port_first_error.tx_channel1_overflow =0x1;
   port_err.ccip_port_first_error.tx_channel1_payloadinsuff =0x1;
   port_err.ccip_port_first_error.tx_channel1_ploverrun =0x1;
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
   write_ccip_csr64(ptr,offset,port_umsg.ccip_port_umsg_dflhdr.csr);


   // USMG capability
   offset = offset + OFFSET;
   port_umsg.ccip_umsg_capability.no_umsg_alloc_port =0x7;
   port_umsg.ccip_umsg_capability.status_usmg_engine =0x1;
   port_umsg.ccip_umsg_capability.umsg_init_satus =0x1;
   write_ccip_csr64(ptr,offset,port_umsg.ccip_umsg_capability.csr);

   //USMG base address
   offset = offset + OFFSET;
   port_umsg.ccip_umsg_base_address.umsg_base_address =0xfff0101;
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
   write_ccip_csr64(ptr,offset,port_stap.ccip_port_stap_dflhdr.csr);


   // SiganlTap CSR
   offset = offset + OFFSET;
   port_stap.ccip_port_stap.rsvd =0;
   write_ccip_csr64(ptr,offset,port_stap.ccip_port_stap.csr);

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

      PDEBUG( "scratch_pad= %lx \n",(long unsigned int)pfme_dev->m_pHDR->fme_scratchpad.scratch_pad);


      PDEBUG( "fabric_verid= %x \n",pfme_dev->m_pHDR->ccip_fme_capability.fabric_verid);
      PDEBUG( "socket_id= %x \n",pfme_dev->m_pHDR->ccip_fme_capability.socket_id);
      PDEBUG( "pci0_link_avile= %x \n",pfme_dev->m_pHDR->ccip_fme_capability.pci0_link_avile);
      PDEBUG( "pci1_link_avile= %x \n",pfme_dev->m_pHDR->ccip_fme_capability.pci1_link_avile);
      PDEBUG( "qpi_link_avile= %x \n",pfme_dev->m_pHDR->ccip_fme_capability.qpi_link_avile);
      PDEBUG( "iommu_support= %x \n",pfme_dev->m_pHDR->ccip_fme_capability.iommu_support);

      PDEBUG( "address_width_bits= %x \n",pfme_dev->m_pHDR->ccip_fme_capability.address_width_bits);
      PDEBUG( "cache_size= %x \n",pfme_dev->m_pHDR->ccip_fme_capability.cache_size);
      PDEBUG( "cache_assoc= %x \n",pfme_dev->m_pHDR->ccip_fme_capability.cache_assoc);
      PDEBUG( "lock_bit= %x \n",pfme_dev->m_pHDR->ccip_fme_capability.lock_bit);

      PDEBUG( "FME Header END \n \n");

   }

   // Print list of ports
   for ( i=0;i<5;i++)
   {
      PDEBUG( "i = %d \n",i);
      if(0 != pfme_dev->m_pHDR->port_offsets[i].port_imp)
      {
         PDEBUG( "PORT count = %d \n",i);
         PDEBUG( "port_imp = %x \n",pfme_dev->m_pHDR->port_offsets[i].port_imp);
         PDEBUG( "port_arbit_poly = %x \n",pfme_dev->m_pHDR->port_offsets[i].port_arbit_poly);
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


      PDEBUG( "force_proc_hot = %x \n",pfme_dev->m_pThermmgmt->ccip_tmp_threshold.force_proc_hot);
      PDEBUG( "proc_hot_setpoint = %x \n",pfme_dev->m_pThermmgmt->ccip_tmp_threshold.proc_hot_setpoint);
      PDEBUG( "setproc_temp_reach_setpoint = %x \n",pfme_dev->m_pThermmgmt->ccip_tmp_threshold.setproc_temp_reach_setpoint);
      PDEBUG( "therm_proc_hot = %x \n",pfme_dev->m_pThermmgmt->ccip_tmp_threshold.therm_proc_hot);
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

      PDEBUG( "fpga_latency_report = %x \n",pfme_dev->m_pPowermgmt->ccip_pm_threshold.fpga_latency_report);
      PDEBUG( "threshold1 = %x \n",pfme_dev->m_pPowermgmt->ccip_pm_threshold.threshold1);
      PDEBUG( "threshold1_sts = %x \n",pfme_dev->m_pPowermgmt->ccip_pm_threshold.threshold1_sts);
      PDEBUG( "threshold2 = %x \n",pfme_dev->m_pPowermgmt->ccip_pm_threshold.threshold2);
      PDEBUG( "threshold2_sts = %x \n",pfme_dev->m_pPowermgmt->ccip_pm_threshold.threshold2_sts);


      PDEBUG( "clock_buffer_supply_voltvalid = %x \n",pfme_dev->m_pPowermgmt->ccip_pm_rdvr.clock_buffer_supply_voltvalid);
      PDEBUG( "core_supply_voltvalid = %x \n",pfme_dev->m_pPowermgmt->ccip_pm_rdvr.core_supply_voltvalid);
      PDEBUG( "trans_supply_voltvalid = %x \n",pfme_dev->m_pPowermgmt->ccip_pm_rdvr.trans_supply_voltvalid);
      PDEBUG( "fpga_supply_voltvalid = %x \n",pfme_dev->m_pPowermgmt->ccip_pm_rdvr.fpga_supply_voltvalid);
      PDEBUG( "clock_buffer_supply_voltvalue = %d \n",pfme_dev->m_pPowermgmt->ccip_pm_rdvr.clock_buffer_supply_voltvalue);
      PDEBUG( "core_supply_voltvalue = %d \n",pfme_dev->m_pPowermgmt->ccip_pm_rdvr.core_supply_voltvalue);
      PDEBUG( "trans_supply_voltvalue = %d \n",pfme_dev->m_pPowermgmt->ccip_pm_rdvr.trans_supply_voltvalue);

      PDEBUG( "fpga_supply_voltvalue = %d \n",pfme_dev->m_pPowermgmt->ccip_pm_rdvr.fpga_supply_voltvalue);
      PDEBUG( "volt_regulator_readmods = %d \n",pfme_dev->m_pPowermgmt->ccip_pm_rdvr.volt_regulator_readmods);
      PDEBUG( "volt_regulator_readmod_value = %d \n",pfme_dev->m_pPowermgmt->ccip_pm_rdvr.volt_regulator_readmod_value);


      PDEBUG( "hw_set_field = %x \n",pfme_dev->m_pPowermgmt->ccip_pm_mrdvr.hw_set_field);
      PDEBUG( "max_clock_supply_voltrec = %d \n",pfme_dev->m_pPowermgmt->ccip_pm_mrdvr.max_clock_supply_voltrec);
      PDEBUG( "max_core_supply_voltrec = %d \n",pfme_dev->m_pPowermgmt->ccip_pm_mrdvr.max_core_supply_voltrec);
      PDEBUG( "max_fpga_supply_voltrec = %d \n",pfme_dev->m_pPowermgmt->ccip_pm_mrdvr.max_fpga_supply_voltrec);
      PDEBUG( "max_trans_supply_voltrec = %d \n",pfme_dev->m_pPowermgmt->ccip_pm_mrdvr.max_trans_supply_voltrec);

      PDEBUG( "FME Power Feature  END \n \n");

   }

   if(pfme_dev->m_pPerf)
   {

      PDEBUG( "FME   Global Performance START \n \n");

      PDEBUG( "Feature_ID = %x \n",pfme_dev->m_pPerf->ccip_fme_fpmon_dflhdr.Feature_ID);
      PDEBUG( "Feature_rev = %x \n",pfme_dev->m_pPerf->ccip_fme_fpmon_dflhdr.Feature_rev);
      PDEBUG( "Type = %x \n",pfme_dev->m_pPerf->ccip_fme_fpmon_dflhdr.Type);
      PDEBUG( "next_DFH_offset = %x \n",pfme_dev->m_pPerf->ccip_fme_fpmon_dflhdr.next_DFH_offset);

      PDEBUG( "cache_event = %x \n",pfme_dev->m_pPerf->ccip_fpmon_ch_ctl.cache_event);
      PDEBUG( "freeze = %x \n",pfme_dev->m_pPerf->ccip_fpmon_ch_ctl.freeze);
      PDEBUG( "reset_counter = %x \n",pfme_dev->m_pPerf->ccip_fpmon_ch_ctl.reset_counter);

      PDEBUG( "cache_counter = %x \n",( unsigned int)pfme_dev->m_pPerf->ccip_fpmon_ch_ctr_0.cache_counter);

      PDEBUG( "cache_counter = %x \n",( unsigned int)pfme_dev->m_pPerf->ccip_fpmon_ch_ctr_1.cache_counter);

      PDEBUG( "ccip_port_filter = %x \n",pfme_dev->m_pPerf->ccip_fpmon_fab_ctl.ccip_port_filter);
      PDEBUG( "port_id = %x \n",pfme_dev->m_pPerf->ccip_fpmon_fab_ctl.port_id);
      PDEBUG( "fabric_evt_code = %x \n",pfme_dev->m_pPerf->ccip_fpmon_fab_ctl.fabric_evt_code);
      PDEBUG( "freeze = %x \n",pfme_dev->m_pPerf->ccip_fpmon_fab_ctl.freeze);
      PDEBUG( "reset_counter = %x \n",pfme_dev->m_pPerf->ccip_fpmon_fab_ctl.reset_counter);

      PDEBUG( "reset_counter = %x \n",pfme_dev->m_pPerf->ccip_fpmon_fab_ctl.reset_counter);

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
      PDEBUG( "ccip_fme_error_mask rsvd = %x \n",( unsigned int)pfme_dev->m_pGerror->ccip_fme_error_mask.rsvd);
      PDEBUG( "ccip_fme_first_error rsvd = %x \n",( unsigned int)pfme_dev->m_pGerror->ccip_fme_first_error.rsvd);
      PDEBUG( "ccip_fme_error rsvd = %x \n",( unsigned int)pfme_dev->m_pGerror->ccip_fme_error.rsvd);
      PDEBUG( "FME   Global Error END \n \n");

   }

   if(pfme_dev->m_pPRmgmt)
   {
      PDEBUG( "FME PR Feature  START \n \n");

      PDEBUG( "Feature_ID = %x \n",pfme_dev->m_pPRmgmt->ccip_pr_dflhdr.Feature_ID);
      PDEBUG( "Feature_rev = %x \n",pfme_dev->m_pPRmgmt->ccip_pr_dflhdr.Feature_rev);
      PDEBUG( "Type = %x \n",pfme_dev->m_pPRmgmt->ccip_pr_dflhdr.Type);
      PDEBUG( "next_DFH_offset = %x \n",pfme_dev->m_pPRmgmt->ccip_pr_dflhdr.next_DFH_offset);

      PDEBUG( "pr_start_req = %x \n",pfme_dev->m_pPRmgmt->ccip_fme_pr_control.pr_start_req);
      PDEBUG( "pr_regionid = %x \n",pfme_dev->m_pPRmgmt->ccip_fme_pr_control.pr_regionid);
      PDEBUG( "pr_port_access = %x \n",pfme_dev->m_pPRmgmt->ccip_fme_pr_control.pr_port_access);

      PDEBUG( "pr_status = %x \n",pfme_dev->m_pPRmgmt->ccip_fme_pr_status.pr_status);

      PDEBUG( "pr_mega_fun_status = %x \n",pfme_dev->m_pPRmgmt->ccip_fme_pr_status.pr_mega_fun_status);
      PDEBUG( "pr_engine_error = %x \n",pfme_dev->m_pPRmgmt->ccip_fme_pr_status.pr_engine_error);
      PDEBUG( "pr_data_overfw_error = %x \n",pfme_dev->m_pPRmgmt->ccip_fme_pr_status.pr_data_overfw_error);
      PDEBUG( "pr_timeout = %x \n",pfme_dev->m_pPRmgmt->ccip_fme_pr_status.pr_timeout_err);
      PDEBUG( "pr_credit = %x \n",pfme_dev->m_pPRmgmt->ccip_fme_pr_status.pr_credit);

      PDEBUG( "pr_data_raw = %x \n",pfme_dev->m_pPRmgmt->ccip_fme_pr_data.pr_data_raw);

      PDEBUG( "FME PR Feature  END \n \n");
   }

   PDEBUG( "*****************************************************************\n");
   PDEBUG( "*****************************************************************\n");

   EROR:

   return res;
}

int print_sim_port_device(struct port_device *pport_dev)
{
   int res;
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


      PDEBUG( "afu_id_l.afu_id_l= %lx \n",( long unsigned int)pport_dev->m_pport_hdr->ccip_port_afuidl.afu_id_l);
      PDEBUG( "afu_id_h.afu_id_h= %lx \n",( long unsigned int)pport_dev->m_pport_hdr->ccip_port_afuidh.afu_id_h);

      PDEBUG( "next_afu.afu_id_offset= %x \n",pport_dev->m_pport_hdr->ccip_port_next_afu.afu_id_offset);

      PDEBUG( "scratch_pad= %lx \n",( long unsigned int)pport_dev->m_pport_hdr->ccip_port_scratchpad.scratch_pad);

      PDEBUG( "interrupts= %x \n",pport_dev->m_pport_hdr->ccip_port_capability.interrupts);
      PDEBUG( "mmio_size= %x \n",pport_dev->m_pport_hdr->ccip_port_capability.mmio_size);
      PDEBUG( "port_id= %x \n",pport_dev->m_pport_hdr->ccip_port_capability.port_id);
      // PDEBUG( "usmg_size= %x \n",pport_dev->m_pport_hdr->ccip_port_capability.usmg_size);

      PDEBUG( "ccip_outstaning_request= %x \n",pport_dev->m_pport_hdr->ccip_port_control.ccip_outstaning_request);
      PDEBUG( "afu_latny_rep= %x \n",pport_dev->m_pport_hdr->ccip_port_control.afu_latny_rep);
      PDEBUG( "port_freeze= %x \n",pport_dev->m_pport_hdr->ccip_port_control.port_freeze);
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

      PDEBUG( "rsvd = %x \n",(  unsigned int)pport_dev->m_pport_err->ccip_port_error_mask.rsvd);

      PDEBUG( "tx_cfg_status = %x \n",pport_dev->m_pport_err->ccip_port_error.tx_cfg_status);

      PDEBUG( "tx_channel0_invalidreq = %x \n",pport_dev->m_pport_err->ccip_port_error.tx_channel0_invalidreq);
      PDEBUG( "tx_channel0_overflow = %x \n",pport_dev->m_pport_err->ccip_port_error.tx_channel0_overflow);
      PDEBUG( "tx_channel0_sop = %x \n",pport_dev->m_pport_err->ccip_port_error.tx_channel0_sop);
      PDEBUG( "tx_channel1_encode = %x \n",pport_dev->m_pport_err->ccip_port_error.tx_channel1_encode);
      PDEBUG( "tx_channel1_incorrect = %x \n",pport_dev->m_pport_err->ccip_port_error.tx_channel1_incorrect);
      PDEBUG( "tx_channel1_invalidreq = %x \n",pport_dev->m_pport_err->ccip_port_error.tx_channel1_invalidreq);
      PDEBUG( "tx_channel1_overflow = %x \n",pport_dev->m_pport_err->ccip_port_error.tx_channel1_overflow);
      PDEBUG( "tx_channel0_encode = %x \n",pport_dev->m_pport_err->ccip_port_error.tx_channel0_encode);
      PDEBUG( "tx_channel1_payloadinsuff = %x \n",pport_dev->m_pport_err->ccip_port_error.tx_channel1_payloadinsuff);
      PDEBUG( "tx_channel1_ploverrun = %x \n",pport_dev->m_pport_err->ccip_port_error.tx_channel1_ploverrun);

      PDEBUG( "tx_cfg_status = %x \n",pport_dev->m_pport_err->ccip_port_first_error.tx_cfg_status);

      PDEBUG( "tx_channel0_invalidreq = %x \n",pport_dev->m_pport_err->ccip_port_first_error.tx_channel0_invalidreq);
      PDEBUG( "tx_channel0_overflow = %x \n",pport_dev->m_pport_err->ccip_port_first_error.tx_channel0_overflow);
      PDEBUG( "tx_channel0_sop = %x \n",pport_dev->m_pport_err->ccip_port_first_error.tx_channel0_sop);
      PDEBUG( "tx_channel1_encode = %x \n",pport_dev->m_pport_err->ccip_port_first_error.tx_channel1_encode);
      PDEBUG( "tx_channel1_incorrect = %x \n",pport_dev->m_pport_err->ccip_port_first_error.tx_channel1_incorrect);
      PDEBUG( "tx_channel1_invalidreq = %x \n",pport_dev->m_pport_err->ccip_port_first_error.tx_channel1_invalidreq);
      PDEBUG( "tx_channel1_overflow = %x \n",pport_dev->m_pport_err->ccip_port_first_error.tx_channel1_overflow);
      PDEBUG( "tx_channel0_encode = %x \n",pport_dev->m_pport_err->ccip_port_first_error.tx_channel0_encode);
      PDEBUG( "tx_channel1_payloadinsuff = %x \n",pport_dev->m_pport_err->ccip_port_first_error.tx_channel1_payloadinsuff);
      PDEBUG( "tx_channel1_ploverrun = %x \n",pport_dev->m_pport_err->ccip_port_first_error.tx_channel1_ploverrun);


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


      PDEBUG( "no_umsg_alloc_port = %x \n",pport_dev->m_pport_umsg->ccip_umsg_capability.no_umsg_alloc_port);
      PDEBUG( "status_usmg_engine = %x \n",pport_dev->m_pport_umsg->ccip_umsg_capability.status_usmg_engine);
      PDEBUG( "umsg_init_satus = %x \n",pport_dev->m_pport_umsg->ccip_umsg_capability.umsg_init_satus);


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

      PDEBUG( "Signal tap rsvd = %x \n",(  unsigned int)pport_dev->m_pport_stap->ccip_port_stap.rsvd);

      PDEBUG( "PORT SIGNAL tap  END \n \n");

   }

   PDEBUG( "*****************************************************************\n");
   PDEBUG( "*****************************************************************\n");

   ERR:

   return res;

}



