#!/bin/bash

export ASE_WORKDIR=${PWD}   || echo ""
export BUILD_DIR=my_build
export LD_LIBRARY_PATH=$PWD/../../myinst/usr/local/lib/

cd ../../aalsamples/Hello_SPL_LB/
./helloSPLlb
