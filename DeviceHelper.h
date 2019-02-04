#pragma once

#include <string>
#include <vector>
using namespace std;

string getDeviceUniqueIdentifier();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
bool getWinMacAddress(string& macstring);
#endif

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
string getDeviceInfo();
#endif