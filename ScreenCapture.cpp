// ScreenCapture.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "ScreenCapture.h"

#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst;								// 当前实例
TCHAR szTitle[MAX_LOADSTRING];					// 标题栏文本
TCHAR szWindowClass[MAX_LOADSTRING];			// 主窗口类名

// 此代码模块中包含的函数的前向声明:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

HDC g_srcMemDc;
HDC g_grayMemDc;
int screenW;
int screenH;

RECT rect = {0};   //画图的矩形区域
bool isDrawing = false;
bool isSelect = false;


void GetScreenCapture();
void CovertToGrayBitmap(HBITMAP hSourceBmp,HDC sourceDc);
void WriteDatatoClipBoard();

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: 在此放置代码。
	MSG msg;
	HACCEL hAccelTable;

	// 初始化全局字符串
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_SCREENCAPTURE, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// 执行应用程序初始化:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SCREENCAPTURE));

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
}



//
//  函数: MyRegisterClass()
//
//  目的: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SCREENCAPTURE));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= 0;//MAKEINTRESOURCE(IDC_SCREENCAPTURE);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

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
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // 将实例句柄存储在全局变量中

   hWnd = CreateWindow(szWindowClass, szTitle, WS_POPUP,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, SW_MAXIMIZE);
   UpdateWindow(hWnd);

   return TRUE;
}

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
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	LOGBRUSH brush;
	brush.lbStyle=BS_NULL;
	HBRUSH hBrush=CreateBrushIndirect(&brush);

	LOGPEN pen;
	POINT penWidth;
	penWidth.x=2;
	penWidth.y=2;
	pen.lopnColor=0x0000FFFF;
	pen.lopnStyle=PS_SOLID;
	pen.lopnWidth=penWidth;
	HPEN hPen=CreatePenIndirect(&pen);


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
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

	case WM_CREATE:
		GetScreenCapture();
		break;
	case WM_PAINT:
		{
			hdc = BeginPaint(hWnd, &ps);
	
			HDC memDc = CreateCompatibleDC(hdc);
			HBITMAP bmp = CreateCompatibleBitmap(hdc, screenW, screenH);
			SelectObject(memDc, bmp);
	
			BitBlt(memDc, 0, 0, screenW,screenH, g_grayMemDc, 0, 0, SRCCOPY);
			SelectObject(memDc, hBrush);
			SelectObject(memDc, hPen);

			if (isDrawing || isSelect)
			{
				
				BitBlt(memDc, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, g_srcMemDc, rect.left, rect.top, SRCCOPY);
				Rectangle(memDc, rect.left, rect.top, rect.right, rect.bottom);
			}

			BitBlt(hdc, 0, 0, screenW, screenH, memDc, 0, 0, SRCCOPY);

			DeleteObject(bmp);
			DeleteObject(memDc);
	
			EndPaint(hWnd, &ps);
			}
		break;

	case WM_LBUTTONDOWN:
		{
			if (!isSelect)
			{
				POINT pt;
				GetCursorPos(&pt);
				rect.left = pt.x;
				rect.top = pt.y;
				rect.right =  pt.x;
				rect.bottom = pt.y;

				isDrawing = true;
				InvalidateRgn(hWnd, 0, false);
			}
		}
		break;

	case WM_LBUTTONUP:
		{
			if (isDrawing && !isSelect)
			{
				isDrawing = false;
				POINT pt;
				GetCursorPos(&pt);
				rect.right = pt.x;
				rect.bottom = pt.y;

				isSelect = true;

				InvalidateRgn(hWnd, 0, false);
			}
		}
		break;

	case WM_MOUSEMOVE:
		{
			if (isDrawing&& !isSelect)
			{
				POINT pt;
				GetCursorPos(&pt);
				rect.right = pt.x;
				rect.bottom = pt.y;
				InvalidateRgn(hWnd, 0, false);
			}
		}
		break;
	case WM_LBUTTONDBLCLK:
		{
			if (isSelect)
			{
				WriteDatatoClipBoard();
				InvalidateRgn(hWnd, 0, false);
				ShowWindow(hWnd, SW_MINIMIZE);
			}
			isSelect = false;
			
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

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


void GetScreenCapture()
{
	HDC disDc = ::CreateDC(L"DISPLAY", 0, 0, 0);  //创建屏幕相关的DC
	screenW = GetDeviceCaps(disDc, HORZRES);//水平分辨率
	screenH = GetDeviceCaps(disDc, VERTRES);//垂直分辨率
	
	g_srcMemDc = CreateCompatibleDC(disDc);  //创建于屏幕兼容的DC（内存DC）
	HBITMAP hbMap = CreateCompatibleBitmap(disDc, screenW, screenH);  //模拟一张画布，其中是没有数据的
	SelectObject(g_srcMemDc, hbMap);   //将画图选入内存DC，其中还是没有数据的
	BitBlt(g_srcMemDc, 0, 0, screenW, screenH, disDc, 0, 0, SRCCOPY);  //将屏幕的dc中的画图，拷贝至内存DC中

	//获取屏幕的灰度图片
	g_grayMemDc = CreateCompatibleDC(disDc);
	HBITMAP grayMap = CreateCompatibleBitmap(disDc, screenW, screenH);  //模拟一张画布，其中是没有数据的
	SelectObject(g_grayMemDc, grayMap);   //将画图选入内存DC，其中还是没有数据的
	BitBlt(g_grayMemDc, 0, 0, screenW, screenH, disDc, 0, 0, SRCCOPY);  //将屏幕的dc中的画图，拷贝至内存DC中

	CovertToGrayBitmap(grayMap, g_grayMemDc);  //将彩色图片转换灰度图片

	DeleteObject(hbMap);
	DeleteObject(grayMap);
	DeleteObject(disDc);
}

void CovertToGrayBitmap(HBITMAP hSourceBmp,HDC sourceDc)
{
	HBITMAP retBmp=hSourceBmp;
	BITMAPINFO bmpInfo;
	ZeroMemory(&bmpInfo,sizeof(BITMAPINFO));
	bmpInfo.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
	GetDIBits(sourceDc,retBmp,0,0,NULL,&bmpInfo,DIB_RGB_COLORS);

	BYTE* bits=new BYTE[bmpInfo.bmiHeader.biSizeImage];
	GetBitmapBits(retBmp,bmpInfo.bmiHeader.biSizeImage,bits);

	int bytePerPixel=4;//默认32位
	if(bmpInfo.bmiHeader.biBitCount==24)
	{
		bytePerPixel=3;
	}
	for(DWORD i=0;i<bmpInfo.bmiHeader.biSizeImage;i+=bytePerPixel)
	{
		BYTE r=*(bits+i);
		BYTE g=*(bits+i+1);
		BYTE b=*(bits+i+2);
		*(bits+i)=*(bits+i+1)=*(bits+i+2)=(r+b+g)/3;
	}
	SetBitmapBits(hSourceBmp,bmpInfo.bmiHeader.biSizeImage,bits);
	delete[] bits;
}

void WriteDatatoClipBoard()
{
	HDC hMemDc,hScrDc;
	HBITMAP hBmp,hOldBmp;
	int width,height;
	width=rect.right-rect.left;
	height=rect.bottom-rect.top;

	hScrDc=CreateDC(L"DISPLAY",NULL,NULL,NULL);
	hMemDc=CreateCompatibleDC(hScrDc);
	hBmp=CreateCompatibleBitmap(hScrDc,width,height);
	
	hOldBmp=(HBITMAP)SelectObject(hMemDc,hBmp);
	BitBlt(hMemDc,0,0,width,height,g_srcMemDc,rect.left,rect.top,SRCCOPY);
	hBmp=(HBITMAP)SelectObject(hMemDc,hOldBmp);
	DeleteDC(hMemDc);
	DeleteDC(hScrDc);
	//复制到剪贴板
	if(OpenClipboard(0))
	{
		EmptyClipboard();
		SetClipboardData(CF_BITMAP,hBmp);
		CloseClipboard();
	}

	DeleteObject(hBmp);
	DeleteObject(hMemDc);
	DeleteObject(hScrDc);
	
}
