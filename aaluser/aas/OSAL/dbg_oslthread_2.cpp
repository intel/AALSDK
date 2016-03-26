// INTEL CONFIDENTIAL - For Intel Internal Use Only

// dbg_oslthread_2.cpp

#ifdef DBG_OSLTHREAD

# if   defined( __AAL_WINDOWS__ )

   //std::cout << "thr: OSLThread::StartThread() del " << ::AAL::GetThreadID() << std::endl << std::flush;
   ::AAL::Testing::DbgOSLThreadDelThr(::AAL::GetThreadID());

# elif defined( __AAL_LINUX__ )

   pthread_cleanup_pop(0);
   {
      AutoLock(&::AAL::Testing::gOSLThreadCountLock);
      --::AAL::Testing::gOSLThreadCount;
   }

# endif // OS

#endif // DBG_OSLTHREAD

