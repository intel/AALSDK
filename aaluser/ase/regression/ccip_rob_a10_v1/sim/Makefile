all:
	vcs -sverilog ../rob_gram_sdp.v tb_rob_gram_sdp.sv -full64 -nc -timescale_override=10ns/10ns -debug_all \
	-v /nfs/site/eda/data/tools.6/altera/quartusprime/15.1.0.185/common/quartus//eda/sim_lib/altera_mf.v \
	-v /nfs/site/eda/data/tools.6/altera/quartusprime/15.1.0.185/common/quartus//eda/sim_lib/altera_lnsim.sv \
	-l run.log

sim:
	./simv -l sim.log -ucli -do run.tcl
