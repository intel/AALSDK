# INTEL CONFIDENTIAL - For Intel Internal Use Only

SUBDIRS=\
vallib \
MMIO_Mapping \
UMsg_Allocation \
DMA_Buffer \
Read_Perf_Counters \
AFU_Reset \
Signal_Tap \
Partial_Reconfig/PR_SingleApp \
Partial_Reconfig/PR_TwoApp/nlb_app \
Partial_Reconfig/PR_TwoApp/pr_app \
RAS \
Power_Thermal \
Error


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

#doc docs: docclean
#	doc/output/./test.sh
#	doxygen doc/Doxyfile
#docclean docsclean:
#	@$(RM) -rf doc/doxygen
#
#.PHONY:doc docs docclean docsclean
