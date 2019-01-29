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

using namespace std;

#include "Types.h"

#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include "resource.h"
#include "DirectX.h"

#include "File.h"
#include "myAssert.h"

#include "6502_CPU.h"
#include "6502_Memory.h"
#include "6502_Emulator.h"
#include "6502_OpcodeTable.h"
#include "SHEILA.h"
#include "VideoULA.h"
#include "SAA5050.h"

#include "BBC_Emulator.h"

