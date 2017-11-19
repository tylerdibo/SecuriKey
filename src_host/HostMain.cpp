#include <iostream>
#include <fstream>
#include "Logger.h"
#define LIBSSH_STATIC 1
#include "libssh/libssh.h"

#ifdef WIN32
#include <Rpc.h>
#else
#include <uuid/uuid.h>
#endif

using namespace std;

Logger logg("host_log.txt", LOGGING_LEVEL); //Initialize log file

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

bool sshVerifyOmega(ssh_session omega_sess){
	//Check if omega is in known hosts
	int state = ssh_is_server_known(omega_sess);
	
	switch(state){
		case SSH_SERVER_KNOWN_OK:
			break;
		case SSH_SERVER_KNOWN_CHANGED:
			logg.error("SSH Verification", "Server key has changed, someone may be attempting to tamper with the connection");
			return false;
		case SSH_SERVER_FOUND_OTHER:
			logg.error("SSH Verification", "Wrong type of key, someone may be attempting to tamper with the connection");
			return false;
		case SSH_SERVER_FILE_NOT_FOUND:
			logg.warning("SSH Verification", "Known host file not found, creating file");
			//continue to registering omega
		case SSH_SERVER_NOT_KNOWN:
			logg.info("SSH Verification", "Unknown server");
			cout << "Unknown server. Do you want to connect anyways and register the server? (y/n)" << endl;
			char answer;
			do{
				cin >> answer;				
			}while(answer != 'y' && answer != 'n');
			if(answer == 'n'){
				return false;
			}
			//Write to known hosts file
			if(ssh_write_knownhost(omega_sess) < 0){
				logg.error("SSH Verification", "Writing to known hosts file failed. Exiting...");
				return false;
			}
			break;
		case SSH_SERVER_ERROR:
			logg.error("SSH Verification", "Unknown Error");
			return false;
			break;
	}
	
	return true;
}

int main(const int argc, const char* const argv[]){
	
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
	
	ssh_session omega_ssh = ssh_new();
	int rc;
	if(omega_ssh == NULL){
		return -1;
	}
	
	int verbosity = SSH_LOG_PACKET;
	int port = 22;
	ssh_options_set(omega_ssh, SSH_OPTIONS_HOST, "192.168.7.2");
	ssh_options_set(omega_ssh, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
	ssh_options_set(omega_ssh, SSH_OPTIONS_PORT, &port);
	
	rc = ssh_connect(omega_ssh);
	if(rc != SSH_OK){
		logg.error("Main", "Could not connect to omega. Exiting...");
		return -1;
	}
	
	if(!sshVerifyOmega(omega_ssh)){
		//Error message logged in above function
		return -1;
	}
	
	//MAIN LOOP HERE
	
	
	ssh_disconnect(omega_ssh);
	ssh_free(omega_ssh);
	
	return 0;
}