# INTEL CONFIDENTIAL - For Intel Internal Use Only
ACLOCAL_AMFLAGS=-I m4

dist-file-list: dist-all
	@tar ztf "$(top_builddir)/$(distdir).tar.gz" | sed -e '/\/$$/d' -e 's/$(distdir)/\./' >"$(top_builddir)/dist-file-list"
	@$(RM) $(DIST_ARCHIVES)


clean-local:
	@$(RM) $(top_builddir)/dist-file-list


SUBDIRS=\
aas/OSAL \
aas/AASLib \
aas/AALRuntime \
aas/AIAService \
aas/RRMBrokerService \
aas/AASResourceManager \
ase/sw \
clp \
utils \
tests

aalsdkhdrsdir=$(includedir)/aalsdk
aashdrsdir=$(aalsdkhdrsdir)/aas
aalclphdrsdir=$(aalsdkhdrsdir)/aalclp
asehdrsdir=$(aalsdkhdrsdir)/ase
ccilibhdrsdir=$(aalsdkhdrsdir)/ccilib
edshdrsdir=$(aalsdkhdrsdir)/eds
kernelhdrsdir=$(aalsdkhdrsdir)/kernel
encoderhdrsdir=$(kernelhdrsdir)/encoder_sample_kbae
osalhdrsdir=$(aalsdkhdrsdir)/osal
rmhdrsdir=$(aalsdkhdrsdir)/rm
servicehdrsdir=$(aalsdkhdrsdir)/service
uaiahdrsdir=$(aalsdkhdrsdir)/uaia
utilshdrsdir=$(aalsdkhdrsdir)/utils

aalsdkhdrs_HEADERS=\
include/aalsdk/AALBase.h \
include/aalsdk/_AALDefs.h \
include/aalsdk/AALDefs.h \
include/aalsdk/AALEvent.h \
include/aalsdk/AAL.h \
include/aalsdk/AALIDDefs.h \
include/aalsdk/AALLoggerExtern.h \
include/aalsdk/AALLogger.h \
include/aalsdk/AALMAFU.h \
include/aalsdk/AALNamedValueSet.h \
include/aalsdk/AALNVSMarshaller.h \
include/aalsdk/AALTransactionID.h \
include/aalsdk/_AALTypes.h \
include/aalsdk/AALTypes.h \
include/aalsdk/AASystem.h \
include/aalsdk/AASLib.h \
include/aalsdk/AFU.h \
include/aalsdk/AFU_ID.h \
include/aalsdk/AFUpkg.h \
include/aalsdk/CAALBase.h \
include/aalsdk/CAALEvent.h \
include/aalsdk/CAALLogger.h \
include/aalsdk/CCountedObject.h \
include/aalsdk/INamedValueSet.h \
include/aalsdk/INTCDefs.h \
include/aalsdk/OSAL.h \
include/aalsdk/ResMgr.h \
include/aalsdk/uAIALib.h \
include/aalsdk/Runtime.h \
include/aalsdk/CUnCopyable.h \
include/aalsdk/IServiceClient.h \
include/aalsdk/IResConfig.h

aashdrs_HEADERS=\
include/aalsdk/aas/AALInProcServiceFactory.h \
include/aalsdk/aas/AALIPCServiceFactory.h \
include/aalsdk/aas/AALService.h \
include/aalsdk/aas/AALServiceModule.h \
include/aalsdk/aas/AASService.h \
include/aalsdk/aas/IServiceBroker.h \
include/aalsdk/aas/IServiceRevoke.h \
include/aalsdk/aas/ServiceHost.h \
include/aalsdk/aas/Dispatchables.h 

aalclphdrs_HEADERS=\
include/aalsdk/aalclp/aalclp.h

asehdrs_HEADERS=\
ase/sw/ase_common.h

edshdrs_HEADERS=\
include/aalsdk/eds/AASEventDeliveryService.h 

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
include/aalsdk/kernel/KernelStructs.h \
include/aalsdk/kernel/kosal.h \
include/aalsdk/kernel/vafu2defs.h


