// Copyright(c) 2015-2016, Intel Corporation
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
// @file diag-nlb-common.h
// @brief Functionality common to all NLB utils.
// @ingroup
// @verbatim
// Accelerator Abstraction Layer
//
// AUTHORS: Tim Whisonant, Intel Corporation
// 			Sadruta Chandrashekar, Intel Corporation
//
// HISTORY:
// WHEN:          WHO:     WHAT:
// 06/09/2013     TSW      Initial version.
// 01/07/2015	  SC	   fpgadiag version.@endverbatim
//****************************************************************************
#ifndef __DIAG_NLB_COMMON_H__
#define __DIAG_NLB_COMMON_H__

#include <aalsdk/aalclp/aalclp.h>
#include "utils.h"

BEGIN_C_DECLS

#define HIGH 0xffffffff
#define LOW  0x00000000

# define NLB_TESTMODE_LPBK1  "TestMode_lpbk1"
# define NLB_TESTMODE_READ   "TestMode_read"
# define NLB_TESTMODE_WRITE  "TestMode_write"
# define NLB_TESTMODE_TRPUT  "TestMode_trput"
# define NLB_TESTMODE_SW     "TestMode_sw"
# define NLB_TESTMODE_ATOMIC "TestMode_atomic"
struct NLBDefaults
{
   wkspc_size_type mincls;
   wkspc_size_type maxcls;
   wkspc_size_type begincls;
   wkspc_size_type endcls;
   wkspc_size_type multicls;
   phys_type       dsmphys;
   phys_type       srcphys;
   phys_type       dstphys;
   freq_type       clkfreq;
   const char     *warmfpgacache;
   const char     *coolfpgacache;
   const char	  *coolcpucache;
   const char     *nobw;
   const char     *tabular;
   const char     *suppresshdr;
   const char     *wli;
   const char     *wlm;
   const char     *wpi;
   const char     *rds;
   const char     *rdi;
  // const char     *rdo;
   const char     *cont;
#if   defined( __AAL_WINDOWS__ )
# error TODO
#elif defined( __AAL_LINUX__ )
   //timespec_type   to_nsec;
   timespec_type   to_usec;
   timespec_type   to_msec;
   timespec_type   to_sec;
   timespec_type   to_min;
   timespec_type   to_hour;
#endif // OS
   const char     *poll;
   const char     *csr_write;
   const char     *umsg_data;
   const char     *umsg_hint;

   const char     *rva;
   const char     *rvl0;
   const char     *rvh0;
   const char     *rvh1;
   const char     *rvr;

   const char     *wva;
   const char     *wvl0;
   const char     *wvh0;
   const char     *wvh1;
   const char     *wvr;

   const char     *wrfva;
   const char     *wrfvl0;
   const char     *wrfvh0;
   const char     *wrfvh1;

   const char     *awp;

   const char     *st;
   const char     *ut;
   cmp_xchg_type  mincx;
   cmp_xchg_type  maxcx;
   cmp_xchg_type  cx;
   quad_word_type hqw;
   quad_word_type sqw;
   quad_word_type minhqw;
   quad_word_type maxhqw;
   quad_word_type minsqw;
   quad_word_type maxsqw;
   wkspc_size_type strides;
   wkspc_size_type min_strides;
   wkspc_size_type max_strides;
};

struct NLBBandwidth
{
   int             dirty;
   uint_type       iter;
   uint_type       sample;
   wkspc_size_type bytes;
   double          bw;
#define NLB_MAX_BW_STR 20
   char            bwstr[NLB_MAX_BW_STR];
};
#define NLB_BW_CUR_RD 0
#define NLB_BW_CUR_WR 1
#define NLB_BW_MIN_RD 2
#define NLB_BW_MIN_WR 3
#define NLB_BW_MAX_RD 4
#define NLB_BW_MAX_WR 5
#define NLB_BW_AVG_RD 6
#define NLB_BW_AVG_WR 7

