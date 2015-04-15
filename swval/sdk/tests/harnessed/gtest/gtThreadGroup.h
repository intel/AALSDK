// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifndef __GTTHREADGROUP_H__
#define __GTTHREADGROUP_H__
#include "gtCommon.h"

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

   virtual void operator() ()
   {
      EXPECT_EQ(m_ExpPostRes, m_ToPost.Post(m_PostCount));
      EXPECT_EQ(m_ExpWaitRes, m_ToWait.Wait(m_WaitTimeout));
      delete this;
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
   virtual void operator() () { delete this; }
};

class YieldD : public IDispatchable
{
public:
   YieldD() {}
   virtual void operator() ()
   {
      cpu_yield();
      delete this;
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
      delete this;
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

   virtual void operator() ()
   {
      EXPECT_EQ(m_ExpPostRes, m_ToPost.Post(m_PostCount));
      delete this;
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

   virtual void operator() ()
   {
      SleepMilli(m_MillisToSleep);
      EXPECT_EQ(m_ExpPostRes, m_ToPost.Post(m_PostCount));
      delete this;
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

   virtual void operator() ()
   {
      EXPECT_EQ(m_ExpWaitRes, m_ToWait.Wait(m_WaitTimeout));
      delete this;
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

   virtual void operator() ()
   {
      EXPECT_EQ(m_ExpPostRes0, m_ToPost0.Post(m_PostCount0));
      EXPECT_EQ(m_ExpWaitRes0, m_ToWait0.Wait(m_WaitTimeout0));
      EXPECT_EQ(m_ExpPostRes1, m_ToPost1.Post(m_PostCount1));
      delete this;
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
   virtual void operator() ()
   {
      m_i -= m_decr;
      delete this;
   }

protected:
   AAL::btInt &m_i;
   AAL::btInt  m_decr;
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

   virtual void operator() ()
   {
      EXPECT_EQ(m_bExpectedResult, m_pTG->Join(m_Timeout));
      delete this;
   }

protected:
   OSLThreadGroup *m_pTG;
   AAL::btTime     m_Timeout;
   AAL::btBool     m_bExpectedResult;
};

class PostThenJoinThreadGroupD : public IDispatchable
{
public:
   PostThenJoinThreadGroupD(CSemaphore     &ToPost,
                            AAL::btInt      ValueToPost,
                            OSLThreadGroup *pTG,
                            AAL::btBool     bExpectPostRes=true,
                            AAL::btTime     JoinTimeout=AAL_INFINITE_WAIT,
                            AAL::btBool     bExpectJoinRes=true) :
      m_ToPost(ToPost),
      m_ValueToPost(ValueToPost),
      m_pTG(pTG),
      m_bExpectPostRes(bExpectPostRes),
      m_JoinTimeout(JoinTimeout),
      m_bExpectJoinRes(bExpectJoinRes)
   {}

   virtual void operator() ()
   {
      EXPECT_EQ(m_bExpectPostRes, m_ToPost.Post(m_ValueToPost));
      EXPECT_EQ(m_bExpectJoinRes, m_pTG->Join(m_JoinTimeout));
      delete this;
   }

protected:
   CSemaphore     &m_ToPost;
   AAL::btInt      m_ValueToPost;
   OSLThreadGroup *m_pTG;
   AAL::btBool     m_bExpectPostRes;
   AAL::btTime     m_JoinTimeout;
   AAL::btBool     m_bExpectJoinRes;
};

class SleepThenPostThenJoinThreadGroupD : public IDispatchable
{
public:
   SleepThenPostThenJoinThreadGroupD(AAL::btTime     SleepMillis,
                                     CSemaphore     &ToPost,
                                     AAL::btInt      ValueToPost,
                                     OSLThreadGroup *pTG,
                                     AAL::btBool     bExpectPostRes=true,
                                     AAL::btTime     JoinTimeout=AAL_INFINITE_WAIT,
                                     AAL::btBool     bExpectJoinRes=true) :
      m_SleepMillis(SleepMillis),
      m_ToPost(ToPost),
      m_ValueToPost(ValueToPost),
      m_pTG(pTG),
      m_bExpectPostRes(bExpectPostRes),
      m_JoinTimeout(JoinTimeout),
      m_bExpectJoinRes(bExpectJoinRes)
   {}

   virtual void operator() ()
   {
      SleepMilli(m_SleepMillis);
      EXPECT_EQ(m_bExpectPostRes, m_ToPost.Post(m_ValueToPost));
      EXPECT_EQ(m_bExpectJoinRes, m_pTG->Join(m_JoinTimeout));
      delete this;
   }

protected:
   AAL::btTime     m_SleepMillis;
   CSemaphore     &m_ToPost;
   AAL::btInt      m_ValueToPost;
   OSLThreadGroup *m_pTG;
   AAL::btBool     m_bExpectPostRes;
   AAL::btTime     m_JoinTimeout;
   AAL::btBool     m_bExpectJoinRes;
};

class SleepThenPostThenDeleteThreadGroupD : public IDispatchable
{
public:
   SleepThenPostThenDeleteThreadGroupD(AAL::btTime     SleepMillis,
                                       CSemaphore     &ToPost,
                                       AAL::btInt      ValueToPost,
                                       OSLThreadGroup *pTG,
                                       AAL::btBool     bExpectPostRes=true) :
      m_SleepMillis(SleepMillis),
      m_ToPost(ToPost),
      m_ValueToPost(ValueToPost),
      m_pTG(pTG),
      m_bExpectPostRes(bExpectPostRes)
   {}

   virtual void operator() ()
   {
      SleepMilli(m_SleepMillis);
      EXPECT_EQ(m_bExpectPostRes, m_ToPost.Post(m_ValueToPost));
      delete m_pTG;
      delete this;
   }

protected:
   AAL::btTime     m_SleepMillis;
   CSemaphore     &m_ToPost;
   AAL::btInt      m_ValueToPost;
   OSLThreadGroup *m_pTG;
   AAL::btBool     m_bExpectPostRes;
};

class PostThenDeleteThreadGroupD : public IDispatchable
{
public:
   PostThenDeleteThreadGroupD(CSemaphore     &ToPost,
                              AAL::btInt      ValueToPost,
                              OSLThreadGroup *pTG,
                              AAL::btBool     bExpectPostRes=true) :
      m_ToPost(ToPost),
      m_ValueToPost(ValueToPost),
      m_pTG(pTG),
      m_bExpectPostRes(bExpectPostRes)
   {}

   virtual void operator() ()
   {
      EXPECT_EQ(m_bExpectPostRes, m_ToPost.Post(m_ValueToPost));
      delete m_pTG;
      delete this;
   }

protected:
   CSemaphore     &m_ToPost;
   AAL::btInt      m_ValueToPost;
   OSLThreadGroup *m_pTG;
   AAL::btBool     m_bExpectPostRes;
};

class DeleteThreadGroupD : public IDispatchable
{
public:
   DeleteThreadGroupD(OSLThreadGroup *pTG) :
      m_pTG(pTG)
   {}

   virtual void operator() ()
   {
      delete m_pTG;
      delete this;
   }

protected:
   OSLThreadGroup *m_pTG;
};


class SetThreadGroupPtrToNULLD : public IDispatchable
{
public:
   SetThreadGroupPtrToNULLD(OSLThreadGroup * & pTG) :
      m_pTG(pTG)
   {}

   virtual void operator() ()
   {
      m_pTG = NULL;
      delete this;
   }

protected:
   OSLThreadGroup * & m_pTG;
};

class StopThreadGroupD : public IDispatchable
{
public:
   StopThreadGroupD(OSLThreadGroup *pTG) :
      m_pTG(pTG)
   {}

   virtual void operator() ()
   {
      m_pTG->Stop();
      delete this;
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

   virtual void operator() ()
   {
      EXPECT_EQ(m_bExpectedResult, m_pTG->Drain());
      delete this;
   }

protected:
   OSLThreadGroup *m_pTG;
   AAL::btBool     m_bExpectedResult;
};

#endif // __GTTHREADGROUP_H__

