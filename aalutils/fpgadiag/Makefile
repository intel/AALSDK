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

all: fpgadiag

fpgadiag: fpgadiag.o diag_ccip_lpbk1.o diag_ccip_read.o diag_ccip_write.o diag_ccip_trput.o diag_ccip_sw.o diag-nlb-common.o #diag_lpbk1.o diag_read.o diag_write.o diag_trput.o diag_sw.o 
	$(CXX) -g -O2 -o fpgadiag fpgadiag.o diag_ccip_lpbk1.o diag_ccip_read.o diag_ccip_write.o diag_ccip_trput.o diag_ccip_sw.o diag-nlb-common.o $(LDFLAGS) -lOSAL -lAAS -laalrt -laalclp #diag_lpbk1.o diag_read.o diag_write.o diag_trput.o diag_sw.o 

#fpgadiag.o: fpgadiag.cpp diag-common.h diag_defaults.h diag-nlb-common.h fpgadiagDefs.h Makefile
#	$(CXX) $(CPPFLAGS) -D__AAL_USER__=1 -g -O2 -c -o fpgadiag.o fpgadiag.cpp

#diag_lpbk1.o: diag_lpbk1.cpp diag-common.h diag_defaults.h diag-nlb-common.h fpgadiagDefs.h Makefile
#	$(CXX) $(CPPFLAGS) -D__AAL_USER__=1 -g -O2 -c -o diag_lpbk1.o diag_lpbk1.cpp

#diag_read.o: diag_read.cpp diag-common.h diag_defaults.h diag-nlb-common.h fpgadiagDefs.h Makefile
#	$(CXX) $(CPPFLAGS) -D__AAL_USER__=1 -g -O2 -c -o diag_read.o diag_read.cpp

#diag_write.o: diag_write.cpp diag-common.h diag_defaults.h diag-nlb-common.h fpgadiagDefs.h Makefile
#	$(CXX) $(CPPFLAGS) -D__AAL_USER__=1 -g -O2 -c -o diag_write.o diag_write.cpp

#diag_trput.o: diag_trput.cpp diag-common.h diag_defaults.h diag-nlb-common.h fpgadiagDefs.h Makefile
#	$(CXX) $(CPPFLAGS) -D__AAL_USER__=1 -g -O2 -c -o diag_trput.o diag_trput.cpp
	
#diag_sw.o: diag_sw.cpp diag-common.h diag_defaults.h diag-nlb-common.h fpgadiagDefs.h Makefile
#	$(CXX) $(CPPFLAGS) -D__AAL_USER__=1 -g -O2 -c -o diag_sw.o diag_sw.cpp
	
diag_ccip_lpbk1.o: diag_ccip_lpbk1.cpp diag-common.h diag_defaults.h diag-nlb-common.h fpgadiagDefs.h Makefile
	$(CXX) $(CPPFLAGS) -D__AAL_USER__=1 -g -O2 -c -o diag_ccip_lpbk1.o diag_ccip_lpbk1.cpp

diag_ccip_read.o: diag_ccip_read.cpp diag-common.h diag_defaults.h diag-nlb-common.h fpgadiagDefs.h Makefile
	$(CXX) $(CPPFLAGS) -D__AAL_USER__=1 -g -O2 -c -o diag_ccip_read.o diag_ccip_read.cpp
	
diag_ccip_write.o: diag_ccip_write.cpp diag-common.h diag_defaults.h diag-nlb-common.h fpgadiagDefs.h Makefile
	$(CXX) $(CPPFLAGS) -D__AAL_USER__=1 -g -O2 -c -o diag_ccip_write.o diag_ccip_write.cpp
	
diag_ccip_trput.o: diag_ccip_trput.cpp diag-common.h diag_defaults.h diag-nlb-common.h fpgadiagDefs.h Makefile
	$(CXX) $(CPPFLAGS) -D__AAL_USER__=1 -g -O2 -c -o diag_ccip_trput.o diag_ccip_trput.cpp
	
diag_ccip_sw.o: diag_ccip_sw.cpp diag-common.h diag_defaults.h diag-nlb-common.h fpgadiagDefs.h Makefile
	$(CXX) $(CPPFLAGS) -D__AAL_USER__=1 -g -O2 -c -o diag_ccip_sw.o diag_ccip_sw.cpp

diag-nlb-common.o: diag-nlb-common.cpp diag-common.h diag_defaults.h diag-nlb-common.h fpgadiagDefs.h Makefile
	$(CXX) $(CPPFLAGS) -D__AAL_USER__=1 -g -O2 -c -o diag-nlb-common.o diag-nlb-common.cpp

clean:
	$(RM) fpgadiag fpgadiag.o diag_ccip_lpbk1.o diag_ccip_read.o diag_ccip_write.o diag_ccip_trput.o diag_ccip_sw.o diag-nlb-common.o #diag_lpbk1.o diag_read.o diag_write.o diag_trput.o diag_sw.o 

.PHONY:all clean
