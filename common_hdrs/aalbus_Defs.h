//******************************************************************************
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2014-2015, Intel Corporation.
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
//  Copyright(c) 2014-2015, Intel Corporation.
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
//        FILE: aalbusDefs.h
//     CREATED: Feb. 27, 2015
//      AUTHOR: Joseph Grecco, Intel Corporation.
//
// PURPOSE: Public Definitions for the AAL Bus subsystem
// HISTORY:
// WHEN:          WHO:     WHAT:
//****************************************************************************
#ifndef __AALSDK_KERNEL_BUS_DEFS_H__
#define __AALSDK_KERNEL_BUS_DEFS_H__


//-----------------------------------------------------------------------------
// Request message IDs
//-----------------------------------------------------------------------------
typedef enum
{
   // User Resource Manager requests
   reqid_bus_RequestDevice = 1,     // Device Allocation Request
   reqid_bus_ReleaseDevice,        // Device Release Request

   // Device configuration requests
   reqid_bus_DeviceRequest,          // Send a device request

   // Administration
   reqid_bus_Shutdown,                  // Request that the Service session shutdown
   reqid_bus_Restart,                   // Request to restart the RMCS without close/open

   // Kernel Resource Manager requests
   reqid_bus_SetConfigUpdates,     // Used to set configuration updates

   // Response and Event IDs
   rspid_bus_RequestDevice = 0xF000, // Device Allocation Response

   // Device configuration requests
   rspid_bus_DeviceRequest,          // Device Request response

   // Kernel Resource Manager Responses and events
   evtid_bus_ConfigUpdate,         // Configuration Update

   // Administration
   rspid_bus_Shutdown,                  // Service is shutdown
   rspid_bus_Started

} aalbus_msgIDs_e;


//-----------------------------------------------------------------------------
// Shutdown reason codes
//-----------------------------------------------------------------------------
typedef enum
{
   aalbus_shutdownReasonNormal = 0,
   aalbus_shutdownReasonMaint,
   aalbus_aalbus_shutdownFailure,
   aalbus_aalbus_shutdownReasonRestart
} aalbus_shutdownreason_e;

//-----------------------------------------------------------------------------
// Result message IDs - TODO COMBINE THESE WITH A GLOBAL SET
//-----------------------------------------------------------------------------
typedef enum
{
   aalbus_resultOK = 0,                                        // No error
   aalbus_resultMaxOwnersErr,                                  // Max device owners
   aalbus_resultDuplicateOwnerErr,                             // PID already owner
   aalbus_resultNotOwnerErr,                                   // PID not owner
   aalbus_resultInvalidDevice,                                 // Invalid device handle
   aalbus_resultErrno,                                         // Errno has only info
   aalbus_resultBadParm,                                       // Invalid parameter
   aalbus_resultCancelled,                                     // Transaction cancelled
   aalbus_resultDeviceHasNoPIPAssigned,                        // No PIP assigned
   aalbus_resultNoAppropriateInterface,                        // No interface
   aalbus_resultSystemErr                                      // System Error
} aalbus_result_e;



#endif //__AALSDK_KERNEL_BUS_DEFS_H__
