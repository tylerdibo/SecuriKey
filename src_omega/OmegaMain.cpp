#include <iostream>
#include "Logger.h"

using namespace std;
#ifdef WIN32
#include <Rpc.h>
#else
#include <uuid/uuid.h>
#endif

bool checkMACAddress(const char* currMAC){
	const char* letterMAC = currMAC;
	
	ifstream mac_file;
	mac_file.open("MAC_addresses");
	if(!mac_file.is_open()){
		return false;
	}
	
	const int maxLineLength = 50;
	char* lineArray = new char[maxLineLength];
	char* lineChar = lineArray;
	
	bool done = false;
	bool foundMatch = false;
	int lineNumber = 0;
	while(!done && !foundMatch){
		lineNumber++;
		if(!mac_file.getline(lineChar, maxLineLength)){
			if(mac_file.eof()){
				done = true;
			}else{
				return -1;
			}
		}else{
			letterMAC = currMAC;
			bool different = false;
			while(*lineChar != 0 && *letterMAC != 0 && !different){
				if(*lineChar != *letterMAC){
					different = true;
				}
				lineChar++;
				letterMAC++;
			}
			if(*letterMAC != 0 || *lineChar != 0){
				different = true;
			}
			if(!different){
				foundMatch = true;
			}
		}
	}
	
	delete[] lineArray;
	
	return foundMatch;
}

std::string newUUID(){ //credit to: https://stackoverflow.com/questions/543306/platform-independent-guid-generation-in-c
#ifdef WIN32
    UUID uuid;
    UuidCreate(&uuid);

    unsigned char* str;
    UuidToStringA(&uuid, &str);

    std::string uuidStr((char*)str);

    RpcStringFreeA ( &str );
#else
    uuid_t uuid;
    uuid_generate_random ( uuid );
    char s[37];
    uuid_unparse ( uuid, s );
#endif
    return uuidStr;
}

int main(const int argc, const char* const argv[]){
	Logger logg("omega_log.txt", LOGGING_LEVEL); //Initialize log file
	logg.info("Main", "Program Started");
	
	if(argc < 2){
		logg.error("Main", "No MAC address provided");
		return -1;
	}
	if(!checkMACAddress(argv[1])){
		logg.warning("Main", "MAC address not permitted");
		//promptForPasscode(); + ask if public computer
		cout << "Enter passcode to gain access anyways: ";
		string inputPass;
		getline(cin, inputPass);
		//hash the input
		if(true){ //compare hash to stored passcode
			
		}
	}else{
		cout << "Match found" << endl;
	}
	//logg.info("Main", "MAC address: " + argv[1]);
	
	cout << "Enter command: " << endl;
	
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