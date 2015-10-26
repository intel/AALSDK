##******************************************************************************
## Part of the Intel(R) QuickAssist Technology Accelerator Abstraction Layer
##
## This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
##         redistributing this file, you may do so under either license.
##
##                            GPL LICENSE SUMMARY
##
##  Copyright(c) 2012-2015, Intel Corporation.
##
##  This program  is  free software;  you  can redistribute it  and/or  modify
##  it  under  the  terms of  version 2 of  the GNU General Public License  as
##  published by the Free Software Foundation.
##
##  This  program  is distributed  in the  hope that it  will  be useful,  but
##  WITHOUT   ANY   WARRANTY;   without   even  the   implied   warranty    of
##  MERCHANTABILITY  or  FITNESS  FOR  A  PARTICULAR  PURPOSE.  See  the   GNU
##  General Public License for more details.
##
##  The  full  GNU  General Public License is  included in  this  distribution
##  in the file called README.GPLV2-LICENSE.TXT.
##
##  Contact Information:
##  Henry Mitchel, henry.mitchel at intel.com
##  77 Reed Rd., Hudson, MA  01749
##
##                                BSD LICENSE
##
##  Copyright(c) 2012-2015, Intel Corporation.
##
##  Redistribution and  use  in source  and  binary  forms,  with  or  without
##  modification,  are   permitted  provided  that  the  following  conditions
##  are met:
##
##    * Redistributions  of  source  code  must  retain  the  above  copyright
##      notice, this list of conditions and the following disclaimer.
##    * Redistributions in  binary form  must  reproduce  the  above copyright
##      notice,  this  list of  conditions  and  the  following disclaimer  in
##      the   documentation   and/or   other   materials   provided  with  the
##      distribution.
##    * Neither   the  name   of  Intel  Corporation  nor  the  names  of  its
##      contributors  may  be  used  to  endorse  or promote  products derived
##      from this software without specific prior written permission.
##
##  THIS  SOFTWARE  IS  PROVIDED  BY  THE  COPYRIGHT HOLDERS  AND CONTRIBUTORS
##  "AS IS"  AND  ANY  EXPRESS  OR  IMPLIED  WARRANTIES,  INCLUDING,  BUT  NOT
##  LIMITED  TO, THE  IMPLIED WARRANTIES OF  MERCHANTABILITY  AND FITNESS  FOR
##  A  PARTICULAR  PURPOSE  ARE  DISCLAIMED.  IN NO EVENT  SHALL THE COPYRIGHT
##  OWNER OR CONTRIBUTORS BE LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,
##  SPECIAL,  EXEMPLARY,  OR  CONSEQUENTIAL   DAMAGES  (INCLUDING,   BUT   NOT
##  LIMITED  TO,  PROCUREMENT  OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF USE,
##  DATA,  OR PROFITS;  OR BUSINESS INTERRUPTION)  HOWEVER  CAUSED  AND ON ANY
##  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT LIABILITY,  OR TORT
##  (INCLUDING  NEGLIGENCE  OR OTHERWISE) ARISING  IN ANY WAY  OUT  OF THE USE
##  OF  THIS  SOFTWARE, EVEN IF ADVISED  OF  THE  POSSIBILITY  OF SUCH DAMAGE.
##******************************************************************************
dnl Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])

AC_INIT([aalkernel], [4.5.0], [joe.grecco@intel.com])
AC_COPYRIGHT([Copyright (c) 2003-2015 Intel Corporation])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_SRCDIR([kOSAL/kOSAL.c])
AC_CONFIG_HEADERS([config.h])
AC_LANG([C])

dnl PACKAGE_URL not available until autoconf 2.64, but we support 2.63.
m4_ifdef([AC_PACKAGE_URL], [],
         [AC_DEFINE_UNQUOTED(PACKAGE_URL, "", [Define to the home page for this package.])])
   
AM_INIT_AUTOMAKE([-Wall -Wno-portability -Werror silent-rules])
AM_SILENT_RULES([yes])
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

AALSDK_COPYRIGHT_NOTICE([Copyright (c) 2003-2015 Intel Corporation])
dnl ############################################################################
dnl ## Module Versioning #######################################################
dnl ############################################################################
AALKERNEL_DRIVER_VERSION([AALBUS],   [aalbus],   [4],[2],[0])
AALKERNEL_DRIVER_VERSION([AALRMS],   [aalrms],   [4],[2],[0])
AALKERNEL_DRIVER_VERSION([AALRMC],   [aalrmc],   [4],[2],[0])
AALKERNEL_DRIVER_VERSION([AALUIDRV], [aaluidrv], [4],[2],[0])
AALKERNEL_DRIVER_VERSION([SPL2PIP],  [spl2pip],  [4],[2],[0])
AALKERNEL_DRIVER_VERSION([CCIV4DRV], [cciv4drv], [4],[2],[0])
AALKERNEL_DRIVER_VERSION([CCIPCIEDRV], [ccipciedrv], [4],[2],[0])

