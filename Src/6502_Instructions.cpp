#include "stdafx.h"

//-------------------------------------------------------------------------------------------------

static inline u8 FetchPointer( )
{
	//    PC       R  fetch pointer address, increment PC
	u8 location = mem.Read( cpu.reg.PC );
	cpu.IncPC();
	return location;
}
//-------------------------------------------------------------------------------------------------

static inline u8 FetchOperand( )
{
	//    PC       R  fetch operand, increment PC
	u8 operand = mem.Read( cpu.reg.PC );
	cpu.IncPC();
	return operand;
}
//-------------------------------------------------------------------------------------------------

static inline u16 Get16BitAddressFromPointer( u8 pointer )
{
	// pointer    R  read from the address
	u16 address = 0;
	mem.ReadLoByte( pointer, address );
	cpu.Tick();

	mem.ReadHiByte( ( pointer + 1 )/* & 0xff*/, address );
	cpu.Tick();

	return address;
	//not sure
}
//-------------------------------------------------------------------------------------------------

static inline void DiscardNextPC( )
{
	//    PC     R  read next instruction byte (and throw it away),
	mem.Read(cpu.reg.PC);
}

//-------------------------------------------------------------------------------------------------
static inline void PushP_BRK( )
{
	//  $0100,S  W  push P on stack, decrement S
	mem.Write( cpu.StackAddress( ), cpu.reg.P | flag_B | flag_unused );
}
//-------------------------------------------------------------------------------------------------
static inline void PushP_noBRK( )
{
	//  $0100,S  W  push P on stack, decrement S
	mem.Write( cpu.StackAddress( ), (cpu.reg.P & (~flag_B)) | flag_unused );
}

//-------------------------------------------------------------------------------------------------
static inline void PushA()
{
	//  $0100,S  W  push A on stack, decrement S
	mem.Write(cpu.StackAddress(), cpu.reg.A);
}
//-------------------------------------------------------------------------------------------------
static inline void PushPCH( )
{
	//  $0100,S  W  push PCH on stack (with B flag set), decrement S
	mem.WriteHiByte( cpu.StackAddress( ), cpu.reg.PC );
}

//-------------------------------------------------------------------------------------------------
static inline void PushPCL( )
{
	//  $0100,S  W  push PCL on stack, decrement S
	mem.WriteLoByte( cpu.StackAddress( ), cpu.reg.PC );
}

//-------------------------------------------------------------------------------------------------
static inline void PullP( )
{
    //  $0100,S  R  pull P from stack, increment S
	cpu.reg.P = mem.Read( cpu.StackAddress( ) );
	//assert( cpu.reg.GetFlag( flag_B ) == 0 );
	cpu.reg.SetFlag( flag_B, 0 );
	// NOTE: masking out BRK and unused flags to pass tests :(
	//cpu.reg.P = ( cpu.reg.P & 0x30 ) | ( mem.Read( cpu.StackAddress( ) ) & 0xcf );
}

//-------------------------------------------------------------------------------------------------
static inline void PullA()
{
	//  $0100,S  R  pull A from stack, increment S
	cpu.reg.A = mem.Read(cpu.StackAddress());
	cpu.SetZN(cpu.reg.A);
}

//-------------------------------------------------------------------------------------------------
static inline void PullPCL( )
{
    //  $0100,S  R  pull PCL from stack, increment S
	cpu.reg.PC = mem.ReadLoByte( cpu.StackAddress( ), cpu.reg.PC );
}
//-------------------------------------------------------------------------------------------------
static inline void PullPCH( )
{
    //  $0100,S  R  pull PCH from stack
	cpu.reg.PC = mem.ReadHiByte( cpu.StackAddress( ), cpu.reg.PC );
}


//-------------------------------------------------------------------------------------------------
static inline void SetPC( u8 lo, u8 hi )
{
	//   $FFFF   R  fetch PCH
	cpu.reg.PC = lo | ( u16( hi ) << 8 );
}

//-------------------------------------------------------------------------------------------------

void SetFlags( u8 flags )
{
	cpu.reg.P |= flags;
}
//-------------------------------------------------------------------------------------------------

void ClearFlags( u8 flags )
{
	cpu.reg.P &= ~flags;
}

//=================================================================================================
//
// Operations
//
// Flags set as per http://www.obelisk.me.uk/6502/reference.html
//
//=================================================================================================

inline u8& reg_cpuX()
{
	return cpu.reg.X;
}
//-------------------------------------------------------------------------------------------------
inline u8& reg_cpuY()
{
	return cpu.reg.Y;
}
//-------------------------------------------------------------------------------------------------
inline u8& reg_cpuA()
{
	return cpu.reg.A;
}
//-------------------------------------------------------------------------------------------------
inline u16& reg_cpuPC()
{
	return cpu.reg.PC;
}

//-------------------------------------------------------------------------------------------------

inline u8 op_ADC(u8 val)
{
	//
	// BCD version adapted from beebem implementation
	//
	if ( cpu.GetFlag( flag_D ) )
	{
		int TmpResult = cpu.reg.A + val + cpu.GetFlag( flag_C );
		int TmpCarry = 0;

		int nLowDec = ( cpu.reg.A & 0xf ) + ( val & 0xf ) + cpu.GetFlag( flag_C );
		if ( nLowDec > 9 ) 
		{
			nLowDec += 6;
			nLowDec &= 0xf;
			TmpCarry = 0x10;
		}
		int nHighDec = ( cpu.reg.A & 0xf0 ) + ( val & 0xf0 ) + TmpCarry;

		cpu.SetFlag( flag_V, ( ( nHighDec ^ cpu.reg.A ) & 128 ) && ( !( ( cpu.reg.A ^ val ) & 128 ) ) );
		if ( nHighDec > 0x90 ) 
		{
			nHighDec += 0x60;
			nHighDec &= 0xf0;
			cpu.SetFlag( flag_C, 1 );
		}
		else
		{
			cpu.SetFlag( flag_C, 0 );
		}
		cpu.reg.A = nHighDec | nLowDec;
		cpu.SetZN(cpu.reg.A);
	}
	else
	{
		u16 newA = cpu.reg.A;
		newA += val;
		newA += cpu.GetFlag(flag_C);
		cpu.SetFlag(flag_C, (newA&0x100)?1:0);
		cpu.SetFlag(flag_V,(newA^cpu.reg.A)&(val^newA)&0x80); // from http://www.righto.com/2012/12/the-6502-overflow-flag-explained.html
		cpu.reg.A = newA & 0xff;
		cpu.SetZN(cpu.reg.A);
	}
	return cpu.reg.A;
}

