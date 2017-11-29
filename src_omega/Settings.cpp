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

int Settings::setTime(int begin, int end){
	begin = getStartTime();
	end = getEndTime();
	return begin,end;
}

string Settings::addWifi(string wifi){
	wifiNetworks.add(wifi);
}

string Settings::removeWifi(string wifi){
	for(unsigned int i = 0; i < wifiNetworks.size(); i++){
		if(wifiNetworks.at(i) == wifi){
			wifiNetworks.erase(i);
		}
	}