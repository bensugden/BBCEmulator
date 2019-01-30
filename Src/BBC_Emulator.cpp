#include "stdafx.h"
#include "stdio.h"
#include <time.h>

//-------------------------------------------------------------------------------------------------

CPUState		cpu;
MemoryState		mem( 32768, 65536 );

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

BBC_Emulator::BBC_Emulator()
{
	mem.LoadROM( "roms\\Os12.rom", 0xC000 );
	mem.LoadROM( "roms\\Basic2.rom", 0x8000 );

	m_cpuEmulator.Reset();

	mem.LoadROM("test\\6502_functional_test_configured.bin", 0x0000);
	cpu.PC = 0x400;
}

//-------------------------------------------------------------------------------------------------

BBC_Emulator::~BBC_Emulator( )
{

}

//-------------------------------------------------------------------------------------------------

bool BBC_Emulator::RunFrame( std::string* pDebugOutput )
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

	while ( cpu.GetCycleCount() < nTotalCyclesPerFrame )
	{
		ProcessInstructions( 1, pDebugOutput );
	}
	m_bStarted = true;
	
	return true;
}

//-------------------------------------------------------------------------------------------------

void BBC_Emulator::ProcessInstructions( int nCount, std::string* pDebugOutput )
{
	for ( int i = 0 ; i < nCount; i++ )
	{
		if ( pDebugOutput )
		{
			string dissassemble;
			u16 lastpc = cpu.PC;
			u16 nextpc = m_cpuEmulator.DisassemblePC( cpu.PC, dissassemble, nullptr );

			(*pDebugOutput) += dissassemble.c_str();
			(*pDebugOutput) += "\n";

			OutputDebugStringA( dissassemble.c_str() );
			OutputDebugStringA( "\n" );

		}
		m_cpuEmulator.ProcessSingleInstruction();
	}
}
//-------------------------------------------------------------------------------------------------
