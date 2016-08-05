#!/usr/bin/python
"""64 bit MMIO read from PCI(e) BARs"""
import mmap, sys, binascii, struct

MAPSIZE = mmap.PAGESIZE
MAPMASK = MAPSIZE-1

if __name__ == "__main__":

   if len(sys.argv) < 3:
      sys.stderr.write("USAGE: pcipeek <resource> <offset>\n")
      sys.exit(-1)

   address = int(sys.argv[2], 0)
   base = address & ~MAPMASK
   offset = address & MAPMASK

   # mmap PCI resource
   with open(sys.argv[1], "rb", 0) as f:
      mm = mmap.mmap(f.fileno(), MAPSIZE, mmap.MAP_SHARED, mmap.PROT_READ, 0, base)

      # read data (64 bit)
      data_be = mm[offset:offset+8]
      data = binascii.hexlify(data_be[::-1])

      # close mapping
      mm.close()

      # print data
      print "0x" + data



