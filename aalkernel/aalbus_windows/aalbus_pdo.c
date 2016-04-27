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
/// @aalbus_pdo.c.c
/// @brief Implements the constructor for the Physical Device Object (PDO)
///        associated with the AAL Virtual device being registered.
/// @ingroup System
/// @verbatim
/// Accelerator Abstraction Layer
///
/// AUTHORS: Joseph Grecco, Intel Corporation.
///
/// HISTORY: 
/// WHEN:          WHO:     WHAT:
/// 05/16/2014      JG       Initial version
//****************************************************************************
#include "aalsdk/kernel/kosal.h"
#include "aalsdk/kernel/aalbus_iupdate_config.h"

#include "aalbus_internal.h"

#include "aalsdk/kernel/aalbus-device.h"
#include "aalsdk/kernel/iaaldevice.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, Bus_CreatePdo)
#pragma alloc_text(PAGE, Bus_EvtDeviceListCreatePdo)
#endif
ULONG BusEnumDebugLevel;


// Extenal functions
//struct aal_device * aaldev_create_device(struct device_attributes *);
extern int aaldev_destroy_device(struct aal_device *);

// Declare the Context Accessor
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME( AAL_PDO_DEVICE_CONTEXT, WdfObjectGetAALContext );


//=============================================================================
// Name: Bus_EvtChildListIdentificationDescriptionDuplicate
// Description:  Called by framework when a copy of a ID Decriptor needs to be
//               made. This happens when a request is made to create a new 
//               child device by calling 
//               WdfChildListAddOrUpdateChildDescriptionAsPresent.
// Interface: public
// Inputs: DeviceList - Devcie list
//         SrcIDDesc - Source ID
//         DestIDDesc - Dest ID
// Outputs: NTSTATUS
// Comments: Callback is invoked with an internal lock held.  So do not call 
//           out to any WDF function which will require this lock
//           (basically any other WDFCHILDLIST api)
//=============================================================================
NTSTATUS 
   Bus_EvtChildListIdentificationDescriptionDuplicate( WDFCHILDLIST DeviceList,
                                                       PWDF_CHILD_IDENTIFICATION_DESCRIPTION_HEADER SrcIDDesc,
                                                       PWDF_CHILD_IDENTIFICATION_DESCRIPTION_HEADER DestIDDesc)
{
   PAAL_DEVICE_PDO_IDENTIFICATION_DESCRIPTION src, dst;
   size_t safeMultResult;
   NTSTATUS status = STATUS_SUCCESS;

   UNREFERENCED_PARAMETER(DeviceList);

   src = CONTAINING_RECORD( SrcIDDesc,
                            AAL_DEVICE_PDO_IDENTIFICATION_DESCRIPTION,
                            Header);
   dst = CONTAINING_RECORD( DestIDDesc,
                            AAL_DEVICE_PDO_IDENTIFICATION_DESCRIPTION,
                            Header);

   // Copy the manifest which contains a variable length portion
   dst->pAttributes = kosal_kmalloc(src->pAttributes->size);
   if (NULL == dst->pAttributes){
      return STATUS_INSUFFICIENT_RESOURCES;
   }

   RtlCopyMemory(dst->pAttributes, src->pAttributes, src->pAttributes->size);

   return status;
}


//=============================================================================
// Name: Bus_EvtChildListIdentificationDescriptionCompare
// Description:  Called when 2 IDs need to be compared
// Interface: public
// Inputs: DeviceList - Devcie list
//         SrcIDDesc - Source ID
//         DestIDDesc - Dest ID
// Outputs: TRUE if match
// Comments: Callback is invoked with an internal lock held.  So do not call 
//           out to any WDF function which will require this lock
//           (basically any other WDFCHILDLIST api)
//=============================================================================
BOOLEAN
Bus_EvtChildListIdentificationDescriptionCompare( WDFCHILDLIST DeviceList,
                              PWDF_CHILD_IDENTIFICATION_DESCRIPTION_HEADER SrcIDDesc,
                              PWDF_CHILD_IDENTIFICATION_DESCRIPTION_HEADER DestIDDesc)
{
    PAAL_DEVICE_PDO_IDENTIFICATION_DESCRIPTION lhs, rhs;
    SIZE_T result;
    SIZE_T temp = sizeof(struct aal_device_id);

    UNREFERENCED_PARAMETER(DeviceList);
    PTRACEIN;

    lhs = CONTAINING_RECORD(SrcIDDesc,
                            AAL_DEVICE_PDO_IDENTIFICATION_DESCRIPTION,
                            Header);
    rhs = CONTAINING_RECORD(DestIDDesc,
                            AAL_DEVICE_PDO_IDENTIFICATION_DESCRIPTION,
                            Header);

    // Use the devcie ID as the unique identifier
    result = RtlCompareMemory(&(lhs->pAttributes->device_id),
                              &(rhs->pAttributes->device_id),
                              sizeof(struct aal_device_id));

    if (result == sizeof(struct aal_device_id)){
       return TRUE;
    }
    
    return FALSE;
}

