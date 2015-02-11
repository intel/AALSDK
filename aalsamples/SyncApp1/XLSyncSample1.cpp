// Copyright (c) 2007-2015, Intel Corporation
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
/// @file XLSyncSample1.cpp
/// @brief Basic AFU interaction.
/// @ingroup XLSyncSample1
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer Sample Application
///
///    This application is for example purposes only.
///    It is not intended to represent a model for developing commercially-deployable applications.
///    It is designed to show working examples of the AAL programming model and APIs.
///
/// AUTHORS: Joseph Grecco, Intel Corporation.
///          Henry Mitchel, Intel Corporation.
///          Tim Whisonant, Intel Corporation.
///          Sadruta Chandrashekar, Intel Corporation.
///
/// This Sample demonstrates the following:
///    - The basic structure of an AAL program using the XL APIs.
///    - The ISampleAFUPing interface of SampleAFU1.
///    - System initialization and shutdown.
///    - Use of interface IDs (iids).
///    - Creating a AFU class that exposes a proprietary interface.
///    - Invoking a method on an AFU using a proprietary interface.
///    - Accessing object interfaces through the Interface functions.
///
/// This sample is designed to be used with SampleAFU1.
///
/// HISTORY:
/// WHEN:         WHO:     WHAT:
/// 08/29/2007    JG       Initial version started based on older sample code
/// 04/09/2008    JG       Added semaphore to protect against premature exit
/// 05/10/2008    HM       Comments & License
/// 06/24/2008    HM       Added <stdlib.h> for gcc 4.3.1 for exit()
/// 01/04/2009    HM       Updated Copyright
/// 07/22/2010    AG       SampleAFU2 now registers with AIA
/// 09/30/2011    JG       Renamed Sample 1. Ported to SDK 3.0
/// 06/18/2014    TSW      Ported to XL.
/// 09/28/2014    HM       SympleSync Conversion
/// 10/06/2014    HM       Compression into single runtime and service client@endverbatim
//****************************************************************************
#include <aalsdk/AAL.h>
#include <aalsdk/xlRuntime.h>
#include <aalsdk/utils/CSyncClient.h>
#include <aalsdk/AALLoggerExtern.h>   // Logger

using namespace AAL;

#include "SampleAFU1Service.h"        // AFU package specific definitions

// Convenience macros for printing messages and errors.
#ifndef MSG
# define MSG(x) std::cout << __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() : " << x << std::endl
#endif // MSG
#ifndef ERR
# define ERR(x) std::cerr << __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() **Error : " << x << std::endl
#endif // ERR

// Print/don't print the event ID's entered in the event handlers.
#if 1
# define EVENT_CASE(x) case x : MSG(#x);
#else
# define EVENT_CASE(x) case x :
#endif

/// @addtogroup XLSyncSample1
/// @{

///  Create a synchronizing client for a specific interface, in this case
///    ISampleAFUPingClient, that is, the Client of the Ping AFU.
///
/// Derive from CSyncClient, which provides synchronized interfaces and
///    implementations for IRuntimeClient and IServiceClient, so that you
///    can just use them.
class MyPingClient: public CSyncClient,      // Inherit interface and implementation
                    public ISampleAFUPingClient
{
public:
   /// Constructor
   MyPingClient()
   { // Publish our interface
      SetInterface(iidSampleAFUPingClient,
            dynamic_cast<ISampleAFUPingClient *>(this));
   }
   /// Destructor with empty implementation
   ~MyPingClient(){}

   ///////////////////////////////////////////////////////////////////////////
   // <begin ISampleAFUPingClient interface>

   /// Implement ISampleAFUPingClient::PingReceived
   void PingReceived(TransactionID const &rTranID)
   {
      Post();
   }
   // <end ISampleAFUPingClient interface>
   ///////////////////////////////////////////////////////////////////////////

   ///////////////////////////////////////////////////////////////////////////
   // ADDITIONAL PING CLIENT FUNCTIONS

   /// Synchronizing function for ISampleAFUPingClient::PingReceived
   void WaitForPing(void)
   {
      Wait();
   }
   ///////////////////////////////////////////////////////////////////////////
};

/// @} group XLSyncSample1


