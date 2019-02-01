//-------------------------------------------------------------------------------------------------
//
// RAM For 6502 emulator
//
//-------------------------------------------------------------------------------------------------

#include "stdafx.h"

//-------------------------------------------------------------------------------------------------
extern CPU cpu;

void MemoryState::DumpMemoryToString( u16 address, int xcolumns, int yrows, string& output )
{
	u16 current_address = address;
	output.clear();

	for ( int c = 0 ; c < xcolumns + 8; c++ )
	{
		output += ' ';
	}
	for ( u8 x = 0; x < xcolumns; x++ )
	{
		output += Utils::toHex( x, false ) ;
		output += "  ";
	}
	output += "\n";

	for ( int y = 0 ; y < yrows; y++ )
	{
		output += Utils::toHex( current_address ) + " ";

		for ( int x = 0; x < xcolumns; x++ )
		{
			u8 readByte = Read( current_address+x );
			if ( readByte >= 32 && readByte < 0x7F )
				output += char( readByte );
			else
				output += '.';
		}
		output += ' ';

		for ( int x = 0; x < xcolumns; x++ )
		{
			output += Utils::toHex( Read( current_address+x ) ) + " ";
		}
		current_address += xcolumns;

		output += "\n";
	}
}

//-------------------------------------------------------------------------------------------------

