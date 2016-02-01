//****************************************************************************
//                               INTEL CONFIDENTIAL
//
//        Copyright (c) 2014-2016 Intel Corporation All Rights Reserved.
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
/// @file cci_pcie_main_windows.c
/// @brief Implements the kernel driver shell for CCI-P PCIe devices.
/// @ingroup DeviceDrivers
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHOR: Joseph Grecco, Intel Corporation
///
/// HISTORY: Initial version based on Microsoft Sample Code
/// WHEN:          WHO:     WHAT:
/// 01/27/2016     JG       Initial Version Created
//****************************************************************************
#include "aalsdk/kernel/kosal.h"

#include <Initguid.h>               // This header must be included here so that the 
                                    //  guids in IUpdateConfig are implemented
                                    //  Only include this once in this module.
#include <wdmguid.h>                //  Used for GUID_BUS_INTERFACE_STANDARD

#include "aalsdk/kernel/aalbus_iupdate_config.h"
#include <devguid.h>

#define MODULE_FLAGS CCIPCIE_DBG_MOD

#include "cci_pcie_windows.h"


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
/* Module selection flags */
   | CCIPCIE_DBG_MOD
   | CCIPCIE_DBG_DEV
   | CCIPCIE_DBG_AFU
   | CCIPCIE_DBG_MAFU
   | CCIPCIE_DBG_MMAP
   | CCIPCIE_DBG_CMD
   | CCIPCIE_DBG_CFG
#if 0
// TODO: temporary
   | AALBUS_DBG_MOD
   | AALBUS_DBG_FILE
   | AALBUS_DBG_MMAP
   | AALBUS_DBG_IOCTL
   | UIDRV_DBG_MOD
   | UIDRV_DBG_FILE
   | UIDRV_DBG_MMAP
   | UIDRV_DBG_IOCTL
#endif
;

//=============================================================
// DriverEntry is discardable and the other entry points can be 
//  placed in pagable memory
EVT_WDF_DEVICE_FILE_CREATE  CCIPDrvDeviceFileCreate;
EVT_WDF_FILE_CLEANUP CCIPDrvDeviceFileCleanup;
EVT_WDF_FILE_CLOSE CCIPDrvDeviceFileClose;

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
//#pragma alloc_text (PAGE, CCIPDrvEvtDeviceAdd)
//#pragma alloc_text (PAGE, CCIPDrvEvtIoRead)
//#pragma alloc_text (PAGE, CCIPDrvEvtIoWrite)
//#pragma alloc_text (PAGE, CCIPDrvEvtIoDeviceControl)
//#pragma alloc_text (PAGE, CCIPDrvEvtDevicePrepareHardware)
//#pragma alloc_text (PAGE, CCIPDrvEvtDeviceReleaseHardware)
//#pragma alloc_text (PAGE, CCIPDrvMapHWResources)
//#pragma alloc_text (PAGE, CCIPDrvUnMapHWResources)
//#pragma alloc_text (PAGE, CCIPDrvGetDeviceInformation)
#endif

//=============================================================================
// Name: DriverEntry
// Description:     DriverEntry initializes the driver and is the first routine
//     called by the system after the driver is loaded. DriverEntry configures 
//     and creates a WDF driver object.
// Interface: public
// Inputs: DriverObject - Instance of this function driver. Remains in scope 
//                        until the driver is unloaded.
//         RegistryPath - Driver specific path in Registry. Can be used by 
//                        driver to store persistent (i.e., survives reboots) 
//                        data.
// Outputs: STATUS_SUCCESS if successful,
//          STATUS_UNSUCCESSFUL if failure.
// Comments:
//=============================================================================
NTSTATUS
DriverEntry(IN PDRIVER_OBJECT  DriverObject,
			   IN PUNICODE_STRING RegistryPath)
{
   NTSTATUS          status = STATUS_DEVICE_CONFIGURATION_ERROR;
   WDF_DRIVER_CONFIG config;

   PTRACEIN;

   // Sign on message
   DbgPrint("Intel(R) QuickAssist Technology Accelerator Abstraction Layer\n");
   DbgPrint("Intel(R) AAL CCI-P PCIe Driver\n");
   DbgPrint("Copyright (c) 2012-2016 Intel Corporation\n");
   DbgPrint("Built %s %s\n", __DATE__, __TIME__);


   //==================================================================================================================================
   // Create a framework driver object for the calling driver.
   //   A driver that uses Kernel-Mode Driver Framework must call WdfDriverCreate from within its  
   //   DriverEntry routine, before calling any other framework routines. 
   //   The driver must call WDF_DRIVER_CONFIG_INIT to initialize its WDF_DRIVER_CONFIG structure.
   //   The framework driver object is the top of the driver's tree of framework objects and therefore does not have a parent object.
   //   If the driver provides EvtCleanupCallback or EvtDestroyCallback callback functions for the driver object,
   //   note that the framework calls these callback functions at IRQL = PASSIVE_LEVEL.
   //
   // Override the default attributes. WDF provides defaults.
   WDF_DRIVER_CONFIG_INIT( &config, CCIPDrvEvtDeviceAdd );

   status = WdfDriverCreate(DriverObject,
                            RegistryPath,
                            WDF_NO_OBJECT_ATTRIBUTES,	// No Cleanup or Destroy callbacks etc...
                            &config,					      // Driver Config Info
                            WDF_NO_HANDLE);			      // Currently not used but may be in the future to get a handle to this driver

   if ( !NT_SUCCESS(status) ) {
      PERR("WdfDriverCreate failed with status 0x%x\n", status);
   }

   PTRACEOUT_LINT(status);
   return status;
}

