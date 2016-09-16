// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifndef __GTCOMMON_RUNTIMEADAPTER_H__
#define __GTCOMMON_RUNTIMEADAPTER_H__

/// ===================================================================
/// @brief        Runtime listener interface, providing a delegation
///               interface for adapter classes.
///

class GTCOMMON_API IRuntimeListener : public Listener
{
public:
   virtual void OnRuntimeCreateOrGetProxyFailed(IEvent const& rEvent)                         = 0;
   virtual void OnRuntimeStarted(IRuntime* pRuntime, const NamedValueSet& rConfigParams)      = 0;
   virtual void OnRuntimeStopped(IRuntime* pRuntime)                                          = 0;
   virtual void OnRuntimeStartFailed(IEvent const& rEvent)                                    = 0;
   virtual void OnRuntimeStopFailed(IEvent const& rEvent)                                     = 0;
   virtual void OnRuntimeAllocateServiceFailed(IEvent const& rEvent)                          = 0;
   virtual void OnRuntimeAllocateServiceSucceeded(IBase* pBase, TransactionID const& rTranID) = 0;
   virtual void OnRuntimeEvent(IEvent const& rEvent)                                          = 0;
};

/// ===================================================================
/// @brief        Default empty implementation of the Runtime listener
///               interface.
///
class GTCOMMON_API CRuntimeListener : public IRuntimeListener
{

public:
   CRuntimeListener(CListenerLock* pLock);

   // virtual destructor to allow for deletion from a base pointer.
   virtual ~CRuntimeListener();

   virtual void OnRuntimeCreateOrGetProxyFailed(IEvent const& rEvent);
   virtual void OnRuntimeStarted(IRuntime* pRuntime, const NamedValueSet& rConfigParams);
   virtual void OnRuntimeStopped(IRuntime* pRuntime);
   virtual void OnRuntimeStartFailed(IEvent const& rEvent);
   virtual void OnRuntimeStopFailed(IEvent const& rEvent);
   virtual void OnRuntimeAllocateServiceFailed(IEvent const& rEvent);
   virtual void OnRuntimeAllocateServiceSucceeded(IBase* pBase, TransactionID const& rTranID);
   virtual void OnRuntimeEvent(IEvent const& rEvent);
   // members
   CListenerLock* m_pLock;
};

/// ===================================================================
/// @brief        Wrapper class / custom proxy for the runtime singleton.
///
class GTCOMMON_API CRuntimeAdapter : public CAASBase, public IRuntime
{

public:
   CRuntimeAdapter(IRuntimeClient* pRCA);

   void setListenerLock(CListenerLock* pLock);

   // virtual descructor to allow deletion from a base pointer
   virtual ~CRuntimeAdapter();

   virtual btBool start(const NamedValueSet& rconfigParms);
   virtual void stop();
   virtual btBool schedDispatchable(IDispatchable* pDispatchable);
   virtual IRuntime* getRuntimeProxy(IRuntimeClient* pClient);
   virtual btBool releaseRuntimeProxy();
   virtual IRuntimeClient* getRuntimeClient();
   virtual btBool IsOK();

   // required for use of the custom in-module service factory
   virtual void allocService(IBase* pServiceClient, NamedValueSet const& rManifest, TransactionID const& rTranID);

private:
   Runtime* m_pRuntimeDelegate;
   IRuntimeClient* m_pRCA;
   CListenerLock* m_pLock;
};

#endif   // __GTCOMMON_RUNTIMEADAPTER_H__
