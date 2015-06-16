// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifndef __AAL0017_H__
#define __AAL0017_H__
#include "isolated.h"

void aal0017();

class aal0017Fixture : public TestFixture
{
public:
   aal0017Fixture() {}
   virtual ~aal0017Fixture() {}

   virtual void SetUp()
   {
      unsigned i;
      for ( i = 0 ; i < sizeof(m_pThrs) / sizeof(m_pThrs[0]) ; ++i ) {
         m_pThrs[i] = NULL;
         m_TIDs[i]  = 0;
      }

      for ( i = 0 ; i < sizeof(m_Scratch) / sizeof(m_Scratch[0]) ; ++i ) {
         m_Scratch[i] = 0;
      }
   }

   virtual void Run();

   virtual void TearDown()
   {
      YIELD_WHILE(CurrentThreads() > 0);
      m_Semaphore.Destroy();
   }

   OSLThread  *m_pThrs[3];
   btTID       m_TIDs[3];
   btUIntPtr   m_Scratch[10];
   CSemaphore  m_Semaphore;

   static void Thr0(OSLThread * , void * );
};

#endif // __AAL0017_H__

