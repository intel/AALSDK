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
aas/RRMBrokerService \
aas/AASRegistrar \
aas/uAIA \
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
faptranshdrsdir=$(aalsdkhdrsdir)/faptrans
kernelhdrsdir=$(aalsdkhdrsdir)/kernel
encoderhdrsdir=$(kernelhdrsdir)/encoder_sample_kbae
osalhdrsdir=$(aalsdkhdrsdir)/osal
registrarhdrsdir=$(aalsdkhdrsdir)/registrar
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
include/aalsdk/Registrar.h \
include/aalsdk/ResMgr.h \
include/aalsdk/uAIALib.h \
include/aalsdk/aas/Dispatchables.h \
include/aalsdk/Runtime.h \
include/aalsdk/CUnCopyable.h \
include/aalsdk/IServiceClient.h \
include/aalsdk/IResConfig.h

aashdrs_HEADERS=\
include/aalsdk/aas/AALDeviceService.h \
include/aalsdk/aas/AALInProcServiceFactory.h \
include/aalsdk/aas/AALIPCServiceFactory.h \
include/aalsdk/aas/AALService.h \
include/aalsdk/aas/AALServiceModule.h \
include/aalsdk/aas/AASService.h \
include/aalsdk/aas/IServiceBroker.h \
include/aalsdk/aas/ServiceHost.h 

aalclphdrs_HEADERS=\
include/aalsdk/aalclp/aalclp.h

asehdrs_HEADERS=\
ase/sw/ase_common.h

edshdrs_HEADERS=\
include/aalsdk/eds/AASEventDeliveryService.h 

faptranshdrs_HEADERS=\
include/aalsdk/faptrans/FAP10.h \
include/aalsdk/faptrans/FAP10Service.h \
include/aalsdk/faptrans/FAP20.h \
include/aalsdk/faptrans/FAP20Service.h \
include/aalsdk/faptrans/MAFUTransactions.h \
include/aalsdk/faptrans/MAFUTransService.h

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

registrarhdrs_HEADERS=\
include/aalsdk/registrar/AASRegistrar.h \
include/aalsdk/registrar/AASRegistrarService.h \
include/aalsdk/registrar/CAASRegistrar.h \
include/aalsdk/registrar/RegDBProxy.h \
include/aalsdk/registrar/RegDBStub.h

rmhdrs_HEADERS=\
include/aalsdk/rm/AALResourceManager.h \
include/aalsdk/rm/AALResourceManagerClient.h \
include/aalsdk/rm/AALResourceManagerProxy.h \
include/aalsdk/rm/CAASResourceManager.h \
include/aalsdk/rm/CRegistrarDatabase.h \
include/aalsdk/rm/InstanceRecord.h \
include/aalsdk/rm/RegDBSkeleton.h

servicehdrs_HEADERS=\
include/aalsdk/service/ICCIAFU.h \
include/aalsdk/service/ICCIClient.h \
include/aalsdk/service/ISPLAFU.h \
include/aalsdk/service/ISPLClient.h \
include/aalsdk/service/CCIAFUService.h \
include/aalsdk/service/HWCCIAFUService.h \
include/aalsdk/service/ASECCIAFUService.h \
include/aalsdk/service/SWSimCCIAFUService.h \
include/aalsdk/service/SPLAFUService.h \
include/aalsdk/service/HWSPLAFUService.h \
include/aalsdk/service/ASESPLAFUService.h \
include/aalsdk/service/SWSimSPLAFUService.h \
include/aalsdk/service/IALIAFU.h \
include/aalsdk/service/ALIAFUService.h \
include/aalsdk/service/HWALIAFUService.h \
include/aalsdk/service/ASEALIAFUService.h \
include/aalsdk/service/SWSimALIAFUService.h

uaiahdrs_HEADERS=\
include/aalsdk/uaia/AALuAIA_Messaging.h \
include/aalsdk/uaia/AALuAIA_UIDriverClient.h \
include/aalsdk/uaia/AIA.h \
include/aalsdk/uaia/FAPPIP_AFUdev.h \
include/aalsdk/uaia/IUAIASession.h \
include/aalsdk/uaia/uAIADefs.h \
include/aalsdk/uaia/uAIA.h \
include/aalsdk/uaia/uAIAService.h \
include/aalsdk/uaia/uAIASession.h \
include/aalsdk/uaia/uidrvMessage.h \
include/aalsdk/uaia/IAFUProxy.h

