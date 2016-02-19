# #############################################################################
# Copyright(c) 2011-2016, Intel Corporation
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# * Redistributions of source code must retain the above copyright notice,
# this list of conditions and the following disclaimer.
# * Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and/or other materials provided with the distribution.
# * Neither the name of Intel Corporation nor the names of its contributors
# may be used to endorse or promote products derived from this software
# without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
# #############################################################################
#
# Module Info:
# Language   : System{Verilog} | C/C++
# Owner      : Rahul R Sharma
#              rahul.r.sharma@intel.com
#              Intel Corporation
#
# ASE environment build - Makefile
#
#########################################################################
# Provides a template for compiling ASE with RTL CAFU sources in VCS
# and Questasim
# For a full experience, roll your own Makefile
#
##########################################################################

include ase_sources.mk

## ASE Version
ASE_VERSION = 5.0.1

# ASE switches (these tend to increase log sizes)
ASE_DEBUG ?= 1
ASE_PROFILE_ENABLE ?= 1
ASE_DISABLE_LOGGER = 0

GLS_SIM = 1

# Timescale
TIMESCALE = 1ns/1ns

# ASE Source directory
ASE_SRCDIR = $(shell pwd)

#########################################################################
#                          Default options                              #
#########################################################################
## Choice of VCS or QUESTA ##
SIMULATOR?=VCS
CC=gcc

## Include directory
ifeq ($(SIMULATOR), VCS)
TOOL_DPI_INCDIR=$(VCS_HOME)
endif
ifeq ($(SIMULATOR), QUESTA)
TOOL_DPI_INCDIR=$(MTI_HOME)
endif

## C Compiler options
CC_OPT = -g -m64 -fPIC -D SIM_SIDE=1
CC_OPT+= -I $(TOOL_DPI_INCDIR)/include
CC_OPT+= -Wall
# -std=c99
# -D_POSIX_C_SOURCE=200809L
# -D_XOPEN_SOURCE=700
ifeq ($(ASE_DEBUG), 1)
CC_OPT+= -D ASE_DEBUG=1
# CC_OPT+= -pedantic
endif

## Print information ##
 $(info SIMULATOR=$(SIMULATOR))
 $(info CC=$(CC))


#########################################################################
#                          Build Variables                              #
#########################################################################
# Variables
ASE_SHOBJ_NAME = ase_libs
ASE_SHOBJ_SO = $(ASE_SHOBJ_NAME).so

# Work directory
WORK = work

# -lrt option is required in order to use shared memory /dev/shm,
# -message queues and pthreads
ASE_LD_SWITCHES= -lrt -lpthread -lstdc++


#########################################################################
#                          VCS Build Switches                           #
#########################################################################
## VHDL compile
SNPS_VHDLAN_OPT = -nc -verbose -full64
SNPS_VHDLAN_OPT+= -work $(WORK)

## Verilog compile
SNPS_VLOGAN_OPT = -nc -V -full64 +v2k -sverilog -l vlogan.log
SNPS_VLOGAN_OPT+= +incdir+$(DUT_INCDIR) +incdir+$(ASE_INCDIR)
SNPS_VLOGAN_OPT+= +librescan -work $(WORK) -override_timescale=$(TIMESCALE)
SNPS_VLOGAN_OPT+= +define+VENDOR_ALTERA +define+TOOL_QUARTUS +define+NUM_AFUS=1 +define+NLB400_MODE_0
SNPS_VLOGAN_OPT+= +define+CCIP_IF_V0_1 +define+USE_PLATFORM_CCIP  +define+CCI_SIMULATION=1 +define+SIM_MODE=1
ifeq ($(ASE_DEBUG), 1)
  ifeq ($(ASE_DISABLE_LOGGER), 1)
   SNPS_VLOGAN_OPT+= +define+ASE_DISABLE_LOGGER
  endif
  SNPS_VLOGAN_OPT+= +define+ASE_DEBUG +lint=TFIPC-L
