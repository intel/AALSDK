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
/// @file AALIDDefs.h
/// @brief Top-level Vendor ID's and construction Macros for AAL_ID's.
///
/// The intent is for Vendors to be included in this top-level file.
///
/// Vendors can define their own in a separate file, and so long as the two
/// Vendor's programs are not used at the same time the binary Vendor ID's
/// will be distinguishable.
///
/// Even if two Vendors use the same ID, all that will happen is that at run time
/// it will be impossible to distinguish an error code based on the ID alone.
///
/// @ingroup Events
/// @verbatim
/// Accelerator Abstraction Layer
///
/// AUTHORS: Henry Mitchel, Intel Corporation
///          Joseph Grecco, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 05/17/2007     HM       Initial version started
/// 08/25/2007     HM       Registrar codes finalized with interface implementation
/// 08/28/2007     HM       Changed Vendor field to accept 16-bits so as to
///                            be able to handle PCI-SIG Vendor ID's
/// 02/21/2008     JG       Made file 'C' friendly
/// 05/08/2008     HM       Comments & License
/// 07/23/2008     HM       Added beginning of Resource Manager keys, started
///                            testing conversion to consts from macros
/// 09/22/2008	    HM       Added ResMgr sub-system key
/// 09/23/2008     JG+HM    Added Management AFU sub-system keys
/// 10/12/2008     HM       Merging updates for MAFU, adding Registrar keys
/// 11/13/2008     HM       Fixed RegistrarKey macro creation bug
/// 11/13/2008     HM       Adding keys for Instance Record
/// 11/25/2008     JG       Added keys for uAIA and AFUDev
/// 12/14/2008     HM       Added AAL_* prefix for string keys
/// 12/17/2008     HM       Corrected spelling for bt32bitInt
/// 01/04/2009     HM       Updated Copyright
/// 02/14/2009     HM       Added tranevtUnBindAFUDevEvent
/// 02/15/2009     HM       Added various keys needed for device allocation
/// 07/15/2009     HM       Added AAL_sysShutdown
/// 10/22/2009     AC       Added AAL_sysASM
/// 03/22/2010     AC       Added errAFUTransactionNotFound
/// 06/21/2011     JG       Added iidService@endverbatim
//****************************************************************************
#ifndef __AALSDK_AALIDDEFS_H__
#define __AALSDK_AALIDDEFS_H__

/*=============================================================================
===============================================================================
=============== Define Vendors - do not go over 0x0FFF ========================
===============================================================================
=============================================================================*/

// Use PCI Vendor ID's, controlled by PCI-SIG

#define  AAL_vendAAL    (0x0000)    // Maintainers of AAL
#define  AAL_vendINTC   (0x8086)
#define  AAL_vendUSER   (0xFFFF)    // For experimentation, need to check if
//    conflicts with any PCI-SIG user

/*=============================================================================
===============================================================================
================== Define Symbol Construction Macros ==========================
===============================================================================
=============================================================================*/

// AAL_IDs Based on 64-bit ID

// Define bit fields for the AAL_ID macro
#define AAL_AvailableBits     64 // (8*sizeof(unsigned long long))
// - hardcoded because can't use sizeof() in #if
// layout is 0x0VVV VSSS ETTT IIII
#define AAL_VendorBits        16
#define AAL_VendorBitsMask    ((1<<AAL_VendorBits)-1) // 0xFFFF
#define AAL_VendorShift       44

#define AAL_SubSystemBits     12
#define AAL_SubSystemBitsMask ((1<<AAL_SubSystemBits)-1)   // 0x0FFF
#define AAL_SubSystemShift    32

#define AAL_ExceptionBits     1
#define AAL_ExceptionBitsMask ((1<<AAL_ExceptionBits)-1) // 1
#define AAL_ExceptionShift    31

#define AAL_TypeBits          15
#define AAL_TypeBitsMask      ((1<<AAL_TypeBits)-1) // 0x7FFF
#define AAL_TypeShift         16

#define AAL_ItemBits          16
#define AAL_ItemBitsMask      ((1<<AAL_ItemBits)-1) // 0xFFFF
#define AAL_ItemShift         0

// Worker functions - recursively expand macros
#define AAL_STRING(x) #x               // e.g. __LINE__ the token becomes "__LINE__" the string
#define AAL_STRING2(x) AAL_STRING(x)   // e.g. "__LINE__" the string is expanded to its macro definition "42"

// Check integrity of the bit field definitions
#if (!((AAL_VendorBits + AAL_VendorShift) <= AAL_AvailableBits))   // test size and shift integrity
#pragma message ("MacroError: (AAL_VendorBits + AAL_VendorShift) > AAL_AvailableBits in " __FILE__ "[" AAL_STRING2(__LINE__) "]")
#endif

#if (!((AAL_VendorBits + AAL_SubSystemBits + AAL_SubSystemShift) <= AAL_AvailableBits))
#pragma message ("MacroError: (AAL_VendorBits + AAL_SubSystemBits + AAL_SubSystemShift) > AAL_AvailableBits in " __FILE__ "[" AAL_STRING2(__LINE__) "]")
#endif

#if (!((AAL_VendorBits + AAL_SubSystemBits + AAL_ExceptionBits + AAL_ExceptionShift) <= AAL_AvailableBits))
#pragma message ("MacroError: (AAL_VendorBits + AAL_SubSystemBits + AAL_TypeBits + AAL_TypeShift) > AAL_AvailableBits in " __FILE__ "[" AAL_STRING2(__LINE__) "]")
#endif

#if (!((AAL_VendorBits + AAL_SubSystemBits + AAL_ExceptionBits + AAL_TypeBits + AAL_TypeShift) <= AAL_AvailableBits))
#pragma message ("MacroError: (AAL_VendorBits + AAL_SubSystemBits + AAL_TypeBits + AAL_TypeShift) > AAL_AvailableBits in " __FILE__ "[" AAL_STRING2(__LINE__) "]")
#endif

#if (!((AAL_VendorBits + AAL_SubSystemBits + AAL_ExceptionBits + AAL_TypeBits + AAL_ItemBits) <= AAL_AvailableBits))
#pragma message ("MacroError: (AAL_VendorBits + AAL_SubSystemBits + AAL_TypeBits + AAL_ItemBits) > AAL_AvailableBits in " __FILE__ "[" AAL_STRING2(__LINE__) "]")
#endif

/******************************************************************************
* Define the ID building macro itself
******************************************************************************/
#define AAL_ID(vendor,subsys,excpt,type,item) \
           ((((AAL::btID)((vendor) & (AAL_VendorBitsMask)))   <<AAL_VendorShift ) | \
            (((AAL::btID)((subsys) & (AAL_SubSystemBitsMask)))<<AAL_SubSystemShift ) | \
            (((AAL::btID)((excpt)  & (AAL_ExceptionBitsMask)))<<AAL_ExceptionShift ) | \
            (((AAL::btID)((type)   & (AAL_TypeBitsMask)))     <<AAL_TypeShift ) | \
            (((AAL::btID)((item)   & (AAL_ItemBitsMask)))     <<AAL_ItemShift ) \
           )

/*=============================================================================
===============================================================================
========================= Define AAL Base Symbols =============================
===============================================================================
=============================================================================*/

/******************************************************************************
* Define AAL Built-in Subsystems - Vendors extend from AAL_sysBase
******************************************************************************/

/* 0x000 to 0x0FF Reserved for AAL */
#define AAL_sysAny                     (0x0001)
#define AAL_sysAAL                     (0x0002)
#define AAL_sysAAS                     (0x0003)
#define AAL_sysAIA                     (0x0004)
#define AAL_sysAFUFactory              (0x0005)
#define AAL_sysAFU                     (0x0006)
#define AAL_sysRegistrar               (0x0007)
#define AAL_sysFactory                 (0x0008)
#define AAL_sysDatabase                (0x0009) // Database used by Resource Manager
#define AAL_sysEDS                     (0x000a) // Event Delivery Service
#define AAL_sysResMgr                  (0x000b) // Resource Manager
#define AAL_sysResMgrClient            (0x000c) // Resource Manager Client
#define AAL_sysManagementAFU           (0x000d) // Management AFU
#define AAL_sysUAIA                    (0x000e) // Universal AIA
#define AAL_sysShutdown                (0x000f) // not a true sub-system, but useful for logging
                                                //    May wish to disable this and set DEBUG_BEYOND_LOGGER
                                                //    in AALLoggerExtern.h, instead
