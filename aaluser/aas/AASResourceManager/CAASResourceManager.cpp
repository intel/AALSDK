// Copyright (c) 2006-2015, Intel Corporation
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
/// @file CAASResourceManager.cpp
/// @brief Implement ResourceManager Class defined in CAASResourceManager.h.
///        Originally from AASResourceManager.cpp, by Henry Mitchel.
/// @ingroup ResMgr
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Henry Mitchel, Intel Corporation.
///          Joseph Grecco, Intel Corporation.
///
/// HISTORY: 
/// WHEN:          WHO:     WHAT:
/// 08/31/2008     HM       Initial version
/// 09/12/2008     HM       Major checkin
/// 09/12/2008     HM       Added configuration update message support
/// 10/05/2008     HM       Converted to use AALLogger
/// 10/07/2008     HM       Merge with new structures used by kernel
/// 10/07/2008     HM       Put in "m_mydevice" temporary kludge to provide
///                            0.7 functionality
/// 11/09/2008     JG/HM    Initial version of proper handling for config updates
/// 11/13/2008     HM       Further along the config update path, not done yet
/// 11/18/2008     HM       Fix 64 to 128-bit GUID conversion, fixed backdoor
///                            RequestDevice code
/// 11/25/2008     HM       Large merge
/// 11/26/2008     HM       Fixed segfault caused by changing position of
///                            pIoctlReq cleanup code. Tweaked comments,
///                            removed #if 0/#endif around RequestDevice code
/// 11/28/2008     HM       Fixed legal header
/// 11/30/2008     HM       Added code for globalRMFileDescriptor
/// 12/20/2009     HM       Changed some of the DEBUG statements to VERBOSE
/// 01/04/2009     HM       Updated Copyright
/// 01/15/2009     HM       Cleared IoctlReq to 0 before use, so that if
///                            interrupted, the size field for the payload
///                            will be null. Fixes a GPF on Ctrl-C.
/// 01/17/2009     HM       Moved debug output in DoRequestDevice so that a
///                            NULL payload will not blow the logger.
/// 02/09/2009     HM       Set Send_AALRMS_Msg so that EINVAL will not return
///                            an error. EINVAL is expected now if sending a
///                            response to a request that has disappeared due
///                            to client application abort (e.g. Ctrl-C).
/// 02/15/2009     HM       Add keyRegNumAllocated and keyRegAFU_ShareMax
///                            initialization to DoConfigUpdate()
/// 02/17/2009     HM       DoConfigUpdate now builds the Instance Record Map
///                            instead of using the database. Keeps instance
///                            records out of the database.
/// 02/19/2009     HM       Implemented Instance Record allocation tracking
///                            state machine in DoRequestDevice
/// 02/20/2009     HM       Added dual error handling in Send_AALRMS_Message
///                            so could add backtracking of Instance Record
///                            allocation in DoRequestDevice
/// 05/11/2009     HM       Fix to ActivateDevice code in DoConfigUpdate() when
///                            a board is seen.
/// 10/19/2009     JG       Added support for the maxOwners attribute
/// 09/28/2010     HM       #if'd out board check on Configuration Update
///                         Will need to do something similar/more complex
///                            on detection of MAFU, so detection code left in.@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include "aalsdk/AALTypes.h"               // btUnsigned32bitInt, etc.
#include "aalsdk/AALLoggerExtern.h"        // theLogger, a reference to a singleton ILogger interface
#include "aalsdk/rm/CAASResourceManager.h" // Also brings in the skeleton, which brings in the database, the proxy, and <string>
                                           // and <aas/kernel/aalrm_server.h>, the definition of aalrm_ioctlreq
                                           // Also defines debug LogMask_t bitmasks for detailed debugging of Resource Manager
#include "aalsdk/kernel/KernelStructs.h"   // various operator<<
#include "aalsdk/utils/ResMgrUtilities.h"  // string, name, and GUID inter-conversion operators


BEGIN_NAMESPACE(AAL)

CResMgr::CResMgr(const CResMgr & ) {/*empty*/}
CResMgr & CResMgr::operator=(const CResMgr & ) { return *this; }


