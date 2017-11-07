CC = g++
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall $(DEBUG)
TODELETE = *.o

Logger.o : Logger.h Logger.cpp
	$(CC) $(CFLAGS) Logger.cpp

clean:
	\rm $(TODELETE)