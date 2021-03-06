#include "stdafx.h"
#include "stdio.h"
#include <time.h>

//-------------------------------------------------------------------------------------------------

MemoryState mem( 32768, 65536 );
CPU			cpu;

//-------------------------------------------------------------------------------------------------

BBC_Emulator::BBC_Emulator( bool bBreakOnStartup )
	: m_videoULA( m_teletext, m_crtc, m_systemVIA )
	, m_teletext( m_crtc )
	, m_systemVIA( m_keyboard, m_videoULA, m_ti76489 )
	, m_keyboard( m_systemVIA )
	, m_crtc( m_videoULA )
	, m_debugServer( *this )
	, m_bPaused( bBreakOnStartup )
{
	m_floppies[ 0 ] = nullptr;
	m_floppies[ 1 ] = nullptr;
	InitializeCriticalSection( &m_csForBreak );
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
	m_portsVIA.Tick( nClocksElapsed );
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
	QueryPerformanceCounter( &m_lastTime );
	QueryPerformanceFrequency(&m_timerFreq);
	m_lastTimeInSeconds =  (((double)(m_lastTime.QuadPart)) / ((double)m_timerFreq.QuadPart)); 

	if ( m_bEnableSound )
		m_ti76489.SetVolume( 255 );
	else
		m_ti76489.SetVolume( 0 );

	//
	// Prime disassembler
	//
	m_disassembler.DisassembleFrom( mem.ReadAddress( CPU::c_Reset_Vector ) );
	m_disassembler.DisassembleFrom( mem.ReadAddress( CPU::c_IRQ_Vector ) );
	m_disassembler.DisassembleFrom( mem.ReadAddress( CPU::c_NMI_Vector ) );
	// test
	std::string code;
	m_disassembler.GenerateCode( code );
	FILE* fp = fopen( "disassembly\\_session.6502", "w" );
	fwrite( code.c_str(), code.length() + 1, 1, fp );
	fclose( fp );
}

//-------------------------------------------------------------------------------------------------

void BBC_Emulator::EnterEmulatorCriticalSection()
{
	EnterCriticalSection( &m_csForBreak );
}

//-------------------------------------------------------------------------------------------------

void BBC_Emulator::ExitEmulatorCriticalSection()
{
	LeaveCriticalSection( &m_csForBreak );
}

//-------------------------------------------------------------------------------------------------

bool BBC_Emulator::RunFrame( std::string* pDisassemblyString, bool bDebug )
{
	if ( m_bPaused )
	{
		return false;
	}

	EnterEmulatorCriticalSection();
	LARGE_INTEGER thisTime;
	QueryPerformanceCounter(&thisTime);
	if ( m_bStarted )
	{
		//
		// Return if 1/50th of a second hasn't elapsed since the beginning of last frame
		//
		double elapsed = ((double)(thisTime.QuadPart-m_lastTime.QuadPart)) / ((double)m_timerFreq.QuadPart);
		if ( elapsed < 1.0f / 50.f )
		{
			ExitEmulatorCriticalSection();
			return false;
		}
	}
	m_lastTime = thisTime;


	u64 nTotalCyclesPerFrame = cpu.GetClockCounter() + 40960;
	bool bBreakpoint = false;
	while ( cpu.GetClockCounter() < nTotalCyclesPerFrame )
	{
		if ( ProcessInstructions( 1, nullptr, bDebug ) )
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

	ExitEmulatorCriticalSection();
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
	u8 bytes[ 3 ];
	int nNumBytes = cpu.GetBytesAtPC( cpu.reg.PC, bytes );
	m_history.RecordCPUState( cpu.reg, bytes );
}

//-------------------------------------------------------------------------------------------------
//#define TEST_CODE
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

void BBC_Emulator::RunFrame()
{
	bool bBreak = RunFrame( nullptr, false );
	RefreshDisplay();
}

//-------------------------------------------------------------------------------------------------

void BBC_Emulator::RefreshDisplay()
{
	m_videoULA.RefreshDisplay();
	GFXSystem::Render();
}

//-------------------------------------------------------------------------------------------------
