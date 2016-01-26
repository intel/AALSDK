#!/bin/sh

mcname=`uname -n`
if [ $mcname == "atp-ase1" ]
then 
    export MTI_HOME=/opt/mentor/questasim/
fi

if [ $mcname == "plxc15025" ]
then 
    export MTI_HOME=/p/eda/acd/mentor/questasim/6.6a/
fi

export PATH=${MTI_HOME}/linux_x86_64/:${PATH}
export MGLS_LICENSE_FILE="1717@fmylic38b.fm.intel.com:1717@fmylic38a.fm.intel.com:1717@fmylic16p.fm.intel.com:1717@fmylic7006.fm.intel.com"
export LM_LICENSE_FILE="1717@fmylic38b.fm.intel.com:1717@fmylic38a.fm.intel.com:1717@fmylic16p.fm.intel.com:${LM_LICENSE_FILE}"
export LM_PROJECT=APD
