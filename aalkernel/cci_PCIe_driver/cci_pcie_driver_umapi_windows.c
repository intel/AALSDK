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
/// @file cci_pcie_driver_umapi_windows.c
/// @brief Implements the kernel driver shell for CCI-P PCIe devices.
/// @ingroup DeviceDrivers
/// @verbatim
/// Intel(R) Accelerator Abstraction Layer
///
/// AUTHOR: Joseph Grecco, Intel Corporation
/// PURPOSE:  This file contains the main startup and shutdown code for the
///           Intel(R) Accelerator Abstraction Layer (AAL)
///           User Mode Interface for the AAL CCI device driver for Windows
/// HISTORY: Initial version based on Microsoft Sample Code
/// WHEN:          WHO:     WHAT:
/// 02/01/2016     JG       Initial Version Created
//****************************************************************************
#include "aalsdk/kernel/kosal.h"
#include <Initguid.h>               // This header must be included here so that the 
//  guids in IUpdateConfig are implemented
//  Only include this once in this module.

#include <devguid.h>



#define MODULE_FLAGS UIDRV_DBG_MOD
#include "cci_pcie_windows.h"
#include "cci_pcie_driver_umapi.h"

// User Mode Interface ID
DEFINE_GUID( GUID_DEVINTERFACE_CCIP,
             0x3a704f1b, 0xdba, 0x408d, 0xb4, 0xd5, 0x9d, 0x3d, 0x7a, 0x35, 0xc, 0xf3 );

// TODO seperate Linux um_APIDriver
struct um_driver            umDriver;

// Protoypes
EVT_WDF_DEVICE_FILE_CREATE  CCIPDrvDeviceFileCreate;
EVT_WDF_FILE_CLEANUP CCIPDrvDeviceFileCleanup;
EVT_WDF_FILE_CLOSE CCIPDrvDeviceFileClose;

//=============================================================================
// Name: ccidrv_initUMAPIFileObject
// Description: Initialization routine for the file object on the device. 
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments:
//=============================================================================
NTSTATUS
ccidrv_initUMAPIFileObject( PWDFDEVICE_INIT DeviceInit )
{
   WDF_FILEOBJECT_CONFIG           fileObjectConfig;

   //
   // Register File object callbacks
   //
   WDF_FILEOBJECT_CONFIG_INIT( &fileObjectConfig,
                               CCIPDrvDeviceFileCreate,
                               CCIPDrvDeviceFileClose,
                               CCIPDrvDeviceFileCleanup
                               );


   WdfDeviceInitSetFileObjectConfig( DeviceInit,
                                     &fileObjectConfig,
                                     WDF_NO_OBJECT_ATTRIBUTES );

   return STATUS_SUCCESS;
}