#pragma prefast(push)
#pragma prefast(disable:6101, "No need to assign IdentificationDescription")

//=============================================================================
// Name: Bus_EvtChildListIdentificationDescriptionCleanup
// Description:  called when an ID needs to be deleted
// Interface: public
// Inputs: DeviceList - Device list
//         SrcIDDesc - Source ID
//         DestIDDesc - Dest ID
// Outputs: NTSTATUS
// Comments: Callback is invoked with an internal lock held.  So do not call 
//           out to any WDF function which will require this lock
//           (basically any other WDFCHILDLIST api)
//=============================================================================
VOID
Bus_EvtChildListIdentificationDescriptionCleanup( _In_ WDFCHILDLIST DeviceList,
                                                  _Out_ PWDF_CHILD_IDENTIFICATION_DESCRIPTION_HEADER IDDesc)
{
   PAAL_DEVICE_PDO_IDENTIFICATION_DESCRIPTION pDesc;

   UNREFERENCED_PARAMETER(DeviceList);
   PTRACEIN;
   pDesc = CONTAINING_RECORD( IDDesc,
                              AAL_DEVICE_PDO_IDENTIFICATION_DESCRIPTION,
                              Header);

   if (NULL != pDesc->pAttributes){
      kosal_kfree(pDesc->pAttributes, pDesc->pAttributes->size);
      pDesc->pAttributes = NULL;
   }
	return FALSE;
   
}

#pragma prefast(pop) // disable:6101
//=============================================================================
// Name: Bus_EvtDeviceListCreatePdo
// Description:  called when an new device is added to the enumeration
// Interface: public
// Inputs: DeviceList - Devcie list
//         pDesc - Define identifcation record
//         ChildInit - opaque structure used in collecting device settings
//                     and passed in as a parameter to CreateDevice.
// Outputs: NTSTATUS
// Comments: Callback is invoked with an internal lock held.  So do not call 
//           out to any WDF function which will require this lock
//           (basically any other WDFCHILDLIST api)
//=============================================================================
NTSTATUS
Bus_EvtDeviceListCreatePdo( WDFCHILDLIST DeviceList,
                            PWDF_CHILD_IDENTIFICATION_DESCRIPTION_HEADER pDesc,
                            PWDFDEVICE_INIT ChildInit)
{
    PAAL_DEVICE_PDO_IDENTIFICATION_DESCRIPTION pAALID;

    PAGED_CODE();
    
    KdPrint(("Entered Bus_EvtDeviceListCreatePdo\n"));

    pAALID = CONTAINING_RECORD( pDesc,
                                AAL_DEVICE_PDO_IDENTIFICATION_DESCRIPTION,
                                Header);

    return Bus_CreatePdo( WdfChildListGetDevice(DeviceList),
                          ChildInit,
                          pAALID);

}
//=============================================================================
// Name: Bus_EvtDestroyPdo
// Description:  Called when the device is destroyed
// Interface: public
// Inputs: Device - Child devcic
// Outputs: NTSTATUS
// Comments: 
//=============================================================================
_Use_decl_annotations_
NTSTATUS
Bus_EvtDestroyPdo(_In_ WDFDEVICE       Device)
{
   // Destroy the internal Device structure
   aaldev_destroy_device(WdfObjectGetAALDevice(Device));
   return STATUS_SUCCESS;
}