#define AAL_sysResConf                 (0x0010) // Resource Configuration Service
#define AAL_sysALI                     (0x0011) // ALI Service

/* Reserved for AAL to 0x0FF */
#define AAL_sysBase                    (0x0100) // Start from sysBase to define your own

/******************************************************************************
* Define AAL Built-in Exceptions, either it is, or it isn't
******************************************************************************/
#define AAL_Exception      1
#define AAL_NotException   0

#define AAL_Exception_Bit  (AAL_ID(0,0,AAL_Exception,0,0)) // Test Bit if ID is an exception
/// Determine whether an interface id is that of an exception.
/// @ingroup Events
#define AAL_IS_EXCEPTION(ID) (((AAL::btID)(ID)) & AAL_Exception_Bit)
/******************************************************************************
* Define AAL Built-in Types - Vendors extend from AAL_typeBase
******************************************************************************/

/* 0x0000 to 0x00FF Reserved for AAL --------------- naming convention */

#define AAL_typeAny                       (0x0001)             // Any named ID
#define AAL_typeIID                       (0x0002)             // iidXXX
#define AAL_typeEvt                       (0x0003)             // evtXXX

#define AAL_typeTranEvt                   (0x0004)             // tranevt
#define AAL_typeTransactionEvent          (AAL_typeTranEvt)    // alias

#define AAL_typeExceptionNumber           (0x0005) // err      major exception number
#define AAL_typeReasonCode                (0x0006) // reas     minor exception number
#define AAL_typeReasonString              (0x0007) // str      descriptive string

#define AAL_typeKey                       (0x0008)             // This is a Key in the Registrar Database

/* Start from typeBase to define your own, Maximum 0x7FFF */
#define AAL_typeBase                      (0x0100)

/* Use AAL_typeEXCEPTION to code exception-specific ID's for rapid identification */

/*=============================================================================
===============================================================================
======================= Define AAL Global Event Codes =========================
===============================================================================
=============================================================================*/

/******************************************************************************
* Define AAL Generic Macros for TYPE - extend from these for VENDOR
******************************************************************************/

// id is Interface ID, or class definition
#define AAL_IID(vend,sys,id)                       AAL_ID((vend),(sys),AAL_NotException,AAL_typeIID,(id))

// key is any ID that could be used as a key to an NVS
#define AAL_Nid(vend,sys,id)                       AAL_ID((vend),(sys),AAL_NotException,AAL_typeAny,(id))

// event is an unsolicited event
#define AAL_Event(vend,sys,id)                     AAL_ID((vend),(sys),AAL_NotException,AAL_typeEvt,(id))

// exception event is an unsolicited event
#define AAL_ExceptionEvent(vend,sys,id)            AAL_ID((vend),(sys),AAL_Exception,AAL_typeEvt,(id))

// transaction event is successful return from asynchronous call
#define AAL_TransactionEvent(vend,sys,id)          AAL_ID((vend),(sys),AAL_NotException,AAL_typeTranEvt,(id))

// exception transaction event is unsuccessful return from asynchronous call
#define AAL_ExceptionTransactionEvent(vend,sys,id) AAL_ID((vend),(sys),AAL_Exception,AAL_typeTranEvt,(id))

// excode is major exception number returned in a exception transaction event
#define AAL_ExceptionCode(vend,sys,excode)         AAL_ID((vend),(sys),AAL_Exception,AAL_typeExceptionNumber,(excode))

// reasoncode is minor exception number returned in a exception transaction event
#define AAL_ReasonCode(vend,sys,reascode)          AAL_ID((vend),(sys),AAL_Exception,AAL_typeReasonCode,(reascode))

// key is a key in the Registrar Database
#define AAL_RegistrarKey(vend,sys,key)             AAL_ID((vend),(sys),AAL_NotException,AAL_typeKey,(key))

// Enum,Key is one of an enumeration in the Registrar Database
// Where Key is 0, the resulting value is used as the Name in the NVP of the Enumeration.
// For other Keys, the key is the enumeration value.
// The enumType must be a valid AAL Type, generally an extension of AAL_typeBase, e.g. AAL_typeBase+1, +2, etc.
// The enumeration in the database is an NVP, where the Name is this macro with key==0, and the value
//    is this macro with the same enumType and a different key
#define AAL_Enumeration(vend,sys,enumType,key)     AAL_ID((vend),(sys),AAL_NotException,(enumType),(key))


/******************************************************************************
* Define Vendor-AAL specific versions of above macros
******************************************************************************/
#define __AAL_IID(sys, Num)         AAL_IID( AAL_vendAAL, (sys), (Num))
#define __AAL_Nid(sys, Num)         AAL_Nid( AAL_vendAAL, (sys), (Num))
#define __AAL_Evt(sys, Num)         AAL_Event( AAL_vendAAL, (sys), (Num))
#define __AAL_ExEvt(sys, Num)       AAL_ExceptionEvent( AAL_vendAAL, (sys), (Num))
#define __AAL_TranEvt(sys, Num)     AAL_TransactionEvent( AAL_vendAAL, (sys), (Num))
#define __AAL_ExTranEvt(sys, Num)   AAL_ExceptionTransactionEvent( AAL_vendAAL, (sys), (Num))
#define __AAL_ErrNum(sys, Num)      AAL_ExceptionCode( AAL_vendAAL, (sys), (Num))
#define __AAL_ReasCode(sys, Num)    AAL_ReasonCode( AAL_vendAAL, (sys), (Num))

/*=============================================================================
===============================================================================
===================== Define AAL-Specific Generic Errors ======================
===============================================================================
=============================================================================*/

/******************************************************************************
* AAL-subsystem-wide events, error numbers, reason codes, and reason strings
******************************************************************************/

/// @addtogroup SysEvents
/// @{

/// CObjectDestroyedTransactionEvent interface id.
#define tranevtObjectDestroyed         __AAL_TranEvt(AAL_sysAAL, 0x0001)
#define tranevtMethodFailed            __AAL_TranEvt(AAL_sysAAL, 0x0002)
#define tranevtCancel                  __AAL_TranEvt(AAL_sysAAL, 0x0003)
#define tranevtNotImplemented          __AAL_TranEvt(AAL_sysAAL, 0x0004)
#define tranevtDeferRequest            __AAL_TranEvt(AAL_sysAAL, 0x0005)

#define extranevtObjectDestroyed       __AAL_ExTranEvt(AAL_sysAAL, 0x0001)
#define extranevtMethodFailed          __AAL_ExTranEvt(AAL_sysAAL, 0x0002)
#define extranevtNotImplemented        __AAL_ExTranEvt(AAL_sysAAL, 0x0003)
#define extranevtDeferRequest          __AAL_ExTranEvt(AAL_sysAAL, 0x0004)

/// @}

/// COKEvent interface id.
#define evtOK                          __AAL_Evt(AAL_sysAAL, 0x0001)


/// @addtogroup SysEvents
/// @{

#define AAL_ErrNum(Num)                __AAL_ErrNum(AAL_sysAAL, Num)
#define AAL_ReasCode(Num)              __AAL_ReasCode(AAL_sysAAL, Num)

#define errOK                          AAL_ErrNum     (0x0000)
#define errBadParameter                AAL_ErrNum     (0x0001)
#define errCauseUnknown                AAL_ErrNum     (0x0002)
#define errCreationFailure             AAL_ErrNum     (0x0003)
#define errEDSFailed                   AAL_ErrNum     (0x0004)
#define errInternal                    AAL_ErrNum     (0x0005)
#define errMethodFailure               AAL_ErrNum     (0x0006)
#define errMethodNotImplemented        AAL_ErrNum     (0x0007)
#define errNoFactory                   AAL_ErrNum     (0x0008)
#define errSendMessageFailure          AAL_ErrNum     (0x0009)
#define errFileError                   AAL_ErrNum     (0x000A) // Generic File Error, typically followed by reasCheckErrno
#define errMemory                      AAL_ErrNum     (0x000B)
#define errDevice                      AAL_ErrNum     (0x000C)
#define errAFUPackage                  AAL_ErrNum     (0x000D)
#define errAFUTransactionNotFound	   AAL_ErrNum     (0x000E)
#define errServiceNotFound             AAL_ErrNum     (0x000F)
#define errAllocationFailure           AAL_ErrNum     (0x0010)
#define errReleaseFailure              AAL_ErrNum     (0x0011)
#define errProxyInvalid                AAL_ErrNum     (0x0012)
#define errProxyDestroy                AAL_ErrNum     (0x0013)

