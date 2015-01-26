// Copyright (c) 2012-2015, Intel Corporation
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
// @file UMsgTest.cpp
// @ingroup UMsgTest
// @verbatim
// Intel(R) QuickAssist Technology Accelerator Abstraction Layer Sample Application
//
//    This application is for example purposes only.
//    It is not intended to represent a model for developing commercially-deployable applications.
//    It is designed to show working examples of the AAL programming model and APIs.
//
// AUTHOR: Henry Mitchel, Intel Corporation
//         Pratik Marolia, Intel Corporation
//
// HISTORY:
// WHEN:          WHO:       WHAT:
// 02/24/2012     TSW        Original version.
// 08/29/2014     HM/PM      Conversion from 3.3.2 to 4.x
// 12/04/2014     NR         Included timeout feature@endverbatim
//****************************************************************************
#include <strings.h> // strcasecmp
#include <iostream>
#include <iomanip>

#include "CCILib.h"
#include <aalsdk/aalclp/aalclp.h>

USING_NAMESPACE(std)
USING_NAMESPACE(AAL)
USING_NAMESPACE(CCILib)

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

#define NUM_LINES       	    	16		//Number of Cache Lines 
#define	TIMEOUT			    		10		//Timeout duration in seconds

#define SW1_BUFFER_SIZE             CL(NUM_LINES+1)
#define SW1_DSM_SIZE                MB(4)
#define NUM_UMSGS                   32
#define UMSG_BUFFER_SIZE            CL(64*NUM_UMSGS)

#define CSR_CIPUCTL                0x0280

#define CSR_AFU_DSM_BASEL          0x1a00
#define CSR_AFU_DSM_BASEH          0x1a04
#define CSR_SRC_ADDR               0x1a20
#define CSR_DST_ADDR               0x1a24
#define CSR_NUM_LINES              0x1a28
#define CSR_CTL                    0x1a2c
#define CSR_CFG                    0x1a34
#define CSR_UMSGBASE               0x03F4
#define CSR_UMSGMODE               0x03F8
#define CSR_CIRBSTAT               0x0278
#define CSR_SW1NOTICE              0x1B00

#define CSR_OFFSET(x)              ((x) / sizeof(bt32bitCSR))

#define DSM_STATUS_TEST_COMPLETE   0x40
#define DSM_STATUS_TEST_ERROR      0x44
#define DSM_STATUS_MODE_ERROR_0    0x60

#define DSM_STATUS_ERROR_REGS      8


////////////////////////////////////////////////////////////////////////////////
BEGIN_C_DECLS

struct UMsgTestCmdLine
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

struct UMsgTestCmdLine gUMsgTestCmdLine =
{
   0,
   CCI_NULL,
   0,
   0,
   0,
   0
};

int ccidemo_on_nix_long_option_only(AALCLP_USER_DEFINED , const char * );
int ccidemo_on_nix_long_option(AALCLP_USER_DEFINED , const char * , const char * );

aalclp_option_only ccidemo_nix_long_option_only = { ccidemo_on_nix_long_option_only, };
aalclp_option      ccidemo_nix_long_option      = { ccidemo_on_nix_long_option,      };

void help_msg_callback(FILE * , struct _aalclp_gcs_compliance_data * );
void showhelp(FILE * , struct _aalclp_gcs_compliance_data * );

AALCLP_DECLARE_GCS_COMPLIANT(stdout,
                             "UMsgTest",
                             CCILIB_VERSION,
                             "",
                             help_msg_callback,
                             &gUMsgTestCmdLine)

