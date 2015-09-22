# Date: 4-9-2015
 # Created by: pratik.m.marolia@intel.com
   # Description: Dumps detailed performance statistics including cache hit, miss, s, credit throttling etc
   # Usage:
  #   1. Set the path to your application in TEST_CMD variable
   #   2. Run the script. The script will execute the test and print the performance statistics
   #
  # Tested with: SR 4.1.0 release
  #
#!/bin/bash

   # Specify command to invoke your application below
TEST_CMD="<application>"

  # DO NOT EDIT the following
  #===============================================================================================================
ITERATIONS=1
  # CSR Addresses
PERF1C=0x27c
PERF1=0x28c
TCOUNT0=0x394
TCOUNT1=0x398
TCOUNT2=0x3A0
TCOUNT3=0x3A4
TCOUNT4=0x3A8
TCOUNT5=0x3AC
TCOUNT6=0x3B0
TCOUNT7=0x3B4

RESET_COUNTERS=0x10000000;
OUT_OF_RESET=0x00000000;

P_RDHIT=0;
P_WRHIT=1;
P_RDMISS=2;
P_WRMISS=3;
P0_CHWR=0x00000006;
P0_TGWR=0x00000007;
P0_TX=0x00000008;
P0_RX=0x00000009;
P0_EVICT=0x0000000a;
P0_ADDRCNFLT=0x0000000b;
P0_TGRD=0x0000000c;
P0_FLAGT4=0x0000000d;

P1_CHWR=0x80000006;
P1_TGWR=0x80000007;
P1_TX=0x80000008;
P1_RX=0x80000009;
P1_EVICT=0x8000000a;
P1_ADDRCNFLT=0x8000000b;
P1_TGRD=0x8000000c;
P1_FLAGT4=0x8000000d;

VENDOR=0x8086
DEVICE=0xbcbc

