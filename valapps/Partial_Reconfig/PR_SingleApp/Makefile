# INTEL CONFIDENTIAL - For Intel Internal Use Only

# valapps/Partial_Reconfig/Makefile
VALAPP   = ../../valapp
CPPFLAGS ?= --std=c++11 -I$(VALAPP)
CXX      ?= g++
LDFLAGS  ?= 

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
LDFLAGS  += -L$(prefix)/lib -Wl,-rpath-link -Wl,$(prefix)/lib -Wl,-rpath -Wl,$(prefix)/lib \
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

	
all: Partial_Reconfig

Partial_Reconfig: main.o ALINLB.o ALIReconf.o
	$(CXX) -g -O2 -o Partial_Reconfig main.o ALINLB.o ALIReconf.o $(VALAPP)/libvalutils.la $(LDFLAGS) -lOSAL -lAAS -laalrt -laalclp

main.o: main.cpp Makefile
	$(CXX) $(CPPFLAGS) -D__AAL_USER__=1 -g -O2 -c -o main.o main.cpp

ALINLB.o: ALINLB.cpp ALINLB.h Makefile
	$(CXX) $(CPPFLAGS) -D__AAL_USER__=1 -g -O2 -c -o ALINLB.o ALINLB.cpp
	
ALIReconf.o: ALIReconf.cpp ALIReconf.h Makefile
	$(CXX) $(CPPFLAGS) -D__AAL_USER__=1 -g -O2 -c -o ALIReconf.o ALIReconf.cpp
	
clean:
	$(RM) Partial_Reconfig main.o ALINLB.o ALIReconf.o

.PHONY:all clean
