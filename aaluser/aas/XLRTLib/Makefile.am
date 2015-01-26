## Copyright (c) 2014-2015, Intel Corporation
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
##     libxlrt: Intel(R) Accelerator Abstraction Layer Library
##        Accelerator Abstraction Services XL (accelerator) Runtime Library
##     aas/XLRTLib/Makefile.am
##  Author:
##     Joseph Grecco, Intel Corporation
##     Tim Whisonant, Intel Corporation
##  History:
##******************************************************************************

## libxlrt is a typically loaded shared object library

lib_LTLIBRARIES=libxlrt.la

libxlrt_la_SOURCES=\
xlrtlib.cpp \
xlRuntime.cpp \
_xlRuntimeImpl.h \
_xlRuntimeImpl.cpp \
_xlMessageDelivery.h \
_xlMessageDelivery.cpp \
_xlServiceBroker.h \
_xlServiceBroker.cpp \
ServiceHost.cpp

libxlrt_la_CPPFLAGS=\
-I$(top_srcdir)/include \
-I$(top_builddir)/include \
-I$(srcdir)

libxlrt_la_LDFLAGS=\
-version-info $(XLRT_VERSION_CURRENT):$(XLRT_VERSION_REVISION):$(XLRT_VERSION_AGE)

libxlrt_la_LIBADD=\
$(top_builddir)/aas/OSAL/libOSAL.la \
$(top_builddir)/aas/AASLib/libAAS.la
