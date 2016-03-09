// Copyright (c) 2012-2014, Intel Corporation
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

/// @defgroup CCIDemo CCI Demo
/// @ingroup Samples
//****************************************************************************
/// @file mb_2_3.cpp
/// AUTHOR: Tim Whisonant, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:       WHAT:
/// 02/24/2012     TSW        Original version.
/// 04/18/2014     HM         Remove NLB Versioning@endverbatim
/// 11/20/2014     NR	      Modified NLB to MicroBenchmarks 2&3  
//****************************************************************************
// #ifdef HAVE_CONFIG_H
// # include <config.h>
// #endif /* HAVE_CONFIG_H */
// #include <stdio.h>

// #ifdef STDC_HEADERS
// # include <stdlib.h>
// # include <stddef.h>
// #else
// # ifdef HAVE_STDLIB_H
// #    include <stdlib.h>
// # else
// #    error Required system header stdlib.h not found.
// # endif /* HAVE_STDLIB_H */
// #endif /* STDC_HEADERS */

// ##ifdef HAVE_STRING_H
// //#if !defined( STDC_HEADERS ) && defined( HAVE_MEMORY_H )
// //#include <memory.h>
// //#endif /* !STDC_HEADERS && HAVE_MEMORY_H */
//  include <string.h>
// #endif /* HAVE_STRING_H */

// #ifdef HAVE_UNISTD_H
// # include <unistd.h>
// #endif /* HAVE_UNISTD_H */

// #include <strings.h> // strcasecmp

// #include <iostream>
// #include <iomanip>

// #include <aalsdk/ccilib/CCILib.h>
// #include <aalsdk/aalclp/aalclp.h>

// USING_NAMESPACE(std)
// USING_NAMESPACE(AAL)
// USING_NAMESPACE(CCILib)

#define ASE_ENABLING

#include <aalsdk/aalclp/aalclp.h>
#include <strings.h> // strcasecmp

using namespace std;

#include "CCILib.h"
using namespace AAL;
using namespace CCILib;

#ifdef ASE_ENABLING
#include <aalsdk/ase/ase_common.h>
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

#define CACHELINE_ALIGNED_ADDR(p)  ((p) >> LOG2_CL)
#include <pthread.h>
#include <math.h>

//
// Test capabilities
// -----------------
// The Microbenchmark comprises of 2 individual test modules : read test and  SW test
// 1. The read test is overlapped with one or more instances of SW test(see NUM_SW_INSTANCE below).
// 2. It is possible to enable/disable the SW test and the read test(see SW_TEST_DISABLE and SEL_TEST_DISABLE below).
// 3. The read test performs read streaming. It can be configured in normal mode or cont mode(see RD_CONT below).
// 4. In normal mode : No. of reads  = No. of cache lines set for read test(see RD_LINES below).
// 	  In cont mode   : After reading RD_LINES the test starts again. Such a test never terminates.
// 5. When both SW test and read test are enabled, the microbenchmark ends upon completion of all instances [1 to 31] of SW test.
// 6. The SW test does back and forth data movement, flag write (to denote availability of data) between CPU and FPGA.
// 7. Each instance of SW test does the following: (Let's call this one iteration)
// 	   	*) FPGA writes N cache lines [0 to 2047](see SW_LINES below), followed by a flag. 
// 		*) CPU which is polling on the flag copies N lines to a buffer and notifies the FPGA.
// 		*) Three possible notification schemes: 	(one of these chosen using command line input: --notice_type= [0/2/3])
//											  1. Writing a Flag to a location where AFU is polling,
// 											  2. Send an UMsg with Data,
// 											  3. Send an UMsgH followed by UMsg with Data.	
//		*) When notified, FPGA reads N cache lines [0 to 2047].
// 8. It is possible to choose the number of iterations done by each instance of SW test. (See NUM_SW_REPEAT below)
//
// BM3: (Measures Latency of Data Transfer)
// ----------------------------------------
// Read Test  parameters:
// - Test disabled
// SW Test parameters:
// - Test Enabled
// - NUM_SW_REPEAT = 2047, 
// - Vary SW_LINES, measure the average latency for 1 iteration.
//
// BM2: (Measures Impact of Polling on read streaming Bandwidth)
// -----------------------------------------------------------
// Read Test  parameters:
// - Test enabled
// - set in cont mode
// SW Test parameters:
// - Test Enabled
// - NUM_SW_REPEAT = 2047, 
// - Increase the NUM_SW_INSTANCE and analyze the impact
// - Compare the impact of BM3 thread on Read Bandwidth of read test while using polling, UMsg, UMsgH Schemes 
//
//---------------------------------USER CONTROL PARAMETERS-----------------------------------------------------------------------
#define NUM_SW_REPEAT		     16	//No. of times the SW test has to be repeated	[1 to 2047]
#define NUM_SW_INSTANCE		     2			//No. of Instances (threads) of SW test			[1 to 31]									
#define SW_TEST_DISABLE		     0			//Set this to 1 to disable SW test				[0 or 1]
#define SEL_TEST_DISABLE	     0x40000000	//0x80000000 - disables SW test					
											//0x40000000 - disables read test
											//0xc0000000 - disables both tests
											//0x00000000 - enables both tests
#define SW_LINES	     	     0			//No. of cache lines for SW test				[0 to 2047]
#define RD_LINES		     	 5		//No. of cache lines for read test				[1 to 65536]
#define RD_CONT			     	 0x00000002	//0x00000002 - Rd test cont mode, test completes when sw test completes
											//0x00000000 - Normal mode - Only one iteration of Read test.														
#define TIMEOUT			     	 20
//-------------------------------------------------------------------------------------------------------------------------------

#define NUM_THREADS		 		 NUM_SW_INSTANCE
#define NUM_XEON_THREADS 			 1
#define SW1_BUFFER_SIZE          CL(2048)*32	//Allocate 4MB Buffer for sw test
#define RD_BUFFER_SIZE           CL(65536)		//Allocate 4MB Buffer for Rd streaming test
#define SW_CONTROL 				(NUM_SW_REPEAT << 6) | (NUM_SW_INSTANCE)

#define SW1_DSM_SIZE             MB(4)
#define NUM_UMSGS 		 		 32
#define UMSG_BUFFER_SIZE         CL(64*NUM_UMSGS)

// CSRs 
#define CSR_CIPUCTL              0x0280
#define CSR_AFU_DSM_BASEL        0x1a00
#define CSR_AFU_DSM_BASEH        0x1a04
#define CSR_SRC_ADDR             0x1a20
#define CSR_DST_ADDR             0x1a24
#define CSR_NUM_LINES            0x1a28
#define CSR_CTL                  0x1a2c
#define CSR_CFG                  0x1a34
#define CSR_SRC_ADDR_SW          0x1a40
#define CSR_DST_ADDR_SW          0x1a44
#define CSR_NUM_LINES_SW         0x1a48
#define CSR_SW_CTL		   		 0x1a4c
#define CSR_SW1NOTICE            0x1B00
#define CSR_UMSGBASE             0x03F4
#define CSR_UMSGMODE             0x03F8
#define CSR_CIRBSTAT             0x0278
#define CSR_OFFSET(x)            ((x) / sizeof(bt32bitCSR))

