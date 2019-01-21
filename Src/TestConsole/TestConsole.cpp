// TestConsole.cpp : Defines the entry point for the console application.
//

#include "..\stdafx.h"
#include <conio.h>
#include <stdio.h>

void BuildOpcodeTables();


int main()
{
	BuildOpcodeTables();

	printf("Press any key to continue\n");
	_getch();

    return 0;
}