struct NLBCmdLine
{
   const char              *copyright;
   const char              *mode;    // "LPBK1", "READ", "WRITE", "TRPUT", "SW"
   const char              *title;
   u64_type                 cmdflags;
#define NLB_CMD_PARSE_ERROR       		(u64_type)0x00000001  /* Command parser error, invalid option, etc.                       */
#define NLB_CMD_FLAG_HELP         		(u64_type)0x00000002  /* --help                                                           */
#define NLB_CMD_FLAG_VERSION      		(u64_type)0x00000004  /* --version                                                        */

#define NLB_CMD_FLAG_TABULAR      		(u64_type)0x00000008  /* --tabular         (show tabular output, like legacy NLBTest)     */
#define NLB_CMD_FLAG_SUPPRESSHDR  		(u64_type)0x00000010  /* --suppress-hdr    (don't show column headers in text mode)       */
#define NLB_CMD_FLAG_BANDWIDTH    		(u64_type)0x00000020  /* --no-bw           (don't calculate bandwidth numbers when clear) */

#define NLB_CMD_FLAG_DSM_PHYS     		(u64_type)0x00000040  /* --dsm-phys  X     (physical address of device status workspace)  */
#define NLB_CMD_FLAG_SRC_PHYS     		(u64_type)0x00000080  /* --src-phys  X     (physical address of source workspace)         */
#define NLB_CMD_FLAG_DST_PHYS     		(u64_type)0x00000100  /* --dest-phys X     (physical address of destination workspace)    */

#define NLB_CMD_FLAG_BEGINCL      		(u64_type)0x00000800  /* --begin X         (number of cache lines)                        */
#define NLB_CMD_FLAG_ENDCL        		(u64_type)0x00001000  /* --end X           (number of cache lines)                        */
#define NLB_CMD_FLAG_WRLINE_I      		(u64_type)0x00002000  /* --wli             (write-through caching)                        */
#define NLB_CMD_FLAG_WRLINE_M      		(u64_type)0x00004000  /* --wlm             (write-back caching)                           */
#define NLB_CMD_FLAG_PWR          		(u64_type)0x00008000  /* --pwr             (posted writes)                                */
#define NLB_CMD_FLAG_CONT         		(u64_type)0x00010000  /* --cont            (continuous mode)                              */
//#define NLB_CMD_FLAG_TONSEC       		(u64_type)0x00020000  /* --timeout-nsec X  (nanosecond timeout for continuous mode)       */
#define NLB_CMD_FLAG_TOUSEC       		(u64_type)0x00040000  /* --timeout-usec X  (microsecond timeout for continuous mode)      */
#define NLB_CMD_FLAG_TOMSEC        		(u64_type)0x00080000  /* --timeout-msec X  (millisecond timeout for continuous mode)      */
#define NLB_CMD_FLAG_TOSEC        		(u64_type)0x00100000  /* --timeout-sec  X  (second timeout for continuous mode)           */
#define NLB_CMD_FLAG_TOMIN        		(u64_type)0x00200000  /* --timeout-min  X  (minute timeout for continuous mode)           */
#define NLB_CMD_FLAG_TOHOUR       		(u64_type)0x00400000  /* --timeout-hour X  (hour timeout for continuous mode)             */
#define NLB_CMD_FLAG_CLKFREQ      		(u64_type)0x00800000  /* --clock-freq X    (fpga clock frequency, default=200 MHz)        */

#define NLB_CMD_FLAGS_TO          		(NLB_CMD_FLAG_TOUSEC | \
                                        NLB_CMD_FLAG_TOMSEC | \
                                        NLB_CMD_FLAG_TOSEC  | \
                                        NLB_CMD_FLAG_TOMIN  | \
                                        NLB_CMD_FLAG_TOHOUR)

#define NLB_CMD_FLAG_CSRS         		(u64_type)0x01000000  		/* --csrs            (show CSR read/write)                          	  */

#define NLB_CMD_FLAG_WARM_FPGA_CACHE 	(u64_type)0x02000000  		/* --warm-fpga-cache (attempt to load the cache with hits at init)		  */
#define NLB_CMD_FLAG_COOL_FPGA_CACHE 	(u64_type)0x04000000  		/* --cool-fpga-cache (attempt to load the cache with misses at init)	  */

