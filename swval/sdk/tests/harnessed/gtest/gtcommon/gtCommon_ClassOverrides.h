// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifndef __GTCOMMON_CLASSOVERRIDES_H__
#define __GTCOMMON_CLASSOVERRIDES_H__

/// ===================================================================
/// @brief        A facilitator class for listeners to derive from
///               CriticalSection as needed.
///
class GTCOMMON_API Listener : public CAASBase
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
class GTCOMMON_API CListenerLock
{

private:
   volatile int signal_count;
   // volatile may be helpful, but does not guarantee atomic access

public:
   /// ================================================================
   /// @brief        A constructor, supporting wrap of the SDK semaphore.
   ///
   /// @details      Default value is NULL as the use of this parameter
   ///               depends on disabled code.
   ///
   CListenerLock() : signal_count(0)
   {
   }

   virtual ~CListenerLock()
   {
   }

   void signal()
   {
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
      // stdout indicator to aid in debugging
      MOCKDEBUG("                              ||||||||||");
      ++signal_count;
      // m_Sem.Wait();
   }
};

#endif   // __GTCOMMON_CLASSOVERRIDES_H__
