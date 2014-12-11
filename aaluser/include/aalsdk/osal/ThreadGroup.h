// Copyright (c) 2003-2014, Intel Corporation
//
// Redistribution  and  use  in source  and  binary  forms,  with  or  without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of  source code  must retain the  above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name  of Intel Corporation  nor the names of its contributors
//   may be used to  endorse or promote  products derived  from this  software
//   without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  BUT NOT LIMITED TO,  THE
// IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT  SHALL THE COPYRIGHT OWNER  OR CONTRIBUTORS BE
// LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR
// CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT LIMITED  TO,  PROCUREMENT  OF
// SUBSTITUTE GOODS OR SERVICES;  LOSS OF USE,  DATA, OR PROFITS;  OR BUSINESS
// INTERRUPTION)  HOWEVER CAUSED  AND ON ANY THEORY  OF LIABILITY,  WHETHER IN
// CONTRACT,  STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE  OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,  EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//****************************************************************************
/// @file ThreadGroup.h
/// @brief Interface for the ThreadGroup class
/// @ingroup OSAL
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Joseph Grecco, Intel Corporation
///          Henry Mitchel, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 05/08/2008     HM       Comments & License
/// 01/04/2009     HM       Updated Copyright@endverbatim
/// 03/06/2014     JG       Complete rewrite.
//****************************************************************************
#ifndef __AALSDK_OSAL_THREADGROUP_H__
#define __AALSDK_OSAL_THREADGROUP_H__
#include <aalsdk/osal/OSSemaphore.h>
#include <aalsdk/osal/Thread.h>
#include <aalsdk/osal/IDispatchable.h>


//=============================================================================
// Name: OSLThreadGroup
// Description: ThreadGroup (a.k.a. thread pooled) functor scheduler.
// Interface: public
// Comments: 
//=============================================================================
class OSAL_API OSLThreadGroup : private CriticalSection
{
   enum eState {
      Running = 0,
      Stopped,
      Draining,
      Destruct
   };

public:
   // Default Min thread is to let the TG decide. Max < min then Max == Min
   OSLThreadGroup(AAL::btUnsignedInt uiMinThreads=0,
                  AAL::btUnsignedInt uiMaxThreads=0,
                  OSLThread::ThreadPriority nPriority=OSLThread::THREADPRIORITY_NORMAL);

   virtual ~OSLThreadGroup();

   AAL::btBool    Add(IDispatchable *);

   void           Stop();
   AAL::btBool    Start();
   AAL::btBool    Drain();
   AAL::btBool    IsCurThreadInGroup();
   AAL::btInt     GetNumThreads() const;
   AAL::btInt     GetNumWorkItems() const;

private:
   typedef std::vector<OSLThread    *> VeOSLThreads_t;
   typedef std::queue<IDispatchable *> work_queue_t;
   //
   // Object that holds state and semiphores for the
   //  thread group. This object "lives" outside the
   //  ThreadGroup object so that Threads may continue to
   //  clean-up even if teh ThreadGroup proper has been destroyed.
   struct ThrGrpState
   {
      // Object is initialized with the number of threads in the
      //  group as a reference count.  When threads are deleted as
      //  part of the destruction mechanism they decrement the ref count
      //  (See DeleteThr).  When the count goes to zero this object
      //  destroys itself
      ThrGrpState(AAL::btInt NumThreads) :
         m_eState(Running),
         m_WaitforJoin(true)
      {
         m_WorkSem.Create(0, INT_MAX);
         m_DrainSem.Create(0, INT_MAX);
         m_JoinSem.Create(0, INT_MAX);
         // Use count up to wait for threads to start
         m_StartupSem.Create(-NumThreads, INT_MAX);
      }


      ~ThrGrpState(){
         // Make sure we are out of
         //  the DeleteThr()
         m_CritSect.Lock();

      }

      void DeleteThr(OSLThread *p)
      {
         AAL::btBool del = false;

         m_CritSect.Lock();

         VeOSLThreads_t::iterator iter = find(m_Threads.begin(), m_Threads.end(), p);
         if ( m_Threads.end() != iter ) {
            m_Threads.erase(iter);
            if ( 0 == m_Threads.size() ) {
               del = true;
            }
         }

         // If someone is waiting
         if( m_WaitforJoin ){
            m_JoinSem.Post(1);
         }

         // If no one is going to wait for
         //  thread death and we are done
         if ( del && !m_WaitforJoin ) {
            m_CritSect.Unlock();
            delete this;
            return;
         }
         m_CritSect.Unlock();
      }

      // Typically we want to wait for all threads to die
      //  in destructor BUT if we can't because the caller
      //  is in one of the Groups threads we need to auto delete
      void DontJoinThreads() {m_WaitforJoin = false;}

      AAL::btBool JoinAllThreads(AAL::btTime Timeout){
         if(!m_WaitforJoin){
            return false;
         }
         return m_JoinSem.Wait();
      }

      enum eState     m_eState;
      AAL::btBool     m_WaitforJoin;
      CSemaphore      m_WorkSem;
      CSemaphore      m_DrainSem;
      CSemaphore      m_JoinSem;
      CSemaphore      m_StartupSem;
      CriticalSection m_CritSect;
#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4251)
#endif // _MSC_VER
      work_queue_t    m_workqueue;
      VeOSLThreads_t  m_Threads;
#ifdef _MSC_VER
# pragma warning(pop)
#endif // _MSC_VER
   };

   static void ExecProc(OSLThread *pThread, void *lpParms);

   AAL::btInt                m_nNumThreads;
   AAL::btInt                m_nMinThreads;
   AAL::btInt                m_nMaxThreads;
   OSLThread::ThreadPriority m_ThreadPriority;
   ThrGrpState              *m_pState;

   static eState GetWorkItem(ThrGrpState * , IDispatchable * & );
};

#endif // __AALSDK_OSAL_THREADGROUP_H__

