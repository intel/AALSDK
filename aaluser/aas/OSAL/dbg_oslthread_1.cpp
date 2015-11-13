// INTEL CONFIDENTIAL - For Intel Internal Use Only

// dbg_oslthread_1.cpp

#ifdef DBG_OSLTHREAD

# if   defined( __AAL_WINDOWS__ )



# elif defined( __AAL_LINUX__ )

   {
      AutoLock(&AAL::Testing::gOSLThreadCountLock);
      ++AAL::Testing::gOSLThreadCount;
   }
   pthread_cleanup_push(AAL::Testing::pthread_OSLThreadCount_cleanup, pThread);

# endif // OS

#endif // DBG_OSLTHREAD

