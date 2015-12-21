// Copyright (c) 2015, Intel Corporation
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
// @file diag-common.h
// @brief Functionality common to all NLB utils.
// @ingroup
// @verbatim
// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
//
// AUTHORS: Tim Whisonant, Intel Corporation
// 			Sadruta Chandrashekar, Intel Corporation
//
// HISTORY:
// WHEN:          WHO:     WHAT:
// 06/09/2013     TSW      Initial version.
// 01/07/2015	  SC	   fpgadiag version.@endverbatim
//****************************************************************************
#ifndef __DIAG_COMMON_H__
#define __DIAG_COMMON_H__

#include <aalsdk/AALLoggerExtern.h>
#include <aalsdk/aalclp/aalclp.h>

#include <aalsdk/service/IALIAFU.h>
#include <aalsdk/service/ALIAFUService.h>
//#include <aalsdk/service/IALIClient.h>

//#include <aalsdk/utils/SingleAFUApp.h>
//#include <aalsdk/utils/Utilities.h>
#include <aalsdk/AAL.h>
#include <aalsdk/Runtime.h>

#include <aalsdk/utils/NLBVAFU.h>

#include <string>
#include "diag-nlb-common.h"

using namespace AAL;

#ifdef INFO
# undef INFO
#endif // INFO
#if 1
# define INFO(x) AAL_INFO(LM_Any, __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() : " << x << std::endl)
#else
# define INFO(x)
#endif

#ifdef ERR
# undef ERR
#endif // ERR
#if 1
# define ERR(x) AAL_ERR(LM_Any, __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() : " << x << std::endl)
#else
# define ERR(x)
#endif


/// @brief cciapp-specific instantiation of ICCIClient that receives the event notifications
///        sent by the ICCIAFU.
////////////////////////////////////////////////////////////////////////////////
// CMyCCIClient

