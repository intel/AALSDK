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
/// @file aalbus_configcntrl.c
/// @brief The AAL Bus is notified of changes in resources through the 
///          UpdateConfig interface. This module implements that interface
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
#include "aalsdk/kernel/kosal.h"
#include "aalbus_internal.h"

#include <Initguid.h>               // This header must be included here so that the 
#include "aalsdk/kernel/aalbus_iupdate_config.h"

#include <wdf.h>

BOOLEAN Bus_UpdateConfigAddDevice(WDFDEVICE, PVOID);
BOOLEAN Bus_UpdateConfigRemoveDevice(WDFDEVICE, PVOID);
BOOLEAN Bus_UpdateConfigDeviceStateChange(WDFDEVICE, PVOID);

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE,Bus_UpdateConfigAddDevice)
#pragma alloc_text (PAGE,Bus_UpdateConfigRemoveDevice)
#pragma alloc_text (PAGE,Bus_UpdateConfigDeviceStateChange)
#endif


//=============================================================================
// Name: Bus_EvtConfigStateUpdateIoDeviceControl
// Description:  Ioctl interface to the ConfigStateMonitor interface
// Interface: public
// Inputs:
// Comments: 
//=============================================================================
VOID 
Bus_EvtConfigStateUpdateIoDeviceControl(IN WDFQUEUE     Queue,
                                          IN WDFREQUEST   Request,
                                          IN size_t       OutputBufferLength,
                                          IN size_t       InputBufferLength,
                                          IN ULONG        IoControlCode)

{
   NTSTATUS                 status = STATUS_INVALID_PARAMETER;
   WDFDEVICE                hDevice;
   size_t                   length = 0;
   struct IUpdateConfig_DeviceManifest  *manifest = NULL;

   UNREFERENCED_PARAMETER(OutputBufferLength);

   PAGED_CODE();

   hDevice = WdfIoQueueGetDevice(Queue);

   KdPrint(("Bus_EvtConfigStateUpdateIoDeviceControl: 0x%p\n", hDevice));


   status = WdfRequestRetrieveInputBuffer(Request,
	   sizeof(struct IUpdateConfig_DeviceManifest),
	   &manifest,
	   &length);
   if (!NT_SUCCESS(status)) {
	   KdPrint(("WdfRequestRetrieveInputBuffer failed 0x%x\n", status));
	   return status;
   }

   ASSERT(length == InputBufferLength);

   if (length < sizeof(struct IUpdateConfig_DeviceManifest) + manifest->manifest_size ||
	   length < manifest->size){
	   WdfRequestCompleteWithInformation(Request, status, length);

	   return STATUS_INVALID_PARAMETER;
   }



   switch (IoControlCode) {
   case IOCTL_AALBUS_ADD_DEVICE:
      
	   if (TRUE == Bus_UpdateConfigAddDevice(hDevice, manifest)){
		   status = STATUS_SUCCESS;
	   }
	   break;

   case IOCTL_AALBUS_REMOVE_DEVICE:
	   if(TRUE == Bus_UpdateConfigRemoveDevice(hDevice, manifest)){
         status = STATUS_SUCCESS;
      }
	   break;

   case IOCTL_AALBUS_UPDATE_DEVICE_STATE:
      if (TRUE == Bus_UpdateConfigDeviceStateChange(hDevice, manifest)){
         status = STATUS_SUCCESS;
      }
      break;

   default:
      break; // default status is STATUS_INVALID_PARAMETER
   }

   WdfRequestCompleteWithInformation(Request, status, length);
}