utilshdrs_HEADERS=\
include/aalsdk/utils/AALEventUtilities.h \
include/aalsdk/utils/AALWorkSpaceUtilities.h \
include/aalsdk/utils/CSyncClient.h \
include/aalsdk/utils/NLBVAFU.h \
include/aalsdk/utils/ResMgrUtilities.h \
include/aalsdk/utils/SingleAFUApp.h \
include/aalsdk/utils/Utilities.h


regrepodir=$(datadir)/aalsdk/RegistrarRepository
regrepolinuxdir=$(regrepodir)/linux

regrepo_DATA=\
RegistrarRepository/AASRegistrarDatabase.linux \
RegistrarRepository/AASRegistrarDatabase.linux.backup \
RegistrarRepository/AASRegistrarDatabase.win \
RegistrarRepository/AASRegistrarDatabase.win.backup \
RegistrarRepository/GenDB.linux \
RegistrarRepository/GenDB.win

regrepolinux_DATA=\
RegistrarRepository/linux/1.nvs \
RegistrarRepository/linux/2.nvs \
RegistrarRepository/linux/3.nvs \
RegistrarRepository/linux/4.nvs \
RegistrarRepository/linux/5.nvs


AAS_EXTRA=\
include/aalsdk/aas/AASService.h.in

AASREGISTRAR_EXTRA=\
include/aalsdk/registrar/AASRegistrarService.h.in

ASE_EXTRA=\
ase/sw/mqueue_ops.c \
ase/sw/mem_model.c \
ase/sw/ipc_mgmt_ops.c \
ase/sw/shm_ops.c \
ase/sw/tstamp_ops.c \
ase/sw/error_report.c \
ase/sw/linked_list_ops.c \
ase/sw/capcm_ops.c \
ase/sw/ase_common.h \
ase/sw/protocol_backend.c \
ase/sw/randomness_control.c \
ase/sw/ase_ops.c \
ase/ase.cfg \
ase/ase_regress.sh \
ase/Makefile \
ase/distclean.sh \
ase/hw/counter.sv \
ase/hw/platform.vh \
ase/hw/ase_fifo.sv \
ase/hw/latency_pipe.sv \
ase/hw/sdp_ram.sv \
ase/hw/cci_emulator.sv \
ase/hw/latency_scoreboard.sv \
ase/hw/ase_global.vh \
ase/hw/cci_sniffer.sv \
ase/hw/stream_checker.sv \
ase/scripts/generate_ase_environment.py \
ase/scripts/vcs_icc.make \
ase/scripts/ase_setup.csh \
ase/scripts/ase_functions.py \
ase/scripts/ipc_clean.py \
ase/README

CLP_EXTRA=\
include/aalsdk/aalclp/aalclp.h.in

FAPTRANS_EXTRA=\
include/aalsdk/faptrans/FAP10Service.h.in \
include/aalsdk/faptrans/FAP20Service.h.in \
include/aalsdk/faptrans/MAFUTransService.h.in

OSAL_EXTRA=\
include/aalsdk/osal/OSALService.h.in

SERVICE_EXTRA=\
include/aalsdk/service/CCIAFUService.h.in \
include/aalsdk/service/HWCCIAFUService.h.in \
include/aalsdk/service/ASECCIAFUService.h.in \
include/aalsdk/service/SWSimCCIAFUService.h.in \
include/aalsdk/service/SPLAFUService.h.in \
include/aalsdk/service/HWSPLAFUService.h.in \
include/aalsdk/service/ASESPLAFUService.h.in \
include/aalsdk/service/SWSimSPLAFUService.h.in \
include/aalsdk/service/ALIAFUService.h.in \
include/aalsdk/service/HWALIAFUService.h.in \
include/aalsdk/service/ASEALIAFUService.h.in \
include/aalsdk/service/SWSimALIAFUService.h.in

UAIA_EXTRA=\
include/aalsdk/uaia/uAIAService.h.in

EXTRA_DIST=\
$(regrepo_DATA) \
$(regrepolinux_DATA) \
$(AAS_EXTRA) \
$(AASEDS_EXTRA) \
$(AASREGISTRAR_EXTRA) \
$(ASE_EXTRA) \
$(CLP_EXTRA) \
$(FAPTRANS_EXTRA) \
$(OSAL_EXTRA) \
$(SERVICE_EXTRA) \
$(UAIA_EXTRA)

