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
/// @file Sudoku_SPL.cpp
/// @brief SPL enabled version of Sudoku.
/// @ingroup Sudoku
/// @verbatim
/// Accelerator Abstraction Layer Sample Application
///
///    This application is for example purposes only.
///    It is not intended to represent a model for developing commercially-deployable applications.
///    It is designed to show working examples of the AAL programming model and APIs.
///
/// AUTHORS: David Sheffield, Intel Corporation.
///
/// This Sample demonstrates the following:
///    - Doing Sudoku within the SPL framework
///
/// This sample is designed to be used with the SPLAFU Service.
///
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 06/29/2015     HM       Added David's version to the tree@endverbatim
//****************************************************************************
#include <aalsdk/AAL.h>
#include <aalsdk/Runtime.h>
#include <aalsdk/AALLoggerExtern.h> // Logger


#include <aalsdk/service/ISPLAFU.h>       // Service Interface
#include <aalsdk/service/SPLAFUService.h>
#include <aalsdk/service/ISPLClient.h>    // Service Client Interface
#include <aalsdk/kernel/vafu2defs.h>      // AFU structure definitions (brings in spl2defs.h)

#include <string.h>
#include <cassert>

//****************************************************************************
// UN-COMMENT appropriate #define in order to enable either Hardware or ASE.
//    DEFAULT is to use Software Simulation.
//****************************************************************************
// #define  HWAFU
#define  ASEAFU

using namespace std;
using namespace AAL;

// Convenience macros for printing messages and errors.
#ifdef MSG
# undef MSG
#endif // MSG
#define MSG(x) std::cout << __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() : " << x << std::endl
#ifdef ERR
# undef ERR
#endif // ERR
# define ERR(x) std::cerr << __AAL_SHORT_FILE__ << ':' << __LINE__ << ':' << __AAL_FUNC__ << "() **Error : " << x << std::endl

// Print/don't print the event ID's entered in the event handlers.
#if 1
# define EVENT_CASE(x) case x : MSG(#x);
#else
# define EVENT_CASE(x) case x :
#endif

#ifndef CL
# define CL(x)                     ((x) * 64)
#endif // CL
#ifndef LOG2_CL
# define LOG2_CL                   6
#endif // LOG2_CL
#ifndef MB
# define MB(x)                     ((x) * 1024 * 1024)
#endif // MB

#define LPBK1_DSM_SIZE           MB(4)

/// @addtogroup SudokuSample
/// @{

inline uint32_t ln2(uint32_t x)
{
   uint32_t y = 1;
   while ( x > 1 ) {
      y++;
      x = x >> 1;
   }
   return y;
}

inline uint32_t isPow2(uint32_t x)
{
   uint32_t pow2 = (x & (x - 1));
   return (pow2 == 0);
}

inline void find_min(uint32_t *board, int32_t *min_idx, int *min_pos)
{
   int32_t tmp, idx, i, j;
   int32_t tmin_idx, tmin_pos;

   tmin_idx = 0;
   tmin_pos = INT_MAX;
   for (idx = 0; idx < 81; idx++)
      {
      tmp = __builtin_popcount(board[idx]);
      tmp = (tmp == 1) ? INT_MAX : tmp;
      if ( tmp < tmin_pos )
         {
         tmin_pos = tmp;
         tmin_idx = idx;
      }
   }
   *min_idx = tmin_idx;
   *min_pos = tmin_pos;
}

#define RuntimeClient SudokuSPLRuntimeClient
/// @brief   Define our Runtime client class so that we can receive the runtime started/stopped notifications.
///
/// We implement a Service client within, to handle AAL Service allocation/free.
/// We also implement a Semaphore for synchronization with the AAL runtime.
class RuntimeClient : public CAASBase,
                      public IRuntimeClient
{
public:
	RuntimeClient();
	   ~RuntimeClient();

	   void end();
	   IRuntime* getRuntime();

	   btBool isOK();

   // <begin IRuntimeClient interface>
   void runtimeCreateOrGetProxyFailed(IEvent const &rEvent);

   void runtimeStarted(IRuntime            *pRuntime,
                       const NamedValueSet &rConfigParms);

   void runtimeStopped(IRuntime *pRuntime);

   void runtimeStartFailed(const IEvent &rEvent);

   void runtimeStopFailed(const IEvent &rEvent);

   void runtimeAllocateServiceFailed( IEvent const &rEvent);

   void runtimeAllocateServiceSucceeded(IBase               *pClient,
                                        TransactionID const &rTranID);

   void runtimeEvent(const IEvent &rEvent);
   

   // <end IRuntimeClient interface>


protected:
   IRuntime        *m_pRuntime;  // Pointer to AAL runtime instance.
   Runtime          m_Runtime;   // AAL Runtime
   btBool           m_isOK;      // Status
   CSemaphore       m_Sem;       // For synchronizing with the AAL runtime.
};

