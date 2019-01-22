// TestConsole.cpp : Defines the entry point for the console application.
//

#include "..\stdafx.h"


int main()
{
	BBC_Emulator bbcMicroModelB;

	//bbcMicroModelB.Run();

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

	printf("Press any key to continue\n");
	_getch();

    return 0;
}