//=============================================================================
// Name:          CResMgr::CResMgr
// Description:   Constructor
//                All parameters are defaulted in the declaration
// Interface:     public
// Inputs:        btBool dDebug, true for run-time debugging, defaults to true
// Outputs:       none
// Comments:
//=============================================================================
CResMgr::CResMgr(NamedValueSet *pOptArgs, const std::string &sDevName)
   :  m_sResMgrDevName  (sDevName),
      m_pRegDBSkeleton  (NULL),
      m_pIoctlReq       (NULL),
      m_fdServer        (-1),
      m_bIsOK           (false),
      m_pOptArgs        (pOptArgs),
      m_sDatabasePath   (),               // Will be initialized below
      m_state           (eCRMS_Starting),
      m_InstRecMap      (),
      m_mydevice        (0)
{
   // Set global file handle
   if (-1 != globalRMFileDescriptor) {
      AAL_ERR(LM_ResMgr,"CResMgr::CResMgr sees globalRMFileDescriptor set, will reset and lose file\n");
      globalRMFileDescriptor = -1;
   }

   // Have a path, get the database up
   m_pRegDBSkeleton = new(std::nothrow) RegDBSkeleton(m_pOptArgs);
   if (m_pRegDBSkeleton) {
      AAL_DEBUG(LM_ResMgr,"CResMgr created a RegDBSkeleton at " << static_cast<void*>(m_pRegDBSkeleton) << std::endl);
   } else {
      AAL_ERR(LM_ResMgr,"CResMgr could not create a RegDBSkeleton\n");
      goto getout_1;
   }

   // Get a globally usable (or backup) ioctlreq. Not currently used (2008.09.11)
   m_pIoctlReq = new(std::nothrow) struct aalrm_ioctlreq;
   if( m_pIoctlReq ){
      AAL_DEBUG(LM_ResMgr,"CResMgr created a aalrm_ioctlreq at " << static_cast<void*>(m_pIoctlReq) << std::endl);
   } else {
      AAL_ERR(LM_ResMgr,"CResMgr could not create an aalrm_ioctlreq\n");
      goto getout_2;
   }

   globalRMFileDescriptor = m_fdServer = open (m_sResMgrDevName.c_str(), O_RDWR);
   if (m_fdServer >= 0){                                      // success
      AAL_DEBUG(LM_ResMgr,"CResMgr opened file " << m_sResMgrDevName << " as file " << m_fdServer << std::endl);
   } else {
      int saverr = errno;
      AAL_ERR(LM_ResMgr,"CResMgr open of " << m_sResMgrDevName << " failed with error code " << saverr
                                           << ". Reason string is: " << pAALLogger()->GetErrorString(saverr) << std::endl);
      goto getout_3;
   }

   // Don't use one of these objects unless bIsOK returns true
   m_bIsOK = m_pRegDBSkeleton->IsOK();
   m_state = eCRMS_Running;
   return;

getout_3:
   delete m_pIoctlReq; m_pIoctlReq = NULL;
getout_2:
   delete m_pRegDBSkeleton; m_pRegDBSkeleton = NULL;
getout_1:
   return;
}  // end of CResMgr::CResMgr

//=============================================================================
// Name:          CResMgr::~CResMgr
// Description:   Destructor
// Interface:     public
// Inputs:
// Outputs:       none
// Comments:
//=============================================================================
CResMgr::~CResMgr()
{
   // TODO: handle popping out of the poll that GetMsg will be in, possibly.
   // As of right now, things are pretty much single threaded, so the only
   // way to get here is for the main thread to decide to be here, which
   // means it is not in the GetMsg poll. However, in the future, should there
   // need to be an asynchronous ending (e.g. from SIGHUP), then have to handle
   // popping out of the poll here via a special ioctl.
   if( m_fdServer >= 0 ) {
      close (m_fdServer);
      m_fdServer = globalRMFileDescriptor = -1;
   }
   if( m_pIoctlReq ){
      delete m_pIoctlReq;
      m_pIoctlReq = NULL;
   }
   if( m_pRegDBSkeleton ){
      delete m_pRegDBSkeleton;
      m_pRegDBSkeleton = NULL;
   }
} // end of CResMgr::~CResMgr

//=============================================================================
// Name:          CResMgr::GetMsg
// Description:   Retrieve an aalrm_ioctlreq from the device
// Interface:     public
// Inputs:        fdServer - file descriptor of the Server (Resource Manager, Database, etc.) - NOT CHECKED FOR VALIDITY
//                m_pIoctlReq - ioctl structure - NOT CHECKED FOR VALIDITY
// Outputs:       *m_pIoctlReq passed in is overlaid with the new information
//                The payload will be either NULL or a valid ptr to char that has been new'd, and must
//                   be free'd by delete [] of a char*
//                The message must be responded to, or the message will sit in kernel space forever
//                The req_hndl must be used in the response message
// Returns:       0 for success.
//                Positive value is an errno.
//                Negative is completely unexpected error (inverted)
// Comments:      As an internal function, the parameters are NOT CHECKED FOR VALIDITY
//                The typical call setup is:
//                   int m_fdServer = open();
//                   struct aalrm_ioctlreq m_pIoctlReq;
//                   while (!GetMsg()) {}
//                The typical flow if there is a message waiting is:
//                   first ioctl succeeds, message is retrieved and returned
//                The typical flow if there is NOT a message waiting is:
//                   first ioctl fails with -1
//                   errno == EAGAIN, so the poll is entered
//                   poll returns, loop occurs,
//                   first ioctl succeeds (since a message is waiting)
//                      message is retrieved and returned
//=============================================================================

