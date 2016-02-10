## Copyright(c) 2013-2016, Intel Corporation
##
## Redistribution  and  use  in source  and  binary  forms,  with  or  without
## modification, are permitted provided that the following conditions are met:
##
## * Redistributions of  source code  must retain the  above copyright notice,
##   this list of conditions and the following disclaimer.
## * Redistributions in binary form must reproduce the above copyright notice,
##   this list of conditions and the following disclaimer in the documentation
##   and/or other materials provided with the distribution.
## * Neither the name  of Intel Corporation  nor the names of its contributors
##   may be used to  endorse or promote  products derived  from this  software
##   without specific prior written permission.
##
## THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
## AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  BUT NOT LIMITED TO,  THE
## IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
## ARE DISCLAIMED.  IN NO EVENT  SHALL THE COPYRIGHT OWNER  OR CONTRIBUTORS BE
## LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR
## CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT LIMITED  TO,  PROCUREMENT  OF
## SUBSTITUTE GOODS OR SERVICES;  LOSS OF USE,  DATA, OR PROFITS;  OR BUSINESS
## INTERRUPTION)  HOWEVER CAUSED  AND ON ANY THEORY  OF LIABILITY,  WHETHER IN
## CONTRACT,  STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE  OR OTHERWISE)
## ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,  EVEN IF ADVISED OF THE
## POSSIBILITY OF SUCH DAMAGE.
##****************************************************************************
##     Intel(R) Accelerator Abstraction Layer Library Software Developer
##        Kit (SDK)
##  Content:
##     Intel(R) QuickAssist Technology
##     AALSDK/configure.ac
##  Author:
##     Tim Whisonant, Intel Corporation
##  History:
##     06/26/2013    TSW   Initial version
##     04/01/2014    JG    Added aalrtlib for AAL 4.x
##******************************************************************************
dnl Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])

AC_INIT([aalsdk], [5.0.1], [joe.grecco@intel.com])
AC_COPYRIGHT([Copyright(c) 2003-2016, Intel Corporation])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_SRCDIR([aas/AASLib/AALlib.cpp])
AC_CONFIG_HEADERS([config.h])
AC_LANG([C++])

dnl PACKAGE_URL not available until autoconf 2.64, but we support 2.63.
m4_ifdef([AC_PACKAGE_URL], [],
         [AC_DEFINE_UNQUOTED(PACKAGE_URL, "", [Define to the home page for this package.])])

AM_INIT_AUTOMAKE([-Wall -Werror silent-rules])
AM_SILENT_RULES([yes])
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

LT_PREREQ([2.2])
LT_INIT([dlopen])
LT_LANG([C++])

AALSDK_COPYRIGHT_NOTICE([Copyright(c) 2003-2016, Intel Corporation])
AC_CONFIG_COMMANDS_PRE([CPPFLAGS+=" -D__AAL_USER__=1"])
dnl ############################################################################
dnl ## Library Versioning ######################################################
dnl ############################################################################
AALSDK_LTLIB_VERSION([OSAL],         [libOSAL],         [0],[0],[0])
AALSDK_LTLIB_VERSION([AAS],          [libAAS],          [0],[0],[0])
AALSDK_LTLIB_VERSION([AALRUNTIME],   [libaalrt],        [0],[0],[0])
AALSDK_LTLIB_VERSION([RESMGR],       [libAASResMgr],    [0],[0],[0])
AALSDK_LTLIB_VERSION([RRM],          [librrm],          [0],[0],[0])
AALSDK_LTLIB_VERSION([RRMBROKER],    [librrmbroker],    [0],[0],[0])
AALSDK_LTLIB_VERSION([AIASERVICE],   [libaia],          [0],[0],[0])
AALSDK_LTLIB_VERSION([AALCLP],       [libaalclp],       [0],[0],[0])
AALSDK_LTLIB_VERSION([ASE],          [libASE],          [0],[0],[0])
AALSDK_LTLIB_VERSION([ALIAFU],       [libALIAFU],       [0],[0],[0])
AALSDK_LTLIB_VERSION([VTP],          [libVTP],          [0],[0],[0])
AALSDK_LTLIB_VERSION([HWALIAFU],     [libHWALIAFU],     [0],[0],[0])
AALSDK_LTLIB_VERSION([ASEALIAFU],    [libASEALIAFU],    [0],[0],[0])
AALSDK_LTLIB_VERSION([SWSIMALIAFU],  [libSWSimALIAFU],  [0],[0],[0])

dnl ############################################################################
dnl ## Optional Packages #######################################################
dnl ############################################################################

