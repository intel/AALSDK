//******************************************************************************
// Part of the Intel(R) QuickAssist Technology Accelerator Abstraction Layer
//
// This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
//         redistributing this file, you may do so under either license.
//
//                            GPL LICENSE SUMMARY
//
//  Copyright(c) 2012-2015, Intel Corporation.
//
//  This program  is  free software;  you  can redistribute it  and/or  modify
//  it  under  the  terms of  version 2 of  the GNU General Public License  as
//  published by the Free Software Foundation.
//
//  This  program  is distributed  in the  hope that it  will  be useful,  but
//  WITHOUT   ANY   WARRANTY;   without   even  the   implied   warranty    of
//  MERCHANTABILITY  or  FITNESS  FOR  A  PARTICULAR  PURPOSE.  See  the   GNU
//  General Public License for more details.
//
//  The  full  GNU  General Public License is  included in  this  distribution
//  in the file called README.GPLV2-LICENSE.TXT.
//
//  Contact Information:
//  Henry Mitchel, henry.mitchel at intel.com
//  77 Reed Rd., Hudson, MA  01749
//
//                                BSD LICENSE
//
//  Copyright(c) 2012-2015, Intel Corporation.
//
//  Redistribution and  use  in source  and  binary  forms,  with  or  without
//  modification,  are   permitted  provided  that  the  following  conditions
//  are met:
//
//    * Redistributions  of  source  code  must  retain  the  above  copyright
//      notice, this list of conditions and the following disclaimer.
//    * Redistributions in  binary form  must  reproduce  the  above copyright
//      notice,  this  list of  conditions  and  the  following disclaimer  in
//      the   documentation   and/or   other   materials   provided  with  the
//      distribution.
//    * Neither   the  name   of  Intel  Corporation  nor  the  names  of  its
//      contributors  may  be  used  to  endorse  or promote  products derived
//      from this software without specific prior written permission.
//
//  THIS  SOFTWARE  IS  PROVIDED  BY  THE  COPYRIGHT HOLDERS  AND CONTRIBUTORS
//  "AS IS"  AND  ANY  EXPRESS  OR  IMPLIED  WARRANTIES,  INCLUDING,  BUT  NOT
//  LIMITED  TO, THE  IMPLIED WARRANTIES OF  MERCHANTABILITY  AND FITNESS  FOR
//  A  PARTICULAR  PURPOSE  ARE  DISCLAIMED.  IN NO EVENT  SHALL THE COPYRIGHT
//  OWNER OR CONTRIBUTORS BE LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,
//  SPECIAL,  EXEMPLARY,  OR  CONSEQUENTIAL   DAMAGES  (INCLUDING,   BUT   NOT
//  LIMITED  TO,  PROCUREMENT  OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF USE,
//  DATA,  OR PROFITS;  OR BUSINESS INTERRUPTION)  HOWEVER  CAUSED  AND ON ANY
//  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT LIABILITY,  OR TORT
//  (INCLUDING  NEGLIGENCE  OR OTHERWISE) ARISING  IN ANY WAY  OUT  OF THE USE
//  OF  THIS  SOFTWARE, EVEN IF ADVISED  OF  THE  POSSIBILITY  OF SUCH DAMAGE.
//******************************************************************************
//****************************************************************************
/// @file kosal.h
/// @brief Defines the scalar type abstraction used by AAL.
/// @ingroup kOSAL
/// @verbatim
/// Intel(R) QuickAssist Technology Accelerator Abstraction Layer
///          OS Kernel Abstraction Layer library definitions
///
///  AUTHORS:  Joseph Grecco, Intel Corporation
///            Tim Whisonant, Intel Corporation
///
/// PURPOSE:
/// HISTORY:
/// WHEN:          WHO:     WHAT:
/// 12/27/2012     JG       Initial version started
//****************************************************************************
#include <aalsdk/kernel/aaltypes.h>

#ifndef PCI_VENDOR_ID_INTEL
# define PCI_VENDOR_ID_INTEL 0x8086
#endif // PCI_VENDOR_ID_INTEL

#if defined( __AAL_KERNEL__ )

#ifndef __AALSDK_KERNEL_KOSAL_H__
#define __AALSDK_KERNEL_KOSAL_H__

USING_NAMESPACE(AAL)

#ifdef __AAL_UNKNOWN_OS__
# error Update kosal for unknown OS.
#endif // __AAL_UNKNOWN_OS__


//
// Temporary Workarounds - TO BE REMOVED
//
#if   defined( __AAL_LINUX__ )

#define UNREFERENCED_PARAMETER(p)

#elif defined( __AAL_WINDOWS__ )

#endif // OS


//
// Modules
//
#if   defined( __AAL_LINUX__ )

#include <linux/kernel.h>
#include <linux/module.h>
// Used for reference counting
typedef struct module *kosal_ownermodule;

#elif defined( __AAL_WINDOWS__ )

// Undefined - Used for reference counting
typedef void * kosal_ownermodule;
#endif


//
// Debug printing
//
#define KOSAL_DBG_MOD      ((btUnsignedInt)1 <<  0)

#define AALBUS_DBG_MOD     ((btUnsignedInt)1 <<  0)
#define AALBUS_DBG_FILE    ((btUnsignedInt)1 <<  1)
#define AALBUS_DBG_MMAP    ((btUnsignedInt)1 <<  2)
#define AALBUS_DBG_IOCTL   ((btUnsignedInt)1 <<  3)

#define AALRMC_DBG_MOD     ((btUnsignedInt)1 <<  0)
#define AALRMC_DBG_FILE    ((btUnsignedInt)1 <<  1)
#define AALRMC_DBG_MMAP    ((btUnsignedInt)1 <<  2)
#define AALRMC_DBG_IOCTL   ((btUnsignedInt)1 <<  3)

#define AALRMS_DBG_MOD     ((btUnsignedInt)1 <<  0)
#define AALRMS_DBG_FILE    ((btUnsignedInt)1 <<  1)
#define AALRMS_DBG_MMAP    ((btUnsignedInt)1 <<  2)
#define AALRMS_DBG_IOCTL   ((btUnsignedInt)1 <<  3)