int CResMgr::Get_AALRMS_Msg(int fdServer, struct aalrm_ioctlreq *pIoctlReq)
{
   static unsigned ctMessages = 0;                                // track messages
   btVirtAddr      pBuf       = NULL;                             // data buffer, passed on, so not an auto_ptr
   memset(pIoctlReq, 0, sizeof(struct aalrm_ioctlreq));           // ensure that on an interrupted return, size is 0

   // loop either 0 or 1 times, typically. Exit directly from the loop. This is the easiest way to handle the numerous error cases.
   do {
      // check to see if something is already available, this is a peek, not a retrieval
      int iRetIoctl = ioctl (fdServer, AALRM_IOCTL_GETMSG_DESC, pIoctlReq);

      // 3 cases here:
      //    iRetIoctl = 0 means there is a message waiting, optimize for this as it indicates highest load
      //    iRetIoctl = -1 means some kind of error
      //       errno == EAGAIN means there is no message waiting, need to wait in a poll
      //       errno == anything but EAGAIN means there really was an error, return it

      if (0 == iRetIoctl) {               // iRetIoctl = 0 means there is a message waiting, get it
         AAL_DEBUG(LM_ResMgr, "Get_AALRMS_Msg: AALRM_IOCTL_GETMSG_DESC succeeded:\n\tType = " << std::showbase <<
                              pIoctlReq->id << "\n\tPayload length = " << std::hex << pIoctlReq->size <<
                              " " << std::dec << pIoctlReq->size << std::endl);
         // Get a payload buffer if required
         if (pIoctlReq->size) {                                   // do we need a buffer?
            pBuf = reinterpret_cast<btVirtAddr>(new(std::nothrow) btByte[pIoctlReq->size]); // yes we do, get a buffer
            if (!pBuf) {
               return ENOMEM;
            }
            pIoctlReq->payload = pBuf;                            // set the pointer
         }
         else {
            pIoctlReq->payload = NULL;                            // don't need no stinking buffer
         }
         // Get the message, in its entirety, from the queue. This has to be called even if the
         //    buffer is null, because this call actually removes the message from the queue
         if (ioctl (fdServer, AALRM_IOCTL_GETMSG, pIoctlReq) == 0) {
            AAL_DEBUG(LM_ResMgr, "Get_AALRMS_Msg:[" << ctMessages++ << "] " << *pIoctlReq << std::endl);
            return 0;                                             // Success! NORMAL TERMINATION
         }
         else {                                                   // ioctl failed
            int saverr = errno;
            if (pBuf) {
               delete [] pBuf;
               pBuf = NULL;
            }
            pIoctlReq->payload = NULL;

            if (EAGAIN == saverr) {       // Possible that peek succeeded, but then the message disappeared
               AAL_INFO(LM_ResMgr,"AAL Resource Manager Get_AALRMS_Msg::GetMsg did not find an expected message."
                     " Possibly due to a client dying unexpectedly.\n");
               continue;                  // so just log and ignore
            } else {
               return saverr;
            }
         }
      }  // if (0 == iRetIoctl)
      else {                              // iRetIoctl != 0 means some kind of error
         if (EAGAIN == errno) {           // errno == EAGAIN means there is no message waiting, need to wait in a poll
            const int numHandles  = 1;    // there is only one file
            const int index       = 0;    // and its index is 0
            struct pollfd           pollfds[numHandles];
            pollfds[index].fd     = fdServer;
            pollfds[index].events = POLLPRI;

            AAL_INFO(LM_ResMgr,"Get_AALRMS_Msg::GetMsg polling for a message\n");
            int iRetPoll          = poll( pollfds, numHandles, -1 /* infinite timeout */);
            AAL_INFO(LM_ResMgr,"Get_AALRMS_Msg::GetMsg poll returned with code " << iRetPoll << std::endl);

            // 4 cases here:
            //    iRetPoll == 1 means the poll returned on the file, need to get the message
            //    iRetPoll == 0 means timeout, very unexpected since passed in -1 for timeout
            //    iRetPoll < 0 means error
            //    iRetPoss > 1 means something is complete hosed
            if (1 == iRetPoll) {          // happiness, something happened on the pollfds[0]
            }                             // do nothing but loop so as to get the message
            else if (iRetPoll < 0) {      // classic error on the poll
               return errno;
            }
            else if (0 == iRetPoll) {     // timeout, very strange
               return ETIME;              // overload errno
            }
            else {                        // must be a positive integer > numHandles
               return -iRetPoll;          // negate the return value so it is negative
            }
         }  // if (EAGAIN == errno)
         else {                           // errno == anything but EAGAIN means there was really an error, return it
            return errno;
         } // else (EAGAIN == errno)
      }  // else (0 == iRetIoctl)
   } while (true);                        // exit from within the loop, not really an infinite loop
   return 0;
}  // end of CResMgr::GetMsg()