#define reasCauseUnknown               AAL_ReasCode   (0x0001)
#define reasFeatureNotSupported        AAL_ReasCode   (0x0002)
#define reasInvalidParameter           AAL_ReasCode   (0x0003)
#define reasMissingParameter           AAL_ReasCode   (0x0004)
#define reasNoFactoryForClass          AAL_ReasCode   (0x0005)
#define reasParameterNameInvalid       AAL_ReasCode   (0x0006)
#define reasParameterTypeInvalid       AAL_ReasCode   (0x0007)
#define reasParameterUnknown           AAL_ReasCode   (0x0008)
#define reasParameterValueInvalid      AAL_ReasCode   (0x0009)
#define reasResourcesNotAvailable      AAL_ReasCode   (0x000A)
#define reasUnknown                    AAL_ReasCode   (0x000B)
#define reasUnknownMessage             AAL_ReasCode   (0x000C)
#define reasUnsupportedMethod          AAL_ReasCode   (0x000D)
#define reasCheckErrno                 AAL_ReasCode   (0x000E) // err is file error, reason code is in errno
#define reasInternalError              AAL_ReasCode   (0x000F) // internal integrity check error found
#define reasNoDevice                   AAL_ReasCode   (0x0010)
#define reasMapFailed                  AAL_ReasCode   (0x0011)
#define reasBadConfiguration           AAL_ReasCode   (0x0012)
#define reasDeviceBusy                 AAL_ReasCode   (0x0013)
#define reasInvalidState               AAL_ReasCode   (0x0014)
#define reasNoSharedLibrary            AAL_ReasCode   (0x0015)
#define reasInvalidService             AAL_ReasCode   (0x0016)
#define reasNotImplemented             AAL_ReasCode   (0x0017)
#define reasSubModuleFailed            AAL_ReasCode   (0x0018) // some lower level module failed unexpectedly
#define reasCommunicationFailed        AAL_ReasCode   (0x0019) // communications problem
#define reasMissingInterface           AAL_ReasCode   (0x001A)
#define reasParentReleased             AAL_ReasCode   (0x001B)
#define reasSingletoneExists           AAL_ReasCode   (0x001C)
#define reasTimeout                    AAL_ReasCode   (0x001D)
#define reasRuntimeNotStarted          AAL_ReasCode   (0x001E)
#define reasRuntimeAlreadyStarted      AAL_ReasCode   (0x001F)

#define strNoError                     "No error."
#define strInternalSystemFailure       "Internal system failure!"
#define strNoFactoryDescr              "No appropriate factory"
#define strNoResourceDescr             "Resources unavailable"
#define strUnknownCause                "Unknown cause"
#define strUnknownMessage              "Unknown message"
#define strNotImplemented              "Not implemented"
#define strFeatureNotSupported         "Unsupported feature"
#define strBadMessage                  "Bad message"
#define strInvalidParameter            "Invalid Parameter"
#define strInvalidParameterType        "Invalid Parameter type"
#define strNoDevice                    "No Device"
#define strMapFailed                   "Workspace Map operation failed"
#define strBadConfiguration            "Bad or invalid configuration information in registry"
#define strMissingParameter            "Bad or missing parameter"
#define strDeviceBusy                  "Device busy"
#define strInvalidState                "Invalid state"
#define strNoManifest                  "No Manifest present"
#define strInvalidService              "Specified service invalid."
#define strMissingInterface            "Missing required interface"
/// @}

/*=============================================================================
===============================================================================
========================= Define AAL-Specific Classes =========================
===============================================================================
=============================================================================*/

/// @addtogroup CommonBase
/// @{

/// IBase interface id.
#define iidBase                      __AAL_IID(AAL_sysAAL, 0x0001)

/// @}

/// @addtogroup Services
/// @{

/// IServiceModule interface id.
#define iidServiceProvider           __AAL_IID(AAL_sysAAL, 0x0003)   // Class implements both service and proxy

/// IAALService interface id.
#define iidService                   __AAL_IID(AAL_sysAAL, 0x0004)   // Class implements service
#define iidServiceBase               __AAL_IID(AAL_sysAAL, 0x0005)   // Class implements service base

#define iidServiceClient             __AAL_IID(AAL_sysAAL, 0x0006)   // Class implements IServiceClient

#define iidRuntimeClient             __AAL_IID(AAL_sysAAL, 0x0007)   // Class implements IRuntimeClient

#define iidRuntime                   __AAL_IID(AAL_sysAAL, 0x0008)   // Class implements IRuntime

#define iidServiceRevoke             __AAL_IID(AAL_sysAAL, 0x0009)   // IServiceRevoke

/// @}

/// @addtogroup Events
/// @{

/// IEvent interface id.
#define iidEvent                     __AAL_Evt(AAL_sysAAS, 0x0000)
/// IExceptionEvent interface id.
#define iidExEvent                   __AAL_ExEvt(AAL_sysAAS, 0x0000)
/// ITransactionEvent interface id.
#define iidTranEvent                 __AAL_TranEvt(AAL_sysAAS, 0x0000)
/// IExceptionTransactionEvent interface id.
#define iidExTranEvent               __AAL_ExTranEvt(AAL_sysAAS, 0x0000)

/// IReleaseRequestEvent interface id.
#define iidReleaseRequestEvent       __AAL_ExTranEvt(AAL_sysAAS, 0x0000)

/// @}

/// IAASServiceContainer interface id.
/// @ingroup SysServices
#define iidServiceContainer          __AAL_IID(AAL_sysAAS, 0x0001)
/// Registrar interface id.
/// @ingroup Registrar
#define iidRegistrar                 __AAL_IID(AAL_sysAAS, 0x0002)
#define iidMDS                       __AAL_IID(AAL_sysAAS, 0x0003)

#define iidFactory                   __AAL_IID(AAL_sysAAS, 0x0004)
/// @ingroup uAIA
#define iidAIA                       __AAL_IID(AAL_sysAAS, 0x0005)
/// @ingroup AFU
#define iidAFU                       __AAL_IID(AAL_sysAAS, 0x0006)
/// @ingroup ResMgr
#define iidRMCS                      __AAL_IID(AAL_sysAAS, 0x0007)
#define iidResMgr                    __AAL_IID(AAL_sysAAS, 0x0008)
#define iidResMgrClient              __AAL_IID(AAL_sysAAS, 0x0009)
/// @ingroup ServiceBroker
#define iidServiceBroker             __AAL_IID(AAL_sysAAS, 0x000A)

/// @ingroup uAIA
#define iidAIA_uAIA                  __AAL_IID(AAL_sysAIA, 0x0005)

#define iidAFU_Factory               __AAL_IID(AAL_sysAFU, 0x0001)

#define iidManagementAFU             __AAL_IID(AAL_sysManagementAFU, 0x0001)

/// @ingroup IResConf
#define iidResConfClient             __INTC_IID(AAL_sysResConf, 0x0001)
#define iidResConfService            __INTC_IID(AAL_sysResConf, 0x0002)

#define iidAFUDev                    __AAL_IID(AAL_sysUAIA, 0x0001)


/*=============================================================================
===============================================================================
========================= Define AAL-Specific Events ==========================
===============================================================================
=============================================================================*/

/******************************************************************************
* Define System Specific macros
******************************************************************************/
#define __AAL_SysEvt(Num)                 __AAL_Evt      (AAL_sysAAS, Num)
#define __AAL_SysTranEvt(Num)             __AAL_TranEvt  (AAL_sysAAS, Num)
#define __AAL_SysExTranEvt(Num)           __AAL_ExTranEvt(AAL_sysAAS, Num)
#define __AAL_SysErrNum(Num)              __AAL_ErrNum   (AAL_sysAAS, Num)
#define __AAL_SysReasCode(Num)            __AAL_ReasCode (AAL_sysAAS, Num)
#define __AAL_SysNid(Num)                 __AAL_Nid      (AAL_sysAAS, Num)


/// @addtogroup AALRUNTIME
/// @{

/// Interface id of event sent in response to successful SystemInit.
#define tranevtSystemInit                 __AAL_SysTranEvt   (0x0001)
/// Interface id of exception event sent in response to failed SystemInit.
#define extranevtSystemInit               __AAL_SysExTranEvt (0x0001)

