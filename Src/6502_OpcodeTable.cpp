#include "stdafx.h"
#include <ios>
#include <sstream>
#include <iomanip>
using namespace std;

//-------------------------------------------------------------------------------------------------
//
// Opcode Tables derived from http://www.oxyron.de/html/opcodes02.html
//
//-------------------------------------------------------------------------------------------------

string g_opcodeToInstructionName[]=
{
	"BRK","ORA","KIL","SLO","NOP","ORA","ASL","SLO","PHP","ORA","ASL","ANC","NOP","ORA","ASL","SLO",
	"BPL","ORA","KIL","SLO","NOP","ORA","ASL","SLO","CLC","ORA","NOP","SLO","NOP","ORA","ASL","SLO",
	"JSR","AND","KIL","RLA","BIT","AND","ROL","RLA","PLP","AND","ROL","ANC","BIT","AND","ROL","RLA",
	"BMI","AND","KIL","RLA","NOP","AND","ROL","RLA","SEC","AND","NOP","RLA","NOP","AND","ROL","RLA",
	"RTI","EOR","KIL","SRE","NOP","EOR","LSR","SRE","PHA","EOR","LSR","ALR","JMP","EOR","LSR","SRE",
	"BVC","EOR","KIL","SRE","NOP","EOR","LSR","SRE","CLI","EOR","NOP","SRE","NOP","EOR","LSR","SRE",
	"RTS","ADC","KIL","RRA","NOP","ADC","ROR","RRA","PLA","ADC","ROR","ARR","JMP","ADC","ROR","RRA",
	"BVS","ADC","KIL","RRA","NOP","ADC","ROR","RRA","SEI","ADC","NOP","RRA","NOP","ADC","ROR","RRA",
	"NOP","STA","NOP","SAX","STY","STA","STX","SAX","DEY","NOP","TXA","XAA","STY","STA","STX","SAX",
	"BCC","STA","KIL","AHX","STY","STA","STX","SAX","TYA","STA","TXS","TAS","SHY","STA","SHX","AHX",
	"LDY","LDA","LDX","LAX","LDY","LDA","LDX","LAX","TAY","LDA","TAX","LAX","LDY","LDA","LDX","LAX",
	"BCS","LDA","KIL","LAX","LDY","LDA","LDX","LAX","CLV","LDA","TSX","LAS","LDY","LDA","LDX","LAX",
	"CPY","CMP","NOP","DCP","CPY","CMP","DEC","DCP","INY","CMP","DEX","AXS","CPY","CMP","DEC","DCP",
	"BNE","CMP","KIL","DCP","NOP","CMP","DEC","DCP","CLD","CMP","NOP","DCP","NOP","CMP","DEC","DCP",
	"CPX","SBC","NOP","ISC","CPX","SBC","INC","ISC","INX","SBC","NOP","SBC","CPX","SBC","INC","ISC",
	"BEQ","SBC","KIL","ISC","NOP","SBC","INC","ISC","SED","SBC","NOP","ISC","NOP","SBC","INC","ISC"
};

//-------------------------------------------------------------------------------------------------
/*
	"imm = #$00"
	"zp = $00"
	"zpx = $00,X"
	"zpy = $00,Y"
	"izx = ($00,X)"
	"izy = ($00),Y"
	"abs = $0000"
	"abx = $0000,X"
	"aby = $0000,Y"
	"ind = ($0000)"
	"rel = $0000 (PC-relative)"
*/
//-------------------------------------------------------------------------------------------------