//=============================================================================
// Name:          CResMgr::ParseMsg
// Description:   Parse an aalrm_ioctlreq by its id
// Interface:     public
// Inputs:        m_pIoctlReq - ioctl structure - NOT CHECKED FOR VALIDITY
// Outputs:
// Returns:       0 if success, else non-zero error (TBD)
// Comments:      Vectors to the correct parser based on id
//=============================================================================
int CResMgr::Parse_AALRMS_Msg(int fdServer, struct aalrm_ioctlreq *pIoctlReq)
{
   int Retval;

   switch ( pIoctlReq->id ) {
      case reqid_URMS_RequestDevice: {
         Retval = DoRequestDevice( fdServer, pIoctlReq);
         break;
      } // End of case reqid_URMS_ReleaseDevice:
#if 0 //TODO DEPRECATED
      case reqid_URMS_ReleaseDevice: {
         Retval = DoReleaseDevice( fdServer, pIoctlReq);
         break;
      } // End of case reqid_ReleaseDevice:
#endif
      case reqid_RS_Registrar: {
         Retval = DoRegistrar( fdServer, pIoctlReq);
         break;
      } // end of case reqid_Registrar:

      case evtid_KRMS_ConfigUpdate: {
         Retval = DoConfigUpdate( fdServer, pIoctlReq);
         break;
      } // End of case evtid_KRMS_ConfigUpdate:

      case reqid_Shutdown: {
         Retval = DoShutdown( fdServer, pIoctlReq);
         break;
      } // End of case evtid_KRMS_ConfigUpdate:

      case reqid_Restart: {
         Retval = DoRestart( fdServer, pIoctlReq);
         break;
      } // End of case evtid_KRMS_ConfigUpdate:

      default: {
         AAL_ERR(LM_ResMgr, "CResMgr::Parse_AALRMS_Msg: unknown pIoctlReq->id "
               << pIoctlReq->id << std::endl);
         Retval = EINVAL; // Invalid argument
         break;
      }
   } // End of switch (pIoctlReq->id)

   return Retval;
} // end of CResMgr::Parse_AALRMS_Msg

//=============================================================================
// Name:          CResMgr::SendMsg
// Description:   Put an aalrm_ioctlreq to a file
// Interface:     public
// Inputs:        fdServer - file index - NOT CHECKED FOR VALIDITY
//                pIoctlReq - ioctl structure - NOT CHECKED FOR VALIDITY
//                pRealRetVal - pointer to place to put real return value. When
//                   error processing is carried out, some errors are masked and
//                   a fake return is provided through the return code. This is
//                   provided for those functions that need the REAL return code.
//                   Since it defaults to NULL, it is optional.
// Outputs:
// Returns:       0 for Success, errno for failure, with perror having already
//                been called.
// Comments:      Written this way rather than using member variables because
//                this routine will likely need to be called from various
//                environments and needs to be flexible
//=============================================================================
int CResMgr::Send_AALRMS_Msg(int fdServer, struct aalrm_ioctlreq *pIoctlReq, int *pRealRetVal)
{
   AAL_DEBUG(LM_ResMgr, "CResMgr::Send_AALRMS_Msg:" << *pIoctlReq);

   int RealRetVal = 0;     // default response is no error
   int FakeRetVal = 0;     // fakeout response if needed

   if (ioctl (fdServer, AALRM_IOCTL_SENDMSG, pIoctlReq) == -1){
      RealRetVal = errno;
      if (EINVAL == RealRetVal) {
         AAL_WARNING(LM_ResMgr,"AAL Resource Manager CResMgr::Send_AALRMS_Msg failed"
               " due to invalid argument (EINVAL). Most likely this is due to a client application"
               " shutting down unexpectedly.\n");
         FakeRetVal = 0;
      } else {
         AAL_ERR(LM_ResMgr,"CResMgr::Send_AALRMS_Msg failed, reason code: "
               << pAALLogger()->GetErrorString(RealRetVal) );
         FakeRetVal = RealRetVal;
      }
   }

   if (pRealRetVal) *pRealRetVal = RealRetVal;
   return FakeRetVal;

}  // end of CResMgr::SendMsg


//=============================================================================
// Name:          CResMgr::EnableConfigUpdates
// Description:   Send an Ioctl that turns on configuration update messages
// Interface:     public
// Inputs:        fdServer - file index - NOT CHECKED FOR VALIDITY
//                pIoctlReq - ioctl structure - NOT CHECKED FOR VALIDITY
// Outputs:
// Returns:       0 for Success, errno for failure, with perror having already
//                been called.
// Comments:      Written this way rather than using member variables because
//                this routine will likely need to be called from various
//                environments and needs to be flexible
//=============================================================================
int CResMgr::EnableConfigUpdates(int fdServer, struct aalrm_ioctlreq *pIoctlReq)
{
   AAL_DEBUG(LM_ResMgr,"CResMgr::EnableConfigUpdates called\n");
   memset(pIoctlReq, 0, sizeof(struct aalrm_ioctlreq));
   pIoctlReq->req_handle = NULL;
   pIoctlReq->id = reqid_KRMS_SetConfigUpdates;
   return Send_AALRMS_Msg(fdServer, pIoctlReq);
}  // end of CResMgr::SendMsg

