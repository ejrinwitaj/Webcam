#pragma once

#include "resource.h"
#include "Capture.h"

#define MAX_LOADSTRING 100

// ȫ�ֱ���:
HINSTANCE hInst;								// ��ǰʵ��
TCHAR szTitle[MAX_LOADSTRING];					// �������ı�
TCHAR szWindowClass[MAX_LOADSTRING];			// ����������

// �˴���ģ���а����ĺ�����ǰ������:
//ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
//LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DialogMain(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);


INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

//LONG WINAPI UnhandledExceptionFilter(struct _EXCEPTION_POINTERS *ExceptionInfo);