#define DSM_STATUS_TEST_COMPLETE 0x40
#define DSM_STATUS_TEST_ERROR    0x44
#define DSM_STATUS_MODE_ERROR_0  0x60
#define DSM_STATUS_ERROR_REGS    16

// CSR for Cache Counters
#define CSR_PERF1C                 0x27c
#define CSR_PERF1                  0x28c
// Number of locks
#define NUM_LOCKS 65536

// Start and Stride Pattern
#define START_OF_STRIDE_XEON 1
#define START_OF_STRIDE_FPGA 1
#define STRIDE_LENGTH 1

//
#define  NUM_CLINE_THREAD 2048
#define  NUM_BUCKET_PER_CACHE 8
#define  TOTAL_BUCKET_PER_THREAD (NUM_CLINE_THREAD*NUM_BUCKET_PER_CACHE)

//Define Cache Flush Size
#define CACHE_FLUSH_SIZE 100*1024*1024

//Histogram Parameters
#define BUCKETS 1024*1024

//-------------------------------------------------
BEGIN_C_DECLS
struct UMsgCmdLine
{
   btUIntPtr               flags;
#define CCIDEMO_CMD_FLAG_HELP    0x00000001
#define CCIDEMO_CMD_FLAG_VERSION 0x00000002

   CCIDeviceImplementation target;
   int                     log;
   int                     trace;
   int                     notice_type;
   int                     read_type;
};

struct UMsgCmdLine gUMsgCmdLine =
{
   0,
   CCI_NULL,
   0,
   0, 
   2, // notice_type
   0
};
// ------------------------------------------------

int ccidemo_on_nix_long_option_only(AALCLP_USER_DEFINED , const char * );
int ccidemo_on_nix_long_option(AALCLP_USER_DEFINED , const char * , const char * );

aalclp_option_only ccidemo_nix_long_option_only = { ccidemo_on_nix_long_option_only, };
aalclp_option      ccidemo_nix_long_option      = { ccidemo_on_nix_long_option,      };

void help_msg_callback(FILE * , struct _aalclp_gcs_compliance_data * );
void showhelp(FILE * , struct _aalclp_gcs_compliance_data * );

AALCLP_DECLARE_GCS_COMPLIANT(stdout,
                             "Histogram",
                             CCILIB_VERSION,
                             "",
                             help_msg_callback,
                             &gUMsgCmdLine)

int parsecmds(struct UMsgCmdLine * , int , char *[] );
int verifycmds(struct UMsgCmdLine * );
END_C_DECLS
////////////////////////////////////////////////////////////////////////////////
//define lock variable
pthread_mutex_t lock[NUM_LOCKS];
// Lock Methods
void Fetch_Lock(pthread_mutex_t lock)
{
	pthread_mutex_lock(& lock);
}
void Release_Lock(pthread_mutex_t lock)
{
	pthread_mutex_unlock(& lock);

}

struct ThreadData
{
	long threadid;
	long randomNumber[NUM_SW_REPEAT];
};

volatile btVirtAddr pUMsgUsrVirt;    
volatile btVirtAddr pInputUsrVirt_sw;       
volatile btVirtAddr pOutputUsrVirt_sw;    
ICCIDevice *pCCIDevice; 
int timeout = 0;
int ase_flag= 0;
long bucketMem[BUCKETS];
long size;
void *CPU_Poll_Write(void *threadData)
{
   time_t startTime1 = time(NULL);
   long tid; int k;
   ThreadData *fpga_thr_data = static_cast<ThreadData*>(threadData);
   tid = (long)fpga_thr_data->threadid;
   int random[NUM_SW_REPEAT];

   btUnsigned64bitInt umsg_testdata[8];
   char *umsg_testdata_chr;

   //volatile int lockbit, index;
   
   /*
   for(k=0;k<NUM_SW_REPEAT;k++)
   {
		cout<<"\n"<<fpga_thr_data->randomNumber[k];
   }*/
/*   
   // Define seed for random number
   srand(time(NULL));
   
   for(k=0;k<NUM_SW_REPEAT;k++)
   {
		random[k] = rand() % NUM_LOCKS;
		//cout<<"\n"<<random[k];
   }
  */
	
  
    for(k=0;k<NUM_SW_REPEAT;k++)
	{
	   
	   //STEP3 and 4: AFU does N writes and then (N+1)th write. CPU polling on AFU write to N+1
	   //cout<<"\nAddress of Flag:"<<&(pOutputUsrVirt_sw + CL(2047*32 + tid + 1));
	   //cout<<"\nValue of Flag:"<<*((volatile btUnsigned32bitInt *)(pOutputUsrVirt_sw + CL(2047*32 + tid + 1)));
	   //cout<<"\nVirtual Address:"<<(void *)pOutputUsrVirt_sw;
	   //cout<<"\nData Passed by FPGA:"<<(void *)(pOutputUsrVirt_sw+ CL(2047*32 + tid + 1));
	   //cout<<"\nSize of Bucket:"<<sizeof(bucketMem);
	   //cout<<"\nSize of Long "<<sizeof(size);
	   volatile int lockbit, index;
	   //cout<<"\n"<<"["<< tid<<"]Data Passed by FPGA:"<<*((volatile btUnsigned32bitInt *)(pOutputUsrVirt_sw + CL(2047*32 + tid + 1)));
	   //cout<<"\n K:"<<k;
	   //cout<<"\n Lock Bit:"<<(*((volatile btUnsigned32bitInt *)(pOutputUsrVirt_sw + CL(2047*32 + tid + 1))) & 1);
	   lockbit 	= (*((volatile btUnsigned32bitInt *)(pOutputUsrVirt_sw + CL(2047*32 + tid + 1))) & 1);
	   //cout<<"\nLock Bit:"<<lockbit;
	   index 	=  *((volatile btUnsigned32bitInt *)(pOutputUsrVirt_sw + CL(2047*32 + tid + 1)));
	   //cout<<"\nIndex:"<<index;
	   while(*((volatile btUnsigned32bitInt *)(pOutputUsrVirt_sw + CL(2047*32 + tid + 1)))!= k+1)
	   {
		  if(tid!=0)
		  //cout<<"\nRead Polling!!";
		  if((time(NULL) - startTime1 > TIMEOUT) && ase_flag==0) 
		   { 
		   timeout++ ; 
		   break;
		   }
		   
		   else
		   continue;
		   
	   }
		
		
	   //STEP5: CPU does some work (memcpy N lines) before writing flag (N+1)th line to AFU 
	   //memcpy((void *)(pInputUsrVirt_sw + CL(tid*SW_LINES)),(void *)(pOutputUsrVirt_sw+ CL(tid*SW_LINES)), CL(SW_LINES));    
	   //usleep(100);
	   //if (lockbit == 0)
	   if(k%2 == 0)
	   {
	     //cout<<"\nFetch Lock["<<random[k] <<"] For FPGA:"<<k<<endl;
	     //cout<<"\n Index:"<<index+tid;
	     //cout<<"\n"<<"["<<tid<<"]Lock Access Pattern for FPGA:"<<fpga_thr_data->randomNumber[k];
	     pthread_mutex_lock(& lock[(fpga_thr_data->randomNumber[k])>>3]);
	     //cout<<"\n"<<"["<<tid<<"] got Lock";
		
	     ///*
		
	     if	(gUMsgCmdLine.notice_type==0)
	       *(btUnsigned64bitInt *) (pInputUsrVirt_sw + CL(2047*32 + tid + 1)) = (((fpga_thr_data->randomNumber[index]%TOTAL_BUCKET_PER_THREAD)<<35) | (k+1)<<3 | 1);	 //k+1;
	     else 
	       {				 		     
#ifdef ASE_ENABLING
		 umsg_testdata[0] = (((fpga_thr_data->randomNumber[index]%TOTAL_BUCKET_PER_THREAD)<<35) | (k+1)<<3 | 1);
		 umsg_testdata_chr = (char*) &umsg_testdata;
		 umsg_send (tid, umsg_testdata_chr);
#else
		 *(volatile btUnsigned32bitInt *) (pUMsgUsrVirt + CL(tid*64+ tid)) = k+1; 
#endif
			  }
	     __sync_synchronize();
	   //*/
	   }
	   else
	   {
		//cout<<"\nRelease Lock["<<random[k] <<"] For FPGA:"<<k<<endl;
		//cout<<"\nRandom Number passed CHECK!!:"<<fpga_thr_data->randomNumber[index];
		//cout<<"\n Index CHECK!!:"<<index-1;
		pthread_mutex_unlock(& lock[(fpga_thr_data->randomNumber[k-1]>>3)]);
		//cout<<"\n"<<"["<<tid<<"] Releases Lock";
		if	(gUMsgCmdLine.notice_type==0)
		  *(btUnsigned64bitInt *) (pInputUsrVirt_sw + CL(2047*32 + tid + 1)) = (((fpga_thr_data->randomNumber[index-1]%TOTAL_BUCKET_PER_THREAD)<<35) | (k+1)<<3 | 0);	
		else
#ifdef ASE_ENABLING
		 umsg_testdata[0] = (((fpga_thr_data->randomNumber[index-1]%TOTAL_BUCKET_PER_THREAD)<<35) | (k+1)<<3 | 0);	
		 umsg_testdata_chr = (char*) &umsg_testdata;
		 umsg_send (tid, umsg_testdata_chr);		  
#else
		  *(volatile btUnsigned32bitInt *) (pUMsgUsrVirt + CL(tid*64+ tid)) = k+1;
#endif
	   __sync_synchronize();
	   
	   }
       	   //Fence Operation to ensure Flag is written after N writes
	   //__sync_synchronize();	   
	   
		
	   //STEP6:CPU notifying AFU (write to flag)
	   //if	(gUMsgCmdLine.notice_type==0)*(btUnsigned32bitInt *) (pInputUsrVirt_sw + CL(2047*32 + tid + 1)) = k+1;	
	   //else 				 		     *(volatile btUnsigned32bitInt *) (pUMsgUsrVirt + CL(tid*64+ tid)) = k+1; 
	}
    pthread_exit(NULL);
} 

