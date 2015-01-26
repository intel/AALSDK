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
/// This tests teh various capabilities of OSAL sempaphores
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

#include "aalsdk/osal/Thread.h"
#include "aalsdk/osal/Sleep.h"

USING_NAMESPACE(std)
USING_NAMESPACE(AAL)

//--------------------------------------------------------------------------------------------
// System context - The system context is an object which holds the consumer and producer
//                  Queues, synchronization objects and cached copies of the various AFU
//                  interfaces.  This object is accessed by multiple threads simultaneously
//                  and must be thread safe.
//--------------------------------------------------------------------------------------------
// AppContext sysContext;

CSemaphore           TestSem;
int                  Countup;

//=============================================================================
// Prototypes
//=============================================================================

#if 1
#define EVENT_CASE(x) case x : cerr << __AAL_SHORT_FILE__ << " : " << __AAL_FUNC__ << "() " #x << endl;
#else
#define EVENT_CASE(x) case x :
#endif

// Thread proceedures
static void Test1Thread(OSLThread *pThread, void *pContext);
static void Test3Thread(OSLThread *pThread, void *pContext);
static void Test5Thread(OSLThread *pThread, void *pContext);

//=============================================================================
// Name: main
// Description: Entry point to the application
// Inputs: Argv - Optional file name
// Outputs: Encoded file sent to standard output
// Comments:
//=============================================================================
int main(int argc, char* argv[])
{
   cerr << "=========================" << endl;
   cerr << " OSAL Semaphore Unit Test"         << endl;
   cerr << "=========================" << endl << endl;


   //=============================================================================================
   // Test 1 tests basic semaphore
   //=============================================================================================
   {
     cerr << "Testing basic functionality. Create(), Destroy(), infinite wait.\n";
	  if( false == TestSem.Create(0,INT_MAX) ){
		  cerr << "failed to create semaphore (0,INT_MAX)\n";
		  exit(1);
	  }
      // Object pointer stored in system context in the thread itself (See ConsumerThread() )
      OSLThread *t = new OSLThread( Test1Thread,
                                    OSLThread::THREADPRIORITY_NORMAL,
                                    NULL);
      // Wait forever
      if(false == TestSem.Wait()){
		  cerr << "failed to wait forever on semaphore";
		  exit(1);

      }
      t->Join();
      delete t;

      if( false == TestSem.Destroy() ){
		  cerr << "failed to destroy semaphore\n";
		  exit(1);
	   }
      cerr <<"Passed!"<<endl;
      
   } // End Test 1



   //=============================================================================================
   // Test 2 tests count down semaphore
   //=============================================================================================
   {
     cerr << "Testing recreate and set to 3\n";
     if( false == TestSem.Create(3,INT_MAX) ){
		  cerr << "failed to create semaphore (0,INT_MAX)\n";
		  exit(1);
	  }

      
      int x=0;

      for(; x < 3; x++){
         // Wait 1 second
         if(false == TestSem.Wait(1000)){
		         cerr << "failed on " << x+1 << " wait" << endl;
		         exit(1);
         }else{
            cerr << "Wait number " << x+1 << " of 3" <<  endl;
         }
         
      }
      cerr << "Passed!" <<endl;
      // Wait 1 second
      cerr << "Testing waiting with timeout and no signal\n";
      if(false != TestSem.Wait(1000)){
		   cerr << "failed 4th wait of 3";
         exit(1);
      }
      
      if( false == TestSem.Destroy() ){
		  cerr << "failed to destroy semaphore\n";
		  exit(1);
	   }
      cerr << "Passed!" <<endl;

   } // End Test 2

   //=============================================================================================
   // Test 3 tests count up semaphore
   //=============================================================================================
   {
     cerr << "Testing count up semaphore to 3\n";
     Countup = 3;
	  if( false == TestSem.Create(-3,INT_MAX) ){
		  cerr << "failed to create semaphore (0,INT_MAX)\n";
		  exit(1);
	  }

      // Object pointer stored in system context in the thread itself (See ConsumerThread() )
      OSLThread *t = new OSLThread( Test3Thread,
                                    OSLThread::THREADPRIORITY_NORMAL,
                                    NULL);


      // Wait 1 second
      if(false == TestSem.Wait(10000)){
		   cerr << "Failed Count up";
         exit(1);
      }

      t->Join();
      delete t;

      if( false == TestSem.Destroy() ){
		  cerr << "failed to destroy semaphore\n";
		  exit(1);
	   }

      cerr << "Passed!" <<endl;

   } // End Test 3


   //=============================================================================================
   // Test 4 tests reset semaphore
   //=============================================================================================
   {
      cerr << "Testing reset and counts on semaphore\n";
      if( false == TestSem.Create(3,INT_MAX) ){
		  cerr << "failed to create semaphore (0,INT_MAX)\n";
		  exit(1);
	   }


      btInt currCount, maxCount;

      TestSem.CurrCounts(currCount,maxCount);
      cerr << "Current count is " << currCount << " max count is " << maxCount <<endl;

      cerr << "Waiting once" << endl;
      if( false == TestSem.Wait(1000)){
         cerr << "Wait Timed out" <<endl;
         cerr << "Failed" << endl;
         exit(1);
      } 
      cerr << "Passed" << endl;

      TestSem.CurrCounts(currCount,maxCount);
      cerr << "Current count is " << currCount  <<endl;
      if( 2 != currCount){
         cerr << "Current not expected value!" <<endl;
         cerr << "Failed" << endl;
         exit(1);
      }
      cerr << "Passed" << endl;

      cerr << "resetting to -5" <<endl;
      TestSem.Reset(-(5));

      TestSem.CurrCounts(currCount,maxCount);
      cerr << "Current count is " << currCount << " which should be 1 more than reset value to count up to signal state"<<endl;
      if(currCount != -4){
         cerr << "Count not as expected!" << endl;
         cerr << "Failed" << endl;
      }
      cerr << "resetting to 5" <<endl;
      TestSem.Reset(5);

      TestSem.CurrCounts(currCount,maxCount);
      cerr << "Current count is " << currCount  <<endl;
      if(currCount != 5){
         cerr << "Count not as expected!" << endl;
         cerr << "Failed" << endl;
      }

      cerr << "Waiting 5 times with timeout" << endl;
      int x=1;
      for(; x <= 5; x++){

         // Wait 1 second
         if(false == TestSem.Wait(1000)){
            cerr << "Failed on Wait number " << x << " of 5" <<  endl;
            exit(1);
         }else{
            cerr << "Wait number " << x << " of 5" <<  endl;
            TestSem.CurrCounts(currCount,maxCount);
            cerr << "Current count is " << currCount <<endl;
            if(currCount != (5-x)){
               cerr << "Current not expected value!" <<endl;
               cerr << "Failed" << endl;
               exit(1);
            }
         }
      }

      cerr << "Checking Count up with reset" << endl;
      cerr << "resetting to -5" <<endl;
      TestSem.Reset(-(5));

      TestSem.CurrCounts(currCount,maxCount);
      cerr << "Current count is " << currCount << " which should be 1 more than reset value to count up to signal state" <<endl;
      if(currCount != -4){
         cerr << "Count not as expected!" << endl;
         cerr << "Failed" << endl;
      }

      cerr << "Waiting 5 times with timeout" << endl;
      x=1;
      for(; x <= 5; x++){

         // Wait 1 second
         if(false == TestSem.Post(1)){
            cerr << "Failed on Post number " << x+1 << " of 5" <<  endl;
            exit(1);
         }else{
            cerr << "Post number " << x << " of 5" <<  endl;
            TestSem.CurrCounts(currCount,maxCount);
            cerr << "Current count is " << currCount  <<endl;
            if(currCount != -(5-(x+1))){
               cerr << "Current not expected value!" <<endl;
               cerr << "Failed" << endl;
               exit(1);
            }
         }
      }
      cerr << "Semaphore now in signaled state as expected." << endl;
      cerr << "Passed" << endl;
   } //Test 4
	
   
   if( false == TestSem.Destroy() ){
      cerr << "Failed to destroy" << endl;
   }
   
   cerr << "Unit Test Complete - Success\n";
   exit(0);
}

