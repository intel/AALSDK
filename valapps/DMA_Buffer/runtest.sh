#!/bin/bash -e

echo "***DMA_Buffer 1MB test"
./DMA_Buffer -s 1 -d 2
if [ $? -eq 0 ]
   then echo "Passed."
fi

echo "***DMA_Buffer 2MB test"
./DMA_Buffer -s 2 -d 2
if [ $? -eq 0 ]
   then echo "Passed."
fi

echo "***DMA_Buffer 3MB test"
./DMA_Buffer -s 3 -d 2
if [ $? -eq 0 ]
   then echo "Passed."
fi

echo "***DMA_Buffer 4MB test"
./DMA_Buffer -s 4 -d 2
if [ $? -eq 0 ]
   then echo "Passed."
fi

echo "***DMA_Buffer 5MB test"
./DMA_Buffer -s 5 -d 2
if [ $? -eq 0 ]
   then echo "Passed."
fi

exit 0

