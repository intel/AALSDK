#1/bin/sh

#Sanity check for nlb400_3 bitstream

#READ test
./fpgadiag --target=fpga --mode=ccip-read --begin=65535 --cont --timeout-sec=10


#WRITE test
./fpgadiag --target=fpga --mode=ccip-write --begin=65535 --cont --timeout-sec=10


#TRPUT test
./fpgadiag --target=fpga --mode=ccip-trput --begin=65535 --cont --timeout-sec=10
