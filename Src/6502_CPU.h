#pragma once

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
//
// Here's the CPU
//
//-------------------------------------------------------------------------------------------------

struct CPUState
{
	u8 A;
	u8 X;
	u8 Y;
	u8 S;
	u8 P;
	u16 PC;
	int nTotalCycles;

	CPUState()
	{
		nTotalCycles = 0;
		S = 0xFF;
		A = X = Y = 0;
		PC = c_Reset_Lo;
		P = 0;
		SetFlag( flag_I, 1 );
	}

	inline u16 StackAddress( )
	{
		return 0x100 + S;
	}

	void SetFlag(EFlag flag, u8 val)
	{
		P &= ~flag;
		if (val!=0)
			P |= flag;
	}

	u8 GetFlag(EFlag flag)
	{
		if (P&flag)
			return 1;
		return 0;
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

	inline void SetPCL( u8 val )
	{
		PC = (PC&0xff00)|val; 
	}

	//-------------------------------------------------------------------------------------------------

	inline void SetPCH( u8 val )
	{
		PC = (PC&0xff)|(u16( val ) << 8);
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
		PC++;
	}

	//-------------------------------------------------------------------------------------------------

	inline void DecPC( )
	{
		PC--;
	}
	//-------------------------------------------------------------------------------------------------
	
	inline void IncS( )
	{
		S++;
	}

	//-------------------------------------------------------------------------------------------------
	
	inline void DecS( )
	{
		S--;
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
};

//-------------------------------------------------------------------------------------------------