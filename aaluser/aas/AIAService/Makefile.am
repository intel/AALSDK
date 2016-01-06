## Copyright(c) 2015-2016, Intel Corporation
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
##     libuAIA: Intel(R) Accelerator Abstraction Layer Library
##        Accelerator Abstraction Services
##     aas/AIAService/Makefile.am
##  Author:
##     Tim Whisonant, Intel Corporation
##  History:
##     12/7/2015    TSW   Initial version
##******************************************************************************

# We _are_ dlopen'ing libaia.

lib_LTLIBRARIES=libaia.la

libaia_la_SOURCES=\
AIADllMain.cpp \
AIA-internal.h \
AIAService.cpp \
AIATransactions.cpp \
AIATransactions.h \
ALIAFUProxy.cpp \
ALIAFUProxy.h \
UIDriverInterfaceAdapter.cpp \
UIDriverInterfaceAdapter.h \
uidrvMessage.cpp \
uidrvMessage.h

libaia_la_CPPFLAGS=\
-I$(top_srcdir)/include \
-I$(top_builddir)/include

libaia_la_LDFLAGS=\
-module \
-version-info $(AIASERVICE_VERSION_CURRENT):$(AIASERVICE_VERSION_REVISION):$(AIASERVICE_VERSION_AGE)

libaia_la_LIBADD=\
$(top_builddir)/aas/OSAL/libOSAL.la \
$(top_builddir)/aas/AASLib/libAAS.la \
$(top_builddir)/aas/AALRuntime/libaalrt.la
