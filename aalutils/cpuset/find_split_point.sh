#
# Copyright(c) 2016, Intel Corporation
#
# Redistribution  and  use  in source  and  binary  forms,  with  or  without
# modification, are permitted provided that the following conditions are met:
#
# * Redistributions of  source code  must retain the  above copyright notice,
#   this list of conditions and the following disclaimer.
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   andor other materials provided with the distribution.
# * Neither the name  of Intel Corporation  nor the names of its contributors
#   may be used to  endorse or promote  products derived  from this  software
#   without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  BUT NOT LIMITED TO,  THE
# IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT  SHALL THE COPYRIGHT OWNER  OR CONTRIBUTORS BE
# LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR
# CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT LIMITED  TO,  PROCUREMENT  OF
# SUBSTITUTE GOODS OR SERVICES;  LOSS OF USE,  DATA, OR PROFITS;  OR BUSINESS
# INTERRUPTION)  HOWEVER CAUSED  AND ON ANY THEORY  OF LIABILITY,  WHETHER IN
# CONTRACT,  STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE  OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,  EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#****************************************************************************
# @file find_split_point.sh
# @brief Use to find first processor on second socket.
# @verbatim
# To use:  ./find_split_point.sh /proc/cpuinfo
# Examples: 
#./find_split_point.sh mycpuinfo (file simulating a 2 socket sytem, with a 14 core processor)
# 28
#./find_split_point.sh /proc/cpuinfo (done on a real one socket system)
# 0
#
#
# AUTHORS: Carl Ohgren, Intel Corporation
#
#
# HISTORY:  When doing PR, it is necessary to know what processors correspond
# to what sockets.  When doing core-idling, the cpus allowed list mask in
# /proc/pid#/status should only be changed for bits that correspond to 
# cpus for the socket for which the PR is applied.  The core idling algorithm
# takes both the socket number and the split point as input.  The source of the
# split is (will be) part of platform meta-data.  This shell provides a
# mechanism to set that value.
#
#
# WHEN:          WHO:     WHAT:
# 07/17/2016     CGO      Find first processor number on second socket.
#                         
#@endverbatim
#
#****************************************************************************

rm pro_x1
rm physid_x1
rm phys_pro_pairs
# Could hard code /proc/cpuinfo instead using $1.
cat "$1" | grep 'processor' > pro_x1
cat "$1" | grep 'physical id' > physid_x1
awk 'FNR==NR { a[FNR""] = $0; next} {print a[FNR""], $0 }' physid_x1 pro_x1 > phys_pro_pairs
split_point=`awk ' ($4 == 1) {print $7; exit }' phys_pro_pairs`
if [ "$split_point" != "" ]
then
    echo $split_point
else
    echo 0
fi
