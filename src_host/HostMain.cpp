#include <iostream>
#include <uuid/uuid.h>
#include <fstream>
#include "Logger.h"
#define LIBSSH_STATIC 1
#include "libssh/libssh.h"

using namespace std;

std::string newUUID(){ //credit to: https://stackoverflow.com/questions/543306/platform-independent-guid-generation-in-c
#ifdef WIN32
    UUID uuid;
    UuidCreate(&uuid);

    unsigned char* str;
    UuidToStringA(&uuid, &str);

    std::string uuidStr((char*)str);

    RpcStringFreeA(&str);
#else
    uuid_t uuid;
    uuid_generate_random(uuid);
    char uuidStr[37];
    uuid_unparse(uuid, uuidStr);
#endif
    return uuidStr;
}

bool writeUUID(const char* filename, string uuid){
	ofstream uuid_file;
	uuid_file.open(filename);
	if(!uuid_file.is_open()){
		return false;
	}
	
	uuid_file << uuid;
	return true;
}

string readUUID(const char* filename){
	ifstream uuid_file;
	uuid_file.open(filename);
	if(!uuid_file.is_open()){
		return "";
	}
	
	const int maxLineLength = 37;
	char line[maxLineLength];
	if(!uuid_file.getline(line, maxLineLength)){
		return "";
	}
	return line;
}

int main(const int argc, const char* const argv[]){
	Logger logg("host_log.txt", LOGGING_LEVEL); //Initialize log file
	logg.info("Main", "Program Started");
	
	//Send raw UUID to omega, have omega hash it
	string uuid = readUUID("UUID");
	if(uuid == ""){
		logg.warning("Main", "Could not read UUID from file, generating a new one...");
		uuid = newUUID();
		//save to disk
		writeUUID("UUID", uuid);
	}
	logg.debug("Main", "UUID is " + uuid);
	//sendUUIDToOmega();
	
	ssh_session test_ssh_session = ssh_new();
	if(test_ssh_session == NULL){
		return -1;
	}
	ssh_free(test_ssh_session);
	
	return 0;
}