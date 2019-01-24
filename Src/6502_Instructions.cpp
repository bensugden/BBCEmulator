#include "stdafx.h"
#include "6502_Operations.h"

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

static inline void LastTick( )
{
	// do nothing - pipelines with next opcode fetch instruction
}

//-------------------------------------------------------------------------------------------------

static inline u8 FetchPointer( )
{
	//    PC       R  fetch pointer address, increment PC
	u8 location = mem.Read( cpu.PC );
	IncPC(); 
	return location;
}
//-------------------------------------------------------------------------------------------------

static inline u8 FetchOperand( )
{
	//    PC       R  fetch operand, increment PC
	u8 operand = mem.Read( cpu.PC );
	IncPC(); 
	return operand;
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
	mem.Read(cpu.PC);
}

//-------------------------------------------------------------------------------------------------
static inline void PushP( )
{
	//  $0100,S  W  push P on stack, decrement S
	mem.Write( cpu.StackAddress( ), cpu.P );
}
//-------------------------------------------------------------------------------------------------
static inline void PushA()
{
	//  $0100,S  W  push A on stack, decrement S
	mem.Write(cpu.StackAddress(), cpu.A);
}
//-------------------------------------------------------------------------------------------------
static inline void PushPCH( )
{
	//  $0100,S  W  push PCH on stack (with B flag set), decrement S
	mem.WriteHiByte( cpu.StackAddress( ), cpu.PC );
}

//-------------------------------------------------------------------------------------------------
static inline void PushPCL( )
{
	//  $0100,S  W  push PCL on stack, decrement S
	mem.WriteLoByte( cpu.StackAddress( ), cpu.PC );
}

//-------------------------------------------------------------------------------------------------
static inline void PullP( )
{
    //  $0100,S  R  pull P from stack, increment S
	cpu.P = mem.Read( cpu.StackAddress( ) );
}

//-------------------------------------------------------------------------------------------------
static inline void PullA()
{
	//  $0100,S  R  pull A from stack, increment S
	cpu.A = mem.Read(cpu.StackAddress());
}

//-------------------------------------------------------------------------------------------------
static inline void PullPCL( )
{
    //  $0100,S  R  pull PCL from stack, increment S
	cpu.PC = mem.ReadLoByte( cpu.StackAddress( ), cpu.PC );
}
//-------------------------------------------------------------------------------------------------
static inline void PullPCH( )
{
    //  $0100,S  R  pull PCH from stack
	cpu.PC = mem.ReadHiByte( cpu.StackAddress( ), cpu.PC );
}


//-------------------------------------------------------------------------------------------------
static inline void SetPC( u8 lo, u8 hi )
{
	//   $FFFF   R  fetch PCH
	cpu.PC = lo | ( u16( hi ) << 8 );
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
	RegisterInstructionHandlers();
	//
	// Perform Reset
	//
	mem.ReadLoByte( cpu.c_Reset_Lo, cpu.PC );
	mem.ReadHiByte( cpu.c_Reset_Hi, cpu.PC );
}

//-------------------------------------------------------------------------------------------------

CPUEmulator::~CPUEmulator() 
{
}

//-------------------------------------------------------------------------------------------------

void CPUEmulator::ProcessSingleInstruction()
{
	//
	// fetch
	//
	u8 opcode = mem.Read( cpu.PC );
	IncPC(); 
	Tick();

	//
	// decode
	//
	const CommandInfo& command = GetCommandForOpcode( opcode );

	//
	// dispatch
	//
	command.m_functionHandler( );
}

//=================================================================================================
//
//  Instructions accessing the stack
//
//=================================================================================================

namespace StackInstructions
{
	void fn_BRK( )
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
		IncPC();
		Tick();
		
		SetFlags( flag_B ); 
		PushPCH( ); 
		DecS(); 
		Tick();

		PushPCL(); 
		DecS(); 
		Tick();

		PushP( ); 
		DecS(); 
		Tick();

		cpu.SetPCL( mem.Read( cpu.c_IRQ_Lo ) );  
		Tick();