osalhdrs_HEADERS=\
include/aalsdk/osal/CriticalSection.h \
include/aalsdk/osal/DynLinkLibrary.h \
include/aalsdk/osal/Env.h \
include/aalsdk/osal/OSALService.h \
include/aalsdk/osal/OSSemaphore.h \
include/aalsdk/osal/Barrier.h \
include/aalsdk/osal/OSServiceModule.h \
include/aalsdk/osal/Sleep.h \
include/aalsdk/osal/ThreadGroup.h \
include/aalsdk/osal/Thread.h \
include/aalsdk/osal/Timer.h \
include/aalsdk/osal/IDispatchable.h

rmhdrs_HEADERS=\
include/aalsdk/rm/AALResourceManager.h \
include/aalsdk/rm/AALResourceManagerClient.h \
include/aalsdk/rm/AALResourceManagerProxy.h \
include/aalsdk/rm/CAASResourceManager.h \
include/aalsdk/rm/InstanceRecord.h 

servicehdrs_HEADERS=\
include/aalsdk/service/IALIAFU.h \
include/aalsdk/service/ALIAFUService.h \
include/aalsdk/service/HWALIAFUService.h \
include/aalsdk/service/ASEALIAFUService.h \
include/aalsdk/service/SWSimALIAFUService.h

uaiahdrs_HEADERS=\
include/aalsdk/uaia/AIA.h \
include/aalsdk/uaia/IAFUProxy.h

utilshdrs_HEADERS=\
include/aalsdk/utils/AALEventUtilities.h \
include/aalsdk/utils/AALWorkSpaceUtilities.h \
include/aalsdk/utils/CSyncClient.h \
include/aalsdk/utils/NLBVAFU.h \
include/aalsdk/utils/ResMgrUtilities.h \
include/aalsdk/utils/SingleAFUApp.h \
include/aalsdk/utils/Utilities.h

AAS_EXTRA=\
include/aalsdk/aas/AASService.h.in

ASE_EXTRA=\
ase/sw/ase_ops.c \
ase/sw/Makefile \
ase/sw/ipc_mgmt_ops.c \
ase/sw/mem_model.c \
ase/sw/protocol_backend.c \
ase/sw/ase_common.h \
ase/sw/tstamp_ops.c \
ase/sw/mqueue_ops.c \
ase/sw/app_backend.c \
ase/sw/error_report.c \
ase/sw/linked_list_ops.c \
ase/sw/randomness_control.c \
ase/Makefile \
ase/README \
ase/ase.cfg \
ase/distclean.sh \
ase/LICENSE.txt \
ase/ase_regress.sh \
ase/hw/outoforder_wrf_channel.sv \
ase/hw/counter.sv \
ase/hw/ccip_if_pkg.sv \
ase/hw/ase_global.vh \
ase/hw/ase_fifo.sv \
ase/hw/latency_pipe.sv \
ase/hw/ccip_emulator.sv \
ase/hw/ccip_logger.sv \
ase/hw/ccip_sniffer.sv \
ase/hw/ase_top.sv \
ase/hw/sdp_ram.sv \
ase/hw/stream_checker.sv \
ase/hw/ase_pkg.sv \
ase/hw/platform.vh \
ase/scripts/generate_ase_environment.py \
ase/scripts/vcs_icc.make \
ase/scripts/ipc_clean.py \
ase/scripts/ase_functions.py \
ase/scripts/ase_setup.csh

CLP_EXTRA=\
include/aalsdk/aalclp/aalclp.h.in

OSAL_EXTRA=\
include/aalsdk/osal/OSALService.h.in

RM_EXTRA=\
include/aalsdk/rm/ResMgrService.h.in \
include/aalsdk/rm/RRMService.h.in

SERVICE_EXTRA=\
include/aalsdk/service/ALIAFUService.h.in \
include/aalsdk/service/HWALIAFUService.h.in \
include/aalsdk/service/ASEALIAFUService.h.in \
include/aalsdk/service/SWSimALIAFUService.h.in

UAIA_EXTRA=\
include/aalsdk/uaia/AIAService.h.in 

EXTRA_DIST=\
$(AAS_EXTRA) \
$(AASEDS_EXTRA) \
$(ASE_EXTRA) \
$(CLP_EXTRA) \
$(OSAL_EXTRA) \
$(RM_EXTRA) \
$(SERVICE_EXTRA) \
$(UAIA_EXTRA)

