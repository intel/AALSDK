#!/bin/sh

mcname=`uname -n`

if [ $mcname == "atp-ase1" ] 
then
    export VCS_HOME=/usr/synopsys/vcs-mx/G-2012.09-SP1/
#    export VCS_HOME=/opt/synopsys/vcs-mx/I-2014.03-2/
    export PATH=${VCS_HOME}/bin:${PATH}
    export LD_LIBRARY_PATH=${VCS_HOME}/lib/:${LD_LIBRARY_PATH}
    export LM_LICENSE_FILE="1800@plxs0307.pdx.intel.com:1800@scylic09.sc.intel.com:1800@scylic17.sc.intel.com:1800@fmylic38c.fm.intel.com:1800@fmylic36b.fm.intel.com:1717@fmylic16p.fm.intel.com:1717@chlslic03.ch.intel.com:1717@fmylic06a.fm.intel.com:5219@plxs0305.pdx.intel.com:26586@fmylic24p.fm.intel.com:26586@fmylic25p.fm.intel.com:26586@fmylic03c.fm.intel.com:26586@fmylic36c.fm.intel.com:26586@ilic1020.iil.intel.com:26586@plxs0402.pdx.intel.com:26586@plxs0406.pdx.intel.com:26586@plxs0414.pdx.intel.com:26586@plxs0415.pdx.intel.com:26586@plxs0417.pdx.intel.com:2100@plxs0308.pdx.intel.com:2100@fmylic37a.fm.intel.com:2100@fmylic40a.fm.intel.com:2100@hdylic03.hd.intel.com:2100@chlslic06.ch.intel.com:26586@plxs0402.pdx.intel.com:26586@plxs0406.pdx.intel.com:26586@plxs0414.pdx.intel.com:26586@plxs0415.pdx.intel.com:26586@plxs0416.pdx.intel.com"

    # Print the variables modified
    echo "VCS_HOME        = " $VCS_HOME
    echo "PATH            = " $PATH
    echo "LM_LICENSE_FILE = " $LM_LICENSE_FILE
fi

