// Copyright (c) 2014-2015, Intel Corporation
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
// **************************************************************************
/* 
 * Module Info: ASE common (C header file)
 * Language   : C/C++
 * Owner      : Rahul R Sharma
 *              rahul.r.sharma@intel.com
 *              Intel Corporation
 * 
 */


/*
 * Prevent recursive declarations
 */
#ifndef _ASE_COMMON_H_
#define _ASE_COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>   
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <time.h>       
#include <ctype.h>         
#include <mqueue.h>        
#include <errno.h>         
#include <signal.h>        
#include <pthread.h>       
#include <sys/resource.h>  
#include <sys/time.h>      
#include <math.h>
#include <sys/socket.h>
#include <sys/un.h>

#ifdef SIM_SIDE 
#include "svdpi.h"
#endif

/*
 * Return integers
 */
#define OK     0
#define NOT_OK -1

/* *******************************************************************************
 *
 * SYSTEM FACTS
 * 
 * *******************************************************************************/
#define FPGA_ADDR_WIDTH       38
#define PHYS_ADDR_PREFIX_MASK (uint64_t)(-1) << FPGA_ADDR_WIDTH
#define CL_ALIGN_SHIFT        6

// Width of a cache line in bytes
#define CL_BYTE_WIDTH        64
#define SIZEOF_1GB_BYTES     (uint64_t)pow(1024, 3)

// CSR memory map size
#define CSR_MAP_SIZE            64*1024

// Size of page
#define ASE_PAGESIZE   0x1000        // 4096 bytes
#define CCI_CHUNK_SIZE 2*1024*1024   // CCI 2 MB physical chunks 

/*
 * Unordered Message (UMSG) Address space
 */
// UMSG specific CSRs
#define ASE_UMSGBASE_CSROFF            0x3F4  // UMSG base address
#define ASE_UMSGMODE_CSROFF            0x3F8  // UMSG mode
#define ASE_CIRBSTAT_CSROFF            0x278  // CIRBSTAT

/*
 * SPL constants
 */
#define SPL_DSM_BASEL_OFF 0x1000 //0x910
#define SPL_DSM_BASEH_OFF 0x1004 //0x914
#define SPL_CXT_BASEL_OFF 0x1008 //0x918 // SPL Context Physical address
#define SPL_CXT_BASEH_OFF 0x100c //0x91c 
#define SPL_CH_CTRL_OFF   0x1010 //0x920

/*
 * AFU constants
 */
#define AFU_DSM_BASEL_OFF 0x8A00
#define AFU_DSM_BASEH_OFF 0x8A04
#define AFU_CXT_BASEL_OFF 0x8A08
#define AFU_CXT_BASEH_OFF 0x8A0c

//                                Byte Offset  Attribute  Width  Comments
#define      DSM_AFU_ID            0            // RO      32b    non-zero value to uniquely identify the AFU
#define      DSM_STATUS            0x40         // RO      512b   test status and error info


/* *******************************************************************************
 *
 * ASE INTERNAL MACROS
 *
 * *******************************************************************************/
// SHM memory name length
#define ASE_SHM_NAME_LEN   40

// ASE filepath length
#define ASE_FILEPATH_LEN  256

// ASE logger len
#define ASE_LOGGER_LEN    1024

// work Directory location
char *ase_workdir_path;

// Run location
char *ase_run_path;

// Timestamp IPC file
#define TSTAMP_FILENAME ".ase_timestamp"
char *tstamp_filepath;

// IPC control list
char *ipclist_filepath;

// Ready filepath
char *ase_ready_filepath;

// CONFIG,SCRIPT parameter paths received from SV (initial)
char *sv2c_config_filepath;
char *sv2c_script_filepath;

// ASE-APP run command
char *app_run_cmd;

// ASE Mode macros
#define ASE_MODE_DAEMON_NO_SIMKILL   1
#define ASE_MODE_DAEMON_SIMKILL      2
#define ASE_MODE_DAEMON_SW_SIMKILL   3
#define ASE_MODE_REGRESSION          4

