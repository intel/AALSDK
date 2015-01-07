##******************************************************************************
## Part of the Intel(R) QuickAssist Technology Accelerator Abstraction Layer
##
## This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
##         redistributing this file, you may do so under either license.
##
##                            GPL LICENSE SUMMARY
##
##  Copyright(c) 2008-2015, Intel Corporation.
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
##  Copyright(c) 2008-2015, Intel Corporation.
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
##****************************************************************************
##  FILE: aalkernel/aal_uidriver/Makefile
##     Intel(R) QuickAssist Technology Accelerator Abstraction Layer (AAL)
##     Intel(R) QuickAssist Technology AAL UIDrv
##        Driver Module for Linux
##  Author:
##     Tim Whisonant, Intel Corporation
##  Created:
##     08/15/2014
##****************************************************************************
MY_KBUILD_VERBOSE    = $(my__kbuild_verbose_$(V))
my__kbuild_verbose_  = $(AM_DEFAULT_VERBOSITY)
my__kbuild_verbose_0 = 0
my__kbuild_verbose_1 = 1

moduledir=@AAL_OS_MODULE_INSTALL_PATH@

module_DATA=\
kbuild/@SPL2PIP_DRV_NAME@.ko

EXTRA_DIST=Kbuild.in

EXTRA_PROGRAMS=driver

driver_SOURCES=\
buf-desc.h \
buf-desc-table.h \
buf-desc-table-impl.h \
buf-mem-type.h \
cci3_afu_pip.c \
cci3_pip_mmap.c \
kernver-utils.h \
mem-internal-fops.h \
mem-internal-ioctl.h \
mem-pte.h \
mem-sess.h \
mem-virt.h \
mem-virt-impl.h \
spl2_afu_pip.c \
spl2diag.h \
spl2_mafu_pip.c \
spl2mem.h \
spl2mem-kern.h \
spl2_pip_internal.h \
spl2_pip_main.c \
spl2_pip_mmap.c \
spl2_pip_nopcie.c \
spl2_pip_public.h \
spl2_primitives.c \
spl2_session.c \
spl2_session.h \
xsid.h

kbuild/@SPL2PIP_DRV_NAME@.ko: kbuild/Module.symvers

kbuild/Module.symvers: $(abs_top_builddir)/aalbus/kbuild/Module.symvers $(driver_SOURCES) $(abs_top_builddir)/spl2_pip_dksm/kbuild/kOSAL.c $(abs_top_builddir)/spl2_pip_dksm/kbuild/Makefile
	@cp --preserve=all "$(abs_top_builddir)/aalbus/kbuild/Module.symvers" "$(abs_top_builddir)/spl2_pip_dksm/kbuild" && \
$(MAKE) -C "@AAL_OS_KERNEL_BUILD_PATH@" KBUILD_VERBOSE=$(MY_KBUILD_VERBOSE) M="$(abs_top_builddir)/spl2_pip_dksm/kbuild" aalkernelinc="$(abs_top_srcdir)/include" modules && \
if [ -f "$(abs_top_builddir)/spl2_pip_dksm/kbuild/@SPL2PIP_DRV_NAME@.o" ]; then touch "$(abs_top_builddir)/spl2_pip_dksm/kbuild/@SPL2PIP_DRV_NAME@.o" ; fi && \
if [ -f "$(abs_top_builddir)/spl2_pip_dksm/kbuild/@SPL2PIP_DRV_NAME@.mod.o" ]; then touch "$(abs_top_builddir)/spl2_pip_dksm/kbuild/@SPL2PIP_DRV_NAME@.mod.o" ; fi && \
if [ -f "$(abs_top_builddir)/spl2_pip_dksm/kbuild/Module.symvers" ]; then touch "$(abs_top_builddir)/spl2_pip_dksm/kbuild/Module.symvers" ; fi && \
if [ -f "$(abs_top_builddir)/spl2_pip_dksm/kbuild/@SPL2PIP_DRV_NAME@.ko" ]; then touch "$(abs_top_builddir)/spl2_pip_dksm/kbuild/@SPL2PIP_DRV_NAME@.ko" ; fi

clean-local:
	@$(MAKE) -C "@AAL_OS_KERNEL_BUILD_PATH@" KBUILD_VERBOSE=$(MY_KBUILD_VERBOSE) M="$(abs_top_builddir)/spl2_pip_dksm/kbuild" clean
