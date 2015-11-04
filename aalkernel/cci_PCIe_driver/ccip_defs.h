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
/// @file ccip_defs.h
/// @brief  Definitions for CCI-P compliant devices.
/// @ingroup aalkernel_ccip
/// @verbatim
//        FILE: ccip_defs.h
//     CREATED: Oct 28, 2015
//      AUTHOR: Ananda Ravuri, Intel <ananda.ravuri@intel.com>
//              Joseph Grecco, Intel <joe.grecco@intel.com>
//
// PURPOSE:   This file contains the definitions for the CCI-P compliant FPGA
//            devices.
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 10/28/15       JG       Created from prototype written by AR
//****************************************************************************
#ifndef __AALKERNEL_CCIP_DEFS_H_
#define __AALKERNEL_CCIP_DEFS_H_
#include "aalsdk/kernel/kosal.h"

#if !defined(MODULE_FLAGS)
#define MODULE_FLAGS CCIPCIE_DBG_MOD // Prints all
#endif

#include <aalsdk/kernel/aaltypes.h>
#include "cci_pcie_driver_internal.h"

BEGIN_NAMESPACE(AAL)


/// @addtogroup aalkernel_ccip
/// @{

/// MMIO space Size
/// Simulation  mmio size
#define CCIP_MMIO_SIZE              ( 0x120000 )

// Max number of pci bars to scan
#define CCIP_MAX_PCIBAR             5

///=================================================================
/// Enumerations
///=================================================================

/// CCIP Device type ID
enum e_CCIP_DEVTPPE_ID {
   CCIP_DFType_rsvd      = 0x0,
   CCIP_DFType_afu       = 0x1,
   CCIP_DFType_bbb       = 0x2,
   CCIP_DFType_private   = 0x3
};

// FPGA Management Engine (FME)
//-----------------------------

// Device Feature ID
enum e_CCIP_DFL_ID {
   CCIP_FME_DFLID_THERM  = 0x1,
   CCIP_FME_DFLID_POWER  = 0x2,
   CCIP_FME_DFLID_GPERF  = 0x3,
   CCIP_FME_DFLID_GERR   = 0x4,
   CCIP_FME_DFLID_PR     = 0x5,
   CCIP_PORT_DFLID_ERROR = 0x10,
   CCIP_PORT_DFLID_USMG  = 0x11,
   CCIP_PORT_DFLID_PR    = 0x12,
   CCIP_PORT_DFLID_STP   = 0x13
};

// Device Feature revision ID
enum e_CCIP_DFL_rev {
   CCIP_DFL_rev0    = 0x0,
   CCIP_DFL_rev1    = 0x1,
   CCIP_DFL_rev2    = 0x2,
   CCIP_DFL_rev3    = 0x3,
   CCIP_DFL_rev4    = 0x4
};


// AFU power states
enum e_AFU_Power_State {
   AFU_Power_Normal = 0x0,
   AFU_Power_AP1 = 0x1,
   AFU_Power_AP2 = 0x2,
   AFU_Power_AP6 = 0x6
};

// Cache Event codes
enum e_Cache_Event_Code {
   Cache_Read_Hit = 0x0,
   Cache_Write_Hit = 0x1,
   Cache_Read_Miss = 0x2,
   Cache_Write_Miss = 0x3,
   Cache_Rsvd = 0x4,
   Cache_Hold_Req = 0x5,
   Cache_Data_WrtPort_Conten = 0x6,
   Cache_Tag_WrtPort_Conten = 0x7,
   Cache_Tx_Req_stall = 0x8,
   Cache_Rx_Stalls = 0x9,
   Cache_Evictions = 0xA

};

// Fabric event codes
enum e_Fabric_Event_Code {
   Fabric_PCIe0_Read = 0x0,
   Fabric_PCIe0_Write = 0x1,
   Fabric_PCIe1_Read = 0x2,
   Fabric_PCIe1_Write = 0x3,
   Fabric_UPI_Read = 0x4,
   Fabric_UPI_Write = 0x5
};

/// CCI-P port id
enum e_CCIP_Port_Id {
   CCIP_Port_Id0 = 0x0,
   CCIP_Port_Id1 = 0x1,
   CCIP_Port_Rsvd = 0x2,
   CCIP_Port_Rsvd1 = 0x3
};

/// Partial Reconfiguration ID
enum e_FME_PR_REGION_ID {
   CCIP_PR_Region0 = 0x0,
   CCIP_PR_Region1 = 0x1,
   CCIP_PR_rsvd1 = 0x2,
   CCIP_PR_rsvd2 = 0x3

};

/// Partial Reconfiguration status
enum e_CCIP_PORT_PR_status {
   CCIP_PORT_PR_Idle = 0x0,
   CCIP_PORT_PR_RecStart = 0x1,
   CCIP_PORT_DoingPR1 = 0x2,
   CCIP_PORT_DoingPR2 = 0x3,
   CCIP_PORT_DoingPR3 = 0x4,
   CCIP_PORT_DoingPR4 = 0x5,
   CCIP_PORT_DoingPR5 = 0x6,
   CCIP_PORT_PR_complete = 0x7,
   CCIP_PORT_PR_error = 0x8
};

/// Partial Reconfiguration  mega function status codes
enum e_CCIP_PR_Megafun_status {
   CCIP_PR_MF_pwrup = 0x0,
   CCIP_PR_MF_error = 0x1,
   CCIP_PR_MF_crc_err = 0x2,
   CCIP_PR_MF_Incomp_bts_err = 0x3,
   CCIP_PR_MF_opr_inPros = 0x4,
   CCIP_PR_MF_por_pass = 0x5,
   CCIP_PR_MF_rsvd1 = 0x6,
   CCIP_PR_MF_rsvd2 = 0x7
};


/******************************************************************************
 *  FPGA  Header and CSR
 *
 *   Description:
 *
 ******************************************************************************/

/// Device Feature Header CSR
struct CCIP_DFH {

   union {
      btUnsigned64bitInt csr;
      struct {
         btUnsigned64bitInt Feature_ID :12;     // Feature ID

         //enum e_CCIP_DFL_ID Feature_ID :12;     // Feature ID