///  MyRuntimeClient Implementation
RuntimeClient::RuntimeClient() :
    m_Runtime(this),        // Instantiate the AAL Runtime
    m_pRuntime(NULL),
    m_isOK(false)
{
   NamedValueSet configArgs;
   NamedValueSet configRecord;

   // Publish our interface
   SetInterface(iidRuntimeClient, dynamic_cast<IRuntimeClient *>(this));

   m_Sem.Create(0, 1);

   // Using Hardware Services requires the Remote Resource Manager Broker Service
   //  Note that this could also be accomplished by setting the environment variable
   //   XLRUNTIME_CONFIG_BROKER_SERVICE to librrmbroker
#if defined( HWAFU )
   configRecord.Add(XLRUNTIME_CONFIG_BROKER_SERVICE, "librrmbroker");
   configArgs.Add(XLRUNTIME_CONFIG_RECORD,configRecord);
#endif

   m_Runtime.start(configArgs);
   m_Sem.Wait();
}

RuntimeClient::~RuntimeClient()
{
    m_Sem.Destroy();
}

/// @brief Checks that the object is in an internally consistent state
///
/// The general paradigm in AAL is for an object to track its internal state for subsequent query,
/// as opposed to throwing exceptions or having to constantly check return codes.
/// We implement this to check if the status of the service allocated.
/// In this case, isOK can be false for many reasons, but those reasons will already have been indicated by logging output.
btBool RuntimeClient::isOK()
{
   return m_isOK;
}

void RuntimeClient::runtimeCreateOrGetProxyFailed(IEvent const &rEvent)
{
   MSG("Runtime Create or Get Proxy failed");
   m_isOK = false;
   m_Sem.Post(1);
}

void RuntimeClient::runtimeStarted(IRuntime *pRuntime,
                                   const NamedValueSet &rConfigParms)
{
   // Save a copy of our runtime interface instance.
   m_pRuntime = pRuntime;
   m_isOK = true;
   m_Sem.Post(1);
}

/// @brief Synchronous wrapper for stopping the Runtime.
///
/// @return void
void RuntimeClient::end()
{
   m_Runtime.stop();
   m_Sem.Wait();
}

void RuntimeClient::runtimeStopped(IRuntime *pRuntime)
{
   MSG("Runtime stopped");
   m_isOK = false;
   m_Sem.Post(1);
}

void RuntimeClient::runtimeStartFailed(const IEvent &rEvent)
{
   IExceptionTransactionEvent * pExEvent = dynamic_ptr<IExceptionTransactionEvent>(iidExTranEvent, rEvent);
   ERR("Runtime start failed");
   ERR(pExEvent->Description());
}

void RuntimeClient::runtimeStopFailed(const IEvent &rEvent)
{
   MSG("Runtime stop failed");
}

void RuntimeClient::runtimeAllocateServiceFailed(IEvent const &rEvent)
{
   IExceptionTransactionEvent * pExEvent = dynamic_ptr<IExceptionTransactionEvent>(iidExTranEvent, rEvent);
   ERR("Runtime AllocateService failed");
   ERR(pExEvent->Description());
}

void RuntimeClient::runtimeAllocateServiceSucceeded(IBase *pClient,
                                                    TransactionID const &rTranID)
{
   MSG("Runtime Allocate Service Succeeded");
}

void RuntimeClient::runtimeEvent(const IEvent &rEvent)
{
   MSG("Generic message handler (runtime)");
}

/// @brief Accessor for pointer to IRuntime stored in Runtime Client
///
/// @return A pointer to the IRuntime.
IRuntime * RuntimeClient::getRuntime()
{
   return m_pRuntime;
}


/// @brief   Define our Service client class so that we can receive Service-related notifications from the AAL Runtime.
///          The Service Client contains the application logic.
///
/// When we request an AFU (Service) from AAL, the request will be fulfilled by calling into this interface.
class Sudoku: public CAASBase, public IServiceClient, public ISPLClient
{
public:

   Sudoku(RuntimeClient * rtc, char *puzName);
   ~Sudoku();

   btInt run();

   // <ISPLClient>
   virtual void OnTransactionStarted(TransactionID const &TranID,
                                     btVirtAddr AFUDSM,
                                     btWSSize AFUDSMSize);
   virtual void OnContextWorkspaceSet(TransactionID const &TranID);

   virtual void OnTransactionFailed(const IEvent &Event);

   virtual void OnTransactionComplete(TransactionID const &TranID);

   virtual void OnTransactionStopped(TransactionID const &TranID);
   virtual void OnWorkspaceAllocated(TransactionID const &TranID,
                                     btVirtAddr WkspcVirt,
                                     btPhysAddr WkspcPhys,
                                     btWSSize WkspcSize);

   virtual void OnWorkspaceAllocateFailed(const IEvent &Event);

   virtual void OnWorkspaceFreed(TransactionID const &TranID);

