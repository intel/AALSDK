// Copyright (c) 2008-2015, Intel Corporation
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
/// @file KernelStructs.h
/// @brief Utilities used when working with the Resource Manager
/// @ingroup ResMgr
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHOR: Henry Mitchel, Intel Corporation.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 11/09/2008     HM       Initial version started
/// 12/14/2008     HM       Added initial support for ui_ioctl,
///                            Found namespacing issues
/// 01/01/2009     HM       Resolved most namespacing issues by making all of them
///                            global in scope, only qualifying the objects being
///                            streamed.
///                         Still some questions as to why some do not resolve
///                            even in global scope, and actually need a restricted
///                            scope, e.g. for TTASK_MODE.
/// 01/04/2009     HM       Updated Copyright
/// 03/08/2009     HM       Removed DestroyRMIoctlReq() to CAASResourceManager.h
/// 03/20/2009     HM       Added more kernel structures
/// 12/09/2009     JG       Pulled out AFU PIP commands aalui_afucmd_e
///                            and moved it to fappip.h and defined them as FAP
///                            pip specific.@endverbatim
//****************************************************************************
#ifndef __AALSDK_KERNEL_KERNELSTRUCTS_H__
#define __AALSDK_KERNEL_KERNELSTRUCTS_H__
#include <aalsdk/AALLoggerExtern.h>       // LOG_VERBOSE
#include <aalsdk/ResMgr.h>                // Definitions for user mode RM users
                                          // Types, Events, aalrm, aalrm_server, aalrm_client
                                          // Interestingly, for just operator<<, not needed
#include <aalsdk/kernel/aalui.h>          // user mode UI users, as well
//#include <aalsdk/kernel/fappip.h>         // FAP PIP interface
#include <aalsdk/kernel/aalmafu.h>        // MAFU interface
#include <aalsdk/kernel/AALWorkspace.h>   // TTASK_MODE, TDESC_TYPE


BEGIN_NAMESPACE(AAL)

typedef enum
{
   eLogLevel_Emergency =      LOG_EMERG,     // system is unusable
   eLogLevel_Alert =          LOG_ALERT,     // action must be taken immediately
   eLogLevel_Critical =       LOG_CRIT,      // critical conditions
   eLogLevel_Error =          LOG_ERR,       // error conditions
   eLogLevel_Warning =        LOG_WARNING,   // warning conditions
   eLogLevel_Notice =         LOG_NOTICE,    // normal but significant condition
   eLogLevel_Informational =  LOG_INFO,      // informational
   eLogLevel_Debug =          LOG_DEBUG,     // debug-level messages
   eLogLevel_Verbose =        LOG_VERBOSE    // NOT a LINUX level. AAL for really verbose output
} LogLevel_t;

   /*
    * Apparently, this (definition of operator << in both global and AAL:: namespaces) is needed
    * because this operator is called from within
    * another AAL::operator<<, and it does not resolve unless there is an
    * AAL::operator<<(TTASK_MODE). I am not sure if this is correct, but empirically
    * it appears to be occurring. If this function is removed, TTASK_MODE printed from
    * within 'std::ostream& AAL::operator << (std::ostream& s, const WorkSpaceMapper::WkSp& wksp)'
    * will not resolve to the generic one, but instead will print the enum as
    * if it were an integer. Specifically, that reference will NOT find the version
    * that is in global scope, above.
    */

//--------------------------   A A L T Y P E S   ------------------------

AASLIB_API std::ostream & operator << (std::ostream & , const btVirtAddr & );

//-----------------------------  L I N U X   ----------------------------

AASLIB_API std::ostream & operator << (std::ostream & , const LogLevel_t & );

//-------------------------------   R M   -------------------------------

