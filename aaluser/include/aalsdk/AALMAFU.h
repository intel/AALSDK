// Copyright(c) 2005-2016, Intel Corporation
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
/// file AALMAFU.h
/// brief IMAFU and related events.
/// ingroup MAFU
/// verbatim
/// Accelerator Abstraction Layer
///
/// AUTHOR: Joseph Grecco, Intel Corporation.
///         Henry Mitchel, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 09/22/2008     JG/HM    Initial version started
/// 01/04/2009     HM       Updated Copyright
/// 02/14/2012     JG       Added AFUIDH and L to deviceid in CreateAFU Request endverbatim
//****************************************************************************
#ifndef __AALSDK_AALMAFU_H__
#define __AALSDK_AALMAFU_H__
#include <aalsdk/AALTransactionID.h>
#include <aalsdk/AALNamedValueSet.h>
#include <aalsdk/kernel/aalmafu.h>

//  The IMAFU interface

BEGIN_NAMESPACE(AAL)

/// @addtogroup SysEvents
/// @{

//=============================================================================
// Typedefs and Constants
//=============================================================================

//--------------------------------------------------
// Standard Keys used for the Configure manifest NVS
//--------------------------------------------------
#define MAFU_WORK_ORDER                "MAFU_WORK_ORDER"         // DataTye NVS[]
#define MAFU_CONFIGURE_CREATE          "MAFU_CONFIGURE_CREATE"   // Datatype NVS
#define MAFU_CONFIGURE_DESTROY         "MAFU_CONFIGURE_DESTROY"  // Datatype NVS

// The following keys are used when creating an AFU
#define MAFU_CONFIGURE_SHAREMAX         keyRegAFU_ShareMax

#define MAFU_CONFIGURE_MANIFEST        "keyRegManifest"     // DataType btByteArray,
//   #define MAFU_CONFIGURE_AFUID           "keyRegAFUID"        // DataType btByteArray,
#define MAFU_CONFIGURE_AFUIDL          "keyRegAFUIDL"       // DataType btByteArray,
#define MAFU_CONFIGURE_AFUIDH          "keyRegAFUIDH"       // DataType btByteArray,
#define MAFU_CONFIGURE_UEVENT          "keyRegUevent"       // btStringArray
#define MAFU_CONFIGURE_AFUNAME          keyRegAFU_Name       // AFU name

#define MAFU_CONFIGURE_HANDLE           keyRegAFU_Handle     // DataType HAndle_t

//=============================================================================
// Name: IMAFUConfigureTransactionEvent
// Description: Event sent as a result of the Configure Device Transaction Event
//=============================================================================
class IMAFUConfigureTransactionEvent
{
public:
   //CHANGE TO take an index
   virtual const afu_descriptor * afu_descriptors(btUnsigned32bitInt index) const = 0;
   virtual const btUnsigned32bitInt num_descriptors() const = 0;
   virtual ~IMAFUConfigureTransactionEvent() {}
};

//=============================================================================
// Name: IMAFUQueryDeviceTransactionEvent
// Description: Event sent as a result of the Query Device Transaction Event
//=============================================================================
class IMAFUQueryDeviceTransactionEvent
{
public:
   virtual const NamedValueSet & Results()    const = 0; // Keys from QueryDevice with values
   virtual const NamedValueSet & Exceptions() const = 0; // Array of Keys from QueryDevice that failed
   virtual ~IMAFUQueryDeviceTransactionEvent() {}
};

/// @}

//=============================================================================
// Name: IMAFU
// Description: Public Interface base class for the management AFU
//=============================================================================
class IMAFU
{
public:
   // Method used to configure a device
   //   Generates a tranevtMAFUConfig or exttranevtMAFUConfig event
   virtual void ConfigureDevice(const NamedValueSet &nvs,
                                const TransactionID &TranID) = 0;

   // Returns queried for attributes in tranevtMAFUQueryDevice
   //   Generates a tranevtMAFUQueryDevice or exttranevtMAFUQueryDevice event
   virtual void QueryDevice    (const NamedValueSet &nvs,     // Array of keys of requested attributes
                                const TransactionID &TranID) = 0;
   virtual ~IMAFU() {}
};

END_NAMESPACE(AAL)

#endif // __AALSDK_AALMAFU_H__

