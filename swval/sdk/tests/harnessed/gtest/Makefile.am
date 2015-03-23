# INTEL CONFIDENTIAL - For Intel Internal Use Only

SUBDIRS=\
pthreads_hooks

gtest_cases=\
cases/gtCommon.cpp \
cases/gtCritSect.cpp \
cases/gtSem.cpp \
cases/gtNVS.cpp \
cases/gtThread.cpp \
cases/gtThreadGroup.cpp \
cases/gtPaths.cpp \
cases/gtStartupShutdown.cpp \
cases/gtEnvVar.cpp

EXTRA_DIST=$(gtest_cases)

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
-I$(srcdir)/pthreads_hooks/bridge \
-I$(srcdir)/pthreads_hooks/hooks \
-I$(top_builddir)/include $(GTEST_CPPFLAGS)

COMMON_LDADD=\
$(check_LTLIBRARIES) \
$(top_builddir)/aas/OSAL/libOSAL.la \
$(top_builddir)/aas/AASLib/libAAS.la \
$(top_builddir)/aas/XLRTLib/libxlrt.la \
$(builddir)/pthreads_hooks/bridge/libapphooksbridge.la

check_PROGRAMS=\
bat \
nightly \
weekly

bat_SOURCES=\
main.cpp \
BAT-TEST-LIST.h

bat_CPPFLAGS=\
-DTEST_SUITE_BAT=1 $(COMMON_CPPFLAGS)

bat_LDADD=$(COMMON_LDADD)

$(bat_SOURCES): $(gtest_cases)


nightly_SOURCES=\
main.cpp \
NIGHTLY-TEST-LIST.h

nightly_CPPFLAGS=\
-DTEST_SUITE_NIGHTLY=1 $(COMMON_CPPFLAGS)

nightly_LDADD=$(COMMON_LDADD)

$(nightly_SOURCES): $(gtest_cases)


weekly_SOURCES=\
main.cpp \
WEEKLY-TEST-LIST.h

weekly_CPPFLAGS=\
-DTEST_SUITE_WEEKLY=1 $(COMMON_CPPFLAGS)

weekly_LDADD=$(COMMON_LDADD)

$(weekly_SOURCES): $(gtest_cases)

CLEANFILES=gtest-all.cc

else

EXTRA_DIST+=\
main.cpp \
BAT-TEST-LIST.h \
NIGHTLY-TEST-LIST.h \
WEEKLY-TEST-LIST.h

endif

