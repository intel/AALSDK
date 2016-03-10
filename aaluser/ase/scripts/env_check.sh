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



