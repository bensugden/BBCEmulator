#include "stdafx.h"
#include "stdio.h"

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
static inline void FetchPCL( u16 address )
{
	//   $FFFE   R  fetch PCL
	mem.ReadLoByte( address, cpu.PC );
	Tick();
}

//-------------------------------------------------------------------------------------------------
static inline void FetchPCH( u16 address )
{
	//   $FFFF   R  fetch PCH
	mem.ReadHiByte( address, cpu.PC );
	Tick();
}

//-------------------------------------------------------------------------------------------------

void SetFlags( u8 flags )
{
	cpu.P |= flags;
}
//-------------------------------------------------------------------------------------------------

void ClearFlags( u8 flags )
{
	cpu.P &= ~flags;
}

//=================================================================================================
//
// Main 6502 Execution Loop
//
//=================================================================================================
CPUEmulator::CPUEmulator()
{
	//
	// Perform Reset
	//
	mem.ReadLoByte( cpu.c_Reset_Lo, cpu.PC );
	mem.ReadHiByte( cpu.c_Reset_Hi, cpu.PC );
}

//-------------------------------------------------------------------------------------------------

void CPUEmulator::ProcessSingleInstruction()
{
	//
	// fetch
	//
	u8 opcode = FetchOpcode(); 
	
	//
	// decode
	//
	const CommandInfo& command = GetCommandForOpcode( opcode );

	//
	// dispatch
	//
	command.m_instruction( command );
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
		DiscardNextPC();
		SetFlags( flag_B ); PushPCH( ); 
		PushPCL( ); 
		PushP( ); 
		FetchPCL( cpu.c_IRQ_Lo ); 
		FetchPCH( cpu.c_IRQ_Hi ); 
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
		DiscardNextPC();
		IncS(); Tick();
		PullPCL();
		PullPCH();
		IncPC(); Tick();
	}
	//-------------------------------------------------------------------------------------------------
	/*
	PHA, PHP

        #  address R/W description
       --- ------- --- -----------------------------------------------
        1    PC     R  fetch opcode, increment PC
        2    PC     R  read next instruction byte (and throw it away)
        3  $0100,S  W  push register on stack, decrement S

     PLA, PLP

        #  address R/W description
       --- ------- --- -----------------------------------------------
        1    PC     R  fetch opcode, increment PC
        2    PC     R  read next instruction byte (and throw it away)
        3  $0100,S  R  increment S
        4  $0100,S  R  pull register from stack

     JSR

        #  address R/W description
       --- ------- --- -------------------------------------------------
        1    PC     R  fetch opcode, increment PC
        2    PC     R  fetch low address byte, increment PC
        3  $0100,S  R  internal operation (predecrement S?)
        4  $0100,S  W  push PCH on stack, decrement S
        5  $0100,S  W  push PCL on stack, decrement S
        6    PC     R  copy low address byte to PCL, fetch high address
                       byte to PCH
					   */
};

//=================================================================================================
//
//	Accumulator or implied addressing
//
//=================================================================================================
namespace AccumulatorOrImpliedAddressing
{
/*
        #  address R/W description
       --- ------- --- -----------------------------------------------
        1    PC     R  fetch opcode, increment PC
        2    PC     R  read next instruction byte (and throw it away)
		*/
}
//=================================================================================================
//
//	Immediate addressing
//
//=================================================================================================
namespace ImmediateAddressing
{
  /*

        #  address R/W description
       --- ------- --- ------------------------------------------
        1    PC     R  fetch opcode, increment PC
        2    PC     R  fetch value, increment PC
		*/
}