//=============================================================================
// Name:          CResMgr::DoRequestDevice
// Description:   Handle reqid_URMS_RequestDevice for the parser
// Interface:     public
// Inputs:        fdServer - file index - NOT CHECKED FOR VALIDITY
//                pIoctlReq - ioctl structure - NOT CHECKED FOR VALIDITY
// Outputs:
// Returns:       0 for Success, errno for failure, with perror having already
//                been called.
// Comments:
//=============================================================================
int CResMgr::DoRequestDevice(int fdServer, struct aalrm_ioctlreq *pIoctlReq)
{
   AAL_DEBUG(LM_ResMgr,"CResMgr::DoRequestDevice: reqid_URMS_RequestDevice seen\n");

   // Assume failure, makes all the failure scenarios simpler
   pInstRec_t pInstRec(NULL);

   pIoctlReq->result_code = rms_resultErrno;
   pIoctlReq->id = rspid_URMS_RequestDevice;
   if (pIoctlReq->size) {   // There is a payload, so retrieve the NVS and take appropriate action

      // Get the Manifest from the IoctlReq
      NamedValueSet nvsManifest(pIoctlReq->payload, pIoctlReq->size);

      // Compute the Goal Records, manifest in, list of goal records out
      nvsList listGoalRecords;
      NamedValueSet nvsGoal;

      if (ComputeGoalRecords( nvsManifest, listGoalRecords) &&
              GetPolicyResults( listGoalRecords, nvsGoal)) {
         // if success, have to return various things:

         // Get the handle from the GoalRecord NVS, which is a btObjectType, which is a void*
         // TODO: this will have to change to handle arrays of Handles

         if ( ENamedValuesOK == nvsGoal.Get( keyRegHandle, &pIoctlReq->res_handle)) {
            // All success now, no more failure scenarios

            // Get the Instance Record index from the goal record and use it to increment NumAllocated in the Instance Record
            // This is only for real Instance Records. AFU records with no device address should not be incremented.
            // TODO: Give AFU records real device addresses
            btNumberKey index;
            if ( (ENamedValuesOK == nvsGoal.Get( keyRegDeviceAddress, &index)) &&
                 (m_InstRecMap.Get( index, &pInstRec)) ) {
               pInstRec->IncrementAllocations();               // The equivalent decrement is in DoUpdateConfig, devOwnerRemoved
                                                               // and also in failure cases below.
            }

            pIoctlReq->result_code = rms_resultOK;             // This payload is valid

            // Clean out the old payload, note that size > 0 or we would not be here
            delete[] pIoctlReq->payload;  // matches allocator in Get_AALRMS_Msg

            // Write in the new payload, which is just a serialized copy of nvsGoal
            std::string s(nvsGoal);
            pIoctlReq->size = s.length();
            pIoctlReq->payload = reinterpret_cast<btVirtAddr>(new btByte[pIoctlReq->size]);
            BufFromString( pIoctlReq->payload, s);
         }
         else {
            // Error, no handle found, just return original payload with failure indication
            AAL_ERR(LM_ResMgr, "CResMgr::DoRequestDevice, Goal Record had no Handle, Request failed\n");
         }

         // Show the returned Goal Record
         AAL_DEBUG(LM_ResMgr,"CResMgr::DoRequestDevice: Goal Record being returned:" <<
               "\nAs a string:\n" << reinterpret_cast<btByte *>(pIoctlReq->payload) );
      }
      else {
         // Expecting that ComputeGoalRecords and/or GetPolicyResults will already have issued error messages
         // Leave the payload alone and just return error (set above by pIoctlReq->result_code = rms_resultErrno;)
      }
   }  // if (pIoctlReq->size)

   // No Payload, either error or backdoor 0
   else {
      // This is the correct response once the initial Backdoor is DEPRECATED, which means just delete this whole block

      // Baseline backdoor, TODO: DEPRECATE this
      pIoctlReq->id = rspid_URMS_RequestDevice;
      pIoctlReq->res_handle = m_mydevice;
      if (m_mydevice) { // If non-0, it has been set
         pIoctlReq->result_code = rms_resultOK;
      }
      else {
         pIoctlReq->result_code = rms_resultErrno;
      }

      // Null payload response debug
      AAL_DEBUG(LM_ResMgr,"CResMgr::DoRequestDevice: No payload, no Goal Record being returned." <<
            "\nres_handle:  " << pIoctlReq->res_handle <<
            "\nresult_code: " << pIoctlReq->result_code << std::endl);

   } // else if (pIoctlReq->size)

   // Send the message
   int RealRetVal;
   int FakeRetVal = Send_AALRMS_Msg(fdServer, pIoctlReq, &RealRetVal);

   // If there was an error, need to decrement the number of allocations
   if (RealRetVal && pInstRec) {
      AAL_DEBUG(LM_ResMgr,"CResMgr::DoRequestDevice: Send_AALRMS_Message Error causes decrement of Allocation.\n");
      pInstRec->DecrementAllocations();
   }

   // Return the filtered return value from SendMessage
   return FakeRetVal;
}

//=============================================================================
// Name:          CResMgr::DoReleaseDevice
// Description:   Handle reqid_URMS_ReleaseDevice for the parser
// Interface:     public
// Inputs:        fdServer - file index - NOT CHECKED FOR VALIDITY
//                pIoctlReq - ioctl structure - NOT CHECKED FOR VALIDITY
// Outputs:
// Returns:       0 for Success, errno for failure, with perror having already
//                been called.
// Comments:
//=============================================================================
int CResMgr::DoReleaseDevice(int fdServer, struct aalrm_ioctlreq *pIoctlReq)
{
   AAL_DEBUG(LM_ResMgr,"CResMgr::DoReleaseDevice: reqid_URMS_ReleaseDevice seen\n");

   // TODO: Actual resource management call, this is bogo code

   pIoctlReq->res_handle = (void*) 4;
   pIoctlReq->result_code = rms_resultErrno;
   pIoctlReq->size = 0;
   AAL_DEBUG(LM_ResMgr, "Responding with:\n"
         << "\tResponse Handle: " << pIoctlReq->req_handle
         << "\tDevice Handle:   " << pIoctlReq->res_handle
         << "\tResult Code:     " << pIoctlReq->result_code
         << std::endl);

   // Respond
   return Send_AALRMS_Msg(fdServer, pIoctlReq);
}

