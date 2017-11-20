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

bool connectSerial(){
	return false;
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
	
	//Initialize SSH session with omega
	ssh_session omega_ssh = ssh_new();
	int rc;
	if(omega_ssh == NULL){
		return -1;
	}
	
	int verbosity = SSH_LOG_PACKET;
	int port = 22;
	ssh_options_set(omega_ssh, SSH_OPTIONS_HOST, "omega-ABCD.local");
	ssh_options_set(omega_ssh, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
	ssh_options_set(omega_ssh, SSH_OPTIONS_PORT, &port);
	
	rc = ssh_connect(omega_ssh);
	if(rc != SSH_OK){
		logg.warning("Main", "Could not connect to omega wirelessly. Attempting serial connection.");
		if(!connectSerial()){
			logg.error("Main", "Could not connet to omega through serial. Exiting...");
			return -1;
		}
	}
	
	if(!sshVerifyOmega(omega_ssh)){
		//Error message logged in sshVerifyOmega() function
		return -1;
	}
	
	
	//Initialize SSH channel
	ssh_channel channel = ssh_channel_new(omega_ssh);
	if(channel == NULL){
		logg.error("Main", "Could not open SSH channel. Exiting...");
		return -1;
	}
	
	if(ssh_channel_open_session(channel) != SSH_OK){
		ssh_channel_free(channel);
		logg.error("Main", "Could not open SSH session. Exiting...");
		return -1;
	}
	
	//Initialize shell
	rc = ssh_channel_request_pty(channel); //Not needed for non-interactive shell
	if (rc != SSH_OK){
		logg.error("Main", "Error opening remote shell [0]. Exiting...");
		return -1;
	}
	rc = ssh_channel_change_pty_size(channel, 80, 24);
	if (rc != SSH_OK){
		logg.error("Main", "Error opening remote shell [1]. Exiting...");
		return -1;
	}
	rc = ssh_channel_request_shell(channel);
	if (rc != SSH_OK){
		logg.error("Main", "Error opening remote shell [2]. Exiting...");
		return -1;
	}
	
	//MAIN LOOP HERE
	int bufSize = 256;
	char buffer[bufSize];
	int nBytesRead;
	while(ssh_channel_is_open(channel) && !ssh_channel_is_eof(channel)){
		nBytesRead = ssh_channel_read_timeout(channel, buffer, bufSize, 0, -1);
		if(nBytesRead < 0){
			logg.error("Main", "Error reading bytes from ssh. Exiting...");
			return -1;
		}
		if(nBytesRead > 0){
			//Display to terminal
			write(1, buffer, nBytesRead);
			logg.debug("SSH Read", buffer);
		}
	}
	
	//Closing connections and freeing memory
	ssh_channel_close(channel);
	ssh_channel_send_eof(channel);
	ssh_channel_free(channel);
	
	ssh_disconnect(omega_ssh);
	ssh_free(omega_ssh);
	
	return 0;
}