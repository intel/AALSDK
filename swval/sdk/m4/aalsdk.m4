dnl Copyright (c) 2013-2015, Intel Corporation
dnl
dnl Redistribution  and  use  in source  and  binary  forms,  with  or  without
dnl modification, are permitted provided that the following conditions are met:
dnl
dnl * Redistributions of  source code  must retain the  above copyright notice,
dnl   this list of conditions and the following disclaimer.
dnl * Redistributions in binary form must reproduce the above copyright notice,
dnl   this list of conditions and the following disclaimer in the documentation
dnl   and/or other materials provided with the distribution.
dnl * Neither the name  of Intel Corporation  nor the names of its contributors
dnl   may be used to  endorse or promote  products derived  from this  software
dnl   without specific prior written permission.
dnl
dnl THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
dnl AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  BUT NOT LIMITED TO,  THE
dnl IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
dnl ARE DISCLAIMED.  IN NO EVENT  SHALL THE COPYRIGHT OWNER  OR CONTRIBUTORS BE
dnl LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR
dnl CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT LIMITED  TO,  PROCUREMENT  OF
dnl SUBSTITUTE GOODS OR SERVICES;  LOSS OF USE,  DATA, OR PROFITS;  OR BUSINESS
dnl INTERRUPTION)  HOWEVER CAUSED  AND ON ANY THEORY  OF LIABILITY,  WHETHER IN
dnl CONTRACT,  STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE  OR OTHERWISE)
dnl ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,  EVEN IF ADVISED OF THE
dnl POSSIBILITY OF SUCH DAMAGE.
dnl ****************************************************************************
dnl      Intel(R) Accelerator Abstraction Layer Library Software Developer
dnl         Kit (SDK)
dnl   Content:
dnl      Intel(R) Accelerator Abstraction Layer Library
dnl      m4/aalsdk.m4
dnl   Author:
dnl      Tim Whisonant, Intel Corporation
dnl   History:
dnl      07/18/2013    TSW   Initial version
dnl ******************************************************************************

dnl # AALSDK_YES_NO(VALUE, VARIABLE-NAME, [ACTION-IF-YES], [ACTION-IF-NO], [ACTION-IF-NEITHER])
dnl #
AC_DEFUN([AALSDK_YES_NO], [
   _aal_yes_no_val=m4_tolower([$1])
   AS_IF([test x"${_aal_yes_no_val}" = x0],       [$2=no],
         [test x"${_aal_yes_no_val}" = xn],       [$2=no],
         [test x"${_aal_yes_no_val}" = xno],      [$2=no],
         [test x"${_aal_yes_no_val}" = xoff],     [$2=no],
         [test x"${_aal_yes_no_val}" = xfalse],   [$2=no],
         [test x"${_aal_yes_no_val}" = xnone],    [$2=no],
         [test x"${_aal_yes_no_val}" = xdisable], [$2=no],
         [test x"${_aal_yes_no_val}" = xwithout], [$2=no],
         [test x"${_aal_yes_no_val}" = x1],       [$2=yes],
         [test x"${_aal_yes_no_val}" = xy],       [$2=yes],
         [test x"${_aal_yes_no_val}" = xon],      [$2=yes],
         [test x"${_aal_yes_no_val}" = xyes],     [$2=yes],
         [test x"${_aal_yes_no_val}" = xtrue],    [$2=yes],
         [test x"${_aal_yes_no_val}" = xenable],  [$2=yes],
         [test x"${_aal_yes_no_val}" = xwith],    [$2=yes],
         [
          $2="$1"
          m4_if([$5], , :, [$5])
         ])
   AS_IF([test x"[$]$2" = xyes], [m4_if([$3], , :, [$3])],
                                 [m4_if([$4], , :, [$4])])
]) dnl # AALSDK_YES_NO


dnl Copyright

dnl # AALSDK_COPYRIGHT_NOTICE(COPYRIGHT-STMNT)
dnl # ---
dnl # Specify the copyright statement.
dnl # Creates cache value:
dnl #    ac_cv_aal_copyright_stmnt
dnl # Creates the #define
dnl #    AALSDK_COPYRIGHT : (const char *) copyright statement
dnl # Performs the substitution:
dnl #    @AALSDK_COPYRIGHT@
AC_DEFUN([AALSDK_COPYRIGHT_NOTICE], [
   AC_CACHE_VAL([ac_cv_aal_copyright], [ac_cv_aal_copyright="$1"])
   AC_DEFINE_UNQUOTED(AALSDK_COPYRIGHT, "${ac_cv_aal_copyright}",
                      [define to the Copyright statement for the AALSDK])
   AC_SUBST([AALSDK_COPYRIGHT], [${ac_cv_aal_copyright}])
]) dnl # AALSDK_COPYRIGHT_NOTICE


dnl Library Versioning

