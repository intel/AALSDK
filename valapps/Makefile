# INTEL CONFIDENTIAL - For Intel Internal Use Only

SUBDIRS=\
MMIO_Mapping \
UMsg_Allocation \
DMA_Buffer \
Read_Perf_Counters \
AFU_Reset \
Signal_Tap \
Partial_Reconfig \
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
