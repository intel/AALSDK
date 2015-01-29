## Copyright (c) 2005-2015, Intel Corporation
##
## Redistribution  and  use  in source  and  binary  forms,  with  or  without
## modification, are permitted provided that the following conditions are met:
##
## * Redistributions of  source code  must retain the  above copyright notice,
##   this list of conditions and the following disclaimer.
## * Redistributions in binary form must reproduce the above copyright notice,
##   this list of conditions and the following disclaimer in the documentation
##   and/or other materials provided with the distribution.
## * Neither the name  of Intel Corporation  nor the names of its contributors
##   may be used to  endorse or promote  products derived  from this  software
##   without specific prior written permission.
##
## THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
## AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  BUT NOT LIMITED TO,  THE
## IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
## ARE DISCLAIMED.  IN NO EVENT  SHALL THE COPYRIGHT OWNER  OR CONTRIBUTORS BE
## LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR
## CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT LIMITED  TO,  PROCUREMENT  OF
## SUBSTITUTE GOODS OR SERVICES;  LOSS OF USE,  DATA, OR PROFITS;  OR BUSINESS
## INTERRUPTION)  HOWEVER CAUSED  AND ON ANY THEORY  OF LIABILITY,  WHETHER IN
## CONTRACT,  STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE  OR OTHERWISE)
## ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,  EVEN IF ADVISED OF THE
## POSSIBILITY OF SUCH DAMAGE.
##****************************************************************************
##     Intel(R) Accelerator Abstraction Layer Library Software Developer
##        Kit (SDK)
##  Content:
##     Intel(R) Accelerator Abstraction Layer Library
##     AALSDK/Makefile.am
##  Author:
##     Tim Whisonant, Intel Corporation
##  History:
##     06/26/2013    TSW   Initial version
##****************************************************************************
ACLOCAL_AMFLAGS=-I m4

dist-file-list: dist-all
	@tar ztf "$(top_builddir)/$(distdir).tar.gz" | sed -e '/\/$$/d' -e 's/$(distdir)/\./' >"$(top_builddir)/dist-file-list"
	@$(RM) $(DIST_ARCHIVES)


if AAL_COND_DOXYGEN

if AAL_COND_DOXYGEN_HTML
DOXYGEN_HTML_CLEANFILES="$(DOX_OUTPUT_DIRECTORY)/html"
endif

if AAL_COND_DOXYGEN_LATEX
DOXYGEN_LATEX_CLEANFILES="$(DOX_OUTPUT_DIRECTORY)/latex"
endif

$(DOX_GENERATE_TAGFILE): $(DOX_DOXYFILE)
	$(RM) -r "$(DOX_OUTPUT_DIRECTORY)"
	$(DOXYGEN) "$<"

DOXYGEN_RUN=doxygen-run
doxygen-run: $(DOX_GENERATE_TAGFILE)

DOXYGEN_DOC=doxygen-doc
doxygen-doc: doxygen-run $(DOXYGEN_PDF)

DOXYGEN_CLEAN=doxygen-clean
doxygen-clean:
	@$(RM) "$(DOX_GENERATE_TAGFILE)" $(DOXYGEN_PDF_CLEANFILES)
	@$(RM) -r $(DOXYGEN_HTML_CLEANFILES) $(DOXYGEN_LATEX_CLEANFILES)

endif
.PHONY: $(DOXYGEN_DOC) $(DOXYGEN_RUN) $(DOXYGEN_PDF) $(DOXYGEN_CLEAN)
.INTERMEDIATE: $(DOXYGEN_RUN) $(DOXYGEN_PDF)

.PHONY: doc docs docclean docsclean
doc docs: $(DOXYGEN_DOC)
docclean docsclean: $(DOXYGEN_CLEAN)


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


clean-local: $(DOXYGEN_CLEAN)
	@$(RM) $(top_builddir)/dist-file-list


SUBDIRS=\
aas/OSAL \
aas/AASLib \
aas/XLRTLib \
aas/XLRRMBrokerService \
aas/AASRegistrar \
aas/uAIA \
aas/AASResourceManager \
ase/sw \
clp \
utils/aalscan \
utils/data_model \
utils/fpgasane \
utils/CCIAFU \
utils/SPLAFU


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
include/aalsdk/AALCNamedValueSet.h \
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
include/aalsdk/AASEDS.h \
include/aalsdk/AASystem.h \
include/aalsdk/AASLib.h \
include/aalsdk/AFU.h \
include/aalsdk/AFU_ID.h \
include/aalsdk/AFUpkg.h \
include/aalsdk/CAALBase.h \
include/aalsdk/CAALEvent.h \
include/aalsdk/CAALLogger.h \
include/aalsdk/CCountedObject.h \
include/aalsdk/INTCDefs.h \
include/aalsdk/OSAL.h \
include/aalsdk/Registrar.h \
include/aalsdk/ResMgr.h \
include/aalsdk/uAIALib.h \
include/aalsdk/xlrtlib.h \
include/aalsdk/xlRuntime.h \
include/aalsdk/CUnCopyable.h \
include/aalsdk/IServiceClient.h \
include/aalsdk/IMessageHandler.h 

aashdrs_HEADERS=\
include/aalsdk/aas/AALDeviceService.h \
include/aalsdk/aas/AALInProcServiceFactory.h \
include/aalsdk/aas/AALIPCServiceFactory.h \
include/aalsdk/aas/AALService.h \
include/aalsdk/aas/AALServiceModule.h \
include/aalsdk/aas/AASService.h \
include/aalsdk/aas/IServiceBroker.h \
include/aalsdk/aas/ServiceHost.h \
include/aalsdk/aas/_xlRuntimeServices.h \
include/aalsdk/aas/XLRuntimeMessages.h

aalclphdrs_HEADERS=\
include/aalsdk/aalclp/aalclp.h

asehdrs_HEADERS=\
ase/sw/ase_common.h

edshdrs_HEADERS=\
include/aalsdk/eds/AASEDSService.h \
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
include/aalsdk/kernel/NLBVAFU.h \
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
include/aalsdk/rm/XLResourceManager.h \
include/aalsdk/rm/XLResourceManagerClient.h \
include/aalsdk/rm/XLResourceManagerProxy.h \
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
include/aalsdk/service/SWSimSPLAFUService.h

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
include/aalsdk/uaia/uidrvMessage.h

utilshdrs_HEADERS=\
include/aalsdk/utils/AALEventUtilities.h \
include/aalsdk/utils/AALWorkSpaceUtilities.h \
include/aalsdk/utils/CSyncClient.h \
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

AASEDS_EXTRA=\
include/aalsdk/eds/AASEDSService.h.in

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
ase/sw/ase_ops.c \
ase/ase.cfg \
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
ase/hw/cci_rule_checker.sv \
ase/scripts/run_ase_automated.py \
ase/scripts/gen_afu_env.py \
ase/scripts/vcs_icc.make \
ase/scripts/ase_setup.csh \
ase/scripts/ase_functions.py \
ase/scripts/ase_ipc_clean.py \
ase/README

CLP_EXTRA=\
include/aalsdk/aalclp/aalclp.h.in

DOC_EXTRA=\
doc/Doxyfile.in \
doc/DoxygenLayout.xml \
doc/intel_rgb_62.png

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
include/aalsdk/service/SWSimSPLAFUService.h.in

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
$(DOC_EXTRA) \
$(FAPTRANS_EXTRA) \
$(OSAL_EXTRA) \
$(SERVICE_EXTRA) \
$(UAIA_EXTRA)

