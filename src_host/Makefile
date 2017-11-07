OBJS = HostMain.o ../logger/Logger.o
CC = g++
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall $(DEBUG)
INCLUDES = ../logger

ifeq ($(OS),Windows_NT)
	detected_OS := Windows
else
	detected_OS := $(shell uname -s)
endif

ifeq ($(detected_OS),Windows)
	LIBRARIES = Rpcrt4
	TODELETE = *.o HostMain.exe *.txt
else
	LIBRARIES = 
	TODELETE = *.o *~ HostMain *.txt
endif

HostMain : $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -l$(LIBRARIES) -o HostMain

HostMain.o : HostMain.cpp
	$(CC) $(CFLAGS) -I$(INCLUDES) -DLOGGING_LEVEL=3 HostMain.cpp
	#TODO: make LOGGING_LEVEL able to be modified when compiling

clean:
	\rm $(TODELETE)