void *XEON_COMPUTE(void *threadData)
{
	// This will mimic the CPU thread functionality
	//TO DO:1) Call the lock method 
	//2) Sleep for sometime to mimic some compute
	//3) Release the lock by calling the lock method
	
	//cout<<"\nStarting Xeon Threads..."<<endl;
	long tid; int k;
	ThreadData *xeon_thr_data = static_cast<ThreadData*>(threadData);
	tid = (long)xeon_thr_data->threadid;

	//int k;
	//int random[NUM_SW_REPEAT];
	// Define a seed for random number
	//srand(time(NULL));
	/*
	for(k=0;k<NUM_SW_REPEAT;k++)
	{
	random[k] = rand() % NUM_LOCKS;
	//cout<<"\n"<<random[k];
	}*/
	/*int test=0;
	for(k=0;k<1000000000000;k++)
	{
		test++;
	}*/
	// Define the pointers for the Histogram Workspace
	volatile btUnsigned64bitInt *pInput_sw    		= (volatile btUnsigned64bitInt *)pInputUsrVirt_sw;
	volatile btUnsigned64bitInt *pInput_sw_lock_base		=(volatile btUnsigned64bitInt *)pInputUsrVirt_sw +8;
	volatile btUnsigned64bitInt *pInput_sw_lock = 0;
	//= (volatile btUnsigned64bitInt *)pInputUsrVirt_sw + 8+ lockid;
	//cout<<"\nAdded Number:"<<xeon_thr_data->randomNumber[0];
	//pInput_sw_lock = pInput_sw_lock+xeon_thr_data->randomNumber[0];
	//*pInput_sw_lock = *pInput_sw_lock+1;
	for(k=0;k<NUM_SW_REPEAT;k++)
	{
		if(k%2==0)
		{
		 //cout<<"\nLock access pattern for xeon:"<<xeon_thr_data->randomNumber[k];
		 // Call Fetch lock method
		 pthread_mutex_lock(& lock[(xeon_thr_data->randomNumber[k])%NUM_LOCKS]);
		 pInput_sw_lock = pInput_sw_lock_base + xeon_thr_data->randomNumber[k];
		 //cout<<"\nXeon Access Address"<<&pInput_sw_lock;
		 *pInput_sw_lock = *pInput_sw_lock+1;
		 
		 __sync_synchronize();
		 
		}
		else
		{
		// cout<<"\nReleasing lock["<<random[k] <<"] for Xeon"<<k<<endl;
		 // Call Release Lock Method
		 pthread_mutex_unlock(& lock[(xeon_thr_data->randomNumber[k-1])%NUM_LOCKS]);
		}
	}
}