dnl ############################################################################
dnl ## Optional Packages #######################################################
dnl ############################################################################
AALKERNEL_MOD_BUILD_PATH
AALKERNEL_MOD_INSTALL_PATH
AALKERNEL_MODPROBE_CONF_PATH
AALKERNEL_UDEV_RULES_PATH

dnl ############################################################################
dnl ## Optional Features #######################################################
dnl ############################################################################
AALSDK_DEBUG_OUTPUT([yes])
AALSDK_ASSERT([yes])
AALSDK_MAINTAINER

dnl ############################################################################
dnl ## Checks for programs #####################################################
dnl ############################################################################
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_MKDIR_P
AM_PROG_CC_C_O

AALSDK_PROG_GIT

dnl ############################################################################
dnl ## Checks for typedefs, structures, and compiler characteristics  ##########
dnl ############################################################################
AC_C_INLINE

dnl ############################################################################
dnl ## Makefile substitutions ##################################################
dnl ############################################################################

AC_CONFIG_FILES([insdrv:build/insdrv.in], [chmod 'a+x' insdrv])
AC_CONFIG_FILES([rmdrv:build/rmdrv.in],   [chmod 'a+x' rmdrv])
AM_COND_IF([COND_AAL_MODPROBE_CONFIG],
           [AC_CONFIG_FILES([aalsdk.conf:build/aalsdk.conf.in])])
AM_COND_IF([COND_AAL_UDEV_RULES],
           [AC_CONFIG_FILES([40-aalsdk.rules:build/40-aalsdk.rules.in])])

AC_CONFIG_FILES([aalbus/kbuild/Makefile:aalbus/Kbuild.in])
AC_CONFIG_LINKS([aalbus/kbuild/aalbus-device.c:aalbus/aalbus-device.c
                 aalbus/kbuild/aalbus-main.c:aalbus/aalbus-main.c
                 aalbus/kbuild/aalbus-int.h:aalbus/aalbus-int.h
                 aalbus/kbuild/kOSAL.c:kOSAL/kOSAL.c])

AC_CONFIG_FILES([aalresourcemgr_server/kbuild/Makefile:aalresourcemgr_server/Kbuild.in])
AC_CONFIG_LINKS([aalresourcemgr_server/kbuild/aalrm_server-file.c:aalresourcemgr_server/aalrm_server-file.c
                 aalresourcemgr_server/kbuild/aalrm_server-int.h:aalresourcemgr_server/aalrm_server-int.h
                 aalresourcemgr_server/kbuild/aalrm_server-ioctl.c:aalresourcemgr_server/aalrm_server-ioctl.c
                 aalresourcemgr_server/kbuild/aalrm_server-main.c:aalresourcemgr_server/aalrm_server-main.c
                 aalresourcemgr_server/kbuild/aalrm_server-services.c:aalresourcemgr_server/aalrm_server-services.c
                 aalresourcemgr_server/kbuild/aalrms-events-int.h:aalresourcemgr_server/aalrms-events-int.h
                 aalresourcemgr_server/kbuild/kOSAL.c:kOSAL/kOSAL.c])

AC_CONFIG_FILES([aalresourcemgr_client/kbuild/Makefile:aalresourcemgr_client/Kbuild.in])
AC_CONFIG_LINKS([aalresourcemgr_client/kbuild/aalrm-file.c:aalresourcemgr_client/aalrm-file.c
                 aalresourcemgr_client/kbuild/aalrm-int.h:aalresourcemgr_client/aalrm-int.h
                 aalresourcemgr_client/kbuild/aalrm-ioctl.c:aalresourcemgr_client/aalrm-ioctl.c
                 aalresourcemgr_client/kbuild/aalrm-main.c:aalresourcemgr_client/aalrm-main.c
                 aalresourcemgr_client/kbuild/aalrm-events-int.h:aalresourcemgr_client/aalrm-events-int.h
                 aalresourcemgr_client/kbuild/kOSAL.c:kOSAL/kOSAL.c])

