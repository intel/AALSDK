## Copyright (c) 2014, Intel Corporation
##
## Redistribution  and  use  in source  and  binary  forms,  with  or  without
## modification, are permitted provided that the following conditions are met:
##
## * Redistributions of  source code  must retain the  above copyright notice,
##   this list of conditions and the following disclaimer.
## * Redistributions in binary form must reproduce the above copyright notice,
##   this list of conditions and the following disclaimer in the documentation
##   and/or other materials provided with the distribution.
## * Neither the name  of Intel Corporation  nor the names of its contributors
##   may be used to  endorse or promote  products derived  from this  software
##   without specific prior written permission.
##
## THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
## AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  BUT NOT LIMITED TO,  THE
## IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
## ARE DISCLAIMED.  IN NO EVENT  SHALL THE COPYRIGHT OWNER  OR CONTRIBUTORS BE
## LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR
## CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT LIMITED  TO,  PROCUREMENT  OF
## SUBSTITUTE GOODS OR SERVICES;  LOSS OF USE,  DATA, OR PROFITS;  OR BUSINESS
## INTERRUPTION)  HOWEVER CAUSED  AND ON ANY THEORY  OF LIABILITY,  WHETHER IN
## CONTRACT,  STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE  OR OTHERWISE)
## ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,  EVEN IF ADVISED OF THE
## POSSIBILITY OF SUCH DAMAGE.
##****************************************************************************
##     Intel(R) Accelerator Abstraction Layer Library Software Developer
##        Kit (SDK)
##  Content:
##        Basic Acceptance Test
##     bat/Makefile.am
##  Author:
##     Tim Whisonant, Intel Corporation
##  History:
##     08/29/2014    TSW   Initial version
##******************************************************************************
test_cases=\
cases/Sem.cpp \
cases/NVS.cpp \
cases/ThreadGroup.cpp \
cases/Paths.cpp \
cases/StartupShutdown.cpp

EXTRA_DIST=$(test_cases)


if AAL_COND_GTEST
check_LTLIBRARIES=libgtest.la

nodist_libgtest_la_SOURCES=gtest-all.cc

gtest-all.cc:
	@ln -s $(GTEST_SRCDIR)/gtest-all.cc $@

libgtest_la_CPPFLAGS=$(GTEST_CPPFLAGS)
libgtest_la_CFLAGS=$(GTEST_CFLAGS)
libgtest_la_CXXFLAGS=$(GTEST_CXXFLAGS)
libgtest_la_LDFLAGS=$(GTEST_LDFLAGS)


check_PROGRAMS=bat

bat_SOURCES=\
bat.cpp \
BAT-TEST-LIST.h

bat_CPPFLAGS=\
-DOSAL_LIBDIR=\"$(abs_top_builddir)/aas/OSAL/.libs\" \
-DAAS_LIBDIR=\"$(abs_top_builddir)/aas/AASLib/.libs\" \
-DXLRT_LIBDIR=\"$(abs_top_builddir)/aas/XLRTLib/.libs\" \
-DXLRRMBROKER_LIBDIR=\"$(abs_top_builddir)/aas/XLRRMBrokerService/.libs\" \
-DAASEDS_LIBDIR=\"$(abs_top_builddir)/aas/AASEventDeliveryService/.libs\" \
-DUAIA_LIBDIR=\"$(abs_top_builddir)/aas/uAIA/.libs\" \
-I$(top_srcdir)/include \
-I$(top_builddir)/include $(GTEST_CPPFLAGS)

bat_LDADD=\
$(check_LTLIBRARIES) \
$(top_builddir)/aas/OSAL/libOSAL.la \
$(top_builddir)/aas/AASLib/libAAS.la \
$(top_builddir)/aas/XLRTLib/libxlrt.la

$(bat_SOURCES): $(test_cases)

check_SCRIPTS=\
bat.sh

# check_SCRIPTS=\
# sample1.sh \
# sample2.sh \
# cciapp.sh \
# splapp.sh \
# bat.sh

TESTS=$(check_SCRIPTS)
CLEANFILES=$(check_SCRIPTS) gtest-all.cc

bat.sh:
	@echo "  sh     $@"
	@echo -e "#!/bin/sh\nset -e\n./$(subst .sh,,$@)" >$@
	@chmod '+x' $@

# sample1.sh:
# 	@echo "  sh     $@"
# 	@echo -e "#!/bin/sh\nset -e\nLD_LIBRARY_PATH=$(abs_top_builddir)/samples/AFU1/.libs $(abs_top_builddir)/samples/App1/XLSample1" >$@
# 	@chmod '+x' $@
# 
# sample2.sh:
# 	@echo "  sh     $@"
# 	@echo -e "#!/bin/sh\nset -e\nLD_LIBRARY_PATH=$(abs_top_builddir)/samples/AFU2/.libs $(abs_top_builddir)/samples/App2/XLSample2 <$@ | LD_LIBRARY_PATH=$(abs_top_builddir)/samples/AFU2/.libs $(abs_top_builddir)/samples/App2/XLSample2" >$@
# 	@chmod '+x' $@
# 
# cciapp.sh:
# 	@echo "  sh     $@"
# 	@echo -e "#!/bin/sh\nset -e\nLD_LIBRARY_PATH=$(abs_top_builddir)/samples/CCIAFU/.libs:$(abs_top_builddir)/samples/CCIAFU/SWSimCCIAFU/.libs $(abs_top_builddir)/samples/cciapp/cciapp --target=swsim" >$@
# 	@chmod '+x' $@
# 
# splapp.sh:
# 	@echo "  sh     $@"
# 	@echo -e "#!/bin/sh\nset -e\nLD_LIBRARY_PATH=$(abs_top_builddir)/samples/SPLAFU/.libs:$(abs_top_builddir)/samples/SPLAFU/SWSimSPLAFU/.libs $(abs_top_builddir)/samples/splapp/splapp --target=swsim" >$@
# 	@chmod '+x' $@

endif

