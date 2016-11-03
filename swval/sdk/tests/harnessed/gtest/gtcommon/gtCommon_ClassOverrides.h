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
/// @todo This should ultimately get wrapped around or integrated with
/// the Synchronizing classes in mocks.
///
/// @internal     Added AutoLock to ensure serial access to the counter
/// variable. Thu 03 Nov 2016 05:26:21 AM PDT
///
class GTCOMMON_API CListenerLock : public CAASBase
{

private:
   volatile int signal_count;
   // volatile may be helpful, but does not guarantee atomic access

public:
   CListenerLock() : signal_count( 0 )
   {
   }

   virtual ~CListenerLock()
   {
   }

   void signal()
   {
      // stdout indicator to aid in debugging
      MSG( "                              >>>>>>>>>>" );
      AutoLock( this );
      --signal_count;
   }

   void wait()
   {
      while ( signal_count >= 0 ) {
         SleepZero();
      }
      // stdout indicator to aid in debugging
      MSG( "                              ||||||||||" );
      AutoLock( this );
      ++signal_count;
   }
};

#endif   // __GTCOMMON_CLASSOVERRIDES_H__
