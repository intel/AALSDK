## Copyright (c) 2005-2015, Intel Corporation
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
##     samples/AFU1/Makefile
##  Author:
##     Tim Whisonant, Intel Corporation
##  History:
##     11/17/2014    TSW   Initial version
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
LDFLAGS  += -L$(prefix)/lib -L$(prefix)/lib64
endif
else
ifeq (,$(prefix))
prefix = /usr/local
endif
CPPFLAGS += -I$(DESTDIR)$(prefix)/include
LDFLAGS  += -L$(DESTDIR)$(prefix)/lib -L$(DESTDIR)$(prefix)/lib64
endif

all: libsampleafu1.so libsampleafu1.so.0

libsampleafu1.so: libsampleafu1.so.0.0.0
	ln -sf libsampleafu1.so.0.0.0 libsampleafu1.so

libsampleafu1.so.0: libsampleafu1.so.0.0.0
	ln -sf libsampleafu1.so.0.0.0 libsampleafu1.so.0

libsampleafu1.so.0.0.0: libsampleafu1.o
	$(CXX) -fPIC -shared -Wl,-soname -Wl,libsampleafu1.so.0 libsampleafu1.o $(LDFLAGS) -lOSAL -lAAS -lxlrt -o libsampleafu1.so.0.0.0

libsampleafu1.o: SampleAFU1Service.cpp SampleAFU1Service-internal.h SampleAFU1Service.h Makefile
	$(CXX) $(CPPFLAGS) -D__AAL_USER__=1 -fPIC -DPIC -g -O2 -c -o libsampleafu1.o SampleAFU1Service.cpp

clean:
	$(RM) libsampleafu1.so libsampleafu1.so.0 libsampleafu1.so.0.0.0 libsampleafu1.o

.PHONY: all clean
