// Copyright (c) 2007-2014, Intel Corporation
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
/// @file XLSample2.cpp
/// @brief Streaming AFU Producer/Consumer.
/// @ingroup XLSample2
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer Sample Application
///
///    This application is for example purposes only.
///    It is not intended to represent a model for developing commercially-deployable applications.
///    It is designed to show working examples of the AAL programming model and APIs.
///
/// AUTHORS: Joseph Grecco, Intel Corporation
///          Henry Mitchel, Intel Corporation
///          Tim Whisonant, Intel Corporation
///
/// This Sample demonstrates a Producer/Consumer data streaming model. The
/// application represents using a data transform AFU.
/// Data is pulled from standard input or a file, pushed through the transform
/// AFU where it is encoded and streamed back to the consumer portion of the
/// application where it is sent to standard output.
///
/// In addition to demonstrating the features of SampleAFU2, it demonstrates the following:
///    - Implementing a multi-threaded AALSDK application.
///    - Using a system-level context for synchronization.
///    - An AFU that supports multiple Interfaces simultaneously.
///    - Custom events.
///    - Principles of creating blocking wrapper functions around AAL.
///    - Multiple Event Handlers.
///    - Advanced use of TransactionIDs for custom callbacks.
///    - A data streaming design pattern with AFU defined buffers.
///
/// This sample is designed to be used with SampleAFU2.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 11/16/2007     JG       Initial version started based on AALSample2
/// 02/03/2008     HM       Cleaned up various things brought out by ICC
/// 05/10/2008     HM       Comments & License
/// 06/15/2008     HM       Changed dynamic_cast<> to subclass_ptr<> in
///                           tranevtSystemInit handler
/// 06/24/2008     HM       Added <stdlib.h> for gcc 4.3.1 for exit()
/// 01/04/2009     HM       Updated Copyright
/// 10/06/2011     JG       Ported to SDK 3.0
/// 06/24/2014     TSW      Ported to SDK 4.0@endverbatim
//****************************************************************************
#include "XLSample2.h"

#if __AAL_WINDOWS__
# include <io.h>
# include <fcntl.h>
#endif // __AAL_WINDOWS__

#include <fstream>

// Input file stream
fstream  ifile;

istream * GetInputStream(int argc, char *argv[])
{
   istream *pIStream = &cin; // Default to cin

   // Get the optional file-based input source
   if ( argc > 1 ) {
      ifile.open(argv[1], fstream::in|fstream::binary);
      pIStream = &ifile;
   }
#if __AAL_WINDOWS__
   else if ( -1 == _setmode(_fileno(stdin), _O_BINARY) ) {
      return NULL;
   }

   if ( -1 == _setmode(_fileno(stdout), _O_BINARY) ) {
      return NULL;
   }
#endif // __AAL_WINDOWS__

   return pIStream;
}

// After attaching, set gWaitForDebuggerAttach to 0 via the debugger to unblock the app.
#if DBG_HOOK
btBool gWaitForDebuggerAttach = true;
#endif // DBG_HOOK

//--------------------------------------------------------------------------------------------
// System context - The system context is an object which holds the consumer and producer
//                  Queues, synchronization objects and cached copies of the various AFU
//                  interfaces.  This object is accessed by multiple threads simultaneously
//                  and must be thread safe.
//--------------------------------------------------------------------------------------------
AppContext TheAppContext;

//=============================================================================
// Prototypes
//=============================================================================

// Consumer thread proceedure
static void ConsumerThread(OSLThread *pThread, void *pContext);

