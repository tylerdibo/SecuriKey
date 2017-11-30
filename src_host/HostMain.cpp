#include <iostream>
#include <fstream>
#include <vector>
#include <string.h>
#include <chrono>
#include <thread>
#include "Logger.h"
#define LIBSSH_STATIC 1
#include "libssh/libssh.h"

#ifdef WIN32
#include <Rpc.h>
#else
#include <uuid/uuid.h>
#include <unistd.h>
#endif

using namespace std;

//Global variables
Logger logg;
const int ssh_timeout = 15; //timeout read requests after 15 seconds

void waitFor(unsigned int millis){
#ifdef WIN32
	Sleep(millis);
#else
	usleep(millis*1000);
#endif
}

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
			logg.info("SSH Verification", "New server registered");
			break;
		case SSH_SERVER_ERROR:
			logg.error("SSH Verification", "Unknown Error");
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
	logg.debug("ParseOmegaInput", string(buffer, nBytes));
	string bufStr(buffer, nBytes);
	int start = bufStr.find("%%");
	int end = bufStr.find("$$", start);
	
	if(start == -1 || end == -1){
		return "";
	}
	
	string output = bufStr.substr(start+2, end-start-2); //get string between %% and $$
	return output;
}

vector<string> requestPassForUser(string website, string username, ssh_channel chan){
	int bufSize = 512;
	char buffer[bufSize];
	//Send request to omega
	string toSend = "request " + website + " " + username + "\n";
	int nBytesWritten = ssh_channel_write(chan, toSend.c_str(), toSend.size());
	if(nBytesWritten != (int)toSend.size()){
		logg.error("Get", "Mismatch in bytes to send and bytes sent. Continuing anyways.");
	}
	
	waitFor(500); //Wait for omega to output
	
	//receive username and password
	int nBytesRead = ssh_channel_read_timeout(chan, buffer, bufSize, 0, ssh_timeout);
	if(nBytesRead < 0){
		logg.error("Get", "Error reading bytes from ssh. Exiting...");
		return vector<string>();
	}
	string userpass = parseOmegaInput(buffer, nBytesRead);
	return split(userpass, " ");
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

void closeConnection(ssh_session session, ssh_channel channel){
	if(ssh_channel_is_open(channel)){
		ssh_channel_close(channel);
		ssh_channel_send_eof(channel);
		ssh_channel_free(channel);
	}

	ssh_disconnect(session);
	ssh_free(session);
}

int main(const int argc, const char* const argv[]){
	if(logg.init("host_log.txt", LOGGING_LEVEL) == -1){ //Initialize log file
		cerr << "Logfile failed to initialize. Exiting..." << endl;
		return -1;
	}
	
	cout << "   _____                      _ __ __          \n" <<
			"  / ___/___  _______  _______(_) //_/__  __  __\n" <<
			"  \\__ \\/ _ \\/ ___/ / / / ___/ / ,< / _ \\/ / / /\n" <<
			" ___/ /  __/ /__/ /_/ / /  / / /| /  __/ /_/ / \n" <<
			"/____/\\___/\\___/\\__,_/_/  /_/_/ |_\\___/\\__, /  \n" <<
			"                                      /____/" << endl;
	
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
	
	int verbosity = SSH_LOG_WARNING;
	int port = 22;
	ssh_options_set(omega_ssh, SSH_OPTIONS_HOST, "omega-A030.local");
	ssh_options_set(omega_ssh, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
	ssh_options_set(omega_ssh, SSH_OPTIONS_PORT, &port);
	
	rc = ssh_connect(omega_ssh);
	if(rc != SSH_OK){
		logg.warning("Main", "Could not connect to omega wirelessly. Attempting serial connection.");
		if(!connectSerial()){
			logg.error("Main", "Could not connect to omega through serial. Exiting...");
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
		logg.error("Main", "Error authenticating with password. Exiting...");
		ssh_disconnect(omega_ssh);
		ssh_free(omega_ssh);
		return -1;
	}
	
	//Initialize SSH channel
	ssh_channel channel = ssh_channel_new(omega_ssh);
	if(channel == NULL){
		logg.error("Main", "Could not open SSH channel. Exiting...");
		ssh_disconnect(omega_ssh);
		ssh_free(omega_ssh);
		return -1;
	}
	
	rc = ssh_channel_open_session(channel);
	if(rc != SSH_OK){
		//cerr << ssh_get_error(omega_ssh);
		ssh_channel_free(channel);
		ssh_disconnect(omega_ssh);
		ssh_free(omega_ssh);
		logg.error("Main", "Could not open SSH session. Exiting...");
		return -1;
	}
	
	//Initialize shell
	rc = ssh_channel_request_pty(channel); //Not needed for non-interactive shell
	if (rc != SSH_OK){
		logg.error("Main", "Error opening remote shell [0]. Exiting...");
		return -1;
	}
	rc = ssh_channel_change_pty_size(channel, 160, 48);
	if (rc != SSH_OK){
		logg.error("Main", "Error opening remote shell [1]. Exiting...");
		return -1;
	}
	rc = ssh_channel_request_shell(channel);
	if(rc != SSH_OK){
		logg.error("Main", "Error opening remote shell [2]. Exiting...");
		return -1;
	}
	
	/*rc = ssh_channel_request_exec(channel, "/root/OmegaMain");
	if(rc != SSH_OK){
		logg.error("Main", "Could not execute command.");
		closeConnection(omega_ssh, channel);
		return -1;
	}*/
	string pathToProgram = "/root/OmegaMain test\n";
	int nInitWritten = ssh_channel_write(channel, pathToProgram.c_str(), pathToProgram.size());
	if(nInitWritten != (int)pathToProgram.size()){
		logg.error("Add", "Mismatch in bytes to send and bytes sent. Continuing anyways.");
	}
	
	logg.debug("Main", "Entering main loop");
	
	waitFor(2000); //Delay to let omega output
	
	//MAIN LOOP HERE
	int bufSize = 1024;
	char buffer[bufSize];
	int nBytesRead, nBytesWritten;
	string toSend;
	while(ssh_channel_is_open(channel) && !ssh_channel_is_eof(channel)){
		//Read initial input
		nBytesRead = ssh_channel_read_timeout(channel, buffer, bufSize, 0, ssh_timeout);
		if(nBytesRead < 0){
			logg.error("Main", "Error reading bytes from ssh. Exiting...");
			closeConnection(omega_ssh, channel);
			return -1;
		}
		if(nBytesRead > 0){
			//Display to terminal
			//write(1, buffer, nBytesRead);
			logg.debug("SSH Read", string(buffer, nBytesRead));
		}
		
		cout << "Enter command: ";
		
		//Get input
		string inputStr;
		getline(cin, inputStr);
		
		size_t spaceIndex = inputStr.find_first_of(" ");
		string command = inputStr.substr(0, spaceIndex);
		
		if(command == "add"){
			//Get info from user
			string newWebsite, newUser, newPass;
			cout << "Enter website: ";
			getline(cin, newWebsite);
			cout << "Enter new username: ";
			getline(cin, newUser);
			cout << "Enter new password: (Leave blank for auto generation) ";
			getline(cin, newPass);
			//TODO: clear terminal after each command?
			toSend = "add " + newWebsite + " " + newUser + " " + newPass + "\n";
			logg.debug("SSH Write", "Sending: " + toSend);
			
			//Send to omega
			nBytesWritten = ssh_channel_write(channel, toSend.c_str(), toSend.size());
			if(nBytesWritten != (int)toSend.size()){
				logg.error("Add", "Mismatch in bytes to send and bytes sent. Continuing anyways.");
			}
			
			waitFor(300); //Wait for omega to output
			
			//Receive confirmation
			nBytesRead = ssh_channel_read_timeout(channel, buffer, bufSize, 0, ssh_timeout);
			if(nBytesRead < 0){
				logg.error("Add", "Error reading bytes from ssh. Exiting...");
				closeConnection(omega_ssh, channel);
				return -1;
			}
			string confirmation = parseOmegaInput(buffer, nBytesRead);
			if(confirmation != "Add successful"){
				logg.warning("Add", "No confirmation received from omega. Continuing anyways.");
			}
		}else if(command == "get"){
			//Get info from user
			string website;
			cout << "Enter website: ";
			getline(cin, website);
			
			toSend = "request " + website + "\n";
			logg.debug("SSH Write", "Sending: " + toSend);
			
			//Send to omega
			nBytesWritten = ssh_channel_write(channel, toSend.c_str(), toSend.size());
			if(nBytesWritten != (int)toSend.size()){
				logg.error("Get", "Mismatch in bytes to send and bytes sent. Continuing anyways.");
			}
			
			waitFor(500); //Wait for omega to output
			
			//receive username + password, or list of usernames
			nBytesRead = ssh_channel_read_timeout(channel, buffer, bufSize, 0, ssh_timeout);
			if(nBytesRead < 0){
				logg.error("Get", "Error reading bytes from ssh. Exiting...");
				closeConnection(omega_ssh, channel);
				return -1;
			}
			string returned = parseOmegaInput(buffer, nBytesRead);
			//parse input from omega
			vector<string> returnedVals = split(returned, " ");
			
			//if just one username+password combo
			if(returnedVals.size() == 1){
				string username = returnedVals.at(0);
				vector<string> userpass = requestPassForUser(website, username, channel);
				string password = userpass.at(1);
				
				addToClipboard(username);
				//Wait for user to paste username
				cout << "Username copied to clipboard. Press enter to copy password." << endl;
				string placeholder;
				getline(cin, placeholder);
				addToClipboard(password);
				//Maybe clear the clipboard after a certain amount of time?
			}else if(returnedVals.size() > 1){
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
					cout << "Invalid number" << endl;
				}else{
					//continue
					//Send request to omega
					vector<string> userpassVect = requestPassForUser(website, returnedVals.at(numSelect-1), channel);
					
					addToClipboard(userpassVect.at(0));
					//Wait for user to paste username
					cout << "Username copied to clipboard. Press enter to copy password." << endl;
					string placeholder;
					getline(cin, placeholder);
					addToClipboard(userpassVect.at(1));
					cout << "Password copied." << endl;
					//Maybe clear the clipboard after a certain amount of time?
				}
			}else{
				logg.debug("Get", "No usernames returned from omega.");
				cout << "No credentials found for specified website." << endl;
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
			closeConnection(omega_ssh, channel);
			
			return 0;
		}else{
			cout << "Invalid command. Try 'help' for a list of commands." << endl;
		}
	}
	
	//Closing connections and freeing memory
	closeConnection(omega_ssh, channel);
	
	return 0;
}