// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifndef __AAL0014_H__
#define __AAL0014_H__
#include "isolated.h"

void aal0014();

class aal0014Fixture : public TestFixture
{
public:
   aal0014Fixture() {}
   virtual ~aal0014Fixture() {}

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

   OSLThread          *m_pThrs[3];
   btTID               m_TIDs[3];
   volatile btUIntPtr  m_Scratch[10];
   CSemaphore          m_Semaphore;

   static void Thr0(OSLThread * , void * );
   static void Thr1(OSLThread * , void * );

   static void EmptySIGUSR1Handler(int , siginfo_t * , void * );
};

#endif // __AAL0014_H__

