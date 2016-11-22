# INTEL CONFIDENTIAL - For Intel Internal Use Only

# valapps/appbase/Makefile

CPPFLAGS ?=-I. -I../valapp
CXX      ?= g++
LDFLAGS  ?=-L../valapp

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

all: libappbase.a

#libappbase.so: libappbase.so.0.0.0
#	ln -sf libappbase.so.0.0.0 libappbase.so

#libappbase.so.0: libappbase.so.0.0.0
#	ln -sf libappbase.so.0.0.0 libappbase.so.0

#libappbase.so.0.0.0: libappbase.o
#	$(CXX) -fPIC -shared -Wl,-soname -Wl,libappbase.so.0 libappbase.o $(LDFLAGS) -lOSAL -lAAS -laalrt -o libappbase.so.0.0.0

libappbase.a: appbase.o appconstants.o appbuffer.o
	ar -rcs libappbase.a appbase.o appconstants.o appbuffer.o ../valapp/libvalutils.la

appbase.o: appbase.cpp appbase.h appconstants.cpp appbuffer.cpp Makefile
	$(CXX) $(CPPFLAGS) -D__AAL_USER__=1 -fPIC -DPIC -g3  -c -lOSAL -lAAS -laalrt -lvalutils appbase.cpp -o appbase.o

appconstants.o: appbase.cpp appbase.h appconstants.cpp appbuffer.cpp Makefile
	$(CXX) $(CPPFLAGS) -D__AAL_USER__=1 -fPIC -DPIC -g3  -c -lOSAL -lAAS -laalrt appconstants.cpp -o appconstants.o

appbuffer.o: appbase.cpp appbase.h appconstants.cpp appbuffer.cpp Makefile
	$(CXX) $(CPPFLAGS) -D__AAL_USER__=1 -fPIC -DPIC -g3  -c -lOSAL -lAAS -laalrt appbuffer.cpp -o appbuffer.o

clean:
	$(RM) libappbase.so libappbase.so.0 libappbase.so.0.0.0 appbase.o appconstants.o appbuffer.o libappbase.a

.PHONY: all clean
