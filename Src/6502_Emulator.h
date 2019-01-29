#pragma once

//-------------------------------------------------------------------------------------------------

#include "6502_OpcodeTable.h"

//-------------------------------------------------------------------------------------------------

class CPUEmulator
{
public:
	CPUEmulator( );
	~CPUEmulator( );
	void		ProcessSingleInstruction();
	int			DisassemblePC( int pc_in, string& dissassemble, const CommandInfo** ppOutCommand );

private:
	OpcodeTable m_opcodeTable;
};

//-------------------------------------------------------------------------------------------------

extern CPUState			cpu;
extern MemoryState		mem;

//-------------------------------------------------------------------------------------------------
//
// Some Functions (need wrapping in a class or something)
//
//-------------------------------------------------------------------------------------------------

void RegisterInstructionHandlers();

//-------------------------------------------------------------------------------------------------
