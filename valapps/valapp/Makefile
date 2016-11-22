# INTEL CONFIDENTIAL - For Intel Internal Use Only

# valapps/appbase/makefile

CPPFLAGS ?=--std=c++11 -I. -I../vallib
CXX      ?= g++
LDFLAGS  ?= 
GFLAGS   ?= 

ifneq (,$(ndebug))
else
CPPFLAGS += -DENABLE_DEBUG=1
endif
ifneq (,$(nassert))
else
CPPFLAGS += -DENABLE_ASSERT=1
endif

ifeq (,$(DESTDIR))
ifneq (,$(prefix))
CPPFLAGS += -I$(prefix)/include
LDFLAGS  += -L. -L$(prefix)/lib -Wl,-rpath-link -Wl,$(prefix)/lib -Wl,-rpath -Wl,$(prefix)/lib \
            -L$(prefix)/lib64 -Wl,-rpath-link -Wl,$(prefix)/lib64 -Wl,-rpath -Wl,$(prefix)/lib64
endif
else
ifeq (,$(prefix))
prefix = /usr/local
endif
CPPFLAGS += -I$(DESTDIR)$(prefix)/include
LDFLAGS  += -L$(DESTDIR)$(prefix)/lib -Wl,-rpath-link -Wl,$(prefix)/lib -Wl,-rpath -Wl,$(DESTDIR)$(prefix)/lib \
            -L$(DESTDIR)$(prefix)/lib64 -Wl,-rpath-link -Wl,$(prefix)/lib64 -Wl,-rpath -Wl,$(DESTDIR)$(prefix)/lib64
endif


UTILS_CPP = arguments.cpp utils.cpp process.cpp
UTILS_H = $(UTILS_CPP:.cpp=.h)
UTILS_O = $(UTILS_CPP:.cpp=.o) 

VALAFU_CPP = afu_client.cpp \
             afu_test.cpp \
             client_factory.cpp \
             nlb_client.cpp \
             reconfigure_client.cpp \
             service_client.cpp \
             service_manager.cpp \
             test_args.cpp \
             test_context.cpp \
             test_manager.cpp

VALAFU_H = $(VALAFU_CPP:.cpp=.h)             
VALAFU_O = $(VALAFU_CPP:.cpp=.o) 

AFU_TESTS_CPP = afu_reset.cpp \
                dma_buffer.cpp \
                mmio_mapping.cpp \
                perf_counters_tests.cpp \
                reconfigure_test.cpp \
                stap_tests.cpp

AFU_TESTS_H = $(AFU_TESTS_CPP:.cpp=.h)
AFU_TESTS_O = $(AFU_TESTS_CPP:.cpp=.o)


    
all: libvalutils.so libvalutils.la valapp libafu_tests.so libaalcif.so

$(UTILS_O): %.o: %.cpp %.h Makefile
	$(CXX) $(CPPFLAGS) -fPIC -DPIC $(GFLAGS) -c $< -o $@

libvalutils.so: $(UTILS_O)
	$(CXX) $(CPPFLAGS) -fPIC -shared -pthread $(GFLAGS) $(UTILS_O) $(LDFLAGS) -o libvalutils.so

libvalutils.la: $(UTILS_O)
	ar rcs libvalutils.la $(UTILS_O)

$(VALAFU_O): %.o: %.cpp %.h Makefile
	$(CXX) $(CPPFLAGS) -fPIC -DPIC $(GFLAGS) -c $< -o $@

libvalafu.so: $(VALAFU_O) jsoncpp.o
	$(CXX) $(CPPFLAGS) -fPIC -shared -pthread $(GFLAGS) $(VALAFU_O) jsoncpp.o $(LDFLAGS) -lvalutils -lOSAL -lAAS -laalrt -ldl -o libvalafu.so

$(AFU_TESTS_O): %.o: %.cpp %.h Makefile
	$(CXX) $(CPPFLAGS) -fPIC -DPIC $(GFLAGS) -c $< -o $@

libafu_tests.so: $(AFU_TESTS_O)
	$(CXX) $(CPPFLAGS) -fPIC -shared $(GFLAGS) $(AFU_TESTS_O) $(LDFLAGS) -o libafu_tests.so 

jsoncpp.o : jsoncpp.cpp
	$(CXX) $(CPPFLAGS) -fPIC -DPIC $(GFLAGS) -c jsoncpp.cpp -o jsoncpp.o

tester: testit.cpp $(UTILS_CPP) $(UTILS_H)
	$(CXX) $(CPPFLAGS) -fPIC -DPIC $(GFLAGS) testit.cpp $(UTILS_CPP) -o tester

valapp: main.o libvalafu.so libvalutils.so
	$(CXX) $(CPPFLAGS) $(GFLAGS) main.cpp -o valapp -I. -I$(prefix)/include $(LDFLAGS) -lvalutils -lvalafu 

libaalcif.so: c_service_manager.cpp c_service_manager.h 
	$(CXX) $(CPPFLAGS) $(GFLAGS) -fPIC -shared c_service_manager.cpp -I. -L. -lvalafu -o libaalcif.so


clean:
	$(RM) *.o *.so *.la