/// Interface id of event sent in response to successful SystemStop.
#define tranevtSystemStop                 __AAL_SysTranEvt   (0x0002)
/// Interface id of exception event sent in response to failed SystemStop.
#define exttranevtSystemStop              __AAL_SysExTranEvt (0x0002)
#define tranevtServiceShutdown            __AAL_SysTranEvt   (0x0003)
#define exttranevtServiceShutdown         __AAL_SysExTranEvt (0x0003)
#define exttranevtSystemStart             __AAL_SysExTranEvt (0x0004)

/// @}


#define exttranevtNotImplemented          __AAL_SysExTranEvt (0x0004)

// System event codes
#define errSysSystemStarted               __AAL_SysErrNum(0x0001)
#define reasSystemAlreadyStarted          __AAL_SysReasCode(0x0001)
#define strSystemAlreadyStarted           "System already started"
#define errSysNoClient                    __AAL_SysErrNum(0x0002)
#define reasSystemMissingInterface        __AAL_SysReasCode(0x0002)
#define strSystemMissingInterface         "missing required interface,"

#define errSystemTimeout                  __AAL_SysErrNum(0x0003)
#define reasSystemTimeout                 __AAL_SysReasCode(0x0003)
#define strSystemTimeout                  "Timeout"

#define errSysAALLibLdr                   __AAL_SysErrNum(0x0004)
#define reasAALLibLdr                     __AAL_SysReasCode(0x0004)
#define strAALLibLdr                      "AAL Dyn Load Library Error"

#define errSysSystemPermission            __AAL_SysErrNum(0x0005)
#define reasAALLibLdr                     __AAL_SysReasCode(0x0004)
#define strAALLibLdr                      "AAL Dyn Load Library Error"


// System Specific Keys
#define nidSystemRegistrarPath            ("_AAL_SYSTEM_REGISTRAR_PATH")
#define nidSystemMode                     __AAL_SysNid (0x0001)

/******************************************************************************
* Define Factory Specific macros
******************************************************************************/
#define __AAL_FactTranEvt(Num)            __AAL_TranEvt  (AAL_sysFactory, Num)
#define __AAL_FactExTranEvt(Num)          __AAL_ExTranEvt(AAL_sysFactory, Num)
#define __AAL_FactErrNum(Num)             __AAL_ErrNum   (AAL_sysFactory, Num)
#define __AAL_FactReasCode(Num)           __AAL_ReasCode (AAL_sysFactory, Num)

/// @addtogroup SysEvents
/// @{

/// IObjectCreatedEvent interface id.
#define tranevtFactoryCreate              __AAL_FactTranEvt          (0x0001)
/// ObjectCreatedExceptionEvent interface id.
#define extranevtFactoryCreate            __AAL_FactExTranEvt        (0x0002)

/// @}

#define reasParameterNameInvalidToFactory __AAL_FactReasCode         (0x0001)
#define reasMissingParameterToFactory     __AAL_FactReasCode         (0x0002)

#define tranevtResourceManagerClientEvent          __AAL_FactTranEvt    (0x0003)
#define exttranevtResourceManagerClientEvent       __AAL_FactExTranEvt  (0x0003)
#define tranevtResourceManagerConfigRecordEvent    __AAL_FactTranEvt    (0x0004)
#define exttranevtResourceManagerConfigRecordEvent __AAL_FactExTranEvt  (0x0004)

#define errRMCErrorEvent                  __AAL_FactErrNum(0x0001)
#define reasRMCSpecific                   __AAL_FactReasCode(0x0001)
#define reasRMCNoResource                 __AAL_FactReasCode(0x0002)
#define strRMCError                       "Resource Manager Client returned an error"
#define strRMCNoResourceError             "Resource not available"

#define strInvalidParameterToFactory   "Factory reports an invalid or unrecognized parameter"
#define strMissingParameterToFactory   "Factory reports missing required parameter"


/******************************************************************************
* Define Resource Manager Client Service macros
******************************************************************************/
#define __AAL_RmcsTranEvt(Num)         __AAL_TranEvt  (AAL_sysResMgrClient, Num)
#define __AAL_RmcsExTranEvt(Num)       __AAL_ExTranEvt(AAL_sysResMgrClient, Num)
#define __AAL_RmcsErrNum(Num)          __AAL_ErrNum   (AAL_sysResMgrClient, Num)
#define __AAL_RmcsReasCode(Num)        __AAL_ReasCode (AAL_sysResMgrClient, Num)

/// @addtogroup ResMgr
/// @{

#define tranevtRequestDevice           __AAL_RmcsTranEvt       (0x0001)
#define extranevtRequestDevice         __AAL_RmcsExTranEvt     (0x0001)

/// @}

/******************************************************************************
* Define Universal AIA Service Specific Macros
******************************************************************************/
#define __AAL_UAIAEvt(Num)             __AAL_Evt      (AAL_sysUAIA, Num)
#define __AAL_UAIAExEvt(Num)           __AAL_ExEvt    (AAL_sysUAIA, Num)
#define __AAL_UAIATranEvt(Num)         __AAL_TranEvt  (AAL_sysUAIA, Num)
#define __AAL_UAIAExTranEvt(Num)       __AAL_ExTranEvt(AAL_sysUAIA, Num)
#define __AAL_UAIAErrNum(Num)          __AAL_ErrNum   (AAL_sysUAIA, Num)
#define __AAL_UAIAReasCode(Num)        __AAL_ReasCode (AAL_sysUAIA, Num)

/// @addtogroup uAIA
/// @{

#define evtUIDriverClientEvent         __AAL_UAIAEvt           (0x0001)

#define evtUIDriverClientEvent         __AAL_UAIAEvt           (0x0001)
#define tranevtUIDriverClientEvent     __AAL_UAIATranEvt       (0x0001)
#define exttranevtUIDriverClientEvent  __AAL_UAIAExTranEvt     (0x0001)

#define tranevtBindAFUDevEvent         __AAL_UAIATranEvt       (0x0002)
#define exttranevtBindAFUDevEvent      __AAL_UAIAExTranEvt     (0x0002)

#define tranevtInitAFUDevEvent         __AAL_UAIATranEvt       (0x0003)
#define exttranevtInitAFUDevEvent      __AAL_UAIAExTranEvt     (0x0003)

#define tranevtUnBindAFUDevEvent       __AAL_UAIATranEvt       (0x0004)
#define exttranevtUnBindAFUDevEvent    __AAL_UAIAExTranEvt     (0x0004)

/// @}

/******************************************************************************
* Define Event Deliver Service Specific macros
******************************************************************************/
#define __AAL_EDSTranEvt(Num)         __AAL_TranEvt  (AAL_sysEDS, Num)
#define __AAL_EDSExTranEvt(Num)       __AAL_ExTranEvt(AAL_sysEDS, Num)
#define __AAL_EDSErrNum(Num)          __AAL_ErrNum   (AAL_sysEDS, Num)
#define __AAL_EDSReasCode(Num)        __AAL_ReasCode (AAL_sysEDS, Num)

/// @addtogroup MDS 
/// @{

/// Interface id of exception event sent on Event Delivery Service failure.
#define extranevtEDS                  __AAL_FactExTranEvt     (0x0002)

#define strEnknownEDSfailure           "Unknown EDS Failure"

/// @}

/*=============================================================================
====================== Define Registrar Specific macros =======================
=============================================================================*/

/******************************************************************************
* Top-Level Macros
******************************************************************************/
#define __AAL_RegTranEvt(Num)          __AAL_TranEvt  (AAL_sysRegistrar, Num)
#define __AAL_RegExTranEvt(Num)        __AAL_ExTranEvt(AAL_sysRegistrar, Num)
#define __AAL_RegErrNum(Num)           __AAL_ErrNum   (AAL_sysRegistrar, Num)
#define __AAL_RegReasCode(Num)         __AAL_ReasCode (AAL_sysRegistrar, Num)
#define AAL_RegKey(Key)                AAL_RegistrarKey(AAL_vendAAL,AAL_sysRegistrar, (Key))
#define AAL_RegEnum(Key,Enum)          AAL_Enumeration(AAL_vendAAL,AAL_sysRegistrar, (Key), (Enum))

// Someone else's enum's would look like this:
// #define MY_RegEnum(Key,Enum)        AAL_ID(MY_vend,AAL_mySys or AAL_sysRegistrar okay,AAL_NotException, Key, Enum)

/******************************************************************************
* Registrar Enums
******************************************************************************/

/// @addtogroup Registrar
/// @{

#define AAL_typeEnumRecordType            (AAL_typeBase+0x0001)            /* This is a Key/Value pair in the Registrar Database, */
                                                                           /*    Where the NVP is an enum of RecordType */
