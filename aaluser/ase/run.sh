#!/bin/sh

./work/ase_simv \
    -l run.log \
    -ucli -do ./regression/nlb_400/vcs_run.tcl \
    +CONFIG=${PWD}/ase.cfg \
    +SCRIPT=${PWD}/regression/nlb_400/ase_regress.sh

