// INTEL CONFIDENTIAL - For Intel Internal Use Only

#ifdef DBG_BARRIER

BEGIN_NAMESPACE(AAL)
   BEGIN_NAMESPACE(Testing)

class OSAL_API IAfterBarrierAutoLock
{
public:
   virtual ~IAfterBarrierAutoLock() {}

   virtual void     OnCreate(AAL::btUnsignedInt , AAL::btBool )            = 0; // [0] called when Barrier::Create(btUnsignedInt , btBool ) acquires AutoLock()
   virtual void    OnDestroy()                                             = 0; // [1] called when Barrier::Destroy() acquires AutoLock()
   virtual void      OnReset(AAL::btUnsignedInt )                          = 0; // [2] called when Barrier::Reset(btUnsignedInt ) acquires AutoLock()
   virtual void OnCurrCounts(AAL::btUnsignedInt & , AAL::btUnsignedInt & ) = 0; // [3] called when Barrier::CurrCounts(btUnsignedInt & , btUnsignedInt & ) acquires AutoLock()
   virtual void       OnPost(AAL::btUnsignedInt )                          = 0; // [4] called when Barrier::Post(btUnsignedInt ) acquires AutoLock()
   virtual void OnUnblockAll()                                             = 0; // [5] called when Barrier::UnblockAll() acquires AutoLock()
   virtual void       OnWait()                                             = 0; // [6] called when Barrier::Wait() acquires AutoLock()
   virtual void       OnWait(AAL::btTime )                                 = 0; // [7] called when Barrier::Wait(btTime ) acquires AutoLock()

   virtual void      OnSleep()                                             = 0; // Called just prior to INFINITE wait on _UnlockedWaitForSingleObject / _PThreadCondWait
   virtual void      OnSleep(AAL::btTime )                                 = 0; // Called just prior to timed wait on _UnlockedWaitForSingleObject / _PThreadCondTimedWait
};

class OSAL_API EmptyAfterBarrierAutoLock : public AAL::Testing::IAfterBarrierAutoLock
{
public:
   EmptyAfterBarrierAutoLock()                                             {}
   virtual void     OnCreate(AAL::btUnsignedInt , AAL::btBool )            {}
   virtual void    OnDestroy()                                             {}
   virtual void      OnReset(AAL::btUnsignedInt )                          {}
   virtual void OnCurrCounts(AAL::btUnsignedInt & , AAL::btUnsignedInt & ) {}
   virtual void       OnPost(AAL::btUnsignedInt )                          {}
   virtual void OnUnblockAll()                                             {}
   virtual void       OnWait()                                             {}
   virtual void       OnWait(AAL::btTime )                                 {}
   virtual void      OnSleep()                                             {}
   virtual void      OnSleep(AAL::btTime )                                 {}
};

   END_NAMESPACE(Testing)
END_NAMESPACE(AAL)

#endif // DBG_BARRIER
