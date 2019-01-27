#include "stdafx.h"
#include "stdio.h"
#include <time.h>

//-------------------------------------------------------------------------------------------------

CPUState		cpu;
MemoryState		mem( cpu, 65536 );

//-------------------------------------------------------------------------------------------------

BBC_Emulator::BBC_Emulator()
{
	mem.LoadROM( "roms\\Os12.rom", 0xC000 );
	mem.LoadROM( "roms\\Basic2.rom", 0x8000 );
}

//-------------------------------------------------------------------------------------------------

BBC_Emulator::~BBC_Emulator( )
{

}

//-------------------------------------------------------------------------------------------------

bool BBC_Emulator::RunFrame( std::string* pDebugOutput )
{
	if ( bPaused )
	{
		return false;
	}
	if ( bStarted )
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
	bStarted = true;
	
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
