#include "stdafx.h"
#include "stdio.h"

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

void BBC_Emulator::Run( )
{
	CPUEmulator emu;

	while ( true )
	{
		string dissassemble;
		u16 lastpc = cpu.PC;
		u16 nextpc = emu.DisassemblePC( cpu.PC, dissassemble, nullptr );
		printf( dissassemble.c_str() );
		printf( "\n" );
		_getch();
		emu.ProcessSingleInstruction();
	}
}

//-------------------------------------------------------------------------------------------------