//-------------------------------------------------------------------------------------------------
inline u8 op_AND(u8 val)
{
	cpu.reg.A &= val;
	cpu.SetZN(cpu.reg.A);
	return cpu.reg.A;
}
//-------------------------------------------------------------------------------------------------
inline u8 op_ASL(u8 val)
{
	cpu.SetFlag(flag_C, (val & 0x80));
	val <<= 1;
	cpu.SetZN(val);
	return val;
}
//-------------------------------------------------------------------------------------------------
inline bool op_BCC()
{
	return cpu.GetFlag(flag_C)==0;
}
//-------------------------------------------------------------------------------------------------
inline bool op_BCS()
{
	return cpu.GetFlag(flag_C)!=0;

}
//-------------------------------------------------------------------------------------------------
inline bool op_BEQ()
{
	return cpu.GetFlag(flag_Z)!=0;
}
//-------------------------------------------------------------------------------------------------
inline u8 op_BIT(u8 val)
{
	cpu.SetFlag(flag_Z,(cpu.reg.A&val)==0);
	cpu.SetFlag(flag_V, (val & 0x40));
	cpu.SetFlag(flag_N, (val & 0x80));
	return 0;
}
//-------------------------------------------------------------------------------------------------
inline bool op_BMI()
{
	return cpu.GetFlag(flag_N)!=0;
}
//-------------------------------------------------------------------------------------------------
inline bool op_BNE()
{
	return cpu.GetFlag(flag_Z)==0;
}
//-------------------------------------------------------------------------------------------------
inline bool op_BPL()
{
	return cpu.GetFlag(flag_N)==0;
}
//-------------------------------------------------------------------------------------------------
inline bool op_BVC()
{
	return cpu.GetFlag(flag_V)==0;
}
//-------------------------------------------------------------------------------------------------
inline bool op_BVS()
{
	return cpu.GetFlag(flag_V)!=0;
}
//-------------------------------------------------------------------------------------------------
inline u8 op_CLC(u8 val)
{
	cpu.SetFlag( flag_C, 0 );
	return 0;
}
//-------------------------------------------------------------------------------------------------
inline u8 op_CLD(u8 val)
{
	cpu.SetFlag( flag_D, 0 );
	return 0;
}
//-------------------------------------------------------------------------------------------------
inline u8 op_CLI(u8 val)
{
	cpu.SetFlag( flag_I, 0 );
	return 0;
}
//-------------------------------------------------------------------------------------------------
inline u8 op_CLV(u8 val)
{
	cpu.SetFlag( flag_V, 0 );
	return 0;
}
//-------------------------------------------------------------------------------------------------
inline u8 op_CMP(u8 val)
{
	cpu.SetFlag(flag_C,cpu.reg.A>=val);
	cpu.SetZN( cpu.reg.A - val );
	return 0;
}
//-------------------------------------------------------------------------------------------------
inline u8 op_CPX(u8 val)
{
	cpu.SetFlag(flag_C,cpu.reg.X>=val);
	cpu.SetZN( cpu.reg.X - val );
	return 0;
}
//-------------------------------------------------------------------------------------------------
inline u8 op_CPY(u8 val)
{
	cpu.SetFlag(flag_C,cpu.reg.Y>=val);
	cpu.SetZN( cpu.reg.Y - val );
	return 0;
}
//-------------------------------------------------------------------------------------------------
inline u8 op_DEC(u8 val)
{
	val--;
	cpu.SetZN(val);
	return val;
}
//-------------------------------------------------------------------------------------------------
inline u8 op_DEX(u8 val)
{
	cpu.reg.X--;
	cpu.SetZN(cpu.reg.X);
	return cpu.reg.X;
}
//-------------------------------------------------------------------------------------------------
inline u8 op_DEY(u8 val)
{
	cpu.reg.Y--;
	cpu.SetZN(cpu.reg.Y);
	return cpu.reg.Y;
}
//-------------------------------------------------------------------------------------------------
inline u8 op_EOR(u8 val)
{
	cpu.reg.A ^= val;
	cpu.SetZN(cpu.reg.A);
	return cpu.reg.A;
}
//-------------------------------------------------------------------------------------------------
inline u8 op_INC(u8 val)
{
	val ++;
	cpu.SetZN(val);
	return val;
}
//-------------------------------------------------------------------------------------------------
inline u8 op_INX(u8 val)
{
	cpu.reg.X++;
	cpu.SetZN(cpu.reg.X);
	return cpu.reg.X;
}
//-------------------------------------------------------------------------------------------------
inline u8 op_INY(u8 val)
{
	cpu.reg.Y++;
	cpu.SetZN(cpu.reg.Y);
	return cpu.reg.Y;
}
//-------------------------------------------------------------------------------------------------
inline u8 op_LDA(u8 val)
{
	cpu.reg.A = val;
	cpu.SetZN(val);
	return val;
}
//-------------------------------------------------------------------------------------------------
inline u8 op_LDX(u8 val)
{
	cpu.reg.X = val;
	cpu.SetZN(val);
	return val;
}
//-------------------------------------------------------------------------------------------------
inline u8 op_LDY(u8 val)
{
	cpu.reg.Y = val;
	cpu.SetZN(val);
	return val;
}
//-------------------------------------------------------------------------------------------------
inline u8 op_LSR(u8 val)
{
	cpu.SetFlag(flag_C, (val & 0x1));
	val >>= 1;
	cpu.SetZN(val);
	return val;
}
//-------------------------------------------------------------------------------------------------
inline u8 op_NULL(u8 val)
{
	return val;
}
//-------------------------------------------------------------------------------------------------
inline u8 op_NOP(u8 val)
{
	return val;
}
//-------------------------------------------------------------------------------------------------
inline u8 op_ORA(u8 val)
{
	cpu.reg.A |= val;
	cpu.SetZN(cpu.reg.A);
	return cpu.reg.A;
}
//-------------------------------------------------------------------------------------------------
inline u8 op_ROL(u8 val)
{
	u8 C = cpu.GetFlag(flag_C);
	cpu.SetFlag(flag_C, (val & 0x80));
	val <<= 1;
	val |= C;
	cpu.SetZN( val );
	return val;
}
//-------------------------------------------------------------------------------------------------
inline u8 op_ROR(u8 val)
{
	u8 C = cpu.GetFlag(flag_C);
	cpu.SetFlag(flag_C, (val & 0x1));
	val >>= 1;
	val |= C<<7;
	cpu.SetZN(val);
	return val;
}
//-------------------------------------------------------------------------------------------------
inline u8 op_SBC(u8 val)
{
	if ( cpu.GetFlag( flag_D ) )
	{
		//
		// BCD version adapted from beebem implementation
		//
		int TmpCarry = 0;
		u8 nhn = ( cpu.reg.A >> 4 ) & 15;
		u8 nln = cpu.reg.A & 15;

		// Z flag determined from 2's compl result, not BCD result!
		int TmpResult = cpu.reg.A - val - ( 1 - cpu.GetFlag( flag_C ) );
		int ZFlag = ( ( TmpResult & 0xff ) == 0 );

		int ohn = val & 0xf0;
		int oln = val & 0xf;
		if ( ( oln > 9 ) && ( ( cpu.reg.A & 15 ) < 10 ) )
		{
			oln -= 10;
			ohn += 0x10;
		}
		// promote the lower nibble to the next ten, and increase the higher nibble
		int ln = ( cpu.reg.A & 0xf ) - oln - ( 1 - cpu.GetFlag( flag_C ) );
		if ( ln < 0 )
		{
			if ( ( cpu.reg.A & 15 ) < 10 )
			{
				ln -= 6;
			}
			ln &= 0xf;
			TmpCarry = 0x10;
		}
		int hn = ( cpu.reg.A & 0xf0 ) - ohn - TmpCarry;
		// N and V flags are determined before high nibble is adjusted.
		//   NOTE: V is not always correct 
		int TmpResultV = ( signed char ) cpu.reg.A - ( signed char ) val - ( 1 - cpu.GetFlag( flag_C ) );
		if ( ( TmpResultV < -128 ) || ( TmpResultV > 127 ) )
		{
			cpu.SetFlag( flag_V, 1 );
		}
		else
		{
			cpu.SetFlag( flag_V, 0 );
		}

		if ( hn < 0 ) 
		{
			hn -= 0x60;
			hn &= 0xf0;
		}
		cpu.reg.A = hn | ln;
		if ( cpu.reg.A == 0 )
		{
			ZFlag = 1;
		}
		cpu.SetFlag( flag_Z, ZFlag );
		cpu.SetFlag( flag_N, ( hn & 128 ) );
		cpu.SetFlag( flag_C, ( TmpResult & 256 ) == 0 );
		return val;
	}
	else
	{
		return op_ADC( ~val );
	}
}
//-------------------------------------------------------------------------------------------------
inline u8 op_SEC(u8 val)
{
	cpu.SetFlag( flag_C, 1 );
	return 1;
}
//-------------------------------------------------------------------------------------------------
inline u8 op_SED(u8 val)
{
	cpu.SetFlag( flag_D, 1 );
	return 1;
}
//-------------------------------------------------------------------------------------------------
inline u8 op_SEI(u8 val)
{
	cpu.SetFlag( flag_I, 1 );
	return 1;
}
//-------------------------------------------------------------------------------------------------
inline u8 op_STA(u8 val)
{
	return cpu.reg.A;
}
//-------------------------------------------------------------------------------------------------
inline u8 op_STX(u8 val)
{
	return cpu.reg.X;
}
//-------------------------------------------------------------------------------------------------
inline u8 op_STY(u8 val)
{
	return cpu.reg.Y;
}
//-------------------------------------------------------------------------------------------------
inline u8 op_TAX(u8 val)
{
	cpu.reg.X = cpu.reg.A;
	cpu.SetZN(cpu.reg.X);
	return cpu.reg.A;
}
//-------------------------------------------------------------------------------------------------
inline u8 op_TAY(u8 val)
{
	cpu.reg.Y = cpu.reg.A;
	cpu.SetZN(cpu.reg.Y);
	return cpu.reg.A;
}
//-------------------------------------------------------------------------------------------------
inline u8 op_TSX(u8 val)
{
	cpu.reg.X = cpu.reg.S;
	cpu.SetZN(cpu.reg.X);
	return cpu.reg.X;
}
//-------------------------------------------------------------------------------------------------
inline u8 op_TXA(u8 val)
{
	cpu.reg.A = cpu.reg.X;
	cpu.SetZN(cpu.reg.A);
	return cpu.reg.A;
}
//-------------------------------------------------------------------------------------------------
inline u8 op_TXS(u8 val)
{
	cpu.reg.S = cpu.reg.X;
	return cpu.reg.S;
}
//-------------------------------------------------------------------------------------------------
inline u8 op_TYA(u8 val)
{
	cpu.reg.A = cpu.reg.Y;
	cpu.SetZN(cpu.reg.A);
	return cpu.reg.A;
}

