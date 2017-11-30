OBJS = OmegaMain.obj ../logger/Logger.obj Settings.obj
CXX = g++
DEBUG = -g
CXXFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall $(DEBUG)
INCLUDES = ../logger

LOGLVL ?= 4

ifeq ($(OS),Windows_NT)
	detected_OS := Windows
else
	detected_OS := $(shell uname -s)
endif

ifeq ($(detected_OS),Windows)
	LIBRARIES = 
	TODELETE = *.o ../bin/OmegaMain.exe *.txt
else
	LIBRARIES = 
	TODELETE = *.o *~ ../bin/OmegaMain *.txt
endif

OmegaMain : $(OBJS)
	$(CXX) $(LFLAGS) $(OBJS) $(LIBRARIES) -o ../bin/OmegaMain

../logger/Logger.obj : ../logger/Logger.cpp ../logger/Logger.h
	$(CXX) $(CFLAGS) -I$(INCLUDES) ../logger/Logger.cpp -o ../logger/Logger.obj

Settings.obj : Settings.cpp Settings.h
	$(CXX) $(CFLAGS) -I$(INCLUDES) Settings.cpp -o Settings.obj

OmegaMain.obj : OmegaMain.cpp ../logger/Logger.obj Settings.obj
	$(CXX) $(CFLAGS) -I$(INCLUDES) -DLOGGING_LEVEL=$(LOGLVL) OmegaMain.cpp -o OmegaMain.obj

clean:
	\rm $(TODELETE)
