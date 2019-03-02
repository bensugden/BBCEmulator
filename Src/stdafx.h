// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once
 #pragma warning(disable: 4577)
 #pragma warning(disable: 4530)
#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <conio.h>
#include <functional>

using namespace std;

#include "Types.h"

#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include "resource.h"
#include "DirectX.h"

#include "File.h"
#include "myAssert.h"
#include "MiscUtils.h"

using namespace Utils;
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::placeholders::_4;

#include "6502_CPU.h"
#include "6502_Memory.h"
#include "6502_OpcodeTable.h"
#include "SHEILA.h"
#include "VideoULA.h"
#include "SAA5050.h"
#include "6522_VIA.h"
#include "6522_VIA_System.h"
#include "6522_VIA_Ports.h"
#include "6845_CRTC.h"
#include "IBM3740_FloppyDisk.h"
#include "8271_FDC.h"
#include "SystemClock.h"
#include "LS161_PagedRomController.h"

#include "BBC_Keyboard.h"
#include "BBC_Emulator.h"

extern CPU cpu;
extern MemoryState mem;

//-------------------------------------------------------------------------------------------------
#define debugOutput( ... )
/*
inline void debugOutput( const char* strMsg, ... )
{
	char strBuffer[512];

	va_list		argptr;
	va_start( argptr, strMsg );
    _vsnprintf_s(strBuffer, 512, strMsg, argptr);
	va_end  ( argptr );

	OutputDebugStringA( strBuffer );
	OutputDebugStringA( "\n" );
}
*/
//-------------------------------------------------------------------------------------------------