string g_addressingModesAndCycleTimes[256]={

	"7","izx 6","","izx 8","zp 3","zp 3","zp 5","zp 5","3","imm 2","2","imm 2","abs 4","abs 4","abs 6","abs 6",
	"rel 2*","izy 5*","","izy 8","zpx 4","zpx 4","zpx 6","zpx 6","2","aby 4*","2","aby 7","abx 4*","abx 4*","abx 7","abx 7",
	"abs 6","izx 6","","izx 8","zp 3","zp 3","zp 5","zp 5","4","imm 2","2","imm 2","abs 4","abs 4","abs 6","abs 6",
	"rel 2*","izy 5*","","izy 8","zpx 4","zpx 4","zpx 6","zpx 6","2","aby 4*","2","aby 7","abx 4*","abx 4*","abx 7","abx 7",
	"6","izx 6","","izx 8","zp 3","zp 3","zp 5","zp 5","3","imm 2","2","imm 2","abs 3","abs 4","abs 6","abs 6",
	"rel 2*","izy 5*","","izy 8","zpx 4","zpx 4","zpx 6","zpx 6","2","aby 4*","2","aby 7","abx 4*","abx 4*","abx 7","abx 7",
	"6","izx 6","","izx 8","zp 3","zp 3","zp 5","zp 5","4","imm 2","2","imm 2","ind 5","abs 4","abs 6","abs 6",
	"rel 2*","izy 5*","","izy 8","zpx 4","zpx 4","zpx 6","zpx 6","2","aby 4*","2","aby 7","abx 4*","abx 4*","abx 7","abx 7",
	"imm 2","izx 6","imm 2","izx 6","zp 3","zp 3","zp 3","zp 3","2","imm 2","2","imm 2","abs 4","abs 4","abs 4","abs 4",
	"rel 2*","izy 6","","izy 6","zpx 4","zpx 4","zpy 4","zpy 4","2","aby 5","2","aby 5","abx 5","abx 5","aby 5","aby 5",
	"imm 2","izx 6","imm 2","izx 6","zp 3","zp 3","zp 3","zp 3","2","imm 2","2","imm 2","abs 4","abs 4","abs 4","abs 4",
	"rel 2*","izy 5*","","izy 5*","zpx 4","zpx 4","zpy 4","zpy 4","2","aby 4*","2","aby 4*","abx 4*","abx 4*","aby 4*","aby 4*",
	"imm 2","izx 6","imm 2","izx 8","zp 3","zp 3","zp 5","zp 5","2","imm 2","2","imm 2","abs 4","abs 4","abs 6","abs 6",
	"rel 2*","izy 5*","","izy 8","zpx 4","zpx 4","zpx 6","zpx 6","2","aby 4*","2","aby 7","abx 4*","abx 4*","abx 7","abx 7",
	"imm 2","izx 6","imm 2","izx 8","zp 3","zp 3","zp 5","zp 5","2","imm 2","2","imm 2","abs 4","abs 4","abs 6","abs 6",
	"rel 2*","izy 5*","","izy 8","zpx 4","zpx 4","zpx 6","zpx 6","2","aby 4*","2","aby 7","abx 4*","abx 4*","abx 7","abx 7"
};
//	"*" : add 1 cycle if page boundary is crossed.			
//	add 1 cycle on branches if taken.			

//-------------------------------------------------------------------------------------------------


