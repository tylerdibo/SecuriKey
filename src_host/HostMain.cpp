#include <iostream>
#include "Logger.h"

using namespace std;

int main(const int argc, const char* const argv[]){
	Logger logg("host_log.txt", LOGGING_LEVEL); //Initialize log file
	logg.info("Main", "Program Started");
	
	return 0;
}