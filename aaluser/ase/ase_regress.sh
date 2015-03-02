#!/bin/bash

export ASE_WORKDIR=${PWD}   || echo ""
export BUILD_DIR=my_build

cd ../../aalsamples/cciapp
./cciapp --target=ase
./cciapp --target=ase
./cciapp --target=ase
./cciapp --target=ase
