// Copyright (c) 2008-2015, Intel Corporation
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
/// @file RegDBStub.cpp
/// @brief External char*-based interface to a Database Daemon.
/// @ingroup Registrar
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Henry Mitchel, Intel Corporation.
///          Joseph Grecco, Intel Corporation.
///
/// COMMENT: The RegSendMsg and RegRcvMsg functions handle the protocol with
///          the corresponding DBSendMsg and DBRcvMsg.
///          Their knowledge of each other is temporary
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 06/19/2008     HM       Registrar side of transport to/from Database
///                            Initially designed to be independent from
///                            Registrar, it is really now just a subset,
///                            and could be included directly in
///                            CAASRegistrar.cpp
/// 07/03/2008     HM       Finalizing splitting of Registrar and Database
/// 08/10/2008     HM       Moved write_itr_to_RCP() and read_itr_from_RCP()
///                            from RegDBProxy.cpp
/// 11/03/2008     HM       Fixed MarshalCommand() to copy NVS's that contain
///                            embedded nulls
/// 11/10/2008     HM       Handle changes to ioctlreq payload to void*
/// 01/04/2009     HM       Updated Copyright
/// 02/11/2010     JG       Added support for glib version 4.4@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include "aalsdk/AALTypes.h"
#include "aalsdk/registrar/CAASRegistrar.h"
#include "aalsdk/kernel/aalrm.h"          // kernel transport services


BEGIN_NAMESPACE(AAL)


//=============================================================================
// Name: RegistrarCommand_New
// Description: Ctor for the RegistrarCmdResp_t structure as a Command.
//              True ctor not allowed, as not a true class. If make it a true
//              class then cannot have the terminating szNVS
//=============================================================================
pRegistrarCmdResp_t RegistrarCommand_New(size_t lenszNVS)
{
   pRegistrarCmdResp_t pRC = reinterpret_cast<pRegistrarCmdResp_t>(new char[sizeof(RegistrarCmdResp_t) + lenszNVS]);
   if (pRC) {
      memset (pRC, 0, sizeof(RegistrarCmdResp_t) + lenszNVS);               // Clear to zero, insert signature and len of NVS
      memcpy (&pRC->szSignature, RegistrarCmdCommandSignature, lenRegistrarCmdRespSignature);
      pRC->LengthOfNVS = lenszNVS;
      pRC->LengthOfStruct = lenszNVS + sizeof(RegistrarCmdResp_t);
   }
   return pRC;
} // End of RegistrarCommand_New


//=============================================================================
// Name: MarshalCommand
// Description: Do the grunt work to create a command buffer
// Comments: pnvs and pitr default to NULL
//=============================================================================
pRegistrarCmdResp_t MarshalCommand(eCmd          Command,
                                   const         TransactionID& rTransID,
                                   pRegRetFunc   pFunc,
                                   const         NamedValueSet* pnvs,
                                   const         ITR_t*         pitr)
{
   size_t             lenszNVS;
   std::ostringstream oss;

   if (pnvs) {
      oss << *pnvs;
      lenszNVS = oss.str().length()+1; // +1 for terminating null
   } else {
      lenszNVS = 0;
   }

   pRegistrarCmdResp_t pRC = RegistrarCommand_New(lenszNVS);
   if (pRC) {
      pRC->TranID   = rTransID;
      pRC->pFunc    = pFunc;
      pRC->Command  = Command;
      pRC->Response = eRegOK;
      if( pitr ){
         write_itr_to_RCP ( pRC, pitr);
      }
      if( pnvs ){
//         strcpy( pRC->szNVS, oss.str().c_str());
         oss.str().copy(pRC->szNVS,lenszNVS);
      }
   }
   return pRC;
}

//=============================================================================
// Name: write_itr_to_RCP
// Description: write the contents of an "itr", a generic iterator, out to
//              a RegistrarCmdResp
// Use like this:
//    ITR_t itr;              // initially CRegDB, RegKey later
//    pRegistrarCmdResp_t p;
//    write_itr_to_RCP (p, &itr)
//=============================================================================
void write_itr_to_RCP(pRegistrarCmdResp_t pRCR, const ITR_t* pitr)
{
   pRCR->Key = pitr->m_PrimaryKey;
   pRCR->Lock = pitr->m_LockKey;
}

//=============================================================================
// Name: read_itr_from_RCP
// Description: read the contents of an "itr", a generic iterator, in from
//              a RegistrarCmdResp
// Use like this:
//    ITR_t itr;              // initially CRegDB, RegKey later
//    pRegistrarCmdResp_t p;
//    read_itr_from_RCP (p, &itr)
//=============================================================================
void read_itr_from_RCP(const pRegistrarCmdResp_t pRCR, pITR_t pitr)
{
   pitr->m_PrimaryKey = pRCR->Key;
   pitr->m_LockKey = pRCR->Lock;
}


