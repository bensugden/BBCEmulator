//-------------------------------------------------------------------------------------------------
//
// 6522 Versatile Interface Adapters
//
// TODO - work out correct functionality for timer2
//        get correct timing for timer1
//		  shift registers
//		  handshake
//		  port a/port b registers
//		  ....a lot still to do!
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
	m_register[ IFR ] &= ~value; 

	//
	// Set top bit if any bits are set, otherwise clear
	//
	if ( m_register[ IFR ] & ( ~INTERRUPT_SET_CLEAR ) ) // 0x7f
	{
		m_register[ IFR ] |= INTERRUPT_SET_CLEAR; // ( bit 7 or 0x80 )
	}
	else
	{
		m_register[ IFR ] = 0; // no interrupts flagged at this stage
	}
	return m_register[ IFR ];
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
		m_register[ IER ] |= value & ( ~INTERRUPT_SET_CLEAR ); // 0x7f
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

	m_register[ IER ] |= INTERRUPT_SET_CLEAR; // always set to 1 for reading ( bit 7 or 0x80 )

	return m_register[ IER ];
}

//-------------------------------------------------------------------------------------------------

u8 VIA_6522::WriteT1( u16 address, u8 value )
{
	ReadWriteChannel in = (ReadWriteChannel)( address - m_baseAddress );

	if ( ( in == RW_T1C_L ) || ( in == RW_T1L_L ) )
	{
		//
		// These are the same op. Copy to low latch
		//
		m_register[ T1_LATCH_L ] = value;
	}
	else if ( ( in == RW_T1C_H ) || ( in == RW_T1L_H ) )
	{
		//
		// Copy to high latch
		//
		m_register[ T1_LATCH_H ] = value;

		//
		// Reset T1 interrupt flag
		//
		mem.Write( m_baseAddress + RW_IFR, INTERRUPT_TIMER1 );

		//
		// RW_T1C_H also transfers latch into low and high counters (i.e. readies another timer)
		//
		if ( in == RW_T1C_H )
		{
			m_register[ T1_COUNTER_L ] = m_register[ T1_LATCH_L ];
			m_register[ T1_COUNTER_H ] = m_register[ T1_LATCH_H ];

			//
			// One shot mode. PB7 low on load
			//
			if ( ( m_register[ ACR ] >> 6 ) == 2 )
			{
				m_register[ PCR ] &= 0x7f; // clear PB7
			}
		}
	}
	return value;
}


//-------------------------------------------------------------------------------------------------

u8 VIA_6522::ReadT1( u16 address, u8 value )
{
	ReadWriteChannel in = (ReadWriteChannel)( address - m_baseAddress );

	switch ( in )
	{
		case RW_T1C_L:
		{
			//
			// Clear T1 interrupt flag
			//
			mem.Write( m_baseAddress + RW_IFR, INTERRUPT_TIMER1 );

			return m_register[ T1_COUNTER_L ];
		}
		case RW_T1C_H:
			return m_register[ T1_COUNTER_H ];
		case RW_T1L_L:
			return m_register[ T1_LATCH_L ];
		case RW_T1L_H:
			return m_register[ T1_LATCH_H ];
	}
	return value;
}

//-------------------------------------------------------------------------------------------------

u8 VIA_6522::WriteT2( u16 address, u8 value )
{
	ReadWriteChannel in = (ReadWriteChannel)( address - m_baseAddress );

	if ( in == RW_T2C_L )
	{
		m_register[ T2_LATCH_L ] = value;
	}
	else if ( in == RW_T2C_H )
	{
		m_register[ T2_COUNTER_H ] = value;
		m_register[ T2_COUNTER_L ] = m_register[ T2_LATCH_L ];
		//
		// Clear T2 interrupt flag
		//
		mem.Write( m_baseAddress + RW_IFR, INTERRUPT_TIMER2 );
	}
		
	return value;
}
//-------------------------------------------------------------------------------------------------

