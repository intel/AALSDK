//******************************************************************************
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2009-2016, Intel Corporation.
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
//  Copyright(c) 2009-2016, Intel Corporation.
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
//        FILE: aalmafu.h
//     CREATED: Oct 22, 2009
//      AUTHOR: Joseph Grecco, Intel Corporation.
//
// PURPOSE: Definitions for the AAL Management AFU low-level interface
// HISTORY:
// WHEN:          WHO:     WHAT:
// 05/06/2009     HM       Removed inappropriate hard path coding of 
//                            aas/kernel/include
//****************************************************************************
#ifndef __AALSDK_KERNEL_AALMAFU_H__
#define __AALSDK_KERNEL_AALMAFU_H__
#include <aalsdk/kernel/aaltypes.h>
#include <aalsdk/kernel/iaaldevice.h>


BEGIN_NAMESPACE(AAL)

BEGIN_C_DECLS

//----------------
// MAFU commands
//----------------
typedef enum {
   // Management AFU commands
   aalui_mafucmd,
   aalui_mafucmdActvateAFU,
   aalui_mafucmdDeactvateAFU,
   aalui_mafucmdInitializeAFU,
   aalui_mafucmdFreeAFU,
   aalui_mafucmdCreateAFU,
   aalui_mafucmdDestroyAFU,
} uid_mgtAfuCmdID_e;


//=============================================================================
// Name:        mafu_CreateAFU
// Type[Dir]:   Request [IN]
// Object:      PIP
// Command ID:  aalui_mafucmdCreateAFU
// Input:
// Description: Used when sending a Create AFU request to the MAFU
// Comments:    Pointed to by the payload of mafu_request.
//              NOTE: The size parameter in mafu_request must equal
//              sizeof(struct mafu_CreateAFU)+sizeof(uevent_args)
//              where sizeof(uevent_args) will be
//              sigma(strlen(uevent_args[0->(n-1)]) + n). In other words the
//              size of the entire contiguous block.
//=============================================================================
struct mafu_CreateAFU {
   struct aal_device_id    device_id;

   btUnsignedInt           actionflags;            // state to leave device

   // Name length must fit parameters of the Kobject device name
   #define MAFU_MAX_BASENAME_LEN   8               // Maximum size of name base
   btByte                  basename[MAFU_MAX_BASENAME_LEN+1]; // Base name of device
   btUnsignedInt           maxshares;              // Maximum number of shares
#define MAFU_CONFIGURE_UNLIMTEDSHARES  (~0U)

   btUnsigned16bitInt      enableDirectAPI;        // Enable direct PIP access

   btWSSize                manifest_size;          // Length of optional parms
   btByteArray             manifest;               // Optional additional parms
   btUnsignedInt           num_uevent_args;        // Number of uevent arguments
   btByte                  uevent_args[1];         // Block of ASCIIZ args
};

#define mafu_CreateAFU_STATIC_INITIALIZER(mca) \
{                                              \
   .actionflags = 0,                           \
   .basename = { 0, },                         \
   .maxshares = 0,                             \
   .enableDirectAPI = AAL_DEV_APIMAP_NONE,     \
   .manifest_size = 0,                         \
   .manifest = NULL,                           \
   .num_uevent_args = 0,                       \
}

//=============================================================================
// Name: afu_descriptor
// Type[Dir]: Request{IN] Event/Response [OUT]
// Object: MAFU engine
// Description: Result from an AFU Create or enum. Contains basic AFU
//              attributes.
// Comments:
//=============================================================================
typedef struct afu_descriptor {
   krms_cfgUpDate_e     cfgStatus;
   struct aal_device_id devid;   // Device ID
   btByte               basename[MAFU_MAX_BASENAME_LEN+1];
} afu_descriptor;


//=============================================================================
// Name:        mafu_DestroyAFU
// Type[Dir]:   Request [IN]
// Object:      PIP
// Command ID:  aalui_mafucmdDestroyAFU
// Input:       dev_desc - afu_descriptor for device to destroy.
// Description: Used when sending a Destroy AFU request to the MAFU
// Comments:    Pointed to by the payload of mafu_request.
//              The descriptor must be unambiguous and describe a device owned
//              by this MAFU.
//=============================================================================
struct mafu_DestroyAFU {
   afu_descriptor dev_desc;
};


//=============================================================================
// Name: mafu_request
// Type[Dir]: Request [IN]
// Object: Management AFU
// Command ID: reqid_UID_SendAFU
// Input: subdeviceMask - device mask indicates devices to operate on
//        size - size of optional payload
//        payload - optional payload
// Description: Request a device be configured
//=============================================================================
struct mafu_request {
   uid_mgtAfuCmdID_e     cmd;              // Management AFU command

   union {
      btUnsigned64bitInt subdeviceMask;    // Channel
      btObjectType       devhandle;        //device handle
   };

   btWSSize              size;             // size of payload
   btVirtAddr            payload;          // optional payload, of length size
};


END_C_DECLS

END_NAMESPACE(AAL)

#endif // __AALSDK_KERNEL_AALMAFU_H__

