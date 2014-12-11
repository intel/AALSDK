dnl ****************************************************************************
dnl  Part of the Intel(R) QuickAssist Technology Accelerator Abstraction Layer
dnl 
dnl  This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
dnl          redistributing this file, you may do so under either license.
dnl 
dnl                             GPL LICENSE SUMMARY
dnl 
dnl   Copyright(c) 2014, Intel Corporation.
dnl 
dnl   This program  is  free software;  you  can redistribute it  and/or  modify
dnl   it  under  the  terms of  version 2 of  the GNU General Public License  as
dnl   published by the Free Software Foundation.
dnl 
dnl   This  program  is distributed  in the  hope that it  will  be useful,  but
dnl   WITHOUT   ANY   WARRANTY;   without   even  the   implied   warranty    of
dnl   MERCHANTABILITY  or  FITNESS  FOR  A  PARTICULAR  PURPOSE.  See  the   GNU
dnl   General Public License for more details.
dnl 
dnl   The  full  GNU  General Public License is  included in  this  distribution
dnl   in the file called README.GPLV2-LICENSE.TXT.
dnl 
dnl   Contact Information:
dnl   Henry Mitchel, henry.mitchel at intel.com
dnl   77 Reed Rd., Hudson, MA  01749
dnl 
dnl                                 BSD LICENSE
dnl 
dnl   Copyright(c) 2014, Intel Corporation.
dnl 
dnl   Redistribution and  use  in source  and  binary  forms,  with  or  without
dnl   modification,  are   permitted  provided  that  the  following  conditions
dnl   are met:
dnl 
dnl     * Redistributions  of  source  code  must  retain  the  above  copyright
dnl       notice, this list of conditions and the following disclaimer.
dnl     * Redistributions in  binary form  must  reproduce  the  above copyright
dnl       notice,  this  list of  conditions  and  the  following disclaimer  in
dnl       the   documentation   and/or   other   materials   provided  with  the
dnl       distribution.
dnl     * Neither   the  name   of  Intel  Corporation  nor  the  names  of  its
dnl       contributors  may  be  used  to  endorse  or promote  products derived
dnl       from this software without specific prior written permission.
dnl 
dnl   THIS  SOFTWARE  IS  PROVIDED  BY  THE  COPYRIGHT HOLDERS  AND CONTRIBUTORS
dnl   "AS IS"  AND  ANY  EXPRESS  OR  IMPLIED  WARRANTIES,  INCLUDING,  BUT  NOT
dnl   LIMITED  TO, THE  IMPLIED WARRANTIES OF  MERCHANTABILITY  AND FITNESS  FOR
dnl   A  PARTICULAR  PURPOSE  ARE  DISCLAIMED.  IN NO EVENT  SHALL THE COPYRIGHT
dnl   OWNER OR CONTRIBUTORS BE LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,
dnl   SPECIAL,  EXEMPLARY,  OR  CONSEQUENTIAL   DAMAGES  (INCLUDING,   BUT   NOT
dnl   LIMITED  TO,  PROCUREMENT  OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF USE,
dnl   DATA,  OR PROFITS;  OR BUSINESS INTERRUPTION)  HOWEVER  CAUSED  AND ON ANY
dnl   THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT LIABILITY,  OR TORT
dnl   (INCLUDING  NEGLIGENCE  OR OTHERWISE) ARISING  IN ANY WAY  OUT  OF THE USE
dnl   OF  THIS  SOFTWARE, EVEN IF ADVISED  OF  THE  POSSIBILITY  OF SUCH DAMAGE.
dnl ****************************************************************************
dnl  Intel(R) Accelerator Abstraction Layer Library Software Developer Kit (SDK)
dnl   Content:
dnl      Intel(R) Accelerator Abstraction Layer Library
dnl      m4/aalkernel.m4
dnl   Author:
dnl      Tim Whisonant, Intel Corporation
dnl   History:
dnl      08/16/2014    TSW   Initial version
dnl ****************************************************************************

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


dnl OS kernel sources

