#!/bin/sh

rm output.*.log

./build.sh

vc_arr[0]="0"
vc_arr[1]="1"
vc_arr[2]="2"
vc_arr[3]="3"


mcl_arr[0]="0"
mcl_arr[1]="1"
mcl_arr[2]="3"


if [ -z "$1" ]
then
    NUM_TESTS=10
else
    NUM_TESTS=$1
fi

echo "Stress test will run" $NUM_TESTS "tests"
for i in `seq 1 $NUM_TESTS`;
do
    if pgrep "ase_simv" -u $USER
    then
	echo "------------------------------------------------"
	echo "Running test" $i

	index=$[ $RANDOM % 4 ]
	vc_set=${vc_arr[$index]}

	index=$[ $RANDOM % 3 ]
	mcl_set=${mcl_arr[$index]}
	mcl_cnt=$(($mcl_set + 1))

	num_cl=`shuf -i 12000-16000 -n 1`
#	num_cl=`shuf -i 256-1024 -n 1`
#	echo $mcl_cnt $num_cl
	num_cl=$(($num_cl * $mcl_cnt))
	
	echo ./nlb_test $num_cl $vc_set $mcl_set
	./nlb_test $num_cl $vc_set $mcl_set > output.$i.log
	if grep -q "ERROR" output.$i.log
	then
	    echo "***** Test error *****"
	    exit
	fi
	sleep 1
    else
	echo "Simulator not running... EXIT";
	exit
    fi
done
echo "------------------------------------------------"
# pkill ase_simv