#define enumRegRecordType_Key             AAL_RegEnum(AAL_typeEnumRecordType, 0x0000) /* RecordType key */
#define enumRegRecordType_AFU             AAL_RegEnum(AAL_typeEnumRecordType, 0x0001) /* RecordType is AFU_ID */
#define enumRegRecordType_AHM             AAL_RegEnum(AAL_typeEnumRecordType, 0x0002) /* RecordType is AFU_ID */
#define enumRegRecordType_AIA             AAL_RegEnum(AAL_typeEnumRecordType, 0x0003) /* RecordType is AIA_ID */
#define enumRegRecordType_Instance        AAL_RegEnum(AAL_typeEnumRecordType, 0x0004) /* RecordType is Instance */
#define enumRegRecordType_Goal            AAL_RegEnum(AAL_typeEnumRecordType, 0x0005) /* RecordType is Goal */
#define enumRegRecordType_Configure       AAL_RegEnum(AAL_typeEnumRecordType, 0x0006) /* RecordType is Configure */
#define enumRegRecordType_CostMetric      AAL_RegEnum(AAL_typeEnumRecordType, 0x0007) /* RecordType is Cost Metric */
#define enumRegRecordType_Service         AAL_RegEnum(AAL_typeEnumRecordType, 0x0008) /* RecordType defines a Service */


#define AAL_typeEnumShareType             (AAL_typeBase+0x0002)            /* This is a Key/Value pair in the Registrar Database,
                                                                                 Where the NVP is an enum of ShareType. */
#define enumRegShareType_Key              AAL_RegEnum(AAL_typeEnumShareType,  0x0000) /* ShareType key - what kind of sharing can an AFU support? */
#define enumRegShareType_Shareable        AAL_RegEnum(AAL_typeEnumShareType,  0x0001) /* AFU can be serially shared (context can switch between tasks) */
#define enumRegShareType_NotShareable     AAL_RegEnum(AAL_typeEnumShareType,  0x0002) /* AFU is exclusively allocated, DEFAULT */
#define enumRegShareType_ShareableToMax   AAL_RegEnum(AAL_typeEnumShareType,  0x0003) /* AFU can be serially shared to a maximum number of tasks */

#define AAL_typeEnumBusType               (AAL_typeBase+0x0003)            /* This is a Key/Value pair in the Registrar Database,
                                                                                 Where the NVP is an enum of BusType.
                                                                                 Keep this in step with aaldevice.h:aal_bus_types_e */
#define enumRegBusType_Key                AAL_RegEnum(AAL_typeEnumBusType,    0x0000) /* BusType key - what kind of sharing can an AFU support? */
                                                                           /* The data value is the direct value of the aaldevice.h:aal_bus_types_e */
//#define enumRegBusType_Unknown            AAL_RegEnum(AAL_typeEnumBusType,    0x0001) /* Bus is Unknown */
//#define enumRegBusType_FSB                AAL_RegEnum(AAL_typeEnumBusType,    0x0002) /* Bus is FSB */
//#define enumRegBusType_QPI                AAL_RegEnum(AAL_typeEnumBusType,    0x0003) /* Bus is QPI */
//#define enumRegBusType_PCI                AAL_RegEnum(AAL_typeEnumBusType,    0x0004) /* Bus is PCIe */
//#define enumRegBusType_ASM                AAL_RegEnum(AAL_typeEnumBusType,    0x0005) /* Bus is ASM */
//#define enumRegBusType_Prop               AAL_RegEnum(AAL_typeEnumBusType,    0x0006) /* Bus is Proprietary */

#define AAL_typeEnumConfig                (AAL_typeBase+0x0004)            /* This is a Key/Value pair in the Registrar Database, */
                                                                           /*    Where the NVP is an enum of Configuration Action Type */
#define enumRegConfig_Key                 AAL_RegEnum(AAL_typeEnumConfig,     0x0000) /* Key - what configuration is required? */
#define enumRegConfig_ReturnHandle        AAL_RegEnum(AAL_typeEnumConfig,     0x0001) /* Reconfiguration Action is just return the handle */
#define enumRegConfig_Configure           AAL_RegEnum(AAL_typeEnumConfig,     0x0002) /* Reconfiguration Action is to configure (e.g. download) */
#define enumRegConfig_Quiesce             AAL_RegEnum(AAL_typeEnumConfig,     0x0003) /* Reconfiguration Action is to quiesce first */
#define enumRegConfig_FromFlash           AAL_RegEnum(AAL_typeEnumConfig,     0x0004) /* Reconfiguration Action is to copy from local flash */
#define enumRegConfig_Default             AAL_RegEnum(AAL_typeEnumConfig,     0x0005) /* Not an action. Used to set default cost metric */

/******************************************************************************
* Registrar Keys, all of type AAL_typeKey
******************************************************************************/

// Basic ID's
#define keyRegAFU_ID                      "AAL_keyRegAFU_ID"
//#define keyRegAFU_ID                      AAL_RegKey(0x0001)   /* AFU_ID key, data is AFU_ID as string GUID */
#define keyRegAHM_ID                      "AAL_keyRegAHM_ID"
//#define keyRegAHM_ID                      AAL_RegKey(0x0002)   /* AHM_ID key, data is AHM_ID as btUnsigned64bitInt */
#define keyRegAIA_ID                      "AAL_keyRegAIA_ID"
//#define keyRegAIA_ID                      AAL_RegKey(0x0003)   /* AIA_ID key, data is AIA_ID as btUnsigned64bitInt */
#define keyRegPIP_ID                      "AAL_keyRegPIP_ID"
//#define keyRegPIP_ID                      AAL_RegKey(0x0004)   /* PIP_ID key, data is PIP_ID as btUnsigned64bitInt */
#define keyRegPid                         "AAL_keyRegPid"
//#define keyRegPid                         AAL_RegKey(0x0005)   /* Process id as bt32bitInt */
#define keyRegNumAllocated                "AAL_keyRegNumAllocated"
//#define keyRegNumAllocated                AAL_RegKey(0x0006)   /* Number of times this AFU has been allocated by the Resource Manager */
#define keyRegRecordNum                   "AAL_keyRegRecordNum"
//#define keyRegRecordNum                   AAL_RegKey(0x0007)   /* Record Number */

#define keyRegBusType                     "AAL_keyRegBusType"
//#define keyRegBusType                     AAL_RegKey(0x0010)   /* DataType: bt32bitInt, defined by aal_bus_types_e */

#define keyRegBusNumber                   "AAL_keyRegBusNumber"
//#define keyRegBusNumber                   AAL_RegKey(0x0011)   /* DataType: btUnsigned32bitInt
//                                                                  Bus type is defined by enumeration. This is which bus of
//                                                                  that type in the system */
#define keyRegDeviceNumber                "AAL_keyRegDeviceNumber"
//#define keyRegDeviceNumber                AAL_RegKey(0x0012)   /* DataType: btUnsigned32bitInt
//                                                                  Which slot (PCI) or socket (FSB, QPI) on this bus */

#define keyRegFunctionNumber              "AAL_keyRegFunctionNumber"
//                                                                  Channel on the device, e.g. SPL interface (0) or
//                                                                  sub-device number. -1 if not defined */

#define keyRegSubDeviceNumber               "AAL_keyRegSubDeviceNumber"
//                                                                  Subdevice within a B:D:F

#define keyRegInstanceNumber              "AAL_keyRegInstanceNumber"
//                                                                  Instance identifier within a B:D:F:S

