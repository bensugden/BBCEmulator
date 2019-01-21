#include "stdafx.h"

//-------------------------------------------------------------------------------------------------
//
// Here's the computer
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
	u8 flags;

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
};

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


	u8* m_pMemory;
	CPUState& m_cpu;
};

//-------------------------------------------------------------------------------------------------

static CPUState		cpu;
static MemoryState	mem( cpu, 65536 );

//-------------------------------------------------------------------------------------------------
//
// Common CPU Functions
//
//-------------------------------------------------------------------------------------------------
static inline void IncPC( )
{
	cpu.PC++;
}

//-------------------------------------------------------------------------------------------------
static inline void IncS( )
{
	cpu.S++;
}

//-------------------------------------------------------------------------------------------------
static inline void DecS( )
{
	cpu.S--;
}

//-------------------------------------------------------------------------------------------------

static inline void Tick( )
{
	cpu.Tick();
}
//-------------------------------------------------------------------------------------------------

static inline u8 FetchOpcode( )
{
	//    PC     R  fetch opcode, increment PC
	u8 opcode = mem.Read( cpu.PC );
	IncPC(); 
	Tick();
	return opcode;
}

//-------------------------------------------------------------------------------------------------

static inline u8 FetchPointer( )
{
	//    PC       R  fetch pointer address, increment PC
	u8 location = mem.Read( cpu.PC );
	IncPC(); 
	Tick();
	return location;
}

//-------------------------------------------------------------------------------------------------

static inline u16 Get16BitAddressFromPointer( u8 pointer )
{
	// pointer    R  read from the address
	u16 address = 0;
	mem.ReadLoByte( pointer, address );
	Tick();

	mem.ReadHiByte( ( pointer + 1 ) & 0xff, address );
	Tick();

	return address;
	//not sure
}
//-------------------------------------------------------------------------------------------------

static inline void DiscardNextPC( )
{
	//    PC     R  read next instruction byte (and throw it away),
	//			increment PC
	IncPC(); 
	Tick();
}

//-------------------------------------------------------------------------------------------------
static inline void PushP( )
{
	//  $0100,S  W  push P on stack, decrement S
	mem.Write( cpu.StackAddress( ), cpu.P );
	DecS(); 
	Tick();
}

//-------------------------------------------------------------------------------------------------
static inline void PushPCH( )
{
	//  $0100,S  W  push PCH on stack (with B flag set), decrement S
	mem.WriteHiByte( cpu.StackAddress( ), cpu.PC );
	DecS(); 
	Tick();
}

//-------------------------------------------------------------------------------------------------
static inline void PushPCL( )
{
	//  $0100,S  W  push PCL on stack, decrement S
	mem.WriteLoByte( cpu.StackAddress( ), cpu.PC );
	DecS(); 
	Tick();
}

//-------------------------------------------------------------------------------------------------
static inline void PullP( )
{
    //  $0100,S  R  pull P from stack, increment S
	cpu.P = mem.Read( cpu.StackAddress( ) );
	IncS();
	Tick();
}

//-------------------------------------------------------------------------------------------------
static inline void PullPCL( )
{
    //  $0100,S  R  pull PCL from stack, increment S
	cpu.PC = mem.ReadLoByte( cpu.StackAddress( ), cpu.PC );
	IncS(); 
	Tick();
}
//-------------------------------------------------------------------------------------------------
static inline void PullPCH( )
{
    //  $0100,S  R  pull PCH from stack
	cpu.PC = mem.ReadHiByte( cpu.StackAddress( ), cpu.PC );
	Tick();
}

//-------------------------------------------------------------------------------------------------
static inline void FetchPCL( u16 address = 0xFFFE )
{
	//   $FFFE   R  fetch PCL
	mem.ReadLoByte( address, cpu.PC );
	Tick();
}

//-------------------------------------------------------------------------------------------------
static inline void FetchPCH( u16 address = 0xFFFF )
{
	//   $FFFF   R  fetch PCH
	mem.ReadHiByte( address, cpu.PC );
	Tick();
}

//-------------------------------------------------------------------------------------------------

void SetFlags( u8 flags )
{
	cpu.flags |= flags;
}
//-------------------------------------------------------------------------------------------------

void ClearFlags( u8 flags )
{
	cpu.flags &= ~flags;
}

//=================================================================================================
//
// Function Tables
//
//=================================================================================================

//=================================================================================================
//
//  Instructions accessing the stack
//
//=================================================================================================

