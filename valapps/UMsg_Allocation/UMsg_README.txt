To Test the UMsgs, 

Load the latest mode7 bitstream. Please note that the beta release bitstream won't work because the AFU ID has changed since then. 

Go to aalsdk/valapps/UMsg_Allocation.
Run $./UMsg_Allocation
This tests SW-UMSG-03, 04, 05, 06 & 07

To test SW-UMSG-01, go to <install location>/bin
Run $./fpgasane_nlb400_mode7.sh

To test SW-UMSG-02, load a known BAD mode7 bitstream. It is available in atp-lab:~XFER/sadrutac/Bad_mode7_bitstreams/bdx-p_nlb400_24eb_seed3.rbf
Go to <install location>/bin
Run $./fpgasane_nlb400_mode7.sh 