//=============================================================================
// Name: Bus_CreatePdo
// Description:  Create the Physical Device Object
// Interface: public
// Inputs: Device - Child device
//         DeviceInit 
//         pAALID 
// Outputs: NTSTATUS
// Comments: 
//=============================================================================
NTSTATUS
Bus_CreatePdo( _In_ WDFDEVICE       Device,
               _In_ PWDFDEVICE_INIT DeviceInit,
               _In_ PAAL_DEVICE_PDO_IDENTIFICATION_DESCRIPTION pAALID)
{
    NTSTATUS                    status = STATUS_SUCCESS;
//    PPDO_DEVICE_DATA            pdoData = NULL;
    WDFDEVICE                   hChild = NULL;
    WDF_QUERY_INTERFACE_CONFIG  qiConfig;
    WDF_OBJECT_ATTRIBUTES       pdoAttributes;
    WDF_DEVICE_PNP_CAPABILITIES pnpCaps;
    WDF_DEVICE_POWER_CAPABILITIES powerCaps;
    UNICODE_STRING               deviceId;
    char                         *cbTemp;
    unsigned                     cbSize;


    DECLARE_CONST_UNICODE_STRING(deviceLocation, L"AAL Bus  0");
    DECLARE_UNICODE_STRING_SIZE(buffer, 80);
    ANSI_STRING               asTemp;
    PAGED_CODE();

    UNREFERENCED_PARAMETER(Device);

    PTRACEIN;

    //
    // Set DeviceType
    //
    WdfDeviceInitSetDeviceType(DeviceInit, FILE_DEVICE_BUS_EXTENDER);

    //
    // Provide DeviceID, HardwareIDs, CompatibleIDs and InstanceId
    //

    // Construct device ID. Will be of the form {0C008A76-C352-459B-8D45-C05FC7DF0563}\SPL2
    //  First create an ASCIIZ string. Alloc temp space + 1 for "\" and 1 for NULL 
    cbSize = strlen(pAALID->pAttributes->basename) + strlen(pAALID->pAttributes->device_id.m_deviguid) + 2;
    cbTemp = kosal_kmalloc(cbSize);

    strcpy(cbTemp, pAALID->pAttributes->device_id.m_deviguid);
    strcat(cbTemp, "\\");
    strcat(cbTemp, pAALID->pAttributes->basename);

    // Now convert the ASCIIZ to UNICODE by converting to ANSI String (counted char string)
    //  then convery ANSI String to UNICODE
    RtlZeroMemory(&asTemp, sizeof(asTemp));
    RtlInitAnsiString(&asTemp, cbTemp);

    RtlZeroMemory(&deviceId, sizeof(deviceId));

    // Create the UNICODE string. The TRUE means allocate buffer.
    status =  RtlAnsiStringToUnicodeString( &deviceId,
                                            &asTemp,
                                             TRUE);
    // Free the intermediate buffer
    kosal_kfree(cbTemp, cbSize);

    // Assign as device ID to be used when searching INF files.
    status = WdfPdoInitAssignDeviceID(DeviceInit, &deviceId);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    //
    // NOTE: same string  is used to initialize hardware id too
    status = WdfPdoInitAddHardwareID(DeviceInit, &deviceId);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    status = WdfPdoInitAddCompatibleID(DeviceInit, &deviceId);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    // Free the UNICODE string
    RtlFreeUnicodeString(&deviceId);

   // Create a unique instance ID out of the AAL Address.
    status = RtlUnicodeStringPrintf(&buffer, L"AAL_ADDR:%02d:%02d:%02d:%02d", pAALID->pAttributes->device_id.m_devaddr.m_busnum,
                                                                              pAALID->pAttributes->device_id.m_devaddr.m_bustype,
                                                                              pAALID->pAttributes->device_id.m_devaddr.m_devicenum,
                                                                              pAALID->pAttributes->device_id.m_devaddr.m_subdevnum);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    status = WdfPdoInitAssignInstanceID(DeviceInit, &buffer);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    //
    // Provide a description about the device. This text is usually read from
    // the device. In the case of USB device, this text comes from the string
    // descriptor. This text is displayed momentarily by the PnP manager while
    // it's looking for a matching INF. If it finds one, it uses the Device
    // Description from the INF file or the friendly name created by
    // coinstallers to display in the device manager. FriendlyName takes
    // precedence over the DeviceDesc from the INF file.
    //
    status = RtlUnicodeStringPrintf( &buffer,
                                     L"AAL Device");
    if (!NT_SUCCESS(status)) {
        return status;
    }

    //
    // You can call WdfPdoInitAddDeviceText multiple times, adding device
    // text for multiple locales. When the system displays the text, it
    // chooses the text that matches the current locale, if available.
    // Otherwise it will use the string for the default locale.
    // The driver can specify the driver's default locale by calling
    // WdfPdoInitSetDefaultLocale.
    //
    status = WdfPdoInitAddDeviceText(  DeviceInit,
                                      &buffer,
                                      &deviceLocation,
                                       0x409 );
    if (!NT_SUCCESS(status)) {
        return status;
    }

    WdfPdoInitSetDefaultLocale(DeviceInit, 0x409);

    //
    // Initialize the attributes to specify the size of PDO device extension.
    // All the state information private to the PDO will be tracked here.
    //
    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&pdoAttributes, AAL_PDO_DEVICE_CONTEXT);
    pdoAttributes.EvtDestroyCallback = Bus_EvtDestroyPdo;
    // Create the PDO
    status = WdfDeviceCreate(&DeviceInit, &pdoAttributes, &hChild);
    if (!NT_SUCCESS(status)) {
        return status;
    }
    
    // Save the attributes in the context
    WdfObjectGetAALContext( hChild )->m_pdevAttributes = pAALID->pAttributes;
#if 0 // TODO fix
    // Set the AAL Device object up.
    WdfObjectGetAALDevice(hChild) = aaldev_create_device(pAALID->pAttributes);
