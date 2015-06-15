CC=g++
RANLIB=ranlib

CLNTSRC = clftp.cpp
SRVRSRC = srftp.cpp


INCS=-I.
CFLAGS = -Wall -g -std=c++11 $(INCS)

TAR=tar
TARFLAGS=-cvf
TARNAME=ex5.tar
TARSRCS=$(CLNTSRC) $(SRVRSRC) Makefile README


all: clean clftp srftp

clftp: $(CLNTSRC)
	$(CC) $(CFLAGS) $^ -lpthread -o $@

srftp: $(SRVRSRC)
	$(CC) $(CFLAGS) $^ -lpthread -o $@

clean:
	$(RM) $(TARGETS) $(EXLIB) $(OBJ) $(LIBOBJ) *~ *core

tar:
	$(TAR) $(TARFLAGS) $(TARNAME) $(TARSRCS)


