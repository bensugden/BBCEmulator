//--------------------------------------------------------------------------------------

#include "stdafx.h"
#include "Win32.h"
#include "windowsx.h"

//--------------------------------------------------------------------------------------

#define MAX_LOADSTRING 100

HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

HWND g_mainHWND = nullptr;
HWND g_debuggerHWND = nullptr;
HWND g_debuggerDisassemblyHWND = nullptr;
HWND g_debuggerP_HWND  = nullptr;
HWND g_debuggerPC_HWND = nullptr;
HWND g_debuggerS_HWND  = nullptr;
HWND g_debuggerA_HWND  = nullptr;
HWND g_debuggerX_HWND  = nullptr;
HWND g_debuggerY_HWND  = nullptr;
HWND g_debuggerMemory_HWND = nullptr;
HWND g_breakpointReason_HWND = nullptr;

bool g_bDebuggerActive = false;
BBC_Emulator* g_emulator = nullptr;

int g_nStep = 0;
bool g_bRun = true;
bool g_bDisplayOutput = true;
bool g_bBreakOnWriteActive = false;
bool g_bBreakOnReadActive = false;
bool g_bDisassemblyConstantSpew = false;

u16 g_nMemoryAddressToDebug = 0;

std::string g_disassembly;
std::string g_memoryDebug;

std::vector< u16 > g_breakpoints;

//-------------------------------------------------------------------------------------------------

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Debugger(HWND, UINT, WPARAM, LPARAM);

//-------------------------------------------------------------------------------------------------

void UpdateDisassemblyWindow( bool bForceUpdateHistory )
{
	g_emulator->ProcessInstructions( 0, &g_disassembly, g_bDisplayOutput, bForceUpdateHistory );
	SetWindowTextA( g_debuggerDisassemblyHWND, g_disassembly.c_str() );
}
//-------------------------------------------------------------------------------------------------

void UpdateStatusWindows( bool bForceUpdateHistory = false )
{
	UpdateDisassemblyWindow( bForceUpdateHistory );
	SetWindowTextA( g_debuggerS_HWND, Utils::toHex( cpu.reg.S ).c_str() );
	SetWindowTextA( g_debuggerP_HWND, Utils::toHex( cpu.reg.P ).c_str() );
	SetWindowTextA( g_debuggerPC_HWND, Utils::toHex( cpu.reg.PC ).c_str() );
	SetWindowTextA( g_debuggerA_HWND, Utils::toHex( cpu.reg.A ).c_str() );
	SetWindowTextA( g_debuggerX_HWND, Utils::toHex( cpu.reg.X ).c_str() );
	SetWindowTextA( g_debuggerY_HWND, Utils::toHex( cpu.reg.Y ).c_str() );

	mem.DumpMemoryToString( g_nMemoryAddressToDebug, 16, 16, g_memoryDebug );
	SetWindowTextA( g_debuggerMemory_HWND, g_memoryDebug.c_str() );
}

//-------------------------------------------------------------------------------------------------

void UpdateBreakpointReason( bool bHitBreak )
{
	if ( bHitBreak )
	{
		SetWindowTextA( g_breakpointReason_HWND, cpu.GetBreakpointReason().c_str() );
	}
	else
	{
		SetWindowTextA( g_breakpointReason_HWND, "" );
	}
}

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

    // Main message loop:
	bool bLastRun = g_bRun;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg) )
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
		if ( g_bDebuggerActive )
		{
			if ( g_bRun )
			{
				bool bBreak;
				if ( g_bDisplayOutput )
				{
					bBreak = g_emulator->ProcessInstructions( 2000000 / 50, &g_disassembly, false, false, g_bDisassemblyConstantSpew ) ;
				}
				else
				{
					bBreak = g_emulator->RunFrame( &g_disassembly, false );
				}

				UpdateBreakpointReason( bBreak );
				g_bRun = !bBreak;
				SleepEx( 1, false );
				bLastRun = true;
			}
			else
			if ( g_nStep > 0 )
			{
				bool bBreak = g_emulator->ProcessInstructions( g_nStep, &g_disassembly, true, false, g_bDisassemblyConstantSpew );
				UpdateBreakpointReason( bBreak );
				if ( g_bDisplayOutput )
					UpdateStatusWindows();
				g_nStep = 0;
			}
			else
			if ( bLastRun )
			{
				UpdateStatusWindows();
				bLastRun = false;
			}
			if ( g_emulator !=nullptr )
			{
				g_emulator->RefreshDisplay();
			}
			GFXSystem::Render();
		}
    }

    GFXSystem::Shutdown();

    return (int) msg.wParam;
}

