#include "stdafx.h"
#include "stdio.h"
#include "6502_CPU.h"

//-------------------------------------------------------------------------------------------------

void RegisterInstructionHandlers( OpcodeTable& opcodeTable );
void fn_IRQ();
void fn_NMI();

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
	m_nLastNMI = 0;
	reg.PC = mem.ReadAddress( c_Reset_Vector );
}

//-------------------------------------------------------------------------------------------------

CPU::~CPU() 
{
}

//-------------------------------------------------------------------------------------------------

bool CPU::ProcessSingleInstruction()
{
	//
	// Interrupt?
	//
	if ( m_pendingInterrupt )
	{
		if ( m_pendingInterrupt & INTERRUPT_IRQ )
		{
			if ( GetFlag( flag_I ) == 0 )
			{
				debugOutput("INTERRUPT_IRQ");
				//	cpu.ThrowBreakpoint(std::string("INTERRUPT_IRQ"));
				fn_IRQ();
			}
		}

		if (( m_pendingInterrupt & INTERRUPT_NMI )&&( !m_nLastNMI ))
		{
			//cpu.ThrowBreakpoint(std::string("INTERRUPT_NMI"));
			debugOutput("INTERRUPT_NMI");
			fn_NMI();
			SetFlag( flag_I, 1 );
		}

		if ( m_pendingInterrupt & INTERRUPT_RESET )
		{
			//cpu.ThrowBreakpoint(std::string("INTERRUPT_RESET"));
			Reset();
		}
	}
	m_nLastNMI = m_pendingInterrupt & INTERRUPT_NMI;

	//
	// init performance tests
	//
	#ifdef TEST_CYCLE_TIMES
	u64 startCycleTime = m_pClock->GetClockCounter();
	m_dbgExtraCycleDueToPageFault = false;
	m_dbgTookBranch = false;
	#endif

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
	// check performance metrics
	//
	#ifdef TEST_CYCLE_TIMES
	u64 totalCycles = m_pClock->GetClockCounter() - startCycleTime;
	u64 okCycles = command.m_cycles;
	if ( command.m_addCycleIfPageBoundaryCrossed && m_dbgExtraCycleDueToPageFault )
		okCycles++;
	if ( command.m_isConditionalBranch && m_dbgTookBranch )
		okCycles++;
	assert( totalCycles == okCycles);
	assert( ( !m_dbgExtraCycleDueToPageFault ) || (command.m_addCycleIfPageBoundaryCrossed) );
	assert( ( !m_dbgTookBranch ) || ( command.m_isConditionalBranch) );
	#endif

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
int CPU::DisassembleInstruction( u16 PC, string& dissassemble, const CommandInfo** ppOutCommand )
{
	u8 opcode = mem.Read_Internal( PC );

	const CommandInfo& command = m_opcodeTable.GetCommandForOpcode( opcode );
	EAddressingMode addrmode = command.m_addressingMode;
	dissassemble += toHex( (u8)opcode, false ) + " ";

	int nSize = GetMemSizePerEA( addrmode );
	if ( nSize == 1 )
	{
		u8 address = mem.Read_Internal( PC + 1 );

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
			dissassemble += toHex( (u16)(PC + 2 + ((s8)address)) )+" (PC-relative)"; 
			break;
		default:
			break;
		}
	}
	else if ( nSize == 2 )
	{
		u8 lo =  mem.Read_Internal( PC + 1 );
		u8 hi =  mem.Read_Internal( PC + 2 );

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

	if ( ppOutCommand != nullptr )
	{
		*ppOutCommand = &command;
	}
	return nSize + 1; 
}

//-------------------------------------------------------------------------------------------------

int CPU::DisassembleAtCPUState( const Registers& reg, string& dissassemble, const CommandInfo** ppOutCommand )
{
	dissassemble = toHex( (u16)reg.PC, false ) + " ";

	int iSize = DisassembleInstruction( reg.PC, dissassemble, ppOutCommand );

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

	dissassemble += "\n";
	return iSize;
}



//-------------------------------------------------------------------------------------------------
