# INTEL CONFIDENTIAL - For Intel Internal Use Only

if AAL_COND_GTEST

check_PROGRAMS=\
nlb0test

nlb0test_SOURCES=\
main.cpp

nlb0test_CPPFLAGS=\
-I$(top_srcdir)/include \
-I$(top_srcdir)/tests/harnessed/gtest/gtcommon \
-I$(top_srcdir)/tests/swvalmod \
-I$(top_builddir)/include $(GTEST_CPPFLAGS)

nlb0test_LDADD=\
$(top_builddir)/tests/harnessed/gtest/libgtest.la \
$(top_builddir)/tests/harnessed/gtest/gtcommon/libgtcommon.la \
$(top_builddir)/aas/OSAL/libOSAL.la \
$(top_builddir)/aas/AASLib/libAAS.la \
$(top_builddir)/aas/AALRuntime/libaalrt.la

else

EXTRA_DIST=\
main.cpp

endif

