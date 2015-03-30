// INTEL CONFIDENTIAL - For Intel Internal Use Only

// dbg_oslthread_2.cpp

#ifdef DBG_OSLTHREAD

# if   defined( __AAL_WINDOWS__ )



# elif defined( __AAL_LINUX__ )

   pthread_cleanup_pop(0);
   gOSLThreadCountLock.Lock();
   --gOSLThreadCount;
   gOSLThreadCountLock.Unlock();

# endif // OS

#endif // DBG_OSLTHREAD