//=================================================================================================
//
//	Absolute addressing
//
//=================================================================================================
namespace AbsoluteAddressing
{
/*
     JMP

        #  address R/W description
       --- ------- --- -------------------------------------------------
        1    PC     R  fetch opcode, increment PC
        2    PC     R  fetch low address byte, increment PC
        3    PC     R  copy low address byte to PCL, fetch high address
                       byte to PCH

     Read instructions (LDA, LDX, LDY, EOR, AND, ORA, ADC, SBC, CMP, BIT,
                        LAX, NOP)

        #  address R/W description
       --- ------- --- ------------------------------------------
        1    PC     R  fetch opcode, increment PC
        2    PC     R  fetch low byte of address, increment PC
        3    PC     R  fetch high byte of address, increment PC
        4  address  R  read from effective address

     Read-Modify-Write instructions (ASL, LSR, ROL, ROR, INC, DEC,
                                     SLO, SRE, RLA, RRA, ISB, DCP)

        #  address R/W description
       --- ------- --- ------------------------------------------
        1    PC     R  fetch opcode, increment PC
        2    PC     R  fetch low byte of address, increment PC
        3    PC     R  fetch high byte of address, increment PC
        4  address  R  read from effective address
        5  address  W  write the value back to effective address,
                       and do the operation on it
        6  address  W  write the new value to effective address

     Write instructions (STA, STX, STY, SAX)

        #  address R/W description
       --- ------- --- ------------------------------------------
        1    PC     R  fetch opcode, increment PC
        2    PC     R  fetch low byte of address, increment PC
        3    PC     R  fetch high byte of address, increment PC
        4  address  W  write register to effective address
		*/
}
//=================================================================================================
//
//	Zero page addressing
//
//=================================================================================================
namespace ZeroPageAddressing
{
/*
     Read instructions (LDA, LDX, LDY, EOR, AND, ORA, ADC, SBC, CMP, BIT,
                        LAX, NOP)

        #  address R/W description
       --- ------- --- ------------------------------------------
        1    PC     R  fetch opcode, increment PC
        2    PC     R  fetch address, increment PC
        3  address  R  read from effective address

     Read-Modify-Write instructions (ASL, LSR, ROL, ROR, INC, DEC,
                                     SLO, SRE, RLA, RRA, ISB, DCP)

        #  address R/W description
       --- ------- --- ------------------------------------------
        1    PC     R  fetch opcode, increment PC
        2    PC     R  fetch address, increment PC
        3  address  R  read from effective address
        4  address  W  write the value back to effective address,
                       and do the operation on it
        5  address  W  write the new value to effective address

     Write instructions (STA, STX, STY, SAX)

        #  address R/W description
       --- ------- --- ------------------------------------------
        1    PC     R  fetch opcode, increment PC
        2    PC     R  fetch address, increment PC
        3  address  W  write register to effective address
		*/
}
//=================================================================================================
//
//	Zero page indexed addressing
//
//=================================================================================================

namespace ZeroPageIndexedAddressing
{
/*
     Read instructions (LDA, LDX, LDY, EOR, AND, ORA, ADC, SBC, CMP, BIT,
                        LAX, NOP)

        #   address  R/W description
       --- --------- --- ------------------------------------------
        1     PC      R  fetch opcode, increment PC
        2     PC      R  fetch address, increment PC
        3   address   R  read from address, add index register to it
        4  address+I* R  read from effective address

       Notes: I denotes either index register (X or Y).

              * The high byte of the effective address is always zero,
                i.e. page boundary crossings are not handled.

     Read-Modify-Write instructions (ASL, LSR, ROL, ROR, INC, DEC,
                                     SLO, SRE, RLA, RRA, ISB, DCP)

        #   address  R/W description
       --- --------- --- ---------------------------------------------
        1     PC      R  fetch opcode, increment PC
        2     PC      R  fetch address, increment PC
        3   address   R  read from address, add index register X to it
        4  address+X* R  read from effective address
        5  address+X* W  write the value back to effective address,
                         and do the operation on it
        6  address+X* W  write the new value to effective address

       Note: * The high byte of the effective address is always zero,
               i.e. page boundary crossings are not handled.

     Write instructions (STA, STX, STY, SAX)

        #   address  R/W description
       --- --------- --- -------------------------------------------
        1     PC      R  fetch opcode, increment PC
        2     PC      R  fetch address, increment PC
        3   address   R  read from address, add index register to it
        4  address+I* W  write to effective address

       Notes: I denotes either index register (X or Y).

              * The high byte of the effective address is always zero,
                i.e. page boundary crossings are not handled.


*/
}
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

	void fn_ReadInstructions( const CommandInfo& command )
	{
	}

	//-------------------------------------------------------------------------------------------------
	//
	// Read-Modify-Write instructions (ASL, LSR, ROL, ROR, INC, DEC,
	//                                     SLO, SRE, RLA, RRA, ISB, DCP)
	//
	//-------------------------------------------------------------------------------------------------
	void ASL(u8& val)
	{
		cpu.SetFlagValue(flag_C, (val & 0x80));
		val <<= 1;
		cpu.SetZN(val);
	}
	//-------------------------------------------------------------------------------------------------
	void LSR(u8& val)
	{
		cpu.SetFlagValue(flag_C, (val & 0x1));
		val >>= 1;
		cpu.SetZN(val);
	}
	//-------------------------------------------------------------------------------------------------
	void ROL(u8& val)
	{
		u8 C = cpu.IsFlagSet(flag_C);
		cpu.SetFlagValue(flag_C, (val & 0x80));
		val <<= 1;
		val |= C;
		cpu.SetZN( val );
	}
	//-------------------------------------------------------------------------------------------------
	void ROR(u8& val)
	{
		u8 C = cpu.IsFlagSet(flag_C);
		cpu.SetFlagValue(flag_C, (val & 0x1));
		val >>= 1;
		val |= C<<7;
		cpu.SetZN(val);
	}
	//-------------------------------------------------------------------------------------------------
	void INC(u8& val)
	{
		val ++;
		cpu.SetZN(val);
	}
	//-------------------------------------------------------------------------------------------------
	void DEC(u8& val)
	{
		val--;
		cpu.SetZN(val);
	}
	template <void(*T)(u8 &)>
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
		T(value);
		Tick();

		mem.Write( address, value );
		Tick();
	}
	// ben look at this!
	void(*testFn0)(const CommandInfo& command) = fn_ReadModifyWriteInstructions<ASL>;
	void(*testFn1)(const CommandInfo& command) = fn_ReadModifyWriteInstructions<LSR>;

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
//	Relative addressing
//
//=================================================================================================

