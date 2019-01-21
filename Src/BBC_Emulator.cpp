#include "stdafx.h"
#include "stdio.h"

//-------------------------------------------------------------------------------------------------

CPUState		cpu;
MemoryState		mem( cpu, 65536 );

//-------------------------------------------------------------------------------------------------

BBC_Emulator::BBC_Emulator()
{
	BuildOpcodeTables();

	mem.LoadROM( "roms\\Os12.rom", 0xC000 );
	mem.LoadROM( "roms\\Basic2.rom", 0x8000 );

	int pc = 0x8023;
	while ( pc < 0xffff )
	{
		for ( int i = 0 ; i < 20; i++ )
		{
			string dissassemble;
			pc = DisassemblePC( pc, dissassemble );
			printf( dissassemble.c_str() );
			printf( "\n" );
		}
		_getch();
	}

}

//-------------------------------------------------------------------------------------------------

BBC_Emulator::~BBC_Emulator( )
{

}

//-------------------------------------------------------------------------------------------------
