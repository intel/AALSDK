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

# Base directory
ASE_BASEDIR = $(PWD)

# ASE switches (these tend to increase log sizes)
ASE_DEBUG ?= 1
ASE_DISABLE_LOGGER = 0

# Profile settings
ASE_PROFILE ?= 0

# Coverage settings (lcov in SW and VCS-CM in HW)
ASE_COVERAGE ?= 0
COV_DIR=$(ASE_BASEDIR)/coverage/

# Enable Altera gates library
GLS_SIM = 1
GLS_SAMPLE_LIB = $(QUARTUS_HOME)/eda/sim_lib/altera_primitives.v

# Timescale
TIMESCALE = 1ps/1ps

# Work directory
WORK = work

# ASE Source directory
ASE_SRCDIR = $(shell pwd)
ASE_WORKDIR = $(ASE_SRCDIR)/$(WORK)

# Configuration & regression file inputs
ASE_CONFIG ?= $(ASE_SRCDIR)/ase.cfg
ASE_SCRIPT ?= $(ASE_SRCDIR)/ase_regress.sh

# ASE Absolute max runtime - set to 12 hours
ASE_ABSOLUTE_TIMEOUT ?= 43200

#########################################################################
#                          Default options                              #
#########################################################################
## Choice of VCS or QUESTA ##
SIMULATOR?=VCS
CC=gcc

## RTL command
SNPS_COMMAND = $(shell command -v vcs)
MENT_COMMAND = $(shell command -v vsim)

## C Compiler options
CC_OPT = -g  -m64 -fPIC -D SIM_SIDE=1 -D SIMULATOR=$(SIMULATOR)
# CC_OPT+= -std=c11
CC_OPT+= -Wall -fstack-protector-all
ifeq ($(ASE_DEBUG), 1)
  CC_OPT+= -D ASE_DEBUG=1
endif
ifeq ($(SIMULATOR), VCS)
  CC_OPT+= -I $(VCS_HOME)/include/
endif
ifeq ($(SIMULATOR), QUESTA)
  CC_OPT+= -I $(MTI_HOME)/include/
endif
ifeq ($(ASE_COVERAGE), 1)
  CC_OPT+= -fprofile-arcs -ftest-coverage
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

# -lrt option is required in order to use shared memory /dev/shm,
# -message queues and pthreads
ASE_LD_SWITCHES = -lrt -lpthread -lstdc++
ifeq ($(ASE_COVERAGE), 1)
  ASE_LD_SWITCHES+= -lgcov
endif


#########################################################################
#                          VCS Build Switches                           #
#########################################################################
## VHDL compile
SNPS_VHDLAN_OPT = -nc -verbose -full64
SNPS_VHDLAN_OPT+= -work $(WORK)

## Verilog compile
SNPS_VLOGAN_OPT = -nc -V -full64 +v2k -sverilog +define+$(SIMULATOR)
SNPS_VLOGAN_OPT+= +incdir+$(DUT_INCDIR) +incdir+$(ASE_INCDIR)
SNPS_VLOGAN_OPT+= +librescan -work $(WORK) -override_timescale=$(TIMESCALE)
# SNPS_VLOGAN_OPT+= +define+VENDOR_ALTERA +define+TOOL_QUARTUS +define+NUM_AFUS=1 +define+NLB400_MODE_0
SNPS_VLOGAN_OPT+= +define+CCIP_IF_V0_1 +define+MPF_PLATFORM_BDX  +define+CCI_SIMULATION=1 +define+SIM_MODE=1
SNPS_VLOGAN_OPT+= +lint=all,noVCDE,noNS,NCEID,CAWM,TFIPC,IPDP,PCWM
ifeq ($(ASE_DISABLE_LOGGER), 1)
  SNPS_VLOGAN_OPT+= +define+ASE_DISABLE_LOGGER
endif
ifeq ($(ASE_DEBUG), 1)
  SNPS_VLOGAN_OPT+= +define+ASE_DEBUG +warn