int parsecmds(struct UMsgTestCmdLine * , int , char *[] );
int verifycmds(struct UMsgTestCmdLine * );
END_C_DECLS
////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
   if ( argc < 2 ) {
      showhelp(stdout, &_aalclp_gcs_data);
      return 1;
   } else if ( parsecmds(&gUMsgTestCmdLine, argc, argv) ) {
      cerr << "Error scanning command line." << endl;
      return 2;
   } else if ( flag_is_set(gUMsgTestCmdLine.flags, CCIDEMO_CMD_FLAG_HELP|CCIDEMO_CMD_FLAG_VERSION) ) {
      return 0; // per GCS
   } else if ( verifycmds(&gUMsgTestCmdLine) ) {
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
   const CCIDeviceImplementation CCIDevImpl = gUMsgTestCmdLine.target;

   // Obtain a 'Factory' object matching the chosen Device Implementation.
   // The Factory object is used to create the specific underlying device instance.
   ICCIDeviceFactory *pCCIDevFactory = GetCCIDeviceFactory(CCIDevImpl);


   // Use the implementation-specific Device Factory to create a device.
   ICCIDevice *pCCIDevice = pCCIDevFactory->CreateCCIDevice();
#if (1 == ENABLE_DEBUG)
   pCCIDevice->GetSynchronizer()->SetLogLevel(gUMsgTestCmdLine.log);
   pCCIDevice->GetSynchronizer()->SetTraceLevel(gUMsgTestCmdLine.trace);
#endif // ENABLE_DEBUG
   //
   // Use the CCI Device to create Workspaces (buffers).
   // For NLB loopback mode (SW), we need four Workspaces:
   //
   // 1.) A workspace to serve as the DSM buffer for FPGA -> Host communication.
   // 2.) A data input buffer.
   // 3.) A data output buffer.
   // 4.) A UMsg buffer
   //
   ICCIWorkspace *pDSMWorkspace    = pCCIDevice->AllocateWorkspace(SW1_DSM_SIZE);
   ICCIWorkspace *pInputWorkspace  = pCCIDevice->AllocateWorkspace(SW1_BUFFER_SIZE);
   ICCIWorkspace *pOutputWorkspace = pCCIDevice->AllocateWorkspace(SW1_BUFFER_SIZE);
   ICCIWorkspace *pUMsgWorkspace   = pCCIDevice->AllocateWorkspace(UMSG_BUFFER_SIZE);
   
   // We need to initialize the input and output buffers, so we need addresses suitable
   // for dereferencing in user address space.
   // volatile, because the FPGA will be updating the buffers, too.
   volatile btVirtAddr pInputUsrVirt = pInputWorkspace->GetUserVirtualAddress();

   const    btUnsigned32bitInt  InputData = 0xdecafbad;
   volatile btUnsigned32bitInt *pInput    = (volatile btUnsigned32bitInt *)pInputUsrVirt;
   volatile btUnsigned32bitInt *pEndInput = (volatile btUnsigned32bitInt *)pInput +
                                     (pInputWorkspace->GetSizeInBytes() / sizeof(btUnsigned32bitInt));

   for ( ; pInput < pEndInput ; ++pInput ) {
      *pInput = InputData;
   }

   volatile btVirtAddr pOutputUsrVirt = pOutputWorkspace->GetUserVirtualAddress();
   volatile btVirtAddr pUMsgUsrVirt   = pUMsgWorkspace->GetUserVirtualAddress();
   
   memset((void *)pOutputUsrVirt, 0, pOutputWorkspace->GetSizeInBytes());
   memset((void *)pUMsgUsrVirt, 0, pUMsgWorkspace->GetSizeInBytes());
   
   volatile btVirtAddr pDSMUsrVirt  = pDSMWorkspace->GetUserVirtualAddress();


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

   // Set input workspace address
   pCCIDevice->SetCSR(CSR_SRC_ADDR, CACHELINE_ALIGNED_ADDR(pInputWorkspace->GetPhysicalAddress()));

   // Set output workspace address
   pCCIDevice->SetCSR(CSR_DST_ADDR,  CACHELINE_ALIGNED_ADDR(pOutputWorkspace->GetPhysicalAddress()));

   //PM: UMsg Base
   if(gUMsgTestCmdLine.notice_type==2 || gUMsgTestCmdLine.notice_type==3)
   {
      cout<<"\nConfiguring UMsg CSRs";
      btUnsigned32bitInt umsg_base_cl = CACHELINE_ALIGNED_ADDR(pUMsgWorkspace->GetPhysicalAddress()) & 0xfffffffc;
      btUnsigned32bitInt umsgbase_csr_val = umsg_base_cl | 0x1;
      pCCIDevice->SetCSR(CSR_UMSGBASE, umsgbase_csr_val);
      cout<<"\numsg base csr "<<umsgbase_csr_val;
      btUnsigned32bitInt umsgmode_csr_val;
      if(gUMsgTestCmdLine.notice_type==3)
      {
         umsgmode_csr_val = 0xffffffff;
      }
      else
      {
         umsgmode_csr_val = 0;
      }
      pCCIDevice->SetCSR(CSR_UMSGMODE, umsgmode_csr_val);
      cout<<"\numsg mode csr "<<umsgmode_csr_val<<endl<<flush;

      // Check if UMsg has completed Initialization 
      uint32_t umsg_status = 0;
      do
      {
         pCCIDevice->GetCSR(CSR_CIRBSTAT, &umsg_status);
      }
      while((umsg_status & 0x1) == 0);
      cout<<"\nUMsg Init Done";
   }


   // Set the number of cache lines for the test
   pCCIDevice->SetCSR(CSR_NUM_LINES, NUM_LINES);
   
   // Set the test mode
   /*   0 - RdLine_S   : Cached in Shared state
    *   1 - RdLine_I   : Not cached. Marked as Invalid. However it may cause cache pollution
    *   2 - RdLine_O   : Cached in Owned State
    */
   btUnsigned32bitInt csr_read_type= 0;
   if(gUMsgTestCmdLine.read_type==1) {
      csr_read_type = 1<<9;
   } else if (gUMsgTestCmdLine.read_type==2) {
      csr_read_type = 2<<9;
   }
   
   btUnsigned32bitInt test_mode_csr_val = gUMsgTestCmdLine.notice_type<<26 | 0x7<<2 |  csr_read_type;
   pCCIDevice->SetCSR(CSR_CFG,       test_mode_csr_val);
   
   
   volatile bt32bitCSR *StatusAddr = (volatile bt32bitCSR *)
                                     (pDSMUsrVirt  + DSM_STATUS_TEST_COMPLETE);

   // Start the test
   pCCIDevice->SetCSR(CSR_CTL, 0x3);
   if ( CCI_ASE == gUMsgTestCmdLine.target ) {
      cout << "\n\n####################################################################";
      cout << "\nNote:\nTime out feature Disabled in ASE.\nTest terminates only upon successful completion";
      cout << "\n####################################################################\n\n";
   } else {
      cout << "\n\n##################################################";
      cout << "\nNote:\nTime out feature Enabled.\nTest terminates on timeout/completion";
      cout << "\n##################################################\n\n";
   }
   
   // PM: Test flow
   // 1. CPU Polls on Addr N+1
   int timeout=0;
   time_t startTime = time(NULL);
   while ( *(volatile btUnsigned32bitInt *) (pOutputUsrVirt+ CL(NUM_LINES)) != 0xffffffff ) {

      //cout<<"SW Polling on Addr "<<(btUnsigned32bitInt *)(pOutputUsrVirt+CL(NUM_LINES))<<"\n"; 
      if ( (time(NULL) - startTime > TIMEOUT) && ( CCI_ASE != gUMsgTestCmdLine.target ) ) { 
         timeout++; 
         break;
      }

   }
   
   // 2.CPU copies from dst to src buffer
   // copy could perturb the latency numbers based on CPU load
   memcpy((void *)pInputUsrVirt, (void *)pOutputUsrVirt, CL(NUM_LINES));

   // Fence Operation
   __sync_synchronize();
	   
   // 3.CPU-> FPGA message. Select notice type
   if(gUMsgTestCmdLine.notice_type==0) {
      *(btUnsigned32bitInt *) (pInputUsrVirt+CL(NUM_LINES)) = 0xffffffff;
      *(btUnsigned32bitInt *) (pInputUsrVirt+CL(NUM_LINES)+4) = 0xffffffff;
      *(btUnsigned32bitInt *) (pInputUsrVirt+CL(NUM_LINES)+8) = 0xffffffff;
   } else if (gUMsgTestCmdLine.notice_type==1) {
      pCCIDevice->SetCSR(CSR_SW1NOTICE, 0x10101010);
   } else if(gUMsgTestCmdLine.notice_type==2 || gUMsgTestCmdLine.notice_type==3) {
      *(volatile btUnsigned32bitInt *) (pUMsgUsrVirt) = 0xffffffff;
   }
  
   //CPU polls on test completion
   cout<<"\nWaiting for Overall Test completion";  
   cout.flush();
	
   time_t startTime1 = time(NULL); 
   while( 0 == *StatusAddr && 0 == timeout ) {

      if ( ( (time(NULL) - startTime1) > TIMEOUT) && ( CCI_ASE != gUMsgTestCmdLine.target ) ) {
         timeout++;
         break;
      }

   }

   btBool bPassed = false;	 
   if ( 0 == timeout ) {   
      bPassed = true;
      cout<<"\nReceived Test completion signal\n";     
   }
   
   // Disable UMsgs upon Test completion
   pCCIDevice->SetCSR(CSR_UMSGBASE, 0x00000000);
   
   // Verify the device
   if ( (*(StatusAddr + CSR_OFFSET(DSM_STATUS_TEST_ERROR - DSM_STATUS_TEST_COMPLETE)) != 0) || timeout!=0 ) {
      bPassed = false;
	  
      if ( timeout != 0 ) {
         cout <<"\nTest Failed to Complete before timeout\n" ;
      } else {
         cout << *(StatusAddr + CSR_OFFSET(DSM_STATUS_TEST_ERROR - DSM_STATUS_TEST_COMPLETE))
              << " Device Errors" << endl;

         ios::fmtflags f = cout.flags();
         cout.flags(ios_base::right);
         cout.fill('0');

         for ( i = 0 ; i < DSM_STATUS_ERROR_REGS ; ++i ) {
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
   if ( bPassed ) {
      
      btUnsigned64bitInt num_ticks = *(StatusAddr + 3);         // high
      num_ticks = (num_ticks << 32) | *(StatusAddr + 2); // high<<32 | low
      int penalty_start = *(StatusAddr + 6);
      int penalty_end = *(StatusAddr + 7);
      btUnsigned64bitInt latency = num_ticks - penalty_start -penalty_end;
      cout <<"\nNum_Writes: "<<dec<<*(StatusAddr + 5);
      cout <<"\nNum_Reads : "<<*(StatusAddr + 4);
      cout <<"\nLatency   : "<<latency;
      cout<<"\n";
   }

   // Clean up..

   // Stop the device
   pCCIDevice->SetCSR(CSR_CTL,      0x7);


   // Release the Workspaces
   pCCIDevice->FreeWorkspace(pInputWorkspace);
   pCCIDevice->FreeWorkspace(pOutputWorkspace);
   pCCIDevice->FreeWorkspace(pDSMWorkspace);
   pCCIDevice->FreeWorkspace(pUMsgWorkspace);
   
   // Release the CCI Device instance.
   pCCIDevFactory->DestroyCCIDevice(pCCIDevice);

   // Release the CCI Device Factory instance.
   delete pCCIDevFactory;

   return bPassed ? 0 : 1;
}


BEGIN_C_DECLS

int ccidemo_on_nix_long_option_only(AALCLP_USER_DEFINED user, const char *option)
{
   struct UMsgTestCmdLine *cl = (struct UMsgTestCmdLine *)user;

   if ( 0 == strcmp("--help", option) ) {
      flag_setf(cl->flags, CCIDEMO_CMD_FLAG_HELP);
   } else if ( 0 == strcmp("--version", option) ) {
      flag_setf(cl->flags, CCIDEMO_CMD_FLAG_VERSION);
   }

   return 0;
}

int ccidemo_on_nix_long_option(AALCLP_USER_DEFINED user, const char *option, const char *value)
{
   struct UMsgTestCmdLine *cl = (struct UMsgTestCmdLine *)user;

   if ( 0 == strcmp("--target", option) ) {
      if ( 0 == strcasecmp("aal", value) ) {
         cl->target = CCI_AAL;
      } else if ( 0 == strcasecmp("ase", value) ) {
         cl->target = CCI_ASE;
      } else if ( 0 == strcasecmp("swsim", value) ) {
         cl->target = CCI_SWSIM;
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
         cout <<"Notice type: "<< value <<" ";
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
   } else if( 0 == strcmp("--read_type", option)){
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

   return 0;
}

void help_msg_callback(FILE *fp, struct _aalclp_gcs_compliance_data *gcs)
{
   fprintf(fp, "Usage:\n");
   fprintf(fp, "   UMsgTest [--target=<TARGET>]\n");
   fprintf(fp, "\n");
   fprintf(fp, "      <TARGET> = one of { AAL ASE SWSim }\n");
   fprintf(fp, "\n");
}

void showhelp(FILE *fp, struct _aalclp_gcs_compliance_data *gcs)
{
   help_msg_callback(fp, gcs);
}

int parsecmds(struct UMsgTestCmdLine *cl, int argc, char *argv[])
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

int verifycmds(struct UMsgTestCmdLine *cl)
{
   if ( CCI_NULL == cl->target ) {
      cout << "No valid --target specified." << endl;
      return 1;
   }

   return 0;
}

END_C_DECLS

/*
@addtogroup UMsgTest
@{

@verbatim
This application is for example purposes only.
It is not intended to represent a model for developing commercially-deployable applications.
It is designed to show working examples of the AAL programming model and APIs.@endverbatim

Date: 4/10/2014
CCIDemo modified to use NLB SW1 test with UMsgs

Usage"
./UMsgTest --target=Direct --notice_type=<notice type option> --read_type=<read type option>

<notice type option> :  select types of message from CPU-to-FPGA
			 0 - read polling from AFU
			 1 - CSR Write
			 2 - UMsg with Data
			 3 - UMsgH (Hint without Data)

<read type option> : select type of read request to use. Gives control of the caching behavior.
			0 - RdLine_S, cached in Shared state
			1 - RdLine_I. Not cached, but it may cause FPGA cache pollution, since FPGA cache is used as a data buffer.
			2 - RdLine_O, cached in Owned state. May be used to pre-fetch data to FPGA cache.




// High level Test flow from HW AFU point of view:
// 1. Wait on test_go
// 2. Start HW timer. Write N cache lines. WrData= {16{32'h0000_0001}}
// 3. Write Fence.
// 4. FPGA -> CPU Message. Write to address N+1. WrData = {{14{32'h0000_0000}},{64{1'b1}}}
// 5. CPU -> FPGA Message. Configure one of the following methods:
//   a. Poll on Addr N+1. Expected Data [63:32]==32'hffff_ffff
//   b. CSR write to Address 0xB00. Data= Dont Care
//   c. UMsg Mode 0 with data
//   d. UMsgH Mode 1 , i.e. hint without data.
// 7. Read N cache lines. Wait for all read completions.
// 6. Stop HW timer. Send test completion.

@} group UMsgTest
*/