// UMAS establishment status
#define UMAS_NOT_ESTABLISHED 0x0
#define UMAS_ESTABLISHED     0xBEEF

/*
 * Console colors
 */
// ERROR codes are in RED color
#define BEGIN_RED_FONTCOLOR    printf("\033[1;31m");
#define END_RED_FONTCOLOR      printf("\033[0m");

// INFO or INSTRUCTIONS are in GREEN color
#define BEGIN_GREEN_FONTCOLOR  printf("\033[32;1m");
#define END_GREEN_FONTCOLOR    printf("\033[0m");

// WARNING codes in YELLOW color
#define BEGIN_YELLOW_FONTCOLOR printf("\033[0;33m");
#define END_YELLOW_FONTCOLOR   printf("\033[0m");

/*
 * ASE Error codes
 */
#define ASE_USR_CAPCM_NOINIT           0x1    // CAPCM not initialized
#define ASE_OS_MQUEUE_ERR              0x2    // MQ open error
#define ASE_OS_SHM_ERR                 0x3    // SHM open error
#define ASE_OS_FOPEN_ERR               0x4    // Normal fopen failure
#define ASE_OS_MEMMAP_ERR              0x5    // Memory map/unmap errors
#define ASE_OS_MQTXRX_ERR              0x6    // MQ send receive error
#define ASE_OS_MALLOC_ERR              0x7    // Malloc error
#define ASE_OS_STRING_ERR              0x8    // String operations error
#define ASE_IPCKILL_CATERR             0xA    // Catastropic error when cleaning
                                              // IPCs, manual intervention required
#define ASE_UNDEF_ERROR                0xFF   // Undefined error, pls report

// Remote Start Stop messages
/* #define HDR_ASE_READY_STAT   0xFACEFEED */
/* #define HDR_ASE_KILL_CTRL    0xC00CB00C */

// Simkill message
#define ASE_SIMKILL_MSG      0xDEADDEAD


/* *******************************************************************************
 *
 * Shared buffer 
 * 
 * ******************************************************************************/
// Buffer information structure
struct buffer_t                   //  Descriptiion                    Computed by
{                                 // --------------------------------------------
  int fd_app;                     // File descriptor                 |   APP
  int fd_ase;                     // File descriptor                 |   SIM
  int index;                      // Tracking id                     | INTERNAL
  int valid;                      // Valid buffer indicator          | INTERNAL
  int metadata;                   // MQ marshalling command          | INTERNAL
  char memname[ASE_SHM_NAME_LEN]; // Shared memory name              | INTERNAL
  uint32_t memsize;               // Memory size                     |   APP
  uint64_t vbase;                 // SW virtual address              |   APP
  uint64_t pbase;                 // SIM virtual address             |   SIM
  uint64_t fake_paddr;            // unique low FPGA_ADDR_WIDTH addr |   SIM
  uint64_t fake_paddr_hi;         // unique hi FPGA_ADDR_WIDTH addr  |   SIM
  int is_privmem;                 // Flag memory as a private memory |    
  int is_csrmap;                  // Flag memory as DSM              |   
  int is_umas;                    // Flag memory as UMAS region      |
  struct buffer_t *next;
};

// Compute buffer_t size 
#define BUFSIZE     sizeof(struct buffer_t)


// Head and tail pointers of DPI side Linked list
extern struct buffer_t *head;      // Head pointer
extern struct buffer_t *end;       // Tail pointer
// CSR fake physical base address
extern uint64_t csr_fake_pin;      // Setting up a pinned fake_paddr (contiguous)
// DPI side CSR base, offsets updated on CSR writes
extern uint32_t *ase_csr_base;      
// Timestamp reference time
extern struct timeval start;

// ASE buffer valid/invalid indicator
// When a buffer is 'allocated' successfully, it will be valid, when
// it is deallocated, it will become invalid.
#define ASE_BUFFER_VALID        0xFFFF
#define ASE_BUFFER_INVALID      0x0

