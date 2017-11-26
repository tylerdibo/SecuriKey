#include <iostream>
#include <fstream>
#include <vector>
#include "Logger.h"
#define LIBSSH_STATIC 1
#include "libssh/libssh.h"

#ifdef WIN32
#include <Rpc.h>
#else
#include <uuid/uuid.h>
#endif

using namespace std;

//Global variables
Logger* logg;
const int ssh_timeout = -1;

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
			logg->error("SSH Verification", "Server key has changed, someone may be attempting to tamper with the connection");
			return false;
		case SSH_SERVER_FOUND_OTHER:
			logg->error("SSH Verification", "Wrong type of key, someone may be attempting to tamper with the connection");
			return false;
		case SSH_SERVER_FILE_NOT_FOUND:
			logg->warning("SSH Verification", "Known host file not found, creating file");
			//continue to registering omega
		case SSH_SERVER_NOT_KNOWN:
			logg->info("SSH Verification", "Unknown server");
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
				logg->error("SSH Verification", "Writing to known hosts file failed. Exiting...");
				return false;
			}
			logg->info("SSH Verification", "New server registered");
			break;
		case SSH_SERVER_ERROR:
			logg->error("SSH Verification", "Unknown Error");
			return false;
			break;
	}
	
	return true;
}

vector<string> split(string str, string sep){
    char* cstr = const_cast<char*>(str.c_str());	// const_cast used as c_str() returns const char*
    char* currentStr;
    vector<string> arr;
    currentStr = strtok(cstr, sep.c_str());

    while(currentStr != NULL){
        arr.push_back(currentStr);
        currentStr = strtok(NULL, sep.c_str());  // passing NULL lets strtok know that you are tokenizing same string
    }

    return arr;
}

string parseOmegaInput(char* buffer, int nBytes){
	string bufStr(buffer, nBytes);
	int start = bufStr.find("%%");
	int end = bufStr.find("$$", start);
	
	string output = bufStr.substr(start+2, end-start-2); //get string between %% and $$
	return output;
}

int addToClipboard(string toAdd){
	#ifdef WIN32
	OpenClipboard(0);
	EmptyClipboard();	
	HGLOBAL hg=GlobalAlloc(GMEM_MOVEABLE, toAdd.size()+1);
	if (!hg){
		CloseClipboard();
		return -1;
	}
	memcpy(GlobalLock(hg), toAdd.c_str(), toAdd.size()+1);
	GlobalUnlock(hg);
	SetClipboardData(CF_TEXT, hg);
	CloseClipboard();
	GlobalFree(hg);
	#endif
	
	return 0;
}

