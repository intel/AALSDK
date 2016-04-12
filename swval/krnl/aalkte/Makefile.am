# INTEL CONFIDENTIAL - For Intel Internal Use Only
MY_KBUILD_VERBOSE    = $(my__kbuild_verbose_$(V))
my__kbuild_verbose_  = $(AM_DEFAULT_VERBOSITY)
my__kbuild_verbose_0 = 0
my__kbuild_verbose_1 = 1

moduledir=@AAL_OS_MODULE_INSTALL_PATH@

module_DATA=\
kbuild/@AALKTE_DRV_NAME@.ko

EXTRA_DIST=Kbuild.in

EXTRA_PROGRAMS=driver

driver_SOURCES=\
aalkte-main.c \
aalkte.h \
aalkte-control.c \
aalkte-status.c \
aalkte-thr.c

kbuild/@AALKTE_DRV_NAME@.ko: kbuild/Module.symvers

kbuild/Module.symvers: $(driver_SOURCES) $(abs_top_builddir)/aalkte/kbuild/Makefile
	@$(MAKE) -C "@AAL_OS_KERNEL_BUILD_PATH@" KBUILD_VERBOSE=$(MY_KBUILD_VERBOSE) M="$(abs_top_builddir)/aalkte/kbuild" aalkernelinc="$(abs_top_srcdir)/include" modules && \
if [ -f "$(abs_top_builddir)/aalkte/kbuild/@AALKTE_DRV_NAME@.o" ]; then touch "$(abs_top_builddir)/aalkte/kbuild/@AALKTE_DRV_NAME@.o" ; fi && \
if [ -f "$(abs_top_builddir)/aalkte/kbuild/@AALKTE_DRV_NAME@.mod.o" ]; then touch "$(abs_top_builddir)/aalkte/kbuild/@AALKTE_DRV_NAME@.mod.o" ; fi && \
if [ -f "$(abs_top_builddir)/aalkte/kbuild/Module.symvers" ]; then touch "$(abs_top_builddir)/aalkte/kbuild/Module.symvers" ; fi && \
if [ -f "$(abs_top_builddir)/aalkte/kbuild/@AALKTE_DRV_NAME@.ko" ]; then touch "$(abs_top_builddir)/aalkte/kbuild/@AALKTE_DRV_NAME@.ko" ; fi

clean-local:
	@$(MAKE) -C "@AAL_OS_KERNEL_BUILD_PATH@" KBUILD_VERBOSE=$(MY_KBUILD_VERBOSE) M="$(abs_top_builddir)/aalkte/kbuild" clean
