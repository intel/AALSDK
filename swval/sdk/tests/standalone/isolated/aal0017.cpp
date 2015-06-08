// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "aal0017.h"

static void aal0017_testcase()
{
   aal0017Fixture f;
   Execute(&f);
}

void aal0017()
{
   int       i = 0;
   const int N = 1000;

   while ( 1 ) {
      aal0017_testcase();

      ++i;
      if ( i == N ) {
         std::cerr << "17 ";
         i = 0;
      }
   }
}

void aal0017Fixture::Thr0(OSLThread *pThread, void *pContext)
{
   aal0017Fixture *pTC = static_cast<aal0017Fixture *>(pContext);
   ASSERT_NONNULL(pTC);

   pTC->m_Scratch[0] = 1; // set the first scratch space to 1 to signify that we're running.
   pTC->m_Semaphore.Wait();
}

void aal0017Fixture::Run()
{
   // After calling OSLThread::Cancel() for a thread other than the current thread, the current
   // thread should be able to immediately OSLThread::Join() the canceled thread.

   ASSERT_TRUE(m_Semaphore.Create(0, 1));

   m_pThrs[0] = new OSLThread(aal0017Fixture::Thr0,
                              OSLThread::THREADPRIORITY_NORMAL,
                              this,
                              false);

   EXPECT_TRUE(m_pThrs[0]->IsOK());

   // wait for Thr0 to begin.
   YIELD_WHILE(0 == m_Scratch[0]);
   YIELD_X(5);

   // cancel the child thread.
   m_pThrs[0]->Cancel();

   EXPECT_TRUE(m_Semaphore.Post(1));

   m_pThrs[0]->Join();
   delete m_pThrs[0];
}