//=============================================================================
// Name: main
// Description: Entry point to the application
// Inputs: Argv - Optional file name
// Outputs: Encoded file sent to standard output
// Comments:
//=============================================================================
int main(int argc, char* argv[])
{
   cerr << "=================================" << endl;
   cerr << "    AAL Sample 2 Application"      << endl;
   cerr << " Streaming AFU Producer/Consumer"  << endl;
   cerr << "=================================" << endl << endl;

#if DBG_HOOK
   MSG("Waiting for debugger attach..");
   while ( gWaitForDebuggerAttach ) {
      SleepSec(1);
   }
   // Init the AAL logger.
   pAALLogger()->AddToMask(LM_All, 8); // All subsystems
   pAALLogger()->SetDestination(ILogger::CERR);
#endif // DBG_HOOK

   istream *pIStream = GetInputStream(argc, argv);

   // sanity check our pIStream
   if ( NULL == pIStream ) {
      ERR("pIStream is NULL.");
      return 1;
   }

   AAL::XL::RT::Runtime runtime;
   NamedValueSet        runtimeargs;

   runtime.start(&TheAppContext, runtimeargs);

   TheAppContext.Wait();

   if ( !TheAppContext.IsOK() ) {
      return 2;
   }

   //===================================================================
   // Create the AFU and get the wrapped Producer and Consumer objects.
   //===================================================================
   NamedValueSet Manifest(strConfigRecord);

   Manifest.Add(AAL_FACTORY_CREATE_SERVICENAME, "SampleAFU2");

#if DBG_HOOK
   cout << Manifest << endl;
#endif // DBG_HOOK

   MSG("Allocating Sample AFU 2");

   TheAppContext.Runtime()->allocService(&TheAppContext, Manifest);

   TheAppContext.Wait(); // For Service allocation.

   if ( ( NULL == TheAppContext.ProducerAFU() ) ||
        ( NULL == TheAppContext.ConsumerAFU() ) ) {
      TheAppContext.IsOK(false);
      ERR("Service allocation failed.");
      return 3;
   }

   //=============================================================================================
   // Start the consumer thread
   //=============================================================================================

   // Object pointer stored in system context in the thread itself (See ConsumerThread())
   new OSLThread(ConsumerThread,
                 OSLThread::THREADPRIORITY_NORMAL,
                 &TheAppContext);
   // Wait for the Consumer to be ready
   TheAppContext.Wait();


   //=============================================================================================
   // Start the Producer
   //=============================================================================================
   TransactionID tid;

   //--------------------------------
   // Start the transform
   //--------------------------------
   TheAppContext.StreamState(Streaming);
   TheAppContext.ConsumerAFU()->Start(tid, 0x4d);
   TheAppContext.Wait();

   //---------------------------------------------------------------------
   // Application Producer loop waits for empty buffers to be available
   // from the AFU (See ProducerEventHandler) fills them from
   // the input source and send them to the AFU for transformation
   //---------------------------------------------------------------------
   while ( TheAppContext.IsOK() ) {

      WSBufDesc pBuffer = TheAppContext.ConsumerQ().GetNext();

      pIStream->read((char *)pBuffer.pBuf, pBuffer.size);

      // If we hit eof
      if ( pIStream->eof() ) {
         // Out the final buffer
         pBuffer.size = pIStream->gcount();
         TheAppContext.ConsumerAFU()->PutBuffer(pBuffer);

         // Send the EOD which will cause the Consumer
         // Thread to finish up and free remaining buffers
         if ( pBuffer.size != 0 ) {
            pBuffer = TheAppContext.ConsumerQ().GetNext();
            pBuffer.size = 0;
            TheAppContext.ConsumerAFU()->PutBuffer(pBuffer);
         }

         // Wait for consumer thread to terminate
         TheAppContext.ConsumerThread()->Join();
         delete TheAppContext.ConsumerThread();

         TheAppContext.Lock();
         // Signal producer's buffer recieve handler to stop
         TheAppContext.StreamState(Stopping);

         // Stop the transform
         TheAppContext.ConsumerAFU()->Stop(tid);
         TheAppContext.Unlock();

         // Drain the empty queue the outstanding buffers
         while ( TheAppContext.ConsumerQ().Size() ) {
            pBuffer = TheAppContext.ConsumerQ().GetNext();
            // Return the buffer
            TheAppContext.ConsumerAFU()->PutBuffer(pBuffer);
         }

         // Wait for operation complete
         TheAppContext.Wait();
         break;
      }

      // Send the buffer to be transformed
      TheAppContext.ConsumerAFU()->PutBuffer(pBuffer);
   }

   //=============================================================================================
   // Destroy the AFU
   //=============================================================================================
   TheAppContext.Service()->Release(TransactionID());
   TheAppContext.Wait(); // For the AFU freed notification.

   //=============================================================================================
   // Stop the system
   //=============================================================================================
   TheAppContext.Runtime()->stop();
   TheAppContext.Wait(); // For Runtime stopped notification.

   MSG("Goodbye");
   return 0;
}

