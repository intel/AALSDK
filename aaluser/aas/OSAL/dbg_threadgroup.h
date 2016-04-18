// INTEL CONFIDENTIAL - For Intel Internal Use Only

#ifdef DBG_THREADGROUP

BEGIN_NAMESPACE(AAL)
   BEGIN_NAMESPACE(Testing)

class OSAL_API IAfterThreadGroupAutoLock
{
public:
   virtual ~IAfterThreadGroupAutoLock() {}

   virtual void     OnAdd(AAL::IDispatchable * ) = 0; // [0] called when OSLThreadGroup::ThrGrpState::Add(IDispatchable * ) acquires AutoLock()
   virtual void   OnStart()                      = 0; // [1] called when OSLThreadGroup::ThrGrpState::Start() acquires AutoLock()
   virtual void    OnStop()                      = 0; // [2] called when OSLThreadGroup::ThrGrpState::Stop() acquires AutoLock()
   virtual void   OnDrain()                      = 0; // [3] called when OSLThreadGroup::ThrGrpState::Drain() acquires AutoLock()
   virtual void    OnJoin(AAL::btTime )          = 0; // [4] called when OSLThreadGroup::ThrGrpState::Join(btTime ) acquires AutoLock()
   virtual void OnDestroy(AAL::btTime )          = 0; // [5] called when OSLThreadGroup::ThrGrpState::Destroy(btTime ) acquires AutoLock()
};

class OSAL_API EmptyAfterThreadGroupAutoLock : public AAL::Testing::IAfterThreadGroupAutoLock
{
public:
   EmptyAfterThreadGroupAutoLock()               {}
   virtual void     OnAdd(AAL::IDispatchable * ) {}
   virtual void   OnStart()                      {}
   virtual void    OnStop()                      {}
   virtual void   OnDrain()                      {}
   virtual void    OnJoin(AAL::btTime )          {}
   virtual void OnDestroy(AAL::btTime )          {}
};

   END_NAMESPACE(Testing)
END_NAMESPACE(AAL)

#endif // DBG_THREADGROUP

