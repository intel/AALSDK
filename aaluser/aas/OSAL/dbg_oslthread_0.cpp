// INTEL CONFIDENTIAL - For Intel Internal Use Only

// dbg_oslthread_0.cpp

#ifdef DBG_OSLTHREAD

namespace Testing {

static volatile AAL::btUIntPtr gOSLThreadCount = 0;

OSAL_API AAL::btUIntPtr DbgOSLThreadCount()
{
   return gOSLThreadCount;
}

static AAL::CriticalSection gOSLThreadCountLock;

} // Testing

# if   defined( __AAL_WINDOWS__ )

namespace Testing {

static std::map<AAL::btTID, AAL::btUnsigned32bitInt> gOSLThreadMap;

static void DbgOSLThreadNewThr(AAL::btTID tid)
{
   AutoLock(&gOSLThreadCountLock);

   std::map<AAL::btTID, AAL::btUnsigned32bitInt>::iterator iter = gOSLThreadMap.find(tid);

   if ( gOSLThreadMap.end() == iter ) {
      // tid not found - add it.
      gOSLThreadMap.insert(std::make_pair(tid, 1));
      ++gOSLThreadCount;
   } else {
      // increment the tracking count for tid.
      ++(iter->second);
   }
}

OSAL_API void DbgOSLThreadDelThr(AAL::btTID tid)
{
   AutoLock(&gOSLThreadCountLock);

   std::map<AAL::btTID, AAL::btUnsigned32bitInt>::iterator iter = gOSLThreadMap.find(tid);

   if ( gOSLThreadMap.end() != iter ) {
      --(iter->second);
      if ( 0 == iter->second ) {
         // Done tracking tid.
         gOSLThreadMap.erase(iter);
         --gOSLThreadCount;
      }
   }
}

} // Testing

BOOL APIENTRY DllMain(HANDLE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved)
{
   HANDLE hTemp  = hModule;
   LPVOID lpVoid = lpReserved;
   switch ( ul_reason_for_call ) {
      case DLL_PROCESS_ATTACH : /* FALL THROUGH */
      case DLL_PROCESS_DETACH : break;

      case DLL_THREAD_ATTACH  : {
         //std::cout << "thr: DllMain() new " << ::AAL::GetThreadID() << std::endl << std::flush;
         ::AAL::Testing::DbgOSLThreadNewThr(::AAL::GetThreadID());
      } break;

      case DLL_THREAD_DETACH  : {
         //std::cout << "thr: DllMain() del " << ::AAL::GetThreadID() << std::endl << std::flush;
         ::AAL::Testing::DbgOSLThreadDelThr(::AAL::GetThreadID());
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

