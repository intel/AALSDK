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
/// @file aalbus_resmonitor.c
/// @brief The AAL Bus resource monitor is an object that exposes the RM 
///          API to user mode.
/// @ingroup System
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Joseph Grecco, Intel Corporation.
///
/// HISTORY: 
/// WHEN:          WHO:     WHAT:
/// 05/4/2014      JG       Initial version of aalrmsserver created
//****************************************************************************
#include "aalbus_internal.h"
#include "aalsdk/kernel/aalbus_imonitorconfig.h"

//=============================================================================
// Name: Bus_EvtConfigStateMonitorIoDeviceControl
// Description:  Ioctl interface to the ConfigStateUpdate interface
// Interface: public
// Inputs:
// Comments: 
//=============================================================================
VOID
Bus_EvtConfigStateMonitorIoDeviceControl(IN WDFQUEUE     Queue,
                                         IN WDFREQUEST   Request,
                                         IN size_t       OutputBufferLength,
                                         IN size_t       InputBufferLength,
                                         IN ULONG        IoControlCode)
{
   NTSTATUS                 status = STATUS_INVALID_PARAMETER;
   WDFDEVICE                hDevice;
   size_t                   length = 0;

   UNREFERENCED_PARAMETER(OutputBufferLength);

   PAGED_CODE();
#if 0 
   hDevice = WdfIoQueueGetDevice(Queue);

   KdPrint(("Bus_EvtConfigStateMonitorIoDeviceControl: 0x%p\n", hDevice));

   switch (IoControlCode) {
   case IOCTL_BUSENUM_PLUGIN_HARDWARE:

      status = WdfRequestRetrieveInputBuffer(Request,
         sizeof (BUSENUM_PLUGIN_HARDWARE)+
         (sizeof(UNICODE_NULL)* 2), // 2 for double NULL termination (MULTI_SZ)
         &plugIn, &length);
      if (!NT_SUCCESS(status)) {
         KdPrint(("WdfRequestRetrieveInputBuffer failed 0x%x\n", status));
         break;
      }

      ASSERT(length == InputBufferLength);

      if (sizeof (BUSENUM_PLUGIN_HARDWARE) == plugIn->Size)
      {

         length = (InputBufferLength - sizeof (BUSENUM_PLUGIN_HARDWARE)) / sizeof(WCHAR);
         //
         // Make sure the IDs is two NULL terminated.
         //
         if ((UNICODE_NULL != plugIn->HardwareIDs[length - 1]) ||
            (UNICODE_NULL != plugIn->HardwareIDs[length - 2])) {

            status = STATUS_INVALID_PARAMETER;
            break;
         }

         status = Bus_PlugInDevice(hDevice,
            plugIn->HardwareIDs,
            length,
            plugIn->SerialNo);
      }

      break;

   case IOCTL_BUSENUM_UNPLUG_HARDWARE:

      status = WdfRequestRetrieveInputBuffer(Request,
         sizeof(BUSENUM_UNPLUG_HARDWARE),
         &unPlug,
         &length);
      if (!NT_SUCCESS(status)) {
         KdPrint(("WdfRequestRetrieveInputBuffer failed 0x%x\n", status));
         break;
      }

      if (unPlug->Size == InputBufferLength)
      {

         status = Bus_UnPlugDevice(hDevice, unPlug->SerialNo);

      }

      break;

   case IOCTL_BUSENUM_EJECT_HARDWARE:

      status = WdfRequestRetrieveInputBuffer(Request,
         sizeof (BUSENUM_EJECT_HARDWARE),
         &eject, &length);
      if (!NT_SUCCESS(status)) {
         KdPrint(("WdfRequestRetrieveInputBuffer failed 0x%x\n", status));
         break;
      }

      if (eject->Size == InputBufferLength)
      {
         status = Bus_EjectDevice(hDevice, eject->SerialNo);

      }

      break;

   default:
      break; // default status is STATUS_INVALID_PARAMETER
   }
#endif
   WdfRequestCompleteWithInformation(Request, status, length);
}


//=============================================================================
// Name: Bus_EnableIConfigStateMonitor
// Description:  Enables the ConfigStateMonitor API
// Interface: public
// Inputs: Device - Bus PDO 
// Outputs: STATUS_SUCCESS if successful,
//          STATUS_UNSUCCESSFUL if failure.
// Comments: 
//=============================================================================
_Use_decl_annotations_
   NTSTATUS Bus_EnableIConfigStateMonitor(WDFDEVICE device)

