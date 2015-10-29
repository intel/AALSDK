
SRC := $(shell pwd)
LDFLAGS += -lstdc++ $(DEBUG)
CFLAGS += -c $(DEBUG)
CC := arm-linux-gnueabihf-g++
#DEBUG := -g -O0 -fno-inline
DEBUG :=

MMLINK_OBJS := udp_log.o mmlink_server.o mmlink_connection.o mm_debug_link.o main.o

mmlink: $(MMLINK_OBJS)
	$(CC) $(LDFLAGS) $(MMLINK_OBJS) -o $@

# To enable udp logging, add 
#   -D MM_LINK_UDP_DEBUG
# To capture udp log messages on the computer 
# at the given IP address: nc -luv 4444
udp_log.o: udp_log.cpp udp_log.h
	$(CC) $(CFLAGS) \
	  -D MM_LINK_UDP_DEBUG_IP=\"192.168.100.1\" \
	  -D MM_LINK_UDP_DEBUG_PORT=4444 \
	  udp_log.cpp

mmlink_server.o: mmlink_server.cpp mmlink_server.h mmlink_connection.h mm_debug_link_interface.h dprint.h
	$(CC) $(CFLAGS) mmlink_server.cpp

mmlink_connection.o: mmlink_connection.cpp mmlink_connection.h dprint.h
	$(CC) $(CFLAGS) mmlink_connection.cpp

mm_debug_link.o: mm_debug_link_linux.cpp mm_debug_link_linux.h dprint.h
	$(CC) -I $(KERNEL_SRC_DIR)/include $(CFLAGS) mm_debug_link_linux.cpp -o mm_debug_link.o

main.o: main.cpp mmlink_server.h mm_debug_link_interface.h udp_log.h
	$(CC) $(CFLAGS) main.cpp

###
# "test" is a simple application which writes data to the driver 
# without need for a socket connection from the host.
TEST_OBJS := udp_log.o mm_debug_link.o test.o
test: $(TEST_OBJS)
	$(CC) $(LDFLAGS) $(TEST_OBJS) -o $@

test.o: test.cpp mm_debug_link_interface.h udp_log.h dprint.h
	$(CC) $(CFLAGS) test.cpp
	
clean:
	rm -f mmlink test
	rm -f *.o *~ core .depend .*.cmd *.ko *.mod.c
	rm -f Module.markers Module.symvers modules.order
	rm -rf .tmp_versions Modules.symvers