//=============================================================================
// Name: ConsumerThread
// Description: This thread consumes data from the AFU and writes to stdout
// Inputs:  pThread - Thread object pointer
//          pContext - Pointer to the context  object
// Outputs: none.
// Comments:
//=============================================================================
void ConsumerThread(OSLThread *pThread, void *pContext)
{
   AppContext *pAppContext = static_cast<AppContext *>(pContext);

   // set my pointer in the context
   pAppContext->ConsumerThread(pThread);

   TransactionID tid;

   //=============================================================================================
   // Start the Consumer
   //=============================================================================================

   //This may look odd.  The first tid is the argument to Register tid and is used
   // to specify the transaction to associate BufferFull events with.
   // The second tid is for the Register command itself.
   // In both cases we want the ConsumerEventHandler to handle the events
   pAppContext->ProducerAFU()->RegisterConsumer(tid, tid);

   // Wait for the completion event
   pThread->Wait();

   // Wake up main thread
   pAppContext->Post();

   // While all is well consume buffers
   while ( pAppContext->IsOK() ) {
      WSBufDesc Buffer = pAppContext->ProducerQ().GetNext();

      // A null buffer means we are done
      if ( Buffer.size != 0 ) {
         cout.write((const char *)Buffer.pBuf, Buffer.size);

         // Free the buffer
         pAppContext->ProducerAFU()->FreeBuffer(Buffer);

      } else {
         // Exit the loop and the thread
         pAppContext->ProducerAFU()->FreeBuffer(Buffer);
         break;
      }
   }

   // Unregister our callback
   pAppContext->ProducerAFU()->UnRegisterConsumer(tid);

   // Wait for the Unregister complete
   pThread->Wait();
}



