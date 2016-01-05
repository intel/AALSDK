//******************************************************************************
// Part of the Intel(R) QuickAssist Technology Accelerator Abstraction Layer
//
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
//        FILE: aalbus_imonitorconfig.h
//     CREATED: May 15, 2014
//      AUTHOR: Joseph Grecco, Intel Corporation.
//
// PURPOSE: Definitions for the AAL IMonitorConfig AAL Bus interface
// HISTORY:
// WHEN:          WHO:     WHAT:
//****************************************************************************
#ifndef __AALSDK_KERNEL_BUS_IMONITOR_CONFIG_H__
#define __AALSDK_KERNEL_BUS_IMONITOR_CONFIG_H__

//
// Define an Interface Guid for toaster device class.
// This GUID is used to register (IoRegisterDeviceInterface)
// an instance of an interface so that user application
// can control the toaster device.
//
// {701D4F32-26CB-4A87-BBC4-D1EB0A3B51D2}
DEFINE_GUID(GUID_DEVINTERFACE_AALBUS_CONFIG_STATE_MONITOR,
   0x701d4f32, 0x26cb, 0x4a87, 0xbb, 0xc4, 0xd1, 0xeb, 0xa, 0x3b, 0x51, 0xd2);

BEGIN_NAMESPACE(AAL)

#define NTDEVICE_MONITORCONFIG_NAME_STRING      L"\\Device\\AAL_IMonitorConfig"
#define SYMBOLIC_MONITORCONFIG_NAME_STRING      L"\\DosDevices\\AAL_IMonitorConfig"

#define FILE_DEVICE_BUSENUM         FILE_DEVICE_BUS_EXTENDER

#define AALBUS_MONITOR_CONFIG_IOCTL(_index_) \
    CTL_CODE (FILE_DEVICE_BUSENUM, (_index_ + 0xF), METHOD_BUFFERED, FILE_READ_DATA)

#define IOCTL_BUSENUM_PLUGIN_HARDWARE               AALBUS_MONITOR_CONFIG_IOCTL(0x0)
#define IOCTL_BUSENUM_UNPLUG_HARDWARE               AALBUS_MONITOR_CONFIG_IOCTL(0x1)
#define IOCTL_BUSENUM_EJECT_HARDWARE                AALBUS_MONITOR_CONFIG_IOCTL(0x2)


typedef struct _AAL_BUS_MONITORCONFIG_CONTEXT {

   PVOID   ControlData; // Store your control data here

} AAL_BUS_MONITORCONFIG_CONTEXT, *PAAL_BUS_MONITORCONFIG_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME( AAL_BUS_MONITORCONFIG_CONTEXT,
                                    MonitorConfigGetContext)

END_NAMESPACE(AAL)

#endif //__AALSDK_KERNEL_BUS_IMONITOR_CONFIG_H__

