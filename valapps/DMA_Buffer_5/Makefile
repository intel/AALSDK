# INTEL CONFIDENTIAL - For Intel Internal Use Only

# valapps/DMA_Buffer_5/Makefile

CPPFLAGS ?=-I../appbase -I../valapp
CXX      ?= g++
LDFLAGS  ?=-L../appbase -L../valapp

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

all: DMA_Buffer_5

DMA_Buffer_5: main.o
	$(CXX) -g -O2 -o DMA_Buffer_5 main.o ../valapp/libvalutils.la $(LDFLAGS) -lappbase -lOSAL -lAAS -laalrt

main.o: DMABuffer5.h main.cpp Makefile
	$(CXX) $(CPPFLAGS) -D__AAL_USER__=1 -g -O2 -c -o main.o main.cpp

clean:
	$(RM) DMA_Buffer_5 main.o

.PHONY:all clean
