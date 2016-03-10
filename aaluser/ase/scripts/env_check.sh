## Copyright(c) 2016, Intel Corporation
##
## Redistribution  and  use  in source  and  binary  forms,  with  or  without
## modification, are permitted provided that the following conditions are met:
##
## * Redistributions of  source code  must retain the  above copyright notice,
##   this list of conditions and the following disclaimer.
## * Redistributions in binary form must reproduce the above copyright notice,
##   this list of conditions and the following disclaimer in the documentation
##   and/or other materials provided with the distribution.
## * Neither the name  of Intel Corporation  nor the names of its contributors
##   may be used to  endorse or promote  products derived  from this  software
##   without specific prior written permission.
##
## THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
## AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  BUT NOT LIMITED TO,  THE
## IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
## ARE DISCLAIMED.  IN NO EVENT  SHALL THE COPYRIGHT OWNER  OR CONTRIBUTORS BE
## LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR
## CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT LIMITED  TO,  PROCUREMENT  OF
## SUBSTITUTE GOODS OR SERVICES;  LOSS OF USE,  DATA, OR PROFITS;  OR BUSINESS
## INTERRUPTION)  HOWEVER CAUSED  AND ON ANY THEORY  OF LIABILITY,  WHETHER IN
## CONTRACT,  STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE  OR OTHERWISE)
## ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,  EVEN IF ADVISED OF THE
## POSSIBILITY OF SUCH DAMAGE.

#!/bin/sh

uname=`uname -a`
os=`uname -s`
nodename=`uname -n`
kernel_rel=`uname -r`
kernel_ver=`uname -v`
arch=`uname -p`
distro=`lsb_release -a`
shm_testfile=`echo /dev/shm/$USER.ase_envcheck`

## Print header, and basic info
echo "############################################################"
echo "#                                                          #"
echo "#   Xeon (R) + FPGA Accelerator Abstraction Layer 5.0.3    #"
echo "#         AFU Simulation Environment (ASE)                 #"
echo "#                                                          #"
echo "############################################################"
echo "Checking Machine... "
echo "OS             = ${os} "
echo "Kernel Release = ${kernel_rel}"
echo "Kernel Version = ${kernel_ver}"
echo "Machine        = ${arch}"
echo "Distro         = "
echo "${distro}"
echo "-----------------------------------------------------------"

## If Machine is not 64-bit, flash message
if [ $os == "Linux" ]; then
    if [ $arch == "x86_64" ]; then
	echo "  [INFO] 64-bit Linux found"	
    else
	echo "  [WARN] 32-bit Linux found --- ASE works best on 64-bit Linux !"
    fi
else
    echo "  [WARN] Non-Linux distro found --- ASE is not supported on non-Linux platforms !"
fi
echo "-----------------------------------------------------------"

## Check shell environment
shell=`basename \`echo $SHELL\``
echo "  [INFO] SHELL identified as ${shell} (located `echo $SHELL`)"
if   [ $shell == "bash" ] ; then
    echo "  [INFO] SHELL ${shell} version : `echo $BASH_VERSION`"
elif [ $shell == "zsh" ] ; then
    echo "  [INFO] SHELL ${shell} version : `zsh --version`"
elif [ $shell == "tcsh" ] ; then
    echo "  [INFO] SHELL ${shell} version : `tcsh --version`"
elif [ $shell == "csh" ] ; then
    echo "  [INFO] SHELL ${shell} version : `csh --version`"
else
    echo "  [WARN] SHELL ${shell} is unknown !"
fi
echo "-----------------------------------------------------------"

## Check if /dev/shm is mounted, try writing then deleting a file for access check
if [ -d /dev/shm/ ]; then
    echo "  [INFO] /dev/shm is accessible ... testing further"
    touch $shm_testfile
    echo $USER >> $shm_testfile
    readback_shmfile=`cat $shm_testfile`
    if [ $readback_shmfile == $USER ] ; then
	echo "  [INFO]  SHM self-check completed successfully."
    else
	echo "  [WARN]  SHM self-check failed !"
    fi
    rm $shm_testfile
else
    echo "  [WARN] /dev/shm seems to be inaccessible ! "
    echo "  [WARN] ASE uses this location for data sharing between SW and simulator"
    echo "  [WARN] Please mount this location before proceeding...  see 'man shm_overview'"
fi

echo "-----------------------------------------------------------"

## GCC version check

echo "-----------------------------------------------------------"

## VCS version check

echo "-----------------------------------------------------------"

## Questasim version check

echo "-----------------------------------------------------------"

## Quartus version not available

echo "-----------------------------------------------------------"