endif
ifeq ($(GLS_SIM), 1)
  SNPS_VLOGAN_OPT+= $(QUARTUS_HOME)/eda/sim_lib/altera_primitives.v
  SNPS_VLOGAN_OPT+= $(QUARTUS_HOME)/eda/sim_lib/220model.v
  SNPS_VLOGAN_OPT+= $(QUARTUS_HOME)/eda/sim_lib/sgate.v
  SNPS_VLOGAN_OPT+= $(QUARTUS_HOME)/eda/sim_lib/altera_mf.v
  SNPS_VLOGAN_OPT+= $(QUARTUS_HOME)/eda/sim_lib/stratixv_atoms.v
  SNPS_VLOGAN_OPT+= $(QUARTUS_HOME)/eda/sim_lib/twentynm_atoms.v
  SNPS_VLOGAN_OPT+= $(QUARTUS_HOME)/eda/sim_lib/stratixv_hssi_atoms.v
  SNPS_VLOGAN_OPT+= $(QUARTUS_HOME)/eda/sim_lib/stratixv_pcie_hip_atoms.v
  SNPS_VLOGAN_OPT+= $(QUARTUS_HOME)/eda/sim_lib/altera_lnsim.sv
endif

## VCS elaboration
SNPS_VCS_OPT = -nc -V +vcs+lic+wait -full64 -debug_all -Mupdate -lca
SNPS_VCS_OPT+= -j 4
SNPS_VCS_OPT+= -l vcs_elab.log +vhdllib+$(WORK) -Mlib=$(WORK) +lint=TFIPC-L
SNPS_VCS_OPT+= -override_timescale=$(TIMESCALE) -o $(WORK)/ase_simv
## Debug switches
ifeq ($(ASE_DEBUG), 1)
  SNPS_VCS_OPT+=  -race
endif
## Coverage switches
ifeq ($(ASE_COVERAGE), 1)
  SNPS_VCS_OPT+= -cm_dir $(COV_DIR)/vcs_coverage -cm_name ase_cov
  SNPS_VCS_OPT+= -cm cond+fsm+branch -cm_tgl mda
  SNPS_VCS_OPT+= -cm_hier $(ASEVAL_GIT)/ase_coverage.cfg
endif
## Profile switches
ifeq ($(ASE_PROFILE), 1)
  SNPS_VCS_OPT+= +vcs+loopreport +vcs+loopdetect
  SNPS_VCS_OPT+= -simprofile +prof
endif
# SNPS_VCS_OPT+= -LDFLAGS="-m64"             #  !!! DO NOT EDIT !!!

## Simulation options
VCS_SIM_OPT = -ucli -do $(ASE_SRCDIR)/vcs_run.tcl
VCS_SIM_OPT+= -l run.log
ifeq ($(ASE_PROFILE), 1)
  VCS_SIM_OPT+= -cm cond+fsm+branch
endif


#########################################################################
#                         Questa Build Switches                         #
#########################################################################
## VHDL compile
MENT_VCOM_OPT = -nologo -work $(WORK)

## VLOG compile
# MENT_VLOG_OPT = -64
MENT_VLOG_OPT+= -nologo +librescan -work $(WORK) +define+$(SIMULATOR)
MENT_VLOG_OPT+= -dpiheader work/dpiheader.h
MENT_VLOG_OPT+= +incdir+$(ASE_INCDIR)+$(DUT_INCDIR)+$(WORK)
MENT_VLOG_OPT+= -sv  -timescale $(TIMESCALE) -l vlog.log
MENT_VLOG_OPT+= +define+VENDOR_ALTERA +define+TOOL_QUARTUS +define+NUM_AFUS=1 +define+NLB400_MODE_0
ifeq ($(ASE_DEBUG), 1)
  MENT_VLOG_OPT+= +define+ASE_DEBUG -hazards
endif
ifeq ($(GLS_SIM), 1)
  MENT_VLOG_OPT+= $(QUARTUS_HOME)/eda/sim_lib/altera_primitives.v
  MENT_VLOG_OPT+= $(QUARTUS_HOME)/eda/sim_lib/220model.v
  MENT_VLOG_OPT+= $(QUARTUS_HOME)/eda/sim_lib/sgate.v
  MENT_VLOG_OPT+= $(QUARTUS_HOME)/eda/sim_lib/altera_mf.v
  MENT_VLOG_OPT+= $(QUARTUS_HOME)/eda/sim_lib/stratixv_atoms.v