//=============================================================================
// Name: Bus_EnableIUpdateInterface
// Description:  Enables the IUpdateConfig interface
// Interface: public
// Inputs: Device - Bus PDO 
// Outputs: STATUS_SUCCESS if successful,
//          STATUS_UNSUCCESSFUL if failure.
// Comments: 
//=============================================================================
_Use_decl_annotations_
   NTSTATUS Bus_EnableIUpdateInterface(WDFDEVICE device)
{
      WDF_IO_QUEUE_CONFIG              queueConfig;
      WDFQUEUE                         queue;
      NTSTATUS                         status = STATUS_INVALID_PARAMETER;
      UPDATECONFIG_INTERFACE_STANDARD  ConfigUpdateInterface;
      WDF_QUERY_INTERFACE_CONFIG       qiConfig;

      //
      // This queue is used for processing the configuration update interface
      WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE( &queueConfig,
                                               WdfIoQueueDispatchParallel);

      queueConfig.EvtIoDeviceControl = Bus_EvtConfigStateUpdateIoDeviceControl;

      //
      // By default, Static Driver Verifier (SDV) displays a warning if it 
      // doesn't find the EvtIoStop callback on a power-managed queue. 
      // The 'assume' below causes SDV to suppress this warning. If the driver 
      // has not explicitly set PowerManaged to WdfFalse, the framework creates
      // power-managed queues when the device is not a filter driver.  Normally 
      // the EvtIoStop is required for power-managed queues, but for this driver
      // it is not needed b/c the driver doesn't hold on to the requests or 
      // forward them to other drivers. This driver completes the requests 
      // directly in the queue's handlers. If the EvtIoStop callback is not 
      // implemented, the framework waits for all driver-owned requests to be
      // done before moving in the Dx/sleep states or before removing the 
      // device, which is the correct behavior for this type of driver.
      // If the requests were taking an indeterminate amount of time to complete,
      // or if the driver forwarded the requests to a lower driver/another stack,
      // the queue should have an EvtIoStop/EvtIoResume.  
      __analysis_assume(queueConfig.EvtIoStop != 0);
      status = WdfIoQueueCreate(  device,
                                 &queueConfig,
                                  WDF_NO_OBJECT_ATTRIBUTES,
                                 &queue);
      __analysis_assume(queueConfig.EvtIoStop == 0);

      if (!NT_SUCCESS(status)) {
         PERR("WdfIoQueueCreate failed status 0x%x\n", status);
         return status;
      }

      //
      // Create device interface for this device. The interface will be
      // enabled by the framework when we return from StartDevice successfully.
      // Clients of this driver will open this interface and send ioctls.
      status = WdfDeviceCreateDeviceInterface(  device,
                                                &GUID_DEVINTERFACE_AALBUS_UPDATECONFIG,
                                                NULL);
      if (!NT_SUCCESS(status)) {
         PERR("WdfDeviceCreateDeviceInterface failed status 0x%x\n", status);
         return status;
      }

      //
      // Initialize the interface for drivers.  Drivers use the Query Interface
      //  model.
      RtlZeroMemory(&ConfigUpdateInterface, sizeof(ConfigUpdateInterface));

      ConfigUpdateInterface.InterfaceHeader.Size = sizeof(ConfigUpdateInterface);
      ConfigUpdateInterface.InterfaceHeader.Version = 1;
      ConfigUpdateInterface.InterfaceHeader.Context = (PVOID)device;

      //
      // Let the framework handle reference counting. The functions are stubs.
      ConfigUpdateInterface.InterfaceHeader.InterfaceReference = WdfDeviceInterfaceReferenceNoOp;
      ConfigUpdateInterface.InterfaceHeader.InterfaceDereference = WdfDeviceInterfaceDereferenceNoOp;

      ConfigUpdateInterface.AddDevice              = Bus_UpdateConfigAddDevice;
      ConfigUpdateInterface.RemoveDevice           = Bus_UpdateConfigRemoveDevice;
      ConfigUpdateInterface.EvtDeviceStateChange   = Bus_UpdateConfigDeviceStateChange;

      WDF_QUERY_INTERFACE_CONFIG_INIT( &qiConfig,
                                       (PINTERFACE)&ConfigUpdateInterface,
                                       &GUID_AALBUS_UPDATECONFIG_INTERFACE_STANDARD,
                                        NULL);
      //
      // Add the interface
      status = WdfDeviceAddQueryInterface(device, &qiConfig);

      if (!NT_SUCCESS(status)) {
         return status;
      }
}

