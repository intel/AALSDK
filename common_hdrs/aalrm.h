//******************************************************************************
// Part of the Intel(R) QuickAssist Technology Accelerator Abstraction Layer
//
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2008-2015, Intel Corporation.
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
//  Copyright(c) 2008-2015, Intel Corporation.
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
//        FILE: aalrm.h
//     CREATED: 02/25/2008
//      AUTHOR: Joseph Grecco - Intel
//
// PURPOSE: This file contains public definitions common to the
//          AAL Resource Manager Server and Client Kernel Module Interface
// HISTORY:
// COMMENTS:
// WHEN:          WHO:     WHAT:
// 02/25/08       JG       Initial version created
// 11/10/08       JG       Separated common definitions
// 11/11/2008     JG       Added legal header
// 01/04/2009     HM       Updated Copyright
// 01/08/2009     JG       Cleanup and refactoring
//****************************************************************************
#ifndef __AALSDK_KERNEL_AALRM_H__
#define __AALSDK_KERNEL_AALRM_H__
#include <aalsdk/kernel/AALTransactionID_s.h>
#include <aalsdk/kernel/aaldevice.h>

BEGIN_NAMESPACE(AAL)

BEGIN_C_DECLS


//-----------------------------------------------------------------------------
// Request message IDs
//-----------------------------------------------------------------------------
typedef enum
{
   // User Resource Manager requests
   reqid_URMS_RequestDevice=1,      // Device Allocation Request
   reqid_URMS_ReleaseDevice,        // Device Release Request

   // Registrar requests
   reqid_RS_Registrar,              // Registrar Request

   // Device configuration requests
   reqid_RM_DeviceRequest,          // Send a device request

   // Administration
   reqid_Shutdown,                  // Request that the Service session shutdown
   reqid_Restart,                   // Request to restart the RMCS without close/open

   // Kernel Resource Manager requests
   reqid_KRMS_SetConfigUpdates,     // Used to set configuration updates

   // Response and Event IDs
   rspid_URMS_RequestDevice=0xF000, // Device Allocation Response

   // Registrar responses and events
   rspid_RS_Registrar,              // Registrar Response

   // Device configuration requests
   rspid_RM_DeviceRequest,          // Device Request response

   // Kernel Resource Manager Responses and events
   evtid_KRMS_ConfigUpdate,         // Configuration Update

   // Administration
   rspid_Shutdown,                  // Service is shutdown
   rspid_Started

} rms_msgIDs_e;

//-----------------------------------------------------------------------------
// Request message IDs
//-----------------------------------------------------------------------------
typedef enum
{
   rms_resultOK = aaldev_addowner_OK,                      // No error
   rms_resultMaxOwnersErr = aaldev_addowner_MaxOwners,     // Max device owners
   rms_resultDuplicateOwnerErr = aaldev_addowner_DupOwner, // PID already owner
   rms_resultNotOwnerErr = aaldev_addowner_NotOwner,       // PID not owner
   rms_resultInvalidDevice = aaldev_addowner_InvalidDevice,// Invalid device handle
   rms_resultErrno,                    // Errno has only info
   rms_resultBadParm,                  // Invalid parameter
   rms_resultCancelled,                // Transaction cancelled
   rms_resultDeviceHasNoPIPAssigned,   // No PIP assigned
   rms_resultNoAppropriateInterface,   // No interface
   rms_resultSystemErr = aaldev_addowner_SysErr             // System Error
} rms_result_e;


//-----------------------------------------------------------------------------
// Shutdown reason codes
//-----------------------------------------------------------------------------
typedef enum
{
   rms_shutdownReasonNormal=0,
   rms_shutdownReasonMaint,
   rms_rms_shutdownFailure,
   rms_rms_shutdownReasonRestart
} rms_shutdownreason_e;


//=============================================================================
// Name: aalrms_DeviceRequest
// Type[Dir]: Request [IN]
// Object: Kernel Resource Manager Service
// Command ID: reqid_RM_DeviceRequest
// Input:
// Description: Request a device be configured
//=============================================================================
struct aalrms_DeviceRequest {
   btUnsigned64bitInt         subdeviceMask; // Channel [IN]
   enum aal_device_request_e  reqid;         // Request id
   btWSSize                   size;          // size of payload [IN]
   btByte                     payload;       // first char of variably length payload [IN]
};

// Size of structure without payload
#define DEVREQUEST_DEVHDRSZ  ( offsetof(struct aalrms_DeviceRequest, payload) )



//=============================================================================
// Name: aalrm_ioctlreq
// Description: Structure used for sending and receiving messages between the
//              Resource Manager user and the AAL kernel
//              services
// Comments: Fields labeled [IN] refer to parameters that are passed into the
//           kernel services from the user space app. OUT refers to parameters
//           that will be filled in by the kernel services.
//=============================================================================
struct aalrm_ioctlreq {
   // Message header
   rms_msgIDs_e          id;          // ID of request on Queue Head [IN/OUT]
   rms_result_e          result_code; // Result code [IN/OUT]
   btObjectType          req_handle;  // Request Handle used in response [IN/OUT]
   stTransactionID_t     tranID;      // TransactionID
   btObjectType          context;     // Context
   // Generic Payload
   union
   {
      btUnsigned64bitInt data;        // Payload data [IN/OUT]
      btObjectType       res_handle;  // Resource handle
   };
   btWSSize              size;        // Size of payload [IN/OUT]
   btVirtAddr            payload;     // Pointer to Payload buffer [IN/OUT]
};


#ifdef __AAL_LINUX__
# define AALRM_IOCTL_GETMSG_DESC  _IOR  ('x', 0x00, struct aalrm_ioctlreq)
# define AALRM_IOCTL_GETMSG       _IOWR ('x', 0x01, struct aalrm_ioctlreq)
# define AALRM_IOCTL_SENDMSG      _IOWR ('x', 0x02, struct aalrm_ioctlreq)
#endif // __AAL_LINUX__


END_C_DECLS

END_NAMESPACE(AAL)

#endif // __AALSDK_KERNEL_AALRM_H__

