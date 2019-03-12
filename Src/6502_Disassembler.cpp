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

int Disassembler::GetDestAddress( int pc, const CommandInfo& command, u8 lo, u8 hi )
{
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
		return int( lo + ( u16( hi ) <<8 ) );
	}
	return -1;
}

//-------------------------------------------------------------------------------------------------

void Disassembler::CrawlCodeFrom( int pc )
{
	while ( true )
	{
		u8 bytes[ 3 ];
		bytes[ 0 ] = mem.Read_Internal( pc );

		//
		// Already processed?
		//
		if ( m_memory[ pc ].type != MemReference::MEM_UNKNOWN )
		{
			//
			// Memory not changed
			//
			if ( m_memory[ pc ].data == bytes[ 0 ] )
			{
				return;
			}
		}
		m_memory[ pc ].data = bytes[ 0 ];
		m_memory[ pc ].type = MemReference::MEM_INSTRUCTION;

		//
		// Decode and read extra bytes for instruction
		//
		const CommandInfo& command = cpu.GetOpcodeTable().GetCommandForOpcode( bytes[ 0 ] );
		EAddressingMode mode = command.m_addressingMode;

		int nSize = 1;
		if ( mode == mode_imp || mode == mode_invalid )
			nSize = 0;
		if ( mode >= mode_abs && mode < mode_rel )
			nSize = 2;

		int nCount = 1;
		while ( nCount <= nSize )
		{
			bytes[ nCount ] = mem.Read_Internal( pc + nCount );

			m_memory[ pc  + nCount ].data = bytes[ nCount ];
			m_memory[ pc  + nCount ].type =  MemReference::MEM_EA;

			nCount++;
		}

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
				int address = GetDestAddress( pc, command, bytes[1], bytes[2] );
				if ( address != -1 )
				{
					CrawlCodeFrom( address );
				}
				pc += nCount;
				break;
			}
			case JMP: 
			{
				// unconditional jump - modify pc
				int address = GetDestAddress( pc, command, bytes[1], bytes[2] );
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
				pc += nCount;
				break;
			}
		}
	}
}

//-------------------------------------------------------------------------------------------------