#define keyRegDeviceAddress               "AAL_keyRegDeviceAddress"
//#define keyRegDeviceAddress               AAL_RegKey(0x0014)   /* DataType: btUnsigned64bitInt (also, btID, and the native key type of an NVS
//                                                                  Encoded version of BusType, BusNumber, DeviceNumber, ChannelNumber */
#define keyRegDeviceType                  "AAL_keyRegDeviceType"
//#define keyRegDeviceType                  AAL_RegKey(0x0015)   /* DataType: btUnsigned32bitInt, defined by aal_device_type_e */
#define keyRegHandle                      "AAL_keyRegHandle"
//#define keyRegHandle                      AAL_RegKey(0x0016)   /* DataType: Initially btObjectType (i.e. void*)
//                                                                  Eventually we will need a RegHandle Array, e.g. btObjectType Array (i.e. void**)
//                                                                  Suggest that is handled by a different key and a special value for this one (e.g. -1)
//                                                                  This is a Device or other Handle defined by the kernel */
#define keyRegDeviceState                 "AAL_keyRegDeviceState"
//#define keyRegDeviceState                 AAL_RegKey(0x0017)   /* DataType: bt32bitInt, probably eventually an enum
//                                                                  1 if Activated, 0 if Quiescent */
#define keyRegNumOwners                   "AAL_keyRegNumOwners"
//#define keyRegNumOwners                   AAL_RegKey(0x0018)   /* DataType: bt32bitInt, number of owners of the device */
#define keyRegOwners                      "AAL_keyRegOwners"
//#define keyRegOwners                      AAL_RegKey(0x0019)   /* DataType: bt32bitIntArray[] of Pids */
#define keyRegVendor                      "AAL_keyRegVendor"
//#define keyRegVendor                      AAL_RegKey(0x001A)   /* DataType: btUnsigned32bitInt, Vendor ID, using PCI SIG value */
#define keyRegName                        "AAL_keyRegName"
//#define keyRegName                        AAL_RegKey(0x001B)   /* DataType: btcString (was keyRegAFU_Name)
//                                                                  [OPTIONAL] a string representation of the function.
//                                                                  This was originally for AFU's, but in fact it also holds
//                                                                  true for an Service, in general, e.g. AIA, general Service,
//                                                                  AFU, or anything else */
#define keyRegMaxOwners                   "AAL_keyRegMaxOwners"
//#define keyRegNumOwners                   AAL_RegKey(0x001C)   /* DataType: bt32bitInt, ,ax number of owners of the device */


/************
 * AFU Record
 ***********/
// These keys are required, and are defined elsewhere
//      enumRegRecordType_Key:   enumRegRecordType_AFU
//      keyRegAFU_ID:            <AFU_ID>
//      keyRegPIP_ID:            <PIP_ID>
//      keyRegAHM_ID:            <Array of AHM_IDs upon which this AFU can be instantiated, see AFU Embedded Records, below>
//      keyRegAIA_ID:            <Array of AIA_IDs defining the AIAs that must be loaded by the Factory>

// AFU Record-specific REQUIRED Keys
#define keyRegAFU_Package                 "AAL_keyRegAFU_Package"
//#define keyRegAFU_Package                 AAL_RegKey(0x0120)   /* DataType: btcString
//                                                                  Fully qualified path and file name of the AFU Package
//                                                                  executable that the Factory will launch in order to
//                                                                  instantiate this AFU */

// These keys are optional, and are defined elsewhere
//      enumRegShareType_Key:    <one of the ShareType enumerations, defaults to enumRegShareType_NotShareable>

// AFU Record-specific OPTIONAL Keys
#define keyRegAFU_Name                    AAL_RegKey(0x0130)   /* DataType: btcString (TODO:DEPRECATED in favor of keyRegName)
                                                                  [OPTIONAL] a string representation of the AFU's function */
#define keyRegAFU_VendorName              AAL_RegKey(0x0131)   /* DataType: btcString
                                                                  [OPTIONAL] Vendor Name */
#define keyRegAFU_ShareMax                "AAL_keyRegAFU_ShareMax"
//#define keyRegAFU_ShareMax                AAL_RegKey(0x0132)   /* DataType: bt32bitInt
//                                                                  [OPTIONAL] max # of shared tasks if enumRegShareType_Key
//                                                                  is enumRegShareType_ShareableToMax */
#define keyRegAFU_Links_Required          AAL_RegKey(0x0133)   /* DataType: Either LinkSet, or Vendor-Defined Enumeration
                                                                  [OPTIONAL] One array element for each configuration of the
                                                                  hardware AFU's that is sufficient for this AFU to be instantiated.
                                                                  This NVP must be in an embedded AHM Record, see below.
                                                                  DEFAULT: each AFU is single and can run anywhere on the board(s),
                                                                  and there are keyRegAHM_NumChan of them */
#define keyRegAFU_Handle                  AAL_RegKey(0x0134)   /* DataType: btObjectType */

// AFU Record Embedded Sub-Records:
//
//    AHM Record: For each value in the keyRegAHM_ID, there may be an embedded AHM Record with additional information
//                   about that AHM, e.g. keyRegAFU_Configurations_Required
//                The name is the AHM_ID.
//
//    Configure Record: For any board address (fully qualified or not), there may be an embedded Configure Record with
//                   additional information about how to configure this AFU onto that board.
//                The name is the Board Address with all the 4 fields packed into a single 64bit uint.
//                It is known to be a Configure Record because it contains a enumRegRecordType_Key:enumRegRecordType_Configure NVP

/************
 * AHM Record
 ***********/
// These keys are required, and are defined elsewhere
//      enumRegRecordType_Key:   enumRegRecordType_AHM
//      keyRegAHM_ID:            <AHM_ID>

// AHM Record-specific REQUIRED keys
#define keyRegAHM_NumChan                 AAL_RegKey(0x0150)   /* DataType: bt32bitInt
                                                                  Number of AFU's on the board. Unless other distinguishing
                                                                  information is provided in a form TBD, all AFU's are
                                                                  considered equivalent */
// AHM Record-specific OPTIONAL keys
#define keyRegAHM_VendorName              AAL_RegKey(0x0160)   /* DataType: btcString
                                                                  [OPTIONAL] Vendor Name */
#define keyRegAHM_Links_Provided          AAL_RegKey(0x0161)   /* DataType: LinkMap
                                                                  [OPTIONAL] Map defining the configuration of the
                                                                  hardware AFU's that is provided on this board. If missing,
                                                                  then assumed to be fully inter-connected based on NumChan.
                                                                  See keyRegAFU_Links_Required. */
/************
 * AIA Record
 ***********/
// These keys are required, and are defined elsewhere
//       enumRegRecordType_Key:  enumRegRecordType_AIA
//       keyRegAIA_ID:           <AIA_ID>

// AIA Record-specific REQUIRED keys
#define keyRegAIAPackage                  AAL_RegKey(0x0180)   /* DataType: btcString
                                                                  Fully qualified path and file name of the executable
                                                                  that the Factory will load in order to instantiate this AIA */
/******************
 * Configure Record
 *****************/
// These keys are required, and are defined elsewhere
//       enumRegRecordType_Key:  enumRegRecordType_Configure
//       enumRegConfig_Key:   <array of Config Enumerations>

// Configure Record-specific OPTIONAL keys
#define keyRegConfigCostMetric            AAL_RegKey(0x0190)   /* DataType: bt32bitInt, lower number is lower cost
                                                                  Cost metric to instantiate this configuration, computed
                                                                  by resource manager from the configuration enumeration */
#define keyRegConfigProcIDs               AAL_RegKey(0x01A1)   /* DataType: Array of PIDs (either void* or u64)
                                                                  Array of PIDs that need to be quiesced if the configuration
                                                                  enumeration contains a quiesce. Cost metric for Quiesce
                                                                  is applied for each PID in the array */

/**********************************************************************************
 * Internal Record Bases, shift to always be in unused area after the above records
 *********************************************************************************/
#define AALResourceManagerKeyBase         (0x0400)
#define AALRMB                            AALResourceManagerKeyBase /* alias */
#define keyRegInstUpdateType              AAL_RegKey(AALRMB+1)


/******************************************************************************
* Registrar Events and Errors
******************************************************************************/

BEGIN_NAMESPACE(AAL)
#define tranevtRegistrarOpen           __AAL_RegTranEvt(0x0001)
END_NAMESPACE(AAL)

//#define extranevtRegistrarOpen         __AAL_RegExTranEvt(0x0001)  // not yet needed
#define reasRegOpen                    __AAL_RegReasCode (0x0001)
#define strRegOpen                     "Registrar::Open"

#define tranevtRegistrarClose          __AAL_RegTranEvt  (0x0002)
//#define extranevtRegistrarClose        __AAL_RegExTranEvt(0x0002)  // not yet needed
#define reasRegClose                   __AAL_RegReasCode (0x0002)
#define strRegClose                    "Registrar::Close"

#define tranevtRegistrarRegister       __AAL_RegTranEvt  (0x0003)
//#define extranevtRegistrarRegister     __AAL_RegExTranEvt(0x0003)
#define reasRegRegister                __AAL_RegReasCode (0x0003)
#define strRegRegister                 "Registrar::Register"