         btUnsigned64bitInt Feature_rev :4;     // Feature revision
         btUnsigned64bitInt next_DFH_offset :24;// Next Device Feature header offset
         btUnsigned64bitInt rsvd :20;           // Reserved
         btUnsigned64bitInt Type :4;            // Type of Device

         //enum e_CCIP_DEVTPPE_ID Type :4;

      }; //end struct
   }; // end union

}; //end struct CCIP_DFH
CASSERT(sizeof(struct CCIP_DFH) == 8);

/// AFU ID low CSR
struct CCIP_AFU_ID_L {

   union {
      btUnsigned64bitInt csr;
      struct {
         btUnsigned64bitInt afu_id_l :64; // AFU ID low

      }; // end struct
   }; // end union

}; //end struct CCIP_AFU_ID_L
CASSERT(sizeof(struct CCIP_AFU_ID_L) == 8);

/// AFU ID high CSR
struct CCIP_AFU_ID_H {

   union {
      btUnsigned64bitInt csr;
      struct {
         btUnsigned64bitInt afu_id_h :64; // AFU ID low

      }; // end struct
   }; // end union

}; //end struct CCIP_AFU_ID_L
CASSERT(sizeof(struct CCIP_AFU_ID_H) == 8);


/// Next AFU offset CSR
struct CCIP_NEXT_AFU {

   union {
      btUnsigned64bitInt csr;
      struct {
         btUnsigned64bitInt afu_id_offset :24;   // Next AFU DFH byte offset
         btUnsigned64bitInt rsvd :40;            // Reserved

      }; // end struct
   }; // end union

}; // end struct CCIP_NEXT_AFU
CASSERT(sizeof(struct CCIP_NEXT_AFU) == 8);

/// FME Scratch pad CSR
struct CCIP_SCRATCHPAD {
   union {
      btUnsigned64bitInt csr;
      struct {
         btUnsigned64bitInt scratch_pad :64; //Scratch pad Register

      }; // end struct
   }; // end union

}; // end struct CCIP_SCRATCHPAD
CASSERT(sizeof(struct CCIP_SCRATCHPAD) == 8);

/// FPGA Port offset CSR
struct CCIP_PORT_AFU_OFFSET {

   union {
      btUnsigned64bitInt csr;
      struct {
         btUnsigned64bitInt port_offset :24; // Port offset
         btUnsigned64bitInt rsvd1 :8; // Reserved
         btUnsigned64bitInt port_bar :3; // Port BAR
         btUnsigned64bitInt rsvd2 :21; // Reserved
         btUnsigned64bitInt port_arbit_poly :4; // Port Arbitration
         btUnsigned64bitInt port_imp :1; // Port Implemented
         btUnsigned64bitInt rsvd3 :3; // Reserved
      }; // end struct
   }; // end union

}; // end struct CCIP_PORT_AFU_OFFSET
CASSERT(sizeof(struct CCIP_PORT_AFU_OFFSET) == 8);



/******************************************************************************
 *  FPGA Management Engine  Header
 *  CCI-P version =1
 *  CCI-P minor version =0
 *  Type = afu
 *  Header Description:
 *
 ******************************************************************************/
///============================================================================
/// Name: CCIP_FME_HDR
/// @brief   FPGA Management Engine  Header
///  Version: 1
///  Type:  Private
///  Feature: AFU
///============================================================================
struct CCIP_FME_HDR {

   // FME  Header
   struct CCIP_DFH         dfh;               // Offset 0

   // FME afu id  low
   struct CCIP_AFU_ID_L    afu_id_l;         // Offset 0x8

   // FME afu id  high
   struct CCIP_AFU_ID_H    afu_id_h;         // Offset 0x10

   // Next AFU offset
   struct CCIP_NEXT_AFU    next_afu;         // Offset 0x18

   // Reserved
   btUnsigned64bitInt      rsvd_fmehdr;      // Offset 0x20

   // FME Scratch pad
   struct CCIP_SCRATCHPAD  fme_scratchpad;

   // Fabric capability
   struct CCIP_FAB_CAPABILITY {

      union {
         btUnsigned64bitInt csr;
         struct {
            btUnsigned64bitInt fabric_verid :8; // Fabric version ID
            btUnsigned64bitInt socket_id :1; // Socket id
            btUnsigned64bitInt rsvd1 :3;
            btUnsigned64bitInt pci0_link_avile :1; // pci0 link available yes /no
            btUnsigned64bitInt pci1_link_avile :1; // pci1 link available yes /no
            btUnsigned64bitInt qpi_link_avile :1; //Coherent (QPI/UPI) link available yes /no
            btUnsigned64bitInt rsvd2 :1;
            btUnsigned64bitInt iommu_support :1; //IOMMU or VT-d supported  yes/no
            btUnsigned64bitInt rsvd3 :7;
            btUnsigned64bitInt address_width_bits :6; // Address width supported in bits  BXT -0x26 , SKX -0x30
            btUnsigned64bitInt rsvd4 :2;
            btUnsigned64bitInt cache_size :12; // Size of cache supported in kb
            btUnsigned64bitInt cache_assoc :4; // Cache Associativity
            btUnsigned64bitInt rsvd5 :14;
            btUnsigned64bitInt lock_bit :1; //Lock bit
         }; // end struct
      }; // end union

   } ccip_fme_capability; // struct CCIP_FME_CAPABILITY

   // Beginning of the list of offsets to the Port regions
   struct CCIP_PORT_AFU_OFFSET port_offsets[1];

}; //end struct CCIP_FME_HDR
CASSERT(sizeof(struct CCIP_FME_HDR) == (8 * 8));

///============================================================================
/// Name: CCIP_FME_DFL_THERM
/// @brief   FPGA Management Engine  Thermal  Management Feature
///  Feature ID: 0x1
///  Feature Type: = Private
///  Feature Revision: 0
///============================================================================
struct CCIP_FME_DFL_THERM {

   // Thermal  Management Feature Header
   struct CCIP_DFH  ccip_fme_tmp_dflhdr ;

   // Temperature Threshold
   struct CCIP_TEMP_THRESHOLD {

