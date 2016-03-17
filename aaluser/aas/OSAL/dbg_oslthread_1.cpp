// INTEL CONFIDENTIAL - For Intel Internal Use Only

// dbg_oslthread_1.cpp

#ifdef DBG_OSLTHREAD

   {
      AutoLock(&::AAL::Testing::gOSLThreadCountLock);
      ++::AAL::Testing::gOSLThreadCount;
   }

# if defined( __AAL_LINUX__ )

   pthread_cleanup_push(::AAL::Testing::pthread_OSLThreadCount_cleanup, pThread);

# endif // OS

#endif // DBG_OSLTHREAD