int main(int argc, char *argv[])
{
   if ( argc < 2 ) {
      showhelp(stdout, &_aalclp_gcs_data);
      return 1;
   } else if ( parsecmds(&gUMsgCmdLine, argc, argv) ) {
      cerr << "Error scanning command line." << endl;
      return 2;
   } else if ( flag_is_set(gUMsgCmdLine.flags, CCIDEMO_CMD_FLAG_HELP|CCIDEMO_CMD_FLAG_VERSION) ) {
      return 0; // per GCS
   } else if ( verifycmds(&gUMsgCmdLine) ) {
      return 3;
   }

   // Select the desired CCI Device Implementation. Valid choices are..
   //
   //     CCI_AAL    - Run against the full AAL stack.
   //                - Requires that the AAL drivers be loaded.
   //                  eg
   //                     cd rootdir/drvbin
   //                     sudo ./insdrv
   //
   //     CCI_DIRECT - Run against the direct driver.
   //
   //     CCI_ASE    - Run against the AFU Simulation Environment.
   //
   
   const CCIDeviceImplementation CCIDevImpl = gUMsgCmdLine.target;
 
   // Obtain a 'Factory' object matching the chosen Device Implementation.
   // The Factory object is used to create the specific underlying device instance.
   ICCIDeviceFactory *pCCIDevFactory = GetCCIDeviceFactory(CCIDevImpl);
   
   // Use the implementation-specific Device Factory to create a device.
   
   pCCIDevice = pCCIDevFactory->CreateCCIDevice();
#if (1 == ENABLE_DEBUG)
   pCCIDevice->GetSynchronizer()->SetLogLevel(gUMsgCmdLine.log);
   pCCIDevice->GetSynchronizer()->SetTraceLevel(gUMsgCmdLine.trace);
#endif // ENABLE_DEBUG
   //
   // Use the CCI Device to create Workspaces (buffers).
   //
   // 1.) A workspace to serve as the DSM buffer for FPGA -> Host communication.
   // 2.) A data input buffer for Rd Test
   // 3.) A data input buffer for Sw Test
   // 4.) A data output buffer for Sw Test
   // 5.) A UMsg buffer
   //
    
   ICCIWorkspace *pDSMWorkspace    		= pCCIDevice->AllocateWorkspace(SW1_DSM_SIZE);
   ICCIWorkspace *pInputWorkspace  		= pCCIDevice->AllocateWorkspace(RD_BUFFER_SIZE);  
   ICCIWorkspace *pInputWorkspace_sw  	= pCCIDevice->AllocateWorkspace(SW1_BUFFER_SIZE);
   ICCIWorkspace *pOutputWorkspace_sw 	= pCCIDevice->AllocateWorkspace(SW1_BUFFER_SIZE); 
#ifdef ASE_ENABLING
   umas_init ( 0xFFFFFFFF );
#else
   ICCIWorkspace *pUMsgWorkspace   		= pCCIDevice->AllocateWorkspace(UMSG_BUFFER_SIZE);
#endif

   // We need to initialize the input and output buffers, so we need addresses suitable
   // for dereferencing in user address space.
   // volatile, because the FPGA will be updating the buffers, too.
   volatile btVirtAddr pInputUsrVirt      = pInputWorkspace->GetUserVirtualAddress(); 
  		               pInputUsrVirt_sw   = pInputWorkspace_sw->GetUserVirtualAddress();
   
   const    btUnsigned32bitInt  InputData_1 = 0xdeadbeef;
   const    btUnsigned32bitInt  InputData_2 = 0x00000000;
   volatile btUnsigned32bitInt *pInput    = (volatile btUnsigned32bitInt *)pInputUsrVirt;
   volatile btUnsigned32bitInt *pEndInput = (volatile btUnsigned32bitInt *)pInput + (pInputWorkspace->GetSizeInBytes() / sizeof(btUnsigned32bitInt));

   for ( ; pInput < pEndInput ; ++pInput ) 
   {
      *pInput = InputData_1;
   }
   
   long lockid 										= 1;
   volatile btUnsigned32bitInt *pInput_sw    		= (volatile btUnsigned32bitInt *)pInputUsrVirt_sw;
   volatile btUnsigned32bitInt *pEndInput_sw 		= (volatile btUnsigned32bitInt *)pInput_sw +(pInputWorkspace_sw->GetSizeInBytes() / sizeof(btUnsigned32bitInt));

   for ( ; pInput_sw < pEndInput_sw ; ++pInput_sw ) 
   {
      *pInput_sw = InputData_2;
	  
   }
   
     
   pOutputUsrVirt_sw = pOutputWorkspace_sw->GetUserVirtualAddress();
   
   memset((void *)pOutputUsrVirt_sw, 0, pOutputWorkspace_sw->GetSizeInBytes());
   
#ifndef ASE_ENABLING
   pUMsgUsrVirt   = pUMsgWorkspace->GetUserVirtualAddress();
   memset((void *)pUMsgUsrVirt, 0, pUMsgWorkspace->GetSizeInBytes());
#endif

   volatile btVirtAddr pDSMUsrVirt  	   = pDSMWorkspace->GetUserVirtualAddress();
   volatile btUnsigned64bitInt pDSMUsrPhy  = pDSMWorkspace->GetPhysicalAddress();
   
   bt32bitCSR i;
   bt32bitCSR csr;

   // Clear the DSM
   memset((void *)pDSMUsrVirt, 0, pDSMWorkspace->GetSizeInBytes());

   // Assert CAFU Reset
   csr = 0;
   pCCIDevice->GetCSR(CSR_CIPUCTL, &csr);
   csr |= 0x01000000;
   pCCIDevice->SetCSR(CSR_CIPUCTL, csr);

   // De-assert CAFU Reset
   csr = 0;
   pCCIDevice->GetCSR(CSR_CIPUCTL, &csr);
   csr &= ~0x01000000;
   pCCIDevice->SetCSR(CSR_CIPUCTL, csr);

   // Set DSM base, high then low
   pCCIDevice->SetCSR(CSR_AFU_DSM_BASEH, pDSMWorkspace->GetPhysicalAddress() >> 32);
   pCCIDevice->SetCSR(CSR_AFU_DSM_BASEL, pDSMWorkspace->GetPhysicalAddress() & 0xffffffff);

   // Poll for AFU ID
   do
   {
    csr = *(volatile btUnsigned32bitInt *)pDSMUsrVirt;
   }while( 0 == csr );

   // Print the AFU ID
   cout << "AFU ID=";
   for ( i = 0 ; i < 4 ; ++i ) {
      cout << std::setw(8) << std::hex << std::setfill('0')
           << *(btUnsigned32bitInt *)(pDSMUsrVirt + (3 - i) * sizeof(btUnsigned32bitInt));
   }
   cout << endl;

   // Assert Device Reset
   pCCIDevice->SetCSR(CSR_CTL, 0);

   // Clear the DSM
   memset((void *)pDSMUsrVirt, 0, pDSMWorkspace->GetSizeInBytes());

   // De-assert Device Reset
   pCCIDevice->SetCSR(CSR_CTL, 1);

   // Set the number of cache lines for the sw test
   pCCIDevice->SetCSR(CSR_NUM_LINES_SW, SW_LINES);
   
   // Set Control Parameters for sw test (No. of instances and No. of times each instance has to run)
   pCCIDevice->SetCSR(CSR_SW_CTL,SW_CONTROL);
   
   // Set input workspace address for sw test
   pCCIDevice->SetCSR(CSR_SRC_ADDR_SW, CACHELINE_ALIGNED_ADDR(pInputWorkspace_sw->GetPhysicalAddress())+1);
   //cout<<"\nHistogram Workspace:"<<CACHELINE_ALIGNED_ADDR(pInputWorkspace_sw->GetPhysicalAddress());
   //cout<<"\nHistogram Workspace Virtual Address:"<<pInputWorkspace_sw->GetUserVirtualAddress();
     
   // Set output workspace address for sw test
   pCCIDevice->SetCSR(CSR_DST_ADDR_SW, CACHELINE_ALIGNED_ADDR(pOutputWorkspace_sw->GetPhysicalAddress())+1);
   
   // Set input workspace address for rd test 
   pCCIDevice->SetCSR(CSR_SRC_ADDR, CACHELINE_ALIGNED_ADDR(pInputWorkspace->GetPhysicalAddress()));
   
   // Set the number of cache lines for rd test
   pCCIDevice->SetCSR(CSR_NUM_LINES, RD_LINES);
      

#ifndef ASE_ENABLING
   //PM: UMsg Base
   if(gUMsgCmdLine.notice_type==2 || gUMsgCmdLine.notice_type==3)
   {
   	cout<<"\nConfiguring UMsg CSRs";
    	
    // UMSGBASE
    btUnsigned32bitInt umsg_base_cl = CACHELINE_ALIGNED_ADDR(pUMsgWorkspace->GetPhysicalAddress()) & 0xfffffffc;
   	btUnsigned32bitInt umsgbase_csr_val = umsg_base_cl | 0x1; 	// Enable Umsgs
   	pCCIDevice->SetCSR(CSR_UMSGBASE, umsgbase_csr_val);
	cout<<"\numsg base csr "<<umsgbase_csr_val;
   	
    // UMSGMODE
    btUnsigned32bitInt umsgmode_csr_val;
   	if(gUMsgCmdLine.notice_type==3)
   	{
   	     umsgmode_csr_val = 0xffffffff;							// All 32 UMsgs in Hint Mode
   	}
   	else
   	{
   	     umsgmode_csr_val = 0;									// All 32 UMsgs in Data Mode
   	}
   	pCCIDevice->SetCSR(CSR_UMSGMODE, umsgmode_csr_val);
	cout<<"\numsg mode csr "<<umsgmode_csr_val<<endl<<flush;
   
	// Poll on CIRBSTAT[0]==1 to Check if UMsg has completed Initialization 
    btUnsigned32bitInt umsg_status = 0x00000000;
	do
    {
    pCCIDevice->GetCSR(CSR_CIRBSTAT, &umsg_status);
    }
    while((umsg_status & 0x1) == 0);
    cout<<"\nUMsg Init Done";
	cout<<"\nCIRBSTAT = "<< umsg_status <<endl<<flush;               
   }
#endif

   // Set the test mode
   /*	0 - RdLine_S	: Cached in Shared state
	1 - RdLine_I	: Not cached. Marked as Invalid. However it may cause cache pollution
	2 - RdLine_O	: Cached in Owned State 
   */
   
   btUnsigned32bitInt csr_read_type= 0;
   if(gUMsgCmdLine.read_type==1)
   { 
	csr_read_type = 1<<9;
   } else if (gUMsgCmdLine.read_type==2)
   {
	csr_read_type = 2<<9;
   }

   btUnsigned32bitInt test_mode_csr_val = gUMsgCmdLine.notice_type<<26 | 0x7<<2 |  csr_read_type | SEL_TEST_DISABLE | RD_CONT;
   pCCIDevice->SetCSR(CSR_CFG,test_mode_csr_val);

   volatile bt32bitCSR *StatusAddr   = (volatile bt32bitCSR *) (pDSMUsrVirt  + DSM_STATUS_TEST_COMPLETE);	
   volatile btUnsigned64bitInt StatusAddr_phy = (pDSMUsrPhy  + DSM_STATUS_TEST_COMPLETE);

  
        // Cache Read counters 																
        btUnsigned32bitInt csr_val;
        btUnsigned32bitInt cache_read_hit;

        pCCIDevice->SetCSR(CSR_PERF1C,0x00000000);
        pCCIDevice->GetCSR(CSR_PERF1,&csr_val);
        cache_read_hit=csr_val;

        pCCIDevice->SetCSR(CSR_PERF1C,0x80000000);
        pCCIDevice->GetCSR(CSR_PERF1,&csr_val);
        cache_read_hit+=csr_val;
        cout <<"\ncache_read_hits START\t\t: "<<dec<<cache_read_hit;

        btUnsigned32bitInt csr_val_1;
        btUnsigned32bitInt cache_read_miss;

        pCCIDevice->SetCSR(CSR_PERF1C,0x00000002);
        pCCIDevice->GetCSR(CSR_PERF1,&csr_val_1);
        cache_read_miss=csr_val_1;

        pCCIDevice->SetCSR(CSR_PERF1C,0x80000002);
        pCCIDevice->GetCSR(CSR_PERF1,&csr_val_1);
        cache_read_miss+=csr_val_1;
        cout <<"\ncache_read_miss START\t\t: "<<dec<<cache_read_miss;
  
	// Cache Wtite counters
        btUnsigned32bitInt csr_val_wr;
        btUnsigned32bitInt cache_write_hit;

        pCCIDevice->SetCSR(CSR_PERF1C,0x00000001);
        pCCIDevice->GetCSR(CSR_PERF1,&csr_val_wr);
        cache_write_hit=csr_val_wr;

        pCCIDevice->SetCSR(CSR_PERF1C,0x80000001);
        pCCIDevice->GetCSR(CSR_PERF1,&csr_val_wr);
        cache_write_hit+=csr_val_wr;
        cout <<"\ncache_write_hits START\t\t: "<<dec<<cache_write_hit;

        btUnsigned32bitInt csr_val_wr1;
        btUnsigned32bitInt cache_write_miss;

        pCCIDevice->SetCSR(CSR_PERF1C,0x00000003);
        pCCIDevice->GetCSR(CSR_PERF1,&csr_val_wr1);
        cache_write_miss=csr_val_wr1;

        pCCIDevice->SetCSR(CSR_PERF1C,0x80000003);
        pCCIDevice->GetCSR(CSR_PERF1,&csr_val_wr1);
       	cache_write_miss+=csr_val_wr1;
        cout <<"\ncache_write_miss START\t\t: "<<dec<<cache_write_miss;	

		
		cout << "\nFlushing Cache\n";
		// Flush out the contents of the cache 
		const int size_d = 100*1024*1024; 		// Allocate 100M. 
		char *c = (char *)malloc(size_d);
		for (int i1 = 0; i1 < 1; i1++)		
		{
			for (int j1 = 0; j1 < size_d; j1++)
				c[j1] = i1*j1;
		}		
	
							
    // Launch Threads, Start the test and wait for threads to join
	pthread_t threads[NUM_THREADS];
	pthread_t xeon_threads[NUM_XEON_THREADS];
	
	//Arguments for FPGA threads
	int ec,k;
	long t;
	//int random_fpga[NUM_SW_REPEAT];
	
	ThreadData fpga_thread_data[NUM_THREADS];
	ThreadData xeon_thread_data[NUM_XEON_THREADS];
	
	// Initialze the array with random numbers
	
	// Define seed for random number
	srand(time(NULL));
    // Populate Random access pattern for FPGA Thread
	for( int i1 = 0; i1<NUM_THREADS; i1++ )
	{
		for(int j1 = 0; j1 <NUM_SW_REPEAT; j1++)
		{
			fpga_thread_data[i1].randomNumber[j1] = (START_OF_STRIDE_FPGA+j1*STRIDE_LENGTH)% CL(32);
			cout<<"\nFPGA Random Data:"<<fpga_thread_data[i1].randomNumber[j1];
		}
	}
	// Populate Random access pattern for Xeon Thread
	for(int i1 = 0; i1<NUM_XEON_THREADS; i1++)
	{
		for(int j1 = 0; j1<NUM_SW_REPEAT; j1++ )
		{
			xeon_thread_data[i1].randomNumber[j1] = (START_OF_STRIDE_XEON+j1*STRIDE_LENGTH)% CL(32);
			cout<<"\nXeon Random Data:"<<xeon_thread_data[i1].randomNumber[j1];
		}
	}
	/*for(k=0;k<NUM_SW_REPEAT;k++)
	{
		//Initializing the Random Numbers/Stride patterns
		//cout<<"\nAccess Pattern["<< k<<"]"<<(START_OF_STRIDE+k*STRIDE_LENGTH)%NUM_LOCKS;
		fpga_thread_data.randomNumber[k] = (START_OF_STRIDE_FPGA+k*STRIDE_LENGTH)% NUM_LOCKS;
		xeon_thread_data.randomNumber[k] = (START_OF_STRIDE_XEON+k*STRIDE_LENGTH)% NUM_LOCKS;
		//cout<<"\nThis is before passing";
		//cout<<"\n"<<fpga_thread_data.randomNumber[k];
	}*/
	
	//Arguments for Xeon Thread
	int ec_xeon; 
	long t_xeon;
	//int random_xeon[NUM_SW_REPEAT];
	   
	if(SW_TEST_DISABLE == 0)
	{
		//STEP1: Spawn Threads
		 
		  for(t=0;t<NUM_THREADS;t++)
		   {		
			 fpga_thread_data[t].threadid = t;
			 ec = pthread_create(&threads[t], NULL, CPU_Poll_Write, (void *)&fpga_thread_data[t]);
			 if (ec)  exit(-1);
		   }
		  // STEP1.1 SPAWN Xeon Thread 
		  for(t_xeon=0;t_xeon<NUM_XEON_THREADS;t_xeon++)
		  {
			ec_xeon = pthread_create(&xeon_threads[t_xeon],NULL, XEON_COMPUTE,(void *)&xeon_thread_data[t_xeon]);
			if	(ec_xeon) exit(-1);
		  }

		//STEP2: Start the test
		  
		  pCCIDevice->SetCSR(CSR_CTL,0x3);
		  if(ase_flag==1)
		  {
		   cout <<"\n\n####################################################################";
		   cout <<"\nNote:\nTime out feature Disabled in ASE.\nTest terminates only upon successful completion";
		   cout <<"\n####################################################################\n\n";
		  }
		  
		  else
		  {
		   cout <<"\n\n##################################################";
		   cout <<"\nNote:\nTime out feature Enabled.\nTest terminates on timeout/completion";
		   cout <<"\n##################################################\n\n";
		  }

		//STEP8: Join all the threads
		  for(t=0;t<NUM_THREADS;t++)
		  {
			   if(pthread_join(threads[t], NULL))
			   { 
                cout<<"\nFailed to Join thread\n";
			   } 
		  }	 
	}
	else	// If sw test is disabled, Start the Read streaming Test
	{
		pCCIDevice->SetCSR(CSR_CTL,0x3);
		if(ase_flag==1)
		{
		 cout <<"\n\n####################################################################";
		 cout <<"\nNote:\nTime out feature Disabled in ASE.\nTest terminates only upon successful completion";
		 cout <<"\n####################################################################\n\n";
		}

		else
		{
		 cout <<"\n\n##################################################";
		 cout <<"\nNote:\nTime out feature Enabled.\nTest terminates on timeout/completion";
		 cout <<"\n##################################################\n\n";
		}
	}
   	
    //CPU polls on test completion
     cout<<"\nWaiting for Overall Test completion";  
     cout.flush();
	
	 time_t startTime = time(NULL); // return current time in seconds 
	 while( 0 == *StatusAddr && timeout==0) 
     {
		if((time(NULL) - startTime > TIMEOUT) && ase_flag==0)
       	{
		 timeout++;
		 break;
		}
		else
		continue;    
     }

	   bool bPassed;	 
	   if(timeout==0)
	   {   
	   bPassed= true;
	   cout<<"\nReceived Test completion signal ";     
	   }
   
   // Disable UMsgs upon Test completion
#ifndef ASE_ENABLING
   pCCIDevice->SetCSR(CSR_UMSGBASE, 0x00000000);
#endif
   
   // Verify the device
   if ( (*(StatusAddr + CSR_OFFSET(DSM_STATUS_TEST_ERROR - DSM_STATUS_TEST_COMPLETE)) != 0) || (timeout!=0) ) 
   {
      bPassed = false;
	  
	  if(timeout!=0)
	  {
		  cout <<"\nTest Failed to Complete before timeout\n" ;
	  }
	  
	  else
	  {
		  cout << *(StatusAddr + CSR_OFFSET(DSM_STATUS_TEST_ERROR - DSM_STATUS_TEST_COMPLETE))
			   << " Device Errors" << endl;

		  ios::fmtflags f = cout.flags();
		  cout.flags(ios_base::right);
		  cout.fill('0');

		  for ( i = 0 ; i < DSM_STATUS_ERROR_REGS ; ++i ) 
		  {
			 cout << "Error Status[" << dec << i << "] = 0x"
					  << hex << setw(8) <<
						 *(StatusAddr + CSR_OFFSET(DSM_STATUS_MODE_ERROR_0 - DSM_STATUS_TEST_COMPLETE) + i)
					  << endl;
		  }

		  cout.fill(' ');
		  cout.flags(f);
	  }
   }
   cout << endl;
   cout << (bPassed ? "PASS" : "ERROR") << endl << flush;
     
  

   // PM: Print status information
 if(bPassed)
  {
        btUnsigned64bitInt num_ticks = *(StatusAddr + 3);       // high
        num_ticks = (num_ticks << 32) | *(StatusAddr + 2);      // high<<32 | low
        int penalty_start = *(StatusAddr + 6);
        int penalty_end = *(StatusAddr + 7);
        btUnsigned64bitInt latency = (num_ticks>=1) ? num_ticks-penalty_start-penalty_end : 0;
	
        cout <<"\nNum_Writes\t\t\t: "<<dec<<*(StatusAddr + 5);
        cout <<"\nTotal Number of Reads\t\t: "<<*(StatusAddr + 9);
        cout <<"\nNum_Reads from rd test\t\t: "<<*(StatusAddr + 8);
        cout <<"\nNum_Reads from sw test\t\t: "<<*(StatusAddr + 4);
        cout <<"\nNum_Clks to complete sw test\t: "<<latency;
        cout <<"\nTime taken to complete sw test\t: "<<latency*5*(pow(10,-9))<<"\tsecs";
        cout <<"\nRead Bandwidth\t\t\t: "<<((*(StatusAddr + 8))/(latency*5*(pow(10,-9))))*64<<"\tbytes/sec";
        cout <<"\nWrite Bandwidth\t\t\t: "<<((*(StatusAddr + 5))/(latency*5*(pow(10,-9))))*64<<"\tbytes/sec";
  
  // Printing result
  /*for ( ; pInput_sw < pEndInput_sw ; ++pInput_sw ) 
   {
      cout<<"\nResult Data:"<<*pInput_sw;
   }*/
   volatile btUnsigned64bitInt *pInput_sw    = (volatile btUnsigned64bitInt *)pInputUsrVirt_sw+8;
   volatile btUnsigned64bitInt *pEndInput_sw = (volatile btUnsigned64bitInt *)pInput_sw +(pInputWorkspace_sw->GetSizeInBytes() / sizeof(btUnsigned64bitInt));
   volatile btUnsigned64bitInt *pInput_sw_temp = (volatile btUnsigned64bitInt *) pInput_sw +CL(16)/sizeof(btUnsigned64bitInt);
   volatile btUnsigned64bitInt *pInput_sw_temp_2= (volatile btUnsigned64bitInt *) pInput_sw +CL(2047)/sizeof(btUnsigned64bitInt);
   volatile btUnsigned64bitInt *pInput_sw_temp_3= (volatile btUnsigned64bitInt *) pInput_sw +CL(2049)/sizeof(btUnsigned64bitInt); // Sri: 3 was 2049
   cout<<"\nStart Address of Histogram Workspace:"<<*pInput_sw;
   cout<<"\nEnd Address of Histogram Workspace:"<<*pInput_sw_temp;
   cout<<"\nSize of Histogram Workspace:"<<pInputWorkspace_sw->GetSizeInBytes();
   cout<<"\nSize of data in Bytes:"<<sizeof(btUnsigned64bitInt);
   for ( ; pInput_sw < pInput_sw_temp ; ++pInput_sw ) 
   {
      
     cout<<"\nResult Address:"<<&pInput_sw<<"\tResult Data:"<<*pInput_sw;
   }
   cout<<"\n-----------------------------------------------------------";
   for ( ; pInput_sw_temp_2 < pInput_sw_temp_3 ; ++pInput_sw_temp_2 ) 
   {
      
	  cout<<"\nResult Address:"<<&pInput_sw_temp_2<<"\tResult Data:"<<*pInput_sw_temp_2;
   }
  btUnsigned32bitInt g;
  
  for ( g =0; g< 50 ; g++ ) 
   {
      //*pInput_sw = InputData_2;
	  //cout<<"\nAddress of Flag:"<<&(pOutputUsrVirt_sw + CL(2047*32 + tid + 1));
	  //cout<<"\nAddress of Histogram Workspace:"<<&(pInput_sw +g);
	  //cout<<"\nResult Data:"<<*((volatile btUnsigned32bitInt *)(pInput_sw +CL(g)));
   }
   }
  
   
 
     // Cache Read counters
	 pCCIDevice->SetCSR(CSR_PERF1C,0x00000000);
	 pCCIDevice->GetCSR(CSR_PERF1,&csr_val);
	 cache_read_hit=csr_val - cache_read_hit;

	 pCCIDevice->SetCSR(CSR_PERF1C,0x80000000);
	 pCCIDevice->GetCSR(CSR_PERF1,&csr_val);
	 cache_read_hit+=csr_val;
	 cout <<"\ncache_read_hits END\t\t: "<<dec<<cache_read_hit;

	 pCCIDevice->SetCSR(CSR_PERF1C,0x00000002);
	 pCCIDevice->GetCSR(CSR_PERF1,&csr_val_1);
	 cache_read_miss=csr_val_1 - cache_read_miss;

	 pCCIDevice->SetCSR(CSR_PERF1C,0x80000002);
	 pCCIDevice->GetCSR(CSR_PERF1,&csr_val_1);
	 cache_read_miss+=csr_val_1;
	 cout <<"\ncache_read_miss END\t\t: "<<dec<<cache_read_miss;
	 cout <<"\nTotal cache read-transactions\t: "<< dec << cache_read_hit + cache_read_miss;
	 
	 // Cache Write Counters

	 pCCIDevice->SetCSR(CSR_PERF1C,0x00000001);
	 pCCIDevice->GetCSR(CSR_PERF1,&csr_val_wr);
	 cache_write_hit=csr_val_wr - cache_write_hit;

	 pCCIDevice->SetCSR(CSR_PERF1C,0x80000001);
	 pCCIDevice->GetCSR(CSR_PERF1,&csr_val_wr);
	 cache_write_hit+=csr_val_wr;
	 cout <<"\ncache_write_hits END\t\t: "<<dec<<cache_write_hit;

	 pCCIDevice->SetCSR(CSR_PERF1C,0x00000003);
	 pCCIDevice->GetCSR(CSR_PERF1,&csr_val_wr1);
	 cache_write_miss=csr_val_wr1 - cache_write_miss;

	 pCCIDevice->SetCSR(CSR_PERF1C,0x80000003);
	 pCCIDevice->GetCSR(CSR_PERF1,&csr_val_wr1);
	 cache_write_miss+=csr_val_wr1;
	 cout <<"\ncache_write_miss END\t\t: "<<dec<<cache_write_miss;
	 cout <<"\nTotal cache write-transactions\t: "<< dec << cache_write_hit + cache_write_miss;

 	 cout<<"\n\n";


   // Stop the device
   pCCIDevice->SetCSR(CSR_CTL,0x7);
   
   // Release the Workspaces
   pCCIDevice->FreeWorkspace(pInputWorkspace);   
   pCCIDevice->FreeWorkspace(pInputWorkspace_sw);
   pCCIDevice->FreeWorkspace(pOutputWorkspace_sw);
   pCCIDevice->FreeWorkspace(pDSMWorkspace);
#ifdef ASE_ENABLING
   umas_deinit();
#else
   pCCIDevice->FreeWorkspace(pUMsgWorkspace);
#endif

   // Release the CCI Device instance.
   pCCIDevFactory->DestroyCCIDevice(pCCIDevice);

   // Release the CCI Device Factory instance.
   delete pCCIDevFactory;
   
   return bPassed ? 0 : 1;
}


