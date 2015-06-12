// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifndef __GTTHREADGROUP_H__
#define __GTTHREADGROUP_H__
#include "gtCommon.h"

// Note: we don't 'delete this' in any of the operator()'s here, because the work items
//       are all tracked within each test fixture to ensure none are lost.

class PostThenWaitD : public IDispatchable
{
public:
   PostThenWaitD(CSemaphore &ToPost,
                 CSemaphore &ToWait,
                 AAL::btInt  PostCount=1,
                 AAL::btTime WaitTimeout=AAL_INFINITE_WAIT,
                 AAL::btBool ExpPostRes=true,
                 AAL::btBool ExpWaitRes=true) :
      m_ToPost(ToPost),
      m_ToWait(ToWait),
      m_PostCount(PostCount),
      m_WaitTimeout(WaitTimeout),
      m_ExpPostRes(ExpPostRes),
      m_ExpWaitRes(ExpWaitRes)
   {}
   virtual ~PostThenWaitD() {}

   virtual void operator() ()
   {
      EXPECT_EQ(m_ExpPostRes, m_ToPost.Post(m_PostCount));
      EXPECT_EQ(m_ExpWaitRes, m_ToWait.Wait(m_WaitTimeout));
   }

protected:
   CSemaphore &m_ToPost;
   CSemaphore &m_ToWait;
   AAL::btInt  m_PostCount;
   AAL::btTime m_WaitTimeout;
   AAL::btBool m_ExpPostRes;
   AAL::btBool m_ExpWaitRes;
};

class NopD : public IDispatchable
{
public:
   NopD() {}
   virtual ~NopD() {}
   virtual void operator() () { ; }
};

class YieldD : public IDispatchable
{
public:
   YieldD() {}
   virtual ~YieldD() {}
   virtual void operator() ()
   {
      cpu_yield();
   }
};

class AddNopToThreadGroupD : public IDispatchable
{
public:
   AddNopToThreadGroupD(OSLThreadGroup    *pTG,
                        AAL::btUnsignedInt NumToAdd=1,
                        AAL::btBool        bExpectedResult=true) :
      m_pTG(pTG),
      m_NumToAdd(NumToAdd),
      m_bExpectedResult(bExpectedResult)
   {}
   virtual ~AddNopToThreadGroupD() {}

   virtual void operator() ()
   {
      AAL::btUnsignedInt i;
      for ( i = 0 ; i < m_NumToAdd ; ++i ) {
         NopD *nop = new NopD();
         ASSERT_NONNULL(nop);

         AAL::btBool res = m_pTG->Add(nop);
         EXPECT_EQ(res, m_bExpectedResult);
         if ( !res ) {
            delete nop;
         }
      }
   }

protected:
   OSLThreadGroup    *m_pTG;
   AAL::btUnsignedInt m_NumToAdd;
   AAL::btBool        m_bExpectedResult;
};

class PostD : public IDispatchable
{
public:
   PostD(CSemaphore &ToPost,
         AAL::btInt  PostCount=1,
         AAL::btBool ExpPostRes=true) :
      m_ToPost(ToPost),
      m_PostCount(PostCount),
      m_ExpPostRes(ExpPostRes)
   {}
   virtual ~PostD() {}

   virtual void operator() ()
   {
      EXPECT_EQ(m_ExpPostRes, m_ToPost.Post(m_PostCount));
   }

protected:
   CSemaphore &m_ToPost;
   AAL::btInt  m_PostCount;
   AAL::btBool m_ExpPostRes;
};

class SleepThenPostD : public IDispatchable
{
public:
   SleepThenPostD(AAL::btUnsignedInt MillisToSleep,
                  CSemaphore        &ToPost,
                  AAL::btInt         PostCount=1,
                  AAL::btBool        ExpPostRes=true) :
      m_MillisToSleep(MillisToSleep),
      m_ToPost(ToPost),
      m_PostCount(PostCount),
      m_ExpPostRes(ExpPostRes)
   {}
   virtual ~SleepThenPostD() {}

   virtual void operator() ()
   {
      SleepMilli(m_MillisToSleep);
      EXPECT_EQ(m_ExpPostRes, m_ToPost.Post(m_PostCount));
   }

protected:
   AAL::btUnsignedInt m_MillisToSleep;
   CSemaphore        &m_ToPost;
   AAL::btInt         m_PostCount;
   AAL::btBool        m_ExpPostRes;
};

class WaitD : public IDispatchable
{
public:
   WaitD(CSemaphore &ToWait,
         AAL::btTime WaitTimeout=AAL_INFINITE_WAIT,
         AAL::btBool ExpWaitRes=true) :
      m_ToWait(ToWait),
      m_WaitTimeout(WaitTimeout),
      m_ExpWaitRes(ExpWaitRes)
   {}
   virtual ~WaitD() {}

   virtual void operator() ()
   {
      EXPECT_EQ(m_ExpWaitRes, m_ToWait.Wait(m_WaitTimeout));
   }

protected:
   CSemaphore &m_ToWait;
   AAL::btTime m_WaitTimeout;
   AAL::btBool m_ExpWaitRes;
};