#define tranevtRegistrarDeRegister     __AAL_RegTranEvt  (0x0004)
#define extranevtRegistrarDeRegister   __AAL_RegExTranEvt(0x0004)
#define reasRegDeRegister              __AAL_RegReasCode (0x0004)
#define strRegDeRegister               "Registrar::DeRegister"

#define tranevtRegistrarCommit         __AAL_RegTranEvt  (0x0005)
//#define extranevtRegistrarCommit       __AAL_RegExTranEvt(0x0005)
#define reasRegCommit                  __AAL_RegReasCode (0x0005)
#define strRegCommit                   "Registrar::Commit"

#define tranevtRegistrarFind           __AAL_RegTranEvt  (0x0006)
//#define extranevtRegistrarFind         __AAL_RegExTranEvt(0x0006)
#define reasRegFind                    __AAL_RegReasCode (0x0006)
#define strRegFind                     "Registrar::Find"

#define tranevtRegistrarFindNext       __AAL_RegTranEvt  (0x0007)
#define extranevtRegistrarFindNext     __AAL_RegExTranEvt(0x0007)
#define reasRegFindNext                __AAL_RegReasCode (0x0007)
#define strRegFindNext                 "Registrar::FindNext"

#define tranevtRegistrarGet            __AAL_RegTranEvt  (0x0008)
#define extranevtRegistrarGet          __AAL_RegExTranEvt(0x0008)
#define reasRegGet                     __AAL_RegReasCode (0x0008)
#define strRegGet                      "Registrar::Get"

// Returned when attempt to access un-opened database
#define extranevtRegistrarDatabaseNotOpen __AAL_RegExTranEvt(0x0009)

#define errRegDatabaseNotLoaded        __AAL_RegErrNum   (0x0009)
#define strRegDatabaseNotLoaded        "Call made when Register Database Not Loaded, operation failed"

#define errRegLoadDatabaseFailed       __AAL_RegErrNum   (0x000A)
// Accompanied by reas #define reasCheckErrno
// Specific file error on database file open will be in errno
// String associated with Errno will be in perror

#define errRegWriteDatabaseFailed      __AAL_RegErrNum   (0x000B)
#define reasRegCheckErrno              __AAL_RegReasCode (0x000B)


/// @}


/******************************************************************************
* Define AIA Specific macros
******************************************************************************/

/// @addtogroup uAIA
/// @{

#define __AIA_TranEvt(Num)             __AAL_TranEvt  (AAL_sysAIA, Num)
#define __AIA_ExTranEvt(Num)           __AAL_ExTranEvt(AAL_sysAIA, Num)
#define __AIA_ErrNum(Num)              __AAL_ErrNum   (AAL_sysAIA, Num)
#define __AIA_ReasCode(Num)            __AAL_ReasCode (AAL_sysAIA, Num)

#define tranevtAIA_Release             __AIA_FactTranEvt  (0x0001)
#define extranevtAIA_Release           __AIA_FactExTranEvt(0x0001)

#define tranevtAIA_Shutdown            __AIA_FactTranEvt  (0x0002)
#define extranevtAIA_Shutdown          __AIA_FactExTranEvt(0x0002)

#define tranevtAIA_SessionCreate       __AIA_TranEvt  (0x0003)
#define extranevtAIA_SessionCreate     __AIA_ExTranEvt(0x0003)

#define reasAIAFailedToStart           __AIA_FactReasCode (0x0001)

#define strAIAFailedToStart            "AIA Failed to start"

#define tranevtAIA_WorkSpaceAllocate   __AIA_TranEvt  (0x0003)
#define extranevtAIA_WorkSpaceAllocate __AIA_ExTranEvt(0x0003)

#define tranevtAIA_SendMessage         __AIA_TranEvt  (0x0004)
#define extranevtAIA_SendMessage       __AIA_ExTranEvt(0x0004)

#define tranevtAIA_WorkSpaceFree       __AIA_TranEvt  (0x0007)
#define extranevtAIA_WorkSpaceFree     __AIA_ExTranEvt(0x0008)

#define errAIAWorkSpace                __AIA_ErrNum(0x0001)
#define reasAIANoWorkSpaceManager      __AIA_ReasCode(0x0001)
#define reasAIANoMemeory               __AIA_ReasCode(0x0002)
#define strAIAWorkSpaceAllocateFailed  "No workspace manager"
#define strAIANoMemory                 "No Memory"

/// @}

/******************************************************************************
* Define AFU Specific macros
******************************************************************************/

/// @addtogroup AFU
/// @{

#define __AFU_FactTranEvt(Num)         __AAL_TranEvt  (AAL_sysAFU, Num)
#define __AFU_FactExTranEvt(Num)       __AAL_ExTranEvt(AAL_sysAFU, Num)
#define __AFU_FactErrNum(Num)          __AAL_ErrNum   (AAL_sysAFU, Num)
#define __AFU_FactReasCode(Num)        __AAL_ReasCode (AAL_sysAFU, Num)

#define tranevtAFU                     __AFU_FactTranEvt  (0x0000)
#define extranevtAFU                   __AFU_FactExTranEvt(0x0000)

#define tranevtAFU_FactoryCreate       __AFU_FactTranEvt  (0x0001)
#define extranevtAFU_FactoryCreate     __AFU_FactExTranEvt(0x0001)

#define reasAFUNoFactory               __AFU_FactReasCode (0x0001)

#define strAFUNoFactory                "No appropriate factory for requested AFU class"

#define tranevtAFU_FactoryDestroy      __AFU_FactTranEvt  (0x0002)
#define extranevtAFU_FactoryDestroy    __AFU_FactExTranEvt(0x0002)

#define tranevtAFU_WorkSpaceAllocate   __AFU_FactTranEvt  (0x0003)
#define extranevtAFU_WorkSpaceAllocate __AFU_FactExTranEvt(0x0003)

#define errAFUWorkSpace                __AFU_FactErrNum(0x0001)
#define reasAFUNoWorkSpaceManager      __AFU_FactReasCode(0x0001)
#define reasAFUNoMemory               __AFU_FactReasCode(0x0002)
#define strAFUWorkSpaceAllocateFailed  "No workspace manager"
#define strAFUNoMemory                 "No Memory"

#define reasAFUMissingArgument         __AFU_FactReasCode(0x0003)


#define tranevtAFU_WorkSpaceFree       __AFU_FactTranEvt  (0x0004)
#define extranevtAFU_WorkSpaceFree     __AFU_FactExTranEvt(0x0004)

#define tranevtAFU_ProcessMessage      __AFU_FactTranEvt  (0x0005)
#define extranevtAFU_ProcessMessage    __AFU_FactExTranEvt(0x0005)

/// @}

/******************************************************************************
* Define ManagementAFU Specific macros
******************************************************************************/
#define __AFU_MAFUTranEvt(Num)            __AAL_TranEvt  (AAL_sysManagementAFU, Num)
#define __AFU_MAFUExTranEvt(Num)          __AAL_ExTranEvt(AAL_sysManagementAFU, Num)
#define __AFU_MAFUErrNum(Num)             __AAL_ErrNum   (AAL_sysManagementAFU, Num)
#define __AFU_MAFUReasCode(Num)           __AAL_ReasCode (AAL_sysManagementAFU, Num)
#define __AAL_MAFUNid(Num)                __AAL_Nid      (AAL_sysManagementAFU, Num)

#define tranevtMAFUConfig                 __AFU_MAFUTranEvt  (0x0000)
#define extranevtMAFUConfig               __AFU_MAFUExTranEvt(0x0000)

#define tranevtMAFUQueryDevice            __AFU_MAFUTranEvt  (0x0001)
#define extranevtMAFUQueryDevice          __AFU_MAFUExTranEvt(0x0001)

//#define errAFUWorkSpace                __AFU_MAFUErrNum(0x0001)
//#define reasAFUNoWorkSpaceManager      __AFU_MAFUReasCode(0x0001)
//#define reasAFUNoMemory               __AFU_MAFUReasCode(0x0002)
//#define strAFUWorkSpaceAllocateFailed  "No workspace manager"
//#define strAFUNoMemory                 "No Memory"

// NamedValueSet Keys for ConfigureAFU
#define nidMAFUConfigKeyAFU_ID            __AAL_MAFUNid(0x0000)   // AFU_ID to activate
#define nidMAFUConfigKeyAFU_CH            __AAL_MAFUNid(0x0001)   // AFU Channel to activate
#define nidMAFUConfigKeyAFU_Bin           __AAL_MAFUNid(0x0002)   // string/string array: filename
#define nidMAFUConfigKeyAFU_BinLocation   __AAL_MAFUNid(0x0003)   // string/string array: cache, path, any thing appropriate, URL
#define nidMAFUConfigKeyAFU_BinTarget     __AAL_MAFUNid(0x0004)   // string/string array: Identifier of target for binary
#define nidMAFUConfigKeyParms             __AAL_MAFUNid(0x0005)   // NVS: containing vendor-specific parameters to be used during configuration

