# INTEL CONFIDENTIAL - for Intel Internal Use Only
# Makefile for nightly regression tests

test_cases=\
tests/some.cpp

SDK_SRC      ?= $(shell cd ../.. && pwd)
SDK_BUILD    ?= $(SDK_SRC)/mybuild

LIBTOOL      ?= libtool
CXX          ?= g++

lib_RPATH    ?= $(shell pwd)

DIRS          = .deps .libs

GTEST_DIR     = $(SDK_SRC)/gtest
GTEST_DISTDIR = gtest-$(shell cat $(GTEST_DIR)/CURRENT)
GTEST_TARBALL = $(GTEST_DISTDIR).tar.gz
GTEST_LINK    = gtest


APP_EXE=nightly
APP_SRC=nightly.cpp
APP_OBJ=nightly.o
APP_PO=nightly.Po

APP_CPPFLAGS=\
-DENABLE_DEBUG=1 \
-DENABLE_ASSERT=1 \
-D__AAL_USER__=1 \
-I$(SDK_SRC)/include \
-I$(SDK_SRC)/aalkernel/include \
-I$(SDK_BUILD)/include \
-DOSAL_LIBDIR=\"$(SDK_BUILD)/aas/OSAL/.libs\" \
-DAAS_LIBDIR=\"$(SDK_BUILD)/aas/AASLib/.libs\" \
-DXLRT_LIBDIR=\"$(SDK_BUILD)/aas/XLRTLib/.libs\" \
-DXLRRMBROKER_LIBDIR=\"$(SDK_BUILD)/aas/XLRRMBrokerService/.libs\" \
-DUAIA_LIBDIR=\"$(SDK_BUILD)/aas/uAIA/.libs\" \
-DSAMPLE_AFU1_LIBDIR=\"$(SDK_BUILD)/samples/AFU1/.libs\" \
-I$(GTEST_LINK) \
-I$(GTEST_LINK)/include $(CPPFLAGS)

APP_CXXFLAGS=\
-g -O2 $(CXXFLAGS)

APP_LDFLAGS= $(LDFLAGS)

APP_LDADD=\
$(SDK_BUILD)/aas/OSAL/libOSAL.la \
$(SDK_BUILD)/aas/AASLib/libAAS.la \
$(SDK_BUILD)/aas/XLRTLib/libxlrt.la \
$(libGTEST_LA)

APP_LIBS=\
-lrt -lltdl -lpthread


libGTEST_LA=libgtest.la
libGTEST_SRC=$(GTEST_LINK)/src/gtest-all.cc
libGTEST_OBJ=libgtest.lo
libGTEST_PLO=libgtest.Plo

libGTEST_CPPFLAGS=\
-I$(GTEST_LINK) \
-I$(GTEST_LINK)/include \
-DGTEST_HAS_TR1_TUPLE=1 $(CPPFLAGS)

libGTEST_CXXFLAGS=\
-g -O2 $(CXXFLAGS)

libGTEST_LDFLAGS=\
-avoid-version $(LDFLAGS)

libGTEST_LIBADD=\
$(LIBADD)

libGTEST_LIBS=\
-lpthread


all: $(APP_EXE)

$(APP_EXE): $(APP_OBJ) $(APP_LDADD)
	$(LIBTOOL) --tag=CXX --mode=link $(CXX) $(APP_CXXFLAGS) $(APP_LDFLAGS) -o $@ $(APP_OBJ) $(APP_LDADD) $(APP_LIBS)

.deps/$(APP_PO): $(GTEST_LINK) $(DIRS)

-include .deps/$(APP_PO)

.cpp.o:
	$(CXX) $(APP_CPPFLAGS) $(APP_CXXFLAGS) -MT $@ -MD -MP -MF .deps/$(@:.o=.Tpo) -c -o $@ $<
	mv .deps/$(@:.o=.Tpo) .deps/$(@:.o=.Po)

$(APP_OBJ): $(GTEST_LINK) $(DIRS)


$(libGTEST_LA): $(libGTEST_OBJ)
	$(LIBTOOL) --tag=CXX --mode=link $(CXX) $(libGTEST_CXXFLAGS) $(libGTEST_LDFLAGS) -o $@ -rpath $(lib_RPATH) $(libGTEST_OBJ) $(libGTEST_LIBADD) $(libGTEST_LIBS)

.deps/$(libGTEST_PLO): $(GTEST_LINK) $(DIRS)

-include .deps/$(libGTEST_PLO)

$(libGTEST_OBJ): $(libGTEST_SRC) 
	$(LIBTOOL) --tag=CXX --mode=compile $(CXX) $(libGTEST_CPPFLAGS) $(libGTEST_CXXFLAGS) -MT $@ -MD -MP -MF .deps/$*.Tpo -c -o $@ $<
	mv .deps/$*.Tpo .deps/$*.Plo

$(libGTEST_SRC): $(GTEST_LINK) $(DIRS) $(libGTEST_LIBADD)

$(GTEST_LINK):
	@if ! test -d '$(GTEST_DIR)/gtest' ; then tar -C $(GTEST_DIR) -zxf $(GTEST_DIR)/$(GTEST_TARBALL) && mv $(GTEST_DIR)/$(GTEST_DISTDIR) $(GTEST_DIR)/gtest ; fi ; if ! test -L '$(GTEST_LINK)' ; then ln -s $(GTEST_DIR)/gtest $(GTEST_LINK) ; fi



$(DIRS):
	@mkdir -p $@

clean:
	$(RM) $(APP_EXE) $(APP_OBJ) $(libGTEST_LA) $(libGTEST_OBJ) $(libGTEST_OBJ:.lo=.o) $(GTEST_LINK) -r $(DIRS)

.PHONY: all clean
