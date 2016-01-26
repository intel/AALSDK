channel:
	vcs -j 4 -full64 -nc -l comp.log \
	-sverilog -override_timescale=10ps/10ps \
	+incdir+../../hw/ -debug_all \
	+vcs+loopreport +vcs+loopdetect \
	-cm line+cond+fsm+tgl+branch+assert \
	-cm_tgl mda -cm_cond basic+allops -cm_dir coverage \
	+define+ASE_DEBUG \
	../../hw/ase_pkg.sv \
	../../hw/stream_checker.sv \
	../../hw/outoforder_wrf_channel.sv \
	tb_channel.sv \

	# ../../hw/ase_fifo.sv \
	# ../../hw/sdp_ram.sv \


latency_pipe:
	vcs -lca -full64 -nc ../../hw/latency_pipe.sv tb_latency_pipe.sv -sverilog +v2k -override_timescale=10ps/10ps -l comp.log


fifo:
	vcs -j 4 -full64 -nc  -sverilog -override_timescale=10ps/10ps +define+ASE_DEBUG \
	+incdir+../../hw/ -debug_all \
	../../hw/ase_fifo.sv \
	../../hw/sdp_ram.sv \
	tb_ase_fifo.sv

ase_svfifo:
	vcs -j 4 -full64 -nc  -sverilog -override_timescale=10ps/10ps \
	+incdir+../../hw/ -debug_all \
	../../hw/ase_svfifo.sv \
	tb_ase_fifo.sv

prng:
	vcs -j 4 -full64 -nc \
	 -Mupdate -sverilog -override_timescale=10ps/10ps +incdir+../../hw/ -debug_all +lint=all \
	prng.sv \
	../../hw/latency_scoreboard.sv \
	../../hw/ase_fifo.sv \
	../../hw/sdp_ram.sv

string:
	vcs -full64 -nc -sverilog ../../hw/ase_pkg.sv ../../hw/ccip_logger.sv string_test.sv -debug_all +incdir+../../hw/


################################################
all: ase_svfifo

sim:
	./simv -l run.log -ucli -do run.tcl  -cm line+fsm+cond+tgl+branch
#	urg -dir work/coverage.vdb -format both
#	dve -vpd inter.vpd -session session.tcl

################################################
clean:
	rm -rf *.vpd ucli.key DVEFiles/ DVEfiles/ *.log
	rm -rf simv* csrc/
