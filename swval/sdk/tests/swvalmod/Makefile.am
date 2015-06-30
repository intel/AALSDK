# INTEL CONFIDENTIAL - For Intel Internal Use Only
lib_LTLIBRARIES=\
libswvalmod.la \
libswvalsvcmod.la


libswvalmod_la_SOURCES=\
swvalmod.h \
swvalmod.c

libswvalmod_la_CPPFLAGS=\
-I$(top_srcdir)/include \
-I$(top_builddir)/include

libswvalmod_la_LDFLAGS=\
-module \
-version-info $(SWVALMOD_VERSION_CURRENT):$(SWVALMOD_VERSION_REVISION):$(SWVALMOD_VERSION_AGE)

libswvalmod_la_LIBADD=\
$(top_builddir)/aas/OSAL/libOSAL.la \
$(top_builddir)/aas/AASLib/libAAS.la \
$(top_builddir)/aas/AALRuntime/libaalrt.la


libswvalsvcmod_la_SOURCES=\
swvalsvcmod.h \
swvalsvcmod.cpp

libswvalsvcmod_la_CPPFLAGS=\
-I$(top_srcdir)/include \
-I$(top_builddir)/include

libswvalsvcmod_la_LDFLAGS=\
-module \
-version-info $(SWVALSVCMOD_VERSION_CURRENT):$(SWVALSVCMOD_VERSION_REVISION):$(SWVALSVCMOD_VERSION_AGE)

libswvalsvcmod_la_LIBADD=\
$(top_builddir)/aas/OSAL/libOSAL.la \
$(top_builddir)/aas/AASLib/libAAS.la \
$(top_builddir)/aas/AALRuntime/libaalrt.la

