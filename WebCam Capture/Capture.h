#pragma once
#include "stdafx.h"
#include <cv.h>
#include <highgui.h>
#include <string>
#include <Windows.h>
#include "CameraDS.h"

#define WM_STOP WM_USER+1
#define WM_EXIT WM_USER+2
typedef struct _THREAD_DATA
{
	int nDevice;
	RECT rcWindow;
	std::string strWindowName;
	std::string strDeviceName;
}THREAD_DATA, *PTHREAD_DATA;

BOOL GetCurrentDeviceName(const int& nDeviceID, std::string& strDeviceName);
DWORD WINAPI ThreadProc(LPVOID lpParameter);