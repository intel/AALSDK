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
/// @file aalbus.c
/// @brief The AAL Bus is a logical device bus responsible for enumerating
///         and reporting changes in AAL resources (e.g., accelerators)
/// @ingroup System
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Joseph Grecco, Intel Corporation.
/// COMMENTS: The AAL Bus suports the dynamic management and 
///           provisioning of resources used by AAL Services. The most obvious
///           example of a resource is a programmable or fixed function 
///           accelerator. Resources are exposed upstream to the system as 
///           "logical" devices.  A logical device is an atomic unit of 
///           allocation. It typically represents some bundle of functionality
///           but does not necessarily have a 1:1 coorespondance to hardware
///           resources.  I.e., a single accelerator may expose multiple
///           "logical" devices or a single "logical" device may be implemented
///           via multiple discrete HW devices.
///
///           Unlike a HW bus, the AAL Bus does not have a means of discovering
///           devices.  Devices "appear" on the bus as a result of a management
///           entity notifying the Bus. The management entity may be a driver or
///           a user mode application. The AAL Bus implements an API for this
///           purpose.
///
///           The AAL Bus also implements an API to allow applications to be
///           notified as the state of a "logical" device changes.  This is 
///           typically used by Resource Management Services.
///
/// HISTORY: 
/// WHEN:          WHO:     WHAT:
/// 05/4/2014      JG       Initial version of aalrmsserver created
//****************************************************************************



#include "aalsdk/kernel/kosal.h"

#include "aalsdk/kernel/aalbus_iupdate_config.h"
#include <Initguid.h>               // Include to cause GUIDs to be implemented
                                    //  Do not include in more than 1 source
                                    //  modeule such that GUIDs will be defined
                                    // multiple times
#include "aalsdk/kernel/aalbus.h"   // Common definitions for AAL Bus
#include "aalbus_internal.h"        // Windows specific definitions


#define MODULE_FLAGS AALBUS_DBG_MOD

btUnsignedInt debug = 0
/* Type and Level selection flags */
| PTRACE_FLAG
| PVERBOSE_FLAG
| PDEBUG_FLAG
| PINFO_FLAG
| PNOTICE_FLAG
| PMEMORY_FLAG
| PPCI_FLAG
| PPOLLING_FLAG
/* Shared Module selection flags */
| KOSAL_DBG_MOD
| AALBUS_DBG_MOD
;

// Prototypes
VOID Bus_EvtUnload(_In_ WDFDRIVER Driver);
VOID Bus_EvtDeviceContextCleanup(WDFOBJECT Device);


WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(AAL_BUS_FDO_DEVICE_CONTEXT, AALBusGetContext);

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, Bus_EvtDeviceAdd)
#pragma alloc_text (PAGE, Bus_EvtUnload)
#pragma alloc_text (PAGE, Bus_EvtDeviceContextCleanup)
#endif

//=============================================================================
// Name:          DriverEntry
// Description:   Driver entry point 
// Interface:     public
// Inputs:  DriverObject -  A pointer to a DRIVER_OBJECT structure. 
//                          This is the driver's driver object.     
//          RegistryPath - A pointer to a counted Unicode string specifying the 
//                         path to the driver's registry key.
// Returns: NTSTATUS
// Comments:  Responsible for initializing the driver.
//=============================================================================
NTSTATUS
DriverEntry( IN PDRIVER_OBJECT DriverObject,
             IN PUNICODE_STRING RegistryPath )

{
   WDF_DRIVER_CONFIG   config;
   NTSTATUS            status;
   WDFDRIVER           driver;
    
   PTRACEIN;

   // Sign on message
   DbgPrint("Intel(R) QuickAssist Technology Accelerator Abstraction Layer\n");
   DbgPrint("Intel(R) AAL Bus\n");
   DbgPrint("Copyright (c) 2014 Intel Corporation\n");
#if ENABLE_DEBUG
   // These seem to only be available in Debug builds.
   DbgPrint("Built %s %s\n", __DATE__, __TIME__);
#endif // ENABLE_DEBUG
      
   // Set the Driver callbacks and other global data.
   WDF_DRIVER_CONFIG_INIT( &config, Bus_EvtDeviceAdd );
   config.EvtDriverUnload = Bus_EvtUnload;


   // Create a framework driver object to represent our driver.
   status = WdfDriverCreate(DriverObject,
                           RegistryPath,
                           WDF_NO_OBJECT_ATTRIBUTES,
                           &config,
                           &driver);

   if (!NT_SUCCESS(status)) {
      PERR("WdfDriverCreate failed with status 0x%x\n", status);
   }

   return status;

}
//=============================================================================
// Name:          Bus_EvtUnload
// Description:   
// Interface:     public
// Inputs:  DriverObject -  A pointer to a DRIVER_OBJECT structure. 
//                          This is the driver's driver object.     
// Returns: N/A
// Comments:  
//=============================================================================
VOID Bus_EvtUnload(_In_ WDFDRIVER Driver)
{
};