//=============================================================================
// Name: CCIPDrvEvtDeviceAdd
// Description:  Responsible for creating functional device objects (FDO) or 
//               filter device objects (filter DO) for devices enumerated by 
//               the Plug and Play (PnP) manager.
// Interface: public
// Inputs: Driver - This is the driver's driver object. 
//         DeviceInit - Caller-supplied pointer to a DEVICE_OBJECT structure 
//                      representing a physical device object (PDO) created by 
//                      a lower-level driver.
// Outputs: STATUS_SUCCESS if successful,
//          STATUS_UNSUCCESSFUL if failure.
// Comments: Creates and initializes a WDF device object to
//           represent a new instance of CCIPDrv device. (Should be a singleton?)
//=============================================================================
NTSTATUS
CCIPDrvEvtDeviceAdd( IN WDFDRIVER       Driver,
			  	         IN PWDFDEVICE_INIT DeviceInit)
{
   UNREFERENCED_PARAMETER( Driver );
   UNREFERENCED_PARAMETER( DeviceInit );
   NTSTATUS                        status = STATUS_SUCCESS;

   PCCI_DEVICE                     pWinccidev;
//   WDF_IO_QUEUE_CONFIG             queueConfig;
   WDF_OBJECT_ATTRIBUTES           fdoAttributes;
   WDFDEVICE                       hDevice;
//   WDFQUEUE                        queue;
   WDF_PNPPOWER_EVENT_CALLBACKS    pnpPowerCallbacks;
   WDF_FILEOBJECT_CONFIG           fileObjectConfig;

//   UPDATECONFIG_INTERFACE_STANDARD interface;

//   struct spl2_device             *pspl2dev = NULL;

   // Used to remove compiler warnings
   UNREFERENCED_PARAMETER(Driver);

   PTRACEIN;
   // Make sure that we are running at a pagable IRQ level (checked code only)
   PAGED_CODE();

   // Set to directIO (NOT sure if I need this)
   WdfDeviceInitSetIoType(DeviceInit, WdfDeviceIoDirect);
   
   // Set the device type. Must match the type in IOCTL
   WdfDeviceInitSetDeviceType(DeviceInit, FILE_DEVICE_UNKNOWN);

   // Set the pnpPowerCallbacks. There are a slew of these
   //   but we are at least interested in the ones that will
   //   allow us to initialize and disable the HW
   WDF_PNPPOWER_EVENT_CALLBACKS_INIT(&pnpPowerCallbacks);

   // The EvtDevicePrepareHardware callback function accesses the device's raw and translated hardware resources by using the ResourcesRaw and 
   // ResourcesTranslated handles that it receives. The callback function can call WdfCmResourceListGetCount and WdfCmResourceListGetDescriptor 
   // to traverse the resource lists. This callback function cannot modify the resource lists.
   // Typically, the driver's EvtDevicePrepareHardware callback function does the following, if necessary:
   //   - Maps physical memory addresses to virtual addresses so the driver can access memory that is assigned to the device
   //   - Determines the device's revision number
   //   - Configures USB devices
   //   - Obtains driver-defined interfaces from other drivers
   pnpPowerCallbacks.EvtDevicePrepareHardware = CCIPDrvEvtDevicePrepareHardware;
   pnpPowerCallbacks.EvtDeviceReleaseHardware = CCIPDrvEvtDeviceReleaseHardware;

   //
   // These two callbacks set up and tear down hardware state that must be
   // done every time the device moves in and out of the D0-working state.
   pnpPowerCallbacks.EvtDeviceD0Entry = CCIPDrvEvtDeviceD0Entry;
   pnpPowerCallbacks.EvtDeviceD0Exit  = CCIPDrvEvtDeviceD0Exit;

   //
   // Register the PnP and power callbacks. Power policy related callbacks will be registered
   // later.
   WdfDeviceInitSetPnpPowerEventCallbacks(DeviceInit, &pnpPowerCallbacks);

   //
   // Register File object callbacks
   //
   WDF_FILEOBJECT_CONFIG_INIT(&fileObjectConfig,
                               CCIPDrvDeviceFileCreate,
                               CCIPDrvDeviceFileClose,
                               CCIPDrvDeviceFileCleanup
                               );


   WdfDeviceInitSetFileObjectConfig( DeviceInit,
                                     &fileObjectConfig,
                                     WDF_NO_OBJECT_ATTRIBUTES);


   //
   // Initialize attributes and a context area for the device object.
   WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE( &fdoAttributes, CCI_DEVICE );


   // Create a framework device object. This call will in-turn create
   // a WDM device object, attach it to the lower stack, and set the
   // appropriate flags and attributes.
   status = WdfDeviceCreate(&DeviceInit, &fdoAttributes, &hDevice);
   if ( !NT_SUCCESS(status) ) {
      PERR("WdfDeviceCreate failed with status code 0x%x\n", status);
      PTRACEOUT_LINT(status);
      return status;
   }


   //
   // Get the device context by using the accessor function specified in
   // the WDF_DECLARE_CONTEXT_TYPE_WITH_NAME macro for CCIPDrv_DEVICE_DATA (WinCCIPDrv.h).
   pWinccidev = CCIFdoGetCCIDev(hDevice);

   // Save the FDO Device handle
   PWIN_CCIP_DEVICE_TO_WDF_DEVICE( pWinccidev ) = hDevice;
 
   //
   // Get the BUS_INTERFACE_STANDARD for our device so that we can
   // read & write to PCI config space.
   status = WdfFdoQueryForInterface( PWIN_CCIP_DEVICE_TO_WDF_DEVICE( pWinccidev ),
                                     &GUID_BUS_INTERFACE_STANDARD,
                                     (PINTERFACE)&PWIN_CCIP_DEVICE_BUS( pWinccidev ),
                                     sizeof(BUS_INTERFACE_STANDARD),
                                     1, // Version
                                     NULL); //InterfaceSpecificData
   pWinccidev->m_softdevice = 0; // Assume hardware
   if ( !NT_SUCCESS(status) ) {
      PERR("Failed to get the BUS_INTERFACE_STANDARD. Assuming soft device\n");
      PTRACEOUT_LINT(status);
      pWinccidev->m_softdevice = true;    
   }else{
#if 0 
      // Save the OS specific PCI device object in the OS independent CCIPDrv 
      //  device object so it can be used in generic kosal calls.
      pspl2dev = PWIN_CCIPDrv_DEVICE_TO_PCCIPDrv_DEVICE(pWinccidev);
      spl2_dev_pci_dev(pspl2dev) = & PWIN_CCIPDrv_DEVICE_BUS(pWinccidev);
     
      // Get the board info and determine if it is supported by this driver
      status = CCIPDrvGetDeviceInformation(pWinccidev);
      if ( !NT_SUCCESS(status) ) {
         PERR("Board not supported by this driver\n");
         PTRACEOUT_LINT(status);
         return status;
      }
#endif
   }
#if 0 // Enable UIDDRV interface
   //
   // Tell the Framework that this device will need an interface
   status = WdfDeviceCreateDeviceInterface( hDevice,
                                            (LPGUID) &GUID_DEVINTERFACE_CCIPDrv,
                                            NULL); // ReferenceString
   if ( !NT_SUCCESS(status) ) {
      PERR("WdfDeviceCreateDeviceInterface failed\n");
      PTRACEOUT_LINT(status);
      return status;
   }

   //
   // Register I/O callbacks to tell the framework that you are interested
   // in handling IRP_MJ_READ, IRP_MJ_WRITE, and IRP_MJ_DEVICE_CONTROL requests.
   // If a specific callback function is not specified for one of these,
   // the request will be dispatched to the EvtIoDefault handler, if any.
   // If there is no EvtIoDefault handler, the request will be failed with
   // STATUS_INVALID_DEVICE_REQUEST.
   // WdfIoQueueDispatchParallel means that we are capable of handling
   // all the I/O requests simultaneously and we are responsible for protecting
   // data that could be accessed by these callbacks simultaneously.
   // A default queue gets all the requests that are 
   // configured for forwarding using WdfDeviceConfigureRequestDispatching.
   //
   WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&queueConfig,  WdfIoQueueDispatchParallel);

   //    queueConfig.EvtIoRead = CCIPDrvEvtIoRead;
   //    queueConfig.EvtIoWrite = CCIPDrvEvtIoWrite;
   queueConfig.EvtIoDeviceControl = CCIPDrvEvtIoDeviceControl;
   
   // WdfDeviceInitSetIoInCallerContextCallback,  MAY NEED THIS FOR MAPPING
   //  IF we cannot guarantee the calling process context im DevIoControl.
   // NOTE: The top end driver (e.g., UIDRV or equiv will need to trap
   //  IOCTL_AAL_MAP calls (so that it can safetly map to user space)
   // May need to do a multistep process like in Linux but in this case
   // 1) User calls Allocate WS. Kernel PIP sets up MDL but does not map returns pmdl and size
   ///   probably wrapped ina WSID
   // 2) User takes WSID and issues a MAP which is handled by TOP driver (IDRV?) which does the
   //    actual map and returns the address.


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
   //
   __analysis_assume(queueConfig.EvtIoStop != 0);
   status = WdfIoQueueCreate(  hDevice,
                              &queueConfig,
                               WDF_NO_OBJECT_ATTRIBUTES, 
                              &queue);
   __analysis_assume(queueConfig.EvtIoStop == 0);



   //
   // We will provide an example on how to get a bus-specific direct
   // call interface from a bus driver.
   //
   {
      WDFIOTARGET IoTarget;
      WDF_IO_TARGET_OPEN_PARAMS  openParams;
      DECLARE_CONST_UNICODE_STRING( ntDeviceName, AALBUS_UPDATECONFIG_SERVICE_NAME_STRING );

      status = WdfIoTargetCreate(  hDevice,
                                   WDF_NO_OBJECT_ATTRIBUTES,
                                  &IoTarget);
      if (!NT_SUCCESS(status)) {
         PERR("WdfIoTargetCreate failed\n");
         PTRACEOUT_LINT(status);
         return status;
      }
      
      WDF_IO_TARGET_OPEN_PARAMS_INIT_CREATE_BY_NAME(&openParams, &ntDeviceName, STANDARD_RIGHTS_ALL);
      
      status = WdfIoTargetOpen(  IoTarget,
                                &openParams);
      if (!NT_SUCCESS(status)) {
         PERR("WdfFdoQueryForInterface failed\n");
         PTRACEOUT_LINT(status);
         return status;
      }

      status = WdfIoTargetQueryForInterface(  IoTarget,
                                             &GUID_AALBUS_UPDATECONFIG_QUERY_INTERFACE,
                                              (PINTERFACE)&interface,
                                              sizeof(UPDATECONFIG_INTERFACE_STANDARD),
                                              1,
                                              NULL);// InterfaceSpecific Data

      if (!NT_SUCCESS(status)) {
         PERR("WdfFdoQueryForInterface failed\n");
         PTRACEOUT_LINT(status);
         return status;
      }
   }
   
   {
#if 1    
      char *manifest_args = "Hello SPL2";
      size_t size;

      struct device_attributes *manifest = aalbus_create_device_attrib( manifest_args, 
                                                                                     (strlen( manifest_args ) + 1),
                                                                                     NULL, 
                                                                                     0 );
      struct aal_device_id          device_id;
      char * temp = NULL;

      // First clear the whole struct
      memset(&device_id, 0, sizeof(struct aal_device_id));

      // Invalidate the address
      memset(&(device_id.m_devaddr), -1, sizeof(struct aal_device_addr));

      device_id.m_devaddr.m_bustype = aal_bustype_Prop;
      device_id.m_devaddr.m_busnum = 1;
      device_id.m_devaddr.m_devicenum = 2;
      device_id.m_devaddr.m_subdevnum = 1;   

      device_id.m_devicetype = aal_devtypeAHM;

      manifest->device_id = device_id;

      size = strlen("SPL2");
      manifest_args = "SPL2";
      temp = manifest->basename;
      RtlStringCchCopyA(manifest->basename, strlen("SPL2")+1, "SPL2");
      //kosal_guidStringtoChar16(manifest->device_id.m_deviguid, "{0C008A76-C352-459B-8D45-C05FC7DF0563}");
      
      RtlStringCchCopyA(manifest->device_id.m_deviguid, strlen("{0C008A76-C352-459B-8D45-C05FC7DF0563}")+1, "{0C008A76-C352-459B-8D45-C05FC7DF0563}");

      status = interface.AddDevice(interface.InterfaceHeader.Context, manifest);
      aalbus_destroy_device_attrib( manifest);
      return STATUS_SUCCESS;
#endif
   }