// Ugly hack so that Doxygen produces the correct class diagrams.
/*#define CMyALIClient CMyfpgasaneALIClient

class CMyALIClient : public IALIClient,
                     public CAASBase
{
public:
   /// Native Loopback (NLB) requires three workspaces:
   /// <ul>
   ///   <li>a Device Status Memory workspace.</li>
   ///   <li>an Input or source data buffer for the memory copy.</li>
   ///   <li>an Output or destination data buffer for the memory copy.</li>
   /// </ul>
   enum WorkspaceType
   {
      WKSPC_DSM, ///< Device Status Memory
      WKSPC_IN,  ///< Input workspace
      WKSPC_OUT,  ///< Output workspace
      WKSPC_UMSG  ///< UMsg workspace
   };

   CMyALIClient();

   // <IALIClient>
   virtual void      OnWorkspaceAllocated(TransactionID const &TranID,
                                          btVirtAddr           WkspcVirt,
                                          btPhysAddr           WkspcPhys,
                                          btWSSize             WkspcSize);

   virtual void OnWorkspaceAllocateFailed(const IEvent &Event);

   virtual void          OnWorkspaceFreed(TransactionID const &TranID);

   virtual void     OnWorkspaceFreeFailed(const IEvent &Event);
   // </IALIClient>

   btVirtAddr DSMVirt()    const { return m_DSMVirt;    } ///< Accessor for the DSM workspace.
   btVirtAddr InputVirt()  const { return m_InputVirt;  } ///< Accessor for the Input workspace.
   btVirtAddr OutputVirt() const { return m_OutputVirt; } ///< Accessor for the Output workspace.
   btVirtAddr UMsgVirt()   const { return m_UMsgVirt;   } ///< Accessor for the UMsg workspace.

   btPhysAddr DSMPhys()    const { return m_DSMPhys;    } ///< Accessor for the DSM workspace.
   btPhysAddr InputPhys()  const { return m_InputPhys;  } ///< Accessor for the Input workspace.
   btPhysAddr OutputPhys() const { return m_OutputPhys; } ///< Accessor for the Output workspace.
   btPhysAddr UMsgPhys()   const { return m_UMsgPhys;   } ///< Accessor for the UMsg workspace.

   btWSSize   DSMSize()    const { return m_DSMSize;    } ///< Accessor for the DSM workspace.
   btWSSize   InputSize()  const { return m_InputSize;  } ///< Accessor for the Input workspace.
   btWSSize   OutputSize() const { return m_OutputSize; } ///< Accessor for the Output workspace.
   btWSSize   UMsgSize()   const { return m_UMsgSize;   } ///< Accessor for the UMsg workspace.


   /// @brief Wait on the client's internal semaphore.
   void ClientWait();

protected:
   /// @brief Signal (one count) to the client's internal semaphore.
   void ClientPost();

   /// @brief Mutator for the DSM workspace.
   void DSM(btVirtAddr v, btPhysAddr p, btWSSize s)
   {
      m_DSMVirt = v;
      m_DSMPhys = p;
      m_DSMSize = s;
   }

   /// @brief Mutator for the Input workspace.
   void Input(btVirtAddr v, btPhysAddr p, btWSSize s)
   {
      m_InputVirt = v;
      m_InputPhys = p;
      m_InputSize = s;
   }

   /// @brief Mutator for the Output workspace.
   void Output(btVirtAddr v, btPhysAddr p, btWSSize s)
   {
      m_OutputVirt = v;
      m_OutputPhys = p;
      m_OutputSize = s;
   }

   /// @brief Mutator for the Output workspace.
   void UMsg(btVirtAddr v, btPhysAddr p, btWSSize s)
   {
	  m_UMsgVirt = v;
	  m_UMsgPhys = p;
	  m_UMsgSize = s;
   }

   btVirtAddr m_DSMVirt;    ///< DSM workspace virtual address.
   btPhysAddr m_DSMPhys;    ///< DSM workspace physical address.
   btWSSize   m_DSMSize;    ///< DSM workspace size in bytes.
   btVirtAddr m_InputVirt;  ///< Input workspace virtual address.
   btPhysAddr m_InputPhys;  ///< Input workspace physical address.
   btWSSize   m_InputSize;  ///< Input workspace size in bytes.
   btVirtAddr m_OutputVirt; ///< Output workspace virtual address.
   btPhysAddr m_OutputPhys; ///< Output workspace physical address.
   btWSSize   m_OutputSize; ///< Output workspace size in bytes.
   btVirtAddr m_UMsgVirt;   ///< UMsg workspace virtual address.
   btPhysAddr m_UMsgPhys;   ///< UMsg workspace physical address.
   btWSSize   m_UMsgSize;   ///< UMsg workspace size in bytes.
   btInt      m_Wkspcs;     ///< current number of workspaces allocated.
   CSemaphore m_Sem;        ///< client's internal semaphore.
};
*/
/// The default ALIAFU Delegate.
#define DEFAULT_TARGET_AFU ALIAFU_NVS_VAL_TARGET_FPGA


/// The default Test Mode.
#define DEFAULT_TEST_MODE NLB_TESTMODE_LPBK1


/// The default Sub-Device ID.
#define DEFAULT_TARGET_DEV -1

////////////////////////////////////////////////////////////////////////////////
// CMyApp
// Ugly hack so that Doxygen produces the correct class diagrams.
#define CMyApp CMyfpgasaneApp


