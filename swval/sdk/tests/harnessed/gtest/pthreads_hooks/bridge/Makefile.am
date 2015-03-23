# INTEL CONFIDENTIAL - For Intel Internal Use Only
noinst_LTLIBRARIES=libapphooksbridge.la
libapphooksbridge_la_SOURCES=\
hooksbridge.h \
hooksbridge.c
libapphooksbridge_la_CPPFLAGS=\
-I$(srcdir)/../hooks
libapphooksbridge_la_LDFLAGS=\
-avoid-version \
-Wl,-flat_namespace,-undefined,dynamic_lookup
