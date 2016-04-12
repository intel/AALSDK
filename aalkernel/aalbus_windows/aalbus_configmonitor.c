// Copyright (c) 2014-2015, Intel Corporation
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
#include "aalsdk/kernel/kosal.h"
#include "aalbus_session.h"
#include "aalbus_internal.h"
#include "aalsdk/kernel/aalbus_iconfigmonitor.h"
#include "aalbus_events.h"
#include "aalbus_poll.h"

#define MODULE_FLAGS AALRMS_DBG_MOD
EVT_WDF_DEVICE_FILE_CREATE    Bus_IResMon_FileCreate;
EVT_WDF_FILE_CLEANUP          Bus_IResMon_FileCleanup;
EVT_WDF_FILE_CLOSE            Bus_IResMon_FileClose;


kosal_list_head               gSessions;

typedef struct _AAL_BUS_CONFIGMONITOR_CONTEXT {

   PVOID   ControlData; // Store your control data here

} AAL_BUS_CONFIGMONITOR_CONTEXT, *PAAL_BUS_CONFIGMONITOR_CONTEXT;


WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(AAL_BUS_CONFIGMONITOR_CONTEXT,
                                    MonitorConfigGetContext)




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
   NTSTATUS                                status = STATUS_INVALID_PARAMETER;
   struct aalbus_configmonitor_message    *presp = NULL;
   WDFDEVICE                               hDevice = NULL;
   size_t                                  length = 0;
   WDFFILEOBJECT                           FileObject = NULL;
   PFILE_OBJECT                            pFile;
   struct aalbus_session                  *psess = NULL;
   UNREFERENCED_PARAMETER(OutputBufferLength);

   PAGED_CODE();
   PTRACEIN;


   KdPrint(("Bus_EvtConfigStateMonitorIoDeviceControl: 0x%p\n", hDevice));

   //
   // Get the session for this process
   //---------------------------------
  
   // Get the WDF File Object from the Request
   FileObject = WdfRequestGetFileObject( Request );
   if(NULL == FileObject){
      PDEBUG( "Failed to get WDF file object from request\n" );
      WdfRequestComplete( Request, STATUS_DEVICE_DATA_ERROR );
      return;
   }

   // Get the WDM File object so we can access the session
   pFile = WdfFileObjectWdmGetFileObject( FileObject );
   if(NULL == pFile){
      PDEBUG( "Failed to get file object from WDF File object\n" );
      WdfRequestComplete( Request, STATUS_DEVICE_DATA_ERROR );
      return;
   }

   // Get the session from the File object
   if(NULL == pFile->FsContext){
      PDEBUG( "Failed to get Sesion from File object\n" );
      WdfRequestComplete( Request, STATUS_DEVICE_DATA_ERROR );
      return;
   }
   else {
      psess = (struct aalbus_session   *)pFile->FsContext;
   }

   
   //
   // Process the command
   //--------------------
   switch (IoControlCode) {

   //--------------
   // Poll Function
   //--------------
   case IOCTL_CONFMON_POLL:
   {

	   // TODO - IF THE FLOW IS SUCH THAT WE CAN GET RE-ENTERED LOCK THE FOLLOWING

	   // 
      // If there is a request waiting something is wrong so wake it up 
      //   and replace it.
	   if (psess->m_pollobj){
		   PDEBUG("Poll Oops there was an outstanding poll\n");
		   kosal_wake_up_interruptible(&psess->m_pollobj);
	   }

	   // Save the session pointer in the request
	   WdfRequestSetInformation(Request, psess);

	   // Activate the Poll functionality.
	   //  If the poll is sucessful then the system
	   //  has been armed to send a notification if an event is available.
	   //  In this case the request must be saved in the session as a 
	   //  kosal_poll_object.  Once the poll object has been signaled as a 
	   //  result of an event it will be removed.
	   //
	   //  if the poll function returns FALSE that means the event 
	   //  queue is not empty and the poll object has been signaled.
	   //  In this case the request does not get saved.
	   if (true == aalbus_poll(AAL_WDFREQUEST_TO_POLLOBJ(Request), &psess->m_eventq)){
		   psess->m_pollobj = AAL_WDFREQUEST_TO_POLLOBJ(Request);
	   }

	   // Nothing more to do. IRP completion is handled in Poll function
	   return;
   }
   break;

   //-----------------------------------
   // Get next queued message descriptor
   // This will contain things like its
   // size and type
   //---------------------------------
   case IOCTL_CONFMON_GETMSG_DESC:
   {
      size_t                               prespsize  = 0;
      struct aalbus_configmonitor_message *presp      = NULL;
      size_t                               preqsize   = 0;
      struct aalbus_configmonitor_message *preq       = NULL;
      struct aal_q_item                   *pqitem     = NULL;

      // If there is no room for reply fail
      if( OutputBufferLength < sizeof( struct aalbus_configmonitor_message ) ){
         status = STATUS_DEVICE_DATA_ERROR;
         OutputBufferLength = 0;
         break;
      }

      // Get the output buffer for the response
      status = WdfRequestRetrieveOutputBuffer( Request, sizeof( struct aalbus_configmonitor_message ), (PVOID*)&presp, &prespsize );
      if(!NT_SUCCESS( status )) {
         PERR( "WdfRequestRetrieveOutputBuffer failed\n" );
         WdfRequestCompleteWithInformation( Request, status, (ULONG_PTR)0 );
         PTRACEOUT;
         return;
      }

      // Make sure there is a message to be had
      if( _aal_q_empty(&psess->m_eventq) ) {
         PDEBUG("No Message available\n");
         status = STATUS_RESOURCE_DATA_NOT_FOUND;
         break;
      }

      // Peek the head of the message queue
      pqitem = _aal_q_peek(&psess->m_eventq);
      if( NULL == pqitem ) {
         PERR("Corrupt event queue\n");
         status = STATUS_DEVICE_DATA_ERROR;
         break;
      }

      status = WdfRequestRetrieveInputBuffer( Request, sizeof( struct aalbus_configmonitor_message ), (PVOID*)&preq, &preqsize );
      if(!NT_SUCCESS( status )) {
         PERR( "WdfRequestRetrieveInputBuffer failed\n" );
         break;
      }

      // Copy the request parameters so they appear in response. (eg., TransactionID)
      memcpy( presp, preq, sizeof( struct aalbus_configmonitor_message ) );   

      // Return the type and total sizeof the message that will be returned
      presp->id = (aalbus_msgIDs_e)QI_QID( pqitem );
      presp->size = QI_LEN(pqitem);
      length = OutputBufferLength;
      PDEBUG("Getting Message Decriptor - size = %" PRIu64 "\n", preq->size);
      status = STATUS_SUCCESS;
   }
   break;


   //----------------------------------------------------
   // Get the next message off of the queue and 
   //  marshal it upstream.
   //----------------------------------------------------
   case IOCTL_CONFMON_GETMSG:
   {
      size_t                               prespsize = 0;
      struct aalbus_configmonitor_message *presp = NULL;
      size_t                               preqsize = 0;
      struct aalbus_configmonitor_message *preq = NULL;
      struct aal_q_item                   *pqitem = NULL;

      // If there is no room for reply fail
      if(OutputBufferLength < sizeof( struct aalbus_configmonitor_message )){
         status = STATUS_INVALID_PARAMETER;
         OutputBufferLength = 0;
         break;
      }

      // Get the output buffer for the response
      status = WdfRequestRetrieveOutputBuffer( Request, sizeof( struct aalbus_configmonitor_message ), (PVOID*)&presp, &prespsize );
      if(!NT_SUCCESS( status )) {
         PERR( "WdfRequestRetrieveOutputBuffer failed\n" );    
         break;
      }

      // Make sure there is a message to be had
      if(_aal_q_empty( &psess->m_eventq )) {
         PDEBUG( "No Message available\n" );
         status = STATUS_RESOURCE_DATA_NOT_FOUND;
         break;
      }

      // Get the request message
      pqitem = _aal_q_dequeue( &psess->m_eventq );
      if(NULL == pqitem) {
         PERR( "Corrupt event queue\n" );
         status = STATUS_DEVICE_DATA_ERROR;
         break;
      }

      status = WdfRequestRetrieveInputBuffer( Request, sizeof( struct aalbus_configmonitor_message ), (PVOID*)&preq, &preqsize );
      if(!NT_SUCCESS( status )) {
         PERR( "WdfRequestRetrieveInputBuffer failed\n" );
         break;
      }

      // Copy the request parameters so they appear in response. (eg., TransactionID)
      memcpy( presp, preq, sizeof( struct aalbus_configmonitor_message ) );

      if(0 != aalbus_process_configmon_event( psess, pqitem, presp, &OutputBufferLength )){
         PDEBUG( "Failed to oricess event\n" );
         status = STATUS_DEVICE_DATA_ERROR;
         break;
      }
      length = OutputBufferLength;
      status = STATUS_SUCCESS;
   } 
   break; // case  AALUID_IOCTL_GETMSG:


#if 0
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
#endif
   default:
      break; // default status is STATUS_INVALID_PARAMETER
   }

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
      WDF_FILEOBJECT_CONFIG            fileObjectConfig;

      DECLARE_CONST_UNICODE_STRING( ntDeviceName, AALBUS_CONFMON_SERVICE_NAME_STRING );
      DECLARE_CONST_UNICODE_STRING( symbolicLinkName, AALBUS_CONFMON_SERVICE_SYMBOLIC_NAME_STRING );

      PAGED_CODE();

      KdPrint(("Creating Control Device\n"));

      //
      //
      // In order to create a control device, we first need to allocate a
      // WDFDEVICE_INIT structure and set all properties.
      pInit = WdfControlDeviceInitAllocate(  WdfDeviceGetDriver(device),
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
      // Register File object callbacks
      //
      WDF_FILEOBJECT_CONFIG_INIT( &fileObjectConfig,
                                   Bus_IResMon_FileCreate,
                                   Bus_IResMon_FileClose,
                                   Bus_IResMon_FileCleanup
         );


      WdfDeviceInitSetFileObjectConfig(  pInit,
                                        &fileObjectConfig,
                                        WDF_NO_OBJECT_ATTRIBUTES);

      //
      // Specify the size of device context - I.e., private data to this object
      WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE( &controlAttributes,
                                                AAL_BUS_CONFIGMONITOR_CONTEXT);
      status = WdfDeviceCreate( &pInit,
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

      // Initialize session's lists, queues and sync objects
      kosal_list_init(&gSessions);

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




//=============================================================================
// Name: Bus_IResMon_FileCleanup
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
VOID Bus_IResMon_FileCleanup(_In_  WDFFILEOBJECT FileObject)
{

   struct aalbus_session * psess    = NULL; 
   PFILE_OBJECT            pFile    = NULL;
   WDFDEVICE               hDevice  = WdfFileObjectGetDevice(FileObject);
   
   PTRACEIN;

   pFile = WdfFileObjectWdmGetFileObject(FileObject);
   if (NULL == pFile){
      PDEBUG("Failed to get file from WDF File object\n");
      return;
   }

   if (NULL == pFile->FsContext){
      PDEBUG("Failed to get session from File \n");
      return;
   }

   // TODO LOCK THIS AGAINST POLLING SO THAT A MESSAGE POLLER CANNOT COME IN DURING DESTRUCTION
   psess = pFile->FsContext;
   pFile->FsContext = NULL;

   // Clean-up in case of dirty close
   if(NULL != psess){

      // Unblock any waiting threads
      // TODO MAKE SURE EVENT QUEUE EMPTY BEFORE FREE
      if (kosal_poll_object_is_valid(&psess->m_pollobj)) {
         kosal_wake_up_interruptible(&psess->m_pollobj);
         kosal_poll_object_consume(&psess->m_pollobj);
      }

      // Remove this session from the list
      kosal_list_del(&(psess->m_sessions));

      aalbus_destroySession(psess);


   }// End NULL !=puisess

}



//=============================================================================
// Name: Bus_IResMon_FileClose
// Description:  Called when the framework receives IRP_MJ_DEVICE_CONTROL
//               requests from the system.
// Interface: public
// Inputs: FileObject - A handle to a framework file object, which was 
//                      previously received by the driver's EvtDeviceFileCreate 
//                      callback function.
// Outputs: None.
// Comments: 
//=============================================================================
VOID Bus_IResMon_FileClose(_In_  WDFFILEOBJECT FileObject)
{

   PFILE_OBJECT pFile = NULL;

   PTRACEIN;
 
   pFile = WdfFileObjectWdmGetFileObject(FileObject);
   if (NULL == pFile){
      PDEBUG("Failed to get file from File object\n");
      return;
   }

   if (NULL != pFile->FsContext){
      PDEBUG("File context not NULL. Check for failure to release session.");
   }

}

//=============================================================================
// Name: Bus_IResMon_FileCreate
// Description:  Called when the framework receives IRP_MJ_DEVICE_CONTROL
//               requests from the system.
// Interface: public
// Inputs: hDevice - Handle to the framework device object that is associated
//                   with the I/O request.
//         Request - Handle to a framework request object.
//         FileObject - A handle to a framework file object, which was 
//                      previously received by the driver's EvtDeviceFileCreate 
//                      callback function.
// Outputs: None.
// Comments: 
//=============================================================================
VOID Bus_IResMon_FileCreate( IN WDFDEVICE       hDevice,
                             IN WDFREQUEST      Request,
                             IN WDFFILEOBJECT   FileObject)
{

   struct aalbus_session * psess = NULL;
   PFILE_OBJECT pFile = NULL;

   UNREFERENCED_PARAMETER(Request);

   PTRACEIN;

   pFile = WdfFileObjectWdmGetFileObject(FileObject);
   if (NULL == pFile){
      PDEBUG("Failed to get file from File object\n");
      WdfRequestComplete(Request, STATUS_DEVICE_DATA_ERROR);
      return;
   }

   if (NULL != pFile->FsContext){
      PDEBUG("Failed to get file from File object\n");
      WdfRequestComplete(Request, STATUS_DEVICE_DATA_ERROR);
   }
   psess = aalbus_createSession(kosal_get_pid());
   
   if (NULL == psess){

      WdfRequestComplete(Request, STATUS_DEVICE_DATA_ERROR);
      return;
   }

   // Add this session onto the queue
   kosal_list_add(&(psess->m_sessions), &gSessions);
   
   pFile->FsContext = psess;

   // Add the session to the global session list.
   WdfRequestComplete(Request, STATUS_SUCCESS);

   PDEBUG("Config Montor Opened by pid %d\n", kosal_get_pid());

}


//=============================================================================
// Name: Bus_IResMon_SendEvent
// Description: Send a Resource Monitor Event to all waiting Sessions
// Interface: public
// Inputs:
// Outputs: none.
// Comments:
//=============================================================================
btInt Bus_IResMon_SendEvent( struct aal_q_item *eventp )
{
   btInt                 ret = 0;
   
   pkosal_list_head      lentry =  kosal_list_next( &gSessions );

   struct aalbus_session *psess = kosal_list_entry( lentry, struct aalbus_session, m_sessions );

   PTRACEIN;
   ASSERT( NULL != psess );

   if( NULL != eventp ) {

      if( kosal_sem_get_user_alertable( &psess->m_sem ) ) { /* FIXME */ }
      PNOTICE( "Waking Up Application with event\n" );

      // Enqueue the completion event
      _aal_q_enqueue( eventp, &psess->m_eventq );

      // Unblock select() calls.
      if( psess->m_pollobj ) {
         kosal_wake_up_interruptible( &psess->m_pollobj );
         psess->m_pollobj = NULL;
      }
      kosal_sem_put( &psess->m_sem );
   }

   PTRACEOUT_INT( ret );
   return ret;
}


