#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <string.h>
#include <tuple>
#include "Logger.h"

using namespace std;

Logger logg;

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

string encryptDecrypt(string toEncrypt) {
    char key[6] = {'K', 'C', 'Q', 2, 'e', '+'};
    string output = toEncrypt;
    
    for (int i = 0; i < toEncrypt.size(); i++)
        output[i] = toEncrypt[i] ^ key[i % (sizeof(key) / sizeof(char))];
    
    return output;
}

bool encrypt(string filename){
    ifstream in(filename);

    vector<string> userString;
    string str;
    string encrypted;
    while (getline(in, str)){
        encrypted = encryptDecrypt(str);
        userString.push_back(encrypted);
    }

    ofstream out(filename);
    for(int i=0; i < userString.size(); i++){
       out << userString[i] << endl;
    }

    in.close();
    out.close();

    return 0;
}

bool decrypt(string filename){
    ifstream in(filename);

    vector<string> userString;
    string str;
    string encrypted;
    while (getline(in, str)){
        encrypted = encryptDecrypt(str);
        userString.push_back(encrypted);
    }

    ofstream out(filename);
    for(int i=0; i < userString.size(); i++){
       out << userString[i] << endl;
    }

    in.close();
    out.close();

    return 0;
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

tuple<string, string, bool> search(string filename, string website){
  ifstream in(filename);
  vector<string> list;
  string str = "";
  string user = "";
  string pass = "";
  bool notFound = true;

  while (getline(in, str) && notFound){
    list = split(str, " ");
    if (list[0] == website){
      user = list[1];
      pass = list[2];
      notFound = false;
    }
  }

  return make_tuple(user, pass, notFound);
}

int main(const int argc, const char* const argv[]){
	if(logg.init("omega_log.txt", LOGGING_LEVEL) == -1){ //Initialize log file
		cerr << "Logfile failed to initialize. Exiting..." << endl;
		return -1;
	}

	logg.info("Main", "Program Started");
	
	if(argc < 2){
		logg.error("Main", "No MAC address provided");
		return -1;
	}

  bool exit = false;

	if(!checkMACAddress(argv[1])){
		logg.warning("Main", "MAC address not permitted");
    logg.info("Main", "Exiting program...");
    exit = true;
	}else{
		cout << "Match found" << endl;
	}
	//logg.info("Main", "MAC address: " + argv[1]);
	
	cout << "Enter command: " << endl;
	
	//MAIN PROGRAM LOOP
	while(!exit){
    string inputStr;
    getline(cin, inputStr);

    string filename;
    string website;

    vector<string> input = split(inputStr, " ");

    int size = static_cast<int>(input.size());

    if (size < 1){
      logg.error("Main", "Insufficient number of arguments");
      return -1;
    } else {
      string command = input[0];
      logg.debug("Main", "Command entered: " + command);
    }

    if(command == "request"){

      if (size < 2){
        logg.error("Main", "Insufficient number of arguments");
        return -1;
      }

      filename = "9d0bnLHA7HWB.txt";
      string user = "";
      string pass = "";
      bool err = true;

      website = input[1];

      decrypt(filename);
      tie(user, pass, err) = search(filename, website);

      if (!err){
        // send credentials to hostMain
        cout << "%%" << user << " " << pass << "$$" << endl;
      }

      encrypt(filename);

    }
    else if(command == "add"){

      if (size < 4){
        logg.error("Main", "Insufficient number of arguments");
        return -1;
      }

      filename = "9d0bnLHA7HWB.txt";
      website = input[1];
      string user = input[2];
      string pass = input[3];

      decrypt(filename);

      ofstream file;

      file.open(filename, ios::app);
      string outputString = website + " " + user + " " + pass; 
      file << outputString << endl;

      file.close();

      encrypt(filename);

		}
    else if(command == "getsettings"){
			
		}
    else if(command == "setsettings"){
			
		}
    else if(command == "exit"){
			logg.info("Main", "Exiting program...");
			exit = true;
		}
    else if(command == "help"){
			//printHelp();
		}
    else{
			logg.warning("Main", "Invalid command");
		}
	}
	
	return 0;
}