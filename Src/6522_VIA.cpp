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
// Writing 1 to any bit (except 7) will clear that flag
// Bit 7 is set if any other bits in the register are set
//
//-------------------------------------------------------------------------------------------------

u8 VIA_6522::WriteIFR( u16 address, u8 value )
{
	//
	// Clear any bits specified
	//
	m_register[ IER ] &= ~value; 

	//
	// Set top bit if any bits are set, otherwise clear
	//
	if ( m_register[ IER ] & 0x7f )
	{
		m_register[ IER ] |= 0x80;
	}
	else
	{
		m_register[ IER ] = 0;
	}
	return m_register[ IER ];
}

//-------------------------------------------------------------------------------------------------
//
// Interrupt enable register	
//
//-------------------------------------------------------------------------------------------------

u8 VIA_6522::WriteIER( u16 address, u8 value )
{
	if ( value & 0x80 )
	{
		//
		// Set mode. Sets any 1's in "value" to 1 in the IER
		//
		m_register[ IER ] |= value & 0x7f;
	}
	else
	{
		//
		// Clear mode. Any 1's in "value" will be cleared in the IER
		//
		m_register[ IER ] &= ~value; // NOTE: bit 7 is zero, so this is "masked in" implicitly

		//
		// Clear corresponding bits in IFR. 
		// Calling via mem interface so the memory mapped address in RAM also gets updated
		//
		mem.Write( m_baseAddress + IFR, value );
	}

	m_register[ IER ] |= 0x80; // always set to 1 for reading
	return m_register[ IER ];
}

//-------------------------------------------------------------------------------------------------

u8 VIA_6522::Write6522Via( u16 address, u8 value )
{
	m_register[ address - m_baseAddress ] = value;
	return m_register[ address - m_baseAddress ];
}

//-------------------------------------------------------------------------------------------------

VIA_6522::VIA_6522( u16 viaMemoryMappedStartAddressForORA_B )
	: m_baseAddress( viaMemoryMappedStartAddressForORA_B )
{
	u16 offset = m_baseAddress + SHEILA::WRITE_6522_VIA_A_ORB_IRB_Output_register_B;

	mem.RegisterMemoryMappedAddress( offset + SHEILA::WRITE_6522_VIA_A_ORB_IRB_Output_register_B		, MemoryMapHandler( VIA_6522::Write6522Via ));
	mem.RegisterMemoryMappedAddress( offset + SHEILA::WRITE_6522_VIA_A_ORA_IRA_Output_register_A		, MemoryMapHandler( VIA_6522::Write6522Via ));
	mem.RegisterMemoryMappedAddress( offset + SHEILA::WRITE_6522_VIA_A_DDRB_Data_direction_register_B	, MemoryMapHandler( VIA_6522::Write6522Via ));
	mem.RegisterMemoryMappedAddress( offset + SHEILA::WRITE_6522_VIA_A_DDRA_Data_direction_register_A	, MemoryMapHandler( VIA_6522::Write6522Via ));
	mem.RegisterMemoryMappedAddress( offset + SHEILA::WRITE_6522_VIA_A_T1CL_T1_low_order_latches		, MemoryMapHandler( VIA_6522::Write6522Via ));
	mem.RegisterMemoryMappedAddress( offset + SHEILA::WRITE_6522_VIA_A_T1CH_T1_high_order_counter		, MemoryMapHandler( VIA_6522::Write6522Via ));
	mem.RegisterMemoryMappedAddress( offset + SHEILA::WRITE_6522_VIA_A_T1LL_T1_low_order_latches		, MemoryMapHandler( VIA_6522::Write6522Via ));
	mem.RegisterMemoryMappedAddress( offset + SHEILA::WRITE_6522_VIA_A_T1LH_T1_high_order_latches		, MemoryMapHandler( VIA_6522::Write6522Via ));
	mem.RegisterMemoryMappedAddress( offset + SHEILA::WRITE_6522_VIA_A_T2CL_T2_low_order_latches		, MemoryMapHandler( VIA_6522::Write6522Via ));
	mem.RegisterMemoryMappedAddress( offset + SHEILA::WRITE_6522_VIA_A_T2CH_T2_high_order_counter		, MemoryMapHandler( VIA_6522::Write6522Via ));
	mem.RegisterMemoryMappedAddress( offset + SHEILA::WRITE_6522_VIA_A_SR_Shift_register				, MemoryMapHandler( VIA_6522::Write6522Via ));
	mem.RegisterMemoryMappedAddress( offset + SHEILA::WRITE_6522_VIA_A_ACR_Auxiliary_control_register	, MemoryMapHandler( VIA_6522::Write6522Via ));
	mem.RegisterMemoryMappedAddress( offset + SHEILA::WRITE_6522_VIA_A_PCR_Peripheral_control_register	, MemoryMapHandler( VIA_6522::Write6522Via ));
	mem.RegisterMemoryMappedAddress( offset + SHEILA::WRITE_6522_VIA_A_IFR_Interrupt_flag_register		, MemoryMapHandler( VIA_6522::WriteIFR ));
	mem.RegisterMemoryMappedAddress( offset + SHEILA::WRITE_6522_VIA_A_IER_Interrupt_enable_register	, MemoryMapHandler( VIA_6522::WriteIER ));
	mem.RegisterMemoryMappedAddress( offset + SHEILA::WRITE_6522_VIA_A_ORA_IRA_NO_HANDSHAKE				, MemoryMapHandler( VIA_6522::Write6522Via ));
}

//-------------------------------------------------------------------------------------------------