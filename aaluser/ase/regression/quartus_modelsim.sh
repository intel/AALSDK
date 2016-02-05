#!/bin/sh

export MTI_HOME=/opt/altera/15.1/modelsim_ae
# /opt/altera/15.1/modelsim_ae/linuxaloem/
export PATH=${MTI_HOME}/linuxaloem/:${PATH}
export MGLS_LICENSE_FILE="1717@fmylic38b.fm.intel.com:1717@fmylic38a.fm.intel.com:1717@fmylic16p.fm.intel.com"
export LM_LICENSE_FILE="1717@fmylic38b.fm.intel.com:1717@fmylic38a.fm.intel.com:1717@fmylic16p.fm.intel.com:${LM_LICENSE_FILE}"
export LM_PROJECT=APD
