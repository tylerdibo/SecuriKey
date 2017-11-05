OBJS = Logger.o OmegaMain.o
CC = g++
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall $(DEBUG)

OmegaMain : $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o OmegaMain

OmegaMain.o : OmegaMain.cpp Logger.h
	$(CC) $(CFLAGS) OmegaMain.cpp

Logger.o : Logger.h Logger.cpp
	$(CC) $(CFLAGS) Logger.cpp

clean:
	\rm *.o *~ OmegaMain