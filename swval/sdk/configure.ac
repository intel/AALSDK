# INTEL CONFIDENTIAL - For Intel Internal Use Only
dnl Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])

AC_INIT([swval-sdk], [1.1.1], [tim.whisonant@intel.com])
AC_COPYRIGHT([INTEL CONFIDENTIAL - For Intel Internal Use Only])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_SRCDIR([aas/AASLib/AALlib.cpp])
AC_CONFIG_TESTDIR([tests])
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

AALSDK_COPYRIGHT_NOTICE([INTEL CONFIDENTIAL - For Intel Internal Use Only])
dnl ############################################################################
dnl ## Library Versioning ######################################################
dnl ############################################################################
AALSDK_LTLIB_VERSION([OSAL],         [libOSAL],         [1],[1],[1])
AALSDK_LTLIB_VERSION([AAS],          [libAAS],          [1],[1],[1])
AALSDK_LTLIB_VERSION([AALRUNTIME],   [libaalrt],        [1],[1],[1])
AALSDK_LTLIB_VERSION([RRMBROKER],    [librrmbroker],    [1],[1],[1])
AALSDK_LTLIB_VERSION([FAPTRANS1],    [libFAPTrans1],    [1],[1],[1])
AALSDK_LTLIB_VERSION([FAPTRANS2],    [libFAPTrans2],    [1],[1],[1])
AALSDK_LTLIB_VERSION([MAFUTRANS],    [libMAFUTrans],    [1],[1],[1])
AALSDK_LTLIB_VERSION([AASREGISTRAR], [libAASRegistrar], [1],[1],[1])
AALSDK_LTLIB_VERSION([AASUAIA],      [libAASUAIA],      [1],[1],[1])
AALSDK_LTLIB_VERSION([AALCLP],       [libaalclp],       [1],[1],[1])
AALSDK_LTLIB_VERSION([ASE],          [libASE],          [1],[1],[1])
AALSDK_LTLIB_VERSION([CCIAFU],       [libCCIAFU],       [1],[1],[1])
AALSDK_LTLIB_VERSION([HWCCIAFU],     [libHWCCIAFU],     [1],[1],[1])
AALSDK_LTLIB_VERSION([ASECCIAFU],    [libASECCIAFU],    [1],[1],[1])
AALSDK_LTLIB_VERSION([SWSIMCCIAFU],  [libSWSimCCIAFU],  [1],[1],[1])
AALSDK_LTLIB_VERSION([SPLAFU],       [libSPLAFU],       [1],[1],[1])
AALSDK_LTLIB_VERSION([HWSPLAFU],     [libHWSPLAFU],     [1],[1],[1])
AALSDK_LTLIB_VERSION([ASESPLAFU],    [libASESPLAFU],    [1],[1],[1])
AALSDK_LTLIB_VERSION([SWSIMSPLAFU],  [libSWSimSPLAFU],  [1],[1],[1])
AALSDK_LTLIB_VERSION([CCILIB],       [libCCI],          [1],[1],[1])
AALSDK_LTLIB_VERSION([ALIAFU],       [libALIAFU],       [1],[1],[1])
AALSDK_LTLIB_VERSION([HWALIAFU],     [libHWALIAFU],     [1],[1],[1])
AALSDK_LTLIB_VERSION([ASEALIAFU],    [libASEALIAFU],    [1],[1],[1])
AALSDK_LTLIB_VERSION([SWSIMALIAFU],  [libSWSimALIAFU],  [1],[1],[1])

AALSDK_LTLIB_VERSION([SWVALMOD],     [libswvalmod],     [3],[2],[1])
AALSDK_LTLIB_VERSION([SWVALSVCMOD],  [libswvalsvcmod],  [6],[5],[4])

dnl ############################################################################
dnl ## Optional Packages #######################################################
dnl ############################################################################
WITH_GCOV_ANALYSIS
WITH_VALGRIND_ANALYSIS

dnl ############################################################################
dnl ## Optional Features #######################################################
dnl ############################################################################
AALSDK_DEBUG_OUTPUT([yes])
AALSDK_DEBUG_DYNLOAD([no])
AALSDK_ASSERT([yes])
AALSDK_MAINTAINER
AALSDK_LOCAL_GTEST([tests/harnessed/gtest], [1.7.0])

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
AM_MISSING_PROG([AUTOM4TE], [autom4te])

AALSDK_PROG_BASH
AALSDK_PROG_FLEX
AALSDK_PROG_GIT
AALSDK_PROG_GDBTUI

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
VERIFY_OPTIONS