//=============================================================================
// Name: Test1Thread
// Description:
// Inputs:  pThread - Thread object pointer
//          pContext - Pointer to the context  object
// Outputs: none.
// Comments:
//=============================================================================
void Test1Thread( OSLThread *pThread, void *pContext )

{
	SleepSec(1);

    //Wake up main thread
	TestSem.Post(1);
    return;
}


//=============================================================================
// Name: Test3Thread
// Description:
// Inputs:  pThread - Thread object pointer
//          pContext - Pointer to the context  object
// Outputs: none.
// Comments:
//=============================================================================
void Test3Thread( OSLThread *pThread, void *pContext )

{
	
   //Wake up main thread
   Countup--;
	TestSem.Post(1);
   cerr << "Post 1\n";
  	
   //Wake up main thread
	TestSem.Post(1);
   Countup--;
   cerr << "Post 2\n";
  	
   //Wake up main thread
	TestSem.Post(1);
   Countup--;
   cerr << "Post 3\n";

   return;
}

//=============================================================================
// Name: Test5Thread
// Description:
// Inputs:  pThread - Thread object pointer
//          pContext - Pointer to the context  object
// Outputs: none.
// Comments:
//=============================================================================
void Test5Thread( OSLThread *pThread, void *pContext )

{
	SleepSec(1);

    //Destroy

    return;
}
