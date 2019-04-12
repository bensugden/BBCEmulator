#pragma once

//-------------------------------------------------------------------------------------------------

#include "SystemClock.h"

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

//-------------------------------------------------------------------------------------------------

enum EInterrupt
{
	INTERRUPT_NONE	= 0,
	INTERRUPT_NMI	= 1,
	INTERRUPT_RESET = 2,
	INTERRUPT_IRQ   = 4
};

//-------------------------------------------------------------------------------------------------
//
// Enable this to do performance checking on cycle counts
//
//#define TEST_CYCLE_TIMES 

//-------------------------------------------------------------------------------------------------

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
			if (val!=0)
			{
				P |= flag;
			}
			else
			{
				P &= ~flag;
			}
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

	//-------------------------------------------------------------------------------------------------
	//
	// 6502 Interrupt Vectors
	//
	//-------------------------------------------------------------------------------------------------

	static const int c_NMI_Vector	= 0xFFFA;
	static const int c_Reset_Vector = 0xFFFC;
	static const int c_IRQ_Vector	= 0xFFFE;

	//-------------------------------------------------------------------------------------------------

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

	static inline int Frequency()
	{
		return 2000000;
	}

	//-------------------------------------------------------------------------------------------------

	inline u64 GetClockCounter() 
	{
		return m_pClock->GetClockCounter();
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
		if ( m_pClock )
		{
			m_pClock->Tick();
		}
	}
	
	//-------------------------------------------------------------------------------------------------

	inline void LastTick( )
	{
		Tick( );
		m_pClock->PollChips();
		// pipelines with next opcode fetch instruction
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

	void SetClock( ISystemClock* pClock )
	{
		m_pClock = pClock;
	}

	//-------------------------------------------------------------------------------------------------

	inline void ThrowBreakpoint( const char* reason )
	{
		m_bExternalBreakpoint = true;
		m_breakpointReason = std::string(reason);
	}

	//-------------------------------------------------------------------------------------------------

	inline void ThrowBreakpoint( std::string& reason )
	{
		m_bExternalBreakpoint = true;
		m_breakpointReason = reason;
	}
	
	//-------------------------------------------------------------------------------------------------
	
	inline void ThrowIRQ()
	{
		m_pendingInterrupt |= INTERRUPT_IRQ;
	}

	//-------------------------------------------------------------------------------------------------
	
	inline void ClearIRQ( )
	{
		m_pendingInterrupt &= ~INTERRUPT_IRQ;
	}

	//-------------------------------------------------------------------------------------------------

	inline void SetNMI( )
	{
		m_pendingInterrupt |= INTERRUPT_NMI;
	}

	//-------------------------------------------------------------------------------------------------

	inline void ClearNMI( )
	{
		m_pendingInterrupt &= ~INTERRUPT_NMI;
	}

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

	int							DisassembleAtCPUState( const Registers& reg_state, string& dissassemble, const CommandInfo** ppOutCommand );
	int							DisassembleInstruction( u16 PC, string& dissassemble, const CommandInfo** ppOutCommand = nullptr );

	void						SetBreakpoint( u16 address );
	void						ClearBreakpoints();
	int							GetBytesAtPC( int pc, u8* bytes ); // grabs 1-3 bytes for next instruction
	const std::string&			GetBreakpointReason() { return m_breakpointReason; }
	const OpcodeTable&			GetOpcodeTable() const { return m_opcodeTable; }
	bool						dbgWillNMINextCycle( ) { return ( m_pendingInterrupt & INTERRUPT_NMI )&&( !m_nLastNMI ); } // debug only
	//-------------------------------------------------------------------------------------------------
private:
	bool						CheckBreakPoints();
	bool						CheckExternalBreakpoints();

	OpcodeTable					m_opcodeTable;
	bool						m_bExternalBreakpoint;
	std::string					m_breakpointReason;
	u8							m_pendingInterrupt;
	u8							m_nLastNMI;
	class ISystemClock*			m_pClock;
	#ifdef TEST_CYCLE_TIMES
public:
	bool						m_dbgTookBranch;
	bool						m_dbgExtraCycleDueToPageFault;
	#endif
	//-------------------------------------------------------------------------------------------------
};

//-------------------------------------------------------------------------------------------------