   virtual void OnWorkspaceFreeFailed(const IEvent &Event);
   // </ISPLClient>

   // <begin IServiceClient interface>
   virtual void serviceAllocated(IBase *pServiceBase,
                                 TransactionID const &rTranID);

   virtual void serviceAllocateFailed(const IEvent &rEvent);

   void serviceReleaseFailed(const IEvent &rEvent);

   void serviceReleased(TransactionID const &rTranID);

   virtual void serviceEvent(const IEvent &rEvent);
   // <end IServiceClient interface>

   /* SW implementation of a sudoku solver */
   static void print_board(uint32_t *board);
   static int32_t sudoku_norec(uint32_t *board, uint32_t *os);
   static int32_t check_correct(uint32_t *board, uint32_t *unsolved_pieces);
   static int32_t solve(uint32_t *board, uint32_t *os);
   protected:

   char          *m_puzName;
   IBase         *m_pAALService;    // The generic AAL Service interface for the AFU.
   RuntimeClient *m_runtimClient;
   ISPLAFU       *m_SPLService;
   CSemaphore     m_Sem;            // For synchronizing with the AAL runtime.
   btInt          m_Result;

   // Workspace info
   btVirtAddr     m_pWkspcVirt;     ///< Workspace virtual address.
   btWSSize       m_WkspcSize;      ///< DSM workspace size in bytes.

   btVirtAddr     m_AFUDSMVirt;     ///< Points to DSM
   btWSSize       m_AFUDSMSize;     ///< Length in bytes of DSM
};




/* DBS: for sudoku */


/// @brief Prints the Sudoku board.
/// @param[in] board A pointer the board.
/// @return void
void Sudoku::print_board(uint32_t *board)
{
   int32_t i, j;
   char buf[80] = { 0 };
   for (i = 0; i < 9; i++) {
      for (j = 0; j < 9; j++) {
         if ( board[i * 9 + j] == 511 )
            printf("? ");
         else
            printf("%d ", ln2(board[i * 9 + j]));
      }
      printf("\n");
   }
   printf("\n");
}

/// @brief Checks if the Puzzle is solved.
///
/// @param[in] board A pointer the board.
/// @param[in] unsolved_pieces A pointer to the board containining unsolved squares.
/// @retval True if the board is correct, the puzzle is solved.
/// @retval False if the board is not correct.
int32_t Sudoku::check_correct(uint32_t *board, uint32_t *unsolved_pieces)
{
   int32_t i, j;
   int32_t ii, jj;
   int32_t si, sj;
   int32_t tmp;

   *unsolved_pieces = 0;
   int32_t violated = 0;

   uint32_t counts[81];
   for (i = 0; i < 81; i++)
      {
      counts[i] = __builtin_popcount(board[i]);
      if ( counts[i] != 1 )
         {
         *unsolved_pieces = 1;
         return 0;
      }
   }

   /* for each row */
   for (i = 0; i < 9; i++)
      {
      uint32_t sums[9] = { 0 };
      for (j = 0; j < 9; j++)
         {
         if ( counts[i * 9 + j] == 1 )
            {
            tmp = ln2(board[i * 9 + j]) - 1;
            sums[tmp]++;
            if ( sums[tmp] > 1 )
               {
               //char buf[80];
               //sprintf_binary(board[i*9+j],buf,80);
               //printf("violated row %d, sums[%d]=%d, board = %s\n", i, tmp, sums[tmp], buf);
               //print_board(board);
               violated = 1;
               goto done;
            }
         }
      }
   }
   /* for each column */

   for (j = 0; j < 9; j++)
      {
      uint32_t sums[9] = { 0 };
      for (i = 0; i < 9; i++)
         {
         if ( counts[i * 9 + j] == 1 )
            {
            tmp = ln2(board[i * 9 + j]) - 1;
            sums[tmp]++;
            if ( sums[tmp] > 1 )
               {
               violated = 1;
               goto done;
               //printf("violated column %d, sums[%d]=%d\n", i, tmp, sums[tmp]);
               //return 0;
            }
         }
      }
   }

   for (i = 0; i < 9; i++)
      {
      si = 3 * (i / 3);
      for (j = 0; j < 9; j++)
         {
         sj = 3 * (j / 3);
         uint32_t sums[9] = { 0 };
         for (ii = si; ii < (si + 3); ii++)
            {
            for (jj = sj; jj < (sj + 3); jj++)
               {
               if ( counts[ii * 9 + jj] == 1 )
                  {
                  tmp = ln2(board[ii * 9 + jj]) - 1;
                  sums[tmp]++;
                  if ( sums[tmp] > 1 )
                     {
                     violated = 1;
                     goto done;
                  }
               }
            }
         }
      }
   }

   done:
   return (violated == 0);
}

inline uint32_t one_set(uint32_t x)
{
   /* all ones if pow2, otherwise 0 */
   uint32_t pow2 = (x & (x - 1));
   uint32_t m = (pow2 == 0);
   return ((~m) + 1) & x;
}