endif
ifdef GLS_SIM
  SNPS_VLOGAN_OPT+= $(QUARTUS_HOME)/eda/sim_lib/altera_primitives.v
#  SNPS_VLOGAN_OPT+= $(QUARTUS_HOME)/eda/sim_lib/220model.v
  SNPS_VLOGAN_OPT+= $(QUARTUS_HOME)/eda/sim_lib/sgate.v
  SNPS_VLOGAN_OPT+= $(QUARTUS_HOME)/eda/sim_lib/altera_mf.v
  # SNPS_VLOGAN_OPT+= $(QUARTUS_HOME)/eda/sim_lib/synopsys/stratixv_atoms_ncrypt.v
  # SNPS_VLOGAN_OPT+= $(QUARTUS_HOME)/eda/sim_lib/stratixv_atoms.v
  # SNPS_VLOGAN_OPT+= $(QUARTUS_HOME)/eda/sim_lib/synopsys/stratixv_hssi_atoms_ncrypt.v
  # SNPS_VLOGAN_OPT+= $(QUARTUS_HOME)/eda/sim_lib/stratixv_hssi_atoms.v
  # SNPS_VLOGAN_OPT+= $(QUARTUS_HOME)/eda/sim_lib/synopsys/stratixv_pcie_hip_atoms_ncrypt.v
  # SNPS_VLOGAN_OPT+= $(QUARTUS_HOME)/eda/sim_lib/stratixv_pcie_hip_atoms.v
  SNPS_VLOGAN_OPT+= $(QUARTUS_HOME)/eda/sim_lib/altera_lnsim.sv
endif

## VCS elaboration
SNPS_VCS_OPT = -nc -V +vcs+lic+wait -full64 -debug_all -Mupdate -lca
SNPS_VCS_OPT+= -l vcs_elab.log +vhdllib+$(WORK) -Mlib=$(WORK)
SNPS_VCS_OPT+= -override_timescale=$(TIMESCALE) -o $(WORK)/ase_simv
ifeq ($(ASE_DEBUG), 1)
  ifeq ($(ASE_PROFILE_ENABLE), 1)
    SNPS_VCS_OPT+= +vcs+loopreport +vcs+loopdetect
    SNPS_VCS_OPT+= -cm line+cond+fsm+tgl+branch+assert -cm_tgl mda
    SNPS_VCS_OPT+= -cm_cond basic+allops -cm_dir $(WORK)/coverage/
    SNPS_VCS_OPT+= -simprofile +prof -race
  endif
# SNPS_VCS_OPT+= -LDFLAGS="-m64"             #  !!! DO NOT EDIT !!!
endif

## Simulation options
VCS_SIM_OPT = -l run.log -ucli -do $(ASE_SRCDIR)/vcs_run.tcl


#########################################################################
#                         Questa Build Switches                         #
#########################################################################
## VHDL compile
MENT_VCOM_OPT = -nologo -work $(WORK)

## VLOG compile
MENT_VLOG_OPT = -64 -nologo +librescan -work $(WORK)
MENT_VLOG_OPT+= +incdir+$(ASE_INCDIR)+$(DUT_INCDIR)
MENT_VLOG_OPT+= -sv  -timescale $(TIMESCALE) -l vlog.log
MENT_VLOG_OPT+= +define+VENDOR_ALTERA +define+TOOL_QUARTUS +define+NUM_AFUS=1 +define+NLB400_MODE_0

## VSIM elaboration, and run options
MENT_VSIM_OPT = -64 -c -l run.log
MENT_VSIM_OPT+= -sv_lib $(WORK)/$(ASE_SHOBJ_NAME) -do vsim_run.tcl