//=============================================================================
// Name: Bus_UpdateConfigAddDevice
// Description:  Called by a device managment enity to indicate that a new 
//               logical device is to be created.
// Interface: public
// Inputs: Device - Bus PDO 
//         Reocrd - Logical device information
// Outputs: TRUE  if successful,
//          FALSE if failure.
// Comments: 
//=============================================================================
_Use_decl_annotations_
BOOLEAN Bus_UpdateConfigAddDevice( _In_ WDFDEVICE hDevice,
                                   _In_ struct IUpdateConfig_DeviceManifest  *pmanifest)
{
   AAL_DEVICE_PDO_IDENTIFICATION_DESCRIPTION  description;
   NTSTATUS                                   status;
 
   PAGED_CODE();


   //
   // Initialize the description with the information about the newly
   // plugged in device.
   WDF_CHILD_IDENTIFICATION_DESCRIPTION_HEADER_INIT( &description.Header,
                                                     sizeof(AAL_DEVICE_PDO_IDENTIFICATION_DESCRIPTION));
   
   description.pManifest = kosal_kmalloc(pmanifest->size);

   RtlCopyMemory(description.pManifest, pmanifest, pmanifest->size);

   //
   // Call the framework to add this child to the childlist. This call
   // will internaly call our DescriptionCompare callback to check
   // whether this device is a new device or existing device. If
   // it's a new device, the framework will call DescriptionDuplicate to create
   // a copy of this description in nonpaged pool.
   // The actual creation of the child device will happen when the framework
   // receives QUERY_DEVICE_RELATION request from the PNP manager in
   // response to InvalidateDeviceRelations call made as part of adding
   // a new child.
   status = WdfChildListAddOrUpdateChildDescriptionAsPresent(  WdfFdoGetDefaultChildList(hDevice), 
                                                              &description.Header,
                                                               NULL); // AddressDescription
   if(status == STATUS_OBJECT_NAME_EXISTS) {
      //
      // The description is already present in the list
      status = STATUS_INVALID_PARAMETER;
   }

   kosal_kfree(description.pManifest, pmanifest->size);
   return status;
}

//=============================================================================
// Name: Bus_UpdateConfigRemoveDevice
// Description:  Called by a device managment enity to indicate that a new 
//               logical device is to be created.
// Interface: public
// Inputs: Device - Bus PDO 
//         Reocrd - Logical device information
// Outputs: TRUE  if successful,
//          FALSE if failure.
// Comments: 
//=============================================================================
_Use_decl_annotations_
BOOLEAN Bus_UpdateConfigRemoveDevice( _In_ WDFDEVICE   Device,
									  _In_ struct IUpdateConfig_DeviceManifest  *manifest)
{
   NTSTATUS       status = STATUS_INSUFFICIENT_RESOURCES;
   WDFCHILDLIST   list;

   PAGED_CODE();
#if 0
   list = WdfFdoGetDefaultChildList(Device);

   if (0 == SerialNo) {
      //
      // Unplug everybody.  We do this by starting a scan and then not reporting
      // any children upon its completion
      //
      status = STATUS_SUCCESS;

      WdfChildListBeginScan(list);
      //
      // A call to WdfChildListBeginScan indicates to the framework that the
      // driver is about to scan for dynamic children. After this call has
      // returned, all previously reported children associated with this will be
      // marked as potentially missing.  A call to either
      // WdfChildListUpdateChildDescriptionAsPresent  or
      // WdfChildListMarkAllChildDescriptionsPresent will mark all previuosly
      // reported missing children as present.  If any children currently
      // present are not reported present by calling
      // WdfChildListUpdateChildDescriptionAsPresent at the time of
      // WdfChildListEndScan, they will be reported as missing to the PnP subsystem
      // After WdfChildListEndScan call has returned, the framework will
      // invalidate the device relations for the FDO associated with the list
      // and report the changes
      //
      WdfChildListEndScan(list);

   }
   else {
      AAL_DEVICE_PDO_IDENTIFICATION_DESCRIPTION description;

      WDF_CHILD_IDENTIFICATION_DESCRIPTION_HEADER_INIT(
         &description.Header,
         sizeof(description)
         );

      description.SerialNo = SerialNo;
      //
      // WdfFdoUpdateChildDescriptionAsMissing indicates to the framework that a
      // child device that was previuosly detected is no longe present on the bus.
      // This API can be called by itself or after a call to WdfChildListBeginScan.
      // After this call has returned, the framework will invalidate the device
      // relations for the FDO associated with the list and report the changes.
      //
      status = WdfChildListUpdateChildDescriptionAsMissing(list,
         &description.Header);
      if (status == STATUS_NO_SUCH_DEVICE) {
         //
         // serial number didn't exist. Remap it to a status that user
         // application can understand when it gets translated to win32
         // error code.
         //
         status = STATUS_INVALID_PARAMETER;
      }
   }
#endif
   return status;
}

