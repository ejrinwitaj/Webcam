#include "stdafx.h"
#include "Capture.h"

BOOL GetCurrentDeviceName(const int& nDeviceID, std::string& strDeviceName)
{
	char cBuf[MAX_PATH] = {0};
	int nResult = CCameraDS::CameraName(nDeviceID, cBuf, sizeof(cBuf));
	if(nResult < 0)
	{
		return FALSE;
	}
	strDeviceName = std::string(cBuf);
	return TRUE;
}

DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
	
	MSG msg;
	CCameraDS camera;
	IplImage  *pFrame = NULL;
	IplImage  *pFrameTransfromed = NULL;
	int nCount = CCameraDS::CameraCount();
	PTHREAD_DATA pData = (PTHREAD_DATA)lpParameter;
	
	if(!GetCurrentDeviceName(pData->nDevice, pData->strDeviceName))
	{
		return - 1;
	}
	
	if(!camera.OpenCamera(pData->nDevice, false))
	{
		return -1;
	}

	pFrame = camera.QueryFrame();
	while(TRUE)
	{
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if(msg.message == WM_EXIT)
			{
				//MessageBox(NULL, TEXT("Got Message"), TEXT("Tip"), MB_ICONINFORMATION);
				break;
			}
		}
		pFrame = camera.QueryFrame();
		
		switch(nCount)
		{
		case 1:
			//pFrame->width = pData->rcWindow.right;
			//pFrame->height = pData->rcWindow.bottom;
			pFrameTransfromed = ::cvCreateImage(::cvSize(pData->rcWindow.right, pData->rcWindow.bottom),
				                                pFrame->depth,
				                                pFrame->nChannels);
			::cvResize(pFrame, pFrameTransfromed);
			
			break;
		case 2:
			pFrameTransfromed = ::cvCreateImage(::cvSize(pData->rcWindow.right / 2, pData->rcWindow.bottom),
				                                pFrame->depth,
				                                pFrame->nChannels);
			::cvResize(pFrame, pFrameTransfromed);

		}
		
		cvShowImage(pData->strWindowName.c_str(), pFrameTransfromed);
		cvReleaseImage(&pFrameTransfromed);
		Sleep(50);
	}
	//cvReleaseImage(&pFrame);
	pFrame = NULL;
	camera.CloseCamera();
	return 0;

}