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
/// @file FAP10.h
/// @brief Defines FAP v1.0 Service for the Universal AIA.
/// @ingroup uAIA
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Henry Mitchel, Intel Corporation
///          Joseph Grecco, Intel Corporation
///          Alvin, Chen,   Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 12/01/2008     HM       Moved FAP10 AFUDev declarations from uAIA.h
/// 12/14/2008     HM       Allocate and Free done
/// 12/15/2008     HM       All new WkSp_Single_Allocate_AFUTransaction
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
/// 03/17/2009     JG       Modified CASR_AFUTransactionfor new GetSet
///                            structure
/// 05/09/2009     HM       Added virtual destructors to objects that needed
///                            them.
/// 05/11/2009     HM       Fixed bug introduced on 5/9 (duh)
/// 05/15/2009     HM       Added AFUTransaction to support Activate/DeActivate
///                            for Management AFU
/// 05/19/2009     HM       Added AFUTransaction to support Initialize/Free
///                            for Management AFU
/// 06/17/2009     AC       Remove wsid from SubmitTask_Serial_AFUTransaction and
///                            BufferDescriptor
/// 06/17/2009     AC       Move Render to public
/// 08/16/2009     HM       Added support for Zero-Length buffers by providing
///                            a Requested_Length as well as an actual length
///                         For Requested_Length==0, actual length is set to 1.
/// 10/22/2009     JG       Added aalmafu.h
/// 12/28/2009     JG       Added MapCSR Transactions
/// 09/17/2010     HM       Format cleanup
/// 03/05/2012     JG       Added SPL2_Start_AFUTransaction@endverbatim
//****************************************************************************
#ifndef __AALSDK_FAPTRANS_FAP10_H__
#define __AALSDK_FAPTRANS_FAP10_H__
#include <aalsdk/kernel/aalui.h>                // aalui_WSMParms, only a convenience
#include <aalsdk/kernel/aalids.h>               // aalui_WSMParms, only a convenience
#include <aalsdk/kernel/aalmafu.h>              // MAFU interface
#include <aalsdk/kernel/fappip.h>               // struct ahm_req, csr_offset_value

#include <aalsdk/AALTypes.h>
#include <aalsdk/uAIALib.h>
#include <aalsdk/utils/AALWorkSpaceUtilities.h> // WorkSpaceMapper::WkSp
#include <aalsdk/osal/OSServiceModule.h>


AAL_DECLARE_MOD(libFAPTrans1, FAPTRANS1_API)


