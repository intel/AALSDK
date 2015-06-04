#!/bin/bash

export ASE_WORKDIR=${PWD}   || echo ""
export BUILD_DIR=my_build
export LD_LIBRARY_PATH=$PWD/../../myinst/usr/local/lib/

# cd ../../aalsamples/cciapp
# ./cciapp --target=ase
# ./cciapp --target=ase
# ./cciapp --target=ase
# ./cciapp --target=ase

# cd ../../aalsamples/UMsgTest_ASE
# ./UMsgTest --target=ase --notice_type=2

# cd ../../aalsamples/Histogram_ASE
# ./Histogram --target=ase

# cd /nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/regression/apps/
# ./nlb_test 128
# ./nlb_test 16
# ./nlb_test 8192
# ./nlb_test 32768
# ./nlb_test 1024

cd /nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aaluser/ase/regression/apps
./umsg_test

