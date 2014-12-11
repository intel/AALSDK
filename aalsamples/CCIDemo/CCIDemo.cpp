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
//****************************************************************************
/// @file CCIDemo.cpp
/// @brief Native Loopback (LPBK1), one cache line, via CCILib.
/// @ingroup CCIDemo
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer Sample Application
///
///    This application is for example purposes only.
///    It is not intended to represent a model for developing commercially-deployable applications.
///    It is designed to show working examples of the AAL programming model and APIs.
///
/// AUTHOR: Tim Whisonant, Intel Corporation
///
/// HISTORY:
/// WHEN:          WHO:       WHAT:
/// 02/24/2012     TSW        Original version.@endverbatim
//****************************************************************************
#include <aalsdk/aalclp/aalclp.h>
#include <strings.h> // strcasecmp

#include "CCILib.h"

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
#define LPBK1_BUFFER_SIZE          CL(1)

#define LPBK1_DSM_SIZE             MB(4)

#define CSR_CIPUCTL                0x280

#define CSR_AFU_DSM_BASEL          0x1a00
#define CSR_AFU_DSM_BASEH          0x1a04
#define CSR_SRC_ADDR               0x1a20
#define CSR_DST_ADDR               0x1a24
#define CSR_NUM_LINES              0x1a28
#define CSR_CTL                    0x1a2c
#define CSR_CFG                    0x1a34

#define CSR_OFFSET(x)              ((x) / sizeof(bt32bitCSR))

#define DSM_STATUS_TEST_COMPLETE   0x40
#define DSM_STATUS_TEST_ERROR      0x44
#define DSM_STATUS_MODE_ERROR_0    0x60

#define DSM_STATUS_ERROR_REGS      8


////////////////////////////////////////////////////////////////////////////////
BEGIN_C_DECLS

struct CCIDemoCmdLine
{
   btUIntPtr               flags;
#define CCIDEMO_CMD_FLAG_HELP    0x00000001
#define CCIDEMO_CMD_FLAG_VERSION 0x00000002

   CCIDeviceImplementation target;
   int                     log;
   int                     trace;
};

struct CCIDemoCmdLine gCCIDemoCmdLine =
{
   0,
   CCI_NULL,
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
                             "CCIDemo",
                             CCILIB_VERSION,
                             "",
                             help_msg_callback,
                             &gCCIDemoCmdLine)

