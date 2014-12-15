# set -v

gcc -g -o nlb_test nlbv11_all_test.c \
   ../../sw/tstamp_ops.c ../../sw/ase_ops.c ../../sw/shm_ops.c ../../sw/mqueue_ops.c ../../sw/error_report.c \
    -lrt -lm 

# gcc -g -o spl_basic spl_basic.c \
#    ../../sw/tstamp_ops.c ../../sw/ase_ops.c ../../sw/shm_ops.c ../../sw/mqueue_ops.c ../../sw/error_report.c \
#     -lrt -lm 

# gcc -g -o spl_basic_mod spl_basic_mod.c \
#    ../../sw/tstamp_ops.c ../../sw/ase_ops.c ../../sw/shm_ops.c ../../sw/mqueue_ops.c ../../sw/error_report.c \
#     -lrt -lm 

gcc -g -o spl_shm spl_shm.c ../../sw/tstamp_ops.c ../../sw/ase_ops.c ../../sw/shm_ops.c ../../sw/mqueue_ops.c ../../sw/error_report.c -lrt -lm 

# gcc -g -o msg_create_test msg_create_test.c ../../sw/tstamp_ops.c ../../sw/ase_ops.c ../../sw/shm_ops.c ../../sw/mqueue_ops.c ../../sw/error_report.c -lrt -lm 

# gcc -g -o umsg_test umsg_test.c ../../sw/tstamp_ops.c ../../sw/ase_ops.c ../../sw/shm_ops.c ../../sw/mqueue_ops.c ../../sw/error_report.c -lrt -lm 

# gcc -g -o capcm_setting_test capcm_setting_test.c ../../sw/dpi_ops.c ../../sw/linked_list_ops.c ../../sw/mqueue_ops.c -lm -I /nfs/site/eda/tools/synopsys/vcsmx/I-2014.03-1/common//include/ -D SIM_SIDE=1