#  MENT_VLOG_OPT+= $(QUARTUS_HOME)/eda/sim_lib/synopsys/stratixv_hssi_atoms_ncrypt.v
  MENT_VLOG_OPT+= $(QUARTUS_HOME)/eda/sim_lib/stratixv_hssi_atoms.v
  MENT_VLOG_OPT+= $(QUARTUS_HOME)/eda/sim_lib/stratixv_pcie_hip_atoms.v
  MENT_VLOG_OPT+= $(QUARTUS_HOME)/eda/sim_lib/altera_lnsim.sv
endif

## VSIM elaboration, and run options
# MENT_VSIM_OPT = -64
MENT_VSIM_OPT+= -c -l run.log -dpioutoftheblue 1
MENT_VSIM_OPT+= -sv_lib $(ASE_SHOBJ_NAME) -do $(ASE_SRCDIR)/vsim_run.tcl
ifeq ($(ASE_DEBUG), 1)
  MENT_VLOG_OPT+= -hazards
endif
# -voptargs="+acc"

#########################################################################
#                         Valgrind options                              #
#########################################################################
VALGRIND_OPTS = --tool=memcheck
VALGRIND_OPTS+= -v --log-file=valgrind.log --error-limit=no
VALGRIND_OPTS+= --track-fds=yes --trace-children=yes
VALGRIND_OPTS+= --leak-check=full --track-origins=yes
VALGRIND_OPTS+= --show-reachable=yes
VALGRIND_OPTS+= --show-leak-kinds=definite,possible
VALGRIND_OPTS+= --undef-value-errors=yes


#########################################################################
#                          Clang settings                               #
#########################################################################
CLANG_VERSION=3.8

