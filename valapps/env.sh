#!/bin/bash +x
# Please note:  This is inactive template code that can and should only be run manually, after an 
# appropriate change to the path varialbes.

export LD_LIBRARY_PATH="/home/lab/workspace/rpan1/lib/lib"
export PATH="/home/lab/workspace/rpan1/lib/bin:$PATH"

export ToolPATH="/home/lab/workspace/rpan1/lib/bin"
export NumSocket=1 

# Changing this value from 2 to 1 to reflect the reality that we are only testing on single socket 
# systems right now.  And, to avoid potential for doubt when using the script without a clear 
# understanding of the implications during the migration process.  This will prevent an accidental 
# set to 2 sockets that might get picked up during a sanity check, running old code.

export SystemConfig="native"
export SampleAppPATH="/home/lab/workspace/rpan1/release/6.2.1-RC0/Samples/"
export RTL="/home/lab/workspace/rpan1/rtl_rp/skx_pr_afu.rbf"
export RTLNLB0="/home/lab/workspace/rpan1/rtl_release/101516_skxp_622_pr_2_b447_sd00_skxnlb400m0.rbf"
