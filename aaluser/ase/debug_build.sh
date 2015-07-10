#!/bin/sh

make clean

mkdir -p work
cd work ; gcc -g -m64 -fPIC -D SIM_SIDE=1 -I /p/atp/tools/synopsys/vcs/I-2014.03//include -Wall -std=c99 -D_XOPEN_SOURCE=700 -c /nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/sw/ase_ops.c /nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/sw/capcm_ops.c /nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/sw/error_report.c /nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/sw/ipc_mgmt_ops.c /nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/sw/linked_list_ops.c /nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/sw/mem_model.c /nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/sw/mqueue_ops.c /nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/sw/protocol_backend.c /nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/sw/randomness_control.c /nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/sw/shm_ops.c /nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/sw/tstamp_ops.c  ; cd - 
cd work ; gcc -g -shared -o ase_libs.so `ls *.o` -lrt -lpthread -lstdc++ ; cd -
nm work/ase_libs.so > work/ase_libs.nm

echo "############################################################"
echo "#                                                          #"
echo "#                VCS-GCC build initiated                   #"
echo "#                                                          #"
echo "############################################################"
make sw_build
vlogan -nc -V -full64 +v2k -sverilog -l vlogan.log +incdir+/nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/regression/nlb.may2015+/nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/regression/nlb.may2015/include_files+/nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/regression/nlb.may2015/include_files/altr+/nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/regression/nlb.may2015/include_files/common+/nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/regression/nlb.may2015/include_files/xlnx+ +incdir+/nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/hw/+ +librescan -work work -override_timescale=1ns/1ns -f vlog_files.list 
vlogan -nc -V -full64 +v2k -sverilog -l vlogan.log +incdir+/nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/regression/nlb.may2015+/nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/regression/nlb.may2015/include_files+/nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/regression/nlb.may2015/include_files/altr+/nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/regression/nlb.may2015/include_files/common+/nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/regression/nlb.may2015/include_files/xlnx+ +incdir+/nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/hw/+ +librescan -work work -override_timescale=1ns/1ns /nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/hw/ase_fifo.sv /nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/hw/cci_emulator.sv /nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/hw/cci_sniffer.sv /nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/hw/counter.sv /nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/hw/latency_pipe.sv /nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/hw/latency_scoreboard.sv /nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/hw/sdp_ram.sv /nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/hw/stream_checker.sv /nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/hw/cci_logger.sv /nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/hw/ase_top.sv
vcs -nc -V +vcs+lic+wait -full64 -debug_all -Mupdate -lca -l vcs_elab.log +vhdllib+work -Mlib=work  -override_timescale=1ns/1ns -o work/ase_simv work/ase_libs.so ase_top -lrt -lpthread -lstdc++ -simprofile 


echo "############################################################"
echo "#                                                          #"
echo "# Intel QuickAssist FPGA AFU Simulation Environment 4.1.7    #"
echo "#                                                          #"
echo "############################################################"
./work/ase_simv -l run.log -ucli -do vcs_run.tcl
python scripts/ipc_clean.py
