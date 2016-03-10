## Copyright(c) 2012-2016, Intel Corporation
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
##     Accelerator Abstraction Layer Library
##        clp/checks/Makefile.am
##  Author:
##     Tim Whisonant, Intel Corporation
##  History:
##     11/27/2012    TSW   Initial version
##******************************************************************************
check_PROGRAMS=scanner

scanner_SOURCES=\
scanner.c

scanner_CPPFLAGS=\
-I$(top_srcdir)/include \
-I$(top_builddir)/include

scanner_LDADD=\
$(top_builddir)/clp/libaalclp.la

EXTRA_DIST=_check.bash
check_SCRIPTS=check.bash
TESTS=$(check_SCRIPTS)

check.bash: $(top_srcdir)/clp/checks/_check.bash
	@cp $< $@
	@chmod 'a+x' $@

CLEANFILES=\
check.bash \
nix_short_checks \
nix_short_checks_compare \
nix_long_checks \
nix_long_checks_compare \
win_short_checks \
win_short_checks_compare \
win_long_checks \
win_long_checks_compare \
non_option_checks \
non_option_checks_compare