namespace RelativeAddressing
{
/*
Relative addressing (BCC, BCS, BNE, BEQ, BPL, BMI, BVC, BVS)

        #   address  R/W description
       --- --------- --- ---------------------------------------------
        1     PC      R  fetch opcode, increment PC
        2     PC      R  fetch operand, increment PC
        3     PC      R  Fetch opcode of next instruction,
                         If branch is taken, add operand to PCL.
                         Otherwise increment PC.
        4+    PC*     R  Fetch opcode of next instruction.
                         Fix PCH. If it did not change, increment PC.
        5!    PC      R  Fetch opcode of next instruction,
                         increment PC.

       Notes: The opcode fetch of the next instruction is included to
              this diagram for illustration purposes. When determining
              real execution times, remember to subtract the last
              cycle.

              * The high byte of Program Counter (PCH) may be invalid
                at this time, i.e. it may be smaller or bigger by $100.

              + If branch is taken, this cycle will be executed.

              ! If branch occurs to different page, this cycle will be
                executed.
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
		u8 pointer = FetchPointer();

		mem.Read( pointer );
		pointer += cpu.X;
		Tick();

		u16 address = Get16BitAddressFromPointer( pointer );

		u8 value = command.m_operation( 0, 0 );
		mem.Write( address, value );
		Tick();
	}
};

//=================================================================================================
//
//	Indirect indexed addressing
//
//=================================================================================================

namespace IndexedIndirectAddressing
{
   	//-------------------------------------------------------------------------------------------------
	//
	// Read instructions (LDA, EOR, AND, ORA, ADC, SBC, CMP)
	//
   	//-------------------------------------------------------------------------------------------------
	/*
        #    address   R/W description
       --- ----------- --- ------------------------------------------
        1      PC       R  fetch opcode, increment PC
        2      PC       R  fetch pointer address, increment PC
        3    pointer    R  fetch effective address low
        4   pointer+1   R  fetch effective address high,
                           add Y to low byte of effective address
        5   address+Y*  R  read from effective address,
                           fix high byte of effective address
        6+  address+Y   R  read from effective address

       Notes: The effective address is always fetched from zero page,
              i.e. the zero page boundary crossing is not handled.

              * The high byte of the effective address may be invalid
                at this time, i.e. it may be smaller by $100.

              + This cycle will be executed only if the effective address
                was invalid during cycle #5, i.e. page boundary was crossed.
	*/
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
        3    pointer    R  fetch effective address low
        4   pointer+1   R  fetch effective address high,
                           add Y to low byte of effective address
        5   address+Y*  R  read from effective address,
                           fix high byte of effective address
        6   address+Y   R  read from effective address
        7   address+Y   W  write the value back to effective address,
                           and do the operation on it
        8   address+Y   W  write the new value to effective address

       Notes: The effective address is always fetched from zero page,
              i.e. the zero page boundary crossing is not handled.

              * The high byte of the effective address may be invalid
                at this time, i.e. it may be smaller by $100.
	*/

	//-------------------------------------------------------------------------------------------------
	//
    // Write instructions (STA, SHA)
	//
	//-------------------------------------------------------------------------------------------------
	/*
        #    address   R/W description
       --- ----------- --- ------------------------------------------
        1      PC       R  fetch opcode, increment PC
        2      PC       R  fetch pointer address, increment PC
        3    pointer    R  fetch effective address low
        4   pointer+1   R  fetch effective address high,
                           add Y to low byte of effective address
        5   address+Y*  R  read from effective address,
                           fix high byte of effective address
        6   address+Y   W  write to effective address

       Notes: The effective address is always fetched from zero page,
              i.e. the zero page boundary crossing is not handled.

              * The high byte of the effective address may be invalid
                at this time, i.e. it may be smaller by $100.
	*/
};