      union {
         btUnsigned64bitInt csr;
         struct {
            btUnsigned64bitInt tmp_thshold1 :7; // temperature Threshold 1
            btUnsigned64bitInt tmp_thshold1_status :1; // temperature Threshold 1 enable /disable
            btUnsigned64bitInt tmp_thshold2 :7; // temperature Threshold 2
            btUnsigned64bitInt tmp_thshold2_status :1; // temperature Threshold 2 enable /disable
            btUnsigned64bitInt proc_hot_setpoint :7; // proc hot set point
            btUnsigned64bitInt rsvd4 :1;
            btUnsigned64bitInt therm_trip_thshold :7; // Thermeal Trip Threshold
            btUnsigned64bitInt rsvd3 :1;
            btUnsigned64bitInt thshold1_status :1; // Threshold 1 Status
            btUnsigned64bitInt thshold2_status :1; // Threshold 2 Status
            btUnsigned64bitInt setproc_temp_reach_setpoint :1; // Set temperature reaches proc hot set point
            btUnsigned64bitInt therm_trip_thshold_status :1; //  Thermeal Trip Threshold status
            btUnsigned64bitInt rsvd2 :4;
            btUnsigned64bitInt force_proc_hot :1; //  force proc hot
            btUnsigned64bitInt therm_proc_hot :1; //  Thermeal Trip Hot
            btUnsigned64bitInt rsvd1 :2;
            btUnsigned64bitInt thshold_policy :1; // threshold policy

            // enum e_AFU_Power_State thshold_policy :1; // threshold policy;

            btUnsigned64bitInt rsvd :19;
         }; //end struct
      }; // end union

   }ccip_tmp_threshold; // end struct CCIP_TMP_THRESHOLD

   // Temperature Sensor Read values
   struct CCIP_TEMP_RDSSENSOR_FMT1 {

      union {
         btUnsigned64bitInt csr;
         struct {
            btUnsigned64bitInt tmp_reading :7; // Reads out FPGA temperature in celsius.
            btUnsigned64bitInt rsvd2 :1;
            btUnsigned64bitInt tmp_reading_seq_num :16; // Temperature reading sequence number
            btUnsigned64bitInt tmp_reading_valid :1; // Temperature reading is valid
            btUnsigned64bitInt rsvd1 :7;
            btUnsigned64bitInt dbg_mode :8; //Debug mode
            btUnsigned64bitInt rsvd :24;

         }; // end struct
      } ; // end union

   }ccip_tmp_rdssensor_fm1; // end struct CCIP_TMP_RDSSENSOR_FMT1

   // Temperature sensor read values
   struct CCIP_TEMP_RDSSENSOR_FMT2 {

      union {
         btUnsigned64bitInt csr;
         struct {
            btUnsigned64bitInt rsvd :64;  // TBD
         }; // end struct
      }; // end union

   }ccip_tmp_rdssensor_fm2; // end struct CCIP_TMP_RDSSENSOR_FMT2

}; //end struct CCIP_FME_TMP_DFL
CASSERT(sizeof(struct CCIP_FME_DFL_THERM) == (4*8));


///============================================================================
/// Name: CCIP_FME_DFL_PM
/// @brief   FPGA Management Engine Power Management Feature
///  Feature ID: 0x2
///  Feature Type: = Private
///  Feature Revision: 0
///============================================================================
struct CCIP_FME_DFL_PM {

   // FME Power Management Feature header
   struct CCIP_DFH  ccip_fme_pm_dflhdr;

   //Power Management threshold
   struct CCIP_PM_THRESHOLD {

      union {
         btUnsigned64bitInt csr;
         struct {
            btUnsigned64bitInt threshold1 :7; //threshold 1
            btUnsigned64bitInt rsvd1 :1;
            btUnsigned64bitInt threshold2 :7; //threshold 2
            btUnsigned64bitInt rsvd2 :1;
            btUnsigned64bitInt threshold1_sts :1; // Threshold 1 status
            btUnsigned64bitInt threshold2_sts :1; // Threshold 2 status
            btUnsigned64bitInt fpga_latency_report :1; // FPGA Latency Tolerance Reporting (LTR)
            btUnsigned64bitInt rsvd3 :45;
         }; // end struct
      }; // end union

   } ccip_pm_threshold; // end struct CCIP_PM_THRESHOLD

   //Power Management Record  values
   struct CCIP_PM_RDVR {

      union {
         btUnsigned64bitInt csr;
         struct {
            btUnsigned64bitInt clock_buffer_supply_voltvalid :1; // clock buffer supply voltage valid
            btUnsigned64bitInt core_supply_voltvalid :1; // core  supply voltage valid
            btUnsigned64bitInt trans_supply_voltvalid :1; // transceiver  supply voltage valid
            btUnsigned64bitInt fpga_supply_voltvalid :1; // fpga 1.8v  supply voltage valid
            btUnsigned64bitInt volt_regulator_readmods :1; // Voltage regulator read modes
            btUnsigned64bitInt rsvd :3;
            btUnsigned64bitInt clock_buffer_supply_voltvalue :8; // clock buffer supply voltage value
            btUnsigned64bitInt core_supply_voltvalue :16; // core  supply voltage value
            btUnsigned64bitInt trans_supply_voltvalue :8; // transceiver  supply voltage value
            btUnsigned64bitInt fpga_supply_voltvalue :8; // fpga   supply voltage value
            btUnsigned64bitInt volt_regulator_readmod_value :16; // read sample  sequence number
         }; // end struct
      }; // end union

   } ccip_pm_rdvr; // end struct CCIP_PM_RDVR

   //Power Management Record Maximum values
   struct CCIP_PM_MAXVR {

      union {
         btUnsigned64bitInt csr;
         struct {
            btUnsigned64bitInt hw_set_field :1; //Hardware set field
            btUnsigned64bitInt rsvd :7;
            btUnsigned64bitInt max_clock_supply_voltrec :8; // Maximum clock buffer supply voltage recorded
            btUnsigned64bitInt max_core_supply_voltrec :8; // Maximum core  supply voltage recorded
            btUnsigned64bitInt max_trans_supply_voltrec :8; // Maximum Transceiver  supply voltage recorded
            btUnsigned64bitInt max_fpga_supply_voltrec :8; // Maximum FPGA  supply voltage recorded
            btUnsigned64bitInt rsvd1 :16;
         }; // end struct
      }; // end union

   }ccip_pm_mrdvr; // end struct CCIP_PM_MAXVR

}; // end struct CCIP_PM_MAXVR
CASSERT(sizeof(struct CCIP_FME_DFL_PM) == (4*8));

