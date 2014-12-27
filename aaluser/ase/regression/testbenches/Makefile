all:

latency_pipe:
	vcs -lca -full64 -nc ../../hw/latency_pipe.sv tb_latency_pipe.sv -sverilog +v2k -override_timescale=10ps/10ps -l comp.log

latency_scoreboard:
	vcs -j 4 -full64 -nc \
	 -Mupdate -sverilog -override_timescale=10ps/10ps +incdir+../../hw/ -debug_all +define+ASE_DEBUG \
	+vcs+loopreport +vcs+loopdetect \
	tb_latency_scoreboard.sv \
	../../hw/stream_checker.sv \
	../../hw/latency_scoreboard.sv \
	../../hw/ase_fifo.sv \
	../../hw/sdp_ram.sv

# +define+ASE_DEBUG \

ase_fifo:
	vcs -j 4 -full64 -nc  -sverilog -override_timescale=10ps/10ps \
	+incdir+../../hw/ -debug_all \
	../../hw/ase_fifo.sv \
	../../hw/sdp_ram.sv \
	tb_ase_fifo.sv

prng:
	vcs -j 4 -full64 -nc \
	 -Mupdate -sverilog -override_timescale=10ps/10ps +incdir+../../hw/ -debug_all +lint=all \
	prng.sv \
	../../hw/latency_scoreboard.sv \
	../../hw/ase_fifo.sv \
	../../hw/sdp_ram.sv


################################################
sim:
	./simv -l run.log -ucli -do run.tcl
#	dve -vpd inter.vpd -session session.tcl

################################################
clean:
	rm -rf *.vpd ucli.key DVEFiles/ DVEfiles/ *.log
	rm -rf simv* csrc/
