// INTEL CONFIDENTIAL - For Intel Internal Use Only

// dbg_oslthread_0.cpp

#ifdef DBG_OSLTHREAD

namespace Testing {

static volatile btUIntPtr gOSLThreadCount = 0;

btUIntPtr DbgOSLThreadCount()
{
   return gOSLThreadCount;
}

static CriticalSection gOSLThreadCountLock;

} // Testing

# if   defined( __AAL_WINDOWS__ )

BOOL APIENTRY DllMain(HANDLE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved)
{
   HANDLE hTemp  = hModule;
   LPVOID lpVoid = lpReserved;
   switch ( ul_reason_for_call ) {
      case DLL_PROCESS_ATTACH : /* FALL THROUGH */
      case DLL_THREAD_ATTACH  : /* FALL THROUGH */
      case DLL_PROCESS_DETACH : break;

      case DLL_THREAD_DETACH  : {

         AutoLock(&::AAL::Testing::gOSLThreadCountLock);
         --::AAL::Testing::gOSLThreadCount;

      } break;
   }
   return TRUE;
}

# elif defined( __AAL_LINUX__ )

namespace Testing {

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

