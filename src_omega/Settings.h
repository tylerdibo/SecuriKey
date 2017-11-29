#ifndef __SETTINGS_H
#define __SETTINGS_H
#include <vector>
#include <chrono>
#include <string>

class Settings{
private:
	int startTime;
	int endtime;
	vector<string> wifiNetworks();

public:
	int getStartTime();
	int getEndTime();
	string getWifiNetworks();
	int setTime();
	string addWifi(string wifi);
	string removeWifi(string wifi);
	};

#endif /* __SETTINGS_H */