//=================================================================================================
//
//	Absolute indirect addressing (JMP)
//
//=================================================================================================

namespace AbsoluteIndirectAddressing
{
/*


        #   address  R/W description
       --- --------- --- ------------------------------------------
        1     PC      R  fetch opcode, increment PC
        2     PC      R  fetch pointer address low, increment PC
        3     PC      R  fetch pointer address high, increment PC
        4   pointer   R  fetch low address to latch
        5  pointer+1* R  fetch PCH, copy latch to PCL

       Note: * The PCH will always be fetched from the same page
               than PCL, i.e. page boundary crossing is not handled.
			   */
};

/*
                How Real Programmers Acknowledge Interrupts

  With RMW instructions:

        ; beginning of combined raster/timer interrupt routine
        LSR $D019       ; clear VIC interrupts, read raster interrupt flag to C
        BCS raster      ; jump if VIC caused an interrupt
        ...             ; timer interrupt routine

        Operational diagram of LSR $D019:

          #  data  address  R/W
         --- ----  -------  ---  ---------------------------------
          1   4E     PC      R   fetch opcode
          2   19    PC+1     R   fetch address low
          3   D0    PC+2     R   fetch address high
          4   xx    $D019    R   read memory
          5   xx    $D019    W   write the value back, rotate right
          6  xx/2   $D019    W   write the new value back

        The 5th cycle acknowledges the interrupt by writing the same
        value back. If only raster interrupts are used, the 6th cycle
        has no effect on the VIC. (It might acknowledge also some
        other interrupts.)

  With indexed addressing:

        ; acknowledge interrupts to both CIAs
        LDX #$10
        LDA $DCFD,X

        Operational diagram of LDA $DCFD,X:

          #  data  address  R/W  description
         --- ----  -------  ---  ---------------------------------
          1   BD     PC      R   fetch opcode
          2   FD    PC+1     R   fetch address low
          3   DC    PC+2     R   fetch address high, add X to address low
          4   xx    $DC0D    R   read from address, fix high byte of address
          5   yy    $DD0D    R   read from right address

        ; acknowledge interrupts to CIA 2
        LDX #$10
        STA $DDFD,X

        Operational diagram of STA $DDFD,X:

          #  data  address  R/W  description
         --- ----  -------  ---  ---------------------------------
          1   9D     PC      R   fetch opcode
          2   FD    PC+1     R   fetch address low
          3   DC    PC+2     R   fetch address high, add X to address low
          4   xx    $DD0D    R   read from address, fix high byte of address
          5   ac    $DE0D    W   write to right address

  With branch instructions:

        ; acknowledge interrupts to CIA 2
                LDA #$00  ; clear N flag
                JMP $DD0A
        DD0A    BPL $DC9D ; branch
        DC9D    BRK       ; return

        You need the following preparations to initialize the CIA registers:

                LDA #$91  ; argument of BPL
                STA $DD0B
                LDA #$10  ; BPL
                STA $DD0A
                STA $DD08 ; load the ToD values from the latches
                LDA $DD0B ; freeze the ToD display
                LDA #$7F
                STA $DC0D ; assure that $DC0D is $00

        Operational diagram of BPL $DC9D:

          #  data  address  R/W  description
         --- ----  -------  ---  ---------------------------------
          1   10    $DD0A    R   fetch opcode
          2   91    $DD0B    R   fetch argument
          3   xx    $DD0C    R   fetch opcode, add argument to PCL
          4   yy    $DD9D    R   fetch opcode, fix PCH
        ( 5   00    $DC9D    R   fetch opcode )

        ; acknowledge interrupts to CIA 1
                LSR       ; clear N flag
                JMP $DCFA
        DCFA    BPL $DD0D
        DD0D    BRK

        ; Again you need to set the ToD registers of CIA 1 and the
        ; Interrupt Control Register of CIA 2 first.

        Operational diagram of BPL $DD0D:

          #  data  address  R/W  description
         --- ----  -------  ---  ---------------------------------
          1   10    $DCFA    R   fetch opcode
          2   11    $DCFB    R   fetch argument
          3   xx    $DCFC    R   fetch opcode, add argument to PCL
          4   yy    $DC0D    R   fetch opcode, fix PCH
        ( 5   00    $DD0D    R   fetch opcode )

        ; acknowledge interrupts to CIA 2 automagically
                ; preparations
                LDA #$7F
                STA $DD0D       ; disable all interrupt sources of CIA2
                LDA $DD0E
                AND #$BE        ; ensure that $DD0C remains constant
                STA $DD0E       ; and stop the timer
                LDA #$FD
                STA $DD0C       ; parameter of BPL
                LDA #$10
                STA $DD0B       ; BPL
                LDA #$40
                STA $DD0A       ; RTI/parameter of LSR
                LDA #$46
                STA $DD09       ; LSR
                STA $DD08       ; load the ToD values from the latches
                LDA $DD0B       ; freeze the ToD display
                LDA #$09
                STA $0318
                LDA #$DD
                STA $0319       ; change NMI vector to $DD09
                LDA #$FF        ; Try changing this instruction's operand
                STA $DD05       ; (see comment below).
                LDA #$FF
                STA $DD04       ; set interrupt frequency to 1/65536 cycles
                LDA $DD0E
                AND #$80
                ORA #$11
                LDX #$81
                STX $DD0D       ; enable timer interrupt
                STA $DD0E       ; start timer

                LDA #$00        ; To see that the interrupts really occur,
                STA $D011       ; use something like this and see how
        LOOP    DEC $D020       ; changing the byte loaded to $DD05 from
                BNE LOOP        ; #$FF to #$0F changes the image.

        When an NMI occurs, the processor jumps to Kernal code, which jumps to
        ($0318), which points to the following routine:

        DD09    LSR $40         ; clear N flag
                BPL $DD0A       ; Note: $DD0A contains RTI.

        Operational diagram of BPL $DD0A:

          #  data  address  R/W  description
         --- ----  -------  ---  ---------------------------------
          1   10    $DD0B    R   fetch opcode
          2   11    $DD0C    R   fetch argument
          3   xx    $DD0D    R   fetch opcode, add argument to PCL
          4   40    $DD0A    R   fetch opcode, (fix PCH)

  With RTI:

        ; the fastest possible interrupt handler in the 6500 family
                ; preparations
                SEI
                LDA $01         ; disable ROM and enable I/O
                AND #$FD
                ORA #$05
                STA $01
                LDA #$7F
                STA $DD0D       ; disable CIA 2's all interrupt sources
                LDA $DD0E
                AND #$BE        ; ensure that $DD0C remains constant
                STA $DD0E       ; and stop the timer
                LDA #$40
                STA $DD0C       ; store RTI to $DD0C
                LDA #$0C
                STA $FFFA
                LDA #$DD
                STA $FFFB       ; change NMI vector to $DD0C
                LDA #$FF        ; Try changing this instruction's operand
                STA $DD05       ; (see comment below).
                LDA #$FF
                STA $DD04       ; set interrupt frequency to 1/65536 cycles
                LDA $DD0E
                AND #$80
                ORA #$11
                LDX #$81
                STX $DD0D       ; enable timer interrupt
                STA $DD0E       ; start timer

                LDA #$00        ; To see that the interrupts really occur,
                STA $D011       ; use something like this and see how
        LOOP    DEC $D020       ; changing the byte loaded to $DD05 from
                BNE LOOP        ; #$FF to #$0F changes the image.

        When an NMI occurs, the processor jumps to Kernal code, which
        jumps to ($0318), which points to the following routine:

        DD0C    RTI

        How on earth can this clear the interrupts? Remember, the
        processor always fetches two successive bytes for each
        instruction.

        A little more practical version of this is redirecting the NMI
        (or IRQ) to your own routine, whose last instruction is JMP
        $DD0C or JMP $DC0C.  If you want to confuse more, change the 0
        in the address to a hexadecimal digit different from the one
        you used when writing the RTI.

        Or you can combine the latter two methods:

        DD09    LSR $xx  ; xx is any appropriate BCD value 00-59.
                BPL $DCFC
        DCFC    RTI

        This example acknowledges interrupts to both CIAs.

  If you want to confuse the examiners of your code, you can use any
of these techniques. Although these examples use no undefined opcodes,
they do not necessarily run correctly on CMOS processors. However, the
RTI example should run on 65C02 and 65C816, and the latter branch
instruction example might work as well.

  The RMW instruction method has been used in some demos, others were
developed by Marko M"akel"a. His favourite is the automagical RTI
method, although it does not have any practical applications, except
for some time dependent data decryption routines for very complicated
copy protections.

	*/