/// @brief cciapp-specific instantiation of ISingleAFUApp that provides access to an ICCIAFU.
class CMyApp : public IRuntimeClient,
			   public IServiceClient,
			   public CAASBase
{
public:

   enum WorkspaceType
   {
	  WKSPC_DSM, ///< Device Status Memory
	  WKSPC_IN,  ///< Input workspace
	  WKSPC_OUT,  ///< Output workspace
	  WKSPC_UMSG  ///< UMsg workspace
   };

   CMyApp();
   virtual ~CMyApp();

   // <IRuntimeClient>
   virtual void     runtimeStarted(IRuntime *,
                                   const NamedValueSet &);
   virtual void     runtimeStopped(IRuntime *);
   virtual void     runtimeStartFailed(const IEvent &);
   virtual void     runtimeAllocateServiceFailed(IEvent const &);
   virtual void     runtimeAllocateServiceSucceeded(IBase *,
                                                    TransactionID const & );
   virtual void     runtimeEvent(const IEvent & );
   virtual void 	runtimeCreateOrGetProxyFailed(IEvent const &rEvent);
   virtual void 	runtimeStopFailed(const IEvent &rEvent);
   // </IRuntimeClient>

   // <IServiceClient>
   virtual void      serviceAllocated(IBase *,
                                      TransactionID const & = TransactionID());
   virtual void serviceAllocateFailed(const IEvent &);
   virtual void          serviceFreed(TransactionID const & = TransactionID());
   virtual void          serviceEvent(const IEvent &);
   virtual void       serviceReleased(TransactionID const &rTranID = TransactionID());
   virtual void  serviceReleaseFailed(const IEvent &rEvent);
   // </IServiceClient>

   btVirtAddr DSMVirt()    const { return m_DSMVirt;    } ///< Accessor for the DSM workspace.
   btVirtAddr InputVirt()  const { return m_InputVirt;  } ///< Accessor for the Input workspace.
   btVirtAddr OutputVirt() const { return m_OutputVirt; } ///< Accessor for the Output workspace.
   btVirtAddr UMsgVirt()   const { return m_UMsgVirt;   } ///< Accessor for the UMsg workspace.

   btPhysAddr DSMPhys()    const { return m_DSMPhys;    } ///< Accessor for the DSM workspace.
   btPhysAddr InputPhys()  const { return m_InputPhys;  } ///< Accessor for the Input workspace.
   btPhysAddr OutputPhys() const { return m_OutputPhys; } ///< Accessor for the Output workspace.
   btPhysAddr UMsgPhys()   const { return m_UMsgPhys;   } ///< Accessor for the UMsg workspace.

   btWSSize   DSMSize()    const { return m_DSMSize;    } ///< Accessor for the DSM workspace.
   btWSSize   InputSize()  const { return m_InputSize;  } ///< Accessor for the Input workspace.
   btWSSize   OutputSize() const { return m_OutputSize; } ///< Accessor for the Output workspace.
   btWSSize   UMsgSize()   const { return m_UMsgSize;   } ///< Accessor for the UMsg workspace.

   btBool isOK()  {return m_isOK;}

   void Wait() { m_Sem.Wait();  }
   void Post() { m_Sem.Post(1); }
   void Stop();

   /// @brief Mutator for setting the NVS value that selects the AFU Delegate.
   void AFUTarget(const std::string &target) { m_AFUTarget = target; }
   /// @brief Accessor for the NVS value that selects the AFU Delegate.
   std::string AFUTarget() const             { return m_AFUTarget;   }

   /// @brief Mutator for setting the NVS value that selects Sub Device.
   void DevTarget(const btInt &target) { m_DevTarget = target; }
   /// @brief Accessor for the NVS value that selects Sub Device.
   btInt DevTarget() const             { return m_DevTarget;   }

   /// @brief Mutator for setting the test mode.
   void TestMode(const std::string &mode) { m_TestMode = mode; }
   /// @brief Accessor for the test mode.
   std::string TestMode() const             { return m_TestMode;   }

   operator IAALService * () { return m_pAALService;  }

   operator IALIMMIO * ()   { return m_pALIMMIOService; }
   operator IALIBuffer * () { return m_pALIBufferService; }
   operator IALIReset * ()  { return m_pALIResetService; }
   operator IALIUMsg * ()   { return m_pALIuMSGService; }

protected:
   std::string  m_AFUTarget; ///< The NVS value used to select the AFU Delegate (FPGA, ASE, or SWSim).
   btInt        m_DevTarget; ///< The NVS value used to select the Sub Device.
   std::string  m_TestMode; ///< The NVS value used to select the Test mode (LPBK1, READ, WRITE, TRPUT, SW or CCIP_LPBK1).
   //CMyALIClient m_ALIClient; ///< The ICCIClient used to communicate with the allocated Service.
   //Runtime      m_Runtime;           ///< AAL Runtime
   IRuntime    *m_pRuntime;
   IAALService *m_pAALService;
   CSemaphore   m_Sem;
   IALIBuffer  *m_pALIBufferService; ///< Pointer to Buffer Service
   IALIMMIO    *m_pALIMMIOService;   ///< Pointer to MMIO Service
   IALIReset   *m_pALIResetService;  ///< Pointer to AFU Reset Service
   IALIUMsg    *m_pALIuMSGService;   ///< Pointer to uMSg Service
   btBool       m_isOK;

   // Workspace info
   btVirtAddr     m_DSMVirt;        ///< DSM workspace virtual address.
   btPhysAddr     m_DSMPhys;        ///< DSM workspace physical address.
   btWSSize       m_DSMSize;        ///< DSM workspace size in bytes.
   btVirtAddr     m_InputVirt;      ///< Input workspace virtual address.
   btPhysAddr     m_InputPhys;      ///< Input workspace physical address.
   btWSSize       m_InputSize;      ///< Input workspace size in bytes.
   btVirtAddr     m_OutputVirt;     ///< Output workspace virtual address.
   btPhysAddr     m_OutputPhys;     ///< Output workspace physical address.
   btWSSize       m_OutputSize;     ///< Output workspace size in bytes.
   btVirtAddr 	  m_UMsgVirt;   	///< UMsg workspace virtual address.
   btPhysAddr 	  m_UMsgPhys;   	///< UMsg workspace physical address.
   btWSSize   	  m_UMsgSize;   	///< UMsg workspace size in bytes.
};



