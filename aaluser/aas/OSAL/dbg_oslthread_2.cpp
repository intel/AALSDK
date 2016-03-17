// INTEL CONFIDENTIAL - For Intel Internal Use Only

// dbg_oslthread_2.cpp

#ifdef DBG_OSLTHREAD

// Don't manipulate gOSLThreadCount here for Windows. See dbg_oslthread_0.cpp

# if defined( __AAL_LINUX__ )

   pthread_cleanup_pop(0);
   {
      AutoLock(&::AAL::Testing::gOSLThreadCountLock);
      --::AAL::Testing::gOSLThreadCount;
   }

# endif // OS

#endif // DBG_OSLTHREAD