//-----------------------------------------------------------------------------------------------------------------//
//                                                                                                                 //
// "Illegal" opcodes                                                                                               //
//                                                                                                                 //
//-----------------------------------------------------------------------------------------------------------------//
// Opcode	imp	imm	zp	zpx	zpy	izx	izy	abs	abx	aby	ind	rel	Function					N	V	B	D	I	Z	C  //
//-----------------------------------------------------------------------------------------------------------------//
// SLO	 	 	$07	$17	 	$03	$13	$0F	$1F	$1B	 	 	{adr}:={adr}*2 A:=A or {adr}	*	 	 	 	 	*	*  //
// RLA	 	 	$27	$37	 	$23	$33	$2F	$3F	$3B	 	 	{adr}:={adr}rol A:=A and {adr}	*	 	 	 	 	*	*  //
// SRE	 	 	$47	$57	 	$43	$53	$4F	$5F	$5B	 	 	{adr}:={adr}/2 A:=A exor {adr}	*	 	 	 	 	*	*  //
// RRA	 	 	$67	$77	 	$63	$73	$6F	$7F	$7B	 	 	{adr}:={adr}ror A:=A adc {adr}	*	*	 	 	 	*	*  //
// SAX	 	 	$87	 	$97	$83	 	$8F	 	 	 	 	{adr}:=A&X	 	 	 	 	 	                           //
// LAX	 	 	$A7	 	$B7	$A3	$B3	$AF	 	$BF	 	 	A,X:={adr}						*	 	 	 	 	*	   //
// DCP	 	 	$C7	$D7	 	$C3	$D3	$CF	$DF	$DB	 	 	{adr}:={adr}-1 A-{adr}			*	 	 	 	 	*	*  //
// ISC	 	 	$E7	$F7	 	$E3	$F3	$EF	$FF	$FB	 	 	{adr}:={adr}+1 A:=A-{adr}		*	*	 	 	 	*	*  //
// ANC	 	$0B	 	 	 	 	 	 	 	 	 	 	A:=A&#{imm}						*	 	 	 	 	*	*  //
// ANC	 	$2B	 	 	 	 	 	 	 	 	 	 	A:=A&#{imm}						*	 	 	 	 	*	*  //
// ALR	 	$4B	 	 	 	 	 	 	 	 	 	 	A:=(A&#{imm})/2					*	 	 	 	 	*	*  //
// ARR	 	$6B	 	 	 	 	 	 	 	 	 	 	A:=(A&#{imm})/2					*	*	 	 	 	*	*  //
// XAA�	 	$8B	 	 	 	 	 	 	 	 	 	 	A:=X&#{imm}						*	 	 	 	 	*	   //
// LAX�	 	$AB	 	 	 	 	 	 	 	 	 	 	A,X:=#{imm}						*	 	 	 	 	*	   //
// AXS	 	$CB	 	 	 	 	 	 	 	 	 	 	X:=A&X-#{imm}					*	 	 	 	 	*	*  //
// SBC	 	$EB	 	 	 	 	 	 	 	 	 	 	A:=A-#{imm}						*	*	 	 	 	*	*  //
// AHX�	 	 	 	 	 	 	$93	 	 	$9F	 	 	{adr}:=A&X&H	 	 	 	 		 	                   //
// SHY�	 	 	 	 	 	 	 	 	$9C	 	 	 	{adr}:=Y&H	 	 	 	 	 		 	                   //
// SHX�	 	 	 	 	 	 	 	 	 	$9E	 	 	{adr}:=X&H	 	 	 	 	 		 	                   //
// TAS�	 	 	 	 	 	 	 	 	 	$9B	 	 	S:=A&X {adr}:=S&H	 	 	 		 	                   //
// LAS	 	 	 	 	 	 	 	 	 	$BB	 	 	A,X,S:={adr}&S					*	 	 	 	 	*	   //
//                                                                                                                 //
//-----------------------------------------------------------------------------------------------------------------//
// � = unstable in certain matters                                                                                 //
// � = highly unstable (results are not predictable on some machines)                                              //
//-----------------------------------------------------------------------------------------------------------------//

inline u8 op_SLO( u8 val )
{

	// {adr}:={adr}*2 A:=A or {adr}
	cpu.SetFlag(flag_C, (val & 0x80));
	val = val << 1;
	cpu.reg.A |= val;
	cpu.SetZN(cpu.reg.A);
	return val;
}
//-------------------------------------------------------------------------------------------------
inline u8 op_RLA(u8 val)
{
	// {adr}:={adr}rol A:=A and {adr}
	u8 C = cpu.GetFlag(flag_C);
	cpu.SetFlag(flag_C, (val & 0x80));
	val <<= 1;
	val |= C;
	cpu.reg.A &= val;
	cpu.SetZN( cpu.reg.A );
	return val;
}
//-------------------------------------------------------------------------------------------------
inline u8 op_SRE(u8 val)
{
	// {adr}:={adr}/2 A:=A exor {adr}
	cpu.SetFlag(flag_C, (val & 1));
	val = val >> 1;
	cpu.reg.A ^= val;
	cpu.SetZN( cpu.reg.A);
	return val;
}
//-------------------------------------------------------------------------------------------------
inline u8 op_RRA(u8 val)
{
	// {adr}:={adr}ror A:=A adc {adr}
	u8 C = cpu.GetFlag(flag_C);
	cpu.SetFlag(flag_C, (val & 0x1));
	val >>= 1;
	val |= C<<7;
	op_ADC(val);
	return val;
}
//-------------------------------------------------------------------------------------------------
inline u8 op_SAX(u8 val)
{
	// {adr}:=A&X	 
	val = cpu.reg.A & cpu.reg.X;
	return val;
}
//-------------------------------------------------------------------------------------------------
inline u8 op_LAX(u8 val)
{
	// A,X:={adr}
	cpu.reg.A = val;
	cpu.reg.X = val;
	cpu.SetZN(cpu.reg.A);
	return val;
}
//-------------------------------------------------------------------------------------------------
inline u8 op_DCP(u8 val)
{
	// {adr}:={adr}-1 A-{adr}	
	val--;
    cpu.SetZN(cpu.reg.A-val);
	cpu.SetFlag(flag_C, ( cpu.reg.A >= val ));
	return val;
}

//-------------------------------------------------------------------------------------------------
inline u8 op_ISC(u8 val)
{
	// {adr}:={adr}+1 A:=A-{adr}		
	val++;
	op_SBC( val );
	return val;
}
//-------------------------------------------------------------------------------------------------
inline u8 op_ANC(u8 val)
{
	//A:=A&#{imm}
	cpu.reg.A &= val;
	cpu.SetZN( cpu.reg.A );
	cpu.SetFlag(flag_C,cpu.GetFlag(flag_N));
	return cpu.reg.A;
}

//-------------------------------------------------------------------------------------------------
inline u8 op_ALR(u8 val)
{
	// A:=(A&#{imm})/2					
	cpu.reg.A &= val;
	cpu.SetFlag(flag_C,cpu.reg.A&1);
	cpu.reg.A>>=1;
	cpu.SetZN( cpu.reg.A );
	return cpu.reg.A;
}
//-------------------------------------------------------------------------------------------------
inline u8 op_ARR(u8 val)
{
	// A:=(A&#{imm})/2	
	if ( cpu.GetFlag( flag_D ) ) 
	{
		// 
		// Adapted this version from b-em 
		//
		// This instruction is just as broken on a real 6502 as it is here
		//
		/*
		cpu.reg.A &= val;
		cpu.SetFlag( flag_V, ( cpu.reg.A >> 6 ) ^ ( cpu.reg.A >> 7 ) ); //V set if bit 6 changes in ROR
		cpu.reg.A >>= 1;
		if ( cpu.GetFlag( flag_C ) )
		{
			cpu.reg.A |= 0x80;
			cpu.SetFlag( flag_Z, 1 );
			cpu.SetFlag( flag_N, 1 );
			cpu.SetFlag( flag_C, 0 );
		}
		else
		{
			cpu.SetFlag( flag_Z, 0 );
			cpu.SetFlag( flag_N, 0 );
		}
		if ( ( cpu.reg.A & 0xF ) + ( cpu.reg.A & 1 ) > 5 )
		{
			cpu.reg.A = ( cpu.reg.A & 0xF0 ) + ( ( cpu.reg.A & 0xF ) + 6 ); //Do broken fixup
		}
		if ( ( cpu.reg.A & 0xF0 ) + ( cpu.reg.A & 0x10 ) > 0x50 )
		{
			cpu.reg.A += 0x60;
			cpu.SetFlag( flag_C, 1 );
		}
		*/
		// 
		// Adapted this version from 64doc.txt https://atarihq.com/danb/files/64doc.txt
		//

        u8 t = cpu.reg.A & val;            // Perform the AND. 
        u8 AH = t >> 4;                    // Separate the high 
        u8 AL = t & 15;                    // and low nybbles.

		cpu.reg.A = (t >> 1) | (cpu.GetFlag( flag_C ) << 7);
        cpu.SetFlag( flag_N, cpu.GetFlag( flag_C ));                // Set the N and
        cpu.SetFlag( flag_Z, cpu.reg.A != 0 );						// Z flags traditionally
        cpu.SetFlag( flag_V,  (t ^ cpu.reg.A) & 64 );               // and V flag in a weird way.

		// BCD "fixup" for low nybble.
        if (AL + (AL & 1) > 5)						
		{
			cpu.reg.A = (cpu.reg.A & 0xF0) | ((cpu.reg.A + 6) & 0xF);
		}

		// Set the Carry flag. 
        if ( AH + (AH & 1) > 5 )      
		{
			// BCD "fixup" for high nybble.
			cpu.reg.A = (cpu.reg.A + 0x60) & 0xFF;       
			cpu.SetFlag( flag_C, 1 );
		}
		else
		{
			cpu.SetFlag( flag_C, 0 );
		}
	}
	else
	{
		// V & C flag behaviours in 64doc.txt are backwards
		cpu.reg.A &= val;
		cpu.SetFlag( flag_V, ( cpu.reg.A >> 6 ) ^ ( cpu.reg.A >> 7 ) ); // V set if bit 6 changes in ROR
		cpu.reg.A >>= 1;
		if ( cpu.GetFlag( flag_C ) )
		{
			cpu.reg.A |= 0x80;
		}
		cpu.SetZN( cpu.reg.A );
		cpu.SetFlag( flag_C, cpu.reg.A & 0x40 );
	}
	return cpu.reg.A;
}
//-------------------------------------------------------------------------------------------------
inline u8 op_XAA(u8 val)
{
	// A:=X&#{imm}						
	cpu.reg.A = cpu.reg.X & val;
	cpu.SetZN( cpu.reg.A );
	cpu.SetFlag(flag_C,cpu.GetFlag(flag_N));
	return cpu.reg.A;
}