#define NLB_CMD_FLAG_RDS          		(u64_type)0x08000000  		/* --rds             (readline - shared)                            	  */
#define NLB_CMD_FLAG_RDI          		(u64_type)0x10000000  		/* --rdi             (readline - invalidate)                        	  */
//#define NLB_CMD_FLAG_RDO         		(u64_type)0x20000000  		/* --rdo             (readline - ownership)                         	  */

#define NLB_CMD_FLAG_POLL         		(u64_type)0x40000000   		/* --poll            notice sent from CPU to FPGA via poll         		  */
#define NLB_CMD_FLAG_CSR_WRITE    		(u64_type)0x200000000  		/* --csr-write       notice sent from CPU to FPGA via csr-write    		  */
#define NLB_CMD_FLAG_UMSG_DATA    		(u64_type)0x400000000  		/* --umsg-data       notice sent from CPU to FPGA via umsg-data    		  */
#define NLB_CMD_FLAG_UMSG_HINT    		(u64_type)0x800000000  		/* --umsg-hint       notice sent from CPU to FPGA via umsg-hint	   		  */

#define NLB_CMD_FLAG_COOL_CPU_CACHE 	(u64_type)0x1000000000 		/* --cool-cpu-cache  Cool CPU Cache							           	  */

#define NLB_CMD_FLAG_READ_VA    		(u64_type)0x2000000000 		/* --rva	          Distribute data among QPI, PCIe0 and PCIe1 channels  */
#define NLB_CMD_FLAG_READ_VL0		   	(u64_type)0x4000000000 		/* --rvl0		      Data transferred on QPI channel for reads		        */
#define NLB_CMD_FLAG_READ_VH0	   		(u64_type)0x8000000000 		/* --rvh0		      Data transferred on PCIe0  channel for reads		     */
#define NLB_CMD_FLAG_READ_VH1   		(u64_type)0x10000000000		/* --rvh1		      Data transferred on PCIe1  channel for reads  	     */
#define NLB_CMD_FLAG_READ_VR            (u64_type)0x20000000000     /* --rvr              Data transferred on randomly chosen channel for reads*/

#define NLB_CMD_FLAG_MULTICL            (u64_type)0x40000000000		/* --multi-cl X  	 (number of cache lines)        					  */

#define NLB_CMD_FLAG_ST		            (u64_type)0x80000000000		/*  --shared-token   (Sub-mode for atomic tests) 					  */
#define NLB_CMD_FLAG_UT		            (u64_type)0x100000000000	/*  --seperate-token (Sub-mode for atomic tests) 					  */
#define NLB_CMD_FLAG_HQW	            (u64_type)0x200000000000	/*  --hardware-qw  	 (QWord number on the Hardware thread) 	     */
#define NLB_CMD_FLAG_SQW	            (u64_type)0x400000000000	/*  --software-qw    (QWord number on the Software thread)		 */
#define NLB_CMD_FLAG_CX		            (u64_type)0x800000000000	/*  --cmp-xchg       (Number of Cmp-Xchg operations)				  */

#define NLB_CMD_FLAG_ALT_WR_PRN         (u64_type)0x1000000000000   /* --alt-wr-pattern  (Alternate write patterns)                  */
#define NLB_CMD_FLAG_STRIDED_ACS        (u64_type)0x2000000000000   /* --strided-access  (non-unit strides in NLB)                   */

#define NLB_CMD_FLAG_WRITE_VA           (u64_type)0x4000000000000   /* --wva          Distribute data among QPI, PCIe0 and PCIe1 channels   */
#define NLB_CMD_FLAG_WRITE_VL0          (u64_type)0x8000000000000   /* --wvl0         Data transferred on QPI channel for writes            */
#define NLB_CMD_FLAG_WRITE_VH0          (u64_type)0x10000000000000  /* --wvh0         Data transferred on PCIe0  channel for writes         */
#define NLB_CMD_FLAG_WRITE_VH1          (u64_type)0x20000000000000  /* --wvh1         Data transferred on PCIe1  channel for writes         */
#define NLB_CMD_FLAG_WRITE_VR           (u64_type)0x40000000000000  /* --wvr          Data transferred on randomly chosen channel for writes*/

