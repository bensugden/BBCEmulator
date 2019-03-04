#include "stdafx.h"
#include "stdio.h"
#include <time.h>

//-------------------------------------------------------------------------------------------------

u16 s_screenResolutions[ ][ 2 ] =
{ 
	{ 640, 256 },  // Mode 0
	{ 320, 256 },  // Mode 1 
	{ 160, 256 },  // Mode 2
	{ 640, 256 },  // Mode 3
	{ 160, 256 },  // Mode 4
	{ 320, 256 },  // Mode 5
	{ 320, 256 },  // Mode 6
	{ 160, 256 },  // Mode 7
};

//-------------------------------------------------------------------------------------------------

MemoryState mem( 32768, 65536 );
CPU			cpu;

//-------------------------------------------------------------------------------------------------

BBC_Emulator::BBC_Emulator()
	: m_videoULA( m_teletext, m_crtc )
	, m_teletext( m_crtc )
	, m_systemVIA( m_keyboard, m_videoULA, m_ti76489 )
	, m_keyboard( m_systemVIA )
{
	m_floppies[ 0 ] = nullptr;
	m_floppies[ 1 ] = nullptr;

	Reset();
	cpu.SetClock( this );
}

//-------------------------------------------------------------------------------------------------

BBC_Emulator::~BBC_Emulator( )
{
	delete m_floppies[ 0 ];
	delete m_floppies[ 1 ];
}

//-------------------------------------------------------------------------------------------------

void BBC_Emulator::Tick()
{
	m_nClockCounter++;
}

//-------------------------------------------------------------------------------------------------

void BBC_Emulator::PollChips()
{
	int nClocksElapsed = (int)(m_nClockCounter - m_nLastClockCounter);
	
	m_systemVIA.Tick( nClocksElapsed );
	m_fdc.Tick( nClocksElapsed );
	m_ti76489.Tick( nClocksElapsed );

	m_nLastClockCounter = m_nClockCounter;
}

//-------------------------------------------------------------------------------------------------

void BBC_Emulator::Reset()
{
	m_nClockCounter = 0;
	m_nLastClockCounter = 0;

	mem.LoadROM( "roms\\Os12.rom", 0xC000 );
	mem.LoadROM( "roms\\Basic2.rom", 0x8000, 15 );
	mem.LoadROM( "roms\\DFS-0.9.rom", 0x8000, 14 );
	mem.Clear( 0xfc00, 0xff00 );

	cpu.Reset();
	m_keyboard.ClearKeys();

	//mem.LoadROM("test\\6502_functional_test.bin", 0x0000);
	//cpu.reg.PC = 0x400;

	m_history.Clear();
}

//-------------------------------------------------------------------------------------------------

bool BBC_Emulator::RunFrame( std::string* pDisassemblyString, bool bDebug )
{
	if ( m_bPaused )
	{
		return false;
	}
	if ( m_bStarted )
	{
		//
		// Return if 1/50th of a second hasn't elapsed since the beginning of last frame
		//
		double elapsed = 0;
		time_t timer;
		time(&timer);
		if ( difftime( timer, m_lastTime ) < 1.0f )
		{
			return false;
		}
	}
	time( &m_lastTime );


	int nTotalCyclesPerFrame = cpu.GetCycleCount() + 2000000 / 50;
	bool bBreakpoint = false;
	while ( cpu.GetCycleCount() < nTotalCyclesPerFrame )
	{
		if ( ProcessInstructions( 1, nullptr, true ) )
		{
			bBreakpoint = true;
			break;
		}
	}
	m_bStarted = true;
	if ( pDisassemblyString && bDebug )
	{
		m_history.GetHistory(*pDisassemblyString);
		OutputDebugStringA( pDisassemblyString->c_str() );
	}
	return bBreakpoint;
}

//-------------------------------------------------------------------------------------------------

void BBC_Emulator::SetBreakpoint( u16 address )
{
	cpu.SetBreakpoint( address );
}

//-------------------------------------------------------------------------------------------------

void BBC_Emulator::DebugDecodeNextInstruction()
{
	string dissassemble;
	u8 bytes[ 3 ];
	int nNumBytes = cpu.GetBytesAtPC( cpu.reg.PC, bytes );
	m_history.RecordCPUState( cpu.reg, bytes );
}

//-------------------------------------------------------------------------------------------------
#define TEST_CODE
#ifdef TEST_CODE
static std::string testCode="*exec !boot";
#endif
//-------------------------------------------------------------------------------------------------
bool g_bExternalDebuggerBreakpoint = false;

bool BBC_Emulator::ProcessInstructions( int nCount, std::string* pDisassemblyString, bool bDebug, bool bForceDebugPC, bool bAlwaysSpewToOutputWindow )
{
#ifdef TEST_CODE
	static int iTotalCount = -8;
	iTotalCount++;
	static u8 oldDown = 0;

	if ( ( iTotalCount >=0 ) && ( iTotalCount & 3 ) == 0 )
	{
		int index = iTotalCount >> 2;
		if ( index < (int)testCode.length() )
		{
			SetKeyDown( testCode[ index ], false );
			oldDown = testCode[ index ];
		}
		if ( index == testCode.length() )
		{
			SetKeyDown( VK_RETURN, true );
		}
	}
#endif
	bool bBreakpoint = false;

	if ( bDebug && ( m_history.IsEmpty()|| bForceDebugPC ) )
	{
		DebugDecodeNextInstruction();
	}
	for ( int i = 0 ; ( i < nCount ) && ( !bBreakpoint ) ; i++ )
	{
		if ( cpu.ProcessSingleInstruction() )
		{
			bBreakpoint = true;
		}
		if ( bDebug || bBreakpoint  )
		{
			DebugDecodeNextInstruction();
		}
		if ( bAlwaysSpewToOutputWindow )
		{
			static std::string disassembly;
			m_history.GetLastInstruction(disassembly);
			OutputDebugStringA( disassembly.c_str() );
		}
		if ( g_bExternalDebuggerBreakpoint )
		{
			bBreakpoint = true;
			g_bExternalDebuggerBreakpoint = false;
		}
#ifdef TEST_CODE
		if ( i > 20000 )
		if ( oldDown != 0 )
		{
			SetKeyUp( oldDown, false );
			oldDown = 0;
		}
#endif
	}
	if ( ((!bAlwaysSpewToOutputWindow)||bBreakpoint) && ( (bBreakpoint||bDebug) && pDisassemblyString ) )
	{
		m_history.GetHistory(*pDisassemblyString);
		OutputDebugStringA( pDisassemblyString->c_str() );
	}
	if ( m_keyboard.IsResetDown() )
		Reset();
	return bBreakpoint;
}

//-------------------------------------------------------------------------------------------------

void BBC_Emulator::InsertDisk( int drive, const std::string& filename )
{
	FloppyDisk* pFloppy = new FloppyDisk( filename );
	if ( m_floppies[ drive ] != nullptr )
	{
		m_fdc.EjectDisk( drive );
		delete m_floppies[ drive ];
	}
	m_fdc.InsertDisk( drive, pFloppy );
	m_floppies[ drive ] = pFloppy;
}

//-------------------------------------------------------------------------------------------------

void BBC_Emulator::EjectDisk( int drive )
{
	if ( m_floppies[ drive ] != nullptr )
	{
		m_fdc.EjectDisk( drive );
		delete m_floppies[ drive ];
	}
	m_floppies[ drive ] = nullptr;
}

//-------------------------------------------------------------------------------------------------

void BBC_Emulator::RefreshDisplay()
{
	m_videoULA.RefreshDisplay();
}

//-------------------------------------------------------------------------------------------------