{
      PWDFDEVICE_INIT                  pInit = NULL;
      WDFDEVICE                        controlDevice = NULL;
      WDF_OBJECT_ATTRIBUTES            controlAttributes;
      WDF_IO_QUEUE_CONFIG              ioQueueConfig;
      NTSTATUS                         status;
      WDFQUEUE                         queue;
      PAAL_BUS_FDO_DEVICE_CONTEXT      pBusContext = NULL;

      DECLARE_CONST_UNICODE_STRING(ntDeviceName, NTDEVICE_MONITORCONFIG_NAME_STRING);
      DECLARE_CONST_UNICODE_STRING(symbolicLinkName, SYMBOLIC_MONITORCONFIG_NAME_STRING);

      PAGED_CODE();

      KdPrint(("Creating Control Device\n"));

      //
      //
      // In order to create a control device, we first need to allocate a
      // WDFDEVICE_INIT structure and set all properties.
      pInit = WdfControlDeviceInitAllocate(WdfDeviceGetDriver(device),
         &SDDL_DEVOBJ_SYS_ALL_ADM_RWX_WORLD_RW_RES_R);

      if (pInit == NULL) {
         status = STATUS_INSUFFICIENT_RESOURCES;
         goto Error;
      }

      //
      // Set exclusive to false so that more than one app can talk to the
      // control device simultaneously.
      WdfDeviceInitSetExclusive(pInit, FALSE);

      status = WdfDeviceInitAssignName(pInit, &ntDeviceName);

      if (!NT_SUCCESS(status)) {
         goto Error;
      }

      //
      // Specify the size of device context - I.e., private data to this object
      WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE( &controlAttributes,
                                                AAL_BUS_MONITORCONFIG_CONTEXT);
      status = WdfDeviceCreate(&pInit,
         &controlAttributes,
         &controlDevice);
      if (!NT_SUCCESS(status)) {
         goto Error;
      }

      //
      // Create a symbolic link for the control object so that usermode can open
      // the device.
      status = WdfDeviceCreateSymbolicLink(  controlDevice,
                                            &symbolicLinkName);
      if (!NT_SUCCESS(status)) {
         goto Error;
      }

      //
      // Configure the default queue associated with the control device object
      // to be Serial so that request passed to EvtIoDeviceControl are serialized.
      WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE( &ioQueueConfig,
                                               WdfIoQueueDispatchSequential);
      ioQueueConfig.EvtIoDeviceControl = Bus_EvtConfigStateMonitorIoDeviceControl;

      //
      // Framework by default creates non-power managed queues for
      // filter drivers.
      status = WdfIoQueueCreate(  controlDevice,
                                 &ioQueueConfig,
                                  WDF_NO_OBJECT_ATTRIBUTES,
                                 &queue);
      if (!NT_SUCCESS(status)) {
         goto Error;
      }

      //
      // Control devices must notify WDF when they are done initializing.   I/O is
      // rejected until this call is made.
      WdfControlFinishInitializing(controlDevice);
      pBusContext = AALBusGetContext(device);

      // Save the object in the bus context
      pBusContext->pMontorConfigAPIControlDevice = controlDevice;

      return STATUS_SUCCESS;

   Error:

      if (pInit != NULL) {
         WdfDeviceInitFree(pInit);
      }

      if (controlDevice != NULL) {
         //
         // Release the reference on the newly created object, since
         // we couldn't initialize it.
         //
         WdfObjectDelete(controlDevice);
         controlDevice = NULL;
      }

      return status;
   }

_Use_decl_annotations_
   VOID
   FilterDeleteControlDevice(
   WDFDEVICE Device
   )
   /*++

   Routine Description:

   This routine deletes the control by doing a simple dereference.

   Arguments:

   Device - Handle to a framework filter device object.

   Return Value:

   WDF status code

   --*/
{
      UNREFERENCED_PARAMETER(Device);

      PAGED_CODE();

      KdPrint(("Deleting Control Device\n"));
#if 0 
      if (ControlDevice) {
         WdfObjectDelete(ControlDevice);
         ControlDevice = NULL;
      }
#endif
   }




