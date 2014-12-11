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
##     libFAPTrans1 /
##     libFAPTrans2 : Intel(R) Accelerator Abstraction Layer Library
##        Accelerator Abstraction Services
##     aas/faptrans/Makefile.am
##  Author:
##     Tim Whisonant, Intel Corporation
##  History:
##     07/11/2013    TSW   Initial version
##******************************************************************************

# FAPTrans1 / FAPTrans2 / MAFUTrans are not currently being dlopen'ed.

lib_LTLIBRARIES=libFAPTrans1.la libFAPTrans2.la libMAFUTrans.la

libFAPTrans1_la_SOURCES=\
FAP10.cpp

libFAPTrans1_la_CPPFLAGS=\
-I$(top_srcdir)/include \
-I$(top_builddir)/include

libFAPTrans1_la_LDFLAGS=\
-version-info $(FAPTRANS1_VERSION_CURRENT):$(FAPTRANS1_VERSION_REVISION):$(FAPTRANS1_VERSION_AGE)

libFAPTrans1_la_LIBADD=\
$(top_builddir)/aas/OSAL/libOSAL.la


libFAPTrans2_la_SOURCES=\
FAP20.cpp

libFAPTrans2_la_CPPFLAGS=\
-I$(top_srcdir)/include \
-I$(top_builddir)/include

libFAPTrans2_la_LDFLAGS=\
-version-info $(FAPTRANS2_VERSION_CURRENT):$(FAPTRANS2_VERSION_REVISION):$(FAPTRANS2_VERSION_AGE)

libFAPTrans2_la_LIBADD=\
$(top_builddir)/aas/OSAL/libOSAL.la


libMAFUTrans_la_SOURCES=\
MAFUTransactions.cpp

libMAFUTrans_la_CPPFLAGS=\
-I$(top_srcdir)/include \
-I$(top_builddir)/include

libMAFUTrans_la_LDFLAGS=\
-version-info $(MAFUTRANS_VERSION_CURRENT):$(MAFUTRANS_VERSION_REVISION):$(MAFUTRANS_VERSION_AGE)