#endif
   PTRACEOUT_LINT(status);

   return status;


}

//=============================================================================
// Name: CCIPDrvGetDeviceInformation
// Description:  Get the device information and determine if the board is 
//               supported.
// Interface: private
// Inputs:  pWinccidev - Driver specific data
// Outputs: STATUS_SUCCESS if successful,
//          STATUS_DEVICE_CONFIGURATION_ERROR if failure.
// Comments: Currently only used under debugger
//=============================================================================
NTSTATUS
CCIPDrvGetDeviceInformation( IN PCCI_DEVICE pWinccidev )
{
   UNREFERENCED_PARAMETER( pWinccidev );

   NTSTATUS            status   = STATUS_DEVICE_CONFIGURATION_ERROR;
#if 0

//   struct spl2_device *pspl2dev = NULL;

//   DECLSPEC_ALIGN(MEMORY_ALLOCATION_ALIGNMENT) DWORD32 viddid = 0;  

   PTRACEIN;
   PAGED_CODE();
   // DEBUG 
   return 0;
   // If no pWinccidev, then no spl2_device..
   ASSERT(NULL != pWinccidev);
   if ( NULL == pWinccidev ) {
      PTRACEOUT_LINT(status);
      return status;
   }
#if 0
   // Get the spl2_device object from the Windows specific device context
   //  spl2_device is portable and used by common code.
   pspl2dev = PWIN_CCIPDrv_DEVICE_TO_PCCIPDrv_DEVICE(pWinccidev);
   ASSERT(NULL != pspl2dev);
   if ( NULL == pspl2dev ) {
      PTRACEOUT_LINT(status);
      return status;
   }

   // Get the Vendor/Device IDs and determine if it is one of ours
   if ( !kosal_pci_read_config_dword(spl2_dev_pci_dev(pspl2dev), 
                                     0, 
                                     &viddid) ) {
      PTRACEOUT_LINT(status);
      return status;
   }

   if ( !spl2_viddid_is_supported(viddid & 0xffff, viddid >> 16) ) {
      PTRACEOUT_LINT(status);
      return status;
   }
#endif
   status = STATUS_SUCCESS;
   PTRACEOUT_LINT(status);
#endif
   return status;
}



//=============================================================================
// Name: CCIPDrvEvtDevicePrepareHardware
// Description:  Performs any operations that are needed to make a device 
//               accessible to the driver. The framework calls the driver's
//               EvtDeviceStart callback when the PnP manager sends an 
//               IRP_MN_START_DEVICE request to the driver stack.
// Interface: public
// Inputs: Device [in] -A handle to a framework device object.
//         ResourcesRaw [in] - A handle to a framework resource-list object 
//                             that identifies the raw hardware resources that 
//                             the Plug and Play manager has assigned to the 
//                             device.
//         ResourcesTranslated [in] - A handle to a framework resource-list 
//                                    object that identifies the translated 
//                                    hardware resources that the Plug and Play 
//                                    manager has assigned to the device.
//                                    The resources appear from the CPU's point
//                                    of view. Use this list of resources to 
//                                    map I/O space and device-accessible 
//                                    memory into virtual address space
// Outputs: STATUS_SUCCESS - success.
//=============================================================================
NTSTATUS
CCIPDrvEvtDevicePrepareHardware(WDFDEVICE    Device,
                             WDFCMRESLIST Resources,
                             WDFCMRESLIST ResourcesTranslated)
{
   UNREFERENCED_PARAMETER( Device );
   UNREFERENCED_PARAMETER( Resources );
   UNREFERENCED_PARAMETER( ResourcesTranslated );

   NTSTATUS         status  = STATUS_DEVICE_CONFIGURATION_ERROR;
#if 0 
   PWIN_CCIPDrv_DEVICE pWinccidev = NULL;

   UNREFERENCED_PARAMETER(Resources);

   PTRACEIN;
   PAGED_CODE();
   return 0; //DEBUG
   ASSERT(NULL != Device);
   if ( NULL == Device ) {
      PTRACEOUT_LINT(status);
      return status;
   }
#if 0 
   pWinccidev = CCIFdoGetCCIDev(Device);
   ASSERT(NULL != pWinccidev);
   if ( NULL == pWinccidev ) {
      PTRACEOUT_LINT(status);
      return status;
   }

   status = CCIPDrvMapHWResources(pWinccidev, ResourcesTranslated);
   if ( !NT_SUCCESS(status) ) {
      PERR("CCIPDrvMapHWResources() failed\n");
      PTRACEOUT_LINT(status);
      return status;
   }


   if(NULL != pWinccidev){
      check_qlp_feature_id(PWIN_CCIPDrv_DEVICE_TO_PCCIPDrv_DEVICE(pWinccidev));
   }
#endif

   PTRACEOUT_LINT(status);
#endif
   return status;
}

