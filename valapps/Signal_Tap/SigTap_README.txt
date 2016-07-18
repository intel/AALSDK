To Test MMLink, 

Load the platform with a RemoteSTP enabled mode0 bitstream.

To test SW-STAP-01, 
Go to valapps/Signal_Tap
Run $./Signal_Tap.

To test SW-STAP-02, 
Go to aalsamples/MMLink/SW
Run $./mmlink

In your windows terminal, run the system console. Please see attached "BKMs STP.pdf" file for steps to setup system-console on a windows host.
$ system-console --rc_script=mmlink_profiled_setup.tcl remote_debug.sof <ip of board> 3333

Start the Quartus Signal tap and follow the steps in the PDF document until it is ready to acquire.

In your linux host machine, open another terminal and go to <install location>/bin
Run $./fpgasane_nlb400_mode0.sh 

Verify that the signals were captured on the windows host machine.
 