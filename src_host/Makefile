OBJS = HostMain.o ../logger/Logger.o
CC = g++
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall $(DEBUG)
INCLUDES = -I../logger -I../inc
LIB_PATHS = -L../lib

ifeq ($(OS),Windows_NT)
	detected_OS := Windows
else
	detected_OS := $(shell uname -s)
endif

ifeq ($(detected_OS),Windows)
	LIBRARIES = -lRpcrt4 -lssh
	TODELETE = *.o ../bin/HostMain.exe *.txt
else
	LIBRARIES = 
	TODELETE = *.o *~ ../bin/HostMain *.txt
endif

HostMain : $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) $(LIB_PATHS) $(LIBRARIES) -o ../bin/HostMain

HostMain.o : HostMain.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -DLOGGING_LEVEL=$(LOGLVL) HostMain.cpp

clean:
	\rm $(TODELETE)