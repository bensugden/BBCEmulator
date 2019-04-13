//-------------------------------------------------------------------------------------------------
//
// 6502 Disassembler / Code Crawler
//
//-------------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "6502_Disassembler.h"


//-------------------------------------------------------------------------------------------------

DisassemblerContext::DisassemblerContext( Disassembler& disassembler, BBC_Emulator& emulator )
	: m_disassembler( disassembler )
	, m_emulator( emulator )
{
	//
	// Pause the emulator so we can disassemble, grab PC, etc
	//
	m_emulator.EnterEmulatorCriticalSection();
}

//-------------------------------------------------------------------------------------------------

DisassemblerContext::~DisassemblerContext()
{
	//
	// Release emulator so we can disassemble, grab PC, etc
	//
	m_emulator.EnterEmulatorCriticalSection();
}

//-------------------------------------------------------------------------------------------------

Disassembler::Disassembler( )
{
	m_memory.resize( mem.GetAllocatedMemorySize() );
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

void Disassembler::MarkAsDecoded( int pc, const CommandInfo& command, bool bConfirmed )
{
	u32 uConfimed = bConfirmed ? MemReference::MEM_CONFIRMED : 0;
	for ( int i = 0; i< command.m_nSize; i++ )
	{
		m_memory[ pc  + i ].data = mem.Read_Internal( pc + i );
		m_memory[ pc  + i ].flags =  i == 0 ? MemReference::MEM_INSTRUCTION : MemReference::MEM_EA;
		m_memory[ pc  + i ].flags |= uConfimed;
	}
}

//-------------------------------------------------------------------------------------------------

bool Disassembler::IsMemoryAlreadyDisassembled( int pc, const CommandInfo& command )
{
	u32 nFlags =  MemReference::MEM_CONFIRMED | MemReference::MEM_INSTRUCTION ;
	for ( int i = 0; i < command.m_nSize; i++ )
	{
		//
		// Already processed?
		//
		if (( m_memory[ pc ].flags & nFlags ) != nFlags )
		{
			return false;
		}
		//
		// Memory changed
		//
		if ( m_memory[ pc ].data != mem.Read_Internal( pc ) )
		{
			return false;
		}
		nFlags = MemReference::MEM_CONFIRMED | MemReference::MEM_EA ;
		pc++;
	}
	return true;
}

//-------------------------------------------------------------------------------------------------

void Disassembler::GenerateCode( std::string& code )
{
	//
	// Take a stab at disassembling any remaining raw memory
	//
	for ( u32 pc = 0; pc < mem.GetAllocatedMemorySize(); pc++ )
	{
		if ( m_memory[ pc ].flags == MemReference::MEM_UNKNOWN )
		{
			m_memory[ pc ].data = mem.Read_Internal( pc );
		}
	}

	for ( u32 pc = 0; pc < mem.GetAllocatedMemorySize(); )
	{
		const CommandInfo& command = DecodeAt( pc );
		if ( IsMemoryAlreadyDisassembled( pc, command ) )
		{
			pc += command.m_nSize;
		}
		else
		{
			if ( command.IsIllegalOpcode() || 
				 ( command.m_instruction == BRK ) ||
				 ( command.m_nSize + pc >= mem.GetAllocatedMemorySize() ) )
			{
				pc++;
			}
			else
			{
				MarkAsDecoded( pc, command, false );
				pc += command.m_nSize;
			}
		}
	}

	for ( u32 pc = 0; pc < mem.GetAllocatedMemorySize(); )
	{
		code += toHex( u16(pc), true ) + " ";
		if ( pc == 0xfff4)
		{
			int i = 0;i++;
		}
		//
		// Instruction ?
		//
		if ( m_memory[ pc ].flags & MemReference::MEM_INSTRUCTION )
		{
			const CommandInfo* pCommand;
			cpu.DisassembleInstruction( (u16)pc, code, &pCommand );

			pc += pCommand->m_nSize;
		}
		else
		{
			//
			// Data
			//
			if ( ( pc < mem.GetAllocatedMemorySize( ) - 1 ) && ( ( m_memory[ pc + 1 ].flags & MemReference::MEM_INSTRUCTION ) == 0 ) )
			{
				u8 bytelo = m_memory[ pc ].data;
				u8 bytehi = m_memory[ pc + 1 ].data;
				u16 word = ( u16( bytehi ) << 8 ) + u16( bytelo );
				code += toHex( bytelo, false ) + " " + toHex( bytehi, false ) + "        EQUW " + toHex( word, true );
				pc+=2;
			}
			else
			{
				//
				// Data
				//
				u8 byte = m_memory[ pc ].data;
				code += toHex( byte, false ) + "           EQUB " + toHex( byte, true );
				pc++;
			}
		}
		code += "\n";
	}
}

//-------------------------------------------------------------------------------------------------

void Disassembler::DisassembleFrom( int pc )
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

		MarkAsDecoded( pc, command, true );

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
					DisassembleFrom( address );
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