dnl # AALSDK_LTLIB_VERSION(DEFNAME, SVCMOD, CURRENT, REVISION, AGE)
dnl # ---
dnl # Use in the implementing package's configure.ac (where the source lives) to specify
dnl # the version of a libtool library.
dnl #
dnl # DEFNAME is the #define name of the module, the name prefix given to the following
dnl # defines: eg OSAL results in
dnl #    OSAL_VERSION_CURRENT  : (int)          version[CURRENT]  (libtool scheme)
dnl #    OSAL_VERSION_REVISION : (int)          version[REVISION] (libtool scheme)
dnl #    OSAL_VERSION_AGE      : (int)          version[AGE]      (libtool scheme)
dnl #    OSAL_VERSION          : (const char *) "CURRENT:REVISION:AGE"
dnl #
dnl # Additionally, the following substitutions are made: (cont'ing the OSAL example)
dnl #    @OSAL_SVC_MOD@ -> SVCMOD, the 'module name' as seen by libtool (eg libOSAL)
dnl #    @OSAL_VERSION_CURRENT@
dnl #    @OSAL_VERSION_REVISION@
dnl #    @OSAL_VERSION_AGE@
dnl #    @OSAL_VERSION@
AC_DEFUN([AALSDK_LTLIB_VERSION], [
   AC_DEFINE_UNQUOTED($1_VERSION_CURRENT, $3,
                      [define to current version of $1 (libtool version scheme)])
   AC_DEFINE_UNQUOTED($1_VERSION_REVISION, $4,
                      [define to revision version of $1 (libtool version scheme)])
   AC_DEFINE_UNQUOTED($1_VERSION_AGE, $5,
                      [define to age version of $1 (libtool version scheme)])
   AC_DEFINE_UNQUOTED($1_VERSION, "$3.$4.$5",
                      [define to full version of $1 (libtool version scheme)])
   AC_SUBST([$1_SVC_MOD],          [$2])
   AC_SUBST([$1_SVC_MLEN],         [m4_len($2)])
   AC_SUBST([$1_VERSION_CURRENT],  [$3])
   AC_SUBST([$1_VERSION_REVISION], [$4])
   AC_SUBST([$1_VERSION_AGE],      [$5])
   AC_SUBST([$1_VERSION],          [$3.$4.$5])
]) dnl # AALSDK_LTLIB_VERSION


dnl Optional Features

dnl # AALSDK_DEBUG_OUTPUT(DEF-VAL)
dnl # ---
dnl # (optional feature) - debug output (ENABLE_DEBUG)
dnl # Users specify whether they want to see debug print output / enable debugging features.
AC_DEFUN([AALSDK_DEBUG_OUTPUT], [
   _aal_dbg_output="$1"
   AC_ARG_ENABLE([aal-dbg],
                 [AS_HELP_STRING([--enable-aal-dbg], [Enable debug facilities @<:@default=$1@:>@])],
                 [AALSDK_YES_NO([${enableval}], [_aal_dbg_output], [], [],
                                [AC_MSG_ERROR([Invalid value '${enableval}' for --enable-aal-dbg])])])
   AC_CACHE_CHECK([whether to create a debug build], [ac_cv_aal_enable_dbg_output],
                  [ac_cv_aal_enable_dbg_output="${_aal_dbg_output}"])
   AC_SUBST([AALSDK_ENABLE_DBG], [${ac_cv_aal_enable_dbg_output}])
   AM_CONDITIONAL([AAL_COND_ENABLE_DBG_OUTPUT], [test "x${ac_cv_aal_enable_dbg_output}" = xyes])
   AM_COND_IF([AAL_COND_ENABLE_DBG_OUTPUT],
              [
               DEBUG_CPPFLAGS='-DENABLE_DEBUG=1'
               CPPFLAGS+=' -DENABLE_DEBUG=1'
              ])
   AC_SUBST([DEBUG_CPPFLAGS], [${DEBUG_CPPFLAGS}])
]) dnl # AALSDK_DEBUG_OUTPUT

