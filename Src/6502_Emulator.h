#pragma once

enum EFlagSetMode
{
	flag_set_unchanged = 0,
	flag_set_zero = 1,
	flag_set_one = 2,
	flag_set_conditional = 3
};

//-------------------------------------------------------------------------------------------------

enum EFlag
{
	flag_N = 1 << 0  ,		// negative flag (1 when result is negative)
	flag_V = 1 << 1 ,		// overflow flag (1 on signed overflow)
	flag_unused = 1 << 2 ,	// unused (always 1)
	flag_B = 1 << 3 ,		// break flag (1 when interupt was caused by a BRK)
	flag_D = 1 << 4 ,		// decimal flag (1 when CPU in BCD mode)
	flag_I = 1 << 5 ,		// IRQ flag (when 1"," no interupts will occur (exceptions are IRQs forced by BRK and NMIs))"
	flag_Z = 1 << 6 ,		// zero flag (1 when all bits of a result are 0)
	flag_C = 1 << 7 ,		// carry flag (1 on unsigned overflow)
};

//-------------------------------------------------------------------------------------------------

enum EAddressingMode
{
	mode_imp = 0,
	mode_imm ,
	mode_zp  ,
	mode_zpx ,
	mode_zpy ,
	mode_izx ,
	mode_izy ,
	mode_abs ,
	mode_abx ,
	mode_aby ,
	mode_ind ,
	mode_rel ,
	mode_invalid
};

//-------------------------------------------------------------------------------------------------

struct CommandInfo
{
	CommandInfo()
	{
		memset( m_flagMode, sizeof( m_flagMode ), 0 );
		m_addCycleIfPageBoundaryCrossed = false;
	}
	string				m_name;
	string				m_function;
	int					m_index;
	int					m_cycles;
	EAddressingMode		m_addressingMode;
	bool				m_addCycleIfPageBoundaryCrossed;
	EFlagSetMode		m_flagMode[ 8 ];
	u8					(*m_operation)(u8,u8);
};

//-------------------------------------------------------------------------------------------------