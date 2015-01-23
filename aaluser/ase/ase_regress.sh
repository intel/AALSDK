#!/bin/bash

export ASE_WORKDIR=${PWD}   || echo ""
export BUILD_DIR=my_build

# cd ../${BUILD_DIR}/bin/ || exit 1
cd regression/apps/ || exit 1
# ./CCIDemo --target=ase
./nlb_test 32
./nlb_test 256
./nlb_test 64
./nlb_test 1024
