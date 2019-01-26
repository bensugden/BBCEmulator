#pragma once

#include "6502_OpcodeTable.h"

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
		// todo setup default flag values
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
//
// Here's our RAM
//
//-------------------------------------------------------------------------------------------------

struct MemoryState
{
	MemoryState( CPUState& cpu, int nAllocation = 65536 )
		: m_cpu( cpu )
	{
		m_pMemory = new u8[ nAllocation ];
		memset( m_pMemory, 0, nAllocation );

		// todo - set up default memory values for 6502
	}

	inline u8 Read( int nAddress ) const
	{
		return m_pMemory[ nAddress ];
	}

	inline void Write( int nAddress, u8 value )
	{
		m_pMemory[ nAddress ] = value;
	}

	inline void WriteHiByte( int nAddress, u16 value )
	{
		m_pMemory[ nAddress ] = ( value >> 8) & 0xff;
	}

	inline void WriteLoByte( int nAddress, u16 value )
	{
		m_pMemory[ nAddress ] = value & 0xff;
	}

	inline u16 ReadHiByte( int nAddress, u16& valueToModify ) const
	{
		u8 readValue = Read( nAddress );
		valueToModify &= (0xffff00ff); 
		valueToModify |= ( readValue & 0xff ) << 8;
		return valueToModify;
	}

	inline u16 ReadLoByte( int nAddress, u16& valueToModify ) const
	{
		u8 readValue = Read( nAddress );
		valueToModify &= (0xffffff00); 
		valueToModify |= readValue & 0xff;
		return valueToModify;
	}

	void LoadROM( string filename, int nAddress )
	{
		CFile file( filename, "rb" );
		file.Load( m_pMemory + nAddress, file.GetLength(), 1 );
	}

	u8* m_pMemory;
	CPUState& m_cpu;
};

//-------------------------------------------------------------------------------------------------

class CPUEmulator
{
public:
	CPUEmulator( );
	~CPUEmulator( );
	void		ProcessSingleInstruction();
	int			DisassemblePC( int pc_in, string& dissassemble, const CommandInfo** ppOutCommand );

private:
	OpcodeTable m_opcodeTable;
};

//-------------------------------------------------------------------------------------------------

extern CPUState			cpu;
extern MemoryState		mem;

//-------------------------------------------------------------------------------------------------
//
// Some Functions (need wrapping in a class or something)
//
//-------------------------------------------------------------------------------------------------

void RegisterInstructionHandlers();

//-------------------------------------------------------------------------------------------------
