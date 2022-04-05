// MonitorToy.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
// use ANSI
#undef UNICODE
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <utility>
#include <string>
#include <exception>
#include "windows.h"
#include "winuser.h"
#include "physicalmonitorenumerationapi.h"

#pragma comment(lib, "Dxva2.lib")

void getErrorAndString(DWORD* error, LPTSTR* errstr) {
	*error = GetLastError();
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM
		| FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, *error, 0, (LPTSTR)errstr, 0, NULL);
}
void handleError(BOOL success) {
	if (success) return;
	DWORD error;
	LPTSTR errstr;
	getErrorAndString(&error, &errstr);
	std::string whatstr = std::string((char*)errstr);
	throw new std::system_error((int)error, std::system_category(), whatstr);
}

std::string	to_string(RECT rect) {
	using namespace std;
	return string("(") + to_string(rect.left) + string(", ") + to_string(rect.top) + string("), ")
		+ string("(") + to_string(rect.right) + string(", ") + to_string(rect.bottom) + string(")");
}

class Monitors {
	static const int MAX_MON = 128;
	HMONITOR hMonitors[MAX_MON];
	LPPHYSICAL_MONITOR pPhysicalMonitorArray = NULL;
	int numPhyMon = 0;
public:
	static BOOL CALLBACK InfoEnumProc(HMONITOR hmonitor, HDC, LPRECT, LPARAM pMonitors) {
		Monitors& self = *reinterpret_cast<Monitors*>(pMonitors);
		if (self.numPhyMon >= MAX_MON) throw new std::runtime_error("Too many monitors");
		self.hMonitors[self.numPhyMon] = hmonitor;
		++self.numPhyMon;
		return TRUE;
	}
	Monitors() {}
	~Monitors() {
		if (pPhysicalMonitorArray) {
			DestroyPhysicalMonitors(numPhyMon, pPhysicalMonitorArray);
			free(pPhysicalMonitorArray);
		}
	}
	void init() {
		BOOL success = EnumDisplayMonitors(NULL, NULL, &InfoEnumProc, reinterpret_cast<LPARAM>(this));
		handleError(success);
		pPhysicalMonitorArray = (LPPHYSICAL_MONITOR) malloc(numPhyMon * sizeof(PHYSICAL_MONITOR));
		if (!pPhysicalMonitorArray) throw new std::bad_alloc();
		for (int i = 0; i < numPhyMon; ++i) {
			success = GetPhysicalMonitorsFromHMONITOR(hMonitors[i], 1, &pPhysicalMonitorArray[i]);
			handleError(success);
		}
		
	}
	DWORD getNumPhyMon() { return numPhyMon; }
	void enumMonitorInfo() {
		for (int i = 0; i < numPhyMon; ++i) {
			MONITORINFOEX monInfo;
			monInfo.cbSize = sizeof(MONITORINFOEX);
			BOOL success = GetMonitorInfo(hMonitors[i], &monInfo);
			handleError(success);
			std::cout << monInfo.szDevice 
				      << ((monInfo.dwFlags & MONITORINFOF_PRIMARY) ? " [PRIMARY]" : " ") << std::endl;
			std::cout << "rcMonitor " << to_string(monInfo.rcMonitor) << std::endl;
			std::cout << "rcWork " << to_string(monInfo.rcWork) << std::endl;
		}
	}
};
int main()
{
	Monitors mon;

	mon.init();
	while (1) {
		std::cout << mon.getNumPhyMon() << std::endl;
		mon.enumMonitorInfo();
		Sleep(5000);
	}
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
