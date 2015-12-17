// ***************************************************************************
//
//        Copyright (C) 2008-2015 Intel Corporation All Rights Reserved.
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
//
// Engineer :           Pratik Marolia
// Creation Date :	12-10-2015
// Last Modified :	Tue 13 Oct 2015 09:26:38 AM PDT
// Module Name :	ccis_if_pkg.sv
// Project :             
// Description : CCI-S package  
//
// ***************************************************************************

package ccis_if_pkg;
    // CCI-S paramters - Delete this after NLB is converted to be CCI-P compliant
    //----------------------------------------------------------------------
    parameter CCIS_TXHDR_WIDTH     = 61;
    parameter CCIS_TXHDR_MDATA_LSB = 0;
    parameter CCIS_TXHDR_MDATA_MSB = 13;
    parameter CCIS_TXHDR_ADDR_LSB  = CCIS_TXHDR_MDATA_MSB+1;
    parameter CCIS_TXHDR_ADDR_MSB  = CCIS_TXHDR_MDATA_MSB+32;
    parameter CCIS_TXHDR_REQ_LSB   = CCIS_TXHDR_ADDR_MSB+7;
    parameter CCIS_TXHDR_REQ_MSB   = CCIS_TXHDR_ADDR_MSB+6+4;
    
    parameter CCIS_RXHDR_WIDTH     = 24;
    parameter CCIS_RXHDR_MDATA_LSB = 0;
    parameter CCIS_RXHDR_MDATA_MSB = 13;
    parameter CCIS_RXHDR_REQ_LSB   = CCIS_RXHDR_MDATA_MSB+1;
    parameter CCIS_RXHDR_REQ_MSB   = CCIS_RXHDR_MDATA_MSB+4;

endpackage