// Buffer allocate/deallocate message headers
#define HDR_MEM_ALLOC_REQ    0x7F
#define HDR_MEM_ALLOC_REPLY  0xFF
#define HDR_MEM_DEALLOC_REQ  0x0F

// UMSG info structure
typedef struct {
  int id;
  int hint;
  char data[CL_BYTE_WIDTH];
} umsg_pack_t;

// Size map
#define SIZEOF_UMSG_PACK_T    sizeof(umsg_pack_t)


/* ********************************************************************
 *
 * FUNCTION PROTOTYPES
 *
 * ********************************************************************/
// Linked list functions
void ll_print_info(struct buffer_t *);
void ll_traverse_print();
void ll_append_buffer(struct buffer_t *);
void ll_remove_buffer(struct buffer_t *);
struct buffer_t* ll_search_buffer(int);
uint32_t check_if_physaddr_used(uint64_t);

// Mem-ops functions
void ase_mqueue_setup();
void ase_mqueue_teardown();
int ase_recv_msg(struct buffer_t *);
void ase_alloc_action(struct buffer_t *);
void ase_dealloc_action(struct buffer_t *);
void ase_destroy();
uint64_t* ase_fakeaddr_to_vaddr(uint64_t);
void ase_dbg_memtest(struct buffer_t *);
void ase_perror_teardown();
void ase_empty_buffer(struct buffer_t *);
uint64_t get_range_checked_physaddr(uint32_t);
void ase_write_seed(uint64_t);
uint64_t ase_read_seed();
void ase_memory_barrier();

// ASE operations
void ase_buffer_info(struct buffer_t *);
void ase_buffer_oneline(struct buffer_t *);
void ase_buffer_t_to_str(struct buffer_t *, char *);
void ase_str_to_buffer_t(char *, struct buffer_t *);
int ase_dump_to_file(struct buffer_t*, char*);
uint64_t ase_rand64();
char* ase_eval_session_directory();

// Message queue operations
void ipc_init();
void mqueue_create(char*);
int mqueue_open(char*, int);
void mqueue_close(int);
void mqueue_destroy(char*);
void mqueue_send(int, char*);
int mqueue_recv(int, char*);

// Debug interface
void shm_dbg_memtest(struct buffer_t *);

// Timestamp functions
void put_timestamp();
char* get_timestamp(int);
char* generate_tstamp_path(char*);

// Error report functions
void ase_error_report(char *, int , int );

// IPC management functions
void final_ipc_cleanup();
void add_to_ipc_list(char *, char *);
void create_ipc_listfile();

/*
 * These functions are called by C++ AALSDK Applications
 */
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
  // Shared memory alloc/dealloc operations
  void session_init();
  void session_deinit();
  void allocate_buffer(struct buffer_t *);
  void deallocate_buffer(struct buffer_t *);
  void csr_write(uint32_t, uint32_t);
  uint32_t csr_read(uint32_t);
  // SPL bridge functions *FIXME*
  void setup_spl_cxt_pte(struct buffer_t *, struct buffer_t *);
  void spl_driver_dsm_setup(struct buffer_t *);
  void spl_driver_reset(struct buffer_t *);
  void spl_driver_afu_setup(struct buffer_t *);
  void spl_driver_start(uint64_t *);
  void spl_driver_stop();
  // UMSG subsystem
  void umas_init(uint32_t);
  void umsg_send(int, char *);
  void umas_deinit();
#ifdef __cplusplus
}
#endif // __cplusplus

#define DUMPSTRVAR(varname) fprintf(fp_workspace_log, "%s", #varname);


/* ********************************************************************
 *
 * MESSAGING IPC
 *
 * ********************************************************************/