dnl # AALKERNEL_MOD_BUILD_PATH
dnl # ---
dnl # Give users the option to specify where the Linux kernel sources / build env live. (typically, /lib/modules/`uname -r`/build)
dnl # Prompt with an error message and exit when not found.
dnl # Creates cache val:
dnl #    ac_cv_aal_os_kernel_build_path
dnl # And matching substitution:
dnl #    @AAL_OS_KERNEL_BUILD_PATH@
AC_DEFUN([AALKERNEL_MOD_BUILD_PATH], [
   AC_ARG_WITH([module-build],
               [AS_HELP_STRING([--with-module-build], [Specify the Linux module build path @<:@default=/lib/modules/`uname -r`/build@:>@])],
               [AS_IF([test -d "${withval}"], [AC_CACHE_VAL([ac_cv_aal_os_kernel_build_path], [ac_cv_aal_os_kernel_build_path="${withval}"])])],
               [AS_IF([test -d "/lib/modules/`uname -r`/build"], [AC_CACHE_VAL([ac_cv_aal_os_kernel_build_path], [ac_cv_aal_os_kernel_build_path="/lib/modules/`uname -r`/build"])])])
   AC_MSG_CHECKING([for Linux module build])
   AS_IF([test "x${ac_cv_aal_os_kernel_build_path}" = x], [AC_MSG_ERROR([Linux module build path not found. Please use --with-module-build=/path/to/linux/module/build])],
                                                         [AC_SUBST([AAL_OS_KERNEL_BUILD_PATH], [${ac_cv_aal_os_kernel_build_path}])])
   AC_MSG_RESULT([${ac_cv_aal_os_kernel_build_path}])
]) dnl # AALKERNEL_MOD_BUILD_PATH


dnl OS kernel module install path

dnl # AALKERNEL_MOD_INSTALL_PATH
dnl # ---
dnl # Give users the option to specify where the Linux module device drivers should be installed. (typically, /lib/modules/`uname -r`)
dnl # Prompt with an error message and exit when not found.
dnl # Creates cache val:
dnl #    ac_cv_aal_os_module_install_path
dnl # And matching substitution:
dnl #    @AAL_OS_MODULE_INSTALL_PATH@
AC_DEFUN([AALKERNEL_MOD_INSTALL_PATH], [
   AC_ARG_WITH([module-install],
               [AS_HELP_STRING([--with-module-install], [Specify the Linux module installation path @<:@default=/lib/modules/`uname -r`/aalsdk@:>@])],
               [AS_IF([test -d "${withval}"], [AC_CACHE_VAL([ac_cv_aal_os_module_install_path], [ac_cv_aal_os_module_install_path="${withval}"])])],
               [AS_IF([test -d "/lib/modules/`uname -r`"], [AC_CACHE_VAL([ac_cv_aal_os_module_install_path], [ac_cv_aal_os_module_install_path="/lib/modules/`uname -r`/aalsdk"])])])
   AC_MSG_CHECKING([for Linux module installation path])
   AS_IF([test "x${ac_cv_aal_os_module_install_path}" = x], [AC_MSG_ERROR([Linux module installation path not found. Please use --with-module-install=/path/to/linux/modules])],
                                                           [AC_SUBST([AAL_OS_MODULE_INSTALL_PATH], [${ac_cv_aal_os_module_install_path}])])
   AC_MSG_RESULT([${ac_cv_aal_os_module_install_path}])
]) dnl # AALKERNEL_MOD_INSTALL_PATH


dnl modprobe configuration

