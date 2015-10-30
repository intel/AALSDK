// Copyright (c) 2007-2015, Intel Corporation
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
/// @file FAP10.cpp
/// @brief FAP v1.0 specific details for the Universal Application Interface
///        Adaptor.
/// @ingroup uAIA
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Henry Mitchel, Intel Corporation
///          Joseph Grecco, Intel Corporation
///          Alvin Chen, Intel Corporation
///
/// 
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 12/01/2008     HM       Moved FAP10 AFUDev declarations from uAIA.cpp
/// 12/10/2008     HM/JG    WkSp_MappingEventHandlerObject::WkSp_Allocate_Mapping_EventHandler
///                            and WkSp_Single_Allocate_AFUTransaction, finished
/// 12/11/2008     HM       More AFU Transactions
/// 12/14/2008     HM       Allocate and Free done
/// 12/15/2008     HM       All new SubmitTransaction
/// 12/20/2008     HM       Finished SubmitTask_Serial_AFUTransaction, which
///                            is the best way to submit a serial mode task
///                         Added SubmitTask_VMaster_AFUTransaction, which is
///                            the best way to submit a Physical MM mode task
///                         Added CSR_AFUTransaction, the best way to Get and
///                            Set one or more CSRs
/// 12/30/2008     HM       Carry more things in the WorkSpaceMapper so can
///                            reduce the client load. WorkSpaceMapper moves
///                            from AFU to AFUDev, for use by AFUTransaction.
/// 01/04/2009     HM       Updated Copyright
/// 01/04/2009     HM       Changed WkSp_Free_Mapping_EventHandlerObject
///                            ctor signature and calls
/// 02/08/2009     HM       Finished new implementation of
///                            SubmitTask_VMaster_AFUTransaction and
///                            SubmitTask_VMaster_Output_AFUTransaction.
///                            Still needs testing.
/// 03/07/2009     HM       Fixed bug in WkSp_Single_Allocate_AFUTransaction
///                            found by Suchit
/// 03/17/2009     JG       Modified AFUTransaction::Render() for new GetSet
///                            structure
/// 03/20/2009     JG/HM    Global change to AFU_Response that generically puts
///                            payloads after the structure with a pointer to
///                            them. Ptr must be converted kernel to user.
/// 05/15/2009     HM       Added AFUTransaction to support Activate/DeActivate
///                            for Management AFU
/// 05/19/2009     HM       Added AFUTransaction to support Initialize/Free
///                            for Management AFU
/// 06/17/2009     AC       Remove wsid from SubmitTask_Serial_AFUTransaction
/// 06/20/2009     JG       Moved to FAPTransaction library
///                         Made compatible with AALBaseProxy and AALProxy
///                            mechanisms.
/// 06/24/2009     AC       Enable the output DESC can be with NULL buffer for Serial_AFUTransaction
/// 07/01/2009     AC       Dispatch error message if the buffer address is invalid.
/// 08/05/2009     AC       In mmap handler, no need to do the mmap if the error code is not OK.
/// 08/07/2009     HM       Fixed some printing problems, robustified
///                            Allocate_Workspace_Handler.
///                         TODO: Similar for Free_Workspace_Handler
/// 08/16/2009     HM       Removed cautionary string from
///                            SubmitTask_Serial_AFUTransaction ctor as it was
///                            fixed by Alvin
/// 08/16/2009     HM       Added missing WkSp::RemoveFromMap() that was
///                            missing from WkSp_Single_Free_AFUTransaction
/// 08/16/2009     HM       Added support for Zero-Length buffers by providing
///                            a Requested_Length as well as an actual length
///                         For Requested_Length==0, actual length is set to 1.
/// 12/09/2009     JG       Pulled out AFU PIP commands aalui_afucmd_e
///                         and moved it to fappip,h and defined them as FAP
///                         pip specific.
/// 12/27/2009     JG       Added support for optimized signaling through
///                            memory mapped CSRs (partial)
/// 01/12/2010     HM       Fixed use of Length for Slave Mode buffer in the
///                            Submit_XPhysical code. Previously the length of
///                            original buffer was used, but now input length
///                            is used, allowing allocation of a large buffer
///                            followed by use of only a portion of it.
///                         Based on customer feedback.
/// 09/09/2010     HM       Filled in size parameters for payloads which had
///                            not been previously initialized
/// 09/18/2010     HM       Formatting and comment cleanup
/// 09/01/2011     JG       Removed AIAProxy code
/// 03/05/2012     JG       Added SPL2_Start_AFUTransaction@endverbatim
//****************************************************************************
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H

#include <aalsdk/AALBase.h>                 // IBase
#include <aalsdk/INTCDefs.h>
#include <aalsdk/AFUpkg.h>                  // CAFUAllocateWorkSpaceTransactionEvent
#include <aalsdk/faptrans/FAP10.h>          // Definitions of AFUTransactions
#include <aalsdk/faptrans/FAP10Service.h>
#include <aalsdk/uaia/FAPPIP_AFUdev.h>

#include <aalsdk/AALLoggerExtern.h>         // Logger
#include <aalsdk/kernel/KernelStructs.h>    // Printing structures
#include <aalsdk/utils/AALEventUtilities.h> // ReThrow


#if defined ( __AAL_WINDOWS__ )                                               
# pragma warning(push)            
# pragma warning(disable : 4996) // destination of copy is unsafe            
#endif // __AAL_WINDOWS__

FAPTRANS1_BEGIN_MOD()
   /* No commands other than default, at the moment. */
FAPTRANS1_END_MOD()

#if defined ( __AAL_WINDOWS__ )
# pragma warning(pop)
#endif // __AAL_WINDOWS__