u8 VIA_6522::ReadT2( u16 address, u8 value )
{
	ReadWriteChannel in = (ReadWriteChannel)( address - m_baseAddress );

	if ( in == RW_T2C_L )
	{
		//
		// Clear T2 interrupt flag
		//
		mem.Write( m_baseAddress + RW_IFR, INTERRUPT_TIMER2 );
		return  m_register[ T2_COUNTER_L ];
	}
	else // if ( in == RW_T2C_H )
	{
		return m_register[ T2_COUNTER_H ];
	}
}


//-------------------------------------------------------------------------------------------------

u8 VIA_6522::WriteACR( u16 address, u8 value )
{
	m_register[ ACR ] = value;
	return value;
}

//-------------------------------------------------------------------------------------------------

u8 VIA_6522::WriteOR( u16 address, u8 value )
{
	ReadWriteChannel in = (ReadWriteChannel)( address - m_baseAddress );

	//
	// Only copy those bits set in DDR from ORA / ORB to PA / PB
	//
	if ( in == RW_ORA_IRA )
	{
		m_register[ ORA ] = value;
		m_register[ PA ] &= ~m_register[ DDRA ];
		m_register[ PA ] |= value & m_register[ DDRA ];
	}
	else //	if ( in == RW_ORB_IRB )
	{
		m_register[ ORB ] = value;
		m_register[ PB ] &= ~m_register[  DDRB ];
		m_register[ PB ] |= value & m_register[ DDRB ];
	}
	return value;
}

//-------------------------------------------------------------------------------------------------

u8 VIA_6522::ReadIR( u16 address, u8 value )
{
	ReadWriteChannel in = (ReadWriteChannel)( address - m_baseAddress );

	if ( in == RW_ORA_IRA )
	{
		//
		// check if latching - if yes, use IRA else use PA
		//
		return ( m_register[ PCR ] & 1 ) ? m_register[ IRA ] : m_register[ PA ]; // IRA gets set when CA1 made active transition 
	}
	else
	{
		//
		// check if latching - if yes, use IRB else use PB
		//
		u8 input =  ( m_register[ PCR ] & 2 ) ? m_register[ IRB ] : m_register[ PB ]; // IRB gets set when CB1 made active transition 
		return ( m_register[ DDRB ] & m_register[ ORB ] ) | ( (~m_register[ DDRB ]) & input ); 
	}
}
//-------------------------------------------------------------------------------------------------

u8 VIA_6522::WriteDDR( u16 address, u8 value )
{
	ReadWriteChannel in = (ReadWriteChannel)( address - m_baseAddress );
	//
	// Refresh PB by rewriting ORA/ORB registers
	//
	if ( in == RW_DDRA )
	{
		m_register[ DDRA ] = value;
		WriteOR( m_baseAddress + RW_ORA_IRA, m_register[ ORA ] );
	}
	else // if ( in == RW_DDRB )
	{
		m_register[ DDRB ] = value;
		WriteOR( m_baseAddress + RW_ORB_IRB, m_register[ ORB ] );
	}
	return value;
}

//-------------------------------------------------------------------------------------------------

u8 VIA_6522::WriteShift( u16 address, u8 value )
{
	m_register[ SHIFT ] = value;
	assert( false );//todo
	return value;
}

//-------------------------------------------------------------------------------------------------

u8 VIA_6522::WritePCR( u16 address, u8 value )
{
	m_register[ PCR ] = value;
	assert( false );//todo
	return value;
}

//-------------------------------------------------------------------------------------------------

void VIA_6522::SetCA1( u8 value )
{
	assert( false );//todo
}

//-------------------------------------------------------------------------------------------------

void VIA_6522::SetCA2( u8 value )
{
	assert( false );//todo
}
//-------------------------------------------------------------------------------------------------

void VIA_6522::SetCB1( u8 value )
{
	assert( false );//todo
}

//-------------------------------------------------------------------------------------------------

void VIA_6522::SetCB2( u8 value )
{
	assert( false );//todo
}

//-------------------------------------------------------------------------------------------------