string g_fullCommandList[77*21]=
{

	// Logical and arithmetic commands:
	// Opcode","imp","imm","zp","zpx","zpy","izx","izy","abs","abx","aby","ind","rel","Function","N","V","B","D","I","Z","C"

	"ORA","","$9 ","$5 ","$15 ","","$1 ","$11 ","$0D","$1D","$19 ","","","A:=A or {adr}","*","","","","","*","",
	"AND","","$29 ","$25 ","$35 ","","$21 ","$31 ","$2D","$3D","$39 ","","","A:=A&{adr}","*","","","","","*","",
	"EOR","","$49 ","$45 ","$55 ","","$41 ","$51 ","$4D","$5D","$59 ","","","A:=A exor {adr}","*","","","","","*","",
	"ADC","","$69 ","$65 ","$75 ","","$61 ","$71 ","$6D","$7D","$79 ","","","A:=A+{adr}","*","*","","","","*","*",
	"SBC","","$E9","$E5","$F5","","$E1","$F1","$ED","$FD","$F9","","","A:=A-{adr}","*","*","","","","*","*",
	"CMP","","$C9","$C5","$D5","","$C1","$D1","$CD","$DD","$D9","","","A-{adr}","*","","","","","*","*",
	"CPX","","$E0","$E4","","","","","$EC","","","","","X-{adr}","*","","","","","*","*",
	"CPY","","$C0","$C4","","","","","$CC","","","","","Y-{adr}","*","","","","","*","*",
	"DEC","","","$C6","$D6","","","","$CE","$DE","","","","{adr}:={adr}-1","*","","","","","*","",
	"DEX","$CA","","","","","","","","","","","","X:=X-1","*","","","","","*","",
	"DEY","$88 ","","","","","","","","","","","","Y:=Y-1","*","","","","","*","",
	"INC","","","$E6","$F6","","","","$EE","$FE","","","","{adr}:={adr}+1","*","","","","","*","",
	"INX","$E8","","","","","","","","","","","","X:=X+1","*","","","","","*","",
	"INY","$C8","","","","","","","","","","","","Y:=Y+1","*","","","","","*","",
	"ASL","$0A","","$6 ","$16 ","","","","$0E","$1E","","","","{adr}:={adr}*2","*","","","","","*","*",
	"ROL","$2A","","$26 ","$36 ","","","","$2E","$3E","","","","{adr}:={adr}*2+C","*","","","","","*","*",
	"LSR","$4A","","$46 ","$56 ","","","","$4E","$5E","","","","{adr}:={adr}/2","*","","","","","*","*",
	"ROR","$6A","","$66 ","$76 ","","","","$6E","$7E","","","","{adr}:={adr}/2+C*128","*","","","","","*","*",

	// Move commands:
	// Opcode","imp","imm","zp","zpx","zpy","izx","izy","abs","abx","aby","ind","rel","Function","N","V","B","D","I","Z","C

	"LDA","","$A9","$A5","$B5","","$A1","$B1","$AD","$BD","$B9","","","A:={adr}","*","","","","","*","",
	"STA","","","$85 ","$95 ","","$81 ","$91 ","$8D","$9D","$99 ","","","{adr}:=A","","","","","","","",
	"LDX","","$A2","$A6","","$B6","","","$AE","","$BE","","","X:={adr}","*","","","","","*","",
	"STX","","","$86 ","","$96 ","","","$8E","","","","","{adr}:=X","","","","","","","",
	"LDY","","$A0","$A4","$B4","","","","$AC","$BC","","","","Y:={adr}","*","","","","","*","",
	"STY","","","$84 ","$94 ","","","","$8C","","","","","{adr}:=Y","","","","","","","",
	"TAX","$AA","","","","","","","","","","","","X:=A","*","","","","","*","",
	"TXA","$8A","","","","","","","","","","","","A:=X","*","","","","","*","",
	"TAY","$A8","","","","","","","","","","","","Y:=A","*","","","","","*","",
	"TYA","$98 ","","","","","","","","","","","","A:=Y","*","","","","","*","",
	"TSX","$BA","","","","","","","","","","","","X:=S","*","","","","","*","",
	"TXS","$9A","","","","","","","","","","","","S:=X","","","","","","","",
	"PLA","$68 ","","","","","","","","","","","","A:=+(S)","*","","","","","*","",
	"PHA","$48 ","","","","","","","","","","","","(S)-:=A","","","","","","","",
	"PLP","$28 ","","","","","","","","","","","","P:=+(S)","*","*","","*","*","*","*",
	"PHP","$8 ","","","","","","","","","","","","(S)-:=P","","","","","","","",

	// Jump/Flag commands:
	// Opcode","imp","imm","zp","zpx","zpy","izx","izy","abs","abx","aby","ind","rel","Function","N","V","B","D","I","Z","C

	"BPL","","","","","","","","","","","","$10 ","branch on N=0","","","","","","","",
	"BMI","","","","","","","","","","","","$30 ","branch on N=1","","","","","","","",
	"BVC","","","","","","","","","","","","$50 ","branch on V=0","","","","","","","",
	"BVS","","","","","","","","","","","","$70 ","branch on V=1","","","","","","","",
	"BCC","","","","","","","","","","","","$90 ","branch on C=0","","","","","","","",
	"BCS","","","","","","","","","","","","$B0","branch on C=1","","","","","","","",
	"BNE","","","","","","","","","","","","$D0","branch on Z=0","","","","","","","",
	"BEQ","","","","","","","","","","","","$F0","branch on Z=1","","","","","","","",
	"BRK","$0 ","","","","","","","","","","","","(S)-:=PC,P PC:=($FFFE)","","","1","","1","","",
	"RTI","$40 ","","","","","","","","","","","","P,PC:=+(S)","*","*","","*","*","*","*",
	"JSR","","","","","","","","$20 ","","","","","(S)-:=PC PC:={adr}","","","","","","","",
	"RTS","$60 ","","","","","","","","","","","","PC:=+(S)","","","","","","","",
	"JMP","","","","","","","","$4C","","","$6C","","PC:={adr}","","","","","","","",
	"BIT","","","$24 ","","","","","$2C","","","","","N:=b7 V:=b6 Z:=A&{adr}","*","*","","","","*","",
	"CLC","$18 ","","","","","","","","","","","","C:=0","","","","","","","0",
	"SEC","$38 ","","","","","","","","","","","","C:=1","","","","","","","1",
	"CLD","$D8","","","","","","","","","","","","D:=0","","","","0","","","",
	"SED","$F8","","","","","","","","","","","","D:=1","","","","1","","","",
	"CLI","$58 ","","","","","","","","","","","","I:=0","","","","","0","","",
	"SEI","$78 ","","","","","","","","","","","","I:=1","","","","","1","","",
	"CLV","$B8","","","","","","","","","","","","V:=0","","0","","","","","",
	"NOP","$EA","","","","","","","","","","","","","","","","","","","",

	// Illegal opcodes:
	// Opcode","imp","imm","zp","zpx","zpy","izx","izy","abs","abx","aby","ind","rel","Function","N","V","B","D","I","Z","C

	"SLO","","","$7 ","$17 ","","$3 ","$13 ","$0F","$1F","$1B","","","{adr}:={adr}*2 A:=A or {adr}","*","","","","","*","*",
	"RLA","","","$27 ","$37 ","","$23 ","$33 ","$2F","$3F","$3B","","","{adr}:={adr}rol A:=A and {adr}","*","","","","","*","*",
	"SRE","","","$47 ","$57 ","","$43 ","$53 ","$4F","$5F","$5B","","","{adr}:={adr}/2 A:=A exor {adr}","*","","","","","*","*",
	"RRA","","","$67 ","$77 ","","$63 ","$73 ","$6F","$7F","$7B","","","{adr}:={adr}ror A:=A adc {adr}","*","*","","","","*","*",
	"SAX","","","$87 ","","$97 ","$83 ","","$8F","","","","","{adr}:=A&X","","","","","","","",
	"LAX","","","$A7","","$B7","$A3","$B3","$AF","","$BF","","","A,X:={adr}","*","","","","","*","",
	"DCP","","","$C7","$D7","","$C3","$D3","$CF","$DF","$DB","","","{adr}:={adr}-1 A-{adr}","*","","","","","*","*",
	"ISC","","","$E7","$F7","","$E3","$F3","$EF","$FF","$FB","","","{adr}:={adr}+1 A:=A-{adr}","*","*","","","","*","*",
	"ANC","","$0B","","","","","","","","","","","A:=A&#{imm}","*","","","","","*","*",
	"ANC","","$2B","","","","","","","","","","","A:=A&#{imm}","*","","","","","*","*",
	"ALR","","$4B","","","","","","","","","","","A:=(A&#{imm})/2","*","","","","","*","*",
	"ARR","","$6B","","","","","","","","","","","A:=(A&#{imm})/2","*","*","","","","*","*",
	"XAA�","","$8B","","","","","","","","","","","A:=X&#{imm}","*","","","","","*","",
	"LAX�","","$AB","","","","","","","","","","","A,X:=#{imm}","*","","","","","*","",
	"AXS","","$CB","","","","","","","","","","","X:=A&X-#{imm}","*","","","","","*","*",
	"SBC","","$EB","","","","","","","","","","","A:=A-#{imm}","*","*","","","","*","*",
	"AHX�","","","","","","","$93 ","","","$9F","","","{adr}:=A&X&H","","","","","","","",
	"SHY�","","","","","","","","","$9C","","","","{adr}:=Y&H","","","","","","","",
	"SHX�","","","","","","","","","","$9E","","","{adr}:=X&H","","","","","","","",
	"TAS�","","","","","","","","","","$9B","","","S:=A&X {adr}:=S&H","","","","","","","",
	"LAS","","","","","","","","","","$BB","","","A,X,S:={adr}&S","*","","","","","*","",

};