///============================================================================
/// Name: CCIP_FME_DFL_FPMON
/// @brief   FPGA Management Engine Performance Monitor Feature
///  Feature ID: 0x3
///  Feature Type: = Private
///  Feature Revision: 0
///============================================================================
struct CCIP_FME_DFL_FPMON {

   //Performance Fabric control Header
   struct CCIP_DFH ccip_fme_fpmon_dflhdr;

   // FPGA performance counters
   struct CCIP_FPMON_CH_CTL {

      union {
         btUnsigned64bitInt csr;
         struct {
            btUnsigned64bitInt reset_counter :1; //Reset counter
            btUnsigned64bitInt rsvd2 :7;
            btUnsigned64bitInt freeze :1; // Freeze
            btUnsigned64bitInt rsvd1 :7;
            btUnsigned64bitInt cache_event :4; // Cache Event code

            //enum  e_Cache_Event_Code  cache_event :4;

            btUnsigned64bitInt rsvd :44;
         }; // end struct
      }; // end union

   }ccip_fpmon_ch_ctl; // end struct CCIP_FPMON_CH_CTL

   // FPGA  Cache port0
   struct CCIP_FPMON_CH_CTR_0 {

      union {
         btUnsigned64bitInt csr;
         struct {
            btUnsigned64bitInt cache_counter :48; //Cache counter
            btUnsigned64bitInt rsvd :16;
         }; // end struct
      }; // end union

   }ccip_fpmon_ch_ctr_0; // end CIP_FPMON_CH_CTR_0

   // FPGA Cache port1
   struct CCIP_FPMON_CH_CTR_1 {

      union {
         btUnsigned64bitInt csr;
         struct {
            btUnsigned64bitInt cache_counter :48; //Cache counter
            btUnsigned64bitInt rsvd :16;
         }; // end struct
      }; // end union

   }ccip_fpmon_ch_ctr_1; // end struct CCIP_FPMON_CH_CTR_1

   // FPGA  Fabric control
   struct CCIP_FPMON_FAB_CTL {

      union {
         btUnsigned64bitInt csr;
         struct {
            btUnsigned64bitInt reset_counter :1; // Reset Counters
            btUnsigned64bitInt rsvd3 :7;
            btUnsigned64bitInt freeze :1; // Freeze
            btUnsigned64bitInt rsvd2 :7;
            btUnsigned64bitInt fabric_evt_code :4; // Fabric Event Code

            // e_Fabric_Event_Code fabric_evt_code :4; // Fabric Event Code

            btUnsigned64bitInt port_id :2; // CCI-P Port ID

            //enum e_CCIP_Port_Id port_id :2; // CCI-P Port ID

            btUnsigned64bitInt rsvd1 :1;
            btUnsigned64bitInt ccip_port_filter :1; // CCI-P Port Filter enable /disbale
            btUnsigned64bitInt rsvd :40;
         }; // end stuct
      }; // end unon

   }ccip_fpmon_fab_ctl; // end struct CCIP_FPMON_FAB_CTL

   // FPGA  Fabric control
   struct CCIP_FPMON_FAB_CTR {

      union {
         btUnsigned64bitInt csr;
         struct {
            btUnsigned64bitInt fabric_counter :48; // Fabric event counter
            btUnsigned64bitInt rsvd :16;
         }; // end struct
      }; //end union

   }ccip_fpmon_fab_ctr; //end struct CCIP_FPMON_FAB_CTR

   // FPGA  Fabric control
   struct CCIP_FPMON_CLK_CTRS {

      union {
         btUnsigned64bitInt csr;
         struct {
            btUnsigned64bitInt afu_interf_clock :64; // AFU interface clock
         }; // end struct
      }; //end union


   }ccip_fpmon_clk_ctrs; // end struct CCIP_FPMON_CLK_CTRS

};
CASSERT(sizeof(struct CCIP_FME_DFL_FPMON) == (7*8));

///============================================================================
/// Name: CCIP_FME_DFL_GERROR
/// @brief   FPGA Management Engine Performance Global Error Feature
///  Feature ID: 0x4
///  Feature Type: = Private
///  Feature Revision: 0
///============================================================================
struct CCIP_FME_DFL_GERROR {

   // FME Global Error header
   struct CCIP_DFH ccip_gerror_dflhdr;

   //FME  Error mask  CSR
   struct CCIP_FME_ERROR_MASK {

      union {
         btUnsigned64bitInt csr;
         struct {
            btUnsigned64bitInt rsvd :64; // TBD
         }; // end struct
      }; // end union

   }ccip_fme_error_mask; //end CCIP_FME_ERROR_MASK


   // FME error CSR
   struct CCIP_FME_ERROR {

      union {
         btUnsigned64bitInt csr;
         struct {
            btUnsigned64bitInt rsvd :64;  // TBD
         }; // end struct
      }; // end union

   } ccip_fme_error; // end struct CCIP_FME_ERROR


   // FME first error CSR
   struct CCIP_FME_FIRST_ERROR {

      union {
         btUnsigned64bitInt csr;
         struct {
            btUnsigned64bitInt rsvd :64; // TBD
         }; //end struct
      }; // end union

   } ccip_fme_first_error; // end struct CCIP_FME_FIRST_ERROR

}; //end CCIP_FME_GERROR_feature
CASSERT(sizeof(struct CCIP_FME_DFL_GERROR) ==(4* 8));

///============================================================================
/// Name: CCIP_FME_DFL_PR
/// @brief   FPGA Management Engine Partial Reconfiguration Feature
///  Feature ID: 0x5
///  Feature Type: = Private
///  Feature Revision: 0
///============================================================================
struct CCIP_FME_DFL_PR {