		cpu.SetPCH( mem.Read( cpu.c_IRQ_Hi ) ); 
		LastTick();
	}

	//-------------------------------------------------------------------------------------------------

	void fn_RTI( )
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
		Tick();
		
		IncS(); 
		Tick();

		PullP(); 
		IncS(); 
		Tick();

		PullPCL(); 
		IncS(); 
		Tick();

		PullPCH(); 
		LastTick();
	}

	//-------------------------------------------------------------------------------------------------

	void fn_RTS( )
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
		Tick();
		
		IncS(); 
		Tick();
		
		PullPCL(); 
		IncS(); 
		Tick();
		
		PullPCH(); 
		Tick();
		
		IncPC(); 
		LastTick();
	}
	//-------------------------------------------------------------------------------------------------
	
	//-------------------------------------------------------------------------------------------------
	//
	// PHA, PHP
	//
	//-------------------------------------------------------------------------------------------------
	/*
        #  address R/W description
       --- ------- --- -----------------------------------------------
        1    PC     R  fetch opcode, increment PC
        2    PC     R  read next instruction byte (and throw it away)
        3  $0100,S  W  push register on stack, decrement S
	*/
	void fn_PHA()
	{
		DiscardNextPC();  
		Tick();
		
		PushA(); 
		DecS(); 
		LastTick();
	}
	//-------------------------------------------------------------------------------------------------
	void fn_PHP()
	{
		DiscardNextPC(); 
		Tick();
		
		PushP(); 
		DecS(); 
		LastTick();
	}
	//-------------------------------------------------------------------------------------------------
	//
    // PLA, PLP
	//
	//-------------------------------------------------------------------------------------------------
	/*
        #  address R/W description
       --- ------- --- -----------------------------------------------
        1    PC     R  fetch opcode, increment PC
        2    PC     R  read next instruction byte (and throw it away)
        3  $0100,S  R  increment S
        4  $0100,S  R  pull register from stack
	*/
	void fn_PLA()
	{
		DiscardNextPC(); 
		Tick();
		
		IncS(); 
		Tick();
		
		PullA(); 
		LastTick();
	}
	//-------------------------------------------------------------------------------------------------
	void fn_PLP()
	{
		DiscardNextPC(); 
		Tick();
		
		IncS(); 
		Tick();

		PullP(); 
		LastTick();
	}
	//-------------------------------------------------------------------------------------------------
	//
	// JSR
	//
	//-------------------------------------------------------------------------------------------------
	/*
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
	void fn_JSR()
	{
		u8 lo = mem.Read(cpu.PC);
		IncPC();
		Tick();

		// internal operation?
		DecS();
		Tick();

		PushPCH();
		DecS();
		Tick();

		PushPCL();
		DecS();
		Tick();

		u8 hi = mem.Read(cpu.PC);
		SetPC( lo, hi );
		LastTick();
	}

	//-------------------------------------------------------------------------------------------------
	void RegisterInstructions()
	{
		SetFunctionHandler( mode_imp, BRK, fn_BRK );
		SetFunctionHandler( mode_imp, RTI, fn_RTI );
		SetFunctionHandler( mode_imp, RTS, fn_RTS );
		SetFunctionHandler( mode_imp, PHA, fn_PHA );
		SetFunctionHandler( mode_imp, PHP, fn_PHP );
		SetFunctionHandler( mode_imp, PLA, fn_PLA );
		SetFunctionHandler( mode_imp, PLP, fn_PLP );

		SetFunctionHandler( mode_abs, JSR, fn_JSR );
	}
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
	template <u8(*Operation)(u8)>
	void fn_Implied()
	{
		DiscardNextPC();
		Tick();

		Operation(0);
		LastTick();
	}
	//-------------------------------------------------------------------------------------------------
	void RegisterInstructions()
	{
		SetFunctionHandler( mode_imp, DEX, fn_Implied<op_DEX> );
		SetFunctionHandler( mode_imp, INX, fn_Implied<op_INX> );
		SetFunctionHandler( mode_imp, DEY, fn_Implied<op_DEY> );
		SetFunctionHandler( mode_imp, INY, fn_Implied<op_INY> );
		SetFunctionHandler( mode_imp, ASL, fn_Implied<op_ASL> );
		SetFunctionHandler( mode_imp, ROL, fn_Implied<op_ROL> );
		SetFunctionHandler( mode_imp, LSR, fn_Implied<op_LSR> );
		SetFunctionHandler( mode_imp, ROR, fn_Implied<op_ROR> );
		SetFunctionHandler( mode_imp, TAX, fn_Implied<op_TAX> );
		SetFunctionHandler( mode_imp, TXA, fn_Implied<op_TXA> );
		SetFunctionHandler( mode_imp, TAY, fn_Implied<op_TAY> );
		SetFunctionHandler( mode_imp, TYA, fn_Implied<op_TYA> );
		SetFunctionHandler( mode_imp, TSX, fn_Implied<op_TSX> );
		SetFunctionHandler( mode_imp, TXS, fn_Implied<op_TXS> );
		SetFunctionHandler( mode_imp, CLC, fn_Implied<op_CLC> );
		SetFunctionHandler( mode_imp, SEC, fn_Implied<op_SEC> );
		SetFunctionHandler( mode_imp, CLD, fn_Implied<op_CLD> );
		SetFunctionHandler( mode_imp, SED, fn_Implied<op_SED> );
		SetFunctionHandler( mode_imp, CLI, fn_Implied<op_CLI> );
		SetFunctionHandler( mode_imp, SEI, fn_Implied<op_SEI> );
		SetFunctionHandler( mode_imp, CLV, fn_Implied<op_CLV> );
		SetFunctionHandler( mode_imp, NOP, fn_Implied<op_NOP> );
	}
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
	template <u8(*Operation)(u8)>
	void fn_Immediate()
	{
		u8 value = mem.Read(cpu.PC);
		IncPC();
		Tick();

		Operation(value);
		LastTick();
	}

	//-------------------------------------------------------------------------------------------------
	void RegisterInstructions()
	{
		SetFunctionHandler( mode_imm, ORA, fn_Immediate<op_ORA> );
		SetFunctionHandler( mode_imm, AND, fn_Immediate<op_AND> );
		SetFunctionHandler( mode_imm, EOR, fn_Immediate<op_EOR> );
		SetFunctionHandler( mode_imm, ADC, fn_Immediate<op_ADC> );
		SetFunctionHandler( mode_imm, SBC, fn_Immediate<op_SBC> );
		SetFunctionHandler( mode_imm, CMP, fn_Immediate<op_CMP> );
		SetFunctionHandler( mode_imm, CPX, fn_Immediate<op_CPX> );
		SetFunctionHandler( mode_imm, CPY, fn_Immediate<op_CPY> );
		SetFunctionHandler( mode_imm, LDA, fn_Immediate<op_LDA> );
		SetFunctionHandler( mode_imm, LDX, fn_Immediate<op_LDX> );
		SetFunctionHandler( mode_imm, LDY, fn_Immediate<op_LDY> );
	}
}

//=================================================================================================
//
//	Absolute addressing
//
//=================================================================================================
namespace AbsoluteAddressing
{

    //-------------------------------------------------------------------------------------------------
	//
	// JMP
	//
    //-------------------------------------------------------------------------------------------------
	/*

        #  address R/W description
       --- ------- --- -------------------------------------------------
        1    PC     R  fetch opcode, increment PC
        2    PC     R  fetch low address byte, increment PC
        3    PC     R  copy low address byte to PCL, fetch high address
                       byte to PCH
   */
	void fn_JMP()
	{
		u8 lo = mem.Read(cpu.PC);
		IncPC();
		Tick();

		u8 hi = mem.Read(cpu.PC);
		SetPC( lo, hi );
		LastTick();
	}
    //-------------------------------------------------------------------------------------------------
	//
    // Read instructions (LDA, LDX, LDY, EOR, AND, ORA, ADC, SBC, CMP, BIT,
    //                    LAX, NOP)
	//
	//-------------------------------------------------------------------------------------------------
	/*
        #  address R/W description
       --- ------- --- ------------------------------------------
        1    PC     R  fetch opcode, increment PC
        2    PC     R  fetch low byte of address, increment PC
        3    PC     R  fetch high byte of address, increment PC
        4  address  R  read from effective address
	*/
	template <u8(*Operation)(u8)>
	void fn_ReadInstructions()
	{
		u16 address = 0;
		address = mem.ReadLoByte(cpu.PC, address);
		IncPC();
		Tick();

		address = mem.ReadHiByte(cpu.PC, address);
		IncPC();
		Tick();

		u8 value = mem.Read(address);
		Tick();

		Operation(value);
		LastTick();
	}
	//-------------------------------------------------------------------------------------------------
	//
    // Read-Modify-Write instructions (ASL, LSR, ROL, ROR, INC, DEC,
	//                                 SLO, SRE, RLA, RRA, ISB, DCP)
	//
	//-------------------------------------------------------------------------------------------------
	/*
        #  address R/W description
       --- ------- --- ------------------------------------------
        1    PC     R  fetch opcode, increment PC
        2    PC     R  fetch low byte of address, increment PC
        3    PC     R  fetch high byte of address, increment PC
        4  address  R  read from effective address
        5  address  W  write the value back to effective address,
                       and do the operation on it
        6  address  W  write the new value to effective address
	*/
	template <u8(*Operation)(u8)>
	void fn_ReadModifyWriteInstructions()
	{
		u16 address = 0;
		address = mem.ReadLoByte(cpu.PC, address);
		IncPC();
		Tick();

		address = mem.ReadHiByte(cpu.PC, address);
		IncPC();
		Tick();

		u8 value = mem.Read(address);
		Tick();

		mem.Write(address, value);
		value = Operation(value);
		Tick();

		mem.Write(address, value);
		LastTick();
	}
	//-------------------------------------------------------------------------------------------------
	//
    // Write instructions (STA, STX, STY, SAX)
	//
	//-------------------------------------------------------------------------------------------------
	/*
        #  address R/W description
       --- ------- --- ------------------------------------------
        1    PC     R  fetch opcode, increment PC
        2    PC     R  fetch low byte of address, increment PC
        3    PC     R  fetch high byte of address, increment PC
        4  address  W  write register to effective address
	*/	
	template <u8&(*Register)()>
	void fn_WriteInstructions()
	{
		u16 address = 0;
		address = mem.ReadLoByte(cpu.PC, address);
		IncPC();
		Tick();

		address = mem.ReadHiByte(cpu.PC, address);
		IncPC();
		Tick();

		mem.Write(address, Register());
		LastTick();
	}

	//-------------------------------------------------------------------------------------------------
	void RegisterInstructions()
	{
		SetFunctionHandler( mode_abs, JMP, fn_JMP );

		SetFunctionHandler(mode_abs, LDA, fn_ReadInstructions<op_LDA>);
		SetFunctionHandler(mode_abs, LDX, fn_ReadInstructions<op_LDX>);
		SetFunctionHandler(mode_abs, LDY, fn_ReadInstructions<op_LDY>);
		SetFunctionHandler(mode_abs, EOR, fn_ReadInstructions<op_EOR>);
		SetFunctionHandler(mode_abs, AND, fn_ReadInstructions<op_AND>);
		SetFunctionHandler(mode_abs, ORA, fn_ReadInstructions<op_ORA>);
		SetFunctionHandler(mode_abs, ADC, fn_ReadInstructions<op_ADC>);
		SetFunctionHandler(mode_abs, SBC, fn_ReadInstructions<op_SBC>);
		SetFunctionHandler(mode_abs, CMP, fn_ReadInstructions<op_CMP>);
		SetFunctionHandler(mode_abs, CPX, fn_ReadInstructions<op_CPX>);
		SetFunctionHandler(mode_abs, CPY, fn_ReadInstructions<op_CPY>);
		SetFunctionHandler(mode_abs, BIT, fn_ReadInstructions<op_BIT>);
		// Not implemented: LAX

		SetFunctionHandler(mode_abs, ASL, fn_ReadModifyWriteInstructions<op_ASL>);
		SetFunctionHandler(mode_abs, LSR, fn_ReadModifyWriteInstructions<op_LSR>);
		SetFunctionHandler(mode_abs, ROL, fn_ReadModifyWriteInstructions<op_ROL>);
		SetFunctionHandler(mode_abs, ROR, fn_ReadModifyWriteInstructions<op_ROR>);
		SetFunctionHandler(mode_abs, INC, fn_ReadModifyWriteInstructions<op_INC>);
		SetFunctionHandler(mode_abs, DEC, fn_ReadModifyWriteInstructions<op_DEC>);
		// Not implemented: SLO, SRE, RLA, RRA, ISB, DCP

		SetFunctionHandler(mode_abs, STA, fn_WriteInstructions<reg_cpuA>);
		SetFunctionHandler(mode_abs, STX, fn_WriteInstructions<reg_cpuX>);
		SetFunctionHandler(mode_abs, STY, fn_WriteInstructions<reg_cpuY>);
		// Not implemented: SAX
	}
}
//=================================================================================================
//
//	Zero page addressing
//
//=================================================================================================
namespace ZeroPageAddressing
{
    //-------------------------------------------------------------------------------------------------
	//
	// Read instructions (LDA, LDX, LDY, EOR, AND, ORA, ADC, SBC, CMP, BIT,
    //                    LAX, NOP)
	//
    //-------------------------------------------------------------------------------------------------
	/*
        #  address R/W description
       --- ------- --- ------------------------------------------
        1    PC     R  fetch opcode, increment PC
        2    PC     R  fetch address, increment PC
        3  address  R  read from effective address
	*/
	template <u8&(*Index)(), u8(*Operation)(u8)>
	void fn_ReadInstructions()
	{
		u8 address = 0;
		address = mem.Read(cpu.PC);
		IncPC();
		Tick();

		u8 value = mem.Read(address);
		Tick();

		Operation(value);
		LastTick();
	}

