# Makefile for qpiregs

ifndef enable_assert
enable_assert:=n
endif

ifndef enable_debug
enable_debug:=y
endif

ifndef enable_ult
enable_ult:=n
endif

ifeq ($(enable_assert),y)
ifdef CPPFLAGS
ifeq ($(subst -DENABLE_ASSERT=,,$(CPPFLAGS)),$(CPPFLAGS))
override CPPFLAGS:=$(strip $(CPPFLAGS) -DENABLE_ASSERT=1)
endif
else
CPPFLAGS:=-DENABLE_ASSERT=1
endif
endif

ifeq ($(enable_debug),y)
ifdef CPPFLAGS
ifeq ($(subst -DENABLE_DEBUG=,,$(CPPFLAGS)),$(CPPFLAGS))
override CPPFLAGS:=$(strip $(CPPFLAGS) -DENABLE_DEBUG=1)
endif
else
CPPFLAGS:=-DENABLE_DEBUG=1
endif
endif

ifeq ($(enable_ult),y)
ifdef CPPFLAGS
ifeq ($(subst -DENABLE_ULT=,,$(CPPFLAGS)),$(CPPFLAGS))
override CPPFLAGS:=$(strip $(CPPFLAGS) -DENABLE_ULT=1)
endif
else
CPPFLAGS:=-DENABLE_ULT=1
endif
endif

#CPP:=cpp
#override CPPFLAGS:=$(strip $(CPPFLAGS) -Ixyz)
#AS:=as
#ASFLAGS:=
CC:=gcc
CXX:=g++
ifeq ($(enable_debug),y)
override CFLAGS:=$(strip $(CFLAGS) -g)
override CXXFLAGS:=$(strip $(CXXFLAGS) -g)
else
override CFLAGS:=$(strip $(CFLAGS) -O2)
override CXXFLAGS:=$(strip $(CXXFLAGS) -O2)
endif
LDFLAGS:=-lpci


.PHONY: all clean

all: qpiregs

qpiregs: qpiregs.cpp

clean:
	$(RM) qpiregs