#########################################################################
#                         Valgring options                              #
#########################################################################
VALGRIND_OPTS = --tool=memcheck
VALGRIND_OPTS+= -v --log-file=valgrind.log --error-limit=no
VALGRIND_OPTS+= --track-fds=yes --trace-children=yes
VALGRIND_OPTS+= --leak-check=full --track-origins=yes
VALGRIND_OPTS+= --show-reachable=yes
# VALGRIND_OPTS+= --gen-suppressions=all
VALGRIND_OPTS+= --show-leak-kinds=definite,possible
VALGRIND_OPTS+= --undef-value-errors=yes


#########################################################################
#                          Clang settings                               #
#########################################################################
CLANG_VERSION=3.6

CLANG_OPT = -v -v -v -k -analyze-headers --use-cc=gcc -stats
CLANG_OPT+= -maxloop 100
CLANG_OPT+= -o $(PWD)/scanbuild/
CLANG_OPT+= --use-analyzer=/usr/bin/clang-$(CLANG_VERSION)
CLANG_OPT+= \
-enable-checker alpha.core.BoolAssignment \
-enable-checker alpha.core.CallAndMessageUnInitRefArg \
-enable-checker alpha.core.CastSize \
-enable-checker alpha.core.CastToStruct \
-enable-checker alpha.core.FixedAddr \
-enable-checker alpha.core.IdenticalExpr \
-enable-checker alpha.core.PointerArithm \
-enable-checker alpha.core.PointerSub \
-enable-checker alpha.core.SizeofPtr \
-enable-checker alpha.core.TestAfterDivZero \
-enable-checker alpha.cplusplus.VirtualCall \
-enable-checker alpha.deadcode.UnreachableCode \
-enable-checker alpha.osx.cocoa.Dealloc \
-enable-checker alpha.osx.cocoa.DirectIvarAssignment \
-enable-checker alpha.osx.cocoa.DirectIvarAssignmentForAnnotatedFunctions \
-enable-checker alpha.osx.cocoa.InstanceVariableInvalidation \
-enable-checker alpha.osx.cocoa.MissingInvalidationMethod \
-enable-checker alpha.security.ArrayBound \
-enable-checker alpha.security.ArrayBoundV2 \
-enable-checker alpha.security.MallocOverflow \
-enable-checker alpha.security.ReturnPtrRange \
-enable-checker alpha.security.taint.TaintPropagation \
-enable-checker alpha.unix.Chroot \
-enable-checker alpha.unix.MallocWithAnnotations \
-enable-checker alpha.unix.PthreadLock \
-enable-checker alpha.unix.SimpleStream \
-enable-checker alpha.unix.Stream \
-enable-checker alpha.unix.cstring.BufferOverlap \
-enable-checker alpha.unix.cstring.NotNullTerminated \
-enable-checker alpha.unix.cstring.OutOfBounds \
-enable-checker debug.ConfigDumper \
-enable-checker debug.DumpCFG \
-enable-checker debug.DumpCalls \
-enable-checker debug.DumpDominators \
-enable-checker debug.DumpLiveVars \
-enable-checker debug.DumpTraversal \
-enable-checker debug.ExprInspection \
-enable-checker debug.Stats \
-enable-checker debug.TaintTest \
-enable-checker debug.ViewCFG \
-enable-checker debug.ViewExplodedGraph \
-enable-checker llvm.Conventions \
-enable-checker osx.API \
-enable-checker osx.SecKeychainAPI \
-enable-checker osx.cocoa.AtSync \
-enable-checker osx.cocoa.ClassRelease \
-enable-checker osx.cocoa.IncompatibleMethodTypes \
-enable-checker osx.cocoa.Loops \
-enable-checker osx.cocoa.MissingSuperCall \
-enable-checker osx.cocoa.NSAutoreleasePool \
-enable-checker osx.cocoa.NSError \
-enable-checker osx.cocoa.NilArg \
-enable-checker osx.cocoa.NonNilReturnValue \
-enable-checker osx.cocoa.RetainCount \
-enable-checker osx.cocoa.SelfInit \
-enable-checker osx.cocoa.UnusedIvars \
-enable-checker osx.cocoa.VariadicMethodTypes \
-enable-checker osx.coreFoundation.CFError \
-enable-checker osx.coreFoundation.CFNumber \
-enable-checker osx.coreFoundation.CFRetainRelease \
-enable-checker osx.coreFoundation.containers.OutOfBounds \
-enable-checker osx.coreFoundation.containers.PointerSizedValues \
-enable-checker security.FloatLoopCounter \
-enable-checker security.insecureAPI.rand \
-enable-checker security.insecureAPI.strcpy \
-enable-checker debug.DumpCallGraph \
# -enable-checker debug.ViewCallGraph \


