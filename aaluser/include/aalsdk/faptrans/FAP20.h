// Copyright (c) 2012-2014, Intel Corporation
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
/// @file FAP20.h
/// @brief Defines FAP v2.0 Service for the Universal AIA.
/// @ingroup uAIA
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Henry Mitchel, Intel Corporation
///          Joseph Grecco, Intel Corporation
///          Tim Whisonant, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 04/19/2012     TSW      Moved FAP20 classes from FAP10.h@endverbatim
//****************************************************************************
#ifndef __AALSDK_FAPTRANS_FAP20_H__
#define __AALSDK_FAPTRANS_FAP20_H__
#include <aalsdk/kernel/aalids.h>
#include <aalsdk/kernel/fappip.h>

#include <aalsdk/AALTypes.h>
#include <aalsdk/osal/OSServiceModule.h>
#include <aalsdk/uAIALib.h>


AAL_DECLARE_MOD(libFAPTrans2, FAPTRANS2_API)


BEGIN_NAMESPACE(AAL)
   BEGIN_NAMESPACE(AAS)
      BEGIN_NAMESPACE(AIA)
         BEGIN_NAMESPACE(FAP_20)

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////                                     //////////////////////
/////////////////                   SPL2                    ///////////////////
/////////////////                                           ///////////////////
/////////////////      A F U   T R A N S A C T I O N S      ///////////////////
////////////////////                                     //////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//=============================================================================
// Name:          SPL2_Start_AFUTransaction
// Description:   Starts a transaction on an SPL2 AFU
// Input: pIAFUDev - CAFUdev pointer
//        tranID   - Trasnaction ID
//        Address  - (optional) Workspace pointer (user virtual address)
//        pollrate - pollrate (units I forget)
// Comments:      This function enables the application to start interacting
//                  with the SPL2 AFU. Starting a transaction is required to
//                  put the AFU in a state where interactions are permitted.
//                  These include setting CSRs and accessing the Device State
//                  Memory region (DSM).
//                After sending this, the response will be in a
//                   tranevtUIDriverClientEvent.  The event will contain a
//                   pointer to the valid device DSM.
//=============================================================================
#define SPL2_DEFAULT_POLLRATE 10
class FAPTRANS2_API SPL2_Start_AFUTransaction : public IAFUTransaction
{
public:
   // Used by the user to set up the descriptor list
   SPL2_Start_AFUTransaction(IAFUDev             *pIAFUDev,
                             TransactionID const &tranID,
                             btVirtAddr           Address = NULL,
                             btTime               pollrate = SPL2_DEFAULT_POLLRATE);

   btBool         IsOK() const;
   // Called if IsOK() fails
   std::string    GetError() const;
   btID           GetPIPVer();
   btID           GetAPIVer();
   // NULL means no embedded TranID
   btVirtAddr     GetPayloadPtr();
   btWSSize       GetSize();
   TransactionID *GetTranIDPtr();
   uid_msgIDs_e   GetCommand();

private:
   struct big
   {
      struct aalui_AFUmessage afuMsg;
      struct spl2req          req;
   };

   btBool        m_bIsOK;       // If the object is okay. Any faults turn this off.
   struct big    m_big;
#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4251)
#endif // _MSC_VER
   std::string   m_ErrorString;
#ifdef _MSC_VER
# pragma warning(pop)
#endif // _MSC_VER
   TransactionID m_tidEmbedded; // for mapping into user space

   // copy constructor and assignment operator. Nobody should be calling these.
   SPL2_Start_AFUTransaction(const SPL2_Start_AFUTransaction & );
   SPL2_Start_AFUTransaction & operator = (const SPL2_Start_AFUTransaction & );

   static void AFUDSMMappingHandler(AAL::IEvent const &theEvent);
}; // class SPL2_Start_AFUTransaction

