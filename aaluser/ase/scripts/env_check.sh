## Copyright(c) 2013-2016, Intel Corporation
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

## If Machine is not 64-bit, flash message
if [ $os == "Linux" ]; then
    if [ $arch == "x86_64" ]; then
	echo "64-bit Linux found"	
    else
	echo "32-bit Linux found --- ASE works best on 64-bit Linux !"
    fi
else
    echo "Non-Linux distro found --- ASE is not supported on non-Linux platforms !"
fi

## Check if /dev/shm is mounted
if mountpoint -q /dev/shm ; then
    echo "/dev/shm is mounted" 
else
    echo "/dev/shm is not mounted --- this is not preferable !"
    echo "in case of simulation crashes, temp files may be stored here, and may be un-cleanable"
    echo "Please contact your admin to make /dev/shm/ mount available"
fi

## GCC version check


## VCS version check


## Questasim version check


## Quartus version not available



