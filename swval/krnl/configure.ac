# INTEL CONFIDENTIAL - For Intel Internal Use Only
dnl Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])

AC_INIT([swval-krnl], [1.1.1], [tim.whisonant@intel.com])
AC_COPYRIGHT([INTEL CONFIDENTIAL - For Intel Internal Use Only])
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

AALSDK_COPYRIGHT_NOTICE([INTEL CONFIDENTIAL - For Intel Internal Use Only])
dnl ############################################################################
dnl ## Module Versioning #######################################################
dnl ############################################################################
AALKERNEL_DRIVER_VERSION([AALBUS],     [aalbus],     [1],[1],[1])
AALKERNEL_DRIVER_VERSION([AALRMS],     [aalrms],     [1],[1],[1])
AALKERNEL_DRIVER_VERSION([AALRMC],     [aalrmc],     [1],[1],[1])
AALKERNEL_DRIVER_VERSION([CCIPCIEDRV], [ccipciedrv], [1],[1],[1])

dnl ############################################################################
dnl ## Optional Packages #######################################################
dnl ############################################################################
AALKERNEL_MOD_BUILD_PATH
AALKERNEL_MOD_INSTALL_PATH
AALKERNEL_MODPROBE_CONF_PATH
AALKERNEL_UDEV_RULES_PATH

WITH_GCOV_ANALYSIS

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
VERIFY_OPTIONS

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


AC_CONFIG_FILES([cci_PCIe_driver/kbuild/Makefile:cci_PCIe_driver/Kbuild.in])
AC_CONFIG_LINKS([cci_PCIe_driver/kbuild/cci_pcie_driver_main.c:cci_PCIe_driver/cci_pcie_driver_main.c
                 cci_PCIe_driver/kbuild/cci_pcie_driver_internal.h:cci_PCIe_driver/cci_pcie_driver_internal.h
                 cci_PCIe_driver/kbuild/cci_pcie_driver_simulator.c:cci_PCIe_driver/cci_pcie_driver_simulator.c
                 cci_PCIe_driver/kbuild/cci_pcie_driver_simulator.h:cci_PCIe_driver/cci_pcie_driver_simulator.h
                 cci_PCIe_driver/kbuild/ccip_sim_mmio.c:cci_PCIe_driver/ccip_sim_mmio.c             
                 cci_PCIe_driver/kbuild/cci_pcie_driver_PIPsession.c:cci_PCIe_driver/cci_pcie_driver_PIPsession.c
                 cci_PCIe_driver/kbuild/cci_pcie_driver_PIPsession.h:cci_PCIe_driver/cci_pcie_driver_PIPsession.h
                 cci_PCIe_driver/kbuild/cci_pcie_common.c:cci_PCIe_driver/cci_pcie_common.c
                 cci_PCIe_driver/kbuild/cci_pcie_driver_pdde_linux.c:cci_PCIe_driver/cci_pcie_driver_pdde_linux.c
                 cci_PCIe_driver/kbuild/cci_pcie_driver_umapi_linux.c:cci_PCIe_driver/cci_pcie_driver_umapi_linux.c
                 cci_PCIe_driver/kbuild/cci_pcie_driver_umapi_common.c:cci_PCIe_driver/cci_pcie_driver_umapi_common.c
                 cci_PCIe_driver/kbuild/cci_pcie_driver_umapi_linux.h:cci_PCIe_driver/cci_pcie_driver_umapi_linux.h
                 cci_PCIe_driver/kbuild/ccipdrv-events.h:cci_PCIe_driver/ccipdrv-events.h
                 cci_PCIe_driver/kbuild/ccip_fme.c:cci_PCIe_driver/ccip_fme.c
                 cci_PCIe_driver/kbuild/ccip_fme.h:cci_PCIe_driver/ccip_fme.h
                 cci_PCIe_driver/kbuild/ccip_port.c:cci_PCIe_driver/ccip_port.c
                 cci_PCIe_driver/kbuild/ccip_port.h:cci_PCIe_driver/ccip_port.h
                 cci_PCIe_driver/kbuild/ccip_afu.c:cci_PCIe_driver/ccip_afu.c
                 cci_PCIe_driver/kbuild/ccip_stap.c:cci_PCIe_driver/ccip_stap.c
                 cci_PCIe_driver/kbuild/ccip_pr.c:cci_PCIe_driver/ccip_pr.c
                 cci_PCIe_driver/kbuild/ccip_perfmon.h:cci_PCIe_driver/ccip_perfmon.h
                 cci_PCIe_driver/kbuild/ccip_perfmon.c:cci_PCIe_driver/ccip_perfmon.c
                 cci_PCIe_driver/kbuild/kOSAL.c:kOSAL/kOSAL.c])

AC_CONFIG_FILES([Makefile
                 aalbus/Makefile
                 aalresourcemgr_server/Makefile
                 aalresourcemgr_client/Makefile
                 cci_PCIe_driver/Makefile])

AC_OUTPUT

dnl ############################################################################
dnl ## Config / Next Steps #####################################################
dnl ############################################################################
echo \
"
${ac_cv_aal_copyright}

AALSDK Validation Project"
SHOW_OPTIONS