AC_CONFIG_FILES([aal_uidriver/kbuild/Makefile:aal_uidriver/Kbuild.in])
AC_CONFIG_LINKS([aal_uidriver/kbuild/aaluidrv-file.c:aal_uidriver/aaluidrv-file.c
                 aal_uidriver/kbuild/aaluidrv-int.h:aal_uidriver/aaluidrv-int.h
                 aal_uidriver/kbuild/aaluidrv-main-linux.c:aal_uidriver/aaluidrv-main-linux.c
                 aal_uidriver/kbuild/aaluidrv-mmap-linux.c:aal_uidriver/aaluidrv-mmap-linux.c
                 aal_uidriver/kbuild/aaluidrv-message.c:aal_uidriver/aaluidrv-message.c
                 aal_uidriver/kbuild/aalui-events-int.h:aal_uidriver/aalui-events-int.h
                 aal_uidriver/kbuild/kOSAL.c:kOSAL/kOSAL.c])

AC_CONFIG_FILES([spl2_pip_dksm/kbuild/Makefile:spl2_pip_dksm/Kbuild.in])
AC_CONFIG_LINKS([spl2_pip_dksm/kbuild/buf-desc.h:spl2_pip_dksm/buf-desc.h
                 spl2_pip_dksm/kbuild/buf-desc-table.h:spl2_pip_dksm/buf-desc-table.h
                 spl2_pip_dksm/kbuild/buf-desc-table-impl.h:spl2_pip_dksm/buf-desc-table-impl.h
                 spl2_pip_dksm/kbuild/buf-mem-type.h:spl2_pip_dksm/buf-mem-type.h
                 spl2_pip_dksm/kbuild/cci3_afu_pip.c:spl2_pip_dksm/cci3_afu_pip.c
                 spl2_pip_dksm/kbuild/cci3_pip_mmap.c:spl2_pip_dksm/cci3_pip_mmap.c
                 spl2_pip_dksm/kbuild/kernver-utils.h:spl2_pip_dksm/kernver-utils.h
                 spl2_pip_dksm/kbuild/mem-internal-fops.h:spl2_pip_dksm/mem-internal-fops.h
                 spl2_pip_dksm/kbuild/mem-internal-ioctl.h:spl2_pip_dksm/mem-internal-ioctl.h
                 spl2_pip_dksm/kbuild/mem-pte.h:spl2_pip_dksm/mem-pte.h
                 spl2_pip_dksm/kbuild/mem-sess.h:spl2_pip_dksm/mem-sess.h
                 spl2_pip_dksm/kbuild/mem-virt.h:spl2_pip_dksm/mem-virt.h
                 spl2_pip_dksm/kbuild/mem-virt-impl.h:spl2_pip_dksm/mem-virt-impl.h
                 spl2_pip_dksm/kbuild/spl2_afu_pip.c:spl2_pip_dksm/spl2_afu_pip.c
                 spl2_pip_dksm/kbuild/spl2diag.h:spl2_pip_dksm/spl2diag.h
                 spl2_pip_dksm/kbuild/spl2_mafu_pip.c:spl2_pip_dksm/spl2_mafu_pip.c
                 spl2_pip_dksm/kbuild/spl2mem.h:spl2_pip_dksm/spl2mem.h
                 spl2_pip_dksm/kbuild/spl2mem-kern.h:spl2_pip_dksm/spl2mem-kern.h
                 spl2_pip_dksm/kbuild/spl2_pip_internal.h:spl2_pip_dksm/spl2_pip_internal.h
                 spl2_pip_dksm/kbuild/spl2_pip_main.c:spl2_pip_dksm/spl2_pip_main.c
                 spl2_pip_dksm/kbuild/spl2_pip_mmap.c:spl2_pip_dksm/spl2_pip_mmap.c
                 spl2_pip_dksm/kbuild/spl2_pip_nopcie.c:spl2_pip_dksm/spl2_pip_nopcie.c
                 spl2_pip_dksm/kbuild/spl2_pip_public.h:spl2_pip_dksm/spl2_pip_public.h
                 spl2_pip_dksm/kbuild/spl2_primitives.c:spl2_pip_dksm/spl2_primitives.c
                 spl2_pip_dksm/kbuild/spl2_session.c:spl2_pip_dksm/spl2_session.c
                 spl2_pip_dksm/kbuild/spl2_session.h:spl2_pip_dksm/spl2_session.h
                 spl2_pip_dksm/kbuild/xsid.h:spl2_pip_dksm/xsid.h
                 spl2_pip_dksm/kbuild/kOSAL.c:kOSAL/kOSAL.c])