/// @brief Logic to solve the puzzle.
/// @param[in] board A pointer the board.
/// @param[in] os A pointer to the board to store interim results.
/// @returns 0.
int32_t Sudoku::solve(uint32_t *board, uint32_t *os)
{
   int32_t i, j, idx;
   int32_t ii, jj;
   int32_t ib, jb;
   uint32_t set_row, set_col, set_sqr;
   uint32_t row_or, col_or, sqr_or;
   uint32_t tmp;
   int32_t changed = 0;

   do
   {
      changed = 0;
      //print_board(board);
      /* compute all positions one's set value */
      for (i = 0; i < 9; i++)
         {
         for (j = 0; j < 9; j++)
            {
            idx = i * 9 + j;
            os[idx] = one_set(board[idx]);
         }
      }

      for (i = 0; i < 9; i++)
         {
         for (j = 0; j < 9; j++)
            {
            /* already solved */
            if ( isPow2(board[i * 9 + j]) )
               continue;
            else if ( board[idx] == 0 )
               return 0;

            row_or = set_row = 0;
            for (jj = 0; jj < 9; jj++)
               {
               idx = i * 9 + jj;
               if ( jj == j )
                  continue;
               set_row |= os[idx];
               row_or |= board[idx];
            }
            col_or = set_col = 0;
            for (ii = 0; ii < 9; ii++)
               {
               idx = ii * 9 + j;
               if ( ii == i )
                  continue;
               set_col |= os[idx];
               col_or |= board[idx];
            }
            sqr_or = set_sqr = 0;
            ib = 3 * (i / 3);
            jb = 3 * (j / 3);
            for (ii = ib; ii < ib + 3; ii++)
               {
               for (jj = jb; jj < jb + 3; jj++)
                  {
                  idx = ii * 9 + jj;
                  if ( (i == ii) && (j == jj) )
                     continue;
                  set_sqr |= os[idx];
                  sqr_or |= board[idx];
               }
            }
            tmp = board[i * 9 + j] & ~(set_row | set_col | set_sqr);

            if ( tmp != board[i * 9 + j] )
               {
               changed = 1;
            }
            board[i * 9 + j] = tmp;

            /* check for singletons */
            tmp = 0;
            tmp = one_set(board[i * 9 + j] & (~row_or));
            tmp |= one_set(board[i * 9 + j] & (~col_or));
            tmp |= one_set(board[i * 9 + j] & (~sqr_or));
            if ( tmp != 0 && (board[i * 9 + j] != tmp) )
               {
               board[i * 9 + j] = tmp;
               changed = 1;
            }
         }
      }

   } while ( changed );

   return 0;
}

/// @brief Solve a puzzle.
/// @param[in] board A pointer the board.
/// @param[in] os A pointer to the board to store interim results.
/// @returns 1.
int32_t Sudoku::sudoku_norec(uint32_t *board, uint32_t *os)
{
   int32_t rc;

   int32_t tmp, min_pos;
   int32_t min_idx;
   int32_t i, j, idx;

   uint32_t cell;
   uint32_t old[81];

   uint32_t unsolved_pieces = 0;
   uint32_t *bptr, *nbptr;

   int32_t stack_pos = 0;
   int32_t stack_size = (1 << 6);
   uint32_t **stack = 0;

   stack = (uint32_t**) malloc(sizeof(uint32_t*) * stack_size);
   for (i = 0; i < stack_size; i++)
      {
      stack[i] = (uint32_t*) malloc(sizeof(uint32_t) * 81);
   }

   memcpy(stack[stack_pos++], board, sizeof(uint32_t) * 81);

   while ( stack_pos > 0 )
   {
      unsolved_pieces = 0;
      bptr = stack[--stack_pos];

      bzero(os, sizeof(uint32_t) * 81);
      solve(bptr, os);
      rc = check_correct(bptr, &unsolved_pieces);
      /* solved puzzle */
      if ( rc == 1 && unsolved_pieces == 0 )
         {
         memcpy(board, bptr, sizeof(uint32_t) * 81);
         goto solved_puzzle;
      }
      /* traversed to bottom of search tree and
       * didn't find a valid solution */
      if ( rc == 0 && unsolved_pieces == 0 )
         {
         continue;
      }

      find_min(bptr, &min_idx, &min_pos);
      cell = bptr[min_idx];
      while ( cell != 0 )
      {
         tmp = __builtin_ctz(cell);
         cell &= ~(1 << tmp);
         nbptr = stack[stack_pos];
         stack_pos++;
         memcpy(nbptr, bptr, sizeof(uint32_t) * 81);
         nbptr[min_idx] = 1 << tmp;

         assert(stack_pos < stack_size);
      }
   }
   solved_puzzle:

   for (i = 0; i < stack_size; i++)
      {
      free(stack[i]);
   }
   free(stack);

   return 1;
}