#define nidMAFUQueryIntKeyArray           __AAL_MAFUNid(0x0040)   // IntArray: Keys to query, and Keys returned in exception
#define nidMAFUQueryStringKeyArray        __AAL_MAFUNid(0x0041)   // StringArray: Keys to query, and Keys returned in exception


/*=============================================================================
===============================================================================
========================= Debug macro if defined ==============================
===============================================================================
=============================================================================*/

#if defined(DBG_AAL_ID_MACRO)
#undef DBG_AAL_ID_MACRO    // Only do this once even if included multiple times

#define DBG_AAL_MAKE_NAME(token) {#token, token},

struct __AAL_dbg_ID_MACRO_struct {
   char           *name;
   AAL::btID       Id;
};

static void __AAL_dbg_ID_MACRO(void) {
   __AAL_dbg_ID_MACRO_struct  rg[] =
      {
         {"Global Error Codes ----------------", 0},
         DBG_AAL_MAKE_NAME(errBadParameter)
         DBG_AAL_MAKE_NAME(errCauseUnknown)
         DBG_AAL_MAKE_NAME(errCreationFailure)
         DBG_AAL_MAKE_NAME(errEDSFailed)
         DBG_AAL_MAKE_NAME(errInternal)
         DBG_AAL_MAKE_NAME(errInvokeMethodFailure)
         DBG_AAL_MAKE_NAME(errMethodNotImplemented)
         DBG_AAL_MAKE_NAME(errNoFactory)
         DBG_AAL_MAKE_NAME(errSendMessageFailure) {"Global Reason Codes ---------------", 0
         },
         DBG_AAL_MAKE_NAME(reasCauseUnknown)
         DBG_AAL_MAKE_NAME(reasFeatuerNotSupported)
         DBG_AAL_MAKE_NAME(reasInvalidParameter)
         DBG_AAL_MAKE_NAME(reasMissingParameter)
         DBG_AAL_MAKE_NAME(reasNoFactoryForClass)
         DBG_AAL_MAKE_NAME(reasParameterNameInvalid)
         DBG_AAL_MAKE_NAME(reasParameterTypeInvalid)
         DBG_AAL_MAKE_NAME(reasParameterUnknown)
         DBG_AAL_MAKE_NAME(reasParameterValueInvalid)
         DBG_AAL_MAKE_NAME(reasResourcesNotAvailable)
         DBG_AAL_MAKE_NAME(reasUnknown)
         DBG_AAL_MAKE_NAME(reasUnknownMessage)
         DBG_AAL_MAKE_NAME(reasUnsupportedMethod) {"IIDs ------------------------------", 0
         },
         DBG_AAL_MAKE_NAME(iidBase)
         DBG_AAL_MAKE_NAME(iidServiceContainer)
         DBG_AAL_MAKE_NAME(iidRegistrar)
         DBG_AAL_MAKE_NAME(iidFactory)
         DBG_AAL_MAKE_NAME(iidAIA)
         DBG_AAL_MAKE_NAME(iidAFU)
         DBG_AAL_MAKE_NAME(iidAIA_Factory)
         DBG_AAL_MAKE_NAME(iidAIA_WorkSpace)
         DBG_AAL_MAKE_NAME(iidAFU_Factory)
         DBG_AAL_MAKE_NAME(iidAFU_WorkSpace) {"SystemInit Defines ----------------", 0
         },
         DBG_AAL_MAKE_NAME(tranevtSystemInit)
         DBG_AAL_MAKE_NAME(extranevtSystemInit)
         DBG_AAL_MAKE_NAME(tranevtSystemStop) {"Factory Defines -------------------", 0
         },
         DBG_AAL_MAKE_NAME(tranevtFactoryCreate)
         DBG_AAL_MAKE_NAME(extranevtFactoryCreate)
         DBG_AAL_MAKE_NAME(reasParameterNameInvalidToFactory)
         DBG_AAL_MAKE_NAME(reasMissingParameterToFactory) {"Other Defines ---------------------", 0
         },
         DBG_AAL_MAKE_NAME(tranevtRegistrarDestroy)
         DBG_AAL_MAKE_NAME(extranevtRegistrarDestroy)
         DBG_AAL_MAKE_NAME(tranevtRegistrarRegister)
         DBG_AAL_MAKE_NAME(extranevtRegistrarRegister)
         DBG_AAL_MAKE_NAME(tranevtRegistrarFind)
         DBG_AAL_MAKE_NAME(extranevtRegistrarFind)
         DBG_AAL_MAKE_NAME(tranevtRegistrarGet)
         DBG_AAL_MAKE_NAME(extranevtRegistrarGet)
         DBG_AAL_MAKE_NAME(tranevtRegistrarPut)
         DBG_AAL_MAKE_NAME(extranevtRegistrarPut)
         DBG_AAL_MAKE_NAME(tranevtRegistrarPutRecord)
         DBG_AAL_MAKE_NAME(extranevtRegistrarPutRecord)
         DBG_AAL_MAKE_NAME(tranevtAIA_FactoryCreate)
         DBG_AAL_MAKE_NAME(extranevtAIA_FactoryCreate)
         DBG_AAL_MAKE_NAME(tranevtAIA_FactoryDestroy)
         DBG_AAL_MAKE_NAME(extranevtAIA_FactoryDestroy)
         //DBG_AAL_MAKE_NAME(tranevtAIA_WorkSpaceAllocate)
         //DBG_AAL_MAKE_NAME(extranevtAIA_WorkSpaceAllocate)
         //DBG_AAL_MAKE_NAME(tranevtAIA_WorkSpaceFree)
         //DBG_AAL_MAKE_NAME(extranevtAIA_WorkSpaceFree)
         DBG_AAL_MAKE_NAME(tranevtAFU_FactoryCreate)
         DBG_AAL_MAKE_NAME(extranevtAFU_FactoryCreate)
         DBG_AAL_MAKE_NAME(tranevtAFU_FactoryDestroy)
         DBG_AAL_MAKE_NAME(extranevtAFU_FactoryDestroy)
         DBG_AAL_MAKE_NAME(tranevtAFU_WorkSpaceAllocate)
         DBG_AAL_MAKE_NAME(extranevtAFU_WorkSpaceAllocate)
         DBG_AAL_MAKE_NAME(tranevtAFU_WorkSpaceFree)
         DBG_AAL_MAKE_NAME(extranevtAFU_WorkSpaceFree)
         DBG_AAL_MAKE_NAME(tranevtAFU_ProcessMessage)
         DBG_AAL_MAKE_NAME(extranevtAFU_ProcessMessage) {
            0, 0
         }
      };

   printf_s ("sizeof(unsigned long long) is %d\n", sizeof(unsigned long long));
   printf_s ("sizeof(AAL::btID) is %d\n", sizeof(AAL::btID));

   printf_s ("AAL_VendorBitsMask %#010X\n", AAL_VendorBitsMask);
   printf_s ("AAL_SubSystemBitsMask 0x%08X\n", AAL_SubSystemBitsMask);
   printf_s ("AAL_TypeBitsMask 0x%08X\n", AAL_TypeBitsMask);
   printf_s ("AAL_ItemBitsMask 0x%08X\n", AAL_ItemBitsMask);

   printf_s ("Id(1,2,3,4) = %#018I64X\n", AAL_ID(1,2,1,4,5));

   for (unsigned i = 0; rg[i].name; ++i)
      printf_s ("%6d\t%35s\t%#018I64X\n", i, rg[i].name, rg[i].Id);

   //AAL::btID temp = extranevtAFU_WorkSpaceAllocate;
   //if (AAL_IS_EXCEPTION(temp))
   if (AAL_IS_EXCEPTION(extranevtAFU_WorkSpaceAllocate))
      printf_s ("extranevtAFU_WorkSpaceAllocate IS an exception\n");
   else
      printf_s ("extranevtAFU_WorkSpaceAllocate IS NOT an exception\n");
}
#endif // defined(DBG_AAL_ID_MACRO)


#endif // __AALSDK_AALIDDEFS_H__