//-------------------------------------------------------------------------------------------------
inline u8 op_AXS(u8 val)
{
	// X:=A&X-#{imm}					
	cpu.SetFlag(flag_C,( cpu.reg.A & cpu.reg.X )>=val);
	cpu.reg.X = ( cpu.reg.A & cpu.reg.X ) - val;
	cpu.SetZN( cpu.reg.X );
	return cpu.reg.X;
}
//-------------------------------------------------------------------------------------------------
inline u8 op_AHX(u8 val,u8 H /*((addr>>8)+1))*/ )
{
	// {adr}:=A&X&H	 	 	 	 	
    val = cpu.reg.A & cpu.reg.X & H;
	return val;
}
//-------------------------------------------------------------------------------------------------
inline u8 op_SHY(u8 val,u8 H /*((addr>>8)+1))*/ )
{
	// {adr}:=Y&H	 	 	 	 	 	
    val = cpu.reg.Y & H;
	return val;
}
//-------------------------------------------------------------------------------------------------
inline u8 op_SHX(u8 val,u8 H /*((addr>>8)+1))*/ )
{
	// {adr}:=X&H	 	 	 	 	 	
    val = cpu.reg.X & H;
	return val;
}
//-------------------------------------------------------------------------------------------------
inline u8 op_TAS(u8 val,u8 H /*((addr>>8)+1))*/ )
{
	// S:=A&X {adr}:=S&H
	cpu.reg.S = cpu.reg.A & cpu.reg.X;
	val = cpu.reg.S & H;
	return val;
}
//-------------------------------------------------------------------------------------------------
inline u8 op_LAS(u8 val)
{
	// A,X,S:={adr}&S					
	cpu.reg.S &= val;
	cpu.reg.X = cpu.reg.S;
	cpu.reg.A = cpu.reg.S;
	cpu.SetZN( cpu.reg.A );
	return cpu.reg.A;
}

//=================================================================================================
//
// Interrupts
//
//=================================================================================================
//
//      NMI and IRQ both take 7 cycles. Their timing diagram is much like
//      BRK's (see below). IRQ will be executed only when the I flag is
//      clear. IRQ and BRK both set the I flag, whereas the NMI does not
//      affect its state.
// 
//      The processor will usually wait for the current instruction to
//      complete before executing the interrupt sequence. To process the
//      interrupt before the next instruction, the interrupt must occur
//      before the last cycle of the current instruction.
// 
//      There is one exception to this rule: the BRK instruction. If a
//      hardware interrupt (NMI or IRQ) occurs before the fourth (flags
//      saving) cycle of BRK, the BRK instruction will be skipped, and
//      the processor will jump to the hardware interrupt vector. This
//      sequence will always take 7 cycles.
// 
//      You do not completely lose the BRK interrupt, the B flag will be
//      set in the pushed status register if a BRK instruction gets
//      interrupted. When BRK and IRQ occur at the same time, this does
//      not cause any problems, as your program will consider it as a
//      BRK, and the IRQ would occur again after the processor returned
//      from your BRK routine, unless you cleared the interrupt source in
//      your BRK handler. But the simultaneous occurrence of NMI and BRK
//      is far more fatal. If you do not check the B flag in the NMI
//      routine and subtract two from the return address when needed, the
//      BRK instruction will be skipped.
// 
//      If the NMI and IRQ interrupts overlap each other (one interrupt
//      occurs before fetching the interrupt vector for the other
//      interrupt), the processor will most probably jump to the NMI
//      vector in every case, and then jump to the IRQ vector after
//      processing the first instruction of the NMI handler. This has not
//      been measured yet, but the IRQ is very similar to BRK, and many
//      sources state that the NMI has higher priority than IRQ. However,
//      it might be that the processor takes the interrupt that comes
//      later, i.e. you could lose an NMI interrupt if an IRQ occurred in
//      four cycles after it.
// 
//      After finishing the interrupt sequence, the processor will start
//      to execute the first instruction of the interrupt routine. This
//      proves that the processor uses a sort of pipelining: it finishes
//      the current instruction (or interrupt sequence) while reading the
//      opcode of the next instruction.
// 
//      RESET does not push program counter on stack, and it lasts
//      probably 6 cycles after deactivating the signal. Like NMI, RESET
//      preserves all registers except PC.
//
//=================================================================================================

void fn_NMI( )
{
	//DiscardNextPC(); 
	//cpu.IncPC();
	//cpu.Tick();

	PushPCH( ); 
	cpu.DecS(); 
	cpu.Tick();

	PushPCL(); 
	cpu.DecS(); 
	cpu.Tick();

	PushP_noBRK( ); 
	cpu.DecS(); 
	cpu.Tick();

	cpu.SetPCL( mem.Read( cpu.c_NMI_Vector ) );  
	cpu.Tick();
	cpu.Tick();
	cpu.Tick();

	cpu.SetPCH( mem.Read( cpu.c_NMI_Vector + 1 ) ); 
	cpu.LastTick();
}

//-------------------------------------------------------------------------------------------------

void fn_IRQ( )
{
	//DiscardNextPC(); 
	//cpu.IncPC();
	//cpu.Tick();

	PushPCH( ); 
	cpu.DecS(); 
	cpu.Tick();

	PushPCL(); 
	cpu.DecS(); 
	cpu.Tick();

	PushP_noBRK( ); 
	cpu.DecS(); 
	cpu.Tick();

	cpu.SetPCL( mem.Read( cpu.c_IRQ_Vector ) );  
	cpu.Tick();
	cpu.Tick();
	cpu.Tick();

	cpu.SetPCH( mem.Read( cpu.c_IRQ_Vector + 1 ) ); 
	cpu.LastTick();
	SetFlags( flag_I ); 
}

//=================================================================================================
//
//  Instructions accessing the stack
//
//=================================================================================================
namespace StackInstructions
{
	//-------------------------------------------------------------------------------------------------
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
		cpu.IncPC();
		cpu.Tick();
		
		SetFlags( flag_B ); 
		PushPCH( ); 
		cpu.DecS(); 
		cpu.Tick();

		PushPCL(); 
		cpu.DecS(); 
		cpu.Tick();

		PushP_BRK( ); 
		cpu.DecS(); 
		cpu.Tick();

		cpu.SetPCL( mem.Read( cpu.c_IRQ_Vector ) );  
		cpu.Tick();

		cpu.SetPCH( mem.Read( cpu.c_IRQ_Vector + 1 ) ); 
		cpu.LastTick();
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
		cpu.Tick();
		
		cpu.IncS(); 
		cpu.Tick();

		PullP(); 
		cpu.IncS(); 
		cpu.Tick();

		PullPCL(); 
		cpu.IncS(); 
		cpu.Tick();

		PullPCH(); 
		cpu.LastTick();
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
		cpu.Tick();
		
		cpu.IncS(); 
		cpu.Tick();
		
		PullPCL(); 
		cpu.IncS(); 
		cpu.Tick();
		
		PullPCH(); 
		cpu.Tick();
		
		cpu.IncPC();
		cpu.LastTick();
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
		cpu.Tick();
		