#########################################################################
#                            Build Targets                              #
#########################################################################
# Default
all: check build

# Check
check: header
ifneq ($(CC), gcc)
	@echo "#                                                          #"
	@echo "# CC=$(CC) not supported                                   #"
	@echo "# Run 'make help' for more information                     #"
	@echo "#                                                          #"
	@echo "############################################################"
	exit 1
endif
ifneq ($(SIMULATOR), VCS)
  ifneq ($(SIMULATOR), QUESTA)
	@echo "#                                                          #"
	@echo "# SIMULATOR=$(SIMULATOR) not supported                     #"
	@echo "# Run 'make help' for more information                     #"
	@echo "#                                                          #"
	@echo "############################################################"
	exit 1
  endif
endif


# Print version number
header:
	@echo "############################################################"
	@echo "#                                                          #"
	@echo "# Intel Xeon+FPGA BDX-P AFU Simulation Environment $(ASE_VERSION)   #"
	@echo "#                                                          #"
	@echo "############################################################"


# Help information
help: header
	@echo "#                                                          #"
	@echo "#  COMMAND       |   DESCRIPTION                           #"
	@echo "# -------------------------------------------------------- #"
	@echo "# make           | Build the HW Model using RTL supplied   #"
	@echo "#                |                                         #"
	@echo "# make sim       | Run simulator                           #"
	@echo "#                | - ASE can be run in one of 4 modes set  #"
	@echo "#                |   in ase.cfg                            #"
	@echo "#                | - A regression mode can be enabled by   #"
	@echo "#                |   writing ASE_MODE = 4 in ase.cfg and   #"
	@echo "#                |   supplying an ase_regress.sh script    #"
	@echo "#                |                                         #"
	@echo "# make wave      | Open the waveform (if created)          #"
	@echo "#                | To be run after simulation completes    #"
	@echo "#                |                                         #"
	@echo "# make clean     | Clean simulation files                  #"
	@echo "#                |                                         #"
	@echo "# make distclean | Clean ASE sub-distribution              #"
	@echo "#                                                          #"
	@echo "############################################################"


## Build ASE Software objects and shared library ##
sw_build:
	mkdir -p $(WORK)
	cd $(WORK) ; $(CC) $(CC_OPT) -c $(ASESW_FILE_LIST) || exit 1 ; cd -
	cd $(WORK) ; $(CC) -g -shared -o $(ASE_SHOBJ_SO) `ls *.o` $(ASE_LD_SWITCHES) ; cd -
	nm $(WORK)/$(ASE_SHOBJ_SO) > $(WORK)/$(ASE_SHOBJ_NAME).nm

## VCS build template ##
vcs_build:
	@echo "############################################################"
	@echo "#                                                          #"
	@echo "#                VCS-GCC build initiated                   #"
	@echo "#                                                          #"
	@echo "############################################################"
	make sw_build
	vlogan $(SNPS_VLOGAN_OPT) $(ASEHW_FILE_LIST)
ifdef DUT_VHD_SRC_LIST
	vhdlan $(SNPS_VHDLAN_OPT) -f $(DUT_VHD_SRC_LIST)
endif
ifdef DUT_VLOG_SRC_LIST
	vlogan $(SNPS_VLOGAN_OPT) -f $(DUT_VLOG_SRC_LIST)
