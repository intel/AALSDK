##******************************************************************************
## Part of the Intel(R) QuickAssist Technology Accelerator Abstraction Layer
##
## This  file  is  provided  under  a  dual BSD/GPLv2  license.  When using or
##         redistributing this file, you may do so under either license.
##
##                            GPL LICENSE SUMMARY
##
##  Copyright(c) 2012-2014, Intel Corporation.
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
##  Copyright(c) 2012-2014, Intel Corporation.
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
ACLOCAL_AMFLAGS=-I m4

.PHONY: dist-file-list
dist-file-list: dist-all
	@tar ztf "$(top_builddir)/$(distdir).tar.gz" | sed -e '/\/$$/d' -e 's/$(distdir)/\./' >"$(top_builddir)/dist-file-list"
	@$(RM) $(DIST_ARCHIVES)

clean-local:
	@$(RM) $(top_builddir)/dist-file-list

if AAL_COND_MAINTAINER

MAINTAINER_CHECK=maintainer-check
maintainer-check:
	   @+$(TIME) $(BASH) -c '$(top_srcdir)/build/maintainer-check \
			--conf='$(abs_top_srcdir)/configure' \
			--srcdir='$(abs_top_srcdir)' \
			--distdir='$(distdir)' \
			--wkspc='$(abs_top_builddir)/maintainer-check''

endif
.PHONY: $(MAINTAINER_CHECK)

SUBDIRS=\
aalbus \
aalresourcemgr_server \
aalresourcemgr_client \
aal_uidriver \
spl2_pip_dksm

aalsdkhdrsdir=$(includedir)/aalsdk
kernelhdrsdir=$(aalsdkhdrsdir)/kernel
encoderhdrsdir=$(kernelhdrsdir)/encoder_sample_kbae

kernelhdrs_HEADERS=\
include/aalsdk/kernel/aalbus-device.h \
include/aalsdk/kernel/aalbus.h \
include/aalsdk/kernel/aalbus_imonitorconfig.h \
include/aalsdk/kernel/aalbus-ipip.h \
include/aalsdk/kernel/aalbus_iupdate_config.h \
include/aalsdk/kernel/aaldefs.h \
include/aalsdk/kernel/aaldevice.h \
include/aalsdk/kernel/aalids.h \
include/aalsdk/kernel/aalinterface.h \
include/aalsdk/kernel/aalmafu-events.h \
include/aalsdk/kernel/aalmafu.h \
include/aalsdk/kernel/aalqueue.h \
include/aalsdk/kernel/aalrm_client.h \
include/aalsdk/kernel/aalrm.h \
include/aalsdk/kernel/aalrm_server.h \
include/aalsdk/kernel/aalrm_server-services.h \
include/aalsdk/kernel/aalrm-services.h \
include/aalsdk/kernel/aalsession.h \
include/aalsdk/kernel/AALTransactionID_s.h \
include/aalsdk/kernel/aaltypes.h \
include/aalsdk/kernel/aalui-events.h \
include/aalsdk/kernel/aalui.h \
include/aalsdk/kernel/AALWorkspace.h \
include/aalsdk/kernel/aalwsservice.h \
include/aalsdk/kernel/ahmpip.h \
include/aalsdk/kernel/ahmpipdefs.h \
include/aalsdk/kernel/ccidefs.h \
include/aalsdk/kernel/fappip.h \
include/aalsdk/kernel/KernelStructs.h \
include/aalsdk/kernel/kosal.h \
include/aalsdk/kernel/spl2defs.h \
include/aalsdk/kernel/splpip.h \
include/aalsdk/kernel/vafu2defs.h

encoderhdrs_HEADERS=\
include/aalsdk/kernel/encoder_sample_kbae/encoder_kbae-public.h

if COND_AAL_MODPROBE_CONFIG
modprobeconfdir=@AAL_MODPROBE_CONFIG_PATH@
modprobeconf_DATA=\
aalsdk.conf
endif

if COND_AAL_UDEV_RULES
udevrulesdir=@AAL_UDEV_RULES_PATH@
udevrules_DATA=\
40-aalsdk.rules
endif

EXTRA_DIST=\
README.GPLV2-LICENSE.TXT \
build/aalsdk.conf.in \
build/40-aalsdk.rules.in \
build/insdrv.in \
build/rmdrv.in

