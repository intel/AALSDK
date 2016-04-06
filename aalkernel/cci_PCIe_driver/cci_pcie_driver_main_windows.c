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
/// Accelerator Abstraction Layer
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
//#include <devguid.h>

#define MODULE_FLAGS CCIPCIE_DBG_MOD

#include "cci_pcie_windows.h"
#include "cci_pcie_driver_simulator.h"

btUnsigned32bitInt sim = 0;

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

//================================================================
// Create the accessor CCIFdoGetCCIDev(WDFHANDLE fdo)
//  Description: Takes and FDO and returns the CCIDevice structure
//================================================================
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME( CCI_DEVICE, CCIFdoGetCCIDev );

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

// Exported interfaces from AALBus Services
AAL_DEVICE_INTERFACE                iaalbus;

UPDATECONFIG_INTERFACE_STANDARD     iupdateconfig;


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
   DbgPrint("Accelerator Abstraction Layer\n");
   DbgPrint("AAL CCI-P PCIe Driver\n");
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

   WDF_PNPPOWER_EVENT_CALLBACKS    pnpPowerCallbacks;

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
   // Initialize attributes and a context area for the device object.
   WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE( &fdoAttributes, CCI_DEVICE );

   //
   // Setup the file object that enables access to the device.
   status = ccidrv_initUMAPIFileObject( DeviceInit );
   if( !NT_SUCCESS( status ) ) {
      PERR( "ccidrv_initUMAPIFileObject failed with status code 0x%x\n", status );
      PTRACEOUT_LINT( status );
      return status;
   }


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
   kosal_list_init( &pWinccidev->g_device_list );

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
      int ret = 0;

      PERR("Failed to get the BUS_INTERFACE_STANDARD. Assuming simulated device\n");
      PTRACEOUT_LINT(status);
      pWinccidev->m_softdevice = true;    

      // Enumerate and Instantiate 2 Simulated Devices
      ret = cci_sim_discover_devices( 2, &pWinccidev->g_device_list );
      ASSERT(0 == ret);
      if(0 >ret){
         PERR("Failed to create simulated CCI devices.\n");
         // If cci_sim_discover_devices() fails it will have cleaned up.
         return STATUS_UNSUCCESSFUL;
      }
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
#if 0
   {
      int size = sizeof( UPDATECONFIG_INTERFACE_STANDARD );
      size++;
   }
   //
   // Get the AAL Bus Services interfaces
   // Stack is built - Get the device interface
   //
   // Get the BUS_INTERFACE_STANDARD for our device so that we can
   // read & write to PCI config space.
   status = WdfFdoQueryForInterface( hDevice,
                                     &GUID_AALBUS_CONFIGMANAGER_QUERY_INTERFACE,
                                     (PINTERFACE)&iupdateconfig,
                                     sizeof( UPDATECONFIG_INTERFACE_STANDARD ),
                                     1, // Version
                                     NULL ); //InterfaceSpecificData

   if( !NT_SUCCESS( status ) ) {
      PERR( "Failed to get the Device interface.\n" );
      PTRACEOUT_LINT( status );
      return status;
   } else {

      iupdateconfig.AddDevice(NULL, NULL);
   }
#endif 

{
//   WDF_OBJECT_ATTRIBUTES  ioTargetAttrib;
   WDFIOTARGET  ioTarget;
   WDF_IO_TARGET_OPEN_PARAMS  openParams;
   DECLARE_CONST_UNICODE_STRING(devname,L"\\Device\\aalbus\\AAL_IConfigManager");
#if 0 
   WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(
      &ioTargetAttrib,
      AAL_PDO_DEVICE_CONTEXT
      );
#endif
   status = WdfIoTargetCreate(
      hDevice,
      WDF_NO_OBJECT_ATTRIBUTES,
      &ioTarget
      );
   if( !NT_SUCCESS( status ) ) {
      return status;
   }
   WDF_IO_TARGET_OPEN_PARAMS_INIT_OPEN_BY_NAME(
      &openParams,
      &devname,
      STANDARD_RIGHTS_ALL
      );
   status = WdfIoTargetOpen(
      ioTarget,
      &openParams
      );
   if( !NT_SUCCESS( status ) ) {
      WdfObjectDelete( ioTarget );
      return status;
   }


   status = WdfIoTargetQueryForInterface(
      ioTarget,
      &GUID_AALBUS_CONFIGMANAGER_QUERY_INTERFACE,
      (PINTERFACE)&iupdateconfig,
      sizeof( UPDATECONFIG_INTERFACE_STANDARD ),
      1, // Version
      NULL );

   if( !NT_SUCCESS( status ) ) {

#if DBG
      DbgPrint( "WdfIoTargetQueryForInterface failed 0x%0x\n", status );
#endif

      return( status );
   }
}
   // Create the use mode interface to the driver
   status = ccidrv_initUMAPI( hDevice );
   if( !NT_SUCCESS( status ) ) {
      PERR( "Failed to initialize the user mode interface\n" );
   }

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
CCIPDrvEvtDevicePrepareHardware( WDFDEVICE    hDevice,
                                 WDFCMRESLIST Resources,
                                 WDFCMRESLIST ResourcesTranslated)
{

   UNREFERENCED_PARAMETER( Resources );
   UNREFERENCED_PARAMETER( ResourcesTranslated );

   PCCI_DEVICE       pWinccidev  = NULL;
   NTSTATUS          status      = STATUS_DEVICE_CONFIGURATION_ERROR;

   
   pWinccidev = CCIFdoGetCCIDev( hDevice );

   if( true == pWinccidev->m_softdevice){
      if( 0 != cci_sim_discover_devices( 2, &( pWinccidev->g_device_list ) ) ) {
         PERR("Failed to create simulated CCI devices.\n");
         // If cci_sim_discover_devices() fails it will have cleaned up.
         return status;
      }
   }

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



void
Testtask_poller( struct kosal_work_object *pwork)
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

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////          AAL SUPPORT FUNCTIONS           ////////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//=============================================================================
// Name: aalbus_get_bus
// Description: Returns a pointer to the bus object
// Interface: public export
// Inputs: none.
// Returns: Pointer to aalbus object
// Comments: Used by clients of the bus to get access to the bus interface
//=============================================================================
struct aal_bus *
   aalbus_get_bus( void )
{
   return NULL;
}

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
