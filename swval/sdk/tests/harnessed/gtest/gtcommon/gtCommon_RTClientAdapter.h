#ifndef __RUNTIME_CLIENT_ADAPTER_H__
#define __RUNTIME_CLIENT_ADAPTER_H__

#include "aalsdk/Runtime.h"
#include "gtCommon_RTAdapter.h"

class GTCOMMON_API CRuntimeClientAdapter : public CAASBase, public IRuntimeClient
{

public:
   /// ========================================================================
   /// @brief        Default constructor, taking a shared lock.
   ///
   /// @param        pLock    The lock
   ///
   CRuntimeClientAdapter(CListenerLock* pLock);

   // virtual destructor, allowing deletion from a base pointer.
   virtual ~CRuntimeClientAdapter();

protected:
   CRuntimeAdapter*  m_pRuntimeAdapter;
   btBool            m_isOK;
   IRuntimeListener* m_pRTListener;
   CListenerLock*    m_pLock;

public:
   // custom functions
   CListenerLock* getListenerLock();
   IRuntime* getRuntimeAdapter();

   // functions taken from samples
   void end();
   IRuntime* getRuntime();
   btBool isOK();

   /// ========================================================================
   /// @brief        Runtime client interface
   ///

   virtual void runtimeCreateOrGetProxyFailed(IEvent const& rEvent);
   virtual void runtimeStarted(IRuntime* pRuntime, const NamedValueSet& rConfigParms);
   virtual void runtimeStopped(IRuntime* pRuntime);
   virtual void runtimeStartFailed(const IEvent& rEvent);
   virtual void runtimeStopFailed(const IEvent& rEvent);
   virtual void runtimeAllocateServiceFailed(IEvent const& rEvent);
   virtual void runtimeAllocateServiceSucceeded(IBase* pBase, TransactionID const& rTranID);
   virtual void runtimeEvent(IEvent const& rEvent);
};

#endif   // __RUNTIME_CLIENT_ADAPTER_H__