///  Implementation
Sudoku::Sudoku(RuntimeClient *rtc, char *puzName) :
   m_puzName(puzName),
   m_pAALService(NULL),
   m_runtimClient(rtc),
   m_SPLService(NULL),
   m_Result(0),
   m_pWkspcVirt(NULL),
   m_WkspcSize(0),
   m_AFUDSMVirt(NULL),
   m_AFUDSMSize(0)
{
   SetInterface(iidServiceClient, dynamic_cast<IServiceClient *>(this));
   SetInterface(iidSPLClient, dynamic_cast<ISPLClient *>(this));
   SetInterface(iidCCIClient, dynamic_cast<ICCIClient *>(this));
   m_Sem.Create(0, 1);
}

Sudoku::~Sudoku()
{
   m_Sem.Destroy();
}

/// @brief Called by the main part of the application
///
/// Application Requests Service using Runtime Client passing a pointer to self.
/// Blocks calling thread from [Main} untill application is done.
/// @retval 0 if Successful.
btInt Sudoku::run()
{
   cout <<"======================="<<endl;
   cout <<"= Hello SPL LB Sample ="<<endl;
   cout <<"======================="<<endl;

   // Request our AFU.

   // NOTE: This example is bypassing the Resource Manager's configuration record lookup
   //  mechanism.  This code is work around code and subject to change.
   NamedValueSet Manifest;
   NamedValueSet ConfigRecord;


#if defined( HWAFU )                /* Use FPGA hardware */
   ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libHWSPLAFU");
   ConfigRecord.Add(keyRegAFU_ID,"5DA62813-9A75-4228-8FDB-5D4006DD55CE");
   ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_AIA_NAME, "libAASUAIA");

   #elif defined ( ASEAFU )
   ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libASESPLAFU");
   ConfigRecord.Add(AAL_FACTORY_CREATE_SOFTWARE_SERVICE,true);

#else

   ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libSWSimSPLAFU");
   ConfigRecord.Add(AAL_FACTORY_CREATE_SOFTWARE_SERVICE,true);