class PostThenWaitThenPostD : public IDispatchable
{
public:
   PostThenWaitThenPostD(CSemaphore &ToPost0,
                         CSemaphore &ToWait0,
                         CSemaphore &ToPost1,
                         AAL::btInt  PostCount0=1,
                         AAL::btTime WaitTimeout0=AAL_INFINITE_WAIT,
                         AAL::btBool ExpPostRes0=true,
                         AAL::btBool ExpWaitRes0=true,
                         AAL::btInt  PostCount1=1,
                         AAL::btBool ExpPostRes1=true) :
      m_ToPost0(ToPost0),
      m_ToWait0(ToWait0),
      m_ToPost1(ToPost1),
      m_PostCount0(PostCount0),
      m_WaitTimeout0(WaitTimeout0),
      m_ExpPostRes0(ExpPostRes0),
      m_ExpWaitRes0(ExpWaitRes0),
      m_PostCount1(PostCount1),
      m_ExpPostRes1(ExpPostRes1)
   {}
   virtual ~PostThenWaitThenPostD() {}

   virtual void operator() ()
   {
      EXPECT_EQ(m_ExpPostRes0, m_ToPost0.Post(m_PostCount0));
      EXPECT_EQ(m_ExpWaitRes0, m_ToWait0.Wait(m_WaitTimeout0));
      EXPECT_EQ(m_ExpPostRes1, m_ToPost1.Post(m_PostCount1));
   }

protected:
   CSemaphore &m_ToPost0;
   CSemaphore &m_ToWait0;
   CSemaphore &m_ToPost1;
   AAL::btInt  m_PostCount0;
   AAL::btTime m_WaitTimeout0;
   AAL::btBool m_ExpPostRes0;
   AAL::btBool m_ExpWaitRes0;
   AAL::btInt  m_PostCount1;
   AAL::btBool m_ExpPostRes1;
};

class UnsafeCountUpD : public IDispatchable
{
public:
   UnsafeCountUpD(AAL::btInt &i,
                  AAL::btInt incr=1) :
      m_i(i),
      m_incr(incr)
   {}
   virtual ~UnsafeCountUpD() {}
   virtual void operator() ()
   {
      m_i += m_incr;
   }

protected:
   AAL::btInt &m_i;
   AAL::btInt  m_incr;
};

class DelUnsafeCountUpD : public IDispatchable
{
public:
   DelUnsafeCountUpD(AAL::btInt &i,
                     AAL::btInt incr=1) :
      m_i(i),
      m_incr(incr)
   {}
   virtual ~DelUnsafeCountUpD() {}
   virtual void operator() ()
   {
      m_i += m_incr;
      delete this;
   }

protected:
   AAL::btInt &m_i;
   AAL::btInt  m_incr;
};

class UnsafeCountDownD : public IDispatchable
{
public:
   UnsafeCountDownD(AAL::btInt &i,
                    AAL::btInt  decr=1) :
      m_i(i),
      m_decr(decr)
   {}
   virtual ~UnsafeCountDownD() {}
   virtual void operator() ()
   {
      m_i -= m_decr;
   }

protected:
   AAL::btInt &m_i;
   AAL::btInt  m_decr;
};

class StopThreadGroupD : public IDispatchable
{
public:
   StopThreadGroupD(OSLThreadGroup *pTG) :
      m_pTG(pTG)
   {}
   virtual ~StopThreadGroupD() {}
   virtual void operator() ()
   {
      m_pTG->Stop();
   }

protected:
   OSLThreadGroup *m_pTG;
};

class DrainThreadGroupD : public IDispatchable
{
public:
   DrainThreadGroupD(OSLThreadGroup *pTG,
                     AAL::btBool     bExpectedResult=true) :
      m_pTG(pTG),
      m_bExpectedResult(bExpectedResult)
   {}
   virtual ~DrainThreadGroupD() {}
   virtual void operator() ()
   {
      EXPECT_EQ(m_bExpectedResult, m_pTG->Drain());
   }

protected:
   OSLThreadGroup *m_pTG;
   AAL::btBool     m_bExpectedResult;
};

class JoinThreadGroupD : public IDispatchable
{
public:
   JoinThreadGroupD(OSLThreadGroup *pTG,
                    AAL::btTime     Timeout=AAL_INFINITE_WAIT,
                    AAL::btBool     bExpectedResult=true) :
      m_pTG(pTG),
      m_Timeout(Timeout),
      m_bExpectedResult(bExpectedResult)
   {}
   virtual ~JoinThreadGroupD() {}
   virtual void operator() ()
   {
      EXPECT_EQ(m_bExpectedResult, m_pTG->Join(m_Timeout));
   }

protected:
   OSLThreadGroup *m_pTG;
   AAL::btTime     m_Timeout;
   AAL::btBool     m_bExpectedResult;
};

class DestroyThreadGroupD : public IDispatchable
{
public:
   DestroyThreadGroupD(OSLThreadGroup *pTG,
                       AAL::btTime     Timeout=AAL_INFINITE_WAIT,
                       AAL::btBool     bExpectedResult=true) :
      m_pTG(pTG),
      m_Timeout(Timeout),
      m_bExpectedResult(bExpectedResult)
   {}
   virtual ~DestroyThreadGroupD() {}
   virtual void operator() ()
   {
      EXPECT_EQ(m_bExpectedResult, m_pTG->Destroy(m_Timeout));
   }

protected:
   OSLThreadGroup *m_pTG;
   AAL::btTime     m_Timeout;
   AAL::btBool     m_bExpectedResult;
};

#if 0
class SetThreadGroupPtrToNULLD : public IDispatchable
{
public:
   SetThreadGroupPtrToNULLD(OSLThreadGroup * & pTG) :
      m_pTG(pTG)
   {}
   virtual ~SetThreadGroupPtrToNULLD() {}
   virtual void operator() ()
   {
      m_pTG = NULL;
   }

protected:
   OSLThreadGroup * & m_pTG;
};
#endif

#endif // __GTTHREADGROUP_H__

