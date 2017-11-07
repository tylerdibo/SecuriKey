OBJS = Logger.o OmegaMain.o
CC = g++
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall $(DEBUG)
LIBRARIES = Rpcrt4

OmegaMain : $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -l$(LIBRARIES) -o OmegaMain

OmegaMain.o : OmegaMain.cpp Logger.h
	$(CC) $(CFLAGS) -DLOGGING_LEVEL=3 OmegaMain.cpp
	#TODO: make LOGGING_LEVEL able to be modified when compiling

Logger.o : Logger.h Logger.cpp
	$(CC) $(CFLAGS) Logger.cpp

clean:
	\rm *.o *.exe *~ OmegaMain