   // FME PR device feature header
   struct CCIP_DFH ccip_pr_dflhdr;

   //Partial Reconfiguration control  CSR
   struct CCIP_FME_PR_CONTROL {

      union {
         btUnsigned64bitInt csr;
         struct {
            btUnsigned64bitInt pr_port_access :1; // Enable PR Port access
            btUnsigned64bitInt rsvd3 :7;
            btUnsigned64bitInt pr_regionid :2; // PR Region ID

            //enum e_FME_PR_REGION_ID  pr_regionid :2; // PR Region ID

            btUnsigned64bitInt rsvd1 :2;
            btUnsigned64bitInt pr_start_req :1; // PR Start Request
            btUnsigned64bitInt rsvd :51;
         }; // end struct
      }; //end union


   }ccip_fme_pr_control; // end struct CCIP_FME_PR_CONTROL

   //Partial Reconfiguration Status  CSR
   struct CCIP_FME_PR_STATUS {

      union {
         btUnsigned64bitInt csr;
         struct {
            btUnsigned64bitInt pr_credit :9;           // PR Credits
            btUnsigned64bitInt rsvd2 :7;
            btUnsigned64bitInt pr_timeout_err :1;      // PR timeout error
            btUnsigned64bitInt pr_engine_error :1;     //  PR engine Error
            btUnsigned64bitInt pr_data_overfw_error :1; // PR data overflow error
            btUnsigned64bitInt rsvd3 :1;
            btUnsigned64bitInt pr_mega_fun_status :3;    // Altra PR Contoller Block status

            //enum e_CCIP_PR_Megafun_status pr_mega_fun_status :3;

            btUnsigned64bitInt rsvd1 :1;
            btUnsigned64bitInt pr_status :4;

            // enum e_CCIP_PORT_PR_status  pr_status :4;

            btUnsigned64bitInt rsvd :35;
         }; // end struct
      }; // end union


   }ccip_fme_pr_status; // end struct CCIP_FME_PR_STATUS

   //Partial Reconfiguration data  CSR
   struct CCIP_FME_PR_DATA {
      union {
         btUnsigned64bitInt csr;
         struct {
            btUnsigned64bitInt pr_data_raw :32;  // PR data from the raw binary file
            btUnsigned64bitInt rsvd :32;
         }; // end struct
      }; // end union


   }ccip_fme_pr_data; // end struct CCIP_FME_PR_DATA

}; //end struct  CCIP_FME_GERROR_feature
CASSERT(sizeof(struct CCIP_FME_DFL_PR) ==(4* 8));

/******************************************************************************
 *  FPGA port header
 *  Type  = afu
 *  CCI-P version =0x1
 *  CCI-P minor version =0x0
 *  Header Description:
 *
 ******************************************************************************/

// Port status
struct CCIP_PORT_HDR {

   // port  header
   struct CCIP_DFH ccip_port_dfh;


   // Port afu id  low
   struct CCIP_AFU_ID_L ccip_port_afuidl;

   // Port afu id  low
   struct CCIP_AFU_ID_H  ccip_port_afuidh;

   // Next AFU offset
   struct CCIP_NEXT_AFU  ccip_port_next_afu;

   // Reserved
   btUnsigned64bitInt rsvd_porthdr;

   // Port Scratch pad
   struct CCIP_SCRATCHPAD ccip_port_scratchpad;

   //  Port scratch pad
   struct CCIP_PORT_CAPABILITY {

      union {
         btUnsigned64bitInt csr;
         struct {
            btUnsigned64bitInt port_id :2;   // CCI-P port ID
            btUnsigned64bitInt rsvd1 :6;
            btUnsigned64bitInt mmio_size :16; // MMIO size
            btUnsigned64bitInt rsvd3 :8;
            btUnsigned64bitInt interrupts :4; //Interrupts supported
            btUnsigned64bitInt rsvd :28;
         }; // end struct
      }; // end union

   }ccip_port_capability; // end  CCIP_PORT_CAPABILITY

   // Port control
   struct CCIP_PORT_CONTROL {
      union {
         btUnsigned64bitInt csr;
         struct {
            btUnsigned64bitInt port_sftreset_control :1; //port soft reset control
            btUnsigned64bitInt port_freeze :1; //  Port Freeze
            btUnsigned64bitInt afu_latny_rep :1; // AFU Latency Tolerance Reorting
            btUnsigned64bitInt rsvd1 :1;
            btUnsigned64bitInt ccip_outstaning_request :1; // NO outstanding CCI-P requests, set to 1
            btUnsigned64bitInt rsvd :59;
         }; // end struct
      }; // end union

   }ccip_port_control; // end struct CCIP_PORT_CONTROL

   // Port status
   struct CCIP_PORT_STATUS {
      union {
         btUnsigned64bitInt csr;
         struct {
            btUnsigned64bitInt rsvd1 :8;
            btUnsigned64bitInt afu_pwr_state :4; //Reports AFU Power state

            // enum e_AFU_Power_State afu_pwr_state :4; //Reports AFU Power state

            btUnsigned64bitInt rsvd :48;
         }; // end struct
      }; // end union

   }ccip_port_status; // end struct CCIP_PORT_STATUS

}; // end struct CCIP_FME_HDR
CASSERT(sizeof(struct CCIP_PORT_HDR) == (9 *8));

/******************************************************************************
 *  64-bit FPGA Port Error  Feature List
 *  Feature ID =0x10
 *  Feature Description Port Error Management
 *  Feature Type = Private
 *  offset 0x40 bytes form Device Feature Header
 ******************************************************************************/

struct CCIP_PORT_ERR_DFL {

   // Port Error Header
   struct CCIP_DFH ccip_port_err_dflhdr;

   struct CCIP_PORT_ERROR_MASK {
      union {
         btUnsigned64bitInt csr;
         struct {
            btUnsigned64bitInt rsvd :64; //Reserved
         };
      };

   }ccip_port_error_mask; // end struct CCIP_PORT_ERROR_MASK

