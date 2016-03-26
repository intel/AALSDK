// INTEL CONFIDENTIAL - For Intel Internal Use Only

#ifdef DBG_CSEMAPHORE

BEGIN_NAMESPACE(AAL)
   BEGIN_NAMESPACE(Testing)

class OSAL_API IAfterCSemaphoreAutoLock
{
public:
   virtual ~IAfterCSemaphoreAutoLock() {}

   virtual void     OnCreate(AAL::btInt , AAL::btUnsignedInt ) = 0; // [0] called when CSemaphore::Create() acquires AutoLock()
   virtual void    OnDestroy()                                 = 0; // [1] called when CSemaphore::Destroy() acquires AutoLock()
   virtual void      OnReset(AAL::btInt )                      = 0; // [2] called when CSemaphore::Reset(btInt ) acquires AutoLock()
   virtual void OnCurrCounts(AAL::btInt & , AAL::btInt & )     = 0; // [3] called when CSemaphore::CurrCounts(btInt & , btInt & ) acquires AutoLock()
   virtual void       OnPost(AAL::btInt )                      = 0; // [4] called when CSempahore::Post(btInt )  acquires AutoLock()
   virtual void OnUnblockAll()                                 = 0; // [5] called when CSempahore::UnblockAll()  acquires AutoLock()
   virtual void       OnWait()                                 = 0; // [6] called when CSempahore::Wait()        acquires AutoLock()
   virtual void       OnWait(AAL::btTime )                     = 0; // [7] called when CSempahore::Wait(btTime ) acquires AutoLock()
};

class OSAL_API EmptyAfterCSemaphoreAutoLock : public AAL::Testing::IAfterCSemaphoreAutoLock
{
public:
   EmptyAfterCSemaphoreAutoLock()                              {}
   virtual void     OnCreate(AAL::btInt , AAL::btUnsignedInt ) {}
   virtual void    OnDestroy()                                 {}
   virtual void      OnReset(AAL::btInt )                      {}
   virtual void OnCurrCounts(AAL::btInt & , AAL::btInt & )     {}
   virtual void       OnPost(AAL::btInt )                      {}
   virtual void OnUnblockAll()                                 {}
   virtual void       OnWait()                                 {}
   virtual void       OnWait(AAL::btTime )                     {}
};

   END_NAMESPACE(Testing)
END_NAMESPACE(AAL)

#endif // DBG_CSEMAPHORE