/**
@addtogroup XLSample2
@{

@verbatim
This application is for example purposes only.
It is not intended to represent a model for developing commercially-deployable applications.
It is designed to show working examples of the AAL programming model and APIs.@endverbatim

This sample uses SampleAFU2. The application represents a data transform filter. Data is sourced
from standard input or a file, pushed through the transform AFU, encoded and streamed back to
the consumer portion of the application where it is sent to standard output.

In addition to demonstrating the features of SampleAFU2, it demonstrates the following:

<ul>
  <li>Implementing a multi-threaded AALSDK application.</li>
  <li>Using a system-level context for synchronization.</li>
  <li>An AFU that supports multiple Interfaces simultaneously.</li>
  <li>Custom events.</li>
  <li>Principles of creating blocking wrapper functions around AAL.</li>
  <li>Multiple Event Handlers.</li>
  <li>Advanced use of TransactionIDs for custom callbacks.</li>
  <li>A data streaming design pattern with AFU defined buffers.</li>
</ul>

This sample is designed to be used with SampleAFU2.

1 Principles of Operation

The XLSample2 program uses the ISampleAFU2Producer and ISampleAFU2Consumer interfaces of
SampleAFU2. Refer to SampleAFU2Service.h.

This example implements a simple "encryption" transform AFU. It uses a simple
algorithm to encrypt an input data stream. It responds by outputting the
encrypted stream.

The input and output streaming occur in parallel. The process continues until
the application stops the AFU. The actual "encryption" is implemented by
simply performing an XOR, flipping each bit in the octet. Running the
"encrypted" file through the transform a second time will result in the
original file being restored.

The input source can be a file passed as a single command line argument,
otherwise it takes its data from standard input - terminate with
<enter>^D<enter>. Output is streamed to the standard output stream. Sign on
banner and other status is displayed through standard error.

Example Usages:
Takes the input source from file.in and writes the output to stdout. Commentary
output on stderr is redirected as otherwise it becomes intermixed with the
output.
@code
$ XLSample2 file.in 2>/dev/null@endcode

Displays status info and stores output in file.out. In this case, stdout is
going to file.out, so stderr comes to the console.
@code
$ XLSample2 file.in >file.out@endcode

Encrypts original file and then decrypts, displaying the result on stdout and
redirecting status messages to /dev/null. This is equivalent to the cat command.
@code
$ XLSample2 <file.in 2>/dev/null | XLSample2 2>/dev/null@endcode

1.0 Implementing a multi-threaded application

The XLSample2 application creates 2 threads. The main thread is responsible for
system start/stop and AFU creation. It also acts as the "Producer" application.
That is, the application that passes the sourced data to the transform AFU's
“Consumer” interface.

The second thread is the "Consumer" Application. This thread is responsible for
receiving full, encrypted data packets from the "Producer" AFU interface and
sending them to standard out.

In addition to the application's 2 threads, the “Consumer” and “Producer” transform functions
from SampleAFU2 also operate in their own threads. These threads are not AAL-
specific threads. (i.e. they are not created by AAL like the event delivery threads)

Finally, AAL provides a couple of additional threads making a total of 6 threads
running in this application at any one time.
<ul>
  <li>The 5th thread is the AAL EDS, or Event Delivery Service, thread.</li>
  <li>The 6th thread is for AAL maintenance and does not participate in the relevant flows.</li>
</ul>

See Diagram below for the data flow.
@code
T1 Producer App   T2 AFU Consumer   T3 AFU Producer  T4 EDS  T5 Consumer App
------|--------   ------|--------   --------|------  ---|--  ---------|-----
                        |--Empty Buf------------------->|
      |<---Empty Buf Evt--------------------------------|
      |----Input Data-->|----Input Data---->|---Output->|
                                                        |-Output Buf->|
                                            |<-----Empty Buf----------|

                    Diagram 1: Message Sequence Chart@endcode

NOTE: The Sample is constructed as a single process that is logically divided
      into a Producer and Consumer "application" component.  When we refer to
      "Producer" or "Consumer" application we refer to the logical functional
      unit in the single application process.

When the Producer application reaches the end of data (EOD) it passes an empty
buffer to the Consumer AFU which gets passed back to the Consumer App, delimiting
the stream and signaling to the Consumer App that it should exit (return from thread).

The Main thread (Producer) waits for the termination of the Consumer thread,
indicating that all of the streamed data has been processed and it is OK to
stop the AFU. The App then issues a stop and returns any unused empty buffers
back to the AFU. Once the buffers are returned and the AFU is done a Stop
complete is returned.

NOTE: This protocol is not suggested as a prefered method but merely serves
      as a simple example.

Diagram 2 illustrates this protocol.

@code
T1 Producer App   T2 AFU Consumer   T3 AFU Producer        T4 EDS  T5 Consumer App
------|--------   ------|--------   --------|-------  --------|--  ----|-----
      |----Last Data--->|----Input Data---->|-----Output----->|
      |---Empty Buffer->|----Empty Buffer-->|--Empty Buffer-->|
      | "Wait for T5 Exit - final buffers are processsed"
                                                        |-----NULL---->| (Exit)
      |<----------------------------- Signal  -------------------------|
      |----- Stop------>|
      |-- Unused Bufs-->|
      |<--- Stop Complt-|

                    Diagram 2: Completion protocol - Message Sequence Chart@endcode

1.1 Using a system level context for synchronization

An application-defined context object is used to hold the Consumer and Producer
Queues, synchronization objects, and cached copies of the various AFU interfaces.
This object is used to synchronize the activities of the various threads as
well as provide a container to cache the AFU interfaces and data queues. This
object is accessed by multiple threads simultaneously and must be thread safe.

1.2 AFU that support multiple Interfaces simultaneously

SampleAFU2 implements a simple transform filter. Because it implements both
sides of the transform, it publishes two interfaces in addition to ServiceBase. The
ISampleAFU2Consumer interface presents the interface to the source of the data.
ISampleAFU2Producer represents the interface that produces the output data.

The interfaces have been split to abstract the functionality of the Producer and
Consumer from the implementation detail of a single AFU. This allows the
Consumer and Producer application components to interact with the AFU Producer
interface and AFU Consumer Interface respectively without "seeing" the details
of the other. This design pattern gives the illusion of having 2 independent
AFUs.

1.3 Custom events

The SampleAFU2 interface defines 2 proprietary event interfaces for passing
full and empty buffers to the application.

1.4 Principles of creating blocking wrapper functions around AAL

XLSample2 demonstrates the principles behind creating blocking (i.e., synchronous)
functions using AAL's asynchronous model.

The main thread uses a context object containing a semaphore to synchronize with
the events reported through the AAL Event Delivery Service. The source encloses
the code blocks that could logically be separated out as subroutines or methods
in braces, thus { }.

For a full example of creating a synchronous library, refer to XLSample2.

1.5 Multiple Event Handlers

This sample uses AAL's event routing capabilities to define multiple specialized
event handlers to simplify the logic and design. The event handlers perform
short, fast operations on the events they receive, signaling their client
threads (e.g., Producer or Consumer) who then perform the bulk of the work. This
minimizes the time spent in the AAL Event Delivery Thread and improves event
throughput from AAL.

The sample demonstrates using the AFU in a multi-threaded application. The AFU
generates events to request data when it has empty buffers. The event contains
an available workspace buffer.

The application fills the buffer in its producer (main) thread and passes it back
to the AFU for processing. When the AFU has processed a buffer (i.e., in this case
trivially encrypting it), it generates an event to the application consumer thread,
passing it the buffer in the event.

When the Consumer application has completed its processing in turn, it returns
the buffer to the AFU. This continues until the Producer data is exhausted, at
which point the AFU is halted.

1.6 Advanced use of TransactionIDs for custom callbacks

Until now (AALSample1 & XLSample2) TransactionID's have been passed when invoking an operation
and have been used for synchronizing with the completion event. In this example we
show how the TransactionID's can be used to implement a custom callback paradigm that
uses the Event Delivery Services for dispatching asynchronous messages back to
the application.

The AFU defines ISampleAFU2Producer::RegisterConsumer() and ISampleAFU2Producer::UnRegisterConsumer()
methods as part of the SampleAFU2 interface. The RegisterConsumer() method allows the
application to register a callback handler to which buffer full (and therefore
available) events will be delivered. The AFU then uses the EDS to deliver the
events.

While the AFU could have defined a completely custom interface for installing
callbacks, using the EDS allows the application to immediately be able to take
advantage of all of the facilities provided by EDS such as thread safety,
Transaction Contexts, etc.

This model can be used to create a more peer-to-peer relationship between
application and AFU.

1.7 A data streaming design pattern with AFU defined buffers

Unlike previous examples where the application was responsible for allocating
and freeing workspace buffers, in this example the AFU allocates and frees all
workspace buffers. It implements a "pull" model for sourcing data in that the
AFU will pass the application an empty buffer that it needs filled. The
application fills the buffer and returns it to the AFU. The AFU pushes full
packets to the Consumer Application, who then processes (sends to cout) the
content and returns the buffer back to the AFU.

2 Running the application

Takes the input source from file.in and writes the output to stdout. Commentary
output on stderr is redirected as otherwise it becomes intermixed with the
output.
@code
$ XLSample2 file.in 2>/dev/null@endcode

Displays status info and stores output in file.out. In this case, stdout is
going to file.out, so stderr comes to the console.
@code
$ XLSample2 file.in >file.out@endcode

Encrypts original file and then decrypts, displaying the result on stdout and
redirecting status messages to /dev/null. This is equivalent to the cat command.
@code
$ XLSample2 <file.in 2>/dev/null | XLSample2 2>/dev/null@endcode

@} group XLSample2
*/

