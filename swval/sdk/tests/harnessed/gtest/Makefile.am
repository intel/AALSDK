# INTEL CONFIDENTIAL - For Intel Internal Use Only

if AAL_COND_GTEST

check_LTLIBRARIES=libgtest.la

nodist_libgtest_la_SOURCES=gtest-all.cc
gtest-all.cc:
	@ln -s $(GTEST_SRCDIR)/gtest-all.cc $@

libgtest_la_CPPFLAGS=$(GTEST_CPPFLAGS)
libgtest_la_CFLAGS=$(GTEST_CFLAGS)
libgtest_la_CXXFLAGS=$(GTEST_CXXFLAGS)
libgtest_la_LDFLAGS=$(GTEST_LDFLAGS)

COMMON_CPPFLAGS=\
-I$(top_srcdir)/include \
-I$(top_srcdir)/aas/OSAL \
-I$(top_srcdir)/aas/AALRuntime \
-I$(top_srcdir)/aas/AIAService \
-I$(top_srcdir)/aas/RRMBrokerService \
-I$(top_srcdir)/tests/swvalmod \
-I$(top_builddir)/include $(GTEST_CPPFLAGS)

COMMON_LDADD=\
$(check_LTLIBRARIES) \
$(top_builddir)/aas/OSAL/libOSAL.la \
$(top_builddir)/aas/AASLib/libAAS.la \
$(top_builddir)/aas/AALRuntime/libaalrt.la \
$(top_builddir)/aas/AASResourceManager/libAASResMgr.la

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
gtCommon.cpp \
gtCommon.h \
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
-DTEST_SUITE_BAT=1 $(COMMON_CPPFLAGS)

swtest_LDADD=$(COMMON_LDADD)


# check_PROGRAMS=
# bat 
# nightly 
# weekly

# bat_SOURCES=
# gtAALEvent.cpp 
# gtAALService.cpp 
# gtAAS.cpp 
# gtAASBase.cpp 
# gtAASResMgr.cpp 
# gtAIAService.cpp 
# gtBarrier.cpp 
# gtCommon.cpp 
# gtCommon.h 
# gtCritSect.cpp 
# gtDispatchables.cpp 
# gtDynLinkLibrary.cpp 
# gtEnvVar.cpp 
# gtHWALIAFU.cpp 
# gtMDS.cpp 
# gtOSAL.cpp 
# gtOSServiceModule.cpp 
# gtRRMBrokerService.cpp 
# gtRuntime.cpp 
# gtRuntime_Int.cpp 
# gtSem.cpp 
# gtServiceBroker.cpp 
# gtServiceHost.cpp 
# gtSleep.cpp 
# gtThread.cpp 
# gtThreadGroup.cpp 
# gtThreadGroup.h 
# gtThreadGroupSR.cpp 
# gtTimer.cpp 
# gtTransactionID.cpp 
# main.cpp

# bat_CPPFLAGS=
# -DTEST_SUITE_BAT=1 $(COMMON_CPPFLAGS)

# bat_LDADD=$(COMMON_LDADD)


# nightly_SOURCES=
# gtAASResMgr.cpp 
# gtAIAService.cpp 
# gtCommon.cpp 
# gtCommon.h 
# gtCritSect.cpp 
# gtDynLinkLibrary.cpp 
# gtOSServiceModule.cpp 
# gtRRMBrokerService.cpp 
# gtSem.cpp 
# gtThread.cpp 
# gtThreadGroup.cpp 
# gtThreadGroup.h 
# gtThreadGroupSR.cpp 
# main.cpp

# nightly_CPPFLAGS=
# -DTEST_SUITE_NIGHTLY=1 $(COMMON_CPPFLAGS)

# nightly_LDADD=$(COMMON_LDADD)


# weekly_SOURCES=
# gtAASResMgr.cpp 
# gtAIAService.cpp 
# gtCValue.cpp 
# gtCommon.cpp 
# gtCommon.h 
# gtNVS0.cpp 
# gtNVS1.cpp 
# gtNVS2.cpp 
# gtNVS3.cpp 
# gtNVS4.cpp 
# gtNVS5.cpp 
# gtNVSLegacy.cpp 
# gtNVSTester.cpp 
# gtNVSTester.h 
# gtRRMBrokerService.cpp 
# gtSeqRand.cpp 
# gtSeqRand.h 
# main.cpp

# weekly_CPPFLAGS=
# -DTEST_SUITE_WEEKLY=1 $(COMMON_CPPFLAGS)

# weekly_LDADD=$(COMMON_LDADD)

CLEANFILES=gtest-all.cc

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
gtCommon.cpp \
gtCommon.h \
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