//-------------------------------------------------------------------------------------------------
//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//-------------------------------------------------------------------------------------------------

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
//-------------------------------------------------------------------------------------------------

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable
		// Create window
	RECT rc = { 0, 0, 400 * 2, 300 * 2 };
	AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance,nullptr );

	if (!hWnd)
	{
		return FALSE;
	}
   
	if( FAILED( GFXSystem::Init(hWnd) ) )
	{
		GFXSystem::Shutdown();
		return 0;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	g_mainHWND = hWnd;
	g_debuggerHWND = CreateDialog(hInst, MAKEINTRESOURCE(IDD_DEBUGGER), hWnd, Debugger);
	g_bDebuggerActive = true;

	g_debuggerDisassemblyHWND	= GetDlgItem( g_debuggerHWND, IDC_DEBUG_OUTPUT );
	g_debuggerP_HWND			= GetDlgItem( g_debuggerHWND, IDC_SET_P );
	g_debuggerPC_HWND			= GetDlgItem( g_debuggerHWND, IDC_SET_PC );
	g_debuggerS_HWND			= GetDlgItem( g_debuggerHWND, IDC_SET_S );
	g_debuggerA_HWND			= GetDlgItem( g_debuggerHWND, IDC_SET_A );
	g_debuggerX_HWND			= GetDlgItem( g_debuggerHWND, IDC_SET_X );
	g_debuggerY_HWND			= GetDlgItem( g_debuggerHWND, IDC_SET_Y );
	g_debuggerMemory_HWND		= GetDlgItem( g_debuggerHWND, IDC_MEMORY_DEBUG );
	g_breakpointReason_HWND		= GetDlgItem( g_debuggerHWND, IDC_BREAKPOINT_REASON );

	ShowWindow( g_debuggerHWND, nCmdShow&&g_bDebuggerActive );
	CheckMenuItem( GetMenu(hWnd), IDM_SHOW_DEBUGGER, g_bDebuggerActive ? MF_CHECKED : MF_UNCHECKED );

	g_emulator = new BBC_Emulator();
	g_emulator->InsertDisk( 0, "disks\\test.ssd" );

	UpdateStatusWindows( true );

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
//-------------------------------------------------------------------------------------------------

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
				case IDM_DRIVE0_INSERT:
					g_emulator->InsertDisk( 0, "disks\\test.ssd" );
					//g_emulator->InsertDisk( 0, "disks\\chuckieegg.ssd" );
					break;
				case IDM_DRIVE0_EJECT:
					g_emulator->EjectDisk( 0 );
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
				
			}
			break;
 		case WM_KEYDOWN:
 		{
 			u8 key = (u8)wParam;
			int c;
			byte ks[256];
			GetKeyboardState(ks);
			char xx[3];
			xx[0] = 0;
			c = ToAscii(key, MapVirtualKey(key,0), ks, (LPWORD) &xx[0], 0);
 			if ( c == 1 )
				g_emulator->SetKeyDown( xx[0], false );
			else
 				g_emulator->SetKeyDown( key, true );

 			break;
 		}
 		case WM_KEYUP:
 		{
 			u8 key = (u8)wParam;
			int c;
			byte ks[256];
			GetKeyboardState(ks);
			char xx[3];
			xx[0] = 0;
			c = ToAscii(key, MapVirtualKey(key,0), ks, (LPWORD) &xx[0], 0);
 			if ( c == 1 )
				g_emulator->SetKeyUp( xx[0], false );
			else
				g_emulator->SetKeyUp( key, true );
 			break;
 		}


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
int HexOrDecToInt( char* hex, bool assume_hex = true )
{
	char* lastHex = hex;
	if ( !hex )
		return -1;

	bool isHex = assume_hex;
	while ( hex[ 0 ] == '0' || hex[ 0 ] == 'x' || hex[ 0 ] == 'X' || hex[ 0 ] == '$' )
	{
		if ( hex[ 0 ] != '0' )
			isHex = true;

		hex ++;
		if ( hex[ 0 ]==0  )
		{
			if ( hex > lastHex )
				return 0;
			return -1;
		}
	}

	int total = 0;
	do
	{
		int nextInt = ValidHexChar( *hex++ );
		if ( nextInt == -1 )
			return -1;
		total *= isHex ? 16 : 10;
		total += nextInt;
	}
	while ( hex[ 0 ]!=0 );
	return total;
}