for ((i=0; i<$ITERATIONS; i++))
do
        setpci -d $VENDOR:$DEVICE $PERF1C.L=$RESET_COUNTERS
        setpci -d $VENDOR:$DEVICE $PERF1C.L=$OUT_OF_RESET

        tcount0_start_value=`echo  "  $((16#$(setpci -d $VENDOR:$DEVICE $TCOUNT0.L) ))"`      >> /dev/null  #   tcount0_start_value="$(./csr read $TCOUNT0 | awk '{print $8}')"
        tcount1_start_value=`echo  "  $((16#$(setpci -d $VENDOR:$DEVICE $TCOUNT1.L) ))"`      >> /dev/null  #  tcount1_start_value="$(./csr read $TCOUNT1 | awk '{print $8}')"
	tcount2_start_value=`echo  "  $((16#$(setpci -d $VENDOR:$DEVICE $TCOUNT2.L) ))"`      >> /dev/null  #   tcount2_start_value="$(./csr read $TCOUNT2 | awk '{print $8}')"
        tcount3_start_value=`echo  "  $((16#$(setpci -d $VENDOR:$DEVICE $TCOUNT3.L) ))"`      >> /dev/null  #   tcount3_start_value="$(./csr read $TCOUNT3 | awk '{print $8}')"
        tcount4_start_value=`echo  "  $((16#$(setpci -d $VENDOR:$DEVICE $TCOUNT4.L) ))"`      >> /dev/null  #   tcount4_start_value="$(./csr read $TCOUNT4 | awk '{print $8}')"
        tcount5_start_value=`echo  "  $((16#$(setpci -d $VENDOR:$DEVICE $TCOUNT5.L) ))"`      >> /dev/null  #   tcount5_start_value="$(./csr read $TCOUNT5 | awk '{print $8}')"
        tcount6_start_value=`echo  "  $((16#$(setpci -d $VENDOR:$DEVICE $TCOUNT6.L) ))"`      >> /dev/null  #   tcount6_start_value="$(./csr read $TCOUNT6 | awk '{print $8}')"
        tcount7_start_value=`echo  "  $((16#$(setpci -d $VENDOR:$DEVICE $TCOUNT7.L) ))"`      >> /dev/null  #   tcount7_start_value="$(./csr read $TCOUNT7 | awk '{print $8}')"

 #Cachelines Read_Count Write_Count Cache_Rd_Hit Cache_Wr_Hit Cache_Rd_Miss Cache_Wr_Miss   Eviction 'Ticks(@200 MHz)'   Rd_Bandwidth   Wr_Bandwidth
        RESULT=`$TEST_CMD`
        arr=($RESULT)
        for((j=0;j<9;j++))
        do
                echo ${arr[$j]} : ${arr[12+$j]}
        done
  #     =====================================================================================================================================================================   
  #     ||                              SETPCI                                                              #                           CSR                                 ||          
   #    ======================================================================================================================================================================  
        setpci -d $VENDOR:$DEVICE $PERF1C.L=$P0_CHWR                               	      >> /dev/null  #    ./csr write $PERF1C $P0_CHWR >> /dev/null
        p0ch_end_value=`echo  "  $((16#$(setpci -d $VENDOR:$DEVICE $PERF1.L) ))"`             >> /dev/null  #    p0ch_end_value="$(./csr read $PERF1 | awk '{print $8}')"
        setpci -d $VENDOR:$DEVICE $PERF1C.L=$P0_TGWR                                	      >> /dev/null  #    ./csr write $PERF1C $P0_TGWR >> /dev/null
        p0tg_end_value=`echo  "  $((16#$(setpci -d $VENDOR:$DEVICE $PERF1.L) ))"`             >> /dev/null  #    p0tg_end_value="$(./csr read $PERF1 | awk '{print $8}')"
        setpci -d $VENDOR:$DEVICE $PERF1C.L=$P0_TX                                            >> /dev/null  #    ./csr write $PERF1C $P0_TX >> /dev/null
        p0tx_end_value=`echo  "  $((16#$(setpci -d $VENDOR:$DEVICE $PERF1.L) ))"`             >> /dev/null  #    p0tx_end_value="$(./csr read $PERF1 | awk '{print $8}')"
        setpci -d $VENDOR:$DEVICE $PERF1C.L=$P0_RX                                            >> /dev/null  #    ./csr write $PERF1C $P0_RX >> /dev/null
        p0rx_end_value=`echo  "  $((16#$(setpci -d $VENDOR:$DEVICE $PERF1.L) ))"`             >> /dev/null  #    p0rx_end_value="$(./csr read $PERF1 | awk '{print $8}')"
        setpci -d $VENDOR:$DEVICE $PERF1C.L=$P0_ADDRCNFLT                          	      >> /dev/null  #    ./csr write $PERF1C $P0_ADDRCNFLT >> /dev/null
        p0addr_end_value=`echo  "  $((16#$(setpci -d $VENDOR:$DEVICE $PERF1.L) ))"`           >> /dev/null  #    p0addr_end_value="$(./csr read $PERF1 | awk '{print $8}')"
        setpci -d $VENDOR:$DEVICE $PERF1C.L=$P0_TGRD                                          >> /dev/null  #    ./csr write $PERF1C $P0_TGRD >> /dev/null
        p0tgrd_end_value=`echo  "  $((16#$(setpci -d $VENDOR:$DEVICE $PERF1.L) ))"`           >> /dev/null  #    p0tgrd_end_value="$(./csr read $PERF1 | awk '{print $8}')"
        setpci -d $VENDOR:$DEVICE $PERF1C.L=$P0_EVICT                                         >> /dev/null  #    ./csr write $PERF1C $P0_EVICT >> /dev/null
        p0evict_end_value=`echo  "  $((16#$(setpci -d $VENDOR:$DEVICE $PERF1.L) ))"`          >> /dev/null  #    p0evict_end_value="$(./csr read $PERF1 | awk '{print $8}')"
        setpci -d $VENDOR:$DEVICE $PERF1C.L=$P0_FLAGT4                                        >> /dev/null  #    ./csr write $PERF1C $P0_FLAGT4 >> /dev/null
        p0flagt4_end_value=`echo  "  $((16#$(setpci -d $VENDOR:$DEVICE $PERF1.L) ))"`         >> /dev/null  #    p0flagt4_end_value="$(./csr read $PERF1 | awk '{print $8}')"
                                                                                              >> /dev/null  #
        setpci -d $VENDOR:$DEVICE $PERF1C.L=$P1_CHWR                                          >> /dev/null  #    ./csr write $PERF1C $P1_CHWR >> /dev/null
        p1ch_end_value=`echo  "  $((16#$(setpci -d $VENDOR:$DEVICE $PERF1.L) ))"`             >> /dev/null  #    p1ch_end_value="$(./csr read $PERF1 | awk '{print $8}')"
        setpci -d $VENDOR:$DEVICE $PERF1C.L=$P1_TGWR                                          >> /dev/null  #    ./csr write $PERF1C $P1_TGWR >> /dev/null
        p1tg_end_value=`echo  "  $((16#$(setpci -d $VENDOR:$DEVICE $PERF1.L) ))"`             >> /dev/null  #    p1tg_end_value="$(./csr read $PERF1 | awk '{print $8}')"
        setpci -d $VENDOR:$DEVICE $PERF1C.L=$P1_TX                                            >> /dev/null  #    ./csr write $PERF1C $P1_TX >> /dev/null
        p1tx_end_value=`echo  "  $((16#$(setpci -d $VENDOR:$DEVICE $PERF1.L) ))"`             >> /dev/null  #    p1tx_end_value="$(./csr read $PERF1 | awk '{print $8}')"
        setpci -d $VENDOR:$DEVICE $PERF1C.L=$P1_RX                                            >> /dev/null  #    ./csr write $PERF1C $P1_RX >> /dev/null
        p1rx_end_value=`echo  "  $((16#$(setpci -d $VENDOR:$DEVICE $PERF1.L) ))"`             >> /dev/null  #    p1rx_end_value="$(./csr read $PERF1 | awk '{print $8}')"
        setpci -d $VENDOR:$DEVICE $PERF1C.L=$P1_ADDRCNFLT                                     >> /dev/null  #    ./csr write $PERF1C $P1_ADDRCNFLT >> /dev/null
        p1addr_end_value=`echo  "  $((16#$(setpci -d $VENDOR:$DEVICE $PERF1.L) ))"`           >> /dev/null  #    p1addr_end_value="$(./csr read $PERF1 | awk '{print $8}')"
        setpci -d $VENDOR:$DEVICE $PERF1C.L=$P1_TGRD                                          >> /dev/null  #    ./csr write $PERF1C $P1_TGRD >> /dev/null
        p1tgrd_end_value=`echo  "  $((16#$(setpci -d $VENDOR:$DEVICE $PERF1.L) ))"`           >> /dev/null  #    p1tgrd_end_value="$(./csr read $PERF1 | awk '{print $8}')"
        setpci -d $VENDOR:$DEVICE $PERF1C.L=$P1_EVICT                                         >> /dev/null  #    ./csr write $PERF1C $P1_EVICT >> /dev/null
        p1evict_end_value=`echo  "  $((16#$(setpci -d $VENDOR:$DEVICE $PERF1.L) ))"`          >> /dev/null  #    p1evict_end_value="$(./csr read $PERF1 | awk '{print $8}')"
        setpci -d $VENDOR:$DEVICE $PERF1C.L=$P1_FLAGT4                                        >> /dev/null  #    ./csr write $PERF1C $P1_FLAGT4 >> /dev/null
        p1flagt4_end_value=`echo  "  $((16#$(setpci -d $VENDOR:$DEVICE $PERF1.L) ))"`         >> /dev/null  #    p1flagt4_end_value="$(./csr read $PERF1 | awk '{print $8}')"
                                                                                                            #
        tcount0_end_value=`echo  "  $((16#$(setpci -d $VENDOR:$DEVICE $TCOUNT0.L) ))"`        >> /dev/null  #   tcount0_end_value="$(./csr read $TCOUNT0 | awk '{print $8}')"       
        tcount1_end_value=`echo  "  $((16#$(setpci -d $VENDOR:$DEVICE $TCOUNT1.L) ))"`        >> /dev/null  #   tcount1_end_value="$(./csr read $TCOUNT1 | awk '{print $8}')"
        tcount2_end_value=`echo  "  $((16#$(setpci -d $VENDOR:$DEVICE $TCOUNT2.L) ))"`        >> /dev/null  #   tcount2_end_value="$(./csr read $TCOUNT2 | awk '{print $8}')"
        tcount3_end_value=`echo  "  $((16#$(setpci -d $VENDOR:$DEVICE $TCOUNT3.L) ))"`        >> /dev/null  #   tcount3_end_value="$(./csr read $TCOUNT3 | awk '{print $8}')"
        tcount4_end_value=`echo  "  $((16#$(setpci -d $VENDOR:$DEVICE $TCOUNT4.L) ))"`        >> /dev/null  #   tcount4_end_value="$(./csr read $TCOUNT4 | awk '{print $8}')"
        tcount5_end_value=`echo  "  $((16#$(setpci -d $VENDOR:$DEVICE $TCOUNT5.L) ))"`        >> /dev/null  #   tcount5_end_value="$(./csr read $TCOUNT5 | awk '{print $8}')"
        tcount6_end_value=`echo  "  $((16#$(setpci -d $VENDOR:$DEVICE $TCOUNT6.L) ))"`        >> /dev/null  #   tcount6_end_value="$(./csr read $TCOUNT6 | awk '{print $8}')"
        tcount7_end_value=`echo  "  $((16#$(setpci -d $VENDOR:$DEVICE $TCOUNT7.L) ))"`        >> /dev/null  #   tcount7_end_value="$(./csr read $TCOUNT7 | awk '{print $8}')"

           val_ch=$(expr $p1ch_end_value + $p0ch_end_value)
           val_tg=$(expr $p1tg_end_value + $p0tg_end_value)
           val_tx=$(expr $p1tx_end_value + $p0tx_end_value)
           val_rx=$(expr $p1rx_end_value + $p0rx_end_value)
           val_addr=$(expr $p1addr_end_value + $p0addr_end_value)
           val_tgrd=$(expr $p1tgrd_end_value + $p0tgrd_end_value)
           val_evict=$(expr $p1evict_end_value + $p0evict_end_value)
           val_flag4=$(expr $p1flagt4_end_value + $p0flagt4_end_value)

           val_tcount0=$(expr $tcount0_end_value - $tcount0_start_value)
           val_tcount1=$(expr $tcount1_end_value - $tcount1_start_value)
           val_tcount2=$(expr $tcount2_end_value - $tcount2_start_value)
           val_tcount3=$(expr $tcount3_end_value - $tcount3_start_value)
           val_tcount4=$(expr $tcount4_end_value - $tcount4_start_value)
           val_tcount5=$(expr $tcount5_end_value - $tcount5_start_value)
           val_tcount6=$(expr $tcount6_end_value - $tcount6_start_value)
           val_tcount7=$(expr $tcount7_end_value - $tcount7_start_value)

 #      echo "=Read Pipe="
        echo "P0  CH : $p0ch_end_value"
        echo "P0  TG Wr: $p0tg_end_value"
        echo "P0  TX : $p0tx_end_value"
        echo "P0  RX : $p0rx_end_value"
        echo "P0  Evicts: $p0evict_end_value"
        echo "P0  Addr Conflict: $p0addr_end_value"
        echo "P0  TG Rd : $p0tgrd_end_value"
        echo "P0  FlagT4: $p0flag4_end_value"

  #     echo "=Write Pipe="
        echo "P1  CH : $p1ch_end_value"
        echo "P1  TG Wr: $p1tg_end_value"
        echo "P1  TX : $p1tx_end_value"
        echo "P1  RX : $p1rx_end_value"
        echo "P1  Evicts: $p1evict_end_value"
        echo "P1  Addr Conflict: $p1addr_end_value"
        echo "P1  TG Rd : $p1tgrd_end_value"
        echo "P1  FlagT4: $p1flag4_end_value"

 #     echo "=Total P0+P1="
        echo "P0+P1  CH : $val_ch"
        echo "P0+P1  TG Wr: $val_tg"
        echo "P0+P1  TX : $val_tx"
        echo "P0+P1  RX : $val_rx"
        echo "P0+P1  Evicts: $val_evict"
        echo "P0+P1  Addr Conflict : $val_addr"
        echo "P0+P1  TG Rd : $val_tgrd"
        echo "P0+P1  FlagT4: $val_flag4"


 #      echo "=TCOUNTs from fsm="
        echo "Tx Request Credit throttle : " $val_tcount0
        echo "Tx Response Credit throttle : " $val_tcount1
        echo "Rx Ack throttle : " $val_tcount2
        echo "Tx Ack throttle : " $val_tcount3
        echo "Tx Request Q throttle : " $val_tcount4
        echo "Tx Response Q throttle : " $val_tcount5
          #echo "Rx CRC : " $val_tcount6
        #echo "Tx CRC : " $val_tcount7
done