//=============================================================================
// Name:          CResMgr::DoRegistrar
// Description:   Handle reqid_RS_Registrar for the parser
// Interface:     public
// Inputs:        fdServer - file index - NOT CHECKED FOR VALIDITY
//                pIoctlReq - ioctl structure - NOT CHECKED FOR VALIDITY
// Outputs:
// Returns:       0 for Success, errno for failure, with perror having already
//                been called.
// Comments:
//=============================================================================
int CResMgr::DoRegistrar(int fdServer, struct aalrm_ioctlreq *pIoctlReq)
{
   AAL_DEBUG(LM_ResMgr,"CResMgr::DoRegistrar: reqid_Registrar seen\n");
   int Retval;

   if (pIoctlReq->size) { // only do anything if there is something to do

      // extract the RegistrarCmdResp_t
      pRegistrarCmdResp_t pRCR =
            reinterpret_cast<pRegistrarCmdResp_t> (pIoctlReq->payload);
      AAL_DEBUG(LM_ResMgr,"CResMgr::DoRegistrar: RECEIVED RegistrarCmdResp is:\n" << *pRCR << std::endl);

      // execute the database command, collect the response in pResp
      // TODO: use auto_ptr here, see how it affects other calls using the pointer type
      pRegistrarCmdResp_t pResp = m_pRegDBSkeleton->ParseCommand(pRCR);
      if (NULL == pResp) {
         AAL_DEBUG(LM_ResMgr,"CResMgr::DoRegistrar: RESPONSE RegistrarCmdResp is NULL\n");
      } else {
         AAL_DEBUG(LM_ResMgr,"CResMgr::DoRegistrar: RESPONSE RegistrarCmdResp is:\n" << *pResp << std::endl);
      }

      // Done with the input payload, free it
      if (pIoctlReq->payload)
         delete[] pIoctlReq->payload; // matches allocation in Get_AALRMS_Msg

      // Setup static fields in response IoctlReq
      pIoctlReq->id = rspid_RS_Registrar;

      // Setup response-specific  fields
      if (pResp) {
         pIoctlReq->size = pResp->LengthOfStruct;
         pIoctlReq->payload = reinterpret_cast<btVirtAddr>(pResp);
         pIoctlReq->result_code = rms_resultOK;
      } else {
         // valid to have no return, but still have to respond to the kernel so it can clear itself
         pIoctlReq->size = 0;
         pIoctlReq->payload = NULL;
         pIoctlReq->result_code = rms_resultOK;
      }

      // Send the response
      Retval = Send_AALRMS_Msg(fdServer, pIoctlReq);

      // Delete the (now sent) response
//       RegistrarCmdResp_Destroy(pResp); // Now handled in top loop

   } else { // No buffer provided by client, still have to clean up
      AAL_DEBUG(LM_ResMgr,"CResMgr::DoRegistrar:reqid_Registrar sees null payload. Invalid parameter. Sending clear message.\n");
      pIoctlReq->id = rspid_RS_Registrar;
      pIoctlReq->size = 0;
      pIoctlReq->payload = NULL;
      pIoctlReq->result_code = rms_resultOK;
      Send_AALRMS_Msg(fdServer, pIoctlReq);
      Retval = EINVAL;
   }
   return Retval;
} // CResMgr::DoRegistrar

//=============================================================================
// Name:          CResMgr::DoShutdown
// Description:   Handle reqid_URMS_RequestDevice for the parser
// Interface:     public
// Inputs:        fdServer - file index - NOT CHECKED FOR VALIDITY
//                pIoctlReq - ioctl structure - NOT CHECKED FOR VALIDITY
// Outputs:
// Returns:       0 for Success, errno for failure, with perror having already
//                been called.
// Comments:
//=============================================================================
int CResMgr::DoShutdown(int fdServer, struct aalrm_ioctlreq *pIoctlReq)
{
   AAL_DEBUG(LM_ResMgr,"CResMgr::DoShutdown\n");
   m_pRegDBSkeleton->Database()->Close();
   m_state = eCRMS_Stopping;                 // Signal main loop to shut down
   // TODO - DoShutdown code
   return 0;
} // CResMgr::DoShutdown

//=============================================================================
// Name:          CResMgr::DoRestart
// Description:   Handle reqid_URMS_RequestDevice for the parser
// Interface:     public
// Inputs:        fdServer - file index - NOT CHECKED FOR VALIDITY
//                pIoctlReq - ioctl structure - NOT CHECKED FOR VALIDITY
// Outputs:
// Returns:       0 for Success, errno for failure, with perror having already
//                been called.
// Comments:
//=============================================================================
int CResMgr::DoRestart(int fdServer, struct aalrm_ioctlreq *pIoctlReq)
{
   AAL_DEBUG(LM_ResMgr,"CResMgr::DoRestart - NOT YET IMPLEMENTED\n");
   // TODO - DoRestart code
   return 0;
} // CResMgr::DoRestart

