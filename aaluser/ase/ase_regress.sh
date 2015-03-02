#!/bin/bash

export ASE_WORKDIR=${PWD}   || echo ""
export BUILD_DIR=my_build
export LD_LIBRARY_PATH=$PWD/../../myinst/usr/local/lib/

cd ../../aalsamples/cciapp
./cciapp --target=ase
./cciapp --target=ase
./cciapp --target=ase
./cciapp --target=ase
