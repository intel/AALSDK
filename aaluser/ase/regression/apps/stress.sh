#!/bin/sh

while true
do
    if pgrep "ase_simv" 
    then
	echo "Running test"
	# ./nlb_test `shuf -i 4-4000 -n 1` > output.log
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