BEGIN_C_DECLS

int ccidemo_on_nix_long_option_only(AALCLP_USER_DEFINED user, const char *option)
{
   struct UMsgCmdLine *cl = (struct UMsgCmdLine *)user;

   if ( 0 == strcmp("--help", option) ) {
      flag_setf(cl->flags, CCIDEMO_CMD_FLAG_HELP);
   } else if ( 0 == strcmp("--version", option) ) {
      flag_setf(cl->flags, CCIDEMO_CMD_FLAG_VERSION);
   }

   return 0;
}

int ccidemo_on_nix_long_option(AALCLP_USER_DEFINED user, const char *option, const char *value)
{
   struct UMsgCmdLine *cl = (struct UMsgCmdLine *)user;

   if ( 0 == strcmp("--target", option) ) {
      if ( 0 == strcasecmp("aal", value) ) {
#if (1 == CCILIB_ENABLE_AAL)
         cl->target = CCI_AAL;
#else
         cout << "The version of CCILib was built without support for --target=AAL" << endl;
         return 1;
#endif // CCILIB_ENABLE_AAL
      } else if ( 0 == strcasecmp("ase", value) ) {
#if (1 == CCILIB_ENABLE_ASE)
         cl->target = CCI_ASE;
#else
         cout << "The version of CCILib was built without support for --target=ASE" << endl;
         return 2;
#endif // CCILIB_ENABLE_ASE
      } else if ( 0 == strcasecmp("direct", value) ) {
#if (1 == CCILIB_ENABLE_DIRECT)
         cl->target = CCI_DIRECT;
#else
         cout << "The version of CCILib was built without support for --target=Direct" << endl;
         return 3;
#endif // CCILIB_ENABLE_DIRECT
      } else {
         cout << "Invalid value for --target : " << value << endl;
         return 4;
      }
   } else if ( 0 == strcmp("--log", option) ) {
      char *endptr = NULL;

      cl->log = (int)strtol(value, &endptr, 0);
      if ( endptr != value + strlen(value) ) {
         cl->log = 0;
      } else if ( cl->log < 0) {
         cl->log = 0;
      } else if ( cl->log > 7) {
         cl->log = 7;
      }
   } else if ( 0 == strcmp("--trace", option) ) {
      char *endptr = NULL;

      cl->trace = (int)strtol(value, &endptr, 0);
      if ( endptr != value + strlen(value) ) {
         cl->trace = 0;
      } else if ( cl->trace < 0) {
         cl->trace = 0;
      } else if ( cl->trace > 7) {
         cl->trace = 7;
      }
   } else if( 0 == strcmp("--notice_type", option)){
      char *endptr = NULL;

      cl->notice_type = (int)strtol(value, &endptr, 0);
      if ( endptr != value + strlen(value) ) {
         cl->notice_type = 0;
      } else  {
        cout <<"\nNotice type: "<< value <<" ";
   	switch(cl->notice_type)
   	{
   	     case 0: cout <<"Polling method"; break;
   	     case 1: cout <<"CSR Write"; break;
   	     case 2: cout <<"UMsg with data"; break;
   	     case 3: cout <<"UMsgH, i.e. Hint without data"; break;
   	     default: cout <<"invalid notice type"; break;
   	}
	cout<<endl;
      }
   }else if( 0 == strcmp("--read_type", option)){
      char *endptr = NULL;

      cl->read_type = (int)strtol(value, &endptr, 0);
      if ( endptr != value + strlen(value) ) {
         cl->read_type = 0;
      } else  {
        cout <<"Read type: "<< value <<" ";
   	switch(cl->read_type)
   	{
   	     case 0: cout <<"RdLine_S"; break;
   	     case 1: cout <<"RdLine_I"; break;
   	     case 2: cout <<"RdLine_O"; break;
   	     default: cout <<"invalid read type"; break;
   	}
	cout<<endl;
      }
   }

    if ( 0 == strcmp("--target", option) ) 
    {
		if ( 0 == strcasecmp("ase", value) ) 
		{
		 ase_flag = 1;
		}
    }

   return 0;
}

