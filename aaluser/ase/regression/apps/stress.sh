#!/bin/sh

while true
do
    if pgrep "ase_simv" 
    then
	echo "Running test"
#	./nlb_test `shuf -i 4-65535 -n 1`
	./nlb_test 4
	sleep 1
    else
	echo "Simulator not running... EXIT";
	exit
    fi
done

