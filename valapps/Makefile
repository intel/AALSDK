# INTEL CONFIDENTIAL - For Intel Internal Use Only


## These subdirectories must be built first
SUBDIRS1=\
valapp   \
appbase/ \
vallib/  

## Automagically include ALL sub-directories that have a Makefile (Thanks TBD!)
SUBDIRS2 := $(shell find . -name Makefile|sed 's^./^^;s/Makefile//')

## Removed SUBDIRS2 go below. Notice the final comma.
## Need to remove those in SUBDIRS1
## If you add a directory that you do not want built, add it here.
SUBDIRS2 :=$(filter-out \
${SUBDIRS1} \
, $(SUBDIRS2))

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
#	@echo  ${SUBDIRS1} ${SUBDIRS2}
	@for d in ${SUBDIRS1} ${SUBDIRS2} ; do (cd "$$d" && $(MAKE) $(MFLAGS) $@) || exit 1 ; done

.PHONY: all clean

#doc docs: docclean
#	doc/output/./test.sh
#	doxygen doc/Doxyfile
#docclean docsclean:
#	@$(RM) -rf doc/doxygen
#
#.PHONY:doc docs docclean docsclean
