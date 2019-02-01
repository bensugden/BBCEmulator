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
{
	Reset();
}

//-------------------------------------------------------------------------------------------------

BBC_Emulator::~BBC_Emulator( )
{

}

//-------------------------------------------------------------------------------------------------

void BBC_Emulator::Reset()
{
	mem.LoadROM( "roms\\Os12.rom", 0xC000 );
	mem.LoadROM( "roms\\Basic2.rom", 0x8000 );

	cpu.Reset();

	mem.LoadROM("test\\6502_functional_test.bin", 0x0000);
	cpu.reg.PC = 0x400;

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

bool BBC_Emulator::ProcessInstructions( int nCount, std::string* pDisassemblyString, bool bDebug, bool bForceDebugPC )
{
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
		if ( bDebug  )
		{
			DebugDecodeNextInstruction();
		}
	}
	if ( bDebug && pDisassemblyString )
	{
		m_history.GetHistory(*pDisassemblyString);
		OutputDebugStringA( pDisassemblyString->c_str() );
	}
	return bBreakpoint;
}
//-------------------------------------------------------------------------------------------------