int parsecmds(struct CCIDemoCmdLine * , int , char *[] );
int verifycmds(struct CCIDemoCmdLine * );
END_C_DECLS
////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
   if ( argc < 2 ) {
      showhelp(stdout, &_aalclp_gcs_data);
      return 1;
   } else if ( parsecmds(&gCCIDemoCmdLine, argc, argv) ) {
      cerr << "Error scanning command line." << endl;
      return 2;
   } else if ( flag_is_set(gCCIDemoCmdLine.flags, CCIDEMO_CMD_FLAG_HELP|CCIDEMO_CMD_FLAG_VERSION) ) {
      return 0; // per GCS
   } else if ( verifycmds(&gCCIDemoCmdLine) ) {
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
   const CCIDeviceImplementation CCIDevImpl = gCCIDemoCmdLine.target;

   // Obtain a 'Factory' object matching the chosen Device Implementation.
   // The Factory object is used to create the specific underlying device instance.
   ICCIDeviceFactory *pCCIDevFactory = GetCCIDeviceFactory(CCIDevImpl);


   // Use the implementation-specific Device Factory to create a device.
   ICCIDevice *pCCIDevice = pCCIDevFactory->CreateCCIDevice();
#if (1 == ENABLE_DEBUG)
   pCCIDevice->GetSynchronizer()->SetLogLevel(gCCIDemoCmdLine.log);
   pCCIDevice->GetSynchronizer()->SetTraceLevel(gCCIDemoCmdLine.trace);
#endif // ENABLE_DEBUG
   //
   // Use the CCI Device to create Workspaces (buffers).
   // For NLB loopback mode (LPBK1), we need three Workspaces:
   //
   // 1.) A workspace to serve as the DSM buffer for FPGA -> Host communication.
   // 2.) A data input buffer.
   // 3.) A data output buffer.
   //
   ICCIWorkspace *pDSMWorkspace    = pCCIDevice->AllocateWorkspace(LPBK1_DSM_SIZE);
   ICCIWorkspace *pInputWorkspace  = pCCIDevice->AllocateWorkspace(LPBK1_BUFFER_SIZE);
   ICCIWorkspace *pOutputWorkspace = pCCIDevice->AllocateWorkspace(LPBK1_BUFFER_SIZE);

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

   memset((void *)pOutputUsrVirt, 0, pOutputWorkspace->GetSizeInBytes());

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

   // Set the number of cache lines for the test
   pCCIDevice->SetCSR(CSR_NUM_LINES, LPBK1_BUFFER_SIZE / CL(1));

   // Set the test mode
   pCCIDevice->SetCSR(CSR_CFG,       0);


   volatile bt32bitCSR *StatusAddr = (volatile bt32bitCSR *)
                                    (pDSMUsrVirt  + DSM_STATUS_TEST_COMPLETE);

   // Start the test
   pCCIDevice->SetCSR(CSR_CTL,      0x3);


   // Wait for test completion
   while( 0 == *StatusAddr ) {
      usleep(100);
   }


   // Verify the test results
   btBool bPassed = true;

   // Verify the buffers
   if ( memcmp((void *)pInputUsrVirt, (void *)pOutputUsrVirt, LPBK1_BUFFER_SIZE) != 0 ) {
      bPassed = false;

      pInput = (volatile btUnsigned32bitInt *)pInputUsrVirt;
      volatile btUnsigned32bitInt *pOutput = (volatile btUnsigned32bitInt *)pOutputUsrVirt;

      btUnsigned32bitInt Byte;

      ios::fmtflags f = cout.flags();
      cout.flags(ios_base::right | ios_base::hex);
      cout.fill('0');

      for ( Byte = 0 ; pInput < pEndInput ; ++pInput, ++pOutput, Byte += sizeof(uint32_t) ) {

         if ( *pInput != *pOutput ) {

            cout << "Buffer[0x" << setw(2) << Byte << "] Input: 0x"
                     << *pInput << " != Output: 0x" << *pOutput << endl;

         }

      }

      cout.fill(' ');
      cout.flags(f);
   }

   // Verify the device
   if ( *(StatusAddr + CSR_OFFSET(DSM_STATUS_TEST_ERROR - DSM_STATUS_TEST_COMPLETE)) != 0 ) {

      bPassed = false;

      cout <<
              *(StatusAddr + CSR_OFFSET(DSM_STATUS_TEST_ERROR - DSM_STATUS_TEST_COMPLETE))
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


   cout << (bPassed ? "PASS" : "ERROR") << endl << flush;


   // Clean up..

   // Stop the device
   pCCIDevice->SetCSR(CSR_CTL,      0x7);


   // Release the Workspaces
   pCCIDevice->FreeWorkspace(pInputWorkspace);
   pCCIDevice->FreeWorkspace(pOutputWorkspace);
   pCCIDevice->FreeWorkspace(pDSMWorkspace);

   // Release the CCI Device instance.
   pCCIDevFactory->DestroyCCIDevice(pCCIDevice);

   // Release the CCI Device Factory instance.
   delete pCCIDevFactory;

   return bPassed ? 0 : 1;
}


BEGIN_C_DECLS

int ccidemo_on_nix_long_option_only(AALCLP_USER_DEFINED user, const char *option)
{
   struct CCIDemoCmdLine *cl = (struct CCIDemoCmdLine *)user;

   if ( 0 == strcmp("--help", option) ) {
      flag_setf(cl->flags, CCIDEMO_CMD_FLAG_HELP);
   } else if ( 0 == strcmp("--version", option) ) {
      flag_setf(cl->flags, CCIDEMO_CMD_FLAG_VERSION);
   }

   return 0;
}

int ccidemo_on_nix_long_option(AALCLP_USER_DEFINED user, const char *option, const char *value)
{
   struct CCIDemoCmdLine *cl = (struct CCIDemoCmdLine *)user;

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
   }

   return 0;
}

void help_msg_callback(FILE *fp, struct _aalclp_gcs_compliance_data *gcs)
{
   fprintf(fp, "Usage:\n");
   fprintf(fp, "   CCIDemo [--target=<TARGET>]\n");
   fprintf(fp, "\n");
   fprintf(fp, "      <TARGET> = one of { AAL ASE SWSim }\n");
   fprintf(fp, "\n");
}

void showhelp(FILE *fp, struct _aalclp_gcs_compliance_data *gcs)
{
   help_msg_callback(fp, gcs);
}

int parsecmds(struct CCIDemoCmdLine *cl, int argc, char *argv[])
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

int verifycmds(struct CCIDemoCmdLine *cl)
{
   if ( CCI_NULL == cl->target ) {
      cout << "No valid --target specified." << endl;
      return 1;
   }

   return 0;
}

END_C_DECLS


/**
@addtogroup CCIDemo
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

CCIDemo runs an NLB mode 0 (lpbk1) memcpy transfer for a buffer size of 1 cache line.

2 Running the application

2.1 Using the Direct Driver

@code
$ sudo /sbin/insmod fapdiag.ko qpi_aperture_phys=0xc8080000 # Jaketown -OR-
$ sudo /sbin/insmod fapdiag.ko qpi_aperture_phys=0x88080000 # Nehalem
$ sudo chmod 'a+rw' /dev/fapdiag
$ CCIDemo --target=Direct@endcode

2.2 Using ASE

(xterm 0)
@code
$ cd AALSDK/ase
$ make dpi CAFU=nlb
$ make sim@endcode

(xterm 1)
@code
$ CCIDemo --target=ASE@endcode

@} group CCIDemo
*/