    //-------------------------------------------------------------------------------------------------
	//
	// Read-Modify-Write instructions (ASL, LSR, ROL, ROR, INC, DEC,
	//                                 SLO, SRE, RLA, RRA, ISB, DCP)
	//
	//-------------------------------------------------------------------------------------------------
	/*
        #  address R/W description
       --- ------- --- ------------------------------------------
        1    PC     R  fetch opcode, increment PC
        2    PC     R  fetch address, increment PC
        3  address  R  read from effective address
        4  address  W  write the value back to effective address,
                       and do the operation on it
        5  address  W  write the new value to effective address
	*/
	template <u8(*Operation)(u8)>
	void fn_ReadModifyWriteInstructions()
	{
		u8 address = 0;
		address = mem.Read(cpu.PC);
		IncPC();
		Tick();

		u8 value = mem.Read(address);
		Tick();

		mem.Write(address, value);
		value = Operation(value);
		Tick();

		mem.Write(address, value);
		LastTick();
	}
	//-------------------------------------------------------------------------------------------------
    //
	// Write instructions (STA, STX, STY, SAX)
	//
	//-------------------------------------------------------------------------------------------------
	/*
        #  address R/W description
       --- ------- --- ------------------------------------------
        1    PC     R  fetch opcode, increment PC
        2    PC     R  fetch address, increment PC
        3  address  W  write register to effective address
	*/
	template <u8&(*Index)(), u8&(*Register)()>
	void fn_WriteInstructions()
	{
		u8 address = 0;
		address = mem.Read(cpu.PC);
		IncPC();
		Tick();

		mem.Write(address, Register());
		LastTick();
	}