VIA_6522::VIA_6522( u16 viaMemoryMappedStartAddressForORA_B )
	: m_baseAddress( viaMemoryMappedStartAddressForORA_B )
{
	memset( m_register, 0, sizeof( m_register ) );

	u16 offset = m_baseAddress - SHEILA::WRITE_6522_VIA_A_ORB_IRB_Output_register_B;

	mem.RegisterMemoryMap_Write( offset + SHEILA::WRITE_6522_VIA_A_ORB_IRB_Output_register_B		, MemoryMapHandler( VIA_6522::WriteOR ));
	mem.RegisterMemoryMap_Write( offset + SHEILA::WRITE_6522_VIA_A_ORA_IRA_Output_register_A		, MemoryMapHandler( VIA_6522::WriteOR ));
	mem.RegisterMemoryMap_Write( offset + SHEILA::WRITE_6522_VIA_A_DDRB_Data_direction_register_B	, MemoryMapHandler( VIA_6522::WriteDDR ));
	mem.RegisterMemoryMap_Write( offset + SHEILA::WRITE_6522_VIA_A_DDRA_Data_direction_register_A	, MemoryMapHandler( VIA_6522::WriteDDR ));
	mem.RegisterMemoryMap_Write( offset + SHEILA::WRITE_6522_VIA_A_T1CL_T1_low_order_latches		, MemoryMapHandler( VIA_6522::WriteT1 ));
	mem.RegisterMemoryMap_Write( offset + SHEILA::WRITE_6522_VIA_A_T1CH_T1_high_order_counter		, MemoryMapHandler( VIA_6522::WriteT1 ));
	mem.RegisterMemoryMap_Write( offset + SHEILA::WRITE_6522_VIA_A_T1LL_T1_low_order_latches		, MemoryMapHandler( VIA_6522::WriteT1 ));
	mem.RegisterMemoryMap_Write( offset + SHEILA::WRITE_6522_VIA_A_T1LH_T1_high_order_latches		, MemoryMapHandler( VIA_6522::WriteT1 ));
	mem.RegisterMemoryMap_Write( offset + SHEILA::WRITE_6522_VIA_A_T2CL_T2_low_order_latches		, MemoryMapHandler( VIA_6522::WriteT2 ));
	mem.RegisterMemoryMap_Write( offset + SHEILA::WRITE_6522_VIA_A_T2CH_T2_high_order_counter		, MemoryMapHandler( VIA_6522::WriteT2 ));
	mem.RegisterMemoryMap_Write( offset + SHEILA::WRITE_6522_VIA_A_SR_Shift_register				, MemoryMapHandler( VIA_6522::WriteShift ));
	mem.RegisterMemoryMap_Write( offset + SHEILA::WRITE_6522_VIA_A_ACR_Auxiliary_control_register	, MemoryMapHandler( VIA_6522::WriteACR ));
	mem.RegisterMemoryMap_Write( offset + SHEILA::WRITE_6522_VIA_A_PCR_Peripheral_control_register	, MemoryMapHandler( VIA_6522::WritePCR ));
	mem.RegisterMemoryMap_Write( offset + SHEILA::WRITE_6522_VIA_A_IFR_Interrupt_flag_register		, MemoryMapHandler( VIA_6522::WriteIFR ));
	mem.RegisterMemoryMap_Write( offset + SHEILA::WRITE_6522_VIA_A_IER_Interrupt_enable_register	, MemoryMapHandler( VIA_6522::WriteIER ));
	mem.RegisterMemoryMap_Write( offset + SHEILA::WRITE_6522_VIA_A_ORA_IRA_NO_HANDSHAKE				, MemoryMapHandler( VIA_6522::WriteOR ));

	mem.RegisterMemoryMap_Read( offset + SHEILA::WRITE_6522_VIA_A_ORB_IRB_Output_register_B			, MemoryMapHandler( VIA_6522::ReadIR ));
	mem.RegisterMemoryMap_Read( offset + SHEILA::WRITE_6522_VIA_A_ORA_IRA_Output_register_A			, MemoryMapHandler( VIA_6522::ReadIR ));
	mem.RegisterMemoryMap_Write( offset + SHEILA::WRITE_6522_VIA_A_T1CL_T1_low_order_latches		, MemoryMapHandler( VIA_6522::ReadT1 ));
	mem.RegisterMemoryMap_Write( offset + SHEILA::WRITE_6522_VIA_A_T1CH_T1_high_order_counter		, MemoryMapHandler( VIA_6522::ReadT1 ));
	mem.RegisterMemoryMap_Write( offset + SHEILA::WRITE_6522_VIA_A_T1LL_T1_low_order_latches		, MemoryMapHandler( VIA_6522::ReadT1 ));
	mem.RegisterMemoryMap_Write( offset + SHEILA::WRITE_6522_VIA_A_T1LH_T1_high_order_latches		, MemoryMapHandler( VIA_6522::ReadT1 ));
	mem.RegisterMemoryMap_Write( offset + SHEILA::WRITE_6522_VIA_A_T2CL_T2_low_order_latches		, MemoryMapHandler( VIA_6522::ReadT2 ));
	mem.RegisterMemoryMap_Write( offset + SHEILA::WRITE_6522_VIA_A_T2CH_T2_high_order_counter		, MemoryMapHandler( VIA_6522::ReadT2 ));
}