//=============================================================================
// Name:          SPL2_SetContextWorkspace_AFUTransaction
// Description:   Sets the workspace for the AFU Context and starts a transaction
//                 on the SPL2 AFU.
// Input: pIAFUDev - CAFUdev pointer
//        tranID   - Trasnaction ID
//        Address  - Workspace pointer (user virtual address)
//        pollrate - pollrate (units I forget)
// Comments:      This function enables the application to start interacting
//                  with the SPL2 AFU, in the case that no Address for the AFU
//                  context was supplied in the SPL2_Start_AFUTransaction.
//                The actions performed should be those that would have resulted
//                  from providing the workspace pointer in the Start call, ie
//                  the transaction will start within the current session.
//=============================================================================
class FAPTRANS2_API SPL2_SetContextWorkspace_AFUTransaction : public IAFUTransaction
{
public:
   // Used by the user to set up the descriptor list
   SPL2_SetContextWorkspace_AFUTransaction(IAFUDev             *pIAFUDev,
                                           TransactionID const &tranID,
                                           btVirtAddr           Address,
                                           btTime               pollrate = SPL2_DEFAULT_POLLRATE);

   btBool         IsOK() const;
   // Called if IsOK() fails
   std::string    GetError() const;
   btID           GetPIPVer();
   btID           GetAPIVer();
   // NULL means no embedded TranID
   btVirtAddr     GetPayloadPtr();
   btWSSize       GetSize();
   TransactionID *GetTranIDPtr();
   uid_msgIDs_e   GetCommand();

private:
   struct big
   {
      struct aalui_AFUmessage afuMsg;
      struct spl2req          req;
   };

   btBool      m_bIsOK;             // If the object is okay. Any faults turn this off.
   struct big  m_big;
#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4251)
#endif // _MSC_VER
   std::string m_ErrorString;
#ifdef _MSC_VER
# pragma warning(pop)
#endif // _MSC_VER

   // copy constructor and assignment operator. Nobody should be calling these.
   SPL2_SetContextWorkspace_AFUTransaction(const SPL2_SetContextWorkspace_AFUTransaction & );
   SPL2_SetContextWorkspace_AFUTransaction & operator = (const SPL2_SetContextWorkspace_AFUTransaction & );

}; // class SPL2_SetContextWorkspace_AFUTransaction

//=============================================================================
// Name:          SPL2_Stop_AFUTransaction
// Description:   Stopped a transaction on an SPL2 AFU
// Input: pIAFUDev - CAFUdev pointer
//        tranID   - Transaction ID
// Comments:      This function causes the AFU transaction state to become
//                stopped. It does not change the owner session or free any
//                resources. I.e., The application still controls the AFU and
//                still has any allocated workspaces.
//=============================================================================
class FAPTRANS2_API SPL2_Stop_AFUTransaction : public IAFUTransaction
{
public:
   // Used by the user to set up the descriptor list
   SPL2_Stop_AFUTransaction(IAFUDev             *pIAFUDev,
                             TransactionID const &tranID);

   btBool         IsOK() const;
   // Called if IsOK() fails
   std::string    GetError() const;
   btID           GetPIPVer();
   btID           GetAPIVer();
   // NULL means no embedded TranID
   btVirtAddr     GetPayloadPtr();
   btWSSize       GetSize();
   TransactionID *GetTranIDPtr();
   uid_msgIDs_e   GetCommand();

private:
   struct big
   {
      struct aalui_AFUmessage afuMsg;
      struct spl2req          req;
   };

   btBool        m_bIsOK;       // If the object is okay. Any faults turn this off.
   struct big    m_big;
#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4251)
#endif // _MSC_VER
   std::string   m_ErrorString;
#ifdef _MSC_VER
# pragma warning(pop)
#endif // _MSC_VER
   TransactionID m_tidEmbedded; // for mapping into user space

   // copy constructor and assignment operator. Nobody should be calling these.
   SPL2_Stop_AFUTransaction(const SPL2_Stop_AFUTransaction & );
   SPL2_Stop_AFUTransaction & operator = (const SPL2_Stop_AFUTransaction & );
}; // class SPL2_Stop_AFUTransaction

         END_NAMESPACE(FAP_20)
      END_NAMESPACE(AIA)
   END_NAMESPACE(AAS)
END_NAMESPACE(AAL)

#endif // __AALSDK_FAPTRANS_FAP20_H__

