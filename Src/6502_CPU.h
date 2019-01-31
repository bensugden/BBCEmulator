#pragma once

//-------------------------------------------------------------------------------------------------

enum EFlag
{
	flag_C = 1 << 0 ,		// carry flag (1 on unsigned overflow)
	flag_Z = 1 << 1 ,		// zero flag (1 when all bits of a result are 0)
	flag_I = 1 << 2 ,		// IRQ flag (when 1"," no interupts will occur (exceptions are IRQs forced by BRK and NMIs))"
	flag_D = 1 << 3 ,		// decimal flag (1 when CPU in BCD mode)
	flag_B = 1 << 4 ,		// break flag (1 when interupt was caused by a BRK)
	flag_unused = 1 << 5 ,	// unused (always 1)
	flag_V = 1 << 6 ,		// overflow flag (1 on signed overflow)
	flag_N = 1 << 7  ,		// negative flag (1 when result is negative)
};

#include "6502_OpcodeTable.h"

//-------------------------------------------------------------------------------------------------
//
// Here's the CPU
//
//-------------------------------------------------------------------------------------------------
static const int c_maxBreakpoints = 256;

struct CPU
{
	struct Registers
	{
		u8 A;
		u8 X;
		u8 Y;
		u8 S;
		u8 P;
		u16 PC;

		//-------------------------------------------------------------------------------------------------
	
		inline void SetFlag(EFlag flag, u8 val)
		{
			P &= ~flag;
			if (val!=0)
				P |= flag;
		}

		//-------------------------------------------------------------------------------------------------

		inline u8 GetFlag(EFlag flag) const
		{
			if (P&flag)
				return 1;
			return 0;
		}
		//-------------------------------------------------------------------------------------------------

	};
	Registers reg;
	int nTotalCycles;
	int nBreakpoints;
	u16 pBreakpoints[ c_maxBreakpoints ];

	//-------------------------------------------------------------------------------------------------

	CPU( );
	~CPU();

	//-------------------------------------------------------------------------------------------------

	inline u16 StackAddress( )
	{
		return 0x100 + reg.S;
	}

	//-------------------------------------------------------------------------------------------------

	int GetCycleCount() 
	{
		return nTotalCycles;
	}

	//-------------------------------------------------------------------------------------------------
	//
	// Common CPU Functions
	//
	//-------------------------------------------------------------------------------------------------

	inline void SetZN( u8 val )
	{
		SetFlag(flag_Z, val == 0);
		SetFlag(flag_N, (val & 0x80));
	}

	//-------------------------------------------------------------------------------------------------
	
	inline void SetFlag(EFlag flag, u8 val)
	{
		reg.SetFlag( flag, val );
	}

	//-------------------------------------------------------------------------------------------------

	inline u8 GetFlag(EFlag flag) const
	{
		return reg.GetFlag( flag );
	}

	//-------------------------------------------------------------------------------------------------

	inline void SetPCL( u8 val )
	{
		reg.PC = (reg.PC&0xff00)|val; 
	}

	//-------------------------------------------------------------------------------------------------

	inline void SetPCH( u8 val )
	{
		reg.PC = (reg.PC&0xff)|(u16( val ) << 8);
	}

	//-------------------------------------------------------------------------------------------------

	inline void Tick( )
	{
		nTotalCycles ++;
	}
	
	//-------------------------------------------------------------------------------------------------

	inline void LastTick( )
	{
		// do nothing - pipelines with next opcode fetch instruction
	}

	//-------------------------------------------------------------------------------------------------

	inline void IncPC( )
	{
		reg.PC++;
	}

	//-------------------------------------------------------------------------------------------------

	inline void DecPC( )
	{
		reg.PC--;
	}

	//-------------------------------------------------------------------------------------------------
	
	inline void IncS( )
	{
		reg.S++;
	}

	//-------------------------------------------------------------------------------------------------
	
	inline void DecS( )
	{
		reg.S--;
	}
	
	//-------------------------------------------------------------------------------------------------

	inline void ThrowBreakpoint()
	{
		m_bExternalBreakpoint = true;
	}

	//-------------------------------------------------------------------------------------------------
	//
	// 6502 Interrupt Vectors
	//
	static const int c_NMI_Lo		= 0xFFFA;
	static const int c_NMI_Hi		= 0xFFFB;
	static const int c_Reset_Lo		= 0xFFFC;
	static const int c_Reset_Hi		= 0xFFFD;
	static const int c_IRQ_Lo		= 0xFFFE;
	static const int c_IRQ_Hi		= 0xFFFF;

	//-------------------------------------------------------------------------------------------------

	void						Reset();
	bool						ProcessSingleInstruction(); // returns true if breakpoint hit
	
	//-------------------------------------------------------------------------------------------------
	//
	// Debug Stuff
	//
	//-------------------------------------------------------------------------------------------------
	struct BreakPoint
	{
		enum Type
		{
			BREAKPOINT,
			BREAK_ON_READ,
			BREAD_ON_WRITE
		};
		Type type;
		u16 address;
	};

	void						Disassemble( const Registers& reg, const u8* bytes, string& dissassemble, const CommandInfo** ppOutCommand );
	void						SetBreakpoint( u16 address );
	void						ClearBreakpoints();
	int							GetBytesAtPC( int pc, u8* bytes ); // grabs 1-3 bytes for next instruction

	static std::string			toHex( u8 i, bool bPrefix = true  );
	static std::string			toHex( u16 i, bool bPrefix = true  );

	//-------------------------------------------------------------------------------------------------
private:
	bool						CheckBreakPoints();
	bool						CheckExternalBreakpoints();

	OpcodeTable					m_opcodeTable;
	bool						m_bExternalBreakpoint;
	//-------------------------------------------------------------------------------------------------
public:

};

//-------------------------------------------------------------------------------------------------