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
##     aalutils/Makefile
##  Author:
##     Tim Whisonant, Intel Corporation
##  History:
##     07/21/2015    TSW   Initial version
##******************************************************************************

# nothing to build right now
SUBDIRS=

MFLAGS  =
ifneq (,$(ndebug))
MFLAGS += ndebug=$(ndebug)
endif
ifneq (,$(nassert))
MFLAGS += nassert=$(nassert)
endif

ifeq (,$(DESTDIR))
ifneq (,$(prefix))
MFLAGS += prefix=$(prefix)
endif
else
ifeq (,$(prefix))
prefix  = /usr/local
else
MFLAGS += prefix=$(prefix)
endif
MFLAGS += DESTDIR=$(DESTDIR)
endif

all clean:
	@for d in ${SUBDIRS} ; do (cd "$$d" && $(MAKE) $(MFLAGS) $@) || exit 1 ; done

.PHONY:all clean

doc docs: docclean
	doxygen doc/Doxyfile

docclean docsclean:
	@$(RM) -rf doc/doxygen

.PHONY:doc docs docclean docsclean