//=============================================================================
// Name: CCIPDrvMapHWResources
// Description:  Gets the HW resources assigned by the bus driver and maps them 
//               to system address space. 
// Interface: public
// Inputs: pWinccidev [in][out] - Pointer to the CCIPDrv Device object
//         ResourcesTranslated [in] - A handle to a framework resource-list 
//                                    object that identifies the translated 
//                                    hardware resources that the Plug and Play 
//                                    manager has assigned to the device.
//                                    The resources appear from the CPU's point
//                                    of view. Use this list of resources to 
//                                    map I/O space and device-accessible 
//                                    memory into virtual address space
// Outputs: STATUS_SUCCESS - success.
//=============================================================================
NTSTATUS
CCIPDrvMapHWResources( IN PCCI_DEVICE pWinccidev,
                   IN WDFCMRESLIST     ResourcesTranslated)
{
   UNREFERENCED_PARAMETER( pWinccidev );
   UNREFERENCED_PARAMETER( ResourcesTranslated );
   NTSTATUS                        status   = STATUS_DEVICE_CONFIGURATION_ERROR;
#if 0 
   PCM_PARTIAL_RESOURCE_DESCRIPTOR descriptor;
   ULONG                           i;

   struct spl2_device             *pspl2dev = NULL;
   PDEVICE_OBJECT                  pdo        = NULL;   
   UCHAR                           buffer[2048];
   ULONG                           buflen;
   DECLSPEC_ALIGN(MEMORY_ALLOCATION_ALIGNMENT) DWORD32 viddid = 0;  
   unsigned memcount=0;
   unsigned ishybrid=0;

   PTRACEIN;
   PAGED_CODE();
   return 0; //debug
   // If no pWinccidev, then no spl2_device..
   ASSERT(NULL != pWinccidev);
   if ( NULL == pWinccidev ) {
      PTRACEOUT_LINT(status);
      return status;
   }
#if 0 
   // Get the spl2_device object from the Windows specific device context
   //  spl2_device is portable and used by common code.
   pspl2dev = PWIN_CCIPDrv_DEVICE_TO_PCCIPDrv_DEVICE(pWinccidev);
   ASSERT(NULL != pspl2dev);
   if ( NULL == pspl2dev ) {
      PTRACEOUT_LINT(status);
      return status;
   }


   // Done if simulated
   if( pWinccidev->m_softdevice){
      spl2_dev_len_config(pspl2dev) = QPI_APERTURE_SIZE;
      return STATUS_SUCCESS;
   }

   //================================================
   // Determine if this is a BAR0 or BAR1 CCIPDrv device
   //================================================
   pdo      = WdfDeviceWdmGetPhysicalDevice(PWIN_CCIPDrv_DEVICE_TO_WDF_DEVICE(pWinccidev));
   status = IoGetDeviceProperty( pdo, 
                                 DevicePropertyHardwareID,
                                 sizeof(buffer),
                                 buffer,
                                 &buflen);


   if(!NT_SUCCESS(status)){
      PDEBUG("IoGetDeviceProperty Failed\n");
        buffer[0]=0;
   }else{
      buffer[buflen]=0;
   }

   PVERBOSE("Device ID = %ws\n",buffer);

   // Get the Vendor/Device IDs and determine if it is one of ours
   if ( !kosal_pci_read_config_dword(spl2_dev_pci_dev(pspl2dev), 
                                     0, 
                                     &viddid) ) {
      PTRACEOUT_LINT(status);
      return status;
   }

   ishybrid = (viddid >> 16) == 0x0bcd ? 1 : 0;
   //===================================================

   // Loop through all of the resources assigned to the device.
   //   We should only be looking for memory
   for ( i = 0 ; i < WdfCmResourceListGetCount(ResourcesTranslated) ; i++ ) {



      descriptor = WdfCmResourceListGetDescriptor(ResourcesTranslated, i);

      ASSERT(NULL != descriptor);
      if ( NULL == descriptor ) {
         continue;
      }

      switch ( descriptor->Type ) {

         case CmResourceTypePort : {
            // Not expected
         } break;

         case CmResourceTypeMemory : {
			// Count memory resources
			memcount++;

            // If this is not a hybrid device then CCIPDrv memory is BAR0 (1st detected). If it is a hybrid the
			//   CCIPDrv memory is BAR1 (2nd detected). Ignore any other memory resources.
			
            if( (!ishybrid && ( memcount == 1 ) ) || 
				            (ishybrid && (memcount == 2 )) ){

				// This should be our memory aperture which reflect BAR0
				PVERBOSE("Memory mapped CSR:(0x%x:0x%x) Length:(%u [0x%x])\n",
							descriptor->u.Memory.Start.LowPart,
							descriptor->u.Memory.Start.HighPart,
							descriptor->u.Memory.Length, descriptor->u.Memory.Length);
               
				// Grab the base address register. Only use low part as High is zero
				spl2_dev_phys_config(pspl2dev) = (btPhysAddr)ULongToPtr(descriptor->u.Memory.Start.LowPart);
				spl2_dev_len_config(pspl2dev)  = (btWSSize)descriptor->u.Memory.Length;

				// Remap the bar0 region into kernel space.
				spl2_dev_kvp_config(pspl2dev) = (btVirtAddr)MmMapIoSpace(descriptor->u.Memory.Start,
																		 (SIZE_T)spl2_dev_len_config(pspl2dev),
																		 MmNonCached);
				//PVERBOSE("CSRAddress=%p\n", pWinccidev->CSRAddress);
				ASSERT(NULL != spl2_dev_kvp_config(pspl2dev));
				if ( NULL == spl2_dev_kvp_config(pspl2dev) ) {
				   PTRACEOUT_LINT(status);
				   return status;
				}
			} // end if(hybrid...
         } break;

         case CmResourceTypeInterrupt : {
            // Currently not using interrupts but board does report the resource

         } break;

         default : {
            //
            // This could be device-private type added by the PCI bus driver. We
            // shouldn't filter this or change the information contained in it.
            //
            PVERBOSE("Unhandled resource type (0x%x)\n", descriptor->Type);
         } break;
      }
   }

    // Did we get a memory map?
   ASSERT(NULL != spl2_dev_kvp_config(pspl2dev));
   if ( NULL == spl2_dev_kvp_config(pspl2dev) ) {
      PTRACEOUT_LINT(status);
      return status;
   }
#endif
   status = STATUS_SUCCESS;
   PTRACEOUT_LINT(status);
#endif
   return status;
}


//=============================================================================
// Name: CCIPDrvUnmapHWResources
// Description:  Unmaps HW resources
// Interface: public
// Inputs: pWinccidev [in] - Pointer to the CCIPDrv Device object
// Outputs: STATUS_SUCCESS - success.
//=============================================================================
NTSTATUS
CCIPDrvUnmapHWResources( IN PCCI_DEVICE pWinccidev )
{
   NTSTATUS            status   = STATUS_DEVICE_CONFIGURATION_ERROR;
   UNREFERENCED_PARAMETER( pWinccidev );
#if 0
   struct spl2_device *pspl2dev = NULL;

   PTRACEIN;
   PAGED_CODE();
   
   // If no pWinccidev, then no spl2_device..
   ASSERT(NULL != pWinccidev);
   if ( NULL == pWinccidev ) {
      PTRACEOUT_LINT(status);
      return status;
   }
#if 0
   // Get the spl2_device object from the Windows specific device context
   //  spl2_device is portable and used by common code.
   pspl2dev = PWIN_CCIPDrv_DEVICE_TO_PCCIPDrv_DEVICE(pWinccidev);
   ASSERT(NULL != pspl2dev);
   if ( NULL == pspl2dev ) {
      PTRACEOUT_LINT(status);
      return status;
   }

   //
   // Free hardware resources
   //
   if ( NULL != spl2_dev_kvp_config(pspl2dev) ) {
      MmUnmapIoSpace(spl2_dev_kvp_config(pspl2dev), (SIZE_T)spl2_dev_len_config(pspl2dev));
      spl2_dev_kvp_config(pspl2dev) = NULL;
   }
#endif
   status = STATUS_SUCCESS;
	PTRACEOUT_LINT(status);
#endif
   return status;
}

//=============================================================================
// Name: uidrv_session_create
// Description: Create a new application session
// Interface: public
// Inputs: pid - ID of process
// Outputs: none.
// Comments: The process ID is used to identify the session uniquely
//=============================================================================
struct uidrv_session * uidrv_session_create(btPID pid)
{
   UNREFERENCED_PARAMETER( pid );
#if 0
   struct uidrv_session *psession = NULL;

   PTRACEIN;
#if 0 
   psession = (struct uidrv_session *)kosal_kmalloc(sizeof(struct uidrv_session));
   ASSERT(NULL != psession);
   if ( unlikely( NULL == psession ) ) {
      PTRACEOUT_PTR(NULL);
      return NULL;
   }

   // Initialize session's lists, queues and sync objects
   kosal_list_init(&psession->m_sessions);
   kosal_list_init(&psession->m_devicelist);

   // Initialize queues
   kosal_init_waitqueue_head(&psession->m_waitq);
   aal_queue_init(&psession->m_eventq);

   kosal_mutex_init(&psession->m_sem);

   // Record the owning process
   psession->m_pid = pid;

   // Bind the UI driver message handler
   psession->m_msgHandler.sendevent = uidrv_sendevent;
   psession->m_msgHandler.getwsid   = uidrv_getwsid;
   psession->m_msgHandler.freewsid  = uidrv_freewsid;

