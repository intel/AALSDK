// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifndef __GTTHREADGROUP_H__
#define __GTTHREADGROUP_H__
#include "gtCommon.h"
#include "dbg_threadgroup.h"

// Note: we don't 'delete this' in any of the operator()'s here unless explicitly named,
//       eg DelUnsafeCountUpD, because the work items are all tracked within each test
//       fixture to ensure none are lost. eg, OSLThreadGroup::Stop() removes items from
//       the work queue.

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

class WaitThenJoinThreadGroupD : public WaitD
{
public:
   WaitThenJoinThreadGroupD(CSemaphore     &ToWait,
                            OSLThreadGroup *pTG,
                            btTime          SemWaitTimeout = AAL_INFINITE_WAIT,
                            btBool          SemExpWaitRes = true,
                            btTime          TGJoinTimeout = AAL_INFINITE_WAIT,
                            btBool          TGExpectedResult = true) :
      WaitD(ToWait, SemWaitTimeout, SemExpWaitRes),
      m_pTG(pTG),
      m_Timeout(TGJoinTimeout),
      m_bExpectedResult(TGExpectedResult)
   {}
   virtual ~WaitThenJoinThreadGroupD() {}

   virtual void operator() ()
   {
      WaitD::operator() ();
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

class WaitThenDestroyThreadGroupD : public WaitD
{
public:
   WaitThenDestroyThreadGroupD(CSemaphore     &ToWait,
                               OSLThreadGroup *pTG,
                               btTime          SemWaitTimeout=AAL_INFINITE_WAIT,
                               btBool          SemExpWaitRes=true,
                               btTime          TGWaitTimeout=AAL_INFINITE_WAIT,
                               btBool          TGExpectedResult=true) :
      WaitD(ToWait, SemWaitTimeout, SemExpWaitRes),
      m_pTG(pTG),
      m_Timeout(TGWaitTimeout),
      m_bExpectedResult(TGExpectedResult)
   {}
   virtual ~WaitThenDestroyThreadGroupD() {}

   virtual void operator() ()
   {
      WaitD::operator() ();
      EXPECT_EQ(m_bExpectedResult, m_pTG->Destroy(m_Timeout));
   }

protected:
   OSLThreadGroup *m_pTG;
   AAL::btTime     m_Timeout;
   AAL::btBool     m_bExpectedResult;
};

#endif // __GTTHREADGROUP_H__