//=============================================================================
// Name:        CRegistrar::InitClientFileDescriptor
// Description: Initialize Client file descriptor
// Comments:    Called before using the file descriptor
//              Just call it once, although can withstand multiple calls
// SideEffect:  Operates on CRegistrar::m_fdRMClient as both input and output
// Returns:     true if can use CRegistrar::m_fdRMClient
//=============================================================================
btBool CRegistrar::InitClientFileDescriptor()
{
   if( -1 != m_fdRMClient ){     // Initialized?
      return true;               // Yes - get out
   } else {                      // No - create it
      m_fdRMClient = open (m_sResMgrClientDevName.c_str(), O_RDWR);
      if (m_fdRMClient >= 0){                                      // success
         DPOUT(cout << "CRegistrar::InitClientFileDescriptor opened file " << m_sResMgrClientDevName << " as file " << m_fdRMClient << endl;)
         return true;
      } else {
         m_fdRMClient = -1;
         std::cerr << "CRegistrar::InitClientFileDescriptor open of " << m_sResMgrClientDevName << " failed with error code " << errno << "\n\tReason string is: ";
         perror(NULL);
//         char* pBuf = strerror_r ( saverr, m_pErrBuf, m_lenErrBuf);
//         if (pBuf) cerr << pBuf << endl;
         return false;
      }
   }
}  // CRegistrar::InitClientFileDescriptor

//=============================================================================
// Name:        CRegistrar::RegRcvMsg
// Description: Registrar's Receiver for Database's Send Message
//              Message pump that drives Registrar callbacks
// Comments:    If there ends up being another transport mechanism it
//              will be implemented here
//=============================================================================
void CRegistrar::RegRcvMsg(OSLThread *pThread, void *pContext)
{
   // Get a pointer to this object, a CRegistrar
   CRegistrar *This = reinterpret_cast<CRegistrar*>(pContext);

   // Open the file, this must happen here, so the main thread is blocked until it is done
   if (!This->InitClientFileDescriptor()) {  // doing it here rather than in constructor as it can be switched at run-time to something else
      return;                                // failure is signified by m_fdRMClient==-1
   }

   // Let the constructor continue
   This->SemPost();

   // Polling structures
   struct pollfd           pollfds[1];          // Just one file
   struct aalrm_ioctlreq   req;                 // Client version of the Resource Manager IoctlReq
   int                     ret;                 // Multiply re-used return code
   unsigned                numMessages = 0;     //
   pollfds[0].fd     =     This->m_fdRMClient;  // Initialized in InitClientFileDescriptor
   pollfds[0].events =     POLLPRI;

   // Poll the driver processing messages as they come
   while (This->m_fKeepRunning){
      ++numMessages;

      DPOUT(cout << "CRegistrar::RegRcvMsg[" << numMessages <<"] Entering Poll\n";)
      ret = poll(pollfds,1,-1);
      DPOUT(cout << "CRegistrar::RegRcvMsg[" << numMessages <<"] Exiting Poll with return code " << ret << endl;)

      // We could be shutting down, so if all is not hunky dory, get out
      if (!This->m_fKeepRunning) {
         DPOUT(cout << "CRegistrar::RegRcvMsg Shutting Down\n";)
         break;
      }

      if((ret == -1) || (ret == 0)){
         std::cerr << "CRegistrar::RegRcvMsg Poll returned unexpected value: " << ret << ". Retrying.\n";
         continue;
      }

      // Get the message description
      ret = ioctl (This->m_fdRMClient, AALRM_IOCTL_GETMSG_DESC, &req);
      if (-1 == ret){
         std::cerr << "CRegistrar::RegRcvMsg AALRM_IOCTL_GETMSG_DESC failed with return code " << ret << ". Retrying.\n";
         perror ("CRegistrar::RegRcvMsg AALRM_IOCTL_GETMSG_DESC");
         continue;
      }

      if (rspid_Shutdown == req.id) {

          DPOUT(cout << "CRegistrar::RegRcvMsg Type = " << req.id << ", Payload length " << req.size << endl;)

          // TODO - I don't think there ever IS a payload for this message. Don't need to do this.

          req.payload = new unsigned char[req.size];      // Let this one throw, TODO: need a better signal, at least a handler at the top

          // Get the ACTUAL MESSAGE
          ret = ioctl (This->m_fdRMClient, AALRM_IOCTL_GETMSG, &req);

          if (-1 != ret) {                       // If success ...
             DPOUT(cout << "CRegistrar::RegRcvMsg AALRM_IOCTL_GETMSG succeeded, for shutdown\n";)
             // Break outof the thread
             break;

          }else {                                // Failure
             perror ("CRegistrar::RegRcvMsg AALRM_IOCTL_GETMSG");
          }

          // Clean up and loop to poll again
          if (req.payload) delete [] static_cast<unsigned char*>(req.payload);
          continue;
       }



      // Expecting to handle only Registrar Responses
      if (rspid_RS_Registrar == req.id) {
         DPOUT(cout << "CRegistrar::RegRcvMsg Type = " << req.id << ", Payload length " << req.size << endl;)

         req.payload = new unsigned char[req.size];      // Let this one throw, TODO: need a better signal, at least a handler at the top

         // Get the ACTUAL MESSAGE
         ret = ioctl (This->m_fdRMClient, AALRM_IOCTL_GETMSG, &req);

         if (-1 != ret) {                       // If success ...
            DPOUT(cout << "CRegistrar::RegRcvMsg AALRM_IOCTL_GETMSG succeeded, here is the response:\n"
                 << *reinterpret_cast<pRegistrarCmdResp_t>(req.payload)
                 << "CRegistrar::RegRcvMsg Executing response\n";)

            // Drive the response to the Registrar request
            This->ParseResponse (reinterpret_cast<pRegistrarCmdResp_t>(req.payload));

         }else {                                // Failure
            perror ("CRegistrar::RegRcvMsg AALRM_IOCTL_GETMSG");
         }

         // Clean up and loop to poll again
         if (req.payload) delete [] static_cast<unsigned char*>(req.payload);
      }
      else {
         std::cerr << "CRegistrar::RegRcvMsg received response id: " << req.id << ", which is not for Registrar. Discard and Retrying.\n";
      }
   }  // while
}  // RegRcvMsg


