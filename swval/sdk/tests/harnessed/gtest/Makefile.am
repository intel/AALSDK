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
-I$(top_srcdir)/tests/swvalmod \
-I$(top_builddir)/include $(GTEST_CPPFLAGS)

COMMON_LDADD=\
$(check_LTLIBRARIES) \
$(top_builddir)/aas/OSAL/libOSAL.la \
$(top_builddir)/aas/AASLib/libAAS.la \
$(top_builddir)/aas/AALRuntime/libaalrt.la

check_PROGRAMS=\
bat \
nightly \
weekly

bat_SOURCES=\
main.cpp \
gtCommon.h \
gtCommon.cpp \
gtCritSect.cpp \
gtSem.cpp \
gtBarrier.cpp \
gtThread.cpp \
gtThreadGroup.h \
gtThreadGroup.cpp \
gtThreadGroupSR.cpp \
gtOSServiceModule.cpp \
gtDynLinkLibrary.cpp \
gtEnvVar.cpp \
gtTimer.cpp \
gtSleep.cpp \
gtAASBase.cpp

bat_CPPFLAGS=\
-DTEST_SUITE_BAT=1 $(COMMON_CPPFLAGS)

bat_LDADD=$(COMMON_LDADD)


nightly_SOURCES=\
main.cpp \
gtCommon.h \
gtCommon.cpp \
gtCritSect.cpp \
gtSem.cpp \
gtThread.cpp \
gtThreadGroup.h \
gtThreadGroup.cpp \
gtThreadGroupSR.cpp \
gtOSServiceModule.cpp \
gtDynLinkLibrary.cpp



nightly_CPPFLAGS=\
-DTEST_SUITE_NIGHTLY=1 $(COMMON_CPPFLAGS)

nightly_LDADD=$(COMMON_LDADD)


weekly_SOURCES=\
main.cpp \
gtCommon.h \
gtCommon.cpp \
gtNVS.cpp

weekly_CPPFLAGS=\
-DTEST_SUITE_WEEKLY=1 $(COMMON_CPPFLAGS)

weekly_LDADD=$(COMMON_LDADD)

CLEANFILES=gtest-all.cc

else

EXTRA_DIST=\
main.cpp \
gtCommon.h \
gtCommon.cpp \
gtCritSect.cpp \
gtSem.cpp \
gtNVS.cpp \
gtThread.cpp \
gtThreadGroup.h \
gtThreadGroup.cpp \
gtThreadGroupSR.cpp \
gtPaths.cpp \
gtStartupShutdown.cpp \
gtEnvVar.cpp \
gtOSServiceModule.cpp \
gtDynLinkLibrary.cpp \
gtEnvVar.cpp \
gtTimer.cpp \
gtSleep.cpp

endif

