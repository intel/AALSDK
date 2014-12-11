// Copyright (c) 2014, Intel Corporation
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
/// @file aalbus_internal.h
/// @brief Private definitions for the AAL Windows Bus driver.
/// @ingroup System
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Joseph Grecco, Intel Corporation.
///
/// HISTORY: 
/// WHEN:          WHO:     WHAT:
/// 05/9/2014      JG       Initial version
//****************************************************************************
#include <ntddk.h>
#include <wdf.h>
#define NTSTRSAFE_LIB
#include <ntstrsafe.h>
#include <ntintsafe.h>

#include <aalbusMof.h>


#ifndef __AALBUS_INTERNAL_H__
#define BUSENUM_H

extern ULONG BusEnumDebugLevel;

#define BUS_TAG         'bLAA'
#define BUSRESOURCENAME L"MofAALBusResourceName"

#ifndef min
#define min(_a, _b)     (((_a) < (_b)) ? (_a) : (_b))
#endif

#ifndef max
#define max(_a, _b)     (((_a) > (_b)) ? (_a) : (_b))
#endif

//
// Structure for reporting data to WMI
//

typedef AALBusInformation AAL_BUS_WMI_STD_DATA, *PAAL_BUS_WMI_STD_DATA;

//
// The goal of the identification and address description abstractions is that enough
// information is stored for a discovered device so that when it appears on the bus,
// the framework (with the help of the driver writer) can determine if it is a new or
// existing device.  The identification and address descriptions are opaque structures
// to the framework, they are private to the driver writer.  The only thing the framework
// knows about these descriptions is what their size is.
// The identification contains the bus specific information required to recognize
// an instance of a device on its the bus.  The identification information usually
// contains device IDs along with any serial or slot numbers.
// For some buses (like USB and PCI), the identification of the device is sufficient to
// address the device on the bus; in these instances there is no need for a separate
// address description.  Once reported, the identification description remains static
// for the lifetime of the device.  For example, the identification description that the
// PCI bus driver would use for a child would contain the vendor ID, device ID,
// subsystem ID, revision, and class for the device. This sample uses only identification
// description.
// On other busses (like 1394 and auto LUN SCSI), the device is assigned a dynamic
// address by the hardware (which may reassigned and updated periodically); in these
// instances the driver will use the address description to encapsulate this dynamic piece
// of data.    For example in a 1394 driver, the address description would contain the
// device's current generation count while the identification description would contain
// vendor name, model name, unit spec ID, and unit software version.
//
typedef struct _AAL_DEVICE_PDO_IDENTIFICATION_DESCRIPTION
{
   WDF_CHILD_IDENTIFICATION_DESCRIPTION_HEADER Header; // should contain this header

   // Device attributes
   struct IUpdateConfig_DeviceManifest        *pManifest;

} AAL_DEVICE_PDO_IDENTIFICATION_DESCRIPTION, *PAAL_DEVICE_PDO_IDENTIFICATION_DESCRIPTION;


//
// The context of the bus device itself.  
//
typedef struct _AAL_BUS_FDO_DEVICE_CONTEXT
{
    AAL_BUS_WMI_STD_DATA   StdAALdeviceData;
    WDFDEVICE              pMontorConfigAPIControlDevice;

} AAL_BUS_FDO_DEVICE_CONTEXT, *PAAL_BUS_FDO_DEVICE_CONTEXT;

extern PAAL_BUS_FDO_DEVICE_CONTEXT AALBusGetContext(WDFOBJECT);


//
// Prototypes of functions
//

DRIVER_INITIALIZE DriverEntry;

EVT_WDF_DRIVER_DEVICE_ADD Bus_EvtDeviceAdd;

NTSTATUS Bus_EnableIUpdateInterface(WDFDEVICE);
NTSTATUS Bus_EnableIConfigStateMonitor(WDFDEVICE);

EVT_WDF_CHILD_LIST_CREATE_DEVICE Bus_EvtDeviceListCreatePdo;
EVT_WDF_CHILD_LIST_IDENTIFICATION_DESCRIPTION_COMPARE Bus_EvtChildListIdentificationDescriptionCompare;
EVT_WDF_CHILD_LIST_IDENTIFICATION_DESCRIPTION_CLEANUP Bus_EvtChildListIdentificationDescriptionCleanup;
EVT_WDF_CHILD_LIST_IDENTIFICATION_DESCRIPTION_DUPLICATE Bus_EvtChildListIdentificationDescriptionDuplicate;


NTSTATUS
Bus_CreatePdo(
    _In_ WDFDEVICE       Device,
    _In_ PWDFDEVICE_INIT ChildInit,
    _In_reads_(MAX_INSTANCE_ID_LEN) PCWSTR HardwareIds,
    _In_ ULONG           SerialNo
    );


//
// Defined in wmi.c
//

NTSTATUS
Bus_WmiRegistration(
    WDFDEVICE      Device
    );

EVT_WDF_WMI_INSTANCE_SET_ITEM Bus_EvtStdDataSetItem;
EVT_WDF_WMI_INSTANCE_SET_INSTANCE Bus_EvtStdDataSetInstance;
EVT_WDF_WMI_INSTANCE_QUERY_INSTANCE Bus_EvtStdDataQueryInstance;

#endif

