## Copyright (c) 2005-2014, Intel Corporation
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
##     libOSAL: Intel(R) Accelerator Abstraction Layer Library
##        OS Abstraction Layer
##     aas/OSAL/Makefile.am
##  Author:
##     Tim Whisonant, Intel Corporation
##  History:
##     06/26/2013    TSW   Initial version
##******************************************************************************
lib_LTLIBRARIES=libOSAL.la

libOSAL_la_SOURCES=\
CriticalSection.cpp \
DynLinkLibrary.cpp \
OSLib.cpp \
OSSemaphore.cpp \
OSServiceModule.c \
Sleep.cpp \
Thread.cpp \
ThreadGroup.cpp \
Timer.cpp

libOSAL_la_CPPFLAGS=\
-DLIBDIR=\"$(libdir)\"

if AAL_COND_ENABLE_DBG_DYNLOAD
libOSAL_la_CPPFLAGS+=-DDBG_DYN_LOAD=1
endif

libOSAL_la_CPPFLAGS+=\
-I$(top_srcdir)/include \
-I$(top_builddir)/include

libOSAL_la_LDFLAGS=\
-version-info $(OSAL_VERSION_CURRENT):$(OSAL_VERSION_REVISION):$(OSAL_VERSION_AGE)
