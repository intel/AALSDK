#ifndef __GTCOMMON_CLASSOVERRIDES_H__
#define __GTCOMMON_CLASSOVERRIDES_H__

/// ===================================================================
/// @brief        This is a facilitator class for all listeners to derive
///               from CriticalSection if needed.
///
/// @details      CriticalSection is a class found in CriticalSection.h 
///               and is a member of the osal group of classes (Operating 
///               System Abstraction Layer).
/// 
class GTCOMMON_API Listener : public CriticalSection
{
public:
   virtual ~Listener()
   {
   }
};

/// ===================================================================
/// @brief        Basic, light-weight spin-lock semaphore that works without
///               native synchronization primitives.
///
/// @details      This class can be used in place of the SDK framework
///               semaphore when an atomic guarantee is not required to
///               synchronize events.
///
///               @internal     I added this because I suspected some
///               problems I was having were related to the SDK version
///               (or my incorrect use of it). I needed to factor that
///               possibility out of the equation and subsequently, left
///               this scaffolding in place since it seems to work well
///               enough for my purposes.
///
class GTCOMMON_API CListenerLock
{

private:
   volatile int signal_count;
   // volatile may be helpful, but does not guarantee atomic access
   CriticalSection* m_pCritSec;
   CSemaphore m_Sem;
   // we own deletion of the critical section

public:
   /// ================================================================
   /// @brief        Constructor, supporting wrap of the SDK semaphore.
   ///
   /// @param        pCS    Pointer to a critical section that can be
   ///                      used to create the SDK semaphore.
   ///
   CListenerLock(CriticalSection* pCS = NULL)
      : signal_count(0)
      , m_pCritSec(pCS)
   {
      m_Sem.Create(0, 2);
   }

   virtual ~CListenerLock()
   {
      delete m_pCritSec;
      m_Sem.Destroy();
   }

   /// ================================================================
   /// @brief        Copy constructor
   ///
   /// @param        lock    The lock from which to copy the critical
   ///                       section.
   ///
   CListenerLock(CListenerLock const* const lock) : signal_count(0)
   {
      m_pCritSec = lock->m_pCritSec;
      m_Sem.Create(0, 2);
   }

   void signal()
   {
      // AutoLock(m_pCritSec);
      MOCKDEBUG("                              >>>>>>>>>>");
      --signal_count;
      // m_Sem.Post(1);
   }

   void wait()
   {
      while(signal_count >= 0) {
         SleepZero();
      }
      // AutoLock(m_pCritSec);
      MOCKDEBUG("                              ||||||||||");
      ++signal_count;
      // m_Sem.Wait();
   }
};

#endif   // __GTCOMMON_CLASSOVERRIDES_H__