// Buffer exchange messages
/* #define APP2SIM_SMQ_PREFIX          "app2sim_bufping_smq." */
/* #define SIM2APP_SMQ_PREFIX          "sim2app_bufpong_smq." */
/* // CSR write messages */
/* #define APP2SIM_CSR_WR_SMQ_PREFIX   "app2sim_csr_wr_smq." */
/* // UMSG control messages from APP to SIM */
/* #define APP2SIM_UMSG_SMQ_PREFIX     "app2sim_umsg_smq." */
/* // Interrupt message from SIM to APP */
/* #if 0 */
/* #define SIM2APP_INTR_SMQ_PREFIX     "sim2app_intr_smq." */
/* #endif */
/* // Simkill control messages */
/* #define APP2SIM_SIMKILL_SMQ_PREFIX  "app2sim_simkill_smq." */

// Message Queue establishment status
#define MQ_NOT_ESTABLISHED 0x0
#define MQ_ESTABLISHED     0xCAFE

// Message queue parameters
#define ASE_MQ_MAXMSG     8
#define ASE_MQ_MSGSIZE    1024
#define ASE_MQ_NAME_LEN   64
#define ASE_MQ_INSTANCES  5

// Message presence setting
#define ASE_MSG_PRESENT 0xD33D
#define ASE_MSG_ABSENT  0xDEAD

// Message queue controls
struct ipc_t
{
  char name[ASE_MQ_NAME_LEN];
  char path[ASE_FILEPATH_LEN];
  int  perm_flag;
};
struct ipc_t mq_array[ASE_MQ_INSTANCES];


/* ********************************************************************
 *
 * DEBUG STRUCTURES
 *
 * ********************************************************************/
// Enable function call entry/exit
// Extremely noisy debug feature to watch function entry/exit
// #define ENABLE_ENTRY_EXIT_WATCH
#ifdef  ENABLE_ENTRY_EXIT_WATCH
#define FUNC_CALL_ENTRY printf("--- ENTER: %s ---\n", __FUNCTION__);
#define FUNC_CALL_EXIT  printf("--- EXIT : %s ---\n", __FUNCTION__);
#else
#define FUNC_CALL_ENTRY
#define FUNC_CALL_EXIT
#endif

// ---------------------------------------------------------------------
// Enable memory test function
// ---------------------------------------------------------------------
// Basic Memory Read/Write test feature (runs on allocate_buffer)
// Leaving this setting ON automatically scrubs memory (sets 0s)
// Read shm_dbg_memtest() and ase_dbg_memtest()
// #define ASE_MEMTEST_ENABLE


// ------------------------------------------------------------------
// DANGEROUS/BUGGY statements - uncomment prudently (OPEN ISSUES)
// These statements have screwed data structures during testing
// WARNING: Uncomment only if you want to debug these statements.
// ------------------------------------------------------------------
// free(void*) : Free a memory block, "*** glibc detected ***"
//#define ENABLE_FREE_STATEMENT


// ------------------------------------------------------------------
// Triggers, safety catches and debug information used in the AFU
// simulator environment.
// ------------------------------------------------------------------
// ASE message view #define - Print messages as they go around
// #define ASE_MSG_VIEW

// Enable debug info from linked lists 
// #define ASE_LL_VIEW

// Print buffers as they are being alloc/dealloc
// *FIXME*: Connect to ase.cfg
// #define ASE_BUFFER_VIEW



/* *********************************************************************
 *
 * SIMULATION-ONLY (SIM_SIDE) declarations
 * - This is available only in simulation side 
 * - This compiled in when SIM_SIDE is set to 1
 *
 * *********************************************************************/
#ifdef SIM_SIDE

/*
 * ASE config structure
 * This will reflect ase.cfg
 */
struct ase_cfg_t
{
  int ase_mode;
  int ase_timeout;
  int ase_num_tests;
  int enable_reuse_seed;
  int num_umsg_log2;
  int enable_cl_view;
  int enable_capcm;
  int memmap_sad_setting;
};
struct ase_cfg_t *cfg;

// ASE config file
#define ASE_CONFIG_FILE "ase.cfg"

// ASE seed file
#define ASE_SEED_FILE  "ase_seed.txt"

