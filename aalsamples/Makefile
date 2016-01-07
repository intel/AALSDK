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
##     samples/Makefile
##  Author:
##     Tim Whisonant, Intel Corporation
##  History:
##     11/18/2014    TSW   Initial version
##******************************************************************************

## NOTE: Removing SPL support but will add back in something similar, so 
##       leaving in the code but not attempting to build (SPL will not build)
## NOTE: Removed CCI-S support, but difference between Hello_CCI_NLB and
##       Hello_ALI_NLB is useful, so leaving Hello_CCI_NLB in place for now.
SUBDIRS=\
Hello_AAL_Service/SW \
Hello_AAL/SW \
Hello_ALI_NLB/SW \
ALI_Multi_AFU_with_PerfMon/SW \
ALI_Configure_AFU/SW \
SignalTapService \
SigTapSample/SW

#Removed SUBDIRS
#SUBDIRS=\
#Hello_CCI_NLB/SW \
#Hello_SPL_LB/SW \
#Sudoku_SPL/SW \
#LinkedList_SPL/SW \



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

.PHONY: all clean

doc docs: docclean
	doc/output/./test.sh
	doxygen doc/Doxyfile
docclean docsclean:
	@$(RM) -rf doc/doxygen

.PHONY:doc docs docclean docsclean
