// Win32.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Win32.h"

//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------


#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HWND g_debuggerHWND = nullptr;
HWND g_debuggerSpewHWND = nullptr;
bool g_bDebuggerActive = false;
BBC_Emulator* g_emulator = nullptr;
int g_nStep = 0;
bool g_bRun = false;

//-------------------------------------------------------------------------------------------------

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Debugger(HWND, UINT, WPARAM, LPARAM);
//-------------------------------------------------------------------------------------------------

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);


    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_BBC_EMULATOR, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_BBC_EMULATOR));
    MSG msg;

	static const int nNumSpewLines = 32;
	std::string debugSpew;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
		if ( g_bDebuggerActive )
		{
			if ( g_bRun )
			{
				if ( g_emulator->RunFrame( &debugSpew ) )
				{
					SetWindowTextA( g_debuggerSpewHWND, debugSpew.c_str() );
				}
			}
			else
			if ( g_nStep > 0 )
			{
				g_emulator->ProcessInstructions( g_nStep, &debugSpew );
				SetWindowTextA( g_debuggerSpewHWND, debugSpew.c_str() );
				g_nStep = 0;
			}
		}
    }

    CleanupDevice();

    return (int) msg.wParam;
}


//-------------------------------------------------------------------------------------------------

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_BBC_EMULATOR));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_BBC_EMULATOR);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}
//-------------------------------------------------------------------------------------------------

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable
		// Create window
	RECT rc = { 0, 0, 640, 512 };
	AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance,nullptr );

	if (!hWnd)
	{
		return FALSE;
	}
   
	if( FAILED( InitDevice(hWnd) ) )
	{
		CleanupDevice();
		return 0;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	g_debuggerHWND = CreateDialog(hInst, MAKEINTRESOURCE(IDD_DEBUGGER), hWnd, Debugger);
	g_bDebuggerActive = true;
	g_debuggerSpewHWND = GetDlgItem( g_debuggerHWND, IDC_DEBUG_SPEW );

	ShowWindow( g_debuggerHWND, nCmdShow&&g_bDebuggerActive );
	CheckMenuItem( GetMenu(hWnd), IDM_SHOW_DEBUGGER, g_bDebuggerActive ? MF_CHECKED : MF_UNCHECKED );

	g_emulator = new BBC_Emulator();

	return TRUE;
}



//-------------------------------------------------------------------------------------------------

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
			case IDM_SHOW_DEBUGGER:
				g_bDebuggerActive = !g_bDebuggerActive;
				ShowWindow( g_debuggerHWND, g_bDebuggerActive );
				CheckMenuItem( GetMenu(hWnd), IDM_SHOW_DEBUGGER, g_bDebuggerActive ? MF_CHECKED : MF_UNCHECKED );
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
        //    PAINTSTRUCT ps;
	    //    HDC hdc = BeginPaint(hWnd, &ps);
                        Render();

         //   EndPaint(hWnd, &ps);
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

//-------------------------------------------------------------------------------------------------
// Message handler for debugger box.
INT_PTR CALLBACK Debugger(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
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
		switch( HIWORD( wParam ) )
		{
			case BN_CLICKED:
				if ( LOWORD( wParam ) == IDC_STEP )
				{
					g_nStep = 1;
				}
				if ( LOWORD( wParam ) == IDC_STEP_1000 )
				{
					g_nStep = 1000;
				}
				else if ( LOWORD( wParam ) == IDC_PLAY )
				{
					g_bRun = true;
				}
				else if ( LOWORD( wParam ) == IDC_PAUSE )
				{
					g_bRun = false;
				}
				break;
		}
        break;
    }
    return (INT_PTR)FALSE;
}
//-------------------------------------------------------------------------------------------------
// Message handler for about box.
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
//-------------------------------------------------------------------------------------------------
