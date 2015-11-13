// INTEL CONFIDENTIAL - For Intel Internal Use Only

// dbg_oslthread_0.cpp

#ifdef DBG_OSLTHREAD

# if   defined( __AAL_WINDOWS__ )



# elif defined( __AAL_LINUX__ )

namespace Testing {

static volatile btUIntPtr gOSLThreadCount = 0;

btUIntPtr DbgOSLThreadCount()
{
   return gOSLThreadCount;
}

static CriticalSection gOSLThreadCountLock;

static void pthread_OSLThreadCount_cleanup(void *arg)
{
   // Executes when the thread is canceled or if the thread terminates by calling pthread_exit().
   // Does not execute when the thread returns normally.
   AutoLock(&gOSLThreadCountLock);
   --gOSLThreadCount;
}

} // Testing

# endif // OS

#endif // DBG_OSLTHREAD

