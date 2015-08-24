#ifndef __DIAG_COMMON_H__
#define __DIAG_COMMON_H__

#include <aalsdk/AALLoggerExtern.h>
#include <aalsdk/aalclp/aalclp.h>

#include <aalsdk/service/ICCIAFU.h>
#include <aalsdk/service/CCIAFUService.h>
#include <aalsdk/service/ICCIClient.h>

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

// Change DBG_HOOK to 1 if you want an opportunity to attach the debugger.
// After attaching, set gWaitForDebuggerAttach to 0 via the debugger to unblock the app.
#define DBG_HOOK 0
#if DBG_HOOK
btBool gWaitForDebuggerAttach = true;
#endif // DBG_HOOK

/// @brief cciapp-specific instantiation of ICCIClient that receives the event notifications
///        sent by the ICCIAFU.
////////////////////////////////////////////////////////////////////////////////
// CMyCCIClient

// Ugly hack so that Doxygen produces the correct class diagrams.
#define CMyCCIClient CMyfpgasaneCCIClient

class CMyCCIClient : public ICCIClient,
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
      WKSPC_OUT  ///< Output workspace
   };

   CMyCCIClient();

   // <ICCIClient>
   virtual void      OnWorkspaceAllocated(TransactionID const &TranID,
                                          btVirtAddr           WkspcVirt,
                                          btPhysAddr           WkspcPhys,
                                          btWSSize             WkspcSize);

   virtual void OnWorkspaceAllocateFailed(const IEvent &Event);

   virtual void          OnWorkspaceFreed(TransactionID const &TranID);

   virtual void     OnWorkspaceFreeFailed(const IEvent &Event);
   // </ICCIClient>

   btVirtAddr DSMVirt()    const { return m_DSMVirt;    } ///< Accessor for the DSM workspace.
   btVirtAddr InputVirt()  const { return m_InputVirt;  } ///< Accessor for the Input workspace.
   btVirtAddr OutputVirt() const { return m_OutputVirt; } ///< Accessor for the Output workspace.

   btPhysAddr DSMPhys()    const { return m_DSMPhys;    } ///< Accessor for the DSM workspace.
   btPhysAddr InputPhys()  const { return m_InputPhys;  } ///< Accessor for the Input workspace.
   btPhysAddr OutputPhys() const { return m_OutputPhys; } ///< Accessor for the Output workspace.

   btWSSize   DSMSize()    const { return m_DSMSize;    } ///< Accessor for the DSM workspace.
   btWSSize   InputSize()  const { return m_InputSize;  } ///< Accessor for the Input workspace.
   btWSSize   OutputSize() const { return m_OutputSize; } ///< Accessor for the Output workspace.

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

   btVirtAddr m_DSMVirt;    ///< DSM workspace virtual address.
   btPhysAddr m_DSMPhys;    ///< DSM workspace physical address.
   btWSSize   m_DSMSize;    ///< DSM workspace size in bytes.
   btVirtAddr m_InputVirt;  ///< Input workspace virtual address.
   btPhysAddr m_InputPhys;  ///< Input workspace physical address.
   btWSSize   m_InputSize;  ///< Input workspace size in bytes.
   btVirtAddr m_OutputVirt; ///< Output workspace virtual address.
   btPhysAddr m_OutputPhys; ///< Output workspace physical address.
   btWSSize   m_OutputSize; ///< Output workspace size in bytes.
   btInt      m_Wkspcs;     ///< current number of workspaces allocated.
   CSemaphore m_Sem;        ///< client's internal semaphore.
};

/// The default CCIAFU Delegate.
#define DEFAULT_TARGET_AFU CCIAFU_NVS_VAL_TARGET_FPGA


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
   // </IRuntimeClient>

   // <IServiceClient>
   virtual void      serviceAllocated(IBase *,
                                      TransactionID const & = TransactionID());
   virtual void serviceAllocateFailed(const IEvent &);
   virtual void          serviceFreed(TransactionID const & = TransactionID());
   virtual void          serviceEvent(const IEvent &);
   // </IServiceClient>

   void Wait() { m_Sem.Wait();  }
   void Post() { m_Sem.Post(1); }
   void Stop();

   /// @brief Mutator for setting the NVS value that selects the AFU Delegate.
   void AFUTarget(const std::string &target) { m_AFUTarget = target; }
   /// @brief Accessor for the NVS value that selects the AFU Delegate.
   std::string AFUTarget() const             { return m_AFUTarget;   }

   /// @brief Wait on the m_CCIClient's internal semaphore.
   void ClientWait()       { m_CCIClient.ClientWait();  }
   /// @brief Determine m_CCIClient's status.
   btBool ClientOK() const { return m_CCIClient.IsOK(); }

   btVirtAddr DSMVirt()    const { return m_CCIClient.DSMVirt();    } ///< Accessor for the DSM workspace.
   btVirtAddr InputVirt()  const { return m_CCIClient.InputVirt();  } ///< Accessor for the Input workspace.
   btVirtAddr OutputVirt() const { return m_CCIClient.OutputVirt(); } ///< Accessor for the Output workspace.

   btPhysAddr DSMPhys()    const { return m_CCIClient.DSMPhys();    } ///< Accessor for the DSM workspace.
   btPhysAddr InputPhys()  const { return m_CCIClient.InputPhys();  } ///< Accessor for the Input workspace.
   btPhysAddr OutputPhys() const { return m_CCIClient.OutputPhys(); } ///< Accessor for the Output workspace.

   btWSSize   DSMSize()    const { return m_CCIClient.DSMSize();    } ///< Accessor for the DSM workspace.
   btWSSize   InputSize()  const { return m_CCIClient.InputSize();  } ///< Accessor for the Input workspace.
   btWSSize   OutputSize() const { return m_CCIClient.OutputSize(); } ///< Accessor for the Output workspace.

   operator IAALService * () { return m_pAALService;  }

   operator ICCIAFU * () { return m_pProprietary; }

