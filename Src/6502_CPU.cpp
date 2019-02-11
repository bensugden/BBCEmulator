#include "stdafx.h"
#include "stdio.h"
#include "6502_CPU.h"

//-------------------------------------------------------------------------------------------------

void RegisterInstructionHandlers( OpcodeTable& opcodeTable );

//=================================================================================================
//
// Main 6502 Execution Loop
//
//=================================================================================================

CPU::CPU( )
{
	m_pClock = nullptr;
	nBreakpoints = 0;
	m_bExternalBreakpoint = false;
	RegisterInstructionHandlers( m_opcodeTable );
	Reset();
}

//-------------------------------------------------------------------------------------------------

void CPU::Reset()
{
	//
	// Perform Reset
	//
	nTotalCycles = 0;
	reg.S = 0xFF;
	reg.A = reg.X = reg.Y = 0;
	reg.P = 0;
	SetFlag( flag_I, 1 );
	SetFlag( flag_unused, 1 );
	SetFlag( flag_B, 1 ); // temp
	m_pendingInterrupt = INTERRUPT_NONE;

	reg.PC = mem.ReadAddress( c_Reset_Vector );
}

//-------------------------------------------------------------------------------------------------

CPU::~CPU() 
{
}

//-------------------------------------------------------------------------------------------------
void fn_IRQ();
void fn_NMI();

bool CPU::ProcessSingleInstruction()
{
	//
	// Interrupt?
	//
	if ( m_pendingInterrupt != INTERRUPT_NONE )
	{
		switch ( m_pendingInterrupt )
		{
			case INTERRUPT_IRQ :
			{
				if ( GetFlag( flag_I ) == 0 )
				{
					//	cpu.ThrowBreakpoint(std::string("INTERRUPT_IRQ"));
					m_pendingInterrupt = INTERRUPT_NONE;
					fn_IRQ();
				}
				break;
			}
			case INTERRUPT_NMI :
			{
				//cpu.ThrowBreakpoint(std::string("INTERRUPT_NMI"));
				m_pendingInterrupt = INTERRUPT_NONE;
				fn_NMI();

				break;
			}
			case INTERRUPT_RESET :
			{
				//cpu.ThrowBreakpoint(std::string("INTERRUPT_RESET"));
				m_pendingInterrupt = INTERRUPT_NONE;
				Reset();
				break;
			}
			default :
				break;
		}
	}
	

	//
	// fetch
	//
	u8 opcode = mem.Read( reg.PC );
	IncPC();
	Tick();

	//
	// decode
	//
	const CommandInfo& command = m_opcodeTable.GetCommandForOpcode( opcode );

	//
	// dispatch
	//
	assert( command.m_functionHandler != nullptr ); // illegal opcode.
	command.m_functionHandler( );

	//
	// check for breakpoint
	//
	return CheckBreakPoints() || CheckExternalBreakpoints();
}

//-------------------------------------------------------------------------------------------------

void CPU::ClearBreakpoints()
{
	nBreakpoints = 0;
}

//-------------------------------------------------------------------------------------------------

bool CPU::CheckBreakPoints()
{
	for ( int i = 0; i < nBreakpoints; i++ )
	{
		if ( reg.PC == pBreakpoints[ i ] )
		{
			m_breakpointReason = std::string( "Breakpoint hit at :" + Utils::toHex( reg.PC ) );
			return true;
		}
	}
	return false;
}
//-------------------------------------------------------------------------------------------------

bool CPU::CheckExternalBreakpoints()
{
	bool bBreak = m_bExternalBreakpoint ;
	m_bExternalBreakpoint = false;
	return bBreak;
}

//-------------------------------------------------------------------------------------------------

void CPU::SetBreakpoint( u16 address )
{
	assert( nBreakpoints < c_maxBreakpoints );
	pBreakpoints[ nBreakpoints++ ] = address;
}

//-------------------------------------------------------------------------------------------------

static inline int GetMemSizePerEA( EAddressingMode mode )
{
	if ( mode == mode_imp || mode == mode_invalid )
		return 0;
	if ( mode >= mode_abs && mode < mode_rel )
		return 2;
	return 1;
};

//-------------------------------------------------------------------------------------------------

int CPU::GetBytesAtPC( int pc, u8* bytes )
{
	bytes[ 0 ] = mem.Read_Internal( pc++ );

	const CommandInfo& command = m_opcodeTable.GetCommandForOpcode( bytes[ 0 ] );
	EAddressingMode addrmode = command.m_addressingMode;

	int nSize = GetMemSizePerEA( addrmode );
	int nCount = 1;
	while ( nCount <= nSize )
	{
		bytes[ nCount++ ] = mem.Read_Internal( pc++ );
	}
	return nCount;
}

