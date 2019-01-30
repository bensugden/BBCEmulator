#include "stdafx.h"
#include "stdio.h"

//-------------------------------------------------------------------------------------------------

extern CPUState			cpu;
extern MemoryState		mem;
void RegisterInstructionHandlers( OpcodeTable& opcodeTable );

//=================================================================================================
//
// Main 6502 Execution Loop
//
//=================================================================================================

CPUEmulator::CPUEmulator( )
{
	RegisterInstructionHandlers( m_opcodeTable );
}

//-------------------------------------------------------------------------------------------------

void CPUEmulator::Reset()
{
	//
	// Perform Reset
	//
	mem.ReadLoByte( cpu.c_Reset_Lo, cpu.PC );
	mem.ReadHiByte( cpu.c_Reset_Hi, cpu.PC );
}

//-------------------------------------------------------------------------------------------------

CPUEmulator::~CPUEmulator() 
{
}

//-------------------------------------------------------------------------------------------------

void CPUEmulator::ProcessSingleInstruction()
{
	//
	// fetch
	//
	u8 opcode = mem.Read( cpu.PC );
	cpu.IncPC();; 
	cpu.Tick();

	//
	// decode
	//
	const CommandInfo& command = m_opcodeTable.GetCommandForOpcode( opcode );

	//
	// dispatch
	//
	command.m_functionHandler( );
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
static inline std::string toHex( u8 i, bool bPrefix = true )
{
	char buffer[256];
	_itoa( i, buffer, 16 );
	string outp = string(buffer);
	while ( outp.length() < 2 )	
	{
		outp = "0" + outp;
	}
	for ( u32 i = 0 ; i < outp.length(); i++ )
	{
		if ( outp[i]>='a'&&outp[i]<='z')
			outp[i]+='A'-'a';
	}

	if ( bPrefix )
		return "$"+outp;
	return outp;
}
//-------------------------------------------------------------------------------------------------
static inline  std::string toHex( u16 i, bool bPrefix = true  )
{
	char buffer[256];
	_itoa( i, buffer, 16 );
	string outp = string(buffer);
	while ( outp.length() < 4 )	
	{
		outp = "0" + outp;
	}
	for ( u32 i = 0 ; i < outp.length(); i++ )
	{
		if ( outp[i]>='a'&&outp[i]<='z')
			outp[i]+='A'-'a';
	}

	if ( bPrefix )
		return "$"+outp;
	return outp;
}
//-------------------------------------------------------------------------------------------------

int CPUEmulator::DisassemblePC( int pc_in, string& dissassemble, const CommandInfo** ppOutCommand )
{
	int pc = pc_in;

	dissassemble = toHex( (u16)pc, false ) + " ";

	u8 opcode = mem.Read( pc++ );

	const CommandInfo& command = m_opcodeTable.GetCommandForOpcode( opcode );
	EAddressingMode addrmode = command.m_addressingMode;

	dissassemble += toHex( (u8)opcode, false ) + " ";

	int nSize = GetMemSizePerEA( addrmode );
	if ( nSize == 1 )
	{
		u8 address = mem.Read( pc++ );

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
			dissassemble += toHex( (u16)(pc + ((s8)address)) )+" (PC-relative)"; 
			break;
		default:
			break;
		}
	}
	else if ( nSize == 2 )
	{
		u8 lo = mem.Read( pc++ );
		u8 hi = mem.Read( pc++ );

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
	dissassemble +="  A:" + toHex( (u8)cpu.A );
	dissassemble +="  X:" + toHex( (u8)cpu.X );
	dissassemble +="  Y:" + toHex( (u8)cpu.Y );
	dissassemble +="  S:" + toHex( (u8)cpu.S );
	dissassemble += " C:"; dissassemble += (cpu.GetFlag(flag_C)?"1":"0");
	dissassemble += " Z:"; dissassemble += (cpu.GetFlag(flag_Z)?"1":"0");
	dissassemble += " I:"; dissassemble += (cpu.GetFlag(flag_I)?"1":"0");
	dissassemble += " D:"; dissassemble += (cpu.GetFlag(flag_D)?"1":"0");
	dissassemble += " *:"; dissassemble += (cpu.GetFlag(flag_unused)?"1":"0");
	dissassemble += " B:"; dissassemble += (cpu.GetFlag(flag_B)?"1":"0");
	dissassemble += " V:"; dissassemble += (cpu.GetFlag(flag_V)?"1":"0");
	dissassemble +="  N:"; dissassemble += (cpu.GetFlag(flag_N)?"1":"0");
	if ( ppOutCommand != nullptr )
	{
		*ppOutCommand = &command;
	}
	return pc;
}

//-------------------------------------------------------------------------------------------------