//=============================================================================
// Name:          Bus_EvtDeviceAdd
// Description:   Called as a result of the Bus itself coming into existance.
// Interface:     public
// Inputs:  Driver -  This driver's object
//          pDeviceInit - A pointer to a framework-allocated WDFDEVICE_INIT 
//                       structure.
// Returns: NTSTATUS
// Comments:  This device represents the bus itself and will usually be a 
//            singleton. This device is SW only.
//=============================================================================
NTSTATUS Bus_EvtDeviceAdd( IN WDFDRIVER        Driver,
                           IN PWDFDEVICE_INIT  pDeviceInit)
{
    WDF_CHILD_LIST_CONFIG      config;
    WDF_OBJECT_ATTRIBUTES      fdoAttributes;
    NTSTATUS                   status;
    WDFDEVICE                  device;
    PNP_BUS_INFORMATION        busInfo;

    WDFQUEUE                   queue;
    WDF_FILEOBJECT_CONFIG           fileObjectConfig;
    UNICODE_STRING DeviceName0, DeviceName1;
    DECLARE_CONST_UNICODE_STRING(ntDeviceName, NTDEVICE_UPDATECONFIG_NAME_STRING);

    UNREFERENCED_PARAMETER(Driver);

    PAGED_CODE ();

    PTRACEIN;

    //
    // Initialize all the properties specific to the device.
    WdfDeviceInitSetDeviceType(pDeviceInit, FILE_DEVICE_BUS_EXTENDER);

    // Allow multiple processes to open the control interface
    WdfDeviceInitSetExclusive(pDeviceInit, FALSE);

    //
    // WDF_ DEVICE_LIST_CONFIG describes how the framework should handle
    // dynamic child enumeration on behalf of the driver writer.
    // Since we are a bus driver, we need to specify identification description
    // for our child devices. This description will serve as the identity of our
    // child device. Since the description is opaque to the framework, we
    // have to provide bunch of callbacks to compare, copy, or free
    // any other resources associated with the description.
    WDF_CHILD_LIST_CONFIG_INIT(&config,
                                sizeof(AAL_DEVICE_PDO_IDENTIFICATION_DESCRIPTION),
                                Bus_EvtDeviceListCreatePdo );
    
    //
    // The following registered methods perform the task of copy, equivelence check and
    //  destruction of proprietary portions of Physical Device Object Description Object
    config.EvtChildListIdentificationDescriptionDuplicate =
                                Bus_EvtChildListIdentificationDescriptionDuplicate;

    config.EvtChildListIdentificationDescriptionCompare =
                                Bus_EvtChildListIdentificationDescriptionCompare;

    config.EvtChildListIdentificationDescriptionCleanup =
                                Bus_EvtChildListIdentificationDescriptionCleanup;

    //
    // Tell the framework to use the built-in childlist to track the state
    // of the devices based on the configuration we just created.
    WdfFdoInitSetDefaultChildListConfig( pDeviceInit,
                                         &config,
                                         WDF_NO_OBJECT_ATTRIBUTES);

    //
    // Initialize attributes structure to specify size and accessor function
    //  for storing device context.  The AAL_BUS_FDO_DEVICE_DATA object represents
    //  the data associated with the bus device and is used by the bus itself.
    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&fdoAttributes, AAL_BUS_FDO_DEVICE_CONTEXT);
    
    // Callback when a device is removed from the bus
    fdoAttributes.EvtCleanupCallback = Bus_EvtDeviceContextCleanup;

    // Create a custom interface so that other drivers can
    //  query (IRP_MN_QUERY_INTERFACE) and use our callbacks directly.
    //  This is the name used by applications that wish to access the
    //  IUpdateConfig interface from user mode.
    status = WdfDeviceInitAssignName(pDeviceInit, &ntDeviceName);

    if (!NT_SUCCESS(status)) {
       PERR("Failed to WdfDeviceInitAssignName status = 0x%x", status);
       return status;
    }

    //
    // Create a framework device object. In response to this call, the framework
    //  creates a WDM deviceobject and attaches it to the PDO.
    status = WdfDeviceCreate(&pDeviceInit, &fdoAttributes, &device);

    if (!NT_SUCCESS(status)) {
        PERR("Error creating device 0x%x\n", status);
        return status;
    }
    
    //
    // Set up the interfaces for this driver.  Its implements 2 independent
    //  interfaces. 
    //   - The IUpdateConfiguration is accessed from User mode and
    //       from other drivers and allows configuration entities to construct
    //       aal_device (PDO) objects on demand.  
    //   - The IConfigStateMonitor interface is used by user mode apps such as
    //       Resource Manager, to detect changes in the configuration state
    status = Bus_EnableIUpdateInterface(device);

    if (!NT_SUCCESS(status)) {
       PERR("Error creating EnableIUpdateInterface status =  0x%x\n", status);
       return status;
    }

    status = Bus_EnableIConfigStateMonitor(device);

    if (!NT_SUCCESS(status)) {
       PERR("Error creating EnableIUpdateInterface status =  0x%x\n", status);
       return status;
    }

    //
    // This value is used in responding to the IRP_MN_QUERY_BUS_INFORMATION
    // for the child devices. This is an optional information provided to
    // uniquely idenitfy the bus the device is connected.
    busInfo.BusTypeGuid = GUID_DEVCLASS_AAL_DEVICE;
    busInfo.LegacyBusType = PNPBus;
    busInfo.BusNumber = 0;

    WdfDeviceSetBusInformationForChildren(device, &busInfo);

    //
    // Register with WMI for management interface
    status = Bus_WmiRegistration(device);
    if (!NT_SUCCESS(status)) {
       PERR("Error registering with WMI.  status =  0x%x\n", status);
       return status;
    }

    return status;
}

#pragma warning(push)
#pragma warning(disable:28118) // this callback will run at IRQL=PASSIVE_LEVEL
_Use_decl_annotations_
VOID
Bus_EvtDeviceContextCleanup(WDFOBJECT Device)
{
   ULONG   count;
   PAAL_BUS_FDO_DEVICE_CONTEXT    pBusContext;
   PAGED_CODE();

   KdPrint(("Entered FilterEvtDeviceContextCleanup\n"));

   pBusContext = AALBusGetContext(Device);

   if (NULL != pBusContext->pMontorConfigAPIControlDevice)
   {

      WdfObjectDelete((WDFDEVICE)pBusContext->pMontorConfigAPIControlDevice);
      pBusContext->pMontorConfigAPIControlDevice = NULL;
   }

}