AASLIB_API std::ostream & operator << (std::ostream & , const stTransactionID_t        & );
AASLIB_API std::ostream & operator << (std::ostream & , const aal_bus_types_e          & );
AASLIB_API std::ostream & operator << (std::ostream & , const aal_device_type_e        & );
AASLIB_API std::ostream & operator << (std::ostream & , const aal_device_addr          & );
AASLIB_API std::ostream & operator << (std::ostream & , const aal_device_id            & );
AASLIB_API std::ostream & operator << (std::ostream & , const device_attrib            & );
AASLIB_API std::ostream & operator << (std::ostream & , const krms_cfgUpDate_e         & );
AASLIB_API std::ostream & operator << (std::ostream & , const aalrms_configUpDateEvent & );
AASLIB_API std::ostream & operator << (std::ostream & , const rms_msgIDs_e             & );
AASLIB_API std::ostream & operator << (std::ostream & , const rms_result_e             & );
AASLIB_API std::ostream & operator << (std::ostream & , const aalrms_requestdevice     & );
AASLIB_API std::ostream & operator << (std::ostream & , const aalrm_ioctlreq           & );


//-------------------------------   FAP   --------------------------------
//AASLIB_API std::ostream & operator << (std::ostream & , const fappip_afuCmdID_e  & );

//-------------------------------   U I   --------------------------------

AASLIB_API std::ostream & operator << (std::ostream & , const uid_msgIDs_e       & );
AASLIB_API std::ostream & operator << (std::ostream & , const uid_errnum_e       & );
AASLIB_API std::ostream & operator << (std::ostream & , const uid_mgtAfuCmdID_e  & );
AASLIB_API std::ostream & operator << (std::ostream & , const uid_afurespID_e    & );
AASLIB_API std::ostream & operator << (std::ostream & , const uid_wseventID_e    & );

//-------------------------------   W S  --------------------------------
AASLIB_API std::ostream & operator << (std::ostream & , const TTASK_MODE     & );
AASLIB_API std::ostream & operator << (std::ostream & , const TDESC_TYPE     & );
AASLIB_API std::ostream & operator << (std::ostream & , const TDESC_POSITION & );

END_NAMESPACE(AAL)


AASLIB_API std::ostream & operator << (std::ostream & , const AAL::btVirtAddr & );

AASLIB_API std::ostream & operator << (std::ostream & , const AAL::LogLevel_t & );

AASLIB_API std::ostream & operator << (std::ostream & , const AAL::stTransactionID_t        & );
AASLIB_API std::ostream & operator << (std::ostream & , const AAL::aal_bus_types_e          & );
AASLIB_API std::ostream & operator << (std::ostream & , const AAL::aal_device_type_e        & );
AASLIB_API std::ostream & operator << (std::ostream & , const AAL::aal_device_addr          & );
AASLIB_API std::ostream & operator << (std::ostream & , const AAL::aal_device_id            & );
AASLIB_API std::ostream & operator << (std::ostream & , const AAL::device_attrib            & );
AASLIB_API std::ostream & operator << (std::ostream & , const AAL::krms_cfgUpDate_e         & );
AASLIB_API std::ostream & operator << (std::ostream & , const AAL::aalrms_configUpDateEvent & );
AASLIB_API std::ostream & operator << (std::ostream & , const AAL::rms_msgIDs_e             & );
AASLIB_API std::ostream & operator << (std::ostream & , const AAL::rms_result_e             & );
AASLIB_API std::ostream & operator << (std::ostream & , const AAL::aalrms_requestdevice     & );
AASLIB_API std::ostream & operator << (std::ostream & , const AAL::aalrm_ioctlreq           & );

//AASLIB_API std::ostream & operator << (std::ostream & , const AAL::fappip_afuCmdID_e  & );

AASLIB_API std::ostream & operator << (std::ostream & , const AAL::uid_msgIDs_e       & );
AASLIB_API std::ostream & operator << (std::ostream & , const AAL::uid_errnum_e       & );
AASLIB_API std::ostream & operator << (std::ostream & , const AAL::uid_mgtAfuCmdID_e  & );
AASLIB_API std::ostream & operator << (std::ostream & , const AAL::uid_afurespID_e    & );
AASLIB_API std::ostream & operator << (std::ostream & , const AAL::uid_wseventID_e    & );

AASLIB_API std::ostream & operator << (std::ostream & , const AAL::TTASK_MODE     & );
AASLIB_API std::ostream & operator << (std::ostream & , const AAL::TDESC_TYPE     & );
AASLIB_API std::ostream & operator << (std::ostream & , const AAL::TDESC_POSITION & );

#endif // __AALSDK_KERNEL_KERNELSTRUCTS_H__