dnl ############################################################################
dnl ## Optional Features #######################################################
dnl ############################################################################
AALSDK_DEBUG_OUTPUT([yes])
AALSDK_DEBUG_DYNLOAD([no])
AALSDK_ASSERT([yes])
AALSDK_MAINTAINER

dnl ############################################################################
dnl ## Checks for programs #####################################################
dnl ############################################################################
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_MKDIR_P
AM_PROG_CC_C_O

AALSDK_PROG_BASH
AALSDK_PROG_FLEX
AALSDK_PROG_GIT
AALSDK_PROG_DOXYGEN([1.7.6], [html])

dnl ############################################################################
dnl ## Checks for libraries ####################################################
dnl ############################################################################
AC_CHECK_LIB([pthread], [pthread_create], [],
             [AC_MSG_ERROR([Required library pthread not found. Install pthreads and try again.])])
AC_CHECK_LIB([ltdl], [lt_dlopen], [],
             [AC_MSG_ERROR([Required library ltdl not found.])])
AC_CHECK_LIB([rt], [shm_open], [],
             [AC_MSG_ERROR([Required library rt not found.])])

dnl ############################################################################
dnl ## Checks for header files #################################################
dnl ############################################################################
AC_STDC_HEADERS
AC_HEADER_STDBOOL
AC_CHECK_HEADERS([pthread.h ltdl.h sys/mman.h])
AC_CHECK_HEADERS([ctype.h errno.h fcntl.h inttypes.h stddef.h stdint.h stdlib.h string.h time.h unistd.h])
AC_CHECK_HEADERS([sys/ioctl.h sys/resource.h sys/time.h sys/types.h])

dnl ############################################################################
dnl ## Checks for typedefs, structures, and compiler characteristics  ##########
dnl ############################################################################
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_INT8_T
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINTPTR_T
AC_TYPE_OFF_T

AC_CHECK_TYPES([struct timeval],[],[],[
AC_INCLUDES_DEFAULT
#include <sys/time.h>])
AC_CHECK_TYPES([struct timespec],[],[],[
AC_INCLUDES_DEFAULT
#include <time.h>])

dnl ############################################################################
dnl ## Checks for library functions ############################################
dnl ############################################################################
AC_FUNC_MMAP
AC_CHECK_FUNCS([gettimeofday memset munmap strchr strerror strncpy])

dnl ############################################################################
dnl ## Makefile substitutions ##################################################
dnl ############################################################################

AC_CONFIG_FILES([doc/Doxyfile])

AC_CONFIG_FILES([include/aalsdk/osal/OSALService.h
                 include/aalsdk/aas/AASService.h
                 include/aalsdk/aas/AALRuntimeModule.h
                 include/aalsdk/rm/ResMgrService.h
                 include/aalsdk/rm/RRMService.h
                 include/aalsdk/uaia/AIAService.h
                 include/aalsdk/aalclp/aalclp.h
                 include/aalsdk/service/ALIAFUService.h
                 include/aalsdk/service/VTPService.h
                 include/aalsdk/service/HWALIAFUService.h
                 include/aalsdk/service/ASEALIAFUService.h
                 include/aalsdk/service/SWSimALIAFUService.h])

AC_CONFIG_LINKS([include/aalsdk/ase/ase_common.h:ase/sw/ase_common.h])

AC_CONFIG_FILES([Makefile
                 aas/OSAL/Makefile
                 aas/AASLib/Makefile
                 aas/AALRuntime/Makefile
                 aas/AIAService/Makefile
                 aas/RRMBrokerService/Makefile
                 aas/AASResourceManager/Makefile])

AC_CONFIG_FILES([ase/sw/Makefile
                 clp/Makefile
                 utils/Makefile
                 utils/aalscan/Makefile
                 utils/fpgadiag/Makefile
                 utils/data_model/Makefile
                 utils/ALIAFU/Makefile
                 utils/ALIAFU/VTP/Makefile
                 utils/ALIAFU/HWALIAFU/Makefile
                 utils/ALIAFU/ASEALIAFU/Makefile
                 utils/ALIAFU/SWSimALIAFU/Makefile])

# temporarily removed fpgasane
#                 utils/fpgasane/Makefile

                 
AC_OUTPUT

dnl ############################################################################
dnl ## Config / Next Steps #####################################################
dnl ############################################################################
echo \
"
${ac_cv_aal_copyright}
Intel(R) QuickAssist Technology Accelerator Abstraction Layer

${PACKAGE_NAME} ${PACKAGE_VERSION}

Prefix       : ${prefix}
C compiler   : ${CC} ${CFLAGS}
C++ compiler : ${CXX} ${CXXFLAGS}
Linker       : ${LD} ${LDFLAGS} ${LIBS}

To build     : make
To install   : make install <install options>
   : Please refer to the Software Installation Guide for installation instructions.
"