AC_CONFIG_FILES([cciv4_driver/kbuild/Makefile:cciv4_driver/Kbuild.in])
AC_CONFIG_LINKS([cciv4_driver/kbuild/cciv4_driver_main.c:cciv4_driver/cciv4_driver_main.c
                 cciv4_driver/kbuild/cciv4_driver_internal.h:cciv4_driver/cciv4_driver_internal.h
                 cciv4_driver/kbuild/cciv4_simulator.c:cciv4_driver/cciv4_simulator.c
                 cciv4_driver/kbuild/cciv4_simulator.h:cciv4_driver/cciv4_simulator.h
                 cciv4_driver/kbuild/cciv4_common.c:cciv4_driver/cciv4_common.c
                 cciv4_driver/kbuild/cciv4_PIPsession.h:cciv4_driver/cciv4_PIPsession.h
                 cciv4_driver/kbuild/cciv4_PIPsession.c:cciv4_driver/cciv4_PIPsession.c
                 cciv4_driver/kbuild/cciv4_sim_afu_pip.c:cciv4_driver/cciv4_sim_afu_pip.c  
                 cciv4_driver/kbuild/cciv4_sim_mafu_pip.c:cciv4_driver/cciv4_sim_mafu_pip.c      
                 cciv4_driver/kbuild/cciv4_sim_afu_mmap.c:cciv4_driver/cciv4_sim_afu_mmap.c                                                                                    
                 cciv4_driver/kbuild/kOSAL.c:kOSAL/kOSAL.c])

AC_CONFIG_FILES([cci_PCIe_driver/kbuild/Makefile:cci_PCIe_driver/Kbuild.in])
AC_CONFIG_LINKS([cci_PCIe_driver/kbuild/cci_pcie_driver_main.c:cci_PCIe_driver/cci_pcie_driver_main.c
                 cci_PCIe_driver/kbuild/cci_pcie_driver_internal.h:cci_PCIe_driver/cci_pcie_driver_internal.h
                 cci_PCIe_driver/kbuild/cci_pcie_driver_simulator.c:cci_PCIe_driver/cci_pcie_driver_simulator.c
                 cci_PCIe_driver/kbuild/cci_pcie_driver_simulator.h:cci_PCIe_driver/cci_pcie_driver_simulator.h
                 cci_PCIe_driver/kbuild/cci_pcie_driver_PIPsession.c:cci_PCIe_driver/cci_pcie_driver_PIPsession.c
                 cci_PCIe_driver/kbuild/cci_pcie_driver_PIPsession.h:cci_PCIe_driver/cci_pcie_driver_PIPsession.h                 
                 cci_PCIe_driver/kbuild/cci_pcie_common.c:cci_PCIe_driver/cci_pcie_common.c
                 cci_PCIe_driver/kbuild/cci_pcie_driver_pdde.c:cci_PCIe_driver/cci_pcie_driver_pdde.c
                 cci_PCIe_driver/kbuild/cci_pcie_driver_umapi_linux.c:cci_PCIe_driver/cci_pcie_driver_umapi_linux.c
                 cci_PCIe_driver/kbuild/cci_pcie_driver_umapi_common.c:cci_PCIe_driver/cci_pcie_driver_umapi_common.c
                 cci_PCIe_driver/kbuild/cci_pcie_driver_umapi_linux.h:cci_PCIe_driver/cci_pcie_driver_umapi_linux.h
                 cci_PCIe_driver/kbuild/cciui-events.h:cci_PCIe_driver/cciui-events.h
                 cci_PCIe_driver/kbuild/kOSAL.c:kOSAL/kOSAL.c])

AC_CONFIG_FILES([Makefile
                 aalbus/Makefile
                 aalresourcemgr_server/Makefile
                 aalresourcemgr_client/Makefile
                 aal_uidriver/Makefile
                 spl2_pip_dksm/Makefile
                 cciv4_driver/Makefile
                 cci_PCIe_driver/Makefile])

AC_OUTPUT

dnl ############################################################################
dnl ## Config / Next Steps #####################################################
dnl ############################################################################
echo \
"
${ac_cv_aal_copyright}
Intel(R) QuickAssist Technology Accelerator Abstraction Layer Drivers for Linux

${PACKAGE_NAME} ${PACKAGE_VERSION}

Kernel build    : ${ac_cv_aal_os_kernel_build_path}
Module install  : ${ac_cv_aal_os_module_install_path}"

AM_COND_IF([COND_AAL_MODPROBE_CONFIG],
           [echo "modprobe config : ${ac_cv_aal_modprobe_config_path}"])

AM_COND_IF([COND_AAL_UDEV_RULES],
           [echo "udev rules      : ${ac_cv_aal_udev_rules_path}"])

echo \
"
Prefix          : ${prefix}
C compiler      : ${CC}

To build        : make
To install      : sudo make install
"