//-------------------------------------------------------------------------------------------------

void UpdateBreakOnWriteAddress( HWND hDlg )
{
	char addressStr[ 256 ];
	GetDlgItemTextA( hDlg, IDC_BREAK_ON_WRITE_ADDRESS, addressStr, 256 );
	int address = HexOrDecToInt( addressStr );

	GetDlgItemTextA( hDlg, IDC_BREAK_ON_WRITE_VALUE, addressStr, 256 );
	int value = HexOrDecToInt( addressStr );

	if ( address == -1  )
		return;
	if ( g_bBreakOnWriteActive )
		mem.SetWriteBreakpoint( address, value );
	else
		mem.ClearWriteBreakpoint( );


}

//-------------------------------------------------------------------------------------------------

void UpdateBreakOnReadAddress( HWND hDlg )
{
	char addressStr[ 256 ];
	GetDlgItemTextA( hDlg, IDC_BREAK_ON_READ_ADDRESS, addressStr, 256 );
	int address = HexOrDecToInt( addressStr );
	if ( address == -1  )
		return;
	if ( g_bBreakOnReadActive )
		mem.SetReadBreakpoint( address );
	else
		mem.ClearReadBreakpoint( );
}

//-------------------------------------------------------------------------------------------------
template <class T>
void UpdateRegisterFromDialog( HWND hDlg, int nIDC, T& reg )
{
	char addressStr[ 256 ];
	GetDlgItemTextA( hDlg, nIDC, addressStr, 256 );
	int address = HexOrDecToInt( addressStr );
	if (( address != -1 )&&( reg != address ))
	{
		reg = address;
		UpdateStatusWindows( true );
	}
}
//-------------------------------------------------------------------------------------------------
void SetMemoryAddressToWatch( HWND hDlg )
{
	char addressStr[ 256 ];
	GetDlgItemTextA( hDlg, IDC_MEMORY_ADDRESS, addressStr, 256 );
	int address = HexOrDecToInt( addressStr );
	if ( address != -1 )
	{
		g_nMemoryAddressToDebug = address;
		UpdateStatusWindows( false );
	}
}

