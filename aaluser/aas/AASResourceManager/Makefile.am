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
##     Intel(R) Accelerator Abstraction Layer Library Software Developer
##        Kit (SDK)
##  Content:
##     libAAS: Intel(R) Accelerator Abstraction Layer Library
##        Accelerator Abstraction Services
##     aas/AASResourceManager/Makefile.am
##  Author:
##     Tim Whisonant, Intel Corporation
##  History:
##     06/27/2013    TSW   Initial version
##******************************************************************************
lib_LTLIBRARIES=libAASResMgr.la

libAASResMgr_la_SOURCES=\
CAASResourceManager.cpp \
InstanceRecord.cpp \
ResMgrAlgorithms.cpp	

libAASResMgr_la_CPPFLAGS=\
-I$(top_srcdir)/include \
-I$(top_builddir)/include

libAASResMgr_la_LIBADD=\
$(top_builddir)/aas/OSAL/libOSAL.la \
$(top_builddir)/aas/AASLib/libAAS.la



bin_PROGRAMS=AASResourceManager

AASResourceManager_SOURCES=\
AASResourceManager.cpp

AASResourceManager_CPPFLAGS=\
-I$(top_srcdir)/include \
-I$(top_builddir)/include

AASResourceManager_LDADD=\
$(top_builddir)/aas/OSAL/libOSAL.la \
$(top_builddir)/aas/AASLib/libAAS.la \
$(top_builddir)/aas/AALRuntime/libaalrt.la


