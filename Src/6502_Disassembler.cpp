//-------------------------------------------------------------------------------------------------
//
// 6502 Disassembler / Code Crawler
//
//-------------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "6502_Disassembler.h"

//-------------------------------------------------------------------------------------------------

Disassembler::Disassembler( int nMemorySize )
{
	m_memory.resize( nMemorySize );
}

//-------------------------------------------------------------------------------------------------

int Disassembler::GetDestAddress( int pc, const CommandInfo& command )
{
	u8 lo = mem.Read_Internal( pc + 1 );
	if ( command.m_addressingMode == mode_rel )
	{
		return (int)(pc + 2 + ((s8)lo)); 
	}
	else if ( command.m_addressingMode == mode_imm )
	{
		return lo;
	}
	else if ( command.m_addressingMode == mode_abs )
	{
		u8 hi = mem.Read_Internal( pc + 2 );
		return int( lo + ( u16( hi ) <<8 ) );
	}
	return -1;
}

//-------------------------------------------------------------------------------------------------

const CommandInfo& Disassembler::DecodeAt( int pc )
{
	//
	// Decode and read extra bytes for instruction
	//
	return cpu.GetOpcodeTable().GetCommandForOpcode( mem.Read_Internal( pc ) );
}

//-------------------------------------------------------------------------------------------------

void Disassembler::MarkAsDecoded( int pc, const CommandInfo& command )
{
	for ( int i = 0; i< command.m_nSize; i++ )
	{
		m_memory[ pc  + i ].data = mem.Read_Internal( pc + i );
		m_memory[ pc  + i ].type =  i == 0 ? MemReference::MEM_INSTRUCTION : MemReference::MEM_EA;
	}
}

//-------------------------------------------------------------------------------------------------

bool Disassembler::IsMemoryAlreadyDisassembled( int pc, const CommandInfo& command )
{
	for ( int i = 0; i < command.m_nSize; i++ )
	{
		//
		// Already processed?
		//
		if ( m_memory[ pc ].type != MemReference::MEM_UNKNOWN )
		{
			//
			// Memory not changed
			//
			if ( m_memory[ pc ].data == mem.Read_Internal( pc ) )
			{
				return true;
			}
		}
	}
	return false;
}

//-------------------------------------------------------------------------------------------------

void Disassembler::DisassembleFrom( int entry_point )
{
	//
	// First crawl from entry point PC
	//
	CrawlCodeFrom( entry_point );

	//
	// Then take a stab at disassembling raw memory
	//
	for ( u32 pc = 0; pc < mem.GetAllocatedMemorySize(); )
	{
		const CommandInfo& command = DecodeAt( pc );
		if ( IsMemoryAlreadyDisassembled( pc, command ) || command.IsIllegalOpcode() )
		{
			pc++;
		}
		else
		{
			MarkAsDecoded( pc, command );
			pc += command.m_nSize;
		}
	}
}
//-------------------------------------------------------------------------------------------------

void Disassembler::CrawlCodeFrom( int pc )
{
	while ( true )
	{
		//
		// Decode instruction at PC
		//
		const CommandInfo& command = DecodeAt( pc );

		if ( IsMemoryAlreadyDisassembled( pc, command ) )
		{
			return;
		}

		MarkAsDecoded( pc, command );

		//
		// Check for branching, jumping or termination
		//
		switch ( command.m_instruction )
		{
			case BPL:
			case BMI:
			case BVC: 
			case BVS: 
			case BCC: 
			case BCS: 
			case BNE: 
			case BEQ:
			case JSR:
			{
				// conditional jump - take both paths
				int address = GetDestAddress( pc, command );
				if ( address != -1 )
				{
					CrawlCodeFrom( address );
				}
				pc += command.m_nSize;
				break;
			}
			case JMP: 
			{
				// unconditional jump - modify pc
				int address = GetDestAddress( pc, command );
				if ( address == -1 )
				{
					return;
				}
				pc = address;
				break;
			}
			case RTI:
			case RTS:
			case BRK:
			{
				// termination
				return;
			}
			default:
			{
				pc += command.m_nSize;
				break;
			}
		}
	}
}

//-------------------------------------------------------------------------------------------------