   struct CCIP_PORT_ERROR {
      union {
         btUnsigned64bitInt csr;
         struct {
            btUnsigned64bitInt tx_channel0_overflow :1; //Tx channel 0 : overflow
            btUnsigned64bitInt tx_channel0_encode :1; //Tx channel 0 : request encoding mismatch with valid signal
            btUnsigned64bitInt tx_channel0_invalidreq :1; //Tx channel 0 : invalid encoding request
            btUnsigned64bitInt tx_channel0_sop :1; //Tx channel 0 : sop field not set
            btUnsigned64bitInt rsvd :4;

            btUnsigned64bitInt tx_channel1_overflow :1; //Tx channel 1 : overflow
            btUnsigned64bitInt tx_channel1_encode :1; //Tx channel 1 : request encoding mismatch with valid signal
            btUnsigned64bitInt tx_channel1_invalidreq :1; //Tx channel 1 : invalid encoding request
            btUnsigned64bitInt tx_channel1_payloadinsuff :1; //Tx channel 1 : insufficient data payload
            btUnsigned64bitInt tx_channel1_ploverrun :1; //Tx channel 1 : playload over run
            btUnsigned64bitInt tx_channel1_incorrect :1; //Tx channel 1 : incorrect address
            btUnsigned64bitInt rsvd1 :2;

            btUnsigned64bitInt tx_cfg_status :1; // Cfg channel
            btUnsigned64bitInt rsvd3 :37;
         }; // end struct
      }; // end union

   }ccip_port_error; // end struct CCIP_PORT_ERROR

   // First Error register
   struct CCIP_PORT_FIRST_ERROR {
      union {
         btUnsigned64bitInt csr;
         struct {
            btUnsigned64bitInt tx_channel0_overflow :1; //Tx channel 0 : overflow
            btUnsigned64bitInt tx_channel0_encode :1; //Tx channel 0 : request encoding mismatch with valid signal
            btUnsigned64bitInt tx_channel0_invalidreq :1; //Tx channel 0 : invalid encoding request
            btUnsigned64bitInt tx_channel0_sop :1; //Tx channel 0 : sop field not set
            btUnsigned64bitInt rsvd :4;
            btUnsigned64bitInt tx_channel1_overflow :1; //Tx channel 1 : overflow
            btUnsigned64bitInt tx_channel1_encode :1; //Tx channel 1 : request encoding mismatch with valid signal
            btUnsigned64bitInt tx_channel1_invalidreq :1; //Tx channel 1 : invalid encoding request
            btUnsigned64bitInt tx_channel1_payloadinsuff :1; //Tx channel 1 : insufficient data payload
            btUnsigned64bitInt tx_channel1_ploverrun :1; //Tx channel 1 : playload over run
            btUnsigned64bitInt tx_channel1_incorrect :1; //Tx channel 1 : incorrect address
            btUnsigned64bitInt rsvd1 :2;
            btUnsigned64bitInt tx_cfg_status :1; // Cfg channel
            btUnsigned64bitInt rsvd3 :37;
         }; // end struct
      }; // end union

   }ccip_port_first_error; // end struct CCIP_PORT_FIRST_ERROR


   // port malformed request0
   struct CCIP_PORT_MALFORMED_REQ_0 {

      union {
         btUnsigned64bitInt csr;
         struct {
            btUnsigned64bitInt malfrd_req_lsb :64; // LSB header bit first malformed request
         }; // end struct
      }; // end union

   }ccip_port_malformed_req_0; // end struct CCIP_PORT_MALFORMED_REQ_0

   // port  malformed request1
   struct CCIP_PORT_MALFORMED_REQ_1 {

      union {
         btUnsigned64bitInt csr;
         struct {
            btUnsigned64bitInt malfrd_req_msb :64; // MSB header bit first malformed request
         }; // end struct
      }; // end union

   }ccip_port_malformed_req_1;  // end struct CCIP_PORT_MALFORMED_REQ_1

}; // end struct CCIP_PORT_ERR_DFL
CASSERT(sizeof(struct CCIP_PORT_ERR_DFL) == (6 *8));

/******************************************************************************
 *  FPGA Port USMG Feature
 *  Feature ID =0x11
 *  Feature Description Port USMG
 *  Feature Type = Private
 *
 ******************************************************************************/

struct CCIP_PORT_UMSG_DFL {

   // Port usmg Header
   struct CCIP_DFH ccip_port_umsg_dflhdr;

   // umsg capability
   struct CCIP_UMSG_CAPABILITY {

      union {
         btUnsigned64bitInt csr;
         struct {
            btUnsigned64bitInt no_umsg_alloc_port :8; // number of umag allocated to this port
            btUnsigned64bitInt status_usmg_engine :1; //  enable umsg engine for this ort 1-enable 0-disable
            btUnsigned64bitInt umsg_init_satus :1; // usmg initialization status
            btUnsigned64bitInt rsvd :54;
         }; // end struct
      }; // end union

   }ccip_umsg_capability; // end struct CCIP_UMSG_CAPABILITY


   // umsg base address
   struct CCIP_UMSG_BASE_ADDRESS {

      union {
         btUnsigned64bitInt csr;
         struct {
            btUnsigned64bitInt umsg_base_address :64; // UMAS segment start physical byte address
         }; // end struct
      }; // end union

   }ccip_umsg_base_address; // end struct CCIP_UMSG_BASE_ADDRESS


   // umsg mode
   struct CCIP_UMSG_MODE {

      union {
         btUnsigned64bitInt csr;
         struct {
            btUnsigned64bitInt umsg_hit :32; // UMSG hit enable / disable
            btUnsigned64bitInt rsvd :32;
         }; // ens struct
      }; // end union

   }ccip_umsg_mode; // end struct CCIP_UMSG_MODE


}; // end struct CCIP_PORT_UMSG_feature
CASSERT(sizeof(struct CCIP_PORT_UMSG_DFL) == (4 *8));


/******************************************************************************
 *  64-bit FPGA Port/afu Partial Reconfiguration Feature list
 *  Feature ID =0x12
 *  Feature Description Port/ AFU partial reconfiguration
 *  Feature Type = Private
 *
 ******************************************************************************/
struct CCIP_PORT_PR_DFL {

   // Port usmg Header
   struct CCIP_DFH ccip_port_pr_dflhdr;

