// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifndef __GTCOMMON_CLASSOVERRIDES_H__
#define __GTCOMMON_CLASSOVERRIDES_H__

/// ===================================================================
/// @brief        A facilitator class for listeners to derive from
///               CriticalSection as needed.
///
/// @details      CriticalSection is a class found in CriticalSection.h
///               and is a member of the osal group of classes
///               (Operating System Abstraction Layer).
///
class GTCOMMON_API Listener : public CAASBase // changing this from CriticalSection to CAASBase Fri 23 Sep
                                              // 2016 09:22:55 AM PDT
{
public:
   virtual ~Listener()
   {
   }
};

/// ===================================================================
/// @brief        A basic, light-weight spin-lock semaphore that works
///               without native synchronization primitives.
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
   // we own deletion of the critical section
   CSemaphore m_Sem;

public:
   /// ================================================================
   /// @brief        A constructor, supporting wrap of the SDK semaphore.
   ///
   /// @param        pCS    A pointer to the critical section, that will
   ///                      be used to create the SDK semaphore if
   ///                      required.
   ///
   /// @details      Default value is NULL as the use of this parameter
   ///               depends on disabled code.
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
   /// @brief        The copy constructor
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
      // stdout indicator to aid in debugging 
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
      // stdout indicator to aid in debugging 
      MOCKDEBUG("                              ||||||||||");
      ++signal_count;
      // m_Sem.Wait();
   }
};

#endif   // __GTCOMMON_CLASSOVERRIDES_H__