#define AALWSMGR_DBG_MOD   ((btUnsignedInt)1 <<  0)
#define AALWSMGR_DBG_FILE  ((btUnsignedInt)1 <<  1)
#define AALWSMGR_DBG_MMAP  ((btUnsignedInt)1 <<  2)
#define AALWSMGR_DBG_IOCTL ((btUnsignedInt)1 <<  3)

#define AALWKSP_DBG_MOD    ((btUnsignedInt)1 <<  4)

#define AHMPIP_DBG_MOD     ((btUnsignedInt)1 <<  0)
#define AHMPIP_DBG_FILE    ((btUnsignedInt)1 <<  1)
#define AHMPIP_DBG_IOCTL   ((btUnsignedInt)1 <<  2)
#define AHMPIP_DBG_HWTASK  ((btUnsignedInt)1 <<  3)
#define AHMPIP_DBG_MEM     ((btUnsignedInt)1 <<  4)
#define AHMPIP_DBG_SESS    ((btUnsignedInt)1 <<  5)
#define AHMPIP_DBG_DEV     ((btUnsignedInt)1 <<  6)
#define AHMPIP_DBG_AUTOD   ((btUnsignedInt)1 <<  7)
#define AHMPIP_DBG_TIMEOUT ((btUnsignedInt)1 <<  8)
#define AHMPIP_DBG_MSGHDLR ((btUnsignedInt)1 <<  9)
#define AHMPIP_DBG_MMAP    ((btUnsignedInt)1 << 10)

#define UIDRV_DBG_MOD      ((btUnsignedInt)1 <<  0)
#define UIDRV_DBG_FILE     ((btUnsignedInt)1 <<  1)
#define UIDRV_DBG_MMAP     ((btUnsignedInt)1 <<  2)
#define UIDRV_DBG_IOCTL    ((btUnsignedInt)1 <<  3)

#define ENCODER_DBG_MOD    ((btUnsignedInt)1 <<  0)
#define ENCODER_DBG_AFU    ((btUnsignedInt)1 <<  1)
#define ENCODER_DBG_MMAP   ((btUnsignedInt)1 <<  2)

#define HBAFU_DBG_MOD      ((btUnsignedInt)1 <<  0)
#define HBAFU_DBG_AFU      ((btUnsignedInt)1 <<  1)
#define HBAFU_DBG_MAFU     ((btUnsignedInt)1 <<  2)

#define QPIDKSM_DBG_MOD    ((btUnsignedInt)1 <<  0)
#define QPIDKSM_DBG_MMAP   ((btUnsignedInt)1 <<  1)
#define QPIDKSM_DBG_DEV    ((btUnsignedInt)1 <<  2)
#define QPIDKSM_DBG_AFU    ((btUnsignedInt)1 <<  3)
#define QPIDKSM_DBG_MAFU   ((btUnsignedInt)1 <<  4)
#define QPIDKSM_DBG_CMD    ((btUnsignedInt)1 <<  5)

#define SPL2_DBG_MOD       ((btUnsignedInt)1 <<  0)
#define SPL2_DBG_DEV       ((btUnsignedInt)1 <<  1)
#define SPL2_DBG_AFU       ((btUnsignedInt)1 <<  2)
#define SPL2_DBG_MAFU      ((btUnsignedInt)1 <<  3)
#define SPL2_DBG_MMAP      ((btUnsignedInt)1 <<  4)
#define SPL2_DBG_CMD       ((btUnsignedInt)1 <<  5)
#define SPL2_DBG_CFG       ((btUnsignedInt)1 <<  6)

#define FAPDIAG_DBG_MOD    ((btUnsignedInt)1 <<  0)


#if (1 == ENABLE_DEBUG)
extern btUnsignedInt debug; // Bit mask defined by module, DRV_NAME similarly
#endif // ENABLE_DEBUG

#define KOSAL_PRINTK_DRV_NAME_FMT "10s"
#define KOSAL_PRINTK_FILE_FMT     "21s"
#define KOSAL_PRINTK_LINE_FMT      "4d"
#define KOSAL_PRINTK_FN_FMT       "35s"

#if   defined( __AAL_LINUX__ )
# define KOSAL_PRINTK_PID_FMT "5d"
# define KOSAL_PRINTK_TID_FMT "5llu"
# define kosal_printk_level_here(_level, _file, _line, _fn, _fmt, ...)                                                                                                                                  \
   printk(_level "[%" KOSAL_PRINTK_DRV_NAME_FMT ",%" KOSAL_PRINTK_FILE_FMT ",%" KOSAL_PRINTK_LINE_FMT ",%" KOSAL_PRINTK_FN_FMT "(),pid=%" KOSAL_PRINTK_PID_FMT ",tid=%" KOSAL_PRINTK_TID_FMT "] " _fmt, \
             DRV_NAME, _file, _line, _fn, kosal_get_pid(), kosal_get_tid(), __VA_ARGS__)