#define NLB_CMD_FLAG_WRFENCE_VA         (u64_type)0x80000000000000  /* --wrfva        Distribute data among QPI, PCIe0 and PCIe1 channels   		*/
#define NLB_CMD_FLAG_WRFENCE_VL0        (u64_type)0x100000000000000 /* --wrfvl0       Data transferred on QPI channel for write fence       		*/
#define NLB_CMD_FLAG_WRFENCE_VH0        (u64_type)0x200000000000000 /* --wrfvh0       Data transferred on PCIe0  channel for write fence          */
#define NLB_CMD_FLAG_WRFENCE_VH1        (u64_type)0x400000000000000 /* --wrfvh1       Data transferred on PCIe1  channel for write fence          */

#define NLB_CMD_FLAG_WRPUSH_I       	(u64_type)0x800000000000000 /* --wpi      	  WritePush_I caching          */

#define NLB_CMD_FLAG_FEATURE0     		(u64_type)0x80000000   		/* --0 */
#define NLB_CMD_FLAG_FEATURE1     		(u64_type)0x100000000  		/* --1 */

   uint_type                dispflags;
   uint_type                iter;
   wkspc_size_type          begincls;
   wkspc_size_type          endcls;
   wkspc_size_type          multicls;
   phys_type                dsmphys;
   phys_type                srcphys;
   phys_type                dstphys;
   freq_type                clkfreq;
   cmp_xchg_type            cx;
   quad_word_type           hqw;
   quad_word_type           sqw;
   wkspc_size_type          strided_acs;
#if   defined( __AAL_WINDOWS__ )
# error TODO
#elif defined( __AAL_LINUX__ )
   struct timespec          timeout;
  // timespec_type            to_nsec;
   timespec_type            to_usec;
   timespec_type            to_msec;
   timespec_type            to_sec;
   timespec_type            to_min;
   timespec_type            to_hour;
#endif // OS
   const struct NLBDefaults defaults;
   int                      bwdirty;
   struct NLBBandwidth      bw[8];

   //Added from fpgasane - CMyCmdLine
   AAL::btUIntPtr   flags;
   #define MY_CMD_FLAG_HELP    0x00000001
   #define MY_CMD_FLAG_VERSION 0x00000002
   std::string      AFUTarget;
   AAL::btInt       DevTarget;
   std::string      TestMode;
   AAL::btInt       LogLevel;
};

