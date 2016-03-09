// Copyright(c) 2007-2016, Intel Corporation
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
/// @file INTCDefs.h
/// @brief Intel-specific defines.
/// @ingroup Events
/// @verbatim
/// Accelerator Abstraction Layer
///
/// AUTHORS: Joseph Grecco, Intel Corporation
///           Henry Mitchel, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 07/13/2007     JG       Initial version started
/// 05/08/2008     HM       Comments & License
/// 01/04/2009     HM       Updated Copyright@endverbatim
//****************************************************************************
#ifndef __AALSDK_INTCDEFS_H__
#define __AALSDK_INTCDEFS_H__
#include <aalsdk/AALTypes.h>
#include <aalsdk/AALIDDefs.h>

/******************************************************************************
* Define Vendor-INTC specific macros
******************************************************************************/
#define AAL_FACTORY_CREATE_SERVICENAME "ServiceName"
#define AAL_FACTORY_CREATE_AIANAME     "AIAName"

// The following keys are used to override Registrar lookup
#define AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED          "ConfigRecordIncluded"
#define AAL_FACTORY_CREATE_CONFIGRECORD_FULL_AIA_NAME     "AIAExecutable"
#define AAL_FACTORY_CREATE_CONFIGRECORD_FULL_AFU_NAME     "AFUExecutable"
#define AAL_FACTORY_CREATE_SOFTWARE_SERVICE               "_CreateSoftService"
#define AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME "ServiceExecutable"
#define AAL_FACTORY_CREATE_CONFIGRECORD_SERVICEPROXY      "AALServiceProxy"
#define AAL_FACTORY_CREATE_CONFIGRECORD_SERVICENAME       "ConfigRecordServiceName"



/******************************************************************************
* Define INTC-AAL specific versions of macros
******************************************************************************/
#define __INTC_IID(sys, Num)         AAL_IID( AAL_vendINTC, (sys), (Num))
#define __INTC_Evt(sys, Num)         AAL_Event( AAL_vendINTC, (sys), (Num))
#define __INTC_ExEvt(sys, Num)       AAL_ExceptionEvent( AAL_vendINTC, (sys), (Num))
#define __INTC_TranEvt(sys, Num)     AAL_TransactionEvent( AAL_vendINTC, (sys), (Num))
#define __INTC_ExTranEvt(sys, Num)   AAL_ExceptionTransactionEvent( AAL_vendINTC, (sys), (Num))
#define __INTC_ErrNum(sys, Num)      AAL_ExceptionCode( AAL_vendINTC, (sys), (Num))
#define __INTC_ReasCode(sys, Num)    AAL_ReasonCode( AAL_vendINTC, (sys), (Num))
#define __INTC_RegistrarKey(sys,Num) AAL_RegistrarKey( AAL_vendINTC, (sys), (Num))

/// CAASBase interface id.
#define iidCBase                     __INTC_IID(AAL_sysAAL, 0x0001)
/// CAALEvent interface id.
#define iidCEvent                    __INTC_Evt(AAL_sysAAS, 0x0000)
#define iidCExEvent                  __INTC_ExEvt(AAL_sysAAS, 0x0000)
#define iidCTranEvent                __INTC_TranEvt(AAL_sysAAS, 0x0000)
#define iidCExTranEvent              __INTC_ExTranEvt(AAL_sysAAS, 0x0000)

#define iidCAFUDev                   __INTC_IID(AAL_sysUAIA, 0x0001)

#define __INTC_typeCommandHeader    (AAL_typeBase+0x0001)

#define __INTC_OpCode(Num)          AAL_ID(AAL_vendINTC,AAL_sysAFU,AAL_NotException,__INTC_typeCommandHeader,(Num))
#define __INTC_opcodeCommand        __INTC_OpCode(0x0001)

#define __INTC_typeParm             (AAL_typeBase+0x0002)
#define __INTC_Parameter(Num)       AAL_ID(AAL_vendINTC,AAL_sysAFU,__INTC_typeParm,(Num))
#define __INTC_parmSubClass         __INTC_Parameter(0x0001)

/******************************************************************************
* Define INTC-AAL specific Systems - Only use in INTC namespace
******************************************************************************/
#define  INTC_sysBase(num)             (AAL_sysBase+(num))
#define  INTC_sysNullPIPAIA            INTC_sysBase(0x0001)    // NULL PIP AIA
#define  INTC_sysFSBAIA                INTC_sysBase(0x0002)    // FSB AIA
#define  INTC_sysSampleAIA             INTC_sysBase(0x0003)    // SDK Sample AIA
#define  INTC_sysFSBV1AIA              INTC_sysBase(0x0004)    // FSB Version 1.0 AIA
#define  INTC_sysAIA                   INTC_sysBase(0x0005)    // Version 4 AIA

#define  iidAIAService                 __AAL_IID(INTC_sysAIA, 0x0001)   // AIA Service interface
#define  iidAFUProxy                   __INTC_IID(AAL_sysUAIA, 0x0002)  // AIA Proxy interface
#define  iidAFUProxyClient             __INTC_IID(AAL_sysUAIA, 0x0003)  // AIA Proxy interface

#define  INTC_sysSampleAFU             INTC_sysBase(0x0005)    // Sample AFU
#define  INTC_sysAFULinkInterface      INTC_sysBase(0x0006)    // AFU Link Interface & derivatives

#define  INTC_sysNext                  0x0009

// Used in samples
#define  INTC_sysSampleMisc            INTC_sysBase(0x1000)

#endif // __AALSDK_INTCDEFS_H__

