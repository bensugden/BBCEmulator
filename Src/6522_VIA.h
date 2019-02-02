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
protected:
	VIA_6522(  u16 viaMemoryMappedStartAddressForORA_B  );

	enum Register
	{
		ORB	= 0,					// Output register B					
		ORA	,						// Output register A					
		DDRB,						// Data direction register B				
		DDRA,						// Data direction register A				
		T1CL,						// T1 low order latches					
		T1CH,						// T1 high order counter					
		T1LL,						// T1 low order latches					
		T1LH,						// T1 high order latches					
		T2CL,						// T2 low order latches					
		T2CH,						// T2 high order counter					
		SR	,						// hift register							
		ACR	,						// Auxiliary control register				
		PCR	,						// Peripheral control register			
		IFR	,						// Interrupt flag register				
		IER	,						// Interrupt enable register				
		ORA_IRA_NO_HANDSHAKE,		// ORA / IRA no handshake
		_COUNT
	};

	void	ThrowInterrupt( u8 flag )
	{
		m_register[ IFR ] |= 0x80 | flag ; // set bit 7 of Interrupt flag register plus the relevant bit for that interrupt
	}

	u8		Write6522Via( u16 address, u8 value );
	u8		WriteIFR( u16 address, u8 value );
	u8		WriteIER( u16 address, u8 value );

	u16		m_baseAddress;
	u8		m_register[ Register::_COUNT ];
};

//-------------------------------------------------------------------------------------------------