#endif

   Manifest.Add(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &ConfigRecord);

   Manifest.Add(AAL_FACTORY_CREATE_SERVICENAME, "Hello SPL LB");

   MSG("Allocating Service");

   // Allocate the Service and allocate the required workspace.
   //   This happens in the background via callbacks (simple state machine).
   //   When everything is set we do the real work here in the main thread.
   m_runtimClient->getRuntime()->allocService(dynamic_cast<IBase *>(this), Manifest);

   m_Sem.Wait();

   // If all went well run test.
   //   NOTE: If not successful we simply bail.
   //         A better design would do all appropriate clean-up.
   if(0 == m_Result){


      //=============================
      // Now we have the NLB Service
      //   now we can use it
      //=============================
      MSG("Running Test");

      btVirtAddr         pWSUsrVirt = m_pWkspcVirt; // Address of Workspace
      const btWSSize     WSLen      = m_WkspcSize; // Length of workspace

      INFO("Allocated " << WSLen << "-byte Workspace at virtual address "
                        << std::hex << (void *)pWSUsrVirt);

      // Number of bytes in each of the source and destination buffers (4 MiB in this case)
      btUnsigned32bitInt a_num_bytes= (btUnsigned32bitInt) ((WSLen - sizeof(VAFU2_CNTXT)) / 2);
      btUnsigned32bitInt a_num_cl   = a_num_bytes / CL(1);  // number of cache lines in buffer

      // VAFU Context is at the beginning of the buffer
      VAFU2_CNTXT       *pVAFU2_cntxt = reinterpret_cast<VAFU2_CNTXT *>(pWSUsrVirt);

      // The source buffer is right after the VAFU Context
      btVirtAddr         pSource = pWSUsrVirt + sizeof(VAFU2_CNTXT);

      // The destination buffer is right after the source buffer
      btVirtAddr         pDest   = pSource + a_num_bytes;

      struct OneCL {                      // Make a cache-line sized structure
         btUnsigned32bitInt dw[16];       //    for array arithmetic
      };
      struct OneCL      *pSourceCL = reinterpret_cast<struct OneCL *>(pSource);
      struct OneCL      *pDestCL   = reinterpret_cast<struct OneCL *>(pDest);

      // Note: the usage of the VAFU2_CNTXT structure here is specific to the underlying bitstream
      // implementation. The bitstream targeted for use with this sample application must implement
      // the Validation AFU 2 interface and abide by the contract that a VAFU2_CNTXT structure will
      // appear at byte offset 0 within the supplied AFU Context workspace.

      // Initialize the command buffer
      ::memset(pVAFU2_cntxt, 0, sizeof(VAFU2_CNTXT));
      pVAFU2_cntxt->num_cl  = a_num_cl;
      pVAFU2_cntxt->pSource = pSource;
      pVAFU2_cntxt->pDest   = pDest;

      INFO("VAFU2 Context=" << std::hex << (void *)pVAFU2_cntxt <<
           " Src="          << std::hex << (void *)pVAFU2_cntxt->pSource <<
           " Dest="         << std::hex << (void *)pVAFU2_cntxt->pDest << std::dec);

      /* Read a puzzle file : assuming format from http://magictour.free.fr/top95  */
      uint32_t nPuzzles = 0;
      FILE *fp = fopen(m_puzName, "r");
      assert(fp);
      int c;
      while((c = fgetc(fp)) != EOF) {
         if(c == '\n')
         nPuzzles++;
      }
      printf("nPuzzles=%u\n", nPuzzles);
      uint32_t **puzzles = (uint32_t**)malloc(sizeof(uint32_t*)*(nPuzzles));
      rewind(fp);
      {
         uint32_t j = 0, i = 0;
         for (i = 0; i < (nPuzzles); i++)
            puzzles[i] = (uint32_t*) malloc(sizeof(uint32_t) * 81);
         i = 0;
         printf("done with malloc\n");
         while ( (c = fgetc(fp)) != EOF )
         {
            if ( c == '\n' )
               {
               //printf("i=%u\n", i);
               assert(j == 81);
               j = 0;
               i++;
               if ( i == nPuzzles )
                  break;
            }
            else if ( c == ' ' )
               {
               continue;
            }
            else if ( c == '.' )
               {
               puzzles[i][j++] = 0x1ff;
            }
            else
            {
               if ( c >= 65 && c <= 70 )
                  c -= 55;
               else
                  c -= 48;
               puzzles[i][j++] = 1 << (c - 1);
            }
         }
      }
      fclose(fp);
      printf("got puzzles\n");
      volatile uint32_t *boardIn = (uint32_t*)pSource;
      /* Forget about everything but the first one */
      memcpy((void*)boardIn, puzzles[nPuzzles-1], sizeof(uint32_t)*81);
      print_board((uint32_t*)boardIn);

      for(uint32_t i = 0; i < nPuzzles; i++)
         free(puzzles[i]);
      free(puzzles);

      /* Call software implementation */
      uint32_t board[81] = {0};
      uint32_t os[81] = {0};
      memcpy(board, (void*)boardIn, 81*sizeof(uint32_t));
      sudoku_norec(board, os);

      // Buffers have been initialized
      ////////////////////////////////////////////////////////////////////////////

      ////////////////////////////////////////////////////////////////////////////
      // Get the AFU and start talking to it

      // Acquire the AFU. Once acquired in a TransactionContext, can issue CSR Writes and access DSM.
      // Provide a workspace and so also start the task.
      // The VAFU2 Context is assumed to be at the start of the workspace.
      INFO("Starting SPL Transaction with Workspace");
      m_SPLService->StartTransactionContext(TransactionID(), pWSUsrVirt, 100);
      m_Sem.Wait();

      // The AFU is running
      ////////////////////////////////////////////////////////////////////////////

      ////////////////////////////////////////////////////////////////////////////
      // Wait for the AFU to be done. This is AFU-specific, we have chosen to poll ...

      // Set timeout increment based on hardware, software, or simulation
      bt32bitInt count(500);  // 5 seconds with 10 millisecond sleep
      bt32bitInt delay(1000);   // 10 milliseconds is the default

      // Wait for SPL VAFU to finish code
      volatile bt32bitInt done = pVAFU2_cntxt->Status & VAFU2_CNTXT_STATUS_DONE;
      while (!done && --count) {
         SleepMilli( delay );
         done = pVAFU2_cntxt->Status & VAFU2_CNTXT_STATUS_DONE;
      }
      if ( !done ) {
         // must have dropped out of loop due to count -- never saw update
         ERR("AFU never signaled it was done. Timing out anyway. Results may be strange.\n");
      }
      ////////////////////////////////////////////////////////////////////////////
     // Stop the AFU
      volatile uint32_t *boardOut = (uint32_t*)pDest;
      printf("SW:\n");
      print_board(board);
      printf("HW:\n");
      print_board((uint32_t*)boardOut);


      // Issue Stop Transaction and wait for OnTransactionStopped
      INFO("Stopping SPL Transaction");
      m_SPLService->StopTransactionContext(TransactionID());
      m_Sem.Wait();
      INFO("SPL Transaction complete");


   }

   ////////////////////////////////////////////////////////////////////////////
   // Clean up and exit
   INFO("Workspace verification complete, freeing workspace.");
   m_SPLService->WorkspaceFree(m_pWkspcVirt, TransactionID());
   m_Sem.Wait();

   m_runtimClient->end();
   return m_Result;
}

// We must implement the IServiceClient interface (IServiceClient.h):

// <begin IServiceClient interface>