	//-------------------------------------------------------------------------------------------------

	void RegisterInstructions()
	{
		SetFunctionHandler(mode_zp, LDA, fn_ReadInstructions<reg_cpuX, op_LDA>);
		SetFunctionHandler(mode_zp, LDX, fn_ReadInstructions<reg_cpuX, op_LDX>);
		SetFunctionHandler(mode_zp, LDY, fn_ReadInstructions<reg_cpuX, op_LDY>);
		SetFunctionHandler(mode_zp, EOR, fn_ReadInstructions<reg_cpuX, op_EOR>);
		SetFunctionHandler(mode_zp, AND, fn_ReadInstructions<reg_cpuX, op_AND>);
		SetFunctionHandler(mode_zp, ORA, fn_ReadInstructions<reg_cpuX, op_ORA>);
		SetFunctionHandler(mode_zp, ADC, fn_ReadInstructions<reg_cpuX, op_ADC>);
		SetFunctionHandler(mode_zp, SBC, fn_ReadInstructions<reg_cpuX, op_SBC>);
		SetFunctionHandler(mode_zp, CMP, fn_ReadInstructions<reg_cpuX, op_CMP>);
		SetFunctionHandler(mode_zp, CPX, fn_ReadInstructions<reg_cpuX, op_CPX>);
		SetFunctionHandler(mode_zp, CPY, fn_ReadInstructions<reg_cpuX, op_CPY>);
		SetFunctionHandler(mode_zp, BIT, fn_ReadInstructions<reg_cpuX, op_BIT>);
		// Not implemented: LAX

		SetFunctionHandler(mode_zp, ASL, fn_ReadModifyWriteInstructions<op_ASL>);
		SetFunctionHandler(mode_zp, LSR, fn_ReadModifyWriteInstructions<op_LSR>);
		SetFunctionHandler(mode_zp, ROL, fn_ReadModifyWriteInstructions<op_ROL>);
		SetFunctionHandler(mode_zp, ROR, fn_ReadModifyWriteInstructions<op_ROR>);
		SetFunctionHandler(mode_zp, INC, fn_ReadModifyWriteInstructions<op_INC>);
		SetFunctionHandler(mode_zp, DEC, fn_ReadModifyWriteInstructions<op_DEC>);
		// Not implemented: SLO, SRE, RLA, RRA, ISB, DCP

		SetFunctionHandler(mode_zp, STA, fn_WriteInstructions<reg_cpuX, reg_cpuA>);
		SetFunctionHandler(mode_zp, STX, fn_WriteInstructions<reg_cpuX, reg_cpuX>);
		SetFunctionHandler(mode_zp, STY, fn_WriteInstructions<reg_cpuX, reg_cpuY>);
		// Not implemented: SAX
	}

}
//=================================================================================================
//
//	Zero page indexed addressing
//
//=================================================================================================

