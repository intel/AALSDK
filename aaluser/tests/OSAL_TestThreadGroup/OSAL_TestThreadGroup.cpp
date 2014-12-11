// Copyright (c) 2014, Intel Corporation
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
/// file OSAL_TestSem.cpp
/// brief Unit Test for OSAL Semaphores
/// ingroup OSAL_TestSem
/// verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer Test Application
///
///
/// AUTHORS: Joseph Grecco, Intel Corporation
///
/// This tests the various capabilities of OSAL ThreadGroup
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 08/20/2013     JG       Initial version started endverbatim
//****************************************************************************
#include <stdlib.h>                    // for exit()
#include <iostream>
#include <fstream>

#ifdef __linux__
#include <limits.h>
#endif

#include "aalsdk/osal/ThreadGroup.h"
#include "aalsdk/osal/OSSemaphore.h"
#include "aalsdk/osal/Sleep.h"
#include "aalsdk/osal/IDispatchable.h"
#include "aalsdk/osal/CriticalSection.h"

USING_NAMESPACE(std)
USING_NAMESPACE(AAL)


CSemaphore           TestSem;

// Functors
class FireAndForget: public IDispatchable
{
public:
   FireAndForget(CSemaphore *psem) :m_psem(psem), m_tg(){}
   
   void Fire(){m_tg.Add(this);}

   void  operator()()
   {
      cerr << "Fired! Sleeping 2 seconds" << endl;
      SleepSec(2);      
      m_psem->Post(1);

      delete this;
   };

protected:
  CSemaphore*     m_psem;
  OSLThreadGroup  m_tg;
};

class Numbered: public IDispatchable
{
public:
   Numbered(CSemaphore *psem, int x) : m_psem(psem), m_x(x){}
   
   void  operator()()
   {
      cerr << "Functor Number " << m_x << endl;
      m_psem->Post(1);
      delete this;
   };

protected:
  CSemaphore*     m_psem;
  int             m_x;
};




//=============================================================================
// Prototypes
//=============================================================================

#if 1
#define EVENT_CASE(x) case x : cerr << __AAL_SHORT_FILE__ << " : " << __AAL_FUNC__ << "() " #x << endl;
#else
#define EVENT_CASE(x) case x :
#endif

// Consumer thread proceedure
static void Test1Thread(OSLThread *pThread, void *pContext);
static void Test3Thread(OSLThread *pThread, void *pContext);

//=============================================================================
// Name: main
// Description: Entry point to the application
// Inputs: Argv - Optional file name
// Outputs: Encoded file sent to standard output
// Comments:
//=============================================================================
int main(int argc, char* argv[])
{
   cerr << "============================" << endl;
   cerr << " OSAL ThreadGroup Unit Test"         << endl;
   cerr << "============================" << endl << endl;


   //=============================================================================================
   // Test 1 tests basic semaphore
   //=============================================================================================
   {
     cerr << "Testing FireAndForget.\n";
	  if( false == TestSem.Create(0,INT_MAX) ){
		  cerr << "failed to create semaphore (0,INT_MAX)\n";
		  exit(1);
	  }
 
     (new FireAndForget(&TestSem))->Fire();


      // Wait forever
      cerr << "Waiting\n";
      if(false == TestSem.Wait()){
		  cerr << "failed to wait forever on semaphore";
		  exit(1);

      }
 
      cerr <<"Passed!"<<endl;
      
   } // End Test 1

   {
      cerr << "Testing queuing  work on running single threaded group.\n";
      OSLThreadGroup tg;
      TestSem.Reset(-(10));
      for(int x=1; x<= 10; x++){
         tg.Add(new Numbered(&TestSem, x));
      }

      if(false == TestSem.Wait(10000)){
		  cerr << "failed to wait forever on semaphore";
		  exit(1);
      }
   }

   cerr << "Unit Test Complete - Success\n";
   exit(0);
}

