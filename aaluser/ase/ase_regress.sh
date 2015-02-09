#!/bin/bash

export ASE_WORKDIR=${PWD}   || echo ""
export BUILD_DIR=my_build

# cd ../${BUILD_DIR}/bin/ || exit 1
# cd regression/apps/ || exit 1
# ./CCIDemo --target=ase
# ./nlb_test 32
# ./nlb_test 256
# ./nlb_test 64
# ./nlb_test 1024

# cd /nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aalsamples/cciapp
# ./cciapp --target=ase
# ./cciapp --target=ase
# ./cciapp --target=ase
# ./cciapp --target=ase

cd /nfs/pdx/disks/atp.06/user/rrsharma/aalsdk_repos/aalsdk/aalsamples/splapp2
splapp2 --target=ase