   // Record a pointer to the singleton UDDI
   psession->m_aaldriver = &thisDriver;

   PTRACEOUT_PTR(psession);
   return psession;
#endif
#endif
   return NULL;
}


//=============================================================================
// Name: CCIPDrvEvtDeviceD0Entry
// Description:  Perform any hardware initialization.  Set up contexts etc. (TBD)
// Interface: public
// Inputs: Device [in] - Device handle
//         PreviousState 
// Outputs: STATUS_SUCCESS - success.
// Comments: This function is not marked pageable. It runs in the device power 
//           up path so we want to avoid anything that can cause a page fault. 
//           
//=============================================================================
NTSTATUS
CCIPDrvEvtDeviceD0Entry(IN WDFDEVICE              Device,
                     IN WDF_POWER_DEVICE_STATE PreviousState)
{
   UNREFERENCED_PARAMETER( Device );
   UNREFERENCED_PARAMETER( PreviousState );
   NTSTATUS              status = STATUS_DEVICE_CONFIGURATION_ERROR;
#if 0

   PWIN_CCIPDrv_DEVICE      pWinccidev  = NULL;
   struct spl2_device   *pspl2dev = NULL;
   btInt                 res      = 0;
//   struct aal_device_id  aaldevid;

//   UNREFERENCED_PARAMETER(aaldevid);
   UNREFERENCED_PARAMETER(PreviousState);

   PTRACEIN;
#if 0 
   ASSERT(NULL != Device);
   if ( NULL == Device ) {
      PTRACEOUT_LINT(status);
      return status;
   }

   pWinccidev = CCIFdoGetCCIDev(Device);
   ASSERT(NULL != pWinccidev);
   if ( NULL == pWinccidev ) {
      PTRACEOUT_LINT(status);
      return status;
   }

   pspl2dev = PWIN_CCIPDrv_DEVICE_TO_PCCIPDrv_DEVICE(pWinccidev);
   ASSERT(NULL != pspl2dev);
   if ( NULL == pspl2dev) {
      PTRACEOUT_LINT(status);
      return status;
   }

   spl2_dev_to_os_dev(pspl2dev) = (kosal_os_dev) WdfDeviceWdmGetDeviceObject(Device);


   // If this is a soft device then setup the simulator
   if( pWinccidev->m_softdevice ){

      aaldevid_afuguidh(spl2_dev_to_sim_devid(pspl2dev)) = 0;
      aaldevid_afuguidl(spl2_dev_to_sim_devid(pspl2dev)) = 0x11100181;

      PVERBOSE("Making the Simulated AFU ID  0x%Lx  0x%Lx\n",
               aaldevid_afuguidh(spl2_dev_to_sim_devid(pspl2dev)),
               aaldevid_afuguidl(spl2_dev_to_sim_devid(pspl2dev)) );

      res = spl2_sim_internal_probe(pspl2dev, &aaldevid);
      if (0 != res) {
         PTRACEOUT_LINT(status);
         return status;
      }
   }

   // Initial the CCIPDrv service and add it to the list
   res = spl2_device_init(pspl2dev, &aaldevid, &g_device_list);
   if(0 != res){
      PTRACEOUT_LINT(status);
      return status;
   }
   
#if 0 
   // Check for QLP. Only valid if QPI
   if( -1 == check_qlp_feature_id(pspl2dev)){
      status = STATUS_DEVICE_CONFIGURATION_ERROR;
   }
#endif
#endif
   status = STATUS_SUCCESS;
   PTRACEOUT_LINT(status);
#endif
   return status;
}

//=============================================================================
// Name: CCIPDrvEvtDeviceD0Exit
// Description:  Perform any hardware cleanup.
// Interface: public
// Inputs: Device [in] - Device handle
//         TargetState 
// Outputs: STATUS_SUCCESS - success.
// Comments: This function is not marked pageable. It runs in the device power 
//           up path so we want to avoid anything that can cause a page fault. 
//           
//=============================================================================
NTSTATUS
CCIPDrvEvtDeviceD0Exit(IN WDFDEVICE              Device,
                    IN WDF_POWER_DEVICE_STATE TargetState)
{
   NTSTATUS            status   = STATUS_DEVICE_CONFIGURATION_ERROR;
   UNREFERENCED_PARAMETER( Device );
   UNREFERENCED_PARAMETER( TargetState );
#if 0

   PWIN_CCIPDrv_DEVICE    pWinccidev  = NULL;
   struct spl2_device *pspl2dev = NULL;
   pkosal_list_head    This     = NULL;
   pkosal_list_head    tmp      = NULL;      
   PTRACEIN;

   ASSERT(NULL != Device);
   if ( NULL == Device ) {
      PTRACEOUT_LINT(status);
      return status;
   }
#if 0 
   pWinccidev = CCIFdoGetCCIDev(Device);
   ASSERT(NULL != pWinccidev);
   if ( NULL == pWinccidev ) {
      PTRACEOUT_LINT(status);
      return status;
   }

   pspl2dev = PWIN_CCIPDrv_DEVICE_TO_PCCIPDrv_DEVICE(pWinccidev);
   ASSERT(NULL != pspl2dev);
   if ( NULL == pspl2dev ) {
      PTRACEOUT_LINT(status);
      return status;
   }

   // TODO THIS MAY BE LEAKY AND WRONG. CHECK LIST
  if ( !kosal_list_is_empty(&g_device_list) ) {

      // Run through list of devices.  Use safe variant
      //  as we will be deleting entries
      kosal_list_for_each_safe(This, tmp, &g_device_list) {

         pspl2dev = spl2_list_to_spl2_device(This);

         kosal_sem_get_krnl( &pspl2dev->m_sem );

         if ( !spl2_dev_pci_dev_is_enabled(pspl2dev) ) {

            // Remove from the list
            kosal_list_del(This);

            kosal_sem_put( &pspl2dev->m_sem );

            PDEBUG("Deleting device 0x%p with list head 0x%p from list 0x%p\n",
                      pspl2dev, This, &g_device_list);

            spl2_device_destroy(pspl2dev);
         } else {
            kosal_sem_put( &pspl2dev->m_sem );
         }

      }

   }

#if 0 
   // Reset the device
   //spl2_reset(pspl2dev);

	//PINFO("moving to %s\n", DbgDevicePowerString(TargetState));
   // Free the SPL Device Status Memory (DSM) region
   if ( NULL != spl2_dev_SPLDSM(pspl2dev) ) {
      kosal_free_contiguous_mem((btVirtAddr)spl2_dev_SPLDSM(pspl2dev), spl2_dev_SPLDSM_size);
      spl2_dev_SPLDSM(pspl2dev) = NULL;
   }

   // Free the SPL Context region
   if ( NULL != spl2_dev_SPLCTX(pspl2dev) ) {
      kosal_free_contiguous_mem((btVirtAddr)spl2_dev_SPLCTX(pspl2dev), spl2_dev_SPLCTX_size);
      spl2_dev_SPLCTX(pspl2dev) = NULL;
   }

   // Free the AFU DSM
   if ( NULL != spl2_dev_AFUDSM(pspl2dev) ) {
      kosal_free_contiguous_mem((btVirtAddr)spl2_dev_AFUDSM(pspl2dev), spl2_dev_AFUDSM_size);
      spl2_dev_AFUDSM(pspl2dev) = NULL;
   }
#endif
#endif

   switch ( TargetState ) {
      case WdfPowerDeviceD1 :
      case WdfPowerDeviceD2 :
      case WdfPowerDeviceD3 :
      case WdfPowerDevicePrepareForHibernation :
      case WdfPowerDeviceD3Final :
      default :
      break;
   }

   status = STATUS_SUCCESS;
   PTRACEOUT_LINT(status);
#endif
   return status;
}

