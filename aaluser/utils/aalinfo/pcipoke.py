#!/usr/bin/python
"""64 bit MMIO read from PCI(e) BARs"""
# Copyright(c) 2015-2016, Intel Corporation
#
# Redistribution  and  use  in source  and  binary  forms,  with  or  without
# modification, are permitted provided that the following conditions are met:
#
# * Redistributions of  source code  must retain the  above copyright notice,
#   this list of conditions and the following disclaimer.
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
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
# @file pcipoke.py
# @brief Script to perform 64bit MMIO read on PCI resources.
# @ingroup
# @verbatim
# Accelerator Abstraction Layer Sample Application
#
#    This application is for example purposes only.
#    It is not intended to represent a model for developing commercially-deployable applications.
#    It is designed to show working examples of the AAL programming model and APIs.
#
# AUTHORS: Enno Luebbers, Intel Corporation
#
# HISTORY:
# WHEN:          WHO:     WHAT:
# 8/09/2016      EL       Initial version.
#****************************************************************************
import mmap, sys, binascii

MAPSIZE = mmap.PAGESIZE
MAPMASK = MAPSIZE-1

if __name__ == "__main__":

   if len(sys.argv) < 4:
      sys.stderr.write("USAGE: pcipoke <resource> <offset> <value>\n")
      sys.exit(-1)

   address = int(sys.argv[2], 0)
   data_param = int(sys.argv[3], 0)
   base = address & ~MAPMASK
   offset = address & MAPMASK

   # mmap PCI resource
   with open(sys.argv[1], "wb", 0) as f:
      mm = mmap.mmap(f.fileno(), MAPSIZE, mmap.MAP_SHARED, mmap.PROT_WRITE, 0, base)

      # write data (64 bit)
      data = binascii.unhexlify(data_param)
      data_be = data[::-1]
      mm[offset:offset+8] = data[0:8]

      # close mapping
      mm.close()

