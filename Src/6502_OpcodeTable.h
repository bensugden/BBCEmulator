#pragma once

//-------------------------------------------------------------------------------------------------

enum EFlagSetMode
{
	flag_set_unchanged = 0,
	flag_set_zero = 1,
	flag_set_one = 2,
	flag_set_conditional = 3
};

//-------------------------------------------------------------------------------------------------

enum EAddressingMode
{
	mode_imp = 0,
	mode_imm ,	//"imm = #$00"
	mode_zp  ,	//"zp = $00"
	mode_zpx ,	//"zpx = $00,X"
	mode_zpy ,	//"zpy = $00,Y"
	mode_izx ,	//"izx = ($00,X)" 
	mode_izy ,	//"izy = ($00),Y"
	mode_abs ,	//"abs = $0000"
	mode_abx ,	//"abx = $0000,X" absolute indexed addressing
	mode_aby ,	//"aby = $0000,Y" absolute indexed addressing
	mode_ind ,	//"ind = ($0000)"
	mode_rel ,	//"rel = $0000 (PC-relative)"
	mode_invalid
};

//-------------------------------------------------------------------------------------------------

enum EInstruction
{
	ORA = 0, AND, EOR, ADC, SBC, CMP, CPX, CPY, DEC,
	DEX, DEY, INC, INX, INY, ASL, ROL, LSR, ROR,
	LDA, STA, LDX, STX, LDY, STY, TAX, TXA, TAY,
	TYA, TSX, TXS, PLA, PHA, PLP, PHP, BPL, BMI,
	BVC, BVS, BCC, BCS, BNE, BEQ, BRK, RTI, JSR,
	RTS, JMP, BIT, CLC, SEC, CLD, SED, CLI, SEI,
	CLV, NOP, SLO, RLA, SRE, RRA, SAX, LAX, DCP,
	ISC, ANC, _ANC, ALR, ARR, XAA, _LAX, AXS, _SBC,
	AHX, SHY, SHX, TAS, LAS
};

#define FIRST_ILLEGAL_OPCODE SLO

//-------------------------------------------------------------------------------------------------

struct CommandInfo
{
	CommandInfo()
	{
		memset( m_flagMode, sizeof( m_flagMode ), 0 );
		m_addCycleIfPageBoundaryCrossed = false;
		m_functionHandler = nullptr;
	}
	bool IsIllegalOpcode() const
	{
		return ( m_instruction >= FIRST_ILLEGAL_OPCODE );  
	}
	string				m_name;
	string				m_functionName;
	int					m_index;
	int					m_cycles;
	int					m_nSize;
	EAddressingMode		m_addressingMode;
	bool				m_addCycleIfPageBoundaryCrossed;
	EFlagSetMode		m_flagMode[ 8 ];
	void				(*m_functionHandler)( );
	EInstruction		m_instruction;
};

//-------------------------------------------------------------------------------------------------

class OpcodeTable
{
public:
	OpcodeTable();
	void					BuildOpcodeTables();
	bool					SetFunctionHandler( EAddressingMode ea, EInstruction instruction, void (*functionHandler)( ) );
	const CommandInfo&		GetCommandForOpcode( u8 opcode ) const ;

private:

	int						m_iRegisteredInstructionCount;
	map<string, int>		m_instructionToOpcodeMap;
	vector< CommandInfo >	m_commands;
};

//-------------------------------------------------------------------------------------------------
