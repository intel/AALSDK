#!/bin/bash -e

dmesg | tail -n 1000 | dd of=initial.txt
./AFU_Reset
if [ $? -eq 0 ]
   then echo "Code run ok."
fi


dmesg | tail -n 1000 | dd of=final.txt

diff -a --suppress-common-lines final.txt initial.txt | grep "<" | dd of=differences.txt
grep -Fxq differences.txt expected.txt
if [ $? -eq 1 ]
   then echo "Test passed."
fi

exit 0

