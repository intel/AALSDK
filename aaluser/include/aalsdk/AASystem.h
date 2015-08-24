// Copyright (c) 2007-2015, Intel Corporation
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
/// @file AASystem.h
/// @brief Public defines for the Intel(R) QuickAssist Technology System Services
/// @ingroup AALRUNTIME
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Joseph Grecco, Intel Corporation
///          Henry Mitchel, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 04/23/2007     JG       Initial version started
/// 05/08/2008     HM       Comments & License
/// 01/04/2009     HM       Updated Copyright@endverbatim
//****************************************************************************
#ifndef __AALSDK_AASYSTEM_H__
#define __AALSDK_AASYSTEM_H__
#include <aalsdk/AALNamedValueSet.h>
#include <aalsdk/AALTransactionID.h>


//=============================================================================
// Global functions and definitions for the Intel(R) QuickAssist Technology AAS
//=============================================================================
BEGIN_NAMESPACE(AAL)


//=============================================================================
// Typedefs and constants
//=============================================================================

#if DEPRECATED
// Defines the callback model of the running instance of the AAS.
typedef enum EnumCallbackModel
{
   CBM_Preemptive = 1, //< @todo describe me
   CBM_Serialized,     //< @todo describe me
   CBM_NonPreemptive,  //< @todo describe me
   CBM_Unknown = 0     //< Callback model is undefined.
} EnumCallbackModel;
#endif // DEPRECATED


//--------------------------
// System configuration keys
//--------------------------

/// @addtogroup AALRUNTIME
/// @{

/// Key for System shutdown time.
#define SYSINIT_KEY_SYSTEM_SHUTDOWN_TIME "sysinit_system_shutdown_time"
/// Maximum time hint for system shutdown. This time is not a hard max and may be exceeded.
#define DEF_SYSINIT_SYSTEM_SHUTDOWN_TIME AAL_INFINITE_WAIT
/// Key for kernel-less mode.
#define SYSINIT_KEY_SYSTEM_NOKERNEL      "sysinit_system_no_kernel"

/// @}

//=============================================================================
// Global system functions
//=============================================================================

#if DEPRECATED
// @brief Initialize the active elements of the AAS subsystem.
// @ingroup Init
//
// Initialization status is returned by callback to the provided EventHandler.
// - An event (interface id tranevtSystemInit) is returned on successful initialization. The
//   event contains an IAASServiceContainer (interface id iidServiceContainer) accessible with
//   subclass_ref<> or subclass_ptr<>.
// - An exception event (interface id extranevtSystemInit) is returned to the callback on failure.
//
// Ex.)
// @code
// if ( iidServiceContainer == theEvent.Object().SubClassID() ) {
//
//    IAASServiceContainer *pSC = subclass_ptr<IAASServiceContainer>(theEvent.Object());
//
//    if ( NULL != pSC ) {
//       // Access the Service Container interface via pSC.
//    }
//
// }
// @endcode
AASLIB_API
btInt
SystemInit(EnumCallbackModel    CallBackModel,
           btEventHandler       theEventHandler,
           btApplicationContext Context,
           TransactionID const &tranID  = TransactionID(),
           NamedValueSet const &optArgs = NamedValueSet(),
           btBool               Block   = false);
#endif // DEPRECATED

#if DEPRECATED
// @brief Shut down the active elements of the AAS subsystem.
// @ingroup Shutdown
//
// SystemStop causes a clean shutdown of the system. Once all elements of the system are shut down, an
// event of subclass tranevtSystemStop is be generated to the event handler registered with SystemInit.
// SystemInit will unblock if it was invoked with the Block set to true.  An exception event of subclass
// extranevtSystemStop is generated in error.
AASLIB_API
void
SystemStop(TransactionID const &tranID);
#endif // DEPRECATED


END_NAMESPACE(AAL)

#endif // __AALSDK_AASYSTEM_H__

