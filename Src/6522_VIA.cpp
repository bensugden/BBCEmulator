//-------------------------------------------------------------------------------------------------
//
// 6522 Versatile Interface Adapters
//
//-------------------------------------------------------------------------------------------------

#include "stdafx.h"

//-------------------------------------------------------------------------------------------------
//
// Interrupt flag register	
//
// Writing 1 to any bit (except 7) will clear that flag ( i.e. we acknowledge that interrupt )
// Bit 7 is set if any bits (0-6) in the register are set
//
//-------------------------------------------------------------------------------------------------

u8 VIA_6522::WriteIFR( u16 address, u8 value )
{
	//
	// Clear any bits specified
	//
	m_register[ REG_IFR ] &= ~value; 

	//
	// Set top bit if any bits are set, otherwise clear
	//
	if ( m_register[ REG_IFR ] & ( ~INTERRUPT_SET_CLEAR ) ) // 0x7f
	{
		m_register[ REG_IFR ] |= INTERRUPT_SET_CLEAR; // ( bit 7 or 0x80 )
	}
	else
	{
		m_register[ REG_IFR ] = 0; // no interrupts flagged at this stage
	}
	return m_register[ REG_IFR ];
}

//-------------------------------------------------------------------------------------------------
//
// Interrupt enable register	
//
//-------------------------------------------------------------------------------------------------

u8 VIA_6522::WriteIER( u16 address, u8 value )
{
	if ( value & INTERRUPT_SET_CLEAR )  // ( bit 7 or 0x80 )
	{
		//
		// Set mode. Sets any 1's in "value" to 1 in the IER
		//
		m_register[ REG_IER ] |= value & ( ~INTERRUPT_SET_CLEAR ); // 0x7f
	}
	else
	{
		//
		// Clear mode. Any 1's in "value" will be cleared in the IER
		//
		m_register[ REG_IER ] &= ~value; // NOTE: bit 7 is zero, so this is "masked in" implicitly

		//
		// Clear corresponding bits in IFR. 
		// Calling via mem interface so the memory mapped address in RAM also gets updated
		//
		mem.Write( m_baseAddress + REG_IFR, value );
	}

	m_register[ REG_IER ] |= INTERRUPT_SET_CLEAR; // always set to 1 for reading ( bit 7 or 0x80 )

	return m_register[ REG_IER ];
}

//-------------------------------------------------------------------------------------------------

u8 VIA_6522::Write6522Via( u16 address, u8 value )
{
	m_register[ address - m_baseAddress ] = value;
	return value;
}

//-------------------------------------------------------------------------------------------------

u8 VIA_6522::WriteT1( u16 address, u8 value )
{
	m_register[ address - m_baseAddress ] = value;
	return m_register[ address - m_baseAddress ];
}

//-------------------------------------------------------------------------------------------------

u8 VIA_6522::WriteT2( u16 address, u8 value )
{
	m_register[ address - m_baseAddress ] = value;
	return m_register[ address - m_baseAddress ];
}

//-------------------------------------------------------------------------------------------------

u8 VIA_6522::WriteACR( u16 address, u8 value )
{
	T1Mode new_mode =  (T1Mode)( value >> 6 );
	
	//
	// Change Timer Mode?
	// 
	if ( new_mode != m_t1Mode )
	{
		m_t1Mode = new_mode;
	}

	m_register[ address - m_baseAddress ] = value;
	return value;
}

//-------------------------------------------------------------------------------------------------

u8 VIA_6522::WriteOR( u16 address, u8 value )
{
	Register reg = (Register)( address - m_baseAddress );

	//
	// If a bit in DDR == 0 then it's input
	//                 == 1			  output
	//
	// If bit is INPUT then we ignore write of that bit
	//
	if ( reg == REG_ORA_IRA )
	{
		m_register[ _REG_ORA ] &= ~m_register[ REG_DDRA ];
		m_register[ _REG_ORA ] |= value & m_register[ REG_DDRA ];

		value = m_register[ _REG_IRA ];
	}
	else
	{
		//	if ( reg == ORB_IRB )
		m_register[ _REG_ORB ] &= ~m_register[ REG_DDRB ];
		m_register[ _REG_ORB ] |= value & m_register[ REG_DDRB ];

		value = m_register[ _REG_IRB ];
	}

	return value;
}