# if (1 == ENABLE_DEBUG)
#    define kosal_printk_level(_level, _fmt, ...)                                                                                                                                                            \
        printk(_level "[%" KOSAL_PRINTK_DRV_NAME_FMT ",%" KOSAL_PRINTK_FILE_FMT ",%" KOSAL_PRINTK_LINE_FMT ",%" KOSAL_PRINTK_FN_FMT "(),pid=%" KOSAL_PRINTK_PID_FMT ",tid=%" KOSAL_PRINTK_TID_FMT "] " _fmt, \
        		DRV_NAME, __AAL_SHORT_FILE__, __LINE__, __AAL_FUNC__, kosal_get_pid(), kosal_get_tid(), ##__VA_ARGS__)

# else
#    define kosal_printk_level(_level, _fmt, ...) printk(_level "[%" KOSAL_PRINTK_DRV_NAME_FMT "] " _fmt, DRV_NAME, __VA_ARGS__)
# endif // ENABLE_DEBUG
#elif defined( __AAL_WINDOWS__ )
// wdm.h
# define KERN_EMERG   0
# define KERN_ALERT   1
# define KERN_CRIT    2
# define KERN_ERR     3
# define KERN_WARNING 4
# define KERN_NOTICE  5
# define KERN_INFO    6
# define KERN_DEBUG   7
# define KERN_DEFAULT 8
# define KERN_CONT    9
# define kosal_printk_level_here(_level, _file, _line, _fn, _fmt, ...)                                                                               \
   DbgPrint("[%" KOSAL_PRINTK_DRV_NAME_FMT ",%" KOSAL_PRINTK_FILE_FMT ",%" KOSAL_PRINTK_LINE_FMT ",%" KOSAL_PRINTK_FN_FMT "(),pid=%u,tid=%u] " _fmt, \
               DRV_NAME, _file, _line, _fn, kosal_get_pid(), kosal_get_tid(), __VA_ARGS__)
# if (1 == ENABLE_DEBUG)
#    define kosal_printk_level(_level, _fmt, ...)                                                                                                         \
        DbgPrint("[%" KOSAL_PRINTK_DRV_NAME_FMT ",%" KOSAL_PRINTK_FILE_FMT ",%" KOSAL_PRINTK_LINE_FMT ",%" KOSAL_PRINTK_FN_FMT "(),pid=%u,tid=%u] " _fmt, \
                    DRV_NAME, __AAL_SHORT_FILE__, __LINE__, __AAL_FUNC__, kosal_get_pid(), kosal_get_tid(), ## __VA_ARGS__)
# else
#    define kosal_printk_level(_level, _fmt, ...) DbgPrint("[%" KOSAL_PRINTK_DRV_NAME_FMT "] " _fmt, DRV_NAME, __VA_ARGS__)
# endif // ENABLE_DEBUG
#endif // OS

#define kosal_printk(_fmt, ...) kosal_printk_level(KERN_ERR, _fmt, ## __VA_ARGS__)


#if (1 == ENABLE_DEBUG)
# define DPRINTF(_level, _fmt, ...)                                       \
do                                                                        \
{                                                                         \
   if ( flag_is_set(debug, _level) ) { kosal_printk(_fmt, ##__VA_ARGS__); } \
}while(0)
#else
# ifdef DPRINTF
#    undef DPRINTF
# endif // DPRINTF
# define DPRINTF(_level, _fmt, ...) do{}while(0)
#endif // ENABLE_DEBUG

//=============================================================================
//=============================================================================
// PDEBUG & Friends, including PTRACE, PINFO, PNOTICE, PWARNING, PERR, PCRIT,...
// These only exist if the module was built with the DEBUG flag
//=============================================================================
//=============================================================================
//
//------>>   unsigned debug; and DRV_NAME are expected to be #defined  <<------
//------>>    and resolvable in each file that includes this header    <<------
//
//=============================================================================
//=============================================================================
// USAGE:
//
// For each logical module to be debugged individually, define a bit flag, as so:

/* (sans comments, of course)
#define MEMMGR_DBG_MOD   (1 << 0)
#define MEMMGR_DBG_MEM   (1 << 1)
#define MEMMGR_DBG_MMAP  (1 << 2)
#define MEMMGR_DBG_IOCTL (1 << 3)
*/

//
// And in the place where the int debug variable is defined, use a statement such
//    as this:  [just use the stuff in the #if block -- it is there to prevent problems]

/* (sans comments, of course)
unsigned debug = 0
// Type and Level selection flags
   | PTRACE_FLAG
   | PVERBOSE_FLAG
   | PDEBUG_FLAG
   | PINFO_FLAG
   | PNOTICE_FLAG
// Module selection flags
   | MEMMGR_DBG_MOD
   | MEMMGR_DBG_MEM
   | MEMMGR_DBG_MMAP
   | MEMMGR_DBG_IOCTL
;
*/

//
// Comment out any of the items you do not wish to trace.
//
// PTRACE_FLAG enables PTRACEIN/OUT and friends to show entry/exit of functions.
//    Caveat: PTRACE prints at KERN_DEBUG level, so if syslog is blocking KERN_DEBUG and PTRACE is also blocked
//
// PVERBOSE_FLAG to PNOTICE_FLAG set the levels that are allowed to be printed, independently of the
//    syslog level setting. The reason is that we generally debug with syslog set to DEBUG, but in fact that
//    can be overly verbose. So, once the sub-system is working fairly well, I will disable the more verbose
//    levels.
//    Caveat: at the current time each level is a bitflag and so it is possible (although not likely what
//       you would want to do) to print out VERBOSE and NOTICE levels but not DEBUG or INFO by setting the
//       appropriate bits.
//
// Syslog levels of WARNING and above do not have module selection or specific level flags, since
//    anything at that level is (presumably) important enough to always print and also not voluminous.
//
//=============================================================================
//=============================================================================
                                 // Disabling KERN_DEBUG in syslog also disables VERBOSE and TRACE
#define PTRACE_FLAG     ((btUnsignedInt)1 << 31)  // Add this to the debug flags if you want PTRACING
                                 //    0x8000_0000
#define PVERBOSE_FLAG   ((btUnsignedInt)1 << 30)  // Add this to the debug flags if you want VERBOSE debugging
                                 //    0x4000_0000
#define PDEBUG_FLAG     ((btUnsignedInt)1 << 29)  // Add this to the debug flags if you want DEBUG debugging
                                 //    0x2000_0000
#define PINFO_FLAG      ((btUnsignedInt)1 << 28)  // Add this to the debug flags if you want INFO debugging
                                 //    0x1000_0000
#define PNOTICE_FLAG    ((btUnsignedInt)1 << 27)  // Add this to the debug flags if you want NOTICE debugging
                                 //    0x0800_0000

#define PMEMORY_FLAG    ((btUnsignedInt)1 << 26)  // Add this to the debug flags if you want memory alloc/free debugging
                                 //    0x0400_0000
#define PPCI_FLAG       ((btUnsignedInt)1 << 25)  // Add this to the debug flags if you want PCI(e) OS calls debugging
                                 //    0x0200_0000
#define PPOLLING_FLAG   ((btUnsignedInt)1 << 24)  // Add this to the debug flags if you want poll/spin calls debugging
                                 //    0x0100_0000

///////////////////////////////////////////////////////////////////////////////
//
// DEBUG PRINT MACROS that test debug flag for level before printing
//
// DRV_NAME must be defined; typically in the module-int.h file
// MODULE_FLAGS must be defined to be one of the Module Selection Flags,
//    e.g. MEMMGR_DBG_MOD. MODULE_FLAGS should be defined in each *.c file
//    BEFORE #including <kosal.h>
//    This makes all print statements in the file and in all header files
//       included by the file all have consistent MODULE_FLAGS.
//    Being able to properly handle #included files is a new feature that did
//       not work well with the older system
//
///////////////////////////////////////////////////////////////////////////////

#if (1 == ENABLE_DEBUG)
// Prints if syslog >= KERN_DEBUG && specific module flag set
# define PDEBUG(_fmt, ...) \
   if ( flags_are_set(debug, MODULE_FLAGS|PDEBUG_FLAG) ) { kosal_printk_level(KERN_DEBUG, "d:" _fmt, ##__VA_ARGS__); }

// Prints if syslog >= KERN_DEBUG && specific module flag set && VERBOSE flag set
# define PVERBOSE(_fmt, ...) \
   if ( flags_are_set(debug, MODULE_FLAGS|PVERBOSE_FLAG) ) { kosal_printk_level(KERN_DEBUG, "v:" _fmt, ##__VA_ARGS__); }

// Prints if syslog >= KERN_INFO && specific module flag set
# define PINFO(_fmt, ...) \
   if ( flags_are_set(debug, MODULE_FLAGS|PINFO_FLAG) ) { kosal_printk_level(KERN_INFO, "i:" _fmt, ##__VA_ARGS__); }

// Prints if syslog >= KERN_NOTICE && specific module flag set
# define PNOTICE(_fmt, ...) \
   if ( flags_are_set(debug, MODULE_FLAGS|PNOTICE_FLAG) ) { kosal_printk_level(KERN_NOTICE, "n:" _fmt, ##__VA_ARGS__); }

// Prints if syslog >= KERN_DEBUG && specific module flag set
# if (1 == ENABLE_ASSERT)
#    define PMEMORY_HERE(_fmt, ...) \
        if ( flags_are_set(debug, MODULE_FLAGS|PMEMORY_FLAG) ) { kosal_printk_level_here(KERN_DEBUG, __file, __line, __fn, "mem:" _fmt, __VA_ARGS__); }
# else
#    define PMEMORY_HERE(_fmt, ...)
# endif // ENABLE_ASSERT
# define PMEMORY(_fmt, ...) \
   if ( flags_are_set(debug, MODULE_FLAGS|PMEMORY_FLAG) ) { kosal_printk_level(KERN_DEBUG, "mem:" _fmt, ##__VA_ARGS__); }

// Prints if syslog >= KERN_DEBUG && specific module flag set
# if (1 == ENABLE_ASSERT)
#    define PPCI_HERE(_fmt, ...) \
        if ( flags_are_set(debug, MODULE_FLAGS|PPCI_FLAG) ) { kosal_printk_level_here(KERN_DEBUG, __file, __line, __fn, "pci:" _fmt, __VA_ARGS__); }
# else
#    define PPCI_HERE(_fmt, ...)
# endif // ENABLE_ASSERT
# define PPCI(_fmt, ...) \
   if ( flags_are_set(debug, MODULE_FLAGS|PPCI_FLAG) ) { kosal_printk_level(KERN_DEBUG, "pci:" _fmt, ##__VA_ARGS__); }

// Prints if syslog >= KERN_DEBUG && specific module flag set
# if (1 == ENABLE_ASSERT)
#    define PPOLLING_HERE(_fmt, ...) \
        if ( flags_are_set(debug, MODULE_FLAGS|PPOLLING_FLAG) ) { kosal_printk_level_here(KERN_DEBUG, __file, __line, __fn, "poll:" _fmt, __VA_ARGS__); }
# else
#    define PPOLLING_HERE(_fmt, ...)
# endif // ENABLE_ASSERT
# define PPOLLING(_fmt, ...) \
   if ( flags_are_set(debug, MODULE_FLAGS|PPOLLING_FLAG) ) { kosal_printk_level(KERN_DEBUG, "poll:" _fmt, ##__VA_ARGS__); }

#else
# ifdef PDEBUG
#    undef PDEBUG
# endif // PDEBUG
# define PDEBUG(_fmt, ...)        0
# ifdef PVERBOSE
#    undef PVERBOSE
# endif // PVERBOSE
# define PVERBOSE(_fmt, ...)      0
# ifdef PINFO
#    undef PINFO
# endif // PINFO
# define PINFO(_fmt, ...)         0
# ifdef PNOTICE
#    undef PNOTICE
# endif // PNOTICE
# define PNOTICE(_fmt, ...)       0
# ifdef PMEMORY_HERE
#    undef PMEMORY_HERE
# endif // PMEMORY_HERE
# define PMEMORY_HERE(_fmt, ...)  0
# ifdef PMEMORY
#    undef PMEMORY
# endif // PMEMORY
# define PMEMORY(_fmt, ...)       0
# ifdef PPCI_HERE
#    undef PPCI_HERE
# endif // PPCI_HERE
# define PPCI_HERE(_fmt, ...)     0
# ifdef PPCI
#    undef PPCI
# endif // PPCI
# define PPCI(_fmt, ...)          0
# ifdef PPOLLING_HERE
#    undef PPOLLING_HERE
# endif // PPOLLING_HERE
# define PPOLLING_HERE(_fmt, ...) 0
# ifdef PPOLLING
#    undef PPOLLING
# endif // PPOLLING
# define PPOLLING(_fmt, ...)      0
#endif // ENABLE_DEBUG

///////////////////////////////////////////////////////////////////////////////
//
// DEBUG TRACE MACROS
//
///////////////////////////////////////////////////////////////////////////////

#if (1 == ENABLE_DEBUG)
// Prints if syslog >= KERN_INFO && PTRACE flag set
# define PTRACEIN \
   ( flag_is_set(debug, PTRACE_FLAG) ? kosal_printk_level(KERN_INFO, "t:Enter\n"), 1 : 0 )

// Prints if syslog >= KERN_INFO && PTRACE flag set
# define PTRACEOUT \
   ( flag_is_set(debug, PTRACE_FLAG) ? kosal_printk_level(KERN_INFO, "t:Exit\n"), 1 : 0 )

// Do not call this from a function -- it is internal worker for PTRACEOUT_type macros
# define _PTRACEOUT_RET(_retval, _printf_fmt) \
   ( flag_is_set(debug, PTRACE_FLAG) ? kosal_printk_level(KERN_INFO, "t:Exit %" #_printf_fmt "\n", _retval), 1 : 0 )

// Prints if syslog >= KERN_INFO && PTRACE flag set
// These are for specific types of return values, and will print them accordingly.
# define PTRACEOUT_INT(_retval)  _PTRACEOUT_RET(_retval, d)
# define PTRACEOUT_LINT(_retval) _PTRACEOUT_RET(_retval, ld)
# define PTRACEOUT_PTR(_retval)  _PTRACEOUT_RET(_retval, p)

#else
# ifdef PTRACEIN
#    undef PTRACEIN
# endif // PTRACEIN
# define PTRACEIN                0
# ifdef PTRACEOUT
#    undef PTRACEOUT
# endif // PTRACEOUT
# define PTRACEOUT               0
# ifdef PTRACEOUT_INT
#    undef PTRACEOUT_INT
# endif // PTRACEOUT_INT
# define PTRACEOUT_INT(_retval)  0
# ifdef PTRACEOUT_LINT
#    undef PTRACEOUT_LINT
# endif // PTRACEOUT_LINT
# define PTRACEOUT_LINT(_retval) 0
# ifdef PTRACEOUT_PTR
#    undef PTRACEOUT_PTR
# endif // PTRACEOUT_PTR
# define PTRACEOUT_PTR(_retval)  0
#endif // ENABLE_DEBUG

//=============================================================================
//=============================================================================
// PINFO & Friends, including PNOTICE, PWARNING, PERR, PCRIT,...
// These ALWAYS exist, and are used for standard always on printing
//
// DRV_NAME is expected to be #defined
//
//=============================================================================
//=============================================================================

#define PWARN(_fmt, ...)  ( kosal_printk_level(KERN_WARNING, "w:" _fmt, ##__VA_ARGS__), 1 )
#define PERR(_fmt, ...)   ( kosal_printk_level(KERN_ERR,     "e:" _fmt, ##__VA_ARGS__), 1 )
#define PCRIT(_fmt, ...)  ( kosal_printk_level(KERN_CRIT,    "c:" _fmt, ##__VA_ARGS__), 1 )
#define PALERT(_fmt, ...) ( kosal_printk_level(KERN_ALERT,   "a:" _fmt, ##__VA_ARGS__), 1 )
#define PEMERG(_fmt, ...) ( kosal_printk_level(KERN_EMERG,   "E:" _fmt, ##__VA_ARGS__), 1 )

//=============================================================================
//=============================================================================
//                         THOUGHTS FOR THE FUTURE
//=============================================================================
//=============================================================================
//
// CAVEAT: This is not a particularly clever implementation, and I do not yet (and may never) have an
//    implementation that allows levels specific to individual modules, which is probably what is
//    really needed. That would allow one to set VERBOSE for the current development module while
//    only looking at DEBUG or INFO level on already debugged modules. I think that is the holy grail.
// For example an easy way to do it would be to have separate debug variables for each sub-system.
// And/Or, for up to N modules in each sub-system, define each module as a 32/N slice, and specify levels
//    within that slice. E.g.
//    Assuming 5 flags needed define:
//       MEMMGR_MOD_TRACE
//       MEMMGR_MOD_VERBOSE
//       MEMMGR_MOD_DEBUG
//       MEMMGR_MOD_INFO
//       MEMMGR_MOD_NOTICE
//           ...
//       MEMMGR_MEM_TRACE
//       MEMMGR_MEM_VERBOSE
//       MEMMGR_MEM_DEBUG
//       MEMMGR_MEM_INFO
//       MEMMGR_MEM_NOTICE
//    and select against each of these individually ...
//
//    Something like:
//       #define MEMMGR_MOD 0
//       #define MEMMGR_MEM 5
//            ...
//       #define PTRACE_SHIFT   0
//       #define PVERBOSE_SHIFT 1
//       #define PDEBUG_SHIFT   2
//       #define PINFO_SHIFT    3
//       #define PNOTICE_SHIFT  4
//            ...
//       PTRACEIN(MEMMGR_MEM);   // test against bit (1 << [(MEMMGR_MEM + PTRACE_SHIFT) = 5])
//       PDEBUG  (MEMMGR_MEM);   // test against bit (1 << [(MEMMGR_MEM + PDEBUG_SHIFT) = 7])
//
//    and the debug variable definition becomes something like:
//       int debug = 0
//          | 1 << (MEMMGR_MOD + PTRACE_SHIFT)
//          | 1 << (MEMMGR_MOD + PVERBOSE_SHIFT)
//          | 1 << (MEMMGR_MOD + PDEBUG_SHIFT)
//          | 1 << (MEMMGR_MOD + PINFO_SHIFT)
//          | 1 << (MEMMGR_MOD + PNOTICE_SHIFT)
//          | 1 << (MEMMGR_MEM + PTRACE_SHIFT)
//          | 1 << (MEMMGR_MEM + PVERBOSE_SHIFT)
//          | 1 << (MEMMGR_MEM + PDEBUG_SHIFT)
//          | 1 << (MEMMGR_MEM + PINFO_SHIFT)
//          | 1 << (MEMMGR_MEM + PNOTICE_SHIFT)
//       ;
//    You now have very fine grained control at the module and debug level...
//
// Prints if syslog >= KERN_DEBUG && specific module flag set
//
// Only one flag is passed in, which is the module flag, and expected to
//    be one (or more -- but that kind of defeats the purpose) of 0, 5, 10, ...
/*
#define PDEBUG(module_flag, fmt, args...) do { \
   if ( (debug & ( (module_flag) << PDEBUG_SHIFT)) ) { \
      printk(KERN_DEBUG "d:" DRV_NAME ":%s[%d] " fmt, __AAL_FUNC__, current->tgid, ##args); \
   } \
} while (0)
*/
//
// The call is:
//    PDEBUG( MEMMGR_MEM, ...);
//
// The limitation is that one can define only 6 sub-modules with a 32-bit debug variable ...
//
//=============================================================================
// Another thing -- sometimes want multi-line output, e.g. some information with
//    all the goodies like function name and module, etc., but then a bunch of
//    lines that are not so long. Could enable it with a special flag added
//    specifically, e.g.:
//    PDEBUG( MEMMGR_MOD, < regular stuff > )
//    for () PDEBUG( MEMMGR_MOD+PCLEAN, < no header needed stuff >
//
//  The "no header needed stuff" still needs to go through the same checking, but it doesn't need all
//    the acouterments. Possible implementation is extra bit flag removed before check. Something like:
//    #define PCLEAN 1<<31
/*
#define PDEBUG(module_flag, fmt, args...) do { \
   if ( (debug & ( (module_flag) << PDEBUG_SHIFT)) ) { \
      char *pstr; \
      if ( module_flag & PCLEAR) pstr=KERN_DEBUG; \
      else pstr=KERN_DEBUG "d:" DRV_NAME ":%s[%d] " fmt; \
      printk( pstr, __AAL_FUNC__, current->tgid, ##args); \
   } \
} while (0)
*/
//=============================================================================
//=============================================================================


//
// Error Codes & Misc.
//
#if   defined( __AAL_LINUX__ )
# include <linux/version.h> // LINUX_VERSION_CODE
# include <linux/stddef.h>
# define kosal_offsetof(_type, _memb) offsetof(_type, _memb)
# define kosal_container_of(_ptr, _type, _memb) container_of(_ptr, _type, _memb)
# include <linux/errno.h>
# include <linux/compiler.h>  // likely(), unlikely(), etc.
# include <linux/sched.h>
# define kosal_get_pid() (btPID)( current->tgid )
# define kosal_get_tid() (btTID)( current->pid  )

typedef struct bus_type kosal_bus_type, *pkosal_bus_type;

#elif defined( __AAL_WINDOWS__ )
# include <stddef.h>
# define kosal_offsetof(_type, _memb) offsetof(_type, _memb)
# define kosal_container_of(_ptr, _type, _memb) ( (_type *)( (btByteArray)(_ptr) - kosal_offsetof(_type, _memb) ) )
# include <crt/errno.h>
# ifndef likely
#    define likely(x) x
# endif // likely
# ifndef unlikely
#    define unlikely(x) x
# endif // unlikely
# include <ntddk.h>
//# define kosal_get_pid() (btPID)PsGetCurrentProcessId()
// HACK TO GET AROUND WINDOWS CLEANUP ISSUE
# define kosal_get_pid() ((btPID)0)  
# define kosal_get_tid() (btTID)PsGetCurrentThreadId()

typedef btAny           kosal_bus_type, *pkosal_bus_type;

#endif // OS

#define kosal_get_object_containing(_ptr, _type, _memb) kosal_container_of(_ptr, _type, _memb)

btInt _kosal_mdelay(__ASSERT_HERE_PROTO btTime );
#ifdef kosal_mdelay
# undef kosal_mdelay
#endif // kosal_mdelay
#define kosal_mdelay(__delay) _kosal_mdelay(__ASSERT_HERE_ARGS __delay)

btInt _kosal_udelay(__ASSERT_HERE_PROTO btTime );
#ifdef kosal_udelay
# undef kosal_udelay
#endif // kosal_udelay
#define kosal_udelay(__delay) _kosal_udelay(__ASSERT_HERE_ARGS __delay)

//
// PCI Device
//
#if   defined( __AAL_LINUX__ )
# include <linux/pci.h>
  typedef struct pci_dev         kosal_pci_dev, *pkosal_pci_dev;
  typedef struct device          kosal_os_dev,  *pkosal_os_dev;
#elif defined( __AAL_WINDOWS__ )
# include <wdm.h>
  typedef BUS_INTERFACE_STANDARD kosal_pci_dev, *pkosal_pci_dev;
  typedef btAny                  kosal_os_dev,  *pkosal_os_dev;
#endif // OS

btInt _kosal_pci_read_config_dword(__ASSERT_HERE_PROTO pkosal_pci_dev , btUnsigned32bitInt , btUnsigned32bitInt * );
#ifdef kosal_pci_read_config_dword
# undef kosal_pci_read_config_dword
#endif // kosal_pci_read_config_dword
#define kosal_pci_read_config_dword(__pdev, __offset, __pval) _kosal_pci_read_config_dword(__ASSERT_HERE_ARGS __pdev, __offset, __pval)

//
// Semaphore
//
#if   defined( __AAL_LINUX__ )
# if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27)
#    include <asm/semaphore.h>
# else
#    include <linux/semaphore.h>
# endif // < KERNEL_VERSION(2,6,27)

  typedef struct semaphore kosal_semaphore, *pkosal_semaphore;
// Count (c) cannot be negative.
# define kosal_sem_init(sptr, c)  do{ ASSERT(c >= 0); sema_init(sptr, c); }while(0)

// Non-alertable, infinite wait. When thread is servicing a user process and running in the user process context.
# define kosal_sem_get_user(sptr) down(sptr)

// Alertable wait. When thread is servicing a user process and running in the user process context.
// returns true if interrupted.
# define kosal_sem_get_user_alertable(sptr) (btBool)down_interruptible(sptr)

// Non-alertable, infinite wait. When thread is not servicing a user process.
# define kosal_sem_get_krnl(sptr) down(sptr)

// Alertable wait. When thread is not servicing a user process.
// returns true if interrupted.
# define kosal_sem_get_krnl_alertable(sptr) (btBool)down_interruptible(sptr)

// Release one count of the semaphore.
# define kosal_sem_put(sptr)      up(sptr)

#elif defined( __AAL_WINDOWS__ )
  #include <ntdef.h>
  typedef KSEMAPHORE       kosal_semaphore, *pkosal_semaphore;

// Count (c) cannot be negative.
// Last parameter is number of threads to resume when sem is put.
# define kosal_sem_init(sptr, c)  do{ ASSERT(c >= 0); KeInitializeSemaphore(sptr, c, 1); }while(0)

// Non-alertable, infinite wait. When thread is servicing a user process and running in the user process context.
static inline
void kosal_sem_get_user(pkosal_semaphore sptr) {                                                                          
   NTSTATUS __s;                                                           
   do                                                                         
   {                                                                          
     __s = KeWaitForSingleObject(sptr, UserRequest, KernelMode, FALSE, NULL); 
   } while( STATUS_SUCCESS != __s && STATUS_ABANDONED_WAIT_0 != __s ); 
}

// Alertable wait. When thread is servicing a user process and running in the user process context.
// returns true if alerted/abandoned.
static inline
btBool kosal_sem_get_user_alertable(pkosal_semaphore sptr) {
   NTSTATUS __s;
   do
   {
      __s = KeWaitForSingleObject(sptr, UserRequest, KernelMode, TRUE, NULL);
   } while( STATUS_SUCCESS != __s && STATUS_ALERTED != __s && STATUS_ABANDONED_WAIT_0 != __s );
   return STATUS_SUCCESS != __s;
}

// Non-alertable, infinite wait. When thread is not servicing a user process.
static inline 
void kosal_sem_get_krnl(pkosal_semaphore sptr) {                                                                          
   NTSTATUS __s;                                                           
   do                                                                         
   {                                                                          
     __s = KeWaitForSingleObject(sptr, Executive,   KernelMode, FALSE, NULL); 
   } while( STATUS_SUCCESS != __s && STATUS_ABANDONED_WAIT_0 != __s ); 
}

// Alertable wait. When thread is not servicing a user process.
// returns true if alerted/abandoned.
static inline
btBool kosal_sem_get_krnl_alertable(pkosal_semaphore sptr) {
   NTSTATUS __s;
   do
   {
      __s = KeWaitForSingleObject(sptr, Executive, KernelMode, TRUE, NULL);
   } while( STATUS_SUCCESS != __s && STATUS_ALERTED != __s && STATUS_ABANDONED_WAIT_0 != __s );
   return STATUS_SUCCESS != __s;
}

// Release one count of the semaphore. Count must not be negative.
# define kosal_sem_put(sptr)      KeReleaseSemaphore(sptr, 0, 1, FALSE)

#endif // OS

//
// Mutex
//
typedef kosal_semaphore kosal_mutex, *pkosal_mutex;
#define kosal_mutex_init(mptr) kosal_sem_init(mptr, 1)

//
// List
//
#if   defined( __AAL_LINUX__ )
# include <linux/list.h>
  typedef struct list_head kosal_list_head, *pkosal_list_head;
# define kosal_list_prev(_h)                 (_h)->prev
# define kosal_list_next(_h)                 (_h)->next

# define kosal_list_init(_h)                 INIT_LIST_HEAD(_h)
# define kosal_list_is_empty(_h)             list_empty(_h)

# define kosal_list_add_head(_new, _h)       list_add(_new, _h)
# define kosal_list_add_tail(_new, _h)       list_add_tail(_new, _h)

# define kosal_list_del_init(_h)             list_del_init(_h)
# define kosal_list_del(_h)                  list_del(_h)

# define kosal_list_replace(_old, _new)      list_replace(_old, _new)

# if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,16)
#    define kosal_list_replace_init(_old, _new) list_replace_init(_old, _new)
# else
#    define kosal_list_replace_init(_old, _new) do{ list_replace_rcu(_old, _new); INIT_LIST_HEAD(_old); }while(0)
# endif // > KERNEL_VERSION(2,6,16)

# define kosal_list_move(_new, _h)           list_move(_new, _h)

# define kosal_list_for_each(_pos, _h)                                list_for_each(_pos, _h)
# define kosal_list_for_each_safe(_pos, _n, _h)                       list_for_each_safe(_pos, _n, _h)
# define kosal_list_for_each_entry(_pos, _h, _memb, _cont_t)          list_for_each_entry(_pos, _h, _memb)
# define kosal_list_for_each_entry_safe(_pos, _n, _h, _memb, _cont_t) list_for_each_entry_safe(_pos, _n, _h, _memb)


#elif defined( __AAL_WINDOWS__ )
# include <ntdef.h>
  typedef LIST_ENTRY       kosal_list_head, *pkosal_list_head;
# define kosal_list_prev(_h)                 (_h)->Blink
# define kosal_list_next(_h)                 (_h)->Flink

# define kosal_list_init(_h)                 InitializeListHead(_h)
# define kosal_list_is_empty(_h)             IsListEmpty(_h)

# define kosal_list_add_head(_new, _h)       InsertHeadList(_h, _new)
# define kosal_list_add_tail(_new, _h)       InsertTailList(_h, _new)

# define kosal_list_del_init(_h)             do{ RemoveEntryList(_h); InitializeListHead(_h); }while(0)
# define kosal_list_del(_h)                  RemoveEntryList(_h)

# define kosal_list_replace(_old, _new)           \
do                                                \
{                                                 \
   kosal_list_prev(_new) = kosal_list_prev(_old); \
   kosal_list_next(kosal_list_prev(_new)) = _new; \
   kosal_list_next(_new) = kosal_list_next(_old); \
   kosal_list_prev(kosal_list_next(_new)) = _new; \
}while(0)

# define kosal_list_replace_init(_old, _new) do{ kosal_list_replace(_old, _new); kosal_list_init(_old); }while(0)

# define kosal_list_move(_new, _h)           do{ RemoveEntryList(_new); InsertHeadList(_h, _new); }while(0)

# define kosal_list_for_each(_pos, _h) \
   for ( (_pos) = kosal_list_next(_h) ; (_pos) != (_h) ; (_pos) = kosal_list_next(_pos) )

# define kosal_list_for_each_safe(_pos, _n, _h)                       \
   for ( (_pos) = kosal_list_next(_h), (_n) = kosal_list_next(_pos) ; \
            (_pos) != (_h) ;                                          \
               (_pos) = (_n), (_n) = kosal_list_next(_pos) )

# define kosal_list_for_each_entry(_pos, _h, _memb, _cont_t)                   \
      for ( (_pos) = kosal_container_of(kosal_list_next(_h), _cont_t, _memb) ; \
               &(_pos)->_memb != (_h) ;                                        \
                  (_pos) = kosal_container_of(kosal_list_next(&(_pos)->_memb), _cont_t, _memb) )

# define kosal_list_for_each_entry_safe(_pos, _n, _h, _memb, _cont_t)                   \
   for ( (_pos) = kosal_container_of(kosal_list_next(_h),   _cont_t, _memb),            \
         (_n)   = kosal_container_of(kosal_list_next(&(_pos)->_memb), _cont_t, _memb) ; \
            &(_pos)->_memb != (_h) ;                                                    \
               (_pos) = (_n),                                                           \
               (_n)   = kosal_container_of(kosal_list_next(&(_pos)->_memb), _cont_t, _memb) )

#endif // OS

#define kosal_list_add(_new, _h)                  kosal_list_add_head(_new, _h)

#define kosal_list_entry(_ptr, _type, _memb)      kosal_container_of(_ptr, _type, _memb)
#define kosal_list_get_object(_ptr, _type, _memb) kosal_container_of(_ptr, _type, _memb)


//
// Memory
//
#if   defined( __AAL_LINUX__ )
# include <linux/slab.h>
#elif defined( __AAL_WINDOWS__ )
# include <ntddk.h>
#endif // OS

btVirtAddr _kosal_kmalloc(__ASSERT_HERE_PROTO btWSSize );
#ifdef kosal_kmalloc
# undef kosal_kmalloc
#endif // kosal_kmalloc
#define kosal_kmalloc(__size) _kosal_kmalloc(__ASSERT_HERE_ARGS __size)

void _kosal_kfree(__ASSERT_HERE_PROTO btAny , btWSSize );
#ifdef kosal_kfree
# undef kosal_kfree
#endif // kosal_kfree
#define kosal_kfree(__ptr, __size) _kosal_kfree(__ASSERT_HERE_ARGS __ptr, __size)

btPhysAddr kosal_virt_to_phys(btAny );

btVirtAddr _kosal_alloc_contiguous_mem_nocache(__ASSERT_HERE_PROTO btWSSize );
#ifdef kosal_alloc_contiguous_mem_nocache
# undef kosal_alloc_contiguous_mem_nocache
#endif // kosal_alloc_contiguous_mem_nocache
#define kosal_alloc_contiguous_mem_nocache(__size) _kosal_alloc_contiguous_mem_nocache(__ASSERT_HERE_ARGS __size)

void _kosal_free_contiguous_mem(__ASSERT_HERE_PROTO btAny , btWSSize );
# ifdef kosal_free_contiguous_mem
#    undef kosal_free_contiguous_mem
# endif // kosal_free_contiguous_mem
# define kosal_free_contiguous_mem(__ptr, __size) _kosal_free_contiguous_mem(__ASSERT_HERE_ARGS __ptr, __size)

static inline
btWSSize kosal_round_up_to_page_size(btWSSize s) {
   btWSSize m = ((btWSSize)1 << PAGE_SHIFT) - 1;
   btWSSize r = s & m;
   return (s & ~m) + (((btWSSize)0 == r) ? 0 : PAGE_SIZE);
}


//
// Work queue
//
#if   defined( __AAL_LINUX__ )
# include <linux/workqueue.h>
   //struct workqueue_struct   * kosal_work_queue
   #define kosal_create_workqueue(pd)  create_workqueue((aaldev_to_basedev(pd))
    // Worker thread items

   #if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,19)
      typedef struct workqueue_struct *kosal_work_queue;
   #else
   # error Linux version < 2.6.19 not supported
   #endif

   typedef void (*osfunc)(void * , void * );


//   typedef struct work_object *pwork_object;
   struct work_object;

   typedef void (*kosal_work_handler)(struct work_object * );

   typedef struct {
    struct delayed_work  workobj;
      void *               context;
      osfunc               callback;
      kosal_work_handler   fnct;
   } work_object, *pwork_object;



   void task_poller(struct work_struct *work);


#define KOSAL_INIT_WORK(w,f)      do {(w->fnct) = f; \
                                      (w->context)=(&w); \
                                      (w->callback)=WorkItemCallback; \
                                      INIT_DELAYED_WORK(w->workobj,f); \
                                     }while(0);

#define kosal_init_waitqueue_head(q) init_waitqueue_head(q)

#elif defined( __AAL_WINDOWS__ )
   typedef PIO_WORKITEM    kosal_work_queue;

#define kosal_init_waitqueue_head(q)

#define kosal_create_workqueue(pd)  IoAllocateWorkItem(aaldev_to_basedev(pd))
//kosal_create_workqueue

typedef void (*osfunc)(PDEVICE_OBJECT,PVOID);

typedef void(*kosal_work_handler)(pwork_object);

typedef struct {
   void *               context;
   osfunc               callback;
   btTime               msec_delay;
   kosal_work_handler   fnct;
}work_object;

#define KOSAL_INIT_WORK(w,f)      do {((w)->fnct) = f; \
                                      ((w)->context)=(w); \
                                      ((w)->callback)=WorkItemCallback; \
                                     }while(0)




typedef work_object *pwork_object;

void WorkItemCallback(IN PDEVICE_OBJECT pdevObject, IN PVOID Context);
void kosal_queue_delayed_work(kosal_work_queue wq, pwork_object pwo,btTime msec);
#if 0
static void
   kosal_queue_delayed_work(kosal_work_queue wq, pwork_object pwo,btTime msec)
{
   pwo->msec_delay = msec;
   IoQueueWorkItem(wq, WorkItemCallback, DelayedWorkQueue, pwo);
}
#endif
#endif // OS Work queue

//
// Polling/Events
//
#if   defined( __AAL_LINUX__ )
// struct __wait_queue_head {
// ...
// };
// typedef struct __wait_queue_head wait_queue_head_t;
# include <linux/wait.h>
typedef wait_queue_head_t kosal_poll_object;

# define kosal_poll_object_is_valid(__kpo_ptr) ( !list_empty(&(__kpo_ptr)->task_list) )

# define kosal_poll_object_consume(__kpo_ptr) \
do                                            \
{  wait_queue_t __junk;                       \
   remove_wait_queue(__kpo_ptr, &__junk);     \
}while(0)

# define kosal_wake_up_interruptible(__kpo_ptr) wake_up_interruptible(__kpo_ptr)

#elif defined( __AAL_WINDOWS__ )
//
// typedef WDFREQUEST__ *WDFREQUEST;
//
# include <wdf.h>
# include <wdftypes.h>
typedef WDFREQUEST        kosal_poll_object; 

# define kosal_poll_object_is_valid(__kpo_ptr) ( (NULL != (__kpo_ptr)) && (NULL != *(__kpo_ptr)) )

# define kosal_poll_object_consume(__kpo_ptr)  ( *(__kpo_ptr) = NULL )

void kosal_wake_up_interruptible(kosal_poll_object * );

# define kosal_poll_wait(q)             //IoMarkIrpPending(WdfRequestWdmGetIrp(q))
#endif // Polling

#endif // __AALSDK_KERNEL_KOSAL_H__
#endif // __AAL_KERNEL__