int main(const int argc, const char* const argv[]){
	try{
		logg = new Logger("host_log.txt", LOGGING_LEVEL); //Initialize log file
	}catch(int e){
		cerr << "Logfile failed to initialize. Exiting..." << endl;
		return -1;
	}
	
	cout << "   _____                      _ __ __          \n" <<
			"  / ___/___  _______  _______(_) //_/__  __  __\n" <<
			"  \\__ \\/ _ \\/ ___/ / / / ___/ / ,< / _ \\/ / / /\n" <<
			" ___/ /  __/ /__/ /_/ / /  / / /| /  __/ /_/ / \n" <<
			"/____/\\___/\\___/\\__,_/_/  /_/_/ |_\\___/\\__, /  \n" <<
			"                                      /____/" << endl;
	
	logg->info("Main", "Program Started");
	
	//Send raw UUID to omega, have omega hash it
	string uuid = readUUID("UUID");
	if(uuid == ""){
		logg->warning("Main", "Could not read UUID from file, generating a new one...");
		uuid = newUUID();
		//save to disk
		writeUUID("UUID", uuid);
	}
	logg->debug("Main", "UUID is " + uuid);
	//sendUUIDToOmega();
	
	//Initialize SSH session with omega
	ssh_session omega_ssh = ssh_new();
	int rc;
	if(omega_ssh == NULL){
		return -1;
	}
	
	int verbosity = SSH_LOG_PACKET;
	int port = 22;
	ssh_options_set(omega_ssh, SSH_OPTIONS_HOST, "omega-A030.local");
	ssh_options_set(omega_ssh, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
	ssh_options_set(omega_ssh, SSH_OPTIONS_PORT, &port);
	
	rc = ssh_connect(omega_ssh);
	if(rc != SSH_OK){
		logg->warning("Main", "Could not connect to omega wirelessly. Attempting serial connection.");
		if(!connectSerial()){
			logg->error("Main", "Could not connect to omega through serial. Exiting...");
			return -1;
		}
	}
	
	//Authenticate omega
	if(!sshVerifyOmega(omega_ssh)){
		//Error message logged in sshVerifyOmega() function
		ssh_disconnect(omega_ssh);
		ssh_free(omega_ssh);
		return -1;
	}
	
	//Authenticate host
	rc = ssh_userauth_password(omega_ssh, "root", "onioneer"); //Change to custom user account
	if(rc != SSH_AUTH_SUCCESS){
		logg->error("Main", "Error authenticating with password. Exiting...");
		ssh_disconnect(omega_ssh);
		ssh_free(omega_ssh);
		return -1;
	}
	
	//Initialize SSH channel
	ssh_channel channel = ssh_channel_new(omega_ssh);
	if(channel == NULL){
		logg->error("Main", "Could not open SSH channel. Exiting...");
		return -1;
	}
	
	rc = ssh_channel_open_session(channel);
	if(rc != SSH_OK){
		//cerr << ssh_get_error(omega_ssh);
		ssh_channel_free(channel);
		logg->error("Main", "Could not open SSH session. Exiting...");
		return -1;
	}
	
	//Initialize shell
	rc = ssh_channel_request_pty(channel); //Not needed for non-interactive shell
	/*if (rc != SSH_OK){
		logg->error("Main", "Error opening remote shell [0]. Exiting...");
		return -1;
	}
	rc = ssh_channel_change_pty_size(channel, 80, 24);
	if (rc != SSH_OK){
		logg->error("Main", "Error opening remote shell [1]. Exiting...");
		return -1;
	}*/
	rc = ssh_channel_request_shell(channel);
	if (rc != SSH_OK){
		logg->error("Main", "Error opening remote shell [2]. Exiting...");
		return -1;
	}
	
	//MAIN LOOP HERE
	int bufSize = 1024;
	char buffer[bufSize];
	int nBytesRead, nBytesWritten;
	string toSend;
	while(ssh_channel_is_open(channel) && !ssh_channel_is_eof(channel)){
		//Read initial input
		nBytesRead = ssh_channel_read_timeout(channel, buffer, bufSize, 0, ssh_timeout);
		if(nBytesRead < 0){
			logg->error("Main", "Error reading bytes from ssh. Exiting...");
			return -1;
		}
		if(nBytesRead > 0){
			//Display to terminal
			//write(1, buffer, nBytesRead);
			logg->debug("SSH Read", buffer);
		}
		
		//Get input
		string inputStr;
		getline(cin, inputStr);
		
		size_t spaceIndex = inputStr.find_first_of(" ");
		string command = inputStr.substr(0, spaceIndex);
		
		if(command == "add"){
			//Get info from user
			string newWebsite, newUser, newPass;
			cout << "Enter website: " << endl;
			getline(cin, newWebsite);
			cout << "Enter new username: " << endl;
			getline(cin, newUser);
			cout << "Enter new password: (Leave blank for auto generation)" << endl;
			getline(cin, newPass);
			toSend = "add " + newWebsite + " " + newUser + " " + newPass;
			logg->debug("SSH Write", "Sending: " + toSend);
			
			//Send to omega
			nBytesWritten = ssh_channel_write(channel, toSend.c_str(), toSend.size()); //TODO: test this
			if(nBytesWritten != (int)toSend.size()){
				logg->error("Add", "Mismatch in bytes to send and bytes sent. Continuing anyways.");
			}
			
			//Receive confirmation
			nBytesRead = ssh_channel_read_timeout(channel, buffer, bufSize, 0, ssh_timeout);
			if(nBytesRead < 0){
				logg->error("Add", "Error reading bytes from ssh. Exiting...");
				return -1;
			}
			string confirmation = parseOmegaInput(buffer, nBytesRead);
			if(confirmation != "Add successful"){
				logg->warning("Add", "No confirmation received from omega. Continuing anyways.");
			}
		}else if(command == "get"){
			//Get info from user
			string website;
			cout << "Enter website: " << endl;
			getline(cin, website);
			
			toSend = "request " + website;
			logg->debug("SSH Write", "Sending: " + toSend);
			
			//Send to omega
			nBytesWritten = ssh_channel_write(channel, toSend.c_str(), toSend.size()); //TODO: test this
			if(nBytesWritten != (int)toSend.size()){
				logg->error("Get", "Mismatch in bytes to send and bytes sent. Continuing anyways.");
			}
			
			//receive username + password, or list of usernames
			nBytesRead = ssh_channel_read_timeout(channel, buffer, bufSize, 0, ssh_timeout);
			if(nBytesRead < 0){
				logg->error("Get", "Error reading bytes from ssh. Exiting...");
				return -1;
			}
			string returned = parseOmegaInput(buffer, nBytesRead);
			//parse input from omega
			vector<string> returnedVals = split(returned, " ");
			
			//if just one username+password combo
			if(returnedVals.size() == 2){
				string username = returnedVals.at(0);
				string password = returnedVals.at(1);
				
				addToClipboard(username);
				//Wait for user to paste username
				string placeholder;
				getline(cin, placeholder);
				addToClipboard(password);
				//Maybe clear the clipboard after a certain amount of time?
			}else if(returnedVals.size() > 2){
				//prompt for user to input number to select username
				cout << "Press the number for the desired username:" << endl;
				for(unsigned int i = 0; i < returnedVals.size(); i++){
					cout << i+1 << ": " << returnedVals.at(i) << endl;
				}
				
				string selection;
				getline(cin, selection);
				int numSelect = stoi(selection); //TODO: catch error from inputting non-integer
				
				if(numSelect < 1 || numSelect > (int)returnedVals.size()){
					//exit if statement
				}else{
					//continue
					//Send request to omega
					toSend = "request " + website + " " + returnedVals.at(numSelect-1);
					nBytesWritten = ssh_channel_write(channel, toSend.c_str(), toSend.size()); //TODO: test this
					if(nBytesWritten != (int)toSend.size()){
						logg->error("Get", "Mismatch in bytes to send and bytes sent. Continuing anyways.");
					}
					
					//receive username and password
					nBytesRead = ssh_channel_read_timeout(channel, buffer, bufSize, 0, ssh_timeout);
					if(nBytesRead < 0){
						logg->error("Get", "Error reading bytes from ssh. Exiting...");
						return -1;
					}
					string userpass = parseOmegaInput(buffer, nBytesRead);
					vector<string> userpassVect = split(returned, " ");
					
					addToClipboard(userpassVect.at(0));
					//Wait for user to paste username
					string placeholder;
					getline(cin, placeholder);
					addToClipboard(userpassVect.at(1));
					//Maybe clear the clipboard after a certain amount of time?
				}
			}
		}else if(command == "help"){
			//Display list of commands with their explanations
			cout << "Available commands: " << endl << endl;
			cout << "add:\tSend a new user account to the omega to be stored and encrypted." << endl;
			cout << "get:\tRetrieve the user account from the omega for the given website." << endl;
			cout << "\tIf more than one user account exists for the website, a username must be selected." << endl;
			cout << "help:\tDisplay this list of commands." << endl;
			cout << "exit:\tGracefully exit the program." << endl;
		}else if(command == "exit"){
			//Closing connections and freeing memory
			ssh_channel_close(channel);
			ssh_channel_send_eof(channel);
			ssh_channel_free(channel);
			
			ssh_disconnect(omega_ssh);
			ssh_free(omega_ssh);
			
			delete logg;
			
			return 0;
		}
	}
	
	//Closing connections and freeing memory
	ssh_channel_close(channel);
	ssh_channel_send_eof(channel);
	ssh_channel_free(channel);
	
	ssh_disconnect(omega_ssh);
	ssh_free(omega_ssh);
	
	delete logg;
	
	return 0;
}