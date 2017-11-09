OBJS = OmegaMain.o ../logger/Logger.o
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
	TODELETE = *.o ../bin/OmegaMain.exe *.txt
else
	LIBRARIES = uuid
	TODELETE = *.o *~ ../bin/OmegaMain *.txt
endif

OmegaMain : $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -l$(LIBRARIES) -o ../bin/OmegaMain

OmegaMain.o : OmegaMain.cpp
	$(CC) $(CFLAGS) -I$(INCLUDES) -DLOGGING_LEVEL=$(LOGLVL) OmegaMain.cpp
	#TODO: make LOGGING_LEVEL able to be modified when compiling

clean:
	\rm $(TODELETE)