//-------------------------------------------------------------------------------------------------

void VIA_6522::Tick()
{
	//
	// Timer 1 enabled?
	//
	if ( m_register[ IER ] & INTERRUPT_TIMER1 )
	{
		u16 timer = ( u16( m_register[ T1_COUNTER_H ] ) << 8 ) + m_register[ T1_COUNTER_L ];
		
		if ( --timer == 0 )
		{
			u8 mode = m_register[ ACR ] >> 6;
			if ( mode == 2 )
			{
				//
				// One shot mode "2". PB7 high on terminate.
				//
				m_register[ PCR ] |= 0x80; // set PB7
			}
			else if ( mode == 1 )
			{
				//
				// Free running mode "1"
				//
				m_register[ T1_COUNTER_L ] = m_register[ T1_LATCH_L ];
				m_register[ T1_COUNTER_H ] = m_register[ T1_LATCH_H ];
			}
			else if ( mode == 3 )
			{
				//
				// Free running mode "3"
				//
				m_register[ T1_COUNTER_L ] = m_register[ T1_LATCH_L ];
				m_register[ T1_COUNTER_H ] = m_register[ T1_LATCH_H ];
				m_register[ PCR ] ^= 0x80; // toggle PB7
			}
			
			m_register[ IFR ] |= INTERRUPT_SET_CLEAR | INTERRUPT_TIMER1 ; // set bit 7 of Interrupt flag register plus the relevant bit for that interrupt
			cpu.ThrowInterrupt();
		}
		m_register[ T1_COUNTER_H ] = timer >> 8;
		m_register[ T1_COUNTER_L ] = timer & 0xff;
	}

	//
	// Timer 2
	//
	if ( m_register[ IER ] & INTERRUPT_TIMER2 )
	{
		u16 timer = ( u16( m_register[ T2_COUNTER_H ] ) << 8 ) + m_register[ T2_COUNTER_L ];
		u8 mode = m_register[ ACR ] & 0x20;
		
		// BEN - not sure about all this. need to clarify functionality
		// re-read page 8 on data sheet about this
		if  ( mode == 0 )
		{
			if ( --timer == 0 )
			{
				//
				// Clear T2 interrupt flag
				//
				WriteIFR( m_baseAddress + IFR, INTERRUPT_TIMER2 );
				// set it. this is clearly wrong.
				m_register[ IFR ] |= INTERRUPT_SET_CLEAR | INTERRUPT_TIMER2 ; // set bit 7 of Interrupt flag register plus the relevant bit for that interrupt
				cpu.ThrowInterrupt();
			}
		}
		else
		{
			if ( m_register[ PCR ] & 0x40 )
			{
				if ( --timer == 0 )
				{
					m_register[ IFR ] |= INTERRUPT_SET_CLEAR | INTERRUPT_TIMER2 ; // set bit 7 of Interrupt flag register plus the relevant bit for that interrupt
					cpu.ThrowInterrupt();
				}
			}
		}
		m_register[ T2_COUNTER_H ] = timer >> 8;
		m_register[ T2_COUNTER_L ] = timer & 0xff;
	}
}

//-------------------------------------------------------------------------------------------------