dnl # AALKERNEL_MODPROBE_CONF_PATH
dnl # ---
dnl # Give users the option to specify whether and where to install the modprobe config file for Linux module drivers. (typically, /etc/modprobe.d)
dnl # Creates cache val:
dnl #    ac_cv_aal_modprobe_config_path
dnl # And matching substitutions:
dnl #    @AAL_MODPROBE_CONFIG_PATH@
AC_DEFUN([AALKERNEL_MODPROBE_CONF_PATH], [
   AC_ARG_WITH([modprobe-config],
               [AS_HELP_STRING([--with-modprobe-config], [Specify the modprobe configuration file installation path @<:@default=none@:>@])],
               [],
               [with_modprobe_config=no])

   AALSDK_YES_NO([${with_modprobe_config}], [with_modprobe_config],
                 [AS_IF([test -d '/etc/modprobe.d'], [with_modprobe_config='/etc/modprobe.d'])],
                 [],
                 [AS_IF([test -d "${with_modprobe_config}"], [],
                        [AC_MSG_ERROR([Invalid value '${with_modprobe_config}' for --with-modprobe-config])])])

   AC_MSG_CHECKING([whether and where to install modprobe config file])
   AC_CACHE_VAL([ac_cv_aal_modprobe_config_path], [ac_cv_aal_modprobe_config_path="${with_modprobe_config}"])
   AM_CONDITIONAL([COND_AAL_MODPROBE_CONFIG], [test "x${ac_cv_aal_modprobe_config_path}" != xno])
   AC_SUBST([AAL_MODPROBE_CONFIG_PATH], [${ac_cv_aal_modprobe_config_path}])
   AC_MSG_RESULT([${ac_cv_aal_modprobe_config_path}])
]) dnl # AALKERNEL_MODPROBE_CONF_PATH


dnl udev rules

dnl # AALKERNEL_UDEV_RULES_PATH
dnl # ---
dnl # Give users the option to specify whether and where to install the udev rules file for the Linux module drivers. (typically, /lib/udev/rules.d)
dnl # Creates cache val:
dnl #    ac_cv_aal_udev_rules_path
dnl # And matching substitutions:
dnl #    @AAL_UDEV_RULES_PATH@
AC_DEFUN([AALKERNEL_UDEV_RULES_PATH], [
   AC_ARG_WITH([udev-rules],
               [AS_HELP_STRING([--with-udev-rules], [Specify the udev rules installation path @<:@default=none@:>@])],
               [],
               [with_udev_rules=no])

   AALSDK_YES_NO([${with_udev_rules}], [with_udev_rules],
                 [AS_IF([test -d '/lib/udev/rules.d'], [with_udev_rules='/lib/udev/rules.d'])],            
                 [],
                 [AS_IF([test -d "${with_udev_rules}"], [],
                        [AC_MSG_ERROR([Invalid value '${with_udev_rules}' for --with-udev-rules])])])

   AC_MSG_CHECKING([whether and where to install udev rules])
   AC_CACHE_VAL([ac_cv_aal_udev_rules_path], [ac_cv_aal_udev_rules_path="${with_udev_rules}"])
   AM_CONDITIONAL([COND_AAL_UDEV_RULES], [test "x${ac_cv_aal_udev_rules_path}" != xno])
   AC_SUBST([AAL_UDEV_RULES_PATH], [${ac_cv_aal_udev_rules_path}])
   AC_MSG_RESULT([${ac_cv_aal_udev_rules_path}])
]) dnl # AALKERNEL_UDEV_RULES_PATH


dnl Driver Versioning

dnl # AALKERNEL_DRIVER_VERSION(DEFNAME, DRVNAME, MAJOR, MINOR, RELEASE)
dnl # ---
dnl # Use to specify the version of a driver.
dnl #
dnl # DEFNAME is the #define name of the driver, the name prefix given to the following
dnl # defines: eg AALBUS results in
dnl #    AALBUS_VERSION_MAJOR   : (int)          version[MAJOR]   (major.minor.release scheme)
dnl #    AALBUS_VERSION_MINOR   : (int)          version[MINOR]   (major.minor.release scheme)
dnl #    AALBUS_VERSION_RELEASE : (int)          version[RELEASE] (major.minor.release scheme)
dnl #    AALBUS_VERSION         : (const char *) "MAJOR:MINOR:RELEASE"
dnl #
dnl # Additionally, the following substitutions are made: (cont'ing the AALBUS example)
dnl #    @AALBUS_DRV_NAME@ -> DRVNAME, the driver name.
dnl #    @AALBUS_VERSION_MAJOR@
dnl #    @AALBUS_VERSION_MINOR@
dnl #    @AALBUS_VERSION_RELEASE@
dnl #    @AALBUS_VERSION@
AC_DEFUN([AALKERNEL_DRIVER_VERSION], [
AC_DEFINE_UNQUOTED($1_VERSION_MAJOR, $3,
                   [define to current version of $1 (major.minor.release version scheme)])
AC_DEFINE_UNQUOTED($1_VERSION_MINOR, $4,
                   [define to revision version of $1 (major.minor.release version scheme)])
AC_DEFINE_UNQUOTED($1_VERSION_RELEASE, $5,
                   [define to age version of $1 (major.minor.release version scheme)])
AC_DEFINE_UNQUOTED($1_VERSION, "$3.$4.$5",
                   [define to full version of $1 (major.minor.release version scheme)])
AC_SUBST([$1_DRV_NAME],       [$2])
AC_SUBST([$1_VERSION_MAJOR],  [$3])
AC_SUBST([$1_VERSION_MINOR],  [$4])
AC_SUBST([$1_VERSION_RELEASE],[$5])
AC_SUBST([$1_VERSION],        [$3.$4.$5])
]) dnl # AALKERNEL_DRIVER_VERSION