#define NLB_DISP_FLAG_TITLE         (uint_type)0x00000001  /* NLB test title / description                 */
#define NLB_DISP_FLAG_HEADERS       (uint_type)0x00000002  /* (text mode) column headers                   */
#define NLB_DISP_FLAG_ITERS         (uint_type)0x00000004  /* current test iteration                       */
#define NLB_DISP_FLAG_CLS           (uint_type)0x00000008  /* current number of cache lines                */
#define NLB_DISP_FLAG_FREQ          (uint_type)0x00000010  /* FPGA clock frequency                         */
#define NLB_DISP_FLAG_EVICT         (uint_type)0x00000020  /* cache evictions perf counter                 */
#define NLB_DISP_FLAG_CACHE_INIT    (uint_type)0x00000040  /* cache initialization (prefill hits / misses) */
#define NLB_DISP_FLAG_WR_TYPE       (uint_type)0x00000080  /* write type (write-back or write-through)     */
#define NLB_DISP_FLAG_POSTED_WR     (uint_type)0x00000100  /* whether using posted writes                  */
#define NLB_DISP_FLAG_READS         (uint_type)0x00000200  /* number of reads                              */
#define NLB_DISP_FLAG_CACHE_RD_HITS (uint_type)0x00000400  /* cache read hits                              */
#define NLB_DISP_FLAG_CACHE_RD_MISS (uint_type)0x00000800  /* cache read misses                            */
#define NLB_DISP_FLAG_RD_BW         (uint_type)0x00001000  /* read bandwidth                               */
#define NLB_DISP_FLAG_PEAK_RD_BW    (uint_type)0x00002000  /* peak read bandwidth                          */
#define NLB_DISP_FLAG_AVG_RD_BW     (uint_type)0x00004000  /* average read bandwidth                       */
#define NLB_DISP_FLAG_WRITES        (uint_type)0x00008000  /* number of writes                             */
#define NLB_DISP_FLAG_CACHE_WR_HITS (uint_type)0x00010000  /* cache write hits                             */
#define NLB_DISP_FLAG_CACHE_WR_MISS (uint_type)0x00020000  /* cache write misses                           */
#define NLB_DISP_FLAG_WR_BW         (uint_type)0x00040000  /* write bandwidth                              */
#define NLB_DISP_FLAG_PEAK_WR_BW    (uint_type)0x00080000  /* peak write bandwidth                         */
#define NLB_DISP_FLAG_AVG_WR_BW     (uint_type)0x00100000  /* average write bandwidth                      */
#define NLB_DISP_FLAG_CONT          (uint_type)0x00200000  /* whether continuous mode is enabled           */
#define NLB_DISP_FLAG_CSRS          (uint_type)0x00400000  /* CSRs and read/write values                   */
#define NLB_DISP_FLAG_QLP_VERSION   (uint_type)0x00800000  /* QPI Link Protocol version                    */
#define NLB_DISP_FLAG_NLB_VERSION   (uint_type)0x01000000  /* NLB version                                  */
#define NLB_DISP_FLAG_VERSION       (uint_type)0x02000000  /* program version                              */
#define NLB_DISP_FLAG_COPYRIGHT     (uint_type)0x04000000  /* copyright notice                             */
#define NLB_DISP_FLAG_SRC_WKSPC     (uint_type)0x08000000  /* source workspace                             */
#define NLB_DISP_FLAG_DST_WKSPC     (uint_type)0x10000000  /* destination workspace                        */

#define NLB_DISP_FLAG_ALL           (NLB_DISP_FLAG_TITLE         | \
                                     NLB_DISP_FLAG_HEADERS       | \
                                     NLB_DISP_FLAG_ITERS         | \
                                     NLB_DISP_FLAG_CLS           | \
                                     NLB_DISP_FLAG_FREQ          | \
                                     NLB_DISP_FLAG_EVICT         | \
                                     NLB_DISP_FLAG_CACHE_INIT    | \
                                     NLB_DISP_FLAG_WR_TYPE       | \
                                     NLB_DISP_FLAG_POSTED_WR     | \
                                     NLB_DISP_FLAG_READS         | \
                                     NLB_DISP_FLAG_CACHE_RD_HITS | \
                                     NLB_DISP_FLAG_CACHE_RD_MISS | \
                                     NLB_DISP_FLAG_RD_BW         | \
                                     NLB_DISP_FLAG_PEAK_RD_BW    | \
                                     NLB_DISP_FLAG_WRITES        | \
                                     NLB_DISP_FLAG_CACHE_WR_HITS | \
                                     NLB_DISP_FLAG_CACHE_WR_MISS | \
                                     NLB_DISP_FLAG_WR_BW         | \
                                     NLB_DISP_FLAG_PEAK_WR_BW    | \
                                     NLB_DISP_FLAG_CONT          | \
                                     NLB_DISP_FLAG_CSRS          | \
                                     NLB_DISP_FLAG_QLP_VERSION   | \
                                     NLB_DISP_FLAG_NLB_VERSION   | \
                                     NLB_DISP_FLAG_VERSION       | \
                                     NLB_DISP_FLAG_COPYRIGHT     | \
                                     NLB_DISP_FLAG_SRC_WKSPC     | \
                                     NLB_DISP_FLAG_DST_WKSPC)


void NLBSetupCmdLineParser(aalclp * , struct NLBCmdLine * );
void nlb_help_message_callback(FILE * , struct _aalclp_gcs_compliance_data * );
void MyNLBShowHelp(FILE * , aalclp_gcs_compliance_data * );

END_C_DECLS

bool         NLBVerifyCmdLine(NLBCmdLine       & ,
                              std::ostream     & ) throw();

#endif // __DIAG_NLB_COMMON_H__

