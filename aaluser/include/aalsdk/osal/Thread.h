// Copyright(c) 2003-2016, Intel Corporation
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
/// @file Thread.h
/// @brief interface for the OSLThread class
/// @ingroup OSAL
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///
/// AUTHORS: Charlie Lasswell, Intel Corporation
///          Joseph Grecco,    Intel Corporation
///          Henry Mitchel,    Intel Corporation
///          Tim Whisonant,    Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 04/04/2008     JG       Added IsOK method to OLThread object
/// 05/08/2008     HM       Comments & License
/// 01/04/2009     HM       Updated Copyright
/// 07/15/2009     HM/JG    Added m_Joined to OSLThread to notify whether
///                            thread has been joined or not
/// 06/14/2012     HM       Added Cancel()@endverbatim
//****************************************************************************
#ifndef __AALSDK_OSAL_THREAD_H__
#define __AALSDK_OSAL_THREAD_H__
#include <aalsdk/osal/OSSemaphore.h>

#ifdef __AAL_UNKNOWN_OS__
# error TODO: Threads for unknown OS.
#endif // __AAL_UNKNOWN_OS__

/// @addtogroup OSAL
/// @{

BEGIN_NAMESPACE(AAL)

class OSLThread;
/// The thread Function signature.
typedef void (*ThreadProc)(OSLThread *pThread, void *pContext);

/// Retrieve the OS process id of the current process.
OSAL_API btPID     GetProcessID();

/// Retrieve the OS thread id of the current thread.
OSAL_API btTID      GetThreadID();

/// Compare the two OS thread id's, returning true if they identify the same thread.
OSAL_API btBool   ThreadIDEqual(btTID , btTID );

/// Cause the calling thread to exit immediately, passing ExitStatus back to the OS.
OSAL_API void ExitCurrentThread(btUIntPtr ExitStatus);

/*
/// Retrieve the number of CPUs.
///
/// @note This function is not currently implemented.
OSAL_API btInt GetNumProcessors();
*/

/// Retrieve a 32-bit random number in a thread-safe manner.
///
/// @param[in,out]  storage  Provides the seed for the generation (and the thread-specific storage).
OSAL_API btUnsigned32bitInt GetRand(btUnsigned32bitInt *storage);


//
///@brief  Object representing current thread's ID. Cast as btTID to get value.
class CurrentThreadID
{
public:
   CurrentThreadID() :
      m_tid(GetThreadID())
   {}

   btTID   getTID () { return m_tid; }
   operator btTID () { return m_tid; }
private:
   btTID m_tid;
};


/// OS Abstraction interface for Threads.
class OSAL_API OSLThread : public CriticalSection
{
// flags for m_State
#define THR_ST_OK        0x00000001
#define THR_ST_LOCAL     0x00000002
#define THR_ST_JOINED    0x00000004
#define THR_ST_DETACHED  0x00000008
#define THR_ST_CANCELED  0x00000010
#define THR_ST_UNBLOCKED 0x00000020
public:
   /// Identifies thread priority.
   enum ThreadPriority {
      THREADPRIORITY_INVALID = -1,
      THREADPRIORITY_HIGHEST = 0,  ///< Thread priority Highest.
      THREADPRIORITY_FIRST   = THREADPRIORITY_HIGHEST,
      THREADPRIORITY_HIGH,         ///< Thread priority High.
      THREADPRIORITY_ABOVE_NORMAL, ///< Thread priority Above Normal.
      THREADPRIORITY_NORMAL,       ///< Thread priority Normal.
      THREADPRIORITY_LOW,          ///< Thread priority Low.
      THREADPRIORITY_LAST    = THREADPRIORITY_LOW,
      THREADPRIORITY_COUNT
   };

   /// OSLThread Constructor.
   ///
   /// @param[in]  pProc       The thread function to be executed.
   /// @param[in]  nPriority   The thread priority. Must be one of ThreadPriority values. If not, default is normal.
   /// @param[in]  pContext    Parameter to be passed to pProc.
   /// @param[in]  ThisThread  true if pProc is to be run in the context of this thread. false if in a new thread.
	OSLThread(ThreadProc                    pProc,
	          OSLThread::ThreadPriority     nPriority,
	          void                         *pContext,
	          btBool                        ThisThread = false);
   /// OSLThread Destructor.
	virtual ~OSLThread();
	/// Internal state check.
   btBool              IsOK() { return 0 != flag_is_set(m_State, THR_ST_OK); }
   /// Send a kill signal to a thread to unblock a system call.
   void             Unblock();
   /// Post one count to the thread's local synchronization object.
   void              Signal();
   /// Wait for the thread's local synchronization object for a given time.
   void                Wait(btTime ulMilliseconds);
   /// Wait for the thread's local synchronization object.
   void                Wait();
   /// Wait for the thread to exit.
   void                Join();
   /// The underlying thread resource will never be join()'ed.
   void              Detach();
   /// The non-Windows implementation of this member function issues a pthread_cancel to the thread.
   ///
   /// @note There is currently no Windows implementation.
   void              Cancel();
   /// Compare this thread's identifier with id.
   btBool      IsThisThread(btID id) const;
   /// Retrieve this thread's identifier. Don't compare ID's outright. Use IsThisThread().
   btTID                tid();


   static const btInt sm_PriorityTranslationTable[(btInt)THREADPRIORITY_COUNT];
   static const btInt sm_DefaultPriority;

private:
#if   defined( __AAL_WINDOWS__ )
   HANDLE             m_hEvent;
   HANDLE             m_hJoinEvent;
#elif defined( __AAL_LINUX__ )
   pthread_t          m_Thread;
   CSemaphore         m_Semaphore;
#endif // OS

   btTID              m_tid;
   ThreadProc         m_pProc;
   btInt              m_nPriority;
   void              *m_pContext;
   btUnsignedInt      m_State;

#if   defined( __AAL_WINDOWS__ )
   // _beginthread() takes this signature.
   static void   StartThread(void * );
#elif defined( __AAL_LINUX__ )
   // pthread_create() takes this signature.
   static void * StartThread(void * );
#endif // OS

/*   friend OSAL_API void SetThreadPriority(OSLThread::ThreadPriority nPriority); */
};

/*
/// Set the thread priority of the current thread.
///
/// @param[in]  nPriority  Must be one of ThreadPriority values. If not, then no action is taken.
OSAL_API void SetThreadPriority(OSLThread::ThreadPriority nPriority);
*/

END_NAMESPACE(AAL)

/// @}

#endif // __AALSDK_OSAL_THREAD_H__