dnl Optional Features



dnl # AALSDK_DEBUG_OUTPUT(DEF-VAL)
dnl # ---
dnl # (optional feature) - debug output (ENABLE_DEBUG)
dnl # Users specify whether they want to see debug print output / enable debugging features.
AC_DEFUN([AALSDK_DEBUG_OUTPUT], [
_aal_dbg_output="$1"
AC_ARG_ENABLE([aal-dbg],
              [AS_HELP_STRING([--enable-aal-dbg], [AAL Debug output @<:@default=$1@:>@])],
              [AALSDK_YES_NO([${enableval}], [_aal_dbg_output], [], [],
                             [AC_MSG_ERROR([Invalid value '${enableval}' for --enable-aal-dbg])])])
AC_CACHE_CHECK([whether to enable AALSDK debug output], [ac_cv_aal_enable_dbg_output],
               [ac_cv_aal_enable_dbg_output="${_aal_dbg_output}"])
AM_CONDITIONAL([AAL_COND_ENABLE_DBG_OUTPUT], [test "x${ac_cv_aal_enable_dbg_output}" = xyes])
if test x"${_aal_dbg_output}" = xyes; then
   _aal_dbg_output=1
else
   _aal_dbg_output=0
fi
AC_SUBST([ENABLE_DEBUG], [${_aal_dbg_output}])
]) dnl # AALSDK_DEBUG_OUTPUT

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
AM_CONDITIONAL([AAL_COND_ENABLE_ASSERT], [test "x${ac_cv_aal_enable_assert}" = xyes])
if test x"${_aal_enable_assert}" = xyes; then
   _aal_enable_assert=1
else
   _aal_enable_assert=0
fi
AC_SUBST([ENABLE_ASSERT], [${_aal_enable_assert}])
]) dnl # AALSDK_ASSERT


dnl Maintainer

dnl # AALSDK_MAINTAINER
AC_DEFUN([AALSDK_MAINTAINER], [
if test -f "${ac_cv_aal_package_srcdir}/build/maintainer-check" ; then
   chmod 'u+rx' "${ac_cv_aal_package_srcdir}/build/maintainer-check"
   test "x${BASH}" = x && AC_PATH_PROG([BASH], [bash])
   test "x${TIME}" = x && AC_PATH_PROG([TIME], [time])
   AM_CONDITIONAL([AAL_COND_MAINTAINER],[true])
else
   AM_CONDITIONAL([AAL_COND_MAINTAINER],[false])
fi
]) dnl # AALSDK_MAINTAINER


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
         [test -d "${srcdir}/.git"],    [_aal_git_commit_id="`${GIT} --git-dir="${srcdir}/.git" log -1 --format='%H%d'`"],
         [test -d "${srcdir}/../.git"], [_aal_git_commit_id="`${GIT} --git-dir="${srcdir}/../.git" log -1 --format='%H%d'`"])
   AC_CACHE_VAL([ac_cv_aal_git_commit_id], [ac_cv_aal_git_commit_id="${_aal_git_commit_id}"])
   AC_DEFINE_UNQUOTED(GIT_COMMIT_ID, "${ac_cv_aal_git_commit_id}", [git commit id.])
]) dnl # AALSDK_PROG_GIT

