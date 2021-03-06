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
	void Tick( int nCPUClocks );
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
		u8 ORB;							// Output register B					
		u8 ORA;							// Output register A					
		u8 IRB;							// Input register B					
		u8 IRA;							// Input register A					
		u8 DDRB;						// Data direction register B				
		u8 DDRA;						// Data direction register A				
		int	T1_COUNTER;					// T1 Counter * 2 ( measured in CPU clocks NOT 1Mhz Clocks )
		u8 T1_LATCH_L;					// T1 low order latch				
		u8 T1_LATCH_H;					// T1 high order latch					
		int	T2_COUNTER;					// T1 Counter * 2 ( measured in CPU clocks NOT 1Mhz Clocks )
		u8 T2_LATCH_L;					// T2 latch low
		u8 SHIFT;						// Shift register							
		u8 ACR;							// Auxiliary control register				
		u8 PCR;							// Peripheral control register			
		u8 IFR;							// Interrupt flag register				
		u8 IER;							// Interrupt enable register				
		u8 ORA_IRA_NO_HANDSHAKE;		// ORA / IRA no handshake
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
		INTERRUPT_CA2			= 1 << 0, // 0x01
		INTERRUPT_CA1			= 1 << 1, // 0x02
		INTERRUPT_SHIFT			= 1 << 2, // 0x04
		INTERRUPT_CB2			= 1 << 3, // 0x08
		INTERRUPT_CB1			= 1 << 4, // 0x10
		INTERRUPT_TIMER2		= 1 << 5, // 0x20
		INTERRUPT_TIMER1		= 1 << 6, // 0x40
		INTERRUPT_SET			= 1 << 7, // 0x80
	};

	//-------------------------------------------------------------------------------------------------
	//
	// Port Interface ( To be implemented by specific VIA chips )
	//
	//-------------------------------------------------------------------------------------------------

	virtual void	WritePortA( u8 value ) = 0; 
	virtual void	WritePortB( u8 value ) = 0; 

	virtual u8		ReadPortA( ) = 0;
	virtual u8		ReadPortB( ) = 0;

	//-------------------------------------------------------------------------------------------------
	//
	// Memory Map Handlers
	//
	//-------------------------------------------------------------------------------------------------

	u8				WriteIFR( u16 address, u8 value );
	u8				WriteIER( u16 address, u8 value );
	u8				WriteORA( u16 address, u8 value );
	u8				WriteORB( u16 address, u8 value );
	u8				WriteACR( u16 address, u8 value );
	u8				WriteT1( u16 address, u8 value );
	u8				WriteT2( u16 address, u8 value );
	u8				WritePCR( u16 address, u8 value );
	u8				WriteShift( u16 address, u8 value );
	u8				WriteDDR( u16 address, u8 value );

	u8				ReadORA( u16 address, u8 value );
	u8				ReadORB( u16 address, u8 value );
	u8				ReadIR( u16 address, u8 value );
	u8				ReadT1( u16 address, u8 value );
	u8				ReadT2( u16 address, u8 value );
	u8				ReadIFR( u16 address, u8 value );

	//-------------------------------------------------------------------------------------------------

	u8				GetControlLineModeCA1( ) const;
	u8				GetControlLineModeCA2( ) const;
	u8				GetControlLineModeCB1( ) const;
	u8				GetControlLineModeCB2( ) const;
	u8				GetShiftMode( ) const;

	void			SetCA1( u8 value );
	void			SetCA2( u8 value );
	void			SetCB1( u8 value );
	void			SetCB2( u8 value );

	Registers		reg;

private:
	void			UpdateIFR();
	void			Read_WriteA();
	void			Read_WriteB();
	void			UpdateControlChannel_DuringReadOrWriteOfPort( ReadWriteChannel in );
	void			ThrowInterrupt( InterruptFlags interrupt );

	u16				m_baseAddress;

	friend class VideoULA;
};

//-------------------------------------------------------------------------------------------------


