// Copyright(c) 2007-2016, Intel Corporation
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
/// @file main.cpp
/// @brief Basic AFU interaction.
/// @ingroup Power Manger
/// @verbatim
/// Accelerator Abstraction Layer Sample Application
///
///    This application is for example purposes only.
///    It is not intended to represent a model for developing commercially-deployable applications.
///    It is designed to show working examples of the AAL programming model and APIs.
///
/// AUTHORS: Ananda  Ravuri, Intel Corporation.
///

///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 07/25/2016     AR       Initial version started based on older sample code.@endverbatim
//****************************************************************************
/// @}
//=============================================================================
// Name: main
// Description: Entry point to the application
// Inputs: none
// Outputs: none
// Comments: Main initializes the system. The rest of the example is implemented
//           in the objects.
//=============================================================================

#include <signal.h>                        // sigaction
#include <errno.h>                         // EINTR

#include "PwrMgrApp.h"

#define MAX_NUM_SOCKETS  2

PwrMgrRuntime * g_PwrMgrRuntime                   = NULL;
PwrMgrClient  * g_PwrMgrClient[MAX_NUM_SOCKETS]  = {0};



void Release_PwrMgrservice()
{
   for(int i=0;i< MAX_NUM_SOCKETS;i++) {

      if( NULL != g_PwrMgrClient[i]) {
            g_PwrMgrClient[i]->FreeService();
           delete g_PwrMgrClient[i];
           g_PwrMgrClient[i] = NULL;
        }

   }

   if( NULL != g_PwrMgrRuntime) {
      g_PwrMgrRuntime->runtimeStop();
      delete g_PwrMgrRuntime;
      g_PwrMgrRuntime = NULL;
   }
}


void signal_handler(int signo, siginfo_t *info, void *ctx)
{
   ERR("  Power Manger APP signal_handler Enter ");

   Release_PwrMgrservice();

   ERR("  Power Manger APP signal_handler Exit ");
   exit(0);
}


//=============================================================================
// Name: main
// Description: Entry point to the application / demon
// Inputs: none
// Outputs: none
// Comments: Main initializes the system. The rest of the example is implemented
//           in the object theApp.
//=============================================================================
int main(int argc, char *argv[])
{

   int                 result  = 0;
   /*
    * set up signal handler
    */
   struct sigaction sa;
   sigfillset(&sa.sa_mask);               // Don't mask any signals
   sa.sa_sigaction = signal_handler;      // Our handler
   sa.sa_flags = SA_SIGINFO | SA_RESTART; // Don't fail interrupted call

   // Replace these signal handlers
   sigaction(SIGINT, &sa, NULL);
   sigaction(SIGHUP, &sa, NULL);
   sigaction(SIGTERM, &sa, NULL);

   if(getuid()) {
       ERR(" export LD_LIBRARY_PATH in root and Run Power Manager with root privialge ");
       exit(1);
   }

   g_PwrMgrRuntime = new (nothrow) PwrMgrRuntime();

   if( NULL == g_PwrMgrRuntime ){
      ERR(" Power Manger Failed to  memory for AAL Runtime");
      exit(1);
   }

   // Runtime started
   if(g_PwrMgrRuntime->IsOK()){

      for(int i=0;i< MAX_NUM_SOCKETS;i++) {

         // Allocate Power manager service
         g_PwrMgrClient[i] = new (nothrow) PwrMgrClient(g_PwrMgrRuntime->getRuntime());
         if( NULL != g_PwrMgrClient[i] ) {

            // Allocate power manager service
            g_PwrMgrClient[i]->AllocateService();
            if(!g_PwrMgrClient[i]->IsOK()) {
               ERR(" App Failed to allocate Power Mgr Service");
               delete g_PwrMgrClient[i];
               g_PwrMgrClient[i] = NULL;

            }else {
               std::cout << "AlLocated Power Manager Service for Socket:" << i<<  std::endl;
            }

         } else {
            ERR("Power Manger Failed to  memory for AAL Power Manger service");
         }

      } // end of for

   } else {
      ERR(" Power Manger Failed to Allocate AAL Runtime");
      goto ERROR;
   } // end of if


   MSG("Power manger Demon Starts");

   //  TBD
   // Waits for signal form driver
   while(true)
   {

   }

   MSG("Power manger Demon Ends");
   Release_PwrMgrservice();

   return result ;

ERROR:

   Release_PwrMgrservice();

   exit(1);
}


