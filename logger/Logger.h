#ifndef __LOGGER_H
#define __LOGGER_H

#include <iostream>
#include <fstream>
#include <string>

#define LEVEL_ERROR 1
#define LEVEL_WARNING 2
#define LEVEL_INFO 3
#define LEVEL_DEBUG 4

using namespace std;

class Logger{
private:
	ofstream log_stream;
	int log_level; //critical = 0, error = 1, warning = 2, info = 3, debug = 4
	string get_current_timestamp();
	
public:
	Logger(string log_location, int level);
	~Logger();
	bool error(string tag, string message);
	bool warning(string tag, string message);
	bool info(string tag, string message);
	bool debug(string tag, string message);
};

#endif /* __LOGGER_H */