/// @}
////////////////////////////////////////////////////////////////////////////////
class INLB
{
public:
   virtual ~INLB() {}
   virtual btInt RunTest(const NLBCmdLine &cmd) = 0;

   std::string ReadBandwidth()  const { return m_RdBw; }
   std::string WriteBandwidth() const { return m_WrBw; }

protected:
   INLB(CMyApp *pMyApp) :
      m_pMyApp(pMyApp),
      m_pALIMMIOService((IALIMMIO *) *pMyApp), // uses type cast operator from ISingleAFUApp.
      m_pALIBufferService((IALIBuffer *) *pMyApp),
      m_pALIResetService((IALIReset *) *pMyApp),
      m_pALIuMSGService((IALIUMsg *) *pMyApp)
   {
      ASSERT(NULL != m_pMyApp);
      ASSERT(NULL != m_pALIMMIOService);
      ASSERT(NULL != m_pALIBufferService);
      ASSERT(NULL != m_pALIResetService);

      btInt i;
      for ( i = 0 ; i < sizeof(m_QLPCounters) / sizeof(m_QLPCounters[0]) ; ++i ) {
         m_QLPCounters[i] = 0;
         m_SavedQLPCounters[i] = 0;
      }
   }

   btInt ResetHandshake();
   btInt CacheCooldown(btVirtAddr CoolVirt, btPhysAddr CoolPhys, btWSSize CoolSize);

   void       ReadQLPCounters();
   void       SaveQLPCounters();
   bt32bitCSR   GetQLPCounter(btUnsignedInt ) const;

   std::string  CalcReadBandwidth(const NLBCmdLine &cmd);
   std::string CalcWriteBandwidth(const NLBCmdLine &cmd);
   std::string         Normalized(const NLBCmdLine &cmd) const throw();

   void EnableCSRPrint(bool bEnable, bool bReplay=true);

