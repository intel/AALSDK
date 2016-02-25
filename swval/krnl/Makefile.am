# INTEL CONFIDENTIAL - For Intel Internal Use Only
ACLOCAL_AMFLAGS=-I m4

.PHONY: dist-file-list
dist-file-list: dist-all
	@tar ztf "$(top_builddir)/$(distdir).tar.gz" | sed -e '/\/$$/d' -e 's/$(distdir)/\./' >"$(top_builddir)/dist-file-list"
	@$(RM) $(DIST_ARCHIVES)

clean-local:
	@$(RM) $(top_builddir)/dist-file-list


SUBDIRS=\
aalbus \
aalresourcemgr_server \
aalresourcemgr_client \
cci_PCIe_driver

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
include/aalsdk/kernel/iaaldevice.h \
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
include/aalsdk/kernel/AALTransactionID_s.h \
include/aalsdk/kernel/aaltypes.h \
include/aalsdk/kernel/aalui-events.h \
include/aalsdk/kernel/AALWorkspace.h \
include/aalsdk/kernel/aalwsservice.h \
include/aalsdk/kernel/ahmpip.h \
include/aalsdk/kernel/ahmpipdefs.h \
include/aalsdk/kernel/ccidefs.h \
include/aalsdk/kernel/fappip.h \
include/aalsdk/kernel/spl2defs.h \
include/aalsdk/kernel/aalui.h \
include/aalsdk/kernel/ccipdriver.h \
include/aalsdk/kernel/ccip_defs.h \
include/aalsdk/kernel/KernelStructs.h \
include/aalsdk/kernel/kosal.h \
include/aalsdk/kernel/vafu2defs.h

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