   // Partial Receconfiguration Control
   struct CCIP_PORT_PR_CONTROL {

      union {
         btUnsigned64bitInt csr;
         struct {
            btUnsigned64bitInt pr_start_req :1; // PR start Request
            btUnsigned64bitInt rsvd :63;
         }; // end struct
      }; // end union

   }ccip_port_pr_control; // end struct CCIP_PORT_PR_CONTROL


   // Partial Reconfiguration status
   struct CCIP_PORT_PR_STATUS {
      union {
         btUnsigned64bitInt csr;
         struct {
            btUnsigned64bitInt pr_credit :9;         // Credits
            btUnsigned64bitInt rsvd2 :3;
            btUnsigned64bitInt pr_acces_grant :1;    // PR access Grant
            btUnsigned64bitInt pr_access :1;         // PR access enabled
            btUnsigned64bitInt rsvd3 :2;
            btUnsigned64bitInt pr_timeout_error :1;  // PR port timeout error
            btUnsigned64bitInt pr_engine_error :1;   // PR Engine error
            btUnsigned64bitInt pr_data_ovrferr :1;   // PR data overfow error
            btUnsigned64bitInt rsvd4 :1;
            btUnsigned64bitInt pr_mega_fstatus :3;   // Altra PR Mega-function status

            // enum e_CCIP_PR_Megafun_status pr_mega_fstatus :3; // Altra PR Mega-function status

            btUnsigned64bitInt rsvd1 :1;
            btUnsigned64bitInt pr_status :4;         // PR Status

            //enum e_CCIP_PORT_PR_status   pr_status :4; // PR Status

            btUnsigned64bitInt rsvd :35;
         }; // end struct
      };  // end union

   }ccip_port_pr_status; // end struct CCIP_PORT_PR_STATUS

   // Partial Receconfiguration Data
   struct CCIP_PORT_PR_DATA {

      union {
         btUnsigned64bitInt csr;
         struct {
            btUnsigned64bitInt pr_data_raw :32; // PR data from the raw-binary file
            btUnsigned64bitInt rsvd :32;
         }; // end struct
      }; // end union

   }ccip_port_pr_data; // end struct CCIP_PORT_PR_DATA

   struct CCIP_PORT_PR_PBUDGET {

      union {
         btUnsigned64bitInt csr;
         struct {
            btUnsigned64bitInt pwr_format :48; // AFU Power budget format
            btUnsigned64bitInt rsvd :16;

         }; // end struct
      }; // end union

   }ccip_port_pr_pbudget; // end struct CCIP_PORT_PR_PBUDGET


   // User Clock Frequency
   struct CCIP_USR_CLK_FREQ {

      union {
         btUnsigned64bitInt csr;
         struct {
            btUnsigned64bitInt rsvd :64; // TBD
         }; // end union
      }; // end stuct

   }ccip_usr_clk_freq; // end struct CCIP_USR_CLK_FREQ


}; // end struct CCIP_PORT_UMSG_feature
CASSERT(sizeof(struct CCIP_PORT_PR_DFL) == (6 *8));


/******************************************************************************
 *  FPGA Port signal tap  feature list
 *  Feature ID =0x13
 *  Feature Description Port signal Tap
 *  Feature Type = Private
 *
 ******************************************************************************/

struct CCIP_PORT_STAP_DFL {

   // Port usmg Header
   struct CCIP_DFH ccip_port_stap_dflhdr;

   // Remote Signal Tap
   struct CCIP_PORT_STAP {
      union {
         btUnsigned64bitInt csr;
         struct {
            btUnsigned64bitInt rsvd :64; // TBD
         };
      };

   }ccip_port_stap; // end struct CCIP_PORT_STAP

}; // end struct CCIP_PORT_UMSG_feature
CASSERT(sizeof(struct CCIP_PORT_STAP_DFL) == (2*8));


/******************************************************************************
 *  FPGA Port signal tap  feature
 *  Feature ID =0x13
 *  Feature Description Port signal Tap
 *  Feature Type = Private
 *
 ******************************************************************************/

struct CCIP_AFU_Header {

   // FME  Header
   struct CCIP_DFH  ccip_dfh;

   // FME afu id  low
   struct CCIP_AFU_ID_L ccip_afu_id_l;

   // FME afu id  low
   struct CCIP_AFU_ID_H  ccip_afu_id_h;

   // Next AFU offset
   struct CCIP_NEXT_AFU  ccip_next_afu;
} ;
CASSERT(sizeof(struct CCIP_AFU_Header) == (4*8));




// CCIP port device
struct port_device
{
   struct CCIP_PORT_HDR         *m_pport_hdr;        // PORRT Header
   struct CCIP_PORT_ERR_DFL 	  *m_pport_err;       // PORT Error DFL
   struct CCIP_PORT_UMSG_DFL    *m_pport_umsg;       // PORT USMG DFL
   struct CCIP_PORT_PR_DFL      *m_pport_pr;         // PORT PR DFL
   struct CCIP_PORT_STAP_DFL    *m_pport_stap;       // PORT Signal tap DFL

   struct pci_dev               *m_pcidev;         // Linux pci_dev pointer (or NULL if manual)
   // Used for being added to the global list of devices.
   struct list_head              m_list;           // List itself

   // Private semaphore
   struct semaphore              m_sem;

   // struct ccip_PIPsession   *m_pPIPSession;     // PIP session object

   // struct aal_device         aal_dev;         // AAL Device from which this is derived


}; // end struct port_device

#define ccip_port_dev_pci_dev(pdev)          ((pdev)->m_pcidev)

#define ccip_port_aal_dev(pdev)               ((pdev)->aal_dev)
#define ccip_port_hdr(pdev)                   ((pdev)->m_pport_hdr)
#define ccip_port_err(pdev)                   ((pdev)->m_pport_err)
#define ccip_port_umsg(pdev)                  ((pdev)->m_pport_umsg)
#define ccip_port_pr(pdev)                    ((pdev)->m_pport_pr)
#define ccip_port_stap(pdev)                  ((pdev)->m_pport_stap)


#define ccip_port_mem_sessionp(pdev)              ((pdev)->m_pmem_session)