   CMyApp     *m_pMyApp;
   IALIBuffer *m_pALIBufferService; ///< Pointer to Buffer Service
   IALIMMIO   *m_pALIMMIOService;   ///< Pointer to MMIO Service
   IALIReset  *m_pALIResetService;  ///< Pointer to AFU Reset Service
   IALIUMsg   *m_pALIuMSGService;   ///< Pointer to uMSg Service
   bt32bitCSR  m_QLPCounters[QLP_NUM_COUNTERS];
   bt32bitCSR  m_SavedQLPCounters[QLP_NUM_COUNTERS];
   std::string m_RdBw;
   std::string m_WrBw;
};

class CNLBLpbk1 : public INLB
{
public:
   CNLBLpbk1(CMyApp *pMyApp) :
      INLB(pMyApp)
   {}
   virtual btInt RunTest(const NLBCmdLine &cmd);
   virtual void  PrintOutput(const NLBCmdLine &cmd, wkspc_size_type cls);
};

class CNLBRead : public INLB
{
public:
	CNLBRead(CMyApp *pMyApp) :
      INLB(pMyApp)
   {}
   virtual btInt RunTest(const NLBCmdLine &cmd);
   virtual void  PrintOutput(const NLBCmdLine &cmd, wkspc_size_type cls);
};

class CNLBWrite : public INLB
{
public:
	CNLBWrite(CMyApp *pMyApp) :
      INLB(pMyApp)
   {}
   virtual btInt RunTest(const NLBCmdLine &cmd);
   virtual void  PrintOutput(const NLBCmdLine &cmd, wkspc_size_type cls);
};


class CNLBTrput : public INLB
{
public:
   CNLBTrput(CMyApp *pMyApp) :
      INLB(pMyApp)
   {}
   virtual btInt RunTest(const NLBCmdLine &cmd);
   virtual void  PrintOutput(const NLBCmdLine &cmd, wkspc_size_type cls);
};


class CNLBSW : public INLB
{
public:
	CNLBSW(CMyApp *pMyApp) :
      INLB(pMyApp)
    {}
   virtual btInt RunTest(const NLBCmdLine &cmd);
   virtual void  PrintOutput(const NLBCmdLine &cmd, wkspc_size_type cls);
};
class CNLBCcipLpbk1 : public INLB
{
public:
	CNLBCcipLpbk1(CMyApp *pMyApp) :
      INLB(pMyApp)
    {}
   virtual btInt RunTest(const NLBCmdLine &cmd);
   virtual void  PrintOutput(const NLBCmdLine &cmd, wkspc_size_type cls);
};
class CNLBCcipRead : public INLB
{
public:
	CNLBCcipRead(CMyApp *pMyApp) :
      INLB(pMyApp)
    {}
   virtual btInt RunTest(const NLBCmdLine &cmd);
   virtual void  PrintOutput(const NLBCmdLine &cmd, wkspc_size_type cls);
};

class CNLBCcipWrite : public INLB
{
public:
	CNLBCcipWrite(CMyApp *pMyApp) :
      INLB(pMyApp)
    {}
   virtual btInt RunTest(const NLBCmdLine &cmd);
   virtual void  PrintOutput(const NLBCmdLine &cmd, wkspc_size_type cls);
};

class CNLBCcipTrput : public INLB
{
public:
	CNLBCcipTrput(CMyApp *pMyApp) :
      INLB(pMyApp)
    {}
   virtual btInt RunTest(const NLBCmdLine &cmd);
   virtual void  PrintOutput(const NLBCmdLine &cmd, wkspc_size_type cls);
};

class CNLBCcipSW : public INLB
{
public:
	CNLBCcipSW(CMyApp *pMyApp) :
      INLB(pMyApp)
    {}
   virtual btInt RunTest(const NLBCmdLine &cmd);
   virtual void  PrintOutput(const NLBCmdLine &cmd, wkspc_size_type cls);
};
#endif
