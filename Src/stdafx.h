// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

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
#include "6522_VIA_MOS.h"
#include "6522_VIA_Ports.h"
#include "6845_CRTC.h"

#include "BBC_Emulator.h"

extern CPU cpu;
extern MemoryState mem;