//=============================================================================
// Name:          CResMgr::DoConfigUpdate
// Description:   Handle evtid_KRMS_ConfigUpdate for the parser
// Interface:     public
// Inputs:        fdServer - file index - NOT CHECKED FOR VALIDITY
//                pIoctlReq - ioctl structure - NOT CHECKED FOR VALIDITY
// Outputs:
// Returns:       0 for Success, errno for failure, with perror having already
//                been called.
// Comments:      2/15/2009 - adding a state machine to instance record
//                   keyRegAFU_ShareMax defaults to 1
//                   keyRegAFU_NumAllocated initialized to 0,
//                      increments on allocation, decrements
//                      on config update Device Removed
//=============================================================================
int CResMgr::DoConfigUpdate(int fdServer, struct aalrm_ioctlreq *pIoctlReq)
{
   AAL_DEBUG(LM_ResMgr,"CResMgr::DoConfigUpdate: evtid_KRMS_ConfigUpdate seen\n");
   int Retval = 0;

   struct aalrms_configUpDateEvent *pcfgUpDate =
         (struct aalrms_configUpDateEvent *) pIoctlReq->payload;

   if (pcfgUpDate) {
      krms_cfgUpDate_e     id     = pcfgUpDate->id;

      AAL_DEBUG(LM_ResMgr,"CResMgr::DoConfigUpdate: Event Type: " << id <<
            "\n\tPID:         " << pcfgUpDate->pid << "\n" << pcfgUpDate->devattrs << std::endl);

      // Record the device handle - used for BackDoor 0, which just records a handle and
      // returns it if there is no Request Device Manifest at all
      if (krms_ccfgUpdate_DevAdded == id) {
         m_mydevice = pcfgUpDate->devattrs.Handle;
      }

      //=======================================================================
      // Find the existing Instance Record in the map and update it,
      //    or create a new one
      //=======================================================================
      btNumberKey instrecIndex = IntNameFromDeviceAddress( &pcfgUpDate->devattrs.devid.m_devaddr);
      pInstRec_t pInstRecExisting( NULL);

      // If the map contains an existing Instance
      if (m_InstRecMap.Has( instrecIndex)) {

         // Get a pointer to the existing InstRec, which holds the configuration update record
         if (m_InstRecMap.Get( instrecIndex, &pInstRecExisting)) {

            // Update the existing InstRec with the new information
            if (pInstRecExisting->ReplaceStruct( pcfgUpDate)) {
               // Successful replacement. Do nothing more, ready to continue to next steps
               // NOTE: Do NOT update numAllocations as the state machine needs to run
            }
            else {
               AAL_WARNING(LM_ResMgr,"CResMgr::DoConfigUpdate ReplaceStruct of existing Instance Record failed.\n");
               return EPERM;
            }
         }
         else {
            AAL_WARNING(LM_ResMgr,"CResMgr::DoConfigUpdate Get of existing Instance Record failed.\n");
            return EPERM;
         }
      }
      else {

         // The map does not have a match. Create a new Instance record and add it to the map
         InstRec instrec( pcfgUpDate);       // Note that this initializes number of Allocations to numOwners
         if ( m_InstRecMap.Add( instrec)) {

            // Add succeeded, get the record from the map
            if( m_InstRecMap.Get( instrecIndex, &pInstRecExisting)) {
               // Successful creation, do nothing else
            }
            else {
               AAL_WARNING(LM_ResMgr,"CResMgr::DoConfigUpdate Get of freshly added Instance Record failed.\n");
               return EPERM;
            }
         }
         else {
            AAL_WARNING(LM_ResMgr,"CResMgr::DoConfigUpdate attempted to Add a new Instance Record to map failed.\n" << instrec);
            return EPERM;
         }
      }

      //=======================================================================
      // Have an Instance Record in the map:
      //    Index:   instrecIndex
      //    Address: pInstRecExisting
      //=======================================================================

      switch (id) {
         case krms_ccfgUpdate_DevAdded: {
            // TODO: when see MAFU (subdevnum == 0) then consult activation record
            //       to initialize known afus.
#if 0
            // If a board, then need to activate it. Otherwise, all done
            // Is it a board?
            if(pcfgUpDate->devattrs.devid.m_devaddr.m_subdevnum == -1) {

               // Re-initialize the ioctlreq. Note use of the payload, must be before clearing payload
               pIoctlReq->req_handle = NULL;                // This is a new request, not a response
               pIoctlReq->id = reqid_RM_DeviceRequest;
               pIoctlReq->res_handle = pcfgUpDate->devattrs.Handle;
               pIoctlReq->result_code = rms_resultOK;

               // Clear out the current payload, pcfgUpDate will be NULL after this function returns
               pcfgUpDate = static_cast<struct aalrms_configUpDateEvent*>(DestroyRMIoctlReqPayload( pIoctlReq)); // clear payload

               // Create new payload
               struct aalrms_DeviceRequest* pDevReq;
               pIoctlReq->payload = pDevReq = new struct aalrms_DeviceRequest;    // will throw if no memory
               pIoctlReq->size = sizeof(struct aalrms_DeviceRequest);

               // Initialize the payload, pDevReq is just for convenience
               memset( pDevReq, 0, sizeof( struct aalrms_DeviceRequest));
               pDevReq->reqid = aaldev_reqActivate;
               pDevReq->subdeviceMask = AAL_DEVIDMASK_AFU(0) + AAL_DEVIDMASK_AFU(1) + AAL_DEVIDMASK_AFU(2) + AAL_DEVIDMASK_AFU(3);

               AAL_VERBOSE(LM_ResMgr,
                     "CResMgr::DoConfigUpdate:krms_ccfgUpdate_DevAdded: Sending Device_Request.\n");

               // Send the response
               Retval = Send_AALRMS_Msg(fdServer, pIoctlReq);
            }  // if( ...m_subdevnum == -1)  /* is it a board? */
#endif
            break;
         }
         case krms_ccfgUpdate_DevRemoved: {
            // Need to delete it from the Map
            pInstRecExisting = NULL;
            m_InstRecMap.Delete( instrecIndex);
            break;
         }
         case krms_ccfgUpdate_DevOwnerAdded:
            // Number of allocations was incremented during DeviceRequest
            // OwnerAdded implies that the number of allocations should be one
            //    more than before but we have no way to check, so don't care
            break;
         case krms_ccfgUpdate_DevOwnerUpdated:
            // Don't care
            break;
         case krms_ccfgUpdate_DevOwnerRemoved: {
            // Decrement the Number of Allocations that the RM is tracking
            if ( ! pInstRecExisting->DecrementAllocations()) {
               AAL_ERR(LM_ResMgr, "CResMgr::DoConfigUpdate DevOwnerRemoved failed\n");
               return EPERM;
            }
            break;
         }
         case krms_ccfgUpdate_DevActivated:
            // Don't care
            break;
         case krms_ccfgUpdate_DevQuiesced:
            // Don't care
            break;
         default:
            break;
      }  // switch (id)

   } // if (pcfgUpDate)
   else {
      AAL_ERR(LM_ResMgr, "CResMgr::DoConfigUpdate: evtid_KRMS_ConfigUpdate contained no payload\n");
      return EPERM;
   }

   // No need for response

   return Retval;
} // CResMgr::DoConfigUpdate


