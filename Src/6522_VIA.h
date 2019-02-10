//-------------------------------------------------------------------------------------------------
//
// 6522 VIA
//
// NOTE: This is a base class and contains common functionality for the 6522. 
// The specific MOS and parallel/user port chips are derived from this.
//
//-------------------------------------------------------------------------------------------------

#pragma once

//-------------------------------------------------------------------------------------------------

class VIA_6522
{
public:
	void Tick();
protected:
	VIA_6522(  u16 viaMemoryMappedStartAddressForORA_B  );

	//-------------------------------------------------------------------------------------------------

	enum ReadWriteChannel
	{
		RW_ORB_IRB	= 0,				// Output / Input register B		
		RW_ORA_IRA	,					// Output / Input register A		
		RW_DDRB,						// Data direction register B	
		RW_DDRA,						// Data direction register A	
		RW_T1C_L,						// T1 low order latch
		RW_T1C_H,						// T1 high order counter		
		RW_T1L_L,						// T1 low order latch			
		RW_T1L_H,						// T1 high order latch			
		RW_T2C_L,						// T2 low order Counter	
		RW_T2C_H,						// T2 high order counter		
		RW_SR	,						// Shift register				
		RW_ACR	,						// Auxiliary control register	
		RW_PCR	,						// Peripheral control register
		RW_IFR	,						// Interrupt flag register		
		RW_IER	,						// Interrupt enable register	
		RW_ORA_IRA_NO_HANDSHAKE,		// ORA / IRA no handshake
	};

	//-------------------------------------------------------------------------------------------------

	struct Registers
	{
		u8 B;							// Output Input register B					
		u8 A;							// Output Input register A					
		u8 DDRB;						// Data direction register B				
		u8 DDRA;						// Data direction register A				
		u8 T1_COUNTER_L;				// T1 low order Counter
		u8 T1_COUNTER_H;				// T1 high order counter					
		u8 T1_LATCH_L;					// T1 low order latch				
		u8 T1_LATCH_H;					// T1 high order latch					
		u8 T2_COUNTER_L;				// T2 low order Counter	
		u8 T2_COUNTER_H;				// T2 high order counter					
		u8 T2_LATCH_L;					// T2 latch low
		u8 SHIFT;						// Shift register							
		u8 ACR;							// Auxiliary control register				
		u8 PCR;							// Peripheral control register			
		u8 IFR;							// Interrupt flag register				
		u8 IER;							// Interrupt enable register				
		u8 ORA_IRA_NO_HANDSHAKE;		// ORA / IRA no handshake
		u8 PA;							// Peripheral Port A
		u8 PB;							// Peripheral Port B
		u8 CA1;							// Control line A1
		u8 CA2;							// Control line A2
		u8 CB1;							// Control line B1
		u8 CB2;							// Control line B2
		u8 CA2_TIMER;					// 1 cycle counter for CA2 in mode 5
		u8 CB2_TIMER;					// 1 cycle counter for CB2 in mode 5
		u8 T2_INTERRUPT_ENABLED;		// T2 timer should interrupt when timer is at 0
	};

	//-------------------------------------------------------------------------------------------------
	//
	// Interrupt Flag Register / Interrupt Enable Register
	//
	// 0 - ca2 active edge
	// 1 - ca1 active edge
	// 2 - complete 8 shifts
	// 3 - cb2 active edge
	// 4 - cb1 active edge
	// 5 - time out of t2
	// 6 - time out of t1
	// 7 - any interrupt
	//	
	//-------------------------------------------------------------------------------------------------

	enum InterruptFlags
	{
		INTERRUPT_CA2			= 1 << 0,
		INTERRUPT_CA1			= 1 << 1,
		INTERRUPT_SHIFT			= 1 << 2,
		INTERRUPT_CB2			= 1 << 3,
		INTERRUPT_CB1			= 1 << 4,
		INTERRUPT_TIMER2		= 1 << 5,
		INTERRUPT_TIMER1		= 1 << 6,
		INTERRUPT_SET			= 1 << 7,
	};

	//-------------------------------------------------------------------------------------------------

	u8			WriteIFR( u16 address, u8 value );
	u8			WriteIER( u16 address, u8 value );
	u8			WriteOR( u16 address, u8 value );
	u8			WriteACR( u16 address, u8 value );
	u8			WriteT1( u16 address, u8 value );
	u8			WriteT2( u16 address, u8 value );
	u8			WritePCR( u16 address, u8 value );
	u8			WriteShift( u16 address, u8 value );
	u8			WriteDDR( u16 address, u8 value );
	u8			ReadIR( u16 address, u8 value );
	u8			ReadT1( u16 address, u8 value );
	u8			ReadT2( u16 address, u8 value );

	u8			GetControlLineModeCA1( ) const;
	u8			GetControlLineModeCA2( ) const;
	u8			GetControlLineModeCB1( ) const;
	u8			GetControlLineModeCB2( ) const;
	u8			GetShiftMode( ) const;

	void		SetCA1( u8 value );
	void		SetCA2( u8 value );
	void		SetCB1( u8 value );
	void		SetCB2( u8 value );

	Registers	reg;

private:
	void		SetCAB1( u8 value, bool B, InterruptFlags interrupt );
	void		SetCAB2( u8 value, bool B, InterruptFlags interrupt );
	void		UpdateControlChannel_DuringReadOrWriteOfPort( ReadWriteChannel in );
	void		ThrowInterrupt( InterruptFlags interrupt );

	u16			m_baseAddress;
};

//-------------------------------------------------------------------------------------------------