//=============================================================================
// Name: CCIPDrvEvtDeviceReleaseHardware
// Description:  Free resources allocated to HW.
// Interface: public
// Inputs: Device [in] - Device handle
//         TargetState 
// Outputs: STATUS_SUCCESS - success.
// Comments: This function is not marked pageable. It runs in the device power 
//           up path so we want to avoid anything that can cause a page fault. 
//           
//=============================================================================
NTSTATUS
CCIPDrvEvtDeviceReleaseHardware(IN  WDFDEVICE    Device,
                             IN  WDFCMRESLIST ResourcesTranslated)
{
   NTSTATUS         status = STATUS_DEVICE_CONFIGURATION_ERROR;

   UNREFERENCED_PARAMETER( Device );
   UNREFERENCED_PARAMETER( ResourcesTranslated );
#if 0
   PWIN_CCIPDrv_DEVICE pWinccidev;

   UNREFERENCED_PARAMETER(ResourcesTranslated);

   PTRACEIN;
   PAGED_CODE();
#if 0 
   ASSERT(NULL != Device);
   if ( NULL == Device ) {
      PTRACEOUT_LINT(status);
      return status;
   }

   pWinccidev = CCIFdoGetCCIDev(Device);
   ASSERT(NULL != pWinccidev);
   if ( NULL == pWinccidev ) {
      PTRACEOUT_LINT(status);
      return status;
   }

   //
   // Unmap any I/O ports. Disconnecting from the interrupt will be done
   // automatically by the framework.
   //
   status = CCIPDrvUnmapHWResources(pWinccidev);
#endif
   PTRACEOUT_LINT(status);
#endif
   return status;
}



//=============================================================================
// Name: CCIPDrvDeviceFileClose
// Description:  Called when the framework receives IRP_MJ_DEVICE_CONTROL
//               requests from the system.
// Interface: public
// Inputs: FileObject - A handle to a framework file object, which was 
//                      previously received by the driver's EvtDeviceFileCreate 
//                      callback function.
// Outputs: None.
// Comments: The framework calls a driver's EvtFileCleanup callback function 
//           when the last handle to the specified file object has been closed.
//           (Because of outstanding I/O requests, this handle might not have 
//            been released.)
//            After the framework calls a driver's EvtFileCleanup callback 
//            function, it calls the driver's EvtFileClose callback function.
//            The EvtFileCleanup callback function is called synchronously, 
//            in the context of the thread that closed the last file object 
//            handle.
//=============================================================================
VOID CCIPDrvDeviceFileCleanup(  _In_  WDFFILEOBJECT FileObject)
{
   UNREFERENCED_PARAMETER( FileObject );
#if 0
   WDFDEVICE  hDevice = WdfFileObjectGetDevice(FileObject);
   PWIN_CCIPDrv_DEVICE        pWinccidev    = NULL;
   struct spl2_device *pspl2dev  = NULL, *tmp = NULL;
   struct aal_device     * pdev       = NULL;
//      struct memmgr_session   *pmem_sess  = NULL;
   struct spl2_session     *sessp      = NULL;
   struct aaldev_ownerSession *  pownerSess =NULL;
#if 0
   PDEBUG("CCIPDrvDeviceFileCleanup() entered\n");

   pWinccidev = CCIFdoGetCCIDev(hDevice);
  
//   pspl2dev =  PWIN_CCIPDrv_DEVICE_TO_PCCIPDrv_DEVICE(pWinccidev);


   // Clean-up in case of dirty close
   if(NULL != puisess){

      // Release all devices since this device is as good as uidrv in Linux (HACK)
      kosal_list_for_each_entry_safe(pspl2dev, tmp, &g_device_list, m_session_list, struct spl2_device){

         // Clean up after device
         pdev = spl2_dev_to_aaldev(pspl2dev);
         if(likely(pdev) ){           

            pownerSess = dev_OwnerSession(pdev,kosal_get_pid());
            if(NULL!=pownerSess){
               sessp = (struct spl2_session *)aalsess_pipHandle(pownerSess);

               // Free all memmory mapped
               if(NULL != sessp){

                  // Protect access to activesess
                  kosal_sem_get_user( spl2_dev_semp(pdev) );
               
                  // If a session is active stop it
                  if ( NULL != spl2_dev_activesess(pspl2dev) ) {
                     // Wait for complete
                     spl2_trans_end(pspl2dev);
                  };
                  // Done accessing activesess
                  kosal_sem_put( spl2_dev_semp(pdev) );

                  // Unmap CSR memory
                  if(NULL != sessp->m_csruvAddr){
                     if(NULL != sessp->m_csrmap){
                        MmUnmapLockedPages( sessp->m_csruvAddr, sessp->m_csrmap);
                        IoFreeMdl(sessp->m_csrmap);
                     }
                  }
               
                  // Unmap DSM memory
                  if(NULL != sessp->m_wsuvAddr){
                     if(NULL != sessp->m_wsmap){
                        MmUnmapLockedPages( sessp->m_wsuvAddr, sessp->m_wsmap);
                        IoFreeMdl(sessp->m_wsmap);
                     }
                  }

                  flush_all_wsids(sessp);
                  session_destroy(sessp);

               }// End NULL != sessp
               // Remove it from the owner list  
               if( dev_removeOwner( pdev, puisess->m_pid) != aaldev_addowner_OK)  {

                  PERR("failed to claim the device\n");
               }
            }//End NULL != pownersession
         }// End likely(pdev)
      }// Kosal for

      // Unblock any waiting threads
      // TODO MAKE SURE EVENT QUEUE EMPTY BEFORE FREE
      if(puisess->m_waitq){
         kosal_wake_up_interruptible(puisess->m_waitq);
         puisess->m_waitq = NULL;
      }  
      if(--numdevicesOpen == 0){
         kosal_kfree(puisess, sizeof(struct uidrv_session));
         puisess = NULL;
      }
   }// End NULL !=puisess
#endif
#endif
}



//=============================================================================
// Name: CCIPDrvDeviceFileClose
// Description:  Called when the framework receives IRP_MJ_DEVICE_CONTROL
//               requests from the system.
// Interface: public
// Inputs: FileObject - A handle to a framework file object, which was 
//                      previously received by the driver's EvtDeviceFileCreate 
//                      callback function.
// Outputs: None.
// Comments: 
//=============================================================================
VOID CCIPDrvDeviceFileClose( _In_  WDFFILEOBJECT FileObject)
{
      UNREFERENCED_PARAMETER(FileObject);
//      PDEBUG("CCIPDrvDeviceFileClose() entered\n");

}

//=============================================================================
// Name: CCIPDrvDeviceFileCreate
// Description:  Called when the framework receives IRP_MJ_DEVICE_CONTROL
//               requests from the system.
// Interface: public
// Inputs: hDevice - Handle to the framework device object that is associated
//                with the I/O request.
//         Request - Handle to a framework request object.
//          FileObject - A handle to a framework file object, which was 
//                      previously received by the driver's EvtDeviceFileCreate 
//                      callback function.
// Outputs: None.
// Comments: 
//=============================================================================
VOID CCIPDrvDeviceFileCreate ( IN WDFDEVICE  hDevice,
                            IN WDFREQUEST  Request,
                            IN WDFFILEOBJECT  FileObject)
{
   UNREFERENCED_PARAMETER( hDevice );
   UNREFERENCED_PARAMETER( FileObject );
   UNREFERENCED_PARAMETER( Request );

#if 0
   PWIN_CCIPDrv_DEVICE        pWinccidev    = NULL;
   struct spl2_device      *pspl2dev  = NULL;
   struct aal_device     * pdev       = NULL;

   UNREFERENCED_PARAMETER(Request);
   UNREFERENCED_PARAMETER(FileObject);
#if 0   
   PDEBUG("CCIPDrvDeviceFileCreate() entered\n");

   // Create the uisession  ALL HAC RIGHT NOW
   if ( NULL == puisess ) {     
      puisess = uidrv_session_create(kosal_get_pid());
   }
   
   pWinccidev = CCIFdoGetCCIDev(hDevice);
   pspl2dev =  PWIN_CCIPDrv_DEVICE_TO_PCCIPDrv_DEVICE(pWinccidev);
         
   pdev = spl2_dev_to_aaldev(pspl2dev);
#if 0 
   if(likely(pdev) ){
      aaldev_AddOwner_e ret;
      if( unlikely( (ret=dev_addOwner( pdev,
                                       puisess->m_pid,
                                       puisess, // TODO MANIFEST ON OWNER NOT SUPPORTED YET
                                       &puisess->m_devicelist)) != aaldev_addowner_OK)  ) {

         PERR("failed to claim the device\n");
         WdfRequestComplete(Request, STATUS_DEVICE_DATA_ERROR);
         return;
      }
   }   
#endif   
   // Count the device
   numdevicesOpen++;
#endif
   WdfRequestComplete(Request, STATUS_SUCCESS);
//   PDEBUG("Device Owned by pid %d\n",puisess->m_pid);
#endif
}

