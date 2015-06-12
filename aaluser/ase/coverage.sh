#!/bin/sh

echo "############################################################"
echo "#                                                          #"
echo "#                VCS-GCC build initiated                   #"
echo "#                                                          #"
echo "############################################################"
make sw_build
vlogan -nc -V -full64 +v2k -sverilog -l vlogan.log +incdir+/nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/regression/nlb.may2015+/nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/regression/nlb.may2015/include_files+/nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/regression/nlb.may2015/include_files/altr+/nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/regression/nlb.may2015/include_files/common+/nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/regression/nlb.may2015/include_files/xlnx+ +incdir+/nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/hw/+ +librescan -work work -override_timescale=1ns/1ns +define+ASE_DEBUG  -f vlog_files.list 
vlogan -nc -V -full64 +v2k -sverilog -l vlogan.log +incdir+/nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/regression/nlb.may2015+/nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/regression/nlb.may2015/include_files+/nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/regression/nlb.may2015/include_files/altr+/nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/regression/nlb.may2015/include_files/common+/nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/regression/nlb.may2015/include_files/xlnx+ +incdir+/nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/hw/+ +librescan -work work -override_timescale=1ns/1ns +define+ASE_DEBUG  /nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/hw/ase_fifo.sv /nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/hw/cci_emulator.sv /nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/hw/cci_sniffer.sv /nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/hw/counter.sv /nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/hw/latency_pipe.sv /nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/hw/latency_scoreboard.sv /nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/hw/sdp_ram.sv /nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/hw/stream_checker.sv  
vcs -nc -V +vcs+lic+wait -full64 -debug_all -Mupdate -lca -l vcs_elab.log +vhdllib+work -Mlib=work  -override_timescale=1ns/1ns -o work/ase_simv +vcs+loopreport +vcs+loopdetect  -cm line+cond+fsm+tgl+branch+assert -cm_tgl mda -cm_cond basic+allops -cm_dir work/coverage/ -cm_hier regression/nlb.may2015/cci_std_afu.sv work/ase_libs.so cci_emulator -lrt -lpthread -lstdc++

./work/ase_simv -l run.log -ucli -do vcs_run.tcl -cm line+fsm+cond+tgl+branch
urg -dir work/coverage.vdb/ -format both


