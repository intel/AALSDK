#ifndef __RUNTIME_CLIENT_ADAPTER_H__
#define __RUNTIME_CLIENT_ADAPTER_H__

#include "aalsdk/Runtime.h"
#include "gtCommon_RTAdapter.h"

class GTCOMMON_API CRuntimeClientAdapter : public CAASBase, public IRuntimeClient
{

public:
   /// ========================================================================
   /// @brief        The default constructor, taking a shared lock.
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

   /// ================================================================
   /// @brief        Gets the listener lock.
   ///
   /// @details      Invoked when service clients needs a lock to
   ///               synchronize runtime events through a listener.
   ///
   /// @return       The listener lock.
   ///
   CListenerLock* getListenerLock();
   /// ================================================================
   /// @brief        Gets the runtime adapter.
   ///
   /// @return       The runtime adapter.
   ///
   /// @details      Provides a redirect mechanism when the framework
   ///               calls getRuntime(), plugging the custom runtime
   ///               proxy into existing framework code.
   ///
   IRuntime* getRuntimeAdapter();

   // functions taken from samples
   void end();
   IRuntime* getRuntime();
   btBool isOK();

   /// ========================================================================
   /// @brief        The runtime client interface
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