//=============================================================================
// Name: CCIPDrvEvtIoDeviceControl
// Description:  Called when the framework receives IRP_MJ_DEVICE_CONTROL
//               requests from the system.
// Interface: public
// Inputs: Queue - Handle to the framework queue object that is associated
//                with the I/O request.
//         Request - Handle to a framework request object.
//
//         OutputBufferLength - length of the request's output buffer,
//                              if an output buffer is available.
//         InputBufferLength - length of the request's input buffer,
//                              if an input buffer is available.
//
//         IoControlCode - the driver-defined or system-defined I/O control code
//                         (IOCTL) that is associated with the request.
// Outputs: None.
// Comments: Primary message entry point.
//=============================================================================
VOID
CCIPDrvEvtIoDeviceControl(IN WDFQUEUE   Queue,
                       IN WDFREQUEST Request,
                       IN size_t     OutputBufferLength,
                       IN size_t     InputBufferLength,
                       IN ULONG      IoControlCode)
{

   UNREFERENCED_PARAMETER( Queue );
   UNREFERENCED_PARAMETER( Request );
   UNREFERENCED_PARAMETER( OutputBufferLength );
   UNREFERENCED_PARAMETER( InputBufferLength );
   UNREFERENCED_PARAMETER( IoControlCode );
#if 0

   NTSTATUS                status     = STATUS_SUCCESS;
   WDFDEVICE               hDevice    = NULL;
   PDEVICE_OBJECT          pdevObject = NULL;
   PWIN_CCIPDrv_DEVICE        pWinccidev    = NULL;
   struct aalui_ioctlreq * presp      = NULL;
   struct aalui_ioctlreq * preq       = NULL;
 
   UNREFERENCED_PARAMETER(OutputBufferLength);
   UNREFERENCED_PARAMETER(InputBufferLength);
#if 0
   PTRACEIN;
   PAGED_CODE();

   // Get the device associated with this queue
   hDevice  = WdfIoQueueGetDevice(Queue);
   pWinccidev  = CCIFdoGetCCIDev(hDevice);


   PDEBUG("uidrv_ioctl() received a 0x%x %d \n", IoControlCode,IoControlCode);
   ///////////////////////////////////////////
   // If its an AAL command process seperately
   //
   if ( (IoControlCode & AALUID_IOCTL<<2) == (AALUID_IOCTL<<2)  ) {
      size_t   preqsize  = 0; // size_t for compatibility with WdfRequestRetrieveInputBuffer
      btWSSize ReqSize;
      size_t   prespsize = 0; // size_t for compatibility with WdfRequestRetrieveOutputBuffer
      btWSSize RespSize;
      btInt    ret;
  //    struct spl2_device *pspl2dev =  PWIN_CCIPDrv_DEVICE_TO_PCCIPDrv_DEVICE(pWinccidev);

      //
      // Use WdfRequestRetrieveInputBuffer and WdfRequestRetrieveOutputBuffer
      // to get the request buffers. (Should change so assigment is not in conditioal)
      //
      status = WdfRequestRetrieveInputBuffer(Request, sizeof(struct aalui_ioctlreq), (PVOID*)&preq, &preqsize);
      if ( !NT_SUCCESS(status) ) {
         PERR("WdfRequestRetrieveInputBuffer failed\n");
         WdfRequestCompleteWithInformation(Request, status, (ULONG_PTR)0);
         PTRACEOUT;
         return;
      }
      ReqSize = (btWSSize)preqsize;

      status = WdfRequestRetrieveOutputBuffer(Request, sizeof(struct aalui_ioctlreq), (PVOID*)&presp, &prespsize);
      if ( !NT_SUCCESS(status) ) {
         PERR("WdfRequestRetrieveOutputBuffer failed\n");
         WdfRequestCompleteWithInformation(Request, status, (ULONG_PTR)0);
         PTRACEOUT;
         return;
      }
      RespSize = (btWSSize)prespsize;
       
      // Must have a session from FileCreate()
      if ( NULL == puisess ) {     
         WdfRequestComplete(Request, STATUS_DEVICE_DATA_ERROR);
         return;
      }
      
      if ( AALUID_IOCTL_POLL == IoControlCode ) {
         uidrv_poll(Request, puisess);
         return;
      }

      // MMAP emulation
      if ( AALUID_IOCTL_MMAP == IoControlCode ) {
         struct aal_wsid *wsidp = NULL;
         if(NULL == preq->context){
            WdfRequestComplete(Request, STATUS_DEVICE_DATA_ERROR);
            return;
         }
         wsidp = pgoff_to_wsidobj((unsigned long long)preq->context);
         if(NULL != (presp->context = uidrv_mmap(wsidp, puisess))){
            WdfRequestCompleteWithInformation(Request, STATUS_SUCCESS, (ULONG_PTR)OutputBufferLength);
         }else{
            WdfRequestComplete(Request, STATUS_DEVICE_DATA_ERROR);
         }
         return;
      }

      ret = uidrv_ioctl(puisess,
                        IoControlCode,
                        (struct aalui_ioctlreq *)preq,
                        preqsize,
                        (struct aalui_ioctlreq *)presp,
                       &RespSize);
      if ( ret < 0 ) {
         PERR("uidrv_ioctl() failed with 0x%" PRIx64 "\n", ret);
         WdfRequestComplete(Request, STATUS_DEVICE_DATA_ERROR);
      } else {
         WdfRequestCompleteWithInformation(Request, STATUS_SUCCESS, (ULONG_PTR)OutputBufferLength);
      }

      PTRACEOUT;
      return;
   }
    

   //////////////////////////
   // Process Direct commands

   switch ( IoControlCode ) {

      // Protoytpe Event loop
      case IOCTL_CCIPDrv_DIRECT_WAIT_MESSAGE : {
         // Protect by semaphore. If request non-NULL then return immediate as queue not empty
         pWinccidev->currReq = Request;
         IoMarkIrpPending(WdfRequestWdmGetIrp(Request));
         PTRACEOUT;
      } return;

      case IOCTL_CCIPDrv_DIRECT_GET_MESSAGE : {
         // Copy message into response buffer
         WdfRequestCompleteWithInformation(pWinccidev->currReq, STATUS_SUCCESS, (ULONG_PTR) sizeof(struct aalui_ioctlreq));

         // When the last event is dequeued 
         pWinccidev->currReq = NULL;
         PTRACEOUT;
      } return;

      case IOCTL_CCIPDrv_DIRECT_CREATE_WORKSPACE : {
         int s = sizeof(struct aalui_ioctlreq);
         UNREFERENCED_PARAMETER(s);
 #if 0
         struct aalui_ioctlreq   *inreq       = NULL;
         struct aalui_ioctlreq   *outreq       = NULL;
         struct spl2_device    *pspl2dev = NULL;
         void * pSPLCTX  = NULL;

         WdfRequestRetrieveInputBuffer(Request, s, (PVOID*)&inreq,NULL);
      
         // Map the SPL context to user space

         pspl2dev = PWIN_CCIPDrv_DEVICE_TO_PCCIPDrv_DEVICE(pWinccidev);
         pSPLCTX = (PVOID)spl2_dev_SPLDSM(pspl2dev);

         // Create an MDL around the system memory
         pWinccidev->m_pmdl = IoAllocateMdl(pSPLCTX,spl2_dev_SPLDSM_size,FALSE,FALSE,NULL);
      
         if(NULL == pWinccidev->m_pmdl){
            PERR("Failed to crrate MDL\n");
            status = STATUS_INSUFFICIENT_RESOURCES;
            break;
         }
         MmBuildMdlForNonPagedPool(pWinccidev->m_pmdl);

         WdfRequestRetrieveOutputBuffer(Request, s, (PVOID*)&outreq,NULL);
         if(NULL==outreq){
            PERR("Failed to crrate MDL\n");
            status = STATUS_INSUFFICIENT_RESOURCES;
            break;
         }

         try{
            // Map the memory into user land
            outreq->context = MmMapLockedPagesSpecifyCache(pWinccidev->m_pmdl,
                                                           UserMode,
                                                           MmNonCached,
                                                           NULL,
                                                           FALSE,
                                                           HighPagePriority);
         } except(EXCEPTION_EXECUTE_HANDLER){
            outreq->context=NULL;
         }
#endif
         WdfRequestCompleteWithInformation(Request, STATUS_SUCCESS, (ULONG_PTR) 0);
         //IoMarkIrpPending(WdfRequestWdmGetIrp(Request));      

      } break;


      case IOCTL_CCIPDrv_DIRECT_FREE_WORKSPACE : {
         int s = sizeof(struct aalui_ioctlreq);
         UNREFERENCED_PARAMETER(s);

#if 0
         struct aalui_ioctlreq   *inreq       = NULL;
         WdfRequestRetrieveInputBuffer(Request, s, (PVOID*)&inreq,NULL);

         //Note that since this was created with MmMapLockedPagesSpecifyCache 
         //  specifing user mode, the caller must be in the context of the original 
         //  process before calling MmUnmapLockedPages or the unmapping operation 
         //  could delete the address range of a random process.
         if(NULL != pWinccidev->m_pmdl){
            MmUnmapLockedPages(inreq->context, pWinccidev->m_pmdl);
            IoFreeMdl(pWinccidev->m_pmdl);
         }
#endif
         WdfRequestCompleteWithInformation(Request, STATUS_SUCCESS, 0);

      } break;

      default :
         status = STATUS_INVALID_DEVICE_REQUEST;
      break;
   }

   //
   // Run a background thread to fake an async from HW
   //
   try {
      pdevObject = WdfDeviceWdmGetDeviceObject(hDevice);
   } except(EXCEPTION_EXECUTE_HANDLER) {
      PERR("WdfDeviceWdmGetDeviceObject failed\n");
      WdfRequestCompleteWithInformation(Request, STATUS_DEVICE_DATA_ERROR, (ULONG_PTR)0);
      PTRACEOUT;
      return;
   }
   
   // Start a work item by scheduling a  worker thread
#if 0
   pWinccidev->pwork = IoAllocateWorkItem(pdevObject);
   IoQueueWorkItem(pWinccidev->pwork,TestWorkItemCallback, DelayedWorkQueue, (PVOID)pWinccidev);
#endif
   {
      struct aal_device  dev;
      aaldev_to_basedev(&dev) = pdevObject;
      if(NULL == pWinccidev->m_workq ){
//         pWinccidev->m_workq = kosal_create_workqueue(&dev);
         pWinccidev->m_workq = IoAllocateWorkItem(pdevObject); 
         KOSAL_INIT_WORK( &(pWinccidev->task_handler), Testtask_poller);
         kosal_queue_delayed_work(pWinccidev->m_workq, &(pWinccidev->task_handler), 20000);
      }

   }
#endif
   PTRACEOUT;
#endif
}