namespace ZeroPageIndexedAddressing
{
	//-------------------------------------------------------------------------------------------------
	//
    //  Read instructions (LDA, LDX, LDY, EOR, AND, ORA, ADC, SBC, CMP, BIT,
    //                     LAX, NOP)
	//
	//-------------------------------------------------------------------------------------------------
	/*
        #   address  R/W description
       --- --------- --- ------------------------------------------
        1     PC      R  fetch opcode, increment PC
        2     PC      R  fetch address, increment PC
        3   address   R  read from address, add index register to it
        4  address+I* R  read from effective address

       Notes: I denotes either index register (X or Y).

              * The high byte of the effective address is always zero,
                i.e. page boundary crossings are not handled.
	*/
	template <u8&(*Index)(),u8(*Operation)(u8)>
	void fn_ReadInstructions( )
	{
		u8 address = 0;
		address = mem.Read( cpu.PC );
		IncPC(); 
		Tick();

		mem.Read(address);
		address += Index();
		Tick();

		u8 value = mem.Read(address);
		Tick();

		Operation(value);
		LastTick();
	}

	//-------------------------------------------------------------------------------------------------
	//
	// Read-Modify-Write instructions (ASL, LSR, ROL, ROR, INC, DEC,
    //                                 SLO, SRE, RLA, RRA, ISB, DCP)
	//
	//-------------------------------------------------------------------------------------------------
	/*
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
	 */
	template <u8(*Operation)(u8)>
	void fn_ReadModifyWriteInstructions( )
	{
		u8 address = 0;
		address = mem.Read( cpu.PC );
		IncPC(); 
		Tick();

		mem.Read(address);
		address += cpu.X;
		Tick();

		u8 value = mem.Read(address);
		Tick();

		mem.Write(address,value);
		value = Operation(value);
		Tick();

		mem.Write(address,value);
		LastTick();
	}
	//-------------------------------------------------------------------------------------------------	
	//
    // Write instructions (STA, STX, STY, SAX)
	//
	//-------------------------------------------------------------------------------------------------	
	/*
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
	template <u8&(*Index)(),u8&(*Register)()>
	void fn_WriteInstructions( )
	{
		u8 address = 0;
		address = mem.Read( cpu.PC );
		IncPC(); 
		Tick();

		mem.Read(address);
		address += Index();
		Tick();

		mem.Write( address, Register() );
		LastTick();
	}

	//-------------------------------------------------------------------------------------------------

	void RegisterInstructions()
	{
		SetFunctionHandler( mode_zpx, LDA, fn_ReadInstructions<reg_cpuX,op_LDA> );
		SetFunctionHandler( mode_zpx, LDY, fn_ReadInstructions<reg_cpuX,op_LDY> );
		SetFunctionHandler( mode_zpx, EOR, fn_ReadInstructions<reg_cpuX,op_EOR> );
		SetFunctionHandler( mode_zpx, AND, fn_ReadInstructions<reg_cpuX,op_AND> );
		SetFunctionHandler( mode_zpx, ORA, fn_ReadInstructions<reg_cpuX,op_ORA> );
		SetFunctionHandler( mode_zpx, ADC, fn_ReadInstructions<reg_cpuX,op_ADC> );
		SetFunctionHandler( mode_zpx, SBC, fn_ReadInstructions<reg_cpuX,op_SBC> );
		SetFunctionHandler( mode_zpx, CMP, fn_ReadInstructions<reg_cpuX,op_CMP> );
		// Not implemented: LAX

		SetFunctionHandler( mode_zpy, LDX, fn_ReadInstructions<reg_cpuY,op_LDX> );
		// Not implemented: LAX

		SetFunctionHandler( mode_zpx, ASL, fn_ReadModifyWriteInstructions<op_ASL> );
		SetFunctionHandler( mode_zpx, LSR, fn_ReadModifyWriteInstructions<op_LSR> );
		SetFunctionHandler( mode_zpx, ROL, fn_ReadModifyWriteInstructions<op_ROL> );
		SetFunctionHandler( mode_zpx, ROR, fn_ReadModifyWriteInstructions<op_ROR> );
		SetFunctionHandler( mode_zpx, INC, fn_ReadModifyWriteInstructions<op_INC> );
		SetFunctionHandler( mode_zpx, DEC, fn_ReadModifyWriteInstructions<op_DEC> );
		// Not implemented: SLO, SRE, RLA, RRA, ISB, DCP

		SetFunctionHandler( mode_zpx, STA, fn_WriteInstructions<reg_cpuX,reg_cpuA> );
		SetFunctionHandler( mode_zpx, STY, fn_WriteInstructions<reg_cpuX,reg_cpuY> );
		// Not implemented: SAX
		
		SetFunctionHandler( mode_zpy, STX, fn_WriteInstructions<reg_cpuY,reg_cpuX> );
		// Not implemented: SAX
	}
}
//=================================================================================================
//
// Absolute indexed addressing
//
//=================================================================================================

namespace AbsoluteIndexedAddressing
{
	//-------------------------------------------------------------------------------------------------
	//
	// Read instructions (LDA, LDX, LDY, EOR, AND, ORA, ADC, SBC, CMP, BIT,
	//					  LAX, LAE, SHS, NOP)
	//
	//-------------------------------------------------------------------------------------------------
	/*
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
	template <u8&(*Index)(),u8(*Operation)(u8)>
	void fn_ReadInstructions( )
	{
		u16 address = 0;
		mem.ReadLoByte( cpu.PC, address );
		IncPC(); 
		Tick();

		mem.ReadHiByte( cpu.PC, address );
		u16 temp_address = ( address & 0xffffff00 ) + ( ( address + Index() ) & 0xff );
		IncPC(); 
		Tick();

		u8 value = mem.Read( temp_address );
		address = address + Index(); 
		Tick();

		if ( address != temp_address )
		{ 
			value = mem.Read( address );
			Tick();
		}
		Operation(value);
		LastTick();
	}

	//-------------------------------------------------------------------------------------------------
	//
	// Read-Modify-Write instructions (ASL, LSR, ROL, ROR, INC, DEC,
	//                                 SLO, SRE, RLA, RRA, ISB, DCP)
	//
	//-------------------------------------------------------------------------------------------------
	template <u8(*Operation)(u8)>
	void fn_ReadModifyWriteInstructions( )
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
		u16 temp_address = ( address & 0xffffff00 ) + ( ( address + cpu.X ) & 0xff );
		IncPC(); 
		Tick();

		u8 value = mem.Read( temp_address );
		address = address + cpu.X;
		Tick();

		value = mem.Read( temp_address );
		Tick();

		mem.Write( address, value );
		Operation(value);
		Tick();

		mem.Write( address, value );
		LastTick();
	}

	//-------------------------------------------------------------------------------------------------
	template <u8&(*Index)(),u8&(*Register)()>
	void fn_WriteInstructions( )
	{
		//-------------------------------------------------------------------------------------------------
		//	
		// Write instructions (STA, STX, STY, SHA, SHX, SHY)
		//
		//-------------------------------------------------------------------------------------------------
		/*
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

		u16 address = 0;
		mem.ReadLoByte( cpu.PC, address );
		IncPC(); 
		Tick();

		mem.ReadHiByte( cpu.PC, address );
		u16 temp_address = ( address & 0xffffff00 ) + ( ( address + Index() ) & 0xff );
		IncPC(); 
		Tick();

		u8 value = mem.Read( temp_address );
		address = address + Index(); 
		Tick();

		mem.Write( address, Register() );
		LastTick();
	}
	//-------------------------------------------------------------------------------------------------
	void RegisterInstructions()
	{
		SetFunctionHandler( mode_abx, LDA, fn_ReadInstructions<reg_cpuX,op_LDA> );
		SetFunctionHandler( mode_abx, LDY, fn_ReadInstructions<reg_cpuX,op_LDY> );
		SetFunctionHandler( mode_abx, EOR, fn_ReadInstructions<reg_cpuX,op_EOR> );
		SetFunctionHandler( mode_abx, AND, fn_ReadInstructions<reg_cpuX,op_AND> );
		SetFunctionHandler( mode_abx, ORA, fn_ReadInstructions<reg_cpuX,op_ORA> );
		SetFunctionHandler( mode_abx, ADC, fn_ReadInstructions<reg_cpuX,op_ADC> );
		SetFunctionHandler( mode_abx, SBC, fn_ReadInstructions<reg_cpuX,op_SBC> );
		SetFunctionHandler( mode_abx, CMP, fn_ReadInstructions<reg_cpuX,op_CMP> );
		// Not implemented: LAX, LAE, SHS
		
		SetFunctionHandler( mode_aby, LDA, fn_ReadInstructions<reg_cpuY,op_LDA> );
		SetFunctionHandler( mode_aby, LDX, fn_ReadInstructions<reg_cpuY,op_LDX> );
		SetFunctionHandler( mode_aby, EOR, fn_ReadInstructions<reg_cpuY,op_EOR> );
		SetFunctionHandler( mode_aby, AND, fn_ReadInstructions<reg_cpuY,op_AND> );
		SetFunctionHandler( mode_aby, ORA, fn_ReadInstructions<reg_cpuY,op_ORA> );
		SetFunctionHandler( mode_aby, ADC, fn_ReadInstructions<reg_cpuY,op_ADC> );
		SetFunctionHandler( mode_aby, SBC, fn_ReadInstructions<reg_cpuY,op_SBC> );
		SetFunctionHandler( mode_aby, CMP, fn_ReadInstructions<reg_cpuY,op_CMP> );
		// Not implemented: LAX, LAE, SHS

		SetFunctionHandler( mode_abx, ASL, fn_ReadModifyWriteInstructions<op_ASL> );
		SetFunctionHandler( mode_abx, LSR, fn_ReadModifyWriteInstructions<op_LSR> );
		SetFunctionHandler( mode_abx, ROL, fn_ReadModifyWriteInstructions<op_ROL> );
		SetFunctionHandler( mode_abx, ROR, fn_ReadModifyWriteInstructions<op_ROR> );
		SetFunctionHandler( mode_abx, INC, fn_ReadModifyWriteInstructions<op_INC> );
		SetFunctionHandler( mode_abx, DEC, fn_ReadModifyWriteInstructions<op_DEC> );
		// Not implemented: SLO, SRE, RLA, RRA, ISB, DCP

		SetFunctionHandler( mode_abx, STA, fn_WriteInstructions<reg_cpuX,reg_cpuA> );
		// Not implemented: SHA, SHX, SHY
		
		SetFunctionHandler( mode_aby, STA, fn_WriteInstructions<reg_cpuY,reg_cpuA> );
		// Not implemented: SHA, SHX, SHY

	}
	//-------------------------------------------------------------------------------------------------

};
//=================================================================================================
//
//	Relative addressing
//
//=================================================================================================

namespace RelativeAddressing
{
	//-------------------------------------------------------------------------------------------------
	//
	// Relative addressing (BCC, BCS, BNE, BEQ, BPL, BMI, BVC, BVS)
	//
	//-------------------------------------------------------------------------------------------------
	/*
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
	template <bool(*CheckBranch)()>
	void fn_BranchInstructions( )
	{
		u8 operand = FetchOperand();
		Tick();

		if ( CheckBranch() )
		{
			u16 newPC = cpu.PC + operand;
			cpu.SetPCL( operand );
			
			if ( newPC != cpu.PC )
			{
				Tick();
				cpu.PC = newPC;
			}
		}
		LastTick();
	}
	
	//-------------------------------------------------------------------------------------------------
	void RegisterInstructions()
	{
		SetFunctionHandler( mode_rel, BCC, fn_BranchInstructions<op_BCC> );
		SetFunctionHandler( mode_rel, BCS, fn_BranchInstructions<op_BCS> );
		SetFunctionHandler( mode_rel, BNE, fn_BranchInstructions<op_BNE> );
		SetFunctionHandler( mode_rel, BEQ, fn_BranchInstructions<op_BEQ> );
		SetFunctionHandler( mode_rel, BPL, fn_BranchInstructions<op_BPL> );
		SetFunctionHandler( mode_rel, BMI, fn_BranchInstructions<op_BMI> );
		SetFunctionHandler( mode_rel, BVC, fn_BranchInstructions<op_BVC> );
		SetFunctionHandler( mode_rel, BVS, fn_BranchInstructions<op_BVS> );
	}
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
	template <u8(*Operation)(u8)>
	void fn_ReadInstructions( )
	{
		u8 pointer = FetchPointer();
		Tick();

		mem.Read( pointer );
		pointer += cpu.X;
		Tick();

		u16 address = Get16BitAddressFromPointer( pointer );

		u8 value = mem.Read( address );
		Tick();

		Operation( value );
		LastTick();
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
	template <u8(*Operation)(u8)>
	void fn_ReadModifyWriteInstructions( )
	{
		u8 pointer = FetchPointer();
		Tick();

		mem.Read( pointer );
		pointer += cpu.X;
		Tick();

		u16 address = Get16BitAddressFromPointer( pointer );

		u8 value = mem.Read( address );
		Tick();

		mem.Write( address, value );
		value = Operation( value );
		Tick();

		mem.Write( address, value );
		LastTick();
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
	template <u8&(*Register)()>
	void fn_WriteInstructions( )
	{
		u8 pointer = FetchPointer();
		Tick();

		mem.Read( pointer );
		pointer += cpu.X;
		Tick();

		u16 address = Get16BitAddressFromPointer( pointer );

		u8 value = Register();
		mem.Write( address, value );
		LastTick();
	}

	//-------------------------------------------------------------------------------------------------
	void RegisterInstructions()
	{
		SetFunctionHandler( mode_izx, LDA, fn_ReadInstructions<op_LDA> );
		SetFunctionHandler( mode_izx, ORA, fn_ReadInstructions<op_ORA> );
		SetFunctionHandler( mode_izx, EOR, fn_ReadInstructions<op_EOR> );
		SetFunctionHandler( mode_izx, AND, fn_ReadInstructions<op_AND> );
		SetFunctionHandler( mode_izx, ADC, fn_ReadInstructions<op_ADC> );
		SetFunctionHandler( mode_izx, CMP, fn_ReadInstructions<op_CMP> );
		SetFunctionHandler( mode_izx, SBC, fn_ReadInstructions<op_SBC> );
		// Not implemented: LAX

		// Read-Modify-Write instructions 
		// Not implemented : (SLO, SRE, RLA, RRA, ISB, DCP)

		SetFunctionHandler( mode_izx, STA, fn_WriteInstructions<reg_cpuA> );
		// Not implemented: SAX
	}

};

//=================================================================================================
//
//	Indirect indexed addressing
//
//=================================================================================================

namespace IndirectIndexedAddressing
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
	template <u8(*Operation)(u8)>
	void fn_ReadInstructions( )
	{
		u8 pointer = FetchPointer();
		Tick();

		mem.Read( pointer );
		Tick();

		u16 address = Get16BitAddressFromPointer( pointer );

		u16 temp_address = ( address & 0xffffff00 ) + ( ( address + cpu.Y ) & 0xff );
		address += cpu.Y;

		Tick();

		u8 value = mem.Read( temp_address );
		if ( temp_address != address )
		{
			u8 value = mem.Read( address );
			Tick();
		}
		Operation( value );
		LastTick();
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
	template <u8(*Operation)(u8)>
	void fn_ReadModifyWriteInstructions( )
	{
		u8 pointer = FetchPointer();
		Tick();

		mem.Read( pointer );
		Tick();
		u16 address = Get16BitAddressFromPointer( pointer );

		u16 temp_address = ( address & 0xffffff00 ) + ( ( address + Index() ) & 0xff );
		address += cpu.Y;

		Tick();

		u8 value = mem.Read( temp_address );
		if ( temp_address != address )
		{
			u8 value = mem.Read( address );
			Tick();
		}
		mem.Write( address, value );
		value = Operation( value );
		Tick();

		mem.Write( address, value );
		LastTick();
	}
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
	template <u8&(*Register)()>
	void fn_WriteInstructions( )
	{
		u8 pointer = FetchPointer();
		Tick();

		mem.Read( pointer );
		Tick();
		u16 address = Get16BitAddressFromPointer( pointer );

		u16 temp_address = ( address & 0xffffff00 ) + ( ( address + cpu.Y ) & 0xff );
		address += cpu.Y;

		Tick();

		u8 value = mem.Read( temp_address );
		if ( temp_address != address )
		{
			u8 value = mem.Read( address );
			Tick();
		}
		mem.Write( address, Register() );
		LastTick();
	}

	//-------------------------------------------------------------------------------------------------
	void RegisterInstructions()
	{
		SetFunctionHandler( mode_izy, LDA, fn_ReadInstructions<op_LDA> );
		SetFunctionHandler( mode_izy, ORA, fn_ReadInstructions<op_ORA> );
		SetFunctionHandler( mode_izy, EOR, fn_ReadInstructions<op_EOR> );
		SetFunctionHandler( mode_izy, AND, fn_ReadInstructions<op_AND> );
		SetFunctionHandler( mode_izy, ADC, fn_ReadInstructions<op_ADC> );
		SetFunctionHandler( mode_izy, CMP, fn_ReadInstructions<op_CMP> );
		SetFunctionHandler( mode_izy, SBC, fn_ReadInstructions<op_SBC> );
		// Not implemented: LAX

		// Read-Modify-Write instructions 
		// Not implemented : (SLO, SRE, RLA, RRA, ISB, DCP)

		SetFunctionHandler( mode_izy, STA, fn_WriteInstructions<reg_cpuA> );
		// Not implemented: SAX
	}
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

	void fn_JMP()
	{
		u8 lo = mem.Read(cpu.PC);
		IncPC();
		Tick();

		u8 hi = mem.Read(cpu.PC);
		SetPC( lo, hi );
		Tick();

		lo = mem.Read(cpu.PC);
		IncPC();
		Tick();

		hi = mem.Read(cpu.PC);
		SetPC( lo, hi );
		LastTick();
	}

	//-------------------------------------------------------------------------------------------------
	void RegisterInstructions()
	{
		SetFunctionHandler( mode_ind, JMP, fn_JMP );
	}
};

//=================================================================================================

void RegisterInstructionHandlers()
{
	StackInstructions				::RegisterInstructions();
	AccumulatorOrImpliedAddressing	::RegisterInstructions();
	ImmediateAddressing				::RegisterInstructions();
	AbsoluteAddressing				::RegisterInstructions();
	ZeroPageAddressing				::RegisterInstructions();
	ZeroPageIndexedAddressing		::RegisterInstructions();
	AbsoluteIndexedAddressing		::RegisterInstructions();
	RelativeAddressing				::RegisterInstructions();
	AbsoluteIndirectAddressing		::RegisterInstructions();
	IndirectIndexedAddressing		::RegisterInstructions();
	IndexedIndirectAddressing		::RegisterInstructions();
}
//=================================================================================================
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
