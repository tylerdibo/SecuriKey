#include "Settings.h"
#include <vector>
#include <iostream>
#include <chrono>
#include <string>

using namespace std;

Settings::Settings(){

}

int Settings::getStartTime(){
	return startTime;
}
int Settings::getEndTime(){
	return endTime;
}

string Settings::getWifiNetworks(){
	for(unsigned int i = 0; i < wifiNetworks.size(); i++){
		return wifiNetworks.at(i);
	}
}

void Settings::setTime(int begin, int end){
	startTime = begin;
	endTime = end;
}

string Settings::addWifi(string wifi){
	wifiNetworks.push_back(wifi);
}

string Settings::removeWifi(string wifi){
	for(vector<string>::iterator i = wifiNetworks.begin(); i != wifiNetworks.end(); i++){
		if(*i == wifi){
			wifiNetworks.erase(i);
		}
	}
}
