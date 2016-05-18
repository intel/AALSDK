# INTEL CONFIDENTIAL - For Intel Internal Use Only

if AAL_COND_GTEST

check_LTLIBRARIES=\
libgtcommon.la

libgtcommon_la_SOURCES=\
gtCommon.h \
gtCommon.cpp

libgtcommon_la_CPPFLAGS=\
-I$(top_srcdir)/include \
-I$(top_srcdir)/tests/swvalmod \
-I$(top_builddir)/include $(GTEST_CPPFLAGS)

libgtcommon_la_LDFLAGS=\
-avoid-version

libgtcommon_la_LIBADD=\
$(top_builddir)/tests/harnessed/gtest/libgtest.la

endif

