# set -v

gcc -g -o nlb_test \
    nlbv11_all_test.c \
    ../../sw/tstamp_ops.c ../../sw/ase_ops.c ../../sw/shm_ops.c \
    ../../sw/mqueue_ops.c ../../sw/error_report.c \
    -lrt -lm -I ../../sw/ \
    -D ASE_DEBUG


# gcc -g -o spl_basic spl_basic.c \
#    ../../sw/tstamp_ops.c ../../sw/ase_ops.c ../../sw/shm_ops.c ../../sw/mqueue_ops.c ../../sw/error_report.c \
#     -lrt -lm 

# gcc -g -o spl_basic_mod spl_basic_mod.c \
#    ../../sw/tstamp_ops.c ../../sw/ase_ops.c ../../sw/shm_ops.c ../../sw/mqueue_ops.c ../../sw/error_report.c \
#     -lrt -lm 

# gcc -g -o spl_shm spl_shm.c ../../sw/tstamp_ops.c ../../sw/ase_ops.c ../../sw/shm_ops.c ../../sw/mqueue_ops.c ../../sw/error_report.c -lrt -lm  -I ../../sw/

# gcc -g -o msg_create_test msg_create_test.c ../../sw/tstamp_ops.c ../../sw/ase_ops.c ../../sw/shm_ops.c ../../sw/mqueue_ops.c ../../sw/error_report.c -lrt -lm 

# gcc -g -o umsg_test umsg_test.c ../../sw/tstamp_ops.c ../../sw/ase_ops.c ../../sw/shm_ops.c ../../sw/mqueue_ops.c ../../sw/error_report.c -lrt -lm  -I ../../sw/ 

# gcc -g -o mq_test mq_test.c ../ase_debug_stub.c \
    # -D SIM_SIDE=1 -I $VCS_HOME/include/ -lrt -I ../../sw/ -lm -D ASE_DEBUG=1 \
    # ../../sw/mqueue_ops.c \
    # ../../sw/mem_model.c \
    # ../../sw/linked_list_ops.c \
    # ../../sw/ase_ops.c \
    # ../../sw/ipc_mgmt_ops.c \
    # ../../sw/error_report.c \
    # ../../sw/randomness_control.c \
    # ../../sw/tstamp_ops.c \
    # ../../sw/protocol_backend.c \

gcc -g -o myserver myserver.c -I ../../sw/

