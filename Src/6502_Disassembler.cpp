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
	m_currentLabelIndex = 0 ;
}
//-------------------------------------------------------------------------------------------------

int Disassembler::GetDestAddress( int pc, const CommandInfo& command )
{
	int nAddress = -1;

	u8 lo = mem.Read_Internal( pc + 1 );
	if ( command.m_addressingMode == mode_rel )
	{
		nAddress = (int)(pc + 2 + ((s8)lo));
		if ( command.IsBranch() )
			CreateLabel( nAddress );
	}
	else if ( command.m_addressingMode == mode_imm )
	{
		nAddress= lo;
	}
	else if ( command.m_addressingMode == mode_abs )
	{
		u8 hi = mem.Read_Internal( pc + 2 );
		nAddress = int( lo + ( u16( hi ) <<8 ) );
		if ( command.IsBranch() )
			CreateLabel( nAddress );
	}
	else if (( command.m_addressingMode == mode_abx )||
		     ( command.m_addressingMode == mode_aby )||
			 ( command.m_addressingMode == mode_ind ))
	{
		// don't take branch because we don't know the offset - just create a label
		u8 hi = mem.Read_Internal( pc + 2 );
		int tempAddress = int( lo + ( u16( hi ) <<8 ) );
		if ( command.IsBranch() )
			CreateLabel( tempAddress );
	}

	return nAddress;
}

//-------------------------------------------------------------------------------------------------

const CommandInfo& Disassembler::DecodeAt( int pc )
{
	if ( pc == 0xd514 )
	{
		int i =0 ; i++;
	}
	//
	// Decode and read extra bytes for instruction
	//
	const CommandInfo& command = cpu.GetOpcodeTable().GetCommandForOpcode( mem.Read_Internal( pc ) );
	GetDestAddress( pc, command ); // call this to potentially cache any labels
	return command;
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

void Disassembler::RegenerateLabelNameInAddressOrder()
{
	std::vector< int > addresses;
	addresses.reserve( m_memToLabelMap.size() );
	for ( auto  it = m_memToLabelMap.begin(); it != m_memToLabelMap.end(); it++ )
	{
		addresses.push_back( (*it).first );
	}
	m_currentLabelIndex = 0;
	m_memToLabelMap.clear();
	for ( size_t i = 0; i < addresses.size(); i++ )
	{
		CreateLabel( addresses[ i ] );
	}
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
	//
	// Would prefer labels in address order for easy reading...
	//
	RegenerateLabelNameInAddressOrder();

	int nLine = 0;
	
	m_pcToLineMap.resize(0);
	m_lineToPCMap.resize(0);

	for ( u32 pc = 0; pc < mem.GetAllocatedMemorySize(); )
	{
		//
		// Label ?
		//
		auto it = m_memToLabelMap.find( pc );
		if ( it != m_memToLabelMap.end() )
		{
			code += "."+(*it).second + "\n";

			m_lineToPCMap.push_back( pc );
			nLine++;
		}

		//
		// Address
		//
		code += "    " + toHex( u16(pc), false ) + "   ";
		
		//
		// Instruction ?
		//
		int nInstructionSize = 0;
		if ( m_memory[ pc ].flags & MemReference::MEM_INSTRUCTION )
		{
			const CommandInfo* pCommand;
			cpu.DisassembleInstruction( (u16)pc, code, &pCommand, &m_memToLabelMap );

			nInstructionSize = pCommand->m_nSize;
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
				nInstructionSize = 2;
			}
			else
			{
				//
				// Data
				//
				u8 byte = m_memory[ pc ].data;
				code += toHex( byte, false ) + "           EQUB " + toHex( byte, true );
				nInstructionSize = 1;
			}
		}

		code += "\n";

		assert( m_pcToLineMap.size() == pc );
		assert( m_lineToPCMap.size() == nLine );
		
		for ( int i =0 ; i < nInstructionSize; i++ )
		{
			m_pcToLineMap.push_back( nLine );
		}
		m_lineToPCMap.push_back( pc );

		pc += nInstructionSize;
		nLine++;
	}
}

//-------------------------------------------------------------------------------------------------

void Disassembler::CreateLabel( int address )
{
	auto it = m_memToLabelMap.find( address );
	if ( it != m_memToLabelMap.end() )
	{
		return;
	}
	std::string labelname = "_"+toHex(u16(address), false);//+std::to_string(m_currentLabelIndex++);
	m_memToLabelMap.insert(std::pair<int,std::string>(address,labelname));
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