CLANG_OPT = -v -v -v -k -analyze-headers --use-cc=gcc -stats
CLANG_OPT+= -maxloop 100
CLANG_OPT+= -o $(PWD)/scanbuild/
CLANG_OPT+= --use-analyzer=/usr/bin/clang-$(CLANG_VERSION)
CLANG_OPT+= \
	-enable-checker  alpha.core.BoolAssignment \
	-enable-checker  alpha.core.CallAndMessageUnInitRefArg \
	-enable-checker  alpha.core.CastSize \
	-enable-checker  alpha.core.DynamicTypeChecker \
	-enable-checker  alpha.core.FixedAddr \
	-enable-checker  alpha.core.IdenticalExpr \
	-enable-checker  alpha.core.PointerArithm \
	-enable-checker  alpha.core.PointerSub \
	-enable-checker  alpha.core.SizeofPtr \
	-enable-checker  alpha.core.TestAfterDivZero \
	-enable-checker  alpha.cplusplus.VirtualCall \
	-enable-checker  alpha.security.ArrayBound \
	-enable-checker  alpha.security.ArrayBoundV2 \
	-enable-checker  alpha.security.MallocOverflow \
	-enable-checker  alpha.security.ReturnPtrRange \
	-enable-checker  alpha.security.taint.TaintPropagation \
	-enable-checker  alpha.unix.Chroot \
	-enable-checker  alpha.unix.PthreadLock \
	-enable-checker  alpha.unix.SimpleStream \
	-enable-checker  alpha.unix.Stream \
	-enable-checker  alpha.unix.cstring.BufferOverlap \
	-enable-checker  alpha.unix.cstring.NotNullTerminated \
	-enable-checker  alpha.unix.cstring.OutOfBounds \
	-enable-checker  core.CallAndMessage \
	-enable-checker  core.DivideZero \
	-enable-checker  core.DynamicTypePropagation \
	-enable-checker  core.NonNullParamChecker \
	-enable-checker  core.NullDereference \
	-enable-checker  core.StackAddressEscape \
	-enable-checker  core.UndefinedBinaryOperatorResult \
	-enable-checker  core.VLASize \
	-enable-checker  core.builtin.BuiltinFunctions \
	-enable-checker  core.builtin.NoReturnFunctions \
	-enable-checker  core.uninitialized.ArraySubscript \
	-enable-checker  core.uninitialized.Assign \
	-enable-checker  core.uninitialized.Branch \
	-enable-checker  core.uninitialized.CapturedBlockVariable \
	-enable-checker  core.uninitialized.UndefReturn \
	-enable-checker  cplusplus.NewDelete \
	-enable-checker  cplusplus.NewDeleteLeaks \
	-enable-checker  deadcode.DeadStores \
	-enable-checker  debug.ExprInspection \
	-enable-checker  debug.Stats \
	-enable-checker  debug.TaintTest \
	-enable-checker  debug.ViewCFG \
	-enable-checker  debug.ViewExplodedGraph \
	-enable-checker  llvm.Conventions \
	-enable-checker  nullability.NullPassedToNonnull \
	-enable-checker  nullability.NullReturnedFromNonnull \
	-enable-checker  nullability.NullableDereferenced \
	-enable-checker  nullability.NullablePassedToNonnull \
	-enable-checker  nullability.NullablePassedToNonnull \
	-enable-checker  optin.performance.Padding \
	-enable-checker  security.FloatLoopCounter \
	-enable-checker  security.insecureAPI.UncheckedReturn \
	-enable-checker  security.insecureAPI.getpw \
	-enable-checker  security.insecureAPI.gets \
	-enable-checker  security.insecureAPI.mkstemp \
	-enable-checker  security.insecureAPI.mktemp \
	-enable-checker  security.insecureAPI.rand \
	-enable-checker  security.insecureAPI.strcpy \
	-enable-checker  security.insecureAPI.vfork \
	-enable-checker  unix.API \
	-enable-checker  unix.Malloc \
	-enable-checker  unix.MallocSizeof \
	-enable-checker  unix.MismatchedDeallocator \
	-enable-checker  unix.Vfork \
	-enable-checker  unix.cstring.BadSizeArg \
	-enable-checker  unix.cstring.NullArg \


#########################################################################
#                            Build Targets                              #
#########################################################################
# Default
all: check build

# Check
check: header
# Ensure GCC is available
ifneq ($(CC), gcc)
	@echo "#                                                          #"
	@echo "# CC=$(CC) not supported                                   #"
	@echo "# Run 'make help' for more information                     #"
	@echo "#                                                          #"
	@echo "############################################################"
	exit 1
endif
# Echo simulator setting
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
# Check gate simulation libraries
ifeq ($(GLS_SIM), 1)
  ifndef QUARTUS_HOME
	@echo "** ERROR **: env(QUARTUS_HOME) has not been set."
	@echo "** ERROR **: GLS_SIM needs QUARTUS_HOME environment variable to be set."
    ifdef QUARTUS_ROOT
	@echo "** ERROR **: env(QUARTUS_ROOT) is set up. Copy its contents to env(QUARTUS_HOME)."
	@echo "** ERROR **: This makefile references libraries based on env(QUARTUS_HOME)."
    endif
  else
    ifeq ($(wildcard $(GLS_SAMPLE_LIB)),)
	@echo "** ERROR **: env(QUARTUS_HOME) not set up correctly."
	@echo "** ERROR **: Cannot find Quartus library files in known locations"
    endif
  endif
endif
# Check if some RTL simulator exists
ifeq ($(SIMULATOR), VCS)
  ifndef VCS_HOME
	@echo "**ERROR** : env(VCS_HOME) is not set ! svdpi.h cannot be found !"
  endif
  ifndef SNPS_COMMAND
	@echo "**ERROR** : Synopsys commands (vlogan, vcs) not found !"
  endif
else
  ifeq ($(SIMULATOR), QUESTA)
    ifndef MTI_HOME
	@echo "**ERROR** : env(MTI_HOME) is not set ! svdpi.h cannot be found !"
    endif
    ifndef MENT_COMMAND
	@echo "**ERROR** : Modelsim commands (vlog, vsim) not found !"
    endif
  else
     @echo "**ERROR**: Unknown RTL simulator tool in use --- this is unsupported !"
  endif
