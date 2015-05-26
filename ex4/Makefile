CC=g++
RANLIB=ranlib

LIBSRC=Logger.cpp CachingFileSystem.cpp
LIBHEADER=Logger.h CacheState.h
LIBOBJ=$(LIBSRC:.cpp=.o)

INCS=-I.
CFLAGS = -Wall -g -std=c++11 $(INCS)

EXLIB = caching
TARGETS = $(EXLIB)

TAR=tar
TARFLAGS=-cvf
TARNAME=ex4.tar
TARSRCS=$(LIBSRC) $(LIBHEADER) Makefile README

all: $(TARGETS)

$(TARGETS): $(LIBSRC) $(LIBHEADER)
	$(CC) $(CFLAGS) $<  `pkg-config fuse --cflags --libs` -o $@

clean:
	$(RM) $(TARGETS) $(EXLIB) $(OBJ) $(LIBOBJ) *~ *core

depend:
	makedepend -- $(CFLAGS) -- $(SRC) $(LIBSRC)

tar:
	$(TAR) $(TARFLAGS) $(TARNAME) $(TARSRCS)


