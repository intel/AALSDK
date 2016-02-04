#!/bin/sh
ASE_DIR=/proj/vssad/local/x86_64_linux30/pkgs/Intel_QAFPGA/bdx-src-ase
MPF_DIR=../cci_mpf
DESIGN=/proj/vssad/local/x86_64_linux30/pkgs/Intel_QAFPGA/bdx-src-hw/RTL/bdx_fpga/design

export ASE_DIR
vcs -V -o nlb_mpf_400_hw.exe +v2k +libext+.v \
    +define+TOP=ase_top -top ase_top \
    \
    +define+CCIP_IF_V0_1 \
    +define+USE_PLATFORM_CCIP \
    +define+CCI_SIMULATION=1 \
    +define+SIM_MODE=1 \
    +define+VENDOR_ALTERA \
    +define+TOOL_QUARTUS \
    +define+NUM_AFUS=1 \
    +define+NLB400_MODE_0 \
    \
    +lint=all,noVCDE -assert svaext -full64 -sverilog +librescan +libext+.sv \
    -LDFLAGS -lstdc++ -LDFLAGS -lpthread -LDFLAGS -lrt \
    -override_timescale=10ps/10ps \
    +incdir+../nlb_mpf_400/include_files/common+${ASE_DIR}/hw+${MPF_DIR}+${MPF_DIR}/cci-if+${MPF_DIR}/cci-mpf-if+${MPF_DIR}/cci-mpf-prims+${MPF_DIR}/cci-mpf-shims \
    \
    -CFLAGS -DSIM_SIDE -CFLAGS -I${ASE_DIR}/sw \
    \
    ${ASE_DIR}/sw/ase_ops.c \
    ${ASE_DIR}/sw/error_report.c \
    ${ASE_DIR}/sw/ipc_mgmt_ops.c \
    ${ASE_DIR}/sw/linked_list_ops.c \
    ${ASE_DIR}/sw/mem_model.c \
    ${ASE_DIR}/sw/mqueue_ops.c \
    ${ASE_DIR}/sw/protocol_backend.c \
    ${ASE_DIR}/sw/randomness_control.c \
    ${ASE_DIR}/sw/tstamp_ops.c \
    \
    ${ASE_DIR}/hw/ase_pkg.sv \
    ${MPF_DIR}/cci-if/ccis_if_pkg.sv \
    ${MPF_DIR}/cci-if/ccis_if_funcs_pkg.sv \
    ${MPF_DIR}/cci-if/ccip_if_pkg.sv \
    ${MPF_DIR}/cci-if/ccip_if_funcs_pkg.sv \
    ${MPF_DIR}/cci-mpf-if/cci_mpf_if_pkg.sv \
    ${MPF_DIR}/cci-if/cci_csr_if_pkg.sv \
    ${MPF_DIR}/cci-if/ccip_feature_list_pkg.sv \
    ${MPF_DIR}/cci_mpf_csrs.h \
    ${MPF_DIR}/cci_mpf_csrs_pkg.sv \
    ${MPF_DIR}/cci-mpf-shims/cci_mpf_shim_buffer_afu.sv \
    ${MPF_DIR}/cci-mpf-prims/cci_mpf_prim_scoreboard.sv \
    ${MPF_DIR}/cci-mpf-prims/cci_mpf_prim_scoreboard_obuf.sv \
    ${MPF_DIR}/cci-mpf-prims/cci_mpf_prim_filter_cam.sv \
    ${MPF_DIR}/cci-mpf-shims/cci_mpf_shim_vtp.sv \
    ${MPF_DIR}/cci-mpf-shims/cci_mpf_shim_buffer_lockstep_afu.sv \
    ${MPF_DIR}/cci-mpf-prims/cci_mpf_prim_fifo1.sv \
    ${MPF_DIR}/cci-mpf-shims/cci_mpf_shim_rsp_order.sv \
    ${MPF_DIR}/cci-mpf-if/ccip_wires_to_mpf.sv \
    ${MPF_DIR}/cci-mpf-prims/cci_mpf_prim_lru_pseudo.sv \
    ${MPF_DIR}/cci-mpf-shims/cci_mpf_shim_wro.sv \
    ${MPF_DIR}/cci-mpf-shims/cci_mpf_shim_canonicalize_to_fiu.sv \
    ${MPF_DIR}/cci-mpf-shims/cci_mpf_shim_null.sv \
    ${MPF_DIR}/cci-mpf-shims/cci_mpf_shim_buffer_fiu.sv \
    ${MPF_DIR}/cci-mpf-shims/cci_mpf_shim_csr.sv \
    ${MPF_DIR}/cci-mpf-prims/cci_mpf_prim_fifo_lutram.sv \
    ${MPF_DIR}/cci-mpf-prims/cci_mpf_prim_fifo2.sv \
    ${MPF_DIR}/cci_mpf.sv \
    ${MPF_DIR}/cci-mpf-shims/cci_mpf_shim_mux.sv \
    ${MPF_DIR}/cci-mpf-prims/cci_mpf_prim_dualport_ram.sv \
    ${MPF_DIR}/cci-mpf-if/ccis_wires_to_mpf.sv \
    ${MPF_DIR}/cci-mpf-prims/cci_mpf_prim_heap.sv \
    ${MPF_DIR}/cci-mpf-prims/cci_mpf_prim_filter_counting.sv \
    \
    ${ASE_DIR}/hw/ase_fifo.sv \
    ${ASE_DIR}/hw/ase_pkg.sv \
    ${ASE_DIR}/hw/ase_top.sv \
    ${ASE_DIR}/hw/ccip_emulator.sv \
    ${ASE_DIR}/hw/ccip_logger.sv \
    ${ASE_DIR}/hw/ccip_sniffer.sv \
    ${ASE_DIR}/hw/counter.sv \
    ${ASE_DIR}/hw/latency_pipe.sv \
    ${ASE_DIR}/hw/outoforder_wrf_channel.sv \
    ${ASE_DIR}/hw/sdp_ram.sv \
    ${ASE_DIR}/hw/stream_checker.sv \
    \
    ${DESIGN}/qpi/common/memory/gram_sdp.v \
    ${DESIGN}/ccip_fabric/common/gram_sdp_2clks.v \
    ${DESIGN}/ccip_fabric/common/sbv_gfifo_v2.v \
    ${DESIGN}/ccip_fabric/common/sync_C0Tx_fifo.v \
    ${DESIGN}/ccip_fabric/common/sync_C1Tx_fifo.v \
    \
    ../nlb_mpf_400/ccip_std_afu.sv \
    ../nlb_mpf_400/nlb_lpbk.sv \
    ../nlb_mpf_400/requestor.sv \
    ../nlb_mpf_400/arbiter.v \
    ../nlb_mpf_400/nlb_csr.v \
    ../nlb_mpf_400/nlb_gfifo.v \
    ../nlb_mpf_400/nlb_gram_sdp.v \
    ../nlb_mpf_400/nlb_sbv_gfifo.v \
    ../nlb_mpf_400/test_lpbk1.v \
    ../nlb_mpf_400/test_lpbk2.v \
    ../nlb_mpf_400/test_lpbk3.v \
    ../nlb_mpf_400/test_rdwr.v \
    ../nlb_mpf_400/test_sw1.v