//=============================================================================
// Name:          CResMgr::NVSFromConfigUpdate
// Description:   Handle evtid_KRMS_ConfigUpdate for the parser
// Interface:     public
// Inputs:        fdServer - file index - NOT CHECKED FOR VALIDITY
//                pIoctlReq - ioctl structure - NOT CHECKED FOR VALIDITY
// Outputs:
// Returns:       0 for Success, errno for failure, with perror having already
//                been called.
// Comments:
//=============================================================================
void CResMgr::NVSFromConfigUpdate(const aalrms_configUpDateEvent &cfgUpdate, NamedValueSet &nvs)
{
   nvs.Add(keyRegInstUpdateType,       cfgUpdate.id);
   nvs.Add(keyRegPid,                  cfgUpdate.pid);

   nvs.Add(keyRegHandle,               cfgUpdate.devattrs.Handle);
   nvs.Add(keyRegDeviceState,          cfgUpdate.devattrs.state);
   nvs.Add(keyRegMaxOwners,            cfgUpdate.devattrs.maxOwners);
   nvs.Add(keyRegNumOwners,            cfgUpdate.devattrs.numOwners);
   nvs.Add(keyRegOwners,               const_cast<int*>(cfgUpdate.devattrs.ownerlist), cfgUpdate.devattrs.numOwners);

   nvs.Add(keyRegVendor,               cfgUpdate.devattrs.devid.m_vendor);
   nvs.Add(keyRegPIP_ID,               static_cast<btUnsigned64bitInt>(cfgUpdate.devattrs.devid.m_pipGUID));
   nvs.Add(keyRegAHM_ID,               static_cast<btUnsigned64bitInt>(cfgUpdate.devattrs.devid.m_ahmGUID));

   // TODO - fix up to use the real m_afuGUID whenever it becomes a struct, in AFU_IDNameFromConfigStruct()
   nvs.Add(keyRegAFU_ID,               AFU_IDNameFromConfigStruct( cfgUpdate).c_str());

   nvs.Add(keyRegBusType,              static_cast<bt32bitInt>(cfgUpdate.devattrs.devid.m_devaddr.m_bustype));
   nvs.Add(keyRegBusNumber,            cfgUpdate.devattrs.devid.m_devaddr.m_busnum);
   nvs.Add(keyRegDeviceNumber,         cfgUpdate.devattrs.devid.m_devaddr.m_devicenum);
   nvs.Add(keyRegChannelNumber,        cfgUpdate.devattrs.devid.m_devaddr.m_subdevnum);

   nvs.Add(keyRegDeviceType,           cfgUpdate.devattrs.devid.m_devicetype);

   // Compute combined device address as a data value
   nvs.Add(keyRegDeviceAddress,        IntNameFromDeviceAddress( &cfgUpdate.devattrs.devid.m_devaddr));

   // this is an Instance Record, mark it as such
   nvs.Add(enumRegRecordType_Key,      enumRegRecordType_Instance);

}  // end of CResMgr::NVSFromConfigUpdate


END_NAMESPACE(AAL)

