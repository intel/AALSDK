# set -v

./clean.csh

# gcc -g -o tstamp_test     tstamp_test.c ../../sw/tstamp_ops.c 

# gcc -g -o mq_test         mq_test.c     ../../sw/tstamp_ops.c ../../sw/mqueue_ops.c ../../sw/error_report.c ../../sw/ipc_mgmt_ops.c -I ../../sw/    -lrt
# gcc -g -o mq_test         mq_test.c        -lrt

# gcc -g -o shuffle \
    # -I ../../sw/  -I $VCS_HOME/include/ -D SIM_SIDE=1 \
    # shuffle.c ../../sw/randomness_control.c ../../sw/error_report.c 

# gcc -g -o server_sock     server_sock.c

# gcc -g -o client_sock     client_sock.c

# gcc -g -o large_shm       large_shm.c      -lrt

# gcc -g -o large_fileio large_fileio.c  

# gcc -g -Wall -o capcm_test capcm_test.c ../../sw/ase_ops.c ../../sw/capcm_ops.c ../../sw/dpi_ops.c ../../sw/error_report.c ../../sw/ipc_mgmt_ops.c ../../sw/linked_list_ops.c ../../sw/mqueue_ops.c ../../sw/tstamp_ops.c -lrt


# gcc -g -Wall -o div_test div_test.c

# gcc -g -Wall -o devzero devzero.c

# gcc -g -o cfg_parse cfg_parse.c ../../sw/error_report.c -D SIM_SIDE=1 -I /usr/synopsys/vcs-mx/G-2012.09-SP1/include/

# gcc -g -o reusable_seed reusable_seed.c ../../sw/error_report.c -lrt -lm -D SIM_SIDE=1 -I $VCS_HOME/include/

# gcc -g -o remote_start remote_start.c \
#    ../../sw/tstamp_ops.c ../../sw/ase_ops.c ../../sw/app_backend.c ../../sw/mqueue_ops.c ../../sw/error_report.c \
#     -lrt -lm 

# gcc -g -o read_file read_file.c

# gcc -g -o ase_mq_test ase_mq_test.c \
#     ../../sw/mqueue_ops.c \
#     ../../sw/error_report.c \
#     ../../sw/tstamp_ops.c \
#     -I ../../sw/ -lrt
# inotify_test.c

<<<<<<< HEAD
# gcc -g -o inotify_test inotify_test.c -I../../sw/ -lrt

gcc -g -o struct_typecast struct_typecast.c ../../sw/ase_ops.c ../../sw/error_report.c -I../../sw/ -lrt


=======
gcc -g -o inotify_test inotify_test.c -I../../sw/ -lrt
>>>>>>> 23c89f94d68c0f6761c0bfb4db3b91c9ba738146