/* 
 * Data-exchange functions and structures
 */
// CCI transaction packet
typedef struct {
  long long meta;
  long long qword[8];
  int       cfgvalid;
  int       wrvalid;
  int       rdvalid;
  int       intrvalid;
  int       umsgvalid; 	       
} cci_pkt;


/*
 * Function prototypes
 */
// DPI-C export(C to SV) calls
extern void simkill();
extern void sw_simkill_request();
extern void csr_write_init();
extern void csr_write_dispatch(int, int, int);
extern void buffer_messages(char *);
/* extern void umsg_init(); */
extern void umsg_dispatch(int, int, int, int, char*);
extern void ase_config_dex(struct ase_cfg_t *);

// DPI-C import(SV to C) calls
int ase_init();
int ase_ready();
int ase_listener();
void ase_config_parse(char*);

// Simulation control function
void start_simkill_countdown();
void run_clocks(int num_clocks);

// CSR Write 
void csr_write_dex(cci_pkt *csr);
void csr_write_completed();

// Read system memory line
void rd_memline_dex(cci_pkt *pkt, int *cl_addr, int *mdata );
// Write system memory line
void wr_memline_dex(cci_pkt *pkt, int *cl_addr, int *mdata, char *wr_data );

// CAPCM functions
extern void capcm_init();

// UMSG functions
void ase_umsg_init();
/* int umsg_listener(); */
void ase_umsg_init();


/*
 * Request/Response options
 */ 
// TX0 channel
#define ASE_TX0_RDLINE       0x4
// TX1 channel
#define ASE_TX1_WRTHRU       0x1
#define ASE_TX1_WRLINE       0x2
#define ASE_TX1_WRFENCE      0x5   // CCI 1.8
#define ASE_TX1_INTRVALID    0x8   // CCI 1.8
// RX0 channel
#define ASE_RX0_CSR_WRITE    0x0
#define ASE_RX0_WR_RESP      0x1
#define ASE_RX0_RD_RESP      0x4
#define ASE_RX0_INTR_CMPLT   0x8   // CCI 1.8
#define ASE_RX0_UMSG         0xf   // CCI 1.8
// RX1 channel
#define ASE_RX1_WR_RESP      0x1
#define ASE_RX1_INTR_CMPLT   0x8   // CCI 1.8


/*
 * ASE Ready session control files, for wrapping with autorun script
 */
FILE *ase_ready_fd;
#define ASE_READY_FILENAME ".ase_ready"

// ASE seed 
uint64_t ase_addr_seed;

// ASE error file
FILE *error_fp;

/*
 * QPI-CA private memory implementation
 *
 * Caching agent private memory is enabled in hw/platform.vh. This
 * block is enabled only in the simulator, application has no need to
 * see this.  The buffer is implemented in /dev/shm for performance
 * reasons. Linux swap space is used to make memory management very
 * efficient.
 *
 * CAPCM_BASENAME : Memory basename is concatenated with index and a
 * timestamp
 * CAPCM_CHUNKSIZE : Large CA private memories are chained together in
 * default 1 GB chunks.
 */
#define CAPCM_BASENAME "/capcm"
#define CAPCM_CHUNKSIZE (1024*1024*1024UL)
uint64_t capcm_num_buffers;

// CAPCM buffer chain info (each buffer holds 1 GB)
struct buffer_t *capcm_buf;


/*
 * IPC cleanup on catastrophic errors
 */
#define IPC_LOCAL_FILENAME ".ase_ipc_local"
FILE *local_ipc_fp;

/*
 * Physical Memory ranges for PrivMem & SysMem
 */
// System Memory
uint64_t sysmem_size;
uint64_t sysmem_phys_lo;
uint64_t sysmem_phys_hi;

// CAPCM
uint64_t capcm_size;
uint64_t capcm_phys_lo;
uint64_t capcm_phys_hi;

// ASE PID
int ase_pid;

// Workspace information log (information dump of 
FILE *fp_workspace_log;

#endif
#endif
