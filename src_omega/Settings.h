#ifndef __SETTINGS_H
#define __SETTINGS_H
#include <vector>
#include <chrono>
#include <string>

using namespace std;

class Settings{
private:
	int startTime;
	int endTime;
	vector<string> wifiNetworks;

public:
	Settings();
	int getStartTime();
	int getEndTime();
	string getWifiNetworks();
	void setTime(int begin, int end);
	string addWifi(string wifi);
	string removeWifi(string wifi);
};

#endif /* __SETTINGS_H */
