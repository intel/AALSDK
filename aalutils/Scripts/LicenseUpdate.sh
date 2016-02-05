# execute from aalkernel directory

# Find the file, then feed it to the perl script. First part is a good check. Uncomment the second part to do damage.
find . -type f -exec grep -IHl 'Copyright' '{}' \; | xargs perl -pi.orig -e 's/Copyright[()Cc <]+([\d]{4,4}).* (Intel|INTEL)/Copyright(c) \1-2016, Intel/'

# for VHDL files from Arthur -- should not be needed in the future
#find . -type f -exec grep -IHl 'Copyright' '{}' \; | xargs perl -pi.orig -e 's/Copyright[()Cc <]+(Intel|INTEL) +([\d]{4,4})[-, ]*([\d]{4,4}[-, ]*)*\./Copyright (c) \2-2010 Intel Corporation All Rights Reserved/'