protected:
   std::string  m_AFUTarget; ///< The NVS value used to select the AFU Delegate (FPGA, ASE, or SWSim).
   CMyCCIClient m_CCIClient; ///< The ICCIClient used to communicate with the allocated Service.
   IRuntime    *m_pRuntime;
   IAALService *m_pAALService;
   CSemaphore   m_Sem;
   ICCIAFU     *m_pProprietary;
};



/// @}
////////////////////////////////////////////////////////////////////////////////
class INLB
{
public:
   virtual ~INLB() {}
   virtual btInt RunTest(const NLBCmdLine &cmd, btWSSize wssize) = 0;

   std::string ReadBandwidth()  const { return m_RdBw; }
   std::string WriteBandwidth() const { return m_WrBw; }

protected:
   INLB(CMyApp *pMyApp) :
      m_pMyApp(pMyApp),
      m_pCCIAFU((ICCIAFU *) *pMyApp) // uses type cast operator from ISingleAFUApp.
   {
      ASSERT(NULL != m_pMyApp);
      ASSERT(NULL != m_pCCIAFU);

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

   CMyApp     *m_pMyApp;
   ICCIAFU    *m_pCCIAFU;
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
   virtual btInt RunTest(const NLBCmdLine &cmd, btWSSize wssize);
   virtual void  PrintOutput(const NLBCmdLine &cmd, wkspc_size_type cls);
};

class CNLBRead : public INLB
{
public:
	CNLBRead(CMyApp *pMyApp) :
      INLB(pMyApp)
   {}
   virtual btInt RunTest(const NLBCmdLine &cmd, btWSSize wssize);
   virtual void  PrintOutput(const NLBCmdLine &cmd, wkspc_size_type cls);
};

class CNLBWrite : public INLB
{
public:
	CNLBWrite(CMyApp *pMyApp) :
      INLB(pMyApp)
   {}
   virtual btInt RunTest(const NLBCmdLine &cmd, btWSSize wssize);
   virtual void  PrintOutput(const NLBCmdLine &cmd, wkspc_size_type cls);
};


class CNLBTrput : public INLB
				  // , public PrintFormatter
{
public:
   CNLBTrput(CMyApp *pMyApp) :
      INLB(pMyApp)
   {}
   virtual btInt RunTest(const NLBCmdLine &cmd, btWSSize wssize);
   virtual void  PrintOutput(const NLBCmdLine &cmd, wkspc_size_type cls);
};



/*class PrintFormatter : public INLB
{
public:

	void PrintFormatter::PrintNLBTabular(std::ostream &os, const NLBCmdLine &cmd) const throw()
	{
	   //CNLBWorkload *pNLBWkld = dynamic_cast<CNLBWorkload *>(m_pWkld);
	   //CNLBMetrics   m        = pNLBWkld->GetMetrics();

		nlb_vafu_dsm *pAFUDSM = (nlb_vafu_dsm *)m_pMyApp->DSMVirt();

	   if ( flag_is_set(cmd.cmdflags, NLB_TABULAR_DO_HDR) ) {
	           //0123456789 0123456789 01234567890 012345678901 012345678901 0123456789012 0123456789012 0123456789 0123456789012
	      os << "Cachelines Read_Count Write_Count Cache_Rd_Hit Cache_Wr_Hit Cache_Rd_Miss Cache_Wr_Miss   Eviction 'Ticks(@"
	         << cmd.clkfreq << ")'";

	      if ( flag_is_clr(cmd.cmdflags, NLB_TABULAR_NO_BW) ) {
	              // 01234567890123 01234567890123
	         os << "   Rd_Bandwidth   Wr_Bandwidth";
	      }

	      os << std::endl;
	   }

	   //os << setw(10) << m.Size().CacheLines() << ' '
	   os << setw(10) << pAFUDSM->num_reads    << ' '
	      << setw(11) << pAFUDSM->num_writes   << ' ';
	     /* << setw(12) << m.CacheReadHits()     << ' '
	      << setw(12) << m.CacheWriteHits()    << ' '
	      << setw(13) << m.CacheReadMisses()   << ' '
	      << setw(13) << m.CacheWriteMisses()  << ' '
	      << setw(10) << m.CacheEvictions()    << ' '
	      << setw(16) << m.Ticks();*/

	  /*if ( flag_is_clr(cmd.cmdflags, NLB_TABULAR_NO_BW) ) {
		   std::string RdBw = CalcReadBandwidth(cmd);
	      os << "  "
	         << setw(14) << RdBw << ' '
	         << setw(14) << CalcWriteBandwidth(cmd);
	   }

	   os << std::endl;

	}
};*/
#endif
