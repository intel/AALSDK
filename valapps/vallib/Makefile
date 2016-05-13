# INTEL CONFIDENTIAL - For Intel Internal Use Only

# valapps/vallib/Makefile

CPPFLAGS ?=
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
LDFLAGS  += -L$(prefix)/lib -L$(prefix)/lib64
endif
else
ifeq (,$(prefix))
prefix = /usr/local
endif
CPPFLAGS += -I$(DESTDIR)$(prefix)/include
LDFLAGS  += -L$(DESTDIR)$(prefix)/lib -L$(DESTDIR)$(prefix)/lib64
endif

all: libvallib.so libvallib.so.0

libvallib.so: libvallib.so.0.0.0
	ln -sf libvallib.so.0.0.0 libvallib.so

libvallib.so.0: libvallib.so.0.0.0
	ln -sf libvallib.so.0.0.0 libvallib.so.0

libvallib.so.0.0.0: libvallib.o
	$(CXX) -fPIC -shared -Wl,-soname -Wl,libvallib.so.0 libvallib.o $(LDFLAGS) -lOSAL -lAAS -laalrt -o libvallib.so.0.0.0

libvallib.o: vallib.cpp vallib.h Makefile
	$(CXX) $(CPPFLAGS) -D__AAL_USER__=1 -fPIC -DPIC -g -O2 -c -o libvallib.o vallib.cpp

clean:
	$(RM) libvallib.so libvallib.so.0 libvallib.so.0.0.0 libvallib.o

.PHONY: all clean