//-------------------------------------------------------------------------------------------------

/*

	� = unstable in certain matters
	� = highly unstable (results are not predictable on some machines)
	A = Accumulator
	X = X-Register
	Y = Y-Register
	S = Stack-Pointer
	P = Status-Register
	+(S) = Stack-Pointer relative with pre-increment
	(S)- = Stack-Pointer relative with post-decrement

	Flags of the status register:

	"The processor status register has 8 bits"," where 7 are used as flags:"

	N = negative flag (1 when result is negative)
	V = overflow flag (1 on signed overflow)
	# = unused (always 1)
	B = break flag (1 when interrupt was caused by a BRK)
	D = decimal flag (1 when CPU in BCD mode)
	"I = IRQ flag (when 1"," no interrupts will occur (exceptions are IRQs forced by BRK and NMIs))"
	Z = zero flag (1 when all bits of a result are 0)
	C = carry flag (1 on unsigned overflow)

	Hardware vectors:

	$FFFA = NMI vector (NMI=not maskable interrupts)
	$FFFC = Reset vector
	$FFFE = IRQ vector

*/

//-------------------------------------------------------------------------------------------------

OpcodeTable::OpcodeTable()
{
	m_iRegisteredInstructionCount = 0;

	for ( int i = 0 ; i < 256; i++ )
	{
		m_instructionToOpcodeMap.insert(pair<string, int>( g_opcodeToInstructionName[ i ], i ));
		
		CommandInfo command;
		command.m_name = g_opcodeToInstructionName[ i ];
		command.m_index = i;

		//
		// Parse cycle timings and addressing mode
		//
		const string& s = g_addressingModesAndCycleTimes[ i ];
		const char* p = s.c_str();

		int iCycleIndex = 4;

		if ( s.length() == 0 )
		{
			command.m_addressingMode = mode_invalid;
			command.m_cycles = INT_MAX;
		}
		else
		{
			if ( p[0] < 58 ) // its a number
			{
				command.m_addressingMode = mode_imp ;
				iCycleIndex = 0;
			}
			else if ( p[0] == 'i' )
			{
				if ( p[1] == 'm' )
				{
					command.m_addressingMode = mode_imm ;
				}
				else if ( p[2] == 'x' )
				{
					command.m_addressingMode = mode_izx ;
				}
				else if ( p[2] == 'y' )
				{
					command.m_addressingMode = mode_izy ;
				}
				else
				{
					command.m_addressingMode = mode_ind ;
				}
			}
			else if ( p[0] == 'a' )
			{
				if ( p[2] == 's' )
				{
					command.m_addressingMode = mode_abs ;
				}
				else if ( p[2] == 'x' )
				{
					command.m_addressingMode = mode_abx ;
				}
				else 
				{
					command.m_addressingMode = mode_aby ;
				}
			}
			else if ( p[0] == 'z' )
			{
				if ( p[2] == 'x' )
				{
					command.m_addressingMode = mode_zpx ;
				}
				else if ( p[2] == 'y' )
				{
					command.m_addressingMode = mode_zpy ;
				}
				else
				{
					command.m_addressingMode = mode_zp ;
					iCycleIndex = 3;
				}
			}
			else
			{
				command.m_addressingMode = mode_rel ;
			}
		}

		command.m_nSize = 2;
		if ( command.m_addressingMode == mode_imp || command.m_addressingMode == mode_invalid )
			command.m_nSize = 1;
		if ( command.m_addressingMode >= mode_abs && command.m_addressingMode < mode_rel )
			command.m_nSize = 3;

		command.m_cycles = p[iCycleIndex]- 48;
		command.m_addCycleIfPageBoundaryCrossed = ( s.find("*")!=string::npos);
		command.m_isConditionalBranch = command.m_name[0]=='B';
		m_commands.push_back( command );
	}
	//
	// Finally iterate command lists to get flags etc set
	//
	for ( int i = 0 ; i < 77 * 21; i+= 21 )
	{
		string opcode = g_fullCommandList[ i + 00 ];
		for ( int ea = 0; ea < 12; ea++ )
		{
			if ( g_fullCommandList[ i + 1 + ea ].length() == 0 )
				continue;

			string hexValue = "0" + g_fullCommandList[ i + 1 + ea ] ;
			hexValue[1]='x';

			int opcodeWithEAIndex = std::stoi( hexValue, nullptr, 0 ); 
			CommandInfo& command = m_commands[ opcodeWithEAIndex ];

			//
			// Check we have correct opcode
			//
			assert( opcode.compare( 0, 3, command.m_name ) == 0 );
			command.m_instruction = (EInstruction)( i / 21 );
			
			//
			// Patch up duplicated illegal opcodes
			//
			if (command.m_instruction == _ANC)
				command.m_instruction = ANC;
			else if (command.m_instruction == _LAX)
				command.m_instruction = LAX;
			else if (command.m_instruction == _SBC)
				command.m_instruction = SBC;

			//
			// Mark flags
			//
			command.m_functionName = g_fullCommandList[ i + 13 ];
			for ( int flag = 0; flag < 7; flag++ )
			{
				EFlagSetMode flagMode = flag_set_unchanged;

				if ( g_fullCommandList[ i + 14 + flag ][ 0 ] == '0' )
					flagMode = flag_set_zero;
				if ( g_fullCommandList[ i + 14 + flag ][ 0 ] == '1' )
					flagMode = flag_set_one;
				if ( g_fullCommandList[ i + 14 + flag ][ 0 ] == '*' )
					flagMode = flag_set_conditional;

				if ( flag < 2 )
				{
					command.m_flagMode[ flag < 2 ? flag : flag + 1  ] = flagMode;
				}
			}
		}
	}
}