endif
ifeq ($(CC), gcc)
	vcs $(SNPS_VCS_OPT) $(ASE_TOP) $(WORK)/$(ASE_SHOBJ_SO) $(ASE_LD_SWITCHES)
endif


## Questasim template ##
questa_build:
	@echo "############################################################"
	@echo "#                                                          #"
	@echo "#              QuestaSim-GCC build initiated               #"
	@echo "#                                                          #"
	@echo "############################################################"
	vlib $(WORK)
	vmap work $(WORK)
	make sw_build
	vlog $(MENT_VLOG_OPT) $(ASEHW_FILE_LIST)
ifdef DUT_VHD_SRC_LIST
	vcom $(MENT_VCOM_OPT) -f $(DUT_VHD_SRC_LIST)
endif
ifdef DUT_VLOG_SRC_LIST
	vlog $(MENT_VLOG_OPT) -f $(DUT_VLOG_SRC_LIST)
endif


## Build Simulator objects ##
build:
ifeq ($(SIMULATOR), VCS)
	make vcs_build
else
  ifeq ($(SIMULATOR), QUESTA)
	make questa_build
  else
	@echo "############################################################"
	@echo "#         SIMULATOR=$(SIMULATOR) not supported             #"
	@echo "############################################################"
  endif
endif

## Run ASE Simulator ##
sim: check
ifeq ($(SIMULATOR), VCS)
	cd ./$(WORK)/; ./ase_simv $(VCS_SIM_OPT) +CONFIG=$(ASE_SRCDIR)/ase.cfg +SCRIPT=$(ASE_SRCDIR)/ase_regress.sh ; cd -
else
  ifeq ($(SIMULATOR), QUESTA)
	vsim $(MENT_VSIM_OPT) $(ASE_TOP)
  else
	@echo "############################################################"
	@echo "#         SIMULATOR=$(SIMULATOR) not supported             #"
	@echo "############################################################"
  endif
endif
#	python scripts/ipc_clean.py

# Open Wave file
wave: check
ifeq ($(SIMULATOR), VCS)
	dve -vpd `find . -name inter.vpd` || dve -vpd `find . -name inter.vpd` -full64
else
  ifeq ($(SIMULATOR), QUESTA)
	vsim -view vsim.wlf
  endif
endif


#########################################################################
#                            Clean all                                  #
#########################################################################
clean: header
	rm -rf work/ *.log *.tsv AN.DB/ || echo "make: Didn't clean up work"
	rm -rf csrc/ vc_hdrs.h .vlogansetup.* *.vpd
	rm -rf transcript modelsim.ini vsim.wlf ucli.key vsim_stacktrace.vstf
	rm -rf profile* simprofile* scanbuild/ DVEfiles/ csrc/
	rm -rf .ase_* *.o ase_seed.txt warnings.txt

distclean: header clean
	./distclean.sh


#########################################################################
#                            Internal only                              #
#########################################################################
# Source scan-build
scan-build:
	mkdir -p scanbuild
	scan-build-$(CLANG_VERSION) $(CLANG_OPT) make sw_build | tee scan-build.log

# Coverage
coverage:
	urg -dir ./work/coverage.vdb
#	firefox urgReport/dashboard.html &

# Valgrind run
valgrind_run:
	cd ./$(WORK)/; valgrind $(VALGRIND_OPTS) ./ase_simv $(VCS_SIM_OPT) +CONFIG=$(ASE_SRCDIR)/ase.cfg +SCRIPT=$(ASE_SRCDIR)/ase_regress.sh ; cd -

# Test
test: all sim

# On long regression runs, dont create a run.log
long_sim:
	cd ./$(WORK)/; ./ase_simv -ucli -do $(ASE_SRCDIR)/vcs_run.tcl +CONFIG=$(ASE_SRCDIR)/ase.cfg +SCRIPT=$(ASE_SRCDIR)/ase_regress.sh ; cd -


