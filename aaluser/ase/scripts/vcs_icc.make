include "ase_sources.mk"

all:
	vcs -sverilog +v2k +incdir+$(DUT_INCDIR) +incdir+$(ASE_INCDIR) $(SNPS_VCS_OPT) $(WORK)/$(ASE_SHOBJ_SO) $(ASE_LD_SWITCHES) -f $(DUT_VLOG_SRC_LIST) $(ASEHW_FILE_LIST)