namespace StackInstructions
{
	void fn_BRK( const CommandInfo& command )
	{
		/*
		--- ------- --- -----------------------------------------------
		1    PC     R  fetch opcode, increment PC
		2    PC     R  read next instruction byte (and throw it away),
						increment PC
		3  $0100,S  W  push PCH on stack (with B flag set), decrement S
		4  $0100,S  W  push PCL on stack, decrement S
		5  $0100,S  W  push P on stack, decrement S
		6   $FFFE   R  fetch PCL
		7   $FFFF   R  fetch PCH
		*/

		u8 opcode = FetchOpcode(); 

		DiscardNextPC();
		SetFlags( flag_B ); PushPCH( ); 
		PushPCL( ); 
		PushP( ); 
		FetchPCL( ); 
		FetchPCH( ); 
	}

	//-------------------------------------------------------------------------------------------------

	void fn_RTI( const CommandInfo& command )
	{
		/*
			#  address R/W description
		   --- ------- --- -----------------------------------------------
			1    PC     R  fetch opcode, increment PC
			2    PC     R  read next instruction byte (and throw it away)
			3  $0100,S  R  increment S
			4  $0100,S  R  pull P from stack, increment S
			5  $0100,S  R  pull PCL from stack, increment S
			6  $0100,S  R  pull PCH from stack
			*/

		u8 opcode = FetchOpcode();

		DiscardNextPC();
		IncS(); Tick();
		PullP();
		PullPCL();
		PullPCH();
	}

	//-------------------------------------------------------------------------------------------------

	void fn_RTS( const CommandInfo& command )
	{    
		/*
			#  address R/W description
		   --- ------- --- -----------------------------------------------
			1    PC     R  fetch opcode, increment PC
			2    PC     R  read next instruction byte (and throw it away)
			3  $0100,S  R  increment S
			4  $0100,S  R  pull PCL from stack, increment S
			5  $0100,S  R  pull PCH from stack
			6    PC     R  increment PC
			*/
		//todo
	}
	//-------------------------------------------------------------------------------------------------
};

//=================================================================================================
//
// Absolute indexed addressing
//
//=================================================================================================

namespace AbsoluteIndexedAddressing
{
	/*
	Read instructions (LDA, LDX, LDY, EOR, AND, ORA, ADC, SBC, CMP, BIT,
							LAX, LAE, SHS, NOP)

			#   address  R/W description
		   --- --------- --- ------------------------------------------
			1     PC      R  fetch opcode, increment PC
			2     PC      R  fetch low byte of address, increment PC
			3     PC      R  fetch high byte of address,
							 add index register to low address byte,
							 increment PC
			4  address+I* R  read from effective address,
							 fix the high byte of effective address
			5+ address+I  R  re-read from effective address

		   Notes: I denotes either index register (X or Y).

				  * The high byte of the effective address may be invalid
					at this time, i.e. it may be smaller by $100.

				  + This cycle will be executed only if the effective address
					was invalid during cycle #4, i.e. page boundary was crossed.
		*/
	//-------------------------------------------------------------------------------------------------
	//
	// Read-Modify-Write instructions (ASL, LSR, ROL, ROR, INC, DEC,
	//                                     SLO, SRE, RLA, RRA, ISB, DCP)
	//
	//-------------------------------------------------------------------------------------------------

	void fn_ReadModifyWriteInstructions( const CommandInfo& command )
	{
		/*
			#   address  R/W description
		   --- --------- --- ------------------------------------------
			1    PC       R  fetch opcode, increment PC
			2    PC       R  fetch low byte of address, increment PC
			3    PC       R  fetch high byte of address,
							 add index register X to low address byte,
							 increment PC
			4  address+X* R  read from effective address,
							 fix the high byte of effective address
			5  address+X  R  re-read from effective address
			6  address+X  W  write the value back to effective address,
							 and do the operation on it
			7  address+X  W  write the new value to effective address

		   Notes: * The high byte of the effective address may be invalid
					at this time, i.e. it may be smaller by $100.
		*/
		u8 opcode = FetchOpcode();

		u16 address = 0;
		mem.ReadLoByte( cpu.PC, address );
		IncPC(); 
		Tick();

		mem.ReadHiByte( cpu.PC, address );
		address = ( address & 0xffffff00 ) + ( address + cpu.X ) & 0xff;
		IncPC(); 
		Tick();

		u8 value = mem.Read( address );
		address = address + cpu.X;
		Tick();

		mem.Write( address, value );
		value = command.m_operation( value, 0 );
		Tick();

		mem.Write( address, value );
		Tick();
	}