endif


# Print version number
header:
	@echo "############################################################"
	@echo "#                                                          #"
	@echo "#       Xeon(R) + FPGA Accelerator Abstraction Layer       #"
	@echo "#             AFU Simulation Environment (ASE)             #"
	@echo "#                                                          #"
	@echo "############################################################"


# Help information
help: header
	@echo "#                |                                         #"
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
	@echo "#                |                                         #"
	@echo "# ======================================================== #"
	@echo "# CONFIG         |   DESCRIPTION                           #"
	@echo "# -------------------------------------------------------- #"
	@echo "# ASE_CONFIG     | Directly input an ASE configuration     #"
	@echo "#                |   file path (ase.cfg)                   #"
	@echo "#                |                                         #"
	@echo "# ASE_SCRIPT     | Directly input an ASE regression file   #"
	@echo "#                |   path (ase_regress.sh, for ASE_MODE=4) #"
	@echo "#                |                                         #"
	@echo "# SIMULATOR      | Directly input a simulator brand        #"
	@echo "#                |   (select between 'VCS' or 'QUESTA')    #"
	@echo "#                                                          #"
	@echo "############################################################"


## Build ASE Software objects and shared library ##
sw_build:
	make header
	mkdir -p $(WORK)
	cd $(WORK) ; $(CC) $(CC_OPT) -c $(ASESW_FILE_LIST) || exit 1 ; cd -
	cd $(WORK) ; $(CC) -g -shared -o $(ASE_SHOBJ_SO) `ls *.o` $(ASE_LD_SWITCHES) ; cd -
	nm $(WORK)/$(ASE_SHOBJ_SO) > $(WORK)/$(ASE_SHOBJ_NAME).nm
ifeq ($(ASE_DEBUG), 1)
	objdump -S $(WORK)/$(ASE_SHOBJ_SO) > $(WORK)/$(ASE_SHOBJ_NAME).obj
endif


## VCS build template ##
vcs_build: sw_build
	@echo "############################################################"
	@echo "#                                                          #"
	@echo "#                VCS-GCC build initiated                   #"
	@echo "#                                                          #"
	@echo "############################################################"
	vlogan $(SNPS_VLOGAN_OPT) $(ASEHW_FILE_LIST) -l vlogan-ase.log
ifdef DUT_VHD_SRC_LIST
	vhdlan $(SNPS_VHDLAN_OPT) -f $(DUT_VHD_SRC_LIST)
endif
ifdef DUT_VLOG_SRC_LIST
	vlogan $(SNPS_VLOGAN_OPT) -f $(DUT_VLOG_SRC_LIST) -l vlogan-afu.log
endif
ifeq ($(CC), gcc)
	vcs $(SNPS_VCS_OPT) $(ASE_TOP) $(WORK)/$(ASE_SHOBJ_SO) $(ASE_LD_SWITCHES)
endif


## Questasim template ##
questa_build: sw_build
	@echo "############################################################"
	@echo "#                                                          #"
	@echo "#              QuestaSim-GCC build initiated               #"
	@echo "#                                                          #"
	@echo "############################################################"
	cd $(WORK) ; vlib $(WORK) ; vmap work $(WORK) ; cd -
	cd $(WORK) ; vlog $(MENT_VLOG_OPT) $(ASEHW_FILE_LIST) ; cd -
ifdef DUT_VHD_SRC_LIST
	cd $(WORK) ; vcom $(MENT_VCOM_OPT) -f $(DUT_VHD_SRC_LIST) ; cd -