dnl # AALSDK_DEBUG_DYNLOAD(DEF-VAL)
dnl # ---
dnl # (optional feature) - debug output for dynamic load libraries (DBG_DYN_LOAD)
dnl # Users specify whether they want to see debug print output regarding dll's.
AC_DEFUN([AALSDK_DEBUG_DYNLOAD], [
   _aal_dbg_dynload="$1"
   AC_ARG_ENABLE([aal-dbg-dynload],
                 [AS_HELP_STRING([--enable-aal-dbg-dynload], [AAL Debug output for dll's @<:@default=$1@:>@])],
                 [AALSDK_YES_NO([${enableval}], [_aal_dbg_dynload], [], [],
                                [AC_MSG_ERROR([Invalid value '${enableval}' for --enable-aal-dbg-dynload])])])
   AC_CACHE_CHECK([whether to enable AALSDK debug output for dll's], [ac_cv_aal_enable_dbg_dynload],
                  [ac_cv_aal_enable_dbg_dynload="${_aal_dbg_dynload}"])
   AM_CONDITIONAL([AAL_COND_ENABLE_DBG_DYNLOAD], [test "x${ac_cv_aal_enable_dbg_dynload}" = xyes])
]) dnl # AALSDK_DEBUG_DYNLOAD

dnl # AALSDK_ASSERT(DEF-VAL)
dnl # ---
dnl # (optional feature) - asserts (ENABLE_ASSERT)
dnl # Users specify whether they want to enable AAL assertions.
AC_DEFUN([AALSDK_ASSERT], [
   _aal_enable_assert="$1"
   AC_ARG_ENABLE([aal-assert],
                 [AS_HELP_STRING([--enable-aal-assert], [AAL Assertions @<:@default=$1@:>@])],
                 [AALSDK_YES_NO([${enableval}], [_aal_enable_assert], [], [],
                                [AC_MSG_ERROR([Invalid value '${enableval}' for --enable-aal-assert])])])
   AC_CACHE_CHECK([whether to enable AALSDK runtime assertions], [ac_cv_aal_enable_assert],
                  [ac_cv_aal_enable_assert="${_aal_enable_assert}"])
   AC_SUBST([AALSDK_ENABLE_ASSERT], [${ac_cv_aal_enable_assert}])
   AM_CONDITIONAL([AAL_COND_ENABLE_ASSERT], [test "x${ac_cv_aal_enable_assert}" = xyes])
   AM_COND_IF([AAL_COND_ENABLE_ASSERT],
              [
               ASSERT_CPPFLAGS='-DENABLE_ASSERT=1'
               CPPFLAGS+=' -DENABLE_ASSERT=1'
              ])
   AC_SUBST([ASSERT_CPPFLAGS], [${ASSERT_CPPFLAGS}])
]) dnl # AALSDK_ASSERT



dnl BASH

dnl # AALSDK_PROG_BASH
dnl # ---
AC_DEFUN([AALSDK_PROG_BASH], [
   AS_IF([test "x${BASH}" = x], [AC_PATH_PROG([BASH], [bash])])
]) dnl # AALSDK_PROG_BASH


dnl Git

dnl # AALSDK_PROG_GIT
dnl # ---
dnl # If git is found in the PATH and GIT-DIR exists, create cache val ac_cv_aal_git_commit_id
dnl # and #define GIT_COMMIT_ID to the decorated name and commit hash. Otherwise, #define
dnl # GIT_COMMIT_ID to "unknown".
AC_DEFUN([AALSDK_PROG_GIT], [
   _aal_git_commit_id=unknown
   AS_IF([test "x${GIT}" = x], [AC_PATH_PROG([GIT], [git])])
   AS_IF([test "x${GIT}" = x],       [],
         [test -e "${srcdir}/.git"],    [_aal_git_commit_id="`${GIT} --git-dir="${srcdir}/.git" log -1 --format='%H%d'`"],
         [test -e "${srcdir}/../.git"], [_aal_git_commit_id="`${GIT} --git-dir="${srcdir}/../.git" log -1 --format='%H%d'`"])
   AC_CACHE_VAL([ac_cv_aal_git_commit_id], [ac_cv_aal_git_commit_id="${_aal_git_commit_id}"])
   AC_DEFINE_UNQUOTED(GIT_COMMIT_ID, "${ac_cv_aal_git_commit_id}", [git commit id.])
   AC_SUBST([GIT_COMMIT_ID], [${ac_cv_aal_git_commit_id}])
]) dnl # AALSDK_PROG_GIT


dnl flex

dnl # AALSDK_PROG_FLEX
dnl # ---
dnl # If flex is missing, report an error and abort.
AC_DEFUN([AALSDK_PROG_FLEX], [
   AC_PROG_LEX
   AS_IF([test "x${ac_cv_prog_LEX}" = x], [AC_MSG_ERROR([Required program flex not found. Install flex and try again.])])
]) dnl # AALSDK_PROG_FLEX


dnl Maintainer

dnl # AALSDK_MAINTAINER
AC_DEFUN([AALSDK_MAINTAINER], [
   AS_IF([test -f "${srcdir}/build/maintainer-check"],
         [
          chmod 'u+rx' "${srcdir}/build/maintainer-check"
          AS_IF([test "x${BASH}" = x], [AC_PATH_PROG([BASH], [bash])])
          AS_IF([test "x${TIME}" = x], [AC_PATH_PROG([TIME], [time])])
          AM_CONDITIONAL([AAL_COND_MAINTAINER], [true])
         ],
         [AM_CONDITIONAL([AAL_COND_MAINTAINER], [false])])
]) dnl # AALSDK_MAINTAINER