void help_msg_callback(FILE *fp, struct _aalclp_gcs_compliance_data *gcs)
{
   fprintf(fp, "Usage:\n");
   fprintf(fp, "   Histogram [--target=<TARGET>]\n");
   fprintf(fp, "\n");
   fprintf(fp, "      <TARGET> = one of { ");
#if (1 == CCILIB_ENABLE_AAL)
   fprintf(fp, "AAL ");
#endif // CCILIB_ENABLE_AAL
#if (1 == CCILIB_ENABLE_ASE)
   fprintf(fp, "ASE ");
#endif // CCILIB_ENABLE_ASE
#if (1 == CCILIB_ENABLE_DIRECT)
   fprintf(fp, "Direct ");
#endif // CCILIB_ENABLE_DIRECT
   fprintf(fp, "}\n");
   fprintf(fp, "\n");
}

void showhelp(FILE *fp, struct _aalclp_gcs_compliance_data *gcs)
{
   help_msg_callback(fp, gcs);
}

int parsecmds(struct UMsgCmdLine *cl, int argc, char *argv[])
{
   int    res;
   int    clean;
   aalclp clp;

   res = aalclp_init(&clp);
   if ( 0 != res ) {
      cerr << "aalclp_init() failed : " << res << ' ' << strerror(res) << endl;
      return res;
   }

   ccidemo_nix_long_option_only.user = cl;
   aalclp_add_nix_long_option_only(&clp, &ccidemo_nix_long_option_only);

   ccidemo_nix_long_option.user = cl;
   aalclp_add_nix_long_option(&clp, &ccidemo_nix_long_option);

   res = aalclp_add_gcs_compliance(&clp);
   if ( 0 != res ) {
      cerr << "aalclp_add_gcs_compliance() failed : " << res << ' ' << strerror(res) << endl;
      goto CLEANUP;
   }

   res = aalclp_scan_argv(&clp, argc, argv);
   if ( 0 != res ) {
      cerr << "aalclp_scan_argv() failed : " << res << ' ' << strerror(res) << endl;
   }

CLEANUP:
   clean = aalclp_destroy(&clp);
   if ( 0 != clean ) {
      cerr << "aalclp_destroy() failed : " << clean << ' ' << strerror(clean) << endl;
   }

   return res;
}

