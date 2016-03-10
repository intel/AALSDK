//******************************************************************************
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2014-2016, Intel Corporation.
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
//  Copyright(c) 2014-2016, Intel Corporation.
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
//        FILE: aalbus_iupdate_config.h
//     CREATED: May 13, 2014
//      AUTHOR: Joseph Grecco, Intel Corporation.
//
// PURPOSE: Definitions for the AAL IUpdateConfig AAL Bus interface
// HISTORY:
// WHEN:          WHO:     WHAT:
//
//****************************************************************************
#ifndef __AALSDK_KERNEL_BUS_IUPDATE_CONFIG_H__
#define __AALSDK_KERNEL_BUS_IUPDATE_CONFIG_H__
#include <aalsdk/kernel/kosal.h>

#include <aalsdk/kernel/aaltypes.h>
#include <aalsdk/kernel/aaldevice.h>
#include <aalsdk/kernel/aalui.h>

//
// Define an Interface Guid to access the proprietary toaster interface.
// This guid is used to identify a specific interface in IRP_MN_QUERY_INTERFACE
// handler.
//


BEGIN_C_DECLS

//==============
// IUpdateConfig
//==============
//
// Define Interface reference/dereference routines for
//  Interfaces exported by IRP_MN_QUERY_INTERFACE
//
typedef VOID(*PINTERFACE_REFERENCE)(PVOID Context);
typedef VOID(*PINTERFACE_DEREFERENCE)(PVOID Context);

// Main methods
typedef BOOLEAN(*PUPDATECONFIG_ADD_DEVICE)(IN WDFDEVICE, IN   PVOID Record);
typedef BOOLEAN(*PUPDATECONFIG_REMOVE_DEVICE)(IN WDFDEVICE, IN PVOID Record);
typedef BOOLEAN(*PUPDATECONFIG_DEVICE_STATE_EVT)(IN WDFDEVICE, IN PVOID Event);

//
// Interface for changing a resoure state
//
typedef struct _UPDATECONFIG_INTERFACE_STANDARD {
   INTERFACE                        InterfaceHeader;
   PUPDATECONFIG_ADD_DEVICE         AddDevice;
   PUPDATECONFIG_REMOVE_DEVICE      RemoveDevice;
   PUPDATECONFIG_DEVICE_STATE_EVT   EvtDeviceStateChange;
} UPDATECONFIG_INTERFACE_STANDARD, *PUPDATECONFIG_INTERFACE_STANDARD;


//--------------------------------
// Interface definitions commands
//--------------------------------
#if defined( __AAL_WINDOWS__ )

// Interface for kernel access
DEFINE_GUID(GUID_AALBUS_UPDATECONFIG_INTERFACE_STANDARD,
0x47205c5a, 0x958, 0x4cb0, 0x9e, 0xe, 0xcf, 0xf2, 0x84, 0x71, 0x88, 0x50);
// {47205C5A-0958-4CB0-9E0E-CFF284718850}


// Inteface for user mode access
DEFINE_GUID(GUID_DEVINTERFACE_AALBUS_UPDATECONFIG,
   0x868c257d, 0xc827, 0x496b, 0xae, 0x33, 0x13, 0x53, 0x80, 0x24, 0xe, 0x8c);
// {868C257D-C827-496B-AE33-135380240E8C}

// Device names for user mode access
#define NTDEVICE_UPDATECONFIG_NAME_STRING      L"\\Device\\aalbus\AAL_IUpdateConfig"
#define SYMBOLIC_UPDATECONFIG_NAME_STRING      L"\\DosDevices\\aalbus\AAL_IUpdateConfig"


#define FILE_DEVICE_AALBUS	FILE_DEVICE_BUS_EXTENDER

#define AALBUS_IUPDATE_IOCTL(_index_) \
    CTL_CODE (FILE_DEVICE_AALBUS, _index_, METHOD_BUFFERED, FILE_READ_DATA)

#define IOCTL_AALBUS_ADD_DEVICE					AALBUS_IUPDATE_IOCTL(0x0)
#define IOCTL_AALBUS_REMOVE_DEVICE           AALBUS_IUPDATE_IOCTL(0x1)
#define IOCTL_AALBUS_UPDATE_DEVICE_STATE     AALBUS_IUPDATE_IOCTL(0x2)
#elif defined(__AAL_LINUX__)


#endif

//=============================================================================
// Name:        IUpdateConfig_DeviceManifest
// Type[Dir]:   Request [IN]
// Command ID:  IOCTL_AALBUS_CREATE_DEVICE
// Description: Used when sending a Create Device request to the AAL Bus
// Comments:  
//=============================================================================
struct IUpdateConfig_DeviceManifest {
   btUIntPtr               size;                   // sizeof(struct IUpdateConfig_DeviceManifest)
                                                   //  + manifest_size
   struct aal_device_id    device_id;              // AAL Bus device Identifer

   btUnsignedInt           actionflags;            // state to leave device

   #define MAX_BASENAME_LEN   256                  // Maximum size of name base
   btByte                  basename[MAX_BASENAME_LEN + 1]; // Base name of device
   
   btUnsignedInt           maxshares;              // Maximum number of shares
#define MAFU_CONFIGURE_UNLIMTEDSHARES  (~0U)

   btUnsigned16bitInt      enableDirectAPI;        // Enable direct PIP access

   btWSSize                manifest_size;          // Length of optional parms
   btByte                  manifest;               // Optional additional parms
};



END_C_DECLS

#endif // __AALSDK_KERNEL_BUS_IUPDATE_CONFIG_H__