/// @brief Callback called after a Service allocation succeeded.
/// @param[out] pServiceBase A pointer to Service.
/// @param[out] rEvent A reference to an IEvent containing information
///             about the failure.
/// @return void
void Sudoku::serviceAllocated(IBase               *pServiceBase,
                              TransactionID const &rTranID)
{
   m_pAALService = pServiceBase;
   ASSERT(NULL != m_pAALService);

   // Documentation says SPLAFU Service publishes ISPLAFU as subclass interface
   m_SPLService = dynamic_ptr<ISPLAFU>(iidSPLAFU, pServiceBase);

   ASSERT(NULL != m_SPLService);
   if ( NULL == m_SPLService ) {
      return;
   }

   MSG("Service Allocated");

   // Allocate Workspaces needed. ASE runs more slowly and we want to watch the transfers,
   //   so have fewer of them.
#if defined ( ASEAFU )
#define LB_BUFFER_SIZE CL(16)
#else
#define LB_BUFFER_SIZE MB(4)
#endif

   m_SPLService->WorkspaceAllocate(sizeof(VAFU2_CNTXT) + LB_BUFFER_SIZE + LB_BUFFER_SIZE,
      TransactionID());

}

/// @brief Callback called after a Service allocation failed.
/// @param[out] rEvent A reference to an IEvent containing information
///             about the failure.
/// @return void
void Sudoku::serviceAllocateFailed(const IEvent &rEvent)
{
   IExceptionTransactionEvent * pExEvent = dynamic_ptr<IExceptionTransactionEvent>(iidExTranEvent, rEvent);
   ERR("Failed to allocate a Service");
   ERR(pExEvent->Description());
   ++m_Result;
   m_Sem.Post(1);
}

/// @brief Callback called after a Service release failed.
///
/// @param[out] rEvent A reference to an IEvent containing information
///             about the failure.
/// @return void
void Sudoku::serviceReleaseFailed(const IEvent        &rEvent)
{
   MSG("Failed to Release a Service");
   m_Sem.Post(1);
}

/// @brief Callback called after a Service has been successfully
///        released.
///
/// @param[out] rTranID A reference to the TransactionID.
/// @return void
void Sudoku::serviceReleased(TransactionID const &rTranID)
{
   MSG("Service Released");
  m_Sem.Post(1);
}

// <ISPLClient>

/// @brief A Client callback called after a Workspace is allocated.
///
/// Sudoku Client implementation of ISPLClient::OnWorkspaceAllocated().
///
/// This is how the client is supplied with the data required to
/// utilize the Workspace - its address and size. This callback is
/// the only mechanism for the client to notified when a Workspace
/// has been allocated and must be implemented for the client to
/// receive these notifications.
///
/// @param[out] TranID A reference to the TransactionID.
/// @param[out] WkspcVirt The virtual address of the Workspace.
/// @param[out] WkspcPhys The physical address of the Workspace.
/// @param[out] WkspcSize The size of the Workspace.
/// @return void
void Sudoku::OnWorkspaceAllocated(TransactionID const &TranID,
                                  btVirtAddr           WkspcVirt,
                                  btPhysAddr           WkspcPhys,
                                  btWSSize             WkspcSize)
{
   AutoLock(this);

   m_pWkspcVirt = WkspcVirt;
   m_WkspcSize = WkspcSize;

   INFO("Got Workspace");         // Got workspace so unblock the Run() thread
   m_Sem.Post(1);
}

/// @brief A Client callback called after a Workspace allocation failed.
///
/// Sudoku Client implementation of ISPLClient::OnWorkspaceAllocateFailed().
///
/// This callback is the only mechanism for the client to be notified that
/// a Workspace allocation has failed and must be implemented for the client
/// to receive these notifications.
///
/// @param[out] rEvent A reference to the IEvent containing details about
///             the failure.
/// @return void
void Sudoku::OnWorkspaceAllocateFailed(const IEvent &rEvent)
{
   IExceptionTransactionEvent * pExEvent = dynamic_ptr<IExceptionTransactionEvent>(iidExTranEvent, rEvent);
   ERR("OnWorkspaceAllocateFailed");
   ERR(pExEvent->Description());
   ++m_Result;
   m_Sem.Post(1);
}

/// @brief A Client callback called after a Workspace is freed.
///
/// Sudoku Client implementation of ISPLClient::OnWorkspaceFreed().
///
/// This callback is the only mechanism for the client to be notified when
/// a Workspace has been freed and must be implemented for the client to
/// receive these notifications.
///
/// @param[out] TranID A reference to the TransactionID.
/// @return void
void Sudoku::OnWorkspaceFreed(TransactionID const &TranID)
{
   ERR("OnWorkspaceFreed");
   // Freed so now Release() the Service through the Services IAALService::Release() method
   (dynamic_ptr<IAALService>(iidService, m_pAALService))->Release(TransactionID());
}