//=============================================================================
// Name: Bus_UpdateConfigDeviceStateChange
// Description:  Called by a device managment enity to indicate that a new 
//               logical device is to be created.
// Interface: public
// Inputs: Device - Bus PDO 
//         Reocrd - Logical device information
// Outputs: TRUE  if successful,
//          FALSE if failure.
// Comments: 
//=============================================================================
_Use_decl_annotations_
BOOLEAN Bus_UpdateConfigDeviceStateChange( _In_ WDFDEVICE   Device,
										   _In_ struct IUpdateConfig_DeviceManifest  *manifest)
{
   NTSTATUS         status = STATUS_INVALID_PARAMETER;
#if 0 
   WDFDEVICE        hChild;

   WDFCHILDLIST     list;

   PAGED_CODE();

   list = WdfFdoGetDefaultChildList(Device);

   //
   // A zero serial number means eject all children
   //
   if (0 == SerialNo) {
      WDF_CHILD_LIST_ITERATOR     iterator;

      WDF_CHILD_LIST_ITERATOR_INIT(&iterator,
         WdfRetrievePresentChildren);

      WdfChildListBeginIteration(list, &iterator);

      for (;;) {
         WDF_CHILD_RETRIEVE_INFO         childInfo;
         AAL_DEVICE_PDO_IDENTIFICATION_DESCRIPTION  description;
         BOOLEAN                         ret;

         //
         // Init the structures.
         //
         WDF_CHILD_IDENTIFICATION_DESCRIPTION_HEADER_INIT(&description.Header, sizeof(description));
         WDF_CHILD_RETRIEVE_INFO_INIT(&childInfo, &description.Header);

         WDF_CHILD_IDENTIFICATION_DESCRIPTION_HEADER_INIT(
            &description.Header,
            sizeof(description)
            );
         //
         // Get the device identification description
         //
         status = WdfChildListRetrieveNextDevice(list,
            &iterator,
            &hChild,
            &childInfo);

         if (!NT_SUCCESS(status) || status == STATUS_NO_MORE_ENTRIES) {
            break;
         }

         ASSERT(childInfo.Status == WdfChildListRetrieveDeviceSuccess);

         //
         // Use that description to request an eject.
         //
         ret = WdfChildListRequestChildEject(list, &description.Header);
         if (!ret) {
            WDFVERIFY(ret);
         }

      }

      WdfChildListEndIteration(list, &iterator);

      if (status == STATUS_NO_MORE_ENTRIES) {
         status = STATUS_SUCCESS;
      }

   }
   else {

      AAL_DEVICE_PDO_IDENTIFICATION_DESCRIPTION description;

      WDF_CHILD_IDENTIFICATION_DESCRIPTION_HEADER_INIT(
         &description.Header,
         sizeof(description)
         );

      description.SerialNo = SerialNo;

      if (WdfChildListRequestChildEject(list, &description.Header)) {
         status = STATUS_SUCCESS;
      }
   }
#endif
   return status;
}