//-------------------------------------------------------------------------------------------------

VIA_6522::VIA_6522( u16 viaMemoryMappedStartAddressForORA_B )
	: m_baseAddress( viaMemoryMappedStartAddressForORA_B )
{
	memset( m_register, 0, sizeof( m_register ) );
	m_t1Mode = INACTIVE;

	u16 offset = m_baseAddress + SHEILA::WRITE_6522_VIA_A_ORB_IRB_Output_register_B;

	mem.RegisterMemoryMappedAddress( offset + SHEILA::WRITE_6522_VIA_A_ORB_IRB_Output_register_B		, MemoryMapHandler( VIA_6522::WriteOR ));
	mem.RegisterMemoryMappedAddress( offset + SHEILA::WRITE_6522_VIA_A_ORA_IRA_Output_register_A		, MemoryMapHandler( VIA_6522::WriteOR ));
	mem.RegisterMemoryMappedAddress( offset + SHEILA::WRITE_6522_VIA_A_DDRB_Data_direction_register_B	, MemoryMapHandler( VIA_6522::Write6522Via ));
	mem.RegisterMemoryMappedAddress( offset + SHEILA::WRITE_6522_VIA_A_DDRA_Data_direction_register_A	, MemoryMapHandler( VIA_6522::Write6522Via ));
	mem.RegisterMemoryMappedAddress( offset + SHEILA::WRITE_6522_VIA_A_T1CL_T1_low_order_latches		, MemoryMapHandler( VIA_6522::WriteT1 ));
	mem.RegisterMemoryMappedAddress( offset + SHEILA::WRITE_6522_VIA_A_T1CH_T1_high_order_counter		, MemoryMapHandler( VIA_6522::WriteT1 ));
	mem.RegisterMemoryMappedAddress( offset + SHEILA::WRITE_6522_VIA_A_T1LL_T1_low_order_latches		, MemoryMapHandler( VIA_6522::WriteT1 ));
	mem.RegisterMemoryMappedAddress( offset + SHEILA::WRITE_6522_VIA_A_T1LH_T1_high_order_latches		, MemoryMapHandler( VIA_6522::WriteT1 ));
	mem.RegisterMemoryMappedAddress( offset + SHEILA::WRITE_6522_VIA_A_T2CL_T2_low_order_latches		, MemoryMapHandler( VIA_6522::WriteT2 ));
	mem.RegisterMemoryMappedAddress( offset + SHEILA::WRITE_6522_VIA_A_T2CH_T2_high_order_counter		, MemoryMapHandler( VIA_6522::WriteT2 ));
	mem.RegisterMemoryMappedAddress( offset + SHEILA::WRITE_6522_VIA_A_SR_Shift_register				, MemoryMapHandler( VIA_6522::Write6522Via ));
	mem.RegisterMemoryMappedAddress( offset + SHEILA::WRITE_6522_VIA_A_ACR_Auxiliary_control_register	, MemoryMapHandler( VIA_6522::WriteACR ));
	mem.RegisterMemoryMappedAddress( offset + SHEILA::WRITE_6522_VIA_A_PCR_Peripheral_control_register	, MemoryMapHandler( VIA_6522::Write6522Via ));
	mem.RegisterMemoryMappedAddress( offset + SHEILA::WRITE_6522_VIA_A_IFR_Interrupt_flag_register		, MemoryMapHandler( VIA_6522::WriteIFR ));
	mem.RegisterMemoryMappedAddress( offset + SHEILA::WRITE_6522_VIA_A_IER_Interrupt_enable_register	, MemoryMapHandler( VIA_6522::WriteIER ));
	mem.RegisterMemoryMappedAddress( offset + SHEILA::WRITE_6522_VIA_A_ORA_IRA_NO_HANDSHAKE				, MemoryMapHandler( VIA_6522::Write6522Via ));
}

//-------------------------------------------------------------------------------------------------

void VIA_6522::Tick()
{
	if ( m_t1Mode != INACTIVE )
	{
		m_t1Timer--;
		if ( m_t1Timer == 0 )
		{
			ThrowInterrupt( INTERRUPT_TIMER1 );
			// TODO: do other stuff like reset for free running etc.
		}
	}
}

//-------------------------------------------------------------------------------------------------