# INTEL CONFIDENTIAL - For Intel Internal Use Only

if AAL_COND_GTEST

check_PROGRAMS=\
swtest

swtest_SOURCES=\
gtAALEvent.cpp \
gtAALService.cpp \
gtAAS.cpp \
gtAASBase.cpp \
gtAASResMgr.cpp \
gtAIAService.cpp \
gtBarrier.cpp \
gtCValue.cpp \
gtCritSect.cpp \
gtDispatchables.cpp \
gtDynLinkLibrary.cpp \
gtEnvVar.cpp \
gtHWALIAFU.cpp \
gtMDS.cpp \
gtNVS0.cpp \
gtNVS1.cpp \
gtNVS2.cpp \
gtNVS3.cpp \
gtNVS4.cpp \
gtNVS5.cpp \
gtNVSLegacy.cpp \
gtNVSTester.cpp \
gtNVSTester.h \
gtOSAL.cpp \
gtOSServiceModule.cpp \
gtRRMBrokerService.cpp \
gtRuntime.cpp \
gtRuntime_Int.cpp \
gtSem.cpp \
gtSeqRand.cpp \
gtSeqRand.h \
gtServiceBroker.cpp \
gtServiceHost.cpp \
gtSleep.cpp \
gtThread.cpp \
gtThreadGroup.cpp \
gtThreadGroup.h \
gtThreadGroupSR.cpp \
gtTimer.cpp \
gtTransactionID.cpp \
main.cpp

swtest_CPPFLAGS=\
-DTEST_SUITE_BAT=1 \
-I$(top_srcdir)/include \
-I$(top_srcdir)/aas/OSAL \
-I$(top_srcdir)/aas/AALRuntime \
-I$(top_srcdir)/aas/AIAService \
-I$(top_srcdir)/aas/RRMBrokerService \
-I$(top_srcdir)/tests/harnessed/gtest/gtcommon \
-I$(top_srcdir)/tests/swvalmod \
-I$(top_builddir)/include $(GTEST_CPPFLAGS)

swtest_LDADD=\
$(top_builddir)/tests/harnessed/gtest/libgtest.la \
$(top_builddir)/tests/harnessed/gtest/gtcommon/libgtcommon.la \
$(top_builddir)/aas/OSAL/libOSAL.la \
$(top_builddir)/aas/AASLib/libAAS.la \
$(top_builddir)/aas/AALRuntime/libaalrt.la \
$(top_builddir)/aas/AASResourceManager/libAASResMgr.la

else

EXTRA_DIST=\
gtAALEvent.cpp \
gtAALService.cpp \
gtAAS.cpp \
gtAASBase.cpp \
gtAASResMgr.cpp \
gtAIAService.cpp \
gtBarrier.cpp \
gtCValue.cpp \
gtCritSect.cpp \
gtDispatchables.cpp \
gtDynLinkLibrary.cpp \
gtEnvVar.cpp \
gtHWALIAFU.cpp \
gtMDS.cpp \
gtNVS0.cpp \
gtNVS1.cpp \
gtNVS2.cpp \
gtNVS3.cpp \
gtNVS4.cpp \
gtNVS5.cpp \
gtNVSLegacy.cpp \
gtNVSTester.cpp \
gtNVSTester.h \
gtOSAL.cpp \
gtOSServiceModule.cpp \
gtRRMBrokerService.cpp \
gtRuntime.cpp \
gtRuntime_Int.cpp \
gtSem.cpp \
gtSeqRand.cpp \
gtSeqRand.h \
gtServiceBroker.cpp \
gtServiceHost.cpp \
gtSleep.cpp \
gtThread.cpp \
gtThreadGroup.cpp \
gtThreadGroup.h \
gtThreadGroupSR.cpp \
gtTimer.cpp \
gtTransactionID.cpp \
main.cpp

endif

