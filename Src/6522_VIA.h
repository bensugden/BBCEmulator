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

	enum InternalRegister
	{
		ORB	= 0,						// Output Input register B					
		ORA	,							// Output Input register A					
		DDRB,							// Data direction register B				
		DDRA,							// Data direction register A				
		T1_COUNTER_L,					// T1 low order Counter
		T1_COUNTER_H,					// T1 high order counter					
		T1_LATCH_L,						// T1 low order latch				
		T1_LATCH_H,						// T1 high order latch					
		T2_COUNTER_L,					// T2 low order Counter	
		T2_COUNTER_H,					// T2 high order counter					
		T2_LATCH_L,						// T2 latch low
		SR	,							// Shift register							
		ACR	,							// Auxiliary control register				
		PCR	,							// Peripheral control register			
		IFR	,							// Interrupt flag register				
		IER	,							// Interrupt enable register				
		ORA_IRA_NO_HANDSHAKE,			// ORA / IRA no handshake
		PA,								// Peripheral Port A
		PB,								// Peripheral Port B
		CA1,							// Control line A1
		CA2,							// Control line A2
		CB1,							// Control line B1
		CB2,							// Control line B2
		IRA,							// Input register A
		IRB,							// Input register B
		_COUNT
	};

	enum InterruptFlags
	{
		INTERRUPT_CA2			= 1 << 0,
		INTERRUPT_CA1			= 1 << 1,
		INTERRUPT_SHIFT			= 1 << 2,
		INTERRUPT_CB2			= 1 << 3,
		INTERRUPT_CB1			= 1 << 4,
		INTERRUPT_TIMER2		= 1 << 5,
		INTERRUPT_TIMER1		= 1 << 6,
		INTERRUPT_SET_CLEAR		= 1 << 7,
	};

	enum T1Mode
	{
		SINGLE_TIME_OUT_NO_PB7				= 0,
		FREE_RUNNING_NO_PB7					= 1,
		SINGLE_TIME_OUT_PB7_PULSE_ON_LOAD	= 2,
		FREE_RUNNING_PB7_SQUARE_WAVE		= 3,
	};

	u8		WriteIFR( u16 address, u8 value );
	u8		WriteIER( u16 address, u8 value );
	u8		WriteOR( u16 address, u8 value );
	u8		WriteACR( u16 address, u8 value );
	u8		WriteT1( u16 address, u8 value );
	u8		WriteT2( u16 address, u8 value );
	u8		WritePCR( u16 address, u8 value );
	u8		WriteShift( u16 address, u8 value );
	u8		WriteDDR( u16 address, u8 value );

	u8		ReadT1CL( u16 address, u8 value );
	u8		ReadT2CL( u16 address, u8 value );

	u16		m_baseAddress;
	u8		m_register[ InternalRegister::_COUNT ];
	T1Mode	m_t1Mode;
};

//-------------------------------------------------------------------------------------------------


