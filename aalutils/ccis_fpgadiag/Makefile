## Copyright (c) 2015, Intel Corporation
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
##     aalutils/fpgadiag/Makefile
##  Author:
##     Tim Whisonant, Intel Corporation
##  History:
##     07/21/2015    TSW   Initial version
##******************************************************************************
CPPFLAGS ?=
CXX      ?= g++
LDFLAGS  ?=

ifneq (,$(ndebug))
else
CPPFLAGS += -DENABLE_DEBUG=1
endif
ifneq (,$(nassert))
else
CPPFLAGS += -DENABLE_ASSERT=1
endif

ifeq (,$(DESTDIR))
ifneq (,$(prefix))
CPPFLAGS += -I$(prefix)/include
LDFLAGS  += -L$(prefix)/lib -Wl,-rpath-link -Wl,$(prefix)/lib -Wl,-rpath -Wl,$(prefix)/lib \
            -L$(prefix)/lib64 -Wl,-rpath-link -Wl,$(prefix)/lib64 -Wl,-rpath -Wl,$(prefix)/lib64
endif
else
ifeq (,$(prefix))
prefix = /usr/local
endif
CPPFLAGS += -I$(DESTDIR)$(prefix)/include
LDFLAGS  += -L$(DESTDIR)$(prefix)/lib -Wl,-rpath-link -Wl,$(prefix)/lib -Wl,-rpath -Wl,$(DESTDIR)$(prefix)/lib \
            -L$(DESTDIR)$(prefix)/lib64 -Wl,-rpath-link -Wl,$(prefix)/lib64 -Wl,-rpath -Wl,$(DESTDIR)$(prefix)/lib64
endif

all: ccis_fpgadiag

ccis_fpgadiag: ccis_fpgadiag.o ccis_diag_lpbk1.o ccis_diag_read.o ccis_diag_write.o ccis_diag_trput.o ccis_diag_sw.o ccis_diag_ccip_lpbk1.o ccis_diag_ccip_read.o ccis_diag_ccip_write.o ccis_diag_ccip_trput.o ccis_diag_ccip_sw.o ccis_diag-nlb-common.o
	$(CXX) -g -O2 -o ccis_fpgadiag ccis_fpgadiag.o ccis_diag_lpbk1.o ccis_diag_read.o ccis_diag_write.o ccis_diag_trput.o ccis_diag_sw.o ccis_diag_ccip_lpbk1.o ccis_diag_ccip_read.o ccis_diag_ccip_write.o ccis_diag_ccip_trput.o ccis_diag_ccip_sw.o ccis_diag-nlb-common.o $(LDFLAGS) -lOSAL -lAAS -laalrt -laalclp

ccis_fpgadiag.o: ccis_fpgadiag.cpp ccis_diag-common.h ccis_diag_defaults.h ccis_diag-nlb-common.h ccis_fpgadiagDefs.h Makefile
	$(CXX) $(CPPFLAGS) -D__AAL_USER__=1 -g -O2 -c -o ccis_fpgadiag.o ccis_fpgadiag.cpp

ccis_diag_lpbk1.o: ccis_diag_lpbk1.cpp ccis_diag-common.h ccis_diag_defaults.h ccis_diag-nlb-common.h ccis_fpgadiagDefs.h Makefile
	$(CXX) $(CPPFLAGS) -D__AAL_USER__=1 -g -O2 -c -o ccis_diag_lpbk1.o ccis_diag_lpbk1.cpp

ccis_diag_read.o: ccis_diag_read.cpp ccis_diag-common.h ccis_diag_defaults.h ccis_diag-nlb-common.h ccis_fpgadiagDefs.h Makefile
	$(CXX) $(CPPFLAGS) -D__AAL_USER__=1 -g -O2 -c -o ccis_diag_read.o ccis_diag_read.cpp

