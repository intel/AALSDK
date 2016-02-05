all:
	vcs -sverilog  -full64 -nc -timescale_override=10ns/10ns -debug_all \
	/home/rrsharma/xeon-fpga-src/aalsdk/aaluser/ase/regression/ccip_async_a10_v1/sim/tb_async_channel.sv \
	/home/rrsharma/xeon-fpga-src/aalsdk/aaluser/ase/regression/ccip_async_a10_v1/afifo_channel.sv \
	$(QUARTUS_HOME)/eda/sim_lib/altera_mf.v \
	$(QUARTUS_HOME)/eda/sim_lib/altera_lnsim.sv \
	-l run.log


# /home/rrsharma/xeon-fpga-src/aalsdk/aaluser/ase/hw/ccip_if_pkg.sv \



sim:
	./simv -l sim.log -ucli -do run.tcl

