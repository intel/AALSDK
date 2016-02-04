#!/bin/sh

# while true
NUM_TESTS=$1
echo "Stress test will run" $NUM_TESTS "tests"

for i in `seq 1 $NUM_TESTS`;
do
    if pgrep "ase_simv" -u $USER
    then
	echo "------------------------------------------------"
	echo "Running test" $i
	./nlb_test `shuf -i 4000-16383 -n 1` > output.log
	if grep -q "ERROR" output.log
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

