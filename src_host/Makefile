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
	TODELETE = *.o HostMain.exe *.txt
else
	LIBRARIES = 
	TODELETE = *.o *~ HostMain *.txt
endif

HostMain : $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) $(LIB_PATHS) $(LIBRARIES) -o HostMain

HostMain.o : HostMain.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -DLOGGING_LEVEL=3 HostMain.cpp
	#TODO: make LOGGING_LEVEL able to be modified when compiling

clean:
	\rm $(TODELETE)