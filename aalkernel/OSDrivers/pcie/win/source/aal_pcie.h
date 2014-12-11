//****************************************************************************
//                               INTEL CONFIDENTIAL
//
//        Copyright (c) 2014 Intel Corporation All Rights Reserved.
//
// The source code contained or described herein and all  documents related to
// the  source  code  ("Material")  are  owned  by  Intel  Corporation  or its
// suppliers  or  licensors.    Title  to  the  Material  remains  with  Intel
// Corporation or  its suppliers  and licensors.  The Material  contains trade
// secrets  and  proprietary  and  confidential  information  of  Intel or its
// suppliers and licensors.  The Material is protected  by worldwide copyright
// and trade secret laws and treaty provisions. No part of the Material may be
// used,   copied,   reproduced,   modified,   published,   uploaded,  posted,
// transmitted,  distributed,  or  disclosed  in any way without Intel's prior
// express written permission.
//
// No license under any patent,  copyright, trade secret or other intellectual
// property  right  is  granted  to  or  conferred  upon  you by disclosure or
// delivery  of  the  Materials, either expressly, by implication, inducement,
// estoppel or otherwise.  Any license under such intellectual property rights
// must be express and approved by Intel in writing.
//****************************************************************************
//****************************************************************************
//        FILE: aal_pcie.h
//     CREATED: 04/30/2014
//      AUTHOR: Joseph Grecco, Intel Corporation
//
// PURPOSE: Definitions used by the AAL Windows PCIe shell driver
//
// HISTORY:
// WHEN:          WHO:     WHAT:
//****************************************************************************
#ifndef _AAL_PCIE_H_
#define _AAL_PCIE_H_
#include <ntddk.h>
#include <wdf.h>
#define NTSTRSAFE_LIB
#include <ntstrsafe.h>
#include <wmilib.h>
#include <initguid.h>
#include <wdmguid.h> // required for WMILIB_CONTEXT

//#include "spl2_pip_internal.h"
#include "aal_pciedriver.h"

#define AALPCIE_POOL_TAG (ULONG) 'plaa'

#define MOFRESOURCENAME L"AALPCIEEWMI"

//
// The device extension for the device object
//
typedef struct _WIN_AALPCIE_DEVICE
{
//   struct spl2_device      m_spl2_device;
   WDFDEVICE               m_WdfDevice;
   WDFWMIINSTANCE          m_WmiDeviceArrivalEvent;

   BOOLEAN                 m_WmiPowerDeviceEnableRegistered;
   BUS_INTERFACE_STANDARD  m_BusInterface;

   PMDL                    m_pmdl;

   WDFREQUEST              currReq;

   // AAL queue
//   kosal_work_queue        m_workq;
//   work_object             task_handler;
//   pkosal_os_dev           m_dev;

//   struct uidrv_session *  m_uidrvsess;

   BOOLEAN                 m_softdevice;

}  WIN_AALPCIE_DEVICE, *PWIN_AALPCIE_DEVICE;


WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(WIN_AALPCIE_DEVICE, AALPCIEFdoGetData)

// Converts a Windows AALPCIE Device to a common AALPCIE Device
#define WIN_AALPCIE_DEVICE_TO_AALPCIE_DEVICE(d)       ((d).m_spl2_device)

#define PWIN_AALPCIE_DEVICE_TO_WDF_DEVICE(d)       ((d)->m_WdfDevice)
#define PWIN_AALPCIE_DEVICE_BUS(d)                 ((d)->m_BusInterface)

#define PWIN_AALPCIE_DEVICE_TO_PAALPCIE_DEVICE(d)     (&((d)->m_spl2_device))
#define PWIN_AALPCIE_DEVICE_TO_UIDRV_SESS(d)       (&((d)->m_uidrvsess))

//
// Connector Types
//



DRIVER_INITIALIZE DriverEntry;
NTSTATUS AALPCIEMapHWResources( IN OUT PWIN_AALPCIE_DEVICE,
                             IN WDFCMRESLIST);

NTSTATUS AALPCIEUnMapHWResources( IN OUT PWIN_AALPCIE_DEVICE FdoData);
NTSTATUS AALPCIEGetDeviceInformation( IN PWIN_AALPCIE_DEVICE fdoData );
NTSTATUS AALPCIEEvtDeviceD0Entry(IN  WDFDEVICE Device, IN  WDF_POWER_DEVICE_STATE PreviousState);
NTSTATUS AALPCIEEvtDeviceD0Exit(IN  WDFDEVICE Device, IN  WDF_POWER_DEVICE_STATE TargetState);


EVT_WDF_DRIVER_DEVICE_ADD AALPCIEEvtDeviceAdd;

EVT_WDF_DEVICE_CONTEXT_CLEANUP AALPCIEEvtDeviceContextCleanup;
EVT_WDF_DEVICE_D0_ENTRY AALPCIEEvtDeviceD0Entry;
EVT_WDF_DEVICE_D0_EXIT AALPCIEEvtDeviceD0Exit;
EVT_WDF_DEVICE_PREPARE_HARDWARE AALPCIEEvtDevicePrepareHardware;
EVT_WDF_DEVICE_RELEASE_HARDWARE AALPCIEEvtDeviceReleaseHardware;

EVT_WDF_DEVICE_SELF_MANAGED_IO_INIT AALPCIEEvtDeviceSelfManagedIoInit;

//
// Io events callbacks.
//
//EVT_WDF_IO_QUEUE_IO_READ AALPCIEEvtIoRead;
//EVT_WDF_IO_QUEUE_IO_WRITE AALPCIEEvtIoWrite;
EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL AALPCIEEvtIoDeviceControl;
//EVT_WDF_DEVICE_FILE_CREATE AALPCIEEvtDeviceFileCreate;
//EVT_WDF_FILE_CLOSE AALPCIEEvtFileClose;

#endif  // _AAL_PCIE_H_
