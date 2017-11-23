#include "Logger.h"
#include <chrono>
#include <sstream>
#include <iomanip>

using namespace std;

Logger::Logger(string log_location, int level){
	log_level = level;
	log_stream.open(log_location, ios_base::app);
	//check if open
	if(!log_stream.is_open()){
		throw -1;
	}
}

Logger::~Logger(){
	log_stream.close();
	log_level = -1; //object deleted marker
}

string Logger::get_current_timestamp(){
	chrono::system_clock::time_point now = chrono::system_clock::now();
	time_t time = chrono::system_clock::to_time_t(now);
	stringstream sstream;
	sstream << put_time(localtime(&time), "[%c]");
	return sstream.str();
}

bool Logger::error(string tag, string message){
	if(log_level >= LEVEL_ERROR){
		cerr << get_current_timestamp() << ": ERROR: " << tag << ": " << message << endl;
	}
	log_stream << get_current_timestamp() << ": ERROR: " << tag << ": " << message << endl;
	return (log_stream.bad());
}

bool Logger::warning(string tag, string message){
	if(log_level >= LEVEL_WARNING){
		cout << get_current_timestamp() << ": Warning: " << tag << ": " << message << endl;
	}
	log_stream << get_current_timestamp() << ": Warning: " << tag << ": " << message << endl;
	return (log_stream.bad());
}

bool Logger::info(string tag, string message){
	if(log_level >= LEVEL_INFO){
		cout << get_current_timestamp() << ": Info: " << tag << ": " << message << endl;
	}
	log_stream << get_current_timestamp() << ": Info: " << tag << ": " << message << endl;
	return (log_stream.bad());
}

bool Logger::debug(string tag, string message){
	if(log_level >= LEVEL_DEBUG){
		cout << get_current_timestamp() << ": Debug: " << tag << ": " << message << endl;
	}
	log_stream << get_current_timestamp() << ": Debug: " << tag << ": " << message << endl;
	return (log_stream.bad());
}