AC_CONFIG_FILES([tests/atlocal])
AC_CONFIG_FILES([tests/bat:tests/run/bat.in],         [chmod 755 tests/bat])
AC_CONFIG_FILES([tests/nightly:tests/run/nightly.in], [chmod 755 tests/nightly])
AC_CONFIG_FILES([tests/weekly:tests/run/weekly.in],   [chmod 755 tests/weekly])
AC_CONFIG_FILES([tests/OSAL_TestSem:tests/run/OSAL_TestSem.in], [chmod 755 tests/OSAL_TestSem])
AC_CONFIG_FILES([tests/OSAL_TestThreadGroup:tests/run/OSAL_TestThreadGroup.in], [chmod 755 tests/OSAL_TestThreadGroup])

AC_CONFIG_FILES([gdb/gdbinit])

AC_CONFIG_FILES([include/aalsdk/osal/OSALService.h
                 include/aalsdk/aas/AASService.h
                 include/aalsdk/aas/AALRuntimeModule.h
                 include/aalsdk/faptrans/FAP10Service.h
                 include/aalsdk/faptrans/FAP20Service.h
                 include/aalsdk/faptrans/MAFUTransService.h
                 include/aalsdk/eds/AASEDSService.h
                 include/aalsdk/registrar/AASRegistrarService.h
                 include/aalsdk/uaia/uAIAService.h
                 include/aalsdk/aalclp/aalclp.h
                 include/aalsdk/service/CCIAFUService.h
                 include/aalsdk/service/HWCCIAFUService.h
                 include/aalsdk/service/ASECCIAFUService.h
                 include/aalsdk/service/SWSimCCIAFUService.h
                 include/aalsdk/service/SPLAFUService.h
                 include/aalsdk/service/HWSPLAFUService.h
                 include/aalsdk/service/ASESPLAFUService.h
                 include/aalsdk/service/SWSimSPLAFUService.h
                 include/aalsdk/service/ALIAFUService.h
                 include/aalsdk/service/HWALIAFUService.h
                 include/aalsdk/service/ASEALIAFUService.h
                 include/aalsdk/service/SWSimALIAFUService.h])

AC_CONFIG_LINKS([include/aalsdk/ase/ase_common.h:ase/sw/ase_common.h])

AC_CONFIG_FILES([Makefile
                 aas/OSAL/Makefile
                 aas/AASLib/Makefile
                 aas/AALRuntime/Makefile
                 aas/RRMBrokerService/Makefile
                 aas/AASRegistrar/Makefile
                 aas/uAIA/Makefile
                 aas/AASResourceManager/Makefile])

AC_CONFIG_FILES([ase/sw/Makefile
                 clp/Makefile
                 utils/Makefile
                 utils/aalscan/Makefile
                 utils/data_model/Makefile
                 utils/fpgasane/Makefile
                 utils/CCIAFU/Makefile
                 utils/CCIAFU/HWCCIAFU/Makefile
                 utils/CCIAFU/ASECCIAFU/Makefile
                 utils/CCIAFU/SWSimCCIAFU/Makefile
                 utils/SPLAFU/Makefile
                 utils/SPLAFU/HWSPLAFU/Makefile
                 utils/SPLAFU/ASESPLAFU/Makefile
                 utils/SPLAFU/SWSimSPLAFU/Makefile
                 utils/cciapp/Makefile
                 utils/splapp/Makefile
                 utils/splapp2/Makefile
                 utils/splapp3/Makefile
                 utils/CCILib/Makefile
                 utils/ALIAFU/Makefile
                 utils/ALIAFU/HWALIAFU/Makefile
                 utils/ALIAFU/ASEALIAFU/Makefile
                 utils/ALIAFU/SWSimALIAFU/Makefile])

AC_CONFIG_FILES([tests/Makefile
                 tests/harnessed/Makefile
                 tests/harnessed/gtest/Makefile
                 tests/standalone/Makefile
                 tests/standalone/OSAL_TestSem/Makefile
                 tests/standalone/OSAL_TestThreadGroup/Makefile
                 tests/standalone/isolated/Makefile
                 tests/swvalmod/Makefile])

AC_OUTPUT

dnl ############################################################################
dnl ## Config / Next Steps #####################################################
dnl ############################################################################
echo \
"
${ac_cv_aal_copyright}

AALSDK Validation Project"
SHOW_OPTIONS