	/*
	
     Write instructions (STA, STX, STY, SHA, SHX, SHY)

        #   address  R/W description
       --- --------- --- ------------------------------------------
        1     PC      R  fetch opcode, increment PC
        2     PC      R  fetch low byte of address, increment PC
        3     PC      R  fetch high byte of address,
                         add index register to low address byte,
                         increment PC
        4  address+I* R  read from effective address,
                         fix the high byte of effective address
        5  address+I  W  write to effective address

       Notes: I denotes either index register (X or Y).

              * The high byte of the effective address may be invalid
                at this time, i.e. it may be smaller by $100. Because
                the processor cannot undo a write to an invalid
                address, it always reads from the address first.
				*/
};

//=================================================================================================
//
//	Indexed indirect addressing
//
//=================================================================================================

namespace IndexedIndirectAddressing
{
	//-------------------------------------------------------------------------------------------------
	//
	//     Read instructions (LDA, ORA, EOR, AND, ADC, CMP, SBC, LAX)
	//
	//-------------------------------------------------------------------------------------------------
	/*
		#    address   R/W description
		--- ----------- --- ------------------------------------------
		1      PC       R  fetch opcode, increment PC
		2      PC       R  fetch pointer address, increment PC
		3    pointer    R  read from the address, add X to it
		4   pointer+X   R  fetch effective address low
		5  pointer+X+1  R  fetch effective address high
		6    address    R  read from effective address

		Note: The effective address is always fetched from zero page,
				i.e. the zero page boundary crossing is not handled.
	*/
	void fn_ReadInstructions( const CommandInfo& command )
	{
		u8 opcode = FetchOpcode();
		u8 pointer = FetchPointer();
		
		mem.Read( pointer );
		pointer += cpu.X;
		Tick();

		u16 address = Get16BitAddressFromPointer( pointer );

		u8 value = mem.Read( address );
		Tick();

		command.m_operation( value, 0 );
		// no tick as writing to register
	}
	
	//-------------------------------------------------------------------------------------------------
	//
	// Read-Modify-Write instructions (SLO, SRE, RLA, RRA, ISB, DCP)
	//
	//-------------------------------------------------------------------------------------------------
	/*
        #    address   R/W description
       --- ----------- --- ------------------------------------------
        1      PC       R  fetch opcode, increment PC
        2      PC       R  fetch pointer address, increment PC
        3    pointer    R  read from the address, add X to it
        4   pointer+X   R  fetch effective address low
        5  pointer+X+1  R  fetch effective address high
        6    address    R  read from effective address
        7    address    W  write the value back to effective address,
                           and do the operation on it
        8    address    W  write the new value to effective address

       Note: The effective address is always fetched from zero page,
             i.e. the zero page boundary crossing is not handled.
	*/
	void fn_ReadModifyWriteInstructions( const CommandInfo& command )
	{
		u8 opcode = FetchOpcode();
		u8 pointer = FetchPointer();

		mem.Read( pointer );
		pointer += cpu.X;
		Tick();

		u16 address = Get16BitAddressFromPointer( pointer );

		u8 value = mem.Read( address );
		Tick();

		mem.Write( address, value );
		value = command.m_operation( value, 0 );
		Tick();

		mem.Write( address, value );
		Tick();
	}

	
   	//-------------------------------------------------------------------------------------------------
	//
	// Write instructions (STA, SAX)
	//
   	//-------------------------------------------------------------------------------------------------
	/*
        #    address   R/W description
       --- ----------- --- ------------------------------------------
        1      PC       R  fetch opcode, increment PC
        2      PC       R  fetch pointer address, increment PC
        3    pointer    R  read from the address, add X to it
        4   pointer+X   R  fetch effective address low
        5  pointer+X+1  R  fetch effective address high
        6    address    W  write to effective address

       Note: The effective address is always fetched from zero page,
             i.e. the zero page boundary crossing is not handled.
	 */
	void fn_WriteInstructions( const CommandInfo& command )
	{
		u8 opcode = FetchOpcode();
		u8 pointer = FetchPointer();

		mem.Read( pointer );
		pointer += cpu.X;
		Tick();

		u16 address = Get16BitAddressFromPointer( pointer );

		u8 value = command.m_operation( 0, 0 );
		mem.Write( address, value );
		Tick();
	}
}