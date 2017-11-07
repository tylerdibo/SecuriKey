OBJS = Logger.o OmegaMain.o
CC = g++
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall $(DEBUG)

ifeq ($(OS),Windows_NT)
	detected_OS := Windows
else
	detected_OS := $(shell uname -s)
endif

ifeq ($(detected_OS),Windows)
	LIBRARIES = Rpcrt4
	TODELETE = *.o OmegaMain.exe *.txt
else
	LIBRARIES = 
	TODELETE = *.o *~ OmegaMain *.txt
endif

OmegaMain : $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -l$(LIBRARIES) -o OmegaMain

OmegaMain.o : OmegaMain.cpp Logger.h
	$(CC) $(CFLAGS) -DLOGGING_LEVEL=3 OmegaMain.cpp
	#TODO: make LOGGING_LEVEL able to be modified when compiling

Logger.o : Logger.h Logger.cpp
	$(CC) $(CFLAGS) Logger.cpp

clean:
	\rm $(TODELETE)