//=============================================================================
// Name: ccidrv_initUMAPI
// Description: Initialization routine for the interface. 
// Interface: public
// Inputs: none.
// Outputs: none.
// Comments:
//=============================================================================
NTSTATUS
ccidrv_initUMAPI( WDFDEVICE hDevice )
{
   NTSTATUS                        status        = STATUS_DEVICE_CONFIGURATION_ERROR;
   WDF_IO_QUEUE_CONFIG             queueConfig;
   WDFQUEUE                        queue;



   //
   // Tell the Framework that this device will need an interface
   status = WdfDeviceCreateDeviceInterface( hDevice,
                                            (LPGUID)&GUID_DEVINTERFACE_CCIP,
                                            NULL ); // ReferenceString
   if( !NT_SUCCESS( status ) ) {
      PERR( "WdfDeviceCreateDeviceInterface failed\n" );
      PTRACEOUT_LINT( status );
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
   WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE( &queueConfig, WdfIoQueueDispatchParallel );

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
   __analysis_assume( queueConfig.EvtIoStop != 0 );
   status = WdfIoQueueCreate( hDevice,
                              &queueConfig,
                              WDF_NO_OBJECT_ATTRIBUTES,
                              &queue );
   __analysis_assume( queueConfig.EvtIoStop == 0 );

   //---------------------------
   // Initialize data structures
   //---------------------------
   kosal_mutex_init( &umDriver.m_qsem );
   kosal_list_init( &umDriver.m_sessq );
   kosal_mutex_init( &umDriver.m_sem );

   kosal_mutex_init( &umDriver.wsid_list_sem );
   kosal_list_init( &umDriver.wsid_list_head );

   return status;
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
CCIPDrvEvtIoDeviceControl( IN WDFQUEUE   Queue,
                           IN WDFREQUEST Request,
                           IN size_t     OutputBufferLength,
                           IN size_t     InputBufferLength,
                           IN ULONG      IoControlCode )
{

   UNREFERENCED_PARAMETER( Queue );
   UNREFERENCED_PARAMETER( Request );
   UNREFERENCED_PARAMETER( OutputBufferLength );
   UNREFERENCED_PARAMETER( InputBufferLength );
   UNREFERENCED_PARAMETER( IoControlCode );
#if 0

   NTSTATUS                status = STATUS_SUCCESS;
   WDFDEVICE               hDevice = NULL;
   PDEVICE_OBJECT          pdevObject = NULL;
   PWIN_CCIPDrv_DEVICE        pWinccidev = NULL;
   struct aalui_ioctlreq * presp = NULL;
   struct aalui_ioctlreq * preq = NULL;

   UNREFERENCED_PARAMETER( OutputBufferLength );
   UNREFERENCED_PARAMETER( InputBufferLength );
#if 0
   PTRACEIN;
   PAGED_CODE();

   // Get the device associated with this queue
   hDevice = WdfIoQueueGetDevice( Queue );
   pWinccidev = CCIFdoGetCCIDev( hDevice );


   PDEBUG( "uidrv_ioctl() received a 0x%x %d \n", IoControlCode, IoControlCode );
   ///////////////////////////////////////////
   // If its an AAL command process seperately
   //
   if( ( IoControlCode & AALUID_IOCTL << 2 ) == ( AALUID_IOCTL << 2 ) ) {
      size_t   preqsize = 0; // size_t for compatibility with WdfRequestRetrieveInputBuffer
      btWSSize ReqSize;
      size_t   prespsize = 0; // size_t for compatibility with WdfRequestRetrieveOutputBuffer
      btWSSize RespSize;
      btInt    ret;
      //    struct spl2_device *pspl2dev =  PWIN_CCIPDrv_DEVICE_TO_PCCIPDrv_DEVICE(pWinccidev);

      //
      // Use WdfRequestRetrieveInputBuffer and WdfRequestRetrieveOutputBuffer
      // to get the request buffers. (Should change so assigment is not in conditioal)
      //
      status = WdfRequestRetrieveInputBuffer( Request, sizeof( struct aalui_ioctlreq ), (PVOID*)&preq, &preqsize );
      if( !NT_SUCCESS( status ) ) {
         PERR( "WdfRequestRetrieveInputBuffer failed\n" );
         WdfRequestCompleteWithInformation( Request, status, (ULONG_PTR)0 );
         PTRACEOUT;
         return;
      }
      ReqSize = (btWSSize)preqsize;

      status = WdfRequestRetrieveOutputBuffer( Request, sizeof( struct aalui_ioctlreq ), (PVOID*)&presp, &prespsize );
      if( !NT_SUCCESS( status ) ) {
         PERR( "WdfRequestRetrieveOutputBuffer failed\n" );
         WdfRequestCompleteWithInformation( Request, status, (ULONG_PTR)0 );
         PTRACEOUT;
         return;
      }
      RespSize = (btWSSize)prespsize;

      // Must have a session from FileCreate()
      if( NULL == puisess ) {
         WdfRequestComplete( Request, STATUS_DEVICE_DATA_ERROR );
         return;
      }

      if( AALUID_IOCTL_POLL == IoControlCode ) {
         uidrv_poll( Request, puisess );
         return;
      }

      // MMAP emulation
      if( AALUID_IOCTL_MMAP == IoControlCode ) {
         struct aal_wsid *wsidp = NULL;
         if( NULL == preq->context ) {
            WdfRequestComplete( Request, STATUS_DEVICE_DATA_ERROR );
            return;
         }
         wsidp = pgoff_to_wsidobj( ( unsigned long long )preq->context );
         if( NULL != ( presp->context = uidrv_mmap( wsidp, puisess ) ) ) {
            WdfRequestCompleteWithInformation( Request, STATUS_SUCCESS, (ULONG_PTR)OutputBufferLength );
         } else {
            WdfRequestComplete( Request, STATUS_DEVICE_DATA_ERROR );
         }
         return;
      }

      ret = uidrv_ioctl( puisess,
                         IoControlCode,
                         ( struct aalui_ioctlreq * )preq,
                         preqsize,
                         ( struct aalui_ioctlreq * )presp,
                         &RespSize );
      if( ret < 0 ) {
         PERR( "uidrv_ioctl() failed with 0x%" PRIx64 "\n", ret );
         WdfRequestComplete( Request, STATUS_DEVICE_DATA_ERROR );
      } else {
         WdfRequestCompleteWithInformation( Request, STATUS_SUCCESS, (ULONG_PTR)OutputBufferLength );
      }

      PTRACEOUT;
      return;
   }


   //////////////////////////
   // Process Direct commands

   switch( IoControlCode ) {

      // Protoytpe Event loop
      case IOCTL_CCIPDrv_DIRECT_WAIT_MESSAGE: {
         // Protect by semaphore. If request non-NULL then return immediate as queue not empty
         pWinccidev->currReq = Request;
         IoMarkIrpPending( WdfRequestWdmGetIrp( Request ) );
         PTRACEOUT;
      } return;

      case IOCTL_CCIPDrv_DIRECT_GET_MESSAGE: {
         // Copy message into response buffer
         WdfRequestCompleteWithInformation( pWinccidev->currReq, STATUS_SUCCESS, ( ULONG_PTR ) sizeof( struct aalui_ioctlreq ) );

         // When the last event is dequeued 
         pWinccidev->currReq = NULL;
         PTRACEOUT;
      } return;

      case IOCTL_CCIPDrv_DIRECT_CREATE_WORKSPACE: {
         int s = sizeof( struct aalui_ioctlreq );
         UNREFERENCED_PARAMETER( s );
#if 0
         struct aalui_ioctlreq   *inreq = NULL;
         struct aalui_ioctlreq   *outreq = NULL;
         struct spl2_device    *pspl2dev = NULL;
         void * pSPLCTX = NULL;

         WdfRequestRetrieveInputBuffer( Request, s, (PVOID*)&inreq, NULL );

         // Map the SPL context to user space

         pspl2dev = PWIN_CCIPDrv_DEVICE_TO_PCCIPDrv_DEVICE( pWinccidev );
         pSPLCTX = (PVOID)spl2_dev_SPLDSM( pspl2dev );

         // Create an MDL around the system memory
         pWinccidev->m_pmdl = IoAllocateMdl( pSPLCTX, spl2_dev_SPLDSM_size, FALSE, FALSE, NULL );

         if( NULL == pWinccidev->m_pmdl ) {
            PERR( "Failed to crrate MDL\n" );
            status = STATUS_INSUFFICIENT_RESOURCES;
            break;
         }
         MmBuildMdlForNonPagedPool( pWinccidev->m_pmdl );

         WdfRequestRetrieveOutputBuffer( Request, s, (PVOID*)&outreq, NULL );
         if( NULL == outreq ) {
            PERR( "Failed to crrate MDL\n" );
            status = STATUS_INSUFFICIENT_RESOURCES;
            break;
         }

         try {
            // Map the memory into user land
            outreq->context = MmMapLockedPagesSpecifyCache( pWinccidev->m_pmdl,
                                                            UserMode,
                                                            MmNonCached,
                                                            NULL,
                                                            FALSE,
                                                            HighPagePriority );
         } except( EXCEPTION_EXECUTE_HANDLER ){
            outreq->context = NULL;
         }
#endif
         WdfRequestCompleteWithInformation( Request, STATUS_SUCCESS, (ULONG_PTR)0 );
         //IoMarkIrpPending(WdfRequestWdmGetIrp(Request));      

      } break;


      case IOCTL_CCIPDrv_DIRECT_FREE_WORKSPACE: {
         int s = sizeof( struct aalui_ioctlreq );
         UNREFERENCED_PARAMETER( s );

#if 0
         struct aalui_ioctlreq   *inreq = NULL;
         WdfRequestRetrieveInputBuffer( Request, s, (PVOID*)&inreq, NULL );

         //Note that since this was created with MmMapLockedPagesSpecifyCache 
         //  specifing user mode, the caller must be in the context of the original 
         //  process before calling MmUnmapLockedPages or the unmapping operation 
         //  could delete the address range of a random process.
         if( NULL != pWinccidev->m_pmdl ) {
            MmUnmapLockedPages( inreq->context, pWinccidev->m_pmdl );
            IoFreeMdl( pWinccidev->m_pmdl );
         }
#endif
         WdfRequestCompleteWithInformation( Request, STATUS_SUCCESS, 0 );

      } break;

      default:
         status = STATUS_INVALID_DEVICE_REQUEST;
         break;
   }

   //
   // Run a background thread to fake an async from HW
   //
   try {
      pdevObject = WdfDeviceWdmGetDeviceObject( hDevice );
   } except( EXCEPTION_EXECUTE_HANDLER ) {
      PERR( "WdfDeviceWdmGetDeviceObject failed\n" );
      WdfRequestCompleteWithInformation( Request, STATUS_DEVICE_DATA_ERROR, (ULONG_PTR)0 );
      PTRACEOUT;
      return;
   }

   // Start a work item by scheduling a  worker thread
#if 0
   pWinccidev->pwork = IoAllocateWorkItem( pdevObject );
   IoQueueWorkItem( pWinccidev->pwork, TestWorkItemCallback, DelayedWorkQueue, (PVOID)pWinccidev );
#endif
   {
      struct aal_device  dev;
      aaldev_to_basedev( &dev ) = pdevObject;
      if( NULL == pWinccidev->m_workq ) {
         //         pWinccidev->m_workq = kosal_create_workqueue("WorkQueue", &dev);
         pWinccidev->m_workq = IoAllocateWorkItem( pdevObject );
         KOSAL_INIT_WORK( &( pWinccidev->task_handler ), Testtask_poller );
         kosal_queue_delayed_work( pWinccidev->m_workq, &( pWinccidev->task_handler ), 20000 );
      }

   }
#endif
   PTRACEOUT;
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
VOID CCIPDrvDeviceFileCleanup( _In_  WDFFILEOBJECT FileObject )
{
   UNREFERENCED_PARAMETER( FileObject );
#if 0
   WDFDEVICE  hDevice = WdfFileObjectGetDevice( FileObject );
   PWIN_CCIPDrv_DEVICE        pWinccidev = NULL;
   struct spl2_device *pspl2dev = NULL, *tmp = NULL;
   struct aal_device     * pdev = NULL;
   //      struct memmgr_session   *pmem_sess  = NULL;
   struct spl2_session     *sessp = NULL;
   struct aaldev_ownerSession *  pownerSess = NULL;
#if 0
   PDEBUG( "CCIPDrvDeviceFileCleanup() entered\n" );

   pWinccidev = CCIFdoGetCCIDev( hDevice );

   //   pspl2dev =  PWIN_CCIPDrv_DEVICE_TO_PCCIPDrv_DEVICE(pWinccidev);


   // Clean-up in case of dirty close
   if( NULL != puisess ) {

      // Release all devices since this device is as good as uidrv in Linux (HACK)
      kosal_list_for_each_entry_safe( pspl2dev, tmp, &g_device_list, m_session_list, struct spl2_device ){

         // Clean up after device
         pdev = spl2_dev_to_aaldev( pspl2dev );
         if( likely( pdev ) ) {

            pownerSess = dev_OwnerSession( pdev, kosal_get_pid() );
            if( NULL != pownerSess ) {
               sessp = ( struct spl2_session * )aalsess_pipHandle( pownerSess );

               // Free all memmory mapped
               if( NULL != sessp ) {

                  // Protect access to activesess
                  kosal_sem_get_user( spl2_dev_semp( pdev ) );

                  // If a session is active stop it
                  if( NULL != spl2_dev_activesess( pspl2dev ) ) {
                     // Wait for complete
                     spl2_trans_end( pspl2dev );
                  };
                  // Done accessing activesess
                  kosal_sem_put( spl2_dev_semp( pdev ) );

                  // Unmap CSR memory
                  if( NULL != sessp->m_csruvAddr ) {
                     if( NULL != sessp->m_csrmap ) {
                        MmUnmapLockedPages( sessp->m_csruvAddr, sessp->m_csrmap );
                        IoFreeMdl( sessp->m_csrmap );
                     }
                  }

                  // Unmap DSM memory
                  if( NULL != sessp->m_wsuvAddr ) {
                     if( NULL != sessp->m_wsmap ) {
                        MmUnmapLockedPages( sessp->m_wsuvAddr, sessp->m_wsmap );
                        IoFreeMdl( sessp->m_wsmap );
                     }
                  }

                  flush_all_wsids( sessp );
                  session_destroy( sessp );

               }// End NULL != sessp
               // Remove it from the owner list  
               if( dev_removeOwner( pdev, puisess->m_pid ) != aaldev_addowner_OK ) {

                  PERR( "failed to claim the device\n" );
               }
            }//End NULL != pownersession
         }// End likely(pdev)
      }// Kosal for

      // Unblock any waiting threads
      // TODO MAKE SURE EVENT QUEUE EMPTY BEFORE FREE
      if( puisess->m_waitq ) {
         kosal_wake_up_interruptible( puisess->m_waitq );
         puisess->m_waitq = NULL;
      }
      if( --numdevicesOpen == 0 ) {
         kosal_kfree( puisess, sizeof( struct uidrv_session ) );
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
VOID CCIPDrvDeviceFileClose( _In_  WDFFILEOBJECT FileObject )
{
   UNREFERENCED_PARAMETER( FileObject );
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
VOID CCIPDrvDeviceFileCreate( IN WDFDEVICE  hDevice,
                              IN WDFREQUEST  Request,
                              IN WDFFILEOBJECT  FileObject )
{
   UNREFERENCED_PARAMETER( hDevice );
   UNREFERENCED_PARAMETER( FileObject );
   UNREFERENCED_PARAMETER( Request );

#if 0
   PWIN_CCIPDrv_DEVICE        pWinccidev = NULL;
   struct spl2_device      *pspl2dev = NULL;
   struct aal_device     * pdev = NULL;

   UNREFERENCED_PARAMETER( Request );
   UNREFERENCED_PARAMETER( FileObject );
#if 0   
   PDEBUG( "CCIPDrvDeviceFileCreate() entered\n" );

   // Create the uisession  ALL HAC RIGHT NOW
   if( NULL == puisess ) {
      puisess = uidrv_session_create( kosal_get_pid() );
   }

   pWinccidev = CCIFdoGetCCIDev( hDevice );
   pspl2dev = PWIN_CCIPDrv_DEVICE_TO_PCCIPDrv_DEVICE( pWinccidev );

   pdev = spl2_dev_to_aaldev( pspl2dev );
#if 0 
   if( likely( pdev ) ) {
      aaldev_AddOwner_e ret;
      if( unlikely( ( ret = dev_addOwner( pdev,
         puisess->m_pid,
         puisess, // TODO MANIFEST ON OWNER NOT SUPPORTED YET
         &puisess->m_devicelist ) ) != aaldev_addowner_OK ) ) {

         PERR( "failed to claim the device\n" );
         WdfRequestComplete( Request, STATUS_DEVICE_DATA_ERROR );
         return;
      }
   }
#endif   
   // Count the device
   numdevicesOpen++;
#endif
   WdfRequestComplete( Request, STATUS_SUCCESS );
   //   PDEBUG("Device Owned by pid %d\n",puisess->m_pid);
#endif
}

