## Copyright(c) 2005-2016, Intel Corporation
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
##  Accelerator Abstraction Layer Library Software Developer Kit (SDK)
##
##  Content:
##     samples/Hello_ALI_NLB/Makefile
##  Author:
##     Tim Whisonant, Intel Corporation
##     Joseph Grecco, Intel Corporation
##  History:
##     06/09/2015    JG   Initial version based off of canonical sample makefile
##******************************************************************************
CPPFLAGS ?= -I ../../../valapp
CXX      ?= g++
LDFLAGS  ?=

ifeq (,$(CFLAGS))
CFLAGS = -g -O2
endif

ifneq (,$(ndebug))
else
CPPFLAGS += -DENABLE_DEBUG=1 -D ASE_DEBUG=1
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

all: ALInlb

ALInlb: main.o ALINLB.o
	$(CXX) $(CFLAGS) -o ALInlb main.o ALINLB.o ../../../valapp/libvalutils.la $(LDFLAGS) -lOSAL -lAAS -laalrt

main.o: main.cpp  Makefile
	$(CXX) $(CPPFLAGS) -D__AAL_USER__=1  $(CFLAGS) -c -o main.o main.cpp

ALINLB.o: ../../PR_SingleApp/ALINLB.cpp ../../PR_SingleApp/ALINLB.h Makefile
	$(CXX) $(CPPFLAGS) -D__AAL_USER__=1 -g -O2 -c -o ALINLB.o ../../PR_SingleApp/ALINLB.cpp
	
clean:
	$(RM) ALInlb main.o ALINLB.o

.PHONY:all clean
