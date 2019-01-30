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
bool g_bDisplayOutput = false;

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
				if ( g_bDisplayOutput )
				{
					g_emulator->ProcessInstructions( 2000000 / 32, &debugSpew, g_bDisplayOutput );
					SetWindowTextA( g_debuggerSpewHWND, debugSpew.c_str() );
					SleepEx( 1, false );
				}
				else
				{
					g_emulator->RunFrame( &debugSpew, false );
					SleepEx( 1, false );
				}
			}
			else
			if ( g_nStep > 0 )
			{
				g_emulator->ProcessInstructions( g_nStep, &debugSpew, g_bDisplayOutput );
				if ( g_bDisplayOutput )
					SetWindowTextA( g_debuggerSpewHWND,  debugSpew.c_str()  );
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
	g_debuggerSpewHWND = GetDlgItem( g_debuggerHWND, IDC_DEBUG_OUTPUT );

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
int ValidHexChar( char h )
{
	if ( h >= '0' && h <='9' )
		return h - '0';
	if ( h >= 'a' && h <='f' )
		return h-'a'+10;
	if ( h >= 'A' && h <='F' )
		return h-'A'+10;
	return -1;
}
//-------------------------------------------------------------------------------------------------
int HexToInt( char* hex )
{
	if ( !hex )
		return -1;

	while ( hex[ 0 ] == '0' || hex[ 0 ] == 'x' || hex[ 0 ] == 'X' || hex[ 0 ] == '$' )
	{
		hex ++;
		if ( hex[ 0 ]==0  )
			return -1;
	}

	int total = 0;
	do
	{
		int nextInt = ValidHexChar( *hex++ );
		if ( nextInt == -1 )
			return -1;
		total *= 16;
		total += nextInt;
	}
	while ( hex[ 0 ]!=0 );
	return total;
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
			{
				switch( LOWORD( wParam ) )
				{
					case IDC_STEP :
					{
						g_nStep = 1;
						break;
					}
					case IDC_STEP_1000:
					{
						g_nStep = 1000;
						break;
					}
					case IDC_STEP_FRAME:
					{
						g_nStep = 2000000/50;
						break;
					}
					case IDC_PLAY:
					{
						g_bRun = true;
						break;
					}
					case IDC_PAUSE:
					{
						g_bRun = false;
						break;
					}
					case IDC_DISPLAY_OUTPUT:
					{
						g_bDisplayOutput = !g_bDisplayOutput;
						break;
					}
					case IDC_BREAK_ON_READ:
					{
						// break on mem write
						char addressStr[256];
						GetDlgItemTextA( hDlg, IDC_BREAK_ON_READ_ADDRESS, addressStr, 256 );
						int address = HexToInt( addressStr );
						break;
					}
					case IDC_BREAK_ON_WRITE:
					{
						// break on mem write
						char addressStr[256];
						GetDlgItemTextA( hDlg, IDC_BREAK_ON_WRITE_ADDRESS, addressStr, 256 );
						int address = HexToInt( addressStr );
						break;
					}
					case IDC_SET_BREAKPOINT:
					{
						// breakpoint
						char addressStr[256];
						GetDlgItemTextA( hDlg, IDC_BREAKPOINT_ADDRESS, addressStr, 256 );
						int address = HexToInt( addressStr );
						//IDC_BREAKPOINTS
						break;
					}
					default:
						break;
				}
				break;
			}
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