ccis_diag_write.o: ccis_diag_write.cpp ccis_diag-common.h ccis_diag_defaults.h ccis_diag-nlb-common.h ccis_fpgadiagDefs.h Makefile
	$(CXX) $(CPPFLAGS) -D__AAL_USER__=1 -g -O2 -c -o ccis_diag_write.o ccis_diag_write.cpp

ccis_diag_trput.o: ccis_diag_trput.cpp ccis_diag-common.h ccis_diag_defaults.h ccis_diag-nlb-common.h ccis_fpgadiagDefs.h Makefile
	$(CXX) $(CPPFLAGS) -D__AAL_USER__=1 -g -O2 -c -o ccis_diag_trput.o ccis_diag_trput.cpp
	
ccis_diag_sw.o: ccis_diag_sw.cpp ccis_diag-common.h ccis_diag_defaults.h ccis_diag-nlb-common.h ccis_fpgadiagDefs.h Makefile
	$(CXX) $(CPPFLAGS) -D__AAL_USER__=1 -g -O2 -c -o ccis_diag_sw.o ccis_diag_sw.cpp
	
ccis_diag_ccip_lpbk1.o: ccis_diag_ccip_lpbk1.cpp ccis_diag-common.h ccis_diag_defaults.h ccis_diag-nlb-common.h ccis_fpgadiagDefs.h Makefile
	$(CXX) $(CPPFLAGS) -D__AAL_USER__=1 -g -O2 -c -o ccis_diag_ccip_lpbk1.o ccis_diag_ccip_lpbk1.cpp

ccis_diag_ccip_read.o: ccis_diag_ccip_read.cpp ccis_diag-common.h ccis_diag_defaults.h ccis_diag-nlb-common.h ccis_fpgadiagDefs.h Makefile
	$(CXX) $(CPPFLAGS) -D__AAL_USER__=1 -g -O2 -c -o ccis_diag_ccip_read.o ccis_diag_ccip_read.cpp
	
ccis_diag_ccip_write.o: ccis_diag_ccip_write.cpp ccis_diag-common.h ccis_diag_defaults.h ccis_diag-nlb-common.h ccis_fpgadiagDefs.h Makefile
	$(CXX) $(CPPFLAGS) -D__AAL_USER__=1 -g -O2 -c -o ccis_diag_ccip_write.o ccis_diag_ccip_write.cpp
	
ccis_diag_ccip_trput.o: ccis_diag_ccip_trput.cpp ccis_diag-common.h ccis_diag_defaults.h ccis_diag-nlb-common.h ccis_fpgadiagDefs.h Makefile
	$(CXX) $(CPPFLAGS) -D__AAL_USER__=1 -g -O2 -c -o ccis_diag_ccip_trput.o ccis_diag_ccip_trput.cpp
	
ccis_diag_ccip_sw.o: ccis_diag_ccip_sw.cpp ccis_diag-common.h ccis_diag_defaults.h ccis_diag-nlb-common.h ccis_fpgadiagDefs.h Makefile
	$(CXX) $(CPPFLAGS) -D__AAL_USER__=1 -g -O2 -c -o ccis_diag_ccip_sw.o ccis_diag_ccip_sw.cpp

ccis_diag-nlb-common.o: ccis_diag-nlb-common.cpp ccis_diag-common.h ccis_diag_defaults.h ccis_diag-nlb-common.h ccis_fpgadiagDefs.h Makefile
	$(CXX) $(CPPFLAGS) -D__AAL_USER__=1 -g -O2 -c -o ccis_diag-nlb-common.o ccis_diag-nlb-common.cpp

clean:
	$(RM) ccis_fpgadiag ccis_fpgadiag.o ccis_diag_lpbk1.o ccis_diag_read.o ccis_diag_write.o ccis_diag_trput.o ccis_diag_sw.o ccis_diag_ccip_lpbk1.o ccis_diag_ccip_read.o ccis_diag_ccip_write.o ccis_diag_ccip_trput.o ccis_diag_ccip_sw.o ccis_diag-nlb-common.o

.PHONY:all clean