#define ccip_dev_port_mmio(pdev)              ((pdev)->m_phys_port_mmio)
#define ccip_dev_port_kvp_mmio(pdev)          ((pdev)->m_kvp_port_mmio)
#define ccip_dev_port_len_mmio(pdev)          ((pdev)->m_len_port_mmio)

#define ccip_dev_port_to_pci_dev(pdev)            ((pdev)->m_pcidev)
#define ccip_dev_port_to_aaldev(pdev)             ((pdev)->m_aal_dev)



#define ccip_dev_port_list_head(pdev)             ((pdev)->m_list)
#define ccip_list_port_to_ccip_device(plist)      kosal_list_entry(plist, struct port_device, m_list)
#define aaldev_to_ccip_port_device(plist)         kosal_list_entry(plist, struct port_device, m_list)
#define ccip_dev_port_to_PIPsessionp(pdev)        ((pdev)->m_pPIPSession)
#define ccip_dev_port_psem(pdev)                  (&(pdev)->m_sem)



// CCIP afu device
struct afu_device
{

   //   struct aal_device         aal_dev;         // AAL Device from which this is derived

   struct CCIP_AFU_Header    *pAfu_header;

   struct CCIP_AFU_Feature   *pAfu_feature;


}; // end struct ccip_afu_device

// CCIP board device
struct ccip_device
{
   // Used for being added to the global list of devices.
   struct list_head           m_list;

   // Head of the list of AAL devices created
   struct list_head           m_devlisthead;

   struct fme_device         *m_pfme_dev;    // FME Device

   // List of ports TBD
   struct port_device        *m_pport_dev;   // FME Port

   // List of AFU  TBD
   struct afu_device         *m_pafu_dev;    // FME AFU

   struct pci_dev            *m_pcidev;         // Linux pci_dev pointer

   // TBD ?
   //struct pr_device         *m_ppr_dev ;

   btUnsignedInt              m_flags;

   // Private semaphore
   struct semaphore           m_sem;

   int                        m_simulated;

   btUnsigned32bitInt         m_busNum;
   btUnsigned32bitInt         m_devicenum;   // device number
   btUnsigned32bitInt         m_functnum;    // function number

   // FME MMIO Space
   btVirtAddr                 m_kvp_fme_mmio;   // kv address of MMIO space
   btPhysAddr                 m_phys_fme_mmio;  // Physical address of MMIO space
   size_t                     m_len_fme_mmio;   // Bytes

   btVirtAddr                 m_kvp_port_mmio;   // kv address of MMIO space
   btPhysAddr                 m_phys_port_mmio;  // Physical address of MMIO space
   size_t                     m_len_port_mmio;

   // AFU MMIO Space
   btVirtAddr                 m_kvp_afu_mmio;   // kv address of MMIO space
   btPhysAddr                 m_phys_afu_mmio;  // Physical address of MMIO space
   size_t                     m_len_afu_mmio;   // Bytes


}; // end struct ccip_afu_device


#define pci_dev_to_ccip_dev(ptr)             ccip_container_of(ptr, struct pci_dev, m_pcidev, struct ccip_device)
#define ccip_dev_to_fme_dev(pdev)            (pdev->m_pfme_dev)

#define ccip_dev_pci_dev(pdev)               ((pdev)->m_pcidev)

#define cciv4_dev_board_type(pdev)            ((pdev)->m_boardtype)

#define ccip_set_simulated(pdev)             ((pdev)->m_simulated = 1)
#define ccip_clr_simulated(pdev)             ((pdev)->m_simulated = 0)
#define ccip_is_simulated(pdev)              ((pdev)->m_simulated == 1)

#define ccip_dev_to_pci_dev(pdev)            ((pdev)->m_pcidev)
#define ccip_dev_to_aaldev(pdev)             ((pdev)->m_aaldev)


#define ccip_dev_list_head(pdev)             ((pdev)->m_list)
#define ccip_list_to_ccip_device(plist)     kosal_list_entry(plist, struct ccip_device, m_list)
#define aaldev_to_ccip_device(plist)         kosal_list_entry(plist, struct ccip_device, m_list)
#define ccip_dev_to_PIPsessionp(pdev)        ((pdev)->m_pPIPSession)
#define ccip_dev_psem(pdev)                  (&(pdev)->m_sem)

#define ccip_aal_dev_list(pdev)              ((pdev)->m_devlisthead)

#define ccip_fmedev_phys_afu_mmio(pdev)         ((pdev)->m_phys_fme_mmio)
#define ccip_fmedev_kvp_afu_mmio(pdev)          ((pdev)->m_kvp_fme_mmio)
#define ccip_fmedev_len_afu_mmio(pdev)          ((pdev)->m_len_fme_mmio)

#define ccip_portdev_phys_afu_mmio(pdev)         ((pdev)->m_phys_port_mmio)
#define ccip_portdev_kvp_afu_mmio(pdev)          ((pdev)->m_kvp_port_mmio)
#define ccip_portdev_len_afu_mmio(pdev)          ((pdev)->m_len_port_mmio)


#define ccip_dev_pcie_busnum(pdev)              ((pdev)->m_busNum)
#define ccip_dev_pcie_devnum(pdev)              ((pdev)->m_devicenum)
#define ccip_dev_pcie_fcnnum(pdev)              ((pdev)->m_functnum)

/// @brief   Writes 64 bit control and status registers.
///
/// @param[in]  baseAddress   base CSR address.
/// @param[in]  offset        offset of CSR  .
/// @param[in]  value    value  going to be write in CSR.
/// @return   void
int write_ccip_csr64(btVirtAddr baseAddress, btCSROffset offset,bt64bitCSR value);

/// @brief   read 64 bit control and status registers.
///
/// @param[in]  baseAddress   base CSR address.
/// @param[in]  offset        offset of CSR  .
/// @return    64 bit  CSR value
bt64bitCSR read_ccip_csr64(btVirtAddr baseAddress ,  btCSROffset offset );

/// @} group aalkernel_ccip



END_NAMESPACE(AAL)

#endif /* __AALKERNEL_CCIP_SIM_DEF_H_ */