#endif
    //
    // Set some properties for the child device.
    //
    WDF_DEVICE_PNP_CAPABILITIES_INIT(&pnpCaps);
    pnpCaps.Removable         = WdfTrue;
    pnpCaps.EjectSupported    = WdfTrue;
    pnpCaps.SurpriseRemovalOK = WdfTrue;

//    pnpCaps.Address  = SerialNo;
//    pnpCaps.UINumber = SerialNo;

    WdfDeviceSetPnpCapabilities(hChild, &pnpCaps);

    WDF_DEVICE_POWER_CAPABILITIES_INIT(&powerCaps);

    powerCaps.DeviceD1 = WdfTrue;
    powerCaps.WakeFromD1 = WdfTrue;
    powerCaps.DeviceWake = PowerDeviceD1;

    powerCaps.DeviceState[PowerSystemWorking]   = PowerDeviceD1;
    powerCaps.DeviceState[PowerSystemSleeping1] = PowerDeviceD1;
    powerCaps.DeviceState[PowerSystemSleeping2] = PowerDeviceD2;
    powerCaps.DeviceState[PowerSystemSleeping3] = PowerDeviceD2;
    powerCaps.DeviceState[PowerSystemHibernate] = PowerDeviceD3;
    powerCaps.DeviceState[PowerSystemShutdown]  = PowerDeviceD3;

    WdfDeviceSetPowerCapabilities(hChild, &powerCaps);


    //
    // Initialize the interface for drivers.  Drivers use the Query Interface
    //  model.
#if 0 
    RtlZeroMemory( &DeviceAccessorInterface, sizeof( DeviceAccessorInterface ) );

    DeviceAccessorInterface.InterfaceHeader.Size = sizeof( DeviceAccessorInterface );
    DeviceAccessorInterface.InterfaceHeader.Version = 1;
    DeviceAccessorInterface.InterfaceHeader.Context = hChild;

    //
    // Let the framework handle reference counting. The functions are stubs.
    DeviceAccessorInterface.InterfaceHeader.InterfaceReference = WdfDeviceInterfaceReferenceNoOp;
    DeviceAccessorInterface.InterfaceHeader.InterfaceDereference = WdfDeviceInterfaceDereferenceNoOp;

  
    DeviceAccessorInterface.getExtendedAttributes = aalbus_getExtendedAttributes;
    DeviceAccessorInterface.getPublicAttributes = aalbus_getPublicAttributes;
    DeviceAccessorInterface.sendDeviceUpdate = aalbus_sendDeviceUpdate;

    WDF_QUERY_INTERFACE_CONFIG_INIT( &qiConfig,
                                     (PINTERFACE)&DeviceAccessorInterface,
                                     &GUID_AAL_DEVICE_INTERFACE_STANDARD,
                                     NULL );
#endif
    RtlZeroMemory( &aaldev_interface( WdfObjectGetAALDevice( hChild ) ), sizeof( AAL_DEVICE_INTERFACE ) );

    aaldev_interface( WdfObjectGetAALDevice( hChild ) ).InterfaceHeader.Size = sizeof( AAL_DEVICE_INTERFACE );
    aaldev_interface( WdfObjectGetAALDevice( hChild ) ).InterfaceHeader.Version = 1;
    aaldev_interface( WdfObjectGetAALDevice( hChild ) ).InterfaceHeader.Context = hChild;
    //
    // Let the framework handle reference counting. The functions are stubs.
    aaldev_interface( WdfObjectGetAALDevice( hChild ) ).InterfaceHeader.InterfaceReference = WdfDeviceInterfaceReferenceNoOp;
    aaldev_interface( WdfObjectGetAALDevice( hChild ) ).InterfaceHeader.InterfaceDereference = WdfDeviceInterfaceDereferenceNoOp;

    aaldev_init( WdfObjectGetAALDevice( hChild ) );

    WDF_QUERY_INTERFACE_CONFIG_INIT( &qiConfig,
                                     (PINTERFACE) &aaldev_interface( WdfObjectGetAALDevice( hChild ) ),
                                     &GUID_AAL_DEVICE_QUERY_INTERFACE,
                                     NULL );


    //
    // Add the interface so that other hChild can get to it.
    status = WdfDeviceAddQueryInterface( hChild, &qiConfig );

    if( !NT_SUCCESS( status ) ) {
       return FALSE;
    }



    return status;
}


BOOLEAN
Bus_IsSafetyLockEnabled(
    IN   WDFDEVICE ChildDevice
    )
/*++

Routine Description:

    Routine to check whether safety lock is enabled

Arguments:

    Context        pointer to  PDO device extension

Return Value:

    TRUE or FALSE

--*/
{
    UNREFERENCED_PARAMETER(ChildDevice);

    KdPrint(("IsSafetyLockEnabled\n"));

    return TRUE;
}