//-------------------------------------------------------------------------------------------------
void CPU::Disassemble( const CPU::Registers& reg, const u8* bytes, string& dissassemble, const CommandInfo** ppOutCommand )
{
	dissassemble = toHex( (u16)reg.PC, false ) + " ";

	u8 opcode = *bytes++;

	const CommandInfo& command = m_opcodeTable.GetCommandForOpcode( opcode );
	EAddressingMode addrmode = command.m_addressingMode;
	dissassemble += toHex( (u8)opcode, false ) + " ";

	int nSize = GetMemSizePerEA( addrmode );
	if ( nSize == 1 )
	{
		u8 address = *bytes++;

		dissassemble += toHex( (u8)address, false ) + "    ";
		dissassemble += "    " + command.m_name + " ";

		switch ( addrmode )
		{
		case mode_imm:
			//"imm = #$00"
			dissassemble += "#"+toHex( address ); 
			break;
		case mode_zp:
			//"zp = $00"
			dissassemble += toHex( address );
			break;
		case mode_zpx:
			//"zpx = $00,X"
			dissassemble += toHex( address )+",X";
			break;
		case mode_zpy:
			//"zpy = $00,Y"
			dissassemble += toHex( address )+",Y";
			break;
		case mode_izx:
			//"izx = ($00,X)"
			dissassemble += "("+toHex( address )+",X)";
			break;
		case mode_izy:
			//"izy = ($00),Y"
			dissassemble += "("+toHex( address )+"),Y";
			break;
		case mode_rel:
			//"rel = $0000 (PC-relative)"
			dissassemble += toHex( (u16)(reg.PC + 2 + ((s8)address)) )+" (PC-relative)"; 
			break;
		default:
			break;
		}
	}
	else if ( nSize == 2 )
	{
		u8 lo = *bytes++;
		u8 hi = *bytes++;

		dissassemble += toHex( (u8)lo, false ) + " " +toHex( (u8)hi, false ) + " ";

		u16 address = lo + ( u16( hi ) <<8 );

		dissassemble += "    " + command.m_name + " ";

		switch ( addrmode )
		{
		case mode_abs:
			//"abs = $0000"
			dissassemble += toHex( address ); 
			break;
		case mode_abx:
			//"abx = $0000,X"
			dissassemble += toHex( address )+",X"; 
			break;
		case mode_aby:
			//"aby = $0000,Y"
			dissassemble += toHex( address )+",Y"; 
			break;
		case mode_ind:
			//"ind = ($0000)"
			dissassemble += "("+toHex( address )+")"; 
			break;
		default:
			break;
		}
	}
	else
	{
		dissassemble += "          " + command.m_name + " ";
	}
	int iExtraSpaces = 48 - (int)dissassemble.length();
	while ( iExtraSpaces-- > 0 )
		dissassemble +=" ";
	dissassemble +="  A:" + toHex( (u8)reg.A );
	dissassemble +="  X:" + toHex( (u8)reg.X );
	dissassemble +="  Y:" + toHex( (u8)reg.Y );
	dissassemble +="  S:" + toHex( (u8)reg.S );

	dissassemble += " N:"; dissassemble += ( reg.GetFlag( flag_N ) ? "1" : "0" );
	dissassemble += " V:"; dissassemble += ( reg.GetFlag( flag_V ) ? "1" : "0" );
	dissassemble += " B:"; dissassemble += ( reg.GetFlag( flag_B ) ? "1" : "0" );
	dissassemble += " *:"; dissassemble += ( reg.GetFlag( flag_unused ) ? "1" : "0" );
	dissassemble += " D:"; dissassemble += ( reg.GetFlag( flag_D ) ? "1" : "0" );
	dissassemble += " I:"; dissassemble += ( reg.GetFlag( flag_I ) ? "1" : "0" );
	dissassemble += " Z:"; dissassemble += ( reg.GetFlag( flag_Z ) ? "1" : "0" );
	dissassemble += " C:"; dissassemble += ( reg.GetFlag( flag_C ) ? "1" : "0" );

	dissassemble += " P:="; dissassemble += toHex( (u8)reg.P );
	if ( ppOutCommand != nullptr )
	{
		*ppOutCommand = &command;
	}
	dissassemble += "\n";
}

//-------------------------------------------------------------------------------------------------