void
Testtask_poller( pwork_object pwork)
{
   UNREFERENCED_PARAMETER( pwork );
#if 0 
//   PWIN_CCIPDrv_DEVICE pWinccidev = kosal_get_object_containing( pwork, WIN_CCIPDrv_DEVICE, task_handler );
   if(NULL != pWinccidev->currReq){
      // Mark as complete. In real implementation this will be in queue_event and the size will be the size of the first event
      WdfRequestCompleteWithInformation(pWinccidev->currReq, STATUS_SUCCESS, (ULONG_PTR) sizeof(struct aalui_ioctlreq));
   }

   // Cleanup
   if(NULL != pWinccidev->m_workq){
     IoFreeWorkItem(pWinccidev->m_workq);
     pWinccidev->m_workq = NULL;
   }
#endif
}

//=============================================================================
// Name: TestWorkItemCallback
// Description:  Called when the framework receives IRP_MJ_DEVICE_CONTROL
//               requests from the system.
// Interface: public
// Inputs: 
// Outputs: None.
// Comments: Primary message entry point.
//=============================================================================
VOID TestWorkItemCallback(IN PDEVICE_OBJECT pdevObject, IN PVOID Context)
{ 
   UNREFERENCED_PARAMETER( pdevObject );
   UNREFERENCED_PARAMETER( Context );

#if 0    
   PWIN_CCIPDrv_DEVICE        pWinccidev     = (PWIN_CCIPDrv_DEVICE) Context;
   int ret;

   // Wait 5 seconds
   ret = kosal_mdelay(5000);

   // Mark as complete
   WdfRequestCompleteWithInformation(pWinccidev->currReq, STATUS_SUCCESS, (ULONG_PTR) sizeof(struct aalui_ioctlreq));
   
   // Cleanup
   IoFreeWorkItem(pWinccidev->m_workq);
   
   pWinccidev->m_workq = NULL;
   pWinccidev->currReq = NULL;
#endif
}

#if 0
VOID
CCIPDrvEvtIoRead (
    WDFQUEUE      Queue,
    WDFREQUEST    Request,
    size_t        Length
    )
/*++

Routine Description:

    Performs read from the toaster device. This event is called when the
    framework receives IRP_MJ_READ requests.

Arguments:

    Queue -  Handle to the framework queue object that is associated with the
             I/O request.
    Request - Handle to a framework request object.

    Lenght - Length of the data buffer associated with the request.
                 By default, the queue does not dispatch
                 zero length read & write requests to the driver and instead to
                 complete such requests with status success. So we will never get
                 a zero length request.

Return Value:

  None.

--*/
{
    NTSTATUS    status;
    ULONG_PTR bytesCopied =0;
    WDFMEMORY memory;

    UNREFERENCED_PARAMETER(Queue);
    UNREFERENCED_PARAMETER(Length);

    PAGED_CODE();

    KdPrint(( "CCIPDrvEvtIoRead: Request: 0x%p, Queue: 0x%p\n",
                                    Request, Queue));

    //
    // Get the request memory and perform read operation here
    //
    status = WdfRequestRetrieveOutputMemory(Request, &memory);
    if(NT_SUCCESS(status) ) {
        //
        // Copy data into the memory buffer using WdfMemoryCopyFromBuffer
        //
    }

    WdfRequestCompleteWithInformation(Request, status, bytesCopied);
}

VOID
CCIPDrvEvtIoWrite (
    WDFQUEUE      Queue,
    WDFREQUEST    Request,
    size_t        Length
    )
/*++

Routine Description:

    Performs write to the toaster device. This event is called when the
    framework receives IRP_MJ_WRITE requests.

Arguments:

    Queue -  Handle to the framework queue object that is associated with the
            I/O request.
    Request - Handle to a framework request object.

    Lenght - Length of the data buffer associated with the request.
                 The default property of the queue is to not dispatch
                 zero lenght read & write requests to the driver and
                 complete is with status success. So we will never get
                 a zero length request.

Return Value:

   None
--*/

{
    NTSTATUS    status;
    ULONG_PTR bytesWritten =0;
    WDFMEMORY memory;

    UNREFERENCED_PARAMETER(Queue);
    UNREFERENCED_PARAMETER(Length);

    KdPrint(("CCIPDrvEvtIoWrite. Request: 0x%p, Queue: 0x%p\n",
                                Request, Queue));

    PAGED_CODE();

    //
    // Get the request buffer and perform write operation here
    //
    status = WdfRequestRetrieveInputMemory(Request, &memory);
    if(NT_SUCCESS(status) ) {
        //
        // 1) Use WdfMemoryCopyToBuffer to copy data from the request
        // to driver buffer.
        // 2) Or get the buffer pointer from the request by calling
        // WdfRequestRetrieveInputBuffer
        // 3) Or you can get the buffer pointer from the memory handle
        // by calling WdfMemoryGetBuffer.
        //
        bytesWritten = Length;
    }

    WdfRequestCompleteWithInformation(Request, status, bytesWritten);

}
#endif

