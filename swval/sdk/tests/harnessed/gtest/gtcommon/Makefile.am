# INTEL CONFIDENTIAL - For Intel Internal Use Only

if AAL_COND_GTEST

check_LTLIBRARIES=\
libgtcommon.la

libgtcommon_la_SOURCES=\
gtCommon.h \
gtCommon_Config.h \
gtCommon_RNG.h \
gtCommon_Stream.h \
gtCommon_Console.h \
gtCommon_Status.h \
gtCommon_Signals.h \
gtCommon_GTest.h \
gtCommon_Mocks.h \
gtCommon_ClassOverrides.h \
gtCommon_DoWorker.h \
gtCommon_ModuleStub.h \
gtCommon_RTAdapter.h \
gtCommon_RTClientAdapter.h \
gtCommon_SMocks.h \
gtCommon_ServiceListener.h \
gtCommon_RTAdapter.cpp \
gtCommon_RTClientAdapter.cpp \
gtCommon_SMocks.cpp \
gtCommon.cpp \
gtCommon_Config.cpp \
gtCommon_RNG.cpp \
gtCommon_Stream.cpp \
gtCommon_Console.cpp \
gtCommon_Status.cpp \
gtCommon_Signals.cpp \
gtCommon_GTest.cpp \
gtCommon_Mocks.cpp 

libgtcommon_la_CPPFLAGS=\
-I$(top_srcdir)/include \
-I$(top_srcdir)/tests/swvalmod \
-I$(top_builddir)/include $(GTEST_CPPFLAGS)

libgtcommon_la_LDFLAGS=\
-avoid-version

libgtcommon_la_LIBADD=\
$(top_builddir)/tests/harnessed/gtest/libgtest.la

endif