BEGIN_NAMESPACE(AAL)

   //=============================================================================
   // Name:          TDESC_POSITION_from_delim()
   // Description:   Encapsulates conversion of a kernel structure to a user-
   //                   viewable enumeration
   //=============================================================================
   FAPTRANS1_API
   TDESC_POSITION TDESC_POSITION_from_delim(btUnsigned16bitInt delim);

   /*
    * Theory of operation for an AFUTransaction Object:
    *
    *    The user of the object creates it, and uses AddBuffer() to add buffer
    *    descriptions to it in the order that it wishes them to be delivered to
    *    the device.
    *
    *    The user sets the StartTask and EndTask bits to denote whether or not
    *    the set of descriptors starts or ends a task.
    *
    *    In the first version, multiple tasks cannot be submitted in one object,
    *    primarily to increase the safety for the user. The default action is for
    *    all of the buffers in a transaction to be members of that transaction,
    *    and for the first to be SOT (Start of Task) and the last to be EOT (End
    *    of Task).
    *
    *    The SOT/EOT can be over-ridden explicitly by a user wishing to stream
    *    descriptors through multiple transaction objects. The SOT/EOT will have
    *    to be over-ridden for EACH transaction in the stream. (We can add a stream
    *    default version that defaults to MOT (Middle of Transaction) if there is
    *    demand for it).
    *
    *    The receiver of the object uses GetSize() and GetPayloadPtr() to retrieve
    *    the rendered buffer descriptors. The PIP and API versions are also available.
    *
    *    It is likely that the size will not be known until the object is rendered,
    *    so a call to either GetSize() or GetPayloadPtr() will cause the object to be
    *    rendered.
    *
    *    A subsequent call to AddBuffer will destroy the rendering and add the
    *    buffer to the descriptor list.
    *
    *    The user can Clear() the Transaction in order to reuse it.
    */

   //=============================================================================
   // Name:          BufferDescriptor, BufferDescriptor_t, BufDescVec_t
   // Description:   Encapsulates the user view of a buffer
   // Comments:      Workspace ID (wsid) is currently needed, but it is derivable
   //                   from the pointer, is redundant, and error-prone
   //=============================================================================

   typedef struct FAPTRANS1_API BufferDescriptor
   {
      btVirtAddr    m_uvptr;   // user virtual pointer
      btWSSize      m_len;     // length of buffer
      btObjectType  m_context; // context pointer
      btBool        m_bRsp;    // Request a response when this descriptor
                               //    completes, separate from Task Done

      BufferDescriptor(btVirtAddr ptr, btWSSize len, btObjectType ctx, btBool rsp = false) :
         m_uvptr(ptr),
         m_len(len),
         m_context(ctx),
         m_bRsp(rsp)
      {}

   } BufferDescriptor_t;

   typedef std::vector<BufferDescriptor_t> BufDescVec_t;


   ///////////////////////////////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////////////////////////////
   ////////////////////                                     //////////////////////
   /////////////////           S U B M I T   T A S K           ///////////////////
   /////////////////                                           ///////////////////
   /////////////////      A F U   T R A N S A C T I O N S      ///////////////////
   ////////////////////                                     //////////////////////
   ///////////////////////////////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////////////////////////////

   //=============================================================================
   // Name:          SubmitTask_Serial_AFUTransaction
   // Description:   Encapsulates the sending of a list of buffer descriptors to
   //                   an FSB/QPI PIP in slave (or serial) mode, using 1.0
   //                   interface and semantics
   // Comments:      After sending this, the response will be in a
   //                   tranevtUIDriverClientEvent,
   //                      with MessageID()==rspid_AFU_Response, and
   //                      no Payload().
   //                   TODO: the Payload should be the 32-byte return, and some
   //                      flags
   //                In the Sample_MemCpyAFU, this is converted into
   //                   a tranevtSampleMemCpyTaskComplete
   // WARNING: NEEDS TO BE UPDATED TO HANDLE fResponse and Context in the downlink
   //=============================================================================

   class FAPTRANS1_API SubmitTask_Serial_AFUTransaction : public IAFUTransaction
   {
   public:
      // Used by the user to set up the descriptor list
                        SubmitTask_Serial_AFUTransaction();
      virtual          ~SubmitTask_Serial_AFUTransaction();

                        // Add a buffer to the set of descriptors
      void               AddBuffer(btBool       fOutput,          // Is this an output descriptor?
                                   btVirtAddr   Address,          // User mode virtual pointer to buffer
                                   btWSSize     Length,           // Length of the buffer
                                   btObjectType Context = NULL,   // Descriptor context
                                   btBool       fRsp    = false); // Request a response when this descriptor completes,
                                                                       //    separate from Task Done
      void               StartTask(btBool sot)                   { m_bStartTask = sot;      } // This set of descriptors starts a task, defaults to true
      void               EndTask  (btBool eot)                   { m_bEndTask   = eot;      } // This set of descriptors ends a task, defaults to true
      void               SetTranID(const TransactionID &rTranID) { m_TranID     = rTranID;  }
      void               SetAFUDev(IAFUDev * const pIAFUDev)     { m_pIAFUDev   = pIAFUDev; }
      void               Clear();             // Clear the Transaction for reuse

      // Used by AIA to marshal the payload to the driver
      btVirtAddr      GetPayloadPtr(); // Return address of the rendered payload
      btWSSize        GetSize();       // Return the size of the rendered payload
      TransactionID * GetTranIDPtr()   { return NULL;               }
      uid_msgIDs_e    GetCommand()     { return reqid_UID_SendAFU;  }
      btID            GetPIPVer()      { return AAL_AHMPIP_IID_1_0; } // Return the PIP version, burned in
      btID            GetAPIVer()      { return AAL_AHMAPI_IID_1_0; } // Return the API version, burned in
      btBool          IsOK()     const { return m_bIsOK;            }
      std::string     GetError() const { return m_ErrorString;      } // Called if IsOK() fails
      void            Render();

   private:
      struct big
      {
         struct aalui_AFUmessage       afuMsg;
         struct submit_descriptors_req desc;
      };
#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4251)
#endif // _MSC_VER
      BufDescVec_t  m_viBufDesc;         // Ordered list of descriptors of input buffers that will be rendered
      BufDescVec_t  m_voBufDesc;         // Ordered list of descriptors of output buffers that will be rendered
#ifdef _MSC_VER
# pragma warning(pop)
#endif // _MSC_VER
      btVirtAddr    m_payload;           // Internal variable pointing to rendered payload, not really chars
      IAFUDev      *m_pIAFUDev;          // This transaction is bound to this device
      btWSSize      m_size;              // Internal variable containing length of rendered payload
      btBool        m_bRendered;         // True if the payload has been rendered (by call to GetPayloadPtr or GetSize)
      btBool        m_bIsOK;             // If the object is okay. Any faults turn this off.
      btBool        m_bStartTask;        // True if this set of descriptors starts a task
      btBool        m_bEndTask;          // True if this set of descriptors ends a task
      TransactionID m_TranID;            // TranID for this Task

      // Internal worker routines
      void               ClearRender();
      btUnsigned16bitInt ComputeMask(size_t index, size_t numRecs);
#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4251)
#endif // _MSC_VER
      std::string        m_ErrorString;
#ifdef _MSC_VER
# pragma warning(pop)
#endif // _MSC_VER

      // copy constructor and assignment operator.  nobody should be calling these
      SubmitTask_Serial_AFUTransaction(const SubmitTask_Serial_AFUTransaction & );
      SubmitTask_Serial_AFUTransaction & operator = (const SubmitTask_Serial_AFUTransaction & );

   }; // end of class SubmitTask_Serial_AFUTransaction

   //=============================================================================
   // Name:          SubmitTask_VMaster_AFUTransaction
   // Description:   Submit a Virtual Master Mode task
   // Comment:       Until a multi-description version is created, one must first
   //                   call SubmitTask_VMaster_AFUTransaction before
   //                   calling this, passing the same TransactionID to both.
   //=============================================================================

   class FAPTRANS1_API SubmitTask_VMaster_AFUTransaction : public IAFUTransaction
   {
   public:
      SubmitTask_VMaster_AFUTransaction(
            btVirtAddr             inAddress,  // User mode virtual pointer to input SLAVE MODE buffer
            btWSSize               inLength,   // Length of the input buffer
            btVirtAddr             wsAddress,  // User mode virtual pointer to VMM Workspace
            IAFUDev               *pIAFUDev,   // This transaction is bound to this device
            const TransactionID   &rTranID,    // Original TransactionID from Application
            btBool                 fResponse,  // Request a response when this descriptor completes, separate from Task Done
            btObjectType           Context     // Context to be returned with response
            );

      virtual ~SubmitTask_VMaster_AFUTransaction();

      // Used by AIA to marshal the payload to the driver
      btVirtAddr      GetPayloadPtr();     // Return address of the rendered payload
      btWSSize        GetSize();           // Return the size of the rendered payload
      TransactionID * GetTranIDPtr();
      uid_msgIDs_e    GetCommand();
      btID            GetPIPVer();         // Return the PIP version, burned in
      btID            GetAPIVer();         // Return the API version, burned in
      btBool          IsOK();
      std::string     GetError();          // Called if IsOK() fails

   private:
      struct big
      {
         struct aalui_AFUmessage       afuMsg;
         struct submit_descriptors_req desc;
      };
#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4251)
#endif // _MSC_VER
      std::string m_ErrorString;
#ifdef _MSC_VER
# pragma warning(pop)
#endif // _MSC_VER
      btBool      m_bIsOK;
      btVirtAddr  m_payload;           // Internal variable pointing to rendered payload, not really chars
      btWSSize    m_size;              // Internal variable containing length of rendered payload

      // copy constructor and assignment operator.  nobody should be calling these
      SubmitTask_VMaster_AFUTransaction(const SubmitTask_VMaster_AFUTransaction & );
      SubmitTask_VMaster_AFUTransaction & operator = (const SubmitTask_VMaster_AFUTransaction & );

   }; // end of class SubmitTask_VMaster_AFUTransaction


   //=============================================================================
   // Name:          SubmitTask_PMaster_AFUTransaction
   // Description:   Encapsulates the sending of a single buffer descriptors using
   //                   the old syntax to an FSB/QPI PIP in any mode, using 1.0
   //                   interface and semantics
   // Comments:   1) Task Mode (Virtual/Physical Master, Slave), and WorkSpaceID
   //                are obtained from the Address by looking it up in the Work-
   //                SpaceMapper embedded in the AFUDev.
   //             2) The construction can fail, e.g. if the passed in Address is
   //                not valid, or the Address+Length does not describe a buffer
   //                that is completely within a WorkSpace. So, it is important
   //                after creating this AFUTransaction to check IsOK before using
   //                i.
   //             3) If IsOK() fails, an appropriate error message will be
   //                available from GetError()
   //             4) rTranID must be the same for every descriptor submitted for
   //                this hardware task, from Start (SOT) to finish (EOT). This
   //                TransactionID is controlled by and is for the use of the
   //                Application, and its identity identifies all of the
   //                descriptors that make up a single task.
   //=============================================================================

   class FAPTRANS1_API SubmitTask_PMaster_AFUTransaction : public IAFUTransaction
   {
   public:
      // Used by the user to set up the descriptor list
      SubmitTask_PMaster_AFUTransaction( 
            btVirtAddr           inAddress,  // User mode virtual pointer to input SLAVE MODE buffer
            btWSSize             inLength,   // Length of the input buffer
            IAFUDev             *pIAFUDev,   // This transaction is bound to this device
            const TransactionID &rTranID,    // Original TransactionID from Application
            btBool               fResponse,  // Request a response when this descriptor completes, separate from Task Done
            btObjectType         Context     // Context to be returned with response
      );

      virtual ~SubmitTask_PMaster_AFUTransaction();

      // Used by AIA to marshal the payload to the driver
      btVirtAddr      GetPayloadPtr();     // Return address of the rendered payload
      btWSSize        GetSize();           // Return the size of the rendered payload
      TransactionID * GetTranIDPtr();
      uid_msgIDs_e    GetCommand();
      btID            GetPIPVer();         // Return the PIP version, burned in
      btID            GetAPIVer();         // Return the API version, burned in
      btBool          IsOK();
      std::string     GetError();          // Called if IsOK() fails

   private:
      struct big
      {
         struct aalui_AFUmessage       afuMsg;
         struct submit_descriptors_req desc;
      };
#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4251)
#endif // _MSC_VER
      std::string m_ErrorString;
#ifdef _MSC_VER
# pragma warning(pop)
#endif // _MSC_VER
      btBool      m_bIsOK;
      btVirtAddr  m_payload;  // Internal variable pointing to rendered payload, not really chars
      btWSSize    m_size;     // Internal variable containing length of rendered payload

   private:
      void ConstructWorker(   
         btVirtAddr           inAddress,  // User mode virtual pointer to input SLAVE MODE buffer
         btWSSize             inLength,   // Length of the input buffer
         IAFUDev             *pIAFUDev,   // This transaction is bound to this device
         const TransactionID &rTranID,    // Original TransactionID from Application
         btBool               fResponse,  // Request a response when this descriptor completes, separate from Task Done
         btObjectType         Context     // Context to be returned with response
      );

      // copy constructor and assignment operator.  nobody should be calling these
      SubmitTask_PMaster_AFUTransaction(const SubmitTask_PMaster_AFUTransaction & );
      SubmitTask_PMaster_AFUTransaction & operator = (const SubmitTask_PMaster_AFUTransaction & );

   }; // end of class SubmitTask_PMaster_AFUTransaction


   ///////////////////////////////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////////////////////////////
   ////////////////////                                     //////////////////////
   /////////////////            W O R K S P A C E              ///////////////////
   /////////////////                                           ///////////////////
   /////////////////      A F U   T R A N S A C T I O N S      ///////////////////
   ////////////////////                                     //////////////////////
   ///////////////////////////////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////////////////////////////


   //=============================================================================
   // Name:          WkSp_Allocate_Mapping_EventHandlerObject
   // Description:   Encapsulates the WkSp_Single_Allocate_AFUTransaction callback
   //                   function that must mmap the wsid to get the pointer
   //=============================================================================

   class FAPTRANS1_API WkSp_Allocate_Mapping_EventHandlerObject
   {
   public:
      WkSp_Allocate_Mapping_EventHandlerObject(TTASK_MODE task_mode, btWSSize len) :
         m_task_mode(task_mode),
         m_len(len)
      {}
      virtual ~WkSp_Allocate_Mapping_EventHandlerObject();

      static void WkSp_Allocate_Mapping_EventHandler(IEvent const &theEvent);
      virtual btEventHandler Get_Allocate_Mapping_EventHandler();

   private:
      WkSp_Allocate_Mapping_EventHandlerObject();  // private default ctor
      TTASK_MODE m_task_mode;   // Task type that will use the buffer, passed
                                //    into the creating AFUTransaction
      btWSSize   m_len;         // Requested length passed into the allocator
                                //    Could be 0, in which case allocation is of 1

   }; // end of class WkSp_Allocate_Mapping_EventHandlerObject

   //=============================================================================
   // Name:          WkSp_Single_Allocate_AFUTransaction
   // Description:   Encapsulates the request for a single workspace
   // Constructor:   Parameter is the length of the buffer requested, in bytes
   //=============================================================================

   class FAPTRANS1_API WkSp_Single_Allocate_AFUTransaction : public IAFUTransaction
   {
   public:
      WkSp_Single_Allocate_AFUTransaction(btWSSize   len,
                                          TTASK_MODE TaskMode);

      btVirtAddr      GetPayloadPtr();
      btWSSize        GetSize();
      TransactionID * GetTranIDPtr();
      uid_msgIDs_e    GetCommand();
      btID            GetPIPVer();         // Return the PIP version, burned in
      btID            GetAPIVer();         // Return the API version, burned in
      btBool          IsOK();
      std::string     GetError();          // Called if IsOK() fails

   protected:
      struct big
      {
         struct aalui_AFUmessage afuMsg;
         struct ahm_req          ahmreq;
      };
      struct big                                m_big;
      btBool                                    m_bIsOK;
      TransactionID                             m_tidEmbedded;
      WkSp_Allocate_Mapping_EventHandlerObject *m_pWkspMapObj; // cleaned up in the handler
#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4251)
#endif // _MSC_VER
      std::string                               m_ErrorString;
#ifdef _MSC_VER
# pragma warning(pop)
#endif // _MSC_VER

   protected:
      void ConstructWorker(btWSSize   len,
                           TTASK_MODE TaskMode);

   }; // end of class WkSp_Single_Allocate_AFUTransaction

   //=============================================================================
   // Name:          WkSp_Free_Mapping_EventHandlerObject
   // Description:   Encapsulates the WkSp_Single_Allocate_AFUTransaction callback
   //                   function that must mmap the wsid to get the pointer
   //=============================================================================

   class FAPTRANS1_API WkSp_Free_Mapping_EventHandlerObject
   {
   public:
      WkSp_Free_Mapping_EventHandlerObject(const WorkSpaceMapper::WkSp &rParms);
      virtual ~WkSp_Free_Mapping_EventHandlerObject();

      static void WkSp_Free_Mapping_EventHandler(IEvent const &theEvent);
      virtual btEventHandler Get_Free_Mapping_EventHandler();

   private:
      WkSp_Free_Mapping_EventHandlerObject();
      struct aalui_WSMParms m_WSMParms;
   }; // class WkSp_Free_Mapping_EventHandlerObject

   //=============================================================================
   // Name:          WkSp_Single_Free_AFUTransaction
   // Description:   Encapsulates the request to free a single workspace
   // Parameters:    address, length, wsid obtained from allocate
   //=============================================================================

   class FAPTRANS1_API WkSp_Single_Free_AFUTransaction : public IAFUTransaction
   {
   public:
      WkSp_Single_Free_AFUTransaction(IAFUDev   *pIAFUDev,
                                      btVirtAddr ptr);

      btVirtAddr      GetPayloadPtr();
      btWSSize        GetSize();
      TransactionID * GetTranIDPtr();
      uid_msgIDs_e    GetCommand();
      btID            GetPIPVer();         // Return the PIP version, burned in
      btID            GetAPIVer();         // Return the API version, burned in
      btBool          IsOK();
      std::string     GetError();          // Called if IsOK() fails

   protected:
      struct big
      {
         struct aalui_AFUmessage afuMsg;
         struct ahm_req          ahmreq;
      };
      struct big                            m_big;
      btBool                                m_bIsOK;
      TransactionID                         m_tidEmbedded;
      WkSp_Free_Mapping_EventHandlerObject *m_pWkspMapObj; // cleaned up in the handler
#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4251)
#endif // _MSC_VER
      std::string                           m_ErrorString;
#ifdef _MSC_VER
# pragma warning(pop)
#endif // _MSC_VER

   }; // class WkSp_Single_Free_AFUTransaction


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

   class FAPTRANS1_API Activate_AFUTransaction : public IAFUTransaction
   {
   public:
      Activate_AFUTransaction(btUnsigned64bitInt ActivateMask);

      btVirtAddr      GetPayloadPtr();
      btWSSize        GetSize();
      TransactionID * GetTranIDPtr();
      uid_msgIDs_e    GetCommand();
      btID            GetPIPVer();         // Return the PIP version, burned in
      btID            GetAPIVer();         // Return the API version, burned in
      btBool          IsOK();
      std::string     GetError();          // Called if IsOK() fails

   protected:
      struct big
      {
         struct aalui_AFUmessage afumsg;
         struct mafu_request     mafureq;
      };
      struct big     m_msg;
      btBool         m_bIsOK;
      TransactionID  m_tidEmbedded;
#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4251)
#endif // _MSC_VER
      std::string    m_ErrorString;
#ifdef _MSC_VER
# pragma warning(pop)
#endif // _MSC_VER

   }; // class Activate_AFUTransaction

   //=============================================================================
   // Name:          DeActivate_AFUTransaction
   // Description:   Encapsulates the request to DeActivate sub-devices on a device
   // Parameters:    Bitmap describing sub-devices to DeActivate:
   //                   2^n==1 means to DeActivate that sub-device
   //=============================================================================

   class FAPTRANS1_API DeActivate_AFUTransaction : public IAFUTransaction
   {
   public:
      DeActivate_AFUTransaction(btUnsigned64bitInt DeActivateMask);

      btVirtAddr      GetPayloadPtr();
      btWSSize        GetSize();
      TransactionID * GetTranIDPtr();
      uid_msgIDs_e    GetCommand();
      btID            GetPIPVer();         // Return the PIP version, burned in
      btID            GetAPIVer();         // Return the API version, burned in
      btBool          IsOK();
      std::string     GetError();          // Called if IsOK() fails

   protected:
      struct big
      {
         struct aalui_AFUmessage afumsg;
         struct mafu_request     mafureq;
      };
      struct big    m_msg;
      btBool        m_bIsOK;
      TransactionID m_tidEmbedded;
#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4251)
#endif // _MSC_VER
      std::string   m_ErrorString;
#ifdef _MSC_VER
# pragma warning(pop)
#endif // _MSC_VER

   }; // class DeActivate_AFUTransaction

   //=============================================================================
   // Name:          Initialize_AFUTransaction
   // Description:   Encapsulates the request to Initialize sub-devices on a device
   // Parameters:    Bitmap describing sub-devices to Initialize:
   //                   2^n==1 means to Initialize that sub-device
   //                An opaque payload to be passed to the Management AFU during
   //                   initialization of another AFU, characterized by user
   //                   virtual pointer and length. NULL & 0 are okay values.
   //=============================================================================

   class FAPTRANS1_API Initialize_AFUTransaction : public IAFUTransaction
   {
   public:
      Initialize_AFUTransaction(btUnsigned64bitInt InitializeMask,
                                btVirtAddr         Payload,
                                btWSSize           Size);

      btVirtAddr      GetPayloadPtr();
      btWSSize        GetSize();
      TransactionID * GetTranIDPtr();
      uid_msgIDs_e    GetCommand();
      btID            GetPIPVer();         // Return the PIP version, burned in
      btID            GetAPIVer();         // Return the API version, burned in
      btBool          IsOK();
      std::string     GetError();          // Called if IsOK() fails

   protected:
      struct big
      {
         struct aalui_AFUmessage afumsg;
         struct mafu_request     mafureq;
      };
      struct big    m_msg;
      btBool        m_bIsOK;
      TransactionID m_tidEmbedded;
#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4251)
#endif // _MSC_VER
      std::string   m_ErrorString;
#ifdef _MSC_VER
# pragma warning(pop)
#endif // _MSC_VER
   }; // class Initialize_AFUTransaction

   //=============================================================================
   // Name:          Free_AFUTransaction
   // Description:   Encapsulates the request to Free sub-devices on a device
   // Parameters:    Bitmap describing sub-devices to Free:
   //                   2^n==1 means to Free that sub-device
   //=============================================================================

   class FAPTRANS1_API Free_AFUTransaction : public IAFUTransaction
   {
   public:
      Free_AFUTransaction(btUnsigned64bitInt FreeMask);

      btVirtAddr      GetPayloadPtr();
      btWSSize        GetSize();
      TransactionID * GetTranIDPtr();
      uid_msgIDs_e    GetCommand();
      btID            GetPIPVer();         // Return the PIP version, burned in
      btID            GetAPIVer();         // Return the API version, burned in
      btBool          IsOK();
      std::string     GetError();          // Called if IsOK() fails

   protected:
      struct big
      {
         struct aalui_AFUmessage afumsg;
         struct mafu_request     mafureq;
      };
      struct big    m_msg;
      btBool        m_bIsOK;
      TransactionID m_tidEmbedded;
#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4251)
#endif // _MSC_VER
      std::string   m_ErrorString;
#ifdef _MSC_VER
# pragma warning(pop)
#endif // _MSC_VER
   }; // class Free_AFUTransaction


   ///////////////////////////////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////////////////////////////
   ////////////////////                                     //////////////////////
   /////////////////                   C S R                   ///////////////////
   /////////////////                                           ///////////////////
   /////////////////      P I P   T R A N S A C T I O N S      ///////////////////
   ////////////////////                                     //////////////////////
   ///////////////////////////////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////////////////////////////

   //=============================================================================
   // Name:          CSR_vec_t
   // Description:   Vector of csr_offset_value structures, defined in fappip.h
   //   uint64_t csr_offset;       // CSR index: 0, 1, 2, ...
   //   uint64_t csr_value;        // Set by client on SET CSR, Set by Driver on GET CSR
   //=============================================================================

   typedef std::vector<csr_offset_value> CSR_vec_t;

   //=============================================================================
   // Name:          CSR_AFUTransaction
   // Description:   Encapsulates the sending of a list of CSR Set/Gets
   // NOTE: NOT FINISHED YET
   //=============================================================================

   class FAPTRANS1_API CSR_AFUTransaction : public IAFUTransaction
   {
   public:
      // Used by the user to set up the descriptor list
                        CSR_AFUTransaction();   // Default constructor
      virtual          ~CSR_AFUTransaction();

      // Public interfaces
                                                // Define a CSR to Get
      void              AddGetCSR(btCSROffset CSR);
                                                // Define a CSR to Set, and its value
      void              AddSetCSR(btCSROffset CSR, btCSRValue Value);
      void              Clear();                // Clear the Transaction for reuse

      // Used by AIA to marshal the payload to the driver
      btVirtAddr      GetPayloadPtr();        // Return address of the rendered payload
      btWSSize        GetSize();              // Return the size of the rendered payload
      TransactionID * GetTranIDPtr();         // Return embedded TranID, or NULL if none
      uid_msgIDs_e    GetCommand();           // Kind of ioctlreq message
      btID            GetPIPVer();            // Return the PIP version, burned in
      btID            GetAPIVer();            // Return the API version, burned in
      btBool          IsOK();
      std::string     GetError();             // Called if IsOK() fails

   private:
#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4251)
#endif // _MSC_VER
      CSR_vec_t               m_vGetCSR;      // List of CSRs that will be gotten
      CSR_vec_t               m_vSetCSR;      // List of CSRs that will be set
#ifdef _MSC_VER
# pragma warning(pop)
#endif // _MSC_VER
      struct aalui_AFUmessage pipMsg;
      btVirtAddr              m_payload;      // Returned data structure
      btWSSize                m_size;         // Internal variable containing length of rendered payload
      btBool                  m_bRendered;    // True if the payload has been rendered (by call to GetPayloadPtr or GetSize)
      btBool                  m_bIsOK;        // If the object is okay. Any faults turn this off.
#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4251)
#endif // _MSC_VER
      std::string             m_ErrorString;
#ifdef _MSC_VER
# pragma warning(pop)
#endif // _MSC_VER

      void Render();
      void ClearRender();

      // copy constructor and assignment operator.  nobody should be calling these
      CSR_AFUTransaction(const CSR_AFUTransaction & );
      CSR_AFUTransaction & operator = (const CSR_AFUTransaction & );

   }; // end of class CSR_AFUTransaction





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
   // Name:          Sig_MapCSRSpace_EventHandlerObject
   // Description:   Encapsulates the Sig_MapCSRSpace_AFUTransaction callback
   //=============================================================================
   class FAPTRANS1_API Sig_MapCSRSpace_EventHandlerObject
   {
   public:
      Sig_MapCSRSpace_EventHandlerObject();
      virtual ~Sig_MapCSRSpace_EventHandlerObject();

      static void MapCSRSpace_EventHandler(IEvent const &theEvent);
      virtual btEventHandler GetEventHandler();
   }; // end of class Sig_MapCSRSpace_EventHandlerObject

   //=============================================================================
   // Name:          Sig_MapMMIO_Space_AFUTransaction
   // Description:   Encapsulates the request to Map device MMIO-r or UMSG space
   //=============================================================================
   class FAPTRANS1_API Sig_MapMMIO_Space_AFUTransaction : public IAFUTransaction
   {
   public:
      Sig_MapMMIO_Space_AFUTransaction();

      btVirtAddr      GetPayloadPtr();
      btWSSize        GetSize();
      TransactionID * GetTranIDPtr();
      uid_msgIDs_e    GetCommand();
      btID            GetPIPVer();         // Return the PIP version, burned in
      btID            GetAPIVer();         // Return the API version, burned in
      btBool          IsOK();
      std::string     GetError();          // Called if IsOK() fails

   protected:
      struct big
      {
         struct aalui_AFUmessage afuMsg;
         struct ahm_req          ahmreq;
      };
      struct big                          m_big;
      btBool                              m_bIsOK;
      TransactionID                       m_tidEmbedded;
      Sig_MapCSRSpace_EventHandlerObject *m_pEventHandler;
#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4251)
#endif // _MSC_VER
      std::string                         m_ErrorString;
#ifdef _MSC_VER
# pragma warning(pop)
#endif // _MSC_VER

   }; // end of class Sig_MapMMIO_Space_AFUTransaction

   //=============================================================================
   // Name:          Sig_MapUMSGpace_AFUTransaction
   // Description:   Encapsulates the request to Map device MMIO-r or UMSG space
   //=============================================================================
   class FAPTRANS1_API Sig_MapUMSGpace_AFUTransaction : public IAFUTransaction
   {
   public:
      Sig_MapUMSGpace_AFUTransaction();

      btVirtAddr      GetPayloadPtr();
      btWSSize        GetSize();
      TransactionID * GetTranIDPtr();
      uid_msgIDs_e    GetCommand();
      btID            GetPIPVer();         // Return the PIP version, burned in
      btID            GetAPIVer();         // Return the API version, burned in
      btBool          IsOK();
      std::string     GetError();          // Called if IsOK() fails

   protected:
      struct big
      {
         struct aalui_AFUmessage afuMsg;
         struct ahm_req          ahmreq;
      };
      struct big                          m_big;
      btBool                              m_bIsOK;
      TransactionID                       m_tidEmbedded;
      Sig_MapCSRSpace_EventHandlerObject *m_pEventHandler;
#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4251)
#endif // _MSC_VER
      std::string                         m_ErrorString;
#ifdef _MSC_VER
# pragma warning(pop)
#endif // _MSC_VER

   }; // end of class Sig_MapUMSGpace_AFUTransaction


   //=============================================================================
   // Name:          Sig_MapCSRSpace_AFUTransaction
   // Description:   Encapsulates the request to Map device CSR signaling space
   // Constructor:   Parameter is the length of the buffer requested, in bytes
   //=============================================================================
   class FAPTRANS1_API Sig_MapCSRSpace_AFUTransaction : public IAFUTransaction
   {
   public:
      Sig_MapCSRSpace_AFUTransaction(btWSID id);

      btVirtAddr      GetPayloadPtr();
      btWSSize        GetSize();
      TransactionID * GetTranIDPtr();
      uid_msgIDs_e    GetCommand();
      btID            GetPIPVer();         // Return the PIP version, burned in
      btID            GetAPIVer();         // Return the API version, burned in
      btBool          IsOK();
      std::string     GetError();          // Called if IsOK() fails

   protected:
      struct big
      {
         struct aalui_AFUmessage afuMsg;
         struct ahm_req          ahmreq;
      };
      struct big                          m_big;
      btBool                              m_bIsOK;
      TransactionID                       m_tidEmbedded;
      Sig_MapCSRSpace_EventHandlerObject *m_pEventHandler;
#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4251)
#endif // _MSC_VER
      std::string                         m_ErrorString;
#ifdef _MSC_VER
# pragma warning(pop)
#endif // _MSC_VER
   private:
      Sig_MapCSRSpace_AFUTransaction();

   }; // end of class WkSp_Single_Allocate_AFUTransaction

END_NAMESPACE(AAL)

#endif // __AALSDK_FAPTRANS_FAP10_H__