/// @brief A Client callback called after an attempt to free a Workspace
///        failed.
///
/// Sudoku Client implementation of ISPLClient::OnWorkspaceFreeFailed().
///
/// This callback is the only mechanism for the client to be notified when
/// an attempt to free a Workspace has failed and must be implemented for the
/// client to receive these notifications.
///
/// @param[out] rEvent A reference to the IEvent containing details about
///             the failure.
/// @return void
void Sudoku::OnWorkspaceFreeFailed(const IEvent &rEvent)
{
   IExceptionTransactionEvent * pExEvent = dynamic_ptr<IExceptionTransactionEvent>(iidExTranEvent, rEvent);
   ERR("OnWorkspaceAllocateFailed");
   ERR(pExEvent->Description());
   ++m_Result;
   m_Sem.Post(1);
}

/// @brief A Client callback called after a Transaction is started.
///
/// Sudoku Client implementation of ISPLClient::OnTransactionStarted().
///
/// This callback is the only mechanism for the client to be notified when
/// a Transaction has started and must be implemented for the client to
/// receive these notifications.
///
/// @param[out] TranID A reference to the TransactionID.
/// @param[out] AFUDSMVirt The virtual address of the Device Status Memory.
/// @param[out] AFUDSMSize The size of the Device Status Memory.
/// @return void
void Sudoku::OnTransactionStarted( TransactionID const &TranID,
                                   btVirtAddr           AFUDSMVirt,
                                   btWSSize             AFUDSMSize)
{
   INFO("Transaction Started");
   m_AFUDSMVirt = AFUDSMVirt;
   m_AFUDSMSize =  AFUDSMSize;
   m_Sem.Post(1);
}

/// @brief A Client callback called after a Context Workspace is set.
///
/// Sudoku Client implementation of ISPLClient::OnContextWorkspaceSet().
///
/// This callback is the only mechanism for the client to be notified when
/// a Context Workspace is set and must be implemented for the client to
/// receive these notifications.
///
/// @param[out] TranID A reference to the TransactionID.
/// @return void
void Sudoku::OnContextWorkspaceSet( TransactionID const &TranID)
{
   INFO("Context Set");
   m_Sem.Post(1);
}

/// @brief A Client callback called after a Transaction failed.
///
/// Sudoku Client implementation of ISPLClient::OnTransactionFailed().
///
/// This callback is the only mechanism for the client to be notified when
/// a Transaction failed and must be implemented for the client to
/// receive these notifications.
///
/// @param[out] rEvent A reference to the IEvent containing details about
///             the failure.
/// @return void
void Sudoku::OnTransactionFailed( const IEvent &rEvent)
{
   IExceptionTransactionEvent * pExEvent = dynamic_ptr<IExceptionTransactionEvent>(iidExTranEvent, rEvent);
   MSG("Runtime AllocateService failed");
   MSG(pExEvent->Description());
   m_bIsOK = false;
   ++m_Result;
   m_AFUDSMVirt = NULL;
   m_AFUDSMSize =  0;
   ERR("Transaction Failed");
   m_Sem.Post(1);
}

/// @brief A Client callback called after a Transaction is completed.
///
/// Sudoku Client implementation of ISPLClient::OnTransactionComplete().
///
/// This callback is the only mechanism for the client to be notified when
/// a Transaction has completed and must be implemented for the client to
/// receive these notifications.
///
/// @param[out] TranID A reference to the TransactionID.
/// @return void
void Sudoku::OnTransactionComplete( TransactionID const &TranID)
{
   m_AFUDSMVirt = NULL;
   m_AFUDSMSize =  0;
   INFO("Transaction Complete");
   m_Sem.Post(1);
}

/// @brief A Client callback called after a Transaction is stopped.
///
/// Sudoku Client implementation of ISPLClient::OnTransactionStopped().
///
/// This callback is the only mechanism for the client to be notified when
/// a Transaction has been stopped and must be implemented for the client to
/// receive these notifications.
///
/// @param[out] TranID A reference to the TransactionID.
/// @return void
void Sudoku::OnTransactionStopped( TransactionID const &TranID)
{
   m_AFUDSMVirt = NULL;
   m_AFUDSMSize =  0;
   INFO("Transaction Stopped");
   m_Sem.Post(1);
}

/// @brief Callback called to send unsolicited or unusual events.
///
/// Sudoku Client implementation of ISPLClient::serviceEvent().
///
/// @param[out] rEvent A reference to an IEvent containing information
///              about the event.
/// @return void
void Sudoku::serviceEvent(const IEvent &rEvent)
{
   ERR("unexpected event 0x" << hex << rEvent.SubClassID());
}
// <end IServiceClient interface>

/// @}


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
   RuntimeClient  runtimeClient;
   Sudoku theApp(&runtimeClient, argv[1]);

   if(!runtimeClient.isOK()){
      ERR("Runtime Failed to Start");
      exit(1);
   }
   btInt Result = theApp.run();

   MSG("Done");
   return Result;
}