//=============================================================================
// Name: main
// Description: Entry point to the application
// Inputs: none
// Outputs: none
// Comments: Main initializes the system. The rest of the example is implemented
//           in the objects.
//=============================================================================
int main(int argc, char *argv[])
{
   bool                    bRuntimeGood;     // Tracks if the runtime is initialized
   MyPingClient            pingClient;       // Specific Ping Client Object. Note that
                                             //    it IS_A CSyncClient Object, as well.
   IBase                  *pServiceBase;     // Pointer to Service containing Ping
   ISampleAFUPing         *pPingAFU;         // Specific Pointer to Ping Service

   cout << "=====================================" << endl;
   cout << " AAL Synchronous Sample 1 Application" << endl;
   cout << "=====================================" << endl << endl;

   ////////////////////////////////////////////////////////////////////////////
   // Start the Runtime Client, which in turn will start the RunTime itself.
   //    Tell the runtime to use the default service broker. The broker will be
   //       different for a hardware- vs. software-based service. This is software based.

   MSG("Starting Runtime");

   NamedValueSet configArgs, configRecord;
   configRecord.Add(XLRUNTIME_CONFIG_BROKER_SERVICE, "libsamplebroker");
   configArgs.Add(XLRUNTIME_CONFIG_RECORD, configRecord);

   bRuntimeGood = pingClient.syncStart( configArgs );
   if (!bRuntimeGood) {
      ERR("Runtime failed to start.");
      pingClient.syncStop();
      return 1;
   }

   ////////////////////////////////////////////////////////////////////////////
   // Get the Service
   // NOTE: This example is bypassing the Resource Manager's configuration record lookup
   //  mechanism.  This code is work around code and subject to change.

   MSG("Allocating Sample AFU 1 (a.k.a. Ping)");

   NamedValueSet Manifest( strConfigRecord );
   Manifest.Add( AAL_FACTORY_CREATE_SERVICENAME, "AFU 1" );

#if DBG_HOOK
   MSG(Manifest);
#endif // DBG_HOOK

   pServiceBase = pingClient.syncAllocService( Manifest );
   if ( !pServiceBase ) {
      ERR("Could not allocate AFU 1 (a.k.a Ping) Service.");
      pingClient.syncStop();
      return 2;
   }

   ////////////////////////////////////////////////////////////////////////////
   // The IBase pointer to the Service object contains by aggregation all the
   //    sub-pointers that we need.
   // In this case we want the pointer to the Ping Service.

   pPingAFU = dynamic_ptr<ISampleAFUPing>( iidSampleAFUPing, pServiceBase);
   if ( !pPingAFU ) {      // this would represent an internal logic error
      ERR("Ping Service pointer does not exist.");
      pingClient.syncStop();
      return 3;
   }

   ////////////////////////////////////////////////////////////////////////////
   // Use the Ping Service
   //   1) Invoke the Ping Service through its pointer
   //   2) See that it calls back 5 times

   pPingAFU->Ping("Hello World", TransactionID()); // This function now generates 5 event responses.

   for (int i = 1; i <= 5; ++i) {
      pingClient.WaitForPing();
      MSG("got Ping Received " << i);
   }

   ////////////////////////////////////////////////////////////////////////////
   // Shut everything down in reverse order

   // Tell Service to shut down, returns to serviceClient when finished
   pingClient.syncRelease(TransactionID());
   MSG("Sample AFU 1 Freed");

   // Tell Runtime to shut down, returns to runtime client
   pingClient.syncStop();
   MSG("Runtime stopped");

   // All done
   MSG("Done");
   return 0;
}

/**
@addtogroup XLSyncSample1
@{

@verbatim
This application is for example purposes only.
It is not intended to represent a model for developing commercially-deployable applications.
It is designed to show working examples of the AAL programming model and APIs.@endverbatim

This Sample demonstrates the following:

<ul>
  <li>The basic structure of an AAL program using the serialized callback model.</li>
  <li>The ISampleAFUPing interface of SampleAFU1.</li>
  <li>Use of the CSyncClient super-class to inherit implementation of system
      initialization, shutdown, and access to ISampleAFUPing.</li>
  <li>Invoking a method on an AFU using a proprietary interface.</li>
</ul>

This sample is designed to be used with AFU1/SampleAFU1Service, a.k.a ISampleAFUPing.

1 Summary of Operation

XLSyncSample1 relies on its instantiation of CSyncClient inherited by the MyPingClient
object to perform the brunt of the XL runtime interaction. CSyncClient instantiates
a instance of the XL Runtime object, and provides default operations for the
IRuntime and IServiceClient interfaces. MyPingClient inherits this implementation
and extends it to add ISampleAFUPingClient and synchronous versions of the
ISampleAFUPing interfaces.

The MyPingClient object declared in main() handles all of these functions, hopefully leaving
the bulk of the interesting processing to be exposed cleanly in main().

There are no command line arguments.

MyPingClient is instantiated and then its method syncStart() called. This synchronously
starts the Runtime. When it returns the return value indicates whether or not the
Runtime started okay.

Then MyPingClient's syncAllocate() is called to find and instantiate a Ping Service. It returns
the pointer to the Ping Service object from which the pointer to the actual ISampleAFUPing
interface is cast. This operation is shown in this program, but in general can be hidden
as in other samples.

Once the ISampleAFUPing interface is available, it can be called directly, and is by
pPingAFU->Ping(). The Ping Service then called back to the pingClient 5 times and the program
is unwound by Releasing the Service and stopping the Runtime and exiting.

2 Running the application

@code
$ XLSyncSample1
@endcode

@} group XLSyncSample1
*/

