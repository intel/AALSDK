# execute from aalkernel directory

# Find the file, then feed it to the perl script. First part is a good check. Uncomment the second part to do damage.
# find . -type f | xargs perl -0777 -pi.orig -e 's/\/\/ +This +file +is +provided +under +a +dual.*BSD LICENSE//s'
find . -type f | xargs perl -0777 -pi -e 's/\/\/ +This +file +is +provided +under +a +dual.*BSD LICENSE//s'
find . -type f | xargs perl -0777 -pi -e 's/## +This +file +is +provided +under +a +dual.*BSD LICENSE//s'


#perl -0777 -pi.orig -e 's/\/\/ +This +file +is +provided +under +a +dual.*BSD LICENSE//s' aalbus-device.c

#find . -type f -exec grep -IHl 'Copyright' '{}' \; | xargs perl -pi.orig -e 's/Copyright[()Cc <]+([\d]{4,4}).* (Intel|INTEL)/Copyright(c) \1-2016, Intel/'

