//****************************************************************************
//                               INTEL CONFIDENTIAL
//
//        Copyright (c) 2016 Intel Corporation All Rights Reserved.
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
//        FILE: cci_pcie_windows.h
//     CREATED: 01/26/2016
//      AUTHOR: Joseph Grecco, Intel Corporation
//
// PURPOSE:
//
// HISTORY:
// WHEN:          WHO:     WHAT:
//****************************************************************************
#ifndef _CCI_PCIE_WINDOWS_H_
#define _CCI_PCIE_WINDOWS_H_
#include "aalsdk/kernel/kosal.h"

#include "cci_pcie_driver_internal.h"

#define CCIP_POOL_TAG (ULONG) 'picc'

#define MOFRESOURCENAME L"CCIPWMI"

//
// The device extension for the device object
//
typedef struct _CCI_DEVICE
{
   struct ccip_device      m_cci_device;
   kosal_list_head         g_device_list;

   WDFDEVICE               m_WdfDevice;
   WDFWMIINSTANCE          m_WmiDeviceArrivalEvent;

   BOOLEAN                 m_WmiPowerDeviceEnableRegistered;
   BUS_INTERFACE_STANDARD  m_BusInterface;

   PMDL                    m_pmdl;

   WDFREQUEST              currReq;

   // AAL queue
   kosal_work_queue        m_workq;
   work_object             task_handler;
   pkosal_os_dev           m_dev;

//   struct uidrv_session *  m_uidrvsess;

   BOOLEAN                 m_softdevice;

}  CCI_DEVICE, *PCCI_DEVICE;

extern PCCI_DEVICE CCIFdoGetCCIDev( WDFOBJECT );

// Converts a Windows CCIPDrv Device to a common CCIPDrv Device
#define WIN_CCIP_DEVICE_TO_CCIP_DEVICE(d)       ((d).m_cci_device)

#define PWIN_CCIP_DEVICE_TO_WDF_DEVICE(d)       ((d)->m_WdfDevice)
#define PWIN_CCIP_DEVICE_BUS(d)                 ((d)->m_BusInterface)
/*
#define PWIN_CCIPDrv_DEVICE_TO_PCCIPDrv_DEVICE(d)     (&((d)->m_spl2_device))
#define PWIN_CCIPDrv_DEVICE_TO_UIDRV_SESS(d)       (&((d)->m_uidrvsess))
*/
//
// Connector Types
//


DRIVER_INITIALIZE DriverEntry;
NTSTATUS CCIPDrvMapHWResources( IN OUT PCCI_DEVICE,
                             IN WDFCMRESLIST);

NTSTATUS CCIPDrvUnMapHWResources( IN OUT PCCI_DEVICE FdoData );
NTSTATUS CCIPDrvGetDeviceInformation( IN PCCI_DEVICE fdoData );
NTSTATUS CCIPDrvEvtDeviceD0Entry(IN  WDFDEVICE Device, IN  WDF_POWER_DEVICE_STATE PreviousState);
NTSTATUS CCIPDrvEvtDeviceD0Exit(IN  WDFDEVICE Device, IN  WDF_POWER_DEVICE_STATE TargetState);


EVT_WDF_DRIVER_DEVICE_ADD CCIPDrvEvtDeviceAdd;

EVT_WDF_DEVICE_CONTEXT_CLEANUP CCIPDrvEvtDeviceContextCleanup;
EVT_WDF_DEVICE_D0_ENTRY CCIPDrvEvtDeviceD0Entry;
EVT_WDF_DEVICE_D0_EXIT CCIPDrvEvtDeviceD0Exit;
EVT_WDF_DEVICE_PREPARE_HARDWARE CCIPDrvEvtDevicePrepareHardware;
EVT_WDF_DEVICE_RELEASE_HARDWARE CCIPDrvEvtDeviceReleaseHardware;

EVT_WDF_DEVICE_SELF_MANAGED_IO_INIT CCIPDrvEvtDeviceSelfManagedIoInit;

NTSTATUS ccidrv_initUMAPI( WDFDEVICE );

//
// Io events callbacks.
//
//EVT_WDF_IO_QUEUE_IO_READ CCIPDrvEvtIoRead;
//EVT_WDF_IO_QUEUE_IO_WRITE CCIPDrvEvtIoWrite;
EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL CCIPDrvEvtIoDeviceControl;
//EVT_WDF_DEVICE_FILE_CREATE CCIPDrvEvtDeviceFileCreate;
//EVT_WDF_FILE_CLOSE CCIPDrvEvtFileClose;

#endif  // _CCI_PCIE_WINDOWS_H_