#if 0
//=============================================================================
// Name: FSBV1_AIA
// Description: Constructor
// Interface: public
// Inputs: none
// Outputs: none.
// Comments: Initalizes driver
//=============================================================================
void FSBV1_AIA::MessageDeliveryThread(OSLThread *pThread, void *pContext)
{
   //Get a pointer to this objects context
   FSBV1_AIA *This = (FSBV1_AIA*)pContext;

   // Poll the driver processing messages ars they come
   while (This->IsOK() && (This->m_State==Running)){
      fd_set fds;
      FD_ZERO (&fds);              // Clear the mask
      FD_SET (This->m_dev, &fds);  // Set with our dev

      // Poll the driver with no timeout
      int ret = select (This->m_dev + 1, &fds, NULL, NULL, NULL);
      if (This->m_State!=Running){
         return;
      }

      if (ret == -1){
         perror ("FAP_AIALinux.cpp select");
         continue;
      }

      // Use the same handler used for Signal mode
      siginfo_t SigAction;
      SigAction.si_fd = This->m_dev;
      This->DriverEventHandler(0,            // Sig number unsued
                              &SigAction,
                              NULL);        // uContext unused
  }

}
#endif


//=============================================================================
// Name: RegSendMsg
// Description: Registrar's Send Message to Database's Receiver
//=============================================================================
void CRegistrar::RegSendMsg(pRegistrarCmdResp_t pBlockToSend) const
{
   struct aalrm_ioctlreq req;

   // EXTREMELY UNLIKELY - could use an ASSERT that m_fdRMClient is != -1
   if (-1 == m_fdRMClient) {           // oops, need to initialize
      std::cerr << "CRegistrar::RegSendMsg having to initialize file. LOGIC ERROR.\n";
      btBool ret = const_cast<CRegistrar*>(this)->InitClientFileDescriptor();
      if (!ret) {
         std::cerr << "CRegistrar::RegSendMsg could not initialize file. Aborting.\n";
         return;
      }
   }

   req.id = reqid_RS_Registrar;
   req.size  = pBlockToSend->LengthOfStruct;
   req.payload = reinterpret_cast<unsigned char*>(pBlockToSend);
   req.tranID = (stTransactionID_t &)TransactionID(123456);
   req.context = (void*)67890;

   DPOUT(std::cout << "CRegistrar::RegSendMsg: Sending Registrar Request:\n" << *pBlockToSend << std::endl;)

   if (ioctl (m_fdRMClient, AALRM_IOCTL_SENDMSG, &req) == -1){
      perror ("CRegistrar::RegSendMsg");
   }
}  // RegSendMsg

//=============================================================================
// Name: ParseResponse
// Description: Perform initial parse of a pRegistrarCmdResp_t as a response
// Interface: should be private
// Inputs: pointer to the RegistrarCmdResp_t
// Outputs: none
// Comments: calls appropriate Registrar xxxx_ret routine
//=============================================================================
void CRegistrar::ParseResponse( pRegistrarCmdResp_t p )
{
#ifdef DEBUG_REGISTRAR
   std::cout << "CRegistrar::ParseResponse seen\n";
#endif

   // check signature; not equal means different, therefore error
   if ( memcmp ( p->szSignature, RegistrarCmdResponseSignature, lenRegistrarCmdRespSignature ) ) {
      std::cerr << "CRegistrar::ParseResponse szSignature not Response signature, aborting\n";
      // TODO: call global exception handler with EXCEPTION - not clear how to do this with the correct transaction ID
      return;
   }

#ifdef DEBUG_REGISTRAR
   std::cout << "RegistrarCmdResp is:" << *p;
#endif

   if( p->pFunc ){                     // if have valid function pointer, call it. NULL should be handled in the skeleton
      (this->*(p->pFunc))(p);
   }                                   // no valid function pointer is okay for certain functions, just eat the response

//    else {
//       cerr << "CRegistrar::ParseResponse - invalid return function pointer\n";
//    }

} // end of ParseResponse


END_NAMESPACE(AAL)

 