BEGIN_NAMESPACE(AAL)


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////     U T I L I T Y   F U N C T I O N S     ///////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//=============================================================================
// Name:          TDESC_POSITION_from_delim()
// Description:   Encapsulates conversion of a kernel structure to a user-
//                   viewable enumeration
// Comments:      Handles possible size mismatch between the enumeration and
//                   the kernel version.
//=============================================================================
TDESC_POSITION TDESC_POSITION_from_delim(btUnsigned16bitInt delim)
{
   // If the sizes are the same, this will compile to just the return and
   //    the else will be discarded.

   // If the sizes are the same, just need a cast
   if (sizeof(TDESC_POSITION) == sizeof(btUnsigned16bitInt)) {
      return  static_cast<TDESC_POSITION>(delim);
   }
   else {
   // otherwise, need to be explicit about the conversion
      TDESC_POSITION dp((TDESC_POSITION)0);
      if (delim & (AHM_DESC_SOT | AHM_DESC_MOT | AHM_DESC_EOT)) {
         dp = COMPLETE_TASK;
      }
      else if (delim & AHM_DESC_SOT) {
         dp = START_OF_TASK;
      }
      else if (delim & AHM_DESC_MOT) {
         dp = MIDDLE_OF_TASK;
      }
      else if (delim & AHM_DESC_EOT) {
         dp = END_OF_TASK;
      }
      return dp;
   }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////     F A P - V 1 . 0 - S P E C I F I C     ///////////////////
/////////////////      A F U   T R A N S A C T I O N S      ///////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////     SubmitTask_Serial_AFUTransaction      ///////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//=============================================================================
// Name:          SubmitTask_Serial_AFUTransaction::SubmitTask_Serial_AFUTransaction()
// Description:   Destructor
// Interface:     public
// Inputs:        none
// Outputs:       none
// Comments:      virtual
//=============================================================================

SubmitTask_Serial_AFUTransaction::SubmitTask_Serial_AFUTransaction()
:  m_viBufDesc(),
   m_voBufDesc(),
   m_payload(NULL),
   m_pIAFUDev(NULL),
   m_size(0),
   m_bRendered(false),  // At startup nothing as been rendered
   m_bIsOK(false),       // At startup everything is okay
   m_bStartTask(true),  // Default is that this is an atomic task
   m_bEndTask(true),    // Default is that this is an atomic task
   m_TranID()           // Default to a new, unique, TransactionID
{
   AAL_VERBOSE(LM_All, "SubmitTask_Serial_AFUTransaction ctor.\n");
}


//=============================================================================
// Name:          SubmitTask_Serial_AFUTransaction::~SubmitTask_Serial_AFUTransaction()
// Description:   Destructor
// Interface:     public
// Inputs:        none
// Outputs:       none
// Comments:      virtual
//=============================================================================
SubmitTask_Serial_AFUTransaction::~SubmitTask_Serial_AFUTransaction()
{
   Clear();
}  // SubmitTask_Serial_AFUTransaction::~SubmitTask_Serial_AFUTransaction

//=============================================================================
// Name:          SubmitTask_Serial_AFUTransaction::GetPayloadPtr
// Description:   Causes rendering to occur, and a pointer to the rendered
//                   block is returned
// Interface:     public
// Inputs:        none
// Outputs:       none
// Comments:      If rendering has already occurred, it is not done again
//=============================================================================
btVirtAddr SubmitTask_Serial_AFUTransaction::GetPayloadPtr()
{
   AAL_VERBOSE(LM_All, "SubmitTask_Serial_AFUTransaction::GetPayloadPtr seen.\n");
   if (!m_bRendered) Render();
   if (IsOK()) {
      return m_payload;
   } else {
      return NULL;
   }
}  // SubmitTask_Serial_AFUTransaction::GetPayloadPtr

//=============================================================================
// Name:          SubmitTask_Serial_AFUTransaction::GetSize
// Description:   Causes rendering to occur, and the size of the rendered
//                   block is returned
// Interface:     public
// Inputs:        none
// Outputs:       none
// Comments:      If rendering has already occurred, it is not done again
//=============================================================================
btWSSize SubmitTask_Serial_AFUTransaction::GetSize()
{
   AAL_VERBOSE(LM_UAIA, "SubmitTask_Serial_AFUTransaction::GetSize seen"  << std::endl );
   if (!m_bRendered) Render();
   if (IsOK()) {
      return m_size;
   } else {
      return 0;
   }
}  // SubmitTask_Serial_AFUTransaction::GetSize()

//=============================================================================
// Name:          SubmitTask_Serial_AFUTransaction::AddBuffer
// Description:   Adds a buffer/length description to the object
//                Last parameter wsid is defaulted to -1, which is very illegal.
//                   wsid will be deprecated.
// Interface:     public
// Inputs:        payload pointer and length. Payload pointer is user virtual.
// Outputs:       none
// Comments:      If rendering has already occurred, the rendering is destroyed
//=============================================================================
void SubmitTask_Serial_AFUTransaction::AddBuffer(btBool       fOutput,    // Is this an output descriptor?
                                                 btVirtAddr   Address,    // User mode virtual pointer to buffer
                                                 btWSSize     Length,     // Length of the buffer
                                                 btObjectType Context,    // Descriptor context
                                                 btBool       fRsp)       // Notify?
{
   if (m_bRendered) ClearRender();
   AAL_VERBOSE(LM_UAIA, "Context=" << std::hex << Context << std::endl );

   if (fOutput) {                               // add to input descriptor list
      m_voBufDesc.push_back( BufferDescriptor_t( Address, Length, Context, fRsp));
   } else {                                     // add to output descriptor list
      m_viBufDesc.push_back( BufferDescriptor_t( Address, Length, Context, fRsp));
   }
}  // SubmitTask_Serial_AFUTransaction::AddBuffer

//=============================================================================
// Name:          SubmitTask_Serial_AFUTransaction::ComputeMask
// Description:   Computes SOT/MOT/EOT bits based on position, m_bStartTask and
//                   m_bEndTask
// Interface:     private
// Inputs:
// Outputs:
// Comments:      internal worker function
//=============================================================================
btUnsigned16bitInt SubmitTask_Serial_AFUTransaction::ComputeMask(size_t index, size_t numRecs)
{
   btUnsigned16bitInt mask=0;

   if (1 == numRecs) {                    // special case
      if (m_bStartTask) {
         if (m_bEndTask) {                // Both start and end
            mask = AHM_DESC_SOT | AHM_DESC_MOT | AHM_DESC_EOT;
         } else {                         // Start, but not End
            mask = AHM_DESC_SOT;
         }
      } else {
         if (m_bEndTask) {                // End, but not Start
            mask = AHM_DESC_EOT;
         } else {                         // Neither start nor end
            mask = AHM_DESC_MOT;
         }
      }

   // Always two or more records (or zero, but then it will just fall out)
   // Every record in this case will be exactly one of the three
   } else {
      if (0 == index) {                   // If it is the first record
         if (m_bStartTask) {              // and start task is on
            mask = AHM_DESC_SOT;
         } else {                         // if not start task, need something
            mask = AHM_DESC_MOT;          // and can not be EOT, should be MOT
         }                                // Done with this record
      }
      else if ((numRecs-1) == index) {    // If it is the last record
         if (m_bEndTask) {                // and end task is on
            mask = AHM_DESC_EOT;
         } else {                         // if not end task, need MOT
            mask = AHM_DESC_MOT;
         }
      }
      else {
         mask = AHM_DESC_MOT;             // neither start nor end, must be MOT
      }
   }
   return mask;

}  // SubmitTask_Serial_AFUTransaction::ComputeMask

//=============================================================================
// Name:          SubmitTask_Serial_AFUTransaction::Render
// Description:   Renders (serializes) the object into a buffer
// Interface:     private
// Inputs:        none directly. Indirectly m_vBufDesc
// Outputs:       none directly. Indirectly, m_payload and m_size and m_bRendered
//                   set, with m_payload pointing to a malloc'd buffer of length
//                   m_size.
// Comments:      If rendering has already occurred, it is not repeated
//=============================================================================
void SubmitTask_Serial_AFUTransaction::Render() {
   if (m_bRendered)
      return;
   m_bIsOK = false;

   //////////////////////////////////////////////////////////////////////////////////////
   if (m_pIAFUDev == NULL)
      return;

   AAL_VERBOSE(LM_All, "SubmitTask_Serial_AFUTransaction::Render seen.\n");

   size_t numIRecs = m_viBufDesc.size();
   size_t numORecs = m_voBufDesc.size();
   size_t lenBytes = sizeof(struct big) + (numIRecs + numORecs) * sizeof(struct desc_share_info);

   // get a new big structure with space for the descriptors, as well
   m_payload = (btVirtAddr)new btByte[lenBytes];
   memset(m_payload, 0, lenBytes);

   //////////////////////////////////////////////////////////////////////////////////////////////
   CAFUDev* pCAFUDev = dynamic_ptr<CAFUDev> (iidCAFUDev, dynamic_cast<IBase*> (m_pIAFUDev));

   // Load the afuMsg
   struct big* pBig = reinterpret_cast<struct big*> (m_payload);

   pBig->afuMsg.cmd     = AHM_DESC_MULTISUBMIT;
   pBig->afuMsg.payload = (btVirtAddr)&pBig->desc;
   pBig->afuMsg.size    = sizeof(submit_descriptors_req_t)
                          + (numIRecs + numORecs) * sizeof(struct desc_share_info);
   pBig->afuMsg.apiver  = GetAPIVer();
   pBig->afuMsg.pipver  = GetPIPVer();
   AAL_VERBOSE(LM_UAIA, "afuMsg.payload:" <<
         std::hex << pBig->afuMsg.payload <<
         "\n\tm_payload="<<static_cast<void*>(m_payload)<< std::endl
   );

   // Load the submit_descriptors structure
   submit_descriptors_req_t *pSubmit = reinterpret_cast<submit_descriptors_req_t *>(pBig->afuMsg.payload);
   pSubmit->m_mode                   = SLAVE_MODE;
   pSubmit->m_tranID                 = (stTransactionID_t&) m_TranID;
   pSubmit->m_nDesc                  = (btUnsigned16bitInt)( numORecs + numIRecs );

   // Initialize output descriptors, then input descriptors
   size_t index = 0;
   for (index = 0; index < numORecs; ++index) {
      pSubmit->m_arrDescInfo[index].m_type      = OUTPUT_DESC;
      pSubmit->m_arrDescInfo[index].m_delim     = ComputeMask(index, numORecs);
      pSubmit->m_arrDescInfo[index].m_uvptr     = (btVirtAddr)m_voBufDesc[index].m_uvptr; /* In: the user virtual pointer in the workspace */
      pSubmit->m_arrDescInfo[index].m_len       = (size_t)m_voBufDesc[index].m_len; /* In: length of the buffer within the workspace, currently in bytes */
      pSubmit->m_arrDescInfo[index].m_no_notify = !m_voBufDesc[index].m_bRsp;
      pSubmit->m_arrDescInfo[index].m_context   = (btVirtAddr)m_voBufDesc[index].m_context;

      //////////////////////////////////////////////////////////////////////////////////
      // Need to get workspace. Input is a pointer and a length describing a buffer.
      WorkSpaceMapper::pcWkSp_t pWkSp; // will point to a WkSp

      // Get the associated workspace
      WorkSpaceMapper::eWSM_Ret eRet = pCAFUDev->WSM().GetWkSp(m_voBufDesc[index].m_uvptr, &pWkSp,
                                                               m_voBufDesc[index].m_len);

      // Address not found anywhere in mapper
      if (WorkSpaceMapper::NOT_FOUND == eRet) {
         m_ErrorString = "Address/Length Pair do not describe a buffer within a known workspace";
         Clear();
         return;
      }
      pSubmit->m_arrDescInfo[index].m_wsid = pWkSp->m_wsid; /* In: the workspace id */
      pSubmit->m_arrDescInfo[index].m_vwsid = pWkSp->m_wsid; /* In: the workspace id: TO BE DEPRECATED*/

      AAL_VERBOSE(LM_UAIA, "Input Context=" << std::hex << pSubmit->m_arrDescInfo[index].m_context << std::endl );
   }

   // Initialize input descriptors, which come immediately after output descriptors
   // Thus, no re-initialization of preq
   for (size_t i = 0; i < numIRecs; ++index, ++i) {
      pSubmit->m_arrDescInfo[index].m_type      = INPUT_DESC;
      pSubmit->m_arrDescInfo[index].m_delim     = ComputeMask(i, numIRecs);
      pSubmit->m_arrDescInfo[index].m_uvptr     = (btVirtAddr)m_viBufDesc[i].m_uvptr; /* In: the user virtual pointer in the workspace */
      pSubmit->m_arrDescInfo[index].m_len       = (size_t)m_viBufDesc[i].m_len; /* In: length of the buffer within the workspace, currently in bytes */
      pSubmit->m_arrDescInfo[index].m_no_notify = !m_viBufDesc[i].m_bRsp;
      pSubmit->m_arrDescInfo[index].m_context   = (btVirtAddr)m_viBufDesc[i].m_context;

      //////////////////////////////////////////////////////////////////////////////////
      // Need to get workspace. Input is a pointer and a length describing a buffer.
      WorkSpaceMapper::pcWkSp_t pWkSp; // will point to a WkSp
      // Get the associated workspace
      WorkSpaceMapper::eWSM_Ret eRet = pCAFUDev->WSM().GetWkSp(m_viBufDesc[i].m_uvptr, &pWkSp,
                                                               m_viBufDesc[i].m_len);

      // Address not found anywhere in mapper
      if (WorkSpaceMapper::NOT_FOUND == eRet) {
         m_ErrorString = "Address/Length Pair do not describe a buffer within a known workspace";
         Clear();
         return;
      }
      pSubmit->m_arrDescInfo[index].m_wsid = pWkSp->m_wsid; /* In: the workspace id */
      pSubmit->m_arrDescInfo[index].m_vwsid = pWkSp->m_wsid; /* In: the workspace id: TO BE DEPRECATED*/
      AAL_VERBOSE(LM_UAIA, "Output Context=" << std::hex << pSubmit->m_arrDescInfo[index].m_context << std::endl );
   }

   /////////////////////////////////////////////////////////////////////////////////////////////////////////////
   m_size = lenBytes;
   m_bIsOK = true;
   m_bRendered = true;

} // SubmitTask_Serial_AFUTransaction::Render

//=============================================================================
// Name:          SubmitTask_Serial_AFUTransaction::ClearRender
// Description:   Destroys the serialized contents of the buffer
// Interface:     private
// Inputs:        none directly. Indirectly, m_payload and m_size and m_bRendered
// Outputs:       none directly. Indirectly, m_payload and m_size and m_bRendered
//                   all cleared, with the previously malloc'd buffer freed
// Comments:      Need to leave the state intact (buffer descriptors, EOT, SOT,
//                   etc.) so that object can be re-rendered.
//=============================================================================
void SubmitTask_Serial_AFUTransaction::ClearRender()
{
   if (m_payload) {
      delete [] m_payload;
      m_payload = NULL;
   }
   m_size      = 0;
   m_bRendered = false;
   m_bIsOK     = false;
}  // SubmitTask_Serial_AFUTransaction::ClearRender


//=============================================================================
// Name:          SubmitTask_Serial_AFUTransaction::Clear
// Description:   Destroys all state but leaves the object intact, ready for
//                   reloaded by the user
// Interface:     public
// Inputs:        none directly.
// Outputs:       none directly.
//=============================================================================
void SubmitTask_Serial_AFUTransaction::Clear()
{
   ClearRender();
   m_viBufDesc.clear();
   m_voBufDesc.clear();

   // Get a new, unique, TransactionID
   TransactionID newTid;
   SetTranID( newTid);
   SetAFUDev(NULL);
}  // CSR_AFUTransaction::ClearRender



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////     SubmitTask_PMaster_AFUTransaction      ///////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


//=============================================================================
// Name:          SubmitTask_PMaster_AFUTransaction::SubmitTask_PMaster_AFUTransaction()
// Description:   Constructor of object that encapsulates the sending of a
//                   single descriptor
// Interface:     public
// Inputs:        described below
// Outputs:       none
// Comments:
//=============================================================================
SubmitTask_PMaster_AFUTransaction::SubmitTask_PMaster_AFUTransaction(
         btVirtAddr           inAddress,  // User mode virtual pointer to input SLAVE MODE buffer
         btWSSize             inLength,   // Length of the input SLAVE MODE buffer
         IAFUDev             *pIAFUDev,   // This transaction is bound to this device
         const TransactionID &rTranID,    // Original TransactionID from Application
         btBool               fResponse,  // Request a response when this descriptor completes,
                                          //    separate from Task Done
         btObjectType         Context     // Context to be returned with response
) :
   m_ErrorString(), m_bIsOK(false), m_payload(NULL), m_size(0)
{
   ConstructWorker(inAddress, inLength, pIAFUDev, rTranID, fResponse, Context);
}

void
SubmitTask_PMaster_AFUTransaction::ConstructWorker(
         btVirtAddr           Address,    // User mode virtual pointer to input SLAVE MODE buffer
         btWSSize             Length,     // Length of the input SLAVE MODE buffer
         IAFUDev             *pIAFUDev,   // This transaction is bound to this device
         const TransactionID &rTranID,    // Original TransactionID from Application
         btBool               fResponse,  // Request a response when this descriptor completes,
                                          //    separate from Task Done
         btObjectType         Context     // Context to be returned with response
)
{
   AAL_VERBOSE(LM_All, "SubmitTask_PMaster_AFUTransaction::ConstructWorker seen.\n");
   size_t lenBytes = sizeof(struct big) + 2 * sizeof(struct desc_share_info);

   // get a new big structure with space for the descriptors, as well
   m_payload = (btVirtAddr)new btByte[lenBytes];
   memset(m_payload, 0, lenBytes);

   //////////////////////////////////////////////////////////////////////////////////////////////
   CAFUDev* pCAFUDev = dynamic_ptr <CAFUDev> (iidCAFUDev,
                                              dynamic_cast <IBase*> (pIAFUDev));

   /*
    * Need to get workspace. Input is a pointer and a length describing a buffer.
    */
   WorkSpaceMapper::pcWkSp_t pWkSp; // will point to a WkSp
   // Get the associated workspace
   WorkSpaceMapper::eWSM_Ret eRet = pCAFUDev->WSM().GetWkSp(Address, &pWkSp, Length);

   // Address not found anywhere in mapper
   if (WorkSpaceMapper::NOT_FOUND == eRet) {
      m_ErrorString
               = "Address/Length Pair do not describe a buffer within a known workspace";
      return;
   }

   // Load the afuMsg
   struct big* pBig     = reinterpret_cast <struct big*> (m_payload);

   pBig->afuMsg.cmd     = AHM_DESC_SUBMIT;
   pBig->afuMsg.payload = (btVirtAddr)&pBig->desc;
   pBig->afuMsg.size    = sizeof(submit_descriptors_req_t) + 2 * sizeof(struct desc_share_info);
   pBig->afuMsg.apiver  = GetAPIVer();
   pBig->afuMsg.pipver  = GetPIPVer();
   AAL_VERBOSE(LM_UAIA, "afuMsg.payload:" <<
            std::hex << pBig->afuMsg.payload <<
            "\n\tm_payload="<<static_cast<void*>(m_payload)<< std::endl
   );

   // Load the submit_descriptors structure
   submit_descriptors_req_t *pSubmit =
            reinterpret_cast<submit_descriptors_req_t *>(pBig->afuMsg.payload);
   pSubmit->m_mode   = MASTER_PHYS_MODE;
   pSubmit->m_tranID = (stTransactionID_t&) rTranID;
   pSubmit->m_nDesc  = 2; /* A pair of Descriptors */

   //////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Temporary for both Descriptors
   desc_share_info_t* pdesc_info;

   // Output Descriptor
   pdesc_info              = &pSubmit->m_arrDescInfo[0];
   pdesc_info->m_type      = OUTPUT_DESC;
   pdesc_info->m_delim     = AHM_DESC_SOT | AHM_DESC_MOT | AHM_DESC_EOT; // both SOT & EOT
   pdesc_info->m_wsid      = 0;
   pdesc_info->m_vwsid     = 0;
   pdesc_info->m_uvptr     = NULL;
   pdesc_info->m_context   = (btVirtAddr)Context;
   pdesc_info->m_no_notify = !fResponse;
   pdesc_info->m_len       = 0;

   AAL_VERBOSE(LM_UAIA, "SubmitTask_PMaster_AFUTransaction: output descriptor submitted is:" <<
            std::hex << std::showbase <<
            "\n\ttype   " << pdesc_info->m_type <<
            "\n\tmode   " << pSubmit->m_mode <<
            "\n\tdelim  " << TDESC_POSITION_from_delim(pdesc_info->m_delim) <<
            "\n\twsid   " << pdesc_info->m_wsid <<
            "\n\tuvptr  " << pdesc_info->m_uvptr <<
            "\n\tlength " << pdesc_info->m_len <<
            "\n\tTranID " << pSubmit->m_tranID
   );

   /////////////////////////////////////////////////////////////////////////////////////////////////////
   // Input Descriptor
   pdesc_info              = &pSubmit->m_arrDescInfo[1];
   pdesc_info->m_type      = INPUT_DESC;
   pdesc_info->m_delim     = AHM_DESC_SOT | AHM_DESC_MOT | AHM_DESC_EOT; // both SOT & EOT
   pdesc_info->m_wsid      = pWkSp->m_wsid;
   pdesc_info->m_vwsid     = pWkSp->m_wsid;
   pdesc_info->m_uvptr     = (btVirtAddr)pWkSp->m_ptr;
   pdesc_info->m_len       = (size_t)Length;
   pdesc_info->m_context   = (btVirtAddr)Context;
   pdesc_info->m_no_notify = !fResponse;

   AAL_VERBOSE(LM_UAIA, "SubmitTask_PMaster_AFUTransaction: input descriptor submitted is:" <<
            std::hex << std::showbase <<
            "\n\ttype   " << pdesc_info->m_type <<
            "\n\tmode   " << pSubmit->m_mode <<
            "\n\tdelim  " << TDESC_POSITION_from_delim(pdesc_info->m_delim) <<
            "\n\twsid   " << pdesc_info->m_wsid <<
            "\n\tuvptr  " << pdesc_info->m_uvptr <<
            "\n\tlength " << pdesc_info->m_len <<
            "\n\tTranID " << pSubmit->m_tranID
   );

   ////////////////////////////////////////////////////////////////////////////////////////////////////
   m_bIsOK = true;
   m_size = lenBytes;
}

SubmitTask_PMaster_AFUTransaction::~SubmitTask_PMaster_AFUTransaction()
{
   if (m_payload) {
      delete [] m_payload;
      m_payload = NULL;
   }
   m_size      = 0;
   m_bIsOK     = true;
}

btVirtAddr SubmitTask_PMaster_AFUTransaction::GetPayloadPtr()
{ 
   if ( IsOK() ) {
      return m_payload;
   } else {
      return NULL;
   }
}

btWSSize SubmitTask_PMaster_AFUTransaction::GetSize()
{ 
   if ( IsOK() ) {
      return m_size;
   } else {
      return 0;
   }
}
TransactionID *SubmitTask_PMaster_AFUTransaction::GetTranIDPtr() { return NULL;               }
uid_msgIDs_e   SubmitTask_PMaster_AFUTransaction::GetCommand()   { return reqid_UID_SendAFU;  }
btID           SubmitTask_PMaster_AFUTransaction::GetPIPVer()    { return AAL_AHMPIP_IID_1_0; }
btID           SubmitTask_PMaster_AFUTransaction::GetAPIVer()    { return AAL_AHMAPI_IID_1_0; }
btBool         SubmitTask_PMaster_AFUTransaction::IsOK()         { return m_bIsOK;            }
std::string    SubmitTask_PMaster_AFUTransaction::GetError()     { return m_ErrorString;      }


//=============================================================================
// Name:          SubmitTask_VMaster_AFUTransaction()
// Description:   Constructor of object that encapsulates the sending of a
//                   single Virtual Master Mode Input descriptor
// Interface:     public
// Inputs:        described below
// Outputs:       none
// Comments:      TODO: Can be made cleaner once Multi-Submit capability is
//                  enabled
//=============================================================================
SubmitTask_VMaster_AFUTransaction::
SubmitTask_VMaster_AFUTransaction (
      btVirtAddr           inAddress,   // User mode virtual pointer to input SLAVE MODE buffer
      btWSSize             inLength,    // Length of the input buffer
      btVirtAddr           wsAddress,   // User mode virtual pointer to VMM Workspace
      IAFUDev             *pIAFUDev,    // This transaction is bound to this device
      const TransactionID &rTranID,     // Original TransactionID from Application
      btBool               fResponse,   // Request a response when this descriptor completes, separate from Task Done
      btObjectType         Context      // Context to be returned with response
      )
	: m_bIsOK(false), m_payload(NULL), m_size(0)
{
   AAL_VERBOSE(LM_All, "SubmitTask_VMaster_AFUTransaction::Constructer seen.\n");
   size_t lenBytes = sizeof(struct big) + 2 * sizeof(struct desc_share_info);

   // get a new big structure with space for the descriptors, as well
   m_payload = (btVirtAddr)new btByte[lenBytes];
   memset(m_payload, 0, lenBytes);


	///////////////////////////////////////////////////////////////////////////////////////////////
   CAFUDev* pCAFUDev = dynamic_ptr<CAFUDev>( iidCAFUDev, dynamic_cast<IBase*>( pIAFUDev));

   /*
    * Need to get Slave Mode input buffer workspace.
    *    Input is the pointer and length describing the buffer.
    */
   WorkSpaceMapper::pcWkSp_t pWkSpSlave = NULL; // will point to a WkSp

   // SPL1 requires a slave mode component
   if ( inAddress ) {
      WorkSpaceMapper::eWSM_Ret eRetSlave = pCAFUDev->WSM().GetWkSp( inAddress, &pWkSpSlave, inLength);

      // Buffer not found anywhere in mapper
      if (  (NULL == pWkSpSlave)                      ||
            (WorkSpaceMapper::NOT_FOUND == eRetSlave) ||
            (  (MASTER_PHYS_MODE != pWkSpSlave->m_task_mode) &&
               (SLAVE_MODE       != pWkSpSlave->m_task_mode) )) {
         m_ErrorString = "Arguments 1 and 2: Address/Length Pair do not describe a Slave or Physical Master Mode buffer within a known workspace";
         return;
      }
   }

   /*
    * Need to get Master Mode input buffer workspace.
    *    Input is just the pointer. Must exactly match beginning.
    */
   WorkSpaceMapper::pcWkSp_t pWkSpMaster = NULL;   // will point to a WkSp
   WorkSpaceMapper::eWSM_Ret eRetMaster = pCAFUDev->WSM().GetWkSp( wsAddress, &pWkSpMaster);

   // Address not found anywhere in mapper
   if ( (NULL == pWkSpMaster)                        ||
        (WorkSpaceMapper::FOUND_EXACT != eRetMaster) ||
        (MASTER_VIRT_MODE != pWkSpMaster->m_task_mode) ) {
      m_ErrorString = "Argument 3: Address does not describe a Virtual Master Mode workspace";
      return;
   }

   // Load the afuMsg
   struct big* pBig = reinterpret_cast<struct big*>(m_payload);

   pBig->afuMsg.cmd     = AHM_DESC_SUBMIT;
   pBig->afuMsg.payload = (btVirtAddr)&pBig->desc;
   pBig->afuMsg.size    = sizeof(submit_descriptors_req_t) + 2 * sizeof(struct desc_share_info);
   pBig->afuMsg.apiver  = GetAPIVer();
   pBig->afuMsg.pipver  = GetPIPVer();
   AAL_VERBOSE(LM_UAIA, "afuMsg.payload:" <<
            std::hex << pBig->afuMsg.payload <<
            "\n\tm_payload="<<static_cast<void*>(m_payload)<< std::endl
   );

   // Load the submit_descriptors structure
   submit_descriptors_req_t *pSubmit =
            reinterpret_cast<submit_descriptors_req_t *>(pBig->afuMsg.payload);
   pSubmit->m_mode   = MASTER_VIRT_MODE;
   pSubmit->m_tranID = (stTransactionID_t&) rTranID;
   pSubmit->m_nDesc  = 2; /* A pair of Descriptors */

   //////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Temporary for both descriptors
   desc_share_info_t* pdesc_info;

   // Output Descriptor
   pdesc_info              = &pSubmit->m_arrDescInfo[0];
   pdesc_info->m_type      = OUTPUT_DESC;
   pdesc_info->m_delim     = AHM_DESC_SOT | AHM_DESC_MOT | AHM_DESC_EOT; // both SOT & EOT
   pdesc_info->m_wsid      = 0;
   pdesc_info->m_vwsid     = 0;
   pdesc_info->m_uvptr     = NULL;
   pdesc_info->m_context   = (btVirtAddr)Context;
   pdesc_info->m_no_notify = !fResponse;
   pdesc_info->m_len       = 0;

   AAL_VERBOSE(LM_UAIA, "SubmitTask_VMaster_AFUTransaction: output descriptor submitted is:" <<
         std::hex << std::showbase <<
         "\n\ttype   " << pdesc_info->m_type <<
         "\n\tmode   " << pSubmit->m_mode <<
         "\n\tdelim  " << TDESC_POSITION_from_delim(pdesc_info->m_delim) <<
         "\n\twsid   " << pdesc_info->m_wsid <<
         "\n\tuvptr  " << pdesc_info->m_uvptr <<
         "\n\tlength " << pdesc_info->m_len <<
         "\n\tTranID " << pSubmit->m_tranID
         );


   /////////////////////////////////////////////////////////////////////////////////////////////////////
   // Input Descriptor
   pdesc_info              = &pSubmit->m_arrDescInfo[1];
   pdesc_info->m_type      = INPUT_DESC;
   pdesc_info->m_delim     = AHM_DESC_SOT | AHM_DESC_MOT | AHM_DESC_EOT; // both SOT & EOT
   pdesc_info->m_wsid      = (NULL == pWkSpSlave) ? -1   : pWkSpSlave->m_wsid;
   pdesc_info->m_vwsid     = pWkSpMaster->m_wsid;
   pdesc_info->m_uvptr     = (NULL == pWkSpSlave) ? NULL : pWkSpSlave->m_ptr;
   pdesc_info->m_len       = inLength;
   pdesc_info->m_context   = (btVirtAddr)Context;
   pdesc_info->m_no_notify = !fResponse;

   AAL_VERBOSE(LM_UAIA, "SubmitTask_VMaster_AFUTransaction: input descriptor submitted is:" <<
         std::hex << std::showbase <<
         "\n\ttype   " << pdesc_info->m_type <<
         "\n\tmode   " << pSubmit->m_mode <<
         "\n\tdelim  " << TDESC_POSITION_from_delim(pdesc_info->m_delim) <<
         "\n\twsid   " << pdesc_info->m_wsid <<
         "\n\tuvptr  " << pdesc_info->m_uvptr <<
         "\n\tlength " << pdesc_info->m_len <<
         "\n\tTranID " << pSubmit->m_tranID
         );

   ////////////////////////////////////////////////////////////////////////////////////////////////////
   m_bIsOK 	= true;
   m_size 	= lenBytes;
}


SubmitTask_VMaster_AFUTransaction::~SubmitTask_VMaster_AFUTransaction()
{
   if (m_payload) {
      delete [] m_payload;
      m_payload = NULL;
   }
   m_size      = 0;
   m_bIsOK     = true;
}

btVirtAddr SubmitTask_VMaster_AFUTransaction::GetPayloadPtr()
{ 
   if ( IsOK() ) {
      return m_payload;
   } else {
      return NULL;
   }
}

btWSSize SubmitTask_VMaster_AFUTransaction::GetSize()
{ 
   if ( IsOK() ) {
      return m_size;
   } else {
      return 0;
   }
}


TransactionID *SubmitTask_VMaster_AFUTransaction::GetTranIDPtr() { return NULL;               }
uid_msgIDs_e   SubmitTask_VMaster_AFUTransaction::GetCommand()   { return reqid_UID_SendAFU;  }
btID           SubmitTask_VMaster_AFUTransaction::GetPIPVer()    { return AAL_AHMPIP_IID_1_0; }
btID           SubmitTask_VMaster_AFUTransaction::GetAPIVer()    { return AAL_AHMAPI_IID_1_0; }
btBool         SubmitTask_VMaster_AFUTransaction::IsOK()         { return m_bIsOK;            }
std::string    SubmitTask_VMaster_AFUTransaction::GetError()     { return m_ErrorString;      }


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////            W O R K S P A C E              ///////////////////
/////////////////                                           ///////////////////
/////////////////      A F U   T R A N S A C T I O N S      ///////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#if 0
//=============================================================================
// Name:          WkSp_Single_Allocate_AFUTransaction:: various functions
// Description:   Encapsulates the request for a single workspace
// Constructor:   Parameter 1: Length of the buffer requested, in bytes
//                Parameter 2: Value of enum TTASK_MODE in AALWorkspace.h
// Comments:      Callback for WkSp_Single_Allocate_AFUTransaction is below
// Notes:         At this time there is no way for an error return to occur,
//                   so there is no dtor. Should there be a need for one,
//                   *m_pWkspMapObj will have to be deleted
//=============================================================================
WkSp_Single_Allocate_AFUTransaction::WkSp_Single_Allocate_AFUTransaction(btWSSize len, TTASK_MODE TaskMode) :
      m_big(), m_bIsOK(false), m_tidEmbedded(), m_pWkspMapObj(NULL), m_ErrorString()
{
   ConstructWorker( len, TaskMode);
}

void WkSp_Single_Allocate_AFUTransaction::ConstructWorker(btWSSize len, TTASK_MODE TaskMode)
{
   memset(&m_big, 0, sizeof(struct big));

//   if (0 == len) {      // Not legal to allocate a zero-length buffer
//      m_ErrorString = "Buffer size must be greater than 0.\n";
//      return;
//   }
   if (0 == len) {      // Not legal to allocate a zero-length buffer, make it one byte long
      m_big.ahmreq.u.wksp.m_size = 1;
   }
   else {
      m_big.ahmreq.u.wksp.m_size = len;
   }

   m_big.afuMsg.cmd     = ((MASTER_VIRT_MODE==TaskMode) ? AHM_WKSP_VALLOC : AHM_WKSP_ALLOC);
   m_big.afuMsg.payload = (btVirtAddr)&m_big.ahmreq;
   m_big.afuMsg.size    = sizeof(struct ahm_req);
   m_big.afuMsg.apiver  = GetAPIVer();
   m_big.afuMsg.pipver  = GetPIPVer();

   m_pWkspMapObj = new WkSp_Allocate_Mapping_EventHandlerObject (TaskMode, len);
   m_tidEmbedded.Context(static_cast<btApplicationContext> (m_pWkspMapObj));
   m_tidEmbedded.Handler(m_pWkspMapObj->Get_Allocate_Mapping_EventHandler());
   m_tidEmbedded.Filter(true);
   m_bIsOK = true;
}


btVirtAddr     WkSp_Single_Allocate_AFUTransaction::GetPayloadPtr() { return reinterpret_cast<btVirtAddr>(&m_big); }
btWSSize       WkSp_Single_Allocate_AFUTransaction::GetSize()       { return sizeof(struct big); }
TransactionID *WkSp_Single_Allocate_AFUTransaction::GetTranIDPtr()  { return &m_tidEmbedded;     }
uid_msgIDs_e   WkSp_Single_Allocate_AFUTransaction::GetCommand()    { return reqid_UID_SendWSM;  }
btID           WkSp_Single_Allocate_AFUTransaction::GetPIPVer()     { return AAL_AHMPIP_IID_1_0; }
btID           WkSp_Single_Allocate_AFUTransaction::GetAPIVer()     { return AAL_AHMAPI_IID_1_0; }
btBool         WkSp_Single_Allocate_AFUTransaction::IsOK()          { return m_bIsOK;            }
std::string    WkSp_Single_Allocate_AFUTransaction::GetError()      { return m_ErrorString;      }


//=============================================================================
// Name:          WkSp_Allocate_Mapping_EventHandlerObject::Get_Allocate_Mapping_EventHandler
// Description:   Returns the address of the event handler
//=============================================================================
btEventHandler WkSp_Allocate_Mapping_EventHandlerObject::Get_Allocate_Mapping_EventHandler()
{
   return WkSp_Allocate_Mapping_EventHandler;
}

WkSp_Allocate_Mapping_EventHandlerObject::~WkSp_Allocate_Mapping_EventHandlerObject() {}

//=============================================================================
// Name:          WkSp_Allocate_Mapping_EventHandlerObject::WkSp_Allocate_Mapping_EventHandler
// Description:   Encapsulates the WkSp_Single_Allocate_AFUTransaction callback
//                   function that must mmap the wsid to get the pointer
//=============================================================================
void
WkSp_Allocate_Mapping_EventHandlerObject::WkSp_Allocate_Mapping_EventHandler(IEvent const &theEvent )
{
   //////////////////////////////////////////////////////////////////////////////////////
   // Get the original transactionID and CAFUDev.
   //
   // See how it was wrapped up in the TranIDWrapper in SendAFUTransaction()
   //
   // Do this first to prevent memory leaks if errors found later on
   //////////////////////////////////////////////////////////////////////////////////////

   // Get the TransactionID
   TransactionID
            wrappingTranID(dynamic_ref <ITransactionEvent>(iidTranEvent, theEvent).TranID());

   // Its context points to a TranIDWrapper
   TranIDWrapper* pWrapper = static_cast <TranIDWrapper*> (wrappingTranID.Context());

   // Get the contents of the TranIDWrapper
   TransactionID origUserTranID(pWrapper->origTranID);
   CAFUDev* pCAFUDev = reinterpret_cast <CAFUDev*> (pWrapper->pCAFUDev);

   // Get the This pointer. It is in the event's context
   WkSp_Allocate_Mapping_EventHandlerObject *This =
            static_cast <WkSp_Allocate_Mapping_EventHandlerObject*> (pWrapper->Context);
   AAL_VERBOSE(LM_UAIA, "WkSp_Allocate_Mapping_EventHandler This = " << static_cast<void*> (This) << std::endl);

   // Delete the TranID Wrapper. It was allocated in SendAFUTransaction
   delete pWrapper;


   //////////////////////////////////////////////////////////////////////////////////////
   // Get objects and pointers required to send the final event
   //////////////////////////////////////////////////////////////////////////////////////

   // Get the AIA object that sent this Event. Use it to get to the EDS and AFU

   uAIA *pAIA = static_cast <uAIA*> (theEvent.pObject());

   AAL_VERBOSE(LM_UAIA, "WkSp_Allocate_Mapping_EventHandler AIA = " << static_cast<void*> (pAIA) << std::endl);

   // Get the AFU pointer. The AFU created the AIA object, and so the AIA object's Context should be the AFU pointer.
   //    Need it only for the 'this' pointer in event delivery, and there it is an IBase, so just get it that way.
   // TODO: enforce putting the pAFU in the context or in the AIA
   // Note that pIBaseAFU will be different from pAFU
#if AAL_LOG_LEVEL >= LOG_VERBOSE
   IBase* pIBaseAFU = static_cast <IBase*> (theEvent.Object().Context());
   AAL_VERBOSE(LM_UAIA, "WkSp_Allocate_Mapping_EventHandler pAFU = " << static_cast<void*> (pIBaseAFU) << std::endl);
#endif

   //////////////////////////////////////////////////////////////////////////////////////
   // Get the actual data to be returned.
   //////////////////////////////////////////////////////////////////////////////////////

   // This Event is a IUIDriverClientEvent. Get the results from the call.
   IUIDriverClientEvent *pevt = dynamic_ptr <IUIDriverClientEvent> (tranevtUIDriverClientEvent, theEvent);

   if (uid_errnumOK == pevt->ResultCode()) {
      // DEBUG CODE
      if (rspid_WSM_Response != pevt->MessageID()) { // expect this to be rspid_WSM_Response
         AAL_ERR(LM_UAIA, "WkSp_Allocate_Mapping_EventHandler MessageID " << pevt->MessageID()
                  << ". LOGIC ERROR, Expected rspid_WSM_Response\n");
         pevt->ResultCode(uid_errnumSystem);
      }

      // Since MessageID is rspid_WSM_Response, Payload is a aalui_WSMEvent.
      struct aalui_WSMEvent *pResult =
               reinterpret_cast<struct aalui_WSMEvent *>(pevt->Payload());

      // DEBUG CODE
      if (uid_wseventAllocate != pResult->evtID) {
         AAL_ERR(LM_UAIA, "WkSp_Allocate_Mapping_EventHandler EventID " << pResult->evtID
                  << ". LOGIC ERROR, Expected uid_wseventAllocate\n");
         pevt->ResultCode(uid_errnumSystem);
      }

      // MMAP the workspace id in order to get the pointer
      btBool fRet;
      fRet = pCAFUDev->MapWSID(pResult->wsParms.size,
                               pResult->wsParms.wsid,
                               &pResult->wsParms.ptr);

      // Fix up the lower 12-bits of  virtual pointer, in case it is not page aligned
      // those 12 bits are in the physptr
      pResult->wsParms.ptr = pResult->wsParms.ptr +
               ( static_cast<btUnsigned64bitInt>(pResult->wsParms.physptr) & 0x00000FFFULL );


      AAL_VERBOSE(LM_UAIA, "WkSp_Allocate_Mapping_EventHandler MMAP result: " << std::boolalpha << fRet <<
               "\n\tsize    " << pResult->wsParms.size <<
               "\n\twsid    " << std::hex << pResult->wsParms.wsid <<
               "\n\tptr     " << std::hex << pResult->wsParms.ptr <<
               "\n\tphysptr " << std::hex << pResult->wsParms.physptr <<
               std::endl);

      pResult->wsParms.type = This->m_task_mode; // Store the task mode into the parms so it can be read in the next handler

      // If can't map, store that in the event
      if (!fRet) {
         pevt->ResultCode(uid_errnumNoMap);
      }
      else {
         // Store the pointer in the map
         fRet = pCAFUDev->WSM().AddToMap(pResult->wsParms.ptr,
                                         pResult->wsParms.size,
                                         This->m_len,                // Might be zero if wsParms.size is 1
                                         pResult->wsParms.wsid,
                                         pResult->wsParms.type,
                                         pResult->wsParms.physptr);

         // Fake out AFU, if requested 0 length buffer, tell it that this is 0 length, even though actually 1
         pResult->wsParms.size = This->m_len;

         if (!fRet) {
            pevt->ResultCode(uid_errnumBadMapping);
         }
      }

      AAL_VERBOSE(LM_UAIA, "WkSp_Allocate_Mapping_EventHandler:Workspace Map after Allocate: " << pCAFUDev->WSM());
   }  // uid_errnumOK == pevt->ResultCode()

   ReThrow(&theEvent.Object(), // Pointer to AIA as IBase*
           theEvent,
           pAIA->getRuntime(),
           pCAFUDev->Handler(),
           &origUserTranID);

   // delete the WkSp_Allocate_Mapping_EventHandlerObject allocated in WkSp_Single_Allocate_AFUTransaction
   delete This;

} // end of WkSp_Allocate_Mapping_EventHandlerObject::WkSp_Allocate_Mapping_EventHandler

//=============================================================================
// Name:          WkSp_Single_Free_AFUTransaction:: various functions
// Description:   Encapsulates the request for a single workspace
// Constructor:   Parameter is the address and length of the buffer to be freed,
//                   as well as the workspace id, currently.
// Comments:      Will attempt to DEPRECATE wsid
//=============================================================================
WkSp_Single_Free_AFUTransaction::WkSp_Single_Free_AFUTransaction(IAFUDev   *pIAFUDev,
                                                                 btVirtAddr Address) :
   m_big(),
   m_bIsOK(false),
   m_tidEmbedded(),
   m_pWkspMapObj(NULL),
   m_ErrorString()
{
   memset(&m_big, 0, sizeof(struct big));

   CAFUDev* pCAFUDev = dynamic_ptr<CAFUDev>( iidCAFUDev, dynamic_cast<IBase*>( pIAFUDev));

   /*
    * Need to get workspace. Input should be only a pointer.
    */
   WorkSpaceMapper::pcWkSp_t pWkSp = NULL;          // will point to a WkSp
   WorkSpaceMapper::eWSM_Ret eRet  = pCAFUDev->WSM().GetWkSp(Address, &pWkSp);

   // Address not found anywhere in mapper
   if ( WorkSpaceMapper::NOT_FOUND == eRet ) {
      m_ErrorString = "Unknown Workspace Address";
      return;
   }
   ASSERT(NULL != pWkSp);

   // Address found inside a block, not at the beginning
   if ( WorkSpaceMapper::FOUND_INCLUDED == eRet ) {
      std::ostringstream oss;
      oss << "Workspace Address " << static_cast<void*>(Address) <<
             " is not the beginning of its workspace. Did you mean to pass " <<
             static_cast<void*>(pWkSp->m_ptr) << "?";
      m_ErrorString = oss.str();
      return;
   }
   // At this point, Address must be equal to pWkSp->m_ptr

   if ( 0 == pWkSp->m_len ) {   // if allocated size is zero, then free it, not a real buffer
      free(pWkSp->m_ptr);
   } else {
      // no error checking for now. munmap() returns MAP_FAILED on error, with errno set
      pCAFUDev->UnMapWSID(pWkSp->m_ptr, pWkSp->m_len, pWkSp->m_wsid);
   }

   m_big.ahmreq.u.wksp.m_wsid = pWkSp->m_wsid;
   m_big.afuMsg.cmd           = ((MASTER_VIRT_MODE==pWkSp->m_task_mode) ? AHM_WKSP_VFREE : AHM_WKSP_FREE);
   m_big.afuMsg.payload       = (btVirtAddr)&m_big.ahmreq;
   m_big.afuMsg.size          = sizeof(struct ahm_req);
   m_big.afuMsg.apiver        = GetAPIVer();
   m_big.afuMsg.pipver        = GetPIPVer();

   m_pWkspMapObj = new WkSp_Free_Mapping_EventHandlerObject(*pWkSp);
   m_tidEmbedded.Context(static_cast<btApplicationContext> (m_pWkspMapObj));
   m_tidEmbedded.Handler(m_pWkspMapObj->Get_Free_Mapping_EventHandler());
   m_tidEmbedded.Filter(true);

   // Free up with workspace map
   pCAFUDev->WSM().RemoveFromMap(pWkSp->m_ptr);
   AAL_VERBOSE(LM_UAIA, "WkSp_Single_Free_AFUTransaction:Workspace Map after Free: " << pCAFUDev->WSM());

   m_bIsOK = true;
}

btVirtAddr     WkSp_Single_Free_AFUTransaction::GetPayloadPtr() { return reinterpret_cast<btVirtAddr>(&m_big); }
btWSSize       WkSp_Single_Free_AFUTransaction::GetSize()       { return sizeof(struct big); }
TransactionID *WkSp_Single_Free_AFUTransaction::GetTranIDPtr()  { return &m_tidEmbedded;     }
uid_msgIDs_e   WkSp_Single_Free_AFUTransaction::GetCommand()    { return reqid_UID_SendWSM;  }
btID           WkSp_Single_Free_AFUTransaction::GetPIPVer()     { return AAL_AHMPIP_IID_1_0; }
btID           WkSp_Single_Free_AFUTransaction::GetAPIVer()     { return AAL_AHMAPI_IID_1_0; }
btBool         WkSp_Single_Free_AFUTransaction::IsOK()          { return m_bIsOK;            }
std::string    WkSp_Single_Free_AFUTransaction::GetError()      { return m_ErrorString;      }

//=============================================================================
// Name:          WkSp_Free_Mapping_EventHandlerObject::WkSp_Free_Mapping_EventHandlerObject
// Description:   CTOR
// Comments:      Could not initialize the m_WSMParms in the standard manner.
//=============================================================================
WkSp_Free_Mapping_EventHandlerObject::WkSp_Free_Mapping_EventHandlerObject(const WorkSpaceMapper::WkSp &rParms)
{
   memset(&m_WSMParms, 0, sizeof(m_WSMParms));
   m_WSMParms.ptr     = rParms.m_ptr;
   m_WSMParms.size    = rParms.m_len;
   m_WSMParms.wsid    = rParms.m_wsid;
   m_WSMParms.physptr = rParms.m_phys_ptr;
   m_WSMParms.type    = rParms.m_task_mode;
}

WkSp_Free_Mapping_EventHandlerObject::~WkSp_Free_Mapping_EventHandlerObject() {}

//=============================================================================
// Name:          WkSp_Free_Mapping_EventHandlerObject::Get_Free_Mapping_EventHandler
// Description:   Returns the address of the event handler
//=============================================================================
btEventHandler WkSp_Free_Mapping_EventHandlerObject::Get_Free_Mapping_EventHandler()
{
   return WkSp_Free_Mapping_EventHandler;
}

//=============================================================================
// Name:          WkSp_Free_Mapping_EventHandlerObject::WkSp_Free_Mapping_EventHandler
// Description:   Encapsulates the WkSp_Single_Allocate_AFUTransaction callback
//                   function that must mmap the wsid to get the pointer
//=============================================================================
void WkSp_Free_Mapping_EventHandlerObject::WkSp_Free_Mapping_EventHandler(IEvent const &theEvent)
{
   //////////////////////////////////////////////////////////////////////////////////////
   // Get objects and pointers required to send the final event
   //////////////////////////////////////////////////////////////////////////////////////

   // get the AIA object that sent this Event. Use it to get to the EDS and AFU
   uAIA *pAIA = static_cast<uAIA*>(theEvent.pObject());

   AAL_VERBOSE(LM_UAIA, "WkSp_Free_Mapping_EventHandler AIA = " << static_cast<void*> (pAIA) << std::endl);

   // Get the AFU pointer. The AFU created the AIA object, and so the AIA object's Context should be the AFU pointer.
   //    Need it only for the 'this' pointer in event delivery, and there it is an IBase, so just get it that way.
   // TODO: enforce putting the pAFU in the context or in the AIA
   // Note that pIBaseAFU will be different from pAFU
#if AAL_LOG_LEVEL >= LOG_VERBOSE
   IBase* pIBaseAFU = static_cast<IBase*>(theEvent.Object().Context());
   AAL_VERBOSE(LM_UAIA, "WkSp_Free_Mapping_EventHandler pAFU = " << static_cast<void*> (pIBaseAFU) << std::endl);
#endif

   //////////////////////////////////////////////////////////////////////////////////////
   // Get the original transactionID and CAFUDev.
   //
   // See how it was wrapped up in the TranIDWrapper in SendAFUTransaction()
   //////////////////////////////////////////////////////////////////////////////////////

   // Get the TransactionID
   TransactionID wrappingTranID(dynamic_ref<ITransactionEvent>(iidTranEvent, theEvent).TranID());

   // Its context points to a TranIDWrapper
   TranIDWrapper* pWrapper = static_cast<TranIDWrapper*> (wrappingTranID.Context());

   // Get the contents of the TranIDWrapper
   TransactionID origUserTranID(pWrapper->origTranID);
   CAFUDev* pCAFUDev = reinterpret_cast<CAFUDev*> (pWrapper->pCAFUDev);

   // Get the This pointer. It is in the event's context
   WkSp_Free_Mapping_EventHandlerObject *This =
         static_cast<WkSp_Free_Mapping_EventHandlerObject*>(pWrapper->Context);
   AAL_VERBOSE(LM_UAIA, "WkSp_Free_Mapping_EventHandler This = " << static_cast<void*> (This) << std::endl);

   // Delete the TranID Wrapper. It was allocated in SendAFUTransaction
   delete pWrapper;


   //////////////////////////////////////////////////////////////////////////////////////
   // Get the actual data to be returned.
   //
   // At this point, meaningful error handling could be introduced
   //////////////////////////////////////////////////////////////////////////////////////

   // This Event is a IUIDriverClientEvent. Get the results from the call.
   IUIDriverClientEvent *pevt = dynamic_ptr<IUIDriverClientEvent> (tranevtUIDriverClientEvent, theEvent);

   // DEBUG CODE
   if (rspid_WSM_Response != pevt->MessageID()) { // expect this to be rspid_WSM_Response
      AAL_ERR(LM_UAIA, "WkSp_Free_Mapping_EventHandler MessageID " << pevt->MessageID()
            << ". LOGIC ERROR, Expected rspid_WSM_Response\n");
      // send exception message somewhere?
      pevt->ResultCode(uid_errnumSystem);
   }

   // Since MessageID is rspid_WSM_Response, Payload is a aalui_WSMEvent.
   struct aalui_WSMEvent *pResult = reinterpret_cast<struct aalui_WSMEvent *>(pevt->Payload());

   // DEBUG CODE
   if (uid_wseventFree != pResult->evtID) {
      AAL_ERR(LM_UAIA, "WkSp_Free_Mapping_EventHandler EventID " << pResult->evtID
            << ". LOGIC ERROR, Expected uid_wseventFree\n");
      // send exception message somewhere?
      pevt->ResultCode(uid_errnumSystem);
   }

   // Store the Workspace information back into the Event, as it was not returned by the
   // driver, but may be useful to the client and is needed to remove the workspace from the mapper

   pResult->wsParms = This->m_WSMParms;

   AAL_VERBOSE(LM_UAIA,  "WkSp_Free_Mapping_EventHandler: " << std::showbase <<
                        "\n\tsize " << std::dec << pResult->wsParms.size <<
                        "\n\twsid " << std::hex << pResult->wsParms.wsid <<
                        "\n\tptr  " << std::hex << (void *)pResult->wsParms.ptr <<
                        std::endl);

   ReThrow( &theEvent.Object(),  // Pointer to AIA as IBase*
            theEvent,
            pAIA->getRuntime(),
            pCAFUDev->Handler(),
            &origUserTranID);

   // delete the WkSp_Free_Mapping_EventHandlerObject allocated in WkSp_Single_Allocate_AFUTransaction
   delete This;

} // end of WkSp_Free_Mapping_EventHandlerObject::WkSp_Free_Mapping_EventHandler

#endif
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////          I M A N A G E M E N T            ///////////////////
/////////////////                                           ///////////////////
/////////////////      A F U   T R A N S A C T I O N S      ///////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


//=============================================================================
// Name:          Activate_AFUTransaction
// Description:   Encapsulates the request to Activate sub-devices on a device
// Parameters:    Bitmap describing sub-devices to Activate:
//                   2^n==1 means to Activate that sub-device
//=============================================================================
Activate_AFUTransaction::Activate_AFUTransaction( btUnsigned64bitInt ActivateMask) :
   m_msg(), m_bIsOK(false), m_tidEmbedded(), m_ErrorString()
{
   memset(&m_msg, 0, sizeof(m_msg));

   // Set up the aalui_AFUmessage
   m_msg.afumsg.apiver  = GetAPIVer();
   m_msg.afumsg.pipver  = GetPIPVer();
   m_msg.afumsg.cmd     = aalui_mafucmd;
   m_msg.afumsg.size    = sizeof(struct mafu_request);
   m_msg.afumsg.payload = (btVirtAddr)&m_msg.mafureq;

   // Set up the mafu_request
   m_msg.mafureq.subdeviceMask = ActivateMask;
   m_msg.mafureq.cmd           = aalui_mafucmdActvateAFU;
   m_msg.mafureq.size          = 0;
   m_msg.mafureq.payload       = NULL;

   // All is good, m_ErrorString is ""
   m_bIsOK = true;
}

btVirtAddr     Activate_AFUTransaction::GetPayloadPtr() { return reinterpret_cast<btVirtAddr>(&m_msg); }
btWSSize       Activate_AFUTransaction::GetSize()       { return sizeof(m_msg);      }
TransactionID *Activate_AFUTransaction::GetTranIDPtr()  { return NULL;               }
uid_msgIDs_e   Activate_AFUTransaction::GetCommand()    { return reqid_UID_SendAFU;  }
btID           Activate_AFUTransaction::GetPIPVer()     { return AAL_AHMPIP_IID_1_0; }
btID           Activate_AFUTransaction::GetAPIVer()     { return AAL_AHMAPI_IID_1_0; }
btBool         Activate_AFUTransaction::IsOK()          { return m_bIsOK;            }
std::string    Activate_AFUTransaction::GetError()      { return m_ErrorString;      }


//=============================================================================
// Name:          DeActivate_AFUTransaction
// Description:   Encapsulates the request to DeActivate sub-devices on a device
// Parameters:    Bitmap describing sub-devices to DeActivate:
//                   2^n==1 means to DeActivate that sub-device
//=============================================================================
DeActivate_AFUTransaction::DeActivate_AFUTransaction( btUnsigned64bitInt DeActivateMask)
   :  m_msg(), m_bIsOK(false), m_tidEmbedded(), m_ErrorString()
{
   memset(&m_msg, 0, sizeof(m_msg));

   // Set up the aalui_AFUmessage
   m_msg.afumsg.apiver  = GetAPIVer();
   m_msg.afumsg.pipver  = GetPIPVer();
   m_msg.afumsg.cmd     = aalui_mafucmd;
   m_msg.afumsg.size    = sizeof(struct mafu_request);
   m_msg.afumsg.payload = (btVirtAddr)&m_msg.mafureq;

   // Set up the mafu_request
   m_msg.mafureq.subdeviceMask = DeActivateMask;
   m_msg.mafureq.cmd           = aalui_mafucmdDeactvateAFU;
   m_msg.mafureq.size          = 0;
   m_msg.mafureq.payload       = NULL;

   // All is good, m_ErrorString is ""
   m_bIsOK = true;
}

btVirtAddr     DeActivate_AFUTransaction::GetPayloadPtr() { return reinterpret_cast<btVirtAddr>(&m_msg); }
btWSSize       DeActivate_AFUTransaction::GetSize()       { return sizeof(m_msg);      }
TransactionID *DeActivate_AFUTransaction::GetTranIDPtr()  { return NULL;               }
uid_msgIDs_e   DeActivate_AFUTransaction::GetCommand()    { return reqid_UID_SendAFU;  }
btID           DeActivate_AFUTransaction::GetPIPVer()     { return AAL_AHMPIP_IID_1_0; }
btID           DeActivate_AFUTransaction::GetAPIVer()     { return AAL_AHMAPI_IID_1_0; }
btBool         DeActivate_AFUTransaction::IsOK()          { return m_bIsOK;            }
std::string    DeActivate_AFUTransaction::GetError()      { return m_ErrorString;      }


//=============================================================================
// Name:          Initialize_AFUTransaction
// Description:   Encapsulates the request to Initialize sub-devices on a device
// Parameters:    Bitmap describing sub-devices to Initialize:
//                   2^n==1 means to Initialize that sub-device
//=============================================================================
Initialize_AFUTransaction::Initialize_AFUTransaction(
      btUnsigned64bitInt InitializeMask,
      btVirtAddr         Payload,
      btWSSize           Size) :
   m_msg(), m_bIsOK(false), m_tidEmbedded(), m_ErrorString()
{
   // Set up the aalui_AFUmessage
   m_msg.afumsg.apiver  = GetAPIVer();
   m_msg.afumsg.pipver  = GetPIPVer();
   m_msg.afumsg.cmd     = aalui_mafucmd;
   m_msg.afumsg.size    = sizeof(struct mafu_request);
   m_msg.afumsg.payload = (btVirtAddr)&m_msg.mafureq;

   // Set up the mafu_request
   m_msg.mafureq.subdeviceMask = InitializeMask;
   m_msg.mafureq.cmd           = aalui_mafucmdInitializeAFU;
   m_msg.mafureq.size          = Size;
   m_msg.mafureq.payload       = Payload;

   // All is good, m_ErrorString is ""
   m_bIsOK = true;
}

btVirtAddr     Initialize_AFUTransaction::GetPayloadPtr() { return reinterpret_cast<btVirtAddr>(&m_msg); }
btWSSize       Initialize_AFUTransaction::GetSize()       { return sizeof(m_msg);      }
TransactionID *Initialize_AFUTransaction::GetTranIDPtr()  { return NULL;               }
uid_msgIDs_e   Initialize_AFUTransaction::GetCommand()    { return reqid_UID_SendAFU;  }
btID           Initialize_AFUTransaction::GetPIPVer()     { return AAL_AHMPIP_IID_1_0; }
btID           Initialize_AFUTransaction::GetAPIVer()     { return AAL_AHMAPI_IID_1_0; }
btBool         Initialize_AFUTransaction::IsOK()          { return m_bIsOK;            }
std::string    Initialize_AFUTransaction::GetError()      { return m_ErrorString;      }

//=============================================================================
// Name:          Free_AFUTransaction
// Description:   Encapsulates the request to Free sub-devices on a device
// Parameters:    Bitmap describing sub-devices to Free:
//                   2^n==1 means to Free that sub-device
//=============================================================================
Free_AFUTransaction::Free_AFUTransaction( btUnsigned64bitInt FreeMask)
   :  m_msg(), m_bIsOK(false), m_tidEmbedded(), m_ErrorString()
{
   memset(&m_msg, 0, sizeof(m_msg));

   // Set up the aalui_AFUmessage
   m_msg.afumsg.apiver  = GetAPIVer();
   m_msg.afumsg.pipver  = GetPIPVer();
   m_msg.afumsg.cmd     = aalui_mafucmd;
   m_msg.afumsg.size    = sizeof(struct mafu_request);
   m_msg.afumsg.payload = (btVirtAddr)&m_msg.mafureq;

   // Set up the mafu_request
   m_msg.mafureq.subdeviceMask = FreeMask;
   m_msg.mafureq.cmd           = aalui_mafucmdFreeAFU;
   m_msg.mafureq.size          = 0;
   m_msg.mafureq.payload       = NULL;

   // All is good, m_ErrorString is ""
   m_bIsOK = true;
}

btVirtAddr     Free_AFUTransaction::GetPayloadPtr() { return reinterpret_cast<btVirtAddr>(&m_msg); }
btWSSize       Free_AFUTransaction::GetSize()       { return sizeof(m_msg);      }
TransactionID *Free_AFUTransaction::GetTranIDPtr()  { return NULL;               }
uid_msgIDs_e   Free_AFUTransaction::GetCommand()    { return reqid_UID_SendAFU;  }
btID           Free_AFUTransaction::GetPIPVer()     { return AAL_AHMPIP_IID_1_0; }
btID           Free_AFUTransaction::GetAPIVer()     { return AAL_AHMAPI_IID_1_0; }
btBool         Free_AFUTransaction::IsOK()          { return m_bIsOK;            }
std::string    Free_AFUTransaction::GetError()      { return m_ErrorString;      }


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////     F A P - V 1 . 0 - S P E C I F I C     ///////////////////
/////////////////      P I P   T R A N S A C T I O N S      ///////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//XXXXXXXXXXXXXXXXXX                                     XXXXXXXXXXXXXXXXXXXXXX
//XXXXXXXXXXXXXXX            CSR_AFUTransaction             XXXXXXXXXXXXXXXXXXX
//XXXXXXXXXXXXXXXXXX                                     XXXXXXXXXXXXXXXXXXXXXX
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

//=============================================================================
// Name:          CSR_AFUTransaction Default Constructor
// Description:   Destructor
// Interface:     public
// Inputs:        none
// Outputs:       none
// Comments:      virtual
//=============================================================================
CSR_AFUTransaction::CSR_AFUTransaction()
:  m_vGetCSR(),
   m_vSetCSR(),
   m_payload(NULL),
   m_size(0),
   m_bRendered(false),  // At startup nothing as been rendered
   m_bIsOK(true),       // At startup everything is okay
   m_ErrorString()
{
}  // CSR_AFUTransaction::CSR_AFUTransaction

TransactionID *CSR_AFUTransaction::GetTranIDPtr()  { return NULL; }
uid_msgIDs_e   CSR_AFUTransaction::GetCommand()    { return reqid_UID_SendAFU; }
btID           CSR_AFUTransaction::GetPIPVer()     { return AAL_AHMPIP_IID_1_0; }
btID           CSR_AFUTransaction::GetAPIVer()     { return AAL_AHMAPI_IID_1_0; }
btBool         CSR_AFUTransaction::IsOK()          { return m_bIsOK; }
std::string    CSR_AFUTransaction::GetError()      { return m_ErrorString; }


//=============================================================================
// Name:          CSR_AFUTransaction::~CSR_AFUTransaction()
// Description:   Destructor
// Interface:     public
// Inputs:        none
// Outputs:       none
// Comments:      virtual
//=============================================================================
CSR_AFUTransaction::~CSR_AFUTransaction()
{
   Clear();
}  // CSR_AFUTransaction::~CSR_AFUTransaction

//=============================================================================
// Name:          CSR_AFUTransaction::GetPayloadPtr
// Description:   Causes rendering to occur, and a pointer to the rendered
//                   block is returned
// Interface:     public
// Inputs:        none
// Outputs:       none
// Comments:      If rendering has already occurred, it is not done again
//=============================================================================
btVirtAddr CSR_AFUTransaction::GetPayloadPtr()
{
   if (!m_bRendered) Render();
   if ( IsOK() ) {
      return m_payload;
   } else {
      return NULL;
   }
}  // CSR_AFUTransaction::GetPayloadPtr

//=============================================================================
// Name:          CSR_AFUTransaction::GetSize
// Description:   Causes rendering to occur, and the size of the rendered
//                   block is returned
// Interface:     public
// Inputs:        none
// Outputs:       none
// Comments:      If rendering has already occurred, it is not done again
//=============================================================================
btWSSize CSR_AFUTransaction::GetSize()
{
   if (!m_bRendered) Render();
   if ( IsOK() ) {
      return m_size;
   } else {
      return 0;
   }
}  // CSR_AFUTransaction::GetSize()

//=============================================================================
// Name:          CSR_AFUTransaction::AddGetCSR
// Description:   Adds a CSR to Get when this transaction is sent
// Interface:     public
// Inputs:        AFU CSR offset, e.g. 0, 1, 2 ...
// Outputs:       none
// Comments:      If rendering has already occurred, the rendering is destroyed
//=============================================================================
void CSR_AFUTransaction::AddGetCSR(btCSROffset CSR)
{
   if (m_bRendered) Clear();
   csr_offset_value t;
   t.csr_offset = CSR;
   t.csr_value = 0;
   m_vGetCSR.push_back(t);
}  // CSR_AFUTransaction::AddGetCSR

//=============================================================================
// Name:          CSR_AFUTransaction::AddSetCSR
// Description:   Adds a CSR to Set when this transaction is sent
// Interface:     public
// Inputs:        AFU CSR offset, e.g. 0, 1, 2 ..., and the value to Set it to.
// Outputs:       none
// Comments:      If rendering has already occurred, the rendering is destroyed
//=============================================================================
void CSR_AFUTransaction::AddSetCSR(btCSROffset CSR, btCSRValue Value)
{
   if (m_bRendered) Clear();
   csr_offset_value t;
   t.csr_offset = CSR;
   t.csr_value  = Value;
   m_vSetCSR.push_back(t);
}  // CSR_AFUTransaction::AddSetCSR

//=============================================================================
// Name:          CSR_AFUTransaction::Render
// Description:   Renders (serializes) the object into a buffer
// Interface:     private
// Inputs:        none directly. Indirectly m_vBufDesc
// Outputs:       none directly. Indirectly, m_payload and m_size and m_bRendered
//                   set, with m_payload pointing to a malloc'd buffer of length
//                   m_size.
// Comments:      If rendering has already occurred, it is not repeated
//=============================================================================
void CSR_AFUTransaction::Render()
{
   if (m_bRendered) return;
   m_bIsOK = false;

   btWSSize numGetRecs = m_vGetCSR.size();
   btWSSize numSetRecs = m_vSetCSR.size();

   //------------------------------------------------------------------------------------
   // Construct a contiguous buffer that holds the AFU message and the CSR block payload
   //------------------------------------------------------------------------------------
   btWSSize lenCSRBlock = sizeof(struct csr_read_write_blk)
                       + (numGetRecs+numSetRecs)*sizeof(csr_offset_value);
   btWSSize lenPayload  = sizeof(struct aalui_AFUmessage) + lenCSRBlock;

   m_payload = (btVirtAddr)new btByte[(btUnsignedInt)lenPayload];
   memset(m_payload, 0, (size_t)lenPayload);

   struct aalui_AFUmessage *pipMsg = reinterpret_cast<struct aalui_AFUmessage*>(m_payload);

   pipMsg->cmd     = fappip_afucmdCSR_GETSET;
   pipMsg->payload = reinterpret_cast<btVirtAddr>(pipMsg) + sizeof(struct aalui_AFUmessage);
   pipMsg->size    = lenCSRBlock;
   pipMsg->apiver  = GetAPIVer();
   pipMsg->pipver  = GetPIPVer();

   // Load the csr_read_write
   csr_read_write_blk *pcsrStruct = reinterpret_cast<csr_read_write_blk *>(pipMsg->payload);

   pcsrStruct->num_to_get = numGetRecs;
   pcsrStruct->num_to_set = numSetRecs;

   // Load in the Get Records, then the Set Records
   btUnsignedInt n;
   for ( n = 0 ; n < (btUnsignedInt)(numGetRecs + numSetRecs) ; ++n ) {
      if (n<numGetRecs) {
         pcsrStruct->csr_array[n] = m_vGetCSR[n];
      }
      else {
         pcsrStruct->csr_array[n] = m_vSetCSR[n - (btUnsignedInt)numGetRecs];
      }
   }

   m_size      = lenPayload;
   m_bRendered = true;
   m_bIsOK     = true;

}  // CSR_AFUTransaction::Render

//=============================================================================
// Name:          CSR_AFUTransaction::ClearRender
// Description:   Destroys the serialized contents of the buffer
// Interface:     private
// Inputs:        none directly. Indirectly, m_payload and m_size and m_bRendered
// Outputs:       none directly. Indirectly, m_payload and m_size and m_bRendered
//                   all cleared, with the previously malloc'd buffer freed
// Comments:      Need to leave the state intact (CSR vectors)
//=============================================================================
void CSR_AFUTransaction::ClearRender()
{
   if ( m_payload ) {
      delete[] m_payload;
      m_payload = NULL;
   }
   m_size      = 0;
   m_bRendered = false;
   m_bIsOK     = true;
}  // CSR_AFUTransaction::ClearRender


//=============================================================================
// Name:          CSR_AFUTransaction::Clear
// Description:   Destroys all state but leaves the object intact, ready for
//                   reloaded by the user
// Interface:     public
// Inputs:        none directly.
// Outputs:       none directly.
//=============================================================================
void CSR_AFUTransaction::Clear()
{
   ClearRender();
   m_vGetCSR.clear();
   m_vSetCSR.clear();
}  // CSR_AFUTransaction::ClearRender



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////            OPTIMIZED SIGNALING            ///////////////////
/////////////////                                           ///////////////////
/////////////////      A F U   T R A N S A C T I O N S      ///////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//=============================================================================
// Name:          Sig_MapMMIO_Space_AFUTransaction::
// Description:   Causes the device's Control and Status Registers to be
//                Mapped into user space.
// Constructor:
// Comments:      Returns and CSR manipulator specifc to this implementation.
//=============================================================================
Sig_MapMMIO_Space_AFUTransaction::Sig_MapMMIO_Space_AFUTransaction() :
      m_big(), m_bIsOK(false), m_tidEmbedded(), m_pEventHandler(NULL), m_ErrorString()
{

   memset(&m_big, 0, sizeof(struct big));

   m_big.afuMsg.cmd           = fappip_getMMIORmap;
   m_big.afuMsg.payload       = (btVirtAddr)&m_big.ahmreq;
   m_big.afuMsg.size          = sizeof(struct ahm_req);
   m_big.afuMsg.apiver        = GetAPIVer();
   m_big.afuMsg.pipver        = GetPIPVer();
   m_big.ahmreq.u.wksp.m_wsid = WSID_MAP_MMIOR;

   m_pEventHandler = new Sig_MapCSRSpace_EventHandlerObject();
   m_tidEmbedded.Context(static_cast<btApplicationContext>(m_pEventHandler));
   m_tidEmbedded.Handler(m_pEventHandler->GetEventHandler());
   m_tidEmbedded.Filter(true);
   m_bIsOK = true;
}

btVirtAddr     Sig_MapMMIO_Space_AFUTransaction::GetPayloadPtr() { return reinterpret_cast<btVirtAddr>(&m_big); }
btWSSize       Sig_MapMMIO_Space_AFUTransaction::GetSize()       { return sizeof(struct big); }
TransactionID *Sig_MapMMIO_Space_AFUTransaction::GetTranIDPtr()  { return &m_tidEmbedded;     }
uid_msgIDs_e   Sig_MapMMIO_Space_AFUTransaction::GetCommand()    { return reqid_UID_SendAFU;  }
btID           Sig_MapMMIO_Space_AFUTransaction::GetPIPVer()     { return AAL_AHMPIP_IID_1_0; }
btID           Sig_MapMMIO_Space_AFUTransaction::GetAPIVer()     { return AAL_AHMAPI_IID_1_0; }
btBool         Sig_MapMMIO_Space_AFUTransaction::IsOK()          { return m_bIsOK;            }
std::string    Sig_MapMMIO_Space_AFUTransaction::GetError()      { return m_ErrorString;      }

//=============================================================================
// Name:          Sig_MapUMSGpace_AFUTransaction::
// Description:   Causes the device's Control and Status Registers to be
//                Mapped into user space.
// Constructor:
// Comments:      Returns and CSR manipulator specifc to this implementation.
//=============================================================================
Sig_MapUMSGpace_AFUTransaction::Sig_MapUMSGpace_AFUTransaction() :
      m_big(), m_bIsOK(false), m_tidEmbedded(), m_pEventHandler(NULL), m_ErrorString()
{

   memset(&m_big, 0, sizeof(struct big));

   m_big.afuMsg.cmd           = fappip_getuMSGmap;
   m_big.afuMsg.payload       = (btVirtAddr)&m_big.ahmreq;
   m_big.afuMsg.size          = sizeof(struct ahm_req);
   m_big.afuMsg.apiver        = GetAPIVer();
   m_big.afuMsg.pipver        = GetPIPVer();
   m_big.ahmreq.u.wksp.m_wsid = WSID_MAP_UMSG;

   m_pEventHandler = new Sig_MapCSRSpace_EventHandlerObject();
   m_tidEmbedded.Context(static_cast<btApplicationContext>(m_pEventHandler));
   m_tidEmbedded.Handler(m_pEventHandler->GetEventHandler());
   m_tidEmbedded.Filter(true);
   m_bIsOK = true;
}

btVirtAddr     Sig_MapUMSGpace_AFUTransaction::GetPayloadPtr() { return reinterpret_cast<btVirtAddr>(&m_big); }
btWSSize       Sig_MapUMSGpace_AFUTransaction::GetSize()       { return sizeof(struct big); }
TransactionID *Sig_MapUMSGpace_AFUTransaction::GetTranIDPtr()  { return &m_tidEmbedded;     }
uid_msgIDs_e   Sig_MapUMSGpace_AFUTransaction::GetCommand()    { return reqid_UID_SendAFU;  }
btID           Sig_MapUMSGpace_AFUTransaction::GetPIPVer()     { return AAL_AHMPIP_IID_1_0; }
btID           Sig_MapUMSGpace_AFUTransaction::GetAPIVer()     { return AAL_AHMAPI_IID_1_0; }
btBool         Sig_MapUMSGpace_AFUTransaction::IsOK()          { return m_bIsOK;            }
std::string    Sig_MapUMSGpace_AFUTransaction::GetError()      { return m_ErrorString;      }


//=============================================================================
// Name:          Sig_MapCSRSpace_AFUTransaction::
// Description:   Causes the device's Control and Status Registers to be
//                Mapped into user space.
// Constructor:
// Comments:      Returns and CSR manipulator specifc to this implementation.
//=============================================================================
Sig_MapCSRSpace_AFUTransaction::Sig_MapCSRSpace_AFUTransaction(btWSID id) :
      m_big(), m_bIsOK(false), m_tidEmbedded(), m_pEventHandler(NULL), m_ErrorString()
{

   memset(&m_big, 0, sizeof(struct big));

   m_big.afuMsg.cmd           = fappip_getCSRmap;
   m_big.afuMsg.payload       = (btVirtAddr)&m_big.ahmreq;
   m_big.afuMsg.size          = sizeof(struct ahm_req);
   m_big.afuMsg.apiver        = GetAPIVer();
   m_big.afuMsg.pipver        = GetPIPVer();
   m_big.ahmreq.u.wksp.m_wsid = id;

   m_pEventHandler = new Sig_MapCSRSpace_EventHandlerObject();
   m_tidEmbedded.Context(static_cast<btApplicationContext>(m_pEventHandler));
   m_tidEmbedded.Handler(m_pEventHandler->GetEventHandler());
   m_tidEmbedded.Filter(true);
   m_bIsOK = true;
}

btVirtAddr     Sig_MapCSRSpace_AFUTransaction::GetPayloadPtr() { return reinterpret_cast<btVirtAddr>(&m_big); }
btWSSize       Sig_MapCSRSpace_AFUTransaction::GetSize()       { return sizeof(struct big); }
TransactionID *Sig_MapCSRSpace_AFUTransaction::GetTranIDPtr()  { return &m_tidEmbedded;     }
uid_msgIDs_e   Sig_MapCSRSpace_AFUTransaction::GetCommand()    { return reqid_UID_SendAFU;  }
btID           Sig_MapCSRSpace_AFUTransaction::GetPIPVer()     { return AAL_AHMPIP_IID_1_0; }
btID           Sig_MapCSRSpace_AFUTransaction::GetAPIVer()     { return AAL_AHMAPI_IID_1_0; }
btBool         Sig_MapCSRSpace_AFUTransaction::IsOK()          { return m_bIsOK;            }
std::string    Sig_MapCSRSpace_AFUTransaction::GetError()      { return m_ErrorString;      }


//=============================================================================
// Name:          Sig_MapCSRSpace_EventHandlerObject::Get__EventHandler
// Description:   Returns the address of the event handler
//=============================================================================
btEventHandler Sig_MapCSRSpace_EventHandlerObject::GetEventHandler()
{
   return Sig_MapCSRSpace_EventHandlerObject::MapCSRSpace_EventHandler;
}

Sig_MapCSRSpace_EventHandlerObject::Sig_MapCSRSpace_EventHandlerObject() {/*empty*/}
Sig_MapCSRSpace_EventHandlerObject::~Sig_MapCSRSpace_EventHandlerObject() {/*empty*/}

//=============================================================================
// Name:        Sig_MapCSRSpace_EventHandlerObject::EventHandler
// Description:
//=============================================================================
void Sig_MapCSRSpace_EventHandlerObject::MapCSRSpace_EventHandler(IEvent const &theEvent)
{
   ////////////////////////////////////////////////////////////////////////
   // Get the original transactionID and CAFUDev.
   //
   // Wrapped up in the TranIDWrapper in SendAFUTransaction()
   //
   // Get the data out of the wrapper and delete the wrapper ASAP to
   //   avoid possible memory leaks.
   /////////////////////////////////////////////////////////////////////////

   // Get the TransactionID from this event. This is the top level Transaction ID.
   //  Within the transaction ID's context is a payload containing information
   //  necessary to complete the job.
   TransactionID
            wrappingTranID(dynamic_ref<ITransactionEvent>(iidTranEvent, theEvent).TranID());

   // wrappingTranID context points to a TranIDWrapper object created in
   //  SendAFUTransaction. The canonical pattern for AFUTransaction processing is
   //  that the CAFUDev and the original TransactionID passed when the AFU function
   //  was invoked (i.e., the application's TransactionID) is passed in a TranIDWrapper
   //  object.  The TranIDWrapper is carried in the context of other TransactionIDs as a
   //  way of keeping the information handy as a job goes through its various states.
   TranIDWrapper* pWrapper = static_cast <TranIDWrapper*> (wrappingTranID.Context());

   // This is the original Application TrasnactionID passed when all of this was started
   TransactionID origUserTranID(pWrapper->origTranID);

   // CAFUDev of this device
   CAFUDev* pCAFUDev = reinterpret_cast <CAFUDev*> (pWrapper->pCAFUDev);

   // Get the This pointer. It is in the wrapper's context
   Sig_MapCSRSpace_EventHandlerObject *This =
            static_cast <Sig_MapCSRSpace_EventHandlerObject*> (pWrapper->Context);
   AAL_VERBOSE(LM_UAIA, "Sig_MapCSRSpace_EventHandlerObject This = " << static_cast<void*> (This) << std::endl);

   // Delete the TranID Wrapper. It was allocated in SendAFUTransaction
   delete pWrapper;


   //////////////////////////////////////////////////////////////////////////////////////
   // Get objects and pointers required to send the final event
   //////////////////////////////////////////////////////////////////////////////////////

   // Get the AIA object that sent this Event. Use it to get to the EDS and AFU
   uAIA *pAIA =static_cast <uAIA*> (theEvent.pObject());

   AAL_VERBOSE(LM_UAIA, "Sig_MapCSRSpace_EventHandlerObject AIA = " << static_cast<void*> (pAIA) << std::endl);

   // Get the AFU pointer. The AFU created the AIA object, and so the AIA object's Context should be the AFU pointer.
   //    Need it only for the 'this' pointer in event delivery, and there it is an IBase, so just get it that way.
   // TODO: enforce putting the pAFU in the context or in the AIA
   // Note that pIBaseAFU will be different from pAFU
#if AAL_LOG_LEVEL >= LOG_VERBOSE
   IBase* pIBaseAFU = static_cast <IBase*> (theEvent.Object().Context());
   AAL_VERBOSE(LM_UAIA, "Sig_MapCSRSpace_EventHandlerObject pAFU = " << static_cast<void*> (pIBaseAFU) << std::endl);
#endif

   //////////////////////////////////////////////////////////////////////////////////////
   // Get the actual data to be returned.
   //////////////////////////////////////////////////////////////////////////////////////

   // This Event is a IUIDriverClientEvent. Get the results from the call.
   IUIDriverClientEvent *pevt = dynamic_ptr <IUIDriverClientEvent> (tranevtUIDriverClientEvent, theEvent);
   if(pevt==NULL){
      AAL_ERR(LM_UAIA, "Sig_MapCSRSpace_EventHandlerObject Invalid event\n.LOGIC ERROR \n");
   }

   if (NULL != pevt && uid_errnumOK == pevt->ResultCode()) {
      // DEBUG CODE
      if (rspid_WSM_Response != pevt->MessageID()) { // expect this to be rspid_WSM_Response
         AAL_ERR(LM_UAIA, "Sig_MapCSRSpace_EventHandlerObject MessageID " << pevt->MessageID()
                  << ". LOGIC ERROR, Expected rspid_WSM_Response\n");
         pevt->ResultCode(uid_errnumSystem);
      }

      // Since MessageID is rspid_WSM_Response, Payload is a aalui_WSMEvent.
      struct aalui_WSMEvent *pResult =
               reinterpret_cast<struct aalui_WSMEvent *>(pevt->Payload());

      // DEBUG CODE
      if (uid_wseventCSRMap != pResult->evtID) {
         AAL_ERR(LM_UAIA, "Sig_MapCSRSpace_EventHandlerObject EventID " << pResult->evtID
                  << ". LOGIC ERROR, Expected uid_wseventCSRMap\n");
         pevt->ResultCode(uid_errnumSystem);
      }

      // Map the 2 apertures into user space
      btBool fRet;
      fRet = pCAFUDev->MapWSID(pResult->wsParms.size,
                               pResult->wsParms.wsid,
                               &pResult->wsParms.ptr);

      // Fix up the lower 12-bits of  virtual pointer, in case it is not page aligned
      // those 12 bits are in the physptr
      //
      // XXX could write a CSR here ((char *)pResult->wsParms.ptr + 0x920) = 1;
      // e00 is added in here via physptr.  offset 74000 into read ?
      pResult->wsParms.ptr = pResult->wsParms.ptr +
               ( static_cast<btUnsigned64bitInt>(pResult->wsParms.physptr) & 0x00000FFFULL );
#if 0
      AAL_VERBOSE(LM_UAIA, "Sig_MapCSRSpace_EventHandlerObject MMAP result: " << std::boolalpha << fRet <<
               "\n\tsize    " << pResult->wsParms.size <<
               "\n\twsid    " << std::hex << pResult->wsParms.wsid <<
               "\n\tptr     " << std::hex << pResult->wsParms.ptr <<
               "\n\tphysptr " << std::hex << pResult->wsParms.physptr <<
               endl);
#endif
      // If can't map, store that in the event
      if (!fRet) {
         pevt->ResultCode(uid_errnumNoMap);
      }
   }
   ReThrow(&theEvent.Object(), // Pointer to AIA as IBase*
            theEvent,
            pAIA->getRuntime(),
            pCAFUDev->Handler(),
            &origUserTranID);

   // delete the Sig_MapCSRSpace_EventHandlerObject
   delete This;

} // end of Sig_MapCSRSpace_EventHandlerObject::Sig_MapCSRSpace_EventHandlerObject

END_NAMESPACE(AAL)

