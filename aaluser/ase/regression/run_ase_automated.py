###################################################################
# Run ASE simulation
# Author : Rahul R Sharma 
# 
# A wrapper to run the ASE simulation in server-client fashion
#
# WARNING: Misuse of this script can forkbomb your system. Running
# this script absolves Intel Corporation of any responsiblity.
#
###################################################################

import os, re, commands, time

ASE_WORKDIR   = os.environ['PWD']
HWSIM_DIR     = ASE_WORKDIR
# HWSIM_BLD_CMD = "make"
HWSIM_BLD_CMD = "ls"

## 1
SWAPP_DIR     = str(ASE_WORKDIR) + "/regression/apps/"
SWAPP_BLD_CMD = "gcc -g -o app nlbv11_all_test.c ../../sw/tstamp_ops.c ../../sw/ase_ops.c ../../sw/shm_ops.c ../../sw/mqueue_ops.c ../../sw/error_report.c -lrt"
HWSIM_RUN_CMD = "make sim &" 
SWAPP_RUN_CMD = "./app"

## 2
# SWAPP_DIR     = str(ASE_WORKDIR) + "/../my_build/samples/splapp/"
# SWAPP_BLD_CMD = "ls"
# HWSIM_RUN_CMD = "make sim &" 
# SWAPP_RUN_CMD = "./splapp --target=ase"


# Main run function
if __name__ == "__main__":
    # Build HW/SW
    os.chdir (SWAPP_DIR)
    os.system (SWAPP_BLD_CMD)
    os.chdir (HWSIM_DIR)
    os.system (HWSIM_BLD_CMD)
    # Run simulation
    os.chdir (HWSIM_DIR)
    os.system (HWSIM_RUN_CMD)
    # Wait for simulation ready
    while 1:
        test_file = os.path.isfile(HWSIM_DIR + "/.ase_ready")
        if test_file == True:
            break
    # Run software
    os.chdir (SWAPP_DIR)
    os.environ['ASE_WORKDIR'] = ASE_WORKDIR
    os.system (SWAPP_RUN_CMD)
    print "Wrapper script exits"
