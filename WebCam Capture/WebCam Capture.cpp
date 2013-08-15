// WebCam Cpture.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "WebCam Capture.h"



int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: 在此放置代码。
	//MSG msg;
	//HACCEL hAccelTable;

	// 初始化全局字符串
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_WEBCAMCPTURE, szWindowClass, MAX_LOADSTRING);
	//MyRegisterClass(hInstance);

	// 执行应用程序初始化:
	/*
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}
	*/

	/*
	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WEBCAMCPTURE));

	// 主消息循环:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
	*/
	return DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG_MAIN), NULL, DialogMain);
}



//
//  函数: MyRegisterClass()
//
//  目的: 注册窗口类。
//
/*
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WEBCAMCPTURE));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_WEBCAMCPTURE);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}
*/
//
//   函数: InitInstance(HINSTANCE, int)
//
//   目的: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
/*
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // 将实例句柄存储在全局变量中

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}
*/
//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的: 处理主窗口的消息。
//
//  WM_COMMAND	- 处理应用程序菜单
//  WM_PAINT	- 绘制主窗口
//  WM_DESTROY	- 发送退出消息并返回
//
//

INT_PTR CALLBACK DialogMain(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static THREAD_DATA sThreadData[2];
	static HANDLE      hThread[2];
	static DWORD       dwTid[2];
	RECT               rcWindow;
	HWND               hWndLeft;
	HWND               hWndRight;
	int    nCount = CCameraDS::CameraCount();
	LONG   lWindowStyle;
	SystemParametersInfo(SPI_GETWORKAREA, 0, (PVOID)&rcWindow, 0);
	switch(uMsg)
	{
	case WM_INITDIALOG:	
			switch(nCount)
			{
			case 0:
				MessageBox(hwndDlg, TEXT("No webcam detected!"), TEXT("Info"), MB_ICONINFORMATION);
				break;
			case 1:
				sThreadData[0].nDevice = 0;
				sThreadData[0].strWindowName = "Video Output Window Left";
				cvNamedWindow(sThreadData[0].strWindowName.c_str());
				sThreadData[0].rcWindow = rcWindow;
				//GetClientRect(hWnd, &rcWindow);
				hWndLeft = FindWindow(TEXT("Main HighGUI class"), TEXT("Video Output Window Left"));
				_ASSERT(hWndLeft != NULL);
				lWindowStyle = GetWindowLong(hWndLeft, GWL_STYLE);
				lWindowStyle &= ~WS_CAPTION;
				SetWindowLong(hWndLeft, GWL_STYLE, lWindowStyle);
				MoveWindow(hWndLeft, rcWindow.left, rcWindow.top, rcWindow.right, rcWindow.bottom, TRUE);
				//SetParent(hWndLeft, hWnd);
				//MoveWindow(hWndLeft, rcWindow.left, rcWindow.right, rcWindow.right, rcWindow.bottom, TRUE);
				hThread[0] = CreateThread(NULL,
					                      0,
										  ThreadProc,
										  (LPVOID)&sThreadData[0],
										  0,
										  &dwTid[0]);
				break;
			case 2:
				for(int i = 0; i < nCount; ++i)
				{
					sThreadData[i].nDevice = i;
					switch(i)
					{
					case 0:
						sThreadData[i].strWindowName = "Video Output Window Left";
						break;
					case 1:
						sThreadData[i].strWindowName = "Video Output Window Right";
						break;
					}
					cvNamedWindow(sThreadData[i].strWindowName.c_str(), CV_WINDOW_AUTOSIZE);
					//GetWindowRect(hWnd, &sThreadData[0].rcWindow);
					
					sThreadData[i].rcWindow = rcWindow;
					hThread[i] = CreateThread(NULL,
					                          0,
										      ThreadProc,
										      (LPVOID)&sThreadData[i],
										      0,
										      &dwTid[i]);
				}
				//GetWindowRect(hWnd, &rcWindow);
				hWndLeft = FindWindow(TEXT("Main HighGUI class"), TEXT("Video Output Window Left"));
				hWndRight = FindWindow(TEXT("Main HighGUI class"), TEXT("Video Output Window Right"));
				_ASSERT(hWndLeft != NULL);
				_ASSERT(hWndRight != NULL);
				lWindowStyle = GetWindowLong(hWndLeft, GWL_STYLE);
				lWindowStyle &= ~WS_CAPTION;
				SetWindowLong(hWndLeft, GWL_STYLE, lWindowStyle);
				lWindowStyle = GetWindowLong(hWndRight, GWL_STYLE);
				lWindowStyle &= ~WS_CAPTION;
				SetWindowLong(hWndRight, GWL_STYLE, lWindowStyle);
				
				MoveWindow(hWndLeft, rcWindow.left, rcWindow.top, rcWindow.right / 2, rcWindow.bottom /2, TRUE);
				MoveWindow(hWndRight, rcWindow.right / 2, rcWindow.top, rcWindow.right / 2, rcWindow.bottom /2, TRUE);
				//SetParent(hWndLeft, hWnd);
				//SetParent(hWndRight, hWnd);
				
				break;
			default:
				MessageBox(hwndDlg, TEXT("Webcam count not supported!"), TEXT("Error"), MB_ICONSTOP);
			}

		return (INT_PTR)TRUE;
	case WM_CLOSE:
		switch(nCount)
		{
		case 1:
			PostThreadMessage(dwTid[0], WM_EXIT, NULL, NULL);
			cvDestroyWindow(sThreadData[0].strWindowName.c_str());
			WaitForSingleObject(hThread[0], INFINITE);
			CloseHandle(hThread[0]);
			break;
		case 2:
			for(int i = 0; i < nCount; ++i)
			{

				PostThreadMessage(dwTid[i], WM_EXIT, NULL, NULL);
				cvDestroyWindow(sThreadData[i].strWindowName.c_str());
				WaitForSingleObject(hThread[i], INFINITE);
				CloseHandle(hThread[i]);
			}

		}
		EndDialog(hwndDlg, 0);
		return (INT_PTR)TRUE;
	}
	return (INT_PTR)0;
} 

/*
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	//HWND hTarget;
	
	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// 分析菜单选择:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			//PostThreadMessage(dwTID, WM_STOP, NULL, NULL);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: 在此添加任意绘图代码...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		
		
		PostQuitMessage(0);
		//ExitProcess(0);
		break;
	case WM_CREATE:
		{
		
		}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
*/
/*
LONG WINAPI UnhandledExceptionFilter(struct _EXCEPTION_POINTERS *ExceptionInfo)
{
	TerminateProcess(GetCurrentProcess(), -1);
	return EXCEPTION_EXECUTE_HANDLER; 
}
*/
// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
