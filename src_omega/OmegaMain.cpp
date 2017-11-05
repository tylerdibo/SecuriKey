#include <iostream>
#include "Logger.h"

using namespace std;

bool checkMACAddress(const char* currMAC){
	const char* letterMAC = currMAC;
	
	ifstream mac_file;
	mac_file.open("MAC_addresses");
	if(!mac_file.is_open()){
		return false;
	}
	
	const int maxLineLength = 50;
	char* line = new char[maxLineLength];
	
	bool done = false;
	bool foundMatch = false;
	int lineNumber = 0;
	while(!done && !foundMatch){
		lineNumber++;
		if(!mac_file.getline(line, maxLineLength)){
			if(mac_file.eof()){
				done = true;
			}else{
				return -1;
			}
		}else{
			letterMAC = currMAC;
			bool different = false;
			while(*line != 0 && *letterMAC != 0 && !different){
				if(*line != *letterMAC){
					different = true;
				}
				line++;
				letterMAC++;
			}
			if(*letterMAC != 0 || *line != 0){
				different = true;
			}
			if(!different){
				foundMatch = true;
			}
		}
	}
	
	delete[] line;
	
	return foundMatch;
}

int main(const int argc, const char* const argv[]){
	Logger logg("test_log.txt", LEVEL_DEBUG); //Initialize log file
	logg.info("Main", "Program Started");
	
	if(argc < 2){
		logg.error("Main", "No MAC address provided");
		return -1;
	}
	if(!checkMACAddress(argv[1])){
		logg.warning("Main", "MAC address not permitted");
		//promptForPasscode(); + ask if public computer
	}else{
		cout << "Match found" << endl;
	}
	//logg.info("Main", "MAC address: " + argv[1]);
	
	
	//MAIN PROGRAM LOOP
	bool exit = false;
	while(!exit){
		//Get input, then process, and execute command
		string inputStr;
		getline(cin, inputStr);
		
		size_t spaceIndex = inputStr.find_first_of(" ");
		string command = inputStr.substr(0, spaceIndex);
		
		logg.debug("Main", "Command entered: " + command);
		
		if(command == "request"){
			
		}else if(command == "add"){
			
		}else if(command == "getsettings"){
			
		}else if(command == "setsettings"){
			
		}else if(command == "exit"){
			logg.info("Main", "Exiting program...");
			exit = true;
		}else if(command == "help"){
			
		}else{
			cout << "Invalid command" << endl;
			//printHelp();
		}
	}
	
	return 0;
}