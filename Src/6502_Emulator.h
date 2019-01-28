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
	MemoryState( u16 nUserMemory = 32768, u32 nAllocation = 65536 )
		: m_nEndUserMemory( nUserMemory )
		, m_maxAllocatedMemory( nAllocation )
	{
		m_pMemory = new u8[ nAllocation ];
		memset( m_pMemory, 0, nAllocation );

		//
		// This is a list of IDs for listener sub-systems 
		// i.e. areas which are memory mapped
		//
		m_pListenerIDs = new u8[ nUserMemory ];
		memset( m_pListenerIDs, 0, nUserMemory );
		m_nNumListeners = 0; //[0 indicates no listener]

		memset( m_listenerFunctionForSystem, 0, sizeof( m_listenerFunctionForSystem )) ;

		// todo - set up default memory values for 6502
	}

	//-------------------------------------------------------------------------------------------------
	//
	// Note: Inclusive memory address range 
	//
	void RegisterMemoryMappedSystem( u16 addressStart, u16 addressEnd, void (*listenerFunction)( u16 address, u8 value ) )
	{
		assert( m_nNumListeners < 255 );
		m_listenerFunctionForSystem[ ++m_nNumListeners ] = listenerFunction; // 0 will indicate no listener registered for this memory location
		for ( int i = addressStart; i <= addressEnd; i++ )
		{
			m_pListenerIDs[ addressStart - m_nEndUserMemory ] = m_nNumListeners;
		}
	}

	//-------------------------------------------------------------------------------------------------

	inline u8 Read( int nAddress ) const
	{
		return m_pMemory[ nAddress ];
	}

	//-------------------------------------------------------------------------------------------------

	inline void CheckWriteMemoryMapped( u16 nAddress, u8 value )
	{
		//
		// Is this write in user memory ? If so, not memory mapped.
		//
		if ( nAddress < m_nEndUserMemory )
			return;

		//
		// Check if a system registered this address as a memory mapped address
		//
		u8 nListenerID = m_pListenerIDs[ nAddress - m_nEndUserMemory ];
		if ( nListenerID == 0 )
			return;

		// 
		// Call system that is memory mapped to this address
		//
		m_listenerFunctionForSystem[ nListenerID ]( nAddress, value );
	}

	//-------------------------------------------------------------------------------------------------

	inline void Write( u16 nAddress, u8 value )
	{
		m_pMemory[ nAddress ] = value;
		CheckWriteMemoryMapped( nAddress, value );
	}

	//-------------------------------------------------------------------------------------------------

	inline void WriteHiByte( u16 nAddress, u16 value )
	{
		m_pMemory[ nAddress ] = ( value >> 8) & 0xff;
		CheckWriteMemoryMapped( nAddress, m_pMemory[ nAddress ] );
	}

	//-------------------------------------------------------------------------------------------------

	inline void WriteLoByte( u16 nAddress, u16 value )
	{
		m_pMemory[ nAddress ] = value & 0xff;
		CheckWriteMemoryMapped( nAddress, m_pMemory[ nAddress ] );
	}

	//-------------------------------------------------------------------------------------------------

	inline u16 ReadHiByte( u16 nAddress, u16& valueToModify ) const
	{
		u8 readValue = Read( nAddress );
		valueToModify &= (0xffff00ff); 
		valueToModify |= ( readValue & 0xff ) << 8;
		return valueToModify;
	}

	//-------------------------------------------------------------------------------------------------

	inline u16 ReadLoByte( u16 nAddress, u16& valueToModify ) const
	{
		u8 readValue = Read( nAddress );
		valueToModify &= (0xffffff00); 
		valueToModify |= readValue & 0xff;
		return valueToModify;
	}

	//-------------------------------------------------------------------------------------------------

	void LoadROM( string filename, u16 nAddress )
	{
		CFile file( filename, "rb" );
		file.Load( m_pMemory + nAddress, file.GetLength(), 1 );
	}

	//-------------------------------------------------------------------------------------------------

	u8*			m_pMemory;
	u16			m_nEndUserMemory;		// everything up to this point is guaranteed to NOT be memory mapped
	u32			m_maxAllocatedMemory; 
	u8*			m_pListenerIDs;
	void (*m_listenerFunctionForSystem[256])( u16 address, u8 value );
	u8			m_nNumListeners;
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