endif
ifdef DUT_VLOG_SRC_LIST
	cd $(WORK) ; vlog $(MENT_VLOG_OPT) -f $(DUT_VLOG_SRC_LIST) ; cd -
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
	cd $(ASE_WORKDIR) ; /usr/bin/timeout $(ASE_ABSOLUTE_TIMEOUT) ./ase_simv $(VCS_SIM_OPT) +CONFIG=$(ASE_CONFIG) +SCRIPT=$(ASE_SCRIPT) ; cd -
  ifeq ($(ASE_COVERAGE), 1)
	cd $(COV_DIR) ; lcov --base-directory $(COV_DIR) --directory $(ASE_WORKDIR) --capture --output-file $(COV_DIR)/sim_coverage.info ; genhtml $(COV_DIR)/sim_coverage.info --output-directory $(COV_DIR)/sim_coverage ; cd -
	cd $(COV_DIR) ; urg -full64 -dir $(COV_DIR)/vcs_coverage.vdb ; cd -
  endif
else
  ifeq ($(SIMULATOR), QUESTA)
	cd $(ASE_WORKDIR) ; /usr/bin/timeout $(ASE_ABSOLUTE_TIMEOUT) vsim $(MENT_VSIM_OPT) +CONFIG=$(ASE_CONFIG) +SCRIPT=$(ASE_SCRIPT) $(ASE_TOP) ; cd -
  else	
	@echo "############################################################"
	@echo "#         SIMULATOR=$(SIMULATOR) not supported             #"
	@echo "############################################################"
  endif
endif

# Open Wave file
wave: check
ifeq ($(SIMULATOR), VCS)
	dve -vpd `find . -name inter.vpd` || dve -vpd `find . -name inter.vpd` -full64
else
  ifeq ($(SIMULATOR), QUESTA)
	vsim -view `find . -name vsim.wlf`
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
	rm -rf transcript *.log .ase_ipc_local ase_seed.txt
	rm -rf vsim.wlf *_smq __hdl_xmr.tab

distclean: header clean
	./distclean.sh


#########################################################################
#                            Internal only                              #
#########################################################################
NLB_CSRC = $(ASEVAL_GIT)/apps/nlb_lpbk1_test.c
NLB_CSRC+= ${ASE_SRCDIR}/sw/tstamp_ops.c
NLB_CSRC+= ${ASE_SRCDIR}/sw/ase_ops.c 
NLB_CSRC+= ${ASE_SRCDIR}/sw/app_backend.c 
NLB_CSRC+= ${ASE_SRCDIR}/sw/mqueue_ops.c 
NLB_CSRC+= ${ASE_SRCDIR}/sw/error_report.c 

# NLB peek-poke build
nlb-build:
	gcc -g -o nlb_test.out $(NLB_CSRC) -lrt -lm -lpthread -I $(ASE_SRCDIR)/sw/

# Source scan-build
scan-build:
	rm -rf scanbuild
	mkdir -p scanbuild
	scan-build-$(CLANG_VERSION) $(CLANG_OPT) make sw_build | tee scan-build-sim.log
#	scan-build-$(CLANG_VERSION) $(CLANG_OPT) make nlb-build | tee scan-build-app.log

# GUI call for coverage
cov_gui: coverage_gui

coverage_gui:
	chromium-browser  $(COV_DIR)/urgReport/dashboard.html &
	chromium-browser  $(COV_DIR)/lcov_coverage/index.html &
	chromium-browser  $(COV_DIR)/profileReport.html &
	dve -dir $(COV_DIR)/vcs_coverage.vdb/ -full64 &

# Valgrind run
valgrind_run:
	cd ./$(WORK)/; valgrind $(VALGRIND_OPTS) ./ase_simv $(VCS_SIM_OPT) +CONFIG=$(ASE_CONFIG) +SCRIPT=$(ASE_SCRIPT) ; cd -

# Test
test: all sim

# On long regression runs, dont create a run.log
long_sim:
	cd ./$(WORK)/; ./ase_simv -ucli -do $(ASE_SRCDIR)/vcs_run.tcl +CONFIG=$(ASE_CONFIG) +SCRIPT=$(ASE_SCRIPT) ; cd -

# GDB enabled sim run
gdb_sim:
	cd ./$(WORK)/; gdb -ex run --args ./ase_simv $(VCS_SIM_OPT) +CONFIG=$(ASE_CONFIG) +SCRIPT=$(ASE_SCRIPT) ; cd -
