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
##  Accelerator Abstraction Layer Library Software Developer Kit (SDK)
##
##  Content:
##     samples/UMsgTest/Makefile
##  Author:
##     Tim Whisonant, Intel Corporation
##  History:
##     11/20/2014    TSW   Initial version
##******************************************************************************
CPPFLAGS ?=
CXX      ?= g++
LDFLAGS  ?=

ifneq (,$(ndebug))
else
CPPFLAGS += -DENABLE_DEBUG=1 -DASE_DEBUG=1
endif
ifneq (,$(nassert))
else
CPPFLAGS += -DENABLE_ASSERT=1 -DASE_DEBUG=1
endif

ifeq (,$(DESTDIR))
ifneq (,$(prefix))
CPPFLAGS += -I$(prefix)/include
LDFLAGS  += -L$(prefix)/lib -Wl,-rpath-link -Wl,$(prefix)/lib -Wl,-rpath -Wl,$(prefix)/lib
endif
else
ifeq (,$(prefix))
prefix = /usr/local
endif
CPPFLAGS += -I$(DESTDIR)$(prefix)/include
LDFLAGS  += -L$(DESTDIR)$(prefix)/lib -Wl,-rpath-link -Wl,$(DESTDIR)$(prefix)/lib -Wl,-rpath -Wl,$(DESTDIR)$(prefix)/lib
endif

all: Histogram

Histogram: Histogram.o
	gcc -c -std=c99 -o shm_ops.o      ../../aaluser/ase/sw/shm_ops.c -I ../../aaluser/ase/sw/ -lrt
	gcc -c -std=c99 -o tstamp_ops.o   ../../aaluser/ase/sw/tstamp_ops.c -I ../../aaluser/ase/sw/ -lrt
	gcc -c -std=c99 -o mqueue_ops.o   ../../aaluser/ase/sw/mqueue_ops.c -I ../../aaluser/ase/sw/ -lrt
	gcc -c -std=c99 -o error_report.o ../../aaluser/ase/sw/error_report.c -I ../../aaluser/ase/sw/ -lrt
	gcc -c -std=c99 -o ase_ops.o      ../../aaluser/ase/sw/ase_ops.c -I ../../aaluser/ase/sw/ -lrt
	g++ -o Histogram Histogram.o ase_ops.o error_report.o mqueue_ops.o shm_ops.o tstamp_ops.o ../../myinst/usr/local/lib/libAAS.so ../../myinst/usr/local/lib/libAASRegistrar.so ../../myinst/usr/local/lib/libAASUAIA.so ../../myinst/usr/local/lib/libASE.so ../../myinst/usr/local/lib/libASECCIAFU.so ../../myinst/usr/local/lib/libASESPLAFU.so ../../myinst/usr/local/lib/libCCIAFU.so ../../myinst/usr/local/lib/libHWCCIAFU.so ../../myinst/usr/local/lib/libHWSPLAFU.so ../../myinst/usr/local/lib/libOSAL.so ../../myinst/usr/local/lib/libSPLAFU.so ../../myinst/usr/local/lib/libSWSimCCIAFU.so ../../myinst/usr/local/lib/libSWSimSPLAFU.so ../../myinst/usr/local/lib/libaalclp.so ../../myinst/usr/local/lib/librrmbroker.so ../../myinst/usr/local/lib/libxlrt.so ../CCILib/CCILib.o -lrt

	objdump -D -S Histogram > Histogram.objdump.log

#	$(CXX) -g -O2 -o Histogram Histogram.o `ls *.o` $(LDFLAGS) -L../CCILib -lCCI -laalclp -lrt

Histogram.o: Histogram.cpp ../CCILib/CCILib.h Makefile
	$(CXX) $(CPPFLAGS) -std=c++0x -D__AAL_USER__=1 -I../CCILib -g -O2 -c -o Histogram.o Histogram.cpp -DCCILIB_ENABLE_AAL=1 -DCCILIB_ENABLE_ASE=1

clean:
	$(RM) Histogram *.o 

.PHONY:all clean