//-------------------------------------------------------------------------------------------------

bool OpcodeTable::SetFunctionHandler( EAddressingMode ea, EInstruction instruction, void (*functionHandler)( ) )
{
	//
	// Linear search - slow. Really only use at setup time
	//
	bool bFound = false;
	for ( u32 i = 0; i < m_commands.size(); i++ )
	{
		if  ( ( m_commands[ i ].m_instruction == instruction ) && 
			  ( m_commands[ i ].m_addressingMode == ea ) )
		{
			assert( m_commands[ i ].m_functionHandler == nullptr );
			m_commands[ i ].m_functionHandler = functionHandler;
			m_iRegisteredInstructionCount++;
			bFound = true;
		}
	}
	//
	// No such combination ?
	//
	assert( bFound );
	return bFound;
}

//-------------------------------------------------------------------------------------------------
template < int cycles > static void static_fn_NOP(  )
{
	for ( int i = 1; i < cycles; i++ )
	{
		cpu.Tick();
	}
	cpu.LastTick();
}
//-------------------------------------------------------------------------------------------------

void OpcodeTable::CheckForMissingOpcodes()
{
	for ( u32 i = 0; i < m_commands.size(); i++ )
	{
		if ( m_commands[ i ].m_functionHandler == nullptr )
		{ 
			if (( m_commands[ i ].m_name != "KIL" )&&( m_commands[ i ].m_name != "NOP" ))
			{
				assert(false);
			}
			else
			{
				if ( m_commands[ i ].m_cycles == 1 )
					m_commands[ i ].m_functionHandler = static_fn_NOP<1>;
				else
				if ( m_commands[ i ].m_cycles == 2 )
					m_commands[ i ].m_functionHandler = static_fn_NOP<2>;
				else
				if ( m_commands[ i ].m_cycles == 3 )
					m_commands[ i ].m_functionHandler = static_fn_NOP<3>;
				else
				if ( m_commands[ i ].m_cycles == 4 )
					m_commands[ i ].m_functionHandler = static_fn_NOP<4>;
				else
					m_commands[ i ].m_functionHandler = static_fn_NOP<1>;
				m_iRegisteredInstructionCount++;
			}
		}
	}
}


//-------------------------------------------------------------------------------------------------
