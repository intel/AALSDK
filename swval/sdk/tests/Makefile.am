# INTEL CONFIDENTIAL - For Intel Internal Use Only

SUBDIRS=\
standalone \
harnessed

TESTSUITE_AT=\
standalone/standalone.at \
standalone/OSAL_TestSem/OSAL_TestSem.at \
standalone/OSAL_TestThreadGroup/OSAL_TestThreadGroup.at \
harnessed/harnessed.at \
harnessed/gtest/gtest.at \
local.at \
testsuite.at

EXTRA_DIST=\
$(TESTSUITE_AT) \
atlocal.in \
testsuite \
package.m4 \
run/bat.in \
run/nightly.in \
run/weekly.in \
run/OSAL_TestSem.in \
run/OSAL_TestThreadGroup.in

DISTCLEANFILES=\
atconfig \
$(check_SCRIPTS)

MAINTAINERCLEANFILES=\
Makefile.in \
$(TESTSUITE)

$(srcdir)/package.m4: $(top_srcdir)/configure.ac
	@{ \
	   echo '# Signature of the current package.' ; \
	   echo 'm4_define([AT_PACKAGE_NAME],      [$(PACKAGE_NAME)])' ; \
	   echo 'm4_define([AT_PACKAGE_TARNAME],   [$(PACKAGE_TARNAME)])' ; \
	   echo 'm4_define([AT_PACKAGE_VERSION],   [$(PACKAGE_VERSION)])' ; \
	   echo 'm4_define([AT_PACKAGE_STRING],    [$(PACKAGE_STRING)])' ; \
	   echo 'm4_define([AT_PACKAGE_BUGREPORT], [$(PACKAGE_BUGREPORT)])' ; \
	   echo 'm4_define([AT_PACKAGE_URL],       [$(PACKAGE_URL)])' ; \
	} >'$(srcdir)/package.m4'

TESTSUITE=$(srcdir)/testsuite

AUTOTEST=$(AUTOM4TE) --language=autotest

$(TESTSUITE): package.m4 $(TESTSUITE_AT)
	$(AUTOTEST) -I $(srcdir) testsuite.at -o $@.tmp
	mv $@.tmp $@

atconfig: $(top_builddir)/config.status
	cd $(top_builddir) && ./config.status tests/$@

check_SCRIPTS=\
bat \
nightly \
weekly \
OSAL_TestSem \
OSAL_TestThreadGroup

clean-local:
	test ! -f '$(TESTSUITE)' || $(SHELL) '$(TESTSUITE)' --clean

# Run the test suite locally.
check-local: atconfig atlocal $(TESTSUITE)
	$(SHELL) '$(TESTSUITE)' $(TESTSUITEFLAGS)

# Run the test suite on the *installed* tree.
installcheck-local: atconfig atlocal $(TESTSUITE)
	$(SHELL) '$(TESTSUITE)' AUTOTEST_PATH='$(bindir)' $(TESTSUITEFLAGS)

