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
	void				(*m_instruction)( const CommandInfo& command );
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
		// todo setup default flag values
	}

	inline u16 StackAddress( )
	{
		return 0x100 + S;
	}

	void Tick( )
	{
		nTotalCycles ++;
	}
	
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
	CPUEmulator();
	~CPUEmulator() {};
	void ProcessSingleInstruction();
};

//-------------------------------------------------------------------------------------------------
extern CPUState			cpu;
extern MemoryState		mem;
//-------------------------------------------------------------------------------------------------
//
// Some Functions (need wrapping in a class or something)
//
//-------------------------------------------------------------------------------------------------

void				BuildOpcodeTables	( );
int					DisassemblePC		( int pc, string& dissassemble );
const CommandInfo&	GetCommandForOpcode	( u8 opcode );

//-------------------------------------------------------------------------------------------------