//-------------------------------------------------------------------------------------------------
// Message handler for debugger box.
INT_PTR CALLBACK Debugger(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;
	case WM_MOUSEWHEEL:
	{	
		POINT p;
		GetCursorPos(&p);
		RECT rect;
		GetWindowRect(g_debuggerMemory_HWND, &rect );
		if ( rect.left < p.x && rect.right > p.x && rect.top < p.y && rect.bottom > p.y )
		{
			short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		}

        return (INT_PTR)TRUE;
	}
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
						//g_bRun = false;
						extern bool g_bExternalDebuggerBreakpoint ;
						g_bExternalDebuggerBreakpoint = true;
						break;
					}
					case IDC_OUTPUT_DISASSEMBLY:
					{
						g_bDisassemblyConstantSpew = !g_bDisassemblyConstantSpew;
						break;
					}
					case IDC_BREAK_ON_READ:
					{
						// break on mem read
						g_bBreakOnReadActive = !g_bBreakOnReadActive;
						UpdateBreakOnReadAddress( hDlg );

						break;
					}
					case IDC_BREAK_ON_WRITE:
					{
						// break on mem write
						g_bBreakOnWriteActive = !g_bBreakOnWriteActive;

						UpdateBreakOnWriteAddress( hDlg );
						break;
					}

					case IDC_SET_BREAKPOINT:
					{
						// breakpoint
						char addressStr[256];
						GetDlgItemTextA( hDlg, IDC_BREAKPOINT_ADDRESS, addressStr, 256 );
						int address = HexOrDecToInt( addressStr );
						if ( ( address != -1 ) && find( g_breakpoints.begin(), g_breakpoints.end(), address ) == g_breakpoints.end() )
						{
							string breakpoints ;
							g_breakpoints.push_back( address );
							cpu.ClearBreakpoints();
							for ( size_t i = 0; i < g_breakpoints.size(); i++ )
							{
								cpu.SetBreakpoint( g_breakpoints[i] );

								breakpoints.append( Utils::toHex( u16( g_breakpoints[i] ), true ) );
								breakpoints.append("\n");
							}
							SetDlgItemTextA( hDlg, IDC_BREAKPOINTS, breakpoints.c_str() );
						}
						break;
					}
					case IDC_CLEAR_BREAKPOINTS:
					{
						g_breakpoints.clear();
						SetDlgItemTextA( hDlg, IDC_BREAKPOINTS, "" );
						cpu.ClearBreakpoints();
						break;
					}
					case IDC_SHOW_MEMORY:
					{
						SetMemoryAddressToWatch( hDlg );

						break;
					}
					case IDC_RESET:
					{
						g_emulator->Reset();
						UpdateStatusWindows( true );
						break;
					}
					default:
						break;
				}
				break;
			}
			case EN_UPDATE:
			{
				switch( LOWORD( wParam ) )
				{
					case IDC_BREAK_ON_WRITE_ADDRESS:
					{
						// break on mem write
						UpdateBreakOnWriteAddress( hDlg );
						break;
					}
					case IDC_BREAK_ON_WRITE_VALUE:
					{
						// break on mem write
						UpdateBreakOnWriteAddress( hDlg );
						break;
					}
					case IDC_BREAK_ON_READ_ADDRESS:
					{
						// break on mem read
						UpdateBreakOnReadAddress( hDlg );

						break;
					}
				}
				break;
			}
			case EN_KILLFOCUS:
			{
				switch( LOWORD( wParam ) )
				{
					case IDC_MEMORY_ADDRESS:
					{
						SetMemoryAddressToWatch( hDlg );
						break;
					}
					case IDC_SET_PC:
					{
						UpdateRegisterFromDialog( hDlg, IDC_SET_PC, cpu.reg.PC );
						break;
					}
					case IDC_SET_A:
					{
						UpdateRegisterFromDialog( hDlg, IDC_SET_A, cpu.reg.A );
						break;
					}
					case IDC_SET_X:
					{
						UpdateRegisterFromDialog( hDlg, IDC_SET_X, cpu.reg.X );
						break;
					}
					case IDC_SET_Y:
					{
						UpdateRegisterFromDialog( hDlg, IDC_SET_Y, cpu.reg.Y );
						break;
					}
					case IDC_SET_S:
					{
						UpdateRegisterFromDialog( hDlg, IDC_SET_S, cpu.reg.S );
						break;
					}
					case IDC_SET_P:
					{
						UpdateRegisterFromDialog( hDlg, IDC_SET_P, cpu.reg.P );
						break;
					}
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
