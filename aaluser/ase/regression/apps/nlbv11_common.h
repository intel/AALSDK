// ****************************************************************************
//                               INTEL CONFIDENTIAL
//
//        Copyright (c) 2011 Intel Corporation All Rights Reserved.
//
// The source code contained or described herein and all  documents related to
// the  source  code  ("Material")  are  owned  by  Intel  Corporation  or its
// suppliers  or  licensors.    Title  to  the  Material  remains  with  Intel
// Corporation or  its suppliers  and licensors.  The Material  contains trade
// secrets  and  proprietary  and  confidential  information  of  Intel or its
// suppliers and licensors.  The Material is protected  by worldwide copyright
// and trade secret laws and treaty provisions. No part of the Material may be
// used,   copied,   reproduced,   modified,   published,   uploaded,  posted,
// transmitted,  distributed,  or  disclosed  in any way without Intel's prior
// express written permission.
//
// No license under any patent,  copyright, trade secret or other intellectual
// property  right  is  granted  to  or  conferred  upon  you by disclosure or
// delivery  of  the  Materials, either expressly, by implication, inducement,
// estoppel or otherwise.  Any license under such intellectual property rights
// must be express and approved by Intel in writing.
// ****************************************************************************
#define NLB_v1_1         0xC000C9660D824272
#define NLB_v1_1_H         0xC000C9660D824272
#define NLB_v1_1_L         0x9AEFFE5F84570612

#define NLB_DSM_BASEL_OFF 0x1A00
#define NLB_DSM_BASEH_OFF 0x1A04
#define NLB_SRC_ADDR_OFF  0x1A20
#define NLB_DST_ADDR_OFF  0x1A24
#define NLB_NUM_LINES_OFF 0x1A28
#define NLB_CTL_OFF       0x1A2C
#define NLB_STATUS_OFF    0x1A40
#define NLB_CFG_OFF       0x1A34
#define NLB_INACT_THRESH  0x1A38

//----------------------------------------------------------------------------------
// Device Status Memory (DSM) Address Map ***** DO NOT MODIFY *****
// Physical address = value at CSR_AFU_DSM_BASE + Byte offset
//----------------------------------------------------------------------------------
//                                 Byte Offset              Attribute    Width   Comments
#define      DSM_AFU_ID            0                     // RO           32b     non-zero value to uniquely identify the AFU
#define      DSM_STATUS            0x40                  // RO           512b    test status and error info