int verifycmds(struct UMsgCmdLine *cl)
{
   if ( CCI_NULL == cl->target ) {
      cout << "No valid --target specified." << endl;
      return 1;
   }

   return 0;
}

void *
rpl_malloc(size_t n) {
   if ( 0 == n ) {
      n = 1;
   }
   return malloc(n);
}

END_C_DECLS


/**
@addtogroup UMsg
@{

@verbatim
This application is for example purposes only.
It is not intended to represent a model for developing commercially-deployable applications.
It is designed to show working examples of the AAL programming model and APIs.@endverbatim

This Sample demonstrates using CCILib to access the Native Loopback (NLB) VAFU.

It is intended to be a simpler illustration of writing software that interfaces
with the NLB VAFU than NLBTest, the latter of which is more suitable for hardware
validation than for illustration purposes.

The sample also illustrates the programming model differences between NLB 1.0 and NLB 1.1.

1 Principles of Operation

UMsg runs an NLB mode 0 (lpbk1) memcpy transfer for a buffer size of 1 cache line.

2 Running the application

2.1 Using the Direct Driver

@code
$ sudo /sbin/insmod fapdiag.ko qpi_aperture_phys=0xc8080000 # Jaketown -OR-
$ sudo /sbin/insmod fapdiag.ko qpi_aperture_phys=0x88080000 # Nehalem
$ sudo chmod 'a+rw' /dev/fapdiag
$ UMsg --target=Direct@endcode

2.2 Using ASE

(xterm 0)
@code
$ cd AALSDK/ase
$ make dpi CAFU=nlb
$ make sim@endcode

(xterm 1)
@code
$ UMsg --target=ASE@endcode

@} group UMsg
*/



