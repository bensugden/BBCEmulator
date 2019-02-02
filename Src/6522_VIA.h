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

	enum Register
	{
		REG_ORB_IRB	= 0,				// Output / Input register B					
		REG_ORA_IRA	,					// Output / Input register A					
		REG_DDRB,						// Data direction register B				
		REG_DDRA,						// Data direction register A				
		REG_T1_CL_COUNTER,				// T1 low order latches / T1 Counter
		REG_T1_CH,						// T1 high order counter					
		REG_T1_LL,						// T1 low order latches					
		REG_T1_LH,						// T1 high order latches					
		REG_T2_CL_COUNTER,				// T2 low order latches	 / T2 Counter	
		REG_T2_CH,						// T2 high order counter					
		REG_SR	,						// hift register							
		REG_ACR	,						// Auxiliary control register				
		REG_PCR	,						// Peripheral control register			
		REG_IFR	,						// Interrupt flag register				
		REG_IER	,						// Interrupt enable register				
		REG_ORA_IRA_NO_HANDSHAKE,		// ORA / IRA no handshake
		_REG_ORB,						// [internal] Output register B	
		_REG_ORA,						// [internal] Output register A	 
		_REG_IRB,						// [internal] Input register B	
		_REG_IRA,						// [internal] Input register A	 
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
		SINGLE_TIME_OUT_NO_PB7,
		FREE_RUNNING_NO_PB7,
		SINGLE_TIME_OUT_PB7_PULSE_ON_LOAD,
		FREE_RUNNING_PB7_SQUARE_WAVE,
		INACTIVE
	};

	void	ThrowInterrupt( u8 flag )
	{
		m_register[ REG_IFR ] |= INTERRUPT_SET_CLEAR | flag ; // set bit 7 of Interrupt flag register plus the relevant bit for that interrupt
		cpu.ThrowInterrupt();
	}

	u8		Write6522Via( u16 address, u8 value );
	u8		WriteIFR( u16 address, u8 value );
	u8		WriteIER( u16 address, u8 value );
	u8		WriteOR( u16 address, u8 value );
	u8		WriteACR( u16 address, u8 value );
	u8		WriteT1( u16 address, u8 value );
	u8		WriteT2( u16 address, u8 value );

	u16		m_baseAddress;
	u8		m_register[ Register::_COUNT ];
	T1Mode	m_t1Mode;
	u16		m_t1Timer;
	u16		m_t2Timer;
};

//-------------------------------------------------------------------------------------------------