		PushA(); 
		cpu.DecS(); 
		cpu.LastTick();
	}
	//-------------------------------------------------------------------------------------------------
	void fn_PHP()
	{
		DiscardNextPC(); 
		cpu.Tick();
		
		PushP_BRK( ); 
		cpu.DecS(); 
		cpu.LastTick();
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
		cpu.Tick();
		
		cpu.IncS(); 
		cpu.Tick();
		
		PullA(); 
		cpu.LastTick();
	}
	//-------------------------------------------------------------------------------------------------
	void fn_PLP()
	{
		DiscardNextPC(); 
		cpu.Tick();
		
		cpu.IncS(); 
		cpu.Tick();

		PullP(); 
		cpu.LastTick();
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
		u8 lo = mem.Read(cpu.reg.PC);
		cpu.IncPC();
		cpu.Tick();

		// internal operation?
		cpu.Tick();

		PushPCH();
		cpu.DecS();
		cpu.Tick();

		PushPCL();
		cpu.DecS();
		cpu.Tick();

		u8 hi = mem.Read(cpu.reg.PC);
		SetPC( lo, hi );
		cpu.LastTick();
	}

	//-------------------------------------------------------------------------------------------------
	void RegisterInstructions( OpcodeTable& opcodeTable )
	{
		opcodeTable.SetFunctionHandler( mode_imp, BRK, fn_BRK );
		opcodeTable.SetFunctionHandler( mode_imp, RTI, fn_RTI );
		opcodeTable.SetFunctionHandler( mode_imp, RTS, fn_RTS );
		opcodeTable.SetFunctionHandler( mode_imp, PHA, fn_PHA );
		opcodeTable.SetFunctionHandler( mode_imp, PHP, fn_PHP );
		opcodeTable.SetFunctionHandler( mode_imp, PLA, fn_PLA );
		opcodeTable.SetFunctionHandler( mode_imp, PLP, fn_PLP );

		opcodeTable.SetFunctionHandler( mode_abs, JSR, fn_JSR );
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
		Operation(0);
		cpu.LastTick();
	}
	template <u8(*Operation)(u8)>
	void fn_Accumulator()
	{
		DiscardNextPC();
		cpu.reg.A = Operation(cpu.reg.A);
		cpu.LastTick();
	}
	//-------------------------------------------------------------------------------------------------
	void RegisterInstructions( OpcodeTable& opcodeTable )
	{
		opcodeTable.SetFunctionHandler( mode_imp, ASL, fn_Accumulator<op_ASL> );
		opcodeTable.SetFunctionHandler( mode_imp, ROL, fn_Accumulator<op_ROL> );
		opcodeTable.SetFunctionHandler( mode_imp, ROR, fn_Accumulator<op_ROR> );
		opcodeTable.SetFunctionHandler( mode_imp, LSR, fn_Accumulator<op_LSR> );

		opcodeTable.SetFunctionHandler( mode_imp, DEX, fn_Implied<op_DEX> );
		opcodeTable.SetFunctionHandler( mode_imp, INX, fn_Implied<op_INX> );
		opcodeTable.SetFunctionHandler( mode_imp, DEY, fn_Implied<op_DEY> );
		opcodeTable.SetFunctionHandler( mode_imp, INY, fn_Implied<op_INY> );
		opcodeTable.SetFunctionHandler( mode_imp, TAX, fn_Implied<op_TAX> );
		opcodeTable.SetFunctionHandler( mode_imp, TXA, fn_Implied<op_TXA> );
		opcodeTable.SetFunctionHandler( mode_imp, TAY, fn_Implied<op_TAY> );
		opcodeTable.SetFunctionHandler( mode_imp, TYA, fn_Implied<op_TYA> );
		opcodeTable.SetFunctionHandler( mode_imp, TSX, fn_Implied<op_TSX> );
		opcodeTable.SetFunctionHandler( mode_imp, TXS, fn_Implied<op_TXS> );
		opcodeTable.SetFunctionHandler( mode_imp, CLC, fn_Implied<op_CLC> );
		opcodeTable.SetFunctionHandler( mode_imp, SEC, fn_Implied<op_SEC> );
		opcodeTable.SetFunctionHandler( mode_imp, CLD, fn_Implied<op_CLD> );
		opcodeTable.SetFunctionHandler( mode_imp, SED, fn_Implied<op_SED> );
		opcodeTable.SetFunctionHandler( mode_imp, CLI, fn_Implied<op_CLI> );
		opcodeTable.SetFunctionHandler( mode_imp, SEI, fn_Implied<op_SEI> );
		opcodeTable.SetFunctionHandler( mode_imp, CLV, fn_Implied<op_CLV> );
		opcodeTable.SetFunctionHandler( mode_imp, NOP, fn_Implied<op_NOP> );
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
		u8 value = mem.Read(cpu.reg.PC);
		cpu.IncPC();
		Operation(value);
		cpu.LastTick();
	}

	//-------------------------------------------------------------------------------------------------
	void RegisterInstructions( OpcodeTable& opcodeTable )
	{
		opcodeTable.SetFunctionHandler( mode_imm, ORA, fn_Immediate<op_ORA> );
		opcodeTable.SetFunctionHandler( mode_imm, AND, fn_Immediate<op_AND> );
		opcodeTable.SetFunctionHandler( mode_imm, EOR, fn_Immediate<op_EOR> );
		opcodeTable.SetFunctionHandler( mode_imm, ADC, fn_Immediate<op_ADC> );
		opcodeTable.SetFunctionHandler( mode_imm, SBC, fn_Immediate<op_SBC> );
		opcodeTable.SetFunctionHandler( mode_imm, CMP, fn_Immediate<op_CMP> );
		opcodeTable.SetFunctionHandler( mode_imm, CPX, fn_Immediate<op_CPX> );
		opcodeTable.SetFunctionHandler( mode_imm, CPY, fn_Immediate<op_CPY> );
		opcodeTable.SetFunctionHandler( mode_imm, LDA, fn_Immediate<op_LDA> );
		opcodeTable.SetFunctionHandler( mode_imm, LDX, fn_Immediate<op_LDX> );
		opcodeTable.SetFunctionHandler( mode_imm, LDY, fn_Immediate<op_LDY> );

		opcodeTable.SetFunctionHandler( mode_imm, ANC, fn_Immediate<op_ANC> );
		opcodeTable.SetFunctionHandler( mode_imm, ALR, fn_Immediate<op_ALR> );
		opcodeTable.SetFunctionHandler( mode_imm, ARR, fn_Immediate<op_ARR> );
		opcodeTable.SetFunctionHandler( mode_imm, XAA, fn_Immediate<op_XAA> );
		opcodeTable.SetFunctionHandler( mode_imm, LAX, fn_Immediate<op_LAX> );
		opcodeTable.SetFunctionHandler( mode_imm, AXS, fn_Immediate<op_AXS> );
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
		u8 lo = mem.Read(cpu.reg.PC);
		cpu.IncPC();
		cpu.Tick();

		u8 hi = mem.Read(cpu.reg.PC);
		SetPC( lo, hi );
		cpu.LastTick();
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
		address = mem.ReadLoByte(cpu.reg.PC, address);
		cpu.IncPC();
		cpu.Tick();

		address = mem.ReadHiByte(cpu.reg.PC, address);
		cpu.IncPC();
		cpu.Tick();

		u8 value = mem.Read(address);
		Operation(value);
		cpu.LastTick();
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
		address = mem.ReadLoByte(cpu.reg.PC, address);
		cpu.IncPC();
		cpu.Tick();

		address = mem.ReadHiByte(cpu.reg.PC, address);
		cpu.IncPC();
		cpu.Tick();

		u8 value = mem.Read(address);
		cpu.Tick();

		mem.Write(address, value);
		value = Operation(value);
		cpu.Tick();

		mem.Write(address, value);
		cpu.LastTick();
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
	template <u8&(*Register)(),u8(*Operation)(u8)>
	void fn_WriteInstructions()
	{
		u16 address = 0;
		address = mem.ReadLoByte(cpu.reg.PC, address);
		cpu.IncPC();
		cpu.Tick();

		address = mem.ReadHiByte(cpu.reg.PC, address);
		cpu.IncPC();
		cpu.Tick();

		mem.Write(address, Operation( Register() ));
		cpu.LastTick();
	}

	//-------------------------------------------------------------------------------------------------
	void RegisterInstructions( OpcodeTable& opcodeTable )
	{
		opcodeTable.SetFunctionHandler( mode_abs, JMP, fn_JMP );

		opcodeTable.SetFunctionHandler(mode_abs, LDA, fn_ReadInstructions<op_LDA>);
		opcodeTable.SetFunctionHandler(mode_abs, LDX, fn_ReadInstructions<op_LDX>);
		opcodeTable.SetFunctionHandler(mode_abs, LDY, fn_ReadInstructions<op_LDY>);
		opcodeTable.SetFunctionHandler(mode_abs, EOR, fn_ReadInstructions<op_EOR>);
		opcodeTable.SetFunctionHandler(mode_abs, AND, fn_ReadInstructions<op_AND>);
		opcodeTable.SetFunctionHandler(mode_abs, ORA, fn_ReadInstructions<op_ORA>);
		opcodeTable.SetFunctionHandler(mode_abs, ADC, fn_ReadInstructions<op_ADC>);
		opcodeTable.SetFunctionHandler(mode_abs, SBC, fn_ReadInstructions<op_SBC>);
		opcodeTable.SetFunctionHandler(mode_abs, CMP, fn_ReadInstructions<op_CMP>);
		opcodeTable.SetFunctionHandler(mode_abs, CPX, fn_ReadInstructions<op_CPX>);
		opcodeTable.SetFunctionHandler(mode_abs, CPY, fn_ReadInstructions<op_CPY>);
		opcodeTable.SetFunctionHandler(mode_abs, BIT, fn_ReadInstructions<op_BIT>);
		opcodeTable.SetFunctionHandler(mode_abs, LAX, fn_ReadInstructions<op_LAX>);

		opcodeTable.SetFunctionHandler(mode_abs, ASL, fn_ReadModifyWriteInstructions<op_ASL>);
		opcodeTable.SetFunctionHandler(mode_abs, LSR, fn_ReadModifyWriteInstructions<op_LSR>);
		opcodeTable.SetFunctionHandler(mode_abs, ROL, fn_ReadModifyWriteInstructions<op_ROL>);
		opcodeTable.SetFunctionHandler(mode_abs, ROR, fn_ReadModifyWriteInstructions<op_ROR>);
		opcodeTable.SetFunctionHandler(mode_abs, INC, fn_ReadModifyWriteInstructions<op_INC>);
		opcodeTable.SetFunctionHandler(mode_abs, DEC, fn_ReadModifyWriteInstructions<op_DEC>);

		opcodeTable.SetFunctionHandler(mode_abs, SLO, fn_ReadModifyWriteInstructions<op_SLO>);
		opcodeTable.SetFunctionHandler(mode_abs, SRE, fn_ReadModifyWriteInstructions<op_SRE>);
		opcodeTable.SetFunctionHandler(mode_abs, RLA, fn_ReadModifyWriteInstructions<op_RLA>);
		opcodeTable.SetFunctionHandler(mode_abs, RRA, fn_ReadModifyWriteInstructions<op_RRA>);
		opcodeTable.SetFunctionHandler(mode_abs, ISC, fn_ReadModifyWriteInstructions<op_ISC>);
		opcodeTable.SetFunctionHandler(mode_abs, DCP, fn_ReadModifyWriteInstructions<op_DCP>);

		opcodeTable.SetFunctionHandler(mode_abs, STA, fn_WriteInstructions<reg_cpuA,op_NULL>);
		opcodeTable.SetFunctionHandler(mode_abs, STX, fn_WriteInstructions<reg_cpuX,op_NULL>);
		opcodeTable.SetFunctionHandler(mode_abs, STY, fn_WriteInstructions<reg_cpuY,op_NULL>);
		opcodeTable.SetFunctionHandler(mode_abs, SAX, fn_WriteInstructions<reg_cpuY,op_SAX>);
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
		address = mem.Read(cpu.reg.PC);
		cpu.IncPC();
		cpu.Tick();

		u8 value = mem.Read(address);
		Operation(value);
		cpu.LastTick();
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
		address = mem.Read(cpu.reg.PC);
		cpu.IncPC();
		cpu.Tick();

		u8 value = mem.Read(address);
		cpu.Tick();

		mem.Write(address, value);
		value = Operation(value);
		cpu.Tick();

		mem.Write(address, value);
		cpu.LastTick();
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
	template <u8&(*Register)(), u8(*Operation)(u8)>
	void fn_WriteInstructions()
	{
		u8 address = 0;
		address = mem.Read(cpu.reg.PC);
		cpu.IncPC();
		cpu.Tick();

		mem.Write(address, Operation( Register() ) );
		cpu.LastTick();
	}

	//-------------------------------------------------------------------------------------------------

	void RegisterInstructions( OpcodeTable& opcodeTable )
	{
		opcodeTable.SetFunctionHandler(mode_zp, LDA, fn_ReadInstructions<reg_cpuX, op_LDA>);
		opcodeTable.SetFunctionHandler(mode_zp, LDX, fn_ReadInstructions<reg_cpuX, op_LDX>);
		opcodeTable.SetFunctionHandler(mode_zp, LDY, fn_ReadInstructions<reg_cpuX, op_LDY>);
		opcodeTable.SetFunctionHandler(mode_zp, EOR, fn_ReadInstructions<reg_cpuX, op_EOR>);
		opcodeTable.SetFunctionHandler(mode_zp, AND, fn_ReadInstructions<reg_cpuX, op_AND>);
		opcodeTable.SetFunctionHandler(mode_zp, ORA, fn_ReadInstructions<reg_cpuX, op_ORA>);
		opcodeTable.SetFunctionHandler(mode_zp, ADC, fn_ReadInstructions<reg_cpuX, op_ADC>);
		opcodeTable.SetFunctionHandler(mode_zp, SBC, fn_ReadInstructions<reg_cpuX, op_SBC>);
		opcodeTable.SetFunctionHandler(mode_zp, CMP, fn_ReadInstructions<reg_cpuX, op_CMP>);
		opcodeTable.SetFunctionHandler(mode_zp, CPX, fn_ReadInstructions<reg_cpuX, op_CPX>);
		opcodeTable.SetFunctionHandler(mode_zp, CPY, fn_ReadInstructions<reg_cpuX, op_CPY>);
		opcodeTable.SetFunctionHandler(mode_zp, BIT, fn_ReadInstructions<reg_cpuX, op_BIT>);
		opcodeTable.SetFunctionHandler(mode_zp, LAX, fn_ReadInstructions<reg_cpuX, op_LAX>);

		opcodeTable.SetFunctionHandler(mode_zp, ASL, fn_ReadModifyWriteInstructions<op_ASL>);
		opcodeTable.SetFunctionHandler(mode_zp, LSR, fn_ReadModifyWriteInstructions<op_LSR>);
		opcodeTable.SetFunctionHandler(mode_zp, ROL, fn_ReadModifyWriteInstructions<op_ROL>);
		opcodeTable.SetFunctionHandler(mode_zp, ROR, fn_ReadModifyWriteInstructions<op_ROR>);
		opcodeTable.SetFunctionHandler(mode_zp, INC, fn_ReadModifyWriteInstructions<op_INC>);
		opcodeTable.SetFunctionHandler(mode_zp, DEC, fn_ReadModifyWriteInstructions<op_DEC>);

		opcodeTable.SetFunctionHandler(mode_zp, SLO, fn_ReadModifyWriteInstructions<op_SLO>);
		opcodeTable.SetFunctionHandler(mode_zp, SRE, fn_ReadModifyWriteInstructions<op_SRE>);
		opcodeTable.SetFunctionHandler(mode_zp, RLA, fn_ReadModifyWriteInstructions<op_RLA>);
		opcodeTable.SetFunctionHandler(mode_zp, RRA, fn_ReadModifyWriteInstructions<op_RRA>);
		opcodeTable.SetFunctionHandler(mode_zp, ISC, fn_ReadModifyWriteInstructions<op_ISC>);
		opcodeTable.SetFunctionHandler(mode_zp, DCP, fn_ReadModifyWriteInstructions<op_DCP>);

		opcodeTable.SetFunctionHandler(mode_zp, STA, fn_WriteInstructions<reg_cpuA, op_NULL>);
		opcodeTable.SetFunctionHandler(mode_zp, STX, fn_WriteInstructions<reg_cpuX, op_NULL>);
		opcodeTable.SetFunctionHandler(mode_zp, STY, fn_WriteInstructions<reg_cpuY, op_NULL>);
		opcodeTable.SetFunctionHandler(mode_zp, SAX, fn_WriteInstructions<reg_cpuA, op_SAX>);
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
		address = mem.Read( cpu.reg.PC );
		cpu.IncPC();
		cpu.Tick();

		mem.Read(address);
		address += Index();
		cpu.Tick();

		u8 value = mem.Read(address);
		Operation(value);
		cpu.LastTick();
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
		address = mem.Read( cpu.reg.PC );
		cpu.IncPC();
		cpu.Tick();

		mem.Read(address);
		address += cpu.reg.X;
		cpu.Tick();

		u8 value = mem.Read(address);
		cpu.Tick();

		mem.Write(address,value);
		value = Operation(value);
		cpu.Tick();

		mem.Write(address,value);
		cpu.LastTick();
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
	template <u8&(*Index)(),u8&(*Register)(),u8(*Operation)(u8)>
	void fn_WriteInstructions( )
	{
		u8 address = 0;
		address = mem.Read( cpu.reg.PC );
		cpu.IncPC();
		cpu.Tick();

		mem.Read(address);
		address += Index();
		cpu.Tick();

		mem.Write( address, Operation( Register() ) );
		cpu.LastTick();
	}

	//-------------------------------------------------------------------------------------------------

	void RegisterInstructions( OpcodeTable& opcodeTable )
	{
		opcodeTable.SetFunctionHandler( mode_zpx, LDA, fn_ReadInstructions<reg_cpuX,op_LDA> );
		opcodeTable.SetFunctionHandler( mode_zpx, LDY, fn_ReadInstructions<reg_cpuX,op_LDY> );
		opcodeTable.SetFunctionHandler( mode_zpx, EOR, fn_ReadInstructions<reg_cpuX,op_EOR> );
		opcodeTable.SetFunctionHandler( mode_zpx, AND, fn_ReadInstructions<reg_cpuX,op_AND> );
		opcodeTable.SetFunctionHandler( mode_zpx, ORA, fn_ReadInstructions<reg_cpuX,op_ORA> );
		opcodeTable.SetFunctionHandler( mode_zpx, ADC, fn_ReadInstructions<reg_cpuX,op_ADC> );
		opcodeTable.SetFunctionHandler( mode_zpx, SBC, fn_ReadInstructions<reg_cpuX,op_SBC> );
		opcodeTable.SetFunctionHandler( mode_zpx, CMP, fn_ReadInstructions<reg_cpuX,op_CMP> );

		opcodeTable.SetFunctionHandler( mode_zpy, LDX, fn_ReadInstructions<reg_cpuY,op_LDX> );
		opcodeTable.SetFunctionHandler( mode_zpy, LAX, fn_ReadInstructions<reg_cpuY,op_LAX> );

		opcodeTable.SetFunctionHandler( mode_zpx, ASL, fn_ReadModifyWriteInstructions<op_ASL> );
		opcodeTable.SetFunctionHandler( mode_zpx, LSR, fn_ReadModifyWriteInstructions<op_LSR> );
		opcodeTable.SetFunctionHandler( mode_zpx, ROL, fn_ReadModifyWriteInstructions<op_ROL> );
		opcodeTable.SetFunctionHandler( mode_zpx, ROR, fn_ReadModifyWriteInstructions<op_ROR> );
		opcodeTable.SetFunctionHandler( mode_zpx, INC, fn_ReadModifyWriteInstructions<op_INC> );
		opcodeTable.SetFunctionHandler( mode_zpx, DEC, fn_ReadModifyWriteInstructions<op_DEC> );

		opcodeTable.SetFunctionHandler( mode_zpx, SLO, fn_ReadModifyWriteInstructions<op_SLO>);
		opcodeTable.SetFunctionHandler( mode_zpx, SRE, fn_ReadModifyWriteInstructions<op_SRE>);
		opcodeTable.SetFunctionHandler( mode_zpx, RLA, fn_ReadModifyWriteInstructions<op_RLA>);
		opcodeTable.SetFunctionHandler( mode_zpx, RRA, fn_ReadModifyWriteInstructions<op_RRA>);
		opcodeTable.SetFunctionHandler( mode_zpx, ISC, fn_ReadModifyWriteInstructions<op_ISC>);
		opcodeTable.SetFunctionHandler( mode_zpx, DCP, fn_ReadModifyWriteInstructions<op_DCP>);

		opcodeTable.SetFunctionHandler( mode_zpx, STA, fn_WriteInstructions<reg_cpuX,reg_cpuA,op_NULL> );
		opcodeTable.SetFunctionHandler( mode_zpx, STY, fn_WriteInstructions<reg_cpuX,reg_cpuY,op_NULL> );
		
		opcodeTable.SetFunctionHandler( mode_zpy, STX, fn_WriteInstructions<reg_cpuY,reg_cpuX,op_NULL> );
		opcodeTable.SetFunctionHandler( mode_zpy, SAX, fn_WriteInstructions<reg_cpuY,reg_cpuA,op_SAX> );
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
		mem.ReadLoByte( cpu.reg.PC, address );
		cpu.IncPC();
		cpu.Tick();

		mem.ReadHiByte( cpu.reg.PC, address );
		u16 temp_address = ( address & 0xffffff00 ) + ( ( address + Index() ) & 0xff );
		cpu.IncPC();
		cpu.Tick();

		u8 value = mem.Read( temp_address );
		address = address + Index(); 

		if ( address != temp_address )
		{ 
			value = mem.Read( address );
			cpu.Tick();
			#ifdef TEST_CYCLE_TIMES
			cpu.m_dbgExtraCycleDueToPageFault = true;
			#endif
		}
		Operation(value);
		cpu.LastTick();
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
		mem.ReadLoByte( cpu.reg.PC, address );
		cpu.IncPC();
		cpu.Tick();

		mem.ReadHiByte( cpu.reg.PC, address );
		u16 temp_address = ( address & 0xffffff00 ) + ( ( address + cpu.reg.X ) & 0xff );
		cpu.IncPC();
		cpu.Tick();

		u8 value = mem.Read( temp_address );
		address = address + cpu.reg.X;
		cpu.Tick();

		value = mem.Read( temp_address );
		cpu.Tick();

		mem.Write( address, value );
		value = Operation(value);
		cpu.Tick();

		mem.Write( address, value );
		cpu.LastTick();
	}

	//-------------------------------------------------------------------------------------------------
	template <u8&(*Index)(),u8&(*Register)(),u8(*Operation)(u8)>
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
		mem.ReadLoByte( cpu.reg.PC, address );
		cpu.IncPC();
		cpu.Tick();

		mem.ReadHiByte( cpu.reg.PC, address );
		u16 temp_address = ( address & 0xffffff00 ) + ( ( address + Index() ) & 0xff );
		cpu.IncPC();
		cpu.Tick();

		u8 value = mem.Read( temp_address );
		address = address + Index(); 
		cpu.Tick();

		mem.Write( address, Operation( Register() ) );
		cpu.LastTick();
	}
	//-------------------------------------------------------------------------------------------------
	// This variant passes the high byte in of the effective address (plus 1 ?!)
	template <u8&(*Index)(),u8&(*Register)(),u8(*Operation)(u8,u8)>
	void fn_WriteInstructionsH( )
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
		mem.ReadLoByte( cpu.reg.PC, address );
		cpu.IncPC();
		cpu.Tick();

		mem.ReadHiByte( cpu.reg.PC, address );
		u16 temp_address = ( address & 0xffffff00 ) + ( ( address + Index() ) & 0xff );
		cpu.IncPC();
		cpu.Tick();

		u8 value = mem.Read( temp_address );
		address = address + Index(); 
		cpu.Tick();

		mem.Write( address, Operation( Register(), 1 + ( ( address >> 8 ) & 0xff ) ) );
		cpu.LastTick();
	}
	//-------------------------------------------------------------------------------------------------
	void RegisterInstructions( OpcodeTable& opcodeTable )
	{
		opcodeTable.SetFunctionHandler( mode_abx, LDA, fn_ReadInstructions<reg_cpuX,op_LDA> );
		opcodeTable.SetFunctionHandler( mode_abx, LDY, fn_ReadInstructions<reg_cpuX,op_LDY> );
		opcodeTable.SetFunctionHandler( mode_abx, EOR, fn_ReadInstructions<reg_cpuX,op_EOR> );
		opcodeTable.SetFunctionHandler( mode_abx, AND, fn_ReadInstructions<reg_cpuX,op_AND> );
		opcodeTable.SetFunctionHandler( mode_abx, ORA, fn_ReadInstructions<reg_cpuX,op_ORA> );
		opcodeTable.SetFunctionHandler( mode_abx, ADC, fn_ReadInstructions<reg_cpuX,op_ADC> );
		opcodeTable.SetFunctionHandler( mode_abx, SBC, fn_ReadInstructions<reg_cpuX,op_SBC> );
		opcodeTable.SetFunctionHandler( mode_abx, CMP, fn_ReadInstructions<reg_cpuX,op_CMP> );
		
		opcodeTable.SetFunctionHandler( mode_aby, LDA, fn_ReadInstructions<reg_cpuY,op_LDA> );
		opcodeTable.SetFunctionHandler( mode_aby, LDX, fn_ReadInstructions<reg_cpuY,op_LDX> );
		opcodeTable.SetFunctionHandler( mode_aby, EOR, fn_ReadInstructions<reg_cpuY,op_EOR> );
		opcodeTable.SetFunctionHandler( mode_aby, AND, fn_ReadInstructions<reg_cpuY,op_AND> );
		opcodeTable.SetFunctionHandler( mode_aby, ORA, fn_ReadInstructions<reg_cpuY,op_ORA> );
		opcodeTable.SetFunctionHandler( mode_aby, ADC, fn_ReadInstructions<reg_cpuY,op_ADC> );
		opcodeTable.SetFunctionHandler( mode_aby, SBC, fn_ReadInstructions<reg_cpuY,op_SBC> );
		opcodeTable.SetFunctionHandler( mode_aby, CMP, fn_ReadInstructions<reg_cpuY,op_CMP> );

		opcodeTable.SetFunctionHandler( mode_abx, ASL, fn_ReadModifyWriteInstructions<op_ASL> );
		opcodeTable.SetFunctionHandler( mode_abx, LSR, fn_ReadModifyWriteInstructions<op_LSR> );
		opcodeTable.SetFunctionHandler( mode_abx, ROL, fn_ReadModifyWriteInstructions<op_ROL> );
		opcodeTable.SetFunctionHandler( mode_abx, ROR, fn_ReadModifyWriteInstructions<op_ROR> );
		opcodeTable.SetFunctionHandler( mode_abx, INC, fn_ReadModifyWriteInstructions<op_INC> );
		opcodeTable.SetFunctionHandler( mode_abx, DEC, fn_ReadModifyWriteInstructions<op_DEC> );

		opcodeTable.SetFunctionHandler( mode_abx, SLO, fn_ReadModifyWriteInstructions<op_SLO>);
		opcodeTable.SetFunctionHandler( mode_abx, SRE, fn_ReadModifyWriteInstructions<op_SRE>);
		opcodeTable.SetFunctionHandler( mode_abx, RLA, fn_ReadModifyWriteInstructions<op_RLA>);
		opcodeTable.SetFunctionHandler( mode_abx, RRA, fn_ReadModifyWriteInstructions<op_RRA>);
		opcodeTable.SetFunctionHandler( mode_abx, ISC, fn_ReadModifyWriteInstructions<op_ISC>);
		opcodeTable.SetFunctionHandler( mode_abx, DCP, fn_ReadModifyWriteInstructions<op_DCP>);

		opcodeTable.SetFunctionHandler( mode_abx, STA, fn_WriteInstructions<reg_cpuX,reg_cpuA,op_NULL> );
		opcodeTable.SetFunctionHandler( mode_abx, SHY, fn_WriteInstructionsH<reg_cpuX,reg_cpuA,op_SHY> );
		
		opcodeTable.SetFunctionHandler( mode_aby, STA, fn_WriteInstructions<reg_cpuY,reg_cpuA,op_NULL> );
		opcodeTable.SetFunctionHandler( mode_aby, LAX, fn_WriteInstructions<reg_cpuY,reg_cpuA,op_LAX> );
		opcodeTable.SetFunctionHandler( mode_aby, AHX, fn_WriteInstructionsH<reg_cpuY,reg_cpuA,op_AHX> );
		opcodeTable.SetFunctionHandler( mode_aby, SHX, fn_WriteInstructionsH<reg_cpuY,reg_cpuA,op_SHX> );
		opcodeTable.SetFunctionHandler( mode_aby, TAS, fn_WriteInstructionsH<reg_cpuY,reg_cpuA,op_TAS> );
		opcodeTable.SetFunctionHandler( mode_aby, LAS, fn_WriteInstructions<reg_cpuY,reg_cpuA,op_LAS> );

		opcodeTable.SetFunctionHandler( mode_aby, SLO, fn_ReadModifyWriteInstructions<op_SLO>);
		opcodeTable.SetFunctionHandler( mode_aby, SRE, fn_ReadModifyWriteInstructions<op_SRE>);
		opcodeTable.SetFunctionHandler( mode_aby, RLA, fn_ReadModifyWriteInstructions<op_RLA>);
		opcodeTable.SetFunctionHandler( mode_aby, RRA, fn_ReadModifyWriteInstructions<op_RRA>);
		opcodeTable.SetFunctionHandler( mode_aby, ISC, fn_ReadModifyWriteInstructions<op_ISC>);
		opcodeTable.SetFunctionHandler( mode_aby, DCP, fn_ReadModifyWriteInstructions<op_DCP>);
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
		u16 oldPC = cpu.reg.PC-1;
		s8 operand = FetchOperand();
		if ( CheckBranch() )
		{
			#ifdef TEST_CYCLE_TIMES
			cpu.m_dbgTookBranch = true;
			#endif
			u16 newPC = cpu.reg.PC + operand;
			cpu.reg.PC = ( cpu.reg.PC & 0xff00 ) | (( operand + cpu.reg.PC )&0xff );
			cpu.Tick();
			
			if ( newPC != cpu.reg.PC )
			{
				cpu.reg.PC = newPC;
				cpu.Tick();
				#ifdef TEST_CYCLE_TIMES
				cpu.m_dbgExtraCycleDueToPageFault = true;
				#endif
			}
		}
		cpu.LastTick();
	}
	
	//-------------------------------------------------------------------------------------------------
	void RegisterInstructions( OpcodeTable& opcodeTable )
	{
		opcodeTable.SetFunctionHandler( mode_rel, BCC, fn_BranchInstructions<op_BCC> );
		opcodeTable.SetFunctionHandler( mode_rel, BCS, fn_BranchInstructions<op_BCS> );
		opcodeTable.SetFunctionHandler( mode_rel, BNE, fn_BranchInstructions<op_BNE> );
		opcodeTable.SetFunctionHandler( mode_rel, BEQ, fn_BranchInstructions<op_BEQ> );
		opcodeTable.SetFunctionHandler( mode_rel, BPL, fn_BranchInstructions<op_BPL> );
		opcodeTable.SetFunctionHandler( mode_rel, BMI, fn_BranchInstructions<op_BMI> );
		opcodeTable.SetFunctionHandler( mode_rel, BVC, fn_BranchInstructions<op_BVC> );
		opcodeTable.SetFunctionHandler( mode_rel, BVS, fn_BranchInstructions<op_BVS> );
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
		cpu.Tick();

		mem.Read( pointer );
		pointer += cpu.reg.X;
		u16 address = Get16BitAddressFromPointer( pointer );  // (+2 ticks)

		u8 value = mem.Read( address );
		cpu.Tick();

		Operation( value );
		cpu.LastTick();
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
		cpu.Tick();

		mem.Read( pointer );
		pointer += cpu.reg.X;
		cpu.Tick();

		u16 address = Get16BitAddressFromPointer( pointer ); // (+2 ticks)

		u8 value = mem.Read( address );
		cpu.Tick();

		mem.Write( address, value );
		value = Operation( value );
		cpu.Tick();

		mem.Write( address, value );
		cpu.LastTick();
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
	template <u8&(*Register)(),u8(*Operation)(u8)>
	void fn_WriteInstructions( )
	{
		u8 pointer = FetchPointer();
		cpu.Tick();

		mem.Read( pointer );
		pointer += cpu.reg.X;
		cpu.Tick();

		u16 address = Get16BitAddressFromPointer( pointer ); // (+2 ticks)

		u8 value = Operation( Register() );
		mem.Write( address, value );
		cpu.LastTick();
	}
	//-------------------------------------------------------------------------------------------------
	void RegisterInstructions( OpcodeTable& opcodeTable )
	{
		opcodeTable.SetFunctionHandler( mode_izx, LDA, fn_ReadInstructions<op_LDA> );
		opcodeTable.SetFunctionHandler( mode_izx, ORA, fn_ReadInstructions<op_ORA> );
		opcodeTable.SetFunctionHandler( mode_izx, EOR, fn_ReadInstructions<op_EOR> );
		opcodeTable.SetFunctionHandler( mode_izx, AND, fn_ReadInstructions<op_AND> );
		opcodeTable.SetFunctionHandler( mode_izx, ADC, fn_ReadInstructions<op_ADC> );
		opcodeTable.SetFunctionHandler( mode_izx, CMP, fn_ReadInstructions<op_CMP> );
		opcodeTable.SetFunctionHandler( mode_izx, SBC, fn_ReadInstructions<op_SBC> );

		opcodeTable.SetFunctionHandler( mode_izx, LAX, fn_ReadInstructions<op_LAX> );

		opcodeTable.SetFunctionHandler( mode_izx, SLO, fn_ReadModifyWriteInstructions<op_SLO>);
		opcodeTable.SetFunctionHandler( mode_izx, RLA, fn_ReadModifyWriteInstructions<op_RLA>);
		opcodeTable.SetFunctionHandler( mode_izx, SRE, fn_ReadModifyWriteInstructions<op_SRE>);
		opcodeTable.SetFunctionHandler( mode_izx, RRA, fn_ReadModifyWriteInstructions<op_RRA>);
		opcodeTable.SetFunctionHandler( mode_izx, ISC, fn_ReadModifyWriteInstructions<op_ISC>);
		opcodeTable.SetFunctionHandler( mode_izx, DCP, fn_ReadModifyWriteInstructions<op_DCP>);

		opcodeTable.SetFunctionHandler( mode_izx, STA, fn_WriteInstructions<reg_cpuA,op_NULL> );
		opcodeTable.SetFunctionHandler( mode_izx, SAX, fn_WriteInstructions<reg_cpuA,op_SAX> );
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
		cpu.Tick();

		mem.Read( pointer );

		u16 address = Get16BitAddressFromPointer( pointer ); // (+2 ticks)

		u16 temp_address = ( address & 0xffffff00 ) + ( ( address + cpu.reg.Y ) & 0xff );
		address += cpu.reg.Y;

		u8 value = mem.Read( temp_address );
		if ( temp_address != address )
		{
			value = mem.Read( address );
			cpu.Tick();
			#ifdef TEST_CYCLE_TIMES
			cpu.m_dbgExtraCycleDueToPageFault = true;
			#endif
		}
		Operation( value );
		cpu.LastTick();
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
		cpu.Tick();

		mem.Read( pointer );
		u16 address = Get16BitAddressFromPointer( pointer );  // (+2 ticks)

		u16 temp_address = ( address & 0xffffff00 ) + ( ( address + cpu.reg.Y ) & 0xff );
		address += cpu.reg.Y;

		cpu.Tick();

		u8 value = mem.Read( temp_address );
		if ( temp_address != address )
		{
			value = mem.Read( address );
		}
		cpu.Tick();
		mem.Write( address, value );
		value = Operation( value );
		cpu.Tick();

		mem.Write( address, value );
		cpu.LastTick();
	}
	//-------------------------------------------------------------------------------------------------
	//
    // Write instructions (SHA)
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
	template <u8&(*Register)(),u8(*Operation)(u8)>
	void fn_WriteInstructions( )
	{
		u8 pointer = FetchPointer();
		cpu.Tick();

		mem.Read( pointer );
		u16 address = Get16BitAddressFromPointer( pointer ); // (+2 ticks)

		u16 temp_address = ( address & 0xffffff00 ) + ( ( address + cpu.reg.Y ) & 0xff );
		address += cpu.reg.Y;

		u8 value = mem.Read( temp_address );
		if ( temp_address != address )
		{
			value = mem.Read( address );
		}
		cpu.Tick();
		mem.Write( address, Operation( Register() ) );
		cpu.LastTick();
	}

	//-------------------------------------------------------------------------------------------------
	//
    // Write instructions (STA)
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
	// This variant passes the high byte in of the effective address (plus 1 ?!)
	template <u8&(*Register)(),u8(*Operation)(u8,u8)>
	void fn_WriteInstructionsH( )
	{
		u8 pointer = FetchPointer();
		cpu.Tick();

		mem.Read( pointer );
		u16 address = Get16BitAddressFromPointer( pointer ); // (+2 ticks)

		u16 temp_address = ( address & 0xffffff00 ) + ( ( address + cpu.reg.Y ) & 0xff );
		address += cpu.reg.Y;

		u8 value = mem.Read( temp_address );
		if ( temp_address != address )
		{
			value = mem.Read( address );
		}
		cpu.Tick();

		mem.Write( address, Operation( Register(), 1 + ( ( address >> 8 ) & 0xff ) ) );
		cpu.LastTick();
	}
	//-------------------------------------------------------------------------------------------------
	void RegisterInstructions( OpcodeTable& opcodeTable )
	{
		opcodeTable.SetFunctionHandler( mode_izy, LDA, fn_ReadInstructions<op_LDA> );
		opcodeTable.SetFunctionHandler( mode_izy, ORA, fn_ReadInstructions<op_ORA> );
		opcodeTable.SetFunctionHandler( mode_izy, EOR, fn_ReadInstructions<op_EOR> );
		opcodeTable.SetFunctionHandler( mode_izy, AND, fn_ReadInstructions<op_AND> );
		opcodeTable.SetFunctionHandler( mode_izy, ADC, fn_ReadInstructions<op_ADC> );
		opcodeTable.SetFunctionHandler( mode_izy, CMP, fn_ReadInstructions<op_CMP> );
		opcodeTable.SetFunctionHandler( mode_izy, SBC, fn_ReadInstructions<op_SBC> );

		opcodeTable.SetFunctionHandler( mode_izy, LAX, fn_ReadInstructions<op_LAX> );

		opcodeTable.SetFunctionHandler( mode_izy, SLO, fn_ReadModifyWriteInstructions<op_SLO>);
		opcodeTable.SetFunctionHandler( mode_izy, RLA, fn_ReadModifyWriteInstructions<op_RLA>);
		opcodeTable.SetFunctionHandler( mode_izy, SRE, fn_ReadModifyWriteInstructions<op_SRE>);
		opcodeTable.SetFunctionHandler( mode_izy, RRA, fn_ReadModifyWriteInstructions<op_RRA>);
		opcodeTable.SetFunctionHandler( mode_izy, ISC, fn_ReadModifyWriteInstructions<op_ISC>);
		opcodeTable.SetFunctionHandler( mode_izy, DCP, fn_ReadModifyWriteInstructions<op_DCP>);

		opcodeTable.SetFunctionHandler( mode_izy, STA, fn_WriteInstructions<reg_cpuA,op_NULL> );
		opcodeTable.SetFunctionHandler( mode_izy, AHX, fn_WriteInstructionsH<reg_cpuA,op_AHX> );
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
		u8 lo = mem.Read(cpu.reg.PC);
		cpu.IncPC();
		cpu.Tick();

		u8 hi = mem.Read(cpu.reg.PC);
		SetPC( lo, hi );
		cpu.Tick();

		lo = mem.Read(cpu.reg.PC);
		cpu.IncPC();
		cpu.Tick();

		hi = mem.Read(cpu.reg.PC);
		SetPC( lo, hi );
		cpu.LastTick();
	}

	//-------------------------------------------------------------------------------------------------
	void RegisterInstructions( OpcodeTable& opcodeTable )
	{
		opcodeTable.SetFunctionHandler( mode_ind, JMP, fn_JMP );
	}
};

//-------------------------------------------------------------------------------------------------

void RegisterInstructionHandlers( OpcodeTable& opcodeTable )
{
	StackInstructions				::RegisterInstructions( opcodeTable );
	AccumulatorOrImpliedAddressing	::RegisterInstructions( opcodeTable );
	ImmediateAddressing				::RegisterInstructions( opcodeTable );
	AbsoluteAddressing				::RegisterInstructions( opcodeTable );
	ZeroPageAddressing				::RegisterInstructions( opcodeTable );
	ZeroPageIndexedAddressing		::RegisterInstructions( opcodeTable );
	AbsoluteIndexedAddressing		::RegisterInstructions( opcodeTable );
	RelativeAddressing				::RegisterInstructions( opcodeTable );
	AbsoluteIndirectAddressing		::RegisterInstructions( opcodeTable );
	IndirectIndexedAddressing		::RegisterInstructions( opcodeTable );
	IndexedIndirectAddressing		::RegisterInstructions( opcodeTable );

	opcodeTable.CheckForMissingOpcodes();
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
